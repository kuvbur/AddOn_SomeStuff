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

} // namespace TestFunc
#endif
