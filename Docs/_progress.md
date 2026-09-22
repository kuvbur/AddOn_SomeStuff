# Состояние работы — документирование кодовой базы

## Текущая ветка
`docs/codebase-map` (создана)

## Хеш коммита
`72ec74a` (2026-09-22) — фазы 0-2 задокументированы и закоммичены

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
| TestFunc | Sources/AddOn/TestFunc.cpp/hpp | 1 | 1 | Большой |
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
