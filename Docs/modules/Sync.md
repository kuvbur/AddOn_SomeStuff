# Sync — Синхронизация свойств

> Хеш коммита: 3a24131 (2026-09-22). Номера строк — определения в `.cpp` (1-based, проверены grep). CallHierarchy собран для 6 ключевых функций (0-based у clangd; в таблицах 1-based).

## Назначение
Главный модуль: синхронизация и мониторинг свойств между элементами по правилам из описаний свойств (Sync_from/Sync_to). Ядро add-on. [по коду]

## Файлы
- `Sync.cpp/hpp`

## Ключевые типы

| Тип | Описание |
|-----|----------|
| `SyncRule` | paramNameFrom/paramFrom, paramNameTo/paramTo, ignorevals, templatestring, synctype, syncdirection [из комментария, Sync.hpp:13-30] |
| `WriteData` | guidTo, guidFrom, paramFrom, paramTo, ignorevals, formatstring, toSub, fromSub [из комментария] |
| `ParsedPropertyCommand` | commandType ("Sync", "Renum_flag", "Renum", "Sum", "Spec_rule"), fullCommand, parameters, isValid, errorMessage [из комментария, Sync.hpp:205-211] |

## Публичный API

| Функция | .cpp строка | Назначение |
|---------|-------------|------------|
| `MonAll` | 64 | Мониторинг для всех активных типов [из комментария]; вызывается из `Initialize` (Main:555) и `MenuCommandHandler` (Main:400) [из callgraph.json] |
| `MonByType` | 117 | Мониторинг для заданного типа [из комментария] |
| `SyncAndMonAll` | 170 | Полная синхронизация + мониторинг [из комментария] |
| `SyncByType` | 306 | Синхронизация элементов выбранного типа [из комментария]; вызывается из `SyncAndMonAll` ×5 [из callgraph.json] |
| `SyncElement` | 383/391 | Синхронизация одного элемента и подэлементов [из комментария] |
| `SyncSelected` | 435 | Синхронизация выбранных [из комментария]; вызывается из SomeStuff_Main.cpp:410 и внутри Sync.cpp:565/2246 [по коду, grep] |
| `SyncArray` | 456 | Синхронизация массива, возвращает обработанные GUID [из комментария] |
| `RunParamSelected` / `RunParam` | 536 / 584 | Параметрические правила [из комментария]; RunParamSelected — из SomeStuff_Main.cpp:437 [по коду, grep] |
| `SyncRelationsElement` | 641 | Синхронизация связанных элементов [из комментария]; вызывается из `SyncElement` (:416) [из callgraph.json] |
| `SyncData` | 669 | Синхронизация по описаниям свойств [из комментария] |
| `ParseSyncString` | 1088 | Парсинг описания в WriteData [из комментария] |
| `SyncString` | 1445 | Парсинг одной команды [из комментария] |
| `SyncSetSubelement` | 2163 | Запись GUID подэлементов в основной элемент [из комментария]; вызывается из SomeStuff_Main.cpp:453 [по коду, grep] |
| `SyncSetSubelementScope` | 2258 | Запись GUID (для CallUndoableCommand) [из комментария] |
| `SyncShowSubelement` | 2316 | Подсветка элементов по Sync_GUID [из комментария]; вызывается из SomeStuff_Main.cpp:445 [по коду, grep] |
| `SyncGetParentelement` / `SyncGetSubelement` | 2531 / 2649 | Словари родительских/дочерних GUID [из комментария] |
| `ParsePropertyDescription` | 2750 | Парсинг полного описания (все команды) [из комментария] |

## Карточки

### `SyncAndMonAll(SyncSettings &syncSettings)`
- Расположение: `Sources/AddOn/Sync.cpp:170`
- Назначение: полная синхронизация и мониторинг для активных элементов. [из комментария]
- Контракт: перед обходом типов — `ResetProperty()`: при успехе сброса ранний выход (Sync.cpp:173-174). [по коду]
- Побочные эффекты: **запись свойств элементов** (SyncByType-цепочка: wallS/widoS/objS/cwallS); подключение мониторинга; `ElementsWrite`/`WriteInfo` (Helpers); информирование о непрочитанных GDL (`CountUnreadGDLParams`). [по коду]
- Вызывает: `SyncByType` ×5 (:191/202/212/222/229), `SyncArray` (:296), `ResetProperty` (:174), `ElementsWrite`, `WriteInfo`, `IsDummyModeOn`, `CountUnreadGDLParams`, `isEng`, `Get*S` (SyncSettings), `msg_rep`. [из callgraph.json]
- Вызывается из: `MenuCommandHandler` (SomeStuff_Main.cpp:405). [из callgraph.json]

### `SyncData(const API_Guid &elemGuid, ..., ParamDictElement &paramToWrite, int dummymode) -> bool`
- Расположение: `Sources/AddOn/Sync.cpp:669`
- Назначение: применяет правила синхронизации к элементу и подэлементам. [из комментария]
- Контракт: не редактируемый элемент пропускается (`IsElementEditable`, :683); dummy-режим — ранний выход (:700). [по коду]
- Побочные эффекты: **запись значений в свойства/параметры/GDL** целевых элементов (через SyncCalcRule → ElementsWrite-цепочку); может назначить автокласс (`SetAutoclass`, :689). [по коду]
- Вызывает: `ParseSyncString` (:729), `SyncAddSubelement` (:746), `SyncCalcRule` (:754/759), `SyncNeedResync` (:760), `SetAutoclass`, `ElementsRead`, `CompareParamDictElement`, `GetElemState`/`Reverse`, `IsElementEditable`, `SubGuid_GetParamValue`, `AddPropertyDefinition`. [из callgraph.json]

### `SyncElement(const API_Guid &elemGuid, const SyncSettings &syncSettings, ParamDictElement &paramToWrite, int dummymode) -> bool`
- Расположение: `Sources/AddOn/Sync.cpp:383` (перегрузка с UnicGuid — :391)
- Назначение: синхронизация одного элемента и его подэлементов. [из комментария]
- Контракт: дедупликация через `ContainsKey` по словарям (guid уже в worklist — пропуск); рекурсия: перегрузка вызывает основную (:388). [по коду]
- Побочные эффекты: **запись свойств** (через SyncData); трогает связанные элементы (`SyncRelationsElement`, :416). [по коду]
- Вызывает: `SyncData` (:411/425), `SyncRelationsElement` (:416), `GetTypeByGUID` (:399), `GetParentGUIDSectElem` (:404), `GetRelationsElement` (:410). [из callgraph.json]
- Вызывается из: `SyncArray` (:479), `SyncByType` (:352), перегрузка `SyncElement` (:388). [из callgraph.json]

## Зависимости
- `Helpers.hpp`, `Propertycache.hpp`, `CommonFunction.hpp`, `DG.h`, `dialogs/SyncSettings.hpp` [по include]

## Зависимости (используется в)
- `BrowserPalette`, `SyncSettings.cpp`, `SomeStuff_Main` [по коду]

## Инварианты
- Throttling предотвращает повторную обработку [из комментария, Sync.hpp:62]
- Спец-значения игнорирования: "empty", "trim_empty", "def" [из комментария, Sync.hpp:52-56]
