# REPOMAP — Карта репозитория

## Корень
`D:/SomeStuff_addon` — git repo, ветка `docs/codebase-map`

## Структура

```
D:/SomeStuff_addon/
├── Sources/AddOn/              # C++ source (ВСЕ исходники здесь)
│   ├── SomeStuff_Main.cpp/hpp  # Entry point
│   ├── Helpers.*               # Core helpers
│   ├── Propertycache.*         # Property cache
│   ├── Sync.*                  # Sync & monitoring
│   ├── Summ.*                  # Property summation
│   ├── Spec.*                  # Spec rules
│   ├── ReNum.*                 # Renumbering
│   ├── Revision.*              # Revision markers
│   ├── Dimensions.*            # Dimensions (line 158: pen_original — DO NOT FIX)
│   ├── Roombook.*              # Finish schedule
│   ├── ClassificationFunction.*# Auto-classification
│   ├── ResetProperty.*         # Property reset (undo regions — see AGENTS.md)
│   ├── AutomateFunction.*      # Alignment (pk/ module)
│   ├── MEPv1.*                 # MEP
│   ├── CommonFunction.*        # Utils
│   ├── dialogs/                # DG dialogs, HTML interface
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
│   └── test_html.ps1           # HTML test (run after every Interface_ru.html edit)
├── docs/
│   ├── ARCHITECTURE.md         # This file
│   ├── REPOMAP.md              # This file
│   ├── modules/                # Per-module documentation
│   │   └── pk.md               # Pilot: pk/ module
│   ├── tools/
│   │   └── generate_symbols.py  # Script: symbols.json + callgraph.json
│   └── _generated/
│       ├── symbols.json         # Extracted symbols (8421 entries)
│       └── callgraph.json       # Callgraph edges (empty — needs work)
├── compile_commands.json       # Build commands (210 entries, no includes)
├── config.json                 # Build config
├── CMakeLists.txt              # CMake entry
├── package.json                # npm package
└── AGENTS.md                   # Repo rules
```

## Модули и файлы (сводка)

| Каталог | Назначение | Файлы |
|---------|-----------|--------|
| Core | Entry point, helpers, properties | SomeStuff_Main, Helpers, Propertycache, Sync, CommonFunction |
| Data | Суммирование, спецификации, перенумерация | Summ, Spec, ReNum, Roombook |
| Automation | Выравнивание, сброс, ревизии | pk/ (AutomateFunction, ResetProperty, Revision) |
| Rules | Auto-classification, MEP | ClassificationFunction, MEPv1 |
| UI | Dialogs, palette, HTML interface | dialogs/, BrowserPalette |
| Tables | Table rendering, navigation | table/TableRenderer, table/TablesNavigator |
| Third-party | Embedded libraries | third_party/qrcodegen |

## Статистика (из compile_commands.json)
- Всего записей: 210
- Cpp файлов: ~80
- C файлов: 1
- HPP файлов: ~40
- H файлов: ~10
- **include paths в compile_commands: 0** (нет –ivfили /isystem флагов)
