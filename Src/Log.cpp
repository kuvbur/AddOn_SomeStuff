#include	"APIEnvir.h"
#include	"ACAPinc.h"
#include "Log.hpp"
#include "Helpers.hpp"

//// Create an own structure which contains all the necessary informations what you want to store in element's user data 
//typedef struct {
//	Int32 myNumbers[5];
//	char timeStr[128];
//} LogData;
//
//// ----------------------------------------------------------------------------- 
//bool SetElementUserData(const API_Guid& guid, const API_ElemTypeID& typeID) // Add extra parameters if needed 
//{
//	API_ElementUserData	userData;
//	LogData** ulogData = NULL;
//	API_Elem_Head		elemHead;
//	GSErrCode			err = NoError;
//
//	BNZeroMemory(&userData, sizeof(API_ElementUserData));
//	userData.dataVersion = 1;
//	userData.platformSign = GS::Act_Platform_Sign;
//	// Set flags if you need: 
//	//  userData.flags = APIUserDataFlag_FillWith | APIUserDataFlag_Pickup | APIUserDataFlag_UndoAble | APIUserDataFlag_SkipRecalcAndDraw; 
//	userData.dataHdl = BMAllocateHandle(sizeof(LogData), ALLOCATE_CLEAR, 0);
//	if (userData.dataHdl == NULL)
//		return false;
//
//	myUserData = (LogData**)(userData.dataHdl);
//	// Fill the allocated userdata as you want: 
//	for (Int32 ii = 0; ii < 5; ++ii) {
//		(*ulogData)->myNumbers[ii] = ii + 1;
//	}
//	char ttimeStr[128];
//	TIGetTimeString(TIGetTime(), ttimeStr, TI_SHORT_DATE_FORMAT | TI_SHORT_TIME_FORMAT);
//	sprintf((*ulogData)->timeStr, ttimeStr);
//
//
//	// Attach userdata to the element: 
//	BNZeroMemory(&elemHead, sizeof(API_Elem_Head));
//	elemHead.guid = guid;
//	elemHead.typeID = typeID;
//	err = ACAPI_Element_SetUserData(&elemHead, &userData);
//	if (userData.dataHdl != NULL)
//		BMKillHandle(&userData.dataHdl);
//
//	return err == NoError ? true : false;
//}
//
//// ----------------------------------------------------------------------------- 
//bool GetElementUserData(const API_Guid& guid, const API_ElemTypeID& typeID, LogData** ulogData)
//{
//	API_ElementUserData	userData;
//	API_Elem_Head		elemHead;
//	GSErrCode			err = NoError;
//
//	BNZeroMemory(&userData, sizeof(API_ElementUserData));
//	// Get userdata from the element: 
//	BNZeroMemory(&elemHead, sizeof(API_Elem_Head));
//	elemHead.guid = guid;
//	elemHead.typeID = typeID;
//	err = ACAPI_Element_GetUserData(&elemHead, &userData);
//	if (err != NoError || userData.dataVersion != 1) {
//		if (userData.dataHdl != NULL)
//			BMKillHandle(&userData.dataHdl);
//		return false;
//	}
//	ulogData = (LogData**)(userData.dataHdl);
//	return true;
//}
//
//// ----------------------------------------------------------------------------- 
//// Usage example: 
//bool result = false;
//result = SetElementUserData(myGuid, myType);
//
//// ... 
//
//MyElementUserData** myUserData = NULL;
//result = GetElementUserData(myGuid, myType, myUserData);
//
//if (result) {
//	// read the userdata 
//	Int32 myFirstNumber = (*myUserData)->myNumbers[0];
//	msg_rep("LOG", dataStr, NoError, elemGuid);
//}
//
//
//// Don't forget to release the handle after using! 
//if (myUserData != NULL)
//BMKillHandle((GSHandle*)&myUserData);

void Do_ShowSelElems(const API_Guid& elemGuid) {
	GSErrCode			err;
	API_Elem_Head		elemHead;
	API_ElementUserData userData;

	BNZeroMemory(&elemHead, sizeof(API_Elem_Head));
	elemHead.guid = elemGuid;
	err = ACAPI_Element_GetHeader(&elemHead);
	BNZeroMemory(&userData, sizeof(API_ElementUserData));
	err = ACAPI_Element_GetUserData(&elemHead, &userData);
	if (err == NoError) {
		char	dataStr[256];
		CHTruncate(*userData.dataHdl, dataStr, BMGetHandleSize(userData.dataHdl));
		msg_rep("LOG", dataStr, NoError, elemGuid);
		BMKillHandle(&userData.dataHdl);
	}
}

void Do_MarkSelElems(const API_Guid& elemGuid)
{
	GSErrCode			err;
	API_Elem_Head		elemHead;
	API_ElementUserData userData;

	BNZeroMemory(&elemHead, sizeof(API_Elem_Head));
	elemHead.guid = elemGuid;
	err = ACAPI_Element_GetHeader(&elemHead);
	BNZeroMemory(&userData, sizeof(API_ElementUserData));

	userData.dataVersion = 1;
	userData.platformSign = GS::Act_Platform_Sign;
	userData.dataHdl = BMAllocateHandle(0, ALLOCATE_CLEAR, 0);
	if (userData.dataHdl == NULL)
		return;

	char	timeStr[128];
	TIGetTimeString(TIGetTime(), timeStr, TI_SHORT_DATE_FORMAT | TI_SHORT_TIME_FORMAT);

	//SpeedTest
	TIReset(0, "ACAPI_Element_SetUserData - SpeedTest");
	TIStart(0);
	Int32 dataLen = Strlen32(timeStr) + 1;
	userData.dataHdl = BMReallocHandle(userData.dataHdl, dataLen, REALLOC_FULLCLEAR, 0);
	if (userData.dataHdl != NULL) {					// error handling...
		CHCopyC(timeStr, *userData.dataHdl);
		userData.flags = APIUserDataFlag_FillWith | APIUserDataFlag_Pickup;
		err = ACAPI_Element_SetUserData(&elemHead, &userData);
	}
	//SpeedTest
	TIStop(0);
	TIPrintTimers();

	BMKillHandle(&userData.dataHdl);
}		// Do_MarkSelElems
