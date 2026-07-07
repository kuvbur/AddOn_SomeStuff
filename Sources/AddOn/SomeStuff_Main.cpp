//------------ kuvbur 2022 ------------
#include	"ACAPinc.h"
#include	"APIEnvir.h"
#include	<stdio.h>
#ifdef TESTING
#include "TestFunc.hpp"
#endif
#include	"DGModule.hpp"
#include	"UniString.hpp"
#include	"APIdefs_Properties.h"
#include	"SomeStuff_Main.hpp"
#include	"Sync.hpp"
#ifndef AC_22
#include	"AutomateFunction.hpp"
#include	"ReNum.hpp"
#endif
#include	"Roombook.hpp"
#include	"Revision.hpp"
#include	"Summ.hpp"
#include	"Dimensions.hpp"
#include	"Spec.hpp"
#include	"Propertycache.hpp"
#if defined(AC_27) || defined (AC_28) || defined(AC_29)
#include	"MEPv1.hpp"
#endif // AC_27

//-----------------------------------------------------------------------------
// Срабатывает при событиях в тимворк
//-----------------------------------------------------------------------------
#if defined(AC_28) || defined(AC_29)
static GSErrCode ReservationChangeHandler (const GS::HashTable<API_Guid, short>& reserved,
                                           const GS::HashSet<API_Guid>& released,
                                           const GS::HashSet<API_Guid>& deleted)
{
    #else
static GSErrCode __ACENV_CALL	ReservationChangeHandler (const GS::HashTable<API_Guid, short>&reserved,
                                                          const GS::HashSet<API_Guid>&released,
                                                          const GS::HashSet<API_Guid>&deleted)
{
    #endif
    (void) deleted;
    (void) released;
    #ifdef TESTING
    DBprnt ("ReservationChangeHandler");
    #endif
    SyncSettings syncSettings (false, false, true, true, true, true, false);
    LoadSyncSettingsFromPreferences (syncSettings);
    #ifdef EXTNDVERSION
    syncSettings.syncMon = true;
    #endif // PK_1
    for (GS::HashTable<API_Guid, short>::ConstPairIterator it = reserved.EnumeratePairs (); it != nullptr; ++it) {
        #if defined(AC_28) || defined(AC_29)
        AttachObserver ((it->key), syncSettings);
        #else
        AttachObserver (*(it->key), syncSettings);
        #endif
    }
    ACAPI_KeepInMemory (true);
    return NoError;
}

// -----------------------------------------------------------------------------
// Срабатывает при событиях проекта (открытие, сохранение)
// -----------------------------------------------------------------------------
#if defined(AC_28) || defined(AC_29)
static GSErrCode ProjectEventHandlerProc (API_NotifyEventID notifID, Int32 param)
{
    #else
static GSErrCode __ACENV_CALL    ProjectEventHandlerProc (API_NotifyEventID notifID, Int32 param)
{
    #endif
    #ifdef TESTING
    DBprnt ("ProjectEventHandlerProc");
    #endif
    SyncSettings syncSettings (false, false, true, true, true, true, false);
    LoadSyncSettingsFromPreferences (syncSettings, true);
    #ifdef EXTNDVERSION
    syncSettings.syncMon = true;
    #endif // PK_1
    MenuSetState (syncSettings);
    GSErrCode err = NoError;
    switch (notifID) {
        case APINotify_New:
        case APINotify_NewAndReset:
        case APINotify_Open:
            Do_ElementMonitor (syncSettings.syncMon);
            PROPERTYCACHE ().Update ();
            break;
        case APINotify_Close:
        case APINotify_Quit:
            #if defined(AC_27) || defined(AC_28) || defined(AC_29)
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
    (void) param;
    ACAPI_KeepInMemory (true);
    return NoError;
}	// ProjectEventHandlerProc

// -----------------------------------------------------------------------------
// Срабатывает при изменении элемента
// -----------------------------------------------------------------------------
#if defined(AC_28) || defined(AC_29)
GSErrCode ElementEventHandlerProc (const API_NotifyElementType * elemType)
{
    #else
GSErrCode __ACENV_CALL	ElementEventHandlerProc (const API_NotifyElementType * elemType)
{
    #endif
    //#if defined(TESTING)
    //GS::UniString tst = APIGuid2GSGuid (elemType->elemHead.guid).ToUniString ();
    //tst.Append (SPACESTRING);
    //switch (elemType->notifID) {
    //    case APINotifyElement_BeginEvents:
    //        tst.Append ("APINotifyElement_BeginEvents");
    //        break;
    //    case APINotifyElement_EndEvents:
    //        tst.Append ("APINotifyElement_EndEvents");
    //        break;
    //    case APINotifyElement_New:
    //        tst.Append ("APINotifyElement_New");
    //        break;
    //    case APINotifyElement_Copy:
    //        tst.Append ("APINotifyElement_Copy");
    //        break;
    //    case APINotifyElement_Change:
    //        tst.Append ("APINotifyElement_Change");
    //        break;
    //    case APINotifyElement_Edit:
    //        tst.Append ("APINotifyElement_Edit");
    //        break;
    //    case APINotifyElement_Delete:
    //        tst.Append ("APINotifyElement_Delete");
    //        break;
    //    case APINotifyElement_Undo_Created:
    //        tst.Append ("APINotifyElement_Undo_Created");
    //        break;
    //    case APINotifyElement_Undo_Modified:
    //        tst.Append ("APINotifyElement_Undo_Modified");
    //        break;
    //    case APINotifyElement_Undo_Deleted:
    //        tst.Append ("APINotifyElement_Undo_Deleted");
    //        break;
    //    case APINotifyElement_Redo_Created:
    //        tst.Append ("APINotifyElement_Redo_Created");
    //        break;
    //    case APINotifyElement_Redo_Modified:
    //        tst.Append ("APINotifyElement_Redo_Modified");
    //        break;
    //    case APINotifyElement_Redo_Deleted:
    //        tst.Append ("APINotifyElement_Redo_Deleted");
    //        break;
    //    case APINotifyElement_PropertyValueChange:
    //        tst.Append ("APINotifyElement_PropertyValueChange");
    //        break;
    //    case APINotifyElement_ClassificationChange:
    //        tst.Append ("APINotifyElement_ClassificationChange");
    //        break;
    //    default:
    //        break;
    //}
    //tst.Append (SPACESTRING);
    //DBprnt ("ElementEvent", tst);
    //#endif
    if (elemType->elemHead.hotlinkGuid != APINULLGuid) return NoError;
    ACAPI_KeepInMemory (true);
    SyncSettings syncSettings (false, false, true, true, true, true, false);
    LoadSyncSettingsFromPreferences (syncSettings);
    int dummymode = DUMMY_MODE_UNDEF;
    #ifdef EXTNDVERSION
    syncSettings.syncMon = true;
    #endif // PK_1
    if (!syncSettings.syncMon) return NoError;
    if (elemType->notifID == APINotifyElement_BeginEvents) {
        PROPERTYCACHE ().compositeCache.Clear ();
        return NoError;
    }
    // Смотрим - что поменялось
    API_ElemTypeID elementType = GetElemTypeID (elemType->elemHead);
    switch (elementType) {
        case API_ZombieElemID:
        case API_GroupID:
        case API_DimensionID:
            if (PROPERTYCACHE ().hasDimAutotext) {
                if (elemType->notifID == APINotifyElement_New) AttachObserver (elemType->elemHead.guid, syncSettings);
                DimAutoRoundOne (elemType->elemHead.guid, syncSettings, false);
            }
            return NoError;
            #if defined (AC_28) || defined (AC_29)
        case API_ExternalElemID:
            MEPv1::ClearRoutingSubelemCache ();
            break;
            #endif
        default:
            if (elemType->notifID == APINotifyElement_EndEvents) {
                if (!IsElementThrottled (APINULLGuid)) {
                    DimRoundAll (syncSettings, true);
                }
            }
            break;
    }
    #if defined(TESTING)
    DBprnt ("ElementEventHandlerProc start");
    #endif
    ParamDictElement paramToWrite = {};
    if (IsElementThrottled (elemType->elemHead.guid)) return NoError;
    if (!IsElementEditable (elemType->elemHead.guid, syncSettings, true, elementType)) return NoError;
    bool needresync = false;
    switch (elemType->notifID) {
        case APINotifyElement_New:
            AttachObserver (elemType->elemHead.guid, syncSettings);
        case APINotifyElement_Change:
        case APINotifyElement_PropertyValueChange:
        case APINotifyElement_Edit:
        case APINotifyElement_ClassificationChange:
            dummymode = IsDummyModeOn ();
            if (dummymode == DUMMY_MODE_ON) {
                syncSettings.syncMon = true;
                syncSettings.wallS = true;
                syncSettings.widoS = true;
                syncSettings.objS = true;
            }
            // Отключение обработки панелей навесных стен после изменения самой навесной стены
            // Панели навесных стен обрабатываются далее, в функции SyncElement
            if (syncSettings.logMon && elementType != API_CurtainWallPanelID && elementType != API_CurtainWallSegmentID && elementType != API_CurtainWallFrameID && elementType != API_CurtainWallJunctionID && elementType != API_CurtainWallAccessoryID) {
                syncSettings.logMon = false;
                WriteSyncSettingsToPreferences (syncSettings);
            }
            if (syncSettings.logMon) {
                syncSettings.cwallS = false;
            }
            if (!syncSettings.logMon && elementType == API_CurtainWallID) {
                syncSettings.logMon = true;
                WriteSyncSettingsToPreferences (syncSettings);
            }
            needresync = SyncElement (elemType->elemHead.guid, syncSettings, paramToWrite, dummymode);
            if (!paramToWrite.IsEmpty ()) {
                GS::Array<API_Guid> rereadelem = {};
                rereadelem = ParamHelpers::ElementsWrite (paramToWrite);
                if (needresync) {
                    paramToWrite.Clear ();
                    needresync = SyncElement (elemType->elemHead.guid, syncSettings, paramToWrite, dummymode);
                    GS::Array<API_Guid> rereadelem_ = {};
                    rereadelem_ = ParamHelpers::ElementsWrite (paramToWrite);
                    if (!rereadelem_.IsEmpty ()) rereadelem.Append (rereadelem_);
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
}	// ElementEventHandlerProc

// -----------------------------------------------------------------------------
// Включение мониторинга
// -----------------------------------------------------------------------------
void	Do_ElementMonitor (bool syncMon)
{
    #ifdef EXTNDVERSION
    syncMon = true;
    #endif
    bool isteamwork = false;
    short userid = 0;
    GSErrCode err = IsTeamwork (isteamwork, userid);
    if (syncMon) {
        #if defined(TESTING)
        DBprnt ("Do_ElementMonitor on");
        #endif
        #if defined(AC_27) || defined(AC_28) || defined(AC_29)
        ACAPI_Element_CatchNewElement (nullptr, ElementEventHandlerProc);
        ACAPI_Element_InstallElementObserver (ElementEventHandlerProc);
        if (isteamwork) ACAPI_Notification_CatchElementReservationChange (ReservationChangeHandler);
        #else
        ACAPI_Notify_CatchNewElement (nullptr, ElementEventHandlerProc);			// for all elements
        ACAPI_Notify_InstallElementObserver (ElementEventHandlerProc);
        if (isteamwork) ACAPI_Notify_CatchElementReservationChange (ReservationChangeHandler);
        #endif
    }
    if (!syncMon) {
        #if defined(TESTING)
        DBprnt ("Do_ElementMonitor off");
        #endif
        #if defined(AC_27) || defined(AC_28) || defined(AC_29)
        ACAPI_Element_CatchNewElement (nullptr, nullptr);
        ACAPI_Element_InstallElementObserver (nullptr);
        if (isteamwork) ACAPI_Notification_CatchElementReservationChange (nullptr);
        #else
        ACAPI_Notify_CatchNewElement (nullptr, nullptr);
        ACAPI_Notify_InstallElementObserver (nullptr);
        if (isteamwork) ACAPI_Notify_CatchElementReservationChange (nullptr);
        #endif
    }
    return;
}	// Do_ElementMonitor

// -----------------------------------------------------------------------------
// Обновление отмеченных в меню пунктов
// -----------------------------------------------------------------------------
void MenuSetState (SyncSettings & syncSettings)
{
    MenuItemCheckAC (Menu_MonAll, syncSettings.syncMon);
    MenuItemCheckAC (Menu_wallS, syncSettings.wallS);
    MenuItemCheckAC (Menu_widoS, syncSettings.widoS);
    MenuItemCheckAC (Menu_objS, syncSettings.objS);
    MenuItemCheckAC (Menu_cwallS, syncSettings.cwallS);
    if (!isEng ()) return;
    for (UInt32 i = 0; i < MENU_ITEM_COUNT; i++) {
        SetPaletteMenuText (i);
    }
}

void SetPaletteMenuText (short paletteItemInd)
{
    API_MenuItemRef itemRef = {};
    GS::UniString itemStr = "";
    const Int32 bisEng = ID_ADDON_PROMT + isEng ();
    itemStr = RSGetIndString (bisEng, paletteItemInd + 1, ACAPI_GetOwnResModule ());
    itemRef.menuResID = ID_ADDON_MENU;
    itemRef.itemIndex = paletteItemInd;
    #if defined(AC_27) || defined(AC_28) || defined(AC_29)
    ACAPI_MenuItem_SetMenuItemText (&itemRef, nullptr, &itemStr);
    #else
    ACAPI_Interface (APIIo_SetMenuItemTextID, &itemRef, nullptr, &itemStr);
    #endif
    return;
}
static GSErrCode MenuCommandHandler (const API_MenuParams * menuParams)
{
    GSErrCode err = NoError;
    #if defined(TESTING)
    DBprnt ("MenuCommandHandler start");
    #endif
    SyncSettings syncSettings (false, false, true, true, true, true, false);
    LoadSyncSettingsFromPreferences (syncSettings, true);
    #ifdef EXTNDVERSION
    syncSettings.syncMon = true;
    #endif // PK_1
    #if defined(AC_27) || defined(AC_28) || defined(AC_29)
    ACAPI_UserInput_ClearElementHighlight ();
    #else
    #if defined(AC_26)
    ACAPI_Interface_ClearElementHighlight ();
    #else
    ACAPI_Interface (APIIo_HighlightElementsID);
    #endif
    #endif
    const Int32 AddOnMenuID = ID_ADDON_MENU;
    PROPERTYCACHE ().Update ();
    switch (menuParams->menuItemRef.menuResID) {
        case AddOnMenuID:
            switch (menuParams->menuItemRef.itemIndex) {
                case MonAll_CommandID:
                    syncSettings.syncAll = false;
                    #ifndef EXTNDVERSION
                    syncSettings.syncMon = !syncSettings.syncMon;
                    #endif // PK_1
                    Do_ElementMonitor (syncSettings.syncMon);
                    MonAll (syncSettings);
                    break;
                case SyncAll_CommandID:
                    msg_rep ("SyncAll", "============== START ==============", NoError, APINULLGuid);
                    syncSettings.syncAll = true;
                    SyncAndMonAll (syncSettings);
                    syncSettings.syncAll = false;
                    msg_rep ("SyncAll", "=============== END ===============", NoError, APINULLGuid);
                    break;
                case SyncSelect_CommandID:
                    SyncSelected (syncSettings);
                    break;
                case wallS_CommandID:
                    syncSettings.wallS = !syncSettings.wallS;
                    break;
                case widoS_CommandID:
                    syncSettings.widoS = !syncSettings.widoS;
                    break;
                case objS_CommandID:
                    syncSettings.objS = !syncSettings.objS;
                    break;
                case cwallS_CommandID:
                    syncSettings.cwallS = !syncSettings.cwallS;
                    break;
                    #ifndef AC_22
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
                    #ifndef AC_22
                case Auto3D_CommandID:
                    AutoFunc::ProfileByLine ();
                    break;
                case AutoLay_CommandID:
                    AutoFunc::AlignDrawingsByPoints ();
                    break;
                    #endif
            }
            break;
    }
    (void) err;
    DimRoundAll (syncSettings, false);
    WriteSyncSettingsToPreferences (syncSettings);
    MenuSetState (syncSettings);
    ACAPI_KeepInMemory (true);
    #ifdef TESTING
    DBprnt ("MenuCommandHandler end");
    #endif
    return NoError;
}

#if defined(AC_28) || defined(AC_29)
API_AddonType CheckEnvironment (API_EnvirParams * envir)
{
    #else
API_AddonType __ACDLL_CALL CheckEnvironment (API_EnvirParams * envir)
{
    #endif
    #ifdef TESTING
    DBprnt ("CheckEnvironment");
    TestFunc::Test ();
    #endif
    RSGetIndString (&envir->addOnInfo.name, ID_ADDON_INFO + isEng (), AddOnNameID, ACAPI_GetOwnResModule ());
    RSGetIndString (&envir->addOnInfo.description, ID_ADDON_INFO + isEng (), AddOnDescriptionID, ACAPI_GetOwnResModule ());
    ACAPI_KeepInMemory (true);
    return APIAddon_Preload;
}
#if defined(AC_28) || defined(AC_29)
GSErrCode RegisterInterface (void)
{
    #else
GSErrCode __ACDLL_CALL RegisterInterface (void)
{
    #endif
    #if defined(TESTING)
    DBprnt ("RegisterInterface");
    #endif
    GSErrCode err = NoError;
    #if defined(AC_27) || defined(AC_28) || defined(AC_29)
    err = ACAPI_MenuItem_RegisterMenu (ID_ADDON_MENU, ID_ADDON_PROMT + isEng (), MenuCode_Tools, MenuFlag_Default);
    #else
    err = ACAPI_Register_Menu (ID_ADDON_MENU, ID_ADDON_PROMT + isEng (), MenuCode_Tools, MenuFlag_Default);
    #endif
    ACAPI_KeepInMemory (true);
    return err;
}
#if defined(AC_28) || defined(AC_29)
GSErrCode Initialize (void)
{
    #else
GSErrCode __ACENV_CALL Initialize (void)
{
    #endif
    #if defined(TESTING)
    DBprnt ("Initialize");
    #endif
    SyncSettings syncSettings (false, false, true, true, true, true, false);
    LoadSyncSettingsFromPreferences (syncSettings, true);
    #ifdef EXTNDVERSION
    syncSettings.syncMon = true;
    #endif // PK_1
    MenuSetState (syncSettings);
    Do_ElementMonitor (syncSettings.syncMon);
    MonAll (syncSettings);
    #if defined(AC_27) || defined(AC_28) || defined(AC_29)
    ACAPI_ProjectOperation_CatchProjectEvent (APINotify_ChangeWindow | APINotify_ChangeFloor | APINotify_New | APINotify_NewAndReset | APINotify_Open | APINotify_Close | APINotify_Quit | APINotify_ChangeProjectDB, ProjectEventHandlerProc);
    #else
    ACAPI_Notify_CatchProjectEvent (APINotify_ChangeWindow | APINotify_ChangeFloor | APINotify_New | APINotify_NewAndReset | APINotify_Open | APINotify_Close | APINotify_Quit | APINotify_ChangeProjectDB, ProjectEventHandlerProc);
    #endif
    ACAPI_KeepInMemory (true);
    #if defined(AC_27) || defined(AC_28) || defined(AC_29)
    return ACAPI_MenuItem_InstallMenuHandler (ID_ADDON_MENU, MenuCommandHandler);
    #else
    return ACAPI_Install_MenuHandler (ID_ADDON_MENU, MenuCommandHandler);
    #endif
}
#if defined(AC_28) || defined(AC_29)
GSErrCode FreeData (void)
{
    #else
GSErrCode __ACENV_CALL FreeData (void)
{
    #endif
    #if defined(TESTING)
    DBprnt ("!!!!!FreeData");
    #endif
    return NoError;
}
