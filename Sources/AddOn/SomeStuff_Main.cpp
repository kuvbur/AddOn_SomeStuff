//------------ kuvbur 2022 ------------
#include	<stdio.h>
#include	"APIEnvir.h"
#include	"ACAPinc.h"
#ifdef AC_25
#include	"APICommon25.h"
#endif // AC_25
#ifdef AC_26
#include	"APICommon26.h"
#endif // AC_26
#ifdef AC_27
#include	"APICommon27.h"
#endif // AC_27
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
	(void)deleted;
	(void)released;
	DBPrintf("== SMSTF == ReservationChangeHandler\n");
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
	DBPrintf("== SMSTF == ProjectEventHandlerProc\n");
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
#ifdef AC_27

		//TODO Заменить на АС27
#else
		ACAPI_Notify_CatchNewElement(nullptr, nullptr);
		ACAPI_Notify_InstallElementObserver(nullptr);
#endif
		break;
	case APINotify_ChangeWindow:
	case APINotify_ChangeFloor:
		break;
	default:
		break;
	}
	(void)param;
	DimRoundAll(syncSettings);
	return NoError;
}	// ProjectEventHandlerProc

// -----------------------------------------------------------------------------
// Срабатывает при изменении элемента
// -----------------------------------------------------------------------------
GSErrCode __ACENV_CALL	ElementEventHandlerProc(const API_NotifyElementType* elemType) {
	SyncSettings syncSettings(false, false, true, true, true, true, false);
	LoadSyncSettingsFromPreferences(syncSettings);
#ifdef PK_1
	syncSettings.syncMon = true;
#endif // PK_1
	API_ActTranPars actTranPars;
#ifdef AC_27

	//TODO Заменить на АС27
#else
	ACAPI_Notify_GetTranParams(&actTranPars);
#endif
	API_EditCmdID acttype = actTranPars.typeID;
	if (!syncSettings.syncMon) return NoError;
	if (elemType->notifID == APINotifyElement_EndEvents) {
		DimRoundAll(syncSettings);
		return NoError;
	}
	if (elemType->notifID == APINotifyElement_BeginEvents || elemType->notifID == APINotifyElement_EndEvents) return NoError;
	if (elemType->elemHead.hotlinkGuid != APINULLGuid) return false;

	// Смотрим - что поменялось
	DBPrintf("== SMSTF == ElementEventHandlerProc start\n");
	if (acttype == APIEdit_Drag) {
		if (is_equal(actTranPars.theDisp.x, 0) && is_equal(actTranPars.theDisp.y, 0) && is_equal(actTranPars.theDispZ, 0)) {
			DBPrintf("== SMSTF == acttype == APIEdit_Drag 0\n");
			return NoError;
		}
	}

#ifdef AC_26
	if (elemType->elemHead.type.typeID == API_GroupID) return NoError;
	if (!CheckElementType(elemType->elemHead.type.typeID, syncSettings)) return NoError;
#else
#ifdef AC_27

	//TODO Заменить на АС27
#else
	if (elemType->elemHead.typeID == API_GroupID) return NoError;
	if (!CheckElementType(elemType->elemHead.typeID, syncSettings)) return NoError;
#endif
#endif
	if (!IsElementEditable(elemType->elemHead.guid, syncSettings, true)) return NoError;
	ParamDictValue propertyParams = {};
	ParamDictElement paramToWrite = {};
	switch (elemType->notifID) {
	case APINotifyElement_New:
	case APINotifyElement_Change:
	case APINotifyElement_PropertyValueChange:
	case APINotifyElement_Edit:
	case APINotifyElement_ClassificationChange:
		SyncElement(elemType->elemHead.guid, syncSettings, propertyParams, paramToWrite);
		if (!paramToWrite.IsEmpty()) {
			ParamHelpers::ElementsWrite(paramToWrite);
		}
	default:
		break;
	}
	DBPrintf("== SMSTF == ElementEventHandlerProc end\n");
	return NoError;
}	// ElementEventHandlerProc

// -----------------------------------------------------------------------------
// Включение мониторинга
// -----------------------------------------------------------------------------
void	Do_ElementMonitor(bool& syncMon) {
#ifdef PK_1
	syncMon = true;
#endif

	if (syncMon) {
		DBPrintf("== SMSTF == Do_ElementMonitor on\n");
#ifdef AC_27

		//TODO Заменить на АС27
#else
		ACAPI_Notify_CatchNewElement(nullptr, ElementEventHandlerProc);			// for all elements
		ACAPI_Notify_InstallElementObserver(ElementEventHandlerProc);
		ACAPI_Notify_CatchElementReservationChange(ReservationChangeHandler);
#endif
	}
	if (!syncMon) {
		DBPrintf("== SMSTF == Do_ElementMonitor off\n");
#ifdef AC_27

		//TODO Заменить на АС27
#else
		ACAPI_Notify_CatchNewElement(nullptr, nullptr);
		ACAPI_Notify_InstallElementObserver(nullptr);
		ACAPI_Notify_CatchElementReservationChange(nullptr);
#endif
	}
	return;
}	// Do_ElementMonitor

// -----------------------------------------------------------------------------
// Обновление отмеченных в меню пунктов
// -----------------------------------------------------------------------------
void MenuSetState(SyncSettings& syncSettings) {
	MenuItemCheckAC(Menu_MonAll, syncSettings.syncMon);
	MenuItemCheckAC(Menu_wallS, syncSettings.wallS);
	MenuItemCheckAC(Menu_widoS, syncSettings.widoS);
	MenuItemCheckAC(Menu_objS, syncSettings.objS);
	MenuItemCheckAC(Menu_cwallS, syncSettings.cwallS);
}

static GSErrCode MenuCommandHandler(const API_MenuParams* menuParams) {
	GSErrCode err = NoError;
	DBPrintf("== SMSTF == MenuCommandHandler start\n");
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
			MonAll(syncSettings);
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
			DimRoundAll(syncSettings);
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
			err = ReNumSelected(syncSettings);
			DimRoundAll(syncSettings);
			break;
		case Sum_CommandID:
			err = SumSelected(syncSettings);
			DimRoundAll(syncSettings);
			break;
		case RunParam_CommandID:
			RunParamSelected(syncSettings);
			DimRoundAll(syncSettings);
			break;
		case SetGUID_CommandID:
			SetSyncGUID();
			break;
		case ShowGUID_CommandID:
			ShowSyncGUID();
			break;
		}
		break;
	}
	(void)err;
	WriteSyncSettingsToPreferences(syncSettings);
	MenuSetState(syncSettings);
#ifdef AC_27
	ACAPI_ProcessWindow_CloseProcessWindow();
#else
	ACAPI_Interface(APIIo_CloseProcessWindowID, nullptr, nullptr);
#endif
	ACAPI_KeepInMemory(true);
	DBPrintf("== SMSTF == MenuCommandHandler end\n");
	return NoError;
}

API_AddonType __ACDLL_CALL CheckEnvironment(API_EnvirParams* envir) {
	DBPrintf("== SMSTF == CheckEnvironment\n");
	RSGetIndString(&envir->addOnInfo.name, AddOnInfoID, AddOnNameID, ACAPI_GetOwnResModule());
	RSGetIndString(&envir->addOnInfo.description, AddOnInfoID, AddOnDescriptionID, ACAPI_GetOwnResModule());
	ACAPI_KeepInMemory(true);
	return APIAddon_Preload;
}

GSErrCode __ACDLL_CALL RegisterInterface(void) {
	DBPrintf("== SMSTF == RegisterInterface\n");
#ifdef AC_27

	//TODO Заменить на АС27
#else
	GSErrCode err = ACAPI_Register_Menu(AddOnMenuID, AddOnPromtID, MenuCode_Tools, MenuFlag_Default);
#endif
	return err;
}

GSErrCode __ACENV_CALL Initialize(void) {
	DBPrintf("== SMSTF == Initialize\n");
	SyncSettings syncSettings(false, false, true, true, true, true, false);
	LoadSyncSettingsFromPreferences(syncSettings);
#ifdef PK_1
	syncSettings.syncMon = true;
#endif // PK_1
	MenuSetState(syncSettings);
	Do_ElementMonitor(syncSettings.syncMon);
	MonAll(syncSettings);
#ifdef AC_27

	//TODO Заменить на АС27
#else
	ACAPI_Notify_CatchProjectEvent(APINotify_ChangeWindow | APINotify_ChangeFloor | APINotify_New | APINotify_NewAndReset | APINotify_Open | APINotify_Close | APINotify_Quit, ProjectEventHandlerProc);
#endif
	ACAPI_KeepInMemory(true);
#ifdef AC_27

	//TODO Заменить на АС27
#else
	return ACAPI_Install_MenuHandler(AddOnMenuID, MenuCommandHandler);
#endif
}

GSErrCode __ACENV_CALL FreeData(void) {
	DBPrintf("== SMSTF == FreeData\n");
	return NoError;
}