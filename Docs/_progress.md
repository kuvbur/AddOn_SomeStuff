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
