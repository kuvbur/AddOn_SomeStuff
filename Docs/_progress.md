# Состояние документации — актуально

Единственный источник возобновления работы по документации. История задач — в git-логе
`Docs/` и в архиве задач владельца (`IDEA.md`/`IDEA_ARCHIVE.md`); здесь — только текущее
состояние и процедура.

## Видимость BrowserPalette (2026-11-12)

- Карточка дополнена разделением явной пользовательской видимости и временного скрытия Archicad. Только BrowserPalette-срез symbols.json обновлён по свежему isolated documentSymbol: 35→36 записей, включая SetUserVisible; остальные записи/порядок сохранены. Shared MCP вернул прежний AST без нового метода и не использовался; callgraph исторический.
- Controlled exact menu/close/callback/SetUserVisible, clang++ C++14: RED22/9 → GREEN22/0. DG/UI/settings — подменённые границы, не native-приёмка. Полная TU baseline/current SDK25 MSVC C++14 /W3 /WX /c успешно скомпилирована в scratch OBJ, source/header hashes сверены. Fresh isolated LSP имеет одинаковые три сообщения: старые Wswitch/unused-but-set-variable и unused Sync.hpp, новых нет. APX/link/restart/commit/push и GitHub-операции исключены по scope/«без issue».
- Evidence: Hermes scratch/palette-visibility/ (red/green, offline-baseline/current, offline-comparison.json). Native-переключение видов и другие SDK/macOS not verified.
- Финальное ревью нашло stale-перезапись в MenuCommandHandler; локальная копия перечитывается только в ветке палитры, exact outer handler RED7/2 → GREEN7/0. Повторное независимое ревью: No issues found in inspected scope. Main baseline/current TU SDK25 C++14 /WX exit0; fresh LSP — одинаковые два unused-includes. Parent повторил полную BrowserPalette TU после параллельной #323: fe442fdd/header d42375c4, MSVC exit0/OBJ создан, LSP три сообщения; controlled palette22/0 и outer menu7/0 повторены на live. Адресно обновлены 12 Main и прежние 36 palette-записей по isolated documentSymbol; новые символы #323 остаются её задачей. APX/native не выполнялись.

- Приёмка владельца: «коммить и закрывай issue - я проверил». Закрытие по этой проверке; отдельный final runner агента не выполнялся. Коммит включает только видимость, без параллельной #323.

## Неблокирующий partition (#322, 2026-11-11)

- Roombook.md дополнена контрактом локальных ошибок source/zone-face, исключения всей конфликтной группы, честного complete и неблокирующего отчёта caller-а. Штатный cleanup не меняется; сохранность прежней отделки отсутствующего участка не гарантируется. [по коду]
- Parent-run SDK25 Geometry/GSRoot/C++14: дословные helpers/partition/PreparePartitions, RED71/24 → GREEN71/0. Preparation/report и throw отдельной грани — controlled seams, не native. Семь новых постоянных assertions извлечены из TestRoombookGeometry и исполнены: RED7/7 → GREEN7/0. Test TU с TESTING и /WX скомпилирован в scratch OBJ. Canonical BuildAddOn.py -c config.json -v25 -r -l INT успешен, Data compile/APX link подтверждены логом. Data SHA57ad0a0aa0d67435517a1f42a1cd1451fb511cae49a62bd0ba708c613557a320, test SHA4cd20afa17aee7fb1b939b5fea07d235b75547a807b609e4ea3f8a16a1c50a6a; постороннее форматирование preprocessor возвращено к baseline, изменённый блок отдельно отформатирован, TU/постоянные assertions/LSP и canonical build повторены на финальном файле.
- Fresh isolated LSP version1 baseline/live: одинаковые восемь диагностик (три unused-variable и пять unused-includes), test без диагностик; header проверен через обе реальные TU. Source hashes совпали до/после и с parent-check; namespace Data end9821/file9822 подтверждает свежесть AST. Baseline clangd --check ранее дошёл до AST, затем timeout на token feature pass; не объявлен pass. Независимое read-only ревью deleg_c2267fc8: No issues found in inspected scope. В symbols.json адресно обновлены три production-функции (имя/координата по fresh documentSymbol); обе test-функции сверены, остальные записи/порядок и исторический callgraph сохранены. Полного сбора не было.
- Native-сложный профиль, загрузка нового APX, другие SDK/macOS не проверены; host не перезапускался, оба PLN и загруженный Debug APX по SHA256 неизменны. Evidence: Hermes scratch/partition-nonblocking-20261111/. Commit/push не запрошены, #322 OPEN.

## Свойства плинтусов (#308, 2026-11-10)

- Независимое read-only ревью deleg_1f408400: No issues found in inspected scope; SHA Draw совпадает с проверенной компиляцией/регрессией и текущим файлом. Wiki RU: две строки в существующей таблице свойств элементов, независимость/сброс и явные обязательные GUID-связи. Маркеры сверены с фактическим propdesc_prefix_element: some_stuff_element_tip_skirting и some_stuff_element_length_skirting; те же литералы в карточке и body #308, remote read-back OPEN. Wiki сохранила UTF-8/no BOM/CRLF и ссылки; кроме вставок изменена лишь уточняющая фраза о GUID-свойствах.
- Roombook.md дополнена контрактом OtdSlab_WriteSkirt: независимые свойства типа/длины, guard успешности целевого пола/потолка с no-op, нулевая длина/очистка типа, прежняя общая запись. Исходник a6bb81c479f77b3791075e8549d73521923cfea35e2f075b111229c3c38d5ff3, рабочая дельта к 00c6dcc. Вне helper байты совпадают с пользовательским baseline; Constants/Data не изменены агентом.
- Parent-run controlled MSVC/C++14 /WX: RED295/101FAIL → GREEN295/0, characterization40/0 в обоих вариантах и строки совпадают. Дословно извлечены helper и AddParamValue2ParamDictElement; GS/типы/конвертеры — stand-ins, не SDK/native-приёмка. Полная TU SDK25/MSVC/C++14 /WX успешно скомпилирована в scratch OBJ; APX не собирался. Fresh isolated publishDiagnostics version1 baseline/live имеют одинаковые пять unused-variable и один unused-include; в изменённой функции диагностик нет. Baseline-попытка с clang-only PCH wrapper дала C4068/C2220; MSVC затем использует его фактический include Tools/AddOn.hpp без подавления предупреждений. Хост и проектные настройки не менялись.
- symbols.json: только Draw-срез 48→49 записей по fresh isolated clangd documentSymbol, все name/line joins сверены, 1031 внешняя запись и порядок сохранены. Сборщик учитывает Function и Variable; первая проверка числа без kLineEps остановилась до записи, затем типовая схема исправлена. Shared MCP известен устаревшим, его координаты не использованы. Callgraph и остальные исторические координаты не пересобраны. Evidence: Hermes scratch/skirting-properties-308/ (regression/, validation/, own.diff, symbols-acceptance.json). Native-персистентность, canonical build/load, другие SDK/macOS не проверены; VS Run/ArchiCAD25 не прерывался, commit/push не выполнялись.

## Безопасный отказ Wall writer (#317, 2026-11-08)

- Карточка Roombook описывает require_rollback, границу ordinary/fatal failure, cleanup только новой стены и передачу ошибки всеми тремя циклами в существующий undo callback. [по коду]
- SDK25/C++14 MSVC, пять полных неизменённых извлечённых функций: RED205/117FAIL → GREEN205/0; characterization105/0 на baseline и live. Родитель повторил четыре suite-запуска, проверил 41 hash manifest и пять полных extraction hashes. API failures/undo snapshot контролируемые, не native-приёмка. Canonical BuildAddOn.py -c config.json -v25 -r -l INT exit0, Draw перекомпилирован/APX перелинкован. Оба PLN и загруженный Debug APX по SHA256 неизменны.
- Fresh isolated clangd version1: baseline/live имеют одинаковые пять unused-variable и один include hint; новых диагностик нет. Shared MCP stale (namespace end2748 против реального2996), его нулевые диагностики не приняты. Независимое ревью production-дельты: No issues found in inspected scope.
- symbols.json: 48 Draw name/range joins со свежим documentSymbol, 22 координаты адресно обновлены; внешние записи/порядок на момент записи сохранены. Callgraph не пересобран и не объявляется актуальным. Evidence: Hermes scratch/roombook-317-20261108/ (regression/, lsp/, parent-rerun.json, parent-validation.json, symbols-acceptance.json, build-ac25-int.log). #317 ждёт native-восстановления стены/проёмов и согласованного runner; commit/push/restart не выполнялись, другие SDK/macOS not verified.

## Конвертация дуг Polygon2D (#298, 2026-11-08)

- Адресно дополнены CommonFunction/TestFunc: dest-first копирование со смещением Geometry arcs +1, постоянная реальная SDK25-регрессия и регистрация core. RED110/22 → GREEN134/0; 24 обратные проверки на RED пропущены ради безопасности. Canonical AC25 Windows RelWithDebInfo INT build успешен; полные TU теста/registry с TESTING MSVC/C++14 /WX exit0. Native writers и полный host-прогон не исполнялись. [исходник, SDK25, выполненные build/stand]
- Fresh isolated clangd version1: converter имеет те же 5 unused-диагностик/2 include hints на baseline/live; тест/registry без ошибок. Standalone TestFunc.hpp без PCH не самодостаточен, ошибки API-типов не выдаются за дефект новой декларации: реальные TU скомпилированы. Проверены точные function name/range joins; anonymous namespace placeholder и header-declaration end tokens исключены из проверки окончания тела.
- symbols.json: адресно уточнены два converter-определения и добавлены 5 записей нового test TU по fresh documentSymbol; остальной generated-срез и исторический callgraph не пересобирались. Evidence: Hermes scratch/issue298/ (baseline/, candidate/, fresh-lsp/, baseline-lsp/, full-test-tus/, build-ac25.log). Без commit/push/restart; обе PLN-копии по SHA256 неизменны.

## Неблокирующий отказ Mesh-пола / Roof (#321, 2026-11-07)

- Адресный контракт Roombook: поэлементные ошибки Roof, отсутствие ложного success, продолжение остальных writers/комнат и отложенный cleanup старого Mesh-остатка с сохранением при отказе того же zone/base/role. Диагностика указывает избранное и composite без изменения атрибута. [по коду]
- Исходный отказ зоны113 воспроизведён под VS: composite153 разрешён только для Slab, Roof_Prepare возвращает APIERR_BADPARS до Create, старый batch даёт rooms[]. Canonical AC25 Windows Debug BuildAddOn.py-v25 exit0; источник a24c88e8/APXfc7c51f2 и оба сохранённых PLN сверены до/после F5/native. GREEN113:26результатов/22success/4error, прежние4Roof сохранены. GREENall:25зон/1172результата/1168success/4error только113;24другихзоны обработаны, successful GUID прочитаны независимо. Геометрическая полнота и native-инъекция Create/Change не заявляются.
- Fresh isolated clangd version1: шесть прежних диагностик baseline (пять unused-variable, одно unused-include), новых нет; parent сверил manifest,16hashes и49selection/range/name joins, namespace end2972. Только Draw-срез symbols.json42→48,1068→1074;1026внешних записей/порядок сохранены. Добавлены новый cleanup-helper и прежние пропуски, включая forward declaration. Остальные координаты/callgraph исторические, не из этой коллекции. Независимое ревью дельты без дефектов.
- Дополнительный SDK25/C++14 controlled stand остановлен по two-failure guard: сначала ошибки seams harness, затем LNK2019 ACAPI_Element_Tool; проверки НЕ исполнялись и RED/GREEN не получены. Это не отказ canonical сборки или уже пройденной native-приёмки. Артефакты и blocker сохранены, очередной retry не выполнялся.
- IDEA321 архивирован как завершённое исходное исправление с явными границами. Issue321OPEN; итоговый comment6705435769 прочитан обратно, код и документы включены в текущий коммит по команде владельца; push/Save не выполнялись. Во время native-приёмки host оставлен Run, изменения только в памяти,8исходныхBP восстановлены, собственных нет. Другие SDK/macOS/final issue-closing runner не проверены. Evidence: Hermes scratch/roof-nonblocking/ (green*-acceptance.json, build-ac25.log, validation/parent-acceptance.json, regression/blocked-report.json).

- Перед коммитом владелец оставил только английский текст диагностики. Единственная дельта к native-снимку a24c88e8 — это сообщение; текущий Draw11bfe814 проверен повторным canonical AC25 Debug build exit0, APXaa2e7e14. Новый native-прогон не выполнялся, host закрыт; оба PLN восстановлены побайтово. 48 Draw-координат перенесены по точному diff и проверены source/name join, внешний generated-срез и порядок неизменны; fresh LSP выше относится к прежнему native-снимку.

## Исправление округления Spec (#318, 2026-11-07)

- Область: AC25 Windows/C++14, локальный RoundFinalSums и постоянные TestSpecRounding. Устранены три замечания e84e042: нижнее кратное из-за широкого допуска, несогласованные int/text и пропуск raw-ноля. Карточка SpecPlanning отражает текущий контракт: одно-ULP защита кратного, обработка underflow и согласование представлений штатным форматированием. [по коду]
- SDK25 stand RED649/24FAIL → GREEN649/0 (TestSpecRounding136, соседние513); parser815/0. Пропущены native-localized Bool case и4 AddRule/native assertions. Полные TU SpecPlanning/Spec/TestSpec MSVC/C++14 exit0; fresh version1 LSP:0ошибок,5прежних unused-includes;169диапазонов проверены с учётом anonymous placeholders и header declarations. Независимое ревью production-дельты без замечаний; последние3табличных кейса после ревью проверены parent. Native persistence не исполнялся.
- Canonical Tools/BuildAddOn.py -c config.json -v25 -r -l INT выполнен после финальной правки: exit0, AC25 Windows RelWithDebInfo INT, APX перелинкован. Загруженный Debug APX и оба test_25.pln по SHA256 неизменны; host/модель не тронуты. Другие SDK/macOS not verified.
- symbols.json:85записей двух изменённых .cpp адресно приняты из свежего isolated clangd documentSymbol с hash/range/name guards;983внешних записей и порядок сохранены. Добавлены недостающие SpecFixture::Real и TestSpecRounding:1066→1068. Shared MCP stale (RoundFinalSums отсутствует, конец namespace474 вместо541); его координаты не публиковались. Callgraph и остальные исторические координаты не пересобирались и не объявляются актуальными.
- #318 OPEN до согласованной native-проверки фактической записи и повтора; body и итоговый comment6704764673 прочитаны обратно. Исправления включены в текущий коммит; push/Save/restart/Stop не выполнялись. Evidence: Hermes scratch/spec318-fix/ (regression/{red-final,green-final}, validation/, parser/green/, build-ac25-final.log, symbols-acceptance.json).

## Минимальная площадь отделки (#320, 2026-11-06)

- Адресный контракт Roombook: финальная фильтрация всей площадной отделки по `min_area`, сохранение равенства, возврат неиспользованных slab GUID и малые contact-диагностики. [по коду]
- symbols.json: семь затронутых функций адресно сверены с новым isolated clangd documentSymbol (Data SHA256 `1d64becff1ee5a8a5a4322484532f1197616fc7fe68a3579dfa9659eac9e97fc`); добавлены четыре отсутствовавшие записи, три существовавшие переадресованы. Остальные записи/порядок, включая параллельный Spec-срез, сохранены; унаследованные координаты и callgraph не пересобирались и не объявляются актуальными.
- Standalone C++14: одинаковые assertions RED232/57FAIL → GREEN232/0, parent повторил GREEN. Controlled stages/SDK substitutes, не native-геометрия. Реальный полный Data TU SDK25/MSVC `/std:c++14 /c /WX` exit0, scratch obj; новый clangd имеет те же восемь диагностик, что baseline, новых нет. Canonical addon build/link и native не выполнялись: активный VS Run не останавливался.
- Evidence: Hermes scratch/min-area-20261106/regression/ и validation/. Без коммита/push/Save/restart.

## Необязательный шаг в s() Spec (#318, 2026-11-06)

- Карточка Spec актуализирована только для parser-only контракта: две обязательные схемы и необязательный положительный конечный шаг, отдельная ошибка InvalidRoundingStep, поле sum_rounding_step. Применение округления к суммам не реализовано по границе текущего этапа. [по коду]
- symbols.json:111существующих записей Spec.cpp/TestSpec.cpp адресно сверены со свежим isolated clangd documentSymbol;950внешних записей и порядок сохранены. Header symbols не добавлялись. Shared MCP устарел (namespace end3150 против3170), его данные не публиковались. Callgraph не пересобирался и не объявляется актуальным.
- Родитель подтвердил source hashes, version1 didOpen/publishDiagnostics и364диапазона символов; ошибок нет, только unused-includes. Полные Spec.cpp/TestSpec.cpp скомпилированы MSVC SDK25 C++14 /W3 /WX в scratch obj exit0. Это не canonical addon build/link.
- Родитель повторил SDK25 standalone: семь parser suites, RED815/123FAIL → GREEN815/0. Настоящие GS::UniString/Array и project parser/числовые функции. Пропущен AddRule/native-dictionary блок ParseError (4 assertions); alias/formula/native locale boundaries fail-closed. Native/runtime и другие SDK/macOS не проверены, активный VS Run не останавливался.
- Evidence: Hermes scratch/spec318-parser/parent-acceptance.json, regression/, validation/. Код после принятых проверок не менялся. Коммит и push не выполнялись.

## Округление итоговых сумм Spec (#318, 2026-11-06)

- Реализовано применение шага: `Spec::RoundFinalSums` (SpecPlanning.cpp:278, hpp:163) — итоговые числовые суммарные слоты округляются вверх к +∞ после `PlanRuleRows`, до сверки; защита точного кратного через `is_equal`, целые слоты с нецелым результатом не меняются, doubleValue/rawDoubleValue согласованы. [по коду]
- Регрессия: RED 548/7 → GREEN 548/0 (извлечённый SDK25/C++14 стенд, реальные GS-контейнеры и project helpers; новый TestSpecRounding 35 проверок; смежные suites 513/0). Это не native и не полный addon build. [по коду]
- Полные TU SDK25 MSVC (SpecPlanning.cpp, Spec.cpp, TestSpec.cpp) скомпилированы exit 0; fresh isolated clangd — 0 ошибок, только прежние unused-include; format и diff --check чист. [по коду]
- Карточки Spec/SpecPlanning обновлены адресно, symbols.json +1 запись (RoundFinalSums, 1066), callgraph не пересобирался (оговорено в карточке). Evidence: Hermes scratch/spec318-rounding/. Коммит и push не выполнялись.

## Удаление документации модуля table (2026-11-06)

- Удалены четыре устаревших документа, текущие ссылки обзоров/wiki и table-срез JSON: 87 символов и 10 рёбер. Остальные JSON-записи и их порядок сохранены относительно рабочего baseline; полный сбор не выполнялся. Исторические снимки оставлены. Чекпойнт: [table-remove], без issue по указанию владельца.

## Команды этажей #315 — изолированный снимок (2026-11-06)

- Обновлены карточки CommonFunction/Propertycache/Helpers/Sync и русская страница команд: API-index отделён от UI-index, высота верхнего этажа берётся через виртуальный уровень SDK25. Задокументирован числовой index. [по коду]
- После параллельного коммита873c58f со Sync.cpp и частью Story подготовлена только оставшаяся дельта #315; refactor и RoomBook не включаются в неё. Четыре полных TU SDK25/MSVC C++14 /WX повторно exit0 на финальном снимке; controlled-регрессия64/0 повторена parent. Это не canonical addon build и не native-приёмка.
- symbols.json: свежие настоящие clangd DocumentSymbol isolated snapshot,236cpp-записей CommonFunction/Helpers/Propertycache проверены по исходникам; восстановлена полнота существующих overload-групп, добавлен ReadStory. Остальные947записей и порядок сохранены из873c58f, не из этой коллекции; их координаты не объявляются актуальными. Заголовки проверены clangd, но в cpp-only artifact не добавлялись. Полный parent-аудит специальных header-имён не завершён; опубликованные cpp-записи проверены отдельно. Callgraph и обзорные документы вне адресного обновления.
- Артефакты: Hermes scratch/story-315-only-commit/validation/ (final-results.json и snapshot-final). Рабочие исходники не подменялись; Sync-срез сохранён из параллельного коммита, RoomBook WIP не включён.

## R1 — тип matcher линейной отделки (#316, 2026-11-06)

- Адресная карточка Roombook описывает выбор matcher по типу избранного и ошибку раннего отказа Object_Draw_Linear. Две записи symbols.json обновлены по изолированному свежему documentSymbol; MCP-снимок отвергнут как stale (namespace end2748 против текущего2892). Остальные записи и унаследованный callgraph не обновлялись и не объявляются актуальными.
- Fresh publishDiagnostics version1 на baseline/live имеет одинаковые шесть диагностик (пять unused-variable и один unused-include); SHA256 live совпадает со снимком, все48selection/name joins проверены. Формат/scoped diff-check и canonical AC25 Windows RelWithDebInfo INT build пройдены, Draw перекомпилирован и APX перелинкован. Parent повторил standalone75/19FAIL →75/0: реальные matcher/selection и точный Object-префикс, controlled API/domain fixtures; это не полное выполнение writer/native. Independent review: No issues found in inspected scope. Debug APX/оба PLN по SHA256 неизменны; native-приёмка отдельно. Итог — в IDEA.md #316 и Hermes scratch/roombook-r1-20261106/.

## Декомпозиция распознавания SyncString (2026-11-06)

- В `Sync.cpp` вынесены локальные распознаватели ID, GDL, Material, Formula, Morph, Coord, Property, Info, IFC, Glob, Class, Element, MEP и ListData. Проверки применимости и общий разбор остаются в `SyncString`; `SyncStringFile`, `SyncStringAttribute` и `SyncStringStory` сохранены как самостоятельные ветви coordinator. [по коду]
- Карточка Sync отражает текущий порядок вызовов. В `symbols.json` адресно обновлён Sync-срез: 50 записей с обеими перегрузками `SyncElement`; остальные 1098 записей и порядок сохранены. Callgraph не обновлялся: исторические Sync-рёбра не объявляются актуальными.
- `clang-format --dry-run -Werror` и `git diff --check` для файлов задачи пройдены. Canonical AC25 Windows RelWithDebInfo INT build успешна: `Sync.cpp` перекомпилирован и APX перелинкован. Native-проверка не выполнялась. Диагностики clangd MCP не использованы как приёмка: сервер вернул нулевой набор, противоречащий прежнему baseline.
- Артефакты: `C:/Users/da-rogojin/AppData/Local/hermes/cache/scratch/syncstring-extract-20261106/`. Независимые structural review и standalone-регрессия относятся к сохранённому applied-снимку до последующей перестановки coordinator; текущая сборка подтверждает компиляцию текущего порядка, но не эквивалентность его прежнему порядку. Без issue по прямому указанию владельца.

## Полный подготовленный JSON-набор RoomBook (#314, 2026-11-06)

- Адресные контракты Roombook/json_commands обновлены по рабочей дельте к04e4dd9: все подготовленные records, отдельный sentinel marker, validity/eligibility/local-error, null GUID и finite/unavailable количества. Полный native-инвентарь и раскрытие состава не добавлялись.
- symbols.json: адресно сверены четыре helper/orchestrator записи хвоста Draw и четыре сдвинутые Favorites-записи JSON. Использован независимый fresh documentSymbol/version1 после доказанной stale MCP-сессии. Восемь source/name joins приняты; остальные1122записи и порядок сохранены. Прочие исторические координаты Draw и прежний callgraph не пересобирались и не считаются актуальными; полная регенерация вне scope.
- Parent сверил source/text hashes и563symbol selection ranges трёх изменённых файлов, повторил SDK25/C++14 GREEN1287/0 и baseline RED277/99FAIL. Canonical AC25 Windows RelWithDebInfo build успешна. Native загрузка/приёмка новой версии не выполнялись; открытый host и Debug APX/PLN сохранены. Артефакты: Hermes scratch/roombook-all-results-20261106/. Коммит не запрошен.

## Адресный вход RoomBook zones (#309, 2026-11-05)

- Карточки Roombook/json_commands описывают необязательный zones:string[], отказ всего запроса при ошибке, дедупликацию и const-copy входа. Исходный HEAD f2d94a2; описана только рабочая дельта #309, прочие исторические координаты не пересобраны.
- symbols.json: обновлены существовавшие записи RoomBookCommand.cpp и добавлен RoomBook_IsGuidString; в Roombook.cpp раздельно отражены wrapper и const-перегрузка. Использованы raw documentSymbol независимого свежего LSP (MCP Roombook вернул старый AST), все добавленные/изменённые координаты проверены по source. Неактивная AC22/23-заглушка сохранена и сверена по диску. Остальные записи/порядок сохранены; отсутствующие в старом JSON-срезе sibling-команды не добавлялись.
- Артефакты и hashes: Hermes scratch/roombook-zones-20261105/review-validation/ и generated-acceptance.json. Полные TU SDK25/C++14 в scratch obj успешны; canonical addon build/native не выполнялись при активной VS Run.
- SDK25 standalone JSON105/0 и защита входа Run34/0 повторены родителем; schema/source invariants21/0 отдельно. Native JSON-schema enforcement (включая zones:null, теряемый SDK25 ObjectState conversion) не выполнялся. Исходники после принятых compile/LSP/tests не менялись.
- callgraph.json не обновлён: унаследованные RoomBook_Run-рёбра содержат старые координаты и неоднозначное сопоставление одноимённых перегрузок/неактивной заглушки. Не выдавать его за актуальную карту вызовов; для нового входа источники — live source и обновлённые карточки. Полная регенерация графа — отдельная документационная задача.

## Адресная сверка Spec (2026-11-04)

- Обновлены текущие контракты Spec/SpecPlanning по рабочей дельте к `13dfb0c`:
  позиции неполных сумм, счётчики строк/удалений, отмена, диагностика флага,
  GUID-связи. Исторические разделы и чужие модули не пересобирались.
  Координаты новых пунктов сверены с live source; hashes и исходные MCP-ответы
  сохранены в scratch/spec-review-fixes-20261104/docs.
- MCP documentSymbols TestSpec.cpp оказался stale после force_refresh.
  Для адресной проверки использован независимый isolated clangd LSP из
  `fresh-lsp/publish-manifest.json`: hashes совпали с live, все 127 диапазонов
  проверены. Это не свежий MCP-сбор TestSpec; Spec.cpp/SpecPlanning.cpp также
  сверены с независимым источником.
- Адресный symbols-срез: 119 записей трёх файлов; внешние записи и их порядок
  сохраняются. Callgraph не обновлён: адресная проверка унаследованных
  call_sites не завершена, старые рёбра сохранены и не объявляются актуальными.
- Native runtime и другие SDK этой дельтой не проверялись. Issue307 открыт;
  коммит документации не выполнялся.

## Текущее состояние (2026-10-30)

- Базовый коммит кода: `b199125` `[C3-WIP]` (снимок roombook-рефакторинга, ветвь
  `roombook_refactor`, 10 коммитов вперёд от master). Правки C++ в этой задаче не выполнялись.
- `Docs/` переписан по этому снимку: Roombook.md с нуля, сопутствующие карточки и обзорные
  файлы актуализированы, карточка удалённого `dialogs/CommandHelpers` удалена.
- **Docs-коммит не выполнен** — ожидает решения владельца: AGENTS §17 ведёт Docs в `master`,
  а текущая ветвь `roombook_refactor`. Варианты: закоммитить Docs отдельным коммитом в
  текущей ветви (сольются при мерже) или отложить до мержа roombook-рефакторинга.
- `compile_commands.json` пересобран (`BuildAddOn.py --lsp` AC25): 54 записи, свежая база
  clangd. В git не отслеживается (`.gitignore`).

## Что сделано в этой итерации

- `Docs/_generated/symbols.json` пересобран полностью: 1128 записей, 47 файлов, строки
  1-based, привязаны к диску после `b199125`. Phantom-записи отброшены, roombook-файлы
  пересобраны clangd MCP с дисковой сверкой (0 промахов на выборке 80 символов).
- `Docs/_generated/callgraph.json`: roombook-срез пересобран grep-методом по диску — 807
  рёбер из 1329; входные рёбра `MenuCommandHandler`/`RoomBookCommand::Execute`/
  `RoomBookGetFavoritesCommand::Execute` добавлены явно. Не-spec срезы — прежнего прогона,
  140 известных устаревших рёбер (`meta.known_stale_non_spec`).
- Карта создания/изменения перенесена из scratch-артефакта (6 Create / 6 Change / 3 Delete /
  2 Classification с дисковыми строками) в `Docs/modules/Roombook.md`; 13 писателей
  (Wall/Slab/Beam/Object/Roof/OtdRoom/OtdRooms/OtdOpening/OtdElements_SetSync/Class_Set/
  Class_Set_OtdOpening/LayerCombination_ApplyFinishIfExists/Roof_WritePrepared) с guard'ами
  needupdate (`RoombookDraw.cpp:890–920`).
- Карточки: `modules/Roombook.md` (с нуля), `Helpers.md` (+11 строк после вставки в
  Helpers.hpp), `json_commands.md` (FavoritesList/Favorite_CreateOtd/Roombook_CheckFinish-
  ConfigCommand, регистрация HealthCommand), `Propertycache.md`
  (`ParsePropertyDescriptionToRules` -> Propertycache.cpp:1344), `DISCREPANCIES.md`
  (roombook-координата RoombookData.cpp:7104), `ARCHITECTURE.md`, `REPOMAP.md`,
  `SEQUENCES.md` (диаграмма 11 переписана под конвейер `RoomBook_Run`).
- Удалена `Docs/modules/dialogs/CommandHelpers.md` (модуль удалён в `485072d`).

## Известные пробелы (честные пометки)

- `MEPv1.cpp`: clangd отдаёт 0 символов при 3 функциях в файле — в symbols.json отсутствует
  (нужен regex-сбор или починка clangd-индекса).
- Regression-файлы тестов (`RoomAnalyticRegression.cpp`, `SkirtingRegression.cpp`,
  `CoordinateSearchCacheRegression.cpp`) пусты в symbols.json — код под `#ifdef`, в TU не
  входит; это ожидаемо, не дефект.
- Не-spec срезы callgraph (Spec/Sync/Summ и пр.) не пересобирались в этой итерации: 140
  устаревших рёбер унаследованы от прогона 2026-09-22 — отдельная задача.
- Координаты SDK-рёбер в callgraph не развёрнуты (только счётчики outgoing_count).
- Полнота cross-TU графа не заявляется: собраны project-рёбра, внешние вызовы
  (Helpers.cpp/CommonFunction.cpp) — частично.

## Процедура обновления (проверено)

1. Свежий `compile_commands.json`: `python Tools/BuildAddOn.py -c config.json -v 25 --lsp`
   (в git не отслеживается, не коммитить).
2. `symbols.json`: clangd MCP `documentSymbol` по каждому .cpp, строки переводить
   0-based -> 1-based; после коммитов кода сверять координаты с диском (имя присутствует
   на строке), протухшие исправлять поиском в окне +/-N строк. Phantom-записи (имя
   отсутствует в файле) отбрасывать, не «чинить».
3. `callgraph.json`: `generate_symbols.py` callHierarchy не собирает (пишет `[]`) и не
   работает через subprocess.PIPE на Windows; собирать через clangd MCP
   `get_call_hierarchy` на позиции имени в определении либо grep-методом по диску.
   SDK-рёбра фильтровать по пути `Sources/AddOn/`.
4. После правки кода: обновить карточку модуля и `_generated/` в том же коммите
   (AGENTS §17); расхождения «комментарий != код» — в `DISCREPANCIES.md`.
5. Скрипт слияния батчей symbols хранить вне репозитория (случайный `scratch/` в корне
   репо уже создавался и был удалён — не повторять).

## Статус фаз

- [x] Фаза 1–2: compile_commands, symbols, callgraph (база)
- [x] Фаза 3: все модули имеют карточки `Docs/modules/<module>.md` (после удаления
      CommandHelpers и table — 25 файлов)
- [x] Фаза 4: `ARCHITECTURE.md`, `REPOMAP.md`, `SEQUENCES.md`
- [x] Фаза 5: сверка -> `DISCREPANCIES.md`
- [x] Roombook-срез: полная регенерация по снимку `b199125`
- [ ] Docs-коммит (решение владельца: ветвь vs master)
- [ ] Не-spec срезы callgraph (отдельная задача)
