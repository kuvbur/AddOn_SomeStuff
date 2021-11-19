#pragma once
#if !defined (ADDON_HPP)
#define	ADDON_HPP
GSErrCode __ACENV_CALL	ElementEventHandlerProc(const API_NotifyElementType* elemType);
static GSErrCode __ACENV_CALL    ProjectEventHandlerProc(API_NotifyEventID notifID, Int32 param);
void Do_ElementMonitor(bool& syncMon);
static GSErrCode MenuCommandHandler(const API_MenuParams* menuParams);
#endif