#include "ACAPinc.h"

#include "tests/TestFunc.hpp"

#if defined(TESTING)

    #include <algorithm>
    #include <cmath>
    #include <cstdlib>
    #include <cstring>
    #include <limits>
    #include <vector>

    #include "CommonFunction.hpp"
    #include "OnExit.hpp"
    #include "roombook/Roombook.hpp"
    #include "tests/TestKit.hpp"

    #ifdef ServerMainVers_2700
        #define SMSTF_INDEX(i) ACAPI_CreateAttributeIndex (i)
        #define SMSTF_IDX_LT(a, b) ((a).ToInt32_Deprecated () < (b).ToInt32_Deprecated ())
    #else
        #define SMSTF_INDEX(i) (i)
        #define SMSTF_IDX_LT(a, b) ((a) < (b))
    #endif

    // Тесты используют Favorite_* и Roombook-типы, объявленные только для AC >= 24.
    #if defined(ServerMainVers_2400)

namespace Roombook {
    extern MatarialToFavoriteDict favdict;
}

namespace TestFunc {
    void TestMeshFloorNative () {
        const char *filter = std::getenv ("SMSTF_TEST");
        if (filter == nullptr || std::strstr (filter, "TestMeshFloorNative") == nullptr)
            DBskip ("Native fixture requires an explicit suite name in SMSTF_TEST");
        using namespace Roombook;
        if (!IsTestProjectOpen ())
            DBskip ("Native Mesh floor checks require an explicit test project");
        const char *caseName = "settings";
        const auto check = [&] (bool passed, const char *name) {
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
                    const bool identityMatches = readError == NoError &&
                                                 current.header.guid == ownedLayer.header.guid &&
                                                 current.header.index != SMSTF_INDEX (1);
                    check (identityMatches, "cleanup verifies owned layer GUID and nonprotected index");
                    if (identityMatches) {
        #ifdef ServerMainVers_2600
                        const GSErrCode deleteError = ACAPI_Attribute_Delete (current.header);
        #else
                        const GSErrCode deleteError = ACAPI_Attribute_Delete (&current.header);
        #endif
                        check (deleteError == NoError, "cleanup deletes only the new owned layer");
                        API_Attr_Head lookup = {};
                        lookup.typeID = API_LayerID;
                        lookup.uniStringNamePtr = &layerName;
                        check (ACAPI_Attribute_Search (&lookup) == APIERR_BADNAME,
                               "cleanup owned layer absent by name");
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
                layerOwned = ownedLayer.header.index != SMSTF_INDEX (1);
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
                                      entry->type == API_SlabID && !entry->is_composite_read &&
                                      entry->composite.IsEmpty ();
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
                check (entry != nullptr && entry->composite.IsEmpty (),
                       "missing request publishes no material composite");
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
                    const bool invalidLayerArmed = err == NoError &&
                                                   invalidBefore.header.guid == ownedLayer.header.guid &&
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
                    check (readback != nullptr && readback->fromMaterial &&
                               readback->rawName == invalidRequest.rawName && !readback->isValid &&
                               readback->val.uniStringValue.IsEmpty (),
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
                std::sort (
                    band.materials.begin (),
                    band.materials.end (),
                    [] (const API_AttributeIndex &a, const API_AttributeIndex &b) { return SMSTF_IDX_LT (a, b); });
                band.materials.erase (std::unique (band.materials.begin (), band.materials.end ()),
                                      band.materials.end ());
                bands.Push (band);
            }
            return bands;
        };
        const GS::Array<Point2D> cutFixtureCoords = {{0, 0}, {2, 0}, {2, 2}, {0, 2}};
        const auto cutFixture = Geometry::Polygon2D::Create (cutFixtureCoords, 0);
        DBrequire (cutFixture.GetSize () == 1, "Cut fixture contains one polygon");
        bool fullyInside = false;
        bool degenerate = false;
        GS::Int32 warning = 0;
        const auto leftHalf = cutFixture[0].Cut (HalfPlane2DData (1, 0, 1), &fullyInside, &warning, &degenerate);
        check (leftHalf.GetSize () == 1 && std::fabs (leftHalf[0].CalcArea () - 2) < 1e-6 && !fullyInside &&
                   warning == 0 && !degenerate,
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
                    if (floor.draw.base_guid == owner)
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
            check (MeshFloorFinishes_BuildProjected (room, 0, 0, {}, {}, floors, domain) && floors.IsEmpty () &&
                       domain.IsEmpty (),
                   "no Mesh leaves the legacy domain untouched");

            auto source = mesh (ownerA, 0, 4, -2, 1, true);
            check (MeshFloorFinishes_BuildProjected (room, 0, 0, {source}, {}, floors, domain),
                   "solid sloped Mesh is admitted by its bottom");
            check (floors.GetSize () == 1 && sameCoverage (domain, room), "finite sloped Mesh covers the room exactly");
            check (floors.GetSize () == 1 && floors[0].draw.base_guid == ownerA &&
                       floors[0].draw.base_type == API_MeshID && floors[0].draw.draw_type == API_RoofID &&
                       floors[0].draw.type == Floor && floors[0].surface_geometry &&
                       std::fabs (floors[0].slope_x - 1) < 1e-6 &&
                       std::fabs (floors[0].zBottom - floors[0].plane_origin.x + 2) < 1e-6,
                   "Mesh output retains owner type and the actual contact plane");

            floors.Clear ();
            source.hasBottom = false;
            source.surface.snapshot.bodies[0].surfaces = {source.surface.snapshot.bodies[0].surfaces[0]};
            check (MeshFloorFinishes_BuildProjected (room, 0, 0, {source}, {}, floors, domain),
                   "Mesh without bottom uses its local top");
            check (sameCoverage (domain, rectangle (0, 0, 2, 2)),
                   "Mesh without bottom clips eligibility locally not by global Z");

            floors.Clear ();
            auto high = mesh (ownerA, 0, 4, 1, 0, false);
            check (MeshFloorFinishes_BuildProjected (room, 0, 0, {high}, {}, floors, domain) && floors.IsEmpty () &&
                       domain.IsEmpty (),
                   "Mesh without bottom above the floor does not replace the legacy domain");
            auto raisedBottom = mesh (ownerA, 0, 4, 2, 0, true);
            auto &bottomFace = raisedBottom.surface.snapshot.bodies[0].surfaces[1];
            for (auto &vertex : bottomFace.contours[0].vertices)
                vertex.world.z = 1;
            bottomFace.planeOffset = 1;
            check (MeshFloorFinishes_BuildProjected (room, 0, 0, {raisedBottom}, {}, floors, domain) &&
                       floors.IsEmpty () && domain.IsEmpty (),
                   "solid Mesh above the floor is ineligible despite a valid top");

            floors.Clear ();
            source = mesh (ownerA, 0, 4, -2, 1, true);
            OtdSlab support;
            support.poly = room;
            support.draw.base_guid = ownerB;
            support.draw.base_type = API_SlabID;
            support.draw.type = Floor;
            support.surface_geometry = true;
            check (MeshFloorFinishes_BuildProjected (room, 0, 0, {source}, {support}, floors, domain),
                   "crossing Mesh and Slab heights are not an ambiguous area");
            check (sameCoverage (coverageFor (floors, ownerB), rectangle (0, 0, 2, 2)) &&
                       sameCoverage (coverageFor (floors, ownerA), rectangle (2, 0, 4, 2)),
                   "Mesh Slab priority changes at the actual equality line");
            check (floors.GetSize () == 2, "crossing Mesh Slab emits no duplicate coverage");

            floors.Clear ();
            auto other = mesh (ownerB, 0, 4, 2, -1, true);
            check (MeshFloorFinishes_BuildProjected (room, 0, 0, {source, other}, {}, floors, domain),
                   "two crossing Mesh planes are locally ordered");
            check (sameCoverage (coverageFor (floors, ownerB), rectangle (0, 0, 2, 2)) &&
                       sameCoverage (coverageFor (floors, ownerA), rectangle (2, 0, 4, 2)),
                   "different slopes retain both finite upper cells");
            const auto referenceA = coverageFor (floors, ownerA);
            const auto referenceB = coverageFor (floors, ownerB);
            floors.Clear ();
            check (
                MeshFloorFinishes_BuildProjected (room, 0, 0, {other, source}, {}, floors, domain) &&
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
            check (!MeshFloorFinishes_BuildProjected (room, 0, 0, {flatA, flatB}, {}, floors, domain),
                   "coincident different owners reject instead of GUID tiebreak");
            check (floors.GetSize () == 1 && floors[0].draw.base_guid == ownerB && sameCoverage (domain, room),
                   "rejection leaves every output unchanged");
            floors.Clear ();
            high = mesh (ownerC, 0, 4, 1, 0, true);
            check (MeshFloorFinishes_BuildProjected (room, 0, 0, {flatA, flatB, high}, {}, floors, domain) &&
                       sameCoverage (coverageFor (floors, ownerC), room) && floors.GetSize () == 1,
                   "higher third source hides a lower ownership tie");

            floors.Clear ();
            auto left = mesh (ownerA, 0, 2, -1, 0.2, true);
            auto right = mesh (ownerA, 2, 4, -1, 0.2, true);
            left.surface.snapshot.bodies[0].surfaces.Append (right.surface.snapshot.bodies[0].surfaces);
            check (MeshFloorFinishes_BuildProjected (room, 0, 0, {left}, {}, floors, domain) &&
                       floors.GetSize () == 1 && sameCoverage (domain, room),
                   "adjacent coplanar same owner patches merge to one plane");
            floors.Clear ();
            left = mesh (ownerA, 0, 1, -1, 0, true);
            right = mesh (ownerA, 3, 4, -1, 0, true);
            left.surface.snapshot.bodies[0].surfaces.Append (right.surface.snapshot.bodies[0].surfaces);
            check (MeshFloorFinishes_BuildProjected (room, 0, 0, {left}, {}, floors, domain) &&
                       floors.GetSize () == 2 && std::fabs (domain.CalcArea () - 4) < 1e-5,
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
            check (MeshFloorFinishes_BuildProjected (room, 0, 0, {holed}, {}, floors, domain) &&
                       std::fabs (domain.CalcArea () - 6) < 1e-5 && floors.GetSize () == 1 &&
                       floors[0].poly.GetContourNum () == 2,
                   "Mesh holes remain uncovered");
            floors.Clear ();
            check (MeshFloorFinishes_BuildProjected (rectangle (0, 0, 2, 2), 0, 0, {holed}, {}, floors, domain) &&
                       std::fabs (domain.CalcArea () - 3) < 1e-5 && floors.GetSize () == 1 &&
                       floors[0].poly.GetContourNum () == 1,
                   "partial clipping opens a hole into the exterior correctly");

            floors.Clear ();
            auto broken = flatA;
            broken.surface.snapshot.status = RoomSurfaceStatus::ReadError;
            check (!MeshFloorFinishes_BuildProjected (room, 0, 0, {broken}, {}, floors, domain) && floors.IsEmpty (),
                   "incomplete Mesh snapshot refuses nominal replacement");
            broken = flatA;
            broken.surface.snapshot.bodies[0].surfaces[0].contours[0].vertices[2].world.z += 0.01;
            check (!MeshFloorFinishes_BuildProjected (room, 0, 0, {broken}, {}, floors, domain),
                   "nonplanar top face is rejected before writing");
            broken = flatA;
            broken.surface.snapshot.guid = ownerB;
            check (!MeshFloorFinishes_BuildProjected (room, 0, 0, {broken}, {}, floors, domain),
                   "snapshot owner mismatch is rejected");

            {
                caseName = "finite Mesh provenance";
                floors.Clear ();
                MeshFloorProvenance provenance;
                const auto partial = rectangle (0, 0, 2, 2);
                check (MeshFloorFinishes_BuildProjectedWithProvenance (
                           partial, 0, 0, {holed}, {}, floors, domain, provenance),
                       "finite source projection is captured before room clipping");
                check (provenance.supports.GetSize () == 1 &&
                           std::fabs (provenance.supports[0].projected.CalcArea () - 6) < 1e-5 &&
                           std::fabs (provenance.supports[0].admitted.CalcArea () - 6) < 1e-5 &&
                           provenance.supports[0].projected[0].GetContourNum () == 2,
                       "source hole survives outside the partial room");
                check (provenance.contributions.GetSize () == 1 && provenance.contributions[0].floorIndex == 0 &&
                           provenance.contributions[0].supportIndex == 0 &&
                           std::fabs (provenance.contributions[0].domain.CalcArea () - 3) < 1e-5 &&
                           provenance.contributions[0].domain[0].GetContourNum () == 1,
                       "winner contribution opens the source hole at the room boundary");
                check (provenance.supports.GetSize () == 1 && provenance.supports[0].ownerGuid == ownerA &&
                           provenance.supports[0].geometryGuid == ownerA &&
                           provenance.supports[0].ownerType == API_MeshID && provenance.supports[0].bodyIndex == 1 &&
                           provenance.supports[0].polygonIndex == 1,
                       "finite support keeps exact source ownership and face indexes");

                floors.Clear ();
                provenance = MeshFloorProvenance{};
                auto adjacent = mesh (ownerA, 0, 2, -1, 0, false);
                auto neighbour = mesh (ownerA, 2, 4, -1, 0, false);
                neighbour.surface.snapshot.bodies[0].surfaces[0].polygonIndex = 7;
                adjacent.surface.snapshot.bodies[0].surfaces.Append (neighbour.surface.snapshot.bodies[0].surfaces);
                check (MeshFloorFinishes_BuildProjectedWithProvenance (
                           room, 0, 0, {adjacent}, {}, floors, domain, provenance) &&
                           floors.GetSize () == 1 && provenance.supports.GetSize () == 2 &&
                           provenance.contributions.GetSize () == 2,
                       "coplanar merge retains both original face references");
                bool exactContributions =
                    provenance.supports.GetSize () == 2 && provenance.contributions.GetSize () == 2;
                for (const auto &contribution : provenance.contributions) {
                    exactContributions = exactContributions &&
                                         contribution.supportIndex < provenance.supports.GetSize () &&
                                         contribution.floorIndex == 0;
                    if (!exactContributions)
                        break;
                    const bool first = provenance.supports[contribution.supportIndex].polygonIndex == 1;
                    exactContributions =
                        sameCoverage (contribution.domain, first ? rectangle (0, 0, 2, 2) : rectangle (2, 0, 4, 2));
                }
                check (exactContributions, "each coplanar contribution stops at its own finite source");
                check (MeshFloorFinishes_BuildProjectedWithProvenance (
                           room, 0, 0, {adjacent}, {}, floors, domain, provenance) &&
                           floors.GetSize () == 2 && provenance.supports.GetSize () == 4 &&
                           provenance.contributions.GetSize () == 4 && provenance.contributions[2].floorIndex == 1 &&
                           provenance.contributions[2].supportIndex >= 2,
                       "appended provenance indexes retain the previous floor prefix");
                check (!MeshFloorFinishes_BuildProjectedWithProvenance (
                           room, 0, 0, {flatA, flatB}, {}, floors, domain, provenance) &&
                           floors.GetSize () == 2 && provenance.supports.GetSize () == 4 &&
                           provenance.contributions.GetSize () == 4 && sameCoverage (domain, room),
                       "late ambiguity preserves existing floors domain and provenance");

                floors.Clear ();
                provenance = MeshFloorProvenance{};
                check (MeshFloorFinishes_BuildProjectedWithProvenance (
                           room, 0, 0, {holed}, {support}, floors, domain, provenance) &&
                           provenance.supports.GetSize () == 1 && provenance.contributions.IsEmpty () &&
                           floors.GetSize () == 1 && floors[0].draw.base_guid == ownerB,
                       "hidden Mesh keeps its source but has no winner contribution");
            }
            caseName = "settings";
        }
        const Stories stories = GetStories ();
        DBrequire (!stories.IsEmpty (), "Native Mesh floor fixture has a story");

        API_Element settings = {};
        SetElemTypeID (settings, API_SlabID);
        GSErrCode error = ACAPI_Element_GetDefaults (&settings, nullptr);
        check (error == NoError, "read slab settings");
        if (error != NoError)
            return;
        settings.header.layer = SMSTF_INDEX (1);
        settings.slab.modelElemStructureType = API_BasicStructure;
        settings.slab.thickness = 0.08;

        API_Element compositeSettings = settings;
        bool hasComposite = false;
        #ifdef ServerMainVers_2700
        GS::UInt32 count = 0;
        error = ACAPI_Attribute_GetNum (API_CompWallID, count);
        #else
        API_AttributeIndex count = 0;
        error = ACAPI_Attribute_GetNum (API_CompWallID, &count);
        #endif
        check (error == NoError, "enumerate composites");
        if (error == NoError) {
        #ifdef ServerMainVers_2700
            for (GS::UInt32 index = 1; index <= count; ++index) {
        #else
            for (API_AttributeIndex index = 1; index <= count; ++index) {
        #endif
                API_Attribute attribute = {};
                attribute.header.typeID = API_CompWallID;
                attribute.header.index = SMSTF_INDEX (index);
                if (ACAPI_Attribute_Get (&attribute) != NoError || attribute.compWall.nComps < 2 ||
                    (attribute.header.flags & APICWall_ForRoof) == 0 ||
                    (attribute.header.flags & APICWall_ForSlab) == 0)
                    continue;
                compositeSettings.slab.modelElemStructureType = API_CompositeStructure;
                compositeSettings.slab.composite = SMSTF_INDEX (index);
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
                mesh.header.layer = SMSTF_INDEX (1);
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
                memo.coords =
                    reinterpret_cast<API_Coord **> (BMAllocateHandle (6 * sizeof (API_Coord), ALLOCATE_CLEAR, 0));
                memo.pends = reinterpret_cast<Int32 **> (BMAllocateHandle (2 * sizeof (Int32), ALLOCATE_CLEAR, 0));
                memo.meshPolyZ =
                    reinterpret_cast<double **> (BMAllocateHandle (6 * sizeof (double), ALLOCATE_CLEAR, 0));
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
                const auto snapshot = RoomSurfaces_Read (mesh.header);
                check (snapshot.IsComplete (), "read native mesh world faces");
                if (scenario.hasCenter) {
                    bool hasActualCenter = false;
                    for (const auto &body : snapshot.bodies)
                        for (const auto &face : body.surfaces)
                            for (const auto &contour : face.contours)
                                for (const auto &vertex : contour.vertices) {
                                    const auto &v = vertex.world;
                                    hasActualCenter =
                                        hasActualCenter || (std::hypot (std::hypot (v.x - 1001, v.y - 1001),
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
                            check (source.surface.baseType == API_MeshID &&
                                       source.surface.geometryGuid == mesh.header.guid,
                                   "typed discovery retains native mesh type and GUID");
                            check (source.hasBottom == (scenario.skirt == 1),
                                   "typed discovery distinguishes solid and open mesh");
                            check (source.surface.snapshot.status == RoomSurfaceStatus::NotRead,
                                   "typed discovery defers native body read");
                            source.surface.snapshot = snapshot;
                            GS::Array<OtdSlab> projected;
                            Geometry::MultiPolygon2D coverage;
                            check (
                                MeshFloorFinishes_BuildProjected (
                                    discoveryFootprints[0], 1, mesh.header.floorInd, {source}, {}, projected, coverage),
                                "real native mesh reaches projected floor kernel");
                            Geometry::MultiPolygon2D expected;
                            expected.Push (discoveryFootprints[0]);
                            const auto missing = Geometry::MultiPolygon2D::Substract (expected, coverage);
                            const auto outside = Geometry::MultiPolygon2D::Substract (coverage, expected);
                            bool typed = !projected.IsEmpty ();
                            for (const auto &floor : projected)
                                typed = typed && floor.draw.base_guid == mesh.header.guid &&
                                        floor.draw.base_type == API_MeshID && floor.draw.draw_type == API_RoofID;
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
                        if (face.normalWorld.z <= 1e-6 || face.contours.IsEmpty () ||
                            face.contours[0].vertices.IsEmpty ())
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
                            floor.draw.type = Floor;
                            floor.draw.base_type = API_MeshID;
                            floor.draw.base_guid = mesh.header.guid;
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
                                check (clipped.GetSize () == 1,
                                       "room boundary clips finite support and opens hole notch");
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
                            floor.draw.otd_guid = APINULLGuid;
                            const GSErrCode createError = Roof_Draw (stories, floor, slabSettings, &floors);
                            check (createError == NoError && floor.draw.otd_guid != APINULLGuid,
                                   mode == 0 ? "create basic roof on native mesh plane"
                                             : "create composite roof on native mesh plane");
                            if (createError == NoError)
                                created.Push (floor.draw.otd_guid);
                        }
                        return NoError;
                    });
                    check (error == NoError, "roof undo transaction");
                    if (mode != 0) {
                        API_Elem_Head referenceHeader = {};
                        referenceHeader.guid = referenceGuid;
                        const auto reference = RoomSurfaces_Read (referenceHeader);
                        OtdSlab support;
                        support.zBottom = 0;
                        support.plane_origin = {1010, 1010};
                        referenceBands = layerBands (reference, support);
                        check (referenceGuid != APINULLGuid && reference.IsComplete () && !referenceBands.IsEmpty (),
                               "read independent slab layer bands and materials");
                    }
                    for (const auto &floor : floors) {
                        if (floor.draw.otd_guid == APINULLGuid)
                            continue;
                        API_Element native = {};
                        native.header.guid = floor.draw.otd_guid;
                        const GSErrCode readError = ACAPI_Element_Get (&native);
                        check (readError == NoError && GetElemTypeID (native) == API_RoofID,
                               "read actual native roof type");
                        const auto roof = RoomSurfaces_Read (native.header);
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
                                    actualContact.Append (Geometry::Polygon2D::Create (
                                        contactCoords,
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
                        check (std::isfinite (minimum) && std::fabs (minimum) < 1e-6,
                               "roof bottom contacts mesh plane");
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
                                            if (std::fabs (n.x * support.x + n.y * support.y + n.z * support.z) >
                                                1 - 1e-6)
                                                continue;
                                            if (pointOnly && std::fabs (n.x * difference.x + n.y * difference.y +
                                                                        n.z * difference.z) /
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
                    if (floor.draw.base_guid == owner)
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
            base.draw.base_guid = slabOwner;
            base.draw.base_type = API_SlabID;
            base.zBottom = -1;
            base.height = 1;
            base.reference_z = 17;
            base.tip = "retained base metadata";
            base.draw.favorite.name = "retained Favorite metadata";
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
            nominal.draw.base_guid = zoneOwner;
            nominal.draw.base_type = API_ZoneID;
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
            check (
                OtdSlab_BuildFiniteMixedGeometry (zone, sources, roles, 0, nominal, true, prepared, 0, {mesh}, hidden),
                "Slab winner inside finite Mesh domain is accepted");
            check (hidden.floors.GetSize () == 3 && std::fabs (hidden.meshDomain.CalcArea () - 4) <= AreaTolerance &&
                       ownerCoverage (hidden.floors, meshOwner).IsEmpty () &&
                       std::fabs (ownerCoverage (hidden.floors, slabOwner).CalcArea () - 12) <= AreaTolerance &&
                       hidden.changedDomain.IsEmpty (),
                   "domain remains nonempty when every visible winner is Slab without "
                   "duplicated nominal floor");

            mesh.surface.snapshot.bodies[0].surfaces[0] = face (2, 0, 4, 2, 1, true);
            MixedFloorGeometry visible;
            check (
                OtdSlab_BuildFiniteMixedGeometry (zone, sources, roles, 0, nominal, true, prepared, 0, {mesh}, visible),
                "central finite Mesh preserves both disconnected legacy remainders");
            check (visible.floors.GetSize () == 3 &&
                       std::fabs (ownerCoverage (visible.floors, slabOwner).CalcArea () - 8) <= AreaTolerance &&
                       std::fabs (ownerCoverage (visible.floors, meshOwner).CalcArea () - 4) <= AreaTolerance &&
                       std::fabs (FloorCoverage (visible.floors).CalcArea () - 12) <= AreaTolerance,
                   "all outside-domain components survive rather than PopLargest");
            bool metadata = true;
            for (const auto &floor : visible.floors)
                if (floor.draw.base_guid == slabOwner)
                    metadata = metadata && floor.reference_z == 17 && floor.tip == base.tip &&
                               floor.draw.favorite.name == base.draw.favorite.name &&
                               floor.draw.base_type == API_SlabID;
            check (metadata, "split remainder retains owner reference plane and Favorite metadata");

            auto holed = mesh;
            for (auto &surface : holed.surface.snapshot.bodies[0].surfaces) {
                auto hole = face (2.5, 0.5, 3.5, 1.5, surface.normalWorld.z > 0 ? 1 : -3, true).contours[0];
                hole.hole = true;
                surface.contours.Push (hole);
            }
            MixedFloorGeometry holes;
            check (
                OtdSlab_BuildFiniteMixedGeometry (zone, sources, roles, 0, nominal, true, prepared, 0, {holed}, holes),
                "finite Mesh hole is outside its admitted domain");
            check (std::fabs (holes.meshDomain.CalcArea () - 3) <= AreaTolerance && holes.floors.GetSize () == 4 &&
                       std::fabs (ownerCoverage (holes.floors, slabOwner).CalcArea () - 9) <= AreaTolerance &&
                       std::fabs (ownerCoverage (holes.floors, meshOwner).CalcArea () - 3) <= AreaTolerance,
                   "hole interior remains legacy coverage with both exterior legacy "
                   "components");
            bool meshHole = false;
            for (const auto &floor : holes.floors)
                if (floor.draw.base_guid == meshOwner)
                    meshHole = floor.poly.GetContourNum () == 2;
            check (meshHole, "Mesh winner preserves its actual hole contour");

            // Более высокая плита разрезает остаток; legacy сохраняет только правую
            // большую часть.
            const API_Guid upperOwner = APIGuidFromString ("00000000-0000-0000-0000-00000000A004");
            auto upperBase = base;
            upperBase.draw.base_guid = upperOwner;
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
            check (OtdSlab_BuildProjectedFinishes (zone,
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
            check (OtdSlab_BuildFiniteMixedGeometry (zone,
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
            emptyDomainMesh.surface.snapshot.bodies[0].surfaces = {
                emptyDomainMesh.surface.snapshot.bodies[0].surfaces[0]};
            MixedFloorGeometry emptyDomain;
            check (OtdSlab_BuildFiniteMixedGeometry (
                       zone, sources, roles, 0, nominal, true, prepared, 0, {emptyDomainMesh}, emptyDomain) &&
                       emptyDomain.meshDomain.IsEmpty () && emptyDomain.floors.GetSize () == 1 &&
                       std::fabs (FloorCoverage (emptyDomain.floors).CalcArea () - 12) <= AreaTolerance,
                   "ineligible open Mesh delegates unchanged legacy floor for empty "
                   "admitted domain");

            GS::Array<OtdSlab> original;
            GS::Array<OtdWall> originalSteps;
            double originalRejected = 0;
            const bool legacyOK = OtdSlab_BuildProjectedFinishes (
                zone, sources, roles, Floor, 0, original, originalRejected, &nominal, true, &originalSteps, &prepared);
            MixedFloorGeometry delegated;
            check (legacyOK && OtdSlab_BuildFiniteMixedGeometry (zone,
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
            check (!OtdSlab_BuildFiniteMixedGeometry (
                       zone, sources, roles, 0, nominal, true, prepared, 0, {broken}, sentinel) &&
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
            check (OtdSlab_BuildFiniteMixedGeometry (
                       zone, sources, roles, 0, nominal, true, prepared, 0, {inclined}, crossing) &&
                       std::fabs (ownerCoverage (crossing.floors, meshOwner).CalcArea () - 2) <= AreaTolerance &&
                       std::fabs (ownerCoverage (crossing.floors, slabOwner).CalcArea () - 10) <= AreaTolerance,
                   "local plane height splits Mesh Slab priority inside finite domain");
            bool inclinedMetadata = false;
            for (const auto &floor : crossing.floors)
                if (floor.draw.base_guid == meshOwner)
                    inclinedMetadata =
                        floor.slope_x == 1 && floor.slope_y == 0 && floor.draw.draw_type == API_RoofID &&
                        floor.draw.type == Floor && floor.surface_geometry &&
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
            const bool stepsBuilt = OtdSlab_BuildProjectedFinishes (zone,
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
            const bool mixedStepsBuilt = OtdSlab_BuildFiniteMixedGeometry (zone,
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
                    stepsEqual = stepsEqual && a.geom.seg.begC.x == c.geom.seg.begC.x &&
                                 a.geom.seg.begC.y == c.geom.seg.begC.y && a.geom.seg.endC.x == c.geom.seg.endC.x &&
                                 a.geom.seg.endC.y == c.geom.seg.endC.y &&
                                 OtdLinearSegment_GetZBottom (a.geom) == OtdLinearSegment_GetZBottom (c.geom) &&
                                 a.geom.height == c.geom.height && a.draw.base_guid == c.draw.base_guid &&
                                 a.draw.base_type == c.draw.base_type && a.geom.floorInd == c.geom.floorInd &&
                                 a.draw.type == c.draw.type && a.surface_geometry == c.surface_geometry &&
                                 a.slab_step == c.slab_step && a.isValid == c.isValid &&
                                 a.slab_step_writer_eligible == c.slab_step_writer_eligible;
                }

            // #271-4c-c next: общий provenance, без активации production Mesh.
            MixedFloorGeometry traced;
            MixedFloorProvenance trace;
            check (OtdSlab_BuildFiniteMixedGeometryWithProvenance (
                       zone, sources, roles, 0, nominal, true, prepared, 0, {mesh}, traced, trace),
                   "mixed provenance builds visible finite Mesh and both legacy components");
            const auto contributionArea = [] (const auto &contributions) {
                double area = 0;
                for (const auto &c : contributions)
                    area += c.domain.CalcArea ();
                return area;
            };
            check (traced.floors.GetSize () == 3 && trace.slab.supports.GetSize () == 1 &&
                       trace.slab.supports[0].origin == RoomSurfaceOrigin::PreparedAnalytic &&
                       trace.slab.supports[0].preparedIndex == 0 && trace.slab.supports[0].bodyIndex == 0 &&
                       trace.slab.supports[0].polygonIndex == 0 &&
                       same (trace.slab.supports[0].projected, trace.slab.supports[0].admitted) &&
                       std::fabs (trace.slab.supports[0].projected.CalcArea () - 12) <= AreaTolerance &&
                       std::fabs (contributionArea (trace.slab.contributions) - 8) <= AreaTolerance &&
                       std::fabs (contributionArea (trace.mesh.contributions) - 4) <= AreaTolerance,
                   "Slab original admission 12 winner 8 and Mesh winner 4 are independent");
            const auto refsValid = [&] (const MixedFloorGeometry &geometry, const MixedFloorProvenance &p) {
                bool valid = true;
                const auto checkRefs = [&] (const auto &supports, const auto &contributions) {
                    for (const auto &c : contributions) {
                        if (c.supportIndex >= supports.GetSize () || c.floorIndex >= geometry.floors.GetSize ()) {
                            valid = false;
                            continue;
                        }
                        const auto &s = supports[c.supportIndex];
                        const auto &f = geometry.floors[c.floorIndex];
                        Geometry::MultiPolygon2D winner;
                        winner.Push (f.poly);
                        valid = valid && s.ownerGuid == f.draw.base_guid && s.ownerType == f.draw.base_type &&
                                same (c.domain, Geometry::MultiPolygon2D::Intersect (s.admitted, winner));
                    }
                };
                checkRefs (p.mesh.supports, p.mesh.contributions);
                checkRefs (p.slab.supports, p.slab.contributions);
                return valid;
            };
            check (refsValid (traced, trace), "all mixed references point at actual owner type and finite winner");
            const UIndex floorPrefix = traced.floors.GetSize ();
            const UIndex meshSupportPrefix = trace.mesh.supports.GetSize ();
            const UIndex slabSupportPrefix = trace.slab.supports.GetSize ();
            const UIndex meshContributionPrefix = trace.mesh.contributions.GetSize ();
            const UIndex slabContributionPrefix = trace.slab.contributions.GetSize ();
            check (OtdSlab_BuildFiniteMixedGeometryWithProvenance (
                       zone, sources, roles, 0, nominal, true, prepared, 0, {mesh}, traced, trace) &&
                       traced.floors.GetSize () == 2 * floorPrefix && refsValid (traced, trace),
                   "second mixed append offsets full floor destination rather than localInside");
            bool offsets = true;
            for (UIndex i = meshContributionPrefix; i < trace.mesh.contributions.GetSize (); ++i)
                offsets = offsets && trace.mesh.contributions[i].floorIndex >= floorPrefix &&
                          trace.mesh.contributions[i].supportIndex >= meshSupportPrefix;
            for (UIndex i = slabContributionPrefix; i < trace.slab.contributions.GetSize (); ++i)
                offsets = offsets && trace.slab.contributions[i].floorIndex >= floorPrefix &&
                          trace.slab.contributions[i].supportIndex >= slabSupportPrefix;
            check (offsets, "Mesh Slab support prefixes and final floor prefixes remap independently");
            const auto floorCount = traced.floors.GetSize ();
            const auto meshCount = trace.mesh.supports.GetSize ();
            const auto slabCount = trace.slab.supports.GetSize ();
            check (!OtdSlab_BuildFiniteMixedGeometryWithProvenance (
                       zone, sources, roles, 0, nominal, true, prepared, 0, {broken}, traced, trace) &&
                       traced.floors.GetSize () == floorCount && trace.mesh.supports.GetSize () == meshCount &&
                       trace.slab.supports.GetSize () == slabCount && refsValid (traced, trace),
                   "late invalid Mesh leaves mixed append destination and provenance prefixes intact");

            MixedFloorGeometry hiddenTrace, holesTrace, recoveredTrace, noMeshTrace;
            MixedFloorProvenance hiddenP, holesP, recoveredP, noMeshP;
            auto hiddenMesh = mesh;
            hiddenMesh.surface.snapshot.bodies[0].surfaces[0] = face (2, 0, 4, 2, -1, true);
            check (OtdSlab_BuildFiniteMixedGeometryWithProvenance (
                       zone, sources, roles, 0, nominal, true, prepared, 0, {hiddenMesh}, hiddenTrace, hiddenP) &&
                       refsValid (hiddenTrace, hiddenP) && hiddenP.mesh.contributions.IsEmpty () &&
                       !hiddenP.mesh.supports.IsEmpty () &&
                       std::fabs (contributionArea (hiddenP.slab.contributions) - 12) <= AreaTolerance,
                   "fully hidden Mesh retains support but never claims Slab winner");
            check (OtdSlab_BuildFiniteMixedGeometryWithProvenance (
                       zone, sources, roles, 0, nominal, true, prepared, 0, {holed}, holesTrace, holesP) &&
                       refsValid (holesTrace, holesP) &&
                       std::fabs (contributionArea (holesP.mesh.contributions) - 3) <= AreaTolerance &&
                       std::fabs (contributionArea (holesP.slab.contributions) - 9) <= AreaTolerance,
                   "hole interior and both exterior Slab remnants have final winner indices");
            check (OtdSlab_BuildFiniteMixedGeometryWithProvenance (zone,
                                                                   suppressedSources,
                                                                   suppressedRoles,
                                                                   0,
                                                                   nominal,
                                                                   true,
                                                                   suppressedPrepared,
                                                                   0,
                                                                   {leftMesh},
                                                                   recoveredTrace,
                                                                   recoveredP) &&
                       refsValid (recoveredTrace, recoveredP) && recoveredP.slab.supports.GetSize () == 2 &&
                       std::fabs (contributionArea (recoveredP.slab.contributions) - 12) <= AreaTolerance &&
                       recoveredP.mesh.contributions.IsEmpty (),
                   "raw Slab provenance includes restored fragment absent in lossy legacy");
            check (OtdSlab_BuildFiniteMixedGeometryWithProvenance (zone,
                                                                   sources,
                                                                   roles,
                                                                   0,
                                                                   nominal,
                                                                   true,
                                                                   prepared,
                                                                   std::numeric_limits<double>::quiet_NaN (),
                                                                   {},
                                                                   noMeshTrace,
                                                                   noMeshP) &&
                       refsValid (noMeshTrace, noMeshP) && noMeshP.mesh.supports.IsEmpty () &&
                       std::fabs (contributionArea (noMeshP.slab.contributions) - 12) <= AreaTolerance,
                   "provenance no Mesh leaves Mesh roomFloor guard unevaluated");
            auto larger = base;
            larger.poly = rectangle (-1, -1, 7, 3);
            MixedFloorGeometry unroomedTrace;
            MixedFloorProvenance unroomedP;
            check (OtdSlab_BuildFiniteMixedGeometryWithProvenance (
                       zone, sources, roles, 0, nominal, true, {larger}, 0, {mesh}, unroomedTrace, unroomedP) &&
                       refsValid (unroomedTrace, unroomedP) && unroomedP.slab.supports.GetSize () == 1 &&
                       std::fabs (unroomedP.slab.supports[0].projected.CalcArea () - 32) <= AreaTolerance &&
                       same (unroomedP.slab.supports[0].projected, unroomedP.slab.supports[0].admitted) &&
                       std::fabs (contributionArea (unroomedP.slab.contributions) - 8) <= AreaTolerance,
                   "original and admitted Slab support remain unroomed outside final winners");
            auto nativeSource = slabSource;
            nativeSource.origin = RoomSurfaceOrigin::Native3D;
            RoomSurfaceBody nativeBody;
            nativeBody.index = 7;
            auto nativeTop = face (0, 0, 6, 2, 0, true);
            nativeTop.bodyIndex = 7;
            nativeTop.polygonIndex = 11;
            nativeBody.surfaces.Push (nativeTop);
            nativeBody.surfaces.Push (face (0, 0, 6, 2, -1, false));
            nativeSource.snapshot.bodies.Push (nativeBody);
            MixedFloorGeometry nativeTrace;
            MixedFloorProvenance nativeP;
            check (OtdSlab_BuildFiniteMixedGeometryWithProvenance (
                       zone, {nativeSource}, roles, 0, nominal, true, {}, 0, {mesh}, nativeTrace, nativeP) &&
                       refsValid (nativeTrace, nativeP) && nativeP.slab.supports.GetSize () == 1 &&
                       nativeP.slab.supports[0].origin == RoomSurfaceOrigin::Native3D &&
                       nativeP.slab.supports[0].bodyIndex == 7 && nativeP.slab.supports[0].polygonIndex == 11 &&
                       std::fabs (contributionArea (nativeP.slab.contributions) - 8) <= AreaTolerance,
                   "snapshot Slab records actual native face identity not synthetic prepared IDs");

            auto twoPartZone = zone;
            twoPartZone.bodies[0].surfaces.Push (face (8, 0, 10, 2, 0, false));
            auto twoPartBase = base;
            twoPartBase.poly = rectangle (0, 0, 10, 2);
            auto spanningMesh = mesh;
            spanningMesh.surface.snapshot.bodies[0].surfaces[0] = face (2, 0, 10, 2, 1, true);
            spanningMesh.surface.snapshot.bodies[0].surfaces[1] = face (2, 0, 10, 2, -3, false);
            MixedFloorGeometry twoPartTrace;
            MixedFloorProvenance twoPartP;
            check (OtdSlab_BuildFiniteMixedGeometryWithProvenance (twoPartZone,
                                                                   sources,
                                                                   roles,
                                                                   0,
                                                                   nominal,
                                                                   true,
                                                                   {twoPartBase},
                                                                   0,
                                                                   {spanningMesh},
                                                                   twoPartTrace,
                                                                   twoPartP) &&
                       refsValid (twoPartTrace, twoPartP) && twoPartP.mesh.contributions.GetSize () == 2 &&
                       twoPartP.mesh.contributions[0].floorIndex != twoPartP.mesh.contributions[1].floorIndex &&
                       std::fabs (contributionArea (twoPartP.mesh.contributions) - 12) <= AreaTolerance &&
                       std::fabs (contributionArea (twoPartP.slab.contributions) - 4) <= AreaTolerance,
                   "disconnected room components offset Mesh floor references before mixed merge");

            {
                // Горизонтальная опора с дугой не требует прямолинейного Mesh-представления.
                PolyArcRec curvedArc;
                curvedArc.begIndex = 2;
                curvedArc.endIndex = 3;
                curvedArc.arcAngle = .5;
                const GS::Array<Point2D> curvedPoints = {{1, .5}, {3, .5}, {3, 1.5}, {1, 1.5}};
                const auto curvedPolygons =
                    Geometry::Polygon2D::Create (Geometry::PolyFlags_IsSameDirEdgeLegal,
                                                 curvedPoints,
                                                 GS::Array<PolyArcRec>{curvedArc},
                                                 GS::Array<UIndex>{3},
                                                 Geometry::PolyCreateFlags_IdxRefInContourEndFromZero |
                                                     Geometry::PolyCreateFlags_IdxRefInArcFromZero);
                check (curvedPolygons.GetSize () == 1, "curved Slab fixture creates one real SDK polygon");
                if (curvedPolygons.GetSize () == 1) {
                    auto curvedBase = base;
                    curvedBase.poly = curvedPolygons[0];
                    Geometry::MultiPolygon2D curvedExpected;
                    curvedExpected.Push (curvedBase.poly);
                    const auto containsArc = [] (const Geometry::Polygon2D &polygon) {
                        GS::Array<Point2D> points;
                        GS::Array<PolyArcRec> arcs;
                        GS::Array<UIndex> ends;
                        GS::Array<Geometry::Polygon2D::ConstContourIterator> reversed;
                        polygon.CopyPolygonData (
                            points, arcs, ends, reversed, Geometry::PolyCreateFlags_IdxRefInContourEndFromZero);
                        return !arcs.IsEmpty ();
                    };
                    GS::Array<OtdSlab> curvedLegacy;
                    double curvedRejected = 0;
                    const GS::Array<OtdSlab> curvedPrepared = {curvedBase};
                    const bool curvedLegacyBuilt = OtdSlab_BuildProjectedFinishes (zone,
                                                                                   sources,
                                                                                   roles,
                                                                                   Floor,
                                                                                   0,
                                                                                   curvedLegacy,
                                                                                   curvedRejected,
                                                                                   &nominal,
                                                                                   true,
                                                                                   nullptr,
                                                                                   &curvedPrepared);
                    check (curvedLegacyBuilt && curvedLegacy.GetSize () == 1 && containsArc (curvedLegacy[0].poly) &&
                               same (FloorCoverage (curvedLegacy), curvedExpected),
                           "legacy horizontal Slab accepts and preserves original curved "
                           "footprint");
                    MixedFloorGeometry curvedGeometry;
                    MixedFloorProvenance curvedProvenance;
                    const bool curvedBuilt = OtdSlab_BuildFiniteMixedGeometryWithProvenance (
                        zone, sources, roles, 0, nominal, true, {curvedBase}, 0, {}, curvedGeometry, curvedProvenance);
                    check (curvedBuilt,
                           "Slab provenance accepts identical horizontal planes "
                           "with curved overlap");
                    check (curvedBuilt && curvedGeometry.floors.GetSize () == 1 &&
                               curvedProvenance.slab.supports.GetSize () == 1 &&
                               curvedProvenance.slab.contributions.GetSize () == 1 &&
                               curvedProvenance.mesh.supports.IsEmpty () &&
                               curvedProvenance.slab.contributions[0].supportIndex == 0 &&
                               curvedProvenance.slab.contributions[0].floorIndex == 0 &&
                               curvedProvenance.slab.supports[0].plane.slope_x == 0 &&
                               curvedProvenance.slab.supports[0].plane.slope_y == 0 &&
                               curvedProvenance.slab.supports[0].plane.zBottom == 0 &&
                               containsArc (curvedGeometry.floors[0].poly) &&
                               same (curvedProvenance.slab.supports[0].projected, curvedExpected) &&
                               same (curvedProvenance.slab.supports[0].admitted, curvedExpected) &&
                               same (curvedProvenance.slab.contributions[0].domain, curvedExpected),
                           "curved support admitted and winner domains retain exact finite arc "
                           "and indices");
                    auto curvedOutsideMesh = mesh;
                    curvedOutsideMesh.surface.snapshot.bodies[0].surfaces[0] = face (8, 0, 10, 2, 1, true);
                    curvedOutsideMesh.surface.snapshot.bodies[0].surfaces[1] = face (8, 0, 10, 2, -3, false);
                    MixedFloorGeometry curvedOutsideGeometry;
                    MixedFloorProvenance curvedOutsideProvenance;
                    const bool curvedOutsideBuilt =
                        OtdSlab_BuildFiniteMixedGeometryWithProvenance (zone,
                                                                        sources,
                                                                        roles,
                                                                        0,
                                                                        nominal,
                                                                        true,
                                                                        {curvedBase},
                                                                        0,
                                                                        {curvedOutsideMesh},
                                                                        curvedOutsideGeometry,
                                                                        curvedOutsideProvenance);
                    check (curvedOutsideBuilt && curvedOutsideGeometry.meshDomain.IsEmpty () &&
                               curvedOutsideGeometry.floors.GetSize () == 1 &&
                               !curvedOutsideProvenance.mesh.supports.IsEmpty () &&
                               curvedOutsideProvenance.mesh.contributions.IsEmpty () &&
                               curvedOutsideProvenance.slab.contributions.GetSize () == 1 &&
                               same (curvedOutsideProvenance.slab.contributions[0].domain, curvedExpected),
                           "empty Mesh domain preserves curved Slab provenance and hidden "
                           "unroomed support");
                }
            }

            // Дополнительные проверки перед завершающей проверкой независимых ступеней.
            // C++14; этот блок не включает производственный Mesh-маршрут.
            {
                const auto exArray = [] (const auto &a, const auto &b, const auto &eq) {
                    if (a.GetSize () != b.GetSize ())
                        return false;
                    for (UIndex i = 0; i < a.GetSize (); ++i)
                        if (!eq (a[i], b[i]))
                            return false;
                    return true;
                };
                const auto exPoly = [&] (const auto &a, const auto &b) {
                    Geometry::MultiPolygon2D aa, bb;
                    aa.Push (a);
                    bb.Push (b);
                    return a.GetContourNum () == b.GetContourNum () && same (aa, bb);
                };
                const auto exComposite = [&] (const auto &a, const auto &b) {
                    return a.inx == b.inx && a.fillThick == b.fillThick && a.fillThick_min == b.fillThick_min &&
                           a.rfromstart == b.rfromstart && a.structype == b.structype && a.num == b.num &&
                           a.val == b.val && a.area == b.area && a.area_fill == b.area_fill && a.volume == b.volume &&
                           a.length == b.length && a.width == b.width && a.kzap == b.kzap && a.qty == b.qty &&
                           a.unit == b.unit && a.pos == b.pos && true;
                };
                const auto exFavorite = [&] (const auto &a, const auto &b) {
                    return a.name == b.name && a.type == b.type && a.is_composite_read == b.is_composite_read &&
                           a.thickness == b.thickness && exArray (a.composite, b.composite, exComposite);
                };
                const auto exMaterial = [&] (const auto &a, const auto &b) {
                    return a.material == b.material && a.smaterial == b.smaterial && a.rawname == b.rawname &&
                           a.rawname_bytype == b.rawname_bytype && true;
                };
                const auto exDraw = [&] (const auto &a, const auto &b) {
                    return a.base_guid == b.base_guid && a.otd_guid == b.otd_guid && a.base_type == b.base_type &&
                           a.draw_type == b.draw_type && a.type == b.type && a.draw_error == b.draw_error &&
                           exMaterial (a.material, b.material) && exFavorite (a.favorite, b.favorite);
                };
                const auto exFloor = [&] (const auto &a, const auto &b) {
                    return a.zBottom == b.zBottom && a.floorInd == b.floorInd && a.height == b.height &&
                           a.reference_z == b.reference_z && a.plane_origin.x == b.plane_origin.x &&
                           a.plane_origin.y == b.plane_origin.y && a.slope_x == b.slope_x && a.slope_y == b.slope_y &&
                           a.tip == b.tip && a.isValid == b.isValid && a.surface_geometry == b.surface_geometry &&
                           exPoly (a.poly, b.poly) && exDraw (a.draw, b.draw) &&
                           exArray (a.base_composite, b.base_composite, exComposite);
                };
                const auto exOpening = [&] (const auto &a, const auto &b) {
                    return a.zBottom == b.zBottom && a.height == b.height && a.width == b.width &&
                           a.objLoc == b.objLoc && a.lower == b.lower && a.reflected == b.reflected &&
                           a.ref_side == b.ref_side && a.reveal_from_sill == b.reveal_from_sill &&
                           a.has_reveal == b.has_reveal && a.create_reveal == b.create_reveal &&
                           a.surface_geometry == b.surface_geometry && a.base_reveal_width == b.base_reveal_width &&
                           a.base_guid == b.base_guid && a.otd_guid == b.otd_guid &&
                           exArray (a.source_opening_guids, b.source_opening_guids, [] (const auto &x, const auto &y) {
                               return x == y;
                           });
                };
                const auto exStep = [&] (const auto &a, const auto &b) {
                    return a.geom.seg.begC.x == b.geom.seg.begC.x && a.geom.seg.begC.y == b.geom.seg.begC.y &&
                           a.geom.seg.begC.z == b.geom.seg.begC.z && a.geom.seg.endC.x == b.geom.seg.endC.x &&
                           a.geom.seg.endC.y == b.geom.seg.endC.y && a.geom.seg.endC.z == b.geom.seg.endC.z &&
                           a.geom.ang_begC == b.geom.ang_begC && a.geom.ang_endC == b.geom.ang_endC &&
                           a.geom.height == b.geom.height && a.geom.width == b.geom.width &&
                           a.geom.length == b.geom.length && a.geom.floorInd == b.geom.floorInd &&
                           a.floor_core_offset == b.floor_core_offset &&
                           a.floor_core_offset_valid == b.floor_core_offset_valid && a.base_th == b.base_th &&
                           a.base_flipped == b.base_flipped && a.isValid == b.isValid &&
                           a.slant_angle == b.slant_angle && a.surface_geometry == b.surface_geometry &&
                           a.rise_x == b.rise_x && a.rise_y == b.rise_y && a.slab_step == b.slab_step &&
                           a.slab_step_writer_eligible == b.slab_step_writer_eligible && exDraw (a.draw, b.draw) &&
                           exArray (a.base_composite, b.base_composite, exComposite) &&
                           exArray (a.openings, b.openings, exOpening);
                };
                const auto exMeshSupport = [&] (const auto &a, const auto &b) {
                    return a.ownerGuid == b.ownerGuid && a.geometryGuid == b.geometryGuid &&
                           a.ownerType == b.ownerType && a.bodyIndex == b.bodyIndex &&
                           a.polygonIndex == b.polygonIndex && a.modelMaterialIndex == b.modelMaterialIndex &&
                           exFloor (a.plane, b.plane) && same (a.projected, b.projected) &&
                           same (a.admitted, b.admitted);
                };
                const auto exSlabSupport = [&] (const auto &a, const auto &b) {
                    return a.ownerGuid == b.ownerGuid && a.geometryGuid == b.geometryGuid &&
                           a.ownerType == b.ownerType && a.origin == b.origin && a.preparedIndex == b.preparedIndex &&
                           a.bodyIndex == b.bodyIndex && a.polygonIndex == b.polygonIndex &&
                           a.modelMaterialIndex == b.modelMaterialIndex && exFloor (a.plane, b.plane) &&
                           same (a.projected, b.projected) && same (a.admitted, b.admitted);
                };
                const auto exContribution = [&] (const auto &a, const auto &b) {
                    return a.supportIndex == b.supportIndex && a.floorIndex == b.floorIndex &&
                           same (a.domain, b.domain);
                };
                const auto exGeometry = [&] (const MixedFloorGeometry &a, const MixedFloorGeometry &b) {
                    return exArray (a.floors, b.floors, exFloor) && exArray (a.steps, b.steps, exStep) &&
                           same (a.meshDomain, b.meshDomain) && same (a.changedDomain, b.changedDomain) &&
                           a.rejectedArea == b.rejectedArea;
                };
                const auto exProvenance = [&] (const MixedFloorProvenance &a, const MixedFloorProvenance &b) {
                    return exArray (a.mesh.supports, b.mesh.supports, exMeshSupport) &&
                           exArray (a.slab.supports, b.slab.supports, exSlabSupport) &&
                           exArray (a.mesh.contributions, b.mesh.contributions, exContribution) &&
                           exArray (a.slab.contributions, b.slab.contributions, exContribution);
                };
                const auto exRect = [&] (double x0, double y0, double x1, double y1) {
                    Geometry::MultiPolygon2D p;
                    p.Push (rectangle (x0, y0, x1, y1));
                    return p;
                };
                const auto exDomains = [&] (const auto &cs) {
                    Geometry::MultiPolygon2D p;
                    for (const auto &c : cs)
                        p.Append (c.domain);
                    p.Unify ();
                    return p;
                };
                // Сравниваем упорядоченные префикс и добавленный набор, а не только суммарное покрытие.
                const auto exAppend = [&] (const MixedFloorGeometry &before,
                                           const MixedFloorProvenance &bp,
                                           const MixedFloorGeometry &batch,
                                           const MixedFloorProvenance &p,
                                           const MixedFloorGeometry &after,
                                           const MixedFloorProvenance &ap) {
                    MixedFloorGeometry e = before;
                    MixedFloorProvenance ep = bp;
                    auto mp = p.mesh.contributions;
                    auto sp = p.slab.contributions;
                    for (auto &c : mp) {
                        c.floorIndex += before.floors.GetSize ();
                        c.supportIndex += bp.mesh.supports.GetSize ();
                    }
                    for (auto &c : sp) {
                        c.floorIndex += before.floors.GetSize ();
                        c.supportIndex += bp.slab.supports.GetSize ();
                    }
                    e.floors.Append (batch.floors);
                    e.steps.Append (batch.steps);
                    e.meshDomain.Append (batch.meshDomain);
                    e.meshDomain.Unify ();
                    e.changedDomain.Append (batch.changedDomain);
                    e.changedDomain.Unify ();
                    e.rejectedArea += batch.rejectedArea;
                    ep.mesh.supports.Append (p.mesh.supports);
                    ep.slab.supports.Append (p.slab.supports);
                    ep.mesh.contributions.Append (mp);
                    ep.slab.contributions.Append (sp);
                    return exGeometry (e, after) && exProvenance (ep, ap);
                };
                MixedFloorGeometry exBatch;
                MixedFloorProvenance exBP;
                const bool exBuilt = OtdSlab_BuildFiniteMixedGeometryWithProvenance (
                    zone, sources, roles, 0, nominal, true, prepared, 0, {mesh}, exBatch, exBP);
                check (exBuilt && same (ownerCoverage (exBatch.floors, meshOwner), exRect (2, 0, 4, 2)) &&
                           same (exDomains (exBP.mesh.contributions), exRect (2, 0, 4, 2)) &&
                           same (exDomains (exBP.slab.contributions),
                                 Geometry::MultiPolygon2D::Substract (exRect (0, 0, 6, 2), exRect (2, 0, 4, 2))),
                       "extra explicit independent central winner domains");
                MixedFloorGeometry exTwice = exBatch;
                MixedFloorProvenance exTP = exBP;
                check (exBuilt &&
                           OtdSlab_BuildFiniteMixedGeometryWithProvenance (
                               zone, sources, roles, 0, nominal, true, prepared, 0, {mesh}, exTwice, exTP) &&
                           exAppend (exBatch, exBP, exBatch, exBP, exTwice, exTP),
                       "extra second append preserves complete ordered prefix and exact remaps");
                // Независимый префикс содержит разное число Mesh- и Slab-опор.
                MixedFloorGeometry exSeed = noMeshTrace;
                MixedFloorProvenance exSP = noMeshP;
                OtdSlab exUnrelated = base;
                exUnrelated.poly = rectangle (20, 20, 21, 21);
                exUnrelated.draw.base_guid = zoneOwner;
                exUnrelated.draw.base_type = API_ZoneID;
                exUnrelated.reference_z = 43;
                exUnrelated.plane_origin = {20, 20};
                exUnrelated.slope_x = 0.125;
                exUnrelated.tip = "unrelated prefix";
                exUnrelated.draw.material.material = 19;
                exUnrelated.draw.favorite.name = "prefix favorite";
                exUnrelated.draw.favorite.thickness = 0.17;
                exSeed.floors.Push (exUnrelated);
                exSeed.steps.Append (beforeSteps);
                exSeed.rejectedArea = 0.375;
                exSeed.meshDomain = exRect (20, 20, 21, 21);
                exSeed.changedDomain = exRect (22, 20, 23, 21);
                const auto exSeedBefore = exSeed;
                const auto exSPBefore = exSP;
                check (exBuilt &&
                           OtdSlab_BuildFiniteMixedGeometryWithProvenance (
                               zone, sources, roles, 0, nominal, true, prepared, 0, {mesh}, exSeed, exSP) &&
                           exAppend (exSeedBefore, exSPBefore, exBatch, exBP, exSeed, exSP),
                       "extra unrelated nonempty prefix independent support offsets domains rejected area");
                MixedFloorGeometry exStepBatch;
                MixedFloorProvenance exStepP;
                const bool exStepBuilt = OtdSlab_BuildFiniteMixedGeometryWithProvenance (zone,
                                                                                         suppressedSources,
                                                                                         suppressedRoles,
                                                                                         0,
                                                                                         stepNominal,
                                                                                         true,
                                                                                         suppressedPrepared,
                                                                                         0,
                                                                                         {rightMesh},
                                                                                         exStepBatch,
                                                                                         exStepP);
                check (exStepBuilt && beforeSteps.GetSize () == 2 && exArray (exStepBatch.steps, beforeSteps, exStep) &&
                           exArray (exStepBatch.steps, independentSteps.steps, exStep),
                       "extra independent Slab step complete geometry metadata eligibility and endpoints");
                const auto exBeforeStepAppend = exSeed;
                const auto exBeforeStepP = exSP;
                check (exStepBuilt &&
                           OtdSlab_BuildFiniteMixedGeometryWithProvenance (zone,
                                                                           suppressedSources,
                                                                           suppressedRoles,
                                                                           0,
                                                                           stepNominal,
                                                                           true,
                                                                           suppressedPrepared,
                                                                           0,
                                                                           {rightMesh},
                                                                           exSeed,
                                                                           exSP) &&
                           exAppend (exBeforeStepAppend, exBeforeStepP, exStepBatch, exStepP, exSeed, exSP),
                       "extra Slab steps append after unrelated existing step prefix");
                const auto exAtomic = [&] (const RoomSurfaceSnapshot &z,
                                           const GS::Array<RoomSurfaceSource> &ss,
                                           const GS::Array<OtdSlab> &pp,
                                           const GS::Array<MeshFloorSource> &mm) {
                    auto g = exSeed;
                    auto p = exSP;
                    return !OtdSlab_BuildFiniteMixedGeometryWithProvenance (
                               z, ss, roles, 0, nominal, true, pp, 0, mm, g, p) &&
                           exGeometry (g, exSeed) && exProvenance (p, exSP);
                };
                check (exAtomic (zone, sources, prepared, {broken}),
                       "extra invalid Mesh atomic all outputs not only counts");
                auto exWrongType = base;
                exWrongType.draw.base_type = API_WallID;
                check (exAtomic (zone, sources, {exWrongType}, {}),
                       "extra malformed prepared base_type opt in atomic refusal");
                MixedFloorGeometry exOldWrongType;
                check (OtdSlab_BuildFiniteMixedGeometry (
                           zone, sources, roles, 0, nominal, true, {exWrongType}, 0, {}, exOldWrongType),
                       "extra nullptr route retains legacy prepared base_type acceptance");
                auto exWrongPreparedGuid = base;
                exWrongPreparedGuid.draw.base_guid = zoneOwner;
                check (exAtomic (zone, sources, {exWrongPreparedGuid}, {}),
                       "extra malformed prepared owner GUID mismatch atomic refusal");
                auto exOldMissing = exSeed;
                check (!OtdSlab_BuildFiniteMixedGeometry (
                           zone, sources, roles, 0, nominal, true, {exWrongPreparedGuid}, 0, {}, exOldMissing) &&
                           exGeometry (exOldMissing, exSeed),
                       "extra nullptr missing prepared owner retains existing atomic refusal policy");
                auto exWrongGuidSource = slabSource;
                exWrongGuidSource.snapshot.guid = zoneOwner;
                check (exAtomic (zone, {exWrongGuidSource}, prepared, {mesh}),
                       "extra malformed owner snapshot guid mismatch opt in atomic refusal");
                MixedFloorGeometry exOldWrongGuid;
                check (OtdSlab_BuildFiniteMixedGeometry (
                           zone, {exWrongGuidSource}, roles, 0, nominal, true, prepared, 0, {mesh}, exOldWrongGuid) &&
                           exGeometry (exOldWrongGuid, visible),
                       "extra nullptr route unchanged on owner snapshot guid mismatch");
                auto exTie = mesh;
                exTie.surface.snapshot.bodies[0].surfaces[0] = face (2, 0, 4, 2, 0, true);
                check (exAtomic (zone, sources, prepared, {exTie}),
                       "extra coplanar Mesh versus Slab ambiguity atomic refusal");
                // В первой компоненте Mesh строго выше; во второй он совпадает с отметкой Slab.
                auto exLate = spanningMesh;
                exLate.surface.snapshot.bodies[0].surfaces[0] = face (2, 0, 6, 2, 1, true);
                exLate.surface.snapshot.bodies[0].surfaces.Push (face (8, 0, 10, 2, 0, true));
                MixedFloorGeometry exFirst;
                MixedFloorProvenance exFirstP;
                check (OtdSlab_BuildFiniteMixedGeometryWithProvenance (
                           zone, sources, roles, 0, nominal, true, {twoPartBase}, 0, {exLate}, exFirst, exFirstP) &&
                           same (exDomains (exFirstP.mesh.contributions), exRect (2, 0, 6, 2)),
                       "extra late ambiguity first component independently succeeds");
                check (exAtomic (twoPartZone, sources, {twoPartBase}, {exLate}),
                       "extra second footprint component ambiguity leaves complete outputs atomic");
                // Ожидаемые конечные области и индексы заданы отдельно для каждой компоненты.
                bool exDisconnected = twoPartP.mesh.contributions.GetSize () == 2;
                unsigned exLeft = 0, exRight = 0;
                for (const auto &c : twoPartP.mesh.contributions) {
                    if (c.floorIndex >= twoPartTrace.floors.GetSize ()) {
                        exDisconnected = false;
                        continue;
                    }
                    Geometry::MultiPolygon2D fp;
                    fp.Push (twoPartTrace.floors[c.floorIndex].poly);
                    if (same (c.domain, exRect (2, 0, 6, 2))) {
                        ++exLeft;
                        exDisconnected = exDisconnected && same (fp, exRect (2, 0, 6, 2));
                    } else if (same (c.domain, exRect (8, 0, 10, 2))) {
                        ++exRight;
                        exDisconnected = exDisconnected && same (fp, exRect (8, 0, 10, 2));
                    } else
                        exDisconnected = false;
                }
                check (exDisconnected && exLeft == 1 && exRight == 1 &&
                           same (exDomains (twoPartP.slab.contributions), exRect (0, 0, 2, 2)),
                       "extra disconnected Mesh exact domains resolve to exact final floor indices");
                // Отверстие Slab внутри D закрывает более высокий Mesh; прежняя политика отверстий не меняется.
                auto exNativeHole = nativeSource;
                auto exHoleContour = face (2.5, 0.5, 3.5, 1.5, 0, true).contours[0];
                exHoleContour.hole = true;
                exNativeHole.snapshot.bodies[0].surfaces[0].contours.Push (exHoleContour);
                    // Проверяем перенос индекса из синтетического snapshot, а не чтение native-материала.
        #ifdef ServerMainVers_2700
                exNativeHole.snapshot.bodies[0].surfaces[0].modelMaterialIndex = ACAPI_CreateAttributeIndex (23);
        #else
                exNativeHole.snapshot.bodies[0].surfaces[0].modelMaterialIndex = 23;
        #endif
                MixedFloorGeometry exHoleG;
                MixedFloorProvenance exHoleP;
                const auto exHoleDomain =
                    Geometry::MultiPolygon2D::Substract (exRect (0, 0, 6, 2), exRect (2.5, 0.5, 3.5, 1.5));
                const bool exHoleOK = OtdSlab_BuildFiniteMixedGeometryWithProvenance (
                    zone, {exNativeHole}, roles, 0, nominal, true, {}, 0, {mesh}, exHoleG, exHoleP);
                check (exHoleOK && exHoleP.slab.supports.GetSize () == 1 && exHoleP.slab.supports[0].bodyIndex == 7 &&
                           exHoleP.slab.supports[0].polygonIndex == 11 &&
        #ifdef ServerMainVers_2700
                           exHoleP.slab.supports[0].modelMaterialIndex == ACAPI_CreateAttributeIndex (23) &&
        #else
                           exHoleP.slab.supports[0].modelMaterialIndex == 23 &&
        #endif
                           same (exHoleP.slab.supports[0].projected, exHoleDomain) &&
                           same (exHoleP.slab.supports[0].admitted, exHoleDomain) &&
                           same (exDomains (exHoleP.slab.contributions),
                                 Geometry::MultiPolygon2D::Substract (exHoleDomain, exRect (2, 0, 4, 2))),
                       "extra native Slab hole original admitted and winner preserve face material identity");
                // Перекрывающиеся грани одного владельца сохраняют оба источника provenance.
                auto exMultiNative = nativeSource;
                auto exSecondNative = exMultiNative.snapshot.bodies[0].surfaces[0];
                exSecondNative.polygonIndex = 12;
        #ifdef ServerMainVers_2700
                exSecondNative.modelMaterialIndex = ACAPI_CreateAttributeIndex (24);
        #else
                exSecondNative.modelMaterialIndex = 24;
        #endif
                exMultiNative.snapshot.bodies[0].surfaces.Push (exSecondNative);
                MixedFloorGeometry exMultiG;
                MixedFloorProvenance exMultiP;
                const bool exMultiOK = OtdSlab_BuildFiniteMixedGeometryWithProvenance (
                    zone, {exMultiNative}, roles, 0, nominal, true, {}, 0, {mesh}, exMultiG, exMultiP);
                bool exBothNative =
                    exMultiOK && exMultiP.slab.supports.GetSize () == 2 && exMultiP.slab.contributions.GetSize () == 4;
                unsigned exFace11 = 0, exFace12 = 0;
                for (const auto &su : exMultiP.slab.supports) {
                    exBothNative = exBothNative && su.bodyIndex == 7 && same (su.projected, exRect (0, 0, 6, 2)) &&
                                   same (su.admitted, exRect (0, 0, 6, 2));
                    if (su.polygonIndex == 11)
                        ++exFace11;
        #ifdef ServerMainVers_2700
                    if (su.polygonIndex == 12 && su.modelMaterialIndex == ACAPI_CreateAttributeIndex (24))
        #else
                    if (su.polygonIndex == 12 && su.modelMaterialIndex == 24)
        #endif
                        ++exFace12;
                }
                check (exBothNative && exFace11 == 1 && exFace12 == 1 && refsValid (exMultiG, exMultiP) &&
                           same (exDomains (exMultiP.slab.contributions),
                                 Geometry::MultiPolygon2D::Substract (exRect (0, 0, 6, 2), exRect (2, 0, 4, 2))),
                       "extra same owner multiple coplanar native faces retain overlapping provenance");
                // Остаток от Zone не получает выдуманную Slab-опору.
                MixedFloorGeometry exZoneG;
                MixedFloorProvenance exZoneP;
                check (OtdSlab_BuildFiniteMixedGeometryWithProvenance (
                           zone, {}, {}, 0, nominal, false, {}, 0, {mesh}, exZoneG, exZoneP) &&
                           exZoneP.slab.supports.IsEmpty () && exZoneP.slab.contributions.IsEmpty () &&
                           same (ownerCoverage (exZoneG.floors, zoneOwner),
                                 Geometry::MultiPolygon2D::Substract (exRect (0, 0, 6, 2), exRect (2, 0, 4, 2))) &&
                           same (exDomains (exZoneP.mesh.contributions), exRect (2, 0, 4, 2)),
                       "extra onlyOnSlab false nominal Zone fallback has no fake Slab provenance");
                MixedFloorGeometry exEmptyG;
                MixedFloorProvenance exEmptyP;
                check (
                    OtdSlab_BuildFiniteMixedGeometryWithProvenance (
                        zone, sources, roles, 0, nominal, true, prepared, 0, {emptyDomainMesh}, exEmptyG, exEmptyP) &&
                        exEmptyG.meshDomain.IsEmpty () && exEmptyP.mesh.supports.GetSize () == 1 &&
                        exEmptyP.mesh.contributions.IsEmpty () &&
                        same (exEmptyP.mesh.supports[0].projected, exRect (2, 0, 4, 2)) &&
                        exEmptyP.mesh.supports[0].admitted.IsEmpty () &&
                        same (exDomains (exEmptyP.slab.contributions), exRect (0, 0, 6, 2)),
                    "extra empty admitted D retains rejected Mesh original support and Slab winner");
                auto exOutside = mesh;
                exOutside.surface.snapshot.bodies[0].surfaces[0] = face (20, 0, 22, 2, 1, true);
                exOutside.surface.snapshot.bodies[0].surfaces[1] = face (20, 0, 22, 2, -3, false);
                MixedFloorGeometry exOutsideG;
                MixedFloorProvenance exOutsideP;
                check (OtdSlab_BuildFiniteMixedGeometryWithProvenance (
                           zone, sources, roles, 0, nominal, true, prepared, 0, {exOutside}, exOutsideG, exOutsideP) &&
                           exOutsideG.meshDomain.IsEmpty () && exOutsideP.mesh.supports.GetSize () == 1 &&
                           exOutsideP.mesh.contributions.IsEmpty () &&
                           same (exOutsideP.mesh.supports[0].projected, exRect (20, 0, 22, 2)) &&
                           same (exOutsideP.mesh.supports[0].admitted, exRect (20, 0, 22, 2)) &&
                           same (exDomains (exOutsideP.slab.contributions), exRect (0, 0, 6, 2)),
                       "extra outside room Mesh support remains unroomed despite empty D");
                const auto exSlabCoverage = [&] (const MixedFloorGeometry &geometry,
                                                 const MixedFloorProvenance &provenance) {
                    USize slabCount = 0;
                    for (UIndex i = 0; i < geometry.floors.GetSize (); ++i) {
                        const auto &floor = geometry.floors[i];
                        if (floor.draw.base_type != API_SlabID)
                            continue;
                        ++slabCount;
                        Geometry::MultiPolygon2D covered, expected;
                        expected.Push (floor.poly);
                        USize count = 0;
                        for (const auto &contribution : provenance.slab.contributions) {
                            if (contribution.floorIndex != i)
                                continue;
                            if (contribution.supportIndex >= provenance.slab.supports.GetSize ())
                                return false;
                            const auto &support = provenance.slab.supports[contribution.supportIndex];
                            if (support.ownerGuid != floor.draw.base_guid || support.ownerType != API_SlabID ||
                                support.plane.slope_x != 0 || support.plane.slope_y != 0 || floor.slope_x != 0 ||
                                floor.slope_y != 0 || std::fabs (support.plane.zBottom - floor.zBottom) > 1.e-6 ||
                                !same (contribution.domain,
                                       Geometry::MultiPolygon2D::Intersect (support.admitted, expected)))
                                return false;
                            ++count;
                            covered.Append (contribution.domain);
                        }
                        covered.Unify ();
                        if (count == 0 || !same (covered, expected))
                            return false;
                    }
                    return slabCount != 0;
                };
                check (exBuilt && exStepBuilt && exHoleOK && exMultiOK && exSlabCoverage (exBatch, exBP) &&
                           exSlabCoverage (exStepBatch, exStepP) && exSlabCoverage (exHoleG, exHoleP) &&
                           exSlabCoverage (exMultiG, exMultiP) && exSlabCoverage (noMeshTrace, noMeshP),
                       "extra every final Slab floor has full same plane finite contribution union");
                check (twoPartP.mesh.contributions.GetSize () == 2 &&
                           same (twoPartP.mesh.contributions[0].domain, exRect (2, 0, 6, 2)) &&
                           same (twoPartP.mesh.contributions[1].domain, exRect (8, 0, 10, 2)),
                       "extra actual ordered footprint visits valid left component before ambiguous right component");
                auto exRaised = base;
                exRaised.height = 2;
                MixedFloorGeometry exLevelG;
                MixedFloorProvenance exLevelP;
                const bool exLevelsOK = OtdSlab_BuildFiniteMixedGeometryWithProvenance (
                    zone, sources, roles, 0, nominal, true, {base, exRaised}, 0, {}, exLevelG, exLevelP);
                check (exLevelsOK && exLevelG.floors.GetSize () == 1 && exLevelG.floors[0].zBottom == 1 &&
                           exLevelP.slab.supports.GetSize () == 2 && exLevelP.slab.supports[0].plane.zBottom == 0 &&
                           exLevelP.slab.supports[1].plane.zBottom == 1 &&
                           exLevelP.slab.contributions.GetSize () == 1 &&
                           exLevelP.slab.contributions[0].supportIndex == 1 &&
                           exLevelP.slab.contributions[0].floorIndex == 0 && exSlabCoverage (exLevelG, exLevelP),
                       "extra same owner identical XY at distinct Slab levels maps only the true winning plane");
                auto exOutsideHoleSource = nativeSource;
                auto exOutsideHoleContour = face (.5, .5, 1.5, 1.5, 0, true).contours[0];
                exOutsideHoleContour.hole = true;
                exOutsideHoleSource.snapshot.bodies[0].surfaces[0].contours.Push (exOutsideHoleContour);
                const auto exOutsideHoleDomain =
                    Geometry::MultiPolygon2D::Substract (exRect (0, 0, 6, 2), exRect (.5, .5, 1.5, 1.5));
                MixedFloorGeometry exOutsideHoleG;
                MixedFloorProvenance exOutsideHoleP;
                const bool exOutsideHoleOK = OtdSlab_BuildFiniteMixedGeometryWithProvenance (zone,
                                                                                             {exOutsideHoleSource},
                                                                                             roles,
                                                                                             0,
                                                                                             nominal,
                                                                                             true,
                                                                                             {},
                                                                                             0,
                                                                                             {mesh},
                                                                                             exOutsideHoleG,
                                                                                             exOutsideHoleP);
                bool exRetainedHole = false;
                for (const auto &floor : exOutsideHoleG.floors)
                    if (floor.draw.base_guid == slabOwner && floor.poly.GetContourNum () == 2)
                        exRetainedHole = true;
                check (exOutsideHoleOK && exRetainedHole && exSlabCoverage (exOutsideHoleG, exOutsideHoleP) &&
                           same (ownerCoverage (exOutsideHoleG.floors, slabOwner),
                                 Geometry::MultiPolygon2D::Substract (exOutsideHoleDomain, exRect (2, 0, 4, 2))),
                       "extra legacy Slab winner retains hole outside D despite hole removal from remainder");
                MixedFloorGeometry exOnlyHoleG;
                MixedFloorProvenance exOnlyHoleP;
                check (OtdSlab_BuildFiniteMixedGeometryWithProvenance (
                           zone, {exOutsideHoleSource}, roles, 0, nominal, true, {}, 0, {}, exOnlyHoleG, exOnlyHoleP) &&
                           exOnlyHoleG.floors.GetSize () == 1 && exOnlyHoleG.floors[0].poly.GetContourNum () == 2 &&
                           exSlabCoverage (exOnlyHoleG, exOnlyHoleP) &&
                           same (exDomains (exOnlyHoleP.slab.contributions), exOutsideHoleDomain),
                       "extra Slab only original hole is not filled in emitted winner");
                auto exOtherBase = base;
                exOtherBase.draw.base_guid = upperOwner;
                auto exOtherSource = slabSource;
                exOtherSource.baseGuid = upperOwner;
                exOtherSource.geometryGuid = upperOwner;
                exOtherSource.snapshot.guid = upperOwner;
                MixedFloorGeometry exAmbiguousLegacy, exAmbiguousG;
                MixedFloorProvenance exAmbiguousP;
                const bool exAmbiguousLegacyOK = OtdSlab_BuildFiniteMixedGeometry (zone,
                                                                                   {slabSource, exOtherSource},
                                                                                   {slabOwner, upperOwner},
                                                                                   0,
                                                                                   nominal,
                                                                                   true,
                                                                                   {base, exOtherBase},
                                                                                   0,
                                                                                   {},
                                                                                   exAmbiguousLegacy);
                check (exAmbiguousLegacyOK &&
                           OtdSlab_BuildFiniteMixedGeometryWithProvenance (zone,
                                                                           {slabSource, exOtherSource},
                                                                           {slabOwner, upperOwner},
                                                                           0,
                                                                           nominal,
                                                                           true,
                                                                           {base, exOtherBase},
                                                                           0,
                                                                           {},
                                                                           exAmbiguousG,
                                                                           exAmbiguousP) &&
                           exGeometry (exAmbiguousLegacy, exAmbiguousG) && exAmbiguousG.floors.IsEmpty () &&
                           exAmbiguousG.rejectedArea > 0 && exAmbiguousP.slab.supports.GetSize () == 2 &&
                           exAmbiguousP.slab.contributions.IsEmpty (),
                       "extra Slab only ambiguity preserves legacy rejected area without invented winner");
                auto exMixedAmbiguousG = exSeed;
                auto exMixedAmbiguousP = exSP;
                check (!OtdSlab_BuildFiniteMixedGeometryWithProvenance (zone,
                                                                        {slabSource, exOtherSource},
                                                                        {slabOwner, upperOwner},
                                                                        0,
                                                                        nominal,
                                                                        true,
                                                                        {base, exOtherBase},
                                                                        0,
                                                                        {hiddenMesh},
                                                                        exMixedAmbiguousG,
                                                                        exMixedAmbiguousP) &&
                           exGeometry (exMixedAmbiguousG, exSeed) && exProvenance (exMixedAmbiguousP, exSP),
                       "extra distinct Slab owners coplanar inside Mesh domain refuse atomically");
            }

            check (stepsEqual,
                   "independent Slab step order geometry owner and pending writer eligibility are unchanged");
        }
    }

} // namespace TestFunc

    #endif // ServerMainVers_2400

#endif
