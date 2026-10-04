//------------ kuvbur 2022 ------------
#ifdef TESTING
    #include "ACAPinc.h"

    #include "api_headers/APIEnvir.h"

    #include "Helpers.hpp"
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
        };

        const Case cases[] = {{"normal", 1, 3, 2, 0.5, 0, false, 3, 1, 3},
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
                              {"vertical wall", 1, 3, 2, 0.5, 2, false, 3, 1, 3}};
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
                source.height = 10;
                source.base_guid = guid;
                source.floorInd = -1;
                source.material.smaterial = "source material";
                source.favorite.name = "source favorite";
                ParamValueComposite layer;
                layer.structype = 0;
                layer.val = "source layer";
                source.base_composite.Push (layer);
                Roombook::OtdOpening opening;
                opening.objLoc = 5;
                opening.width = c.width;
                opening.zBottom = c.bottom;
                opening.height = c.height;
                opening.base_reveal_width = c.depth;
                opening.has_reveal = hadReveal;
                source.openings.Push (opening);
                Geometry::Vector2<double> perpendicular = {0, 1};
                if (c.orientation == 2)
                    perpendicular = {-1, 0};
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
                double bottom = 0, down = 0, main = 0, up = 0, height = 20;
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
                DBtest (opening.objLoc == 5 && opening.width == c.width && opening.height == c.height &&
                            opening.zBottom == c.bottom && opening.base_reveal_width == c.depth,
                        label + " opening dimensions unchanged");
                DBtest (source.height == 10 && source.base_guid == guid && source.floorInd == -1 &&
                            source.material.smaterial == "source material" &&
                            source.favorite.name == "source favorite" && source.base_composite.GetSize () == 1 &&
                            source.openings.GetSize () == 1,
                        label + " source preserved");
                DBtest (slabs.GetSize () == 1 && slabs.Get (0).height == 99, label + " slab output untouched");
                DBtest (bottom == 0 && down == 0 && main == 0 && up == 0 && height == 20,
                        label + " band inputs unchanged");
                if (walls.GetSize () != c.count)
                    continue;
                for (UInt32 i = 0; i < walls.GetSize (); ++i) {
                    const Roombook::OtdWall &wall = walls.Get (i);
                    bool top = c.zeroPerp || i == 2;
                    double left = 5 - c.width / 2, right = 5 + c.width / 2;
                    const auto pointAt = [&] (double distance) -> Point2D {
                        if (c.orientation == 1)
                            return {20 - distance, 20};
                        if (c.orientation == 2)
                            return {10, 20 + distance};
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
    }

} // namespace TestFunc
#endif
