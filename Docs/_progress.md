# Состояние работы — документирование кодовой базы

## Обновления по задачам
- #260 P3a (2026-10-03, AC25 Windows Debug): временный `paramDict` перемещён из `RoomProcessingContext` в `ReadElementParameters` без дополнительных SDK-вызовов; `clang-format`, clangd 0 диагностик, `BuildAddOn.py -v 25` success, TestKit 69/2567/0. Пять первых и три парных A/B прогона на чистом PLN: 1778 новых элементов, сохранены исходные GUID, мультимножество `(тип, bounding box, status=normal свойства)` без различий с частичным baseline; полные контуры/материалы не проверены. A/B худшие времена 65.7253702 с исходный, 65.5245175 с P3a; владелец разрешил колебания до 5% и велел брать худшее значение за эталон — по этому критерию шаг принят, не как доказательство равенства скорости. `Roombook.md` обновлён; `symbols.json` адресно удаляет поле и сдвигает 10 координат на одну строку, сверенных с живым исходником. `get_document_symbols` после сборки всё ещё возвращал поле `paramDict` (устаревший AST), поэтому не использован для записи этого среза; callgraph не менялся — вызовы не менялись. Отдельная сверка среза `Roombook.cpp` с исходником: до правки 75 расхождений, после 75, новых 0, исправленных 0 — протухшие координаты ниже затронутой области унаследованы от прежних сборов, не трогались.
- #260 (2026-10-03, AC25 Windows Debug, ветка `roombook_refactor`): расширен характеризационный `TestBuildOtdByParent` в `tests/TestCore.cpp` без изменения production-кода; первая фикстура неверно ожидала мутации значения после `GS::HashTable::Add` (проверено в заголовке DevKit AC25), исправлена отдельной таблицей родителей. clangd: 0 диагностик; `BuildAddOn.py -v 25` success; свежий TestKit: `TestBuildOtdByParent passed=12 failed=0`, `SUMMARY suites=69 passed=2567 failed=0`. Карточка `TestFunc.md` дополнена; `symbols.json` не менялся — clangd подтвердил то же определение на `TestCore.cpp:79`; callgraph не менялся. Baseline конечной геометрии/материалов RoomBook остаётся неполным, производственный рефакторинг не начинался.
- #235 (2026-10-03, CLOSED, `2da2264`): кэш Layer/CompWall/BuildingMaterial и запись Basic ↔ Composite в `Helpers.cpp`/`Propertycache.cpp`; карточки `Helpers.md`, `Propertycache.md`, `Sync.md`, `TestFunc.md`. `symbols.json`: 1011 → 1012, новая static-функция, 73 позиции сдвинуты по git diff и целевой сверке имён (включая 10 в TestSync.cpp); полный сбор через clangd MCP не выполнялся (в исходном файле уже были устаревшие строки), `callgraph.json` вне pk не обновлялся. LSP: 0 диагностик по трём production-файлам; runner: build/load AC25 success; после `.apx` завершился свежий отчёт `SUMMARY suites=67 passed=2490 failed=0`, `TestName2Rawname passed=35 failed=0` (+6), `TestSyncString passed=53 failed=0` (+9). Реальный переход Basic ↔ Composite правилом `Sync_to` на модели проверен владельцем вручную, issue закрыт 2026-10-03. AC26–29 тоже успешно собраны до нового ограничения владельца; AC22 остановилась на ресурсе Browser, AC23–24 — на ошибках компиляции вне файлов #235. Впредь сборка других версий — только по явному запросу владельца.

- #232 (2026-10-02, AC25 Windows Debug): в `tests/TestParam.cpp:259-263` отрицательный вещественный кейс принимает `-123456.7` вместо `-123456.789`, сохраняя проверку `doubleValue` с учётом округления по проекту. `rawDoubleValue` хранит прочитанную точность (`CommonFunction.hpp:97-98`). Обновлена карточка `modules/TestFunc.md`. `symbols.json`/`callgraph.json` не менялись: правка двух литералов не меняет определения, позиции, вызовы или связи. clang-format и clangd: 0 диагностик; параллельный runner собрал `.apx` в 16:57:14 и открыл AC25, ранний выход runner 16:57:29 — 70 по прежнему отчёту (`failed=1`), новый отчёт 16:57:49 после сборки: `TestConvertPropertyToParamValue passed=23 failed=0`, `SUMMARY suites=70 passed=2861 failed=0`. Отдельный `BuildAddOn.py` до этого упёрся в LNK1168 при открытом Archicad; повторный runner не запускается, чтобы не закрывать экземпляр параллельного агента.

- **spec — полное обновление 2026-10-02 (HEAD `2101967`)**: секция `spec/` переписана, карта вызовов восстановлена. `callgraph.json` **155 → 418 рёбер** (146 сохранены, 9 удалено как устаревшие, 272 добавлено); все 272 новых ребра сверены с исходником скриптом «имя присутствует на указанной строке» — **0 расхождений**. `symbols.json` **940 → 1011** (113 для spec, та же сверка — 0 расхождений). `compile_commands.json` пересобран (`BuildAddOn.py -c config.json -v 25 --lsp`, exit 0): теперь 6 из 6 `spec/*.cpp`; до пересборки `SpecCompat.cpp` и `SpecPlanning.cpp` в нём не было, хотя оба подключаются из других TU. Карточки: созданы `spec/SpecExecutor.md` и `spec/SpecCompat.md`, обновлены `Spec.md`, `SpecHelpers.md`, `SpecPlanning.md`, `Spec_libpart.md`; overview — `ARCHITECTURE.md` (подграф spec из 6 узлов + исправлен сценарий 3), `REPOMAP.md` (статистика пересобрана, 4 устаревших счётчика), `DISCREPANCIES.md` (записи 11–14). **Сборка аддона и runtime не выполнялись** — правки только в `Docs/`. Открыто: 140 рёбер вне spec протухли (унаследовано от 2026-09-22, не эта сессия) — см. `callgraph.json` → `meta.known_stale_non_spec`.
- #234 (2026-10-01, AC25 Windows Debug, чекпоинт ниже): read-only валидатор правила по GUID свойства-правила. В `spec/Spec.cpp` новые `EvaluateRuleFlag` (:2447), `CollectUnreadRuleNames` (:2520), `CheckRuleByPropertyGuid` (:2608) и две внутренние `static` (`GetRulePropertyDefinition` :2559, `ReadElementRuleFlag` :2571); в `spec/Spec.hpp` — `PlaceSourceInfo`, `RuleFlagStatus` (5 состояний), `RuleFlagOrigin` (5), `RuleFlagCheck`, `RuleCheckResult` и объявления функций. `GetElementForPlaceProperties` расширена **одним** необязательным параметром `PlaceSourceInfo *readInfo = nullptr` — существующий вызов (`Spec.cpp:662`) не затронут. Карточка `modules/spec/Spec.md`: таблица API (4 строки) + карточка R4.9, где зафиксированы три вещи, которые нельзя вывести из кода: возвращаемый `bool` = `definitionFound`, а не «правило корректно»; прод `GetRuleFromElement` трактует `status == API_Property_NotAvailable` как «флаг включён», а `EvaluateRuleFlag` оставляет это отдельным ответом (расхождение намеренное); на AC22–23 `NotAvailable` невыразимо и схлопывается в `NotEvaluated`. `symbols.json`: 937 -> **940** записей (3 добавлены), 21 устаревшая строка `spec/Spec.cpp` приведена к фактическим определениям — расхождение существовало до этой задачи (в HEAD `GetRuleFromDefaultElem` на :34, в файле было :42), пересортировка файла не выполнялась, порядок записей = порядок сбора; LF, без BOM, без завершающего LF, псевдосимволов 0. `callgraph.json` не тронут: рёбра не собирались (политика §2 — только под горячий модуль). Проверки: clang-format на 5 файлов, clangd 0 ошибок (warning `PushRawSlot` в `TestSpec.cpp:45` — предсуществующий, вне моего хука), AC25 `success`, sweep AC26-29 все `success`, `restart_archicad_for_test.ps1` exit_code=70 (`suites=64 passed=2445 failed=1`) — `TestSpecRuleCheck` 43/43, единственный `FAILED_SUITE TestConvertPropertyToParamValue` предсуществующий и вне области; **дельта по всем 63 существующим наборам = 0**. AC22-24 не собирались: AC22/23 падают на ресурсах (`CompileResources.py`, `.grc`) и в чужих файлах `BrowserPalette.cpp` / `SyncSettings.cpp` / `Roombook.hpp` — проверено на чистом дереве через `git stash`, те же сбои без моих правок. **AC24 not verified** — DevKit-24 в репозитории отсутствует; ветка `#ifdef ServerMainVers_2400` проверена по AC25 и по отсутствию полей в AC23. macOS не собирался.
- #228 (2026-10-01, чекпоинт `4e2c1fa` поверх): четыре функции, помеченные в `spec/Spec.cpp` как `TODO : вынести в SpecHelpers`, вынесены в новый внутренний модуль `spec/SpecHelpers.hpp/.cpp` — `GetSizePlaceElement` (шаг сетки размещения), `ParamValueToDumpString`, `FillDumpFromParamDict`, `FillDumpGDLParameter` (дамп значений элемента, #227). Три последние были `static` с прямыми декларациями в начале `Spec.cpp`; объявление `GetSizePlaceElement` убрано из `Spec.hpp`, `TestSpecSizes` подключает новый заголовок. Тела перенесены дословно — сверено с `HEAD` посимвольно по всем четырём, мультимножество идентификаторов не изменилось, `ACAPI_*` 0 в обоих. Новая карточка `Docs/modules/spec/SpecHelpers.md`; в `Spec.md` строки API помечены как вынесенные, добавлен include; в `REPOMAP.md` состав `spec/`. `symbols.json`: 942 -> 937 записей, 39 -> 40 файлов (сбор через clangd MCP, строки 0-based -> 1-based), 7 записей удалены из `Spec.cpp`, 4 добавлены для `SpecHelpers.cpp`; псевдосимволов 0. `callgraph.json` не тронут. Проверки: clang-format на 5 файлов, clangd 0 ошибок (4 warning в `TestSpec.cpp` — предсуществующие `unused-includes`), AC25 `Build succeeded!`, sweep AC26-29 все `success`, `restart_archicad_for_test.ps1` exit_code=70 (`suites=62 passed=2337 failed=1`, `FAILED_SUITE TestConvertPropertyToParamValue` — предсуществующая вне области), A/B `sh-final` C=2/M=12/D=2, 14 строк, `diff_rows` = 0 против шести эталонов (`p0-smoke`, `r34-final`, `r34-state`, `r73c`, `r74-final`, `r74-inv`), sha256 фикстуры `db1690f…` совпал. macOS и AC22-24 не собирались.

- #231 (2026-09-30, чекпоинты `4478494`…`a7e288e`): переработка тестов. `TestFunc.cpp` 330 KB -> 8 KB (реестр 49 наборов) + `TestKit.*` (файловый отчёт `%TEMP%\somestuff_test_report.txt`, счётчики, отбор `SMSTF_TEST`) + `Test{Spec,Sync,Param,Format,Renum,Core,Util}.*`; всё в `Sources/AddOn/tests/`. Раннер выдаёт `exit_code=70` при `failed>0`. Проверки переведены в таблицы кейсов в трёх наборах. Прогон AC25: `suites=49 passed=1825 failed=1` (провал предсуществующий, оформлен как #232). `compile_commands.json` пересобран: 31 -> 39 `.cpp`, 9 из `tests/`.
- Пересборка `symbols.json` через clangd MCP (2026-09-28, коммит ниже): 8602 -> **840 символов**, все 31 `.cpp` из `compile_commands.json` (покрытие 31/31). Прежний файл собран regex-фолбэком `generate_symbols.py` и на 60.3% состоял из псевдосимволов (`if` 4399, `for` 693, `switch` 66, `while` 15, `return` 12, `sizeof` 2) плюс вызовов вместо определений. Новый файл собран `textDocument/documentSymbol` по каждому файлу (5 файлов напрямую, 26 — через подагентов), строки переведены 0-based -> 1-based. Виды: Function 569, Method 180, Field 36, Variable 23, Constructor 22, Class 5, Enum 5. Сверка с исходниками: расхождений нет (20 записей вида `Class.field` проверены по имени поля, 3 функции в `TablesNavigator.cpp` начинаются со строки `GSErrCode` из-за `__ACENV_CALL` — это корректно). `callgraph.json` не тронут: 155 рёбер на месте (md5 совпадает). Заодно поправлен `generate_symbols.py`: он больше не затирает `callgraph.json` (коммит `9508f2f`) и пишет в `Docs/_`, а не `docs/`.
- Слияние `llm_test` → `master` (2026-09-28, fast-forward, `e8ff2eb`): документация переведена на схему веток. `AGENTS.md` §17 — снята привязка к `llm_test`/`docs/codebase-map`, грабли про squash заменены правилом «docs живут только в `master`»; §11 — добавлено правило веток (модульные feature-ветки от `master`, документация и мелкие правки в `master`, `Helpers.cpp` под замком). `REPOMAP.md` § Корень и `_progress.md` § Хеш коммита обновлены на тот же хеш. Исторические упоминания `llm_test` в шапках карточек (`ARCHITECTURE.md`, `modules/Sync.md`, `modules/SomeStuff_Main.md`, `modules/json_commands.md`) оставлены: это даты и хеши ревизий, снимаемые при обновлении карточек, а не указание на текущую ветку.
- #221 (2026-09-28, AC25, чекпоинт `309606a`): документирована проверка диапазона при приведении `double` → `Int32`. (1) `CommonFunction.md`: две новые карточки `DoubleToInt32` (CommonFunction.cpp:955, hpp:428) и `DoubleToInt32RoundUp` (:990, hpp:437) с контрактом насыщения и перечнем вызывающих (проверено grep, 22 места). (2) `Helpers.md`: раздел «Приведение double к целым» с перечнем охваченных конвертаций и явно названными неохваченными кастами (нормализованный угол в `CoordNorthAngle`, индексы `API_AttributeIndex`) — диапазон там гарантирован кодом, а не данными. (3) `spec/Spec.md`: контракт `GetParamValue` дополнен приведением `intValue` материала слоя через `DoubleToInt32` (Spec.cpp:1472). (4) `Dimensions.md`: карточка `DimParse` — приведение `dimVal_r` (:307-311). (5) `Roombook.md`: инвариант про подсчёт пробелов/разделителей (:1030, :1040, :1251). Шапки обновлены на `309606a`. `symbols.json`: вручную добавлены две записи (`DoubleToInt32` :955, `DoubleToInt32RoundUp` :990) — регенерация `generate_symbols.py` НЕ выполнялась осознанно: regex fallback не ловит свободные функции с типом возврата (в файле нет даже `is_equal`/`ceil_mod`), переписал бы псевдо-символами и обнулил `callgraph.json`; стиль сохранён (LF, без BOM, без завершающего LF). `callgraph.json` не менялся: новые функции — существующий `CommonFunction.cpp`, рёбра не собирались. Runtime-проверка сообщений `msg_rep` не выполнялась; AC22–24/26–29 не собирались.
- #202 (2026-09-28, AC25, чекпоинт `6c9fc4d`): `ParseFileNumber` (Sync.cpp:90) разбирает числовой аргумент правил `File:` только как полное число; отказ фиксируется в `fileNumberValid` и на границе ветки `SyncString` возвращает `false` (`syncdirection = SYNC_NO` внутри ветки правило не отбраковывает). Обновлены `Sync.md` (таблица API + инвариант) и `TestFunc.md` (кейсы `TestSyncString`; в ArchiCAD не выполнялись — набор отключён `927d2d3`). `symbols.json`: добавлена одна запись `ParseFileNumber` вручную — полная регенерация `generate_symbols.py` не выполнялась осознанно: regex fallback переписал 6396/8599 записей псевдо-символами (`if`/`for`/тип возврата) и обнулил `callgraph.json` (69754 байт → 2). `callgraph.json` не менялся: сигнатуры и связи не изменились, новая функция `static` и вне графа. Существующие устаревшие записи (напр. `Name2Rawname` = 1328 против 1074 в карточке) не сверялись.
- #223 (2026-09-28, AC25, базовый HEAD `9d96d96`): в `Docs/modules/Helpers.md` документирован маршрут `WriteProperty` для уже известного определения без загруженного свойства; в `Docs/_generated/symbols.json` адресно обновлена позиция определения `ParamHelpers::WriteProperty` (Helpers.cpp:5326). Полная регенерация `symbols.json` не выполнялась: общий файл содержит незакоммиченные изменения другой задачи, скрипт также обнуляет `callgraph.json`. Прочие устаревшие записи, существовавшие до #223, не сверялись. Итоговый AC25 runtime/checkpoint — см. IDEA.md #223.
- Документация (2026-09-25, рабочее дерево `llm_test`, HEAD `13c1948`): самостоятельный проход по `Docs/`. (1) Новая карточка `Docs/modules/json_commands.md` для `json_commands/` (модуль AC25+ #205/#206/#207/#208, не был задокументирован): `CommandBase`/`ReadOnlyCommand`/`ModifyCommand`, `RegisterJsonCommands`, `RoomBookCommand`, `SpecCommand`, `HealthCommand` (шаблон, не регистрируется); все строки .cpp проверены grep. (2) `symbols.json` пересобран regex fallback — 8549 символов (было 8544; добавлен `dialogs/OtherDbDialog.cpp`, учтён `json_commands/`); `callgraph.json` сохранён (152→155 рёбер: добавлено `RegisterJsonCommands`→`RoomBookCommand`/`SpecCommand` и `Initialize`→`RegisterJsonCommands`, call_sites 1-based). (3) `REPOMAP.md` и `ARCHITECTURE.md`: добавлены `json_commands/` и `dialogs/OtherDbDialog.*` в структуру, слои и Mermaid-граф; исправлена статистика compile_commands.json (31 запись, 1464 `/I` — было «210/0 include»). (4) `SomeStuff_Main.md`: добавлены `RegisterInterface`/`Initialize`/`FreeData`, карточка `Initialize` (с `RegisterJsonCommands` :569), карта вызовов `MenuCommandHandler` обновлена под дерево `llm_test`; `Sync.md` — шапка (#203/#210) + include `OtherDbDialog.hpp`; `Helpers.md` — строка `GetAttributeValues` (определение .cpp:9577, hpp:748; фильтр `fromAttribDefinition`, #209 open). Не тронуты: code-файлы, `IDEA.md`, `DISCREPANCIES.md` (расхождений не найдено).
- #207/#208/#209 (2026-09-25, рабочее дерево `llm_test`, перед checkpoint): пустое выделение с default-правилом теперь доходит до `SpecArray`; JSON Spec требует `placementPoint`, не открывает интерфейс, возвращает код/счётчики; расширение фильтра `GetAttributeValues` на `fromPropertyDefinition` отменено (#209 — open, фильтр возвращён к `fromAttribDefinition`). Финальный runner AC25 собрал аддон и открыл `test_25.pln`; JSON-вызов создал 34 объекта (548→582), в последних 34/34 заполнено «Спецификации материалов/Наименование в объект». `symbols.json` пересобран regex fallback (8544 символа), `callgraph.json` сохранён. AC26–29 и смешанное создание/изменение — не проверены.
- #206 (2026-09-24, рабочее дерево `llm_test`, перед checkpoint): добавлена `SomeStuffCommand.Spec`, которая загружает текущие `SyncSettings` и вызывает `Spec::SpecAll` на главном потоке. `Docs/modules/spec/Spec.md` обновлён; clang-format, clangd 0 и финальный runner AC25 успешны. HTTP-вызов вернул `status="returned"`, `elapsedSeconds=318.3851546`; корректность созданной спецификации отдельно не проверялась.

- #205 (2026-09-24, рабочее дерево `llm_test`, перед checkpoint): восстановлена минимальная инфраструктура `API_AddOnCommand`, зарегистрирована `SomeStuffCommand.RoomBook`. Финальный runner AC25 успешен; два последовательных HTTP-вызова вернули `status="returned"` за 17.5131455 и 16.9828768 с неизменными итоговыми количествами элементов на втором расчёте.

- #193 (2026-09-24, рабочее дерево `llm_test`, перед checkpoint): исправлено направление словаря в `BuildOtdByParent`: внешний GUID из `SyncGetSubelement` остаётся базовым элементом, внутренний GUID отделки используется для определения `TypeOtd`. Синтетический тест AC25 показал 3 RED до исправления и 5 GREEN после, полный TESTING-набор — 0 `ERROR IN TEST`; финальный runner успешен. На свежем `test_25.pln` два последовательных JSON-запуска RoomBook дали стабильные количества Object=599, Wall=487, Slab=9, Beam=63; второй запуск стен удалил и создал по 430 GUID без роста количества. Обновлены `Roombook.md`/`TestFunc.md`; `symbols.json` пересобран regex fallback (8514 символов), `callgraph.json` сохранён байт-в-байт.

- #203 (2026-09-24, рабочее дерево `llm_test`, без checkpoint): `RunParam` после успешного parameter script окна/двери определяет тип через кросс-версионный `GetElemTypeID(element)` и запускает script marker из непустого `openingBase.markGuid`; GUID marker сначала разворачивается в `API_Elem_Head`. `Sync.md` обновлён; `symbols.json`/`callgraph.json` не менялись — сигнатуры и связи функций не изменены. LightRAG не дал контекст для `RunGDLParScript`/`markGuid` во всех режимах; fallback DevKit AC25/29 подтвердил поле и старый API contract. clangd 0, `BuildAddOn.py -v 25` success; runtime marker — не проверено.

- #199 (2026-09-24, рабочее дерево `llm_test`, без коммита): `SyncCalcRule` обходит внешний целевой GUID из `WriteDict`; синтетический TESTING-кейс AC25 показал RED→GREEN, тесты прошли по маркерам панели «Отладка». `BuildAddOn.py -v 25` и `restart_archicad_for_test.ps1` завершились успешно; фактическая запись на PLN — не проверено. Обновлены `Sync.md`/`TestFunc.md`; `symbols.json` пересобран только для этих двух файлов из regex fallback (8488 записей), сохранены записи Roombook и весь callgraph.

- #195 (2026-09-24, рабочее дерево `llm_test`, перед checkpoint): `RoomBook` разделён на контекст, сбор элементов, чтение параметров, обработку отделки, свод/запись материалов, сбор GUID на удаление и резервирование (см. `Docs/modules/Roombook.md`). `symbols.json`: regex fallback выполнен (8514 символов), `callgraph.json` сохранён байт-в-байт. clang-format, clangd 0, финальный AC25 runner и два последовательных runtime-вызова RoomBook успешны; второй расчёт сохранил количества Object/Wall/Slab/Beam без накопления элементов.

## Где ведётся работа
Код и документация `Docs/` ведутся в `master`; глобальные доработки по модулям — в отдельных feature-ветках от `master` (AGENTS.md §11, §17).

## Хеш коммита
`9a6efa5` (2026-09-30, #231 закрыт; дальше — R5.5 Spec-рефакторинг)

## Список модулей

| Модуль | Каталог | .cpp файлов | .h/.hpp файлов | Примерная сложность |
|--------|---------|-------------|----------------|---------------------|
| Core/Helpers | Sources/AddOn/ (Helpers.cpp/hpp) | 1 | 1 | Большой |
| Propertycache | Sources/AddOn/Propertycache.cpp/hpp | 1 | 0 | Средний |
| Sync | Sources/AddOn/Sync.cpp/hpp | 1 | 1 | Большой |
| SomeStuff_Main | Sources/AddOn/SomeStuff_Main.cpp/hpp | 1 | 1 | Маленький |
| CommonFunction | Sources/AddOn/CommonFunction.cpp/hpp | 1 | 1 | Большой |
| Dimensions | Sources/AddOn/Dimensions.cpp/hpp | 1 | 1 | Маленький |
| MEPv1 | Sources/AddOn/MEPv1.cpp/hpp | 1 | 1 | Большой |
| ReNum | Sources/AddOn/ReNum.cpp/hpp | 1 | 1 | Средний |
| Roombook | Sources/AddOn/Roombook.cpp/hpp | 1 | 1 | Огромный |
| Summ | Sources/AddOn/Summ.cpp/hpp | 1 | 1 | Маленький |
| ClassificationFunction | Sources/AddOn/ClassificationFunction.cpp/hpp | 1 | 1 | Маленький |
| Constants | Sources/AddOn/Constants.hpp | 0 | 1 | Справочник |
| tests/TestFunc | Sources/AddOn/tests/TestFunc.cpp/hpp | 1 | 1 | Маленький (реестр 49 наборов) |
| tests/TestKit | Sources/AddOn/tests/TestKit.cpp/hpp | 1 | 1 | Средний |
| tests/Test{Spec,Sync,Param} | Sources/AddOn/tests/Test{Spec,Sync,Param}.cpp | 1 | 1 | Огромный / Большой |
| tests/Test{Format,Renum,Core,Util} | Sources/AddOn/tests/Test{Format,Renum,Core,Util}.cpp | 1 | 1 | Маленький |
| dialogs/BrowserPalette | Sources/AddOn/dialogs/BrowserPalette.cpp/hpp | 1 | 1 | Большой |
| dialogs/CommandHelpers | Sources/AddOn/dialogs/CommandHelpers.cpp/hpp | 1 | 1 | Маленький |
| dialogs/DG4rule | Sources/AddOn/dialogs/DG4rule.cpp/hpp | 1 | 1 | Маленький |
| dialogs/SyncSettings | Sources/AddOn/dialogs/SyncSettings.cpp/hpp | 1 | 1 | Большой |
| spec/Spec | Sources/AddOn/spec/Spec.cpp/hpp | 1 | 1 | Огромный |
| spec/Spec_libpart | Sources/AddOn/spec/Spec_libpart.cpp/hpp | 1 | 1 | Маленький |
| table/TableRenderer | Sources/AddOn/table/TableRenderer.cpp/hpp | 1 | 1 | Средний |
| table/TablesNavigator | Sources/AddOn/table/TablesNavigator.cpp/hpp | 1 | 1 | Большой |
| pk/AutomateFunction | Sources/AddOn/pk/AutomateFunction.cpp/hpp | 1 | 1 | Средний |
| pk/ResetProperty | Sources/AddOn/pk/ResetProperty.cpp/hpp | 1 | 1 | Маленький |
| pk/Revision | Sources/AddOn/pk/Revision.cpp/hpp | 1 | 1 | Средний |
| third_party/qrcodegen | Sources/AddOn/third_party/qrcodegen.cpp/hpp | 1 | 1 | Большой |
| json_commands | Sources/AddOn/json_commands/*.cpp/hpp | 5 | 5 | Маленький (AC25+) |

## Статус фаз

- [x] Фаза 0: Разведка (clangd MCP работает, compile_commands.json найден, модули выделены)
- [x] Фаза 1: Символы собраны через скрипт (docs/tools/generate_symbols.py → _generated/symbols.json)
- [x] Фаза 2: Пилот — модуль pk/ задокументирован в docs/modules/pk.md
- [ ] Фаза 3: Остальные модули
- [ ] Фаза 4: Обзор
- [ ] Фаза 5: Сверка

## Порядок документирования (Фаза 3)
1. pk/ — наибольший входящий спрос (вызывается из Many)
2. dialogs/BrowserPalette — точка входа UI
3. dialogs/SyncSettings — настройки
4. spec/Spec — правила
5. table/TableRenderer — таблицы
6. table/TablesNavigator — навигатор
7. dialogs/CommandHelpers
8. dialogs/DG4rule
9. spec/Spec_libpart
10. ReNum
11. Summ
12. CommonFunction
13. MEPv1
14. ClassificationFunction
15. Roombook
16. SomeStuff_Main
17. TestFunc
18. Propertycache
19. Sync
20. Dimensions
21. third_party/qrcodegen
22. Constants (справочник)

## Замечания
- compile_commands.json покрывает 25 .cpp файлов (не .h/.hpp)
- Связь между модулями: Helpers → Propertycache → Sync → Core
- BrowserPalette использует JS bridge для HTML-интерфейса
- Roombook — самый большой модуль (5686 строк в Namespace)

## Инфраструктура (FIX: compile_commands.json + callgraph)

### Диагностика (Step 1)

**compile_commands.json СОДЕРЖИТ include-пути:**
- Всего записей: 25 (1 cmake_pch + 24 source)
- С полем `command` (string): 25/25
- С полем `arguments` (array): 0/25
- Всего `/I` флагов: 1464
- Уникальных путей: 61
- **Отсутствующих путей: 0** (все существуют на диске)

**clangd диагностика Helpers.cpp:**
- Errors: 20 (все "Expression result unused" / "Unused variable" — стандартные warning'ы компилятора)
- Warnings: 3 (unused-includes)
- **"file not found" errors: 0** — clangd корректно резолвит все заголовки

### Причина пустого callgraph.json

Не в compile_commands.json (он корректен). Причина: `docs/tools/generate_symbols.py` НЕ реализует сбор callgraph — всегда записывает `[]` в `callgraph.json`. Необходимо добавить `textDocument/callHierarchy` запросы.

### Корень проблемы: CMake
- `CMakeLists.txt` line 23: `set(CMAKE_EXPORT_COMPILE_COMMANDS ON)` — включено
- `Tools/BuildAddOn.py` имеет `--lsp` флаг: генерирует compile_commands.json через CMake и копирует в корень (BuildAddOn.py lines 56-88)
- Текущий `compile_commands.json` был сгенерирован CMake в `Build/LspCompileCommands/25/` и скопирован в корень
- Сгенерированный файл использует `command` (string), не `arguments` (array)

### Версия
- AC25 (DevKit-25): `D:/SomeStuff_addon/Build/DevKit/APIDevKit-25/`

### Что исправлено / как починено (Steps 2–4)

**compile_commands.json: исправление НЕ потребовалось** — premise задачи (0 include-путей) не подтвердилась. Файл сгенерирован штатно: `BuildAddOn.py --lsp` → CMake (`CMAKE_EXPORT_COMPILE_COMMANDS ON`, CMakeLists.txt:23) → копирование из `Build/LspCompileCommands/25/` в корень. Проверка clangd-диагностикой Helpers.cpp: 0 ошибок "file not found" (20 ошибок — только -Wunused-value/-Wunused-variable, которые clangd трактует как errors, MSVC прощает).

**callgraph.json пустой по другой причине**: `generate_symbols.py` никогда не реализовывал сбор callHierarchy — всегда писал `[]`. Дополнительно скрипт не может общаться с clangd через subprocess.PIPE на Windows (clangd 22.1.8: нет `--port`, pipe даёт пустой greeting / Errno 22; проверено 3 способами). Сбор выполнен напрямую через clangd MCP (`get_call_hierarchy`) на позициях определений из symbols.json.

**Регенерация callgraph** (для будущих запусков): `get_call_hierarchy` на позиции имени функции в определении (колонка имени, не тела); SDK-рёбра фильтровать по пути `Sources/AddOn/`.

### Проверка (Step 3)
- Helpers.cpp: 0 "file not found" ✓
- Все 1464 `/I` путей из 24 source-записей существуют на диске ✓
- callHierarchy работает (clangd MCP): 7 из 9 запрошенных функций pk/ резолвятся

### Список неполных записей
- `ResetProperty` (ResetProperty.cpp:14), `Revision::SetRevision` (Revision.cpp:14) — clangd: "No call hierarchy available at this position"; не блокирует остальное
- SDK-рёбра (DevKit/MSVC/Windows Kits) не развёрнуты в edges — только счётчики outgoing_count

### Новая статистика
- `callgraph.json`: 29 рёбер внутри проекта (было 0), 9 функций опрошено, 7 резолвится
- `symbols.json`: 8421 символов, 17 модулей (не менялось)
- `pk.md`: добавлены поля «Вызывает»/«Вызывается из» для 9 функций AutomateFunction (не тронуты остальные разделы)

## Фаза 5: Сверка (2026-09-22, коммит f8f599c)

- [x] Фаза 3: все 23 модуля имеют Docs/modules/*.md; закоммичено (f8f599c + недостающие Dimensions/TestFunc/third_party/Helpers)
- [x] Фаза 5: 10 записей сверены с кодом — см. Docs/DISCREPANCIES.md
- Итоговый отчёт: см. ниже

### Итоговый отчёт (фаза 5)

- Модулей задокументировано: 23 файла в Docs/modules/ (включая вложенные dialogs/, spec/, table/, third_party/)
- Покрытие symbols.json (8421 символ, 17 модулей): пофункциональные записи приведены для pk/ (шаблон с «Вызывает»/«Вызывается из»), таблицы API — для остальных
- **Неполное покрытие** (честная пометка): Helpers (2145 символов) и TestFunc (682) — только ключевые типы и таблица API из заголовочных комментариев, без пофункциональных записей; third_party — внешняя библиотека, не документируется пофункционально
- `[не проверено]`/not verified записи: ResetProperty (callHierarchy), Revision::SetRevision (callHierarchy), Helpers.cpp:1761-мемо (ссылка на ревью, статус «исправлено» подтверждён grep'ом)
- Файлы вне compile_commands.json (не индексируются clangd): заголовки без единицы трансляции (все .hpp/.h, Constants.hpp) — индексируются как заголовки через включения; отдельной проблемы не зафиксировано
- Расхождения: 2 активных косметических (TestFunc.hpp:14, Sync.hpp:11), 5 исторических подтверждены исправленными — Docs/DISCREPANCIES.md

### Статус фаз (обновление)
- [x] Фаза 3: модули задокументированы и закоммичены
- [x] Фаза 4: ARCHITECTURE.md, REPOMAP.md созданы; Mermaid-граф зависимостей — см. ARCHITECTURE.md; сценарии — список предложен, ожидает согласования
- [x] Фаза 5: сверка выполнена, DISCREPANCIES.md заполнен

## Доработка пилотного pk.md по замечаниям (2026-09-22)

- Проблема 1: все описания в таблицах и карточках pk.md помечены [из комментария]/[по коду]/[не проверено]; `not verified` заменено на `не проверено`
- Проблема 2: добавлены 16 карточек нетривиальных функций (5 AutoFunc с побочными эффектами, 6 ResetProperty-семейства, 5 Revision); в карточке ResetProperty зафиксирован баг AC27+ (return false) как актуальный
- Проблема 3 (проверено): строки в прежней таблице были строками .hpp; реальные определения в .cpp (1-based): GetNear 15, GetCuplane 34, Get3DProjectionInfo 149, Get3DDocument 208, GetSectLine 260, DoSect 406, PlaceDocSect 489, ProfileByLine 556, AlignOneDrawingsByPoints 771, GetDrawingsSort 892, AlignDrawingsByPoints 926 — подтверждены grep'ом по AutomateFunction.cpp и согласуются с clangd callHierarchy (MCP отдаёт 0-based)
- ResetProperty/SetRevision: clangd callHierarchy не резолвится (2 столбца каждый) → помечено `не проверено` в карточках

## Применение исправленного шаблона ко всем модулям (2026-09-22, после приёмки пилота)

- Все 23 модуля Docs/modules/*.md переписаны по принятому шаблону пилота:
  пометки источника [из комментария]/[по коду]/[не проверено] на каждом описании;
  номера строк — определения в .cpp (проверены grep для Summ, Sync, ReNum, Spec, SomeStuff_Main, CommonFunction, Propertycache; для остальных строка помечена «не проверена» вместо догадки);
  карточки нетривиальных функций с побочными эффектами (2-5 на модуль: SyncAndMonAll/SyncData/SyncElement, SumSelected/Sum_OneRule, ReNumSelected/ReNumOneRule, SpecAll/PlaceElements, Update/GetPropertyRuleFlag, UnhideUnlockElementLayer/EvalExpression, SetAutoclass, SelectionChangeHandler/Show, GetSyncSettingsCache, TableRenderer::Draw/ComputeLayout, DimParse/DimAutoRound)
- Helpers и TestFunc — честно помечены как неполное покрытие (2145/682 символов) в шапке и здесь
- Чего не хватает: callHierarchy для модулей кроме pk (сбор через clangd MCP не выполнялся) — в карточках соответствующие поля помечены [не проверено]

## Grep-fallback для callHierarchy (2026-09-22)

- clangd callHierarchy/find_references на позициях определения ResetProperty.cpp:14 и Revision.cpp:14 не резолвятся (callHierarchy: "No call hierarchy"; find_references: либо пусто, либо 719 чужих ссылок из DevKit-заголовков — неверный символ, по 2 попытки на каждый)
- Fallback: grep по Sources/AddOn (по предложению пользователя):
  - `ResetProperty()` вызывается из `SyncAndMonAll` (Sync.cpp:174, при успехе сброса — ранний выход)
  - `Revision::SetRevision()` вызывается из `MenuCommandHandler` (SomeStuff_Main.cpp:449, case SetRevision_CommandID)
  - ChangeMarkerText/ChangeMarkerTextOnLayout/ChangeLayoutProperty внешних вызовов не имеют (только внутри pk/Revision.cpp)
- Обновлены карточки ResetProperty и SetRevision в pk.md

## Финальный отчёт (2026-09-22, задача документирования завершена)

- Grep-карта вызовов входных функций (все проверены по коду):
  SumSelected ← Main:433; SyncAndMonAll ← Main:405; SyncSelected ← Main:410 (+Sync.cpp:565/2246); RunParamSelected ← Main:437; SyncShowSubelement ← Main:445; ResetProperty ← Sync.cpp:174; SetRevision ← Main:449; SyncSetSubelement ← Main:453; SpecAll ← Main:441; ReNumSelected ← Main:427; ProfileByLine ← Main:462; AlignDrawingsByPoints ← Main:465; MonAll ← Main:400/555; DimRoundAll ← Main:128/165/489; GetAllClassification ← Propertycache.hpp:613; ReadMEP ← Helpers.cpp:5747 (+Propertycache.cpp:149)
- Исправлены номера строк SomeStuff_Main: MenuCommandHandler=366, ProfileByLine=462, AlignDrawingsByPoints=465 (pk.md + callgraph.json)
- Критерии готовности: модулей 23/23, описания с пометками, DISCREPANCIES.md заполнен, вне Docs/ и AGENTS.md изменений нет
- Открытые пункты (не блокируют): список ключевых сценариев в ARCHITECTURE.md ожидает согласования для sequence-диаграмм; Helpers/TestFunc — неполное покрытие; callHierarchy для остальных модулей не собирался (clangd mis-resolve на определениях — обход grep'ом)

## Решение по приоритетам: callgraph точечно (2026-09-22)

- Callgraph собран только для pk/ (29 рёбер, 9 функций). Для остальных 22 модулей «Вызывает»/«Вызывается из» помечены [не проверено] — осознанно.
- Решение: расширять callgraph НЕ на все модули, а точечно под модуль, где профилированием всплыла горячая функция (полный проход ~ тот же объём, что ручной сбор pk; профилирование скорее всего будет по отдельным горячим модулям).
- Процедура точечного сбора — Docs/tools/UPDATE_PROCEDURE.md §2.
- generate_symbols.py callgraph не автоматизирует (subprocess к clangd не работает на Windows) — подтверждено, это не недоделка, а ограничение окружения.

## Точечный callgraph Sync (2026-09-22, коммит de8e94b) — первый прогон политики

- Sync выбран как первый горячий модуль (все команды меню и события изменения элементов проходят через него)
- clangd callHierarchy: все 6 функций резолвятся (MonAll, SyncAndMonAll, SyncByType, SyncElement, SyncData, ParseSyncString). Важно: MCP ждёт 0-based строки — значения из grep (1-based) давать как line-1; col = позиция имени функции
- 49 новых рёбер; итого 78 в callgraph.json
- Открытия: MonAll вызывается не только из меню, но и из Initialize (SomeStuff_Main.cpp:555); ParseSyncString активно тестируется (TestParseSyncStringIndependent, 7 вызовов); SyncAndMonAll сначала зовёт ResetProperty и при успехе делает ранний выход
- Синхронизация координат: в callgraph.json — 0-based (clangd), в modules/*.md — 1-based (grep); правило записано в meta.lines

## Полный проход callgraph по оставшимся модулям (2026-09-22)

- Решение «точечно» перевыполнено командой пользователя: прогнан callHierarchy по всем модулям
- Опрошено 30 функций, резолвится 24; callgraph.json — 152 ребра (было 78)
- Не резолвятся (по 2 попытки, стоп): ReadMEP (MEPv1.cpp:177), RoomBook (Roombook.cpp:56), GetSyncSettingsCache (SyncSettings.cpp:417), ElementEventHandlerProc (Main:141), ResetProperty/SetRevision (известно ранее) — вызывающие известны из callHierarchy MenuCommandHandler и grep, поля в карточках заполнены с пометкой источника
- Открытия: DimRoundAll дёргается из ТРЁХ обработчиков (меню :489, ElementEvent :165, ProjectEvent :128); GetPropertyRuleFlag — только тестовые вызовы (DISCREPANCIES #9); RoomBook entry — Roombook.cpp:57; LoadSyncSettingsFromPreferences — clangd даёт SyncSettings.cpp:518, grep 429 (два совпадения)
- Обновлены: Summ, ReNum, Spec, Dimensions, ClassificationFunction, TableRenderer, Propertycache, SomeStuff_Main, SyncSettings, BrowserPalette, Roombook, MEPv1

## Пункты 2-3 закрыты (2026-09-22)

- П.2 (неполное покрытие): TestFunc — все 33 функции из hpp; Helpers — полный API по объявлениям hpp (FormatStringFunc, топ-уровень, ParamHelpers чтение/запись/конвертации, операторы); Roombook — все ~70 функций из documentSymbols по подсистемам. Остаток «неполного покрытия» снят.
- П.3 (нерезолвящиеся): тела 6 функций разобраны body-scan (Docs/tools/_body_scan.py, brace-matching + whitelist имён проекта) → Docs/_generated/body_scan.json; «Вызывает» заполнены в pk.md (ResetProperty, SetRevision), MEPv1 (ReadMEP: до AC28 false, AC28+ GetMEPData), Roombook (RoomBook — 96 вызовов), SyncSettings (GetSyncSettingsCache → ReadSyncSettings), SomeStuff_Main (ElementEventHandlerProc — диспетчер всей функциональности).
- Находки: ConvertToProperty с TODO «переписать под ParamValue» (Helpers.hpp:419); дубликат объявления CompareParamDictValue (Helpers.hpp:427/432); у ResetProperty body-scan обрезался по внешней скобке — ResetPropertyElement2Defult подтверждён чтением (:34).

## Пункт 1 закрыт: sequence-диаграммы (2026-09-22)

- Набор 10 сценариев подтверждён пользователем; диаграммы — Docs/SEQUENCES.md (Mermaid, все сообщения из callgraph.json/body_scan.json)
- ARCHITECTURE.md ссылается на SEQUENCES.md
- Задача документирования: все фазы и все открытые пункты закрыты (кроме мелких пометок [не проверено] на SDK-вызовах внутри карточек)

## Пункт 1: сценарии согласованы, sequence-диаграммы построены (2026-09-22, коммит 397acd6)

- Все 10 сценариев подтверждены пользователем; sequenceDiagram (Mermaid) добавлены в ARCHITECTURE.md — построены из callgraph.json + body_scan.json, номера строк 1-based
- Задача документирования полностью завершена: 23/23 модулей с полным покрытием, 152 ребра callgraph + body_scan для 6 функций, DISCREPANCIES (9 записей), 10 sequence-диаграмм, регламент обновления (UPDATE_PROCEDURE.md)
