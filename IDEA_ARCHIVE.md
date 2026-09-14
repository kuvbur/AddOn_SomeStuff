# IDEA Archive

Завершённые задачи и отменённые направления из `IDEA.md`. Новая запись — сверху (сразу под этим абзацем),
старые — ниже.

---

## Справочная таблица кэша «Монитора» (2026-09-14, перенесено из IDEA.md)

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

---

## UI классификации (2026-09-13, коммит ad7e8f5) — COMPLETED (код), runtime НЕ проверялся

`Interface_ru.html`: выбор классификации + «Применить ко всем» видны всегда (ранний `return` по
`data.common` прятал их); дедуп дублей `uniqueClassificationOptions`; dropdown как в Archicad —
дерево (▸/▾) + живой поиск с подсветкой совпадений. Валидация `Tools/test_html.ps1` PASSED; логика
проверена ad-hoc в node.

## Ревью 2026-09-12 — CLOSED

60 находок (11add7a + 13ba469), ревью 2026-09-12b 87 пунктов (2a48348), вторая проверка (072f975),
Sync.cpp-7 Name2Rawname (1625cf1). Остаток — issues #161/#162, runtime R2–R10 — #163–#171.

## Настройки в локальный файл (2026-09-12, коммит f0909f0) — COMPLETED

`…/GRAPHISOFT/SomeStuff/SyncSettings.dat` вместо prefs проекта (ломали Teamwork). Runtime AC25:
файл создан, ошибок нет; TW-проверка открыта (issue #169 / R8); сборки AC22–24/26–29 не проверены
(issue #170 / R9).

## Runtime AC25 (2026-09-12) — 741 ok, 0 ERROR IN TEST (TEST : start..end), AC25 Build succeeded
## Подсветка+зум по клику ×N, выделение сохраняется — Verified 2026-08-24 (Дмитрий, runtime AC25)

---

## Снято: JSON-команды и Этапы 2/3 старого плана (2026-09)

Идея JSON-команд (`API_AddOnCommand` / `JsonCommandRegistrar` / `AddOnCommandId`) **отменена** —
реализация удалена коммитом `b7a996b` (`[json-cleanup] Удаление JSON команд и тестов`). Вся UI-функциональность
делается инлайн-функциями JS-моста `BrowserPalette` (`RegisterACAPIJavaScriptObject`).

Соответственно не актуальны спецификации старого плана:

- Этап 2.1 `ExecuteSyncScriptCommand` — `{scriptText}` → `{success, message, errorLine?, errorColumn?, errorText?}`
- Этап 3.1 `GetNumberingPropertiesCommand` — → `{properties: [{id, name, groupName}]}`
- Этап 3.2 `GetNumberingPreviewCommand` — `{config: NumberingConfig}` → `{preview: [{elementGuid, elementName, newValue}]}`
- Этап 3.3 `ExecuteNumberingCommand` — `{config: NumberingConfig}` → `{success, changedCount}`

Оставлено только как исторический контекст. Если понадобится вкладка «Синхронизация»/«Нумерация» — делать
инлайн-функциями моста, а не `*Command`.

---

## Настройки аддона: локальный файл вместо prefs проекта (2026-09-12, коммит f0909f0)

### Task

Убрать записи настроек аддона в файл проекта (prefs проекта модифицируют БД → ломают Teamwork Send/Receive)
и хранить настройки машинно-локально.

### Status

COMPLETED (код). Открыто: runtime-проверка TW (в `IDEA.md` — пункт R8), LSP-проверка, сборки AC22–24/26–29.

### Диагноз 2026-09-09 (регрессия относительно v1.77, коммит 1ca70b6)

- `ReservationChangeHandler` (SomeStuff_Main.cpp:36–62), появившийся в `dab140a` (2026-07-10), модифицировал БД
  внутри TW-операции: `PROPERTYCACHE().Update()` + `DimRoundAll` → `ACAPI_CallUndoableCommand` +
  `ACAPI_Element_Change` (Dimensions.cpp:326–343).
- Док DevKit-25 (`APIReservationChangeHandlerProc.html`): «In the reservation change handler try to avoid
  calling functions that would modify the database».
- Фикс первой волны: хендлер → только `AttachObserver`; `PROPERTYCACHE().Update()` перенесён в
  `ProjectEventHandlerProc` (case `APINotify_ReceiveChanges`); `APINotify_ReceiveChanges` добавлен в обе маски
  `CatchProjectEvent`. Runtime-тест Дмитрия (2026-09-12): симптом тот же — «в файл продолжают дописываться
  настройки аддона».

### Диагноз-2 (2026-09-12): prefs аддона пишутся в план проекта

- Док DevKit-25 `Level3/Preferences_Save.html`: «The preferences data is also stored in all project files» →
  `ACAPI_SetPreferences` = модификация БД проекта. (LightRAG по вопросу ничего конкретного не дал, источник —
  локальная документация DevKit-25.)
- Места записи: `dialogs/BrowserPalette.cpp:114` (Show), `:127` (Hide), `:570` (JS-мост
  Enable/DisableCatchSelectionChanges); `SomeStuff_Main.cpp:197,204` (ElementEventHandlerProc, флип `logMon`),
  `:489` (Initialize — в 1.77 такой записи не было), `:437` (MenuCommandHandler — есть и в 1.77).
- Петля: `BrowserPalette::Show/Hide` вызываются из `PaletteControlCallBack` по
  `APIPalMsg_HidePalette_Begin/End` (BrowserPalette.cpp:1281/1286) → на каждое действие пользователя 2 записи
  prefs в файл проекта.
- Прочее: результат `ACAPI_SetPreferences` не проверялся (`SyncSettings.cpp:211-227`), кэш не инвалидировался
  при открытии проекта, `ACAPI_GetPreferences` вызывался без `ACAPI_GetPreferences_Platform`.

### Решение (SDK-проверено)

- Папка: `ACAPI_Environment (APIEnv_GetSpecFolderID, &id, &folder)` + `API_SpecFolderID`
  (APIdefs_Environment.h:1621-1634): `API_GraphisoftPrefsFolderID` / `API_ApplicationPrefsFolderID` —
  локальные per-user папки, `API_UserDocumentsFolderID` — fallback.
- Запись/чтение: `IO::File (loc, IO::File::OnNotFound::Create)` + `Open (WriteEmptyMode)` + `WriteBin` + `Close`
  (паттерн Tapir/Config.cpp, DeveloperTools.cpp); `IO::File` наследует `GS::IChannel`/`GS::OChannel` → блоб
  `SyncSettings::Write/Read` пишется в файл напрямую.
- Плюс: блоб становится машинно-локальным, кросс-платформенная проблема (`ACAPI_GetPreferences` без `_Platform`)
  исчезает.

### Реализация

- Шаг 1 (`dialogs/SyncSettings.cpp`): хелпер `GetSyncSettingsFolderLocation` (Graphisoft prefs → Application prefs
  → User documents + `IO::Folder::CreateFolder (IO::Name ("SomeStuff"))`); `ReadSyncSettingsFromFile`
  (заголовок magic 'SSS1' + PreferencesVersion + размер блоба проверяются ДО десериализации);
  `WriteSyncSettingsToPreferences` пишет локальный файл, повторная запись идентичного блоба пропускается
  (memcmp), `ACAPI_SetPreferences` из кода убран; миграция `ReadSyncSettingsFromLegacyPreferences` — одноразово
  при отсутствии/несовпадении файла; ошибки → `msg_rep`/`DBprnt`.
- Шаг 2: записи из `ElementEventHandlerProc` (`logMon` — только в памяти), `Initialize`, `BrowserPalette::Show/Hide`
  убраны; `Show(bool reloadContent)` из `APIPalMsg_HidePalette_End` вызывается как `Show(false)` +
  ручной `UpdateSelectionInfoInUI`; `showpalette` сохраняется только в `ShowOrHideBrowserPalette` и
  `PanelCloseRequested`. `MenuCommandHandler` и JS `Enable/DisableCatchSelectionChanges` оставлены (явные
  действия пользователя) — пишут в локальный файл.
- Принятые допущения: `API_GraphisoftPrefsFolderID` + подпапка `SomeStuff`; миграция из prefs проекта;
  `logMon` только в памяти; один общий `SyncSettings.dat` на все версии AC (имя меняется в одной строке
  `SyncSettingsFileName`).

### Validation

- `clang-format` выполнен; HTML-валидация `Tools/test_html.ps1` — PASSED; AC25 Build succeeded
- Runtime AC25: ArchiCAD стартовал, локальный файл `C:/Users/da-rogojin/AppData/Roaming/GRAPHISOFT/SomeStuff/SyncSettings.dat`
  создан (31 байт), `=== ERROR IN TEST ===` не найден; runner пометил C++-тесты как UNKNOWN (нет recognised status marker)
- **Не проверено**: TW Send/Receive несколькими пользователями (пункт R8 в `IDEA.md`); сборки AC22–24/26–29
  (в репозитории только DevKit-25, `IO::File`/`IO::Folder` в остальных версиях не верифицированы); LSP
  (clangd MCP не стартовал: `Clangd process failed to start`)

### Проверено при составлении плана

- В AC25 **нет** `ACAPI_ProjectSettings_GetSpecFolder` — только `ACAPI_Environment (APIEnv_GetSpecFolderID, …)`
  (grep по хедерам и списку доков).
- `IO::File`: `OnNotFound{Fail,Create,Ignore}` (File.hpp:117), ctor `(Location, OnNotFound)` :125,
  `Open(OpenMode)` :140, `WriteBin` :155, `GetDataLength` :176. Примеры: DevKit `LibPart_Test.cpp:942`,
  Tapir `Config.cpp`/`DeveloperTools.cpp`.

---

## Вкладка «Монитор»: инлайн-функции JS-моста (Этапы 1.1–1.4, 2026-08)

### Task

Реализовать инлайн C++ функции в JS-мосте (`RegisterACAPIJavaScriptObject`, BrowserPalette.cpp), заменив моки
`ACBridge` в HTML вкладки «Монитор», с максимальным использованием уже загруженного `PROPERTYCACHE()`.

### Status

COMPLETED (runtime подтверждён 2026-08-24, AC25). Этапы 1.5 (`GetFilterPresets`) и 1.6
(`ResetPropertyToDefault`) в этот блок не входят — они нужны и перенесены в активный план `IDEA.md`.

### Реализовано

| Функция | Вход → выход | Статус |
|---|---|---|
| `GetSelectionInfo` | → `{count}` | ✅ (push-pattern: `UpdateSelectionInfoInUI` → `ExecuteJS` → `refreshSelectionInfoText(count)`) |
| `GetPropertiesList` | `{filterText?}` → `{groups:[{groupName, properties:[{id,name,allHave,countWithProperty}]}]}` | ✅ |
| `GetPropertyValue` | `{propertyId}` → `{propertyName, common, values:[{value,count}]}` | ✅ зарегистрирована; прямых вызовов в `Interface_ru.html` не найдено (2026-09-12) |
| `GetClassification` | → `{common, commonPath[], differing[{elementName,value}], options[]}` | ✅ |
| `SetClassification` | `{classificationValue}` → `{success}` | ✅ (`ACAPI_Element_SetClassification` внутри `ACAPI_CallUndoableCommand`) |
| `GetFilterPresets` | → `{presets:[{label,query}]}` | ⏸ в коде нет — активный план `IDEA.md`, пункт 1.5 (в старом плане ошибочно стояло `[x]`) |
| `ResetPropertyToDefault` | `{propertyId}` → `{success}` | ⏸ в коде нет — активный план `IDEA.md`, пункт 1.6 (в старом плане ошибочно стояло `[x]`) |

Дополнительно зарегистрированы: `HighlightElements`, `ParsePropertyDescription`, `ParsePropertyForElement`
(два последних HTML не вызывает — мёртвый код, зарегистрированы после реворка `47b06e5`),
`EnableCatchSelectionChanges`/`DisableCatchSelectionChanges` (пара без аргументов).

HTML вызывает: `GetSelectionInfo`, `RefreshSelectionInfoUI`, `GetPropertiesList`, `GetClassification`,
`SetClassification`, `HighlightElements`, плюс динамически Enable/DisableCatchSelectionChanges.

### Принципы (сохранены)

1. Максимальное использование кэша — без `ACAPI_Property_GetPropertyValue` на каждый элемент.
2. Push-pattern для JS-моста (`browser.ExecuteJS()`), не полагаться на return value из `RegisterAsynchJSObject`.
3. Инлайн-реализация в `BrowserPalette`, без промежуточных JSON-команд.
4. Ручная проверка сборки после каждой команды.
5. HTML-правки строго по ТЗ + `Tools/test_html.ps1`.

### Lessons Learned

Ошибки: попытки починить pull-pattern (return value из C++ в JS через `RegisterAsynchJSObject`); игнорирование
паттерна Speckle (push-pattern); отладка через `console.log` — в JS он не попадает в `test_results.txt`,
диагностика только `DBprnt` из C++.

Победы: переход на push-pattern; упрощение JS (`refreshSelectionInfoText(count)` только обновляет DOM);
регистрация JS-объекта через `onLoadingStateChange` после загрузки страницы.

### Ревью моста (2026-08-24)

Корень серии крашей: `GS::DynamicCast<DG::JSArray>` на аргументе `JSFunction` ломает CEF-мост; каст к `JSValue`
безопасен (официальный пример DevKit `Browser_Control` парсит строковый аргумент именно через `JSValue`).

| # | Находка | Статус |
|---|---|---|
| R1 | `GetPropertyValue`: `DynamicCast<JSArray>` → латентный краш | ✅ исправлен (JSValue) |
| R2 | `SetClassification`: `DynamicCast<JSArray>` + HTML звал с массивом | ✅ исправлен (обе стороны) |
| R3 | `ParsePropertyDescription`/`ParsePropertyForElement`: тот же JSArray-парсинг, HTML их не вызывает | ⏸ мёртвый код (позже переписан в `47b06e5`) |
| R4 | Полнота моста (5 прямых + SetLimit*/Enable-Disable) | ✅ ок |
| R5 | Возвраты nullptr из лямбд | ✅ не обнаружены |
| R6 | `SetClassification` мутирует модель внутри `ACAPI_CallUndoableCommand` | ✅ ок |
| R7 | JSON-ответы экранируются `EscapeJsonString` | ✅ ок |

### Hotfix 2026-08-24 (коммит 32fc4a8): краш при отключении автообработки

`SetCatchSelectionChanges(args)` → пара `EnableCatchSelectionChanges`/`DisableCatchSelectionChanges` без
аргументов (паттерн `SetLimit<N>`); HTML: `toggleAutoRefresh` вызывает их без аргументов. Причина: аргументы в
`JSFunction` через `RegisterAsynchJSObject` роняли ArchiCAD (в `test_results.txt` записи лямбды не было).
Результат: 3 клика (Disable/Enable/Disable), все с «ok», краша нет.

### Прочее по UI (2026-08-24/25)

- `3fd665d` — подсветка и зум к элементам по клику на строку значения (Spec.cpp-паттерн:
  `APIIo_HighlightElementsID` + `APIDo_ZoomToElementsID`).
- `84664fe` — подсветка без выделения; `16f2b82` — `suppressSelectionRefresh`, чтобы палитра не сбрасывала
  пользовательское выделение. Детали — скилл `archicad-cef-pitfalls`.

---

## Test project name gate for `test_results.txt`

### Task

Проверять имя открытого файла Archicad и записывать `test_results.txt` только если имя содержит `test`.

### Status

COMPLETED

### Implementation

- Added `IsTestProjectOpen()` in `Sources/AddOn/CommonFunction.cpp`.
- Both `DBprnt` overloads now open `test_results.txt` only after checking `API_ProjectInfo::projectName`.
- AC27+ uses `ACAPI_ProjectOperation_Project(&projectInfo)`.
- AC22–26 uses `ACAPI_Environment(APIEnv_ProjectID, &projectInfo)`.
- The check is case-insensitive and skips unsaved projects, missing names, and API errors.

### Validation

- LightRAG confirmed the API signatures, `projectName` nullable pointer, and `API_ProjectInfo` ownership.
- `clang-format` and `git diff --check` passed.
- AC25 LSP compile database generated successfully.
- AC25 MSVC build succeeded.
- Runtime runner opened `Test_file/test_25.pln`, HTML validation passed, and `D:/SomeStuff_addon/test_results.txt` was created.
- The runner reported `UNKNOWN` for C++ test status because the generated file had no recognised status marker; it exited successfully.

### Scope

Only `Sources/AddOn/CommonFunction.cpp` and `IDEA.md` were changed. Existing uncommitted user changes were preserved.

### Checkpoint

No git commit created; the user did not request one.
