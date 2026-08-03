# JSON Commands Architecture — ArchiCAD Add-On SomeStuff

## Текущая структура файлов (2025-07-31)

```
Sources/AddOn/json_commands/
├── CommandBase.hpp/cpp          # Базовые классы (ReadOnlyCommand, ModifyCommand)
├── JsonCommandRegistrar.hpp/cpp # Регистрация всех команд в ArchiCAD
├── GetPropertyDefinitionsCommand.hpp/cpp  # Получение списка свойств (read-only)
├── HealthCommand.hpp/cpp        # Health check + версия аддона (read-only)
└── How JSON Commands work.md    # Этот файл
```

---

## Архитектура команд

### Базовые классы (`CommandBase.hpp`)

```cpp
// ReadOnlyCommand — для команд ТОЛЬКО ЧТЕНИЯ (БЕЗ undo)
// Используется для: GetPropertyDefinitions, Health
class ReadOnlyCommand : public API_AddOnCommand { ... };

// ModifyCommand — для команд ИЗМЕНЕНИЯ (С undo через ACAPI_CallUndoableCommand)
class ModifyCommand : public API_AddOnCommand { ... };
```

**Важные методы базового класса:**
- `GetName()` — имя команды (например, `"GetPropertyDefinitions"`)
- `GetInputParametersSchema()` — JSON Schema для входных параметров (опционально)
- `GetResponseSchema()` — JSON Schema для ответа (опционально)
- `GetExecutionPolicy()` — политика выполнения (instant/main thread)
- `Execute()` — основная логика команды
- `CreateSuccessResponse()` / `CreateErrorResponse()` — помощники для ответов

---

## Реализованные команды

### 1. `GetPropertyDefinitions` (ReadOnlyCommand)

**Назначение:** Возвращает все свойства, загруженные в `PROPERTYCACHE().property`

**Входные параметры:** нет (пустой ObjectState)

**Выходные данные:**
```json
{
  "properties": [
    {
      "rawName": "SomeStuff_PropertyName",
      "name": "CleanName",
      "displayName": "Отображаемое имя",
      "description": "Описание свойства",
      "type": "string|integer|real|boolean|guid"
    }
  ],
  "count": 42,
  "status": "ok"
}
```

**Особенности:**
- Не использует `ACAPI_CallUndoableCommand` (только чтение)
- Итерация по `GS::HashTable` с правильным разыменованием указателей
- Возвращает тип свойства через switch по `paramValue.val.type`

---

### 2. `Health` (ReadOnlyCommand)

**Назначение:** Проверка работоспособности аддона + получение версии

**Входные параметры:** нет

**Выходные данные:**
```json
{
  "status": "ok",
  "version": "1.2.3",
  "addon": "SomeStuff"
}
```

**Получение версии:**
```cpp
GS::UniString version = RSGetIndString(ID_ADDON_STRINGS, VersionId, ACAPI_GetOwnResModule());
// ID_ADDON_STRINGS = 32501 (ResourceIds.hpp)
// VersionId = 49 (Constants.hpp)
```

---

## Регистрация команд (`JsonCommandRegistrar.cpp`)

```cpp
void RegisterJsonCommands()
{
    // Регистрация GetPropertyDefinitions
    {
        GS::Owner<GetPropertyDefinitionsCommand> cmd = GS::NewOwned<GetPropertyDefinitionsCommand>();
        GSErrCode err = ACAPI_Install_AddOnCommandHandler(cmd.Pass());
        if (err != NoError) {
            DBprnt("Failed to register GetPropertyDefinitionsCommand, error: " + GS::ValueToUniString(err));
        } else {
            DBprnt("GetPropertyDefinitionsCommand registered successfully!");
        }
    }
    
    // Регистрация Health
    {
        GS::Owner<HealthCommand> cmd = GS::NewOwned<HealthCommand>();
        GSErrCode err = ACAPI_Install_AddOnCommandHandler(cmd.Pass());
        if (err != NoError) {
            DBprnt("Failed to register HealthCommand, error: " + GS::ValueToUniString(err));
        } else {
            DBprnt("HealthCommand registered successfully!");
        }
    }
}
```

**Вызов в `SomeStuff_Main.cpp` (в `Initialize()`):**
```cpp
// Регистрация JSON команд
RegisterJsonCommands();

ACAPI_KeepInMemory(true);
```

---

## Вызов из Python (property_bridge.py)

Правильный формат для ArchiCAD 25 JSON API:

```python
# Все команды вызываются через API.ExecuteAddOnCommand
payload = {
    "command": "API.ExecuteAddOnCommand",
    "parameters": {
        "addOnCommandId": {
            "commandNamespace": "SomeStuffCommand",  # из CommandBase::GetNamespace()
            "commandName": "GetPropertyDefinitions"  # из Command::GetName()
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

---

## Важные правила

1. **Read-only команды** (GetPropertyDefinitions, Health):
   - НЕ оборачиваются в `ACAPI_CallUndoableCommand`
   - Наследуются от `ReadOnlyCommand`
   - Выполняются мгновенно в параллельном потоке

2. **Modify команды** (будущие):
   - ОБЯЗАТЕЛЬНО оборачиваются в `ACAPI_CallUndoableCommand`
   - Наследуются от `ModifyCommand`
   - Выполняются в главном потоке ArchiCAD

3. **JSON API формат** (ArchiCAD 25):
   - Всегда используйте `API.ExecuteAddOnCommand`
   - Namespace: `SomeStuffCommand` (из `CommandBase::GetNamespace()`)
   - Command name: из `Command::GetName()`
   - Параметры в `addOnCommandParameters`

4. **Обработка ошибок:**
   - Всегда логируйте `GSErrCode` через `DBprnt` с `GS::ValueToUniString(err)`
   - Проверяйте `response.ok` в Python перед парсингом JSON

5. **Кодировка строк:**
   - Используйте `GS::UniString` для всех строк
   - Конвертация в C-строку: `.ToCStr().Get()`
   - Для добавления в `GS::ObjectState`: `response.Add("key", uniString)`

---

## LightRAG проверки (верифицированные)

| API функция | Статус | Версия |
|-------------|--------|--------|
| `ACAPI_Install_AddOnCommandHandler` | ✅ verified | AC25 |
| `ACAPI_CallUndoableCommand` | ✅ verified | AC25 |
| `RSGetIndString` | ✅ verified | AC25 |

---

## Следующие шаги

1. ✅ GetPropertyDefinitions — получение свойств
2. ✅ Health — health check + версия
3. ⏳ **ParsePropertyCommand** — парсинг описаний свойств (Renum_flag, Renum)
4. ⏳ **RenumCommands** — команды пересчёта номеров
5. ⏳ **SpecCommands** — спецификации из описаний
6. ⏳ **MCP Server** — Python сервер для LLM интеграции