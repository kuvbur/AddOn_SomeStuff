# Current Task

## Task

#209: причина отсутствия наименования материала в JSON Spec (AC25). Защитный фильтр `GetAttributeValues` уже восстановлен к `fromAttribDefinition` (правка в рабочем дереве, не закоммичена); осталось воспроизвести потерю наименования и исправить подтверждённую причину.
https://github.com/kuvbur/AddOn_SomeStuff/issues/209

## Scope

Только путь происхождения и чтения свойства материала в `Sources/AddOn/Helpers.cpp`, связанные проверки и карточка `Docs/modules/Helpers.md`. Параллельные задачи не менять. AC25 — проверочная версия.

## Status

BLOCKED — фильтр восстановлен, clangd 0; исходная причина ещё не установлена. AC25 build падал на LNK1168/LNK1104 (занятые linker-файлы, параллельный Archicad), runtime не проводился. Issue #209 открыт.

## Last Completed

2026-09-25 — реорганизация состояния: закрыты #193/#199/#205/#206/#161, завершённые блоки перенесены в `IDEA_ARCHIVE.md`; после пользовательской проверки закрыты #194/#203/#204/#191. По #209: фильтр восстановлен, clang-format, clangd 0, карточка Helpers обновлена. Ранее 34/34 строки имели наименование лишь с ошибочно расширенным фильтром — исправлением #209 это не считается.

## Next Step

Собрать AC25 (linker-файлы свободны), воспроизвести #209 и проследить флаг через `ParseParamNameMaterial` → `SetParamValueFromCache`/`CompareParamDictValue` → `GetAttributeValues`. Исправить подтверждённую причину, повторить runtime, оформить checkpoint. Параллельные задачи не трогать.

## Last Checkpoint

`aa4a672` — `[#210] Вынести OtherDbDialog из Sync.cpp в dialogs/OtherDbDialog.cpp/.hpp` (последний коммит ветки; spec-checkpoint `50cae05` — предыдущий).

## Plan

- [x] #209: вернуть фильтр `GetAttributeValues` к `fromAttribDefinition` (clang-format, clangd 0).
- [/] Воспроизвести потерю наименования и проверить происхождение параметра материала.
- [ ] Исправить подтверждённую причину, проверить AC25, обновить документацию и issue.

## Decisions

- Ранее предложенная причина КЖ/Favorite не относится к тестовому проекту с правилом АР (уточнение пользователя).
- `GetAttributeValues` должен читать только `fromAttribDefinition`; расширение на `fromPropertyDefinition` — обход, а не исправление причины.

## Parallel Task — Name2Rawname (AC25)

### Scope
Только восстановление дополнения отсутствующей фигурной скобки в `Sources/AddOn/Sync.cpp::Name2Rawname`, регрессионные тесты `Sources/AddOn/TestFunc.cpp`, карточка `Docs/modules/Sync.md` и связанные generated docs. Незакоммиченные правки `Sync.cpp` и `Helpers.cpp` других задач сохранить. Пользователь разрешил выполнить без GitHub issue только для этой задачи.

### Status
BLOCKED_FOR_CHECKPOINT — `Name2Rawname` исправлена и локальные RED/GREEN проверки AC25 прошли; полный TESTING-набор не зелёный: после фикса в двух проходах 8 `ERROR IN TEST` в `Helpers.cpp::CompareParamValue` (классификации), отсутствовавших в сопоставимом прогоне с прежним условием (там 8 ошибок новых проверок скобок). После правильного ключа в `ParseSyncString` также остаётся `!subproperty.ContainsKey` для указанного свойства. Не утверждать, что весь сценарий синхронизации восстановлен. Issue не создавался по просьбе пользователя.

### Last Completed
Сравнены ревизии до `11add7a` и до `1625cf1`; RED на реальном AC25 (отсутствующая закрывающая скобка), затем GREEN для неполных/полных/отсутствующих скобок в VS «Отладка». `clang-format`, clangd Sync.cpp 0, финальная сборка `BuildAddOn.py -v 25` успешна. Чужие изменения Sync.cpp/Helpers.cpp сохранены; созданные в этой отладке breakpoint-ы удалены, две ранее существовавшие отключённые точки сохранены.

### Next Step
Разобрать отдельно появившиеся 8 ошибок классификации (`Helpers.cpp:1632-1634`) и отсутствие ключа в `subproperty` без изменения кода вне scope; после зелёного полного прогона решить вопрос checkpoint. `_generated/` не обновлён: сигнатуры и связи не менялись, а генератор обнуляет callgraph; точечное обновление строк — отдельный шаг.

### Last Checkpoint
Не создан: полный TESTING-набор не прошёл; посторонние незакоммиченные изменения Sync.cpp нельзя включать в checkpoint.

### Plan
- [x] Сравнить историю `Name2Rawname` и установить причину для неполного имени (по коду/Git).
- [x] Добавить и выполнить RED + GREEN тесты AC25 на двух недостающих скобках и контрольных формах.
- [x] Исправить условие, проверить clang-format/clangd/AC25 build, обновить карточки Sync/TestFunc.
- [/] Исследовать ошибки полного TESTING-набора и подтверждение сценария `subproperty`, прежде чем делать checkpoint.

## Parallel Task — #211 performance Helpers.cpp

### Scope
Только узкие места `Sources/AddOn/Helpers.cpp` из ревью производительности (профиль/материалы, поиск CSV, перечисления, повторный заголовок, формулы), связанные проверки и карточка `Docs/modules/Helpers.md`. Незакоммиченные правки #209 сохранить, не включать в checkpoint #211. Проверочная версия AC25; остальные версии 22–29 не проверены.

### Status
WAITING_FOR_TEST — по просьбе пользователя checkpoint #211 до проверки поведения: clang-format, clangd (0 диагностик), AC25 BuildAddOn.py и запуск тестового PLN успешны; JSON-тесты пропущены, C++ output/результаты и ускорение не проверены. #209 не включать в коммит.

### Last Completed
Правки пяти участков (профиль/материалы, CSV, перечисления, заголовок элемента, формулы); clang-format, clangd 0, сборка AC25 и запуск тестового PLN через runner успешны. Производительность и выходные значения в Archicad не замерены; #209 сохранён.

### Next Step
Сверить на тестовом PLN выход формул/CSV/перечислений и материалов профиля до/после, собрать C++-тесты из VS «Отладка» и замерить время; обновить generated docs отдельной задачей (текущий генератор обнуляет callgraph). Issue #211 остаётся открытым до проверки.

### Last Checkpoint
`ccf9cae` — `[#211] Сократить повторную работу Helpers` (только правки #211; изменения #209 в Helpers.cpp остались незакоммиченными).

### Plan
- [x] Проверить доступные контракты и исходные вызовы; LightRAG не ответил, использованы проектные определения и AC25 build.
- [x] Устранить повторные операции; clangd и AC25 build успешны, поведенческая проверка отложена пользователем.
- [/] Проверить diff, clang-format, clangd и AC25 build (выполнено); runtime сравнение и профилирование ожидаются.
- [x] Актуализировать карточку и выполнить checkpoint `ccf9cae` без #209 по запросу; generated docs обновить отдельно.

## Parallel Task — #213 JSON-команда SyncAll

### Scope
Только новые `Sources/AddOn/json_commands/SyncAllCommand.hpp/.cpp`, регистрация в `json_commands/JsonCommandRegistrar.cpp`, справка `json_commands/How JSON Commands work.md`, карточка `Docs/modules/json_commands.md`, `Docs/REPOMAP.md` и строка в `Reviews/open-2026-09-12.tracker.csv`. `Sources/AddOn/Sync.cpp` не менять (решение пользователя); незакоммиченные правки других задач (`IDEA.md`, `Sync.cpp`, `Helpers.cpp`, `TestFunc.cpp`, `Docs/modules/Sync.md`, `Docs/modules/TestFunc.md`) сохранить и в checkpoint не включать.

### Status
DONE (локально) — checkpoint `97377dc`, AC25 build и runtime endpoint-вызов пройдены; issue #213 открыт до проверки пользователем синхронизации.

### Last Completed
2026-09-27 — созданы `SyncAllCommand.hpp/.cpp` (обёртка: `LoadSyncSettingsFromPreferences(syncSettings, true)` → `PROPERTYCACHE().Update()` → `SyncAndMonAll`), регистрация в `RegisterJsonCommands`; clang-format, clangd 0 диагностик, `BuildAddOn.py -v 25` и `restart_archicad_for_test.ps1` (build=True, Archicad запущен); `API.ExecuteAddOnCommand` на порту 19723 вернул `{"status":"returned","elapsedSeconds":4.1818899}`.

### Next Step
Оформить checkpoint `[#213] JSON-команда SyncAll` (`Refs: #213`) без чужих правок и добавить строку в `Reviews/open-2026-09-12.tracker.csv`. Issue #213 закрывать после проверки пользователем.

### Last Checkpoint
`97377dc` — `[#213] JSON-команда SyncAll` (только файлы #213; чужие правки остались незакоммиченными).

### Plan
- [x] Согласовать объём и контракт (issue #213).
- [x] Создать команду и зарегистрировать её; clang-format, clangd, AC25 build.
- [x] Runtime: вызов endpoint на запущенном AC25 (`status=returned`).
- [x] Обновить справку, карточку `Docs/modules/json_commands.md` и `Docs/REPOMAP.md`.
- [x] Checkpoint `97377dc` (`Refs: #213`).
- [x] Строка `#213` в `Reviews/open-2026-09-12.tracker.csv` (локальный трекер, вне git).

### Decisions
- Команда вызывает только `SyncAndMonAll`: `DimRoundAll`, `WriteSyncSettingsToPreferences` и обновление меню из пункта меню `SyncAll_CommandID` не воспроизводятся.
- `Sync.cpp` не правится — `SyncAndMonAll` остаётся `void`, прогресс-окно и фазы сохраняются; отсюда контракт ответа только `{status, elapsedSeconds}` (согласовано после уточнения о недоказуемости `skippedByReset`/`elementsToWrite` без правки `Sync.cpp`).
- `LoadSyncSettingsFromPreferences(syncSettings, true)` — у внешнего вызова нет интерфейса для смены флагов обхода; `PROPERTYCACHE().Update()` — паритет с `MenuCommandHandler`.

## Parallel Task — #214 перенос наработок из официального шаблона сборки

### Scope

Только `Tools/CMakeCommon.cmake`, новые `Tools/VersionInfo.rc.in` / `Tools/AddOn.rc.in`, `CMakeLists.txt`, `config.json`, `Tools/AddOn.grc.in`. Кода в `Sources/AddOn/` не трогать. Локализация JSON→grc+XLIFF и code signing macOS исключены по решению пользователя. Пункты 1–7 issue — каждый отдельным шагом и коммитом с `Refs: #214`.
https://github.com/kuvbur/AddOn_SomeStuff/issues/214

### Status

IN_PROGRESS — пункт 1 выполнен (AC25 build + проверенная версия в grc и .apx). Пункты 2–7 не начаты. mac-сборка нужна для пунктов 2 и 7.

### Last Completed

2026-09-27 — пункт 1: `version`/`description`/`copyright` в `config.json` (версия числом `1.78`, без `v`), `parse_version` + `ReadConfigJson` в `CMakeCommon.cmake`, экспорт `ADDON_VERSION` двумя каналами, хардкод `v1.78` убран. AC25 build успешен, версия `1.78 2026-09-27-15` подтверждена в `RINT/AddOn.grc` и в `.apx`, compile DB регенерирован.

### Next Step

Пункт 2: `Tools/VersionInfo.rc.in` + `Tools/AddOn.rc.in`, подключение к target, `AC_ADDON_FOR_DISTRIBUTION`. Перед правкой `-r/--release` согласовать семантику «метка для дистрибуции» (сейчас = RelWithDebInfo + все языки).

### Last Checkpoint

Не создан — задача начата, кода не менялось.

### Plan

- [x] Сравнить форк `Tools/` с upstream, зафиксировать расхождения (скилл + issue #214).
- [x] Пункт 1: версия и метаданные из `config.json` (AC25 build, версия в grc и .apx проверена).
- [ ] Пункт 2: `VersionInfo.rc` + `AddOn.rc` на Windows, `AC_ADDON_FOR_DISTRIBUTION`.
- [ ] Пункт 3: глоб `*.hpp` / `*.c` в target.
- [ ] Пункт 4: `SYSTEM` для `${devKitDir}/Inc` и `ModuleFolders`.
- [ ] Пункт 5: `find_package` с `REQUIRED` и границей версии Python.
- [ ] Пункт 6: `verify_api_devkit_folder` (не заменяя `DetectACVersion`).
- [ ] Пункт 7: `-fno-constant-cfstrings`, мёртвый `MINIMUM_SYSTEM_VERSION`, `addOnNameIdentifier`.

### Decisions

- `Tools/` — форк upstream, сплошная синхронизация запрещена: сломает AC22–24, `-DTESTING` (`TestFunc.cpp`/`DBprnt`), `/W3`, маркеры `AI_STATUS` раннера и `-DAC_${acVersion}` (цепочка `api_headers/APICommon<N>.h`).
- Локализация JSON→grc+XLIFF и code signing macOS не берутся по решению пользователя; глоб `*.json` из исходников тоже не берём.
- Переход `/W3` → `/W4` вынесен из issue: слишком рискован при `/WX`, отдельный follow-up.
- `-r/--release` (`RelWithDebInfo` + все языки) не переименовываем в «метку для дистрибуции» молча — семантика решается при пункте 2.
- Версия — число без `v`: upstream `parse_version` принимает только `^([0-9]+)(\.[0-9]+){0,2}$`, а каждая компонента 0–65535 попадает в `FILEVERSION`.
- Нужны **оба** канала `ADDON_VERSION`: CMake-переменная для `configure_file` → `RINT/AddOn.grc` (компилятор туда не подставится) и `target_compile_definitions` для C++.

## WAITING_FOR_TEST — проверки за пользователем

- #189 `2213388`: инлайн-кнопка сброса убрана из строк «Монитора»; в строке осталась только кнопка закрепления.
- Ревизия Sync `9ae0138`: GUI диалога другой базы (заголовок/подписи из ресурсов), реальный `Sync_to_GUID` и откат на PLN — по потребности; AC22–29 не проверены.

## Backlog — открытые issues

- Ревизия Sync, следующие шаги: #201 (MonByType считает ошибку успешным обходом), #202 (File-правила принимают мусор после числа). #200 закрыт (`02f84c9`: RunParam восстанавливает текущую БД после скриптов элемента и маркера; runtime на двух БД — за пользователем).
- Roombook: #195 (рефакторинг — код готов, закрывать после пользовательской валидации), #198 (замер/оптимизация — после проверки #195), #164 (сверка результата Roombook с прежним на PLN).
- Монитор/палитра: #158 (кэш правил, не завершён — повторный замер после кэша не получен), #159 (маркировка — код готов, визуал за пользователем), #186 (адаптивная палитра), #187 («Автоформат» описания), #181 (placeholder фильтра), #180/#179 (код готов, визуал за пользователем), #178 (сужение палитры, визуал за пользователем), #177/#176 (ведомости Navigator/TableRenderer).
- Runtime-набор R2–R10: #163–#171 (включая #169 R8 Teamwork, #170 R9 сборки AC22–24/26–29).
- Прочее: #156 (классификатор видов работ), #155 (быстрый сброс свойств), #149/#148/#147 (откосы), #143/#142/#141 (чтение атрибутов), #136 (геометрия перекрытий), #130/#129/#128 (фильтры/поиск), #115 (координаты окон/дверей).
- #157 — пресеты фильтров реализованы (issue остался открытым по недосмотру; кандидат на закрытие после проверки палитры).


- **CMake `string(REPLACE)` требует 4 аргумента** (match, replace, output, **input**) — с тремя `requires at least four arguments`. Проверено на `"%Y"`.
- **Вложенный путь JSON в списке CMake разъезжается**: `set (a copyright\;name)` даёт 2 элемента, а не 1 — вложенные поля читать отдельными вызовами `string(JSON ... GET "${json}" copyright name)`.
## Грабли

- **HTML вшит в ресурс** (`'DATA' ID_ADDON_HTML`) — правки `Interface_ru.html` требуют пересборки; после каждой правки `Tools/test_html.ps1`.
- **Дубли строк классификации**: C++ кладёт словарь и под `systemname`, и под `systemname_full` → записи ×2 (JS-дедуп `uniqueClassificationOptions`).
- **Ранний `return` по `data.common`** прятал выбор классификации и «Применить ко всем» при единой классификации/одном элементе.
- **prefs проекта ломают Teamwork** — настройки только в локальный `…/GRAPHISOFT/SomeStuff/SyncSettings.dat`; `ReadSyncSettingsFromFile` отвергает чужую `PreferencesVersion` → новый массив в настройках требует bump версии.
- **Undo-области на каждый элемент** (ResetProperty.cpp) — сотни undo-шагов; док DevKit требует одну область на действие.
- **Ключи кэша параметров всегда в нижнем регистре** (prefix + ToLowerCase + BRACEEND) — имя без нормализации регистра = правило молча не срабатывает.
- **Мост = инлайн-функции `RegisterACAPIJavaScriptObject`**; аргументы `JSFunction` парсить через `DynamicCast<JSValue>` — `DynamicCast<JSArray>` ронял ArchiCAD.
- **Подсветка**: `APIIo_HighlightElementsID` + `APIDo_ZoomToElementsID` триггерят SelectionChangeHandler → `static suppressSelectionRefresh`. `SetElementHighlight` только AC26+/27+; AC25 — прямой `ACAPI_Interface`, сброс = вызов без par1, Clear перед Set.
- **Данные «Монитора» — только из `PROPERTYCACHE()`**, значения — по элементам отдельно; в кэше `property` — только определения.
- **`gh` CLI не в PATH** — `export PATH="$PATH:/c/Program Files/GitHub CLI"`; Reviews/ в .gitignore (BOM+LF, править через python `utf-8-sig`).
- **Снято автором (не фиксить)**: `Dimensions.cpp:158` `pen_original`; пересоздание элементов отделки в Roombook; `Sync.cpp:419-428` накопительный `epm`.
