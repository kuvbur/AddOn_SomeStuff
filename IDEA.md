# План разработки: C++ команды для UI вкладки «Монитор»

## Цель

Реализовать C++ JSON-команды аддона, которые заменят моки `ACBridge` в HTML-интерфейсе вкладки «Монитор». Максимально использовать существующий оптимизированный код чтения свойств (PropertyCache, ParamHelpers).

---

## Существующий код для повторного использования

| Компонент | Файл | Назначение |
|-----------|------|------------|
| `PropertyCache::property` | Propertycache.hpp:107 | Словарь всех свойств (ParamDictValue) — самый быстрый доступ |
| `PropertyCache::propertygroups` | Propertycache.hpp:115 | Группы свойств (Guid → API_PropertyGroup) |
| `PropertyCache::systemdict` | Propertycache.hpp:113 | Системы классификации |
| `GetSelectedElements2()` | CommonFunction.hpp:292 | GUID выделенных элементов (оптимизированная) |
| `ParamHelpers::GetParamValueFromCache()` | Propertycache.hpp:46 | Чтение значения из кэша по rawname |
| `ParamHelpers::GetAllPropertyDefinitionToParamDict()` | Propertycache.hpp:92 | Все определения свойств в ParamDictValue |
| `ParamHelpers::GetGroupFromCache()` | Propertycache.hpp:55 | Имя группы свойств по Guid |
| `ParamHelpers::isPropertyDefinitionRead()` | Propertycache.hpp:51 | Проверка готовности кэша |
| `ClassificationFunc::GetAllClassification()` | ClassificationFunction.hpp:26 | Загрузка всех классификаций |
| `ClassificationFunc::ReadSystemDict()` | ClassificationFunction.hpp:64 | Словарь систем/классов |
| `CommandBase / ReadOnlyCommand / ModifyCommand` | json_commands/CommandBase.hpp | База для JSON-команд |
| `GetPropertyDefinitionsCommand` | json_commands/ | Существующая команда (будет доработана) |
| `RegisterACAPIJavaScriptObject()` | dialogs/BrowserPalette.cpp:100 | JS-мост в BrowserPalette |
| `Interface_ru.html` | AddOnResources/RFIX/HTML/ | HTML-интерфейс (загружается через RFIX) |

---

## Новые C++ JSON-команды

### Этап 1: Команды для вкладки «Монитор» (7 команд)

#### 1. GetSelectionInfoCommand ✅
- **Назначение**: Количество и базовая информация о выделении
- **Использует**: `GetSelectedElements2()` из CommonFunction.hpp
- **Вход**: нет
- **Выход**: `{ count: number }`
- **Тип**: ReadOnlyCommand
- **Файлы**: json_commands/GetSelectionInfoCommand.hpp/.cpp
- **JS-обёртка**: BrowserPalette.cpp : ACAPI.GetSelectionInfo() ✅
- **HTML**: ACBridge.getSelectionInfo() → window.ACAPI.GetSelectionInfo() ✅
- **JSON тест**: test_08_get_selection_info в test_json_commands.py ✅
- **UI интеграция**: ✅ Исправлена через push-pattern (C++ → ExecuteJS → refreshSelectionInfoText)

#### 1.1. Исправление JS-моста для обновления UI ✅
- **Проблема**: Pull-pattern (return value из C++ в JS) не работал корректно
- **Решение**: Переход на push-pattern (как в Speckle) — C++ вызывает `browser.ExecuteJS()` для обновления UI
- **Изменения**:
  - Добавлен метод `UpdateSelectionInfoInUI()` в BrowserPalette
  - JS функция `refreshSelectionInfoText(count)` вызывается напрямую из C++
  - Упрощён JS код — больше не парсит return value
- **Статус**: ✅ Работает, "Выделено элементов: ERROR" исправлено

#### 2. GetPropertiesListCommand
- **Назначение**: Список свойств для текущего выделения, сгруппированный по системным группам, с фильтрацией
- **Использует**: `PropertyCache::property`, `PropertyCache::propertygroups`, `GetGroupFromCache()`, фильтрация по подстроке или regexp
- **Вход**: `{ filterText?: string }`
- **Выход**: `{ groups: [{groupName, properties: [{id, name, allHave, countWithProperty}]}] }`
- **Тип**: ReadOnlyCommand
- **Ключевая оптимизация**: `allHave` и `countWithProperty` считаются через `GetParamValueFromCache()` для каждого GUID из выделения **без повторных запросов к API** — кэш уже загружен

#### 3. GetPropertyValueCommand
- **Назначение**: Фактическое значение свойства для выделенных элементов
- **Использует**: `GetParamValueFromCache()` для каждого GUID
- **Вход**: `{ propertyId: string }`
- **Выход**: `{ propertyName: string, common: bool, values: [{value, count}] }`
- **Тип**: ReadOnlyCommand
- **Ключевая оптимизация**: Все значения уже в кэше — только сравнение и группировка

#### 4. GetClassificationCommand  
- **Назначение**: Дерево классификации для выделенных элементов
- **Использует**: `ClassificationFunc::ReadSystemDict()`, обход выделенных элементов, `API_ClassificationItem` / `ACAPI_Element_GetClassification`
- **Вход**: нет
- **Выход**: `{ common: bool, commonPath: string[], differing: [{elementName, value}], options: string[] }`
- **Тип**: ReadOnlyCommand

#### 5. SetClassificationCommand
- **Назначение**: Назначить единую классификацию всем выделенным
- **Использует**: `ACAPI_Element_SetClassification`
- **Вход**: `{ classificationValue: string }`
- **Выход**: `{ success: bool }`
- **Тип**: ModifyCommand (с undo)

#### 6. GetFilterPresetsCommand
- **Назначение**: Список преднастроенных фильтров свойств (из конфига или кэша)
- **Использует**: статические данные / конфиг аддона
- **Вход**: нет
- **Выход**: `{ presets: [{label, query}] }`
- **Тип**: ReadOnlyCommand

#### 7. ResetPropertyToDefaultCommand
- **Назначение**: Сбросить свойство к значению по умолчанию для всех выделенных
- **Использует**: `ACAPI_Property_SetPropertyValue` с `API_PropertyValue` из default
- **Вход**: `{ propertyId: string }`
- **Выход**: `{ success: bool }`
- **Тип**: ModifyCommand (с undo)

### Этап 2: JS-обёртки в BrowserPalette

Для каждой команды добавляется `DG::JSFunction` в `RegisterACAPIJavaScriptObject()`:

| JS-имя | Вызывает | Статус |
|--------|----------|--------|
| `ACAPI.GetSelectionInfo()` | `GetSelectedElements2()` | ✅ |
| `ACAPI.RefreshSelectionInfoUI()` | Push-pattern (ExecuteJS) | ✅ |
| `ACAPI.GetPropertiesList(filter)` | JSON-команду | ❌ |
| `ACAPI.GetPropertyValue(id)` | JSON-команду | ❌ |
| `ACAPI.GetClassification()` | JSON-команду | ❌ |
| `ACAPI.SetClassification(value)` | JSON-команду | ❌ |
| `ACAPI.GetFilterPresets()` | JSON-команду | ❌ |
| `ACAPI.ResetPropertyToDefault(id)` | JSON-команду | ❌ |

### Этап 3: Замена моков в HTML

В `Interface_ru.html` — замена `ACBridge.xxx()` на `window.ACAPI.xxx()`.

---

## Порядок реализации

```
[x] Этап 1.1 — GetSelectionInfoCommand ✅
[x] Этап 1.1.1 — Исправление JS-моста (push-pattern) ✅
[ ] Этап 1.2 — GetPropertiesListCommand (ключевая)
[ ] Этап 1.3 — GetPropertyValueCommand (ключевая)
[ ] Этап 1.4 — GetClassificationCommand + SetClassificationCommand
[ ] Этап 1.5 — GetFilterPresetsCommand
[ ] Этап 1.6 — ResetPropertyToDefaultCommand
[ ] Тестирование: test_json_commands.py + ArchiCAD
```

---

## Принципы

1. **Максимальное использование кэша** — все данные для вкладки «Монитор» (список свойств, значения для выделения) берутся из `PROPERTYCACHE()`, уже загруженного при старте аддона. Никаких дополнительных `ACAPI_Property_GetPropertyValue` для каждого элемента.
2. **Push-pattern для JS-моста** — C++ активно пушит данные в JS через `browser.ExecuteJS()`, не полагаясь на return value из `RegisterAsynchJSObject`. Архитектура как в проекте Speckle.
3. **Минимальные диффы** — новые файлы команд в `json_commands/`, только одна регистрация в `JsonCommandRegistrar.cpp`.
4. **Ручная проверка сборки** после каждой команды.
5. **TDD** — сначала тест (RED), потом реализация (GREEN).

---

## Lessons Learned

### ❌ Ошибки
1. **Попытки починить pull-pattern** — многократные попытки заставить работать return value из C++ в JS через `RegisterAsynchJSObject`. Потеря времени.
2. **Игнорирование паттерна Speckle** — Speckle использует push-pattern, но я продолжал пытаться починить pull.
3. **Отладка через console.log** — `console.log` в JS не попадает в `test_results.txt`. Правильно: только `DBprnt` из C++.

### ✅ Победы
1. **Переход на push-pattern** — простое и надёжное решение через `browser.ExecuteJS()`.
2. **Упрощение JS кода** — функция `refreshSelectionInfoText(count)` просто обновляет DOM.
3. **Правильная регистрация JS-объекта** — через `onLoadingStateChange` после загрузки страницы.

---

## Last Checkpoint: 686c839 feat: GetSelectionInfoCommand — первая JSON-команда для UI вкладки Монитор
## Next Step: Этап 1.2 — GetPropertiesListCommand (вторая команда вкладки «Монитор»)
## Scope: Sources/AddOn/json_commands/*, dialogs/BrowserPalette.cpp, AddOnResources/RFIX/HTML/Interface_ru.html