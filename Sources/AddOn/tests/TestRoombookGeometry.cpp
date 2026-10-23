#include "ACAPinc.h"

#include "tests/ConnectedElementsCacheChecks.hpp"
#include "tests/FloorCoreChecks.hpp"
#include "tests/RoomAnalyticChecks.hpp"
#include "tests/TestFunc.hpp"

#if defined(TESTING) && defined(ServerMainVers_2500)

    #include <cmath>
    #include <limits>
    #include <vector>

    #include "CommonFunction.hpp"
    #include "roombook/Roombook.hpp"
    #include "tests/TestKit.hpp"

namespace TestFunc {

    void TestRoomSurfaceTransform () {
        const auto check = [&] (bool passed, const char *name) {
            DBtest (passed, "surface transform " + GS::UniString (name));
        };
        const auto close = [] (double actual, double expected) { return std::abs (actual - expected) < 1e-12; };
        const auto makeSurface = [] () {
            Roombook::RoomSurface surface;
            surface.normalLocal = {-1, 0, 1};
            const API_Coord3D points[2][4] = {{{0, 0, 0}, {2, 0, 2}, {2, 2, 2}, {0, 2, 0}},
                                              {{0.5, 0.5, 0.5}, {0.5, 1.5, 0.5}, {1.5, 1.5, 1.5}, {1.5, 0.5, 1.5}}};
            for (Int32 loop = 0; loop < 2; ++loop) {
                Roombook::RoomSurfaceContour contour;
                contour.hole = loop != 0;
                for (const API_Coord3D &point : points[loop]) {
                    Roombook::RoomSurfaceVertex vertex;
                    vertex.local = point;
                    contour.vertices.Push (vertex);
                }
                surface.contours.Push (contour);
            }
            return surface;
        };
        const API_Tranmat matrices[] = {{{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0}},
                                        {{2, 1, 0, 10, 0, 3, 0, -4, 0, 0, 4, 7}},
                                        {{0, -1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0}},
                                        {{-1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0}}};
        const double unit = 1 / std::sqrt (2.0);
        const API_Coord3D expectedNormals[] = {
            {-unit, 0, unit}, {-6.0 / 7, 2.0 / 7, 3.0 / 7}, {0, -unit, unit}, {unit, 0, unit}};
        const char *names[] = {"identity", "scale/shear/translation", "rotation", "reflection"};
        for (Int32 i = 0; i < 4; ++i) {
            Roombook::RoomSurface surface = makeSurface ();
            check (Roombook::RoomSurface_Transform (matrices[i], surface), names[i]);
            check (surface.contours.GetSize () == 2 && !surface.contours[0].hole && surface.contours[1].hole &&
                       surface.contours[0].vertices.GetSize () == 4 && surface.contours[1].vertices.GetSize () == 4,
                   "contours and hole preserved");
            check (close (surface.normalWorld.x, expectedNormals[i].x) &&
                       close (surface.normalWorld.y, expectedNormals[i].y) &&
                       close (surface.normalWorld.z, expectedNormals[i].z),
                   "oriented inverse-transpose normal");
            bool onPlane = true;
            bool localPreserved = true;
            const Roombook::RoomSurface original = makeSurface ();
            for (UInt32 loop = 0; loop < surface.contours.GetSize (); ++loop) {
                for (UInt32 v = 0; v < surface.contours[loop].vertices.GetSize (); ++v) {
                    const Roombook::RoomSurfaceVertex &vertex = surface.contours[loop].vertices[v];
                    onPlane = onPlane &&
                              close (surface.normalWorld.x * vertex.world.x + surface.normalWorld.y * vertex.world.y +
                                         surface.normalWorld.z * vertex.world.z + surface.planeOffset,
                                     0);
                    const API_Coord3D &local = original.contours[loop].vertices[v].local;
                    localPreserved = localPreserved && vertex.local.x == local.x && vertex.local.y == local.y &&
                                     vertex.local.z == local.z;
                }
            }
            check (onPlane, "outer and hole on world plane");
            check (localPreserved, "local vertices unchanged");
            if (i == 1) {
                const API_Coord3D &world = surface.contours[0].vertices[1].world;
                check (close (world.x, 14) && close (world.y, -4) && close (world.z, 15) &&
                           close (surface.planeOffset, 47.0 / 7),
                       "affine world point and plane offset");
            }
        }
        API_Tranmat invalid = matrices[0];
        invalid.tmx[0] = 0;
        Roombook::RoomSurface surface = makeSurface ();
        check (!Roombook::RoomSurface_Transform (invalid, surface), "singular matrix rejected");
        invalid = matrices[0];
        invalid.tmx[3] = std::numeric_limits<double>::infinity ();
        surface = makeSurface ();
        check (!Roombook::RoomSurface_Transform (invalid, surface), "nonfinite matrix rejected");
        surface = makeSurface ();
        surface.normalLocal = {};
        check (!Roombook::RoomSurface_Transform (matrices[0], surface), "zero normal rejected");
        surface = makeSurface ();
        surface.contours[0].vertices[0].local.x = std::numeric_limits<double>::quiet_NaN ();
        check (!Roombook::RoomSurface_Transform (matrices[0], surface), "nonfinite vertex rejected");
        surface = makeSurface ();
        surface.contours.Clear ();
        check (!Roombook::RoomSurface_Transform (matrices[0], surface), "empty contours rejected");
        surface = makeSurface ();
        surface.contours[0].hole = true;
        check (!Roombook::RoomSurface_Transform (matrices[0], surface), "missing outer contour rejected");
        surface = makeSurface ();
        surface.contours[1].vertices.Clear ();
        check (!Roombook::RoomSurface_Transform (matrices[0], surface), "invalid hole rejected");
    }

    void TestRoomSurfacePartition () {
        using namespace Roombook;
        const auto check = [&] (bool passed, const char *name) {
            DBtest (passed, "surface partition " + GS::UniString (name));
        };

        CheckRoomAnalyticGeometry (check);
        CheckConnectedElementsCache (check);
        {
            API_Element element = {};
            SetElemTypeID (element, API_WallID);
            element.header.guid = APIGuidFromString ("00000000-0000-0000-0000-000000000272");
            element.wall.type = APIWtyp_Normal;
            element.wall.profileType = APISect_Normal;
            element.wall.begC = {0, 0};
            element.wall.endC = {4, 0};
            element.wall.height = 3;
            element.wall.thickness = .3;
            element.wall.offsetFromOutside = .1;
            GS::Array<OtdWall> bases;
            RoomSurfaceSource prepared;
            check (OtdWall_BuildPreparedSource (element, 10, {}, bases, prepared) && bases.GetSize () == 4 &&
                       prepared.origin == RoomSurfaceOrigin::PreparedAnalytic &&
                       prepared.snapshot.bodies[0].surfaces.GetSize () == 6,
                   "production ordinary wall preparation");
            bool ownIndices = true;
            for (const auto &face : prepared.snapshot.bodies[0].surfaces)
                ownIndices = ownIndices && face.bodyIndex == 0 && face.polygonIndex == 0 &&
                             face.origin == RoomSurfaceOrigin::PreparedAnalytic && face.analyticFaceId > 0;
            check (ownIndices, "analytic provenance has no invented native indices");
            OtdOpening opening;
            opening.base_guid = APIGuidFromString ("00000000-0000-0000-0000-000000000273");
            opening.objLoc = 2;
            opening.width = 2;
            opening.height = 1;
            opening.lower = 1;
            bases.Clear ();
            check (OtdWall_BuildPreparedSource (element, 10, {opening}, bases, prepared) &&
                       bases[0].openings.GetSize () == 1 && bases[0].openings[0].base_guid == opening.base_guid &&
                       bases[1].openings[0].base_guid == opening.base_guid,
                   "opening GUID retained on both prepared sides");
            // Прямоугольная native-зона на исходной боковой плоскости проверяет активный matcher без изменения его
            // допуска.
            RoomSurface zoneFace;
            zoneFace.bodyIndex = zoneFace.polygonIndex = 1;
            zoneFace.normalWorld = {0, -1, 0};
            zoneFace.planeOffset = .1;
            RoomSurfaceContour contour;
            for (const auto &p :
                 {API_Coord3D{0, .1, 10}, API_Coord3D{4, .1, 10}, API_Coord3D{4, .1, 13}, API_Coord3D{0, .1, 13}}) {
                RoomSurfaceVertex v;
                v.world = p;
                contour.vertices.Push (v);
            }
            zoneFace.contours.Push (contour);
            RoomSurfaceSnapshot zoneSnapshot;
            zoneSnapshot.guid = APIGuidFromString ("00000000-0000-0000-0000-000000000274");
            zoneSnapshot.status = RoomSurfaceStatus::Extracted;
            RoomSurfaceBody zoneBody;
            zoneBody.surfaces.Push (zoneFace);
            zoneSnapshot.bodies.Push (zoneBody);
            const auto partition = RoomSurfaces_Partition (zoneSnapshot, {prepared});
            double matchedArea = 0, unmatchedArea = 0;
            for (const auto &piece : partition.pieces) {
                if (piece.status == RoomSurfacePieceStatus::Matched)
                    matchedArea += piece.area;
                if (piece.status == RoomSurfacePieceStatus::Unmatched)
                    unmatchedArea += piece.area;
            }
            check (partition.complete && std::abs (matchedArea - 10) < 1e-6 && std::abs (unmatchedArea - 2) < 1e-6,
                   "native zone partitions against finite prepared wall minus opening");
            GS::Array<RoomSurfacePiece> grouped;
            GS::Array<OtdWall> finishes;
            bool adapted = SurfaceWallPieces_Group (partition.pieces, 0, grouped);
            for (const auto &piece : grouped)
                if (piece.status == RoomSurfacePieceStatus::Matched)
                    adapted = SurfaceWalls_Build (piece, 0, finishes) && adapted;
            bool realOpening = false;
            for (auto &finish : finishes) {
                adapted = SurfaceOpenings_Link (finish, bases) && adapted;
                for (const auto &op : finish.openings)
                    realOpening = realOpening || op.base_guid == opening.base_guid;
            }
            check (adapted && realOpening, "prepared wall reaches writer adapter with real opening ID");

            const auto checkCut = [&] (const GS::Array<OtdOpening> &cuts, double expected, const char *name) {
                GS::Array<OtdWall> cutBases;
                RoomSurfaceSource cutSource;
                bool ok = OtdWall_BuildPreparedSource (element, 10, cuts, cutBases, cutSource);
                const auto cutPartition = RoomSurfaces_Partition (zoneSnapshot, {cutSource});
                double area = 0;
                for (const auto &piece : cutPartition.pieces)
                    if (piece.status == RoomSurfacePieceStatus::Matched)
                        area += piece.area;
                check (ok && cutPartition.complete && std::abs (area - expected) < 1e-6, name);
            };
            OtdOpening boundary = opening;
            boundary.lower = 0;
            boundary.height = 2;
            checkCut ({boundary}, 8, "production boundary opening");
            OtdOpening completeCut = opening;
            completeCut.width = 4;
            completeCut.height = 3;
            completeCut.lower = 0;
            checkCut ({completeCut}, 0, "production full opening cut");
            OtdOpening overlap = opening;
            overlap.base_guid = APIGuidFromString ("00000000-0000-0000-0000-000000000276");
            overlap.objLoc = 3;
            overlap.lower = .5;
            overlap.height = 2;
            checkCut ({opening, overlap}, 7, "production overlapping opening union");
            OtdWall overlapCut;
            overlapCut.surface_geometry = true;
            overlapCut.base_guid = element.header.guid;
            overlapCut.begC = {0, .1};
            overlapCut.endC = {4, .1};
            overlapCut.height = 3;
            overlapCut.zBottom = 10;
            OtdOpening combined;
            combined.objLoc = 2.5;
            combined.width = 3;
            combined.zBottom = 11;
            combined.height = 1;
            overlapCut.openings.Push (combined);
            GS::Array<OtdWall> overlapBases;
            RoomSurfaceSource overlapSource;
            check (OtdWall_BuildPreparedSource (element, 10, {opening, overlap}, overlapBases, overlapSource) &&
                       SurfaceOpenings_Link (overlapCut, overlapBases, true),
                   "overlap links without choosing fake owner");
            bool bothIds = false;
            for (const auto &cut : overlapCut.openings)
                bothIds =
                    bothIds || (cut.base_guid == APINULLGuid && cut.source_opening_guids.Contains (opening.base_guid) &&
                                cut.source_opening_guids.Contains (overlap.base_guid));
            check (bothIds, "overlap cell retains both real opening IDs");
            std::swap (element.wall.begC, element.wall.endC);
            bases.Clear ();
            check (OtdWall_BuildPreparedSource (element, 10, {}, bases, prepared) &&
                       std::abs (bases[0].begC.y + .1) < 1e-12,
                   "production reversed axis side");
            std::swap (element.wall.begC, element.wall.endC);
            element.wall.flipped = true;
            bases.Clear ();
            check (OtdWall_BuildPreparedSource (element, 10, {opening}, bases, prepared) &&
                       std::abs (bases[0].begC.y + .1) < 1e-12,
                   "production flipped wall side");
            element.wall.relativeTopStory = 1;
            bases.Clear ();
            check (OtdWall_BuildPreparedSource (element, 10, {opening}, bases, prepared) && bases.GetSize () == 4 &&
                       std::abs (bases[0].height - element.wall.height) < 1e-12,
                   "top-linked wall retains resolved API height without native fallback");
            element.wall.relativeTopStory = 0;
            element.wall.modelElemStructureType = API_ProfileStructure;
            bases.Clear ();
            check (!OtdWall_BuildPreparedSource (element, 10, {}, bases, prepared) && bases.IsEmpty () &&
                       !prepared.snapshot.IsComplete (),
                   "profile explicitly requests cached native fallback");

            element = {};
            SetElemTypeID (element, API_SlabID);
            element.header.guid = APIGuidFromString ("00000000-0000-0000-0000-000000000275");
            element.slab.thickness = .3;
            element.slab.offsetFromTop = .2;
            const GS::Array<Point2D> coords = {{0, 0}, {4, 0}, {4, 3}, {0, 3}, {1, 1}, {1, 2}, {2, 2}, {2, 1}};
            GS::Array<PolyArcRec> arcs;
            const GS::Array<UIndex> ends = {3, 7};
            const auto polygons =
                Geometry::Polygon2D::Create (coords, arcs, ends, Geometry::PolyCreateFlags_IdxRefInContourEndFromZero);
            GS::Array<OtdSlab> slabBases;
            check (polygons.GetSize () == 1 &&
                       OtdSlab_BuildPreparedSource (element, 10, polygons[0], slabBases, prepared) &&
                       slabBases.GetSize () == 1 && std::abs (slabBases[0].zBottom - 9.9) < 1e-12 &&
                       std::abs (slabBases[0].reference_z - 10) < 1e-12 &&
                       std::abs (slabBases[0].poly.CalcArea () - 11) < 1e-6,
                   "production slab reference levels and original hole");
            bool hole = false;
            for (const auto &face : prepared.snapshot.bodies[0].surfaces)
                hole = hole || (face.normalWorld.z != 0 && face.contours.GetSize () == 2 && face.contours[1].hole);
            check (hole, "slab hole survives analytic contact surfaces");

            RoomSurface floorFace;
            floorFace.bodyIndex = floorFace.polygonIndex = 1;
            floorFace.normalWorld = {0, 0, -1};
            floorFace.planeOffset = 10;
            RoomSurfaceContour floorContour;
            for (const auto &point :
                 {API_Coord3D{-1, -1, 10}, API_Coord3D{5, -1, 10}, API_Coord3D{5, 4, 10}, API_Coord3D{-1, 4, 10}}) {
                RoomSurfaceVertex vertex;
                vertex.world = point;
                floorContour.vertices.Push (vertex);
            }
            floorFace.contours.Push (floorContour);
            RoomSurfaceSnapshot floorSnapshot;
            floorSnapshot.guid = zoneSnapshot.guid;
            floorSnapshot.status = RoomSurfaceStatus::Extracted;
            RoomSurfaceBody floorBody;
            floorBody.surfaces.Push (floorFace);
            floorSnapshot.bodies.Push (floorBody);
            OtdSlab nominal;
            nominal.zBottom = 10;
            nominal.material.smaterial = "material";
            GS::Array<OtdSlab> projectedBases;
            double rejected = 0;
            check (OtdSlab_BuildProjectedFinishes (floorSnapshot,
                                                   {prepared},
                                                   {element.header.guid},
                                                   Floor,
                                                   0,
                                                   projectedBases,
                                                   rejected,
                                                   &nominal,
                                                   true,
                                                   nullptr,
                                                   &slabBases) &&
                       projectedBases.GetSize () == 1 && std::abs (projectedBases[0].poly.CalcArea () - 11) < 1e-6 &&
                       std::abs (projectedBases[0].zBottom - 10.2) < 1e-12 &&
                       projectedBases[0].material.smaterial == nominal.material.smaterial,
                   "active prepared slab route preserves hole elevation material and by_slab");
            // Два полностью перекрывающихся основания: приоритет должен зависеть от отметки, не порядка GUID.
            const auto full = Geometry::Polygon2D::Create (GS::Array<Point2D>{{-1, -1}, {5, -1}, {5, 4}, {-1, 4}}, 0);
            if (!full.IsEmpty ()) {
                for (const auto type : {Floor, Ceil}) {
                    GS::Array<OtdSlab> bases;
                    GS::Array<RoomSurfaceSource> sources;
                    GS::Array<API_Guid> roles;
                    for (int i = 0; i < 2; ++i) {
                        API_Element candidate = element;
                        candidate.header.guid = APIGuidFromString (i == 0 ? "00000000-0000-0000-0000-000000000276"
                                                                          : "00000000-0000-0000-0000-000000000277");
                        candidate.slab.offsetFromTop = 0;
                        candidate.slab.thickness = type == Floor ? .5545 : .3;
                        RoomSurfaceSource source;
                        const double top = type == Floor ? (i == 0 ? 0 : -.5545) : (i == 0 ? 3.6 : 7.2);
                        check (OtdSlab_BuildPreparedSource (candidate, top, full[0], bases, source),
                               "ordering regression prepared candidate");
                        sources.Push (source);
                        roles.Push (candidate.header.guid);
                    }
                    auto footprint = floorSnapshot;
                    footprint.bodies[0].surfaces[0].normalWorld.z = type == Floor ? -1 : 1;
                    for (bool reverse : {false, true}) {
                        const GS::Array<API_Guid> ordered = reverse ? GS::Array<API_Guid>{roles[1], roles[0]} : roles;
                        GS::Array<OtdSlab> finishes;
                        double rejectedArea = 0;
                        check (OtdSlab_BuildProjectedFinishes (footprint,
                                                               sources,
                                                               ordered,
                                                               type,
                                                               0,
                                                               finishes,
                                                               rejectedArea,
                                                               &nominal,
                                                               true,
                                                               nullptr,
                                                               &bases) &&
                                   finishes.GetSize () == 1 && finishes[0].base_guid == roles[0] &&
                                   std::abs (finishes[0].poly.CalcArea () - 30) < 1e-6 &&
                                   std::abs (finishes[0].zBottom - (type == Floor ? 0 : 3.3)) < 1e-6,
                               type == Floor ? "highest floor wins in both candidate orders"
                                             : "nearest ceiling above floor wins independent of zone top and order");
                    }
                }
            }
            GS::Array<Point2D> curvedCoords = {{0, 0}, {4, 0}, {4, 3}, {0, 3}};
            PolyArcRec arc;
            arc.begIndex = 0;
            arc.endIndex = 1;
            arc.arcAngle = .4;
            arcs.Push (arc);
            const auto curved = Geometry::Polygon2D::Create (curvedCoords,
                                                             arcs,
                                                             GS::Array<UIndex>{3},
                                                             Geometry::PolyCreateFlags_IdxRefInContourEndFromZero |
                                                                 Geometry::PolyCreateFlags_IdxRefInArcFromZero);
            slabBases.Clear ();
            check (curved.GetSize () == 1 && OtdSlab_BuildPreparedSource (element, 10, curved[0], slabBases, prepared),
                   "curved slab analytic preparation");
            if (!slabBases.IsEmpty ()) {
                GS::Array<Point2D> copied;
                GS::Array<PolyArcRec> copiedArcs;
                GS::Array<UIndex> copiedEnds;
                GS::Array<Geometry::Polygon2D::ConstContourIterator> reversed;
                slabBases[0].poly.CopyPolygonData (
                    copied, copiedArcs, copiedEnds, reversed, Geometry::PolyCreateFlags_IdxRefInContourEndFromZero);
                check (!copiedArcs.IsEmpty (), "immutable slab base retains arcs for projection and writer");
            }
        }
        const API_Guid zoneGuid = APIGuidFromString ("00000000-0000-0000-0000-000000000001");
        const API_Guid baseA = APIGuidFromString ("00000000-0000-0000-0000-000000000002");
        const API_Guid baseB = APIGuidFromString ("00000000-0000-0000-0000-000000000003");
        const API_Guid segment = APIGuidFromString ("00000000-0000-0000-0000-000000000004");
        const auto rectangle = [] (double x0, double x1, double y0, double y1, double normal) {
            RoomSurface surface;
            surface.bodyIndex = 1;
            surface.polygonIndex = 1;
            surface.normalLocal = {0, 0, normal};
            surface.normalWorld = surface.normalLocal;
            RoomSurfaceContour contour;
            for (const API_Coord3D &p :
                 {API_Coord3D{x0, y0, 0}, API_Coord3D{x1, y0, 0}, API_Coord3D{x1, y1, 0}, API_Coord3D{x0, y1, 0}}) {
                RoomSurfaceVertex vertex;
                vertex.local = p;
                vertex.world = p;
                contour.vertices.Push (vertex);
            }
            surface.contours.Push (contour);
            return surface;
        };
        const auto snapshot = [] (API_Guid guid, const RoomSurface &surface) {
            RoomSurfaceSnapshot result;
            result.guid = guid;
            result.status = RoomSurfaceStatus::Extracted;
            RoomSurfaceBody body;
            body.index = 1;
            body.surfaces.Push (surface);
            result.bodies.Push (body);
            return result;
        };
        const auto source = [&] (API_Guid guid, const RoomSurface &surface, API_ElemTypeID type = API_WallID) {
            RoomSurfaceSource result;
            result.baseGuid = guid;
            result.baseType = type;
            result.geometryGuid = guid;
            result.snapshot = snapshot (guid, surface);
            return result;
        };
        const auto area = [] (const RoomSurfacePartition &partition, RoomSurfacePieceStatus status) {
            double result = 0;
            for (const auto &piece : partition.pieces)
                if (piece.status == status)
                    result += piece.area;
            return result;
        };
        const auto closeArea = [] (double a, double b) { return std::fabs (a - b) <= 1e-6; };
        const auto zone = snapshot (zoneGuid, rectangle (0, 4, 0, 3, 1));
        GS::Array<RoomSurfaceSource> sources;
        auto result = RoomSurfaces_Partition (zone, sources);
        check (result.complete && result.pieces.GetSize () == 1 &&
                   closeArea (area (result, RoomSurfacePieceStatus::Unmatched), 12),
               "no candidates preserve the whole face");
        sources.Push (source (baseA, rectangle (0, 2, 0, 3, -1)));
        sources.Push (source (baseB, rectangle (2, 4, 0, 3, -1), API_BeamID));
        result = RoomSurfaces_Partition (zone, sources);
        check (result.complete && result.pieces.GetSize () == 2 &&
                   closeArea (area (result, RoomSurfacePieceStatus::Matched), 12) &&
                   closeArea (area (result, RoomSurfacePieceStatus::Ambiguous), 0),
               "adjacent wall and beam split the face");
        bool beamFound = false;
        for (const auto &piece : result.pieces)
            beamFound =
                beamFound || (piece.baseGuid == baseB && piece.baseType == API_BeamID && closeArea (piece.area, 6));
        check (beamFound, "beam ownership survives partition");
        GS::Array<RoomSurfaceSource> reversed;
        reversed.Push (sources[1]);
        reversed.Push (sources[0]);
        const auto reordered = RoomSurfaces_Partition (zone, reversed);
        bool same = reordered.complete && result.pieces.GetSize () == reordered.pieces.GetSize ();
        for (UIndex i = 0; same && i < result.pieces.GetSize (); ++i)
            same = result.pieces[i].baseGuid == reordered.pieces[i].baseGuid &&
                   closeArea (result.pieces[i].area, reordered.pieces[i].area);
        check (same, "candidate order does not change ownership or area");
        sources[0] = source (baseA, rectangle (0, 3, 0, 3, -1));
        sources[1] = source (baseB, rectangle (1, 4, 0, 3, -1));
        result = RoomSurfaces_Partition (zone, sources);
        check (result.complete && closeArea (area (result, RoomSurfacePieceStatus::Ambiguous), 6) &&
                   closeArea (area (result, RoomSurfacePieceStatus::Matched), 6),
               "overlapping bases remain explicitly ambiguous");
        bool noChosenOwner = true;
        for (const auto &piece : result.pieces)
            if (piece.status == RoomSurfacePieceStatus::Ambiguous)
                noChosenOwner = noChosenOwner && piece.baseGuid == APINULLGuid && piece.sources.GetSize () == 2;
        check (noChosenOwner, "ambiguous contact never chooses the first base");
        sources.Clear ();
        sources.Push (source (baseA, rectangle (0, 1, 0, 3, -1)));
        result = RoomSurfaces_Partition (zone, sources);
        check (result.complete && closeArea (area (result, RoomSurfacePieceStatus::Matched), 3) &&
                   closeArea (area (result, RoomSurfacePieceStatus::Unmatched), 9),
               "partial contact retains unmatched remainder");
        sources[0] = source (baseA, rectangle (6, 7, 0, 3, -1));
        result = RoomSurfaces_Partition (zone, sources);
        check (result.complete && closeArea (area (result, RoomSurfacePieceStatus::Unmatched), 12),
               "coplanar distant face is not a contact");
        sources[0] = source (baseA, rectangle (4, 5, 0, 3, -1));
        result = RoomSurfaces_Partition (zone, sources);
        check (result.complete && closeArea (area (result, RoomSurfacePieceStatus::Unmatched), 12),
               "edge-only contact has no area");
        sources[0] = source (baseA, rectangle (0, 4, 0, 3, 1));
        result = RoomSurfaces_Partition (zone, sources);
        check (result.complete && closeArea (area (result, RoomSurfacePieceStatus::Matched), 12),
               "coplanar same-normal face inside the room covers the finish");
        auto distant = rectangle (0, 4, 0, 3, -1);
        for (auto &vertex : distant.contours[0].vertices) {
            vertex.world.z = 2e-4;
            vertex.local.z = 2e-4;
        }
        distant.planeOffset = 2e-4;
        sources[0] = source (baseA, distant);
        result = RoomSurfaces_Partition (zone, sources);
        check (result.complete && closeArea (area (result, RoomSurfacePieceStatus::Unmatched), 12),
               "plane distance beyond tolerance is not accepted");
        sources[0] = source (baseA, rectangle (0, 4, 0, 3, -1));
        sources.Push (sources[0]);
        result = RoomSurfaces_Partition (zone, sources);
        check (result.complete && result.pieces.GetSize () == 1 && result.pieces[0].sources.GetSize () == 1 &&
                   closeArea (area (result, RoomSurfacePieceStatus::Matched), 12),
               "duplicate source does not duplicate its claim");
        sources.Clear ();
        sources.Push (source (baseA, rectangle (0, 2, 0, 3, -1), API_ColumnID));
        auto second = source (baseA, rectangle (2, 4, 0, 3, -1), API_ColumnID);
        second.geometryGuid = segment;
        second.snapshot.guid = segment;
        sources.Push (second);
        result = RoomSurfaces_Partition (zone, sources);
        bool segmentsRetained = false;
        for (const auto &piece : result.pieces)
            for (const auto &face : piece.sources)
                segmentsRetained = segmentsRetained || face.geometryGuid == segment;
        check (result.complete && result.pieces.GetSize () == 2 &&
                   closeArea (area (result, RoomSurfacePieceStatus::Matched), 12) && segmentsRetained,
               "segments retain parent ownership and geometry identity");
        auto holedFace = rectangle (0, 4, 0, 4, 1);
        auto hole = rectangle (1, 3, 1, 3, 1).contours[0];
        hole.hole = true;
        holedFace.contours.Push (hole);
        sources.Clear ();
        sources.Push (source (baseA, rectangle (0, 4, 0, 4, -1), API_SlabID));
        result = RoomSurfaces_Partition (snapshot (zoneGuid, holedFace), sources);
        check (result.complete && result.pieces.GetSize () == 1 && result.pieces[0].contours.GetSize () == 2 &&
                   result.pieces[0].contours[1].hole && closeArea (area (result, RoomSurfacePieceStatus::Matched), 12),
               "zone hole is preserved in matched contour");
        auto holedSource = holedFace;
        holedSource.normalLocal = {0, 0, -1};
        holedSource.normalWorld = holedSource.normalLocal;
        sources[0] = source (baseA, holedSource);
        result = RoomSurfaces_Partition (snapshot (zoneGuid, rectangle (0, 4, 0, 4, 1)), sources);
        check (result.complete && closeArea (area (result, RoomSurfacePieceStatus::Matched), 12) &&
                   closeArea (area (result, RoomSurfacePieceStatus::Unmatched), 4),
               "source hole stays unmatched");
        auto triangular = rectangle (0, 4, 0, 3, -1);
        triangular.contours[0].vertices.Delete (2);
        sources[0] = source (baseA, triangular);
        result = RoomSurfaces_Partition (zone, sources);
        check (result.complete && closeArea (area (result, RoomSurfacePieceStatus::Matched), 6) &&
                   closeArea (area (result, RoomSurfacePieceStatus::Unmatched), 6),
               "finite triangular contact is clipped, not its bounds");
        auto collinear = rectangle (0, 4, 0, 3, -1);
        RoomSurfaceVertex mid;
        mid.local = {2, 0, 0};
        mid.world = mid.local;
        collinear.contours[0].vertices.Insert (1, mid);
        sources[0] = source (baseA, collinear);
        result = RoomSurfaces_Partition (zone, sources);
        check (result.complete && closeArea (area (result, RoomSurfacePieceStatus::Matched), 12),
               "collinear extra vertex does not invalidate contact");
        const API_Tranmat shear = {{1, 0, 0, 10, 0, 1, 0, -3, 1, 0, 1, 7}};
        auto slantedZone = rectangle (0, 4, 0, 3, 1), slantedSource = rectangle (0, 4, 0, 3, -1);
        check (RoomSurface_Transform (shear, slantedZone) && RoomSurface_Transform (shear, slantedSource),
               "slanted fixture transformed by the shared reader helper");
        sources[0] = source (baseA, slantedSource);
        result = RoomSurfaces_Partition (snapshot (zoneGuid, slantedZone), sources);
        check (result.complete && closeArea (area (result, RoomSurfacePieceStatus::Matched), 12 * std::sqrt (2.0)),
               "inclined translated plane is matched in world space");
        // AC25: stored-нормаль SDK-грани может отклоняться от фактической плоскости вершин (~0.18°);
        // планарная грань тогда не проходит копланарность и даёт PartitionInvalidSourcePlane.
        // Newell-нормаль из вершин такую грань принимает.
        auto skewed = rectangle (0, 4, 0, 3, -1);
        const double tilt = 1.0e-4; // На дальнем углу отклонение больше допуска плоскости.
        const double skewLength = std::sqrt (1.0 + tilt * tilt);
        skewed.normalWorld = {tilt / skewLength, 0, -1.0 / skewLength};
        skewed.normalLocal = skewed.normalWorld;
        sources[0] = source (baseA, skewed);
        result = RoomSurfaces_Partition (zone, sources);
        check (result.complete && result.pieces.GetSize () == 1 &&
                   closeArea (area (result, RoomSurfacePieceStatus::Matched), 12),
               "skewed stored normal of a planar face is accepted");
        auto nearPlane = rectangle (0, 4, 0, 3, -1);
        nearPlane.contours[0].vertices[2].world.z = 2.0e-5;
        nearPlane.contours[0].vertices[2].local.z = 2.0e-5;
        sources[0] = source (baseA, nearPlane);
        result = RoomSurfaces_Partition (zone, sources);
        check (result.complete && result.errors.IsEmpty () &&
                   closeArea (area (result, RoomSurfacePieceStatus::Matched), 12),
               "twenty micron source-plane noise is accepted within 0.1 mm");
        // По-настоящему неплоская грань остаётся ошибкой: подъём вершины за допуск не принимается.
        auto warped = rectangle (0, 4, 0, 3, -1);
        warped.contours[0].vertices[2].world.z = 1.0e-3;
        warped.contours[0].vertices[2].local.z = 1.0e-3;
        sources[0] = source (baseA, warped);
        result = RoomSurfaces_Partition (zone, sources);
        bool invalidPlane = false;
        for (const auto &error : result.errors)
            invalidPlane = invalidPlane || error.stage == GS::UniString ("PartitionInvalidSourcePlane");
        check (!result.complete && invalidPlane, "warped face still fails the source-plane check");
        // Малая плоская грань основания не должна останавливать разбиение всей зоны.
        auto tiny = rectangle (20.740000000001132, 20.740500000001131, 2.6792999999999947, 2.6799999999999962, -1);
        for (auto &vertex : tiny.contours[0].vertices) {
            vertex.world = {vertex.world.x, 15.519300000000007, vertex.world.y};
            vertex.local = vertex.world;
        }
        tiny.normalWorld = {0, -1, 0};
        tiny.normalLocal = tiny.normalWorld;
        tiny.planeOffset = 15.519300000000007;
        sources[0] = source (baseA, tiny);
        result = RoomSurfaces_Partition (zone, sources);
        check (result.complete && result.errors.IsEmpty () &&
                   closeArea (area (result, RoomSurfacePieceStatus::Unmatched), 12),
               "tiny planar source does not abort unrelated zone partition");
        sources[0].snapshot.status = RoomSurfaceStatus::ReadError;
        result = RoomSurfaces_Partition (zone, sources);
        check (!result.complete && !result.errors.IsEmpty (), "incomplete source cannot produce a complete partition");
        auto badZone = zone;
        badZone.status = RoomSurfaceStatus::ReadError;
        result = RoomSurfaces_Partition (badZone, sources);
        check (!result.complete && result.pieces.IsEmpty () && !result.errors.IsEmpty (), "incomplete zone rejected");
        sources.Clear ();
        sources.Push (source (baseA, rectangle (0, 4, 0, 3, -1)));
        auto conflicting = sources[0];
        conflicting.snapshot.bodies[0].surfaces[0].contours[0].vertices[0].world.x = 0.25;
        sources.Push (conflicting);
        result = RoomSurfaces_Partition (zone, sources);
        check (!result.complete && !result.errors.IsEmpty (), "conflicting snapshots with the same face key rejected");
        sources.Clear ();
        sources.Push (source (baseA, rectangle (0, 4, 0, 3, -1)));
        sources[0].snapshot.bodies[0].surfaces[0].normalWorld = {};
        result = RoomSurfaces_Partition (zone, sources);
        check (!result.complete && !result.errors.IsEmpty (), "invalid source normal cannot silently become unmatched");
        sources[0] = source (baseA, rectangle (0, 4, 0, 3, -1));
        badZone = zone;
        badZone.bodies[0].surfaces[0].planeOffset = 1;
        result = RoomSurfaces_Partition (badZone, sources);
        check (!result.complete && !result.errors.IsEmpty (),
               "zone plane equation inconsistent with its vertices rejected");
        sources[0].snapshot.bodies[0].surfaces[0].contours[0].vertices[0].world.x =
            std::numeric_limits<double>::quiet_NaN ();
        result = RoomSurfaces_Partition (zone, sources);
        check (!result.complete && !result.errors.IsEmpty (), "nonfinite source vertex rejected");
        // Отделка балки: низ и бока на участке beamPart, обрезка по t и по вертикали комнаты.
        const auto beamFace = [] (double nx, double ny, double nz, const API_Coord3D *points, Int32 count) {
            RoomSurface surface;
            surface.normalWorld = {nx, ny, nz};
            RoomSurfaceContour contour;
            for (Int32 i = 0; i < count; ++i) {
                RoomSurfaceVertex vertex;
                vertex.world = points[i];
                contour.vertices.Push (vertex);
            }
            surface.contours.Push (contour);
            return surface;
        };
        RoomSurfaceSource beamSource;
        beamSource.baseGuid = baseA;
        beamSource.baseType = API_BeamID;
        beamSource.geometryGuid = segment;
        beamSource.snapshot = snapshot (segment, {});
        beamSource.snapshot.status = RoomSurfaceStatus::Extracted;
        {
            RoomSurfaceBody &body = beamSource.snapshot.bodies[0];
            const double zb = 2.0, zt = 2.62, w = 0.15;
            const API_Coord3D bottom[4] = {{0, -w, zb}, {10, -w, zb}, {10, w, zb}, {0, w, zb}};
            const API_Coord3D left[4] = {{0, -w, zb}, {10, -w, zb}, {10, -w, zt}, {0, -w, zt}};
            const API_Coord3D right[4] = {{10, w, zb}, {0, w, zb}, {0, w, zt}, {10, w, zt}};
            const API_Coord3D top[4] = {{0, w, zt}, {10, w, zt}, {10, -w, zt}, {0, -w, zt}};
            const API_Coord3D begEnd[4] = {{0, -w, zb}, {0, -w, zt}, {0, w, zt}, {0, w, zb}};
            body.surfaces.Push (beamFace (0, 0, -1, bottom, 4));
            body.surfaces.Push (beamFace (0, -1, 0, left, 4));
            body.surfaces.Push (beamFace (0, 1, 0, right, 4));
            body.surfaces.Push (beamFace (0, 0, 1, top, 4));
            body.surfaces.Push (beamFace (-1, 0, 0, begEnd, 4));
        }
        const API_Coord beamBeg{0, 0}, beamEnd{10, 0};
        RoomSurfaceBeamPartitions beamResult;
        const auto beamArea = [&] (const RoomSurfaceBeamFinish &finish, RoomSurfaceBeamFace face) {
            double sum = 0;
            for (const auto &piece : finish.pieces)
                if (piece.face == face)
                    sum += piece.area;
            return sum;
        };
        RoomSurfaceSnapshot steppedZone;
        steppedZone.status = RoomSurfaceStatus::Extracted;
        RoomSurfaceBody steppedBody;
        const API_Coord profile[] = {{0, 0}, {8, 0}, {8, 0.5}, {9, 0.5}, {9, 2.5}, {9.5, 2.5}, {9.5, 3}, {0, 3}};
        for (UIndex i = 0; i < 8; ++i) {
            const auto &a = profile[i];
            const auto &b = profile[(i + 1) % 8];
            const double length = std::hypot (b.x - a.x, b.y - a.y);
            const API_Coord3D vertices[] = {{a.x, -1, a.y}, {b.x, -1, b.y}, {b.x, 1, b.y}, {a.x, 1, a.y}};
            auto face = beamFace ((b.y - a.y) / length, 0, -(b.x - a.x) / length, vertices, 4);
            face.planeOffset = -(face.normalWorld.x * vertices[0].x + face.normalWorld.z * vertices[0].z);
            steppedBody.surfaces.Push (face);
        }
        for (double y : {-1.0, 1.0}) {
            API_Coord3D vertices[8];
            for (UIndex i = 0; i < 8; ++i)
                vertices[i] = {profile[i].x, y, profile[i].y};
            auto face = beamFace (0, y, 0, vertices, 8);
            face.planeOffset = -1;
            steppedBody.surfaces.Push (face);
        }
        steppedZone.bodies.Push (steppedBody);
        auto sectionSource = beamSource;
        for (auto &face : sectionSource.snapshot.bodies[0].surfaces) {
            if (face.contours.IsEmpty () || face.contours[0].vertices.IsEmpty ())
                continue;
            const auto &p = face.contours[0].vertices[0].world;
            face.planeOffset = -(face.normalWorld.x * p.x + face.normalWorld.y * p.y + face.normalWorld.z * p.z);
        }
        RoomSurfaceBeamPartitions sectionResult;
        const auto sectionFinish =
            RoomBeamFinish_Build (sectionSource, beamBeg, beamEnd, 0, 80, 0, 3, sectionResult, &steppedZone);
        check (sectionResult.errors.IsEmpty () &&
                   closeArea (beamArea (sectionFinish, RoomSurfaceBeamFace::Bottom), 9 * 0.3) &&
                   closeArea (beamArea (sectionFinish, RoomSurfaceBeamFace::LeftSide), 9 * 0.5 + 9.5 * 0.12) &&
                   closeArea (beamArea (sectionFinish, RoomSurfaceBeamFace::RightSide), 9 * 0.5 + 9.5 * 0.12) &&
                   closeArea (beamArea (sectionFinish, RoomSurfaceBeamFace::Top), 9.5 * 0.3),
               "3D zone section extends beam bottom, top and both sides beyond lower-profile beamPart");
        steppedZone.bodies[0].surfaces.Delete (0);
        sectionResult.errors.Clear ();
        const auto brokenSection =
            RoomBeamFinish_Build (sectionSource, beamBeg, beamEnd, 0, 80, 0, 3, sectionResult, &steppedZone);
        check (brokenSection.pieces.IsEmpty () && !sectionResult.errors.IsEmpty (),
               "open zone section is rejected without partial beam finish");
        auto beam = RoomBeamFinish_Build (beamSource, beamBeg, beamEnd, 0, 100, 0, 3, beamResult);
        check (beam.pieces.GetSize () == 4 && closeArea (beamArea (beam, RoomSurfaceBeamFace::Bottom), 10 * 0.3) &&
                   closeArea (beamArea (beam, RoomSurfaceBeamFace::Top), 10 * 0.3) &&
                   closeArea (beamArea (beam, RoomSurfaceBeamFace::LeftSide), 10 * 0.62) &&
                   closeArea (beamArea (beam, RoomSurfaceBeamFace::RightSide), 10 * 0.62),
               "full beam part yields bottom, top and both sides");
        beam = RoomBeamFinish_Build (beamSource, beamBeg, beamEnd, 25, 75, 0, 3, beamResult);
        check (beam.pieces.GetSize () == 4 && closeArea (beamArea (beam, RoomSurfaceBeamFace::Bottom), 5 * 0.3) &&
                   closeArea (beamArea (beam, RoomSurfaceBeamFace::Top), 5 * 0.3) &&
                   closeArea (beamArea (beam, RoomSurfaceBeamFace::LeftSide), 5 * 0.62),
               "tBeg/tEnd clip bottom, top and sides");
        beam = RoomBeamFinish_Build (beamSource, beamBeg, beamEnd, 0, 100, 0, 2.31, beamResult);
        check (beam.pieces.GetSize () == 3 && closeArea (beamArea (beam, RoomSurfaceBeamFace::LeftSide), 10 * 0.31) &&
                   closeArea (beamArea (beam, RoomSurfaceBeamFace::Bottom), 3),
               "zTop clip keeps the bottom intact");
        beam = RoomBeamFinish_Build (beamSource, beamBeg, beamEnd, 0, 100, 2.31, 3, beamResult);
        check (beam.pieces.GetSize () == 3 && closeArea (beamArea (beam, RoomSurfaceBeamFace::LeftSide), 10 * 0.31) &&
                   closeArea (beamArea (beam, RoomSurfaceBeamFace::Top), 10 * 0.3),
               "zBottom above the beam drops its bottom face");
        beam = RoomBeamFinish_Build (beamSource, beamBeg, beamEnd, 50, 50, 0, 3, beamResult);
        check (beam.pieces.IsEmpty () && !beamResult.errors.IsEmpty (), "empty t-range is an explicit error");
        beam = RoomBeamFinish_Build (beamSource, beamBeg, beamEnd, 25, 75, 0, 3, beamResult);
        bool validVertices = true;
        for (const auto &piece : beam.pieces) {
            if (piece.vertices.GetSize () != 4) {
                validVertices = false;
                continue;
            }
            const auto &a = piece.vertices[0];
            const auto &b = piece.vertices[1];
            const auto &c = piece.vertices[3];
            const API_Coord3D u{b.x - a.x, b.y - a.y, b.z - a.z};
            const API_Coord3D v{c.x - a.x, c.y - a.y, c.z - a.z};
            const API_Coord3D cross{u.y * v.z - u.z * v.y, u.z * v.x - u.x * v.z, u.x * v.y - u.y * v.x};
            const double area = std::sqrt (cross.x * cross.x + cross.y * cross.y + cross.z * cross.z);
            validVertices = validVertices && area > 1e-6 && closeArea (area, piece.area);
            for (const auto &vertex : piece.vertices)
                validVertices = validVertices && vertex.x >= 2.5 - 1e-6 && vertex.x <= 7.5 + 1e-6;
        }
        check (validVertices && beam.pieces.GetSize () == 4,
               "beam vertices have the reported nonzero area and t bounds");
        check (closeArea (beam.tBeg, 25) && closeArea (beam.tEnd, 75), "beam result preserves raw SDK part bounds");
        for (auto &surface : beamSource.snapshot.bodies[0].surfaces)
            for (auto &contour : surface.contours)
                for (auto &vertex : contour.vertices)
                    vertex.world.x = 2 + vertex.world.x * 0.6;
        beam = RoomBeamFinish_Build (beamSource, beamBeg, beamEnd, 0, 100, 0, 3, beamResult);
        check (beam.pieces.GetSize () == 4 && closeArea (beamArea (beam, RoomSurfaceBeamFace::Bottom), 6 * 0.3) &&
                   closeArea (beamArea (beam, RoomSurfaceBeamFace::Top), 6 * 0.3) &&
                   closeArea (beamArea (beam, RoomSurfaceBeamFace::LeftSide), 6 * 0.62),
               "segment face bounds clip bottom, top and sides within the parent beam");
        beamSource.snapshot.bodies[0].surfaces.Clear ();
        // Снимок второй реальной балки AC25 сохранён до перезапуска без записи PLN.
        const API_Coord3D observedBeamFace0[4] = {{8.843547364, -3.715530072, 3.856828094},
                                                  {8.843547364, 13.316251845, 3.856828094},
                                                  {8.543543538, 13.316251845, 3.856828094},
                                                  {8.543543538, -3.715530072, 3.856828094}};
        beamSource.snapshot.bodies[0].surfaces.Push (beamFace (0, 0, 1, observedBeamFace0, 4));
        const API_Coord3D observedBeamFace1[4] = {{8.843547364, -3.715530072, 3.038523477},
                                                  {8.843547364, 13.316251845, 3.038523477},
                                                  {8.843547364, 13.316251845, 3.856828094},
                                                  {8.843547364, -3.715530072, 3.856828094}};
        beamSource.snapshot.bodies[0].surfaces.Push (beamFace (1, 0, 0, observedBeamFace1, 4));
        const API_Coord3D observedBeamFace2[4] = {{8.543543538, 13.316251845, 3.856828094},
                                                  {8.843547364, 13.316251845, 3.856828094},
                                                  {8.843547364, 13.316251845, 3.038523477},
                                                  {8.543543538, 13.316251845, 3.038523477}};
        beamSource.snapshot.bodies[0].surfaces.Push (beamFace (0, 1, 0, observedBeamFace2, 4));
        const API_Coord3D observedBeamFace3[4] = {{8.543543538, -3.715530072, 3.856828094},
                                                  {8.543543538, 13.316251845, 3.856828094},
                                                  {8.543543538, 13.316251845, 3.038523477},
                                                  {8.543543538, -3.715530072, 3.038523477}};
        beamSource.snapshot.bodies[0].surfaces.Push (beamFace (-1, 0, 0, observedBeamFace3, 4));
        const API_Coord3D observedBeamFace4[4] = {{8.843547364, -3.715530072, 3.038523477},
                                                  {8.843547364, -3.715530072, 3.856828094},
                                                  {8.543543538, -3.715530072, 3.856828094},
                                                  {8.543543538, -3.715530072, 3.038523477}};
        beamSource.snapshot.bodies[0].surfaces.Push (beamFace (0, -1, 0, observedBeamFace4, 4));
        const API_Coord3D observedBeamFace5[4] = {{8.543543538, -3.715530072, 3.038523477},
                                                  {8.543543538, 13.316251845, 3.038523477},
                                                  {8.843547364, 13.316251845, 3.038523477},
                                                  {8.843547364, -3.715530072, 3.038523477}};
        beamSource.snapshot.bodies[0].surfaces.Push (beamFace (0, 0, -1, observedBeamFace5, 4));
        beam = RoomBeamFinish_Build (beamSource,
                                     {8.6935454508593946, -3.7155300720242539},
                                     {8.6935454508593946, 13.316251844666528},
                                     7.9217305889226557,
                                     281.60809357750338,
                                     0,
                                     3.62,
                                     beamResult);
        const double observedLength = 12.818740678679376 - (-3.250415462896977);
        check (beam.pieces.GetSize () == 3 &&
                   closeArea (beamArea (beam, RoomSurfaceBeamFace::Bottom),
                              observedLength * (8.843547364 - 8.543543538)) &&
                   closeArea (beamArea (beam, RoomSurfaceBeamFace::LeftSide), observedLength * (3.62 - 3.038523477)) &&
                   closeArea (beamArea (beam, RoomSurfaceBeamFace::RightSide), observedLength * (3.62 - 3.038523477)),
               "captured AC25 beam snapshot yields bottom and both sides within zone height");
        bool observedBounds = true;
        for (const auto &piece : beam.pieces)
            for (const auto &vertex : piece.vertices)
                observedBounds = observedBounds && vertex.y >= -3.250415462896977 - 1e-6 &&
                                 vertex.y <= 12.818740678679376 + 1e-6 && vertex.z <= 3.62 + 1e-6;
        check (observedBounds && beam.pieces.GetSize () == 3,
               "raw AC25 beam projections reproduce measured zone boundaries");
        RoomSurfacePiece contact;
        contact.status = RoomSurfacePieceStatus::Matched;
        contact.baseGuid = baseA;
        contact.baseType = API_ColumnID;
        contact.normal = {0, 1, 0};
        contact.area = 12;
        RoomSurfacePieceContour contactContour;
        contactContour.vertices = {{0, 0, 0}, {4, 0, 0}, {4, 0, 3}, {0, 0, 3}};
        contact.contours.Push (contactContour);
        OtdWall surfaceWall;
        check (SurfaceWall_Build (contact, 0, surfaceWall) && surfaceWall.type == Column &&
                   surfaceWall.base_guid == baseA && surfaceWall.surface_geometry &&
                   closeArea (OtdWall_GetArea (surfaceWall), 12),
               "surface adapter vertical column area and provenance");
        check (surfaceWall.endC.x < surfaceWall.begC.x && surfaceWall.openings.IsEmpty (),
               "Inside baseline points left-side thickness inward and has no double openings");
        contact.area = 12 + 2e-6;
        check (SurfaceWall_Build (contact, 0, surfaceWall),
               "surface adapter accepts area noise within unchanged point tolerance");
        contact.area = 12.01;
        check (!SurfaceWall_Build (contact, 0, surfaceWall), "surface adapter rejects meaningful area mismatch");
        contact.contours[0].vertices[2].y = 1.5;
        contact.contours[0].vertices[3].y = 1.5;
        contact.normal = {0, 1 / std::sqrt (1.25), -0.5 / std::sqrt (1.25)};
        contact.area = 12 * std::sqrt (1.25);
        check (SurfaceWall_Build (contact, 0, surfaceWall) && closeArea (surfaceWall.rise_y, 0.5) &&
                   closeArea (OtdWall_GetArea (surfaceWall), contact.area),
               "surface adapter slanted true area");
        GS::Array<OtdWall> bands;
        TypeOtd bandType = Wall_Main;
        OtdMaterial emptyMaterial;
        check (OtdWall_Delim_One (surfaceWall,
                                  bands,
                                  1,
                                  1,
                                  bandType,
                                  emptyMaterial,
                                  emptyMaterial,
                                  emptyMaterial,
                                  emptyMaterial,
                                  emptyMaterial,
                                  emptyMaterial,
                                  emptyMaterial,
                                  emptyMaterial) &&
                   bands.GetSize () == 1 && closeArea (bands[0].begC.y, 0.5) &&
                   closeArea (OtdWall_GetArea (bands[0]), 4 * std::sqrt (1.25)),
               "slanted height band stays on source plane");
        contact.contours[0].vertices[2].y = -1.5;
        contact.contours[0].vertices[3].y = -1.5;
        contact.normal.z = -contact.normal.z;
        check (SurfaceWall_Build (contact, 0, surfaceWall) && closeArea (surfaceWall.rise_y, -0.5) &&
                   surfaceWall.slant_angle < 90 * DEGRAD,
               "surface adapter opposite lean sign");
        contact.contours[0].vertices[2].x = 3;
        check (!SurfaceWall_Build (contact, 0, surfaceWall),
               "surface adapter rejects nonparallelogram instead of bounds");
        RoomSurfacePiece notched;
        notched.status = RoomSurfacePieceStatus::Matched;
        notched.baseGuid = baseA;
        notched.baseType = API_ColumnID;
        notched.normal = {0, 1, 0};
        notched.area = 11;
        RoomSurfacePieceContour notchedContour;
        notchedContour.vertices = {
            {0, 0, 0}, {4, 0, 0}, {4, 0, 3}, {2.5, 0, 3}, {2.5, 0, 2}, {1.5, 0, 2}, {1.5, 0, 3}, {0, 0, 3}};
        notched.contours.Push (notchedContour);
        GS::Array<OtdWall> notchedWalls;
        const auto wallArea = [] (const GS::Array<OtdWall> &walls) {
            double area = 0;
            for (const auto &wall : walls)
                area += OtdWall_GetArea (wall);
            return area;
        };
        check (SurfaceWalls_Build (notched, 0, notchedWalls) && notchedWalls.GetSize () == 1 &&
                   closeArea (wallArea (notchedWalls), 11),
               "notched contact uses one wall with net area");
        check (notchedWalls.GetSize () == 1 && notchedWalls[0].base_guid == baseA &&
                   notchedWalls[0].openings.GetSize () == 1 && closeArea (notchedWalls[0].openings[0].objLoc, 2) &&
                   closeArea (notchedWalls[0].openings[0].width, 1) &&
                   closeArea (notchedWalls[0].openings[0].lower, 2) &&
                   closeArea (notchedWalls[0].openings[0].height, 1) &&
                   notchedWalls[0].openings[0].base_guid == APINULLGuid && !notchedWalls[0].openings[0].has_reveal,
               "edge notch preserves one base and synthetic opening coordinates");
        notched.contours[0].vertices = {{0, 0, 0}, {4, 0, 0}, {4, 0, 3}, {0, 0, 3}};
        notchedContour.hole = true;
        notchedContour.vertices = {{1.5, 0, 1}, {2.5, 0, 1}, {2.5, 0, 2}, {1.5, 0, 2}};
        notched.contours.Push (notchedContour);
        notchedWalls.Clear ();
        check (SurfaceWalls_Build (notched, 0, notchedWalls) && notchedWalls.GetSize () == 1 &&
                   closeArea (wallArea (notchedWalls), 11),
               "wall contact hole is cut from a single envelope");
        check (notchedWalls.GetSize () == 1 && notchedWalls[0].openings.GetSize () == 1 &&
                   closeArea (notchedWalls[0].openings[0].lower, 1),
               "interior opening has a relative sill before height trim");
        OtdWall sourceOpeningWall = notchedWalls[0];
        sourceOpeningWall.surface_geometry = false;
        sourceOpeningWall.begC = {0, 0};
        sourceOpeningWall.endC = {4, 0};
        sourceOpeningWall.openings[0].base_guid = segment;
        OtdWall linkedWall = notchedWalls[0];
        check (SurfaceOpenings_Link (linkedWall, {sourceOpeningWall}) && linkedWall.openings.GetSize () == 1 &&
                   linkedWall.openings[0].base_guid == segment && linkedWall.openings[0].surface_geometry &&
                   !linkedWall.openings[0].has_reveal && closeArea (OtdWall_GetArea (linkedWall), 11),
               "surface opening retains the real opening guid without duplicate reveals");
        auto foreignOpeningWall = sourceOpeningWall;
        foreignOpeningWall.base_guid = baseB;
        linkedWall = notchedWalls[0];
        check (SurfaceOpenings_Link (linkedWall, {foreignOpeningWall}) &&
                   linkedWall.openings[0].base_guid == APINULLGuid,
               "geometric cutout is not linked to an opening of another base");
        auto overlappingOpeningWall = sourceOpeningWall;
        overlappingOpeningWall.openings[0].base_guid = baseB;
        linkedWall = notchedWalls[0];
        check (!SurfaceOpenings_Link (linkedWall, {sourceOpeningWall, overlappingOpeningWall}) &&
                   linkedWall.openings.GetSize () == 1 && linkedWall.openings[0].base_guid == APINULLGuid &&
                   closeArea (OtdWall_GetArea (linkedWall), 11),
               "ambiguous real opening ownership leaves cutouts unchanged");
        linkedWall = notchedWalls[0];
        check (SurfaceOpenings_Link (linkedWall, {sourceOpeningWall, sourceOpeningWall}) &&
                   linkedWall.openings.GetSize () == 1 && linkedWall.openings[0].base_guid == segment,
               "duplicate legacy records do not duplicate a real opening");
        sourceOpeningWall.openings[0].width = 0.5;
        linkedWall = notchedWalls[0];
        bool mixedLinked = SurfaceOpenings_Link (linkedWall, {sourceOpeningWall});
        double realCutoutArea = 0, geometricCutoutArea = 0;
        for (const auto &opening : linkedWall.openings) {
            if (opening.base_guid == segment)
                realCutoutArea += opening.width * opening.height;
            else if (opening.base_guid == APINULLGuid)
                geometricCutoutArea += opening.width * opening.height;
        }
        check (mixedLinked && linkedWall.openings.GetSize () == 3 && closeArea (realCutoutArea, 0.5) &&
                   closeArea (geometricCutoutArea, 0.5) && closeArea (OtdWall_GetArea (linkedWall), 11),
               "mixed notch splits only openings to preserve real provenance and coverage");
        sourceOpeningWall.begC = {4, 0};
        sourceOpeningWall.endC = {0, 0};
        sourceOpeningWall.openings[0].width = 0.6;
        sourceOpeningWall.openings[0].objLoc = 1.8;
        linkedWall = notchedWalls[0];
        check (SurfaceOpenings_Link (linkedWall, {sourceOpeningWall}) && linkedWall.openings.GetSize () == 2 &&
                   linkedWall.openings[0].base_guid == segment && closeArea (linkedWall.openings[0].objLoc, 1.8) &&
                   closeArea (linkedWall.openings[0].width, 0.6) && closeArea (OtdWall_GetArea (linkedWall), 11),
               "reversed legacy axis preserves the real opening center and guid");
        GS::Array<OtdWall> holedBands;
        check (notchedWalls.GetSize () == 1 &&
                   OtdWall_Delim_One (notchedWalls[0],
                                      holedBands,
                                      0.5,
                                      1.5,
                                      bandType,
                                      emptyMaterial,
                                      emptyMaterial,
                                      emptyMaterial,
                                      emptyMaterial,
                                      emptyMaterial,
                                      emptyMaterial,
                                      emptyMaterial,
                                      emptyMaterial) &&
                   holedBands.GetSize () == 1 && holedBands[0].openings.GetSize () == 1 &&
                   closeArea (holedBands[0].openings[0].lower, 0) &&
                   closeArea (holedBands[0].openings[0].height, 0.5) &&
                   closeArea (OtdWall_GetArea (holedBands[0]), 1.5),
               "height band clips opening and preserves net area");
        OtdWall emptyBand = notchedWalls[0];
        emptyBand.openings[0].width = 4;
        holedBands.Clear ();
        check (!OtdWall_Delim_One (emptyBand,
                                   holedBands,
                                   0.5,
                                   1.5,
                                   bandType,
                                   emptyMaterial,
                                   emptyMaterial,
                                   emptyMaterial,
                                   emptyMaterial,
                                   emptyMaterial,
                                   emptyMaterial,
                                   emptyMaterial,
                                   emptyMaterial) &&
                   holedBands.IsEmpty (),
               "fully cut height band does not create an empty wall");
        holedBands.Clear ();
        check (!OtdWall_Delim_One (notchedWalls[0],
                                   holedBands,
                                   0.5,
                                   2 - min_dim / 2,
                                   bandType,
                                   emptyMaterial,
                                   emptyMaterial,
                                   emptyMaterial,
                                   emptyMaterial,
                                   emptyMaterial,
                                   emptyMaterial,
                                   emptyMaterial,
                                   emptyMaterial) &&
                   holedBands.IsEmpty (),
               "subthreshold clipped hole rejects band instead of filling void");
        RoomSurfacePiece multiple = notched;
        auto secondHole = notchedContour;
        for (auto &vertex : secondHole.vertices)
            vertex.x -= 1.25;
        multiple.contours.Push (secondHole);
        multiple.area = 10;
        GS::Array<OtdWall> multipleWalls;
        check (SurfaceWalls_Build (multiple, 0, multipleWalls) && multipleWalls.GetSize () == 1 &&
                   multipleWalls[0].openings.GetSize () == 2 && closeArea (wallArea (multipleWalls), 10),
               "two holes preserve one wall and subtract both areas");
        for (auto &contour : multiple.contours)
            for (auto &vertex : contour.vertices)
                vertex.x = -vertex.x;
        multipleWalls.Clear ();
        check (SurfaceWalls_Build (multiple, 0, multipleWalls) && multipleWalls.GetSize () == 1 &&
                   multipleWalls[0].openings.GetSize () == 2 && closeArea (multipleWalls[0].openings[0].objLoc, 0.75) &&
                   closeArea (multipleWalls[0].openings[1].objLoc, 2) && closeArea (wallArea (multipleWalls), 10),
               "mirrored contour uses final wall direction for openings");
        RoomSurfacePiece tinyHole = notched;
        for (auto &vertex : tinyHole.contours[1].vertices)
            vertex.x = vertex.x > 2 ? 1.5 + min_dim / 2 : 1.5;
        tinyHole.area = 12 - min_dim / 2;
        GS::Array<OtdWall> rejectedWalls;
        check (!SurfaceWalls_Build (tinyHole, 0, rejectedWalls) && rejectedWalls.IsEmpty (),
               "unrepresentable small cutout does not fill the void");
        for (auto &contour : notched.contours)
            for (auto &vertex : contour.vertices)
                vertex.y = 0.5 * vertex.z;
        notched.normal = {0, 1 / std::sqrt (1.25), -0.5 / std::sqrt (1.25)};
        notched.area = 11 * std::sqrt (1.25);
        notchedWalls.Clear ();
        check (SurfaceWalls_Build (notched, 0, notchedWalls) && notchedWalls.GetSize () == 1 &&
                   closeArea (wallArea (notchedWalls), notched.area) && closeArea (notchedWalls[0].rise_y, 0.5),
               "slanted holed contact preserves true area and source plane");
        notched.contours[0].vertices[2].x = 3;
        const USize previousWalls = notchedWalls.GetSize ();
        check (!SurfaceWalls_Build (notched, 0, notchedWalls) && notchedWalls.GetSize () == previousWalls,
               "unsupported diagonal contour leaves output unchanged");
        const auto rectanglePiece = [&] (double left, double right, double bottom, double top) {
            RoomSurfacePiece piece;
            piece.status = RoomSurfacePieceStatus::Matched;
            piece.baseGuid = baseA;
            piece.baseType = API_WallID;
            piece.zoneBodyIndex = 1;
            piece.normal = {0, 1, 0};
            piece.area = (right - left) * (top - bottom);
            RoomSurfacePieceContour contour;
            contour.vertices = {{left, 0, bottom}, {right, 0, bottom}, {right, 0, top}, {left, 0, top}};
            piece.contours.Push (contour);
            RoomSurfaceSourceFace face;
            face.baseGuid = baseA;
            face.baseType = API_WallID;
            face.geometryGuid = baseA;
            face.bodyIndex = 1;
            face.polygonIndex = 4;
            piece.sources.Push (face);
            return piece;
        };
        const auto sameWalls = [&] (const GS::Array<OtdWall> &a, const GS::Array<OtdWall> &b) {
            if (a.GetSize () != b.GetSize ())
                return false;
            std::vector<bool> used (b.GetSize (), false);
            for (const auto &wall : a) {
                bool found = false;
                for (UIndex i = 0; i < b.GetSize (); ++i) {
                    const auto &other = b[i];
                    if (!used[i] && wall.base_guid == other.base_guid &&
                        std::hypot (wall.begC.x - other.begC.x, wall.begC.y - other.begC.y) < 1e-6 &&
                        std::hypot (wall.endC.x - other.endC.x, wall.endC.y - other.endC.y) < 1e-6 &&
                        std::fabs (wall.zBottom - other.zBottom) < 1e-6 &&
                        std::fabs (wall.height - other.height) < 1e-6 &&
                        std::fabs (wall.slant_angle - other.slant_angle) < 1e-6) {
                        if (wall.openings.GetSize () != other.openings.GetSize ())
                            continue;
                        bool sameOpenings = true;
                        for (UIndex opening = 0; opening < wall.openings.GetSize (); ++opening) {
                            const auto &first = wall.openings[opening], &second = other.openings[opening];
                            sameOpenings = sameOpenings && std::fabs (first.objLoc - second.objLoc) < 1e-6 &&
                                           std::fabs (first.width - second.width) < 1e-6 &&
                                           std::fabs (first.zBottom - second.zBottom) < 1e-6 &&
                                           std::fabs (first.height - second.height) < 1e-6;
                        }
                        if (!sameOpenings)
                            continue;
                        used[i] = true;
                        found = true;
                        break;
                    }
                }
                if (!found)
                    return false;
            }
            return true;
        };
        const auto groupWalls = [&] (const GS::Array<RoomSurfacePiece> &pieces, GS::Array<OtdWall> &walls) {
            GS::Array<RoomSurfacePiece> grouped;
            if (!SurfaceWallPieces_Group (pieces, 0, grouped))
                return false;
            for (const auto &piece : grouped)
                if (!SurfaceWalls_Build (piece, 0, walls))
                    return false;
            return true;
        };
        GS::Array<RoomSurfacePiece> split = {rectanglePiece (0, 4, 0, 2),
                                             rectanglePiece (0, 1, 2, 3),
                                             rectanglePiece (2, 4, 2, 3),
                                             rectanglePiece (4, 5, 1, 3)};
        auto unitedPiece = rectanglePiece (0, 5, 0, 3);
        unitedPiece.area = 13;
        unitedPiece.contours[0].vertices = {{0, 0, 0},
                                            {4, 0, 0},
                                            {4, 0, 1},
                                            {5, 0, 1},
                                            {5, 0, 3},
                                            {2, 0, 3},
                                            {2, 0, 2},
                                            {1, 0, 2},
                                            {1, 0, 3},
                                            {0, 0, 3}};
        GS::Array<OtdWall> separateWalls, canonicalWalls, outlineWalls;
        for (const auto &piece : split)
            SurfaceWalls_Build (piece, 0, separateWalls);
        check (groupWalls ({unitedPiece}, outlineWalls) && !sameWalls (separateWalls, outlineWalls),
               "regression reproduces per-piece seam sensitivity");
        check (groupWalls (split, canonicalWalls) && sameWalls (canonicalWalls, outlineWalls) &&
                   closeArea (wallArea (canonicalWalls), 13),
               "same source union makes split and outline native partitions equal");
        GS::Array<RoomSurfacePiece> reorderedPieces;
        for (UIndex i = split.GetSize (); i > 0; --i) {
            auto piece = split[i - 1];
            auto &vertices = piece.contours[0].vertices;
            const auto firstPoint = vertices[0];
            for (UIndex vertex = 1; vertex < vertices.GetSize (); ++vertex)
                vertices[vertex - 1] = vertices[vertex];
            vertices[vertices.GetSize () - 1] = firstPoint;
            reorderedPieces.Push (piece);
        }
        GS::Array<OtdWall> reorderedWalls;
        check (groupWalls (reorderedPieces, reorderedWalls) && sameWalls (canonicalWalls, reorderedWalls),
               "group union ignores piece order and contour start");
        split.Push (split[0]);
        GS::Array<OtdWall> duplicateWalls;
        check (groupWalls (split, duplicateWalls) && sameWalls (canonicalWalls, duplicateWalls),
               "overlapping duplicate contacts use union rather than parity");
        split = {rectanglePiece (0, 4, 0, 1),
                 rectanglePiece (0, 4, 1, 2),
                 rectanglePiece (0, 1, 2, 3),
                 rectanglePiece (2, 4, 2, 3),
                 rectanglePiece (4, 5, 1, 2),
                 rectanglePiece (4, 5, 2, 3)};
        GS::Array<OtdWall> subdividedWalls;
        check (groupWalls (split, subdividedWalls) && sameWalls (canonicalWalls, subdividedWalls),
               "internal additional splits do not survive canonical union");
        auto diagonalTail = rectanglePiece (4, 4.5, 1, 3);
        diagonalTail.area = 0.75;
        diagonalTail.contours[0].vertices = {{4, 0, 1}, {4.5, 0, 2}, {4.5, 0, 3}, {4, 0, 3}};
        GS::Array<RoomSurfacePiece> unsupportedGrouped;
        GS::Array<OtdWall> unsupportedWalls;
        const bool wholeContact =
            SurfaceWallPieces_Group ({rectanglePiece (0, 4, 0, 3), diagonalTail}, 0, unsupportedGrouped);
        check (wholeContact && unsupportedGrouped.GetSize () == 1 && closeArea (unsupportedGrouped[0].area, 12.75) &&
                   !SurfaceWalls_Build (unsupportedGrouped[0], 0, unsupportedWalls) && unsupportedWalls.IsEmpty (),
               "same source diagonal tail is not dropped before representation check");
        GS::Array<RoomSurfacePiece> repeatedUnsupported;
        check (wholeContact && SurfaceWallPieces_Group (unsupportedGrouped, 0, repeatedUnsupported) &&
                   repeatedUnsupported.GetSize () == 1 &&
                   !SurfaceWalls_Build (repeatedUnsupported[0], 0, unsupportedWalls) && unsupportedWalls.IsEmpty (),
               "unsupported whole contour has the same outcome as its fragmented input");
        GS::Array<RoomSurfacePiece> pair = {rectanglePiece (0, 1, 0, 3), rectanglePiece (1, 2, 0, 3)};
        pair[1].planeOffset = -1.1e-7;
        for (auto &vertex : pair[1].contours[0].vertices)
            vertex.y = 1.1e-7;
        GS::Array<RoomSurfacePiece> nearGrouped;
        check (SurfaceWallPieces_Group (pair, 0, nearGrouped) && nearGrouped.GetSize () == 1 &&
                   closeArea (nearGrouped[0].area, 6),
               "coplanar point noise uses unchanged distance tolerance");
        bool boundaries = true;
        for (Int32 change = 0; change < 7; ++change) {
            pair = {rectanglePiece (0, 1, 0, 3), rectanglePiece (1, 2, 0, 3)};
            if (change == 0)
                pair[1].baseGuid = baseB;
            if (change == 1)
                pair[1].sources[0].geometryGuid = baseB;
            if (change == 2)
                pair[1].sources[0].bodyIndex = 2;
            if (change == 3)
                pair[1].sources[0].polygonIndex = 5;
            if (change == 4) {
    #ifdef ServerMainVers_2700
                pair[1].sources[0].modelMaterialIndex = ACAPI_CreateAttributeIndex (7);
    #else
                pair[1].sources[0].modelMaterialIndex = 7;
    #endif
            }
            if (change == 5)
                pair[1].zoneBodyIndex = 2;
            if (change == 6)
                pair[1].normal.y = -1;
            GS::Array<RoomSurfacePiece> separate;
            boundaries = boundaries && SurfaceWallPieces_Group (pair, 0, separate) && separate.GetSize () == 2;
        }
        check (boundaries, "group union preserves full source and orientation boundaries");
        pair = {rectanglePiece (0, 4, 0, 3), rectanglePiece (5, 6, 0, 1)};
        RoomSurfacePieceContour groupHole;
        groupHole.hole = true;
        groupHole.vertices = {{1, 0, 1}, {2, 0, 1}, {2, 0, 2}, {1, 0, 2}};
        pair[0].contours.Push (groupHole);
        pair[0].area = 11;
        GS::Array<RoomSurfacePiece> holeGrouped;
        GS::Array<OtdWall> holeWalls;
        bool hasHole = false;
        if (SurfaceWallPieces_Group (pair, 0, holeGrouped))
            for (const auto &piece : holeGrouped)
                hasHole = hasHole || piece.contours.GetSize () == 2;
        check (hasHole && holeGrouped.GetSize () == 2 && groupWalls (pair, holeWalls) &&
                   closeArea (wallArea (holeWalls), 12),
               "group union preserves holes and disconnected components");
        contact.normal = {0, 0, 1};
        contact.area = 15;
        contact.baseType = API_BeamID;
        contact.contours.Clear ();
        contactContour.vertices = {{0, 0, 0}, {4, 0, 0}, {4, 4, 0}, {0, 4, 0}};
        contact.contours.Push (contactContour);
        contactContour.hole = true;
        contactContour.vertices = {{1, 1, 0}, {2, 1, 0}, {2, 2, 0}, {1, 2, 0}};
        contact.contours.Push (contactContour);
        GS::Array<OtdSlab> surfaceSlabs;
        check (SurfaceSlabs_Build (contact, 0, surfaceSlabs) && surfaceSlabs.GetSize () == 1 &&
                   surfaceSlabs[0].type == Ceil && closeArea (surfaceSlabs[0].poly.CalcArea (), 15) &&
                   surfaceSlabs[0].base_type == API_BeamID,
               "horizontal adapter preserves ceiling hole and beam parent");
        check (!SurfaceWall_Build (contact, 0, surfaceWall), "holed horizontal surface is not a wall");
        // API_WallPart: направление, углы, сторона, sentinel-границы и сохранение разрывов.
        {
            const auto wallPart = [&] (double a, double b, Int32 edge = 1) {
                API_WallPart part = {};
                part.guid = baseA;
                part.roomEdge = edge;
                part.tBeg = a;
                part.tEnd = b;
                return part;
            };
            const auto contactWall = [&] (double a, double b) {
                OtdWall wall;
                wall.base_guid = baseA;
                wall.base_type = API_WallID;
                wall.surface_geometry = true;
                wall.begC = {a, 0};
                wall.endC = {b, 0};
                wall.length = std::fabs (b - a);
                wall.height = 3;
                return wall;
            };
            const GS::Array<Sector> edges = {{{0, 0}, {4, 0}}, {{4, 1}, {0, 1}}};
            const auto clamp = [&] (const OtdWall &wall,
                                    const GS::Array<API_WallPart> &parts,
                                    const GS::Array<OtdWall> &peers = GS::Array<OtdWall> ()) {
                return SurfaceWall_ClampToParts (wall, {0, 0}, {4, 0}, parts, edges, 0, peers);
            };
            auto wall = contactWall (4, 0);
            auto result = clamp (wall, {wallPart (1, 3)});
            check (result.GetSize () == 1 && closeArea (result[0].begC.x, 3) && closeArea (result[0].endC.x, 1) &&
                       closeArea (result[0].length, 2),
                   "wallPart preserves reversed surface direction");
            OtdOpening opening;
            opening.objLoc = 1.5;
            opening.width = 2;
            opening.height = 1;
            opening.zBottom = .5;
            wall.openings.Push (opening);
            result = clamp (wall, {wallPart (1, 3)});
            check (result.GetSize () == 1 && result[0].openings.GetSize () == 1 &&
                       closeArea (result[0].openings[0].objLoc, .75) && closeArea (result[0].openings[0].width, 1.5),
                   "wallPart clips opening in final direction");
            result = clamp (wall, {wallPart (0, 1), wallPart (3, 4)});
            check (result.GetSize () == 2 && closeArea (result[0].length, 1) && closeArea (result[1].length, 1),
                   "wallPart keeps disconnected intervals");
            result = clamp (wall, {wallPart (0, 3), wallPart (2, 4), wallPart (0, 3)});
            check (result.GetSize () == 1 && closeArea (result[0].length, 4),
                   "wallPart merges overlapping and duplicate intervals");
            result = clamp (wall, {wallPart (0, 0)});
            check (result.GetSize () == 1 && closeArea (result[0].length, 4), "wallPart preserves legacy zero bounds");
            result = clamp (wall, {});
            check (result.GetSize () == 1 && closeArea (result[0].begC.x, 4) && result[0].openings.GetSize () == 1,
                   "wallPart absent keeps wall and openings");
            result = clamp (wall, {wallPart (0, 4, 2)});
            check (result.IsEmpty (), "wallPart opposite room edge cannot trim this side");
            result = clamp (wall, {wallPart (0, 4, 2), wallPart (1, 3)});
            check (result.GetSize () == 1 && closeArea (result[0].length, 2),
                   "wallPart chooses matching roomEdge rather than guid alone");
            check (clamp (wall, {wallPart (0, 4, 0)}).IsEmpty () && clamp (wall, {wallPart (0, 4, 3)}).IsEmpty (),
                   "wallPart invalid edge index rejects without partial output");
            check (clamp (wall, {wallPart (5, 6)}).IsEmpty (), "wallPart nonoverlapping finite edge rejects");
            wall = contactWall (3.8, .2);
            result = clamp (wall, {wallPart (0, 4)});
            check (result.GetSize () == 1 && closeArea (result[0].begC.x, 4) && closeArea (result[0].endC.x, 0),
                   "wallPart extends short outer contact to both room corners");
            auto left = contactWall (1.5, .2), right = contactWall (3.8, 2.5);
            const GS::Array<OtdWall> peers = {left, right};
            result = clamp (left, {wallPart (0, 4)}, peers);
            check (result.GetSize () == 1 && closeArea (result[0].begC.x, 1.5) && closeArea (result[0].endC.x, 0),
                   "wallPart left contact preserves inner gap");
            result = clamp (right, {wallPart (0, 4)}, peers);
            check (result.GetSize () == 1 && closeArea (result[0].begC.x, 4) && closeArea (result[0].endC.x, 2.5),
                   "wallPart right contact preserves inner gap");
            opening.objLoc = .5;
            opening.width = 1;
            wall.openings.Push (opening);
            result = clamp (wall, {wallPart (0, 4)});
            check (result.GetSize () == 1 && result[0].openings.GetSize () == 1 &&
                       closeArea (result[0].openings[0].objLoc, .6) && closeArea (result[0].openings[0].width, 1.2),
                   "wallPart extension keeps edge notch open");
            wall = contactWall (0, 4);
            result = clamp (wall, {wallPart (1, 3)});
            check (result.GetSize () == 1 && closeArea (result[0].begC.x, 1) && closeArea (result[0].endC.x, 3),
                   "wallPart preserves forward surface direction");
            wall.rise_y = .5;
            wall.zBottom = 1;
            wall.begC.y = wall.endC.y = .5;
            result = clamp (wall, {wallPart (1, 3)});
            check (result.GetSize () == 1 && closeArea (result[0].begC.y, .5) && closeArea (result[0].rise_y, .5) &&
                       closeArea (result[0].zBottom, 1),
                   "wallPart translates room edge to slanted contact elevation");
            wall = contactWall (3.8, .2);
            const auto vertical = SurfaceWall_ClampToParts (wall, {4, 0}, {0, 0}, {wallPart (.5, 2)}, edges, 0, {});
            check (vertical.GetSize () == 1 && closeArea (vertical[0].begC.x, 3.5) && closeArea (vertical[0].endC.x, 2),
                   "wallPart measures distances from reversed base start");
            wall.begC = {4, 2.8};
            wall.endC = {4, .2};
            const GS::Array<Sector> cornerEdges = {{{4, 0}, {4, 3}}};
            result = SurfaceWall_ClampToParts (wall, {4, 0}, {4, 3}, {wallPart (0, 3)}, cornerEdges, 0, {});
            check (result.GetSize () == 1 && closeArea (result[0].begC.x, 4) && closeArea (result[0].endC.x, 4) &&
                       closeArea (result[0].endC.y, 0) && closeArea (result[0].begC.y, 3),
                   "wallPart perpendicular wall reaches shared room corner");
            auto foreign = wallPart (1, 3);
            foreign.guid = baseB;
            wall = contactWall (4, 0);
            result = clamp (wall, {foreign});
            check (result.GetSize () == 1 && closeArea (result[0].length, 4),
                   "wallPart foreign guid does not change wall");
            auto invalid = wallPart (0, 4);
            invalid.tBeg = std::numeric_limits<double>::quiet_NaN ();
            check (clamp (wall, {wallPart (1, 3), invalid}).IsEmpty (),
                   "wallPart nonfinite bounds reject without partial output");
            check (clamp (wall, {wallPart (1, 1 + min_dim / 2)}).IsEmpty (), "wallPart subthreshold fragment rejects");
        }
        const auto atLevel = [] (RoomSurface face, double z) {
            face.planeOffset = -face.normalWorld.z * z;
            for (auto &contour : face.contours)
                for (auto &point : contour.vertices)
                    point.world.z = point.local.z = z;
            return face;
        };
        const auto slabArea = [] (const GS::Array<OtdSlab> &slabs) {
            double sum = 0;
            for (const auto &slab : slabs)
                sum += slab.poly.CalcArea ();
            return sum;
        };
        auto floorZone = snapshot (zoneGuid, atLevel (rectangle (0, 4, 0, 3, -1), 0.4));
        auto lower = source (baseA, atLevel (rectangle (0, 4, 0, 3, 1), -0.2), API_SlabID);
        auto slabBottomFace = atLevel (rectangle (0, 4, 0, 3, -1), -0.5);
        slabBottomFace.polygonIndex = 2;
        lower.snapshot.bodies[0].surfaces.Push (slabBottomFace);
        GS::Array<RoomSurfaceSource> slabSources = {lower};
        GS::Array<API_Guid> roles = {baseA};
        GS::Array<OtdSlab> projected;
        double rejected = 0;
        check (OtdSlab_BuildProjectedFinishes (floorZone, slabSources, roles, Floor, 0, projected, rejected) &&
                   projected.GetSize () == 1 && closeArea (slabArea (projected), 12) &&
                   closeArea (projected[0].zBottom, -0.2) && projected[0].base_guid == baseA &&
                   projected[0].type == Floor && projected[0].surface_geometry && rejected == 0,
               "discovered floor across gap uses actual upper slab face and original guid");
        auto ceilingZone = snapshot (zoneGuid, atLevel (rectangle (0, 4, 0, 3, 1), 2.8));
        slabSources = {source (baseB, atLevel (rectangle (0, 4, 0, 3, -1), 3.4), API_SlabID)};
        auto slabTopFace = atLevel (rectangle (0, 4, 0, 3, 1), 3.8);
        slabTopFace.polygonIndex = 2;
        slabSources[0].snapshot.bodies[0].surfaces.Push (slabTopFace);
        roles = {baseB};
        projected.Clear ();
        check (OtdSlab_BuildProjectedFinishes (ceilingZone, slabSources, roles, Ceil, 0, projected, rejected) &&
                   projected.GetSize () == 1 && closeArea (projected[0].zBottom, 3.4) &&
                   projected[0].base_guid == baseB && projected[0].type == Ceil,
               "discovered ceiling across gap uses actual lower slab face");
        OtdSlab nominalCeiling;
        nominalCeiling.poly = projected[0].poly;
        nominalCeiling.zBottom = 2.8;
        projected.Clear ();
        check (OtdSlab_BuildProjectedFinishes (
                   ceilingZone, slabSources, roles, Ceil, 0, projected, rejected, &nominalCeiling, false) &&
                   projected.GetSize () == 1 && closeArea (projected[0].zBottom, 2.8) &&
                   closeArea (slabArea (projected), 12),
               "ceiling above zone height falls back to nominal zone ceiling candidate");
        projected.Clear ();
        check (OtdSlab_BuildProjectedFinishes (
                   ceilingZone, slabSources, roles, Ceil, 0, projected, rejected, &nominalCeiling, true) &&
                   projected.GetSize () == 1 && closeArea (projected[0].zBottom, 3.4) &&
                   projected[0].base_guid == baseB,
               "ceil_by_slab ignores nominal zone ceiling candidate");
        roles = {baseA};
        projected.Clear ();
        check (!OtdSlab_BuildProjectedFinishes (ceilingZone, slabSources, roles, Ceil, 0, projected, rejected) &&
                   projected.IsEmpty (),
               "missing role snapshot fails atomically");
        slabSources = {source (baseA, atLevel (rectangle (0, 2, 0, 3, 1), -0.2), API_SlabID),
                       source (baseB, atLevel (rectangle (2, 4, 0, 3, 1), -0.4), API_SlabID)};
        roles = {baseA, baseB, baseA};
        projected.Clear ();
        check (OtdSlab_BuildProjectedFinishes (floorZone, slabSources, roles, Floor, 0, projected, rejected) &&
                   projected.GetSize () == 2 && closeArea (slabArea (projected), 12) && rejected == 0,
               "adjacent different slab levels split room coverage without duplicate roles");
        GS::Array<OtdSlab> reversedSlabs;
        GS::Array<RoomSurfaceSource> reverseSources = {slabSources[1], slabSources[0]};
        double reverseRejected = 0;
        bool stable = OtdSlab_BuildProjectedFinishes (
                          floorZone, reverseSources, roles, Floor, 0, reversedSlabs, reverseRejected) &&
                      reversedSlabs.GetSize () == projected.GetSize ();
        for (UIndex i = 0; stable && i < projected.GetSize (); ++i)
            stable = projected[i].base_guid == reversedSlabs[i].base_guid &&
                     closeArea (projected[i].zBottom, reversedSlabs[i].zBottom) &&
                     closeArea (projected[i].poly.CalcArea (), reversedSlabs[i].poly.CalcArea ());
        check (stable, "projected ownership is independent of candidate order");
        slabSources[1] = source (baseB, atLevel (rectangle (1, 4, 0, 3, 1), -0.4), API_SlabID);
        projected.Clear ();
        check (OtdSlab_BuildProjectedFinishes (floorZone, slabSources, roles, Floor, 0, projected, rejected) &&
                   closeArea (slabArea (projected), 12) && closeArea (rejected, 0) && projected.GetSize () == 2 &&
                   projected[0].base_guid == baseA && closeArea (projected[0].poly.CalcArea (), 6),
               "unequal floor elevations use descending priority then remaining coverage");
        const auto fullUpper = source (baseA, atLevel (rectangle (0, 4, 0, 3, 1), -0.2), API_SlabID);
        const auto fullLower = source (baseB, atLevel (rectangle (0, 4, 0, 3, 1), -0.6), API_SlabID);
        for (bool reverseSourceOrder : {false, true}) {
            for (bool reverseRoleOrder : {false, true}) {
                const GS::Array<RoomSurfaceSource> fullSources =
                    reverseSourceOrder ? GS::Array<RoomSurfaceSource>{fullLower, fullUpper}
                                       : GS::Array<RoomSurfaceSource>{fullUpper, fullLower};
                const GS::Array<API_Guid> fullRoles =
                    reverseRoleOrder ? GS::Array<API_Guid>{baseB, baseA} : GS::Array<API_Guid>{baseA, baseB};
                GS::Array<OtdSlab> fullProjected;
                double fullRejected = 0;
                check (OtdSlab_BuildProjectedFinishes (
                           floorZone, fullSources, fullRoles, Floor, 0, fullProjected, fullRejected) &&
                           fullProjected.GetSize () == 1 && fullProjected[0].base_guid == baseA &&
                           fullProjected[0].type == Floor && closeArea (fullProjected[0].zBottom, -0.2) &&
                           closeArea (fullProjected[0].poly.CalcArea (), 12) && closeArea (fullRejected, 0),
                       "complete upper floor covers strict lower floor in either source and role order");
            }
        }
        auto holed = atLevel (rectangle (0, 4, 0, 3, 1), -0.2);
        holed.contours.Push (atLevel (rectangle (1, 2, 1, 2, 1), -0.2).contours[0]);
        holed.contours[1].hole = true;
        slabSources = {source (baseA, holed, API_SlabID)};
        roles = {baseA};
        projected.Clear ();
        rejected = 0;
        check (OtdSlab_BuildProjectedFinishes (floorZone, slabSources, roles, Floor, 0, projected, rejected) &&
                   closeArea (slabArea (projected), 11) && rejected == 0,
               "projected slab opening remains empty without zone fallback");
        floorZone.bodies.Push (floorZone.bodies[0]);
        projected.Clear ();
        check (OtdSlab_BuildProjectedFinishes (floorZone, slabSources, roles, Floor, 0, projected, rejected) &&
                   closeArea (slabArea (projected), 11),
               "duplicate room bodies do not duplicate slab finishes");
        auto conflictingFace = atLevel (rectangle (0, 4, 0, 3, 1), -0.6);
        conflictingFace.polygonIndex = 2;
        slabSources[0].snapshot.bodies[0].surfaces.Push (conflictingFace);
        projected.Clear ();
        rejected = 0;
        check (OtdSlab_BuildProjectedFinishes (floorZone, slabSources, roles, Floor, 0, projected, rejected) &&
                   closeArea (slabArea (projected), 11) && closeArea (rejected, 0) && projected.GetSize () == 1 &&
                   closeArea (projected[0].zBottom, -0.2),
               "same slab levels retain higher face and exclude its holes from lower coverage");
        slabSources = {source (baseA, atLevel (rectangle (5, 6, 0, 3, 1), -0.2), API_SlabID)};
        projected.Clear ();
        rejected = 0;
        check (OtdSlab_BuildProjectedFinishes (floorZone, slabSources, roles, Floor, 0, projected, rejected) &&
                   projected.IsEmpty () && rejected == 0,
               "discovered slab outside room projection creates no finish and no fallback");
        OtdSlab nominal;
        nominal.zBottom = 0;
        GS::Array<OtdWall> steps;
        slabSources.Clear ();
        roles.Clear ();
        projected.Clear ();
        check (OtdSlab_BuildProjectedFinishes (
                   floorZone, slabSources, roles, Floor, 2, projected, rejected, &nominal, false, &steps) &&
                   projected.GetSize () == 1 && closeArea (slabArea (projected), 12) &&
                   projected[0].base_guid == APINULLGuid && projected[0].floorInd == 2 && projected[0].zBottom == 0 &&
                   projected[0].surface_geometry && steps.IsEmpty (),
               "zero-source floor uses nominal residual when by-slab is disabled");
        projected.Clear ();
        check (OtdSlab_BuildProjectedFinishes (
                   floorZone, slabSources, roles, Floor, 0, projected, rejected, &nominal, true, &steps) &&
                   projected.IsEmpty (),
               "zero-source floor by-slab suppresses residual");
        nominalCeiling.zBottom = 3.1;
        projected.Clear ();
        check (OtdSlab_BuildProjectedFinishes (
                   ceilingZone, slabSources, roles, Ceil, 0, projected, rejected, &nominalCeiling, false, &steps) &&
                   projected.GetSize () == 1 && closeArea (projected[0].zBottom, 3.1) && projected[0].type == Ceil,
               "zero-source ceiling residual uses supplied main-plus-down nominal plane");
        slabSources = {source (baseA, atLevel (rectangle (0, 2, 0, 3, -1), 3.2), API_SlabID),
                       source (baseB, atLevel (rectangle (1, 4, 0, 3, -1), 3.4), API_SlabID)};
        roles = {baseA, baseB};
        projected.Clear ();
        rejected = 0;
        check (OtdSlab_BuildProjectedFinishes (ceilingZone, slabSources, roles, Ceil, 0, projected, rejected) &&
                   projected.GetSize () == 2 && closeArea (slabArea (projected), 12) &&
                   projected[0].base_guid == baseA && closeArea (projected[0].zBottom, 3.2) &&
                   closeArea (projected[0].poly.CalcArea (), 6) && projected[1].base_guid == baseB &&
                   closeArea (projected[1].zBottom, 3.4) && closeArea (projected[1].poly.CalcArea (), 6) &&
                   rejected == 0,
               "ceiling uses ascending lower-face priority then remaining coverage");
        slabSources[0] = source (baseA, atLevel (rectangle (0, 2, 0, 3, -1), 3.4), API_SlabID);
        // Номинальный потолок идёт после перекрытий, чтобы проверить неоднозначность их покрытия.
        nominalCeiling.zBottom = 3.6;
        projected.Clear ();
        check (OtdSlab_BuildProjectedFinishes (
                   ceilingZone, slabSources, roles, Ceil, 0, projected, rejected, &nominalCeiling, false, &steps) &&
                   closeArea (slabArea (projected), 9) && closeArea (rejected, 3),
               "equal-level owner overlap is excluded, never converted to nominal fallback");
        slabSources = {source (baseA, atLevel (rectangle (0, 2, 0, 3, 1), 0.2), API_SlabID)};
        slabSources[0].snapshot.bodies[0].surfaces.Push (atLevel (rectangle (0, 2, 0, 3, -1), -0.1));
        roles = {baseA};
        nominal.zBottom = 0;
        projected.Clear ();
        steps.Clear ();
        rejected = 0;
        check (OtdSlab_BuildProjectedFinishes (
                   floorZone, slabSources, roles, Floor, 0, projected, rejected, &nominal, false, &steps) &&
                   projected.GetSize () == 2 && closeArea (slabArea (projected), 12) &&
                   projected[1].base_guid == APINULLGuid && closeArea (projected[1].zBottom, 0) &&
                   steps.GetSize () == 1 && closeArea (steps[0].height, 0.2) && closeArea (steps[0].zBottom, 0) &&
                   steps[0].base_guid == baseA && steps[0].type == Floor && steps[0].surface_geometry,
               "partial floor keeps source plus nominal remainder and only internal step edge");
        projected.Clear ();
        steps.Clear ();
        check (OtdSlab_BuildProjectedFinishes (
                   floorZone, slabSources, roles, Floor, 0, projected, rejected, &nominal, true, &steps) &&
                   projected.GetSize () == 1 && closeArea (slabArea (projected), 6),
               "partial floor by-slab flag suppresses nominal remainder only");
        nominal.zBottom = -0.2;
        projected.Clear ();
        steps.Clear ();
        check (OtdSlab_BuildProjectedFinishes (
                   floorZone, slabSources, roles, Floor, 0, projected, rejected, &nominal, false, &steps) &&
                   steps.IsEmpty (),
               "no side finish if nominal floor lies outside source body");
        nominal.zBottom = 0.196;
        projected.Clear ();
        check (OtdSlab_BuildProjectedFinishes (
                   floorZone, slabSources, roles, Floor, 0, projected, rejected, &nominal, false, &steps) &&
                   steps.IsEmpty (),
               "step below legacy five millimeter height threshold is rejected");
        slabSources = {source (baseA, holed, API_SlabID)};
        nominal.zBottom = 0;
        projected.Clear ();
        check (OtdSlab_BuildProjectedFinishes (
                   floorZone, slabSources, roles, Floor, 0, projected, rejected, &nominal, false, &steps) &&
                   projected.GetSize () == 1 && closeArea (slabArea (projected), 11),
               "source holes are not backfilled even when nominal residual is enabled");
        slabSources = {source (baseA, atLevel (rectangle (0, 2, 0, 3, -1), 3.2), API_SlabID)};
        slabSources[0].snapshot.bodies[0].surfaces.Push (atLevel (rectangle (0, 2, 0, 3, 1), 3.6));
        nominalCeiling.zBottom = 3.4;
        projected.Clear ();
        steps.Clear ();
        check (OtdSlab_BuildProjectedFinishes (
                   ceilingZone, slabSources, roles, Ceil, 0, projected, rejected, &nominalCeiling, false, &steps) &&
                   projected.GetSize () == 2 && steps.GetSize () == 1 && steps[0].type == Ceil &&
                   closeArea (steps[0].height, 0.2) && closeArea (steps[0].zBottom, 3.2),
               "ceiling internal step spans source lower face to nominal plane");
        // Проверяем принятые после правил стеновые участки, не исходные прямоугольники перепадов.
        OtdWall side;
        side.base_guid = baseA;
        side.base_type = API_SlabID;
        side.surface_geometry = true;
        side.begC = {0, 0};
        side.endC = {4, 0};
        side.height = 3;
        side.type = Wall_Main;
        side.material.smaterial = "Контакт";
        side.favorite.name = "Стеновое избранное";
        side.favorite.type = API_WallID;
        OtdWall step = side;
        step.slab_step = true;
        step.type = Floor;
        step.material.smaterial = "Перепад";
        const auto acceptedRoom = [&] (const OtdWall &contact, const OtdWall &acceptedStep) {
            OtdRoom room;
            room.create_wall_elements = true;
            room.otdwall = {contact, acceptedStep};
            return room;
        };
        const auto contactArea = [&] (const OtdRoom &room) {
            double area = 0;
            for (const auto &w : room.otdwall)
                if (!w.slab_step)
                    area += OtdWall_GetArea (w);
            return area;
        };
        auto accepted = acceptedRoom (side, step);
        check (SlabContacts_DeduplicateAccepted (accepted) && accepted.otdwall.GetSize () == 1 &&
                   accepted.otdwall[0].slab_step && closeArea (contactArea (accepted), 0),
               "принятое полное покрытие перепада убирает только дубль контакта независимо от материала");
        step.endC = {2, 0};
        step.height = 1;
        accepted = acceptedRoom (side, step);
        check (SlabContacts_DeduplicateAccepted (accepted) && closeArea (contactArea (accepted), 10) &&
                   accepted.otdwall[0].material.smaterial == side.material.smaterial &&
                   accepted.otdwall[0].favorite.name == side.favorite.name && accepted.otdwall[0].type == Wall_Main,
               "частичное конечное покрытие сохраняет остаток и уже принятые правила контакта");
        step.isValid = false;
        accepted = acceptedRoom (side, step);
        check (SlabContacts_DeduplicateAccepted (accepted) && closeArea (contactArea (accepted), 12),
               "отвергнутая правилами боковина не стирает контакт");
        step.isValid = true;
        step.slab_step_writer_eligible = false;
        accepted = acceptedRoom (side, step);
        check (SlabContacts_DeduplicateAccepted (accepted) && closeArea (contactArea (accepted), 12),
               "непредставимое избранное боковины не стирает контакт");
        step.slab_step_writer_eligible = true;
        step.base_guid = baseB;
        accepted = acceptedRoom (side, step);
        check (SlabContacts_DeduplicateAccepted (accepted) && closeArea (contactArea (accepted), 12),
               "другое основание не вычитается");
        step.base_guid = baseA;
        step.begC.y = step.endC.y = 1;
        accepted = acceptedRoom (side, step);
        check (SlabContacts_DeduplicateAccepted (accepted) && closeArea (contactArea (accepted), 12),
               "параллельная другая плоскость не вычитается");
        step.begC.y = step.endC.y = 0;
        step.type = Wall_Up;
        accepted = acceptedRoom (side, step);
        accepted.create_wall_elements = false;
        accepted.create_reveal_elements = true;
        check (SlabContacts_DeduplicateAccepted (accepted) && closeArea (contactArea (accepted), 12),
               "отключённый writer боковины после смены типа не стирает контакт");
        step.type = Ceil;
        step.begC = {1, 0};
        step.endC = {3, 0};
        step.zBottom = 1;
        step.height = 1;
        accepted = acceptedRoom (side, step);
        check (SlabContacts_DeduplicateAccepted (accepted) && closeArea (contactArea (accepted), 10) &&
                   accepted.otdwall[0].openings.GetSize () == 1,
               "внутреннее конечное покрытие потолочной боковины становится точным отверстием");
        OtdOpening acceptedHole;
        acceptedHole.width = 1;
        acceptedHole.height = 1;
        acceptedHole.objLoc = 2;
        acceptedHole.zBottom = 1;
        acceptedHole.lower = 1;
        acceptedHole.surface_geometry = true;
        side.openings = {acceptedHole};
        step.begC = {0, 0};
        step.endC = {1, 0};
        step.zBottom = 0;
        step.height = 3;
        accepted = acceptedRoom (side, step);
        check (SlabContacts_DeduplicateAccepted (accepted) && closeArea (contactArea (accepted), 8) &&
                   accepted.otdwall[0].openings.GetSize () == 1,
               "исходное отверстие контакта сохраняется при частичном вычитании");
        side.openings.Clear ();
        step = side;
        step.slab_step = true;
        step.openings = {acceptedHole};
        accepted = acceptedRoom (side, step);
        check (SlabContacts_DeduplicateAccepted (accepted) && closeArea (contactArea (accepted), 1),
               "отверстие принятой боковины не считается покрытием");
        step.openings.Clear ();
        step.endC = {0.00005, 0};
        accepted = acceptedRoom (side, step);
        check (SlabContacts_DeduplicateAccepted (accepted) && closeArea (contactArea (accepted), 12),
               "конечная боковина ниже порога writer не стирает контакт");
        step.begC = {0.00005, 0};
        step.endC = {4, 0};
        step.zBottom = 0;
        step.height = 3;
        accepted = acceptedRoom (side, step);
        check (!SlabContacts_DeduplicateAccepted (accepted) && closeArea (contactArea (accepted), 12),
               "непредставимый тонкий остаток сохраняется целиком и явно отвергается");
        step = side;
        step.slab_step = true;
        step.begC = {6, 0};
        step.endC = {8, 0};
        accepted = acceptedRoom (side, step);
        check (SlabContacts_DeduplicateAccepted (accepted) && closeArea (contactArea (accepted), 12),
               "конечное непересекающееся покрытие в той же плоскости не вычитается");
        step.begC = {3, 0};
        step.endC = {1, 0};
        accepted = acceptedRoom (side, step);
        check (SlabContacts_DeduplicateAccepted (accepted) && closeArea (contactArea (accepted), 6),
               "обратное направление линии не меняет конечное покрытие боковины");
        side.rise_y = 0.5;
        side.slant_angle = std::atan2 (1.0, 0.5);
        step = side;
        step.slab_step = true;
        step.height = 1;
        accepted = acceptedRoom (side, step);
        check (SlabContacts_DeduplicateAccepted (accepted) &&
                   closeArea (contactArea (accepted), 8 * std::sqrt (1.25)) &&
                   closeArea (accepted.otdwall[0].rise_y, 0.5) && closeArea (accepted.otdwall[0].begC.y, 0.5),
               "наклонный остаток остаётся в исходной плоскости и сохраняет подъём");
        // Отделка ступеней сохраняет классификацию пола/потолка; её индексируют до удаления устаревших элементов.
        check (Otd_IsNativeTypeForFinish (Floor, API_WallID) && Otd_IsNativeTypeForFinish (Ceil, API_WallID),
               "floor and ceiling native stepwalls enter existing finish index");
        check (Otd_IsNativeTypeForFinish (Floor, API_SlabID) && Otd_IsNativeTypeForFinish (Floor, API_ObjectID) &&
                   Otd_IsNativeTypeForFinish (Ceil, API_SlabID) && Otd_IsNativeTypeForFinish (Ceil, API_ObjectID),
               "floor and ceiling legacy native types remain accepted");
        check (!Otd_IsNativeTypeForFinish (Floor, API_BeamID) && !Otd_IsNativeTypeForFinish (Ceil, API_ColumnID) &&
                   !Otd_IsNativeTypeForFinish (Wall_Main, API_SlabID) &&
                   Otd_IsNativeTypeForFinish (Reveal_Main, API_BeamID),
               "unrelated native type guards remain unchanged");
        GS::HashTable<API_Guid, TypeOtd> existingStepwalls;
        UnicGuidByGuid stepwallParents;
        UnicGuid stepwallChildren;
        const API_Guid stepwallGuids[] = {APIGuidFromString ("F08C2CD0-7232-4416-8D64-2DCE440E90DE"),
                                          APIGuidFromString ("A3C9FE16-DDD9-4CA4-A00E-F4559D82C0AF"),
                                          APIGuidFromString ("92ED1CC0-57FC-437A-8EE0-A61A6B8193A0"),
                                          APIGuidFromString ("5E3ED48F-75CB-4146-A4F3-D76874F23728")};
        for (Int32 i = 0; i < 4; ++i) {
            const TypeOtd type = i < 2 ? Ceil : Floor;
            if (Otd_IsNativeTypeForFinish (type, API_WallID))
                existingStepwalls.Add (stepwallGuids[i], type);
            stepwallChildren.Add (stepwallGuids[i], true);
        }
        stepwallParents.Add (baseA, stepwallChildren);
        bool hasStepwallBase = false;
        const UnicGuidByBase indexedStepwalls = Otd_BuildByParent (existingStepwalls, stepwallParents, hasStepwallBase);
        const UnicGuidByTypeOtd *stepwallTypes = indexedStepwalls.GetPtr (baseA);
        const UnicGuid *floorStepwalls = stepwallTypes == nullptr ? nullptr : stepwallTypes->GetPtr (Floor);
        const UnicGuid *ceilingStepwalls = stepwallTypes == nullptr ? nullptr : stepwallTypes->GetPtr (Ceil);
        check (hasStepwallBase && floorStepwalls != nullptr && ceilingStepwalls != nullptr &&
                   floorStepwalls->GetSize () == 2 && ceilingStepwalls->GetSize () == 2 &&
                   ceilingStepwalls->ContainsKey (stepwallGuids[0]) &&
                   ceilingStepwalls->ContainsKey (stepwallGuids[1]) && floorStepwalls->ContainsKey (stepwallGuids[2]) &&
                   floorStepwalls->ContainsKey (stepwallGuids[3]),
               "all four classified stepwalls survive parent indexing for obsolete deletion");
        {
            const auto makeCleanupPolygon = [] (const GS::Array<Point2D> &coords,
                                                const GS::Array<UIndex> &ends,
                                                const GS::Array<PolyArcRec> &arcs) {
                return Geometry::Polygon2D::Create (Geometry::PolyFlags_IsSameDirEdgeLegal,
                                                    coords,
                                                    arcs,
                                                    ends,
                                                    Geometry::PolyCreateFlags_IdxRefInContourEndFromZero |
                                                        Geometry::PolyCreateFlags_IdxRefInArcFromZero)[0];
            };
            Geometry::Polygon2D empty;
            check (PolygonVertices_RemoveCollinear (empty) == 0 && empty.IsEmpty (), "polygon cleanup empty input");
            auto polygon = makeCleanupPolygon ({{2, 0}, {3, 0}, {4, 0}, {4, 3}, {0, 3}, {0, 0}, {1, 0}}, {6}, {});
            check (polygon.GetVertexNum () == 7 && PolygonVertices_RemoveCollinear (polygon) == 3 &&
                       polygon.GetVertexNum () == 4 && closeArea (polygon.CalcArea (), 12),
                   "polygon cleanup collinear chain and closed seam");
            check (PolygonVertices_RemoveCollinear (polygon) == 0 && polygon.GetVertexNum () == 4,
                   "polygon cleanup rectangle and repeated call unchanged");
            auto holed = makeCleanupPolygon (
                {{0, 0}, {4, 0}, {4, 3}, {0, 3}, {1, 1}, {1, 1.5}, {1, 2}, {2, 2}, {2, 1}}, {3, 8}, {});
            check (PolygonVertices_RemoveCollinear (holed) == 1 && holed.GetContourNum () == 2 &&
                       holed.GetVertexNum () == 8 && closeArea (holed.CalcArea (), 11),
                   "polygon cleanup preserves hole and orientation");
            PolyArcRec arc;
            arc.begIndex = 3;
            arc.endIndex = 4;
            arc.arcAngle = .5;
            auto curved = makeCleanupPolygon ({{0, 0}, {2, 0}, {4, 0}, {4, 3}, {0, 3}}, {4}, {arc});
            const double curvedArea = curved.CalcArea ();
            check (PolygonVertices_RemoveCollinear (curved) == 1 && curved.GetVertexNum () == 4 &&
                       closeArea (curved.CalcArea (), curvedArea),
                   "polygon cleanup preserves curved coverage");
        }
        TestFunc::FloorCoreChecks (check);
        ParamValue coreParam;
        coreParam.rawName = "{@material:floor_core_test}";
        ParamDictElement coreRead;
        ParamDictValue coreBase;
        coreBase.Add (coreParam.rawName, coreParam);
        coreRead.Add (baseA, coreBase);
        ParamComposite coreComposite;
        // Штатная читалка заполняет слои, не выставляя общий флаг isValid.
        coreComposite.isValid = false;
        coreComposite.composite_type = API_CompositeStructure;
        ParamValueComposite coreLayer;
        coreLayer.fillThick = .02;
        coreLayer.structype = APICWallComp_Finish;
        coreLayer.val = "l[1:paint]";
        coreComposite.composite.Push (coreLayer);
        coreLayer.fillThick = .03;
        coreLayer.structype = 0;
        coreLayer.val = "ignored";
        coreComposite.composite.Push (coreLayer);
        coreLayer.fillThick = .25;
        coreLayer.structype = APICWallComp_Core;
        coreComposite.composite.Push (coreLayer);
        coreLayer.fillThick = .04;
        coreLayer.structype = APICWallComp_Finish;
        coreLayer.val = "l[1:tile]";
        coreComposite.composite.Push (coreLayer);
        ParamDictComposite coreMap;
        coreMap.Add (coreParam.rawName, coreComposite);
        ParamDictCompositeElement coreComposites;
        coreComposites.Add (baseA, coreMap);
        GS::Array<ParamValueComposite> coreFiltered;
        GS::UniString coreFavorite;
        OtdWall coreSource;
        coreSource.base_type = API_WallID;
        check (Param_SetToBase (
                   baseA, false, coreFiltered, coreRead, coreComposites, coreParam, coreFavorite, &coreSource) &&
                   coreSource.floor_core_offset_valid && closeArea (coreSource.floor_core_offset, .05) &&
                   coreFiltered.GetSize () == 1 && closeArea (coreFiltered[0].fillThick, .02),
               "cached full composition depth independent of layer filtering");
        coreFiltered.Clear ();
        check (Param_SetToBase (
                   baseA, true, coreFiltered, coreRead, coreComposites, coreParam, coreFavorite, &coreSource) &&
                   coreSource.floor_core_offset_valid && closeArea (coreSource.floor_core_offset, .04) &&
                   coreFiltered.GetSize () == 1,
               "cached flipped composition depth read before filtering");
        const auto corePolygon = [] (bool hole) {
            GS::Array<Point2D> coords;
            for (const Point2D &p : {Point2D{0, 0}, Point2D{4, 0}, Point2D{4, 3}, Point2D{0, 3}})
                coords.Push (p);
            GS::Array<UIndex> ends;
            ends.Push (3);
            if (hole) {
                for (const Point2D &p : {Point2D{1, 1}, Point2D{1, 2}, Point2D{2, 2}, Point2D{2, 1}})
                    coords.Push (p);
                ends.Push (7);
            }
            GS::Array<PolyArcRec> arcs;
            return Geometry::Polygon2D::Create (
                coords, arcs, ends, Geometry::PolyCreateFlags_IdxRefInContourEndFromZero)[0];
        };
        const auto coreRoom = [&] (bool hole) {
            OtdRoom room;
            OtdSlab floor;
            floor.poly = corePolygon (hole);
            floor.type = Floor;
            floor.base_guid = baseB;
            floor.floorInd = 7;
            floor.favorite.name = "preserved";
            floor.otd_guid = segment;
            room.otdslab.Push (floor);
            floor.type = Ceil;
            room.otdslab.Push (floor);
            OtdWall wall;
            wall.begC = {0, 0};
            wall.endC = {4, 0};
            wall.height = 3;
            wall.type = Wall_Main;
            wall.base_guid = baseA;
            wall.base_type = API_WallID;
            wall.surface_geometry = true;
            wall.floor_core_offset_valid = true;
            wall.floor_core_offset = .1;
            room.otdwall.Push (wall);
            return room;
        };
        OtdOpening floorOpening;
        floorOpening.base_guid = baseA;
        floorOpening.objLoc = 2;
        floorOpening.width = 1;
        floorOpening.height = 2;
        floorOpening.base_reveal_width = .5;
        const auto openingPolygon = [] (const GS::Array<Point2D> &points) {
            return Geometry::Polygon2D::Create (points, 0)[0];
        };
        const auto sameCoverage = [] (const Geometry::Polygon2D &a, const Geometry::Polygon2D &b) {
            Geometry::MultiPolygon2D first, second;
            first.Push (a);
            second.Push (b);
            return Geometry::MultiPolygon2D::Substract (first, second).CalcArea () <= 1.e-6 &&
                   Geometry::MultiPolygon2D::Substract (second, first).CalcArea () <= 1.e-6;
        };
        const auto expectedEntrance =
            openingPolygon ({{0, 0}, {1.5, 0}, {1.5, -.5}, {2.5, -.5}, {2.5, 0}, {4, 0}, {4, 3}, {0, 3}});
        for (bool roomReveals : {false, true})
            for (bool openingReveals : {false, true}) {
                auto room = coreRoom (false);
                room.create_reveal_elements = roomReveals;
                floorOpening.create_reveal = openingReveals;
                const auto wall = room.otdwall[0];
                check (FloorContour_ExtendToOpening (room, wall, floorOpening) && room.otdslab.GetSize () == 2 &&
                           sameCoverage (room.otdslab[0].poly, expectedEntrance) &&
                           sameCoverage (room.otdslab[1].poly, corePolygon (false)),
                       "floor opening exact coverage independent of both reveal flags, ceiling unchanged");
                check (room.otdslab[0].base_guid == baseB && room.otdslab[0].otd_guid == segment &&
                           room.otdslab[0].favorite.name == "preserved" && room.otdslab[0].floorInd == 7 &&
                           closeArea (room.otdslab[0].zBottom, 0),
                       "floor opening preserves slab ownership favorite story and elevation");
                check (FloorContour_ExtendToOpening (room, wall, floorOpening) &&
                           sameCoverage (room.otdslab[0].poly, expectedEntrance),
                       "floor opening repeated extension does not grow beyond reveal depth");
            }
        {
            auto room = coreRoom (false);
            auto op = floorOpening;
            op.zBottom = -.2;
            check (FloorContour_ExtendToOpening (room, room.otdwall[0], op) &&
                       sameCoverage (room.otdslab[0].poly, expectedEntrance),
                   "floor opening below zone bottom is included");
            for (int invalid = 0; invalid < 7; ++invalid) {
                room = coreRoom (false);
                op = floorOpening;
                if (invalid == 0)
                    op.zBottom = .1;
                if (invalid == 1)
                    op.base_guid = APINULLGuid;
                if (invalid == 2)
                    op.base_reveal_width = 0;
                if (invalid == 3)
                    op.width = 0;
                if (invalid == 4)
                    op.height = 0;
                if (invalid == 5)
                    op.base_reveal_width = std::numeric_limits<double>::quiet_NaN ();
                if (invalid == 6)
                    op.zBottom = -op.height;
                check (FloorContour_ExtendToOpening (room, room.otdwall[0], op) &&
                           sameCoverage (room.otdslab[0].poly, corePolygon (false)),
                       "floor opening invalid above-zone or closed-at-floor input leaves original intact");
            }
        }
        {
            auto room = coreRoom (true);
            check (FloorContour_ExtendToOpening (room, room.otdwall[0], floorOpening) &&
                       room.otdslab[0].poly.GetContourNum () == 2 && closeArea (room.otdslab[0].poly.CalcArea (), 11.5),
                   "floor opening preserves unrelated hole");
            room = coreRoom (false);
            room.otdslab.Push (room.otdslab[0]);
            const auto distant = openingPolygon ({{10, 0}, {14, 0}, {14, 3}, {10, 3}});
            room.otdslab[0].poly = distant;
            check (FloorContour_ExtendToOpening (room, room.otdwall[0], floorOpening) &&
                       sameCoverage (room.otdslab[0].poly, distant) &&
                       sameCoverage (room.otdslab[2].poly, expectedEntrance) && room.otdslab.GetSize () == 3,
                   "floor opening extends contacting slab rather than first slab");
            room = coreRoom (false);
            room.otdslab.Push (room.otdslab[0]);
            room.otdslab[0].poly = openingPolygon ({{0, 0}, {2, 0}, {2, 3}, {0, 3}});
            room.otdslab[2].poly = openingPolygon ({{2, 0}, {4, 0}, {4, 3}, {2, 3}});
            check (FloorContour_ExtendToOpening (room, room.otdwall[0], floorOpening) &&
                       sameCoverage (room.otdslab[0].poly,
                                     openingPolygon ({{0, 0}, {1.5, 0}, {1.5, -.5}, {2, -.5}, {2, 3}, {0, 3}})) &&
                       sameCoverage (room.otdslab[2].poly,
                                     openingPolygon ({{2, -.5}, {2.5, -.5}, {2.5, 0}, {4, 0}, {4, 3}, {2, 3}})) &&
                       room.otdslab.GetSize () == 3,
                   "floor opening split slabs extend only their own finite spans without new slab");
            room = coreRoom (false);
            room.otdslab[0].zBottom = -1;
            check (FloorContour_ExtendToOpening (room, room.otdwall[0], floorOpening) &&
                       sameCoverage (room.otdslab[0].poly, corePolygon (false)),
                   "floor opening does not extend floor below opening bottom");
        }
        for (const Point2D &dir : {Point2D{0, 1}, Point2D{.6, .8}, Point2D{-.6, -.8}}) {
            auto room = coreRoom (false);
            const auto transform = [&] (const GS::Array<Point2D> &coords) {
                GS::Array<Point2D> rotated;
                for (const auto &p : coords)
                    rotated.Push ({10 + dir.x * p.x - dir.y * p.y, 20 + dir.y * p.x + dir.x * p.y});
                return openingPolygon (rotated);
            };
            room.otdslab[0].poly = transform ({{0, 0}, {4, 0}, {4, 3}, {0, 3}});
            room.otdwall[0].begC = {10, 20};
            room.otdwall[0].endC = {10 + dir.x * 4, 20 + dir.y * 4};
            check (FloorContour_ExtendToOpening (room, room.otdwall[0], floorOpening) &&
                       sameCoverage (
                           room.otdslab[0].poly,
                           transform ({{0, 0}, {1.5, 0}, {1.5, -.5}, {2.5, -.5}, {2.5, 0}, {4, 0}, {4, 3}, {0, 3}})),
                   "floor opening rotated reversed and oblique wall uses outward reveal direction");
        }
        auto core = coreRoom (false);
        {
            GS::Array<Point2D> coords = {{0, 0}, {2, 0}, {4, 0}, {4, 3}, {0, 3}};
            GS::Array<PolyArcRec> arcs;
            GS::Array<UIndex> ends = {4};
            core.otdslab[1].poly =
                Geometry::Polygon2D::Create (Geometry::PolyFlags_IsSameDirEdgeLegal,
                                             coords,
                                             arcs,
                                             ends,
                                             Geometry::PolyCreateFlags_IdxRefInContourEndFromZero)[0];
            check (core.otdslab[1].poly.GetVertexNum () == 5 && FloorContours_ExtendToCore (core) &&
                       core.otdslab[1].poly.GetVertexNum () == 4 && closeArea (core.otdslab[1].poly.CalcArea (), 12) &&
                       core.otdslab[1].base_guid == baseB && core.otdslab[1].otd_guid == segment &&
                       core.otdslab[1].floorInd == 7 && core.otdslab[1].favorite.name == "preserved",
                   "polygon cleanup ceiling removes redundant vertex without floor extension or link changes");
            check (FloorContours_ExtendToCore (core) && core.otdslab[1].poly.GetVertexNum () == 4 &&
                       closeArea (core.otdslab[1].poly.CalcArea (), 12),
                   "polygon cleanup ceiling repeated call unchanged");
        }
        core = coreRoom (false);
        check (FloorContours_ExtendToCore (core) && closeArea (core.otdslab[0].poly.CalcArea (), 12.4),
               "SDK finite floor union moves outward");
        check (closeArea (core.otdslab[1].poly.CalcArea (), 12) && core.otdwall[0].begC.x == 0 &&
                   core.otdwall[0].begC.y == 0 && core.otdwall[0].endC.x == 4 && core.otdwall[0].endC.y == 0 &&
                   core.otdwall[0].height == 3,
               "SDK ceiling and wall geometry unchanged");
        check (core.otdslab[0].base_guid == baseB && core.otdslab[0].otd_guid == segment &&
                   core.otdslab[0].floorInd == 7 && core.otdslab[0].favorite.name == "preserved",
               "SDK floor ownership favorite and existing link retained");
        check (FloorContours_ExtendToCore (core) && closeArea (core.otdslab[0].poly.CalcArea (), 12.4),
               "SDK repeated correction is stable");
        core = coreRoom (false);
        core.otdwall[0].begC.x = 1;
        core.otdwall[0].endC.x = 3;
        check (FloorContours_ExtendToCore (core) && closeArea (core.otdslab[0].poly.CalcArea (), 12.2),
               "SDK unmatched finite edge portions retained");
        core = coreRoom (false);
        auto cornerWall = core.otdwall[0];
        cornerWall.begC = {4, 0};
        cornerWall.endC = {4, 3};
        cornerWall.base_guid = segment;
        cornerWall.floor_core_offset = .2;
        core.otdwall.Push (cornerWall);
        check (FloorContours_ExtendToCore (core) && closeArea (core.otdslab[0].poly.CalcArea (), 13.02),
               "SDK unequal adjacent depths preserve exact corner");
        core = coreRoom (true);
        core.otdwall[0].begC = {1, 1};
        core.otdwall[0].endC = {1, 2};
        check (FloorContours_ExtendToCore (core) && closeArea (core.otdslab[0].poly.CalcArea (), 11.1) &&
                   core.otdslab[0].poly.GetContourNum () == 2,
               "SDK hole extends only matched side and remains a hole");
        core = coreRoom (false);
        core.otdwall[0].zBottom = .5;
        check (FloorContours_ExtendToCore (core) && closeArea (core.otdslab[0].poly.CalcArea (), 12),
               "SDK wall above floor excluded");
        core = coreRoom (false);
        core.otdwall[0].floor_core_offset_valid = false;
        check (FloorContours_ExtendToCore (core) && closeArea (core.otdslab[0].poly.CalcArea (), 12),
               "SDK undefined offset unchanged");
        core = coreRoom (false);
        auto coreConflict = core.otdwall[0];
        coreConflict.base_guid = segment;
        core.otdwall.Push (coreConflict);
        check (FloorContours_ExtendToCore (core) && closeArea (core.otdslab[0].poly.CalcArea (), 12),
               "SDK ambiguous base contact unchanged");
        core = coreRoom (false);
        OtdOpening door;
        door.objLoc = 2;
        door.width = 1;
        door.height = 2;
        core.otdwall[0].openings.Push (door);
        check (FloorContours_ExtendToCore (core) && closeArea (core.otdslab[0].poly.CalcArea (), 12.3),
               "SDK opening on floor excludes only its finite span");
        GS::Array<OtdWall> coreBands;
        OtdMaterial coreMaterial;
        coreMaterial.smaterial = "test";
        TypeOtd coreType = Wall_Main;
        check (OtdWall_Delim_One (core.otdwall[0],
                                  coreBands,
                                  1,
                                  0,
                                  coreType,
                                  coreMaterial,
                                  coreMaterial,
                                  coreMaterial,
                                  coreMaterial,
                                  coreMaterial,
                                  coreMaterial,
                                  coreMaterial,
                                  coreMaterial) &&
                   coreBands.GetSize () == 1 && coreBands[0].floor_core_offset_valid &&
                   closeArea (coreBands[0].floor_core_offset, .1),
               "height split carries independent offset");
    }

} // namespace TestFunc

#endif
