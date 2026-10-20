#include "ACAPinc.h"

#include "tests/TestRoombookGeometry.hpp"

#if defined(TESTING) && defined(ServerMainVers_2500)

    #include <algorithm>
    #include <cmath>
    #include <limits>
    #include <vector>

    #include "CommonFunction.hpp"
    #include "roombook/Roombook.hpp"
    #include "roombook/RoomSurfaceFinish.hpp"

namespace Roombook {
    extern MatarialToFavoriteDict favdict;
}

GS::ObjectState RunMeshFloorNativeChecks () {
    using namespace Roombook;
    if (!IsTestProjectOpen ())
        return GS::ObjectState ("status", "TestProjectRequired");
    GS::Array<GS::ObjectState> checks;
    GS::Array<GS::ObjectState> geometry;
    const char *caseName = "settings";
    Int32 failed = 0;
    const auto check = [&] (bool passed, const char *name) {
        checks.Push (GS::ObjectState ("case", caseName, "name", name, "passed", passed));
        if (!passed)
            ++failed;
        DBtest (passed, "Mesh floor native: " + GS::UniString (caseName) + ": " + GS::UniString (name));
    };

    {
        caseName = "favorite missing request read-only layer";
        // Совпавшее имя не даёт права переиспользовать или удалить чужой объект.
        const GS::UniString favoriteName = "smstf test readonly slab fixture";
        GS::UniString layerName = "SomeStuff test readonly owned layer";
        const short protectedBits = APILay_Hidden | APILay_Locked;
        const MatarialToFavoriteDict savedFavdict = Roombook::favdict;
        API_Attribute ownedLayer = {};
        bool layerOwned = false;
        bool favoriteOwned = false;

        GS::OnExit cleanup ([&] () {
            bool favoriteRemoved = !favoriteOwned;
            if (favoriteOwned) {
                const GSErrCode deleteError = ACAPI_Favorite_Delete (favoriteName);
                check (deleteError == NoError, "cleanup owned Favorite Delete succeeds");
                API_Favorite deletedFavorite (favoriteName);
                const GSErrCode getError = ACAPI_Favorite_Get (&deletedFavorite);
                check (getError == APIERR_BADNAME, "cleanup owned Favorite is absent by name");
                favoriteRemoved = deleteError == NoError && getError == APIERR_BADNAME;
            }
            // При неудачном удалении избранного сохраняем его слой, а не оставляем битую ссылку.
            if (layerOwned && favoriteRemoved) {
                API_Attribute current = {};
                current.header.typeID = API_LayerID;
                current.header.index = ownedLayer.header.index;
                const GSErrCode readError = ACAPI_Attribute_Get (&current);
                const bool identityMatches =
                    readError == NoError && current.header.guid == ownedLayer.header.guid && current.header.index != 1;
                check (identityMatches, "cleanup verifies owned layer GUID and nonprotected index");
                if (identityMatches) {
                    const GSErrCode deleteError = ACAPI_Attribute_Delete (&current.header);
                    check (deleteError == NoError, "cleanup deletes only the new owned layer");
                    API_Attr_Head lookup = {};
                    lookup.typeID = API_LayerID;
                    lookup.uniStringNamePtr = &layerName;
                    check (ACAPI_Attribute_Search (&lookup) == APIERR_BADNAME, "cleanup owned layer absent by name");
                }
            } else if (layerOwned) {
                check (false, "cleanup blocked: Favorite retained; owned layer deliberately retained");
            }
            // GetDict заново потерял бы старые cached composite/is_composite_read.
            Roombook::favdict = savedFavdict;
            check (Roombook::favdict.GetSize () == savedFavdict.GetSize (),
                   "restored Favorite dictionary retains original entry count");
        });

        const auto runFavoriteLayerFixture = [&] () {
            API_Favorite absentFavorite (favoriteName);
            GSErrCode err = ACAPI_Favorite_Get (&absentFavorite);
            check (err == APIERR_BADNAME, "setup reserved Favorite name is absent (not an API error)");
            if (err != APIERR_BADNAME)
                return;
            API_Attr_Head absentLayer = {};
            absentLayer.typeID = API_LayerID;
            absentLayer.uniStringNamePtr = &layerName;
            err = ACAPI_Attribute_Search (&absentLayer);
            check (err == APIERR_BADNAME, "setup reserved layer name is absent (never reuse user layer)");
            if (err != APIERR_BADNAME)
                return;

            API_Favorite fixture (favoriteName);
            SetElemTypeID (fixture.element, API_SlabID);
            err = ACAPI_Element_GetDefaults (&fixture.element, nullptr);
            check (err == NoError, "setup real Slab defaults read successfully");
            if (err != NoError)
                return;
            ownedLayer.header.typeID = API_LayerID;
            ownedLayer.header.flags = protectedBits;
            ownedLayer.header.uniStringNamePtr = &layerName;
            ownedLayer.layer.conClassId = 0;
            err = ACAPI_Attribute_Create (&ownedLayer, nullptr);
            check (err == NoError, "setup creates NEW hidden locked layer");
            if (err != NoError)
                return;
            layerOwned = ownedLayer.header.index != 1;
            check (layerOwned, "setup new layer is not protected index one");
            if (!layerOwned)
                return;
            API_Attribute createdReadback = {};
            createdReadback.header.typeID = API_LayerID;
            createdReadback.header.index = ownedLayer.header.index;
            err = ACAPI_Attribute_Get (&createdReadback);
            check (err == NoError, "setup newly created layer read-back succeeds");
            if (err != NoError)
                return;
            ownedLayer = createdReadback;
            ownedLayer.header.uniStringNamePtr = nullptr;
            check (ownedLayer.header.guid != APINULLGuid, "setup layer has actual persistent GUID");
            if (ownedLayer.header.guid == APINULLGuid)
                return;
            fixture.element.header.layer = ownedLayer.header.index;
            fixture.element.header.guid = APINULLGuid;
            err = ACAPI_Favorite_Create (fixture);
            check (err == NoError, "setup real Slab Favorite Create succeeds");
            if (err != NoError)
                return;
            favoriteOwned = true;

            API_Favorite loaded (favoriteName);
            err = ACAPI_Favorite_Get (&loaded);
            const bool actualSettings = err == NoError && GetElemTypeID (loaded.element.header) == API_SlabID &&
                                        loaded.element.header.layer == ownedLayer.header.index;
            check (actualSettings, "setup actual Favorite Get proves Slab and owned layer");
            if (!actualSettings)
                return;
            // SUT читает глобальный словарь; локальный GetDict не вооружил бы проверку.
            const bool dictionaryLoaded = Favorite_GetDict (Roombook::favdict);
            check (dictionaryLoaded, "setup public Favorite_GetDict loads actual global dictionary");
            auto *entry = Roombook::favdict.GetPtr (favoriteName);
            const bool selected = dictionaryLoaded && entry != nullptr && entry->name == favoriteName &&
                                  entry->type == API_SlabID && !entry->is_composite_read && entry->composite.IsEmpty ();
            check (selected, "setup selected uncached Slab Favorite exists in actual dictionary");
            if (!selected)
                return;

            API_Attribute before = {};
            before.header.typeID = API_LayerID;
            before.header.index = ownedLayer.header.index;
            err = ACAPI_Attribute_Get (&before);
            const bool armed = err == NoError && before.header.guid == ownedLayer.header.guid &&
                               (before.header.flags & protectedBits) == protectedBits;
            check (armed, "setup both hidden and locked bits set immediately before SUT");
            if (!armed)
                return;
            ParamValue missingRequest = {};
            missingRequest.rawName = "{@material:smstf_test_missing_request}";
            ParamDictValue requests;
            check (!requests.ContainsKey (missingRequest.rawName), "request key demonstrably absent");
            Favorite_ReadComposite (missingRequest, requests, favoriteName);

            API_Attribute after = {};
            after.header.typeID = API_LayerID;
            after.header.index = ownedLayer.header.index;
            err = ACAPI_Attribute_Get (&after);
            check (err == NoError, "post-SUT owned layer read-back succeeds");
            if (err == NoError) {
                check (after.header.guid == before.header.guid, "post-SUT same actual owned layer");
                check ((after.header.flags & protectedBits) == (before.header.flags & protectedBits),
                       "missing material request must not unhide or unlock Favorite layer");
                check (after.header.flags == before.header.flags,
                       "read-only composite retrieval preserves entire layer flags");
            }
            entry = Roombook::favdict.GetPtr (favoriteName);
            check (entry != nullptr && entry->is_composite_read,
                   "legacy early is_composite_read timing preserved on missing request");
            check (entry != nullptr && entry->composite.IsEmpty (), "missing request publishes no material composite");
            check (requests.IsEmpty (), "missing request dictionary remains empty");

            const auto runPresentInvalidTemplate = [&] () {
                caseName = "favorite present invalid empty material template";
                ParamValue invalidRequest = {};
                invalidRequest.rawName = "{@material:layers,20}";
                invalidRequest.fromMaterial = true;
                invalidRequest.composite_pen = 20;
                invalidRequest.isValid = false;
                invalidRequest.val.uniStringValue.Clear ();
                // Настоящий запрос слоёв, а не неизвестный или игнорируемый ключ.
                // Пустой шаблон не проходит ветку обработки текста в ReadMaterial.
                ParamDictValue invalidRequests;
                invalidRequests.Put (invalidRequest.rawName, invalidRequest);

                // Независимо доказываем чтение реальных слоёв по этому ключу при отказе обработки шаблона.
                ParamDictValue probeRequests = invalidRequests;
                ParamDictComposite probeComposites;
                bool probeQuantities = false;
                const bool probeRead =
                    ParamHelpers::ReadMaterial (loaded.element, probeRequests, probeComposites, probeQuantities);
                const ParamValue *probeRequest = probeRequests.GetPtr (invalidRequest.rawName);
                const ParamComposite *probeComposite = probeComposites.GetPtr (invalidRequest.rawName);
                const bool invalidArmed = !probeRead && probeRequest != nullptr && probeRequest->fromMaterial &&
                                          !probeRequest->isValid && probeRequest->val.uniStringValue.IsEmpty () &&
                                          probeComposite != nullptr && probeComposite->templatestring.IsEmpty () &&
                                          !probeComposite->composite.IsEmpty ();
                check (invalidArmed, "present real layers request has raw layers but invalid empty template");
                if (!invalidArmed)
                    return; // Отказ чтения Slab не доказывал бы отказ обработки шаблона.

                // Сбрасываем только свою запись; внешний cleanup восстанавливает исходный словарь.
                entry = Roombook::favdict.GetPtr (favoriteName);
                check (entry != nullptr && entry->name == favoriteName,
                       "invalid template setup selects only owned Favorite entry");
                if (entry == nullptr || entry->name != favoriteName)
                    return;
                entry->is_composite_read = false;
                entry->composite.Clear ();

                // Вновь задаём флаги независимо от результата предыдущего сценария.
                API_Attribute invalidLayer = before;
                err = ACAPI_Attribute_Modify (&invalidLayer, nullptr);
                check (err == NoError, "invalid template setup independently rearms owned layer");
                if (err != NoError)
                    return;
                API_Attribute invalidBefore = {};
                invalidBefore.header.typeID = API_LayerID;
                invalidBefore.header.index = ownedLayer.header.index;
                err = ACAPI_Attribute_Get (&invalidBefore);
                const bool invalidLayerArmed = err == NoError && invalidBefore.header.guid == ownedLayer.header.guid &&
                                               (invalidBefore.header.flags & protectedBits) == protectedBits;
                check (invalidLayerArmed, "invalid template owned layer hidden locked immediately before SUT");
                if (!invalidLayerArmed)
                    return;
                const ParamValue *present = invalidRequests.GetPtr (invalidRequest.rawName);
                const bool presentArmed = present != nullptr && present->fromMaterial &&
                                          present->rawName == invalidRequest.rawName && !present->isValid &&
                                          present->val.uniStringValue.IsEmpty ();
                check (presentArmed, "invalid material request key is present immediately before SUT");
                if (!presentArmed)
                    return;

                Favorite_ReadComposite (invalidRequest, invalidRequests, favoriteName);

                const ParamValue *readback = invalidRequests.GetPtr (invalidRequest.rawName);
                check (readback != nullptr && readback->fromMaterial && readback->rawName == invalidRequest.rawName &&
                           !readback->isValid && readback->val.uniStringValue.IsEmpty (),
                       "present material request read-back remains invalid with empty template");
                entry = Roombook::favdict.GetPtr (favoriteName);
                check (entry != nullptr && entry->is_composite_read,
                       "legacy early attempted-read flag retained on invalid template");
                check (entry != nullptr && entry->composite.IsEmpty (),
                       "invalid template publishes no Favorite composite");
                API_Attribute invalidAfter = {};
                invalidAfter.header.typeID = API_LayerID;
                invalidAfter.header.index = ownedLayer.header.index;
                err = ACAPI_Attribute_Get (&invalidAfter);
                check (err == NoError && invalidAfter.header.guid == invalidBefore.header.guid,
                       "invalid template post-SUT reads same owned layer");
                if (err == NoError) {
                    check ((invalidAfter.header.flags & protectedBits) == protectedBits,
                           "invalid template must not unhide or unlock Favorite layer");
                    check (invalidAfter.header.flags == invalidBefore.header.flags,
                           "invalid template preserves entire owned layer flags");
                }
            };
            runPresentInvalidTemplate ();
            caseName = "favorite missing request read-only layer";

            // Отдельно вооружаем прежний writer-контракт, независимо от результата SUT.
            API_Attribute writerLayer = before;
            err = ACAPI_Attribute_Modify (&writerLayer, nullptr);
            check (err == NoError, "writer setup rearms hidden and locked flags on owned layer");
            if (err != NoError)
                return;
            API_Attribute writerBefore = {};
            writerBefore.header.typeID = API_LayerID;
            writerBefore.header.index = ownedLayer.header.index;
            err = ACAPI_Attribute_Get (&writerBefore);
            const bool writerArmed = err == NoError && writerBefore.header.guid == ownedLayer.header.guid &&
                                     (writerBefore.header.flags & protectedBits) == protectedBits;
            check (writerArmed, "writer flags independently rearmed before activating wrapper");
            if (!writerArmed)
                return;
            API_Element writerSettings = {};
            const bool writerRead = Favorite_GetByName (favoriteName, writerSettings);
            check (writerRead && GetElemTypeID (writerSettings.header) == API_SlabID &&
                       writerSettings.header.layer == ownedLayer.header.index,
                   "legacy writer wrapper returns actual selected Slab settings");
            API_Attribute writerAfter = {};
            writerAfter.header.typeID = API_LayerID;
            writerAfter.header.index = ownedLayer.header.index;
            err = ACAPI_Attribute_Get (&writerAfter);
            check (err == NoError && writerAfter.header.guid == ownedLayer.header.guid &&
                       (writerAfter.header.flags & protectedBits) == 0,
                   "legacy writer wrapper still unhides and unlocks selected layer");
        };
        runFavoriteLayerFixture ();
    }
    caseName = "settings";

    struct LayerBand {
        double minimum = std::numeric_limits<double>::infinity ();
        double maximum = -std::numeric_limits<double>::infinity ();
        std::vector<API_AttributeIndex> materials;
    };

    const auto layerBands = [] (const RoomSurfaceSnapshot &snapshot, const OtdSlab &support) {
        GS::Array<LayerBand> bands;
        const double nz = 1 / std::sqrt (1 + support.slope_x * support.slope_x + support.slope_y * support.slope_y);
        for (const auto &body : snapshot.bodies) {
            LayerBand band;
            for (const auto &face : body.surfaces) {
                const auto &n = face.normalWorld;
                if (std::fabs ((n.z - n.x * support.slope_x - n.y * support.slope_y) * nz) < 1 - 1e-6)
                    band.materials.push_back (face.modelMaterialIndex);
                for (const auto &contour : face.contours)
                    for (const auto &vertex : contour.vertices) {
                        const auto &v = vertex.world;
                        const double distance =
                            (v.z - support.zBottom - support.slope_x * (v.x - support.plane_origin.x) -
                             support.slope_y * (v.y - support.plane_origin.y)) *
                            nz;
                        band.minimum = std::min (band.minimum, distance);
                        band.maximum = std::max (band.maximum, distance);
                    }
            }
            std::sort (band.materials.begin (), band.materials.end ());
            band.materials.erase (std::unique (band.materials.begin (), band.materials.end ()), band.materials.end ());
            bands.Push (band);
        }
        return bands;
    };
    const GS::Array<Point2D> cutFixtureCoords = {{0, 0}, {2, 0}, {2, 2}, {0, 2}};
    const auto cutFixture = Geometry::Polygon2D::Create (cutFixtureCoords, 0);
    if (cutFixture.GetSize () != 1)
        return GS::ObjectState ("status", "CutFixtureError");
    bool fullyInside = false;
    bool degenerate = false;
    GS::Int32 warning = 0;
    const auto leftHalf = cutFixture[0].Cut (HalfPlane2DData (1, 0, 1), &fullyInside, &warning, &degenerate);
    check (leftHalf.GetSize () == 1 && std::fabs (leftHalf[0].CalcArea () - 2) < 1e-6 && !fullyInside && warning == 0 &&
               !degenerate,
           "SDK half-plane keeps ca*x+cb*y<=cc without warnings");
    const auto whole = cutFixture[0].Cut (HalfPlane2DData (1, 0, 3), &fullyInside, &warning, &degenerate);
    check (whole.GetSize () == 1 && std::fabs (whole[0].CalcArea () - 4) < 1e-6 && fullyInside && warning == 0 &&
               !degenerate,
           "SDK fully-inside cut preserves finite polygon");
    const auto outside = cutFixture[0].Cut (HalfPlane2DData (1, 0, -1), &fullyInside, &warning, &degenerate);
    check (outside.IsEmpty () && !fullyInside && warning == 0, "SDK fully-outside cut returns no area");
    {
        caseName = "projected geometry";
        const API_Guid ownerA = APIGuidFromString ("00000000-0000-0000-0000-000000000271");
        const API_Guid ownerB = APIGuidFromString ("00000000-0000-0000-0000-000000001271");
        const API_Guid ownerC = APIGuidFromString ("00000000-0000-0000-0000-000000002271");
        const auto rectangle = [] (double x0, double y0, double x1, double y1) {
            const GS::Array<Point2D> coordinates = {{x0, y0}, {x1, y0}, {x1, y1}, {x0, y1}};
            return Geometry::Polygon2D::Create (coordinates, 0)[0];
        };
        const auto room = rectangle (0, 0, 4, 2);
        const auto mesh = [] (API_Guid owner, double x0, double x1, double top, double slope, bool bottom) {
            MeshFloorSource result;
            result.hasBottom = bottom;
            auto &source = result.surface;
            source.baseGuid = owner;
            source.geometryGuid = owner;
            source.baseType = API_MeshID;
            source.snapshot.guid = owner;
            source.snapshot.status = RoomSurfaceStatus::Extracted;
            RoomSurfaceBody body;
            body.index = 1;
            const GS::Array<API_Coord> xy = {{x0, 0}, {x1, 0}, {x1, 2}, {x0, 2}};
            for (Int32 side = 0; side < (bottom ? 2 : 1); ++side) {
                RoomSurface face;
                face.bodyIndex = 1;
                face.polygonIndex = side + 1;
                const double length = std::sqrt (1 + slope * slope);
                face.normalWorld = side == 0 ? API_Coord3D{-slope / length, 0, 1 / length} : API_Coord3D{0, 0, -1};
                face.planeOffset = side == 0 ? -top / length : -3;
                RoomSurfaceContour contour;
                for (const auto &point : xy) {
                    RoomSurfaceVertex vertex;
                    vertex.world = {point.x, point.y, side == 0 ? top + slope * point.x : -3};
                    contour.vertices.Push (vertex);
                }
                face.contours.Push (contour);
                body.surfaces.Push (face);
            }
            source.snapshot.bodies.Push (body);
            return result;
        };
        const auto coverageFor = [] (const GS::Array<OtdSlab> &floors, API_Guid owner) {
            Geometry::MultiPolygon2D result;
            for (const auto &floor : floors)
                if (floor.base_guid == owner)
                    result.Push (floor.poly);
            result.Unify ();
            return result;
        };
        const auto sameCoverage = [] (const Geometry::MultiPolygon2D &actual, const Geometry::Polygon2D &expected) {
            Geometry::MultiPolygon2D wanted;
            wanted.Push (expected);
            return Geometry::MultiPolygon2D::Substract (actual, wanted).CalcArea () <= 1e-5 &&
                   Geometry::MultiPolygon2D::Substract (wanted, actual).CalcArea () <= 1e-5;
        };
        GS::Array<OtdSlab> floors;
        Geometry::MultiPolygon2D domain;
        check (BuildProjectedMeshFloorFinishes (room, 0, 0, {}, {}, floors, domain) && floors.IsEmpty () &&
                   domain.IsEmpty (),
               "no Mesh leaves the legacy domain untouched");

        auto source = mesh (ownerA, 0, 4, -2, 1, true);
        check (BuildProjectedMeshFloorFinishes (room, 0, 0, {source}, {}, floors, domain),
               "solid sloped Mesh is admitted by its bottom");
        check (floors.GetSize () == 1 && sameCoverage (domain, room), "finite sloped Mesh covers the room exactly");
        check (floors.GetSize () == 1 && floors[0].base_guid == ownerA && floors[0].base_type == API_MeshID &&
                   floors[0].draw_type == API_RoofID && floors[0].type == Floor && floors[0].surface_geometry &&
                   std::fabs (floors[0].slope_x - 1) < 1e-6 &&
                   std::fabs (floors[0].zBottom - floors[0].plane_origin.x + 2) < 1e-6,
               "Mesh output retains owner type and the actual contact plane");

        floors.Clear ();
        source.hasBottom = false;
        source.surface.snapshot.bodies[0].surfaces = {source.surface.snapshot.bodies[0].surfaces[0]};
        check (BuildProjectedMeshFloorFinishes (room, 0, 0, {source}, {}, floors, domain),
               "Mesh without bottom uses its local top");
        check (sameCoverage (domain, rectangle (0, 0, 2, 2)),
               "Mesh without bottom clips eligibility locally not by global Z");

        floors.Clear ();
        auto high = mesh (ownerA, 0, 4, 1, 0, false);
        check (BuildProjectedMeshFloorFinishes (room, 0, 0, {high}, {}, floors, domain) && floors.IsEmpty () &&
                   domain.IsEmpty (),
               "Mesh without bottom above the floor does not replace the legacy domain");
        auto raisedBottom = mesh (ownerA, 0, 4, 2, 0, true);
        auto &bottomFace = raisedBottom.surface.snapshot.bodies[0].surfaces[1];
        for (auto &vertex : bottomFace.contours[0].vertices)
            vertex.world.z = 1;
        bottomFace.planeOffset = 1;
        check (BuildProjectedMeshFloorFinishes (room, 0, 0, {raisedBottom}, {}, floors, domain) && floors.IsEmpty () &&
                   domain.IsEmpty (),
               "solid Mesh above the floor is ineligible despite a valid top");

        floors.Clear ();
        source = mesh (ownerA, 0, 4, -2, 1, true);
        OtdSlab support;
        support.poly = room;
        support.base_guid = ownerB;
        support.base_type = API_SlabID;
        support.type = Floor;
        support.surface_geometry = true;
        check (BuildProjectedMeshFloorFinishes (room, 0, 0, {source}, {support}, floors, domain),
               "crossing Mesh and Slab heights are not an ambiguous area");
        check (sameCoverage (coverageFor (floors, ownerB), rectangle (0, 0, 2, 2)) &&
                   sameCoverage (coverageFor (floors, ownerA), rectangle (2, 0, 4, 2)),
               "Mesh Slab priority changes at the actual equality line");
        check (floors.GetSize () == 2, "crossing Mesh Slab emits no duplicate coverage");

        floors.Clear ();
        auto other = mesh (ownerB, 0, 4, 2, -1, true);
        check (BuildProjectedMeshFloorFinishes (room, 0, 0, {source, other}, {}, floors, domain),
               "two crossing Mesh planes are locally ordered");
        check (sameCoverage (coverageFor (floors, ownerB), rectangle (0, 0, 2, 2)) &&
                   sameCoverage (coverageFor (floors, ownerA), rectangle (2, 0, 4, 2)),
               "different slopes retain both finite upper cells");
        const auto referenceA = coverageFor (floors, ownerA);
        const auto referenceB = coverageFor (floors, ownerB);
        floors.Clear ();
        check (BuildProjectedMeshFloorFinishes (room, 0, 0, {other, source}, {}, floors, domain) &&
                   Geometry::MultiPolygon2D::Substract (referenceA, coverageFor (floors, ownerA)).CalcArea () < 1e-5 &&
                   Geometry::MultiPolygon2D::Substract (coverageFor (floors, ownerA), referenceA).CalcArea () < 1e-5 &&
                   Geometry::MultiPolygon2D::Substract (referenceB, coverageFor (floors, ownerB)).CalcArea () < 1e-5 &&
                   Geometry::MultiPolygon2D::Substract (coverageFor (floors, ownerB), referenceB).CalcArea () < 1e-5,
               "input permutation does not choose a different owner");

        floors.Clear ();
        auto flatA = mesh (ownerA, 0, 4, 0, 0, true);
        auto flatB = mesh (ownerB, 0, 4, 0, 0, true);
        floors.Push (support);
        domain.Clear ();
        domain.Push (room);
        check (!BuildProjectedMeshFloorFinishes (room, 0, 0, {flatA, flatB}, {}, floors, domain),
               "coincident different owners reject instead of GUID tiebreak");
        check (floors.GetSize () == 1 && floors[0].base_guid == ownerB && sameCoverage (domain, room),
               "rejection leaves every output unchanged");
        floors.Clear ();
        high = mesh (ownerC, 0, 4, 1, 0, true);
        check (BuildProjectedMeshFloorFinishes (room, 0, 0, {flatA, flatB, high}, {}, floors, domain) &&
                   sameCoverage (coverageFor (floors, ownerC), room) && floors.GetSize () == 1,
               "higher third source hides a lower ownership tie");

        floors.Clear ();
        auto left = mesh (ownerA, 0, 2, -1, 0.2, true);
        auto right = mesh (ownerA, 2, 4, -1, 0.2, true);
        left.surface.snapshot.bodies[0].surfaces.Append (right.surface.snapshot.bodies[0].surfaces);
        check (BuildProjectedMeshFloorFinishes (room, 0, 0, {left}, {}, floors, domain) && floors.GetSize () == 1 &&
                   sameCoverage (domain, room),
               "adjacent coplanar same owner patches merge to one plane");
        floors.Clear ();
        left = mesh (ownerA, 0, 1, -1, 0, true);
        right = mesh (ownerA, 3, 4, -1, 0, true);
        left.surface.snapshot.bodies[0].surfaces.Append (right.surface.snapshot.bodies[0].surfaces);
        check (BuildProjectedMeshFloorFinishes (room, 0, 0, {left}, {}, floors, domain) && floors.GetSize () == 2 &&
                   std::fabs (domain.CalcArea () - 4) < 1e-5,
               "disjoint coplanar islands are retained not PopLargest");

        floors.Clear ();
        auto holed = mesh (ownerA, 0, 4, -1, 0, true);
        for (auto &face : holed.surface.snapshot.bodies[0].surfaces) {
            RoomSurfaceContour hole;
            hole.hole = true;
            const GS::Array<API_Coord> coordinates = {{1, 0.5}, {3, 0.5}, {3, 1.5}, {1, 1.5}};
            for (const auto &point : coordinates) {
                RoomSurfaceVertex vertex;
                vertex.world = {point.x, point.y, face.normalWorld.z > 0 ? -1.0 : -3.0};
                hole.vertices.Push (vertex);
            }
            face.contours.Push (hole);
        }
        check (BuildProjectedMeshFloorFinishes (room, 0, 0, {holed}, {}, floors, domain) &&
                   std::fabs (domain.CalcArea () - 6) < 1e-5 && floors.GetSize () == 1 &&
                   floors[0].poly.GetContourNum () == 2,
               "Mesh holes remain uncovered");
        floors.Clear ();
        check (BuildProjectedMeshFloorFinishes (rectangle (0, 0, 2, 2), 0, 0, {holed}, {}, floors, domain) &&
                   std::fabs (domain.CalcArea () - 3) < 1e-5 && floors.GetSize () == 1 &&
                   floors[0].poly.GetContourNum () == 1,
               "partial clipping opens a hole into the exterior correctly");

        floors.Clear ();
        auto broken = flatA;
        broken.surface.snapshot.status = RoomSurfaceStatus::ReadError;
        check (!BuildProjectedMeshFloorFinishes (room, 0, 0, {broken}, {}, floors, domain) && floors.IsEmpty (),
               "incomplete Mesh snapshot refuses nominal replacement");
        broken = flatA;
        broken.surface.snapshot.bodies[0].surfaces[0].contours[0].vertices[2].world.z += 0.01;
        check (!BuildProjectedMeshFloorFinishes (room, 0, 0, {broken}, {}, floors, domain),
               "nonplanar top face is rejected before writing");
        broken = flatA;
        broken.surface.snapshot.guid = ownerB;
        check (!BuildProjectedMeshFloorFinishes (room, 0, 0, {broken}, {}, floors, domain),
               "snapshot owner mismatch is rejected");
        caseName = "settings";
    }
    const Stories stories = GetStories ();
    if (stories.IsEmpty ())
        return GS::ObjectState ("status", "StoryReadError");

    API_Element settings = {};
    SetElemTypeID (settings, API_SlabID);
    GSErrCode error = ACAPI_Element_GetDefaults (&settings, nullptr);
    check (error == NoError, "read slab settings");
    if (error != NoError)
        return GS::ObjectState ("status", "Failed", "checks", checks, "failed", failed);
    settings.header.layer = 1;
    settings.slab.modelElemStructureType = API_BasicStructure;
    settings.slab.thickness = 0.08;

    API_Element compositeSettings = settings;
    bool hasComposite = false;
    API_AttributeIndex count = 0;
    error = ACAPI_Attribute_GetNum (API_CompWallID, &count);
    check (error == NoError, "enumerate composites");
    if (error == NoError) {
        for (API_AttributeIndex index = 1; index <= count; ++index) {
            API_Attribute attribute = {};
            attribute.header.typeID = API_CompWallID;
            attribute.header.index = index;
            if (ACAPI_Attribute_Get (&attribute) != NoError || attribute.compWall.nComps < 2 ||
                (attribute.header.flags & APICWall_ForRoof) == 0 || (attribute.header.flags & APICWall_ForSlab) == 0)
                continue;
            compositeSettings.slab.modelElemStructureType = API_CompositeStructure;
            compositeSettings.slab.composite = index;
            compositeSettings.slab.thickness = attribute.compWall.totalThick;
            hasComposite = true;
            break;
        }
    }
    check (hasComposite, "fixture has multilayer slab/roof composite");

    struct Scenario {
        const char *name;
        double heights[5];
        bool hole;
        bool singlePlane;
        bool hasCenter = false;
        double center = 0;
        bool expectNewJoint = false;
        bool partial = false;
        short skirt = 1;
    };

    const Scenario scenarios[] = {
        {"valley", {0, 0.1, 0, 0.4, 0}, false, false},
        {"ridge", {0, -0.1, 0, -0.4, 0}, false, false},
        {"sloping joint", {0, 0.3, 0.2, 0.6, 0}, false, false},
        {"horizontal", {0, 0, 0, 0, 0}, false, true},
        {"single slope", {0, 0.2, 0.4, 0.2, 0}, false, true},
        {"partial room footprint", {0, 0.1, 0, 0.4, 0}, true, false, false, 0, false, true},
        {"room hole", {0, 0.1, 0, 0.4, 0}, true, false},
        {"symmetric funnel", {0, 0, 0, 0, 0}, false, false, true, -0.2},
        {"asymmetric funnel", {0, 0.1, 0.15, 0.2, 0}, false, false, true, -0.2, true},
        {"mesh without bottom", {0, 0.2, 0.4, 0.2, 0}, false, true, false, 0, false, false, 2},
        {"mesh without skirt", {0, 0.2, 0.4, 0.2, 0}, false, true, false, 0, false, false, 3}};
    for (const auto &scenario : scenarios) {
        caseName = scenario.name;
        API_Element mesh = {};
        SetElemTypeID (mesh, API_MeshID);
        error = ACAPI_Element_GetDefaults (&mesh, nullptr);
        check (error == NoError, "read mesh defaults");
        GS::Array<API_Guid> created;
        if (error == NoError) {
            mesh.header.layer = 1;
            mesh.mesh.level = GetOffsetFromStory (0, mesh.header.floorInd, stories);
            check (std::fabs (GetzPos (mesh.mesh.level, mesh.header.floorInd, stories)) < 1e-6,
                   "fixture reference is absolute zero on its selected story");
            mesh.mesh.skirt = scenario.skirt;
            mesh.mesh.skirtLevel = 1;
            mesh.mesh.poly.nCoords = 5;
            mesh.mesh.poly.nSubPolys = 1;
            mesh.mesh.poly.nArcs = 0;
            mesh.mesh.levelLines = {};
            API_ElementMemo memo = {};
            const GS::OnExit memoGuard ([&memo] () { ACAPI_DisposeElemMemoHdls (&memo); });
            memo.coords = reinterpret_cast<API_Coord **> (BMAllocateHandle (6 * sizeof (API_Coord), ALLOCATE_CLEAR, 0));
            memo.pends = reinterpret_cast<Int32 **> (BMAllocateHandle (2 * sizeof (Int32), ALLOCATE_CLEAR, 0));
            memo.meshPolyZ = reinterpret_cast<double **> (BMAllocateHandle (6 * sizeof (double), ALLOCATE_CLEAR, 0));
            if (memo.coords == nullptr || memo.pends == nullptr || memo.meshPolyZ == nullptr) {
                check (false, "allocate mesh memo");
            } else {
                const API_Coord points[] = {{1000, 1000}, {1002, 1000}, {1002, 1002}, {1000, 1002}, {1000, 1000}};
                const auto &heights = scenario.heights;
                for (Int32 i = 0; i < 5; ++i) {
                    (*memo.coords)[i + 1] = points[i];
                    (*memo.meshPolyZ)[i + 1] = heights[i];
                }
                (*memo.pends)[1] = 5;
                if (scenario.hasCenter) {
                    mesh.mesh.levelLines.nCoords = 1;
                    mesh.mesh.levelLines.nSubLines = 1;
                    memo.meshLevelCoords = reinterpret_cast<API_MeshLevelCoord **> (
                        BMAllocateHandle (sizeof (API_MeshLevelCoord), ALLOCATE_CLEAR, 0));
                    memo.meshLevelEnds =
                        reinterpret_cast<Int32 **> (BMAllocateHandle (sizeof (Int32), ALLOCATE_CLEAR, 0));
                    if (memo.meshLevelCoords == nullptr || memo.meshLevelEnds == nullptr) {
                        check (false, "allocate native funnel center");
                        continue;
                    }
                    (*memo.meshLevelCoords)[0].c = {1001, 1001, scenario.center};
                    (*memo.meshLevelCoords)[0].vertexID = 6;
                    (*memo.meshLevelEnds)[0] = 1;
                }
                error = ACAPI_CallUndoableCommand ("Mesh floor native fixture", [&] () -> GSErrCode {
                    const GSErrCode createError = ACAPI_Element_Create (&mesh, &memo);
                    if (createError == NoError)
                        created.Push (mesh.header.guid);
                    return createError;
                });
                check (error == NoError, "create native nonplanar mesh");
            }
        }
        if (!created.IsEmpty ()) {
            const auto snapshot = ReadRoomSurfaces (mesh.header);
            check (snapshot.IsComplete (), "read native mesh world faces");
            if (scenario.hasCenter) {
                bool hasActualCenter = false;
                for (const auto &body : snapshot.bodies)
                    for (const auto &face : body.surfaces)
                        for (const auto &contour : face.contours)
                            for (const auto &vertex : contour.vertices) {
                                const auto &v = vertex.world;
                                hasActualCenter = hasActualCenter || (std::hypot (std::hypot (v.x - 1001, v.y - 1001),
                                                                                  v.z - scenario.center) < 1e-6);
                            }
                check (hasActualCenter, "independent native mesh reader confirms requested funnel center");
            }
            const GS::Array<Point2D> discoveryCoords = {
                {1000.25, 1000.25}, {1001.75, 1000.25}, {1001.75, 1001.75}, {1000.25, 1001.75}};
            const auto discoveryFootprints = Geometry::Polygon2D::Create (discoveryCoords, 0);
            check (discoveryFootprints.GetSize () == 1, "prepare independent discovery footprint");
            if (discoveryFootprints.GetSize () == 1) {
                const API_Guid roomGuid = APIGuidFromString ("03040506-0708-0910-1112-131415161718");
                GS::HashTable<API_Guid, Geometry::Polygon2D> footprints;
                footprints.Add (roomGuid, discoveryFootprints[0]);
                MeshFloorIndex index;
                RoomSurfaceCache cache;
                check (Mesh_FindAll (index, {}, footprints, &cache), "discover real mesh by finite XY overlap");
                const auto countOwn = [&] (const MeshFloorIndex &found) {
                    USize countOwn = 0;
                    if (const auto *sources = found.GetPtr (roomGuid))
                        for (const auto &source : *sources)
                            if (source.surface.baseGuid == mesh.header.guid)
                                ++countOwn;
                    return countOwn;
                };
                check (countOwn (index) == 1, "real mesh occurs exactly once in typed index");
                if (const auto *sources = index.GetPtr (roomGuid)) {
                    for (auto source : *sources) {
                        if (source.surface.baseGuid != mesh.header.guid)
                            continue;
                        check (source.surface.baseType == API_MeshID && source.surface.geometryGuid == mesh.header.guid,
                               "typed discovery retains native mesh type and GUID");
                        check (source.hasBottom == (scenario.skirt == 1),
                               "typed discovery distinguishes solid and open mesh");
                        check (source.surface.snapshot.status == RoomSurfaceStatus::NotRead,
                               "typed discovery defers native body read");
                        source.surface.snapshot = snapshot;
                        GS::Array<OtdSlab> projected;
                        Geometry::MultiPolygon2D coverage;
                        check (BuildProjectedMeshFloorFinishes (
                                   discoveryFootprints[0], 1, mesh.header.floorInd, {source}, {}, projected, coverage),
                               "real native mesh reaches projected floor kernel");
                        Geometry::MultiPolygon2D expected;
                        expected.Push (discoveryFootprints[0]);
                        const auto missing = Geometry::MultiPolygon2D::Substract (expected, coverage);
                        const auto outside = Geometry::MultiPolygon2D::Substract (coverage, expected);
                        bool typed = !projected.IsEmpty ();
                        for (const auto &floor : projected)
                            typed = typed && floor.base_guid == mesh.header.guid && floor.base_type == API_MeshID &&
                                    floor.draw_type == API_RoofID;
                        check (typed && missing.CalcArea () < 1e-6 && outside.CalcArea () < 1e-6,
                               "native projection preserves finite footprint and mesh provenance");
                    }
                }
                check (Mesh_FindAll (index, {}, footprints, &cache) && countOwn (index) == 1,
                       "repeated discovery does not append duplicate mesh");
                const GS::Array<Point2D> outsideCoords = {{1002, 1000}, {1003, 1000}, {1003, 1002}, {1002, 1002}};
                const auto outsideFootprint = Geometry::Polygon2D::Create (outsideCoords, 0);
                footprints[roomGuid] = outsideFootprint[0];
                check (Mesh_FindAll (index, {}, footprints, &cache) && countOwn (index) == 0,
                       "edge-only contact does not admit mesh as floor coverage");
                const GS::Array<Point2D> outerCoords = {{999, 999}, {1003, 999}, {1003, 1003}, {999, 1003}};
                const GS::Array<Point2D> holeCoords = {
                    {999.5, 999.5}, {1002.5, 999.5}, {1002.5, 1002.5}, {999.5, 1002.5}};
                const auto outer = Geometry::Polygon2D::Create (outerCoords, 0);
                const auto hole = Geometry::Polygon2D::Create (holeCoords, 0);
                const auto ring = outer[0].Substract (hole[0]);
                footprints[roomGuid] = ring[0];
                check (Mesh_FindAll (index, {}, footprints, &cache) && countOwn (index) == 0,
                       "mesh entirely inside room hole is not admitted");
            }
            GS::Array<OtdSlab> floors;
            GS::Array<RoomSurface> supportFaces;
            for (const auto &body : snapshot.bodies)
                for (const auto &face : body.surfaces) {
                    if (face.normalWorld.z <= 1e-6 || face.contours.IsEmpty () || face.contours[0].vertices.IsEmpty ())
                        continue;
                    GS::Array<Point2D> coords;
                    GS::Array<PolyArcRec> arcs;
                    GS::Array<UIndex> ends;
                    for (const auto &contour : face.contours) {
                        for (const auto &vertex : contour.vertices)
                            coords.Push ({vertex.world.x, vertex.world.y});
                        ends.Push (coords.GetSize () - 1);
                    }
                    const auto polygons = Geometry::Polygon2D::Create (
                        coords, arcs, ends, Geometry::PolyCreateFlags_IdxRefInContourEndFromZero);
                    const auto origin = face.contours[0].vertices[0].world;
                    for (const auto &polygon : polygons) {
                        OtdSlab floor;
                        floor.type = Floor;
                        floor.base_type = API_MeshID;
                        floor.base_guid = mesh.header.guid;
                        floor.floorInd = mesh.header.floorInd;
                        floor.poly = polygon;
                        if (scenario.hole) {
                            const GS::Array<Point2D> holeCoords = {
                                {1001.4, 1000.4}, {1001.6, 1000.4}, {1001.6, 1000.6}, {1001.4, 1000.6}};
                            const auto hole = Geometry::Polygon2D::Create (holeCoords, 0);
                            const auto clipped = polygon.Substract (hole[0]);
                            check (clipped.GetSize () == 1, "room-hole clipping retains one finite patch");
                            if (clipped.GetSize () != 1)
                                continue;
                            floor.poly = clipped[0];
                        }
                        if (scenario.partial) {
                            const GS::Array<Point2D> roomCoords = {
                                {999.9, 999.9}, {1001.5, 999.9}, {1001.5, 1002.1}, {999.9, 1002.1}};
                            const auto footprint = Geometry::Polygon2D::Create (roomCoords, 0);
                            check (footprint.GetSize () == 1, "prepare independent smaller room footprint");
                            if (footprint.GetSize () != 1)
                                continue;
                            const auto clipped = floor.poly.Intersect (footprint[0]);
                            check (clipped.GetSize () == 1, "room boundary clips finite support and opens hole notch");
                            if (clipped.GetSize () != 1)
                                continue;
                            floor.poly = clipped[0];
                        }
                        floor.zBottom = origin.z;
                        floor.plane_origin = {origin.x, origin.y};
                        floor.slope_x = -face.normalWorld.x / face.normalWorld.z;
                        floor.slope_y = -face.normalWorld.y / face.normalWorld.z;
                        floors.Push (floor);
                        supportFaces.Push (face);
                    }
                }
            check (scenario.singlePlane ? !floors.IsEmpty () : floors.GetSize () >= 2,
                   "mesh supplies expected finite upper planes");
            for (Int32 mode = 0; mode < (hasComposite ? 2 : 1); ++mode) {
                const API_Element &slabSettings = mode == 0 ? settings : compositeSettings;
                GS::Array<RoomSurfaceSnapshot> actualRoofs;
                API_Guid referenceGuid = APINULLGuid;
                GS::Array<LayerBand> referenceBands;
                error = ACAPI_CallUndoableCommand ("Mesh floor native roofs", [&] () -> GSErrCode {
                    if (mode != 0 && !floors.IsEmpty ()) {
                        API_Element reference = slabSettings;
                        reference.header.guid = APINULLGuid;
                        reference.header.floorInd = floors[0].floorInd;
                        reference.slab.referencePlaneLocation = APISlabRefPlane_Top;
                        reference.slab.offsetFromTop = 0;
                        reference.slab.level =
                            GetOffsetFromStory (slabSettings.slab.thickness, reference.header.floorInd, stories);
                        const GS::Array<Point2D> coords = {{1010, 1010}, {1012, 1010}, {1010, 1012}};
                        const auto shape = Geometry::Polygon2D::Create (coords, 0);
                        API_ElementMemo referenceMemo = {};
                        const GS::OnExit referenceGuard (
                            [&referenceMemo] () { ACAPI_DisposeElemMemoHdls (&referenceMemo); });
                        GSErrCode referenceError =
                            ConvertPolygon2DToAPIPolygon (shape[0], reference.slab.poly, referenceMemo);
                        if (referenceError == NoError)
                            referenceError = ACAPI_Element_Create (&reference, &referenceMemo);
                        check (referenceError == NoError, "create independent composite slab reference");
                        if (referenceError == NoError) {
                            referenceGuid = reference.header.guid;
                            created.Push (referenceGuid);
                        }
                    }
                    for (auto &floor : floors) {
                        floor.otd_guid = APINULLGuid;
                        const GSErrCode createError = CreateFloorRoof (stories, floor, slabSettings, &floors);
                        check (createError == NoError && floor.otd_guid != APINULLGuid,
                               mode == 0 ? "create basic roof on native mesh plane"
                                         : "create composite roof on native mesh plane");
                        if (createError == NoError)
                            created.Push (floor.otd_guid);
                    }
                    return NoError;
                });
                check (error == NoError, "roof undo transaction");
                if (mode != 0) {
                    API_Elem_Head referenceHeader = {};
                    referenceHeader.guid = referenceGuid;
                    const auto reference = ReadRoomSurfaces (referenceHeader);
                    OtdSlab support;
                    support.zBottom = 0;
                    support.plane_origin = {1010, 1010};
                    referenceBands = layerBands (reference, support);
                    check (referenceGuid != APINULLGuid && reference.IsComplete () && !referenceBands.IsEmpty (),
                           "read independent slab layer bands and materials");
                }
                for (const auto &floor : floors) {
                    if (floor.otd_guid == APINULLGuid)
                        continue;
                    API_Element native = {};
                    native.header.guid = floor.otd_guid;
                    const GSErrCode readError = ACAPI_Element_Get (&native);
                    check (readError == NoError && GetElemTypeID (native) == API_RoofID,
                           "read actual native roof type");
                    const auto roof = ReadRoomSurfaces (native.header);
                    check (roof.IsComplete () && !roof.bodies.IsEmpty (), "read actual roof bodies");
                    actualRoofs.Push (roof);
                    double minimum = std::numeric_limits<double>::infinity ();
                    double maximum = -minimum;
                    bool withinExterior = true;
                    Geometry::MultiPolygon2D actualContact;
                    const double normalZ =
                        1 / std::sqrt (1 + floor.slope_x * floor.slope_x + floor.slope_y * floor.slope_y);
                    for (const auto &body : roof.bodies)
                        for (const auto &face : body.surfaces) {
                            bool onBase = true;
                            GS::Array<Point2D> contactCoords;
                            GS::Array<UIndex> contactEnds;
                            GS::Array<PolyArcRec> contactArcs;
                            for (const auto &contour : face.contours) {
                                for (const auto &vertex : contour.vertices) {
                                    const auto &p = vertex.world;
                                    const double distance =
                                        (p.z - floor.zBottom - floor.slope_x * (p.x - floor.plane_origin.x) -
                                         floor.slope_y * (p.y - floor.plane_origin.y)) *
                                        normalZ;
                                    minimum = std::min (minimum, distance);
                                    maximum = std::max (maximum, distance);
                                    onBase = onBase && std::fabs (distance) < 1e-6;
                                    withinExterior = withinExterior && p.x >= 1000 - 1e-6 && p.x <= 1002 + 1e-6 &&
                                                     p.y >= 1000 - 1e-6 && p.y <= 1002 + 1e-6;
                                    contactCoords.Push ({p.x, p.y});
                                }
                                contactEnds.Push (contactCoords.GetSize () - 1);
                            }
                            if (onBase)
                                actualContact.Append (
                                    Geometry::Polygon2D::Create (contactCoords,
                                                                 contactArcs,
                                                                 contactEnds,
                                                                 Geometry::PolyCreateFlags_IdxRefInContourEndFromZero));
                        }
                    actualContact.Unify ();
                    Geometry::MultiPolygon2D expectedContact;
                    expectedContact.Push (floor.poly);
                    const double areaTolerance = 1e-6 * (1 + 0.05 * floor.poly.CalcArea ());
                    check (!actualContact.IsEmpty () &&
                               Geometry::MultiPolygon2D::Substract (expectedContact, actualContact).CalcArea () <=
                                   areaTolerance &&
                               Geometry::MultiPolygon2D::Substract (actualContact, expectedContact).CalcArea () <=
                                   areaTolerance,
                           "native bottom covers exact finite patch including room hole");
                    check (withinExterior, "all native roof vertices remain inside finite exterior");
                    check (std::isfinite (minimum) && std::fabs (minimum) < 1e-6, "roof bottom contacts mesh plane");
                    check (std::isfinite (maximum) && std::fabs (maximum - slabSettings.slab.thickness) < 1e-6,
                           "roof thickness measured along normal");
                    if (mode != 0) {
                        check (native.roof.shellBase.modelElemStructureType == API_CompositeStructure &&
                                   native.roof.shellBase.composite == slabSettings.slab.composite,
                               "native roof retains selected composite");
                        const auto actualBands = layerBands (roof, floor);
                        bool sameLayers =
                            !referenceBands.IsEmpty () && actualBands.GetSize () == referenceBands.GetSize ();
                        std::vector<bool> matchedBands (referenceBands.GetSize (), false);
                        for (const auto &band : actualBands) {
                            bool found = false;
                            for (UIndex i = 0; i < referenceBands.GetSize (); ++i) {
                                const auto &expected = referenceBands[i];
                                if (!matchedBands[i] && std::fabs (band.minimum - expected.minimum) < 1e-6 &&
                                    std::fabs (band.maximum - expected.maximum) < 1e-6 &&
                                    band.materials == expected.materials) {
                                    matchedBands[i] = true;
                                    found = true;
                                    break;
                                }
                            }
                            sameLayers = sameLayers && found;
                        }
                        check (sameLayers, "roof skin order thicknesses and materials match independent slab");
                    }
                    geometry.Push (GS::ObjectState ("guid",
                                                    APIGuidToString (floor.otd_guid),
                                                    "mode",
                                                    mode,
                                                    "minimumDistance",
                                                    minimum,
                                                    "maximumDistance",
                                                    maximum,
                                                    "expectedThickness",
                                                    slabSettings.slab.thickness,
                                                    "slopeX",
                                                    floor.slope_x,
                                                    "slopeY",
                                                    floor.slope_y));
                }
                const auto sameNativePoint = [] (const API_Coord3D &a, const API_Coord3D &b) {
                    return std::hypot (std::hypot (a.x - b.x, a.y - b.y), a.z - b.z) < 1e-6;
                };
                const auto sameFace = [&] (const RoomSurface &a, const RoomSurface &b) {
                    if (a.contours.GetSize () != b.contours.GetSize ())
                        return false;
                    for (const auto &contour : a.contours)
                        for (const auto &vertex : contour.vertices) {
                            bool found = false;
                            for (const auto &other : b.contours)
                                for (const auto &candidate : other.vertices)
                                    found = found || sameNativePoint (vertex.world, candidate.world);
                            if (!found)
                                return false;
                        }
                    for (const auto &contour : b.contours)
                        for (const auto &vertex : contour.vertices) {
                            bool found = false;
                            for (const auto &other : a.contours)
                                for (const auto &candidate : other.vertices)
                                    found = found || sameNativePoint (vertex.world, candidate.world);
                            if (!found)
                                return false;
                        }
                    return std::hypot (
                               std::hypot (a.normalWorld.x + b.normalWorld.x, a.normalWorld.y + b.normalWorld.y),
                               a.normalWorld.z + b.normalWorld.z) < 1e-6;
                };
                check (actualRoofs.GetSize () == supportFaces.GetSize (),
                       "all roof snapshots available for seam check");
                USize newJointPairs = 0;
                if (actualRoofs.GetSize () == supportFaces.GetSize ())
                    for (UIndex i = 0; i < supportFaces.GetSize (); ++i)
                        for (UIndex j = i + 1; j < supportFaces.GetSize (); ++j) {
                            GS::Array<API_Coord3D> common;
                            for (const auto &a : supportFaces[i].contours[0].vertices)
                                for (const auto &b : supportFaces[j].contours[0].vertices)
                                    if (sameNativePoint (a.world, b.world))
                                        common.Push (a.world);
                            const bool pointOnly = common.GetSize () == 1;
                            if (!pointOnly && (common.GetSize () != 2 || sameNativePoint (common[0], common[1])))
                                continue;
                            const auto &firstNormal = supportFaces[i].normalWorld;
                            const auto &secondNormal = supportFaces[j].normalWorld;
                            const API_Coord3D difference = {firstNormal.x - secondNormal.x,
                                                            firstNormal.y - secondNormal.y,
                                                            firstNormal.z - secondNormal.z};
                            const double differenceLength =
                                std::hypot (std::hypot (difference.x, difference.y), difference.z);
                            if (pointOnly && differenceLength <= 1e-6)
                                continue;
                            bool complete = true;
                            USize count = 0;
                            for (UIndex side = 0; side < 2; ++side) {
                                const UIndex source = side == 0 ? i : j;
                                const UIndex target = side == 0 ? j : i;
                                for (const auto &body : actualRoofs[source].bodies)
                                    for (const auto &face : body.surfaces) {
                                        const auto &n = face.normalWorld;
                                        const auto &support = supportFaces[source].normalWorld;
                                        if (std::fabs (n.x * support.x + n.y * support.y + n.z * support.z) > 1 - 1e-6)
                                            continue;
                                        if (pointOnly &&
                                            std::fabs (n.x * difference.x + n.y * difference.y + n.z * difference.z) /
                                                    differenceLength <
                                                1 - 1e-6)
                                            continue;
                                        bool onSeam = true;
                                        for (const auto &point : common)
                                            onSeam = onSeam && std::fabs (n.x * point.x + n.y * point.y +
                                                                          n.z * point.z + face.planeOffset) < 1e-6;
                                        if (!onSeam)
                                            continue;
                                        ++count;
                                        bool matched = false;
                                        for (const auto &otherBody : actualRoofs[target].bodies)
                                            for (const auto &other : otherBody.surfaces)
                                                matched = matched || sameFace (face, other);
                                        complete = complete && matched;
                                    }
                            }
                            if (pointOnly) {
                                if (count == 0)
                                    continue;
                                ++newJointPairs;
                            }
                            check (count >= 2 && complete,
                                   pointOnly ? "new upper seam between initially nonadjacent planes coincides"
                                             : "native unequal-slope seam faces coincide without gap or overlap");
                        }
                check (newJointPairs == (scenario.expectNewJoint ? 1 : 0),
                       "native upper topology has independently expected new joint pair count");
            }
            error = ACAPI_CallUndoableCommand ("Remove mesh floor native fixture",
                                               [&] () -> GSErrCode { return ACAPI_Element_Delete (created); });
            check (error == NoError, "remove only created native fixture elements");
            bool removed = true;
            for (const auto &guid : created) {
                API_Elem_Head header = {};
                header.guid = guid;
                removed = removed && ACAPI_Element_GetHeader (&header) != NoError;
            }
            check (removed, "created fixture GUIDs no longer exist");
        }
    }
    {
        caseName = "finite mixed floor domain geometry";
        constexpr double AreaTolerance = 1.e-6;
        const API_Guid meshOwner = APIGuidFromString ("00000000-0000-0000-0000-00000000A001");
        const API_Guid slabOwner = APIGuidFromString ("00000000-0000-0000-0000-00000000A002");
        const API_Guid zoneOwner = APIGuidFromString ("00000000-0000-0000-0000-00000000A003");
        const auto rectangle = [] (double x0, double y0, double x1, double y1) {
            const GS::Array<Point2D> points = {{x0, y0}, {x1, y0}, {x1, y1}, {x0, y1}};
            return Geometry::Polygon2D::Create (points, 0)[0];
        };
        const auto face = [] (double x0, double y0, double x1, double y1, double z, bool up) {
            RoomSurface value;
            value.normalWorld = {0, 0, up ? 1.0 : -1.0};
            value.planeOffset = up ? -z : z;
            RoomSurfaceContour outer;
            const GS::Array<Point2D> points = {{x0, y0}, {x1, y0}, {x1, y1}, {x0, y1}};
            for (const auto &point : points) {
                RoomSurfaceVertex vertex;
                vertex.world = {point.x, point.y, z};
                outer.vertices.Push (vertex);
            }
            value.contours.Push (outer);
            return value;
        };
        const auto same = [AreaTolerance] (const Geometry::MultiPolygon2D &a, const Geometry::MultiPolygon2D &b) {
            return Geometry::MultiPolygon2D::Substract (a, b).CalcArea () <= AreaTolerance &&
                   Geometry::MultiPolygon2D::Substract (b, a).CalcArea () <= AreaTolerance;
        };
        const auto ownerCoverage = [] (const GS::Array<OtdSlab> &floors, const API_Guid &owner) {
            Geometry::MultiPolygon2D area;
            for (const auto &floor : floors)
                if (floor.base_guid == owner)
                    area.Push (floor.poly);
            area.Unify ();
            return area;
        };
        RoomSurfaceSnapshot zone;
        zone.guid = zoneOwner;
        zone.status = RoomSurfaceStatus::Extracted;
        RoomSurfaceBody roomBody;
        roomBody.index = 1;
        roomBody.surfaces.Push (face (0, 0, 6, 2, 0, false));
        roomBody.surfaces.Push (face (0, 0, 6, 2, 3, true));
        zone.bodies.Push (roomBody);
        OtdSlab base;
        base.poly = rectangle (0, 0, 6, 2);
        base.base_guid = slabOwner;
        base.base_type = API_SlabID;
        base.zBottom = -1;
        base.height = 1;
        base.reference_z = 17;
        base.tip = "retained base metadata";
        base.favorite.name = "retained Favorite metadata";
        RoomSurfaceSource slabSource;
        slabSource.baseGuid = slabOwner;
        slabSource.geometryGuid = slabOwner;
        slabSource.baseType = API_SlabID;
        slabSource.origin = RoomSurfaceOrigin::PreparedAnalytic;
        slabSource.snapshot.guid = slabOwner;
        slabSource.snapshot.status = RoomSurfaceStatus::Extracted;
        const GS::Array<RoomSurfaceSource> sources = {slabSource};
        const GS::Array<API_Guid> roles = {slabOwner, slabOwner};
        const GS::Array<OtdSlab> prepared = {base};
        OtdSlab nominal;
        nominal.poly = base.poly;
        nominal.zBottom = -2;
        nominal.base_guid = zoneOwner;
        nominal.base_type = API_ZoneID;
        MeshFloorSource mesh;
        mesh.hasBottom = true;
        mesh.surface.baseGuid = meshOwner;
        mesh.surface.geometryGuid = meshOwner;
        mesh.surface.baseType = API_MeshID;
        mesh.surface.snapshot.guid = meshOwner;
        mesh.surface.snapshot.status = RoomSurfaceStatus::Extracted;
        RoomSurfaceBody meshBody;
        meshBody.index = 1;
        meshBody.surfaces.Push (face (2, 0, 4, 2, -1, true));
        meshBody.surfaces.Push (face (2, 0, 4, 2, -3, false));
        mesh.surface.snapshot.bodies.Push (meshBody);

        const auto FloorCoverage = [] (const GS::Array<OtdSlab> &floors) {
            Geometry::MultiPolygon2D result;
            for (const auto &floor : floors)
                result.Push (floor.poly);
            result.Unify ();
            return result;
        };
        MixedFloorGeometry hidden;
        check (BuildFiniteMixedFloorGeometry (zone, sources, roles, 0, nominal, true, prepared, 0, {mesh}, hidden),
               "Slab winner inside finite Mesh domain is accepted");
        check (hidden.floors.GetSize () == 3 && std::fabs (hidden.meshDomain.CalcArea () - 4) <= AreaTolerance &&
                   ownerCoverage (hidden.floors, meshOwner).IsEmpty () &&
                   std::fabs (ownerCoverage (hidden.floors, slabOwner).CalcArea () - 12) <= AreaTolerance &&
                   hidden.changedDomain.IsEmpty (),
               "domain remains nonempty when every visible winner is Slab without "
               "duplicated nominal floor");

        mesh.surface.snapshot.bodies[0].surfaces[0] = face (2, 0, 4, 2, 1, true);
        MixedFloorGeometry visible;
        check (BuildFiniteMixedFloorGeometry (zone, sources, roles, 0, nominal, true, prepared, 0, {mesh}, visible),
               "central finite Mesh preserves both disconnected legacy remainders");
        check (visible.floors.GetSize () == 3 &&
                   std::fabs (ownerCoverage (visible.floors, slabOwner).CalcArea () - 8) <= AreaTolerance &&
                   std::fabs (ownerCoverage (visible.floors, meshOwner).CalcArea () - 4) <= AreaTolerance &&
                   std::fabs (FloorCoverage (visible.floors).CalcArea () - 12) <= AreaTolerance,
               "all outside-domain components survive rather than PopLargest");
        bool metadata = true;
        for (const auto &floor : visible.floors)
            if (floor.base_guid == slabOwner)
                metadata = metadata && floor.reference_z == 17 && floor.tip == base.tip &&
                           floor.favorite.name == base.favorite.name && floor.base_type == API_SlabID;
        check (metadata, "split remainder retains owner reference plane and Favorite metadata");

        auto holed = mesh;
        for (auto &surface : holed.surface.snapshot.bodies[0].surfaces) {
            auto hole = face (2.5, 0.5, 3.5, 1.5, surface.normalWorld.z > 0 ? 1 : -3, true).contours[0];
            hole.hole = true;
            surface.contours.Push (hole);
        }
        MixedFloorGeometry holes;
        check (BuildFiniteMixedFloorGeometry (zone, sources, roles, 0, nominal, true, prepared, 0, {holed}, holes),
               "finite Mesh hole is outside its admitted domain");
        check (std::fabs (holes.meshDomain.CalcArea () - 3) <= AreaTolerance && holes.floors.GetSize () == 4 &&
                   std::fabs (ownerCoverage (holes.floors, slabOwner).CalcArea () - 9) <= AreaTolerance &&
                   std::fabs (ownerCoverage (holes.floors, meshOwner).CalcArea () - 3) <= AreaTolerance,
               "hole interior remains legacy coverage with both exterior legacy "
               "components");
        bool meshHole = false;
        for (const auto &floor : holes.floors)
            if (floor.base_guid == meshOwner)
                meshHole = floor.poly.GetContourNum () == 2;
        check (meshHole, "Mesh winner preserves its actual hole contour");

        // Более высокая плита разрезает остаток; legacy сохраняет только правую
        // большую часть.
        const API_Guid upperOwner = APIGuidFromString ("00000000-0000-0000-0000-00000000A004");
        auto upperBase = base;
        upperBase.base_guid = upperOwner;
        upperBase.poly = rectangle (1, 0, 4, 2);
        upperBase.height = 3;
        auto upperSource = slabSource;
        upperSource.baseGuid = upperOwner;
        upperSource.geometryGuid = upperOwner;
        upperSource.snapshot.guid = upperOwner;
        auto leftMesh = mesh;
        leftMesh.surface.snapshot.bodies[0].surfaces[0] = face (0, 0, 1, 2, -1, true);
        leftMesh.surface.snapshot.bodies[0].surfaces[1] = face (0, 0, 1, 2, -3, false);
        GS::Array<RoomSurfaceSource> suppressedSources = sources;
        suppressedSources.Push (upperSource);
        GS::Array<OtdSlab> suppressedPrepared = prepared;
        suppressedPrepared.Push (upperBase);
        const GS::Array<API_Guid> suppressedRoles = {slabOwner, upperOwner};
        GS::Array<OtdSlab> lossyLegacy;
        double lossyRejected = 0;
        check (BuildProjectedSlabFinishes (zone,
                                           suppressedSources,
                                           suppressedRoles,
                                           Floor,
                                           0,
                                           lossyLegacy,
                                           lossyRejected,
                                           &nominal,
                                           true,
                                           nullptr,
                                           &suppressedPrepared) &&
                   std::fabs (FloorCoverage (lossyLegacy).CalcArea () - 10) <= AreaTolerance,
               "legacy winner reduction really discards the smaller disconnected Slab "
               "fragment");
        MixedFloorGeometry recovered;
        check (BuildFiniteMixedFloorGeometry (zone,
                                              suppressedSources,
                                              suppressedRoles,
                                              0,
                                              nominal,
                                              true,
                                              suppressedPrepared,
                                              0,
                                              {leftMesh},
                                              recovered) &&
                   std::fabs (FloorCoverage (recovered.floors).CalcArea () - 12) <= AreaTolerance &&
                   ownerCoverage (recovered.floors, meshOwner).IsEmpty () &&
                   std::fabs (ownerCoverage (recovered.floors, slabOwner).CalcArea () - 6) <= AreaTolerance,
               "raw role-qualified contact restores Slab winner inside domain despite "
               "lossy legacy result");
        auto emptyDomainMesh = mesh;
        emptyDomainMesh.hasBottom = false;
        emptyDomainMesh.surface.snapshot.bodies[0].surfaces = {emptyDomainMesh.surface.snapshot.bodies[0].surfaces[0]};
        MixedFloorGeometry emptyDomain;
        check (BuildFiniteMixedFloorGeometry (
                   zone, sources, roles, 0, nominal, true, prepared, 0, {emptyDomainMesh}, emptyDomain) &&
                   emptyDomain.meshDomain.IsEmpty () && emptyDomain.floors.GetSize () == 1 &&
                   std::fabs (FloorCoverage (emptyDomain.floors).CalcArea () - 12) <= AreaTolerance,
               "ineligible open Mesh delegates unchanged legacy floor for empty "
               "admitted domain");

        GS::Array<OtdSlab> original;
        GS::Array<OtdWall> originalSteps;
        double originalRejected = 0;
        const bool legacyOK = BuildProjectedSlabFinishes (
            zone, sources, roles, Floor, 0, original, originalRejected, &nominal, true, &originalSteps, &prepared);
        MixedFloorGeometry delegated;
        check (legacyOK && BuildFiniteMixedFloorGeometry (zone,
                                                          sources,
                                                          roles,
                                                          0,
                                                          nominal,
                                                          true,
                                                          prepared,
                                                          std::numeric_limits<double>::quiet_NaN (),
                                                          {},
                                                          delegated),
               "no Mesh delegates exactly without evaluating Mesh-only room-floor "
               "guards");
        check (delegated.floors.GetSize () == original.GetSize () &&
                   delegated.steps.GetSize () == originalSteps.GetSize () &&
                   delegated.rejectedArea == originalRejected &&
                   same (FloorCoverage (delegated.floors), FloorCoverage (original)) &&
                   delegated.meshDomain.IsEmpty () && delegated.changedDomain.IsEmpty (),
               "no Mesh retains legacy output order count rejected area and step count");

        MixedFloorGeometry sentinel = delegated;
        auto broken = mesh;
        broken.surface.snapshot.status = RoomSurfaceStatus::ReadError;
        check (
            !BuildFiniteMixedFloorGeometry (zone, sources, roles, 0, nominal, true, prepared, 0, {broken}, sentinel) &&
                same (FloorCoverage (sentinel.floors), FloorCoverage (delegated.floors)) &&
                sentinel.meshDomain.IsEmpty (),
            "invalid Mesh refuses atomically without publishing staged legacy "
            "fallback");
        auto inclined = mesh;
        auto &inclinedTop = inclined.surface.snapshot.bodies[0].surfaces[0];
        inclinedTop.normalWorld = {-1, 0, 1};
        inclinedTop.planeOffset = 3;
        for (auto &contour : inclinedTop.contours)
            for (auto &point : contour.vertices)
                point.world.z = point.world.x - 3;
        MixedFloorGeometry crossing;
        check (
            BuildFiniteMixedFloorGeometry (zone, sources, roles, 0, nominal, true, prepared, 0, {inclined}, crossing) &&
                std::fabs (ownerCoverage (crossing.floors, meshOwner).CalcArea () - 2) <= AreaTolerance &&
                std::fabs (ownerCoverage (crossing.floors, slabOwner).CalcArea () - 10) <= AreaTolerance,
            "local plane height splits Mesh Slab priority inside finite domain");
        bool inclinedMetadata = false;
        for (const auto &floor : crossing.floors)
            if (floor.base_guid == meshOwner)
                inclinedMetadata =
                    floor.slope_x == 1 && floor.slope_y == 0 && floor.draw_type == API_RoofID && floor.type == Floor &&
                    floor.surface_geometry &&
                    std::fabs (floor.zBottom + floor.slope_x * (3.5 - floor.plane_origin.x) - 0.5) <= 1.e-6;
        check (inclinedMetadata, "mixed winner retains actual Mesh plane native type and floor role");

        OtdSlab stepNominal = nominal;
        stepNominal.zBottom = 0;
        auto rightMesh = mesh;
        rightMesh.surface.snapshot.bodies[0].surfaces[0] = face (5, 0, 6, 2, 1, true);
        rightMesh.surface.snapshot.bodies[0].surfaces[1] = face (5, 0, 6, 2, -3, false);
        GS::Array<OtdSlab> stepLegacy;
        GS::Array<OtdWall> beforeSteps;
        double stepRejected = 0;
        const bool stepsBuilt = BuildProjectedSlabFinishes (zone,
                                                            suppressedSources,
                                                            suppressedRoles,
                                                            Floor,
                                                            0,
                                                            stepLegacy,
                                                            stepRejected,
                                                            &stepNominal,
                                                            true,
                                                            &beforeSteps,
                                                            &suppressedPrepared);
        MixedFloorGeometry independentSteps;
        const bool mixedStepsBuilt = BuildFiniteMixedFloorGeometry (zone,
                                                                    suppressedSources,
                                                                    suppressedRoles,
                                                                    0,
                                                                    stepNominal,
                                                                    true,
                                                                    suppressedPrepared,
                                                                    0,
                                                                    {rightMesh},
                                                                    independentSteps);
        check (stepsBuilt && mixedStepsBuilt && beforeSteps.GetSize () == 2 &&
                   independentSteps.steps.GetSize () == beforeSteps.GetSize (),
               "actual independent legacy Slab steps are retained in mixed geometry");
        bool stepsEqual = stepsBuilt && mixedStepsBuilt && beforeSteps.GetSize () == 2 &&
                          independentSteps.steps.GetSize () == beforeSteps.GetSize ();
        if (stepsEqual)
            for (UIndex i = 0; i < beforeSteps.GetSize (); ++i) {
                const auto &a = beforeSteps[i];
                const auto &c = independentSteps.steps[i];
                stepsEqual = stepsEqual && a.begC.x == c.begC.x && a.begC.y == c.begC.y && a.endC.x == c.endC.x &&
                             a.endC.y == c.endC.y && a.zBottom == c.zBottom && a.height == c.height &&
                             a.base_guid == c.base_guid && a.base_type == c.base_type && a.floorInd == c.floorInd &&
                             a.type == c.type && a.surface_geometry == c.surface_geometry &&
                             a.slab_step == c.slab_step && a.isValid == c.isValid &&
                             a.slab_step_writer_eligible == c.slab_step_writer_eligible;
            }
        check (stepsEqual, "independent Slab step order geometry owner and pending writer eligibility are unchanged");
    }
    return GS::ObjectState (
        "status", failed == 0 ? "Passed" : "Failed", "checks", checks, "failed", failed, "geometry", geometry);
}

#endif
