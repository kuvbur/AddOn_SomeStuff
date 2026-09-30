# TestFunc — Локальное тестирование

> Базовый хеш карточки: 1e67983 (2026-09-22); локальное дополнение #199 — 2026-09-24 (ещё без коммита). Полное покрытие API (hpp:17-123, 33 функции); строки — объявления в hpp, определения .cpp не фиксировались.

## Назначение
Вспомогательные функции для локального тестирования и отладки. Активен только под `TESTING`. [из комментария, TestFunc.hpp:16]

Вывод `DBprnt`/`DBtest` идёт в отладочный вывод ArchiCAD (`DBPrintf`/`DBPrint`); результаты читают в панели «Отладка» Visual Studio через VS MCP `output_read` (AGENTS.md §9). Файл `test_results.txt` больше не используется. Ошибки ищут как `ERROR IN TEST`, набор проверок ограничен строками `TEST : start` / `TEST : end`.

**Тег функции в квадратных скобках (#230).** `DBprnt`/`DBtest` в этом файле — не прод-функции, а локальные макросы, которые подставляют `__func__` в обёртки `TaggedPrnt`/`TaggedTest` (объявлены в анонимном namespace в начале файла, макросы — сразу после). Тег добавляется к содержательному тексту, поэтому строка вывода имеет вид `[TestSpecMergeAndKey] Spec merge sums add up` — имя функции печатается в начале строки, и по нему строка фильтруется в панели «Отладка». Прод-реализация в `CommonFunction.cpp` не менялась: её вывод используется и вне тестов (`Propertycache.hpp` и др.), и подменять его глобально нельзя. Обёртки объявлены до макросов, поэтому вызовы внутри них не подменяются (препроцессор идёт по файлу последовательно); вызовов `DBtest`/`DBprnt` в лямбдах в файле нет, поэтому `__func__` всегда даёт имя теста, а не оператора. [по коду `TestFunc.cpp:22-54`, runtime AC25 2026-09-29]

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

### Преобразования в ParamValue [из комментариев]
`TestConvertToParamValue` (37) — значения; `TestConvertAttributeToParamValue` (40) — атрибуты; `TestConvertPropertyToParamValue` (43) — свойства; `TestConvertPropertyDefinitionToParamValue` (46) — определения; `TestSetParamValueSourseByName` (49) — источник по raw-name; `TestSetrawNameFromProperty` (52) — raw-name из описания свойства.

### Правила, парсинг, TDD [из комментариев]
`TestCheckIgnoreVal` (55) — правила игнорирования; `TestReadProperty` (58) — чтение свойств; `TestAddProperty` (61) — добавление свойств в словарь; `TestPropertyHelpersToString` (64) — структуры → строка; `TestName2Rawname` (67) — имя → rawname, включая имена с одной недостающей скобкой и GREEN-кейсы без скобок/с полными скобками (AC25); `TestName2RawnameWithBrackets` (70) — с уже обёрнутыми скобками; `TestSyncString` (73) — парсинг правила; `TestSyncStringRealRules` (76) — реальные правила из BuildingInformation.xml; `TestParsePrefixes` (79) — константы префиксов; `TestParsePropertyDescription` (82) — команды Sync/Renum/Sum/Spec; `TestParseSyncStringIndependent` (85) — Этап 2 TDD; `TestParsePropertyDescriptionToRules` (88) — в структурированные правила.

### RED/GREEN-регрессии [из комментариев]
`TestSpecGetParamValue` (`TestFunc.cpp:55`, объявление `TestFunc.hpp:20`) — #220: обычное чтение, отсутствующие ключи и составы, числовой/текстовый материал, положительные и отрицательные границы, list-data и формула. AC25 runtime: исходный набор 24 OK / 11 ERROR; после исправления и расширения — 43 OK / 0 ERROR (панель VS «Отладка»). Включён в `TestFunc::Test`; временный отдельный вызов из Main удалён, пользовательское отключение общего набора сохранено. Полный набор тестов не запускался. [по коду и runtime]

`TestSyncString` (#202) — добавлены кейсы правил `File:lookup;"имя_файла",N,"ячейка",...`: полное число валидно и даёт `composite_pen`/`array_column_end`/`array_column_start`/`array_row_end`/`array_row_start`; мусор после числа (`2junk`) отклоняется в каждой из пяти позиций. Имена файла/ячеек — в кавычках: `GetSubstring` берёт первую пару скобок, вложенные `{...}` обрезают правило. Тесты в ArchiCAD не выполнялись — общий набор отключён коммитом `927d2d3`. Проверено только компиляцией AC25. [по коду, `6c9fc4d`]

`TestSyncAddSubelement` (93) — развёртывание from_sub/to_sub и проверка внешнего адресата `Sync_to_GUID` (#199, RED→GREEN на AC25); RED-тест бага P1 (ветка to_sub недостижима) + GREEN-регрессии. `TestBuildOtdByParent` (3133) — RED→GREEN #193: внешний GUID базы сохраняется ключом, внутренние GUID отделки индексируются по `TypeOtd`, неизвестный дочерний GUID пропускается. `TestRenumPosLogic` (100) — RenumPos (конструкторы, Add, FormatToMax, SetToMax), GetMostFrequentPos, ReNumGetFlag — фиксируют текущее поведение. `TestDescToRulesSubGuid` (105) — to_sub/from_sub/GUID: targetType/targetName/hasSub/hasGUID/guidSourceProperty — фиксация контракта при правке P1. `TestGetPropertyRuleFlag` (108) — признак правила в описании. `TestPropertyRuleFlagOnProjectElements` (113) — диагностика #184/#185: путь BrowserPalette::GetPropertiesList на реальных элементах, длины описаний из двух источников.

### Утилиты отладки [из комментариев]
`DumpAllBuiltInProperties` (113) — все встроенные свойства в журнал; `ResetSyncPropertyArray` (116) / `ResetSyncPropertyOne` (119, 122 — перегрузка с набором свойств) — сброс свойств синхронизации.

## Зависимости
- `api_headers/APICommon25/26/27.h` [по include]

## Инварианты
- Прод-код в тестовых задачах read-only; ошибки — читать `ERROR IN TEST` в панели «Отладка» Visual Studio через VS MCP `output_read` (AGENTS.md §9) [из AGENTS.md]
