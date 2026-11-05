# Состояние документации — актуально

Единственный источник возобновления работы по документации. История задач — в git-логе
`Docs/` и в архиве задач владельца (`IDEA.md`/`IDEA_ARCHIVE.md`); здесь — только текущее
состояние и процедура.

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
