# Helpers — Ядро: чтение/запись параметров и свойств

> Хеш коммита: f8f599c (2026-09-22)

## Назначение модуля
Ядро add-on: чтение значений из свойств, GDL, IFC, атрибутов, координат, морфов и др. источников в единый `ParamValue`, и запись обратно. Большинство остальных модулей зависят от него. [по коду include-графа]

## Файлы модуля
- `Sources/AddOn/Helpers.cpp/hpp` (hpp 808 строк; symbols.json — 2145 символов, самый большой модуль)

## Ключевые типы [по коду, Helpers.hpp:14-63]

| Тип | Описание |
|-----|----------|
| `SortGUID` / `SortInx` | Массивы GUID / индексов для сортировки |
| `OrientedSegments` | Отрезки с точкой начала и направлением подрезки |
| `SkipValues` | Правила игнорирования значений: ignorevals, skip_empty, skip_trim_empty, reset_to_def |
| `DimRule` | Правило округления размеров: pen_original/pen_rounded, round_value, flag_change/deletewall/reset/custom, classic_round_mode, expression, layer, paramDict |
| `DimRules` | `HashTable<UniString, DimRule>` |

## Ключевые namespace/подсистемы [по коду заголовка]
- `FormatStringFunc` — разбор формата из формулы (`GetFormatStringFromFormula`, ...)
- `ParamHelpers` — чтение параметров/свойств, определение источника по raw-name (дополнен в Propertycache.hpp)
- Чтение из источников: Property, GDL, IFC, Material (состав конструкции), Coord, Morph, Info, Glob, ID, Classification, Attrib, ListData, MEP (флаги `from*` в `ParamValue`, CommonFunction.hpp:170-197)
- Запись значений в свойства/параметры/GDL

## Публичный API
Полный перечень функций — `Docs/_generated/symbols.json` (модуль Helpers, 2145 символов). **Пофункциональные записи не приведены — покрытие частичное** (см. `_progress.md`, раздел «Неполное покрытие»).

## Зависимости
- `ClassificationFunction.hpp`, `CommonFunction.hpp`, `dialogs/SyncSettings.hpp`, `spec/Spec_libpart.hpp`, `StringConversion.hpp`

## Зависимые модули (по include)
Практически все: Sync, Summ, Spec, ReNum, Dimensions, Roombook, MEPv1, Propertycache, pk/*, dialogs/*

## Инварианты и подводные камни
- `Helpers.cpp:1761` — `API_ElementMemo` без `BNZeroMemory` перед `ACAPI_Element_GetMemo` (риск краша на панелях навесных стен) [из ревью-заметок; статус не проверен в этой сессии — см. DISCREPANCIES.md]
- Мемо-инициализация: перед `ACAPI_Element_GetMemo` всегда `BNZeroMemory(&memo, sizeof(memo))` (AGENTS.md §6)