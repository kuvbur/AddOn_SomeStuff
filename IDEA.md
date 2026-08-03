## Интерфейс редактора описаний свойств

### Суть задачи

Создать веб-интерфейс для работы с описаниями свойств аддона SomeStuff. Интерфейс заменяет текущий React-заглушку на рабочий UI с двумя разделами: отслеживание значений свойств и редактор описаний с конструктором команд.

**HTML подгружается напрямую из файла:** `D:\SomeStuff_addon\Sources\AddOnResources\RFIX\HTML\Interface_ru.html` (не через `html_to_hpp.py` и `HTML_Pages.hpp`).

### Архитектура (существующая, не менять)

| Компонент                         | Назначение                                                                                                                                                             |
| --------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `BrowserPalette` (C++)            | `DG::Palette` + `DG::Browser`, загружает HTML напрямую из файла `D:\SomeStuff_addon\Sources\AddOnResources\RFIX\HTML\Interface_ru.html` через `DG::Browser::LoadURL()` |
| `RegisterACAPIJavaScriptObject()` | Регистрирует `DG::JSObject("ACAPI")` с функциями для вызова из JS                                                                                                      |
| JSON Commands                     | `CommandBase` / `ReadOnlyCommand` / `ModifyCommand` для сложных операций                                                                                               |

**Важно:** Не используется `html_to_hpp.py` и `HTML_Pages.hpp`. HTML загружается напрямую из файловой системы.

### Требования к интерфейсу

**Без CSS:** Все стили — только `style="..."` атрибуты или JS `element.style.*`. Никаких внешних CSS файлов, никаких CSS-in-JS библиотек, никаких `<style>` тегов.

**Без React/Vite/Webpack:** Один чистый `index.html` с vanilla HTML5 + ES6 JavaScript.

**Без внешних шрифтов:** `font-family: system-ui, sans-serif`.

### Структура интерфейса (2 раздела)

#### Раздел 1. Отслеживание значений свойств (постоянный, сверху)

| Элемент                   | Описание                                                          |
| ------------------------- | ----------------------------------------------------------------- |
| Выпадающий список свойств | Заполняется из `ACAPI.GetPropertyDefinitions()` (уже реализовано) |
| Поле «Текущее значение»   | Показывает значение выбранного свойства для выделенного элемента  |
| Кнопка «Обновить»         | Перечитывает значение из выделенного элемента                     |

#### Раздел 2. Работа с описаниями (разворачиваемый, `<details>`)

| Элемент                      | Описание                                                                                     |
| ---------------------------- | -------------------------------------------------------------------------------------------- |
| Выпадающий список свойств    | Тот же, что в разделе 1                                                                      |
| Текущее описание             | `<textarea>` readonly, заполняется из `PROPERTYCACHE().property[...].definition.description` |
| **Кнопки команд**            | Иконки/текст: `Sync_from`, `Sync_to`, `Renum`, `Sum`, `Renum_flag`, `Spec_rule` и т.д.       |
| Редактор описания            | `<textarea>` для ручной правки с подсветкой синтаксиса (опционально)                         |
| Кнопка «Проверить синтаксис» | Вызывает `ACAPI.ParsePropertyDescription(description)`                                       |
| Кнопка «Записать в свойство» | Вызывает `ACAPI.SetPropertyDescription(name, desc)`                                          |

### Вкладки редактора описаний

| Вкладка           | Команды                                                                                                                                                                                    | Конструктор                                                                |
| ----------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ | -------------------------------------------------------------------------- |
| **Синхронизация** | `Sync_from{...}`, `Sync_to{...}`, `Sync_from{Property:...}`, `Sync_from{description:...}`, `Sync_from{IFC:...}`, модификаторы `empty`/`trim_empty`/`def`, массивы `uniq`/`sum`/`max`/`min` | Форма: выбор типа команды → выпадающий список источника → поля параметров  |
| **Нумерация**     | `Renum_flag{...}`, `Renum{...}`, `NULL`/`SPACE`/`ALLNULL`/`n_NULL`                                                                                                                         | Конструктор: свойство-флаг + свойство-позиция + режим заполнения           |
| **Суммирование**  | `Sum{Property:...; Property:...}`, разделитель, `max`/`min`                                                                                                                                | Форма: суммируемое свойство → критерий → разделитель → режим (sum/max/min) |
| **Спецификации**  | `Spec_rule`, `Spec_rule_v2`, `Spec_rule_v3`, группы `g(...)` и `s(...)`                                                                                                                    | Конструктор групп: поля U, P, F, Q для `g(...)`; поля Pn, Qn для `s(...)`  |

### C++ API для вызова из JS (регистрируются в `RegisterACAPIJavaScriptObject`)

| JS-функция                                 | C++ реализация                                                                                                                          | Тип    |
| ------------------------------------------ | --------------------------------------------------------------------------------------------------------------------------------------- | ------ |
| `ACAPI.GetPropertyDefinitions()`           | Уже есть — возвращает массив имён свойств из `PROPERTYCACHE()`                                                                          | read   |
| `ACAPI.GetPropertyDescription(name)`       | Найти в `PROPERTYCACHE().property` по имени, вернуть `definition.description`                                                           | read   |
| `ACAPI.GetPropertyValue(name)`             | Получить значение свойства для выделенного элемента (через `ACAPI_Element_GetPropertyValue`)                                            | read   |
| `ACAPI.ParsePropertyDescription(desc)`     | Парсинг через `ReNum_GetElement` / `Sync_GetParamFromDescription` — возвращает `{ ok: true }` или `{ ok: false, error: "..." }`         | read   |
| `ACAPI.SetPropertyDescription(name, desc)` | Запись через `ACAPI_Property_ChangeProperty` или `ACAPI_Property_ChangePropertyDefinition` — **not verified**, проверить в LightRAG/SDK | modify |

### Изменения в проекте

| Файл                         | Изменение                                                                                                                        |
| ---------------------------- | -------------------------------------------------------------------------------------------------------------------------------- |
| `dialogs/BrowserPalette.cpp` | Добавить `DG::JSFunction` для `GetPropertyDescription`, `GetPropertyValue`, `ParsePropertyDescription`, `SetPropertyDescription` |
| `dialogs/BrowserPalette.hpp` | Объявление новых методов                                                                                                         |

### Пример структуры index.html (стартовая точка)

```html
<!doctype html>
<html lang="ru">
  <head>
    <meta charset="UTF-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1.0" />
    <title>SomeStuff — Свойства</title>
    <!-- Нет CSS — все стили inline или через JS -->
  </head>
  <body
    style="font-family: system-ui, sans-serif; font-size: 13px; line-height: 1.4; color: #333;"
  >
    <!-- Раздел 1: Отслеживание значений -->
    <div id="section-track">
      <h3 style="margin: 0 0 8px 0; font-size: 14px;">Отслеживание значений</h3>
      <select
        id="prop-track-select"
        style="width: 100%; padding: 4px; margin-bottom: 8px;"
      ></select>
      <div
        id="prop-track-value"
        style="padding: 6px; border: 1px solid #ccc; min-height: 30px; font-family: monospace;"
      ></div>
      <button
        onclick="refreshTrackValue()"
        style="margin-top: 8px; padding: 6px 12px;"
      >
        Обновить
      </button>
    </div>

    <hr style="margin: 16px 0;" />

    <!-- Раздел 2: Редактор описаний -->
    <details id="section-edit">
      <summary
        style="cursor: pointer; font-size: 14px; font-weight: bold; margin-bottom: 8px;"
      >
        Редактор описаний
      </summary>

      <select
        id="prop-edit-select"
        style="width: 100%; padding: 4px; margin: 8px 0;"
      ></select>

      <!-- Вкладки -->
      <div id="tabs" style="margin: 12px 0;">
        <button
          onclick="showTab('sync')"
          style="padding: 4px 8px; margin-right: 4px;"
        >
          Синхронизация
        </button>
        <button
          onclick="showTab('renum')"
          style="padding: 4px 8px; margin-right: 4px;"
        >
          Нумерация
        </button>
        <button
          onclick="showTab('sum')"
          style="padding: 4px 8px; margin-right: 4px;"
        >
          Суммирование
        </button>
        <button onclick="showTab('spec')" style="padding: 4px 8px;">
          Спецификации
        </button>
      </div>

      <!-- Панели вкладок -->
      <div
        id="tab-sync"
        style="display: block; padding: 8px; border: 1px solid #ccc;"
      >
        <!-- Конструктор команд синхронизации -->
      </div>
      <div
        id="tab-renum"
        style="display: none; padding: 8px; border: 1px solid #ccc;"
      >
        <!-- Конструктор команд нумерации -->
      </div>
      <div
        id="tab-sum"
        style="display: none; padding: 8px; border: 1px solid #ccc;"
      >
        <!-- Конструктор команд суммирования -->
      </div>
      <div
        id="tab-spec"
        style="display: none; padding: 8px; border: 1px solid #ccc;"
      >
        <!-- Конструктор команд спецификации -->
      </div>

      <!-- Редактор -->
      <div style="margin: 12px 0;">
        <label style="display: block; margin-bottom: 4px;"
          >Текущее описание:</label
        >
        <textarea
          id="prop-desc-current"
          readonly
          style="width: 100%; height: 60px; padding: 4px; font-family: monospace; font-size: 12px;"
        ></textarea>
      </div>

      <div style="margin: 12px 0;">
        <label style="display: block; margin-bottom: 4px;"
          >Новое описание:</label
        >
        <textarea
          id="prop-desc-new"
          style="width: 100%; height: 80px; padding: 4px; font-family: monospace; font-size: 12px;"
        ></textarea>
      </div>

      <!-- Кнопки действий -->
      <div style="margin: 8px 0;">
        <button
          onclick="insertCommand()"
          style="padding: 6px 12px; margin-right: 8px;"
        >
          Вставить команду
        </button>
        <button
          onclick="checkSyntax()"
          style="padding: 6px 12px; margin-right: 8px;"
        >
          Проверить синтаксис
        </button>
        <button
          onclick="saveDescription()"
          style="padding: 6px 12px; background: #0066cc; color: white;"
        >
          Записать в свойство
        </button>
      </div>

      <div
        id="status-message"
        style="margin-top: 8px; padding: 6px; font-family: monospace; font-size: 11px;"
      ></div>
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
        const props = await callACAPI("GetPropertyDefinitions");
        fillSelect("prop-track-select", props);
        fillSelect("prop-edit-select", props);
      }

      function fillSelect(id, items) {
        const sel = document.getElementById(id);
        sel.innerHTML = items
          .map((p) => `<option value="${p}">${p}</option>`)
          .join("");
      }

      function showTab(tabId) {
        ["sync", "renum", "sum", "spec"].forEach((t) => {
          document.getElementById("tab-" + t).style.display =
            t === tabId ? "block" : "none";
        });
      }

      async function refreshTrackValue() {
        const prop = document.getElementById("prop-track-select").value;
        const val = await callACAPI("GetPropertyValue", prop);
        document.getElementById("prop-track-value").textContent =
          val || "(пусто)";
      }

      async function checkSyntax() {
        const desc = document.getElementById("prop-desc-new").value;
        const result = await callACAPI("ParsePropertyDescription", desc);
        const status = document.getElementById("status-message");
        status.style.color = result.ok ? "#2e7d32" : "#c62828";
        status.textContent = result.ok
          ? "✓ Синтаксис корректен"
          : "✗ Ошибка: " + result.error;
      }

      async function saveDescription() {
        const prop = document.getElementById("prop-edit-select").value;
        const desc = document.getElementById("prop-desc-new").value;
        const result = await callACAPI("SetPropertyDescription", prop, desc);
        const status = document.getElementById("status-message");
        status.style.color = result.ok ? "#2e7d32" : "#c62828";
        status.textContent = result.ok
          ? "✓ Сохранено"
          : "✗ Ошибка: " + result.error;
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

---

## Тестирование аддона: процесс и планы (2026-08-03)

### Инфраструктура тестирования

| Компонент                       | Назначение                                                                                                                       |
| ------------------------------- | -------------------------------------------------------------------------------------------------------------------------------- |
| `TESTING` макрос                | Включает тестовый код при сборке; определён в конфигурации CMake                                                                 |
| `DBprnt(msg, val)`              | Вывод отладочной строки в `test_results.txt`                                                                                     |
| `DBtest(условие, msg)`          | Проверка логического условия                                                                                                     |
| `DBtest(a, b, msg)`             | Сравнение двух значений                                                                                                          |
| `TestFunc::Test()`              | Главная функция запуска всех тестов — вызывается в `CheckEnvironment()`                                                          |
| `test_results.txt`              | Файл с результатами тестов, создаётся в корне проекта                                                                            |
| `restart_archicad_for_test.ps1` | PowerShell скрипт: убивает ArchiCAD → собирает аддон → запускает с тестовым `.pln` → ждёт `test_results.txt` → выводит результат |

### Процесс тестирования (workflow)

1. **clang-format** — `clang-format -i Sources/AddOn/TestFunc.cpp`
2. **Сборка** — `python Tools/BuildAddOn.py -c config.json -v 25`
3. **Запуск** — `powershell -File "D:\SomeStuff_addon\Tools\restart_archicad_for_test.ps1"`
   Код Константа Значение
   0 $EXIT_SUCCESS ✅ Всё ок
   10 $EXIT_CONFIG_ERROR Нет config.json/BuildAddOn.py
   20 $EXIT_PREVIOUS_AC_FAILED Не удалось закрыть предыдущий ArchiCAD
   30 $EXIT_BUILD_FAILED ❌ Ошибка сборки C++
   40 $EXIT_AC_START_FAILED ArchiCAD не запустился
   50 $EXIT_RUNTIME_ERROR 💥 Access Violation/Segfault в C++
   60 $EXIT_TEST_TIMEOUT Таймаут test_results.txt
   70 $EXIT_TESTS_FAILED ⚠️ Код собран, логика неверна
   80 $EXIT_AC_SHUTDOWN_FAILED Не удалось завершить ArchiCAD
   90 $EXIT_CLEANUP_FAILED Не удалось удалить .lck
4. **Анализ** — ищем `=== ERROR IN TEST ===` в выводе скрипта

### Структура тестов (на 2026-08-03)

| Функция                                       | Что тестирует                                                | Статус                                     |
| --------------------------------------------- | ------------------------------------------------------------ | ------------------------------------------ |
| `TestFormatString()`                          | Парсинг и применение строк формата (.3m, .3mp, .03mm и т.д.) | ✅                                         |
| `TestCalc()`                                  | Математические выражения `<...>`                             | ✅                                         |
| `TestFormula()`                               | Чтение формул с GDL-ссылками `%ac_postWidth%`                | ✅                                         |
| `TestConvertToParamValue()`                   | Конвертация Int/Double/Bool/String → ParamValue              | ✅                                         |
| `TestConvertAttributeToParamValue()`          | Конвертация атрибутов → ParamValue                           | ✅                                         |
| `TestConvertPropertyToParamValue()`           | Конвертация свойств ArchiCAD → ParamValue                    | ✅                                         |
| `TestConvertPropertyDefinitionToParamValue()` | Конвертация определений свойств → ParamValue                 | ✅                                         |
| `TestSetParamValueSourseByName()`             | Определение типа параметра по rawName-префиксу               | ✅                                         |
| `TestSetrawNameFromProperty()`                | Формирование rawName из описания свойства                    | ✅                                         |
| `TestCheckIgnoreVal()`                        | Фильтрация значений (skip_empty, skip_trim_empty)            | ✅                                         |
| `TestReadProperty()`                          | Чтение свойств через ACAPI                                   | ✅                                         |
| `TestAddProperty()`                           | Добавление свойств в словарь                                 | ✅                                         |
| `TestPropertyHelpersToString()`               | Преобразование свойств в строку                              | ✅                                         |
| `TestStringSplt()`                            | Разделение строк (semicolon, trim, unicode)                  | ✅                                         |
| `TestName2Rawname()`                          | Преобразование имени → rawname                               | ❌ **отключён** (баг в Sync.cpp:1323-1326) |
| `TestSyncString()`                            | Парсинг базовых правил синхронизации                         | ✅                                         |
| `TestSyncStringRealRules()`                   | Реальные правила из BuildingInformation.xml                  | ✅                                         |
| `TestParsePrefixes()`                         | Константы префиксов и режимов синхронизации                  | ✅                                         |

### Планы по дальнейшим тестам

#### 1. Исправить `TestName2Rawname()`

**Корневая причина:** В `Name2Rawname` (Sync.cpp:1323-1326) сначала добавляется `BRACEEND` (`}`), потом `BRACESTART` (`{`). Из-за этого `"Property:TestProperty"` превращается в `"Property:TestProperty}{"` вместо `"{Property:TestProperty}"`. Строка становится непарсable.

**Исправление:** В Sync.cpp поменять порядок — сначала `BRACESTART`, потом `BRACEEND`.

**Пометка в OpenViking:** ✅ сохранено.

#### 2. Добавить тесты для `Name2Rawname` с уже обёрнутыми скобками

Пока не исправлен баг — добавить тесты, которые проверяют `Name2Rawname` с корректным входом `"{Property:TestProperty}"` (уже содержит скобки). Это временное покрытие до исправления функции.

#### 3. Добавить тесты для `ReNum_GetElement()`

Парсинг Renum-команд из описаний свойств:

- `Renum_flag{Sequence}{null;space;null}`
- `Renum{Renum_flag}{Renum_pole}{Renum_pole2}`
- Проверка корректного выделения `numtype` (NULL, SPACE, ALLNULL, n_NULL)
- Проверка восстановления исходного описания после извлечения Renum-команды (строка без Renum)

#### 4. Добавить тесты для `Sum_GetElement()`

Парсинг Sum-команд:

- `Sum{Property:prop1; Property:prop2}`
- `Sum{Property:prop1; Property:prop2; max}`
- `Sum{Property:prop1; Property:prop2; min}`
- С разделителем: `Sum{Property:prop1; Property:prop2; "; "; max}`

#### 5. Добавить тесты для `Spec_rule` парсинга

Парсинг спецификаций:

- `Spec_rule{g(...)}{s(...)}`
- `Spec_rule_v2{...}`
- `Spec_rule_v3{...}`
- Группы `g(U, P, F, Q)` и `s(Pn, Pv, Qn, Qv)`

#### 6. Интеграционные тесты: `SyncString` → `SetParamValueSourseByName`

Цепочка: `SyncString` парсит правило → результат передаётся в `SetParamValueSourseByName` для заполнения `ParamValue`. Проверить, что после обоих шагов все поля `param` корректны.

#### 7. Интеграционные тесты: чтение и запись свойств

- `ReadProperty()` с реальным GUID элемента (не `APINULLGuid`)
- `AddProperty()` с реальным элементом

**Зависимость:** Нужен открытый проект с хотя бы одним элементом, имеющим свойства. Тестовый `.pln` файл должен содержать такие элементы.

#### 8. Тесты на краевые случаи `SyncString`

| Сценарий                                          | Ожидание                       |
| ------------------------------------------------- | ------------------------------ |
| Пустая строка правила                             | `false`                        |
| Только направление, без содержимого `Sync_from{}` | `false`                        |
| Неизвестный префикс `Sync_from{Unknown:value}`    | `false`                        |
| Материал без кавычек `Sync_from{Material:Layers}` | `false` (требует `"..."`)      |
| Слэш в имени свойства `Property:Group/Name`       | `true`, `name == "Group/Name"` |
| Русские символы в `Property:Группа/Имя`           | `true`, `rawname` корректный   |

#### 9. Тесты `SyncString` с корректными флагами syncall/synccoord/syncclass

Для каждой комбинации типа данных проверить, что флаги работают правильно:

| Тип               | syncall | synccoord | syncclass | Ожидание |
| ----------------- | ------- | --------- | --------- | -------- |
| Property          | `false` | `false`   | `false`   | ❌       |
| Property          | `true`  | `false`   | `false`   | ✅       |
| Coord             | `false` | `false`   | `false`   | ❌       |
| Coord             | `false` | `true`    | `false`   | ✅       |
| Classification TO | `true`  | `false`   | `false`   | ❌       |
| Classification TO | `true`  | `false`   | `true`    | ✅       |
| GDL               | `false` | `false`   | `false`   | ❌       |
| GDL               | `true`  | `false`   | `false`   | ✅       |

#### 10. Расширить `TestSyncStringRealRules()` из BuildingInformation.xml

Добавить тесты на все правила из актуального файла `BuildingInformation.xml`, включая:

- Теплотехнические формулы с русскими названиями свойств
- Правила с экранированными слэшами `\/`
- Правила с русскими буквами в именах GDL-параметров (`%толщина%`)
- Material-правила с разными перьями и форматами
- Classification правила с `FullName`, `ItemId`, `Description`

### Известные баги (не тестовые)

| Баг                                    | Файл:Строка        | Описание                                                                         |
| -------------------------------------- | ------------------ | -------------------------------------------------------------------------------- |
| `Name2Rawname` порядок скобок          | Sync.cpp:1323-1326 | Сначала `}`, потом `{` — надо наоборот                                           |
| `GetFormatStringFromFormula` кавычки   | Helpers.cpp:?      | `stringformat.Contains('\"')` падает на строках с `".3m`                         |
| Путаница `SYNC_FROM_SUB`/`SYNC_TO_SUB` | TestFunc.cpp:?     | В `TestParsePrefixes` значения 4 и 3 перепутаны (косметика, константы корректны) |
