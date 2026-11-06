# Состояние документации — актуально

Единственный источник возобновления работы по документации. История задач — в git-логе
`Docs/` и в архиве задач владельца (`IDEA.md`/`IDEA_ARCHIVE.md`); здесь — только текущее
состояние и процедура.

## Команды этажей #315 — изолированный снимок (2026-11-06)

- Обновлены карточки CommonFunction/Propertycache/Helpers/Sync и русская страница команд: API-index отделён от UI-index, высота верхнего этажа берётся через виртуальный уровень SDK25. Задокументирован числовой index. [по коду]
- После параллельного коммита873c58f со Sync.cpp и частью Story подготовлена только оставшаяся дельта #315; refactor и RoomBook не включаются в неё. Четыре полных TU SDK25/MSVC C++14 /WX повторно exit0 на финальном снимке; controlled-регрессия64/0 повторена parent. Это не canonical addon build и не native-приёмка.
- symbols.json: свежие настоящие clangd DocumentSymbol isolated snapshot,236cpp-записей CommonFunction/Helpers/Propertycache проверены по исходникам; восстановлена полнота существующих overload-групп, добавлен ReadStory. Остальные947записей и порядок сохранены из873c58f, не из этой коллекции; их координаты не объявляются актуальными. Заголовки проверены clangd, но в cpp-only artifact не добавлялись. Полный parent-аудит специальных header-имён не завершён; опубликованные cpp-записи проверены отдельно. Callgraph и обзорные документы вне адресного обновления.
- Артефакты: Hermes scratch/story-315-only-commit/validation/ (final-results.json и snapshot-final). Рабочие исходники не подменялись; Sync-срез сохранён из параллельного коммита, RoomBook WIP не включён.

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
      CommandHelpers — 27 файлов)
- [x] Фаза 4: `ARCHITECTURE.md`, `REPOMAP.md`, `SEQUENCES.md`
- [x] Фаза 5: сверка -> `DISCREPANCIES.md`
- [x] Roombook-срез: полная регенерация по снимку `b199125`
- [ ] Docs-коммит (решение владельца: ветвь vs master)
- [ ] Не-spec срезы callgraph (отдельная задача)
