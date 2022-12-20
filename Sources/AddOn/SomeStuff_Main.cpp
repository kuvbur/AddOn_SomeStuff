//------------ kuvbur 2022 ------------
#include	<stdio.h>
#include	"APIEnvir.h"
#include	"ACAPinc.h"
#ifdef ServerMainVers_2500
#include	"APICommon25.h"
#endif // ServerMainVers_2500
#ifdef ServerMainVers_2600
#include	"APICommon26.h"
#endif // ServerMainVers_2600
#include	"DGModule.hpp"
#include	"UniString.hpp"
#include	"APIdefs_Properties.h"
#include	"SomeStuff_Main.hpp"
#include	"Sync.hpp"
#include	"ReNum.hpp"
#include	"Summ.hpp"
#include	"Dimensions.hpp"

//-----------------------------------------------------------------------------
// Срабатывает при событиях в тимворк
//-----------------------------------------------------------------------------
static GSErrCode __ACENV_CALL	ReservationChangeHandler(const GS::HashTable<API_Guid, short>& reserved,
	const GS::HashSet<API_Guid>& released,
	const GS::HashSet<API_Guid>& deleted) {
	SyncSettings syncSettings(false, false, true, true, true, true, false);
	LoadSyncSettingsFromPreferences(syncSettings);
#ifdef PK_1
	syncSettings.syncMon = true;
#endif // PK_1
	for (GS::HashTable<API_Guid, short>::ConstPairIterator it = reserved.EnumeratePairs(); it != nullptr; ++it) {
		AttachObserver(*(it->key), syncSettings);
	}
	return NoError;
}

// -----------------------------------------------------------------------------
// Срабатывает при событиях проекта (открытие, сохранение)
// -----------------------------------------------------------------------------
static GSErrCode __ACENV_CALL    ProjectEventHandlerProc(API_NotifyEventID notifID, Int32 param) {
	SyncSettings syncSettings(false, false, true, true, true, true, false);
	LoadSyncSettingsFromPreferences(syncSettings);
#ifdef PK_1
	syncSettings.syncMon = true;
#endif // PK_1
	MenuSetState(syncSettings);
	switch (notifID) {
	case APINotify_New:
	case APINotify_NewAndReset:
	case APINotify_Open:
		Do_ElementMonitor(syncSettings.syncMon);
		break;
	case APINotify_Close:
	case APINotify_Quit:
		ACAPI_Notify_CatchNewElement(nullptr, nullptr);
		ACAPI_Notify_InstallElementObserver(nullptr);
		break;
	case APINotify_ChangeWindow:
	case APINotify_ChangeFloor:
		param = 1;
		break;
	default:
		break;
	}
	param = 1;
	DimRoundAll(syncSettings);
	return NoError;
}	// ProjectEventHandlerProc

// -----------------------------------------------------------------------------
// Срабатывает при изменении элемента
// -----------------------------------------------------------------------------
GSErrCode __ACENV_CALL	ElementEventHandlerProc(const API_NotifyElementType* elemType)
{
	SyncSettings syncSettings(false, false, true, true, true, true, false);
	LoadSyncSettingsFromPreferences(syncSettings);
#ifdef PK_1
	syncSettings.syncMon = true;
#endif // PK_1
	if (!syncSettings.syncMon) return NoError;
	if (elemType->notifID == APINotifyElement_EndEvents) {
		DimRoundAll(syncSettings);
		return NoError;
	}
	if (elemType->notifID == APINotifyElement_BeginEvents || elemType->notifID == APINotifyElement_EndEvents) return NoError;
#ifdef ServerMainVers_2600
	if (elemType->elemHead.type.typeID == API_GroupID) return NoError;
#else
	if (elemType->elemHead.typeID == API_GroupID) return NoError;
#endif
	if (!IsElementEditable(elemType->elemHead.guid, syncSettings, true)) return NoError;
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
		SyncElement(elemType->elemHead.guid, syncSettings);
	}
	return NoError;
}	// ElementEventHandlerProc

// -----------------------------------------------------------------------------
// Включение мониторинга
// -----------------------------------------------------------------------------
void	Do_ElementMonitor(bool& syncMon)
{
#ifdef PK_1
	syncMon = true;
#endif
	if (syncMon) {
		ACAPI_Notify_CatchNewElement(nullptr, ElementEventHandlerProc);			// for all elements
		ACAPI_Notify_InstallElementObserver(ElementEventHandlerProc);
		ACAPI_Notify_CatchElementReservationChange(ReservationChangeHandler);
	}
	if (!syncMon) {
		ACAPI_Notify_CatchNewElement(nullptr, nullptr);
		ACAPI_Notify_InstallElementObserver(nullptr);
		ACAPI_Notify_CatchElementReservationChange(nullptr);
	}
	return;
}	// Do_ElementMonitor

static GSErrCode MenuCommandHandler(const API_MenuParams* menuParams) {
	bool t_flag = false;
	GSErrCode err = NoError;
	SyncSettings syncSettings(false, false, true, true, true, true, false);
	LoadSyncSettingsFromPreferences(syncSettings);
#ifdef PK_1
	syncSettings.syncMon = true;
#endif // PK_1
	switch (menuParams->menuItemRef.menuResID) {
	case AddOnMenuID:
		switch (menuParams->menuItemRef.itemIndex) {
		case MonAll_CommandID:
			syncSettings.syncAll = false;
#ifndef PK_1
			syncSettings.syncMon = !syncSettings.syncMon;
#endif // PK_1
			Do_ElementMonitor(syncSettings.syncMon);
			SyncAndMonAll(syncSettings);
			DimRoundAll(syncSettings);
			break;
		case SyncAll_CommandID:
			syncSettings.syncAll = true;
			SyncAndMonAll(syncSettings);
			DimRoundAll(syncSettings);
			syncSettings.syncAll = false;
			break;
		case SyncSelect_CommandID:
			SyncSelected(syncSettings);
			DimSelected(syncSettings);
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

API_AddonType __ACDLL_CALL CheckEnvironment(API_EnvirParams* envir)
{
	RSGetIndString(&envir->addOnInfo.name, AddOnInfoID, AddOnNameID, ACAPI_GetOwnResModule());
	RSGetIndString(&envir->addOnInfo.description, AddOnInfoID, AddOnDescriptionID, ACAPI_GetOwnResModule());
	ACAPI_KeepInMemory(true);
	return APIAddon_Preload;
}

GSErrCode __ACDLL_CALL RegisterInterface(void)
{
	GSErrCode err = ACAPI_Register_Menu(AddOnMenuID, AddOnPromtID, MenuCode_Tools, MenuFlag_Default);
	return err;
}

GSErrCode __ACENV_CALL Initialize(void)
{
	SyncSettings syncSettings(false, false, true, true, true, true, false);
	LoadSyncSettingsFromPreferences(syncSettings);
#ifdef PK_1
	syncSettings.syncMon = true;
#endif // PK_1
	MenuSetState(syncSettings);
	Do_ElementMonitor(syncSettings.syncMon);
	ACAPI_Notify_CatchProjectEvent(APINotify_ChangeWindow | APINotify_ChangeFloor | APINotify_New | APINotify_NewAndReset | APINotify_Open | APINotify_Close | APINotify_Quit, ProjectEventHandlerProc);
	ACAPI_KeepInMemory(true);
	return ACAPI_Install_MenuHandler(AddOnMenuID, MenuCommandHandler);
}

GSErrCode __ACENV_CALL FreeData(void)
{
	return NoError;
}