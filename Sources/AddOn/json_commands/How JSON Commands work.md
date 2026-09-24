# JSON Commands Architecture — ArchiCAD Add-On SomeStuff

> Обновлено: 2026-09-24 — после восстановления инфраструктуры JSON-команд (#205) и добавления команды Spec (#206).

## Текущая структура файлов

```
Sources/AddOn/json_commands/
├── CommandBase.hpp/cpp          # Базовый класс CommandBase (+ заготовки ReadOnly/Modify)
├── JsonCommandRegistrar.hpp/cpp # Регистрация команд в ArchiCAD (RoomBook, Spec)
├── RoomBookCommand.hpp/cpp      # Запуск построения отделки (изменяет модель)
├── SpecCommand.hpp/cpp          # Запуск построения спецификаций (изменяет модель)
├── HealthCommand.hpp/cpp        # Health check + версия аддона (в дереве, НЕ регистрируется)
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
Roombook::RoomBook()  /  Spec::SpecAll(syncSettings)
    ↓
GS::ObjectState → {"status":"returned","elapsedSeconds":...}
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
- Выполнение — **только главный поток**: политика задана в базе как `override final`, per-command выбор политики невозможен. Читающие команды (Health) тоже выполняются на главном потоке.
- Заготовки `ReadOnlyCommand` / `ModifyCommand` в заголовке есть, но текущие команды наследуют `CommandBase` напрямую.
- Помощники ответов — свободные функции `CreateErrorResponse(errorCode, message)` / `CreateSuccessResponse()` (`CommandBase.cpp`); текущие команды их не используют.

---

## Реализованные команды

### 1. `RoomBook` (CommandBase) — issue #205

**Назначение:** запускает `Roombook::RoomBook()` — построение/обновление ведомости отделки.

**Входные параметры:** нет.

**Ответ:**
```json
{ "status": "returned", "elapsedSeconds": 15.0850313 }
```

**Семантика ответа:**
- `status="returned"` — только факт возврата из функции, **не** успех расчёта и **не** корректность модели.
- `elapsedSeconds` — длительность вызова `RoomBook()` (steady_clock).
- Изменение модели (создание/удаление элементов отделки) выполняет сам `Roombook` внутри собственного undo-скоупа; команда undo отдельно не открывает.
- Выбор экземпляра Archicad — на вызывающей стороне: HTTP-порт соответствует конкретному запущенному процессу.

**Runtime (AC25, test_25.pln, порт 19723):** endpoint вернул `{"status":"returned","elapsedSeconds":15.0850313}` — dispatch и возврат подтверждены; корректность созданной отделки и отмена — not verified.

---

### 2. `Spec` (CommandBase) — issue #206

**Назначение:** запускает построение спецификаций:

```cpp
SyncSettings syncSettings;
LoadSyncSettingsFromPreferences (syncSettings);   // dialogs/SyncSettings.hpp — локальный JSON-конфиг (#190)
Spec::SpecAll (syncSettings);                     // spec/Spec.cpp — единая точка входа меню Spec
```

**Входные параметры:** нет. Источник элементов `SpecAll` определяет сам: выделение → правила элемента по умолчанию → все видимые элементы.

**Ответ:** как у RoomBook:
```json
{ "status": "returned", "elapsedSeconds": 42.1 }
```

**Семантика:** `status="returned"` = только факт возврата; `SpecAll` возвращает `GSErrCode`, но команда его наружу не пробрасывает. Создание/обновление/удаление элементов спецификации выполняет сам `SpecAll` (`ACAPI_Element_Create` внутри `ACAPI_CallUndoableCommand` в `spec/Spec.cpp`).

**Runtime:** endpoint-вызов — not verified (требуется вызов `API.ExecuteAddOnCommand` на запущенном Archicad); clang-format, clangd (0 диагностик) и `BuildAddOn.py -v 25` — пройдены.

---

### 3. `Health` (CommandBase) — в коде, НЕ зарегистрирован

**Назначение:** проверка доступности аддона + версия.

**Входные параметры:** нет.

**Ответ:**
```json
{ "status": "ok", "version": "1.2.3", "addon": "SomeStuff" }
```

Версия берётся из ресурсов:
```cpp
GS::UniString version = RSGetIndString (ID_ADDON_STRINGS, VersionId, ACAPI_GetOwnResModule ());
// ID_ADDON_STRINGS = 32501 (api_headers/ResourceIds.hpp), VersionId = 49 (Constants.hpp)
```

По решению #205 в регистратор включён минимальный набор (RoomBook, Spec); Health оставлен в дереве как готовый шаблон read-only команды — включается в `RegisterJsonCommands()` при появлении внешнего потребителя.

**Удалённая команда:** `GetPropertyDefinitions` — в восстановленный набор не входит (файлов в `json_commands/` нет, не регистрируется). Историческое описание удалено.

---

## Регистрация команд (`JsonCommandRegistrar.cpp`)

Фактический код (AC25+):

```cpp
void RegisterJsonCommands () {
    GS::Owner<RoomBookCommand> roomBookCommand = GS::NewOwned<RoomBookCommand> ();
    const GSErrCode roomBookErr = ACAPI_Install_AddOnCommandHandler (roomBookCommand.Pass ());
    if (roomBookErr != NoError)
        DBprnt ("Failed to register RoomBookCommand, error: " + GS::ValueToUniString (roomBookErr));

    GS::Owner<SpecCommand> specCommand = GS::NewOwned<SpecCommand> ();
    const GSErrCode specErr = ACAPI_Install_AddOnCommandHandler (specCommand.Pass ());
    if (specErr != NoError)
        DBprnt ("Failed to register SpecCommand, error: " + GS::ValueToUniString (specErr));
}
```

- Регистрация вызывается из `SomeStuff_Main.cpp`, `Initialize()`:
```cpp
// Регистрация JSON команд
RegisterJsonCommands ();
```
- Несколько команд в одном аддоне — штатно (примеры Tapir/Dotbim в cpprag): каждый вызов `ACAPI_Install_AddOnCommandHandler` добавляет одну команду, диспетчеризация — по `commandName`.
- Логирование — только при ошибке: `DBprnt` + `GS::ValueToUniString(err)`; "registered successfully"-логи не пишутся.
- Шаблон добавления новой команды: создать `XxxCommand.hpp/cpp` по образцу `SpecCommand`, добавить include и блок регистрации; `CommonSchema::NotUsed` — если команда не использует общий контракт схем.

---

## Вызов из Python (property_bridge.py)

Правильный формат для ArchiCAD 25 JSON API:

```python
payload = {
    "command": "API.ExecuteAddOnCommand",
    "parameters": {
        "addOnCommandId": {
            "commandNamespace": "SomeStuffCommand",  # CommandBase::GetNamespace()
            "commandName": "Spec"                    # Command::GetName(): Spec | RoomBook | Health
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

- Порт определяется автообнаружением в python (скан 19723–19743), не передаётся снаружи.
- Команды выполнять после инициализации аддона и открытия проекта; runner (`restart_archicad_for_test.ps1`) ждёт 15 секунд после запуска Archicad.

---

## Важные правила

1. **Поток выполнения:** все команды `SomeStuffCommand` выполняются в главном потоке Archicad (`ScheduleForExecutionOnMainThread` в `CommandBase`, `override final`) — они читают/изменяют БД. Параллельного режима нет.
2. **Undo:** undo-скоуп открывает обёрнутая бизнес-логика (`Roombook`, `Spec` — внутри `ACAPI_CallUndoableCommand`). Команда не создаёт и не дублирует undo.
3. **Контракт ответа:** `status="returned"` + `elapsedSeconds` = факт возврата и длительность. Не заявлять успех операции, доступность отмены или корректность модели без проверяемого сигнала.
4. **Порядок include:** `ACAPinc.h` до `#if defined(ServerMainVers_2500)` в каждом `.cpp`/`.hpp` команды.
5. **Ошибки:** `DBprnt` с `GS::ValueToUniString(err)`; ошибки регистрации не фатальны для старта аддона.
6. **Кодировка строк:** `GS::UniString` везде; в C-строку — `.ToCStr().Get()`; в ответ — `response.Add("key", ...)`.

---

## LightRAG / DevKit проверки (AC25)

| Символ/контракт | Статус |
|-----------------|--------|
| `ACAPI_Install_AddOnCommandHandler(GS::Owner<API_AddOnCommand>)` | ✅ verified — LightRAG + ACAPinc.h DevKit-25 |
| `API_AddOnCommand::Execute(const GS::ObjectState&, GS::ProcessControl&) -> GS::ObjectState` | ✅ verified — LightRAG |
| `ScheduleForExecutionOnMainThread` для команд, трогающих БД | ✅ verified — LightRAG |
| Несколько команд на аддон (примеры Tapir/Dotbim) | ✅ verified — примеры cpprag |
| `ACAPI_CallUndoableCommand` | ✅ verified — AC25 (используется в Roombook/Spec) |
| `RSGetIndString` | ✅ verified — AC25 |

---

## Следующие шаги

1. ✅ `RoomBook` — endpoint подтверждён runtime (AC25, 15.085 c, порт 19723)
2. ✅ `Spec` — реализована и собрана (AC25, #206); endpoint-вызов — ожидает проверки на запущенном Archicad
3. ⏳ `Health` — включить в регистратор при появлении внешнего потребителя
4. ⏳ ParseProperty / Renum-команды, MCP-сервер — отдельные задачи
