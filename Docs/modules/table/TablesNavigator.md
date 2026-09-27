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
## Версии
- AC27 (`#170`): legacy-вызовы SDK обёрнуты `#ifdef ServerMainVers_2700` (новое ACAPI-имя) / `#else` (старое): `ACAPI_Database (APIDb_…ID)` → `ACAPI_Database_*`/`ACAPI_Window_*`/`ACAPI_Drawing_*`/`ACAPI_View_*`, `ACAPI_Navigator (APINavigator_…ID)` → `ACAPI_Navigator_*`, `ACAPI_Register/Install_NavigatorAddOnViewPointData*` → `ACAPI_AddOnIntegration_*`. Имена взяты из DevKit-27 `Support/Inc/ACAPI_MigrationHeader.hpp`; сам заголовок в проект не подключается (AGENTS §6). [по коду DevKit + сборка AC27]
- AC27: константы `API_NavgatorViewSettings*ID` переименованы в `API_NavigatorViewSettings*ID` (в SDK исправлена опечатка) — выбор веткой `#ifdef ServerMainVers_2700`. [по коду DevKit]
- AC27: `ACAPI_AttributeIndex` в вызовах рисования, `API_PenType` → `const GS::Array<API_Pen>*` в `ACAPI_Drawing_StartDrawingData`. [по коду DevKit]
