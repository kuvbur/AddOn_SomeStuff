# Current Task

## Task

Roombook — безопасный рефакторинг без изменения поведения, #260: https://github.com/kuvbur/AddOn_SomeStuff/issues/260. Геометрия по граням зоны — отдельная функциональная задача #252, её диагностический Z0 необходим до фиксации новых интерфейсов.

## Scope

AC25 Windows; сначала состояние дерева, SDK и реальный baseline результата/скорости, затем малые локальные изменения Roombook и характеризационные тесты. Сохранять пакетное чтение, порядок вызовов, пересоздание отделки и результат. Не менять общий Helpers.cpp, Sync или новую геометрию #252 в чистом рефакторинге. Остальные версии не собирать без явного запроса.

## Status

IN_PROGRESS — текущее состояние P2b4/P4j–P2b9 фиксируется по запросу владельца checkpoint [P2b9]. Новый TestCachedParameterReader :2362,1009/0; полный TestKit83/26783/0 EXIT0, прежние82END совпали. Production побайтно неизменен после P4m; AC25 BuildAddOn success, fresh Core/registry0errors. Scoped docs/generated актуализированы. Дальнейший рефакторинг в этом ходе не выполнялся; чужой Test_file/test_30.pln исключён, push не выполняется.
P3a–P3e приняты ранее; детали в плане и `Docs/_progress.md`.
Полные контуры/материалы и причины `paramTo.isValid` остаются not verified. Вне scope
`.github/workflows/build_25+.yml` — не трогать.

## Last Completed

P2b9: cached reader — шесть типов × шесть имён × четыре bool/fromProperty состояния, перестановки fallback, aggregate/preset, все поля значения/формата, сохранность источника/шаблона и два GUID в обоих порядках.1009/0, TestKit83/26783/0 EXIT0,82oldEND равны P4m. Production byte-identical, реальное SDK-чтение и новый model read-back не выполнялись.22fresh test symbols/1source-joined edge5sites,478inherited unchanged,graph479. Накопленные P2b4/P4j–P2b9 включаются в checkpoint [P2b9] по запросу владельца; предыдущий7140f15 [P2b3].

P1 частично: четыре независимых прогона исходного AC25 Debug на чистой копии `test_25.pln` — `elapsedSeconds` 64.1296099 / 64.2839503 / 62.4861408 / 62.379905; все 63 зоны и +1301 Wall/+126 Object/+351 Window в трёх полностью снятых pre/post. Прогоны 3 и 4: ограничивающие 3D-параллелепипеды и 25 свойств для всех 1778 новых элементов, мультимножества `{тип, boundingBox3D, доступные свойства}` совпали (0 разниц). Артефакты в `Reviews/roombook_baseline/`. P2a: характеризационный `TestBuildOtdByParent` расширен (два родителя, два ребёнка-стены, неизвестный ребёнок, пустые входы, флаг на входе); исходная фикстура ошибочно ожидала мутации значения после `HashTable::Add`, исправлена. Форматирование, реестр 69/69/69, clangd 0 диагностик, AC25 `BuildAddOn.py` success, свежий TestKit: `TestBuildOtdByParent passed=12 failed=0`, всего `suites=69 passed=2567 failed=0`. Производственный код не менялся. Полная эквивалентность геометрии/материалов — not verified.

## Next Step

Продолжение P2b2 — после отдельного запроса: сначала инвентаризация оставшихся характеризационных сценариев по исходному плану и проверка уже имеющихся assertions, без дробления коротких функций. Непустой непредустановленный reader-запрос с кандидатами при отсутствующем GUID не исполнялся (baseparam без null-guard); не менять production в тестовой задаче. Новые геометрические интерфейсы по-прежнему за Z0/#252; полноценный P1 остаётся неполон. До продолжения перепроверить дерево/VS; push и закрытие #260 не разрешены.

## Last Checkpoint

Чекпоинт этой фиксации — [P2b9] Зафиксировать рефакторинг Roombook и характеризационные тесты (Refs: IDEA.md step P2b9; #260). Его SHA — git log -1; предыдущий checkpoint7140f15 [P2b3]. Накопленные P2b4/P4j–P2b9 включены; push не выполнялся.

## Plan

- [x] P0. Зафиксировать старт, SDK и достоверность источников (конкретная геометрия SDK — отдельный Z0, not verified).
- [x] P1 (частичный, по решению владельца только в этом ходе). Сохранены native PLN и JSON-снимки/времена; контуры/материалы не проверены.
- [x] P2a. Расширить и исполнить характеризационный `TestBuildOtdByParent` на AC25 (12/0, полный TestKit 69/2567/0).
- [x] P2b1. TestOpeningAddOne: 18 сценариев, 114/0 на исходном production, TestKit 70/2681/0 (00:22:54), реестр 70/70/70.
- [x] P4a. CalculateOpeningForWall извлечён из Opening_Add_One; прежние OtdOpening/арифметика/условия. Тест 114/0 до/после, TestKit 70/2681/0, AC25 build success, VS read-back 1778 missing/extra=0, GUID сохранены; LSP без новых диагностик.
- [x] P2b2a. TestOtdWallDelimOne: 15 сценариев, 136/0 на прежнем production; TestKit 71/2817/0 (00:41:52), реестр 71/71/71.
- [x] P4b. TrimOpeningsToWallHeight: цикл и промежуточные zDup/zDown сохранены, публичный const-вход восстановлен после ошибочного fuzzy-патча; AC25 build success, tests 136/0 до/после, TestKit 71/2817/0, VS 1778 missing/extra=0, LSP без новых диагностик.
- [x] P2b2b / #261. Два порядка: RED 149/1 → GREEN 158/0, TestKit 71/2839/0. Отдельный разрешённый bugfix субагента принят после проверки; прежние tests сохранены.
- [x] P2b2c. OtdWall_Delim_All: 8 комбинаций типов + fallback, 149/0; TestKit 72/2988/0 (01:16:29), AC25 build success, реестр 72/72/72.
- [x] P4c. GetWallFinishBandType: порядок проверок сохранён; tests 149/0 до/после, TestKit 72/2988/0, AC25 build success, VS 1778 missing/extra=0, fresh LSP без новых диагностик.
- [x] P2b2d. Fallback после отказа полос, частичное пересечение и два проёма через несколько полос в обоих порядках: TestOtdWallDelimAll 197/0, TestKit 72/3036/0, EXIT 0; AC25 BuildAddOn success, свежий TestCore MCP 0 errors/3 прежних unused-includes. Production не менялся в тестовой подзадаче.
- [x] P4d. GetFallbackWallFinishType: переданный текущий type и четыре if сохранены; весь production эквивалентен pre-step снимку после обратной подстановки. Tests 197/0 до/после, TestKit 72/3036/0; AC25 BuildAddOn success, fresh LSP без новых диагностик, VS 1778 missing/extra=0.
- [x] P2b2e. OtdWall_Add_One 60/0; вертикальное пересечение стены OtdWall_Delim_One 246/0; TestKit 73/3184/0, EXIT 0, AC25 BuildAddOn success, fresh TestCore MCP 0 errors/3 прежних warnings. Production read-only в тестовой подзадаче.
- [x] P4e. CalculateWallHeightIntersection (:3832): guards/арифметика сохранены, весь production совпадает с pre-step снимком после inlining и восстановления локальных объявлений. TestKit 73/3184/0 до/после, AC25 build success, fresh LSP без новых диагностик, VS read-back 1778 missing/extra=0. OtdWall_Add_One не дробился.
- [x] P2b2f. SetMaterialByType: 11 типов × 6 вариантов настроек × structural/finish слой; 1848/0, TestKit 74/5032/0, EXIT 0, AC25 build success. Production не менялся до baseline; fresh TestCore/registry проверены.
- [x] P4f. SelectWallFinishMaterial (:3887): только type/material/settings, без дополнительных копий; полный pre-step inlining совпадает. TestSetMaterialByType 1848/0 до/после, TestKit 74/5032/0; AC25 build success, fresh LSP без новых диагностик, VS read-back 1778 missing/extra=0, scoped symbols132/132 и6edges сверены.
- [x] P2b2g. TestOpeningRevealsCreateOne: 13 сценариев × два начальных has_reveal; 662/0, TestKit 75/5694/0, EXIT 0 на неизменённом production; AC25 BuildAddOn success, fresh TestCore 0 errors/3 прежних warnings, реестр 75. Проверены координаты, порядок, подрезка, материалы и входные поля.
- [x] P4g. InitializeOpeningRevealWall: прежние 13 присваиваний/три site, полный pre-step inlining совпадает; AC25 success, tests662/0 и TestKit75/5694/0 до/после, LSP без новых диагностик, VS1778 missing/extra0, symbols134/134/7 helper edges сверены. Включено в текущий checkpoint [P4g]; push не выполнялся.
- [x] P2b2h. Откосы × полосы: 10 сценариев × два режима материала × два исходных has_reveal; набор 2414/0, TestKit 75/7446/0, EXIT 0 на неизменённом production. AC25 BuildAddOn success; fresh TestCore 0 errors/3 прежних warnings. Проверены порядок/материалы, частичная верхняя балка, fallback, отсутствие выходов при установленном флаге и сохранность входов.
- [x] P4h. CalculateOpeningRevealHeight (:3514): прежние верхнее ограничение/zDown/height/min_dim; полный pre-step inlining совпадает после восстановления локальных объявлений. AC25 BuildAddOn success; откосы2414/0 и TestKit75/7446/0 до/после, fresh LSP без новых диагностик, VS1778 missing/extra0, symbols128/128 и8helper boundaries сверены. Коммит/push не выполнялись.
- [x] P2b2i. 11 новых сценариев × два has_reveal: смещённые отметки и границы min_dim. Production совпадает с проверенным pre-step снимком; AC25 BuildAddOn success, fresh TestCore0 errors/3 прежних warnings, откосы3052/0, TestKit75/8084/0, EXIT0 через VS MCP.
- [x] P3j. BuildMaterialSummaryForRooms (:377): OtdRooms/ParamDictElement напрямую вместо RoomProcessingContext, сняты два алиаса; полный pre-step reverse совпадает. AC25 BuildAddOn success; TestKit75/8084/0 до/после, fresh LSP без новых диагностик, fresh VS before/after1778 missing/extra0, GUID сохранены. Scoped symbols128/128;9helper boundaries/12fresh edges joined, project_edges430. Коммит/push не выполнялись.
- [x] P3k. Инвентаризация5/6полей; контекст в стадиях сохранён. Последние6context-алиасов в ProcessRoomFinishes/RoomBook сняты; production равен6подстановкам modulo formatting. AC25 success, fresh LSP прежний, TestKit75/8084/0 и все75итогов совпали, VS1778/GUID без различий. Scoped symbols128/128,11границ/32fresh edges+Menu source-site, graph450;417прочих inherited edges сохранены.
- [x] P2b2j. Девять сценариев ×2has_reveal: диагонали/objLoc/отрицательная ширина/масштаб перпендикуляра; на неизменённом production откосы3754/0, TestKit76/8802/0, EXIT0 через VS MCP.
- [x] P4i. CalculateOpeningRevealCoordinates :3523; арифметика/endpoint сохранены, три стадии/флаг до clipping прежние. AC25 success, fresh LSP прежний, все76итогов совпали, VS1778 без различий, scoped symbols129/129/graph453. Checkpoint8e242ef.
- [x] P2b3. ClearZoneGUID: характеризация вложенного индекса, dedup/NULL GUID/непросматриваемый тип/повторный вызов на неизменённом production AC25. Короткую функцию не дробить. Проверено:1719/0; AC25 build success, fresh MCP, TestKit77/10521/0 EXIT0.
- [x] P2b4. Накопление материалов OtdData_CalcForRoom: площади/состав/фильтры/типы отделки и повторное суммирование, без форматирования и SDK-записи; baseline на неизменённом production AC25. Baseline8031/0, TestKit78/18552/0 EXIT0, VS1778 без различий.
- [x] P4j. CollectMaterialAreasForRoom :985: два прежних обхода стен/перекрытий; reverse==pre-step, AC25 success, fresh LSP прежний,8031/0 и TestKit78/18552/0 до/после, VS1778 без различий; scoped docs/generated проверены. Включено в текущий checkpoint [P2b9].
- [x] P2b5. TestRoomParameterIsolation:1921/0, TestKit79/20473/0 EXIT0; production byte-identical, AC25 BuildAddOn success, fresh Core/registry0errors, прежние78END совпали.
- [x] P4k. NormalizeRoomFinishHeights :2702, Param_SetToRooms :2722/site3016; полный reverse==pre-step, порядок/валидность до rounding/арифметика прежние. AC25 success, fresh LSP без новых severity/code/message,1921/0 и TestKit79/20473/0 до/после, VS1778 без различий против baseline; scoped docs/generated проверены. Включено в текущий checkpoint [P2b9].
- [x] P2b6. TestRoomMaterialFormatting :1737:973/0, TestKit80/21446/0 EXIT0, прежние79END совпали; production byte-identical, AC25 BuildAddOn success, fresh Core/registry0errors. SDK metrics исполнены, property-write в модель не выполняется. Scoped docs/generated проверены.
- [x] P4l. CollectAreasForFinishTypes :1082: прежний copy/циклы/#if/суммирование; полный reverse==pre-step, AC25 success, fresh Root без новых диагностик. Tests973/0, TestKit80/21446/0 EXIT0, все80итогов совпали; VS1778 без различий/GUID сохранены. Scoped docs/generated проверены, включено в текущий checkpoint [P2b9].
- [x] P2b7. TestOpeningParameterIsolation :1908:3692/0, TestKit81/25138/0 EXIT0, все80унаследованных END совпали.64маски/слои,13состояний ×оба порядка,5early-return на cached-данных; production byte-identical, AC25 success, fresh Core/registry0errors, scoped docs/generated проверены. Реальное GDL/private pipeline не проверены.
- [x] P2b8. Характеризация Param_Property_FindInParams/windowParams на временном cache с RAII-восстановлением; production read-only AC25. Baseline 636/0,82/25774/0 EXIT0,81 old END equal,production bytes unchanged.
- [x] P4m. ResolveParameterRawNames :2596, caller Param_Property_FindInParams :2640/site2646: только прежний цикл, полный reverse==pre-step. AC25 success, fresh LSP без новых диагностик;636/0 и TestKit82/25774/0 до/после, все82итогов равны, VS1778 без различий/GUID сохранены; scoped docs/generated проверены. Включено в текущий checkpoint [P2b9].
- [x] P2b9. TestCachedParameterReader :2362:1009/0; TestKit83/26783/0 EXIT0,82inherited END равны P4m, registry83/83/83. Production byte-identical; AC25 success, fresh Core/registry0errors, scoped22symbols/edge5sites joined,graph479. Текущее состояние P2b4/P4j–P2b9 фиксируется checkpoint по запросу владельца, без push.
- [ ] P2b2. Остальные характеризационные сценарии; полноценный P1 остаётся непокрыт.
- [x] P3a. Локализовать `paramDict` в `ReadElementParameters` без изменения порядка/числа вызовов; AC25, TestKit и частичный read-back проверены, худшее время 65.5245175 с против 65.7253702 с исходного (−0.31%), критерий владельца выполнен.
- [x] P3b. `reducededges`: обнуление после синхронного SDK-вызова; контракт подтверждён, худшее 65.7452587 с против 65.7253702 с (+0,03%).
- [x] P3c. Один владелец прогресса/отмены вместо девяти копипаст-блоков; худшее 64.5880836 с против 65.7253702 с (−1,73%).
- [x] P3d. Снять пять `auto &`-алиасов на поля контекста в теле `RoomBook`; порядок операций прежний, худшее 64.0615044 с против 65.7253702 с исходного (−2,53%), критерий владельца выполнен.
- [x] P3e. Убрать десять алиасов в ApplyFavoriteAndMaterialData, WriteRoomMaterialData и RemoveUnusedFinishingElements; AC25 собран, TestKit 69/2567/0, частичный read-back 1778 элементов без различий. Продолжение серии A/B отменено владельцем: мелкие изменения не требуют замеров производительности.
- [x] P3f. ApplyFavoriteAndMaterialData принимает ParamDictValue/ParamValue вместо RoomProcessingContext; AC25 BuildAddOn success, VS read-back 1778 без различий, GUID сохранены. Clangd stale/timeout, LSP not verified.
- [x] P3g. WriteRoomMaterialData принимает OtdRooms/ParamDictElement вместо RoomProcessingContext; MaterialSummary, порядок и ссылки сохранены. AC25 BuildAddOn success; VS read-back 1778, missing/extra=0, исходные GUID сохранены; свежий TestKit 69/2567/0. LSP/generated not verified (stale clangd).
- [x] P3h. RemoveUnusedFinishingElements принимает конкретные ссылки вместо контекста, прежние итераторы/порядок. AC25 BuildAddOn success, VS read-back 1778 missing/extra=0, исходные GUID сохранены, свежий TestKit 69/2567/0. LSP/generated пропущены с разрешения владельца только в этом ходе.
- [x] P3i. ProcessSlabFinishes/ProcessWallFinishes: конкретные зависимости вместо RoomProcessingContext, прежние копии на проём и порядок. Fresh MCP: диагностик сверх baseline нет, symbols Roombook 112/112. AC25 build success, VS read-back 1778 missing/extra=0, GUID сохранены, TestKit 69/2567/0.
- [ ] P4–P7. Геометрические извлечения только после соответствующих тестов и подтверждения границ Z0; завершение по воротам исходного плана.
- [ ] Z0 (#252). До новых геометрических интерфейсов проверить тело зоны/сегменты на модели; функциональную реализацию вести отдельно.

## Decisions

- #261: https://github.com/kuvbur/AddOn_SomeStuff/issues/261 — предсуществующий перенос zDup между проёмами. Предсуществование сверено по HEAD, runtime-дефект подтверждён на текущем неизменённом production. Bugfix разрешён владельцем и выполнен субагентом, принят родителем по артефактам. **Issue CLOSED 2026-10-04 по решению владельца** (`--reason completed`), комментарий issuecomment-5978504082: `wallTop` вместо перезаписанного `zDup` в отсеве следующего проёма, diff три строки. Проверка фикса по коду в текущем ходе: `Roombook.cpp:3811/:3815`, `git diff 1acdf57 2ea3f87`; `zBottom` в цикле не переприсваивается, `zDup`/`zDown` считаются заново на каждой итерации — остаточной порядко-зависимости нет. Модельное проявление на реальном проекте — **not verified**. Входной параметр `zDown` функции не читается (перезаписывается первой строкой цикла) — сигнатура оставлена прежней намеренно, чтобы diff оставался минимальным bugfix'ем.
- #195 охватывает только извлечение GetTargetZones и уже реализовано; umbrella рефакторинга — #260, изменение геометрии — #252, производительность — #198, сверка на PLN — #164.
- Оригинал `Test_file/test_25.pln` не менять. После RoomBook результат существует в открытой модели, но не сохранён на диск автоматически: снимать данные до перезапуска, потом явно открывать исходное состояние для следующего прогона. Успех JSON-команды `returned` не доказывает конечный результат.
- Решение владельца 2026-10-03: A/B производительности выполнять только для крупных изменений; не прогонять серию после каждой мелочи. Для мелких правок сохраняются сборка, LSP и тесты корректности. Запуски runtime — через Visual Studio MCP.
- Критерий приёмки скорости для крупных изменений (решение владельца 2026-10-03): колебания до 5% допустимы, эталон — худшее значение серии. Он фиксирует приемлемость, а не равенство скоростей; при отладке не замерять под breakpoint.

---

## Закрыто — AC28/AC29: сборка падала на обращении к паре итератора (#257)

**Scope:** `Sources/AddOn/ReNum.cpp` (блок обхода `missing_props` в
`GetRenumElements`). Версии AC28, AC29 — по явному запросу владельца.

**Status:** DONE (обе версии собраны, issue #257 CLOSED 2026-10-03)

**Refs:** issue #257

**Причина:** в ветке `#ifdef ServerMainVers_2800` стояло `cIt.key` / `cIt.value`.
Смена в AC28 затронула **тип полей** `CurrentPair` (указатели → ссылки), а не
способ обращения к паре: `cIt->key` верен в AC22–AC29, `cIt.key` не компилируется
ни в одной версии. Соседние обходы в проекте (`Dimensions.cpp:68`,
`Helpers.cpp:5987`, `Roombook.cpp:1088`) используют `->` — расхождение было
только в новом коде #256.

**Проверено:** `BuildAddOn.py -v 28` → `AI_BUILD_RESULT status=success`;
`-v 29` → `status=success`; clangd по `ReNum.cpp` — 0 диагностик; развёрнутый
аудит всех `#ifdef ServerMainVers_2800` в `Sources/AddOn/` на `cIt.key`/
`cIt.value` — других вхождений нет. `.apx` собраны в `Build/SomeStuff/{28,29}/Debug/`.

**Замечания по граблям:**
- `Build/DevKit/APIDevKit-28/Support/Modules/GSRoot/HashTable.hpp:117` — источник
  истины по `CurrentPair`; `ForwardContainerIterator.hpp:65-66,110-111` — по
  `operator*` / `operator->`;
- при `#ifdef ServerMainVers_2800` в многострочном `for` нельзя судить о форме
  обращения по одной строке заголовка — заголовок `for` часто разбит переносом;
- `gh --body-file` не принимает MSYS-путь (`/c/...`) — нужен нативный
  `C:/Users/...`.

## Открыто — сборка под AC30

**Scope:** `CommonFunction.hpp` (слой `UniStringToLower`), 96 вызовов в 15 файлах,
`api_headers/APICommon30.h/.c`, `dialogs/SyncSettings.cpp` (RapidJSON).

**Status:** DONE по компиляции — `db6f277`. AC30 Debug success, AC25 Debug
success. Runtime не проверен.

**Причина:** в AC30 переименован `GS::UniString::ToLowerCase` в `GetLowerCased`
(старое имя осталось как `ToLowerCaseDeprecated`), GRAPHISOFT переложил RapidJSON
в `Modules/RapidJSON/rapidjson/`, и в проекте не было заголовка
`api_headers/APICommon30.h`.

**Что сделано:**
- `UniStringToLower()` — свободная `inline`-функция в `CommonFunction.hpp` с
  веткой по `ServerMainVers_3000`; 96 вызовов `X.ToLowerCase ()` переведены на
  неё. `SetToLowerCase()` в AC30 сохранился, не заворачивался;
- RapidJSON подключается как `rapidjson/document.h` под AC30, как `document.h`
  раньше;
- `api_headers/APICommon30.h/.c` взяты из примера DevKit-30, подключение через
  `#ifdef AC_30`.

**Грабли по ходу:** скриптовая замена вызовов не должна переписывать файл целиком
с вырезанием `//`-комментариев и не должна терять окончания строк CRLF — оба
случая привели к откату через `git checkout HEAD --`. Парсер выражения-получателя
обязан понимать `->` и `Get (0)`, иначе склейка скобок ломает код.

**not verified:** runtime на живой модели, AC22–24/26–29, macOS.

## Прочее открытое

## Закрыто — ReNum: окно результата по каждому правилу (#256)

**Scope:** `ReNum.cpp/.hpp`, `Constants.hpp`, `DG4rule.cpp/.hpp`, `AddOn.grc.in`,
`Docs/modules/ReNum.md`. AC25.

**Status:** DONE — подтверждено пользователем в живом Archicad (2026-10-03):
заголовки колонок и сообщения отображаются верно.

**Refs:** #256 закрыт; коммиты `a65318e`, `de72d3d`, `ae0932d`.
Побочно заведено **#258** (тот же сдвиг ID в Spec) — открыто.

**Что сделано:**
- `RenumRunResult` — накопитель запуска; одно окно `RuleSelectDialog`
  (`isReadOnly`) вместо всплывающих окон на каждую точку отказа;
- пять колонок по правилу: Элементов / Изменено /Игнорировано / Пропущено /
  Ошибки; колонки отбора закрывают вопрос «куда делась разница»;
- отсутствующие свойства копятся по правилу-владельцу и больше не обнуляют
  весь запуск (прежде `return false` сбивал нумерацию по всем правилам);
- `n_error` отделён от `n_skip` (решение правила ≠ ошибка);
- короткие заголовки, разделители- двоеточия вместо `« - »`, окно 680 px.

**ГЛАВНАЯ НАХОДКА (#256):** номер строки ресурса — это её ПОЗИЦИЯ в блоке
`'STR#'`, а не число в комментарии `/* [ N ] */`. Строки 88/89 отсутствуют, всё
дальше сдвинуто на два. Из-за этого константы ReNum указывали не туда, и это
объясняет все три скриншота. Подробности и способ проверки — AGENTS.md §15.1.
Тот же сдвиг затрагивает `SpecCreatedId/ModifiedId/DeletedId` = 90/91/92
(ID 90 = «Удалено») — НЕ исправлено, вне задачи.

**Замечания по граблям (дописано по второму скриншоту):**
- `totalCount` для табов должен быть `NameTab + valueCount + 1`: прежняя
  формула обрезала заголовок последней колонки, данные при этом оставались;
- `CenterText` не переносит строки по словам (только `Truncation` в
  `DGStaticItem.hpp`), текст обрезается по высоте поля;
- `ValueTab_w` — ширина колонки; длинный заголовок обрезается молча, поэтому
  формулировки колонок короткие;
- в `.grc` номера строк пишутся с разным числом пробелов (`[ 74]` и `[ 94]`),
  grep по точному шаблону их не находит.

## Прочее открытое

## Открыто — Roombook: грани 3D-тела зоны (#252)

**Scope:** отдельное функциональное направление, не чистый рефакторинг: геометрия отделки из граней зоны, состав из сопоставленных стен/колонн. https://github.com/kuvbur/AddOn_SomeStuff/issues/252.

**Состояние:** требование зарегистрировано; `GetZone3DPolygons_StrictAPI` найдена в `13ba469^:Sources/AddOn/Roombook.cpp`. Дополнение внесено в `.hermes/plans/2026-10-03_103032-roombook-safe-refactor.md`; реализация не начата.

**Next Step:** проверить фактические грани зоны для сложного профиля и многосегментной колонны, SDK-контракты AC25, затем согласовать сопоставление и представление геометрии. Runtime — not verified.

**Last Checkpoint:** отсутствует; код не менялся, commit/push не выполнялись.

## Закрыто — #253 (кнопка «сайт автора» в палитре BrowserPalette)

https://github.com/kuvbur/AddOn_SomeStuff/issues/253

**Scope:** `Sources/AddOn/dialogs/BrowserPalette.cpp` (мост `OpenWebsite` +
хелпер `OpenWebsiteInDefaultBrowser`), `Sources/AddOnResources/RFIX/HTML/Interface_ru.html`
(`WEBSITE_URL`, `openWebsite`, `globeIconSvg`, кнопка в `buildShell()`),
`Sources/AddOnResources/RFIX/HTML/ТЗ интерфейс.md` (§4.1),
`Docs/modules/dialogs/BrowserPalette.md`, `config.json` (версия аддона 2.01).

**Состояние:** ЗАКРЫТО 2026-10-03, issue #253 CLOSED, коммит 9eb5e7e. Проверено
владельцем в живом Archicad AC25: кнопка открывает kuvbur.org, палитра не уходит
со страницы. Кнопка стоит в развёрнутом виде колонки вкладок — непосредственно над
переключателем темы.

**Проверено:** `Tools/test_html.ps1` — ALL HTML CHECKS PASSED (HTMLHint + verify.js);
LSP по `BrowserPalette.cpp` — единственная диагностика `withSpecRule`
предсуществующая (есть в HEAD:1732), моих регионов 0; сборка AC25 Debug —
`AI_BUILD_RESULT status=success`; `restart_archicad_for_test.ps1` — build + запуск
AC25, тесты `suites=68 passed=2512 failed=0`, `EXIT 0`, отчёт свежее `.apx`
(mtime отчёта 11:51:59 при `.apx` 11:48:19); HTML вшит в `.apx` — все четыре
маркера (`open-website`, `kuvbur.org`, `globeIconSvg`, `WEBSITE_URL`) найдены в бинарнике.

**`not verified`:** macOS-путь `GS::Process::Create("open", …)` — mac-машины нет.

**Next Step:** нет — задача закрыта.

**Last Checkpoint:** `9eb5e7e` — 5 файлов: `BrowserPalette.cpp`, `Interface_ru.html`,
`ТЗ интерфейс.md`, `Docs/modules/dialogs/BrowserPalette.md`, `config.json`.

## Закрыто — #254 (английский интерфейс палитры)

https://github.com/kuvbur/AddOn_SomeStuff/issues/254

**Scope:** `Sources/AddOnResources/RFIX/HTML/Interface_ru.html` (единый
редактируемый исходник + встроенный RU-каталог), новые
`Sources/AddOnResources/RFIX/HTML/i18n/{inventory.md,glossary.md,en.json,README.md}`,
генерируемый `Interface_ru`-партнёр `Interface_en.html` (не редактируется вручную),
`Tools/localize_html.js` + `Tools/tests/*.test.js`,
`Tools/test_html_locales.ps1`, `package.json` (i18n-скрипты),
`Tools/AddOn.grc.in` (только имя файла в `ID_ADDON_HTML_ENG`),
`Tools/restart_archicad_for_test.ps1` (только функция `Test-HtmlValidation`),
`ТЗ интерфейс.md`, `Docs/modules/dialogs/BrowserPalette.md`, этот файл.
Production C++ **не меняется** — читается для классификации сообщений.

**Версия:** AC25 Windows — целевая приёмка. AC22–24, AC26–29, macOS — вне объёма,
по отдельному запросу владельца.

**Статус:** ЗАКРЫТО 2026-10-03, issue #254 CLOSED. Сборка AC25 выполнена.

**Решение:** автономный EN-файл генерируется из RU-исходника, две ручные копии
логики не заводятся. Перевод инкрементальный: изменение русского `source` или
`context` помечает запись устаревшей и требует явного пересмотра, автоматического
«одобрения» перевода нет. Имена свойств/групп/классификаций/избранного, значения
DSL и пользовательские скрипты не переводятся.

**План:** `.hermes/plans/2026-10-03_115258-interface-english-localization.md`
(шаги 01–22, контрольные точки A–E).

**Last Completed:** шаги 01–20. Issue #254. Миграция выполнена: **159 ключей**,
все видимые строки переведены; `Interface_en.html` сгенерирован (172 КБ).

**Решения владельца:** термины вкладок SomeStuff оставлены как `Specification` /
`Renum` / `Sum` / `Sync` / `Monitor` — совпадают с командами DSL, чтобы слово в
интерфейсе и в скрипте было одним. Итоговый файл — `Interface_en.html`.

**Проверено:**
`Tools/test_html.ps1` — ALL HTML CHECKS PASSED (HTMLHint по обоим файлам,
`verify.js` по обоим, `localize_html.js check`: keys=159 missing=0 stale=0
obsolete=0 unreviewed=0 empty=0 placeholderMismatch=0).
`node --test Tools/tests/*.test.js` — **46/46 зелёные**.
Сравнение RU/EN вне каталога: **различаются ровно 4 строки** — `lang="ru"/"en"` и
`<title>`. Логика, разметка и стили идентичны, значит расхождений поведения нет.
Кириллица вне каталога EN — 44 литерала, все проверены: мок-данные, значения DSL
(`"без суффикса"`, `"Sync_GUID+…"`, `"STR-001 …"`) и маркеры regexp
(`/пусто|empty/`, `/шаблон/`). Это данные проекта и пользовательские значения —
переводу не подлежат по решению владельца.

**Строки C++ не переводились:** `parseErrorText` и `sourceName` приходят из
моста. EN-палитра покажет русский текст ошибок разбора — это ограничение
поднятого в #254, расширение моста — отдельное согласование.

**#255 закрыт** (`1ebcc4e`): `npm run validate:js` был красным **до** этой задачи —
`.eslintrc.js` не подключал установленный `eslint-plugin-html`, ESLint падал на
первом `<`. Фикс: `plugins: ['html']` + `env.node` (CommonJS-инструменты давали
49 ошибок на `require`/`process`/`module` — предсуществующее состояние, скрытое
падением `validate:js`) + `no-new-func: off` только для `Tools/**` (там `new Function`
— единственный способ исполнить JS из HTML в песочнице; в палитре правило действует,
проверено пробой). `npm run validate` — exit 0 впервые; RU/EN по 0 errors.
8 warnings `no-unused-vars` — настоящий мёртвый код (`err`, `chainIconSvg`,
`renderValueBlock`, `destName`), проверено вручную; отдельная задача.
`Tools/AddOn.plist.in` и правки ссылок в `Tools/AddOn.grc.in` — работа
параллельной сессии, не мои, не откатывались.

**Форма каталога (зафиксирована):** массив записей `{ "key", "text" }` в обоих
языках — код инициализации `I18N_CATALOG.forEach` обязан работать в EN без правок.
Три ошибки найдены и исправлены по ходу: генератор рендерил объект вместо массива
(EN упал бы при загрузке), склейка записей без запятых, `.map(fn, [])` — лишний
thisArg. Каждая закрыта тестом.

**Сборка AC25:** `BuildAddOn.py -v 25` — `AI_BUILD_RESULT status=success`,
`SomeStuff.apx` 16.4 МБ. Оба HTML вшиты и изолированы: RU смещение 15794448
(185 928 Б, `lang=ru`), EN 15980376 (170 819 Б, `lang=en`). Дельта по 2302 Б
у обоих — CRLF→LF при компиляции ресурса. Маркеры EN-специфичных строк
(`Property Description Editor`, `no suffix`) найдены в бинарнике.

**Next Step:** нет — задача закрыта. Опционально, по отдельному запросу:
запуск Archicad для проверки переключения языка (не выполнялся — проверено
только, что ресурсы собраны и разнесены).

**Last Checkpoint:** `b9f8e34` — 15 файлов: `Interface_ru.html` (каталог + миграция
159 строк), новые `Interface_en.html`, `i18n/{en.json,glossary.md,inventory.md,README.md}`,
`Tools/localize_html.js`, `Tools/tests/{localize_html,interface_localization}.test.js`,
`Tools/verify.js`, `Tools/test_html.ps1`, `Tools/restart_archicad_for_test.ps1`,
`package.json`, `Docs/modules/dialogs/BrowserPalette.md`, `IDEA.md`.

**Отложено воркtree (не мои правки, параллельная сессия):**
`Tools/AddOn.plist.in` (ссылка на kuvbur.org), `Tools/AddOn.grc.in` (там же
ссылка + почта; МОЯ строка `Interface_en.html` в блоке `ID_ADDON_HTML_ENG`
тоже там — коммит намеренно её не захватил, чтобы не присвоить чужую работу;
при следующем коммите проверить diff `grc.in` вручную).

## Закрыто — #249 (формула в критерии/разбивке ReNum)

**Scope:** только `Sources/AddOn/ReNum.cpp/.hpp`, `tests/TestRenum.cpp`,
`tests/TestFunc.cpp/.hpp`, `wiki/ru/Element-Renumbering-ru.md`,
`Docs/modules/ReNum.md`, `Sources/AddOnResources/RFIX/HTML/ТЗ интерфейс.md`
(§7, только markdown). `Helpers.cpp` не трогался — он был под замком #235.

**Реализовано (путь A, решение владельца в комментарии issue):** детектор —
двойные кавычки, всё между ними шаблон; критерий и разбивка. Шаблон вырезается
**до** `ToLowerCase` (регистр литерала сохраняется), имя свойства между `%…%`
приводит `ReplaceProcToBrace`. Проверка существования свойства для формульной
части не выполняется и в `error_propertyname` она не попадает. Вычисление —
существующий путь `hasFormula` → `ReadFormula` → `ReplaceParamInExpression` →
`EvalExpression`, новый код в `ElementsSeparation` не потребовался.

**Две ошибки, найденные при разборе собственного кода (обе исправлены):**
1. Первая версия оборачивала шаблон в `STRFORMULASTART/END`. Это ломает
   внутреннюю формулу: `EvalExpression` берёт **первую** пару `<…>`
   (`CommonFunction.cpp:1395-1403`) и обрезает по первой `>`. Sync для
   кавыченной формулы так не делает (`Sync.cpp:1774`).
2. Первая версия теста ждала, что `%h%` внутри `<…>` останется `%h%`.
   `ReplaceProcToBrace` переписывает **все** пары `%…%` — ограничение
   задокументировано в вики (процент по модулю в `<…>` записать нельзя).

**Замечено про движок (не чинил, поведение Sync/Spec общее):**
`ReplaceParamInExpression` возвращает `true`, если сработала **хотя бы одна**
зависимость; остальные заменяет пустой строкой. Формула с одной битой
зависимостью даст `Кx2-`, а не отказ. Отдельного кода отказа не вводил —
пробел в «отказе невычислимой формулы» из issue закрыт по факту:
невычислимая даёт `isValid == false` → существующий `RENUM_SKIP` + `msg_rep`.

**Проверено:** сборка AC25 Debug — success; sweep AC25–29 — success, без
предупреждений; LSP по `ReNum.cpp`/`TestRenum.cpp` — 0 диагностик; тесты AC25
свежие (`suites=68 passed=2512 failed=0`, `TestRenumFormulaParse` 22/0,
`TestRenumPosLogic` 20/0). Runtime: **не проверено** — формульный критерий на
живой модели не прогонялся, нужен PLN с `Renum{"…"}`.
Открыто: AC22–24 не собирались, macOS — `not verified` (CI).
`Interface_ru.html` не правился, `test_html.ps1` не требовался.

---

# Previous State

#246 закрыта (см. `## Archive`), #228 закрыт 2026-10-02
(см. `## Closed` ниже), открыта параллельная задача #217.

---

## IN_PROGRESS — слияние `spec_refactor` в `master`

**Scope:** только слияние ветки (fast-forward) и проверки перед ним. Правок
production-кода в этом шаге нет — единственный фикс (#250) уже закоммичен.

**Состояние:** `master` — предок `spec_refactor` (`spec_refactor..master` пусто),
слияние будет fast-forward, конфликтов не ожидается. Рабочее дерево чистое.

**mac-падения закрыты дважды**, оба раза по логу CI, не локально:
- `7af05be` — неиспользуемый захват `rule` в лямбде `report`
  (`SpecPlanning.cpp:283`), диагноз `-Wunused-lambda-capture`;
- `ee5d439` — `GS::Array = {}` вместо `Clear ()`
  (`CommonFunction.cpp:2769`), неоднозначный `operator=`.

MSVC оба диагноза не выдаёт, поэтому Windows-сборка зелёная, а mac-job'ы
красные во всех пяти версиях. Прочие присваивания `= {};` в production-коде
проверены по типам — `GS::Array` среди них не осталось.

**Проверено:** тесты AC25 `suites=67 passed=2475 failed=0`, `EXIT 0` (отчёт
свежее `.apx`); сборка AC25 Debug — success; LSP по изменённым файлам — чисто;
clang по `CommonFunction.cpp` — неоднозначных `operator=` не осталось.

**Next Step:** ждать прогон CI на `ee5d439`; если mac покажет третий диагноз
того же рода — закрывать по логу, локальная имитация mac ненадёжна (см.
грабли). Слияние в `master` — только по явному указанию владельца.

---

## Closed — #228 (рефакторинг Spec)

**#228 закрыт 2026-10-02 решением владельца как «рефакторинг выполнен»** —
см. блок `#228` в `IDEA_ARCHIVE.md`. R3–R9.4 и R9.5 (частично) закрыты
коммитами и прогонами; **не проверено и перенесено в закрытый issue:**
R10.6 (измеренный performance-gate — все A/B шли по `diff_rows`, бюджет
«+5% за чистую архитектуру» не проверен), R8.6 (отказы этапов на живой модели
не наблюдались; часть сценариев недостижима на порту AC25), AC22–24 и macOS.
F1 остаётся в #236, F2 не решена.

---

## WAITING_FOR_TEST — проверки за пользователем

- #234 (мост палитры): вкладка «Спецификация» в Archicad не проверялась.
- #189 `2213388`: инлайн-кнопка сброса убрана из строк «Монитора»; в строке
  осталась только кнопка закрепления.
- Ревизия Sync `9ae0138`: GUI диалога другой базы (заголовок/подписи из
  ресурсов), реальный `Sync_to_GUID` и откат на PLN — по потребности;
  AC22–29 не проверены.

## Backlog — открытые issues

- Roombook: #195 (рефакторинг — код готов, закрывать после пользовательской
  валидации), #198 (замер/оптимизация — после проверки #195), #164 (сверка
  результата Roombook с прежним на PLN).
- Состав материалов: #248 — https://github.com/kuvbur/AddOn_SomeStuff/issues/248
  (сопоставление компонентов, коэффициент запаса, послойные значения и Roombook;
  план в issue). `Helpers.cpp` вне scope текущего рефакторинга Spec; код не менялся.
  Проверка путей решения: статический разбор AC25 завершён; перенос `kzap` перед
  расчётом и флагов перед сохранением копии обоснован. Сопоставление разделить на
  проверку и обогащение; пустой предыдущий состав обрабатывать отдельно от
  несовпадения размеров. Общий `qty` используется как контекст текущего слоя
  (`Helpers.cpp:7706–7715,7745`, поиск обычного ключа: `3929`), суммой не заменять.
  SDK25 `API_MorphType.html` подтверждает: `buildingMaterial` — индекс материала,
  не композита; кандидат — существующая basic-ветка, результат требует теста.
  План: `.hermes/plans/2026-10-02_175212-spec-composition-kzap.md` (8 этапов).
  **Уточнение 2026-10-02: длинная ветка принадлежит Spec, не Roombook.** В
  `Roombook.cpp` 0 вхождений `fromQuantity` и `composite_pen`; его материал-параметр
  помечается только `fromMaterial` (`Roombook.cpp:2300-2302`), поэтому `hasQuantity`
  остаётся false (`Helpers.cpp:5544`), а `ReadQuantities` не вызывается;
  `Favorite_ReadComposite` передаёт `needReadQuantities = false` (`:5479-5480`).
  Единственное присваивание `needReadQuantities = true` во всём `Helpers.cpp` —
  `ComponentsProfileStructure` при `composite_pen < 0` (`:8945-8946`). Включает
  количества Spec (`Spec.cpp:1491-1492`). Значения в строках Spec берутся только
  из `composite[n_layer].val` (`Spec.cpp:1792`), поля `.qty`/`.kzap` в `spec/`
  не читаются. Значит потеря `kzap` (`:7121` против `:7373-7413`) бьёт по Spec.
  Находка №3 issue (`{@material:qty}`) — не дефект, это послойный контекст
  (`:7706-7715`→`7745`→`7749`, обычный ключ ищется первым `:3929`).
  **ПРИЧИНА НАЙДЕНА 2026-10-02 (статически, без догадок).** Владелец подтвердил:
  «старый метод» из отчёта возникает при запуске Spec; сообщение единственное
  в дереве и под `#if defined(TESTING)` (`Helpers.cpp:6984-6987`). Оно печатается
  только при `isOk == false` (`:6983`), а `isOk` падает только при равных размерах
  (`:6893`) — значит размеры равны. Из четырёх причин сработала **первая**:
  для `composite_pen < 0` (Spec = −2) ветка `:9221-9222` копирует состав
  **без присваивания `num`**, а дефолт `num` = 0 (`CommonFunction.hpp:120`);
  нумерация `:9233-9234` живёт в `else` и для отрицательного пера не выполняется.
  `0 <= 0` → длинная ветка **всегда**, независимо от модели. Гипотеза про
  `structype` для профиля опровергнута: профиль задаёт его из `ProfileItem`
  (`:9064-9076`), композит — `flagBits` (`:8896`); `inx`-причина тоже исключена.
  Следствие: потеря `kzap` **маскируется** дефектом `num` — до фикса `:9221-9222`
  числа нельзя считать эталонными. Фикс A = нумерация слоёв, не одна строка `kzap`.
  **Решения владельца 2026-10-02 (все три вопроса закрыты):** нумерация `pen < 0`
  — по порядку в массиве; разные размеры составов — НЕ отказ, считать все данные
  (значит функция сопоставления возвращает три исхода `Matched`/`Partial`/`Mismatch`
  и `matchedPrefix`, длинная ветка становится рабочей, слои сверх префикса сохраняют
  свои величины); чтение состава морфа в объём не входит — остаётся открытой
  находкой, `Helpers.cpp:9437-9443` не трогаем. Отменено моё предложение считать
  разные размеры отказом. План — 7 этапов.
  Scope продолжения: только диагностический baseline AC25 Windows, без правок
  production-кода и без сравнения с историческими эталонами Spec.
  Plan: [/] новый baseline; [x] причина установлена статически;
  [x] решения владельца по трём вопросам; [/] подтверждение точкой останова
  `:6895` на модели; [ ] регрессии и фиксы этапов 2–6;
  [ ] документация и приёмка (этап 7). Next: жду «старт».
  Владелец разрешил текущий `test_25.pln` как новый baseline:
  SHA-256 `8f6ee4be36857f241494d5746fde5429c4fb9da60d094bf74d09ca7cc4901228`;
  `Test_file/` и копия `Build/SomeStuff/25/` совпадают. Старый `db1690f…`
  для этого исследования не используется. Next Step: запуск под VS и наблюдение
  ReadQuantities. Runtime: not verified.
  Last Checkpoint: для этой проверки нет; production-код не менялся, коммит не создавался.
- Нумерация: #249 — **код готов** (формула в качестве свойства-критерия и
  свойства-разделителя, детектор — двойные кавычки, путь A). Сборка AC25–29 и
  тесты зелёные; runtime на модели с `Renum{"…"}` **не проверен**. Issue оставить
  открытым до проверки владельцем.
- Монитор/палитра: #159 (маркировка — код готов, визуал за пользователем),
  #186 (адаптивная палитра), #187 («Автоформат» описания), #178 (сужение
  палитры, визуал за пользователем), #177/#176 (ведомости
  Navigator/TableRenderer).
- Runtime-набор R2–R10: #163–#171 (включая #169 R8 Teamwork). #170 (R9:
  сборки AC25–29) закрыт — см. `IDEA_ARCHIVE.md`.
- Конструкция: #235 закрыт 2026-10-03 (`2da2264`), см. `IDEA_ARCHIVE.md`.
  Реализован объём «только Basic ↔ Composite» для стен/крыш/оболочек/
  перекрытий; `Profile`, колонны и балки остались вне объёма по решению
  владельца. Открытых вопросов по сегментам и по стенам в этом issue нет.
- Спецификация: рефакторинг #228 перенесён в `IDEA_ARCHIVE.md` (R3–R9.5
  закрыты, `cd2a52d`; R8.6 и R10 остаются открытыми). Открыты по #228:
  **F1 (#236)** — диагностика исправлена (issue CLOSED 2026-10-02, `0bd856a`),
  но политика разрушительных действий при неполных данных НЕ решена: при
  `stop_on_error` недостоверный расчёт по-прежнему удаляет ранее размещённые
  строки. Причина удаления в плане больше не путается: `RowRejectedByCalc`
  (отказ расчёта) отделена от `RowAlreadyClaimed` (строку забрал другой
  объект). Механизм `out_param` и порядок записи связи ДО проверки схемы не
  менялись; блокировка элементов закрыта отдельно в #243. **F2** —
  достоверный внешний статус ошибок, публичная семантика не менялась.
- #245 закрыт 2026-10-02 (`0657dfd`, приёмка `db5ecfa`): все всплывающие окна
  запуска убраны, в `spec/` `ACAPI_WriteReport` не осталось. Вживую не
  наблюдалось: вёрстка окна результата и сценарий «пустое выделение + все флаги
  выключены» на стенде не воспроизводятся.
- Тесты: #233 (AC22/AC23).
- Прочее: #156, #155, #149/#148, #143/#142/#141, #136, #130/#129/#128, #115.

Закрыты 2026-10-02: #236 (диагностика причины удаления), #234 (валидатор
правила по GUID + мост палитры), #243 (заблокированные строки), #245 (единое
окно результата), #244 (мост вкладки палитры), #232 (ошибочное ожидание
теста `doubleValue`, код `6617c4b`), #231 (машиночитаемый результат прогона),
#230 (тег набора в отклонениях), #227 (дамп значений элементов в ответе Spec).
#181 — реализован (`83cf8c2`), но issue оставлено открытым: закрывать
без решения владельца нельзя.

### #227 / #230 / #231 — закрытие 2026-10-02

**Runtime = `not verified` по решению владельца** (Archicad запущен
параллельной сессией, `.apx` заблокирован — LNK1168; владелец велел не
трогать). Поэтому проверка велась по артефактам, а не наблюдением.

- **#231** — `TestKit`: построчный `fflush` в файл отчёта, печатаются только
  отклонения/измерения/сводка, `SUMMARY … EXIT 0`, отбор наборов через
  `SMSTF_TEST` (группа, префикс `Sync*`, список через запятую), `DBrequire`
  и `DBskip`. Факт: `SUMMARY suites=70 passed=2861 failed=0 suppressed=0
  notes=36`, `EXIT 0`; наборы `TestSpecScenarioMatrix`/`TestSpecOperationMatrix`
  удалены владельцем осознанно, реестр с этим приведён в соответствие.
  Обещанный в `TestKit.hpp:56` скрипт сверки
  реестра отсутствовал — создан `Tools/check_test_registry.py`; он сразу
  нашёл дубль регистрации `TestSpecOutputSchema` (двойной прогон набора и
  вдвое завышенный `passed=`), дубль убран. Сейчас скрипт даёт
  `registered=67 defined=67 declared=67 OK`.
- **#230** — тега функции в квадратных скобках в выводе НЕ было: строки
  `FAIL` шли вообще без указания набора (`grep -o "\[[A-Za-z_]*\]"` по
  отчёту — пусто). Добавлено имя набора в квадратных скобках в строки
  отклонений: `FAIL [Набор] …` и `ABORT [Набор] …` (`TestKit.cpp:233,248`),
  формат документирован в `TestKit.hpp`. Причина именно такая: метки
  проверок в разных наборах повторяются, а строка читается вне контекста.
  Наличие строки `ABORT [` подтверждено в собранном бинарнике AC29.
  Прод-`DBtest`/`DBprnt` не тронуты — вывод в них используется и вне тестов.
- **#227** — `includeParameters` (по умолчанию `false`) даёт `created`/
  `modified`/`deleted` с `guid`, `favoriteName`, `sourceElement`, `property`,
  `gdlParameter`; списки отсортированы по имени, сбор дампа выключен по
  умолчанию. GDL-параметры пишутся в memo и удаляются из `ParamDictValue`
  до `paramOut`, поэтому дамп собирается в момент записи в memo
  (`SpecExecutor.cpp:157-158`), там же берётся значение после приведения к
  типу. В записанных прогонах `gdlParameter` пуст — правила не содержали
  `{@gdl:…}`, а не дефект сборки.

**Не проверено:** новая форма строк `FAIL`/`ABORT` наблюдалась только в
собранном бинарнике, не в живом прогоне; AC25 не слинкован (только
компиляция — линковка заблокирована Archicad). AC26–29 `success`, AC22–24 и
macOS — `not verified`. Смысла `TestFormatStringFormula` (#231) как
объявленного набора не имеет — отдельно.

## Грабли

- **ID контрола в `'GDLG'` = позиция строки, а не число в комментарии.**
  `/* [ n] */` — пояснение для человека, компилятор его не читает: кнопка,
  объявленная раньше списка, получает ID 3, а список — ID 4. Симптомы
  неочевидые: список пуст (нет заголовков/строк), а подпись из `DGSetItemText`
  уходит «в никуда», и кнопка показывает дефолтный ресурсный текст.
  Компилятор ресурсов такое НЕ диагностирует — молчит. Всегда держать
  `[n]` по возрастанию в порядке строк; при добавлении кнопки в середину —
  дописывать её последней строкой. То же для `'DLGH'`.
- **Выделение в 3D-окне требует СНАЧАЛА перехода в это окно.**
  `ACAPI_Selection_Select` / `ACAPI_Element_Select` выделяют элементы только
  ТЕКУЩЕЙ БД: до перехода в 3D элементы чужой базы выделить нельзя
  (документирован `APIERR_BADDATABASE`). Рабочий порядок для показа элементов
  из другой базы: `ShowAllIn3D` → выделение → подсветка → `ZoomToElements` →
  redraw. Подсветка цветом, в отличие от выделения, работает и для элементов
  чужой базы (контракт — «in the 2D … and 3D window»), поэтому она и нужна
  поверх выделения. Отказ выделения/зума не должен прерывать остальные шаги.
  Ошибка воспроизведения: «выделить GUID'ы, потом уйти в 3D» — выделение
  срабатывает в плане, но в 3D элементы не выделены; «сначала перенести окно»
  — элементы становятся недоступны вовсе.
- **Собранный `.apx` может содержать устаревший ресурс.** `AddOn.grc`
  генерируется из `Tools/AddOn.grc.in` на этапе конфигурации CMake, поэтому
  правка шаблона без пересборки не попадает в аддон; при отладке GUI всегда
  сверять mtime `.apx` и `ResourceObjects/AddOn.grc.rc2` с текущим временем.
  Побочный эффект: временная подмена шаблола ради проверки гипотезы
  «сбивает» сгенерированный `.grc` — восстановить нужно и то, и другое.
- **HTML вшит в ресурс** (`'DATA' ID_ADDON_HTML`) — правки `Interface_ru.html`
  требуют пересборки; после каждой правки `Tools/test_html.ps1`.
- **Дубли строк классификации**: C++ кладёт словарь и под `systemname`, и под
  `systemname_full` → записи ×2 (JS-дедуп `uniqueClassificationOptions`).
- **Ранний `return` по `data.common`** прятал выбор классификации и «Применить
  ко всем» при единой классификации/одном элементе.
- **prefs проекта ломают Teamwork** — настройки только в локальный
  `…/GRAPHISOFT/SomeStuff/SyncSettings.dat`; `ReadSyncSettingsFromFile`
  отвергает чужую `PreferencesVersion` → новый массив в настройках требует bump
  версии.
- **Undo-области на каждый элемент** (ResetProperty.cpp) — сотни undo-шагов;
  док DevKit требует одну область на действие.
- **Ключи кэша параметров всегда в нижнем регистре** (prefix + ToLowerCase +
  BRACEEND) — имя без нормализации регистра = правило молча не срабатывает.
- **Мост = инлайн-функции `RegisterACAPIJavaScriptObject`**; аргументы
  `JSFunction` парсить через `DynamicCast<JSValue>` — `DynamicCast<JSArray>`
  ронял ArchiCAD. Ответ моста — строка JSON: вложенные `DG::JSObject`/`JSArray`
  при передаче в JS теряются.
- **Подсветка**: `APIIo_HighlightElementsID` + `APIDo_ZoomToElementsID`
  триггерят SelectionChangeHandler → `static suppressSelectionRefresh`.
  `SetElementHighlight` только AC26+/27+; AC25 — прямой `ACAPI_Interface`,
  сброс = вызов без par1, Clear перед Set.
- **Данные «Монитора» — только из `PROPERTYCACHE()`**, значения — по
  элементам отдельно; в кэше `property` — только определения.
- **`gh` CLI не в PATH** — `export PATH="$PATH:/c/Program Files/GitHub CLI"`;
  Reviews/ в .gitignore (BOM+LF, править через python `utf-8-sig`).
- **Постоянный MSBuild-узел VS держит `.tlog`** (`nodeReuse:true`,
  `MSB4018 … read.1.tlog`) — процесс живёт часами и при этом **не** означает
  идущую сборку: проверить `Get-CimInstance Win32_Process` по `CommandLine` и
  просто повторить `BuildAddOn.py`, а не ждать и не убивать узел.
- **Править `_generated/*.json` можно только вручную**: генератор обнуляет
  `callgraph.json`, а `json.dump(..., indent=2)` из Windows-скрипта даёт CRLF —
  стиль HEAD: LF, без BOM, **без завершающего LF**.
- **Снято автором (не фиксить)**: `Dimensions.cpp:158` `pen_original`;
  пересоздание элементов отделки в Roombook; `Sync.cpp:419-428`
  накопительный `epm`.
- **MSVC не выдаёт часть диагнозов clang — локальная зелёная сборка Windows
  не означает, что mac-сборка в CI зелёная.** mac собирает с `-Werror` и
  расширенным набором `-W…`, поэтому диагностики, молчащие в MSVC, роняют
  mac-job'ы. Поймано два диагноза подряд: `-Wunused-lambda-capture`
  (`SpecPlanning.cpp:283`) и неоднозначный `operator=` при `GS::Array = {}`
  (`CommonFunction.cpp:2769`). Windows при этом зелёная во всех пяти версиях —
  и это выглядит как проблема кода под macOS, хотя дело в инструменте.
  **`GS::Array` очищать только `Clear ()`, не `= {};`:** у него есть шаблонный
  `operator=` через `ConversionEnumerator`, и пустой список инициализации
  неоднозначен. `= {};` допустимо для POD-структур и `time_point`.
- **Имитация mac-сборки через clang-cl не работает — не тратить на неё время.**
  Проверено: у `clang-cl` нет части флагов mac (`-std=gnu++20` → `/std:c++20`),
  а локальные заглушки `Sources/AddOn/api_headers/` (только под `_MSC_VER`, в
  mac-сборку не входят) дают диагностики `-Wreserved-macro-identifier`/
  `-Wc++98-compat`/`-Wundef`, которых в mac-логе нет. Отличить свои артефакты
  от дефектов не вышло. Такие классы ошибок надёжнее закрывать по логу CI.
  Если проверка всё-таки нужна, `-W…` нельзя давать после исходника (clang-cl
  считает их файлами), `/Zs` несовместим с `-c`, PCH из БД на диске
  отсутствует, `-D вида -DADDON_NAME=\"X\"` нельзя переразбирать `shlex`
  (кавычки теряются → `missing terminating '"'`), нужны `-DACExtension`,
  `-DAC_25`, `/EHsc` (иначе падает `exprtk.h`).

## Parallel Task — #217 счётчики кэшей EvalExpression

### Scope

Только `Sources/AddOn/CommonFunction.cpp` (`EvalExpression`: инкременты и вызов
сводки перед `Clear`), `Sources/AddOn/Propertycache.hpp/.cpp`
(`FormulaCacheStats`, поле в `PropertyCache`, `ReportFormulaCacheStats`),
карточка `Docs/modules/Propertycache.md`.

https://github.com/kuvbur/AddOn_SomeStuff/issues/217

### Status

**КОД В MAIN, ИЗМЕРЕНИЕ НЕ ВЫПОЛНЕНО.** Счётчики закоммичены и находятся в
коде: `eb80db3` («Счётчики кэша», `CommonFunction.cpp`) и `ec299fd`
(`Propertycache.hpp/.cpp`) — оба предки `master` и `spec_refactor`.
Прежняя запись «BLOCKED_FOR_LINK» устарела: линковка с тех пор выполнялась
многократно, а `.apx` от 2026-10-02 14:39 содержит строку `=EvalExpression=`
и тексты `переполнение кэша`.

Осталось единственное: прочитать сводку вживую. Счётчики печатаются в
`DBprnt`, а не в файл, поэтому значение из последнего прогона
(`%TEMP%\somestuff_test_report.txt`) счётчиков не содержит — их надо читать из
панели «Отладка» при работающем Archicad.

### Last Completed

2026-09-27 — issue #217 создан. `FormulaCacheStats` (5 счётчиков `UInt64`) в
`Propertycache.hpp:122`, поле `mutable formulaCacheStats` в `PropertyCache`
(`:229`) с комментарием, что счётчики не сбрасываются ни в конструкторе, ни в
`Update()` — нужна картина за всю сессию. `ReportFormulaCacheStats`
(`Propertycache.cpp:47`) печатает вызовы, попадания обоих уровней с процентами
и число очисток по переполнению; молчит при `calls == 0`. Инкременты в
`EvalExpression`: `calls` перед внешним кэшем (`CommonFunction.cpp:1361`),
`fullHits` в ветке попадания (`:1373`), `fullClears` перед
`exprResultFullCache.Clear` (`:1367`), `exprHits` в ветке попадания внутреннего
кэша (`:1443`), `exprClears` перед `exprResultCache.Clear` (`:1436`). Всё под
`#if defined(TESTING)` — в обычных сборках счётчиков нет.

### Next Step

Открыть модель и вызвать `EvalExpression` с формулами, ссылающимися на
параметры, затем прочитать сводку `=EvalExpression=` из панели «Отладка».
Ожидание: попаданий мало или нет — формулы со ссылками дают уникальный ключ.
По фактическим долям решить судьбу внешнего кэша.

Правку guard `||`→`&&` (`CommonFunction.cpp`, из `8aaa599`) НЕ трогать —
отдельная задача.

### Last Checkpoint

`eb80db3` + `ec299fd` (код, оба на `master`). Живых значений счётчиков не
наблюдалось.

### Plan

- [x] Issue #217, изучить `PropertyCache` и места очистки кэшей в
  `EvalExpression`.
- [x] `FormulaCacheStats` + поле в `PropertyCache` +
  `ReportFormulaCacheStats` через `DBprnt`; инкременты перед каждым `Clear()`.
- [x] Код закоммичен и в дереве; сборка проходит.
- [/] Прочитать сводку вживую на `test_25.pln`.
- [ ] По фактическим долям попадания решить судьбу внешнего уровня кэша.

### Decisions

- Всё под `#if defined(TESTING)`: `TESTING` определён в Debug и ProfileDebug
  (`Tools/CMakeCommon.cmake:106`), этого хватает для замеров, релизные сборки
  не трогаем.
- Печать привязана к `Clear()`, а не к отдельной команде: сброс по
  переполнению уничтожает рабочее множество, и это единственная точка, где
  сводка ещё не потеряна. Плюс `GetSize() > 4096` — единственное место очистки
  обоих кэшей.
- Счётчики `UInt64` — переполнение `UInt32` через ~4 млрд вызовов в длинной
  сессии недопустимо для точных данных.
- `fullHits`/`exprHits` считаются от `calls` (вызовов, дошедших до кэшей), а не
  от всех вызовов функции: ранние `return false` на пустой строке и отсутствии
  разделителей кэши не трогают.
  **РЕАЛИЗОВАНО 2026-10-02 (AC25, runner exit 0, 2475 passed):**
  1. `Helpers.cpp` `LayerStructype (short flags)` перед `ReadQuantities`; подключена в трёх
     местах чтения сырых флагов SDK `:6822`, `:6876`, `:8918`. Инвариант `structype`
     ∈ {0, `APICWallComp_Core`, `APICWallComp_Finish`}. Маска невозможна: `Core` ==
     `ForSlab` == 0x02. Порядок Core→Finish как в TAPT.
  2. `longWayIsMain = (composite_type == API_ProfileStructure)` — для сложного профиля длинный
     путь основной (порядок слоёв по `rfromstart` против порядка компонентов), сообщение
     «Old method» для этого случая снято.
  **Замер до/после на test_25.pln:** «long way» 36→0, «Old method» 36→0, `ERROR IN TEST` 0,
  создано 36 элементов, 31 строка количеств — **0 из 31 изменились**.
  Перебор всех 17 присваиваний `structype` в дереве: прочие — литералы
  `APICWallComp_*`/`venType`, сентинел `-1` (Roombook), копирование — нормализация не нужна.
  **Гипотеза «`num` теряется в ветке `pen < 0`» опровергнута наблюдением** (num = 1,2,3);
  Fix A (нумерация) отменён.
  Комментарий: issuecomment-5957221473.
  **Аудит длинного пути + исправления (AC25, проверено):**
  1. Фильтр `composite_pen > 0` добавлен в цикл расчёта количеств (`:7085`) — раньше
     длинный путь читал композиты всех перов.
  2. `p.kzap = qtyPtr->kzap` (`:7146`) + `p.unit = qtyPtr->unit` (`:7145`) вместо второго
     поиска по хеш-таблице. До правки `kzap` в длинном пути не встречался ни разу, поэтому
     `qty` считался с `kzap = 1` — занижение в 2,5 раза при запасе 2,5.
  3. `break` после первого непустого композита (`:7053`) — по решению владельца НЕ чинить.
  **Проверка с `kzap = 2,5` у кирпича:** изменились ровно 2 строки из 31 (6,70→16,76,
  отношение 2,5015; парапет 2,492…2,508 — отклонение ≤0,8% это округление шага 0,01),
  остальные 29 не изменились. 0 «long way», 0 `ERROR IN TEST`, 212 `: ok`, новых
  сообщений в отчёте нет (побайтовое сравнение с базовым прогоном).
  Комментарий: issuecomment-5957440037.
