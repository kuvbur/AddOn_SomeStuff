#include	"APIEnvir.h"
#include	"ACAPinc.h"
#include	"Log.hpp"
#include	"Helpers.hpp"
void LogGetEmpty(LogData& logData) {
	logData.change.current.isempty = true;
	logData.change.isempty = true;
	logData.change.old_1 = logData.change.current;
	logData.change.old_2 = logData.change.current;
	logData.create = logData.change;
	logData.edit = logData.change;
	logData.classification = logData.change;
	logData.property = logData.change;
}

GSErrCode LogWriteElement(const API_Guid& guid, const LogData& logData)
{
	API_Elem_Head element = {};
	element.guid = guid;
	API_ElementUserData userData = {};
	BNZeroMemory(&userData, sizeof(userData));
	userData.dataVersion = 1;
	userData.platformSign = GS::Act_Platform_Sign;
	userData.dataVersion = 2;
	userData.dataHdl = BMAllocateHandle(sizeof(LogData), ALLOCATE_CLEAR, 0);
	*reinterpret_cast<LogData*> (*userData.dataHdl) = logData;
	GSErrCode err = ACAPI_Element_SetUserData(&element, &userData);
	BMKillHandle(&userData.dataHdl);
	return err;
}

GSErrCode LogReadElement(const API_Guid& guid, LogData& logData)
{
	GSErrCode err = NoError;
	API_Elem_Head element = {};
	element.guid = guid;
	API_ElementUserData userData = {};
	BNZeroMemory(&userData, sizeof(userData));
	userData.dataHdl = BMAllocateHandle(sizeof(LogData), ALLOCATE_CLEAR, 0);
	userData.platformSign = GS::Act_Platform_Sign;
	if (userData.dataHdl != nullptr) {
		err = ACAPI_Element_GetUserData(&element, &userData);
		if (err == NoError && userData.dataHdl != nullptr && userData.platformSign == GS::Act_Platform_Sign && userData.dataVersion == 2)
			logData = *reinterpret_cast<LogData*> (*userData.dataHdl);
	}
	BMKillHandle(&userData.dataHdl);
	return err;
}

void LogRow(lgL3& record, const GS::HashTable<short, API_UserInfo>& userInfoTable) {
	GS::UniString actionByUserStr = GS::UniString::Printf("      %s", record.time);
	if (userInfoTable.ContainsKey(record.userId)) {
		actionByUserStr.Append(" by ");
		actionByUserStr.Append(userInfoTable[record.userId].fullName);
	}
	ACAPI_WriteReport(actionByUserStr, false);
}

void LogRow(lgL2& record, const GS::HashTable<short, API_UserInfo>& userInfoTable) {
	if (!record.current.isempty) {
		LogRow(record.current, userInfoTable);
	}
	if (!record.old_1.isempty) {
		LogRow(record.old_1, userInfoTable);
	}
	if (!record.old_2.isempty) {
		LogRow(record.old_2, userInfoTable);
	}
}

void LogShowElement(const API_Guid& elemGuid, const GS::HashTable<short, API_UserInfo>& userInfoTable) {
	LogData logData;
	LogGetEmpty(logData);
	if (LogReadElement(elemGuid, logData) == NoError) {;
		GS::UniString msg = "GUID: " + APIGuid2GSGuid(elemGuid).ToUniString();
		char    elemStr[32];
		API_ElemTypeID elementType;
		GSErrCode err = GetTypeByGUID(elemGuid, elementType);
		if (err == NoError) {
			if (GetElementTypeString(elementType, elemStr)) {
				msg.Append(GS::UniString::Printf(" (%s)", elemStr));
			}
		}
		ACAPI_WriteReport(msg, false);
		if (!logData.create.isempty) {
			ACAPI_WriteReport("   Create", false);
			LogRow(logData.create, userInfoTable);
		}
		if (!logData.edit.isempty) {
			ACAPI_WriteReport("   Edit", false);
			LogRow(logData.edit, userInfoTable);
		}
		if (!logData.change.isempty) {
			ACAPI_WriteReport("   Change", false);
			LogRow(logData.change, userInfoTable);
		}
		if (!logData.classification.isempty) {
			ACAPI_WriteReport("   Change Classification", false);
			LogRow(logData.classification, userInfoTable);
		}
		if (!logData.property.isempty) {
			ACAPI_WriteReport("   Change Property", false);
			LogRow(logData.property, userInfoTable);
		}
		ACAPI_WriteReport("---------------------------------", false);
	}
}

void LogShowSelected() {
	GS::HashTable<short, API_UserInfo> userInfoTable;
	GetTeamworkMembers(userInfoTable);
	GS::Array<API_Guid> guidArray = GetSelectedElements(true, true);
	if (!guidArray.IsEmpty()) {
		for (UInt32 i = 0; i < guidArray.GetSize(); i++) {
			LogShowElement(guidArray[i], userInfoTable);
		}
	}
}

//void LogDataDelete(const API_Guid& guid, const LogData& logData) {
//	return;
//}

void LogDataRotate(lgL2 &partlogData) {
	char	timeStr[128];
	TIGetTimeString(TIGetTime(), timeStr, TI_SHORT_DATE_FORMAT | TI_SHORT_TIME_FORMAT);
	bool isteamwork = false;
	short userid = 0;
	IsTeamwork(isteamwork, userid);
	partlogData.old_2 = partlogData.old_1;
	partlogData.old_1 = partlogData.current;
	CHCopyC(timeStr, partlogData.current.time);
	partlogData.current.userId = userid;
	partlogData.current.isempty = false;
}

void LogWriteElement(const API_NotifyElementType *elemType) {
	if (!IsElementEditable(elemType->elemHead.guid))
		return;
	LogData logData;
	if (LogReadElement(elemType->elemHead.guid, logData) != NoError) LogGetEmpty(logData);
	switch (elemType->notifID) {
		case APINotifyElement_New:
			LogDataRotate(logData.create);
			logData.create.isempty = false;
			break;
		case APINotifyElement_Copy:
			LogDataRotate(logData.create);
			logData.create.isempty = false;
			break;
		case APINotifyElement_Change:
			LogDataRotate(logData.change);
			logData.change.isempty = false;
			break;
		case APINotifyElement_Edit:
			LogDataRotate(logData.edit);
			logData.edit.isempty = false;
			break;
		case APINotifyElement_ClassificationChange:
			LogDataRotate(logData.classification);
			logData.classification.isempty = false;
			break;
		case APINotifyElement_PropertyValueChange:
			LogDataRotate(logData.property);
			logData.property.isempty = false;
			break;
		// TODO При удалении элемента выводить в примечания и заметки сообщение с именем пользователя
		//case APINotifyElement_Delete:
		//	LogDataDelete(elemType->elemHead.guid, logData);
		//	break;
		default:
			break;
	}
	LogWriteElement(elemType->elemHead.guid, logData);
	return;
}

// -----------------------------------------------------------------------------
// GetTeamworkMembers
//  collects information of joined Teamwork members
// -----------------------------------------------------------------------------
static GSErrCode	GetTeamworkMembers(GS::HashTable<short, API_UserInfo>& userInfoTable)
{
	API_SharingInfo	sharingInfo;
	BNZeroMemory(&sharingInfo, sizeof(API_SharingInfo));
	GSErrCode err = ACAPI_Environment(APIEnv_ProjectSharingID, &sharingInfo);
	if (err == NoError && sharingInfo.users != nullptr) {
		for (Int32 i = 0; i < sharingInfo.nUsers; i++)
			userInfoTable.Add(((*sharingInfo.users)[i]).userId, (*sharingInfo.users)[i]);
	}
	if (sharingInfo.users != nullptr)
		BMhKill(reinterpret_cast<GSHandle*>(&sharingInfo.users));
	return err;
}		/* GetTeamworkMembers */
