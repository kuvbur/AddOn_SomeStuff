#if defined(SKIRTING_REGRESSION)
    #include <cmath>
    #include <cstdio>
    #include <limits>

    #include "roombook/RoomSurfaceFinish.hpp"

namespace {
    bool Equal (double a, double b) { return std::fabs (a - b) < 1e-8; }

    Roombook::OtdWall Wall () {
        Roombook::OtdWall wall;
        wall.type = Roombook::Wall_Main;
        wall.begC = {10, 20};
        wall.endC = {20, 20};
        wall.zBottom = 5;
        wall.height = 3;
        wall.floorInd = 2;
        wall.base_type = API_WallID;
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
} // namespace

int main () {
    using namespace Roombook;
    int passed = 0, failed = 0;
    const auto check = [&] (bool ok, const char *name) {
        std::printf ("%s: %s\n", ok ? "PASS" : "FAIL", name);
        ok ? ++passed : ++failed;
    };
    OtdRoom room;
    room.zBottom = 5;
    check (room.otdbeam_floor.IsEmpty () && room.otdbeam_ceil.IsEmpty (), "both arrays start empty");
    check (room.length_beam_floor == 0 && room.length_beam_ceil == 0, "both skirting lengths start at zero");
    OtdBeam ceiling;
    ceiling.zBottom = 8;
    room.otdbeam_ceil.Push (ceiling);
    room.otdwall.Push (Wall ());
    check (BuildFloorSkirtingSegments (room) && room.otdbeam_floor.GetSize () == 1, "uncut prepared wall");
    check (Equal (room.otdbeam_floor[0].begC.x, 10) && Equal (room.otdbeam_floor[0].endC.x, 20) &&
               Equal (room.otdbeam_floor[0].begC.y, 20) && Equal (room.otdbeam_floor[0].zBottom, 5),
           "prepared coordinates and absolute bottom copied");
    check (room.otdbeam_floor[0].floorInd == 2 && room.otdbeam_floor[0].base_type == API_WallID &&
               room.otdbeam_floor[0].base_guid == room.otdwall[0].base_guid && room.otdbeam_floor[0].wallIndex == 0 &&
               room.otdbeam_floor[0].otd_guid == APINULLGuid,
           "source metadata retained without inventing a created guid");
    room.otdwall[0].openings.Push (Door ());
    check (BuildFloorSkirtingSegments (room) && room.otdbeam_floor.GetSize () == 2 &&
               Equal (room.otdbeam_floor[0].endC.x, 12) && Equal (room.otdbeam_floor[1].begC.x, 14),
           "null-owner geometric door cuts actual wall bottom");
    check (BuildFloorSkirtingSegments (room) && room.otdbeam_floor.GetSize () == 2,
           "repeat replaces rather than appends");
    room.otdwall[0].openings[0].lower = 100;
    room.otdwall[0].rise_x = 2;
    room.otdwall[0].surface_geometry = true;
    check (BuildFloorSkirtingSegments (room) && room.otdbeam_floor.GetSize () == 2 &&
               Equal (room.otdbeam_floor[0].begC.x, 10) && Equal (room.otdbeam_floor[1].begC.x, 14),
           "no second rise shift and absolute opening bottom used");
    check (room.otdwall.GetSize () == 1 && Equal (room.otdwall[0].begC.x, 10) && Equal (room.otdwall[0].endC.x, 20) &&
               Equal (room.otdwall[0].openings[0].lower, 100) && room.otdbeam_ceil.GetSize () == 1 &&
               Equal (room.otdbeam_ceil[0].zBottom, 8),
           "source wall openings and ceiling array untouched");
    room.otdwall[0].begC = {20, 20};
    room.otdwall[0].endC = {10, 20};
    check (BuildFloorSkirtingSegments (room) && room.otdbeam_floor.GetSize () == 2 &&
               Equal (room.otdbeam_floor[0].begC.x, 20) && Equal (room.otdbeam_floor[0].endC.x, 18) &&
               Equal (room.otdbeam_floor[1].begC.x, 16) && Equal (room.otdbeam_floor[1].endC.x, 10),
           "reversed wall preserves direction and opening location");
    room.otdwall[0].begC = {10, 20};
    room.otdwall[0].endC = {16, 28};
    check (BuildFloorSkirtingSegments (room) && room.otdbeam_floor.GetSize () == 2 &&
               Equal (room.otdbeam_floor[0].endC.x, 11.2) && Equal (room.otdbeam_floor[0].endC.y, 21.6) &&
               Equal (room.otdbeam_floor[1].begC.x, 12.4) && Equal (room.otdbeam_floor[1].begC.y, 23.2),
           "diagonal wall intervals map to world coordinates");
    room.otdwall[0].zBottom = 4;
    check (BuildFloorSkirtingSegments (room) && room.otdbeam_floor.IsEmpty (),
           "spanning wall below zone bottom is not selected");
    room.otdwall[0] = Wall ();
    room.otdwall[0].zBottom += 5e-7;
    check (BuildFloorSkirtingSegments (room) && room.otdbeam_floor.GetSize () == 1,
           "zone bottom comparison uses distance tolerance");
    room.otdwall[0].zBottom = 5 + 2e-6;
    check (BuildFloorSkirtingSegments (room) && room.otdbeam_floor.IsEmpty (),
           "wall above bottom outside tolerance is excluded");
    room.otdwall[0] = Wall ();
    room.create_wall_elements = false;
    check (BuildFloorSkirtingSegments (room) && room.otdbeam_floor.IsEmpty (), "disabled wall writer excluded");
    room.otdwall[0].type = Column;
    check (BuildFloorSkirtingSegments (room) && room.otdbeam_floor.GetSize () == 1,
           "enabled column retained independently of wall flag");
    room.create_column_elements = false;
    check (BuildFloorSkirtingSegments (room) && room.otdbeam_floor.IsEmpty (), "disabled column excluded");
    room.create_wall_elements = true;
    room.otdwall.Clear ();
    for (TypeOtd type : {Reveal_Main, Reveal_Up, Reveal_Down, Floor, Ceil}) {
        OtdWall wall = Wall ();
        wall.type = type;
        room.otdwall.Push (wall);
    }
    check (BuildFloorSkirtingSegments (room) && room.otdbeam_floor.IsEmpty (), "reveals and slab sides excluded");
    room.otdwall.Push (Wall ());
    room.otdwall.Push (Wall ());
    room.otdwall[6].isValid = false;
    check (BuildFloorSkirtingSegments (room) && room.otdbeam_floor.GetSize () == 1 &&
               room.otdbeam_floor[0].wallIndex == 5,
           "invalid wall excluded and source index is not output index");
    room.isValid = false;
    check (BuildFloorSkirtingSegments (room) && room.otdbeam_floor.IsEmpty (), "invalid room clears stale result");
    room.isValid = true;
    room.otdwall.Clear ();
    room.otdwall.Push (Wall ());
    OtdWall broken = Wall ();
    broken.openings.Push (Door ());
    broken.openings[0].width = 0;
    room.otdwall.Push (broken);
    check (!BuildFloorSkirtingSegments (room) && room.otdbeam_floor.IsEmpty (),
           "invalid second wall opening publishes no partial room");
    room.otdwall.Clear ();
    room.otdwall.Push (Wall ());
    room.otdwall[0].endC = room.otdwall[0].begC;
    check (!BuildFloorSkirtingSegments (room) && room.otdbeam_floor.IsEmpty (), "zero length fails safely");
    room.otdwall[0] = Wall ();
    room.otdwall[0].zBottom = std::numeric_limits<double>::quiet_NaN ();
    check (!BuildFloorSkirtingSegments (room) && room.otdbeam_floor.IsEmpty (), "nonfinite wall fails safely");
    room.otdwall[0] = Wall ();
    room.zBottom = std::numeric_limits<double>::infinity ();
    check (!BuildFloorSkirtingSegments (room) && room.otdbeam_floor.IsEmpty (), "nonfinite room fails safely");
    std::printf ("passed=%d failed=%d\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
#endif
