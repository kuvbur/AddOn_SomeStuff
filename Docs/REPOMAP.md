# REPOMAP — Карта репозитория

> Обновлено 2026-10-15 (HEAD `68efad7`, ветвь `roombook_refactor`): roombook-срез документации — `roombook/` (13 файлов) после выносов функций #275/#272/#274/#267; `symbols.json` 1471 → 1578, `callgraph.json` 696 → 699 рёбер (roombook-файлы + `RoomBookCommand`; координаты обновлены, `call_sites` не пересобирались). Предыдущая ревизия: 2026-10-02 (HEAD `2101967`).

## Корень
`D:/SomeStuff_addon` — git repo. Код и документация ведутся в `master`; глобальные доработки по модулям идут в отдельных feature-ветках от него (AGENTS.md §17).

## Структура

```
D:/SomeStuff_addon/
├── Sources/AddOn/              # C++ source (ВСЕ исходники здесь)
│   ├── SomeStuff_Main.cpp/hpp  # Entry point
│   ├── Helpers.*               # Core helpers
│   ├── Propertycache.*         # Property cache
│   ├── Sync.*                  # Sync & monitoring
│   ├── Summ.*                  # Property summation
│   ├── ReNum.*                 # Renumbering
│   ├── Dimensions.*            # Dimensions (pen_original: строки 49/184 — DO NOT FIX, AGENTS.md §16)
│   ├── roombook/               # Finish schedule: Roombook (оркестратор) +
│   │                            #   RoomSurfaces/Candidates/Partition/Finish
│   │                            #   (.cpp/.hpp) + CoordinateSearchCache/
│   │                            #   FloorCoreGeometry/RoomAnalyticGeometry (.hpp)
│   ├── ClassificationFunction.*# Auto-classification
│   ├── MEPv1.*                 # MEP
│   ├── CommonFunction.*        # Utils
│   ├── tests/                 # Local tests (TESTING only)
│   │   ├── TestFunc.*         # Registry of suites + TestKit backend
│   │   ├── TestKit.*          # Report file, counters, SMSTF_TEST filter
│   │   └── Test{Spec,Sync,Param,Format,Renum,Core,Util}.*
│   ├── dialogs/                # DG dialogs, HTML interface
│   │   ├── BrowserPalette.*    # Palette + JS bridge
│   │   ├── SyncSettings.*      # Settings (SyncSettings.dat)
│   │   ├── OtherDbDialog.*     # Choose DB/story for linked elements (#210)
│   │   ├── CommandHelpers.*, DG4rule.*
│   ├── json_commands/          # ArchiCAD JSON commands, AC25+ (#205/#206/#207/#208/#213)
│   │   ├── CommandBase.*, JsonCommandRegistrar.*
│   │   ├── RoomBookCommand.*, SpecCommand.*, SyncAllCommand.*, HealthCommand.*
│   │   └── How JSON Commands work.md
│   ├── spec/                   # Spec engine (Spec.*, SpecPlanning.*, SpecHelpers.*, Spec_libpart.*)
│   ├── table/                  # Table renderer, navigator
│   ├── pk/                     # Automation, reset, revision
│   ├── api_headers/            # AC API headers
│   └── third_party/            # qrcodegen
├── Sources/AddOnResources/     # Resources (RFIX/HTML/Interface_ru.html)
├── Sources/MacDarkModeIcon/    # macOS assets
├── Test_file/                  # AC test PLN files (DO NOT EDIT normally)
├── Tools/
│   ├── BuildAddOn.py           # Build/config script
│   ├── CMakeCommon.cmake       # CMake config
│   ├── CompileResources.py     # Resource compiler
│   ├── test_html.ps1           # HTML test (run after every Interface_ru.html edit)
│   ├── restart_archicad_for_test.ps1  # Final build + AC launch + JSON tests
│   └── AddOn.grc.in            # Resources (IDs, RU/EN strings)
├── Docs/
│   ├── _progress.md           # Состояние работ по документации (единственный источник resume)
│   ├── ARCHITECTURE.md         # Architecture overview
│   ├── REPOMAP.md              # This file
│   ├── DISCREPANCIES.md        # Comment/code mismatches
│   ├── SEQUENCES.md            # 11 sequence diagrams (Mermaid)
│   ├── modules/                # Per-module cards (28 файлов: 15 в корне + spec/ 6,
│   │                            #   dialogs/ 4, table/ 2, third_party/ 1;
│   │                            #   dialogs/OtherDbDialog — описан в карточке Sync.md, #210)
│   ├── tools/
│   │   ├── generate_symbols.py  # НЕ запускать для данных: символы — regex-fallback, негоден
│   │   └── UPDATE_PROCEDURE.md  # Doc update procedure
│   └── _generated/
│       ├── symbols.json         # Символы (1578 записей; spec — clangd MCP, 113;
│       │                            #   roombook — clangd MCP, коммиты 18d05b7..68efad7)
│       └── callgraph.json       # Рёбра вызовов (699; spec — 272, сверены с исходником;
│       │                            #   roombook-срез: 378 координат обновлено, 3 ребра
│       │                            #   перенаправлены на CommonFunction, call_sites 1-based)
├── compile_commands.json       # LSP compile commands (51 запись, AC25)
├── config.json                 # Build config
├── CMakeLists.txt              # CMake entry
├── package.json                # npm package
└── AGENTS.md                   # Repo rules
```

## Модули и файлы (сводка)

| Каталог | Назначение | Файлы |
|---------|-----------|--------|
| Core | Entry point, helpers, properties | SomeStuff_Main, Helpers, Propertycache, Sync, CommonFunction |
| Data | Суммирование, перенумерация, отделка | Summ, ReNum, roombook/ |
| Rules | Спецификации, авто-классификация, MEP | spec/, ClassificationFunction, MEPv1 |
| Automation | Выравнивание, сброс, ревизии | pk/ (AutomateFunction, ResetProperty, Revision) |
| UI | Dialogs, palette, HTML interface | dialogs/ (BrowserPalette, SyncSettings, OtherDbDialog, CommandHelpers, DG4rule) |
| JSON commands | HTTP/JSON API AC25+ | json_commands/ (CommandBase, JsonCommandRegistrar, RoomBook, Spec, SyncAll, Health) |
| Tables | Table rendering, navigation | table/TableRenderer, table/TablesNavigator |
| Tests | TESTING-тесты | tests/ (TestFunc, TestKit, TestSpec/Sync/Param/Format/Renum/Core/Util) |
| Third-party | Embedded libraries | third_party/qrcodegen |

## Статистика (из compile_commands.json, 2026-10-15, AC25)
- Всего записей: 51 (1 cmake_pch + 48 source-единиц + 2 записи с `-I SOURCE_DIR`)
- `.cpp`: 48 — корень 10, `dialogs/` 5, `json_commands/` 6, `pk/` 3, **`spec/` 6**, `table/` 2, `tests/` 10, `third_party/` 1, **`roombook/` 5**
- **include paths**: 62 уникальных, из них 61 существующий; единственный «отсутствующий» — `SOURCE_DIR`, неразвёрнутая переменная CMake в 2 записях, а не путь (пересобрано 2026-10-02, `BuildAddOn.py --lsp`; подсчёт записей обновлён 2026-10-15)
- `compile_commands.json` пересобран 2026-10-02 (`BuildAddOn.py --lsp`): теперь **6 из 6 `spec/*.cpp`**. До пересборки `SpecCompat.cpp` и `SpecPlanning.cpp` в нём не было, хотя оба подключаются из других TU (`#include "spec/SpecCompat.hpp"`) — то есть сбор по нему не покрывал два реальных модуля. **Точное число записей в прежней версии восстановить нельзя:** файл не отслеживается git (`.gitignore`), прежняя копия не сохранилась.

## Изменения с предыдущей ревизии карты (2026-10-15)
- Roombook-срез документации (коммиты `18d05b7..68efad7`: f49015d, b96f5d3 #267, ef5c6e7 #192, 27d759e, f2a5e91 #273-C2, 9963693 #274/#252/#267/#268/#272, 8a863af, 0647f14, 7c7010a #272, 68efad7 #275): `roombook/` — каталог из 13 файлов (была плоская пара `Roombook.*`); `AdvanceProcessPhase` вынесен в `CommonFunction.cpp:2653`; `SearchCoordinate` — новая обёртка координатного поиска с `CoordinateSearchCache` (AC25, Windows); `GetSurfaceFinishLayer` удалена.
- `symbols.json` 1471 → 1578 записей (собрано clangd MCP по roombook-срезу: 10 файлов `roombook/*` + `json_commands/RoomBookCommand.cpp`; строки переведены в 1-based).
- `callgraph.json` 696 → 699 рёбер: 378 координат обновлено, 3 ребра перенаправлены на `CommonFunction.cpp:2653` (`AdvanceProcessPhase`), 21 метод разрешён суффиксным матчингом, 7 неоднозначных рёбер (в т.ч. `Set` у `Frame` и `BeamAxis`) оставлены со старыми координатами; `call_sites` переведены в 1-based, но НЕ пересобирались (метка `roombook_scoped_2026_10_15` в `meta`).
- `compile_commands.json` (AC25): 46 → 51 записей — `roombook/` 5 unit, `tests/` 9 → 10 (`TestRoombookGeometry` переехал из корня), корень 11 → 10.
- Карточка `modules/Roombook.md`, обзорные `ARCHITECTURE.md`/`REPOMAP.md`/`SEQUENCES.md` обновлены; добавлена 11-я sequence-диаграмма (спецификация отделки).

## Изменения с предыдущей ревизии карты (2026-10-02)
- Полностью обновлена документация `spec/`: 6 карточек в `Docs/modules/spec/`, `symbols.json` 940 → 1011 записей (113 для spec), `callgraph.json` 155 → **418** рёбер. Все 272 новых spec-ребра сверены с исходником (0 расхождений).
- Удалены 9 рёбер с устаревшими координатами; каждое заменено исправленным — `PlaceElements` переехал в `SpecExecutor.cpp:11`, `SpecAll` 139→143, `SpecArray` 515→609.
- **140 рёбер (все не-spec) устарели** — унаследовано от сбора 2026-09-22, не результат этой сессии. Пример: `LoadSyncSettingsFromPreferences` помечена `:517`, реальное определение `:444`. Освежение не-spec части — отдельная задача (см. `callgraph.json` → `meta.known_stale_non_spec`).

## Изменения с предыдущей ревизии карты (2026-09-30)
- Тесты вынесены в `Sources/AddOn/tests/` и разбиты по группам (#231): `TestFunc.cpp` 330 KB → 8 KB (реестр 49 наборов) + `TestKit.*` (файловый отчёт, счётчики, отбор `SMSTF_TEST`) + `Test{Spec,Sync,Param,Format,Renum,Core,Util}.*`. CMake не правился — `GLOB_RECURSE CONFIGURE_DEPENDS` подхватывает подпапку, а include-каталог содержит сам `Sources/AddOn`, поэтому `#include "tests/..."` резолвится изнутри, а снаружи — как `"tests/..."`.
- `compile_commands.json` пересобран (`BuildAddOn.py --lsp`): 31 → 39 `.cpp`, 9 записей из `tests/`, старых записей `Sources/AddOn/Test*.cpp` не осталось. Файл не отслеживается git (`.gitignore:384`).

## Изменения с предыдущей ревизии карты (2026-09-25 → 2026-09-27)
- Добавлена команда `SomeStuffCommand.SyncAll` (#213): `json_commands/SyncAllCommand.*` — обёртка над `SyncAndMonAll` без правок `Sync.cpp`; статистика и `compile_commands.json` пересобраны для AC25 (28–29+ не пересобирались).
- (2026-09-22 → 2026-09-25)
- Добавлен `json_commands/` (#205/#206/#207/#208): JSON-команды `SomeStuffCommand.RoomBook` и `SomeStuffCommand.Spec` (AC25+); `HealthCommand` — шаблон, не регистрируется.
- `OtherDbDialog` вынесен из `Sync.cpp` в `dialogs/OtherDbDialog.*` (#210).
- `Spec.cpp` — non-interactive `SpecAll` с `placementPoint`/`ruleNames` и `SpecRunResult` (#207/#208).
- `Helpers.cpp` — фильтр `GetAttributeValues` возвращён к `fromAttribDefinition` (рабочее дерево, #209 — открыт).
