# TestFunc — Локальное тестирование

> Хеш коммита: 1e67983 (2026-09-22). Полное покрытие API (hpp:17-123, 33 функции); строки — объявления в hpp, определения .cpp не фиксировались.

## Назначение
Вспомогательные функции для локального тестирования и отладки. Активен только под `TESTING`. [из комментария, TestFunc.hpp:16]

Вывод `DBprnt`/`DBtest` идёт в отладочный вывод ArchiCAD (`DBPrintf`/`DBPrint`); результаты читают в панели «Отладка» Visual Studio через VS MCP `output_read` (AGENTS.md §9). Файл `test_results.txt` больше не используется. Ошибки ищут как `ERROR IN TEST`, набор проверок ограничен строками `TEST : start` / `TEST : end`.

## Файлы
- `Sources/AddOn/TestFunc.cpp/hpp` (hpp целиком под `#ifdef TESTING`)

## Публичный API (namespace TestFunc, все void)

### Базовые тесты [из комментариев]
| Функция | Строка | Назначение |
|---|---|---|
| `Test` | 19 | Запуск набора локальных проверок основных helpers |
| `TestGetTextLineLength` | 22 | Длина текстовой строки в нестандартных случаях |
| `TestCalc` | 25 | Арифметические и логические операции внутренних функций |
| `TestFormula` | 28 | Формульный парсинг и вычисление |
| `TestFormatString` | 31 | Форматирование строк по правилам add-on |
| `TestFormatStringFormula` | 34 | Форматирование строк на основе формул |

### Преобразования в ParamValue [из комментариев]
`TestConvertToParamValue` (37) — значения; `TestConvertAttributeToParamValue` (40) — атрибуты; `TestConvertPropertyToParamValue` (43) — свойства; `TestConvertPropertyDefinitionToParamValue` (46) — определения; `TestSetParamValueSourseByName` (49) — источник по raw-name; `TestSetrawNameFromProperty` (52) — raw-name из описания свойства.

### Правила, парсинг, TDD [из комментариев]
`TestCheckIgnoreVal` (55) — правила игнорирования; `TestReadProperty` (58) — чтение свойств; `TestAddProperty` (61) — добавление свойств в словарь; `TestPropertyHelpersToString` (64) — структуры → строка; `TestName2Rawname` (67) — имя → rawname; `TestName2RawnameWithBrackets` (70) — с уже обёрнутыми скобками (временное решение до исправления бага); `TestSyncString` (73) — парсинг правила; `TestSyncStringRealRules` (76) — реальные правила из BuildingInformation.xml; `TestParsePrefixes` (79) — константы префиксов; `TestParsePropertyDescription` (82) — команды Sync/Renum/Sum/Spec; `TestParseSyncStringIndependent` (85) — Этап 2 TDD; `TestParsePropertyDescriptionToRules` (88) — в структурированные правила.

### RED/GREEN-регрессии [из комментариев]
`TestSyncAddSubelement` (93) — развёртывание from_sub/to_sub; RED-тест бага P1 (ветка to_sub недостижима) + GREEN-регрессии. `TestRenumPosLogic` (97) — RenumPos (конструкторы, Add, FormatToMax, SetToMax), GetMostFrequentPos, ReNumGetFlag — фиксируют текущее поведение. `TestDescToRulesSubGuid` (102) — to_sub/from_sub/GUID: targetType/targetName/hasSub/hasGUID/guidSourceProperty — фиксация контракта при правке P1. `TestGetPropertyRuleFlag` (105) — признак правила в описании. `TestPropertyRuleFlagOnProjectElements` (110) — диагностика #184/#185: путь BrowserPalette::GetPropertiesList на реальных элементах, длины описаний из двух источников.

### Утилиты отладки [из комментариев]
`DumpAllBuiltInProperties` (113) — все встроенные свойства в журнал; `ResetSyncPropertyArray` (116) / `ResetSyncPropertyOne` (119, 122 — перегрузка с набором свойств) — сброс свойств синхронизации.

## Зависимости
- `api_headers/APICommon25/26/27.h` [по include]

## Инварианты
- Прод-код в тестовых задачах read-only; ошибки — читать `ERROR IN TEST` в панели «Отладка» Visual Studio через VS MCP `output_read` (AGENTS.md §9) [из AGENTS.md]
