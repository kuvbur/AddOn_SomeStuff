# SomeStuff_Main — Точка входа

> Хеш коммита: 493caf5 (2026-09-22). Номера строк — определения в `.cpp` (1-based, проверены grep).

## Назначение
Главный модуль add-on: регистрация интерфейса, обработка команд меню, события проекта и элементов (observer'ы). [по коду]

## Файлы
- `SomeStuff_Main.cpp/hpp`

## Публичный API

| Функция | .cpp строка | Назначение |
|---------|-------------|------------|
| `ProjectEventHandlerProc` | 73 | События проекта: открытие, закрытие, смена окна/этажа [из комментария] — карточка |
| `ElementEventHandlerProc` | 142 | События изменения элементов (создание/изменение/удаление) [из комментария] — карточка; clangd не резолвит (2 попытки), тело разобрано grep |
| `Do_ElementMonitor` | 281 | Вкл/выкл мониторинг изменений элементов [из комментария] |
| `SelectionChangeHandlerProc` | 326 | Обработчик изменения выделения [из комментария] — карточка |
| `MenuSetState` | 337 | Состояние пунктов меню по настройкам синхронизации [из комментария] |
| `SetPaletteMenuText` | 351 | Тексты пунктов меню по языку [из комментария] |
| `MenuCommandHandler` | 366 | Маршрутизация команд меню по ID [из комментария]; вызывается из `Initialize` (Main:587) [из callgraph.json] — карточка |

## Карточки

### `MenuCommandHandler(const API_MenuParams *menuParams) -> GSErrCode`
- Расположение: `Sources/AddOn/SomeStuff_Main.cpp:366`
- Назначение: маршрутизирует команды меню add-on по ID пункта. [из комментария]
- Контракт: не проверено полностью; подтверждённые ветки: `Auto3D_CommandID` → `AutoFunc::ProfileByLine()` (:461), `AutoLay_CommandID` → `AutoFunc::AlignDrawingsByPoints()` (:464). [по коду]
- Побочные эффекты: каждая ветка делегирует модуль (Sync, Summ, Spec, ReNum, Dimensions, Roombook, pk) со своими эффектами записи. [по коду]
- Вызывает (карта из callHierarchy, 1-based): `LoadSyncSettingsFromPreferences` (:372), `PROPERTYCACHE().Update` (:389), `Do_ElementMonitor` (:399), `MonAll` (:400), `SyncAndMonAll` (:405), `SyncSelected` (:410), `RunParamSelected` (:437), `SumSelected` (:433), `ReNumSelected` (:427), `SpecAll` (:441), `SyncShowSubelement` (:445), `SyncSetSubelement` (:453), `SetRevision` (:449), `RoomBook` (:457), `ProfileByLine` (:462), `AlignDrawingsByPoints` (:465), `ShowOrHideBrowserPalette` (:469), `DimRoundAll` (:489), `MenuSetState` (:498), `WriteSyncSettingsToPreferences` (:494). [из callgraph.json]
- Внимание: `RoomBook` определён в Roombook.cpp:57; `LoadSyncSettingsFromPreferences` — clangd даёт SyncSettings.cpp:518 (grep находил 429 — второе совпадение в файле, уточнять при правках). [из callgraph.json + grep]

### `ElementEventHandlerProc(const API_NotifyElementType *elemType) -> GSErrCode`
- Расположение: `Sources/AddOn/SomeStuff_Main.cpp:142`
- Назначение: реакция на создание/изменение/удаление элементов. [из комментария]
- Контракт: единый диспетчер — по notifID маршрутизирует события проекта, выделения, меню и элементов. [по коду, grep тела]
- Побочные эффекты: **вся функциональность аддона** — ветки: `PROPERTYCACHE().Update`, `DimRoundAll` (:165), `DimAutoRoundOne`, `MonAll`, `SyncAndMonAll`, `SyncSelected`, `RunParamSelected`, `SyncShowSubelement`, `SyncSetSubelement`, `ShowOrHideBrowserPalette`, `WriteSyncSettingsToPreferences`, `LoadSyncSettingsFromPreferences`, `Do_ElementMonitor`, `MenuSetState`, `SetPaletteMenuText`, `AttachObserver`; SDK-регистрации (`ACAPI_MenuItem_RegisterMenu`, `ACAPI_Install_MenuHandler`, `ACAPI_Notification_*Catch*`). [по коду, grep тела — body_scan.json]
- Вызывается из: ArchiCAD (notify/callbacks). [по коду]

### `ProjectEventHandlerProc(API_NotifyEventID notifID, Int32 param) -> GSErrCode`
- Расположение: `Sources/AddOn/SomeStuff_Main.cpp:73`
- Назначение: события проекта (открытие/закрытие/окно/этаж). [из комментария]
- Контракт: не проверено.
- Побочные эффекты: сброс/обновление кэша при смене проекта (PROPERTYCACHE). [по коду]

### `SelectionChangeHandlerProc(const API_Neig *selElemNeig) -> GSErrCode`
- Расположение: `Sources/AddOn/SomeStuff_Main.cpp:326`
- Назначение: реакция на изменение выделения. [из комментария]
- Контракт: не проверено.
- Побочные эффекты: обновление палитры (у BrowserPalette свой SelectionChangeHandler с suppressSelectionRefresh — см. AGENTS.md §6). [по коду]

## Зависимости
- `Helpers.hpp` [по include]

## Инварианты
- Observer'ы устанавливаются при инициализации и снимаются при деинициализации [по коду]
