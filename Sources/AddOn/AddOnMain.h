void	Do_ElementMonitor();
GSErrCode __ACENV_CALL	ElementEventHandlerProc(const API_NotifyElementType* elemType);
static GSErrCode __ACENV_CALL    ProjectEventHandlerProc(API_NotifyEventID notifID, Int32 param);
static GSErrCode MenuCommandHandler(const API_MenuParams* menuParams);