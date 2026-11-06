# JSON Commands Architecture — ArchiCAD Add-On SomeStuff

> Обновлено: 2026-11-06 — приведено к текущему набору команд (9 зарегистрированных), описаны параметры/ответы и сверка с исходниками; пользовательское описание команд — `wiki/ru/JSON-Commands-ru.md`.

## Текущая структура файлов

```
Sources/AddOn/json_commands/
├── CommandBase.hpp/cpp          # Базовый класс CommandBase (+ заготовки ReadOnly/Modify, CreateError/SuccessResponse)
├── JsonCommandRegistrar.hpp/cpp # Регистрация всех 9 команд в ArchiCAD
├── RoomBookCommand.hpp/cpp      # RoomBook + FavoritesList + Favorite_CreateOtd +
│                                #   Roombook_CheckFinishConfiguration +
│                                #   CreateClassificationItem + SetPropertyDescription
├── SpecCommand.hpp/cpp          # Запуск построения спецификаций (изменяет модель)
├── SyncAllCommand.hpp/cpp       # Запуск полной синхронизации (изменяет модель)
├── HealthCommand.hpp/cpp        # Health check + версия аддона + имя проекта
└── How JSON Commands work.md    # Этот файл
```

- Всё содержимое команд собрано под `#if defined(ServerMainVers_2500)` (AC25+; целевые версии AC25–29). На более старых версиях `RegisterJsonCommands()` — пустая функция.
- `#include "ACAPinc.h"` в `.cpp` обязан стоять ДО проверки `ServerMainVers_2500` — макрос версии задаёт именно этот заголовок; иначе `.cpp` компилируется в пустую единицу и ошибка вылезает только на линковке.
- Новые файлы в подкаталоге подхватываются сборкой автоматически (GLOB_RECURSE) — CMake править не нужно.

## Схема вызова

```
Внешний инструмент (python/агент)
    ↓ HTTP JSON (порт 19723+ конкретного запущенного экземпляра Archicad)
API.ExecuteAddOnCommand
    ↓
CommandBase::Execute  (главный поток)
    ↓
Roombook::RoomBook_Run / Spec::SpecAll / SyncAndMonAll / ACAPI_* (остальные команды)
    ↓
GS::ObjectState → addOnCommandResponse
```

## Архитектура команд

### Базовый класс (`CommandBase.hpp`)

```cpp
class CommandBase : public API_AddOnCommand {
    // GetNamespace()       -> "SomeStuffCommand"              (override final)
    // GetExecutionPolicy() -> ScheduleForExecutionOnMainThread (override final)
    // OnResponseValidationFailed(), GetSchemaDefinitions()   (override final)
    // GetInputParametersSchema() / GetResponseSchema()       (переопределяет команда)
    // Execute()                                              (переопределяет команда)
};
```

- Общий namespace всех команд — `SomeStuffCommand` (`CommandNamespace()` в `CommandBase.cpp`).
- Выполнение — **только главный поток**: политика задана в базе как `override final`, per-command выбор политики невозможен. Читающие команды (Health, FavoritesList, CheckFinishConfiguration) тоже выполняются на главном потоке.
- Заготовки `ReadOnlyCommand` / `ModifyCommand` в заголовке есть; `Roombook_CheckFinishConfiguration` наследует `ReadOnlyCommand`, остальные команды — `CommandBase` напрямую.
- Помощники ответов — свободные функции `CreateErrorResponse(errorCode, message)` / `CreateSuccessResponse()` (`CommandBase.cpp`); используются в командах с проверкой параметров (Favorite_CreateOtd, SetPropertyDescription и др.).

---

## Реализованные команды

Все имена (`GetName()`) и параметры сверены с исходниками; namespace общий — `SomeStuffCommand`.

| # | `commandName` | Класс | Меняет модель |
|---|---------------|-------|---------------|
| 1 | `FavoritesList` | `RoomBookGetFavoritesCommand` | нет |
| 2 | `RoomBook` | `RoomBookCommand` | да |
| 3 | `Favorite_CreateOtd` | `RoomBookCreateFavoriteCommand` | да |
| 4 | `Roombook_CheckFinishConfiguration` | `RoomBookCheckFinishConfigCommand` | нет |
| 5 | `Spec` | `SpecCommand` | да |
| 6 | `SyncAll` | `SyncAllCommand` | да |
| 7 | `Health` | `HealthCommand` | нет |
| 8 | `CreateClassificationItem` | `RoomBookCreateClassificationItemCommand` | да |
| 9 | `SetPropertyDescription` | `RoomBookSetPropertyDescriptionCommand` | да |

### 1. `RoomBook` — issue #205, отчёт #314

**Назначение:** запускает `Roombook::RoomBook_Run (&results, zones)` — построение/обновление ведомости отделки.

**Входные параметры** (`GetInputParametersSchema`):
- `zones` — необязательный непустой массив строк-GUID зон (36 символов с дефисами либо в фигурных скобках, `additionalProperties: false`). Без него — интерактивный отбор.
- Каждый GUID проверяется до запуска: форма (`RoomBook_IsGuidString`), существование (`ACAPI_Element_GetHeader`), тип (`API_ZoneID`), редактируемость/видимость (`ACAPI_Element_Filter` по `APIFilt_IsEditable|OnVisLayer|HasAccessRight|InMyWorkspace|IsVisibleByRenovation`). Ошибка — `CreateErrorResponse` с GUID зоны.
- Повторяющиеся GUID отбрасываются.

**Ответ:** `{"status":"returned","rooms":[...]}` — при заданном `zones` отчёт по результатам, сгруппированный по зонам и типам отделки:
- `rooms[].zone_guid`, `types[].TypeOtd`, `elements[]` с полями `otd_guid`, `base_guid`, `write_succeeded`, `was_created`, `is_valid`, `drawing_enabled`, `draw_error`, `length`/`length_available`, `area`/`area_available`, `material`, `bmaterial`, `structtype`.
- Дубликаты по зоне (sentinel-записи без элемента) в отчёт не попадают; полный состав подготовленных массивов — задача #314.

**Семантика:** `status="returned"` — факт возврата, не успех расчёта. Undo открывает сам `Roombook`. Выбор экземпляра Archicad — на вызывающей стороне (HTTP-порт процесса).

### 2. `Spec` — issue #206, non-interactive запуск

**Назначение:** `Spec::SpecAll (syncSettings, ruleNames?, &placementPoint, &runResult)` — построение спецификаций без диалогов.

**Входные параметры:**
- `placementPoint` `{x, y}` — **обязателен**; точка вставки элементов на плане.
- `ruleNames` — необязательный массив строк (≥1): имена правил; без него — все правила.
- `includeParameters` — необязательный bool (по умолчанию `false`): дамп свойств/GDL-параметров созданных/изменённых элементов.

**Ответ:** `status` (текст результата `GSErrCode`), `resultCode`, `elementsToCreate/Modify/Delete`, `elapsedSeconds`, `includeParameters`; всегда — счётчики этапов `create/grouping/gdl/deleteOld` (`attempted/succeeded/failed`), `hasPrimaryError`, `hasRecoveryError`, `hasUnconfirmedCreate`, `prepareFailureStage` (строка: `none|rulesNotFound|readParamsNotFound|writeParamsNotFound|placeParamsNotFound|tooManyErrorElements|emptyElementsList|nothingCreated|canceledByUser|existingElementsLocked|unknown`). При `includeParameters: true` — также `created`/`modified` (дампы: `guid`, `favoriteName`, `sourceElement[]`, `property[]`, `gdlParameter[]`), `deleted[]`, `rules[]` (created/modified/deleted по правилу), `messages[]`. Списки наполняются `AddList` (`ObjectState::Add` требует уникального имени поля); имена в дампах сортированы — форма ответа не зависит от порядка хеш-таблицы.

**Runtime:** endpoint вызывался на AC25; корректность созданной спецификации проверяется по содержательному ответу, не по `status`.

### 3. `SyncAll` — issue #213

**Назначение:** полная синхронизация:

```cpp
SyncSettings syncSettings;
LoadSyncSettingsFromPreferences (syncSettings, true);  // перечитать локальный JSON-конфиг
PROPERTYCACHE ().Update ();                            // кэш свойств питает SyncByType/SyncData
SyncAndMonAll (syncSettings);                          // Sync.cpp — та же функция, что у пункта меню SyncAll
```

**Входные параметры:** нет.

**Ответ:** `{"status":"returned","elapsedSeconds":...}`.

**Семантика:**
- `SyncAndMonAll` объявлена как `void`, поэтому доказуемы только факт возврата и длительность вызова. Успех синхронизации, доступность undo, корректность модели не заявляются.
- Пункт меню `SyncAll_CommandID` дополнительно вызывает `DimRoundAll`, пишет настройки и обновляет меню — команда этого не делает (решение автора, #213).
- Настройки перечитываются с `forceReload = true`: у внешнего вызова нет интерфейса флагов обхода. Состав обхода определяют `wallS/widoS/objS/cwallS`; флаг `syncAll` в логике не читается (только сериализация).
- Прогресс-окно и фазы `SyncAndMonAll` не подавляются.
- Изменение модели выполняет сама `SyncAndMonAll` внутри своего `ACAPI_CallUndoableCommand`.

**Runtime (AC25, test_25.pln):** dispatch и возврат подтверждены; корректность синхронизации — проверяется обычными средствами.

### 4. `Health` — зарегистрирован

**Назначение:** проверка доступности аддона и выбор экземпляра.

**Ответ:**
```json
{ "status": "ok", "version": "...", "addon": "SomeStuff", "projectName": "..." }
```
- Версия — `RSGetIndString (ID_ADDON_STRINGS = 32501, VersionId = 49)`.
- `projectName` — `ACAPI_ProjectOperation_Project` (AC27+) / `ACAPI_Environment (APIEnv_ProjectID)`; `"untitled"`, если проект не сохранён или недоступен.

### 5. `FavoritesList`

**Назначение:** возвращает словарь избранного Roombook (`Roombook::Favorite_GetDict`).

**Входные параметры:** нет. **Ответ:** `{"favorites":[{"name":...,"type":...}]}` — имя и числовой тип каждого избранного.

### 6. `Favorite_CreateOtd`

**Назначение:** создаёт избранное отделки — `Roombook::Favorite_CreateOtd (type, name, folder)`.

**Входные параметры:** `type` (число, код `TypeOtd`), `name`, `folder` — все обязательны; пустое `name` → ошибка.

**Ответ:** `CreateSuccessResponse()` либо `CreateErrorResponse(APIERR_GENERAL, ...)` при неудаче.

### 7. `Roombook_CheckFinishConfiguration`

**Назначение:** проверка классов отделки и свойств `Sync_GUID` — `Roombook::Roombook_CheckFinishConfiguration (errors)`.

**Входные параметры:** нет. **Ответ:** `{"succeeded":bool,"errors":"строка (по ошибке на строку)"}`.

### 8. `CreateClassificationItem` — issue #303

**Назначение:** создание элемента классификации через `ACAPI_Classification_CreateClassificationItem (item, systemGuid, parentGuid, APINULLGuid)`.

**Входные параметры:** `systemGuid`, `itemId`, `itemName`, `itemDescription` — обязательны; `parentItemGuid` — необязателен (пустой/опущенный → корневой элемент). Нечитаемый `systemGuid` → `APINULLGuid` → ошибка.

**Ответ:** `{"itemGuid": "..."}` — GUID созданного элемента.

### 9. `SetPropertyDescription`

**Назначение:** меняет описание определения свойства (`ACAPI_Property_ChangePropertyDefinition` внутри `ACAPI_CallUndoableCommand`). Свойство ищется в PropertyCache по `propertyGuid` или по полному имени `"Группа/Имя"` (`FindPropertyGuidInCache`, запасной `FindPropertyGuidByGroups`); перед записью определение перечитывается по GUID (ChangePropertyDefinition пишет структуру целиком — устаревшая копия из кэша затёрла бы чужие поля); после записи обновляется кэш (`UpdatePropertyDescriptionInCache` → `GetPropertyRuleFlag` + `ReadFileFromDefinition`).

**Входные параметры:** `description` — обязателен (пустая строка очищает описание); `propertyGuid` или `name` — один из них обязателен.

**Ответ:** `{"propertyGuid":..., "changed":bool, "description":<прочитано обратно из модели>}`; `changed: false` — описание уже совпадало.

---

## Регистрация команд (`JsonCommandRegistrar.cpp`)

Для каждой команды: `GS::NewOwned<XxxCommand>()` + установка хендлера; на AC27+ — `ACAPI_AddOnAddOnCommunication_InstallAddOnCommandHandler`, на AC25/26 — `ACAPI_Install_AddOnCommandHandler`. Ошибка — `DBprnt` с `GS::ValueToUniString(err)`, не фатальна для старта аддона. Зарегистрированы (порядок кода): `RoomBookGetFavoritesCommand`, `RoomBookCommand`, `RoomBookCreateFavoriteCommand`, `RoomBookCheckFinishConfigCommand`, `SpecCommand`, `SyncAllCommand`, `HealthCommand`, `RoomBookCreateClassificationItemCommand`, `RoomBookSetPropertyDescriptionCommand`.

- Регистрация вызывается из `SomeStuff_Main.cpp`, `Initialize()`:
```cpp
// Регистрация JSON команд
RegisterJsonCommands ();
```
- Несколько команд в одном аддоне — штатно (примеры Tapir/Dotbim в cpprag): каждый вызов добавляет одну команду, диспетчеризация — по `commandName`.
- "registered successfully"-логи не пишутся.
- Шаблон добавления новой команды: класс в подходящем `.hpp/cpp` (или новом файле — подхватывается GLOB_RECURSE), `GetName()`, схемы параметров/ответа, include и блок регистрации; `CommonSchema::NotUsed` — если команда не использует общий контракт схем.

---

## Вызов из Python

Формат вызова — тот же, что у используемых в `Tools/` скриптов
(`spec_scenarios.py`, `spec_baseline.py`, `spec_benchmark.py`). Правильная
структура payload для ArchiCAD 25 JSON API:

```python
payload = {
    "command": "API.ExecuteAddOnCommand",
    "parameters": {
        "addOnCommandId": {
            "commandNamespace": "SomeStuffCommand",  # CommandBase::GetNamespace()
            "commandName": "Spec"                    # Command::GetName() — см. таблицу выше
        },
        "addOnCommandParameters": {}
    }
}
```

**Парсинг ответа:**
```python
result = body.get("result", {})
response_data = result.get("addOnCommandResponse", {})
# Данные команды находятся в response_data
```
Ошибки команды приходят в `addOnCommandResponse` как `{"errorCode":..., "error":...}` (помощник `CreateErrorResponse`).

- Порт определяется автообнаружением в python (скан 19723–19743), не передаётся снаружи.
- Команды выполнять после инициализации аддона и открытия проекта; runner (`restart_archicad_for_test.ps1`) ждёт 15 секунд после запуска Archicad.

---

## Важные правила

1. **Поток выполнения:** все команды `SomeStuffCommand` выполняются в главном потоке Archicad (`ScheduleForExecutionOnMainThread` в `CommandBase`, `override final`) — они читают/изменяют БД. Параллельного режима нет.
2. **Undo:** undo-скоуп открывает обёрнутая бизнес-логика (`Roombook`, `Spec`, `SyncAndMonAll`, `SetPropertyDescription` — внутри `ACAPI_CallUndoableCommand`). Команда не создаёт и не дублирует undo.
3. **Контракт ответа:** `status="returned"` + `elapsedSeconds` = факт возврата и длительность, не успех. Команды с проверяемым сигналом возвращают содержательные поля (`Spec` — счётчики этапов, `RoomBook` — отчёт по элементам, `SetPropertyDescription` — read-back).
4. **Порядок include:** `ACAPinc.h` до `#if defined(ServerMainVers_2500)` в каждом `.cpp`/`.hpp` команды.
5. **Ошибки:** `DBprnt` с `GS::ValueToUniString(err)` при регистрации; ошибки выполнения — `CreateErrorResponse(errorCode, message)`.
6. **Кодировка строк:** `GS::UniString` везде; в C-строку — `.ToCStr().Get()`; в ответ — `response.Add("key", ...)`, повторяющиеся поля — только через `AddList`.
7. **Схемы:** `GetInputParametersSchema` описывает форму до SDK-конвертации (нестрогие команды возвращают `GS::NoValue`); обязательность полей дополнительно проверяется в `Execute`.

---

## LightRAG / DevKit проверки (AC25)

| Символ/контракт | Статус |
|-----------------|--------|
| `ACAPI_Install_AddOnCommandHandler(GS::Owner<API_AddOnCommand>)` | ✅ verified — LightRAG + ACAPinc.h DevKit-25 |
| `ACAPI_AddOnAddOnCommunication_InstallAddOnCommandHandler` (AC27+) | ✅ verified — используется в регистраторе |
| `API_AddOnCommand::Execute(const GS::ObjectState&, GS::ProcessControl&) -> GS::ObjectState` | ✅ verified — LightRAG |
| `ScheduleForExecutionOnMainThread` для команд, трогающих БД | ✅ verified — LightRAG |
| Несколько команд на аддон (примеры Tapir/Dotbim) | ✅ verified — примеры cpprag |
| `ACAPI_CallUndoableCommand` | ✅ verified — AC25 (Roombook/Spec/Sync/SetPropertyDescription) |
| `ACAPI_Classification_CreateClassificationItem` | ✅ verified — AC25 (CreateClassificationItem, #303) |
| `ACAPI_Property_ChangePropertyDefinition` | ✅ verified — AC25 (SetPropertyDescription) |
| `RSGetIndString`, `ACAPI_ProjectOperation_Project` / `APIEnv_ProjectID` | ✅ verified — AC25 |

---

## Следующие шаги

1. ✅ `RoomBook` / `Spec` / `SyncAll` / `Health` — реализованы, зарегистрированы
2. ✅ `FavoritesList`, `Favorite_CreateOtd`, `Roombook_CheckFinishConfiguration`, `CreateClassificationItem` (#303), `SetPropertyDescription` — реализованы и зарегистрированы
3. ⏳ Отчёт `RoomBook` по всем подготовленным записям (#314) — приёмка по состоянию задачи
4. ⏳ ParseProperty / Renum-команды, MCP-сервер — отдельные задачи
