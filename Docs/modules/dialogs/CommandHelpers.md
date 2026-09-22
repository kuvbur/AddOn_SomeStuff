# dialogs/CommandHelpers — Помощники команд

> Хеш коммита: 493caf5 (2026-09-22).

## Назначение
Парсинг описаний свойств для интерфейса: структурированные правила синхронизации для BrowserPalette. [из комментария, CommandHelpers.hpp:43-48]

## Ключевые типы [из комментария, CommandHelpers.hpp:11-40]
`SyncRuleInfo` (commandType, fullCommand, parameters, sourceType/targetType+Name, formatString, ignoreVals, isValid, errorMessage, hasSub, hasGUID, guidSourceProperty); `ParsePropertyResult` (syncRules, otherCommands, remainingText, hasSyncRules, hasOtherCommands).

## Публичный API

| Функция | Назначение |
|---------|------------|
| `ParsePropertyDescriptionToRules` | Парсит описание свойства в структурированные правила для UI; вызывает `ParsePropertyDescription` и разбирает Sync-команды на поля [из комментария, CommandHelpers.hpp:43-48] — строки .cpp не проверены |

## Зависимости
- `Helpers.hpp` (ParsePropertyDescription), `Sync.hpp` [по include]

## Зависимости (используется в)
`Propertycache` (PropertyRuleFlag.parsed), `BrowserPalette` [по коду]
