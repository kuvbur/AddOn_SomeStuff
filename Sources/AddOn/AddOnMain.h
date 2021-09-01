#define ELEMSTR_LEN				256
bool	GetElementTypeString(API_ElemTypeID typeID, char* elemStr);
void	Do_ElementMonitor(bool switchOn);
GSErrCode __ACENV_CALL	ElementEventHandlerProc(const API_NotifyElementType* elemType);
