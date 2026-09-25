# SomeStuff_Main — Точка входа

> Базовый хеш: 493caf5 (2026-09-22); номера строк — ревизия базы, в рабочем дереве `llm_test` (HEAD `13c1948`) смещены (например, `Initialize` — 544). Номера строк — определения в `.cpp` (1-based, проверены grep).

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
| `MenuCommandHandler` | 371 | Маршрутизация команд меню по ID [из комментария]; устанавливается из `Initialize` через `ACAPI_Install_MenuHandler` (Main:588) [по коду] — карточка |
| `RegisterInterface` | 525 | Регистрация интерфейса/меню/палитры/навитора (включая `TablesNavigator::RegisterInterface` :537) [по коду] |
| `Initialize` | 544 | Инициализация после открытия проекта: настройки, мониторинг, JSON-команды, палитра, навигатор, menu-handler [по коду] — карточка |
| `FreeData` | 594 | Освобождение при выгрузке (сейчас только `NoError`; AC28 — 592) [по коду] |

## Карточки

### `Initialize() -> GSErrCode`
- Расположение: `Sources/AddOn/SomeStuff_Main.cpp:544` (AC25; AC28 — 542)
- Назначение: инициализация после открытия проекта: настройки, мониторинг, JSON-команды, палитра, навигатор, установка menu-handler. [по коду]
- Контракт: при ошибке `TablesNavigator::Initialize` — возврат её кода; `TestFunc::Test()` — только под `TESTING`; `ACAPI_KeepInMemory(true)`. [по коду]
- Побочные эффекты: `LoadSyncSettingsFromPreferences(syncSettings, true)` (:550, forceReload); `MenuSetState` (:553); `Do_ElementMonitor` (:554); `MonAll` (:555); `RegisterJsonCommands` (:569, AC25+); `BrowserPalette::RegisterPaletteControlCallBack` (:571); `CatchSelectionChange` (:574/576); `InstallMenuHandler` (:586/588). [по коду]
- Вызывается из: ArchiCAD (лifecycle). [по коду]

### `MenuCommandHandler(const API_MenuParams *menuParams) -> GSErrCode`
- Расположение: `Sources/AddOn/SomeStuff_Main.cpp:371` (рабочее дерево `llm_test`; ревизия базы — :366)
- Назначение: маршрутизирует команды меню add-on по ID пункта. [из комментария]
- Контракт: не проверено полностью; подтверждённые ветки: `Auto3D_CommandID` → `AutoFunc::ProfileByLine()` (:467), `AutoLay_CommandID` → `AutoFunc::AlignDrawingsByPoints()` (:470). [по коду, рабочее дерево `llm_test`]
- Побочные эффекты: каждая ветка делегирует модуль (Sync, Summ, Spec, ReNum, Dimensions, Roombook, pk) со своими эффектами записи. [по коду]
- Вызывает (grep рабочего дерева `llm_test`, 1-based): `LoadSyncSettingsFromPreferences` (:377), `PROPERTYCACHE().Update` (:394), `Do_ElementMonitor` (:404), `MonAll` (:405), `SyncAndMonAll` (:410), `SyncSelected` (:415), `ReNumSelected` (:432), `SumSelected` (:438), `RunParamSelected` (:442), `Spec::SpecAll` (:446), `SyncShowSubelement` (:450), `Revision::SetRevision` (:454), `SyncSetSubelement` (:458), `Roombook::RoomBook` (:462), `ProfileByLine` (:467), `AlignDrawingsByPoints` (:470), `ShowOrHideBrowserPalette` (:474), `DimRoundAll` (:493), `WriteSyncSettingsToPreferences` (:498), `MenuSetState` (:500). [по коду]
- Внимание: `Roombook::RoomBook` определён в Roombook.cpp:674 (AC25; AC22–23 — заглушка :23); `LoadSyncSettingsFromPreferences` — SyncSettings.cpp:432. [grep]

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
