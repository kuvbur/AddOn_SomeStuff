# Универсальная заготовка команд для ArchiCAD Add-On

## Структура файлов

```
Sources/AddOn/Commands/
├── CommandBase.hpp/cpp          # Базовые классы (ReadOnlyCommand, ModifyCommand)
├── ExampleCommands.hpp/cpp      # Примеры команд (чтение и изменение)
└── RoomBookCommands/             # Команды для RoomBook (по мере рефакторинга)
    ├── RoomBookCommandBase.hpp/cpp
    ├── GetTargetZonesCommand.hpp/cpp    # Read-only (Шаг 1)
    ├── ProcessRoomFinishesCommand.hpp/cpp  # Modify (Шаг 5)
    └── ...
```

## Использование

### 1. Read-only команда (только чтение, БЕЗ undo)

```cpp
// MyReadOnlyCommand.hpp
class MyReadOnlyCommand : public ReadOnlyCommand
{
public:
    MyReadOnlyCommand() {}
    
    GS::String GetName() const override { return "MyReadOnly"; }
    
    GS::ObjectState Execute(const GS::ObjectState& parameters,
                           GS::ProcessControl& processControl) const override
    {
        // Читаем параметры
        GS::UniString param;
        parameters.Get("param", param);
        
        // Читаем данные из проекта (БЕЗ изменений)
        // НЕТ ACAPI_CallUndoableCommand
        
        GS::ObjectState response;
        response.Add("result", "some data");
        return response;
    }
};
```

### 2. Modify команда (изменяет проект, С undo)

```cpp
// MyModifyCommand.hpp
class MyModifyCommand : public ModifyCommand
{
public:
    MyModifyCommand() {}
    
    GS::String GetName() const override { return "MyModify"; }
    
    GS::ObjectState Execute(const GS::ObjectState& parameters,
                           GS::ProcessControl& processControl) const override
    {
        // Читаем параметры
        GS::ObjectState elementId;
        parameters.Get("elementId", elementId);
        
        // Обёртка undo (ОБЯЗАТЕЛЬНО для изменений)
        GSErrCode err = ACAPI_CallUndoableCommand("My Modify Command", [&]() -> GSErrCode {
            
            // Код изменения проекта
            API_Element element = {};
            // ... изменение элемента
            
            return NoError;
        });
        
        if (err != NoError) {
            return CreateErrorResponse(err, "Failed to modify");
        }
        
        return CreateSuccessResponse();
    }
};
```

## Регистрация команд в AddOnMain.cpp

```cpp
// В функции Initialize()
{
    // Read-only команды
    CommandGroup readCommands("Read Commands");
    err |= RegisterCommand<MyReadOnlyCommand>(readCommands, "1.0.0", "Description");
    AddCommandGroup(readCommands);
    
    // Modify команды
    CommandGroup modifyCommands("Modify Commands");
    err |= RegisterCommand<MyModifyCommand>(modifyCommands, "1.0.0", "Description");
    AddCommandGroup(modifyCommands);
}
```

## Важные замечания

1. **Read-only команды** НЕ оборачиваются в `ACAPI_CallUndoableCommand`
2. **Modify команды** ОБЯЗАТЕЛЬНО оборачиваются в `ACAPI_CallUndoableCommand`
3. Обе наследуются от `API_AddOnCommand` через `CommandBase`
4. JSON-схемы возвращаются через `GS::Optional<GS::UniString>` (пустой = нет схемы)
5. Ответы формируются через `GS::ObjectState`

## Проверка через LightRAG

Сигнатура `ACAPI_CallUndoableCommand` для ArchiCAD 25:
```cpp
GSErrCode ACAPI_CallUndoableCommand(GS::UniString name, lambda returning GSErrCode);
```

Пример из tapir:
```cpp
ACAPI_CallUndoableCommand("Create " + elemTypeName, [&] () -> GSErrCode {
    // код изменения
    return NoError;
});
```
