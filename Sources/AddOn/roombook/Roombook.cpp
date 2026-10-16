//------------ kuvbur 2022 ------------
// SDK подключается раньше стандартных заголовков независимо от категорий сортировки проекта.
// clang-format off
#include "ACAPinc.h"
// clang-format on

#include <limits>
#include <map>
#include <string>

#include "api_headers/APIEnvir.h"

#include "roombook/Roombook.hpp"

#include "Algorithms.hpp"
#include "CommonFunction.hpp"
#include "Helpers.hpp"
#include "OnExit.hpp"
#include "ProfileAdditionalInfo.hpp"
#include "ProfileVectorImage.hpp"
#include "ProfileVectorImageOperations.hpp"
#include "Propertycache.hpp"
#include "roombook/FloorCoreGeometry.hpp"
#include "roombook/RoomAnalyticGeometry.hpp"
#include "roombook/RoomSurfaceFinish.hpp"
#include "Sync.hpp"
#include "third_party/alphanum.h"
#include "VectorImageIterator.hpp"

namespace Roombook

{
#ifndef ServerMainVers_2400
    // В AC22/AC23 сообщает, что расчёт RoomBook недоступен.
    void RoomBook () { SafeWriteReport ("Function not work in AC22 and AC23", true); }
#else
    static const API_ElemTypeID typeinzone[] = {
        API_WindowID, API_DoorID, API_WallID, API_ColumnID, API_SlabID, API_ZoneID};

    RoomEdges *reducededges = nullptr; // Указатель для обработки полигонов зоны

    static const ClassOtd cls;

    GS::Int32 nPhase = 1;

    // Добавляет зоны из выделения, прошедшие фильтр доступности; если список пуст, запрашивает все зоны с тем же
    // фильтром. Возвращает false при ошибке общего запроса или отсутствии зон.
    static bool GetTargetZones (GS::Array<API_Guid> &zones) {
        GSErrCode err = NoError;
        API_SelectionInfo selectionInfo = {};
        GS::Array<API_Neig> selNeigs;
        err = ACAPI_Selection_Get (&selectionInfo, &selNeigs, true);
        BMKillHandle ((GSHandle *)&selectionInfo.marquee.coords);
        if (err != APIERR_NOSEL && selectionInfo.typeID != API_SelEmpty) {
            for (const API_Neig &neig : selNeigs) {
                API_ElemTypeID elementType;
    #ifdef ServerMainVers_2600
                API_ElemType elementType_ = Neig_To_ElemID (neig.neigID);
                elementType = elementType_.typeID;
    #else
                elementType = Neig_To_ElemID (neig.neigID);
    #endif
                if (elementType == API_ZoneID) {
                    if (!ACAPI_Element_Filter (neig.guid,
                                               APIFilt_IsEditable | APIFilt_OnVisLayer | APIFilt_HasAccessRight |
                                                   APIFilt_InMyWorkspace | APIFilt_IsVisibleByRenovation))
                        continue;
                    zones.Push (neig.guid);
                }
            }
        }
        if (zones.IsEmpty ()) {
            err = ACAPI_Element_GetElemList (API_ZoneID,
                                             &zones,
                                             APIFilt_IsEditable | APIFilt_OnVisLayer | APIFilt_HasAccessRight |
                                                 APIFilt_InMyWorkspace | APIFilt_IsVisibleByRenovation);
            if (err != NoError) {
                msg_rep ("RoomBook err", "ACAPI_Element_GetElemList", err, APINULLGuid);
                return false;
            }
        }
        if (zones.IsEmpty ()) {
            msg_rep ("RoomBook err", "Zones not found or all not editable", NoError, APINULLGuid);
            return false;
        }
        return true;
    }

    // AC25 has only typeID/filterBits/loc/z (no variation or additional request fields).
    // Both callers zero-initialize the SDK request, then set every one of these fields.
    static GSErrCode SearchCoordinate (API_ElemSearchPars &request, API_Guid &guid, CoordinateSearchCache *cache) {
        const auto search = [&] () {
            API_Guid found = APINULLGuid;
    #ifdef ServerMainVers_2700
            const GSErrCode error = ACAPI_Element_SearchElementByCoord (&request, &found);
    #else
            const GSErrCode error = ACAPI_Goodies (APIAny_SearchElementByCoordID, &request, &found);
    #endif
            return CoordinateSearchResult{error, found};
        };
        CoordinateSearchResult result;
    #if defined(_WIN32) && defined(ServerMainVers_2500) && !defined(ServerMainVers_2600)
        if (cache != nullptr) {
            const CoordinateSearchRequest key = {request.loc.x,
                                                 request.loc.y,
                                                 request.z,
                                                 static_cast<std::uint64_t> (request.typeID),
                                                 0,
                                                 static_cast<std::uint64_t> (request.filterBits)};
            result = cache->SearchOrRead (key, search);
        } else
    #else
        (void)cache;
    #endif
        {
            result = search ();
        }
        guid = result.guid;
        return result.error;
    }

    struct RoomProcessingContext {
        ClassificationFunc::ClassificationDict finclass;
        UnicGuid finclassguids;
        Stories storyLevels;
        CoordinateSearchStats coordinateSearchStats;
        CoordinateSearchCache coordinateCache;
        RoomSurfaceCache surfaceCache;
        GS::Array<API_Guid> deletelist;
        GS::HashTable<API_Guid, UnicGuidByBase> exsistot_byzone;
        OtdRooms roomsinfo;
        UnicElementByType elementToRead;
        ParamDictElement paramToRead;
        ParamValue param_composite;
        ParamDictValue paramDict_favorite;
        UnicGUIDByType guidselementToRead;
        GS::HashTable<API_Guid, GS::Array<OtdOpening>> openinginwall;
        GS::HashTable<API_Guid, GS::Array<API_Guid>> slabsinzone;
    };

    // Собственные конечные грани: нулевые native-индексы не обозначают SDK-тело/полигон.
    static RoomSurface AnalyticFace (const GS::Array<API_Coord3D> &points,
                                     const API_Coord3D &normal,
                                     Int32 id,
                                     Int32 wallIndex = -1) {
        RoomSurface face;
        face.origin = RoomSurfaceOrigin::PreparedAnalytic;
        face.analyticFaceId = id;
        face.preparedWallIndex = wallIndex;
        face.normalWorld = normal;
        face.normalLocal = normal;
        const auto &p = points[0];
        face.planeOffset = -(normal.x * p.x + normal.y * p.y + normal.z * p.z);
        RoomSurfaceContour contour;
        for (const auto &point : points) {
            RoomSurfaceVertex vertex;
            vertex.world = vertex.local = point;
            contour.vertices.Push (vertex);
        }
        face.contours.Push (contour);
        return face;
    }

    // Один ключ источника соответствует одному конечному контуру, включая отверстия, а не отдельной полосе.
    static bool AppendAnalyticRectangles (const std::vector<AnalyticGeometry::Rect> &rectangles,
                                          const API_Coord3D &origin,
                                          const API_Coord3D &u,
                                          const API_Coord3D &v,
                                          const API_Coord3D &normal,
                                          Int32 &faceId,
                                          RoomSurfaceBody &body,
                                          Int32 wallIndex = -1) {
        Geometry::MultiPolygon2D polygons;
        for (const auto &r : rectangles) {
            const GS::Array<Point2D> points = {
                {r.left, r.bottom}, {r.right, r.bottom}, {r.right, r.top}, {r.left, r.top}};
            const GS::Array<PolyArcRec> arcs;
            const GS::Array<UIndex> ends = {3};
            polygons.Append (
                Geometry::Polygon2D::Create (points, arcs, ends, Geometry::PolyCreateFlags_IdxRefInContourEndFromZero));
        }
        polygons.Unify ();
        for (const auto &polygon : polygons) {
            GS::Array<Point2D> coords;
            GS::Array<PolyArcRec> arcs;
            GS::Array<UIndex> ends;
            GS::Array<Geometry::Polygon2D::ConstContourIterator> reversed;
            polygon.CopyPolygonData (
                coords, arcs, ends, reversed, Geometry::PolyCreateFlags_IdxRefInContourEndFromZero);
            if (ends.IsEmpty () || !arcs.IsEmpty ())
                return false;
            RoomSurface face;
            face.origin = RoomSurfaceOrigin::PreparedAnalytic;
            face.analyticFaceId = ++faceId;
            face.preparedWallIndex = wallIndex;
            face.normalWorld = face.normalLocal = normal;
            UIndex start = 0;
            for (UIndex contourIndex = 0; contourIndex < ends.GetSize (); ++contourIndex) {
                RoomSurfaceContour contour;
                contour.hole = contourIndex != 0;
                for (UIndex i = start; i <= ends[contourIndex]; ++i) {
                    RoomSurfaceVertex vertex;
                    vertex.world = vertex.local = {origin.x + u.x * coords[i].x + v.x * coords[i].y,
                                                   origin.y + u.y * coords[i].x + v.y * coords[i].y,
                                                   origin.z + u.z * coords[i].x + v.z * coords[i].y};
                    contour.vertices.Push (vertex);
                }
                start = ends[contourIndex] + 1;
                face.contours.Push (contour);
            }
            const auto &point = face.contours[0].vertices[0].world;
            face.planeOffset = -(normal.x * point.x + normal.y * point.y + normal.z * point.z);
            body.surfaces.Push (face);
        }
        return true;
    }

    bool BuildPreparedWallSource (const API_Element &element,
                                  double bottom,
                                  const GS::Array<OtdOpening> &openings,
                                  GS::Array<OtdWall> &bases,
                                  RoomSurfaceSource &source) {
        source = RoomSurfaceSource ();
        const auto &wall = element.wall;
        source.baseGuid = source.geometryGuid = element.header.guid;
        source.baseType = API_WallID;
        if (!AnalyticGeometry::OrdinaryWall (wall.modelElemStructureType == API_ProfileStructure,
                                             wall.angle == 0,
                                             wall.profileType == APISect_Normal,
                                             wall.type == APIWtyp_Normal,
                                             wall.logHeight != 0,
                                             wall.relativeTopStory != 0)) {
            // Номинальная прямоугольная модель не доказывает дугу, уклон, профиль/брёвна или связанную высоту.
    #if defined(TESTING)
            if (wall.modelElemStructureType == API_ProfileStructure)
                DBprnt ("    source: ProfileWall");
            else if (wall.angle != 0)
                DBprnt ("    source: CurvedWall");
            else if (wall.profileType != APISect_Normal)
                DBprnt ("    source: SlantedOrTrapezoidSection");
            else if (wall.type != APIWtyp_Normal)
                DBprnt ("    source: PolygonOrTrapezoidWall");
            else if (wall.logHeight != 0)
                DBprnt ("    source: LogWall");
            else
                DBprnt ("    source: TopLinkedWall");
    #endif
            return false;
        }
        if ((wall.hasWindow || wall.hasDoor) && openings.IsEmpty ()) {
    #if defined(TESTING)
            DBprnt ("    source: UnresolvedWallOpeningIds");
    #endif
            return false;
        }
        AnalyticGeometry::Point ob, oe, ib, ie;
        if (!AnalyticGeometry::WallSides ({wall.begC.x, wall.begC.y},
                                          {wall.endC.x, wall.endC.y},
                                          wall.thickness,
                                          wall.offsetFromOutside,
                                          wall.flipped,
                                          ob,
                                          oe,
                                          ib,
                                          ie) ||
            !std::isfinite (bottom) || !std::isfinite (wall.height) || wall.height <= 0) {
    #if defined(TESTING)
            DBprnt ("    source: InvalidWallDimensions");
    #endif
            return false;
        }
        const double length = std::hypot (wall.endC.x - wall.begC.x, wall.endC.y - wall.begC.y);
        std::vector<AnalyticGeometry::Rect> cuts;
        for (const auto &op : openings)
            cuts.push_back (
                {op.objLoc - op.width / 2, bottom + op.lower, op.objLoc + op.width / 2, bottom + op.lower + op.height});
        std::vector<AnalyticGeometry::Rect> rectangles;
        if (!AnalyticGeometry::SubtractOpenings ({0, bottom, length, bottom + wall.height}, cuts, rectangles)) {
    #if defined(TESTING)
            DBprnt ("    source: UnsupportedOpeningDimensions");
    #endif
            return false;
        }
        RoomSurfaceBody body;
        body.parentGuid = source.baseGuid;
        body.parentType = API_WallID;
        GS::Array<OtdWall> prepared;
        Int32 faceId = 0;
        const AnalyticGeometry::Point starts[] = {ob, ie, oe, ib};
        const AnalyticGeometry::Point ends[] = {oe, ib, ie, ob};
        for (Int32 side = 0; side < 4; ++side) {
            const auto &beg = starts[side];
            const auto &end = ends[side];
            const double sideLength = std::hypot (end.x - beg.x, end.y - beg.y);
            if (sideLength <= 0 || !std::isfinite (sideLength)) {
    #if defined(TESTING)
                DBprnt ("    source: InvalidWallSide");
    #endif
                return false;
            }
            // Ориентация контура согласована с наружной нормалью и flipped сечения.
            const double sign = wall.flipped ? -1 : 1;
            const API_Coord3D normal = {-(end.y - beg.y) / sideLength * sign, (end.x - beg.x) / sideLength * sign, 0};
            OtdWall base;
            base.begC = {beg.x, beg.y};
            base.endC = {end.x, end.y};
            base.height = wall.height;
            base.zBottom = bottom;
            base.floorInd = element.header.floorInd;
            base.base_th = wall.thickness;
            base.base_guid = source.baseGuid;
            base.base_type = API_WallID;
            base.type = Wall_Main;
            base.base_flipped = side == 1;
            if (side < 2)
                for (const auto &op : openings)
                    Opening_Add_One (op, side == 1, bottom, 0, length, length, base);
            const Int32 baseIndex = static_cast<Int32> (bases.GetSize () + prepared.GetSize ());
            prepared.Push (base);
            std::vector<AnalyticGeometry::Rect> endRectangles;
            std::vector<AnalyticGeometry::Rect> endCuts;
            if (side >= 2)
                for (const auto &cut : cuts) {
                    const double axial = side == 2 ? length : 0;
                    if (cut.left <= axial && cut.right >= axial)
                        endCuts.push_back ({0, cut.bottom, sideLength, cut.top});
                }
            AnalyticGeometry::SubtractOpenings ({0, bottom, sideLength, bottom + wall.height}, endCuts, endRectangles);
            std::vector<AnalyticGeometry::Rect> sideRectangles;
            for (const auto &r : side < 2 ? rectangles : endRectangles) {
                // Для обратной стороны осевой интервал зеркалится, IDs проёмов остаются исходными.
                sideRectangles.push_back (
                    {side == 1 ? length - r.right : r.left, r.bottom, side == 1 ? length - r.left : r.right, r.top});
            }
            if (!AppendAnalyticRectangles (sideRectangles,
                                           {beg.x, beg.y, 0},
                                           {(end.x - beg.x) / sideLength, (end.y - beg.y) / sideLength, 0},
                                           {0, 0, 1},
                                           normal,
                                           faceId,
                                           body,
                                           baseIndex))
                return false;
        }
        const double top = bottom + wall.height;
        for (const double z : {bottom, top}) {
            std::vector<AnalyticGeometry::Rect> capCuts;
            for (const auto &cut : cuts)
                if (cut.bottom <= z && cut.top >= z)
                    capCuts.push_back ({cut.left, 0, cut.right, 1});
            std::vector<AnalyticGeometry::Rect> caps;
            AnalyticGeometry::SubtractOpenings ({0, 0, length, 1}, capCuts, caps);
            if (!AppendAnalyticRectangles (caps,
                                           {ob.x, ob.y, z},
                                           {(oe.x - ob.x) / length, (oe.y - ob.y) / length, 0},
                                           {ib.x - ob.x, ib.y - ob.y, 0},
                                           {0, 0, z == bottom ? -1.0 : 1.0},
                                           faceId,
                                           body))
                return false;
        }
        source.origin = RoomSurfaceOrigin::PreparedAnalytic;
    #if defined(TESTING)
        DBprnt ("    source: OrdinaryWallSEOTrimsIgnored");
    #endif
        source.snapshot.guid = source.baseGuid;
        source.snapshot.status = RoomSurfaceStatus::Extracted;
        source.snapshot.bodies.Push (body);
        bases.Append (prepared);
        return true;
    }

    bool BuildPreparedSlabSource (const API_Element &element,
                                  double reference,
                                  const Geometry::Polygon2D &polygon,
                                  GS::Array<OtdSlab> &bases,
                                  RoomSurfaceSource &source) {
        source = RoomSurfaceSource ();
        source.baseGuid = source.geometryGuid = element.header.guid;
        source.baseType = API_SlabID;
        double bottom, top;
        if (!AnalyticGeometry::SlabLevels (
                reference, element.slab.offsetFromTop, element.slab.thickness, bottom, top) ||
            polygon.IsEmpty () || !std::isfinite (polygon.CalcArea ()) || polygon.CalcArea () <= 0) {
    #if defined(TESTING)
            DBprnt ("    source: UnsupportedSlabGeometry");
    #endif
            return false;
        }
        // Оригинальные дуги остаются в OtdSlab для пересечений и writer; хорды нужны только partition.
        const auto segmented = polygon.SegmentUp (ArcDivParams (0.00001));
        if (segmented.IsEmpty ()) {
    #if defined(TESTING)
            DBprnt ("    source: UnsupportedSlabContours");
    #endif
            return false;
        }
        RoomSurfaceBody body;
        body.parentGuid = source.baseGuid;
        body.parentType = API_SlabID;
        Int32 faceId = 0;
        for (const auto &part : segmented) {
            GS::Array<Point2D> coords;
            GS::Array<PolyArcRec> arcs;
            GS::Array<UIndex> ends;
            GS::Array<Geometry::Polygon2D::ConstContourIterator> reversed;
            part.CopyPolygonData (coords, arcs, ends, reversed, Geometry::PolyCreateFlags_IdxRefInContourEndFromZero);
            if (!arcs.IsEmpty () || ends.IsEmpty ()) {
    #if defined(TESTING)
                DBprnt ("    source: UnsupportedSlabContours");
    #endif
                return false;
            }
            RoomSurface lower, upper;
            lower.origin = upper.origin = RoomSurfaceOrigin::PreparedAnalytic;
            lower.analyticFaceId = ++faceId;
            upper.analyticFaceId = ++faceId;
            lower.normalWorld = lower.normalLocal = {0, 0, -1};
            upper.normalWorld = upper.normalLocal = {0, 0, 1};
            lower.planeOffset = bottom;
            upper.planeOffset = -top;
            UIndex start = 0;
            for (UIndex contourIndex = 0; contourIndex < ends.GetSize (); ++contourIndex) {
                RoomSurfaceContour low, high;
                low.hole = high.hole = contourIndex != 0;
                for (UIndex i = start; i <= ends[contourIndex]; ++i) {
                    RoomSurfaceVertex a, b;
                    a.local = a.world = {coords[i].x, coords[i].y, bottom};
                    b.local = b.world = {coords[i].x, coords[i].y, top};
                    low.vertices.Push (a);
                    high.vertices.Push (b);
                    const auto next = coords[i == ends[contourIndex] ? start : i + 1];
                    const auto curr = coords[i];
                    const double length = std::hypot (next.x - curr.x, next.y - curr.y);
                    if (length <= 0)
                        continue;
                    body.surfaces.Push (AnalyticFace ({{curr.x, curr.y, bottom},
                                                       {next.x, next.y, bottom},
                                                       {next.x, next.y, top},
                                                       {curr.x, curr.y, top}},
                                                      {(next.y - curr.y) / length, -(next.x - curr.x) / length, 0},
                                                      ++faceId));
                }
                start = ends[contourIndex] + 1;
                lower.contours.Push (low);
                upper.contours.Push (high);
            }
            body.surfaces.Push (lower);
            body.surfaces.Push (upper);
        }
        OtdSlab slab;
        slab.poly = polygon;
        slab.zBottom = bottom;
        slab.height = top - bottom;
        slab.reference_z = reference;
        slab.base_guid = source.baseGuid;
        slab.base_type = API_SlabID;
        slab.floorInd = element.header.floorInd;
        bases.Push (slab);
        source.origin = RoomSurfaceOrigin::PreparedAnalytic;
    #if defined(TESTING)
        DBprnt ("    source: OrdinarySlabSEOTrimsIgnored");
    #endif
        source.snapshot.guid = source.baseGuid;
        source.snapshot.status = RoomSurfaceStatus::Extracted;
        source.snapshot.bodies.Push (body);
        return true;
    }

    // Перекрытия читаются только из polygon memo; native fallback допустим лишь для стен.
    static RoomSurfaceSource PrepareSource (const API_Element &element,
                                            const Stories &stories,
                                            const GS::Array<OtdOpening> &openings,
                                            GS::Array<OtdWall> &walls,
                                            GS::Array<OtdSlab> &slabs,
                                            RoomSurfaceCache *cache) {
        RoomSurfaceSource source;
        bool analytic = false;
        if (GetElemTypeID (element.header) == API_WallID) {
            const auto genericOpenings =
                ReadConnectedElements (element.header.guid, API_OpeningID, APIFilt_IsVisibleByRenovation, cache);
            if (genericOpenings.error != NoError || !genericOpenings.items.IsEmpty ()) {
                source.baseGuid = source.geometryGuid = element.header.guid;
                source.baseType = API_WallID;
                if (cache != nullptr)
                    ++cache->counters.nativeFallbackSources;
                source.snapshot = ReadRoomSurfaces (element.header, cache);
                return source;
            }
            analytic = BuildPreparedWallSource (element,
                                                GetzPos (element.wall.bottomOffset, element.header.floorInd, stories),
                                                openings,
                                                walls,
                                                source);
        } else {
            API_ElementMemo memo = {};
            const GS::OnExit memoGuard ([&memo] () { ACAPI_DisposeElemMemoHdls (&memo); });
            Geometry::Polygon2D polygon;
            const GSErrCode err = ACAPI_Element_GetMemo (element.header.guid, &memo, APIMemoMask_Polygon);
            const GSErrCode polygonError = err == NoError ? ConstructPolygon2DFromElementMemo (memo, polygon) : err;
            if (polygonError == NoError)
                analytic = BuildPreparedSlabSource (
                    element, GetzPos (element.slab.level, element.header.floorInd, stories), polygon, slabs, source);
            else {
                source.baseGuid = source.geometryGuid = element.header.guid;
                source.baseType = API_SlabID;
    #if defined(TESTING)
                DBprnt ("    source: SlabPolygonMemoError");
    #endif
            }
        }
        if (analytic) {
            if (cache != nullptr) {
                if (source.baseType == API_WallID)
                    ++cache->counters.analyticWallSources;
                else
                    ++cache->counters.analyticSlabSources;
                for (const auto &body : source.snapshot.bodies)
                    cache->counters.analyticFaces += body.surfaces.GetSize ();
            }
        } else if (GetElemTypeID (element.header) == API_WallID) {
            if (cache != nullptr)
                ++cache->counters.nativeFallbackSources;
            source.snapshot = ReadRoomSurfaces (element.header, cache);
        }
        return source;
    }

    // Находит классификации отделки, читает уровни этажей и собирает перекрытия, пересекающие заданные зоны.
    static void PrepareRoomProcessingContext (RoomProcessingContext &context, const GS::Array<API_Guid> &zones) {
        Class_FindFinClass (context.finclass, context.finclassguids);
        context.storyLevels = GetStories ();
        Floor_FindAll (context.slabsinzone, context.finclassguids, zones, &context.surfaceCache);
    }

    // Собирает данные комнат и связи с базовыми элементами, добавляет зоны в список чтения и очищает временные GUID
    // зон. Прерывается с false при отмене очередной фазы.
    static bool BuildElementReadIndex (const GS::Array<API_Guid> &zones,
                                       RoomProcessingContext &context,
                                       GS::UniString &funcname) {
        // Один точный кэш на read-фазу запуска; discovery продолжается после чтения проёмов.
        auto &coordinateCache = context.coordinateCache;
        for (const API_Guid &zoneGuid : zones) {
            if (AdvanceProcessPhase (funcname, nPhase))
                return false;
            OtdRoom roominfo;
            if (CollectRoomInfo (context.storyLevels,
                                 zoneGuid,
                                 roominfo,
                                 context.elementToRead,
                                 context.slabsinzone,
                                 &context.surfaceCache,
                                 &coordinateCache)) {
                API_Elem_Head zone = {};
                zone.guid = zoneGuid;
                SetElemTypeID (zone, API_ZoneID);
                GS::Array<API_Guid> finishClasses;
                context.finclassguids.EnumerateKeys (
                    [&] (const API_Guid &classGuid) { finishClasses.Push (classGuid); });
                roominfo.surfaceCandidates = ReadRoomSurfaceCandidates (zone, finishClasses, &context.surfaceCache);
                // Partition ждёт подготовки оснований и реальных проёмов.
                if (!roominfo.surfaceCandidates.complete) {
                    msg_rep ("Surface finish",
                             "Incomplete source/zone geometry; RoomBook stopped before writing",
                             APIERR_BADPARS,
                             zoneGuid);
                    return false;
                }
                for (const auto &source : roominfo.surfaceCandidates.sources) {
                    Param_AddUnicGUIDByType (source.baseGuid, source.baseType, context.guidselementToRead);
                    if (source.baseType == API_WallID || source.baseType == API_SlabID)
                        Param_AddUnicElementByType (source.baseGuid, zoneGuid, source.baseType, context.elementToRead);
                }
                context.roomsinfo.Add (zoneGuid, std::move (roominfo));
            }
        }
        // Полная стена сравнивается вместе со всеми видимыми окнами/дверями, не только roomPart-проёмами.
        if (const auto *wallIndex = context.elementToRead.GetPtr (API_WallID)) {
            const UnicElement walls = *wallIndex;
            for (auto it = walls.EnumeratePairs (); it != nullptr; ++it) {
    #ifdef ServerMainVers_2800
                const API_Guid wallGuid = it->key;
                const auto &roomGuids = it->value;
    #else
                const API_Guid wallGuid = *it->key;
                const auto &roomGuids = *it->value;
    #endif
                for (const auto type : {API_WindowID, API_DoorID}) {
                    const auto openings =
                        ReadConnectedElements (wallGuid, type, APIFilt_IsVisibleByRenovation, &context.surfaceCache);
                    if (openings.error != NoError)
                        return false;
                    for (const auto &openingGuid : openings.items)
                        for (const auto &roomGuid : roomGuids)
                            Param_AddUnicElementByType (openingGuid, roomGuid, type, context.elementToRead);
                }
            }
        }
        context.coordinateSearchStats = coordinateCache.GetStats ();
        context.guidselementToRead.Add (API_ZoneID, zones);
        ClearZoneGUID (context.elementToRead);
        return true;
    }

    // Обрабатывает собранные проёмы, стены, колонны и перекрытия, пропуская элементы с классификацией отделки.
    // Заполняет данные комнат и списки чтения; при отмене возвращает false.
    static bool ProcessElementsForRoomData (RoomProcessingContext &context, GS::UniString &funcname) {
        for (const API_ElemTypeID &typeelem : typeinzone) {

            if (auto *elems = context.elementToRead.GetPtr (typeelem)) {
                for (UnicElement::PairIterator cIt = (*elems).EnumeratePairs (); cIt != NULL; ++cIt) {
                    if (AdvanceProcessPhase (funcname, nPhase))
                        return false;
    #ifdef ServerMainVers_2800
                    API_Guid guid = cIt->key;
                    const GS::Array<API_Guid> &zoneGuids = cIt->value;
    #else
                    API_Guid guid = *cIt->key;
                    const GS::Array<API_Guid> &zoneGuids = *cIt->value;
    #endif
                    // Проверяем классификацию, исключаем отделочные элементы
                    API_Guid classguid;
                    if (Class_IsElementFinClass (guid, context.finclassguids, classguid, &context.surfaceCache))
                        continue;
                    switch (typeelem) {
                    case API_WindowID:
                    case API_DoorID:
                        Opening_Create_One (
                            context.storyLevels, guid, context.openinginwall, context.guidselementToRead);
                        break;
                    case API_WallID:
                        // Legacy-участки задают сторону состава для пола и привязку откосов; не рисуются.
                        OtdWall_Create_FromWall (context.storyLevels,
                                                 guid,
                                                 zoneGuids,
                                                 context.roomsinfo,
                                                 context.openinginwall,
                                                 context.guidselementToRead,
                                                 &context.surfaceCache);
                        break;
                    case API_ColumnID:
                        // Основная геометрия берётся из общего разбиения, а не из номинальных осей/размеров.
                        break;
                    case API_SlabID:
                        Floor_FindInOneRoom (context.storyLevels,
                                             guid,
                                             zoneGuids,
                                             context.roomsinfo,
                                             context.guidselementToRead,
                                             &context.surfaceCache);
                        break;
                    default:
                        break;
                    }
                }
            }
        }
        context.coordinateSearchStats = context.coordinateCache.GetStats ();
        return true;
    }

    // До очистки writer-результатов сохраняем неизменяемые основания и legacy-связи откосов.
    static bool PrepareRoomPartitions (RoomProcessingContext &context) {
        for (auto it = context.roomsinfo.EnumeratePairs (); it != nullptr; ++it) {
    #ifdef ServerMainVers_2800
            auto &room = it->value;
    #else
            auto &room = *it->value;
    #endif
            for (auto &source : room.surfaceCandidates.sources) {
                if (source.baseType != API_WallID && source.baseType != API_SlabID)
                    continue;
                if (source.baseType == API_SlabID) {
                    bool found = false;
                    for (const auto &slabSource : room.slabSources)
                        if (slabSource.baseGuid == source.baseGuid) {
                            source = slabSource;
                            found = true;
                            break;
                        }
                    if (found)
                        continue;
                }
                const ElementRead elementRead = ReadElement (source.baseGuid, &context.surfaceCache);
                if (elementRead.error != NoError)
                    return false;
                const API_Element &element = elementRead.element;
                const auto *openings = context.openinginwall.GetPtr (source.baseGuid);
                source = PrepareSource (element,
                                        context.storyLevels,
                                        openings != nullptr ? *openings : GS::Array<OtdOpening> (),
                                        room.preparedWalls,
                                        room.preparedSlabs,
                                        &context.surfaceCache);
            }
            // Discovery-перекрытия тоже участвуют в контактах, не только в проекции пола.
            for (const auto &slabSource : room.slabSources) {
                bool present = false;
                for (const auto &source : room.surfaceCandidates.sources)
                    present = present || source.baseGuid == slabSource.baseGuid;
                if (!present)
                    room.surfaceCandidates.sources.Push (slabSource);
            }
            room.surfacePartition = PartitionRoomSurfaces (room.surfaceSnapshot, room.surfaceCandidates.sources);
            if (!room.surfacePartition.complete) {
                msg_rep ("Surface finish",
                         "Incomplete prepared geometry; stopped before writing",
                         APIERR_BADPARS,
                         room.zone_guid);
                return false;
            }
        }
        return true;
    }

    struct ReadParamsForRoomBook {
        ReadParams windowParams = Param_GetForWindowParams ();
        ReadParams roomParams = Param_GetForRooms ();
    };

    // Создаёт отдельные шаблоны параметров окон и комнат для последующего чтения.
    static ReadParamsForRoomBook PrepareReadParams () { return {}; }

    // Формирует задания чтения параметров для зон, окон и базовых конструкций из собранных GUID.
    // Набор параметров базовой конструкции также сохраняет для поиска избранного.
    static void ReadElementParameters (RoomProcessingContext &context, ReadParamsForRoomBook &readParams) {
        ParamDictValue paramDict;
        const API_ElemTypeID readTypes[] = {
            API_WindowID, API_DoorID, API_WallID, API_ColumnID, API_SlabID, API_BeamID, API_ZoneID};
        for (const API_ElemTypeID &typeelem : readTypes) {
            if (!context.guidselementToRead.ContainsKey (typeelem))
                continue;
            paramDict.Clear ();
            if (typeelem == API_ZoneID) {
                Param_ToParamDict (paramDict, readParams.roomParams);
            }
            if (typeelem == API_WindowID || typeelem == API_DoorID) {
                Param_ToParamDict (paramDict, readParams.windowParams);
            }
            if (typeelem == API_WallID || typeelem == API_ColumnID || typeelem == API_SlabID ||
                typeelem == API_BeamID) {
                Param_GetForBase (paramDict, context.param_composite);
                context.paramDict_favorite = paramDict;
            }
            if (!paramDict.IsEmpty ()) {
                for (const API_Guid &guid : context.guidselementToRead[typeelem]) {
                    ParamHelpers::AddParamDictValue2ParamDictElement (guid, paramDict, context.paramToRead);
                }
            }
        }
    }

    // Заполняет состав и избранное допустимых полов и потолков и назначает им тип помещения.
    // При наличии привязок забирает подходящий существующий GUID из словаря зоны и базового элемента.
    static void ProcessSlabFinishes (OtdRoom &otd,
                                     const API_Guid &zoneGuid,
                                     ParamDictElement &paramToRead,
                                     ParamValue &param_composite,
                                     ParamDictValue &paramDict_favorite,
                                     GS::HashTable<API_Guid, UnicGuidByBase> &exsistot_byzone,
                                     ParamDictCompositeElement &paramCompositeToRead,
                                     MatarialToFavoriteDict &favdict,
                                     bool has_base_element) {

        for (OtdSlab &otdslab : otd.otdslab) {
            bool base_flipped = false;
            GS::UniString fav_name;
            if (!Param_SetToBase (otdslab.base_guid,
                                  base_flipped,
                                  otdslab.base_composite,
                                  paramToRead,
                                  paramCompositeToRead,
                                  param_composite,
                                  fav_name))
                otdslab.isValid = false;
            if (!otdslab.isValid)
                continue;
            otdslab.favorite.name = fav_name;
            SetMaterialFinish (otdslab.material, otdslab.base_composite);
            Favorite_FindName (otdslab.favorite,
                               otdslab.material,
                               otdslab.type,
                               otdslab.draw_type,
                               favdict,
                               paramDict_favorite,
                               param_composite);
            if (otdslab.type == Ceil)
                otdslab.tip = otd.tip_pot;
            if (otdslab.type == Floor)
                otdslab.tip = otd.tip_pol;
            // Ищем существующие элементы по GUID базового элемента
            if (!otdslab.surface_geometry && exsistot_byzone.ContainsKey (zoneGuid) && has_base_element) {
                UnicGuidByBase &exsistot_byparent = exsistot_byzone.Get (zoneGuid);
                if (exsistot_byparent.ContainsKey (otdslab.base_guid)) {
                    UnicGuidByTypeOtd &exsistype = exsistot_byparent.Get (otdslab.base_guid);
                    if (exsistype.ContainsKey (otdslab.type)) {
                        UnicGuid &exsistot = exsistype.Get (otdslab.type);
                        GS::Optional<API_Guid> v = exsistot.FindValue (true);
                        if (v.HasValue ()) {
                            otdslab.otd_guid = v.Get ();
                            exsistot.Delete (otdslab.otd_guid);
                        }
                    } else {
                        if (exsistype.ContainsKey (NoSet)) {
                            UnicGuid &exsistot = exsistype.Get (NoSet);
                            GS::Optional<API_Guid> v = exsistot.FindValue (true);
                            if (v.HasValue ()) {
                                otdslab.otd_guid = v.Get ();
                                exsistot.Delete (otdslab.otd_guid);
                            }
                        }
                    }
                }
            }
        } // Обработка перекрытий
    }

    // Подбирает избранное для допустимых стен; прочитанный непустой состав избранного добавляет к базовому составу и
    // учитывает в материале.
    static void ApplyFavoriteAndMaterialData (GS::Array<OtdWall> &opw,
                                              ParamDictValue &paramDict_favorite,
                                              ParamValue &param_composite,
                                              MatarialToFavoriteDict &favdict) {
        for (OtdWall &otdw : opw) {
            if (!otdw.isValid)
                continue;
            Favorite_FindName (
                otdw.favorite, otdw.material, otdw.type, otdw.draw_type, favdict, paramDict_favorite, param_composite);
            if (otdw.favorite.is_composite_read && !otdw.favorite.composite.IsEmpty ()) {
                SetMaterialFinish_ByComposite (otdw.material, otdw.favorite.composite);
                otdw.base_composite.Append (otdw.favorite.composite);
            }
        } // Назначение избранного
    }

    // Читает состав стен, рассчитывает откосы проёмов и разбивает стены по высотным участкам помещения.
    // Назначает избранное и заменяет исходный массив стен рассчитанными участками.
    static void ProcessWallFinishes (OtdRoom &otd,
                                     const GS::Array<OtdWall> &openingWalls,
                                     ParamDictElement &paramToRead,
                                     ParamValue &param_composite,
                                     ParamDictValue &paramDict_favorite,
                                     ReadParams &windowParams,
                                     ParamDictCompositeElement &paramCompositeToRead,
                                     MatarialToFavoriteDict &favdict) {

        GS::Array<OtdWall> opw; // Массив созданных стен
        GS::Array<OtdWall> openingOwners = openingWalls;
        openingOwners.Append (otd.preparedWalls);
        // Legacy-участки нужны только для откосов; отверстия surface-стен уже вырезаны.
        GS::Array<OtdWall> wallsForRules = otd.otdwall;
        wallsForRules.Append (openingWalls);
        for (OtdWall &otdw : wallsForRules) {
            // Заполняем данные для отделочных стен (состав)
            GS::UniString fav_name;
            if (!Param_SetToBase (otdw.base_guid,
                                  otdw.base_flipped,
                                  otdw.base_composite,
                                  paramToRead,
                                  paramCompositeToRead,
                                  param_composite,
                                  fav_name,
                                  otdw.surface_geometry ? nullptr : &otdw))
                otdw.isValid = false;
            if (!otdw.isValid)
                continue;
            otdw.favorite.name = fav_name;
            if (otdw.openings.IsEmpty () || otdw.surface_geometry)
                continue;
            Point2D wbegC = {otdw.begC.x, otdw.begC.y};
            Point2D wendC = {otdw.endC.x, otdw.endC.y};
            Sector walledge = {wbegC, wendC};
            GS::Optional<UnitVector_2D> walldir = walledge.GetDirection ();
            Geometry::Vector2<double> walldir_perp;
            if (!walldir.HasValue ())
                continue;
            double angz = -DEGRAD * 90;
            double co = cos (angz);
            double si = sin (angz);
            walldir_perp = walldir.Get ().ToVector2D ().Rotate (si, co);
            for (OtdOpening &op : otdw.openings) {
                // Геометрические вырезы не являются окнами основания и не имеют откосов.
                if (op.base_guid == APINULLGuid)
                    continue;
                // Копия на каждый проём: см. комментарий к копии roomParamsWork выше —
                // общая копия тянула бы флаги первого проёма во все остальные.
                ReadParams windowParamsWork = windowParams;
                Param_SetToWindows (op, paramToRead, windowParamsWork, otdw);
                if (!otd.create_reveal_elements || !op.create_reveal)
                    continue;
                OpeningReveals_Create_One (otd.otdslab,
                                           otdw,
                                           op,
                                           walldir_perp,
                                           opw,
                                           otd.zBottom,
                                           otd.height_down,
                                           otd.height_main,
                                           otd.height_up,
                                           otd.height,
                                           otd.om_main,
                                           otd.om_up,
                                           otd.om_down,
                                           otd.om_reveals,
                                           otd.om_column,
                                           otd.om_floor,
                                           otd.om_ceil,
                                           otd.om_zone);
            }
        } // Обработка стен
        // Surface-адаптер не знает сторону многослойки; берём только отдельно
        // рассчитанную глубину legacy-стены того же основания/направления.
        // Материальные правила и геометрию surface-стены не меняем.
        for (auto &target : wallsForRules) {
            if (!target.surface_geometry || target.base_type != API_WallID || target.slab_step)
                continue;
            bool found = false, ambiguous = false;
            double offset = 0;
            const double tx = target.endC.x - target.begC.x, ty = target.endC.y - target.begC.y;
            for (const auto &source : wallsForRules) {
                if (source.surface_geometry || !source.isValid || !source.floor_core_offset_valid ||
                    source.base_guid != target.base_guid)
                    continue;
                const double sx = source.endC.x - source.begC.x, sy = source.endC.y - source.begC.y;
                const double norm = std::hypot (tx, ty) * std::hypot (sx, sy);
                if (norm <= 1.e-12 || tx * sx + ty * sy <= 0 || std::fabs (tx * sy - ty * sx) > 1.e-6 * norm)
                    continue;
                if (found && std::fabs (offset - source.floor_core_offset) > 1.e-6)
                    ambiguous = true;
                found = true;
                offset = source.floor_core_offset;
            }
            target.floor_core_offset_valid = found && !ambiguous;
            target.floor_core_offset = target.floor_core_offset_valid ? offset : 0;
        }
        for (OtdWall &otdw : wallsForRules) {
            // Основную отделку строим только по поверхностям, не по legacy-границам.
            if (!otdw.isValid || !otdw.surface_geometry)
                continue;
            bool analytic = false;
            for (const auto &source : otd.surfaceCandidates.sources)
                analytic = analytic ||
                           (source.baseGuid == otdw.base_guid && source.origin == RoomSurfaceOrigin::PreparedAnalytic);
            if (!LinkSurfaceOpenings (otdw, openingOwners, analytic)) {
                otdw.isValid = false;
                msg_rep ("Surface finish",
                         "Opening ownership is ambiguous or unrepresentable",
                         APIERR_BADPARS,
                         otdw.base_guid);
                continue;
            }
            OtdWall_Delim_All (opw,
                               otdw,
                               otd.zBottom,
                               otd.height_down,
                               otd.height_main,
                               otd.height_up,
                               otd.height,
                               otd.om_main,
                               otd.om_up,
                               otd.om_down,
                               otd.om_reveals,
                               otd.om_column,
                               otd.om_floor,
                               otd.om_ceil,
                               otd.om_zone);
        } // Разбивка стен
        ApplyFavoriteAndMaterialData (opw, paramDict_favorite, param_composite, favdict);
        // Проверяем тот же выбор избранного/defaults и запрет профиля, что и штатный surface-writer.
        for (auto &wall : opw) {
            if (!wall.slab_step || !wall.isValid)
                continue;
            API_Element settings = {};
            wall.slab_step_writer_eligible = OtdWall_GetDefult_Wall (wall.favorite.name, settings) &&
                                             settings.wall.modelElemStructureType != API_ProfileStructure;
        }
        otd.otdwall = opw; // Заменяем на разбитые стены
    }

    struct MaterialSummary {
        ParamDictElement paramToWrite;
        ColumnFormatDict columnFormat;
        OtdMaterialAreaDictByOtdType dct_bytype;
        GS::Array<API_Guid> zones_bytype;
    };

    // Собирает зоны с включённым созданием отделки и рассчитывает площади для комнат с нужными свойствами результата.
    // Читает форматы столбцов, накапливает сводку и возвращает false при отмене.
    static bool BuildMaterialSummaryForRooms (OtdRooms &roomsinfo,
                                              ParamDictElement &paramToRead,
                                              GS::Array<API_Guid> &zones,
                                              MaterialSummary &summary,
                                              GS::UniString &funcname) {
        auto &paramToWrite = summary.paramToWrite;
        auto &columnFormat = summary.columnFormat;
        auto &dct_bytype = summary.dct_bytype;
        auto &zones_bytype = summary.zones_bytype;
        funcname = GS::UniString::Printf ("Calculate material for %d room(s)", roomsinfo.GetSize ());
        for (OtdRooms::PairIterator cIt = roomsinfo.EnumeratePairs (); cIt != NULL; ++cIt) {
    #ifdef ServerMainVers_2800
            OtdRoom &otd = cIt->value;
    #else
            OtdRoom &otd = *cIt->value;
    #endif
            if (AdvanceProcessPhase (funcname, nPhase))
                return false;
            if (otd.isValid && (otd.create_ceil_elements || otd.create_floor_elements || otd.create_wall_elements ||
                                otd.create_column_elements || otd.create_reveal_elements)) {
                zones.Push (otd.zone_guid);
            }
            if (otd.isValid) {
                const ParamDictValue *roomparams = paramToRead.GetPtr (otd.zone_guid);
                if (roomparams != nullptr) {
                    GS::Array<GS::UniString> rawnames;
                    GS::UniString msg;
                    if (!otd.om_up.rawname.IsEmpty ())
                        rawnames.Push (otd.om_up.rawname);
                    if (!otd.om_main.rawname.IsEmpty ())
                        rawnames.Push (otd.om_main.rawname);
                    if (!otd.om_down.rawname.IsEmpty ())
                        rawnames.Push (otd.om_down.rawname);
                    if (!otd.om_column.rawname.IsEmpty ())
                        rawnames.Push (otd.om_column.rawname);
                    if (!otd.om_reveals.rawname.IsEmpty ())
                        rawnames.Push (otd.om_reveals.rawname);
                    if (!otd.om_floor.rawname.IsEmpty ())
                        rawnames.Push (otd.om_floor.rawname);
                    if (!otd.om_ceil.rawname.IsEmpty ())
                        rawnames.Push (otd.om_ceil.rawname);
                    if (!otd.om_up.rawname_bytype.IsEmpty ())
                        rawnames.Push (otd.om_up.rawname_bytype);
                    if (!otd.om_main.rawname_bytype.IsEmpty ())
                        rawnames.Push (otd.om_main.rawname_bytype);
                    if (!otd.om_down.rawname_bytype.IsEmpty ())
                        rawnames.Push (otd.om_down.rawname_bytype);
                    if (!otd.om_column.rawname_bytype.IsEmpty ())
                        rawnames.Push (otd.om_column.rawname_bytype);
                    if (!otd.om_reveals.rawname_bytype.IsEmpty ())
                        rawnames.Push (otd.om_reveals.rawname_bytype);
                    if (!otd.om_floor.rawname_bytype.IsEmpty ())
                        rawnames.Push (otd.om_floor.rawname_bytype);
                    if (!otd.om_ceil.rawname_bytype.IsEmpty ())
                        rawnames.Push (otd.om_ceil.rawname_bytype);
                    if (otd.om_main.rawname.IsEmpty () && otd.om_main.rawname_bytype.IsEmpty ())
                        msg += "'some_stuff_fin_main_result' ";
                    if (otd.om_down.rawname.IsEmpty () && otd.om_down.rawname_bytype.IsEmpty ())
                        msg += "'some_stuff_fin_down_result' ";
                    if (otd.om_column.rawname.IsEmpty () && otd.om_column.rawname_bytype.IsEmpty ())
                        msg += "'some_stuff_fin_column_result' ";
                    if (otd.om_ceil.rawname.IsEmpty () && otd.om_ceil.rawname_bytype.IsEmpty ())
                        msg += "'some_stuff_fin_ceil_result' ";
                    if (!msg.IsEmpty ()) {
                        msg_rep ("RoomBook",
                                 "Properties for recording finish layers to the Zone "
                                 "were not "
                                 "found.\nRecording will not be performed. Missing " +
                                     msg,
                                 NoError,
                                 APINULLGuid);
                    } else {
                        // Получение форматов столбцов
                        for (const GS::UniString &rawname : rawnames) {
                            if (rawname.IsEmpty ())
                                continue;
                            if (columnFormat.ContainsKey (rawname))
                                continue;
                            if (!roomparams->ContainsKey (rawname))
                                continue;
                            OtdData_GetColumnfFormat (
                                roomparams->Get (rawname).definition.description, rawname, columnFormat);
                        }
                        // Расчёт площадей
                        OtdData_CalcForRoom (columnFormat, otd, paramToWrite, paramToRead, dct_bytype);
                        zones_bytype.Push (otd.zone_guid);
                    }
                }
            }
        }
        return true;
    }

    // Подготавливает значения свойств зон по общей сводке материалов для их tip_otd, используя имена rawname_bytype.
    static void WriteRoomMaterialData (OtdRooms &roomsinfo, ParamDictElement &paramToRead, MaterialSummary &summary) {
        // Запись отделки с разбивкой
        for (const API_Guid &subguid : summary.zones_bytype) {
            OtdRoom &otd = roomsinfo.Get (subguid);
            if (!summary.dct_bytype.ContainsKey (otd.tip_otd))
                continue;
            OtdMaterialAreaDictByType &dct = summary.dct_bytype.Get (otd.tip_otd);
            GS::HashTable<TypeOtd, GS::UniString> paramnamebytype;
            paramnamebytype.Add (Wall_Up,
                                 otd.om_up.rawname_bytype); // Отделка стен выше потолка
            paramnamebytype.Add (Wall_Main,
                                 otd.om_main.rawname_bytype); // Отделка стен основная
            paramnamebytype.Add (Wall_Down,
                                 otd.om_down.rawname_bytype); // Отделка низа стен
            paramnamebytype.Add (Column,
                                 otd.om_column.rawname_bytype); // Отделка колонн
            paramnamebytype.Add (Reveal_Main,
                                 otd.om_reveals.rawname_bytype);      // Отделка откосов
            paramnamebytype.Add (Floor, otd.om_floor.rawname_bytype); // Отделка пола
            paramnamebytype.Add (Ceil,
                                 otd.om_ceil.rawname_bytype); // Отделка потолка
            OtdData_WriteToRoom (
                summary.columnFormat, otd.zone_guid, summary.paramToWrite, paramToRead, dct, paramnamebytype);
        }
    }

    // Вырезы контакта представлены проёмами; legacy-проёмы повторно не добавляются.
    static bool AddSurfaceFinishPiece (OtdRoom &room, const RoomSurfacePiece &piece, RoomSurfaceCache *cache) {
        if (piece.status != RoomSurfacePieceStatus::Matched || piece.baseGuid == APINULLGuid)
            return false;
        GS::Array<OtdWall> walls;
        if (BuildSurfaceWalls (piece, room.floorInd, walls)) {
            ElementRead baseRead;
            if (piece.baseType == API_WallID) {
                baseRead = ReadElement (piece.baseGuid, cache);
                if (baseRead.error != NoError)
                    return false;
            }
            const API_Element &base = baseRead.element;
            for (auto &wall : walls) {
                if (piece.baseType == API_WallID) {
                    const double dx = base.wall.endC.x - base.wall.begC.x;
                    const double dy = base.wall.endC.y - base.wall.begC.y;
                    const double wx = wall.endC.x - wall.begC.x;
                    const double wy = wall.endC.y - wall.begC.y;
                    // Торец не имеет стороны многослойки; боковая грань использует исходное направление.
                    if (std::fabs (dx * wy - dy * wx) > 1e-6 * std::hypot (dx, dy) * std::hypot (wx, wy))
                        continue; // Из-за одной ошибки нельзя прерывать всю работу
                    wall.base_flipped = (dx * wx + dy * wy < 0) != base.wall.flipped;
                    wall.base_th = base.wall.thickness;
                }
            }
            room.otdwall.Append (walls);
            return true;
        }
        GS::Array<OtdSlab> slabs;
        if (!BuildSurfaceSlabs (piece, room.floorInd, slabs))
            return false;
        for (auto &slab : slabs) {
            if ((slab.type == Floor && !room.has_floor) || (slab.type == Ceil && !room.has_ceil))
                continue;
            slab.material = slab.type == Floor ? room.om_floor : room.om_ceil;
            room.otdslab.Push (std::move (slab));
        }
        return true;
    }

    // Подготавливает геометрию для прежних материальных правил и writers; элементы здесь не создаются.
    static bool BuildRoomSurfaceFinishes (OtdRoom &room, RoomSurfaceCache *cache) {
        room.otdwall.Clear ();
        room.otdslab.Clear ();
        Int32 skipped = 0;
        double skippedArea = 0;
        GS::Array<OtdSlab> slabFinishes;
        auto slabSources = room.slabSources;
        auto floors = room.floorslab;
        auto ceilings = room.ceilslab;
        // Точные slab-контакты сохраняются независимо от legacy-проверки номинальных отметок.
        for (const auto &piece : room.surfacePartition.pieces) {
            if (piece.status != RoomSurfacePieceStatus::Matched || piece.baseType != API_SlabID ||
                std::hypot (piece.normal.x, piece.normal.y) > 1e-6 || std::fabs (piece.normal.z) < 1 - 1e-6)
                continue;
            auto &roles = piece.normal.z < 0 ? floors : ceilings;
            bool hasRole = false;
            for (const auto &guid : roles)
                hasRole = hasRole || guid == piece.baseGuid;
            if (!hasRole)
                roles.Push (piece.baseGuid);
            bool hasSource = false;
            for (const auto &source : slabSources)
                hasSource = hasSource || source.baseGuid == piece.baseGuid;
            if (!hasSource)
                for (const auto &source : room.surfaceCandidates.sources)
                    if (source.baseGuid == piece.baseGuid && source.baseType == API_SlabID)
                        slabSources.Push (source);
        }
        OtdSlab nominalFloor = room.poly;
        nominalFloor.zBottom = room.zBottom;
        nominalFloor.material = room.om_floor;
        OtdSlab nominalCeiling = room.poly;
        nominalCeiling.zBottom = room.zBottom + room.height_ceil;
        nominalCeiling.material = room.om_ceil;
        double rejectedSlabArea = 0;
        if ((room.has_floor && !BuildProjectedSlabFinishes (room.surfaceSnapshot,
                                                            slabSources,
                                                            floors,
                                                            Floor,
                                                            room.floorInd,
                                                            slabFinishes,
                                                            rejectedSlabArea,
                                                            &nominalFloor,
                                                            room.floor_by_slab,
                                                            &room.otdwall,
                                                            &room.preparedSlabs)) ||
            (room.has_ceil && !BuildProjectedSlabFinishes (room.surfaceSnapshot,
                                                           slabSources,
                                                           ceilings,
                                                           Ceil,
                                                           room.floorInd,
                                                           slabFinishes,
                                                           rejectedSlabArea,
                                                           &nominalCeiling,
                                                           room.ceil_by_slab,
                                                           &room.otdwall,
                                                           &room.preparedSlabs))) {
            msg_rep ("Surface finish", "Invalid original slab projection", APIERR_BADPARS, room.zone_guid);
            return false;
        }
        if (rejectedSlabArea > 1e-6)
            msg_rep ("Surface finish", "Ambiguous projected slab ownership skipped", APIERR_BADPARS, room.zone_guid);
        for (auto &slab : slabFinishes) {
            slab.material = slab.type == Floor ? room.om_floor : room.om_ceil;
            room.otdslab.Push (std::move (slab));
        }
        // Убираем лишь внутренние швы совместимых контактов; исходный partition сохраняет provenance.
        GS::Array<RoomSurfacePiece> grouped;
        if (!GroupSurfaceWallPieces (room.surfacePartition.pieces, room.floorInd, grouped)) {
            msg_rep ("Surface finish", "Invalid coplanar wall union", APIERR_BADPARS, room.zone_guid);
            return false;
        }
        for (const auto &piece : grouped) {
            // beamPart — единственный маршрут балок: не дублируем совпавшие грани partition.
            if (piece.baseType == API_BeamID)
                continue;
            // Только горизонтальные slab-контакты заменены проекционным маршрутом.
            if (piece.baseType == API_SlabID && std::hypot (piece.normal.x, piece.normal.y) <= 1e-6)
                continue;
            if (!AddSurfaceFinishPiece (room, piece, cache)) {
                ++skipped;
                skippedArea += piece.area;
            }
        }
        // Сверка выполняется после сбора всех граней: соседние куски задают внутренние разрывы.
        GS::Array<OtdWall> correctedWalls;
        for (const auto &wall : room.otdwall) {
            if (wall.base_type != API_WallID || !wall.surface_geometry || wall.slab_step) {
                correctedWalls.Push (wall);
                continue;
            }
            const auto base = ReadElement (wall.base_guid, cache);
            if (base.error != NoError)
                continue; // Из-за одной ошибки нельзя прерывать всю работу
            // Для дуги tBeg/tEnd нельзя интерпретировать как расстояния по хорде.
            if (base.element.wall.zoneRel == APIZRel_None || base.element.wall.angle != 0) {
                correctedWalls.Push (wall);
                continue;
            }
            const auto corrected = ClampSurfaceWallToParts (wall,
                                                            base.element.wall.begC,
                                                            base.element.wall.endC,
                                                            room.wallPart,
                                                            room.edges,
                                                            room.zBottom,
                                                            room.otdwall);
            if (corrected.IsEmpty ()) {
                ++skipped;
                skippedArea +=
                    wall.length * wall.height * std::sqrt (1 + wall.rise_x * wall.rise_x + wall.rise_y * wall.rise_y);
                msg_rep ("Surface finish", "WallPart contact rejected", APIERR_BADPARS, wall.base_guid);
            }
            correctedWalls.Append (corrected);
        }
        room.otdwall = std::move (correctedWalls);
        for (const auto &part : room.surfaceCandidates.beamParts) {
            const RoomSurfaceSource *source = nullptr;
            Int32 sourceCount = 0;
            for (const auto &candidate : room.surfaceCandidates.sources) {
                if (candidate.baseType == API_BeamID && candidate.baseGuid == part.guid) {
                    source = &candidate;
                    ++sourceCount;
                }
            }
            // Многосегментная beamPart пока не подтверждена; не выбираем первый сегмент.
            if (sourceCount != 1) {
                msg_rep ("Surface finish", "Unsupported beam segment ownership", APIERR_BADPARS, part.guid);
                continue; // Из-за одной ошибки нельзя прерывать всю работу: другие beamParts могут быть корректны.
            }
            const ElementRead beamRead = ReadElement (part.guid, cache);
            if (beamRead.error != NoError)
                continue; // Из-за одной ошибки нельзя прерывать всю работу
            const API_Element &beam = beamRead.element;
            RoomSurfaceBeamPartitions result;
            const auto finish = BuildRoomBeamFinish (*source,
                                                     beam.beam.begC,
                                                     beam.beam.endC,
                                                     part.tBeg,
                                                     part.tEnd,
                                                     room.zBottom,
                                                     room.zBottom + room.height,
                                                     result,
                                                     &room.surfaceSnapshot);
            if (!result.errors.IsEmpty ()) {
                msg_rep ("Surface finish", "Unsupported beam geometry", APIERR_BADPARS, part.guid);
                continue; // Из-за одной ошибки нельзя прерывать всю работу
            }
            for (const auto &beamPiece : finish.pieces) {
                RoomSurfacePiece piece;
                piece.status = RoomSurfacePieceStatus::Matched;
                piece.baseGuid = part.guid;
                piece.baseType = API_BeamID;
                piece.area = beamPiece.area;
                RoomSurfacePieceContour contour;
                contour.vertices = beamPiece.vertices;
                piece.contours.Push (contour);
                bool found = false;
                for (const auto &body : source->snapshot.bodies) {
                    for (const auto &face : body.surfaces) {
                        bool onPlane = !beamPiece.vertices.IsEmpty ();
                        for (const auto &p : beamPiece.vertices)
                            onPlane = onPlane && std::fabs (face.normalWorld.x * p.x + face.normalWorld.y * p.y +
                                                            face.normalWorld.z * p.z + face.planeOffset) <= 1e-6;
                        if (!onPlane)
                            continue;
                        // Синтез beamPart допустим только для доказанной прямоугольной исходной грани.
                        RoomSurfacePiece original;
                        original.normal = face.normalWorld;
                        original.planeOffset = face.planeOffset;
                        for (const auto &c : face.contours) {
                            RoomSurfacePieceContour own;
                            own.hole = c.hole;
                            for (const auto &v : c.vertices)
                                own.vertices.Push (v.world);
                            original.contours.Push (own);
                        }
                        if (original.contours.GetSize () != 1 || original.contours[0].vertices.GetSize () != 4)
                            continue;
                        const auto &v = original.contours[0].vertices;
                        const double ax = v[1].x - v[0].x, ay = v[1].y - v[0].y, az = v[1].z - v[0].z;
                        const double bx = v[3].x - v[0].x, by = v[3].y - v[0].y, bz = v[3].z - v[0].z;
                        original.area = std::sqrt ((ay * bz - az * by) * (ay * bz - az * by) +
                                                   (az * bx - ax * bz) * (az * bx - ax * bz) +
                                                   (ax * by - ay * bx) * (ax * by - ay * bx));
                        OtdWall testWall;
                        GS::Array<OtdSlab> testSlabs;
                        if (!BuildSurfaceWall (original, room.floorInd, testWall) &&
                            !BuildSurfaceSlabs (original, room.floorInd, testSlabs))
                            continue;
                        piece.normal = {-face.normalWorld.x, -face.normalWorld.y, -face.normalWorld.z};
                        piece.planeOffset = -face.planeOffset;
                        found = true;
                        break;
                    }
                    if (found)
                        break;
                }
                if (!found || !AddSurfaceFinishPiece (room, piece, cache)) {
                    ++skipped;
                    skippedArea += piece.area;
                }
            }
        }
        if (skipped != 0)
            msg_rep (
                "Surface finish",
                GS::UniString::Printf ("Skipped %d unresolved/unsupported pieces, area %.6f", skipped, skippedArea),
                NoError,
                room.zone_guid);
    #if defined(TESTING)
        DBprnt (static_cast<double> (room.otdwall.GetSize ()), "Surface finish walls before rules");
        DBprnt (static_cast<double> (room.otdslab.GetSize ()), "Surface finish slabs before rules");
    #endif
        return true;
    }

    // Для комнат с прочитанными параметрами рассчитывает полы, потолки, стены и откосы, пропуская недопустимые данные.
    // Использует отдельную копию параметров каждой комнаты; при отмене возвращает false.
    static bool ProcessRoomFinishes (RoomProcessingContext &context,
                                     ReadParamsForRoomBook &readParams,
                                     ParamDictCompositeElement &paramCompositeToRead,
                                     MatarialToFavoriteDict &favdict,
                                     bool has_base_element,
                                     GS::UniString &funcname) {
        auto &roomParams = readParams.roomParams;
        // На этом этапе уже рассчитываются фактические отделочные элементы для каждой комнаты.
        funcname = GS::UniString::Printf ("Calculate finising elements for %d room(s)", context.roomsinfo.GetSize ());
        GS::HashTable<GS::UniString, GS::Int32> material_dict; // Словарь индексов покрытий
        for (OtdRooms::PairIterator cIt = context.roomsinfo.EnumeratePairs (); cIt != NULL; ++cIt) {
    #ifdef ServerMainVers_2800
            OtdRoom &otd = cIt->value;
            API_Guid zoneGuid = cIt->key;
    #else
            OtdRoom &otd = *cIt->value;
            API_Guid zoneGuid = *cIt->key;
    #endif
            if (AdvanceProcessPhase (funcname, nPhase))
                return false;
            if (!context.paramToRead.ContainsKey (zoneGuid))
                continue; // Если у зоны нет прочитанных параметров - дальше делать
                          // нечего
            // Заполняем данные для зон
            // Копия на каждую зону: Param_Property_Read пропускает параметры с уже
            // выставленным isValid и не перечитывает val — общая копия тянула бы
            // значения первой зоны во все остальные.
            ReadParams roomParamsWork = roomParams;
            Param_SetToRooms (material_dict, otd, context.paramToRead, roomParamsWork);
            if (!otd.isValid)
                continue;
            // Сохраняем исходную привязку проёмов до замены стен поверхностными.
            GS::Array<OtdWall> openingWalls;
            for (const auto &wall : otd.otdwall) {
                openingWalls.Push (wall); // Все legacy-участки нужны также для стороны полного состава #268.
            }
            if (!BuildRoomSurfaceFinishes (otd, &context.surfaceCache))
                continue; // Из-за одной ошибки нельзя прерывать всю работу
            if (otd.otdwall.IsEmpty () && otd.otdslab.IsEmpty () && openingWalls.IsEmpty ())
                continue;
            ProcessSlabFinishes (otd,
                                 zoneGuid,
                                 context.paramToRead,
                                 context.param_composite,
                                 context.paramDict_favorite,
                                 context.exsistot_byzone,
                                 paramCompositeToRead,
                                 favdict,
                                 has_base_element);
            // Откосы нужны и при отсутствии представимых основных surface-стен.
            if (otd.otdwall.IsEmpty () && openingWalls.IsEmpty ())
                continue;
            ProcessWallFinishes (otd,
                                 openingWalls,
                                 context.paramToRead,
                                 context.param_composite,
                                 context.paramDict_favorite,
                                 readParams.windowParams,
                                 paramCompositeToRead,
                                 favdict);
            if (!ExtendFloorContoursToCore (otd))
                msg_rep ("Floor core extension",
                         "Contour topology changed; original floor retained",
                         APIERR_BADPARS,
                         zoneGuid);
            if (!DeduplicateAcceptedSlabContacts (otd))
                msg_rep ("Surface finish",
                         "Accepted slab side remainder is ambiguous or unrepresentable",
                         APIERR_BADPARS,
                         zoneGuid);
        } // Обработка зон
        return true;
    }

    // -----------------------------------------------------------------------------
    // Собирает оставшиеся GUID существующей отделки в deletelist; само удаление здесь не выполняется.
    static void RemoveUnusedFinishingElements (GS::HashTable<API_Guid, UnicGuidByBase> &exsistot_byzone,
                                               GS::Array<API_Guid> &deletelist) {
        // Неиспользованные существующие элементы удаляем
        for (GS::HashTable<API_Guid, UnicGuidByBase>::PairIterator cIt_1 = exsistot_byzone.EnumeratePairs ();
             cIt_1 != NULL;
             ++cIt_1) {
    #ifdef ServerMainVers_2800
            const UnicGuidByBase &byzone = cIt_1->value;
    #else
            const UnicGuidByBase &byzone = *cIt_1->value;
    #endif
            for (UnicGuidByBase::ConstPairIterator cIt_21 = byzone.EnumeratePairs (); cIt_21 != NULL; ++cIt_21) {
    #ifdef ServerMainVers_2800
                const UnicGuidByTypeOtd &bytype = cIt_21->value;
    #else
                const UnicGuidByTypeOtd &bytype = *cIt_21->value;
    #endif
                for (UnicGuidByTypeOtd::ConstPairIterator cIt_2 = bytype.EnumeratePairs (); cIt_2 != NULL; ++cIt_2) {
    #ifdef ServerMainVers_2800
                    const UnicGuid &byparent = cIt_2->value;
    #else
                    const UnicGuid &byparent = *cIt_2->value;
    #endif
                    for (UnicGuid::ConstPairIterator cIt_3 = byparent.EnumeratePairs (); cIt_3 != NULL; ++cIt_3) {
    #ifdef ServerMainVers_2800
                        API_Guid guid = cIt_3->key;
    #else
                        API_Guid guid = *cIt_3->key;
    #endif
                        deletelist.Push (guid);
                    }
                }
            }
        }
    }

    // Запись в зону информации об отделке
    // -----------------------------------------------------------------------------
    // REFACTOR PLAN FOR NEXT AGENT:
    // 1. Extract GetTargetZones() from the selection / fallback-to-all-zones block.
    //    Keep the filter flags and error handling unchanged.
    // 2. Extract PrepareRoomProcessingContext() for:
    //    - finclass / finclassguids
    //    - storyLevels
    //    - roomsinfo / elementToRead / param containers
    //    - slabsinzone
    // 3. Extract BuildElementReadIndex() and ProcessElementsForRoomData() for the loop that
    //    creates openings, walls and slabs from the collected elements.
    // 4. Extract PrepareReadParams() and ReadElementParameters() for the parameter preparation loop.
    // 5. Extract ProcessRoomFinishes() and split it into smaller helpers:
    //    - ProcessSlabFinishes()
    //    - ProcessWallFinishes()
    //    - ApplyFavoriteAndMaterialData()
    // 6. Extract BuildMaterialSummaryForRooms() and WriteRoomMaterialData() from the section that
    //    calculates material areas and writes them back to the rooms.
    // 7. Extract RemoveUnusedFinishingElements() and PrepareElementsForUpdate() from the cleanup block.
    // 8. After each extraction, keep behavior identical and verify the build.
    //    The goal is to make RoomBook() a thin orchestrator with small single-purpose helpers.

    // Ищет комбинацию слоёв с именем, заканчивающимся на smstf_fin, и применяет её при наличии.
    bool ApplyFinishLayerCombinationIfExists () {
        GS::Array<API_Attribute> layerCombinations;
    #ifdef ServerMainVers_2700
        if (ACAPI_Attribute_GetAttributesByType (API_LayerCombID, layerCombinations) != NoError)
            return false;
    #else
        if (ParamHelpers::ACAPI_Attribute_GetAttributesByType (API_LayerCombID, layerCombinations) != NoError)
            return false;
    #endif

        for (const API_Attribute &attr : layerCombinations) {
            GS::UniString name (attr.header.name);
            name.SetToLowerCase ();
            if (name.EndsWith ("smstf_fin")) {
                API_AttributeIndex targetComb = attr.header.index;
    #ifdef ServerMainVers_2700
                if (targetComb.IsPositive ()) {
                    GSErrCode err = ACAPI_Navigator_ChangeCurrLayerComb (&targetComb);
                    return (err == NoError);
                }
    #else
                if (targetComb != 0) {
                    GSErrCode err = ACAPI_Environment (APIEnv_ChangeCurrLayerCombID, &targetComb);
                    return (err == NoError);
                }
    #endif
            }
        }
        return false;
    }

    // Собирает данные зон и конструкций, рассчитывает отделку и подготавливает результаты по материалам.
    // Проверяет классификации и доступность существующей отделки, затем вызывает отрисовку и запись связей; при ошибках
    // или отмене выходит.
    void RoomBook () {
        clock_t start, finish;
        double duration;
        start = clock ();
        GS::UniString funcname ("RoomBook");
        nPhase = 1;
        // Окно прогресса помогает видеть, на каком этапе выполняется расчёт отделки.
        ProcessWindowGuard pwGuard (funcname, nPhase);

        ApplyFinishLayerCombinationIfExists ();

        GS::Array<API_Guid> zones;
        if (!GetTargetZones (zones))
            return;

        funcname = GS::UniString::Printf ("Collect info from %d room(s)", zones.GetSize ());
        // Контейнеры остаются общими для всех этапов; ссылки сохраняют текущие места их использования.
        RoomProcessingContext context;
        PrepareRoomProcessingContext (context, zones);
        // После сбора связей временные GUID зон очищаются перед обработкой базовых элементов.
        if (!BuildElementReadIndex (zones, context, funcname))
            return;
        if (!ProcessElementsForRoomData (context, funcname) || !PrepareRoomPartitions (context))
            return;
        // Необходимые для чтения параметры и свойства
        ReadParamsForRoomBook readParams = PrepareReadParams ();
        ReadElementParameters (context, readParams);
        funcname = GS::UniString::Printf ("Read data from %d elements(s)", context.paramToRead.GetSize ());
        if (AdvanceProcessPhase (funcname, nPhase))
            return;
        // Читаем свойства всех элементов
        ParamDictCompositeElement paramCompositeToRead;
        ListData::LibElements paramListDataToRead;
        ParamHelpers::ElementsRead (context.paramToRead, paramCompositeToRead, paramListDataToRead, true, false);
        // Словарь избранного нужен для выбора подходящего шаблона отделки по материалу и типу поверхности.
        MatarialToFavoriteDict favdict = Favorite_GetDict ();
        // Ищём уже существующие элементы отделки и определяем, к каким базовым элементам они привязаны.
        // Это нужно, чтобы не создавать дубли и правильно обновлять существующие элементы.
        bool has_base_element = false;
        UnicGuid reserv_elements; // Словарь незарезервированных или скрытых элементов
        context.exsistot_byzone = Otd_GetOtd_ByZone (
            zones, context.finclassguids, context.finclass, has_base_element, reserv_elements, &context.surfaceCache);
        if (!ProcessRoomFinishes (context, readParams, paramCompositeToRead, favdict, has_base_element, funcname))
            return;
        // Получаем список уже существующих отделочных элементов для обработанных зон.
        // Он понадобится для последующего удаления устаревших объектов.
        zones.Clear ();
        context.paramDict_favorite.Clear ();
        favdict.Clear ();
        MaterialSummary summary;
        if (!BuildMaterialSummaryForRooms (context.roomsinfo, context.paramToRead, zones, summary, funcname))
            return;
        WriteRoomMaterialData (context.roomsinfo, context.paramToRead, summary);
        auto &paramToWrite = summary.paramToWrite;
        context.paramToRead.Clear ();
        // Проверка существования классов и свойств
        if (!zones.IsEmpty ()) {
            if (!Check (context.finclass, context.finclassguids))
                return;
        }
        Draw_Elements (context.exsistot_byzone,
                       context.deletelist,
                       context.roomsinfo,
                       context.storyLevels,
                       context.finclass,
                       reserv_elements,
                       paramToWrite);
        finish = clock ();
        duration = (double)(finish - start) / CLOCKS_PER_SEC;
        GS::UniString time = GS::UniString::Printf ("Calculate complete for %d room(s) by", zones.GetSize ()) +
                             GS::UniString::Printf (" %.3f s", duration);
        msg_rep ("RoomBook", time, NoError, APINULLGuid);
    }

    // Floor/Ceil finishes also use native walls for slab steps and nonhorizontal contacts.
    bool IsExistingFinishNativeType (TypeOtd type_otd, API_ElemTypeID type_subguid) {
        bool correct_type = true;
        switch (type_otd) {
        case Wall_Main:
        case Wall_Up:
        case Wall_Down:
        case Column:
            correct_type = (type_subguid == API_WallID || type_subguid == API_ObjectID);
            break;
        case Reveal_Main:
        case Reveal_Up:
        case Reveal_Down:
            correct_type = (type_subguid == API_WallID || type_subguid == API_ObjectID || type_subguid == API_BeamID);
            break;
        case Floor:
            correct_type = (type_subguid == API_SlabID || type_subguid == API_ObjectID || type_subguid == API_WallID);
            break;
        case Ceil:
            correct_type = (type_subguid == API_SlabID || type_subguid == API_ObjectID || type_subguid == API_WallID);
            break;
        default:
            break;
        }
        return correct_type;
    }

    // Находит существующую отделку по Sync_GUID zone и группирует её по зоне, основанию и типу.
    // Одновременно собирает GUID для проверки резервирования существующих элементов.
    GS::HashTable<API_Guid, UnicGuidByBase> Otd_GetOtd_ByZone (const GS::Array<API_Guid> &zones,
                                                               const UnicGuid &finclassguids,
                                                               const ClassificationFunc::ClassificationDict &finclass,
                                                               bool &has_base_element,
                                                               UnicGuid &reserv_elements,
                                                               RoomSurfaceCache *cache) {
        // Существующие элементы храним в словаре двойной вложенности
        //  Первый уровень - ключ API_Guid зоны
        //  Второй ключ - GUID базового элемента (к которому привязана отделка)
        //                для элементов без базовой конструкции (потолки, полы) там
        //                будет guid зоны
        //  Далее уже идёт словарь с GUID существующих элементов отделки
        GS::HashTable<API_Guid, UnicGuidByBase> exsistot_byzone;
        // Поиск существующих элементов отделки
        UnicGuidByGuid exsistotdelements; // Словарь существующих отделочных элементов с разбивкой по зонам
        int errcode = 0;
        // У элементов отделки есть свойство "Sync_GUID zone", которое указывает на GUID зоны, к которой они привязаны.
        // С помощью этого свойства мы ищем все существующие элементы отделки, чтобы не создавать дубли и правильно
        // обновлять их.
        if (!SyncGetParentelement (zones, exsistotdelements, "zone", errcode)) {
    #if defined(TESTING)
            // Диагностика #193: существующие элементы отделки не найдены через Sync_GUID zone
            DBprnt ("Otd_GetOtd_ByZone",
                    "SyncGetParentelement FAILED, errcode=" + GS::UniString::Printf ("%d", errcode));
    #endif
            return exsistot_byzone;
        }
    #if defined(TESTING)
        // Диагностика #193: сколько зон и элементов отделки найдено по Sync_GUID zone
        DBprnt ("Otd_GetOtd_ByZone",
                "SyncGetParentelement OK: zones=" + GS::UniString::Printf ("%u found_otd_zones=%u",
                                                                           (UInt32)zones.GetSize (),
                                                                           (UInt32)exsistotdelements.GetSize ()));
    #endif
        for (UnicGuidByGuid::PairIterator cIt = exsistotdelements.EnumeratePairs (); cIt != NULL; ++cIt) {
    #ifdef ServerMainVers_2800
            const UnicGuid &guids = cIt->value;
            API_Guid zoneguid = cIt->key;
    #else
            const UnicGuid &guids = *cIt->value;
            API_Guid zoneguid = *cIt->key;
    #endif
            GS::HashTable<API_Guid, TypeOtd> otd_elements;
            // Список всех элементов отделки
    #if defined(TESTING)
            // Диагностика #193: почему найденные элементы не попадают в otd_elements
            UInt32 diag_guids = 0, diag_class_ok = 0, diag_type_fail = 0;
    #endif
            for (UnicGuid::ConstPairIterator cItt = guids.EnumeratePairs (); cItt != NULL; ++cItt) {
    #ifdef ServerMainVers_2800
                API_Guid guid = cItt->key;
                bool isvisible = cItt->value;
    #else
                API_Guid guid = *cItt->key;
                bool isvisible = *cItt->value;
    #endif
    #if defined(TESTING)
                diag_guids += 1;
    #endif
                API_Guid classguid;
                if (Class_IsElementFinClass (guid, finclassguids, classguid, cache)) {
    #if defined(TESTING)
                    diag_class_ok += 1;
    #endif
                    TypeOtd type_otd = Class_GetOtdTypeByClass (classguid, finclass);
                    API_ElemTypeID type_subguid = GetElemTypeID (guid);
                    const bool correct_type = IsExistingFinishNativeType (type_otd, type_subguid);
                    if (!otd_elements.ContainsKey (guid) && correct_type)
                        otd_elements.Add (guid, type_otd);
    #if defined(TESTING)
                    if (!correct_type)
                        diag_type_fail += 1;
    #endif
                    if (!reserv_elements.ContainsKey (guid)) {
                        if (!ACAPI_Element_Filter (guid, APIFilt_InMyWorkspace)) {
                            reserv_elements.Add (guid, true);
                        }
                    }
                }
            }
            if (exsistot_byzone.ContainsKey (zoneguid))
                continue;
    #if defined(TESTING)
            // Диагностика #193: сводка по зоне — сколько элементов прошло фильтры
            DBprnt ("Otd_GetOtd_ByZone",
                    GS::UniString::Printf ("zone guids=%u class_ok=%u type_fail=%u otd_elements=%u",
                                           diag_guids,
                                           diag_class_ok,
                                           diag_type_fail,
                                           (UInt32)otd_elements.GetSize ()));
    #endif
            UnicGuidByBase exsistot_byparent = Otd_GetOtd_Parent (otd_elements, has_base_element);
            exsistot_byzone.Add (zoneguid, exsistot_byparent);
        }
        return exsistot_byzone;
    }

    // Группирует известную отделку по GUID базового элемента и типу, пропуская неизвестные дочерние GUID.
    // При добавлении элемента устанавливает has_base_element, не сбрасывая прежнее значение.
    UnicGuidByBase BuildOtdByParent (GS::HashTable<API_Guid, TypeOtd> &otd_elements,
                                     UnicGuidByGuid &parentdict,
                                     bool &has_base_element) {
        UnicGuidByBase exsistot_byparent;
        for (GS::HashTable<API_Guid, UnicGuid>::PairIterator cIt = parentdict.EnumeratePairs (); cIt != NULL; ++cIt) {
    #ifdef ServerMainVers_2800
            const API_Guid &parentguid = cIt->key;
            UnicGuid &subelements = cIt->value;
    #else
            const API_Guid &parentguid = *cIt->key;
            UnicGuid &subelements = *cIt->value;
    #endif
            for (UnicGuid::PairIterator cItt = subelements.EnumeratePairs (); cItt != NULL; ++cItt) {
    #ifdef ServerMainVers_2800
                const API_Guid &subguid = cItt->key;
    #else
                const API_Guid &subguid = *cItt->key;
    #endif
                const TypeOtd *typePtr = otd_elements.GetPtr (subguid);
                if (typePtr == nullptr)
                    continue;
                const TypeOtd type = *typePtr;
                UnicGuidByTypeOtd *parentMapPtr = exsistot_byparent.GetPtr (parentguid);
                if (parentMapPtr == nullptr) {
                    exsistot_byparent.Add (parentguid, UnicGuidByTypeOtd{});
                    parentMapPtr = exsistot_byparent.GetPtr (parentguid);
                }
                UnicGuid *subdictPtr = parentMapPtr->GetPtr (type);
                if (subdictPtr == nullptr) {
                    parentMapPtr->Add (type, UnicGuid{});
                    subdictPtr = parentMapPtr->GetPtr (type);
                }
                if (!subdictPtr->ContainsKey (subguid)) {
                    subdictPtr->Add (subguid, true);
                    has_base_element = true;
                }
            }
        }
        return exsistot_byparent;
    }

    // Читает связи отделки с основаниями и строит индекс по GUID основания и типу.
    // При отказе чтения помещает известную отделку под APINULLGuid без установки has_base_element.
    UnicGuidByBase Otd_GetOtd_Parent (GS::HashTable<API_Guid, TypeOtd> &otd_elements, bool &has_base_element) {
        int errcode = 0;
        UnicGuidByBase exsistot_byparent;
        UnicGuidByGuid parentdict; // Для считывания элементов
        GS::Array<API_Guid> otd_els;
        for (GS::HashTable<API_Guid, TypeOtd>::PairIterator cIt = otd_elements.EnumeratePairs (); cIt != NULL; ++cIt) {
    #ifdef ServerMainVers_2800
            API_Guid elems = cIt->key;
    #else
            API_Guid elems = *cIt->key;
    #endif
            otd_els.Push (elems);
        }
        // У элементов отделки есть свойство "Sync_GUID base element", которое указывает на базовую конструкцию (стена,
        // колонна, плита), к которой привязана отделка. По этому свойству привязываем существующую отделку к базовым
        // элементам.
        if (!SyncGetSubelement (otd_els, parentdict, "base element", errcode)) {
            API_Guid parentguid = APINULLGuid;
            UnicGuidByTypeOtd *parentMapPtr = exsistot_byparent.GetPtr (parentguid);
            if (parentMapPtr == nullptr) {
                exsistot_byparent.Add (parentguid, UnicGuidByTypeOtd{});
                parentMapPtr = exsistot_byparent.GetPtr (parentguid);
            }
            UnicGuidByTypeOtd &parentMap = *parentMapPtr;
            for (const API_Guid &subguid : otd_els) {
                const TypeOtd *typePtr = otd_elements.GetPtr (subguid);
                if (typePtr == nullptr) {
                    continue;
                }
                TypeOtd t = *typePtr;
                UnicGuid *subdictPtr = parentMap.GetPtr (t);
                if (subdictPtr == nullptr) {
                    parentMap.Add (t, UnicGuid{});
                    subdictPtr = parentMap.GetPtr (t);
                }
                UnicGuid &subdict = *subdictPtr;
                subdict.Add (subguid, true);
            }
            return exsistot_byparent;
        }
        return BuildOtdByParent (otd_elements, parentdict, has_base_element);
    }

    // Настройки для форматирования текста в таблицу
    void OtdData_GetColumnfFormat (GS::UniString descripton,
                                   const GS::UniString &rawname,
                                   ColumnFormatDict &columnFormat) {
        GS::UniString fontname = "GOST 2.304 type A";
        ColumnFormat c;
        if (descripton.Contains (BRACESTART) && descripton.Contains (BRACEEND) && descripton.Contains (SEMICOLON)) {
            descripton = descripton.GetSubstring (CHARBRACESTART, CHARBRACEEND, 0);
            GS::Array<GS::UniString> partstring;
            UInt32 n = StringSplt (descripton, SEMICOLON, partstring, false);
            if (n > 0) {
                fontname = "Arial";
                if (!UniStringToDouble (partstring[0], c.width_mat))
                    c.width_mat = 50;
            }
            if (n > 1) {
                if (!UniStringToDouble (partstring[1], c.width_area))
                    c.width_area = 20;
            }
            if (n > 2) {
                if (!UniStringToDouble (partstring[2], c.fontsize))
                    c.fontsize = 2.5;
            }
            if (n > 3) {
                fontname = partstring[3];
            }
            if (n > 4) {
                GS::UniString space = UniStringToLower (partstring[4]);
                if (space.Contains ("nbs"))
                    space = reinterpret_cast<const char *> (u8"\u2007");
                if (space.Contains ("ns"))
                    space = reinterpret_cast<const char *> (u8"\u202F");
                if (space.Contains ("s"))
                    space = SPACESTRING;
                c.no_breake_space = space;
            }
            if (n > 5) {
                GS::UniString space = UniStringToLower (partstring[5]);
                if (space.Contains ("nbs"))
                    space = reinterpret_cast<const char *> (u8"\u2007");
                if (space.Contains ("ns"))
                    space = reinterpret_cast<const char *> (u8"\u202F");
                if (space.Contains ("s"))
                    space = SPACESTRING;
                c.narow_space = space;
            }
        }
        c.font = GetFontIndex (fontname);
        if (c.font == 1) {
            fontname = "Arial";
            c.font = GetFontIndex (fontname);
        }
        Int32 addspace = 0;
        GS::UniString delim = "-";
        c.width_no_breake_space = GetTextWidth (c.font, c.fontsize, c.no_breake_space);
        if (c.width_no_breake_space < 0.001 || is_equal (c.width_no_breake_space, 0)) {
            c.no_breake_space = SPACESTRING;
            c.width_no_breake_space = GetTextWidth (c.font, c.fontsize, c.no_breake_space);
        }
        if (c.width_no_breake_space < 0.001) {
            c.width_no_breake_space = 0.5;
        }
        c.width_narow_space = GetTextWidth (c.font, c.fontsize, c.narow_space);
        if (c.width_narow_space < 0.001 || is_equal (c.width_narow_space, 0)) {
            c.narow_space = SPACESTRING;
            c.width_narow_space = GetTextWidth (c.font, c.fontsize, c.narow_space);
        }
        if (c.width_narow_space < 0.001) {
            c.width_narow_space = 0.5;
        }
        addspace = DoubleToInt32 (c.width_area / c.width_narow_space, "Roombook", "число пробелов по ширине");
        if (fabs (addspace * c.width_narow_space - c.width_area) > c.width_narow_space)
            addspace -= 1;
        c.space_line = EMPTYSTRING;
        for (Int32 j = 0; j < addspace; j++) {
            c.space_line.Append (c.narow_space);
        }
        c.width_area = GetTextWidth (c.font, c.fontsize, c.space_line);
        double width_delim = GetTextWidth (c.font, c.fontsize, delim);
        addspace =
            DoubleToInt32 ((c.width_mat + c.width_area) / width_delim, "Roombook", "число разделителей по ширине");
        if (fabs (addspace * width_delim - c.width_mat - c.width_area) > c.width_narow_space)
            addspace -= 1;
        c.delim_line = EMPTYSTRING;
        for (Int32 j = 0; j < addspace; j++) {
            c.delim_line.Append (delim);
        }
        c.width_mat = GetTextWidth (c.font, c.fontsize, c.delim_line) - c.width_area;
        c.delim_line.Append (SPACESTRING);
        c.space_line.Append (SPACESTRING);
        columnFormat.Add (rawname, c);
    }

    // Накапливает площади материалов валидных стен и перекрытий комнаты в словаре по типам отделки.
    // Имена слоёв разделяет по точке с запятой; переданный словарь не очищает.
    static void CollectMaterialAreasForRoom (const OtdRoom &otd, OtdMaterialAreaDictByType &dct) {
        // Разбивка по отделочным слоям, вычисление площадей и добавление их в
        // словарь по типам отделки
        for (const OtdWall &otdw : otd.otdwall) {
            if (!otdw.isValid)
                continue;
            const TypeOtd &t = otdw.type;
            const double area = OtdWall_GetArea (otdw);
            for (UInt32 j = 0; j < otdw.base_composite.GetSize (); j++) {
                GS::UniString mat = otdw.base_composite[j].val;
                if (mat.Contains (SEMICOLON)) {
                    GS::Array<GS::UniString> partstring;
                    UInt32 n = StringSpltUnic (mat, SEMICOLON, partstring);
                    if (n < 1) {
                        OtdData_AddValueToDict (dct, t, mat, area);
                    } else {
                        for (const GS::UniString &mat_ : partstring) {
                            OtdData_AddValueToDict (dct, t, mat_, area);
                        }
                    }
                } else {
                    OtdData_AddValueToDict (dct, t, mat, area);
                }
            }
        }

        for (const OtdSlab &otdslab : otd.otdslab) {
            if (!otdslab.isValid)
                continue;
            const TypeOtd &t = otdslab.type;
            const double area = otdslab.poly.CalcArea ();
            for (UInt32 j = 0; j < otdslab.base_composite.GetSize (); j++) {
                GS::UniString mat = otdslab.base_composite[j].val;
                if (mat.Contains (SEMICOLON)) {
                    GS::Array<GS::UniString> partstring;
                    UInt32 n = StringSpltUnic (mat, SEMICOLON, partstring);
                    if (n < 1) {
                        OtdData_AddValueToDict (dct, t, mat, area);
                    } else {
                        for (const GS::UniString &mat_ : partstring) {
                            OtdData_AddValueToDict (dct, t, mat_, area);
                        }
                    }
                } else {
                    OtdData_AddValueToDict (dct, t, mat, area);
                }
            }
        }
    }

    // Собирает площади материалов комнаты, подготавливает её свойства результата и добавляет площади в сводку по
    // tip_otd.
    void OtdData_CalcForRoom (const ColumnFormatDict &columnFormat,
                              const OtdRoom &otd,
                              ParamDictElement &paramToWrite,
                              const ParamDictElement &paramToRead,
                              OtdMaterialAreaDictByOtdType &dct_bytype) {
        OtdMaterialAreaDictByType dct; // Основной словарь
        CollectMaterialAreasForRoom (otd, dct);
        // Записываем результаты в результаты для каждого помещения
        GS::HashTable<TypeOtd, GS::UniString> paramnamebytype;
        paramnamebytype.Add (Wall_Up,
                             otd.om_up.rawname); // Отделка стен выше потолка
        paramnamebytype.Add (Wall_Main,
                             otd.om_main.rawname);                 // Отделка стен основная
        paramnamebytype.Add (Wall_Down, otd.om_down.rawname);      // Отделка низа стен
        paramnamebytype.Add (Column, otd.om_column.rawname);       // Отделка колонн
        paramnamebytype.Add (Reveal_Main, otd.om_reveals.rawname); // Отделка откосов
        paramnamebytype.Add (Floor, otd.om_floor.rawname);         // Отделка пола
        paramnamebytype.Add (Ceil, otd.om_ceil.rawname);           // Отделка потолка
        OtdData_WriteToRoom (columnFormat, otd.zone_guid, paramToWrite, paramToRead, dct, paramnamebytype);
        // Формируем словарь для разбивки по типам

        if (!dct_bytype.ContainsKey (otd.tip_otd)) {
            dct_bytype.Add (otd.tip_otd, dct);
            return;
        }
        OtdMaterialAreaDictByType &dct_to = dct_bytype.Get (otd.tip_otd);
        for (auto &cItt : dct) {
    #ifdef ServerMainVers_2800
            const OtdMaterialAreaDict dct_t = cItt.value;
            const TypeOtd typeotd = cItt.key;
    #else
            const OtdMaterialAreaDict dct_t = *cItt.value;
            const TypeOtd typeotd = *cItt.key;
    #endif
            for (auto &cIt : dct_t) {
    #ifdef ServerMainVers_2800
                const double area = cIt.value;
                const GS::UniString mat = cIt.key;
    #else
                const double area = *cIt.value;
                const GS::UniString mat = *cIt.key;
    #endif
                OtdData_AddValueToDict (dct_to, typeotd, mat, area);
            }
        }
    }

    // Для одного типа копирует его площади материалов; для нескольких суммирует найденные площади в переданный словарь.
    static void CollectAreasForFinishTypes (const GS::Array<TypeOtd> &typeotd,
                                            const OtdMaterialAreaDictByType &dct,
                                            OtdMaterialAreaDict &dcta) {
        if (typeotd.GetSize () == 1) {
            if (dct.ContainsKey (typeotd[0]))
                dcta = dct.Get (typeotd[0]);
        } else {
            // В одно свойство записывается несколько типов. Суммируем
            // материалы.
            for (auto &t : typeotd) {
                if (!dct.ContainsKey (t))
                    continue;
                const OtdMaterialAreaDict &d = dct.Get (t);
                for (auto &m : d) {
    #ifdef ServerMainVers_2800
                    const GS::UniString mat = m.key;
                    const double area = m.value;
    #else
                    const GS::UniString mat = *m.key;
                    const double area = *m.value;
    #endif
                    if (dcta.ContainsKey (mat)) {
                        double area_sum = dcta.Get (mat) + area;
                        dcta.Set (mat, area_sum);
                    } else {
                        dcta.Add (mat, area);
                    }
                }
            }
        }
    }

    // Объединяет типы отделки с одинаковым именем свойства, сортирует материалы и форматирует площади в текстовую
    // таблицу. Добавляет в paramToWrite только изменённые значения; пустой результат заменяет на «-».
    void OtdData_WriteToRoom (const ColumnFormatDict &columnFormat,
                              const API_Guid &zone_guid,
                              ParamDictElement &paramToWrite,
                              const ParamDictElement &paramToRead,
                              const OtdMaterialAreaDictByType &dct,
                              const GS::HashTable<TypeOtd, GS::UniString> &paramnamebytype) {
        // Мы получили словарь по типам отделки
        // Проблема в том, что для разных типов имя свойство может быть одно
        // Это означает, что нужно просуммировать отделку для каждого имени свойства
        GS::HashTable<GS::UniString, GS::Array<TypeOtd>> exsists_rawname; // словарь существующих свойств для типов
                                                                          // отделки
        for (auto &cItt : paramnamebytype) {
    #ifdef ServerMainVers_2800
            const GS::UniString rawname = cItt.value;
            const TypeOtd typeotd = cItt.key;
    #else
            const GS::UniString rawname = *cItt.value;
            const TypeOtd typeotd = *cItt.key;
    #endif
            if (rawname.IsEmpty ())
                continue;
            if (!paramToRead.Get (zone_guid).ContainsKey (rawname))
                continue;
            if (!exsists_rawname.ContainsKey (rawname)) {
                GS::Array<TypeOtd> t;
                exsists_rawname.Add (rawname, t);
            }
            exsists_rawname.Get (rawname).Push (typeotd);
        }

        for (auto &cItt : exsists_rawname) {
    #ifdef ServerMainVers_2800
            GS::UniString rawname = cItt.key;
            GS::Array<TypeOtd> typeotd = cItt.value;
    #else
            GS::UniString rawname = *cItt.key;
            GS::Array<TypeOtd> typeotd = *cItt.value;
    #endif
            // Проверяем - есть ли считанное свойство в зоне для записи
            ParamValue paramtow = paramToRead.Get (zone_guid).Get (rawname);
            GS::UniString old_val = paramtow.val.uniStringValue;
            GS::UniString new_val;
            // Если такой тип отделки есть в словаре - записываем послойно материалы
            // и площади
            OtdMaterialAreaDict dcta;
            CollectAreasForFinishTypes (typeotd, dct, dcta);
            if (!dcta.IsEmpty ()) {
                ColumnFormat c;
                if (columnFormat.ContainsKey (rawname))
                    c = columnFormat.Get (rawname);
                // Сортировка по алфавиту
                std::map<std::string, GS::UniString, doj::alphanum_less<std::string>> abc_material;
                for (auto &cIt : dcta) {
    #ifdef ServerMainVers_2800
                    GS::UniString mat = cIt.key;
    #else
                    GS::UniString mat = *cIt.key;
    #endif
                    GSCharCode chcode = GetCharCode (mat);
                    std::string s = mat.ToCStr (0, MaxUSize, chcode).Get ();
                    abc_material[s] = mat;
                }
                for (std::map<std::string, GS::UniString, doj::alphanum_less<std::string>>::iterator k =
                         abc_material.begin ();
                     k != abc_material.end ();
                     ++k) {
                    GS::UniString mat = k->second;
                    const double *dcta_area = dcta.GetPtr (mat);
                    if (dcta_area == nullptr)
                        continue;
                    double area = *dcta_area;
                    mat.Trim ();
                    mat.ReplaceAll ("  ", SPACESTRING);
                    mat.ReplaceAll ("0&#& ", EMPTYSTRING);
                    mat.ReplaceAll (SPACESTRING, c.no_breake_space);
                    GS::UniString area_sring = GS::UniString::Printf ("%.2f", area);
                    double w_area = GetTextWidth (c.font, c.fontsize, area_sring);
                    if (w_area < c.width_area) {
                        Int32 addspace =
                            DoubleToInt32 ((c.width_area - w_area) / c.width_narow_space, "Roombook", "число пробелов");
                        if (fabs (addspace * c.width_narow_space - c.width_area + w_area) > 0.01)
                            addspace -= 1;
                        if (addspace > 1) {
                            for (Int32 j = 0; j < addspace; j++) {
                                area_sring = c.narow_space + area_sring;
                            }
                        }
                        w_area = GetTextWidth (c.font, c.fontsize, area_sring);
                    }
                    if (w_area - c.width_area > 0.1)
                        area_sring = c.narow_space + area_sring;
                    area_sring.Append (SPACESTRING);
                    if (!new_val.IsEmpty ())
                        new_val.Append (c.delim_line);
                    GS::Array<GS::UniString> lines_mat =
                        DelimTextLine (c.font, c.fontsize, c.width_mat, mat, c.no_breake_space, c.narow_space);
                    for (UInt32 j = 0; j < lines_mat.GetSize (); j++) {
                        new_val.Append (lines_mat[j]);
                        if (j == lines_mat.GetSize () - 1) {
                            new_val.Append (area_sring);
                        } else {
                            new_val.Append (c.space_line);
                        }
                    }
                }
            }
            if (new_val.IsEmpty ())
                new_val = "-";
            if (!old_val.IsEqual (new_val)) {
                paramtow.val.uniStringValue = new_val;
                paramtow.isValid = true;
                ParamHelpers::AddParamValue2ParamDictElement (zone_guid, paramtow, paramToWrite);
            }
        }
    }

    // Добавляет площадь к материалу заданного типа отделки, создавая отсутствующие словари.
    // Пропускает площади меньше 0.000001 и имена, содержащие IGNORENAME.
    void OtdData_AddValueToDict (OtdMaterialAreaDictByType &dct,
                                 const TypeOtd &type,
                                 const GS::UniString &mat,
                                 double area) {
        if (area < 0.000001)
            return;
        if (mat.Contains (IGNORENAME))
            return;
        OtdMaterialAreaDict *innerDictPtr = dct.GetPtr (type);

        if (innerDictPtr == nullptr) {
            // Если такого типа еще нет, создаем и добавляем
            OtdMaterialAreaDict newDict;
            newDict.Add (mat, area);
            dct.Add (type, std::move (newDict));
            return;
        }

        // ищем площадь конкретного материала
        if (double *existingAreaPtr = innerDictPtr->GetPtr (mat)) {
            *existingAreaPtr += area;
        } else {
            innerDictPtr->Add (mat, area);
        }
    }

    // -----------------------------------------------------------------------------
    // Получение площади отделочной стены с учётом отверстий
    // -----------------------------------------------------------------------------
    double OtdWall_GetArea (const OtdWall &otdw) {
        double dx = -otdw.endC.x + otdw.begC.x;
        double dy = -otdw.endC.y + otdw.begC.y;
        double dr = sqrt (dx * dx + dy * dy);
        double area = dr * otdw.height;
        if (otdw.surface_geometry) {
            for (const auto &opening : otdw.openings)
                area -= opening.width * opening.height;
            return std::max (0.0, area) * std::sqrt (1 + otdw.rise_x * otdw.rise_x + otdw.rise_y * otdw.rise_y);
        }
        // Если задана ширина - это откос
        if (!is_equal (otdw.width, 0)) {
            if (!is_equal (otdw.length, 0)) {
                area = otdw.width * otdw.length;
            } else {
                area = otdw.width * otdw.height;
            }
            // В откосах проёмов быть не может - выходим
            if (area < 0)
                area = 0;
            return area;
        }
        if (is_equal (dr, 0))
            return 0;
        if (is_equal (area, 0))
            return 0;
        if (!otdw.openings.IsEmpty ()) {
            double area_op = 0;
            for (const OtdOpening &op : otdw.openings) {
                area_op += op.height * op.width;
            }
            area -= area_op;
            if (area < 0)
                area = 0;
        }
        return area;
    }

    // Читает геометрию и связи зоны, заполняет данные комнаты и индекс связанных конструкций и проёмов.
    // Дополнительно ищет стены по серединам рёбер; возвращает признак найденных связей или стен, при ошибках чтения —
    // false.
    bool CollectRoomInfo (const Stories &storyLevels,
                          const API_Guid &zoneGuid,
                          OtdRoom &roominfo,
                          UnicElementByType &elementToRead,
                          GS::HashTable<API_Guid, GS::Array<API_Guid>> &slabsinzone,
                          RoomSurfaceCache *cache,
                          CoordinateSearchCache *coordinateCache) {
        roominfo.surfaceSnapshot = RoomSurfaceSnapshot ();
        API_Element zoneelement = {};
        zoneelement.header.guid = zoneGuid;

        GSErrCode err = ACAPI_Element_Get (&zoneelement);
        if (err != NoError) {
            msg_rep ("CollectRoomInfo err", "ACAPI_Element_Get zone", err, zoneGuid);
            return false;
        }

        roominfo.height = zoneelement.zone.roomHeight;
        roominfo.height = std::round (roominfo.height * 1000) / 1000;
        if (roominfo.height < 0.0001) {
            msg_rep ("CollectRoomInfo err", "room.height < 0.0001", err, zoneGuid);
            roominfo.isValid = false;
            return false;
        }

        API_ElementMemo zonememo = {};
        const GS::OnExit zoneMemoGuard ([&zonememo] () { ACAPI_DisposeElemMemoHdls (&zonememo); });
        err = ACAPI_Element_GetMemo (zoneelement.header.guid, &zonememo);
        if (err != NoError) {
            msg_rep ("CollectRoomInfo err", "ACAPI_Element_GetMemo zone", err, zoneGuid);
            return false;
        }
        GS::Array<Sector> walledges;   // Границы стен, не явяющихся границей зоны
        GS::Array<Sector> columnedges; // Границы колонн, не явяющихся границей зоны
        GS::Array<Sector> restedges;   // Границы зоны
        GS::Array<Sector> gableedges;
        Edges_GetFromRoom (zonememo, zoneelement, walledges, columnedges, restedges, gableedges);
        roominfo.edges = restedges;
        roominfo.columnedges = columnedges;
        RoomRelationValues relData;
        err = ReadZoneRelations (zoneGuid, cache, relData);
        if (err != NoError) {
            msg_rep ("CollectRoomInfo err", "ACAPI_Element_GetRelations zone", err, zoneGuid);
            return false;
        }
        roominfo.zBottom = GetzPos (zoneelement.zone.roomBaseLev, zoneelement.header.floorInd, storyLevels);
        roominfo.zBottom = std::round (roominfo.zBottom * 1000) / 1000;
        roominfo.floorInd = zoneelement.header.floorInd;
        bool flag = false;
        for (API_ElemTypeID typeelem : typeinzone) {
            if (const GS::Array<API_Guid> *guidsPtr = relData.elementsGroupedByType.GetPtr (typeelem)) {
                for (const API_Guid &elGuid : *guidsPtr) {
                    Param_AddUnicElementByType (elGuid, zoneGuid, typeelem, elementToRead);
                }
                flag = true;
            }
        }
        if (const GS::Array<API_Guid> *slabsPtr = slabsinzone.GetPtr (zoneGuid)) {
            for (const API_Guid &elGuid : *slabsPtr) {
                Param_AddUnicElementByType (elGuid, zoneGuid, API_SlabID, elementToRead);
            }
        }
        OtdSlab otdslab;
        ConstructPolygon2DFromElementMemo (zonememo, otdslab.poly);
    #ifdef ServerMainVers_2700
        otdslab.material.material = zoneelement.zone.material.ToInt32_Deprecated ();
    #else
        otdslab.material.material = zoneelement.zone.material;
    #endif
        otdslab.base_type = API_ZoneID;
        otdslab.base_guid = zoneGuid;
        otdslab.floorInd = zoneelement.header.floorInd;
        roominfo.poly = otdslab;
        if (!walledges.IsEmpty ()) {
            API_Guid elGuid;
            API_ElemSearchPars searchPars = {};
            API_ElemTypeID typeelem = API_WallID;
            for (UInt32 j = 0; j < walledges.GetSize (); j++) {
                Sector tedge = walledges[j];
                BNZeroMemory (&searchPars, sizeof (API_ElemSearchPars));
                searchPars.filterBits =
                    APIFilt_OnVisLayer | APIFilt_In3D | APIFilt_IsVisibleByRenovation | APIFilt_IsInStructureDisplay;
                searchPars.loc.x = tedge.GetMidPoint ().x;
                searchPars.loc.y = tedge.GetMidPoint ().y;
                searchPars.z = roominfo.zBottom + roominfo.height;
    #ifdef ServerMainVers_2600
                searchPars.type.typeID = typeelem;
    #else
                searchPars.typeID = typeelem;
    #endif
                elGuid = APINULLGuid;
                err = SearchCoordinate (searchPars, elGuid, coordinateCache);
                if (err == NoError && elGuid != APINULLGuid) {
                    flag = true;
                    for (const auto connectedType : {API_WindowID, API_DoorID}) {
                        const auto connected =
                            ReadConnectedElements (elGuid,
                                                   connectedType,
                                                   APIFilt_IsEditable | APIFilt_HasAccessRight | APIFilt_InMyWorkspace,
                                                   cache);
                        if (connected.error != NoError) {
                            msg_rep (
                                "CollectRoomInfo err", "ACAPI_Element_GetConnectedElements", connected.error, elGuid);
                            return false;
                        }
                        for (const API_Guid &openingGuid : connected.items)
                            Param_AddUnicElementByType (openingGuid, zoneGuid, API_WindowID, elementToRead);
                    }
                    if (GS::Array<Sector> *sectorsPtr = roominfo.walledges.GetPtr (elGuid)) {
                        sectorsPtr->Push (tedge); // Если нашли, пушим напрямую в оригинальный массив
                    } else {
                        roominfo.walledges.Add (elGuid, {tedge}); // Если не нашли, эффективно добавляем
                    }
                    UnicElement *unicElPtr = elementToRead.GetPtr (typeelem);
                    if (unicElPtr == nullptr) {
                        elementToRead.Add (typeelem, UnicElement{});
                        unicElPtr = elementToRead.GetPtr (typeelem);
                    }
                    if (GS::Array<API_Guid> *zonesPtr = unicElPtr->GetPtr (elGuid)) {
                        if (typeelem == API_WallID || typeelem == API_ColumnID) {
                            zonesPtr->Push (zoneGuid);
                        }
                    } else {
                        unicElPtr->Add (elGuid, {zoneGuid});
                    }
                } else {
                    msg_rep (
                        "CollectRoomInfo err", "ACAPI_Element_SearchElementByCoord element not found", err, zoneGuid);
                }
            }
        }
        roominfo.wallPart = relData.wallPart;
        roominfo.beamPart = relData.beamPart;
        // roominfo.cwSegmentPart = relData.cwSegmentPart;
        // roominfo.niches = relData.niches;
        roominfo.zone_guid = zoneGuid;
        roominfo.isValid = flag;
    #ifdef ServerMainVers_2700
        roominfo.om_zone.material = zoneelement.zone.material.ToInt32_Deprecated ();
    #else
        roominfo.om_zone.material = zoneelement.zone.material;
    #endif
        // Пока только снимок: ошибка 3D не меняет прежний результат сбора и создания отделки.
        roominfo.surfaceSnapshot = ReadRoomSurfaces (zoneelement.header, cache);
        return flag;
    }

    // -----------------------------------------------------------------------------
    // Чтение данных о проёме
    // -----------------------------------------------------------------------------
    void Opening_Create_One (const Stories &storyLevels,
                             const API_Guid &elGuid,
                             GS::HashTable<API_Guid, GS::Array<OtdOpening>> &openinginwall,
                             UnicGUIDByType &guidselementToRead) {
        GSErrCode err;
        API_Element element = {};
        element.header.guid = elGuid;
        err = ACAPI_Element_Get (&element);
        if (err != NoError) {
            msg_rep ("Opening_Create_One err", "ACAPI_Element_Get", err, elGuid);
            return;
        }
        API_Guid wallguid = APINULLGuid;
        OtdOpening op;
        API_ElemTypeID eltype = GetElemTypeID (element);
        switch (eltype) {
        case API_WindowID:
            wallguid = element.window.owner;
            op.height = element.window.openingBase.height;
            op.width = element.window.openingBase.width;
            op.lower = element.window.lower;
            op.objLoc = element.window.objLoc;
            op.reflected = element.window.openingBase.reflected;
            op.ref_side = element.window.openingBase.refSide;
            break;
        case API_DoorID:
            wallguid = element.door.owner;
            op.height = element.door.openingBase.height;
            op.width = element.door.openingBase.width;
            op.lower = element.door.lower;
            op.objLoc = element.door.objLoc;
            op.reflected = element.door.openingBase.reflected;
            op.ref_side = element.door.openingBase.refSide;
            break;
        default:
            break;
        }
        if (op.width < min_dim || op.height < min_dim)
            return;
        op.base_guid = elGuid;
        if (GS::Array<OtdOpening> *openingsPtr = openinginwall.GetPtr (wallguid)) {
            openingsPtr->Push (std::move (op));
        } else {
            openinginwall.Add (wallguid, {std::move (op)});
        }
        Param_AddUnicGUIDByType (elGuid, API_WindowID, guidselementToRead);
    }

    // -----------------------------------------------------------------------------
    // Создание стен-отделок для стен
    // -----------------------------------------------------------------------------
    void OtdWall_Create_FromWall (const Stories &storyLevels,
                                  const API_Guid &elGuid,
                                  const GS::Array<API_Guid> &zoneGuids,
                                  OtdRooms &roomsinfo,
                                  const GS::HashTable<API_Guid, GS::Array<OtdOpening>> &openinginwall,
                                  UnicGUIDByType &guidselementToRead,
                                  RoomSurfaceCache *cache) {
        const ElementRead elementRead = ReadElement (elGuid, cache);
        if (elementRead.error != NoError) {
            msg_rep ("OtdWall_Create_FromWall err", "ACAPI_Element_Get", elementRead.error, elGuid);
            return;
        }
        const API_Element &element = elementRead.element;
        double w_bottomOffset = element.wall.bottomOffset;
        double w_height = element.wall.height;
        Point2D wbegC = {element.wall.begC.x, element.wall.begC.y};
        Point2D wendC = {element.wall.endC.x, element.wall.endC.y};
        Sector walledge = {wbegC, wendC};
        double dx = -element.wall.endC.x + element.wall.begC.x;
        double dy = -element.wall.endC.y + element.wall.begC.y;
        double wallLength = sqrt (dx * dx + dy * dy);
        double zBottom = GetzPos (w_bottomOffset, element.header.floorInd, storyLevels);
        if (wallLength < min_dim) {
    #if defined(TESTING)
            DBprnt ("OtdWall_Create_FromWall err", "walledge.GetLength ()<min_dim");
    #endif
            return;
        }
        GS::Optional<UnitVector_2D> walldir = walledge.GetDirection ();
        if (!walldir.HasValue ()) {
    #if defined(TESTING)
            DBprnt ("OtdWall_Create_FromWall err", "!walldir.HasValue");
    #endif
            return;
        }
        const GS::Optional<Geometry::Line2D> wallline = walledge.AsLine ();
        for (const API_Guid &zoneGuid : zoneGuids) {
            OtdRoom *roominfoPtr = roomsinfo.GetPtr (zoneGuid);
            if (roominfoPtr == nullptr || !roominfoPtr->isValid) {
    #if defined(TESTING)
                DBprnt ("OtdWall_Create_FromWall err", "!roomsinfo.ContainsKey (zoneGuid)");
    #endif
                continue;
            }

            bool flag_find = false;
            if (const auto *sectorsPtr = roominfoPtr->walledges.GetPtr (elGuid)) {
                if (!wallline.HasValue ()) {
    #if defined(TESTING)
                    DBprnt ("OtdWall_Create_FromWall err", "!wallline.HasValue ()");
    #endif
                    continue;
                }
                for (const auto &roomedge : *sectorsPtr) {
                    GS::Optional<Geometry::Line2D> roomeline = roomedge.AsLine ();
                    if (!roomeline.HasValue ()) {
    #if defined(TESTING)
                        DBprnt ("OtdWall_Create_FromWall err", "!roomeline.HasValue ()");
    #endif
                        continue;
                    }
                    GS::Optional<UnitVector_2D> roomedgedir = roomedge.GetDirection ();
                    if (!roomedgedir.HasValue ()) {
    #if defined(TESTING)
                        DBprnt ("OtdWall_Create_FromWall err", "!roomedgedir.HasValue");
    #endif
                        continue;
                    }
                    bool is_parallel = walldir.Get ().IsParallelWith (roomedgedir.Get ());
                    bool is_fliped = false;
                    if (is_parallel)
                        is_fliped = !walldir.Get ().IsEqualWith (roomedgedir.Get ());
                    OtdWall wallotd;
                    if (is_parallel) {
                        Point2D begedge;
                        Point2D endedge;
                        if (is_fliped) {
                            begedge = wallline.Get ().ProjectPoint (roomedge.c2);
                            endedge = wallline.Get ().ProjectPoint (roomedge.c1);
                        } else {
                            begedge = wallline.Get ().ProjectPoint (roomedge.c1);
                            endedge = wallline.Get ().ProjectPoint (roomedge.c2);
                        }
                        double tBeg = sqrt ((wbegC.x - begedge.x) * (wbegC.x - begedge.x) +
                                            (wbegC.y - begedge.y) * (wbegC.y - begedge.y));
                        double tEnd = wallLength - sqrt ((wendC.x - endedge.x) * (wendC.x - endedge.x) +
                                                         (wendC.y - endedge.y) * (wendC.y - endedge.y));
                        // Ищем проёмы в этом участке стены
                        if (const auto *opsPtr = openinginwall.GetPtr (elGuid)) {
                            for (const OtdOpening &op : *opsPtr) {
                                Opening_Add_One (op, is_fliped, zBottom, tBeg, tEnd, wallLength, wallotd);
                            }
                        }
                    }
                    walledge = roomedge;
                    if (OtdWall_Add_One (elGuid,
                                         walledge,
                                         is_fliped,
                                         w_height,
                                         zBottom,
                                         roominfoPtr->floorInd,
                                         element.wall.thickness,
                                         wallotd)) {
                        roominfoPtr->otdwall.Push (std::move (wallotd));
                        flag_find = true;
                    }
                }
            }
            if (!roominfoPtr->wallPart.IsEmpty () && element.wall.zoneRel != APIZRel_None) {
                for (const API_WallPart &wpart : roominfoPtr->wallPart) {
                    if (wpart.guid != elGuid)
                        continue;
                    UInt32 inxedge = wpart.roomEdge - 1;
                    if (inxedge >= roominfoPtr->edges.GetSize ()) {
    #if defined(TESTING)
                        DBprnt ("OtdWall_Create_FromWall err", "inxedge > roomedges.restedges.GetSize ()");
    #endif
                        continue;
                    }
                    const Sector &roomedge = roominfoPtr->edges.Get (inxedge);
                    GS::Optional<Geometry::Line2D> roomeline = roomedge.AsLine ();
                    if (!roomeline.HasValue ()) {
    #if defined(TESTING)
                        DBprnt ("OtdWall_Create_FromWall err", "!roomeline.HasValue ()");
    #endif
                        continue;
                    }
                    GS::Optional<UnitVector_2D> roomedgedir = roomedge.GetDirection ();
                    if (!roomedgedir.HasValue ()) {
    #if defined(TESTING)
                        DBprnt ("OtdWall_Create_FromWall err", "!roomedgedir.HasValue");
    #endif
                        continue;
                    }
                    bool is_parallel = walldir.Get ().IsParallelWith (roomedgedir.Get ());
                    bool is_fliped = false;
                    if (is_parallel)
                        is_fliped = !walldir.Get ().IsEqualWith (roomedgedir.Get ());
                    OtdWall wallotd;
                    if (is_parallel) {
                        Point2D begedge = wbegC;
                        Point2D endedge = wendC;
                        if (!is_equal (wpart.tEnd, 0) || !is_equal (wpart.tBeg, 0)) {
                            if (!is_equal (wpart.tBeg, 0) && !is_equal (wpart.tBeg, wallLength)) {
                                double lambda = wpart.tBeg / wallLength;
                                begedge.x = wbegC.x - dx * lambda;
                                begedge.y = wbegC.y - dy * lambda;
                            }
                            if (!is_equal (wpart.tEnd, 0) && !is_equal (wpart.tEnd, wallLength)) {
                                double lambda = wpart.tEnd / wallLength;
                                endedge.x = wbegC.x - dx * lambda;
                                endedge.y = wbegC.y - dy * lambda;
                            }
                        }
                        begedge = roomeline.Get ().ProjectPoint (begedge);
                        endedge = roomeline.Get ().ProjectPoint (endedge);
                        if (is_fliped) {
                            walledge = {endedge, begedge};
                        } else {
                            walledge = {begedge, endedge};
                        }
                        // Ищем проёмы в этом участке стены
                        if (const auto *opsPtr = openinginwall.GetPtr (elGuid)) {
                            for (const OtdOpening &op : *opsPtr) {
                                Opening_Add_One (op, is_fliped, zBottom, wpart.tBeg, wpart.tEnd, wallLength, wallotd);
                            }
                        }
                        if (element.wall.flipped)
                            is_fliped = !is_fliped;
                    } else {
                        walledge = roomedge; // Торец стены
                    }
                    if (OtdWall_Add_One (elGuid,
                                         walledge,
                                         is_fliped,
                                         w_height,
                                         zBottom,
                                         roominfoPtr->floorInd,
                                         element.wall.thickness,
                                         wallotd)) {
                        roominfoPtr->otdwall.Push (std::move (wallotd));
                        flag_find = true;
                    }
                }
            }
            if (flag_find)
                Param_AddUnicGUIDByType (elGuid, API_WallID, guidselementToRead);
        }
    }

    // Обрезает проём границами участка стены и задаёт его локальное положение, отметку низа и сторону откоса.
    // Возвращает false, если ширина пересечения неположительна.
    static bool CalculateOpeningForWall (const OtdOpening &op,
                                         bool is_fliped,
                                         double zBottom,
                                         double tBeg,
                                         double tEnd,
                                         double wallLength,
                                         OtdOpening &opinwall) {
        // Проверяем - находится ли этот проём на заданном участке стены
        double opLeft = op.objLoc - op.width / 2.0;
        double opRight = op.objLoc + op.width / 2.0;

        // Обрезаем проем границами участка стены
        double newLeft = std::max (opLeft, tBeg);
        double newRight = std::min (opRight, tEnd);

        if (newLeft < newRight) {
            opinwall.base_guid = op.base_guid;
            opinwall.ref_side = op.ref_side;
            // Используем разворот участка до поправки на flipped стены и порядок слоёв.
            opinwall.reveal_from_sill = op.ref_side != is_fliped;
            opinwall.width = newRight - newLeft;
            opinwall.height = op.height;
            opinwall.zBottom = zBottom + op.lower;

            double newCenter = (newLeft + newRight) / 2.0;
            if (!is_fliped) {
                opinwall.objLoc = newCenter - tBeg;
            } else {
                opinwall.objLoc = (wallLength - newCenter) - (wallLength - tEnd);
            }
            return true;
        }
        return false;
    }

    // Добавляет в стену проём, рассчитанный для заданного участка, только при положительной ширине пересечения.
    void Opening_Add_One (const OtdOpening &op,
                          bool is_fliped,
                          double zBottom,
                          double tBeg,
                          double tEnd,
                          double wallLength,
                          OtdWall &wallotd) {
        OtdOpening opinwall;
        if (CalculateOpeningForWall (op, is_fliped, zBottom, tBeg, tEnd, wallLength, opinwall))
            wallotd.openings.Push (std::move (opinwall));
    }

    // Заполняет геометрию и базовые данные участка отделочной стены с типом Wall_Main.
    // Возвращает false, если длина участка меньше min_dim.
    bool OtdWall_Add_One (const API_Guid &wallguid,
                          const Sector &walledge,
                          bool is_fliped,
                          double height,
                          double zBottom,
                          short floorInd,
                          double thickness,
                          OtdWall &wallotd) {
        double walledgedx = -walledge.c1.x + walledge.c2.x;
        double walledgedy = -walledge.c1.y + walledge.c2.y;
        double walledgedr = sqrt (walledgedx * walledgedx + walledgedy * walledgedy);
        if (walledgedr < min_dim)
            return false;
        wallotd.base_guid = wallguid;
        wallotd.height = height;
        wallotd.base_th = thickness;
        wallotd.zBottom = zBottom;
        wallotd.base_flipped = is_fliped;
        wallotd.floorInd = floorInd;
        wallotd.begC = {walledge.c1.x, walledge.c1.y};
        wallotd.endC = {walledge.c2.x, walledge.c2.y};
        wallotd.base_type = API_WallID;
        wallotd.type = Wall_Main;
        return true;
    }

    // -----------------------------------------------------------------------------
    // Создание стен-отделок для колонны
    // -----------------------------------------------------------------------------
    void OtdWall_Create_FromColumn (const Stories &storyLevels,
                                    const API_Guid &elGuid,
                                    const GS::Array<API_Guid> &zoneGuids,
                                    OtdRooms &roomsinfo,
                                    UnicGUIDByType &guidselementToRead) {
        GSErrCode err;
        API_Element element = {};
        element.header.guid = elGuid;
        err = ACAPI_Element_Get (&element);
        if (err != NoError || !element.header.hasMemo) {
            msg_rep ("OtdWall_Create_FromColumn err", "ACAPI_Element_Get", err, elGuid);
            return;
        }
        if (element.column.zoneRel == APIZRel_None) {
    #if defined(TESTING)
            DBprnt ("OtdWall_Create_FromColumn err", "element.column.zoneRel == APIZRel_None");
    #endif
            return;
        }
        API_ElementMemo segmentmemo = {};
        const GS::OnExit memoGuard ([&segmentmemo] () { ACAPI_DisposeElemMemoHdls (&segmentmemo); });
        err = ACAPI_Element_GetMemo (elGuid,
                                     &segmentmemo,
                                     APIMemoMask_ColumnSegment | APIMemoMask_AssemblySegmentScheme |
                                         APIMemoMask_AssemblySegmentProfile);
        if (err != NoError || segmentmemo.columnSegments == nullptr) {
    #if defined(TESTING)
            DBprnt ("OtdWall_Create_FromColumn err", "ACAPI_Element_GetMemo column");
    #endif
            return;
        }
        bool flag_find = false;
        Int32 inx = 0;
        Point2D orig = {element.column.origoPos.x, element.column.origoPos.y};
        Point2D begedge = {0, 0};
        Point2D endedge = {0, 0};
        Sector columnedge = {begedge, endedge};
        if (segmentmemo.columnSegments[inx].assemblySegmentData.modelElemStructureType == API_BasicStructure) {
            double height = segmentmemo.columnSegments[inx].assemblySegmentData.nominalHeight / 2;
            double width = segmentmemo.columnSegments[inx].assemblySegmentData.nominalWidth / 2;
            height = height + segmentmemo.columnSegments[inx].venThick;
            width = width + segmentmemo.columnSegments[inx].venThick;
            // Построение отрезков границ
            GS::Array<Sector> columnedges;
            begedge = {orig.x + width, orig.y + height};
            endedge = {orig.x - width, orig.y + height};
            columnedge = {begedge, endedge};
            columnedges.Push (columnedge);
            begedge = {orig.x - width, orig.y + height};
            endedge = {orig.x - width, orig.y - height};
            columnedge = {begedge, endedge};
            columnedges.Push (columnedge);
            begedge = {orig.x - width, orig.y - height};
            endedge = {orig.x + width, orig.y - height};
            columnedge = {begedge, endedge};
            columnedges.Push (columnedge);
            begedge = {orig.x + width, orig.y - height};
            endedge = {orig.x + width, orig.y + height};
            columnedge = {begedge, endedge};
            columnedges.Push (columnedge);
            double zBottom = GetzPos (element.column.bottomOffset, element.header.floorInd, storyLevels);
            for (const Sector &coledge : columnedges) {
                for (const auto &zoneGuid : zoneGuids) {
                    OtdRoom *roominfo = roomsinfo.GetPtr (zoneGuid);
                    if (roominfo == nullptr || !roominfo->isValid)
                        continue;
                    Sector cdge;
                    bool find = Edge_FindOnEdge (coledge, roominfo->edges, cdge);
                    if (!find)
                        find = Edge_FindOnEdge (coledge, roominfo->columnedges, cdge);
                    if (!find)
                        continue;
                    OtdWall wallotd;
                    wallotd.base_guid = elGuid;
                    wallotd.height = element.column.height;
                    wallotd.zBottom = zBottom;
                    wallotd.floorInd = roominfo->floorInd;
                    wallotd.begC = {cdge.c1.x, cdge.c1.y};
                    wallotd.endC = {cdge.c2.x, cdge.c2.y};
                    wallotd.material = roominfo->om_main;
                    wallotd.base_type = API_ColumnID;
                    wallotd.type = Column;
                    roominfo->otdwall.Push (std::move (wallotd));
                    flag_find = true;
                }
            }
        }
        if (flag_find)
            Param_AddUnicGUIDByType (elGuid, API_ColumnID, guidselementToRead);
    }

    // -----------------------------------------------------------------------------
    // Находит все перекрытия, котрые пересекают зону и не являются элементами
    // отделки
    // -----------------------------------------------------------------------------
    void Floor_FindAll (GS::HashTable<API_Guid, GS::Array<API_Guid>> &slabsinzone,
                        const UnicGuid &finclassguids,
                        const GS::Array<API_Guid> &zones,
                        RoomSurfaceCache *cache) {
        GS::Array<API_Guid> allslabs_;
        GS::Array<API_Guid> allslabs;
        GSErrCode err = NoError;
        err = ACAPI_Element_GetElemList (API_SlabID, &allslabs_, APIFilt_OnVisLayer | APIFilt_IsVisibleByRenovation);
        if (err != NoError) {
            return;
        }
        if (allslabs_.IsEmpty ())
            return;
        for (const auto &guid : allslabs_) {
            API_Guid classguid = APINULLGuid;
            if (!Class_IsElementFinClass (guid, finclassguids, classguid, cache))
                allslabs.Push (std::move (guid));
        }
        if (allslabs.IsEmpty ())
            return;
        // Конечное XY-пересечение независимо от высоты зоны и стен; исходные отверстия сохранены.
        GS::HashTable<API_Guid, Geometry::Polygon2D> zonePolygons;
        for (const auto &zoneGuid : zones) {
            API_ElementMemo memo = {};
            const GS::OnExit memoGuard ([&memo] () { ACAPI_DisposeElemMemoHdls (&memo); });
            Geometry::Polygon2D polygon;
            const GSErrCode memoError = ACAPI_Element_GetMemo (zoneGuid, &memo, APIMemoMask_Polygon);
            if (memoError == NoError && ConstructPolygon2DFromElementMemo (memo, polygon) == NoError)
                zonePolygons.Add (zoneGuid, polygon);
        }
        for (const auto &slabGuid : allslabs) {
            API_ElementMemo memo = {};
            const GS::OnExit memoGuard ([&memo] () { ACAPI_DisposeElemMemoHdls (&memo); });
            Geometry::Polygon2D polygon;
            const GSErrCode memoError = ACAPI_Element_GetMemo (slabGuid, &memo, APIMemoMask_Polygon);
            const GSErrCode polygonError =
                memoError == NoError ? ConstructPolygon2DFromElementMemo (memo, polygon) : memoError;
            if (polygonError != NoError)
                continue;
            for (const auto &zoneGuid : zones) {
                const auto *zonePolygon = zonePolygons.GetPtr (zoneGuid);
                if (zonePolygon == nullptr)
                    continue;
                const auto overlap = polygon.Intersect (*zonePolygon);
                double area = 0;
                for (const auto &part : overlap)
                    area += std::abs (part.CalcArea ());
                if (!std::isfinite (area) || area <= 1e-12)
                    continue;
                if (!slabsinzone.ContainsKey (zoneGuid))
                    slabsinzone.Add (zoneGuid, GS::Array<API_Guid> ());
                auto &indexed = slabsinzone[zoneGuid];
                if (!indexed.Contains (slabGuid))
                    indexed.Push (slabGuid);
            }
        }
    }

    // -----------------------------------------------------------------------------
    // Обработка полов и потолков
    // -----------------------------------------------------------------------------
    // Опоры ниже/на полу и пересекающие пол; потолки строго выше пола независимо от верха зоны.
    // Добавляет перекрытие в список чтения параметров, если обработана хотя бы одна валидная зона.
    void Floor_FindInOneRoom (const Stories &storyLevels,
                              API_Guid &elGuid,
                              const GS::Array<API_Guid> &zoneGuids,
                              OtdRooms &roomsinfo,
                              UnicGUIDByType &guidselementToRead,
                              RoomSurfaceCache *cache) {
        const ElementRead elementRead = ReadElement (elGuid, cache);
        if (elementRead.error != NoError || !elementRead.element.header.hasMemo) {
            msg_rep ("Floor_FindInOneRoom err", "ACAPI_Element_Get", elementRead.error, elGuid);
            return;
        }
        const API_Element &element = elementRead.element;
        GS::Array<OtdWall> walls;
        GS::Array<OtdSlab> bases;
        const auto preparedSource = PrepareSource (element, storyLevels, {}, walls, bases, cache);
        bool flag_find = false;
        double zBottom = GetzPos (element.slab.level + element.slab.offsetFromTop - element.slab.thickness,
                                  element.header.floorInd,
                                  storyLevels);
        double zUp = zBottom + element.slab.thickness;
        for (const API_Guid &zoneGuid : zoneGuids) { // ОПТИМИЗАЦИЯ: Передаем по ссылке, а не копируем 16 байт GUID
            OtdRoom *roominfoPtr = roomsinfo.GetPtr (zoneGuid);
            if (roominfoPtr == nullptr || !roominfoPtr->isValid)
                continue;
            const auto role = AnalyticGeometry::SlabRole (zBottom, zUp, roominfoPtr->zBottom);
            if (role == AnalyticGeometry::SlabCandidateRole::None)
                continue;
            auto &roles =
                role == AnalyticGeometry::SlabCandidateRole::Floor ? roominfoPtr->floorslab : roominfoPtr->ceilslab;
            if (!roles.Contains (elGuid))
                roles.Push (elGuid);
            bool prepared = false;
            for (const auto &source : roominfoPtr->slabSources)
                prepared = prepared || source.baseGuid == elGuid;
            if (!prepared) {
                roominfoPtr->slabSources.Push (preparedSource);
                roominfoPtr->preparedSlabs.Append (bases);
            }
            flag_find = true;
        }
        if (flag_find)
            Param_AddUnicGUIDByType (elGuid, API_SlabID, guidselementToRead);
    }

    // Добавляет все rawnames из списка параметров в словарь значений для чтения.
    void Param_ToParamDict (ParamDictValue &paramDict, ReadParams &zoneparams) {
        if (zoneparams.IsEmpty ())
            return;
        for (const auto &p : zoneparams) {
    #ifdef ServerMainVers_2800
            const ReadParam &param = p.value;
    #else
            const ReadParam &param = *p.value;
    #endif
            for (const auto &rawname : param.rawnames)
                ParamHelpers::AddValueToParamDictValue (paramDict, rawname);
        }
    }

    // Готовит параметры толщины коробки и отступа окна gs_frame_thk и gs_wido_sill для чтения.
    ReadParams Param_GetForWindowParams () {
        ReadParams zoneparams;
        ReadParam zoneparam;
        GS::UniString zoneparam_name;

        zoneparam_name = "frame";
        zoneparam.rawnames.Push ("{@gdl:gs_frame_thk}");
        zoneparams.Add (zoneparam_name, zoneparam);
        zoneparam.rawnames.Clear ();

        zoneparam_name = "sill";
        zoneparam.rawnames.Push ("{@gdl:gs_wido_sill}");
        zoneparams.Add (zoneparam_name, zoneparam);
        zoneparam.rawnames.Clear ();

        // Включение/отключение создания откосов для проёма
        zoneparam_name = "reveal_onoff";
        zoneparam.rawnames.Push ("some_stuff_reveal_onoff");
        zoneparams.Add (zoneparam_name, zoneparam);
        zoneparam.rawnames.Clear ();

        Param_Property_FindInParams (zoneparams);
        return zoneparams;
    }

    // -----------------------------------------------------------------------------
    // Подготовка словаря с параметрами для чтения из стен/колонн/балок
    // -----------------------------------------------------------------------------
    void Param_GetForBase (ParamDictValue &paramDict, ParamValue &param_composite) {
        // Поиск свойств со включением слоя отделки и имени
        GS::UniString propdesc_onoff = "some_stuff_layer_onoff";
        GS::UniString propdesc_desc = "some_stuff_layer_description";
        GS::UniString propdesc_fav = "some_stuff_layer_favorite_name";
        GS::UniString propdesc_hasfin = "some_stuff_layer_has_finish";
        GS::UniString propdesc_hasfin_elem = "some_stuff_element_has_finish";
        GS::UniString propdesc_onoff_elem = "some_stuff_element_onoff";
        GS::UniString rawName_onoff;
        GS::UniString rawName_desc;
        GS::UniString rawName_fav;
        GS::UniString rawName_hasfin;
        ParamValue param_hasfin_elem;
        ParamValue param_onoff_elem;
        GS::UniString msg;
        if (!ParamHelpers::isPropertyDefinitionRead ())
            return;
        ParamDictValue &propertyParams = PROPERTYCACHE ().property;
        for (auto &cItt : propertyParams) {
    #ifdef ServerMainVers_2800
            ParamValue param = cItt.value;
    #else
            ParamValue param = *cItt.value;
    #endif
            if (param.definition.description.Contains (propdesc_onoff)) {
                if (rawName_onoff.IsEmpty ()) {
                    rawName_onoff = param.rawName;
                } else {
                    msg += param.definition.name + SPACESTRING;
                }
                continue;
            }
            if (param.definition.description.Contains (propdesc_desc)) {
                if (rawName_desc.IsEmpty ()) {
                    rawName_desc = param.rawName;
                } else {
                    msg += param.definition.name + SPACESTRING;
                }
                continue;
            }
            if (param.definition.description.Contains (propdesc_fav)) {
                if (rawName_fav.IsEmpty ()) {
                    rawName_fav = param.rawName;
                } else {
                    msg += param.definition.name + SPACESTRING;
                }
                continue;
            }
            if (param.definition.description.Contains (propdesc_hasfin)) {
                if (rawName_hasfin.IsEmpty ()) {
                    rawName_hasfin = param.rawName;
                } else {
                    msg += param.definition.name + SPACESTRING;
                }
                continue;
            }
            if (param.definition.description.Contains (propdesc_hasfin_elem)) {
                if (param_hasfin_elem.rawName.IsEmpty ()) {
                    param_hasfin_elem = param;
                } else {
                    msg += param.definition.name + SPACESTRING;
                }
                continue;
            }

            if (param.definition.description.Contains (propdesc_onoff_elem)) {
                if (param_onoff_elem.rawName.IsEmpty ()) {
                    param_onoff_elem = param;
                } else {
                    msg += param.definition.name + SPACESTRING;
                }
                continue;
            }
        }
        if (!msg.IsEmpty ()) {
            msg_rep (
                "RoomBook", "Duplicate properties for materials/elements were found: " + msg, NoError, APINULLGuid);
            msg.Clear ();
        }
        // Подготавливаем свойство для чтения из слоёв многослойки
        GS::UniString rawName = "{@material:layers,20}";
        param_composite.rawName = rawName;
        param_composite.fromMaterial = true;
        if (!rawName_onoff.IsEmpty () && !rawName_desc.IsEmpty ()) {
            param_composite.val.uniStringValue = "l[" + rawName_onoff + ":" + rawName_desc;
        } else {
            param_composite.val.uniStringValue = "l[1:%BuildingMaterialProperties/Building Material Name%";
            msg_rep ("RoomBook",
                     "Required properties 'some_stuff_layer_onoff' and "
                     "'some_stuff_layer_description' not found in Building Materials. "
                     "Processing all Finish layers with Building Material Name as "
                     "identifier.",
                     NoError,
                     APINULLGuid);
        }
        param_composite.val.uniStringValue.Append ("]f[");
        if (!rawName_fav.IsEmpty ()) {
            param_composite.val.uniStringValue.Append (rawName_fav);
            msg_rep ("RoomBook",
                     "Building Materials contain a property with the description "
                     "'some_stuff_layer_favorite_name'",
                     NoError,
                     APINULLGuid);
        }
        param_composite.val.uniStringValue.Append ("]h[");
        if (!rawName_hasfin.IsEmpty ()) {
            param_composite.val.uniStringValue.Append (rawName_hasfin);
            msg_rep ("RoomBook",
                     "Building Materials contain a property with the description "
                     "'some_stuff_layer_has_finish'",
                     NoError,
                     APINULLGuid);
        }
        param_composite.val.uniStringValue.Append ("] %nosyncname%");
        if (ParamHelpers::ParseParamNameMaterial (param_composite.val.uniStringValue, paramDict)) {
            for (UInt32 inx = 0; inx < 20; inx++) {
                ParamHelpers::AddValueToParamDictValue (paramDict,
                                                        "@property:sync_name" + GS::UniString::Printf ("%d", inx));
            }
            ParamHelpers::CompareParamDictValue (propertyParams, paramDict);
        }
        if (!param_hasfin_elem.rawName.IsEmpty ()) {
            ParamValue param;
            param.rawName = "{@flag:rawname_hasfin_elem}";
            param.val.uniStringValue = param_hasfin_elem.rawName;
            ParamHelpers::AddParamValue2ParamDict (APINULLGuid, param, paramDict);
            ParamHelpers::AddParamValue2ParamDict (APINULLGuid, param_hasfin_elem, paramDict);
            msg_rep ("RoomBook",
                     "Find property with the description "
                     "'some_stuff_element_has_finish'",
                     NoError,
                     APINULLGuid);
        }
        if (!param_onoff_elem.rawName.IsEmpty ()) {
            ParamValue param;
            param.rawName = "{@flag:rawname_element_onoff}";
            param.val.uniStringValue = param_onoff_elem.rawName;
            ParamHelpers::AddParamValue2ParamDict (APINULLGuid, param, paramDict);
            ParamHelpers::AddParamValue2ParamDict (APINULLGuid, param_onoff_elem, paramDict);
            msg_rep ("RoomBook", "Find property with the description 'some_stuff_element_onoff'", NoError, APINULLGuid);
        }
        ParamHelpers::AddParamValue2ParamDict (APINULLGuid, param_composite, paramDict);
    }

    // Готовит варианты имён параметров отделки зоны, свойств для записи результатов и флагов создания.
    // Разрешает описания свойств в rawnames через кеш определений.
    ReadParams Param_GetForRooms () {
        ReadParams zoneparams;
        ReadParam zoneparam;
        GS::UniString zoneparam_name;

        // Включение потолка
        zoneparam_name = "has_ceil";
        zoneparam.rawnames.Push ("some_stuff_fin_has_ceil");
        zoneparam.rawnames.Push ("{@gdl:ispot}");
        zoneparams.Add (zoneparam_name, zoneparam);
        zoneparam.rawnames.Clear ();

        // Включение пола
        zoneparam_name = "has_floor";
        zoneparam.rawnames.Push ("some_stuff_fin_has_floor");
        zoneparam.rawnames.Push ("{@gdl:ispol}");
        zoneparams.Add (zoneparam_name, zoneparam);
        zoneparam.rawnames.Clear ();

        // Включение потолка только по плитам
        zoneparam_name = "floor_by_slab";
        zoneparam.rawnames.Push ("some_stuff_fin_floor_by_slab");
        zoneparam.rawnames.Push ("{@gdl:floor_by_slab}");
        zoneparams.Add (zoneparam_name, zoneparam);
        zoneparam.rawnames.Clear ();

        // Включение потолка только по плитам
        zoneparam_name = "ceil_by_slab";
        zoneparam.rawnames.Push ("some_stuff_fin_ceil_by_slab");
        zoneparam.rawnames.Push ("{@gdl:ceil_by_slab}");
        zoneparams.Add (zoneparam_name, zoneparam);
        zoneparam.rawnames.Clear ();

        // Тип отделки
        zoneparam_name = "tip_otd";
        zoneparam.rawnames.Push ("{@gdl:tip_otd}");
        zoneparam.rawnames.Push ("some_stuff_fin_type");
        zoneparams.Add (zoneparam_name, zoneparam);
        zoneparam.rawnames.Clear ();

        // Тип потолка
        zoneparam_name = "tip_pot";
        zoneparam.rawnames.Push ("some_stuff_fin_type_ceil");
        zoneparam.rawnames.Push ("{@gdl:tip_pot}");
        zoneparams.Add (zoneparam_name, zoneparam);
        zoneparam.rawnames.Clear ();

        // Тип пола
        zoneparam_name = "tip_pol";
        zoneparam.rawnames.Push ("some_stuff_fin_type_floor");
        zoneparam.rawnames.Push ("{@gdl:tip_pol}");
        zoneparams.Add (zoneparam_name, zoneparam);
        zoneparam.rawnames.Clear ();

        // Основная отделка стен
        zoneparam_name = "om_main";
        zoneparam.rawnames.Push ("some_stuff_fin_main_material");
        zoneparam.rawnames.Push ("{@gdl:stwallmat}");
        zoneparam.rawnames.Push ("{@gdl:votw}");
        zoneparams.Add (zoneparam_name, zoneparam);
        zoneparam.rawnames.Clear ();

        // Высота до подвесного потолка
        zoneparam_name = "height_main";
        zoneparam.rawnames.Push ("some_stuff_fin_main_height");
        zoneparam.rawnames.Push ("{@gdl:hroom_pot}");
        zoneparams.Add (zoneparam_name, zoneparam);
        zoneparam.rawnames.Clear ();

        // Отделка стен выше потолка
        zoneparam_name = "om_up";
        zoneparam.rawnames.Push ("some_stuff_fin_up_material");
        zoneparam.rawnames.Push ("{@gdl:votw2}");
        zoneparams.Add (zoneparam_name, zoneparam);
        zoneparam.rawnames.Clear ();

        // Отделка низа стен/колонн
        zoneparam_name = "om_down";
        zoneparam.rawnames.Push ("some_stuff_fin_down_material");
        zoneparam.rawnames.Push ("{@gdl:stwalldownmat}");
        zoneparam.rawnames.Push ("{@gdl:votp}");
        zoneparams.Add (zoneparam_name, zoneparam);
        zoneparam.rawnames.Clear ();

        // Высота нижних панелей
        zoneparam_name = "height_down";
        zoneparam.rawnames.Push ("some_stuff_fin_down_height");
        zoneparam.rawnames.Push ("{@gdl:hpan}");
        zoneparam.rawnames.Push ("{@gdl:z17}");
        zoneparams.Add (zoneparam_name, zoneparam);
        zoneparam.rawnames.Clear ();

        // Наличие нижней отделки (химера)
        zoneparam_name = "him_has_height_down";
        zoneparam.rawnames.Push ("{@gdl:busewalldown}");
        zoneparams.Add (zoneparam_name, zoneparam);
        zoneparam.rawnames.Clear ();

        // Высота нижних панелей (химера)
        zoneparam_name = "him_height_down";
        zoneparam.rawnames.Push ("{@gdl:walldownhigh}");
        zoneparams.Add (zoneparam_name, zoneparam);
        zoneparam.rawnames.Clear ();

        // Отделка колонн
        zoneparam_name = "om_column";
        zoneparam.rawnames.Push ("some_stuff_fin_column_material");
        zoneparam.rawnames.Push ("{@gdl:stcolumnmat}");
        zoneparam.rawnames.Push ("{@gdl:votc}");
        zoneparams.Add (zoneparam_name, zoneparam);
        zoneparam.rawnames.Clear ();

        // Отделка потолка
        zoneparam_name = "om_ceil";
        zoneparam.rawnames.Push ("some_stuff_fin_ceil_material");
        zoneparam.rawnames.Push ("{@gdl:stupmat}");
        zoneparam.rawnames.Push ("{@gdl:vots}");
        zoneparam.rawnames.Push ("{@gdl:vots_fill}");
        zoneparam.rawnames.Push ("{@gdl:vots_bmat}");
        zoneparams.Add (zoneparam_name, zoneparam);
        zoneparam.rawnames.Clear ();

        // Отделка потолка - запись результатов
        zoneparam_name = "om_ceil.rawname";
        zoneparam.rawnames.Push ("some_stuff_fin_ceil_result");
        zoneparams.Add (zoneparam_name, zoneparam);
        zoneparam.rawnames.Clear ();

        zoneparam_name = "om_ceil.rawname_bytype";
        zoneparam.rawnames.Push ("some_stuff_fin_ceil_result_bytype");
        zoneparams.Add (zoneparam_name, zoneparam);
        zoneparam.rawnames.Clear ();

        // Отделка откосов - запись результатов
        zoneparam_name = "om_reveals.rawname";
        zoneparam.rawnames.Push ("some_stuff_fin_reveal_result");
        zoneparam.rawnames.Push ("some_stuff_fin_reveals_result");
        zoneparam.rawnames.Push ("some_stuff_fin_main_result");
        zoneparams.Add (zoneparam_name, zoneparam);
        zoneparam.rawnames.Clear ();

        zoneparam_name = "om_reveals.rawname_bytype";
        zoneparam.rawnames.Push ("some_stuff_fin_reveal_result_bytype");
        zoneparam.rawnames.Push ("some_stuff_fin_reveals_result_bytype");
        zoneparam.rawnames.Push ("some_stuff_fin_main_result_bytype");
        zoneparams.Add (zoneparam_name, zoneparam);
        zoneparam.rawnames.Clear ();

        // Отделка выше потолка - запись результатов
        zoneparam_name = "om_up.rawname";
        zoneparam.rawnames.Push ("some_stuff_fin_up_result");
        zoneparam.rawnames.Push ("some_stuff_fin_main_result");
        zoneparams.Add (zoneparam_name, zoneparam);
        zoneparam.rawnames.Clear ();

        zoneparam_name = "om_up.rawname_bytype";
        zoneparam.rawnames.Push ("some_stuff_fin_up_result_bytype");
        zoneparam.rawnames.Push ("some_stuff_fin_main_result_bytype");
        zoneparams.Add (zoneparam_name, zoneparam);
        zoneparam.rawnames.Clear ();

        // Отделка стен - запись результатов
        zoneparam_name = "om_main.rawname";
        zoneparam.rawnames.Push ("some_stuff_fin_main_result");
        zoneparams.Add (zoneparam_name, zoneparam);
        zoneparam.rawnames.Clear ();

        zoneparam_name = "om_main.rawname_bytype";
        zoneparam.rawnames.Push ("some_stuff_fin_main_result_bytype");
        zoneparams.Add (zoneparam_name, zoneparam);
        zoneparam.rawnames.Clear ();

        // Отделка низа стен/колонн - запись результатов
        zoneparam_name = "om_down.rawname";
        zoneparam.rawnames.Push ("some_stuff_fin_down_result");
        zoneparam.rawnames.Push ("some_stuff_fin_main_result");
        zoneparams.Add (zoneparam_name, zoneparam);
        zoneparam.rawnames.Clear ();

        zoneparam_name = "om_down.rawname_bytype";
        zoneparam.rawnames.Push ("some_stuff_fin_down_result_bytype");
        zoneparam.rawnames.Push ("some_stuff_fin_main_result_bytype");
        zoneparams.Add (zoneparam_name, zoneparam);
        zoneparam.rawnames.Clear ();

        // Отделка колонн - запись результатов
        zoneparam_name = "om_column.rawname";
        zoneparam.rawnames.Push ("some_stuff_fin_column_result");
        zoneparam.rawnames.Push ("some_stuff_fin_main_result");
        zoneparams.Add (zoneparam_name, zoneparam);
        zoneparam.rawnames.Clear ();

        zoneparam_name = "om_column.rawname_bytype";
        zoneparam.rawnames.Push ("some_stuff_fin_column_result_bytype");
        zoneparam.rawnames.Push ("some_stuff_fin_main_result_bytype");
        zoneparams.Add (zoneparam_name, zoneparam);
        zoneparam.rawnames.Clear ();

        // Создание всех элементов
        zoneparam_name = "create_all_elements";
        zoneparam.rawnames.Push ("some_stuff_fin_create_elements");
        zoneparams.Add (zoneparam_name, zoneparam);
        zoneparam.rawnames.Clear ();

        // Создание отделки колонн
        zoneparam_name = "create_column_elements";
        zoneparam.rawnames.Push ("some_stuff_fin_create_column");
        zoneparams.Add (zoneparam_name, zoneparam);
        zoneparam.rawnames.Clear ();

        // Создание отделки стен
        zoneparam_name = "create_wall_elements";
        zoneparam.rawnames.Push ("some_stuff_fin_create_wall");
        zoneparams.Add (zoneparam_name, zoneparam);
        zoneparam.rawnames.Clear ();

        // Создание пола
        zoneparam_name = "create_ceil_elements";
        zoneparam.rawnames.Push ("some_stuff_fin_create_ceil");
        zoneparams.Add (zoneparam_name, zoneparam);
        zoneparam.rawnames.Clear ();

        // Создание пола
        zoneparam_name = "create_floor_elements";
        zoneparam.rawnames.Push ("some_stuff_fin_create_floor");
        zoneparams.Add (zoneparam_name, zoneparam);
        zoneparam.rawnames.Clear ();

        // Создание откосов
        zoneparam_name = "create_reveal_elements";
        zoneparam.rawnames.Push ("some_stuff_fin_create_reveal");
        zoneparams.Add (zoneparam_name, zoneparam);
        zoneparam.rawnames.Clear ();

        Param_Property_FindInParams (zoneparams);
        return zoneparams;
    }

    // Сохраняет имена с PVALPREFIX, остальные заменяет именами свойств, найденных по описанию.
    // Для параметров rawname записывает найденное имя как готовое значение и прекращает поиск.
    static void ResolveParameterRawNames (ReadParams &zoneparams, ParamDictValue &propertyParams) {
        GS::Array<GS::UniString> valid_rawnames; // список проверенных имён параметров
        for (auto &p : zoneparams) {
    #ifdef ServerMainVers_2800
            ReadParam &param = p.value;
            GS::UniString name = p.key;
    #else
            ReadParam &param = *p.value;
            GS::UniString name = *p.key;
    #endif
            valid_rawnames.Clear ();
            for (const GS::UniString &param_name : param.rawnames) {
                if (param_name.Contains (PVALPREFIX)) {
                    // Если это gdl парметр - добавляем
                    valid_rawnames.Push (param_name);
                } else {
                    bool flag_find = false;
                    for (auto &cItt : propertyParams) {
    #ifdef ServerMainVers_2800
                        ParamValue &parameters = cItt.value;
    #else
                        ParamValue &parameters = *cItt.value;
    #endif
                        if (parameters.definition.description.Contains (param_name) &&
                            !parameters.definition.description.Contains (param_name + "_")) {
                            // В дланном случае нам нужно только имя свойства,
                            // считывать его не нужно
                            if (name.Contains ("rawname")) {
                                param.val.uniStringValue = parameters.rawName;
                                param.isValid = true;
                            }
                            valid_rawnames.Push (parameters.rawName);
                            break;
                        }
                    }
                }
                if (param.isValid) {
                    break;
                }
            }
            param.rawnames = valid_rawnames;
        }
    }

    // Разрешает имена параметров через кеш свойств, если список непуст и определения прочитаны.
    void Param_Property_FindInParams (ReadParams &zoneparams) {
        if (zoneparams.IsEmpty ())
            return;
        if (!ParamHelpers::isPropertyDefinitionRead ())
            return;
        ParamDictValue &propertyParams = PROPERTYCACHE ().property;
        ResolveParameterRawNames (zoneparams, propertyParams);
    }

    // Заполняет непрочитанные параметры первым валидным значением из их rawnames и сообщает, найдено ли такое значение.
    // Отбрасывает выключенные vots/vots_fill; предполагает наличие словаря элемента в paramToRead.
    bool Param_Property_Read (const API_Guid &elGuid, ParamDictElement &paramToRead, ReadParams &zoneparams) {
        if (zoneparams.IsEmpty ())
            return false;
        bool flag = false;
        // Прочитанные параметры базового компонента
        const auto *baseparam = paramToRead.GetPtr (elGuid);

        for (auto &p : zoneparams) {
    #ifdef ServerMainVers_2800
            ReadParam &param = p.value;
    #else
            ReadParam &param = *p.value;
    #endif
            if (param.isValid)
                continue; // Пропуск уже прочитанных свойств с именами свойств для
                          // записи
            for (const auto &param_name : param.rawnames) {
                if (const auto *p = baseparam->GetPtr (param_name)) {
                    if (p->isValid) {
                        if (param_name.Contains ("{@gdl:vots}") || param_name.Contains ("{@gdl:vots_fill}")) {
                            if (!p->val.boolValue)
                                continue;
                        }
                        flag = true;
                        param.isValid = true;
                        param.val = p->val;
                        if (p->fromProperty) {
                            param.val.type = API_PropertyStringValueType;
                        }
                        break;
                    }
                }
            }
        }
        return flag;
    }

    // Для строкового значения получает индекс покрытия по имени, используя и пополняя material_dict.
    void Param_Material_Get (GS::HashTable<GS::UniString, GS::Int32> &material_dict, ParamValueData &val) {
        if (val.type != API_PropertyStringValueType)
            return;
        if (const auto *m = material_dict.GetPtr (val.uniStringValue)) {
            val.intValue = *m;
        } else {
            API_AttrTypeID type = API_MaterialID;
            API_AttributeIndex attribinx;
    #ifdef ServerMainVers_2700
            attribinx = ACAPI_CreateAttributeIndex (val.intValue);
    #else
            attribinx = val.intValue;
    #endif
            API_AttributeIndexFindByName (val.uniStringValue, type, attribinx);
    #ifdef ServerMainVers_2700
            val.intValue = attribinx.ToInt32_Deprecated ();
    #else
            val.intValue = attribinx;
    #endif
            material_dict.Add (val.uniStringValue, val.intValue);
        }
    }

    // -----------------------------------------------------------------------------
    // Запись прочитанных свойств в зону
    // -----------------------------------------------------------------------------
    // Пересчитывает высоты основной и верхней отделки с учётом нижней полосы и округляет высоты до миллиметров.
    // При высоте комнаты меньше 0.0001 снимает признак валидности.
    static void NormalizeRoomFinishHeights (OtdRoom &roominfo) {
        // Заполнение непрочитанных
        // Высоты
        if (roominfo.height_main > roominfo.height)
            roominfo.height_main = roominfo.height;
        if (roominfo.height_main < 0.1) {
            roominfo.height_main = roominfo.height - roominfo.height_down;
            roominfo.height_up = 0;
        } else {
            roominfo.height_up = roominfo.height - roominfo.height_main;
            roominfo.height_main = roominfo.height_main - roominfo.height_down;
        }
        if (roominfo.height < 0.0001)
            roominfo.isValid = false;
        roominfo.height_ceil = roominfo.height_main + roominfo.height_down;
        roominfo.height = std::round (roominfo.height * 1000) / 1000;
        roominfo.height_main = std::round (roominfo.height_main * 1000) / 1000;
        roominfo.height_down = std::round (roominfo.height_down * 1000) / 1000;
        roominfo.height_up = std::round (roominfo.height_up * 1000) / 1000;
        roominfo.height_ceil = std::round (roominfo.height_ceil * 1000) / 1000;
    }

    // Переносит валидные флаги создания элементов в комнату.
    // Найденный create_all_elements заменяет все частные флаги, даже если его значение false.
    static void ApplyRoomCreationFlags (const ReadParams &readparams, OtdRoom &roominfo) {
        GS::UniString param_name;
        ParamValueData val;
        bool find_create_all_elements = false;
        bool create_all_elements = true;
        param_name = "create_all_elements";
        if (const auto *p = readparams.GetPtr (param_name)) {
            if (p->isValid) {
                val = p->val;
                find_create_all_elements = true;
                create_all_elements = val.boolValue;
            }
        }
        // Если найдено свойство create_all_elements, то читать смысла нет
        if (!find_create_all_elements) {
            param_name = "create_column_elements";
            if (const auto *p = readparams.GetPtr (param_name)) {
                if (p->isValid) {
                    val = p->val;
                    roominfo.create_column_elements = val.boolValue;
                }
            }
            param_name = "create_wall_elements";
            if (const auto *p = readparams.GetPtr (param_name)) {
                if (p->isValid) {
                    val = p->val;
                    roominfo.create_wall_elements = val.boolValue;
                }
            }
            param_name = "create_floor_elements";
            if (const auto *p = readparams.GetPtr (param_name)) {
                if (p->isValid) {
                    val = p->val;
                    roominfo.create_floor_elements = val.boolValue;
                }
            }
            param_name = "create_ceil_elements";
            if (const auto *p = readparams.GetPtr (param_name)) {
                if (p->isValid) {
                    val = p->val;
                    roominfo.create_ceil_elements = val.boolValue;
                }
            }
            param_name = "create_reveal_elements";
            if (const auto *p = readparams.GetPtr (param_name)) {
                if (p->isValid) {
                    val = p->val;
                    roominfo.create_reveal_elements = val.boolValue;
                }
            }
        }
        if (find_create_all_elements) {
            roominfo.create_ceil_elements = create_all_elements;   // Создавать элементы отделки потолка
            roominfo.create_wall_elements = create_all_elements;   // Создавать элементы отделки стен
            roominfo.create_column_elements = create_all_elements; // Создавать элементы отделки колонн
            roominfo.create_reveal_elements = create_all_elements; // Создавать элементы отделки откосов
            roominfo.create_floor_elements = create_all_elements;  // Создавать элементы отделки пола
        }
    }

    // Переносит валидные имена свойств для записи общих результатов и результатов по типам в материалы комнаты.
    static void ApplyRoomMaterialRawNames (const ReadParams &readparams, OtdRoom &roominfo) {
        GS::UniString param_name;
        ParamValueData val;
        param_name = "om_ceil.rawname";
        if (const auto *p = readparams.GetPtr (param_name)) {
            if (p->isValid) {
                val = p->val;
                roominfo.om_ceil.rawname = val.uniStringValue;
            }
        }
        param_name = "om_reveals.rawname";
        if (const auto *p = readparams.GetPtr (param_name)) {
            if (p->isValid) {
                val = p->val;
                roominfo.om_reveals.rawname = val.uniStringValue;
            }
        }
        param_name = "om_up.rawname";
        if (const auto *p = readparams.GetPtr (param_name)) {
            if (p->isValid) {
                val = p->val;
                roominfo.om_up.rawname = val.uniStringValue;
            }
        }
        param_name = "om_main.rawname";
        if (const auto *p = readparams.GetPtr (param_name)) {
            if (p->isValid) {
                val = p->val;
                roominfo.om_main.rawname = val.uniStringValue;
            }
        }
        param_name = "om_down.rawname";
        if (const auto *p = readparams.GetPtr (param_name)) {
            if (p->isValid) {
                val = p->val;
                roominfo.om_down.rawname = val.uniStringValue;
            }
        }
        param_name = "om_column.rawname";
        if (const auto *p = readparams.GetPtr (param_name)) {
            if (p->isValid) {
                val = p->val;
                roominfo.om_column.rawname = val.uniStringValue;
            }
        }

        param_name = "om_ceil.rawname_bytype";
        if (const auto *p = readparams.GetPtr (param_name)) {
            if (p->isValid) {
                val = p->val;
                roominfo.om_ceil.rawname_bytype = val.uniStringValue;
            }
        }
        param_name = "om_reveals.rawname_bytype";
        if (const auto *p = readparams.GetPtr (param_name)) {
            if (p->isValid) {
                val = p->val;
                roominfo.om_reveals.rawname_bytype = val.uniStringValue;
            }
        }
        param_name = "om_up.rawname_bytype";
        if (const auto *p = readparams.GetPtr (param_name)) {
            if (p->isValid) {
                val = p->val;
                roominfo.om_up.rawname_bytype = val.uniStringValue;
            }
        }
        param_name = "om_main.rawname_bytype";
        if (const auto *p = readparams.GetPtr (param_name)) {
            if (p->isValid) {
                val = p->val;
                roominfo.om_main.rawname_bytype = val.uniStringValue;
            }
        }
        param_name = "om_down.rawname_bytype";
        if (const auto *p = readparams.GetPtr (param_name)) {
            if (p->isValid) {
                val = p->val;
                roominfo.om_down.rawname_bytype = val.uniStringValue;
            }
        }
        param_name = "om_column.rawname_bytype";
        if (const auto *p = readparams.GetPtr (param_name)) {
            if (p->isValid) {
                val = p->val;
                roominfo.om_column.rawname_bytype = val.uniStringValue;
            }
        }
    }

    // Переносит высоты нижней и основной отделки; при отсутствии height_down использует параметры химеры.
    static void ApplyRoomHeightParameters (const ReadParams &readparams, OtdRoom &roominfo) {
        GS::UniString param_name;
        ParamValueData val;
        bool has_height_down = false;

        param_name = "height_down";
        if (const auto *p = readparams.GetPtr (param_name)) {
            if (p->isValid) {
                val = p->val;
                roominfo.height_down = val.doubleValue;
                has_height_down = true;
            }
        }

        if (!has_height_down) {
            param_name = "him_has_height_down";
            if (const auto *p = readparams.GetPtr (param_name)) {
                if (p->isValid) {
                    val = p->val;
                    bool him_has_height_down = val.boolValue;
                    param_name = "him_height_down";
                    if (readparams.ContainsKey (param_name)) {
                        if (readparams.Get (param_name).isValid) {
                            val = readparams.Get (param_name).val;
                            roominfo.height_down = val.doubleValue * him_has_height_down;
                        }
                    }
                }
            }
        }
        param_name = "height_main";
        if (const auto *p = readparams.GetPtr (param_name)) {
            if (p->isValid) {
                val = p->val;
                roominfo.height_main = val.doubleValue;
            }
        }
    }

    // Переносит типы отделки и признаки наличия пола и потолка.
    // Флаги отделки только по плитам применяет лишь при включённом соответствующем виде отделки.
    static void ApplyRoomFinishTypeParameters (const ReadParams &readparams, OtdRoom &roominfo) {
        GS::UniString param_name;
        ParamValueData val;

        param_name = "tip_pot";
        if (const auto *p = readparams.GetPtr (param_name)) {
            if (p->isValid) {
                val = p->val;
                roominfo.tip_pot = val.uniStringValue;
            }
        }

        param_name = "tip_otd";
        if (const auto *p = readparams.GetPtr (param_name)) {
            if (p->isValid) {
                val = p->val;
                roominfo.tip_otd = val.uniStringValue;
            }
        }
        param_name = "tip_pol";
        if (const auto *p = readparams.GetPtr (param_name)) {
            if (p->isValid) {
                val = p->val;
                roominfo.tip_pol = val.uniStringValue;
            }
        }
        param_name = "has_ceil";
        if (const auto *p = readparams.GetPtr (param_name)) {
            if (p->isValid) {
                val = p->val;
                roominfo.has_ceil = val.boolValue;
            }
        }
        param_name = "has_floor";
        if (const auto *p = readparams.GetPtr (param_name)) {
            if (p->isValid) {
                val = p->val;
                roominfo.has_floor = val.boolValue;
            }
        }
        param_name = "ceil_by_slab";
        if (const auto *p = readparams.GetPtr (param_name)) {
            if (p->isValid) {
                val = p->val;
                if (roominfo.has_ceil)
                    roominfo.ceil_by_slab = val.boolValue;
            }
        }

        param_name = "floor_by_slab";
        if (const auto *p = readparams.GetPtr (param_name)) {
            if (p->isValid) {
                val = p->val;
                if (roominfo.has_floor)
                    roominfo.floor_by_slab = val.boolValue;
            }
        }
    }

    // Переносит валидные материалы колонн, нижней, основной, верхней отделки и потолка в комнату.
    // Разрешает имена покрытий в индексы и добавляет префикс 0&#& к строковым обозначениям.
    static void ApplyRoomMaterialParameters (GS::HashTable<GS::UniString, GS::Int32> &material_dict,
                                             const ReadParams &readparams,
                                             OtdRoom &roominfo) {
        GS::UniString param_name;
        ParamValueData val;
        param_name = "om_column";
        if (const auto *p = readparams.GetPtr (param_name)) {
            if (p->isValid) {
                val = p->val;
                Param_Material_Get (material_dict, val);
                val.uniStringValue.Trim ();
                val.uniStringValue = "0&#& " + val.uniStringValue;
                roominfo.om_column.smaterial = val.uniStringValue;
                roominfo.om_column.material = val.intValue;
            }
        }
        param_name = "om_down";
        if (const auto *p = readparams.GetPtr (param_name)) {
            if (p->isValid) {
                val = p->val;
                Param_Material_Get (material_dict, val);
                val.uniStringValue.Trim ();
                val.uniStringValue = "0&#& " + val.uniStringValue;
                roominfo.om_down.material = val.intValue;
                roominfo.om_down.smaterial = val.uniStringValue;
            }
        }
        param_name = "om_main";
        if (const auto *p = readparams.GetPtr (param_name)) {
            if (p->isValid) {
                val = p->val;
                Param_Material_Get (material_dict, val);
                val.uniStringValue.Trim ();
                val.uniStringValue = "0&#& " + val.uniStringValue;
                roominfo.om_main.material = val.intValue;
                roominfo.om_main.smaterial = val.uniStringValue;
            }
        }
        param_name = "om_ceil";
        if (const auto *p = readparams.GetPtr (param_name)) {
            if (p->isValid) {
                val = p->val;
                Param_Material_Get (material_dict, val);
                val.uniStringValue.Trim ();
                val.uniStringValue = "0&#& " + val.uniStringValue;
                roominfo.om_ceil.material = val.intValue;
                roominfo.om_ceil.smaterial = val.uniStringValue;
            }
        }
        param_name = "om_up";
        if (const auto *p = readparams.GetPtr (param_name)) {
            if (p->isValid) {
                val = p->val;
                Param_Material_Get (material_dict, val);
                val.uniStringValue.Trim ();
                val.uniStringValue = "0&#& " + val.uniStringValue;
                roominfo.om_up.material = val.intValue;
                roominfo.om_up.smaterial = val.uniStringValue;
            }
        }
    }

    // Читает параметры зоны, применяет типы, материалы, высоты, имена результатов и флаги создания.
    // При отсутствии прочитанных значений сообщает об ошибке; иначе нормализует высоты отделки.
    void Param_SetToRooms (GS::HashTable<GS::UniString, GS::Int32> &material_dict,
                           OtdRoom &roominfo,
                           ParamDictElement &paramToRead,
                           ReadParams &readparams) {
        API_Guid base_guid = roominfo.zone_guid;
        if (!Param_Property_Read (base_guid, paramToRead, readparams)) {
            msg_rep ("Roombook", "Can't read zone params", NoError, base_guid);
            return;
        }
        ApplyRoomFinishTypeParameters (readparams, roominfo);

        ApplyRoomMaterialParameters (material_dict, readparams, roominfo);
        ApplyRoomHeightParameters (readparams, roominfo);
        ApplyRoomMaterialRawNames (readparams, roominfo);
        ApplyRoomCreationFlags (readparams, roominfo);
        NormalizeRoomFinishHeights (roominfo);
    }

    // -----------------------------------------------------------------------------
    // Запись прочитанных свойств в отделочные стены
    // -----------------------------------------------------------------------------
    // Проверяет доступность состава и флаги базового элемента, затем подготавливает слои отделки и имя избранного.
    // Возвращает false только при валидном выключенном флаге element_onoff; отсутствие данных даёт true.
    bool Param_SetToBase (const API_Guid &base_guid,
                          bool base_flipped,
                          GS::Array<ParamValueComposite> &otdComposite,
                          ParamDictElement &paramToRead,
                          ParamDictCompositeElement &paramCompositeToRead,
                          const ParamValue &param_composite,
                          GS::UniString &fav_name,
                          OtdWall *floorWall) {
        if (floorWall != nullptr) {
            floorWall->floor_core_offset = 0;
            floorWall->floor_core_offset_valid = false;
        }
        ParamDictValue *baseparamPtr = paramToRead.GetPtr (base_guid);
        if (baseparamPtr == nullptr) {
    #if defined(TESTING)
            DBprnt ("Param_SetToBase err", "!paramToRead.ContainsKey(base_guid)");
    #endif
            return true;
        }
        ParamDictValue &baseparam = *baseparamPtr;

        // Проверяем наличие состава в параметрах
        if (!baseparam.ContainsKey (param_composite.rawName)) {
            return true;
        }

        // получаем состав элемента
        auto *compositeMapPtr = paramCompositeToRead.GetPtr (base_guid);
        if (compositeMapPtr == nullptr) {
    #if defined(TESTING)
            DBprnt ("Param_SetToBase err", "!paramCompositeToRead.ContainsKey(base_guid)");
    #endif
            return true;
        }

        const ParamComposite *base_composite_ptr = compositeMapPtr->GetPtr (param_composite.rawName);
        if (base_composite_ptr == nullptr) {
    #if defined(TESTING)
            DBprnt ("Param_SetToBase err", "!compositeMap.ContainsKey(param_composite.rawName)");
    #endif
            return true;
        }

        // проверка флага ON/OFF
        if (const ParamValue *flagOnOff = baseparam.GetPtr ("{@flag:rawname_element_onoff}")) {
            if (const ParamValue *is_fin = baseparam.GetPtr (flagOnOff->val.uniStringValue)) {
                if (is_fin->isValid && !is_fin->val.boolValue) {
                    return false;
                }
            }
        }

        // проверка флага наличия отделки
        bool has_fin = true;
        if (const ParamValue *flagHasFin = baseparam.GetPtr ("{@flag:rawname_hasfin_elem}")) {
            if (const ParamValue *is_fin = baseparam.GetPtr (flagHasFin->val.uniStringValue)) {
                if (is_fin->isValid && !is_fin->val.boolValue) {
                    has_fin = false;
                }
            }
        }
        // Отдельная глубина до фильтрации h/l и до добавления состава избранного.
        if (floorWall != nullptr && !base_composite_ptr->composite.IsEmpty () &&
            base_composite_ptr->composite_type == API_CompositeStructure && floorWall->base_type == API_WallID &&
            !floorWall->slab_step) {
            floorWall->floor_core_offset_valid = FloorCoreGeometry::Offset (base_composite_ptr->composite,
                                                                            base_composite_ptr->composite.GetSize (),
                                                                            base_flipped,
                                                                            APICWallComp_Core,
                                                                            APICWallComp_Finish,
                                                                            floorWall->floor_core_offset);
        }
        Param_SetComposite (*base_composite_ptr, base_flipped, otdComposite, fav_name, has_fin);
        return true;
    }

    // -----------------------------------------------------------------------------
    // Запись прочитанных свойств в отделочные стены
    // -----------------------------------------------------------------------------
    // Добавляет слои от выбранной стороны до первого слоя с точным типом APICWallComp_Core включительно.
    // Уже накопленные слои не очищает; если ядра нет, добавляет весь состав.
    static void CollectBaseCompositeLayers (const ParamComposite &base_composite,
                                            bool base_flipped,
                                            Int32 ncomp,
                                            GS::Array<ParamValueComposite> &cpmpoosite) {
        bool flag_core = false;
        if (base_flipped) {
            for (Int32 j = ncomp - 1; j >= 0; j--) {
                if (!flag_core) {
                    cpmpoosite.Push (base_composite.composite[j]);
                    if (base_composite.composite[j].structype == APICWallComp_Core)
                        flag_core = true;
                }
            }
        } else {
            for (Int32 j = 0; j < ncomp; j++) {
                if (!flag_core) {
                    cpmpoosite.Push (base_composite.composite[j]);
                    if (base_composite.composite[j].structype == APICWallComp_Core)
                        flag_core = true;
                }
            }
        }
    }

    // Разбирает метки l[1:], h и f в слоях выбранной стороны и добавляет результат к переданному составу.
    // Собирает имя избранного; для последнего слоя с structype == -1 читает покрытие строительного материала.
    void Param_SetComposite (const ParamComposite &base_composite,
                             bool base_flipped,
                             GS::Array<ParamValueComposite> &otdcpmpoosite,
                             GS::UniString &fav_name,
                             bool has_fin) {
        Int32 ncomp = base_composite.composite.GetSize ();
        if (ncomp == 0)
            return;
        fav_name.Clear ();
        GS::Array<ParamValueComposite> cpmpoosite;
        CollectBaseCompositeLayers (base_composite, base_flipped, ncomp, cpmpoosite);
        for (UInt32 j = 0; j < cpmpoosite.GetSize (); j++) {
            GS::UniString v = cpmpoosite[j].val;
            bool fin_add = true;
            if (v.Contains ("h[0]"))
                fin_add = false;
            v.ReplaceAll ("h[1]", EMPTYSTRING);
            v.ReplaceAll ("h[0]", EMPTYSTRING);
            v.ReplaceAll ("h[]", EMPTYSTRING);
            if (v.Contains ("f[]")) {
                v = v.GetPrefix (v.GetLength () - 5);
                v = v.GetSuffix (v.GetLength () - 4);
            } else {
                v.ReplaceAll ("f[", ATSIGN);
                GS::UniString fav_part = v.GetSubstring ('@', ']', 0);
                fav_part.Trim ();
                if (!fav_part.IsEmpty ()) {
                    if (fav_name.IsEmpty ()) {
                        fav_name = fav_part;
                    } else {
                        fav_name.Append ("_");
                        fav_name.Append (fav_part);
                    }
                }
                v.ReplaceAll ("l[1:", ATSIGN);
                v = v.GetSubstring ('@', ']', 0);
            }
            if (cpmpoosite[j].val.Contains ("l[1:")) {
                cpmpoosite[j].val = v;
                if (!fin_add)
                    cpmpoosite[j].structype = -1;
                otdcpmpoosite.Push (cpmpoosite[j]);
            }
        }
        if (!fav_name.IsEmpty ())
            fav_name.SetToLowerCase ();
        // Получаем индекс и имя прокрытия для последнего слоя
        ParamValueComposite last;
        if (otdcpmpoosite.IsEmpty () && !has_fin) {
            last = cpmpoosite.GetLast ();
            last.structype = -1;
        } else {
            if (otdcpmpoosite.IsEmpty ())
                return;
            last = otdcpmpoosite.GetLast ();
        }
        if (last.structype == -1) {
            API_Attribute attrib;
            attrib.header.typeID = API_BuildingMaterialID;
            attrib.header.index = last.inx;
            GSErrCode err = ACAPI_Attribute_Get (&attrib);
            if (err == NoError) {
                API_AttributeIndex cutMaterial = attrib.buildingMaterial.cutMaterial;
                GS::UniString attribname;
                BNZeroMemory (&attrib, sizeof (API_Attribute));
                attrib.header.typeID = API_MaterialID;
                attrib.header.index = cutMaterial;
                attrib.header.uniStringNamePtr = &attribname;
                err = ACAPI_Attribute_Get (&attrib);
                if (err == NoError) {
    #ifdef ServerMainVers_2700
                    last.length = cutMaterial.ToInt32_Deprecated ();
    #else
                    last.length = cutMaterial;
    #endif
                    last.pos = attribname;
                    last.val.Clear ();
                    if (otdcpmpoosite.IsEmpty ()) {
                        otdcpmpoosite.Push (last);
                    } else {
                        otdcpmpoosite.GetLast ().length = last.length;
                        otdcpmpoosite.GetLast ().pos = last.pos;
                    }
                } else {
                    msg_rep ("RoomBook", "ACAPI_Attribute_Get material " + last.val, err, APINULLGuid);
                }
            } else {
                msg_rep ("RoomBook", "ACAPI_Attribute_Get building material " + last.val, err, APINULLGuid);
            }
        }
        cpmpoosite.Clear ();
    }

    // -----------------------------------------------------------------------------
    // Задание прочитанных параметров для окон
    // -----------------------------------------------------------------------------
    // После проверки словаря задаёт глубину по толщине стены, затем уточняет её по frame, sill и стороне откоса.
    // При отсутствии словаря выходит без изменений; при отказе чтения или невалидных frame/sill оставляет толщину
    // стены.
    void Param_SetToWindows (OtdOpening &op,
                             ParamDictElement &paramToRead,
                             ReadParams &readparams,
                             const OtdWall &otdw) {
        API_Guid base_guid = op.base_guid;
        if (!paramToRead.ContainsKey (base_guid)) {
    #if defined(TESTING)
            DBprnt ("Param_SetToWindows err", "!paramToRead.ContainsKey(base_guid)");
    #endif
            return;
        }
        op.base_reveal_width = otdw.base_th;
        if (!Param_Property_Read (base_guid, paramToRead, readparams)) {
    #if defined(TESTING)
            DBprnt ("Param_SetToWindows err", "!Param_Property_Read");
    #endif
            return;
        }

        GS::UniString param_name;
        double frame = 0;
        double sill = 0;

        param_name = "frame";
        if (const auto *p = readparams.GetPtr (param_name)) {
            if (p->isValid) {
                frame = p->val.doubleValue;
            } else {
                return;
            }
        } else {
            return;
        }

        param_name = "sill";
        if (const auto *p = readparams.GetPtr (param_name)) {
            if (p->isValid) {
                sill = p->val.doubleValue;
            } else {
                return;
            }
        } else {
            return;
        }

        // Коробка разделяет откосы двух помещений: глубины зависят от стороны отделки.
        op.base_reveal_width = op.reveal_from_sill ? sill : otdw.base_th - sill - frame;

        // Включение/отключение создания откосов по свойству элемента.
        // Если свойства нет или оно не валидно — откосы создаются (create_reveal остаётся true).
        op.create_reveal = true;
        if (const auto *p = readparams.GetPtr ("reveal_onoff")) {
            if (p->isValid && !p->val.boolValue) {
                op.create_reveal = false;
            }
        }
    }

    // -----------------------------------------------------------------------------
    // Получение полигона зоны (в том числе стен, колонн)
    // -----------------------------------------------------------------------------
    #ifdef ServerMainVers_2700
    // Добавляет рёбра редукции в reducededges по типам rest, wall, column и gable; затем освобождает handles.
    // При отсутствии reducededges или некорректных массивах выходит до освобождения handles.
    void RoomReductionPolyProc (const API_RoomReductionPolyType *roomRed)
    #else
    // Добавляет рёбра редукции в reducededges по типам rest, wall, column и gable; затем освобождает handles.
    // При отсутствии reducededges или некорректных массивах выходит до освобождения handles.
    static void __ACENV_CALL RoomRedProc (const API_RoomReductionPolyType *roomRed)
    #endif
    {
        if (reducededges == nullptr) {
    #if defined(TESTING)
            DBprnt ("RoomRedProc err", "reducededges == nullptr");
    #endif
            return;
        }
        if (roomRed->nCoords < 4 || roomRed->coords == nullptr || roomRed->subPolys == nullptr) {
    #if defined(TESTING)
            DBprnt ("RoomRedProc err", "roomRed->coords == nullptr");
    #endif
            return;
        }
        Point2D begC = {0, 0};
        Point2D endC = {0, 0};
        Sector roomedge = {begC, endC};
        GS::Array<Sector> roomedges;
        for (Int32 j = 1; j <= roomRed->nSubPolys; j++) {
            UInt32 begInd = (*roomRed->subPolys)[j - 1] + 1;
            UInt32 endInd = (*roomRed->subPolys)[j];
            for (UInt32 k = begInd; k < endInd; k++) {
                begC = {(*roomRed->coords)[k].x, (*roomRed->coords)[k].y};
                endC = {(*roomRed->coords)[k + 1].x, (*roomRed->coords)[k + 1].y};
                roomedge = {begC, endC};
                roomedges.Push (roomedge);
            }
        }
        switch (roomRed->type) {
        case APIRoomReduction_Rest:
            reducededges->restedges.Append (std::move (roomedges));
            break;
        case APIRoomReduction_Wall:
            reducededges->walledges.Append (std::move (roomedges));
            break;
        case APIRoomReduction_Column:
            reducededges->columnedges.Append (std::move (roomedges));
            break;
        case APIRoomReduction_Hatch:
            break;
        case APIRoomReduction_Gable:
            reducededges->gableedges.Append (std::move (roomedges));
            break;
        default:
            break;
        }
        BMKillHandle ((GSHandle *)&roomRed->coords);
        BMKillHandle ((GSHandle *)&roomRed->subPolys);
        BMKillHandle ((GSHandle *)&roomRed->arcs);
        return;
    } // RoomRedProc

    // -----------------------------------------------------------------------------
    // Получение очищенного полигона зоны, включая стены, колонны
    // -----------------------------------------------------------------------------
    // Получает рёбра редукций зоны и оставляет рёбра стен, колонн и скатов, совпавшие с restedges редукции.
    // В restedges возвращает исходные рёбра полигона с нулевым разделителем после каждого контура.
    void Edges_GetFromRoom (const API_ElementMemo &zonememo,
                            API_Element &zoneelement,
                            GS::Array<Sector> &walledges,
                            GS::Array<Sector> &columnedges,
                            GS::Array<Sector> &restedges,
                            GS::Array<Sector> &gableedges) {
        RoomEdges rdges;
        reducededges = &rdges;
        GSErrCode err = NoError;
    #ifdef ServerMainVers_2700
        err = ACAPI_Element_RoomReductions (&zoneelement.header.guid, RoomReductionPolyProc);
    #else
        err = ACAPI_Database (APIDb_RoomReductionsID, &zoneelement.header.guid, (void *)(GS::IntPtr)RoomRedProc);
    #endif
        // Указатель действует только между присваиванием и возвратом из SDK-вызова:
        // rdges — локальный объект, оставлять его адрес в состоянии модуля нельзя.
        reducededges = nullptr;
        if (err != NoError) {
    #if defined(TESTING)
            DBprnt ("Edges_GetFromRoom err", "APIDb_RoomReductionsID");
    #endif
            return;
        }
        Point2D begC = {0, 0};
        Point2D endC = {0, 0};
        Sector roomedge = {begC, endC};
        GS::Array<Sector> roomedges;
        for (Int32 j = 1; j <= zoneelement.zone.poly.nSubPolys; j++) {
            UInt32 begInd = (*zonememo.pends)[j - 1] + 1;
            UInt32 endInd = (*zonememo.pends)[j];
            for (UInt32 k = begInd; k < endInd; k++) {
                begC = {(*zonememo.coords)[k].x, (*zonememo.coords)[k].y};
                endC = {(*zonememo.coords)[k + 1].x, (*zonememo.coords)[k + 1].y};
                roomedge = {begC, endC};
                roomedges.Push (roomedge);
            }
            // При подсчёте полигонов архикад выдаёт сдвижку на 1 для каждого
            // субполигона
            begC = {0, 0};
            endC = {0, 0};
            roomedge = {begC, endC};
            roomedges.Push (roomedge);
        }

        GS::Array<Sector> edges;

        for (auto &tedge : rdges.columnedges) {
            if (Edge_FindEdge (tedge, rdges.restedges))
                edges.Push (tedge);
        }
        columnedges = edges;
        edges.Clear ();

        for (auto &tedge : rdges.walledges) {
            if (Edge_FindEdge (tedge, rdges.restedges))
                edges.Push (tedge);
        }
        walledges = edges;
        edges.Clear ();

        for (auto &tedge : rdges.gableedges) {
            if (Edge_FindEdge (tedge, rdges.restedges))
                edges.Push (tedge);
        }
        gableedges = edges;
        edges.Clear ();
        restedges = roomedges;
    }

    // -----------------------------------------------------------------------------
    // Убираем задвоение Guid зон у элементов
    // -----------------------------------------------------------------------------
    void ClearZoneGUID (UnicElementByType &elementToRead) {
        UnicGuid zoneGuidsd;
        GS::Array<API_Guid> zoneGuids;
        for (const API_ElemTypeID &typeelem : typeinzone) {
            auto *elemsPtr = elementToRead.GetPtr (typeelem);
            if (elemsPtr == nullptr)
                continue;
            auto &elems = *elemsPtr;
            for (UnicElement::PairIterator cIt = elems.EnumeratePairs (); cIt != NULL; ++cIt) {
    #ifdef ServerMainVers_2800
                API_Guid guid = cIt->key;
                const GS::Array<API_Guid> &zoneGuids_ = cIt->value;
    #else
                API_Guid guid = *cIt->key;
                const GS::Array<API_Guid> &zoneGuids_ = *cIt->value;
    #endif
                zoneGuidsd.Clear ();
                zoneGuids.Clear ();
                for (const API_Guid &zoneGuid : zoneGuids_) {
                    zoneGuidsd.Add (zoneGuid, true);
                }
                for (UnicGuid::PairIterator cIt = zoneGuidsd.EnumeratePairs (); cIt != NULL; ++cIt) {
    #ifdef ServerMainVers_2800
                    const API_Guid &zoneGuid = cIt->key;
    #else
                    const API_Guid &zoneGuid = *cIt->key;
    #endif
                    zoneGuids.Push (zoneGuid);
                }
                elems.Set (guid, zoneGuids);
            }
        }
        return;
    }

    // -----------------------------------------------------------------------------
    // Создание стенок для откосов одного проёма
    // -----------------------------------------------------------------------------
    // Заполняет геометрию стенки откоса и наследует основание, этаж, материал и состав от отделочной стены.
    // Устанавливает тип основания API_WindowID, тип отделки Reveal_Main и заданный тип рисования.
    static void InitializeOpeningRevealWall (const OtdWall &otdw,
                                             OtdWall &wallotd,
                                             double height,
                                             double zBottom,
                                             double width,
                                             double length,
                                             const Point2D &begC,
                                             const Point2D &endC,
                                             API_ElemTypeID draw_type) {
        wallotd.base_guid = otdw.base_guid;
        wallotd.height = height;
        wallotd.zBottom = zBottom;
        wallotd.width = width;
        wallotd.length = length;
        wallotd.floorInd = otdw.floorInd;
        wallotd.begC = {begC.x, begC.y};
        wallotd.endC = {endC.x, endC.y};
        wallotd.material = otdw.material;
        wallotd.base_composite = otdw.base_composite;
        wallotd.base_type = API_WindowID;
        wallotd.draw_type = draw_type;
        wallotd.type = Reveal_Main;
    }

    // Вычисляет вертикальное пересечение проёма со стеной и признак выхода проёма выше стены.
    // Возвращает false, если высота пересечения меньше min_dim.
    static bool CalculateOpeningRevealHeight (
        const OtdWall &otdw, const OtdOpening &op, bool &is_upper_wall, double &zDown, double &height) {
        is_upper_wall = false; // Проём заканчивается выше стены, построение
                               // верхнего откоса не требуется
        if ((op.zBottom + op.height) > (otdw.zBottom + otdw.height))
            is_upper_wall = true;
        double zDup = fmin (op.zBottom + op.height, otdw.zBottom + otdw.height);
        zDown = fmax (op.zBottom, otdw.zBottom);
        height = fmin (op.height, zDup - zDown);
        if (height < min_dim)
            return false;
        return true;
    }

    // Вычисляет концы боковых откосов по положению и ширине проёма и заданному перпендикуляру стены.
    // При нулевой длине стены возвращает false.
    static bool CalculateOpeningRevealCoordinates (const OtdWall &otdw,
                                                   const OtdOpening &op,
                                                   const Geometry::Vector2<double> &walldir_perp,
                                                   Point2D &left_begin,
                                                   Point2D &left_end,
                                                   Point2D &right_begin,
                                                   Point2D &right_end) {
        double dx = -otdw.endC.x + otdw.begC.x;
        double dy = -otdw.endC.y + otdw.begC.y;
        double dr = sqrt (dx * dx + dy * dy);
        if (is_equal (dr, 0))
            return false;
        double lambda = 0;
        lambda = (op.objLoc - op.width / 2) / dr;
        left_begin.x = otdw.begC.x - dx * lambda;
        left_begin.y = otdw.begC.y - dy * lambda;
        left_end.x = left_begin.x + walldir_perp.x * op.base_reveal_width;
        left_end.y = left_begin.y + walldir_perp.y * op.base_reveal_width;
        lambda = (op.objLoc + op.width / 2) / dr;
        right_begin.x = otdw.begC.x - dx * lambda;
        right_begin.y = otdw.begC.y - dy * lambda;
        right_end.x = right_begin.x + walldir_perp.x * op.base_reveal_width;
        right_end.y = right_begin.y + walldir_perp.y * op.base_reveal_width;
        return true;
    }

    // Готовит боковые и, если проём не выходит выше стены, верхний откос, разбивая их по высотам отделки.
    // Пропускает недостаточную глубину или высоту и нулевую геометрию; при подготовке откоса ставит has_reveal.
    void OpeningReveals_Create_One (GS::Array<OtdSlab> &otdslabs,
                                    const OtdWall &otdw,
                                    OtdOpening &op,
                                    const Geometry::Vector2<double> &walldir_perp,
                                    GS::Array<OtdWall> &opw,
                                    double &otd_zBottom,
                                    double &otd_height_down,
                                    double &otd_height_main,
                                    double &otd_height_up,
                                    double &otd_height,
                                    OtdMaterial &om_main,
                                    OtdMaterial &om_up,
                                    OtdMaterial &om_down,
                                    OtdMaterial &om_reveals,
                                    OtdMaterial &om_column,
                                    OtdMaterial &om_floor,
                                    OtdMaterial &om_ceil,
                                    OtdMaterial &om_zone) {
        if (!op.create_reveal)
            return;
        if (op.base_reveal_width < min_dim)
            return;
        bool is_upper_wall = false;
        double zDown = 0;
        double height = 0;
        if (!CalculateOpeningRevealHeight (otdw, op, is_upper_wall, zDown, height))
            return;
        Point2D left_begin = {0, 0};
        Point2D left_end = {0, 0};
        Point2D right_begin = {0, 0};
        Point2D right_end = {0, 0};
        Sector walledge = {left_begin, left_end};
        // Строим перпендикулярные стенки к окну
        if (!CalculateOpeningRevealCoordinates (otdw, op, walldir_perp, left_begin, left_end, right_begin, right_end))
            return;
        walledge = {left_begin, left_end};
        if (!walledge.IsZeroLength ()) {
            OtdWall wallotd;
            InitializeOpeningRevealWall (
                otdw, wallotd, height, zDown, op.base_reveal_width, height, walledge.c1, walledge.c2, API_WallID);
            op.has_reveal = true;
            OtdWall_Delim_All (opw,
                               wallotd,
                               otd_zBottom,
                               otd_height_down,
                               otd_height_main,
                               otd_height_up,
                               otd_height,
                               om_main,
                               om_up,
                               om_down,
                               om_reveals,
                               om_column,
                               om_floor,
                               om_ceil,
                               om_zone);
        }
        walledge = {right_begin, right_end};
        if (!walledge.IsZeroLength ()) {
            OtdWall wallotd;
            InitializeOpeningRevealWall (
                otdw, wallotd, height, zDown, op.base_reveal_width, height, walledge.c2, walledge.c1, API_WallID);
            op.has_reveal = true;
            OtdWall_Delim_All (opw,
                               wallotd,
                               otd_zBottom,
                               otd_height_down,
                               otd_height_main,
                               otd_height_up,
                               otd_height,
                               om_main,
                               om_up,
                               om_down,
                               om_reveals,
                               om_column,
                               om_floor,
                               om_ceil,
                               om_zone);
        }
        walledge = {right_begin, left_begin};
        if (!walledge.IsZeroLength () && !is_upper_wall) {
            OtdWall wallotd;
            InitializeOpeningRevealWall (otdw,
                                         wallotd,
                                         otd_thickness,
                                         zDown + height,
                                         op.base_reveal_width,
                                         walledge.GetLength (),
                                         walledge.c2,
                                         walledge.c1,
                                         API_BeamID);
            op.has_reveal = true;
            OtdWall_Delim_All (opw,
                               wallotd,
                               otd_zBottom,
                               otd_height_down,
                               otd_height_main,
                               otd_height_up,
                               otd_height,
                               om_main,
                               om_up,
                               om_down,
                               om_reveals,
                               om_column,
                               om_floor,
                               om_ceil,
                               om_zone);
        }
    }

    // -----------------------------------------------------------------------------
    // Разбивка созданных стен по высотам на основании информации из зоны
    // -----------------------------------------------------------------------------
    // Преобразует тип высотной полосы в тип отделки с учётом основания и исходного типа пола или потолка.
    static TypeOtd GetWallFinishBandType (const OtdWall &otdw, TypeOtd type) {
        switch (type) {
        case Wall_Down:
            if (otdw.base_type == API_SlabID)
                type = Floor;
            if (otdw.base_type == API_WindowID)
                type = Reveal_Down;
            if (otdw.type == Ceil)
                type = Ceil;
            if (otdw.type == Floor)
                type = Floor;
            break;
        case Wall_Main:
            if (otdw.base_type == API_ColumnID)
                type = Column;
            if (otdw.base_type == API_WindowID)
                type = Reveal_Main;
            if (otdw.type == Ceil)
                type = Ceil;
            if (otdw.type == Floor)
                type = Floor;
            break;
        case Wall_Up:
            if (otdw.type == Ceil)
                type = Ceil;
            if (otdw.type == Floor)
                type = Floor;
            if (otdw.base_type == API_WindowID)
                type = Reveal_Up;
            break;
        default:
            break;
        }
        return type;
    }

    // Выбирает резервный тип отделки по основанию; исходные типы потолка и пола имеют приоритет.
    static TypeOtd GetFallbackWallFinishType (const OtdWall &otdw, TypeOtd type) {
        if (otdw.base_type == API_ColumnID)
            type = Column;
        if (otdw.base_type == API_WindowID)
            type = Reveal_Main;
        if (otdw.type == Ceil)
            type = Ceil;
        if (otdw.type == Floor)
            type = Floor;
        return type;
    }

    // Подрезает стенку по нижней, основной и верхней полосам отделки с положительной высотой.
    // Если ни одна полоса не добавлена, пробует подрезку по полной высоте комнаты.
    void OtdWall_Delim_All (GS::Array<OtdWall> &opw,
                            OtdWall &otdw,
                            double otd_zBottom,
                            double otd_height_down,
                            double otd_height_main,
                            double otd_height_up,
                            double otd_height,
                            OtdMaterial &om_main,
                            OtdMaterial &om_up,
                            OtdMaterial &om_down,
                            OtdMaterial &om_reveals,
                            OtdMaterial &om_column,
                            OtdMaterial &om_floor,
                            OtdMaterial &om_ceil,
                            OtdMaterial &om_zone) {
        TypeOtd type = otdw.type;
        bool has_delim = false; // Найдена разбивка
        double height = 0;      // Высота элемента
        double zBottom = 0;     // Отметка низа
        // Панели
        if (otd_height_down > 0) {
            height = otd_height_down;
            zBottom = otd_zBottom;
            type = GetWallFinishBandType (otdw, Wall_Down);
            if (OtdWall_Delim_One (otdw,
                                   opw,
                                   height,
                                   zBottom,
                                   type,
                                   om_main,
                                   om_up,
                                   om_down,
                                   om_reveals,
                                   om_column,
                                   om_floor,
                                   om_ceil,
                                   om_zone))
                has_delim = true;
        }
        // Основная часть
        if (otd_height_main > 0) {
            type = GetWallFinishBandType (otdw, Wall_Main);
            height = otd_height_main;
            zBottom = otd_zBottom + otd_height_down;
            if (OtdWall_Delim_One (otdw,
                                   opw,
                                   height,
                                   zBottom,
                                   type,
                                   om_main,
                                   om_up,
                                   om_down,
                                   om_reveals,
                                   om_column,
                                   om_floor,
                                   om_ceil,
                                   om_zone))
                has_delim = true;
        }
        // Пространство за потолком
        if (otd_height_up > 0) {
            height = otd_height_up;
            zBottom = otd_zBottom + otd_height_down + otd_height_main;
            type = GetWallFinishBandType (otdw, Wall_Up);
            if (OtdWall_Delim_One (otdw,
                                   opw,
                                   height,
                                   zBottom,
                                   type,
                                   om_main,
                                   om_up,
                                   om_down,
                                   om_reveals,
                                   om_column,
                                   om_floor,
                                   om_ceil,
                                   om_zone))
                has_delim = true;
        }
        // Если высоты заданы не были - подгоним стенку под зону
        if (!has_delim) {
            height = otd_height;
            zBottom = otd_zBottom;
            type = GetFallbackWallFinishType (otdw, type);
            OtdWall_Delim_One (otdw,
                               opw,
                               height,
                               zBottom,
                               type,
                               om_main,
                               om_up,
                               om_down,
                               om_reveals,
                               om_column,
                               om_floor,
                               om_ceil,
                               om_zone);
        }
    }

    // -----------------------------------------------------------------------------
    // Добавляет стену с заданной высотой
    // Удаляет отверстия, не попадающие в диапазон
    // Подгоняет размер отверсий
    // -----------------------------------------------------------------------------
    // Удаляет проёмы вне заданного диапазона и подрезает остальные по высоте с пересчётом lower.
    // Сохраняет только проёмы с высотой и шириной больше min_dim.
    static void TrimOpeningsToWallHeight (
        OtdWall &localCopy, double zBottom, double height, double zDup, double zDown) {
        if (!localCopy.openings.IsEmpty ()) {
            const double wallTop = zDup;
            GS::Array<OtdOpening> newopenings;
            for (OtdOpening &op : localCopy.openings) {
                // Проём начинается выше стенки
                if (op.zBottom > wallTop || is_equal (op.zBottom, wallTop)) {
                    continue;
                }
                double zOpup = op.zBottom + op.height; // Отметка верха проёма
                // Проём заканчивается ниже стенки
                if (zOpup < zBottom || is_equal (zBottom, zOpup)) {
                    continue;
                }
                zDup = fmin (zBottom + height, op.zBottom + op.height);
                zDown = fmax (zBottom, op.zBottom);
                op.zBottom = zDown;
                op.height = fmin (op.height, zDup - zDown);
                op.lower = op.zBottom - zBottom;
                if (op.height > min_dim && op.width > min_dim)
                    newopenings.PushNew (op);
            }
            localCopy.openings = newopenings;
        }
    }

    // Пересекает заданный высотный диапазон со стенкой и обновляет его нижнюю и верхнюю отметки.
    // Возвращает false при отсутствии пересечения или недостаточной высоте.
    static bool CalculateWallHeightIntersection (
        const OtdWall &otdn, double &height, double &zBottom, double &zDup, double &zDown) {
        if (height < min_dim || is_equal (height, 0)) {
            return false;
        }
        // Проверяем - находится ли изначальная конструкция в этом диапазоне?
        if (otdn.zBottom >= zBottom + height) {
            return false; // Конструкция начинается выше необходимого
        }
        if (otdn.zBottom + otdn.height <= zBottom) {
            return false; // Конструкция заканчивается ниже необходимого
        }
        zDup = fmin (zBottom + height, otdn.zBottom + otdn.height); // Новая отметка верха стены
        zDown = fmax (zBottom, otdn.zBottom);                       // Новая отметка низа стены
        height = fmin (otdn.height, zDup - zDown);                  // Новая высота стены
        if (height < min_dim || is_equal (height, 0)) {
            return false;
        }
        zBottom = zDown;         // Переназначаем для расчётов окон
        zDup = zBottom + height; // Переназначаем для расчётов окон
        return true;
    }

    // Добавляет копию стенки, подрезанную по высотному диапазону, с подрезанными проёмами и выбранным материалом.
    // При отсутствии допустимого пересечения возвращает false без добавления.
    bool OtdWall_Delim_One (const OtdWall &otdn,
                            GS::Array<OtdWall> &opw,
                            double height,
                            double zBottom,
                            TypeOtd &type,
                            OtdMaterial &om_main,
                            OtdMaterial &om_up,
                            OtdMaterial &om_down,
                            OtdMaterial &om_reveals,
                            OtdMaterial &om_column,
                            OtdMaterial &om_floor,
                            OtdMaterial &om_ceil,
                            OtdMaterial &om_zone) {
        double zDup = 0;
        double zDown = 0;
        if (!CalculateWallHeightIntersection (otdn, height, zBottom, zDup, zDown)) {
            return false;
        }
        double dlower = otdn.zBottom - zBottom; // Разница отметок стены до и после подрезки
        // Удаляем лишние окна, подстраиваем высоту
        OtdWall localCopy = otdn;
        if (localCopy.surface_geometry) {
            for (const auto &opening : localCopy.openings) {
                const double clippedHeight =
                    std::min (zDup, opening.zBottom + opening.height) - std::max (zBottom, opening.zBottom);
                // Не заполняем пустоту, которую штатный проём не сможет прорезать.
                if (clippedHeight > 0 && (clippedHeight <= min_dim || opening.width <= min_dim))
                    return false;
            }
        }
        TrimOpeningsToWallHeight (localCopy, zBottom, height, zDup, zDown);
        if (localCopy.surface_geometry) {
            const double dz = zBottom - otdn.zBottom;
            localCopy.begC.x += dz * localCopy.rise_x;
            localCopy.begC.y += dz * localCopy.rise_y;
            localCopy.endC.x += dz * localCopy.rise_x;
            localCopy.endC.y += dz * localCopy.rise_y;
        }
        localCopy.zBottom = zBottom;
        localCopy.height = height;
        if (localCopy.surface_geometry && OtdWall_GetArea (localCopy) <= 1.e-6)
            return false;
        if (!is_equal (localCopy.width, 0))
            localCopy.length = height;
        localCopy.type = type;
        SetMaterialByType (localCopy, om_main, om_up, om_down, om_reveals, om_column, om_floor, om_ceil, om_zone);
        opw.Push (std::move (localCopy));
        return true;
    }

    // Выбирает материал по типу отделки, используя резервные материалы при пустом smaterial.
    // Уточняет тип по совпадению свойств записи; окончательный резерв — материал зоны.
    static void SelectWallFinishMaterial (TypeOtd &type,
                                          OtdMaterial &material,
                                          const OtdMaterial &om_main,
                                          const OtdMaterial &om_up,
                                          const OtdMaterial &om_down,
                                          const OtdMaterial &om_reveals,
                                          const OtdMaterial &om_column,
                                          const OtdMaterial &om_floor,
                                          const OtdMaterial &om_ceil,
                                          const OtdMaterial &om_zone) {
        // Проверим существование свойств для записи, при необходимости - поменяем
        // тип отделки
        switch (type) {
        case NoSet:
            material.material = 0;
            material.smaterial = EMPTYSTRING;
    #if defined(TESTING)
            DBprnt ("SetMaterialByType err",
                    "NoSet material.smaterial = "
                    ";");
    #endif
            break;
        case Wall_Main:
            material = om_main;
            break;
        case Wall_Up:
            if (!om_up.smaterial.IsEmpty ()) {
                material = om_up;
            } else {
                material = om_main;
            }
            if (om_up.rawname.IsEqual (om_main.rawname))
                type = Wall_Main;
            break;
        case Wall_Down:
            if (!om_down.smaterial.IsEmpty ()) {
                material = om_down;
            } else {
                material = om_main;
            }
            if (om_down.rawname.IsEqual (om_main.rawname))
                type = Wall_Main;
            break;
        case Reveal_Main:
            if (!om_reveals.smaterial.IsEmpty ()) {
                material = om_reveals;
            } else {
                material = om_main;
            }
            if (material.smaterial.IsEmpty ())
                material = om_main;
            type = Reveal_Main;
            if (om_reveals.rawname.IsEqual (om_main.rawname))
                type = Wall_Main;
            break;
        case Reveal_Up:
            if (!om_reveals.smaterial.IsEmpty ()) {
                material = om_reveals;
            } else {
                material = om_up;
            }
            if (material.smaterial.IsEmpty ())
                material = om_main;
            type = Reveal_Main;
            if (om_reveals.rawname.IsEqual (om_main.rawname))
                type = Wall_Main;
            break;
        case Reveal_Down:
            if (!om_reveals.smaterial.IsEmpty ()) {
                material = om_reveals;
            } else {
                material = om_down;
            }
            if (material.smaterial.IsEmpty ())
                material = om_main;
            type = Reveal_Main;
            if (om_reveals.rawname.IsEqual (om_main.rawname))
                type = Wall_Main;
            break;
        case Column:
            if (!om_column.smaterial.IsEmpty ()) {
                material = om_column;
            } else {
                material = om_main;
            }
            if (om_column.rawname.IsEqual (om_main.rawname))
                type = Wall_Main;
            break;
        case Floor:
            material = om_floor;
            break;
        case Ceil:
            material = om_ceil;
            break;
        default:
            material.material = 0;
            material.smaterial = EMPTYSTRING;
    #if defined(TESTING)
            DBprnt ("SetMaterialByType err",
                    "default material.smaterial = "
                    ";");
    #endif
            break;
        }
        if (material.smaterial.IsEmpty ())
            material = om_zone;
    }

    // Выбирает материал и уточняет тип стенки, учитывает состав основания и сохраняет итоговый материал.
    void SetMaterialByType (OtdWall &otdw,
                            OtdMaterial &om_main,
                            OtdMaterial &om_up,
                            OtdMaterial &om_down,
                            OtdMaterial &om_reveals,
                            OtdMaterial &om_column,
                            OtdMaterial &om_floor,
                            OtdMaterial &om_ceil,
                            OtdMaterial &om_zone) {
        OtdMaterial material;
        SelectWallFinishMaterial (
            otdw.type, material, om_main, om_up, om_down, om_reveals, om_column, om_floor, om_ceil, om_zone);
        SetMaterialFinish (material, otdw.base_composite);
        otdw.material = material;
    }

    // Заменяет покрытие материала индексом и именем последнего слоя состава только при structype == -1.
    void SetMaterialFinish_ByComposite (OtdMaterial &material, GS::Array<ParamValueComposite> &base_composite) {
        // Проверим последний слой - а нужна ли вообще отделка?
        if (base_composite.IsEmpty ())
            return;
        ParamValueComposite &last = base_composite.GetLast ();
        if (last.structype != -1)
            return;
        material.smaterial = last.pos;
        material.material = (short)last.length;
    }

    // Учитывает покрытие последнего слоя и добавляет его обозначение в состав с structype == -1.
    // Метки fin_ignore и IGNORENAME запрещают добавление; при наличии @ извлекает обозначение между разделителями.
    void SetMaterialFinish (OtdMaterial &material, GS::Array<ParamValueComposite> &base_composite) {
        SetMaterialFinish_ByComposite (material, base_composite);
        ParamValueComposite p;
    #ifdef ServerMainVers_2700
        p.inx = ACAPI_CreateAttributeIndex (material.material);
    #else
        p.inx = material.material;
    #endif
        GS::UniString part = material.smaterial;
        if (part.Contains ("fin_ignore")) {
            return;
        }
        if (part.Contains (IGNORENAME)) {
            return;
        }
        part.Trim ();
        part.ReplaceAll ("  ", SPACESTRING);
        if (part.Contains ('@')) {
            part = '@' + part;
            part = part.GetSubstring ('@', '@', 0);
            part.Trim ();
            p.val = part;
        } else {
            p.val = material.smaterial;
        }
        p.structype = -1;
        base_composite.Push (p);
    }

    // Ищет параллельный отрезок с хотя бы одним концом на другом отрезке и длиной больше min_dim.
    // Найденный кандидат записывает в findedge; при отсутствии подходящего возвращает false.
    bool Edge_FindOnEdge (const Sector &edge, const GS::Array<Sector> &edges, Sector &findedge) {
        double dx;
        double dy;
        double dr;
        for (UInt32 i = 0; i < edges.GetSize (); i++) {
            if (edge.ContainsPoint (edges[i].c1) || edge.ContainsPoint (edges[i].c2) ||
                edges[i].ContainsPoint (edge.c1) || edges[i].ContainsPoint (edge.c2)) {
                GS::Optional<UnitVector_2D> edgedir = edge.GetDirection ();
                if (edgedir.HasValue ()) {
                    GS::Optional<UnitVector_2D> roomedgedir = edges[i].GetDirection ();
                    if (roomedgedir.HasValue ()) {
                        if (edgedir.Get ().IsParallelWith (roomedgedir.Get ())) {
                            findedge = edges[i];
                            dx = -findedge.c1.x + findedge.c2.x;
                            dy = -findedge.c1.y + findedge.c2.y;
                            dr = sqrt (dx * dx + dy * dy);
                            if (dr > min_dim)
                                return true;
                        }
                    }
                }
            }
        }
        return false;
    }

    // -----------------------------------------------------------------------------
    // Проверка наличия отрезка в массиве отрезков.
    // В случае нахождение проверяется направление, при необходимости
    // разворачивается
    // -----------------------------------------------------------------------------
    bool Edge_FindEdge (Sector &edge, GS::Array<Sector> &edges) {
        for (UInt32 i = 0; i < edges.GetSize (); i++) {
            if (edges[i].c1.IsNear (edge.c1, min_dim) && edges[i].c2.IsNear (edge.c2, min_dim)) {
                return true;
            } else {
                if (edges[i].c1.IsNear (edge.c2, min_dim) && edges[i].c2.IsNear (edge.c1, min_dim)) {
                    edge = edge.InvertDirection ();
                    return true;
                }
            }
        }
        return false;
    }

    static void Draw_Elements_OneRoom (const Stories &storyLevels,
                                       OtdRoom &otd,
                                       UnicElementByType &subelementByparent,
                                       ClassificationFunc::ClassificationDict &finclass) {
    #if defined(TESTING)
        DBprnt ("Draw_Elements_OneRoom", "start");
    #endif
        GSErrCode err = NoError;
        Int32 n_elem = 0;
        GS::Array<API_Guid> group;
        if (!otd.isValid)
            return;
        API_Guid class_guid = APINULLGuid;
        if (otd.create_wall_elements || otd.create_column_elements || otd.create_reveal_elements) {
            for (OtdWall &otdwall : otd.otdwall) {
                if (!otdwall.isValid)
                    continue;
                if ((otdwall.type == Wall_Main || otdwall.type == Wall_Up || otdwall.type == Wall_Down) &&
                    !otd.create_wall_elements)
                    continue;
                if ((otdwall.type == Column) && !otd.create_column_elements)
                    continue;
                if ((otdwall.type == Reveal_Main || otdwall.type == Reveal_Down || otdwall.type == Reveal_Up) &&
                    !otd.create_reveal_elements)
                    continue;
                OtdWall_Draw (storyLevels, otdwall, subelementByparent);
                if (!otdwall.isValid)
                    continue;
                if (otdwall.otd_guid != APINULLGuid) {
                    Param_AddUnicElementByType (otd.zone_guid,
                                                otdwall.otd_guid,
                                                API_ZoneID,
                                                subelementByparent); // Привязка отделочных стен к
                                                                     // зоне
                    for (OtdOpening &op : otdwall.openings) {
                        Param_AddUnicElementByType (otd.zone_guid,
                                                    op.otd_guid,
                                                    API_ZoneID,
                                                    subelementByparent); // Привязка отделочных
                                                                         // проёмов к зоне
                    }
                    // Классификация созданных стен
                    Class_SetClass (otdwall, finclass);
                    for (OtdOpening &op : otdwall.openings) {
                        if (op.otd_guid == APINULLGuid)
                            continue;
                        n_elem += 1;
                        Class_SetClass (op, finclass);
                    }
                    group.Push (otdwall.otd_guid);
                } else {
                    msg_rep ("Draw_Elements_OneRoom", "otdwall.otd_guid == APINULLGuid", NoError, APINULLGuid);
                }
            }
        } else {
            otd.otdwall.Clear ();
        }
        if (otd.create_ceil_elements || otd.create_floor_elements) {
            for (OtdSlab &otdslab : otd.otdslab) {
                if (!otdslab.isValid)
                    continue;
                if (otdslab.type == Ceil && !otd.create_ceil_elements)
                    continue;
                if (otdslab.type == Floor && !otd.create_floor_elements)
                    continue;
                Floor_Draw (storyLevels, otdslab, subelementByparent);
                if (otdslab.otd_guid != APINULLGuid) {
                    Param_AddUnicElementByType (otd.zone_guid,
                                                otdslab.otd_guid,
                                                API_ZoneID,
                                                subelementByparent); // Привязка перекрытий к зоне
                    Class_SetClass (otdslab, finclass);
                    group.Push (otdslab.otd_guid);
                } else {
                    msg_rep ("Draw_Elements_OneRoom", "otdslab.otd_guid == APINULLGuid", NoError, APINULLGuid);
                }
            }
        } else {
            otd.otdslab.Clear ();
        }
        n_elem += group.GetSize ();
        if (group.GetSize () > 1) {
    #ifdef ServerMainVers_2700
            err = ACAPI_Grouping_Tool (group, APITool_Group, nullptr);
    #else
            API_Guid groupGuid = APINULLGuid;
            err = ACAPI_ElementGroup_Create (group, &groupGuid);
        #ifdef ServerMainVers_2300
            if (err != NoError)
                err = ACAPI_Element_Tool (group, APITool_Group, nullptr);
        #endif
    #endif
            if (err != NoError)
                msg_rep ("Draw_Elements_OneRoom", "ACAPI_Grouping_CreateGroup", err, APINULLGuid);
        }
        msg_rep (
            "RoomBook", GS::UniString::Printf ("Create or update %d finishing elements", n_elem), err, APINULLGuid);
    }

    void Draw_Elements (GS::HashTable<API_Guid, UnicGuidByBase> &exsistot_byzone,
                        GS::Array<API_Guid> &deletelist,
                        OtdRooms &roomsinfo,
                        const Stories &storyLevels,
                        ClassificationFunc::ClassificationDict &finclass,
                        UnicGuid &reserv_elements,
                        ParamDictElement &paramToWrite) {
        RemoveUnusedFinishingElements (exsistot_byzone, deletelist);
        UnicElementByType subelementByparent; // Словарь с созданными родительскими
                                              // и дочерними элементами
        // Отрисовка элементов отделки
        GS::UniString funcname = "Draw finishing element(s)";
        GS::UniString UndoString = RSGetIndString (ID_ADDON_STRINGS + isEng (), RoombookId, ACAPI_GetOwnResModule ());
        SuspendGroupsGuard suspGuard; // отключаем режим группировки на время записи.
        ACAPI_CallUndoableCommand (UndoString, [&] () -> GSErrCode {
            // Резервируем и разблокируем
            PrepareElementsResult prepareResult = PrepareElementsResult::NothingToDo;
            if (!PrepareElementsForUpdate (reserv_elements, prepareResult))
                return NoError;
            // Удаляем устаревшие элементы отделки
            if (!deletelist.IsEmpty ()) {
                GSErrCode err = ACAPI_Element_Delete (deletelist);
                msg_rep ("RoomBook",
                         GS::UniString::Printf ("Removed %d obsolete finishing elements", deletelist.GetSize ()),
                         err,
                         APINULLGuid);
            } else {
                msg_rep ("RoomBook", "Obsolete finishing elements not found", NoError, APINULLGuid);
            }
            for (OtdRooms::PairIterator cIt = roomsinfo.EnumeratePairs (); cIt != NULL; ++cIt) {
    #ifdef ServerMainVers_2800
                OtdRoom &otd = cIt->value;
    #else
                OtdRoom &otd = *cIt->value;
    #endif
                if (AdvanceProcessPhase (funcname, nPhase))
                    return NoError;
                Draw_Elements_OneRoom (storyLevels, otd, subelementByparent, finclass);
            }
            return NoError;
        });
        // Привязка отделочных элементов к базовым
        SetSyncOtdWall (subelementByparent, paramToWrite);
    }

    // -----------------------------------------------------------------------------
    // Построение отделочных стен (общая)
    // -----------------------------------------------------------------------------
    void OtdWall_Draw (const Stories &storyLevels, OtdWall &edges, UnicElementByType &subelementByparent) {
        if (edges.height < min_dim)
            return;
        double dx = -edges.endC.x + edges.begC.x;
        double dy = -edges.endC.y + edges.begC.y;
        double dr = sqrt (dx * dx + dy * dy);
        if (dr < min_dim)
            return;
        if (edges.surface_geometry) {
            OtdWall_Draw_Wall (edges.favorite.name, storyLevels, edges, subelementByparent);
            if (edges.isValid && edges.otd_guid != APINULLGuid)
                Param_AddUnicElementByType (edges.base_guid, edges.otd_guid, API_WallID, subelementByparent);
            return;
        }
        if (edges.favorite.type == API_ObjectID) {
            // горизонтальную часть откоса строим в этом случае аксессуаром
            // пола/потолка
            if (edges.draw_type == API_BeamID) {
                OtdBeam_Draw_Object (edges.favorite.name, storyLevels, edges);
            } else {
                OtdWall_Draw_Object (edges.favorite.name, storyLevels, edges);
            }
        }
        if (edges.favorite.type == API_WallID) {
            OtdWall_Draw_Wall (edges.favorite.name, storyLevels, edges, subelementByparent);
        }
        if (edges.favorite.type == API_BeamID) {
            OtdBeam_Draw_Beam (edges.favorite.name, storyLevels, edges);
        }
        Param_AddUnicElementByType (edges.base_guid,
                                    edges.otd_guid,
                                    API_WallID,
                                    subelementByparent); // Привязка отделочной стены к базовой
    }

    // Создаёт балку с координатами, размерами первого сегмента и материалом из edges; задаёт этаж и отметку.
    // После успешного создания записывает GUID в edges.otd_guid, при ошибке создания сообщает о ней.
    void OtdBeam_Draw_Beam (const GS::UniString &favorite_name, const Stories &storyLevels, OtdWall &edges) {
        GSErrCode err = NoError;
        API_Element beamelement = {};
        API_ElementMemo beammemo = {};
        const GS::OnExit memoGuard ([&beammemo] () { ACAPI_DisposeElemMemoHdls (&beammemo); });
        if (!OtdBeam_GetDefult_Beam (favorite_name, beamelement, beammemo)) {
            return;
        }
        beamelement.beam.begC = edges.begC;
        beamelement.beam.endC = edges.endC;
        if (beammemo.beamSegments != nullptr) {
            beammemo.beamSegments[0].assemblySegmentData.nominalWidth = edges.width;
            beammemo.beamSegments[0].assemblySegmentData.nominalHeight = edges.height;

    #ifdef ServerMainVers_2700
            // Материал горизонтального откоса: для AC_27+ структура API_BeamSegmentType содержит
            // bottomMaterial, leftMaterial, rightMaterial, topMaterial, endsMaterial (AC_29+ extrusionMaterial)
            // AC27: API_OverriddenAttribute заменён на APIOptional<API_AttributeIndex>
            // (как сделано ниже для стены: hasValue + value).
            const API_AttributeIndex beamMaterial = ACAPI_CreateAttributeIndex (edges.material.material);
            beammemo.beamSegments[0].bottomMaterial.hasValue = true;
            beammemo.beamSegments[0].bottomMaterial.value = beamMaterial;

            beammemo.beamSegments[0].leftMaterial.hasValue = true;
            beammemo.beamSegments[0].leftMaterial.value = beamMaterial;

            beammemo.beamSegments[0].rightMaterial.hasValue = true;
            beammemo.beamSegments[0].rightMaterial.value = beamMaterial;

            beammemo.beamSegments[0].topMaterial.hasValue = true;
            beammemo.beamSegments[0].topMaterial.value = beamMaterial;

            beammemo.beamSegments[0].endsMaterial.hasValue = true;
            beammemo.beamSegments[0].endsMaterial.value = beamMaterial;
    #else
            beammemo.beamSegments[0].bottomMaterial.overridden = true;
            beammemo.beamSegments[0].bottomMaterial.attributeIndex = edges.material.material;

            beammemo.beamSegments[0].leftMaterial.overridden = true;
            beammemo.beamSegments[0].leftMaterial.attributeIndex = edges.material.material;

            beammemo.beamSegments[0].rightMaterial.overridden = true;
            beammemo.beamSegments[0].rightMaterial.attributeIndex = edges.material.material;
    #endif
        }
        beamelement.header.floorInd = edges.floorInd;
        beamelement.beam.level = GetOffsetFromStory (edges.zBottom, edges.floorInd, storyLevels);
        beamelement.beam.isFlipped = false;
        err = ACAPI_Element_Create (&beamelement, &beammemo);
        if (err != NoError) {
            msg_rep ("OtdBeam_Draw_Beam err", "ACAPI_Element_Create", err, APINULLGuid);
            return;
        }
        edges.otd_guid = beamelement.header.guid;
    }

    // Обнуляет элемент и memo; использует избранное, если оно содержит балку, иначе получает настройки балки по
    // умолчанию. При ошибке получения настроек освобождает memo и возвращает false.
    bool OtdBeam_GetDefult_Beam (const GS::UniString &favorite_name,
                                 API_Element &beamelement,
                                 API_ElementMemo &beammemo) {
        GSErrCode err = NoError;
        BNZeroMemory (&beamelement, sizeof (API_Element));
        BNZeroMemory (&beammemo, sizeof (API_ElementMemo));
        if (Favorite_GetByName (favorite_name, beamelement, beammemo)) {
            if (GetElemTypeID (beamelement) == API_BeamID)
                return true;
        }
        SetElemTypeID (beamelement, API_BeamID);
        err = ACAPI_Element_GetDefaults (&beamelement, &beammemo);
        if (err != NoError) {
            ACAPI_DisposeElemMemoHdls (&beammemo);
            return false;
        }
        return true;
    }

    // Ничего не делает: тело создания объекта закомментировано, edges.otd_guid не изменяется.
    void OtdBeam_Draw_Object (const GS::UniString &favorite_name, const Stories &storyLevels, OtdWall &edges) {
        // API_Element beamelement;
        // API_ElementMemo beammemo;
        // GSErrCode err = NoError;
        // if (!OtdBeam_GetDefult_Object (favorite_name, beamelement, beammemo)) {
        //     return;
        // }
        // edges.otd_guid = beamelement.header.guid;
        // ACAPI_DisposeElemMemoHdls (&beammemo);
    }

    // Обнуляет элемент и memo; возвращает объект из избранного либо получает настройки объекта по умолчанию.
    // Только для настроек по умолчанию сбрасывает угол и автовидимость по этажам, включает параметры пера, линий и
    // материалов объекта.
    bool OtdBeam_GetDefult_Object (const GS::UniString &favorite_name,
                                   API_Element &beamelement,
                                   API_ElementMemo &beammemo) {
        GSErrCode err = NoError;
        BNZeroMemory (&beammemo, sizeof (API_ElementMemo));
        BNZeroMemory (&beamelement, sizeof (API_Element));
        if (Favorite_GetByName (favorite_name, beamelement, beammemo)) {
            if (GetElemTypeID (beamelement) == API_ObjectID)
                return true;
        }
        SetElemTypeID (beamelement, API_ObjectID);
        err = ACAPI_Element_GetDefaults (&beamelement, &beammemo);
        if (err != NoError) {
            ACAPI_DisposeElemMemoHdls (&beammemo);
            return false;
        }
        beamelement.object.angle = 0;
        beamelement.object.isAutoOnStoryVisibility = false;
        beamelement.object.useObjPens = true;
        beamelement.object.useObjLtypes = true;
        beamelement.object.useObjMaterials = true;
        return true;
    }

    // -----------------------------------------------------------------------------
    // Построение отделочных стен аксессуаром
    // -----------------------------------------------------------------------------
    void OtdWall_Draw_Object (const GS::UniString &favorite_name, const Stories &storyLevels, OtdWall &edges) {
        GSErrCode err = NoError;
        API_Element wallobjelement = {};
        API_ElementMemo wallobjmemo = {};
        const GS::OnExit memoGuard ([&wallobjmemo] () { ACAPI_DisposeElemMemoHdls (&wallobjmemo); });
        if (!OtdWall_GetDefult_Object (favorite_name, wallobjelement, wallobjmemo)) {
            return;
        }
        Point2D wbegC = {edges.begC.x, edges.begC.y};
        Point2D wendC = {edges.endC.x, edges.endC.y};
        Sector walledge = {wbegC, wendC};
        GS::Optional<UnitVector_2D> walldir = walledge.GetDirection ();
        if (!walldir.HasValue ()) {
    #if defined(TESTING)
            DBprnt ("OtdWall_Create_FromWall err", "!walldir.HasValue");
    #endif
            return;
        }
        double ac_wall_length = walledge.GetLength ();
        GDLHelpers::ParamDict accsessoryparams;
        GDLHelpers::Param p;
        p.num = 1;
        accsessoryparams.Add ("{@gdl:ac_refside}", p);
        p.num = edges.material.material;
        accsessoryparams.Add ("{@gdl:gs_bw_mat}", p);
        p.num = edges.material.material;
        accsessoryparams.Add ("{@gdl:matp1}", p);

        p.num = edges.height;
        accsessoryparams.Add ("{@gdl:ac_wall_height}", p);
        p.num = ac_wall_length;
        accsessoryparams.Add ("{@gdl:ac_wall_length}", p);
        p.num = 0;
        accsessoryparams.Add ("{@gdl:ac_wall_radius}", p);

        p.dim1 = 1;
        p.dim2 = 2;
        p.arr_num.Push (0);
        p.arr_num.Push (0);
        accsessoryparams.Add ("{@gdl:ac_side_poly}", p);
        p.arr_num.Clear ();

        p.dim1 = 2;
        p.dim2 = 2;
        p.arr_num.Push (0);
        p.arr_num.Push (edges.height);
        p.arr_num.Push (ac_wall_length);
        p.arr_num.Push (edges.height);
        accsessoryparams.Add ("{@gdl:ac_top_poly}", p);
        p.arr_num.Clear ();

        p.dim1 = 2;
        p.dim2 = 2;
        p.arr_num.Push (0);
        p.arr_num.Push (0);
        p.arr_num.Push (ac_wall_length);
        p.arr_num.Push (0);
        accsessoryparams.Add ("{@gdl:ac_bot_poly}", p);
        p.arr_num.Clear ();

        p.dim1 = 1;
        p.dim2 = 2;
        p.arr_num.Push (edges.ang_begC);
        p.arr_num.Push (edges.ang_endC);
        accsessoryparams.Add ("{@gdl:ac_angles}", p);
        p.arr_num.Clear ();

        if (edges.openings.IsEmpty ()) {
            p.dim1 = 1;
            p.dim2 = 2;
            p.arr_num.Push (0);
            p.arr_num.Push (0);
        } else {
            Int32 dim2 = 0;
            UInt32 addedOpenings = 0;
            for (OtdOpening &op : edges.openings) {
                if (op.width < min_dim || op.height < min_dim)
                    continue;
                ++addedOpenings;
                double objLoc = ac_wall_length - op.objLoc;
                double halfwidth = op.width / 2;

                p.arr_num.Push (0.106);
                p.arr_num.Push (objLoc - halfwidth);
                p.arr_num.Push (op.lower);

                p.arr_num.Push (objLoc - halfwidth);
                p.arr_num.Push (op.lower + op.height);

                p.arr_num.Push (objLoc + halfwidth);
                p.arr_num.Push (op.lower + op.height);

                p.arr_num.Push (objLoc + halfwidth);
                p.arr_num.Push (op.lower);
            }
            p.dim1 = addedOpenings;
            p.dim2 = 9;
        }
        accsessoryparams.Add ("{@gdl:ac_wd_poly}", p);
        p.arr_num.Clear ();
        GDLHelpers::ParamToMemo (wallobjmemo, accsessoryparams);
        Vector2D ref = {-1, 0};
        Vector2D wallvect = walldir.Get ().ToVector2D ();
        double ang = wallvect.CalcAngleToReference (ref);
        wallobjelement.object.angle = ang;
        wallobjelement.object.pos = edges.endC;
        wallobjelement.header.floorInd = edges.floorInd;
    #ifdef ServerMainVers_2700
        API_AttributeIndex ematerial = ACAPI_CreateAttributeIndex (edges.material.material);
        wallobjelement.object.mat = ematerial;
    #else
        wallobjelement.object.mat = edges.material.material;
    #endif
        wallobjelement.object.level = GetOffsetFromStory (edges.zBottom, edges.floorInd, storyLevels);
        err = ACAPI_Element_Create (&wallobjelement, &wallobjmemo);
        if (err != NoError) {
            msg_rep ("OtdWall_Draw_Object err", "ACAPI_Element_Create", err, APINULLGuid);
            return;
        }
        edges.otd_guid = wallobjelement.header.guid;
    }

    // -----------------------------------------------------------------------------
    // Получение из избранного аксессуара стены для простроения
    // -----------------------------------------------------------------------------
    // Обнуляет элемент и memo; возвращает объект из избранного либо получает настройки объекта по умолчанию.
    // Только в ветке настроек по умолчанию задаёт угол, видимость и использование перьев, линий и материалов объекта.
    bool OtdWall_GetDefult_Object (const GS::UniString &favorite_name,
                                   API_Element &wallobjelement,
                                   API_ElementMemo &wallobjmemo) {
        GSErrCode err = NoError;
        BNZeroMemory (&wallobjmemo, sizeof (API_ElementMemo));
        BNZeroMemory (&wallobjelement, sizeof (API_Element));
        if (Favorite_GetByName (favorite_name, wallobjelement, wallobjmemo)) {
            if (GetElemTypeID (wallobjelement) == API_ObjectID)
                return true;
        }
        SetElemTypeID (wallobjelement, API_ObjectID);
        err = ACAPI_Element_GetDefaults (&wallobjelement, &wallobjmemo);
        if (err != NoError) {
            ACAPI_DisposeElemMemoHdls (&wallobjmemo);
            return false;
        }
        wallobjelement.object.angle = 0;
        wallobjelement.object.isAutoOnStoryVisibility = false;
        wallobjelement.object.useObjPens = true;
        wallobjelement.object.useObjLtypes = true;
        wallobjelement.object.useObjMaterials = true;
        return true;
    }

    // -----------------------------------------------------------------------------
    // Построение отделочных стен стандартной стеной
    // -----------------------------------------------------------------------------
    void OtdWall_Draw_Wall (const GS::UniString &favorite_name,
                            const Stories &storyLevels,
                            OtdWall &edges,
                            UnicElementByType &subelementByparent) {
        GSErrCode err = NoError;
        API_Element wallelement = {};
        if (!OtdWall_GetDefult_Wall (favorite_name, wallelement)) {
            return;
        }
        if (edges.surface_geometry) {
            if (wallelement.wall.modelElemStructureType == API_ProfileStructure) {
                msg_rep (
                    "Surface finish", "Profile wall favorite is not representable", APIERR_BADPARS, edges.base_guid);
                return;
            }
            wallelement.wall.type = APIWtyp_Normal;
            wallelement.wall.referenceLineLocation = APIWallRefLine_Inside;
            wallelement.wall.offset = 0;
            wallelement.wall.offsetFromOutside = wallelement.wall.thickness;
        }
        const bool is_upper_reveal =
            !edges.surface_geometry && edges.draw_type == API_BeamID && edges.favorite.type == API_WallID;
        // Откосы
        if (is_upper_reveal) {
            edges.height = wallelement.wall.thickness;
            edges.zBottom -= edges.height;
            if (!is_equal (edges.width, 0) && edges.width > 0)
                wallelement.wall.thickness = edges.width;
            wallelement.wall.offset = wallelement.wall.thickness;
            wallelement.wall.offsetFromOutside = 0;
        }
        // Верхний откос уже ориентирован при подготовке; повторный разворот меняет сторону глубины.
        wallelement.wall.begC = edges.begC;
        wallelement.wall.endC = edges.endC;
        wallelement.wall.height = edges.height;
        if (edges.slant_angle > 0 && !is_upper_reveal) {
            wallelement.wall.profileType = is_equal (edges.slant_angle, 90 * DEGRAD) ? APISect_Normal : APISect_Slanted;
            wallelement.wall.slantAlpha = edges.slant_angle;
            wallelement.wall.relativeTopStory = 0;
        }
        wallelement.header.floorInd = edges.floorInd;
        wallelement.wall.bottomOffset = GetOffsetFromStory (edges.zBottom, edges.floorInd, storyLevels);
    #ifdef ServerMainVers_2700
        API_AttributeIndex ematerial = ACAPI_CreateAttributeIndex (edges.material.material);
        wallelement.wall.refMat.hasValue = true;
        wallelement.wall.oppMat.hasValue = true;
        wallelement.wall.sidMat.hasValue = true;
        wallelement.wall.refMat.value = ematerial;
        wallelement.wall.oppMat.value = ematerial;
        wallelement.wall.sidMat.value = ematerial;
    #else
        wallelement.wall.refMat.overridden = true;
        wallelement.wall.oppMat.overridden = true;
        wallelement.wall.sidMat.overridden = true;
        wallelement.wall.refMat.attributeIndex = edges.material.material;
        wallelement.wall.oppMat.attributeIndex = edges.material.material;
        wallelement.wall.sidMat.attributeIndex = edges.material.material;
    #endif
        wallelement.wall.zoneRel = APIZRel_None;
        wallelement.wall.flipped = false;
        err = ACAPI_Element_Create (&wallelement, nullptr);
        if (err != NoError) {
            msg_rep ("OtdWall_Draw_Wall err", "ACAPI_Element_Create", err, APINULLGuid);
            return;
        }
        edges.otd_guid = wallelement.header.guid;
        for (OtdOpening &op : edges.openings) {
            Opening_Draw (wallelement, op, subelementByparent, edges.zBottom);
            if (edges.surface_geometry && op.otd_guid == APINULLGuid) {
                // Непрорезанная оболочка даёт лишнюю отделку; удаляем только новый результат.
                const GS::Array<API_Guid> incomplete = {wallelement.header.guid};
                const GSErrCode deleteErr = ACAPI_Element_Delete (incomplete);
                msg_rep ("Surface finish", "Opening failed; removing incomplete wall", deleteErr, edges.base_guid);
                edges.isValid = false;
                if (deleteErr == NoError) {
                    edges.otd_guid = APINULLGuid;
                    for (auto &opening : edges.openings)
                        opening.otd_guid = APINULLGuid;
                }
                return;
            }
        }
    }

    // -----------------------------------------------------------------------------
    // Получение из избранного стены для простроения
    // -----------------------------------------------------------------------------
    // Возвращает стену из избранного, если её тип подходит; иначе получает настройки стены по умолчанию.
    // В ветке настроек по умолчанию задаёт базовую структуру, толщину otd_thickness, внутреннюю линию привязки и
    // zoneRel.
    bool OtdWall_GetDefult_Wall (const GS::UniString &favorite_name, API_Element &wallelement) {
        GSErrCode err = NoError;
        BNZeroMemory (&wallelement, sizeof (API_Element));
        if (Favorite_GetByName (favorite_name, wallelement)) {
            if (GetElemTypeID (wallelement) == API_WallID)
                return true;
        }
        SetElemTypeID (wallelement, API_WallID);
        err = ACAPI_Element_GetDefaults (&wallelement, nullptr);
        if (err != NoError) {
            return false;
        }
        wallelement.wall.zoneRel = APIZRel_None;
        wallelement.wall.referenceLineLocation = APIWallRefLine_Inside;
        wallelement.wall.flipped = false;
    #ifdef ServerMainVers_2600
    #else
        wallelement.wall.refMat.overridden = true;
        wallelement.wall.oppMat.overridden = true;
        wallelement.wall.sidMat.overridden = true;
    #endif
        wallelement.wall.modelElemStructureType = API_BasicStructure;
        wallelement.wall.thickness = otd_thickness;
        return true;
    }

    // -----------------------------------------------------------------------------
    // Построение проёмов в отделочных стенах
    // -----------------------------------------------------------------------------
    void Opening_Draw (API_Element &wallelement,
                       OtdOpening &op,
                       UnicElementByType &subelementByparent,
                       double zBottom) {
        GSErrCode err = NoError;
        API_Element windowelement = {};
        API_ElementMemo windowmemo = {};
        if (wallelement.wall.type == APIWtyp_Poly)
            return;
        if (!Opening_GetDefult ("smstf window", windowelement, windowmemo)) {
            return;
        }
        windowelement.window.owner = wallelement.header.guid;
        const GS::OnExit memoGuard ([&windowmemo] () { ACAPI_DisposeElemMemoHdls (&windowmemo); });
        if (op.surface_geometry) {
            // Вырез следует наклону стены, независимо от точки фиксации избранного.
            windowelement.window.directionType = API_WDAssociativeToWall;
            windowelement.window.fixPoint = APIHoleAnchor_CenterFix;
        }
        if (op.has_reveal) {
            double th = wallelement.wall.thickness;
            double zUp_op = op.zBottom + op.height;
            double zUp_wall = zBottom + wallelement.wall.height;
            // Уменьшаем проём на толщину откоса по высоте
            if (!is_equal (zUp_op, zUp_wall)) {
                op.height -= th;
            }
            double lwall = sqrt ((wallelement.wall.begC.x - wallelement.wall.endC.x) *
                                     (wallelement.wall.begC.x - wallelement.wall.endC.x) +
                                 (wallelement.wall.begC.y - wallelement.wall.endC.y) *
                                     (wallelement.wall.begC.y - wallelement.wall.endC.y));
            if (!is_equal (op.objLoc + op.width / 2, lwall)) {
                op.width -= th;
                op.objLoc -= th;
            }
            if (!is_equal (op.objLoc - op.width / 2, 0)) {
                op.width -= th;
                op.objLoc += th;
            }
        }
        // objLoc присваиваем ПОСЛЕ коррекции откосов, чтобы центр проёма совпадал со скорректированными размерами
        windowelement.window.objLoc = op.objLoc;
        if (op.width < min_dim || op.height < min_dim) {
            return;
        }
        windowelement.window.openingBase.width = op.width;
        windowelement.window.openingBase.height = op.height;
        windowelement.window.lower = op.lower;
        err = ACAPI_Element_Create (&windowelement, &windowmemo);
        if (err != NoError) {
            msg_rep ("Opening_Draw err", "ACAPI_Element_Create", err, APINULLGuid);
            return;
        }
        op.otd_guid = windowelement.header.guid;
        Param_AddUnicElementByType (op.base_guid,
                                    op.otd_guid,
                                    API_WindowID,
                                    subelementByparent); // Привязка отделочного проёма к базовому
    }

    // -----------------------------------------------------------------------------
    // Получение настроек проёмов в отделочных стенах
    // -----------------------------------------------------------------------------
    bool Opening_GetDefult (const GS::UniString &favorite_name,
                            API_Element &windowelement,
                            API_ElementMemo &windowmemo) {
        GSErrCode err = NoError;
        BNZeroMemory (&windowmemo, sizeof (API_ElementMemo));
        BNZeroMemory (&windowelement, sizeof (API_Element));
        if (Favorite_GetByName (favorite_name, windowelement, windowmemo)) {
            if (GetElemTypeID (windowelement) == API_WindowID)
                return true;
        }
        SetElemTypeID (windowelement, API_WindowID);
        err = ACAPI_Element_GetDefaults (&windowelement, &windowmemo);
        if (err != NoError) {
            ACAPI_DisposeElemMemoHdls (&windowmemo);
            return false;
        }
        return true;
    }

    // -----------------------------------------------------------------------------
    // Построение потолка/пола (общая)
    // -----------------------------------------------------------------------------
    void Floor_Draw (const Stories &storyLevels, OtdSlab &otdslab, UnicElementByType &subelementByparent) {
        if (otdslab.surface_geometry) {
            if (otdslab.favorite.type == API_ObjectID) {
                Floor_Draw_Object (otdslab.favorite.name, storyLevels, otdslab);
            } else {
                Floor_Draw_Slab (otdslab.favorite.name, storyLevels, otdslab);
            }
            if (otdslab.otd_guid != APINULLGuid)
                Param_AddUnicElementByType (otdslab.base_guid, otdslab.otd_guid, API_SlabID, subelementByparent);
            return;
        }
        // Провери тип существующего элемента
        API_ElemTypeID type = otdslab.favorite.type;
        API_Guid otd_guid = otdslab.otd_guid;
        bool is_new = (otd_guid == APINULLGuid);
        if (!is_new) {
            type = GetElemTypeID (otd_guid);
        }
        if (type == API_ObjectID) {
            Floor_Draw_Object (otdslab.favorite.name, storyLevels, otdslab);
        } else {
            Floor_Draw_Slab (otdslab.favorite.name, storyLevels, otdslab);
        }
        // Если обновить объект не получилось - удаляем старый и создаём новый
        if (!is_new && otdslab.otd_guid == APINULLGuid) {
            msg_rep ("Floor_Draw err", "Can't update element, create new", NoError, APINULLGuid);
            GS::Array<API_Guid> deletelist = {otd_guid};
            ACAPI_Element_Delete (deletelist);
            if (otdslab.favorite.type == API_ObjectID) {
                Floor_Draw_Object (otdslab.favorite.name, storyLevels, otdslab);
            } else {
                Floor_Draw_Slab (otdslab.favorite.name, storyLevels, otdslab);
            }
        }
        otdslab.poly.Clear ();
        Param_AddUnicElementByType (otdslab.base_guid,
                                    otdslab.otd_guid,
                                    API_SlabID,
                                    subelementByparent); // Привязка отделочного перекрытия к базовому
    }

    // Создаёт или изменяет перекрытие по полигону, отметке и материалу otdslab; пропускает площадь менее 0.0001.
    // При изменении задаёт материалы и перпендикулярную подрезку рёбер; GUID записывает только при успехе и не обнуляет
    // при ошибках.
    void Floor_Draw_Slab (const GS::UniString &favorite_name, const Stories &storyLevels, OtdSlab &otdslab) {
        GSErrCode err = NoError;
        if (otdslab.poly.CalcArea () < 0.0001)
            return;
        API_Element slabelement = {};
        bool is_new = (otdslab.otd_guid == APINULLGuid); // Элемент уже существует, нужно только обновить
        API_ElementMemo memo = {};
        const GS::OnExit memoGuard ([&memo] () { ACAPI_DisposeElemMemoHdls (&memo); });
        if (is_new) {
            if (!Floor_GetDefult_Slab (favorite_name, slabelement))
                return;
        } else {
            slabelement.header.guid = otdslab.otd_guid;
            err = ACAPI_Element_Get (&slabelement);
            if (err != NoError) {
                msg_rep ("Floor_Draw_Object", "ACAPI_Element_Get", err, otdslab.otd_guid);
                return;
            }
            if (GetElemTypeID (slabelement) != API_SlabID)
                return;
            err = ACAPI_Element_GetMemo (slabelement.header.guid, &memo, APIMemoMask_Polygon);
            if (err != NoError) {
                msg_rep ("Floor_Draw_Object", "ACAPI_Element_GetMemo", err, otdslab.otd_guid);
                return;
            }
        }
        slabelement.header.floorInd = otdslab.floorInd;
        slabelement.slab.level = GetOffsetFromStory (otdslab.zBottom, otdslab.floorInd, storyLevels);
        if (otdslab.surface_geometry) {
            slabelement.slab.referencePlaneLocation = APISlabRefPlane_Top;
            slabelement.slab.offsetFromTop = 0;
            if (favorite_name.IsEmpty ()) {
                slabelement.slab.modelElemStructureType = API_BasicStructure;
                slabelement.slab.thickness = otd_thickness;
            }
            // Потолок уходит вниз от контакта, пол — вверх; level задаёт верх перекрытия.
            if (otdslab.type == Floor)
                slabelement.slab.level += slabelement.slab.thickness;
        }
    #ifdef ServerMainVers_2700
        API_AttributeIndex ematerial = ACAPI_CreateAttributeIndex (otdslab.material.material);
        slabelement.slab.topMat.value = ematerial;
        slabelement.slab.sideMat.value = ematerial;
        slabelement.slab.botMat.value = ematerial;
        slabelement.slab.sideMat.hasValue = true;
    #else
        slabelement.slab.topMat.attributeIndex = otdslab.material.material;
        slabelement.slab.sideMat.attributeIndex = otdslab.material.material;
        slabelement.slab.botMat.attributeIndex = otdslab.material.material;
        slabelement.slab.sideMat.overridden = true;
    #endif
        if (otdslab.type == Ceil) {
    #ifdef ServerMainVers_2700
            slabelement.slab.botMat.hasValue = true;
    #else
            slabelement.slab.botMat.overridden = true;
    #endif
        } else {
    #ifdef ServerMainVers_2700
            slabelement.slab.topMat.hasValue = true;
    #else
            slabelement.slab.topMat.overridden = true;
    #endif
        }
        err = ConvertPolygon2DToAPIPolygon (otdslab.poly, slabelement.slab.poly, memo);
        if (err != NoError) {
            return;
        }
        if (is_new) {
            err = ACAPI_Element_Create (&slabelement, &memo);
        } else {
            // Setting side materials and edge angles
            BMhKill ((GSHandle *)&memo.edgeTrims);
            BMhKill ((GSHandle *)&memo.edgeIDs);
            BMpFree (reinterpret_cast<GSPtr> (memo.sideMaterials));
            API_EdgeTrimID edgeType = APIEdgeTrim_Perpendicular;
            memo.edgeTrims = (API_EdgeTrim **)BMAllocateHandle (
                (slabelement.slab.poly.nCoords + 1) * sizeof (API_EdgeTrim), ALLOCATE_CLEAR, 0);
            memo.sideMaterials = (API_OverriddenAttribute *)BMAllocatePtr (
                (slabelement.slab.poly.nCoords + 1) * sizeof (API_OverriddenAttribute), ALLOCATE_CLEAR, 0);
            for (Int32 k = 1; k <= slabelement.slab.poly.nCoords; ++k) {
                memo.sideMaterials[k] = slabelement.slab.sideMat;
                (*(memo.edgeTrims))[k].sideType = edgeType;
                (*(memo.edgeTrims))[k].sideAngle = PI / 2;
            }
            API_Element mask = {};
            ACAPI_ELEMENT_MASK_SET (mask, API_SlabType, botMat);
            ACAPI_ELEMENT_MASK_SET (mask, API_SlabType, topMat);
            ACAPI_ELEMENT_MASK_SET (mask, API_SlabType, sideMat);
            ACAPI_ELEMENT_MASK_SET (mask, API_SlabType, poly.nSubPolys);
            ACAPI_ELEMENT_MASK_SET (mask, API_SlabType, poly.nCoords);
            ACAPI_ELEMENT_MASK_SET (mask, API_SlabType, poly.nArcs);
            ACAPI_ELEMENT_MASK_SET (mask, API_SlabType, level);
            if (otdslab.surface_geometry) {
                ACAPI_ELEMENT_MASK_SET (mask, API_Elem_Head, layer);
                ACAPI_ELEMENT_MASK_SET (mask, API_SlabType, referencePlaneLocation);
                ACAPI_ELEMENT_MASK_SET (mask, API_SlabType, offsetFromTop);
                ACAPI_ELEMENT_MASK_SET (mask, API_SlabType, thickness);
                ACAPI_ELEMENT_MASK_SET (mask, API_SlabType, modelElemStructureType);
            }
            ACAPI_ELEMENT_MASK_SET (mask, API_Elem_Head, floorInd);
            err = ACAPI_Element_Change (&slabelement,
                                        &mask,
                                        &memo,
                                        APIMemoMask_Polygon | APIMemoMask_SideMaterials | APIMemoMask_EdgeTrims,
                                        true);
        }
        if (err != NoError) {
            msg_rep ("Floor_Draw_Slab", favorite_name, err, APINULLGuid);
            return;
        } else {
            otdslab.otd_guid = slabelement.header.guid;
        }
        return;
    }

    // Создаёт или обновляет объект пола/потолка, передавая контуры и параметры отделки в GDL.
    // При отказе чтения существующего элемента, memo или записи обнуляет otd_guid; при успехе сохраняет GUID.
    void Floor_Draw_Object (const GS::UniString &favorite_name, const Stories &storyLevels, OtdSlab &otdslab) {
        GSErrCode err = NoError;
        API_Element slabobjelement = {};
        API_ElementMemo slabobjmemo = {};
        const GS::OnExit memoGuard ([&slabobjmemo] () { ACAPI_DisposeElemMemoHdls (&slabobjmemo); });
        bool is_new = (otdslab.otd_guid == APINULLGuid); // Элемент уже существует, нужно только обновить
        if (is_new) {
            if (!Floor_GetDefult_Object (favorite_name, slabobjelement, slabobjmemo))
                return;
        } else {
            slabobjelement.header.guid = otdslab.otd_guid;
            API_Guid savedGuid_Get = otdslab.otd_guid; // Сохраняем GUID для логирования перед обнулением
            err = ACAPI_Element_Get (&slabobjelement);
            if (err != NoError) {
                otdslab.otd_guid = APINULLGuid;
                msg_rep ("Floor_Draw_Object", "ACAPI_Element_Get", err, savedGuid_Get);
                return;
            }
            API_Guid savedGuid_GetMemo = otdslab.otd_guid; // Сохраняем GUID для логирования перед обнулением
            err = ACAPI_Element_GetMemo (otdslab.otd_guid, &slabobjmemo);
            if (err != NoError) {
                otdslab.otd_guid = APINULLGuid;
                msg_rep ("Floor_Draw_Object", "ACAPI_Element_GetMemo", err, savedGuid_GetMemo);
                return;
            }
        }
        GDLHelpers::ParamDict accsessoryparams;
        GDLHelpers::Param p;
        if (otdslab.type == Ceil) {
            // Высота уже задана положением объекта; GDL дополнительно прибавляет ac_ref_height.
            p.num = 0;
            accsessoryparams.Add ("{@gdl:ac_ref_height}", p);
            slabobjelement.object.level = GetOffsetFromStory (otdslab.zBottom, otdslab.floorInd, storyLevels);
        }
        if (otdslab.type == Floor) {
            p.num = otdslab.zBottom;
            accsessoryparams.Add ("{@gdl:ac_ref_height}", p);
            slabobjelement.object.level = GetOffsetFromStory (otdslab.zBottom, otdslab.floorInd, storyLevels);
        }
        p.str = otdslab.tip;
        accsessoryparams.Add ("{@gdl:tip_pol}", p);
        p.str = EMPTYSTRING;
        p.num = 1;
        accsessoryparams.Add ("{@gdl:mun_zone}", p);
        p.num = 0;
        accsessoryparams.Add ("{@gdl:ac_pitch}", p);
        p.num = 0;
        accsessoryparams.Add ("{@gdl:ac_ceilling_side}", p);
        p.num = 0;
        accsessoryparams.Add ("{@gdl:ac_ceiling_type}", p);
        p.num = otdslab.material.material;
        accsessoryparams.Add ("{@gdl:ceil_mat}", p);
        p.num = otdslab.material.material;
        accsessoryparams.Add ("{@gdl:matedge}", p);
        p.num = otdslab.material.material;
        accsessoryparams.Add ("{@gdl:matceipanel}", p);
        p.num = 0.02;
        accsessoryparams.Add ("{@gdl:ceil_thk}", p);
        p.num = 0.02;
        accsessoryparams.Add ("{@gdl:ac_thickness}", p);
        p.num = 0;
        accsessoryparams.Add ("{@gdl:ac_slab_side}", p);
        Geometry::Polygon2DData polygon2DData;
        Geometry::InitPolygon2DData (&polygon2DData);
        Geometry::ConvertPolygon2DToPolygon2DData (polygon2DData, otdslab.poly);
        Int32 nCoords = polygon2DData.nVertices;
        Int32 nSubPolys = polygon2DData.nContours;
        Box2DData bbox = otdslab.poly.GetBoundBox ();
        GDLHelpers::Param ac_coords;
        GDLHelpers::Param ac_whole_poly;
        for (Int32 j = 1; j <= nSubPolys; j++) {
            UInt32 begInd = (*polygon2DData.contourEnds)[j - 1] + 1;
            UInt32 endInd = (*polygon2DData.contourEnds)[j];
            double x_f = (*polygon2DData.vertices)[begInd].x - bbox.xMin;
            double y_f = (*polygon2DData.vertices)[begInd].y - bbox.yMin;
            for (UInt32 k = begInd; k < endInd; k++) {
                double x = (*polygon2DData.vertices)[k].x - bbox.xMin;
                double y = (*polygon2DData.vertices)[k].y - bbox.yMin;
                ac_coords.arr_num.Push (0.025);
                ac_coords.arr_num.Push (x);
                ac_coords.arr_num.Push (y);
                if (j == 1) {
                    ac_whole_poly.arr_num.Push (x);
                    ac_whole_poly.arr_num.Push (y);
                }
            }
            ac_coords.arr_num.Push (0.051);
            ac_coords.arr_num.Push (x_f);
            ac_coords.arr_num.Push (y_f);
        }
        ac_coords.dim1 = ac_coords.arr_num.GetSize () / 3;
        ac_coords.dim2 = 3;
        ac_whole_poly.dim1 = 1;
        ac_whole_poly.dim2 = ac_whole_poly.arr_num.GetSize ();
        accsessoryparams.Add ("{@gdl:ac_whole_poly}", ac_whole_poly);
        accsessoryparams.Add ("{@gdl:ac_coords}", ac_coords);
        if (is_new) {
            if (otdslab.type == Ceil) {
                p.num = bbox.xMax - bbox.xMin - 0.1;
                accsessoryparams.Add ("{@gdl:pos_x2}", p);
                p.num = bbox.yMax - bbox.yMin - 0.1;
                accsessoryparams.Add ("{@gdl:pos_y2}", p);

                p.num = (bbox.xMax - bbox.xMin - 0.1) / 2;
                accsessoryparams.Add ("{@gdl:pos_xtxt}", p);
                p.num = (bbox.yMax - bbox.yMin - 0.1) / 2;
                accsessoryparams.Add ("{@gdl:pos_ytxt}", p);
            }
            if (otdslab.type == Floor) {
                p.num = otdslab.poly.GetCenter ().x - bbox.xMin;
                accsessoryparams.Add ("{@gdl:pos_x1}", p);
                p.num = otdslab.poly.GetCenter ().y - bbox.yMin;
                accsessoryparams.Add ("{@gdl:pos_y1}", p);
            }
            p.dim1 = 1;
            p.dim2 = 2;
            p.arr_num.Push (otdslab.poly.GetCenter ().x - bbox.xMin);
            p.arr_num.Push (otdslab.poly.GetCenter ().y - bbox.yMin);
            accsessoryparams.Add ("{@gdl:fieldorig}", p);
        }
        Geometry::FreePolygon2DData (&polygon2DData);
        GDLHelpers::ParamToMemo (slabobjmemo, accsessoryparams);
        slabobjelement.header.floorInd = otdslab.floorInd;
        slabobjelement.object.pos.x = bbox.xMin;
        slabobjelement.object.pos.y = bbox.yMin;
        double y_start = 0;
        if (is_new) {
            err = ACAPI_Element_Create (&slabobjelement, &slabobjmemo);
        } else {
            API_Element mask = {};
            ACAPI_ELEMENT_MASK_CLEAR (mask);
            ACAPI_ELEMENT_MASK_SET (mask, API_ObjectType, pos);
            ACAPI_ELEMENT_MASK_SET (mask, API_ObjectType, level);
            ACAPI_ELEMENT_MASK_SET (mask, API_Elem_Head, floorInd);
            err = ACAPI_Element_Change (&slabobjelement, &mask, &slabobjmemo, APIMemoMask_AddPars, true);
        }
        if (err != NoError) {
            otdslab.otd_guid = APINULLGuid;
            msg_rep ("Floor_Draw_Object", favorite_name, err, APINULLGuid);
            return;
        } else {
            otdslab.otd_guid = slabobjelement.header.guid;
        }
    }

    // Возвращает объект из избранного подходящего типа либо получает настройки объекта по умолчанию.
    // Только для настроек по умолчанию сбрасывает угол и автовидимость по этажам, включает перья, линии и материалы
    // объекта.
    bool Floor_GetDefult_Object (const GS::UniString &favorite_name,
                                 API_Element &slabobjelement,
                                 API_ElementMemo &slabobjmemo) {
        GSErrCode err = NoError;
        BNZeroMemory (&slabobjmemo, sizeof (API_ElementMemo));
        BNZeroMemory (&slabobjelement, sizeof (API_Element));
        if (Favorite_GetByName (favorite_name, slabobjelement, slabobjmemo)) {
            if (GetElemTypeID (slabobjelement) == API_ObjectID)
                return true;
        }
        SetElemTypeID (slabobjelement, API_ObjectID);
        err = ACAPI_Element_GetDefaults (&slabobjelement, &slabobjmemo);
        if (err != NoError) {
            ACAPI_DisposeElemMemoHdls (&slabobjmemo);
            return false;
        }
        slabobjelement.object.angle = 0;
        slabobjelement.object.isAutoOnStoryVisibility = false;
        slabobjelement.object.useObjPens = true;
        slabobjelement.object.useObjLtypes = true;
        slabobjelement.object.useObjMaterials = true;
        return true;
    }

    // Возвращает перекрытие из избранного подходящего типа либо получает настройки перекрытия по умолчанию.
    // Только для настроек по умолчанию задаёт нулевой offsetFromTop, связанные материалы и базовую структуру.
    bool Floor_GetDefult_Slab (const GS::UniString &favorite_name, API_Element &slabelement) {
        GSErrCode err = NoError;
        BNZeroMemory (&slabelement, sizeof (API_Element));
        if (Favorite_GetByName (favorite_name, slabelement)) {
            if (GetElemTypeID (slabelement) == API_SlabID)
                return true;
        }
        SetElemTypeID (slabelement, API_SlabID);
        err = ACAPI_Element_GetDefaults (&slabelement, nullptr);
        if (err != NoError) {
            return false;
        }
        slabelement.slab.offsetFromTop = 0;
        slabelement.slab.materialsChained = true;
    #ifdef ServerMainVers_2600
    #else
        slabelement.slab.sideMat.overridden = true;
        slabelement.slab.topMat.overridden = true;
        slabelement.slab.botMat.overridden = true;
    #endif
        slabelement.slab.modelElemStructureType = API_BasicStructure;
        return true;
    }

    // -----------------------------------------------------------------------------
    // Назначение класса стене
    // -----------------------------------------------------------------------------
    void Class_SetClass (const OtdWall &op, const ClassificationFunc::ClassificationDict &finclass) {
        if (op.otd_guid == APINULLGuid)
            return;
        API_Guid class_guid = Class_GetClassGuid (op.type, finclass);
        if (class_guid != APINULLGuid)
            ACAPI_Element_AddClassificationItem (op.otd_guid, class_guid);
    }

    // -----------------------------------------------------------------------------
    // Назначение класса проёму
    // -----------------------------------------------------------------------------
    // Для ненулевого GUID проёма выбирает класс NoSet, затем при его отсутствии класс Wall_Main.
    // Вызывает добавление классификации только при ненулевом GUID выбранного класса.
    void Class_SetClass (const OtdOpening &op, const ClassificationFunc::ClassificationDict &finclass) {
        if (op.otd_guid == APINULLGuid)
            return;
        API_Guid class_guid = Class_GetClassGuid (NoSet, finclass);
        if (class_guid == APINULLGuid)
            class_guid = Class_GetClassGuid (Wall_Main, finclass);
        if (class_guid != APINULLGuid)
            ACAPI_Element_AddClassificationItem (op.otd_guid, class_guid);
    }

    // -----------------------------------------------------------------------------
    // Назначение класса полу/потолку
    // -----------------------------------------------------------------------------
    void Class_SetClass (const OtdSlab &op, const ClassificationFunc::ClassificationDict &finclass) {
        if (op.otd_guid == APINULLGuid)
            return;
        API_Guid class_guid = Class_GetClassGuid (op.type, finclass);
        if (class_guid != APINULLGuid)
            ACAPI_Element_AddClassificationItem (op.otd_guid, class_guid);
    }

    // Выбирает GUID класса по типу отделки; для откосов и нижней стены пробует также общий класс стен.
    // Если подходящий класс не найден, возвращает класс all_class либо APINULLGuid.
    API_Guid Class_GetClassGuid (const TypeOtd &type, const ClassificationFunc::ClassificationDict &finclass) {
        switch (type) {
        case Column:
            if (auto *p = finclass.GetPtr (cls.column_class))
                return p->item.guid;
            break;

        case Reveal_Main:
        case Reveal_Up:
        case Reveal_Down:
            if (auto *p = finclass.GetPtr (cls.reveal_class))
                return p->item.guid;
            if (auto *p = finclass.GetPtr (cls.otdwall_class))
                return p->item.guid;
            break;

        case Wall_Down:
            if (auto *p = finclass.GetPtr (cls.otdwall_down_class))
                return p->item.guid;
            if (auto *p = finclass.GetPtr (cls.otdwall_class))
                return p->item.guid;
            break;

        case Wall_Main:
        case Wall_Up:
            if (auto *p = finclass.GetPtr (cls.otdwall_class))
                return p->item.guid;
            break;

        case Floor:
            if (auto *p = finclass.GetPtr (cls.floor_class))
                return p->item.guid;
            break;

        case Ceil:
            if (auto *p = finclass.GetPtr (cls.ceil_class))
                return p->item.guid;
            break;

        default:
            break;
        }
        if (auto *p = finclass.GetPtr (cls.all_class))
            return p->item.guid;
        return APINULLGuid;
    }

    // Сопоставляет GUID с классами стен, нижней стены, потолка, пола, колонны и откосов в порядке проверок.
    // Классы оконных и дверных откосов возвращают Reveal_Main; общий и неизвестный класс возвращают NoSet.
    TypeOtd Class_GetOtdTypeByClass (const API_Guid &classguid,
                                     const ClassificationFunc::ClassificationDict &finclass) {

        if (const auto *p = finclass.GetPtr (cls.otdwall_class)) {
            if (p->item.guid == classguid)
                return Wall_Main;
        }

        if (const auto *p = finclass.GetPtr (cls.otdwall_down_class)) {
            if (p->item.guid == classguid)
                return Wall_Down;
        }

        if (const auto *p = finclass.GetPtr (cls.ceil_class)) {
            if (p->item.guid == classguid)
                return Ceil;
        }

        if (const auto *p = finclass.GetPtr (cls.floor_class)) {
            if (p->item.guid == classguid)
                return Floor;
        }

        if (const auto *p = finclass.GetPtr (cls.column_class)) {
            if (p->item.guid == classguid)
                return Column;
        }

        if (const auto *p = finclass.GetPtr (cls.reveal_class)) {
            if (p->item.guid == classguid)
                return Reveal_Main;
        }

        if (const auto *p = finclass.GetPtr (cls.reveal_windows_class)) {
            if (p->item.guid == classguid)
                return Reveal_Main;
        }

        if (const auto *p = finclass.GetPtr (cls.reveal_doors_class)) {
            if (p->item.guid == classguid)
                return Reveal_Main;
        }

        if (const auto *p = finclass.GetPtr (cls.all_class)) {
            if (p->item.guid == classguid)
                return NoSet;
        }

        return NoSet;
    }

    // -----------------------------------------------------------------------------
    // Поиск классов для отделочных стен (some_stuff_fin_ в описании класса)
    // -----------------------------------------------------------------------------
    void Class_FindFinClass (ClassificationFunc::ClassificationDict &findict, UnicGuid &finclassguids) {
        if (!ClassificationFunc::ReadSystemDict ())
            return;
        ClassificationFunc::SystemDict &systemdict = PROPERTYCACHE ().systemdict;
        for (GS::HashTable<GS::UniString, ClassificationFunc::ClassificationDict>::PairIterator cIt =
                 systemdict.EnumeratePairs ();
             cIt != NULL;
             ++cIt) {
    #ifdef ServerMainVers_2800
            GS::UniString system = cIt->key;
            ClassificationFunc::ClassificationDict &classes = cIt->value;
    #else
            GS::UniString system = *cIt->key;
            ClassificationFunc::ClassificationDict &classes = *cIt->value;
    #endif
            for (GS::HashTable<GS::UniString, ClassificationFunc::ClassificationValues>::PairIterator cIt =
                     classes.EnumeratePairs ();
                 cIt != NULL;
                 ++cIt) {
    #ifdef ServerMainVers_2800
                GS::UniString clasname = cIt->key;
                ClassificationFunc::ClassificationValues &clas = cIt->value;
    #else
                GS::UniString clasname = *cIt->key;
                ClassificationFunc::ClassificationValues &clas = *cIt->value;
    #endif
                GS::UniString desc = UniStringToLower (clas.item.description);
                if (desc.Contains ("some_stuff_fin_")) {
                    if (desc.Contains (cls.all_class)) {
                        findict.Add (cls.all_class, clas);
                        if (!finclassguids.ContainsKey (clas.item.guid))
                            finclassguids.Add (clas.item.guid, false);
                    }
                    if (desc.Contains (cls.ceil_class)) {
                        findict.Add (cls.ceil_class, clas);
                        if (!finclassguids.ContainsKey (clas.item.guid))
                            finclassguids.Add (clas.item.guid, false);
                    }
                    if (desc.Contains (cls.column_class)) {
                        findict.Add (cls.column_class, clas);
                        if (!finclassguids.ContainsKey (clas.item.guid))
                            finclassguids.Add (clas.item.guid, false);
                    }
                    if (desc.Contains (cls.floor_class)) {
                        findict.Add (cls.floor_class, clas);
                        if (!finclassguids.ContainsKey (clas.item.guid))
                            finclassguids.Add (clas.item.guid, false);
                    }
                    if (desc.Contains (cls.otdwall_class)) {
                        findict.Add (cls.otdwall_class, clas);
                        if (!finclassguids.ContainsKey (clas.item.guid))
                            finclassguids.Add (clas.item.guid, false);
                    }
                    if (desc.Contains (cls.reveal_class)) {
                        findict.Add (cls.reveal_class, clas);
                        if (!finclassguids.ContainsKey (clas.item.guid))
                            finclassguids.Add (clas.item.guid, false);
                    }
                    if (desc.Contains (cls.reveal_windows_class)) {
                        findict.Add (cls.reveal_windows_class, clas);
                        if (!finclassguids.ContainsKey (clas.item.guid))
                            finclassguids.Add (clas.item.guid, false);
                    }
                    if (desc.Contains (cls.reveal_doors_class)) {
                        findict.Add (cls.reveal_doors_class, clas);
                        if (!finclassguids.ContainsKey (clas.item.guid))
                            finclassguids.Add (clas.item.guid, false);
                    }
                    if (desc.Contains (cls.otdwall_down_class)) {
                        findict.Add (cls.otdwall_down_class, clas);
                        if (!finclassguids.ContainsKey (clas.item.guid))
                            finclassguids.Add (clas.item.guid, false);
                    }
                }
            }
        }
    }

    // -----------------------------------------------------------------------------
    // Связывание созданных элементов отделки с базовыми элементами
    // -----------------------------------------------------------------------------
    void SetSyncOtdWall (UnicElementByType &subelementByparent, ParamDictElement &paramToWrite) {
        API_Elem_Head parentelementhead;
        UnicGuid syncguidsdict;
        GS::UniString suffix;
        GS::Array<API_Guid> syncguids;
        GS::UniString funcname = "Create link with base element";
        SuspendGroupsGuard suspGuard; // отключаем режим группировки на время записи.
        for (const API_ElemTypeID &typeelem : typeinzone) {
            if (!subelementByparent.ContainsKey (typeelem))
                continue;
            for (UnicElement::PairIterator cIt = subelementByparent.Get (typeelem).EnumeratePairs (); cIt != NULL;
                 ++cIt) {
    #ifdef ServerMainVers_2800
                API_Guid guid = cIt->key;
                const GS::Array<API_Guid> &subguids = cIt->value;
    #else
                API_Guid guid = *cIt->key;
                const GS::Array<API_Guid> &subguids = *cIt->value;
    #endif

                if (AdvanceProcessPhase (funcname, nPhase))
                    return;

                parentelementhead.guid = guid;
                if (typeelem == API_ZoneID) {
                    suffix = "zone";
                } else {
                    suffix = "base element";
                }
                if (!SyncSetSubelementScope (parentelementhead, subguids, paramToWrite, suffix, false)) {
    #if defined(TESTING)
                    // Диагностика #193: привязка Sync_GUID не записана
                    DBprnt ("SetSyncOtdWall",
                            "SyncSetSubelementScope FAILED: suffix=" + suffix +
                                " subguids=" + GS::UniString::Printf ("%u", (UInt32)subguids.GetSize ()));
    #endif
                    continue;
                }
                for (const auto &g : subguids) {
                    if (!syncguidsdict.ContainsKey (g)) {
                        syncguidsdict.Add (g, true);
                        syncguids.Push (g);
                    }
                }
            }
        }
        if (!paramToWrite.IsEmpty ()) {
            funcname = GS::UniString::Printf ("Write GUID base and GUID zone to %d finishing element(s)",
                                              paramToWrite.GetSize ());
            if (AdvanceProcessPhase (funcname, nPhase)) {
                return;
            }
            ACAPI_CallUndoableCommand ("Write property to finishing element", [&] () -> GSErrCode {
                ParamHelpers::ElementsWrite (paramToWrite);
                return NoError;
            });
        }
        if (syncguids.IsEmpty ())
            return;
        funcname = GS::UniString::Printf ("Sync %d finishing element with base and zone", paramToWrite.GetSize ());
        if (AdvanceProcessPhase (funcname, nPhase)) {
            return;
        }
        SyncSettings syncSettings;
        LoadSyncSettingsFromPreferences (syncSettings);
        GS::Array<API_Guid> rereadelem = SyncArray (syncSettings, syncguids);
        if (!rereadelem.IsEmpty ()) {
    #if defined(TESTING)
            DBprnt ("===== REREAD =======");
    #endif
            SyncArray (syncSettings, rereadelem);
        }
    }

    // Ищет среди классификаций элемента первый GUID, присутствующий в finclassguids, записывает его в classguid и
    // возвращает true. При ошибке чтения или отсутствии совпадения возвращает false, не меняя classguid.
    bool Class_IsElementFinClass (const API_Guid &elGuid,
                                  const UnicGuid &finclassguids,
                                  API_Guid &classguid,
                                  RoomSurfaceCache *cache) {
        const ClassificationRead read = ReadClassificationItems (elGuid, cache);
        if (read.error != NoError || read.items.IsEmpty ())
            return false;
        for (const auto &cl : read.items) {
            if (!finclassguids.ContainsKey (cl.second))
                continue;
            classguid = cl.second;
            return true;
        }
        return false;
    }

    // Добавляет GUID потомка в массив по типу и GUID родителя, создавая отсутствующие записи; нулевые GUID пропускает.
    // Повторные GUID не проверяет и не удаляет.
    void Param_AddUnicElementByType (const API_Guid &parentguid,
                                     const API_Guid &guid,
                                     API_ElemTypeID elemtype,
                                     UnicElementByType &elementToRead) {
        if (parentguid == APINULLGuid)
            return;
        if (guid == APINULLGuid)
            return;

        auto *bytypePtr = elementToRead.GetPtr (elemtype);

        if (bytypePtr == nullptr) {
            UnicElement el;
            elementToRead.Add (elemtype, std::move (el));
            bytypePtr = elementToRead.GetPtr (elemtype);
        }

        if (bytypePtr == nullptr)
            return;
        if (auto *byparent = bytypePtr->GetPtr (parentguid)) {
            byparent->Push (guid);
        } else {
            GS::Array<API_Guid> el;
            el.Push (guid);
            bytypePtr->Add (parentguid, std::move (el));
        }
    }

    // Добавляет ненулевой GUID в массив заданного типа, создавая массив при отсутствии; дубликаты не проверяет.
    void Param_AddUnicGUIDByType (const API_Guid &elGuid, API_ElemTypeID elemtype, UnicGUIDByType &guidselementToRead) {
        if (elGuid == APINULLGuid)
            return;
        if (auto *p = guidselementToRead.GetPtr (elemtype)) {
            p->Push (elGuid);
        } else {
            GS::Array<API_Guid> z;
            z.Push (elGuid);
            guidselementToRead.Add (elemtype, std::move (z));
        }
    }

    // Возвращает тип избранного по имени; для пустого имени, ошибки чтения или отсутствия ServerMainVers_2300
    // возвращает API_ZombieElemID.
    API_ElemTypeID Favorite_GetType (const GS::UniString &favorite_name) {
        GSErrCode err = NoError;
    #ifndef ServerMainVers_2300
        return API_ZombieElemID;
    #else
        if (favorite_name.IsEmpty ())
            return API_ZombieElemID;
        API_Favorite favorite (favorite_name);
        err = ACAPI_Favorite_Get (&favorite);
        if (err == NoError)
            return GetElemTypeID (favorite.element.header);
        return API_ZombieElemID;
    #endif
    }

    // Собирает избранное окон, дверей, стен, колонн, перекрытий, объектов и зон в словарь по обрезанным именам в нижнем
    // регистре. Повторные имена пропускает и сообщает об отсутствии рекомендуемых имён; балки в перечень собираемых
    // типов не входят.
    MatarialToFavoriteDict Favorite_GetDict () {
        // Словарь с избранным
        MatarialToFavoriteDict favdict;
        MatarialToFavorite fav;
        // Поиск элементов по-умолчанию
        short count = 0;
        GS::Array<GS::UniString> names;
    #if defined(TESTING)
        // Диагностика #192: GetNum возвращает names только корневых избранных?
        // Логируем count/names/folders по каждому типу, чтобы увидеть,
        // попадают ли избранные из папок палитры «Избранное» в names.
        for (const auto &type : typeinzone) {
            short diag_count = 0;
            GS::Array<GS::UniString> diag_names;
            GS::Array<API_FavoriteFolderHierarchy> diag_folders;
            GSErrCode diag_err = Favorite_GetNum (type, &diag_count, &diag_folders, &diag_names);
            GS::UniString msg = GS::UniString::Printf ("type=%hd err=%lld count=%hd names=%u folders=%u",
                                                       (short)type,
                                                       (long long)diag_err,
                                                       diag_count,
                                                       (UInt32)diag_names.GetSize (),
                                                       (UInt32)diag_folders.GetSize ());
            for (const GS::UniString &n : diag_names)
                msg += " | '" + n + "'";
            for (const API_FavoriteFolderHierarchy &f : diag_folders)
                for (const GS::UniString &fn : f)
                    msg += " F{" + fn + "}";
            DBprnt ("Favorite_GetDict", msg);
        }
        // Пробник #192: доступно ли избранное по точному имени (тем же вызовом,
        // которым потом создаётся элемент) и какого оно типа.
        for (const GS::UniString &probeName :
             {GS::UniString ("smstf wall"), GS::UniString ("smstf floor"), GS::UniString ("smstf ceil")}) {
            API_Favorite favorite (probeName);
            GSErrCode probe_err = ACAPI_Favorite_Get (&favorite);
            DBprnt ("Favorite_GetDict probe",
                    probeName + " err=" +
                        GS::UniString::Printf (
                            "%lld type=%hd", (long long)probe_err, (short)GetElemTypeID (favorite.element.header)));
        }
    #endif
        // #192: избранные полов/потолков могут быть объектами (API_ObjectID) —
        // GetNum по API_SlabID их не возвращает, поэтому собираем и объектные.
        // Favorite_FindName/Floor_Draw уже учитывают favorite.type == API_ObjectID.
        const API_ElemTypeID favtypes[] = {
            API_WindowID, API_DoorID, API_WallID, API_ColumnID, API_SlabID, API_ObjectID, API_ZoneID};
        for (const auto &type : favtypes) {
            if (Favorite_GetNum (type, &count, nullptr, &names) != NoError)
                continue;
            fav.type = type;
            for (GS::UniString &name : names) {
                name.SetToLowerCase ();
                name.Trim ();
                fav.name = name;
                if (!favdict.ContainsKey (name))
                    favdict.Add (name, fav);
            }
        }
        if (favdict.IsEmpty ()) {
            msg_rep ("RoomBook favorite", "Recommended items not found in favorites", NoError, APINULLGuid);
        } else {
            GS::UniString msg;
            if (!favdict.ContainsKey ("smstf wall"))
                msg += "'smstf wall' ";
            if (!favdict.ContainsKey ("smstf floor"))
                msg += "'smstf floor' ";
            if (!favdict.ContainsKey ("smstf ceil"))
                msg += "'smstf ceil' ";
            if (!favdict.ContainsKey ("smstf window"))
                msg += "'smstf window' ";
            if (!msg.IsEmpty ()) {
                msg_rep ("RoomBook favorite",
                         "The recommended elements were not found in Favorites. Default "
                         "tool settings will be used to create these elements.\nMissing "
                         "element names:" +
                             msg,
                         NoError,
                         APINULLGuid);
            }

            msg = EMPTYSTRING;
            if (!favdict.ContainsKey ("smstf reveal side"))
                msg += "'smstf reveal side' ";
            if (!favdict.ContainsKey ("smstf reveal up"))
                msg += "'smstf reveal up' ";
            if (!msg.IsEmpty ()) {
                if (favdict.ContainsKey ("smstf wall")) {
                    msg_rep ("RoomBook favorite",
                             "The recommended elements for reveal were not found in "
                             "Favorites. 'smstf wall' will be used to create these "
                             "elements.\nMissing element names:" +
                                 msg,
                             NoError,
                             APINULLGuid);
                } else {
                    msg_rep ("RoomBook favorite",
                             "The recommended elements for reveal were not found in "
                             "Favorites. Default tool settings will be used to "
                             "create these "
                             "elements.\nMissing element names:" +
                                 msg,
                             NoError,
                             APINULLGuid);
                }
            }
        }
        return favdict;
    }

    // Помечает избранное как прочитанное до получения элемента и чтения материалов; при дальнейшей ошибке флаг не
    // сбрасывает. При наличии требуемого параметра и состава материала заполняет composite через Param_SetComposite.
    void Favorite_ReadComposite (const ParamValue &param_composite,
                                 MatarialToFavoriteDict &favdict,
                                 ParamDictValue &paramToRead_favorite,
                                 const GS::UniString &fav_name) {

        // ОПТИМИЗАЦИЯ: 1 поиск в favdict вместо 5 (!) с одновременной проверкой флага
        auto *favData = favdict.GetPtr (fav_name);
        if (favData == nullptr || favData->is_composite_read)
            return;

        favData->is_composite_read = true;

        API_Element element = {};
        if (!Favorite_GetByName (favData->name, element))
            return;

        // ОПТИМИЗАЦИЯ: Получаем элемент для записи за 1 проход вместо ContainsKey + Set
        ParamValue *favParamPtr = paramToRead_favorite.GetPtr (param_composite.rawName);
        if (favParamPtr == nullptr)
            return;

        *favParamPtr = param_composite;

        ParamDictComposite paramcomposite;
        bool needReadQuantities = false;
        if (!ParamHelpers::ReadMaterial (element, paramToRead_favorite, paramcomposite, needReadQuantities))
            return;
        favParamPtr = paramToRead_favorite.GetPtr (param_composite.rawName);
        if (favParamPtr == nullptr || !favParamPtr->isValid)
            return;
        const ParamComposite *base_composite = paramcomposite.GetPtr (param_composite.rawName);
        if (base_composite == nullptr)
            return;
        GS::UniString fav_name_;
        Param_SetComposite (*base_composite, element.wall.flipped, favData->composite, fav_name_, true);
    }

    // Подбирает избранное по имени материала и суффиксу, затем по текущему или рекомендуемому имени.
    // Учитывает draw_type и допустимый объект-аксессуар; выбранную запись копирует вместе с прочитанным составом.
    void Favorite_FindName (MatarialToFavorite &favorite,
                            const OtdMaterial &material,
                            const TypeOtd &type,
                            const API_ElemTypeID &draw_type,
                            MatarialToFavoriteDict &favdict,
                            ParamDictValue &paramToRead_favorite,
                            ParamValue &param_composite) {
        API_ElemTypeID possibly_type = draw_type;
        GS::UniString fav_name = favorite.name;
        if (favdict.IsEmpty ())
            return;
        if (material.smaterial.Contains (ATSIGN)) {
            GS::UniString part = UniStringToLower (material.smaterial) + '@';
            part = part.GetSubstring ('@', '@', 0);
            part.Trim ();
            if (!fav_name.IsEmpty ()) {
                GS::UniString part_full = part + "_" + fav_name;
                if (auto *favPtr = favdict.GetPtr (part_full)) {
                    if (favPtr->type == draw_type || favPtr->type == API_ObjectID) {
                        Favorite_ReadComposite (param_composite, favdict, paramToRead_favorite, part_full);
                        if (auto *updatedFavPtr = favdict.GetPtr (part_full)) {
                            favorite = *updatedFavPtr;
                        }
                        return;
                    }
                }
            }
            if (auto *favPtr = favdict.GetPtr (part)) {
                if (favPtr->type == draw_type || favPtr->type == API_ObjectID) {
                    Favorite_ReadComposite (param_composite, favdict, paramToRead_favorite, part);
                    if (auto *updatedFavPtr = favdict.GetPtr (part)) {
                        favorite = *updatedFavPtr;
                    }
                    return;
                }
            }
        }
        if (!fav_name.IsEmpty ()) {
            if (auto *favPtr = favdict.GetPtr (fav_name)) {
                if (favPtr->type == draw_type || favPtr->type == API_ObjectID) {
                    Favorite_ReadComposite (param_composite, favdict, paramToRead_favorite, fav_name);
                    if (auto *updatedFavPtr = favdict.GetPtr (fav_name)) {
                        favorite = *updatedFavPtr;
                    }
                    return;
                }
            }
        }
        if (favorite.type == API_ZombieElemID) {
            GS::UniString favorite_name_defult;
            switch (type) {
            case Wall_Main:
            case Wall_Up:
            case Wall_Down:
            case Column:
                favorite_name_defult = "smstf wall";
                break;
            case Reveal_Main:
            case Reveal_Up:
            case Reveal_Down:
                if (draw_type == API_BeamID) {
                    favorite_name_defult = "smstf reveal up";
                }
                if (draw_type == API_ColumnID) {
                    favorite_name_defult = "smstf reveal side";
                }
                if (!favdict.ContainsKey (favorite_name_defult) || favorite_name_defult.IsEmpty ()) {
                    favorite_name_defult = "smstf wall";
                    possibly_type = API_WallID;
                }
                break;
            case Floor:
                favorite_name_defult = "smstf floor";
                if (draw_type == API_WallID) {
                    favorite_name_defult = "smstf wall";
                    possibly_type = API_WallID;
                }
                break;
            case Ceil:
                favorite_name_defult = "smstf ceil";
                break;
            default:
                return;
                break;
            }
            if (auto *favPtr = favdict.GetPtr (favorite_name_defult)) {
                if (favPtr->type == possibly_type || favPtr->type == API_ObjectID) {
                    Favorite_ReadComposite (param_composite, favdict, paramToRead_favorite, favorite_name_defult);
                    if (auto *updatedFavPtr = favdict.GetPtr (favorite_name_defult)) {
                        favorite = *updatedFavPtr;
                    }
                    return;
                }
            }
        }
        if (favorite.name.IsEmpty ())
            favorite.type = draw_type;
        return;
    }

    // При успешном чтении избранного копирует элемент, вызывает UnhideUnlockElementLayer и возвращает true.
    // Для пустого имени, ошибки чтения или отсутствия ServerMainVers_2300 возвращает false.
    bool Favorite_GetByName (const GS::UniString &favorite_name, API_Element &element) {
        GSErrCode err = NoError;
    #ifndef ServerMainVers_2300
        return false;
    #else
        if (favorite_name.IsEmpty ())
            return false;
        API_Favorite favorite (favorite_name);
        err = ACAPI_Favorite_Get (&favorite);
        if (err == NoError) {
            element = favorite.element;
            UnhideUnlockElementLayer (element.header);
            return true;
        }
        return false;
    #endif
    }

    // Читает избранное с memo; при успехе копирует элемент и memo, вызывает UnhideUnlockElementLayer и возвращает true.
    // При ошибке чтения освобождает memo избранного; пустое имя и отсутствие ServerMainVers_2300 также дают false.
    bool Favorite_GetByName (const GS::UniString &favorite_name, API_Element &element, API_ElementMemo &memo) {
        GSErrCode err = NoError;
    #ifndef ServerMainVers_2300
        return false;
    #else
        if (favorite_name.IsEmpty ())
            return false;
        API_Favorite favorite (favorite_name);
        favorite.memo.New ();
        BNZeroMemory (&favorite.memo.Get (), sizeof (API_ElementMemo));
        GS::OnExit favMemoGuard ([&favorite] () { ACAPI_DisposeElemMemoHdls (&favorite.memo.Get ()); });
        err = ACAPI_Favorite_Get (&favorite);
        if (err == NoError) {
            element = favorite.element;
            UnhideUnlockElementLayer (element.header);
            memo = *favorite.memo;
            favMemoGuard.Deactivate ();
            return true;
        }
        return false;
    #endif
    }

    // Проверяет наличие классов отделки и доступность Sync_GUID zone для GUID в finclassguids.
    // Уже установленные флаги не сбрасывает; проблемы Sync_GUID base element сообщает без отказа расчёта.
    bool Check (const ClassificationFunc::ClassificationDict &finclass, UnicGuid &finclassguids) {
        // Проверка наличия классов
        const Int32 iseng = ID_ADDON_STRINGS + isEng ();
        if (finclass.IsEmpty ()) {
            GS::UniString msgString = RSGetIndString (iseng, 50, ACAPI_GetOwnResModule ());
            msg_rep ("RoomBook err", msgString, APIERR_GENERAL, APINULLGuid);
            SafeWriteReport (msgString, true);
            return false;
        }
        if (!finclass.ContainsKey (cls.all_class)) {
            bool msg = false;
            if (!finclass.ContainsKey (cls.floor_class))
                msg = true;
            if (!finclass.ContainsKey (cls.ceil_class))
                msg = true;
            if (!finclass.ContainsKey (cls.otdwall_class))
                msg = true;
            if (msg) {
                GS::UniString msgString = RSGetIndString (iseng, 51, ACAPI_GetOwnResModule ());
                msg_rep ("RoomBook err", msgString, APIERR_GENERAL, APINULLGuid);
                return false;
            }
        }
        // Проверка наличия свойств `Sync_GUID zone`
        if (!ParamHelpers::isPropertyDefinitionRead ())
            return false;
        const ParamDictValue &propertyParams = PROPERTYCACHE ().property;
        for (const auto &cItt : propertyParams) {
    #ifdef ServerMainVers_2800
            const ParamValue &param = cItt.value;
    #else
            const ParamValue &param = *cItt.value;
    #endif
            if (param.definition.description.Contains (SYNCGUID) && param.definition.description.Contains ("zone")) {
                for (const API_Guid &classguid : param.definition.availability) {
                    if (finclassguids.ContainsKey (classguid))
                        finclassguids.Set (classguid, true);
                }
            }
        }
        for (GS::HashTable<API_Guid, bool>::PairIterator cIt = finclassguids.EnumeratePairs (); cIt != NULL; ++cIt) {
    #ifdef ServerMainVers_2800
            bool msg = cIt->value;
    #else
            bool msg = *cIt->value;
    #endif
            if (!msg) {
                GS::UniString msgString = RSGetIndString (iseng, 52, ACAPI_GetOwnResModule ());
                msg_rep ("RoomBook err", msgString, APIERR_GENERAL, APINULLGuid);
                SafeWriteReport (msgString, true);
                return false;
            }
        }

        // Проверка наличия свойств `Sync_GUID base`
        bool find = false;
        for (const auto &cItt : propertyParams) {
    #ifdef ServerMainVers_2800
            const ParamValue &param = cItt.value;
    #else
            const ParamValue &param = *cItt.value;
    #endif
            if (param.definition.description.Contains (SYNCGUID) &&
                param.definition.description.Contains ("base element")) {
                find = true;
                bool find_cls = false;
                for (const API_Guid &classguid : param.definition.availability) {
                    if (finclassguids.ContainsKey (classguid))
                        find_cls = true;
                }
                if (!find_cls) {
                    msg_rep ("RoomBook",
                             "'Sync_GUID base element' property found, but not "
                             "displayed in "
                             "Finish Element classification.\nFloor/ceiling updates "
                             "require "
                             "this property to be visible. Existing elements will be "
                             "replaced.",
                             NoError,
                             APINULLGuid);
                }
            }
        }
        if (!find) {
            msg_rep ("RoomBook",
                     "Missing 'Sync_GUID base element' property. Floor/ceiling updates "
                     "require this property. Existing elements will be replaced",
                     NoError,
                     APINULLGuid);
        }
        return true;
    }
#endif
} // namespace Roombook
