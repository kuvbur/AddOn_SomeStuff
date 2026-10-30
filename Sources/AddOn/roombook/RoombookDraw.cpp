// SDK подключается раньше стандартных заголовков.
// clang-format off
#include "ACAPinc.h"
// clang-format on

#include <algorithm>
#include <cmath>
#include <limits>
#include <string>
#include <utility>

#include "api_headers/APIEnvir.h"

#include "CommonFunction.hpp"
#include "Helpers.hpp"
#include "OnExit.hpp"
#include "Propertycache.hpp"
#include "roombook/Roombook.hpp"
#include "Sync.hpp"

#ifdef ServerMainVers_2400
namespace Roombook {

    // -----------------------------------------------------------------------------
    // Проверяет необходимость обновления данных в API_Elem_Head
    // -----------------------------------------------------------------------------
    static bool Elems_Check_Update (API_Elem_Head &elemhead,
                                    API_Element &mask,
                                    short floorInd,
                                    const API_AttributeIndex &layer,
                                    const API_RenovationStatusType &renovationStatus) {
        bool needupdate = false;
        if (elemhead.floorInd != floorInd) {
            ACAPI_ELEMENT_MASK_SET (mask, API_Elem_Head, floorInd);
            elemhead.floorInd = floorInd;
            needupdate = true;
        }
        if (elemhead.layer != layer) {
            ACAPI_ELEMENT_MASK_SET (mask, API_Elem_Head, layer);
            elemhead.layer = layer;
            needupdate = true;
        }
        bool renovationChanged = elemhead.renovationStatus != renovationStatus;
        if (renovationStatus == API_DefaultStatus && elemhead.guid != APINULLGuid) {
            // Default — только входной псевдостатус; сравниваем категории без перевода GUID в enum.
            API_ElemCategory category = {};
            category.categoryID = API_ElemCategory_RenovationStatus;
            API_ElemCategoryValue defaultValue = {};
            API_ElemCategoryValue currentValue = {};
    #ifdef ServerMainVers_2700
            GSErrCode err = ACAPI_Category_GetCategoryValueDefault (elemhead.type, category, &defaultValue);
    #elif defined ServerMainVers_2600
            GSErrCode err = ACAPI_Element_GetCategoryValueDefault (elemhead.type, category, &defaultValue);
    #else
            GSErrCode err =
                ACAPI_Element_GetCategoryValueDefault (elemhead.typeID, elemhead.variationID, category, &defaultValue);
    #endif
            if (err == NoError && defaultValue.guid != APINULLGuid) {
    #ifdef ServerMainVers_2700
                err = ACAPI_Category_GetCategoryValue (elemhead.guid, category, &currentValue);
    #else
                err = ACAPI_Element_GetCategoryValue (elemhead.guid, category, &currentValue);
    #endif
                if (err == NoError && currentValue.guid != APINULLGuid)
                    renovationChanged = currentValue.guid != defaultValue.guid;
            }
            // При ошибке чтения сохраняем прежнее применение Default, а не объявляем no-op.
        }
        if (renovationChanged) {
            ACAPI_ELEMENT_MASK_SET (mask, API_Elem_Head, renovationStatus);
            elemhead.renovationStatus = renovationStatus;
            needupdate = true;
        }
        return needupdate;
    }

    // -----------------------------------------------------------------------------
    // Назначение класса элементу
    // -----------------------------------------------------------------------------
    static void Class_Set (const TypeOtd &type,
                           const API_Guid &guid,
                           const ClassificationFunc::ClassificationDict &finclass) {
        if (guid == APINULLGuid)
            return;
        API_Guid class_guid = Class_GetClassGuid (type, finclass);
        if (class_guid != APINULLGuid)
            ACAPI_Element_AddClassificationItem (guid, class_guid);
    }

    // -----------------------------------------------------------------------------
    // Назначение класса проёму
    // Для ненулевого GUID проёма выбирает класс NoSet, затем при его отсутствии класс Wall_Main.
    // Вызывает добавление классификации только при ненулевом GUID выбранного класса.
    // -----------------------------------------------------------------------------
    static void Class_Set_OtdOpening (const OtdOpening &op, const ClassificationFunc::ClassificationDict &finclass) {
        if (op.otd_guid == APINULLGuid)
            return;
        API_Guid class_guid = Class_GetClassGuid (NoSet, finclass);
        if (class_guid == APINULLGuid)
            class_guid = Class_GetClassGuid (Wall_Main, finclass);
        if (class_guid != APINULLGuid)
            ACAPI_Element_AddClassificationItem (op.otd_guid, class_guid);
    }

    static bool GetDefault (const GS::UniString &favorite_name,
                            API_Element &element,
                            const API_ElemTypeID &eltype,
                            bool activate = true) {
        GSErrCode err = NoError;
        BNZeroMemory (&element, sizeof (API_Element));
        if (activate ? Favorite_GetByName (favorite_name, element) : Favorite_ReadByName (favorite_name, element)) {
            if (GetElemTypeID (element) == eltype)
                return true;
        }
        SetElemTypeID (element, eltype);
        err = ACAPI_Element_GetDefaults (&element, nullptr);
        if (err != NoError) {
            return false;
        }
        return true;
    }

    static bool GetDefault (const GS::UniString &favorite_name,
                            API_Element &element,
                            API_ElementMemo &memo,
                            const API_ElemTypeID &eltype) {
        GSErrCode err = NoError;
        BNZeroMemory (&memo, sizeof (API_ElementMemo));
        BNZeroMemory (&element, sizeof (API_Element));
        if (Favorite_GetByName (favorite_name, element, memo)) {
            if (GetElemTypeID (element) == eltype)
                return true;
        }
        SetElemTypeID (element, eltype);
        err = ACAPI_Element_GetDefaults (&element, &memo);
        if (err != NoError) {
            ACAPI_DisposeElemMemoHdls (&memo);
            return false;
        }
        return true;
    }

    static bool GetExsist (const API_Guid &guid, API_Element &element, API_ElementMemo &memo) {
        GSErrCode err = NoError;
        BNZeroMemory (&memo, sizeof (API_ElementMemo));
        BNZeroMemory (&element, sizeof (API_Element));
        element.header.guid = guid;
        err = ACAPI_Element_Get (&element);
        if (err != NoError) {
            msg_rep ("GetExsist", "ACAPI_Element_Get", err, guid);
            return false;
        }
        err = ACAPI_Element_GetMemo (guid, &memo);
        if (err != NoError) {
            msg_rep ("GetExsist", "ACAPI_Element_GetMemo", err, guid);
            return false;
        }
        return true;
    }

    // -----------------------------------------------------------------------------
    // Получение из избранного стены для простроения
    // -----------------------------------------------------------------------------
    bool Wall_GetDefault (const GS::UniString &favorite_name,
                          const TypeOtd &type,
                          API_Element &wallelement,
                          bool activate) {
        if (!GetDefault (favorite_name, wallelement, API_WallID, activate)) {
            return false;
        }
        wallelement.wall.zoneRel = APIZRel_None;
        wallelement.wall.referenceLineLocation = APIWallRefLine_Inside;
        wallelement.wall.flipped = false;
    #ifdef ServerMainVers_2600
    #else
        wallelement.wall.refMat.overridden = true;
        wallelement.wall.oppMat.overridden = true;
        wallelement.wall.sidMat.overridden = true;
    #endif
        wallelement.wall.modelElemStructureType = API_BasicStructure;
        wallelement.wall.thickness = otd_thickness;
        return true;
    }

    // -----------------------------------------------------------------------------
    // Обнуляет элемент и memo; использует избранное, если оно содержит балку, иначе получает настройки балки по
    // умолчанию. При ошибке получения настроек освобождает memo и возвращает false.
    // -----------------------------------------------------------------------------
    bool Beam_GetDefault (const GS::UniString &favorite_name,
                          const TypeOtd &type,
                          API_Element &beamelement,
                          API_ElementMemo &beammemo) {
        if (!GetDefault (favorite_name, beamelement, beammemo, API_BeamID)) {
            return false;
        }
        return true;
    }

    // -----------------------------------------------------------------------------
    // Возвращает объект из избранного подходящего типа либо получает настройки объекта по умолчанию.
    // Только для настроек по умолчанию сбрасывает угол и автовидимость по этажам, включает перья, линии и материалы
    // объекта.
    // -----------------------------------------------------------------------------
    bool Object_GetDefault (const GS::UniString &favorite_name,
                            const TypeOtd &type,
                            API_Element &element,
                            API_ElementMemo &memo) {
        if (!GetDefault (favorite_name, element, memo, API_ObjectID)) {
            return false;
        }
        element.object.angle = 0;
        element.object.isAutoOnStoryVisibility = false;
        element.object.useObjPens = true;
        element.object.useObjLtypes = true;
        element.object.useObjMaterials = true;
        return true;
    }

    // -----------------------------------------------------------------------------
    // Возвращает перекрытие из избранного подходящего типа либо получает настройки перекрытия по умолчанию.
    // Только для настроек по умолчанию задаёт нулевой offsetFromTop, связанные материалы и базовую структуру.
    // -----------------------------------------------------------------------------
    bool Slab_GetDefault (const GS::UniString &favorite_name, const TypeOtd &type, API_Element &element) {
        if (!GetDefault (favorite_name, element, API_SlabID)) {
            return false;
        }
        element.slab.offsetFromTop = 0;
        element.slab.materialsChained = true;
    #ifdef ServerMainVers_2600
    #else
        element.slab.sideMat.overridden = true;
        element.slab.topMat.overridden = true;
        element.slab.botMat.overridden = true;
    #endif
        element.slab.modelElemStructureType = API_BasicStructure;
        return true;
    }

    // -----------------------------------------------------------------------------
    // Получение настроек проёмов в отделочных стенах
    // -----------------------------------------------------------------------------
    static bool OtdOpening_GetDefault (const GS::UniString &favorite_name,
                                       API_Element &windowelement,
                                       API_ElementMemo &windowmemo) {
        GSErrCode err = NoError;
        BNZeroMemory (&windowmemo, sizeof (API_ElementMemo));
        BNZeroMemory (&windowelement, sizeof (API_Element));
        if (Favorite_GetByName (favorite_name, windowelement, windowmemo)) {
            if (GetElemTypeID (windowelement) == API_WindowID)
                return true;
        }
        SetElemTypeID (windowelement, API_WindowID);
        err = ACAPI_Element_GetDefaults (&windowelement, &windowmemo);
        if (err != NoError) {
            ACAPI_DisposeElemMemoHdls (&windowmemo);
            return false;
        }
        return true;
    }

    // -----------------------------------------------------------------------------
    // Построение проёмов в отделочных стенах
    // -----------------------------------------------------------------------------
    static void OtdOpening_Draw (API_Element &wallelement, OtdOpening &op, double zBottom) {
        GSErrCode err = NoError;
        API_Element windowelement = {};
        API_ElementMemo windowmemo = {};
        if (wallelement.wall.type == APIWtyp_Poly)
            return;
        if (!OtdOpening_GetDefault ("smstf window", windowelement, windowmemo)) {
            return;
        }
        windowelement.window.owner = wallelement.header.guid;
        const GS::OnExit memoGuard ([&windowmemo] () { ACAPI_DisposeElemMemoHdls (&windowmemo); });
        if (op.surface_geometry) {
            // Вырез следует наклону стены, независимо от точки фиксации избранного.
            windowelement.window.directionType = API_WDAssociativeToWall;
            windowelement.window.fixPoint = APIHoleAnchor_CenterFix;
        }
        if (op.has_reveal) {
            double th = wallelement.wall.thickness;
            double zUp_op = op.zBottom + op.height;
            double zUp_wall = zBottom + wallelement.wall.height;
            // Уменьшаем проём на толщину откоса по высоте
            if (!is_equal (zUp_op, zUp_wall)) {
                op.height -= th;
            }
            double lwall = sqrt ((wallelement.wall.begC.x - wallelement.wall.endC.x) *
                                     (wallelement.wall.begC.x - wallelement.wall.endC.x) +
                                 (wallelement.wall.begC.y - wallelement.wall.endC.y) *
                                     (wallelement.wall.begC.y - wallelement.wall.endC.y));
            if (!is_equal (op.objLoc + op.width / 2, lwall)) {
                op.width -= th;
                op.objLoc -= th;
            }
            if (!is_equal (op.objLoc - op.width / 2, 0)) {
                op.width -= th;
                op.objLoc += th;
            }
        }
        // objLoc присваиваем ПОСЛЕ коррекции откосов, чтобы центр проёма совпадал со скорректированными размерами
        windowelement.window.objLoc = op.objLoc;
        if (op.width < min_dim || op.height < min_dim) {
            return;
        }
        windowelement.window.openingBase.width = op.width;
        windowelement.window.openingBase.height = op.height;
        windowelement.window.lower = op.lower;
        err = ACAPI_Element_Create (&windowelement, &windowmemo);
        if (err != NoError) {
            msg_rep ("OtdOpening_Draw err", "ACAPI_Element_Create", err, APINULLGuid);
            return;
        }
        op.otd_guid = windowelement.header.guid;
    }

    // -----------------------------------------------------------------------------
    // Собирает оставшиеся GUID существующей отделки в deletelist; само удаление здесь не выполняется.
    static void OtdRooms_CollectUnusedFinishingElements (GS::HashTable<API_Guid, UnicGuidByBase> &exsistot_byzone,
                                                         GS::Array<API_Guid> &deletelist) {
        // Неиспользованные существующие элементы удаляем
        for (GS::HashTable<API_Guid, UnicGuidByBase>::PairIterator cIt_1 = exsistot_byzone.EnumeratePairs ();
             cIt_1 != NULL;
             ++cIt_1) {
    #ifdef ServerMainVers_2800
            const UnicGuidByBase &byzone = cIt_1->value;
    #else
            const UnicGuidByBase &byzone = *cIt_1->value;
    #endif
            for (UnicGuidByBase::ConstPairIterator cIt_21 = byzone.EnumeratePairs (); cIt_21 != NULL; ++cIt_21) {
    #ifdef ServerMainVers_2800
                const UnicGuidByTypeOtd &bytype = cIt_21->value;
    #else
                const UnicGuidByTypeOtd &bytype = *cIt_21->value;
    #endif
                for (UnicGuidByTypeOtd::ConstPairIterator cIt_2 = bytype.EnumeratePairs (); cIt_2 != NULL; ++cIt_2) {
    #ifdef ServerMainVers_2800
                    const UnicGuid &byparent = cIt_2->value;
    #else
                    const UnicGuid &byparent = *cIt_2->value;
    #endif
                    for (UnicGuid::ConstPairIterator cIt_3 = byparent.EnumeratePairs (); cIt_3 != NULL; ++cIt_3) {
    #ifdef ServerMainVers_2800
                        API_Guid guid = cIt_3->key;
    #else
                        API_Guid guid = *cIt_3->key;
    #endif
                        deletelist.Push (guid);
                    }
                }
            }
        }
    }

    static bool OtdSlab_ShouldDrawFinish (const OtdRoom &room, const OtdSlab &otdslab) {
        if (!room.isValid || !otdslab.isValid)
            return false;
        switch (otdslab.draw.type) {
        case Ceil:
            if (!room.create_ceil_elements)
                return false;
            break;
        case Floor:
            if (!room.create_floor_elements)
                return false;
            break;
        default:
            break;
        }
        return true;
    }

    static bool OtdWall_ShouldDrawFinish (const OtdRoom &room, const OtdWall &wall) {
        if (!room.isValid || !wall.isValid)
            return false;
        switch (wall.draw.type) {
        case Wall_Main:
        case Wall_Up:
        case Wall_Down:
            return room.create_wall_elements;
        case Column:
            return room.create_column_elements;
        case Reveal_Main:
        case Reveal_Up:
        case Reveal_Down:
            return room.create_reveal_elements;
        case Ceil:
            return room.create_ceil_elements;
        case Floor:
            return room.create_floor_elements;
        default:
            break;
        }
        return true;
    }

    API_Guid OtdWall_TakeExistingFinish (UnicGuid &pool,
                                         const API_Element &target,
                                         const GS::HashTable<API_Guid, API_Element> &existing) {
        API_Guid selected = APINULLGuid;
        std::string selectedKey;
        const API_ElemTypeID type = GetElemTypeID (target);
        for (auto it = pool.EnumeratePairs (); it != nullptr; ++it) {
    #ifdef ServerMainVers_2800
            const API_Guid guid = it->key;
            const bool available = it->value;
    #else
            const API_Guid guid = *it->key;
            const bool available = *it->value;
    #endif
            const API_Element *element = existing.GetPtr (guid);
            if (!available || element == nullptr || GetElemTypeID (*element) != type)
                continue;
            if (type == API_WallID && (target.wall.type == APIWtyp_Poly || element->wall.type == APIWtyp_Poly ||
                                       target.wall.modelElemStructureType == API_ProfileStructure ||
                                       element->wall.modelElemStructureType == API_ProfileStructure))
                continue;
            if (type == API_ObjectID && element->object.libInd != target.object.libInd)
                continue;
            if (type == API_BeamID &&
                (target.beam.nSegments != 1 || element->beam.nSegments != 1 || target.beam.nSchemes != 1 ||
                 element->beam.nSchemes != 1 || target.beam.nCuts != 2 || element->beam.nCuts != 2))
                continue;
            if (type == API_RoofID &&
                (target.roof.roofClass != API_PlaneRoofID || element->roof.roofClass != API_PlaneRoofID))
                continue;
            if (type != API_WallID && type != API_ObjectID && type != API_BeamID && type != API_SlabID &&
                type != API_RoofID)
                continue;
            const std::string key = APIGuidToString (guid).ToCStr ().Get ();
            if (selected == APINULLGuid || key < selectedKey) {
                selected = guid;
                selectedKey = key;
            }
        }
        if (selected != APINULLGuid)
            pool.Delete (selected);
        return selected;
    }

    static bool OtdLinearElement_CanDraw (const OtdLinearElement &geom, bool requireHeight);

    // Соответствия назначаются только окончательным участкам; один GUID нельзя использовать в двух зонах.
    static GSErrCode OtdRooms_AssignExistingWallFinishes (OtdRooms &rooms,
                                                          GS::HashTable<API_Guid, UnicGuidByBase> &byZone,
                                                          const ClassificationFunc::ClassificationDict &finclass) {
        GS::HashTable<API_Guid, API_Element> existing;
        UnicGuid used;
        for (auto it = rooms.EnumeratePairs (); it != nullptr; ++it) {
    #ifdef ServerMainVers_2800
            const OtdRoom &room = it->value;
    #else
            const OtdRoom &room = *it->value;
    #endif
            for (const auto &slab : room.otdslab)
                if (slab.draw.otd_guid != APINULLGuid)
                    used.Add (slab.draw.otd_guid, true);
            for (const auto &wall : room.otdwall)
                if (wall.draw.otd_guid != APINULLGuid)
                    used.Add (wall.draw.otd_guid, true);
            for (const auto *beams : {&room.otdbeam_floor, &room.otdbeam_ceil})
                for (const auto &beam : *beams)
                    if (beam.draw.otd_guid != APINULLGuid)
                        used.Add (beam.draw.otd_guid, true);
        }
        for (auto roomIt = rooms.EnumeratePairs (); roomIt != nullptr; ++roomIt) {
    #ifdef ServerMainVers_2800
            OtdRoom &room = roomIt->value;
    #else
            OtdRoom &room = *roomIt->value;
    #endif
            auto *byBase = byZone.GetPtr (room.zone_guid);
            if (byBase == nullptr)
                continue;
            const auto findPool = [&] (const OtdDrawData &draw) -> UnicGuid * {
                const API_Guid classGuid = Class_GetClassGuid (draw.type, finclass);
                if (classGuid == APINULLGuid)
                    return nullptr;
                auto *byType = byBase->GetPtr (draw.base_guid);
                return byType == nullptr
                           ? nullptr
                           : byType->GetPtr (Class_GetOtdTypeByClass (classGuid, finclass, API_ZombieElemID));
            };
            const auto assign = [&] (OtdDrawData &draw, const API_Element &target, UnicGuid &pool) -> GSErrCode {
                UnicGuid candidates;
                for (auto it = pool.EnumeratePairs (); it != nullptr; ++it) {
    #ifdef ServerMainVers_2800
                    const API_Guid guid = it->key;
                    const bool available = it->value;
    #else
                    const API_Guid guid = *it->key;
                    const bool available = *it->value;
    #endif
                    if (!available || used.ContainsKey (guid))
                        continue;
                    if (!existing.ContainsKey (guid)) {
                        API_Element element = {};
                        element.header.guid = guid;
                        const GSErrCode err = ACAPI_Element_Get (&element);
                        if (err != NoError)
                            return err;
                        existing.Add (guid, element);
                    }
                    if (GetElemTypeID (target) == API_BeamID && GetElemTypeID (existing.Get (guid)) == API_BeamID) {
                        API_ElementMemo memo = {};
                        const GS::OnExit memoGuard ([&] () { ACAPI_DisposeElemMemoHdls (&memo); });
                        const GSErrCode err =
                            ACAPI_Element_GetMemo (guid,
                                                   &memo,
                                                   APIMemoMask_BeamSegment | APIMemoMask_AssemblySegmentScheme |
                                                       APIMemoMask_AssemblySegmentCut);
                        if (err != NoError)
                            return err;
                        if (memo.beamSegments == nullptr || memo.assemblySegmentSchemes == nullptr ||
                            memo.assemblySegmentCuts == nullptr)
                            continue;
                    }
                    candidates.Add (guid, true);
                }
                draw.otd_guid = OtdWall_TakeExistingFinish (candidates, target, existing);
                if (draw.otd_guid != APINULLGuid) {
                    used.Add (draw.otd_guid, true);
                    pool.Delete (draw.otd_guid);
                }
                return NoError;
            };
            for (auto &wall : room.otdwall) {
                if (!OtdWall_ShouldDrawFinish (room, wall) || wall.draw.otd_guid != APINULLGuid ||
                    wall.geom.height < min_dim ||
                    std::hypot (wall.geom.seg.endC.x - wall.geom.seg.begC.x,
                                wall.geom.seg.endC.y - wall.geom.seg.begC.y) < min_dim)
                    continue;
                auto *pool = findPool (wall.draw);
                if (pool == nullptr || pool->IsEmpty ())
                    continue;
                API_Element target = {};
                API_ElementMemo memo = {};
                const GS::OnExit memoGuard ([&] () { ACAPI_DisposeElemMemoHdls (&memo); });
                bool ready = false;
                if (wall.surface_geometry || wall.draw.favorite.type == API_WallID) {
                    ready = Wall_GetDefault (wall.draw.favorite.name, wall.draw.type, target);
                    if (wall.surface_geometry)
                        target.wall.type = APIWtyp_Normal;
                } else if (wall.draw.favorite.type == API_ObjectID && wall.draw.draw_type != API_BeamID) {
                    ready = Object_GetDefault (wall.draw.favorite.name, wall.draw.type, target, memo);
                } else if (wall.draw.favorite.type == API_BeamID) {
                    ready = Beam_GetDefault (wall.draw.favorite.name, wall.draw.type, target, memo);
                } else {
                    continue; // Незавершённый Object-writer горизонтального откоса не активируем.
                }
                if (!ready)
                    return APIERR_BADPARS;
                const GSErrCode err = assign (wall.draw, target, *pool);
                if (err != NoError)
                    return err;
            }
            for (auto &slab : room.otdslab) {
                const double area = slab.poly.CalcArea ();
                if (!OtdSlab_ShouldDrawFinish (room, slab) || !slab.surface_geometry ||
                    slab.draw.otd_guid != APINULLGuid || !std::isfinite (area) || area < 0.0001 ||
                    !std::isfinite (slab.zBottom))
                    continue;
                auto *pool = findPool (slab.draw);
                if (pool == nullptr || pool->IsEmpty ())
                    continue;
                API_Element target = {};
                API_ElementMemo memo = {};
                const GS::OnExit memoGuard ([&] () { ACAPI_DisposeElemMemoHdls (&memo); });
                bool ready = false;
                if (slab.draw.base_type == API_MeshID) {
                    SetElemTypeID (target, API_RoofID);
                    target.roof.roofClass = API_PlaneRoofID;
                    ready = true; // Геометрию и избранное уже проверил RoofBatch_Validate.
                } else if (slab.draw.favorite.type == API_SlabID) {
                    ready = Slab_GetDefault (slab.draw.favorite.name, slab.draw.type, target);
                } else if (slab.draw.favorite.type == API_ObjectID) {
                    ready = Object_GetDefault (slab.draw.favorite.name, slab.draw.type, target, memo);
                }
                if (!ready)
                    return APIERR_BADPARS;
                const GSErrCode err = assign (slab.draw, target, *pool);
                if (err != NoError)
                    return err;
            }
            if (!room.isValid)
                continue;
            for (auto *beams : {&room.otdbeam_floor, &room.otdbeam_ceil}) {
                for (auto &beam : *beams) {
                    if (beam.draw.otd_guid != APINULLGuid || beam.draw.favorite.type != API_BeamID ||
                        !OtdLinearElement_CanDraw (beam.geom, false))
                        continue;
                    auto *pool = findPool (beam.draw);
                    if (pool == nullptr || pool->IsEmpty ())
                        continue;
                    API_Element target = {};
                    API_ElementMemo memo = {};
                    const GS::OnExit memoGuard ([&] () { ACAPI_DisposeElemMemoHdls (&memo); });
                    if (!Beam_GetDefault (beam.draw.favorite.name, beam.draw.type, target, memo))
                        return APIERR_BADPARS;
                    if (target.beam.nSegments != 1 || target.beam.nSchemes != 1 || target.beam.nCuts != 2 ||
                        memo.beamSegments == nullptr || memo.assemblySegmentSchemes == nullptr ||
                        memo.assemblySegmentCuts == nullptr)
                        continue;
                    const GSErrCode err = assign (beam.draw, target, *pool);
                    if (err != NoError)
                        return err;
                }
            }
        }
        // Удаляем использованные GUID и из возможных повторных записей индекса.
        for (auto zoneIt = byZone.EnumeratePairs (); zoneIt != nullptr; ++zoneIt) {
    #ifdef ServerMainVers_2800
            auto &bases = zoneIt->value;
    #else
            auto &bases = *zoneIt->value;
    #endif
            for (auto baseIt = bases.EnumeratePairs (); baseIt != nullptr; ++baseIt) {
    #ifdef ServerMainVers_2800
                auto &types = baseIt->value;
    #else
                auto &types = *baseIt->value;
    #endif
                for (auto typeIt = types.EnumeratePairs (); typeIt != nullptr; ++typeIt) {
    #ifdef ServerMainVers_2800
                    auto &pool = typeIt->value;
    #else
                    auto &pool = *typeIt->value;
    #endif
                    for (auto usedIt = used.EnumeratePairs (); usedIt != nullptr; ++usedIt) {
    #ifdef ServerMainVers_2800
                        pool.Delete (usedIt->key);
    #else
                        pool.Delete (*usedIt->key);
    #endif
                    }
                }
            }
        }
        return NoError;
    }

    // -----------------------------------------------------------------------------
    // Связи отделки с основаниями и зонами; общая запись накопленных свойств, включая сводку площадей.
    // -----------------------------------------------------------------------------
    static void OtdElements_SetSync (UnicElementByType &subelementByparent,
                                     ParamDictElement &paramToWrite,
                                     const API_ElemTypeID *elementTypes,
                                     std::size_t elementTypeCount,
                                     GS::Int32 &nPhase) {
        API_Elem_Head parentelementhead;
        UnicGuid syncguidsdict;
        GS::UniString suffix;
        GS::Array<API_Guid> syncguids;
        GS::UniString funcname = "Create link with base element";
        SuspendGroupsGuard suspGuard; // отключаем режим группировки на время записи.
        bool meshListed = false;
        for (std::size_t i = 0; i < elementTypeCount; ++i)
            meshListed = meshListed || elementTypes[i] == API_MeshID;
        for (std::size_t typeIndex = 0; typeIndex <= elementTypeCount; ++typeIndex) {
            if (typeIndex == elementTypeCount && meshListed)
                continue;
            const API_ElemTypeID typeelem = typeIndex < elementTypeCount ? elementTypes[typeIndex] : API_MeshID;
            if (!subelementByparent.ContainsKey (typeelem))
                continue;
            for (UnicElement::PairIterator cIt = subelementByparent.Get (typeelem).EnumeratePairs (); cIt != NULL;
                 ++cIt) {
    #ifdef ServerMainVers_2800
                API_Guid guid = cIt->key;
                const GS::Array<API_Guid> &subguids = cIt->value;
    #else
                API_Guid guid = *cIt->key;
                const GS::Array<API_Guid> &subguids = *cIt->value;
    #endif

                if (AdvanceProcessPhase (funcname, nPhase))
                    return;

                parentelementhead.guid = guid;
                if (typeelem == API_ZoneID) {
                    suffix = "zone";
                } else {
                    suffix = "base element";
                }
                if (!SyncSetSubelementScope (parentelementhead, subguids, paramToWrite, suffix, false)) {
                    continue;
                }
                for (const auto &g : subguids) {
                    if (!syncguidsdict.ContainsKey (g)) {
                        syncguidsdict.Add (g, true);
                        syncguids.Push (g);
                    }
                }
            }
        }
        if (!paramToWrite.IsEmpty ()) {
            funcname = GS::UniString::Printf ("Write GUID base and GUID zone to %d finishing element(s)",
                                              paramToWrite.GetSize ());
            if (AdvanceProcessPhase (funcname, nPhase)) {
                return;
            }
            ACAPI_CallUndoableCommand ("Write property to finishing element", [&] () -> GSErrCode {
                ParamHelpers::ElementsWrite (paramToWrite);
                return NoError;
            });
        }
        if (syncguids.IsEmpty ())
            return;
        funcname = GS::UniString::Printf ("Sync %d finishing element with base and zone", paramToWrite.GetSize ());
        if (AdvanceProcessPhase (funcname, nPhase)) {
            return;
        }
        SyncSettings syncSettings;
        LoadSyncSettingsFromPreferences (syncSettings);
        if (!syncSettings.GetSyncMon ())
            return;
        GS::Array<API_Guid> rereadelem = SyncArray (syncSettings, syncguids);
        if (!rereadelem.IsEmpty ()) {
            SyncArray (syncSettings, rereadelem);
        }
    }

    static bool OtdLinearElement_CanDraw (const OtdLinearElement &geom, bool requireHeight) {
        const auto finite = [] (const API_Coord3D &point) {
            return std::isfinite (point.x) && std::isfinite (point.y) && std::isfinite (point.z);
        };
        if (!finite (geom.seg.begC) || !finite (geom.seg.endC))
            return false;
        // Всем линейным элементам нужна ненулевая ось в плане, не только длина в 3D.
        const double length = std::hypot (geom.seg.endC.x - geom.seg.begC.x, geom.seg.endC.y - geom.seg.begC.y);
        if (!std::isfinite (length) || length < min_dim || !std::isfinite (geom.seg.endC.z - geom.seg.begC.z))
            return false;
        // Нулевая необязательная высота сохраняет сечение избранного.
        return std::isfinite (geom.height) && ((!requireHeight && geom.height == 0) || geom.height >= min_dim);
    }

    // Верх проёма подготовлен слева направо: глубина лежит справа от оси.
    // Столбцы сетки Beam anchorPoint: 0/3/6 — слева, 1/4/7 — центр, 2/5/8 — справа.
    static bool UpperReveal_GetBeamOffset (short anchorPoint, double width, double &offset) {
        if (anchorPoint < 0 || anchorPoint > 8 || !std::isfinite (width) || width <= min_dim)
            return false;
        offset = -width * (2 - anchorPoint % 3) / 2;
        return true;
    }

    static bool UpperReveal_GetWallOffset (const API_WallType &wall, double &offset) {
        if (!std::isfinite (wall.thickness) || wall.thickness <= min_dim)
            return false;
        // При flipped меняются стороны тела, но не направление подготовленной оси.
        switch (wall.referenceLineLocation) {
        case APIWallRefLine_Outside:
            offset = wall.flipped ? -wall.thickness : 0;
            return true;
        case APIWallRefLine_Inside:
            offset = wall.flipped ? 0 : -wall.thickness;
            return true;
        default:
            return false; // Center/CoreCenter не допускают offset; core-привязки требуют Composite.
        }
    }

    // -----------------------------------------------------------------------------
    // Создание балки
    // -----------------------------------------------------------------------------
    static void Beam_Draw (const Stories &storyLevels, const OtdLinearElement &geom, OtdDrawData &draw) {
        const API_Guid otd_guid = draw.otd_guid;
        draw.otd_guid = APINULLGuid;
        draw.draw_error = APIERR_BADPARS;
        if (!OtdLinearElement_CanDraw (geom, false))
            return;
        const double planLength = std::hypot (geom.seg.endC.x - geom.seg.begC.x, geom.seg.endC.y - geom.seg.begC.y);
        const double dz = geom.seg.endC.z - geom.seg.begC.z;
        const double level = GetOffsetFromStory (geom.seg.begC.z, geom.floorInd, storyLevels);
        if (!std::isfinite (level))
            return;
        API_Element beamelement = {};
        API_ElementMemo beammemo = {};
        API_Element settings = {};
        API_ElementMemo settingsMemo = {};
        API_Element mask = {};
        const GS::OnExit memoGuard ([&beammemo] () { ACAPI_DisposeElemMemoHdls (&beammemo); });
        const GS::OnExit settingsGuard ([&settingsMemo] () { ACAPI_DisposeElemMemoHdls (&settingsMemo); });
        if (!Beam_GetDefault (draw.favorite.name, draw.type, settings, settingsMemo))
            return;
        const bool is_new = (otd_guid == APINULLGuid);
        if (is_new) {
            beamelement = settings;
            std::swap (beammemo, settingsMemo);
        } else {
            if (!GetExsist (otd_guid, beamelement, beammemo))
                return;
        }
        if (GetElemTypeID (beamelement) != API_BeamID || beamelement.beam.nSegments != 1 ||
            beamelement.beam.nSchemes != 1 || beamelement.beam.nCuts != 2 || beammemo.beamSegments == nullptr ||
            beammemo.assemblySegmentSchemes == nullptr || beammemo.assemblySegmentCuts == nullptr)
            return;
        beamelement.header.guid = otd_guid;
        API_Coord begC = {geom.seg.begC.x, geom.seg.begC.y}; // begin point of the beam reference line
        API_Coord endC = {geom.seg.endC.x, geom.seg.endC.y};
        bool needupdate = Elems_Check_Update (
            beamelement.header, mask, geom.floorInd, settings.header.layer, settings.header.renovationStatus);
        if (beamelement.beam.begC.x != begC.x || beamelement.beam.begC.y != begC.y) {
            ACAPI_ELEMENT_MASK_SET (mask, API_BeamType, begC);
            beamelement.beam.begC = begC;
            needupdate = true;
        }
        if (beamelement.beam.endC.x != endC.x || beamelement.beam.endC.y != endC.y) {
            ACAPI_ELEMENT_MASK_SET (mask, API_BeamType, endC);
            beamelement.beam.endC = endC;
            needupdate = true;
        }
        if (beamelement.beam.level != level) {
            ACAPI_ELEMENT_MASK_SET (mask, API_BeamType, level);
            beamelement.beam.level = level;
            needupdate = true;
        }
        if (beamelement.beam.curveAngle != 0) {
            ACAPI_ELEMENT_MASK_SET (mask, API_BeamType, curveAngle);
            beamelement.beam.curveAngle = 0;
            needupdate = true;
        }
        if (beamelement.beam.verticalCurveHeight != 0) {
            ACAPI_ELEMENT_MASK_SET (mask, API_BeamType, verticalCurveHeight);
            beamelement.beam.verticalCurveHeight = 0;
            needupdate = true;
        }
        if (beamelement.beam.isSlanted != (dz != 0)) {
            ACAPI_ELEMENT_MASK_SET (mask, API_BeamType, isSlanted);
            beamelement.beam.isSlanted = dz != 0;
            needupdate = true;
        }
        const double slantAngle = std::atan2 (dz, planLength);
        if (beamelement.beam.slantAngle != slantAngle) {
            ACAPI_ELEMENT_MASK_SET (mask, API_BeamType, slantAngle);
            beamelement.beam.slantAngle = slantAngle;
            needupdate = true;
        }
        // Один отрезок задаёт всю длину балки, независимо от фиксированной длины шаблона.
        if (beammemo.assemblySegmentSchemes[0].lengthType != APIAssemblySegment_Proportional) {
            beammemo.assemblySegmentSchemes[0].lengthType = APIAssemblySegment_Proportional;
            needupdate = true;
        }
        if (beammemo.assemblySegmentSchemes[0].lengthProportion != 1) {
            beammemo.assemblySegmentSchemes[0].lengthProportion = 1;
            needupdate = true;
        }
        if (beammemo.assemblySegmentSchemes[0].fixedLength != 0) {
            beammemo.assemblySegmentSchemes[0].fixedLength = 0;
            needupdate = true;
        }
        // Нулевые размеры и материал означают сохранение соответствующих настроек шаблона.
        if (!is_equal (geom.width, 0) && geom.width > min_dim &&
            beammemo.beamSegments[0].assemblySegmentData.nominalWidth != geom.width) {
            beammemo.beamSegments[0].assemblySegmentData.nominalWidth = geom.width;
            needupdate = true;
        }
        if (!is_equal (geom.height, 0) && geom.height > min_dim &&
            beammemo.beamSegments[0].assemblySegmentData.nominalHeight != geom.height) {
            beammemo.beamSegments[0].assemblySegmentData.nominalHeight = geom.height;
            needupdate = true;
        }
        if (draw.is_upper_reveal) {
            const auto &segment = beammemo.beamSegments[0].assemblySegmentData;
            double offset = beamelement.beam.offset;
            if (segment.modelElemStructureType != API_BasicStructure || segment.circleBased ||
                !is_equal (beamelement.beam.profileAngle, 0) ||
                !UpperReveal_GetBeamOffset (beamelement.beam.anchorPoint, segment.nominalWidth, offset))
                return;
            if (beamelement.beam.offset != offset) {
                ACAPI_ELEMENT_MASK_SET (mask, API_BeamType, offset);
                beamelement.beam.offset = offset;
                needupdate = true;
            }
        }
        if (draw.material.material > 0) {
    #ifdef ServerMainVers_2700
            const API_AttributeIndex beamMaterial = ACAPI_CreateAttributeIndex (draw.material.material);
            for (auto *material : {&beammemo.beamSegments[0].bottomMaterial,
                                   &beammemo.beamSegments[0].leftMaterial,
                                   &beammemo.beamSegments[0].rightMaterial,
                                   &beammemo.beamSegments[0].topMaterial,
                                   &beammemo.beamSegments[0].endsMaterial}) {
                if (!material->hasValue || material->value != beamMaterial) {
                    material->hasValue = true;
                    material->value = beamMaterial;
                    needupdate = true;
                }
            }
    #else
            for (auto *material : {&beammemo.beamSegments[0].bottomMaterial,
                                   &beammemo.beamSegments[0].leftMaterial,
                                   &beammemo.beamSegments[0].rightMaterial}) {
                if (!material->overridden || material->attributeIndex != draw.material.material) {
                    material->overridden = true;
                    material->attributeIndex = draw.material.material;
                    needupdate = true;
                }
            }
    #endif
        }
        GSErrCode err = NoError;
        if (is_new) {
            err = ACAPI_Element_Create (&beamelement, &beammemo);
        } else if (needupdate) {
            err = ACAPI_Element_Change (
                &beamelement, &mask, &beammemo, APIMemoMask_BeamSegment | APIMemoMask_AssemblySegmentScheme, true);
        }
        draw.draw_error = err;
        if (err != NoError) {
            msg_rep ("Beam_Draw err", is_new ? "ACAPI_Element_Create" : "ACAPI_Element_Change", err, otd_guid);
            return;
        }
        draw.material.structtype = beammemo.beamSegments[0].assemblySegmentData.modelElemStructureType;
        switch (draw.material.structtype) {
        case API_BasicStructure:
            draw.material.bmaterial =
                AttributeIndexGetShort (beammemo.beamSegments[0].assemblySegmentData.buildingMaterial);
            break;
        case API_CompositeStructure:
            draw.material.bmaterial = AttributeIndexGetShort (beammemo.beamSegments[0].assemblySegmentData.profileAttr);
            break;
        case API_ProfileStructure:
            draw.material.bmaterial = 0;
            break;
        }
        draw.otd_guid = beamelement.header.guid;
        draw.write_succeeded = true;
        draw.was_created = is_new;
    }

    // -----------------------------------------------------------------------------
    // ParamToMemo возвращает полноту поиска, а не наличие отличий. Убираем равные
    // параметры до записи; отсутствующие и неподдерживаемые параметры не меняют объект.
    static bool Object_UpdateParams (API_ElementMemo &memo, GDLHelpers::ParamDict &params) {
        if (memo.params == nullptr)
            return false;
        bool needupdate = false;
        const GSSize count = BMGetHandleSize ((GSHandle)memo.params) / sizeof (API_AddParType);
        for (GSIndex i = 0; i < count; ++i) {
            const API_AddParType &current = (*memo.params)[i];
            GS::UniString name = GDLNAMEPREFIX;
            name += GS::UniString (current.name);
            name.SetToLowerCase ();
            name += BRACEEND;
            const auto *value = params.GetPtr (name);
            if (value == nullptr)
                continue;
            bool different = false;
            switch (current.typeID) {
            case APIParT_LineTyp:
            case APIParT_Profile:
            case APIParT_BuildingMaterial:
            case APIParT_FillPat:
            case APIParT_Mater:
            case APIParT_Boolean:
            case APIParT_Integer:
            case APIParT_PenCol:
            case APIParT_Length:
            case APIParT_RealNum:
            case APIParT_ColRGB:
            case APIParT_Intens:
            case APIParT_Angle:
                if (current.typeMod == API_ParSimple) {
                    different = current.value.real != value->num;
                } else {
                    different = current.dim1 != value->dim1 || current.dim2 != value->dim2;
                    const USize size = (USize)value->dim1 * value->dim2;
                    if (!different && value->arr_num.GetSize () >= size && size > 0) {
                        different = current.value.array == nullptr ||
                                    BMGetHandleSize (current.value.array) < size * sizeof (double);
                        for (USize j = 0; j < size && !different; ++j)
                            different = ((double *)*current.value.array)[j] != value->arr_num[j];
                    }
                }
                break;
            case APIParT_CString:
                if (current.typeMod == API_ParSimple) {
                    const GS::UniString target (
                        value->str.ToUStr (0, GS::Min (value->str.GetLength (), (USize)(API_UAddParStrLen - 1)))
                            .Get ());
                    different = GS::UniString (current.value.uStr) != target;
                }
                break;
            default:
                break;
            }
            if (different)
                needupdate = true;
            else
                params.Delete (name);
        }
        if (needupdate)
            GDLHelpers::ParamToMemo (memo, params);
        return needupdate;
    }

    // Построение отделочных стен аксессуаром
    // -----------------------------------------------------------------------------
    static void Object_Draw_Linear (const Stories &storyLevels,
                                    const OtdLinearElement &geom,
                                    OtdDrawData &draw,
                                    GS::Array<OtdOpening> &openings) {
        const double level = GetOffsetFromStory (geom, storyLevels);
        if (!OtdLinearElement_CanDraw (geom, true) || !std::isfinite (level)) {
            draw.otd_guid = APINULLGuid;
            draw.draw_error = APIERR_BADPARS;
            return;
        }
        API_ElemTypeID type = draw.favorite.type;
        API_Guid otd_guid = draw.otd_guid;
        bool is_new = (otd_guid == APINULLGuid);
        GSErrCode err = NoError;
        API_Element wallobjelement = {};
        API_ElementMemo wallobjmemo = {};
        API_Element settings = {};
        API_ElementMemo settingsMemo = {};
        API_Element mask = {};
        const GS::OnExit memoGuard ([&wallobjmemo] () { ACAPI_DisposeElemMemoHdls (&wallobjmemo); });
        const GS::OnExit settingsGuard ([&settingsMemo] () { ACAPI_DisposeElemMemoHdls (&settingsMemo); });
        if (!Object_GetDefault (draw.favorite.name, draw.type, settings, settingsMemo))
            return;
        if (is_new) {
            wallobjelement = settings;
            std::swap (wallobjmemo, settingsMemo);
        } else {
            if (!GetExsist (draw.otd_guid, wallobjelement, wallobjmemo))
                return;
            if (GetElemTypeID (wallobjelement) != API_ObjectID) {
                return;
            }
        }
        Point2D wbegC = {geom.seg.begC.x, geom.seg.begC.y};
        Point2D wendC = {geom.seg.endC.x, geom.seg.endC.y};
        Sector walledge = {wbegC, wendC};
        GS::Optional<UnitVector_2D> walldir = walledge.GetDirection ();
        if (!walldir.HasValue ()) {
            return;
        }
        double ac_wall_length = walledge.GetLength ();
        GDLHelpers::ParamDict accsessoryparams;
        GDLHelpers::Param p;
        p.num = 1;
        accsessoryparams.Add ("{@gdl:ac_refside}", p);
        p.num = draw.material.material;
        accsessoryparams.Add ("{@gdl:gs_bw_mat}", p);
        p.num = draw.material.material;
        accsessoryparams.Add ("{@gdl:matp1}", p);

        p.num = geom.height;
        accsessoryparams.Add ("{@gdl:ac_wall_height}", p);
        p.num = ac_wall_length;
        accsessoryparams.Add ("{@gdl:ac_wall_length}", p);
        p.num = 0;
        accsessoryparams.Add ("{@gdl:ac_wall_radius}", p);

        p.dim1 = 1;
        p.dim2 = 2;
        p.arr_num.Push (0);
        p.arr_num.Push (0);
        accsessoryparams.Add ("{@gdl:ac_side_poly}", p);
        p.arr_num.Clear ();

        p.dim1 = 2;
        p.dim2 = 2;
        p.arr_num.Push (0);
        p.arr_num.Push (geom.height);
        p.arr_num.Push (ac_wall_length);
        p.arr_num.Push (geom.height);
        accsessoryparams.Add ("{@gdl:ac_top_poly}", p);
        p.arr_num.Clear ();

        p.dim1 = 2;
        p.dim2 = 2;
        p.arr_num.Push (0);
        p.arr_num.Push (0);
        p.arr_num.Push (ac_wall_length);
        p.arr_num.Push (0);
        accsessoryparams.Add ("{@gdl:ac_bot_poly}", p);
        p.arr_num.Clear ();

        p.dim1 = 1;
        p.dim2 = 2;
        p.arr_num.Push (geom.ang_begC);
        p.arr_num.Push (geom.ang_endC);
        accsessoryparams.Add ("{@gdl:ac_angles}", p);
        p.arr_num.Clear ();

        if (openings.IsEmpty ()) {
            p.dim1 = 1;
            p.dim2 = 2;
            p.arr_num.Push (0);
            p.arr_num.Push (0);
        } else {
            Int32 dim2 = 0;
            UInt32 addedOpenings = 0;
            for (OtdOpening &op : openings) {
                if (op.width < min_dim || op.height < min_dim)
                    continue;
                ++addedOpenings;
                double objLoc = ac_wall_length - op.objLoc;
                double halfwidth = op.width / 2;

                p.arr_num.Push (0.106);
                p.arr_num.Push (objLoc - halfwidth);
                p.arr_num.Push (op.lower);

                p.arr_num.Push (objLoc - halfwidth);
                p.arr_num.Push (op.lower + op.height);

                p.arr_num.Push (objLoc + halfwidth);
                p.arr_num.Push (op.lower + op.height);

                p.arr_num.Push (objLoc + halfwidth);
                p.arr_num.Push (op.lower);
            }
            p.dim1 = addedOpenings;
            p.dim2 = 9;
        }
        accsessoryparams.Add ("{@gdl:ac_wd_poly}", p);
        p.arr_num.Clear ();
        bool needupdate = Object_UpdateParams (wallobjmemo, accsessoryparams);
        Vector2D ref = {-1, 0};
        Vector2D wallvect = walldir.Get ().ToVector2D ();
        double ang = wallvect.CalcAngleToReference (ref);
        if (wallobjelement.object.angle != ang) {
            wallobjelement.object.angle = ang;
            needupdate = true;
        }
        if (wallobjelement.object.pos.x != geom.seg.endC.x || wallobjelement.object.pos.y != geom.seg.endC.y) {
            wallobjelement.object.pos = {geom.seg.endC.x, geom.seg.endC.y};
            needupdate = true;
        }
        needupdate =
            Elems_Check_Update (
                wallobjelement.header, mask, geom.floorInd, settings.header.layer, settings.header.renovationStatus) ||
            needupdate;
    #ifdef ServerMainVers_2700
        API_AttributeIndex ematerial = ACAPI_CreateAttributeIndex (draw.material.material);
        if (wallobjelement.object.mat != ematerial) {
            wallobjelement.object.mat = ematerial;
            needupdate = true;
        }
    #else
        if (wallobjelement.object.mat != draw.material.material) {
            wallobjelement.object.mat = draw.material.material;
            needupdate = true;
        }
    #endif
        if (wallobjelement.object.level != level) {
            wallobjelement.object.level = level;
            needupdate = true;
        }
        if (is_new) {
            err = ACAPI_Element_Create (&wallobjelement, &wallobjmemo);
        } else if (needupdate) {
            ACAPI_ELEMENT_MASK_SET (mask, API_ObjectType, pos);
            ACAPI_ELEMENT_MASK_SET (mask, API_ObjectType, angle);
            ACAPI_ELEMENT_MASK_SET (mask, API_ObjectType, level);
            ACAPI_ELEMENT_MASK_SET (mask, API_ObjectType, mat);
            err = ACAPI_Element_Change (&wallobjelement, &mask, &wallobjmemo, APIMemoMask_AddPars, true);
        }
        draw.draw_error = err;
        if (err != NoError) {
            if (wallobjelement.object.libInd == 0) {
                GS::UniString message =
                    "Cannot create or update the wall finish: the GDL object has no valid library part (library index is 0). Favorite: '";
                message += draw.favorite.name;
                message +=
                    "'. Check that the required library is loaded and reselect the GDL object in the Favorite. If no Favorite is used, check the Object tool defaults.";
                msg_rep ("RoomBook", message, err, draw.base_guid);
            } else {
                msg_rep ("Draw_Object_Linear err", "ACAPI_Element_Create", err, APINULLGuid);
            }
            return;
        }
        draw.otd_guid = wallobjelement.header.guid;
        draw.write_succeeded = true;
        draw.was_created = is_new;
    }

    // -----------------------------------------------------------------------------
    // Построение отделочных стен стандартной стеной
    // -----------------------------------------------------------------------------
    static void Wall_Draw (const Stories &storyLevels,
                           OtdLinearElement &geom,
                           OtdDrawData &draw,
                           double slant_angle,
                           GS::Array<OtdOpening> &openings) {
        const API_Guid existingGuid = draw.otd_guid;
        draw.otd_guid = APINULLGuid;
        draw.draw_error = APIERR_BADPARS;
        if (!OtdLinearElement_CanDraw (geom, true))
            return;
        const double level = GetOffsetFromStory (geom, storyLevels);
        if (!std::isfinite (level))
            return;
        GSErrCode err = NoError;
        API_Element wallelement = {};
        const bool is_upper_reveal = draw.is_upper_reveal;
        // Общий Wall_GetDefault нормализует привязку; верх проёма должен сохранить избранное.
        if (!Wall_GetDefault (draw.favorite.name, draw.type, wallelement)) {
            return;
        }
        if (is_upper_reveal) {
            if (wallelement.wall.modelElemStructureType != API_BasicStructure ||
                wallelement.wall.type != APIWtyp_Normal)
                return;
            if (!is_equal (geom.width, 0) && geom.width > 0)
                wallelement.wall.thickness = geom.width;
            wallelement.wall.thickness1 = wallelement.wall.thickness;
            if (!UpperReveal_GetWallOffset (wallelement.wall, wallelement.wall.offset))
                return;
        }
        // Верхний откос уже ориентирован при подготовке; повторный разворот меняет сторону глубины.
        wallelement.wall.begC = {geom.seg.begC.x, geom.seg.begC.y};
        wallelement.wall.endC = {geom.seg.endC.x, geom.seg.endC.y};
        wallelement.wall.height = geom.height;
        if (slant_angle > 0 && !is_upper_reveal) {
            wallelement.wall.profileType = is_equal (slant_angle, 90 * DEGRAD) ? APISect_Normal : APISect_Slanted;
            wallelement.wall.slantAlpha = slant_angle;
            wallelement.wall.relativeTopStory = 0;
        }
        wallelement.header.floorInd = geom.floorInd;
        wallelement.wall.bottomOffset = level;
    #ifdef ServerMainVers_2700
        API_AttributeIndex ematerial = ACAPI_CreateAttributeIndex (draw.material.material);
        wallelement.wall.refMat.hasValue = true;
        wallelement.wall.oppMat.hasValue = true;
        wallelement.wall.sidMat.hasValue = true;
        wallelement.wall.refMat.value = ematerial;
        wallelement.wall.oppMat.value = ematerial;
        wallelement.wall.sidMat.value = ematerial;
    #else
        wallelement.wall.refMat.overridden = true;
        wallelement.wall.oppMat.overridden = true;
        wallelement.wall.sidMat.overridden = true;
        wallelement.wall.refMat.attributeIndex = draw.material.material;
        wallelement.wall.oppMat.attributeIndex = draw.material.material;
        wallelement.wall.sidMat.attributeIndex = draw.material.material;
    #endif
        wallelement.wall.zoneRel = APIZRel_None;
        if (!is_upper_reveal)
            wallelement.wall.flipped = false;
        if (existingGuid == APINULLGuid) {
            err = ACAPI_Element_Create (&wallelement, nullptr);
            if (err != NoError) {
                draw.draw_error = err;
                return;
            }
        } else {
            API_Element current = {};
            current.header.guid = existingGuid;
            err = ACAPI_Element_Get (&current);
            if (err != NoError) {
                draw.draw_error = err;
                return;
            }
            if (GetElemTypeID (current) != API_WallID || current.wall.type == APIWtyp_Poly ||
                wallelement.wall.type == APIWtyp_Poly || current.wall.modelElemStructureType == API_ProfileStructure ||
                wallelement.wall.modelElemStructureType == API_ProfileStructure)
                return;
            GS::Array<API_Guid> oldOpenings;
            ClassificationFunc::ClassificationDict finclass;
            UnicGuid finclassguids;
            Class_FindFinClass (finclass, finclassguids);
            for (const API_ElemTypeID type : {API_WindowID, API_DoorID}) {
                const auto children = ConnectedElements_Read (existingGuid, type, APIFilt_None);
                if (children.error != NoError) {
                    draw.draw_error = children.error;
                    return;
                }
                for (const auto &guid : children.items) {
                    API_Guid classGuid = APINULLGuid;
                    if (!Class_IsElementFinClass (guid, finclassguids, classGuid, nullptr))
                        return; // Чужие hosted-элементы не удаляем и стену с ними не меняем.
                    oldOpenings.Push (guid);
                }
            }
            // Один раз на стену, даже когда новый список проёмов пуст.
            if (!oldOpenings.IsEmpty ()) {
                err = ACAPI_Element_Delete (oldOpenings);
                if (err != NoError) {
                    draw.draw_error = err;
                    return;
                }
            }
            wallelement.header.guid = existingGuid;
            API_Element mask = {};
            bool needupdate = Elems_Check_Update (current.header,
                                                  mask,
                                                  wallelement.header.floorInd,
                                                  wallelement.header.layer,
                                                  wallelement.header.renovationStatus);
            if (current.wall.begC.x != wallelement.wall.begC.x || current.wall.begC.y != wallelement.wall.begC.y) {
                current.wall.begC = wallelement.wall.begC;
                needupdate = true;
            }
            if (current.wall.endC.x != wallelement.wall.endC.x || current.wall.endC.y != wallelement.wall.endC.y) {
                current.wall.endC = wallelement.wall.endC;
                needupdate = true;
            }
            if (current.wall.height != wallelement.wall.height) {
                current.wall.height = wallelement.wall.height;
                needupdate = true;
            }
            if (current.wall.bottomOffset != wallelement.wall.bottomOffset) {
                current.wall.bottomOffset = wallelement.wall.bottomOffset;
                needupdate = true;
            }
            if (current.wall.topOffset != wallelement.wall.topOffset) {
                current.wall.topOffset = wallelement.wall.topOffset;
                needupdate = true;
            }
            if (current.wall.relativeTopStory != wallelement.wall.relativeTopStory) {
                current.wall.relativeTopStory = wallelement.wall.relativeTopStory;
                needupdate = true;
            }
            if (current.wall.thickness != wallelement.wall.thickness) {
                current.wall.thickness = wallelement.wall.thickness;
                needupdate = true;
            }
            if (current.wall.thickness1 != wallelement.wall.thickness1) {
                current.wall.thickness1 = wallelement.wall.thickness1;
                needupdate = true;
            }
            if (current.wall.type != wallelement.wall.type) {
                current.wall.type = wallelement.wall.type;
                needupdate = true;
            }
            if (current.wall.angle != wallelement.wall.angle) {
                current.wall.angle = wallelement.wall.angle;
                needupdate = true;
            }
            if (current.wall.referenceLineLocation != wallelement.wall.referenceLineLocation) {
                current.wall.referenceLineLocation = wallelement.wall.referenceLineLocation;
                needupdate = true;
            }
            if (current.wall.offset != wallelement.wall.offset) {
                current.wall.offset = wallelement.wall.offset;
                needupdate = true;
            }
            if (current.wall.modelElemStructureType != wallelement.wall.modelElemStructureType) {
                current.wall.modelElemStructureType = wallelement.wall.modelElemStructureType;
                needupdate = true;
            }
            if (wallelement.wall.modelElemStructureType == API_BasicStructure &&
                current.wall.buildingMaterial != wallelement.wall.buildingMaterial) {
                current.wall.buildingMaterial = wallelement.wall.buildingMaterial;
                needupdate = true;
            }
            if (wallelement.wall.modelElemStructureType == API_CompositeStructure &&
                current.wall.composite != wallelement.wall.composite) {
                current.wall.composite = wallelement.wall.composite;
                needupdate = true;
            }
            if (current.wall.profileType != wallelement.wall.profileType) {
                current.wall.profileType = wallelement.wall.profileType;
                needupdate = true;
            }
            if (current.wall.slantAlpha != wallelement.wall.slantAlpha) {
                current.wall.slantAlpha = wallelement.wall.slantAlpha;
                needupdate = true;
            }
            if (current.wall.slantBeta != wallelement.wall.slantBeta) {
                current.wall.slantBeta = wallelement.wall.slantBeta;
                needupdate = true;
            }
            for (const auto &pair : {std::make_pair (&current.wall.refMat, &wallelement.wall.refMat),
                                     std::make_pair (&current.wall.oppMat, &wallelement.wall.oppMat),
                                     std::make_pair (&current.wall.sidMat, &wallelement.wall.sidMat)}) {
    #ifdef ServerMainVers_2700
                if (pair.first->hasValue != pair.second->hasValue || pair.first->value != pair.second->value) {
    #else
                if (pair.first->overridden != pair.second->overridden ||
                    pair.first->attributeIndex != pair.second->attributeIndex) {
    #endif
                    *pair.first = *pair.second;
                    needupdate = true;
                }
            }
            if (current.wall.zoneRel != wallelement.wall.zoneRel) {
                current.wall.zoneRel = wallelement.wall.zoneRel;
                needupdate = true;
            }
            if (current.wall.flipped != wallelement.wall.flipped) {
                current.wall.flipped = wallelement.wall.flipped;
                needupdate = true;
            }
            wallelement = current;
            ACAPI_ELEMENT_MASK_SET (mask, API_WallType, begC);
            ACAPI_ELEMENT_MASK_SET (mask, API_WallType, endC);
            ACAPI_ELEMENT_MASK_SET (mask, API_WallType, height);
            ACAPI_ELEMENT_MASK_SET (mask, API_WallType, bottomOffset);
            ACAPI_ELEMENT_MASK_SET (mask, API_WallType, topOffset);
            ACAPI_ELEMENT_MASK_SET (mask, API_WallType, relativeTopStory);
            ACAPI_ELEMENT_MASK_SET (mask, API_WallType, thickness);
            ACAPI_ELEMENT_MASK_SET (mask, API_WallType, thickness1);
            ACAPI_ELEMENT_MASK_SET (mask, API_WallType, type);
            ACAPI_ELEMENT_MASK_SET (mask, API_WallType, angle);
            ACAPI_ELEMENT_MASK_SET (mask, API_WallType, referenceLineLocation);
            ACAPI_ELEMENT_MASK_SET (mask, API_WallType, offset);
            ACAPI_ELEMENT_MASK_SET (mask, API_WallType, modelElemStructureType);
            if (wallelement.wall.modelElemStructureType == API_BasicStructure)
                ACAPI_ELEMENT_MASK_SET (mask, API_WallType, buildingMaterial);
            if (wallelement.wall.modelElemStructureType == API_CompositeStructure)
                ACAPI_ELEMENT_MASK_SET (mask, API_WallType, composite);
            ACAPI_ELEMENT_MASK_SET (mask, API_WallType, profileType);
            ACAPI_ELEMENT_MASK_SET (mask, API_WallType, slantAlpha);
            ACAPI_ELEMENT_MASK_SET (mask, API_WallType, slantBeta);
            ACAPI_ELEMENT_MASK_SET (mask, API_WallType, refMat);
            ACAPI_ELEMENT_MASK_SET (mask, API_WallType, oppMat);
            ACAPI_ELEMENT_MASK_SET (mask, API_WallType, sidMat);
            ACAPI_ELEMENT_MASK_SET (mask, API_WallType, zoneRel);
            ACAPI_ELEMENT_MASK_SET (mask, API_WallType, flipped);
            if (needupdate)
                err = ACAPI_Element_Change (&wallelement, &mask, nullptr, 0, true);
        }

        draw.draw_error = err;
        if (err != NoError) {
            msg_rep ("Wall_Draw err", "ACAPI_Element_Create", err, APINULLGuid);
            return;
        }
        draw.otd_guid = wallelement.header.guid;
        for (OtdOpening &op : openings) {
            op.otd_guid = APINULLGuid;
            OtdOpening_Draw (wallelement, op, OtdLinearSegment_GetZBottom (geom));
            const bool opening_failed = op.otd_guid == APINULLGuid;
            if (!opening_failed)
                continue;

            draw.draw_error = APIERR_BADPARS;
            // Непрорезанная оболочка даёт лишнюю отделку; удаляем только новый результат.
            const GS::Array<API_Guid> incomplete = {wallelement.header.guid};
            const GSErrCode deleteErr = ACAPI_Element_Delete (incomplete);
            msg_rep ("Surface finish", "Opening failed; removing incomplete wall", deleteErr, draw.base_guid);
            if (deleteErr == NoError) {
                draw.otd_guid = APINULLGuid;
                for (auto &opening : openings)
                    opening.otd_guid = APINULLGuid;
            }
            return;
        }
        draw.material.structtype = wallelement.wall.modelElemStructureType;

        switch (draw.material.structtype) {
        case API_BasicStructure:
            draw.material.bmaterial = AttributeIndexGetShort (wallelement.wall.buildingMaterial);
            break;
        case API_CompositeStructure:
            draw.material.bmaterial = AttributeIndexGetShort (wallelement.wall.composite);
            break;
        case API_ProfileStructure:
            draw.material.bmaterial = AttributeIndexGetShort (wallelement.wall.profileAttr);
            break;
        }

        draw.write_succeeded = true;
        draw.was_created = (existingGuid == APINULLGuid);
    }

    // Контур Roof привязан к верху: подрезку сначала переносим на верхнюю плоскость,
    // затем ограничиваем общим конечным покрытием, чтобы не выйти за края и отверстия.
    static GSErrCode OtdSlab_SetRoofEdgeTrims (const OtdSlab &floor,
                                               const GS::Array<OtdSlab> &neighbors,
                                               double thickness,
                                               API_Polygon &polygon,
                                               API_ElementMemo &memo) {
        struct Joint {
            HalfPlane2DData cut;
            API_Coord3D bisector;
        };

        GS::Array<Joint> joints;
        GS::Array<HalfPlane2DData> cuts;
        const double length = std::sqrt (1 + floor.slope_x * floor.slope_x + floor.slope_y * floor.slope_y);
        const API_Coord3D normal = {-floor.slope_x / length, -floor.slope_y / length, 1 / length};
        const auto height = [] (const OtdSlab &source, const API_Coord &point) {
            return source.zBottom + source.slope_x * (point.x - source.plane_origin.x) +
                   source.slope_y * (point.y - source.plane_origin.y);
        };
        Geometry::MultiPolygon2D coverage;
        coverage.Push (floor.poly);
        for (const auto &neighbor : neighbors) {
            if (neighbor.draw.type != Floor || neighbor.draw.base_type != API_MeshID)
                continue;
            API_Polygon otherPolygon = {};
            API_ElementMemo otherMemo = {};
            const GS::OnExit otherGuard ([&otherMemo] () { ACAPI_DisposeElemMemoHdls (&otherMemo); });
            const GSErrCode error = ConvertPolygon2DToAPIPolygon (neighbor.poly, otherPolygon, otherMemo);
            if (error != NoError)
                return error;
            API_Coord common = {};
            bool touches = false;
            for (Int32 i = 1; i <= polygon.nCoords && !touches; ++i)
                for (Int32 j = 1; j <= otherPolygon.nCoords; ++j) {
                    const auto &a = (*memo.coords)[i];
                    const auto &b = (*otherMemo.coords)[j];
                    if (std::hypot (a.x - b.x, a.y - b.y) <= 1e-6 &&
                        std::fabs (height (floor, a) - height (neighbor, a)) <= 1e-6) {
                        common = a;
                        touches = true;
                        break;
                    }
                }
            if (!touches)
                continue;
            coverage.Push (neighbor.poly);
            const double otherLength =
                std::sqrt (1 + neighbor.slope_x * neighbor.slope_x + neighbor.slope_y * neighbor.slope_y);
            const API_Coord3D otherNormal = {
                -neighbor.slope_x / otherLength, -neighbor.slope_y / otherLength, 1 / otherLength};
            const API_Coord3D difference = {
                normal.x - otherNormal.x, normal.y - otherNormal.y, normal.z - otherNormal.z};
            const double differenceLength = std::hypot (std::hypot (difference.x, difference.y), difference.z);
            if (differenceLength <= 1e-6)
                continue;
            const double z = height (floor, common);
            const auto limits =
                [&] (const OtdSlab &source, const API_Polygon &shape, const API_ElementMemo &sourceMemo) {
                    double minimum = std::numeric_limits<double>::infinity ();
                    double maximum = -minimum;
                    for (Int32 i = 1; i <= shape.nCoords; ++i) {
                        const auto &point = (*sourceMemo.coords)[i];
                        const double value = difference.x * (point.x - common.x) + difference.y * (point.y - common.y) +
                                             difference.z * (height (source, point) - z);
                        minimum = std::min (minimum, value);
                        maximum = std::max (maximum, value);
                    }
                    return std::make_pair (minimum, maximum);
                };
            const auto ownLimits = limits (floor, polygon, memo);
            const auto otherLimits = limits (neighbor, otherPolygon, otherMemo);
            const double tolerance = 1e-6 * differenceLength;
            const bool negative = ownLimits.second <= tolerance && otherLimits.first >= -tolerance &&
                                  ownLimits.first < -tolerance && otherLimits.second > tolerance;
            const bool positive = ownLimits.first >= -tolerance && otherLimits.second <= tolerance &&
                                  ownLimits.second > tolerance && otherLimits.first < -tolerance;
            if (!negative && !positive)
                return APIERR_BADPARS;
            double a = difference.x + difference.z * floor.slope_x;
            double b = difference.y + difference.z * floor.slope_y;
            double offset = difference.z * thickness * length;
            const double cutLength = std::hypot (a, b);
            if (cutLength <= 1e-12)
                return APIERR_BADPARS;
            const double factor = (negative ? 1 : -1) / cutLength;
            a *= factor;
            b *= factor;
            offset *= factor;
            const HalfPlane2DData cut (a, b, a * common.x + b * common.y - offset);
            const API_Coord3D bisector = {normal.x + otherNormal.x, normal.y + otherNormal.y, normal.z + otherNormal.z};
            cuts.Push (cut);
            joints.Push (Joint{cut, bisector});
        }
        if (!joints.IsEmpty ()) {
            // В многогранном узле новая верхняя грань стыка может возникнуть между участками,
            // которые внизу имеют лишь общую вершину. Поэтому строим ячейку всеми плоскостями,
            // а не сдвигаем рёбра прежнего графа; конечное покрытие сохраняет края и отверстия.
            coverage.Unify ();
            Geometry::MultiPolygon2D tops;
            for (const auto &part : coverage) {
                GS::Int32 warning = 0;
                bool degenerate = false;
                const auto clipped = part.Cut (cuts, nullptr, &warning, &degenerate);
                if (warning != 0 || degenerate)
                    return APIERR_BADPARS;
                for (const auto &piece : clipped)
                    tops.Push (piece);
            }
            if (tops.GetSize () != 1)
                return APIERR_BADPARS;
            ACAPI_DisposeElemMemoHdls (&memo);
            memo = {};
            const GSErrCode error = ConvertPolygon2DToAPIPolygon (tops[0], polygon, memo);
            if (error != NoError)
                return error;
        }
        memo.edgeTrims = reinterpret_cast<API_EdgeTrim **> (
            BMAllocateHandle ((polygon.nCoords + 1) * sizeof (API_EdgeTrim), ALLOCATE_CLEAR, 0));
        if (memo.edgeTrims == nullptr)
            return APIERR_MEMFULL;
        for (Int32 i = 0; i <= polygon.nCoords; ++i) {
            (*memo.edgeTrims)[i].sideType = APIEdgeTrim_Vertical;
            (*memo.edgeTrims)[i].sideAngle = PI / 2;
        }
        for (Int32 contour = 1; contour <= polygon.nSubPolys; ++contour) {
            for (Int32 i = (*memo.pends)[contour - 1] + 1; i < (*memo.pends)[contour]; ++i) {
                const auto &a = (*memo.coords)[i];
                const auto &b = (*memo.coords)[i + 1];
                for (const auto &joint : joints) {
                    const auto onLine = [&] (const API_Coord &point) {
                        return std::fabs (joint.cut.ca * point.x + joint.cut.cb * point.y - joint.cut.cc) <= 1e-6;
                    };
                    if (!onLine (a) || !onLine (b))
                        continue;
                    const double dx = b.x - a.x, dy = b.y - a.y;
                    const double dz = floor.slope_x * dx + floor.slope_y * dy;
                    const double edgeLength = std::hypot (std::hypot (dx, dy), dz);
                    if (edgeLength <= 1e-6)
                        return APIERR_BADPARS;
                    const API_Coord3D edge = {dx / edgeLength, dy / edgeLength, dz / edgeLength};
                    const API_Coord3D inward = {normal.y * edge.z - normal.z * edge.y,
                                                normal.z * edge.x - normal.x * edge.z,
                                                normal.x * edge.y - normal.y * edge.x};
                    const auto &bisector = joint.bisector;
                    const double angle =
                        PI - std::atan2 (bisector.x * normal.x + bisector.y * normal.y + bisector.z * normal.z,
                                         bisector.x * inward.x + bisector.y * inward.y + bisector.z * inward.z);
                    if (!std::isfinite (angle) || angle <= 0 || angle >= PI)
                        return APIERR_BADPARS;
                    auto &trim = (*memo.edgeTrims)[i];
                    if (trim.sideType == APIEdgeTrim_CustomAngle && std::fabs (trim.sideAngle - angle) > 1e-6)
                        return APIERR_BADPARS;
                    trim.sideType = APIEdgeTrim_CustomAngle;
                    trim.sideAngle = angle;
                }
            }
            (*memo.edgeTrims)[(*memo.pends)[contour]] = (*memo.edgeTrims)[(*memo.pends)[contour - 1] + 1];
        }
        return NoError;
    }

    // Готовит настройки, контур и подрезки Roof без создания элемента.
    // Вызывающий код передаёт нулевые element/memo и освобождает memo при любом исходе.
    static GSErrCode Roof_Prepare (const Stories &storyLevels,
                                   const OtdSlab &floor,
                                   const API_Element &slabSettings,
                                   const GS::Array<OtdSlab> *neighbors,
                                   API_Element &element,
                                   API_ElementMemo &memo) {
        if (GetElemTypeID (slabSettings) != API_SlabID || floor.draw.type != Floor || !std::isfinite (floor.zBottom) ||
            !std::isfinite (floor.plane_origin.x) || !std::isfinite (floor.plane_origin.y) ||
            !std::isfinite (floor.slope_x) || !std::isfinite (floor.slope_y) || floor.poly.CalcArea () < 0.0001)
            return APIERR_BADPARS;
        const API_SlabType &settings = slabSettings.slab;
        double thickness = settings.thickness;
        if (settings.modelElemStructureType == API_CompositeStructure) {
            API_Attribute composite = {};
            composite.header.typeID = API_CompWallID;
            composite.header.index = settings.composite;
            const GSErrCode error = ACAPI_Attribute_Get (&composite);
            if (error != NoError)
                return error;
            if ((composite.header.flags & APICWall_ForRoof) == 0)
                return APIERR_BADPARS;
            thickness = composite.compWall.totalThick;
        } else if (settings.modelElemStructureType != API_BasicStructure) {
            return APIERR_BADPARS;
        }
        if (!std::isfinite (thickness) || thickness <= 0)
            return APIERR_BADPARS;

        SetElemTypeID (element, API_RoofID);
        element.roof.roofClass = API_PlaneRoofID;
        GSErrCode error = ACAPI_Element_GetDefaults (&element, nullptr);
        if (error != NoError)
            return error;
        element.header.layer = slabSettings.header.layer;
        element.header.renovationStatus = slabSettings.header.renovationStatus;
        element.header.floorInd = floor.floorInd;
        element.roof.roofClass = API_PlaneRoofID;
        auto &base = element.roof.shellBase;
        base.modelElemStructureType = settings.modelElemStructureType;
        base.buildingMaterial = settings.buildingMaterial;
        base.composite = settings.composite;
        base.thickness = thickness;
        base.pen = settings.pen;
        base.ltypeInd = settings.ltypeInd;
    #ifndef ServerMainVers_2700
        base.penOverride = settings.penOverride;
    #endif
        base.sectContPen = settings.sectContPen;
        base.sectContLtype = settings.sectContLtype;
        base.topMat = settings.topMat;
        base.botMat = settings.botMat;
        base.sidMat = settings.sideMat;
        base.materialsChained = settings.materialsChained;
        base.useFloorFill = settings.useFloorFill;
        base.use3DHatching = settings.use3DHatching;
        base.floorFillPen = settings.floorFillPen;
        base.floorFillBGPen = settings.floorFillBGPen;
        base.floorFillInd = settings.floorFillInd;
        base.hatchOrientation = settings.hatchOrientation;
        base.edgeTrim.sideType = APIEdgeTrim_Vertical;
        base.edgeTrim.sideAngle = PI / 2;
        base.level = GetOffsetFromStory (floor.zBottom, floor.floorInd, storyLevels);
        if (!std::isfinite (base.level))
            return APIERR_BADPARS;
        auto &plane = element.roof.u.planeRoof;
        const double slope = std::hypot (floor.slope_x, floor.slope_y);
        plane.angle = std::atan (slope);
        plane.posSign = true;
        plane.baseLine.c1 = floor.plane_origin;
        plane.baseLine.c2 = {floor.plane_origin.x + (slope > 0 ? floor.slope_y / slope : 1),
                             floor.plane_origin.y - (slope > 0 ? floor.slope_x / slope : 0)};
        error = ConvertPolygon2DToAPIPolygon (floor.poly, plane.poly, memo);
        if (error == NoError && neighbors != nullptr)
            error = OtdSlab_SetRoofEdgeTrims (floor, *neighbors, thickness, plane.poly, memo);
        return error;
    }

    // ID и padding не описывают геометрию; сравниваем только записываемые координаты,
    // границы контуров и дуги, до освобождения прежних handles конвертером.
    static bool Polygon_IsEqual (const API_Polygon &a,
                                 const API_ElementMemo &am,
                                 const API_Polygon &b,
                                 const API_ElementMemo &bm) {
        if (a.nCoords != b.nCoords || a.nSubPolys != b.nSubPolys || a.nArcs != b.nArcs || am.coords == nullptr ||
            bm.coords == nullptr || am.pends == nullptr || bm.pends == nullptr)
            return false;
        for (Int32 i = 1; i <= a.nCoords; ++i) {
            if ((*am.coords)[i].x != (*bm.coords)[i].x || (*am.coords)[i].y != (*bm.coords)[i].y)
                return false;
        }
        for (Int32 i = 0; i <= a.nSubPolys; ++i) {
            if ((*am.pends)[i] != (*bm.pends)[i])
                return false;
        }
        if (a.nArcs > 0 && (am.parcs == nullptr || bm.parcs == nullptr))
            return false;
        for (Int32 i = 0; i < a.nArcs; ++i) {
            const auto &aa = (*am.parcs)[i];
            const auto &ba = (*bm.parcs)[i];
            if (aa.begIndex != ba.begIndex || aa.endIndex != ba.endIndex || aa.arcAngle != ba.arcAngle)
                return false;
        }
        return true;
    }

    // Полностью заменяемый контур получает новые ID вершин; прежние настройки рёбер не привязываем к новым индексам.
    static GSErrCode Roof_WritePrepared (API_Element &element, API_ElementMemo &memo, const API_Guid &existingGuid) {
        if (existingGuid == APINULLGuid)
            return ACAPI_Element_Create (&element, &memo);
        API_Element existing = {};
        existing.header.guid = existingGuid;
        GSErrCode err = ACAPI_Element_Get (&existing);
        if (err != NoError)
            return err;
        if (GetElemTypeID (existing) != API_RoofID || existing.roof.roofClass != API_PlaneRoofID)
            return APIERR_BADPARS;
        API_ElementMemo previousMemo = {};
        const GS::OnExit memoGuard ([&] () { ACAPI_DisposeElemMemoHdls (&previousMemo); });
        err = ACAPI_Element_GetMemo (existingGuid,
                                     &previousMemo,
                                     APIMemoMask_Polygon | APIMemoMask_EdgeTrims | APIMemoMask_SideMaterials |
                                         APIMemoMask_RoofEdgeTypes);
        if (err != NoError)
            return err;
        bool needupdate =
            !Polygon_IsEqual (existing.roof.u.planeRoof.poly, previousMemo, element.roof.u.planeRoof.poly, memo);
        if (!needupdate) {
            // Замыкающая вершина контура не задаёт ребро; SDK обнуляет её edge payload.
            for (Int32 contour = 1; contour <= element.roof.u.planeRoof.poly.nSubPolys; ++contour) {
                for (Int32 i = (*memo.pends)[contour - 1] + 1; i < (*memo.pends)[contour]; ++i) {
                    const API_EdgeTrim previousTrim = previousMemo.edgeTrims == nullptr
                                                          ? existing.roof.shellBase.edgeTrim
                                                          : (*previousMemo.edgeTrims)[i];
                    const API_EdgeTrim targetTrim =
                        memo.edgeTrims == nullptr ? element.roof.shellBase.edgeTrim : (*memo.edgeTrims)[i];
                    if (previousTrim.sideType != targetTrim.sideType ||
                        (targetTrim.sideType == APIEdgeTrim_CustomAngle &&
                         previousTrim.sideAngle != targetTrim.sideAngle))
                        needupdate = true;
                    if (previousMemo.sideMaterials != nullptr) {
    #ifdef ServerMainVers_2700
                        if (previousMemo.sideMaterials[i].hasValue)
    #else
                        if (previousMemo.sideMaterials[i].overridden)
    #endif
                            needupdate = true;
                    }
                    if (previousMemo.roofEdgeTypes != nullptr &&
                        previousMemo.roofEdgeTypes[i] != APIRoofEdgeType_Undefined)
                        needupdate = true;
                }
            }
        }
        API_Element mask = {};
        needupdate = Elems_Check_Update (existing.header,
                                         mask,
                                         element.header.floorInd,
                                         element.header.layer,
                                         element.header.renovationStatus) ||
                     needupdate;
        auto &current = existing.roof.shellBase;
        const auto &target = element.roof.shellBase;
        if (current.modelElemStructureType != target.modelElemStructureType) {
            current.modelElemStructureType = target.modelElemStructureType;
            needupdate = true;
        }
        if (target.modelElemStructureType == API_BasicStructure &&
            current.buildingMaterial != target.buildingMaterial) {
            current.buildingMaterial = target.buildingMaterial;
            needupdate = true;
        }
        if (target.modelElemStructureType == API_CompositeStructure && current.composite != target.composite) {
            current.composite = target.composite;
            needupdate = true;
        }
        if (current.thickness != target.thickness) {
            current.thickness = target.thickness;
            needupdate = true;
        }
        if (current.pen != target.pen) {
            current.pen = target.pen;
            needupdate = true;
        }
        if (current.ltypeInd != target.ltypeInd) {
            current.ltypeInd = target.ltypeInd;
            needupdate = true;
        }
    #ifndef ServerMainVers_2700
        if (current.penOverride.cutFillPen != target.penOverride.cutFillPen ||
            current.penOverride.cutFillBackgroundPen != target.penOverride.cutFillBackgroundPen ||
            current.penOverride.overrideCutFillPen != target.penOverride.overrideCutFillPen ||
            current.penOverride.overrideCutFillBackgroundPen != target.penOverride.overrideCutFillBackgroundPen) {
            current.penOverride = target.penOverride;
            needupdate = true;
        }
    #endif
        if (current.sectContPen != target.sectContPen) {
            current.sectContPen = target.sectContPen;
            needupdate = true;
        }
        if (current.sectContLtype != target.sectContLtype) {
            current.sectContLtype = target.sectContLtype;
            needupdate = true;
        }
        for (const auto &pair : {std::make_pair (&current.topMat, &target.topMat),
                                 std::make_pair (&current.botMat, &target.botMat),
                                 std::make_pair (&current.sidMat, &target.sidMat)}) {
    #ifdef ServerMainVers_2700
            if (pair.first->hasValue != pair.second->hasValue || pair.first->value != pair.second->value) {
    #else
            if (pair.first->overridden != pair.second->overridden ||
                pair.first->attributeIndex != pair.second->attributeIndex) {
    #endif
                *pair.first = *pair.second;
                needupdate = true;
            }
        }

        if (current.materialsChained != target.materialsChained) {
            current.materialsChained = target.materialsChained;
            needupdate = true;
        }
        if (current.useFloorFill != target.useFloorFill) {
            current.useFloorFill = target.useFloorFill;
            needupdate = true;
        }
        if (current.use3DHatching != target.use3DHatching) {
            current.use3DHatching = target.use3DHatching;
            needupdate = true;
        }
        if (current.floorFillPen != target.floorFillPen) {
            current.floorFillPen = target.floorFillPen;
            needupdate = true;
        }
        if (current.floorFillBGPen != target.floorFillBGPen) {
            current.floorFillBGPen = target.floorFillBGPen;
            needupdate = true;
        }
        if (current.floorFillInd != target.floorFillInd) {
            current.floorFillInd = target.floorFillInd;
            needupdate = true;
        }
        const auto &a = current.hatchOrientation;
        const auto &b = target.hatchOrientation;
        if (a.type != b.type || a.version != b.version || a.flags != b.flags || a.origo.x != b.origo.x ||
            a.origo.y != b.origo.y || a.matrix00 != b.matrix00 || a.matrix10 != b.matrix10 ||
            a.matrix01 != b.matrix01 || a.matrix11 != b.matrix11 || a.innerRadius != b.innerRadius) {
            current.hatchOrientation = target.hatchOrientation;
            needupdate = true;
        }
        if (current.edgeTrim.sideType != target.edgeTrim.sideType ||
            (target.edgeTrim.sideType == APIEdgeTrim_CustomAngle &&
             current.edgeTrim.sideAngle != target.edgeTrim.sideAngle)) {
            current.edgeTrim = target.edgeTrim;
            needupdate = true;
        }
        if (current.level != target.level) {
            current.level = target.level;
            needupdate = true;
        }
        auto &plane = existing.roof.u.planeRoof;
        const auto &targetPlane = element.roof.u.planeRoof;
        if (plane.angle != targetPlane.angle) {
            plane.angle = targetPlane.angle;
            needupdate = true;
        }
        if (plane.posSign != targetPlane.posSign) {
            plane.posSign = targetPlane.posSign;
            needupdate = true;
        }
        if (plane.baseLine.c1.x != targetPlane.baseLine.c1.x || plane.baseLine.c1.y != targetPlane.baseLine.c1.y ||
            plane.baseLine.c2.x != targetPlane.baseLine.c2.x || plane.baseLine.c2.y != targetPlane.baseLine.c2.y) {
            plane.baseLine = targetPlane.baseLine;
            needupdate = true;
        }
        plane.poly = targetPlane.poly;
        element = existing;
        if (!needupdate)
            return NoError;
        if (memo.vertexIDs == nullptr)
            return APIERR_MEMFULL;
        (*memo.vertexIDs)[0] = previousMemo.vertexIDs == nullptr ? 0 : (*previousMemo.vertexIDs)[0];
        for (Int32 i = 1; i <= element.roof.u.planeRoof.poly.nCoords; ++i)
            (*memo.vertexIDs)[i] = 0;
        element.header.guid = existingGuid;
        ACAPI_ELEMENT_MASK_SET (mask, API_RoofType, shellBase.modelElemStructureType);
        if (element.roof.shellBase.modelElemStructureType == API_BasicStructure)
            ACAPI_ELEMENT_MASK_SET (mask, API_RoofType, shellBase.buildingMaterial);
        if (element.roof.shellBase.modelElemStructureType == API_CompositeStructure)
            ACAPI_ELEMENT_MASK_SET (mask, API_RoofType, shellBase.composite);
        ACAPI_ELEMENT_MASK_SET (mask, API_RoofType, shellBase.thickness);
        ACAPI_ELEMENT_MASK_SET (mask, API_RoofType, shellBase.pen);
        ACAPI_ELEMENT_MASK_SET (mask, API_RoofType, shellBase.ltypeInd);
    #ifndef ServerMainVers_2700
        ACAPI_ELEMENT_MASK_SET (mask, API_RoofType, shellBase.penOverride);
    #endif
        ACAPI_ELEMENT_MASK_SET (mask, API_RoofType, shellBase.sectContPen);
        ACAPI_ELEMENT_MASK_SET (mask, API_RoofType, shellBase.sectContLtype);
        ACAPI_ELEMENT_MASK_SET (mask, API_RoofType, shellBase.topMat);
        ACAPI_ELEMENT_MASK_SET (mask, API_RoofType, shellBase.botMat);
        ACAPI_ELEMENT_MASK_SET (mask, API_RoofType, shellBase.sidMat);
        ACAPI_ELEMENT_MASK_SET (mask, API_RoofType, shellBase.materialsChained);
        ACAPI_ELEMENT_MASK_SET (mask, API_RoofType, shellBase.useFloorFill);
        ACAPI_ELEMENT_MASK_SET (mask, API_RoofType, shellBase.use3DHatching);
        ACAPI_ELEMENT_MASK_SET (mask, API_RoofType, shellBase.floorFillPen);
        ACAPI_ELEMENT_MASK_SET (mask, API_RoofType, shellBase.floorFillBGPen);
        ACAPI_ELEMENT_MASK_SET (mask, API_RoofType, shellBase.floorFillInd);
        ACAPI_ELEMENT_MASK_SET (mask, API_RoofType, shellBase.hatchOrientation);
        ACAPI_ELEMENT_MASK_SET (mask, API_RoofType, shellBase.edgeTrim);
        ACAPI_ELEMENT_MASK_SET (mask, API_RoofType, shellBase.level);
        ACAPI_ELEMENT_MASK_SET (mask, API_RoofType, u.planeRoof.angle);
        ACAPI_ELEMENT_MASK_SET (mask, API_RoofType, u.planeRoof.posSign);
        ACAPI_ELEMENT_MASK_SET (mask, API_RoofType, u.planeRoof.baseLine);
        ACAPI_ELEMENT_MASK_SET (mask, API_RoofType, u.planeRoof.poly.nCoords);
        ACAPI_ELEMENT_MASK_SET (mask, API_RoofType, u.planeRoof.poly.nSubPolys);
        ACAPI_ELEMENT_MASK_SET (mask, API_RoofType, u.planeRoof.poly.nArcs);
        // Нулевые edge payload сбрасывают прежние индивидуальные настройки, как в SDK-примере GetMemoMask.
        return ACAPI_Element_Change (&element,
                                     &mask,
                                     &memo,
                                     APIMemoMask_Polygon | APIMemoMask_EdgeTrims | APIMemoMask_SideMaterials |
                                         APIMemoMask_RoofEdgeTypes,
                                     true);
    }

    GSErrCode Roof_Draw (const Stories &storyLevels,
                         OtdSlab &floor,
                         const API_Element &slabSettings,
                         const GS::Array<OtdSlab> *neighbors) {
        const API_Guid existingGuid = floor.draw.otd_guid;
        floor.draw.otd_guid = APINULLGuid;
        floor.draw.write_succeeded = false;
        floor.draw.was_created = false;
        API_Element element = {};
        API_ElementMemo memo = {};
        const GS::OnExit memoGuard ([&memo] () { ACAPI_DisposeElemMemoHdls (&memo); });
        GSErrCode error = Roof_Prepare (storyLevels, floor, slabSettings, neighbors, element, memo);
        if (error == NoError) {
            UnhideUnlockElementLayer (element.header);
            error = Roof_WritePrepared (element, memo, existingGuid);
        }
        floor.draw.draw_error = error;
        if (error != NoError)
            return error;
        floor.draw.otd_guid = element.header.guid;
        floor.draw.write_succeeded = true;
        floor.draw.was_created = existingGuid == APINULLGuid;
        floor.draw.material.structtype = element.roof.shellBase.modelElemStructureType;
        switch (floor.draw.material.structtype) {
        case API_BasicStructure:
            floor.draw.material.bmaterial = AttributeIndexGetShort (element.roof.shellBase.buildingMaterial);
            break;
        case API_CompositeStructure:
            floor.draw.material.bmaterial = AttributeIndexGetShort (element.roof.shellBase.composite);
            break;
        case API_ProfileStructure:
            floor.draw.material.bmaterial = 0;
            break;
        }
        return error;
    }

    // Только выбранный Slab: Object не подменяется крышей, отсутствующее избранное не заменяется defaults.
    static bool Roof_ReadSlabSettings (const OtdSlab &floor, API_Element &settings) {
        if (floor.draw.type != Floor || floor.draw.draw_type != API_RoofID || floor.draw.favorite.type != API_SlabID ||
            !Favorite_ReadByName (floor.draw.favorite.name, settings) || GetElemTypeID (settings) != API_SlabID)
            return false;
    #ifdef ServerMainVers_2700
        const auto material = ACAPI_CreateAttributeIndex (floor.draw.material.material);
        settings.slab.topMat.value = material;
        settings.slab.botMat.value = material;
        settings.slab.sideMat.value = material;
        settings.slab.topMat.hasValue = settings.slab.sideMat.hasValue = true;
    #else
        settings.slab.topMat.attributeIndex = floor.draw.material.material;
        settings.slab.botMat.attributeIndex = floor.draw.material.material;
        settings.slab.sideMat.attributeIndex = floor.draw.material.material;
        settings.slab.topMat.overridden = settings.slab.sideMat.overridden = true;
    #endif
        return true;
    }

    static GSErrCode RoofBatch_Validate (const Stories &stories, OtdRooms &rooms) {
        for (auto it = rooms.EnumeratePairs (); it != nullptr; ++it) {
    #ifdef ServerMainVers_2800
            OtdRoom &room = it->value;
    #else
            OtdRoom &room = *it->value;
    #endif
            for (auto &floor : room.otdslab) {
                if (floor.draw.base_type != API_MeshID || !OtdSlab_ShouldDrawFinish (room, floor))
                    continue;
                API_Element settings = {};
                if (!Roof_ReadSlabSettings (floor, settings)) {
                    msg_rep ("Mesh floor",
                             "Selected floor Favorite cannot be rendered as Roof",
                             APIERR_BADPARS,
                             floor.draw.base_guid);
                    return APIERR_BADPARS;
                }
                API_Element element = {};
                API_ElementMemo memo = {};
                const GS::OnExit memoGuard ([&memo] () { ACAPI_DisposeElemMemoHdls (&memo); });
                const GSErrCode err = Roof_Prepare (stories, floor, settings, nullptr, element, memo);
                if (err != NoError)
                    return err;
            }
        }
        return NoError;
    }

    // Создаёт или изменяет перекрытие по готовой геометрии; GUID публикует только при успешной записи.
    // При изменении сохраняет прежнюю политику материалов и перпендикулярной подрезки рёбер.
    static void Slab_Draw (const Stories &storyLevels,
                           const Geometry::Polygon2D &poly,
                           double zBottom,
                           short floorInd,
                           bool surface_geometry,
                           OtdDrawData &draw) {
        const API_Guid existingGuid = draw.otd_guid;
        draw.otd_guid = APINULLGuid;
        draw.draw_error = APIERR_BADPARS;
        draw.write_succeeded = false;
        draw.was_created = false;
        GSErrCode err = NoError;
        API_Element slabelement = {};
        API_Element settings = {};
        API_Element mask = {};
        API_ElementMemo memo = {};
        const GS::OnExit memoGuard ([&memo] () { ACAPI_DisposeElemMemoHdls (&memo); });
        if (!Slab_GetDefault (draw.favorite.name, draw.type, settings))
            return;
        bool is_new = (existingGuid == APINULLGuid); // Элемент уже существует, нужно только обновить
        if (is_new) {
            slabelement = settings;
        } else {
            if (!GetExsist (existingGuid, slabelement, memo))
                return;
            if (GetElemTypeID (slabelement) != API_SlabID)
                return;
        }
        bool needupdate = Elems_Check_Update (
            slabelement.header, mask, floorInd, settings.header.layer, settings.header.renovationStatus);
        double level = GetOffsetFromStory (zBottom, floorInd, storyLevels);
        if (surface_geometry) {
            if (slabelement.slab.referencePlaneLocation != APISlabRefPlane_Top) {
                slabelement.slab.referencePlaneLocation = APISlabRefPlane_Top;
                needupdate = true;
            }
            if (slabelement.slab.offsetFromTop != 0) {
                slabelement.slab.offsetFromTop = 0;
                needupdate = true;
            }
            if (draw.favorite.name.IsEmpty ()) {
                if (slabelement.slab.modelElemStructureType != API_BasicStructure) {
                    slabelement.slab.modelElemStructureType = API_BasicStructure;
                    needupdate = true;
                }
                if (slabelement.slab.thickness != otd_thickness) {
                    slabelement.slab.thickness = otd_thickness;
                    needupdate = true;
                }
            }
            // Потолок уходит вниз от контакта, пол — вверх; level задаёт верх перекрытия.
            if (draw.type == Floor)
                level += slabelement.slab.thickness;
        }
        if (slabelement.slab.level != level) {
            slabelement.slab.level = level;
            needupdate = true;
        }
    #ifdef ServerMainVers_2700
        API_AttributeIndex ematerial = ACAPI_CreateAttributeIndex (draw.material.material);
        for (auto *material : {&slabelement.slab.topMat, &slabelement.slab.sideMat, &slabelement.slab.botMat}) {
            if (material->value != ematerial) {
                material->value = ematerial;
                needupdate = true;
            }
        }
        if (!slabelement.slab.sideMat.hasValue) {
            slabelement.slab.sideMat.hasValue = true;
            needupdate = true;
        }
    #else
        for (auto *material : {&slabelement.slab.topMat, &slabelement.slab.sideMat, &slabelement.slab.botMat}) {
            if (material->attributeIndex != draw.material.material) {
                material->attributeIndex = draw.material.material;
                needupdate = true;
            }
        }
        if (!slabelement.slab.sideMat.overridden) {
            slabelement.slab.sideMat.overridden = true;
            needupdate = true;
        }
    #endif
        if (draw.type == Ceil) {
    #ifdef ServerMainVers_2700
            if (!slabelement.slab.botMat.hasValue) {
                slabelement.slab.botMat.hasValue = true;
                needupdate = true;
            }
    #else
            if (!slabelement.slab.botMat.overridden) {
                slabelement.slab.botMat.overridden = true;
                needupdate = true;
            }
    #endif
        } else {
    #ifdef ServerMainVers_2700
            if (!slabelement.slab.topMat.hasValue) {
                slabelement.slab.topMat.hasValue = true;
                needupdate = true;
            }
    #else
            if (!slabelement.slab.topMat.overridden) {
                slabelement.slab.topMat.overridden = true;
                needupdate = true;
            }
    #endif
        }
        const UInt32 previousMaxVertexID = memo.vertexIDs == nullptr ? 0 : (*memo.vertexIDs)[0];
        API_Polygon targetPolygon = {};
        API_ElementMemo targetMemo = {};
        const GS::OnExit targetGuard ([&targetMemo] () { ACAPI_DisposeElemMemoHdls (&targetMemo); });
        err = ConvertPolygon2DToAPIPolygon (poly, targetPolygon, targetMemo);
        if (err != NoError) {
            draw.draw_error = err;
            return;
        }
        if (!Polygon_IsEqual (slabelement.slab.poly, memo, targetPolygon, targetMemo)) {
            slabelement.slab.poly = targetPolygon;
            needupdate = true;
        }
        if (!is_new && !needupdate) {
            for (Int32 contour = 1; contour <= targetPolygon.nSubPolys; ++contour) {
                for (Int32 i = (*targetMemo.pends)[contour - 1] + 1; i < (*targetMemo.pends)[contour]; ++i) {
                    if (memo.edgeTrims == nullptr ||
                        BMGetHandleSize ((GSHandle)memo.edgeTrims) < (i + 1) * sizeof (API_EdgeTrim) ||
                        (*memo.edgeTrims)[i].sideType != APIEdgeTrim_Perpendicular)
                        needupdate = true;
                    if (memo.sideMaterials == nullptr ||
                        BMGetPtrSize ((GSPtr)memo.sideMaterials) < (i + 1) * sizeof (*memo.sideMaterials)) {
                        needupdate = true;
                    } else {
    #ifdef ServerMainVers_2700
                        if (memo.sideMaterials[i].hasValue != slabelement.slab.sideMat.hasValue ||
                            memo.sideMaterials[i].value != slabelement.slab.sideMat.value)
    #else
                        if (memo.sideMaterials[i].overridden != slabelement.slab.sideMat.overridden ||
                            memo.sideMaterials[i].attributeIndex != slabelement.slab.sideMat.attributeIndex)
    #endif
                            needupdate = true;
                    }
                }
            }
        }
        std::swap (memo.coords, targetMemo.coords);
        std::swap (memo.pends, targetMemo.pends);
        std::swap (memo.parcs, targetMemo.parcs);
        std::swap (memo.vertexIDs, targetMemo.vertexIDs);
        if (is_new) {
            err = ACAPI_Element_Create (&slabelement, &memo);
        } else if (needupdate) {
            if (surface_geometry) {
                if (memo.vertexIDs == nullptr) {
                    draw.draw_error = APIERR_MEMFULL;
                    return;
                }
                (*memo.vertexIDs)[0] = previousMaxVertexID;
                for (Int32 i = 1; i <= slabelement.slab.poly.nCoords; ++i)
                    (*memo.vertexIDs)[i] = 0;
                BMhKill ((GSHandle *)&memo.contourIDs);
            }
            // Setting side materials and edge angles
            BMhKill ((GSHandle *)&memo.edgeTrims);
            BMhKill ((GSHandle *)&memo.edgeIDs);
            BMpFree (reinterpret_cast<GSPtr> (memo.sideMaterials));
            API_EdgeTrimID edgeType = APIEdgeTrim_Perpendicular;
            memo.edgeTrims = (API_EdgeTrim **)BMAllocateHandle (
                (slabelement.slab.poly.nCoords + 1) * sizeof (API_EdgeTrim), ALLOCATE_CLEAR, 0);
            memo.sideMaterials = (API_OverriddenAttribute *)BMAllocatePtr (
                (slabelement.slab.poly.nCoords + 1) * sizeof (API_OverriddenAttribute), ALLOCATE_CLEAR, 0);
            if (memo.edgeTrims == nullptr || memo.sideMaterials == nullptr) {
                draw.draw_error = APIERR_MEMFULL;
                return;
            }
            for (Int32 k = 1; k <= slabelement.slab.poly.nCoords; ++k) {
                memo.sideMaterials[k] = slabelement.slab.sideMat;
                (*(memo.edgeTrims))[k].sideType = edgeType;
                (*(memo.edgeTrims))[k].sideAngle = PI / 2;
            }
            ACAPI_ELEMENT_MASK_SET (mask, API_SlabType, botMat);
            ACAPI_ELEMENT_MASK_SET (mask, API_SlabType, topMat);
            ACAPI_ELEMENT_MASK_SET (mask, API_SlabType, sideMat);
            ACAPI_ELEMENT_MASK_SET (mask, API_SlabType, poly.nSubPolys);
            ACAPI_ELEMENT_MASK_SET (mask, API_SlabType, poly.nCoords);
            ACAPI_ELEMENT_MASK_SET (mask, API_SlabType, poly.nArcs);
            ACAPI_ELEMENT_MASK_SET (mask, API_SlabType, level);
            if (surface_geometry) {
                ACAPI_ELEMENT_MASK_SET (mask, API_SlabType, referencePlaneLocation);
                ACAPI_ELEMENT_MASK_SET (mask, API_SlabType, offsetFromTop);
                ACAPI_ELEMENT_MASK_SET (mask, API_SlabType, thickness);
                ACAPI_ELEMENT_MASK_SET (mask, API_SlabType, modelElemStructureType);
            }
            err = ACAPI_Element_Change (&slabelement,
                                        &mask,
                                        &memo,
                                        APIMemoMask_Polygon | APIMemoMask_SideMaterials | APIMemoMask_EdgeTrims,
                                        true);
        }
        draw.draw_error = err;
        if (err != NoError) {
            msg_rep ("Slab_Draw", draw.favorite.name, err, APINULLGuid);
            return;
        } else {
            draw.otd_guid = slabelement.header.guid;
            draw.write_succeeded = true;
            draw.was_created = is_new;
        }
        draw.material.structtype = slabelement.slab.modelElemStructureType;
        switch (draw.material.structtype) {
        case API_BasicStructure:
            draw.material.bmaterial = AttributeIndexGetShort (slabelement.slab.buildingMaterial);
            break;
        case API_CompositeStructure:
            draw.material.bmaterial = AttributeIndexGetShort (slabelement.slab.composite);
            break;
        case API_ProfileStructure:
            draw.material.bmaterial = 0;
            break;
        }
        return;
    }

    // Создаёт или обновляет объект пола/потолка, передавая контуры и параметры отделки в GDL.
    // При отказе чтения существующего элемента, memo или записи обнуляет otd_guid; при успехе сохраняет GUID.
    static void Object_Draw_Poly (const Stories &storyLevels,
                                  const Geometry::Polygon2D &poly,
                                  double zBottom,
                                  short floorInd,
                                  const GS::UniString &tip,
                                  OtdDrawData &draw) {
        const API_Guid existingGuid = draw.otd_guid;
        draw.otd_guid = APINULLGuid;
        draw.write_succeeded = false;
        draw.was_created = false;
        draw.draw_error = APIERR_BADPARS;
        GSErrCode err = NoError;
        API_Element slabobjelement = {};
        API_ElementMemo slabobjmemo = {};
        API_Element settings = {};
        API_ElementMemo settingsMemo = {};
        API_Element mask = {};
        const GS::OnExit memoGuard ([&slabobjmemo] () { ACAPI_DisposeElemMemoHdls (&slabobjmemo); });
        const GS::OnExit settingsGuard ([&settingsMemo] () { ACAPI_DisposeElemMemoHdls (&settingsMemo); });
        if (!Object_GetDefault (draw.favorite.name, draw.type, settings, settingsMemo))
            return;
        bool is_new = (existingGuid == APINULLGuid); // Элемент уже существует, нужно только обновить
        if (is_new) {
            slabobjelement = settings;
            std::swap (slabobjmemo, settingsMemo);
        } else {
            if (!GetExsist (existingGuid, slabobjelement, slabobjmemo))
                return;
            if (GetElemTypeID (slabobjelement) != API_ObjectID)
                return;
        }
        GDLHelpers::ParamDict accsessoryparams;
        GDLHelpers::Param p;
        if (draw.type == Ceil) {
            // Высота уже задана положением объекта; GDL дополнительно прибавляет ac_ref_height.
            p.num = 0;
            accsessoryparams.Add ("{@gdl:ac_ref_height}", p);
        }
        if (draw.type == Floor) {
            p.num = zBottom;
            accsessoryparams.Add ("{@gdl:ac_ref_height}", p);
        }
        p.str = tip;
        accsessoryparams.Add ("{@gdl:tip_pol}", p);
        p.str = EMPTYSTRING;
        p.num = 1;
        accsessoryparams.Add ("{@gdl:mun_zone}", p);
        p.num = 0;
        accsessoryparams.Add ("{@gdl:ac_pitch}", p);
        p.num = 0;
        accsessoryparams.Add ("{@gdl:ac_ceilling_side}", p);
        p.num = 0;
        accsessoryparams.Add ("{@gdl:ac_ceiling_type}", p);
        p.num = draw.material.material;
        accsessoryparams.Add ("{@gdl:ceil_mat}", p);
        p.num = draw.material.material;
        accsessoryparams.Add ("{@gdl:matedge}", p);
        p.num = draw.material.material;
        accsessoryparams.Add ("{@gdl:matceipanel}", p);
        p.num = 0.02;
        accsessoryparams.Add ("{@gdl:ceil_thk}", p);
        p.num = 0.02;
        accsessoryparams.Add ("{@gdl:ac_thickness}", p);
        p.num = 0;
        accsessoryparams.Add ("{@gdl:ac_slab_side}", p);
        Geometry::Polygon2DData polygon2DData;
        Geometry::InitPolygon2DData (&polygon2DData);
        Geometry::ConvertPolygon2DToPolygon2DData (polygon2DData, poly);
        Int32 nCoords = polygon2DData.nVertices;
        Int32 nSubPolys = polygon2DData.nContours;
        Box2DData bbox = poly.GetBoundBox ();
        GDLHelpers::Param ac_coords;
        GDLHelpers::Param ac_whole_poly;
        for (Int32 j = 1; j <= nSubPolys; j++) {
            UInt32 begInd = (*polygon2DData.contourEnds)[j - 1] + 1;
            UInt32 endInd = (*polygon2DData.contourEnds)[j];
            double x_f = (*polygon2DData.vertices)[begInd].x - bbox.xMin;
            double y_f = (*polygon2DData.vertices)[begInd].y - bbox.yMin;
            for (UInt32 k = begInd; k < endInd; k++) {
                double x = (*polygon2DData.vertices)[k].x - bbox.xMin;
                double y = (*polygon2DData.vertices)[k].y - bbox.yMin;
                ac_coords.arr_num.Push (0.025);
                ac_coords.arr_num.Push (x);
                ac_coords.arr_num.Push (y);
                if (j == 1) {
                    ac_whole_poly.arr_num.Push (x);
                    ac_whole_poly.arr_num.Push (y);
                }
            }
            ac_coords.arr_num.Push (0.051);
            ac_coords.arr_num.Push (x_f);
            ac_coords.arr_num.Push (y_f);
        }
        ac_coords.dim1 = ac_coords.arr_num.GetSize () / 3;
        ac_coords.dim2 = 3;
        ac_whole_poly.dim1 = 1;
        ac_whole_poly.dim2 = ac_whole_poly.arr_num.GetSize ();
        accsessoryparams.Add ("{@gdl:ac_whole_poly}", ac_whole_poly);
        accsessoryparams.Add ("{@gdl:ac_coords}", ac_coords);
        if (is_new) {
            if (draw.type == Ceil) {
                p.num = bbox.xMax - bbox.xMin - 0.1;
                accsessoryparams.Add ("{@gdl:pos_x2}", p);
                p.num = bbox.yMax - bbox.yMin - 0.1;
                accsessoryparams.Add ("{@gdl:pos_y2}", p);

                p.num = (bbox.xMax - bbox.xMin - 0.1) / 2;
                accsessoryparams.Add ("{@gdl:pos_xtxt}", p);
                p.num = (bbox.yMax - bbox.yMin - 0.1) / 2;
                accsessoryparams.Add ("{@gdl:pos_ytxt}", p);
            }
            if (draw.type == Floor) {
                p.num = poly.GetCenter ().x - bbox.xMin;
                accsessoryparams.Add ("{@gdl:pos_x1}", p);
                p.num = poly.GetCenter ().y - bbox.yMin;
                accsessoryparams.Add ("{@gdl:pos_y1}", p);
            }
            p.dim1 = 1;
            p.dim2 = 2;
            p.arr_num.Push (poly.GetCenter ().x - bbox.xMin);
            p.arr_num.Push (poly.GetCenter ().y - bbox.yMin);
            accsessoryparams.Add ("{@gdl:fieldorig}", p);
        }
        Geometry::FreePolygon2DData (&polygon2DData);
        bool needupdate = Object_UpdateParams (slabobjmemo, accsessoryparams);
        if (draw.type == Ceil || draw.type == Floor) {
            const double level = GetOffsetFromStory (zBottom, floorInd, storyLevels);
            if (slabobjelement.object.level != level) {
                slabobjelement.object.level = level;
                needupdate = true;
            }
        }
        needupdate =
            Elems_Check_Update (
                slabobjelement.header, mask, floorInd, settings.header.layer, settings.header.renovationStatus) ||
            needupdate;
        if (slabobjelement.object.pos.x != bbox.xMin || slabobjelement.object.pos.y != bbox.yMin) {
            slabobjelement.object.pos = {bbox.xMin, bbox.yMin};
            needupdate = true;
        }
        double y_start = 0;
        if (is_new) {
            err = ACAPI_Element_Create (&slabobjelement, &slabobjmemo);
        } else if (needupdate) {
            ACAPI_ELEMENT_MASK_SET (mask, API_ObjectType, pos);
            ACAPI_ELEMENT_MASK_SET (mask, API_ObjectType, level);
            err = ACAPI_Element_Change (&slabobjelement, &mask, &slabobjmemo, APIMemoMask_AddPars, true);
        }
        draw.draw_error = err;
        if (err != NoError) {
            draw.otd_guid = APINULLGuid;
            msg_rep ("Object_Draw_Poly", draw.favorite.name, err, APINULLGuid);
            return;
        } else {
            draw.otd_guid = slabobjelement.header.guid;
            draw.write_succeeded = true;
            draw.was_created = is_new;
        }
    }

    // -----------------------------------------------------------------------------
    // Построение потолка/пола (общая)
    // Выбирает - каким элементом строить - slab/mesh/object
    // -----------------------------------------------------------------------------
    static void OtdSlab_Draw (const Stories &storyLevels, OtdSlab &otdslab, UnicElementByType &subelementByparent) {
        otdslab.draw.write_succeeded = false;
        otdslab.draw.was_created = false;
        const double area = otdslab.poly.CalcArea ();
        if (!std::isfinite (area) || area < 0.0001 || !std::isfinite (otdslab.zBottom)) {
            otdslab.draw.otd_guid = APINULLGuid;
            otdslab.draw.draw_error = APIERR_BADPARS;
            return;
        }
        if (otdslab.draw.base_type == API_MeshID) {
            const API_Guid existingGuid = otdslab.draw.otd_guid;
            otdslab.draw.otd_guid = APINULLGuid;
            otdslab.draw.draw_error = APIERR_BADPARS;
            API_Element settings = {};
            if (!Roof_ReadSlabSettings (otdslab, settings))
                return;
            otdslab.draw.otd_guid = existingGuid;
            otdslab.draw.draw_error = Roof_Draw (storyLevels, otdslab, settings, nullptr);
        } else
            switch (otdslab.draw.favorite.type) {
            case API_SlabID:
                Slab_Draw (storyLevels,
                           otdslab.poly,
                           otdslab.zBottom,
                           otdslab.floorInd,
                           otdslab.surface_geometry,
                           otdslab.draw);
                break;
            case API_ObjectID:
                Object_Draw_Poly (
                    storyLevels, otdslab.poly, otdslab.zBottom, otdslab.floorInd, otdslab.tip, otdslab.draw);
                break;
            default:
                otdslab.draw.otd_guid = APINULLGuid;
                otdslab.draw.draw_error = APIERR_BADPARS;
                break;
            }
        if (otdslab.draw.draw_error != NoError)
            return;
        Param_AddUnicElementByType (otdslab.draw.base_guid,
                                    otdslab.draw.otd_guid,
                                    otdslab.draw.base_type == API_MeshID ? API_MeshID : API_SlabID,
                                    subelementByparent);
    }

    // Выбирает способ построения плинтуса и собирает связь с его основанием.
    static void OtdBeam_Draw (const Stories &storyLevels, OtdBeam &beam, UnicElementByType &subelementByparent) {
        beam.draw.write_succeeded = false;
        beam.draw.was_created = false;
        beam.draw.draw_error = NoError;
        GS::Array<OtdOpening> openings;
        switch (beam.draw.favorite.type) {
        case API_BeamID:
            Beam_Draw (storyLevels, beam.geom, beam.draw);
            break;
        case API_WallID:
            Wall_Draw (storyLevels, beam.geom, beam.draw, 0, openings);
            break;
        case API_ObjectID:
            Object_Draw_Linear (storyLevels, beam.geom, beam.draw, openings);
            break;
        default:
            beam.draw.draw_error = APIERR_BADPARS;
            break;
        }
        if (beam.draw.draw_error != NoError)
            return;
        Param_AddUnicElementByType (beam.draw.base_guid, beam.draw.otd_guid, beam.draw.base_type, subelementByparent);
    }

    // -----------------------------------------------------------------------------
    // Построение отделочных стен (общая)
    // Решает - чем строить - стеной/балкой/объектом
    // -----------------------------------------------------------------------------
    void OtdWall_Draw (const Stories &storyLevels, OtdWall &edges, UnicElementByType &subelementByparent) {
        edges.draw.draw_error = NoError;
        if (!OtdLinearElement_CanDraw (edges.geom, edges.draw.favorite.type != API_BeamID)) {
            edges.draw.otd_guid = APINULLGuid;
            edges.draw.draw_error = APIERR_BADPARS;
            return;
        }
        // Выбор элемента для отрисовки
        switch (edges.draw.favorite.type) {
        case API_WallID:
            Wall_Draw (storyLevels, edges.geom, edges.draw, edges.slant_angle, edges.openings);
            break;
        case API_BeamID:
            Beam_Draw (storyLevels, edges.geom, edges.draw);
            break;
        case API_ObjectID:
            Object_Draw_Linear (storyLevels, edges.geom, edges.draw, edges.openings);
            break;
        default:
            edges.draw.otd_guid = APINULLGuid;
            edges.draw.draw_error = APIERR_BADPARS;
            break;
        }
        // Привязка элемента к отделке
        Param_AddUnicElementByType (edges.draw.base_guid,
                                    edges.draw.otd_guid,
                                    API_WallID,
                                    subelementByparent); // Привязка отделочной стены к базовой
        for (OtdOpening &op : edges.openings) {
            Param_AddUnicElementByType (op.base_guid,
                                        op.otd_guid,
                                        API_WindowID,
                                        subelementByparent); // Привязка отделочного проёма к базовому
        }
    }

    static GSErrCode OtdRoom_Draw (const Stories &storyLevels,
                                   OtdRoom &otd,
                                   UnicElementByType &subelementByparent,
                                   ClassificationFunc::ClassificationDict &finclass) {
        GSErrCode err = NoError;
        GS::Array<API_Guid> group;
        if (!otd.isValid)
            return NoError;
        API_Guid class_guid = APINULLGuid;
        for (OtdWall &otdwall : otd.otdwall) {
            otdwall.draw.write_succeeded = false;
            if (!OtdWall_ShouldDrawFinish (otd, otdwall))
                continue;
            OtdWall_Draw (storyLevels, otdwall, subelementByparent);
            if (otdwall.draw.draw_error != NoError) {
                msg_rep ("OtdRoom_Draw", "OtdWall_Draw err", otdwall.draw.draw_error, otdwall.draw.otd_guid);
                continue;
            }
            Param_AddUnicElementByType (otd.zone_guid,
                                        otdwall.draw.otd_guid,
                                        API_ZoneID,
                                        subelementByparent); // Привязка отделочных стен к
                                                             // зоне
            for (OtdOpening &op : otdwall.openings) {
                Param_AddUnicElementByType (otd.zone_guid,
                                            op.otd_guid,
                                            API_ZoneID,
                                            subelementByparent); // Привязка отделочных
                                                                 // проёмов к зоне
            }
            // Классификация созданных стен
            Class_Set (otdwall.draw.type, otdwall.draw.otd_guid, finclass);
            for (OtdOpening &op : otdwall.openings) {
                Class_Set_OtdOpening (op, finclass);
            }
            group.Push (otdwall.draw.otd_guid);
        }
        for (OtdSlab &otdslab : otd.otdslab) {
            otdslab.draw.write_succeeded = false;
            if (!OtdSlab_ShouldDrawFinish (otd, otdslab))
                continue;
            OtdSlab_Draw (storyLevels, otdslab, subelementByparent);
            if (otdslab.draw.draw_error != NoError) {
                msg_rep ("OtdRoom_Draw", "OtdSlab_Draw err", otdslab.draw.draw_error, otdslab.draw.otd_guid);
                if (otdslab.draw.base_type == API_MeshID)
                    return otdslab.draw.draw_error;
                continue;
            }
            Param_AddUnicElementByType (otd.zone_guid,
                                        otdslab.draw.otd_guid,
                                        API_ZoneID,
                                        subelementByparent); // Привязка перекрытий к зоне
            Class_Set (otdslab.draw.type, otdslab.draw.otd_guid, finclass);
            group.Push (otdslab.draw.otd_guid);
        }
        for (OtdBeam &beam : otd.otdbeam_ceil) {
            beam.draw.write_succeeded = false;
            OtdBeam_Draw (storyLevels, beam, subelementByparent);
            if (beam.draw.draw_error != NoError) {
                msg_rep ("OtdRoom_Draw", "OtdBeam_Draw err", beam.draw.draw_error, beam.draw.otd_guid);
                continue;
            }
            Param_AddUnicElementByType (otd.zone_guid, beam.draw.otd_guid, API_ZoneID, subelementByparent);
            Class_Set (beam.draw.type, beam.draw.otd_guid, finclass);
            group.Push (beam.draw.otd_guid);
        }
        for (OtdBeam &beam : otd.otdbeam_floor) {
            beam.draw.write_succeeded = false;
            OtdBeam_Draw (storyLevels, beam, subelementByparent);
            if (beam.draw.draw_error != NoError) {
                msg_rep ("OtdRoom_Draw", "OtdBeam_Draw err", beam.draw.draw_error, beam.draw.otd_guid);
                continue;
            }
            Param_AddUnicElementByType (otd.zone_guid, beam.draw.otd_guid, API_ZoneID, subelementByparent);
            Class_Set (beam.draw.type, beam.draw.otd_guid, finclass);
            group.Push (beam.draw.otd_guid);
        }

        if (group.GetSize () > 1) {
    #ifdef ServerMainVers_2700
            err = ACAPI_Grouping_Tool (group, APITool_Group, nullptr);
    #else
            API_Guid groupGuid = APINULLGuid;
            err = ACAPI_ElementGroup_Create (group, &groupGuid);
        #ifdef ServerMainVers_2300
            if (err != NoError)
                err = ACAPI_Element_Tool (group, APITool_Group, nullptr);
        #endif
    #endif
            if (err != NoError)
                msg_rep ("OtdRoom_Draw", "ACAPI_Grouping_CreateGroup", err, APINULLGuid);
        }
        return err;
    }

    static void AppendRoomFinish (GS::Array<RoomBookFinishResult> &results,
                                  const API_Guid &zone,
                                  const OtdDrawData &draw,
                                  double length,
                                  bool hasLength,
                                  double area,
                                  bool hasArea) {
        if (!draw.write_succeeded || !draw.was_created || draw.draw_error != NoError || draw.otd_guid == APINULLGuid)
            return;
        RoomBookFinishResult result;
        result.zone_guid = zone;
        result.draw = draw;
        result.length = length;
        result.has_length = hasLength;
        result.area = area;
        result.has_area = hasArea;
        results.Push (result);
    }

    static double FinishLinearLength (const OtdLinearElement &geom) {
        return std::hypot (std::hypot (geom.seg.endC.x - geom.seg.begC.x, geom.seg.endC.y - geom.seg.begC.y),
                           geom.seg.endC.z - geom.seg.begC.z);
    }

    static void CollectRoomFinishes (const OtdRoom &room, GS::Array<RoomBookFinishResult> &results) {
        for (const OtdWall &wall : room.otdwall)
            AppendRoomFinish (
                results, room.zone_guid, wall.draw, FinishLinearLength (wall.geom), true, OtdWall_GetArea (wall), true);
        for (const OtdSlab &slab : room.otdslab) {
            const double area =
                slab.poly.CalcArea () *
                (slab.draw.base_type == API_MeshID ? std::hypot (1.0, std::hypot (slab.slope_x, slab.slope_y)) : 1.0);
            AppendRoomFinish (results, room.zone_guid, slab.draw, 0, false, area, true);
        }
        for (const OtdBeam &beam : room.otdbeam_ceil)
            AppendRoomFinish (results, room.zone_guid, beam.draw, FinishLinearLength (beam.geom), true, 0, false);
        for (const OtdBeam &beam : room.otdbeam_floor)
            AppendRoomFinish (results, room.zone_guid, beam.draw, FinishLinearLength (beam.geom), true, 0, false);
    }

    void OtdRooms_Draw (GS::HashTable<API_Guid, UnicGuidByBase> &exsistot_byzone,
                        GS::Array<API_Guid> &deletelist,
                        OtdRooms &roomsinfo,
                        const Stories &storyLevels,
                        ClassificationFunc::ClassificationDict &finclass,
                        UnicGuid &reserv_elements,
                        ParamDictElement &paramToWrite,
                        const API_ElemTypeID *elementTypes,
                        std::size_t elementTypeCount,
                        GS::Int32 &nPhase,
                        GS::Array<RoomBookFinishResult> *results) {
        GS::Array<RoomBookFinishResult> pendingResults;
        // Проверяем все Roof до очистки и записи модели; memo освобождаются внутри проверки.
        const GSErrCode roofErr = RoofBatch_Validate (storyLevels, roomsinfo);
        if (roofErr != NoError) {
            msg_rep ("Mesh floor", "Roof preflight failed; existing finishes retained", roofErr, APINULLGuid);
            return;
        }
        // Повторный запуск: назначаем совместимые существующие GUID окончательным участкам отделки.
        const GSErrCode matchErr = OtdRooms_AssignExistingWallFinishes (roomsinfo, exsistot_byzone, finclass);
        if (matchErr != NoError) {
            msg_rep ("RoomBook", "Cannot prepare existing finish updates", matchErr, APINULLGuid);
            return;
        }
        // Только список удаления; модель изменяется ниже, внутри команды создания и обновления.
        OtdRooms_CollectUnusedFinishingElements (exsistot_byzone, deletelist);
        UnicElementByType subelementByparent; // Словарь с созданными родительскими
                                              // и дочерними элементами
        // Отрисовка элементов отделки
        GS::UniString funcname = "Draw finishing element(s)";
        GS::UniString UndoString = RSGetIndString (ID_ADDON_STRINGS + isEng (), RoombookId, ACAPI_GetOwnResModule ());
        SuspendGroupsGuard suspGuard; // отключаем режим группировки на время записи.
        const GSErrCode drawErr = ACAPI_CallUndoableCommand (UndoString, [&] () -> GSErrCode {
            // Резервируем и разблокируем
            PrepareElementsResult prepareResult = PrepareElementsResult::NothingToDo;
            if (!PrepareElementsForUpdate (reserv_elements, prepareResult))
                return APIERR_CANCEL;
            // Удаляем устаревшие элементы отделки
            if (!deletelist.IsEmpty ()) {
                GSErrCode err = ACAPI_Element_Delete (deletelist);
                if (err != NoError) {
                    msg_rep ("RoomBook", "Cannot remove obsolete finishing elements", err, APINULLGuid);
                    return err;
                }
            }
            for (OtdRooms::PairIterator cIt = roomsinfo.EnumeratePairs (); cIt != NULL; ++cIt) {
    #ifdef ServerMainVers_2800
                OtdRoom &otd = cIt->value;
    #else
                OtdRoom &otd = *cIt->value;
    #endif
                if (AdvanceProcessPhase (funcname, nPhase))
                    return APIERR_CANCEL;
                const GSErrCode err = OtdRoom_Draw (storyLevels, otd, subelementByparent, finclass);
                if (err != NoError)
                    return err;
                if (results != nullptr && otd.isValid) {
                    RoomBookFinishResult roomResult;
                    roomResult.zone_guid = otd.zone_guid;
                    pendingResults.Push (roomResult);
                    CollectRoomFinishes (otd, pendingResults);
                }
            }
            return NoError;
        });
        if (drawErr != NoError) {
            msg_rep ("RoomBook", "Finish update rolled back", drawErr, APINULLGuid);
            return; // Связи частично выполненного пакета не публикуем.
        }
        if (results != nullptr)
            *results = pendingResults;
        // Только после успешной отрисовки: связи с основаниями/зонами и запись общего paramToWrite.
        OtdElements_SetSync (subelementByparent, paramToWrite, elementTypes, elementTypeCount, nPhase);
    }
} // namespace Roombook
#endif
