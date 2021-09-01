#define ELEMSTR_LEN				256
bool	GetElementTypeString(API_ElemTypeID typeID, char* elemStr);
void	Do_ElementMonitor(bool switchOn);
bool SyncParamAndProp(const API_Guid& elemGuid, API_Property& property);
GSErrCode __ACENV_CALL	ElementEventHandlerProc(const API_NotifyElementType* elemType);
static void SyncData(const API_Guid& elemGuid);
bool GetLibParam(const API_Guid& elemGuid, const GS::UniString& paramName, API_AddParType& nthParameter);
void CallOnSelectedElem(void (*function)(const API_Guid&), bool assertIfNoSel = true, bool onlyEditable = true);