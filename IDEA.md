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

## Parallel Task — диагностика ReadQuantities при SpecAll (AC25)

### Scope
Только отладка предупреждения для балки `8EBEA526-944C-4678-AB84-38829E7B9379`; код не менять, остальные задачи не затрагивать.

### Status
IN_PROGRESS (история пути ReadMaterial) — установлено, когда код начал проверять флаги; причина нулей именно в полученных данных/условиях их формирования не доказана. Исправление не выполнялось.

### Last Completed
При вызове JSON Spec через VS Debug остановка на GUID балки: `API_ProfileStructure`, `pll.num=1`, индексы материала `43=43`, `pll.structype=2` (Core из `ProfileItem::IsCore`), `pdd.structype=0` (из `API_CompositeQuantity.flags`); все три компонента количества имеют `flags=0`, все три профильных — `structype=2`. `ACAPI_Element_GetMoreQuantities` вернул `NoError`. Путь по коду: `ParamDictRead` при `param.fromQuantity` выставляет `hasQuantity` (:5513–5536), `ReadMaterial` (`Helpers.cpp:7475–7492`) → `Components` → `ComponentsProfileStructure` (:9069–9084) → `ReadQuantities` (:6732, :6784, :6852, :6907). Коммит `1cab197` от 2026-03-17 добавил присвоение `structype=flags` и сравнение с профилем; до него в соответствующей ветке при добавлении слоя ставился `APICWallComp_Core`, а сравнения не было. Поэтому новая проверка показывает расхождение, которого старый путь не проверял; когда именно данные стали нулевыми, по Git не установлено. На :6975–6976 выбирается запасной расчёт, `APIERR_GENERAL` подставлен логированием; доказательств, что это прерывает SpecAll, нет. Созданные брейкпоинты удалены, отладка остановлена.

### Next Step
Если нужна именно причина изменения входных флагов: сравнить тот же GUID и профиль в архивном PLN/предыдущем работающем окружении; проверить до/после `ReadMaterial` значения и условия формирования без предположения об ошибке SDK. Для изменения кода — отдельная задача/issue и регрессионная проверка результатов быстрой и запасной веток.

### Last Checkpoint
Нет: исходный код не изменялся; запись расследования в IDEA.md не коммитить вместе с чужими правками.

### Plan
- [x] Воспроизвести конкретное предупреждение в AC25 под отладчиком.
- [x] Сопоставить путь ReadMaterial и ревизию до `1cab197`.
- [x] Удалить свои брейкпоинты и завершить сеанс отладки.
- [ ] Установить первопричину нулевых входных флагов сравнением с работающим PLN/окружением; не приписывать её SDK без доказательства.

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

## Parallel Task — #218 перенос наработок официального шаблона сборки (пункты 2–7)

### Scope

Только `Tools/CMakeCommon.cmake`, `Tools/VersionInfo.rc.in` (новый), `Tools/AddOn.rc.in` (новый), `Tools/BuildAddOn.py`, `Tools/AddOn.plist.in`, `CMakeLists.txt`. Кода в `Sources/AddOn/` не трогать. Проверено на AC25/AC26 (Windows).

https://github.com/kuvbur/AddOn_SomeStuff/issues/218

### Status

DONE (код) — все шесть пунктов реализованы, сборки AC25/AC26 и `--release` зелёные, checkpoint `d815152`. Не проверено: macOS-сборка (пункты 2 и 7 меняют bundle) и AC22–24.

### Last Completed

2026-09-27 — все пункты 2–7 выполнены отдельными правками с общей проверкой.

- **Пункт 2** (`VersionInfo.rc.in` + `AddOn.rc.in`, `AC_ADDON_FOR_DISTRIBUTION`): портированы из upstream, подключены через новую функцию `generate_add_on_version_info` (`CMakeCommon.cmake:260`). `FILEVERSION` из `config.json`, `Translation` 0x0409/0x04b0 (константа, TODO на таблицу языков), `gsBuildNum=0` на Windows (источник — только Info.plist фреймворка, как FIXME в upstream). В `AddOn.rc.in` длина `STRS 18000` считается из кода языка, а не константа `4L` из upstream. `-r/--release` теперь передаёт `-DAC_ADDON_FOR_DISTRIBUTION=ON` (`BuildAddOn.py:333`, решение владельца).
- **Пункт 3**: глоб `*.hpp` добавлен; глоб `*.c` **не добавлен намеренно** — под `Sources/AddOn/api_headers/` лежат все восемь `APICommon22.c`…`APICommon29.c` с одинаковыми символами, глоб дал бы LNK2005/LNK2019. Проверено: `APICommon*.c` в vcxproj 0 вхождений.
- **Пункт 4**: `SYSTEM PUBLIC` для `${devKitDir}/Inc` и `ModuleFolders`.
- **Пункт 5**: `find_package (Python3 3.8 REQUIRED COMPONENTS Interpreter)` + обе ссылки на `${Python_EXECUTABLE}` → `${Python3_EXECUTABLE}` (решение владельца: 3.8 = минимум CI).
- **Пункт 6**: `verify_api_devkit_folder` портирована, вызывается в `CMakeLists.txt:24` **до** `DetectACVersion` (не заменяя её).
- **Пункт 7**: `-fno-constant-cfstrings` возвращён; `AddOn.plist.in` переведён на `@MINIMUM_SYSTEM_VERSION@` и `@bundleIdentifier@`; `addOnNameIdentifier` убран, идентификатор считается как `com.graphisoft.addon.${addOnName}` (решение владельца — идентичность аддона не меняем).

**Попутно исправлен скрытый баг из пункта 1 #214:** `addOnCompanyName`/`addOnCopyrightYear` в `ReadConfigJson` не экспортировались через `PARENT_SCOPE`, поэтому первый VERSIONINFO собрался с пустым `CompanyName` и обрезанным `LegalCopyright`. Добавлен `PARENT_SCOPE`.

### Next Step

mac-сборка AC25 (или любой версии) для пунктов 2 и 7: без неё не подтверждены `Info.plist` (`LSMinimumSystemVersion` из DevKit вместо 10.15), bundle identifier и `Translation` для не-INT языков. Затем закрыть #218.

### Last Checkpoint

`d815152` — `[#218] Сборка: VersionInfo.rc, глоб .hpp, SYSTEM-инклуды, Python3 REQUIRED, проверка DevKit` (только файлы #218; правки #217 и `Spec.cpp` остались незакоммиченными).

### Plan

- [x] Пункт 2: `VersionInfo.rc` + `AddOn.rc` + `AC_ADDON_FOR_DISTRIBUTION` + семантика `--release`.
- [x] Пункт 3: глоб `*.hpp` (без `*.c` — см. обоснование).
- [x] Пункт 4: `SYSTEM` для DevKit-инклудов.
- [x] Пункт 5: `Python3 REQUIRED` + граница 3.8.
- [x] Пункт 6: `verify_api_devkit_folder` (проверена негативным тестом: неверный путь → `CMake Error ... does not exist` до `DetectACVersion`).
- [x] Пункт 7: `-fno-constant-cfstrings`, `MINIMUM_SYSTEM_VERSION`, bundle identifier.
- [x] Сборки: AC25 Debug (раннер exit 0, build=True, archicad=running), AC25 RelWithDebInfo `--release` (`IsPrivateBuild: False`), AC26 Debug (регрессия).
- [x] mac-сборка AC25 — требуется перед закрытием issue; закрыть #218.

### Decisions

- Глоб `*.c` не берём: `api_headers/APICommon<N>.c` — восемь версий одного файла, версия выбирается `-DAC_${acVersion}` через `api_headers/APIEnvir.h`. В target они попадать не должны.
- `--release` = релиз для дистрибуции: снимает `PrivateBuild` из VERSIONINFO. CI уже пакует этот вариант, отдельный флаг не заводили.
- Идентификатор bundle остаётся `com.graphisoft.addon.SomeStuff` — он уже зашит в plist и в `CFBundleIdentifier`; менять идентичность опубликованного аддона владелец не стал. Считается в CMake, подставляется в plist, чтобы значения не разъезжались.
- `ADDON_VERSION` не переопределяется результатом `generate_add_on_version_info`: там три компоненты (`1.78.0` из `1;78;0`), а в UI версия как в `config.json` (`1.78`). Три компонента нужны только `FILEVERSION`. Проверено: в grc осталось `1.78`.
- Таблица `Translation` — константа с пометкой TODO: `config.json` объявляет только `INT`. Upstream берёт её из `GSLocalization.h` через `LocalizationMappingTable.py` + `AC_WIN_LANGCHARSET` из `BuildAddOn.py` — это нужно при добавлении языков.
- `/W3` → `/W4` остаётся вне объёма (риск при `/WX`).

### Validation

- Verified: `SomeStuff-VersionInfo.rc` и `SomeStuff-AddOn.rc` генерируются и попадают в vcxproj как `ResourceCompile`; `CompanyName=kuvbur`, `FileVersion=1.78.0`, `LegalCopyright=Copyright © kuvbur, 1984-2026`; оба `.res` собраны rc.exe; `Translation` 0x0409/0x04b0.
- Compiled: да — AC25 Debug (`restart_archicad_for_test.ps1`, exit 0, build=True, archicad=running), AC25 RelWithDebInfo, AC26 Debug.
- Tested: runtime — аддон загружен в AC25, но поведение функций не проверялось (задача сборочная); mac-сборка и AC22–24 — `not verified`.

## WAITING_FOR_TEST — проверки за пользователем

- #189 `2213388`: инлайн-кнопка сброса убрана из строк «Монитора»; в строке осталась только кнопка закрепления.
- Ревизия Sync `9ae0138`: GUI диалога другой базы (заголовок/подписи из ресурсов), реальный `Sync_to_GUID` и откат на PLN — по потребности; AC22–29 не проверены.

## Backlog — открытые issues

- Ревизия Sync, следующие шаги: #201 (MonByType считает ошибку успешным обходом), #202 (File-правила принимают мусор после числа). #200 закрыт (`02f84c9`: RunParam восстанавливает текущую БД после скриптов элемента и маркера; runtime на двух БД — за пользователем).
- Roombook: #195 (рефакторинг — код готов, закрывать после пользовательской валидации), #198 (замер/оптимизация — после проверки #195), #164 (сверка результата Roombook с прежним на PLN).
- Монитор/палитра: #158 (кэш правил, не завершён — повторный замер после кэша не получен), #159 (маркировка — код готов, визуал за пользователем), #186 (адаптивная палитра), #187 («Автоформат» описания), #181 (placeholder фильтра), #180/#179 (код готов, визуал за пользователем), #178 (сужение палитры, визуал за пользователем), #177/#176 (ведомости Navigator/TableRenderer).
- Runtime-набор R2–R10: #163–#171 (включая #169 R8 Teamwork). #170 (R9: сборки AC25–29) закрыт — см. IDEA_ARCHIVE.md.
- Прочее: #156 (классификатор видов работ), #155 (быстрый сброс свойств), #149/#148/#147 (откосы), #143/#142/#141 (чтение атрибутов), #136 (геометрия перекрытий), #130/#129/#128 (фильтры/поиск), #115 (координаты окон/дверей).
- #157 — пресеты фильтров реализованы (issue остался открытым по недосмотру; кандидат на закрытие после проверки палитры).
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

## Parallel Task — #217 счётчики кэшей EvalExpression

### Scope
Только `Sources/AddOn/CommonFunction.cpp` (`EvalExpression`: инкременты и вызов сводки перед `Clear`), `Sources/AddOn/Propertycache.hpp/.cpp` (`FormulaCacheStats`, поле в `PropertyCache`, `ReportFormulaCacheStats`), карточка `Docs/modules/Propertycache.md`. Без JSON-команд — по решению пользователя сводка печатается через `DBprnt` перед очисткой. Правки `IDEA.md` и остальных файлов других задач не включать в checkpoint.

https://github.com/kuvbur/AddOn_SomeStuff/issues/217

### Status
BLOCKED_FOR_LINK — код готов, `clang-format` выполнен, LSP 0 ошибок по `Propertycache.cpp`; компиляция всех файлов прошла при `/WX` (мои строки найдены в `Propertycache.obj`/`CommonFunction.obj` как UTF-8). Линковка не выполнена: `LNK1168` — `Build/SomeStuff/25/Debug/SomeStuff.apx` держит активная debug-сессия VS (остановлена на брейкпоинте в `Spec::GetParamToReadFromRule`, `Spec.cpp:1308`), файл подтверждённо заблокирован (Permission denied на открытие). Счётчики вживую не наблюдались.

### Last Completed
2026-09-27 — issue #217 создан. `FormulaCacheStats` (5 счётчиков `UInt64`) в `Propertycache.hpp:115-129`, поле `mutable formulaCacheStats` в `PropertyCache` (`:224-230`) с явным комментарием, что счётчики не сбрасываются ни в конструкторе, ни в `Update()` — нужна картина за всю сессию. `ReportFormulaCacheStats` (`Propertycache.cpp:47-66`) печатает вызовы, попадания обоих уровней с процентами и число очисток по переполнению; молчит при `calls == 0`. Инкременты в `EvalExpression`: `calls` перед внешним кэшем, `fullHits` в ветке попадания, `fullClears` перед `exprResultFullCache.Clear`, `exprHits` в ветке попадания внутреннего кэша, `exprClears` перед `exprResultCache.Clear` (`CommonFunction.cpp:1309,1314-1315,1321,1383-1384,1393`). Всё под `#if defined(TESTING)` — в обычных сборках счётчиков нет. `clang-format` идемпотентен; повторный прогон не даёт diff.

### Next Step
Снять чужую debug-сессию VS (`debugger_stop`), собрать `BuildAddOn.py -c config.json -v 25`, запустить `Tools/restart_archicad_for_test.ps1`, прочитать сводку `=EvalExpression=` из панели «Отладка» на `test_25.pln`. Ожидаемый результат по замеру: попаданий мало или нет, формулы со ссылками на параметры дают уникальный ключ. По фактическим долям решить судьбу внешнего кэша. Правку guard `||`→`&&` (`CommonFunction.cpp:1300`, из `8aaa599`) НЕ трогать — отдельная задача.

### Last Checkpoint
Не создан: линковка не прошла, поведение не подтверждено.

### Plan
- [x] Issue #217, изучить `PropertyCache` и места очистки кэшей в `EvalExpression`.
- [x] `FormulaCacheStats` + поле в `PropertyCache` + `ReportFormulaCacheStats` через `DBprnt`.
- [x] Инкременты и печать перед каждым `Clear()` в `EvalExpression`; `clang-format`, LSP 0.
- [/] Сборка AC25 (блокер — чужая debug-сессия VS держит `.apx`) и чтение сводки на `test_25.pln`.
- [ ] По фактическим долям попадания решить судьбу внешнего уровня кэша; оформить checkpoint.

### Decisions
- Всё под `#if defined(TESTING)`: `TESTING` определён в Debug и ProfileDebug (`Tools/CMakeCommon.cmake:106`), этого хватает для замеров, релизные сборки не трогаем.
- Печать привязана к `Clear()`, а не к отдельной команде: сброс по переполнению уничтожает рабочее множество, и это единственная точка, где сводка ещё не потеряна. Плюс `GetSize() > 4096` — единственное место очистки обоих кэшей.
- Счётчики `UInt64` — переполнение `UInt32` через ~4 млрд вызовов в длинной сессии недопустимо для точных данных.
- `fullHits`/`exprHits` считаются от `calls` (вызовов, дошедших до кэшей), а не от всех вызовов функции: ранние `return false` на пустой строке и отсутствии разделителей кэши не трогают.

