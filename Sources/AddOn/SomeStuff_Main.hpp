//------------ kuvbur 2022 ------------
#pragma once
#if !defined(ADDON_HPP)
    #define ADDON_HPP
    #include "Helpers.hpp"

// Главный модуль add-on: регистрация интерфейса, обработка команд меню
// и реакция на события проекта и элементов модели.

    #ifdef ServerMainVers_2800
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
