// clang-format off
#include "ACAPinc.h"
// clang-format on

#include "api_headers/APIEnvir.h"

#include "Algorithms.hpp"
#include "Helpers.hpp"
#include "ProfileAdditionalInfo.hpp"
#include "ProfileVectorImage.hpp"
#include "ProfileVectorImageOperations.hpp"
#include "Propertycache.hpp"
#include "roombook/FloorCoreGeometry.hpp"
#include "roombook/RoomAnalyticGeometry.hpp"
#include "roombook/Roombook.hpp"
#include "Sync.hpp"
#include "third_party/alphanum.h"
#include "VectorImageIterator.hpp"

#ifdef ServerMainVers_2400
    #include <algorithm>
    #include <cmath>
    #include <limits>
    #include <map>
    #include <string>
    #include <utility>
    #include <vector>

    #include "CommonFunction.hpp"
    #include "OnExit.hpp"
    #include "Polygon2D.hpp"

namespace Roombook {

    struct GuidByZ {
        double r = 0;
        double zBottom = 0;
        double zUp = 0;
        API_Guid guid = APINULLGuid;
    };

    struct RoomEdges {
        GS::Array<Sector> walledges;   // Границы стен, не явяющихся границей зоны
        GS::Array<Sector> columnedges; // Границы колонн, не явяющихся границей зоны
        GS::Array<Sector> restedges;   // Границы зоны
        GS::Array<Sector> gableedges;
    };

    static ElementRead Element_Read (const API_Guid &guid, RoomSurfaceCache *cache = nullptr);

    static ClassificationRead Class_ReadItems (const API_Guid &guid, RoomSurfaceCache *cache = nullptr);

    static GSErrCode Zone_ReadRelations (const API_Guid &zoneGuid, RoomSurfaceCache *cache, RoomRelationValues &values);

    // Набор SDK relations, не доказательство полноты произвольных касаний в 3D.
    static RoomSurfaceCandidates RoomSurfaceCandidates_Read (const API_Elem_Head &zone,
                                                             const GS::Array<API_Guid> &finishClassGuids,
                                                             RoomSurfaceCache *cache = nullptr);

    // -----------------------------------------------------------------------------
    // Получение информации из зоны о полгионах и находящейся в ней элементах
    // -----------------------------------------------------------------------------
    static bool OtdRoom_CollectInfo (const Stories &storyLevels,
                                     const API_Guid &zoneGuid,
                                     OtdRoom &roominfo,
                                     UnicElementByType &elementToRead,
                                     GS::HashTable<API_Guid, GS::Array<API_Guid>> &slabsinzone,
                                     RoomSurfaceCache *cache = nullptr,
                                     CoordinateSearchCache *coordinateCache = nullptr,
                                     const MeshFloorIndex *meshesinzone = nullptr);

    // -----------------------------------------------------------------------------
    // Находит все перекрытия, котрые пересекают зону и не являются элементами отделки
    // -----------------------------------------------------------------------------
    static void OtdSlab_FindAll (GS::HashTable<API_Guid, GS::Array<API_Guid>> &slabsinzone,
                                 const UnicGuid &finclassguids,
                                 const GS::Array<API_Guid> &zones,
                                 RoomSurfaceCache *cache = nullptr);

    // -----------------------------------------------------------------------------
    // Чтение данных об одном проёме
    // -----------------------------------------------------------------------------
    static void Opening_Create_One (const Stories &storyLevels,
                                    const API_Guid &elGuid,
                                    GS::HashTable<API_Guid, GS::Array<OtdOpening>> &openinginwall,
                                    UnicGUIDByType &guidselementToRead);

    // Создание стен-отделок для стен
    // -----------------------------------------------------------------------------
    static void OtdWall_Create_FromWall (const Stories &storyLevels,
                                         const API_Guid &elGuid,
                                         const GS::Array<API_Guid> &zoneGuids,
                                         OtdRooms &roomsinfo,
                                         const GS::HashTable<API_Guid, GS::Array<OtdOpening>> &openinginwall,
                                         UnicGUIDByType &guidselementToRead,
                                         RoomSurfaceCache *cache = nullptr);

    // -----------------------------------------------------------------------------
    // Обработка полов и потолков
    // -----------------------------------------------------------------------------
    static void OtdSlab_FindInRoom (const Stories &storyLevels,
                                    API_Guid &elGuid,
                                    const GS::Array<API_Guid> &zoneGuids,
                                    OtdRooms &roomsinfo,
                                    UnicGUIDByType &guidselementToRead,
                                    RoomSurfaceCache *cache = nullptr);

    static void OtdRoom_GetEdges (const API_ElementMemo &zonememo,
                                  API_Element &zoneelement,
                                  GS::Array<Sector> &walledges,
                                  GS::Array<Sector> &columnedges,
                                  GS::Array<Sector> &restedges,
                                  GS::Array<Sector> &gableedges);

    bool Edge_FindOnEdge (const Sector &edge, const GS::Array<Sector> &edges, Sector &findedge);

    static void Param_AddUnicGUIDByType (const API_Guid &elGuid,
                                         API_ElemTypeID elemtype,
                                         UnicGUIDByType &guidselementToRead);

    API_ElemTypeID Favorite_GetType (const GS::UniString &favorite_name);
    bool Favorite_ReadByName (const GS::UniString &favorite_name, API_Element &element);

    static UnicGuidByBase Otd_GetOtd_Parent (GS::HashTable<API_Guid, TypeOtd> &otd_elements, bool &has_base_element);
    static void Param_GetForBase (ParamDictValue &paramDict, ParamValue &param_composite);
    static void OtdMaterial_SetFinishByComposite (OtdMaterial &material,
                                                  GS::Array<ParamValueComposite> &base_composite);
    static void OtdMaterial_SetFinish (OtdMaterial &material, GS::Array<ParamValueComposite> &base_composite);
    static bool Edge_FindEdge (Sector &edge, GS::Array<Sector> &edges);
    static void Param_ToParamDict (ParamDictValue &paramDict, ReadParams &zoneparams);
    static void Param_Material_Get (GS::HashTable<GS::UniString, GS::Int32> &material_dict, ParamValueData &val);
    static void Favorite_FindName (MatarialToFavorite &favorite,
                                   const OtdMaterial &material,
                                   const TypeOtd &type,
                                   const API_ElemTypeID &draw_type,
                                   ParamDictValue &paramToRead_favorite,
                                   ParamValue &param_composite,
                                   GS::UniString &tip,
                                   MatarialToFavoriteDict &favdict);

    static bool OtdBeam_ProcessFinishes (OtdRoom &room,
                                         ParamDictValue &favoriteParams,
                                         ParamValue &compositeParam,
                                         MatarialToFavoriteDict &favdict);

    RoomEdges *reducededges = nullptr; // Указатель для обработки полигонов зоны

    void OtdLinearSegment_SetZBottom (OtdLinearElement &el, double zBottom) {
        el.seg.begC.z = zBottom;
        el.seg.endC.z = zBottom;
    }

    double OtdLinearSegment_GetZBottom (const OtdLinearElement &el) {
        if (is_equal (el.seg.begC.z, el.seg.endC.z)) {
            return el.seg.begC.z;
        } else {
            return (el.seg.begC.z + el.seg.endC.z) / 2;
        }
    }

    double GetOffsetFromStory (const OtdLinearElement &el, const Stories &stories) {
        const double zBottom = OtdLinearSegment_GetZBottom (el);
        return GetOffsetFromStory (zBottom, el.floorInd, stories);
    }

    bool RoomSurface_Transform (const API_Tranmat &matrix, RoomSurface &surface) {
        if (surface.contours.IsEmpty () || surface.contours[0].hole || surface.contours[0].vertices.GetSize () < 3)
            return false;
        for (double value : matrix.tmx) {
            if (!std::isfinite (value))
                return false;
        }
        const double *m = matrix.tmx;
        const double c00 = m[5] * m[10] - m[6] * m[9];
        const double c01 = m[6] * m[8] - m[4] * m[10];
        const double c02 = m[4] * m[9] - m[5] * m[8];
        const double determinant = m[0] * c00 + m[1] * c01 + m[2] * c02;
        if (!std::isfinite (determinant) || determinant == 0)
            return false;
        const API_Coord3D &n = surface.normalLocal;
        // Нормаль — обратная транспонированная матрица, не преобразование точки/обычного вектора.
        API_Coord3D normal = {(c00 * n.x + c01 * n.y + c02 * n.z) / determinant,
                              ((m[2] * m[9] - m[1] * m[10]) * n.x + (m[0] * m[10] - m[2] * m[8]) * n.y +
                               (m[1] * m[8] - m[0] * m[9]) * n.z) /
                                  determinant,
                              ((m[1] * m[6] - m[2] * m[5]) * n.x + (m[2] * m[4] - m[0] * m[6]) * n.y +
                               (m[0] * m[5] - m[1] * m[4]) * n.z) /
                                  determinant};
        const double length = std::hypot (std::hypot (normal.x, normal.y), normal.z);
        if (!std::isfinite (length) || length == 0)
            return false;
        normal.x /= length;
        normal.y /= length;
        normal.z /= length;
        for (RoomSurfaceContour &contour : surface.contours) {
            if (contour.vertices.GetSize () < 3)
                return false;
            for (RoomSurfaceVertex &vertex : contour.vertices) {
                const API_Coord3D &point = vertex.local;
                vertex.world = {m[0] * point.x + m[1] * point.y + m[2] * point.z + m[3],
                                m[4] * point.x + m[5] * point.y + m[6] * point.z + m[7],
                                m[8] * point.x + m[9] * point.y + m[10] * point.z + m[11]};
                if (!std::isfinite (vertex.world.x) || !std::isfinite (vertex.world.y) ||
                    !std::isfinite (vertex.world.z))
                    return false;
            }
        }
        surface.normalWorld = normal;
        const API_Coord3D &point = surface.contours[0].vertices[0].world;
        surface.planeOffset = -(normal.x * point.x + normal.y * point.y + normal.z * point.z);
        return std::isfinite (surface.planeOffset);
    }

    namespace {

        void RoomSurfaceSnapshot_AddError (RoomSurfaceSnapshot &snapshot, const char *stage, GSErrCode code) {
            RoomSurfaceReadError error;
            error.stage = stage;
            error.code = code;
            snapshot.errors.Push (error);
        }

        struct ComponentEntry {
            API_Component3D component = {};
            GSErrCode error = NoError;
        };

        struct BodyComponentCache {
            std::map<std::pair<API_3DTypeID, Int32>, ComponentEntry> entries;
        };

        bool BodyComponent_Read (BodyComponentCache &cache,
                                 API_3DTypeID type,
                                 Int32 index,
                                 Int32 count,
                                 API_Component3D &component,
                                 RoomSurfaceSnapshot &snapshot,
                                 const char *stage) {
            component = {};
            component.header.typeID = type;
            component.header.index = index;
            if (index < 1 || index > count) {
                RoomSurfaceSnapshot_AddError (snapshot, stage, APIERR_BADINDEX);
                return false;
            }
            const auto key = std::make_pair (type, index);
            const auto found = cache.entries.find (key);
            GSErrCode err;
            if (found != cache.entries.end ()) {
                component = found->second.component;
                err = found->second.error;
            } else {
                err = ACAPI_3D_GetComponent (&component);
                cache.entries.emplace (key, ComponentEntry{component, err});
            }
            // Ошибка сохраняется для каждого не прочитанного компонента, в том числе при попадании в кэш.
            if (err != NoError)
                RoomSurfaceSnapshot_AddError (snapshot, stage, err);
            return err == NoError;
        }

        bool RoomSurface_ReadPolygon (BodyComponentCache &cache,
                                      const API_BodyType &body,
                                      Int32 bodyIndex,
                                      Int32 polygonIndex,
                                      RoomSurface &out,
                                      RoomSurfaceSnapshot &snapshot) {
            API_Component3D component = {};
            if (!BodyComponent_Read (cache, API_PgonID, polygonIndex, body.nPgon, component, snapshot, "polygon"))
                return false;
            const API_PgonType polygon = component.pgon;
            out.bodyIndex = bodyIndex;
            out.polygonIndex = polygonIndex;
    #ifdef ServerMainVers_2700
            out.modelMaterialIndex = ACAPI_CreateAttributeIndex (polygon.iumat);
    #else
            out.modelMaterialIndex = polygon.iumat;
    #endif
            if (polygon.ivect == std::numeric_limits<Int32>::min ()) {
                RoomSurfaceSnapshot_AddError (snapshot, "normal", APIERR_BADINDEX);
                return false;
            }
            if (!BodyComponent_Read (
                    cache, API_VectID, std::abs (polygon.ivect), body.nVect, component, snapshot, "normal"))
                return false;
            const double sign = polygon.ivect < 0 ? -1.0 : 1.0;
            out.normalLocal = {component.vect.x * sign, component.vect.y * sign, component.vect.z * sign};
            if (polygon.fpedg < 1 || polygon.lpedg > body.nPedg || polygon.fpedg > polygon.lpedg) {
                RoomSurfaceSnapshot_AddError (snapshot, "pedgRange", APIERR_BADINDEX);
                return false;
            }
            RoomSurfaceContour contour;
            Int32 firstVertex = 0;
            Int32 previousEnd = 0;
            const auto finishContour = [&] () -> bool {
                if (contour.vertices.GetSize () < 3 || firstVertex != previousEnd) {
                    RoomSurfaceSnapshot_AddError (snapshot, "contourClosure", APIERR_BADPARS);
                    return false;
                }
                contour.hole = !out.contours.IsEmpty ();
                out.contours.Push (contour);
                contour.vertices.Clear ();
                firstVertex = previousEnd = 0;
                return true;
            };
            // Индексы примитивов локальны выбранному телу; нулевой PEDG разделяет контуры отверстий.
            for (Int32 pe = polygon.fpedg; pe <= polygon.lpedg; ++pe) {
                if (!BodyComponent_Read (cache, API_PedgID, pe, body.nPedg, component, snapshot, "pedg"))
                    return false;
                const Int32 signedEdge = component.pedg.pedg;
                if (signedEdge == 0) {
                    if (!finishContour ())
                        return false;
                    continue;
                }
                if (signedEdge == std::numeric_limits<Int32>::min ()) {
                    RoomSurfaceSnapshot_AddError (snapshot, "edge", APIERR_BADINDEX);
                    return false;
                }
                if (!BodyComponent_Read (
                        cache, API_EdgeID, std::abs (signedEdge), body.nEdge, component, snapshot, "edge"))
                    return false;
                const Int32 start = signedEdge > 0 ? component.edge.vert1 : component.edge.vert2;
                const Int32 end = signedEdge > 0 ? component.edge.vert2 : component.edge.vert1;
                if (end < 1 || end > body.nVert || (!contour.vertices.IsEmpty () && start != previousEnd)) {
                    RoomSurfaceSnapshot_AddError (snapshot, "edgeContinuity", APIERR_BADINDEX);
                    return false;
                }
                if (contour.vertices.IsEmpty ())
                    firstVertex = start;
                previousEnd = end;
                if (!BodyComponent_Read (cache, API_VertID, start, body.nVert, component, snapshot, "vertex"))
                    return false;
                RoomSurfaceVertex vertex;
                vertex.local = {component.vert.x, component.vert.y, component.vert.z};
                contour.vertices.Push (vertex);
            }
            if (!finishContour ())
                return false;
            if (!RoomSurface_Transform (body.tranmat, out)) {
                RoomSurfaceSnapshot_AddError (snapshot, "transform", APIERR_BADPARS);
                return false;
            }
            return true;
        }
    } // namespace

    RoomSurfaceSnapshot RoomSurfaces_Read (const API_Elem_Head &head, RoomSurfaceCache *cache) {
        if (cache != nullptr) {
            if (const auto *existing = cache->snapshots.GetPtr (head.guid)) {
                return *existing;
            }
        }
        RoomSurfaceSnapshot snapshot;
        snapshot.guid = head.guid;
        API_ElemInfo3D info = {};
        const GSErrCode err = ACAPI_Element_Get3DInfo (head, &info);
        if (err != NoError) {
            RoomSurfaceSnapshot_AddError (snapshot, "get3DInfo", err);
        } else {
            for (Int32 bi = info.fbody; bi > 0 && bi <= info.lbody; ++bi) {
                API_Component3D component = {};
                component.header.typeID = API_BodyID;
                component.header.index = bi;
                const GSErrCode bodyErr = ACAPI_3D_GetComponent (&component);
                if (bodyErr == APIERR_DELETED) {
                    continue;
                }
                if (bodyErr != NoError) {
                    RoomSurfaceSnapshot_AddError (snapshot, "body", bodyErr);
                    continue;
                }
                const API_BodyType body = component.body;
                RoomSurfaceBody bodyOut;
                bodyOut.index = bi;
                BodyComponentCache components;
                for (Int32 pi = 1; pi <= body.nPgon; ++pi) {
                    RoomSurface surface;
                    if (RoomSurface_ReadPolygon (components, body, bi, pi, surface, snapshot))
                        bodyOut.surfaces.Push (std::move (surface));
                }
                snapshot.bodies.Push (std::move (bodyOut));
            }
        }
        snapshot.status = !snapshot.errors.IsEmpty ()  ? RoomSurfaceStatus::ReadError
                          : snapshot.bodies.IsEmpty () ? RoomSurfaceStatus::Empty
                                                       : RoomSurfaceStatus::Extracted;
        if (cache != nullptr)
            cache->snapshots.Add (head.guid, snapshot);
        return snapshot;
    }

    static ElementRead Element_Read (const API_Guid &guid, RoomSurfaceCache *cache) {
        if (cache != nullptr) {
            if (const auto *found = cache->elements.GetPtr (guid)) {
                return *found;
            }
        }
        ElementRead read;
        read.element.header.guid = guid;
        read.error = ACAPI_Element_Get (&read.element);
        if (cache != nullptr)
            cache->elements.Add (guid, read);
        return read;
    }

    static ClassificationRead Class_ReadItems (const API_Guid &guid, RoomSurfaceCache *cache) {
        if (cache != nullptr) {
            if (const auto *found = cache->classifications.GetPtr (guid)) {
                return *found;
            }
        }
        ClassificationRead read;
        read.error = ACAPI_Element_GetClassificationItems (guid, read.items);
        if (cache != nullptr)
            cache->classifications.Add (guid, read);
        return read;
    }

    static GSErrCode Zone_ReadRelations (const API_Guid &zoneGuid,
                                         RoomSurfaceCache *cache,
                                         RoomRelationValues &values) {
        if (cache != nullptr) {
            if (const auto *found = cache->relations.GetPtr (zoneGuid)) {
                values = *found;
                return NoError;
            }
        }
        API_RoomRelation relations = {};
        const GSErrCode error = ACAPI_Element_GetRelations (zoneGuid, API_ZombieElemID, &relations);
        if (error != NoError) {
            ACAPI_DisposeRoomRelationHdls (&relations);
            return error;
        }
        // Только value-поля: владеющие хендлы API_RoomRelation живут в niches, которые не читаем.
        values.elementsGroupedByType = relations.elementsGroupedByType;
        values.wallPart = relations.wallPart;
        values.beamPart = relations.beamPart;
        ACAPI_DisposeRoomRelationHdls (&relations);
        if (cache != nullptr)
            cache->relations.Add (zoneGuid, values);
        return NoError;
    }

    ConnectedElementsRead ConnectedElements_Read (const API_Guid &guid,
                                                  API_ElemTypeID type,
                                                  API_ElemFilterFlags filter,
                                                  RoomSurfaceCache *cache,
                                                  API_ElemVariationID variation,
                                                  const API_Guid &renovationFilterGuid) {
        return ReadConnectedElementsCached (
            guid, type, filter, variation, renovationFilterGuid, cache, [&] (GS::Array<API_Guid> &items) {
    #ifdef ServerMainVers_2700
                return ACAPI_Grouping_GetConnectedElements (
                    guid, API_ElemType (type, variation), &items, filter, renovationFilterGuid);
    #elif defined(ServerMainVers_2600)
                return ACAPI_Element_GetConnectedElements (
                    guid, API_ElemType (type, variation), &items, filter, renovationFilterGuid);
    #else
                return ACAPI_Element_GetConnectedElements (guid, type, &items, filter, variation, renovationFilterGuid);
    #endif
            });
    }

    // Сбор оснований зоны и проверка полноты исходных данных.
    namespace {
        void RoomSurfaceCandidates_AddError (RoomSurfaceCandidates &result, const char *stage, GSErrCode code) {
            result.errors.Push ({GS::UniString (stage), code});
        }

        bool Element_IsFinish (const API_Guid &guid,
                               const GS::Array<API_Guid> &finishClassGuids,
                               RoomSurfaceCandidates &result,
                               bool &valid,
                               RoomSurfaceCache *cache) {
            const ClassificationRead read = Class_ReadItems (guid, cache);
            valid = read.error == NoError;
            if (!valid) {
                RoomSurfaceCandidates_AddError (result, "candidateClassification", read.error);
                return false;
            }
            // Как Class_IsElementFinClass, но ошибка чтения не означает «не отделка».
            for (const auto &item : read.items) {
                if (finishClassGuids.Contains (item.second)) {
                    return true;
                }
            }
            return false;
        }

        void RoomSurfaceCandidate_Read (const API_Elem_Head &base,
                                        const API_Elem_Head &geometry,
                                        const GS::Array<API_Guid> &finishClassGuids,
                                        RoomSurfaceCandidates &result,
                                        RoomSurfaceCache *cache) {
            if (geometry.guid != base.guid) {
                bool valid = false;
                if (Element_IsFinish (geometry.guid, finishClassGuids, result, valid, cache) || !valid)
                    return;
            }
            for (const auto &source : result.sources) {
                if (source.baseGuid == base.guid && source.geometryGuid == geometry.guid)
                    return;
            }
            RoomSurfaceSource source;
            source.baseGuid = base.guid;
            source.baseType = GetElemTypeID (base);
            source.geometryGuid = geometry.guid;
            source.snapshot = RoomSurfaces_Read (geometry, cache);
            if (!source.snapshot.IsComplete ())
                RoomSurfaceCandidates_AddError (result, "candidateSurfaceSnapshot", APIERR_GENERAL);
            result.sources.Push (std::move (source));
        }

        template <class Segment>
        void ElementSegments_Read (const API_Elem_Head &base,
                                   const Segment *segments,
                                   UInt32 count,
                                   API_ElemTypeID expectedType,
                                   const GS::Array<API_Guid> &finishClassGuids,
                                   RoomSurfaceCandidates &result,
                                   RoomSurfaceCache *cache) {
            const GSSize bytes =
                segments != nullptr ? BMGetPtrSize (reinterpret_cast<GSPtr> (const_cast<Segment *> (segments))) : 0;
            if (count == 0 || bytes <= 0 || bytes % static_cast<GSSize> (sizeof (Segment)) != 0 ||
                static_cast<UInt64> (bytes) / sizeof (Segment) != count) {
                RoomSurfaceCandidates_AddError (result, "candidateSegmentCount", APIERR_BADPARS);
                return;
            }
            GS::Array<API_Guid> seen;
            for (UInt32 i = 0; i < count; ++i) {
                const auto &segment = segments[i];
                if (segment.head.guid == APINULLGuid || segment.owner != base.guid ||
                    GetElemTypeID (segment.head) != expectedType || seen.Contains (segment.head.guid)) {
                    RoomSurfaceCandidates_AddError (result, "candidateSegmentHeader", APIERR_BADPARS);
                    continue;
                }
                seen.Push (segment.head.guid);
                RoomSurfaceCandidate_Read (base, segment.head, finishClassGuids, result, cache);
            }
        }

        void BaseElement_Read (API_Guid guid,
                               API_ElemTypeID type,
                               const GS::Array<API_Guid> &finishClassGuids,
                               RoomSurfaceCandidates &result,
                               RoomSurfaceCache *cache) {
            const ElementRead read = Element_Read (guid, cache);
            if (read.error != NoError || GetElemTypeID (read.element.header) != type) {
                RoomSurfaceCandidates_AddError (
                    result, "candidateElement", read.error != NoError ? read.error : APIERR_BADID);
                return;
            }
            bool valid = false;
            if (Element_IsFinish (guid, finishClassGuids, result, valid, cache) || !valid)
                return;
            if (type == API_WallID || type == API_SlabID) {
                // Wall/Slab готовятся после проёмов и legacy-контактов; discovery не читает их 3D.
                RoomSurfaceSource source;
                source.baseGuid = guid;
                source.baseType = type;
                source.geometryGuid = guid;
                result.sources.Push (std::move (source));
                return;
            }
            API_ElementMemo memo = {};
            const GS::OnExit memoGuard ([&memo] () { ACAPI_DisposeElemMemoHdls (&memo); });

            const GSErrCode memoError = ACAPI_Element_GetMemo (
                guid, &memo, type == API_ColumnID ? APIMemoMask_ColumnSegment : APIMemoMask_BeamSegment);
            if (memoError != NoError) {
                RoomSurfaceCandidates_AddError (result, "candidateSegmentMemo", memoError);
                return;
            }
            if (type == API_ColumnID)
                ElementSegments_Read (read.element.header,
                                      memo.columnSegments,
                                      read.element.column.nSegments,
                                      API_ColumnSegmentID,
                                      finishClassGuids,
                                      result,
                                      cache);
            else
                ElementSegments_Read (read.element.header,
                                      memo.beamSegments,
                                      read.element.beam.nSegments,
                                      API_BeamSegmentID,
                                      finishClassGuids,
                                      result,
                                      cache);
        }
    } // namespace

    static RoomSurfaceCandidates RoomSurfaceCandidates_Read (const API_Elem_Head &zone,
                                                             const GS::Array<API_Guid> &finishClassGuids,
                                                             RoomSurfaceCache *cache) {
        RoomSurfaceCandidates result;
        result.zoneGuid = zone.guid;
        if (zone.guid == APINULLGuid || GetElemTypeID (zone) != API_ZoneID) {
            RoomSurfaceCandidates_AddError (result, "candidateZoneHeader", APIERR_BADID);
            return result;
        }
        RoomRelationValues relData;
        const GSErrCode error = Zone_ReadRelations (zone.guid, cache, relData);
        if (error != NoError) {
            RoomSurfaceCandidates_AddError (result, "candidateRoomRelations", error);
            return result;
        }
        for (const API_BeamPart &part : relData.beamPart)
            result.beamParts.Push (part);
        GS::Array<API_Guid> seen;
        for (API_ElemTypeID type : {API_WallID, API_ColumnID, API_SlabID, API_BeamID}) {
            GS::Array<API_Guid> guids;
            if (const auto *group = relData.elementsGroupedByType.GetPtr (type))
                guids = *group;
            if (type == API_BeamID) {
                for (const auto &part : relData.beamPart)
                    if (!guids.Contains (part.guid))
                        guids.Push (part.guid);
            }
            for (const API_Guid &guid : guids) {
                if (guid == APINULLGuid) {
                    RoomSurfaceCandidates_AddError (result, "candidateNullGuid", APIERR_BADPARS);
                    continue;
                }
                if (seen.Contains (guid))
                    continue;
                seen.Push (guid);
                BaseElement_Read (guid, type, finishClassGuids, result, cache);
            }
        }
        result.complete = result.errors.IsEmpty ();
        return result;
    }

    // Конечные контакты зоны с основаниями и балками.
    namespace {
        using Polygons = Geometry::MultiPolygon2D;

        API_Coord3D Coord3D_Subtract (const API_Coord3D &a, const API_Coord3D &b) {
            return {a.x - b.x, a.y - b.y, a.z - b.z};
        }

        double Coord3D_Dot (const API_Coord3D &a, const API_Coord3D &b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

        API_Coord3D Coord3D_Cross (const API_Coord3D &a, const API_Coord3D &b) {
            return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
        }

        bool Coord3D_IsFinite (const API_Coord3D &v) {
            return std::isfinite (v.x) && std::isfinite (v.y) && std::isfinite (v.z);
        }

        struct Frame {
            API_Coord3D origin = {};
            API_Coord3D normal = {};
            API_Coord3D u = {};
            API_Coord3D v = {};

            bool Frame_Set (const RoomSurface &surface) {
                normal = surface.normalWorld;
                const double length = std::sqrt (Coord3D_Dot (normal, normal));
                if (!Coord3D_IsFinite (normal) || !std::isfinite (surface.planeOffset) ||
                    std::fabs (length - 1.0) > DistanceTolerance || surface.contours.IsEmpty () ||
                    surface.contours[0].hole || surface.contours[0].vertices.IsEmpty ())
                    return false;
                normal = {normal.x / length, normal.y / length, normal.z / length};
                origin = surface.contours[0].vertices[0].world;
                API_Coord3D axis = {1, 0, 0};
                if (std::fabs (normal.y) < std::fabs (normal.x))
                    axis = {0, 1, 0};
                if (std::fabs (normal.z) < std::fabs (Coord3D_Dot (normal, axis)))
                    axis = {0, 0, 1};
                u = Coord3D_Cross (axis, normal);
                const double uLength = std::sqrt (Coord3D_Dot (u, u));
                u = {u.x / uLength, u.y / uLength, u.z / uLength};
                v = Coord3D_Cross (normal, u);
                return Coord3D_IsFinite (origin) && Coord3D_IsFinite (u) && Coord3D_IsFinite (v) &&
                       std::fabs (Coord3D_Dot (surface.normalWorld, origin) + surface.planeOffset) <= PlaneTolerance;
            }

            bool Frame_SetFromVertices (const RoomSurface &surface) {
                if (!Frame_Set (surface))
                    return false;
                // Согласованную исходную плоскость сохраняем, включая малые грани.
                bool onStoredPlane = true;
                for (const auto &contour : surface.contours)
                    for (const auto &vertex : contour.vertices) {
                        if (!Coord3D_IsFinite (vertex.world))
                            return false;
                        onStoredPlane =
                            onStoredPlane &&
                            std::fabs (Coord3D_Dot (normal, Coord3D_Subtract (vertex.world, origin))) <= PlaneTolerance;
                    }
                if (onStoredPlane)
                    return true;
                // При расхождении нормали с плоским контуром считаем вектор площади относительно
                // первой вершины: абсолютные координаты не должны ухудшать точность малых граней.
                API_Coord3D newell = {};
                const auto &outer = surface.contours[0].vertices;
                const USize count = outer.GetSize ();
                for (UIndex i = 0; i < count; ++i) {
                    const API_Coord3D a = Coord3D_Subtract (outer[i].world, origin);
                    const API_Coord3D b = Coord3D_Subtract (outer[(i + 1) % count].world, origin);
                    if (!Coord3D_IsFinite (a) || !Coord3D_IsFinite (b))
                        return false;
                    newell.x += a.y * b.z - a.z * b.y;
                    newell.y += a.z * b.x - a.x * b.z;
                    newell.z += a.x * b.y - a.y * b.x;
                }
                const double length = std::sqrt (Coord3D_Dot (newell, newell));
                if (!Coord3D_IsFinite (newell) || !std::isfinite (length) || length == 0)
                    return false;
                normal = {newell.x / length, newell.y / length, newell.z / length};
                API_Coord3D axis = {1, 0, 0};
                if (std::fabs (normal.y) < std::fabs (normal.x))
                    axis = {0, 1, 0};
                if (std::fabs (normal.z) < std::fabs (Coord3D_Dot (normal, axis)))
                    axis = {0, 0, 1};
                u = Coord3D_Cross (axis, normal);
                const double uLength = std::sqrt (Coord3D_Dot (u, u));
                u = {u.x / uLength, u.y / uLength, u.z / uLength};
                v = Coord3D_Cross (normal, u);
                return Coord3D_IsFinite (u) && Coord3D_IsFinite (v);
            }

            Point2D Frame_Project (const API_Coord3D &point) const {
                const API_Coord3D delta = Coord3D_Subtract (point, origin);
                return {Coord3D_Dot (delta, u), Coord3D_Dot (delta, v)};
            }

            API_Coord3D Frame_Restore (const Point2D &point) const {
                return {origin.x + u.x * point.x + v.x * point.y,
                        origin.y + u.y * point.x + v.y * point.y,
                        origin.z + u.z * point.x + v.z * point.y};
            }
        };

        bool RoomSurface_IsOnPlane (const RoomSurface &surface, const Frame &frame) {
            for (const auto &contour : surface.contours) {
                for (const auto &vertex : contour.vertices) {
                    if (!Coord3D_IsFinite (vertex.world) ||
                        std::fabs (Coord3D_Dot (frame.normal, Coord3D_Subtract (vertex.world, frame.origin))) >
                            PlaneTolerance)
                        return false;
                }
            }
            return true;
        }

        bool RoomSurface_Project (const RoomSurface &surface, const Frame &frame, Polygons &result) {
            GS::Array<Point2D> coords;
            GS::Array<UIndex> ends;
            GS::Array<PolyArcRec> arcs;
            double expectedArea = 0;
            if (surface.contours.IsEmpty () || surface.contours[0].hole || !RoomSurface_IsOnPlane (surface, frame))
                return false;
            for (UIndex c = 0; c < surface.contours.GetSize (); ++c) {
                const auto &contour = surface.contours[c];
                if (contour.vertices.GetSize () < 3 || contour.hole != (c != 0))
                    return false;
                const UIndex start = coords.GetSize ();
                double twiceArea = 0;
                for (UIndex i = 0; i < contour.vertices.GetSize (); ++i) {
                    const Point2D point = frame.Frame_Project (contour.vertices[i].world);
                    const Point2D next =
                        frame.Frame_Project (contour.vertices[(i + 1) % contour.vertices.GetSize ()].world);
                    twiceArea += point.x * next.y - next.x * point.y;
                    coords.Push (point);
                }
                if (!std::isfinite (twiceArea) || twiceArea == 0)
                    return false;
                expectedArea += (contour.hole ? -1 : 1) * std::fabs (twiceArea) / 2;
                if (coords.GetSize () - start < 3)
                    return false;
                ends.Push (coords.GetSize () - 1);
            }
            result =
                Geometry::Polygon2D::Create (coords, arcs, ends, Geometry::PolyCreateFlags_IdxRefInContourEndFromZero);
            return expectedArea > 0 && !result.IsEmpty () &&
                   std::fabs (result.CalcArea () - expectedArea) <= AreaTolerance;
        }

        struct Claim {
            RoomSurfaceSourceFace face;
            const RoomSurface *surface = nullptr;
            std::string baseKey;
            std::string geometryKey;
        };

        bool RoomSurface_AreSame (const RoomSurface &a, const RoomSurface &b) {
            if (a.modelMaterialIndex != b.modelMaterialIndex || a.planeOffset != b.planeOffset ||
                a.normalWorld.x != b.normalWorld.x || a.normalWorld.y != b.normalWorld.y ||
                a.normalWorld.z != b.normalWorld.z || a.contours.GetSize () != b.contours.GetSize ())
                return false;
            for (UIndex c = 0; c < a.contours.GetSize (); ++c) {
                const auto &ac = a.contours[c];
                const auto &bc = b.contours[c];
                if (ac.hole != bc.hole || ac.vertices.GetSize () != bc.vertices.GetSize ())
                    return false;
                for (UIndex i = 0; i < ac.vertices.GetSize (); ++i) {
                    const auto &av = ac.vertices[i].world;
                    const auto &bv = bc.vertices[i].world;
                    if (av.x != bv.x || av.y != bv.y || av.z != bv.z)
                        return false;
                }
            }
            return true;
        }

        bool Claim_AreSame (const Claim &a, const Claim &b) {
            return a.face.baseGuid == b.face.baseGuid && a.face.baseType == b.face.baseType &&
                   a.face.geometryGuid == b.face.geometryGuid && a.face.bodyIndex == b.face.bodyIndex &&
                   a.face.polygonIndex == b.face.polygonIndex && a.face.origin == b.face.origin &&
                   a.face.analyticFaceId == b.face.analyticFaceId &&
                   a.face.preparedWallIndex == b.face.preparedWallIndex;
        }

        struct Cell {
            Geometry::Polygon2D polygon;
            std::vector<UIndex> claims;
        };

        void Cells_Append (const Polygons &polygons, const std::vector<UIndex> &claims, std::vector<Cell> &cells) {
            for (const auto &polygon : polygons) {
                if (polygon.CalcArea () > 0)
                    cells.push_back ({polygon, claims});
            }
        }

        void RoomSurfacePartition_AddError (RoomSurfacePartition &result, const char *stage) {
            RoomSurfaceReadError error;
            error.stage = stage;
            error.code = APIERR_BADPARS;
            result.errors.Push (error);
        }

        RoomSurfacePiece RoomSurfacePiece_Create (const Cell &cell,
                                                  const Frame &frame,
                                                  const RoomSurface &zoneFace,
                                                  const std::vector<Claim> &claims) {
            RoomSurfacePiece piece;
            piece.zoneBodyIndex = zoneFace.bodyIndex;
            piece.zonePolygonIndex = zoneFace.polygonIndex;
            piece.normal = zoneFace.normalWorld;
            piece.planeOffset = zoneFace.planeOffset;
            piece.area = cell.polygon.CalcArea ();
            for (UIndex index : cell.claims)
                piece.sources.Push (claims[index].face);
            if (!piece.sources.IsEmpty ()) {
                const auto &first = piece.sources[0];
                piece.status = RoomSurfacePieceStatus::Matched;
                for (const auto &source : piece.sources) {
                    if (source.baseGuid != first.baseGuid || source.baseType != first.baseType) {
                        piece.status = RoomSurfacePieceStatus::Ambiguous;
                        break;
                    }
                }
                if (piece.status == RoomSurfacePieceStatus::Matched) {
                    piece.baseGuid = first.baseGuid;
                    piece.baseType = first.baseType;
                }
            }
            GS::Array<Point2D> coords;
            GS::Array<PolyArcRec> arcs;
            GS::Array<UIndex> ends;
            GS::Array<Geometry::Polygon2D::ConstContourIterator> reversed;
            cell.polygon.CopyPolygonData (
                coords, arcs, ends, reversed, Geometry::PolyCreateFlags_IdxRefInContourEndFromZero);
            UIndex start = 0;
            for (UIndex c = 0; c < ends.GetSize (); ++c) {
                RoomSurfacePieceContour contour;
                contour.hole = c != 0;
                for (UIndex i = start; i <= ends[c]; ++i)
                    contour.vertices.Push (frame.Frame_Restore (coords[i]));
                piece.contours.Push (contour);
                start = ends[c] + 1;
            }
            return piece;
        }

        bool RoomSurfaceFace_Partition (const RoomSurface &zoneFace,
                                        const std::vector<Claim> &claims,
                                        RoomSurfacePartition &result) {
            Frame frame;
            Polygons zonePolygon;
            if (!frame.Frame_Set (zoneFace) || !RoomSurface_Project (zoneFace, frame, zonePolygon)) {
                RoomSurfacePartition_AddError (result, "PartitionZoneFace");
                return false;
            }
            std::vector<Cell> cells;
            Cells_Append (zonePolygon, {}, cells);
            for (UIndex index = 0; index < claims.size (); ++index) {
                const auto &surface = *claims[index].surface;
                // Контакт — конечная грань конструкции, копланарная грани зоны с перекрытием площадей.
                // Ориентация нормалей не проверяется: балка, врезанная в комнату заподлицо с потолком/полом,
                // даёт грань с той же ориентацией, что у грани зоны, и обязана покрывать её участок.
                if (!RoomSurface_IsOnPlane (surface, frame))
                    continue;
                Frame sourceFrame;
                if (!sourceFrame.Frame_SetFromVertices (surface) || !RoomSurface_IsOnPlane (zoneFace, sourceFrame))
                    continue;
                Polygons sourcePolygon;
                if (!RoomSurface_Project (surface, frame, sourcePolygon)) {
                    RoomSurfacePartition_AddError (result, "PartitionSourceFace");
                    return false;
                }
                std::vector<Cell> next;
                for (const auto &cell : cells) {
                    Polygons current;
                    current.Push (cell.polygon);
                    Cells_Append (Polygons::Substract (current, sourcePolygon), cell.claims, next);
                    auto owners = cell.claims;
                    owners.push_back (index);
                    Cells_Append (Polygons::Intersect (current, sourcePolygon), owners, next);
                }
                cells = std::move (next);
            }
            Polygons coverage;
            for (const auto &cell : cells)
                coverage.Push (cell.polygon);
            coverage.Unify ();
            // Симметрические разности в обе стороны доказывают равенство покрытия зоне; сумма площадей
            // чувствительна к шуму Boolean-движка (замеры: 1.8e-6 при площади 7.7, 2.3e-6 при 182.6), поэтому
            // сравнение площадей убрано, а допуски разностей масштабируются площадью. Точечные допуски 1e-6
            // не меняются; реальная потеря участка при таком допуске — доли мм².
            const double zoneArea = zonePolygon.CalcArea ();
            const double areaTolerance = AreaTolerance * (1.0 + 0.05 * zoneArea);
            const double zoneMinusCoverage = Polygons::Substract (zonePolygon, coverage).CalcArea ();
            const double coverageMinusZone = Polygons::Substract (coverage, zonePolygon).CalcArea ();
            const char *stage = nullptr;
            if (zoneMinusCoverage > areaTolerance)
                stage = "PartitionCoverageZoneMinusCoverage";
            else if (coverageMinusZone > areaTolerance)
                stage = "PartitionCoverageCoverageMinusZone";
            if (stage != nullptr) {
                RoomSurfacePartition_AddError (result, stage);
                return false;
            }
            for (const auto &cell : cells)
                result.pieces.Push (RoomSurfacePiece_Create (cell, frame, zoneFace, claims));
            return true;
        }
    } // namespace

    RoomSurfacePartition RoomSurfaces_Partition (const RoomSurfaceSnapshot &zone,
                                                 const GS::Array<RoomSurfaceSource> &sources) {
        RoomSurfacePartition result;
        result.zoneGuid = zone.guid;
        if (!zone.IsComplete () || zone.status == RoomSurfaceStatus::Empty) {
            RoomSurfacePartition_AddError (result, "PartitionZoneSnapshot");
            return result;
        }
        std::vector<Claim> claims;
        for (UIndex i = 0; i < sources.GetSize (); ++i) {
            const auto &source = sources[i];
            if (source.baseGuid == APINULLGuid || source.geometryGuid == APINULLGuid ||
                source.snapshot.guid != source.geometryGuid || !source.snapshot.IsComplete ()) {
                RoomSurfacePartition_AddError (result, "PartitionSourceSnapshot");
                return result;
            }
            const std::string baseKey = APIGuidToString (source.baseGuid).ToCStr ().Get ();
            const std::string geometryKey = APIGuidToString (source.geometryGuid).ToCStr ().Get ();
            for (const auto &body : source.snapshot.bodies) {
                for (const auto &surface : body.surfaces) {
                    Frame sourceFrame;
                    if (!sourceFrame.Frame_SetFromVertices (surface) || !RoomSurface_IsOnPlane (surface, sourceFrame)) {
                        RoomSurfacePartition_AddError (result, "PartitionInvalidSourcePlane");
                        return result;
                    }
                    Claim claim;
                    claim.face = {source.baseGuid,
                                  source.baseType,
                                  source.geometryGuid,
                                  body.index,
                                  surface.polygonIndex,
                                  surface.modelMaterialIndex};
                    claim.face.origin = surface.origin;
                    claim.face.analyticFaceId = surface.analyticFaceId;
                    claim.face.preparedWallIndex = surface.preparedWallIndex;
                    claim.surface = &surface;
                    claim.baseKey = baseKey;
                    claim.geometryKey = geometryKey;
                    claims.push_back (claim);
                }
            }
        }
        std::sort (claims.begin (), claims.end (), [] (const Claim &a, const Claim &b) {
            if (a.baseKey != b.baseKey)
                return a.baseKey < b.baseKey;
            if (a.face.baseType != b.face.baseType)
                return a.face.baseType < b.face.baseType;
            if (a.geometryKey != b.geometryKey)
                return a.geometryKey < b.geometryKey;
            if (a.face.origin != b.face.origin)
                return a.face.origin < b.face.origin;
            if (a.face.analyticFaceId != b.face.analyticFaceId)
                return a.face.analyticFaceId < b.face.analyticFaceId;
            if (a.face.preparedWallIndex != b.face.preparedWallIndex)
                return a.face.preparedWallIndex < b.face.preparedWallIndex;
            if (a.face.bodyIndex != b.face.bodyIndex)
                return a.face.bodyIndex < b.face.bodyIndex;
            return a.face.polygonIndex < b.face.polygonIndex;
        });
        // Одинаковый ключ с разной геометрией — ошибка снимков, не выбор первого кандидата.
        for (UIndex i = 1; i < claims.size (); ++i) {
            if (Claim_AreSame (claims[i - 1], claims[i]) &&
                !RoomSurface_AreSame (*claims[i - 1].surface, *claims[i].surface)) {
                RoomSurfacePartition_AddError (result, "PartitionConflictingSourceFace");
                return result;
            }
        }
        claims.erase (std::unique (claims.begin (), claims.end (), Claim_AreSame), claims.end ());
        try {
            for (const auto &body : zone.bodies) {
                for (const auto &surface : body.surfaces) {
                    if (!RoomSurfaceFace_Partition (surface, claims, result))
                        return result;
                }
            }
            result.complete = !result.pieces.IsEmpty ();
        } catch (const GS::Exception &) {
            RoomSurfacePartition_AddError (result, "PartitionGeometryException");
        }
        return result;
    }

    namespace {
        // Параметр точки на оси балки: t = проекция (p - beg) на (end - beg), в единицах длины оси.
        struct BeamAxis {
            API_Coord origin = {};
            API_Coord dir = {};
            double length = 0;
            API_Coord perp = {}; // единичный перпендикуляр оси в плане

            bool BeamAxis_Set (const API_Coord &beg, const API_Coord &end) {
                dir = {end.x - beg.x, end.y - beg.y};
                length = std::sqrt (dir.x * dir.x + dir.y * dir.y);
                if (!std::isfinite (length) || length <= DistanceTolerance || !std::isfinite (beg.x) ||
                    !std::isfinite (beg.y) || !std::isfinite (end.x) || !std::isfinite (end.y))
                    return false;
                dir = {dir.x / length, dir.y / length};
                perp = {-dir.y, dir.x};
                origin = beg;
                return true;
            }

            double BeamAxis_GetAlong (const API_Coord3D &p) const {
                return (p.x - origin.x) * dir.x + (p.y - origin.y) * dir.y;
            }

            double BeamAxis_GetOffset (const API_Coord3D &p) const {
                return (p.x - origin.x) * perp.x + (p.y - origin.y) * perp.y;
            }

            API_Coord3D BeamAxis_GetPoint (double t, double offset, double z) const {
                return {origin.x + dir.x * t + perp.x * offset, origin.y + dir.y * t + perp.y * offset, z};
            }
        };

        struct SectionEdge {
            Point2D beg;
            Point2D end;
            bool used = false;
        };

        bool Point2D_AreSame (const Point2D &a, const Point2D &b) {
            return std::hypot (a.x - b.x, a.y - b.y) <= DistanceTolerance;
        }

        // Сечение замкнутого тела, не силуэт проекции: направление рёбер сохраняет отверстия.
        bool Zone_BuildSection (const RoomSurfaceSnapshot &zone, const Frame &frame, Polygons &section) {
            if (!zone.IsComplete ())
                return false;
            for (const auto &body : zone.bodies) {
                std::vector<SectionEdge> edges;
                Polygons outer, holes;
                for (const auto &surface : body.surfaces) {
                    const API_Coord3D direction = Coord3D_Cross (frame.normal, surface.normalWorld);
                    if (Coord3D_Dot (direction, direction) <= DistanceTolerance * DistanceTolerance) {
                        if (RoomSurface_IsOnPlane (surface, frame)) {
                            Polygons boundary;
                            if (!RoomSurface_Project (surface, frame, boundary))
                                return false;
                            for (const auto &polygon : boundary)
                                outer.Push (polygon);
                        }
                        continue;
                    }
                    std::vector<API_Coord3D> hits;
                    for (const auto &contour : surface.contours) {
                        for (UIndex i = 0; i < contour.vertices.GetSize (); ++i) {
                            const auto &a = contour.vertices[i].world;
                            const auto &b = contour.vertices[(i + 1) % contour.vertices.GetSize ()].world;
                            if (!Coord3D_IsFinite (a) || !Coord3D_IsFinite (b))
                                return false;
                            double da = Coord3D_Dot (frame.normal, Coord3D_Subtract (a, frame.origin));
                            double db = Coord3D_Dot (frame.normal, Coord3D_Subtract (b, frame.origin));
                            if (std::fabs (da) <= DistanceTolerance)
                                da = 0;
                            if (std::fabs (db) <= DistanceTolerance)
                                db = 0;
                            // Полуоткрытое правило: общая вершина учитывается ровно один раз.
                            if (!((da <= 0 && db > 0) || (db <= 0 && da > 0)))
                                continue;
                            const double t = da / (da - db);
                            hits.push_back ({a.x + t * (b.x - a.x), a.y + t * (b.y - a.y), a.z + t * (b.z - a.z)});
                        }
                    }
                    std::sort (hits.begin (), hits.end (), [&] (const auto &a, const auto &b) {
                        return Coord3D_Dot (a, direction) < Coord3D_Dot (b, direction);
                    });
                    if (hits.size () % 2 != 0)
                        return false;
                    for (UIndex i = 0; i < hits.size (); i += 2) {
                        const Point2D a = frame.Frame_Project (hits[i]);
                        const Point2D b = frame.Frame_Project (hits[i + 1]);
                        if (Point2D_AreSame (a, b))
                            continue;
                        bool duplicate = false;
                        for (const auto &edge : edges)
                            duplicate = duplicate || (Point2D_AreSame (edge.beg, a) && Point2D_AreSame (edge.end, b));
                        if (!duplicate)
                            edges.push_back ({a, b, false});
                    }
                }
                for (auto &first : edges) {
                    if (first.used)
                        continue;
                    GS::Array<Point2D> coords;
                    first.used = true;
                    coords.Push (first.beg);
                    Point2D end = first.end;
                    while (!Point2D_AreSame (end, coords[0])) {
                        coords.Push (end);
                        SectionEdge *next = nullptr;
                        for (auto &edge : edges) {
                            if (!edge.used && Point2D_AreSame (edge.beg, end)) {
                                // Незамкнутый/разветвлённый контур не заменяем bounding-прямоугольником.
                                if (next != nullptr)
                                    return false;
                                next = &edge;
                            }
                        }
                        if (next == nullptr)
                            return false;
                        next->used = true;
                        end = next->end;
                    }
                    if (coords.GetSize () < 3)
                        return false;
                    double twiceArea = 0;
                    for (UIndex i = 0; i < coords.GetSize (); ++i) {
                        const auto &a = coords[i];
                        const auto &b = coords[(i + 1) % coords.GetSize ()];
                        twiceArea += a.x * b.y - b.x * a.y;
                    }
                    GS::Array<UIndex> ends;
                    GS::Array<PolyArcRec> arcs;
                    ends.Push (coords.GetSize () - 1);
                    const auto polygons = Geometry::Polygon2D::Create (
                        coords, arcs, ends, Geometry::PolyCreateFlags_IdxRefInContourEndFromZero);
                    for (const auto &polygon : polygons)
                        (twiceArea > 0 ? outer : holes).Push (polygon);
                }
                outer.Unify ();
                holes.Unify ();
                const auto bodySection = Polygons::Substract (outer, holes);
                for (const auto &polygon : bodySection)
                    section.Push (polygon);
            }
            section.Unify ();
            return true;
        }

        bool BeamFace_ClipToZone (const RoomSurface &surface,
                                  const RoomSurfaceSnapshot &zone,
                                  const BeamAxis &axis,
                                  double tBeg,
                                  double tEnd,
                                  double zBottom,
                                  double zTop,
                                  RoomSurfaceBeamFace face,
                                  bool wholeZoneContact,
                                  GS::Array<RoomSurfaceBeamPiece> &pieces) {
            Frame frame;
            Polygons original, section;
            if (!frame.Frame_Set (surface) || !RoomSurface_Project (surface, frame, original) ||
                !Zone_BuildSection (zone, frame, section))
                return false;
            if (face != RoomSurfaceBeamFace::Bottom && face != RoomSurfaceBeamFace::Top) {
                double tMin = axis.BeamAxis_GetAlong (surface.contours[0].vertices[0].world);
                double tMax = tMin;
                for (const auto &contour : surface.contours)
                    for (const auto &vertex : contour.vertices) {
                        tMin = std::min (tMin, axis.BeamAxis_GetAlong (vertex.world));
                        tMax = std::max (tMax, axis.BeamAxis_GetAlong (vertex.world));
                    }
                const double offset = axis.BeamAxis_GetOffset (frame.origin);
                GS::Array<Point2D> coords;
                coords.Push (frame.Frame_Project (axis.BeamAxis_GetPoint (tMin, offset, zBottom)));
                coords.Push (frame.Frame_Project (axis.BeamAxis_GetPoint (tMax, offset, zBottom)));
                coords.Push (frame.Frame_Project (axis.BeamAxis_GetPoint (tMax, offset, zTop)));
                coords.Push (frame.Frame_Project (axis.BeamAxis_GetPoint (tMin, offset, zTop)));
                GS::Array<UIndex> ends;
                GS::Array<PolyArcRec> arcs;
                ends.Push (3);
                const auto verticalRange = Geometry::Polygon2D::Create (
                    coords, arcs, ends, Geometry::PolyCreateFlags_IdxRefInContourEndFromZero);
                original = Polygons::Intersect (original, verticalRange);
            }
            // Сечение зоны задаёт фактические границы контакта на каждой высоте, не нижний контур.
            const auto clipped = Polygons::Intersect (original, section);
            GS::Array<RoomSurfaceBeamPiece> own;
            for (const auto &polygon : clipped) {
                GS::Array<Point2D> coords;
                GS::Array<PolyArcRec> arcs;
                GS::Array<UIndex> ends;
                GS::Array<Geometry::Polygon2D::ConstContourIterator> reversed;
                polygon.CopyPolygonData (
                    coords, arcs, ends, reversed, Geometry::PolyCreateFlags_IdxRefInContourEndFromZero);
                // Представление beamPiece пока не хранит отверстия: явный атомарный отказ.
                if (ends.GetSize () != 1 || !arcs.IsEmpty ())
                    return false;
                RoomSurfaceBeamPiece piece;
                piece.face = face;
                piece.area = polygon.CalcArea ();
                double tMin = 0, tMax = 0;
                for (UIndex i = 0; i <= ends[0]; ++i) {
                    const auto point = frame.Frame_Restore (coords[i]);
                    if (point.z < zBottom - DistanceTolerance || point.z > zTop + DistanceTolerance)
                        return false;
                    const double t = axis.BeamAxis_GetAlong (point);
                    if (i == 0)
                        tMin = tMax = t;
                    else {
                        tMin = std::min (tMin, t);
                        tMax = std::max (tMax, t);
                    }
                    piece.vertices.Push (point);
                }
                // beamPart выбирает связную часть; её реальные границы задаёт 3D-зона на высоте грани.
                if ((wholeZoneContact || std::min (tMax, tEnd) - std::max (tMin, tBeg) > DistanceTolerance) &&
                    piece.area > DistanceTolerance * DistanceTolerance)
                    own.Push (std::move (piece));
            }
            pieces.Append (own);
            return true;
        }

        void BeamPiece_AddQuad (RoomSurfaceBeamPiece &piece,
                                const BeamAxis &axis,
                                double t0,
                                double t1,
                                double offset,
                                double z0,
                                double z1) {
            const double ts[4] = {t0, t1, t1, t0};
            const double zs[4] = {z0, z0, z1, z1};
            for (Int32 i = 0; i < 4; ++i)
                piece.vertices.Push (axis.BeamAxis_GetPoint (ts[i], offset, zs[i]));
            const double length = t1 - t0;
            const double height = z1 - z0;
            piece.area = length * height;
        }

        void BeamPiece_AddBottom (RoomSurfaceBeamPiece &piece,
                                  const BeamAxis &axis,
                                  const GS::Array<RoomSurfaceVertex> &vertices,
                                  double t0,
                                  double t1,
                                  double z) {
            double offsetMin = 0, offsetMax = 0, alongMin = 0, alongMax = 0;
            bool first = true;
            for (const auto &vertex : vertices) {
                const double offset = axis.BeamAxis_GetOffset (vertex.world);
                const double along = axis.BeamAxis_GetAlong (vertex.world);
                if (first) {
                    offsetMin = offsetMax = offset;
                    alongMin = alongMax = along;
                    first = false;
                } else {
                    offsetMin = std::min (offsetMin, offset);
                    offsetMax = std::max (offsetMax, offset);
                    alongMin = std::min (alongMin, along);
                    alongMax = std::max (alongMax, along);
                }
            }
            t0 = std::max (t0, alongMin);
            t1 = std::min (t1, alongMax);
            if (first || t1 - t0 <= DistanceTolerance)
                return;
            piece.vertices.Push (axis.BeamAxis_GetPoint (t0, offsetMin, z));
            piece.vertices.Push (axis.BeamAxis_GetPoint (t1, offsetMin, z));
            piece.vertices.Push (axis.BeamAxis_GetPoint (t1, offsetMax, z));
            piece.vertices.Push (axis.BeamAxis_GetPoint (t0, offsetMax, z));
            const double width = offsetMax - offsetMin;
            piece.area = (t1 - t0) * width;
        }
    } // namespace

    RoomSurfaceBeamFinish RoomBeamFinish_Build (const RoomSurfaceSource &beamSource,
                                                const API_Coord &beg,
                                                const API_Coord &end,
                                                double tBeg,
                                                double tEnd,
                                                double zBottom,
                                                double zTop,
                                                RoomSurfaceBeamPartitions &result,
                                                const RoomSurfaceSnapshot *zone) {
        RoomSurfaceBeamFinish finish;
        finish.baseGuid = beamSource.baseGuid;
        finish.geometryGuid = beamSource.geometryGuid;
        if (beamSource.baseType != API_BeamID || !beamSource.snapshot.IsComplete ()) {
            RoomSurfaceReadError error;
            error.stage = "beamSourceSnapshot";
            error.code = APIERR_BADPARS;
            result.errors.Push (error);
            return finish;
        }
        BeamAxis axis;
        if (!axis.BeamAxis_Set (beg, end)) {
            RoomSurfaceReadError error;
            error.stage = "beamAxis";
            error.code = APIERR_BADPARS;
            result.errors.Push (error);
            return finish;
        }
        finish.tBeg = tBeg;
        finish.tEnd = tEnd;
        if (!std::isfinite (tBeg) || !std::isfinite (tEnd) || !std::isfinite (zBottom) || !std::isfinite (zTop) ||
            tEnd < tBeg || zTop < zBottom) {
            RoomSurfaceReadError error;
            error.stage = "beamInvalidPart";
            error.code = APIERR_BADPARS;
            result.errors.Push (error);
            return finish;
        }
        // В AC25 beamPart возвращает проекции на ненормированный вектор оси:
        // проверено по пересечениям границы зоны для двух балок разной длины и направления.
        // В отличие от wallPart, расстояние вдоль единичной оси равно t / length.
        tBeg /= axis.length;
        tEnd /= axis.length;
        const bool wholeZoneContact = zone != nullptr && tEnd - tBeg <= DistanceTolerance;
        if (!std::isfinite (tBeg) || !std::isfinite (tEnd) || (!wholeZoneContact && tEnd - tBeg <= DistanceTolerance) ||
            zTop - zBottom <= DistanceTolerance) {
            RoomSurfaceReadError error;
            error.stage = "beamEmptyPart";
            error.code = APIERR_BADPARS;
            result.errors.Push (error);
            return finish;
        }
        if (zone != nullptr && !zone->IsComplete ()) {
            RoomSurfaceReadError error;
            error.stage = "beamZoneSnapshot";
            error.code = APIERR_BADPARS;
            result.errors.Push (error);
            return finish;
        }
        // Пустая проекция оси не исключает контакт конечной грани; его проверяет полная 3D-зона.
        // Низ — грань с нормалью вниз; бока — вертикальные грани, параллельные оси. Прямые балки с
        // постоянным сечением; наклонные/кривые балки не распознаются и остаются не отделанными.
        for (const auto &body : beamSource.snapshot.bodies) {
            for (const auto &surface : body.surfaces) {
                const auto &n = surface.normalWorld;
                if (!Coord3D_IsFinite (n) || surface.contours.IsEmpty () || surface.contours[0].vertices.IsEmpty ())
                    continue;
                const auto &first = surface.contours[0].vertices[0].world;
                double zMin = 0, zMax = 0;
                bool firstVertex = true;
                for (const auto &contour : surface.contours) {
                    for (const auto &vertex : contour.vertices) {
                        const double z = vertex.world.z;
                        if (firstVertex) {
                            zMin = zMax = z;
                            firstVertex = false;
                        } else {
                            zMin = std::min (zMin, z);
                            zMax = std::max (zMax, z);
                        }
                    }
                }
                const double clippedBottom = std::max (zMin, zBottom);
                const double clippedTop = std::min (zMax, zTop);
                const double z = std::fabs (n.z);
                RoomSurfaceBeamPiece piece;
                if (z > 1.0 - DistanceTolerance) {
                    const double faceZ = first.z;
                    if (faceZ < zBottom - DistanceTolerance || faceZ > zTop + DistanceTolerance)
                        continue;
                    piece.face = n.z > 0 ? RoomSurfaceBeamFace::Top : RoomSurfaceBeamFace::Bottom;
                    if (zone != nullptr) {
                        if (!BeamFace_ClipToZone (surface,
                                                  *zone,
                                                  axis,
                                                  tBeg,
                                                  tEnd,
                                                  zBottom,
                                                  zTop,
                                                  piece.face,
                                                  wholeZoneContact,
                                                  finish.pieces)) {
                            RoomSurfaceReadError error;
                            error.stage = "beamZoneSection";
                            error.code = APIERR_BADPARS;
                            result.errors.Push (error);
                            finish.pieces.Clear ();
                            return finish;
                        }
                        continue;
                    }
                    BeamPiece_AddBottom (piece, axis, surface.contours[0].vertices, tBeg, tEnd, faceZ);
                } else if (z < DistanceTolerance) {
                    if (clippedTop - clippedBottom <= DistanceTolerance)
                        continue;
                    const double alongN = n.x * axis.dir.x + n.y * axis.dir.y;
                    if (std::fabs (alongN) > DistanceTolerance) // торец балки — не отделываем
                        continue;
                    if (zone != nullptr) {
                        piece.face = (n.x * axis.perp.x + n.y * axis.perp.y) > 0 ? RoomSurfaceBeamFace::RightSide
                                                                                 : RoomSurfaceBeamFace::LeftSide;
                        if (!BeamFace_ClipToZone (surface,
                                                  *zone,
                                                  axis,
                                                  tBeg,
                                                  tEnd,
                                                  zBottom,
                                                  zTop,
                                                  piece.face,
                                                  wholeZoneContact,
                                                  finish.pieces)) {
                            RoomSurfaceReadError error;
                            error.stage = "beamZoneSection";
                            error.code = APIERR_BADPARS;
                            result.errors.Push (error);
                            finish.pieces.Clear ();
                            return finish;
                        }
                        continue;
                    }
                    const double offset = axis.BeamAxis_GetOffset (first);
                    double tMin = 0, tMax = 0;
                    firstVertex = true;
                    for (const auto &vertex : surface.contours[0].vertices) {
                        const double t = axis.BeamAxis_GetAlong (vertex.world);
                        if (firstVertex) {
                            tMin = tMax = t;
                            firstVertex = false;
                        } else {
                            tMin = std::min (tMin, t);
                            tMax = std::max (tMax, t);
                        }
                    }
                    const double t0 = std::max (tMin, tBeg);
                    const double t1 = std::min (tMax, tEnd);
                    if (t1 - t0 <= DistanceTolerance)
                        continue;
                    piece.face = (n.x * axis.perp.x + n.y * axis.perp.y) > 0 ? RoomSurfaceBeamFace::RightSide
                                                                             : RoomSurfaceBeamFace::LeftSide;
                    BeamPiece_AddQuad (piece, axis, t0, t1, offset, clippedBottom, clippedTop);
                } else {
                    continue; // наклонная грань — вне объёма задачи
                }
                if (piece.area > DistanceTolerance * DistanceTolerance)
                    finish.pieces.Push (std::move (piece));
            }
        }
        return finish;
    }

    // Подготовка геометрии стен, проёмов, полов и потолков.

    bool FloorSkirtingSegments_Build (OtdRoom &room) {
        constexpr double tolerance = FloorCoreGeometry::Tolerance;
        room.otdbeam_floor.Clear ();
        if (!room.isValid)
            return true;
        if (!std::isfinite (room.zBottom))
            return false;
        GS::Array<OtdBeam> prepared;
        for (UIndex i = 0; i < room.otdwall.GetSize (); ++i) {
            const OtdWall &wall = room.otdwall[i];
            if (!wall.isValid)
                continue;
            const bool supportsFloorSkirting = wall.draw.type == Wall_Main || wall.draw.type == Wall_Down ||
                                               wall.draw.type == Wall_Up || wall.draw.type == Column;
            if (!supportsFloorSkirting)
                continue;
            if (wall.draw.type != Column && !room.create_wall_elements)
                continue;
            if (wall.draw.type == Column && !room.create_column_elements)
                continue;
            if (!std::isfinite (OtdLinearSegment_GetZBottom (wall.geom)))
                return false;
            if (std::fabs (OtdLinearSegment_GetZBottom (wall.geom) - room.zBottom) > tolerance)
                continue;
            const double dx = wall.geom.seg.endC.x - wall.geom.seg.begC.x;
            const double dy = wall.geom.seg.endC.y - wall.geom.seg.begC.y;
            const double length = std::hypot (dx, dy);
            if (!std::isfinite (length) || length <= tolerance || !std::isfinite (wall.geom.seg.begC.x) ||
                !std::isfinite (wall.geom.seg.begC.y) || !std::isfinite (wall.geom.seg.endC.x) ||
                !std::isfinite (wall.geom.seg.endC.y))
                return false;
            std::vector<AnalyticGeometry::Rect> openings;
            openings.reserve (wall.openings.GetSize ());
            for (const auto &opening : wall.openings)
                openings.push_back ({opening.objLoc - opening.width / 2,
                                     opening.zBottom,
                                     opening.objLoc + opening.width / 2,
                                     opening.zBottom + opening.height});
            std::vector<std::pair<double, double>> segments;
            if (!AnalyticGeometry::CutWallBottom ({0,
                                                   OtdLinearSegment_GetZBottom (wall.geom),
                                                   length,
                                                   OtdLinearSegment_GetZBottom (wall.geom) + wall.geom.height},
                                                  openings,
                                                  tolerance,
                                                  segments))
                return false;
            for (const auto &segment : segments) {
                OtdBeam beam;
                beam.geom.seg.begC = {wall.geom.seg.begC.x + dx * (segment.first / length),
                                      wall.geom.seg.begC.y + dy * (segment.first / length),
                                      OtdLinearSegment_GetZBottom (wall.geom)};
                beam.geom.seg.endC = {wall.geom.seg.begC.x + dx * (segment.second / length),
                                      wall.geom.seg.begC.y + dy * (segment.second / length),
                                      OtdLinearSegment_GetZBottom (wall.geom)};

                beam.geom.floorInd = wall.geom.floorInd;
                beam.draw.base_guid = wall.draw.base_guid;
                beam.draw.base_type = wall.draw.base_type;
                // Плинтус рисуется балкой; поле хранит тип рисуемого элемента.
                beam.draw.draw_type = API_BeamID;
                beam.tip = room.tip_floor_skirting; // Тип плинтуса пола из зоны.
                beam.wallIndex = static_cast<Int32> (i);
                prepared.Push (beam);
            }
        }
        room.otdbeam_floor.Swap (prepared);
        return true;
    }

    // slabFaces — комнатные плоскости с толщиной выбранного избранного, индексы исходного otdslab.
    bool SkirtingSegments_Build (OtdRoom &room, const GS::Array<OtdSlab> &slabFaces) {
        room.otdbeam_floor.Clear ();
        room.otdbeam_ceil.Clear ();
        if (!room.isValid)
            return true;
        GS::Array<OtdBeam> floors, ceilings;
        const auto rectangle = [] (double left, double right, double bottom, double top) {
            GS::Array<Point2D> points = {{left, bottom}, {right, bottom}, {right, top}, {left, top}};
            GS::Array<PolyArcRec> arcs;
            GS::Array<UIndex> ends = {3};
            return Geometry::Polygon2D::Create (
                points, arcs, ends, Geometry::PolyCreateFlags_IdxRefInContourEndFromZero);
        };
        for (UIndex wi = 0; wi < room.otdwall.GetSize (); ++wi) {
            const auto &wall = room.otdwall[wi];
            if (!wall.isValid || (wall.draw.type != Wall_Main && wall.draw.type != Wall_Down &&
                                  wall.draw.type != Wall_Up && wall.draw.type != Column))
                continue;
            if (wall.draw.type == Column ? !room.create_column_elements : !room.create_wall_elements)
                continue;
            const double length =
                std::hypot (wall.geom.seg.endC.x - wall.geom.seg.begC.x, wall.geom.seg.endC.y - wall.geom.seg.begC.y);
            if (!std::isfinite (length) || length <= DistanceTolerance || !std::isfinite (wall.geom.seg.begC.x) ||
                !std::isfinite (wall.geom.seg.begC.y) || !std::isfinite (wall.geom.seg.endC.x) ||
                !std::isfinite (wall.geom.seg.endC.y) || !std::isfinite (OtdLinearSegment_GetZBottom (wall.geom)) ||
                !std::isfinite (wall.geom.height) || wall.geom.height <= DistanceTolerance ||
                !std::isfinite (wall.rise_x) || !std::isfinite (wall.rise_y))
                return false;
            const double ux = (wall.geom.seg.endC.x - wall.geom.seg.begC.x) / length;
            const double uy = (wall.geom.seg.endC.y - wall.geom.seg.begC.y) / length;
            Polygons wallPolygons = rectangle (0,
                                               length,
                                               OtdLinearSegment_GetZBottom (wall.geom),
                                               OtdLinearSegment_GetZBottom (wall.geom) + wall.geom.height);
            for (const auto &opening : wall.openings) {
                if (!std::isfinite (opening.objLoc) || !std::isfinite (opening.width) ||
                    !std::isfinite (opening.zBottom) || !std::isfinite (opening.height) || opening.width <= 0 ||
                    opening.height <= 0)
                    return false;
                wallPolygons = Polygons::Substract (wallPolygons,
                                                    rectangle (opening.objLoc - opening.width / 2,
                                                               opening.objLoc + opening.width / 2,
                                                               opening.zBottom,
                                                               opening.zBottom + opening.height));
            }
            for (UIndex si = 0; si < slabFaces.GetSize (); ++si) {
                const auto &slab = slabFaces[si];
                if (!slab.isValid || (slab.draw.type != Floor && slab.draw.type != Ceil) || slab.poly.IsEmpty ())
                    continue;
                if (slab.draw.type == Floor ? (!room.has_floor || !room.create_floor_elements)
                                            : (!room.has_ceil || !room.create_ceil_elements))
                    continue;
                if (!std::isfinite (slab.zBottom) || !std::isfinite (slab.plane_origin.x) ||
                    !std::isfinite (slab.plane_origin.y) || !std::isfinite (slab.slope_x) ||
                    !std::isfinite (slab.slope_y))
                    return false;
                // Применяем rise лишь при переходе от готового низа стены к другой отметке стыка.
                const double divisor = 1 - slab.slope_x * wall.rise_x - slab.slope_y * wall.rise_y;
                if (!std::isfinite (divisor) || std::fabs (divisor) <= DistanceTolerance)
                    return false;
                const double z0 = OtdLinearSegment_GetZBottom (wall.geom) +
                                  (slab.zBottom - OtdLinearSegment_GetZBottom (wall.geom) +
                                   slab.slope_x * (wall.geom.seg.begC.x - slab.plane_origin.x) +
                                   slab.slope_y * (wall.geom.seg.begC.y - slab.plane_origin.y)) /
                                      divisor;
                const double dz = (slab.slope_x * ux + slab.slope_y * uy) / divisor;
                if (!std::isfinite (z0) || !std::isfinite (dz))
                    return false;
                const auto world = [&] (const Point2D &p) -> API_Coord3D {
                    return {
                        wall.geom.seg.begC.x + ux * p.x + wall.rise_x * (p.y - OtdLinearSegment_GetZBottom (wall.geom)),
                        wall.geom.seg.begC.y + uy * p.x + wall.rise_y * (p.y - OtdLinearSegment_GetZBottom (wall.geom)),
                        p.y};
                };
                const Sector line (Point2D (0, z0), Point2D (length, z0 + dz * length));
                for (const auto &polygon : wallPolygons) {
                    for (const auto &part : polygon.CutElem (line, Geometry::Boundary_Closed)) {
                        const auto beg = world (part.c1), end = world (part.c2);
                        const double dx = end.x - beg.x, dy = end.y - beg.y;
                        const double planSquared = dx * dx + dy * dy;
                        if (!std::isfinite (planSquared) || planSquared <= DistanceTolerance * DistanceTolerance)
                            continue;
                        const Sector plan (Point2D (beg.x, beg.y), Point2D (end.x, end.y));
                        // Не распрямляем дуги и не теряем отверстия итогового пола/потолка.
                        for (const auto &cut : slab.poly.CutElem (plan, Geometry::Boundary_Closed)) {
                            OtdBeam beam;
                            const auto height = [&] (const Point2D &p) {
                                const double t = ((p.x - beg.x) * dx + (p.y - beg.y) * dy) / planSquared;
                                return beg.z + t * (end.z - beg.z);
                            };
                            beam.geom.seg.begC = {cut.c1.x, cut.c1.y, height (cut.c1)};
                            beam.geom.seg.endC = {cut.c2.x, cut.c2.y, height (cut.c2)};
                            if (!std::isfinite (beam.geom.seg.begC.z) || !std::isfinite (beam.geom.seg.endC.z))
                                return false;
                            if (std::hypot (std::hypot (beam.geom.seg.endC.x - beam.geom.seg.begC.x,
                                                        beam.geom.seg.endC.y - beam.geom.seg.begC.y),
                                            beam.geom.seg.endC.z - beam.geom.seg.begC.z) <= DistanceTolerance)
                                continue;
                            beam.geom.floorInd = wall.geom.floorInd;
                            beam.draw.base_guid = wall.draw.base_guid;
                            beam.draw.base_type = wall.draw.base_type;
                            // Плинтус рисуется балкой; поле хранит тип рисуемого элемента.
                            beam.draw.draw_type = API_BeamID;
                            beam.wallIndex = static_cast<Int32> (wi);
                            beam.slabIndex = static_cast<Int32> (si);
                            beam.draw.type = slab.draw.type == Floor ? Floor_Skirting : Ceil_Cornice;
                            beam.tip = room.tip_floor_skirting;
                            auto &output = slab.draw.type == Floor ? floors : ceilings;
                            const API_Coord3D origin = {
                                beam.geom.seg.begC.x, beam.geom.seg.begC.y, beam.geom.seg.begC.z};
                            const API_Coord3D axis = {beam.geom.seg.endC.x - origin.x,
                                                      beam.geom.seg.endC.y - origin.y,
                                                      beam.geom.seg.endC.z - origin.z};
                            const double squared = Coord3D_Dot (axis, axis);
                            const double tolerance = DistanceTolerance / std::sqrt (squared);
                            std::vector<std::pair<double, double>> remaining = {{0, 1}};
                            // Смежные конечные участки могут вернуть один стык на общем шве дважды.
                            for (const auto &previous : output) {
                                if (previous.draw.base_guid != beam.draw.base_guid ||
                                    previous.draw.base_type != beam.draw.base_type)
                                    continue;
                                const API_Coord3D p = {
                                    previous.geom.seg.begC.x, previous.geom.seg.begC.y, previous.geom.seg.begC.z};
                                const API_Coord3D q = {
                                    previous.geom.seg.endC.x, previous.geom.seg.endC.y, previous.geom.seg.endC.z};
                                const double a = Coord3D_Dot (Coord3D_Subtract (p, origin), axis) / squared;
                                const double b = Coord3D_Dot (Coord3D_Subtract (q, origin), axis) / squared;
                                const auto pointAt = [&] (double t) -> API_Coord3D {
                                    return {origin.x + axis.x * t, origin.y + axis.y * t, origin.z + axis.z * t};
                                };
                                const auto onLine = [&] (const API_Coord3D &p, double t) {
                                    const auto projected = pointAt (t);
                                    return std::hypot (std::hypot (p.x - projected.x, p.y - projected.y),
                                                       p.z - projected.z) <= DistanceTolerance;
                                };
                                if (!onLine (p, a) || !onLine (q, b))
                                    continue;
                                std::vector<std::pair<double, double>> next;
                                for (const auto &interval : remaining) {
                                    const double lo = std::max (interval.first, std::min (a, b));
                                    const double hi = std::min (interval.second, std::max (a, b));
                                    if (hi - lo <= tolerance) {
                                        next.push_back (interval);
                                        continue;
                                    }
                                    if (lo - interval.first > tolerance)
                                        next.push_back ({interval.first, lo});
                                    if (interval.second - hi > tolerance)
                                        next.push_back ({hi, interval.second});
                                }
                                remaining.swap (next);
                            }
                            for (const auto &interval : remaining) {
                                OtdBeam fragment = beam;
                                fragment.geom.seg.begC = {origin.x + axis.x * interval.first,
                                                          origin.y + axis.y * interval.first};
                                fragment.geom.seg.endC = {origin.x + axis.x * interval.second,
                                                          origin.y + axis.y * interval.second};
                                fragment.geom.seg.begC.z = origin.z + axis.z * interval.first;
                                fragment.geom.seg.endC.z = origin.z + axis.z * interval.second;
                                output.Push (std::move (fragment));
                            }
                        }
                    }
                }
            }
        }
        room.otdbeam_floor.Swap (floors);
        room.otdbeam_ceil.Swap (ceilings);
        return true;
    }

    // Конечный стык выбранных граней. SDK-модель и подготовленные кандидаты не изменяет.
    bool SkirtingContact_Build (const RoomSurface &wallFace,
                                const RoomSurface &slabFace,
                                GS::Array<OtdLinearSegment> &segments) {
        segments.Clear ();
        Frame wallFrame, slabFrame;
        if (!wallFrame.Frame_Set (wallFace) || !slabFrame.Frame_Set (slabFace))
            return false;
        // У partition допуск плоскости шире; здесь зазор не должен превращаться в контакт.
        for (const auto *face : {&wallFace, &slabFace}) {
            const auto &origin = face->contours[0].vertices[0].world;
            if (std::fabs (Coord3D_Dot (face->normalWorld, origin) + face->planeOffset) > DistanceTolerance)
                return false;
            for (const auto &contour : face->contours)
                for (const auto &vertex : contour.vertices)
                    if (!Coord3D_IsFinite (vertex.world) ||
                        std::fabs (Coord3D_Dot (face->normalWorld, Coord3D_Subtract (vertex.world, origin))) >
                            DistanceTolerance)
                        return false;
        }
        Polygons wallPolygons, slabPolygons;
        if (!RoomSurface_Project (wallFace, wallFrame, wallPolygons) ||
            !RoomSurface_Project (slabFace, slabFrame, slabPolygons))
            return false;
        API_Coord3D direction = Coord3D_Cross (wallFrame.normal, slabFrame.normal);
        const double squaredLength = Coord3D_Dot (direction, direction);
        if (!std::isfinite (squaredLength) || squaredLength <= DistanceTolerance * DistanceTolerance)
            return false; // Нет устойчиво определённой единственной линии стыка.
        const double directionLength = std::sqrt (squaredLength);
        direction = {direction.x / directionLength, direction.y / directionLength, direction.z / directionLength};
        // Начало берём около стены, а не около мирового нуля: сохраняем точность удалённых комнат.
        const double cosine = Coord3D_Dot (wallFrame.normal, slabFrame.normal);
        const double distance = Coord3D_Dot (slabFrame.normal, Coord3D_Subtract (wallFrame.origin, slabFrame.origin));
        const double shift = -distance / squaredLength;
        const API_Coord3D origin = {wallFrame.origin.x + shift * (slabFrame.normal.x - cosine * wallFrame.normal.x),
                                    wallFrame.origin.y + shift * (slabFrame.normal.y - cosine * wallFrame.normal.y),
                                    wallFrame.origin.z + shift * (slabFrame.normal.z - cosine * wallFrame.normal.z)};
        if (!Coord3D_IsFinite (origin))
            return false;
        const auto pointAt = [&] (double t) -> API_Coord3D {
            return {origin.x + direction.x * t, origin.y + direction.y * t, origin.z + direction.z * t};
        };
        double first = 0, last = 0;
        bool hasVertex = false;
        for (const auto &contour : wallFace.contours)
            for (const auto &vertex : contour.vertices) {
                const double t = Coord3D_Dot (Coord3D_Subtract (vertex.world, origin), direction);
                if (!std::isfinite (t))
                    return false;
                first = hasVertex ? std::min (first, t) : t;
                last = hasVertex ? std::max (last, t) : t;
                hasVertex = true;
            }
        if (last - first <= DistanceTolerance)
            return true;
        std::vector<std::pair<double, double>> intervals;
        const Sector wallLine (wallFrame.Frame_Project (pointAt (first)), wallFrame.Frame_Project (pointAt (last)));
        for (const auto &wallPolygon : wallPolygons) {
            for (const auto &wallPart : wallPolygon.CutElem (wallLine, Geometry::Boundary_Closed)) {
                const Sector slabLine (slabFrame.Frame_Project (wallFrame.Frame_Restore (wallPart.c1)),
                                       slabFrame.Frame_Project (wallFrame.Frame_Restore (wallPart.c2)));
                for (const auto &slabPolygon : slabPolygons) {
                    for (const auto &part : slabPolygon.CutElem (slabLine, Geometry::Boundary_Closed)) {
                        const double a =
                            Coord3D_Dot (Coord3D_Subtract (slabFrame.Frame_Restore (part.c1), origin), direction);
                        const double b =
                            Coord3D_Dot (Coord3D_Subtract (slabFrame.Frame_Restore (part.c2), origin), direction);
                        if (!std::isfinite (a) || !std::isfinite (b))
                            return false;
                        const double lo = std::max (first, std::min (a, b));
                        const double hi = std::min (last, std::max (a, b));
                        if (hi - lo > DistanceTolerance)
                            intervals.push_back ({lo, hi});
                    }
                }
            }
        }
        std::sort (intervals.begin (), intervals.end ());
        // Regularize может разбить грань на несколько полигонов; их общий шов не удваивает длину.
        GS::Array<OtdLinearSegment> prepared;
        bool hasInterval = false;
        double begin = 0, end = 0;
        for (const auto &interval : intervals) {
            if (hasInterval && interval.first > end + DistanceTolerance) {
                prepared.Push ({pointAt (begin), pointAt (end)});
                hasInterval = false;
            }
            if (!hasInterval) {
                begin = interval.first;
                end = interval.second;
                hasInterval = true;
            } else {
                end = std::max (end, interval.second);
            }
        }
        if (hasInterval)
            prepared.Push ({pointAt (begin), pointAt (end)});
        segments.Swap (prepared);
        return true;
    }

    USize PolygonVertices_RemoveCollinear (Geometry::Polygon2D &polygon) {
        if (polygon.IsEmpty ())
            return 0;
        const USize before = polygon.GetVertexNum ();
        Geometry::Polygon2D::RemoveSameDirEdgesWithSameEdgeUserData (polygon);
        return before - polygon.GetVertexNum ();
    }

    bool FloorContour_ExtendToOpening (OtdRoom &room, const OtdWall &wall, const OtdOpening &opening) {
        constexpr double tolerance = FloorCoreGeometry::Tolerance;
        const double depth = opening.base_reveal_width + opening.base_frame_width / 2;
        const double dx = wall.geom.seg.endC.x - wall.geom.seg.begC.x, dy = wall.geom.seg.endC.y - wall.geom.seg.begC.y;
        const double length = std::hypot (dx, dy);
        if (!wall.isValid || opening.base_guid == APINULLGuid)
            return true;
        if (!std::isfinite (length) || length <= tolerance)
            return true;
        if (!std::isfinite (opening.objLoc) || !std::isfinite (opening.width) || opening.width <= tolerance)
            return true;
        if (!std::isfinite (depth) || depth <= tolerance)
            return true;
        if (!std::isfinite (opening.zBottom) || !std::isfinite (opening.height) || opening.height <= tolerance)
            return true;
        if (opening.zBottom > room.zBottom + tolerance)
            return true;

        const double left = std::max (0.0, opening.objLoc - opening.width / 2);
        const double right = std::min (length, opening.objLoc + opening.width / 2);
        if (right - left <= tolerance)
            return true;
        const Sector entrance = {
            {wall.geom.seg.begC.x + dx * left / length, wall.geom.seg.begC.y + dy * left / length},
            {wall.geom.seg.begC.x + dx * right / length, wall.geom.seg.begC.y + dy * right / length}};
        const double shiftX = dy / length * depth;
        const double shiftY = -dx / length * depth;
        bool complete = true;
        for (auto &floor : room.otdslab) {
            if (floor.draw.base_type == API_MeshID || !floor.isValid || floor.draw.type != Floor ||
                floor.poly.IsEmpty () || opening.zBottom > floor.zBottom + tolerance ||
                opening.zBottom + opening.height <= floor.zBottom + tolerance)
                continue;
            Geometry::MultiPolygon2D united;
            united.Push (floor.poly);
            // Каждый пол входит только на своей конечной части проёма, включая совпавшую границу.
            for (const auto &contact : floor.poly.CutElem (entrance, Geometry::Boundary_Closed)) {
                if (std::hypot (contact.c2.x - contact.c1.x, contact.c2.y - contact.c1.y) <= tolerance)
                    continue;
                const GS::Array<Point2D> coords = {contact.c1,
                                                   contact.c2,
                                                   {contact.c2.x + shiftX, contact.c2.y + shiftY},
                                                   {contact.c1.x + shiftX, contact.c1.y + shiftY}};
                united.Append (Geometry::Polygon2D::Create (coords, 0));
            }
            if (united.GetSize () == 1)
                continue;
            united.Unify ();
            Geometry::MultiPolygon2D original;
            original.Push (floor.poly);
            if (united.GetSize () != 1 ||
                Geometry::MultiPolygon2D::Substract (original, united).CalcArea () > tolerance) {
                complete = false;
                continue;
            }
            floor.poly = united[0];
        }
        return complete;
    }

    bool FloorContours_ExtendToCore (OtdRoom &room) {
        using namespace FloorCoreGeometry;
        GS::Array<OtdSlab> result;
        bool complete = true;
        for (const auto &floor : room.otdslab) {
            if (floor.draw.base_type == API_MeshID || !floor.isValid || floor.draw.type != Floor ||
                floor.poly.IsEmpty ()) {
                result.Push (floor);
                continue;
            }
            std::vector<Contact> contacts;
            std::vector<API_Guid> owners;
            for (const auto &wall : room.otdwall) {
                if (!wall.isValid || wall.draw.base_guid == APINULLGuid || wall.draw.type == Reveal_Main ||
                    wall.draw.type == Reveal_Up || wall.draw.type == Reveal_Down ||
                    floor.zBottom < OtdLinearSegment_GetZBottom (wall.geom) - Tolerance ||
                    floor.zBottom >= OtdLinearSegment_GetZBottom (wall.geom) + wall.geom.height - Tolerance)
                    continue;
                auto owner = std::find (owners.begin (), owners.end (), wall.draw.base_guid);
                if (owner == owners.end ()) {
                    owners.push_back (wall.draw.base_guid);
                    owner = owners.end () - 1;
                }
                const std::size_t ownerIndex = static_cast<std::size_t> (owner - owners.begin ());
                const double dz = floor.zBottom - OtdLinearSegment_GetZBottom (wall.geom);
                const Point begin = {wall.geom.seg.begC.x + dz * wall.rise_x, wall.geom.seg.begC.y + dz * wall.rise_y};
                const Point end = {wall.geom.seg.endC.x + dz * wall.rise_x, wall.geom.seg.endC.y + dz * wall.rise_y};
                const double length = Length (end - begin);
                if (length <= Tolerance)
                    continue;
                const Point dir = (end - begin) * (1 / length);
                // Не расширяем под открытым проёмом на отметке пола.
                std::vector<std::pair<double, double>> ranges = {{0, length}};
                for (const auto &opening : wall.openings) {
                    if (opening.zBottom > floor.zBottom + Tolerance ||
                        opening.zBottom + opening.height <= floor.zBottom + Tolerance)
                        continue;
                    const double a = std::max (0.0, opening.objLoc - opening.width / 2);
                    const double b = std::min (length, opening.objLoc + opening.width / 2);
                    if (b <= a)
                        continue;
                    std::vector<std::pair<double, double>> next;
                    for (const auto &range : ranges) {
                        if (b <= range.first || a >= range.second) {
                            next.push_back (range);
                            continue;
                        }
                        if (a > range.first)
                            next.push_back ({range.first, a});
                        if (b < range.second)
                            next.push_back ({b, range.second});
                    }
                    ranges = std::move (next);
                }
                const double offset =
                    wall.draw.base_type == API_WallID && !wall.slab_step && wall.floor_core_offset_valid
                        ? wall.floor_core_offset
                        : 0;
                for (const auto &range : ranges)
                    contacts.push_back ({begin + dir * range.first, begin + dir * range.second, offset, ownerIndex});
            }
            Geometry::MultiPolygon2D polygons;
            polygons.Push (floor.poly);
            bool changed = false;
            for (auto contour = floor.poly.BeginContour (); contour != floor.poly.EndContour (); ++contour) {
                std::vector<Edge> edges;
                for (auto edge = contour->BeginEdge (); edge != contour->EndEdge (); ++edge) {
                    Point2D begin, end;
                    double angle = 0;
                    floor.poly.GetSector (edge, begin, end, angle);
                    edges.push_back ({{begin.x, begin.y}, {end.x, end.y}, std::fabs (angle) <= Tolerance});
                }
                for (const auto &patch : Extensions (edges, contacts)) {
                    GS::Array<Point2D> coords;
                    for (const auto &point : patch)
                        coords.Push ({point.x, point.y});
                    const auto strips = Geometry::Polygon2D::Create (coords, 0);
                    for (const auto &strip : strips)
                        polygons.Push (strip);
                    changed = changed || !strips.IsEmpty ();
                }
            }
            if (!changed) {
                result.Push (floor);
                continue;
            }
            polygons.Unify ();
            // Расширение не может удалить старое покрытие или разделить один owner на острова.
            Geometry::MultiPolygon2D original;
            original.Push (floor.poly);
            if (polygons.GetSize () != 1 || polygons[0].GetContourNum () != floor.poly.GetContourNum () ||
                Geometry::MultiPolygon2D::Substract (original, polygons).CalcArea () > Tolerance) {
                result.Push (floor);
                complete = false;
                continue;
            }
            OtdSlab extended = floor; // Все links/материальные правила/избранное/отметка сохраняются.
            extended.poly = polygons[0];
            result.Push (std::move (extended));
        }
        for (auto &slab : result)
            if (slab.isValid && (slab.draw.type == Floor || slab.draw.type == Ceil))
                PolygonVertices_RemoveCollinear (slab.poly);
        room.otdslab = std::move (result);
        return complete;
    }

    namespace {

        double Coord3D_GetLength (const API_Coord3D &a) { return std::hypot (std::hypot (a.x, a.y), a.z); }

        bool Coord3D_AreNear (const API_Coord3D &a, const API_Coord3D &b) {
            return Coord3D_GetLength (Coord3D_Subtract (a, b)) <= DistanceTolerance;
        }

        bool RoomSurfacePiece_IsValid (const RoomSurfacePiece &piece) {
            if (!Coord3D_IsFinite (piece.normal) ||
                std::fabs (Coord3D_GetLength (piece.normal) - 1) > DistanceTolerance ||
                !std::isfinite (piece.planeOffset) || !std::isfinite (piece.area) || piece.area <= AreaTolerance ||
                piece.contours.IsEmpty ())
                return false;
            for (const auto &contour : piece.contours) {
                if (contour.vertices.GetSize () < 3)
                    return false;
                for (const auto &p : contour.vertices)
                    if (!Coord3D_IsFinite (p) ||
                        std::fabs (Coord3D_Dot (piece.normal, p) + piece.planeOffset) > DistanceTolerance)
                        return false;
            }
            return true;
        }

        // Удаляем только точки между соседями, не разворот контура.
        std::vector<API_Coord3D> RoomSurfaceContour_GetCorners (const RoomSurfacePieceContour &contour) {
            std::vector<API_Coord3D> points;
            for (const auto &p : contour.vertices)
                if (points.empty () || !Coord3D_AreNear (points.back (), p))
                    points.push_back (p);
            if (points.size () > 1 && Coord3D_AreNear (points.front (), points.back ()))
                points.pop_back ();
            bool changed = true;
            while (changed && points.size () > 3) {
                changed = false;
                for (size_t i = 0; i < points.size (); ++i) {
                    const auto a = Coord3D_Subtract (points[i], points[(i + points.size () - 1) % points.size ()]);
                    const auto b = Coord3D_Subtract (points[(i + 1) % points.size ()], points[i]);
                    if (Coord3D_Dot (a, b) >= 0 &&
                        Coord3D_GetLength (Coord3D_Cross (a, b)) <=
                            DistanceTolerance * (Coord3D_GetLength (a) + Coord3D_GetLength (b))) {
                        points.erase (points.begin () + i);
                        changed = true;
                        break;
                    }
                }
            }
            return points;
        }
    } // namespace

    bool SurfaceWall_Build (const RoomSurfacePiece &piece, short floorInd, OtdWall &wall) {
        if (!RoomSurfacePiece_IsValid (piece) || piece.contours.GetSize () != 1 || piece.contours[0].hole)
            return false;
        const double horizontal = std::hypot (piece.normal.x, piece.normal.y);
        if (horizontal <= DistanceTolerance)
            return false;
        auto points = RoomSurfaceContour_GetCorners (piece.contours[0]);
        if (points.size () != 4)
            return false;
        // Штатной стене нужен параллелограмм; экстремумы произвольной грани не заменяют её контур.
        size_t bottom = 4;
        for (size_t i = 0; i < 4; ++i) {
            if (std::fabs (points[i].z - points[(i + 1) % 4].z) <= DistanceTolerance &&
                points[(i + 2) % 4].z > points[i].z + min_dim && points[(i + 3) % 4].z > points[i].z + min_dim) {
                bottom = i;
                break;
            }
        }
        if (bottom == 4)
            return false;
        auto a = points[bottom], b = points[(bottom + 1) % 4];
        const auto topB = points[(bottom + 2) % 4], topA = points[(bottom + 3) % 4];
        const double height = topA.z - a.z;
        if (std::fabs (topB.z - topA.z) > DistanceTolerance || height < min_dim)
            return false;
        const double riseX = -piece.normal.z * piece.normal.x / (horizontal * horizontal);
        const double riseY = -piece.normal.z * piece.normal.y / (horizontal * horizontal);
        const API_Coord3D rise = {riseX * height, riseY * height, height};
        if (!Coord3D_AreNear (Coord3D_Subtract (topA, a), rise) || !Coord3D_AreNear (Coord3D_Subtract (topB, b), rise))
            return false;
        const double length = std::hypot (b.x - a.x, b.y - a.y);
        const double area = length * height / horizontal;
        // Площадь наследует погрешность точек вдоль периметра; допуск по точкам не меняем.
        const double areaError = std::max (AreaTolerance, 2 * (length + height / horizontal) * DistanceTolerance);
        if (!std::isfinite (area) || length < min_dim || std::fabs (area - piece.area) > areaError)
            return false;
        // Inside при flipped=false строит тело слева от линии: слева должна быть зона.
        // Значит, правый бок (dy,-dx) сонаправлен наружной нормали зоны.
        if ((b.y - a.y) * piece.normal.x - (b.x - a.x) * piece.normal.y < 0)
            std::swap (a, b);
        const double rightX = (b.y - a.y) / length, rightY = -(b.x - a.x) / length;
        if (std::fabs (rightX - piece.normal.x / horizontal) > DistanceTolerance ||
            std::fabs (rightY - piece.normal.y / horizontal) > DistanceTolerance)
            return false;
        OtdWall result;
        const Sector edge = {{a.x, a.y}, {b.x, b.y}};
        if (!OtdWall_Add_One (piece.baseGuid, edge, false, height, a.z, floorInd, 0, result))
            return false;
        result.geom.height = height;
        result.draw.base_guid = piece.baseGuid;
        result.draw.base_type = piece.baseType;
        result.draw.type = piece.baseType == API_ColumnID ? Column : Wall_Main;
        // В AC25 положительный котангенс slantAlpha наклоняет влево от линии.
        // Отрицательная проекция подъёма на правый бок сохраняет заданную контактную плоскость.
        result.slant_angle = std::atan2 (1.0, -(riseX * rightX + riseY * rightY));
        result.surface_geometry = true;
        result.rise_x = riseX;
        result.rise_y = riseY;
        wall = result;
        return true;
    }

    // Участок задаёт конечные границы только на своём roomEdge; внутренние разрывы не заполняем.
    GS::Array<OtdWall> SurfaceWall_ClampToParts (const OtdWall &wall,
                                                 const API_Coord &wallBeg,
                                                 const API_Coord &wallEnd,
                                                 const GS::Array<API_WallPart> &parts,
                                                 const GS::Array<Sector> &roomEdges,
                                                 double roomBottom,
                                                 const GS::Array<OtdWall> &contacts) {
        GS::Array<OtdWall> result;
        const double dx = wallEnd.x - wallBeg.x, dy = wallEnd.y - wallBeg.y;
        const double length = std::hypot (dx, dy);
        const double ex = wall.geom.seg.endC.x - wall.geom.seg.begC.x, ey = wall.geom.seg.endC.y - wall.geom.seg.begC.y;
        const double span = std::hypot (ex, ey);
        if (!std::isfinite (length) || !std::isfinite (span) || !std::isfinite (roomBottom) ||
            !std::isfinite (OtdLinearSegment_GetZBottom (wall.geom)) || !std::isfinite (wall.geom.height) ||
            !std::isfinite (wall.rise_x) || !std::isfinite (wall.rise_y) || length < min_dim || span < min_dim ||
            wall.geom.height < min_dim)
            return result;
        const double ux = dx / length, uy = dy / length, vx = ex / span, vy = ey / span;
        if (std::fabs (ux * vy - uy * vx) > DistanceTolerance)
            return result;
        const auto along = [&] (double x, double y) {
            return (x - wall.geom.seg.begC.x) * vx + (y - wall.geom.seg.begC.y) * vy;
        };
        const auto onLine = [&] (double x, double y) {
            return std::fabs ((x - wall.geom.seg.begC.x) * vy - (y - wall.geom.seg.begC.y) * vx) <= DistanceTolerance;
        };

        std::vector<std::pair<double, double>> intervals;
        bool hasParts = false;
        for (const auto &part : parts) {
            if (part.guid != wall.draw.base_guid)
                continue;
            hasParts = true;
            if (part.roomEdge <= 0 || static_cast<USize> (part.roomEdge) > roomEdges.GetSize () ||
                !std::isfinite (part.tBeg) || !std::isfinite (part.tEnd))
                return {};
            const auto &edge = roomEdges[part.roomEdge - 1];
            // Ребро зоны переносим с отметки пола на низ наклонного контакта.
            const double dz = OtdLinearSegment_GetZBottom (wall.geom) - roomBottom;
            const double ax = edge.c1.x + wall.rise_x * dz, ay = edge.c1.y + wall.rise_y * dz;
            const double bx = edge.c2.x + wall.rise_x * dz, by = edge.c2.y + wall.rise_y * dz;
            if (!onLine (ax, ay) || !onLine (bx, by))
                continue;
            // Sentinel-границы обрабатываем так же, как OtdWall_Create_FromWall.
            const double tBeg =
                std::fabs (part.tBeg) <= DistanceTolerance || std::fabs (part.tBeg - length) <= DistanceTolerance
                    ? 0
                    : part.tBeg;
            const double tEnd =
                std::fabs (part.tEnd) <= DistanceTolerance || std::fabs (part.tEnd - length) <= DistanceTolerance
                    ? length
                    : part.tEnd;
            const double a = along (wallBeg.x + ux * tBeg, wallBeg.y + uy * tBeg);
            const double b = along (wallBeg.x + ux * tEnd, wallBeg.y + uy * tEnd);
            const double begin = std::max (std::min (along (ax, ay), along (bx, by)), std::min (a, b));
            const double end = std::min (std::max (along (ax, ay), along (bx, by)), std::max (a, b));
            if (end - begin > DistanceTolerance)
                intervals.push_back ({begin, end});
        }
        if (!hasParts) {
            result.Push (wall);
            return result;
        }
        std::sort (intervals.begin (), intervals.end ());
        std::vector<std::pair<double, double>> merged;
        for (const auto &interval : intervals) {
            if (!merged.empty () && interval.first <= merged.back ().second + DistanceTolerance)
                merged.back ().second = std::max (merged.back ().second, interval.second);
            else
                merged.push_back (interval);
        }
        for (const auto &interval : merged) {
            double firstContact = 0, lastContact = span;
            for (const auto &other : contacts) {
                if (!other.surface_geometry || other.draw.base_guid != wall.draw.base_guid ||
                    other.draw.base_type != API_WallID || other.slab_step ||
                    std::min (OtdLinearSegment_GetZBottom (wall.geom) + wall.geom.height,
                              OtdLinearSegment_GetZBottom (other.geom) + other.geom.height) -
                            std::max (OtdLinearSegment_GetZBottom (wall.geom),
                                      OtdLinearSegment_GetZBottom (other.geom)) <=
                        DistanceTolerance ||
                    std::fabs (wall.rise_x - other.rise_x) > DistanceTolerance ||
                    std::fabs (wall.rise_y - other.rise_y) > DistanceTolerance)
                    continue;
                const double dz = OtdLinearSegment_GetZBottom (wall.geom) - OtdLinearSegment_GetZBottom (other.geom);
                const double ax = other.geom.seg.begC.x + other.rise_x * dz,
                             ay = other.geom.seg.begC.y + other.rise_y * dz;
                const double bx = other.geom.seg.endC.x + other.rise_x * dz,
                             by = other.geom.seg.endC.y + other.rise_y * dz;
                if (!onLine (ax, ay) || !onLine (bx, by) || (bx - ax) * vx + (by - ay) * vy <= DistanceTolerance)
                    continue;
                if (std::min (std::max (along (ax, ay), along (bx, by)), interval.second) -
                        std::max (std::min (along (ax, ay), along (bx, by)), interval.first) <=
                    DistanceTolerance)
                    continue;
                firstContact = std::min (firstContact, along (ax, ay));
                lastContact = std::max (lastContact, along (bx, by));
            }
            if (std::min (span, interval.second) - std::max (0.0, interval.first) <= DistanceTolerance)
                continue;
            const double begin = firstContact >= -DistanceTolerance ? interval.first : std::max (0.0, interval.first);
            const double end =
                lastContact <= span + DistanceTolerance ? interval.second : std::min (span, interval.second);
            if (end - begin < min_dim)
                return {};
            OtdWall trimmed = wall;
            trimmed.geom.seg.begC = {
                wall.geom.seg.begC.x + vx * begin, wall.geom.seg.begC.y + vy * begin, wall.geom.seg.begC.z};
            trimmed.geom.seg.endC = {
                wall.geom.seg.begC.x + vx * end, wall.geom.seg.begC.y + vy * end, wall.geom.seg.endC.z};
            trimmed.geom.length = end - begin;
            trimmed.openings.Clear ();
            for (const auto &op : wall.openings) {
                const double a = op.objLoc - op.width / 2, b = op.objLoc + op.width / 2;
                // Краевой вырез не закрываем при удлинении оболочки.
                const double left = std::max (a <= DistanceTolerance ? std::min (a, begin) : a, begin);
                const double right = std::min (b >= span - DistanceTolerance ? std::max (b, end) : b, end);
                if (right - left <= DistanceTolerance)
                    continue;
                if (right - left < min_dim)
                    return {};
                OtdOpening clipped = op;
                clipped.width = right - left;
                clipped.objLoc = (left + right) / 2 - begin;
                trimmed.openings.Push (std::move (clipped));
            }
            result.Push (std::move (trimmed));
        }
        return result;
    }

    bool SurfaceWalls_Build (const RoomSurfacePiece &piece, short floorInd, GS::Array<OtdWall> &walls) {
        OtdWall single;
        if (SurfaceWall_Build (piece, floorInd, single)) {
            walls.Push (std::move (single));
            return true;
        }
        if (!RoomSurfacePiece_IsValid (piece) || piece.contours[0].hole)
            return false;
        const double horizontal = std::hypot (piece.normal.x, piece.normal.y);
        if (horizontal <= DistanceTolerance)
            return false;
        const API_Coord3D along = {-piece.normal.y / horizontal, piece.normal.x / horizontal, 0};
        const double riseX = -piece.normal.z * piece.normal.x / (horizontal * horizontal);
        const double riseY = -piece.normal.z * piece.normal.y / (horizontal * horizontal);
        const auto origin = piece.contours[0].vertices[0];
        std::vector<std::vector<API_Coord>> contours;
        std::vector<double> levels;
        double perimeter = 0;
        double leftBound = 0, rightBound = 0;
        for (const auto &contour : piece.contours) {
            if (&contour != &piece.contours[0] && !contour.hole)
                return false;
            std::vector<API_Coord> points;
            for (const auto &p : RoomSurfaceContour_GetCorners (contour)) {
                const double x = Coord3D_Dot (Coord3D_Subtract (p, origin), along);
                points.push_back ({x, p.z});
                levels.push_back (p.z);
                leftBound = std::min (leftBound, x);
                rightBound = std::max (rightBound, x);
            }
            for (size_t i = 0; i < points.size (); ++i) {
                const auto &a = points[i], &b = points[(i + 1) % points.size ()];
                // Прямоугольный проём не заменяет диагональный край контакта.
                if (std::fabs (a.x - b.x) > DistanceTolerance && std::fabs (a.y - b.y) > DistanceTolerance)
                    return false;
                perimeter += std::hypot (a.x - b.x, (a.y - b.y) / horizontal);
            }
            contours.push_back (std::move (points));
        }
        std::sort (levels.begin (), levels.end ());
        levels.erase (std::unique (levels.begin (),
                                   levels.end (),
                                   [] (double a, double b) { return std::fabs (a - b) <= DistanceTolerance; }),
                      levels.end ());
        if (levels.size () < 2)
            return false;
        const auto point = [&] (double x, double z) -> API_Coord3D {
            return {
                origin.x + along.x * x + riseX * (z - origin.z), origin.y + along.y * x + riseY * (z - origin.z), z};
        };
        RoomSurfacePiece envelope = piece;
        envelope.contours.Clear ();
        RoomSurfacePieceContour outline;
        outline.vertices = {point (leftBound, levels.front ()),
                            point (rightBound, levels.front ()),
                            point (rightBound, levels.back ()),
                            point (leftBound, levels.back ())};
        envelope.contours.Push (std::move (outline));
        envelope.area = (rightBound - leftBound) * (levels.back () - levels.front ()) / horizontal;
        if (!SurfaceWall_Build (envelope, floorInd, single))
            return false;

        struct Cutout {
            double left, right, bottom, top;
        };

        std::vector<Cutout> cutouts;
        double area = 0;
        for (size_t band = 1; band < levels.size (); ++band) {
            const double lower = levels[band - 1], upper = levels[band];
            const double middle = (lower + upper) / 2;
            std::vector<double> crossings;
            for (const auto &contour : contours) {
                for (size_t i = 0; i < contour.size (); ++i) {
                    const auto &a = contour[i], &b = contour[(i + 1) % contour.size ()];
                    if ((a.y > middle) != (b.y > middle))
                        crossings.push_back (a.x + (b.x - a.x) * (middle - a.y) / (b.y - a.y));
                }
            }
            std::sort (crossings.begin (), crossings.end ());
            if (crossings.size () % 2 != 0)
                return false;
            // Разлагаем только пустоту внутри оболочки; материал остаётся одной стеной.
            double previous = leftBound;
            for (size_t i = 0; i <= crossings.size (); i += 2) {
                const double next = i < crossings.size () ? crossings[i] : rightBound;
                if (next - previous > DistanceTolerance) {
                    bool merged = false;
                    for (auto &cutout : cutouts) {
                        if (std::fabs (cutout.top - lower) <= DistanceTolerance &&
                            std::fabs (cutout.left - previous) <= DistanceTolerance &&
                            std::fabs (cutout.right - next) <= DistanceTolerance) {
                            cutout.top = upper;
                            merged = true;
                            break;
                        }
                    }
                    if (!merged)
                        cutouts.push_back ({previous, next, lower, upper});
                }
                if (i < crossings.size ()) {
                    area += (crossings[i + 1] - crossings[i]) * (upper - lower) / horizontal;
                    previous = crossings[i + 1];
                }
            }
        }
        if (std::fabs (area - piece.area) > std::max (AreaTolerance, 2 * perimeter * DistanceTolerance))
            return false;
        const double wallLength = rightBound - leftBound;
        const bool reversed = (single.geom.seg.endC.x - single.geom.seg.begC.x) * along.x +
                                  (single.geom.seg.endC.y - single.geom.seg.begC.y) * along.y <
                              0;
        for (const auto &cutout : cutouts) {
            // Не допускам молчаливого заполнения выреза из-за порога штатного writer.
            if (cutout.right - cutout.left <= min_dim || cutout.top - cutout.bottom <= min_dim)
                return false;
            OtdOpening opening;
            opening.width = cutout.right - cutout.left;
            opening.height = cutout.top - cutout.bottom;
            opening.objLoc = (cutout.left + cutout.right) / 2 - leftBound;
            opening.lower = cutout.bottom - OtdLinearSegment_GetZBottom (single.geom);
            OtdOpening_Add (
                opening, reversed, OtdLinearSegment_GetZBottom (single.geom), 0, wallLength, wallLength, single);
            single.openings.GetLast ().lower = opening.lower;
            single.openings.GetLast ().surface_geometry = true;
        }
        if (std::fabs (OtdWall_GetArea (single) - piece.area) >
            std::max (AreaTolerance, 2 * perimeter * DistanceTolerance))
            return false;
        walls.Push (std::move (single));
        return true;
    }

    bool SurfaceOpenings_Link (OtdWall &wall, const GS::Array<OtdWall> &openingWalls, bool analyticOverlap) {
        if (!wall.surface_geometry || wall.openings.IsEmpty ())
            return true;
        const double length =
            std::hypot (wall.geom.seg.endC.x - wall.geom.seg.begC.x, wall.geom.seg.endC.y - wall.geom.seg.begC.y);
        if (length <= min_dim)
            return false;
        const double ux = (wall.geom.seg.endC.x - wall.geom.seg.begC.x) / length;
        const double uy = (wall.geom.seg.endC.y - wall.geom.seg.begC.y) / length;

        struct SourceOpening {
            double left, right, bottom, top;
            OtdOpening opening;
        };

        std::vector<SourceOpening> sources;
        for (const auto &sourceWall : openingWalls) {
            if (sourceWall.draw.base_guid != wall.draw.base_guid)
                continue;
            const double dx = sourceWall.geom.seg.endC.x - sourceWall.geom.seg.begC.x;
            const double dy = sourceWall.geom.seg.endC.y - sourceWall.geom.seg.begC.y;
            const double sourceLength = std::hypot (dx, dy);
            if (sourceLength <= min_dim || std::fabs (dx * uy - dy * ux) > DistanceTolerance * sourceLength)
                continue;
            for (const auto &opening : sourceWall.openings) {
                if (opening.base_guid == APINULLGuid)
                    continue;
                const double x = sourceWall.geom.seg.begC.x + dx * opening.objLoc / sourceLength;
                const double y = sourceWall.geom.seg.begC.y + dy * opening.objLoc / sourceLength;
                const double center = (x - wall.geom.seg.begC.x) * ux + (y - wall.geom.seg.begC.y) * uy;
                sources.push_back ({center - opening.width / 2,
                                    center + opening.width / 2,
                                    opening.zBottom,
                                    opening.zBottom + opening.height,
                                    opening});
            }
        }
        GS::Array<OtdOpening> linked;
        for (const auto &cutout : wall.openings) {
            const double left = cutout.objLoc - cutout.width / 2, right = cutout.objLoc + cutout.width / 2;
            const double bottom = cutout.zBottom, top = cutout.zBottom + cutout.height;
            std::vector<double> xs = {left, right}, zs = {bottom, top};
            for (const auto &source : sources) {
                if (std::min (right, source.right) - std::max (left, source.left) <= DistanceTolerance ||
                    std::min (top, source.top) - std::max (bottom, source.bottom) <= DistanceTolerance)
                    continue;
                xs.push_back (std::max (left, source.left));
                xs.push_back (std::min (right, source.right));
                zs.push_back (std::max (bottom, source.bottom));
                zs.push_back (std::min (top, source.top));
            }
            const auto normalize = [] (std::vector<double> &values) {
                std::sort (values.begin (), values.end ());
                values.erase (std::unique (values.begin (),
                                           values.end (),
                                           [] (double a, double b) { return std::fabs (a - b) <= DistanceTolerance; }),
                              values.end ());
            };
            normalize (xs);
            normalize (zs);
            for (size_t z = 1; z < zs.size (); ++z) {
                for (size_t x = 1; x < xs.size (); ++x) {
                    if (xs[x] - xs[x - 1] <= min_dim || zs[z] - zs[z - 1] <= min_dim)
                        return false;
                    const double centerX = (xs[x] + xs[x - 1]) / 2, centerZ = (zs[z] + zs[z - 1]) / 2;
                    const SourceOpening *owner = nullptr;
                    GS::Array<API_Guid> sourceGuids;
                    for (const auto &source : sources) {
                        if (centerX < source.left - DistanceTolerance || centerX > source.right + DistanceTolerance ||
                            centerZ < source.bottom - DistanceTolerance || centerZ > source.top + DistanceTolerance)
                            continue;
                        if (owner != nullptr && owner->opening.base_guid != source.opening.base_guid &&
                            !analyticOverlap)
                            return false;
                        if (!sourceGuids.Contains (source.opening.base_guid))
                            sourceGuids.Push (source.opening.base_guid);
                        owner = &source;
                    }
                    OtdOpening opening = cutout;
                    opening.objLoc = centerX;
                    opening.width = xs[x] - xs[x - 1];
                    opening.zBottom = zs[z - 1];
                    opening.height = zs[z] - zs[z - 1];
                    opening.lower = opening.zBottom - OtdLinearSegment_GetZBottom (wall.geom);
                    opening.source_opening_guids = sourceGuids;
                    opening.base_guid = sourceGuids.GetSize () == 1 ? sourceGuids[0] : APINULLGuid;
                    // Только привязка к реальному проёму; откосы создаются отдельным legacy-путём.
                    opening.has_reveal = false;
                    opening.surface_geometry = true;
                    linked.Push (std::move (opening));
                }
            }
        }
        double originalArea = 0, linkedArea = 0;
        for (const auto &opening : wall.openings)
            originalArea += opening.width * opening.height;
        for (const auto &opening : linked)
            linkedArea += opening.width * opening.height;
        if (std::fabs (originalArea - linkedArea) >
            std::max (AreaTolerance, 2 * (length + wall.geom.height) * DistanceTolerance))
            return false;
        wall.openings = std::move (linked);
        return true;
    }

    bool SurfaceWallPieces_Group (const GS::Array<RoomSurfacePiece> &pieces,
                                  short floorInd,
                                  GS::Array<RoomSurfacePiece> &grouped) {
        using Polygons = Geometry::MultiPolygon2D;
        (void)floorInd;
        const auto sameFace = [] (const RoomSurfaceSourceFace &a, const RoomSurfaceSourceFace &b) {
            return a.baseGuid == b.baseGuid && a.baseType == b.baseType && a.geometryGuid == b.geometryGuid &&
                   a.bodyIndex == b.bodyIndex && a.polygonIndex == b.polygonIndex &&
                   a.modelMaterialIndex == b.modelMaterialIndex && a.origin == b.origin &&
                   a.analyticFaceId == b.analyticFaceId && a.preparedWallIndex == b.preparedWallIndex;
        };
        // Сравниваем полный набор граней взаимно однозначно: одинакового GUID основания недостаточно.
        const auto sameSource = [&] (const RoomSurfacePiece &a, const RoomSurfacePiece &b) {
            if (a.baseGuid != b.baseGuid || a.baseType != b.baseType || a.zoneBodyIndex != b.zoneBodyIndex ||
                a.sources.GetSize () != b.sources.GetSize ())
                return false;
            std::vector<bool> matched (b.sources.GetSize (), false);
            for (const auto &source : a.sources) {
                bool found = false;
                for (UIndex i = 0; i < b.sources.GetSize (); ++i) {
                    if (!matched[i] && sameFace (source, b.sources[i])) {
                        matched[i] = true;
                        found = true;
                        break;
                    }
                }
                if (!found)
                    return false;
            }
            return true;
        };
        // Объединяем до проверки представимости: диагональный кусок не должен исчезать
        // из общего контура и оставлять случайную прямоугольную часть той же поверхности.
        std::vector<const RoomSurfacePiece *> candidates;
        GS::Array<RoomSurfacePiece> results;
        for (const auto &piece : pieces) {
            if (piece.status == RoomSurfacePieceStatus::Matched && piece.baseGuid != APINULLGuid &&
                piece.baseType == API_WallID && !piece.sources.IsEmpty () && RoomSurfacePiece_IsValid (piece) &&
                std::hypot (piece.normal.x, piece.normal.y) > DistanceTolerance)
                candidates.push_back (&piece);
            else
                results.Push (piece);
        }
        // Опорная плоскость не зависит от порядка входа; кластер не растёт цепочкой близости.
        std::sort (candidates.begin (), candidates.end (), [] (const auto *a, const auto *b) {
            if (a->normal.x != b->normal.x)
                return a->normal.x < b->normal.x;
            if (a->normal.y != b->normal.y)
                return a->normal.y < b->normal.y;
            if (a->normal.z != b->normal.z)
                return a->normal.z < b->normal.z;
            if (a->planeOffset != b->planeOffset)
                return a->planeOffset < b->planeOffset;
            return a->area < b->area;
        });
        std::vector<bool> used (candidates.size (), false);
        for (size_t first = 0; first < candidates.size (); ++first) {
            if (used[first])
                continue;
            const auto &reference = *candidates[first];
            const double normalLength = Coord3D_GetLength (reference.normal);
            const API_Coord3D normal = {reference.normal.x / normalLength,
                                        reference.normal.y / normalLength,
                                        reference.normal.z / normalLength};
            const double offset = reference.planeOffset / normalLength;
            const double horizontal = std::hypot (normal.x, normal.y);
            const API_Coord3D along = {-normal.y / horizontal, normal.x / horizontal, 0};
            // Начало на отметке ноль вычисляем из плоскости, а не из первой вершины контура.
            // Вторая координата — абсолютная Z; площадь в этих координатах делится на horizontal.
            const API_Coord3D origin = {
                -offset * normal.x / (horizontal * horizontal), -offset * normal.y / (horizontal * horizontal), 0};
            const API_Coord3D rise = {
                -normal.z * normal.x / (horizontal * horizontal), -normal.z * normal.y / (horizontal * horizontal), 1};
            const auto restore = [&] (const Point2D &p) -> API_Coord3D {
                return {origin.x + along.x * p.x + rise.x * p.y, origin.y + along.y * p.x + rise.y * p.y, p.y};
            };
            const auto project = [&] (const API_Coord3D &p) -> Point2D {
                return {Coord3D_Dot (Coord3D_Subtract (p, origin), along), p.z};
            };
            std::vector<size_t> members;
            for (size_t index = first; index < candidates.size (); ++index) {
                if (used[index] || !sameSource (reference, *candidates[index]) ||
                    Coord3D_GetLength (Coord3D_Subtract (normal, candidates[index]->normal)) > DistanceTolerance)
                    continue;
                bool coplanar = true;
                for (const auto &contour : candidates[index]->contours)
                    for (const auto &point : contour.vertices)
                        coplanar = coplanar && std::fabs (Coord3D_Dot (normal, point) + offset) <= DistanceTolerance &&
                                   Coord3D_AreNear (restore (project (point)), point);
                if (coplanar)
                    members.push_back (index);
            }
            if (members.empty ())
                return false;
            if (members.size () == 1) {
                used[first] = true;
                results.Push (reference);
                continue;
            }
            Polygons input;
            double perimeter = 0;
            for (size_t index : members) {
                GS::Array<Point2D> coords;
                GS::Array<PolyArcRec> arcs;
                GS::Array<UIndex> ends;
                for (const auto &contour : candidates[index]->contours) {
                    const auto corners = RoomSurfaceContour_GetCorners (contour);
                    for (size_t vertex = 0; vertex < corners.size (); ++vertex) {
                        coords.Push (project (corners[vertex]));
                        perimeter += Coord3D_GetLength (
                            Coord3D_Subtract (corners[vertex], corners[(vertex + 1) % corners.size ()]));
                    }
                    ends.Push (coords.GetSize () - 1);
                }
                const auto polygons = Geometry::Polygon2D::Create (
                    coords, arcs, ends, Geometry::PolyCreateFlags_IdxRefInContourEndFromZero);
                if (polygons.IsEmpty () || std::fabs (polygons.CalcArea () / horizontal - candidates[index]->area) >
                                               std::max (AreaTolerance, 2 * perimeter * DistanceTolerance))
                    return false;
                for (const auto &polygon : polygons)
                    input.Push (polygon);
            }
            // Union убирает внутренние швы и дубли перекрытия до общего разложения по уровням.
            // Чётность применяется внутри одного контура, но не к перекрывающимся исходным кускам.
            Polygons united = input;
            united.Unify ();
            const double areaError = std::max (AreaTolerance, 2 * perimeter * DistanceTolerance);
            // Обе разности проверяют реальное покрытие, не сумму площадей пересекающихся частей.
            if (united.IsEmpty () || Polygons::Substract (input, united).CalcArea () / horizontal > areaError ||
                Polygons::Substract (united, input).CalcArea () / horizontal > areaError)
                return false;
            GS::Array<RoomSurfacePiece> merged;
            for (const auto &polygon : united) {
                RoomSurfacePiece piece = reference;
                piece.normal = normal;
                piece.planeOffset = offset;
                piece.area = polygon.CalcArea () / horizontal;
                piece.contours.Clear ();
                GS::Array<Point2D> coords;
                GS::Array<PolyArcRec> arcs;
                GS::Array<UIndex> ends;
                GS::Array<Geometry::Polygon2D::ConstContourIterator> reversed;
                polygon.CopyPolygonData (
                    coords, arcs, ends, reversed, Geometry::PolyCreateFlags_IdxRefInContourEndFromZero);
                UIndex start = 0;
                for (UIndex contourIndex = 0; contourIndex < ends.GetSize (); ++contourIndex) {
                    RoomSurfacePieceContour contour;
                    contour.hole = contourIndex != 0;
                    for (UIndex vertex = start; vertex <= ends[contourIndex]; ++vertex)
                        contour.vertices.Push (restore (coords[vertex]));
                    piece.contours.Push (std::move (contour));
                    start = ends[contourIndex] + 1;
                }
                merged.Push (std::move (piece));
            }
            results.Append (merged);
            for (size_t index : members)
                used[index] = true;
        }
        // До этой точки grouped не меняется: ошибочное объединение не оставляет частичного результата.
        grouped.Append (results);
        return true;
    }

    bool SlabContacts_DeduplicateAccepted (OtdRoom &room) {
        using Polygons = Geometry::MultiPolygon2D;
        // Условия включения совпадают с OtdRooms_Draw; Floor/Ceil-боковины идут стеновым writer.
        const auto enabled = [&] (const OtdWall &w) {
            if (!room.isValid || !w.isValid ||
                !(room.create_wall_elements || room.create_column_elements || room.create_reveal_elements))
                return false;
            if ((w.draw.type == Wall_Main || w.draw.type == Wall_Up || w.draw.type == Wall_Down) &&
                !room.create_wall_elements)
                return false;
            if (w.draw.type == Column && !room.create_column_elements)
                return false;
            if ((w.draw.type == Reveal_Main || w.draw.type == Reveal_Up || w.draw.type == Reveal_Down) &&
                !room.create_reveal_elements)
                return false;
            return w.surface_geometry && w.draw.draw_type == API_WallID &&
                   (!w.slab_step || w.slab_step_writer_eligible);
        };
        GS::Array<OtdWall> output;
        bool complete = true;
        for (const auto &wall : room.otdwall) {
            if (wall.draw.base_type != API_SlabID || wall.slab_step || !enabled (wall)) {
                output.Push (wall);
                continue;
            }
            const double length =
                std::hypot (wall.geom.seg.endC.x - wall.geom.seg.begC.x, wall.geom.seg.endC.y - wall.geom.seg.begC.y);
            if (!std::isfinite (length) || length <= min_dim || !std::isfinite (wall.geom.height) ||
                wall.geom.height <= min_dim || !std::isfinite (OtdLinearSegment_GetZBottom (wall.geom)) ||
                !std::isfinite (wall.rise_x) || !std::isfinite (wall.rise_y)) {
                output.Push (wall);
                complete = false;
                continue;
            }
            const double ux = (wall.geom.seg.endC.x - wall.geom.seg.begC.x) / length,
                         uy = (wall.geom.seg.endC.y - wall.geom.seg.begC.y) / length;
            API_Coord3D normal = {uy, -ux, -(uy * wall.rise_x - ux * wall.rise_y)};
            const double norm = Coord3D_GetLength (normal);
            normal = {normal.x / norm, normal.y / norm, normal.z / norm};
            const auto point = [&] (double x, double z) -> API_Coord3D {
                return {wall.geom.seg.begC.x + ux * x + wall.rise_x * (z - OtdLinearSegment_GetZBottom (wall.geom)),
                        wall.geom.seg.begC.y + uy * x + wall.rise_y * (z - OtdLinearSegment_GetZBottom (wall.geom)),
                        z};
            };
            const auto rectangle = [] (double left, double right, double bottom, double top) {
                GS::Array<Point2D> coords = {{left, bottom}, {right, bottom}, {right, top}, {left, top}};
                GS::Array<PolyArcRec> arcs;
                GS::Array<UIndex> ends = {3};
                return Geometry::Polygon2D::Create (
                    coords, arcs, ends, Geometry::PolyCreateFlags_IdxRefInContourEndFromZero);
            };
            const auto coverage = [&] (const OtdWall &w, Polygons &polys) {
                const double l =
                    std::hypot (w.geom.seg.endC.x - w.geom.seg.begC.x, w.geom.seg.endC.y - w.geom.seg.begC.y);
                if (!std::isfinite (l) || l <= min_dim || !std::isfinite (w.geom.height) || w.geom.height <= min_dim ||
                    !std::isfinite (OtdLinearSegment_GetZBottom (w.geom)) || !std::isfinite (w.rise_x) ||
                    !std::isfinite (w.rise_y))
                    return false;
                const double dx = (w.geom.seg.endC.x - w.geom.seg.begC.x) / l,
                             dy = (w.geom.seg.endC.y - w.geom.seg.begC.y) / l;
                if (std::fabs (dx * uy - dy * ux) > DistanceTolerance ||
                    std::fabs (w.rise_x - wall.rise_x) > DistanceTolerance ||
                    std::fabs (w.rise_y - wall.rise_y) > DistanceTolerance)
                    return false;
                const double x =
                    (w.geom.seg.begC.x - wall.geom.seg.begC.x) * ux + (w.geom.seg.begC.y - wall.geom.seg.begC.y) * uy;
                const auto restored = point (x, OtdLinearSegment_GetZBottom (w.geom));
                if (!Coord3D_AreNear (restored,
                                      {w.geom.seg.begC.x, w.geom.seg.begC.y, OtdLinearSegment_GetZBottom (w.geom)}))
                    return false;
                const double direction = dx * ux + dy * uy;
                polys = rectangle (std::min (x, x + direction * l),
                                   std::max (x, x + direction * l),
                                   OtdLinearSegment_GetZBottom (w.geom),
                                   OtdLinearSegment_GetZBottom (w.geom) + w.geom.height);
                for (const auto &opening : w.openings) {
                    if (!std::isfinite (opening.objLoc) || !std::isfinite (opening.width) ||
                        !std::isfinite (opening.zBottom) || !std::isfinite (opening.height) ||
                        opening.width <= min_dim || opening.height <= min_dim)
                        return false;
                    const double center = x + direction * opening.objLoc;
                    polys = Polygons::Substract (polys,
                                                 rectangle (center - opening.width / 2,
                                                            center + opening.width / 2,
                                                            opening.zBottom,
                                                            opening.zBottom + opening.height));
                }
                return true;
            };
            Polygons original;
            if (!coverage (wall, original)) {
                output.Push (wall);
                complete = false;
                continue;
            }
            Polygons remainder = original;
            for (const auto &step : room.otdwall) {
                if (!step.slab_step || step.draw.base_guid != wall.draw.base_guid ||
                    step.draw.base_guid == APINULLGuid || step.draw.base_type != API_SlabID || !enabled (step))
                    continue;
                Polygons accepted;
                if (coverage (step, accepted))
                    remainder = Polygons::Substract (remainder, accepted);
            }
            if (Polygons::Substract (original, remainder).IsEmpty ()) {
                output.Push (wall);
                continue;
            }
            GS::Array<OtdWall> fragments;
            bool representable = true;
            // Реальная ownership проёма не теряется при смене разложения: такой случай оставляем явно неоднозначным.
            for (const auto &opening : wall.openings)
                if (opening.base_guid != APINULLGuid)
                    representable = false;
            Polygons rebuilt;
            for (const auto &polygon : remainder) {
                RoomSurfacePiece piece;
                piece.baseGuid = wall.draw.base_guid;
                piece.baseType = wall.draw.base_type;
                piece.normal = normal;
                piece.planeOffset = -Coord3D_Dot (normal, point (0, OtdLinearSegment_GetZBottom (wall.geom)));
                piece.area = polygon.CalcArea () * norm;
                GS::Array<Point2D> coords;
                GS::Array<PolyArcRec> arcs;
                GS::Array<UIndex> ends;
                GS::Array<Geometry::Polygon2D::ConstContourIterator> reversed;
                polygon.CopyPolygonData (
                    coords, arcs, ends, reversed, Geometry::PolyCreateFlags_IdxRefInContourEndFromZero);
                if (!arcs.IsEmpty ()) {
                    representable = false;
                    break;
                }
                UIndex first = 0;
                for (UIndex i = 0; i < ends.GetSize (); ++i) {
                    RoomSurfacePieceContour contour;
                    contour.hole = i != 0;
                    for (UIndex j = first; j <= ends[i]; ++j)
                        contour.vertices.Push (point (coords[j].x, coords[j].y));
                    piece.contours.Push (contour);
                    first = ends[i] + 1;
                }
                GS::Array<OtdWall> adapted;
                if (!SurfaceWalls_Build (piece, wall.geom.floorInd, adapted)) {
                    representable = false;
                    break;
                }
                for (const auto &geometry : adapted) {
                    // Копируем принятые правила, меняя только конечное покрытие; правила повторно не вызываются.
                    OtdWall fragment = wall;
                    fragment.geom.seg.begC = geometry.geom.seg.begC;
                    fragment.geom.seg.endC = geometry.geom.seg.endC;
                    fragment.geom.height = geometry.geom.height;
                    fragment.geom.length = geometry.geom.length;
                    fragment.openings = geometry.openings;
                    Polygons checked;
                    if (!coverage (fragment, checked)) {
                        representable = false;
                        break;
                    }
                    for (const auto &part : checked)
                        rebuilt.Push (part);
                    fragments.Push (fragment);
                }
            }
            // Проверяем обе направленные разности конечных контуров, а не только сумму площадей.
            representable = representable &&
                            Polygons::Substract (remainder, rebuilt).CalcArea () * norm <= AreaTolerance &&
                            Polygons::Substract (rebuilt, remainder).CalcArea () * norm <= AreaTolerance;
            if (representable)
                output.Append (fragments);
            else {
                output.Push (wall);
                complete = false;
            }
        }
        room.otdwall = std::move (output);
        return complete;
    }

    bool SurfaceSlabs_Build (const RoomSurfacePiece &piece, short floorInd, GS::Array<OtdSlab> &slabs) {
        if (!RoomSurfacePiece_IsValid (piece) || std::hypot (piece.normal.x, piece.normal.y) > DistanceTolerance ||
            std::fabs (std::fabs (piece.normal.z) - 1) > DistanceTolerance || piece.contours[0].hole)
            return false;
        const double elevation = piece.contours[0].vertices[0].z;
        GS::Array<Point2D> coords;
        GS::Array<UIndex> ends;
        GS::Array<PolyArcRec> arcs;
        double expectedArea = 0;
        bool firstContour = true;
        for (const auto &contour : piece.contours) {
            // Перегрузка Create с концами контуров принимает внешний контур и отверстия.
            if (contour.hole == firstContour)
                return false;
            firstContour = false;
            // Сохраняем все XY-вершины; регуляризацию контура выполняет Polygon2D.
            std::vector<API_Coord3D> points;
            for (const auto &point : contour.vertices)
                points.push_back (point);
            if (points.size () > 1 && points.front ().x == points.back ().x && points.front ().y == points.back ().y &&
                points.front ().z == points.back ().z)
                points.pop_back ();
            if (points.size () < 3)
                return false;
            double twiceArea = 0;
            const auto origin = points.front ();
            for (size_t i = 0; i < points.size (); ++i) {
                const auto &p = points[i], &next = points[(i + 1) % points.size ()];
                if (std::fabs (p.z - elevation) > DistanceTolerance)
                    return false;
                twiceArea += (p.x - origin.x) * (next.y - origin.y) - (next.x - origin.x) * (p.y - origin.y);
                coords.Push ({p.x, p.y});
            }
            if (!std::isfinite (twiceArea) || std::fabs (twiceArea) <= 2 * AreaTolerance)
                return false;
            expectedArea += (contour.hole ? -1 : 1) * std::fabs (twiceArea) / 2;
            ends.Push (coords.GetSize () - 1);
        }
        if (expectedArea <= AreaTolerance || std::fabs (expectedArea - piece.area) > AreaTolerance)
            return false;
        const Geometry::MultiPolygon2D polygons =
            Geometry::Polygon2D::Create (coords, arcs, ends, Geometry::PolyCreateFlags_IdxRefInContourEndFromZero);
        if (polygons.IsEmpty () || std::fabs (polygons.CalcArea () - expectedArea) > AreaTolerance)
            return false;
        GS::Array<OtdSlab> results;
        for (const auto &polygon : polygons) {
            OtdSlab slab;
            slab.poly = polygon;
            slab.zBottom = elevation;
            slab.floorInd = floorInd;
            slab.draw.base_guid = piece.baseGuid;
            slab.draw.base_type = piece.baseType;
            slab.draw.draw_type = API_SlabID;
            slab.draw.type = piece.normal.z < 0 ? Floor : Ceil;
            slab.surface_geometry = true;
            results.Push (slab);
        }
        slabs.Append (results);
        return true;
    }

    namespace {
        using FloorPolygons = Geometry::MultiPolygon2D;

        double FloorPlane_GetHeight (const OtdSlab &plane, const Point2D &point) {
            return plane.zBottom + plane.slope_x * (point.x - plane.plane_origin.x) +
                   plane.slope_y * (point.y - plane.plane_origin.y);
        }

        bool FloorFace_Project (const RoomSurface &face, OtdSlab &plane, FloorPolygons &polygons) {
            if (!Coord3D_IsFinite (face.normalWorld) || !std::isfinite (face.planeOffset) || face.contours.IsEmpty () ||
                face.contours[0].vertices.IsEmpty () || std::fabs (face.normalWorld.z) <= DistanceTolerance)
                return false;
            const double length = Coord3D_GetLength (face.normalWorld);
            const auto origin = face.contours[0].vertices[0].world;
            if (!Coord3D_IsFinite (origin) ||
                std::fabs (Coord3D_Dot (face.normalWorld, origin) + face.planeOffset) / length > DistanceTolerance)
                return false;
            plane.plane_origin = {origin.x, origin.y};
            plane.zBottom = origin.z;
            plane.reference_z = origin.z;
            plane.slope_x = -face.normalWorld.x / face.normalWorld.z;
            plane.slope_y = -face.normalWorld.y / face.normalWorld.z;
            RoomSurfacePiece piece;
            piece.normal = {0, 0, -1};
            for (const auto &contour : face.contours) {
                if (contour.vertices.GetSize () < 3)
                    return false;
                RoomSurfacePieceContour projected;
                projected.hole = contour.hole;
                double twiceArea = 0;
                for (UIndex i = 0; i < contour.vertices.GetSize (); ++i) {
                    const auto &point = contour.vertices[i].world;
                    const auto &next = contour.vertices[(i + 1) % contour.vertices.GetSize ()].world;
                    if (!Coord3D_IsFinite (point) ||
                        std::fabs (Coord3D_Dot (face.normalWorld, Coord3D_Subtract (point, origin))) / length >
                            DistanceTolerance)
                        return false;
                    projected.vertices.Push ({point.x, point.y, 0});
                    twiceArea +=
                        (point.x - origin.x) * (next.y - origin.y) - (next.x - origin.x) * (point.y - origin.y);
                }
                piece.area += (contour.hole ? -1 : 1) * std::fabs (twiceArea) / 2;
                piece.contours.Push (projected);
            }
            if (!std::isfinite (piece.area) || piece.area < 0)
                return false;
            if (piece.area == 0)
                return true;
            GS::Array<OtdSlab> projected;
            if (!SurfaceSlabs_Build (piece, 0, projected))
                return false;
            for (const auto &part : projected)
                polygons.Push (part.poly);
            return true;
        }

        // Нормализуем коэффициенты только после проверки константной/касательной отсечки.
        bool FloorPolygons_ClipBelow (const FloorPolygons &input,
                                      const OtdSlab &lower,
                                      const OtdSlab &upper,
                                      FloorPolygons &output) {
            const double a = lower.slope_x - upper.slope_x, b = lower.slope_y - upper.slope_y;
            const double length = std::hypot (a, b);
            for (const auto &polygon : input) {
                GS::Array<Point2D> coords;
                GS::Array<PolyArcRec> arcs;
                GS::Array<UIndex> ends;
                GS::Array<Geometry::Polygon2D::ConstContourIterator> reversed;
                polygon.CopyPolygonData (
                    coords, arcs, ends, reversed, Geometry::PolyCreateFlags_IdxRefInContourEndFromZero);
                if (!arcs.IsEmpty () || coords.IsEmpty ())
                    return false;
                double minimum = std::numeric_limits<double>::infinity (), maximum = -minimum;
                for (const auto &point : coords) {
                    const double difference = FloorPlane_GetHeight (lower, point) - FloorPlane_GetHeight (upper, point);
                    if (!std::isfinite (difference))
                        return false;
                    minimum = std::min (minimum, difference);
                    maximum = std::max (maximum, difference);
                }
                if (maximum <= 0) {
                    output.Push (polygon);
                    continue;
                }
                if (minimum >= 0)
                    continue;
                if (!std::isfinite (length) || length == 0)
                    return false;
                const double c = FloorPlane_GetHeight (upper, {lower.plane_origin.x, lower.plane_origin.y}) -
                                 lower.zBottom + a * lower.plane_origin.x + b * lower.plane_origin.y;
                bool inside = false, degenerate = false;
                GS::Int32 warning = 0;
                const auto clipped =
                    polygon.Cut (HalfPlane2DData (a / length, b / length, c / length), &inside, &warning, &degenerate);
                if (warning != 0 || degenerate)
                    return false;
                output.Append (clipped);
            }
            return true;
        }

        bool FloorPlanes_AreSame (const OtdSlab &a, const OtdSlab &b, const FloorPolygons &domain) {
            const double na = std::sqrt (1 + a.slope_x * a.slope_x + a.slope_y * a.slope_y);
            const double nb = std::sqrt (1 + b.slope_x * b.slope_x + b.slope_y * b.slope_y);
            if (Coord3D_GetLength (
                    {a.slope_x / na - b.slope_x / nb, a.slope_y / na - b.slope_y / nb, 1 / na - 1 / nb}) >
                DistanceTolerance)
                return false;
            for (const auto &polygon : domain) {
                GS::Array<Point2D> coords;
                GS::Array<PolyArcRec> arcs;
                GS::Array<UIndex> ends;
                GS::Array<Geometry::Polygon2D::ConstContourIterator> reversed;
                polygon.CopyPolygonData (
                    coords, arcs, ends, reversed, Geometry::PolyCreateFlags_IdxRefInContourEndFromZero);
                if (!arcs.IsEmpty ())
                    return false;
                for (const auto &point : coords)
                    if (std::fabs (FloorPlane_GetHeight (a, point) - FloorPlane_GetHeight (b, point)) >
                        DistanceTolerance)
                        return false;
            }
            return true;
        }
    } // namespace

    static bool MeshFloorFinishes_BuildProjectedImpl (const Geometry::Polygon2D &footprint,
                                                      double roomFloor,
                                                      short floorInd,
                                                      const GS::Array<MeshFloorSource> &meshes,
                                                      const GS::Array<OtdSlab> &slabSupports,
                                                      GS::Array<OtdSlab> &floors,
                                                      Geometry::MultiPolygon2D &meshCoverage,
                                                      MeshFloorProvenance *provenance) {
        MeshFloorProvenance staged = provenance != nullptr ? *provenance : MeshFloorProvenance{};
        if (meshes.IsEmpty ()) {
            if (provenance != nullptr)
                *provenance = std::move (staged);
            meshCoverage.Clear ();
            return true;
        }
        if (footprint.IsEmpty () || !std::isfinite (roomFloor))
            return false;

        struct Patch {
            OtdSlab plane;
            FloorPolygons domain;
            std::vector<OtdSlab> members;
            std::vector<UIndex> supports;
        };

        std::vector<Patch> patches;
        FloorPolygons domain;
        OtdSlab level;
        level.zBottom = roomFloor;
        for (const auto &mesh : meshes) {
            const auto &source = mesh.surface;
            if (source.baseType != API_MeshID || source.baseGuid == APINULLGuid ||
                source.geometryGuid != source.baseGuid || source.snapshot.guid != source.baseGuid ||
                !source.snapshot.IsComplete ())
                return false;
            for (const auto &body : source.snapshot.bodies) {
                FloorPolygons bottom;
                bool hasBottom = false;
                if (mesh.hasBottom)
                    for (const auto &face : body.surfaces) {
                        if (!Coord3D_IsFinite (face.normalWorld))
                            return false;
                        if (face.normalWorld.z >= -DistanceTolerance)
                            continue;
                        hasBottom = true;
                        OtdSlab plane;
                        FloorPolygons projected;
                        if (!FloorFace_Project (face, plane, projected) ||
                            !FloorPolygons_ClipBelow (projected, plane, level, bottom))
                            return false;
                    }
                bottom.Unify ();
                for (const auto &face : body.surfaces) {
                    if (!Coord3D_IsFinite (face.normalWorld))
                        return false;
                    if (face.normalWorld.z <= DistanceTolerance)
                        continue;
                    Patch patch;
                    if (!FloorFace_Project (face, patch.plane, patch.domain))
                        return false;
                    // Сохраняем исходную и допустимую опору отдельно от room clipping.
                    if (provenance != nullptr) {
                        MeshFloorFiniteSupport support;
                        support.ownerGuid = source.baseGuid;
                        support.geometryGuid = source.geometryGuid;
                        support.ownerType = source.baseType;
                        support.bodyIndex = face.bodyIndex;
                        support.polygonIndex = face.polygonIndex;
                        support.modelMaterialIndex = face.modelMaterialIndex;
                        support.plane = patch.plane;
                        support.plane.draw.base_guid = source.baseGuid;
                        support.plane.draw.base_type = API_MeshID;
                        support.plane.draw.draw_type = API_RoofID;
                        support.plane.draw.type = Floor;
                        support.plane.floorInd = floorInd;
                        support.plane.surface_geometry = true;
                        support.projected = patch.domain;
                        if (mesh.hasBottom) {
                            if (!hasBottom)
                                return false;
                            support.admitted = FloorPolygons::Intersect (support.projected, bottom);
                        } else if (!FloorPolygons_ClipBelow (support.projected, patch.plane, level, support.admitted))
                            return false;
                        patch.supports.push_back (staged.supports.GetSize ());
                        staged.supports.Push (support);
                    }
                    FloorPolygons room;
                    room.Push (footprint);
                    patch.domain = FloorPolygons::Intersect (patch.domain, room);
                    if (patch.domain.IsEmpty ())
                        continue;
                    if (mesh.hasBottom) {
                        if (!hasBottom)
                            return false;
                        patch.domain = FloorPolygons::Intersect (patch.domain, bottom);
                    } else {
                        FloorPolygons admitted;
                        if (!FloorPolygons_ClipBelow (patch.domain, patch.plane, level, admitted))
                            return false;
                        patch.domain = admitted;
                    }
                    if (patch.domain.IsEmpty ())
                        continue;
                    patch.plane.draw.base_guid = source.baseGuid;
                    patch.plane.draw.base_type = API_MeshID;
                    patch.plane.draw.draw_type = API_RoofID;
                    patch.plane.surface_geometry = true;
                    patch.plane.draw.type = Floor;
                    patch.plane.floorInd = floorInd;
                    domain.Append (patch.domain);
                    patches.push_back (patch);
                }
            }
        }
        domain.Unify ();
        if (domain.IsEmpty ()) {
            if (provenance != nullptr)
                *provenance = std::move (staged);
            meshCoverage.Clear ();
            return true;
        }
        for (const auto &support : slabSupports) {
            if (!support.isValid || support.draw.base_guid == APINULLGuid || support.draw.base_type != API_SlabID ||
                support.poly.IsEmpty () || !std::isfinite (support.zBottom) || support.slope_x != 0 ||
                support.slope_y != 0)
                return false;
            Patch patch;
            patch.plane = support;
            patch.plane.draw.type = Floor;
            patch.plane.floorInd = floorInd;
            patch.plane.surface_geometry = true;
            FloorPolygons projected;
            projected.Push (support.poly);
            patch.domain = FloorPolygons::Intersect (projected, domain);
            if (!patch.domain.IsEmpty ())
                patches.push_back (patch);
        }
        // Стабильная опорная плоскость и проверка каждого участника не допускают tolerance-chain.
        std::sort (patches.begin (), patches.end (), [] (const Patch &a, const Patch &b) {
            if (a.plane.slope_x != b.plane.slope_x)
                return a.plane.slope_x < b.plane.slope_x;
            if (a.plane.slope_y != b.plane.slope_y)
                return a.plane.slope_y < b.plane.slope_y;
            return FloorPlane_GetHeight (a.plane, {0, 0}) < FloorPlane_GetHeight (b.plane, {0, 0});
        });
        std::vector<Patch> groups;
        for (const auto &patch : patches) {
            bool merged = false;
            for (auto &group : groups) {
                // Грани Mesh остаются отдельными заготовками даже в одной плоскости.
                if (patch.plane.draw.base_type == API_MeshID)
                    continue;
                if (group.plane.draw.base_guid != patch.plane.draw.base_guid ||
                    group.plane.draw.base_type != patch.plane.draw.base_type)
                    continue;
                auto combined = group.domain;
                combined.Append (patch.domain);
                bool compatible = true;
                for (const auto &member : group.members)
                    compatible = compatible && FloorPlanes_AreSame (member, patch.plane, combined);
                if (!compatible)
                    continue;
                group.domain = combined;
                group.domain.Unify ();
                group.members.push_back (patch.plane);
                group.supports.insert (group.supports.end (), patch.supports.begin (), patch.supports.end ());
                merged = true;
                break;
            }
            if (!merged) {
                auto group = patch;
                group.members.push_back (patch.plane);
                groups.push_back (group);
            }
        }
        std::vector<FloorPolygons> visible (groups.size ());
        for (size_t i = 0; i < groups.size (); ++i) {
            visible[i] = groups[i].domain;
            for (size_t j = 0; j < groups.size () && !visible[i].IsEmpty (); ++j) {
                if (i == j)
                    continue;
                const auto overlap = FloorPolygons::Intersect (groups[i].domain, groups[j].domain);
                if (overlap.IsEmpty () || FloorPlanes_AreSame (groups[i].plane, groups[j].plane, overlap))
                    continue;
                FloorPolygons higher;
                if (!FloorPolygons_ClipBelow (groups[j].domain, groups[i].plane, groups[j].plane, higher))
                    return false;
                visible[i] = FloorPolygons::Substract (visible[i], higher);
            }
        }
        // Неоднозначность оценивается после всех более высоких опор, а не до третьего кандидата.
        for (size_t i = 0; i < groups.size (); ++i)
            for (size_t j = 0; j < i; ++j) {
                const auto overlap = FloorPolygons::Intersect (visible[i], visible[j]);
                if (overlap.CalcArea () <= AreaTolerance)
                    continue;
                if (groups[i].plane.draw.base_guid != groups[j].plane.draw.base_guid ||
                    groups[i].plane.draw.base_type != groups[j].plane.draw.base_type)
                    return false;
                visible[i] = FloorPolygons::Substract (visible[i], visible[j]);
            }
        GS::Array<OtdSlab> results;
        FloorPolygons complete;
        for (size_t i = 0; i < groups.size (); ++i)
            for (const auto &polygon : visible[i]) {
                auto floor = groups[i].plane;
                floor.poly = polygon;
                if (provenance != nullptr) {
                    FloorPolygons winner;
                    winner.Push (polygon);
                    for (const UIndex supportIndex : groups[i].supports) {
                        MeshFloorWinnerContribution contribution;
                        contribution.supportIndex = supportIndex;
                        contribution.floorIndex = floors.GetSize () + results.GetSize ();
                        contribution.domain = FloorPolygons::Intersect (staged.supports[supportIndex].admitted, winner);
                        if (!contribution.domain.IsEmpty ())
                            staged.contributions.Push (contribution);
                    }
                }
                results.Push (floor);
                complete.Push (polygon);
            }
        complete.Unify ();
        if (FloorPolygons::Substract (complete, domain).CalcArea () > AreaTolerance ||
            FloorPolygons::Substract (domain, complete).CalcArea () > AreaTolerance)
            return false;
        if (provenance != nullptr)
            *provenance = std::move (staged);
        floors.Append (results);
        meshCoverage = domain;
        return true;
    }

    bool MeshFloorFinishes_BuildProjected (const Geometry::Polygon2D &footprint,
                                           double roomFloor,
                                           short floorInd,
                                           const GS::Array<MeshFloorSource> &meshes,
                                           const GS::Array<OtdSlab> &slabSupports,
                                           GS::Array<OtdSlab> &floors,
                                           Geometry::MultiPolygon2D &meshCoverage) {
        return MeshFloorFinishes_BuildProjectedImpl (
            footprint, roomFloor, floorInd, meshes, slabSupports, floors, meshCoverage, nullptr);
    }

    bool MeshFloorFinishes_BuildProjectedWithProvenance (const Geometry::Polygon2D &footprint,
                                                         double roomFloor,
                                                         short floorInd,
                                                         const GS::Array<MeshFloorSource> &meshes,
                                                         const GS::Array<OtdSlab> &slabSupports,
                                                         GS::Array<OtdSlab> &floors,
                                                         Geometry::MultiPolygon2D &meshCoverage,
                                                         MeshFloorProvenance &provenance) {
        return MeshFloorFinishes_BuildProjectedImpl (
            footprint, roomFloor, floorInd, meshes, slabSupports, floors, meshCoverage, &provenance);
    }

    namespace {
        struct ProjectedSlabCandidate {
            OtdSlab slab;
            double bottom, top;
        };

        // Общие исходные контакты до выбора победителя и ограничения числа компонент.
        bool OtdSlab_CollectProjectedInputsImpl (const RoomSurfaceSnapshot &zone,
                                                 const GS::Array<RoomSurfaceSource> &sources,
                                                 const GS::Array<API_Guid> &roles,
                                                 TypeOtd type,
                                                 short floorInd,
                                                 const GS::Array<OtdSlab> *prepared,
                                                 Geometry::MultiPolygon2D &footprint,
                                                 std::vector<ProjectedSlabCandidate> &candidates,
                                                 SlabFloorProvenance *provenance) {
            if (!zone.IsComplete () || (type != Floor && type != Ceil))
                return false;
            const double normal = type == Floor ? -1 : 1;
            const auto horizontal = [] (const RoomSurface &face) {
                if (face.contours.IsEmpty () || face.contours[0].vertices.IsEmpty () ||
                    std::hypot (face.normalWorld.x, face.normalWorld.y) > DistanceTolerance ||
                    std::fabs (std::fabs (face.normalWorld.z) - 1) > DistanceTolerance)
                    return false;
                const double z = face.contours[0].vertices[0].world.z;
                for (const auto &contour : face.contours)
                    for (const auto &point : contour.vertices)
                        if (!Coord3D_IsFinite (point.world) || std::fabs (point.world.z - z) > DistanceTolerance)
                            return false;
                return true;
            };
            // Объединение проекций убирает дубли разных тел/ступеней зоны, сохраняя
            // отверстия.
            for (const auto &body : zone.bodies)
                for (const auto &face : body.surfaces) {
                    if (!horizontal (face) || face.normalWorld.z * normal < 0)
                        continue;
                    RoomSurfacePiece piece;
                    piece.status = RoomSurfacePieceStatus::Matched;
                    piece.baseGuid = zone.guid;
                    piece.baseType = API_ZoneID;
                    piece.normal = {0, 0, normal};
                    for (const auto &contour : face.contours) {
                        if (contour.vertices.GetSize () < 3)
                            return false;
                        const auto origin = contour.vertices[0].world;
                        RoomSurfacePieceContour projected;
                        projected.hole = contour.hole;
                        double twiceArea = 0;
                        for (UIndex i = 0; i < contour.vertices.GetSize (); ++i) {
                            const auto &a = contour.vertices[i].world;
                            const auto &b = contour.vertices[(i + 1) % contour.vertices.GetSize ()].world;
                            projected.vertices.Push ({a.x, a.y, 0});
                            twiceArea += (a.x - origin.x) * (b.y - origin.y) - (b.x - origin.x) * (a.y - origin.y);
                        }
                        piece.area += (contour.hole ? -1 : 1) * std::fabs (twiceArea) / 2;
                        piece.contours.Push (projected);
                    }
                    GS::Array<OtdSlab> projected;
                    if (!SurfaceSlabs_Build (piece, floorInd, projected))
                        return false;
                    for (const auto &slab : projected)
                        footprint.Push (slab.poly);
                }
            if (footprint.IsEmpty ())
                return false;
            footprint.Unify ();

            const auto recordSupport = [&] (const RoomSurfaceSource &source,
                                            const OtdSlab &slab,
                                            const RoomSurface *face,
                                            UIndex preparedIndex) {
                if (provenance == nullptr)
                    return true;
                if (source.baseGuid == APINULLGuid || source.baseType != API_SlabID ||
                    source.geometryGuid == APINULLGuid || source.snapshot.guid != source.geometryGuid ||
                    slab.draw.base_guid != source.baseGuid || slab.draw.base_type != API_SlabID || !slab.isValid ||
                    slab.poly.IsEmpty () || !std::isfinite (slab.zBottom) || slab.slope_x != 0 || slab.slope_y != 0)
                    return false;
                SlabFloorFiniteSupport support;
                support.ownerGuid = source.baseGuid;
                support.geometryGuid = source.geometryGuid;
                support.ownerType = source.baseType;
                support.origin = source.origin;
                support.preparedIndex = preparedIndex;
                if (face != nullptr) {
                    support.bodyIndex = face->bodyIndex;
                    support.polygonIndex = face->polygonIndex;
                    support.modelMaterialIndex = face->modelMaterialIndex;
                }
                support.plane = slab;
                support.projected.Push (slab.poly);
                // roles уже задают admission Slab. Не добавлять Mesh roomFloor-policy.
                support.admitted = support.projected;
                provenance->supports.Push (support);
                return true;
            };
            GS::Array<API_Guid> visited;
            for (const auto &guid : roles) {
                bool duplicate = false;
                for (const auto &seen : visited)
                    duplicate = duplicate || seen == guid;
                if (duplicate)
                    continue;
                visited.Push (guid);
                bool found = false;
                for (const auto &source : sources) {
                    if (source.baseGuid != guid || source.baseType != API_SlabID)
                        continue;
                    found = true;
                    if (!source.snapshot.IsComplete ())
                        return false;
                    if (source.origin == RoomSurfaceOrigin::PreparedAnalytic && prepared != nullptr) {
                        bool hasBase = false;
                        for (UIndex preparedIndex = 0; preparedIndex < prepared->GetSize (); ++preparedIndex) {
                            const auto &base = (*prepared)[preparedIndex];
                            if (base.draw.base_guid != guid)
                                continue;
                            hasBase = true;
                            OtdSlab slab = base;
                            slab.draw.type = type;
                            slab.floorInd = floorInd;
                            slab.surface_geometry = true;
                            const double bottom = base.zBottom, top = bottom + base.height;
                            slab.zBottom = type == Floor ? top : bottom;
                            if (provenance != nullptr && !recordSupport (source, slab, nullptr, preparedIndex))
                                return false;
                            candidates.push_back ({slab, bottom, top});
                        }
                        if (!hasBase)
                            return false;
                        continue;
                    }
                    for (const auto &body : source.snapshot.bodies) {
                        double bottom = std::numeric_limits<double>::infinity ();
                        double top = -bottom;
                        for (const auto &face : body.surfaces)
                            for (const auto &contour : face.contours)
                                for (const auto &vertex : contour.vertices) {
                                    if (!Coord3D_IsFinite (vertex.world))
                                        return false;
                                    bottom = std::min (bottom, vertex.world.z);
                                    top = std::max (top, vertex.world.z);
                                }
                        for (const auto &face : body.surfaces) {
                            if (!horizontal (face) || face.normalWorld.z * normal > 0)
                                continue;
                            RoomSurfacePiece piece;
                            piece.status = RoomSurfacePieceStatus::Matched;
                            piece.baseGuid = guid;
                            piece.baseType = API_SlabID;
                            piece.normal = {0, 0, normal};
                            const double elevation = face.contours[0].vertices[0].world.z;
                            piece.planeOffset = -normal * elevation;
                            for (const auto &contour : face.contours) {
                                if (contour.vertices.GetSize () < 3)
                                    return false;
                                RoomSurfacePieceContour output;
                                output.hole = contour.hole;
                                const auto origin = contour.vertices[0].world;
                                double twiceArea = 0;
                                for (UIndex i = 0; i < contour.vertices.GetSize (); ++i) {
                                    const auto a = contour.vertices[i].world;
                                    const auto b = contour.vertices[(i + 1) % contour.vertices.GetSize ()].world;
                                    output.vertices.Push (a);
                                    twiceArea +=
                                        (a.x - origin.x) * (b.y - origin.y) - (b.x - origin.x) * (a.y - origin.y);
                                }
                                piece.area += (contour.hole ? -1 : 1) * std::fabs (twiceArea) / 2;
                                piece.contours.Push (output);
                            }
                            GS::Array<OtdSlab> selected;
                            if (!SurfaceSlabs_Build (piece, floorInd, selected))
                                return false;
                            for (const auto &slab : selected) {
                                if (provenance != nullptr && !recordSupport (source, slab, &face, 0))
                                    return false;
                                candidates.push_back ({slab, bottom, top});
                            }
                        }
                    }
                }
                if (!found)
                    return false;
            }
            return true;
        }

        bool OtdSlab_CollectProjectedInputs (const RoomSurfaceSnapshot &zone,
                                             const GS::Array<RoomSurfaceSource> &sources,
                                             const GS::Array<API_Guid> &roles,
                                             TypeOtd type,
                                             short floorInd,
                                             const GS::Array<OtdSlab> *prepared,
                                             Geometry::MultiPolygon2D &footprint,
                                             std::vector<ProjectedSlabCandidate> &candidates) {
            return OtdSlab_CollectProjectedInputsImpl (
                zone, sources, roles, type, floorInd, prepared, footprint, candidates, nullptr);
        }

    } // namespace

    bool OtdSlab_BuildProjectedFinishes (const RoomSurfaceSnapshot &zone,
                                         const GS::Array<RoomSurfaceSource> &sources,
                                         const GS::Array<API_Guid> &roles,
                                         TypeOtd type,
                                         short floorInd,
                                         GS::Array<OtdSlab> &slabs,
                                         double &rejectedArea,
                                         const OtdSlab *nominal,
                                         bool onlyOnSlab,
                                         GS::Array<OtdWall> *walls,
                                         const GS::Array<OtdSlab> *prepared) {
        using Candidate = ProjectedSlabCandidate;
        Geometry::MultiPolygon2D footprint;
        std::vector<Candidate> candidates;
        if (!OtdSlab_CollectProjectedInputs (zone, sources, roles, type, floorInd, prepared, footprint, candidates))
            return false;
        // Потолок зоны — такой же кандидат: ближние перекрытия обрабатываются раньше,
        // а остаток покрывается отметкой height_ceil прежним алгоритмом остатка.
        const bool zoneCeilingCandidate = type == Ceil && nominal != nullptr && !onlyOnSlab;
        if (zoneCeilingCandidate) {
            Candidate zoneCeiling;
            zoneCeiling.slab = *nominal;
            zoneCeiling.slab.draw.type = type;
            zoneCeiling.slab.floorInd = floorInd;
            zoneCeiling.slab.surface_geometry = true;
            zoneCeiling.slab.zBottom = nominal->zBottom;
            // Перепад до номинальной отметки не строим: собственных границ у кандидата
            // нет.
            zoneCeiling.bottom = std::numeric_limits<double>::infinity ();
            zoneCeiling.top = -zoneCeiling.bottom;
            candidates.push_back (zoneCeiling);
        }
        // Изнутри зоны: верхние грани пола по убыванию, нижние грани потолка по
        // возрастанию.
        std::sort (candidates.begin (), candidates.end (), [type] (const Candidate &a, const Candidate &b) {
            return type == Floor ? a.slab.zBottom > b.slab.zBottom : a.slab.zBottom < b.slab.zBottom;
        });
        Geometry::MultiPolygon2D remainder = footprint;
        GS::Array<OtdSlab> results;
        GS::Array<OtdWall> steps;
        double rejected = 0;
        for (UIndex i = 0; i < candidates.size (); ++i) {
            const auto &candidate = candidates[i];
            Geometry::MultiPolygon2D intersections;
            for (const auto &remaining : remainder)
                for (const auto &part : remaining.Intersect (candidate.slab.poly))
                    intersections.Push (part);
            if (intersections.IsEmpty ())
                continue;
            // Явное ограничение legacy: сохраняем только наибольшую несвязную часть
            // пересечения/остатка.
            Geometry::Polygon2D processed = intersections.PopLargest ();
            if (processed.CalcArea () < 0.001)
                continue;
            Geometry::MultiPolygon2D accepted;
            accepted.Push (processed);
            // std::sort не задавал владельца при равных отметках; не выбираем его по
            // GUID/порядку.
            for (UIndex j = 0; j < candidates.size (); ++j) {
                if (j == i || candidates[j].slab.draw.base_guid == candidate.slab.draw.base_guid ||
                    std::fabs (candidates[j].slab.zBottom - candidate.slab.zBottom) > DistanceTolerance)
                    continue;
                Geometry::MultiPolygon2D next;
                for (const auto &part : accepted) {
                    for (const auto &ambiguous : part.Intersect (candidates[j].slab.poly))
                        rejected += ambiguous.CalcArea ();
                    for (const auto &unique : part.Substract (candidates[j].slab.poly))
                        next.Push (unique);
                }
                accepted = next;
            }
            if (!accepted.IsEmpty ()) {
                OtdSlab slab = candidate.slab;
                slab.poly = accepted.PopLargest ();
                if (nominal != nullptr)
                    slab.draw.material = nominal->draw.material;
                results.Push (slab);
                Geometry::Polygon2D sidePolygon = slab.poly;
                sidePolygon.RemoveHoles ();
                if (nominal != nullptr && walls != nullptr && nominal->zBottom >= candidate.bottom &&
                    nominal->zBottom <= candidate.top) {
                    const double height =
                        type == Floor ? slab.zBottom - nominal->zBottom : nominal->zBottom - slab.zBottom;
                    if (height > 0.005) {
                        GS::Array<Point2D> coords;
                        GS::Array<PolyArcRec> arcs;
                        GS::Array<UIndex> ends;
                        GS::Array<Geometry::Polygon2D::ConstContourIterator> reversed;
                        const auto segmentedSides = sidePolygon.SegmentUp (ArcDivParams (0.00001));
                        if (segmentedSides.GetSize () != 1)
                            return false;
                        segmentedSides[0].CopyPolygonData (
                            coords, arcs, ends, reversed, Geometry::PolyCreateFlags_IdxRefInContourEndFromZero);
                        if (!arcs.IsEmpty () || ends.IsEmpty ())
                            return false;
                        for (UIndex k = 0; k <= ends[0]; ++k) {
                            const auto beg = coords[k];
                            const auto end = coords[k == ends[0] ? 0 : k + 1];
                            // Концы внутренней хорды могут лежать на границе; вычитаем только
                            // совпадающие конечные участки ребра, а не всё ребро по его концам.
                            GS::Array<Sector> internalEdges = {Sector (beg, end)};
                            for (const auto &remaining : remainder) {
                                GS::Array<Sector> nextEdges;
                                for (const auto &edge : internalEdges)
                                    nextEdges.Append (remaining.ElemPolyDiffBorderOnly (edge));
                                internalEdges = nextEdges;
                            }
                            for (const auto &edge : internalEdges) {
                                if (std::hypot (edge.c1.x - edge.c2.x, edge.c1.y - edge.c2.y) <= DistanceTolerance)
                                    continue;
                                OtdWall wall;
                                const double z = type == Floor ? nominal->zBottom : slab.zBottom;
                                wall.geom.seg.begC = {edge.c2.x, edge.c2.y, z};
                                wall.geom.seg.endC = {edge.c1.x, edge.c1.y, z};
                                wall.geom.height = height;
                                wall.draw.base_guid = slab.draw.base_guid;
                                wall.draw.base_type = API_SlabID;
                                wall.draw.draw_type = API_WallID;
                                wall.draw.type = type;
                                wall.geom.floorInd = floorInd;
                                wall.draw.material = nominal->draw.material;
                                wall.surface_geometry = true;
                                wall.slab_step = true;
                                steps.Push (std::move (wall));
                            }
                        }
                    }
                }
            }
            // Прежнее исключение: отверстия обработанного покрытия не становятся
            // нижней/резервной отделкой.
            processed.RemoveHoles ();
            Geometry::MultiPolygon2D next;
            for (const auto &remaining : remainder)
                for (const auto &part : remaining.Substract (processed))
                    next.Push (part);
            remainder.Clear ();
            if (!next.IsEmpty ())
                remainder.Push (next.PopLargest ());
        }
        // Номинальный потолок уже был кандидатом: второй остаток на той же отметке не
        // нужен.
        if (!onlyOnSlab && nominal != nullptr && !zoneCeilingCandidate && remainder.CalcArea () > 0.0001) {
            OtdSlab residual = *nominal;
            residual.poly = remainder.PopLargest ();
            residual.floorInd = floorInd;
            residual.draw.type = type;
            residual.surface_geometry = true;
            results.Push (residual);
        }
        if (walls != nullptr)
            walls->Append (steps);
        slabs.Append (results);
        rejectedArea += rejected;
        return true;
    }

    namespace {
        Geometry::MultiPolygon2D OtdSlab_BuildCoverage (const GS::Array<OtdSlab> &floors) {
            Geometry::MultiPolygon2D coverage;
            for (const auto &floor : floors)
                coverage.Push (floor.poly);
            coverage.Unify ();
            return coverage;
        }

        bool OtdSlab_HasSameOwner (const OtdSlab &a, const OtdSlab &b) {
            return a.draw.base_guid == b.draw.base_guid && a.draw.base_type == b.draw.base_type;
        }

        // Положительная площадь с прежним owner и той же плоскостью не меняет
        // смежность.
        Geometry::MultiPolygon2D OtdSlab_BuildChangedCoverage (const GS::Array<OtdSlab> &before,
                                                               const GS::Array<OtdSlab> &after) {
            Geometry::MultiPolygon2D changed;
            const auto appendDifference = [&] (const GS::Array<OtdSlab> &left, const GS::Array<OtdSlab> &right) {
                for (const auto &floor : left) {
                    Geometry::MultiPolygon2D remaining;
                    remaining.Push (floor.poly);
                    for (const auto &other : right) {
                        if (!OtdSlab_HasSameOwner (floor, other))
                            continue;
                        Geometry::MultiPolygon2D domain;
                        domain.Push (other.poly);
                        const auto overlap = Geometry::MultiPolygon2D::Intersect (remaining, domain);
                        if (!overlap.IsEmpty () && FloorPlanes_AreSame (floor, other, overlap))
                            remaining = Geometry::MultiPolygon2D::Substract (remaining, overlap);
                    }
                    changed.Append (remaining);
                }
            };
            appendDifference (before, after);
            appendDifference (after, before);
            changed.Unify ();
            return changed;
        }

        // Все компоненты разности сохраняют исходную запись целиком, кроме контура.
        bool OtdSlab_MergeMixedDomains (const GS::Array<OtdSlab> &legacy,
                                        const GS::Array<OtdSlab> &inside,
                                        const Geometry::MultiPolygon2D &domain,
                                        GS::Array<OtdSlab> &output) {
            GS::Array<OtdSlab> staged = inside;
            for (const auto &floor : legacy) {
                Geometry::MultiPolygon2D original;
                original.Push (floor.poly);
                for (const auto &polygon : Geometry::MultiPolygon2D::Substract (original, domain)) {
                    auto remainder = floor;
                    remainder.poly = polygon;
                    staged.Push (remainder);
                }
            }
            const auto insideCoverage = OtdSlab_BuildCoverage (inside);
            if (Geometry::MultiPolygon2D::Substract (insideCoverage, domain).CalcArea () > AreaTolerance ||
                Geometry::MultiPolygon2D::Substract (domain, insideCoverage).CalcArea () > AreaTolerance)
                return false;
            for (UIndex i = 0; i < staged.GetSize (); ++i)
                for (UIndex j = 0; j < i; ++j) {
                    Geometry::MultiPolygon2D a, b;
                    a.Push (staged[i].poly);
                    b.Push (staged[j].poly);
                    if (Geometry::MultiPolygon2D::Intersect (a, b).CalcArea () > AreaTolerance)
                        return false;
                }
            output = staged;
            return true;
        }
    } // namespace

    static bool OtdSlab_BuildFiniteMixedGeometryImpl (const RoomSurfaceSnapshot &zone,
                                                      const GS::Array<RoomSurfaceSource> &sources,
                                                      const GS::Array<API_Guid> &roles,
                                                      short floorInd,
                                                      const OtdSlab &nominal,
                                                      bool onlyOnSlab,
                                                      const GS::Array<OtdSlab> &prepared,
                                                      double roomFloor,
                                                      const GS::Array<MeshFloorSource> &meshes,
                                                      MixedFloorGeometry &result,
                                                      MixedFloorProvenance *provenance) {
        MixedFloorGeometry staged;
        // Та же функция, аргументы и error-policy; никаких новых проверок для
        // Slab-only.
        if (!OtdSlab_BuildProjectedFinishes (zone,
                                             sources,
                                             roles,
                                             Floor,
                                             floorInd,
                                             staged.floors,
                                             staged.rejectedArea,
                                             &nominal,
                                             onlyOnSlab,
                                             &staged.steps,
                                             &prepared))
            return false;
        Geometry::MultiPolygon2D provenanceFootprint;
        std::vector<ProjectedSlabCandidate> provenanceRaw;
        if (provenance != nullptr && !OtdSlab_CollectProjectedInputsImpl (zone,
                                                                          sources,
                                                                          roles,
                                                                          Floor,
                                                                          floorInd,
                                                                          &prepared,
                                                                          provenanceFootprint,
                                                                          provenanceRaw,
                                                                          &provenance->slab))
            return false;
        const auto recordSlabWinners = [&] () {
            if (provenance == nullptr)
                return true;
            for (UIndex i = 0; i < staged.floors.GetSize (); ++i) {
                const auto &floor = staged.floors[i];
                if (floor.draw.base_type != API_SlabID)
                    continue; // nominal Zone residual никогда не получает ложного Slab-owner.
                Geometry::MultiPolygon2D winner, covered;
                winner.Push (floor.poly);
                for (UIndex j = 0; j < provenance->slab.supports.GetSize (); ++j) {
                    const auto &support = provenance->slab.supports[j];
                    if (support.ownerGuid != floor.draw.base_guid || support.ownerType != floor.draw.base_type)
                        continue;
                    auto overlap = Geometry::MultiPolygon2D::Intersect (support.admitted, winner);
                    // Slab здесь горизонтальны; сравнение плоскостей не зависит от дуг XY-контура.
                    if (overlap.IsEmpty () || floor.slope_x != 0 || floor.slope_y != 0 ||
                        !std::isfinite (floor.zBottom) ||
                        std::fabs (support.plane.zBottom - floor.zBottom) > DistanceTolerance)
                        continue;
                    SlabFloorWinnerContribution contribution;
                    contribution.supportIndex = j;
                    contribution.floorIndex = i;
                    contribution.domain = overlap;
                    provenance->slab.contributions.Push (contribution);
                    covered.Append (overlap);
                }
                covered.Unify ();
                if (Geometry::MultiPolygon2D::Substract (winner, covered).CalcArea () > AreaTolerance ||
                    Geometry::MultiPolygon2D::Substract (covered, winner).CalcArea () > AreaTolerance)
                    return false;
            }
            return true;
        };
        if (meshes.IsEmpty ()) {
            if (provenance != nullptr && !recordSlabWinners ())
                return false;
            result = staged;
            return true;
        }
        Geometry::MultiPolygon2D footprint;
        std::vector<ProjectedSlabCandidate> raw;
        if (provenance != nullptr) {
            footprint = provenanceFootprint;
            raw = provenanceRaw;
        } else if (!OtdSlab_CollectProjectedInputs (zone, sources, roles, Floor, floorInd, &prepared, footprint, raw))
            return false;
        GS::Array<OtdSlab> supports;
        for (const auto &candidate : raw) {
            auto support = candidate.slab;
            // Как и в legacy: material задаётся номиналом, base provenance не
            // подменяется.
            support.draw.material = nominal.draw.material;
            supports.Push (support);
        }
        GS::Array<OtdSlab> inside;
        Geometry::MultiPolygon2D domain;
        for (const auto &component : footprint) {
            Geometry::MultiPolygon2D localDomain;
            GS::Array<OtdSlab> localInside;
            if (provenance != nullptr) {
                // Общий inside сохраняет индексы всех уже обработанных компонент комнаты.
                if (!MeshFloorFinishes_BuildProjectedWithProvenance (
                        component, roomFloor, floorInd, meshes, supports, inside, localDomain, provenance->mesh))
                    return false;
            } else {
                if (!MeshFloorFinishes_BuildProjected (
                        component, roomFloor, floorInd, meshes, supports, localInside, localDomain))
                    return false;
                inside.Append (localInside);
            }
            domain.Append (localDomain);
        }
        domain.Unify ();
        if (domain.IsEmpty ()) {
            if (provenance != nullptr && !recordSlabWinners ())
                return false;
            result = staged;
            return true;
        }
        for (auto &floor : inside)
            floor.draw.material = nominal.draw.material;
        GS::Array<OtdSlab> merged;
        if (!OtdSlab_MergeMixedDomains (staged.floors, inside, domain, merged))
            return false;
        staged.changedDomain = OtdSlab_BuildChangedCoverage (staged.floors, merged);

        staged.floors = merged;
        staged.meshDomain = domain;
        if (provenance != nullptr && !recordSlabWinners ())
            return false;
        result = staged;
        return true;
    }

    bool OtdSlab_BuildFiniteMixedGeometry (const RoomSurfaceSnapshot &zone,
                                           const GS::Array<RoomSurfaceSource> &sources,
                                           const GS::Array<API_Guid> &roles,
                                           short floorInd,
                                           const OtdSlab &nominal,
                                           bool onlyOnSlab,
                                           const GS::Array<OtdSlab> &prepared,
                                           double roomFloor,
                                           const GS::Array<MeshFloorSource> &meshes,
                                           MixedFloorGeometry &result) {
        return OtdSlab_BuildFiniteMixedGeometryImpl (
            zone, sources, roles, floorInd, nominal, onlyOnSlab, prepared, roomFloor, meshes, result, nullptr);
    }

    bool OtdSlab_BuildFiniteMixedGeometryWithProvenance (const RoomSurfaceSnapshot &zone,
                                                         const GS::Array<RoomSurfaceSource> &sources,
                                                         const GS::Array<API_Guid> &roles,
                                                         short floorInd,
                                                         const OtdSlab &nominal,
                                                         bool onlyOnSlab,
                                                         const GS::Array<OtdSlab> &prepared,
                                                         double roomFloor,
                                                         const GS::Array<MeshFloorSource> &meshes,
                                                         MixedFloorGeometry &result,
                                                         MixedFloorProvenance &provenance) {
        MixedFloorGeometry local;
        MixedFloorProvenance localProvenance;
        if (!OtdSlab_BuildFiniteMixedGeometryImpl (zone,
                                                   sources,
                                                   roles,
                                                   floorInd,
                                                   nominal,
                                                   onlyOnSlab,
                                                   prepared,
                                                   roomFloor,
                                                   meshes,
                                                   local,
                                                   &localProvenance))
            return false;
        MixedFloorGeometry staged = result;
        MixedFloorProvenance stagedProvenance = provenance;
        const UIndex floorOffset = staged.floors.GetSize ();
        const UIndex meshOffset = stagedProvenance.mesh.supports.GetSize ();
        const UIndex slabOffset = stagedProvenance.slab.supports.GetSize ();
        for (auto &c : localProvenance.mesh.contributions) {
            c.floorIndex += floorOffset;
            c.supportIndex += meshOffset;
        }
        for (auto &c : localProvenance.slab.contributions) {
            c.floorIndex += floorOffset;
            c.supportIndex += slabOffset;
        }
        staged.floors.Append (local.floors);
        staged.steps.Append (local.steps);
        staged.meshDomain.Append (local.meshDomain);
        staged.meshDomain.Unify ();
        staged.changedDomain.Append (local.changedDomain);
        staged.changedDomain.Unify ();
        staged.rejectedArea += local.rejectedArea;
        stagedProvenance.mesh.supports.Append (localProvenance.mesh.supports);
        stagedProvenance.mesh.contributions.Append (localProvenance.mesh.contributions);
        stagedProvenance.slab.supports.Append (localProvenance.slab.supports);
        stagedProvenance.slab.contributions.Append (localProvenance.slab.contributions);
        result = std::move (staged);
        provenance = std::move (stagedProvenance);
        return true;
    }

    namespace MeshDemand {
        constexpr double E = DistanceTolerance;
        using MP = Geometry::MultiPolygon2D;
        constexpr double ParallelTolerance = 1.e-6; // Безразмерный определитель, не расстояние.
        constexpr double SlopeTolerance = 1.e-6;

        double Z (const OtdSlab &f, Point2D p) {
            return f.zBottom + f.slope_x * (p.x - f.plane_origin.x) + f.slope_y * (p.y - f.plane_origin.y);
        }

        MP One (const Geometry::Polygon2D &p) {
            MP r;
            r.Push (p);
            return r;
        }

        double AT (const MP &a) { return AreaTolerance * (1.0 + 0.05 * std::fabs (a.CalcArea ())); }

        bool Subset (const MP &a, const MP &b) { return MP::Substract (a, b).CalcArea () <= AT (a); }

        bool Same (const OtdSlab &a, const OtdSlab &b, const MP &d) {
            if (!std::isfinite (a.slope_x) || !std::isfinite (a.slope_y) || !std::isfinite (a.zBottom))
                return false;
            if (std::fabs (a.slope_x - b.slope_x) > SlopeTolerance ||
                std::fabs (a.slope_y - b.slope_y) > SlopeTolerance)
                return false;
            for (const auto &p : d)
                for (auto v = p.BeginVertex (); v != p.EndVertex (); ++v) {
                    const auto q = p.GetCoord (v);
                    if (!std::isfinite (Z (a, q)) || !std::isfinite (Z (b, q)) || std::fabs (Z (a, q) - Z (b, q)) > E)
                        return false;
                }
            return true;
        }

        bool Covered (const OtdRoom &r, const OtdSlab &plane, MP demand) {
            const double allowedArea = AT (demand);
            for (const auto &f : r.otdslab)
                if (f.isValid && f.draw.type == Floor && Same (plane, f, demand))
                    demand = MP::Substract (demand, One (f.poly));
            return demand.CalcArea () <= allowedArea;
        }

        // Линейная отсечка по локальной отметке; верхняя граница открыта.
        bool Band (double z0, double z1, double low, double high, double &a, double &b) {
            const double dz = z1 - z0;
            if (std::fabs (dz) <= 1.e-14)
                return z0 >= low - E && z0 < high - E;
            double x = (low - E - z0) / dz, y = (high - E - z0) / dz;
            if (x > y)
                std::swap (x, y);
            a = std::max (a, x);
            b = std::min (b, y);
            return b > a; // Параметр безразмерный; длину контакта проверяет caller.
        }

        bool Axis (const OtdWall &w, const OtdSlab &f, Sector &s) {
            double base = OtdLinearSegment_GetZBottom (w.geom);
            const double den = 1 - f.slope_x * w.rise_x - f.slope_y * w.rise_y;
            if (!std::isfinite (den) || std::fabs (den) < ParallelTolerance)
                return false;
            auto point = [&] (const API_Coord3D &p) {
                double dz = (Z (f, {p.x, p.y}) - base) / den;
                return Point2D{p.x + w.rise_x * dz, p.y + w.rise_y * dz};
            };
            s = {point (w.geom.seg.begC), point (w.geom.seg.endC)};
            return std::isfinite (s.c1.x) && std::isfinite (s.c1.y) && std::isfinite (s.c2.x) && std::isfinite (s.c2.y);
        }

        Point2D At (const Sector &s, double t) {
            return {s.c1.x + (s.c2.x - s.c1.x) * t, s.c1.y + (s.c2.y - s.c1.y) * t};
        }

        MP Strip (Point2D a, Point2D b, double x, double y) {
            return Geometry::Polygon2D::Create (GS::Array<Point2D>{a, b, {b.x + x, b.y + y}, {a.x + x, a.y + y}}, 0);
        }

        // Консервативный отказ допустим лишь при возможном конечном контакте.
        bool NoFiniteContact (const OtdWall &w, const OtdSlab &f, const MP &domain) {
            const double base = OtdLinearSegment_GetZBottom (w.geom);
            double lo = std::numeric_limits<double>::infinity (), hi = -lo;
            for (const auto &p : {w.geom.seg.begC, w.geom.seg.endC})
                for (double h : {0.0, w.geom.height}) {
                    const double q = Z (f, {p.x + w.rise_x * h, p.y + w.rise_y * h}) - base - h;
                    if (!std::isfinite (q))
                        return false;
                    lo = std::min (lo, q);
                    hi = std::max (hi, q);
                }
            if (lo > E || hi < -E)
                return true;
            Point2D a{w.geom.seg.begC.x, w.geom.seg.begC.y}, b{w.geom.seg.endC.x, w.geom.seg.endC.y};
            const double x = w.rise_x * w.geom.height, y = w.rise_y * w.geom.height;
            if (std::fabs (x * (b.y - a.y) - y * (b.x - a.x)) > E * E &&
                MP::Intersect (Strip (a, b, x, y), domain).CalcArea () > AreaTolerance)
                return false;
            const Point2D c{b.x + x, b.y + y}, d{a.x + x, a.y + y};
            for (const auto &edge : {Sector{a, b}, Sector{b, c}, Sector{c, d}, Sector{d, a}})
                for (const auto &poly : domain)
                    for (const auto &cut : poly.CutElem (edge, Geometry::Boundary_Closed))
                        if (std::hypot (cut.c2.x - cut.c1.x, cut.c2.y - cut.c1.y) > E)
                            return false;
            return true;
        }
    } // namespace MeshDemand

    // Профиль уже растянут для экземпляра; origin и знаковые координаты штриховок не нормализуем.
    bool ProfileWall_BuildFloorExclusion (const Geometry::MultiPolygon2D &profile,
                                          const API_WallType &wall,
                                          double wallBase,
                                          double zoneBottom,
                                          const GS::Array<OtdOpening> &openings,
                                          Geometry::MultiPolygon2D &exclusion) {
        exclusion.Clear ();
        constexpr double eps = 1.e-6;
        const double dx = wall.endC.x - wall.begC.x, dy = wall.endC.y - wall.begC.y;
        const double length = std::hypot (dx, dy), cutY = zoneBottom - wallBase;
        if (wall.type != APIWtyp_Normal || !std::isfinite (wall.angle) || std::fabs (wall.angle) > eps ||
            !std::isfinite (wall.begC.x) || !std::isfinite (wall.begC.y) || !std::isfinite (length) || length <= eps ||
            !std::isfinite (cutY))
            return false;
        std::vector<std::pair<double, double>> runs = {{0, length}};
        for (const auto &opening : openings) {
            if (!std::isfinite (opening.zBottom) || !std::isfinite (opening.height) || !std::isfinite (opening.width) ||
                !std::isfinite (opening.objLoc) || !std::isfinite (opening.zBottom + opening.height))
                return false;
            if (opening.height <= eps || opening.width <= eps || opening.zBottom > zoneBottom + eps ||
                opening.zBottom + opening.height <= zoneBottom + eps)
                continue;
            const double left = std::max (0.0, opening.objLoc - opening.width / 2);
            const double right = std::min (length, opening.objLoc + opening.width / 2);
            std::vector<std::pair<double, double>> next;
            for (const auto &run : runs) {
                if (right <= run.first || left >= run.second)
                    next.push_back (run);
                else {
                    if (left > run.first)
                        next.push_back ({run.first, left});
                    if (right < run.second)
                        next.push_back ({right, run.second});
                }
            }
            runs = std::move (next);
        }
        // Неразвёрнутый профиль смотрит вправо от beg→end; flipped отражает его относительно оси.
        const double sign = wall.flipped ? 1.0 : -1.0;
        const Vector2D normal = {-dy / length * sign, dx / length * sign};
        Geometry::MultiPolygon2D result;
        for (const auto &polygon : profile) {
            GS::Array<Sector> sections;
            polygon.Intersect (Point2D{0, cutY}, Vector2D{1, 0}, &sections);
            for (const auto &section : sections) {
                const double left = std::min (section.c1.x, section.c2.x);
                const double right = std::max (section.c1.x, section.c2.x);
                if (!std::isfinite (left) || !std::isfinite (right))
                    return false;
                if (right - left <= eps)
                    continue;
                for (const auto &run : runs) {
                    if (run.second - run.first <= eps)
                        continue;
                    const auto at = [&] (double along, double across) -> Point2D {
                        return {wall.begC.x + dx / length * along + normal.x * across,
                                wall.begC.y + dy / length * along + normal.y * across};
                    };
                    GS::Array<Point2D> coords = {
                        at (run.first, left), at (run.second, left), at (run.second, right), at (run.first, right)};
                    result.Append (Geometry::Polygon2D::Create (coords, 0));
                }
            }
        }
        result.Unify ();
        exclusion = std::move (result);
        return true;
    }

    bool FloorContours_SubtractProfileWalls (OtdRoom &room, const Geometry::MultiPolygon2D &exclusion) {
        if (exclusion.IsEmpty ())
            return true;
        GS::Array<OtdSlab> result;
        for (const auto &floor : room.otdslab) {
            if (!floor.isValid || floor.draw.type != Floor || floor.draw.base_type == API_MeshID ||
                std::fabs (floor.slope_x) > 1.e-6 || std::fabs (floor.slope_y) > 1.e-6) {
                result.Push (floor);
                continue;
            }
            Geometry::MultiPolygon2D original;
            original.Push (floor.poly);
            if (Geometry::MultiPolygon2D::Intersect (original, exclusion).IsEmpty ()) {
                result.Push (floor);
                continue;
            }
            const auto remainder = Geometry::MultiPolygon2D::Substract (original, exclusion);
            bool firstPart = true;
            for (const auto &polygon : remainder) {
                OtdSlab part = floor;
                part.poly = polygon;
                // Один существующий элемент нельзя обновлять несколькими раздельными частями.
                if (!firstPart)
                    part.draw.otd_guid = APINULLGuid;
                firstPart = false;
                result.Push (std::move (part));
            }
        }
        room.otdslab = std::move (result);
        return true;
    }

    static bool ProfileWalls_ReadFloorExclusion (const OtdRoom &room,
                                                 RoomProcessingContext &context,
                                                 Geometry::MultiPolygon2D &exclusion) {
        exclusion.Clear ();
        bool horizontalFloor = false;
        for (const auto &floor : room.otdslab)
            horizontalFloor =
                horizontalFloor || (floor.isValid && floor.draw.type == Floor && floor.draw.base_type != API_MeshID &&
                                    std::fabs (floor.slope_x) <= 1.e-6 && std::fabs (floor.slope_y) <= 1.e-6);
        if (!horizontalFloor)
            return true;
        for (const auto &source : room.surfaceCandidates.sources) {
            if (source.baseType != API_WallID)
                continue;
            const ElementRead read = Element_Read (source.baseGuid, &context.surfaceCache);
            if (read.error != NoError)
                return false;
            const auto &wall = read.element.wall;
            if (wall.modelElemStructureType != API_ProfileStructure)
                continue;
            bool foundStory = false;
            double wallBase = 0;
            for (const auto &story : context.storyLevels)
                if (story.index == read.element.header.floorInd) {
                    wallBase = story.level + wall.bottomOffset;
                    foundStory = true;
                    break;
                }
            if (!foundStory || !std::isfinite (wallBase))
                return false;
            API_ElementMemo memo = {};
            const GS::OnExit memoGuard ([&memo] () { ACAPI_DisposeElemMemoHdls (&memo); });
            if (ACAPI_Element_GetMemo (source.baseGuid, &memo, APIMemoMask_StretchedProfile) != NoError ||
                memo.stretchedProfile == nullptr)
                return false;
            Geometry::MultiPolygon2D profile;
            ConstProfileVectorImageIterator it (*memo.stretchedProfile);
            while (!it.IsEOI ()) {
                if (it->item_Typ == SyHatch) {
                    const HatchObject &hatch = it;
                    Geometry::MultiPolygon2D polygons;
                    if (hatch.ToPolygon2D (polygons, HatchObject::VertexAndEdgeData::Omit) != NoError)
                        return false;
                    profile.Append (polygons);
                }
                ++it;
            }
            GS::Array<OtdOpening> openings;
            if (const auto *raw = context.openinginwall.GetPtr (source.baseGuid)) {
                openings = *raw;
                for (auto &opening : openings)
                    opening.zBottom = wallBase + opening.lower;
            }
            Geometry::MultiPolygon2D wallExclusion;
            if (profile.IsEmpty () ||
                !ProfileWall_BuildFloorExclusion (profile, wall, wallBase, room.zBottom, openings, wallExclusion))
                return false;
            exclusion.Append (wallExclusion);
        }
        exclusion.Unify ();
        return true;
    }

    bool MeshFloor_ValidateFinalDomains (const OtdRoom &room, const MixedFloorProvenance &p) {
        using namespace MeshDemand;
        if (p.mesh.contributions.IsEmpty ()) {
            for (const auto &f : room.otdslab)
                if (f.draw.type == Floor && f.draw.base_type == API_MeshID)
                    return false;
            return true;
        }
        for (const auto &c : p.mesh.contributions) {
            if (c.supportIndex >= p.mesh.supports.GetSize () || c.floorIndex >= room.otdslab.GetSize () ||
                c.domain.IsEmpty ())
                return false;
            const auto &s = p.mesh.supports[c.supportIndex];
            const auto &f = room.otdslab[c.floorIndex];
            if (s.ownerType != API_MeshID || s.ownerGuid == APINULLGuid || s.geometryGuid == APINULLGuid ||
                s.bodyIndex <= 0 || s.polygonIndex <= 0 || f.draw.type != Floor || f.draw.base_type != API_MeshID ||
                f.draw.base_guid != s.ownerGuid || !Same (s.plane, f, c.domain) || !Subset (s.admitted, s.projected) ||
                !Subset (c.domain, s.admitted) || !Subset (c.domain, One (f.poly)))
                return false;
        }
        for (const auto &c : p.slab.contributions) {
            if (c.supportIndex >= p.slab.supports.GetSize () || c.floorIndex >= room.otdslab.GetSize () ||
                c.domain.IsEmpty ())
                return false;
            const auto &s = p.slab.supports[c.supportIndex];
            const auto &f = room.otdslab[c.floorIndex];
            if (s.ownerType != API_SlabID || s.ownerGuid == APINULLGuid || f.draw.base_type != API_SlabID ||
                f.draw.base_guid != s.ownerGuid || f.draw.type != Floor || !Same (s.plane, f, c.domain) ||
                !Subset (s.admitted, s.projected) || !Subset (c.domain, s.admitted) || !Subset (c.domain, One (f.poly)))
                return false;
        }
        for (UIndex i = 0; i < room.otdslab.GetSize (); ++i) {
            const auto &f = room.otdslab[i];
            if (f.draw.type != Floor)
                continue;
            MP expected;
            for (const auto &c : p.mesh.contributions)
                if (c.floorIndex == i)
                    expected.Append (c.domain);
            if (expected.IsEmpty () && f.draw.base_type == API_MeshID)
                return false;
            if (!expected.IsEmpty ()) {
                expected.Unify ();
                if (!Subset (One (f.poly), expected))
                    return false;
            }
            for (const auto &c : p.mesh.contributions)
                if (c.floorIndex != i && MP::Intersect (One (f.poly), c.domain).CalcArea () > AT (c.domain))
                    return false;
        }
        return true;
    }

    bool MeshFloor_OpeningDemandIsCovered (const OtdRoom &room,
                                           const OtdWall &wall,
                                           const OtdOpening &o,
                                           const MixedFloorProvenance &p) {
        using namespace MeshDemand;
        const double depth = o.base_reveal_width + o.base_frame_width / 2;
        if (!room.has_floor || !room.create_floor_elements)
            return true;
        if (!MeshFloor_ValidateFinalDomains (room, p))
            return false;
        if (p.mesh.contributions.IsEmpty ())
            return true;
        const double dx = wall.geom.seg.endC.x - wall.geom.seg.begC.x, dy = wall.geom.seg.endC.y - wall.geom.seg.begC.y,
                     L = std::hypot (dx, dy);
        if (!wall.isValid || o.base_guid == APINULLGuid || !std::isfinite (L) || L <= E || !std::isfinite (o.width) ||
            o.width <= E || !std::isfinite (o.objLoc) || !std::isfinite (depth) || depth <= E ||
            !std::isfinite (o.zBottom) || !std::isfinite (o.height) || o.height <= E || o.zBottom > room.zBottom + E)
            return true;
        for (const auto &c : p.mesh.contributions) {
            const auto &f = room.otdslab[c.floorIndex];
            if (!f.isValid || f.draw.type != Floor)
                continue;
            Sector axis;
            if (!Axis (wall, f, axis)) {
                const double base = OtdLinearSegment_GetZBottom (wall.geom);
                const double lo = std::max (0.0, o.zBottom - base);
                const double hi = std::min (wall.geom.height, o.zBottom + o.height - base);
                const double left = std::max (0.0, (o.objLoc - o.width / 2) / L);
                const double right = std::min (1.0, (o.objLoc + o.width / 2) / L);
                if (hi - lo <= E || (right - left) * L <= E)
                    continue;
                OtdWall finite = wall;
                finite.geom.seg.begC = {wall.geom.seg.begC.x + dx * left + wall.rise_x * lo,
                                        wall.geom.seg.begC.y + dy * left + wall.rise_y * lo,
                                        base + lo};
                finite.geom.seg.endC = {wall.geom.seg.begC.x + dx * right + wall.rise_x * lo,
                                        wall.geom.seg.begC.y + dy * right + wall.rise_y * lo,
                                        base + lo};
                finite.geom.height = hi - lo;
                if (NoFiniteContact (finite, f, c.domain))
                    continue;
                return false;
            }
            double a = std::max (0.0, (o.objLoc - o.width / 2) / L), b = std::min (1.0, (o.objLoc + o.width / 2) / L);
            const double base = OtdLinearSegment_GetZBottom (wall.geom);
            if ((b - a) * L <= E || !Band (Z (f, axis.c1), Z (f, axis.c2), o.zBottom, o.zBottom + o.height, a, b) ||
                !Band (Z (f, axis.c1), Z (f, axis.c2), base, base + wall.geom.height, a, b))
                continue;
            Sector entrance = {At (axis, a), At (axis, b)};
            for (const auto &d : c.domain)
                for (const auto &contact : d.CutElem (entrance, Geometry::Boundary_Closed)) {
                    if (std::hypot (contact.c2.x - contact.c1.x, contact.c2.y - contact.c1.y) <= E)
                        continue;
                    if (!Covered (room, f, Strip (contact.c1, contact.c2, dy / L * depth, -dx / L * depth)))
                        return false;
                }
        }
        return true;
    }

    bool MeshFloor_CoreDemandIsCovered (const OtdRoom &room, const MixedFloorProvenance &p) {
        using namespace MeshDemand;
        using namespace FloorCoreGeometry;
        if (!room.has_floor || !room.create_floor_elements)
            return true;
        if (!MeshFloor_ValidateFinalDomains (room, p))
            return false;
        if (p.mesh.contributions.IsEmpty ())
            return true;
        for (UIndex fi = 0; fi < room.otdslab.GetSize (); ++fi) {
            bool mesh = false;
            for (const auto &c : p.mesh.contributions)
                mesh = mesh || c.floorIndex == fi;
            if (!mesh)
                continue;
            const auto &f = room.otdslab[fi];
            if (!f.isValid || f.draw.type != Floor)
                continue;
            std::vector<Contact> contacts;
            std::vector<API_Guid> owners;
            for (const auto &w : room.otdwall) {
                if (!w.isValid || w.draw.base_guid == APINULLGuid || w.draw.type == Reveal_Main ||
                    w.draw.type == Reveal_Up || w.draw.type == Reveal_Down)
                    continue;
                const double offset = w.draw.base_type == API_WallID && !w.slab_step && w.floor_core_offset_valid
                                          ? w.floor_core_offset
                                          : 0;
                Sector s;
                if (!Axis (w, f, s)) {
                    if (offset > E && !NoFiniteContact (w, f, One (f.poly)))
                        return false;
                    continue;
                }
                double a = 0, b = 1;
                const double z0 = Z (f, s.c1), z1 = Z (f, s.c2), base = OtdLinearSegment_GetZBottom (w.geom);
                if (!Band (z0, z1, base, base + w.geom.height, a, b))
                    continue;
                std::vector<std::pair<double, double>> ranges = {{a, b}};
                const double L =
                    std::hypot (w.geom.seg.endC.x - w.geom.seg.begC.x, w.geom.seg.endC.y - w.geom.seg.begC.y);
                if (L <= E)
                    continue;
                for (const auto &o : w.openings) {
                    double x = std::max (0.0, (o.objLoc - o.width / 2) / L),
                           y = std::min (1.0, (o.objLoc + o.width / 2) / L);
                    if (y <= x || !Band (z0, z1, o.zBottom, o.zBottom + o.height, x, y))
                        continue;
                    std::vector<std::pair<double, double>> next;
                    for (const auto &r : ranges) {
                        if (y <= r.first || x >= r.second)
                            next.push_back (r);
                        else {
                            if (x > r.first)
                                next.push_back ({r.first, x});
                            if (y < r.second)
                                next.push_back ({y, r.second});
                        }
                    }
                    ranges = next;
                }
                auto owner = std::find (owners.begin (), owners.end (), w.draw.base_guid);
                if (owner == owners.end ()) {
                    owners.push_back (w.draw.base_guid);
                    owner = owners.end () - 1;
                }
                for (const auto &r : ranges) {
                    auto x = At (s, r.first), y = At (s, r.second);
                    contacts.push_back (
                        {{x.x, x.y}, {y.x, y.y}, offset, static_cast<size_t> (owner - owners.begin ())});
                }
            }
            for (auto contour = f.poly.BeginContour (); contour != f.poly.EndContour (); ++contour) {
                std::vector<Edge> edges;
                for (auto e = contour->BeginEdge (); e != contour->EndEdge (); ++e) {
                    Point2D a, b;
                    double angle = 0;
                    f.poly.GetSector (e, a, b, angle);
                    edges.push_back ({{a.x, a.y}, {b.x, b.y}, std::fabs (angle) <= E});
                }
                for (const auto &patch : Extensions (edges, contacts)) {
                    GS::Array<Point2D> coords;
                    for (const auto &q : patch)
                        coords.Push ({q.x, q.y});
                    if (!Covered (room, f, Geometry::Polygon2D::Create (coords, 0)))
                        return false;
                }
            }
        }
        return true;
    }

    // Один раз на комнату: сверка исходного снимка без host-чтений и повторного поиска победителя.
    bool MeshFloor_ValidateSourceIdentity (const OtdRoom &room, const MixedFloorProvenance &p) {
        using namespace MeshDemand;
        for (const auto &support : p.mesh.supports) {
            const MeshFloorSource *matched = nullptr;
            for (const auto &mesh : room.meshFloorSources)
                if (mesh.surface.baseGuid == support.ownerGuid) {
                    if (matched != nullptr)
                        return false;
                    matched = &mesh;
                }
            if (matched == nullptr)
                return false;
            const auto &source = matched->surface;
            if (source.baseType != API_MeshID || source.baseGuid == APINULLGuid ||
                source.geometryGuid != source.baseGuid || source.snapshot.guid != source.baseGuid ||
                !source.snapshot.IsComplete () || support.ownerType != source.baseType ||
                support.geometryGuid != source.geometryGuid || support.bodyIndex <= 0 || support.polygonIndex <= 0)
                return false;
            const RoomSurfaceBody *body = nullptr;
            const RoomSurface *face = nullptr;
            for (const auto &b : source.snapshot.bodies)
                if (b.index == support.bodyIndex) {
                    if (body != nullptr)
                        return false;
                    body = &b;
                }
            if (body == nullptr)
                return false;
            for (const auto &f : body->surfaces)
                if (f.bodyIndex == support.bodyIndex && f.polygonIndex == support.polygonIndex) {
                    if (face != nullptr)
                        return false;
                    face = &f;
                }
            if (face == nullptr || face->origin != RoomSurfaceOrigin::Native3D ||
                face->normalWorld.z <= DistanceTolerance || face->modelMaterialIndex != support.modelMaterialIndex)
                return false;
            OtdSlab plane, level;
            level.zBottom = room.zBottom;
            MP projected, admitted;
            if (!FloorFace_Project (*face, plane, projected))
                return false;
            if (matched->hasBottom) {
                MP bottoms;
                bool found = false;
                for (const auto &f : body->surfaces) {
                    if (!Coord3D_IsFinite (f.normalWorld))
                        return false;
                    if (f.normalWorld.z >= -DistanceTolerance)
                        continue;
                    found = true;
                    OtdSlab bottomPlane;
                    MP bottom;
                    if (!FloorFace_Project (f, bottomPlane, bottom) ||
                        !FloorPolygons_ClipBelow (bottom, bottomPlane, level, bottoms))
                        return false;
                }
                if (!found)
                    return false;
                bottoms.Unify ();
                admitted = MP::Intersect (projected, bottoms);
            } else if (!FloorPolygons_ClipBelow (projected, plane, level, admitted))
                return false;
            if (!Same (plane, support.plane, projected) || !Subset (projected, support.projected) ||
                !Subset (support.projected, projected) || !Subset (admitted, support.admitted) ||
                !Subset (support.admitted, admitted))
                return false;
        }
        return true;
    }

    // Подготовка оснований, заданий чтения и отделки для общего контекста запуска.
    // AC25 has only typeID/filterBits/loc/z (no variation or additional request fields).
    // Both callers zero-initialize the SDK request, then set every one of these fields.
    static GSErrCode CoordinateSearch_Search (API_ElemSearchPars &request,
                                              API_Guid &guid,
                                              CoordinateSearchCache *cache) {
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

    // Собственные конечные грани: нулевые native-индексы не обозначают SDK-тело/полигон.
    static RoomSurface RoomSurface_CreateAnalyticFace (const GS::Array<API_Coord3D> &points,
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
    static bool RoomSurfaceBody_AppendAnalyticRectangles (const std::vector<AnalyticGeometry::Rect> &rectangles,
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

    // Предикат проверки обычной (не профильной, прямой, вертикальной, прямоугольной) стены
    static bool OtdWall_IsOrdinary (const API_Element &element) {
        if (GetElemTypeID (element.header) != API_WallID)
            return false;
        const auto &wall = element.wall;
        return AnalyticGeometry::OrdinaryWall (wall.modelElemStructureType == API_ProfileStructure,
                                               wall.angle == 0,
                                               wall.profileType == APISect_Normal,
                                               wall.type == APIWtyp_Normal,
                                               wall.logHeight != 0,
                                               wall.relativeTopStory != 0);
    }

    bool OtdWall_BuildPreparedSource (const API_Element &element,
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
            return false;
        }
        if ((wall.hasWindow || wall.hasDoor) && openings.IsEmpty ()) {
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
            return false;
        }
        const double length = std::hypot (wall.endC.x - wall.begC.x, wall.endC.y - wall.begC.y);
        std::vector<AnalyticGeometry::Rect> cuts;
        for (const auto &op : openings)
            cuts.push_back (
                {op.objLoc - op.width / 2, bottom + op.lower, op.objLoc + op.width / 2, bottom + op.lower + op.height});
        std::vector<AnalyticGeometry::Rect> rectangles;
        if (!AnalyticGeometry::SubtractOpenings ({0, bottom, length, bottom + wall.height}, cuts, rectangles)) {
            return false;
        }
        RoomSurfaceBody body;
        GS::Array<OtdWall> prepared;
        Int32 faceId = 0;
        const AnalyticGeometry::Point starts[] = {ob, ie, oe, ib};
        const AnalyticGeometry::Point ends[] = {oe, ib, ie, ob};
        for (Int32 side = 0; side < 4; ++side) {
            const auto &beg = starts[side];
            const auto &end = ends[side];
            const double sideLength = std::hypot (end.x - beg.x, end.y - beg.y);
            if (sideLength <= 0 || !std::isfinite (sideLength)) {
                return false;
            }
            // Ориентация контура согласована с наружной нормалью и flipped сечения.
            const double sign = wall.flipped ? -1 : 1;
            const API_Coord3D normal = {-(end.y - beg.y) / sideLength * sign, (end.x - beg.x) / sideLength * sign, 0};
            OtdWall base;
            base.geom.seg.begC = {beg.x, beg.y, bottom};
            base.geom.seg.endC = {end.x, end.y, bottom};
            base.geom.height = wall.height;
            base.geom.floorInd = element.header.floorInd;
            base.base_th = wall.thickness;
            base.draw.base_guid = source.baseGuid;
            base.draw.base_type = API_WallID;
            base.draw.draw_type = API_WallID;
            base.draw.type = Wall_Main;
            base.base_flipped = side == 1;
            if (side < 2)
                for (const auto &op : openings)
                    OtdOpening_Add (op, side == 1, bottom, 0, length, length, base);
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
            if (!RoomSurfaceBody_AppendAnalyticRectangles (
                    sideRectangles,
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
            if (!RoomSurfaceBody_AppendAnalyticRectangles (caps,
                                                           {ob.x, ob.y, z},
                                                           {(oe.x - ob.x) / length, (oe.y - ob.y) / length, 0},
                                                           {ib.x - ob.x, ib.y - ob.y, 0},
                                                           {0, 0, z == bottom ? -1.0 : 1.0},
                                                           faceId,
                                                           body))
                return false;
        }
        source.origin = RoomSurfaceOrigin::PreparedAnalytic;
        source.snapshot.guid = source.baseGuid;
        source.snapshot.status = RoomSurfaceStatus::Extracted;
        source.snapshot.bodies.Push (body);
        bases.Append (prepared);
        return true;
    }

    bool OtdSlab_BuildPreparedSource (const API_Element &element,
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
            return false;
        }
        // Оригинальные дуги остаются в OtdSlab для пересечений и writer; хорды нужны только partition.
        const auto segmented = polygon.SegmentUp (ArcDivParams (0.00001));
        if (segmented.IsEmpty ()) {
            return false;
        }
        RoomSurfaceBody body;
        Int32 faceId = 0;
        for (const auto &part : segmented) {
            GS::Array<Point2D> coords;
            GS::Array<PolyArcRec> arcs;
            GS::Array<UIndex> ends;
            GS::Array<Geometry::Polygon2D::ConstContourIterator> reversed;
            part.CopyPolygonData (coords, arcs, ends, reversed, Geometry::PolyCreateFlags_IdxRefInContourEndFromZero);
            if (!arcs.IsEmpty () || ends.IsEmpty ()) {
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
                    body.surfaces.Push (
                        RoomSurface_CreateAnalyticFace ({{curr.x, curr.y, bottom},
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
        slab.draw.base_guid = source.baseGuid;
        slab.draw.base_type = API_SlabID;
        slab.draw.draw_type = API_SlabID;
        slab.floorInd = element.header.floorInd;
        bases.Push (slab);
        source.origin = RoomSurfaceOrigin::PreparedAnalytic;
        source.snapshot.guid = source.baseGuid;
        source.snapshot.status = RoomSurfaceStatus::Extracted;
        source.snapshot.bodies.Push (body);
        return true;
    }

    // Перекрытия читаются только из polygon memo; native fallback допустим лишь для стен.
    static RoomSurfaceSource RoomSurfaceSource_Prepare (const API_Element &element,
                                                        const Stories &stories,
                                                        const GS::Array<OtdOpening> &openings,
                                                        GS::Array<OtdWall> &walls,
                                                        GS::Array<OtdSlab> &slabs,
                                                        RoomSurfaceCache *cache) {
        RoomSurfaceSource source;
        bool analytic = false;
        if (GetElemTypeID (element.header) == API_WallID) {
            const auto genericOpenings =
                ConnectedElements_Read (element.header.guid, API_OpeningID, APIFilt_IsVisibleByRenovation, cache);
            if (genericOpenings.error != NoError || !genericOpenings.items.IsEmpty ()) {
                source.baseGuid = source.geometryGuid = element.header.guid;
                source.baseType = API_WallID;
                source.snapshot = RoomSurfaces_Read (element.header, cache);
                return source;
            }
            analytic =
                OtdWall_BuildPreparedSource (element,
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
                analytic = OtdSlab_BuildPreparedSource (
                    element, GetzPos (element.slab.level, element.header.floorInd, stories), polygon, slabs, source);
            else {
                source.baseGuid = source.geometryGuid = element.header.guid;
                source.baseType = API_SlabID;
            }
        }
        if (!analytic && GetElemTypeID (element.header) == API_WallID) {
            source.snapshot = RoomSurfaces_Read (element.header, cache);
        }
        return source;
    }

    // Находит классификации, уровни этажей и отдельные индексы Slab/Mesh по конечным контурам зон.
    bool RoomProcessingContext_Prepare (RoomProcessingContext &context, const GS::Array<API_Guid> &zones) {
        context.elementTypes = typeinzone;
        context.elementTypeCount = sizeof (typeinzone) / sizeof (typeinzone[0]);
        Class_FindFinClass (context.finclass, context.finclassguids);
        context.storyLevels = GetStories ();
        OtdSlab_FindAll (context.slabsinzone, context.finclassguids, zones, &context.surfaceCache);
        GS::HashTable<API_Guid, Geometry::Polygon2D> zonePolygons;
        for (const auto &zoneGuid : zones) {
            API_ElementMemo memo = {};
            const GS::OnExit memoGuard ([&memo] () { ACAPI_DisposeElemMemoHdls (&memo); });
            Geometry::Polygon2D polygon;
            const GSErrCode memoError = ACAPI_Element_GetMemo (zoneGuid, &memo, APIMemoMask_Polygon);
            const GSErrCode polygonError =
                memoError == NoError ? ConstructPolygon2DFromElementMemo (memo, polygon) : memoError;
            if (polygonError != NoError) {
                msg_rep ("Mesh discovery", "Zone polygon unavailable; stopped before writing", polygonError, zoneGuid);
                return false;
            }
            zonePolygons.Add (zoneGuid, polygon);
        }
        return Mesh_FindAll (context.meshesinzone, context.finclassguids, zonePolygons, &context.surfaceCache);
    }

    // Собирает данные комнат и связи с базовыми элементами, добавляет зоны в список чтения и очищает временные GUID
    // зон. Прерывается с false при отмене очередной фазы.
    bool RoomProcessingContext_BuildElementReadIndex (const GS::Array<API_Guid> &zones,
                                                      RoomProcessingContext &context,
                                                      GS::UniString &funcname,
                                                      GS::Int32 &nPhase) {
        // Один точный кэш на read-фазу запуска; discovery продолжается после чтения проёмов.
        auto &coordinateCache = context.coordinateCache;
        for (const API_Guid &zoneGuid : zones) {
            if (AdvanceProcessPhase (funcname, nPhase))
                return false;
            OtdRoom roominfo;
            if (OtdRoom_CollectInfo (context.storyLevels,
                                     zoneGuid,
                                     roominfo,
                                     context.elementToRead,
                                     context.slabsinzone,
                                     &context.surfaceCache,
                                     &coordinateCache,
                                     &context.meshesinzone)) {
                API_Elem_Head zone = {};
                zone.guid = zoneGuid;
                SetElemTypeID (zone, API_ZoneID);
                GS::Array<API_Guid> finishClasses;
                context.finclassguids.EnumerateKeys (
                    [&] (const API_Guid &classGuid) { finishClasses.Push (classGuid); });
                roominfo.surfaceCandidates = RoomSurfaceCandidates_Read (zone, finishClasses, &context.surfaceCache);
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
                for (const auto &mesh : roominfo.meshFloorSources)
                    Param_AddUnicGUIDByType (mesh.surface.baseGuid, API_MeshID, context.guidselementToRead);
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
                        ConnectedElements_Read (wallGuid, type, APIFilt_IsVisibleByRenovation, &context.surfaceCache);
                    if (openings.error != NoError)
                        return false;
                    for (const auto &openingGuid : openings.items)
                        for (const auto &roomGuid : roomGuids)
                            Param_AddUnicElementByType (openingGuid, roomGuid, type, context.elementToRead);
                }
            }
        }
        context.guidselementToRead.Add (API_ZoneID, zones);
        Zone_ClearDuplicateGUIDs (context.elementToRead);
        return true;
    }

    // Обрабатывает собранные проёмы, стены, колонны и перекрытия, пропуская элементы с классификацией отделки.
    // Заполняет данные комнат и списки чтения; при отмене возвращает false.
    bool RoomProcessingContext_ProcessElementsForRoomData (RoomProcessingContext &context,
                                                           GS::UniString &funcname,
                                                           GS::Int32 &nPhase) {
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
                        OtdSlab_FindInRoom (context.storyLevels,
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
        for (auto it = context.meshesinzone.EnumeratePairs (); it != nullptr; ++it) {
    #ifdef ServerMainVers_2800
            const auto &sources = it->value;
    #else
            const auto &sources = *it->value;
    #endif
            for (const auto &source : sources)
                Param_AddUnicGUIDByType (source.surface.baseGuid, source.surface.baseType, context.guidselementToRead);
        }
        return true;
    }

    // До очистки writer-результатов сохраняем неизменяемые основания и legacy-связи откосов.
    bool RoomProcessingContext_PreparePartitions (RoomProcessingContext &context) {
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
                const ElementRead elementRead = Element_Read (source.baseGuid, &context.surfaceCache);
                if (elementRead.error != NoError)
                    return false;
                const API_Element &element = elementRead.element;
                const auto *openings = context.openinginwall.GetPtr (source.baseGuid);
                source = RoomSurfaceSource_Prepare (element,
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
            room.surfacePartition = RoomSurfaces_Partition (room.surfaceSnapshot, room.surfaceCandidates.sources);
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

    // Формирует задания чтения параметров для зон, окон и базовых конструкций из собранных GUID.
    // Набор параметров базовой конструкции также сохраняет для поиска избранного.
    void RoomProcessingContext_ReadElementParameters (RoomProcessingContext &context,
                                                      ReadParamsForRoomBook &readParams) {
        ParamDictValue paramDict;
        const API_ElemTypeID readTypes[] = {
            API_WindowID, API_DoorID, API_WallID, API_ColumnID, API_SlabID, API_MeshID, API_BeamID, API_ZoneID};
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
                typeelem == API_BeamID || typeelem == API_MeshID) {
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
    static void OtdSlab_ProcessFinishes (OtdRoom &otd,
                                         const API_Guid &zoneGuid,
                                         ParamDictElement &paramToRead,
                                         ParamValue &param_composite,
                                         ParamDictValue &paramDict_favorite,
                                         GS::HashTable<API_Guid, UnicGuidByBase> &exsistot_byzone,
                                         ParamDictCompositeElement &paramCompositeToRead,

                                         bool has_base_element,
                                         MatarialToFavoriteDict &favdict) {

        for (OtdSlab &otdslab : otd.otdslab) {
            bool base_flipped = false;
            GS::UniString fav_name;
            if (!Param_SetToBase (otdslab.draw.base_guid,
                                  base_flipped,
                                  otdslab.base_composite,
                                  paramToRead,
                                  paramCompositeToRead,
                                  param_composite,
                                  fav_name))
                otdslab.isValid = false;
            if (!otdslab.isValid)
                continue;
            if (otdslab.draw.type == Ceil)
                otdslab.tip = otd.tip_ceil;
            if (otdslab.draw.type == Floor)
                otdslab.tip = otd.tip_floor;
            otdslab.draw.favorite.name = fav_name;
            OtdMaterial_SetFinish (otdslab.draw.material, otdslab.base_composite);
            Favorite_FindName (otdslab.draw.favorite,
                               otdslab.draw.material,
                               otdslab.draw.type,
                               otdslab.draw.base_type == API_MeshID ? API_SlabID : otdslab.draw.draw_type,
                               paramDict_favorite,
                               param_composite,
                               otdslab.tip,
                               favdict);
            // Roof сохраняет выбранный Slab-состав и тот же материальный результат, что стеновой маршрут.
            if (otdslab.draw.base_type == API_MeshID && otdslab.draw.favorite.is_composite_read &&
                !otdslab.draw.favorite.composite.IsEmpty ()) {
                OtdMaterial_SetFinishByComposite (otdslab.draw.material, otdslab.draw.favorite.composite);
                otdslab.base_composite.Append (otdslab.draw.favorite.composite);
            }
            // Ищем существующие элементы по GUID базового элемента
            if (!otdslab.surface_geometry && exsistot_byzone.ContainsKey (zoneGuid) && has_base_element) {
                UnicGuidByBase &exsistot_byparent = exsistot_byzone.Get (zoneGuid);
                if (exsistot_byparent.ContainsKey (otdslab.draw.base_guid)) {
                    UnicGuidByTypeOtd &exsistype = exsistot_byparent.Get (otdslab.draw.base_guid);
                    if (exsistype.ContainsKey (otdslab.draw.type)) {
                        UnicGuid &exsistot = exsistype.Get (otdslab.draw.type);
                        GS::Optional<API_Guid> v = exsistot.FindValue (true);
                        if (v.HasValue ()) {
                            otdslab.draw.otd_guid = v.Get ();
                            exsistot.Delete (otdslab.draw.otd_guid);
                        }
                    } else {
                        if (exsistype.ContainsKey (NoSet)) {
                            UnicGuid &exsistot = exsistype.Get (NoSet);
                            GS::Optional<API_Guid> v = exsistot.FindValue (true);
                            if (v.HasValue ()) {
                                otdslab.draw.otd_guid = v.Get ();
                                exsistot.Delete (otdslab.draw.otd_guid);
                            }
                        }
                    }
                }
            }
        } // Обработка перекрытий
    }

    // Подбирает избранное для допустимых стен; прочитанный непустой состав избранного добавляет к базовому составу и
    // учитывает в материале.
    static void OtdWall_ApplyFavoriteAndMaterialData (GS::Array<OtdWall> &opw,
                                                      ParamDictValue &paramDict_favorite,
                                                      ParamValue &param_composite,
                                                      MatarialToFavoriteDict &favdict) {
        GS::UniString tip;
        for (OtdWall &otdw : opw) {
            if (!otdw.isValid)
                continue;
            Favorite_FindName (otdw.draw.favorite,
                               otdw.draw.material,
                               otdw.draw.type,
                               otdw.draw.draw_type,
                               paramDict_favorite,
                               param_composite,
                               tip,
                               favdict);
            if (otdw.draw.favorite.is_composite_read && !otdw.draw.favorite.composite.IsEmpty ()) {
                OtdMaterial_SetFinishByComposite (otdw.draw.material, otdw.draw.favorite.composite);
                otdw.base_composite.Append (otdw.draw.favorite.composite);
            }
        } // Назначение избранного
    }

    // Читает состав стен, рассчитывает откосы проёмов и разбивает стены по высотным участкам помещения.
    // Назначает избранное и заменяет исходный массив стен рассчитанными участками.
    static bool OtdWall_ProcessFinishes (OtdRoom &otd,
                                         const GS::Array<OtdWall> &openingWalls,
                                         ParamDictElement &paramToRead,
                                         ParamValue &param_composite,
                                         ParamDictValue &paramDict_favorite,
                                         ReadParams &windowParams,
                                         ParamDictCompositeElement &paramCompositeToRead,
                                         const MixedFloorProvenance &floorProvenance,
                                         MatarialToFavoriteDict &favdict) {

        GS::Array<OtdWall> opw; // Подготовленные стены и откосы; native-элементы ещё не созданы.
        // Состав оснований и исходные участки стен.
        GS::Array<OtdWall> openingOwners = openingWalls;
        openingOwners.Append (otd.preparedWalls);
        // Legacy-участки нужны только для откосов; отверстия surface-стен уже вырезаны.
        GS::Array<OtdWall> wallsForRules = otd.otdwall;
        const USize finishWallCount = wallsForRules.GetSize ();
        wallsForRules.Append (openingWalls);
        for (UIndex wallIndex = 0; wallIndex < wallsForRules.GetSize (); ++wallIndex) {
            OtdWall &otdw = wallsForRules[wallIndex];
            // Заполняем данные для отделочных стен (состав)
            GS::UniString fav_name;
            if (!Param_SetToBase (otdw.draw.base_guid,
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
            otdw.draw.favorite.name = fav_name;
            // Параметры проёмов, расширение пола и подготовка откосов.
            if (otdw.openings.IsEmpty () || otdw.surface_geometry)
                continue;
            Point2D wbegC = {otdw.geom.seg.begC.x, otdw.geom.seg.begC.y};
            Point2D wendC = {otdw.geom.seg.endC.x, otdw.geom.seg.endC.y};
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
                // Параметры нужны обеим копиям стены; откосы и расширение пола выпускает только исходный владелец.
                if (wallIndex < finishWallCount && !openingWalls.IsEmpty ())
                    continue;
                // Пол расширяется до проверки флага откосов: выключенный откос не отменяет расширение.
                if (!FloorContour_ExtendToOpening (otd, otdw, op))
                    msg_rep ("Floor opening extension",
                             "Contour union failed; original floor retained",
                             APIERR_BADPARS,
                             otd.zone_guid);
                if (!otd.create_reveal_elements || !op.create_reveal)
                    continue;
                OtdOpening_CreateReveals (otd.otdslab,
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
        // Смещение пола до ядра основания.
        // Surface-адаптер не знает сторону многослойки; берём только отдельно
        // рассчитанную глубину legacy-стены того же основания/направления.
        // Материальные правила и геометрию surface-стены не меняем.
        for (auto &target : wallsForRules) {
            if (!target.surface_geometry || target.draw.base_type != API_WallID || target.slab_step)
                continue;
            bool offsetFound = false, offsetAmbiguous = false;
            double matchedOffset = 0;
            const double targetDx = target.geom.seg.endC.x - target.geom.seg.begC.x,
                         targetDy = target.geom.seg.endC.y - target.geom.seg.begC.y;
            for (const auto &source : wallsForRules) {
                if (source.surface_geometry || !source.isValid || !source.floor_core_offset_valid ||
                    source.draw.base_guid != target.draw.base_guid)
                    continue;
                const double sourceDx = source.geom.seg.endC.x - source.geom.seg.begC.x,
                             sourceDy = source.geom.seg.endC.y - source.geom.seg.begC.y;
                const double directionLengthProduct = std::hypot (targetDx, targetDy) * std::hypot (sourceDx, sourceDy);
                if (directionLengthProduct <= 1.e-12)
                    continue;
                const double directionDot = targetDx * sourceDx + targetDy * sourceDy;
                if (directionDot <= 0)
                    continue; // Обратное или перпендикулярное направление.
                const double directionCross = targetDx * sourceDy - targetDy * sourceDx;
                const double parallelTolerance = 1.e-6 * directionLengthProduct;
                if (std::fabs (directionCross) > parallelTolerance)
                    continue;
                if (offsetFound && std::fabs (matchedOffset - source.floor_core_offset) > 1.e-6)
                    offsetAmbiguous = true;
                offsetFound = true;
                matchedOffset = source.floor_core_offset;
            }
            target.floor_core_offset_valid = offsetFound && !offsetAmbiguous;
            target.floor_core_offset = target.floor_core_offset_valid ? matchedOffset : 0;
        }
        // Отверстия и высотные полосы основной отделки; служебные владельцы проёмов не создают стены.
        for (UIndex wallIndex = 0; wallIndex < finishWallCount; ++wallIndex) {
            OtdWall &otdw = wallsForRules[wallIndex];
            // Основную отделку строим по поверхностям (для сложных стен/балок) или по аналитическим участкам (для
            // обычных стен).
            if (!otdw.isValid)
                continue;
            if (otdw.surface_geometry) {
                bool analytic = false;
                for (const auto &source : otd.surfaceCandidates.sources)
                    analytic = analytic || (source.baseGuid == otdw.draw.base_guid &&
                                            source.origin == RoomSurfaceOrigin::PreparedAnalytic);
                if (!SurfaceOpenings_Link (otdw, openingOwners, analytic)) {
                    otdw.isValid = false;
                    msg_rep ("Surface finish",
                             "Opening ownership is ambiguous or unrepresentable",
                             APIERR_BADPARS,
                             otdw.draw.base_guid);
                    continue;
                }
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
        // Избранное и окончательный состав материалов: добавленные слои затем участвуют в ведомости.
        OtdWall_ApplyFavoriteAndMaterialData (opw, paramDict_favorite, param_composite, favdict);
        // Допустимость стены-перепада для выбранного избранного.
        // Проверяем тот же выбор избранного/defaults и запрет профиля, что и штатный surface-writer.
        for (auto &wall : opw) {
            if (!wall.slab_step || !wall.isValid)
                continue;
            API_Element settings = {};
            wall.slab_step_writer_eligible =
                Wall_GetDefault (wall.draw.favorite.name, wall.draw.type, settings, false) &&
                settings.wall.modelElemStructureType != API_ProfileStructure;
        }
        // Окончательный массив подготовленной отделки; создание в модели выполняется позже.
        otd.otdwall = std::move (opw);
        return true;
    }

    // Вырезы контакта представлены проёмами; legacy-проёмы повторно не добавляются.
    static bool OtdRoom_AddSurfaceFinishPiece (OtdRoom &room,
                                               const RoomSurfacePiece &piece,
                                               RoomSurfaceCache *cache,
                                               TypeOtd forceType = NoSet) {
        if (piece.status != RoomSurfacePieceStatus::Matched || piece.baseGuid == APINULLGuid)
            return false;
        GS::Array<OtdWall> walls;
        if (SurfaceWalls_Build (piece, room.floorInd, walls)) {
            ElementRead baseRead;
            if (piece.baseType == API_WallID) {
                baseRead = Element_Read (piece.baseGuid, cache);
                if (baseRead.error != NoError)
                    return false;
            }
            const API_Element &base = baseRead.element;
            for (auto &wall : walls) {
                if (piece.baseType == API_WallID) {
                    const double dx = base.wall.endC.x - base.wall.begC.x;
                    const double dy = base.wall.endC.y - base.wall.begC.y;
                    const double wx = wall.geom.seg.endC.x - wall.geom.seg.begC.x;
                    const double wy = wall.geom.seg.endC.y - wall.geom.seg.begC.y;
                    // Торец не имеет стороны многослойки; боковая грань использует исходное направление.
                    if (std::fabs (dx * wy - dy * wx) > 1e-6 * std::hypot (dx, dy) * std::hypot (wx, wy))
                        continue; // Из-за одной ошибки нельзя прерывать всю работу
                    wall.base_flipped = (dx * wx + dy * wy < 0) != base.wall.flipped;
                    wall.base_th = base.wall.thickness;
                }
                if (forceType != NoSet)
                    wall.draw.type = forceType;
            }
            room.otdwall.Append (walls);
            return true;
        }
        GS::Array<OtdSlab> slabs;
        if (!SurfaceSlabs_Build (piece, room.floorInd, slabs))
            return false;
        for (auto &slab : slabs) {
            if (forceType != NoSet)
                slab.draw.type = forceType;
            if ((slab.draw.type == Floor && !room.has_floor) || (slab.draw.type == Ceil && !room.has_ceil))
                continue;
            slab.draw.material = slab.draw.type == Floor ? room.om_floor : room.om_ceil;
            room.otdslab.Push (std::move (slab));
        }
        return true;
    }

    static void OtdRoom_CollectOrdinaryWalls (const OtdRoom &room,
                                              RoomSurfaceCache *cache,
                                              GS::Array<OtdWall> &ordinaryWalls) {
        // Обычные (не профильные) стены сохраняем из аналитического расчёта по wallPart (как в v1.77).
        // 3D-поверхности строятся только для сложных стен (профили, наклонные, дуги) и балок.
        for (const auto &wall : room.otdwall) {
            if (wall.draw.base_type == API_WallID) {
                const auto base = Element_Read (wall.draw.base_guid, cache);
                if (base.error == NoError && OtdWall_IsOrdinary (base.element)) {
                    ordinaryWalls.Push (wall);
                }
            }
        }
    }

    static bool OtdRoom_BuildHorizontalFinishes (OtdRoom &room, MixedFloorProvenance &floorProvenance) {
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
        nominalFloor.draw.material = room.om_floor;
        OtdSlab nominalCeiling = room.poly;
        nominalCeiling.zBottom = room.zBottom + room.height_ceil;
        nominalCeiling.draw.material = room.om_ceil;
        double rejectedSlabArea = 0;
        if (room.has_floor && !room.meshFloorSources.IsEmpty ()) {
            MixedFloorGeometry mixed;
            if (!OtdSlab_BuildFiniteMixedGeometryWithProvenance (room.surfaceSnapshot,
                                                                 slabSources,
                                                                 floors,
                                                                 room.floorInd,
                                                                 nominalFloor,
                                                                 room.floor_by_slab,
                                                                 room.preparedSlabs,
                                                                 room.zBottom,
                                                                 room.meshFloorSources,
                                                                 mixed,
                                                                 floorProvenance))
                return false;
            slabFinishes.Append (mixed.floors);
            room.otdwall.Append (mixed.steps);
            rejectedSlabArea += mixed.rejectedArea;
        } else if (room.has_floor && !OtdSlab_BuildProjectedFinishes (room.surfaceSnapshot,
                                                                      slabSources,
                                                                      floors,
                                                                      Floor,
                                                                      room.floorInd,
                                                                      slabFinishes,
                                                                      rejectedSlabArea,
                                                                      &nominalFloor,
                                                                      room.floor_by_slab,
                                                                      &room.otdwall,
                                                                      &room.preparedSlabs)) {
            msg_rep ("Surface finish", "Invalid original slab projection", APIERR_BADPARS, room.zone_guid);
            return false;
        }
        if (room.has_ceil && !OtdSlab_BuildProjectedFinishes (room.surfaceSnapshot,
                                                              slabSources,
                                                              ceilings,
                                                              Ceil,
                                                              room.floorInd,
                                                              slabFinishes,
                                                              rejectedSlabArea,
                                                              &nominalCeiling,
                                                              room.ceil_by_slab,
                                                              &room.otdwall,
                                                              &room.preparedSlabs)) {
            msg_rep ("Surface finish", "Invalid original slab projection", APIERR_BADPARS, room.zone_guid);
            return false;
        }
        if (rejectedSlabArea > 1e-6)
            msg_rep ("Surface finish", "Ambiguous projected slab ownership skipped", APIERR_BADPARS, room.zone_guid);
        for (auto &slab : slabFinishes) {
            slab.draw.material = slab.draw.type == Floor ? room.om_floor : room.om_ceil;
            room.otdslab.Push (std::move (slab));
        }
        return true;
    }

    static bool OtdRoom_BuildPartitionWallFinishes (OtdRoom &room,
                                                    RoomSurfaceCache *cache,
                                                    bool &hasSkippedPieces,
                                                    bool &hasAmbiguousPieces) {
        // Убираем лишь внутренние швы совместимых контактов; исходный partition сохраняет provenance.
        GS::Array<RoomSurfacePiece> grouped;
        if (!SurfaceWallPieces_Group (room.surfacePartition.pieces, room.floorInd, grouped)) {
            msg_rep ("Surface finish", "Invalid coplanar wall union", APIERR_BADPARS, room.zone_guid);
            return false;
        }
        for (const auto &piece : grouped) {
            // Незаявленная поверхность не является отказом writer; владельца неоднозначной не выдумываем.
            if (piece.status == RoomSurfacePieceStatus::Unmatched && piece.sources.IsEmpty ())
                continue;
            if (piece.status == RoomSurfacePieceStatus::Ambiguous) {
                hasAmbiguousPieces = true;
                continue;
            }
            // beamPart — единственный маршрут балок: не дублируем совпавшие грани partition.
            if (piece.baseType == API_BeamID)
                continue;
            // Только горизонтальные slab-контакты заменены проекционным маршрутом.
            if (piece.baseType == API_SlabID && std::hypot (piece.normal.x, piece.normal.y) <= 1e-6)
                continue;
            // Грани обычных (не профильных) стен уже построены аналитически из wallPart.
            if (piece.baseType == API_WallID) {
                const auto base = Element_Read (piece.baseGuid, cache);
                if (base.error == NoError && OtdWall_IsOrdinary (base.element))
                    continue;
            }
            if (!OtdRoom_AddSurfaceFinishPiece (room, piece, cache)) {
                hasSkippedPieces = true;
            }
        }
        // Сверка выполняется после сбора всех граней: соседние куски задают внутренние разрывы.
        GS::Array<OtdWall> correctedWalls;
        for (const auto &wall : room.otdwall) {
            if (wall.draw.base_type != API_WallID || !wall.surface_geometry || wall.slab_step) {
                correctedWalls.Push (wall);
                continue;
            }
            const auto base = Element_Read (wall.draw.base_guid, cache);
            if (base.error != NoError)
                continue; // Из-за одной ошибки нельзя прерывать всю работу
            // Для дуги tBeg/tEnd нельзя интерпретировать как расстояния по хорде.
            if (base.element.wall.zoneRel == APIZRel_None || base.element.wall.angle != 0) {
                correctedWalls.Push (wall);
                continue;
            }
            const auto corrected = SurfaceWall_ClampToParts (wall,
                                                             base.element.wall.begC,
                                                             base.element.wall.endC,
                                                             room.wallPart,
                                                             room.edges,
                                                             room.zBottom,
                                                             room.otdwall);
            if (corrected.IsEmpty ()) {
                hasSkippedPieces = true;
                msg_rep ("Surface finish", "WallPart contact rejected", APIERR_BADPARS, wall.draw.base_guid);
            }
            correctedWalls.Append (corrected);
        }
        room.otdwall = std::move (correctedWalls);
        return true;
    }

    static void OtdRoom_BuildBeamFinishes (OtdRoom &room,
                                           const UnicGuid &finclassguids,
                                           RoomSurfaceCache *cache,
                                           bool &hasSkippedPieces) {
        GS::HashTable<API_Guid, const API_BeamPart *> shortestBeamParts;
        for (const auto &part : room.surfaceCandidates.beamParts) {
            if (!std::isfinite (part.tBeg) || !std::isfinite (part.tEnd) || part.tEnd < part.tBeg)
                continue;
            auto shortest = shortestBeamParts.GetPtr (part.guid);
            if (shortest == nullptr)
                shortestBeamParts.Add (part.guid, &part);
            else if (part.tEnd - part.tBeg < (*shortest)->tEnd - (*shortest)->tBeg)
                *shortest = &part;
        }
        GS::HashTable<API_Guid, bool> wholeZoneBeams;
        for (const auto &part : room.surfaceCandidates.beamParts) {
            if (!ACAPI_Element_Filter (
                    part.guid, APIFilt_IsInStructureDisplay | APIFilt_IsVisibleByRenovation | APIFilt_OnVisLayer))
                continue;
            API_Guid classguid = APINULLGuid;
            if (Class_IsElementFinClass (part.guid, finclassguids, classguid, cache))
                continue;
            if (!std::isfinite (part.tBeg) || !std::isfinite (part.tEnd) || part.tEnd < part.tBeg) {
                msg_rep ("Surface finish", "Invalid beam contact interval", APIERR_BADPARS, part.guid);
                continue;
            }
            if (wholeZoneBeams.ContainsKey (part.guid))
                continue;
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
            const ElementRead beamRead = Element_Read (part.guid, cache);
            if (beamRead.error != NoError) {
                msg_rep ("Surface finish", "Failed to read beam", beamRead.error, part.guid);
                continue; // Из-за одной ошибки нельзя прерывать всю работу
            }
            const API_Element &beam = beamRead.element;
            const API_BeamPart *selectedPart = &part;
            const double axisLength =
                std::hypot (beam.beam.endC.x - beam.beam.begC.x, beam.beam.endC.y - beam.beam.begC.y);
            const auto shortest = shortestBeamParts.GetPtr (part.guid);
            if (room.surfaceSnapshot.IsComplete () && shortest != nullptr && std::isfinite (axisLength) &&
                axisLength > DistanceTolerance &&
                ((*shortest)->tEnd - (*shortest)->tBeg) / axisLength <= DistanceTolerance) {
                // Полная зона уже включает все связные контакты; остальные relations этой балки не дублируем.
                selectedPart = *shortest;
                wholeZoneBeams.Add (part.guid, true);
            }
            // Определяем наивысшую точку балки
            double beamTop = -1e9;
            for (const auto &body : source->snapshot.bodies) {
                for (const auto &face : body.surfaces) {
                    for (const auto &c : face.contours) {
                        for (const auto &v : c.vertices) {
                            beamTop = std::max (beamTop, v.world.z);
                        }
                    }
                }
            }
            const double ceilingLevel = room.zBottom + room.height_ceil;
            const bool beamBelowCeiling = beamTop < ceilingLevel - 1e-6;

            RoomSurfaceBeamPartitions result;
            const auto finish = RoomBeamFinish_Build (*source,
                                                      beam.beam.begC,
                                                      beam.beam.endC,
                                                      selectedPart->tBeg,
                                                      selectedPart->tEnd,
                                                      room.zBottom,
                                                      room.zBottom + room.height,
                                                      result,
                                                      &room.surfaceSnapshot);
            if (!result.errors.IsEmpty ()) {
                msg_rep ("Surface finish",
                         GS::UniString ("Beam finish rejected: ") + result.errors[0].stage,
                         result.errors[0].code,
                         part.guid);
                continue; // Из-за одной ошибки нельзя прерывать всю работу
            }
            for (const auto &beamPiece : finish.pieces) {
                // Если балка в потолке (beamTop >= ceilingLevel) — верх не отделываем
                if (!beamBelowCeiling && beamPiece.face == RoomSurfaceBeamFace::Top)
                    continue;

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
                        if (!SurfaceWall_Build (original, room.floorInd, testWall) &&
                            !SurfaceSlabs_Build (original, room.floorInd, testSlabs))
                            continue;
                        piece.normal = {-face.normalWorld.x, -face.normalWorld.y, -face.normalWorld.z};
                        piece.planeOffset = -face.planeOffset;
                        found = true;
                        break;
                    }
                    if (found)
                        break;
                }
                const TypeOtd forceType = beamBelowCeiling ? Wall_Main : Ceil;
                if (!found || !OtdRoom_AddSurfaceFinishPiece (room, piece, cache, forceType)) {
                    hasSkippedPieces = true;
                }
            }
        }
    }

    // Подготавливает геометрию для прежних материальных правил и writers; элементы здесь не создаются.
    static bool OtdRoom_BuildSurfaceFinishes (OtdRoom &room,
                                              const UnicGuid &finclassguids,
                                              RoomSurfaceCache *cache,
                                              MixedFloorProvenance &floorProvenance) {
        GS::Array<OtdWall> ordinaryWalls;
        OtdRoom_CollectOrdinaryWalls (room, cache, ordinaryWalls);
        room.otdwall.Clear ();
        room.otdslab.Clear ();
        bool hasSkippedPieces = false;
        bool hasAmbiguousPieces = false;
        if (!OtdRoom_BuildHorizontalFinishes (room, floorProvenance))
            return false;
        if (!OtdRoom_BuildPartitionWallFinishes (room, cache, hasSkippedPieces, hasAmbiguousPieces))
            return false;
        room.otdwall.Append (ordinaryWalls);
        OtdRoom_BuildBeamFinishes (room, finclassguids, cache, hasSkippedPieces);
        if (hasAmbiguousPieces)
            msg_rep ("Surface finish", "Ambiguous surface contacts were not assigned", NoError, room.zone_guid);
        if (hasSkippedPieces)
            msg_rep ("Surface finish", "Unsupported finish pieces were skipped", NoError, room.zone_guid);
        return true;
    }

    // 3D читается после флагов комнаты; частичный набор снимков не публикуется.
    static bool OtdRoom_PrepareMeshFloorSources (OtdRoom &room, RoomSurfaceCache *cache) {
        if (!room.has_floor || room.meshFloorSources.IsEmpty ())
            return true;
        auto sources = room.meshFloorSources;
        for (auto &mesh : sources) {
            const auto &source = mesh.surface;
            if (source.baseType != API_MeshID || source.baseGuid == APINULLGuid ||
                source.geometryGuid != source.baseGuid || source.origin != RoomSurfaceOrigin::Native3D)
                return false;
            const auto read = Element_Read (source.baseGuid, cache);
            if (read.error != NoError || read.element.header.guid != source.baseGuid ||
                GetElemTypeID (read.element.header) != API_MeshID || read.element.mesh.skirt < 1 ||
                read.element.mesh.skirt > 3 || mesh.hasBottom != (read.element.mesh.skirt == 1))
                return false;
            mesh.surface.snapshot = RoomSurfaces_Read (read.element.header, cache);
            if (!mesh.surface.snapshot.IsComplete () || mesh.surface.snapshot.guid != source.geometryGuid)
                return false;
        }
        room.meshFloorSources = std::move (sources);
        return true;
    }

    // Для комнат с прочитанными параметрами рассчитывает полы, потолки, стены и откосы, пропуская недопустимые данные.
    // Использует отдельную копию параметров каждой комнаты; при отмене возвращает false.
    bool OtdRooms_ProcessFinishes (RoomProcessingContext &context,
                                   ReadParamsForRoomBook &readParams,
                                   ParamDictCompositeElement &paramCompositeToRead,

                                   bool has_base_element,
                                   GS::UniString &funcname,
                                   GS::Int32 &nPhase) {
        auto &roomParams = readParams.roomParams;
        // Подготавливаются Otd-данные каждой комнаты; native-элементы создаются позже в OtdRooms_Draw.
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
            otd.otdbeam_floor.Clear ();
            otd.otdbeam_ceil.Clear ();
            if (AdvanceProcessPhase (funcname, nPhase))
                return false;
            if (!context.paramToRead.ContainsKey (zoneGuid))
                continue; // Если у зоны нет прочитанных параметров - дальше делать
                          // нечего
            // Параметры комнаты: высоты, материалы и флаги создания.
            // Копия на каждую зону: Param_Property_Read пропускает параметры с уже
            // выставленным isValid и не перечитывает val — общая копия тянула бы
            // значения первой зоны во все остальные.
            ReadParams roomParamsWork = roomParams;
            Param_SetToRooms (material_dict, otd, context.paramToRead, roomParamsWork);
            if (!otd.isValid)
                continue;
            if (!OtdRoom_PrepareMeshFloorSources (otd, &context.surfaceCache)) {
                msg_rep ("Mesh floor", "Incomplete source snapshot; stopped before writing", APIERR_BADPARS, zoneGuid);
                return false;
            }
            // Сохраняем исходную привязку проёмов до замены стен поверхностными.
            GS::Array<OtdWall> openingWalls;
            for (const auto &wall : otd.otdwall) {
                openingWalls.Push (wall); // Все участки нужны также для стороны полного состава.
            }
            MixedFloorProvenance floorProvenance;
            if (!OtdRoom_BuildSurfaceFinishes (otd, context.finclassguids, &context.surfaceCache, floorProvenance)) {
                if (otd.has_floor && !otd.meshFloorSources.IsEmpty ()) {
                    msg_rep (
                        "Mesh floor", "Mixed geometry refused; existing finishes retained", APIERR_BADPARS, zoneGuid);
                    return false;
                }
                continue;
            }
            if (!MeshFloor_ValidateSourceIdentity (otd, floorProvenance) ||
                !MeshFloor_ValidateFinalDomains (otd, floorProvenance)) {
                msg_rep (
                    "Mesh floor", "Invalid source provenance; existing finishes retained", APIERR_BADPARS, zoneGuid);
                return false;
            }
            if (otd.otdwall.IsEmpty () && otd.otdslab.IsEmpty () && openingWalls.IsEmpty ())
                continue;
            // Полы и потолки: материалы и избранное для подготовленных контуров.
            OtdSlab_ProcessFinishes (otd,
                                     zoneGuid,
                                     context.paramToRead,
                                     context.param_composite,
                                     context.paramDict_favorite,
                                     context.exsistot_byzone,
                                     paramCompositeToRead,
                                     has_base_element,
                                     context.favdict);
            // Стены и откосы: здесь же пол расширяется в проёмы, независимо от флага создания откосов.
            // Откосы нужны и при отсутствии представимых основных surface-стен.
            Geometry::MultiPolygon2D profileExclusion;
            if (!ProfileWalls_ReadFloorExclusion (otd, context, profileExclusion)) {
                msg_rep ("Profile floor",
                         "Unsupported or incomplete wall section; existing finishes retained",
                         APIERR_BADPARS,
                         zoneGuid);
                return false;
            }
            const bool hasWalls = !otd.otdwall.IsEmpty () || !openingWalls.IsEmpty ();
            if (!hasWalls && profileExclusion.IsEmpty ())
                continue;
            if (hasWalls && !OtdWall_ProcessFinishes (otd,
                                                      openingWalls,
                                                      context.paramToRead,
                                                      context.param_composite,
                                                      context.paramDict_favorite,
                                                      readParams.windowParams,
                                                      paramCompositeToRead,
                                                      floorProvenance,
                                                      context.favdict)) {
                msg_rep (
                    "Mesh floor", "Uncovered opening demand; existing finishes retained", APIERR_BADPARS, zoneGuid);
                return false;
            }
            // Окончательные контуры: расширение пола до ядра, устранение перекрытия контактов, нижние отрезки
            // плинтусов.
            if (!FloorContours_ExtendToCore (otd))
                msg_rep ("Floor core extension",
                         "Contour topology changed; original floor retained",
                         APIERR_BADPARS,
                         zoneGuid);
            if (!MeshFloor_ValidateFinalDomains (otd, floorProvenance)) {
                msg_rep ("Mesh floor", "Final domains changed; existing finishes retained", APIERR_BADPARS, zoneGuid);
                return false;
            }
            // После расширений и проверки provenance: дробление пола больше не меняет её используемые индексы.
            GS::Array<OtdDrawData> profileFloorLinks;
            if (!profileExclusion.IsEmpty ())
                for (const auto &floor : otd.otdslab)
                    if (floor.draw.type == Floor && floor.draw.otd_guid != APINULLGuid)
                        profileFloorLinks.Push (floor.draw);
            if (!FloorContours_SubtractProfileWalls (otd, profileExclusion))
                return false;
            // Полностью вырезанный пол возвращаем в список удаления: ProcessFinishes уже изъял его GUID.
            for (const auto &link : profileFloorLinks) {
                bool retained = false;
                for (const auto &floor : otd.otdslab)
                    retained = retained || floor.draw.otd_guid == link.otd_guid;
                if (retained)
                    continue;
                auto *byBase = context.exsistot_byzone.GetPtr (zoneGuid);
                auto *byType = byBase != nullptr ? byBase->GetPtr (link.base_guid) : nullptr;
                auto *unused = byType != nullptr ? byType->GetPtr (link.type) : nullptr;
                if (unused == nullptr && byType != nullptr)
                    unused = byType->GetPtr (NoSet);
                if (unused == nullptr)
                    return false;
                unused->Add (link.otd_guid, true);
            }
            if (!SlabContacts_DeduplicateAccepted (otd))
                msg_rep ("Surface finish",
                         "Accepted slab side remainder is ambiguous or unrepresentable",
                         APIERR_BADPARS,
                         zoneGuid);
            if (!OtdBeam_ProcessFinishes (otd, context.paramDict_favorite, context.param_composite, context.favdict)) {
                msg_rep ("Skirting", "Cannot prepare final floor/ceiling beam arrays", APIERR_BADPARS, zoneGuid);
                return false;
            }
        } // Обработка зон
        return true;
    }

    // Данные зон и конструкций, параметры, материалы и избранное.
    // Floor/Ceil finishes also use native walls for slab steps and nonhorizontal contacts.
    bool Otd_IsNativeTypeForFinish (TypeOtd type_otd, API_ElemTypeID type_subguid) {
        bool correct_type = true;
        switch (type_otd) {
        case Floor_Skirting:
        case Ceil_Cornice:
            correct_type = type_subguid == API_BeamID || type_subguid == API_WallID || type_subguid == API_ObjectID;
            break;
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
            correct_type = (type_subguid == API_SlabID || type_subguid == API_ObjectID || type_subguid == API_WallID ||
                            type_subguid == API_RoofID);
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
            return exsistot_byzone;
        }
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
            for (UnicGuid::ConstPairIterator cItt = guids.EnumeratePairs (); cItt != NULL; ++cItt) {
    #ifdef ServerMainVers_2800
                API_Guid guid = cItt->key;
                bool isvisible = cItt->value;
    #else
                API_Guid guid = *cItt->key;
                bool isvisible = *cItt->value;
    #endif
                API_Guid classguid;
                if (Class_IsElementFinClass (guid, finclassguids, classguid, cache)) {
                    API_ElemTypeID type_subguid = GetElemTypeID (guid);
                    TypeOtd type_otd = Class_GetOtdTypeByClass (classguid, finclass, type_subguid);
                    const bool correct_type = Otd_IsNativeTypeForFinish (type_otd, type_subguid);
                    if (!otd_elements.ContainsKey (guid) && correct_type)
                        otd_elements.Add (guid, type_otd);
                    if (!reserv_elements.ContainsKey (guid)) {
                        if (!ACAPI_Element_Filter (guid, APIFilt_InMyWorkspace)) {
                            reserv_elements.Add (guid, true);
                        }
                    }
                }
            }
            if (exsistot_byzone.ContainsKey (zoneguid))
                continue;
            UnicGuidByBase exsistot_byparent = Otd_GetOtd_Parent (otd_elements, has_base_element);
            exsistot_byzone.Add (zoneguid, exsistot_byparent);
        }
        return exsistot_byzone;
    }

    // Группирует известную отделку по GUID базового элемента и типу, пропуская неизвестные дочерние GUID.
    // При добавлении элемента устанавливает has_base_element, не сбрасывая прежнее значение.
    UnicGuidByBase Otd_BuildByParent (GS::HashTable<API_Guid, TypeOtd> &otd_elements,
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
    static UnicGuidByBase Otd_GetOtd_Parent (GS::HashTable<API_Guid, TypeOtd> &otd_elements, bool &has_base_element) {
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
        return Otd_BuildByParent (otd_elements, parentdict, has_base_element);
    }

    // Читает геометрию и связи зоны, заполняет данные комнаты и индекс связанных конструкций и проёмов.
    // Дополнительно ищет стены по серединам рёбер; возвращает признак найденных связей или стен, при ошибках чтения —
    // false.
    static bool OtdRoom_CollectInfo (const Stories &storyLevels,
                                     const API_Guid &zoneGuid,
                                     OtdRoom &roominfo,
                                     UnicElementByType &elementToRead,
                                     GS::HashTable<API_Guid, GS::Array<API_Guid>> &slabsinzone,
                                     RoomSurfaceCache *cache,
                                     CoordinateSearchCache *coordinateCache,
                                     const MeshFloorIndex *meshesinzone) {
        roominfo.surfaceSnapshot = RoomSurfaceSnapshot ();
        API_Element zoneelement = {};
        zoneelement.header.guid = zoneGuid;

        GSErrCode err = ACAPI_Element_Get (&zoneelement);
        if (err != NoError) {
            msg_rep ("OtdRoom_CollectInfo err", "ACAPI_Element_Get zone", err, zoneGuid);
            return false;
        }

        roominfo.height = zoneelement.zone.roomHeight;
        roominfo.height = std::round (roominfo.height * 1000) / 1000;
        if (roominfo.height < 0.0001) {
            msg_rep ("OtdRoom_CollectInfo err", "room.height < 0.0001", err, zoneGuid);
            roominfo.isValid = false;
            return false;
        }

        API_ElementMemo zonememo = {};
        const GS::OnExit zoneMemoGuard ([&zonememo] () { ACAPI_DisposeElemMemoHdls (&zonememo); });
        err = ACAPI_Element_GetMemo (zoneelement.header.guid, &zonememo);
        if (err != NoError) {
            msg_rep ("OtdRoom_CollectInfo err", "ACAPI_Element_GetMemo zone", err, zoneGuid);
            return false;
        }
        GS::Array<Sector> walledges;   // Границы стен, не явяющихся границей зоны
        GS::Array<Sector> columnedges; // Границы колонн, не явяющихся границей зоны
        GS::Array<Sector> restedges;   // Границы зоны
        GS::Array<Sector> gableedges;
        OtdRoom_GetEdges (zonememo, zoneelement, walledges, columnedges, restedges, gableedges);
        roominfo.edges = restedges;
        roominfo.columnedges = columnedges;
        RoomRelationValues relData;
        err = Zone_ReadRelations (zoneGuid, cache, relData);
        if (err != NoError) {
            msg_rep ("OtdRoom_CollectInfo err", "ACAPI_Element_GetRelations zone", err, zoneGuid);
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
        if (meshesinzone != nullptr) {
            if (const auto *sources = meshesinzone->GetPtr (zoneGuid)) {
                roominfo.meshFloorSources = *sources;
                flag = flag || !sources->IsEmpty ();
            }
        }
        OtdSlab otdslab;
        ConstructPolygon2DFromElementMemo (zonememo, otdslab.poly);
    #ifdef ServerMainVers_2700
        otdslab.draw.material.material = zoneelement.zone.material.ToInt32_Deprecated ();
    #else
        otdslab.draw.material.material = zoneelement.zone.material;
    #endif
        otdslab.draw.base_type = API_ZoneID;
        otdslab.draw.base_guid = zoneGuid;
        otdslab.draw.draw_type = API_SlabID;
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
                err = CoordinateSearch_Search (searchPars, elGuid, coordinateCache);
                if (err == NoError && elGuid != APINULLGuid) {
                    flag = true;
                    for (const auto connectedType : {API_WindowID, API_DoorID}) {
                        const auto connected =
                            ConnectedElements_Read (elGuid,
                                                    connectedType,
                                                    APIFilt_IsEditable | APIFilt_HasAccessRight | APIFilt_InMyWorkspace,
                                                    cache);
                        if (connected.error != NoError) {
                            msg_rep ("OtdRoom_CollectInfo err",
                                     "ACAPI_Element_GetConnectedElements",
                                     connected.error,
                                     elGuid);
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
                    msg_rep ("OtdRoom_CollectInfo err",
                             "ACAPI_Element_SearchElementByCoord element not found",
                             err,
                             zoneGuid);
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
        roominfo.surfaceSnapshot = RoomSurfaces_Read (zoneelement.header, cache);
        return flag;
    }

    // -----------------------------------------------------------------------------
    // Чтение данных о проёме
    // -----------------------------------------------------------------------------
    static void Opening_Create_One (const Stories &storyLevels,
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
    static void OtdWall_Create_FromWall (const Stories &storyLevels,
                                         const API_Guid &elGuid,
                                         const GS::Array<API_Guid> &zoneGuids,
                                         OtdRooms &roomsinfo,
                                         const GS::HashTable<API_Guid, GS::Array<OtdOpening>> &openinginwall,
                                         UnicGUIDByType &guidselementToRead,
                                         RoomSurfaceCache *cache) {
        const ElementRead elementRead = Element_Read (elGuid, cache);
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
            return;
        }
        GS::Optional<UnitVector_2D> walldir = walledge.GetDirection ();
        if (!walldir.HasValue ()) {
            return;
        }
        const GS::Optional<Geometry::Line2D> wallline = walledge.AsLine ();
        for (const API_Guid &zoneGuid : zoneGuids) {
            OtdRoom *roominfoPtr = roomsinfo.GetPtr (zoneGuid);
            if (roominfoPtr == nullptr || !roominfoPtr->isValid) {
                continue;
            }

            bool flag_find = false;
            if (const auto *sectorsPtr = roominfoPtr->walledges.GetPtr (elGuid)) {
                if (!wallline.HasValue ()) {
                    continue;
                }
                for (const auto &roomedge : *sectorsPtr) {
                    GS::Optional<Geometry::Line2D> roomeline = roomedge.AsLine ();
                    if (!roomeline.HasValue ()) {
                        continue;
                    }
                    GS::Optional<UnitVector_2D> roomedgedir = roomedge.GetDirection ();
                    if (!roomedgedir.HasValue ()) {
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
                                OtdOpening_Add (op, is_fliped, zBottom, tBeg, tEnd, wallLength, wallotd);
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
                        continue;
                    }
                    const Sector &roomedge = roominfoPtr->edges.Get (inxedge);
                    GS::Optional<Geometry::Line2D> roomeline = roomedge.AsLine ();
                    if (!roomeline.HasValue ()) {
                        continue;
                    }
                    GS::Optional<UnitVector_2D> roomedgedir = roomedge.GetDirection ();
                    if (!roomedgedir.HasValue ()) {
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
                                OtdOpening_Add (op, is_fliped, zBottom, wpart.tBeg, wpart.tEnd, wallLength, wallotd);
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
    static bool Opening_CalculateForWall (const OtdOpening &op,
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
    void OtdOpening_Add (const OtdOpening &op,
                         bool is_fliped,
                         double zBottom,
                         double tBeg,
                         double tEnd,
                         double wallLength,
                         OtdWall &wallotd) {
        OtdOpening opinwall;
        if (Opening_CalculateForWall (op, is_fliped, zBottom, tBeg, tEnd, wallLength, opinwall))
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
        wallotd.draw.base_guid = wallguid;
        wallotd.geom.height = height;
        wallotd.base_th = thickness;
        wallotd.base_flipped = is_fliped;
        wallotd.geom.floorInd = floorInd;
        wallotd.geom.seg.begC = {walledge.c1.x, walledge.c1.y, zBottom};
        wallotd.geom.seg.endC = {walledge.c2.x, walledge.c2.y, zBottom};
        wallotd.draw.base_type = API_WallID;
        wallotd.draw.draw_type = API_WallID;
        wallotd.draw.type = Wall_Main;
        return true;
    }

    // Индекс Mesh обновляется целиком: ошибка чтения не оставляет частичный набор оснований.
    bool Mesh_FindAll (MeshFloorIndex &meshesinzone,
                       const UnicGuid &finclassguids,
                       const GS::HashTable<API_Guid, Geometry::Polygon2D> &zonePolygons,
                       RoomSurfaceCache *cache) {
        MeshFloorIndex indexed;
        if (zonePolygons.IsEmpty ()) {
            meshesinzone.Clear ();
            return true;
        }
        GS::Array<API_Guid> meshes;
        const GSErrCode listError =
            ACAPI_Element_GetElemList (API_MeshID, &meshes, APIFilt_OnVisLayer | APIFilt_IsVisibleByRenovation);
        if (listError != NoError) {
            msg_rep ("Mesh discovery", "ACAPI_Element_GetElemList failed before writing", listError, APINULLGuid);
            return false;
        }
        for (const auto &guid : meshes) {
            API_Guid classGuid = APINULLGuid;
            if (Class_IsElementFinClass (guid, finclassguids, classGuid, cache))
                continue;
            API_ElementMemo memo = {};
            const GS::OnExit memoGuard ([&memo] () { ACAPI_DisposeElemMemoHdls (&memo); });
            Geometry::Polygon2D polygon;
            const GSErrCode memoError = ACAPI_Element_GetMemo (guid, &memo, APIMemoMask_Polygon);
            const GSErrCode polygonError =
                memoError == NoError ? ConstructPolygon2DFromElementMemo (memo, polygon) : memoError;
            if (polygonError != NoError) {
                msg_rep ("Mesh discovery", "Incomplete mesh polygon; stopped before writing", polygonError, guid);
                return false;
            }
            GS::Array<API_Guid> overlappingRooms;
            for (auto it = zonePolygons.EnumeratePairs (); it != nullptr; ++it) {
    #ifdef ServerMainVers_2800
                const API_Guid &zoneGuid = it->key;
                const auto &zonePolygon = it->value;
    #else
                const API_Guid &zoneGuid = *it->key;
                const auto &zonePolygon = *it->value;
    #endif
                const auto overlap = polygon.Intersect (zonePolygon);
                double area = 0;
                for (const auto &part : overlap)
                    area += std::abs (part.CalcArea ());
                if (!std::isfinite (area)) {
                    msg_rep ("Mesh discovery", "Non-finite overlap; stopped before writing", APIERR_BADPARS, guid);
                    return false;
                }
                if (area > 1e-12)
                    overlappingRooms.Push (zoneGuid);
            }
            if (overlappingRooms.IsEmpty ())
                continue;
            const auto read = Element_Read (guid, cache);
            if (read.error != NoError || GetElemTypeID (read.element.header) != API_MeshID ||
                read.element.mesh.skirt < 1 || read.element.mesh.skirt > 3) {
                msg_rep ("Mesh discovery",
                         "Mesh settings unavailable; stopped before writing",
                         read.error != NoError ? read.error : APIERR_BADPARS,
                         guid);
                return false;
            }
            MeshFloorSource source;
            source.surface.baseGuid = source.surface.geometryGuid = guid;
            source.surface.baseType = API_MeshID;
            source.hasBottom = read.element.mesh.skirt == 1;
            for (const auto &zoneGuid : overlappingRooms) {
                if (!indexed.ContainsKey (zoneGuid))
                    indexed.Add (zoneGuid, GS::Array<MeshFloorSource> ());
                indexed[zoneGuid].Push (source);
            }
        }
        meshesinzone = std::move (indexed);
        return true;
    }

    // -----------------------------------------------------------------------------
    // Находит все перекрытия, котрые пересекают зону и не являются элементами
    // отделки
    // -----------------------------------------------------------------------------
    static void OtdSlab_FindAll (GS::HashTable<API_Guid, GS::Array<API_Guid>> &slabsinzone,
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
    static void OtdSlab_FindInRoom (const Stories &storyLevels,
                                    API_Guid &elGuid,
                                    const GS::Array<API_Guid> &zoneGuids,
                                    OtdRooms &roomsinfo,
                                    UnicGUIDByType &guidselementToRead,
                                    RoomSurfaceCache *cache) {
        const ElementRead elementRead = Element_Read (elGuid, cache);
        if (elementRead.error != NoError || !elementRead.element.header.hasMemo) {
            msg_rep ("OtdSlab_FindInRoom err", "ACAPI_Element_Get", elementRead.error, elGuid);
            return;
        }
        const API_Element &element = elementRead.element;
        GS::Array<OtdWall> walls;
        GS::Array<OtdSlab> bases;
        const auto preparedSource = RoomSurfaceSource_Prepare (element, storyLevels, {}, walls, bases, cache);
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
            for (const auto &source : roominfoPtr->slabSources) {
                if (source.baseGuid == elGuid) {
                    prepared = true;
                    break;
                }
            }
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
    static void Param_ToParamDict (ParamDictValue &paramDict, ReadParams &zoneparams) {
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
    static void Param_GetForBase (ParamDictValue &paramDict, ParamValue &param_composite) {

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
        }
        param_composite.val.uniStringValue.Append ("]h[");
        if (!rawName_hasfin.IsEmpty ()) {
            param_composite.val.uniStringValue.Append (rawName_hasfin);
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
        }
        if (!param_onoff_elem.rawName.IsEmpty ()) {
            ParamValue param;
            param.rawName = "{@flag:rawname_element_onoff}";
            param.val.uniStringValue = param_onoff_elem.rawName;
            ParamHelpers::AddParamValue2ParamDict (APINULLGuid, param, paramDict);
            ParamHelpers::AddParamValue2ParamDict (APINULLGuid, param_onoff_elem, paramDict);
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

        zoneparam_name = "tip_plint_pot";
        zoneparam.rawnames.Push ("some_stuff_fin_type_ceil_cornice");
        zoneparam.rawnames.Push ("{@gdl:tip_plint_pot}");
        zoneparams.Add (zoneparam_name, zoneparam);
        zoneparam.rawnames.Clear ();

        // Тип пола
        zoneparam_name = "tip_pol";
        zoneparam.rawnames.Push ("some_stuff_fin_type_floor");
        zoneparam.rawnames.Push ("{@gdl:tip_pol}");
        zoneparams.Add (zoneparam_name, zoneparam);
        zoneparam.rawnames.Clear ();

        zoneparam_name = "tip_plint_pol";
        zoneparam.rawnames.Push ("some_stuff_fin_type_floor_skirting");
        zoneparam.rawnames.Push ("{@gdl:tip_plint_pol}");
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

        // Отделка длины плинтуса потолка - запись результатов
        zoneparam_name = "om_ceil_cornice.rawname";
        zoneparam.rawnames.Push ("some_stuff_fin_ceil_cornice_result");
        zoneparams.Add (zoneparam_name, zoneparam);
        zoneparam.rawnames.Clear ();

        zoneparam_name = "om_ceil_cornice.rawname_bytype";
        zoneparam.rawnames.Push ("some_stuff_fin_ceil_cornice_result_bytype");
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
    static void Param_ResolveRawNames (ReadParams &zoneparams, ParamDictValue &propertyParams) {
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
        Param_ResolveRawNames (zoneparams, propertyParams);
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
                const auto *sourceValue = baseparam->GetPtr (param_name);
                if (sourceValue == nullptr || !sourceValue->isValid)
                    continue;
                if (param_name.Contains ("{@gdl:vots}") || param_name.Contains ("{@gdl:vots_fill}")) {
                    if (!sourceValue->val.boolValue)
                        continue;
                }
                flag = true;
                param.isValid = true;
                param.val = sourceValue->val;
                if (sourceValue->fromProperty) {
                    param.val.type = API_PropertyStringValueType;
                }
                break;
            }
        }
        return flag;
    }

    // Для строкового значения получает индекс покрытия по имени, используя и пополняя material_dict.
    static void Param_Material_Get (GS::HashTable<GS::UniString, GS::Int32> &material_dict, ParamValueData &val) {
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
    static void OtdRoom_NormalizeFinishHeights (OtdRoom &roominfo) {
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
    static void OtdRoom_ApplyCreationFlags (const ReadParams &readparams, OtdRoom &roominfo) {
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
    static void OtdRoom_ApplyMaterialRawNames (const ReadParams &readparams, OtdRoom &roominfo) {
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
        param_name = "om_floor_skirting.rawname_bytype";
        if (const auto *p = readparams.GetPtr (param_name)) {
            if (p->isValid) {
                val = p->val;
                roominfo.om_floor_skirting.rawname_bytype = val.uniStringValue;
            }
        }
        param_name = "om_ceil_cornice.rawname_bytype";
        if (const auto *p = readparams.GetPtr (param_name)) {
            if (p->isValid) {
                val = p->val;
                roominfo.om_ceil_cornice.rawname_bytype = val.uniStringValue;
            }
        }
    }

    // Переносит высоты нижней и основной отделки; при отсутствии height_down использует параметры химеры.
    static void OtdRoom_ApplyHeightParameters (const ReadParams &readparams, OtdRoom &roominfo) {
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
    static void OtdRoom_ApplyFinishTypeParameters (const ReadParams &readparams, OtdRoom &roominfo) {
        GS::UniString param_name;
        ParamValueData val;

        param_name = "tip_pot";
        if (const auto *p = readparams.GetPtr (param_name)) {
            if (p->isValid) {
                val = p->val;
                roominfo.tip_ceil = val.uniStringValue;
            }
        }

        param_name = "tip_plint_pot";
        if (const auto *p = readparams.GetPtr (param_name)) {
            if (p->isValid) {
                val = p->val;
                roominfo.tip_ceil_cornice = val.uniStringValue;
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
                roominfo.tip_floor = val.uniStringValue;
            }
        }
        param_name = "tip_plint_pol";
        if (const auto *p = readparams.GetPtr (param_name)) {
            if (p->isValid) {
                val = p->val;
                roominfo.tip_floor_skirting = val.uniStringValue;
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
    static void OtdRoom_ApplyMaterialParameters (GS::HashTable<GS::UniString, GS::Int32> &material_dict,
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
        OtdRoom_ApplyFinishTypeParameters (readparams, roominfo);

        OtdRoom_ApplyMaterialParameters (material_dict, readparams, roominfo);
        OtdRoom_ApplyHeightParameters (readparams, roominfo);
        OtdRoom_ApplyMaterialRawNames (readparams, roominfo);
        OtdRoom_ApplyCreationFlags (readparams, roominfo);
        OtdRoom_NormalizeFinishHeights (roominfo);
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
            return true;
        }

        const ParamComposite *base_composite_ptr = compositeMapPtr->GetPtr (param_composite.rawName);
        if (base_composite_ptr == nullptr) {
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
            base_composite_ptr->composite_type == API_CompositeStructure && floorWall->draw.base_type == API_WallID &&
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
    // Добавляет слои от выбранной стороны до первого слоя с точным типом APICWallComp_Core включительно.
    // Уже накопленные слои не очищает; если ядра нет, добавляет весь состав.
    // -----------------------------------------------------------------------------
    static void ParamComposite_CollectLayersToCore (const ParamComposite &base_composite,
                                                    bool base_flipped,
                                                    Int32 ncomp,
                                                    GS::Array<ParamValueComposite> &cpmpoosite) {
        if (base_flipped) {
            for (Int32 j = ncomp - 1; j >= 0; j--) {
                cpmpoosite.Push (base_composite.composite[j]);
                if (base_composite.composite[j].structype == APICWallComp_Core)
                    break;
            }
        } else {
            for (Int32 j = 0; j < ncomp; j++) {
                cpmpoosite.Push (base_composite.composite[j]);
                if (base_composite.composite[j].structype == APICWallComp_Core)
                    break;
            }
        }
    }

    // -----------------------------------------------------------------------------
    // Разбирает метки l[1:], h и f в слоях выбранной стороны и добавляет результат к переданному составу.
    // Собирает имя избранного; для последнего слоя с structype == -1 читает покрытие строительного материала.
    // -----------------------------------------------------------------------------
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
        ParamComposite_CollectLayersToCore (base_composite, base_flipped, ncomp, cpmpoosite);
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
            API_Attribute attrib = {};
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
    // После проверки словаря задаёт глубину по толщине стены, затем уточняет её по frame, sill и стороне откоса.
    // При отсутствии словаря сохраняет глубину откоса; при отказе чтения или невалидных frame / sill оставляет
    // толщину стены. Добавка пола по коробке сбрасывается перед чтением.
    // -----------------------------------------------------------------------------
    void Param_SetToWindows (OtdOpening &op,
                             ParamDictElement &paramToRead,
                             ReadParams &readparams,
                             const OtdWall &otdw) {
        op.base_frame_width = 0;
        API_Guid base_guid = op.base_guid;
        if (!paramToRead.ContainsKey (base_guid)) {
            return;
        }
        op.base_reveal_width = otdw.base_th;
        if (!Param_Property_Read (base_guid, paramToRead, readparams)) {
            return;
        }

        GS::UniString param_name;
        double frame = 0;
        double sill = 0;

        param_name = "frame";
        const auto *frameParam = readparams.GetPtr (param_name);
        if (frameParam == nullptr || !frameParam->isValid)
            return;
        frame = frameParam->val.doubleValue;

        param_name = "sill";
        const auto *sillParam = readparams.GetPtr (param_name);
        if (sillParam == nullptr || !sillParam->isValid)
            return;
        sill = sillParam->val.doubleValue;

        // Коробка разделяет откосы двух помещений: глубины зависят от стороны отделки.
        op.base_reveal_width = op.reveal_from_sill ? sill : otdw.base_th - sill - frame;
        // Полы встречаются в середине коробки, а сами откосы заканчиваются на её гранях.
        op.base_frame_width = std::isfinite (frame) && frame > 0 ? frame : 0;

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
    // Добавляет рёбра редукции в reducededges по типам rest, wall, column и gable; затем освобождает handles.
    // При отсутствии reducededges или некорректных массивах выходит до освобождения handles.
    // -----------------------------------------------------------------------------
    #ifdef ServerMainVers_2700
    void RoomReductionPolyProc (const API_RoomReductionPolyType *roomRed)
    #else
    static void __ACENV_CALL RoomRedProc (const API_RoomReductionPolyType *roomRed)
    #endif
    {
        if (reducededges == nullptr) {
            return;
        }
        if (roomRed->nCoords < 4 || roomRed->coords == nullptr || roomRed->subPolys == nullptr) {
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
    // Получает рёбра редукций зоны и оставляет рёбра стен, колонн и скатов, совпавшие с restedges редукции.
    // В restedges возвращает исходные рёбра полигона с нулевым разделителем после каждого контура.
    // -----------------------------------------------------------------------------
    static void OtdRoom_GetEdges (const API_ElementMemo &zonememo,
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
    void Zone_ClearDuplicateGUIDs (UnicElementByType &elementToRead) {
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
    // Заполняет геометрию стенки откоса и наследует основание, этаж, материал и состав от отделочной стены.
    // Устанавливает тип основания API_WindowID, тип отделки Reveal_Main и заданный тип рисования.
    // -----------------------------------------------------------------------------
    static void OtdWall_InitializeOpeningReveal (const OtdWall &otdw,
                                                 OtdWall &wallotd,
                                                 double height,
                                                 double zBottom,
                                                 double width,
                                                 double length,
                                                 const Point2D &begC,
                                                 const Point2D &endC,
                                                 API_ElemTypeID draw_type) {
        wallotd.draw.base_guid = otdw.draw.base_guid;
        wallotd.geom.height = height;
        wallotd.geom.width = width;
        wallotd.geom.length = length;
        wallotd.geom.floorInd = otdw.geom.floorInd;
        wallotd.geom.seg.begC = {begC.x, begC.y, zBottom};
        wallotd.geom.seg.endC = {endC.x, endC.y, zBottom};
        wallotd.draw.material = otdw.draw.material;
        wallotd.base_composite = otdw.base_composite;
        wallotd.draw.base_type = API_WindowID;
        wallotd.draw.draw_type = draw_type;
        wallotd.draw.type = Reveal_Main;
    }

    // Вычисляет вертикальное пересечение проёма со стеной и признак выхода проёма выше стены.
    // Возвращает false, если высота пересечения меньше min_dim.
    static bool OtdOpening_CalculateRevealHeight (
        const OtdWall &otdw, const OtdOpening &op, bool &is_upper_wall, double &zDown, double &height) {
        is_upper_wall = false; // Проём заканчивается выше стены, построение
                               // верхнего откоса не требуется
        const double zBottom = OtdLinearSegment_GetZBottom (otdw.geom);
        if ((op.zBottom + op.height) > (zBottom + otdw.geom.height))
            is_upper_wall = true;
        double zDup = fmin (op.zBottom + op.height, zBottom + otdw.geom.height);
        zDown = fmax (op.zBottom, zBottom);
        height = fmin (op.height, zDup - zDown);
        if (height < min_dim)
            return false;
        return true;
    }

    // Вычисляет концы боковых откосов по положению и ширине проёма и заданному перпендикуляру стены.
    // При нулевой длине стены возвращает false.
    static bool OtdOpening_CalculateRevealCoordinates (const OtdWall &otdw,
                                                       const OtdOpening &op,
                                                       const Geometry::Vector2<double> &walldir_perp,
                                                       Point2D &left_begin,
                                                       Point2D &left_end,
                                                       Point2D &right_begin,
                                                       Point2D &right_end) {
        double dx = -otdw.geom.seg.endC.x + otdw.geom.seg.begC.x;
        double dy = -otdw.geom.seg.endC.y + otdw.geom.seg.begC.y;
        double dr = sqrt (dx * dx + dy * dy);
        if (is_equal (dr, 0))
            return false;
        double lambda = 0;
        lambda = (op.objLoc - op.width / 2) / dr;
        left_begin.x = otdw.geom.seg.begC.x - dx * lambda;
        left_begin.y = otdw.geom.seg.begC.y - dy * lambda;
        left_end.x = left_begin.x + walldir_perp.x * op.base_reveal_width;
        left_end.y = left_begin.y + walldir_perp.y * op.base_reveal_width;
        lambda = (op.objLoc + op.width / 2) / dr;
        right_begin.x = otdw.geom.seg.begC.x - dx * lambda;
        right_begin.y = otdw.geom.seg.begC.y - dy * lambda;
        right_end.x = right_begin.x + walldir_perp.x * op.base_reveal_width;
        right_end.y = right_begin.y + walldir_perp.y * op.base_reveal_width;
        return true;
    }

    // Готовит боковые и, если проём не выходит выше стены, верхний откос, разбивая их по высотам отделки.
    // Пропускает недостаточную глубину или высоту и нулевую геометрию; при подготовке откоса ставит has_reveal.
    void OtdOpening_CreateReveals (GS::Array<OtdSlab> &otdslabs,
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
        if (!OtdOpening_CalculateRevealHeight (otdw, op, is_upper_wall, zDown, height))
            return;
        Point2D left_begin = {0, 0};
        Point2D left_end = {0, 0};
        Point2D right_begin = {0, 0};
        Point2D right_end = {0, 0};
        Sector walledge = {left_begin, left_end};
        // Строим перпендикулярные стенки к окну
        if (!OtdOpening_CalculateRevealCoordinates (
                otdw, op, walldir_perp, left_begin, left_end, right_begin, right_end))
            return;
        walledge = {left_begin, left_end};
        if (!walledge.IsZeroLength ()) {
            OtdWall wallotd;
            OtdWall_InitializeOpeningReveal (
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
            OtdWall_InitializeOpeningReveal (
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
            OtdWall_InitializeOpeningReveal (otdw,
                                             wallotd,
                                             otd_thickness,
                                             zDown + height,
                                             op.base_reveal_width,
                                             walledge.GetLength (),
                                             walledge.c2,
                                             walledge.c1,
                                             API_BeamID);
            wallotd.draw.is_upper_reveal = true;
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
    static TypeOtd OtdWall_GetFinishBandType (const OtdWall &otdw, TypeOtd type) {
        switch (type) {
        case Wall_Down:
            if (otdw.draw.base_type == API_SlabID)
                type = Floor;
            if (otdw.draw.base_type == API_WindowID)
                type = Reveal_Down;
            if (otdw.draw.type == Ceil)
                type = Ceil;
            if (otdw.draw.type == Floor)
                type = Floor;
            break;
        case Wall_Main:
            if (otdw.draw.base_type == API_ColumnID)
                type = Column;
            if (otdw.draw.base_type == API_WindowID)
                type = Reveal_Main;
            if (otdw.draw.type == Ceil)
                type = Ceil;
            if (otdw.draw.type == Floor)
                type = Floor;
            break;
        case Wall_Up:
            if (otdw.draw.type == Ceil)
                type = Ceil;
            if (otdw.draw.type == Floor)
                type = Floor;
            if (otdw.draw.base_type == API_WindowID)
                type = Reveal_Up;
            break;
        default:
            break;
        }
        return type;
    }

    // Выбирает резервный тип отделки по основанию; исходные типы потолка и пола имеют приоритет.
    static TypeOtd OtdWall_GetFallbackFinishType (const OtdWall &otdw, TypeOtd type) {
        if (otdw.draw.base_type == API_ColumnID)
            type = Column;
        if (otdw.draw.base_type == API_WindowID)
            type = Reveal_Main;
        if (otdw.draw.type == Ceil)
            type = Ceil;
        if (otdw.draw.type == Floor)
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
        TypeOtd type = otdw.draw.type;
        bool has_delim = false; // Найдена разбивка
        double height = 0;      // Высота элемента
        double zBottom = 0;     // Отметка низа
        // Панели
        if (otd_height_down > 0) {
            height = otd_height_down;
            zBottom = otd_zBottom;
            type = OtdWall_GetFinishBandType (otdw, Wall_Down);
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
            type = OtdWall_GetFinishBandType (otdw, Wall_Main);
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
            type = OtdWall_GetFinishBandType (otdw, Wall_Up);
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
            type = OtdWall_GetFallbackFinishType (otdw, type);
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
    static void OtdWall_TrimOpeningsToHeight (
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
    static bool OtdWall_CalculateHeightIntersection (
        const OtdWall &otdn, double &height, double &zBottom, double &zDup, double &zDown) {
        if (height < min_dim || is_equal (height, 0)) {
            return false;
        }
        const double otdnzBottom = OtdLinearSegment_GetZBottom (otdn.geom);
        // Проверяем - находится ли изначальная конструкция в этом диапазоне?
        if (otdnzBottom >= zBottom + height) {
            return false; // Конструкция начинается выше необходимого
        }
        if (otdnzBottom + otdn.geom.height <= zBottom) {
            return false; // Конструкция заканчивается ниже необходимого
        }
        zDup = fmin (zBottom + height, otdnzBottom + otdn.geom.height); // Новая отметка верха стены
        zDown = fmax (zBottom, otdnzBottom);                            // Новая отметка низа стены
        height = fmin (otdn.geom.height, zDup - zDown);                 // Новая высота стены
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
        if (!OtdWall_CalculateHeightIntersection (otdn, height, zBottom, zDup, zDown)) {
            return false;
        }
        double dlower = OtdLinearSegment_GetZBottom (otdn.geom) - zBottom; // Разница отметок стены до и после подрезки
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
        OtdWall_TrimOpeningsToHeight (localCopy, zBottom, height, zDup, zDown);
        if (localCopy.surface_geometry) {
            const double dz = zBottom - OtdLinearSegment_GetZBottom (otdn.geom);
            localCopy.geom.seg.begC.x += dz * localCopy.rise_x;
            localCopy.geom.seg.begC.y += dz * localCopy.rise_y;
            localCopy.geom.seg.endC.x += dz * localCopy.rise_x;
            localCopy.geom.seg.endC.y += dz * localCopy.rise_y;
        }
        OtdLinearSegment_SetZBottom (localCopy.geom, zBottom);
        localCopy.geom.height = height;
        if (localCopy.surface_geometry && OtdWall_GetArea (localCopy) <= 1.e-6)
            return false;
        if (!is_equal (localCopy.geom.width, 0))
            localCopy.geom.length = height;
        localCopy.draw.type = type;
        OtdWall_SetMaterialByType (
            localCopy, om_main, om_up, om_down, om_reveals, om_column, om_floor, om_ceil, om_zone);
        opw.Push (std::move (localCopy));
        return true;
    }

    // Выбирает материал по типу отделки, используя резервные материалы при пустом smaterial.
    // Уточняет тип по совпадению свойств записи; окончательный резерв — материал зоны.
    static void OtdWall_SelectFinishMaterial (TypeOtd &type,
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
            break;
        }
        if (material.smaterial.IsEmpty ())
            material = om_zone;
    }

    // Выбирает материал и уточняет тип стенки, учитывает состав основания и сохраняет итоговый материал.
    void OtdWall_SetMaterialByType (OtdWall &otdw,
                                    OtdMaterial &om_main,
                                    OtdMaterial &om_up,
                                    OtdMaterial &om_down,
                                    OtdMaterial &om_reveals,
                                    OtdMaterial &om_column,
                                    OtdMaterial &om_floor,
                                    OtdMaterial &om_ceil,
                                    OtdMaterial &om_zone) {
        OtdMaterial material;
        OtdWall_SelectFinishMaterial (
            otdw.draw.type, material, om_main, om_up, om_down, om_reveals, om_column, om_floor, om_ceil, om_zone);
        OtdMaterial_SetFinish (material, otdw.base_composite);
        otdw.draw.material = material;
    }

    // Заменяет покрытие материала индексом и именем последнего слоя состава только при structype == -1.
    static void OtdMaterial_SetFinishByComposite (OtdMaterial &material,
                                                  GS::Array<ParamValueComposite> &base_composite) {
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
    static void OtdMaterial_SetFinish (OtdMaterial &material, GS::Array<ParamValueComposite> &base_composite) {
        OtdMaterial_SetFinishByComposite (material, base_composite);
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
    static bool Edge_FindEdge (Sector &edge, GS::Array<Sector> &edges) {
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

    // -----------------------------------------------------------------------------
    // Выбирает GUID класса по типу отделки; для откосов и нижней стены пробует также общий класс стен.
    // Если специфичный класс не найден  получают класс all_class.
    // -----------------------------------------------------------------------------
    API_Guid Class_GetClassGuid (const TypeOtd &type, const ClassificationFunc::ClassificationDict &finclass) {
        switch (type) {
        case Floor_Skirting:
            if (auto *p = finclass.GetPtr (floor_skirting_class))
                return p->item.guid;
            break;
            if (auto *p = finclass.GetPtr (floor_class))
                return p->item.guid;
            break;
        case Ceil_Cornice:
            if (auto *p = finclass.GetPtr (ceil_cornice_class))
                return p->item.guid;
            break;
            if (auto *p = finclass.GetPtr (ceil_class))
                return p->item.guid;
            break;
        case Column:
            if (auto *p = finclass.GetPtr (column_class))
                return p->item.guid;
            break;

        case Reveal_Main:
        case Reveal_Up:
        case Reveal_Down:
            if (auto *p = finclass.GetPtr (reveal_class))
                return p->item.guid;
            if (auto *p = finclass.GetPtr (wall_class))
                return p->item.guid;
            break;

        case Wall_Down:
            if (auto *p = finclass.GetPtr (wall_down_class))
                return p->item.guid;
            if (auto *p = finclass.GetPtr (wall_class))
                return p->item.guid;
            break;

        case Wall_Main:
        case Wall_Up:
            if (auto *p = finclass.GetPtr (wall_class))
                return p->item.guid;
            break;

        case Floor:
            if (auto *p = finclass.GetPtr (floor_class))
                return p->item.guid;
            break;

        case Ceil:
            if (auto *p = finclass.GetPtr (ceil_class))
                return p->item.guid;
            break;

        case Beam_Ceil:
            if (auto *p = finclass.GetPtr (beam_ceil_class))
                return p->item.guid;
            break;

        case Beam_Wall:
            if (auto *p = finclass.GetPtr (beam_wall_class))
                return p->item.guid;
            break;

        default:
            break;
        }
        if (auto *p = finclass.GetPtr (all_class))
            return p->item.guid;
        return APINULLGuid;
    }

    // -----------------------------------------------------------------------------
    // Сопоставляет GUID с классами стен, нижней стены, потолка, пола, колонн, откосов и балок в порядке проверок.
    // общий и неизвестный класс возвращают NoSet.
    // -----------------------------------------------------------------------------
    TypeOtd Class_GetOtdTypeByClass (const API_Guid &classguid,
                                     const ClassificationFunc::ClassificationDict &finclass,
                                     const API_ElemTypeID &type) {
        if (const auto *p = finclass.GetPtr (floor_skirting_class)) {
            if (p->item.guid == classguid)
                return Floor_Skirting;
        }

        if (const auto *p = finclass.GetPtr (ceil_cornice_class)) {
            if (p->item.guid == classguid)
                return Ceil_Cornice;
        }

        if (const auto *p = finclass.GetPtr (wall_class)) {
            if (p->item.guid == classguid)
                return Wall_Main;
        }

        if (const auto *p = finclass.GetPtr (wall_down_class)) {
            if (p->item.guid == classguid)
                return Wall_Down;
        }

        if (const auto *p = finclass.GetPtr (ceil_class)) {
            if (p->item.guid == classguid) {
                if (type == API_BeamID) {
                    return Ceil_Cornice;
                } else {
                    return Ceil;
                }
            }
        }
        if (const auto *p = finclass.GetPtr (floor_class)) {
            if (p->item.guid == classguid) {
                if (type == API_BeamID) {
                    return Floor_Skirting;
                } else {
                    return Floor;
                }
            }
        }

        if (const auto *p = finclass.GetPtr (column_class)) {
            if (p->item.guid == classguid)
                return Column;
        }

        if (const auto *p = finclass.GetPtr (reveal_class)) {
            if (p->item.guid == classguid)
                return Reveal_Main;
        }

        if (const auto *p = finclass.GetPtr (reveal_windows_class)) {
            if (p->item.guid == classguid)
                return Reveal_Main;
        }

        if (const auto *p = finclass.GetPtr (reveal_doors_class)) {
            if (p->item.guid == classguid)
                return Reveal_Main;
        }

        if (const auto *p = finclass.GetPtr (beam_ceil_class)) {
            if (p->item.guid == classguid)
                return Beam_Ceil;
        }

        if (const auto *p = finclass.GetPtr (beam_wall_class)) {
            if (p->item.guid == classguid)
                return Beam_Wall;
        }
        return NoSet;
    }

    // -----------------------------------------------------------------------------
    // Поиск классов для отделочных стен (some_stuff_fin_ в описании класса)
    // -----------------------------------------------------------------------------
    void Class_FindFinClass (ClassificationFunc::ClassificationDict &findict, UnicGuid &finclassguids) {
        auto &cache = PROPERTYCACHE ();
        if (!cache.findict.IsEmpty () && cache.finclassguids.IsEmpty ()) {
            findict = cache.findict;
            finclassguids = cache.finclassguids;
        }
        if (!ClassificationFunc::ReadSystemDict ())
            return;
        ClassificationFunc::SystemDict &systemdict = cache.systemdict;
        findict.Clear ();
        finclassguids.Clear ();
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
                if (desc.Contains (otd_class_prefix)) {
                    for (const auto &cls : class_all) {
                        if (desc.Contains (cls)) {
                            findict.Add (cls, clas);
                            if (!finclassguids.ContainsKey (clas.item.guid))
                                finclassguids.Add (clas.item.guid, false);
                            // Ранний выход при нахождении не нужен - в одном классе может быть несколько описаний
                        }
                    }
                }
            }
        }
        cache.findict.Clear ();
        cache.finclassguids.Clear ();
        cache.findict = findict;
        cache.finclassguids = finclassguids;
    }

    // Ищет среди классификаций элемента первый GUID, присутствующий в finclassguids, записывает его в classguid и
    // возвращает true. При ошибке чтения или отсутствии совпадения возвращает false, не меняя classguid.
    bool Class_IsElementFinClass (const API_Guid &elGuid,
                                  const UnicGuid &finclassguids,
                                  API_Guid &classguid,
                                  RoomSurfaceCache *cache) {
        const ClassificationRead read = Class_ReadItems (elGuid, cache);
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
    static void Param_AddUnicGUIDByType (const API_Guid &elGuid,
                                         API_ElemTypeID elemtype,
                                         UnicGUIDByType &guidselementToRead) {
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

    // Собирает избранное, включая балки плинтусов, в словарь по обрезанным именам в нижнем регистре.
    // Повторные имена пропускает и сообщает об отсутствии рекомендуемых имён.
    bool Favorite_GetDict (MatarialToFavoriteDict &favdict) {
        // Словарь с избранным
        favdict.Clear ();
        // Поиск элементов по-умолчанию
        short count = 0;
        GS::Array<GS::UniString> names;
        const API_ElemTypeID favtypes[] = {
            API_WindowID, API_DoorID, API_WallID, API_ColumnID, API_SlabID, API_ObjectID, API_ZoneID, API_BeamID};
        for (const auto &type : favtypes) {
            if (Favorite_GetNum (type, &count, nullptr, &names) != NoError)
                continue;
            for (const GS::UniString &name : names) {
                GS::UniString key = name;
                key.SetToLowerCase ();
                key.Trim ();
                if (favdict.ContainsKey (key))
                    continue;
                MatarialToFavorite fav;
                fav.type = type;
                fav.name = key;
                if (type == API_SlabID) {
                    API_Element element = {};
                    if (Favorite_ReadByName (name, element) && GetElemTypeID (element) == API_SlabID) {
                        if (element.slab.modelElemStructureType == API_BasicStructure) {
                            fav.thickness = element.slab.thickness;
                        } else if (element.slab.modelElemStructureType == API_CompositeStructure) {
    #ifdef ServerMainVers_2700
                            const Int32 compositeKey = element.slab.composite.ToInt32_Deprecated ();
    #else
                            const Int32 compositeKey = static_cast<Int32> (element.slab.composite);
    #endif
                            auto &cache = PROPERTYCACHE ().compositeCache;
                            const auto *layers = cache.GetPtr (compositeKey);
                            if (layers == nullptr) {
                                ParamDictValue params, paramsAdd;
                                ParamDictComposite composites;
                                double width = 0, length = 0;
                                if (ParamHelpers::ComponentsCompositeStructure (APINULLGuid,
                                                                                element.slab.composite,
                                                                                params,
                                                                                composites,
                                                                                paramsAdd,
                                                                                width,
                                                                                length))
                                    layers = cache.GetPtr (compositeKey);
                            }
                            if (layers != nullptr) {
                                // Нужна вся толщина, а не отфильтрованные отделочные слои fav.composite.
                                for (const auto &layer : *layers)
                                    fav.thickness += layer.fillThick;
                            }
                        }
                    }
                }
                favdict.Add (key, fav);
            }
        }
        if (favdict.IsEmpty ()) {
            msg_rep ("RoomBook favorite", "Recommended items not found in favorites", NoError, APINULLGuid);
        } else {
            GS::UniString msg;
            if (!favdict.ContainsKey (favorite_wall))
                msg += favorite_wall;
            if (!favdict.ContainsKey (favorite_floor))
                msg += favorite_floor;
            if (!favdict.ContainsKey (favorite_ceil))
                msg += favorite_ceil;
            if (!favdict.ContainsKey (favorite_window))
                msg += favorite_window;
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
            if (!favdict.ContainsKey (favorite_reveal_side))
                msg += favorite_reveal_side;
            if (!favdict.ContainsKey (favorite_reveal_up))
                msg += favorite_reveal_up;
            if (!msg.IsEmpty ()) {
                if (favdict.ContainsKey (favorite_wall)) {
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
        return !favdict.IsEmpty ();
    }

    // Читает только настройки выбранного избранного, не активируя слой и не подставляя настройки по умолчанию.
    bool Favorite_ReadByName (const GS::UniString &favorite_name, API_Element &element) {
    #ifndef ServerMainVers_2300
        return false;
    #else
        if (favorite_name.IsEmpty ())
            return false;
        API_Favorite favorite (favorite_name);
        if (ACAPI_Favorite_Get (&favorite) != NoError)
            return false;
        element = favorite.element;
        return true;
    #endif
    }

    // Подготавливает плинтусы до записи модели; исходные OtdSlab не сдвигает.
    static bool OtdBeam_ProcessFinishes (OtdRoom &room,
                                         ParamDictValue &favoriteParams,
                                         ParamValue &compositeParam,
                                         MatarialToFavoriteDict &favdict) {
        room.otdbeam_floor.Clear ();
        room.otdbeam_ceil.Clear ();
        for (auto &slab : room.otdslab)
            slab.length_skirting = 0;
        GS::Array<OtdSlab> faces = room.otdslab;
        for (auto &face : faces) {
            if (!face.isValid || (face.draw.type != Floor && face.draw.type != Ceil))
                continue;
            if (face.draw.type == Floor ? (!room.has_floor || !room.create_floor_elements)
                                        : (!room.has_ceil || !room.create_ceil_elements)) {
                face.isValid = false;
                continue;
            }
            const bool meshRoof = face.surface_geometry && face.draw.type == Floor &&
                                  face.draw.base_type == API_MeshID && face.draw.draw_type == API_RoofID &&
                                  face.draw.favorite.type == API_SlabID && !face.draw.favorite.name.IsEmpty ();
            if (meshRoof) {
                const double thickness = face.draw.favorite.thickness;
                const double normalLength = std::hypot (1.0, std::hypot (face.slope_x, face.slope_y));
                if (!std::isfinite (face.slope_x) || !std::isfinite (face.slope_y) || !std::isfinite (thickness) ||
                    thickness <= 0 || !std::isfinite (normalLength))
                    return false;
                // Mesh-writer сохраняет XY-контур и вертикальные торцы; толщина Roof задана по нормали.
                face.zBottom += thickness * normalLength;
                if (!std::isfinite (face.zBottom))
                    return false;
                continue;
            }
            if (!face.surface_geometry || std::hypot (face.slope_x, face.slope_y) > DistanceTolerance) {
                msg_rep ("Skirting", "Unsupported prepared slab writer; skirting skipped", NoError, room.zone_guid);
                face.isValid = false;
                continue;
            }
            // Object_Draw_Poly задаёт 2 см; для Slab толщина прочитана вместе с избранным.
            const double thickness =
                face.draw.favorite.type == API_ObjectID
                    ? 0.02
                    : (face.draw.favorite.name.IsEmpty () ? otd_thickness : face.draw.favorite.thickness);
            if (!std::isfinite (thickness) || thickness <= 0)
                return false;
            face.zBottom += face.draw.type == Floor ? thickness : -thickness;
        }
        if (!SkirtingSegments_Build (room, faces))
            return false;
        GS::UniString tip;
        for (auto *beams : {&room.otdbeam_floor, &room.otdbeam_ceil}) {
            for (auto &beam : *beams) {
                // Суммируем готовые стыки в метрах, независимо от последующей записи native-элементов.
                if (beam.slabIndex >= 0 && static_cast<UIndex> (beam.slabIndex) < room.otdslab.GetSize ()) {
                    const double length = beam.geom.Length ();
                    if (std::isfinite (length)) {
                        room.otdslab[static_cast<UIndex> (beam.slabIndex)].length_skirting += length;
                        if (!beam.tip.IsEmpty ()) {
                            room.otdslab[static_cast<UIndex> (beam.slabIndex)].tip_skirting = beam.tip;
                        }
                    }
                }
                Favorite_FindName (beam.draw.favorite,
                                   beam.draw.material,
                                   beam.draw.type,
                                   API_BeamID,
                                   favoriteParams,
                                   compositeParam,
                                   tip,
                                   favdict);
            }
        }
        return true;
    }

    // Помечает избранное как прочитанное до получения элемента и чтения материалов; при дальнейшей ошибке флаг не
    // сбрасывает. При наличии требуемого параметра и состава материала заполняет composite через Param_SetComposite.
    void Favorite_ReadComposite (const ParamValue &param_composite,
                                 ParamDictValue &paramToRead_favorite,
                                 const GS::UniString &fav_name,
                                 MatarialToFavoriteDict &favdict) {

        // ОПТИМИЗАЦИЯ: 1 поиск в favdict вместо 5 (!) с одновременной проверкой флага
        auto *favData = favdict.GetPtr (fav_name);
        if (favData == nullptr || favData->is_composite_read)
            return;

        favData->is_composite_read = true;

        API_Element element = {};
        if (!Favorite_ReadByName (favData->name, element))
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
        const bool flipped = GetElemTypeID (element) == API_BeamID ? false : element.wall.flipped;
        Param_SetComposite (*base_composite, flipped, favData->composite, fav_name_, true);
    }

    static void Favorite_GetPossiblyType (const TypeOtd &type,
                                          const API_ElemTypeID &draw_type,
                                          API_ElemTypeID &possibly_type,
                                          const MatarialToFavoriteDict &favdict) {
        GS::UniString favorite_name_defult;
        switch (type) {
        case Reveal_Main:
        case Reveal_Up:
        case Reveal_Down:
            if (draw_type == API_BeamID) {
                favorite_name_defult = favorite_reveal_up;
            }
            if (!favdict.ContainsKey (favorite_name_defult) || favorite_name_defult.IsEmpty ()) {
                possibly_type = API_WallID;
            }
            break;
        case Floor:
            if (draw_type == API_WallID) {
                possibly_type = API_WallID;
            }
            break;
        case Ceil:
            if (draw_type == API_WallID) {
                possibly_type = API_WallID;
            }
            break;
        default:
            break;
        }
        return;
    }

    static GS::UniString Favorite_GetDefultName (const TypeOtd &type,
                                                 const API_ElemTypeID &draw_type,
                                                 const MatarialToFavoriteDict &favdict) {
        GS::UniString favorite_name_defult;
        switch (type) {
        case Wall_Main:
        case Wall_Up:
        case Wall_Down:
        case Column:
            favorite_name_defult = favorite_wall;
            break;
        case Reveal_Main:
        case Reveal_Up:
        case Reveal_Down:
            if (draw_type == API_BeamID) {
                favorite_name_defult = favorite_reveal_up;
            }
            if (!favdict.ContainsKey (favorite_name_defult) || favorite_name_defult.IsEmpty ()) {
                favorite_name_defult = favorite_wall;
            }
            break;
        case Floor:
            favorite_name_defult = favorite_floor;
            if (draw_type == API_WallID) {
                favorite_name_defult = favorite_wall;
            }
            break;
        case Ceil:
            favorite_name_defult = favorite_ceil;
            if (draw_type == API_WallID) {
                if (favdict.ContainsKey (favorite_ceil_wall)) {
                    favorite_name_defult = favorite_ceil_wall;
                } else if (favdict.ContainsKey (favorite_wall)) {
                    favorite_name_defult = favorite_wall;
                }
            }
            break;
        default:
            break;
        }
        return favorite_name_defult;
    }

    // Подбирает избранное по имени материала и суффиксу, затем по текущему или рекомендуемому имени.
    // Учитывает draw_type и допустимый объект-аксессуар; выбранную запись копирует вместе с прочитанным составом.
    static void Favorite_FindName (MatarialToFavorite &favorite,
                                   const OtdMaterial &material,
                                   const TypeOtd &type,
                                   const API_ElemTypeID &draw_type,
                                   ParamDictValue &paramToRead_favorite,
                                   ParamValue &param_composite,
                                   GS::UniString &tip,
                                   MatarialToFavoriteDict &favdict) {
        if (favdict.IsEmpty ()) {
            return;
        }
        API_ElemTypeID possibly_type = draw_type;
        GS::UniString fav_name = favorite.name;

        // Обработка плинтусов/карнизов: если имя не задано, ищем в словаре по рекомендуемому имени
        if (type == Floor_Skirting || type == Ceil_Cornice) {
            if (fav_name.IsEmpty ())
                fav_name = type == Floor_Skirting ? favorite_floor_skirting : favorite_ceil_cornice;
            favorite = MatarialToFavorite{};
            const auto *entry = favdict.GetPtr (fav_name);
            if (entry != nullptr && (entry->type == API_BeamID || entry->type == API_ObjectID))
                favorite = *entry;
            if (favorite.name.IsEmpty ()) {
                favorite.type = draw_type;
            }
            return;
        }

        // Обработка имени избранного, заданного в имени покрытия материала; при наличии @ извлекает имя избранного от @
        // и до конца строки
        if (material.smaterial.Contains (ATSIGN)) {
            GS::UniString part = UniStringToLower (material.smaterial) + '@';
            part = part.GetSubstring ('@', '@', 0);
            part.Trim ();
            if (!fav_name.IsEmpty ()) {
                GS::UniString part_full = part + "_" + fav_name;
                if (auto *favPtr = favdict.GetPtr (part_full)) {
                    if (favPtr->type == draw_type || favPtr->type == API_ObjectID) {
                        Favorite_ReadComposite (param_composite, paramToRead_favorite, part_full, favdict);
                        if (auto *updatedFavPtr = favdict.GetPtr (part_full)) {
                            favorite = *updatedFavPtr;
                        }
                        return;
                    }
                }
            }
            if (auto *favPtr = favdict.GetPtr (part)) {
                if (favPtr->type == draw_type || favPtr->type == API_ObjectID) {
                    Favorite_ReadComposite (param_composite, paramToRead_favorite, part, favdict);
                    if (auto *updatedFavPtr = favdict.GetPtr (part)) {
                        favorite = *updatedFavPtr;
                    }
                    return;
                }
            }
        }

        // Ищет имя избранного в составе конструкции.
        if (!fav_name.IsEmpty ()) {
            if (auto *favPtr = favdict.GetPtr (fav_name)) {
                if (favPtr->type == draw_type || favPtr->type == API_ObjectID) {
                    Favorite_ReadComposite (param_composite, paramToRead_favorite, fav_name, favdict);
                    if (auto *updatedFavPtr = favdict.GetPtr (fav_name)) {
                        favorite = *updatedFavPtr;
                    }
                    return;
                }
            }
        }

        // Обработка полов/потолков.
        if (favorite.type == API_ZombieElemID && (type == Ceil || type == Floor) && !tip.IsEmpty ()) {
            GS::UniString tipLower = "_";
            tipLower += UniStringToLower (tip);
            tipLower += "_";
            for (MatarialToFavoriteDict::PairIterator cIt = favdict.EnumeratePairs (); cIt != NULL; ++cIt) {
    #ifdef ServerMainVers_2800
                const GS::UniString &fav = cIt->key;
    #else
                const GS::UniString &fav = *cIt->key;
    #endif
                if (fav.Contains (tipLower) && fav.Contains (favorite_suffix)) {
                    if (auto *favPtr = favdict.GetPtr (fav)) {
                        if (favPtr->type == draw_type || favPtr->type == API_ObjectID) {
                            Favorite_ReadComposite (param_composite, paramToRead_favorite, fav, favdict);
                            if (auto *updatedFavPtr = favdict.GetPtr (fav)) {
                                favorite = *updatedFavPtr;
                            }
                            return;
                        }
                    }
                }
            }
        }

        // Если не найдено ни одного совпадения, ищет рекомендуемое имя избранного по типу отделки и проверяет его
        // наличие в словаре.
        if (favorite.type == API_ZombieElemID) {
            GS::UniString favorite_name_defult = Favorite_GetDefultName (type, draw_type, favdict);
            Favorite_GetPossiblyType (type, draw_type, possibly_type, favdict);
            if (auto *favPtr = favdict.GetPtr (favorite_name_defult)) {
                if (favPtr->type == possibly_type || favPtr->type == API_ObjectID) {
                    Favorite_ReadComposite (param_composite, paramToRead_favorite, favorite_name_defult, favdict);
                    if (auto *updatedFavPtr = favdict.GetPtr (favorite_name_defult)) {
                        favorite = *updatedFavPtr;
                    }
                    return;
                }
            }
        }

        // Если не найдено ни одного совпадения, оставляет пустое имя и устанавливает тип в draw_type.
        if (favorite.name.IsEmpty ()) {
            favorite.type = draw_type;
        }
        return;
    }

    // При успешном чтении избранного копирует элемент, вызывает UnhideUnlockElementLayer и возвращает true.
    // Для пустого имени, ошибки чтения или отсутствия ServerMainVers_2300 возвращает false.
    bool Favorite_GetByName (const GS::UniString &favorite_name, API_Element &element) {
        if (!Favorite_ReadByName (favorite_name, element))
            return false;
        UnhideUnlockElementLayer (element.header);
        return true;
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

    // -----------------------------------------------------------------------------
    // Собирает настройки инструмента для типа отделки и сохраняет их в избранное с заданным именем.
    // -----------------------------------------------------------------------------
    bool Favorite_CreateOtd (const TypeOtd &type, const GS::UniString &name, const GS::UniString &folder) {

        ClassificationFunc::ClassificationDict finclass;
        UnicGuid finclassguids;
        Class_FindFinClass (finclass, finclassguids);

        API_Element element = {};
        API_ElementMemo memo = {};
        bool withMemo = false;
        bool ready = false;
        switch (type) {
        case Floor:
        case Ceil:
            // Полы и потолки — перекрытия.
            ready = Slab_GetDefault (EMPTYSTRING, type, element);
            break;
        case Floor_Skirting:
        case Ceil_Cornice:
        case Reveal_Up:
            // Плинтус пола и потолка, а также верхний откос — балки.
            withMemo = true;
            ready = Beam_GetDefault (EMPTYSTRING, type, element, memo);
            break;
        case Wall_Main:
        case Wall_Up:
        case Wall_Down:
        case Reveal_Main:
        case Reveal_Down:
            // Стены отделки и остальные откосы — стены.
            ready = Wall_GetDefault (EMPTYSTRING, type, element);
            break;
        default:
            return false;
        }
        if (!ready) {
            ACAPI_DisposeElemMemoHdls (&memo);
            return false;
        }
        if (name.IsEmpty ())
            return true;
        API_Favorite favorite (name);
        favorite.element = element;
        if (!folder.IsEmpty ()) {
            API_FavoriteFolderHierarchy favoriteFolder;
            favoriteFolder.Push (folder);
            favorite.folder = favoriteFolder;
        }

        // Классификация избранного — пара (система, класс); GUID системы берём из той же записи словаря отделочных
        // классов.
        const API_Guid classguid = Class_GetClassGuid (type, finclass);
        if (classguid != APINULLGuid) {
            for (const auto &clas : finclass) {
    #ifdef ServerMainVers_2800
                const ClassificationFunc::ClassificationValues *cv = &clas.value;
    #else
                const ClassificationFunc::ClassificationValues *cv = clas.value;
    #endif
                if (cv == nullptr || cv->item.guid != classguid)
                    continue;
                favorite.classifications.New ();
                favorite.classifications.Get ().Push (GS::Pair<API_Guid, API_Guid> (cv->system.guid, classguid));
                break;
            }
        }

        if (withMemo) {
            favorite.memo.New ();
            favorite.memo.Get () = memo;
            BNZeroMemory (&memo, sizeof (API_ElementMemo));
        }
        const GS::OnExit favoriteMemoGuard ([&favorite] () {
            if (favorite.memo.HasValue ())
                ACAPI_DisposeElemMemoHdls (&favorite.memo.Get ());
        });
        const GSErrCode err = ACAPI_Favorite_Create (favorite);
        if (err != NoError) {
            msg_rep ("Favorite_CreateOtd", "ACAPI_Favorite_Create", err, APINULLGuid);
            return false;
        }
        return true;
    }

    // Для JSON: собирает классы отделки из систем классификации и выполняет общую проверку конфигурации,
    // возвращая список ошибок текстом для передачи в json.
    bool Roombook_CheckFinishConfiguration (GS::UniString &errors) {
        ClassificationFunc::ClassificationDict finclass;
        UnicGuid finclassguids;
        Class_FindFinClass (finclass, finclassguids);
        return Roombook_CheckFinishConfiguration (finclass, finclassguids, errors);
    }

    // Проверяет наличие классов отделки и доступность Sync_GUID zone для GUID в finclassguids.
    // Уже установленные флаги не сбрасывает; проблемы Sync_GUID base element сообщает без отказа расчёта.
    bool Roombook_CheckFinishConfiguration (const ClassificationFunc::ClassificationDict &finclass,
                                            UnicGuid &finclassguids) {
        GS::UniString errors;
        return Roombook_CheckFinishConfiguration (finclass, finclassguids, errors);
    }

    // Тот же контракт, но собирает все ошибки в errors (по одной на строку); сообщения также идут в отчёт.
    bool Roombook_CheckFinishConfiguration (const ClassificationFunc::ClassificationDict &finclass,
                                            UnicGuid &finclassguids,
                                            GS::UniString &errors) {
        errors.Clear ();
        bool ok = true;
        // Проверка наличия классов
        static const Int32 iseng = ID_ADDON_STRINGS + isEng ();
        if (finclass.IsEmpty ()) {
            GS::UniString msgString = RSGetIndString (iseng, 50, ACAPI_GetOwnResModule ());
            msg_rep ("RoomBook err", msgString, APIERR_GENERAL, APINULLGuid);
            SafeWriteReport (msgString, true);
            errors += msgString;
            return false;
        }
        if (!finclass.ContainsKey (all_class)) {
            bool msg = false;
            if (!finclass.ContainsKey (floor_class))
                msg = true;
            if (!finclass.ContainsKey (ceil_class))
                msg = true;
            if (!finclass.ContainsKey (wall_class))
                msg = true;
            if (msg) {
                GS::UniString msgString = RSGetIndString (iseng, 51, ACAPI_GetOwnResModule ());
                msg_rep ("RoomBook err", msgString, APIERR_GENERAL, APINULLGuid);
                errors += msgString;
                ok = false;
            }
        }
        // Проверка наличия свойств `Sync_GUID zone`
        if (!ParamHelpers::isPropertyDefinitionRead ()) {
            GS::UniString msgString = RSGetIndString (iseng, 51, ACAPI_GetOwnResModule ());
            msg_rep ("RoomBook err", msgString, APIERR_GENERAL, APINULLGuid);
            errors += msgString;
            return false;
        }
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
                // Уточняем сообщение именем класса, чтобы список ошибок был читаемым в json.
                for (GS::HashTable<GS::UniString, ClassificationFunc::ClassificationValues>::ConstPairIterator cCl =
                         finclass.EnumeratePairs ();
                     cCl != NULL;
                     ++cCl) {
    #ifdef ServerMainVers_2800
                    // AC28+: поля пары — ссылки, разыменование не нужно.
                    if (cCl->value.item.guid == cIt->key) {
                        msgString += " ('" + cCl->key + "')";
    #else
                    if ((*cCl->value).item.guid == *cIt->key) {
                        msgString += " ('" + *cCl->key + "')";
    #endif
                        break;
                    }
                }
                msg_rep ("RoomBook err", msgString, APIERR_GENERAL, APINULLGuid);
                SafeWriteReport (msgString, true);
                if (!errors.IsEmpty ())
                    errors += "\n";
                errors += msgString;
                ok = false;
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
                     "Missing 'Sync_GUID base element' property. Floor/ceiling/skirting/cornice updates "
                     "require this property. Existing elements will be replaced",
                     NoError,
                     APINULLGuid);
        }
        return ok;
    }
} // namespace Roombook
#endif
