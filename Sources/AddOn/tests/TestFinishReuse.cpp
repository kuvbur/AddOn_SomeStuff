#include "ACAPinc.h"

#include "tests/TestFunc.hpp"

#if defined(TESTING)

    #include <cmath>
    #include <cstdlib>
    #include <cstring>

    #include "CommonFunction.hpp"
    #include "OnExit.hpp"
    #include "roombook/Roombook.hpp"
    #include "tests/TestKit.hpp"

    #ifdef ServerMainVers_2700
        #define SMSTF_INDEX(i) ACAPI_CreateAttributeIndex (i)
    #else
        #define SMSTF_INDEX(i) (i)
    #endif

    // Тесты целиком используют типы Roombook, объявленные только для AC >= 24.
    #if defined(ServerMainVers_2400)

namespace TestFunc {
    void TestFinishReuse () {
        using namespace Roombook;
        const auto check = [&] (bool passed, const char *name) {
            DBtest (passed, "Finish reuse: " + GS::UniString (name));
        };
        const API_Guid first = APIGuidFromString ("00000001-0000-0000-0000-000000000000");
        const API_Guid second = APIGuidFromString ("00000002-0000-0000-0000-000000000000");
        const API_Guid third = APIGuidFromString ("00000003-0000-0000-0000-000000000000");
        API_Element wall = {};
        SetElemTypeID (wall, API_WallID);
        wall.wall.type = APIWtyp_Normal;
        wall.wall.modelElemStructureType = API_BasicStructure;
        GS::HashTable<API_Guid, API_Element> existing;
        existing.Add (first, wall);
        existing.Add (second, wall);
        UnicGuid pool;
        pool.Add (second, true);
        pool.Add (first, true);
        check (OtdWall_TakeExistingFinish (pool, wall, existing) == first, "stable choice ignores insertion order");
        check (!pool.ContainsKey (first) && pool.ContainsKey (second), "exactly selected GUID is consumed");
        check (OtdWall_TakeExistingFinish (pool, wall, existing) == second, "second piece uses second GUID");
        check (OtdWall_TakeExistingFinish (pool, wall, existing) == APINULLGuid, "exhausted pool requests creation");
        pool.Add (third, true);
        check (OtdWall_TakeExistingFinish (pool, wall, existing) == APINULLGuid && pool.ContainsKey (third),
               "missing snapshot is not consumed");
        pool.Clear ();
        pool.Add (first, false);
        check (OtdWall_TakeExistingFinish (pool, wall, existing) == APINULLGuid, "unavailable candidate is ignored");
        pool.Set (first, true);
        API_Element object = {};
        SetElemTypeID (object, API_ObjectID);
        object.object.libInd = 10;
        check (OtdWall_TakeExistingFinish (pool, object, existing) == APINULLGuid,
               "native types are not interchangeable");
        existing.Set (first, object);
        API_Element otherObject = object;
        otherObject.object.libInd = 11;
        check (OtdWall_TakeExistingFinish (pool, otherObject, existing) == APINULLGuid,
               "different GDL library part is not reused");
        check (OtdWall_TakeExistingFinish (pool, object, existing) == first, "compatible object can be reused");
        pool.Add (first, true);
        API_Element polygonWall = wall;
        polygonWall.wall.type = APIWtyp_Poly;
        existing.Set (first, polygonWall);
        check (OtdWall_TakeExistingFinish (pool, wall, existing) == APINULLGuid,
               "polygon wall cannot become normal wall");
        API_Element profileWall = wall;
        profileWall.wall.modelElemStructureType = API_ProfileStructure;
        existing.Set (first, profileWall);
        check (OtdWall_TakeExistingFinish (pool, wall, existing) == APINULLGuid, "profile wall uses replacement route");
        existing.Set (first, wall);
        check (OtdWall_TakeExistingFinish (pool, polygonWall, existing) == APINULLGuid,
               "polygon target does not consume normal wall");
        API_Element beam = {};
        SetElemTypeID (beam, API_BeamID);
        beam.beam.nSegments = 1;
        beam.beam.nSchemes = 1;
        beam.beam.nCuts = 2;
        existing.Set (first, beam);
        API_Element multiBeam = beam;
        multiBeam.beam.nSegments = 2;
        check (OtdWall_TakeExistingFinish (pool, multiBeam, existing) == APINULLGuid,
               "multisegment target is not silently simplified");
        existing.Set (first, multiBeam);
        check (OtdWall_TakeExistingFinish (pool, beam, existing) == APINULLGuid,
               "multisegment candidate is not reused");
        existing.Set (first, beam);
        check (OtdWall_TakeExistingFinish (pool, beam, existing) == first, "single segment beam can be reused");
        pool.Add (first, true);
        API_Element invalidBeam = beam;
        invalidBeam.beam.nSchemes = 2;
        existing.Set (first, invalidBeam);
        check (OtdWall_TakeExistingFinish (pool, beam, existing) == APINULLGuid,
               "incompatible segment scheme is not consumed");
        pool.Add (first, true);
        invalidBeam = beam;
        invalidBeam.beam.nCuts = 1;
        existing.Set (first, invalidBeam);
        check (OtdWall_TakeExistingFinish (pool, beam, existing) == APINULLGuid,
               "incompatible segment cuts are not consumed");

        API_Element slab = {};
        SetElemTypeID (slab, API_SlabID);
        existing.Set (first, slab);
        check (OtdWall_TakeExistingFinish (pool, slab, existing) == first, "surface slab can be reused");
        pool.Add (first, true);
        check (OtdWall_TakeExistingFinish (pool, object, existing) == APINULLGuid,
               "slab cannot be passed to polygon object writer");
        API_Element roof = {};
        SetElemTypeID (roof, API_RoofID);
        roof.roof.roofClass = API_PlaneRoofID;
        existing.Set (first, roof);
        check (OtdWall_TakeExistingFinish (pool, slab, existing) == APINULLGuid,
               "mesh roof cannot be passed to slab writer");
        API_Element polyRoof = roof;
        polyRoof.roof.roofClass = API_PolyRoofID;
        existing.Set (first, polyRoof);
        check (OtdWall_TakeExistingFinish (pool, roof, existing) == APINULLGuid,
               "multi plane roof is not converted for reuse");
        existing.Set (first, roof);
        check (OtdWall_TakeExistingFinish (pool, polyRoof, existing) == APINULLGuid,
               "multi plane target is not supported");
        check (OtdWall_TakeExistingFinish (pool, roof, existing) == first, "single plane roof can be reused");
    }

    void TestFinishReuseNative () {
        const char *filter = std::getenv ("SMSTF_TEST");
        if (filter == nullptr || std::strstr (filter, "TestFinishReuseNative") == nullptr)
            DBskip ("Native fixture requires an explicit suite name in SMSTF_TEST");
        using namespace Roombook;
        if (!IsTestProjectOpen ())
            DBskip ("Native finish reuse checks require an explicit test project");
        const auto check = [] (bool passed, const char *name) {
            DBtest (passed, "Finish reuse: " + GS::UniString (name));
        };

        const Stories stories = GetStories ();
        API_Attribute material = {};
        material.header.typeID = API_MaterialID;
        material.header.index = SMSTF_INDEX (1);
        const GSErrCode materialErr = ACAPI_Attribute_Get (&material);
        check (materialErr == NoError, "native fixture material exists");
        API_Guid nativeWall = APINULLGuid;
        API_Guid nativeBeam = APINULLGuid;
        GS::Array<API_Guid> removedWindows;
        const GSErrCode nativeErr =
            materialErr != NoError
                ? materialErr
                : ACAPI_CallUndoableCommand ("Finish reuse native checks", [&] () -> GSErrCode {
                      OtdWall source;
                      source.surface_geometry = true;
                      source.draw.favorite.type = API_WallID;
                      source.geom.seg.begC = {10000, 10000};
                      source.geom.seg.endC = {10006, 10000};
                      source.geom.height = 3;
                      source.draw.material.material = 1;
                      source.slant_angle = 90 * DEGRAD;
                      UnicElementByType links;
                      OtdWall initial = source;
                      OtdWall_Draw (stories, initial, links);
                      check (initial.draw.draw_error == NoError && initial.draw.otd_guid != APINULLGuid,
                             "native wall created");
                      if (initial.draw.draw_error != NoError || initial.draw.otd_guid == APINULLGuid)
                          return APIERR_BADPARS;
                      nativeWall = initial.draw.otd_guid;
                      OtdWall changed = source;
                      changed.draw.otd_guid = nativeWall;
                      changed.geom.height = 2.5;
                      changed.geom.seg.endC.x += 1;
                      OtdWall_Draw (stories, changed, links);
                      check (changed.draw.draw_error == NoError && changed.draw.otd_guid == nativeWall,
                             "native wall GUID preserved");
                      if (changed.draw.draw_error != NoError)
                          return changed.draw.draw_error;
                      API_Element actual = {};
                      actual.header.guid = nativeWall;
                      const GSErrCode readErr = ACAPI_Element_Get (&actual);
                      check (readErr == NoError && std::abs (actual.wall.height - 2.5) < 1e-6 &&
                                 std::abs (actual.wall.endC.x - changed.geom.seg.endC.x) < 1e-6,
                             "native wall geometry actually changed");
                      OtdWall invalid = changed;
                      invalid.geom.height = 0;
                      OtdWall_Draw (stories, invalid, links);
                      check (invalid.draw.draw_error != NoError && invalid.draw.otd_guid == APINULLGuid,
                             "invalid update cannot publish stale GUID");

                      OtdWall beamSource = source;
                      beamSource.surface_geometry = false;
                      beamSource.draw.favorite.type = API_BeamID;
                      beamSource.geom.width = 0.2;
                      beamSource.geom.height = 0.1;
                      OtdWall initialBeam = beamSource;
                      OtdWall_Draw (stories, initialBeam, links);
                      check (initialBeam.draw.draw_error == NoError && initialBeam.draw.otd_guid != APINULLGuid,
                             "native beam created");
                      if (initialBeam.draw.draw_error != NoError || initialBeam.draw.otd_guid == APINULLGuid)
                          return APIERR_BADPARS;
                      nativeBeam = initialBeam.draw.otd_guid;
                      OtdWall changedBeam = beamSource;
                      changedBeam.draw.otd_guid = nativeBeam;
                      changedBeam.geom.seg.endC.y += 1;
                      changedBeam.geom.width = 0.3;
                      OtdWall_Draw (stories, changedBeam, links);
                      check (changedBeam.draw.draw_error == NoError && changedBeam.draw.otd_guid == nativeBeam,
                             "native beam GUID preserved");
                      if (changedBeam.draw.draw_error != NoError)
                          return changedBeam.draw.draw_error;
                      API_ElementMemo beamMemo = {};
                      const GS::OnExit memoGuard ([&] () { ACAPI_DisposeElemMemoHdls (&beamMemo); });
                      const GSErrCode memoErr = ACAPI_Element_GetMemo (nativeBeam, &beamMemo, APIMemoMask_BeamSegment);
                      check (memoErr == NoError && beamMemo.beamSegments != nullptr &&
                                 std::abs (beamMemo.beamSegments[0].assemblySegmentData.nominalWidth - 0.3) < 1e-6,
                             "native beam section actually changed");

                      ClassificationFunc::ClassificationDict finclass;
                      UnicGuid classGuids;
                      Class_FindFinClass (finclass, classGuids);
                      API_Guid openingClass = Class_GetClassGuid (NoSet, finclass);
                      if (openingClass == APINULLGuid)
                          openingClass = Class_GetClassGuid (Wall_Main, finclass);
                      check (openingClass != APINULLGuid, "native opening fixture has managed classification");
                      if (openingClass == APINULLGuid)
                          return APIERR_CANCEL;
                      OtdWall cutWall = changed;
                      cutWall.openings.Clear ();
                      for (double position : {1.5, 4.5}) {
                          OtdOpening opening;
                          opening.width = 0.8;
                          opening.height = 1;
                          opening.objLoc = position;
                          opening.lower = 0.5;
                          opening.surface_geometry = true;
                          cutWall.openings.Push (opening);
                      }
                      OtdWall_Draw (stories, cutWall, links);
                      check (cutWall.draw.draw_error == NoError && cutWall.draw.otd_guid == nativeWall,
                             "two native windows created");
                      if (cutWall.draw.draw_error != NoError)
                          return cutWall.draw.draw_error;
                      for (const auto &opening : cutWall.openings) {
                          const GSErrCode classErr =
                              ACAPI_Element_AddClassificationItem (opening.otd_guid, openingClass);
                          check (classErr == NoError, "window marked as managed");
                          if (classErr != NoError)
                              return classErr;
                          removedWindows.Push (opening.otd_guid);
                      }
                      OtdWall repeatedCuts = changed;
                      repeatedCuts.openings = cutWall.openings;
                      OtdWall_Draw (stories, repeatedCuts, links);
                      check (repeatedCuts.draw.draw_error == NoError && repeatedCuts.draw.otd_guid == nativeWall,
                             "repeat with cuts preserves wall GUID");
                      if (repeatedCuts.draw.draw_error != NoError)
                          return repeatedCuts.draw.draw_error;
                      for (const auto &opening : repeatedCuts.openings) {
                          check (opening.otd_guid != APINULLGuid && !removedWindows.Contains (opening.otd_guid),
                                 "repeat replaces managed window GUID");
                          const GSErrCode classErr =
                              ACAPI_Element_AddClassificationItem (opening.otd_guid, openingClass);
                          check (classErr == NoError, "replacement window marked as managed");
                          if (classErr != NoError)
                              return classErr;
                      }
                      const auto repeatedChildren = ConnectedElements_Read (nativeWall, API_WindowID, APIFilt_None);
                      check (repeatedChildren.error == NoError && repeatedChildren.items.GetSize () == 2,
                             "repeat retains exactly two hosted windows without accumulation");
                      OtdWall withoutCuts = changed;
                      withoutCuts.openings.Clear ();
                      OtdWall_Draw (stories, withoutCuts, links);
                      check (withoutCuts.draw.draw_error == NoError && withoutCuts.draw.otd_guid == nativeWall,
                             "zero new windows still updates same wall");
                      if (withoutCuts.draw.draw_error != NoError)
                          return withoutCuts.draw.draw_error;
                      const auto children = ConnectedElements_Read (nativeWall, API_WindowID, APIFilt_None);
                      check (children.error == NoError && children.items.IsEmpty (),
                             "old windows removed with empty new set");
                      return APIERR_CANCEL; // Все временные элементы откатываются, исходная модель не сохраняется.
                  });
        check (nativeErr == APIERR_CANCEL, "native fixture undo scope returns cancellation");
        for (const API_Guid guid : {nativeWall, nativeBeam}) {
            if (guid == APINULLGuid)
                continue;
            API_Elem_Head head = {};
            head.guid = guid;
            const GSErrCode err = ACAPI_Element_GetHeader (&head);
            check (err == APIERR_DELETED || err == APIERR_BADID, "native fixture rollback removed temporary element");
        }
    }

} // namespace TestFunc

    #endif // ServerMainVers_2400

#endif
