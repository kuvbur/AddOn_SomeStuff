//------------ kuvbur 2022 ------------
#pragma once
#if !defined (ADDON_HPP)
#define	ADDON_HPP
#include "Helpers.hpp"
#define	 Menu_MonAll		1
#define	 Menu_SyncAll		2
#define	 Menu_SyncSelect	3
#define	 Menu_wallS			4
#define	 Menu_widoS			5
#define	 Menu_objS			6
#define	 Menu_cwallS		7
#define	 Menu_ReNum			8
#define	 Menu_Sum			9
#define	 Menu_RunParam		10
#define	 Menu_Spec		11
#define	 Menu_AutoList		12
#define	 Menu_AutoProfile	13
#define	 Menu_AutoLay	14

static const Int32 MonAll_CommandID = 1;
static const Int32 SyncAll_CommandID = 2;
static const Int32 SyncSelect_CommandID = 3;
static const Int32 wallS_CommandID = 4;
static const Int32 widoS_CommandID = 5;
static const Int32 objS_CommandID = 6;
static const Int32 cwallS_CommandID = 7;
static const Int32 ReNum_CommandID = 8;
static const Int32 Sum_CommandID = 9;
static const Int32 RunParam_CommandID = 10;
static const Int32 Spec_CommandID = 11;
static const Int32 AutoList_CommandID = 12;
static const Int32 Auto3D_CommandID = 13;
static const Int32 AutoLay_CommandID = 14;

#if defined(AC_28)
GSErrCode ElementEventHandlerProc (const API_NotifyElementType* elemType);

static GSErrCode ProjectEventHandlerProc (API_NotifyEventID notifID, Int32 param);
#else
GSErrCode __ACENV_CALL	ElementEventHandlerProc (const API_NotifyElementType* elemType);

static GSErrCode __ACENV_CALL ProjectEventHandlerProc (API_NotifyEventID notifID, Int32 param);
#endif

void Do_ElementMonitor (bool& syncMon);

void SetPaletteMenuText (short paletteItemInd);

void MenuSetState (SyncSettings& syncSettings);

static GSErrCode MenuCommandHandler (const API_MenuParams* menuParams);
#endif
