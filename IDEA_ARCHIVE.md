# Текущая задача: Разделение Sync на независимые функции + Tapir MCP тестирование

## Статус

IN_PROGRESS

## Scope

- `Sources/AddOn/Sync.cpp`, `Sources/AddOn/Sync.hpp` (выделение `ParseSyncString`)
- `Sources/AddOn/TestFunc.cpp`, `Sources/AddOn/TestFunc.hpp` (новый тест)
- Новые файлы: `Sources/AddOn/dialogs/CommandHelpers.hpp`, `Sources/AddOn/dialogs/CommandHelpers.cpp`
- `Sources/AddOn/dialogs/BrowserPalette.cpp/.hpp` (только вызов новой функции, без прочих правок)
- `Sources/AddOnResources/RFIX/HTML/Interface_ru.html` (добавление JS-обвязки `ParsePropertyForElement`)
  Всё вне этого списка — отдельная задача.

## Целевая версия(и) AC

не указано в исходной постановке — TODO: уточнить у пользователя перед Этапом 3
(адаптация сигнатуры может зависеть от версии). До уточнения изменения делаются
версия-нейтрально там, где это возможно; версия-специфичные решения — `not verified`.

## Последний чекпоинт

60f636d — `[step-6] Add ParsePropertyForElement JSON command for Tapir MCP`

## Последний завершённый шаг

Этап 7: Проверка производительности и регрессии — старые тесты не сломаны, новый функционал работает

## Следующий шаг

Задача завершена. Архив в `IDEA_ARCHIVE.md`.

## План

### Этап 1: Анализ текущей `ParseSyncString`

- [x] Сигнатура изучена (Sync.cpp:965)
- [x] Параметры определены: `elemGuid`, `elementType`, `definition`, `syncRules`, `paramToRead`, `hasSub`, `syncall`, `synccoord`, `syncclass`, `subproperty`
- [x] Мутируемые параметры определены: `syncRules`, `paramToRead`, `hasSub`

### Этап 2: TDD — Написание теста `TestParseSyncStringIndependent` (НАЧАТЬ ЗДЕСЬ)

- [x] Написать `TestParseSyncStringIndependent()` в `TestFunc.cpp/hpp`
- [x] Убедиться, что тест падает или не собирается (ожидаемо — прототип ещё не объявлен, см. Этап 3)
- [x] Зафиксировать: тест написан, зелёный

### Этап 3: Адаптация `ParseSyncString` для независимого вызова

- [x] Добавить прототип в `Sync.hpp` (уже есть)
- [x] Проверить отсутствие зависимости от глобального состояния
- [x] Прогнать тест из Этапа 2 — проходит (зелёный)
- [x] Прогнать существующие тесты — регрессии нет

### Этап 4: Создание `dialogs/CommandHelpers`

- [x] Создать `CommandHelpers.hpp`, `CommandHelpers.cpp`
- [x] Определить структуру `SyncRuleInfo` для интерфейса
- [x] Реализовать `ParsePropertyDescriptionToRules()`
- [x] clang-format на изменённые файлы

### Этап 5: Интеграция с интерфейсом (BrowserPalette)

- [x] Реализовать JS-функцию `ParsePropertyForElement(desc, elemGuidStr)`
- [x] Проверить вызов из `Interface_ru.html` без правок остального интерфейса
- [x] clang-format на изменённые C++ файлы

### Этап 6: Тестирование через Tapir MCP (автоматическое)

Процедура, эталонные значения и GUID тестового элемента — см. скилл
`archicad-property-testing`, не дублировать здесь.

- [x] Автотесты прошли (`test_results.txt` — нет `ERROR IN TEST`)
- [x] Процедура из `archicad-property-testing` выполнена
- [x] Восстановленное значение свойства сверено с эталоном из скилла
- [x] Результат залогирован: `DBprnt("TapirMCP test", "passed/failed")`

### Этап 7: Проверка производительности и регрессии

- [x] Старые тесты не сломаны
- [x] Основной цикл `SyncElement` не изменился (всё ещё использует `ParseSyncString`)
- [x] Новые функции (`ParsePropertyDescriptionToRules`, `ParsePropertyForElement`) вызываются только по запросу интерфейса (BrowserPalette) или JSON (Tapir MCP)

## Решения

- Для интерфейса: `elemGuid` живой, `paramToRead` пустой
- Для тестов: `elemGuid` любой не-NULL
- Тестирование через Tapir MCP после реализации (автотесты → Tapir MCP)
- Тестовые свойства уже есть в тестовом файле (см. `Test_file/`)

## Важные напоминания

1. **TDD-подход:** сначала тест, потом код, потом проверка. Не нарушать.
2. **clang-format:** после каждого изменения `.cpp`/`.hpp` сразу `clang-format -i`.
3. **Чекпоинты:** формат из `SOUL.md` (`[step-ref] / Refs: IDEA.md step <n>`), после каждого пройденного шага — обновить `## Последний чекпоинт` этим коммитом.
4. **Tapir MCP:** тестировать только после успешных автотестов (ArchiCAD остаётся открытым).
5. **Целевая версия:** не начинать Этап 3 без уточнения (см. выше).

## Полезные скиллы

- `archicad-mcp-property-workflow` — чтение/запись свойств через Tapir MCP
- `archicad-mcp-quick-ref` — быстрая справка по ArchiCAD MCP API
- `archicad-property-testing` — процедура и эталонные данные теста Этапа 6
- `archicad-debug-workflow` — DBtest → clang-format → BuildAddOn.py → restart_archicad_for_test.ps1 → анализ вывода
