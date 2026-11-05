# Утилиты SomeStuff / SomeStuff Utilities

## Русский

В папке находятся исходники двух утилит для аддона SomeStuff.
Для работы используются готовые Windows-приложения в папке `dist`:
`roombook_setup.exe` и `somestuff_updater.exe`. Запуск — двойным щелчком по EXE;
отдельно устанавливать Python не требуется.

### roombook_setup — проверка и настройка проекта для отделки

**Исходник:** `roombook_setup.py`. **Приложение:** `dist/roombook_setup.exe`.

Помогает проверить готовность открытого проекта Archicad к функции отделки
Roombook и связать существующие свойства проекта с назначениями отделки.

**Как работает:**

- Находит запущенные экземпляры Archicad и позволяет выбрать проект для проверки.
- Через локальное JSON-соединение проверяет доступность SomeStuff, избранное
  отделки, маркеры в описаниях свойств и элементы классификации.
- Показывает отчёт: что найдено, чего не хватает и что не удалось проверить.
  В частности, проверка комбинации слоёв пока не реализована, а ненайденное
  избранное помечается как неподтверждённое, а не как заведомо отсутствующее.
- В разделе свойств позволяет выбрать существующие свойства проекта и добавить
  в их описания понятный текст с маркером `some_stuff_fin_*`, по которому их
  распознаёт аддон. Сохраняет блок форматирования `{...}`; замена другого
  непустого описания требует включения опции перезаписи.

Для работы нужен открытый проект Archicad с доступным JSON-соединением;
проверка избранного и изменение описаний требуют соответствующих команд SomeStuff.
Сама проверка проект не изменяет. Применение описаний — отдельное действие.
Создание недостающих элементов классификации предусмотрено отдельно в консольном
режиме исходного скрипта, а не кнопкой в графическом интерфейсе.
Утилита не строит отделку и не создаёт свойства, избранное или комбинации слоёв.

### somestuff_updater — обновление установленного аддона

**Исходник:** `somestuff_updater.py`. **Приложение:** `dist/somestuff_updater.exe`.

Помогает найти установленный SomeStuff и заменить его новой сборкой
для выбранной версии Archicad на Windows.

**Как работает:**

- Находит установки Archicad и аддон в стандартных папках, включая вложенные,
  а также по пользовательским путям из реестра. Если найдено несколько копий
  аддона, сообщает о конфликте.
- Читает сведения об установленной сборке и сравнивает их с каналом GitHub:
  **Release** — релизные версии, **Test** — тестовые сборки.
- По команде пользователя ждёт закрытия выбранной установки Archicad,
  скачивает подходящий архив и извлекает `SomeStuff.apx`.
- Перед заменой переименовывает прежний файл в резервную копию
  `SomeStuff.apx._old_version_...`. При ошибке копирования пытается вернуть
  старую версию и показывает результат в журнале.

Для проверки и загрузки нужен доступ к GitHub, для установки — права записи
в папку аддона. Утилита не закрывает Archicad принудительно и не редактирует
проект. Это обновление существующей установки SomeStuff, а не первичная установка.

---

## English

This folder contains the source code for two SomeStuff utilities.
Use the ready-made Windows applications in `dist`:
`roombook_setup.exe` and `somestuff_updater.exe`. Double-click the EXE to start;
a separate Python installation is not required.

### roombook_setup — project checks and setup for room finishes

**Source:** `roombook_setup.py`. **Application:** `dist/roombook_setup.exe`.

Helps check whether an open Archicad project is ready for Roombook finishing
and associate existing project properties with their finishing roles.

**How it works:**

- Finds running Archicad instances and lets you select a project to check.
- Uses a local JSON connection to check SomeStuff availability, finishing
  favorites, markers in property descriptions, and classification items.
- Reports what was found, what is missing, and what could not be checked.
  In particular, the layer-combination check is not implemented yet, and
  favorites that are not found are marked as unconfirmed rather than
  definitely missing.
- In the properties section, lets you select existing project properties and
  add readable descriptions containing the `some_stuff_fin_*` markers used by
  the add-on. Preserves the `{...}` formatting block; replacing a different
  non-empty description requires enabling the overwrite option.

Requires an open Archicad project with an available JSON connection;
favorite checks and description changes require the corresponding SomeStuff
commands. Checking alone does not modify the project. Applying descriptions
is a separate action. Creating missing classification items is available
separately in the source script's console mode, not through a GUI button.
The utility does not generate finishes or create properties, favorites,
or layer combinations.

### somestuff_updater — updating the installed add-on

**Source:** `somestuff_updater.py`. **Application:** `dist/somestuff_updater.exe`.

Helps locate an installed copy of SomeStuff and replace it with a newer build
for the selected Archicad version on Windows.

**How it works:**

- Finds Archicad installations and the add-on in standard folders, including
  subfolders, and through custom paths stored in the registry. Reports a
  conflict if multiple copies of the add-on are found.
- Reads the installed build information and compares it with the selected
  GitHub channel: **Release** for release versions or **Test** for test builds.
- When the user starts an update, waits for the selected Archicad installation
  to close, downloads the matching archive, and extracts `SomeStuff.apx`.
- Renames the previous file to a backup named
  `SomeStuff.apx._old_version_...` before replacement. If copying fails,
  attempts to restore the previous version and reports the result in the log.

Checking and downloading require access to GitHub; installation requires
write permission to the add-on folder. The utility does not forcibly close
Archicad or edit the project. It updates an existing SomeStuff installation
rather than performing a first-time installation.
