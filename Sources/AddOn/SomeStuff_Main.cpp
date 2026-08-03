//------------ kuvbur 2022 ------------
#include <APIdefs_Properties.h>
#include <DGModule.hpp>
#include <stdio.h>
#include <UniString.hpp>

#include "ACAPinc.h"

#include "api_headers/APIEnvir.h"

#include "SomeStuff_Main.hpp"

#include "dialogs/BrowserPalette.hpp"
#include "Dimensions.hpp"
#include "json_commands/JsonCommandRegistrar.hpp"
#include "pk/Revision.hpp"
#include "Propertycache.hpp"
#include "Roombook.hpp"
#include "spec/Spec.hpp"
#include "Summ.hpp"
#include "Sync.hpp"
#ifdef ServerMainVers_2300
    #include "pk/AutomateFunction.hpp"
    #include "ReNum.hpp"
#endif
#ifdef ServerMainVers_2700
    #include "MEPv1.hpp"
#endif // AC_27
#ifdef TESTING
    #include "TestFunc.hpp"
#endif

//-----------------------------------------------------------------------------
// Срабатывает при событиях в тимворк
//-----------------------------------------------------------------------------
#ifdef ServerMainVers_2800
static GSErrCode ReservationChangeHandler (const GS::HashTable<API_Guid, short> &reserved,
                                           const GS::HashSet<API_Guid> &released,
                                           const GS::HashSet<API_Guid> &deleted) {
#else
static GSErrCode __ACENV_CALL ReservationChangeHandler (const GS::HashTable<API_Guid, short> &reserved,
                                                        const GS::HashSet<API_Guid> &released,
                                                        const GS::HashSet<API_Guid> &deleted) {
#endif
    (void)deleted;
    (void)released;
#ifdef TESTING
    DBprnt ("ReservationChangeHandler");
#endif
    SyncSettings syncSettings;
    LoadSyncSettingsFromPreferences (syncSettings);
    for (GS::HashTable<API_Guid, short>::ConstPairIterator it = reserved.EnumeratePairs (); it != nullptr; ++it) {
#ifdef ServerMainVers_2800
        AttachObserver ((it->key), syncSettings);
#else
        AttachObserver (*(it->key), syncSettings);
#endif
    }
    PROPERTYCACHE ().Update ();
    DimRoundAll (syncSettings, false);
    ACAPI_KeepInMemory (true);
    return NoError;
}

// -----------------------------------------------------------------------------
// Срабатывает при событиях проекта: открытие, закрытие, смена окна или этажа.
// Здесь обновляется состояние меню, мониторинг и кэш свойств.
// -----------------------------------------------------------------------------
#ifdef ServerMainVers_2800
static GSErrCode ProjectEventHandlerProc (API_NotifyEventID notifID, Int32 param) {
#else
static GSErrCode __ACENV_CALL ProjectEventHandlerProc (API_NotifyEventID notifID, Int32 param) {
#endif
#ifdef TESTING
    DBprnt ("ProjectEventHandlerProc");
#endif
    SyncSettings syncSettings;
    LoadSyncSettingsFromPreferences (syncSettings, true);
    MenuSetState (syncSettings);
    GSErrCode err = NoError;
    switch (notifID) {
    case APINotify_New:
    case APINotify_NewAndReset:
    case APINotify_Open:
        Do_ElementMonitor (syncSettings.GetSyncMon());
        PROPERTYCACHE ().Update ();
        break;
    case APINotify_Close:
    case APINotify_Quit:
#ifdef ServerMainVers_2700
        ACAPI_Element_CatchNewElement (nullptr, nullptr);
        ACAPI_Element_InstallElementObserver (nullptr);
#else
        ACAPI_Notify_CatchNewElement (nullptr, nullptr);
        ACAPI_Notify_InstallElementObserver (nullptr);
#endif
        break;
    case APINotify_ChangeProjectDB:
    case APINotify_ChangeWindow:
    case APINotify_ChangeFloor:
        DimRoundAll (syncSettings, false);
        break;
    default:
        break;
    }
    (void)param;
    ACAPI_KeepInMemory (true);
    return NoError;
} // ProjectEventHandlerProc

// -----------------------------------------------------------------------------
// Срабатывает при изменении элемента
// -----------------------------------------------------------------------------
#ifdef ServerMainVers_2800
GSErrCode ElementEventHandlerProc (const API_NotifyElementType *elemType) {
#else
GSErrCode __ACENV_CALL ElementEventHandlerProc (const API_NotifyElementType *elemType) {
#endif
    // Элементы из hotlink не обрабатываются, потому что они приходят как внешние ссылки и не доступны для локального
    // редактирования.
    if (elemType->elemHead.hotlinkGuid != APINULLGuid)
        return NoError;
    ACAPI_KeepInMemory (true);
    SyncSettings syncSettings;
    LoadSyncSettingsFromPreferences (syncSettings);
    int dummymode = DUMMY_MODE_UNDEF;
    if (!syncSettings.GetSyncMon())
        return NoError;
    if (elemType->notifID == APINotifyElement_BeginEvents) {
        PROPERTYCACHE ().compositeCache.Clear ();
        return NoError;
    } else if (elemType->notifID == APINotifyElement_EndEvents) {
        if (!IsElementThrottled (APINULLGuid)) {
            DimRoundAll (syncSettings, true);
        }
        return NoError;
    }
    // Смотрим - что поменялось
    API_ElemTypeID elementType = GetElemTypeID (elemType->elemHead);
    switch (elementType) {
    case API_ZombieElemID:
    case API_GroupID:
    case API_DimensionID:
        if (PROPERTYCACHE ().hasDimAutotext) {
            if (elemType->notifID == APINotifyElement_New)
                AttachObserver (elemType->elemHead.guid, syncSettings);
            DimAutoRoundOne (elemType->elemHead.guid, syncSettings, false);
        }
        return NoError;
#ifdef ServerMainVers_2800
    case API_ExternalElemID:
        MEPv1::ClearRoutingSubelemCache ();
        break;
#endif
    default:
        break;
    }
#if defined(TESTING)
    DBprnt ("ElementEventHandlerProc start");
#endif
    ParamDictElement paramToWrite = {};
    if (IsElementThrottled (elemType->elemHead.guid))
        return NoError;
    if (!IsElementEditable (elemType->elemHead.guid, syncSettings, true, elementType))
        return NoError;
    bool needresync = false;
    switch (elemType->notifID) {
    case APINotifyElement_New:
        AttachObserver (elemType->elemHead.guid, syncSettings);
    case APINotifyElement_Change:
    case APINotifyElement_PropertyValueChange:
    case APINotifyElement_Edit:
    case APINotifyElement_ClassificationChange:
        dummymode = IsDummyModeOn ();
        // В dummy-режиме важно принудительно включить мониторинг и основные типы синхронизации,
        // иначе обработка остановится на раннем шаге и не дойдёт до нужных подэлементов.
        if (dummymode == DUMMY_MODE_ON) {
            syncSettings.SetSyncMon(true);
            syncSettings.SetWallS(true);
            syncSettings.SetWidoS(true);
            syncSettings.SetObjS(true);
        }
        // После изменения самой навесной стены панели не обрабатываются отдельно,
        // потому что их синхронизация будет выполнена дальше через SyncElement.
        if (syncSettings.GetLogMon() && elementType != API_CurtainWallPanelID && elementType != API_CurtainWallSegmentID &&
            elementType != API_CurtainWallFrameID && elementType != API_CurtainWallJunctionID &&
            elementType != API_CurtainWallAccessoryID) {
            syncSettings.SetLogMon(false);
            WriteSyncSettingsToPreferences (syncSettings);
        }
        if (syncSettings.GetLogMon()) {
            syncSettings.SetCwallS(false);
        }
        if (!syncSettings.GetLogMon() && elementType == API_CurtainWallID) {
            syncSettings.SetLogMon(true);
            WriteSyncSettingsToPreferences (syncSettings);
        }
        needresync = SyncElement (elemType->elemHead.guid, syncSettings, paramToWrite, dummymode);
        if (!paramToWrite.IsEmpty ()) {
            GS::Array<API_Guid> rereadelem = {};
            rereadelem = ParamHelpers::ElementsWrite (paramToWrite);
            // После первой синхронизации часть элементов может потребовать повторного перечитывания,
            // поэтому выполняем второй проход по той же сущности и собираем дополнительно изменённые GUID.
            if (needresync) {
                paramToWrite.Clear ();
                needresync = SyncElement (elemType->elemHead.guid, syncSettings, paramToWrite, dummymode);
                GS::Array<API_Guid> rereadelem_ = {};
                rereadelem_ = ParamHelpers::ElementsWrite (paramToWrite);
                if (!rereadelem_.IsEmpty ())
                    rereadelem.Append (rereadelem_);
            }
            if (!rereadelem.IsEmpty ()) {
#if defined(TESTING)
                DBprnt ("ElementEventHandlerProc", "reread element");
#endif
                for (UInt32 i = 0; i < rereadelem.GetSize (); i++) {
                    paramToWrite.Clear ();
                    needresync = SyncElement (rereadelem[i], syncSettings, paramToWrite, dummymode);
                    ParamHelpers::ElementsWrite (paramToWrite);
                }
            }
            ParamHelpers::WriteInfo (paramToWrite);
        }
        break;
    default:
        break;
    }
#if defined(TESTING)
    DBprnt ("ElementEventHandlerProc end");
#endif
    return NoError;
} // ElementEventHandlerProc

// -----------------------------------------------------------------------------
// Включает или отключает наблюдение за изменениями элементов.
// При включении регистрируются observers, при выключении — снимаются.
// -----------------------------------------------------------------------------
void Do_ElementMonitor (bool syncMon) {
    bool isteamwork = false;
    short userid = 0;
    GSErrCode err = IsTeamwork (isteamwork, userid);
    if (syncMon) {
#if defined(TESTING)
        DBprnt ("Do_ElementMonitor on");
#endif
#ifdef ServerMainVers_2700
        ACAPI_Element_CatchNewElement (nullptr, ElementEventHandlerProc);
        ACAPI_Element_InstallElementObserver (ElementEventHandlerProc);
        if (isteamwork)
            ACAPI_Notification_CatchElementReservationChange (ReservationChangeHandler);
#else
        ACAPI_Notify_CatchNewElement (nullptr, ElementEventHandlerProc);
        ACAPI_Notify_InstallElementObserver (ElementEventHandlerProc);
        if (isteamwork)
            ACAPI_Notify_CatchElementReservationChange (ReservationChangeHandler);
#endif
    }
    if (!syncMon) {
#if defined(TESTING)
        DBprnt ("Do_ElementMonitor off");
#endif
#ifdef ServerMainVers_2700
        ACAPI_Element_CatchNewElement (nullptr, nullptr);
        ACAPI_Element_InstallElementObserver (nullptr);
        if (isteamwork)
            ACAPI_Notification_CatchElementReservationChange (nullptr);
#else
        ACAPI_Notify_CatchNewElement (nullptr, nullptr);
        ACAPI_Notify_InstallElementObserver (nullptr);
        if (isteamwork)
            ACAPI_Notify_CatchElementReservationChange (nullptr);
#endif
    }
    return;
} // Do_ElementMonitor

// -----------------------------------------------------------------------------
// Синхронизирует состояние пунктов меню с текущими настройками синхронизации.
// -----------------------------------------------------------------------------
void MenuSetState (SyncSettings &syncSettings) {
    MenuItemCheckAC (Menu_MonAll, syncSettings.GetSyncMon());
    MenuItemCheckAC (Menu_wallS, syncSettings.GetWallS());
    MenuItemCheckAC (Menu_widoS, syncSettings.GetWidoS());
    MenuItemCheckAC (Menu_objS, syncSettings.GetObjS());
    MenuItemCheckAC (Menu_cwallS, syncSettings.GetCwallS());
    MenuItemCheckAC (Menu_Pallete, syncSettings.GetShowPalette());
    if (!isEng ())
        return;
    for (UInt32 i = 0; i < MENU_ITEM_COUNT; i++) {
        SetPaletteMenuText (i);
    }
}

void SetPaletteMenuText (short paletteItemInd) {
    API_MenuItemRef itemRef = {};
    GS::UniString itemStr = "";
    const Int32 bisEng = ID_ADDON_PROMT + isEng ();
    itemStr = RSGetIndString (bisEng, paletteItemInd + 1, ACAPI_GetOwnResModule ());
    itemRef.menuResID = ID_ADDON_MENU;
    itemRef.itemIndex = paletteItemInd;
#ifdef ServerMainVers_2700
    ACAPI_MenuItem_SetMenuItemText (&itemRef, nullptr, &itemStr);
#else
    ACAPI_Interface (APIIo_SetMenuItemTextID, &itemRef, nullptr, &itemStr);
#endif
    return;
}

static GSErrCode MenuCommandHandler (const API_MenuParams *menuParams) {
    GSErrCode err = NoError;
#if defined(TESTING)
    DBprnt ("MenuCommandHandler start");
#endif
    SyncSettings syncSettings;
    LoadSyncSettingsFromPreferences (syncSettings, true);
#ifdef ServerMainVers_2700
    ACAPI_UserInput_ClearElementHighlight ();
#else
    #ifdef ServerMainVers_2600
    ACAPI_Interface_ClearElementHighlight ();
    #else
    ACAPI_Interface (APIIo_HighlightElementsID);
    #endif
#endif
    const Int32 AddOnMenuID = ID_ADDON_MENU;
    PROPERTYCACHE ().Update ();
    // Все команды add-on приходят через один обработчик меню, поэтому здесь
    // выполняется маршрутизация по ID пункта меню.
    switch (menuParams->menuItemRef.menuResID) {
    case AddOnMenuID:
        switch (menuParams->menuItemRef.itemIndex) {
        case MonAll_CommandID:
            syncSettings.SetSyncAll(false);
            syncSettings.SetSyncMon(!syncSettings.GetSyncMon());
            Do_ElementMonitor (syncSettings.GetSyncMon());
            MonAll (syncSettings);
            break;
        case SyncAll_CommandID:
            msg_rep ("SyncAll", "============== START ==============", NoError, APINULLGuid);
            syncSettings.SetSyncAll(true);
            SyncAndMonAll (syncSettings);
            syncSettings.SetSyncAll(false);
            msg_rep ("SyncAll", "=============== END ===============", NoError, APINULLGuid);
            break;
        case SyncSelect_CommandID:
            SyncSelected (syncSettings);
            break;
        case wallS_CommandID:
            syncSettings.SetWallS(!syncSettings.GetWallS());
            break;
        case widoS_CommandID:
            syncSettings.SetWidoS(!syncSettings.GetWidoS());
            break;
        case objS_CommandID:
            syncSettings.SetObjS(!syncSettings.GetObjS());
            break;
        case cwallS_CommandID:
            syncSettings.SetCwallS(!syncSettings.GetCwallS());
            break;
#ifdef ServerMainVers_2300
        case ReNum_CommandID:
            msg_rep ("ReNumSelected", "============== START ==============", NoError, APINULLGuid);
            err = ReNumSelected (syncSettings);
            msg_rep ("ReNumSelected", "=============== END ===============", NoError, APINULLGuid);
            break;
#endif
        case Sum_CommandID:
            msg_rep ("SumSelected", "============== START ==============", NoError, APINULLGuid);
            err = SumSelected (syncSettings);
            msg_rep ("SumSelected", "=============== END ===============", NoError, APINULLGuid);
            break;
        case RunParam_CommandID:
            RunParamSelected (syncSettings);
            break;
        case Spec_CommandID:
            msg_rep ("Spec", "============== START ==============", NoError, APINULLGuid);
            err = Spec::SpecAll (syncSettings);
            msg_rep ("Spec", "=============== END ===============", NoError, APINULLGuid);
            break;
        case ShowSub_CommandID:
            SyncShowSubelement (syncSettings);
            break;
        case SetRevision_CommandID:
            msg_rep ("Revision", "============== START ==============", NoError, APINULLGuid);
            Revision::SetRevision ();
            msg_rep ("Revision", "=============== END ===============", NoError, APINULLGuid);
            break;
        case SetSub_CommandID:
            SyncSetSubelement (syncSettings);
            break;
        case RoomBook_CommandID:
            msg_rep ("RoomBook", "============== START ==============", NoError, APINULLGuid);
            Roombook::RoomBook ();
            msg_rep ("RoomBook", "=============== END ===============", NoError, APINULLGuid);
            break;
#ifdef ServerMainVers_2300
        case Auto3D_CommandID:
            AutoFunc::ProfileByLine ();
            break;
        case AutoLay_CommandID:
            AutoFunc::AlignDrawingsByPoints ();
            break;
#endif
        case Pallete_CommandID:
            ShowOrHideBrowserPalette ();
            break;
        }
        break;
    }
    (void)err;
    DimRoundAll (syncSettings, false);
    WriteSyncSettingsToPreferences (syncSettings);
    MenuSetState (syncSettings);
    ACAPI_KeepInMemory (true);
#ifdef TESTING
    DBprnt ("MenuCommandHandler end");
#endif
    return NoError;
}

#ifdef ServerMainVers_2800
API_AddonType CheckEnvironment (API_EnvirParams *envir) {
#else
API_AddonType __ACDLL_CALL CheckEnvironment (API_EnvirParams *envir) {
#endif
#ifdef TESTING
    DBprnt ("CheckEnvironment");
    TestFunc::Test ();
#endif
    RSGetIndString (&envir->addOnInfo.name, ID_ADDON_INFO + isEng (), AddOnNameID, ACAPI_GetOwnResModule ());
    RSGetIndString (
        &envir->addOnInfo.description, ID_ADDON_INFO + isEng (), AddOnDescriptionID, ACAPI_GetOwnResModule ());
    ACAPI_KeepInMemory (true);
    return APIAddon_Preload;
}
#ifdef ServerMainVers_2800
GSErrCode RegisterInterface (void) {
#else
GSErrCode __ACDLL_CALL RegisterInterface (void) {
#endif
#if defined(TESTING)
    DBprnt ("RegisterInterface");
#endif
    GSErrCode err = NoError;
#ifdef ServerMainVers_2700
    err = ACAPI_MenuItem_RegisterMenu (ID_ADDON_MENU, ID_ADDON_PROMT + isEng (), MenuCode_Tools, MenuFlag_Default);
#else
    err = ACAPI_Register_Menu (ID_ADDON_MENU, ID_ADDON_PROMT + isEng (), MenuCode_Tools, MenuFlag_Default);
#endif
    ACAPI_KeepInMemory (true);
    return err;
}
#ifdef ServerMainVers_2800
GSErrCode Initialize (void) {
#else
GSErrCode __ACENV_CALL Initialize (void) {
#endif
#if defined(TESTING)
    DBprnt ("Initialize");
#endif
    SyncSettings syncSettings;
    LoadSyncSettingsFromPreferences (syncSettings, true);
    MenuSetState (syncSettings);
    Do_ElementMonitor (syncSettings.GetSyncMon());
    MonAll (syncSettings);
#ifdef ServerMainVers_2700
    ACAPI_ProjectOperation_CatchProjectEvent (APINotify_ChangeWindow | APINotify_ChangeFloor | APINotify_New |
                                                  APINotify_NewAndReset | APINotify_Open | APINotify_Close |
                                                  APINotify_Quit | APINotify_ChangeProjectDB,
                                              ProjectEventHandlerProc);
#else
    ACAPI_Notify_CatchProjectEvent (APINotify_ChangeWindow | APINotify_ChangeFloor | APINotify_New |
                                        APINotify_NewAndReset | APINotify_Open | APINotify_Close | APINotify_Quit |
                                        APINotify_ChangeProjectDB,
                                    ProjectEventHandlerProc);
#endif

    // Регистрация JSON команд
    RegisterJsonCommands ();
    // Регистрация BrowserPalette
    BrowserPalette::RegisterPaletteControlCallBack ();
    ACAPI_KeepInMemory (true);
#ifdef ServerMainVers_2700
    return ACAPI_MenuItem_InstallMenuHandler (ID_ADDON_MENU, MenuCommandHandler);
#else
    return ACAPI_Install_MenuHandler (ID_ADDON_MENU, MenuCommandHandler);
#endif
}
#ifdef ServerMainVers_2800
GSErrCode FreeData (void) {
#else
GSErrCode __ACENV_CALL FreeData (void) {
#endif
#if defined(TESTING)
    DBprnt ("!!!!!FreeData");
#endif
    return NoError;
}
