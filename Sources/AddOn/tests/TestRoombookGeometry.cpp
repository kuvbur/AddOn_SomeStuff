#include "ACAPinc.h"

#include "tests/TestRoombookGeometry.hpp"

#include "tests/ConnectedElementsCacheChecks.hpp"
#include "tests/FloorCoreChecks.hpp"
#include "tests/RoomAnalyticChecks.hpp"

#if defined(TESTING) && defined(ServerMainVers_2500)

    #include <cmath>
    #include <cstdlib>
    #include <limits>
    #include <vector>

    #include "CommonFunction.hpp"
    #include "ProfileAdditionalInfo.hpp"
    #include "ProfileVectorImage.hpp"
    #include "Propertycache.hpp"
    #include "roombook/Roombook.hpp"
    #include "roombook/RoomSurfaceCandidates.hpp"
    #include "roombook/RoomSurfaceFinish.hpp"
    #include "roombook/RoomSurfacePartition.hpp"
    #include "Sync.hpp"
    #include "VectorImageIterator.hpp"

namespace {

    GS::ObjectState Point (double x, double y, double z) { return GS::ObjectState ("x", x, "y", y, "z", z); }

    GS::ObjectState RunSurfaceTransformChecks () {
        GS::Array<GS::ObjectState> checks;
        Int32 failed = 0;
        const auto check = [&] (bool passed, const char *name) {
            checks.Push (GS::ObjectState ("name", name, "passed", passed));
            if (!passed)
                ++failed;
            DBtest (passed, "Z5 transform " + GS::UniString (name));
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
            check (Roombook::TransformRoomSurface (matrices[i], surface), names[i]);
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
        check (!Roombook::TransformRoomSurface (invalid, surface), "singular matrix rejected");
        invalid = matrices[0];
        invalid.tmx[3] = std::numeric_limits<double>::infinity ();
        surface = makeSurface ();
        check (!Roombook::TransformRoomSurface (invalid, surface), "nonfinite matrix rejected");
        surface = makeSurface ();
        surface.normalLocal = {};
        check (!Roombook::TransformRoomSurface (matrices[0], surface), "zero normal rejected");
        surface = makeSurface ();
        surface.contours[0].vertices[0].local.x = std::numeric_limits<double>::quiet_NaN ();
        check (!Roombook::TransformRoomSurface (matrices[0], surface), "nonfinite vertex rejected");
        surface = makeSurface ();
        surface.contours.Clear ();
        check (!Roombook::TransformRoomSurface (matrices[0], surface), "empty contours rejected");
        surface = makeSurface ();
        surface.contours[0].hole = true;
        check (!Roombook::TransformRoomSurface (matrices[0], surface), "missing outer contour rejected");
        surface = makeSurface ();
        surface.contours[1].vertices.Clear ();
        check (!Roombook::TransformRoomSurface (matrices[0], surface), "invalid hole rejected");
        return GS::ObjectState ("status", failed == 0 ? "Passed" : "Failed", "checks", checks, "failed", failed);
    }

    GS::ObjectState RunSurfacePartitionChecks () {
        using namespace Roombook;
        GS::Array<GS::ObjectState> checks;
        Int32 failed = 0;
        const auto check = [&] (bool passed, const char *name) {
            checks.Push (GS::ObjectState ("name", name, "passed", passed));
            if (!passed)
                ++failed;
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
            check (BuildPreparedWallSource (element, 10, {}, bases, prepared) && bases.GetSize () == 4 &&
                       prepared.origin == RoomSurfaceOrigin::PreparedAnalytic &&
                       prepared.snapshot.bodies[0].surfaces.GetSize () == 6,
                   "272 production ordinary wall preparation");
            bool ownIndices = true;
            for (const auto &face : prepared.snapshot.bodies[0].surfaces)
                ownIndices = ownIndices && face.bodyIndex == 0 && face.polygonIndex == 0 &&
                             face.origin == RoomSurfaceOrigin::PreparedAnalytic && face.analyticFaceId > 0;
            check (ownIndices, "272 analytic provenance has no invented native indices");
            OtdOpening opening;
            opening.base_guid = APIGuidFromString ("00000000-0000-0000-0000-000000000273");
            opening.objLoc = 2;
            opening.width = 2;
            opening.height = 1;
            opening.lower = 1;
            bases.Clear ();
            check (BuildPreparedWallSource (element, 10, {opening}, bases, prepared) &&
                       bases[0].openings.GetSize () == 1 && bases[0].openings[0].base_guid == opening.base_guid &&
                       bases[1].openings[0].base_guid == opening.base_guid,
                   "272 opening GUID retained on both prepared sides");
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
            const auto partition = PartitionRoomSurfaces (zoneSnapshot, {prepared});
            double matchedArea = 0, unmatchedArea = 0;
            for (const auto &piece : partition.pieces) {
                if (piece.status == RoomSurfacePieceStatus::Matched)
                    matchedArea += piece.area;
                if (piece.status == RoomSurfacePieceStatus::Unmatched)
                    unmatchedArea += piece.area;
            }
            check (partition.complete && std::abs (matchedArea - 10) < 1e-6 && std::abs (unmatchedArea - 2) < 1e-6,
                   "272 native zone partitions against finite prepared wall minus opening");
            GS::Array<RoomSurfacePiece> grouped;
            GS::Array<OtdWall> finishes;
            bool adapted = GroupSurfaceWallPieces (partition.pieces, 0, grouped);
            for (const auto &piece : grouped)
                if (piece.status == RoomSurfacePieceStatus::Matched)
                    adapted = BuildSurfaceWalls (piece, 0, finishes) && adapted;
            bool realOpening = false;
            for (auto &finish : finishes) {
                adapted = LinkSurfaceOpenings (finish, bases) && adapted;
                for (const auto &op : finish.openings)
                    realOpening = realOpening || op.base_guid == opening.base_guid;
            }
            check (adapted && realOpening, "272 prepared wall reaches writer adapter with real opening ID");

            const auto checkCut = [&] (const GS::Array<OtdOpening> &cuts, double expected, const char *name) {
                GS::Array<OtdWall> cutBases;
                RoomSurfaceSource cutSource;
                bool ok = BuildPreparedWallSource (element, 10, cuts, cutBases, cutSource);
                const auto cutPartition = PartitionRoomSurfaces (zoneSnapshot, {cutSource});
                double area = 0;
                for (const auto &piece : cutPartition.pieces)
                    if (piece.status == RoomSurfacePieceStatus::Matched)
                        area += piece.area;
                check (ok && cutPartition.complete && std::abs (area - expected) < 1e-6, name);
            };
            OtdOpening boundary = opening;
            boundary.lower = 0;
            boundary.height = 2;
            checkCut ({boundary}, 8, "272 production boundary opening");
            OtdOpening completeCut = opening;
            completeCut.width = 4;
            completeCut.height = 3;
            completeCut.lower = 0;
            checkCut ({completeCut}, 0, "272 production full opening cut");
            OtdOpening overlap = opening;
            overlap.base_guid = APIGuidFromString ("00000000-0000-0000-0000-000000000276");
            overlap.objLoc = 3;
            overlap.lower = .5;
            overlap.height = 2;
            checkCut ({opening, overlap}, 7, "272 production overlapping opening union");
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
            check (BuildPreparedWallSource (element, 10, {opening, overlap}, overlapBases, overlapSource) &&
                       LinkSurfaceOpenings (overlapCut, overlapBases, true),
                   "272 overlap links without choosing fake owner");
            bool bothIds = false;
            for (const auto &cut : overlapCut.openings)
                bothIds =
                    bothIds || (cut.base_guid == APINULLGuid && cut.source_opening_guids.Contains (opening.base_guid) &&
                                cut.source_opening_guids.Contains (overlap.base_guid));
            check (bothIds, "272 overlap cell retains both real opening IDs");
            std::swap (element.wall.begC, element.wall.endC);
            bases.Clear ();
            check (BuildPreparedWallSource (element, 10, {}, bases, prepared) &&
                       std::abs (bases[0].begC.y + .1) < 1e-12,
                   "272 production reversed axis side");
            std::swap (element.wall.begC, element.wall.endC);
            element.wall.flipped = true;
            bases.Clear ();
            check (BuildPreparedWallSource (element, 10, {opening}, bases, prepared) &&
                       std::abs (bases[0].begC.y + .1) < 1e-12,
                   "272 production flipped wall side");
            element.wall.relativeTopStory = 1;
            bases.Clear ();
            check (BuildPreparedWallSource (element, 10, {opening}, bases, prepared) && bases.GetSize () == 4 &&
                       std::abs (bases[0].height - element.wall.height) < 1e-12,
                   "272 top-linked wall retains resolved API height without native fallback");
            element.wall.relativeTopStory = 0;
            element.wall.modelElemStructureType = API_ProfileStructure;
            bases.Clear ();
            check (!BuildPreparedWallSource (element, 10, {}, bases, prepared) && bases.IsEmpty () &&
                       !prepared.snapshot.IsComplete (),
                   "272 profile explicitly requests cached native fallback");

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
                       BuildPreparedSlabSource (element, 10, polygons[0], slabBases, prepared) &&
                       slabBases.GetSize () == 1 && std::abs (slabBases[0].zBottom - 9.9) < 1e-12 &&
                       std::abs (slabBases[0].reference_z - 10) < 1e-12 &&
                       std::abs (slabBases[0].poly.CalcArea () - 11) < 1e-6,
                   "272 production slab reference levels and original hole");
            bool hole = false;
            for (const auto &face : prepared.snapshot.bodies[0].surfaces)
                hole = hole || (face.normalWorld.z != 0 && face.contours.GetSize () == 2 && face.contours[1].hole);
            check (hole, "272 slab hole survives analytic contact surfaces");

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
            nominal.material.smaterial = "272 material";
            GS::Array<OtdSlab> projectedBases;
            double rejected = 0;
            check (BuildProjectedSlabFinishes (floorSnapshot,
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
                   "272 active prepared slab route preserves hole elevation material and by_slab");
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
                        check (BuildPreparedSlabSource (candidate, top, full[0], bases, source),
                               "272 ordering regression prepared candidate");
                        sources.Push (source);
                        roles.Push (candidate.header.guid);
                    }
                    auto footprint = floorSnapshot;
                    footprint.bodies[0].surfaces[0].normalWorld.z = type == Floor ? -1 : 1;
                    for (bool reverse : {false, true}) {
                        const GS::Array<API_Guid> ordered = reverse ? GS::Array<API_Guid>{roles[1], roles[0]} : roles;
                        GS::Array<OtdSlab> finishes;
                        double rejectedArea = 0;
                        check (BuildProjectedSlabFinishes (footprint,
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
                               type == Floor
                                   ? "272 highest floor wins in both candidate orders"
                                   : "272 nearest ceiling above floor wins independent of zone top and order");
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
            check (curved.GetSize () == 1 && BuildPreparedSlabSource (element, 10, curved[0], slabBases, prepared),
                   "272 curved slab analytic preparation");
            if (!slabBases.IsEmpty ()) {
                GS::Array<Point2D> copied;
                GS::Array<PolyArcRec> copiedArcs;
                GS::Array<UIndex> copiedEnds;
                GS::Array<Geometry::Polygon2D::ConstContourIterator> reversed;
                slabBases[0].poly.CopyPolygonData (
                    copied, copiedArcs, copiedEnds, reversed, Geometry::PolyCreateFlags_IdxRefInContourEndFromZero);
                check (!copiedArcs.IsEmpty (), "272 immutable slab base retains arcs for projection and writer");
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
        auto result = PartitionRoomSurfaces (zone, sources);
        check (result.complete && result.pieces.GetSize () == 1 &&
                   closeArea (area (result, RoomSurfacePieceStatus::Unmatched), 12),
               "no candidates preserve the whole face");
        sources.Push (source (baseA, rectangle (0, 2, 0, 3, -1)));
        sources.Push (source (baseB, rectangle (2, 4, 0, 3, -1), API_BeamID));
        result = PartitionRoomSurfaces (zone, sources);
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
        const auto reordered = PartitionRoomSurfaces (zone, reversed);
        bool same = reordered.complete && result.pieces.GetSize () == reordered.pieces.GetSize ();
        for (UIndex i = 0; same && i < result.pieces.GetSize (); ++i)
            same = result.pieces[i].baseGuid == reordered.pieces[i].baseGuid &&
                   closeArea (result.pieces[i].area, reordered.pieces[i].area);
        check (same, "candidate order does not change ownership or area");
        sources[0] = source (baseA, rectangle (0, 3, 0, 3, -1));
        sources[1] = source (baseB, rectangle (1, 4, 0, 3, -1));
        result = PartitionRoomSurfaces (zone, sources);
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
        result = PartitionRoomSurfaces (zone, sources);
        check (result.complete && closeArea (area (result, RoomSurfacePieceStatus::Matched), 3) &&
                   closeArea (area (result, RoomSurfacePieceStatus::Unmatched), 9),
               "partial contact retains unmatched remainder");
        sources[0] = source (baseA, rectangle (6, 7, 0, 3, -1));
        result = PartitionRoomSurfaces (zone, sources);
        check (result.complete && closeArea (area (result, RoomSurfacePieceStatus::Unmatched), 12),
               "coplanar distant face is not a contact");
        sources[0] = source (baseA, rectangle (4, 5, 0, 3, -1));
        result = PartitionRoomSurfaces (zone, sources);
        check (result.complete && closeArea (area (result, RoomSurfacePieceStatus::Unmatched), 12),
               "edge-only contact has no area");
        sources[0] = source (baseA, rectangle (0, 4, 0, 3, 1));
        result = PartitionRoomSurfaces (zone, sources);
        check (result.complete && closeArea (area (result, RoomSurfacePieceStatus::Matched), 12),
               "coplanar same-normal face inside the room covers the finish");
        auto distant = rectangle (0, 4, 0, 3, -1);
        for (auto &vertex : distant.contours[0].vertices) {
            vertex.world.z = 2e-4;
            vertex.local.z = 2e-4;
        }
        distant.planeOffset = 2e-4;
        sources[0] = source (baseA, distant);
        result = PartitionRoomSurfaces (zone, sources);
        check (result.complete && closeArea (area (result, RoomSurfacePieceStatus::Unmatched), 12),
               "plane distance beyond tolerance is not accepted");
        sources[0] = source (baseA, rectangle (0, 4, 0, 3, -1));
        sources.Push (sources[0]);
        result = PartitionRoomSurfaces (zone, sources);
        check (result.complete && result.pieces.GetSize () == 1 && result.pieces[0].sources.GetSize () == 1 &&
                   closeArea (area (result, RoomSurfacePieceStatus::Matched), 12),
               "duplicate source does not duplicate its claim");
        sources.Clear ();
        sources.Push (source (baseA, rectangle (0, 2, 0, 3, -1), API_ColumnID));
        auto second = source (baseA, rectangle (2, 4, 0, 3, -1), API_ColumnID);
        second.geometryGuid = segment;
        second.snapshot.guid = segment;
        sources.Push (second);
        result = PartitionRoomSurfaces (zone, sources);
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
        result = PartitionRoomSurfaces (snapshot (zoneGuid, holedFace), sources);
        check (result.complete && result.pieces.GetSize () == 1 && result.pieces[0].contours.GetSize () == 2 &&
                   result.pieces[0].contours[1].hole && closeArea (area (result, RoomSurfacePieceStatus::Matched), 12),
               "zone hole is preserved in matched contour");
        auto holedSource = holedFace;
        holedSource.normalLocal = {0, 0, -1};
        holedSource.normalWorld = holedSource.normalLocal;
        sources[0] = source (baseA, holedSource);
        result = PartitionRoomSurfaces (snapshot (zoneGuid, rectangle (0, 4, 0, 4, 1)), sources);
        check (result.complete && closeArea (area (result, RoomSurfacePieceStatus::Matched), 12) &&
                   closeArea (area (result, RoomSurfacePieceStatus::Unmatched), 4),
               "source hole stays unmatched");
        auto triangular = rectangle (0, 4, 0, 3, -1);
        triangular.contours[0].vertices.Delete (2);
        sources[0] = source (baseA, triangular);
        result = PartitionRoomSurfaces (zone, sources);
        check (result.complete && closeArea (area (result, RoomSurfacePieceStatus::Matched), 6) &&
                   closeArea (area (result, RoomSurfacePieceStatus::Unmatched), 6),
               "finite triangular contact is clipped, not its bounds");
        auto collinear = rectangle (0, 4, 0, 3, -1);
        RoomSurfaceVertex mid;
        mid.local = {2, 0, 0};
        mid.world = mid.local;
        collinear.contours[0].vertices.Insert (1, mid);
        sources[0] = source (baseA, collinear);
        result = PartitionRoomSurfaces (zone, sources);
        check (result.complete && closeArea (area (result, RoomSurfacePieceStatus::Matched), 12),
               "collinear extra vertex does not invalidate contact");
        const API_Tranmat shear = {{1, 0, 0, 10, 0, 1, 0, -3, 1, 0, 1, 7}};
        auto slantedZone = rectangle (0, 4, 0, 3, 1), slantedSource = rectangle (0, 4, 0, 3, -1);
        check (TransformRoomSurface (shear, slantedZone) && TransformRoomSurface (shear, slantedSource),
               "slanted fixture transformed by the shared reader helper");
        sources[0] = source (baseA, slantedSource);
        result = PartitionRoomSurfaces (snapshot (zoneGuid, slantedZone), sources);
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
        result = PartitionRoomSurfaces (zone, sources);
        check (result.complete && result.pieces.GetSize () == 1 &&
                   closeArea (area (result, RoomSurfacePieceStatus::Matched), 12),
               "skewed stored normal of a planar face is accepted");
        auto nearPlane = rectangle (0, 4, 0, 3, -1);
        nearPlane.contours[0].vertices[2].world.z = 2.0e-5;
        nearPlane.contours[0].vertices[2].local.z = 2.0e-5;
        sources[0] = source (baseA, nearPlane);
        result = PartitionRoomSurfaces (zone, sources);
        check (result.complete && result.errors.IsEmpty () &&
                   closeArea (area (result, RoomSurfacePieceStatus::Matched), 12),
               "twenty micron source-plane noise is accepted within 0.1 mm");
        // По-настоящему неплоская грань остаётся ошибкой: подъём вершины за допуск не принимается.
        auto warped = rectangle (0, 4, 0, 3, -1);
        warped.contours[0].vertices[2].world.z = 1.0e-3;
        warped.contours[0].vertices[2].local.z = 1.0e-3;
        sources[0] = source (baseA, warped);
        result = PartitionRoomSurfaces (zone, sources);
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
        result = PartitionRoomSurfaces (zone, sources);
        check (result.complete && result.errors.IsEmpty () &&
                   closeArea (area (result, RoomSurfacePieceStatus::Unmatched), 12),
               "tiny planar source does not abort unrelated zone partition");
        sources[0].snapshot.status = RoomSurfaceStatus::ReadError;
        result = PartitionRoomSurfaces (zone, sources);
        check (!result.complete && !result.errors.IsEmpty (), "incomplete source cannot produce a complete partition");
        auto badZone = zone;
        badZone.status = RoomSurfaceStatus::ReadError;
        result = PartitionRoomSurfaces (badZone, sources);
        check (!result.complete && result.pieces.IsEmpty () && !result.errors.IsEmpty (), "incomplete zone rejected");
        sources.Clear ();
        sources.Push (source (baseA, rectangle (0, 4, 0, 3, -1)));
        auto conflicting = sources[0];
        conflicting.snapshot.bodies[0].surfaces[0].contours[0].vertices[0].world.x = 0.25;
        sources.Push (conflicting);
        result = PartitionRoomSurfaces (zone, sources);
        check (!result.complete && !result.errors.IsEmpty (), "conflicting snapshots with the same face key rejected");
        sources.Clear ();
        sources.Push (source (baseA, rectangle (0, 4, 0, 3, -1)));
        sources[0].snapshot.bodies[0].surfaces[0].normalWorld = {};
        result = PartitionRoomSurfaces (zone, sources);
        check (!result.complete && !result.errors.IsEmpty (), "invalid source normal cannot silently become unmatched");
        sources[0] = source (baseA, rectangle (0, 4, 0, 3, -1));
        badZone = zone;
        badZone.bodies[0].surfaces[0].planeOffset = 1;
        result = PartitionRoomSurfaces (badZone, sources);
        check (!result.complete && !result.errors.IsEmpty (),
               "zone plane equation inconsistent with its vertices rejected");
        sources[0].snapshot.bodies[0].surfaces[0].contours[0].vertices[0].world.x =
            std::numeric_limits<double>::quiet_NaN ();
        result = PartitionRoomSurfaces (zone, sources);
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
            BuildRoomBeamFinish (sectionSource, beamBeg, beamEnd, 0, 80, 0, 3, sectionResult, &steppedZone);
        check (sectionResult.errors.IsEmpty () &&
                   closeArea (beamArea (sectionFinish, RoomSurfaceBeamFace::Bottom), 9 * 0.3) &&
                   closeArea (beamArea (sectionFinish, RoomSurfaceBeamFace::LeftSide), 9 * 0.5 + 9.5 * 0.12) &&
                   closeArea (beamArea (sectionFinish, RoomSurfaceBeamFace::RightSide), 9 * 0.5 + 9.5 * 0.12),
               "3D zone section extends beam bottom and both sides beyond lower-profile beamPart");
        steppedZone.bodies[0].surfaces.Delete (0);
        sectionResult.errors.Clear ();
        const auto brokenSection =
            BuildRoomBeamFinish (sectionSource, beamBeg, beamEnd, 0, 80, 0, 3, sectionResult, &steppedZone);
        check (brokenSection.pieces.IsEmpty () && !sectionResult.errors.IsEmpty (),
               "open zone section is rejected without partial beam finish");
        auto beam = BuildRoomBeamFinish (beamSource, beamBeg, beamEnd, 0, 100, 0, 3, beamResult);
        check (beam.pieces.GetSize () == 3 && closeArea (beamArea (beam, RoomSurfaceBeamFace::Bottom), 10 * 0.3) &&
                   closeArea (beamArea (beam, RoomSurfaceBeamFace::LeftSide), 10 * 0.62) &&
                   closeArea (beamArea (beam, RoomSurfaceBeamFace::RightSide), 10 * 0.62),
               "full beam part yields bottom and both sides");
        beam = BuildRoomBeamFinish (beamSource, beamBeg, beamEnd, 25, 75, 0, 3, beamResult);
        check (beam.pieces.GetSize () == 3 && closeArea (beamArea (beam, RoomSurfaceBeamFace::Bottom), 5 * 0.3) &&
                   closeArea (beamArea (beam, RoomSurfaceBeamFace::LeftSide), 5 * 0.62),
               "tBeg/tEnd clip bottom and sides");
        beam = BuildRoomBeamFinish (beamSource, beamBeg, beamEnd, 0, 100, 0, 2.31, beamResult);
        check (beam.pieces.GetSize () == 3 && closeArea (beamArea (beam, RoomSurfaceBeamFace::LeftSide), 10 * 0.31) &&
                   closeArea (beamArea (beam, RoomSurfaceBeamFace::Bottom), 3),
               "zTop clip keeps the bottom intact");
        beam = BuildRoomBeamFinish (beamSource, beamBeg, beamEnd, 0, 100, 2.31, 3, beamResult);
        check (beam.pieces.GetSize () == 2 && closeArea (beamArea (beam, RoomSurfaceBeamFace::LeftSide), 10 * 0.31),
               "zBottom above the beam drops its bottom face");
        beam = BuildRoomBeamFinish (beamSource, beamBeg, beamEnd, 50, 50, 0, 3, beamResult);
        check (beam.pieces.IsEmpty () && !beamResult.errors.IsEmpty (), "empty t-range is an explicit error");
        beam = BuildRoomBeamFinish (beamSource, beamBeg, beamEnd, 25, 75, 0, 3, beamResult);
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
        check (validVertices && beam.pieces.GetSize () == 3,
               "beam vertices have the reported nonzero area and t bounds");
        check (closeArea (beam.tBeg, 25) && closeArea (beam.tEnd, 75), "beam result preserves raw SDK part bounds");
        for (auto &surface : beamSource.snapshot.bodies[0].surfaces)
            for (auto &contour : surface.contours)
                for (auto &vertex : contour.vertices)
                    vertex.world.x = 2 + vertex.world.x * 0.6;
        beam = BuildRoomBeamFinish (beamSource, beamBeg, beamEnd, 0, 100, 0, 3, beamResult);
        check (beam.pieces.GetSize () == 3 && closeArea (beamArea (beam, RoomSurfaceBeamFace::Bottom), 6 * 0.3) &&
                   closeArea (beamArea (beam, RoomSurfaceBeamFace::LeftSide), 6 * 0.62),
               "segment face bounds clip bottom and sides within the parent beam");
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
        beam = BuildRoomBeamFinish (beamSource,
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
        check (BuildSurfaceWall (contact, 0, surfaceWall) && surfaceWall.type == Column &&
                   surfaceWall.base_guid == baseA && surfaceWall.surface_geometry &&
                   closeArea (OtdWall_GetArea (surfaceWall), 12),
               "surface adapter vertical column area and provenance");
        check (surfaceWall.endC.x < surfaceWall.begC.x && surfaceWall.openings.IsEmpty (),
               "Inside baseline points left-side thickness inward and has no double openings");
        contact.area = 12 + 2e-6;
        check (BuildSurfaceWall (contact, 0, surfaceWall),
               "surface adapter accepts area noise within unchanged point tolerance");
        contact.area = 12.01;
        check (!BuildSurfaceWall (contact, 0, surfaceWall), "surface adapter rejects meaningful area mismatch");
        contact.contours[0].vertices[2].y = 1.5;
        contact.contours[0].vertices[3].y = 1.5;
        contact.normal = {0, 1 / std::sqrt (1.25), -0.5 / std::sqrt (1.25)};
        contact.area = 12 * std::sqrt (1.25);
        check (BuildSurfaceWall (contact, 0, surfaceWall) && closeArea (surfaceWall.rise_y, 0.5) &&
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
        check (BuildSurfaceWall (contact, 0, surfaceWall) && closeArea (surfaceWall.rise_y, -0.5) &&
                   surfaceWall.slant_angle < 90 * DEGRAD,
               "surface adapter opposite lean sign");
        contact.contours[0].vertices[2].x = 3;
        check (!BuildSurfaceWall (contact, 0, surfaceWall),
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
        check (BuildSurfaceWalls (notched, 0, notchedWalls) && notchedWalls.GetSize () == 1 &&
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
        check (BuildSurfaceWalls (notched, 0, notchedWalls) && notchedWalls.GetSize () == 1 &&
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
        check (LinkSurfaceOpenings (linkedWall, {sourceOpeningWall}) && linkedWall.openings.GetSize () == 1 &&
                   linkedWall.openings[0].base_guid == segment && linkedWall.openings[0].surface_geometry &&
                   !linkedWall.openings[0].has_reveal && closeArea (OtdWall_GetArea (linkedWall), 11),
               "surface opening retains the real opening guid without duplicate reveals");
        auto foreignOpeningWall = sourceOpeningWall;
        foreignOpeningWall.base_guid = baseB;
        linkedWall = notchedWalls[0];
        check (LinkSurfaceOpenings (linkedWall, {foreignOpeningWall}) &&
                   linkedWall.openings[0].base_guid == APINULLGuid,
               "geometric cutout is not linked to an opening of another base");
        auto overlappingOpeningWall = sourceOpeningWall;
        overlappingOpeningWall.openings[0].base_guid = baseB;
        linkedWall = notchedWalls[0];
        check (!LinkSurfaceOpenings (linkedWall, {sourceOpeningWall, overlappingOpeningWall}) &&
                   linkedWall.openings.GetSize () == 1 && linkedWall.openings[0].base_guid == APINULLGuid &&
                   closeArea (OtdWall_GetArea (linkedWall), 11),
               "ambiguous real opening ownership leaves cutouts unchanged");
        linkedWall = notchedWalls[0];
        check (LinkSurfaceOpenings (linkedWall, {sourceOpeningWall, sourceOpeningWall}) &&
                   linkedWall.openings.GetSize () == 1 && linkedWall.openings[0].base_guid == segment,
               "duplicate legacy records do not duplicate a real opening");
        sourceOpeningWall.openings[0].width = 0.5;
        linkedWall = notchedWalls[0];
        bool mixedLinked = LinkSurfaceOpenings (linkedWall, {sourceOpeningWall});
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
        check (LinkSurfaceOpenings (linkedWall, {sourceOpeningWall}) && linkedWall.openings.GetSize () == 2 &&
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
        check (BuildSurfaceWalls (multiple, 0, multipleWalls) && multipleWalls.GetSize () == 1 &&
                   multipleWalls[0].openings.GetSize () == 2 && closeArea (wallArea (multipleWalls), 10),
               "two holes preserve one wall and subtract both areas");
        for (auto &contour : multiple.contours)
            for (auto &vertex : contour.vertices)
                vertex.x = -vertex.x;
        multipleWalls.Clear ();
        check (BuildSurfaceWalls (multiple, 0, multipleWalls) && multipleWalls.GetSize () == 1 &&
                   multipleWalls[0].openings.GetSize () == 2 && closeArea (multipleWalls[0].openings[0].objLoc, 0.75) &&
                   closeArea (multipleWalls[0].openings[1].objLoc, 2) && closeArea (wallArea (multipleWalls), 10),
               "mirrored contour uses final wall direction for openings");
        RoomSurfacePiece tinyHole = notched;
        for (auto &vertex : tinyHole.contours[1].vertices)
            vertex.x = vertex.x > 2 ? 1.5 + min_dim / 2 : 1.5;
        tinyHole.area = 12 - min_dim / 2;
        GS::Array<OtdWall> rejectedWalls;
        check (!BuildSurfaceWalls (tinyHole, 0, rejectedWalls) && rejectedWalls.IsEmpty (),
               "unrepresentable small cutout does not fill the void");
        for (auto &contour : notched.contours)
            for (auto &vertex : contour.vertices)
                vertex.y = 0.5 * vertex.z;
        notched.normal = {0, 1 / std::sqrt (1.25), -0.5 / std::sqrt (1.25)};
        notched.area = 11 * std::sqrt (1.25);
        notchedWalls.Clear ();
        check (BuildSurfaceWalls (notched, 0, notchedWalls) && notchedWalls.GetSize () == 1 &&
                   closeArea (wallArea (notchedWalls), notched.area) && closeArea (notchedWalls[0].rise_y, 0.5),
               "slanted holed contact preserves true area and source plane");
        notched.contours[0].vertices[2].x = 3;
        const USize previousWalls = notchedWalls.GetSize ();
        check (!BuildSurfaceWalls (notched, 0, notchedWalls) && notchedWalls.GetSize () == previousWalls,
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
            if (!GroupSurfaceWallPieces (pieces, 0, grouped))
                return false;
            for (const auto &piece : grouped)
                if (!BuildSurfaceWalls (piece, 0, walls))
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
            BuildSurfaceWalls (piece, 0, separateWalls);
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
            GroupSurfaceWallPieces ({rectanglePiece (0, 4, 0, 3), diagonalTail}, 0, unsupportedGrouped);
        check (wholeContact && unsupportedGrouped.GetSize () == 1 && closeArea (unsupportedGrouped[0].area, 12.75) &&
                   !BuildSurfaceWalls (unsupportedGrouped[0], 0, unsupportedWalls) && unsupportedWalls.IsEmpty (),
               "same source diagonal tail is not dropped before representation check");
        GS::Array<RoomSurfacePiece> repeatedUnsupported;
        check (wholeContact && GroupSurfaceWallPieces (unsupportedGrouped, 0, repeatedUnsupported) &&
                   repeatedUnsupported.GetSize () == 1 &&
                   !BuildSurfaceWalls (repeatedUnsupported[0], 0, unsupportedWalls) && unsupportedWalls.IsEmpty (),
               "unsupported whole contour has the same outcome as its fragmented input");
        GS::Array<RoomSurfacePiece> pair = {rectanglePiece (0, 1, 0, 3), rectanglePiece (1, 2, 0, 3)};
        pair[1].planeOffset = -1.1e-7;
        for (auto &vertex : pair[1].contours[0].vertices)
            vertex.y = 1.1e-7;
        GS::Array<RoomSurfacePiece> nearGrouped;
        check (GroupSurfaceWallPieces (pair, 0, nearGrouped) && nearGrouped.GetSize () == 1 &&
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
            boundaries = boundaries && GroupSurfaceWallPieces (pair, 0, separate) && separate.GetSize () == 2;
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
        if (GroupSurfaceWallPieces (pair, 0, holeGrouped))
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
        check (BuildSurfaceSlabs (contact, 0, surfaceSlabs) && surfaceSlabs.GetSize () == 1 &&
                   surfaceSlabs[0].type == Ceil && closeArea (surfaceSlabs[0].poly.CalcArea (), 15) &&
                   surfaceSlabs[0].base_type == API_BeamID,
               "horizontal adapter preserves ceiling hole and beam parent");
        check (!BuildSurfaceWall (contact, 0, surfaceWall), "holed horizontal surface is not a wall");
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
                return ClampSurfaceWallToParts (wall, {0, 0}, {4, 0}, parts, edges, 0, peers);
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
            const auto vertical = ClampSurfaceWallToParts (wall, {4, 0}, {0, 0}, {wallPart (.5, 2)}, edges, 0, {});
            check (vertical.GetSize () == 1 && closeArea (vertical[0].begC.x, 3.5) && closeArea (vertical[0].endC.x, 2),
                   "wallPart measures distances from reversed base start");
            wall.begC = {4, 2.8};
            wall.endC = {4, .2};
            const GS::Array<Sector> cornerEdges = {{{4, 0}, {4, 3}}};
            result = ClampSurfaceWallToParts (wall, {4, 0}, {4, 3}, {wallPart (0, 3)}, cornerEdges, 0, {});
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
        check (BuildProjectedSlabFinishes (floorZone, slabSources, roles, Floor, 0, projected, rejected) &&
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
        check (BuildProjectedSlabFinishes (ceilingZone, slabSources, roles, Ceil, 0, projected, rejected) &&
                   projected.GetSize () == 1 && closeArea (projected[0].zBottom, 3.4) &&
                   projected[0].base_guid == baseB && projected[0].type == Ceil,
               "discovered ceiling across gap uses actual lower slab face");
        roles = {baseA};
        projected.Clear ();
        check (!BuildProjectedSlabFinishes (ceilingZone, slabSources, roles, Ceil, 0, projected, rejected) &&
                   projected.IsEmpty (),
               "missing role snapshot fails atomically");
        slabSources = {source (baseA, atLevel (rectangle (0, 2, 0, 3, 1), -0.2), API_SlabID),
                       source (baseB, atLevel (rectangle (2, 4, 0, 3, 1), -0.4), API_SlabID)};
        roles = {baseA, baseB, baseA};
        projected.Clear ();
        check (BuildProjectedSlabFinishes (floorZone, slabSources, roles, Floor, 0, projected, rejected) &&
                   projected.GetSize () == 2 && closeArea (slabArea (projected), 12) && rejected == 0,
               "adjacent different slab levels split room coverage without duplicate roles");
        GS::Array<OtdSlab> reversedSlabs;
        GS::Array<RoomSurfaceSource> reverseSources = {slabSources[1], slabSources[0]};
        double reverseRejected = 0;
        bool stable =
            BuildProjectedSlabFinishes (floorZone, reverseSources, roles, Floor, 0, reversedSlabs, reverseRejected) &&
            reversedSlabs.GetSize () == projected.GetSize ();
        for (UIndex i = 0; stable && i < projected.GetSize (); ++i)
            stable = projected[i].base_guid == reversedSlabs[i].base_guid &&
                     closeArea (projected[i].zBottom, reversedSlabs[i].zBottom) &&
                     closeArea (projected[i].poly.CalcArea (), reversedSlabs[i].poly.CalcArea ());
        check (stable, "projected ownership is independent of candidate order");
        slabSources[1] = source (baseB, atLevel (rectangle (1, 4, 0, 3, 1), -0.4), API_SlabID);
        projected.Clear ();
        check (BuildProjectedSlabFinishes (floorZone, slabSources, roles, Floor, 0, projected, rejected) &&
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
                check (BuildProjectedSlabFinishes (
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
        check (BuildProjectedSlabFinishes (floorZone, slabSources, roles, Floor, 0, projected, rejected) &&
                   closeArea (slabArea (projected), 11) && rejected == 0,
               "projected slab opening remains empty without zone fallback");
        floorZone.bodies.Push (floorZone.bodies[0]);
        projected.Clear ();
        check (BuildProjectedSlabFinishes (floorZone, slabSources, roles, Floor, 0, projected, rejected) &&
                   closeArea (slabArea (projected), 11),
               "duplicate room bodies do not duplicate slab finishes");
        auto conflictingFace = atLevel (rectangle (0, 4, 0, 3, 1), -0.6);
        conflictingFace.polygonIndex = 2;
        slabSources[0].snapshot.bodies[0].surfaces.Push (conflictingFace);
        projected.Clear ();
        rejected = 0;
        check (BuildProjectedSlabFinishes (floorZone, slabSources, roles, Floor, 0, projected, rejected) &&
                   closeArea (slabArea (projected), 11) && closeArea (rejected, 0) && projected.GetSize () == 1 &&
                   closeArea (projected[0].zBottom, -0.2),
               "same slab levels retain higher face and exclude its holes from lower coverage");
        slabSources = {source (baseA, atLevel (rectangle (5, 6, 0, 3, 1), -0.2), API_SlabID)};
        projected.Clear ();
        rejected = 0;
        check (BuildProjectedSlabFinishes (floorZone, slabSources, roles, Floor, 0, projected, rejected) &&
                   projected.IsEmpty () && rejected == 0,
               "discovered slab outside room projection creates no finish and no fallback");
        OtdSlab nominal;
        nominal.zBottom = 0;
        GS::Array<OtdWall> steps;
        slabSources.Clear ();
        roles.Clear ();
        projected.Clear ();
        check (BuildProjectedSlabFinishes (
                   floorZone, slabSources, roles, Floor, 2, projected, rejected, &nominal, false, &steps) &&
                   projected.GetSize () == 1 && closeArea (slabArea (projected), 12) &&
                   projected[0].base_guid == APINULLGuid && projected[0].floorInd == 2 && projected[0].zBottom == 0 &&
                   projected[0].surface_geometry && steps.IsEmpty (),
               "zero-source floor uses nominal residual when by-slab is disabled");
        projected.Clear ();
        check (BuildProjectedSlabFinishes (
                   floorZone, slabSources, roles, Floor, 0, projected, rejected, &nominal, true, &steps) &&
                   projected.IsEmpty (),
               "zero-source floor by-slab suppresses residual");
        nominal.zBottom = 3.1;
        projected.Clear ();
        check (BuildProjectedSlabFinishes (
                   ceilingZone, slabSources, roles, Ceil, 0, projected, rejected, &nominal, false, &steps) &&
                   projected.GetSize () == 1 && closeArea (projected[0].zBottom, 3.1) && projected[0].type == Ceil,
               "zero-source ceiling residual uses supplied main-plus-down nominal plane");
        slabSources = {source (baseA, atLevel (rectangle (0, 2, 0, 3, -1), 3.2), API_SlabID),
                       source (baseB, atLevel (rectangle (1, 4, 0, 3, -1), 3.4), API_SlabID)};
        roles = {baseA, baseB};
        projected.Clear ();
        rejected = 0;
        check (BuildProjectedSlabFinishes (ceilingZone, slabSources, roles, Ceil, 0, projected, rejected) &&
                   projected.GetSize () == 2 && closeArea (slabArea (projected), 12) &&
                   projected[0].base_guid == baseA && closeArea (projected[0].zBottom, 3.2) &&
                   closeArea (projected[0].poly.CalcArea (), 6) && projected[1].base_guid == baseB &&
                   closeArea (projected[1].zBottom, 3.4) && closeArea (projected[1].poly.CalcArea (), 6) &&
                   rejected == 0,
               "ceiling uses ascending lower-face priority then remaining coverage");
        slabSources[0] = source (baseA, atLevel (rectangle (0, 2, 0, 3, -1), 3.4), API_SlabID);
        projected.Clear ();
        check (BuildProjectedSlabFinishes (
                   ceilingZone, slabSources, roles, Ceil, 0, projected, rejected, &nominal, false, &steps) &&
                   closeArea (slabArea (projected), 9) && closeArea (rejected, 3),
               "equal-level owner overlap is excluded, never converted to nominal fallback");
        slabSources = {source (baseA, atLevel (rectangle (0, 2, 0, 3, 1), 0.2), API_SlabID)};
        slabSources[0].snapshot.bodies[0].surfaces.Push (atLevel (rectangle (0, 2, 0, 3, -1), -0.1));
        roles = {baseA};
        nominal.zBottom = 0;
        projected.Clear ();
        steps.Clear ();
        rejected = 0;
        check (BuildProjectedSlabFinishes (
                   floorZone, slabSources, roles, Floor, 0, projected, rejected, &nominal, false, &steps) &&
                   projected.GetSize () == 2 && closeArea (slabArea (projected), 12) &&
                   projected[1].base_guid == APINULLGuid && closeArea (projected[1].zBottom, 0) &&
                   steps.GetSize () == 1 && closeArea (steps[0].height, 0.2) && closeArea (steps[0].zBottom, 0) &&
                   steps[0].base_guid == baseA && steps[0].type == Floor && steps[0].surface_geometry,
               "partial floor keeps source plus nominal remainder and only internal step edge");
        projected.Clear ();
        steps.Clear ();
        check (BuildProjectedSlabFinishes (
                   floorZone, slabSources, roles, Floor, 0, projected, rejected, &nominal, true, &steps) &&
                   projected.GetSize () == 1 && closeArea (slabArea (projected), 6),
               "partial floor by-slab flag suppresses nominal remainder only");
        nominal.zBottom = -0.2;
        projected.Clear ();
        steps.Clear ();
        check (BuildProjectedSlabFinishes (
                   floorZone, slabSources, roles, Floor, 0, projected, rejected, &nominal, false, &steps) &&
                   steps.IsEmpty (),
               "no side finish if nominal floor lies outside source body");
        nominal.zBottom = 0.196;
        projected.Clear ();
        check (BuildProjectedSlabFinishes (
                   floorZone, slabSources, roles, Floor, 0, projected, rejected, &nominal, false, &steps) &&
                   steps.IsEmpty (),
               "step below legacy five millimeter height threshold is rejected");
        slabSources = {source (baseA, holed, API_SlabID)};
        nominal.zBottom = 0;
        projected.Clear ();
        check (BuildProjectedSlabFinishes (
                   floorZone, slabSources, roles, Floor, 0, projected, rejected, &nominal, false, &steps) &&
                   projected.GetSize () == 1 && closeArea (slabArea (projected), 11),
               "source holes are not backfilled even when nominal residual is enabled");
        slabSources = {source (baseA, atLevel (rectangle (0, 2, 0, 3, -1), 3.2), API_SlabID)};
        slabSources[0].snapshot.bodies[0].surfaces.Push (atLevel (rectangle (0, 2, 0, 3, 1), 3.6));
        nominal.zBottom = 3.4;
        projected.Clear ();
        steps.Clear ();
        check (BuildProjectedSlabFinishes (
                   ceilingZone, slabSources, roles, Ceil, 0, projected, rejected, &nominal, false, &steps) &&
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
        check (DeduplicateAcceptedSlabContacts (accepted) && accepted.otdwall.GetSize () == 1 &&
                   accepted.otdwall[0].slab_step && closeArea (contactArea (accepted), 0),
               "принятое полное покрытие перепада убирает только дубль контакта независимо от материала");
        step.endC = {2, 0};
        step.height = 1;
        accepted = acceptedRoom (side, step);
        check (DeduplicateAcceptedSlabContacts (accepted) && closeArea (contactArea (accepted), 10) &&
                   accepted.otdwall[0].material.smaterial == side.material.smaterial &&
                   accepted.otdwall[0].favorite.name == side.favorite.name && accepted.otdwall[0].type == Wall_Main,
               "частичное конечное покрытие сохраняет остаток и уже принятые правила контакта");
        step.isValid = false;
        accepted = acceptedRoom (side, step);
        check (DeduplicateAcceptedSlabContacts (accepted) && closeArea (contactArea (accepted), 12),
               "отвергнутая правилами боковина не стирает контакт");
        step.isValid = true;
        step.slab_step_writer_eligible = false;
        accepted = acceptedRoom (side, step);
        check (DeduplicateAcceptedSlabContacts (accepted) && closeArea (contactArea (accepted), 12),
               "непредставимое избранное боковины не стирает контакт");
        step.slab_step_writer_eligible = true;
        step.base_guid = baseB;
        accepted = acceptedRoom (side, step);
        check (DeduplicateAcceptedSlabContacts (accepted) && closeArea (contactArea (accepted), 12),
               "другое основание не вычитается");
        step.base_guid = baseA;
        step.begC.y = step.endC.y = 1;
        accepted = acceptedRoom (side, step);
        check (DeduplicateAcceptedSlabContacts (accepted) && closeArea (contactArea (accepted), 12),
               "параллельная другая плоскость не вычитается");
        step.begC.y = step.endC.y = 0;
        step.type = Wall_Up;
        accepted = acceptedRoom (side, step);
        accepted.create_wall_elements = false;
        accepted.create_reveal_elements = true;
        check (DeduplicateAcceptedSlabContacts (accepted) && closeArea (contactArea (accepted), 12),
               "отключённый writer боковины после смены типа не стирает контакт");
        step.type = Ceil;
        step.begC = {1, 0};
        step.endC = {3, 0};
        step.zBottom = 1;
        step.height = 1;
        accepted = acceptedRoom (side, step);
        check (DeduplicateAcceptedSlabContacts (accepted) && closeArea (contactArea (accepted), 10) &&
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
        check (DeduplicateAcceptedSlabContacts (accepted) && closeArea (contactArea (accepted), 8) &&
                   accepted.otdwall[0].openings.GetSize () == 1,
               "исходное отверстие контакта сохраняется при частичном вычитании");
        side.openings.Clear ();
        step = side;
        step.slab_step = true;
        step.openings = {acceptedHole};
        accepted = acceptedRoom (side, step);
        check (DeduplicateAcceptedSlabContacts (accepted) && closeArea (contactArea (accepted), 1),
               "отверстие принятой боковины не считается покрытием");
        step.openings.Clear ();
        step.endC = {0.00005, 0};
        accepted = acceptedRoom (side, step);
        check (DeduplicateAcceptedSlabContacts (accepted) && closeArea (contactArea (accepted), 12),
               "конечная боковина ниже порога writer не стирает контакт");
        step.begC = {0.00005, 0};
        step.endC = {4, 0};
        step.zBottom = 0;
        step.height = 3;
        accepted = acceptedRoom (side, step);
        check (!DeduplicateAcceptedSlabContacts (accepted) && closeArea (contactArea (accepted), 12),
               "непредставимый тонкий остаток сохраняется целиком и явно отвергается");
        step = side;
        step.slab_step = true;
        step.begC = {6, 0};
        step.endC = {8, 0};
        accepted = acceptedRoom (side, step);
        check (DeduplicateAcceptedSlabContacts (accepted) && closeArea (contactArea (accepted), 12),
               "конечное непересекающееся покрытие в той же плоскости не вычитается");
        step.begC = {3, 0};
        step.endC = {1, 0};
        accepted = acceptedRoom (side, step);
        check (DeduplicateAcceptedSlabContacts (accepted) && closeArea (contactArea (accepted), 6),
               "обратное направление линии не меняет конечное покрытие боковины");
        side.rise_y = 0.5;
        side.slant_angle = std::atan2 (1.0, 0.5);
        step = side;
        step.slab_step = true;
        step.height = 1;
        accepted = acceptedRoom (side, step);
        check (DeduplicateAcceptedSlabContacts (accepted) && closeArea (contactArea (accepted), 8 * std::sqrt (1.25)) &&
                   closeArea (accepted.otdwall[0].rise_y, 0.5) && closeArea (accepted.otdwall[0].begC.y, 0.5),
               "наклонный остаток остаётся в исходной плоскости и сохраняет подъём");
        // #267: native stepwalls retain Floor/Ceil classification; index them before obsolete deletion.
        check (IsExistingFinishNativeType (Floor, API_WallID) && IsExistingFinishNativeType (Ceil, API_WallID),
               "floor and ceiling native stepwalls enter existing finish index");
        check (IsExistingFinishNativeType (Floor, API_SlabID) && IsExistingFinishNativeType (Floor, API_ObjectID) &&
                   IsExistingFinishNativeType (Ceil, API_SlabID) && IsExistingFinishNativeType (Ceil, API_ObjectID),
               "floor and ceiling legacy native types remain accepted");
        check (!IsExistingFinishNativeType (Floor, API_BeamID) && !IsExistingFinishNativeType (Ceil, API_ColumnID) &&
                   !IsExistingFinishNativeType (Wall_Main, API_SlabID) &&
                   IsExistingFinishNativeType (Reveal_Main, API_BeamID),
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
            if (IsExistingFinishNativeType (type, API_WallID))
                existingStepwalls.Add (stepwallGuids[i], type);
            stepwallChildren.Add (stepwallGuids[i], true);
        }
        stepwallParents.Add (baseA, stepwallChildren);
        bool hasStepwallBase = false;
        const UnicGuidByBase indexedStepwalls = BuildOtdByParent (existingStepwalls, stepwallParents, hasStepwallBase);
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
            check (CleanCollinearPolygonVertices (empty) == 0 && empty.IsEmpty (), "polygon cleanup empty input");
            auto polygon = makeCleanupPolygon ({{2, 0}, {3, 0}, {4, 0}, {4, 3}, {0, 3}, {0, 0}, {1, 0}}, {6}, {});
            check (polygon.GetVertexNum () == 7 && CleanCollinearPolygonVertices (polygon) == 3 &&
                       polygon.GetVertexNum () == 4 && closeArea (polygon.CalcArea (), 12),
                   "polygon cleanup collinear chain and closed seam");
            check (CleanCollinearPolygonVertices (polygon) == 0 && polygon.GetVertexNum () == 4,
                   "polygon cleanup rectangle and repeated call unchanged");
            auto holed = makeCleanupPolygon (
                {{0, 0}, {4, 0}, {4, 3}, {0, 3}, {1, 1}, {1, 1.5}, {1, 2}, {2, 2}, {2, 1}}, {3, 8}, {});
            check (CleanCollinearPolygonVertices (holed) == 1 && holed.GetContourNum () == 2 &&
                       holed.GetVertexNum () == 8 && closeArea (holed.CalcArea (), 11),
                   "polygon cleanup preserves hole and orientation");
            PolyArcRec arc;
            arc.begIndex = 3;
            arc.endIndex = 4;
            arc.arcAngle = .5;
            auto curved = makeCleanupPolygon ({{0, 0}, {2, 0}, {4, 0}, {4, 3}, {0, 3}}, {4}, {arc});
            const double curvedArea = curved.CalcArea ();
            check (CleanCollinearPolygonVertices (curved) == 1 && curved.GetVertexNum () == 4 &&
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
               "268 cached full composition depth independent of layer filtering");
        coreFiltered.Clear ();
        check (Param_SetToBase (
                   baseA, true, coreFiltered, coreRead, coreComposites, coreParam, coreFavorite, &coreSource) &&
                   coreSource.floor_core_offset_valid && closeArea (coreSource.floor_core_offset, .04) &&
                   coreFiltered.GetSize () == 1,
               "268 cached flipped composition depth read before filtering");
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
            check (core.otdslab[1].poly.GetVertexNum () == 5 && ExtendFloorContoursToCore (core) &&
                       core.otdslab[1].poly.GetVertexNum () == 4 && closeArea (core.otdslab[1].poly.CalcArea (), 12) &&
                       core.otdslab[1].base_guid == baseB && core.otdslab[1].otd_guid == segment &&
                       core.otdslab[1].floorInd == 7 && core.otdslab[1].favorite.name == "preserved",
                   "polygon cleanup ceiling removes redundant vertex without floor extension or link changes");
            check (ExtendFloorContoursToCore (core) && core.otdslab[1].poly.GetVertexNum () == 4 &&
                       closeArea (core.otdslab[1].poly.CalcArea (), 12),
                   "polygon cleanup ceiling repeated call unchanged");
        }
        core = coreRoom (false);
        check (ExtendFloorContoursToCore (core) && closeArea (core.otdslab[0].poly.CalcArea (), 12.4),
               "268 SDK finite floor union moves outward");
        check (closeArea (core.otdslab[1].poly.CalcArea (), 12) && core.otdwall[0].begC.x == 0 &&
                   core.otdwall[0].begC.y == 0 && core.otdwall[0].endC.x == 4 && core.otdwall[0].endC.y == 0 &&
                   core.otdwall[0].height == 3,
               "268 SDK ceiling and wall geometry unchanged");
        check (core.otdslab[0].base_guid == baseB && core.otdslab[0].otd_guid == segment &&
                   core.otdslab[0].floorInd == 7 && core.otdslab[0].favorite.name == "preserved",
               "268 SDK floor ownership favorite and existing link retained");
        check (ExtendFloorContoursToCore (core) && closeArea (core.otdslab[0].poly.CalcArea (), 12.4),
               "268 SDK repeated correction is stable");
        core = coreRoom (false);
        core.otdwall[0].begC.x = 1;
        core.otdwall[0].endC.x = 3;
        check (ExtendFloorContoursToCore (core) && closeArea (core.otdslab[0].poly.CalcArea (), 12.2),
               "268 SDK unmatched finite edge portions retained");
        core = coreRoom (false);
        auto cornerWall = core.otdwall[0];
        cornerWall.begC = {4, 0};
        cornerWall.endC = {4, 3};
        cornerWall.base_guid = segment;
        cornerWall.floor_core_offset = .2;
        core.otdwall.Push (cornerWall);
        check (ExtendFloorContoursToCore (core) && closeArea (core.otdslab[0].poly.CalcArea (), 13.02),
               "268 SDK unequal adjacent depths preserve exact corner");
        core = coreRoom (true);
        core.otdwall[0].begC = {1, 1};
        core.otdwall[0].endC = {1, 2};
        check (ExtendFloorContoursToCore (core) && closeArea (core.otdslab[0].poly.CalcArea (), 11.1) &&
                   core.otdslab[0].poly.GetContourNum () == 2,
               "268 SDK hole extends only matched side and remains a hole");
        core = coreRoom (false);
        core.otdwall[0].zBottom = .5;
        check (ExtendFloorContoursToCore (core) && closeArea (core.otdslab[0].poly.CalcArea (), 12),
               "268 SDK wall above floor excluded");
        core = coreRoom (false);
        core.otdwall[0].floor_core_offset_valid = false;
        check (ExtendFloorContoursToCore (core) && closeArea (core.otdslab[0].poly.CalcArea (), 12),
               "268 SDK undefined offset unchanged");
        core = coreRoom (false);
        auto coreConflict = core.otdwall[0];
        coreConflict.base_guid = segment;
        core.otdwall.Push (coreConflict);
        check (ExtendFloorContoursToCore (core) && closeArea (core.otdslab[0].poly.CalcArea (), 12),
               "268 SDK ambiguous base contact unchanged");
        core = coreRoom (false);
        OtdOpening door;
        door.objLoc = 2;
        door.width = 1;
        door.height = 2;
        core.otdwall[0].openings.Push (door);
        check (ExtendFloorContoursToCore (core) && closeArea (core.otdslab[0].poly.CalcArea (), 12.3),
               "268 SDK opening on floor excludes only its finite span");
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
               "268 height split carries independent offset");
        return GS::ObjectState ("status", failed == 0 ? "Passed" : "Failed", "checks", checks, "failed", failed);
    }

    void AddError (
        GS::Array<GS::ObjectState> &errors, const char *stage, GSErrCode code, Int32 body, Int32 polygon, Int32 index) {
        errors.Push (GS::ObjectState ("stage", stage, "code", code, "body", body, "polygon", polygon, "index", index));
    }

    Int32 AttributeIndexValue (API_AttributeIndex index) {
    #ifdef ServerMainVers_2700
        return index.ToInt32_Deprecated ();
    #else
        return index;
    #endif
    }

    GS::ObjectState Override (const API_OverriddenAttribute &value) {
    #ifdef ServerMainVers_2700
        return GS::ObjectState (
            "attributeIndex", value.hasValue ? AttributeIndexValue (value.value) : 0, "active", value.hasValue);
    #else
        return GS::ObjectState ("attributeIndex", value.attributeIndex, "active", value.overridden);
    #endif
    }

    GS::ObjectState BuildingMaterial (API_AttributeIndex index) {
        GS::ObjectState out ("index", AttributeIndexValue (index));
        API_Attribute attribute = {};
        GS::UniString name;
        GS::UniString id;
        GS::UniString manufacturer;
        GS::UniString description;
        attribute.header.typeID = API_BuildingMaterialID;
        attribute.header.index = index;
        attribute.header.uniStringNamePtr = &name;
        attribute.buildingMaterial.id = &id;
        attribute.buildingMaterial.manufacturer = &manufacturer;
        attribute.buildingMaterial.description = &description;
        const GSErrCode err = ACAPI_Attribute_Get (&attribute);
        out.Add ("error", err);
        if (err == NoError) {
            out.Add ("name", name);
            out.Add ("defaultSurfaceAttributeIndex", AttributeIndexValue (attribute.buildingMaterial.cutMaterial));
            out.Add ("connectionPriority", attribute.buildingMaterial.connPriority);
        }
        return out;
    }

    GS::ObjectState ReadModelMaterial (API_AttributeIndex index) {
        GS::ObjectState out ("modelMaterialIndex", AttributeIndexValue (index));
        API_Component3D component = {};
        component.header.typeID = API_UmatID;
    #ifdef ServerMainVers_2700
        component.header.index = index.ToInt32_Deprecated ();
    #else
        component.header.index = index;
    #endif
        const GSErrCode err = ACAPI_3D_GetComponent (&component);
        out.Add ("error", err);
        out.Add ("status", err == NoError ? "Resolved" : "ReadError");
        if (err == NoError) {
            const API_MaterialType &material = component.umat.mater;
            out.Add ("surfaceAttributeIndex", AttributeIndexValue (material.head.index));
    #ifdef ServerMainVers_2700
            const bool generated = material.head.index.ToInt32_Deprecated () == 0;
    #else
            const bool generated = material.head.index == 0;
    #endif
            out.Add ("generated", generated);
            out.Add ("modelName", GS::UniString (material.head.name));
            out.Add (
                "rgb",
                GS::ObjectState (
                    "r", material.surfaceRGB.f_red, "g", material.surfaceRGB.f_green, "b", material.surfaceRGB.f_blue));
            out.Add ("transparency", material.transpPc);
            out.Add ("textureName", GS::UniString (material.texture.texName));
            if (!generated) {
                API_Attribute attribute = {};
                GS::UniString name;
                attribute.header.typeID = API_MaterialID;
                attribute.header.index = material.head.index;
                attribute.header.uniStringNamePtr = &name;
                const GSErrCode attributeErr = ACAPI_Attribute_Get (&attribute);
                out.Add ("attributeError", attributeErr);
                if (attributeErr == NoError)
                    out.Add ("name", name);
                delete attribute.material.texture.fileLoc;
            }
        }
        // Как в SDK 3D_Test: указатель текстуры принадлежит вызывающему, в JSON не сохраняется.
        delete component.umat.mater.texture.fileLoc;
        return out;
    }

    GS::ObjectState ReadProfile (const ProfileVectorImage *profile) {
        GS::ObjectState out ("available", profile != nullptr);
        GS::Array<GS::ObjectState> hatches;
        if (profile != nullptr) {
            ConstProfileVectorImageIterator it (*profile);
            while (!it.IsEOI ()) {
                if (it->item_Typ == SyHatch) {
                    const HatchObject &hatch = it;
                    GS::ObjectState item (
                        "hatchGuid",
                        hatch.GetUniqueId ().ToUniString (),
                        "structureComponentId",
                        profile->GetStructureComponentId (hatch.GetUniqueId ()).ValueStringForDump ());
    #ifdef ServerMainVers_2800
                    const API_AttributeIndex buildingMaterial =
                        ACAPI_CreateAttributeIndex (hatch.GetBuildMatIdx ().ToGSAttributeIndex_Deprecated ());
    #elif defined(ServerMainVers_2700)
                    const API_AttributeIndex buildingMaterial =
                        ACAPI_CreateAttributeIndex (hatch.GetBuildMatIdx ().ToGSAttributeIndex ());
    #else
                    const API_AttributeIndex buildingMaterial =
                        static_cast<API_AttributeIndex> (hatch.GetBuildMatIdx ());
    #endif
                    item.Add ("buildingMaterial", BuildingMaterial (buildingMaterial));
    #ifdef ServerMainVers_2700
                    const ProfileItem *info = hatch.HasProfileInfo () ? &hatch.GetProfileItem () : nullptr;
    #else
                    const ProfileItem *info = hatch.GetProfileItemPtr ();
    #endif
                    item.Add ("hasProfileInfo", info != nullptr);
                    if (info != nullptr) {
                        item.Add ("core", info->IsCore ());
                        item.Add ("finish", info->IsFinish ());
                    }
                    GS::Array<GS::ObjectState> coords;
                    GS::Array<GS::ObjectState> edges;
                    for (UIndex i = 0; i < hatch.GetCoords ().GetSize (); ++i) {
                        const Coord &coord = hatch.GetCoords ()[i];
                        coords.Push (GS::ObjectState ("index", i, "x", coord.x, "y", coord.y));
                        // Индексы сырых coords/addInfo — как в Attribute_Test AC25, без преобразования vertex→edge.
    #ifdef ServerMainVers_2700
                        const ProfileEdgeData *edge = info != nullptr && i < hatch.GetProfileEdgeData ().size ()
                                                          ? &hatch.GetProfileEdgeData ()[i]
                                                          : nullptr;
    #else
                        const ProfileEdgeData *edgeData = info != nullptr ? hatch.GetProfileEdgeDataPtr () : nullptr;
                        const ProfileEdgeData *edge = edgeData != nullptr ? edgeData + i : nullptr;
    #endif
                        if (edge != nullptr)
                            edges.Push (GS::ObjectState ("index",
                                                         i,
                                                         "surfaceAttributeIndex",
                                                         edge->GetMaterial (),
                                                         "fromBuildingMaterial",
                                                         edge->IsSurfaceFromBuildMat (),
                                                         "flags",
                                                         edge->GetFlags (),
                                                         "sideSelector",
                                                         static_cast<Int32> (edge->GetSideSelector ())));
                    }
                    item.Add ("coords", coords);
                    item.Add ("subPolyEnds", hatch.GetSubPolyEnds ());
                    item.Add ("arcCount", hatch.GetArcs ().GetSize ());
                    item.Add ("edges", edges);
                    hatches.Push (item);
                }
                ++it;
            }
        }
        out.Add ("hatches", hatches);
        return out;
    }

    GS::ObjectState AttributeProfile (API_AttributeIndex index) {
        API_AttributeDefExt defs = {};
        const GSErrCode err = ACAPI_Attribute_GetDefExt (API_ProfileID, index, &defs);
        GS::ObjectState out ("attributeIndex", AttributeIndexValue (index), "error", err);
        if (err == NoError)
            out.Add ("profile", ReadProfile (defs.profile_vectorImageItems));
        ACAPI_DisposeAttrDefsHdlsExt (&defs);
        return out;
    }

    GS::ObjectState Composite (API_AttributeIndex index) {
        GS::ObjectState out ("attributeIndex", AttributeIndexValue (index));
        API_Attribute attribute = {};
        GS::UniString name;
        attribute.header.typeID = API_CompWallID;
        attribute.header.index = index;
        attribute.header.uniStringNamePtr = &name;
        const GSErrCode err = ACAPI_Attribute_Get (&attribute);
        out.Add ("attributeError", err);
        GS::Array<GS::ObjectState> skins;
        API_AttributeDefExt defs = {};
        if (err == NoError) {
            out.Add ("name", name);
            out.Add ("totalThickness", attribute.compWall.totalThick);
            const GSErrCode defErr = ACAPI_Attribute_GetDefExt (API_CompWallID, index, &defs);
            out.Add ("definitionError", defErr);
            if (defErr == NoError && defs.cwall_compItems != nullptr) {
                for (Int32 i = 0; i < attribute.compWall.nComps; ++i) {
                    const API_CWallComponent &skin = (*defs.cwall_compItems)[i];
                    skins.Push (GS::ObjectState ("ordinal",
                                                 i + 1,
                                                 "thickness",
                                                 skin.fillThick,
                                                 "flags",
                                                 skin.flagBits,
                                                 "buildingMaterial",
                                                 BuildingMaterial (skin.buildingMaterial)));
                }
            }
        }
        out.Add ("skins", skins);
        ACAPI_DisposeAttrDefsHdlsExt (&defs);
        return out;
    }

    GS::ObjectState ColumnSegment (const API_ColumnSegmentType &segment) {
        const API_AssemblySegmentData &data = segment.assemblySegmentData;
        GS::ObjectState out ("guid",
                             APIGuidToString (segment.head.guid),
                             "owner",
                             APIGuidToString (segment.owner),
                             "structureType",
                             static_cast<Int32> (data.modelElemStructureType),
                             "profileAttributeIndex",
                             AttributeIndexValue (data.profileAttr));
        out.Add ("homogeneous", data.isHomogeneous);
        out.Add ("circleBased", data.circleBased);
        out.Add ("width", data.nominalWidth);
        out.Add ("height", data.nominalHeight);
        out.Add ("endWidth", data.endWidth);
        out.Add ("endHeight", data.endHeight);
        out.Add ("relativeHomeStoryLevel", data.relativeHomeStoryLevel);
        out.Add ("extrusionOverride", Override (segment.extrusionSurfaceMaterial));
        out.Add ("endsOverride", Override (segment.endsMaterial));
        out.Add ("veneerThickness", segment.venThick);
        out.Add ("veneerType", static_cast<Int32> (segment.venType));
        if (data.modelElemStructureType == API_BasicStructure)
            out.Add ("buildingMaterial", BuildingMaterial (data.buildingMaterial));
        if (segment.venThick > 0)
            out.Add ("veneerBuildingMaterial", BuildingMaterial (segment.venBuildingMaterial));
        if (data.modelElemStructureType == API_ProfileStructure)
            out.Add ("attributeProfile", AttributeProfile (data.profileAttr));
        return out;
    }

    GS::ObjectState ReadStructure (const API_Guid &guid) {
        GS::ObjectState out;
        API_Element element = {};
        element.header.guid = guid;
        GSErrCode err = ACAPI_Element_Get (&element);
        out.Add ("elementError", err);
        if (err != NoError)
            return out;
        if (GetElemTypeID (element) == API_ColumnSegmentID) {
            out.Add ("selectedSegment", ColumnSegment (element.columnSegment));
            element.header.guid = element.columnSegment.owner;
            err = ACAPI_Element_Get (&element);
            out.Add ("ownerError", err);
            if (err != NoError)
                return out;
        }
        out.Add ("elementGuid", APIGuidToString (element.header.guid));
        out.Add ("elementType", static_cast<Int32> (GetElemTypeID (element)));
        API_ElementMemo memo = {};
        if (GetElemTypeID (element) == API_SlabID) {
            out.Add ("floorIndex", element.header.floorInd);
            out.Add ("level", element.slab.level);
            out.Add ("offsetFromTop", element.slab.offsetFromTop);
            out.Add ("thickness", element.slab.thickness);
            out.Add ("polygonCoordCount", element.slab.poly.nCoords);
            out.Add ("polygonContourCount", element.slab.poly.nSubPolys);
            out.Add ("polygonArcCount", element.slab.poly.nArcs);
            err = ACAPI_Element_GetMemo (guid, &memo, APIMemoMask_Polygon);
            out.Add ("memoError", err);
            // Исходный memo отделяет реальные контуры от SEO и приоритетных подрезок native-тела.
            GS::Array<GS::ObjectState> coords;
            GS::Array<Int32> ends;
            GS::Array<GS::ObjectState> arcs;
            if (err == NoError) {
                if (memo.coords != nullptr) {
                    const GSSize count =
                        BMGetHandleSize (reinterpret_cast<GSHandle> (memo.coords)) / sizeof (API_Coord);
                    for (GSSize i = 1; i < count && i <= element.slab.poly.nCoords; ++i)
                        coords.Push (Point ((*memo.coords)[i].x, (*memo.coords)[i].y, 0));
                }
                if (memo.pends != nullptr) {
                    const GSSize count = BMGetHandleSize (reinterpret_cast<GSHandle> (memo.pends)) / sizeof (Int32);
                    for (GSSize i = 1; i < count && i <= element.slab.poly.nSubPolys; ++i)
                        ends.Push ((*memo.pends)[i]);
                }
                if (memo.parcs != nullptr) {
                    const GSSize count =
                        BMGetHandleSize (reinterpret_cast<GSHandle> (memo.parcs)) / sizeof (API_PolyArc);
                    for (GSSize i = 0; i < count && i < element.slab.poly.nArcs; ++i) {
                        const auto &arc = (*memo.parcs)[i];
                        arcs.Push (GS::ObjectState (
                            "begIndex", arc.begIndex, "endIndex", arc.endIndex, "angle", arc.arcAngle));
                    }
                }
            }
            out.Add ("memoCoordinates", coords);
            out.Add ("memoContourEnds", ends);
            out.Add ("memoArcs", arcs);

        } else if (GetElemTypeID (element) == API_WallID) {
            const API_WallType &wall = element.wall;
            out.Add ("structureType", static_cast<Int32> (wall.modelElemStructureType));
            out.Add ("beg", Point (wall.begC.x, wall.begC.y, 0));
            out.Add ("end", Point (wall.endC.x, wall.endC.y, 0));
            out.Add ("flipped", wall.flipped);
            out.Add ("angle", wall.angle);
            out.Add ("offset", wall.offset);
            out.Add ("offsetFromOutside", wall.offsetFromOutside);
            out.Add ("bottomOffset", wall.bottomOffset);
            out.Add ("height", wall.height);
            out.Add ("thickness", wall.thickness);
            out.Add ("profileType", wall.profileType);
            out.Add ("referenceLineLocation", static_cast<Int32> (wall.referenceLineLocation));
            out.Add ("refOverride", Override (wall.refMat));
            out.Add ("oppOverride", Override (wall.oppMat));
            out.Add ("sideOverride", Override (wall.sidMat));
            if (wall.modelElemStructureType == API_BasicStructure)
                out.Add ("buildingMaterial", BuildingMaterial (wall.buildingMaterial));
            if (wall.modelElemStructureType == API_CompositeStructure)
                out.Add ("composite", Composite (wall.composite));
            if (wall.modelElemStructureType == API_ProfileStructure) {
                err = ACAPI_Element_GetMemo (guid, &memo, APIMemoMask_StretchedProfile);
                out.Add ("memoError", err);
                if (err == NoError)
                    out.Add ("stretchedProfile", ReadProfile (memo.stretchedProfile));
                out.Add ("attributeProfile", AttributeProfile (wall.profileAttr));
            }
        } else if (GetElemTypeID (element) == API_ColumnID) {
            const API_ColumnType &column = element.column;
            out.Add ("origin", Point (column.origoPos.x, column.origoPos.y, 0));
            out.Add ("rotation", column.axisRotationAngle);
            out.Add ("flipped", column.isFlipped);
            out.Add ("slanted", column.isSlanted);
            out.Add ("slantAngle", column.slantAngle);
            out.Add ("slantDirection", column.slantDirectionAngle);
            out.Add ("coreAnchor", column.coreAnchor);
            out.Add ("bottomOffset", column.bottomOffset);
            out.Add ("height", column.height);
            out.Add ("profileCount", column.nProfiles);
            err = ACAPI_Element_GetMemo (element.header.guid,
                                         &memo,
                                         APIMemoMask_ColumnSegment | APIMemoMask_AssemblySegmentProfile |
                                             APIMemoMask_AssemblySegmentScheme | APIMemoMask_AssemblySegmentCut);
            out.Add ("memoError", err);
            GS::Array<GS::ObjectState> segments;
            GS::Array<GS::ObjectState> profiles;
            GS::Array<GS::ObjectState> schemes;
            GS::Array<GS::ObjectState> cuts;
            if (err == NoError) {
                if (memo.columnSegments != nullptr) {
                    const GSSize count =
                        BMGetPtrSize (reinterpret_cast<GSPtr> (memo.columnSegments)) / sizeof (API_ColumnSegmentType);
                    for (GSSize i = 0; i < count; ++i)
                        segments.Push (ColumnSegment (memo.columnSegments[i]));
                }
                if (memo.assemblySegmentProfiles != nullptr) {
                    const GSSize count = BMGetPtrSize (reinterpret_cast<GSPtr> (memo.assemblySegmentProfiles)) /
                                         sizeof (API_AssemblySegmentProfileData);
                    for (GSSize i = 0; i < count; ++i) {
                        const API_AssemblySegmentProfileData &profile = memo.assemblySegmentProfiles[i];
                        profiles.Push (GS::ObjectState ("segmentIndex",
                                                        profile.segmentIndex,
                                                        "stretchedProfile",
                                                        ReadProfile (profile.stretchedProfile),
                                                        "customOrigProfile",
                                                        ReadProfile (profile.customOrigProfile)));
                    }
                }
                if (memo.assemblySegmentSchemes != nullptr) {
                    const GSSize count = BMGetPtrSize (reinterpret_cast<GSPtr> (memo.assemblySegmentSchemes)) /
                                         sizeof (API_AssemblySegmentSchemeData);
                    for (GSSize i = 0; i < count; ++i) {
                        const API_AssemblySegmentSchemeData &scheme = memo.assemblySegmentSchemes[i];
                        schemes.Push (GS::ObjectState ("index",
                                                       i,
                                                       "lengthType",
                                                       static_cast<Int32> (scheme.lengthType),
                                                       "fixedLength",
                                                       scheme.fixedLength,
                                                       "proportion",
                                                       scheme.lengthProportion));
                    }
                }
                if (memo.assemblySegmentCuts != nullptr) {
                    const GSSize count = BMGetPtrSize (reinterpret_cast<GSPtr> (memo.assemblySegmentCuts)) /
                                         sizeof (API_AssemblySegmentCutData);
                    for (GSSize i = 0; i < count; ++i) {
                        const API_AssemblySegmentCutData &cut = memo.assemblySegmentCuts[i];
                        cuts.Push (GS::ObjectState (
                            "index", i, "type", static_cast<Int32> (cut.cutType), "angle", cut.customAngle));
                    }
                }
            }
            out.Add ("segments", segments);
            out.Add ("profiles", profiles);
            out.Add ("schemes", schemes);
            out.Add ("cuts", cuts);
        } else if (GetElemTypeID (element) == API_ObjectID || GetElemTypeID (element) == API_WindowID ||
                   GetElemTypeID (element) == API_DoorID) {
            const bool isObject = GetElemTypeID (element) == API_ObjectID;
            const Int32 libInd = isObject ? element.object.libInd : element.window.openingBase.libInd;
            out.Add ("libInd", libInd);
            API_LibPart libpart = {};
            libpart.index = libInd;
    #ifdef ServerMainVers_2700
            const GSErrCode libErr = ACAPI_LibraryPart_Get (&libpart);
    #else
            const GSErrCode libErr = ACAPI_LibPart_Get (&libpart);
    #endif
            out.Add ("libpartError", libErr);
            if (libErr == NoError) {
                out.Add ("libpartName", GS::UniString (libpart.docu_UName));
                out.Add ("libpartUnID", GS::UniString (libpart.ownUnID));
            }
            delete libpart.location;
            if (!isObject) {
                out.Add ("owner", APIGuidToString (element.window.owner));
                out.Add ("refSide", element.window.openingBase.refSide);
                out.Add ("revealDepthFromSide", element.window.revealDepthFromSide);
                out.Add ("revealDepthOffset", element.window.revealDepthOffset);
            }
            err = ACAPI_Element_GetMemo (guid, &memo, APIMemoMask_AddPars);
            out.Add ("memoError", err);
            GS::Array<GS::ObjectState> parameters;
            if (err == NoError && memo.params != nullptr) {
                const GSSize count =
                    BMGetHandleSize (reinterpret_cast<GSHandle> (memo.params)) / sizeof (API_AddParType);
                for (GSSize i = 0; i < count; ++i) {
                    const API_AddParType &par = (*memo.params)[i];
                    GS::ObjectState item ("name",
                                          GS::UniString (par.name),
                                          "type",
                                          par.typeID,
                                          "typeMod",
                                          par.typeMod,
                                          "dim1",
                                          par.dim1,
                                          "dim2",
                                          par.dim2);
                    if (par.typeMod == 0 && par.typeID != APIParT_CString)
                        item.Add ("number", par.value.real);
                    parameters.Push (item);
                }
            }
            out.Add ("parameters", parameters);
        }
        ACAPI_DisposeElemMemoHdls (&memo);
        return out;
    }

    GS::ObjectState ReadFavorite (const GS::UniString &name) {
        API_Favorite favorite (name);
        favorite.memo.New ();
        BNZeroMemory (&favorite.memo.Get (), sizeof (API_ElementMemo));
        const GSErrCode err = ACAPI_Favorite_Get (&favorite);
        GS::ObjectState out ("favoriteName", name, "favoriteError", err);
        GS::Array<GS::ObjectState> parameters;
        if (err == NoError) {
            const API_Element &element = favorite.element;
            out.Add ("type", static_cast<Int32> (GetElemTypeID (element)));
            if (GetElemTypeID (element) == API_ObjectID || GetElemTypeID (element) == API_WindowID ||
                GetElemTypeID (element) == API_DoorID) {
                const Int32 libInd =
                    GetElemTypeID (element) == API_ObjectID ? element.object.libInd : element.window.openingBase.libInd;
                out.Add ("libInd", libInd);
                API_LibPart libpart = {};
                libpart.index = libInd;
    #ifdef ServerMainVers_2700
                const GSErrCode libErr = ACAPI_LibraryPart_Get (&libpart);
    #else
                const GSErrCode libErr = ACAPI_LibPart_Get (&libpart);
    #endif
                out.Add ("libpartError", libErr);
                if (libErr == NoError) {
                    out.Add ("docu_UName", GS::UniString (libpart.docu_UName));
                    out.Add ("ownUnID", GS::UniString (libpart.ownUnID));
                }
                delete libpart.location;
            }
            if (favorite.memo->params != nullptr) {
                const GSSize count =
                    BMGetHandleSize (reinterpret_cast<GSHandle> (favorite.memo->params)) / sizeof (API_AddParType);
                for (GSSize i = 0; i < count; ++i) {
                    const API_AddParType &par = (*favorite.memo->params)[i];
                    GS::ObjectState item ("name",
                                          GS::UniString (par.name),
                                          "type",
                                          par.typeID,
                                          "typeMod",
                                          par.typeMod,
                                          "dim1",
                                          par.dim1,
                                          "dim2",
                                          par.dim2);
                    if (par.typeMod == 0 && par.typeID != APIParT_CString && par.typeID != APIParT_Separator &&
                        par.typeID != APIParT_Title && par.typeID != APIParT_Dictionary)
                        item.Add ("number", par.value.real);
                    parameters.Push (item);
                }
            }
        }
        out.Add ("parameters", parameters);
        ACAPI_DisposeElemMemoHdls (&favorite.memo.Get ());
        return out;
    }

    const char *SurfaceStatusName (Roombook::RoomSurfaceStatus status) {
        switch (status) {
        case Roombook::RoomSurfaceStatus::NotRead:
            return "NotRead";
        case Roombook::RoomSurfaceStatus::Extracted:
            return "Extracted";
        case Roombook::RoomSurfaceStatus::Empty:
            return "Empty";
        case Roombook::RoomSurfaceStatus::ReadError:
            return "ReadError";
        case Roombook::RoomSurfaceStatus::UnsupportedVersion:
            return "UnsupportedVersion";
        }
        return "ReadError";
    }

    GS::ObjectState GetZone3DPolygons_StrictAPI (const API_Elem_Head &head,
                                                 const Roombook::RoomSurfaceSnapshot &snapshot) {
        GS::ObjectState result ("guid", APIGuidToString (head.guid), "type", static_cast<Int32> (GetElemTypeID (head)));
        GS::Array<GS::ObjectState> errors;
        for (const Roombook::RoomSurfaceReadError &error : snapshot.errors)
            errors.Push (GS::ObjectState ("stage",
                                          error.stage,
                                          "code",
                                          error.code,
                                          "body",
                                          error.body,
                                          "polygon",
                                          error.polygon,
                                          "index",
                                          error.index));
        result.Add ("status", SurfaceStatusName (snapshot.status));
        result.Add ("readComplete", snapshot.IsComplete ());
        if (snapshot.errors.IsEmpty () || snapshot.errors[0].stage != "get3DInfo") {
            result.Add ("firstBody", snapshot.firstBody);
            result.Add ("lastBody", snapshot.lastBody);
        }
        GS::Array<GS::ObjectState> bodies;
        GS::Array<API_AttributeIndex> materialIndices;
        for (const Roombook::RoomSurfaceBody &body : snapshot.bodies) {
            GS::ObjectState bodyOut (
                "index", body.index, "polygonCount", body.polygonCount, "vertexCount", body.vertexCount);
            GS::Array<double> matrix;
            for (double value : body.matrix.tmx)
                matrix.Push (value);
            bodyOut.Add ("matrix", matrix);
            bodyOut.Add ("parentGuid", APIGuidToString (body.parentGuid));
            bodyOut.Add ("parentType", static_cast<Int32> (body.parentType));
            GS::Array<GS::ObjectState> polygons;
            for (const Roombook::RoomSurface &surface : body.surfaces) {
                GS::ObjectState polygon ("index",
                                         surface.polygonIndex,
                                         "material",
                                         AttributeIndexValue (surface.modelMaterialIndex),
                                         "flags",
                                         surface.flags,
                                         "signedNormalIndex",
                                         surface.signedNormalIndex);
                polygon.Add ("normalLocal",
                             Point (surface.normalLocal.x, surface.normalLocal.y, surface.normalLocal.z));
                polygon.Add ("normalWorld",
                             Point (surface.normalWorld.x, surface.normalWorld.y, surface.normalWorld.z));
                polygon.Add ("planeOffset", surface.planeOffset);
                GS::Array<GS::ObjectState> contours;
                for (const Roombook::RoomSurfaceContour &loop : surface.contours) {
                    GS::Array<GS::ObjectState> vertices;
                    for (const Roombook::RoomSurfaceVertex &point : loop.vertices) {
                        GS::ObjectState vertex = Point (point.local.x, point.local.y, point.local.z);
                        vertex.Add ("index", point.index);
                        vertex.Add ("signedEdgeIndex", point.signedEdgeIndex);
                        vertex.Add ("world", Point (point.world.x, point.world.y, point.world.z));
                        vertices.Push (vertex);
                    }
                    contours.Push (GS::ObjectState ("hole", loop.hole, "vertices", vertices));
                }
                polygon.Add ("contours", contours);
                polygons.Push (polygon);
                if (!materialIndices.Contains (surface.modelMaterialIndex))
                    materialIndices.Push (surface.modelMaterialIndex);
            }
            bodyOut.Add ("polygons", polygons);
            bodies.Push (bodyOut);
        }
        result.Add ("bodies", bodies);
        result.Add ("deletedBodies", snapshot.deletedBodies);
        result.Add ("errors", errors);
        // UMAT глобальны: читаются после геометрии, чтобы не затронуть выбранное тело.
        GS::Array<GS::ObjectState> materials;
        for (API_AttributeIndex index : materialIndices)
            materials.Push (ReadModelMaterial (index));
        result.Add ("modelMaterials", materials);
        result.Add ("structure", ReadStructure (head.guid));
        result.Add ("schemaVersion", 2);
        result.Add ("surfaceSnapshotVersion", 1);
        return result;
    }

    GS::ObjectState ReadZoneSurfacePartition (const API_Elem_Head &zone) {
        using namespace Roombook;
        if (GetElemTypeID (zone) != API_ZoneID)
            return GS::ObjectState ("status", "InvalidInput", "partitionComplete", false, "error", APIERR_BADID);
        if (!ClassificationFunc::ReadSystemDict ())
            return GS::ObjectState (
                "status", "ReadError", "partitionComplete", false, "stage", "finishClassificationSystem");
        ClassificationFunc::ClassificationDict finclass;
        UnicGuid finclassguids;
        Class_FindFinClass (finclass, finclassguids);
        GS::Array<API_Guid> finishGuids;
        finclassguids.EnumerateKeys ([&] (const API_Guid &guid) { finishGuids.Push (guid); });
        const auto zoneSnapshot = ReadRoomSurfaces (zone);
        auto candidates = ReadRoomSurfaceCandidates (zone, finishGuids);
        // Диагностика сохраняет native-контакты; production отдельно готовит аналитические основания.
        for (auto &source : candidates.sources) {
            if (source.baseType != API_WallID && source.baseType != API_SlabID)
                continue;
            API_Elem_Head geometry = {};
            geometry.guid = source.geometryGuid;
            SetElemTypeID (geometry, source.baseType);
            source.snapshot = ReadRoomSurfaces (geometry);
            source.origin = RoomSurfaceOrigin::Native3D;
            if (!source.snapshot.IsComplete ()) {
                candidates.errors.Push ({source.geometryGuid, "candidateSurfaceSnapshot", APIERR_GENERAL, 0});
                candidates.complete = false;
            }
        }
        RoomSurfacePartition partition;
        if (candidates.complete)
            partition = PartitionRoomSurfaces (zoneSnapshot, candidates.sources);
        GS::ObjectState result ("status",
                                partition.complete ? "Partitioned" : "ReadError",
                                "zoneGuid",
                                APIGuidToString (zone.guid),
                                "candidateSet",
                                "RoomRelations",
                                "geometryRoute",
                                "DiagnosticNative3D",
                                "candidateSetComplete",
                                candidates.complete,
                                "zoneSnapshotComplete",
                                zoneSnapshot.IsComplete (),
                                "partitionComplete",
                                partition.complete);
        GS::Array<GS::ObjectState> candidateErrors, partitionErrors, sources, excluded, pieces;
        for (const auto &error : candidates.errors)
            candidateErrors.Push (GS::ObjectState (
                "guid", APIGuidToString (error.guid), "stage", error.stage, "code", error.code, "index", error.index));
        for (const auto &error : partition.errors)
            partitionErrors.Push (GS::ObjectState ("stage",
                                                   error.stage,
                                                   "code",
                                                   error.code,
                                                   "body",
                                                   error.body,
                                                   "polygon",
                                                   error.polygon,
                                                   "index",
                                                   error.index));
        for (const auto &source : candidates.sources) {
            GS::Array<GS::ObjectState> readErrors;
            for (const auto &error : source.snapshot.errors)
                readErrors.Push (GS::ObjectState ("stage",
                                                  error.stage,
                                                  "code",
                                                  error.code,
                                                  "body",
                                                  error.body,
                                                  "polygon",
                                                  error.polygon,
                                                  "index",
                                                  error.index));
            sources.Push (GS::ObjectState (
                "baseGuid",
                APIGuidToString (source.baseGuid),
                "baseType",
                static_cast<Int32> (source.baseType),
                "geometryGuid",
                APIGuidToString (source.geometryGuid),
                "origin",
                source.origin == Roombook::RoomSurfaceOrigin::PreparedAnalytic ? "PreparedAnalytic" : "Native3D",
                "snapshotStatus",
                SurfaceStatusName (source.snapshot.status),
                "snapshotComplete",
                source.snapshot.IsComplete (),
                "errors",
                readErrors));
        }
        for (const auto &guid : candidates.excludedFinish)
            excluded.Push (GS::ObjectState ("guid", APIGuidToString (guid)));
        double matched = 0, unmatched = 0, ambiguous = 0;
        for (const auto &piece : partition.pieces) {
            const char *status = "Unmatched";
            if (piece.status == RoomSurfacePieceStatus::Matched) {
                status = "Matched";
                matched += piece.area;
            } else if (piece.status == RoomSurfacePieceStatus::Ambiguous) {
                status = "Ambiguous";
                ambiguous += piece.area;
            } else
                unmatched += piece.area;
            GS::Array<GS::ObjectState> contours, faces;
            for (const auto &contour : piece.contours) {
                GS::Array<GS::ObjectState> vertices;
                for (const auto &vertex : contour.vertices)
                    vertices.Push (Point (vertex.x, vertex.y, vertex.z));
                contours.Push (GS::ObjectState ("hole", contour.hole, "vertices", vertices));
            }
            for (const auto &face : piece.sources)
                faces.Push (GS::ObjectState (
                    "baseGuid",
                    APIGuidToString (face.baseGuid),
                    "baseType",
                    static_cast<Int32> (face.baseType),
                    "geometryGuid",
                    APIGuidToString (face.geometryGuid),
                    "body",
                    face.bodyIndex,
                    "polygon",
                    face.polygonIndex,
                    "origin",
                    face.origin == Roombook::RoomSurfaceOrigin::PreparedAnalytic ? "PreparedAnalytic" : "Native3D",
                    "analyticFaceId",
                    face.analyticFaceId,
                    "preparedWallIndex",
                    face.preparedWallIndex,
                    "modelMaterialIndex",
                    AttributeIndexValue (face.modelMaterialIndex)));
            pieces.Push (GS::ObjectState ("zoneBody",
                                          piece.zoneBodyIndex,
                                          "zonePolygon",
                                          piece.zonePolygonIndex,
                                          "status",
                                          status,
                                          "baseGuid",
                                          APIGuidToString (piece.baseGuid),
                                          "baseType",
                                          static_cast<Int32> (piece.baseType),
                                          "area",
                                          piece.area,
                                          "normalWorld",
                                          Point (piece.normal.x, piece.normal.y, piece.normal.z),
                                          "planeOffset",
                                          piece.planeOffset,
                                          "contours",
                                          contours,
                                          "sources",
                                          faces));
        }
        GS::Array<GS::ObjectState> zoneErrors;
        for (const auto &error : zoneSnapshot.errors)
            zoneErrors.Push (GS::ObjectState ("stage",
                                              error.stage,
                                              "code",
                                              error.code,
                                              "body",
                                              error.body,
                                              "polygon",
                                              error.polygon,
                                              "index",
                                              error.index));
        // Отделка балок: низ и бока на участках beamPart, обрезанные по вертикали комнаты.
        API_Element zoneElement = {};
        zoneElement.header.guid = zone.guid;
        const GSErrCode zoneGetErr = ACAPI_Element_Get (&zoneElement);
        RoomSurfaceBeamPartitions beamResult;
        if (zoneGetErr == NoError) {
            double zBottom = 0;
            bool found = false;
            for (const auto &body : zoneSnapshot.bodies)
                for (const auto &surface : body.surfaces) {
                    if (surface.normalWorld.z > -1 + 1e-6 || surface.contours.IsEmpty ())
                        continue;
                    const double z = surface.contours[0].vertices[0].world.z;
                    if (!found || z < zBottom) {
                        zBottom = z;
                        found = true;
                    }
                }
            if (found) {
                const double zTop = zBottom + zoneElement.zone.roomHeight;
                for (const auto &part : candidates.beamParts) {
                    const RoomSurfaceSource *beamSource = nullptr;
                    for (const auto &source : candidates.sources)
                        if (source.baseType == API_BeamID && source.baseGuid == part.guid)
                            beamSource = &source;
                    if (beamSource == nullptr) {
                        RoomSurfaceReadError error;
                        error.stage = "beamPartWithoutSource";
                        error.code = APIERR_BADPARS;
                        beamResult.errors.Push (error);
                        continue;
                    }
                    API_Element beamElement = {};
                    beamElement.header.guid = part.guid;
                    const GSErrCode beamErr = ACAPI_Element_Get (&beamElement);
                    if (beamErr != NoError) {
                        RoomSurfaceReadError error;
                        error.stage = "beamElement";
                        error.code = beamErr;
                        beamResult.errors.Push (error);
                        continue;
                    }
                    beamResult.beams.Push (BuildRoomBeamFinish (*beamSource,
                                                                beamElement.beam.begC,
                                                                beamElement.beam.endC,
                                                                part.tBeg,
                                                                part.tEnd,
                                                                zBottom,
                                                                zTop,
                                                                beamResult,
                                                                &zoneSnapshot));
                }
            } else {
                RoomSurfaceReadError error;
                error.stage = "zoneFloorFace";
                error.code = APIERR_BADPARS;
                beamResult.errors.Push (error);
            }
        } else {
            RoomSurfaceReadError error;
            error.stage = "zoneElement";
            error.code = zoneGetErr;
            beamResult.errors.Push (error);
        }
        GS::Array<GS::ObjectState> beams;
        for (const auto &beam : beamResult.beams) {
            GS::Array<GS::ObjectState> beamPieces;
            for (const auto &piece : beam.pieces) {
                const char *face = "Bottom";
                if (piece.face == RoomSurfaceBeamFace::LeftSide)
                    face = "LeftSide";
                else if (piece.face == RoomSurfaceBeamFace::RightSide)
                    face = "RightSide";
                GS::Array<GS::ObjectState> vertices;
                for (const auto &vertex : piece.vertices)
                    vertices.Push (Point (vertex.x, vertex.y, vertex.z));
                beamPieces.Push (GS::ObjectState ("face", face, "area", piece.area, "vertices", vertices));
            }
            beams.Push (GS::ObjectState ("baseGuid",
                                         APIGuidToString (beam.baseGuid),
                                         "geometryGuid",
                                         APIGuidToString (beam.geometryGuid),
                                         "tBeg",
                                         beam.tBeg,
                                         "tEnd",
                                         beam.tEnd,
                                         "pieces",
                                         beamPieces,
                                         "pieceCount",
                                         beamPieces.GetSize ()));
        }
        GS::Array<GS::ObjectState> beamErrors;
        for (const auto &error : beamResult.errors)
            beamErrors.Push (GS::ObjectState ("stage", error.stage, "code", error.code));
        result.Add ("zoneErrors", zoneErrors);
        result.Add ("sources", sources);
        result.Add ("excludedFinish", excluded);
        result.Add ("candidateErrors", candidateErrors);
        result.Add ("beamFinish", beams);
        result.Add ("beamErrors", beamErrors);
        result.Add ("partitionErrors", partitionErrors);
        result.Add ("pieces", pieces);
        result.Add ("pieceCount", pieces.GetSize ());
        result.Add ("matchedArea", matched);
        result.Add ("unmatchedArea", unmatched);
        result.Add ("ambiguousArea", ambiguous);
        result.Add ("partitionSchemaVersion", 1);
        return result;
    }

    GS::ObjectState Inventory () {
        GS::ObjectState result;
        GS::Array<GS::ObjectState> elements;
        GS::Array<GS::ObjectState> errors;
        for (const API_ElemTypeID type : {API_ZoneID, API_WallID, API_ColumnID}) {
            GS::Array<API_Guid> guids;
            const GSErrCode listErr = ACAPI_Element_GetElemList (type, &guids, APIFilt_None);
            if (listErr != NoError) {
                AddError (errors, "elementList", listErr, 0, 0, static_cast<Int32> (type));
                continue;
            }
            for (const API_Guid &guid : guids) {
                API_Element element = {};
                element.header.guid = guid;
                const GSErrCode getErr = ACAPI_Element_Get (&element);
                if (getErr != NoError) {
                    AddError (errors, "element", getErr, 0, 0, static_cast<Int32> (type));
                    continue;
                }
                // Для Z0 нужны профильные стены, а не полный дорогой дамп всех стен.
                if (type == API_WallID && element.wall.modelElemStructureType != API_ProfileStructure)
                    continue;
                GS::ObjectState item ("guid",
                                      APIGuidToString (guid),
                                      "type",
                                      static_cast<Int32> (type),
                                      "floor",
                                      element.header.floorInd,
                                      "layer",
                                      AttributeIndexValue (element.header.layer));
                if (type == API_ZoneID) {
                    item.Add ("name", GS::UniString (element.zone.roomName));
                    item.Add ("number", GS::UniString (element.zone.roomNoStr));
                    item.Add ("manual", element.zone.manual);
                    item.Add ("referenceLine", element.zone.refLineFlag);
                    item.Add ("height", element.zone.roomHeight);
                } else if (type == API_WallID) {
                    item.Add ("profile", AttributeIndexValue (element.wall.profileAttr));
                    item.Add ("zoneRelation", static_cast<Int32> (element.wall.zoneRel));
                    item.Add ("beg", Point (element.wall.begC.x, element.wall.begC.y, 0));
                    item.Add ("end", Point (element.wall.endC.x, element.wall.endC.y, 0));
                } else {
                    item.Add ("segmentCount", element.column.nSegments);
                    item.Add ("zoneRelation", static_cast<Int32> (element.column.zoneRel));
                    item.Add ("origin", Point (element.column.origoPos.x, element.column.origoPos.y, 0));
                    API_ElementMemo memo = {};
                    const GSErrCode memoErr = ACAPI_Element_GetMemo (guid, &memo, APIMemoMask_ColumnSegment);
                    GS::Array<GS::ObjectState> segments;
                    if (memoErr == NoError && memo.columnSegments != nullptr) {
                        const GSSize count = BMGetPtrSize (reinterpret_cast<GSPtr> (memo.columnSegments)) /
                                             sizeof (API_ColumnSegmentType);
                        for (GSSize i = 0; i < count; ++i) {
                            const API_ColumnSegmentType &segment = memo.columnSegments[i];
                            const API_AssemblySegmentData &data = segment.assemblySegmentData;
                            segments.Push (GS::ObjectState ("guid",
                                                            APIGuidToString (segment.head.guid),
                                                            "owner",
                                                            APIGuidToString (segment.owner),
                                                            "structure",
                                                            static_cast<Int32> (data.modelElemStructureType),
                                                            "material",
                                                            AttributeIndexValue (data.buildingMaterial),
                                                            "profile",
                                                            AttributeIndexValue (data.profileAttr)));
                        }
                    }
                    item.Add ("memoError", memoErr);
                    item.Add ("segments", segments);
                    ACAPI_DisposeElemMemoHdls (&memo);
                }
                elements.Push (item);
            }
        }
        result.Add ("elements", elements);
        result.Add ("errors", errors);
        return result;
    }
} // namespace

GS::ObjectState DrawRoomSurfaceWalls (const GS::ObjectState &parameters) {
    GS::UniString zoneText;
    GS::Array<GS::ObjectState> inputs;
    if (!parameters.Get ("zoneGuid", zoneText) || !parameters.Get ("surfaceWalls", inputs) || inputs.IsEmpty ())
        return GS::ObjectState ("status", "InputError", "createdCount", 0);
    API_Elem_Head zone = {};
    zone.guid = APIGuidFromString (zoneText.ToCStr ().Get ());
    const GSErrCode zoneErr = ACAPI_Element_GetHeader (&zone);
    if (zoneErr != NoError || GetElemTypeID (zone) != API_ZoneID)
        return GS::ObjectState ("status", "ZoneReadError", "error", zoneErr, "createdCount", 0);
    const Stories stories = GetStories ();
    if (stories.IsEmpty ())
        return GS::ObjectState ("status", "StoryReadError", "createdCount", 0);

    GS::Array<Roombook::OtdWall> walls;
    for (const GS::ObjectState &input : inputs) {
        double bx = 0, by = 0, ex = 0, ey = 0, bottom = 0, height = 0, angle = 0;
        Int32 surface = 0;
        GS::UniString parentText;
        if (!input.Get ("begX", bx) || !input.Get ("begY", by) || !input.Get ("endX", ex) || !input.Get ("endY", ey) ||
            !input.Get ("zBottom", bottom) || !input.Get ("height", height) || !input.Get ("slantAngle", angle) ||
            !input.Get ("surface", surface) || !input.Get ("parentGuid", parentText) || !std::isfinite (bx) ||
            !std::isfinite (by) || !std::isfinite (ex) || !std::isfinite (ey) || !std::isfinite (bottom) ||
            !std::isfinite (height) || !std::isfinite (angle) || height < Roombook::min_dim || angle <= 0 ||
            angle >= 180 * DEGRAD || surface <= 0 || surface > 32767)
            return GS::ObjectState ("status", "InputError", "inputIndex", walls.GetSize (), "createdCount", 0);
        API_Elem_Head parent = {};
        parent.guid = APIGuidFromString (parentText.ToCStr ().Get ());
        const GSErrCode parentErr = ACAPI_Element_GetHeader (&parent);
        if (parentErr != NoError)
            return GS::ObjectState ("status", "ParentReadError", "error", parentErr, "createdCount", 0);
        Roombook::OtdWall wall;
        const Sector edge = {{bx, by}, {ex, ey}};
        if (!Roombook::OtdWall_Add_One (parent.guid, edge, false, height, bottom, zone.floorInd, 0, wall))
            return GS::ObjectState ("status", "InvalidEdge", "inputIndex", walls.GetSize (), "createdCount", 0);
        wall.base_type = GetElemTypeID (parent);
        wall.type = GetElemTypeID (parent) == API_ColumnID ? Roombook::Column : Roombook::Wall_Main;
        wall.slant_angle = angle;
        wall.material.material = static_cast<short> (surface);
        wall.favorite.type = API_WallID;
        input.Get ("favorite", wall.favorite.name);
        walls.Push (std::move (wall));
    }

    bool isolateFinish = false;
    if (parameters.Contains ("isolateFinish") && !parameters.Get ("isolateFinish", isolateFinish))
        return GS::ObjectState ("status", "InputError", "createdCount", 0);
    bool writeLinks = false;
    if (parameters.Contains ("writeLinks") && !parameters.Get ("writeLinks", writeLinks))
        return GS::ObjectState ("status", "InputError", "createdCount", 0);
    ClassificationFunc::ClassificationDict finclass;
    UnicGuid finclassguids;
    if (writeLinks) {
        if (!ParamHelpers::isPropertyDefinitionRead ())
            return GS::ObjectState ("status", "PropertyDefinitionReadError", "createdCount", 0);
        Roombook::Class_FindFinClass (finclass, finclassguids);
        for (const Roombook::OtdWall &wall : walls) {
            if (Roombook::Class_GetClassGuid (wall.type, finclass) == APINULLGuid)
                return GS::ObjectState ("status", "FinishClassificationNotFound", "createdCount", 0);
        }
    }
    API_Attribute finishLayer = {};
    if (isolateFinish) {
        // Создаём только отдельный слой отделки; существующий одноимённый слой не изменяем.
        GS::UniString layerName ("SomeStuff surface finish (no intersections)");
        finishLayer.header.typeID = API_LayerID;
        finishLayer.header.uniStringNamePtr = &layerName;
        finishLayer.layer.conClassId = 0;
        const GSErrCode layerErr = ACAPI_Attribute_Create (&finishLayer, nullptr);
        finishLayer.header.uniStringNamePtr = nullptr;
        if (layerErr != NoError && layerErr != APIERR_ATTREXIST)
            return GS::ObjectState ("status", "LayerCreateError", "error", layerErr, "createdCount", 0);
        const GSErrCode readErr = ACAPI_Attribute_Get (&finishLayer);
        if (readErr != NoError || finishLayer.layer.conClassId != 0 ||
            (finishLayer.header.flags & (APILay_Hidden | APILay_Locked | APILay_NotMine)) != 0)
            return GS::ObjectState ("status", "LayerReadError", "error", readErr, "createdCount", 0);
    }

    // Отдельный TESTING-вход не заменяет обработку правил и состава в обычном RoomBook.
    // Построение, избранное, покрытия и регистрация связи остаются в штатном writer.
    Roombook::UnicElementByType subelementByparent;
    ParamDictElement paramToWrite;
    UInt32 failedIndex = 0;
    const GSErrCode createErr = ACAPI_CallUndoableCommand (GS::UniString ("Surface finish walls"), [&] () -> GSErrCode {
        for (Roombook::OtdWall &wall : walls) {
            Roombook::OtdWall_Draw (stories, wall, subelementByparent);
            if (wall.otd_guid == APINULLGuid)
                return APIERR_GENERAL;
            if (isolateFinish) {
                API_Element element = {};
                element.header.guid = wall.otd_guid;
                const GSErrCode readErr = ACAPI_Element_Get (&element);
                if (readErr != NoError)
                    return readErr;
                API_Element mask = {};
                ACAPI_ELEMENT_MASK_SET (mask, API_Elem_Head, layer);
                element.header.layer = finishLayer.header.index;
                const GSErrCode layerErr = ACAPI_Element_Change (&element, &mask, nullptr, 0, true);
                if (layerErr != NoError)
                    return layerErr;
            }
            if (writeLinks) {
                Roombook::Class_SetClass (wall, finclass);
                GS::Array<API_Guid> subguids;
                subguids.Push (wall.otd_guid);
                API_Elem_Head parent = {};
                parent.guid = wall.base_guid;
                if (!SyncSetSubelementScope (parent, subguids, paramToWrite, "base element", false) ||
                    !SyncSetSubelementScope (zone, subguids, paramToWrite, "zone", false))
                    return APIERR_GENERAL;
            }
            ++failedIndex;
        }
        // Записываем только связи новых стен; полный SetSyncOtdWall дополнительно запускает произвольные Sync-правила.
        if (writeLinks)
            ParamHelpers::ElementsWrite (paramToWrite);
        return NoError;
    });
    if (createErr != NoError)
        return GS::ObjectState (
            "status", "WriterError", "error", createErr, "inputIndex", failedIndex, "createdCount", 0);
    GS::Array<GS::ObjectState> created;
    for (const Roombook::OtdWall &wall : walls) {
        API_Elem_Head head = {};
        head.guid = wall.otd_guid;
        const GSErrCode readErr = ACAPI_Element_GetHeader (&head);
        created.Push (GS::ObjectState ("guid",
                                       APIGuidToString (wall.otd_guid),
                                       "parentGuid",
                                       APIGuidToString (wall.base_guid),
                                       "zoneGuid",
                                       APIGuidToString (zone.guid),
                                       "readError",
                                       readErr,
                                       "type",
                                       static_cast<Int32> (GetElemTypeID (head)),
                                       "layerIndex",
                                       AttributeIndexValue (head.layer)));
    }
    DBprnt (static_cast<double> (created.GetSize ()), "Z4 created surface walls");
    GS::ObjectState response ("status", "Created", "createdCount", created.GetSize ());
    response.Add ("created", created);
    // Факт записи подтверждается отдельным чтением свойств, а не возвратом void/writer.
    response.Add ("linksAttempted", writeLinks);
    return response;
}

RoomGeometryDiagnosticCommand::RoomGeometryDiagnosticCommand () : CommandBase (CommonSchema::NotUsed) {}

GS::String RoomGeometryDiagnosticCommand::GetName () const { return "RoomGeometryDiagnostic"; }

GS::Optional<GS::UniString> RoomGeometryDiagnosticCommand::GetInputParametersSchema () const {
    return GS::UniString (
        R"({"type":"object","properties":{"favoriteName":{"type":"string"},"guid":{"type":"string","pattern":"^[0-9A-Fa-f]{8}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{12}$"},"memoOnly":{"type":"boolean"},"collectRoomInfo":{"type":"boolean"},"transformSelfTest":{"type":"boolean"},"partitionSelfTest":{"type":"boolean"},"partitionSurfaces":{"type":"boolean"}},"additionalProperties":false})");
}

GS::Optional<GS::UniString> RoomGeometryDiagnosticCommand::GetResponseSchema () const { return GS::NoValue; }

GS::ObjectState RoomGeometryDiagnosticCommand::Execute (const GS::ObjectState &parameters,
                                                        GS::ProcessControl & /*processControl*/) const {
    bool partitionSelfTest = false;
    parameters.Get ("partitionSelfTest", partitionSelfTest);
    if (partitionSelfTest)
        return RunSurfacePartitionChecks ();
    bool transformSelfTest = false;
    parameters.Get ("transformSelfTest", transformSelfTest);
    if (transformSelfTest)
        return RunSurfaceTransformChecks ();
    GS::UniString favoriteName;
    if (parameters.Get ("favoriteName", favoriteName))
        return ReadFavorite (favoriteName);
    bool partitionSurfaces = false;
    parameters.Get ("partitionSurfaces", partitionSurfaces);
    GS::UniString guidText;
    if (!parameters.Get ("guid", guidText)) {
        if (partitionSurfaces)
            return GS::ObjectState ("status", "InvalidInput", "partitionComplete", false, "error", APIERR_BADPARS);
        return Inventory ();
    }
    API_Elem_Head head = {};
    head.guid = APIGuidFromString (guidText.ToCStr ().Get ());
    const GSErrCode err = ACAPI_Element_GetHeader (&head);
    if (err != NoError)
        return GS::ObjectState ("status", "ReadError", "readComplete", false, "headerError", err);
    bool memoOnly = false;
    parameters.Get ("memoOnly", memoOnly);
    if (memoOnly)
        return GS::ObjectState ("geometryRoute", "MemoOnly", "structure", ReadStructure (head.guid));
    if (partitionSurfaces)
        return ReadZoneSurfacePartition (head);
    bool collectRoomInfo = false;
    parameters.Get ("collectRoomInfo", collectRoomInfo);
    if (collectRoomInfo) {
        if (GetElemTypeID (head) != API_ZoneID)
            return GS::ObjectState ("status", "InvalidInput", "readComplete", false, "error", APIERR_BADID);
        Roombook::OtdRoom room;
        Roombook::UnicElementByType elements;
        GS::HashTable<API_Guid, GS::Array<API_Guid>> slabs;
        const Stories stories = GetStories ();
        const bool collected = Roombook::CollectRoomInfo (stories, head.guid, room, elements, slabs);
        GS::ObjectState result = GetZone3DPolygons_StrictAPI (head, room.surfaceSnapshot);
        result.Add ("surfaceSnapshotStored",
                    room.surfaceSnapshot.guid == head.guid &&
                        room.surfaceSnapshot.status != Roombook::RoomSurfaceStatus::NotRead);
        result.Add ("legacyCollectResult", collected);
        result.Add ("legacyRoomValid", room.isValid);
        result.Add ("legacyEdgeCount", room.edges.GetSize ());
        result.Add ("finishWallCount", room.otdwall.GetSize ());
        result.Add ("finishSlabCount", room.otdslab.GetSize ());
        DBprnt (static_cast<double> (room.surfaceSnapshot.bodies.GetSize ()), "Z5 OtdRoom surface snapshot bodies");
        return result;
    }
    return GetZone3DPolygons_StrictAPI (head, Roombook::ReadRoomSurfaces (head));
}

#endif
