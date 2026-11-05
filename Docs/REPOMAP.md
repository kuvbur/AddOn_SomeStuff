# REPOMAP — Карта репозитория

> Обновлено 2026-10-30 (коммит `b199125` `[C3-WIP]`, ветвь `roombook_refactor`): `roombook/` сжат до 9 файлов — surface-модули слиты в `RoombookData.cpp`, писатели вынесены в `RoombookDraw.cpp`/`RoombookQuantities.cpp`; `dialogs/CommandHelpers` удалён (коммит `485072d`, парсинг правил перенесён в `Propertycache.cpp:1344`); `symbols.json` пересобран по диску (1128 записей, 47 файлов), roombook-срез `callgraph.json` пересобран (807 рёбер из 1329). Предыдущая ревизия: 2026-10-15 (`68efad7`).

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
│   ├── roombook/               # Finish schedule: Roombook (оркестратор),
│   │                            #   RoombookData (чтение + surface-pipeline),
│   │                            #   RoombookDraw/RoombookQuantities (писатели) — .cpp,
│   │                            #   + Roombook/RoombookConstants/CoordinateSearchCache/
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
│   │   ├── DG4rule.*
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
│   ├── modules/                # Per-module cards (27 файлов: 15 в корне + spec/ 6,
│   │                            #   dialogs/ 3, table/ 2, third_party/ 1;
│   │                            #   dialogs/OtherDbDialog — описан в карточке Sync.md, #210)
│   ├── tools/
│   │   ├── generate_symbols.py  # НЕ запускать для данных: символы — regex-fallback, негоден
│   │   └── UPDATE_PROCEDURE.md  # Doc update procedure
│   └── _generated/
│       ├── symbols.json         # Символы (1128 записей, 47 файлов, строки 1-based;
│       │                            #   roombook-файлы пересобраны по диску 2026-10-30)
│       └── callgraph.json       # Рёбра вызовов (1329; roombook-срез 807, пересобран
│       │                            #   2026-10-30 после b199125; не-spec срезы —
│       │                            #   прежнего прогона, 140 известных устаревших рёбер)
├── compile_commands.json       # LSP compile commands (54 записи, AC25)
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
| UI | Dialogs, palette, HTML interface | dialogs/ (BrowserPalette, SyncSettings, OtherDbDialog, DG4rule) |
| JSON commands | HTTP/JSON API AC25+ | json_commands/ (CommandBase, JsonCommandRegistrar, RoomBook, Spec, SyncAll, Health) |
| Tables | Table rendering, navigation | table/TableRenderer, table/TablesNavigator |
| Tests | TESTING-тесты | tests/ (TestFunc, TestKit, TestSpec/Sync/Param/Format/Renum/Core/Util) |
| Third-party | Embedded libraries | third_party/qrcodegen |

## Статистика (из compile_commands.json, 2026-10-30, AC25)
- Всего записей: 54 (1 cmake_pch + 51 source-единица + 2 записи с `-I SOURCE_DIR`)
- `.cpp`: 51 — корень 10, `dialogs/` 4, `json_commands/` 6, `pk/` 3, **`spec/` 6**, `table/` 2, `tests/` 15, `third_party/` 1, **`roombook/` 4**
- **include paths**: 62 уникальных, из них 61 существующий; единственный «отсутствующий» — `SOURCE_DIR`, неразвёрнутая переменная CMake в 2 записях, а не путь (пересобрано 2026-10-30, `BuildAddOn.py --lsp` AC25)
- `compile_commands.json` пересобран 2026-10-30 (`BuildAddOn.py --lsp`): **6 из 6 `spec/*.cpp`**, 15 файлов `tests/`, 4 файла `roombook/`. Файл не отслеживается git (`.gitignore`), прежняя копия не сохранилась.

## Изменения с предыдущей ревизии карты (2026-10-30, коммит `b199125`)
- `roombook/` сжат до 9 файлов (4 .cpp + 5 .hpp): surface-модули (`RoomSurfaces`/`Candidates`/`Partition`/`Finish`) слиты в `RoombookData.cpp`; писатели вынесены в `RoombookDraw.cpp` (2745 стр.) и `RoombookQuantities.cpp` (584 стр.).
- `symbols.json` полностью пересобран по диску (1128 записей, 47 файлов; roombook-файлы — clangd MCP + дисковая привязка строк после `b199125`).
- `callgraph.json` roombook-срез пересобран (807 рёбер из 1329; входные рёбра `MenuCommandHandler`/`RoomBookCommand::Execute`/`RoomBookGetFavoritesCommand::Execute` добавлены явно); не-spec срезы — прежнего прогона, 140 известных устаревших рёбер (`meta.known_stale_non_spec`).
- `dialogs/CommandHelpers` удалён (коммит `485072d`): `ParsePropertyDescriptionToRules` перенесён в `Propertycache.cpp:1344`; карточка `Docs/modules/dialogs/CommandHelpers.md` удалена.
- `compile_commands.json` (AC25): 54 записи; `tests/` 10 → 15 (`TestMeshFloor`, `TestFinishReuse`, `TestRoombookGeometry`, regression-файлы под `#ifdef`).
- `Helpers.cpp` — фильтр `GetAttributeValues` возвращён к `fromAttribDefinition` (включён в `b199125`; #209 — открыт).
