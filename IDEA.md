# Текущая задача: Разделение Sync на независимые функции + Tapir MCP тестирование

## Статус
IN_PROGRESS

## Последний завершённый шаг
Анализ `ParseSyncString` (Этап 1)

## Следующий шаг
Этап 2 (TDD): Написание теста `TestParseSyncStringIndependent`

## План

### Этап 1: Анализ текущей `ParseSyncString` ✅
- Сигнатура изучена (Sync.cpp:965)
- Параметры: `elemGuid`, `elementType`, `definition`, `syncRules`, `paramToRead`, `hasSub`, `syncall`, `synccoord`, `syncclass`, `subproperty`
- Мутирует: `syncRules`, `paramToRead`, `hasSub`

### Этап 2: TDD — Написание теста `TestParseSyncStringIndependent` (НАЧАТЬ ЗДЕСЬ)
1. [ ] Объявить `ParseSyncString` в `Sync.hpp`
2. [ ] Написать `TestParseSyncStringIndependent()` в `TestFunc.cpp/hpp`
3. [ ] Убедиться, что тест падает (ожидаемо)
4. [ ] Зафиксировать: тест написан, упал

### Этап 3: Адаптация `ParseSyncString` для независимого вызова
1. [ ] Добавить прототип в `Sync.hpp`
2. [ ] Проверить отсутствие зависимости от глобального состояния
3. [ ] Проверить тест — должен пройти
4. [ ] Проверить регрессию (старые тесты)

### Этап 4: Создание `dialogs/CommandHelpers`
- Новые файлы: `CommandHelpers.hpp`, `CommandHelpers.cpp`
- Структура `SyncRuleInfo` для интерфейса
- Функция `ParsePropertyDescriptionToRules()`

### Этап 5: Интеграция с интерфейсом (BrowserPalette)
- JS-функция `ParsePropertyForElement(desc, elemGuidStr)`

### Этап 6: Тестирование через Tapir MCP (автоматическое)

**Условие запуска:** После `restart_archicad_for_test.ps1` — ArchiCAD остаётся открытым, если автотесты прошли успешно и в консоли нет ошибок.

**Критерии верификации (тестовое свойство):**
- Элемент: Балка
- GUID: `3CE3A04D-743D-43FE-8D6D-86E25C19DE74`
- Свойство: `{Property:Состав конструкций/Состав конструкции (сложный профиль, перо 20)}`
- Текущее значение: `Штукатурка - 10мм. Минплита - 150мм. Кирпич 250мм Плотность 1800 0,819`
- Ожидаемое поведение: Сброс или изменение значения свойства должно заставить аддон восстановить прежнее значение.

**Процедура Tapir MCP теста:**
1. [ ] Убедиться, что автотесты прошли (`test_results.txt` — нет `ERROR IN TEST`)
2. [ ] Найти балку с GUID `3CE3A04D-743D-43FE-8D6D-86E25C19DE74` в тестовом файле
3. [ ] Прочитать текущее значение свойства `{Property:Состав конструкций/Состав конструкции (сложный профиль, перо 20)}`
4. [ ] Сбросить значение свойства (установить `default` через Tapir MCP)
5. [ ] Подождать 2-3 секунды (сработает синхронизация аддона)
6. [ ] Проверить, что аддон восстановил значение `Штукатурка - 10мм. Минплита - 150мм. Кирпич 250мм Плотность 1800 0,819`
7. [ ] Залогировать результат: `DBprnt("TapirMCP test", "passed/failed")`

**Признаки успешного теста в `test_results.txt`:**
```
== SMSTF == ProjectEventHandlerProc
== SMSTF == Do_ElementMonitor on
== SMSTF == =PropertyCache= Update start
...
== SMSTF == =PropertyCache= Update end
== SMSTF == ProjectEventHandlerProc
== SMSTF == ProjectEventHandlerProc
```
(Аддон продолжает работать, мониторинг активен, кэш обновлён)

### Этап 7: Проверка производительности и регрессии
1. [ ] Убедиться, что старые тесты не сломались
2. [ ] Проверить, что основной цикл `SyncElement` не замедлился
3. [ ] Новые функции вызываются только по запросу интерфейса/JSON (не в фоне)

## Решения
- Для интерфейса: `elemGuid` живой, `paramToRead` пустой
- Для тестов: `elemGuid` любой не-NULL
- Тестирование через Tapir MCP после реализации (автотесты → Tapir MCP)
- Тестовые свойства уже есть в тестовом файле (см. `Test_file/`)

## Важные напоминания
1. **TDD-подход:** Сначала тест, потом код, потом проверка. Не нарушать!
2. **clang-format:** После каждого изменения `.cpp`/`.hpp` сразу запускать `clang-format -i`
3. **Коммиты:** После каждого прохождения теста — коммит с понятным сообщением
4. **Tapir MCP:** Тестировать только после успешных автотестов (ArchiCAD остаётся открытым)
5. **Свойства для тестов:** Уже есть в тестовом файле (балка с GUID `3CE3A04D-743D-43FE-8D6D-86E25C19DE74`)

## Полезные скиллы
- `archicad-mcp-property-workflow` — чтение/запись свойств через Tapir MCP
- `archicad-mcp-quick-ref` — быстрая справка по ArchiCAD MCP API
- `archicad-property-testing` — специфичные тесты свойств
- `archicad-debug-workflow` — процедура отладки (DBtest → clang-format → BuildAddOn.py → restart_archicad_for_test.ps1 → анализ вывода)
