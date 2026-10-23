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

    static void OtdBeam_Draw_Beam (const GS::UniString &favorite_name, const Stories &storyLevels, OtdWall &edges);

    static void OtdBeam_Draw_Object (const GS::UniString &favorite_name, const Stories &storyLevels, OtdWall &edges);

    bool OtdBeam_GetDefault_Object (const GS::UniString &favorite_name,
                                    API_Element &beamelement,
                                    API_ElementMemo &beammemo);

    // -----------------------------------------------------------------------------
    // Построение отделочных стен аксессуаром
    // -----------------------------------------------------------------------------
    static void OtdWall_Draw_Object (const GS::UniString &favorite_name, const Stories &storyLevels, OtdWall &edges);

    // -----------------------------------------------------------------------------
    // Построение отделочных стен стандартной стеной
    // -----------------------------------------------------------------------------
    static void OtdWall_Draw_Wall (const GS::UniString &favorite_name,
                                   const Stories &storyLevels,
                                   OtdWall &edges,
                                   UnicElementByType &subelementByparent);

    // -----------------------------------------------------------------------------
    // Построение проёмов в отделочных стенах
    // -----------------------------------------------------------------------------
    static void OtdOpening_Draw (API_Element &wallelement,
                                 OtdOpening &op,
                                 UnicElementByType &subelementByparent,
                                 double zBottom);

    // -----------------------------------------------------------------------------
    // Построение потолка/пола (общая)
    // -----------------------------------------------------------------------------
    static void OtdSlab_Draw (const Stories &storyLevels, OtdSlab &otdslab, UnicElementByType &subelementByparent);

    static void OtdSlab_Draw_Slab (const GS::UniString &favorite_name, const Stories &storyLevels, OtdSlab &otdslab);

    static void OtdSlab_Draw_Object (const GS::UniString &favorite_name, const Stories &storyLevels, OtdSlab &otdslab);

    static void OtdWall_SetClass (const OtdWall &op, const ClassificationFunc::ClassificationDict &finclass);

    static void OtdOpening_SetClass (const OtdOpening &op, const ClassificationFunc::ClassificationDict &finclass);

    static void OtdSlab_SetClass (const OtdSlab &op, const ClassificationFunc::ClassificationDict &finclass);

    // Локальные настройки для создания элементов; общим этапам эти функции не нужны.
    static bool OtdBeam_GetDefault_Beam (const GS::UniString &favorite_name,
                                         API_Element &beamelement,
                                         API_ElementMemo &beammemo);

    static bool OtdWall_GetDefault_Object (const GS::UniString &favorite_name,
                                           API_Element &wallobjelement,
                                           API_ElementMemo &wallobjmemo);

    static bool OtdOpening_GetDefault (const GS::UniString &favorite_name,
                                       API_Element &windowelement,
                                       API_ElementMemo &windowmemo);

    static bool OtdSlab_GetDefault_Object (const GS::UniString &favorite_name,
                                           API_Element &slabobjelement,
                                           API_ElementMemo &slabobjmemo);

    static bool OtdSlab_GetDefault_Slab (const GS::UniString &favorite_name, API_Element &slabelement);

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

    static bool OtdWall_ShouldDrawFinish (const OtdRoom &room, const OtdWall &wall) {
        if (!room.isValid || !wall.isValid)
            return false;
        switch (wall.type) {
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
        default:
            return room.create_wall_elements || room.create_column_elements || room.create_reveal_elements ||
                   room.create_ceil_elements;
        }
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
            if (type == API_BeamID && (target.beam.nSegments != 1 || element->beam.nSegments != 1))
                continue;
            if (type != API_WallID && type != API_ObjectID && type != API_BeamID)
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
                if (slab.otd_guid != APINULLGuid)
                    used.Add (slab.otd_guid, true);
            for (const auto &wall : room.otdwall)
                if (wall.otd_guid != APINULLGuid)
                    used.Add (wall.otd_guid, true);
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
            for (auto &wall : room.otdwall) {
                if (!OtdWall_ShouldDrawFinish (room, wall) || wall.otd_guid != APINULLGuid || wall.height < min_dim ||
                    std::hypot (wall.endC.x - wall.begC.x, wall.endC.y - wall.begC.y) < min_dim)
                    continue;
                const API_Guid classGuid = Class_GetClassGuid (wall.type, finclass);
                if (classGuid == APINULLGuid)
                    continue;
                auto *byType = byBase->GetPtr (wall.base_guid);
                if (byType == nullptr)
                    continue;
                auto *pool = byType->GetPtr (Class_GetOtdTypeByClass (classGuid, finclass));
                if (pool == nullptr || pool->IsEmpty ())
                    continue;
                API_Element target = {};
                API_ElementMemo memo = {};
                const GS::OnExit memoGuard ([&] () { ACAPI_DisposeElemMemoHdls (&memo); });
                bool ready = false;
                if (wall.surface_geometry || wall.favorite.type == API_WallID) {
                    ready = OtdWall_GetDefault_Wall (wall.favorite.name, target);
                    if (wall.surface_geometry)
                        target.wall.type = APIWtyp_Normal;
                } else if (wall.favorite.type == API_ObjectID && wall.draw_type != API_BeamID) {
                    ready = OtdWall_GetDefault_Object (wall.favorite.name, target, memo);
                } else if (wall.favorite.type == API_BeamID) {
                    ready = OtdBeam_GetDefault_Beam (wall.favorite.name, target, memo);
                } else {
                    continue; // Незавершённый Object-writer горизонтального откоса не активируем.
                }
                if (!ready)
                    return APIERR_BADPARS;
                UnicGuid candidates;
                for (auto it = pool->EnumeratePairs (); it != nullptr; ++it) {
    #ifdef ServerMainVers_2800
                    const API_Guid guid = it->key;
    #else
                    const API_Guid guid = *it->key;
    #endif
                    if (used.ContainsKey (guid))
                        continue;
                    if (!existing.ContainsKey (guid)) {
                        API_Element element = {};
                        element.header.guid = guid;
                        const GSErrCode err = ACAPI_Element_Get (&element);
                        if (err != NoError)
                            return err;
                        existing.Add (guid, element);
                    }
                    candidates.Add (guid, true);
                }
                wall.otd_guid = OtdWall_TakeExistingFinish (candidates, target, existing);
                if (wall.otd_guid != APINULLGuid) {
                    used.Add (wall.otd_guid, true);
                    pool->Delete (wall.otd_guid);
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

    static GSErrCode OtdRoom_Draw (const Stories &storyLevels,
                                   OtdRoom &otd,
                                   UnicElementByType &subelementByparent,
                                   ClassificationFunc::ClassificationDict &finclass) {
        GSErrCode err = NoError;
        GS::Array<API_Guid> group;
        if (!otd.isValid)
            return NoError;
        API_Guid class_guid = APINULLGuid;
        if (otd.create_wall_elements || otd.create_column_elements || otd.create_reveal_elements ||
            otd.create_ceil_elements) {
            for (OtdWall &otdwall : otd.otdwall) {
                if (!otdwall.isValid)
                    continue;
                if ((otdwall.type == Wall_Main || otdwall.type == Wall_Up || otdwall.type == Wall_Down) &&
                    !otd.create_wall_elements)
                    continue;
                if (otdwall.type == Ceil && !otd.create_ceil_elements)
                    continue;
                if ((otdwall.type == Column) && !otd.create_column_elements)
                    continue;
                if ((otdwall.type == Reveal_Main || otdwall.type == Reveal_Down || otdwall.type == Reveal_Up) &&
                    !otd.create_reveal_elements)
                    continue;
                OtdWall_Draw (storyLevels, otdwall, subelementByparent);
                if (otdwall.draw_error != NoError)
                    return otdwall.draw_error;
                if (!otdwall.isValid)
                    continue;
                if (otdwall.otd_guid != APINULLGuid) {
                    Param_AddUnicElementByType (otd.zone_guid,
                                                otdwall.otd_guid,
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
                    OtdWall_SetClass (otdwall, finclass);
                    for (OtdOpening &op : otdwall.openings) {
                        if (op.otd_guid == APINULLGuid)
                            continue;
                        OtdOpening_SetClass (op, finclass);
                    }
                    group.Push (otdwall.otd_guid);
                } else {
                    msg_rep ("OtdRoom_Draw", "otdwall.otd_guid == APINULLGuid", NoError, APINULLGuid);
                }
            }
        } else {
            otd.otdwall.Clear ();
        }
        if (otd.create_ceil_elements || otd.create_floor_elements) {
            for (OtdSlab &otdslab : otd.otdslab) {
                if (!otdslab.isValid)
                    continue;
                if (otdslab.type == Ceil && !otd.create_ceil_elements)
                    continue;
                if (otdslab.type == Floor && !otd.create_floor_elements)
                    continue;
                OtdSlab_Draw (storyLevels, otdslab, subelementByparent);
                if (otdslab.otd_guid != APINULLGuid) {
                    Param_AddUnicElementByType (otd.zone_guid,
                                                otdslab.otd_guid,
                                                API_ZoneID,
                                                subelementByparent); // Привязка перекрытий к зоне
                    OtdSlab_SetClass (otdslab, finclass);
                    group.Push (otdslab.otd_guid);
                } else {
                    msg_rep ("OtdRoom_Draw", "otdslab.otd_guid == APINULLGuid", NoError, APINULLGuid);
                }
            }
        } else {
            otd.otdslab.Clear ();
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

    // -----------------------------------------------------------------------------
    // Связи отделки с основаниями и зонами; общая запись накопленных свойств, включая сводку площадей.
    // -----------------------------------------------------------------------------
    static void OtdWall_SetSync (UnicElementByType &subelementByparent,
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
        for (std::size_t typeIndex = 0; typeIndex < elementTypeCount; ++typeIndex) {
            const API_ElemTypeID &typeelem = elementTypes[typeIndex];
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
        GS::Array<API_Guid> rereadelem = SyncArray (syncSettings, syncguids);
        if (!rereadelem.IsEmpty ()) {
            SyncArray (syncSettings, rereadelem);
        }
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
                        GS::Int32 &nPhase) {
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
            }
            return NoError;
        });
        if (drawErr != NoError) {
            msg_rep ("RoomBook", "Finish update rolled back", drawErr, APINULLGuid);
            return; // Связи частично выполненного пакета не публикуем.
        }
        // Только после успешной отрисовки: связи с основаниями/зонами и запись общего paramToWrite.
        OtdWall_SetSync (subelementByparent, paramToWrite, elementTypes, elementTypeCount, nPhase);
    }

    // -----------------------------------------------------------------------------
    // Построение отделочных стен (общая)
    // -----------------------------------------------------------------------------
    void OtdWall_Draw (const Stories &storyLevels, OtdWall &edges, UnicElementByType &subelementByparent) {
        edges.draw_error = NoError;
        if (!std::isfinite (edges.height) || !std::isfinite (edges.zBottom) || edges.height < min_dim) {
            edges.otd_guid = APINULLGuid;
            edges.draw_error = APIERR_BADPARS;
            return;
        }
        double dx = -edges.endC.x + edges.begC.x;
        double dy = -edges.endC.y + edges.begC.y;
        double dr = sqrt (dx * dx + dy * dy);
        if (!std::isfinite (dr) || dr < min_dim) {
            edges.otd_guid = APINULLGuid;
            edges.draw_error = APIERR_BADPARS;
            return;
        }
        if (edges.surface_geometry) {
            OtdWall_Draw_Wall (edges.favorite.name, storyLevels, edges, subelementByparent);
            if (edges.isValid && edges.otd_guid != APINULLGuid)
                Param_AddUnicElementByType (edges.base_guid, edges.otd_guid, edges.base_type, subelementByparent);
            return;
        }
        if (edges.favorite.type == API_ObjectID) {
            // горизонтальную часть откоса строим в этом случае аксессуаром
            // пола/потолка
            if (edges.draw_type == API_BeamID) {
                OtdBeam_Draw_Object (edges.favorite.name, storyLevels, edges);
            } else {
                OtdWall_Draw_Object (edges.favorite.name, storyLevels, edges);
            }
        }
        if (edges.favorite.type == API_WallID) {
            OtdWall_Draw_Wall (edges.favorite.name, storyLevels, edges, subelementByparent);
        }
        if (edges.favorite.type == API_BeamID) {
            OtdBeam_Draw_Beam (edges.favorite.name, storyLevels, edges);
        }
        Param_AddUnicElementByType (edges.base_guid,
                                    edges.otd_guid,
                                    API_WallID,
                                    subelementByparent); // Привязка отделочной стены к базовой
    }

    // Создаёт балку с координатами, размерами первого сегмента и материалом из edges; задаёт этаж и отметку.
    // После успешного создания записывает GUID в edges.otd_guid, при ошибке создания сообщает о ней.
    static void OtdBeam_Draw_Beam (const GS::UniString &favorite_name, const Stories &storyLevels, OtdWall &edges) {
        const API_Guid existingGuid = edges.otd_guid;
        edges.otd_guid = APINULLGuid;
        edges.draw_error = APIERR_BADPARS;
        GSErrCode err = NoError;
        API_Element beamelement = {};
        API_ElementMemo beammemo = {};
        const GS::OnExit memoGuard ([&beammemo] () { ACAPI_DisposeElemMemoHdls (&beammemo); });
        if (existingGuid == APINULLGuid) {
            if (!OtdBeam_GetDefault_Beam (favorite_name, beamelement, beammemo))
                return;
        } else {
            beamelement.header.guid = existingGuid;
            err = ACAPI_Element_Get (&beamelement);
            if (err == NoError && GetElemTypeID (beamelement) == API_BeamID && beamelement.beam.nSegments == 1)
                err = ACAPI_Element_GetMemo (existingGuid, &beammemo, APIMemoMask_BeamSegment);
            else if (err == NoError)
                err = APIERR_BADID;
            if (err != NoError) {
                edges.draw_error = err;
                return;
            }
        }
        if (beammemo.beamSegments == nullptr ||
            (existingGuid != APINULLGuid &&
             beammemo.beamSegments[0].assemblySegmentData.modelElemStructureType == API_ProfileStructure))
            return;
        beamelement.beam.begC = edges.begC;
        beamelement.beam.endC = edges.endC;
        if (beammemo.beamSegments != nullptr) {
            beammemo.beamSegments[0].assemblySegmentData.nominalWidth = edges.width;
            beammemo.beamSegments[0].assemblySegmentData.nominalHeight = edges.height;

    #ifdef ServerMainVers_2700
            // Материал горизонтального откоса: для AC_27+ структура API_BeamSegmentType содержит
            // bottomMaterial, leftMaterial, rightMaterial, topMaterial, endsMaterial (AC_29+ extrusionMaterial)
            // AC27: API_OverriddenAttribute заменён на APIOptional<API_AttributeIndex>
            // (как сделано ниже для стены: hasValue + value).
            const API_AttributeIndex beamMaterial = ACAPI_CreateAttributeIndex (edges.material.material);
            beammemo.beamSegments[0].bottomMaterial.hasValue = true;
            beammemo.beamSegments[0].bottomMaterial.value = beamMaterial;

            beammemo.beamSegments[0].leftMaterial.hasValue = true;
            beammemo.beamSegments[0].leftMaterial.value = beamMaterial;

            beammemo.beamSegments[0].rightMaterial.hasValue = true;
            beammemo.beamSegments[0].rightMaterial.value = beamMaterial;

            beammemo.beamSegments[0].topMaterial.hasValue = true;
            beammemo.beamSegments[0].topMaterial.value = beamMaterial;

            beammemo.beamSegments[0].endsMaterial.hasValue = true;
            beammemo.beamSegments[0].endsMaterial.value = beamMaterial;
    #else
            beammemo.beamSegments[0].bottomMaterial.overridden = true;
            beammemo.beamSegments[0].bottomMaterial.attributeIndex = edges.material.material;

            beammemo.beamSegments[0].leftMaterial.overridden = true;
            beammemo.beamSegments[0].leftMaterial.attributeIndex = edges.material.material;

            beammemo.beamSegments[0].rightMaterial.overridden = true;
            beammemo.beamSegments[0].rightMaterial.attributeIndex = edges.material.material;
    #endif
        }
        beamelement.header.floorInd = edges.floorInd;
        beamelement.beam.level = GetOffsetFromStory (edges.zBottom, edges.floorInd, storyLevels);
        beamelement.beam.isFlipped = false;
        if (existingGuid == APINULLGuid) {
            err = ACAPI_Element_Create (&beamelement, &beammemo);
        } else {
            API_Element mask = {};
            ACAPI_ELEMENT_MASK_SET (mask, API_BeamType, begC);
            ACAPI_ELEMENT_MASK_SET (mask, API_BeamType, endC);
            ACAPI_ELEMENT_MASK_SET (mask, API_BeamType, level);
            ACAPI_ELEMENT_MASK_SET (mask, API_BeamType, isFlipped);
            ACAPI_ELEMENT_MASK_SET (mask, API_Elem_Head, floorInd);
            err = ACAPI_Element_Change (&beamelement, &mask, &beammemo, APIMemoMask_BeamSegment, true);
        }
        edges.draw_error = err;
        if (err != NoError) {
            msg_rep ("OtdBeam_Draw_Beam err", "ACAPI_Element_Create", err, APINULLGuid);
            return;
        }
        edges.otd_guid = beamelement.header.guid;
    }

    // Обнуляет элемент и memo; использует избранное, если оно содержит балку, иначе получает настройки балки по
    // умолчанию. При ошибке получения настроек освобождает memo и возвращает false.
    static bool OtdBeam_GetDefault_Beam (const GS::UniString &favorite_name,
                                         API_Element &beamelement,
                                         API_ElementMemo &beammemo) {
        GSErrCode err = NoError;
        BNZeroMemory (&beamelement, sizeof (API_Element));
        BNZeroMemory (&beammemo, sizeof (API_ElementMemo));
        if (Favorite_GetByName (favorite_name, beamelement, beammemo)) {
            if (GetElemTypeID (beamelement) == API_BeamID)
                return true;
        }
        SetElemTypeID (beamelement, API_BeamID);
        err = ACAPI_Element_GetDefaults (&beamelement, &beammemo);
        if (err != NoError) {
            ACAPI_DisposeElemMemoHdls (&beammemo);
            return false;
        }
        return true;
    }

    // Ничего не делает: тело создания объекта закомментировано, edges.otd_guid не изменяется.
    static void OtdBeam_Draw_Object (const GS::UniString &favorite_name, const Stories &storyLevels, OtdWall &edges) {
        // API_Element beamelement;
        // API_ElementMemo beammemo;
        // GSErrCode err = NoError;
        // if (!OtdBeam_GetDefault_Object (favorite_name, beamelement, beammemo)) {
        //     return;
        // }
        // edges.otd_guid = beamelement.header.guid;
        // ACAPI_DisposeElemMemoHdls (&beammemo);
    }

    // Обнуляет элемент и memo; возвращает объект из избранного либо получает настройки объекта по умолчанию.
    // Только для настроек по умолчанию сбрасывает угол и автовидимость по этажам, включает параметры пера, линий и
    // материалов объекта.
    bool OtdBeam_GetDefault_Object (const GS::UniString &favorite_name,
                                    API_Element &beamelement,
                                    API_ElementMemo &beammemo) {
        GSErrCode err = NoError;
        BNZeroMemory (&beammemo, sizeof (API_ElementMemo));
        BNZeroMemory (&beamelement, sizeof (API_Element));
        if (Favorite_GetByName (favorite_name, beamelement, beammemo)) {
            if (GetElemTypeID (beamelement) == API_ObjectID)
                return true;
        }
        SetElemTypeID (beamelement, API_ObjectID);
        err = ACAPI_Element_GetDefaults (&beamelement, &beammemo);
        if (err != NoError) {
            ACAPI_DisposeElemMemoHdls (&beammemo);
            return false;
        }
        beamelement.object.angle = 0;
        beamelement.object.isAutoOnStoryVisibility = false;
        beamelement.object.useObjPens = true;
        beamelement.object.useObjLtypes = true;
        beamelement.object.useObjMaterials = true;
        return true;
    }

    // -----------------------------------------------------------------------------
    // Построение отделочных стен аксессуаром
    // -----------------------------------------------------------------------------
    static void OtdWall_Draw_Object (const GS::UniString &favorite_name, const Stories &storyLevels, OtdWall &edges) {
        const API_Guid existingGuid = edges.otd_guid;
        edges.otd_guid = APINULLGuid;
        edges.draw_error = APIERR_BADPARS;
        GSErrCode err = NoError;
        API_Element wallobjelement = {};
        API_ElementMemo wallobjmemo = {};
        const GS::OnExit memoGuard ([&wallobjmemo] () { ACAPI_DisposeElemMemoHdls (&wallobjmemo); });
        if (existingGuid == APINULLGuid) {
            if (!OtdWall_GetDefault_Object (favorite_name, wallobjelement, wallobjmemo))
                return;
        } else {
            wallobjelement.header.guid = existingGuid;
            err = ACAPI_Element_Get (&wallobjelement);
            if (err == NoError && GetElemTypeID (wallobjelement) == API_ObjectID)
                err = ACAPI_Element_GetMemo (existingGuid, &wallobjmemo, APIMemoMask_AddPars);
            else if (err == NoError)
                err = APIERR_BADID;
            if (err != NoError) {
                edges.draw_error = err;
                return;
            }
        }
        Point2D wbegC = {edges.begC.x, edges.begC.y};
        Point2D wendC = {edges.endC.x, edges.endC.y};
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
        p.num = edges.material.material;
        accsessoryparams.Add ("{@gdl:gs_bw_mat}", p);
        p.num = edges.material.material;
        accsessoryparams.Add ("{@gdl:matp1}", p);

        p.num = edges.height;
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
        p.arr_num.Push (edges.height);
        p.arr_num.Push (ac_wall_length);
        p.arr_num.Push (edges.height);
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
        p.arr_num.Push (edges.ang_begC);
        p.arr_num.Push (edges.ang_endC);
        accsessoryparams.Add ("{@gdl:ac_angles}", p);
        p.arr_num.Clear ();

        if (edges.openings.IsEmpty ()) {
            p.dim1 = 1;
            p.dim2 = 2;
            p.arr_num.Push (0);
            p.arr_num.Push (0);
        } else {
            Int32 dim2 = 0;
            UInt32 addedOpenings = 0;
            for (OtdOpening &op : edges.openings) {
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
        GDLHelpers::ParamToMemo (wallobjmemo, accsessoryparams);
        Vector2D ref = {-1, 0};
        Vector2D wallvect = walldir.Get ().ToVector2D ();
        double ang = wallvect.CalcAngleToReference (ref);
        wallobjelement.object.angle = ang;
        wallobjelement.object.pos = edges.endC;
        wallobjelement.header.floorInd = edges.floorInd;
    #ifdef ServerMainVers_2700
        API_AttributeIndex ematerial = ACAPI_CreateAttributeIndex (edges.material.material);
        wallobjelement.object.mat = ematerial;
    #else
        wallobjelement.object.mat = edges.material.material;
    #endif
        wallobjelement.object.level = GetOffsetFromStory (edges.zBottom, edges.floorInd, storyLevels);
        if (existingGuid == APINULLGuid) {
            err = ACAPI_Element_Create (&wallobjelement, &wallobjmemo);
        } else {
            API_Element mask = {};
            ACAPI_ELEMENT_MASK_SET (mask, API_ObjectType, pos);
            ACAPI_ELEMENT_MASK_SET (mask, API_ObjectType, angle);
            ACAPI_ELEMENT_MASK_SET (mask, API_ObjectType, level);
            ACAPI_ELEMENT_MASK_SET (mask, API_ObjectType, mat);
            ACAPI_ELEMENT_MASK_SET (mask, API_Elem_Head, floorInd);
            err = ACAPI_Element_Change (&wallobjelement, &mask, &wallobjmemo, APIMemoMask_AddPars, true);
        }
        edges.draw_error = err;
        if (err != NoError) {
            if (wallobjelement.object.libInd == 0) {
                GS::UniString message =
                    "Cannot create or update the wall finish: the GDL object has no valid library part (library index is 0). Favorite: '";
                message += favorite_name;
                message +=
                    "'. Check that the required library is loaded and reselect the GDL object in the Favorite. If no Favorite is used, check the Object tool defaults.";
                msg_rep ("RoomBook", message, err, edges.base_guid);
            } else {
                msg_rep ("OtdWall_Draw_Object err", "ACAPI_Element_Create", err, APINULLGuid);
            }
            return;
        }
        edges.otd_guid = wallobjelement.header.guid;
    }

    // -----------------------------------------------------------------------------
    // Получение из избранного аксессуара стены для простроения
    // -----------------------------------------------------------------------------
    // Обнуляет элемент и memo; возвращает объект из избранного либо получает настройки объекта по умолчанию.
    // Только в ветке настроек по умолчанию задаёт угол, видимость и использование перьев, линий и материалов объекта.
    static bool OtdWall_GetDefault_Object (const GS::UniString &favorite_name,
                                           API_Element &wallobjelement,
                                           API_ElementMemo &wallobjmemo) {
        GSErrCode err = NoError;
        BNZeroMemory (&wallobjmemo, sizeof (API_ElementMemo));
        BNZeroMemory (&wallobjelement, sizeof (API_Element));
        if (Favorite_GetByName (favorite_name, wallobjelement, wallobjmemo)) {
            if (GetElemTypeID (wallobjelement) == API_ObjectID)
                return true;
        }
        SetElemTypeID (wallobjelement, API_ObjectID);
        err = ACAPI_Element_GetDefaults (&wallobjelement, &wallobjmemo);
        if (err != NoError) {
            ACAPI_DisposeElemMemoHdls (&wallobjmemo);
            return false;
        }
        wallobjelement.object.angle = 0;
        wallobjelement.object.isAutoOnStoryVisibility = false;
        wallobjelement.object.useObjPens = true;
        wallobjelement.object.useObjLtypes = true;
        wallobjelement.object.useObjMaterials = true;
        return true;
    }

    // -----------------------------------------------------------------------------
    // Построение отделочных стен стандартной стеной
    // -----------------------------------------------------------------------------
    static void OtdWall_Draw_Wall (const GS::UniString &favorite_name,
                                   const Stories &storyLevels,
                                   OtdWall &edges,
                                   UnicElementByType &subelementByparent) {
        const API_Guid existingGuid = edges.otd_guid;
        edges.otd_guid = APINULLGuid;
        edges.draw_error = APIERR_BADPARS;
        GSErrCode err = NoError;
        API_Element wallelement = {};
        if (!OtdWall_GetDefault_Wall (favorite_name, wallelement)) {
            return;
        }
        if (edges.surface_geometry) {
            if (wallelement.wall.modelElemStructureType == API_ProfileStructure) {
                msg_rep (
                    "Surface finish", "Profile wall favorite is not representable", APIERR_BADPARS, edges.base_guid);
                return;
            }
            wallelement.wall.type = APIWtyp_Normal;
            wallelement.wall.referenceLineLocation = APIWallRefLine_Inside;
            wallelement.wall.offset = 0;
            wallelement.wall.offsetFromOutside = wallelement.wall.thickness;
        }
        const bool is_upper_reveal =
            !edges.surface_geometry && edges.draw_type == API_BeamID && edges.favorite.type == API_WallID;
        // Откосы
        if (is_upper_reveal) {
            edges.height = wallelement.wall.thickness;
            edges.zBottom -= edges.height;
            if (!is_equal (edges.width, 0) && edges.width > 0)
                wallelement.wall.thickness = edges.width;
            wallelement.wall.offset = wallelement.wall.thickness;
            wallelement.wall.offsetFromOutside = 0;
        }
        // Верхний откос уже ориентирован при подготовке; повторный разворот меняет сторону глубины.
        wallelement.wall.begC = edges.begC;
        wallelement.wall.endC = edges.endC;
        wallelement.wall.height = edges.height;
        if (edges.slant_angle > 0 && !is_upper_reveal) {
            wallelement.wall.profileType = is_equal (edges.slant_angle, 90 * DEGRAD) ? APISect_Normal : APISect_Slanted;
            wallelement.wall.slantAlpha = edges.slant_angle;
            wallelement.wall.relativeTopStory = 0;
        }
        wallelement.header.floorInd = edges.floorInd;
        wallelement.wall.bottomOffset = GetOffsetFromStory (edges.zBottom, edges.floorInd, storyLevels);
    #ifdef ServerMainVers_2700
        API_AttributeIndex ematerial = ACAPI_CreateAttributeIndex (edges.material.material);
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
        wallelement.wall.refMat.attributeIndex = edges.material.material;
        wallelement.wall.oppMat.attributeIndex = edges.material.material;
        wallelement.wall.sidMat.attributeIndex = edges.material.material;
    #endif
        wallelement.wall.zoneRel = APIZRel_None;
        wallelement.wall.flipped = false;
        if (existingGuid == APINULLGuid) {
            err = ACAPI_Element_Create (&wallelement, nullptr);
        } else {
            API_Element current = {};
            current.header.guid = existingGuid;
            err = ACAPI_Element_Get (&current);
            if (err != NoError) {
                edges.draw_error = err;
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
                    edges.draw_error = children.error;
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
                    edges.draw_error = err;
                    return;
                }
            }
            wallelement.header.guid = existingGuid;
            API_Element mask = {};
            ACAPI_ELEMENT_MASK_SET (mask, API_Elem_Head, floorInd);
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
            ACAPI_ELEMENT_MASK_SET (mask, API_WallType, offsetFromOutside);
            ACAPI_ELEMENT_MASK_SET (mask, API_WallType, modelElemStructureType);
            ACAPI_ELEMENT_MASK_SET (mask, API_WallType, buildingMaterial);
            ACAPI_ELEMENT_MASK_SET (mask, API_WallType, composite);
            ACAPI_ELEMENT_MASK_SET (mask, API_WallType, profileType);
            ACAPI_ELEMENT_MASK_SET (mask, API_WallType, slantAlpha);
            ACAPI_ELEMENT_MASK_SET (mask, API_WallType, slantBeta);
            ACAPI_ELEMENT_MASK_SET (mask, API_WallType, refMat);
            ACAPI_ELEMENT_MASK_SET (mask, API_WallType, oppMat);
            ACAPI_ELEMENT_MASK_SET (mask, API_WallType, sidMat);
            ACAPI_ELEMENT_MASK_SET (mask, API_WallType, zoneRel);
            ACAPI_ELEMENT_MASK_SET (mask, API_WallType, flipped);
            err = ACAPI_Element_Change (&wallelement, &mask, nullptr, 0, true);
        }
        edges.draw_error = err;
        if (err != NoError) {
            msg_rep ("OtdWall_Draw_Wall err", "ACAPI_Element_Create", err, APINULLGuid);
            return;
        }
        edges.otd_guid = wallelement.header.guid;
        for (OtdOpening &op : edges.openings) {
            op.otd_guid = APINULLGuid;
            OtdOpening_Draw (wallelement, op, subelementByparent, edges.zBottom);
            const bool opening_failed = op.otd_guid == APINULLGuid;
            if (!opening_failed)
                continue;
            if (existingGuid != APINULLGuid || !edges.surface_geometry) {
                edges.draw_error = APIERR_BADPARS;
                edges.otd_guid = APINULLGuid;
                return; // Вызывающий undo scope откатит также старые вырезы.
            }
            edges.draw_error = APIERR_BADPARS;
            // Непрорезанная оболочка даёт лишнюю отделку; удаляем только новый результат.
            const GS::Array<API_Guid> incomplete = {wallelement.header.guid};
            const GSErrCode deleteErr = ACAPI_Element_Delete (incomplete);
            msg_rep ("Surface finish", "Opening failed; removing incomplete wall", deleteErr, edges.base_guid);
            edges.isValid = false;
            if (deleteErr == NoError) {
                edges.otd_guid = APINULLGuid;
                for (auto &opening : edges.openings)
                    opening.otd_guid = APINULLGuid;
            }
            return;
        }
    }

    // -----------------------------------------------------------------------------
    // Получение из избранного стены для простроения
    // -----------------------------------------------------------------------------
    // Возвращает стену из избранного, если её тип подходит; иначе получает настройки стены по умолчанию.
    // В ветке настроек по умолчанию задаёт базовую структуру, толщину otd_thickness, внутреннюю линию привязки и
    // zoneRel.
    bool OtdWall_GetDefault_Wall (const GS::UniString &favorite_name, API_Element &wallelement) {
        GSErrCode err = NoError;
        BNZeroMemory (&wallelement, sizeof (API_Element));
        if (Favorite_GetByName (favorite_name, wallelement)) {
            if (GetElemTypeID (wallelement) == API_WallID)
                return true;
        }
        SetElemTypeID (wallelement, API_WallID);
        err = ACAPI_Element_GetDefaults (&wallelement, nullptr);
        if (err != NoError) {
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
    // Построение проёмов в отделочных стенах
    // -----------------------------------------------------------------------------
    static void OtdOpening_Draw (API_Element &wallelement,
                                 OtdOpening &op,
                                 UnicElementByType &subelementByparent,
                                 double zBottom) {
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
        Param_AddUnicElementByType (op.base_guid,
                                    op.otd_guid,
                                    API_WindowID,
                                    subelementByparent); // Привязка отделочного проёма к базовому
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
    // Построение потолка/пола (общая)
    // -----------------------------------------------------------------------------
    static void OtdSlab_Draw (const Stories &storyLevels, OtdSlab &otdslab, UnicElementByType &subelementByparent) {
        if (otdslab.surface_geometry) {
            if (otdslab.favorite.type == API_ObjectID) {
                OtdSlab_Draw_Object (otdslab.favorite.name, storyLevels, otdslab);
            } else {
                OtdSlab_Draw_Slab (otdslab.favorite.name, storyLevels, otdslab);
            }
            if (otdslab.otd_guid != APINULLGuid)
                Param_AddUnicElementByType (otdslab.base_guid, otdslab.otd_guid, API_SlabID, subelementByparent);
            return;
        }
        // Провери тип существующего элемента
        API_ElemTypeID type = otdslab.favorite.type;
        API_Guid otd_guid = otdslab.otd_guid;
        bool is_new = (otd_guid == APINULLGuid);
        if (!is_new) {
            type = GetElemTypeID (otd_guid);
        }
        if (type == API_ObjectID) {
            OtdSlab_Draw_Object (otdslab.favorite.name, storyLevels, otdslab);
        } else {
            OtdSlab_Draw_Slab (otdslab.favorite.name, storyLevels, otdslab);
        }
        // Если обновить объект не получилось - удаляем старый и создаём новый
        if (!is_new && otdslab.otd_guid == APINULLGuid) {
            msg_rep ("OtdSlab_Draw err", "Can't update element, create new", NoError, APINULLGuid);
            GS::Array<API_Guid> deletelist = {otd_guid};
            ACAPI_Element_Delete (deletelist);
            if (otdslab.favorite.type == API_ObjectID) {
                OtdSlab_Draw_Object (otdslab.favorite.name, storyLevels, otdslab);
            } else {
                OtdSlab_Draw_Slab (otdslab.favorite.name, storyLevels, otdslab);
            }
        }
        otdslab.poly.Clear ();
        Param_AddUnicElementByType (otdslab.base_guid,
                                    otdslab.otd_guid,
                                    API_SlabID,
                                    subelementByparent); // Привязка отделочного перекрытия к базовому
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
            if (neighbor.type != Floor || neighbor.base_type != API_MeshID)
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
    static GSErrCode OtdSlab_PrepareRoof (const Stories &storyLevels,
                                          const OtdSlab &floor,
                                          const API_Element &slabSettings,
                                          const GS::Array<OtdSlab> *neighbors,
                                          API_Element &element,
                                          API_ElementMemo &memo) {
        if (GetElemTypeID (slabSettings) != API_SlabID || floor.type != Floor || !std::isfinite (floor.zBottom) ||
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

    GSErrCode OtdSlab_CreateRoof (const Stories &storyLevels,
                                  OtdSlab &floor,
                                  const API_Element &slabSettings,
                                  const GS::Array<OtdSlab> *neighbors) {
        API_Element element = {};
        API_ElementMemo memo = {};
        const GS::OnExit memoGuard ([&memo] () { ACAPI_DisposeElemMemoHdls (&memo); });
        GSErrCode error = OtdSlab_PrepareRoof (storyLevels, floor, slabSettings, neighbors, element, memo);
        if (error == NoError)
            error = ACAPI_Element_Create (&element, &memo);
        if (error == NoError)
            floor.otd_guid = element.header.guid;
        return error;
    }

    // Создаёт или изменяет перекрытие по полигону, отметке и материалу otdslab; пропускает площадь менее 0.0001.
    // При изменении задаёт материалы и перпендикулярную подрезку рёбер; GUID записывает только при успехе и не обнуляет
    // при ошибках.
    static void OtdSlab_Draw_Slab (const GS::UniString &favorite_name, const Stories &storyLevels, OtdSlab &otdslab) {
        GSErrCode err = NoError;
        if (otdslab.poly.CalcArea () < 0.0001)
            return;
        API_Element slabelement = {};
        bool is_new = (otdslab.otd_guid == APINULLGuid); // Элемент уже существует, нужно только обновить
        API_ElementMemo memo = {};
        const GS::OnExit memoGuard ([&memo] () { ACAPI_DisposeElemMemoHdls (&memo); });
        if (is_new) {
            if (!OtdSlab_GetDefault_Slab (favorite_name, slabelement))
                return;
        } else {
            slabelement.header.guid = otdslab.otd_guid;
            err = ACAPI_Element_Get (&slabelement);
            if (err != NoError) {
                msg_rep ("OtdSlab_Draw_Object", "ACAPI_Element_Get", err, otdslab.otd_guid);
                return;
            }
            if (GetElemTypeID (slabelement) != API_SlabID)
                return;
            err = ACAPI_Element_GetMemo (slabelement.header.guid, &memo, APIMemoMask_Polygon);
            if (err != NoError) {
                msg_rep ("OtdSlab_Draw_Object", "ACAPI_Element_GetMemo", err, otdslab.otd_guid);
                return;
            }
        }
        slabelement.header.floorInd = otdslab.floorInd;
        slabelement.slab.level = GetOffsetFromStory (otdslab.zBottom, otdslab.floorInd, storyLevels);
        if (otdslab.surface_geometry) {
            slabelement.slab.referencePlaneLocation = APISlabRefPlane_Top;
            slabelement.slab.offsetFromTop = 0;
            if (favorite_name.IsEmpty ()) {
                slabelement.slab.modelElemStructureType = API_BasicStructure;
                slabelement.slab.thickness = otd_thickness;
            }
            // Потолок уходит вниз от контакта, пол — вверх; level задаёт верх перекрытия.
            if (otdslab.type == Floor)
                slabelement.slab.level += slabelement.slab.thickness;
        }
    #ifdef ServerMainVers_2700
        API_AttributeIndex ematerial = ACAPI_CreateAttributeIndex (otdslab.material.material);
        slabelement.slab.topMat.value = ematerial;
        slabelement.slab.sideMat.value = ematerial;
        slabelement.slab.botMat.value = ematerial;
        slabelement.slab.sideMat.hasValue = true;
    #else
        slabelement.slab.topMat.attributeIndex = otdslab.material.material;
        slabelement.slab.sideMat.attributeIndex = otdslab.material.material;
        slabelement.slab.botMat.attributeIndex = otdslab.material.material;
        slabelement.slab.sideMat.overridden = true;
    #endif
        if (otdslab.type == Ceil) {
    #ifdef ServerMainVers_2700
            slabelement.slab.botMat.hasValue = true;
    #else
            slabelement.slab.botMat.overridden = true;
    #endif
        } else {
    #ifdef ServerMainVers_2700
            slabelement.slab.topMat.hasValue = true;
    #else
            slabelement.slab.topMat.overridden = true;
    #endif
        }
        err = ConvertPolygon2DToAPIPolygon (otdslab.poly, slabelement.slab.poly, memo);
        if (err != NoError) {
            return;
        }
        if (is_new) {
            err = ACAPI_Element_Create (&slabelement, &memo);
        } else {
            // Setting side materials and edge angles
            BMhKill ((GSHandle *)&memo.edgeTrims);
            BMhKill ((GSHandle *)&memo.edgeIDs);
            BMpFree (reinterpret_cast<GSPtr> (memo.sideMaterials));
            API_EdgeTrimID edgeType = APIEdgeTrim_Perpendicular;
            memo.edgeTrims = (API_EdgeTrim **)BMAllocateHandle (
                (slabelement.slab.poly.nCoords + 1) * sizeof (API_EdgeTrim), ALLOCATE_CLEAR, 0);
            memo.sideMaterials = (API_OverriddenAttribute *)BMAllocatePtr (
                (slabelement.slab.poly.nCoords + 1) * sizeof (API_OverriddenAttribute), ALLOCATE_CLEAR, 0);
            for (Int32 k = 1; k <= slabelement.slab.poly.nCoords; ++k) {
                memo.sideMaterials[k] = slabelement.slab.sideMat;
                (*(memo.edgeTrims))[k].sideType = edgeType;
                (*(memo.edgeTrims))[k].sideAngle = PI / 2;
            }
            API_Element mask = {};
            ACAPI_ELEMENT_MASK_SET (mask, API_SlabType, botMat);
            ACAPI_ELEMENT_MASK_SET (mask, API_SlabType, topMat);
            ACAPI_ELEMENT_MASK_SET (mask, API_SlabType, sideMat);
            ACAPI_ELEMENT_MASK_SET (mask, API_SlabType, poly.nSubPolys);
            ACAPI_ELEMENT_MASK_SET (mask, API_SlabType, poly.nCoords);
            ACAPI_ELEMENT_MASK_SET (mask, API_SlabType, poly.nArcs);
            ACAPI_ELEMENT_MASK_SET (mask, API_SlabType, level);
            if (otdslab.surface_geometry) {
                ACAPI_ELEMENT_MASK_SET (mask, API_Elem_Head, layer);
                ACAPI_ELEMENT_MASK_SET (mask, API_SlabType, referencePlaneLocation);
                ACAPI_ELEMENT_MASK_SET (mask, API_SlabType, offsetFromTop);
                ACAPI_ELEMENT_MASK_SET (mask, API_SlabType, thickness);
                ACAPI_ELEMENT_MASK_SET (mask, API_SlabType, modelElemStructureType);
            }
            ACAPI_ELEMENT_MASK_SET (mask, API_Elem_Head, floorInd);
            err = ACAPI_Element_Change (&slabelement,
                                        &mask,
                                        &memo,
                                        APIMemoMask_Polygon | APIMemoMask_SideMaterials | APIMemoMask_EdgeTrims,
                                        true);
        }
        if (err != NoError) {
            msg_rep ("OtdSlab_Draw_Slab", favorite_name, err, APINULLGuid);
            return;
        } else {
            otdslab.otd_guid = slabelement.header.guid;
        }
        return;
    }

    // Создаёт или обновляет объект пола/потолка, передавая контуры и параметры отделки в GDL.
    // При отказе чтения существующего элемента, memo или записи обнуляет otd_guid; при успехе сохраняет GUID.
    static void OtdSlab_Draw_Object (const GS::UniString &favorite_name, const Stories &storyLevels, OtdSlab &otdslab) {
        GSErrCode err = NoError;
        API_Element slabobjelement = {};
        API_ElementMemo slabobjmemo = {};
        const GS::OnExit memoGuard ([&slabobjmemo] () { ACAPI_DisposeElemMemoHdls (&slabobjmemo); });
        bool is_new = (otdslab.otd_guid == APINULLGuid); // Элемент уже существует, нужно только обновить
        if (is_new) {
            if (!OtdSlab_GetDefault_Object (favorite_name, slabobjelement, slabobjmemo))
                return;
        } else {
            slabobjelement.header.guid = otdslab.otd_guid;
            API_Guid savedGuid_Get = otdslab.otd_guid; // Сохраняем GUID для логирования перед обнулением
            err = ACAPI_Element_Get (&slabobjelement);
            if (err != NoError) {
                otdslab.otd_guid = APINULLGuid;
                msg_rep ("OtdSlab_Draw_Object", "ACAPI_Element_Get", err, savedGuid_Get);
                return;
            }
            API_Guid savedGuid_GetMemo = otdslab.otd_guid; // Сохраняем GUID для логирования перед обнулением
            err = ACAPI_Element_GetMemo (otdslab.otd_guid, &slabobjmemo);
            if (err != NoError) {
                otdslab.otd_guid = APINULLGuid;
                msg_rep ("OtdSlab_Draw_Object", "ACAPI_Element_GetMemo", err, savedGuid_GetMemo);
                return;
            }
        }
        GDLHelpers::ParamDict accsessoryparams;
        GDLHelpers::Param p;
        if (otdslab.type == Ceil) {
            // Высота уже задана положением объекта; GDL дополнительно прибавляет ac_ref_height.
            p.num = 0;
            accsessoryparams.Add ("{@gdl:ac_ref_height}", p);
            slabobjelement.object.level = GetOffsetFromStory (otdslab.zBottom, otdslab.floorInd, storyLevels);
        }
        if (otdslab.type == Floor) {
            p.num = otdslab.zBottom;
            accsessoryparams.Add ("{@gdl:ac_ref_height}", p);
            slabobjelement.object.level = GetOffsetFromStory (otdslab.zBottom, otdslab.floorInd, storyLevels);
        }
        p.str = otdslab.tip;
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
        p.num = otdslab.material.material;
        accsessoryparams.Add ("{@gdl:ceil_mat}", p);
        p.num = otdslab.material.material;
        accsessoryparams.Add ("{@gdl:matedge}", p);
        p.num = otdslab.material.material;
        accsessoryparams.Add ("{@gdl:matceipanel}", p);
        p.num = 0.02;
        accsessoryparams.Add ("{@gdl:ceil_thk}", p);
        p.num = 0.02;
        accsessoryparams.Add ("{@gdl:ac_thickness}", p);
        p.num = 0;
        accsessoryparams.Add ("{@gdl:ac_slab_side}", p);
        Geometry::Polygon2DData polygon2DData;
        Geometry::InitPolygon2DData (&polygon2DData);
        Geometry::ConvertPolygon2DToPolygon2DData (polygon2DData, otdslab.poly);
        Int32 nCoords = polygon2DData.nVertices;
        Int32 nSubPolys = polygon2DData.nContours;
        Box2DData bbox = otdslab.poly.GetBoundBox ();
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
            if (otdslab.type == Ceil) {
                p.num = bbox.xMax - bbox.xMin - 0.1;
                accsessoryparams.Add ("{@gdl:pos_x2}", p);
                p.num = bbox.yMax - bbox.yMin - 0.1;
                accsessoryparams.Add ("{@gdl:pos_y2}", p);

                p.num = (bbox.xMax - bbox.xMin - 0.1) / 2;
                accsessoryparams.Add ("{@gdl:pos_xtxt}", p);
                p.num = (bbox.yMax - bbox.yMin - 0.1) / 2;
                accsessoryparams.Add ("{@gdl:pos_ytxt}", p);
            }
            if (otdslab.type == Floor) {
                p.num = otdslab.poly.GetCenter ().x - bbox.xMin;
                accsessoryparams.Add ("{@gdl:pos_x1}", p);
                p.num = otdslab.poly.GetCenter ().y - bbox.yMin;
                accsessoryparams.Add ("{@gdl:pos_y1}", p);
            }
            p.dim1 = 1;
            p.dim2 = 2;
            p.arr_num.Push (otdslab.poly.GetCenter ().x - bbox.xMin);
            p.arr_num.Push (otdslab.poly.GetCenter ().y - bbox.yMin);
            accsessoryparams.Add ("{@gdl:fieldorig}", p);
        }
        Geometry::FreePolygon2DData (&polygon2DData);
        GDLHelpers::ParamToMemo (slabobjmemo, accsessoryparams);
        slabobjelement.header.floorInd = otdslab.floorInd;
        slabobjelement.object.pos.x = bbox.xMin;
        slabobjelement.object.pos.y = bbox.yMin;
        double y_start = 0;
        if (is_new) {
            err = ACAPI_Element_Create (&slabobjelement, &slabobjmemo);
        } else {
            API_Element mask = {};
            ACAPI_ELEMENT_MASK_CLEAR (mask);
            ACAPI_ELEMENT_MASK_SET (mask, API_ObjectType, pos);
            ACAPI_ELEMENT_MASK_SET (mask, API_ObjectType, level);
            ACAPI_ELEMENT_MASK_SET (mask, API_Elem_Head, floorInd);
            err = ACAPI_Element_Change (&slabobjelement, &mask, &slabobjmemo, APIMemoMask_AddPars, true);
        }
        if (err != NoError) {
            otdslab.otd_guid = APINULLGuid;
            msg_rep ("OtdSlab_Draw_Object", favorite_name, err, APINULLGuid);
            return;
        } else {
            otdslab.otd_guid = slabobjelement.header.guid;
        }
    }

    // Возвращает объект из избранного подходящего типа либо получает настройки объекта по умолчанию.
    // Только для настроек по умолчанию сбрасывает угол и автовидимость по этажам, включает перья, линии и материалы
    // объекта.
    static bool OtdSlab_GetDefault_Object (const GS::UniString &favorite_name,
                                           API_Element &slabobjelement,
                                           API_ElementMemo &slabobjmemo) {
        GSErrCode err = NoError;
        BNZeroMemory (&slabobjmemo, sizeof (API_ElementMemo));
        BNZeroMemory (&slabobjelement, sizeof (API_Element));
        if (Favorite_GetByName (favorite_name, slabobjelement, slabobjmemo)) {
            if (GetElemTypeID (slabobjelement) == API_ObjectID)
                return true;
        }
        SetElemTypeID (slabobjelement, API_ObjectID);
        err = ACAPI_Element_GetDefaults (&slabobjelement, &slabobjmemo);
        if (err != NoError) {
            ACAPI_DisposeElemMemoHdls (&slabobjmemo);
            return false;
        }
        slabobjelement.object.angle = 0;
        slabobjelement.object.isAutoOnStoryVisibility = false;
        slabobjelement.object.useObjPens = true;
        slabobjelement.object.useObjLtypes = true;
        slabobjelement.object.useObjMaterials = true;
        return true;
    }

    // Возвращает перекрытие из избранного подходящего типа либо получает настройки перекрытия по умолчанию.
    // Только для настроек по умолчанию задаёт нулевой offsetFromTop, связанные материалы и базовую структуру.
    static bool OtdSlab_GetDefault_Slab (const GS::UniString &favorite_name, API_Element &slabelement) {
        GSErrCode err = NoError;
        BNZeroMemory (&slabelement, sizeof (API_Element));
        if (Favorite_GetByName (favorite_name, slabelement)) {
            if (GetElemTypeID (slabelement) == API_SlabID)
                return true;
        }
        SetElemTypeID (slabelement, API_SlabID);
        err = ACAPI_Element_GetDefaults (&slabelement, nullptr);
        if (err != NoError) {
            return false;
        }
        slabelement.slab.offsetFromTop = 0;
        slabelement.slab.materialsChained = true;
    #ifdef ServerMainVers_2600
    #else
        slabelement.slab.sideMat.overridden = true;
        slabelement.slab.topMat.overridden = true;
        slabelement.slab.botMat.overridden = true;
    #endif
        slabelement.slab.modelElemStructureType = API_BasicStructure;
        return true;
    }

    // -----------------------------------------------------------------------------
    // Назначение класса стене
    // -----------------------------------------------------------------------------
    static void OtdWall_SetClass (const OtdWall &op, const ClassificationFunc::ClassificationDict &finclass) {
        if (op.otd_guid == APINULLGuid)
            return;
        API_Guid class_guid = Class_GetClassGuid (op.type, finclass);
        if (class_guid != APINULLGuid)
            ACAPI_Element_AddClassificationItem (op.otd_guid, class_guid);
    }

    // -----------------------------------------------------------------------------
    // Назначение класса проёму
    // -----------------------------------------------------------------------------
    // Для ненулевого GUID проёма выбирает класс NoSet, затем при его отсутствии класс Wall_Main.
    // Вызывает добавление классификации только при ненулевом GUID выбранного класса.
    static void OtdOpening_SetClass (const OtdOpening &op, const ClassificationFunc::ClassificationDict &finclass) {
        if (op.otd_guid == APINULLGuid)
            return;
        API_Guid class_guid = Class_GetClassGuid (NoSet, finclass);
        if (class_guid == APINULLGuid)
            class_guid = Class_GetClassGuid (Wall_Main, finclass);
        if (class_guid != APINULLGuid)
            ACAPI_Element_AddClassificationItem (op.otd_guid, class_guid);
    }

    // -----------------------------------------------------------------------------
    // Назначение класса полу/потолку
    // -----------------------------------------------------------------------------
    static void OtdSlab_SetClass (const OtdSlab &op, const ClassificationFunc::ClassificationDict &finclass) {
        if (op.otd_guid == APINULLGuid)
            return;
        API_Guid class_guid = Class_GetClassGuid (op.type, finclass);
        if (class_guid != APINULLGuid)
            ACAPI_Element_AddClassificationItem (op.otd_guid, class_guid);
    }

} // namespace Roombook
#endif
