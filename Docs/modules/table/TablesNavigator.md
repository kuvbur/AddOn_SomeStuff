# table/TablesNavigator — Каркас ведомостей

> Хеш коммита: 493caf5 (2026-09-22).

## Назначение
Каркас пользовательских ведомостей в Navigator/MyDraw; границы подсистемы: определение ведомости, снимок таблицы, renderer, точки регистрации. [из комментария, TablesNavigator.hpp:8-13]

## Ключевые типы [из комментария, TablesNavigator.hpp:16-64]
`ColumnDefinition` (id — устойчивый ключ, title); `RowIdentity` (stableKey + sourceElements); `ScheduleDefinition` (payload viewpoint ещё не выбран); `TableCell`/`TableRow`; `TableSnapshot` (отделён от definition).

## Публичный API

| Функция | Назначение |
|---------|------------|
| `RegisterInterface` / `Initialize` / `EnsureNavigatorRoot` / `IsNavigatorRegistrationEnabled` | Точки регистрации в Archicad [из комментария, TablesNavigator.hpp:66-69] — строки .cpp не проверены |

## Зависимости
- `ACAPinc.h` [по include]

## Зависимости (используется в)
`SomeStuff_Main` [по коду]

## Инварианты
- `id` колонки отдельно от `title` (переименование в UI не ломает ключ) [из комментария, TablesNavigator.hpp:20-21]
- Ключ строки не равен позиции row и не сводится к одному GUID [из комментария, TablesNavigator.hpp:24-30]
