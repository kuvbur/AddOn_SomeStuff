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
В папке `D:\SomeStuff_addon\Sources\AddOn\Commands\` уже созданы:
- `CommandBase.hpp/cpp` — базовый класс (полностью подходит, повторяет структуру tapir)
- `ExampleCommands.hpp/cpp` — примеры реализации
- Готовая структура: `ReadOnlyCommand` и `ModifyCommand`

### Следующие шаги
1. Создать `ParsePropertyCommand` для парсинга команд Renum из описания свойств
2. Использовать готовую структуру из `Commands/`
3. Реализовать логику парсинга (используя код из `ReNum_GetElement`)
4. Зарегистрировать команду в `SomeStuff_Main.cpp`
5. Протестировать через Python интерфейс

### Python интерфейс (Часть 1) — СОЗДАН
Создан файл `D:\SomeStuff_addon\property_bridge.py`:
- HTTP сервер на базе `ThreadingHTTPServer` (по аналогии с `archixml.py`)
- HTML/JS интерфейс встроен в код
- Кнопка "Получить свойства" и выпадающий список
- Вызов к Archicad JSON API через `ac_post()`
- Инструкции в `property_bridge_README.md`

**Статус:** Python скрипт готов, синтаксис проверен. Ожидает регистрации команды `GetPropertyDefinitions` в аддоне.

### Полезные пути для поиска
- `D:\SomeStuff_addon\Code_Example\` — примеры реализации (tapir)
- `D:\SomeStuff_addon\Sources\AddOn\Commands\` — готовая база для JSON команд
- `D:\SomeStuff_addon\Sources\AddOn\ReNum.cpp` — логика парсинга Renum
- `D:\SomeStuff_addon\Sources\AddOn\Constants.hpp` — константы (RENUMFLAG, RENUM и др.)
- `D:\SomeStuff_addon\wiki\en\Element-Renumbering-en.md` — документация по Renum
- `D:\SomeStuff_addon\property_bridge.py` — Python интерфейс (НОВОЕ)
- `D:\SomeStuff_addon\property_bridge_README.md` — инструкции по запуску (НОВОЕ)
