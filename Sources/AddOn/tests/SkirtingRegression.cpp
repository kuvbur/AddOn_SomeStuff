#if defined(TESTING)

    // Типы Roombook::OtdWall / OtdOpening / RoomSurface объявлены в
    // Roombook.hpp только для Archicad >= 24; для AC 22 файл отключается.
    #if defined(ServerMainVers_2400)

    #include <algorithm>
    #include <cmath>
    #include <cstdio>
    #include <cstring>
    #include <initializer_list>
    #include <limits>

    #include "roombook/Roombook.hpp"

namespace {
    bool Equal (double a, double b) { return std::fabs (a - b) < 1e-8; }

    Roombook::OtdWall Wall () {
        Roombook::OtdWall wall;
        wall.draw.type = Roombook::Wall_Main;
        wall.geom.seg.begC = {10, 20, 5};
        wall.geom.seg.endC = {20, 20, 5};
        wall.geom.height = 3;
        wall.geom.floorInd = 2;
        wall.draw.base_type = API_WallID;
        return wall;
    }

    Roombook::OtdOpening Door () {
        Roombook::OtdOpening opening;
        opening.objLoc = 3;
        opening.width = 2;
        opening.zBottom = 5;
        opening.height = 2;
        return opening;
    }

    Roombook::RoomSurface Face (API_Coord3D normal, std::initializer_list<API_Coord3D> points) {
        Roombook::RoomSurface face;
        face.normalWorld = normal;
        Roombook::RoomSurfaceContour contour;
        for (const auto &point : points) {
            Roombook::RoomSurfaceVertex vertex;
            vertex.world = point;
            contour.vertices.Push (vertex);
        }
        face.contours.Push (contour);
        const auto &p = face.contours[0].vertices[0].world;
        face.planeOffset = -(normal.x * p.x + normal.y * p.y + normal.z * p.z);
        return face;
    }

    void Hole (Roombook::RoomSurface &face, std::initializer_list<API_Coord3D> points) {
        Roombook::RoomSurfaceContour contour;
        contour.hole = true;
        for (const auto &point : points) {
            Roombook::RoomSurfaceVertex vertex;
            vertex.world = point;
            contour.vertices.Push (vertex);
        }
        face.contours.Push (contour);
    }

    Roombook::RoomSurface WallFace () { return Face ({0, -1, 0}, {{0, 0, 0}, {10, 0, 0}, {10, 0, 3}, {0, 0, 3}}); }

    Roombook::RoomSurface SlabFace (double z, bool ceiling = false) {
        return Face ({0, 0, ceiling ? -1.0 : 1.0}, {{-1, -1, z}, {11, -1, z}, {11, 1, z}, {-1, 1, z}});
    }

    double Length (const GS::Array<Roombook::OtdLinearSegment> &segments) {
        double result = 0;
        for (const auto &segment : segments)
            result += std::hypot (std::hypot (segment.endC.x - segment.begC.x, segment.endC.y - segment.begC.y),
                                  segment.endC.z - segment.begC.z);
        return result;
    }

    void MoveFace (Roombook::RoomSurface &face) {
        const double c = std::cos (0.7), s = std::sin (0.7);
        for (auto &contour : face.contours)
            for (auto &vertex : contour.vertices) {
                const auto p = vertex.world;
                vertex.world = {1e6 + c * p.x - s * p.y, -2e6 + s * p.x + c * p.y, 1000 + p.z};
            }
        const auto n = face.normalWorld;
        face.normalWorld = {c * n.x - s * n.y, s * n.x + c * n.y, n.z};
        const auto &p = face.contours[0].vertices[0].world;
        const auto &normal = face.normalWorld;
        face.planeOffset = -(normal.x * p.x + normal.y * p.y + normal.z * p.z);
    }
} // namespace

int main () {
    using namespace Roombook;
    int passed = 0, failed = 0;
    const auto check = [&] (bool ok, const char *name) {
        std::printf ("%s: %s\n", ok ? "PASS" : "FAIL", name);
        ok ? ++passed : ++failed;
    };
    OtdRoom room;
    room.tip_floor_skirting = "floor";
    room.zBottom = 5;
    check (room.otdbeam_floor.IsEmpty () && room.otdbeam_ceil.IsEmpty (), "both arrays start empty");
    OtdBeam ceiling;
    OtdLinearSegment_SetZBottom (ceiling.geom, 8);
    room.otdbeam_ceil.Push (ceiling);
    room.otdwall.Push (Wall ());
    check (FloorSkirtingSegments_Build (room) && room.otdbeam_floor.GetSize () == 1, "uncut prepared wall");
    check (Equal (room.otdbeam_floor[0].geom.seg.begC.x, 10) && Equal (room.otdbeam_floor[0].geom.seg.endC.x, 20) &&
               Equal (room.otdbeam_floor[0].geom.seg.begC.y, 20) &&
               Equal (OtdLinearSegment_GetZBottom (room.otdbeam_floor[0].geom), 5),
           "prepared coordinates and absolute bottom copied");
    check (room.otdbeam_floor[0].geom.floorInd == 2 && room.otdbeam_floor[0].draw.base_type == API_WallID &&
               room.otdbeam_floor[0].draw.base_guid == room.otdwall[0].draw.base_guid &&
               room.otdbeam_floor[0].wallIndex == 0 && room.otdbeam_floor[0].draw.otd_guid == APINULLGuid,
           "source metadata retained without inventing a created guid");
    room.otdwall[0].openings.Push (Door ());
    check (FloorSkirtingSegments_Build (room) && room.otdbeam_floor.GetSize () == 2 &&
               Equal (room.otdbeam_floor[0].geom.seg.endC.x, 12) && Equal (room.otdbeam_floor[1].geom.seg.begC.x, 14),
           "null-owner geometric door cuts actual wall bottom");
    check (FloorSkirtingSegments_Build (room) && room.otdbeam_floor.GetSize () == 2,
           "repeat replaces rather than appends");
    room.tip_floor_skirting.Clear ();
    check (FloorSkirtingSegments_Build (room) && room.otdbeam_floor.IsEmpty () && room.otdbeam_ceil.GetSize () == 1,
           "empty floor type clears stale floor without touching ceiling");
    room.tip_floor_skirting = "floor";
    room.otdwall[0].openings[0].lower = 100;
    room.otdwall[0].rise_x = 2;
    room.otdwall[0].surface_geometry = true;
    check (FloorSkirtingSegments_Build (room) && room.otdbeam_floor.GetSize () == 2 &&
               Equal (room.otdbeam_floor[0].geom.seg.begC.x, 10) && Equal (room.otdbeam_floor[1].geom.seg.begC.x, 14),
           "no second rise shift and absolute opening bottom used");
    check (room.otdwall.GetSize () == 1 && Equal (room.otdwall[0].geom.seg.begC.x, 10) &&
               Equal (room.otdwall[0].geom.seg.endC.x, 20) && Equal (room.otdwall[0].openings[0].lower, 100) &&
               room.otdbeam_ceil.GetSize () == 1 && Equal (OtdLinearSegment_GetZBottom (room.otdbeam_ceil[0].geom), 8),
           "source wall openings and ceiling array untouched");
    room.otdwall[0].geom.seg.begC = {20, 20};
    room.otdwall[0].geom.seg.endC = {10, 20};
    check (FloorSkirtingSegments_Build (room) && room.otdbeam_floor.GetSize () == 2 &&
               Equal (room.otdbeam_floor[0].geom.seg.begC.x, 20) && Equal (room.otdbeam_floor[0].geom.seg.endC.x, 18) &&
               Equal (room.otdbeam_floor[1].geom.seg.begC.x, 16) && Equal (room.otdbeam_floor[1].geom.seg.endC.x, 10),
           "reversed wall preserves direction and opening location");
    room.otdwall[0].geom.seg.begC = {10, 20};
    room.otdwall[0].geom.seg.endC = {16, 28};
    check (FloorSkirtingSegments_Build (room) && room.otdbeam_floor.GetSize () == 2 &&
               Equal (room.otdbeam_floor[0].geom.seg.endC.x, 11.2) &&
               Equal (room.otdbeam_floor[0].geom.seg.endC.y, 21.6) &&
               Equal (room.otdbeam_floor[1].geom.seg.begC.x, 12.4) &&
               Equal (room.otdbeam_floor[1].geom.seg.begC.y, 23.2),
           "diagonal wall intervals map to world coordinates");
    OtdLinearSegment_SetZBottom (room.otdwall[0].geom, 4);
    check (FloorSkirtingSegments_Build (room) && room.otdbeam_floor.IsEmpty (),
           "spanning wall below zone bottom is not selected");
    room.otdwall[0] = Wall ();
    OtdLinearSegment_SetZBottom (room.otdwall[0].geom, OtdLinearSegment_GetZBottom (room.otdwall[0].geom) + 5e-7);
    check (FloorSkirtingSegments_Build (room) && room.otdbeam_floor.GetSize () == 1,
           "zone bottom comparison uses distance tolerance");
    OtdLinearSegment_SetZBottom (room.otdwall[0].geom, 5 + 2e-6);
    check (FloorSkirtingSegments_Build (room) && room.otdbeam_floor.IsEmpty (),
           "wall above bottom outside tolerance is excluded");
    room.otdwall[0] = Wall ();
    room.create_wall_elements = false;
    check (FloorSkirtingSegments_Build (room) && room.otdbeam_floor.IsEmpty (), "disabled wall writer excluded");
    room.otdwall[0].draw.type = Column;
    check (FloorSkirtingSegments_Build (room) && room.otdbeam_floor.GetSize () == 1,
           "enabled column retained independently of wall flag");
    room.create_column_elements = false;
    check (FloorSkirtingSegments_Build (room) && room.otdbeam_floor.IsEmpty (), "disabled column excluded");
    room.create_wall_elements = true;
    room.otdwall.Clear ();
    for (TypeOtd type : {Reveal_Main, Reveal_Up, Reveal_Down, Floor, Ceil}) {
        OtdWall wall = Wall ();
        wall.draw.type = type;
        room.otdwall.Push (wall);
    }
    check (FloorSkirtingSegments_Build (room) && room.otdbeam_floor.IsEmpty (), "reveals and slab sides excluded");
    room.otdwall.Push (Wall ());
    room.otdwall.Push (Wall ());
    room.otdwall[6].isValid = false;
    check (FloorSkirtingSegments_Build (room) && room.otdbeam_floor.GetSize () == 1 &&
               room.otdbeam_floor[0].wallIndex == 5,
           "invalid wall excluded and source index is not output index");
    room.isValid = false;
    check (FloorSkirtingSegments_Build (room) && room.otdbeam_floor.IsEmpty (), "invalid room clears stale result");
    room.isValid = true;
    room.otdwall.Clear ();
    room.otdwall.Push (Wall ());
    OtdWall broken = Wall ();
    broken.openings.Push (Door ());
    broken.openings[0].width = 0;
    room.otdwall.Push (broken);
    check (!FloorSkirtingSegments_Build (room) && room.otdbeam_floor.IsEmpty (),
           "invalid second wall opening publishes no partial room");
    room.otdwall.Clear ();
    room.otdwall.Push (Wall ());
    room.otdwall[0].geom.seg.endC = room.otdwall[0].geom.seg.begC;
    check (!FloorSkirtingSegments_Build (room) && room.otdbeam_floor.IsEmpty (), "zero length fails safely");
    room.otdwall[0] = Wall ();
    OtdLinearSegment_SetZBottom (room.otdwall[0].geom, std::numeric_limits<double>::quiet_NaN ());
    check (!FloorSkirtingSegments_Build (room) && room.otdbeam_floor.IsEmpty (), "nonfinite wall fails safely");
    room.otdwall[0] = Wall ();
    OtdLinearSegment_SetZBottom (room.otdwall[0].geom, std::numeric_limits<double>::infinity ());
    check (!FloorSkirtingSegments_Build (room) && room.otdbeam_floor.IsEmpty (), "nonfinite room fails safely");
    {
        RoomSurface wall = WallFace ();
        RoomSurface slab = SlabFace (0);
        GS::Array<OtdLinearSegment> joints;
        check (SkirtingContact_Build (wall, slab, joints) && joints.GetSize () == 1 && Equal (Length (joints), 10),
               "finite wall bottom inside extended floor rather than only floor border");
        check (joints.GetSize () == 1 && Equal (joints[0].begC.y, 0) && Equal (joints[0].endC.z, 0),
               "joint uses actual common spatial line");
        check (SkirtingContact_Build (wall, slab, joints) && joints.GetSize () == 1 && Equal (Length (joints), 10),
               "contact repeat replaces output");
        slab = Face ({0, 0, 1}, {{2, -1, 0}, {8, -1, 0}, {8, 1, 0}, {2, 1, 0}});
        check (SkirtingContact_Build (wall, slab, joints) && joints.GetSize () == 1 && Equal (Length (joints), 6),
               "finite floor clips both ends of wall contact");
        slab = SlabFace (-0.01);
        check (SkirtingContact_Build (wall, slab, joints) && joints.IsEmpty (), "vertical gap is not XY contact");
        slab = SlabFace (4);
        check (SkirtingContact_Build (wall, slab, joints) && joints.IsEmpty (), "floor beyond wall top has no contact");
        slab = Face ({0, 0, 1}, {{-1, 1, 0}, {11, 1, 0}, {11, 2, 0}, {-1, 2, 0}});
        check (SkirtingContact_Build (wall, slab, joints) && joints.IsEmpty (),
               "local absence of floor has no contact");
        slab = SlabFace (3, true);
        check (SkirtingContact_Build (wall, slab, joints) && joints.GetSize () == 1 && Equal (Length (joints), 10),
               "ceiling underside shares same finite joint algorithm");
        check (joints.GetSize () == 1 && Equal (joints[0].begC.z, 3) && Equal (joints[0].endC.z, 3),
               "ceiling contact uses actual underside elevation");
        slab = SlabFace (1);
        Hole (wall, {{3, 0, 0.5}, {5, 0, 0.5}, {5, 0, 2}, {3, 0, 2}});
        Hole (slab, {{7, -0.5, 1}, {9, -0.5, 1}, {9, 0.5, 1}, {7, 0.5, 1}});
        check (SkirtingContact_Build (wall, slab, joints) && joints.GetSize () == 3 && Equal (Length (joints), 6),
               "both wall opening and floor hole cut the same joint");
        for (const auto &joint : joints) {
            const double lo = std::min (joint.begC.x, joint.endC.x), hi = std::max (joint.begC.x, joint.endC.x);
            check ((lo >= -1e-8 && hi <= 3 + 1e-8) || (lo >= 5 - 1e-8 && hi <= 7 + 1e-8) ||
                       (lo >= 9 - 1e-8 && hi <= 10 + 1e-8),
                   "individual joint does not bridge either opening");
        }
        check (wall.contours.GetSize () == 2 && slab.contours.GetSize () == 2 &&
                   Equal (wall.contours[1].vertices[0].world.x, 3) && Equal (slab.contours[1].vertices[0].world.x, 7),
               "contact leaves both input faces unchanged");
        MoveFace (wall);
        MoveFace (slab);
        check (SkirtingContact_Build (wall, slab, joints) && joints.GetSize () == 3 &&
                   std::fabs (Length (joints) - 6) < 1e-7,
               "rotated distant room retains both finite holes");
        wall = Face ({0, -1, 0},
                     {{0, 0, 0}, {2, 0, 0}, {2, 0, 2}, {4, 0, 2}, {4, 0, 0}, {10, 0, 0}, {10, 0, 3}, {0, 0, 3}});
        slab = SlabFace (0);
        check (SkirtingContact_Build (wall, slab, joints) && joints.GetSize () == 2 && Equal (Length (joints), 8),
               "door notch reaching wall bottom interrupts contact");
        wall = WallFace ();
        Hole (wall, {{3, 0, 1}, {5, 0, 1}, {5, 0, 2}, {3, 0, 2}});
        check (SkirtingContact_Build (wall, slab, joints) && joints.GetSize () == 1 && Equal (Length (joints), 10),
               "elevated window does not interrupt floor contact");
        slab = SlabFace (1.5, true);
        check (SkirtingContact_Build (wall, slab, joints) && joints.GetSize () == 2 && Equal (Length (joints), 8),
               "same window interrupts actual lowered ceiling contact");
        wall = WallFace ();
        slab = Face ({0, 0, 1}, {{10, -1, 0}, {11, -1, 0}, {11, 1, 0}, {10, 1, 0}});
        check (SkirtingContact_Build (wall, slab, joints) && joints.IsEmpty (), "point-only touch creates no length");
        slab = Face ({0, 0, 1}, {{0, 0, 0}, {10, 0, 0}, {10, 1, 0}, {0, 1, 0}});
        check (SkirtingContact_Build (wall, slab, joints) && joints.GetSize () == 1 && Equal (Length (joints), 10),
               "coincident floor boundary remains valid contact");
        const double slopeLength = std::hypot (1.0, 0.2);
        slab = Face ({-0.2 / slopeLength, 0, 1 / slopeLength},
                     {{-1, -1, -0.2}, {11, -1, 2.2}, {11, 1, 2.2}, {-1, 1, -0.2}});
        check (SkirtingContact_Build (wall, slab, joints) && joints.GetSize () == 1 &&
                   Equal (Length (joints), std::hypot (10.0, 2.0)),
               "sloped floor joint length is spatial not XY projection");
        check (joints.GetSize () == 1 && Equal (joints[0].begC.z, 0.2 * joints[0].begC.x) &&
                   Equal (joints[0].endC.z, 0.2 * joints[0].endC.x),
               "sloped joint endpoints lie on actual floor plane");
        const double wallNormalLength = std::hypot (1.0, 0.25);
        wall = Face ({0, -1 / wallNormalLength, 0.25 / wallNormalLength},
                     {{0, 0, 0}, {10, 0, 0}, {10, 0.75, 3}, {0, 0.75, 3}});
        slab = SlabFace (2);
        check (SkirtingContact_Build (wall, slab, joints) && joints.GetSize () == 1 && Equal (Length (joints), 10),
               "slanted wall meets floor at changed XY section");
        check (joints.GetSize () == 1 && Equal (joints[0].begC.y, 0.5) && Equal (joints[0].endC.y, 0.5),
               "slanted wall contact not replaced by original bottom line");
        wall = WallFace ();
        wall.normalWorld.y = 1;
        wall.planeOffset = 0;
        check (SkirtingContact_Build (wall, slab, joints) && Equal (Length (joints), 10),
               "opposite wall normal preserves contact coverage");
        RoomSurface broken = wall;
        broken.contours.Clear ();
        check (!SkirtingContact_Build (broken, slab, joints) && joints.IsEmpty (),
               "missing wall face clears stale joints");
        broken = slab;
        broken.contours.Clear ();
        check (!SkirtingContact_Build (wall, broken, joints) && joints.IsEmpty (),
               "missing slab face fails not valid empty");
        broken = slab;
        broken.contours[0].vertices[1].world.z += 1e-5;
        check (!SkirtingContact_Build (wall, broken, joints) && joints.IsEmpty (),
               "nonplanar face cannot fabricate contact");
        broken = slab;
        broken.planeOffset += 1e-5;
        check (!SkirtingContact_Build (wall, broken, joints) && joints.IsEmpty (),
               "stored plane inconsistent with vertices rejected");
        broken = wall;
        broken.normalWorld.y = std::numeric_limits<double>::quiet_NaN ();
        check (!SkirtingContact_Build (broken, slab, joints) && joints.IsEmpty (), "nonfinite normal fails safely");
        broken = wall;
        broken.normalWorld.y = 2;
        check (!SkirtingContact_Build (broken, slab, joints) && joints.IsEmpty (), "nonunit snapshot normal rejected");
        broken = slab;
        broken.contours[0].hole = true;
        check (!SkirtingContact_Build (wall, broken, joints) && joints.IsEmpty (),
               "hole without outer contour rejected");
        check (!SkirtingContact_Build (wall, wall, joints) && joints.IsEmpty (),
               "coplanar faces have no unique skirting line");
        broken = WallFace ();
        for (auto &vertex : broken.contours[0].vertices)
            vertex.world.y = 1;
        broken.planeOffset = 1;
        check (!SkirtingContact_Build (wall, broken, joints) && joints.IsEmpty (),
               "parallel faces reported unsupported explicitly");
        check (SkirtingContact_Build (WallFace (), SlabFace (0), joints) && Equal (Length (joints), 10),
               "valid call after failures recovers without stale partial result");
    }
    check (OtdBeam ().draw.favorite.name.IsEmpty (), "room-independent beam favorite is not a stored default");
    {
        OtdBeam floorBeam;
        check (floorBeam.draw.favorite.name.IsEmpty () && floorBeam.draw.favorite.type == API_ZombieElemID,
               "beam favorite starts unselected like wall and slab favorites");
        floorBeam.draw.favorite.name = "smstf plinth down";
        floorBeam.draw.favorite.type = API_BeamID;
        OtdBeam ceilingBeam = floorBeam;
        ceilingBeam.draw.favorite.name = "smstf plinth up";
        check (floorBeam.draw.favorite.name == "smstf plinth down" &&
                   ceilingBeam.draw.favorite.name == "smstf plinth up" && floorBeam.draw.favorite.type == API_BeamID &&
                   ceilingBeam.draw.favorite.type == API_BeamID,
               "floor and ceiling beams carry independent selected favorites");
        floorBeam.draw.favorite.name = "custom plinth";
        GS::Array<OtdBeam> selected;
        selected.Push (floorBeam);
        check (selected[0].draw.favorite.name == "custom plinth" && ceilingBeam.draw.favorite.name == "smstf plinth up",
               "beam storage preserves a custom favorite without replacing it with a default");
    }
    {
        const auto rectangle = [] (double left, double right, bool hole) {
            GS::Array<Point2D> points = {{left, 19}, {right, 19}, {right, 21}, {left, 21}};
            GS::Array<UIndex> ends = {3};
            if (hole) {
                for (const Point2D &p : GS::Array<Point2D>{{14, 19.5}, {14, 20.5}, {16, 20.5}, {16, 19.5}})
                    points.Push (p);
                ends.Push (7);
            }
            GS::Array<PolyArcRec> arcs;
            return Geometry::Polygon2D::Create (
                points, arcs, ends, Geometry::PolyCreateFlags_IdxRefInContourEndFromZero)[0];
        };
        const auto beamLength = [] (const GS::Array<OtdBeam> &beams) {
            double length = 0;
            for (const auto &beam : beams)
                length += std::hypot (std::hypot (beam.geom.seg.endC.x - beam.geom.seg.begC.x,
                                                  beam.geom.seg.endC.y - beam.geom.seg.begC.y),
                                      beam.geom.seg.endC.z - beam.geom.seg.begC.z);
            return length;
        };
        OtdRoom prepared;
        prepared.tip_floor_skirting = "floor";
        prepared.tip_ceil_cornice = "ceiling";
        prepared.otdwall.Push (Wall ());
        prepared.otdwall[0].draw.material.smaterial = "@zone wall material@";
        OtdSlab floor;
        floor.draw.type = Floor;
        floor.zBottom = 5.1;
        floor.poly = rectangle (9, 21, false);
        OtdSlab ceil = floor;
        ceil.draw.type = Ceil;
        ceil.zBottom = 7.8;
        prepared.otdslab = {floor, ceil};
        check (SkirtingSegments_Build (prepared, prepared.otdslab) && prepared.otdbeam_floor.GetSize () == 1 &&
                   prepared.otdbeam_ceil.GetSize () == 1,
               "pipeline both independent final arrays");
        check (prepared.otdbeam_floor[0].tip == prepared.tip_floor_skirting &&
                   prepared.otdbeam_ceil[0].tip == prepared.tip_ceil_cornice,
               "pipeline uses corresponding zone type for each role");
        prepared.tip_floor_skirting.Clear ();
        check (SkirtingSegments_Build (prepared, prepared.otdslab) && prepared.otdbeam_floor.IsEmpty () &&
                   prepared.otdbeam_ceil.GetSize () == 1,
               "empty floor type leaves ceiling enabled");
        prepared.tip_ceil_cornice.Clear ();
        check (SkirtingSegments_Build (prepared, prepared.otdslab) && prepared.otdbeam_floor.IsEmpty () &&
                   prepared.otdbeam_ceil.IsEmpty (),
               "both empty types clear both stale arrays");
        prepared.tip_floor_skirting = "floor";
        check (SkirtingSegments_Build (prepared, prepared.otdslab) && prepared.otdbeam_floor.GetSize () == 1 &&
                   prepared.otdbeam_ceil.IsEmpty (),
               "empty ceiling type leaves floor enabled");
        prepared.tip_ceil_cornice = "ceiling";
        check (SkirtingSegments_Build (prepared, prepared.otdslab) && prepared.otdbeam_floor.GetSize () == 1 &&
                   prepared.otdbeam_ceil.GetSize () == 1,
               "restored types rebuild both arrays");
        check (prepared.otdbeam_floor[0].draw.material.smaterial.IsEmpty () &&
                   prepared.otdbeam_ceil[0].draw.material.smaterial.IsEmpty () &&
                   prepared.otdwall[0].draw.material.smaterial == GS::UniString ("@zone wall material@"),
               "skirting does not inherit zone wall material and leaves source unchanged");
        check (Equal (OtdLinearSegment_GetZBottom (prepared.otdbeam_floor[0].geom), 5.1) &&
                   Equal (prepared.otdbeam_floor[0].geom.seg.endC.z, 5.1) &&
                   Equal (OtdLinearSegment_GetZBottom (prepared.otdbeam_ceil[0].geom), 7.8),
               "pipeline exposed floor and lowered ceiling elevations");
        check (prepared.otdbeam_floor[0].wallIndex == 0 && prepared.otdbeam_floor[0].slabIndex == 0 &&
                   prepared.otdbeam_ceil[0].slabIndex == 1 && prepared.otdbeam_ceil[0].draw.type == Ceil_Cornice &&
                   prepared.otdbeam_floor[0].draw.type == Floor_Skirting &&
                   prepared.otdbeam_floor[0].draw.otd_guid == APINULLGuid,
               "pipeline keeps final participant indices and roles");
        prepared.otdwall[0].openings.Push (Door ());
        check (SkirtingSegments_Build (prepared, prepared.otdslab) && Equal (beamLength (prepared.otdbeam_floor), 8) &&
                   Equal (beamLength (prepared.otdbeam_ceil), 10),
               "pipeline door cuts floor but not ceiling");
        prepared.otdslab[0].poly = rectangle (9, 21, true);
        check (SkirtingSegments_Build (prepared, prepared.otdslab) && Equal (beamLength (prepared.otdbeam_floor), 6),
               "pipeline preserves final floor holes");
        prepared.otdslab[0].poly = rectangle (15, 18, false);
        check (SkirtingSegments_Build (prepared, prepared.otdslab) && Equal (beamLength (prepared.otdbeam_floor), 3) &&
                   Equal (beamLength (prepared.otdbeam_ceil), 10),
               "pipeline finite partial floor independent of ceiling");
        prepared.otdslab[0].zBottom = 4.9;
        check (SkirtingSegments_Build (prepared, prepared.otdslab) && prepared.otdbeam_floor.IsEmpty () &&
                   prepared.otdbeam_ceil.GetSize () == 1,
               "pipeline Z gap does not become contact");
        prepared.otdslab[0] = floor;
        prepared.otdwall[0].openings.Clear ();
        prepared.otdslab[0].plane_origin = {10, 20};
        prepared.otdslab[0].slope_x = 0.1;
        check (SkirtingSegments_Build (prepared, prepared.otdslab) &&
                   Equal (OtdLinearSegment_GetZBottom (prepared.otdbeam_floor[0].geom), 5.1) &&
                   Equal (prepared.otdbeam_floor[0].geom.seg.endC.z, 6.1),
               "pipeline sloped floor retains both endpoint elevations");
        prepared.otdwall[0].rise_y = 0.2;
        check (SkirtingSegments_Build (prepared, prepared.otdslab) &&
                   Equal (prepared.otdbeam_floor[0].geom.seg.begC.y, 20.02) &&
                   Equal (prepared.otdbeam_floor[0].geom.seg.endC.y, 20.22),
               "pipeline applies slanted wall rise at actual joint only");
        check (SkirtingSegments_Build (prepared, prepared.otdslab) && prepared.otdbeam_floor.GetSize () == 1 &&
                   prepared.otdbeam_ceil.GetSize () == 1,
               "pipeline repeat replaces both outputs");
        check (Equal (prepared.otdwall[0].geom.seg.begC.y, 20) && Equal (prepared.otdslab[0].zBottom, 5.1) &&
                   Equal (prepared.otdslab[1].zBottom, 7.8),
               "pipeline source walls and slabs remain unchanged");
        prepared.create_wall_elements = false;
        check (SkirtingSegments_Build (prepared, prepared.otdslab) && prepared.otdbeam_floor.IsEmpty () &&
                   prepared.otdbeam_ceil.IsEmpty (),
               "pipeline disabled participant wall excludes both roles");
        prepared.otdwall[0].draw.type = Column;
        check (SkirtingSegments_Build (prepared, prepared.otdslab) && prepared.otdbeam_floor.GetSize () == 1,
               "pipeline enabled column independent of wall flag");
        prepared.create_floor_elements = false;
        check (SkirtingSegments_Build (prepared, prepared.otdslab) && prepared.otdbeam_floor.IsEmpty () &&
                   prepared.otdbeam_ceil.GetSize () == 1,
               "pipeline floor flag does not disable ceiling");
        prepared.create_floor_elements = true;
        prepared.otdslab[1].slope_x = std::numeric_limits<double>::quiet_NaN ();
        check (!SkirtingSegments_Build (prepared, prepared.otdslab) && prepared.otdbeam_floor.IsEmpty () &&
                   prepared.otdbeam_ceil.IsEmpty (),
               "pipeline malformed late ceiling publishes neither array");
        prepared.otdslab[1] = ceil;
        check (SkirtingSegments_Build (prepared, prepared.otdslab) && prepared.otdbeam_floor.GetSize () == 1 &&
                   prepared.otdbeam_ceil.GetSize () == 1,
               "pipeline recovers after rejected input");
        prepared.otdslab.Push (ceil);
        check (SkirtingSegments_Build (prepared, prepared.otdslab) && prepared.otdbeam_ceil.GetSize () == 1,
               "pipeline shared slab seam does not duplicate contact");
        prepared.otdwall.Push (prepared.otdwall[0]);
        check (SkirtingSegments_Build (prepared, prepared.otdslab) && prepared.otdbeam_floor.GetSize () == 1 &&
                   prepared.otdbeam_ceil.GetSize () == 1,
               "pipeline coincident compatible wall seam does not duplicate contact");
        prepared.otdslab.Clear ();
        check (SkirtingSegments_Build (prepared, prepared.otdslab) && prepared.otdbeam_floor.IsEmpty () &&
                   prepared.otdbeam_ceil.IsEmpty (),
               "pipeline no slab never substitutes room contour");
    }
    {
        const TypeOtd lowerRole = static_cast<TypeOtd> (11);
        const TypeOtd upperRole = static_cast<TypeOtd> (12);
        ClassificationFunc::ClassificationDict classes;
        ClassificationFunc::ClassificationValues lower, upper, generic, floorClass, ceilingClass;
        lower.item.guid = APIGuidFromString ("00000011-0000-0000-0000-000000000001");
        upper.item.guid = APIGuidFromString ("00000012-0000-0000-0000-000000000002");
        generic.item.guid = APIGuidFromString ("00000013-0000-0000-0000-000000000003");
        floorClass.item.guid = APIGuidFromString ("00000014-0000-0000-0000-000000000004");
        ceilingClass.item.guid = APIGuidFromString ("00000015-0000-0000-0000-000000000005");
        classes.Add ("some_stuff_fin_skirting", lower);
        classes.Add ("some_stuff_fin_cornice", upper);
        classes.Add ("some_stuff_fin_class", generic);
        classes.Add ("some_stuff_fin_floors", floorClass);
        classes.Add ("some_stuff_fin_ceils", ceilingClass);
        check (OtdBeam ().draw.type == NoSet, "roles unassigned beam has no implicit finish role");
        check (Class_GetClassGuid (lowerRole, classes) == lower.item.guid,
               "roles lower class uses the configured lower skirting marker");
        check (Class_GetClassGuid (upperRole, classes) == upper.item.guid,
               "roles upper skirting selects its own class");
        check (Class_GetOtdTypeByClass (lower.item.guid, classes) == lowerRole &&
                   Class_GetOtdTypeByClass (upper.item.guid, classes) == upperRole,
               "roles reverse classification preserves both skirting roles");
        check (Class_GetClassGuid (Floor, classes) == floorClass.item.guid &&
                   Class_GetClassGuid (Ceil, classes) == ceilingClass.item.guid &&
                   Class_GetOtdTypeByClass (floorClass.item.guid, classes) == Floor &&
                   Class_GetOtdTypeByClass (ceilingClass.item.guid, classes) == Ceil,
               "roles adjacent slab classification stays unchanged");
        check (Otd_IsNativeTypeForFinish (lowerRole, API_BeamID) && Otd_IsNativeTypeForFinish (upperRole, API_BeamID),
               "roles both skirting types accept native beams");
        check (Otd_IsNativeTypeForFinish (lowerRole, API_WallID) &&
                   Otd_IsNativeTypeForFinish (lowerRole, API_ObjectID) &&
                   Otd_IsNativeTypeForFinish (upperRole, API_WallID) &&
                   Otd_IsNativeTypeForFinish (upperRole, API_ObjectID),
               "roles both skirting types accept wall and object writers");
        check (!Otd_IsNativeTypeForFinish (lowerRole, API_SlabID) &&
                   !Otd_IsNativeTypeForFinish (upperRole, API_SlabID) &&
                   !Otd_IsNativeTypeForFinish (lowerRole, API_ColumnID) &&
                   !Otd_IsNativeTypeForFinish (upperRole, API_RoofID),
               "roles skirting rejects unsupported native writers");
        check (!GS::UniString ("some_stuff_fin_cornice").Contains (GS::UniString ("some_stuff_fin_skirting")),
               "roles upper class marker does not match lower substring search");
        classes.Delete ("some_stuff_fin_cornice");
        check (Class_GetClassGuid (upperRole, classes) == generic.item.guid &&
                   Class_GetClassGuid (lowerRole, classes) == lower.item.guid,
               "roles missing upper class falls back to the generic class");
        check (Class_GetOtdTypeByClass (upper.item.guid, classes) == NoSet,
               "roles unknown upper class is not misclassified as lower");
        classes.Delete ("some_stuff_fin_skirting");
        classes.Add ("some_stuff_fin_cornice", upper);
        check (Class_GetClassGuid (lowerRole, classes) == generic.item.guid &&
                   Class_GetClassGuid (upperRole, classes) == upper.item.guid,
               "roles missing lower class falls back to generic and keeps configured upper");
    }
    // Preflight плинтуса/карниза един с общей конфигурацией отделки: та же функция и те же требования,
    // что у стен. base-свойство только предупреждает; zone-покрытие обязательно для всех ролей.
    {
        ClassificationFunc::ClassificationDict classes;
        UnicGuid recognized;
        ClassificationFunc::ClassificationValues generic, lower, upper;
        generic.item.guid = lower.item.guid = upper.item.guid = APINULLGuid;
        generic.item.guid.time_low = 203;
        lower.item.guid.time_low = 201;
        upper.item.guid.time_low = 202;
        check (!Roombook_CheckFinishConfiguration (classes, recognized),
               "preflight empty class dict rejected");

        auto &cache = PROPERTYCACHE ();
        const ParamDictValue original = cache.property;
        const bool originalFull = cache.isPropertyDefinitionRead_full;
        const bool originalOK = cache.isPropertyDefinition_OK;
        struct CacheRestore {
            PropertyCache &cache;
            ParamDictValue property;
            bool full;
            bool ok;

            explicit CacheRestore (PropertyCache &source)
                : cache (source), property (source.property), full (source.isPropertyDefinitionRead_full),
                  ok (source.isPropertyDefinition_OK) {}

            ~CacheRestore () {
                cache.property = property;
                cache.isPropertyDefinitionRead_full = full;
                cache.isPropertyDefinition_OK = ok;
            }
        } restore (cache);
        cache.isPropertyDefinitionRead_full = true;
        cache.isPropertyDefinition_OK = true;

        classes.Add (all_class, generic);
        recognized.Clear ();
        recognized.Add (generic.item.guid, false);
        cache.property.Clear ();
        check (!Roombook_CheckFinishConfiguration (classes, recognized),
               "preflight zone property missing rejected");

        ParamValue zone;
        zone.definition.description = SYNCGUID + " zone";
        zone.definition.availability.Push (generic.item.guid);
        cache.property.Add ("zone", zone);
        check (Roombook_CheckFinishConfiguration (classes, recognized) && recognized.Get (generic.item.guid),
               "preflight zone availability passes with base property absent");

        recognized.Set (generic.item.guid, false);
        zone.definition.description = "zone";
        cache.property.Set ("zone", zone);
        check (!Roombook_CheckFinishConfiguration (classes, recognized),
               "preflight missing Sync_GUID marker rejected");

        zone.definition.description = SYNCGUID + " zone";
        cache.property.Set ("zone", zone);
        check (Roombook_CheckFinishConfiguration (classes, recognized),
               "preflight recovers after invalid definition");

        // Роли плинтуса/карниза в recognized покрываются тем же механизмом, что и стены.
        recognized.Set (generic.item.guid, true);
        recognized.Add (lower.item.guid, false);
        recognized.Add (upper.item.guid, false);
        check (!Roombook_CheckFinishConfiguration (classes, recognized),
               "preflight uncovered skirting roles rejected like walls");
        ParamValue roleZone = zone;
        roleZone.definition.availability.Push (lower.item.guid);
        roleZone.definition.availability.Push (upper.item.guid);
        cache.property.Set ("zone", roleZone);
        check (Roombook_CheckFinishConfiguration (classes, recognized),
               "preflight skirting roles covered by zone property accepted");
    }
    #if defined(SKIRTING_PROCESS_REGRESSION)
    {
        for (const auto role : {Skirting_Down, Skirting_Up}) {
            const GS::UniString defaultName = role == Skirting_Down ? "smstf plinth down" : "smstf plinth up";
            auto favorite = SkirtingFavoriteForRegression (role, "", "", defaultName, API_BeamID);
            check (favorite.name == defaultName && favorite.type == API_BeamID,
                   "skirting selects dedicated Beam favorite for each role");
            check (SkirtingFavoriteCompositeReadsForRegression () == 0,
                   "skirting selection does not read finish composition");
            favorite = SkirtingFavoriteForRegression (role, "custom skirting", "", "custom skirting", API_BeamID);
            check (favorite.name == GS::UniString ("custom skirting") && favorite.type == API_BeamID,
                   "skirting preserves explicitly selected Beam favorite");
            for (const auto native : {API_BeamID, API_WallID, API_ObjectID, API_SlabID}) {
                favorite = SkirtingFavoriteForRegression (
                    role, "", "@material favorite@", defaultName, API_BeamID, "material favorite", native);
                check (favorite.name == defaultName && favorite.type == API_BeamID,
                       "zone material cannot override dedicated Beam favorite");
                favorite = SkirtingFavoriteForRegression (role,
                                                          "custom skirting",
                                                          "@material favorite@",
                                                          "custom skirting",
                                                          API_BeamID,
                                                          "material favorite_custom skirting",
                                                          native);
                check (favorite.name == GS::UniString ("custom skirting") && favorite.type == API_BeamID,
                       "zone material suffix cannot override explicit Beam favorite");
                favorite = SkirtingFavoriteForRegression (role, "", "@material favorite@", "material favorite", native);
                check (favorite.name.IsEmpty () && favorite.type == API_ZombieElemID,
                       "missing dedicated favorite never adopts a material favorite");
                favorite = SkirtingFavoriteForRegression (role, "", "", "smstf wall", native);
                check (favorite.name.IsEmpty () && favorite.type == API_ZombieElemID,
                       "missing dedicated favorite never falls back to smstf wall");
                if (native == API_BeamID)
                    continue;
                favorite = SkirtingFavoriteForRegression (role, "", "", defaultName, native);
                check (favorite.name.IsEmpty () && favorite.type == API_ZombieElemID,
                       "dedicated favorite of non-Beam type is rejected");
                favorite = SkirtingFavoriteForRegression (
                    role, "custom skirting", "", "custom skirting", native, defaultName, API_BeamID);
                check (favorite.name.IsEmpty () && favorite.type == API_ZombieElemID,
                       "invalid explicit favorite does not silently select another Beam");
            }
            favorite = SkirtingFavoriteForRegression (role, "", "@material favorite@", "", API_BeamID);
            check (favorite.name.IsEmpty () && favorite.type == API_ZombieElemID,
                   "empty favorite dictionary leaves skirting unresolved");
        }
        const auto slab = SkirtingFavoriteForRegression (Floor, "custom floor", "", "custom floor", API_WallID);
        check (slab.name == GS::UniString ("custom floor") && slab.type == API_ZombieElemID,
               "dispatch selector keeps unresolved floor name without adopting a wall type");
        const auto accessory = SkirtingFavoriteForRegression (Floor, "custom floor", "", "custom floor", API_ObjectID);
        check (accessory.name == GS::UniString ("custom floor") && accessory.type == API_ObjectID,
               "dispatch selector preserves adjacent floor accessory selection");
    }
    #endif
    #if defined(SKIRTING_PROCESS_REGRESSION)
    {
        const auto polygon = [] (double left, double right, bool hole) {
            GS::Array<Point2D> points = {{left, 19}, {right, 19}, {right, 21}, {left, 21}};
            GS::Array<UIndex> ends = {3};
            if (hole) {
                for (const auto &p : GS::Array<Point2D>{{14, 19.5}, {14, 20.5}, {16, 20.5}, {16, 19.5}})
                    points.Push (p);
                ends.Push (7);
            }
            GS::Array<PolyArcRec> arcs;
            return Geometry::Polygon2D::Create (
                points, arcs, ends, Geometry::PolyCreateFlags_IdxRefInContourEndFromZero)[0];
        };
        const auto totalLength = [] (const GS::Array<OtdBeam> &beams) {
            double total = 0;
            for (const auto &beam : beams)
                total += std::hypot (std::hypot (beam.geom.seg.endC.x - beam.geom.seg.begC.x,
                                                 beam.geom.seg.endC.y - beam.geom.seg.begC.y),
                                     beam.geom.seg.endC.z - OtdLinearSegment_GetZBottom (beam.geom));
            return total;
        };
        for (bool surface : {false, true}) {
            OtdRoom ready;
            ready.tip_floor_skirting = "floor";
            ready.tip_ceil_cornice = "ceiling";
            ready.otdwall.Push (Wall ());
            OtdSlab lower;
            lower.draw.type = Floor;
            lower.zBottom = 5.1;
            lower.poly = polygon (11, 19, true);
            lower.surface_geometry = surface;
            lower.draw.favorite.type = API_ObjectID;
            lower.draw.favorite.name = "regression accessory";
            OtdSlab upper = lower;
            upper.draw.type = Ceil;
            upper.zBottom = 7.8;
            upper.poly = polygon (12, 18, false);
            ready.otdslab = {lower, upper};
            const bool built = SkirtingProcessForRegression (ready);
            const bool complete = ready.otdbeam_floor.GetSize () == 2 && ready.otdbeam_ceil.GetSize () == 1;
            check (built && complete, "accessory orchestration accepts both roles with either surface flag");
            check (Equal (totalLength (ready.otdbeam_floor), 6) && Equal (totalLength (ready.otdbeam_ceil), 6),
                   "accessory shared polygon clips partial coverage and holes");
            check (complete && Equal (OtdLinearSegment_GetZBottom (ready.otdbeam_floor[0].geom), lower.zBottom) &&
                       Equal (ready.otdbeam_floor[0].geom.seg.endC.z, lower.zBottom) &&
                       Equal (OtdLinearSegment_GetZBottom (ready.otdbeam_ceil[0].geom), upper.zBottom) &&
                       Equal (ready.otdbeam_ceil[0].geom.seg.endC.z, upper.zBottom),
                   "accessory retains prepared elevations without Slab thickness");
            check (complete && ready.otdbeam_floor[0].draw.type == Skirting_Down &&
                       ready.otdbeam_ceil[0].draw.type == Skirting_Up && ready.otdbeam_floor[0].wallIndex == 0 &&
                       ready.otdbeam_floor[0].slabIndex == 0 && ready.otdbeam_ceil[0].slabIndex == 1 &&
                       ready.otdbeam_floor[0].draw.favorite.type == API_BeamID &&
                       ready.otdbeam_ceil[0].draw.favorite.type == API_BeamID,
                   "accessory reaches common beam favorite selection and preserves indices");
            check (SkirtingProcessFavoriteReadsForRegression () == 0 && SkirtingProcessReportsForRegression () == 0,
                   "accessory never reads Slab settings or reports unsupported writer");
            check (Equal (ready.otdslab[0].zBottom, lower.zBottom) && Equal (ready.otdslab[1].zBottom, upper.zBottom) &&
                       Equal (ready.otdslab[0].poly.CalcArea (), lower.poly.CalcArea ()) &&
                       Equal (ready.otdslab[1].poly.CalcArea (), upper.poly.CalcArea ()),
                   "accessory input elevations and polygon areas remain unchanged");
            check (SkirtingProcessForRegression (ready) && ready.otdbeam_floor.GetSize () == 2 &&
                       ready.otdbeam_ceil.GetSize () == 1,
                   "accessory repeat replaces both arrays");
            ready.otdslab[0].draw.favorite.name.Clear ();
            ready.otdslab[1].draw.favorite.name.Clear ();
            check (SkirtingProcessForRegression (ready) && ready.otdbeam_floor.GetSize () == 2 &&
                       ready.otdbeam_ceil.GetSize () == 1 && SkirtingProcessFavoriteReadsForRegression () == 0,
                   "accessory geometry does not require an Object favorite read");
            ready.create_floor_elements = false;
            check (SkirtingProcessForRegression (ready) && ready.otdbeam_floor.IsEmpty () &&
                       ready.otdbeam_ceil.GetSize () == 1,
                   "accessory floor writer flag leaves upper independent");
            ready.create_floor_elements = true;
            ready.has_ceil = false;
            check (SkirtingProcessForRegression (ready) && ready.otdbeam_floor.GetSize () == 2 &&
                       ready.otdbeam_ceil.IsEmpty (),
                   "accessory missing ceiling leaves lower independent");
            ready.otdslab[0].zBottom = 4.5;
            check (SkirtingProcessForRegression (ready) && ready.otdbeam_floor.IsEmpty () &&
                       ready.otdbeam_ceil.IsEmpty (),
                   "accessory common XY never substitutes a Z gap");
            ready.otdslab[0] = lower;
            ready.otdslab[0].slope_x = std::numeric_limits<double>::quiet_NaN ();
            check (!SkirtingProcessForRegression (ready) && ready.otdbeam_floor.IsEmpty () &&
                       ready.otdbeam_ceil.IsEmpty (),
                   "accessory malformed plane publishes no partial arrays");
        }
        OtdRoom native;
        native.tip_floor_skirting = "floor";
        native.tip_ceil_cornice = "ceiling";
        native.otdwall.Push (Wall ());
        OtdSlab lower;
        lower.draw.type = Floor;
        lower.zBottom = 5.1;
        lower.poly = polygon (11, 19, false);
        lower.surface_geometry = true;
        lower.draw.favorite.type = API_SlabID;
        lower.draw.favorite.name = "regression slab";
        OtdSlab upper = lower;
        upper.draw.type = Ceil;
        upper.zBottom = 7.8;
        native.otdslab = {lower, upper};
        const bool built = SkirtingProcessForRegression (native);
        check (built && native.otdbeam_floor.GetSize () == 1 && native.otdbeam_ceil.GetSize () == 1 &&
                   Equal (OtdLinearSegment_GetZBottom (native.otdbeam_floor[0].geom), lower.zBottom + 0.25) &&
                   Equal (OtdLinearSegment_GetZBottom (native.otdbeam_ceil[0].geom), upper.zBottom - 0.25) &&
                   SkirtingProcessFavoriteReadsForRegression () == 2,
               "accessory fix preserves native Slab settings thickness path");
    }
    #endif
    std::printf ("passed=%d failed=%d\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
#endif
#endif
