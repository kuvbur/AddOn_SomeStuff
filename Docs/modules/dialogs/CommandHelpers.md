# dialogs/CommandHelpers — Помощники команд

## Назначение
Парсинг описаний свойств для интерфейса: извлечение правил синхронизации и прочих команд для отображения/редактирования в BrowserPalette.

## Файлы
- `dialogs/CommandHelpers.cpp/hpp` — парсинг команд

## Ключевые типы

| Тип | Описание |
|-----|----------|
| `SyncRuleInfo` | Правило синхронизации: commandType, fullCommand, parameters, sourceType, sourceName, targetType, targetName, formatString, ignoreVals, isValid, errorMessage, hasSub, hasGUID |
| `ParsePropertyResult` | Результат парсинга: syncRules, otherCommands, remainingText, hasSyncRules, hasOtherCommands |

## commandType значения
- `Sync_from`, `Sync_to` — обычная синхронизация
- `Sync_from_sub`, `Sync_to_sub` — с подэлементами
- `Sync_from_GUID`, `Sync_to_GUID` — с другой позиции по GUID

## sourceType значения
Property, GDL, Coord, Formula, ID, Material, File, Classification, Morph, Info, IFC, Glob, Class, Attrib, Listdata, Element, MEP

## Публичный API

| Функция | Назначение |
|---------|------------|
| `ParsePropertyDescriptionToRules` | Парсинг описания свойства в структурированные правила для UI |

## Зависимости
- `Helpers.hpp` — ParsePropertyDescription (вызывается внутри)
- `Sync.hpp` — SyncRule, SkipValues

## Зависимости (используется в)
- `Propertycache` — PropertyRuleFlag (parsed: ParsePropertyResult)
- `BrowserPalette` — отображение правил в интерфейсе
