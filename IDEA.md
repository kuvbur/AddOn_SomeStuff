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

## Last Checkpoint: 4ca38e0 [fix-tosub-expansion] Fix dead to_sub branch in SyncAddSubelement + TDD tests (438 ok / 0 err)
## Next Step: Ревью-хвосты по схеме TDD: P3 (мёртвые ParseProperty* — чинить JSValue-парсингом или удалить), P2 (восстановление групп после SuspendGroups — сначала верифицировать API). Пожелание из бэклога: подсветка+приближение элемента при клике на строку значения свойства (×1 ×2 …)
## Scope: Sources/AddOn/dialogs/BrowserPalette.cpp, Sources/AddOnResources/RFIX/HTML/Interface_ru.html, Sources/AddOn/Sync.cpp; ReNum.cpp зафиксирован в e44b30a

## Задача 2026-08-24 №2: фикс SetClassification/GetPropertyValue + ревью моста (DONE)

Факт: официальный пример DevKit AC25 (Examples/Browser_Control/Src/BrowserPalette.cpp:110) использует JSFunction СО строковым аргументом — теория «аргументы всегда крашат» не доказана. Проверяем эмпирически.

- [x] Шаг 1: диагностические зонды в мост (v1, v2) + авто-вызов из HTML при старте
- [x] Шаг 2: runtime-прогон → **корень найден**: аргументы приходят нормально (как одиночный JSValue); крашит именно `GS::DynamicCast<DG::JSArray>` на аргументе (зонд A с кастом к JSValue прошёл, зонд B с кастом к JSArray упал внутри лямбды)
- [x] Шаг 3: фикс по результатам — обычные сигнатуры со строкой, парсинг через DynamicCast<JSValue>
- [x] Шаг 4: зонды удалены; GetPropertyValue/SetClassification переписаны на JSValue-парсинг; HTML: SetClassification(строка) без [обёртки]
- [x] Шаг 5: рантайм-тест пользователя пройден (проверка палитры — ошибок нет) → чекпоинт 1241122 → память/скилл обновлены

## Результаты ревью моста (2026-08-24)

Корень всех крашей серии: `GS::DynamicCast<DG::JSArray>` на аргументе JSFunction ломает CEF-мост. Каст к `JSValue` безопасен (зонд A). Официальный пример DevKit Browser_Control парсит строковый аргумент именно через JSValue.

| # | Находка | Статус |
|---|---|---|
| R1 | GetPropertyValue: DynamicCast<JSArray> → латентный краш | ✅ исправлен (JSValue) |
| R2 | SetClassification: DynamicCast<JSArray> + HTML звал с [массивом] | ✅ исправлен (обе стороны) |
| R3 | ParsePropertyDescription/ParsePropertyForElement: тот же JSArray-парсинг, НО HTML их не вызывает — мёртвый код | ⏸ не тронут (вне задачи; удалить или починить отдельной задачей) |
| R4 | Полнота моста: все вызовы HTML зарегистрированы (5 прямых + SetLimit*/Enable-Disable динамические) | ✅ ок |
| R5 | Возвраты nullptr из лямбд | ✅ не обнаружены |
| R6 | SetClassification мутирует модель внутри ACAPI_CallUndoableCommand | ✅ ок |
| R7 | JSON-ответы экранируются EscapeJsonString | ✅ ок |

## Hotfix 2026-08-24: краш при нажатии кнопки отключения автообработки (DONE, коммит 32fc4a8)

- [x] BrowserPalette.cpp: SetCatchSelectionChanges(args) → пара EnableCatchSelectionChanges / DisableCatchSelectionChanges без аргументов (паттерн SetLimit<N>)
- [x] Interface_ru.html: toggleAutoRefresh вызывает window.ACAPI.Enable/DisableCatchSelectionChanges() без аргументов
- [x] Валидация: Tools/test_html.ps1 → сборка AC25 → runtime restart_archicad_for_test.ps1
- Причина: передача аргументов в JSFunction через RegisterAsynchJSObject роняет ArchiCAD до входа в лямбду (тот же паттерн, что 2026-08-21 с SetMaxSelectionCount; лог test_results.txt не содержал записи SetCatchSelectionChanges — лямбда не выполнялась)
- Результат: runtime-тест 2026-08-24 пройден — 3 клика (Disable/Enable/Disable), все с «ok» в test_results.txt, краша нет