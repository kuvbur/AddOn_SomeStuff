# Sync — Синхронизация свойств

## Назначение
Главный модуль: синхронизация и мониторинг свойств между элементами по правилам, описанным в описаниях свойств (Sync_from/Sync_to). Ядро функциональности add-on.

## Файлы
- `Sync.cpp/hpp` — синхронизация и мониторинг

## Ключевые типы

| Тип | Описание |
|-----|----------|
| `SyncRule` | Одно правило: paramNameFrom, paramFrom, paramNameTo, paramTo, ignorevals, templatestring, synctype, syncdirection |
| `WriteData` | Данные для записи: guidTo, guidFrom, paramFrom, paramTo, ignorevals, formatstring, toSub, fromSub |
| `SkipValues` | Специальные значения для игнорирования: "empty", "trim_empty", "def" |
| `WriteDict` | `HashTable<API_Guid, Array<WriteData>>` — словарь правил по GUID |
| `ParsedPropertyCommand` | Парсинг описания: commandType ("Sync", "Renum", "Sum", "Spec"), fullCommand, parameters |

## Публичный API

### Синхронизация
| Функция | Назначение |
|---------|------------|
| `SyncAndMonAll` | Полная синхронизация + мониторинг |
| `SyncByType` | Синхронизация по типу элемента |
| `SyncElement` | Синхронизация одного элемента |
| `SyncSelected` | Синхронизация выбранных |
| `SyncArray` | Синхронизация массива GUID |
| `SyncData` | Синхронизация по описанию свойства |
| `SyncRelationsElement` | Синхронизация связанных элементов |
| `RunParamSelected` / `RunParam` | Запуск параметрических правил |

### Мониторинг
| Функция | Назначение |
|---------|------------|
| `MonAll` | Мониторинг всех активных типов |
| `MonByType` | Мониторинг по типу |

### Парсинг
| Функция | Назначение |
|---------|------------|
| `ParseSyncString` | Парсинг описания в массив WriteData |
| `SyncString` | Парсинг одной команды |
| `ParsePropertyDescription` | Парсинг полного описания |

### Связи и подэлементы
| Функция | Назначение |
|---------|------------|
| `SyncSetSubelement` | Связывание GUID подэлементов |
| `SyncSetSubelementScope` | Запись GUID связанных элементов |
| `SyncShowSubelement` | Подсветка связанных элементов |
| `SyncGetParentelement` | Родительские объекты по массиву |
| `SyncGetSubelement` | Дочерние объекты по массиву |
| `SyncGetSyncGUIDProperty` | Получение свойства Sync_GUID |

## Зависимости
- `Helpers.hpp` — GetSelectedElements, ParamHelpers
- `Propertycache.hpp` — PROPERTYCACHE()
- `CommonFunction.hpp` — msg_rep, GetTypeByGUID
- `DG.h`
- `dialogs/SyncSettings.hpp` — SyncSettings

## Зависимости (используется в)
- `BrowserPalette` — отображение правил синхронизации
- `SyncSettings.cpp` — настройки
- `CommonFunction` (через Helpers)

## Инварианты
- `SyncData` — основная функция синхронизации, рекурсивно обрабатывает подэлементы
- `IsElementThrottled` — throttling cache предотвращает повторную обработку одного элемента
- `SyncNeedResync` — проверка необходимости пересинхронизации (FIX: const& вместо копии)
- Запись в свойства идёт через `ACAPI_CallUndoableCommand` (один undo-region на действие)
- SyncSettings хранятся локально (SyncSettings.dat), НЕ в preferences проекта (ломает Teamwork)
