#pragma once
#if !defined (ADDON_HPP)
#define	ADDON_HPP
#include	"Helpers.hpp"
GSErrCode __ACENV_CALL	ElementEventHandlerProc(const API_NotifyElementType* elemType);
GSErrCode Do_Sync(const API_Guid& objectId, SyncSettings& syncSettings);
static GSErrCode __ACENV_CALL    ProjectEventHandlerProc(API_NotifyEventID notifID, Int32 param);
void Do_ElementMonitor(bool& syncMon);
static GSErrCode MenuCommandHandler(const API_MenuParams* menuParams);
#endif