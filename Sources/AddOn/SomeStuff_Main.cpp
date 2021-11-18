#include	"APIEnvir.h"
#include	<stdio.h>
#include	"ACAPinc.h"
#include	"DGModule.hpp"
#include	"APICommon.h"
#include	"UniString.hpp"
#include	"APIdefs_Properties.h"
#include	"SomeStuff_Main.hpp"
#include	"Helpers.hpp"
#include	"Sync.hpp"
#include	"ReNum.hpp"
#include	"Summ.hpp"

// -----------------------------------------------------------------------------
// Срабатывает при событиях проекта (открытие, сохранение)
// -----------------------------------------------------------------------------
static GSErrCode __ACENV_CALL    ProjectEventHandlerProc(API_NotifyEventID notifID, Int32 param) {
	switch (notifID) {
	case APINotify_New:
	case APINotify_NewAndReset:
	case APINotify_Open:
		MenuSetState();
		Do_ElementMonitor();
		break;
	case APINotify_Close:
	case APINotify_Quit:
		ACAPI_Notify_CatchNewElement(nullptr, nullptr);
		ACAPI_Notify_InstallElementObserver(nullptr);
		break;
	default:
		break;
	}
	param = 1;
	return NoError;
}	// ProjectEventHandlerProc

// -----------------------------------------------------------------------------
// Срабатывает при изменении элемента
// -----------------------------------------------------------------------------
GSErrCode __ACENV_CALL	ElementEventHandlerProc(const API_NotifyElementType* elemType)
{
	GSErrCode		err = NoError;
	if (elemType->notifID == APINotifyElement_BeginEvents || elemType->notifID == APINotifyElement_EndEvents) return err;
	if (!IsElementEditable(elemType->elemHead.guid)) {
		return err;
	}
	SyncPrefs prefsData;
	SyncSettingsGet(prefsData);
	if (!prefsData.syncMon) return err;

	bool	sync_prop = false;
	switch (elemType->notifID) {
	case APINotifyElement_New:
	case APINotifyElement_Copy:
	case APINotifyElement_Change:
	case APINotifyElement_Edit:
	case APINotifyElement_Undo_Modified:
	case APINotifyElement_Redo_Created:
	case APINotifyElement_Redo_Modified:
	case APINotifyElement_Redo_Deleted:
	case APINotifyElement_ClassificationChange:
		sync_prop = true;
	default:
		break;
	}
	if (sync_prop) {
		err = AttachObserver(elemType->elemHead.guid);
		if (err == APIERR_LINKEXIST)
			err = NoError;
		if (err == NoError) {
			SyncData(elemType->elemHead.guid);
			SyncRelationsElement(elemType->elemHead.guid);
		}
	}
	return err;
}	// ElementEventHandlerProc

// -----------------------------------------------------------------------------
// Включение мониторинга
// -----------------------------------------------------------------------------
void	Do_ElementMonitor(void)
{
	SyncPrefs prefsData;
	SyncSettingsGet(prefsData);
	if (prefsData.syncMon) {
		ACAPI_Notify_CatchNewElement(nullptr, ElementEventHandlerProc);			// for all elements
		ACAPI_Notify_InstallElementObserver(ElementEventHandlerProc);	
	}
	if (!prefsData.syncMon) {
		ACAPI_Notify_CatchNewElement(nullptr, nullptr);
		ACAPI_Notify_InstallElementObserver(nullptr);
	}
	return;
}	// Do_ElementMonitor

static GSErrCode MenuCommandHandler (const API_MenuParams *menuParams){
	bool t_flag = false;
	GSErrCode err = NoError;
	SyncPrefs prefsData;
	SyncSettingsGet(prefsData);
	switch (menuParams->menuItemRef.menuResID) {
		case AddOnMenuID:
			switch (menuParams->menuItemRef.itemIndex) {
				case MonAll_CommandID:
					prefsData.syncMon = !prefsData.syncMon;
					prefsData.syncAll = false;
					err = ACAPI_SetPreferences(CURR_ADDON_VERS, sizeof(SyncPrefs), (GSPtr)&prefsData);
					Do_ElementMonitor();
					SyncAndMonAll();
					break;
				case SyncAll_CommandID:
					t_flag = prefsData.syncMon;
					if (t_flag) prefsData.syncMon = false;
					prefsData.syncAll = true;
					err = ACAPI_SetPreferences(CURR_ADDON_VERS, sizeof(SyncPrefs), (GSPtr)&prefsData);
					SyncAndMonAll();
					prefsData.syncAll = false;
					if (t_flag) prefsData.syncMon = true;
					err = ACAPI_SetPreferences(CURR_ADDON_VERS, sizeof(SyncPrefs), (GSPtr)&prefsData);
					break;
				case SyncSelect_CommandID:
					SyncSelected();
					break;
				case wallS_CommandID:
					prefsData.wallS = !prefsData.wallS;
					err = ACAPI_SetPreferences(CURR_ADDON_VERS, sizeof(SyncPrefs), (GSPtr)&prefsData);
					break;
				case widoS_CommandID:
					prefsData.widoS = !prefsData.widoS;
					err = ACAPI_SetPreferences(CURR_ADDON_VERS, sizeof(SyncPrefs), (GSPtr)&prefsData);
					break;
				case objS_CommandID:
					prefsData.objS = !prefsData.objS;
					err = ACAPI_SetPreferences(CURR_ADDON_VERS, sizeof(SyncPrefs), (GSPtr)&prefsData);
					break;
				case ReNum_CommandID:
					t_flag = prefsData.syncMon;
					if (t_flag) prefsData.syncMon = false;
					err = ACAPI_SetPreferences(CURR_ADDON_VERS, sizeof(SyncPrefs), (GSPtr)&prefsData);
					err = ReNumSelected();
					if (t_flag) prefsData.syncMon = true;
					err = ACAPI_SetPreferences(CURR_ADDON_VERS, sizeof(SyncPrefs), (GSPtr)&prefsData);
					break;
				case Sum_CommandID:
					t_flag = prefsData.syncMon;
					if (t_flag) prefsData.syncMon = false;
					err = ACAPI_SetPreferences(CURR_ADDON_VERS, sizeof(SyncPrefs), (GSPtr)&prefsData);
					err = SumSelected();
					if (t_flag) prefsData.syncMon = true;
					err = ACAPI_SetPreferences(CURR_ADDON_VERS, sizeof(SyncPrefs), (GSPtr)&prefsData);
					break;
			}
			break;
	}
	MenuSetState();
	ACAPI_Interface(APIIo_CloseProcessWindowID, nullptr, nullptr);
	return NoError;
}

API_AddonType __ACDLL_CALL CheckEnvironment (API_EnvirParams* envir)
{
	RSGetIndString (&envir->addOnInfo.name, AddOnInfoID, AddOnNameID, ACAPI_GetOwnResModule ());
	RSGetIndString (&envir->addOnInfo.description, AddOnInfoID, AddOnDescriptionID, ACAPI_GetOwnResModule ());
	return APIAddon_Preload;
}

GSErrCode __ACDLL_CALL RegisterInterface (void)
{	
	GSErrCode err = ACAPI_Register_Menu(AddOnMenuID, AddOnPromtID, MenuCode_Tools, MenuFlag_Default);
	return err;
}

GSErrCode __ACENV_CALL Initialize (void)
{
	ACAPI_Notify_CatchProjectEvent(APINotify_New | APINotify_NewAndReset | APINotify_Open | APINotify_Close | APINotify_Quit, ProjectEventHandlerProc);
	return ACAPI_Install_MenuHandler (AddOnMenuID, MenuCommandHandler);
}

GSErrCode __ACENV_CALL FreeData (void)
{
	return NoError;
}
