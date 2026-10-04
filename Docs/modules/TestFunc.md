# TestFunc — Локальное тестирование

> Базовый хеш карточки: 1e67983 (2026-09-22); локальное дополнение #199 — 2026-09-24 (ещё без коммита). Полное покрытие API (hpp:17-123, 33 функции); строки — объявления в hpp, определения .cpp не фиксировались.

## Назначение
Вспомогательные функции для локального тестирования и отладки. Активен только под `TESTING`. [из комментария, TestFunc.hpp:16]

`TestBuildOtdByParent` (`tests/TestCore.cpp:79`) характеризует группировку отделки по двум родителям, двух детей-стен одного родителя, неизвестного ребёнка, пустой набор типов и исходное значение `hasBaseElement`. Синтетический тест не проверяет геометрию или материалы созданной отделки. [по коду; runtime AC25 2026-10-03: 12 passed, 0 failed]

Вывод результата — файловый отчёт **TestKit** (`%TEMP%\somestuff_test_report.txt`): каждая строка сбрасывается на диск (`fflush`), поэтому файл переживает падение ArchiCAD; в панель «Отладка» строки дублируются через `DBPrintf`. Успех по умолчанию молчит (`printPass=false`), печатаются отклонения и сводка. Границы наборов — `BEGIN`/`END <имя> passed=N failed=M`, признак завершения прогона — `=== somestuff tests end ===`. Файл `test_results.txt` не используется. [по коду `TestKit.cpp`, runtime AC25 2026-09-30]

`Tools/restart_archicad_for_test.ps1` читает отчёт и выставляет код возврата: `70` (`EXIT_TESTS_FAILED`) при `failed>0`, иначе `0`. Отсутствие отчёта или строки `SUMMARY` даёт `CXX_TESTS_NO_REPORT` / `CXX_TESTS_NO_SUMMARY` без 70 — тесты тогда просто не стартовали, и это не ошибка сборки. Скрипт ждёт отчёт до 90 с: тесты стартуют на `APINotify_Open`, то есть когда проект реально открылся, поэтому без ожидания проверка проходит за ~12 с до первого `BEGIN`. [по коду, runtime AC25 2026-09-30]

**Формат результата (#231).** Проверки идут через `TestKit::Check`, объявленный макросами `DBtest`/`DBrequire`/`DBskip` в `TestKit.hpp`; прод-`DBtest`/`DBprnt` из `CommonFunction.hpp` в тестах не используются — они объявлены в общем заголовке и вызываются также из `Helpers.cpp`/`CommonFunction.cpp`, где вывод остался прежним. Допуск для чисел берётся из продового `is_equal` (absTol 1e-12 / relTol 1e-9), а не из отдельной константы: вторая константа разошлась бы с продом на больших значениях (`TestSpecValueEdges` ждёт 2147483648.0 и -2147483649.0). `DBrequire` прерывает набор (обычный `DBtest` продолжает) — нужен там, где следом идёт разыменование указателя.

**Измерения — `TestKit::Note` (#231).** Ни одного вызова `DBprnt` в `tests/` не осталось. Строка парная, машиночитаемая:

```
NOTE [Verbose] TestPropertyRuleFlagOnProjectElements | RuleFlagProj.list | elements=203 | wallCode=0 | slabCode=0
NOTE [Verbose] TestPropertyRuleFlagOnProjectElements | RuleFlagProj.desc | name=КЖ/КМ/АР Наименование | len=68 | rule=true | text=Sync_from{naen; trim_empty; def}
```

`subject` — точка измерения (`RuleFlagProj.list`, `.elem`, `.desc`, `.definitions`), поля — через `SMSTF_FIELD` (арифметика) или `SMSTF_FIELD_U` (`GS::UniString`), необязательный хвост — свободный текст. Набор подставляет раннер, в записи после прогона — `<run>`. Уровень задаётся `SMSTF_VERBOSE`; на Normal шумные замеры скрыты, на Verbose печатаются с меткой.

 **Раскладка по TU (#231).** Наборы разнесены по файлам, раскладка совпадает с группами
реестра. Счётчик строк не равен числу проверок: `TestSpecParser` — 38 вызовов `DBtest`,
но выполняет их 29-кратный цикл по таблице кейсов, `TestParsePrefixes` — 69 вызовов.
При добавлении набора его объявление идёт в `TestFunc.hpp`, определение — в файл своей
группы, строка `Register` — в `TestFunc.cpp`. CMake менять не нужно: sources берутся
`GLOB_RECURSE CONFIGURE_DEPENDS` по `${addOnSourcesFolder}/*.cpp`, а include-каталог
содержит сам `Sources/AddOn`, поэтому `tests/` подхватывается и видит корневые заголовки
без правок. Внешние потребители `TestFunc.hpp` — `Helpers.cpp`, `SomeStuff_Main.cpp`,
`Sync.cpp`, `spec/Spec.cpp` — включают его как `"tests/TestFunc.hpp"`.

**Границы при написании набора (#231, R5.5).** Три ловушки наполнения
`SpecReadContext`, найденные прогоном, а не чтением заголовков:
`GS::HashTable::Get` на отсутствующем ключе бросает исключение (элемент
заводится через фикстуру до `Get().Put()`); в `uniStringValue` формулы лежит
ВЫРАЖЕНИЕ без префикса `{@formula:` (префикс — в ключе словаря); признак
`fromGDLArray` виден вычислителю только при валидном значении с `fromMaterial`
и отсутствующем составе конструкции. Подробно — в `Docs/modules/spec/Spec.md`
(R5.5). У `GS::HashTable` нет `operator==`, а в pre-AC28 `it->key` — указатель:
сравнение словарей со снимком делается перебором пар с поимённой сверкой полей
`ParamValueData`.

**Табличные кейсы (#231).** Три набора переведены с копипаст-проверок на
таблицы в анонимном namespace, наборы гоняют их циклом:
- `TestParsePrefixes` — 62 сверки констант против литералов, таблицы
  `uniConstCases` (38 строковых) и `intConstCases` (24 числовых);
- `TestName2Rawname` / `TestName2RawnameWithBrackets` — изначально по 13 пар
  (вход -> rawname), таблицы `plainNameCases` / `bracketedNameCases`,
  структура `RawNameCase`; в #235 первая таблица расширена до 16 пар.
До #235, при переносе тестов в таблицы, число ВЫПОЛНЕННЫХ проверок не изменилось; упало только число мест в коде
(`TestSync.cpp` 220 -> 172). Сверка эквивалентности делается ДО подстановки:
пары сравниваются как множества (вход, ожидание, лейбл), и лейблы — посимвольно.
Исключения из таблиц остаются явными: `BeginsWith` там, где ключ несёт хвост
пути (BuildingMaterial), и пустая строка -> false. Если у двух проверок одного
кейса разные лейблы (как у `}{@Coord:Symb_Pos_X}`: `rawname lowered` против
`rawname unchanged`), хвост лейбла выносится в отдельную колонку, а не
сокращается до категории - иначе падение перестаёт показывать, какой вход дал
неверный ключ.

**Отбор наборов.** Реестр наполняется явно в `TestFunc::Test` (49 наборов, 6 групп: spec/sync/param/format/renum/core), а не статическими инициализаторами: при разбиении файла по TU регистратор вместе со своей `static`-функцией выкидывается линковкой и набор молча исчезает из прогона. Отбор — переменная окружения `SMSTF_TEST`: пусто (всё), группа, префикс с `*` или список через запятую. Мёртвый агрегатор `TestSpecRegression`, дублировавший реестр, удалён. [по коду `TestFunc.cpp`, `TestKit.cpp`]

## Roombook — характеризации #260 (AC25, 2026-10-04)

- `TestOpeningAddOne` (`TestCore.cpp:157`) 114/0; `TestOtdWallDelimOne` (:227) 246/0; `TestOtdWallDelimAll` (:424) 197/0; `TestOtdWallAddOne` (:606) 60/0. Последний добавлен в Core через явный Register и объявление в TestFunc.hpp; TestCore содержит 6 наборов. [по коду и штатным отчётам]
- AddOne: 11 сценариев ребра/высоты, отказ без мутации, сохранение неинициализируемых полей. DelimOne: 8 новых wall-intersection сценариев при width=0/1, без изменения существующих opening/многопроёмных assertions (#261). Production read-only до зелёного baseline; после P4e те же числа. [по коду и исполнению]
- Реестр 73 registered/defined/declared; TestKit 73/3184/0, EXIT 0 до/после P4e. BuildAddOn AC25 success; fresh MCP TestCore 0 errors/3 прежних warnings. TestKit-сводки в VS pane не найдены; источник чисел — свежие отчёты 08:09:51/08:13:04 после соответствующих APX, запуски через VS MCP. [по инструментам]
- Числа 49 наборов/103 строки и прежние file-counts в исходной части карточки ниже исторические; весь модуль в этом шаге не пересчитывался. [граница обновления]

## Выбор материала #260 — P2b2f/P4f (AC25, 2026-10-04)

- `TestSetMaterialByType` (`TestCore.cpp:675`) зарегистрирован в Core, объявлен в TestFunc.hpp. 11 типов × 6 конфигураций × 2 вида последнего слоя; 1848/0 до/после, TestKit 74/5032/0, EXIT 0 (11:00:47/11:04:22), registry 74/74/74. TestCore теперь 7 наборов; остальные старые числовые таблицы карточки исторические. [по fresh MCP/registry и отчётам]
- Ожидания отдельно задают выбранные поля материала и итоговый TypeOtd: нормализация rawname не пере выбирает материал, last-layer override меняет только имя/индекс. Проверяются сохранность всех настроек, геометрия, первоначальный слой и добавленный finish. Production read-only до зелёного baseline. [по коду и исполнению]
- AC25 BuildAddOn success до/после; fresh TestCore 0 errors/3 прежних warnings, registry 0 errors/2 warnings. Свежесть обоих отчётов относительно соответствующего APX проверена; suite-маркер в VS pane отсутствует, verdict взят из штатного отчёта. [по инструментам]

## Откосы #260 — P2b2g/P4g (AC25, 2026-10-04)

- `TestOpeningRevealsCreateOne` (`TestCore.cpp:795`) зарегистрирован в Core и объявлен в TestFunc.hpp. 13 сценариев × два начальных has_reveal; 662/0 на исходном и извлечённом initializer, весь TestKit75/5694/0, EXIT0. Registry75 registered/defined/declared, TestCore8 наборов. [по fresh MCP, реестру и штатным отчётам]
- Проверяет две боковые стенки/верхнюю балку: порядок, координаты, высоты и length после подрезки, точный верх/касание/вырожденные размеры/ориентацию, материал и состав, GUID/этаж/типы и сохранение входов. Нулевые размеры характеризуют существующий код, не вводят новые требования к валидации. Production read-only до baseline. [по коду и исполнению]
- BuildAddOn AC25 Windows Debug success до/после; fresh TestCore0 errors/3 прежних warnings, registry0 errors/2 warnings. Оба отчёта свежее своих APX; запуск через VS MCP. В VS pane suite-маркер отсутствует; числа взяты из свежего штатного отчёта, не из отсутствия ERROR IN TEST. [по инструментам]

## Файлы
- `Sources/AddOn/tests/` — все файлы тестов, только под `#ifdef TESTING`:
  - `TestFunc.cpp` (103 строки) — заголовок, `Groups`, реестр 49 наборов, `Test()`;
  - `TestFunc.hpp` — объявления наборов, сгруппированы по группам реестра;
  - `TestKit.cpp/hpp` (#231) — бэкенд: отчёт, счётчики, отбор, макросы проверок;
  - `TestSpec.cpp` (21) · `TestSync.cpp` (10) · `TestParam.cpp` (12) ·
    `TestFormat.cpp` (3) · `TestRenum.cpp` (1) · `TestCore.cpp` (2) — наборы;
  - `TestUtil.cpp` — 4 хелпера: `TestGetTextLineLength`, `DumpAllBuiltInProperties`,
    `ResetSyncPropertyArray`, `ResetSyncPropertyOne` (две перегрузки).

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
Отрицательный вещественный кейс `TestConvertPropertyToParamValue` использует
`-123456.7`: проверка `doubleValue` допускает округление по настройке проекта;
`rawDoubleValue` предназначен для чтения исходной точности. [по коду
`tests/TestParam.cpp:256-263`, `CommonFunction.hpp:97-98`, `Helpers.cpp:8255-8283`]

### Правила, парсинг, TDD [из комментариев]
`TestCheckIgnoreVal` (55) — правила игнорирования; `TestReadProperty` (58) — чтение свойств; `TestAddProperty` (61) — добавление свойств в словарь; `TestPropertyHelpersToString` (64) — структуры → строка; `TestName2Rawname` (67) — имя → rawname, включая имена с одной недостающей скобкой и GREEN-кейсы без скобок/с полными скобками (AC25); `TestName2RawnameWithBrackets` (70) — с уже обёрнутыми скобками; `TestSyncString` (73) — парсинг правила; `TestSyncStringRealRules` (76) — реальные правила из BuildingInformation.xml; `TestParsePrefixes` (79) — константы префиксов; `TestParsePropertyDescription` (82) — команды Sync/Renum/Sum/Spec; `TestParseSyncStringIndependent` (85) — Этап 2 TDD; `TestParsePropertyDescriptionToRules` (88) — в структурированные правила.

### RED/GREEN-регрессии [из комментариев]
`TestSpecGetParamValue` (`tests/TestSpec.cpp:104`, объявление `tests/TestFunc.hpp:57`, реестр `tests/TestFunc.cpp:39`) — #220: обычное чтение, отсутствующие ключи и составы, числовой/текстовый материал, положительные и отрицательные границы, list-data и формула. AC25 runtime: исходный набор 24 OK / 11 ERROR; после исправления и расширения — 43 OK / 0 ERROR (панель VS «Отладка»). Включён в `TestFunc::Test`; временный отдельный вызов из Main удалён, пользовательское отключение общего набора сохранено. Полный набор тестов не запускался. [по коду и runtime]

`TestSyncString` (#202) — добавлены кейсы правил `File:lookup;"имя_файла",N,"ячейка",...`: полное число валидно и даёт `composite_pen`/`array_column_end`/`array_column_start`/`array_row_end`/`array_row_start`; мусор после числа (`2junk`) отклоняется в каждой из пяти позиций. Имена файла/ячеек — в кавычках: `GetSubstring` берёт первую пару скобок, вложенные `{...}` обрезают правило. Тесты в ArchiCAD не выполнялись — общий набор отключён коммитом `927d2d3`. Проверено только компиляцией AC25. [по коду, `6c9fc4d`]

`TestSyncAddSubelement` (93) — развёртывание from_sub/to_sub и проверка внешнего адресата `Sync_to_GUID` (#199, RED→GREEN на AC25); RED-тест бага P1 (ветка to_sub недостижима) + GREEN-регрессии. `TestBuildOtdByParent` (3133) — RED→GREEN #193: внешний GUID базы сохраняется ключом, внутренние GUID отделки индексируются по `TypeOtd`, неизвестный дочерний GUID пропускается. `TestRenumPosLogic` (100) — RenumPos (конструкторы, Add, FormatToMax, SetToMax), GetMostFrequentPos, ReNumGetFlag — фиксируют текущее поведение. `TestDescToRulesSubGuid` (105) — to_sub/from_sub/GUID: targetType/targetName/hasSub/hasGUID/guidSourceProperty — фиксация контракта при правке P1. `TestGetPropertyRuleFlag` (108) — признак правила в описании. `TestPropertyRuleFlagOnProjectElements` (113) — диагностика #184/#185: путь BrowserPalette::GetPropertiesList на реальных элементах, длины описаний из двух источников.

### Утилиты отладки [из комментариев]
`DumpAllBuiltInProperties` (113) — все встроенные свойства в журнал; `ResetSyncPropertyArray` (116) / `ResetSyncPropertyOne` (119, 122 — перегрузка с набором свойств) — сброс свойств синхронизации.

### #235 — имена атрибутов Sync [по коду]
`TestName2Rawname`: в `plainNameCases` добавлены `Attribute:Composite`, `Attribute:BuildingMaterial`, `Attribute:CompositeType` → `{@attrib:...}` (шесть проверок). `TestSyncString`: три правила `Sync_to{Attribute:...}` проверяют направление и признак `fromAttribElement` (девять проверок). Проверяются парсер и диспетчеризация правил, не запись конструкции элемента. Результат исполнения — в `Docs/_progress.md`.

## Зависимости
- `api_headers/APICommon25/26/27.h` [по include]

## Инварианты
- Прод-код в тестовых задачах read-only; провал виден как строка `FAIL ... | expected ... got ...` в отчёте и как `FAILED_SUITE`/`exit_code=70` от раннера [из AGENTS.md]
- **В тестах нет ни одного вызова `DBprnt`**: прод-макрос печатает префикс `== ERROR ==` по вхождению `err`/`ERROR` в тексте (измерение выглядит как ошибка), склеивает аргументы через `" : "` и пишет мимо файла отчёта. Вместо него — `TestKit::Note` (#231).
- Число выполненных проверок не равно числу вызовов `DBtest` в коде: таблицы кейсов (`TestSpecParser` — 29 строк x ~10 проверок) и циклы исполняют один вызов многократно. Для сверки в `END` печатается `passed=`/`failed=` по набору. Незанятая ветка `else` даёт `sites - 1` — это нормально (#231).
- Измерительный набор `TestPropertyRuleFlagOnProjectElements` не содержит `DBtest` по определению — печатает измерения через `Note` (#231).
- Строка `NOTE` парная: `NOTE [Verbose] suite | key=value | … | текст`. Поля всегда разобраны по `=`, поэтому значение с `=` внутри остаётся однозначным; свободный текст идёт последним (#231).
- Уровень измерений — `SMSTF_VERBOSE` (`0`/`1`/`2` = Quiet/Normal/Verbose), по умолчанию Normal. `minLevel` передаётся аргументом `Note`/`NoteFields`, а не переключателем уровня: забытая скобка тихо оставила бы шум на Normal. `Config::noteLevelExplicit` отделяет «явно попросили Normal» от дефолта, иначе переменная окружения перекрыла бы намерение вызывающего (#231).
- В `SUMMARY` есть `notes=N` — считаются и заглушенные записи, поэтому видно, что измерение выполнилось, даже если уровень его скрыл (#231).
- Квалификация в макросах TestKit — `::TestKit` (оператор глобальной области), не `TestKit`: вызовы идут изнутри `namespace TestFunc`, и короткое имя искалось бы как `TestFunc::TestKit` (MSVC C2039; clangd этот контекст не проверяет). Без квалификации — C2065: пространства TestKit и TestFunc соседние (#231).
- `SMSTF_FIELD` принимает арифметику, `SMSTF_FIELD_U` — `GS::UniString`. У `GS::ValueToUniString` нет перегрузок для `bool` и `USize` (`Build/DevKit/APIDevKit-25/Support/Modules/GSRoot/CH.hpp:524-558`), поэтому `bool` печатается как `true`/`false`, а `USize` (= `UInt32`, `Definitions.hpp:387`) приводится явно (#231).
