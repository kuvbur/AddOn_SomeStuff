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

### Этап 1.7 — UI-бэклог вкладки «Монитор» (после подсветки, подтверждена 2026-08-24)

```
[ ] Этап 1.7.1 — Кэш правил Somestuff + фильтр
    Назначение: кэшировать распарсенные правила (Sync/Spec) и фильтровать список свойств по наличию правила
    Вход: нет (правила читаются из описаний свойств через кэш)
    Выход: мост-данные для UI + ускорение повторных рендеров
    Метрика до: замерить время GetPropertiesList на выделении из 10 элементов (DBprnt, clock())
[ ] Этап 1.7.2 — Цветовая маркировка неактивных флагов
    Назначение: в списке свойств визуально отличать выключенные флаги (flagfindspec=false)
    Реализация: HTML+CSS по данным моста (без новых вызовов API)
[ ] Этап 1.7.3 — Pin свойств
    Назначение: закреплять свойства сверху списка при смене выделения
    Состояние: STATE.pinnedProperties (JS), порядок сортировки в renderPropertyList
```

Для каждого пункта: тесты до (GREEN-регрессии текущего поведения) → реализация → тесты после → замер времени (clock() вокруг мост-функций, DBprnt в test_results.txt) → оптимизация если деградация >10%.

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

## Last Checkpoint: bbbe71a checkpoint: до правок по ревью 2026-09-12b (GLM-instructions)
## Next Step: Задача «ревью 2026-09-12b»: ревью применённых правок завершено (3 субагента), исправлены 4 дефекта (ElemHead_To_Neig typeID, восстановлен MenuSetState, доделан п.55 store-сброс, GetMarkerPos порядок guard'ов); 4 субагента реализуют остаток (Roombook suspend/PERF, BrowserPalette 25/26/37-70, Dimensions/Helpers/CommonFunction minor+perf, прочие minor/perf). Затем: clang-format → LSP → сборка AC25 → отчёт. Отдельная TW/prefs-задача ниже — НЕ смешивать.
## Scope: Sources/AddOn/dialogs/BrowserPalette.cpp, Sources/AddOnResources/RFIX/HTML/Interface_ru.html, при необходимости Sync.cpp/Spec.cpp
## Verified 2026-08-24: подсветка+зум по клику ×N работает, выделение сохраняется (подтверждено Дмитрием, runtime AC25)

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

## Задача 2026-09-09: Teamwork «не получается получить изменения» (ДИАГНОЗ, фикс не применён)

Регрессия относительно v1.77 (1ca70b6, 2025-10-02). Корень: `ReservationChangeHandler` (SomeStuff_Main.cpp:36–62) с коммита **dab140a (2026-07-10)** модифицирует БД внутри TW-операции:
- `PROPERTYCACHE ().Update ()` — тяжёлый полный рид кэша на КАЖДОЕ изменение резервирований;
- `DimRoundAll (syncSettings, false)` → `ACAPI_CallUndoableCommand ("Change dimension text")` + `ACAPI_Element_Change` (Dimensions.cpp:326–343) — ЗАПИСЬ БД.

Док DevKit-25 (APIReservationChangeHandlerProc.html): **«In the reservation change handler try to avoid calling functions that would modify the database.»** Хендлер вызывается синхронно внутри Receive/Send/Reserve → запись БД в середине receive-транзакции → receive падает/зависает. В 1.77 хендлер был чистым: только `AttachObserver` (проверено по git show 1ca70b6).

Проверено и отклонено: Get/SetPreferences в хендлерах (LoadSyncSettingsFromPreferences читает статический кэш, forceReload в TW-хендлерах не передаётся; WriteSyncSettingsToPreferences в ElementEventHandlerProc был и в 1.77); AddOnObject/ModulData не используются вообще; маска CatchProjectEvent не включает APINotify_SendChanges/ReceiveChanges (вероятно, Update() в reservation-хендлер добавили как «рефреш после receive» — не то место).

- [x] Диагноз (git-история + док DevKit-25 + исходники; LightRAG недоступен — пустые ответы, fallback на DevKit)
- [x] Фикс применён (SomeStuff_Main.cpp): ReservationChangeHandler → только AttachObserver (убраны PROPERTYCACHE().Update(), DimRoundAll, ACAPI_KeepInMemory); добавлен case APINotify_ReceiveChanges → PROPERTYCACHE().Update() в ProjectEventHandlerProc; APINotify_ReceiveChanges добавлен в обе маски CatchProjectEvent (2700+ и старые). clang-format выполнен.
- [x] Сборка AC25: Build succeeded (Debug SomeStuff.apx)
- [ ] Runtime-тест в TW с включённым мониторингом: Send+Receive несколько циклов
- [x] 2026-09-12: Дмитрий протестировал — симптом тот же; наблюдение: «в файл продолжают дописываться настройки аддона», проблемы синхронизации с другими машинами
- [x] 2026-09-12: ДИАГНОЗ-2 (настройки). Факт из SDK-дока DevKit-25 `Level3/Preferences_Save.html`: «The preferences data is also stored in all project files» → `ACAPI_SetPreferences` пишет блоб аддона В ПЛАН, т.е. каждая запись = модификация БД проекта. LightRAG по этому вопросу ничего конкретного не дал (hybrid/local — общие описания), источник = локальная документация DevKit-25.
- [x] 2026-09-12: Найденные места записи prefs (Sources/AddOn): `dialogs/BrowserPalette.cpp:114` (Show), `:127` (Hide), `:570` (JS-мост Enable/DisableCatchSelectionChanges); `SomeStuff_Main.cpp:197,204` (ElementEventHandlerProc, флип-флаг logMon), `:489` (Initialize — в 1.77 такой записи НЕ было), `:437` (MenuCommandHandler — есть и в 1.77).
- [x] 2026-09-12: Ключевая петля: `BrowserPalette::Show/Hide` дергаются из `PaletteControlCallBack` по `APIPalMsg_HidePalette_Begin/End` (BrowserPalette.cpp:1281/1286) — ArchiCAD присылает их при каждом входе в ввод/модальную операцию, значит на каждое действие пользователя идёт 2 записи prefs в файл проекта (DevKit-пример в этих же сообщениях делает только показ/скрытие UI).
- [x] 2026-09-12: Устойчивость ошибок: `SyncSettings.cpp:211-227` — при сбое `ACAPI_SetPreferences` кэш не обновляется, результат `bool` не проверяется ни в одном месте → запись повторяется на каждом следующем событии.
- [x] 2026-09-12: Кэш настроек `SyncSettings.cpp:194-203` не инвалидируется при открытии проекта (док: prefs из плана перекрывают app-prefs) → observer/палитра пишут в файл stale-копию целиком. Платформа: `ACAPI_GetPreferences` без `ACAPI_GetPreferences_Platform`.
- [x] 2026-09-12: Проверка PropertyCache (`Propertycache.hpp/.cpp`): потоковых гонок нет — все вызовы в главном потоке (допущение, не проверено); реентерабельность `Update()` из хендлеров (SomeStuff_Main.cpp:88,94,351) + указатели из `GetPtr` через очистку таблиц; `isCacheContainsParamValue` (Propertycache.cpp:286-315) — false-negative при частично прочитанном `glob`; `ReadLibraryFile` (Propertycache.cpp:43-47) — мёртвая проверка nullptr после `new[]`, необработанный `std::bad_alloc`.
- [ ] СОГЛАСОВАТЬ с Дмитрием минимальный фикс: убрать записи prefs из автоматических путей (Show/Hide палитры, observer, Initialize), затем AC25-сборка + runtime TW-тест
- [x] 2026-09-12: НАЙДЕНО решение «настройки локально на машине, без записи в файл проекта» (SDK-проверено):
      - путь: `ACAPI_Environment (APIEnv_GetSpecFolderID, &id, &folder)` + `API_SpecFolderID` (APIdefs_Environment.h:1621-1634). Документация `Structures/API_SpecFolderID.html`: `API_ApplicationPrefsFolderID` — «the folder into which the application writes its preferences», `API_GraphisoftPrefsFolderID` — «general preferences of GRAPHISOFT applications» (обе per-user/локальные); `API_UserDocumentsFolderID` — папка вывода по умолчанию (fallback).
      - запись/чтение файла: `IO::File` — `IO::File (loc, IO::File::OnNotFound::Create)` + `Open (IO::File::OpenMode::WriteEmptyMode)` + `WriteBin` + `Close` (пример Tapir/Config.cpp, DeveloperTools.cpp через LightRAG); enum режимов `Modules/InputOutput/File.hpp`: ReadMode/WriteMode/WriteEmptyMode/ReadWriteMode/AppendMode. `IO::File` наследует `GS::IChannel`/`GS::OChannel` → блоб `SyncSettings::Write/Read` можно писать напрямую в файл.
      - плюс: blob становится машинно-локальным, кросс-платформенная проблема (ACAPI_GetPreferences без _Platform) исчезает.
      - миграция: при отсутствии локального файла один раз прочитать старые значения через `ACAPI_GetPreferences` и больше в проект не писать.
- [ ] СЛЕДУЮЩИЙ ШАГ: согласовать реализацию (локальный файл в Graphisoft/App prefs folder + убрать автоматические записи prefs) → clang-format → сборка AC25 → TW-тест

## План изменений (2026-09-12, сверено через LightRAG + хедеры DevKit-25)

### Шаг 1. Локальное хранилище настроек — `dialogs/SyncSettings.cpp`
1.1 Хелпер пути (static):
    `API_SpecFolderID id = API_GraphisoftPrefsFolderID;` → `ACAPI_Environment (APIEnv_GetSpecFolderID, &id, &loc)`; при ошибке fallback: `API_ApplicationPrefsFolderID` → `API_UserDocumentsFolderID`.
    `IO::Folder folder (loc); err = folder.CreateFolder (IO::Name ("SomeStuff"));` — `IO::Folder::TargetExists` считать успехом (Folder.hpp:125); затем `loc.AppendToLocal (IO::Name ("SomeStuff"))` и `loc.AppendToLocal (IO::Name ("SyncSettings.dat"))`.
1.2 `ReadSyncSettings (SyncSettings&)` — читать локальный файл:
    `IO::File file (loc, IO::File::OnNotFound::Fail);` (File.hpp:125) → `file.Open (IO::File::ReadMode)` → `file.GetDataLength (&size)` (File.hpp:176) → буфер → `file.ReadBin (buf, size)` → `file.Close()` → `MemoryIChannel ic (buf, size)` → `syncSettings.Read (ic)`. Перед десериализацией проверять заголовок (magic + `PreferencesVersion` + размер).
    `ACAPI_GetPreferences` (ACAPinc.h:325) остаётся ТОЛЬКО для одноразовой миграции.
1.3 `WriteSyncSettingsToPreferences` (SyncSettings.cpp:211-227) — писать локальный файл, а не проект:
    `MemoryOChannel oc; syncSettings.Write (oc);` → `IO::File file (loc, IO::File::OnNotFound::Create); file.Open (IO::File::WriteEmptyMode); file.WriteBin (data, size); file.Close();` (File.hpp:140/155, пример LibPart_Test:942). Убрать `ACAPI_SetPreferences` (ACAPinc.h:323) из кода полностью.
    Не писать, если blob не изменился с прошлой записи (in-memory сравнение) — снимает лишний I/O из observer-путей.
1.4 Миграция: локального файла нет → один раз прочитать старые значения по образцу DevKit `Database_Control::InitPreferences` (проверка version/size, при необходимости `ACAPI_GetPreferences_Platform`, ACAPinc.h:327) → сразу записать локальный файл → prefs больше не трогать.
1.5 Проверять результат записи/чтения во всех местах (сейчас `bool` игнорируется везде) и логировать ошибку в test_results.txt; кэш настроек обновлять только при успешной записи.

### Шаг 2. Убрать автоматические записи (причина «дописывания в файл»)
2.1 `SomeStuff_Main.cpp:197,204` (ElementEventHandlerProc, флип `logMon`) — не persist (решение: только in-memory).
2.2 `SomeStuff_Main.cpp:489` (Initialize) — убрать безусловную запись; писать только при миграции/смене версии.
2.3 `BrowserPalette.cpp:114,127` (Show/Hide из `APIPalMsg_HidePalette_Begin/End`, см. `:1281/1286`) — убрать запись; `showpalette` сохранять только по явной команде меню.
2.4 `SomeStuff_Main.cpp:437` (MenuCommandHandler) и `BrowserPalette.cpp:570` (JS Enable/DisableCatchSelectionChanges) — оставить: это явные действия пользователя, но писать теперь в локальный файл.

### Шаг 3. Валидация
- clang-format → LSP (SyncSettings.cpp / SomeStuff_Main.cpp / BrowserPalette.cpp) → сборка AC25 (`python Tools\BuildAddOn.py -c config.json -v 25`) → runtime `restart_archicad_for_test.ps1`; вывести в лог путь к файлу настроек (`loc.ToDisplayText()`).
- Проверки: настройки выживают перезапуск AC; в TW при работе с палитрой больше нет локальных изменений/запросов отправки; Receive/Send у нескольких пользователей.
- Негативные: нет прав на запись в папку (fallback), пустой/битый файл настроек (заголовок), файл от другой версии (version mismatch → дефолты).

### Открытые вопросы (нужно решение Дмитрия)
1. Папка: `API_GraphisoftPrefsFolderID` (общая для продуктов GRAPHISOFT, переживает смену версии AC) или `API_ApplicationPrefsFolderID` (в пределах версии AC). Рекомендация: GraphisoftPrefs + подпапка `SomeStuff`.
2. Старые проекты: делать одноразовую миграцию значений из prefs или начать с дефолтов?
3. `logMon`: только в памяти (рекомендация) или все же сохранять в локальный файл?
4. Файл на пользователя/машину — per-AC-версия (`-25`) или один общий для всех версий?

### Проверено при составлении плана
- В AC25 **нет** `ACAPI_ProjectSettings_GetSpecFolder` (встречается только в более новых SDK/Speckle); доступен только `ACAPI_Environment (APIEnv_GetSpecFolderID, …)` — проверено grep по хедерам и по списку доков.
- `IO::File`: `OnNotFound{Fail,Create,Ignore}` (File.hpp:117), ctor `(Location, OnNotFound)` :125, `Open(OpenMode)` :140, `WriteBin` :155, `GetDataLength` :176; класс наследует `GS::IChannel`/`GS::OChannel` → блоб можно лить каналом.
- `IO::Folder::CreateFolder (const Name&, …)` :148, ошибка `TargetExists` :125.
- Применение в DevKit: `LibPart_Test.cpp:942` (`IO::File (folder, IO::Name (file), IO::File::OnNotFound::Create)`), Tapir `Config.cpp`/`DeveloperTools.cpp` (OnNotFound::Create + WriteEmptyMode + WriteBin).