//------------ kuvbur 2022 ------------
#pragma once
#if !defined(ADDON_HPP)
    #define ADDON_HPP
    #include "Helpers.hpp"

    // Главный модуль add-on: регистрация интерфейса, обработка команд меню
    // и реакция на события проекта и элементов модели.
    #define Menu_MonAll 1
    #define Menu_SyncAll 2
    #define Menu_SyncSelect 3
    #define Menu_wallS 4
    #define Menu_widoS 5
    #define Menu_objS 6
    #define Menu_cwallS 7
    #define Menu_ReNum 8
    #define Menu_Sum 9
    #define Menu_RunParam 10
    #define Menu_Spec 11
    #define Menu_ShowSub 12
    #define Menu_SetRevision 13
    #define Menu_SetSub 14
    #define Menu_RoomBook 15
    #define Menu_AutoProfile 16
    #define Menu_AutoLay 17

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
static const Int32 ShowSub_CommandID = 12;
static const Int32 SetRevision_CommandID = 13;
static const Int32 SetSub_CommandID = 14;
static const Int32 RoomBook_CommandID = 15;
static const Int32 Auto3D_CommandID = 16;
static const Int32 AutoLay_CommandID = 17;

static const UInt32 MENU_ITEM_COUNT = 16;

    #if defined(AC_28) || defined(AC_29)
// Обработчик событий изменения элементов модели.
// Служит точкой входа для реакции на создание, изменение и удаление элементов.
GSErrCode ElementEventHandlerProc (const API_NotifyElementType *elemType);

// Обработчик событий проекта: открытие, закрытие, смена окна/этажа.
static GSErrCode ProjectEventHandlerProc (API_NotifyEventID notifID, Int32 param);

// Обработчик изменения выбора элементов.
static GSErrCode SelectionChangeHandlerProc (const API_Neig *selElemNeig);
    #else
// Обработчик событий изменения элементов модели.
GSErrCode __ACENV_CALL ElementEventHandlerProc (const API_NotifyElementType *elemType);

// Обработчик событий проекта: открытие, закрытие, смена окна/этажа.
static GSErrCode __ACENV_CALL ProjectEventHandlerProc (API_NotifyEventID notifID, Int32 param);

// Обработчик изменения выбора элементов.
static GSErrCode __ACENV_CALL SelectionChangeHandlerProc (const API_Neig *selElemNeig);
    #endif

// Включает или отключает наблюдение за изменениями элементов.
void Do_ElementMonitor (bool syncMon);

// Обновляет текст пунктов меню с учётом языка интерфейса.
void SetPaletteMenuText (short paletteItemInd);

// Приводит состояние пунктов меню в соответствие с настройками синхронизации.
void MenuSetState (SyncSettings &syncSettings);

// Маршрутизирует команды меню add-on по ID выбранного пункта.
static GSErrCode MenuCommandHandler (const API_MenuParams *menuParams);
#endif
