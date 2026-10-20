#include "ACAPinc.h"

#include "tests/TestRoombookGeometry.hpp"

#if defined(TESTING) && defined(ServerMainVers_2500)

    #include <cmath>

    #include "CommonFunction.hpp"
    #include "roombook/Roombook.hpp"

GS::ObjectState RunFinishReuseChecks () {
    using namespace Roombook;
    GS::Array<GS::ObjectState> checks;
    Int32 failed = 0;
    const auto check = [&] (bool passed, const char *name) {
        checks.Push (GS::ObjectState ("name", name, "passed", passed));
        if (!passed)
            ++failed;
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
    check (TakeExistingWallFinish (pool, wall, existing) == first, "stable choice ignores insertion order");
    check (!pool.ContainsKey (first) && pool.ContainsKey (second), "exactly selected GUID is consumed");
    check (TakeExistingWallFinish (pool, wall, existing) == second, "second piece uses second GUID");
    check (TakeExistingWallFinish (pool, wall, existing) == APINULLGuid, "exhausted pool requests creation");
    pool.Add (third, true);
    check (TakeExistingWallFinish (pool, wall, existing) == APINULLGuid && pool.ContainsKey (third),
           "missing snapshot is not consumed");
    pool.Clear ();
    pool.Add (first, false);
    check (TakeExistingWallFinish (pool, wall, existing) == APINULLGuid, "unavailable candidate is ignored");
    pool.Set (first, true);
    API_Element object = {};
    SetElemTypeID (object, API_ObjectID);
    object.object.libInd = 10;
    check (TakeExistingWallFinish (pool, object, existing) == APINULLGuid, "native types are not interchangeable");
    existing.Set (first, object);
    API_Element otherObject = object;
    otherObject.object.libInd = 11;
    check (TakeExistingWallFinish (pool, otherObject, existing) == APINULLGuid,
           "different GDL library part is not reused");
    check (TakeExistingWallFinish (pool, object, existing) == first, "compatible object can be reused");
    pool.Add (first, true);
    API_Element polygonWall = wall;
    polygonWall.wall.type = APIWtyp_Poly;
    existing.Set (first, polygonWall);
    check (TakeExistingWallFinish (pool, wall, existing) == APINULLGuid, "polygon wall cannot become normal wall");
    API_Element profileWall = wall;
    profileWall.wall.modelElemStructureType = API_ProfileStructure;
    existing.Set (first, profileWall);
    check (TakeExistingWallFinish (pool, wall, existing) == APINULLGuid, "profile wall uses replacement route");
    existing.Set (first, wall);
    check (TakeExistingWallFinish (pool, polygonWall, existing) == APINULLGuid,
           "polygon target does not consume normal wall");
    API_Element beam = {};
    SetElemTypeID (beam, API_BeamID);
    beam.beam.nSegments = 1;
    existing.Set (first, beam);
    API_Element multiBeam = beam;
    multiBeam.beam.nSegments = 2;
    check (TakeExistingWallFinish (pool, multiBeam, existing) == APINULLGuid,
           "multisegment target is not silently simplified");
    existing.Set (first, multiBeam);
    check (TakeExistingWallFinish (pool, beam, existing) == APINULLGuid, "multisegment candidate is not reused");
    existing.Set (first, beam);
    check (TakeExistingWallFinish (pool, beam, existing) == first, "single segment beam can be reused");

    if (!IsTestProjectOpen ()) {
        GS::ObjectState response ("status", "TestProjectRequired", "failed", failed, "nativeExecuted", false);
        response.Add ("checks", checks);
        return response;
    }

    const Stories stories = GetStories ();
    API_Attribute material = {};
    material.header.typeID = API_MaterialID;
    material.header.index = 1;
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
                  source.favorite.type = API_WallID;
                  source.begC = {10000, 10000};
                  source.endC = {10006, 10000};
                  source.height = 3;
                  source.material.material = 1;
                  source.slant_angle = 90 * DEGRAD;
                  UnicElementByType links;
                  OtdWall initial = source;
                  OtdWall_Draw (stories, initial, links);
                  check (initial.draw_error == NoError && initial.otd_guid != APINULLGuid, "native wall created");
                  if (initial.draw_error != NoError || initial.otd_guid == APINULLGuid)
                      return APIERR_BADPARS;
                  nativeWall = initial.otd_guid;
                  OtdWall changed = source;
                  changed.otd_guid = nativeWall;
                  changed.height = 2.5;
                  changed.endC.x += 1;
                  OtdWall_Draw (stories, changed, links);
                  check (changed.draw_error == NoError && changed.otd_guid == nativeWall, "native wall GUID preserved");
                  if (changed.draw_error != NoError)
                      return changed.draw_error;
                  API_Element actual = {};
                  actual.header.guid = nativeWall;
                  const GSErrCode readErr = ACAPI_Element_Get (&actual);
                  check (readErr == NoError && std::abs (actual.wall.height - 2.5) < 1e-6 &&
                             std::abs (actual.wall.endC.x - changed.endC.x) < 1e-6,
                         "native wall geometry actually changed");
                  OtdWall invalid = changed;
                  invalid.height = 0;
                  OtdWall_Draw (stories, invalid, links);
                  check (invalid.draw_error != NoError && invalid.otd_guid == APINULLGuid,
                         "invalid update cannot publish stale GUID");

                  OtdWall beamSource = source;
                  beamSource.surface_geometry = false;
                  beamSource.favorite.type = API_BeamID;
                  beamSource.width = 0.2;
                  beamSource.height = 0.1;
                  OtdWall initialBeam = beamSource;
                  OtdWall_Draw (stories, initialBeam, links);
                  check (initialBeam.draw_error == NoError && initialBeam.otd_guid != APINULLGuid,
                         "native beam created");
                  if (initialBeam.draw_error != NoError || initialBeam.otd_guid == APINULLGuid)
                      return APIERR_BADPARS;
                  nativeBeam = initialBeam.otd_guid;
                  OtdWall changedBeam = beamSource;
                  changedBeam.otd_guid = nativeBeam;
                  changedBeam.endC.y += 1;
                  changedBeam.width = 0.3;
                  OtdWall_Draw (stories, changedBeam, links);
                  check (changedBeam.draw_error == NoError && changedBeam.otd_guid == nativeBeam,
                         "native beam GUID preserved");
                  if (changedBeam.draw_error != NoError)
                      return changedBeam.draw_error;
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
                  check (cutWall.draw_error == NoError && cutWall.otd_guid == nativeWall, "two native windows created");
                  if (cutWall.draw_error != NoError)
                      return cutWall.draw_error;
                  for (const auto &opening : cutWall.openings) {
                      const GSErrCode classErr = ACAPI_Element_AddClassificationItem (opening.otd_guid, openingClass);
                      check (classErr == NoError, "window marked as managed");
                      if (classErr != NoError)
                          return classErr;
                      removedWindows.Push (opening.otd_guid);
                  }
                  OtdWall repeatedCuts = changed;
                  repeatedCuts.openings = cutWall.openings;
                  OtdWall_Draw (stories, repeatedCuts, links);
                  check (repeatedCuts.draw_error == NoError && repeatedCuts.otd_guid == nativeWall,
                         "repeat with cuts preserves wall GUID");
                  if (repeatedCuts.draw_error != NoError)
                      return repeatedCuts.draw_error;
                  for (const auto &opening : repeatedCuts.openings) {
                      check (opening.otd_guid != APINULLGuid && !removedWindows.Contains (opening.otd_guid),
                             "repeat replaces managed window GUID");
                      const GSErrCode classErr = ACAPI_Element_AddClassificationItem (opening.otd_guid, openingClass);
                      check (classErr == NoError, "replacement window marked as managed");
                      if (classErr != NoError)
                          return classErr;
                  }
                  const auto repeatedChildren = ReadConnectedElements (nativeWall, API_WindowID, APIFilt_None);
                  check (repeatedChildren.error == NoError && repeatedChildren.items.GetSize () == 2,
                         "repeat retains exactly two hosted windows without accumulation");
                  OtdWall withoutCuts = changed;
                  withoutCuts.openings.Clear ();
                  OtdWall_Draw (stories, withoutCuts, links);
                  check (withoutCuts.draw_error == NoError && withoutCuts.otd_guid == nativeWall,
                         "zero new windows still updates same wall");
                  if (withoutCuts.draw_error != NoError)
                      return withoutCuts.draw_error;
                  const auto children = ReadConnectedElements (nativeWall, API_WindowID, APIFilt_None);
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
    GS::ObjectState response (
        "status", failed == 0 ? "Passed" : "Failed", "failed", failed, "nativeExecuted", materialErr == NoError);
    response.Add ("checks", checks);
    return response;
}
#endif
