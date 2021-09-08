#include "APIEnvir.h"
#include <stdio.h>
#include "ACAPinc.h"
#include "AddOnMain.h"
#include "DGModule.hpp"
#include "APICommon.h"
#include "UniString.hpp"
#include "Helpers.hpp"
#include "APIdefs_Properties.h"
#include "ReNum.hpp"
#include "Sync.hpp"
#include "Log.hpp"

static GS::HashTable<API_Guid, GS::UniString>	objectSetsToReport;

// -----------------------------------------------------------------------------
// GetTeamworkMembers
//
//  collects information of joined Teamwork members
// -----------------------------------------------------------------------------
static GSErrCode	GetTeamworkMembers(GS::HashTable<short, API_UserInfo>& userInfoTable, short& myUserId)
{
	API_ProjectInfo	projectInfo;
	BNZeroMemory(&projectInfo, sizeof(API_ProjectInfo));
	GSErrCode err = ACAPI_Environment(APIEnv_ProjectID, &projectInfo);
	if (err != NoError)
		return err;

	myUserId = projectInfo.userId;

	API_SharingInfo	sharingInfo;
	BNZeroMemory(&sharingInfo, sizeof(API_SharingInfo));
	err = ACAPI_Environment(APIEnv_ProjectSharingID, &sharingInfo);
	if (err == NoError && sharingInfo.users != nullptr) {
		for (Int32 i = 0; i < sharingInfo.nUsers; i++)
			userInfoTable.Add(((*sharingInfo.users)[i]).userId, (*sharingInfo.users)[i]);
	}
	if (sharingInfo.users != nullptr)
		BMhKill(reinterpret_cast<GSHandle*>(&sharingInfo.users));

	return err;
}		/* GetTeamworkMembers */


// -----------------------------------------------------------------------------
// Срабатывает при изменении элемента
// -----------------------------------------------------------------------------
GSErrCode __ACENV_CALL	ElementEventHandlerProc(const API_NotifyElementType* elemType)
{
	GSErrCode		err = NoError;
	bool	sync_prop = false;

	if (elemType->notifID != APINotifyElement_BeginEvents && elemType->notifID != APINotifyElement_EndEvents && CheckElementType(elemType->elemHead.typeID)) {
		API_Element			parentElement;
		API_ElementMemo		parentElementMemo;
		API_ElementUserData	parentUserData;

		BNZeroMemory(&parentElement, sizeof(API_Element));
		BNZeroMemory(&parentElementMemo, sizeof(API_ElementMemo));
		BNZeroMemory(&parentUserData, sizeof(API_ElementUserData));
		ACAPI_Notify_GetParentElement(&parentElement, &parentElementMemo, 0, &parentUserData);
		BMKillHandle(&parentUserData.dataHdl);
		SyncPrefs prefsData;
		SyncSettingsGet(prefsData);
		switch (elemType->notifID) {
		case APINotifyElement_New:
			if (!prefsData.syncMon && !prefsData.logMon)
				break;
			if (prefsData.syncMon) sync_prop = true;
			err = ACAPI_Element_AttachObserver(elemType->elemHead.guid);
			if (err == APIERR_LINKEXIST)
				err = NoError;
			break;
		case APINotifyElement_Copy:
			if (!prefsData.syncMon && !prefsData.logMon)
				break;
			if (prefsData.syncMon) sync_prop = true;
			if (parentElement.header.guid != APINULLGuid) {
				err = ACAPI_Element_AttachObserver(elemType->elemHead.guid);
				if (err == APIERR_LINKEXIST)
					err = NoError;
			}
			break;
		case APINotifyElement_Change:
			sync_prop = true;

		case APINotifyElement_Edit:
			sync_prop = true;
			break;

		case APINotifyElement_Undo_Modified:
			sync_prop = true;
			break;

		case APINotifyElement_Redo_Created:
			sync_prop = true;
			break;

		case APINotifyElement_Redo_Modified:
			sync_prop = true;
			break;

		case APINotifyElement_Redo_Deleted:
			sync_prop = true;
			break;

		case APINotifyElement_PropertyValueChange:
			sync_prop = true;
			break;

		case APINotifyElement_ClassificationChange:
			sync_prop = true;
			break;

		default:
			break;
		}
		if (sync_prop) SyncData(elemType->elemHead.guid);
		//Do_MarkSelElems(elemType->elemHead.guid);
		ACAPI_DisposeElemMemoHdls(&parentElementMemo);
	}
	return err;
}	// ElementEventHandlerProc


// -----------------------------------------------------------------------------
// PrintReservationInfo
//
//  prints reservation information of a given element
// -----------------------------------------------------------------------------
static void		PrintReservationInfo(const GS::HashTable<short, API_UserInfo>& userInfoTable,
	short										myUserId,
	const char* actionStr,
	const API_Guid objectId,
	short										ownerId = 0)
{
	GS::UniString actionByUserStr(actionStr);
	if (userInfoTable.ContainsKey(ownerId)) {
		if (ownerId == myUserId) {
			actionByUserStr.Append(" by me (");
			actionByUserStr.Append(userInfoTable[ownerId].fullName);
			actionByUserStr.Append(")");
		}
		else {
			actionByUserStr.Append(" by ");
			actionByUserStr.Append(userInfoTable[ownerId].fullName);
		}
	}
	API_Guid objectd = objectId;
	GS::UniString objectSetName = "---";
	if (!objectSetName.IsEmpty()) {
		const GS::UniString reportString = GS::UniString::Printf("=  %T got %T", objectSetName.ToPrintf(), actionByUserStr.ToPrintf());
		ACAPI_WriteReport(reportString, false);
	}
	//else {
	//	GS::UniString elemTypeName = FindElementTypeName(objectId);
	//	if (!elemTypeName.IsEmpty()) {
	//		const GS::UniString reportString = GS::UniString::Printf("=  %T {%T} is %T", elemTypeName.ToPrintf(), APIGuidToString(objectId).ToPrintf(), actionByUserStr.ToPrintf());
	//		ACAPI_WriteReport(reportString, false);
	//	}
	//}
}		/* PrintReservationInfo */


// -----------------------------------------------------------------------------
// ReservationChangeHandler
//
//  lists the recent reservation changes in Teamwork
// -----------------------------------------------------------------------------
static GSErrCode __ACENV_CALL	ReservationChangeHandler(const GS::HashTable<API_Guid, short>& reserved,
	const GS::HashSet<API_Guid>& released,
	const GS::HashSet<API_Guid>& deleted)
{
	GS::HashTable<short, API_UserInfo> userInfoTable;
	short myUserId = 0;

	GSErrCode err = GetTeamworkMembers(userInfoTable, myUserId);
	if (err != NoError)
		return err;

	ACAPI_WriteReport("=== Attention: Workspace reservation has been changed ===============", false);

	for (GS::HashTable<API_Guid, short>::ConstPairIterator it = reserved.EnumeratePairs(); it != nullptr; ++it) {
		PrintReservationInfo(userInfoTable, myUserId, "reserved", *(it->key), *(it->value));
	}

	for (GS::HashSet<API_Guid>::ConstIterator it = released.Enumerate(); it != nullptr; ++it) {
		PrintReservationInfo(userInfoTable, myUserId, "released", *it);
	}

	for (GS::HashSet<API_Guid>::ConstIterator it = deleted.Enumerate(); it != nullptr; ++it) {
		PrintReservationInfo(userInfoTable, myUserId, "deleted", *it);
	}

	return NoError;
}		/* ReservationChangeHandler */

// ============================================================================
// Do_ElementMonitor
//	observe all newly created elements
// ============================================================================
void	Do_ElementMonitor(void)
{
	SyncPrefs prefsData;
	SyncSettingsGet(prefsData);
	if (prefsData.syncMon || prefsData.logMon) {
		ACAPI_Notify_CatchNewElement(nullptr, ElementEventHandlerProc);			// for all elements
		ACAPI_Notify_InstallElementObserver(ElementEventHandlerProc);	
		ACAPI_Notify_CatchElementReservationChange(ReservationChangeHandler);
	}
	if (!prefsData.syncMon && !prefsData.logMon) 
	{
		ACAPI_Notify_CatchNewElement(nullptr, nullptr);
		ACAPI_Notify_InstallElementObserver(nullptr);
		ACAPI_Notify_CatchElementReservationChange(nullptr);
	}
	return;
}	// Do_ElementMonitor

static void MenuSetState(void) {
	SyncPrefs prefsData;
	SyncSettingsGet(prefsData);
	MenuItemCheckAC(Menu_MonAll, prefsData.syncMon);
	MenuItemCheckAC(Menu_wallS, prefsData.wallS);
	MenuItemCheckAC(Menu_widoS, prefsData.widoS);
	MenuItemCheckAC(Menu_objS, prefsData.objS);
}

static GSErrCode __ACENV_CALL    ProjectEventHandlerProc(API_NotifyEventID notifID, Int32 param)
{
	switch (notifID) {
	case APINotify_New:
		MenuSetState();
		break;
	case APINotify_NewAndReset:
		MenuSetState();
		break;
	case APINotify_Open:
		MenuSetState();
		Do_ElementMonitor();
		break;
	case APINotify_PreSave:
		break;
	case APINotify_TempSave:
		break;
	case APINotify_Save:
		break;
	case APINotify_Close:
		break;
	case APINotify_Quit:
		break;
	case APINotify_SendChanges:
		break;
	case APINotify_ReceiveChanges:
		break;
	case APINotify_ChangeProjectDB:
		break;
	case APINotify_ChangeWindow:
		break;
	case APINotify_ChangeFloor:
		break;
	case APINotify_ChangeLibrary:
		break;
	default:
		break;
	}
	param = 1;
	return NoError;
}	// ProjectEventHandlerProc


static GSErrCode MenuCommandHandler (const API_MenuParams *menuParams)
{
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
					SyncAndMonAll();
					Do_ElementMonitor();
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
					err = ReNum_Selected();
					break;
				case Sum_CommandID:
					break;
				case Log_CommandID:
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
	GSErrCode err = ACAPI_Register_Menu(AddOnMenuID, 0, MenuCode_Tools, MenuFlag_Default);
	return err;
}

GSErrCode __ACENV_CALL Initialize (void)
{
	ACAPI_Notify_CatchProjectEvent(API_AllNotificationMask, ProjectEventHandlerProc);
	return ACAPI_Install_MenuHandler (AddOnMenuID, MenuCommandHandler);
}

GSErrCode __ACENV_CALL FreeData (void)
{
	return NoError;
}
