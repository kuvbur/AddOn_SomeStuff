# json_commands — JSON-команды аддона (AC25–29)

> Карточка создана 2026-09-25 по коду ветки `llm_test` (HEAD `13c1948`). Модуль восстановлен в #205 (RoomBook) и #207/#208 (Spec); AC22–24 не поддерживаются — весь код под `#if defined(ServerMainVers_2500)`. Номера строк — определения в `.cpp`, 1-based (проверены grep/read). Своя справка по API: `Sources/AddOn/json_commands/How JSON Commands work.md` [из файла].

## Назначение

Подсистема HTTP/JSON-команд ArchiCAD (`API_AddOnCommand`): внешний агент (по HTTP-порту экземпляра Archicad) может вызвать модульные операции аддона — построение отделки (RoomBook) и non-interactive построение спецификации (Spec). [по коду]

## Файлы

- `CommandBase.cpp/hpp` — базовые классы команд, общие хелперы ответов
- `RoomBookCommand.cpp/hpp` — команда `SomeStuffCommand.RoomBook` (#205)
- `SpecCommand.cpp/hpp` — команда `SomeStuffCommand.Spec` (#207/#208)
- `HealthCommand.cpp/hpp` — read-only шаблон `SomeStuffCommand.Health` (в регистратор НЕ включён, см. #205 / How JSON Commands work.md:119)
- `JsonCommandRegistrar.cpp/hpp` — `RegisterJsonCommands()`, вызывается из `Initialize` (SomeStuff_Main.cpp:569)
- `How JSON Commands work.md` — внутренняя справка [из файла]

## Ключевые типы

| Тип | Где | Назначение |
|-----|-----|-----------|
| `CommandBase` | CommandBase.hpp:17 | Базовый `API_AddOnCommand`: namespace `SomeStuffCommand` (CommandBase.cpp:8), политика `ScheduleForExecutionOnMainThread` (CommandBase.cpp:23) [по коду] |
| `ReadOnlyCommand` | CommandBase.hpp:58 | Подкласс без undo; унаследован только `HealthCommand` (не зарегистрирован) [по коду] |
| `ModifyCommand` | CommandBase.hpp:77 | Подкласс с поддержкой undo; в актуальном коде не используется — команды-изменения наследуют `CommandBase` напрямую [по коду] |
| `RoomBookCommand` | RoomBookCommand.hpp:7 | Команда без параметров, free-form ответ [по коду] |
| `SpecCommand` | SpecCommand.hpp:7 | Команда со схемой параметров (обязательный `placementPoint`) [по коду] |
| `HealthCommand` | HealthCommand.hpp:7 | Команда статуса/версии, read-only [по коду] |
| `CommonSchema` | CommandBase.hpp:15 | `Used`/`NotUsed` — режим общей схемы ответа; все актуальные команды используют `NotUsed` [по коду] |

## Публичный API

| Функция/метод | .cpp:строка | Назначение |
|---------------|-------------|-----------|
| `RegisterJsonCommands()` | JsonCommandRegistrar.cpp:15 | Регистрирует `RoomBookCommand` и `SpecCommand` через `ACAPI_Install_AddOnCommandHandler`; ошибка — `DBprnt`. Вне AC25 — пустая (JsonCommandRegistrar.cpp:32) [по коду] |
| `CommandBase::GetNamespace` | CommandBase.cpp:18 | Возвращает `SomeStuffCommand` [из комментария] |
| `CommandBase::GetExecutionPolicy` | CommandBase.cpp:23 | `ScheduleForExecutionOnMainThread` — модифицирующие команды только в главном потоке [из комментария] |
| `RoomBookCommand::GetName` | RoomBookCommand.cpp:19 | Имя команды `RoomBook` [по коду] |
| `RoomBookCommand::Execute` | RoomBookCommand.cpp:34 | Запускает `Roombook::RoomBook()`; ответ `{status:"returned", elapsedSeconds}` — `void`-функция не сообщает ошибку/отмену [по коду; IDEA.md Decisions #205] |
| `SpecCommand::GetName` | SpecCommand.cpp:20 | Имя команды `Spec` [по коду] |
| `SpecCommand::GetInputParametersSchema` | SpecCommand.cpp:25 | JSON-схема: `placementPoint{x,y}` обязательна, `ruleNames[]` опциональна (minItems 1) [по коду] |
| `SpecCommand::Execute` | SpecCommand.cpp:57 | Без `placementPoint` — ошибка `APIERR_BADPARS`; иначе `LoadSyncSettingsFromPreferences` → `Spec::SpecAll(syncSettings, ruleNames?, &placementPoint, &runResult)`; ответ: `status` completed/failed, `resultCode`, счётчики `elementsToCreate/Modify/Delete`, `elapsedSeconds` [по коду] |
| `HealthCommand::Execute` | HealthCommand.cpp:34 | Ответ `{status:"ok", version: <из ID_ADDON_STRINGS VersionId>, addon:"SomeStuff"}` [по коду] |
| `CreateErrorResponse` / `CreateSuccessResponse` | CommandBase.cpp:50 / 60 | Свободные хелперы ответов (`error{code}`, `message` / `success:true`) [по коду] |
| `GetGuidFromObjectState` / `CreateGuidObjectState` (2 перегрузки) | объявлены в CommandBase.hpp:99-109 | Хелперы GUID-обёрток; определения не найдены в .cpp — [назначение не установлено: потенциально мёртвый код, не проверено] |

## Карточки

### `RegisterJsonCommands() -> void`
- Расположение: `Sources/AddOn/json_commands/JsonCommandRegistrar.cpp:15`
- Назначение: установка обработчиков двух JSON-команд. [из комментария]
- Контракт: вызывается один раз из `Initialize` (SomeStuff_Main.cpp:569), до `BrowserPalette::RegisterPaletteControlCallBack`. `#include "ACAPinc.h"` обязан предшествовать проверке версии (макрос даёт этот заголовок) — иначе пустая единица трансляции и ошибка на линковке [из How JSON Commands work.md:17-18].
- Побочные эффекты: регистрация HTTP-эндпоинтов на порту экземпляра Archicad; `DBprnt` при ошибке установки.
- Вызывает: `ACAPI_Install_AddOnCommandHandler` (×2, SDK), `DBprnt` (CommonFunction).
- Вызывается из: `Initialize` (SomeStuff_Main.cpp:569). [grep]

### `RoomBookCommand::Execute(parameters, processControl) -> GS::ObjectState`
- Расположение: `Sources/AddOn/json_commands/RoomBookCommand.cpp:34`
- Назначение: запускает полное построение отделки без UI. [из комментария]
- Контракт: параметров нет (`GetInputParametersSchema` — `NoValue`); `processControl` игнорируется — отмена не поддерживается. Ответ `status:"returned"` означает только возврат из `RoomBook` (функция `void`) [по коду; IDEA.md Decisions #205].
- Побочные эффекты: модификация БД (создание/пересоздание элементов отделки) в главном потоке; длительность ~15–318 с (замеры runtime AC25: 15.09 с и 318.39 с, #205/#206).
- Вызывает: `Roombook::RoomBook` (Roombook.cpp, см. Docs/modules/Roombook.md).
- Вызывается из: диспетчеризации Archicad по имени команды (внешний HTTP-вызов); в коде вызовов нет. [по коду]

### `SpecCommand::Execute(parameters, processControl) -> GS::ObjectState`
- Расположение: `Sources/AddOn/json_commands/SpecCommand.cpp:57`
- Назначение: non-interactive построение спецификации: не открывает интерфейс, создаёт элементы по текущим правилам. [из комментария]
- Контракт: обязательный `placementPoint{x,y}` (тип числовой) — иначе `APIERR_BADPARS`; `ruleNames` (массив строк, minItems 1) опционален; передаётся в `Spec::SpecAll` вторым аргументом как `GS::Array<GS::UniString>*` или `nullptr`. Ответ: `status` completed/failed + `resultCode` (GSErrCode) + счётчики `elementsToCreate/elementsToModify/elementsToDelete` из `Spec::SpecRunResult` + `elapsedSeconds` [по коду].
- Побочные эффекты: `LoadSyncSettingsFromPreferences` — чтение настроек (свой путь, не диалог); `Spec::SpecAll` — создание/изменение/удаление элементов БД в главном потоке.
- Вызывает: `LoadSyncSettingsFromPreferences` (dialogs/SyncSettings.cpp), `Spec::SpecAll` (spec/Spec.cpp). [grep]
- Вызывается из: диспетчеризации Archicad по имени команды (внешний HTTP-вызов); в коде вызовов нет. [по коду]

## Зависимости

- `ACAPinc.h`, `api_headers/APIEnvir.h`, `ObjectState.hpp`, `OnExit.hpp` (CommandBase) [по include]
- `Roombook.hpp` (RoomBookCommand), `dialogs/SyncSettings.hpp`, `spec/Spec.hpp` (SpecCommand), `CommonFunction.hpp`, `Constants.hpp`, `api_headers/ResourceIds.hpp` (HealthCommand/Registrator) [по include]

## Инварианты и ограничения

- Все `.cpp` под `#if defined(ServerMainVers_2500)`; на AC22–24 `RegisterJsonCommands()` — пустая функция, модуль не компилируется. [по коду]
- Каждый JSON API HTTP-порт соответствует конкретному запущенному экземпляру Archicad — выбор «куда» делает вызывающий через порт, параметром не передаётся. [IDEA.md Decisions #205]
- Не реинтродуцировать JSON-команды «в ствол» через BrowserPalette-мост: мост и JSON-команды — разные каналы (мост = инлайн-функции `RegisterACAPIJavaScriptObject`), см. ARCHITECTURE.md §JS Bridge. [по коду]
- `HealthCommand` намеренно не регистрируется — остаётся шаблоном read-only команды (решение #205). [из How JSON Commands work.md:119]
- Конфликт имен: `CreateErrorResponse`/`CreateSuccessResponse` существуют и как свободные (CommandBase.cpp:50/60), и как protected-методы `ReadOnlyCommand`/`ModifyCommand` — дублирование API; актуальные команды используют свободные. [по коду]
