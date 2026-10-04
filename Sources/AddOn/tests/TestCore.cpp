//------------ kuvbur 2022 ------------
#ifdef TESTING
    #include "ACAPinc.h"

    #include "api_headers/APIEnvir.h"

    #include "Helpers.hpp"
    #include "Propertycache.hpp"
    #include "Roombook.hpp"
    #include "tests/TestFunc.hpp"
    #include "tests/TestKit.hpp"

namespace TestFunc {

    void TestStringSplt () {
        GS::Array<GS::UniString> parts;
        UInt32 n;

        // Simple delimiter: semicolon
        parts.Clear ();
        n = StringSplt ("a;b;c", ";", parts, true);
        DBtest (n, (UInt32)3, "StringSplt semicolon 3 parts");
        DBtest (parts.GetSize (), (UInt32)3, "StringSplt semicolon GetSize");
        DBtest (parts.Get (0), GS::UniString ("a"), "parts[0]");
        DBtest (parts.Get (1), GS::UniString ("b"), "parts[1]");
        DBtest (parts.Get (2), GS::UniString ("c"), "parts[2]");

        // filter_empty = false — empty tokens preserved
        parts.Clear ();
        n = StringSplt ("a;;c", ";", parts, false);
        DBtest (n, (UInt32)3, "StringSplt empty kept");
        DBtest (parts.GetSize (), (UInt32)3, "StringSplt empty kept GetSize");
        DBtest (parts.Get (1).IsEmpty (), true, "parts[1] is empty");

        // filter_empty = true — empty tokens removed
        parts.Clear ();
        n = StringSplt ("a;;c", ";", parts, true);
        DBtest (n, (UInt32)2, "StringSplt empty filtered");
        DBtest (parts.GetSize (), (UInt32)2, "StringSplt empty filtered GetSize");
        DBtest (parts.Get (0), GS::UniString ("a"), "parts[0] after filter");
        DBtest (parts.Get (1), GS::UniString ("c"), "parts[1] after filter");

        // No delimiter found — whole string is a single element
        parts.Clear ();
        n = StringSplt ("hello", ";", parts, true);
        DBtest (n, (UInt32)1, "StringSplt no delimiter returns 1");
        DBtest (parts.GetSize (), (UInt32)1, "StringSplt no delimiter GetSize");
        DBtest (parts.Get (0), GS::UniString ("hello"), "parts[0] no delimiter");

        // Unicode delimiter (Cyrillic semicolon)
        parts.Clear ();
        n = StringSplt ("один;два;три", ";", parts, true);
        DBtest (n, (UInt32)3, "StringSplt unicode 3 parts");
        DBtest (parts.GetSize (), (UInt32)3, "StringSplt unicode GetSize");
        DBtest (parts.Get (1), GS::UniString ("два"), "parts[1] unicode");

        // Using scratch buffer (nullptr vs external)
        parts.Clear ();
        n = StringSplt ("x@y@z", "@", parts, true, nullptr);
        DBtest (n, (UInt32)3, "StringSplt with nullptr scratch");
        DBtest (parts.Get (1), GS::UniString ("y"), "parts[1] scratch nullptr");

        // Scratch buffer passed externally
        GS::Array<GS::UniString> scratch = {};
        parts.Clear ();
        n = StringSplt ("p;q;r", ";", parts, true, &scratch);
        DBtest (n, (UInt32)3, "StringSplt with external scratch");
        DBtest (parts.Get (2), GS::UniString ("r"), "parts[2] external scratch");

        // Leading/trailing whitespace is trimmed
        parts.Clear ();
        n = StringSplt ("  a  ;  b  ", ";", parts, true);
        DBtest (n, (UInt32)2, "StringSplt trim");
        DBtest (parts.Get (0), GS::UniString ("a"), "parts[0] trimmed");
        DBtest (parts.Get (1), GS::UniString ("b"), "parts[1] trimmed");

        return;
    }

    void TestBuildOtdByParent () {
        const API_Guid baseGuid = APIGuidFromString ("{11111111-1111-1111-1111-111111111111}");
        const API_Guid floorGuid = APIGuidFromString ("{22222222-2222-2222-2222-222222222222}");
        const API_Guid wallGuid = APIGuidFromString ("{33333333-3333-3333-3333-333333333333}");
        const API_Guid unknownGuid = APIGuidFromString ("{44444444-4444-4444-4444-444444444444}");

        GS::HashTable<API_Guid, Roombook::TypeOtd> otdElements;
        otdElements.Add (floorGuid, Roombook::Floor);
        otdElements.Add (wallGuid, Roombook::Wall_Main);

        UnicGuid childElements;
        childElements.Add (floorGuid, true);
        childElements.Add (wallGuid, true);
        UnicGuidByGuid parentDict;
        parentDict.Add (baseGuid, childElements);

        bool hasBaseElement = false;
        Roombook::UnicGuidByBase result = Roombook::BuildOtdByParent (otdElements, parentDict, hasBaseElement);
        const Roombook::UnicGuidByTypeOtd *types = result.GetPtr (baseGuid);
        const UnicGuid *floorElements = types != nullptr ? types->GetPtr (Roombook::Floor) : nullptr;
        const UnicGuid *wallElements = types != nullptr ? types->GetPtr (Roombook::Wall_Main) : nullptr;

        DBtest (hasBaseElement, "BuildOtdByParent: known children set has_base_element");
        DBtest (floorElements != nullptr && floorElements->ContainsKey (floorGuid),
                "BuildOtdByParent: floor child indexed by base GUID");
        DBtest (wallElements != nullptr && wallElements->ContainsKey (wallGuid),
                "BuildOtdByParent: wall child indexed by base GUID");

        UnicGuid unknownChildren;
        unknownChildren.Add (unknownGuid, true);
        UnicGuidByGuid unknownParentDict;
        unknownParentDict.Add (baseGuid, unknownChildren);
        hasBaseElement = false;
        result = Roombook::BuildOtdByParent (otdElements, unknownParentDict, hasBaseElement);
        DBtest (!hasBaseElement, "BuildOtdByParent: unknown child keeps has_base_element false");
        DBtest (result.IsEmpty (), "BuildOtdByParent: unknown child is ignored");

        const API_Guid secondBaseGuid = APIGuidFromString ("{55555555-5555-5555-5555-555555555555}");
        const API_Guid secondWallGuid = APIGuidFromString ("{66666666-6666-6666-6666-666666666666}");
        const API_Guid thirdWallGuid = APIGuidFromString ("{77777777-7777-7777-7777-777777777777}");
        otdElements.Add (secondWallGuid, Roombook::Wall_Main);
        otdElements.Add (thirdWallGuid, Roombook::Wall_Main);

        childElements.Add (secondWallGuid, true);
        childElements.Add (unknownGuid, true);
        UnicGuid otherChildren;
        otherChildren.Add (thirdWallGuid, true);
        UnicGuidByGuid multipleParents;
        multipleParents.Add (baseGuid, childElements);
        multipleParents.Add (secondBaseGuid, otherChildren);
        hasBaseElement = false;
        result = Roombook::BuildOtdByParent (otdElements, multipleParents, hasBaseElement);
        const Roombook::UnicGuidByTypeOtd *firstParent = result.GetPtr (baseGuid);
        const Roombook::UnicGuidByTypeOtd *secondParent = result.GetPtr (secondBaseGuid);
        const UnicGuid *firstWalls = firstParent != nullptr ? firstParent->GetPtr (Roombook::Wall_Main) : nullptr;
        const UnicGuid *secondWalls = secondParent != nullptr ? secondParent->GetPtr (Roombook::Wall_Main) : nullptr;
        DBtest (hasBaseElement, "BuildOtdByParent: known children among unknown set flag");
        DBtest (result.GetSize (), (USize)2, "BuildOtdByParent: two parent entries");
        DBtest (firstWalls != nullptr && firstWalls->GetSize () == 2 && firstWalls->ContainsKey (wallGuid) &&
                    firstWalls->ContainsKey (secondWallGuid) && !firstWalls->ContainsKey (unknownGuid),
                "BuildOtdByParent: two walls of one type, unknown excluded");
        DBtest (secondWalls != nullptr && secondWalls->GetSize () == 1 && secondWalls->ContainsKey (thirdWallGuid) &&
                    !secondWalls->ContainsKey (wallGuid),
                "BuildOtdByParent: second parent stays separate");

        UnicGuidByGuid emptyParents;
        hasBaseElement = false;
        result = Roombook::BuildOtdByParent (otdElements, emptyParents, hasBaseElement);
        DBtest (result.IsEmpty () && !hasBaseElement, "BuildOtdByParent: empty parent input");

        GS::HashTable<API_Guid, Roombook::TypeOtd> emptyTypes;
        result = Roombook::BuildOtdByParent (emptyTypes, multipleParents, hasBaseElement);
        DBtest (result.IsEmpty () && !hasBaseElement, "BuildOtdByParent: empty type input");
        hasBaseElement = true;
        result = Roombook::BuildOtdByParent (emptyTypes, multipleParents, hasBaseElement);
        DBtest (result.IsEmpty () && hasBaseElement, "BuildOtdByParent: input flag is not reset");
    }

    void TestOpeningAddOne () {
        struct Case {
            const char *name;
            double center;
            double width;
            double begin;
            double end;
            double wallLength;
            bool flipped;
            bool added;
            double expectedWidth;
            double expectedCenter;
        };

        const Case cases[] = {{"inside", 5, 2, 0, 10, 10, false, true, 2, 5},
                              {"clip left", 0, 4, 0, 10, 10, false, true, 2, 1},
                              {"clip right", 10, 4, 0, 10, 10, false, true, 2, 9},
                              {"covers segment", 5, 20, 0, 10, 10, false, true, 10, 5},
                              {"outside left", -5, 2, 0, 10, 10, false, false, 0, 0},
                              {"outside right", 15, 2, 0, 10, 10, false, false, 0, 0},
                              {"touch left", -1, 2, 0, 10, 10, false, false, 0, 0},
                              {"touch right", 11, 2, 0, 10, 10, false, false, 0, 0},
                              {"zero opening", 5, 0, 0, 10, 10, false, false, 0, 0},
                              {"zero segment", 5, 2, 5, 5, 10, false, false, 0, 0},
                              {"reversed segment", 5, 2, 8, 2, 10, false, false, 0, 0},
                              {"negative width", 5, -2, 0, 10, 10, false, false, 0, 0},
                              {"negative origin", -2, 2, -4, 2, 10, false, true, 2, 2},
                              {"subsegment", 5, 2, 3, 8, 10, false, true, 2, 2},
                              {"flipped subsegment", 5, 2, 3, 8, 10, true, true, 2, 3},
                              {"flipped left clip", 0, 4, 0, 10, 10, true, true, 2, 9},
                              {"flipped right clip", 10, 4, 0, 10, 10, true, true, 2, 1},
                              {"flipped longer wall", 5, 2, 3, 8, 100, true, true, 2, 3}};
        const API_Guid guid = APIGuidFromString ("{88888888-8888-8888-8888-888888888888}");
        for (const Case &c : cases) {
            GS::UniString label (c.name);
            Roombook::OtdOpening input;
            input.base_guid = guid;
            input.objLoc = c.center;
            input.width = c.width;
            input.height = 2.5;
            input.lower = 0.5;
            input.reflected = true;
            input.has_reveal = true;
            input.base_reveal_width = 0.25;
            Roombook::OtdWall wall;
            Roombook::OtdOpening sentinel;
            sentinel.objLoc = -99;
            wall.openings.Push (sentinel);
            Roombook::Opening_Add_One (input, c.flipped, 100, c.begin, c.end, c.wallLength, wall);
            DBtest (wall.openings.GetSize () == (c.added ? 2 : 1), label + " count");
            DBtest (wall.openings.Get (0).objLoc, -99.0, label + " existing opening preserved");
            DBtest (input.objLoc == c.center && input.width == c.width && input.height == 2.5 && input.lower == 0.5 &&
                        input.base_guid == guid && input.reflected && input.has_reveal &&
                        input.base_reveal_width == 0.25,
                    label + " input unchanged");
            if (!c.added || wall.openings.GetSize () != 2)
                continue;
            const Roombook::OtdOpening &result = wall.openings.Get (1);
            DBtest (result.width, c.expectedWidth, label + " clipped width");
            DBtest (result.objLoc, c.expectedCenter, label + " local center");
            DBtest (result.height, 2.5, label + " height");
            DBtest (result.zBottom, 100.5, label + " absolute bottom");
            DBtest (result.base_guid == guid, label + " source GUID");
            // Характеризация текущего переноса: флаги откосов не копируются в новый проём.
            DBtest (!result.reflected && !result.has_reveal && result.base_reveal_width == 0 &&
                        result.otd_guid == APINULLGuid,
                    label + " output defaults");
        }
    }

    void TestOtdWallDelimOne () {
        struct Case {
            const char *name;
            double bottom;
            double height;
            double openingBottom;
            double openingHeight;
            double openingWidth;
            bool wallAdded;
            bool openingAdded;
            double expectedBottom;
            double expectedHeight;
            double expectedLower;
        };

        const Case cases[] = {{"inside", 0, 10, 1, 2, 1, true, true, 1, 2, 1},
                              {"clip bottom", 2, 4, 1, 2, 1, true, true, 2, 1, 0},
                              {"clip top", 2, 4, 4, 4, 1, true, true, 4, 2, 2},
                              {"covers wall", 2, 4, 0, 10, 1, true, true, 2, 4, 0},
                              {"touch bottom", 2, 4, 0, 2, 1, true, false, 0, 0, 0},
                              {"touch top", 2, 4, 6, 2, 1, true, false, 0, 0, 0},
                              {"outside below", 2, 4, -5, 2, 1, true, false, 0, 0, 0},
                              {"outside above", 2, 4, 8, 2, 1, true, false, 0, 0, 0},
                              {"zero width", 0, 10, 1, 2, 0, true, false, 0, 0, 0},
                              {"zero height", 0, 10, 1, 0, 1, true, false, 0, 0, 0},
                              {"minimum width", 0, 10, 1, 2, Roombook::min_dim, true, false, 0, 0, 0},
                              {"minimum height", 0, 10, 0, Roombook::min_dim, 1, true, false, 0, 0, 0},
                              {"zero requested wall", 0, 0, 1, 2, 1, false, false, 0, 0, 0},
                              {"wall above source", 10, 2, 1, 2, 1, false, false, 0, 0, 0},
                              {"wall below source", -2, 2, 1, 2, 1, false, false, 0, 0, 0}};
        for (const Case &c : cases) {
            GS::UniString label (c.name);
            Roombook::OtdWall input;
            input.zBottom = 0;
            input.height = 10;
            input.length = 77;
            Roombook::OtdOpening opening;
            opening.zBottom = c.openingBottom;
            opening.height = c.openingHeight;
            opening.width = c.openingWidth;
            opening.lower = 99;
            input.openings.Push (opening);
            GS::Array<Roombook::OtdWall> walls;
            Roombook::OtdWall sentinel;
            sentinel.height = -99;
            walls.Push (sentinel);
            Roombook::OtdMaterial material;
            material.smaterial = "test material";
            Roombook::TypeOtd type = Roombook::Wall_Main;
            bool added = Roombook::OtdWall_Delim_One (input,
                                                      walls,
                                                      c.height,
                                                      c.bottom,
                                                      type,
                                                      material,
                                                      material,
                                                      material,
                                                      material,
                                                      material,
                                                      material,
                                                      material,
                                                      material);
            DBtest (added == c.wallAdded, label + " wall return");
            DBtest (walls.GetSize () == (c.wallAdded ? 2 : 1), label + " wall count");
            DBtest (walls.Get (0).height, -99.0, label + " previous wall preserved");
            DBtest (input.height == 10 && input.zBottom == 0 && input.length == 77 && input.openings.GetSize () == 1 &&
                        input.openings.Get (0).lower == 99 && input.openings.Get (0).zBottom == c.openingBottom &&
                        input.openings.Get (0).height == c.openingHeight,
                    label + " source unchanged");
            if (!added || walls.GetSize () != 2)
                continue;
            const Roombook::OtdWall &result = walls.Get (1);
            DBtest (result.zBottom, c.bottom, label + " wall bottom");
            DBtest (result.height, c.height, label + " wall height");
            DBtest (result.length, 77.0, label + " wall length unchanged");
            DBtest (result.type == Roombook::Wall_Main && result.material.smaterial == material.smaterial,
                    label + " type and material");
            DBtest (result.openings.GetSize () == (c.openingAdded ? 1 : 0), label + " opening count");
            if (!c.openingAdded || result.openings.GetSize () != 1)
                continue;
            const Roombook::OtdOpening &op = result.openings.Get (0);
            DBtest (op.zBottom, c.expectedBottom, label + " opening bottom");
            DBtest (op.height, c.expectedHeight, label + " opening height");
            DBtest (op.lower, c.expectedLower, label + " opening lower");
            DBtest (op.width, c.openingWidth, label + " opening width");
        }

        struct WallCase {
            const char *name;
            double bottom;
            double height;
            bool accepted;
            double expectedBottom;
            double expectedHeight;
        };

        const WallCase wallCases[] = {{"source inside request", 0, 10, true, 2, 6},
                                      {"clip source bottom", 4, 8, true, 4, 4},
                                      {"clip source top", 0, 4, true, 2, 2},
                                      {"request inside source", 3, 2, true, 3, 2},
                                      {"touch source top", 8, 2, false, 0, 0},
                                      {"touch source bottom", 0, 2, false, 0, 0},
                                      {"below minimum request", 2, Roombook::min_dim / 2, false, 0, 0},
                                      {"below minimum intersection", 8 - Roombook::min_dim / 2, 1, false, 0, 0}};
        for (const WallCase &c : wallCases) {
            for (double width : {0.0, 1.0}) {
                GS::UniString label (c.name);
                Roombook::OtdWall input;
                input.zBottom = 2;
                input.height = 6;
                input.width = width;
                input.length = 77;
                GS::Array<Roombook::OtdWall> walls;
                Roombook::OtdMaterial material;
                material.smaterial = "wall clipping material";
                Roombook::TypeOtd type = Roombook::Wall_Main;
                bool added = Roombook::OtdWall_Delim_One (input,
                                                          walls,
                                                          c.height,
                                                          c.bottom,
                                                          type,
                                                          material,
                                                          material,
                                                          material,
                                                          material,
                                                          material,
                                                          material,
                                                          material,
                                                          material);
                DBtest (added == c.accepted, label + " intersection return");
                DBtest (walls.GetSize () == (c.accepted ? 1 : 0), label + " intersection count");
                DBtest (input.zBottom == 2 && input.height == 6 && input.width == width && input.length == 77,
                        label + " intersection source unchanged");
                DBtest (type == Roombook::Wall_Main, label + " input type unchanged");
                if (!added || walls.GetSize () != 1)
                    continue;
                const Roombook::OtdWall &result = walls.Get (0);
                DBtest (result.zBottom, c.expectedBottom, label + " intersection bottom");
                DBtest (result.height, c.expectedHeight, label + " intersection height");
                DBtest (result.length, width == 0 ? 77.0 : c.expectedHeight, label + " reveal length");
            }
        }

        for (bool reversed : {false, true}) {
            GS::UniString label = reversed ? "two openings reversed" : "two openings ascending";
            Roombook::OtdWall input;
            input.zBottom = 0;
            input.height = 10;
            Roombook::OtdOpening low;
            low.zBottom = 1;
            low.height = 2;
            low.width = 1;
            low.objLoc = 11;
            low.lower = 99;
            Roombook::OtdOpening high = low;
            high.zBottom = 5;
            high.objLoc = 22;
            input.openings.Push (reversed ? high : low);
            input.openings.Push (reversed ? low : high);
            GS::Array<Roombook::OtdWall> walls;
            Roombook::OtdMaterial material;
            material.smaterial = "test material";
            Roombook::TypeOtd type = Roombook::Wall_Main;
            bool added = Roombook::OtdWall_Delim_One (input,
                                                      walls,
                                                      10,
                                                      0,
                                                      type,
                                                      material,
                                                      material,
                                                      material,
                                                      material,
                                                      material,
                                                      material,
                                                      material,
                                                      material);
            DBtest (added && walls.GetSize () == 1, label + " wall retained");
            DBtest (input.openings.GetSize () == 2 && input.openings.Get (0).lower == 99 &&
                        input.openings.Get (1).lower == 99 && input.height == 10,
                    label + " source unchanged");
            if (!added || walls.GetSize () != 1)
                continue;
            const GS::Array<Roombook::OtdOpening> &openings = walls.Get (0).openings;
            DBtest (openings.GetSize () == 2, label + " both inside openings retained");
            if (openings.GetSize () != 2)
                continue;
            for (UIndex i = 0; i < 2; ++i) {
                const Roombook::OtdOpening &source = input.openings.Get (i);
                const Roombook::OtdOpening &result = openings.Get (i);
                DBtest (result.objLoc, source.objLoc, label + " order preserved");
                DBtest (result.zBottom, source.zBottom, label + " bottom unchanged");
                DBtest (result.height, source.height, label + " height unchanged");
                DBtest (result.lower, source.zBottom, label + " lower recalculated");
            }
        }
    }

    void TestOtdWallDelimAll () {
        struct Case {
            const char *name;
            API_ElemTypeID baseType;
            Roombook::TypeOtd sourceType;
            Roombook::TypeOtd types[3];
            const char *materials[3];
        };

        const Case cases[] = {{"wall",
                               API_WallID,
                               Roombook::Wall_Main,
                               {Roombook::Wall_Down, Roombook::Wall_Main, Roombook::Wall_Up},
                               {"down", "main", "up"}},
                              {"column",
                               API_ColumnID,
                               Roombook::Wall_Main,
                               {Roombook::Wall_Down, Roombook::Column, Roombook::Wall_Up},
                               {"down", "column", "up"}},
                              {"slab",
                               API_SlabID,
                               Roombook::Wall_Main,
                               {Roombook::Floor, Roombook::Wall_Main, Roombook::Wall_Up},
                               {"floor", "main", "up"}},
                              {"window",
                               API_WindowID,
                               Roombook::Wall_Main,
                               {Roombook::Reveal_Main, Roombook::Reveal_Main, Roombook::Reveal_Main},
                               {"reveal", "reveal", "reveal"}},
                              {"ceiling",
                               API_WallID,
                               Roombook::Ceil,
                               {Roombook::Ceil, Roombook::Ceil, Roombook::Ceil},
                               {"ceil", "ceil", "ceil"}},
                              {"floor",
                               API_WallID,
                               Roombook::Floor,
                               {Roombook::Floor, Roombook::Floor, Roombook::Floor},
                               {"floor", "floor", "floor"}},
                              {"window ceiling priority",
                               API_WindowID,
                               Roombook::Ceil,
                               {Roombook::Ceil, Roombook::Ceil, Roombook::Reveal_Main},
                               {"ceil", "ceil", "reveal"}},
                              {"window floor priority",
                               API_WindowID,
                               Roombook::Floor,
                               {Roombook::Floor, Roombook::Floor, Roombook::Reveal_Main},
                               {"floor", "floor", "reveal"}}};
        const auto makeMaterial = [] (const char *name) -> Roombook::OtdMaterial {
            Roombook::OtdMaterial material;
            material.rawname = name;
            material.smaterial = name;
            return material;
        };
        Roombook::OtdMaterial main = makeMaterial ("main");
        Roombook::OtdMaterial up = makeMaterial ("up");
        Roombook::OtdMaterial down = makeMaterial ("down");
        Roombook::OtdMaterial reveal = makeMaterial ("reveal");
        Roombook::OtdMaterial column = makeMaterial ("column");
        Roombook::OtdMaterial floor = makeMaterial ("floor");
        Roombook::OtdMaterial ceil = makeMaterial ("ceil");
        Roombook::OtdMaterial zone = makeMaterial ("zone");
        const double bottoms[] = {0, 2, 7};
        const double heights[] = {2, 5, 3};
        for (const Case &c : cases) {
            GS::UniString label (c.name);
            Roombook::OtdWall input;
            input.zBottom = 0;
            input.height = 10;
            input.length = 77;
            input.base_type = c.baseType;
            input.type = c.sourceType;
            GS::Array<Roombook::OtdWall> walls;
            Roombook::OtdWall sentinel;
            sentinel.height = -99;
            walls.Push (sentinel);
            Roombook::OtdWall_Delim_All (
                walls, input, 0, 2, 5, 3, 10, main, up, down, reveal, column, floor, ceil, zone);
            DBtest (walls.GetSize () == 4, label + " three bands appended");
            DBtest (walls.Get (0).height, -99.0, label + " sentinel unchanged");
            DBtest (input.height == 10 && input.zBottom == 0 && input.length == 77 && input.type == c.sourceType &&
                        input.base_composite.IsEmpty (),
                    label + " source unchanged");
            if (walls.GetSize () != 4)
                continue;
            for (UIndex i = 0; i < 3; ++i) {
                const Roombook::OtdWall &band = walls.Get (i + 1);
                DBtest (band.zBottom, bottoms[i], label + " band bottom and order");
                DBtest (band.height, heights[i], label + " band height");
                DBtest (band.type == c.types[i], label + " band type");
                DBtest (band.material.smaterial, GS::UniString (c.materials[i]), label + " band material");
                DBtest (band.length, 77.0, label + " width-zero length preserved");
            }
        }
        Roombook::OtdWall input;
        input.zBottom = 0;
        input.height = 10;
        input.type = Roombook::Wall_Main;
        input.base_type = API_WallID;
        GS::Array<Roombook::OtdWall> walls;
        Roombook::OtdWall_Delim_All (walls, input, 0, 0, 0, 0, 10, main, up, down, reveal, column, floor, ceil, zone);
        DBtest (walls.GetSize () == 1, "zero bands fallback count");
        if (walls.GetSize () == 1) {
            DBtest (walls.Get (0).height, 10.0, "zero bands fallback height");
            DBtest (walls.Get (0).zBottom, 0.0, "zero bands fallback bottom");
            DBtest (walls.Get (0).type == Roombook::Wall_Main, "zero bands fallback type");
            DBtest (walls.Get (0).material.smaterial, main.smaterial, "zero bands fallback material");
        }

        input.zBottom = 5;
        input.height = 2;
        walls.Clear ();
        Roombook::OtdWall_Delim_All (walls, input, 0, 2, 0, 0, 10, main, up, down, reveal, column, floor, ceil, zone);
        DBtest (walls.GetSize () == 1, "rejected band fallback count");
        if (walls.GetSize () == 1) {
            DBtest (walls.Get (0).zBottom, 5.0, "rejected band fallback source bottom");
            DBtest (walls.Get (0).height, 2.0, "rejected band fallback source height");
            // Текущий fallback сохраняет тип последней попытки, а не назначает Wall_Main.
            DBtest (walls.Get (0).type == Roombook::Wall_Down, "rejected band fallback keeps attempted type");
            DBtest (walls.Get (0).material.smaterial, down.smaterial, "rejected band fallback material");
        }
        walls.Clear ();
        Roombook::OtdWall_Delim_All (walls, input, 0, 2, 5, 3, 10, main, up, down, reveal, column, floor, ceil, zone);
        DBtest (walls.GetSize () == 1, "one intersecting band no fallback duplicate");
        if (walls.GetSize () == 1) {
            DBtest (walls.Get (0).type == Roombook::Wall_Main, "one intersecting band type");
            DBtest (walls.Get (0).height, 2.0, "one intersecting band clipped height");
        }
        walls.Clear ();
        Roombook::OtdWall_Delim_All (walls, input, 20, 1, 1, 1, 10, main, up, down, reveal, column, floor, ceil, zone);
        DBtest (walls.IsEmpty (), "bands and fallback outside source return nothing");
        DBtest (input.zBottom == 5 && input.height == 2 && input.type == Roombook::Wall_Main,
                "fallback source unchanged");

        for (bool reversed : {false, true}) {
            GS::UniString label = reversed ? "cross-band reversed" : "cross-band ascending";
            input.zBottom = 0;
            input.height = 10;
            input.openings.Clear ();
            Roombook::OtdOpening low;
            low.zBottom = 1;
            low.height = 2;
            low.width = 1;
            low.objLoc = 11;
            low.lower = 99;
            Roombook::OtdOpening high = low;
            high.zBottom = 5;
            high.height = 3;
            high.objLoc = 22;
            input.openings.Push (reversed ? high : low);
            input.openings.Push (reversed ? low : high);
            walls.Clear ();
            Roombook::OtdWall_Delim_All (
                walls, input, 0, 2, 5, 3, 10, main, up, down, reveal, column, floor, ceil, zone);
            DBtest (walls.GetSize () == 3, label + " three bands retained");
            DBtest (input.openings.GetSize () == 2 && input.openings.Get (0).lower == 99 &&
                        input.openings.Get (1).lower == 99,
                    label + " source openings unchanged");
            if (walls.GetSize () != 3)
                continue;
            DBtest (walls.Get (0).openings.GetSize () == 1 && walls.Get (1).openings.GetSize () == 2 &&
                        walls.Get (2).openings.GetSize () == 1,
                    label + " clipped opening counts");
            for (UIndex bandIndex = 0; bandIndex < 3; ++bandIndex) {
                const Roombook::OtdWall &band = walls.Get (bandIndex);
                DBtest (band.zBottom, bottoms[bandIndex], label + " band bottom");
                for (const Roombook::OtdOpening &opening : band.openings) {
                    bool isLow = opening.objLoc == 11;
                    double expectedBottom = isLow ? (bandIndex == 0 ? 1 : 2) : (bandIndex == 1 ? 5 : 7);
                    double expectedHeight = !isLow && bandIndex == 1 ? 2 : 1;
                    DBtest (opening.zBottom, expectedBottom, label + " opening bottom");
                    DBtest (opening.height, expectedHeight, label + " opening height");
                    DBtest (opening.lower, expectedBottom - bottoms[bandIndex], label + " opening relative bottom");
                }
            }
            if (walls.Get (1).openings.GetSize () == 2)
                DBtest (
                    walls.Get (1).openings.Get (0).objLoc, reversed ? 22.0 : 11.0, label + " opening order preserved");
        }
    }

    void TestOtdWallAddOne () {
        struct Case {
            const char *name;
            double dx;
            double dy;
            double height;
            bool accepted;
        };

        const Case cases[] = {{"zero", 0, 0, 3, false},
                              {"below minimum", Roombook::min_dim / 2, 0, 3, false},
                              {"short diagonal", Roombook::min_dim * 0.6, Roombook::min_dim * 0.6, 3, false},
                              {"exact minimum", Roombook::min_dim, 0, 3, true},
                              {"above minimum", Roombook::min_dim * 2, 0, 3, true},
                              {"horizontal", 3, 0, 3, true},
                              {"vertical", 0, 4, 3, true},
                              {"diagonal", 3, 4, 3, true},
                              {"reversed", -3, -4, 3, true},
                              {"zero height", 3, 0, 0, true},
                              {"negative height", 3, 0, -3, true}};
        const API_Guid guid = APIGuidFromString ("{11111111-1111-1111-1111-111111111111}");
        for (const Case &c : cases) {
            GS::UniString label (c.name);
            Sector edge;
            edge.c1 = {0, 0};
            edge.c2 = {c.dx, c.dy};
            Roombook::OtdWall wall;
            wall.height = 99;
            wall.zBottom = 88;
            wall.base_th = 77;
            wall.floorInd = 9;
            wall.begC = {6, 7};
            wall.endC = {8, 9};
            wall.type = Roombook::Ceil;
            wall.base_type = API_ColumnID;
            wall.width = 12;
            wall.length = 13;
            wall.material.smaterial = "keep material";
            wall.favorite.name = "keep favorite";
            wall.otd_guid = guid;
            Roombook::OtdOpening opening;
            opening.objLoc = 42;
            wall.openings.Push (opening);
            bool accepted = Roombook::OtdWall_Add_One (guid, edge, true, c.height, -2, -3, 0.25, wall);
            DBtest (accepted == c.accepted, label + " return");
            DBtest (wall.width == 12 && wall.length == 13 && wall.material.smaterial == "keep material" &&
                        wall.favorite.name == "keep favorite" && wall.otd_guid == guid &&
                        wall.openings.GetSize () == 1 && wall.openings.Get (0).objLoc == 42,
                    label + " unrelated fields preserved");
            DBtest (edge.c1.x == 0 && edge.c1.y == 0 && edge.c2.x == c.dx && edge.c2.y == c.dy,
                    label + " edge unchanged");
            if (!accepted) {
                DBtest (wall.height == 99 && wall.zBottom == 88 && wall.base_th == 77 && wall.floorInd == 9 &&
                            wall.begC.x == 6 && wall.begC.y == 7 && wall.endC.x == 8 && wall.endC.y == 9 &&
                            wall.type == Roombook::Ceil && wall.base_type == API_ColumnID &&
                            wall.base_guid == APINULLGuid && !wall.base_flipped,
                        label + " rejected initialization does not mutate wall");
                continue;
            }
            // Инициализатор сейчас проверяет только длину ребра, не знак высоты.
            DBtest (wall.height, c.height, label + " height copied");
            DBtest (wall.zBottom == -2 && wall.base_th == 0.25 && wall.floorInd == -3 && wall.base_flipped &&
                        wall.base_guid == guid && wall.base_type == API_WallID && wall.type == Roombook::Wall_Main,
                    label + " initialization fields");
            DBtest (wall.begC.x == 0 && wall.begC.y == 0 && wall.endC.x == c.dx && wall.endC.y == c.dy,
                    label + " edge coordinates copied");
        }
    }

    void TestSetMaterialByType () {
        struct Case {
            const char *name;
            Roombook::TypeOtd type;
            int materialIndex;
            Roombook::TypeOtd expectedType;
            bool collapsible;
        };

        const Case cases[] = {{"unset", Roombook::NoSet, 7, Roombook::NoSet, false},
                              {"main", Roombook::Wall_Main, 0, Roombook::Wall_Main, false},
                              {"up", Roombook::Wall_Up, 1, Roombook::Wall_Up, true},
                              {"down", Roombook::Wall_Down, 2, Roombook::Wall_Down, true},
                              {"reveal", Roombook::Reveal_Main, 3, Roombook::Reveal_Main, true},
                              {"reveal up", Roombook::Reveal_Up, 3, Roombook::Reveal_Main, true},
                              {"reveal down", Roombook::Reveal_Down, 3, Roombook::Reveal_Main, true},
                              {"column", Roombook::Column, 4, Roombook::Column, true},
                              {"floor", Roombook::Floor, 5, Roombook::Floor, false},
                              {"ceil", Roombook::Ceil, 6, Roombook::Ceil, false},
                              {"default sloped", Roombook::Sloped, 7, Roombook::Sloped, false}};
        const char *names[] = {"main", "up", "down", "reveal", "column", "floor", "ceil", "zone"};
        const auto sameMaterial = [] (const Roombook::OtdMaterial &a, const Roombook::OtdMaterial &b) {
            return a.material == b.material && a.smaterial == b.smaterial && a.rawname == b.rawname &&
                   a.rawname_bytype == b.rawname_bytype;
        };
        for (int mode = 0; mode < 6; ++mode) {
            for (const Case &c : cases) {
                for (bool overrideFinish : {false, true}) {
                    Roombook::OtdMaterial materials[8];
                    for (int i = 0; i < 8; ++i) {
                        materials[i].material = (short)(i + 1);
                        materials[i].smaterial = names[i];
                        materials[i].rawname = names[i];
                        materials[i].rawname_bytype = materials[i].rawname + " by type";
                        if (i < 7 && (mode == 3 || (i > 0 && (mode == 1 || mode == 4))))
                            materials[i].smaterial = EMPTYSTRING;
                        if (i > 0 && i < 7 && mode == 2)
                            materials[i].rawname = "main";
                        if (i < 7 && mode == 4)
                            materials[i].rawname = EMPTYSTRING;
                        if (i > 0 && i < 7 && mode == 5)
                            materials[i].rawname = "MAIN";
                    }
                    const Roombook::OtdMaterial before[] = {materials[0],
                                                            materials[1],
                                                            materials[2],
                                                            materials[3],
                                                            materials[4],
                                                            materials[5],
                                                            materials[6],
                                                            materials[7]};
                    int selected = c.materialIndex;
                    if ((mode == 1 || mode == 4) && c.collapsible)
                        selected = 0;
                    if ((mode == 1 || mode == 4) && (c.type == Roombook::Floor || c.type == Roombook::Ceil))
                        selected = 7;
                    if (mode == 3)
                        selected = 7;
                    Roombook::TypeOtd expectedType = c.expectedType;
                    if ((mode == 2 || mode == 4) && c.collapsible)
                        expectedType = Roombook::Wall_Main;
                    Roombook::OtdMaterial expected = before[selected];
                    // Состав меняет имя/индекс после выбора типа, но не property rawname выбранной настройки.
                    if (overrideFinish) {
                        expected.material = 43;
                        expected.smaterial = "finish override";
                    }
                    Roombook::OtdWall wall;
                    wall.type = c.type;
                    wall.height = 3;
                    wall.zBottom = -2;
                    wall.width = 0.25;
                    wall.length = 9;
                    wall.floorInd = -1;
                    ParamValueComposite layer;
                    layer.structype = overrideFinish ? -1 : 0;
                    layer.pos = "finish override";
                    layer.val = "seed value";
                    layer.length = 43;
                    layer.fillThick = 0.2;
                    wall.base_composite.Push (layer);
                    Roombook::SetMaterialByType (wall,
                                                 materials[0],
                                                 materials[1],
                                                 materials[2],
                                                 materials[3],
                                                 materials[4],
                                                 materials[5],
                                                 materials[6],
                                                 materials[7]);
                    const char *modeNames[] = {"full",
                                               "empty secondary",
                                               "equal rawname",
                                               "zone fallback",
                                               "empty rawname",
                                               "different rawname case"};
                    GS::UniString label = GS::UniString (c.name) + " " + modeNames[mode] +
                                          (overrideFinish ? " composite finish" : " structural layer");
                    DBtest (wall.type == expectedType, label + " result type");
                    DBtest (sameMaterial (wall.material, expected), label + " all material fields");
                    DBtest (wall.height == 3 && wall.zBottom == -2 && wall.width == 0.25 && wall.length == 9 &&
                                wall.floorInd == -1,
                            label + " wall geometry unchanged");
                    for (int i = 0; i < 8; ++i)
                        DBtest (sameMaterial (materials[i], before[i]), label + " settings unchanged");
                    DBtest (wall.base_composite.GetSize () == 2, label + " one finish appended");
                    if (wall.base_composite.GetSize () != 2)
                        continue;
                    const ParamValueComposite &first = wall.base_composite.Get (0);
                    const ParamValueComposite &last = wall.base_composite.Get (1);
                    DBtest (first.structype == layer.structype && first.pos == layer.pos && first.val == layer.val &&
                                first.length == layer.length && first.fillThick == layer.fillThick,
                            label + " original layer preserved");
                    DBtest (last.structype == -1 && last.val == expected.smaterial,
                            label + " appended finish matches final material");
                }
            }
        }
    }

    void TestOpeningRevealsCreateOne () {
        struct Case {
            const char *name;
            double bottom;
            double height;
            double width;
            double depth;
            int orientation;
            bool zeroPerp;
            UInt32 count;
            double resultBottom;
            double resultHeight;
            double wallBottom = 0;
            double position = 5;
            double perpendicularScale = 1;
        };

        const Case cases[] = {
            {"normal", 1, 3, 2, 0.5, 0, false, 3, 1, 3},
            {"diagonal wall", 1, 3, 2, 0.5, 4, false, 3, 1, 3},
            {"reversed diagonal wall", 1, 3, 2, 0.5, 5, false, 3, 1, 3},
            {"opening at wall start", 1, 3, 2, 0.5, 0, false, 3, 1, 3, 0, 0},
            {"opening at wall end", 1, 3, 2, 0.5, 0, false, 3, 1, 3, 0, 10},
            {"opening before wall", 1, 3, 2, 0.5, 4, false, 3, 1, 3, 0, -2},
            {"opening after wall", 1, 3, 2, 0.5, 5, false, 3, 1, 3, 0, 12},
            {"negative opening width", 1, 3, -2, 0.5, 4, false, 3, 1, 3},
            {"scaled perpendicular", 1, 3, 2, 0.5, 4, false, 3, 1, 3, 0, 5, 2},
            {"opposite perpendicular", 1, 3, 2, 0.5, 5, false, 3, 1, 3, 0, 5, -1},
            {"clip bottom", -1, 3, 2, 0.5, 0, false, 3, 0, 2},
            {"clip top", 8, 4, 2, 0.5, 0, false, 2, 8, 2},
            {"exact top", 7, 3, 2, 0.5, 0, false, 3, 7, 3},
            {"touch top", 10, 3, 2, 0.5, 0, false, 0, 0, 0},
            {"touch bottom", -3, 3, 2, 0.5, 0, false, 0, 0, 0},
            {"short depth", 1, 3, 2, Roombook::min_dim / 2, 0, false, 0, 0, 0},
            {"exact depth", 1, 3, 2, Roombook::min_dim, 0, false, 3, 1, 3},
            {"zero wall", 1, 3, 2, 0.5, 3, false, 0, 0, 0},
            {"zero width", 1, 3, 0, 0.5, 0, false, 2, 1, 3},
            {"zero perpendicular", 1, 3, 2, 0.5, 0, true, 1, 1, 3},
            {"reversed wall", 1, 3, 2, 0.5, 1, false, 3, 1, 3},
            {"vertical wall", 1, 3, 2, 0.5, 2, false, 3, 1, 3},
            {"positive wall datum", 101, 3, 2, 0.5, 0, false, 3, 101, 3, 100},
            {"negative wall datum", -99, 3, 2, 0.5, 0, false, 3, -99, 3, -100},
            {"shifted clip bottom", 9, 3, 2, 0.5, 0, false, 3, 10, 2, 10},
            {"shifted clip top", 18, 4, 2, 0.5, 0, false, 2, 18, 2, 10},
            {"shifted exact top", 17, 3, 2, 0.5, 0, false, 3, 17, 3, 10},
            {"shifted below wall", 1, 3, 2, 0.5, 0, false, 0, 0, 0, 10},
            {"short opening height", 0, Roombook::min_dim / 2, 2, 0.5, 0, false, 0, 0, 0},
            {"exact opening height", 0, Roombook::min_dim, 2, 0.5, 0, false, 3, 0, Roombook::min_dim},
            {"above minimum height", 0, Roombook::min_dim * 2, 2, 0.5, 0, false, 3, 0, Roombook::min_dim * 2},
            {"short clipped height", -Roombook::min_dim, Roombook::min_dim * 1.5, 2, 0.5, 0, false, 0, 0, 0},
            {"exact clipped height",
             -Roombook::min_dim,
             Roombook::min_dim * 2,
             2,
             0.5,
             0,
             false,
             3,
             0,
             Roombook::min_dim}};
        const API_Guid guid = APIGuidFromString ("{11111111-1111-1111-1111-111111111111}");
        for (const Case &c : cases) {
            for (bool hadReveal : {false, true}) {
                GS::UniString label (c.name);
                Roombook::OtdWall source;
                source.begC = {10, 20};
                source.endC = {20, 20};
                if (c.orientation == 1) {
                    source.begC = {20, 20};
                    source.endC = {10, 20};
                }
                if (c.orientation == 2)
                    source.endC = {10, 30};
                if (c.orientation == 3)
                    source.endC = source.begC;
                if (c.orientation == 4)
                    source.endC = {16, 28};
                if (c.orientation == 5) {
                    source.begC = {16, 28};
                    source.endC = {10, 20};
                }
                source.height = 10;
                source.zBottom = c.wallBottom;
                source.base_guid = guid;
                source.floorInd = -1;
                source.material.smaterial = "source material";
                source.favorite.name = "source favorite";
                ParamValueComposite layer;
                layer.structype = 0;
                layer.val = "source layer";
                source.base_composite.Push (layer);
                Roombook::OtdOpening opening;
                opening.objLoc = c.position;
                opening.width = c.width;
                opening.zBottom = c.bottom;
                opening.height = c.height;
                opening.base_reveal_width = c.depth;
                opening.has_reveal = hadReveal;
                source.openings.Push (opening);
                Geometry::Vector2<double> perpendicular = {0, 1};
                if (c.orientation == 2)
                    perpendicular = {-1, 0};
                if (c.orientation == 4)
                    perpendicular = {-0.8, 0.6};
                if (c.orientation == 5)
                    perpendicular = {0.8, -0.6};
                perpendicular.x *= c.perpendicularScale;
                perpendicular.y *= c.perpendicularScale;
                if (c.zeroPerp)
                    perpendicular = {0, 0};
                Roombook::OtdMaterial materials[8];
                const char *names[] = {"main", "up", "down", "reveal", "column", "floor", "ceil", "zone"};
                for (int i = 0; i < 8; ++i) {
                    materials[i].smaterial = names[i];
                    materials[i].rawname = names[i];
                }
                GS::Array<Roombook::OtdWall> walls;
                GS::Array<Roombook::OtdSlab> slabs;
                Roombook::OtdSlab sentinel;
                sentinel.height = 99;
                slabs.Push (sentinel);
                double bottom = c.wallBottom, down = 0, main = 0, up = 0, height = 20;
                Roombook::OpeningReveals_Create_One (slabs,
                                                     source,
                                                     opening,
                                                     perpendicular,
                                                     walls,
                                                     bottom,
                                                     down,
                                                     main,
                                                     up,
                                                     height,
                                                     materials[0],
                                                     materials[1],
                                                     materials[2],
                                                     materials[3],
                                                     materials[4],
                                                     materials[5],
                                                     materials[6],
                                                     materials[7]);
                DBtest (walls.GetSize () == c.count, label + " count/order");
                DBtest (opening.has_reveal == (hadReveal || c.count > 0), label + " reveal flag");
                DBtest (opening.objLoc == c.position && opening.width == c.width && opening.height == c.height &&
                            opening.zBottom == c.bottom && opening.base_reveal_width == c.depth,
                        label + " opening dimensions unchanged");
                DBtest (source.height == 10 && source.zBottom == c.wallBottom && source.base_guid == guid &&
                            source.floorInd == -1 && source.material.smaterial == "source material" &&
                            source.favorite.name == "source favorite" && source.base_composite.GetSize () == 1 &&
                            source.openings.GetSize () == 1,
                        label + " source preserved");
                DBtest (slabs.GetSize () == 1 && slabs.Get (0).height == 99, label + " slab output untouched");
                DBtest (bottom == c.wallBottom && down == 0 && main == 0 && up == 0 && height == 20,
                        label + " band inputs unchanged");
                if (walls.GetSize () != c.count)
                    continue;
                for (UInt32 i = 0; i < walls.GetSize (); ++i) {
                    const Roombook::OtdWall &wall = walls.Get (i);
                    bool top = c.zeroPerp || i == 2;
                    double left = c.position - c.width / 2, right = c.position + c.width / 2;
                    const auto pointAt = [&] (double distance) -> Point2D {
                        if (c.orientation == 1)
                            return {20 - distance, 20};
                        if (c.orientation == 2)
                            return {10, 20 + distance};
                        if (c.orientation == 4)
                            return {10 + distance * 0.6, 20 + distance * 0.8};
                        if (c.orientation == 5)
                            return {16 - distance * 0.6, 28 - distance * 0.8};
                        return {10 + distance, 20};
                    };
                    Point2D beg = pointAt (i == 0 && !top ? left : right);
                    Point2D end = top ? pointAt (left) : beg;
                    if (!top) {
                        if (i == 0) {
                            end.x += perpendicular.x * c.depth;
                            end.y += perpendicular.y * c.depth;
                        } else {
                            beg.x += perpendicular.x * c.depth;
                            beg.y += perpendicular.y * c.depth;
                        }
                    }
                    DBtest (wall.begC.x, beg.x, label + " begin x");
                    DBtest (wall.begC.y, beg.y, label + " begin y");
                    DBtest (wall.endC.x, end.x, label + " end x");
                    DBtest (wall.endC.y, end.y, label + " end y");
                    DBtest (wall.height, top ? Roombook::otd_thickness : c.resultHeight, label + " height");
                    DBtest (wall.zBottom, top ? c.resultBottom + c.resultHeight : c.resultBottom, label + " bottom");
                    // Текущая подрезка переназначает length и для верхнего откоса при ненулевом width.
                    DBtest (wall.length, top ? Roombook::otd_thickness : c.resultHeight, label + " length");
                    DBtest (wall.width, c.depth, label + " depth");
                    DBtest (wall.base_guid == guid && wall.base_type == API_WindowID && wall.floorInd == -1 &&
                                wall.draw_type == (top ? API_BeamID : API_WallID) && wall.type == Roombook::Reveal_Main,
                            label + " identity/classification fields");
                    DBtest (wall.material.smaterial == "reveal" && wall.material.rawname == "reveal" &&
                                wall.base_composite.GetSize () == 2 &&
                                wall.base_composite.Get (0).val == "source layer",
                            label + " material/composite");
                    DBtest (wall.openings.IsEmpty () && wall.favorite.name.IsEmpty (),
                            label + " unused fields default");
                }
            }
        }

        struct Piece {
            int side;
            double bottom;
            double height;
            const char *material;
        };

        struct BandCase {
            const char *name;
            double openingBottom;
            double openingHeight;
            double bottom;
            double down;
            double main;
            double up;
            double total;
            UInt32 count;
            Piece pieces[5];
        };

        const double thickness = Roombook::otd_thickness;
        const BandCase bandCases[] = {
            {"three bands",
             1,
             3,
             0,
             2,
             2,
             6,
             10,
             5,
             {{0, 1, 1, "down"}, {0, 2, 2, "main"}, {1, 1, 1, "down"}, {1, 2, 2, "main"}, {2, 4, thickness, "up"}}},
            {"down only and beam fallback",
             1,
             3,
             0,
             2,
             0,
             0,
             10,
             3,
             {{0, 1, 1, "down"}, {1, 1, 1, "down"}, {2, 4, thickness, "main"}}},
            {"band ends at beam bottom", 1, 3, 0, 0, 4, 0, 4, 2, {{0, 1, 3, "main"}, {1, 1, 3, "main"}}},
            {"beam partial intersection",
             1,
             3,
             0,
             0,
             4 + thickness / 2,
             0,
             10,
             3,
             {{0, 1, 3, "main"}, {1, 1, 3, "main"}, {2, 4, thickness / 2, "main"}}},
            {"rejected band and broad fallback",
             1,
             3,
             0,
             0.5,
             0,
             0,
             10,
             3,
             {{0, 1, 3, "main"}, {1, 1, 3, "main"}, {2, 4, thickness, "main"}}},
            {"all outputs rejected", 1, 3, 0, 0.5, 0, 0, 0.5, 0, {}},
            {"zero fallback height", 1, 3, 0, 0, 0, 0, 0, 0, {}},
            {"tiny band and broad fallback",
             1,
             3,
             0,
             Roombook::min_dim / 2,
             0,
             0,
             10,
             3,
             {{0, 1, 3, "main"}, {1, 1, 3, "main"}, {2, 4, thickness, "main"}}},
            {"opening above wall top", 8, 4, 0, 2, 2, 6, 10, 2, {{0, 8, 2, "up"}, {1, 8, 2, "up"}}},
            {"opening below wall bottom",
             -1,
             3,
             0,
             2,
             2,
             6,
             10,
             3,
             {{0, 0, 2, "down"}, {1, 0, 2, "down"}, {2, 2, thickness, "main"}}}};
        for (const BandCase &c : bandCases) {
            for (bool emptyReveal : {false, true}) {
                for (bool hadReveal : {false, true}) {
                    GS::UniString label (c.name);
                    label += emptyReveal ? " empty reveal" : " explicit reveal";
                    label += hadReveal ? " existing flag" : " unset flag";
                    Roombook::OtdWall source;
                    source.begC = {10, 20};
                    source.endC = {20, 20};
                    source.height = 10;
                    source.base_guid = guid;
                    source.floorInd = -1;
                    source.material.smaterial = "source material";
                    ParamValueComposite layer;
                    layer.structype = 0;
                    layer.val = "source layer";
                    source.base_composite.Push (layer);
                    Roombook::OtdOpening opening;
                    opening.objLoc = 5;
                    opening.width = 2;
                    opening.zBottom = c.openingBottom;
                    opening.height = c.openingHeight;
                    opening.base_reveal_width = 0.5;
                    opening.has_reveal = hadReveal;
                    source.openings.Push (opening);
                    Geometry::Vector2<double> perpendicular = {0, 1};
                    Roombook::OtdMaterial materials[8];
                    const char *names[] = {"main", "up", "down", "reveal", "column", "floor", "ceil", "zone"};
                    for (int i = 0; i < 8; ++i) {
                        materials[i].smaterial = names[i];
                        materials[i].rawname = names[i];
                        materials[i].rawname_bytype = names[i];
                    }
                    if (emptyReveal)
                        materials[3].smaterial = EMPTYSTRING;
                    GS::Array<Roombook::OtdWall> walls;
                    Roombook::OtdWall sentinelWall;
                    sentinelWall.height = 99;
                    sentinelWall.material.smaterial = "sentinel wall";
                    walls.Push (sentinelWall);
                    GS::Array<Roombook::OtdSlab> slabs;
                    Roombook::OtdSlab sentinelSlab;
                    sentinelSlab.height = 98;
                    slabs.Push (sentinelSlab);
                    double bottom = c.bottom, down = c.down, main = c.main, up = c.up, height = c.total;
                    Roombook::OpeningReveals_Create_One (slabs,
                                                         source,
                                                         opening,
                                                         perpendicular,
                                                         walls,
                                                         bottom,
                                                         down,
                                                         main,
                                                         up,
                                                         height,
                                                         materials[0],
                                                         materials[1],
                                                         materials[2],
                                                         materials[3],
                                                         materials[4],
                                                         materials[5],
                                                         materials[6],
                                                         materials[7]);
                    DBtest (walls.GetSize () == c.count + 1, label + " count/order");
                    // Флаг отмечает попытку построения до Delim_All, а не наличие выходных стенок.
                    DBtest (opening.has_reveal, label + " flag retained even when all outputs rejected");
                    DBtest (walls.Get (0).height == 99 && walls.Get (0).material.smaterial == "sentinel wall",
                            label + " existing wall retained");
                    DBtest (slabs.GetSize () == 1 && slabs.Get (0).height == 98, label + " slab output untouched");
                    DBtest (bottom == c.bottom && down == c.down && main == c.main && up == c.up && height == c.total,
                            label + " band inputs unchanged");
                    DBtest (opening.objLoc == 5 && opening.width == 2 && opening.zBottom == c.openingBottom &&
                                opening.height == c.openingHeight && opening.base_reveal_width == 0.5,
                            label + " opening dimensions unchanged");
                    DBtest (source.height == 10 && source.base_guid == guid && source.floorInd == -1 &&
                                source.begC.x == 10 && source.endC.x == 20 &&
                                source.material.smaterial == "source material" &&
                                source.base_composite.GetSize () == 1 &&
                                source.base_composite.Get (0).val == "source layer" &&
                                source.openings.GetSize () == 1 && source.openings.Get (0).has_reveal == hadReveal,
                            label + " source preserved");
                    for (int i = 0; i < 8; ++i) {
                        DBtest (materials[i].smaterial == GS::UniString (i == 3 && emptyReveal ? "" : names[i]) &&
                                    materials[i].rawname == names[i] && materials[i].rawname_bytype == names[i],
                                label + " settings preserved");
                    }
                    if (walls.GetSize () != c.count + 1)
                        continue;
                    for (UInt32 i = 0; i < c.count; ++i) {
                        const Piece &piece = c.pieces[i];
                        const Roombook::OtdWall &wall = walls.Get (i + 1);
                        Point2D beg = {14, 20}, end = {14, 20.5};
                        if (piece.side == 1) {
                            beg = {16, 20.5};
                            end = {16, 20};
                        } else if (piece.side == 2) {
                            beg = {16, 20};
                            end = {14, 20};
                        }
                        DBtest (wall.begC.x, beg.x, label + " begin x");
                        DBtest (wall.begC.y, beg.y, label + " begin y");
                        DBtest (wall.endC.x, end.x, label + " end x");
                        DBtest (wall.endC.y, end.y, label + " end y");
                        DBtest (wall.zBottom, piece.bottom, label + " bottom");
                        DBtest (wall.height, piece.height, label + " height");
                        DBtest (wall.length, piece.height, label + " length after clipping");
                        DBtest (wall.width, 0.5, label + " depth");
                        DBtest (wall.type == Roombook::Reveal_Main && wall.base_type == API_WindowID &&
                                    wall.draw_type == (piece.side == 2 ? API_BeamID : API_WallID) &&
                                    wall.base_guid == guid && wall.floorInd == -1,
                                label + " identity/classification");
                        GS::UniString material (emptyReveal ? piece.material : "reveal");
                        DBtest (wall.material.smaterial == material && wall.material.rawname == material &&
                                    wall.material.rawname_bytype == material,
                                label + " selected material");
                        DBtest (wall.base_composite.GetSize () == 2 &&
                                    wall.base_composite.Get (0).val == "source layer",
                                label + " copied composite and finish");
                        DBtest (wall.openings.IsEmpty () && wall.favorite.name.IsEmpty (),
                                label + " unused fields default");
                    }
                }
            }
        }
    }

    void TestClearZoneGuid () {
        const API_Guid zones[] = {APIGuidFromString ("{11111111-1111-1111-1111-111111111111}"),
                                  APIGuidFromString ("{22222222-2222-2222-2222-222222222222}"),
                                  APIGuidFromString ("{33333333-3333-3333-3333-333333333333}"),
                                  APINULLGuid};
        const API_Guid firstElement = APIGuidFromString ("{44444444-4444-4444-4444-444444444444}");
        const API_Guid secondElement = APIGuidFromString ("{55555555-5555-5555-5555-555555555555}");
        const API_ElemTypeID types[] = {API_WindowID, API_DoorID, API_WallID, API_ColumnID, API_SlabID, API_ZoneID};

        struct Case {
            const char *name;
            UInt32 indices[6];
            UInt32 count;
            UInt32 uniqueCount;
            UInt32 mask;
            bool stress = false;
        };

        const Case cases[] = {{"empty list", {}, 0, 0, 0},
                              {"single zone", {0}, 1, 1, 1},
                              {"unique pair", {0, 1}, 2, 2, 3},
                              {"unique triple", {0, 1, 2}, 3, 3, 7},
                              {"one repeated zone", {0, 0, 0}, 3, 1, 1},
                              {"grouped duplicates", {0, 0, 1, 1, 2, 2}, 6, 3, 7},
                              {"interleaved duplicates", {1, 0, 1, 2, 0, 2}, 6, 3, 7},
                              {"reverse duplicates", {2, 1, 0, 2, 1, 0}, 6, 3, 7},
                              {"null zone retained", {3, 3}, 2, 1, 8},
                              {"mixed null zone", {0, 3, 0, 3, 2}, 5, 3, 13},
                              {"duplicate prefix", {2, 2, 0, 1}, 4, 3, 7},
                              {"large repeated input", {}, 0, 4, 15, true}};

        Roombook::UnicElementByType emptyIndex;
        Roombook::ClearZoneGUID (emptyIndex);
        DBtest (emptyIndex.IsEmpty (), "ClearZoneGUID empty index stays empty");
        Roombook::UnicElementByType emptyBucket;
        Roombook::UnicElement noElements;
        emptyBucket.Add (API_WallID, noElements);
        Roombook::ClearZoneGUID (emptyBucket);
        DBrequire (emptyBucket.GetPtr (API_WallID) != nullptr, "ClearZoneGUID empty bucket key retained");
        DBtest (emptyBucket.GetSize () == 1 && emptyBucket.GetPtr (API_WallID)->IsEmpty (),
                "ClearZoneGUID empty bucket stays empty without adding types");

        for (const Case &c : cases) {
            GS::UniString label = GS::UniString ("ClearZoneGUID ") + c.name;
            GS::Array<API_Guid> input;
            for (UInt32 i = 0; i < c.count; ++i)
                input.Push (zones[c.indices[i]]);
            if (c.stress) {
                for (UInt32 i = 0; i < 512; ++i)
                    input.Push (zones[i % 4]);
            }
            GS::Array<API_Guid> sentinel;
            sentinel.Push (zones[2]);
            sentinel.Push (zones[2]);
            Roombook::UnicElementByType index;
            for (API_ElemTypeID type : types) {
                Roombook::UnicElement elements;
                elements.Add (firstElement, input);
                elements.Add (secondElement, sentinel);
                index.Add (type, elements);
            }
            GS::Array<API_Guid> skipped;
            skipped.Push (zones[3]);
            skipped.Push (zones[1]);
            skipped.Push (zones[3]);
            Roombook::UnicElement skippedElements;
            skippedElements.Add (firstElement, skipped);
            index.Add (API_BeamID, skippedElements);

            // Порядок обработанных GUID не фиксируем: production перечисляет HashTable.
            for (UInt32 repeat = 0; repeat < 2; ++repeat) {
                Roombook::ClearZoneGUID (index);
                DBtest (index.GetSize () == 7, label + " type keys retained");
                for (API_ElemTypeID type : types) {
                    const Roombook::UnicElement *elements = index.GetPtr (type);
                    DBrequire (elements != nullptr, label + " processed type retained");
                    DBtest (elements->GetSize () == 2, label + " element keys retained");
                    const GS::Array<API_Guid> *actual = elements->GetPtr (firstElement);
                    const GS::Array<API_Guid> *other = elements->GetPtr (secondElement);
                    DBrequire (actual != nullptr && other != nullptr, label + " element GUIDs retained");
                    DBtest (actual->GetSize () == c.uniqueCount, label + " unique count");
                    UInt32 counts[4] = {};
                    for (const API_Guid &guid : *actual) {
                        bool known = false;
                        for (UInt32 i = 0; i < 4; ++i) {
                            if (guid == zones[i]) {
                                ++counts[i];
                                known = true;
                            }
                        }
                        DBtest (known, label + " no foreign zone GUID");
                    }
                    for (UInt32 i = 0; i < 4; ++i)
                        DBtest (counts[i] == ((c.mask & (1u << i)) != 0 ? 1u : 0u),
                                label + " membership and uniqueness");
                    DBtest (other->GetSize () == 1 && other->Get (0) == zones[2],
                            label + " scratch cleared between elements and types");
                }
                const Roombook::UnicElement *unprocessed = index.GetPtr (API_BeamID);
                DBrequire (unprocessed != nullptr, label + " unprocessed type retained");
                const GS::Array<API_Guid> *untouched = unprocessed->GetPtr (firstElement);
                DBrequire (untouched != nullptr, label + " unprocessed element retained");
                DBtest (unprocessed->GetSize () == 1 && untouched->GetSize () == 3 && untouched->Get (0) == zones[3] &&
                            untouched->Get (1) == zones[1] && untouched->Get (2) == zones[3],
                        label + " unprocessed list preserves duplicates and order");
            }
        }
    }

    void TestRoomMaterialQuantities () {
        const API_Guid firstZone = APIGuidFromString ("{11111111-1111-1111-1111-111111111111}");
        const API_Guid secondZone = APIGuidFromString ("{22222222-2222-2222-2222-222222222222}");
        const GS::UniString materials[] = {
            "M", "N", "", GS::UniString ("Материал", CC_UTF8), GS::UniString ("материал", CC_UTF8)};
        const Roombook::TypeOtd types[] = {Roombook::NoSet,
                                           Roombook::Wall_Main,
                                           Roombook::Wall_Up,
                                           Roombook::Wall_Down,
                                           Roombook::Reveal_Main,
                                           Roombook::Reveal_Up,
                                           Roombook::Reveal_Down,
                                           Roombook::Column,
                                           Roombook::Floor,
                                           Roombook::Ceil,
                                           Roombook::Sloped};

        struct Case {
            const char *name;
            const char *layers[2];
            UInt32 layerCount;
            double expected[5];
            UInt32 geometry = 0;
            UInt32 source = 0;
            bool valid = true;
            UInt32 count = 1;
        };

        const Case cases[] = {{"empty room", {}, 0, {}, 0, 0, true, 0},
                              {"wall without layers", {}, 0, {}},
                              {"single wall material", {"M"}, 1, {12}},
                              {"duplicate inside layer", {"M;M"}, 1, {12}},
                              {"split layer", {"M;N"}, 1, {12, 12}},
                              {"duplicate layers", {"M", "M"}, 2, {24}},
                              {"overlapping layers", {"M;N", "M"}, 2, {24, 12}},
                              {"ignored marker substring", {"prefix----suffix"}, 1, {}},
                              {"mixed ignored material", {"M;----;N"}, 1, {12, 12}},
                              {"empty material name", {""}, 1, {0, 0, 12}},
                              {"unicode case preserved", {"Материал;материал"}, 1, {0, 0, 0, 12, 12}},
                              {"invalid wall", {"M"}, 1, {}, 0, 0, false},
                              {"two walls", {"M"}, 1, {24}, 0, 0, true, 2},
                              {"reveal length", {"M"}, 1, {2}, 1},
                              {"reveal height", {"M"}, 1, {1.5}, 2},
                              {"negative height", {"M"}, 1, {}, 3},
                              {"opening larger than wall", {"M"}, 1, {}, 4},
                              {"partial opening", {"M"}, 1, {10}, 5},
                              {"zero length", {"M"}, 1, {}, 6},
                              {"below area threshold", {"M"}, 1, {}, 7},
                              {"exact area threshold", {"M"}, 1, {0.000001}, 8},
                              {"float accumulation order", {"M"}, 1, {1e16}, 9, 0, true, 3},
                              {"rectangular slab", {"M"}, 1, {6}, 0, 1},
                              {"slab split and dedup", {"M;M;N"}, 1, {6, 6}, 0, 1},
                              {"invalid slab", {"M"}, 1, {}, 0, 1, false},
                              {"empty polygon", {"M"}, 1, {}, 0, 2},
                              {"slab without layers", {}, 0, {}, 0, 1},
                              {"two slabs", {"M"}, 1, {12}, 0, 1, true, 2}};
        Box2DData box = {};
        box.xMin = 0;
        box.yMin = 0;
        box.xMax = 2;
        box.yMax = 3;
        const Geometry::Polygon2D rectangle (box);
        DBtest (rectangle.CalcArea () == 6, "Room materials rectangular fixture area");
        Roombook::ColumnFormatDict format;
        for (Roombook::TypeOtd type : types) {
            for (const Case &c : cases) {
                GS::UniString label = GS::UniString ("Room materials ") + c.name;
                Roombook::OtdRoom room;
                room.zone_guid = firstZone;
                room.tip_otd = "target";
                // Пустые rawname изолируют расчёт от форматирования и записи свойств.
                for (UInt32 i = 0; i < c.count; ++i) {
                    Roombook::OtdWall wall;
                    wall.endC = {4, 0};
                    wall.height = 3;
                    wall.type = type;
                    wall.isValid = c.valid;
                    if (c.geometry == 1) {
                        wall.width = 0.5;
                        wall.length = 4;
                    }
                    if (c.geometry == 2)
                        wall.width = 0.5;
                    if (c.geometry == 3)
                        wall.height = -3;
                    if (c.geometry == 4 || c.geometry == 5) {
                        Roombook::OtdOpening opening;
                        opening.height = c.geometry == 4 ? 10 : 2;
                        opening.width = c.geometry == 4 ? 10 : 1;
                        wall.openings.Push (opening);
                    }
                    if (c.geometry == 6)
                        wall.endC = wall.begC;
                    if (c.geometry == 7 || c.geometry == 8) {
                        wall.endC = {1, 0};
                        wall.height = c.geometry == 7 ? 0.0000001 : 0.000001;
                    }
                    if (c.geometry == 9)
                        wall.height = i == 0 ? 2500000000000000.0 : 0.25;
                    for (UInt32 j = 0; j < c.layerCount; ++j) {
                        ParamValueComposite layer;
                        layer.val = GS::UniString (c.layers[j], CC_UTF8);
                        wall.base_composite.Push (layer);
                    }
                    if (c.source == 0) {
                        room.otdwall.Push (wall);
                    } else {
                        Roombook::OtdSlab slab;
                        slab.type = type;
                        slab.isValid = c.valid;
                        slab.base_composite = wall.base_composite;
                        if (c.source == 1)
                            slab.poly = rectangle;
                        room.otdslab.Push (slab);
                    }
                }
                const Roombook::OtdRoom before = room;
                ParamDictElement read;
                ParamDictElement write;
                ParamDictValue untouched;
                write.Add (firstZone, untouched);
                Roombook::OtdMaterialAreaDict sentinel;
                sentinel.Add ("keep", 99);
                Roombook::OtdMaterialAreaDictByType other;
                other.Add (Roombook::Floor, sentinel);
                Roombook::OtdMaterialAreaDictByOtdType quantities;
                quantities.Add ("other", other);
                for (UInt32 repeat = 1; repeat <= 2; ++repeat) {
                    room.zone_guid = repeat == 1 ? firstZone : secondZone;
                    Roombook::OtdData_CalcForRoom (format, room, write, read, quantities);
                    DBtest (quantities.GetSize () == 2, label + " finish type keys retained");
                    DBrequire (quantities.GetPtr ("target") != nullptr, label + " target finish type created");
                    const Roombook::OtdMaterialAreaDictByType &actual = quantities.Get ("target");
                    UInt32 expectedCount = 0;
                    for (UInt32 j = 0; j < 5; ++j)
                        if (c.expected[j] != 0)
                            ++expectedCount;
                    DBtest (actual.GetSize () == (expectedCount == 0 ? 0u : 1u), label + " only requested finish band");
                    if (expectedCount != 0) {
                        DBrequire (actual.GetPtr (type) != nullptr, label + " finish band retained");
                        const Roombook::OtdMaterialAreaDict &byMaterial = actual.Get (type);
                        DBtest (byMaterial.GetSize () == expectedCount, label + " exact material keys");
                        for (UInt32 j = 0; j < 5; ++j) {
                            if (c.expected[j] == 0) {
                                DBtest (!byMaterial.ContainsKey (materials[j]), label + " absent material");
                            } else {
                                const double *area = byMaterial.GetPtr (materials[j]);
                                DBrequire (area != nullptr, label + " expected material present");
                                DBtest (*area == c.expected[j] * repeat, label + " exact accumulated area");
                            }
                        }
                    }
                    DBtest (quantities.Get ("other").Get (Roombook::Floor).Get ("keep") == 99,
                            label + " unrelated finish type unchanged");
                    DBtest (read.IsEmpty () && write.GetSize () == 1 && write.Get (firstZone).IsEmpty (),
                            label + " no property output without rawnames");
                    DBtest (room.otdwall.GetSize () == before.otdwall.GetSize () &&
                                room.otdslab.GetSize () == before.otdslab.GetSize (),
                            label + " inputs retained");
                    for (UInt32 j = 0; j < room.otdwall.GetSize (); ++j) {
                        const auto &wall = room.otdwall[j];
                        const auto &original = before.otdwall[j];
                        DBtest (wall.endC.x == original.endC.x && wall.height == original.height &&
                                    wall.width == original.width && wall.length == original.length &&
                                    wall.type == original.type && wall.isValid == original.isValid &&
                                    wall.openings.GetSize () == original.openings.GetSize (),
                                label + " wall unchanged");
                        for (UInt32 k = 0; k < wall.base_composite.GetSize (); ++k)
                            DBtest (wall.base_composite[k].val == original.base_composite[k].val,
                                    label + " source layer unchanged");
                    }
                    for (UInt32 j = 0; j < room.otdslab.GetSize (); ++j)
                        DBtest (room.otdslab[j].poly.CalcArea () == before.otdslab[j].poly.CalcArea () &&
                                    room.otdslab[j].type == before.otdslab[j].type &&
                                    room.otdslab[j].isValid == before.otdslab[j].isValid,
                                label + " slab unchanged");
                }
            }
        }
    }

    void TestRoomParameterIsolation () {
        const API_Guid zones[] = {APIGuidFromString ("{11111111-1111-1111-1111-111111111111}"),
                                  APIGuidFromString ("{22222222-2222-2222-2222-222222222222}")};
        const auto add = [] (Roombook::ReadParams &requests,
                             ParamDictValue &data,
                             const GS::UniString &key,
                             const ParamValueData &value,
                             bool valid = true) {
            Roombook::ReadParam request;
            request.rawnames.Push (key);
            requests.Add (key, request);
            ParamValue p;
            p.rawName = key;
            p.isValid = valid;
            p.val = value;
            data.Add (key, p);
        };
        const auto number = [] (double value) {
            ParamValueData p;
            p.type = API_PropertyRealValueType;
            p.doubleValue = value;
            return p;
        };
        const auto boolean = [] (bool value) {
            ParamValueData p;
            p.type = API_PropertyBooleanValueType;
            p.boolValue = value;
            return p;
        };
        const auto string = [] (const GS::UniString &value) {
            ParamValueData p;
            p.type = API_PropertyStringValueType;
            p.uniStringValue = value;
            return p;
        };
        GS::HashTable<GS::UniString, GS::Int32> materials;
        materials.Add ("untouched", 49);
        const char *flagKeys[] = {"create_column_elements",
                                  "create_wall_elements",
                                  "create_floor_elements",
                                  "create_ceil_elements",
                                  "create_reveal_elements"};
        // Перебираем все отдельные флаги: общий отсутствует, невалиден, false либо true.
        for (UInt32 mode = 0; mode < 4; ++mode) {
            for (UInt32 mask = 0; mask < 32; ++mask) {
                for (UInt32 initial = 0; initial < 2; ++initial) {
                    Roombook::ReadParams requests;
                    ParamDictValue data;
                    add (requests, data, "tip_otd", string ("flags"));
                    if (mode != 0)
                        add (requests, data, "create_all_elements", boolean (mode == 3), mode != 1);
                    for (UInt32 j = 0; j < 5; ++j)
                        add (requests, data, flagKeys[j], boolean ((mask & (1u << j)) != 0));
                    ParamDictElement read;
                    read.Add (zones[0], data);
                    Roombook::OtdRoom room;
                    room.zone_guid = zones[0];
                    room.height = 3;
                    room.create_all_elements = initial != 0;
                    Roombook::Param_SetToRooms (materials, room, read, requests);
                    const bool actual[] = {room.create_column_elements,
                                           room.create_wall_elements,
                                           room.create_floor_elements,
                                           room.create_ceil_elements,
                                           room.create_reveal_elements};
                    for (UInt32 j = 0; j < 5; ++j) {
                        const bool expected = mode >= 2 ? mode == 3 : (mask & (1u << j)) != 0;
                        DBtest (actual[j] == expected, "Room params all/individual flag precedence");
                    }
                    DBtest (room.create_all_elements == (mode >= 2 ? mode == 3 : initial != 0),
                            "Room params absent/invalid global preserves initial value");
                    DBtest (room.tip_otd == "flags" && room.height_main == 3 && room.height_up == 0,
                            "Room params flags fixture read and normalized");
                }
            }
        }

        struct HeightCase {
            const char *name;
            double height;
            double main;
            double down;
            double resultHeight;
            double resultMain;
            double resultDown;
            double resultUp;
            bool valid;
            UInt32 direct = 2;
            bool fallback = true;
        };

        const HeightCase heights[] = {{"explicit bands", 3, 2.5, 0.5, 3, 2, 0.5, 0.5, true},
                                      {"main above height", 3, 5, 0.5, 3, 2.5, 0.5, 0, true},
                                      {"main below threshold", 3, 0.099, 0.5, 3, 2.5, 0.5, 0, true},
                                      {"main at threshold", 3, 0.1, 0.5, 3, -0.4, 0.5, 2.9, true},
                                      {"negative main", 3, -2, 0.5, 3, 2.5, 0.5, 0, true},
                                      {"negative down", 3, 2, -0.5, 3, 2.5, -0.5, 1, true},
                                      {"zero height", 0, 2, 0.5, 0, -0.5, 0.5, 0, false},
                                      {"height below validity threshold", 0.00009, 0, 0, 0, 0, 0, 0, false},
                                      {"height at validity threshold", 0.0001, 0, 0, 0, 0, 0, 0, true},
                                      {"millimeter rounding", 3.1234, 2.5434, 0.4324, 3.123, 2.111, 0.432, 0.58, true},
                                      {"missing direct uses fallback", 3, 2.5, 0.5, 3, 2, 0.5, 0.5, true, 0},
                                      {"invalid direct uses fallback", 3, 2.5, 0.5, 3, 2, 0.5, 0.5, true, 1},
                                      {"false fallback makes zero", 3, 2.5, 0.5, 3, 2.5, 0, 0.5, true, 0, false}};
        for (const HeightCase &c : heights) {
            Roombook::ReadParams requests;
            ParamDictValue data;
            add (requests, data, "tip_otd", string ("height"));
            add (requests, data, "height_main", number (c.main));
            if (c.direct != 0)
                add (requests, data, "height_down", number (c.down), c.direct == 2);
            add (requests, data, "him_has_height_down", boolean (c.fallback));
            add (requests, data, "him_height_down", number (c.direct == 2 ? 99 : c.down));
            ParamDictElement read;
            read.Add (zones[0], data);
            Roombook::OtdRoom room;
            room.zone_guid = zones[0];
            room.height = c.height;
            room.height_up = 99;
            Roombook::Param_SetToRooms (materials, room, read, requests);
            const GS::UniString label = GS::UniString ("Room params height ") + c.name;
            DBtest (room.height, c.resultHeight, label + " total");
            DBtest (room.height_main, c.resultMain, label + " main");
            DBtest (room.height_down, c.resultDown, label + " down");
            DBtest (room.height_up, c.resultUp, label + " up");
            DBtest (room.isValid == c.valid, label + " validity before rounding");
        }
        Roombook::ReadParams seed;
        ParamDictValue data[2];
        for (UInt32 i = 0; i < 2; ++i) {
            Roombook::ReadParams requests;
            add (requests, data[i], "tip_otd", string (GS::UniString (i == 0 ? "Зона А" : "Зона Б", CC_UTF8)));
            add (requests, data[i], "tip_pot", string (i == 0 ? "ceiling A" : "ceiling B"));
            add (requests, data[i], "tip_pol", string (i == 0 ? "floor A" : "invalid floor B"), i == 0);
            add (requests, data[i], "height_main", number (i == 0 ? 2.5 : 3.5));
            add (requests, data[i], "height_down", number (i == 0 ? 0.5 : 1));
            add (requests, data[i], "has_ceil", boolean (i != 0));
            add (requests, data[i], "has_floor", boolean (i == 0));
            add (requests, data[i], "ceil_by_slab", boolean (i == 0));
            add (requests, data[i], "floor_by_slab", boolean (i == 0));
            add (requests, data[i], "create_all_elements", boolean (false), i == 0);
            for (UInt32 j = 0; j < 5; ++j)
                add (requests, data[i], flagKeys[j], boolean (i == 0 || j == 1 || j == 3 || j == 4));
            if (i == 0)
                seed = requests;
        }
        Roombook::ReadParam target;
        target.isValid = true;
        target.val = string ("preset target");
        seed.Add ("om_reveals.rawname", target);
        ParamDictElement read;
        read.Add (zones[0], data[0]);
        read.Add (zones[1], data[1]);
        // Как в ProcessRoomFinishes: новая рабочая копия шаблона для каждой зоны.
        for (UInt32 reverse = 0; reverse < 2; ++reverse) {
            for (UInt32 step = 0; step < 2; ++step) {
                const UInt32 i = reverse == 0 ? step : 1 - step;
                Roombook::ReadParams work = seed;
                Roombook::OtdRoom room;
                room.zone_guid = zones[i];
                room.height = i == 0 ? 3 : 4.5;
                room.tip_pol = "keep floor B";
                room.ceil_by_slab = true;
                room.floor_by_slab = i != 0;
                Roombook::Param_SetToRooms (materials, room, read, work);
                DBtest (room.tip_otd == data[i].Get ("tip_otd").val.uniStringValue,
                        "Room params independent zone Unicode finish");
                DBtest (room.tip_pot == data[i].Get ("tip_pot").val.uniStringValue, "Room params independent ceiling");
                DBtest (room.tip_pol == (i == 0 ? "floor A" : "keep floor B"),
                        "Room params invalid value does not inherit first zone");
                DBtest (room.height_main == (i == 0 ? 2 : 2.5) && room.height_down == (i == 0 ? 0.5 : 1) &&
                            room.height_up == (i == 0 ? 0.5 : 1),
                        "Room params independent heights");
                DBtest (room.has_ceil == (i != 0) && room.has_floor == (i == 0),
                        "Room params independent availability");
                DBtest (room.ceil_by_slab == (i == 0) && room.floor_by_slab,
                        "Room params slab flag assigned only when enabled");
                DBtest (room.create_wall_elements == (i != 0) && room.create_ceil_elements == (i != 0) &&
                            room.create_reveal_elements == (i != 0) && !room.create_floor_elements &&
                            !room.create_column_elements,
                        "Room params independent create flags");
                DBtest (room.om_reveals.rawname == "preset target", "Room params preset target retained");
                DBtest (!seed.Get ("tip_otd").isValid && !seed.Get ("height_down").isValid &&
                            !seed.Get ("create_all_elements").isValid && seed.Get ("om_reveals.rawname").isValid,
                        "Room params request template unchanged");
                DBtest (read.Get (zones[1]).Get ("tip_pol").isValid == false &&
                            read.Get (zones[0]).Get ("height_down").val.doubleValue == 0.5 &&
                            read.Get (zones[1]).Get ("height_down").val.doubleValue == 1,
                        "Room params source dictionary unchanged");
            }
        }
        // Fallback выбирает первый валидный rawname; false vots не считается ответом.
        for (UInt32 mode = 0; mode < 6; ++mode) {
            Roombook::ReadParams requests;
            Roombook::ReadParam request;
            request.rawnames.Push ("{@gdl:vots}");
            request.rawnames.Push ("fallback");
            request.isValid = mode == 4;
            request.val = string ("preset");
            requests.Add ("selection", request);
            ParamDictValue source;
            ParamValue first;
            first.isValid = mode != 1;
            first.val = number (7);
            first.val.boolValue = mode != 2;
            first.fromProperty = mode == 3;
            source.Add ("{@gdl:vots}", first);
            ParamValue fallback;
            fallback.isValid = mode != 5;
            fallback.val = number (11);
            source.Add ("fallback", fallback);
            if (mode == 5)
                source.Get ("{@gdl:vots}").isValid = false;
            ParamDictElement inputs;
            inputs.Add (zones[0], source);
            const bool got = Roombook::Param_Property_Read (zones[0], inputs, requests);
            const Roombook::ReadParam &result = requests.Get ("selection");
            DBtest (got == (mode != 4 && mode != 5), "Room reader actual-read return flag");
            if (mode == 4) {
                DBtest (result.val.uniStringValue == "preset", "Room reader skips preset valid request");
            } else if (mode == 5) {
                DBtest (!result.isValid && result.val.uniStringValue == "preset",
                        "Room reader invalid sources preserve previous value");
            } else {
                DBtest (result.isValid && result.val.doubleValue == (mode == 1 || mode == 2 ? 11 : 7),
                        "Room reader ordered fallback and false vots");
                DBtest (result.val.type == (mode == 3 ? API_PropertyStringValueType : API_PropertyRealValueType),
                        "Room reader property type override");
            }
            DBtest (inputs.Get (zones[0]).Get ("fallback").val.doubleValue == 11, "Room reader source retained");
        }
        Roombook::ReadParams empty;
        DBtest (!Roombook::Param_Property_Read (zones[0], read, empty), "Room reader empty requests return false");
        DBtest (materials.GetSize () == 1 && materials.Get ("untouched") == 49,
                "Room params no material lookup without material requests");
    }

    void TestRoomMaterialFormatting () {
        const API_Guid zone = APIGuidFromString ("{11111111-1111-1111-1111-111111111111}");
        const API_Guid other = APIGuidFromString ("{22222222-2222-2222-2222-222222222222}");
        const GS::UniString raw = "{@property:fixture/finish}";
        const GS::UniString untouched = "{@property:fixture/untouched}";
        Roombook::ColumnFormat format;
        format.no_breake_space = "_";
        format.narow_space = "~";
        format.width_mat = 0;
        format.width_area = 0;
        format.width_narow_space = 1;
        format.delim_line = "|";
        format.space_line = "<wrap>";
        GS::UniString probe = "2.00";
        GS::UniString narrow = "~";
        const double probeWidth = GetTextWidth (format.font, format.fontsize, probe);
        const double narrowWidth = GetTextWidth (format.font, format.fontsize, narrow);
        DBtest (probeWidth > 0.1 && narrowWidth > 0.001, "Room formatting SDK text measurement available");
        if (probeWidth <= 0.1 || narrowWidth <= 0.001)
            return;
        const Roombook::TypeOtd types[] = {Roombook::NoSet,
                                           Roombook::Wall_Main,
                                           Roombook::Wall_Up,
                                           Roombook::Wall_Down,
                                           Roombook::Reveal_Main,
                                           Roombook::Reveal_Up,
                                           Roombook::Reveal_Down,
                                           Roombook::Column,
                                           Roombook::Floor,
                                           Roombook::Ceil,
                                           Roombook::Sloped};
        const char *expected[] = {"M~2.00 ",
                                  "-",
                                  "-",
                                  "M~5.00 ",
                                  "Mat1~1.00 |Mat2~2.00 |Mat10~10.00 ",
                                  "M~2.00 ",
                                  "M~2.00 ",
                                  "Материал~2.00 ",
                                  "-",
                                  "M~2.00 ",
                                  "A<wrap>B~2.00 "};
        for (const Roombook::TypeOtd type : types) {
            const Roombook::TypeOtd second = type == Roombook::Wall_Main ? Roombook::Wall_Up : Roombook::Wall_Main;
            for (UInt32 mode = 0; mode < 11; ++mode) {
                ParamValue original;
                original.rawName = raw;
                original.name = "fixture name";
                original.fromGuid = zone;
                original.fromProperty = true;
                original.isValid = false;
                original.val.type = API_PropertyStringValueType;
                original.val.uniStringValue = "old";
                original.val.doubleValue = 47;
                ParamValue sentinel = original;
                sentinel.rawName = untouched;
                sentinel.val.uniStringValue = "keep";
                ParamDictValue values;
                values.Add (raw, original);
                values.Add (untouched, sentinel);
                ParamDictElement input;
                input.Add (zone, values);
                input.Add (other, values);
                ParamDictElement output;
                ParamDictValue keep;
                keep.Add (untouched, sentinel);
                output.Add (zone, keep);
                output.Add (other, values);
                GS::HashTable<Roombook::TypeOtd, GS::UniString> names;
                names.Add (type, raw);
                names.Add (Roombook::Floor == type ? Roombook::Ceil : Roombook::Floor, "");
                Roombook::OtdMaterialAreaDict inner;
                if (mode == 4) {
                    inner.Add ("Mat10", 10);
                    inner.Add ("Mat2", 2);
                    inner.Add ("Mat1", 1);
                } else if (mode != 1 && mode != 2) {
                    GS::UniString material = "M";
                    if (mode == 5)
                        material = "   M   ";
                    if (mode == 6)
                        material = "0&#& M";
                    if (mode == 7)
                        material = GS::UniString ("Материал", CC_UTF8);
                    if (mode == 8)
                        material = "";
                    if (mode == 10)
                        material = "A  B";
                    inner.Add (material, 2);
                }
                Roombook::OtdMaterialAreaDictByType materials;
                if (mode != 1)
                    materials.Add (type, inner);
                if (mode == 3 || mode == 9) {
                    names.Put (second, raw);
                    if (mode == 3) {
                        Roombook::OtdMaterialAreaDict more;
                        more.Add ("M", 3);
                        materials.Add (second, more);
                    }
                }
                Roombook::ColumnFormatDict formats;
                formats.Add (raw, format);
                Roombook::OtdData_WriteToRoom (formats, zone, output, input, materials, names);
                DBtest (output.Get (zone).ContainsKey (raw), "Room formatting output present");
                if (!output.Get (zone).ContainsKey (raw))
                    continue;
                const ParamValue &result = output.Get (zone).Get (raw);
                const GS::UniString text (expected[mode], CC_UTF8);
                DBtest (result.val.uniStringValue, text, "Room formatting exact material text");
                DBtest (result.isValid && result.fromProperty && result.fromGuid == zone &&
                            result.name == original.name && result.val.type == original.val.type &&
                            result.val.doubleValue == 47,
                        "Room formatting retains metadata");
                DBtest (output.Get (zone).Get (untouched).val.uniStringValue == "keep" &&
                            output.Get (other).Get (raw).val.uniStringValue == "old" && output.GetSize () == 2,
                        "Room formatting unrelated outputs retained");
                DBtest (input.Get (zone).Get (raw).val.uniStringValue == "old" && !input.Get (zone).Get (raw).isValid &&
                            input.Get (other).Get (raw).val.uniStringValue == "old",
                        "Room formatting input immutable");
                DBtest (materials.ContainsKey (type) == (mode != 1), "Room formatting material dictionary retained");
                // Имитируем следующий read-back только в локальной фикстуре, без записи в модель.
                ParamDictElement reread = input;
                reread.Get (zone).Put (raw, result);
                ParamDictElement repeat;
                Roombook::OtdData_WriteToRoom (formats, zone, repeat, reread, materials, names);
                DBtest (repeat.IsEmpty (), "Room formatting unchanged text no write");
                names.Put (type, "missing");
                names.Put (second, "");
                ParamDictElement absent;
                Roombook::OtdData_WriteToRoom (formats, zone, absent, input, materials, names);
                DBtest (absent.IsEmpty (), "Room formatting absent/empty target no write");
            }
        }
        // Выравнивание проверяется при измеренной ширине, без привязки к метрике установленного шрифта.
        for (UInt32 spaces = 0; spaces < 4; ++spaces) {
            Roombook::ColumnFormat padded = format;
            padded.width_narow_space = narrowWidth;
            // Малый запас исключает потерю целого числа пробелов из-за float-погрешности.
            padded.width_area = probeWidth + (spaces + 0.0001) * narrowWidth;
            GS::UniString material = "M";
            padded.width_mat = GetTextWidth (padded.font, padded.fontsize, material);
            ParamValue p;
            p.rawName = raw;
            p.val.uniStringValue = "old";
            ParamDictValue v;
            v.Add (raw, p);
            ParamDictElement input;
            input.Add (zone, v);
            Roombook::OtdMaterialAreaDict areas;
            areas.Add ("M", 2);
            Roombook::OtdMaterialAreaDictByType dct;
            dct.Add (Roombook::Wall_Main, areas);
            GS::HashTable<Roombook::TypeOtd, GS::UniString> names;
            names.Add (Roombook::Wall_Main, raw);
            Roombook::ColumnFormatDict formats;
            formats.Add (raw, padded);
            ParamDictElement output;
            Roombook::OtdData_WriteToRoom (formats, zone, output, input, dct, names);
            GS::UniString expectedText = "M";
            if (spaces > 1) {
                for (UInt32 i = 0; i < spaces; ++i)
                    expectedText.Append ("~");
            }
            expectedText.Append ("2.00 ");
            DBtest (output.Get (zone).Get (raw).val.uniStringValue,
                    expectedText,
                    "Room formatting measured area alignment");
        }
    }

    void TestOpeningParameterIsolation () {
        const API_Guid guids[] = {APIGuidFromString ("{11111111-1111-1111-1111-111111111111}"),
                                  APIGuidFromString ("{22222222-2222-2222-2222-222222222222}")};
        const char *keys[] = {"frame", "sill", "plaster_show_3D", "plaster_show_2D", "AutoTurnIn", "bOverIn"};
        Roombook::ReadParams requests;
        for (const char *key : keys) {
            Roombook::ReadParam request;
            request.rawnames.Push (key);
            requests.Add (key, request);
        }
        const auto dataFor = [&] (double frame, double sill, UInt32 flags, int invalid = -1) {
            ParamDictValue data;
            for (UInt32 i = 0; i < 6; ++i) {
                ParamValue value;
                value.rawName = keys[i];
                value.isValid = static_cast<int> (i) != invalid;
                if (i < 2) {
                    value.val.type = API_PropertyRealValueType;
                    value.val.doubleValue = i == 0 ? frame : sill;
                } else {
                    value.val.type = API_PropertyBooleanValueType;
                    value.val.boolValue = (flags & (1u << (i - 2))) != 0;
                }
                data.Add (keys[i], value);
            }
            return data;
        };
        const auto openingFor = [&] (UInt32 i) {
            Roombook::OtdOpening op;
            op.base_guid = guids[i];
            op.otd_guid = guids[1 - i];
            op.width = i == 0 ? 1.5 : 2.5;
            op.height = i == 0 ? 2.2 : 3.2;
            op.zBottom = 7 + i;
            op.objLoc = 3 + i;
            op.lower = 0.4;
            op.base_reveal_width = 99;
            op.reflected = i != 0;
            op.has_reveal = i == 0;
            return op;
        };
        const auto unchangedOpening = [] (const Roombook::OtdOpening &op, const Roombook::OtdOpening &before) {
            DBtest (op.base_guid == before.base_guid && op.otd_guid == before.otd_guid &&
                        op.reflected == before.reflected && op.has_reveal == before.has_reveal,
                    "Opening params preserve identity and flags");
            DBtest (op.zBottom, before.zBottom, "Opening params preserve bottom");
            DBtest (op.objLoc, before.objLoc, "Opening params preserve location");
            DBtest (op.lower, before.lower, "Opening params preserve lower");
        };
        const auto unchangedTemplate = [&] () {
            DBtest (requests.GetSize () == 6, "Opening params template size");
            for (const char *key : keys) {
                const Roombook::ReadParam &request = requests.Get (key);
                DBtest (!request.isValid && request.rawnames.GetSize () == 1 && request.rawnames.Get (0) == key &&
                            request.val.doubleValue == 0 && !request.val.boolValue,
                        "Opening params template remains unread");
            }
        };
        const auto wallFor = [] (double thickness, UInt32 layers) {
            Roombook::OtdWall wall;
            wall.base_th = thickness;
            const double widths[] = {0.02, 0.03, 0.55};
            for (UInt32 i = 0; i < layers; ++i) {
                ParamValueComposite layer;
                layer.fillThick = widths[i];
                layer.val = "source";
                wall.base_composite.Push (layer);
            }
            return wall;
        };
        // Условие поправки и число включённых слоёв характеризуем по прежнему коду.
        for (UInt32 layers = 0; layers <= 3; ++layers) {
            for (UInt32 flags = 0; flags < 16; ++flags) {
                const Roombook::OtdWall wall = wallFor (0.6, layers);
                ParamDictElement read;
                read.Add (guids[0], dataFor (0.1, 0.05, flags));
                Roombook::ReadParams work = requests;
                Roombook::OtdOpening op = openingFor (0);
                const Roombook::OtdOpening before = op;
                Roombook::Param_SetToWindows (op, read, work, wall);
                const bool show = (flags & 3u) == 3u;
                const double autoThickness[] = {0, 0, 0.02, 0.05};
                const double plaster =
                    !show ? 0
                          : ((flags & 4u) != 0 ? autoThickness[layers] : ((flags & 8u) != 0 && layers > 0 ? 0.02 : 0));
                DBtest (op.base_reveal_width, show ? 0.45 : 0, "Opening params reveal depth by visibility");
                DBtest (op.width, 1.5 - plaster * 2, "Opening params layer width correction");
                DBtest (op.height, 2.2 - plaster, "Opening params layer height correction");
                unchangedOpening (op, before);
                DBtest (wall.base_th == 0.6 && wall.base_composite.GetSize () == layers,
                        "Opening params wall metadata unchanged");
                for (UInt32 i = 0; i < layers; ++i) {
                    const double widths[] = {0.02, 0.03, 0.55};
                    DBtest (wall.base_composite.Get (i).fillThick == widths[i] &&
                                wall.base_composite.Get (i).val == "source",
                            "Opening params source layers unchanged");
                }
                for (UInt32 i = 0; i < 6; ++i) {
                    const Roombook::ReadParam &actual = work.Get (keys[i]);
                    const ParamValue &source = read.Get (guids[0]).Get (keys[i]);
                    DBtest (actual.isValid && source.isValid && source.rawName == keys[i],
                            "Opening params read all required keys");
                    if (i < 2)
                        DBtest (actual.val.doubleValue, source.val.doubleValue, "Opening params numeric values");
                    else
                        DBtest (actual.val.boolValue == ((flags & (1u << (i - 2))) != 0) &&
                                    source.val.boolValue == actual.val.boolValue,
                                "Opening params flag values");
                }
                unchangedTemplate ();
            }
        }
        // Два проёма имеют разные числа и флаги; второй может иметь невалидный либо отсутствующий ключ.
        for (UInt32 mode = 0; mode < 13; ++mode) {
            for (UInt32 order = 0; order < 2; ++order) {
                ParamDictElement read;
                read.Add (guids[0], dataFor (0.1, 0.05, 15));
                read.Add (guids[1], dataFor (0.12, 0.03, 3, mode > 0 && mode <= 6 ? static_cast<int> (mode) - 1 : -1));
                for (UInt32 step = 0; step < 2; ++step) {
                    const UInt32 i = order == 0 ? step : 1 - step;
                    const Roombook::OtdWall wall = wallFor (i == 0 ? 0.6 : 0.8, 3);
                    Roombook::ReadParams work = requests;
                    if (i == 1 && mode >= 7)
                        work.Delete (keys[mode - 7]);
                    Roombook::OtdOpening op = openingFor (i);
                    const Roombook::OtdOpening before = op;
                    Roombook::Param_SetToWindows (op, read, work, wall);
                    const bool complete = i == 0 || mode == 0;
                    DBtest (op.base_reveal_width,
                            complete ? (i == 0 ? 0.45 : 0.65) : 0.8,
                            "Opening params independent depth and partial return");
                    DBtest (op.width, i == 0 ? 1.4 : 2.5, "Opening params independent width");
                    DBtest (op.height, i == 0 ? 2.15 : 3.2, "Opening params independent height");
                    unchangedOpening (op, before);
                    for (UInt32 j = 0; j < 6; ++j) {
                        const auto *actual = work.GetPtr (keys[j]);
                        if (i == 1 && mode >= 7 && j == mode - 7) {
                            DBtest (actual == nullptr, "Opening params missing request stays absent");
                        } else {
                            const bool valid = !(i == 1 && mode > 0 && mode <= 6 && j == mode - 1);
                            DBtest (actual != nullptr && actual->isValid == valid,
                                    "Opening params validity isolated by GUID");
                            if (actual != nullptr && valid) {
                                const ParamValueData &expected = read.Get (guids[i]).Get (keys[j]).val;
                                if (j < 2)
                                    DBtest (actual->val.doubleValue,
                                            expected.doubleValue,
                                            "Opening params GUID-specific number");
                                else
                                    DBtest (actual->val.boolValue == expected.boolValue,
                                            "Opening params GUID-specific flag");
                            }
                        }
                    }
                    unchangedTemplate ();
                }
                for (UInt32 i = 0; i < 2; ++i) {
                    const ParamDictValue &source = read.Get (guids[i]);
                    DBtest (source.GetSize () == 6, "Opening params source dictionary size");
                    DBtest (source.Get ("frame").val.doubleValue,
                            i == 0 ? 0.1 : 0.12,
                            "Opening params source frame unchanged");
                    DBtest (source.Get ("sill").val.doubleValue,
                            i == 0 ? 0.05 : 0.03,
                            "Opening params source sill unchanged");
                    for (UInt32 j = 0; j < 6; ++j) {
                        const ParamValue &value = source.Get (keys[j]);
                        DBtest (value.rawName == keys[j] &&
                                    value.isValid == !(i == 1 && mode > 0 && mode <= 6 && j == mode - 1),
                                "Opening params source validity unchanged");
                    }
                }
            }
        }
        // Часть ранних выходов меняет глубину на толщину стены, но не меняет размеры.
        for (UInt32 mode = 0; mode < 5; ++mode) {
            ParamDictElement read;
            Roombook::ReadParams work = requests;
            if (mode != 0) {
                ParamDictValue data = dataFor (0.1, 0.05, 15);
                if (mode == 2) {
                    for (const char *key : keys)
                        data.Get (key).isValid = false;
                }
                read.Add (guids[0], data);
            }
            if (mode == 1)
                work.Clear ();
            if (mode == 3) {
                DBtest (Roombook::Param_Property_Read (guids[0], read, work), "Opening params seed cached reader");
            }
            if (mode == 4)
                read.Get (guids[0]).Clear ();
            const Roombook::OtdWall wall = wallFor (0.6, 3);
            Roombook::OtdOpening op = openingFor (0);
            const Roombook::OtdOpening before = op;
            Roombook::Param_SetToWindows (op, read, work, wall);
            DBtest (op.base_reveal_width, mode == 0 ? 99 : 0.6, "Opening params early-return depth");
            DBtest (op.width, before.width, "Opening params early-return width");
            DBtest (op.height, before.height, "Opening params early-return height");
            unchangedOpening (op, before);
            if (mode == 3)
                DBtest (work.Get ("frame").isValid && work.Get ("frame").val.doubleValue == 0.1,
                        "Opening params cached values survive aggregate false");
            unchangedTemplate ();
        }
    }

    void TestRoomParameterResolution () {
        auto &cache = PROPERTYCACHE ();
        const ParamDictValue original = cache.property;
        const bool originalFull = cache.isPropertyDefinitionRead_full;
        const bool originalOK = cache.isPropertyDefinition_OK;
        {
            // Подменяем только читаемую часть кэша и возвращаем её на любом выходе из фикстуры.
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
            const auto setProperty = [&] (const GS::UniString &description) {
                cache.property.Clear ();
                ParamValue property;
                property.rawName = "{@property:matched}";
                property.definition.description = description;
                property.val.uniStringValue = "property value sentinel";
                cache.property.Add (property.rawName, property);
            };
            const auto requestFor = [] (const GS::Array<GS::UniString> &names, bool valid = false) {
                Roombook::ReadParam request;
                request.rawnames = names;
                request.isValid = valid;
                request.val.uniStringValue = "request sentinel";
                return request;
            };

            struct MatchCase {
                const char *description;
                const char *token;
                bool found;
            };

            const MatchCase cases[] = {{"token", "token", true},
                                       {"prefix token suffix", "token", true},
                                       {"token_extra", "token", false},
                                       {"token and token_extra", "token", false},
                                       {"TOKEN", "token", false},
                                       {"other", "token", false},
                                       {"параметр отделки", "отделки", true}};
            for (const MatchCase &c : cases) {
                for (UInt32 raw = 0; raw < 2; ++raw) {
                    setProperty (c.description);
                    GS::Array<GS::UniString> names;
                    names.Push (c.token);
                    Roombook::ReadParams requests;
                    const GS::UniString key = raw == 0 ? "value_target" : "prefix_rawname_target";
                    requests.Add (key, requestFor (names));
                    Roombook::Param_Property_FindInParams (requests);
                    const Roombook::ReadParam &actual = requests.Get (key);
                    DBtest (actual.rawnames.GetSize () == (c.found ? 1u : 0u), "Resolution description filtering");
                    if (c.found)
                        DBtest (actual.rawnames.Get (0) == "{@property:matched}", "Resolution property raw name");
                    DBtest (actual.isValid == (raw != 0 && c.found), "Resolution rawname-only validity");
                    DBtest (actual.val.uniStringValue ==
                                (raw != 0 && c.found ? "{@property:matched}" : "request sentinel"),
                            "Resolution rawname-only value");
                    DBtest (cache.property.GetSize () == 1 &&
                                cache.property.Get ("{@property:matched}").definition.description == c.description &&
                                cache.property.Get ("{@property:matched}").val.uniStringValue ==
                                    "property value sentinel",
                            "Resolution source property unchanged");
                }
            }

            struct OrderCase {
                const char *key;
                bool initialValid;
                const char *input[3];
                UInt32 count;
                const char *output[3];
                bool valid;
                const char *value;
            };

            const OrderCase orders[] = {
                {"value",
                 false,
                 {"{@gdl:first}", "token", "{@property:direct}"},
                 3,
                 {"{@gdl:first}", "{@property:matched}", "{@property:direct}"},
                 false,
                 "request sentinel"},
                {"rawname",
                 false,
                 {"{@gdl:first}", "token", "{@gdl:last}"},
                 2,
                 {"{@gdl:first}", "{@property:matched}", ""},
                 true,
                 "{@property:matched}"},
                {"value",
                 false,
                 {"token", "{@gdl:last}", "missing"},
                 2,
                 {"{@property:matched}", "{@gdl:last}", ""},
                 false,
                 "request sentinel"},
                {"rawname",
                 false,
                 {"token", "{@gdl:last}", "missing"},
                 1,
                 {"{@property:matched}", "", ""},
                 true,
                 "{@property:matched}"},
                {"rawname",
                 false,
                 {"missing", "{@gdl:last}", "absent"},
                 1,
                 {"{@gdl:last}", "", ""},
                 false,
                 "request sentinel"},
                {"value", true, {"missing", "{@gdl:last}", "token"}, 0, {"", "", ""}, true, "request sentinel"},
                {"rawname",
                 true,
                 {"{@gdl:first}", "token", "{@gdl:last}"},
                 1,
                 {"{@gdl:first}", "", ""},
                 true,
                 "request sentinel"},
                {"rawname",
                 true,
                 {"token", "{@gdl:last}", "missing"},
                 1,
                 {"{@property:matched}", "", ""},
                 true,
                 "{@property:matched}"}};
            setProperty ("token");
            for (const OrderCase &c : orders) {
                GS::Array<GS::UniString> names;
                for (const char *name : c.input)
                    names.Push (name);
                Roombook::ReadParams requests;
                requests.Add (c.key, requestFor (names, c.initialValid));
                Roombook::Param_Property_FindInParams (requests);
                const Roombook::ReadParam &actual = requests.Get (c.key);
                DBtest (actual.rawnames.GetSize () == c.count, "Resolution order and early break count");
                for (UInt32 i = 0; i < actual.rawnames.GetSize () && i < c.count; ++i)
                    DBtest (actual.rawnames.Get (i) == c.output[i], "Resolution preserved candidate order");
                DBtest (actual.isValid == c.valid && actual.val.uniStringValue == c.value,
                        "Resolution preset-valid behavior");
            }
            // Дубликаты выбираются по фактическому порядку словаря, а не по предположению о сортировке.
            ParamValue duplicate;
            duplicate.rawName = "{@property:duplicate}";
            duplicate.definition.description = "token";
            cache.property.Add (duplicate.rawName, duplicate);
            GS::UniString first;
            for (const auto &p : cache.property) {
    #ifdef ServerMainVers_2800
                first = p.value.rawName;
    #else
                first = p.value->rawName;
    #endif
                break;
            }
            GS::Array<GS::UniString> token;
            token.Push ("token");
            Roombook::ReadParams duplicateRequest;
            duplicateRequest.Add ("rawname", requestFor (token));
            Roombook::Param_Property_FindInParams (duplicateRequest);
            DBtest (duplicateRequest.Get ("rawname").val.uniStringValue == first &&
                        duplicateRequest.Get ("rawname").rawnames.GetSize () == 1 && cache.property.GetSize () == 2,
                    "Resolution first matching property wins without source mutation");
            cache.property.Clear ();
            for (UInt32 valid = 0; valid < 2; ++valid) {
                Roombook::ReadParams requests;
                GS::Array<GS::UniString> names;
                requests.Add ("empty", requestFor (names, valid != 0));
                Roombook::Param_Property_FindInParams (requests);
                DBtest (requests.Get ("empty").rawnames.IsEmpty () && requests.Get ("empty").isValid == (valid != 0) &&
                            requests.Get ("empty").val.uniStringValue == "request sentinel",
                        "Resolution empty candidate list preserves value and validity");
            }
            Roombook::ReadParams empty;
            Roombook::Param_Property_FindInParams (empty);
            DBtest (empty.IsEmpty (), "Resolution empty request dictionary");
            cache.isPropertyDefinition_OK = false;
            Roombook::ReadParams unavailable;
            unavailable.Add ("rawname", requestFor (token));
            Roombook::Param_Property_FindInParams (unavailable);
            DBtest (!unavailable.Get ("rawname").isValid && unavailable.Get ("rawname").rawnames.Get (0) == "token" &&
                        unavailable.Get ("rawname").val.uniStringValue == "request sentinel",
                    "Resolution unavailable cached definitions leave request unchanged");
            const char *keys[] = {"frame",
                                  "sill",
                                  "useWallFinishSkin",
                                  "maxPlasterThk",
                                  "AutoTurnIn",
                                  "bOverIn",
                                  "plaster_show_3D",
                                  "plaster_show_2D"};
            const char *names[] = {"{@gdl:gs_frame_thk}",
                                   "{@gdl:gs_wido_sill}",
                                   "{@gdl:gs_usewallfinishskin}",
                                   "{@gdl:gs_maxplasterthk}",
                                   "{@gdl:gs_bautoturnin}",
                                   "{@gdl:gs_boverin}",
                                   "{@gdl:gs_turn_plaster_show_3d}",
                                   "{@gdl:gs_turn_plaster_dim_2d}"};
            for (UInt32 available = 0; available < 2; ++available) {
                cache.isPropertyDefinition_OK = available != 0;
                Roombook::ReadParams windows = Roombook::Param_GetForWindowParams ();
                DBtest (windows.GetSize () == 8, "Window request template size");
                for (UInt32 i = 0; i < 8; ++i) {
                    const auto *request = windows.GetPtr (keys[i]);
                    DBtest (request != nullptr && !request->isValid && request->rawnames.GetSize () == 1 &&
                                request->rawnames.Get (0) == names[i] && request->val.uniStringValue.IsEmpty (),
                            "Window request exact key/GDL name and default validity");
                }
            }
        }
        DBtest (cache.isPropertyDefinitionRead_full == originalFull && cache.isPropertyDefinition_OK == originalOK &&
                    cache.property.GetSize () == original.GetSize (),
                "Resolution cache metadata restored");
        for (const auto &p : original) {
    #ifdef ServerMainVers_2800
            const GS::UniString &key = p.key;
            const ParamValue &before = p.value;
    #else
            const GS::UniString &key = *p.key;
            const ParamValue &before = *p.value;
    #endif
            const auto *after = cache.property.GetPtr (key);
            DBtest (after != nullptr && after->rawName == before.rawName && after->isValid == before.isValid &&
                        after->definition.description == before.definition.description &&
                        after->val.uniStringValue == before.val.uniStringValue && after->val.type == before.val.type,
                    "Resolution cache property restored");
        }
    }

    void TestCachedParameterReader () {
        const API_Guid guids[] = {APIGuidFromString ("{11111111-1111-1111-1111-111111111111}"),
                                  APIGuidFromString ("{22222222-2222-2222-2222-222222222222}")};
        const auto valueFor = [&] (API_VariantType type, Int32 seed, bool boolean) {
            ParamValueData value;
            value.type = type;
            value.uniStringValue = seed == 1 ? "primary cached value" : "fallback cached value";
            value.intValue = seed * 10;
            value.boolValue = boolean;
            value.doubleValue = seed + 0.25;
            value.rawDoubleValue = seed + 0.125;
            value.guidval = guids[seed == 1 ? 0 : 1];
            value.canCalculate = seed == 1;
            value.hasrawDouble = seed != 1;
            value.hasFormula = seed == 1;
            value.array_row_start = seed;
            value.array_row_end = seed + 3;
            value.array_column_start = seed + 4;
            value.array_column_end = seed + 5;
            value.array_format_out = seed;
            value.formatstring.n_zero = seed;
            value.formatstring.stringformat = seed == 1 ? "primary format" : "fallback format";
            value.formatstring.needRound = seed == 1;
            value.formatstring.krat = seed * 2;
            value.formatstring.koeff = seed + 0.5;
            value.formatstring.trim_zero = seed == 1;
            value.formatstring.isRead = seed != 1;
            value.formatstring.isEmpty = seed == 1;
            value.formatstring.forceRaw = seed != 1;
            value.formatstring.delimetr = seed == 1 ? "," : ".";
            return value;
        };
        const auto sameValue = [] (const ParamValueData &a, const ParamValueData &b) {
            return a.type == b.type && a.uniStringValue == b.uniStringValue && a.intValue == b.intValue &&
                   a.boolValue == b.boolValue && a.doubleValue == b.doubleValue &&
                   a.rawDoubleValue == b.rawDoubleValue && a.guidval == b.guidval && a.canCalculate == b.canCalculate &&
                   a.hasrawDouble == b.hasrawDouble && a.hasFormula == b.hasFormula &&
                   a.array_row_start == b.array_row_start && a.array_row_end == b.array_row_end &&
                   a.array_column_start == b.array_column_start && a.array_column_end == b.array_column_end &&
                   a.array_format_out == b.array_format_out && a.formatstring.n_zero == b.formatstring.n_zero &&
                   a.formatstring.stringformat == b.formatstring.stringformat &&
                   a.formatstring.needRound == b.formatstring.needRound && a.formatstring.krat == b.formatstring.krat &&
                   a.formatstring.koeff == b.formatstring.koeff &&
                   a.formatstring.trim_zero == b.formatstring.trim_zero &&
                   a.formatstring.isRead == b.formatstring.isRead && a.formatstring.isEmpty == b.formatstring.isEmpty &&
                   a.formatstring.forceRaw == b.formatstring.forceRaw &&
                   a.formatstring.delimetr == b.formatstring.delimetr;
        };
        const auto sameNames = [] (const GS::Array<GS::UniString> &a, const GS::Array<GS::UniString> &b) {
            if (a.GetSize () != b.GetSize ())
                return false;
            for (UInt32 i = 0; i < a.GetSize (); ++i)
                if (a.Get (i) != b.Get (i))
                    return false;
            return true;
        };
        const auto requestFor = [&] (const GS::Array<GS::UniString> &names, bool valid = false) {
            Roombook::ReadParam request;
            request.rawnames = names;
            request.val = valueFor (API_PropertyRealValueType, 9, false);
            request.isValid = valid;
            return request;
        };
        const API_VariantType types[] = {API_PropertyUndefinedValueType,
                                         API_PropertyIntegerValueType,
                                         API_PropertyRealValueType,
                                         API_PropertyStringValueType,
                                         API_PropertyBooleanValueType,
                                         API_PropertyGuidValueType};

        struct TagCase {
            const char *name;
            bool filtered;
        };

        const TagCase tags[] = {{"{@gdl:vots}", true},
                                {"{@gdl:vots_fill}", true},
                                {"prefix{@gdl:vots}suffix", true},
                                {"{@gdl:vots_fill}_extra", true},
                                {"{@gdl:VOTS}", false},
                                {"ordinary", false}};
        for (API_VariantType type : types) {
            for (const TagCase &tag : tags) {
                for (UInt32 mask = 0; mask < 4; ++mask) {
                    const bool boolean = (mask & 1) != 0;
                    const bool property = (mask & 2) != 0;
                    ParamValue primary;
                    primary.rawName = tag.name;
                    primary.isValid = true;
                    primary.fromProperty = property;
                    primary.definition.description = "primary metadata";
                    primary.val = valueFor (type, 1, boolean);
                    ParamValue fallback;
                    fallback.rawName = "fallback";
                    fallback.isValid = true;
                    fallback.val = valueFor (API_PropertyIntegerValueType, 2, true);
                    ParamDictValue source;
                    source.Add (primary.rawName, primary);
                    source.Add (fallback.rawName, fallback);
                    ParamDictElement read;
                    read.Add (guids[0], source);
                    GS::Array<GS::UniString> names;
                    names.Push (tag.name);
                    names.Push ("missing");
                    names.Push ("fallback");
                    const Roombook::ReadParam templ = requestFor (names);
                    Roombook::ReadParams requests;
                    requests.Add ("target", templ);
                    ParamValueData expected = tag.filtered && !boolean ? fallback.val : primary.val;
                    if (!(tag.filtered && !boolean) && property)
                        expected.type = API_PropertyStringValueType;
                    DBtest (Roombook::Param_Property_Read (guids[0], read, requests), "Cached reader accepted value");
                    const auto &actual = requests.Get ("target");
                    DBtest (actual.isValid && sameValue (actual.val, expected),
                            "Cached reader complete value and type");
                    DBtest (sameNames (actual.rawnames, names), "Cached reader candidate names unchanged");
                    DBtest (!templ.isValid && sameValue (templ.val, valueFor (API_PropertyRealValueType, 9, false)) &&
                                sameNames (templ.rawnames, names),
                            "Cached reader template unchanged");
                    const auto &saved = read.Get (guids[0]).Get (tag.name);
                    DBtest (read.GetSize () == 1 && read.Get (guids[0]).GetSize () == 2 && saved.isValid &&
                                saved.fromProperty == property && saved.definition.description == "primary metadata" &&
                                saved.rawName == tag.name && sameValue (saved.val, primary.val) &&
                                sameValue (read.Get (guids[0]).Get ("fallback").val, fallback.val),
                            "Cached reader source data unchanged");
                    // Мутация результата не должна затрагивать источник или шаблон запроса.
                    requests.Get ("target").val.uniStringValue = "changed result";
                    requests.Get ("target").val.formatstring.stringformat = "changed format";
                    requests.Get ("target").rawnames[0] = "changed candidate";
                    DBtest (sameValue (read.Get (guids[0]).Get (tag.name).val, primary.val) &&
                                sameValue (read.Get (guids[0]).Get ("fallback").val, fallback.val) &&
                                sameNames (templ.rawnames, names) && templ.val.uniStringValue != "changed result",
                            "Cached reader result owns independent copies");
                }
            }
        }
        // Три кандидата: отсутствие, невалидность и false-фильтр различаются; порядок задаёт запрос.
        const char *rawnames[] = {"{@gdl:vots_fill}", "middle", "last"};
        const UInt32 orders[][3] = {{0, 1, 2}, {0, 2, 1}, {1, 0, 2}, {1, 2, 0}, {2, 0, 1}, {2, 1, 0}};
        for (UInt32 mode = 0; mode < 5; ++mode) {
            for (const auto &order : orders) {
                ParamDictValue source;
                for (UInt32 i = 0; i < 3; ++i) {
                    if (mode == 0 && i == 0)
                        continue;
                    ParamValue p;
                    p.rawName = rawnames[i];
                    p.isValid = mode != 4 && !(mode == 1 && i == 0);
                    p.val = valueFor (API_PropertyRealValueType, static_cast<Int32> (i + 1), mode != 2 || i != 0);
                    source.Add (p.rawName, p);
                }
                ParamDictElement read;
                read.Add (guids[0], source);
                GS::Array<GS::UniString> names;
                for (UInt32 i : order)
                    names.Push (rawnames[i]);
                Roombook::ReadParams requests;
                const Roombook::ReadParam templ = requestFor (names);
                requests.Add ("ordered", templ);
                Int32 chosen = -1;
                if (mode != 4)
                    for (UInt32 i : order)
                        if (mode == 3 || i != 0) {
                            chosen = static_cast<Int32> (i);
                            break;
                        }
                const bool got = Roombook::Param_Property_Read (guids[0], read, requests);
                const auto &actual = requests.Get ("ordered");
                DBtest (got == (chosen >= 0) && actual.isValid == (chosen >= 0), "Cached reader ordered fallback flag");
                DBtest (sameValue (actual.val, chosen >= 0 ? source.Get (rawnames[chosen]).val : templ.val),
                        "Cached reader ordered fallback value");
                DBtest (sameNames (actual.rawnames, names) && read.Get (guids[0]).GetSize () == source.GetSize (),
                        "Cached reader ordered fallback inputs retained");
            }
        }
        // Флаг возвращает наличие нового чтения, а не общую валидность набора.
        for (UInt32 mask = 0; mask < 8; ++mask) {
            ParamValue readable;
            readable.rawName = "readable";
            readable.isValid = true;
            readable.val = valueFor (API_PropertyStringValueType, 1, true);
            ParamDictValue source;
            source.Add (readable.rawName, readable);
            ParamDictElement read;
            read.Add (guids[0], source);
            Roombook::ReadParams requests;
            for (UInt32 i = 0; i < 3; ++i) {
                GS::Array<GS::UniString> names;
                names.Push (i == 0 ? "readable" : "missing");
                requests.Add (rawnames[i], requestFor (names, (mask & (1u << i)) != 0));
            }
            const Roombook::ReadParams before = requests;
            DBtest (Roombook::Param_Property_Read (guids[0], read, requests) == ((mask & 1) == 0),
                    "Cached reader aggregate is new-read any, not all-valid");
            for (UInt32 i = 0; i < 3; ++i) {
                const auto &actual = requests.Get (rawnames[i]);
                const auto &saved = before.Get (rawnames[i]);
                const bool newlyRead = i == 0 && (mask & 1) == 0;
                DBtest (actual.isValid == (saved.isValid || newlyRead) &&
                            sameValue (actual.val, newlyRead ? readable.val : saved.val) &&
                            sameNames (actual.rawnames, saved.rawnames),
                        "Cached reader mixed request states");
            }
            DBtest (sameValue (read.Get (guids[0]).Get ("readable").val, readable.val),
                    "Cached reader aggregate source unchanged");
        }
        // Без записи GUID безопасны только пути, которые не разыменовывают baseparam.
        for (UInt32 mode = 0; mode < 4; ++mode) {
            ParamDictElement read;
            Roombook::ReadParams requests;
            GS::Array<GS::UniString> names;
            if (mode == 1 || mode == 3)
                names.Push ("missing");
            if (mode != 0)
                requests.Add ("safe", requestFor (names, mode == 1));
            if (mode == 3)
                read.Add (guids[0], ParamDictValue{});
            const Roombook::ReadParams before = requests;
            DBtest (!Roombook::Param_Property_Read (guids[0], read, requests), "Cached reader safe empty/preset exits");
            if (mode != 0)
                DBtest (requests.Get ("safe").isValid == before.Get ("safe").isValid &&
                            sameValue (requests.Get ("safe").val, before.Get ("safe").val) &&
                            sameNames (requests.Get ("safe").rawnames, before.Get ("safe").rawnames),
                        "Cached reader safe exit preserves request");
        }
        for (UInt32 reverse = 0; reverse < 2; ++reverse) {
            ParamDictElement read;
            GS::Array<GS::UniString> names;
            names.Push ("shared");
            for (UInt32 i = 0; i < 2; ++i) {
                ParamValue p;
                p.isValid = true;
                p.val = valueFor (API_PropertyRealValueType, static_cast<Int32> (i + 1), i == 0);
                ParamDictValue source;
                source.Add ("shared", p);
                read.Add (guids[i], source);
            }
            const Roombook::ReadParam templ = requestFor (names);
            for (UInt32 j = 0; j < 2; ++j) {
                const UInt32 i = reverse == 0 ? j : 1 - j;
                Roombook::ReadParams requests;
                requests.Add ("target", templ);
                DBtest (Roombook::Param_Property_Read (guids[i], read, requests) &&
                            sameValue (requests.Get ("target").val,
                                       valueFor (API_PropertyRealValueType, static_cast<Int32> (i + 1), i == 0)),
                        "Cached reader GUID isolation in both orders");
                DBtest (sameNames (requests.Get ("target").rawnames, names) && !templ.isValid,
                        "Cached reader GUID template retained");
            }
        }
    }

} // namespace TestFunc
#endif
