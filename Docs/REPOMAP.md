# REPOMAP — Карта репозитория

> Обновлено 2026-09-25 (рабочее дерево `llm_test`, HEAD `13c1948`): добавлены `json_commands/` и `dialogs/OtherDbDialog.*`; статистика пересобрана.

## Корень
`D:/SomeStuff_addon` — git repo. Основной разработочный код — ветка `llm_test`; документация ведётся на `docs/codebase-map` (AGENTS.md §17).

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
│   ├── Roombook.*              # Finish schedule
│   ├── ClassificationFunction.*# Auto-classification
│   ├── MEPv1.*                 # MEP
│   ├── CommonFunction.*        # Utils
│   ├── TestFunc.*              # Local tests (TESTING only)
│   ├── dialogs/                # DG dialogs, HTML interface
│   │   ├── BrowserPalette.*    # Palette + JS bridge
│   │   ├── SyncSettings.*      # Settings (SyncSettings.dat)
│   │   ├── OtherDbDialog.*     # Choose DB/story for linked elements (#210)
│   │   ├── CommandHelpers.*, DG4rule.*
│   ├── json_commands/          # ArchiCAD JSON commands, AC25+ (#205/#206/#207/#208)
│   │   ├── CommandBase.*, JsonCommandRegistrar.*
│   │   ├── RoomBookCommand.*, SpecCommand.*, HealthCommand.*
│   │   └── How JSON Commands work.md
│   ├── spec/                   # Spec engine
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
│   ├── ARCHITECTURE.md         # Architecture overview
│   ├── REPOMAP.md              # This file
│   ├── DISCREPANCIES.md        # Comment/code mismatches
│   ├── SEQUENCES.md            # 10 sequence diagrams (Mermaid)
│   ├── modules/                # Per-module documentation (24 модуля + json_commands)
│   ├── tools/
│   │   ├── generate_symbols.py  # symbols.json generator (callgraph: НЕ собирается, см. UPDATE_PROCEDURE.md)
│   │   └── UPDATE_PROCEDURE.md  # Doc update procedure
│   └── _generated/
│       ├── symbols.json         # Extracted symbols (8549 entries, regex fallback)
│       └── callgraph.json       # Callgraph edges (155, clangd MCP + grep-fallback)
├── compile_commands.json       # LSP compile commands (31 записей, AC25)
├── config.json                 # Build config
├── CMakeLists.txt              # CMake entry
├── package.json                # npm package
└── AGENTS.md                   # Repo rules
```

## Модули и файлы (сводка)

| Каталог | Назначение | Файлы |
|---------|-----------|--------|
| Core | Entry point, helpers, properties | SomeStuff_Main, Helpers, Propertycache, Sync, CommonFunction |
| Data | Суммирование, перенумерация, отделка | Summ, ReNum, Roombook |
| Rules | Спецификации, авто-классификация, MEP | spec/, ClassificationFunction, MEPv1 |
| Automation | Выравнивание, сброс, ревизии | pk/ (AutomateFunction, ResetProperty, Revision) |
| UI | Dialogs, palette, HTML interface | dialogs/ (BrowserPalette, SyncSettings, OtherDbDialog, CommandHelpers, DG4rule) |
| JSON commands | HTTP/JSON API AC25+ | json_commands/ (CommandBase, JsonCommandRegistrar, RoomBook, Spec, Health) |
| Tables | Table rendering, navigation | table/TableRenderer, table/TablesNavigator |
| Tests | TESTING-тесты | TestFunc |
| Third-party | Embedded libraries | third_party/qrcodegen |

## Статистика (из compile_commands.json, 2026-09-25)
- Всего записей: 31 (1 cmake_pch + 30 source-единиц)
- `.cpp`: 30 (включая `json_commands/` ×5 и `dialogs/OtherDbDialog.cpp`)
- **include paths**: 1464 `/I` флага, 61 уникальный путь, 0 отсутствующих (проверено 2026-09-22, `Docs/_progress.md`)

## Изменения с предыдущей ревизии карты (2026-09-22 → 2026-09-25)
- Добавлен `json_commands/` (#205/#206/#207/#208): JSON-команды `SomeStuffCommand.RoomBook` и `SomeStuffCommand.Spec` (AC25+); `HealthCommand` — шаблон, не регистрируется.
- `OtherDbDialog` вынесен из `Sync.cpp` в `dialogs/OtherDbDialog.*` (#210).
- `Spec.cpp` — non-interactive `SpecAll` с `placementPoint`/`ruleNames` и `SpecRunResult` (#207/#208).
- `Helpers.cpp` — фильтр `GetAttributeValues` возвращён к `fromAttribDefinition` (рабочее дерево, #209 — открыт).
