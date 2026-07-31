# Current Task

## Task

Рефакторинг `RoomBook()` в `Roombook.cpp` с целью:

1. Разбить монолитную функцию на изолированные, одноцелевые хелперы.
2. Спроектировать эти функции так, чтобы они были пригодны для вызова через JSON/MCP (внешнее управление аддоном).
3. Устранить скрытое разделяемое состояние (`nPhase`, `funcname`, `reducededges`).

## Current State & Resume Marker

- **Status:** IN_PROGRESS
- **Last Action Completed:** Верификация API через LightRAG завершена. Подтверждены: `GS::HashTable` требует копируемые типы значений, `GS::ProcessControl::IsCanceled()`, `GS::Guid::Generate()`, `ACAPI_CallUndoableCommand`.
- **Immediate Next Step:** Запись исправленного детального плана в IDEA.md (этот файл).

## Architectural Decision: JSON/MCP Granularity

**ВЫБРАН ВАРИАНТ Б**: Каждая извлечённая функция — отдельный JSON/MCP-инструмент.

### Стратегия управления состоянием (State Management)

Поскольку `RoomProcessingContext` содержит сложные типы (GS::HashTable с API_Guid ключами, ParamDictElement, OtdRooms и т.п.), полная сериализация в JSON между вызовами слишком дорогая и сложная.

**Выбранный подход:** In-memory state с `sessionId`.

1. Аддон поддерживает словарь активных сессий: `GS::HashTable<GS::UniString, GS::SharedPtr<RoomProcessingContext>> activeSessions`.
2. Первый вызов (например, `roombook_init`) создаёт сессию, возвращает `sessionId`.
3. Последующие вызовы принимают `sessionId` и работают с сохранённым контекстом.
4. Финальный вызов (`roombook_commit` или `roombook_cleanup`) освобождает сессию.

**Важно:** Используется `GS::SharedPtr<RoomProcessingContext>` как значение в хеш-таблице, так как `GS::HashTable` требует копируемые типы значений, а `RoomProcessingContext` содержит некопируемые члены.

**Альтернатива (отклонена):** Полная сериализация контекста в JSON. Слишком сложно для типов ArchiCAD (API_Guid, memo-структуры, полилинии).

### Стратегия транзакций и undo

**Проблема:** `ACAPI_CallUndoableCommand` оборачивает операцию в один undo-шаг. Если каждый MCP-инструмент будет отдельным вызовом `API_AddonCommand`, каждый создаст отдельный undo-шаг — пользователь получит 8 undo-шагов для одной операции `RoomBook`.

**Решение:**

- Каждый MCP-инструмент, изменяющий базу данных, оборачивает свою работу в `ACAPI_CallUndoableCommand`.
- Только инструменты, реально создающие/изменяющие элементы (Шаги 5, 6, 7, 8), вызывают `ACAPI_CallUndoableCommand`.
- Read-only шаги (1–4) не требуют undo-обёртки.
- Оркестратор (MCP-клиент) должен документировать, что многошаговый undo ожидаем.

**Альтернатива (отклонена):** Обернуть весь пайплайн в один undoable command. Конфликтует с дизайном "каждая функция = отдельный MCP-инструмент", так как undoable command выполняется синхронно до завершения.

### Модель потоков

`API_AddonCommand` выполняется в главном потоке (синхронная диспетчеризация). Если это так, `GS::Mutex` в `SessionManager` не нужен сейчас. План должен явно указывать это допущение и отметить, что конкурентные MCP-вызовы будут сериализованы механизмом диспетчеризации ArchiCAD.

### Изменения в дизайне функций для Варианта Б

Каждая извлечённая функция теперь:

- Принимает `GS::UniString sessionId` (или создаёт сессию, если это инициализирующий вызов).
- Возвращает `Result<T>` с возможностью продолжения (не делает `return` из всего `RoomBook()`).
- Не использует глобальное состояние (`nPhase`, `funcname`, `reducededges`).

### Обновлённый порядок реализации

1. **Шаг 0** — `RoomProcessingContext` struct + механизм сессий (`SessionManager`).
2. **Шаг 1** — `GetTargetZones()` как отдельный инструмент (создаёт сессию).
3. Остальные шаги — каждый как отдельный инструмент, работающий с сессией.

---

## Tactical Step-by-Step Plan

### Шаг 0 — Введение `RoomProcessingContext` struct + `SessionManager`

- **Границы:** До начала любых извлечений.
- **Содержание:**
  1. `RoomProcessingContext` struct (POD-контейнер для 13+ полей, перечисленных ниже).
  2. `SessionManager` — класс/пространство имён для управления сессиями:
     ```cpp
     class SessionManager {
     public:
         GS::UniString CreateSession(); // Создаёт новую сессию, возвращает sessionId
         GS::SharedPtr<RoomProcessingContext> GetSession(const GS::UniString& sessionId); // Возвращает контекст по Id
         void DeleteSession(const GS::UniString& sessionId); // Освобождает сессию
     private:
         GS::HashTable<GS::UniString, GS::SharedPtr<RoomProcessingContext>> activeSessions;
     };
     ```
- **Важно:** `SessionManager` пока НЕ требует потокобезопасности (`GS::Mutex` отложен), так как `API_AddonCommand` выполняется в главном потоке.
- **Риск:** Низкий. Только упаковка структур.
- **Верификация:** `GS::HashTable` требует копируемые типы значений. `GS::SharedPtr` — копируемый (подтверждено через LightRAG).

### Шаг 1–8 (каждая функция = отдельный JSON/MCP-инструмент)

Каждая функция теперь:

- Принимает `GS::UniString sessionId` (кроме инициализирующей).
- Возвращает `Result<T>` (значение + `GSErrCode` + сообщение).
- Не делает `return` из всего `RoomBook()`, а возвращает управление оркестратору.

Ниже — обновлённые сигнатуры для каждого шага.

#### Шаг 1 — `GetTargetZones(sessionId?)`

- **Сигнатура:**
  ```cpp
  Result<GS::UniString> InitRoomBook(); // Создаёт сессию, заполняет zones, возвращает sessionId
  ```
- **Действие:** Создаёт сессию, определяет целевые зоны, сохраняет в контекст.
- **Undo:** Не требуется (только чтение).

#### Шаг 2 — `PrepareRoomProcessingContext(sessionId)`

- **Сигнатура:** `Result<void> PrepareContext(const GS::UniString& sessionId);`
- **Действие:** Заполняет поля контекста (кроме тех, что заполняются позже).
- **Undo:** Не требуется (только подготовка данных).

#### Шаг 3 — `BuildElementReadIndex(sessionId)` + `ProcessElementsForRoomData(sessionId)`

- **Сигнатуры:**
  ```cpp
  Result<void> BuildElementReadIndex(const GS::UniString& sessionId);
  Result<void> ProcessElementsForRoomData(const GS::UniString& sessionId);
  ```
- **Риск:** `ClearZoneGUID` на границе между функциями.
- **Undo:** Не требуется (только чтение и индексация).

#### Шаг 4 — `PrepareReadParams(sessionId)` + `ReadElementParameters(sessionId)`

- **Сигнатуры:**
  ```cpp
  Result<void> PrepareReadParams(const GS::UniString& sessionId);
  Result<void> ReadElementParameters(const GS::UniString& sessionId);
  ```
- **Undo:** Не требуется (только чтение параметров).

#### Шаг 5 — `ProcessRoomFinishes(sessionId)`

- **Сигнатура:** `Result<void> ProcessRoomFinishes(const GS::UniString& sessionId);`
- **Подфункции:** `ProcessSlabFinishes`, `ProcessWallFinishes`, `ApplyFavoriteAndMaterialData` — приватные хелперы внутри, не экспортируются как отдельные инструменты (гранулярность не доходит до этого уровня, иначе будет слишком мелко).
- **Undo:** Оборачивается в `ACAPI_CallUndoableCommand` (создаёт/изменяет элементы отделки).

#### Шаг 6 — `BuildMaterialSummaryForRooms(sessionId)` + `WriteRoomMaterialData(sessionId)`

- **Сигнатуры:**
  ```cpp
  Result<void> BuildMaterialSummary(const GS::UniString& sessionId);
  Result<void> WriteRoomMaterialData(const GS::UniString& sessionId);
  ```
- **Undo:** `WriteRoomMaterialData` оборачивается в `ACAPI_CallUndoableCommand` (запись данных в элементы).

#### Шаг 7 — `RemoveUnusedFinishingElements(sessionId)` + `PrepareElementsForUpdate(sessionId)`

- **Сигнатуры:**
  ```cpp
  Result<void> RemoveUnusedElements(const GS::UniString& sessionId);
  Result<void> PrepareUpdate(const GS::UniString& sessionId);
  ```
- **Undo:** `RemoveUnusedElements` оборачивается в `ACAPI_CallUndoableCommand` (удаление элементов).
- **Важно:** Проверить, не зависит ли этот шаг от контейнеров, заполняемых в Шагах 2–4. Если зависит, порядок реализации должен быть скорректирован.

#### Шаг 8 — `CommitRoomBookResults(sessionId)`

- **Сигнатура:** `Result<void> CommitRoomBook(const GS::UniString& sessionId);`
- **Действие:** `Draw_Elements` + `SetSyncOtdWall` + очистка сессии.
- **Undo:** Оборачивается в `ACAPI_CallUndoableCommand` (финальные изменения).

---

## Additional Improvement Ideas (обновлено для Варианта Б)

### Идея №1 — Явная "commit"-фаза (ушла в Шаг 8)

См. выше.

### Идея №2 — `ProgressReporter` вместо глобальных `nPhase`/`funcname`

- **Решение:** Для JSON/MCP-вызовов прогресс-бар не нужен. `CheckCancelled()` становится no-op.
- **Но:** Если функция вызывается и из UI, и из JSON — нужен абстрактный `IProgressReporter` с двумя реализациями: `UIProgressReporter` и `NoOpProgressReporter`.
- **Правка:** Имя метода — `GS::ProcessControl::IsCanceled()` (не `CheckCancelled`).

### Идея №3 — `CheckCancelled()` как no-op для JSON/MCP

- **Решение:** `bool CheckCancelled() { return false; }` для не-UI вызовов.
- **Правка:** Использовать `IsCanceled()` из `GS::ProcessControl` (подтверждено через LightRAG).

### Идея №4 — Структурированный `Result<T>` вместо `msg_rep`

- **Решение (C++14-совместимый):**
  ```cpp
  template<typename T>
  struct Result {
      T value;
      GSErrCode err;
      GS::UniString message;
      GS::Array<GS::UniString> missingProperties; // Для структурированных ошибок
  };
  ```
- **Важно:** `std::optional` НЕ доступен в C++14 — использовать `GS::Optional` или флаг `bool hasValue`.

### Идея №5 — `reducededges` как локальное поле `RoomProcessingContext`

- **Решение:** Перенести из глобальной области в struct. Указатель будет жить только в рамках сессии.

### Идея №6 — Очистка сессий (resource leak prevention)

- **Решение:** `SessionManager` должен очищать неиспользуемые сессии:
  - По таймауту (например, 10 минут неактивности).
  - При выгрузке аддона (в `FreeData` или аналогичном обработчике).
  - Ограничение на максимальное количество сессий (например, 100).

---

## Execution Log & Decisions

- [2026-07-30] Прочитан `Roombook.cpp`, подтверждены границы всех 8 блоков. Глобальные переменные `reducededges` (строка 27), `nPhase` (строка 31) отмечены для рефакторинга.
- [2026-07-30] **ВЫБРАН ВАРИАНТ Б:** Каждая функция — отдельный JSON/MCP-инструмент. Стратегия: in-memory state с `sessionId`.
- [2026-07-30] План обновлён в IDEA.md с учётом верификации через LightRAG:
  - Исправлено использование `GS::SharedPtr<RoomProcessingContext>` в `SessionManager`.
  - Добавлена стратегия undo (только изменяющие шаги оборачиваются в `ACAPI_CallUndoableCommand`).
  - Уточнена модель потоков (`API_AddonCommand` в главном потоке, mutex отложен).
  - Исправлены имена API (`IsCanceled` вместо `CheckCancelled`).
  - Добавлена генерация sessionId через `GS::Guid::Generate().ToUniString()`.
  - Добавлены правила жизненного цикла сессий (таймаут, лимиты, очистка при выгрузке).
  - Указана совместимость `Result<T>` с C++14 (без `std::optional`).
- [2026-07-30] Следующий шаг: Шаг 0 (введение `RoomProcessingContext` + `SessionManager`).

## Implementation Order (Checkpoints)

Строго последовательно, по одному коммиту на пункт, сборка после каждого:

1. **Шаг 0** — Ввести `RoomProcessingContext` struct (без логики, только поля) + `SessionManager` с `GS::SharedPtr`.
2. **Шаг 1** — `InitRoomBook()` (самый изолированный, создаёт сессию).
3. **Шаг 7** — `RemoveUnusedFinishingElements()` + `PrepareElementsForUpdate()` (относительно изолирован, проверка паттерна на менее рискованном участке). **Важно:** проверить зависимости от Шагов 2–4.
4. **Шаг 2** → **Шаг 4** → **Шаг 3** (в этом порядке, т.к. Шаг 3 зависит от контейнеров Шага 2/4).
5. **Шаг 5** (в подпунктах: сначала `ProcessSlabFinishes`, отдельным коммитом `ProcessWallFinishes`, отдельным `ApplyFavoriteAndMaterialData`).
6. **Шаг 6**.
7. **Шаг 8** (commit-фаза) и **Идея №3** (`IsCanceled`) — как отдельные мелкие коммиты после стабилизации основного разбиения.
8. **Идеи №2, №4, №5, №6** — по мере готовности, как отдельные улучшения.

---

## Build Verification

- Каждый шаг должен заканчиваться успешной сборкой (`python Tools/BuildAddOn.py -c config.json -v 25`).
- Проверка LSP (если добавлены новые файлы/функции): `python Tools/BuildAddOn.py -c config.json -v 25 --lsp`.
- Никаких C++17/20 синтаксических конструкций в коде для AC 25 (C++14).
- Использовать `clang-format -i` после каждого редактирования C++ файлов.

---

## API Verification Notes (LightRAG Results)

1. `GS::HashTable` требует копируемые типы значений → используем `GS::SharedPtr`.
2. `GS::ProcessControl::IsCanceled()` — правильное имя метода (не `IsCancelled`).
3. `GS::Guid::Generate()` — статический метод генерации GUID (подтверждено).
4. `GS::Guid::ToUniString()` — конвертация в строку (подтверждено).
5. `ACAPI_CallUndoableCommand(name, lambda)` — обёртка для undo (подтверждено).
6. `GS::SharedPtr` — копируемый умный указатель (подтверждено через LightRAG).
7. `API_AddonCommand` выполняется в главном потоке → mutex отложен.

---

## Новая задача: MCP сервер для Archicad (2026-07-30)

### Суть задачи

Пользователь планирует создать свой MCP сервер для Archicad. Изучен пример реализации:

- **tapir-archicad-MCP** — Python MCP сервер
- **tapir-archicad-addon** — C++ аддон с JSON командами

### Изученная архитектура

1. **tapir-archicad-addon (C++):**
   - `CommandBase` — базовый класс для JSON команд
   - `API_AddOnCommand` — интерфейс ArchiCAD для регистрации команд
   - Регистрация через `ACAPI_AddOnAddOnCommunication_InstallAddOnCommandHandler`
   - Каждая команда: `GetName()`, `GetInputParametersSchema()`, `GetResponseSchema()`, `Execute()`

2. **tapir-archicad-MCP (Python):**
   - `fastmcp` — фреймворк для MCP сервера
   - `multiconn-archicad` — библиотека для связи с Archicad
   - `discover_tools` — семантический поиск по описаниям команд
   - `call_tool` — выполнение конкретной команды

### Разделение на части

**Часть 1:** Отдельный Python процесс с интерфейсом (по аналогии с `archixml.py`)

- HTTP сервер (ThreadingHTTPServer)
- HTML/JS интерфейс (встроен в код или папка)
- Прокси к Archicad JSON API
- WebSocket/SSE для мгновенного отклика UI

**Часть 2:** Взаимодействие с аддоном (JSON)

- Создать базовый класс для JSON команд
- Реализовать команды для работы с описанием свойств
- Зарегистрировать команды в `SomeStuff_Main.cpp`

### Фокус на Renum командах

Изучена документация `Element-Renumbering-en.md` и реализация в `ReNum.cpp`:

- Команды хранятся в описаниях свойств: `Renum_flag{...}` и `Renum{...}`
- Парсинг происходит в `ReNum_GetElement()`
- Константы определены в `Constants.hpp` (RENUMFLAG, RENUM, BRACESTART, BRACEEND и др.)

### Найденные готовые файлы

В папке `D:\SomeStuff_addon` уже созданы:

- `D:\SomeStuff_addon\Sources\AddOn\json_commands\CommandBase.hpp/cpp` — базовый класс (полностью подходит, повторяет структуру tapir)
- `D:\SomeStuff_addon\Code_Example\template_files_json_commands\ExampleCommands.hpp/cpp` — примеры реализации
- Готовая структура: `ReadOnlyCommand` и `ModifyCommand`

### Следующие шаги

1. Создать `ParsePropertyCommand` для парсинга команд Renum из описания свойств
2. Использовать готовую структуру из `json_commands/`
3. Реализовать логику парсинга (используя код из `ReNum_GetElement`)
4. Зарегистрировать команду в `SomeStuff_Main.cpp`
5. Протестировать через Python интерфейс

### Python интерфейс (Часть 1) — СОЗДАН

Создан файл `D:\SomeStuff_addon\python_test_files\property_bridge.py`:

- HTTP сервер на базе `ThreadingHTTPServer` (по аналогии с `archixml.py`)
- HTML/JS интерфейс встроен в код
- Кнопка "Получить свойства" и выпадающий список
- Вызов к Archicad JSON API через `ac_post()`
- Инструкции в `D:\SomeStuff_addon\python_test_files\property_bridge_README.md`

**Статус:** Python скрипт готов, синтаксис проверен. Ожидает регистрации команды `GetPropertyDefinitions` в аддоне.

### Полезные пути для поиска

- `D:\SomeStuff_addon\Code_Example\` — примеры реализации (tapir)
- `D:\SomeStuff_addon\Code_Example\template_files_json_commands\` — готовая база для JSON команд
- `D:\SomeStuff_addon\Sources\AddOn\ReNum.cpp` — логика парсинга Renum
- `D:\SomeStuff_addon\Sources\AddOn\Constants.hpp` — константы (RENUMFLAG, RENUM и др.)
- `D:\SomeStuff_addon\wiki\en\Element-Renumbering-en.md` — документация по Renum
- `D:\SomeStuff_addon\python_test_files\property_bridge.py` — Python интерфейс (НОВОЕ)
- `D:\SomeStuff_addon\python_test_files\property_bridge_README.md` — инструкции по запуску (НОВОЕ)

### JSON API Команды (новые файлы)

- `D:\SomeStuff_addon\Sources\AddOn\json_commands\CommandBase.cpp` / `.hpp` — базовый класс для JSON команд
- `D:\SomeStuff_addon\Sources\AddOn\json_commands\GetPropertyDefinitionsCommand.cpp` / `.hpp` — команда получения определений свойств
- `D:\SomeStuff_addon\Sources\AddOn\json_commands\JsonCommandRegistrar.cpp` / `.hpp` — регистратор JSON команд

### Файлы примеров кода в `D:\SomeStuff_addon\Code_Example` (включая вложенные папки)

#### `tapir-archicad-addon` — C++ аддон с JSON командами (reference implementation)

- `D:\SomeStuff_addon\Code_Example\tapir-archicad-addon\Sources\3DCutPlaneCommands.cpp` / `.hpp` — команды для 3D сечений
- `D:\SomeStuff_addon\Code_Example\tapir-archicad-addon\Sources\AboutDialog.cpp` / `.hpp` — диалог "О программе"
- `D:\SomeStuff_addon\Code_Example\tapir-archicad-addon\Sources\AddOnMain.cpp` — точка входа аддона
- `D:\SomeStuff_addon\Code_Example\tapir-archicad-addon\Sources\AddOnVersion.hpp` — версия аддона
- `D:\SomeStuff_addon\Code_Example\tapir-archicad-addon\Sources\APIEnvir.h` — обёртка для API окружения
- `D:\SomeStuff_addon\Code_Example\tapir-archicad-addon\Sources\ApplicationCommands.cpp` / `.hpp` — команды приложения
- `D:\SomeStuff_addon\Code_Example\tapir-archicad-addon\Sources\AttributeCommands.cpp` / `.hpp` — команды атрибутов
- `D:\SomeStuff_addon\Code_Example\tapir-archicad-addon\Sources\ClassificationCommands.cpp` / `.hpp` — команды классификации
- `D:\SomeStuff_addon\Code_Example\tapir-archicad-addon\Sources\CommandBase.cpp` / `.hpp` — **базовый класс для JSON команд** (CommandBase с виртуальными методами GetName, GetInputParametersSchema, GetResponseSchema, Execute)
- `D:\SomeStuff_addon\Code_Example\tapir-archicad-addon\Sources\Config.cpp` / `.hpp` — конфигурация
- `D:\SomeStuff_addon\Code_Example\tapir-archicad-addon\Sources\DesignOptionCommands.cpp` / `.hpp` — команды вариантов проектирования
- `D:\SomeStuff_addon\Code_Example\tapir-archicad-addon\Sources\DeveloperTools.cpp` / `.hpp` — инструменты разработчика
- `D:\SomeStuff_addon\Code_Example\tapir-archicad-addon\Sources\DocumentCreationCommands.cpp` / `.hpp` — команды создания документов
- `D:\SomeStuff_addon\Code_Example\tapir-archicad-addon\Sources\ElementCommands.cpp` / `.hpp` — команды элементов
- `D:\SomeStuff_addon\Code_Example\tapir-archicad-addon\Sources\ElementCreationCommands.cpp` / `.hpp` — команды создания элементов
- `D:\SomeStuff_addon\Code_Example\tapir-archicad-addon\Sources\ElementGDLParameterCommands.cpp` / `.hpp` — команды GDL параметров элементов
- `D:\SomeStuff_addon\Code_Example\tapir-archicad-addon\Sources\FavoritesCommands.cpp` / `.hpp` — команды избранного
- `D:\SomeStuff_addon\Code_Example\tapir-archicad-addon\Sources\IssueCommands.cpp` / `.hpp` — команды задач/проблем
- `D:\SomeStuff_addon\Code_Example\tapir-archicad-addon\Sources\LibraryCommands.cpp` / `.hpp` — команды библиотек
- `D:\SomeStuff_addon\Code_Example\tapir-archicad-addon\Sources\MigrationHelper.hpp` — помощник миграции
- `D:\SomeStuff_addon\Code_Example\tapir-archicad-addon\Sources\NavigatorCommands.cpp` / `.hpp` — команды навигатора
- `D:\SomeStuff_addon\Code_Example\tapir-archicad-addon\Sources\NotificationCommands.cpp` / `.hpp` — команды уведомлений
- `D:\SomeStuff_addon\Code_Example\tapir-archicad-addon\Sources\ProjectCommands.cpp` / `.hpp` — команды проекта
- `D:\SomeStuff_addon\Code_Example\tapir-archicad-addon\Sources\PropertyCommands.cpp` / `.hpp` — **команды свойств** (ключевой reference для GetPropertyDefinitions)
- `D:\SomeStuff_addon\Code_Example\tapir-archicad-addon\Sources\ResourceIds.hpp` — ID ресурсов
- `D:\SomeStuff_addon\Code_Example\tapir-archicad-addon\Sources\SchemaDefinitions.cpp` / `.hpp` — определения JSON схем
- `D:\SomeStuff_addon\Code_Example\tapir-archicad-addon\Sources\TapirPalette.cpp` / `.hpp` — палитра Tapir
- `D:\SomeStuff_addon\Code_Example\tapir-archicad-addon\Sources\TeamworkCommands.cpp` / `.hpp` — команды Teamwork

#### `tapir-archicad-MCP` — Python MCP сервер

- `D:\SomeStuff_addon\Code_Example\tapir-archicad-MCP\src\` — исходный код Python MCP сервера
- `D:\SomeStuff_addon\Code_Example\tapir-archicad-MCP\scripts\` — скрипты
- `D:\SomeStuff_addon\Code_Example\tapir-archicad-MCP\tests\` — тесты
- `D:\SomeStuff_addon\Code_Example\tapir-archicad-MCP\context\` — контекст/промпты
- `D:\SomeStuff_addon\Code_Example\tapir-archicad-MCP\pyproject.toml` — конфигурация проекта (fastmcp, multiconn-archicad)
- `D:\SomeStuff_addon\Code_Example\tapir-archicad-MCP\uv.lock` — lock-файл зависимостей
- `D:\SomeStuff_addon\Code_Example\tapir-archicad-MCP\README.md` — документация

#### `template_files_json_commands` — шаблоны для JSON команд (минимальный набор)

- `D:\SomeStuff_addon\Code_Example\template_files_json_commands\CommandBase.cpp` / `.hpp` — базовый класс команд (упрощённый)
- `D:\SomeStuff_addon\Code_Example\template_files_json_commands\ExampleCommands.cpp` / `.hpp` — примеры реализации команд
- `D:\SomeStuff_addon\Code_Example\template_files_json_commands\README.md` — описание шаблонов

#### Корневые файлы Code_Example

- `D:\SomeStuff_addon\Code_Example\archixml.py` — Python скрипт HTTP сервера с HTML/JS интерфейсом (reference для property_bridge.py)

---

## Новая задача: Интерфейс редактора описаний свойств (2026-07-31)

### Суть задачи

Создать веб-интерфейс (встроенный в C++ через `html_to_hpp.py`) для работы с описаниями свойств аддона SomeStuff. Интерфейс заменяет текущий React-заглушку (`dialogs/index.html`) на рабочий UI с двумя разделами: отслеживание значений свойств и редактор описаний с конструктором команд.

### Архитектура (существующая, не менять)

| Компонент | Назначение |
|-----------|------------|
| `BrowserPalette` (C++) | `DG::Palette` + `DG::Browser`, загружает HTML из `HTML_Pages.hpp` |
| `RegisterACAPIJavaScriptObject()` | Регистрирует `DG::JSObject("ACAPI")` с функциями для вызова из JS |
| `html_to_hpp.py` | Конвертирует `index.html` → `HTML_Pages.hpp` (C++11 raw string literal) |
| JSON Commands | `CommandBase` / `ReadOnlyCommand` / `ModifyCommand` для сложных операций |

### Требования к интерфейсу

**Без CSS:** Все стили — только `style="..."` атрибуты или JS `element.style.*`. Никаких внешних CSS файлов, никаких CSS-in-JS библиотек, никаких `<style>` тегов.

**Без React/Vite/Webpack:** Один чистый `index.html` с vanilla HTML5 + ES6 JavaScript.

**Без внешних шрифтов:** `font-family: system-ui, sans-serif`.

### Структура интерфейса (2 раздела)

#### Раздел 1. Отслеживание значений свойств (постоянный, сверху)

| Элемент | Описание |
|---------|----------|
| Выпадающий список свойств | Заполняется из `ACAPI.GetPropertyDefinitions()` (уже реализовано) |
| Поле «Текущее значение» | Показывает значение выбранного свойства для выделенного элемента |
| Кнопка «Обновить» | Перечитывает значение из выделенного элемента |

#### Раздел 2. Работа с описаниями (разворачиваемый, `<details>`)

| Элемент | Описание |
|---------|----------|
| Выпадающий список свойств | Тот же, что в разделе 1 |
| Текущее описание | `<textarea>` readonly, заполняется из `PROPERTYCACHE().property[...].definition.description` |
| **Кнопки команд** | Иконки/текст: `Sync_from`, `Sync_to`, `Renum`, `Sum`, `Renum_flag`, `Spec_rule` и т.д. |
| Редактор описания | `<textarea>` для ручной правки с подсветкой синтаксиса (опционально) |
| Кнопка «Проверить синтаксис» | Вызывает `ACAPI.ParsePropertyDescription(description)` |
| Кнопка «Записать в свойство» | Вызывает `ACAPI.SetPropertyDescription(name, desc)` |

### Вкладки редактора описаний

| Вкладка | Команды | Конструктор |
|---------|---------|-------------|
| **Синхронизация** | `Sync_from{...}`, `Sync_to{...}`, `Sync_from{Property:...}`, `Sync_from{description:...}`, `Sync_from{IFC:...}`, модификаторы `empty`/`trim_empty`/`def`, массивы `uniq`/`sum`/`max`/`min` | Форма: выбор типа команды → выпадающий список источника → поля параметров |
| **Нумерация** | `Renum_flag{...}`, `Renum{...}`, `NULL`/`SPACE`/`ALLNULL`/`n_NULL` | Конструктор: свойство-флаг + свойство-позиция + режим заполнения |
| **Суммирование** | `Sum{Property:...; Property:...}`, разделитель, `max`/`min` | Форма: суммируемое свойство → критерий → разделитель → режим (sum/max/min) |
| **Спецификации** | `Spec_rule`, `Spec_rule_v2`, `Spec_rule_v3`, группы `g(...)` и `s(...)` | Конструктор групп: поля U, P, F, Q для `g(...)`; поля Pn, Qn для `s(...)` |

### C++ API для вызова из JS (регистрируются в `RegisterACAPIJavaScriptObject`)

| JS-функция | C++ реализация | Тип |
|------------|----------------|-----|
| `ACAPI.GetPropertyDefinitions()` | Уже есть — возвращает массив имён свойств из `PROPERTYCACHE()` | read |
| `ACAPI.GetPropertyDescription(name)` | Найти в `PROPERTYCACHE().property` по имени, вернуть `definition.description` | read |
| `ACAPI.GetPropertyValue(name)` | Получить значение свойства для выделенного элемента (через `ACAPI_Element_GetPropertyValue`) | read |
| `ACAPI.ParsePropertyDescription(desc)` | Парсинг через `ReNum_GetElement` / `Sync_GetParamFromDescription` — возвращает `{ ok: true }` или `{ ok: false, error: "..." }` | read |
| `ACAPI.SetPropertyDescription(name, desc)` | Запись через `ACAPI_Property_ChangeProperty` или `ACAPI_Property_ChangePropertyDefinition` — **not verified**, проверить в LightRAG/SDK | modify |

### Изменения в проекте

| Файл | Изменение |
|------|-----------|
| `dialogs/index.html` | **Полностью переписать** — удалить React/Figma мусор, оставить чистый HTML5 + vanilla JS без CSS |
| `dialogs/BrowserPalette.cpp` | Добавить `DG::JSFunction` для `GetPropertyDescription`, `GetPropertyValue`, `ParsePropertyDescription`, `SetPropertyDescription` |
| `dialogs/BrowserPalette.hpp` | Объявление новых методов |

### Пример структуры index.html (стартовая точка)

```html
<!doctype html>
<html lang="ru">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>SomeStuff — Свойства</title>
  <!-- Нет CSS — все стили inline или через JS -->
</head>
<body style="font-family: system-ui, sans-serif; font-size: 13px; line-height: 1.4; color: #333;">
  
  <!-- Раздел 1: Отслеживание значений -->
  <div id="section-track">
    <h3 style="margin: 0 0 8px 0; font-size: 14px;">Отслеживание значений</h3>
    <select id="prop-track-select" style="width: 100%; padding: 4px; margin-bottom: 8px;"></select>
    <div id="prop-track-value" style="padding: 6px; border: 1px solid #ccc; min-height: 30px; font-family: monospace;"></div>
    <button onclick="refreshTrackValue()" style="margin-top: 8px; padding: 6px 12px;">Обновить</button>
  </div>
  
  <hr style="margin: 16px 0;">
  
  <!-- Раздел 2: Редактор описаний -->
  <details id="section-edit">
    <summary style="cursor: pointer; font-size: 14px; font-weight: bold; margin-bottom: 8px;">
      Редактор описаний
    </summary>
    
    <select id="prop-edit-select" style="width: 100%; padding: 4px; margin: 8px 0;"></select>
    
    <!-- Вкладки -->
    <div id="tabs" style="margin: 12px 0;">
      <button onclick="showTab('sync')" style="padding: 4px 8px; margin-right: 4px;">Синхронизация</button>
      <button onclick="showTab('renum')" style="padding: 4px 8px; margin-right: 4px;">Нумерация</button>
      <button onclick="showTab('sum')" style="padding: 4px 8px; margin-right: 4px;">Суммирование</button>
      <button onclick="showTab('spec')" style="padding: 4px 8px;">Спецификации</button>
    </div>
    
    <!-- Панели вкладок -->
    <div id="tab-sync" style="display: block; padding: 8px; border: 1px solid #ccc;">
      <!-- Конструктор команд синхронизации -->
    </div>
    <div id="tab-renum" style="display: none; padding: 8px; border: 1px solid #ccc;">
      <!-- Конструктор команд нумерации -->
    </div>
    <div id="tab-sum" style="display: none; padding: 8px; border: 1px solid #ccc;">
      <!-- Конструктор команд суммирования -->
    </div>
    <div id="tab-spec" style="display: none; padding: 8px; border: 1px solid #ccc;">
      <!-- Конструктор команд спецификации -->
    </div>
    
    <!-- Редактор -->
    <div style="margin: 12px 0;">
      <label style="display: block; margin-bottom: 4px;">Текущее описание:</label>
      <textarea id="prop-desc-current" readonly style="width: 100%; height: 60px; padding: 4px; font-family: monospace; font-size: 12px;"></textarea>
    </div>
    
    <div style="margin: 12px 0;">
      <label style="display: block; margin-bottom: 4px;">Новое описание:</label>
      <textarea id="prop-desc-new" style="width: 100%; height: 80px; padding: 4px; font-family: monospace; font-size: 12px;"></textarea>
    </div>
    
    <!-- Кнопки действий -->
    <div style="margin: 8px 0;">
      <button onclick="insertCommand()" style="padding: 6px 12px; margin-right: 8px;">Вставить команду</button>
      <button onclick="checkSyntax()" style="padding: 6px 12px; margin-right: 8px;">Проверить синтаксис</button>
      <button onclick="saveDescription()" style="padding: 6px 12px; background: #0066cc; color: white;">Записать в свойство</button>
    </div>
    
    <div id="status-message" style="margin-top: 8px; padding: 6px; font-family: monospace; font-size: 11px;"></div>
  </details>
  
  <script>
  // JS wrapper для вызова C++ функций
  async function callACAPI(fn, ...args) {
    return new Promise((resolve) => {
      window.ACAPI[fn](...args, resolve);
    });
  }
  
  // Инициализация при загрузке
  async function init() {
    const props = await callACAPI('GetPropertyDefinitions');
    fillSelect('prop-track-select', props);
    fillSelect('prop-edit-select', props);
  }
  
  function fillSelect(id, items) {
    const sel = document.getElementById(id);
    sel.innerHTML = items.map(p => `<option value="${p}">${p}</option>`).join('');
  }
  
  function showTab(tabId) {
    ['sync', 'renum', 'sum', 'spec'].forEach(t => {
      document.getElementById('tab-' + t).style.display = t === tabId ? 'block' : 'none';
    });
  }
  
  async function refreshTrackValue() {
    const prop = document.getElementById('prop-track-select').value;
    const val = await callACAPI('GetPropertyValue', prop);
    document.getElementById('prop-track-value').textContent = val || '(пусто)';
  }
  
  async function checkSyntax() {
    const desc = document.getElementById('prop-desc-new').value;
    const result = await callACAPI('ParsePropertyDescription', desc);
    const status = document.getElementById('status-message');
    status.style.color = result.ok ? '#2e7d32' : '#c62828';
    status.textContent = result.ok ? '✓ Синтаксис корректен' : '✗ Ошибка: ' + result.error;
  }
  
  async function saveDescription() {
    const prop = document.getElementById('prop-edit-select').value;
    const desc = document.getElementById('prop-desc-new').value;
    const result = await callACAPI('SetPropertyDescription', prop, desc);
    const status = document.getElementById('status-message');
    status.style.color = result.ok ? '#2e7d32' : '#c62828';
    status.textContent = result.ok ? '✓ Сохранено' : '✗ Ошибка: ' + result.error;
  }
  
  init();
  </script>
</body>
</html>
```

### Следующие шаги

1. Верифицировать `ACAPI_Property_ChangeProperty` / `ACAPI_Property_ChangePropertyDefinition` через LightRAG
2. Верифицировать `ACAPI_Element_GetPropertyValue` через LightRAG
3. Переписать `dialogs/index.html` (удалить React, вставить чистый HTML/JS)
4. Добавить новые `DG::JSFunction` в `BrowserPalette.cpp`
5. Запустить `html_to_hpp.py` → `HTML_Pages.hpp`
6. Собрать: `python Tools/BuildAddOn.py -c config.json -v 25`
7. Проверить в Archicad 25
