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
#include	"Dimensions.hpp"

// -----------------------------------------------------------------------------
// Срабатывает при событиях проекта (открытие, сохранение)
// -----------------------------------------------------------------------------
static GSErrCode __ACENV_CALL    ProjectEventHandlerProc(API_NotifyEventID notifID, Int32 param) {
	SyncSettings syncSettings(false, false, true, true, true, false);
	switch (notifID) {
	case APINotify_New:
	case APINotify_NewAndReset:
	case APINotify_Open:
		LoadSyncSettingsFromPreferences(syncSettings);
		MenuSetState(syncSettings);
		Do_ElementMonitor(syncSettings.syncMon);
		break;
	case APINotify_Close:
	case APINotify_Quit:
		ACAPI_Notify_CatchNewElement(nullptr, nullptr);
		ACAPI_Notify_InstallElementObserver(nullptr);
		break;
	case APINotify_ChangeWindow:
		param = 1;
		break;
	default:
		break;
	}
	param = 1;

	char    msgStr[256];

	switch (notifID) {
	case APINotify_PreSave:
		CHCopyC("APINotify_PreSave", msgStr);
		break;
	case APINotify_ChangeProjectDB:
		CHCopyC("APINotify_ChangeProjectDB", msgStr);
		break;
	case APINotify_ChangeWindow:
		CHCopyC("APINotify_ChangeWindow", msgStr);
		break;
	}
	ACAPI_WriteReport(msgStr, false);

	return NoError;
}	// ProjectEventHandlerProc

// -----------------------------------------------------------------------------
// Срабатывает при изменении элемента
// -----------------------------------------------------------------------------
GSErrCode __ACENV_CALL	ElementEventHandlerProc(const API_NotifyElementType* elemType)
{
	GSErrCode		err = NoError;
	if (elemType->notifID == APINotifyElement_BeginEvents || elemType->notifID == APINotifyElement_EndEvents) return err;
	SyncSettings syncSettings(false, false, true, true, true, false);
	LoadSyncSettingsFromPreferences(syncSettings);
	if (!syncSettings.syncMon) return err;
	if (!IsElementEditable(elemType->elemHead.guid, syncSettings, true)) {
		return err;
	}
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
		err = AttachObserver(elemType->elemHead.guid, syncSettings);
		if (err == APIERR_LINKEXIST)
			err = NoError;
		if (err == NoError) {
			if (elemType->elemHead.typeID == API_DimensionID) {
				DimAutoRoundSel(elemType->elemHead.guid, syncSettings);
			}
			else {
				SyncData(elemType->elemHead.guid, syncSettings);
				SyncRelationsElement(elemType->elemHead.guid, syncSettings);
				DimRoundAll(syncSettings);
			}
		}
	}
	return err;
}	// ElementEventHandlerProc

// -----------------------------------------------------------------------------
// Включение мониторинга
// -----------------------------------------------------------------------------
void	Do_ElementMonitor(bool& syncMon)
{
	if (syncMon) {
		ACAPI_Notify_CatchNewElement(nullptr, ElementEventHandlerProc);			// for all elements
		ACAPI_Notify_InstallElementObserver(ElementEventHandlerProc);	
	}
	if (!syncMon) {
		ACAPI_Notify_CatchNewElement(nullptr, nullptr);
		ACAPI_Notify_InstallElementObserver(nullptr);
	}
	return;
}	// Do_ElementMonitor

static GSErrCode MenuCommandHandler (const API_MenuParams *menuParams){
	bool t_flag = false;
	GSErrCode err = NoError;
	SyncSettings syncSettings(false, false, true, true, true, false);
	LoadSyncSettingsFromPreferences(syncSettings);
	switch (menuParams->menuItemRef.menuResID) {
		case AddOnMenuID:
			switch (menuParams->menuItemRef.itemIndex) {
				case MonAll_CommandID:
					syncSettings.syncMon = !syncSettings.syncMon;
					syncSettings.syncAll = false;
#ifdef PK_1
					syncSettings.syncMon = true;
#endif
					Do_ElementMonitor(syncSettings.syncMon);
					SyncAndMonAll(syncSettings);
					break;
				case SyncAll_CommandID:
					t_flag = syncSettings.syncMon;
					if (t_flag) syncSettings.syncMon = false;
					syncSettings.syncAll = true;
					SyncAndMonAll(syncSettings);
					syncSettings.syncAll = false;
					if (t_flag) syncSettings.syncMon = true;
					break;
				case SyncSelect_CommandID:
					SyncSelected(syncSettings);
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
				case ReNum_CommandID:
					err = ReNumSelected();
					break;
				case Sum_CommandID:
					err = SumSelected();
					break;
				case Log_CommandID:
					break;
			}
			break;
	}
	WriteSyncSettingsToPreferences(syncSettings);
	MenuSetState(syncSettings);
	ACAPI_Interface(APIIo_CloseProcessWindowID, nullptr, nullptr);
	ACAPI_KeepInMemory(true);
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
	SyncSettings syncSettings(false, false, true, true, true, false);
	LoadSyncSettingsFromPreferences(syncSettings);
	MenuSetState(syncSettings);
	Do_ElementMonitor(syncSettings.syncMon);
	ACAPI_Notify_CatchProjectEvent(APINotify_ChangeWindow | APINotify_PreSave | APINotify_ChangeProjectDB | APINotify_New | APINotify_NewAndReset | APINotify_Open | APINotify_Close | APINotify_Quit, ProjectEventHandlerProc);
	ACAPI_KeepInMemory(true);
	return ACAPI_Install_MenuHandler (AddOnMenuID, MenuCommandHandler);
}

GSErrCode __ACENV_CALL FreeData (void)
{
	return NoError;
}