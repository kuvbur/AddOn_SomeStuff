# Sync — Синхронизация свойств

> Хеш коммита: 493caf5 (2026-09-22). Номера строк — определения в `.cpp` (1-based, проверены grep).

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
| `MonAll` | 64 | Мониторинг для всех активных типов [из комментария] |
| `MonByType` | 117 | Мониторинг для заданного типа [из комментария] |
| `SyncAndMonAll` | 170 | Полная синхронизация + мониторинг [из комментария] |
| `SyncByType` | 306 | Синхронизация элементов выбранного типа [из комментария] |
| `SyncElement` | 383/391 | Синхронизация одного элемента и подэлементов [из комментария] |
| `SyncSelected` | 435 | Синхронизация выбранных [из комментария] |
| `SyncArray` | 456 | Синхронизация массива, возвращает обработанные GUID [из комментария] |
| `RunParamSelected` / `RunParam` | 536 / 584 | Параметрические правила [из комментария] |
| `SyncRelationsElement` | 641 | Синхронизация связанных элементов [из комментария] |
| `SyncData` | 669 | Синхронизация по описаниям свойств [из комментария] |
| `ParseSyncString` | 1088 | Парсинг описания в WriteData [из комментария] |
| `SyncString` | 1445 | Парсинг одной команды [из комментария] |
| `SyncSetSubelement` | 2163 | Запись GUID подэлементов в основной элемент [из комментария] |
| `SyncSetSubelementScope` | 2258 | Запись GUID (для CallUndoableCommand) [из комментария] |
| `SyncShowSubelement` | 2316 | Подсветка элементов по Sync_GUID [из комментария] |
| `SyncGetParentelement` / `SyncGetSubelement` | 2531 / 2649 | Словари родительских/дочерних GUID [из комментария] |
| `ParsePropertyDescription` | 2750 | Парсинг полного описания (все команды) [из комментария] |

## Карточки

### `SyncAndMonAll(SyncSettings &syncSettings)`
- Расположение: `Sources/AddOn/Sync.cpp:170`
- Назначение: полная синхронизация и мониторинг для активных элементов. [из комментария]
- Контракт: не проверено.
- Побочные эффекты: **запись свойств элементов** (SyncByType-цепочка); подключение мониторинга. [по коду]
- Вызывается из: `MenuCommandHandler` [по коду; строка не проверена]

### `SyncData(const API_Guid &elemGuid, ..., ParamDictElement &paramToWrite, int dummymode) -> bool`
- Расположение: `Sources/AddOn/Sync.cpp:669`
- Назначение: применяет правила синхронизации к элементу и подэлементам. [из комментария]
- Контракт: не проверено.
- Побочные эффекты: **запись значений в свойства/параметры/GDL** целевых элементов; рекурсия по подэлементам; dummymode — dry-run без записи. [по коду]
- Вызывает: `ParseSyncString`, `SyncAddRule`, `SyncAddSubelement`, `SyncCalcRule`, `SyncNeedResync` [по коду; detail не проверено]

### `SyncElement(const API_Guid &elemGuid, const SyncSettings &syncSettings, ParamDictElement &paramToWrite, int dummymode) -> bool`
- Расположение: `Sources/AddOn/Sync.cpp:383` (перегрузка с UnicGuid — :391)
- Назначение: синхронизация одного элемента и его подэлементов. [из комментария]
- Контракт: не проверено.
- Побочные эффекты: **запись свойств**; throttling через `IsElementThrottled`. [по коду]
- Вызывает: `SyncData`, `IsElementThrottled` [по коду; detail не проверено]

## Зависимости
- `Helpers.hpp`, `Propertycache.hpp`, `CommonFunction.hpp`, `DG.h`, `dialogs/SyncSettings.hpp` [по include]

## Зависимости (используется в)
- `BrowserPalette`, `SyncSettings.cpp`, `SomeStuff_Main` [по коду]

## Инварианты
- Throttling предотвращает повторную обработку [из комментария, Sync.hpp:62]
- Спец-значения игнорирования: "empty", "trim_empty", "def" [из комментария, Sync.hpp:52-56]
