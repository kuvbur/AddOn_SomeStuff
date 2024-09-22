//------------ kuvbur 2022 ------------
#include	"ACAPinc.h"
#include	"APIEnvir.h"
#include	<stdio.h>
#ifdef PK_1
#include	"AutomateFunction.hpp"
#endif
#ifdef TESTING
#include "TestFunc.hpp"
#endif
#ifdef AC_25
#include	"APICommon25.h"
#endif // AC_25
#ifdef AC_26
#include	"APICommon26.h"
#endif // AC_26
#ifdef AC_27
#include	"APICommon27.h"
#endif // AC_27
#ifdef AC_28
#include	"APICommon28.h"
#endif // AC_28
#include	"DGModule.hpp"
#include	"UniString.hpp"
#include	"APIdefs_Properties.h"
#include	"SomeStuff_Main.hpp"
#include	"Sync.hpp"
#ifndef AC_22
#include	"ReNum.hpp"
#endif
#if !defined(AC_22) && !defined(AC_23)
#include	"Revision.hpp"
#endif
#include	"Summ.hpp"
#include	"Dimensions.hpp"
#include	"Spec.hpp"


//-----------------------------------------------------------------------------
// Срабатывает при событиях в тимворк
//-----------------------------------------------------------------------------
#if defined(AC_28)
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
    DBprnt ("ReservationChangeHandler");
    SyncSettings syncSettings (false, false, true, true, true, true, false);
    LoadSyncSettingsFromPreferences (syncSettings);
#ifdef PK_1
    syncSettings.syncMon = true;
#endif // PK_1
    for (GS::HashTable<API_Guid, short>::ConstPairIterator it = reserved.EnumeratePairs (); it != nullptr; ++it) {
#if defined(AC_28)
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
#if defined(AC_28)
static GSErrCode ProjectEventHandlerProc (API_NotifyEventID notifID, Int32 param)
{
    DBprnt ("ProjectEventHandlerProc");
    SyncSettings syncSettings (false, false, true, true, true, true, false);
#else
static GSErrCode __ACENV_CALL    ProjectEventHandlerProc (API_NotifyEventID notifID, Int32 param)
{
    DBprnt ("ProjectEventHandlerProc");
    SyncSettings syncSettings (false, false, true, true, true, true, false);
#endif
    LoadSyncSettingsFromPreferences (syncSettings);
#ifdef PK_1
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
#if defined(AC_27) || defined(AC_28)
            ACAPI_Element_CatchNewElement (nullptr, nullptr);
            ACAPI_Element_InstallElementObserver (nullptr);
#else
            ACAPI_Notify_CatchNewElement (nullptr, nullptr);
            ACAPI_Notify_InstallElementObserver (nullptr);
#endif
            break;
        case APINotify_ChangeWindow:
        case APINotify_ChangeFloor:
            break;
        default:
            break;
    }
    (void) param;
    DimRoundAll (syncSettings);
    return NoError;
}	// ProjectEventHandlerProc

// -----------------------------------------------------------------------------
// Срабатывает при изменении элемента
// -----------------------------------------------------------------------------
#if defined(AC_28)
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
#ifdef PK_1
    syncSettings.syncMon = true;
#endif // PK_1
    API_ActTranPars actTranPars;
#if defined(AC_27) || defined(AC_28)
    ACAPI_Notification_GetTranParams (&actTranPars);
#else
    ACAPI_Notify_GetTranParams (&actTranPars);
#endif
    API_EditCmdID acttype = actTranPars.typeID;
    if (!syncSettings.syncMon) return NoError;
    if (elemType->notifID == APINotifyElement_EndEvents) {
        DimRoundAll (syncSettings);
        return NoError;
    }
    if (elemType->notifID == APINotifyElement_BeginEvents || elemType->notifID == APINotifyElement_EndEvents) return NoError;
    if (elemType->elemHead.hotlinkGuid != APINULLGuid) return false;

    // Смотрим - что поменялось
    DBprnt ("ElementEventHandlerProc start");
    if (acttype == APIEdit_Drag) {
        if (is_equal (actTranPars.theDisp.x, 0) && is_equal (actTranPars.theDisp.y, 0) && is_equal (actTranPars.theDispZ, 0)) {
            DBprnt ("acttype == APIEdit_Drag 0");
            return NoError;
        }
    }
    API_ElemTypeID elementType;
#if defined AC_26 || defined AC_27 || defined AC_28
    elementType = elemType->elemHead.type.typeID;
#else
    elementType = elemType->elemHead.typeID;
#endif
    if (elementType == API_GroupID) return NoError;
    if (elementType == API_DimensionID) return NoError;
    if (!CheckElementType (elementType, syncSettings)) return NoError;
    if (!IsElementEditable (elemType->elemHead.guid, syncSettings, false)) return NoError;
    ParamDictValue propertyParams = {};
    ParamHelpers::AddValueToParamDictValue (propertyParams, "flag:no_attrib"); // Во время отслеживания не будем получать весь список слоёв
    ParamDictElement paramToWrite = {};
    ClassificationFunc::SystemDict systemdict;
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
            SyncElement (elemType->elemHead.guid, syncSettings, propertyParams, paramToWrite, dummymode, systemdict);
            if (!paramToWrite.IsEmpty ()) {
                GS::Array<API_Guid> rereadelem;
                rereadelem = ParamHelpers::ElementsWrite (paramToWrite);
                if (!rereadelem.IsEmpty ()) {
                    DBprnt ("ElementEventHandlerProc", "reread element");
                    for (UInt32 i = 0; i < rereadelem.GetSize (); i++) {
                        propertyParams.Clear ();
                        paramToWrite.Clear ();
                        SyncElement (rereadelem[i], syncSettings, propertyParams, paramToWrite, dummymode, systemdict);
                        ParamHelpers::ElementsWrite (paramToWrite);
                    }
                }
                ParamHelpers::InfoWrite (paramToWrite);
            }
        default:
            break;
    }
    DBprnt ("ElementEventHandlerProc end");
    return NoError;
}	// ElementEventHandlerProc

// -----------------------------------------------------------------------------
// Включение мониторинга
// -----------------------------------------------------------------------------
void	Do_ElementMonitor (bool& syncMon)
{
#ifdef PK_1
    syncMon = true;
#endif

    if (syncMon) {
        DBprnt ("Do_ElementMonitor on");
#if defined(AC_27) || defined(AC_28)
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
        DBprnt ("Do_ElementMonitor off");
#if defined(AC_27) || defined(AC_28)
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
    if (isEng () > 0) {
        for (UInt32 i = 0; i < 14; i++) {
            SetPaletteMenuText (i);
        }
    }
}

void SetPaletteMenuText (short paletteItemInd)
{
    API_MenuItemRef     itemRef;
    BNZeroMemory (&itemRef, sizeof (API_MenuItemRef));
    GS::UniString itemStr;
    itemStr = RSGetIndString (ID_ADDON_PROMT + isEng (), paletteItemInd + 1, ACAPI_GetOwnResModule ());
    itemRef.menuResID = ID_ADDON_MENU;
    itemRef.itemIndex = paletteItemInd;
#if defined(AC_27) || defined(AC_28)
    ACAPI_MenuItem_SetMenuItemText (&itemRef, nullptr, &itemStr);
#else
    ACAPI_Interface (APIIo_SetMenuItemTextID, &itemRef, nullptr, &itemStr);
#endif
    return;
}
static GSErrCode MenuCommandHandler (const API_MenuParams * menuParams)
{
    GSErrCode err = NoError;
    DBprnt ("MenuCommandHandler start");
    SyncSettings syncSettings (false, false, true, true, true, true, false);
    LoadSyncSettingsFromPreferences (syncSettings);
#ifdef PK_1
    syncSettings.syncMon = true;
#endif // PK_1
    const Int32 AddOnMenuID = ID_ADDON_MENU;
    switch (menuParams->menuItemRef.menuResID) {
        case AddOnMenuID:
            switch (menuParams->menuItemRef.itemIndex) {
                case MonAll_CommandID:
                    syncSettings.syncAll = false;
#ifndef PK_1
                    syncSettings.syncMon = !syncSettings.syncMon;
#endif // PK_1
                    Do_ElementMonitor (syncSettings.syncMon);
                    MonAll (syncSettings);
                    break;
                case SyncAll_CommandID:
                    syncSettings.syncAll = true;
                    SyncAndMonAll (syncSettings);
                    syncSettings.syncAll = false;
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
                    err = ReNumSelected (syncSettings);
                    break;
#endif
                case Sum_CommandID:
                    err = SumSelected (syncSettings);
                    break;
                case RunParam_CommandID:
                    RunParamSelected (syncSettings);
                    break;
                case Spec_CommandID:
                    Spec::SpecAll (syncSettings);
                    break;
                case ShowSub_CommandID:
                    Spec::ShowSub (syncSettings);
                    break;
#if !defined(AC_22) && !defined(AC_23)
                case SetRevision_CommandID:
                    Revision::SetRevision ();
                    break;
#endif
#ifdef PK_1
                case AutoList_CommandID:
                    AutoFunc::KM_ListUpdate ();
                    break;
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
    WriteSyncSettingsToPreferences (syncSettings);
    MenuSetState (syncSettings);
#if defined(AC_27) || defined(AC_28)
    ACAPI_ProcessWindow_CloseProcessWindow ();
#else
    ACAPI_Interface (APIIo_CloseProcessWindowID, nullptr, nullptr);
#endif
    ACAPI_KeepInMemory (true);
    DBprnt ("MenuCommandHandler end");
    return NoError;
}

#if defined(AC_28)
API_AddonType CheckEnvironment (API_EnvirParams * envir)
{
#else
API_AddonType __ACDLL_CALL CheckEnvironment (API_EnvirParams * envir)
{
#endif
    DBprnt ("CheckEnvironment");
#ifdef TESTING
    TestFunc::Test ();
#endif
    RSGetIndString (&envir->addOnInfo.name, ID_ADDON_INFO + isEng (), AddOnNameID, ACAPI_GetOwnResModule ());
    RSGetIndString (&envir->addOnInfo.description, ID_ADDON_INFO + isEng (), AddOnDescriptionID, ACAPI_GetOwnResModule ());
    ACAPI_KeepInMemory (true);
    return APIAddon_Preload;
}
#if defined(AC_28)
GSErrCode RegisterInterface (void)
{
#else
GSErrCode __ACDLL_CALL RegisterInterface (void)
{
#endif
    DBprnt ("RegisterInterface");
    GSErrCode err = NoError;
#if defined(AC_27) || defined(AC_28)
    err = ACAPI_MenuItem_RegisterMenu (ID_ADDON_MENU, ID_ADDON_PROMT + isEng (), MenuCode_Tools, MenuFlag_Default);
#else
    err = ACAPI_Register_Menu (ID_ADDON_MENU, ID_ADDON_PROMT + isEng (), MenuCode_Tools, MenuFlag_Default);
#endif
    return err;
}
#if defined(AC_28)
GSErrCode Initialize (void)
{
#else
GSErrCode __ACENV_CALL Initialize (void)
{
#endif

    DBprnt ("Initialize");
    SyncSettings syncSettings (false, false, true, true, true, true, false);
    LoadSyncSettingsFromPreferences (syncSettings);
#ifdef PK_1
    syncSettings.syncMon = true;
#endif // PK_1
    MenuSetState (syncSettings);
    Do_ElementMonitor (syncSettings.syncMon);
    MonAll (syncSettings);
#if defined(AC_27) || defined(AC_28)
    ACAPI_ProjectOperation_CatchProjectEvent (APINotify_ChangeWindow | APINotify_ChangeFloor | APINotify_New | APINotify_NewAndReset | APINotify_Open | APINotify_Close | APINotify_Quit, ProjectEventHandlerProc);
#else
    ACAPI_Notify_CatchProjectEvent (APINotify_ChangeWindow | APINotify_ChangeFloor | APINotify_New | APINotify_NewAndReset | APINotify_Open | APINotify_Close | APINotify_Quit, ProjectEventHandlerProc);
#endif
    ACAPI_KeepInMemory (true);
#if defined(AC_27) || defined(AC_28)
    return ACAPI_MenuItem_InstallMenuHandler (ID_ADDON_MENU, MenuCommandHandler);
#else
    return ACAPI_Install_MenuHandler (ID_ADDON_MENU, MenuCommandHandler);
#endif
}
#if defined(AC_28)
GSErrCode FreeData (void)
{
#else
GSErrCode __ACENV_CALL FreeData (void)
{
#endif
    DBprnt ("!!!!!FreeData");
    return NoError;
}
