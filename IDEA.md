# План разработки: C++ команды для UI вкладки «Монитор»

## Цель

Реализовать инлайн C++ функции в JS-мосте (BrowserPalette.cpp), которые заменят моки `ACBridge` в HTML-интерфейсе вкладки «Монитор». Максимально использовать существующий оптимизированный код чтс
 **«Техническое задание: UI Панели Archicad»** (`Sources/AddOnResources/RFIX/HTML/ТЗ интерфейс.md`) — **обязательное ТЗ по интерфейсу**. Любые правки `Interface_ru.html` / `index.html` допускаются **только в соответствии с ТЗ**. Нарушение ТЗ без явного согласия пользователя недопустимо. При каждой правке HTML — **обязательно** запускать валидацию: `powershell -File Tools/test_html.ps1`.

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
| `RegisterACAPIJavaScriptObject()` | dialogs/BrowserPalette.cpp:100 | JS-мост в BrowserPalette |
| `Interface_ru.html` | AddOnResources/RFIX/HTML/ | HTML-интерфейс (загружается через RFIX) |

---

## Инлайн C++ функции в JS-мосте (BrowserPalette)

### Этап 1: Функции для вкладки «Монитор» (7 функций)

#### 1. GetSelectionInfo ✅
- **Назначение**: Количество и базовая информация о выделении
- **Использует**: `GetSelectedElements2()` из CommonFunction.hpp
- **Вход**: нет
- **Выход**: `{ count: number }`
- **Реализация**: Инлайн в `RegisterACAPIJavaScriptObject()` (BrowserPalette.cpp)
- **HTML**: `window.ACAPI.GetSelectionInfo()` ✅
- **UI интеграция**: ✅ Исправлена через push-pattern (C++ → ExecuteJS → refreshSelectionInfoText)

#### 1.1. Исправление JS-моста для обновления UI ✅
- **Проблема**: Pull-pattern (return value из C++ в JS) не работал корректно
- **Решение**: Переход на push-pattern (как в Speckle) — C++ вызывает `browser.ExecuteJS()` для обновления UI
- **Изменения**:
  - Добавлен метод `UpdateSelectionInfoInUI()` в BrowserPalette
  - JS функция `refreshSelectionInfoText(count)` вызывается напрямую из C++
  - Упрощён JS код — больше не парсит return value
- **Статус**: ✅ Работает, "Выделено элементов: ERROR" исправлено

#### 2. GetPropertiesList ✅
- **Назначение**: Список свойств для текущего выделения, сгруппированный по системным группам, с фильтрацией
- **Использует**: `PropertyCache::property`, `PropertyCache::propertygroups`, `GetGroupFromCache()`, фильтрация по подстроке или regexp
- **Вход**: `{ filterText?: string }`
- **Выход**: `{ groups: [{groupName, properties: [{id, name, allHave, countWithProperty}]}] }`
- **Реализация**: Инлайн в JS-мосте BrowserPalette.cpp
- **Ключевая оптимизация**: `allHave` и `countWithProperty` считаются через `GetParamValueFromCache()` для каждого GUID из выделения **без повторных запросов к API** — кэш уже загружен

#### 3. GetPropertyValue ✅
- **Назначение**: Фактическое значение свойства для выделенных элементов
- **Использует**: `GetParamValueFromCache()` для каждого GUID
- **Вход**: `{ propertyId: string }`
- **Выход**: `{ propertyName: string, common: bool, values: [{value, count}] }`
- **Реализация**: Инлайн в JS-мосте BrowserPalette.cpp
- **Ключевая оптимизация**: Все значения уже в кэше — только сравнение и группировка

#### 4. GetClassification ✅
- **Назначение**: Дерево классификации для выделенных элементов
- **Использует**: `ClassificationFunc::ReadSystemDict()`, обход выделенных элементов, `API_ClassificationItem` / `ACAPI_Element_GetClassification`
- **Вход**: нет
- **Выход**: `{ common: bool, commonPath: string[], differing: [{elementName, value}], options: string[] }`
- **Реализация**: Инлайн в JS-мосте BrowserPalette.cpp

#### 5. SetClassification ✅
- **Назначение**: Назначить единую классификацию всем выделенным
- **Использует**: `ACAPI_Element_SetClassification`
- **Вход**: `{ classificationValue: string }`
- **Выход**: `{ success: bool }`
- **Реализация**: Инлайн в JS-мосте BrowserPalette.cpp

#### 6. GetFilterPresets ❌
- **Назначение**: Список преднастроенных фильтров свойств (из конфига или кэша)
- **Вход**: нет
- **Выход**: `{ presets: [{label, query}] }`

#### 7. ResetPropertyToDefault ❌
- **Назначение**: Сбросить свойство к значению по умолчанию для всех выделенных
- **Вход**: `{ propertyId: string }`
- **Выход**: `{ success: bool }`

### Этап 2: JS-функции в BrowserPalette

Все функции реализованы инлайн в `RegisterACAPIJavaScriptObject()` в BrowserPalette.cpp:

| JS-имя | Реализация | Статус |
|--------|----------|--------|
| `ACAPI.GetSelectionInfo()` | Инлайн в BrowserPalette.cpp | ✅ |
| `ACAPI.RefreshSelectionInfoUI()` | Push-pattern (ExecuteJS) | ✅ |
| `ACAPI.GetPropertiesList(filter)` | Инлайн в BrowserPalette.cpp | ✅ |
| `ACAPI.GetPropertyValue(id)` | Инлайн в BrowserPalette.cpp | ✅ |
| `ACAPI.GetClassification()` | Инлайн в BrowserPalette.cpp | ✅ |
| `ACAPI.SetClassification(value)` | Инлайн в BrowserPalette.cpp | ✅ |
| `ACAPI.GetFilterPresets()` | Инлайн в BrowserPalette.cpp | ❌ |
| `ACAPI.ResetPropertyToDefault(id)` | Инлайн в BrowserPalette.cpp | ❌ |

### Этап 3: Замена моков в HTML

В `Interface_ru.html` — замена `ACBridge.xxx()` на `window.ACAPI.xxx()`.

---

## Порядок реализации

### Этап 1: Вкладка «Монитор» (ArchiCAD 25)

```
[x] Этап 1.1 — GetSelectionInfo ✅
[x] Этап 1.1.1 — Исправление JS-моста (push-pattern) ✅
[x] Этап 1.2 — GetPropertiesList ✅
    [x] Инлайн реализация в BrowserPalette.cpp
    [x] JS-мост в BrowserPalette
    [x] HTML: замена мока, группировка по группам, фильтрация
    [x] Сворачивание/разворачивание групп
[x] Этап 1.3 — GetPropertyValue ✅
    [x] Инлайн реализация в BrowserPalette.cpp
    [x] Использует ParamHelpers::GetParamValueFromCache для кэшированных значений
    [x] Возвращает {propertyName, common: bool, values: [{value, count}]}
[x] Этап 1.3.1 — HTML: показ значений свойств в списке ✅
    [x] Удален отдельный блок valueBlock
    [x] Значения отображаются под названием свойства мелким шрифтом
    [x] Уникальные значения с количеством (×N)
[x] Этап 1.4 — GetClassification + SetClassification ✅
    [x] Инлайн реализация в BrowserPalette.cpp
[x] Этап 1.5 — GetFilterPresets
[x] Этап 1.6 — ResetPropertyToDefault
```

### Этап 2: Вкладка «Синхронизация» (после Монитора)

```
[ ] Этап 2.1 — ExecuteSyncScriptCommand
    Назначение: Выполнить DSL скрипт над текущим выделением
    Вход: { scriptText: string }
    Выход: { success: bool, message: string, errorLine?: number, errorColumn?: number, errorText?: string }
    Тип: ModifyCommand (если скрипт меняет элементы) / ReadOnlyCommand (если только чтение)
    Реализация: парсинг DSL, выполнение над выделением, возврат результата
```

### Этап 3: Вкладка «Нумерация» (после Синхронизации)

```
[ ] Этап 3.1 — GetNumberingPropertiesCommand
    Назначение: Получить допустимые свойства для настроек нумерации
    Вход: нет
    Выход: { properties: [{id, name, groupName}] }

[ ] Этап 3.2 — GetNumberingPreviewCommand
    Назначение: Рассчитать предпросмотр нумерации
    Вход: { config: NumberingConfig }
    Выход: { preview: [{elementGuid, elementName, newValue}] }

[ ] Этап 3.3 — ExecuteNumberingCommand (архитектурный задел)
    Назначение: Реально применить нумерацию в ArchiCAD
    Вход: { config: NumberingConfig }
    Выход: { success: bool, changedCount: number }
    Тип: ModifyCommand (с undo)
    Примечание: В текущем HTML только предпросмотр, ExecuteNumbering не вызывается. Но закладываем в API.
```

---

## Принципы

1. **Максимальное использование кэша** — все данные для вкладки «Монитор» (список свойств, значения для выделения) берутся из `PROPERTYCACHE()`, уже загруженного при старте аддона. Никаких дополнительных `ACAPI_Property_GetPropertyValue` для каждого элемента.
2. **Push-pattern для JS-моста** — C++ активно пушит данные в JS через `browser.ExecuteJS()`, не полагаясь на return value из `RegisterAsynchJSObject`. Архитектура как в проекте Speckle.
3. **Инлайн реализация в BrowserPalette** — все JS-функции реализуются прямо в `RegisterACAPIJavaScriptObject()`, без промежуточных JSON команд.
4. **Ручная проверка сборки** после каждой команды.
5. **HTML правки строго по ТЗ** — `Sources/AddOnResources/RFIX/HTML/ТЗ интерфейс.md` является обязательным ТЗ. Любые изменения `Interface_ru.html` / `index.html` только в соответствии с ТЗ. При каждой правке HTML — **обязательно** валидация: `powershell -File Tools/test_html.ps1`.

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

## Last Checkpoint: da5b6a6 [fix-limit-setter-noargs] Replace SetMaxSelectionCount(args) with argument-less SetLimit<N> family
## Next Step: Чекпоинт cleanup-html-debug (DEBUG-блоки и console.* удалены из Interface_ru.html, runtime OK); затем Этап 1.5 GetFilterPresets
## Scope: Sources/AddOn/dialogs/BrowserPalette.cpp/.hpp, Sources/AddOnResources/RFIX/HTML/Interface_ru.html; незакоммиченный ReNum.cpp — вне задачи