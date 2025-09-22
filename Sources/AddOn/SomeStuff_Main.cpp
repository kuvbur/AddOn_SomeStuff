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
    return NoError;
}

// -----------------------------------------------------------------------------
// Срабатывает при событиях проекта (открытие, сохранение)
// -----------------------------------------------------------------------------
#if defined(AC_28) || defined(AC_29)
static GSErrCode ProjectEventHandlerProc (API_NotifyEventID notifID, Int32 param)
{
    DBprnt ("ProjectEventHandlerProc");
    SyncSettings syncSettings (false, false, true, true, true, true, false);
    #else
static GSErrCode __ACENV_CALL    ProjectEventHandlerProc (API_NotifyEventID notifID, Int32 param)
{
    #ifdef TESTING
    DBprnt ("ProjectEventHandlerProc");
    #endif
    SyncSettings syncSettings (false, false, true, true, true, true, false);
    #endif
    LoadSyncSettingsFromPreferences (syncSettings);
    #ifdef EXTNDVERSION
    syncSettings.syncMon = true;
    #endif // PK_1
    MenuSetState (syncSettings);
    switch (notifID) {
        case APINotify_New:
        case APINotify_NewAndReset:
        case APINotify_Open:
            Do_ElementMonitor (syncSettings.syncMon);
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
    return NoError;
}	// ProjectEventHandlerProc

// -----------------------------------------------------------------------------
// Срабатывает при изменении элемента
// -----------------------------------------------------------------------------
#if defined(AC_28) || defined(AC_29)
GSErrCode ElementEventHandlerProc (const API_NotifyElementType * elemType)
{
    SyncSettings syncSettings (false, false, true, true, true, true, false);
    LoadSyncSettingsFromPreferences (syncSettings);
    #else
GSErrCode __ACENV_CALL	ElementEventHandlerProc (const API_NotifyElementType * elemType)
{
    SyncSettings syncSettings (false, false, true, true, true, true, false);
    LoadSyncSettingsFromPreferences (syncSettings);
    #endif
    int dummymode = DUMMY_MODE_UNDEF;
    #ifdef EXTNDVERSION
    syncSettings.syncMon = true;
    #endif // PK_1
    if (!syncSettings.syncMon) return NoError;
    if (elemType->notifID == APINotifyElement_EndEvents) {
        DimRoundAll (syncSettings, true);
        return NoError;
    }
    if (elemType->notifID == APINotifyElement_BeginEvents || elemType->notifID == APINotifyElement_EndEvents) return NoError;
    if (elemType->elemHead.hotlinkGuid != APINULLGuid) return false;

    // Смотрим - что поменялось
    #if defined(TESTING)
    DBprnt ("ElementEventHandlerProc start");
    #endif
    API_ElemTypeID elementType = GetElemTypeID (elemType->elemHead);
    #ifdef EXTNDVERSION
    if (elementType == API_DimensionID ||
        elementType == API_RadialDimensionID ||
        elementType == API_LevelDimensionID ||
        elementType == API_AngleDimensionID ||
        elementType == API_TextID ||
        elementType == API_LabelID) {
        GSErrCode err = NoError;
        if (elemType->elemHead.drwIndex < 13) {
            GS::Array<API_Guid> elemList = {};
            elemList.PushNew (elemType->elemHead.guid);
            err = ACAPI_Element_Tool (elemList, APITool_BringToFront, nullptr);
        }
        return err;
    }
    #endif // PK_1
    if (elementType == API_ZombieElemID) return NoError;
    if (elementType == API_GroupID) return NoError;
    if (elementType == API_DimensionID) return NoError;
    ParamDictValue propertyParams = {};
    ParamDictElement paramToWrite = {};
    ClassificationFunc::SystemDict systemdict = {};
    if (!IsElementEditable (elemType->elemHead.guid, syncSettings, true, elementType)) return NoError;
    ParamHelpers::AddValueToParamDictValue (propertyParams, "flag:no_attrib"); // Во время отслеживания не будем получать весь список слоёв
    bool needresync = false;
    switch (elemType->notifID) {
        case APINotifyElement_New:
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
            needresync = SyncElement (elemType->elemHead.guid, syncSettings, propertyParams, paramToWrite, dummymode, systemdict);
            if (!paramToWrite.IsEmpty ()) {
                GS::Array<API_Guid> rereadelem = {};
                rereadelem = ParamHelpers::ElementsWrite (paramToWrite);
                if (needresync) {
                    paramToWrite.Clear ();
                    needresync = SyncElement (elemType->elemHead.guid, syncSettings, propertyParams, paramToWrite, dummymode, systemdict);
                    GS::Array<API_Guid> rereadelem_ = {};
                    rereadelem_ = ParamHelpers::ElementsWrite (paramToWrite);
                    if (!rereadelem_.IsEmpty ()) rereadelem.Append (rereadelem_);
                }
                if (!rereadelem.IsEmpty ()) {
                    #if defined(TESTING)
                    DBprnt ("ElementEventHandlerProc", "reread element");
                    #endif
                    for (UInt32 i = 0; i < rereadelem.GetSize (); i++) {
                        propertyParams.Clear ();
                        paramToWrite.Clear ();
                        needresync = SyncElement (rereadelem[i], syncSettings, propertyParams, paramToWrite, dummymode, systemdict);
                        ParamHelpers::ElementsWrite (paramToWrite);
                    }
                }
                ParamHelpers::InfoWrite (paramToWrite);
            }
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
void	Do_ElementMonitor (bool& syncMon)
{
    #ifdef EXTNDVERSION
    syncMon = true;
    #endif

    if (syncMon) {
        #if defined(TESTING)
        DBprnt ("Do_ElementMonitor on");
        #endif
        #if defined(AC_27) || defined(AC_28) || defined(AC_29)
        ACAPI_Element_CatchNewElement (nullptr, ElementEventHandlerProc);
        ACAPI_Element_InstallElementObserver (ElementEventHandlerProc);
        ACAPI_Notification_CatchElementReservationChange (ReservationChangeHandler);
        #else
        ACAPI_Notify_CatchNewElement (nullptr, ElementEventHandlerProc);			// for all elements
        ACAPI_Notify_InstallElementObserver (ElementEventHandlerProc);
        ACAPI_Notify_CatchElementReservationChange (ReservationChangeHandler);
        #endif
    }
    if (!syncMon) {
        #if defined(TESTING)
        DBprnt ("Do_ElementMonitor off");
        #endif
        #if defined(AC_27) || defined(AC_28) || defined(AC_29)
        ACAPI_Element_CatchNewElement (nullptr, nullptr);
        ACAPI_Element_InstallElementObserver (nullptr);
        ACAPI_Notification_CatchElementReservationChange (nullptr);
        #else
        ACAPI_Notify_CatchNewElement (nullptr, nullptr);
        ACAPI_Notify_InstallElementObserver (nullptr);
        ACAPI_Notify_CatchElementReservationChange (nullptr);
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
    Int32 bisEng = isEng ();
    if (bisEng > 0) {
        for (UInt32 i = 0; i < 16; i++) {
            SetPaletteMenuText (i, bisEng);
        }
    }
}

void SetPaletteMenuText (short paletteItemInd, Int32 & bisEng)
{
    API_MenuItemRef     itemRef;
    BNZeroMemory (&itemRef, sizeof (API_MenuItemRef));
    GS::UniString itemStr;
    itemStr = RSGetIndString (ID_ADDON_PROMT + bisEng, paletteItemInd + 1, ACAPI_GetOwnResModule ());
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
    LoadSyncSettingsFromPreferences (syncSettings);
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
                    Revision::SetRevision ();
                    break;
                case SetSub_CommandID:
                    SyncSetSubelement (syncSettings);
                    break;
                case RoomBook_CommandID:
                    msg_rep ("RoomBook", "============== START ==============", NoError, APINULLGuid);
                    AutoFunc::RoomBook ();
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
    #if defined(AC_27) || defined(AC_28) || defined(AC_29)
    ACAPI_ProcessWindow_CloseProcessWindow ();
    #else
    ACAPI_Interface (APIIo_CloseProcessWindowID, nullptr, nullptr);
    #endif
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
    LoadSyncSettingsFromPreferences (syncSettings);
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
