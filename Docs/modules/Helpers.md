# Helpers — Ядро: чтение/запись параметров и свойств

> Хеш коммита: 493caf5 (2026-09-22). **Неполное покрытие**: пофункциональные карточки не приведены (2145 символов — см. symbols.json); ниже — типы и подсистемы из заголовка.

## Назначение
Ядро add-on: чтение из свойств/GDL/IFC/атрибутов/координат/морфов в единый ParamValue и запись обратно; большинство модулей зависят от него. [по коду include-графа]

## Файлы
- `Sources/AddOn/Helpers.cpp/hpp` (hpp 808 строк)

## Ключевые типы [из комментария / по коду, Helpers.hpp:14-63]

| Тип | Описание |
|-----|----------|
| `SortGUID` / `SortInx` | Массивы GUID / индексов для сортировки [по коду] |
| `OrientedSegments` | Отрезки с точкой начала и направлением подрезки [из комментария, hpp:26] |
| `SkipValues` | ignorevals, skip_empty, skip_trim_empty, reset_to_def [из комментария, hpp:34-46] |
| `DimRule` | pen_original/pen_rounded, round_value, flag_change/deletewall/reset/custom, classic_round_mode, expression, layer, paramDict [из комментария, hpp:48-61] |
| `DimRules` | HashTable<UniString, DimRule> [по коду, hpp:63] |

## Подсистемы [по коду заголовка]
- `FormatStringFunc` — формат из формулы (GetFormatStringFromFormula, hpp:67+)
- `ParamHelpers` — чтение параметров, источник по raw-name (расширен в Propertycache.hpp)
- Источники чтения: Property, GDL, IFC, Material, Coord, Morph, Info, Glob, ID, Classification, Attrib, ListData, MEP (флаги from* — CommonFunction.hpp:170-197)
- Запись в свойства/параметры/GDL

## Карточки ключевых функций

### `FormatStringFunc::GetFormatStringFromFormula(...) -> FormatString`
- Расположение: `Sources/AddOn/Helpers.cpp` (строка не проверена)
- Назначение: разбор формата строки из формулы. [по коду заголовка]
- Побочные эффекты: не проверено.

### `ParamHelpers::*` (чтение источников в ParamValue)
- Расположение: строки .cpp не проверены
- Назначение: заполнение ParamValue из источников; расширен в Propertycache.hpp:36-98 [по коду]
- Побочные эффекты: только чтение + заполнение out-структур. [по коду]

## Зависимости
- `ClassificationFunction.hpp`, `CommonFunction.hpp`, `dialogs/SyncSettings.hpp`, `spec/Spec_libpart.hpp`, `StringConversion.hpp` [по include]

## Зависимые модули
Sync, Summ, Spec, ReNum, Dimensions, Roombook, MEPv1, Propertycache, pk/*, dialogs/* [по include]

## Инварианты
- Мемо: перед `ACAPI_Element_GetMemo` всегда `BNZeroMemory`/`= {}` — все 7 объявлений в Helpers.cpp проверены как `= {}` (2026-09-22, DISCREPANCIES.md #7) [проверено]
