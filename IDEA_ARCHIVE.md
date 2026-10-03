# IDEA Archive
Завершённые задачи и отменённые направления из `IDEA.md`. Новая запись — сверху (сразу под этим абзацем),
старые — ниже.
---
## #235 — Sync_to{Attribute:Composite/BuildingMaterial/CompositeType} (закрытие 2026-10-03, issue #235 CLOSED)
### Задача
Дать правилам синхронизации возможность назначать элементу состав (многослойная
конструкция), стройматериал (однородная конструкция) и тип конструкции, а также
читать эти значения обратно.
### Scope
`Sources/AddOn/Helpers.cpp` (`WriteAttribute`, `ReadAttributeValues`, новая internal
`ResolveConstructionAttribute`), `Helpers.hpp` (сигнатура `ReadAttributeValues`),
`Propertycache.cpp` (`GetAllAttributeToParamDict`), `tests/TestSync.cpp`,
`Docs/modules/{Helpers,Propertycache,Sync,TestFunc}.md`, `Docs/_generated/symbols.json`.
Слой не изменён. Профиль, балки и колонны исключены решением владельца.
### Решение
Объём «только Basic ↔ Composite» зафиксирован комментарием в issue и остаётся
актуальным: Profile, колонны и балки не обрабатываются.

Кэш атрибутов расширен с Layer на Layer + CompWall + BuildingMaterial; ключи
`{@attrib:<тип>_name_<нижний регистр>}` и `{@attrib:<тип>_inx_<индекс>}`.
Значение атрибута разрешается по тексту или по индексу; пригодность состава для
типа элемента проверяется через `ACAPI_Attribute_Get` и флаг `APICWall_For*`
**до** изменения — несовместимый состав молча отбрасывается, а не ломает элемент.

Приоритет при одновременном задании: `Composite` → `BuildingMaterial` →
`CompositeType`. При этом если `Composite` задан, но не разрешился (пусто,
невалидно, нет в кэше), ветка переходит к `BuildingMaterial` — это следствие
цепочки `else if`, а не отдельного правила; при записи меняется только одно из
полей (`composite` либо `buildingMaterial`), так как `modelElemStructureType`
принимает одно значение.

Изменение сигнатуры `ReadAttributeValues` потребовало правки в `Read`: ветка,
не читающая полный `API_Element`, теперь переносит полученный `elem_head` в
`element.header`, иначе терялись GUID и слой.
### Checkpoint
`2da2264` (реализация), `25fb2af` (хеш в `IDEA.md`) — оба `Refs: #235`.
### Validation
- Verified: контракты `ACAPI_Element_Change`, `ACAPI_Attribute_Get`,
  `API_ModelElemStructureType`, `APICWall_For*`, поля `composite` /
  `buildingMaterial` / `modelElemStructureType` для Wall/Slab/Roof/Shell —
  по LightRAG и установленным заголовкам DevKit AC25; вложенность
  `shellBase` у крыш и оболочек проверена по исходникам.
- Compiled: да — AC25 Debug `success`, аддон загружен. AC26–29 также собирались
  успешно, но **до** уточнения владельца; сейчас правило — собирается только
  AC25, остальные версии по явному запросу. AC22 падает на компиляции ресурса
  Browser и на чистом HEAD; AC23–24 — на ошибках в участках вне затронутых
  файлов; macOS — `not verified` (CI).
- Tested: автотесты AC25 после финальной сборки `suites=67 passed=2490
  failed=0`, в том числе новые кейсы `TestName2Rawname` (35/0) и
  `TestSyncString` (53/0) на именах `Attribute:Composite/BuildingMaterial/
  CompositeType`. Ручная проверка переключения Basic ↔ Composite на модели
  выполнена владельцем, issue закрыт по его подтверждению.

## #246 — кнопка «Показать в 3Д» в окне `SyncShowSubelement` (закрытие 2026-10-02, issue #246 CLOSED)
### Задача
В модальном окне `OtherDbDialog` (после `SyncShowSubelement`) добавить третью кнопку —
«Показать в 3Д»: открыть/переключиться в 3D-окно, выделить и подсветить элементы
выбранной в списке базы/этажа, приблизить камеру.
### Scope
`Sources/AddOn/dialogs/OtherDbDialog.cpp`, `Sources/AddOn/Constants.hpp`
(`OtherDbShow3DId` = 87), `Tools/AddOn.grc.in` (строка 87 RU/EN, элемент `[4]` блока
`'GDLG' ID_ADDON_OTHER_DB_DLG`, `Button_2` в `'DLGH'`), `Docs/modules/Sync.md`.
Прод-код вне этих файлов не менялся.
### Решение
Кнопка показывает элементы сразу из ВСЕХ баз/этажей списка — выбор строки на неё
не влияет (уточнено владельцем в процессе приёмки).
Ключевой факт, установленный вживую: **порядок вызовов** в 3D-ветке —
`ShowAllIn3D` → выделение → подсветка → `ZoomToElements` → redraw. Первая версия
(перенос окна на `APIWind_3DModelID` → подсветка → `ZoomToElements`) не работала:
после переноса окна текущей базой становится 3D-окно, и элементы чужой базы для
него недоступны — не подсвечивались и не зумились. Выделение ДО перехода не
срабатывает принципиально (документирован `APIERR_BADDATABASE`), выделение ПОСЛЕ
перехода работает — это проверено на живом ArchiCAD, а не выведено из документации,
поэтому оно и было вынесено в эксперимент владельцем. Отказ выделения или зума не
прерывает остальные шаги. `ShowAllIn3D`, а не `ShowSelectionIn3D`: показывать надо
элементы сразу из нескольких баз, а `ShowSelectionIn3D` ограничен текущим выделением.
Версии: `ACAPI_View_ShowAllIn3D` на AC27+, `APIDo_ShowAllIn3DID` на AC22–26;
`ACAPI_Selection_Select` на AC27+, `ACAPI_Element_Select` на AC22–26.
### Checkpoint
`ba50eb9` (кнопка и функции), `22ca70f` (порядок контролов в `'GDLG'`), `d5ba498`
(переход через выделение), `6b79898` (все базы сразу) — все `Refs: #246`.
### Validation
- Verified: контракты `ACAPI_View_ZoomToElements` («works both in the 2D and 3D
  window»), `ACAPI_UserInput_SetElementHighlight` («in the 2D … and 3D window»),
  `ACAPI_Selection_Select` (перечисляет `APIERR_BADDATABASE`),
  `ACAPI_View_ShowAllIn3D` — по заголовкам DevKit AC25/AC27/AC29; наличие/отсутствие
  `APIDo_ShowAllIn3DID` и `ACAPI_View_ShowAllIn3D` сверено по всем семи DevKit.
- Compiled: да — AC25–29 `success` (AC25 через `restart_archicad_for_test.ps1`,
  exit 0; AC26/AC27/AC29 обе версионные ветки). AC22 падает на компиляции ресурса
  и на чистом HEAD — не регрессия; AC24 — DevKit отсутствует; macOS — `not verified`.
- Tested: автотесты AC25 `suites=67 passed=2475 failed=0` (mtime сверен с `.apx`);
  runtime вживую на AC25 — подтверждено владельцем: список заполнен, подписи на
  месте, 3D-окно открывается, элементы со всех баз подсвечены и выделены, камера
  наводится. Сценарий интерактивный, автотестами не покрыт.
### Грабли (перенесены в `IDEA.md`)
ID контрола в `'GDLG'` = позиция строки, число в комментарии `/* [ n] */` не
читается; `AddOn.grc` регенерируется из `Tools/AddOn.grc.in` на конфигурации CMake,
поэтому отставание `.apx` от правки шаблона выглядит как «ничего не изменилось».

## #225 — фильтр IsElementEditable перед AttachObserver в ReservationChangeHandler (закрытие 2026-09-28, issue #225 CLOSED)
### Задача
В `ReservationChangeHandler` (`Sources/AddOn/SomeStuff_Main.cpp`) для каждого зарезервированного
элемента безусловно вызывался `AttachObserver`. Требовался фильтр `IsElementEditable` перед
подпиской — по аналогии с `ElementEventHandlerProc`, где такой предикат уже стоит.
### Scope
Только `ReservationChangeHandler`: определение `objectId` из итератора `reserved` и фильтр
перед `AttachObserver`. Правки `Helpers.cpp`, `Sync.cpp` и прочих блоков `IDEA.md` не трогались.
### Решение
`objectId` определяется один раз с сохранением версионного различия `ConstPairIterator`:
с `ServerMainVers_2800` `it->key` — ссылка, раньше — указатель (`*it->key`). Из-за этого
дублирующий `#ifdef` в вызове `AttachObserver` отпал — вызов стал один. Записи БД не
добавлялись: док DevKit-25 (APIReservationChangeHandlerProc) запрещает её в reservation
handler, рефреш кэша остался в `ProjectEventHandlerProc` по `APINotify_ReceiveChanges`.
Фильтр `IsElementEditable (objectId, syncSettings, true)`: `true` = проверять `CheckElementType`,
чтобы не подписывать observer на несинхронизируемые типы, элементы в hotlink-модуле, чужие и
нередактируемые.
### Checkpoint
`7bcf37a` — `[#225] ReservationChangeHandler: фильтр IsElementEditable перед AttachObserver`
(`Refs: #225`).
### Validation
- Verified: сигнатуры `IsElementEditable` (`Helpers.hpp:140`, `Helpers.cpp:593`) и
  `AttachObserver` (`Helpers.hpp:127`, `Helpers.cpp:508`) — по исходникам проекта; версионное
  различие `ConstPairIterator` подтверждено соседним кодом `BrowserPalette.cpp:510-515` и
  успешной компиляцией обеих веток.
- Compiled: да — `BuildAddOn.py -v 25` и `-v 28` (покрывают обе ветки `#ifdef`),
  `AI_BUILD_RESULT status=success`; затем `Tools/restart_archicad_for_test.ps1` —
  `AI_RESULT status=success`, ArchiCAD запущена на `test_25.pln` (build+load подтверждён).
- Tested: нет — реальная подписка в Teamwork-сессии не наблюдалась; панель «Отладка» VS
  при запуске через runner не создаётся. Для проверки нужен Teamwork-проект и резервирование
  элемента вне настроек синхронизации. Версии 22/23/24/26/27/29 не собирались.
### Грабли
- Предсуществующая ошибка clangd `Unused variable 'err'` в `Do_ElementMonitor`
  (`SomeStuff_Main.cpp:295`) — вне scope, MSVC её пропускает, обе сборки зелёные.
- Карточка `Docs/modules/SomeStuff_Main.md` не содержит секции по `ReservationChangeHandler` —
  §17-обновление не потребовалось.
## #221 — проверка диапазона при приведении double к Int32 (закрытие 2026-09-28, issue #221 CLOSED)
Итог: `DoubleToInt32` (`CommonFunction.cpp:955`, hpp:428) и `DoubleToInt32RoundUp` (:990, hpp:437) с насыщением на границах диапазона и сообщением через `msg_rep` (`APIERR_BADVALUE`); `NaN` → 0. Заменено 22 приведения `double` → `Int32` из данных проекта (Spec, Helpers, CommonFunction, Dimensions, Roombook). Типы и числовая семантика не менялись. Коммиты: `309606a` (код), `bc0eb5d` (документация). Issue-комментарий: https://github.com/kuvbur/AddOn_SomeStuff/issues/221#issuecomment-5868042086
**Не проверено:** runtime. `Tools/restart_archicad_for_test.ps1` отказался работать (`exit_code=20`, `reason=multiple_archicad_processes` — два процесса ArchiCAD, один с проектом `[PK1 - BIMcloud Basic]`); по решению пользователя ArchiCAD не закрывался. Сообщение о насыщении на живом значении вне диапазона не наблюдалось. AC22–24/26–29 не собирались, проверена только AC25.
Ниже — блок задачи в исходном виде.
### Scope
Только проверка диапазона при приведении `double` к целым: `Sources/AddOn/CommonFunction.hpp/.cpp` (хелперы `DoubleToInt32`/`DoubleToInt32RoundUp` и их вызовы в `ceil_mod_classic`, `DoubleM2IntMM`, `DelimTextLine`, `API_AttributeIndexFindByName`), `Sources/AddOn/Helpers.cpp` (`ReadID`, `NumToString`, `ConvertToParamValue` ×3, `ConvertToParamValue(API_Property)` ×3, `ConvertStringToParamValue`, `ConvertDoubleToParamValue`, `ConvertToParamValue(API_IFCProperty)` ×2, `ConvertByFormatString`), `Sources/AddOn/Dimensions.cpp::DimParse`, `Sources/AddOn/Roombook.cpp` (3 места подсчёта пробелов), `Sources/AddOn/spec/Spec.cpp` (`GetParamValue`, номер строки, `show_type`), карточка `Docs/modules/CommonFunction.md`. Типы данных и числовая семантика не меняются. Незакоммиченные правки `Sync.cpp`/`TestFunc.cpp` (задача Name2Rawname) сохранить и в checkpoint не включать.
https://github.com/kuvbur/AddOn_SomeStuff/issues/221
### Status
DONE (код) / NOT TESTED — код готов, `clang-format` выполнен, clangd по `CommonFunction.cpp` показывает только предсуществующие unused-переменные (строки 139/153/714/1167/1735), в новом коде ошибок нет; сборка AC25 успешна (`AI_BUILD_RESULT status=success`). Runtime-проверка не выполнялась по решению пользователя.
### Last Completed
2026-09-28 — добавлены `DoubleToInt32` (насыщение: NaN → 0 с сообщением, выше `INT32_MAX` → `INT32_MAX`, ниже `INT32_MIN` → `INT32_MIN`, каждое через `msg_rep` с `APIERR_BADVALUE`) и `DoubleToInt32RoundUp` для мест, где после каста шло округление вверх `+= 1` — без него насыщение давало бы переполнение. Заменено 22 приведения `double` → `Int32` из данных проекта (свойства, GDL-параметры, IFC, материалы слоёв, ID, размеры, ширины текста). Не тронуты касты, где диапазон гарантирован кодом выше: нормализованный угол направления (`Helpers.cpp:3598`, `dir %= 8`) и индексы атрибутов `API_AttributeIndex` (`Helpers.cpp:8859/9551/9091/9094`, `CommonFunction.cpp:2565`). Ключевое место issue — `Spec.cpp::GetParamValue:1472` — теперь `DoubleToInt32 (x, "Spec::GetParamValue", "intValue материала слоя " + val, elemguid)`. Документация: карточки `CommonFunction.md`/`Helpers.md`/`spec.Spec.md`/`Dimensions.md`/`Roombook.md`, `symbols.json` (две записи вручную), `_progress.md`.
### Next Step
Задача закрыта. Если понадобится фактическое подтверждение сообщения о насыщении — запустить `Tools/restart_archicad_for_test.ps1` при одном открытом ArchiCAD и создать значение вне диапазона Int32 (строка в строковом свойстве или в слое материала).
### Last Checkpoint
`309606a` — `[#221] Проверка диапазона при приведении double к Int32` (только файлы #221). `bc0eb5d` — `[docs] Актуализация после #221` (карточки `CommonFunction`/`Helpers`/`spec.Spec`/`Dimensions`/`Roombook`, `symbols.json` вручную, `_progress.md`).
### Plan
- [x] Собрать все места небезопасного приведения `double` → `Int32` из данных проекта (grep по `Sources/AddOn`).
- [x] Добавить `DoubleToInt32`/`DoubleToInt32RoundUp` в `CommonFunction` с сообщением через `msg_rep`.
- [x] Заменить приведения, не меняя типов и числовой семантики; `clang-format`, clangd, сборка AC25.
- [x] Обновить карточки `Docs/`, `symbols.json` вручную, `_progress.md`; закрыть issue #221 с указанием непроведённой runtime-проверки.
- [x] Runtime пропущен по решению пользователя (ArchiCAD с проектом PK1 не закрывать).
---
## #170 — совместимость сборок AC25–AC29 (закрытие 2026-09-27, issue #170 CLOSED)
Итог: `python Tools/BuildAddOn.py --configFile config.json --acVersion <V>` — success для AC25/26/27/28/29,
все `Build/SomeStuff/<V>/Debug/SomeStuff.apx` перезаписаны 2026-09-27 (AC25 — 18:27).
AC22–24 в этой среде не проверяемы: в репозитории лежат только DevKit-25…29.
Коммиты: `e0e3a98` (AC26), `a74441f` (AC27), `6ec9b42` (AC28), `acb0232` (AC29).
Runtime-лаунчер `Tools/restart_archicad_for_test.ps1` (AC25, версия из `config.json`): exit 0, `AI_RESULT status=success build=True archicad=running` — аддон собрался и загрузился в Archicad 25; JSON-тесты пропущены (`JSON_TESTS_SKIPPED reason=script_not_found`, `Tools/test_json_commands.py` в репозитории нет).
Ниже — блоки задач в исходном виде (AC26 — из прошлой сессии, AC27/28/29 — из сессии 2026-09-27).
---
## Parallel Task — #170 сборка AC26
### Task
Сборка под AC26 (`Tools/BuildAddOn.py --configFile config.json --acVersion 26`) падала на AC26-несовместимостях SDK; ошибки устранены, сборка зелёная.
https://github.com/kuvbur/AddOn_SomeStuff/issues/170
### Scope
Только AC26-сборка: `Sources/AddOn/dialogs/SyncSettings.cpp`, `Sources/AddOn/json_commands/CommandBase.hpp/.cpp`, `Sources/AddOn/dialogs/BrowserPalette.cpp` и карточки `Docs/modules/json_commands.md`, `Docs/modules/dialogs/BrowserPalette.md`, `Docs/modules/dialogs/SyncSettings.md`. Незакоммиченный раздел ReadQuantities в `IDEA.md` — чужая правка, в checkpoint не включать. AC22–24/27–29 не проверялись.
### Status
DONE (AC26) — build успешен, регрессия AC25 успешна. Issue #170 не закрыт: R9 покрывает ещё AC22–24 и 27–29.
### Last Completed
2026-09-27 — `python Tools/BuildAddOn.py --configFile config.json --acVersion 26`: `AI_BUILD_RESULT status=success`, артефакт `Build/SomeStuff/26/Debug/SomeStuff.apx`; AC25-сборка после правок успешна; clangd (AC26 compile DB) 0 ошибок; clang-format. Правки: pragma clang под `#ifdef __clang__` (AC26 DevKit больше не глушит C4068 при `/WX`); `CommandBase::IsProcessWindowVisible` (новый чистый виртуальный в AC26); void-возврат `ACAPI_Interface_*ElementHighlight`; `UnregisterJSObject` закрыт `#ifndef ServerMainVers_2600`; `head.typeID` → `GetElemTypeID(head)`. AC27-сборка запускалась и падает (десятки переименований API) — это отдельный объём, не правился. Комментарий с деталями — в issue #170.
### Next Step
AC27 (и остальные версии R9) — отдельной задачей. Runtime AC26 не проводился: Archicad 26 не запускался, `test_26.pln` не прогонялся.
### Last Checkpoint
`e0e3a98` — `[#170] Сборка AC26: устранены AC26-несовместимости SDK`.
### Plan
- [x] Воспроизвести падение сборки AC26 и собрать полный список ошибок.
- [x] Подтвердить каждую ошибку по заголовкам DevKit-26/25 (не по памяти).
- [x] Исправить 5 мест; clang-format, clangd, AC26 build, регрессия AC25.
- [x] Обновить карточки модулей и tracker-строку R9; комментарий в #170.
- [ ] AC27–29 и AC22–24 (отдельная задача).
### Decisions
- `IsProcessWindowVisible` возвращает `true` для всех команд: они длительные и идут в главном потоке (`ScheduleForExecutionOnMainThread`), как в DevKit-примере `AddOnCommandTest`. Поведение окна процесса в AC26 runtime не проверялось.
- Ветка AC27 для подсветки правилась в том же операторе (обе новые ветки возвращают `void`), но AC27-сборка не проходит по другим причинам — эта правка не подтверждена сборкой AC27.
## Parallel Task — #170 сборка AC27
### Task
Сборка под AC27 (`Tools/BuildAddOn.py --configFile config.json --acVersion 27`); AC27 переименовал большую часть API в именованные функции.
https://github.com/kuvbur/AddOn_SomeStuff/issues/170
### Scope
Только AC27-сборка: `Sources/AddOn/api_headers/APIEnvir.h` (правка откачена), `Roombook.cpp`, `Sync.cpp`, `dialogs/BrowserPalette.cpp/.hpp`, `dialogs/OtherDbDialog.cpp`, `dialogs/SyncSettings.cpp`, `json_commands/JsonCommandRegistrar.cpp`, `table/TableRenderer.cpp`, `table/TablesNavigator.cpp` и карточки `Docs/modules/`. Разделы IDEA.md других задач не коммитить. AC22–24/28–29 не проверялись.
### Status
DONE (AC27) — build успешен, артефакт `Build/SomeStuff/27/Debug/SomeStuff.apx`; AC26 после правок успешна. Регрессия AC25 НЕ подтверждена: сборка падает на LNK1168 (файл `Build/SomeStuff/25/Debug/SomeStuff.apx` держит запущенный пользователем Archicad — по его указанию процесс не закрывать). Issue #170 не закрыт: R9 покрывает ещё AC22–24 и 28–29.
### Last Completed
2026-09-27 — `python Tools/BuildAddOn.py --configFile config.json --acVersion 27`: `AI_BUILD_RESULT status=success`; AC26 — success; clangd (compile DB AC27) 0 ошибок в табличных файлах и BrowserPalette; clang-format. Правки: legacy-вызовы обёрнуты на местах `#ifdef ServerMainVers_2700` / `#else` (новое имя из DevKit-27 `ACAPI_MigrationHeader.hpp`, сам заголовок в проект НЕ подключается — указание пользователя); `UnregisterJSObject` в AC27 вернулся (JavascriptEngine) → гвард `#if defined(ServerMainVers_2700) || !defined(ServerMainVers_2600)`; `DGModule.hpp` в AC27 больше не тянет `DGBrowser.hpp`; JS-мост AC27 (`JS::*` вместо `DG::JS*`) закрыт псевдонимами в `namespace DG` под `#ifdef AC_27`; `API_Navgator*` → `API_Navigator*`; `API_AttributeIndex` через `ACAPI_CreateAttributeIndex`, шрифт через `ACAPI_Font_GetFont (API_FontType)`, `API_OverriddenAttribute` → `APIOptional<API_AttributeIndex>`.
### Next Step
1) Освободить `Build/SomeStuff/25/Debug/SomeStuff.apx` (закрыть сессию отладки AC25) и прогнать регрессию AC25. 2) Runtime-проверка AC27 (`restart_archicad_for_test.ps1` — только по согласованию, чтобы не мешать отладке пользователя). 3) Остальные версии R9 (22–24, 28–29) — отдельными шагами.
### Last Checkpoint
`a74441f` — `[#170] Сборка AC27: legacy-вызовы API обёрнуты #ifdef ServerMainVers_2700` (закоммичены только исходники и карточки Docs; раздел IDEA.md не коммитится вместе с разделами других задач).
### Plan
- [x] Собрать полный список AC27-ошибок и классифицировать (переименования против структурных).
- [x] Забрать из `ACAPI_MigrationHeader.hpp` нужные соответствия, не подключая заголовок.
- [x] Обернуть вызовы на местах `#ifdef ServerMainVers_2700`; clang-format.
- [x] AC27 build — success; AC26 build — success; clangd по правкам — 0.
- [ ] Регрессия AC25 (блокирована занятым `.apx`).
- [ ] Runtime AC27.
## Parallel Task — #170 сборка AC28
### Task
Сборка под AC28 (`Tools/BuildAddOn.py --configFile config.json --acVersion 28`). AC28 снял `__ACENV_CALL`, перевёл memo текста на `GS::UniString*`, убрал `charCode` и старые MEP-контейнеры.
https://github.com/kuvbur/AddOn_SomeStuff/issues/170
### Scope
Только AC28-сборка: `table/TableRenderer.cpp`, `table/TablesNavigator.cpp`, `dialogs/BrowserPalette.cpp/.hpp`, `MEPv1.hpp` и карточки `Docs/modules/`. Разделы IDEA.md других задач не коммитить. AC22–24 и 29 не проверялись; AC25-регрессия не подтверждена.
### Status
DONE (AC28) — build успешен, артефакт `Build/SomeStuff/28/Debug/SomeStuff.apx`; AC27 и AC26 после правок тоже success. AC25 по-прежнему заблокирована (LNK1168: файл держит сессия отладки пользователя, процесс не закрываем). Issue #170 не закрыт: остались AC22–24, AC29 и AC25-регрессия.
### Last Completed
2026-09-27 — AC28: `AI_BUILD_RESULT status=success mode=build versions=28`; AC27/AC26 — success (артефакты перезаписаны 17:41). Ошибок было 128 → 0 за три прохода. Корни: (1) `__ACENV_CALL` удалён из SDK в AC28 (в 27 определялся в `APICalls.h`) — 4 обработчика TablesNavigator, 2 в BrowserPalette; (2) `API_ElementMemo::textContent` стал `GS::UniString*` (был `GSHandle`), `API_TextType::charCode` удалён — TableRenderer создаёт `new GS::UniString (line)` и код гарнитуры больше не подменяет; (3) `GS::HashTable::CurrentPair::value` — ссылка `Value&` вместо `Value*` (7 мест в BrowserPalette.cpp); (4) JS-псевдонимы `DG::JS* → JS::*` стояли под `#ifdef AC_27`, а в AC28 определён `AC_28` → переведены на `ServerMainVers_2700`; (5) старые MEP-контейнеры (`MEPDuctPreferenceTableContainer.hpp` и др.) в AC28 удалены → include под `#ifndef ServerMainVers_2800`.
### Next Step
1) Освободить `Build/SomeStuff/25/Debug/SomeStuff.apx`, прогнать регрессию AC25. 2) AC29 — отдельным шагом. 3) AC22–24 — отдельным шагом. 4) LSP-диагностика AC28 (запущена) + runtime AC27/AC28 по согласованию.
### Last Checkpoint
`6ec9b42` — `[#170] Сборка AC28: __ACENV_CALL, memo текста, CurrentPair::value` (закоммичены исходники и карточки Docs; раздел IDEA.md не коммитится вместе с разделами других задач).
### Plan
- [x] Собрать список AC28-ошибок и классифицировать (128 → 10 → 0).
- [x] Правки на местах по конвенции (`#ifdef ServerMainVers_2800` + `#else`), clang-format.
- [x] AC28 build — success; регрессия AC27/AC26 — success.
- [ ] Регрессия AC25 (блокирована занятым `.apx`).
- [ ] AC29, AC22–24.
- [ ] Runtime AC27/AC28.
### Decisions
- В AC28 обходной путь с `charCode` не нужен: кодировку несёт `GS::UniString`; ветка 2800 оставлена с комментарием, поведение AC26/27 не тронуто.
- `#ifdef AC_27` для JS-псевдонимов заменён на `ServerMainVers_2700` (27/28/29), т.к. при сборке AC28 определяется `AC_28`, а не `AC_27`.
## Parallel Task — #170 сборка AC29
### Task
Сборка под AC29 (`Tools/BuildAddOn.py --configFile config.json --acVersion 29`). AC29 = правила AC28 плюс переход тулчейна на C++20.
https://github.com/kuvbur/AddOn_SomeStuff/issues/170
### Scope
Только AC29-сборка: `ReNum.hpp` (const-квалификатор `operator==`), карточка `Docs/modules/ReNum.md`. Разделы IDEA.md других задач не коммитить. AC22–24 не проверялись; AC25-регрессия не подтверждена.
### Status
DONE (AC29) — build успешен, артефакт `Build/SomeStuff/29/Debug/SomeStuff.apx`; регрессия AC28/27/26 после правки — success (все `.apx` перезаписаны). AC25 по-прежнему заблокирована (LNK1168: `.apx` держит сессия отладки пользователя). Issue #170 не закрыт: остались AC22–24 и AC25-регрессия.
### Last Completed
2026-09-27 — AC29: `AI_BUILD_RESULT status=success mode=build versions=29`; AC28/27/26 — success. Ошибок было всего 2 (обе `TestFunc.cpp:2839/2841`, C2666 `RenumPos::operator ==`). Корень: AC29 — первый SDK, собирающий аддон как C++20 (`<LanguageStandard>stdcpp20` в vcxproj; в 26–28 `stdcpp17`); для неконстантного члена `operator==` C++20 добавляет перевёрнутый кандидат `y == x`, и `pa == pb` (два разных объекта) становится неоднозначным. Правка — `const` у `bool RenumPos::operator== (const RenumPos &b) const` в `ReNum.hpp`; поведение не меняется, для C++17 правка нейтральна. clangd (compile DB AC29): `ReNum.hpp` 0 ошибок; `TestFunc.cpp` — только предсуществующий clang-only шум (6 `-Wunused*` и 7 `operator= с void` в `GS::Array`), MSVC с `/WX` его пропускает.
### Next Step
1) Освободить `Build/SomeStuff/25/Debug/SomeStuff.apx`, прогнать регрессию AC25. 2) AC22–24 — отдельным шагом (там другой набор API-различий). 3) Runtime AC27/28/29 по согласованию. 4) Закрытие #170 после AC25-регрессии.
### Last Checkpoint
`acb0232` — `[#170] Сборка AC29: C++20 — const у RenumPos::operator==` (закоммичены `ReNum.hpp` и карточка Docs; раздел IDEA.md не коммитится вместе с разделами других задач).
### Plan
- [x] Собрать список AC29-ошибок (2 шт., обе из-за C++20).
- [x] Правка корня — `const` у `operator==`, clang-format.
- [x] AC29 build — success; регрессия AC28/27/26 — success; clangd по правке — 0.
- [ ] Регрессия AC25 (блокирована занятым `.apx`).
- [ ] AC22–24.
- [ ] Runtime AC27/28/29.
### Decisions
- Правится прод-тип (`RenumPos::operator==` становится `const`), а не тест: это C++20-корректность самого класса, а не особенность теста; `TestFunc.cpp` не тронут (по §10 AGENTS тест-файлы при правке прод-кода не правятся).
- AC29 не потребовал новых `#ifdef`-веток: все ветки 2700/2800 в проекте активны и в 29, кроме C++20-эффекта.
## #212 — удаление служебных комментариев FIX (2026-09-27)
### Status
COMPLETED. Issue [#212](https://github.com/kuvbur/AddOn_SomeStuff/issues/212) закрыт; checkpoint `1da2b9f` — `[#212] Удалить комментарии FIX из исходников`.
### Scope и решение
Из 18 файлов `Sources/AddOn/` удалены 198 блоков комментариев с отдельным маркером `FIX`. Исполняемый код и вызовы ArchiCAD API не изменялись. Незакоммиченные изменения #209 в `Helpers.cpp` были временно изолированы на время валидации и восстановлены после checkpoint.
### Validation
AC25 Windows Debug: `restart_archicad_for_test.ps1` завершён с `AI_RESULT status=success exit_code=0 reason=completed build=True archicad=running`; `SomeStuff.apx` собран, тестовый `test_25.pln` запущен. Поиск комментариев `FIX` в `Sources/AddOn/` — 0 совпадений; `git diff --check` — успешно; `clang-format` выполнен. JSON-тесты пропущены: `Tools/test_json_commands.py` отсутствует. C++ output/DBtest не проверялся.
---
## Реорганизация задач (2026-09-25) — закрытие issues и перенос блоков из IDEA.md
### Что сделано
Закрыты issues с подтверждённым результатом (комментарий в каждом с коммитом и проверкой):
[#193](https://github.com/kuvbur/AddOn_SomeStuff/issues/193) — фикс BuildOtdByParent в `b24973c`, runtime подтверждён (Wall 334↓/334↑ без роста количества, Object/Slab/Beam GUID не изменились); блокировка «смешанное рабочее дерево» снята — все пересекающиеся правки закоммичены.
[#199](https://github.com/kuvbur/AddOn_SomeStuff/issues/199) — `9ae0138`, TestSyncAddSubelement RED→GREEN; закрыт по unit-покрытию (решение пользователя), runtime Sync_to_GUID — по потребности.
[#205](https://github.com/kuvbur/AddOn_SomeStuff/issues/205) — `5b6914b`, RoomBookCommand; HTTP-вызов `returned` 15.085 с + два успешных прогона в проверке #193; отмена — not verified.
[#206](https://github.com/kuvbur/AddOn_SomeStuff/issues/206) — `5b6914b` + `50cae05`, SpecCommand; вызов completed, 34/34 строк с наименованием.
[#161](https://github.com/kuvbur/AddOn_SomeStuff/issues/161) — `17cb79d`, RED→GREEN 488 ok; остался открытым после checkpoint по недосмотру.
Статусы по пользовательским проверкам: #204/#203/#194 закрыты после подтверждения пользователя 2026-09-25 (палитра AC25: pin групп, script маркера проёма, «показать родительские»); #191 закрыт — проверка описана в теле issue (runtime AC25, коммит `4f57c50`). Остались: #189 — WAITING_FOR_TEST (проверка палитры за пользователем), ревизия Sync — GUI/`Sync_to_GUID` по потребности.
### Перенесено из активной секции IDEA.md в архив (COMPLETED, ниже нет)
#197 (2026-09-23), #196 (2026-09-23, пользовательские правки вошли в `de8433e` «Изменение механизма пропуска обработки»), #192 (2026-09-22, `89448ea`), #210 (2026-09-25, `aa4a672`), «Монитор, остаток работ» с бэклогом #158/#159/#176–#187 и методикой 1.7.x.
### Грабли, удалённые по проверке
- «Моки `ACBridge` getFilterPresets/resetPropertyToDefault» — getFilterPresets реализован реальным мостом (`#157`, HTML:280–289 `window.ACAPI.GetFilterPresets`); resetPropertyToDefault HTML:442 остаётся моком намеренно (режим выборочного сброса #189, мост сохранён).
- «JSON-команды сняты (b7a996b)» — JSON-команды восстановлены AC25–29 в `5b6914b` (json_commands/); актуально только требование `DynamicCast<JSValue>`.
---
## #207/#208/#209 — JSON Spec и наименование из материала (AC25, 2026-09-25)
### Status
COMPLETED для проверенного AC25-сценария; checkpoint `50cae05`. Issues [#207](https://github.com/kuvbur/AddOn_SomeStuff/issues/207), [#208](https://github.com/kuvbur/AddOn_SomeStuff/issues/208), [#209](https://github.com/kuvbur/AddOn_SomeStuff/issues/209) закрыты после финального runner.
### Scope и решение
`SpecAll` при пустом выделении не обрывается, когда default-правило нашло элементы; JSON-команда принимает обязательный `placementPoint`, необязательный `ruleNames`, обходится без UI-пути и возвращает статус/код/счётчики. `ParamHelpers::GetAttributeValues` не пропускает `fromPropertyDefinition` при чтении строительного материала; `elementsToCreate` считает прирост созданных элементов, а не уже подготовленные изменения. Интерактивный menu-path сохранён.
### Validation и ограничения
AC25 Windows Debug: clangd по изменённым файлам — 0 диагностик; `restart_archicad_for_test.ps1` — `build=True`, запущен `test_25.pln`. JSON-вызов с `{x:0,y:0}` вернул `completed`, `resultCode=0`, `elementsToCreate=34`; объектов 548→582, последние 34/34 имеют непустое «Спецификации материалов/Наименование в объект». Фоновый вызов, начатый до исправления #209, завершился `failed` с нулевыми счётчиками — это не результат новой сборки. **Не проверено:** AC26–29/macOS, смешанное создание+изменение, визуальное отсутствие всех окон.
---
## Справочная таблица кэша «Монитора» (2026-09-14, перенесено из IDEA.md)
| Компонент | Файл | Назначение |
|-----------|------|------------|
| `PropertyCache::property` | Propertycache.hpp:107 | Словарь всех свойств (ParamDictValue) — самый быстрый доступ |
| `PropertyCache::propertygroups` | Propertycache.hpp:115 | Группы свойств (Guid → API_PropertyGroup) |
| `PropertyCache::systemdict` | Propertycache.hpp:113 | Системы классификации |
| `GetSelectedElements2()` | CommonFunction.hpp:292 | GUID выделенных элементов (оптимизированная) |
| `ParamHelpers::GetParamValueFromCache()` | Propertycache.hpp:46 | Чтение значения из кэша по rawname |
| `ParamHelpers::GetAllPropertyDefinitionToParamDict()` | Propertycache.hpp:92 | Все определения свойств в ParamDictValue |
| `ParamHelpers::GetGroupFromCache()` | Propertycache.hpp:55 | Имя группы свойств по Guid |
| `ParamHelpers::isPropertyDefinitionRead()` | Propertycache.hpp:51 | Проверка готовности кэша |
| `ClassificationFunc::GetAllClassification()` | ClassificationFunction.hpp:26 | Загрузка всех классификаций |
| `ClassificationFunc::ReadSystemDict()` | ClassificationFunction.hpp:64 | Словарь систем/классов |
| `RegisterACAPIJavaScriptObject()` | dialogs/BrowserPalette.cpp:100 | JS-мост в BrowserPalette |
---
## UI классификации (2026-09-13, коммит ad7e8f5) — COMPLETED (код), runtime НЕ проверялся
`Interface_ru.html`: выбор классификации + «Применить ко всем» видны всегда (ранний `return` по
`data.common` прятал их); дедуп дублей `uniqueClassificationOptions`; dropdown как в Archicad —
дерево (▸/▾) + живой поиск с подсветкой совпадений. Валидация `Tools/test_html.ps1` PASSED; логика
проверена ad-hoc в node.
## Ревью 2026-09-12 — CLOSED
60 находок (11add7a + 13ba469), ревью 2026-09-12b 87 пунктов (2a48348), вторая проверка (072f975),
Sync.cpp-7 Name2Rawname (1625cf1). Остаток — issues #161/#162, runtime R2–R10 — #163–#171.
## Настройки в локальный файл (2026-09-12, коммит f0909f0) — COMPLETED
`…/GRAPHISOFT/SomeStuff/SyncSettings.dat` вместо prefs проекта (ломали Teamwork). Runtime AC25:
файл создан, ошибок нет; TW-проверка открыта (issue #169 / R8); сборки AC22–24/26–29 не проверены
(issue #170 / R9).
## Runtime AC25 (2026-09-12) — 741 ok, 0 ERROR IN TEST (TEST : start..end), AC25 Build succeeded
## Подсветка+зум по клику ×N, выделение сохраняется — Verified 2026-08-24 (Дмитрий, runtime AC25)
---
## Снято: JSON-команды и Этапы 2/3 старого плана (2026-09)
Идея JSON-команд (`API_AddOnCommand` / `JsonCommandRegistrar` / `AddOnCommandId`) **отменена** —
реализация удалена коммитом `b7a996b` (`[json-cleanup] Удаление JSON команд и тестов`). Вся UI-функциональность
делается инлайн-функциями JS-моста `BrowserPalette` (`RegisterACAPIJavaScriptObject`).
Соответственно не актуальны спецификации старого плана:
- Этап 2.1 `ExecuteSyncScriptCommand` — `{scriptText}` → `{success, message, errorLine?, errorColumn?, errorText?}`
- Этап 3.1 `GetNumberingPropertiesCommand` — → `{properties: [{id, name, groupName}]}`
- Этап 3.2 `GetNumberingPreviewCommand` — `{config: NumberingConfig}` → `{preview: [{elementGuid, elementName, newValue}]}`
- Этап 3.3 `ExecuteNumberingCommand` — `{config: NumberingConfig}` → `{success, changedCount}`
Оставлено только как исторический контекст. Если понадобится вкладка «Синхронизация»/«Нумерация» — делать
инлайн-функциями моста, а не `*Command`.
---
## Настройки аддона: локальный файл вместо prefs проекта (2026-09-12, коммит f0909f0)
### Task
Убрать записи настроек аддона в файл проекта (prefs проекта модифицируют БД → ломают Teamwork Send/Receive)
и хранить настройки машинно-локально.
### Status
COMPLETED (код). Открыто: runtime-проверка TW (в `IDEA.md` — пункт R8), LSP-проверка, сборки AC22–24/26–29.
### Диагноз 2026-09-09 (регрессия относительно v1.77, коммит 1ca70b6)
- `ReservationChangeHandler` (SomeStuff_Main.cpp:36–62), появившийся в `dab140a` (2026-07-10), модифицировал БД
  внутри TW-операции: `PROPERTYCACHE().Update()` + `DimRoundAll` → `ACAPI_CallUndoableCommand` +
  `ACAPI_Element_Change` (Dimensions.cpp:326–343).
- Док DevKit-25 (`APIReservationChangeHandlerProc.html`): «In the reservation change handler try to avoid
  calling functions that would modify the database».
- Фикс первой волны: хендлер → только `AttachObserver`; `PROPERTYCACHE().Update()` перенесён в
  `ProjectEventHandlerProc` (case `APINotify_ReceiveChanges`); `APINotify_ReceiveChanges` добавлен в обе маски
  `CatchProjectEvent`. Runtime-тест Дмитрия (2026-09-12): симптом тот же — «в файл продолжают дописываться
  настройки аддона».
### Диагноз-2 (2026-09-12): prefs аддона пишутся в план проекта
- Док DevKit-25 `Level3/Preferences_Save.html`: «The preferences data is also stored in all project files» →
  `ACAPI_SetPreferences` = модификация БД проекта. (LightRAG по вопросу ничего конкретного не дал, источник —
  локальная документация DevKit-25.)
- Места записи: `dialogs/BrowserPalette.cpp:114` (Show), `:127` (Hide), `:570` (JS-мост
  Enable/DisableCatchSelectionChanges); `SomeStuff_Main.cpp:197,204` (ElementEventHandlerProc, флип `logMon`),
  `:489` (Initialize — в 1.77 такой записи не было), `:437` (MenuCommandHandler — есть и в 1.77).
- Петля: `BrowserPalette::Show/Hide` вызываются из `PaletteControlCallBack` по
  `APIPalMsg_HidePalette_Begin/End` (BrowserPalette.cpp:1281/1286) → на каждое действие пользователя 2 записи
  prefs в файл проекта.
- Прочее: результат `ACAPI_SetPreferences` не проверялся (`SyncSettings.cpp:211-227`), кэш не инвалидировался
  при открытии проекта, `ACAPI_GetPreferences` вызывался без `ACAPI_GetPreferences_Platform`.
### Решение (SDK-проверено)
- Папка: `ACAPI_Environment (APIEnv_GetSpecFolderID, &id, &folder)` + `API_SpecFolderID`
  (APIdefs_Environment.h:1621-1634): `API_GraphisoftPrefsFolderID` / `API_ApplicationPrefsFolderID` —
  локальные per-user папки, `API_UserDocumentsFolderID` — fallback.
- Запись/чтение: `IO::File (loc, IO::File::OnNotFound::Create)` + `Open (WriteEmptyMode)` + `WriteBin` + `Close`
  (паттерн Tapir/Config.cpp, DeveloperTools.cpp); `IO::File` наследует `GS::IChannel`/`GS::OChannel` → блоб
  `SyncSettings::Write/Read` пишется в файл напрямую.
- Плюс: блоб становится машинно-локальным, кросс-платформенная проблема (`ACAPI_GetPreferences` без `_Platform`)
  исчезает.
### Реализация
- Шаг 1 (`dialogs/SyncSettings.cpp`): хелпер `GetSyncSettingsFolderLocation` (Graphisoft prefs → Application prefs
  → User documents + `IO::Folder::CreateFolder (IO::Name ("SomeStuff"))`); `ReadSyncSettingsFromFile`
  (заголовок magic 'SSS1' + PreferencesVersion + размер блоба проверяются ДО десериализации);
  `WriteSyncSettingsToPreferences` пишет локальный файл, повторная запись идентичного блоба пропускается
  (memcmp), `ACAPI_SetPreferences` из кода убран; миграция `ReadSyncSettingsFromLegacyPreferences` — одноразово
  при отсутствии/несовпадении файла; ошибки → `msg_rep`/`DBprnt`.
- Шаг 2: записи из `ElementEventHandlerProc` (`logMon` — только в памяти), `Initialize`, `BrowserPalette::Show/Hide`
  убраны; `Show(bool reloadContent)` из `APIPalMsg_HidePalette_End` вызывается как `Show(false)` +
  ручной `UpdateSelectionInfoInUI`; `showpalette` сохраняется только в `ShowOrHideBrowserPalette` и
  `PanelCloseRequested`. `MenuCommandHandler` и JS `Enable/DisableCatchSelectionChanges` оставлены (явные
  действия пользователя) — пишут в локальный файл.
- Принятые допущения: `API_GraphisoftPrefsFolderID` + подпапка `SomeStuff`; миграция из prefs проекта;
  `logMon` только в памяти; один общий `SyncSettings.dat` на все версии AC (имя меняется в одной строке
  `SyncSettingsFileName`).
### Validation
- `clang-format` выполнен; HTML-валидация `Tools/test_html.ps1` — PASSED; AC25 Build succeeded
- Runtime AC25: ArchiCAD стартовал, локальный файл `C:/Users/da-rogojin/AppData/Roaming/GRAPHISOFT/SomeStuff/SyncSettings.dat`
  создан (31 байт), `=== ERROR IN TEST ===` не найден; runner пометил C++-тесты как UNKNOWN (нет recognised status marker)
- **Не проверено**: TW Send/Receive несколькими пользователями (пункт R8 в `IDEA.md`); сборки AC22–24/26–29
  (в репозитории только DevKit-25, `IO::File`/`IO::Folder` в остальных версиях не верифицированы); LSP
  (clangd MCP не стартовал: `Clangd process failed to start`)
### Проверено при составлении плана
- В AC25 **нет** `ACAPI_ProjectSettings_GetSpecFolder` — только `ACAPI_Environment (APIEnv_GetSpecFolderID, …)`
  (grep по хедерам и списку доков).
- `IO::File`: `OnNotFound{Fail,Create,Ignore}` (File.hpp:117), ctor `(Location, OnNotFound)` :125,
  `Open(OpenMode)` :140, `WriteBin` :155, `GetDataLength` :176. Примеры: DevKit `LibPart_Test.cpp:942`,
  Tapir `Config.cpp`/`DeveloperTools.cpp`.
---
## Вкладка «Монитор»: инлайн-функции JS-моста (Этапы 1.1–1.4, 2026-08)
### Task
Реализовать инлайн C++ функции в JS-мосте (`RegisterACAPIJavaScriptObject`, BrowserPalette.cpp), заменив моки
`ACBridge` в HTML вкладки «Монитор», с максимальным использованием уже загруженного `PROPERTYCACHE()`.
### Status
COMPLETED (runtime подтверждён 2026-08-24, AC25). Этапы 1.5 (`GetFilterPresets`) и 1.6
(`ResetPropertyToDefault`) в этот блок не входят — они нужны и перенесены в активный план `IDEA.md`.
### Реализовано
| Функция | Вход → выход | Статус |
|---|---|---|
| `GetSelectionInfo` | → `{count}` | ✅ (push-pattern: `UpdateSelectionInfoInUI` → `ExecuteJS` → `refreshSelectionInfoText(count)`) |
| `GetPropertiesList` | `{filterText?}` → `{groups:[{groupName, properties:[{id,name,allHave,countWithProperty}]}]}` | ✅ |
| `GetPropertyValue` | `{propertyId}` → `{propertyName, common, values:[{value,count}]}` | ✅ зарегистрирована; прямых вызовов в `Interface_ru.html` не найдено (2026-09-12) |
| `GetClassification` | → `{common, commonPath[], differing[{elementName,value}], options[]}` | ✅ |
| `SetClassification` | `{classificationValue}` → `{success}` | ✅ (`ACAPI_Element_SetClassification` внутри `ACAPI_CallUndoableCommand`) |
| `GetFilterPresets` | → `{presets:[{label,query}]}` | ⏸ в коде нет — активный план `IDEA.md`, пункт 1.5 (в старом плане ошибочно стояло `[x]`) |
| `ResetPropertyToDefault` | `{propertyId}` → `{success}` | ⏸ в коде нет — активный план `IDEA.md`, пункт 1.6 (в старом плане ошибочно стояло `[x]`) |
Дополнительно зарегистрированы: `HighlightElements`, `ParsePropertyDescription`, `ParsePropertyForElement`
(два последних HTML не вызывает — мёртвый код, зарегистрированы после реворка `47b06e5`),
`EnableCatchSelectionChanges`/`DisableCatchSelectionChanges` (пара без аргументов).
HTML вызывает: `GetSelectionInfo`, `RefreshSelectionInfoUI`, `GetPropertiesList`, `GetClassification`,
`SetClassification`, `HighlightElements`, плюс динамически Enable/DisableCatchSelectionChanges.
### Принципы (сохранены)
1. Максимальное использование кэша — без `ACAPI_Property_GetPropertyValue` на каждый элемент.
2. Push-pattern для JS-моста (`browser.ExecuteJS()`), не полагаться на return value из `RegisterAsynchJSObject`.
3. Инлайн-реализация в `BrowserPalette`, без промежуточных JSON-команд.
4. Ручная проверка сборки после каждой команды.
5. HTML-правки строго по ТЗ + `Tools/test_html.ps1`.
### Lessons Learned
Ошибки: попытки починить pull-pattern (return value из C++ в JS через `RegisterAsynchJSObject`); игнорирование
паттерна Speckle (push-pattern); отладка через `console.log` — в JS он не попадает в `test_results.txt`,
диагностика только `DBprnt` из C++.
Победы: переход на push-pattern; упрощение JS (`refreshSelectionInfoText(count)` только обновляет DOM);
регистрация JS-объекта через `onLoadingStateChange` после загрузки страницы.
### Ревью моста (2026-08-24)
Корень серии крашей: `GS::DynamicCast<DG::JSArray>` на аргументе `JSFunction` ломает CEF-мост; каст к `JSValue`
безопасен (официальный пример DevKit `Browser_Control` парсит строковый аргумент именно через `JSValue`).
| # | Находка | Статус |
|---|---|---|
| R1 | `GetPropertyValue`: `DynamicCast<JSArray>` → латентный краш | ✅ исправлен (JSValue) |
| R2 | `SetClassification`: `DynamicCast<JSArray>` + HTML звал с массивом | ✅ исправлен (обе стороны) |
| R3 | `ParsePropertyDescription`/`ParsePropertyForElement`: тот же JSArray-парсинг, HTML их не вызывает | ⏸ мёртвый код (позже переписан в `47b06e5`) |
| R4 | Полнота моста (5 прямых + SetLimit*/Enable-Disable) | ✅ ок |
| R5 | Возвраты nullptr из лямбд | ✅ не обнаружены |
| R6 | `SetClassification` мутирует модель внутри `ACAPI_CallUndoableCommand` | ✅ ок |
| R7 | JSON-ответы экранируются `EscapeJsonString` | ✅ ок |
### Hotfix 2026-08-24 (коммит 32fc4a8): краш при отключении автообработки
`SetCatchSelectionChanges(args)` → пара `EnableCatchSelectionChanges`/`DisableCatchSelectionChanges` без
аргументов (паттерн `SetLimit<N>`); HTML: `toggleAutoRefresh` вызывает их без аргументов. Причина: аргументы в
`JSFunction` через `RegisterAsynchJSObject` роняли ArchiCAD (в `test_results.txt` записи лямбды не было).
Результат: 3 клика (Disable/Enable/Disable), все с «ok», краша нет.
### Прочее по UI (2026-08-24/25)
- `3fd665d` — подсветка и зум к элементам по клику на строку значения (Spec.cpp-паттерн:
  `APIIo_HighlightElementsID` + `APIDo_ZoomToElementsID`).
- `84664fe` — подсветка без выделения; `16f2b82` — `suppressSelectionRefresh`, чтобы палитра не сбрасывала
  пользовательское выделение. Детали — скилл `archicad-cef-pitfalls`.
---
## Test project name gate for `test_results.txt`
### Task
Проверять имя открытого файла Archicad и записывать `test_results.txt` только если имя содержит `test`.
### Status
COMPLETED
### Implementation
- Added `IsTestProjectOpen()` in `Sources/AddOn/CommonFunction.cpp`.
- Both `DBprnt` overloads now open `test_results.txt` only after checking `API_ProjectInfo::projectName`.
- AC27+ uses `ACAPI_ProjectOperation_Project(&projectInfo)`.
- AC22–26 uses `ACAPI_Environment(APIEnv_ProjectID, &projectInfo)`.
- The check is case-insensitive and skips unsaved projects, missing names, and API errors.
### Validation
- LightRAG confirmed the API signatures, `projectName` nullable pointer, and `API_ProjectInfo` ownership.
- `clang-format` and `git diff --check` passed.
- AC25 LSP compile database generated successfully.
- AC25 MSVC build succeeded.
- Runtime runner opened `Test_file/test_25.pln`, HTML validation passed, and `D:/SomeStuff_addon/test_results.txt` was created.
- The runner reported `UNKNOWN` for C++ test status because the generated file had no recognised status marker; it exited successfully.
### Scope
Only `Sources/AddOn/CommonFunction.cpp` and `IDEA.md` were changed. Existing uncommitted user changes were preserved.
### Checkpoint
No git commit created; the user did not request one.
## Parallel Task — #213 JSON-команда SyncAll (COMPLETED, issue #213 CLOSED 2026-09-27)
### Scope
Только новые `Sources/AddOn/json_commands/SyncAllCommand.hpp/.cpp`, регистрация в `json_commands/JsonCommandRegistrar.cpp`, справка `json_commands/How JSON Commands work.md`, карточка `Docs/modules/json_commands.md`, `Docs/REPOMAP.md` и строка в `Reviews/open-2026-09-12.tracker.csv`. `Sources/AddOn/Sync.cpp` не менялся (решение владельца); чужие незакоммиченные правки сохранены и в checkpoint не включены.
### Status
COMPLETED — issue #213 закрыт 2026-09-27.
### Last Completed
Созданы `SyncAllCommand.hpp/.cpp` (обёртка: `LoadSyncSettingsFromPreferences(syncSettings, true)` → `PROPERTYCACHE().Update()` → `SyncAndMonAll`), регистрация в `RegisterJsonCommands`; clang-format, clangd 0 диагностик, `BuildAddOn.py -v 25` и `restart_archicad_for_test.ps1` (build=True, Archicad запущен); `API.ExecuteAddOnCommand` на порту 19723 вернул `{"status":"returned","elapsedSeconds":4.1818899}`.
### Next Step
Нет. Следующие шаги по Sync при необходимости: #201, #202.
### Last Checkpoint
`97377dc` — `[#213] JSON-команда SyncAll` (`Refs: #213`).
### Validation
- Verified: контракт команды и факт регистрации — по исходникам `json_commands/`.
- Compiled: да, AC25, `BuildAddOn.py -c config.json -v 25` (build=True).
- Tested: да, AC25 — JSON-вызов на порту 19723 вернул `status=returned`/`elapsedSeconds=4.1818899`.
- AC26–29: `not verified` (сборка и runtime только на AC25).
### Decisions
- Команда вызывает только `SyncAndMonAll`: `DimRoundAll`, `WriteSyncSettingsToPreferences` и обновление меню из пункта меню `SyncAll_CommandID` не воспроизводятся.
- `Sync.cpp` не правится — `SyncAndMonAll` остаётся `void`, прогресс-окно и фазы сохраняются; отсюда контракт ответа только `{status, elapsedSeconds}` (согласовано после уточнения о недоказуемости `skippedByReset`/`elementsToWrite` без правки `Sync.cpp`).
- `LoadSyncSettingsFromPreferences(syncSettings, true)` — у внешнего вызова нет интерфейса для смены флагов обхода; `PROPERTYCACHE().Update()` — паритет с `MenuCommandHandler`.
---
## Parallel Task — #215 конфигурация сборки ProfileDebug (COMPLETED, issue #215 CLOSED 2026-09-27)
### Scope
Только `CMakeLists.txt`, `Tools/CMakeCommon.cmake` (флаги, линковка, вывод артефакта) и `Tools/BuildAddOn.py` (запуск новой конфигурации). Кода в `Sources/AddOn/` не трогал.
### Status
COMPLETED — issue #215 закрыт 2026-09-27 (критерий `VSInstr /DUMPFUNCS` подтверждён, загрузка артефакта в AC25 проверена пользователем).
### Last Completed
Checkpoint `0549622`. `CMAKE_CONFIGURATION_TYPES` + `ProfileDebug` (только `CMAKE_HOST_WIN32`); флаги `/O2 /Gy /Gw /Zi` + `/wd4724` и `-DDEBUG`/`-DTESTING` через `$<OR:...>`; линковка `/PROFILE /DEBUG /INCREMENTAL:NO`; `RUNTIME_OUTPUT_DIRECTORY_PROFILEDEBUG` → `${CMAKE_BINARY_DIR}/Debug`; `BuildAddOn.py --profile` + копирование `test_<ver>.pln`. Сборка `BuildAddOn.py -c config.json -v 25 --profile` успешна, `.apx` и `.pdb` в `Build/SomeStuff/25/Debug/`, `VSInstr /DUMPFUNCS` вернул 50 966 функций (16 заглушек — `ACAP_STAT.lib` DevKit без `API_c.pdb`).
### Next Step
Нет. Дальше по профилированию — план `.hermes/plans/2026-09-27-addon-profiling-plan-v2.md`, шаг 3.
### Last Checkpoint
`0549622` — `[#215] Сборка: конфигурация ProfileDebug (/PROFILE, вывод артефакта в Debug)`.
### Validation
- Verified: критерий issue — `VSInstr /DUMPFUNCS` возвращает список функций вместо ошибки про `/PROFILE`.
- Compiled: да, AC25 (Windows), конфигурация ProfileDebug, `BuildAddOn.py -c config.json -v 25 --profile` (build=True).
- Tested: да, артефакт загружается в AC25 (пользовательская проверка); реальные замеры — вне объёма #215.
### Decisions
- `ProfileDebug` выводит `.apx` и `.pdb` в общую папку `Debug` (решение владельца): Add-On Manager грузит `.../25/Debug/SomeStuff.apx`, поэтому артефакт подхватывается сам. Перезапись рабочей Debug-сборки — намеренная, суффикс к имени не добавляется.
- `ProfileDebug` включается только на Windows (`CMAKE_HOST_WIN32`): смысл конфигурации в MSVC-флагах, аналога на macOS нет.
- `/GL` не задаётся: требует LTCG-метаданных во входных `.lib` DevKit, при `/WX` предупреждения LTCG фатальны.
### Снято
- Headless-подключение профилировщика VS к процессу даёт 0 образцов — не повторять, для замеров нужен другой путь (WPR/ETW, требует прав администратора). Это к способу замера, а не к конфигурации сборки.
---
## Parallel Task — #216 хэш коммита в ADDON_SUBVERSION (COMPLETED, issue #216 CLOSED 2026-09-27)
### Scope
Только `Tools/CMakeCommon.cmake` (подверсия в `GenerateAddOnProject`) и `IDEA.md`; кода в `Sources/AddOn/` не трогал. Проверочная версия AC25 (Windows).
### Status
COMPLETED — issue #216 закрыт 2026-09-27 (AC25 build + пользовательская проверка версии).
### Last Completed
`find_package(Git QUIET)` + `git rev-parse --short HEAD` в `CMAKE_SOURCE_DIR`; при отсутствии git/репозитория `unknown`. `ADDON_SUBVERSION` = `<YYYY-MM-DD-HH>-<хэш>`; configure печатает `Building from commit: 72a5088`. Сборка через `restart_archicad_for_test.ps1` успешна (первый прогон `BuildAddOn.py` упал на LNK1168 — Archicad держал `.apx`); строка `1.78 2026-09-27-18-72a5088` есть в `RINT/AddOn.grc` и в `.apx` (3 вхождения).
### Next Step
Нет. Помнить: хэш читается на этапе configure, после новых коммитов без повторного configure он не обновится.
### Last Checkpoint
`780f508` — `[#216] CMakeCommon: хэш коммита в ADDON_SUBVERSION`; `6713c75` — запись в IDEA.md. `Refs: #216`
### Validation
- Verified: строка версии в `RINT/AddOn.grc` и в собранном `.apx` (3 вхождения).
- Compiled: да, AC25 (Windows), `BuildAddOn.py` + `restart_archicad_for_test.ps1` (build=True).
- Tested: да, версия проверена в AC25 пользователем.
---
## Parallel Task — #214 перенос наработок из официального шаблона сборки (пункт 1 COMPLETED, issue #214 CLOSED 2026-09-27)
### Scope
Только `Tools/CMakeCommon.cmake`, `CMakeLists.txt`, `config.json`, `Tools/AddOn.grc.in`. Кода в `Sources/AddOn/` не трогал. Локализация JSON→grc+XLIFF и code signing macOS исключены по решению пользователя.
### Status
COMPLETED (пункт 1) — issue #214 закрыт 2026-09-27 по пункту 1; пункты 2–7 вынесены в **#218** (открыт), состояние перенесено туда.
### Last Completed
Пункт 1: `version`/`description`/`copyright` в `config.json` (версия числом `1.78`, без `v`), `parse_version` + `ReadConfigJson` в `CMakeCommon.cmake`, экспорт `ADDON_VERSION` двумя каналами, хардкод `v1.78` в `CMakeLists.txt:5` убран. AC25 build успешен, версия `1.78 2026-09-27-15` подтверждена в `RINT/AddOn.grc` и в `.apx`, compile DB регенерирован. Регрессия `7dd2310`: сборка падала на другой машине (`charmap` 0x8f) — `config.json` читался без кодировки; кириллическое `description`, добавленное в `3c9e14c`, вскрыло скрытый баг.
### Next Step
Пункт 2 в #218: `Tools/VersionInfo.rc.in` + `Tools/AddOn.rc.in`, подключение к target, `AC_ADDON_FOR_DISTRIBUTION`. Перед правкой `-r/--release` согласовать семантику «метка для дистрибуции» (сейчас = RelWithDebInfo + все языки).
### Last Checkpoint
`7dd2310` — `[#214] BuildAddOn.py: явная кодировка UTF-8 при чтении config.json` (предыдущий: `3c9e14c` — версия из config.json).
### Validation
- Verified: `parse_version`/`ReadConfigJson` и оба канала `ADDON_VERSION` — по исходникам; версия в `RINT/AddOn.grc`.
- Compiled: да, AC25, `BuildAddOn.py -c config.json -v 25` (build=True).
- Tested: не в этой сессии — версия видна в UI по сообщению пользователя о рабочей сборке.
- Пункты 2–7: `not verified` — не реализованы, перенесены в #218.
### Decisions
- `Tools/` — форк upstream, сплошная синхронизация запрещена: сломает AC22–24, `-DTESTING` (`TestFunc.cpp`/`DBprnt`), `/W3`, маркеры `AI_STATUS` раннера и `-DAC_${acVersion}` (цепочка `api_headers/APICommon<N>.h`).
- Локализация JSON→grc+XLIFF и code signing macOS не берутся по решению пользователя; глоб `*.json` из исходников тоже не берём.
- Переход `/W3` → `/W4` вынесен из issue: слишком рискован при `/WX`, отдельный follow-up.
- `-r/--release` (`RelWithDebInfo` + все языки) не переименовываем в «метку для дистрибуции» молча — семантика решается при пункте 2.
- Версия — число без `v`: upstream `parse_version` принимает только `^([0-9]+)(\.[0-9]+){0,2}$`, а каждая компонента 0–65535 попадает в `FILEVERSION`.
- Нужны **оба** канала `ADDON_VERSION`: CMake-переменная для `configure_file` → `RINT/AddOn.grc` (компилятор туда не подставится) и `target_compile_definitions` для C++.
### Снято (не возвращать в IDEA.md)
Три грабли по сборочным файлам дублировали durable-знание и перенесены в скилл `archicad-plugin-build` → `references/cmake-json-metadata-pitfalls.md`: `open()` в `Tools/*.py` без `encoding` = кодировка локали ОС; CMake `string(REPLACE)` требует 4 аргумента (match, replace, output, **input**); вложенный путь JSON в списке CMake разъезжается — читать отдельными вызовами `string(JSON ... GET)`.
---
## Archive — диагностика отсутствия элементов SpecAll (AC25) (DIAGNOSED, перенос 2026-09-27)
### Scope
Только диагностика JSON Spec/SpecAll на тестовом PLN через VS Debug и путь чтения материалов; производственный код и задачи #209/ReadQuantities не менялись.
### Status
DIAGNOSED — в текущей AC25-сборке элементы не создаются из-за неполных выходных параметров правила материалов; код не менялся.
### Last Completed
JSON `SomeStuffCommand.Spec` вернул `status=failed`, `resultCode=-2130313215`, `elementsToCreate=0`, `elementsToModify=0`, `elementsToDelete=0`. Под отладчиком в `SpecArray` после обработки правил `n_elements=0`, `elements_new=0`, `elements_mod=0`, `error_element=0`; `Spec.cpp:989-996` возвращает ошибку для пустого списка. Для стены `4041C8C1-CEB0-484B-A310-43EC545F6FC9` шаблон «Материалы/Обозначение в спецификацию» не подставился ни в одном из 6 слоёв (`ReadMaterial`, `Helpers.cpp:7743-7782`, `flag=false`). Состав из 6 слоёв присутствует, но нужный `ParamValue` имеет `fromPropertyDefinition=true`, `fromAttribDefinition=false`, `isValid=false`; `paramsAdd` пуст. `GetAttributeValues` пропускает параметр при `!fromAttribDefinition` (`Helpers.cpp:9756`), а `GetParamValueForElements` отклоняет невалидный источник (`Helpers.cpp:3979-3987`). В `GetElementsForRule` прочитано 6 из 7 выходных параметров, условие полного набора (`Spec.cpp:1664-1682`) не добавляет кандидата. Балка и нулевые флаги количеств к этой установленной причине не привязаны.
### Next Step
Отдельно в рамках #209 выяснить, где должен устанавливаться `fromAttribDefinition` для указанного свойства материала, и проверить исправление на AC25 без обходного расширения `GetAttributeValues`. Исходники в этой диагностике не менялись.
### Last Checkpoint
Нет: диагностика без правок производственного кода.
### Проверка гипотезы «кэш / вариативность флага» (2026-09-27)
Ответ: **на тестовом PLN вариативности нет, а кэш тут ни при чём.** Пять запусков подряд (4 без брейков) дали идентичный результат: `status=failed`, `resultCode=-2130313215`, `elementsToCreate=0`, `elapsedSeconds` 0.4471/0.4471/0.4510/0.4493. Расхождения времени в прошлой сессии (122 с против 0.46 с) объясняются простоем ArchiCAD на брейкпоинтах, а не состоянием кэша: 122 с включали остановки в Break-режиме.
Что проверено наблюдением (VS Debug, AC25, `test_25.pln`):
- Кэш свойств читается полностью: `PROPERTYCACHE().property.GetSize()=519`, `isPropertyDefinitionRead_full=true`, `isPropertyDefinition_OK=true`. Кэш не пуст и не частичен — версии «кэш не успел прогреться» нет.
- `GetAllPropertyDefinitionToParamDict` фильтрует группы по `group.name.Contains("Material")` либо по трём жёстким GUID (AC25, ветка `#else`) — `Propertycache.cpp:760-767`. От того, какая ветка активна, зависит состав кэша, но флаг `fromAttribDefinition` ставится не здесь, а в `ConvertToParamValue_CheckAttrib` по тексту описания свойства.
- Остановка на `Helpers.cpp:9756` (фильтр `!fromAttribDefinition`) показала **не** наше свойство, а material-слот: `typeinx=11` (`MATERIALTYPEINX`, `Constants.hpp:298`), `fromMaterial=true`, `fromPropertyDefinition=false`, `fromAttribDefinition=false`, `definition.guid=APINULLGuid`. Такой слот отбрасывается фильтром штатно, к делу он не относится.
- `rule.stop_on_error=false`, поэтому элементы с неполным набором отбрасываются молча (`Spec.cpp:1676-1682` — ветка `else` без записи в `error_element`). `not_found_paramname=0` — причина не регистрируется в отчёте.
- Итог по правилу: `rule.out_paramrawname=7` против `element.out_param=6`, `rule.out_sum_paramrawname=2` равно `element.out_sum_param=2`. Не хватает ровно одного выходного параметра, и это «Материалы/Обозначение в спецификацию».
Механизм (статически, по коду): флаг `fromAttribDefinition` присваивается исключительно в `ParamHelpers::ConvertToParamValue_CheckAttrib` (`Helpers.cpp:8348`, ветки 8367/8371/8375/8383/8391/8395/8399/8403/8407/8414) — по совпадениям в `definition.description` (`SYNCNAME`, `buildingmaterial`, `component`, `some_stuff_*`) и в `pvalue.rawName` для `{@property:buildingmaterialproperties/...}` (`Helpers.cpp:1381-1382`). Шаблон `%Материалы/Обозначение в спецификацию%` не содержит ни одного из этих признаков, поэтому для него флаг не выставляется ни при одном пересчёте кэша — вариативности быть не может, условие детерминированное. Второй путь, `ParseParamName` (`Helpers.cpp:3796-3799`), для `PROPERTYTYPEINX` ставит только `fromPropertyDefinition`, а затем `CompareParamDictValue` (`Helpers.cpp:7423`, вызов из `ReadMaterial_ReadAddParam`) копирует в `paramsAdd` состояние из `PROPERTYCACHE().property` — то есть флаг приходит из кэша, но значение кэша детерминированно.
Значит «иногда создаются» на этом проекте объясняется не кэшем и не флагом. Возможные иные причины (не проверены, требуют отдельной проверки): другой PLN/правило; состояние правила, заданное вручную в UI до запуска; в прошлых прогонах результат зависел от того, что Archicad стоял на брейкпоинте, а не от данных.
Свойства чтения в отладчике, которые сработали: `param.typeinx`, `param.from*`, `param.definition.guid.time_low`, `params.GetSize()`, `PROPERTYCACHE().property.GetSize()`, `element.out_param.GetSize()`, `rule.out_paramrawname.GetSize()`. Не сработали: `param.rawName.ToCStr()` и `params.ContainsKey("строка")` — VS требует явный `GS::UniString(...)`; `GetPtr(...) != nullptr` — «неизвестное выражение». Для вывода строки использовать `get_locals` (там имена ключей печатаются) либо сравнения с литералом.
## #218 перенос наработок официального шаблона сборки (пункты 2–7) — COMPLETED, issue CLOSED 2026-09-27
### Scope
Только `Tools/CMakeCommon.cmake`, `Tools/VersionInfo.rc.in` (новый), `Tools/AddOn.rc.in` (новый), `Tools/BuildAddOn.py`, `Tools/AddOn.plist.in`, `CMakeLists.txt`. Кода в `Sources/AddOn/` не трогать. Проверено на AC25/AC26 (Windows).
https://github.com/kuvbur/AddOn_SomeStuff/issues/218
### Status
COMPLETED — issue #218 закрыт 2026-09-27 по подтверждению владельца (сборка прошла). Changelog: `d815152` (правки) + `704877a` (IDEA). macOS-сборка и AC22–24 — `not verified`.
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
Нет — задача закрыта. macOS-сборка и AC22–24 остались `not verified`; при их появлении проверять `Info.plist` (`LSMinimumSystemVersion` из DevKit вместо 10.15) и bundle identifier.
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
- [x] Issue #218 закрыт 2026-09-27 по подтверждению владельца (сборка прошла). macOS-сборка и AC22–24 — `not verified`.
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
## Archive — блоки, закрытые коммитами 2026-09-28 (перенос из IDEA.md)
Перенесено после коммитов `9d96d96` (#220), `1c89817`/`5d2c86b` (#222), `ede1fff` (#224), `66666f1`/`a396613` (#223). Issues #220, #222, #223, #224 — CLOSED.
# --- #209 наименование материала в SpecAll ---
## Task
#209: причина отсутствия наименования материала в JSON Spec (AC25). Кандидат в регрессию найден и откачен: оптимизации из #211 в материальном пути (`readMaterialOnce` в `ComponentsProfileStructure`, новая ветка генерации `&N&` в `ReadMaterial`). Правки в рабочем дереве, не закоммичены.
https://github.com/kuvbur/AddOn_SomeStuff/issues/209
https://github.com/kuvbur/AddOn_SomeStuff/issues/211
## Scope
Только путь происхождения и чтения свойства материала в `Sources/AddOn/Helpers.cpp`, связанные проверки и карточка `Docs/modules/Helpers.md`. Параллельные задачи не менять. AC25 — проверочная версия.
## Status
COMPLETED — правка (`ConvertToParamValue_CheckAttrib`, признак материала по `description`, `Helpers.cpp:8378`) и осознанный откат оптимизаций #211 вошли в `66666f1`; issue #209 CLOSED 2026-09-28 с замерами: elementsToCreate 0→2, INCOMPLETE 159→0, 4 прогона.
## Last Completed
2026-09-27 — **правка внесена и подтверждена измерением.** В `ConvertToParamValue_CheckAttrib` (Helpers.cpp, после блока `component` ~8369) возвращена удалённая `91cff91` ветка, текст взят из `git show 91cff91^:Sources/AddOn/Helpers.cpp` и сверен `diff` — **идентичен оригиналу**.
**Функциональное сравнение функции с рабочей версией** (запрошено пользователем, чтобы исключить побочные поломки в `91cff91`): извлечение по сигнатуре из обеих ревизий, нормализация пробелов, `difflib.unified_diff` → 2 отличия, оба косметические из того же `91cff91`:
- `definition.description.ToLowerCase ()` → `description` + `SetToLowerCase ()` — результат идентичен;
- `description.Contains ("sync_name")` → `description.Contains (SYNCNAME)` — константа `Constants.hpp:378` = `"sync_name"`, значение идентично.
Других различий нет — **побочных поломок в функции не найдено**.
**Результат (AC25, `test_25.pln`, 4 прогона подряд):**
| | до правки | после |
|---|---|---|
| `status` | `failed` | `completed` |
| `resultCode` | -2130313215 | 0 |
| `elementsToCreate` | 0 | **2** |
| `INCOMPLETE out=6/7` | 159 | **0** |
| `elapsedSeconds` | ~0.45–2.8 | 0.124–0.134 |
Детерминировано: 4 прогона → `2, 2, 2, 2`, без брейков, расхождений нет.
**Побочные наблюдения:**
- `ReadQuantities err` — 96 → 30 (были и до правки, это отдельная проблема; 48 предупреждений «Old method GUID elem» тоже были).
- `WriteProperty err` — **новая**, 1 на прогон: `!ConvertToProperty {@property:спецификации материалов/собственный этаж}`. Свойство не конвертируется; не связано с материалом, но это отдельный дефект — в объём #209 не входит, требует отдельной задачи.
- В рабочем дереве в `Helpers.cpp:5967` стоит `param.val.uniStringValue = EMPTYSTRING;` вместо голого `param.val.uniStringValue;` из HEAD (`9676fee`) — **это правка не моя**, оставлена как есть.
**Опровергнуто** (каждая — измерением): кэш/вариативность; «Наименование в спецификацию» как потерянное; откат `ccf9cae` (#211) — 3 прогона дали те же `elementsToCreate=0`; неполный разбор `definition`.
Трассировка удалена полностью, `clang-format` выполнен, `git diff --check` = 0.
## Next Step
Issue #209 закрыть с комментарием, указав `66666f1` (правка уже в истории; сам issue открыт, так как коммит не ссылался на него). Затем перенести блок #209 в `IDEA_ARCHIVE.md`. `Docs/modules/Helpers.md` — карточка обновлена в `66666f1`; проверить, что в ней описан возвращённый признак материала по `description`.
## Last Checkpoint
`66666f1` — [#223] содержит и правку #209 (признак материала по `description`, `Helpers.cpp:8378`), и осознанный откат оптимизаций #211. `9d96d96` — [#220]. `1c89817`/`5d2c86b` — [#222]. `ede1fff` — [#224]. `a396613` — состояние задачи #223.
## Plan
- [x] Найти и откатить регрессию `ccf9cae` (#211): `readMaterialOnce` + генерация `&N&`; гипотеза позднее опровергнута измерением.
- [x] Проверить семантику `ParamComposite::isValid` — поле мёртвое, проверка в `Spec.cpp` убрана пользователем.
- [x] Runtime-трассировка полного пути чтения материала через `DBprnt` → панель «Отладка»: определён теряющийся параметр и точка отказа.
- [x] Трассировка `templatestring`, `flag` и `params` — ключ есть, но `isValid=0` и без суффикса; `flag=0` у единственного односоставного шаблона.
- [x] Трассировка `definition` — определение непустое, `fromAttribDefinition=0`, описание содержит `buildingmaterialproperties`.
- [x] История git: рабочая ветка `description.Contains ("{@property:buildingmaterialproperties}")` была удалена коммитом `91cff91` (2026-07-12) → регрессия.
- [x] Вернуть удалённую ветку; функциональное сравнение с `91cff91^` — 2 косметических отличия, побочных поломок нет.
- [x] Сборка + 4 прогона Spec на `test_25.pln`: `elementsToCreate: 2`, `status: completed`, `INCOMPLETE` = 0.
- [x] Checkpoint `66666f1` — правка и откат #211 вошли в коммит по решению пользователя («откат осознанный»).
- [/] Закрыть issue #209 с указанием `66666f1` и перенести блок в архив.
## Decisions
- Ранее предложенная причина КЖ/Favorite не относится к тестовому проекту с правилом АР (уточнение пользователя).
- `GetAttributeValues` должен читать только `fromAttribDefinition`; расширение на `fromPropertyDefinition` — обход, а не исправление причины.
- Оптимизации, меняющие семантику (дедупликация вызовов, смена шаблона подстановки), неприемлемы без отдельного регрессионного теста на мультислойной конструкции — даже если формально ускоряют.
- `ParamComposite::isValid` — мёртвое поле; при необходимости семантики «состав прочитан» следует завести отдельное поле, реально заполняемое в `Components*`.
# --- #220 GetParamValue ---
## Parallel Task — #220 GetParamValue (AC25)
### Scope
`Sources/AddOn/spec/Spec.cpp::GetParamValue`: TODO GetPtr и проверка граничных состояний; регрессионные тесты в TestFunc, карточка Spec. Чужие правки сохранить. Публичную сигнатуру и поведение успешного пустого результата не менять. Проверочная версия AC25 (Windows).
### Status
DONE_AND_TESTED_AC25 — #220 закрыт по правке; 43 проверки `TestSpecGetParamValue` в runtime AC25: 43 OK / 0 ERROR. Создан #221 (отдельная находка, вне объёма). Checkpoint не сделан: пользователь не запрашивал, дерево содержит чужие правки.
### Last Completed
`GetParamValue` (`Spec.cpp:1389-1501`): TODO выполнен — 4 двойных `ContainsKey`+`Get` заменены на `GetPtr` с проверкой nullptr; `pvalue.isValid = false` на входе и повторно перед выбором слоя материала; `n_layer < 0` отсекается до `composite[n_layer]` и до `ListData::AddLibdataToParamValueDict`; у всех трёх путей «успешного пустого результата» добавлен `canCalculate = false`, ветке «слой за концом» возвращён `isValid = true` (было потеряно при правке `uniStringValue = EMPTYSTRING`). `GetPtr` сверен по `Build/DevKit/APIDevKit-25/Support/Modules/GSRoot/HashTable.hpp:756` (const-форма возвращает nullptr при отсутствии ключа); LightRAG на два запроса по GSRoot отдал `No relevant context found`, использованы установленные заголовки. Снятые из дерева изменения (`msg_rep` при 0 элементах, рефакторинг `PlaceElements`, `pcelem.isValid` — мёртвое поле по решению #209) не трогались.
RED→GREEN: `TestSpecGetParamValue` (`TestFunc.cpp:55-142`), 43 DBtest. До правки 24 OK / 11 ERROR (остаточный `isValid`, `canCalculate` у пустого результата, отрицательный индекс). После правки 43 OK / 0 ERROR в панели VS «Отладка» от 21:53:45. Запуск шёл отдельной строкой из `SomeStuff_Main.cpp` (общий набор у пользователя отключён); временный вызов удалён, добавлен в `TestFunc::Test`.
2026-09-27 позже: `BuildAddOn.py -c config.json -v 25` — успешно (канонический путь §9, `AI_BUILD_RESULT status=success`, apx 22:43:44). Первая попытка упала на `C4018`/`C2228` в `Helpers.cpp:6905-6910` — временная трассировка `t209` из #209, снята пользователем между прогонами; вторая — на `MSB4018` по заблокированному `.tlog` (см. Грабли). Формат `Docs/_generated/symbols.json` приведён к стилю HEAD: LF, без BOM, без завершающего LF, 8599 записей; расхождение с `git` — `LF will be replaced by CRLF` из-за `* text=auto`, норма для репозитория. `symbols.json` пересобран **только** по `Spec.cpp` и `TestFunc.cpp`; записи `Helpers.cpp`/`TableRenderer.cpp` остались от HEAD и не соответствуют текущему дереву — обновит владелец #209 при своём чекпойнте.
### Next Step
Checkpoint по запросу пользователя (в дереве смешаны правки `Helpers.cpp` — #209, `SomeStuff_Main.cpp` — отключение набора, `Spec.cpp` — правка #220). #221 (double→Int32 без проверки диапазона, статическая находка) ждёт решения пользователя.
### Last Checkpoint
Нет — пользователь не запрашивал commit; смешанное дерево.
### Plan
- [x] Проверить текущую функцию и контракты, создать https://github.com/kuvbur/AddOn_SomeStuff/issues/220.
- [x] Регрессионные проверки (RED 11 ошибок) и минимальные исправления.
- [x] clang-format, clangd 0, сборка AC25, runtime 43 OK / 0 ERROR, карточки Spec.md и TestFunc.md, точечное обновление `symbols.json` для двух файлов (callgraph не тронут, генератор его обнуляет).
- [x] Checkpoint `9d96d96` — [#220]; #221 остаётся отдельной находкой.
# --- #222 WriteGDL ---
## Parallel Task — #222 WriteGDL APIERR_BADPARS при группировке
https://github.com/kuvbur/AddOn_SomeStuff/issues/222
### Scope
`Sources/AddOn/CommonFunction.hpp` (`SuspendGroupsGuard`), `Sources/AddOn/Helpers.cpp` (`ElementsWrite` + порядок вызовов в `WriteGDL`), замена ручного кода `SuspendGroups` в `ReNum.cpp`/`Roombook.cpp`/`Summ.cpp`/`Sync.cpp`, карточки `CommonFunction.md`/`Helpers.md`. AC25 — проверочная версия. Правки других задач в дереве не трогать.
### Status
DONE_TESTED_AC25 — #222 закрыт по правке. Подтверждено пользователем в ArchiCAD: запись работает, `APIERR_BADPARS` ушёл. Сборка AC25–29 успешна; AC22–24 не проверены (нет DevKit).
### Last Completed
2026-09-27 — причина установлена. Регрессия `2a48348` (2026-09-12, ревью): восстановление тумблера `SuspendGroups` добавлено **после** `ElementsWrite` (`Spec.cpp:1042-1051`). До этого коммита режим suspend включался перед записью и **никогда не выключался** (v1.77: `Sources/AddOn/Spec.cpp:714/717` — только включение), поэтому запись всегда шла с приостановленной группировкой. После `2a48348` режим восстанавливается → на последующих прогонах `suspGrp == true` → `if (!suspGrp)` ложно → `APITool_SuspendGroups` не включается → запись идёт при активной группировке. Симптом ограничен элементами SpecAll: только этот путь создаёт элемент (`Spec.cpp:2651`) и группирует его (`:2673-2684`) в том же тике, до `ElementsWrite` (`:1040`).
Правки: `SuspendGroupsGuard` в `CommonFunction.hpp:211-260` (по образцу `ProcessWindowGuard`; если режим включён — выключает, в деструкторе восстанавливает; копирование запрещено; `APITool_SuspendGroups` — переключатель, массив GUID игнорируется — DevKit `ACAPI_Element_Tool ({}, APITool_SuspendGroups, nullptr)`); создание guard в `ElementsWrite` (`Helpers.cpp:4594`). Дополнительно возвращён порядок v1.77: `CloseParameters` перед `ACAPI_Element_ChangeMemo` (`Helpers.cpp:5290-5298`).
**Снято в ходе разбора** (были ложные гипотезы, все проверены по истории и сняты): порядок вызовов как причина `BADPARS` (Tapir пишет при открытом сеансе — но через `ACAPI_Element_Change`, а не `ChangeMemo`; экстраполяция была ошибкой); `CHTruncate`/`BNZeroMemory`/`ChangeMemo`/`GetGDLParametersHead` — побайтово неизменны с v1.77 по 23 ревизиям; блок `APIParT_CString` менялся (`c0bd9e3`/`eda305a`/`cf81d31`/`11add7a`), но все правки чинят утечку `new[]` и переполнение, а не регрессия; удалённый `ccf9cae` вызов `ACAPI_Element_GetHeader` — дублирующий, эквивалентен `element.header`; навесная стена исключена пользователем (ошибка на обычном объекте).
### Next Step
Прогнать `Tools/restart_archicad_for_test.ps1` на `test_25.pln` и прочитать панель «Отладка» VS: пропал ли `WriteGDL … APIERR_BADPARS`. Если остался — следующий кандидат назван: сам `ACAPI_Element_ChangeMemo` вне документированного контракта (DevKit 25: только `APIMemoMask_Polygon`, рекомендован `ACAPI_Element_Change`; Tapir и `Roombook.cpp:4980`, `pk/Revision.cpp:1078` используют его). Третий кандидат — двойная запись GDL в `PlaceElements` (пишет в memo при создании `:2598-2649`, остаток уходит в `paramOut` и пишется снова).
Отдельно, вне объёма: замена `ACAPI_Element_ChangeMemo` на `ACAPI_Element_Change` — не минимальный диф (у навесной стены `elemGuidt` = `symbolID`, а `Element_Change` меняет сам элемент).
### Last Checkpoint
Создаётся этим коммитом, `Refs: #222`. В коммит вошли только правки #222; смешанные правки других задач (#209, #220, отключение набора в `SomeStuff_Main.cpp`, формат `Sync.cpp`/`Spec.cpp`, `TestFunc.*`, `TableRenderer.cpp`, `_generated/symbols.json`) оставлены в дереве.
### Plan
- [x] Сравнить `WriteGDL` с v1.77 и по 23 ревизиям — регрессии внутри функции нет.
- [x] Сверить запись GDL у Tapir (LightRAG cpprag :9621) — использует `ACAPI_Element_Change`, а не `ChangeMemo`.
- [x] Локализовать путь: только элементы SpecAll, созданные и сгруппированные в одном тике.
- [x] Найти регрессию `2a48348` (восстановление `SuspendGroups` после `ElementsWrite`).
- [x] `SuspendGroupsGuard` + подключение в `ElementsWrite`; порядок v1.77 в `WriteGDL`; clang-format, LSP 0, сборка AC25.
- [x] Runtime-прогон AC25: подтверждено пользователем — запись работает, `APIERR_BADPARS` ушёл.
- [x] Checkpoint с `Refs: #222` (только правки #222, разбор смешанного дерева).
### Decisions
- Семантика флага: `suspGrp == true` означает «приостановка группировки включена», то есть группировка **отключена** и запись проходит — guard ничего не делает. Переключать надо только при `suspGrp == false` (группировка активна). Флаг `enabled` внутри guard означает «переключение сделали мы», инициализируется `false` и выставляется `true` только после успеха — иначе любой ранний выход приводил бы к выключению режима, который мы не включали.
- Гейт версии — `ServerMainVers_2300` (в AC22 `APIEnv_IsSuspendGroupOnID` не существует), внутри AC27+ против AC23–26. Гейт `ServerMainVers_2700` без внешнего `#ifdef ServerMainVers_2300` не компилировался бы на AC22.
- `APITool_SuspendGroups` вызывается с пустым массивом GUID: контракт DevKit — переключатель, массив игнорируется; в прежнем коде передавался `elements_delete`/`rereadelem`/`guidArray`, что на результат не влияло.
- Guard ставится в `ElementsWrite`, а не в `WriteGDL`: он покрывает все типы записи (свойства, ID, классификация, атрибуты, координаты) единообразно и освобождается на выходе из функции. Из-за него ручной код в `ReNum.cpp`/`Roombook.cpp`/`Summ.cpp`/`Sync.cpp` удалён — двойного переключения нет.
- `Spec.cpp:1021-1054` оставлен как есть: там `ElementsWrite` уже обёрнут собственным (несимметричным) переключением, а guard внутри `ElementsWrite` перекрывает его корректно. Чистка `Spec.cpp` — отдельная задача.
# --- #224 синтаксис Spec.cpp ---
## Parallel Task — #224 синтаксис Spec.cpp (AC25)
### Scope
Только убрать лишнюю закрывающую `}` перед `else` в `Sources/AddOn/spec/Spec.cpp:1017-1018` в базовом HEAD; не включать добавленный другим WIP `msg_rep` и остальные незакоммиченные правки. Отдельный issue https://github.com/kuvbur/AddOn_SomeStuff/issues/224; пользователь разрешил отдельный минимальный checkpoint перед #223.
### Status
DONE — checkpoint `ede1fff`; чужие правки восстановлены из stash `d5b18a75...` побайтово (Spec.cpp отличается от снимка только удалённой строкой #224).
### Last Completed
Правка изолирована одним hunком, чужой WIP сохранён stash `d5b18a754bb9707248eaa5720b0d69580502732a`. На изолированном дереве: `BuildAddOn.py -v 25` → `AI_BUILD_RESULT status=success`, `restart_archicad_for_test.ps1` → `AI_RESULT status=success exit_code=0 build=True archicad=running` (JSON-скрипт отсутствует, `JSON_TESTS_SKIPPED` — не пропущенные, а отсутствующие тесты). Commit `ede1fff` содержит только удаление строки. `git blame` уточнил источник: скобка добавлена в `1c89817` (#222); HEAD до него был корректен. Issue #224 создан и проверен GitHub.
### Next Step
Возобновить #223: заново stage только Helpers.cpp и документацию, изолировать WIP, собрать AC25 и выполнить runner с read-back по разнице GUID на свежом тестовом PLN; замеры производительности не проводить.
### Last Checkpoint
`ede1fff` — [#224] Spec: убрать лишнюю скобку перед else (AC25 собран, runner exit 0).
### Plan
- [x] Подтвердить ошибку базового Spec.cpp изолированной сборкой и создать issue #224.
- [x] Подготовить минимальный индекс без чужого WIP.
- [x] Проверить сборку AC25 на точном дереве #224 и сделать отдельный checkpoint.
- [x] Восстановить чужие изменения и передать управление #223.
# --- #223 GUID родителей ---
## Parallel Task — диагностика GUID родителей в SpecAll (AC25)
### Scope
Исправить потерю `{@property:спецификации материалов/связанные элементы}` у созданных объектов SpecAll (AC25) в `Helpers.cpp::WriteProperty`: восстановить путь уже известного определения свойства, не меняя остальные ветки и чужие правки. Функциональная проверка — сборка и read-back; тестирование производительности исключено по запросу пользователя. Основание: `zadacha_spec_guid.md` в Hermes scratch; issue #223.
### Status
DONE — checkpoint `66666f1` (совместно с чужым WIP по решению пользователя); read-back подтвердил непустое свойство у 34 из 34 созданных элементов. Откат части #211 в Helpers.cpp сделан осознанно, отдельное issue не заводилось.
### Исходная диагностика
Два вызова SpecAll завершились `status=completed`, `elementsToCreate=34`, `resultCode=0` на дереве, содержащем чужой незакоммиченный WIP (см. «Последняя проверка»: на чистом дереве этот же вызов не создаёт элементов). Для свойства `Спецификации материалов/Связанные элементы` (GUID определения `D6A4FC26-C303-452C-83AD-35BB29998CFC`) в `Spec.cpp:2529-2536` сформирована строка четырёх GUID и `paramTo.isValid=true`, `paramTo.definition.guid.time_low=3601136678`, но `paramTo.property.definition.guid.time_low=0`. На `Helpers.cpp:5360` у того же параметра и нового элемента (`FBB23CC3-792F-4EB0-9131-016B08C475B1`, второй прогон `13FD3928-7F20-4902-91C8-FB740B3DC4F5`) шаг через условие `if (param.definition.guid == APINULLGuid)` перескочил к :5376: размер `propertyDefinitions=0`, `property2write=0` для него. Это определение заполнено (из `Propertycache.cpp:251-265,767-885`, `Helpers.cpp:8425-8444`), но свойство ещё не загружено; `WriteProperty` игнорирует именно такую комбинацию. Read-back обоих новых элементов через официальный JSON API AC25 (`GetPropertyIds` → `GetPropertyValuesOfElements`) вернул `type=string,status=normal,value=""`. Прежняя гипотеза о сбое обратного поиска имени на :5391-5399 для GUID опровергнута: он не достигается. Git blame: условие :5360 добавлено в `a6feb208` (2026-07-07), но дата первого проявления не установлена. VS Debug остановлен, свои точки удалены, 4 чужие сохранены. Тестовый PLN изменён только в памяти запущенного под VS экземпляра; экземпляр завершён без правки файлов репозитория. LightRAG ответа не дал (local/naive/hybrid); контракт read-back подтверждён DevKit 25 и wheel `archicad==25.3000`.
### Last Completed
После правки в `Helpers.cpp:5375-5376` `clang-format -i` и clangd (0 ошибок/предупреждений), сборка `BuildAddOn.py -v 25` и `restart_archicad_for_test.ps1` завершились успешно. В запущенном `test_25.pln` загружен `Build/SomeStuff/25/Debug/SomeStuff.apx`; вызов Spec создал 34 объекта. Read-back `GetPropertyValuesOfElements` после вызова: 582 объекта всего, ровно 34 со строкой GUID, 0 с пустой строкой; у последних 34 объектов значения непустые (проверен синтаксис GUID), 548 — `notAvailable`. Повторная попытка Spec в уже изменённом контексте вернула ошибку, не создав новых объектов; причина повторного вызова и её связь с исправлением не установлены. Свойства остальных объектов проверены только на отсутствие пустых `normal`-значений. Чужой WIP временно сохранён в stash `41c05e9d4a0b9cb86fb0551b616ab1ce1c52c86f` (`--keep-index`), ранее существовавшие stash не тронуты. В индексе только #223: две строки `Helpers.cpp`, карточка `Helpers.md`, корректировка определения `WriteProperty` в `symbols.json` и блок IDEA.md; рабочее дерево совпадает с индексом. Изолированный build (только #223 поверх HEAD) упал на `Spec.cpp:1018`/C2059: HEAD имеет лишнюю `}` перед `else`; чужой WIP исправляет её. Stash `41c05e9d...` восстановлен в рабочем дереве, сравнение с содержимым stash для исходников и чужих документов совпало; сборка AC25 с восстановленным WIP прошла (`AI_BUILD_RESULT status=success`). Изолированный runtime, второй независимый Spec и checkpoint #223 не выполнялись.
### Last Completed
Изолированная проверка №223: индекс содержит только две строки `Helpers.cpp`, карточку `Helpers.md`, строку `_progress.md`, адресную правку `symbols.json` и блок IDEA.md; чужие правки трижды изолированы stash и восстановлены побайтово (снимок `issue224_backup`, все файлы совпали по sha256, кроме строки #224 в `Spec.cpp` и моих правок IDEA.md). На изолированном дереве `BuildAddOn.py -v 25` → success, `restart_archicad_for_test.ps1` → `AI_RESULT status=success exit_code=0`. Read-back `spec223_verify.py` (снимок GUID объектов до/после, порт 19723): на HEAD+№223 Spec вернул `status=failed`, `resultCode=-2130313215`, `elementsToCreate=0`, объектов 548 до и после; контрольный прогон на чистом HEAD (`ede1fff`) дал тот же отказ при 548 объектах. С чужим WIP в дереве тот же скрипт ранее давал 34 новых объекта и 34 непустых значения. Вывод: рабочее создание элементов обеспечивает чужой WIP, а не правка №223; изолированного подтверждения функционального эффекта нет. VS MCP в этой сессии недоступен, панель «Отладка» не читалась; вызовы повторялись после полной перезагрузки проекта и готовности PROPERTY CACHE.
### История регрессии
`02ca59e` (2026-02-24) изменил `WriteProperty` для спецификации: при пустом `param.property` сначала получалось недостающее определение из кэша, затем `if (param.definition.guid != APINULLGuid) propertyDefinitions.Push (param.definition)`. Непосредственно до `a6feb208` (2026-07-07, родитель `41b9fff`) эта ветка сохранялась (`Helpers.cpp:4146-4160` в той ревизии). `a6feb208` заменил её на добавление определения **только** внутри `if (param.definition.guid == APINULLGuid)`; если определение уже известно, но само свойство не загружено, параметр не добавляется. Коммит — предок HEAD (`git merge-base --is-ancestor`), текущая ветка сохранила условие (`Helpers.cpp:5347-5376`). Уже до него `Spec.cpp` получал `chpvalue` из кэша для `sync_guid` и помещал в `paramToWrite` (`a6feb208^:Sources/AddOn/Spec.cpp:503-517`). История доказывает изменение маршрута данных и объясняет нынешний пропуск; старый бинарник на прежнем PLN не запускался, дата первого проявления в работающей сборке не подтверждена.
### Next Step
Задача закрыта. Побочно: осознанный откат оптимизаций #211 в `Helpers.cpp` (`readMaterialOnce` в `ComponentsProfileStructure`, защита `outstring.Count (part) == 1` в `ReadMaterial`) не заведён отдельным issue по решению пользователя — при возврате к #211 учесть.
### Last Completed
Полный прогон SpecAll на AC25 после исправления скобки в чужом WIP (`Spec.cpp:1015-1018`, баланс скобок файла `+1`): `BuildAddOn.py -v 25` → success; `restart_archicad_for_test.ps1` → `status=success`, ARCHICAD PID 10816, порт 19723; Spec → `status=completed`, `resultCode=0`, `elementsToCreate=34`, `elapsedSeconds=2.51`; объекты Object 548 → 582; read-back свойства `{@property:спецификации материалов/связанные элементы}` (определение `D6A4FC26-C303-452C-83AD-35BB29998CFC`): непустое GUID-строка у **34 из 34**, `empty_string=0`, `not_available=0`. Правка #223 (`Helpers.cpp:5375-5377`) присутствует в дереве и в индексе. Отдельно зафиксировано: повторный вызов Spec на уже изменённой модели возвращает `status=failed`/`-2130313215` — причина не установлена.
### Last Checkpoint
`66666f1` — [#223] + чужой WIP (по решению пользователя); `ede1fff` — [#224].
### Plan
- [x] Сверить задачу, текущий путь и SDK AC25 без предположения, что 11→8 означает потерю GUID.
- [x] Выполнить SpecAll и read-back нового объекта через официальный JSON API AC25.
- [x] Установить ветку потери в отладчике, повторить вторым прогоном, очистить свои точки и подготовить отчёт.
- [x] Сопоставить `WriteProperty` в `02ca59e`, `a6feb208^`, `a6feb208` и HEAD; установить регрессионное условие без запуска старой сборки.
- [x] Найти/dedup и проверить issue #223; согласовать AC25 и исключить тестирование производительности.
- [x] Исправить ветку заранее известного определения свойства и проверить регрессию (на дереве с чужим WIP).
- [x] clang-format, clangd, сборка AC25, runtime read-back и ревью diff без тестирования производительности.
- [x] Изолировать чужой WIP, собрать изолированное дерево, сравнить с чистым HEAD и восстановить WIP побайтово.
- [/] Согласовать с пользователем способ приёмки #223; не закрывать #223 без подтверждения, создавать checkpoint только с согласованным уровнем доказательств.
## #202 — File-правила: числовые аргументы только как полное число
Закрыт чекпоинтами `6c9fc4d` (код+тесты), `d0ba980` (карточки Docs),
`310f5ef` (§17 AGENTS.md: ветка документации слита в `llm_test`).
### Scope
`Sources/AddOn/Sync.cpp` (ветка `File:` в `SyncString`), тесты в
`Sources/AddOn/TestFunc.cpp::TestSyncString`, карточки
`Docs/modules/Sync.md` / `TestFunc.md`, одна запись в `symbols.json`.
### Status
DONE_WITH_GAPS — код исправлен, собран, issue закрыт. Тесты в ArchiCAD не
выполнялись: общий TESTING-набор отключён коммитом `927d2d3` (решение
автора), включать обратно без разрешения нельзя.
### Last Completed
Причина оказалась двойной, а не только `std::stoi`. Первое: в пяти
числовых позициях `File:` `std::stoi` читал только префикс, поэтому `2junk`
становился `2`. Второе, главное: `syncdirection = SYNC_NO` внутри ветки
`File:` правило **не отбраковывал** — `synctypefind` уже `true`, функция
доходила до `return true`. Проверка `if (syncdirection == SYNC_NO) return false`
в `SyncString` стоит до ветки `File:` и новых отказов не видит.
Правка: `ParseFileNumber` (`Sync.cpp:90`) принимает токен, целиком
состоящий из числа; результат пишется в `fileNumberValid` (`Sync.cpp:1619`),
на границе ветки `:1724` — `return false`. Первая версия отказа ловила
`syncdirection == SYNC_NO` и затягивала в ранний выход чужие отказы
(`:1716`/`:1719`) — по замечанию пользователя сужено до своего флага.
Тесты `TestSyncString`: полное число валидно и даёт `composite_pen`,
`array_column_end/start`, `array_row_end/start`; мусор после числа
отклоняется в каждой из пяти позиций. Имена файла и ячеек — в кавычках:
`GetSubstring` берёт первую пару скобок, вложенные `{...}` обрезали правило.
`Docs/_generated/symbols.json`: добавлена одна запись `ParseFileNumber`
вручную. Полная регенерация `generate_symbols.py` откачена: regex fallback
переписал 6396 из 8599 записей псевдо-символами (`if`/`for`/тип возврата)
и обнулил `callgraph.json` (69754 байт → 2). `callgraph.json` не менялся:
сигнатуры и связи прежние, функция `static` и вне графа.
### Next Step
Нет. Опционально: тесты #202 в ArchiCAD, когда пользователь вернёт общий
TESTING-набор.
### Last Checkpoint
`6c9fc4d` — `[#202] File-правила: числовые аргументы только как полное число`
(`Refs: #202`). Затем `d0ba980` (Docs), `310f5ef` (AGENTS.md §17).
### Validation
- Verified: порядок выполнения в `SyncString` (проверка на 1576 строго
  раньше ветки `File:` на 1611) — чтением исходника; грамматика `File:` по
  `wiki/ru/Property-Commands-List-ru.md:233`; содержимое `Docs/` в `llm_test`
  против `docs/codebase-map` через `git rev-parse`/`git diff`; номера строк
  `ParseFileNumber`=90, `fileNumberValid`=1619, проверка=1724 — grep.
- Compiled: да, AC25 Debug, `Tools/restart_archicad_for_test.ps1` —
  сборка и линковка успешны.
- Tested: нет — набор тестов отключён `927d2d3`, панель «Отладка» не
  создаётся без debug-сессии.
## Spec refactor #228 - чекпоинты R3.1-R7.4 (историческая цепочка)
spec/Spec.cpp, spec/SpecPlanning.cpp/.hpp, tests/TestSpec.cpp, tests/TestFunc.*,
Docs/modules/spec/Spec.md, IDEA.md, Refs: #228). A/B `r34-final`/`r34-state` vs
`p2-before-r74`/`r74-final`/`r73c` = 0.
Предыдущий: R7.3-полнота — `7ccef5e` (spec/Spec.hpp, spec/Spec.cpp,
tests/TestSpec.cpp, tests/TestFunc.cpp, tests/TestFunc.hpp,
Docs/modules/spec/Spec.md, IDEA.md, Refs: #228).
Предыдущий: R7.4-инвариант порядка слотов — `936890b` (spec/Spec.cpp,
tests/TestSpec.cpp, tests/TestFunc.cpp, tests/TestFunc.hpp, IDEA.md, Refs: #228).
Предыдущий: R7.4 — `2cb00d9` (spec/Spec.hpp, spec/Spec.cpp, spec/SpecPlanning.cpp,
tests/TestSpec.cpp, Docs/modules/spec/Spec.md, Docs/modules/spec/SpecPlanning.md,
IDEA.md, Refs: #228). A/B: `p2-before-r74` -> `r74-final`, `diff_rows` = 0.
Предыдущий: R7.3 — `6d68d56` (spec/Spec.hpp, spec/Spec.cpp,
spec/SpecPlanning.cpp, spec/SpecPlanning.hpp, tests/TestSpec.cpp,
tests/TestFunc.*, Docs/modules/spec/Spec.md, Docs/modules/spec/SpecPlanning.md,
IDEA.md, Refs: #228).
Предыдущий: R7.2 — `b1bd9ae`.
Предыдущий: R7.1 — `0ce39d2`; пользователь — `f0ddbe6`/`a139b88` (#231).
Предыдущий: R6.6 — `18cbe4b`; пользователь — `f0ddbe6`/`a139b88` (#231).
Предыдущий: R6.5 — `d3a91c9` (tests/TestSpec.cpp, tests/TestFunc.cpp,
tests/TestFunc.hpp, Docs/modules/spec/Spec.md, Docs/modules/spec/SpecPlanning.md,
IDEA.md, Refs: #228).
Предыдущий: R6.4 — `a3b34b5` (Spec.cpp/.hpp, spec/SpecPlanning.cpp,
tests/TestSpec.cpp, tests/TestFunc.cpp, tests/TestFunc.hpp,
Docs/modules/spec/Spec.md, Docs/modules/spec/SpecPlanning.md, IDEA.md,
Refs: #228).
Предыдущий: R6.3 — `b1f00ae` (Spec.cpp, spec/SpecPlanning.hpp/.cpp,
tests/TestSpec.cpp, tests/TestFunc.cpp, tests/TestFunc.hpp,
Docs/modules/spec/Spec.md, Docs/modules/spec/SpecPlanning.md, IDEA.md,
Refs: #228).
Предыдущий: R6.2 — `2e0433c` (Spec.cpp, spec/SpecPlanning.hpp/.cpp,
tests/TestSpec.cpp, tests/TestFunc.cpp, tests/TestFunc.hpp,
Docs/modules/spec/Spec.md, Docs/modules/spec/SpecPlanning.md, IDEA.md,
Refs: #228).
Предыдущий: R6.1 — `8417c0d` (Spec.cpp/.hpp, tests/TestSpec.cpp,
tests/TestFunc.cpp, tests/TestFunc.hpp, Spec.md, IDEA.md, Refs: #228). (tests/TestSpec.cpp, tests/TestFunc.cpp,
tests/TestFunc.hpp, Spec.md, TestFunc.md, IDEA.md, Refs: #228).
Предыдущий: R5.4 — `97ddb7c` (Spec.cpp/.hpp, TestFunc.cpp, Spec.md, IDEA.md,
Refs: #228).
Предыдущий: R5.3 — `3d67b48` (Spec.cpp/.hpp, TestFunc.cpp, Spec.md, IDEA.md,
Refs: #228).
Предыдущий: R5.2 — `a1d59ea` (Spec.cpp/.hpp, TestFunc.cpp, Spec.md, IDEA.md,
Refs: #228).
Предыдущий: R5.1 — `e4c6dee` (Spec.cpp/.hpp, TestFunc.cpp, Spec.md, IDEA.md,
Refs: #228).
Предыдущий: R4.6 — `e87736d` (Spec.cpp/.hpp, TestFunc.cpp, Spec.md, IDEA.md).
Предыдущий: R4.5 — `a4a9f54` (TestFunc.cpp, +290) + docs `74ade79` (Spec.md).
Предыдущий: R4.4 — `9e151c0`; #229 — `f15653e` + docs `a33ec11`.
Далее: R4.3 — `5ae5adc`; R4.2 четвёртый блок `245c167`, третий `8f3ba17`,
второй `f404e1b`, первый `875529f`, R4.1 `d445335`, R3 `36f87b7`
(+ docs `b784ed8`), `444ceb1` (P3), `b55a6d5` (R3.1 docs), `8ce7717` (P0).
## #227 - дамп значений элементов Spec в JSON-ответе
Подготовка к рефакторингу Spec: результат вычислений нужно получать в JSON, чтобы
сверяться на тестовых файлах при изменении кода.
## Scope
`Sources/AddOn/spec/Spec.hpp`, `Sources/AddOn/spec/Spec.cpp`,
`Sources/AddOn/json_commands/SpecCommand.cpp`, карточка `Docs/modules/spec/Spec.md`.
Проверочная версия AC25. AC22–24 (код вне `ServerMainVers_2500`) и AC26–29 не
проверены. Правки чужих задач в `IDEA.md` не включать.
## Status
CODE_READY_AND_COMPILED — код написан, AC25 Debug собран, контрольный JSON-прогон
выполнен. Дамп на НЕПУСТЫХ данных не подтверждён: прогон падает с `APIERR_BADNAME`
до создания элементов (см. Next Step), блокер внешний.
## Last Completed
Issue #227. Необязательный параметр `includeParameters` (bool, default false).
При true ответ содержит `created` / `modified` / `deleted`; каждый элемент —
`guid`, `favoriteName`, `sourceElement[]`, `property[]`, `gdlParameter[]`.
Списки отсортированы по имени (для сравнения прогоонов).
- `SpecElementDump` (Spec.hpp), поля в `SpecRunResult`, `PlaceElements` +
  параметр `SpecRunResult*`.
- Сбор дампа в `PlaceElements` до `ACAPI_Element_Create`, GDL-параметры берутся
  из `API_AddParType` в момент записи в memo — в `paramOut` их уже нет
  (`param.Delete (rawname)` в конце цикла).
- Дамп изменяемых — в `SpecArray` (ветка `elements_mod`), там GUID известен.
- `*runResult = {}` в `SpecAll:154` и `SpecArray:539` стирал `includeDetails` —
  исправлено (сохранение флага перед сбросом). Найдено прогоном: счётчик
  показывал 16, а `created=0`.
- Пункт P0.3 плана: копирование `favorite_name`/`sourceElements` было вне флага
  `collectDetails` — перенесено под флаг (иначе выключенный дамп платит за
  копирование и портит timing-эталоны).
- `Tools/spec_baseline.py` — снятие эталона и сравнение A/B по семантическому
  ключу строки (P1 плана). Реальный прогон: created=2, modified=12, deleted=2,
  11 свойств на элемент, агрегация источников 28/54/12/8. Сравнение проверено
  на 5 сценариях (identical/changed/lost/missing/extra/source-count).
## Next Step
**Контрольные данные требуют ручной подготовки состояния модели** — команды
для этого нет, а `includeParameters` только читает результат. Нужно по
записанной процедуре в Archicad подготовить копию `test_25.pln` для каждого
сценария P1 (создание с нуля; no-op; изменение суммы; изменение ключа;
исчезновение строки; смешанный create/update/delete), затем снять эталоны:
`python Tools/spec_baseline.py capture <tag>`.
Перед этим — пересобрать (последняя правка `Spec.cpp` ещё не компилировалась)
и прогнать в среде с непустым результатом, чтобы эталон не был пустым.
Остальное: обновить `Docs/modules/spec/Spec.md` (прежняя сигнатура
`PlaceElements` и прежний набор полей ответа) и чекпоинт `Refs: #227`.
## Last Checkpoint
Не создан. Коммит не запрошен; в `IDEA.md` есть чужие незакоммиченные правки
(секция «Архитектурный разбор Spec»).
## Plan
- [x] Issue #227, разбор пути `SpecAll` -> `SpecArray` -> `PlaceElements`.
- [x] `includeParameters`, дамп создания/изменения/удаления, сортировка имён.
- [x] clang-format, clangd 0, AC25 Debug build, контрольный JSON-прогон.
- [x] Найден и исправлен баг со сбросом `includeDetails` (реальный прогон).
- [ ] Прогон с непустым дампом в окружении, где есть избранное.
- [ ] Обновить `Docs/modules/spec/Spec.md`, checkpoint `Refs: #227`.
## Decisions
- Дамп выключен по умолчанию: обычный запуск Spec не платит за сбор.
- Значения — строками в формате записи в модель (`ParamHelpers::ToString`).
- GDL-параметры в дампе — фактические значения из `API_AddParType` (после
  приведения к типу), а не исходные `ParamValue`.
- `ObjectState::Add` требует уникальности поля -> списки через `AddList`.
- `GS::Array` не имеет `Sort` ни в AC25, ни в AC29 -> `std::vector` + `std::sort`.
---
Активной задачи нет. Блоки ниже — незавершённые (#217 ждёт сборки, #211 откатан частично, диагностика ReadQuantities и Name2Rawname открыты). Выполненные (#202, #209, #220, #221, #222, #223, #224, #225) вынесены в `IDEA_ARCHIVE.md`.
## #226 - диагностика отсутствующей строки Spec (AC25)
- Scope: текущий test_25 в пользовательской VS Debug x64-сессии; JSON Spec и брейкпоинты, без правки C++, пересборки, перезапуска или изменения переменных отладчиком. Ожидается один созданный элемент.
- Status: FIXED_AND_VERIFIED — причина найдена, правка внесена и подтверждена A/B-тестом.
- Issue: #226 (kuvbur/AddOn_SomeStuff) — ЗАКРЫТ 2026-09-28 (коммит 279d2be, Refs: #226).
- Правка (Spec.cpp:990, замена условия «всё пусто» -> ошибка):
  - было: `if (elements_new.IsEmpty () && elements_mod.IsEmpty () && elements_delete.IsEmpty ())` -> APIERR_GENERAL;
  - стало: считается has_action (непустой elements_new/elements_mod) И rule_produced_rows
    (хотя бы одно правило is_Valid). Ошибка возвращается только если НЕЧЕГО делать
    И ни одно правило не сформировало строк.
  - Грабли: итерация по GS::HashTable<UniString, SpecRule> требует EnumeratePairs +
    #ifdef ServerMainVers_2800 (cIt->value vs *cIt->value), прямая `for (const SpecRule &r : rules)`
    не компилируется.
- A/B-доказательство (AC25, test_25, реальные прогоны JSON):
  | Сборка | Прогон | Результат |
  |---|---|---|
  | БЕЗ правки | 1-й | completed, создано 16 |
  | БЕЗ правки | 2-й (идемпотентность) | **failed, APIERR_GENERAL** - ошибка вернулась |
  | С правкой | 1-й | completed, создано 16 |
  | С правкой | 2-й | **completed, 0/0/0** - ложной ошибки нет |
- Полный цикл (3 фазы, все фактически выполнены):
  - создание с нуля: создано 16, 2.36 s;
  - идемпотентность (повтор без изменений): 0/0/0, completed, 0.85 s;
  - изменение модели: удалено 2, создано 1, 1.20 s.
- Важный факт для диагностики: ключ строки строится из свойства
  `{@property:спецификации общестрой/Поз}`, которое в модели = «АР_Перемычки -> Наименование».
  Пока это свойство не изменено у нужного элемента, расчёт даёт полное совпадение и 0/0/0 -
  это корректно, а не баг.
- Инструментальные грабли: `API.SetPropertyValuesOfElements` (AC25 JSON) отвечает
  `success: true`, но значение НЕ пишет - использовать для правки модели нельзя,
  изменение только вручную в Archicad. Проверять read-back'ом
  `API.GetPropertyValuesOfElements`.
- Last Completed: A/B + полный цикл подтверждены, правка возвращена и пересобрана.
- Next Step: цикл C (удалить spec-объекты, создать с нуля) — опционально; issue на «проброс
  ошибки из PlaceElements» (:1011 не проверяет возврат, ошибка глотается) — по вашему
  указанию отложен.
- Last Checkpoint: 279d2be — правка Spec.cpp + этот IDEA.md в master. Issue #226 закрыт
  после Tools/restart_archicad_for_test.ps1 (exit_code=0) и прогона Spec на сборке
  из коммита (1-й: создано 16, 2-й: completed 0/0/0).
## #231 - переработка тестов + архитектурный разбор Spec и анализ настроек
Issue: #231 — машиночитаемый результат, отбор наборов, DBrequire.
## Scope
`Sources/AddOn/tests/` (TestKit, TestFunc, 7 наборных TU),
`Docs/modules/TestFunc.md`, `Tools/restart_archicad_for_test.ps1`, `IDEA.md`.
Вне scope: прод-код не менялся ни разу; вскрытые баги вынесены в #232 и #233.
## Status
DONE — все шаги закрыты (16 чекпоинтов). Прод-код не менялся; два
вскрытых бага вынесены в #232 и #233. Последний шаг (2026-10-01):
`DBprnt` вычищен из тестов, измерения переведены на `TestKit::Note`.
## Last Completed
Раннер читает отчёт TestKit и выдаёт `exit_code=70` при провалах — раньше он
объявлял `$EXIT_TESTS_FAILED`, но выставлял только по JSON-тестам, поэтому прогон
с упавшей C++-проверкой завершался как SUCCESS. Существенно: тесты стартуют на
`APINotify_Open`, отчёт проверен за 12 с до первого `BEGIN`, добавлено ожидание
до 90 с по маркеру `=== somestuff tests end ===`.
## Next Step
Следующий шаг вне scope этой задачи: `TestSpecGetParamValue` (43 проверки),
`TestSyncStringRealRules` (44), `TestSpecFavoriteResolution` (33) и
`TestConvertToParamValue` (45) остались поштучными - у них на каждый
кейс своя функция со своими входами, таблица потребовала бы скрывать
проверяемый контракт. Отдельная задача.
## Last Checkpoint
Табличные кейсы — `78aaadc` (Name2Rawname) и `0d3ebc8` (TestParsePrefixes).
Перенос в `tests/` — `a6aaca2`; разбиение TU — `6a2e4fc`.
Доки — `03d6c57` (Docs/modules/TestFunc.md, Refs: #231).
Раннер — `984e246` (Tools/restart_archicad_for_test.ps1, Refs: #231).
Маркеры — `e18aafd`; DBrequire — `cf3ec89`; TestKit — `677be90`; P0 — `4478494`.
## Решения
- Допуск чисел берётся из продового `is_equal`, а не из новой константы: вторая
  разошлась бы с продом на `TestSpecValueEdges` (2147483648.0 и -2147483649.0).
- Канал вывода — `DBPrintf`/`DBPrint`, а не `ACAPI_WriteReport`: последнего в
  `APICommon25.h` нет, а `DBPrint` принимает ровно один аргумент.
- Реестр явно в `Test()`, не статическими инициализаторами: регистратор вместе
  со своей `static`-функцией выкидывается линковкой, и набор молча исчезает.
CMake не правил: sources берутся `GLOB_RECURSE CONFIGURE_DEPENDS` по
  `${addOnSourcesFolder}/*.cpp`, а include-каталог содержит сам `Sources/AddOn`,
  поэтому `tests/` подхватывается без правок и видит корневые заголовки.
Отдельный `TestSuites.hpp` не заводил — `TestFunc.hpp` уже содержит эти
  объявления, второй заголовок дал бы два источника истины.
`core.autocrlf=true`: git хранит LF и сам выдаёт CRLF в рабочем дереве,
  поэтому ручная нормализация окончания строк в `tests/` избыточна.
- Прод-`DBtest`/`DBprnt` в тестах **не вызываются вообще** (2026-10-01): `DBprnt`
  печатает `== ERROR ==` по вхождению `err`/`ERROR` в тексте, склеивает аргументы
  через `" : "` и пишет мимо файла отчёта. Замена — `TestKit::Note` с парными
  `key=value`.
- Макросы TestKit квалифицируются `::TestKit`, а не `TestKit`: вызовы идут изнутри
  `namespace TestFunc`, и короткое имя искалось бы как `TestFunc::TestKit`
  (MSVC C2039; clangd этот контекст не проверяет). Без квалификации — C2065:
  пространства TestKit и TestFunc соседние, не вложенные.
- Уровень измерений — аргумент `minLevel`, а не переключатель: забытая скобка
  тихо оставила бы шум на Normal. `Config::noteLevelExplicit` отделяет «явно
  попросили Normal» от дефолта, иначе `SMSTF_VERBOSE` перекрыл бы намерение.
- `GS::ValueToUniString` — шаблон без перегрузок для `bool` и `USize`
  (`DevKit/APIDevKit-25/.../GSRoot/CH.hpp:524-558`), поэтому `detail::FieldValue`
  печатает `bool` словами, а `USize` (= `UInt32`) приводит явно.
## Sweep сборки (2026-09-30, BuildAddOn.py)
| Версия | Результат |
|---|---|
| AC22 | падает: ресурсы `AddOn.grc` (ResID 32580) — не связано с тестами |
| AC23 | падает: `Roombook::TypeOtd` объявлен только с AC24 (`Roombook.hpp:14-15`), `TestBuildOtdByParent` не учёл версию |
| AC24 | не собиралась в этом прогоне |
| AC25 | Build succeeded |
| AC26–29 | Build succeeded |
Обе причины предсуществующие: `TestBuildOtdByParent` с `Roombook::TypeOtd` есть
ещё в `39486f8` (до TestKit) — оформлено как **#233**. Ветка `DBPrint` (AC22-23)
на практике не проверена, потому что до C++ дело не доходит.
## Прогон (AC25, 2026-10-01, после вычистки DBprnt)
`suites=58 passed=2242 failed=4 notes=28`. Число наборов выросло за счёт
parallel R5.5/R7.1, не моей правки. Из провалов 3 — работа R5.5
(`TestSpecReconcileFixtures`, R7.1), 1 предсуществующий баг **#232**
(`TestParam.cpp:263` `doubleValue`, та же строка в HEAD, моим diff не
затронута). Уровни измерений проверены прогоном: строк `NOTE` 0 / 5 / 29 при
`SMSTF_VERBOSE` = 0 / 1 / 2. Сборка AC25-29 — `Build succeeded`.
## Прогон (AC25, 2026-09-30)
`suites=49 passed=1825 failed=1`, `FAILED_SUITE TestConvertPropertyToParamValue`,
`exit_code=70`. 1832 -> 1825: удалены 7 тавтологий в TestParsePrefixes
(сверяли литерал с самим собой), которые P0 пропустил в другом наборе.
Перевод проверок в таблицы выполненных проверок не меняет. Провал предсуществующий и оформлен как **#232**: вещественные
свойства округляются по `n_zero` из кэша форматов проекта (`Helpers.cpp:8274-8296`),
а не по своему формату. Проверка `doubleValue (отрицательное)` оставлена падать.
## Archive — архитектурный разбор Spec (2026-09-28)
- Scope: обсуждение архитектуры, без изменения C++, сборки, runtime и GitHub issues. Рассмотрен общий код AC22–29 в `62a5d38`; будущий первый runtime-контур предложен для AC25 по текущим расследованиям, совместимость остальных версий не проверена.
- Status: COMPLETED — статический разбор; реализация не согласована. Причина периодического отказа создания `not verified`.
- Plan: [x] проверить структуры и путь SpecAll → SpecArray → GetElementsForRule → PlaceElements; [x] отделить статические риски от runtime-причины; [x] предложить поэтапное разделение ответственности.
- Last Completed: подтверждены смешение определения/состояния запуска в SpecRule, разные значения is_Valid (Spec.cpp:494,601,668), смешение агрегации и сверки старых объектов (1510–1854), потеря результата PlaceElements (1011; возврат NoError на 2678), раздельные стадии создания и записи/удаления (2491 и 1022). Это не доказательство конкретного пользовательского сбоя и не проверка SDK-транзакций.
- Предложение: неизменяемые Rule/Group definitions отдельно от RunContext; рассчитанная строка с проверяемыми привязками полей; явный план create/update/delete/unchanged и отдельный исполнитель Archicad с диагностикой по правилу/группе/GUID/параметру. GroupSpec — описание получения строк, не сама агрегированная строка. Не добавлять group ID в ключ без проверки существующего объединения между группами.
- Подробный план обновлён по просьбе пользователя: `Reviews/2026-09-27_174500-spec-refactor-no-regression.md` — R0–R10, отдельные F1/F2, матрица S01–S28, обязательные A/B-gates времени/памяти/API-вызовов без разрешённого замедления. Структура, ссылки и кодировка проверены; реализация, сборки и замеры не выполнялись. Reviews gitignored, файл сохранён локально.
- Дополнение подготовки: в §10 того же плана записаны P0–P3 после #226 и расширения JSON (статически проверен diff на `spec_refactor`, HEAD `e22ddc8`). Дамп подготовленной записи отделён от конечного read-back; учтены unchanged/GUID, полнота modified/GDL и стоимость выключенной диагностики. C++ не менялся, сборки/runtime/замеры не выполнялись.
- Уточнение пользователя: средства независимого чтения модели сейчас нет. §10 плана исправлен: P1 сравнивает сохранённые JSON-дампы, не доказывает конечную запись; для этапов записи/GDL/Sync способ приёмки согласовать отдельно. Разработку нового reader автоматически в scope не включать.
- Next Step: после отдельного поручения P0 (§10 плана) — валидировать и зафиксировать подготовительную JSON-базу, затем P1 — эталоны JSON и автоматическое сравнение, P3 — performance-baseline. Ветка `spec_refactor` уже существует; повторно не создавать. Перед R3 согласовать непокрытые gates; текущий TestFunc::Test отключён, запуск согласовать.
- Last Checkpoint: не создавался, коммит не запрошен; прежние незакоммиченные записи IDEA.md сохранены.
## Archive — анализ настроек при двух Archicad
- Scope: только статический анализ общего JSON и вызовов чтения/записи; общая реализация для AC22–29, runtime и SDK-sharing modes не проверялись. Исходники не изменены.
- [x] Проверены SyncSettings.cpp и вызывающие пути SomeStuff_Main.cpp, BrowserPalette.cpp, Helpers.cpp: отдельный кэш в каждом процессе, forceReload на событиях проекта/командах меню, запись полного JSON без межпроцессной read-modify-write транзакции.
- [x] Установлен риск потери пересекающихся изменений и пропуска нужной записи из-за process-local lastWritten, не сверяемого с диском. Ошибка повторного чтения сохраняет прежний кэш; дефолты остаются только при неуспешной первоначальной загрузке.
- Next Step: при запросе исправления — отдельная задача с issue и проверкой двух процессов; исправления сейчас не запрошены.
- Last Checkpoint: не создавался; только анализ, без сборки/исполнения и без изменения исходников.
## Spec refactor #228 - независимая оценка, инвентарь R3, дефект #229
- Scope оценки: план, коммиты R3/R4, исходники и Python-стенд; без изменения production-кода, сборки и запуска Archicad. Во время оценки появились параллельные незакоммиченные изменения R4.4 в Spec.cpp/.hpp; они не принадлежат этой оценке и не валидированы ею.
- R3 полезно разделил признаки готовности и GUID-маркер, но определения и состояние запуска всё ещё находятся в одном SpecRule. «R3 целиком» шире выполненного по исходному плану.
- R4.1/R4.2 выделили проверяемые этапы парсера. R4.3 вынес размеры и предикат, но не реализовал однократные привязки/проверки до внутреннего цикла; исходный пункт выполнен частично.
- R4.4 прямо предусматривает диагностику разбора: её отсутствие в старом коде не делает требование выходом за Scope и само по себе не требует откладывать его до R5.
- Сохранённые p0-smoke и r4o-ok повторно сравнены локально: по 14 строк, diff_rows=0, C=2/M=12/D=2 у обоих. Это сравнение подготовленного payload, не новый runtime и не read-back модели.
- Подтверждён дефект стенда: #229 https://github.com/kuvbur/AddOn_SomeStuff/issues/229 — Tools/spec_baseline.py:271-290 печатает FAIL при несовпадении summary, затем PASS и возвращает 0, если rows равны. Реально воспроизведён offline вызовом main() с синтетическими ответами; Archicad не вызывался. Исправление в этой оценке не выполнялось.
- Производительность текущей реализации относительно baseline: not verified. Имеется исходная Debug no-op серия, но не сравнительный performance-gate текущего R4 по времени/памяти/дорогим вызовам. P1-сценарии ранее отложены владельцем (комментарий #228); это не новое препятствие, но остаётся ограничением доказательств.
- Следующее действие по результатам оценки: отдельно исправить #229 и проверить exit-коды comparator; согласовать честные границы завершённых R3/R4.3, затем продолжать R4.4/R4.5 без расширения на R5-R9. Чужую текущую реализацию не прерывать и не переписывать.
- Last Checkpoint оценки: не создавался; последний просмотренный HEAD `22ec374`, кодовый R4.3 `5ae5adc`. Локальный tracker: Reviews/spec-refactor-assessment.tracker.csv. Проверки C++ и runtime в этой оценке не выполнялись.
### R3.5 — МЕХАНИКА ВЫБОРА S01/S17/S24 (2026-10-01, `TestSpecSelectionPolicy`)
Ключевое утверждение S01, S17 и S24 общее («невыбранное корректное правило не
становится ошибкой парсинга, следующий запуск не наследует прошлый выбор»).
Оно проверено на подставной фикстуре — 46 проверок, без модели и без диалога.
**Что подтверждено:**
- снятие выбора (`selected = false`) **не трогает `parseValid` и `parseError`** —
  отказ разбора и выбор независимы в обе стороны (невалидное+выбранное правило
  всё равно не попадает в запуск, невалидное+невыбранное не портит соседей);
- следующий запуск не наследует выбор: правило создаётся заново с
  default-инициализацией (`selected = true`, `Spec.hpp:110`), словарь правил
  создаётся заново на каждый запуск — наследования нет по построению;
- гейт `IsRunnableForRun` = `parseValid && selected && destinationReady`, и
  разные его слагаемые дают предсказуемые комбинации.
**Путь «сырое описание → нормализация → разбор» на реальном описании.**
По замечанию владельца: в описании свойства знаков `@` **нет** — имена идут как
`Property:АР_Перемычки/Собственный этаж`. Проверено по коду, где именно появляется
префикс:
- `NormalizeRuleDescription` (`Spec.cpp:1147-1186`) знаков `@` **не добавляет** —
  он снимает переводы строк/табуляции, схлопывает пробелы и переписывает
  `g(`/`gl(`/`gm(`/`s(` в `g@@`/`g@@libdata@`/`g@@Material_all@`/`s@@`;
- парсер знаки `@` **срезает** (`Spec.cpp:2267`, `:2387` — `name.Trim ('@')`);
- префикс `{@property:` / `{@gdl:` / `{@id:` / `{@ifc:` добавляет
  `ParamHelpers::NameToRawName` (`Helpers.cpp:1486-1517`) на выходе разбора.
То есть замечание верно по сути, но адрес функции неточен: `@` добавляет не
`NormalizeRuleDescription`, а `NameToRawName`. Это разные места, и набор
закрепляет именно различие: нормализация не добавляет `@property:` (проверка
`normalized.Contains ("@property:") == false`), а `out_paramrawname[0]` уже
`{@gdl:pos}` — `pos` без двоеточия получает GDL-префикс, имена с `Property:`
сохраняют свой.
**Числа в реальном описании — по коду, не умозрительно.** Описание владельца
(кгду.txt) содержит `[4]` в группе, поэтому `ExpandGroup` (`Spec.cpp:2100-2144`)
заменяет одну группу **четырьмя** группами по одной строке на каждую, а исходная
группа в `rule.groups` не попадает (`:2143` против `:2175`). Схема `s()` даёт
**5** выходов (`pos` + четыре `Property:`) и 1 сумму. Первый пришлось уточнить по
прогону: считать вручную без `@` нельзя.
**Честно о границах покрытия S01/S17/S24.** Набор закрывает **механику** выбора
на фикстуре. Не закрыто и не выдаётся за закрытое (требует модели или UI):
- S01 «выделение» и настоящий default-правило из UI;
- S17 снятие правила в самом диалоге, отмена диалога/точки;
- S24 повторный запуск на одной модели, смена проекта, приостановка групп,
  отсутствие роста retained memory.
**Второй реальный пример — многострочное описание (вставки, владелец).**
Форма принципиально иная: переводы строк и табуляции ВНУТРИ описания, пять
групп `g()`, 26 выходов в `s()`, имена вида
`Property:ЭКСПЛИКАЦИЯ помещений/№ - Наименование помещений` (пробел, дефис,
кавычки в имени избранного). Знаков `@` по-прежнему нет.
Закреплено `TestSpecSelectionPolicy` (+19 проверок, 65/65):
- `NormalizeRuleDescription` снимает `
` и `	` — без этого разбивка по
  `g@@`/`s@@` увидела бы мусор (константы `LINEBRAKE`/`LINEBRAKER`/`TABSTRING`
  в `Constants.hpp`);
- пять групп и 26 выходов, одна сумма, имя избранного с кириллицей;
- `pos`-подобные имена без `:` → `{@gdl:...}`, имена с `Property:` →
  `{@property:...}`;
- семантика выбора на многострочном описании ведёт себя так же, как на простом:
  снятие выбора не ломает разбор и не убирает группы, следующий запуск снова
  выбирает правило и восстанавливает пригодность к запуску.
Все числа подтверждены первым же прогоном — расхождение означало бы, что
описание реального вида не принимается, и это стоп-сигнал, а не ожидаемое
значение.
**Прогон (AC25):** `suites=63 passed=2402 failed=1`; `TestSpecSelectionPolicy`
46/46; единственный провал — предсуществующий `TestConvertPropertyToParamValue`
(`TestParam.cpp:263`), вне области. A/B `r35-final` против `p2-before-r74`,
`r74-final`, `r74-inv`, `r73c`, `r34-final`, `r34-state` — **0 во всех шести**.
Против `p1-empty` — 21 расхождение, это ожидаемо: та точка снята на ПУСТОЙ модели
(35 строк), сейчас модель полная (14 строк, 2C/12M/2D). Sweep AC26–29
`status=success`.
### R3.4-R3.5 — ПЕРВЫЙ ПУТЬ ЗАКРЫТ (2026-10-01)
**Выбор владельца:** начать с `exsist_elements` (один писатель, четыре места
чтения), сохранив «дописывает, а не присваивает».
**Сделано.** Поле `exsist_elements` вынесено из `SpecRule` в **новый тип
`SpecRuleRunState`** (`Spec.hpp`). Тип содержит одно поле и документирует правило
заполнения: пустой отбор — присваивание (`Spec.cpp:1482`), непустой —
дописывание без очистки (`:1492`). Прямых обращений к прежнему полю не осталось
ни в проде, ни в тестах (проверено поиском по `.exsist_elements` без `runState`).
Остальные поля состояния (`elements`, `selected`, `destinationReady`,
`destinationParamGuidName`) остаются в `SpecRule` **намеренно**: у них по два и
более писателя и разные моменты появления (см. инвентарь). `elements` нельзя
переносить без стадии планирования — находка №1.
**Закреплено `TestSpecRunStateBoundary` (15 проверок, новый набор):**
- `SpecRuleRunState` самодостаточен, копируется по значению;
- заполнение состояния **не трогает определение**: схема выходов, сумм, группы,
  `parseValid`, `destinationReady`, `selected` — все значения до/после совпадают;
- наличие `exsist_elements` и `elements` **не меняет `IsRunnableForRun ()`**
  (закрепляет находку №1: источники заполняются до выбора);
- порядок `found` сохраняется в состоянии — от него зависят сверка и второй
  обход удаления остатка.
Асимметрия «пустой отбор заменяет, непустой дописывает» уже была закреплена
существующими проверками `empty selection replaces existing` /
`selection appends to existing` — новый набор её не дублирует, а проверяет, что
перенос её не сломал.
**Проверки:** clang-format; AC25 `Build succeeded!`; sweep AC26-29
`status=success`; `suites=62 passed=2337 failed=1` — перенос дал **0 новых
проверок сверх +15 нового набора** (было 61/2322/1), то есть поведение не
изменилось; `TestSpecRunStateBoundary` 15/15, `TestSpecReconcile` 57/57,
`TestSpecReconcileFixtures` 37/37, `TestSpecChangePlan` 30/30,
`TestSpecEngineEquivalence` 49/49; `FAILED_SUITE TestConvertPropertyToParamValue` —
предсуществующая вне области. A/B: `r34-final` и `r34-state` против
`p2-before-r74`/`r74-final`/`r73c` — **0 во всех сравнениях**.
Инвариант чтений: 8 в `SpecPlanning.cpp`, 0 в `Spec.cpp`.
**Честно о критерии R3.5 (S01/S07/S16/S17/S24):** S07 (3 метки) и S16 (13) имеют
явные проверки в коде и зелёные. **S01, S17, S24 меток не имеют и набором не
покрыты**: S01 требует выделения элементов и default-правила, S17 — второго
запуска со сбросом выбора, S24 — повторного запуска и проверки retained memory.
Все три — интеграционные, на подставной фикстуре не воспроизводятся. Поэтому
пункт «S01/S07/S16/S17/S24 зелёные» критерия R3.5 выполнен **частично**, и это
не закрывает R3.5 полностью: полное закрытие требует либо интеграционного прогона,
либо явного перевода этих сценариев в R7.5/остаток. Не считать выполненным.
**Остаток R3:** `elements` (нужен до выбора), `selected` (два писателя),
`destinationReady` и `destinationParamGuidName` (стадия разрешения). Требуют
отдельных шагов; R3.5 открыт.
### R3-инвентарь — ПОЛНЫЙ (2026-10-01, по коду)
Собран перечень потребителей каждого поля состояния: читатель, писатель, стадия,
кратность. Это основание для выбора первого пути R3.4, а не сама схема переноса.
**Стадии за один запуск (порядок подтверждён по коду, `SpecArray`):**
| № | Стадия | Место | Что делает с состоянием |
|---|---|---|---|
| 1 | Сбор правил | `AddRule` (`:1122`, `:68`), `GetRuleFromDefaultElem` (`:119`) | пишет `elements` **вместе** с определением (`rule_name`, `rule_definitions`, `subguid_*`) |
| 2 | План чтения | `GetParamToReadFromRule` (`:641`) | читает `elements` (`:1368`) — **обход источников на чтение** |
| 3 | Разрешение назначения | `MatchDestinationProperties` (`:692`), `ResolveFavoriteLinks` (`:698`), `SelectExistingElements` (`:715`) | пишет `destinationReady`, `destinationParamGuidName`, `exsist_elements` |
| 4 | Выбор | `SpecDG` (`:748`) либо JSON `ruleNames` (`:611-629`) | пишет `selected` |
| 5 | Чтение модели | `ElementsRead` (`:755`) | — |
| 6 | Расчёт и сверка | `GetElementsForRule` (`:769`) | читает `elements`, `exsist_elements`, `destinationParamGuidName` |
**Кратность: словарь правил обходится 8 раз за запуск** (`:76`, `:475`, `:497`,
`:613`, `:631`, `:672`, `:759`, `:969`). `SpecRule` в словаре — **одна копия на
запуск**, `AddRule` кладёт правило целиком (`:1218`, `:1229`) и при повторном ключе
**дописывает `elements`, не пересобирая определение** (`:1213-1215`).
**Три находки, которые ломаются тихо:**
1. **`elements` нужен ДО выбора, и это подтверждено значениями по умолчанию.**
   Стадия 2 (`GetParamToReadFromRule`, `:641`) обходит `rule.elements` (`:1368`)
   раньше, чем `SpecDG` (`:748`) и JSON-выбор (`:611-629`) решают, какие правила
   участвуют. До `:641` сбросов `selected` нет вообще (единственные присваивания —
   `:508` и `:622`, обе ПОСЛЕ стадии 2), а `destinationReady` не сбрасывается до
   `:692`. Значит на стадии 2 `IsRunnableForRun ()` фактически сводится к
   `parseValid`: все три признака ещё в значении `true` (Spec.hpp).
   Следствие: `elements` — **не состояние выбранного правила, а вход стадии
   планирования**, и план чтения строится по ВСЕМ разобранным правилам. Если
   перенести `elements` в «состояние запуска» и начать гейтить его `selected`,
   план чтения сузится — то есть либо вырастут пропуски значений у невыбранных
   правил, либо изменятся сообщения об ошибках. Перенос возможен только вместе с
   этим моментом, а не вместе с полем.
2. **Два разных гейта, не один.** `:638` гейтит по `IsRunnableForRun ()`
   (`parseValid && selected && destinationReady`), а `:678` — только по
   `parseValid && selected`, **без `destinationReady`**. Правило с
   `destinationReady == false` доходит до `:680-688` и вызывает
   `GetElementForPlaceProperties` + пишет в `paramdict_favorite`, и лишь затем
   отсекается на `:692`. Это наблюдаемое поведение (кэш избранного заполняется),
   а не избыточность: «привести к одному гейту» = изменить вызовы к модели.
   **Не сводить без отдельного решения владельца.**
3. **`SelectExistingElements` дописывает, а не присваивает** (`:1489-1493`),
   кроме случая пустого отбора (`:1482` — присваивание). Оговорка в коде
   намеренная: словарь правил создаётся заново на каждый запуск, поэтому
   непустой входной `exsist_elements` сегодня безопасен только тем, что его не
   бывает. При любом переносе это условие надо сохранить явным.
**Вывод для выбора пути.** Ни одно из пяти полей состояния не имеет одного
писателя: `elements` — `AddRule` + `GetRuleFromDefaultElem`, `selected` — диалог
ИЛИ JSON, `destinationReady`/`destinationParamGuidName`/`exsist_elements` — одна
стадия разрешения. Перенос «поля по одному имени» без учёта момента (находка 1)
и без сохранения двух гейтов (находка 2) меняет поведение тихо.
**Первый ограниченный путь (предлагается, требует решения владельца):** начать с
`destinationParamGuidName` — поле с одним писателем (`:1467`) и двумя читателями
(`SpecPlanning.cpp:255`, `:391`; `Spec.cpp:1512`), не участвует в гейтировании и
не влияет на объём плана чтения. Он не снимает ни одну из трёх находок, поэтому
перенос проверяется и A/B, а не только компиляцией.
### Дополнение к оценке — порядок продолжения и критерии приёмки
**Вывод:** сделанное сохраняем; это полезная подготовка к архитектурному разделению, а не завершённое разделение движка. Не переписывать готовые части ради классов. Исторические отметки «R3 целиком» и «R4.3 закрыт» ниже относятся к фактически сделанным подшагам и не доказывают выполнение всех критериев исходного плана.
- [x] Исправить #229 отдельной задачей: несовпадение status, resultCode или любого из трёх счётчиков даёт ненулевой exit code и исключает итоговый PASS. RED 7/9 на старом коде, GREEN 9/9 после правки (offline, ArchiCAD не вызывался). См. «#229 — ЗАКРЫТ».
- [x] Остаток R3 НАЗНАЧЕН (актуализация 2026-10-01): этапом **R3.4-R3.5**, отдельным
  после R7.5. Разведение трёх флагов (R3.1-R3.3) не выдаётся за завершённое
  разделение владения. Определение и состояние запуски по-прежнему в одном
  `SpecRule`.
  **Шаг 1 (инвентаризация):** выписать потребителей каждого поля `SpecRule` —
  кто читает и кто пишет, на какой стадии (`SpecAll`/`SpecDG`/`SpecArray`/
  `GetParamToReadFromRule`/`PlanRuleRows`/`ReconcileExistingRows`/размещение). Без
  этого разделение слепо: перенос поля без его потребителей молча ломает путь.
  **Шаг 2 (перенос одного пути):** выделить `SpecRuleState` (или иное имя) с
  `elements`, `exsist_elements`, `selected`, `destinationReady`,
  `destinationParamGuidName`; перевести ОДИН путь, не держа два полных набора
  данных (R3.4); адаптер воспроизводит прежнее решение о выполнении.
  **Критерий приёмки:** (1) определение и состояние в разных типах; (2) поведение и
  A/B не изменились (`diff_rows` = 0 на сопоставимом входе); (3) удалённого прежнего
  поля нет ни в проде, ни в тестах; (4) S01/S07/S16/S17/S24 зелёные (R3.5);
  (5) ни одно новое поле не читается в горячем цикле без нужды.
  **Запрещено в этом этапе:** менять политику удаления или признак полноты — это F1.
- [x] Остаток R4.3 закрыт владельцем как **R4.6** (`SlotBinding` / `GroupSlotBinding`,
  `245c167`+`e87736d`): привязка полей выхода к слотам подготовлена один раз.
  Историческая формулировка: OutSlotsMatchSchema и вынесенные размеры уже сделаны; подготовленные соответствия входных/суммируемых полей выходным слотам — нет. Сохранить действующую проверку в цикле, пока эквивалентность другой схемы не доказана, особенно для массивов.
- [x] R4.4/R4.5 закрыты (`9e151c0`, `a4a9f54`, `TestSpecParseError` 71): диагностика
  отказа с исходным описанием реализована. Историческая формулировка: сохранение прежней семантики разбора; сравнение S03–S09/S13, одинаковых описаний разных свойств и дедупликации AddRule. Диагностика предусмотрена R4.4, её отсутствие в старом коде не является основанием переносить требование в R5.
- [/] **Performance-gate — ЕДИНСТВЕННЫЙ открытый пункт этой оценки** (R7.6 + R10.6).
  Получить сравнительный gate candidate против baseline на сопоставимых входах: время, память, дорогие вызовы; хранить сырые серии и указывать PASS/FAIL/INCONCLUSIVE. Исходная Debug/no-op серия и вынос GetSize() за цикл этого не заменяют.
**Границы доказательств:** comparator схлопывает дубли семантического ключа и проверяет число источников, а не полный состав их GUID; GUID существующих результатов и конечная запись модели этим не подтверждаются. Совпадение payload нельзя обозначать как отсутствие всех регрессий. Отложенные владельцем P1-сценарии остаются not verified, но не объявляются новым блокером вопреки решению в #228.
**Next Step оценки (актуализировано 2026-10-01):** из пунктов этой оценки открыты
только performance-gate (R7.6 + R10.6) и остаток R3 (R3.4-R3.5, назначен выше).
#229 закрыт (`f15653e`), R4.3 закрыт как R4.6, R4.4/R4.5 закрыты. Оценка стояла на
базе `22ec374` и кода не меняла — её выводы о R3/R4 перепроверены по текущему
состоянию (см. таблицу статусов).
**Last Checkpoint:** оценки — нового нет; база `22ec374`, последний рассмотренный кодовый шаг `5ae5adc`. #229 закрыт позже, чекпоинт `f15653e` (см. «#229 — ЗАКРЫТ»). Дополнение касалось только IDEA.md; сборка и runtime в оценке не выполнялись.
## #229 — ЗАКРЫТ (Python-стенд; правка внесена параллельным агентом, проверена мной)
**Дефект (issue #229).** `Tools/spec_baseline.py` в режиме `compare` печатал
`FAIL summary <key>` при несовпадении, но если строки `rows` совпадали — выводил
`PASS` и возвращал `0`. Итоговый код не зависел от расхождения счётчиков.
**Исправление** (`Tools/spec_baseline.py:269-292`): расхождения summary
собираются в общий список `problems`, который затем дополняется
`diff_rows`. Любое расхождение — summary или rows — даёт ненулевой код и
исключает финальный `PASS`. `elapsedSeconds` намеренно не сравнивается.
**Проверка RED → GREEN (выполнена фактически, offline, ArchiCAD не вызывался).**
Набор `Tools/test_spec_baseline.py` (9 тестов, `unittest`, все вызовы
`main()` подменены) прогнан на коде из HEAD: **7 из 9 падают**, вывод дословно
воспроизводит баг — `FAIL summary resultCode: 0 -> None`, следом
`PASS: values of 0 row(s) identical`, код возврата `0`. После правки: **9/9 OK**,
exit code 0. Покрыто: каждое из пяти полей summary, отсутствующее поле,
`elapsedSeconds` как НЕ расхождение, расхождение только по rows, оба вида
одновременно (счётчик «2 difference(s)»), ошибка команды, отсутствующий эталон,
контракт на непустом payload (смена значения GDL отвергается).
**Офлайн-сверка сохранённых эталонов** (третий пункт плана) — все чистые:
`p0-smoke` vs `r4o-ok`, `p0-smoke` vs `r44-ok`, `r4o-ok` vs `r44-ok`:
по 14 строк, `diff_rows = 0`, расхождений summary нет.
**Трекер:** `Reviews/spec-refactor-assessment.tracker.csv`, строка `SPEC-REVIEW-1`
переведена `OPEN` -> `FIXED` с фактическим доказательством (BOM+LF сохранены).
**Вне объёма (не менялось):** формат эталонов, семантика дублей/GUID, read-back
модели, performance-серии.
**Открытый вопрос к владельцу:** issue #229 на GitHub закрывать? Коммит
не запрошен.
## Spec refactor #228 - грабли сессии R3-R7 (перенесено из IDEA.md)
1. **Черновик с неиспользуемой переменной.** Попытка вынести проверку привязок
   «до цикла» свелась к `bool out_slots_valid = ...`, который нигде не
   использовался: MSVC `/WX` отверг бы, а использование потребовало пропуска
   цикла и изменило `error_element`. Признак, что правка не до конца продумана,
   — новая переменная без единого чтения. Проверять `grep` по имени после вставки.
2. **Числовые смещения в скриптах правки.** Смещение `-5` в строках после более
   ранней вставки попало в середину строки и склеило вызов с комментарием в
   другом месте файла — файл компилировался как есть, поломка была неочевидной.
   Надёжнее якорь по уникальной строке (`next(i for i,x in enumerate(L) if ...)`)
   плюс ассерт на ожидаемое число строк выносимого блока.
3. **`DBtest` печатает `actual = expected`.** Две мои ошибки ожиданий родились
   из неверного чтения этой строки: `@arr_` хвост считался вставленным ПЕРЕД
   `}` вместо замены самой `}`, а следом в комментарий попало непроверенное
   «имя получается незамкнутым». Фактический результат — `{@gdl:p@arr_1_1_1_1_1}`.
   Читать левую часть, а не догадываться; обе формы правдоподобны.
4. **Возвращаемое значение ≠ признак принятия.** `ParseGroups` возвращает `true`,
   даже когда ни одна группа не принята (отказ по числу параметров отбрасывает
   ГРУППУ, разбор продолжается). Проверять `rule.groups`, а не `ok`.
5. **Ожидания для новых тестов — только симуляцией.** Все три набора этого этапа
   (`TestSpecExpandGroup`, `TestSpecOutSlots`, `TestSpecGroups`) сначала
   просчитаны воспроизведением алгоритма, и это окупилось: каждый прогон находил
   расхождения именно в моих ожиданиях, не в коде.
6. **Золотые входы из живой модели, а не из фикстур.** Отладчик VS не отдаёт
   содержимое `GS::UniString` в locals (`GetCharPtr`, `ToCStr`, итераторы
   недоступны в evaluate) — краткое описание снято временным `DBprnt` в `AddRule`
   (удалён после снятия). Из 239 вхождений 5 уникальных — реальные правила
   `test_25.pln`: многострочные, с формулами и кириллицей. Все 5 приняты схемой
   (7 выходных имён, 3 суммы). Синтетика такое не показывает.
7. **`IDEA.md` содержит ДВА BOM, `patch` его снимает.** Инструмент `patch`
   перенёс байты и отказался на расхождении на один символ, хотя сама пачка
   успела примениться. Надёжный путь: сначала
   `git show HEAD:IDEA.md | head -c 20 | xxd`, писать файл через Python
   (сохранять `head_bom` отдельно), вести вставки по уникальной строке. C++
   править таким путём не пользуйся.
8. **`StringSplt` не может вернуть 0.** Он основан на `UniString::Split`
   (`CommonFunction.cpp:1668`), который всегда даёт минимум одну часть. Значит
   проверка `nrule_group < 1` в `ParseGroups` недостижима, и описание без
   маркера `g@@` разбирается как одна обычная группа, а не отвергается. Моё
   ожидание «нет маркера → отказ» было неверным, и прогон это вскрыл. Проверять
   достижимость ветки по КОДУ разбивки, а не по имени условия: имя
   `NoGroupMarker` звучит правдоподобно, но за ним пустой переход.
9. **`spec_baseline.py compare` сам запускает Spec.** Второй прогон на той же
   модели даёт законный no-op 0/0/0, поэтому `compare <tag> <baseline>` против
   первого прогона печатает ложный FAIL (0 против 2/12/2). Для сверки с
   эталоном сравнивать СОХРАНЁННЫЕ `compare-*.json` через `diff_rows`, не
   запуская ArchiCAD заново. Настоящую A/B-регрессию даёт capture на первом
   прогоне после рестарта.
10. **`test_25.pln.lck` переживает убийство ArchiCAD.** Оставшийся lock-файл
    (`Build/SomeStuff/25/test_25.pln.lck`) не даёт открыть проект: ArchiCAD
    стартует, но тесты НЕ выполняются — панель «Отладка» молча показывает
    0 наборов и 0 ошибок, что выглядит как «тесты не нашли». Признак — сессия
    без единого `start== SMSTF ==`. После ЛЮБОГО принудительного
    `Stop-Process` / LNK1168 / падения удалять `.lck` перед следующим
    запуском. Ловится один раз: `rm -f Build/SomeStuff/<v>/test_<v>.pln.lck`.
11. **`UniString::GetSubstring` возвращает view-тип `Substring`.** Возврат из
    функции с выведенным типом даёт ссылку на временный объект, если источник
    временный: `const auto key = [] (...) { return s.GetSubstring (L, R, 0); }`
    — висящий вид, значение приходит мусором (в прогоне ключ пришёл как
    `g@@Material_all@@u;p;f;` из кэша вместо расчётного). Лечится именованной
    локальной переменной и ЯВНЫМ `-> GS::UniString` в лямбде. Прод-`AddRule` в
    этом смысле безопасен: там `description` — именованная переменная, живущая
    до конца функции.
12. **Точка останова на номере строки, сдвинутом моей же вставкой выше.**
    Поставил на `TestFunc.cpp:578` (по координатам ДО вставки) — после вставки
    это оказался `TestSpecMergeAndKey`, а не dedup; `debugger_remove_breakpoint`
    для старого номера вернул «not found», точка жила под новым номером.
    Симптом: останов в функции, которой не касался. Правило: ставить точки
    ПОСЛЕ последней правки файла, перечитывая актуальные номера, и удалять
    перечислением, а не по памяти.
## Spec refactor #228 - вынос в SpecHelpers и очистка комментариев (2026-10-01)
Выполнено `TODO : вынести в SpecHelpers` из `Spec.cpp`: четыре функции вынесены
в новый внутренний модуль `Sources/AddOn/spec/SpecHelpers.hpp/.cpp`.
| Функция | Было | Стало |
|---|---|---|
| `GetSizePlaceElement` | `Spec.cpp:2581`, объявление в `Spec.hpp` | `SpecHelpers.cpp:27` |
| `ParamValueToDumpString` | `Spec.cpp:2902`, `static` + декларация в начале файла | `SpecHelpers.cpp:92` |
| `FillDumpFromParamDict` | `Spec.cpp:2924`, `static` + декларация | `SpecHelpers.cpp:112` |
| `FillDumpGDLParameter` | `Spec.cpp:2944`, `static` + декларация | `SpecHelpers.cpp:130` |
**Эквивалентность переноса доказана сравнением, а не «похоже на то же»:** все четыре
тела сверены с `HEAD` (`4e2c1fa`) посимвольно — идентичны, снят только `static` в
сигнатуре; мультимножество идентификаторов не изменилось (ни один не ушёл и не
пришёл), число вызовов `ACAPI_*` — 0 в обоих. Значит ни порядок разбора GDL-параметров,
ни ветвление по `show_type`, ни состав дампа не изменились.
**Почему отдельный модуль, а не просто вынос в конец файла:** три функции были
`static` с прямыми декларациями в начале `Spec.cpp` — единственный способ вызвать их
из `PlaceElements`, который стоит ниже определений. `TestSpecSizes` обращается к
`GetSizePlaceElement` напрямую, поэтому объявление переехало из `Spec.hpp` в новый
заголовок, и тест подключает его явно.
**Проверки:** clang-format на 5 файлов; clangd 0 ошибок по `SpecHelpers.cpp`,
`Spec.cpp`, `Spec.hpp`, по `TestSpec.cpp` 0 кроме 4 предсуществующих `unused-includes`
(ACAPinc.h, APIEnvir.h, Propertycache.hpp, Sync.hpp — новая строка используется прямо);
AC25 `Build succeeded!` (`SpecHelpers.cpp` подхвачен сборкой без правки конфигурации —
`GLOB_RECURSE`); sweep AC26-29 — все `success`;
`restart_archicad_for_test.ps1` — build+load успешны, exit_code=70 корректно:
`suites=62 passed=2337 failed=1` — **числа ровно те же, что после R3.4**, то есть
перенос не добавил ни одной проверки; `FAILED_SUITE TestConvertPropertyToParamValue` —
предсуществующая вне области. A/B `sh-final` — C=2/M=12/D=2, 14 строк, `diff_rows` =
**0 против шести эталонов** (`p0-smoke`, `r34-final`, `r34-state`, `r73c`,
`r74-final`, `r74-inv`), sha256 фикстуры `db1690f…` совпал.
**Документация:** новая карточка `Docs/modules/spec/SpecHelpers.md`; в `Spec.md` строки
API помечены как вынесенные, добавлен include; `REPOMAP.md` — состав `spec/`;
`symbols.json` 942 → 937 записей, 39 → 40 файлов (сбор через clangd MCP, 7 записей
удалены из `Spec.cpp`, 4 добавлены для `SpecHelpers.cpp`, псевдосимволов 0);
`callgraph.json` не тронут; запись в `Docs/_progress.md`.
**Не тронуто (осознанно).** Три оставшихся TODO в `Spec.cpp`: два `переписать на
switch-case` в перегрузках `SpecFilter` — план запрещает рефакторить `SpecFilter`
попутно (перегрузки имеют разные пути обработки БД и ошибок), это отдельная задача при
доказанной необходимости; `Вынести это в отдельную функцию, убрать повторение в
GetElemState` — это правка `Helpers.cpp`, который под замком (26 включающих единиц,
AGENTS.md §11), и она меняла бы поведение при совпадении счётчиков. Вне scope.
**Параллельная работа владельца.** Пока шёл перенос, `Spec.cpp`, `Spec.hpp`,
`SpecPlanning.*`, `Spec_libpart.*` и `IDEA.md` правились параллельным агентом (чистка
комментариев: убраны ссылки на этапы R/S/F). Три попытки правки упали на проверках
**до** записи — файл не был повреждён ни разу. Перенос применён к стабильному
состоянию; чужие правки комментариев не тронуты и не откатывались.
**Last Checkpoint:** не создан — коммит не запрошен. Код собран и проверен на AC25,
sweep AC26-29 `success`, runtime и A/B выполнены; macOS и AC22-24 не собирались.
## Очистка комментариев модуля Spec — документационный шаг
- [/] Очищены комментарии исходных `Spec.cpp/.hpp`, `SpecPlanning.cpp/.hpp`, `Spec_libpart.cpp` от ссылок на этапы, тестовые чекпоинты и историю переноса. `Spec_libpart.hpp` не менялся. Фиксация только своих фрагментов ещё не выполнена: `Spec.cpp/.hpp` и `IDEA.md` одновременно меняются в параллельной задаче.
- [/] `clang-format` выполнен; clangd: 0 диагностик для пяти изменённых файлов; рабочий `git diff --check` чист. Две попытки выборочной индексации не прошли; третья дала масштабный newline-diff и не прошла `git diff --cached --check`. Индекс очищен только для намеренных файлов, рабочие правки сохранены. Нельзя фиксировать смешанный или невалидированный индекс. Сборка/runtime для очистки комментариев не выполнялись; AC25 Windows — проверочная версия, остальные версии not verified.
- Next Step: после стабилизации параллельных изменений подготовить отдельный индекс с комментариями, проверить его diff/разделители и только затем создать чекпоинт; отдельно проверить новые `SpecHelpers.cpp/.hpp`. Last Checkpoint: `4e2c1fa` (не для очистки комментариев).
## Очистка комментариев тестов — документационный шаг
- [x] Убраны ссылки на этапы/план/issue/историю из `Sources/AddOn/tests/`: снято 437
меток `R#.#`/`S##` из описаний `DBtest`, переписаны блоки комментариев в
`TestSpec.cpp`, `TestSync.cpp`, `TestParam.cpp`, `TestFunc.hpp/.cpp`,
`TestKit.hpp/.cpp`, `TestRenum.cpp`. Вместо ссылки на шаг написано substance:
`outputsRead stays unset until phase 2` вместо `R6.2 phase 2 flag unset`,
«дефект развёртки to_sub» вместо «RED-тест бага P1», «зафиксированный контракт»
вместо «историческое поведение».
- **Проверка, что код не тронут:** сравнение `HEAD` и рабочей копии с
комментариями, вырезанными, показало, что изменились только строковые литералы —
436 потерянных литералов, и все они содержали метку `R#.#`/`S##` при снятии,
ни одного потерянного литерала без метки. Баланс скобок совпадает с `HEAD`.
- **Защищённое сохранено:** разделители `// ----` и баннеры `// ---- kuvbur
2022 ----` не тронуты (в `TestSync.cpp` удалён дублирующийся блок заголовка из
трёх одинаковых строк — это был дубль, не структура).
- `clang-format` на 8 файлов; `git diff --check` чист; clangd: `TestSpec.cpp` —
0 ошибок (4 предсуществующих `unused-includes`), `TestSync.cpp` / `TestParam.cpp` /
`TestKit.hpp` — 0 ошибок и предупреждений, `TestFunc.hpp` — 6 ошибок, идентичные
на `HEAD` (standalone-парсинг заголовка без `ACAPinc`, предсуществующие).
- AC25 Windows Debug: `Build succeeded!` — все 8 изменённых TU скомпилировались.
Runtime не выполнялся: поведение тестов не менялось, менялись только комментарии
и текст меток `DBtest`. AC22-24/26-29, macOS — `not verified`.
- Last Checkpoint: не создан; коммит не запрошен. Рядом лежат чужие правки
`Spec.cpp/.hpp`, `SpecPlanning.*`, `Spec_libpart.hpp`, `IDEA.md` — они не
индексировались и не коммитились.
## Комментарии к функциям SpecPlanning — документационный шаг
- [x] Добавлены русские пояснения к восьми определениям в `SpecPlanning.cpp`: фазы вклада, схема/суммы, раскладка и сверка; алгоритм не менялся.
- [x] `clang-format` выполнен; clangd для `SpecPlanning.cpp`: 0 ошибок и предупреждений; `git diff --check` чист. Сборка и runtime для этих комментариев не выполнялись. В diff того же файла есть параллельная чужая правка R3 (`runState.exsist_elements`) — она не относится к этому шагу и здесь не валидировалась.
- Next Step: продолжить основной R7.5; Last Checkpoint: `cbaad5d`, новый коммит для комментариев не создавался.

## Spec refactor #228 - закрытые этапы R3-R7: пошаговые записи (перенесено из IDEA.md)

## Last Completed (R4.3) + разбор аномалии A/B

**R4.3 закрыт.** Вынесен предикат `OutSlotsMatchSchema ()`; размеры схемы
(`out_slots`, `sum_slots`) вычисляются один раз ДО цикла по элементам. Цикл не
тронут: `Spec.cpp` +21/-3. Закреплено в `TestSpecOutSlots` 9 проверок (8 кейсов +
пустой элемент), ожидания сверены симуляцией 9/9.

**Почему «связать позиции ОДИН раз» сделано частично.** Позиционная связь
возникает из порядка `Push ()` в `out_param`/`out_sum_param`, а не из явного
индекса. Введение индексов изменило бы порядок наполнения и потребовало
переписать ветки материала/формулы — то есть изменить поведение. План также
запрещает в этом же diff менять раскрытие групп; не трогаю.

**Аномалия A/B (13/2/0, 15 строк) — НЕ регрессия.** Воспроизводилась на чистом
HEAD без правки (прогон `r4o-head`). Причина: `Test_file/test_25.pln` изменился —
sha256 `c84b6a6f…` (mtime 01:07:47) вместо `db1690f…`, который держал весь
предыдущий этап. Лишняя ровно одна строка `АР_Спец_Материалы`.

**Восстановлено:** `Test_file/test_25.bpn` (авто-бэкап ArchiCAD, 28 авг 22:04,
56 792 080 байт) содержит **ровно прежний sha256 `db1690f…`** — эталонная модель
была нетронута. Скопирован в `test_25.pln`, текущий сохранён как
`Test_file/test_25.pln.saved-130747` (untracked, вне git). После восстановления
A/B снова даёт 2/12/2 и 14 строк.

**Опровергнутое утверждение skill `archicad-plugin-build`:** там написано «the
test project is NOT saved on close, so the model resets on every restart» и «the
FIRST Spec run after each restart reproduces a fixed 2/12/2 scenario». Фактически
проект БЫЛ сохранён. Скилл исправлен.

**Проверки:** clang-format; clangd 0; AC25 `Build succeeded!`; sweep AC26-29 — все
`success`; тесты в панели «Отладка» — 13 наборов парны, **564 проверки, 0 ошибок**
(`out slots` 9/0); A/B `r4o-ok` — C=2/M=12/D=2, 14 строк, `diff_rows` против
эталона P0 = 0.

## Last Completed (R4.2, четвёртый блок)

**Раскрытие группы вынесено** в `ExpandGroup (group, min_row, rule)` (`Spec.cpp:2013`,
вызов `:2261`) — 80 строк (разворот массивов + размножение по слоям) из
`ParseGroups`. Перенос проверен мультимножеством: **254 -> 254 идентификатора,
24 -> 24 литерала, потерь ноль**. Это самый чистый перенос из четырёх — блок не
использовал внешних локалов, кроме `group` (меняется: `n_layer`, `is_Valid`),
поэтому передан неконстантной ссылкой.

Закреплено в `TestSpecExpandGroup` 16 проверок: обычная группа; несовпадение числа
параметров выхода; несовпадение числа сумм; материалы (50 слоёв, `n_layer` 0..49);
listdata (100 слоёв, 0..99); разворот массива `min_row = 2`.

**Две ошибки в моих ожиданиях, обе пойманы прогоном.** Первая: `@arr_` хвост я
посчитал вставленным ПЕРЕД `}`, откуда ожидалось `{@gdl:p}@arr_1_1_1_1_1}` — но
`ReplaceAll (BRACEEND, ...)` заменяет саму `}`, и фактический результат
`{@gdl:p@arr_1_1_1_1_1}`. Вторая: при первой попытке исправить я написал в
комментарии «скобка не восстанавливается, имя получается незамкнутым» — не проверив
и выдав догадку за факт. Утверждение неверное, скобка на месте, просто переехала
в конец. Обе формы правдоподобны; различает их только прогон.

Проверки: clang-format; clangd 0; AC25 `Build succeeded!`; sweep AC26-29 — все
`success`; тесты в панели «Отладка» — 12 наборов парны, **556 проверок, 0 ошибок**
(`expand group` 16 успешных / 0 ошибок); A/B `r4e-final` — C=2/M=12/D=2, 14 строк,
`diff_rows` против эталона P0 = 0.

R4.2 этим блоком разобран целиком: парсер `GetRuleFromDescription` теперь
делегирует `ApplyRulePolicy` -> `ParseOutputSchema` -> `ParseGroups` ->
`ExpandGroup`.

## Last Completed (R4.2, третий блок)

**Группа `g()` вынесена** в `ParseGroups ()` (`Spec.cpp:2011`, вызов `:2337`) — 235
строк разбора из парсера. Перенос проверен мультимножеством: 654 -> 654
идентификатора, 58 -> 58 литералов; расхождения только ожидаемые (переименования
параметров и два `return rule;` -> `return false;`).

По дороге сломал файл: числовое смещение `-5` в скрипте правки попало в середину
строки и склеило вызов с комментарием в другом месте файла. Откатил
`git checkout -- Sources/AddOn/spec/Spec.cpp` и перенёс заново по уникальным
строковым меткам вместо индексов. Урок: в скриптах правки надёжнее якорь по
уникальной строке, чем смещение в строках.

**Закреплено в `TestSpecGroups` 24 проверки.** Первый прогон дал 2 ошибки — обе в
моих ожиданиях, не в коде:
1. вторая группа обязана начинаться с `g@@`, иначе `q1@@u2` склеивается в одно
   имя и второй группы не будет;
2. `ParseGroups` возвращает `true` даже когда ни одна группа не принята — отказ
   по числу параметров отбрасывает ГРУППУ, а не правило. Признак принятия —
   `rule.groups`, а не возвращаемое значение.
Обе ошибки закреплены в тесте как контракт и в Docs.

Проверки: clang-format; clangd 0; AC25 `Build succeeded!`; sweep AC26-29 — все
`success`; тесты в панели «Отладка» — 11 наборов парны, **539 проверок, 0 ошибок**
(`groups` 24 успешных / 0 ошибок); A/B `r4g-final` — C=2/M=12/D=2, 14 строк,
`diff_rows` против эталона P0 = 0.

## Last Completed (R4.2, второй блок)

**Выходная схема `s()` вынесена** в `ParseOutputSchema ()` (`Spec.cpp:2016`).
30 строк разбора внутри парсера стали проверяемой единицей: 11 кейсов схемы в
`TestSpecOutputSchema` (ровно две части / одна / три / пустая; срезание `[N]`;
пустые имена; хвостая запятая; отсутствие `s@@`).

**Найдена и исправлена регрессия, которую я внёс сам.** Первая версия функции
сбрасывала `rule.parseValid = false` на входе — безусловно, а не только при отказе.
Это ломало `parseValid` у КАЖДОГО правила: 45 ошибок в тестах, включая все
существующие наборы, которые до этого были зелёными. Поймано собственным тестом
(6 кейсов `parse flag`). Флаг теперь сбрасывается только в ветке отказа.
Урок: при выносе блока, который меняет флаг, сброс нельзя оставлять на входе
функции — он переживает успешный путь.

**Проверено на РЕАЛЬНЫХ данных, а не только на синтетике.** Отладчик VS не отдаёт
содержимое `GS::UniString` в locals (`GetCharPtr`/`ToCStr`/итераторы недоступны
в evaluate), поэтому краткое описание из `test_25.pln` пришлось снять временным
`DBprnt` в `AddRule` (потом удалён). Из 239 вхождений 5 уникальных оказались
реальными правилами модели — все многострочные, с формулами и кириллицей
(`Spec_rule_v3 {"АР_Спец_Материалы"; gm(...%nosyncname%...) s(...)}`).
**Все 5 приняты схемой** (7 выходных имён, 3 суммы). Это приёмка R4.2 на живой
модели, а не на фикстурах.

Проверки: clang-format; clangd 0; AC25 `Build succeeded!`; sweep AC26-29 — все
`success`; тесты в панели «Отладка» — 10 наборов парны, **515 проверок, 0 ошибок**
(`output schema` 26 успешных / 0 ошибок); A/B `r4s-final` — C=2/M=12/D=2,
14 строк, `diff_rows` против эталона P0 = 0.

Два присланных владельцем правила (`АР_Спец_Перемычки` и `Spec_rule_v2` с `gm`)
оказались примерами из `wiki/ru/Breakdown-Composites-Create-Elements-ru.md`
(строки 102-103) — то есть спецификация их считает валидными. Разбор обоих
проверен, обе части `s()` дают ровно две части.

## Last Completed (R4.2, первый блок)

**Политика правила вынесена** из парсера в `ApplyRulePolicy ()` (`Spec.cpp:1969`,
вызов на `:2012`). 27 строк ветвления стали отдельной единицей, проверяемой без
разбора описания целиком: `TestSpecPolicy` (`TestFunc.cpp:536`) — 10 случаев
политики, проверка сохранности разобранных частей и проверка вызова из парсера.

**Зафиксировано как контракт (НЕ исправлено):** имя сравнивается по МЕСТУ
`"pec_rule"`, а не по префиксу целиком, поэтому `"XXpec_rule_v2XX"` получает
политику v2; при двух маркерах выигрывает первый по порядку ветвления (v2 раньше
v3, v3 раньше KM/KZH), а KM/KZH затем перекрывает поля v2/v3. Всё это теперь
закреплено тестом. Менять нельзя: описания правил в существующих моделях на это
завязаны.

`local_scratch` намеренно остался в парсере — он нужен шести вызовам `StringSplt`
ниже по функции, и вынос объявления унёс бы его из области видимости. Проверено
переносом мультимножеством токенов: 0 потерянных токенов и 0 потерянных строковых
литералов.

**Мой тест сперва был неправ и это стоило цикла.** В проверке «политика из
парсера» я передал сырую строку `"Spec_rule_km{Fav;g(u;p;f;q)s(x;y)}"` и получил
`parseValid=false`. Причина — ровно тот контракт, что закреплён в R4.1: парсер
принимает ТОЛЬКО нормализованную строку (`g@@…@@s@@…`). Та же ошибка в остальных
местах тестов отсутствует — проверено сквозным поиском по всем вызовам
`GetRuleFromDescription`.

Проверки: clang-format; clangd 0 ошибок; AC25 `Build succeeded!`; sweep AC26-29 —
все `success`; тесты в панели «Отладка» — 9 наборов парны, 489 проверок в
`SpecRegression`, **0 ошибок** (все 3 проверки `wired` прошли); A/B `r4p-final` —
C=2/M=12/D=2, 14 строк, `diff_rows` против эталона P0 = 0.

## Last Completed (R4.1)

**Нормализация описания вынесена** из тела `AddRule` в отдельную
`NormalizeRuleDescription ()` (`Spec.cpp:1222`). Побочный эффект: 33 замены,
которые нельзя было ни увидеть, ни проверить, стали самостоятельной единицей с
контрактом, закреплённым 18 проверками `TestSpecNormalize` (входы: переводы
строк/табуляции, серии пробелов, скобки и `;`, все виды вызовов `g(`/`s(`/`gl(`/
`gm(`; плюс сквозной путь «нормализация -> разбор» и неизменность исходной строки).

**Мои ожидания сначала оказались неверны, и это было полезно.** Я написал 17 пар
вход->выход умозрительно, ожидая `g@@a;b;c` без хвостовой скобки. Симуляция
алгоритма показала обратное: нормализация `)` НЕ убирает, остаётся `g@@a;b;c)`.
Канон подтверждён уже существующим тестом `TestSpecAddRule` (ключ
`Fav;g@@u;p;f;q@@s@@x;y)`) и комментарием парсера (`description.Trim (')')`).
Тесты переписаны по факту и теперь сверяются с симуляцией автоматически (17/17),
а не «на глаз».

**Попутно исправлено моё же неверное утверждение в комментарии.** Я написал, что
порядок `g(`/`gl(`/`gm(` обязателен, иначе `gl(` станет `g@@libdata@(`. Перебором
перестановок установлено обратное: `"g("` не входит в `"gl("` и `"gm("`, поэтому
внутри блока маркеров порядок неважен. Существенен только порядок МЕЖДУ блоками:
сперва ужимаются пробелы, потом пишутся маркеры — иначе `"g (a)"` даёт `"g@@ (a)"`
и группа не разбирается. Шесть проходов схлопывания сводят серию пробелов к
одному для длин до 64; длиннее — не полностью. Комментарий переписан на проверенный.

**R4.3 выполнен попутно с R4.1:** `GetRuleFromDescription` переведён на
`const GS::UniString &` и правит локальную копию. Раньше парсер оставлял вход
в обрезанном виде, и это держалось тем, что вызывающий не читал строку после
вызова. Тест «input consumed» заменён на «input intact» + «key reusable after parse».

**R4.2 НЕ начат.** Мои промежуточные подпункты («разделить парсер и словарь»)
не совпадают с планом: план требует разбирать по одному — политика правила ->
выходная схема -> группа -> раскрытие массива/материалов/listdata. Нумерация
приведена к плановой, чтобы не выдавать свой порядок за согласованный.

Проверки R4.1: clang-format; clangd 0 ошибок по обоим файлам; AC25 `Build succeeded!`;
sweep AC26-29 тоже `success` (регресс-ворота); тесты в панели «Отладка» — все 8
наборов парны (start=1/end=1), 436 проверок в `SpecRegression`, 0 ошибок
(`input intact` 5/5, `key reusable after parse` 5/5); A/B `r4-final` — C=2/M=12/D=2,
14 строк, `diff_rows` против эталона P0 = 0.

## Last Completed (R3.2/R3.3)

`is_Valid` разнесён на три признака, каждый пишется своей стадией:
- `parseValid` — 7 мест записи в парсере (1986/1994/2032/2061/2264/2266/2268);
- `selected` — диалог `:504`, список имён `:616`;
- `destinationReady` — сверка с избранным `:683`, `:690`;
- `IsRunnableForRun ()` (Spec.hpp:64) заменяет чтения в `:632`, `:830`, `:1040`.

R3.3: `subguid_paramrawname` теперь только маркер из описания (`:1260`), найденное
свойство избранного пишется в `destinationParamGuidName` (`:739`) и читается в
`:783`, `:1710`, `:1876`. Прежде `:727` и `:1876` читали поле ПОСЛЕ перезаписи —
поведение сохранено, но теперь это видно по коду, а не держится на «словарь новый».

`GroupSpec::is_Valid` намеренно оставлен как есть (другой смысл, переименование
не требовалось контрактом этого шага).

**Найдена и исправлена моя регрессия в тесте.** Первый прогон дал
`ERROR IN TEST: Spec link copied` — тест сравнивал `Element::subguid_paramrawname`
с маркером правила, что после R3.3 неверно по замыслу. Фикстура разделена на
маркер и разрешённое свойство, проверка переведена на второе.

**Грабли, стоившие двух зависаний ArchiCAD:** после `Stop-Process` нужно удалять
`.lck` в ОБОИХ местах — `Test_file/test_25.pln.lck` И `Build/SomeStuff/25/test_25.pln.lck`
(runner запускает копию из Build, а debugger-запуск — нет; runner чистит только
свою). Признак зависшего диалога: окно `MainWindowTitle` без имени проекта и
`enabled=False` при UI Automation, при этом файл свободен для записи. Раньше я
проверял только `Test_file/` и дважды поймал «файл уже используется».

**A/B:** 3 прогона после рестарта ArchiCAD (модель не сохраняется на диск —
`test_25.pln` не менялся с 22:04), каждый `completed` C=2/M=12/D=2, 14 строк,
`diff_rows` против `compare-p0-smoke.json` — **0 расхождений**.

Тесты: `SpecRegression values/read plan/grouping/reconcile/parser/add rule/sizes` —
все `end`-маркеры на месте; `ERROR IN TEST` одна,
`ConvertToParamValue(Property) : doubleValue (отрицательное)` — предсуществующая.

## Last Completed (R3.1)

Выписка всех чтений/записей состояния — `Reviews/2026-09-28_r3-state-inventory.md`
(Reviews gitignored, локально). C++ не менялся.

Ключевой результат: `SpecRule::is_Valid` — один флаг с **четырьмя разными смыслами**:
корректность разбора (1979/1987/2025/2054), дедупликация в AddRule (1248/1253/1264),
выбор пользователем или JSON (614-617), готовность назначения — избранное содержит
все выходные свойства (670-696). Плюс шестое место — фильтр показа (81).

Подтверждённые факты:
- `SpecDG` вызывается на 811, то есть ПОСЛЕ сверки с избранным (670-696): пользователю
  показывается меньше правил, чем в словаре, и это нигде не отражено.
- `subguid_paramrawname` хранит и маркер из описания (1256), и найденное имя свойства
  (735); обе перезаписи — только при `is_Valid == true`. Инвариант «маркер == найденное
  имя» в коде не зафиксирован; сейчас безопасно, т.к. словарь правил создаётся заново
  каждый запуск (619-638 идут раньше 719).
- `exsist_elements` не очищается — безопасно только потому, что словарь новый.
- `GroupSpec::is_Valid` — отдельный флаг с другим смыслом, но одноимённый (1304/1583/2222).

## Last Completed (P3)

**Фактически проверено, что автоматизировать сценарии P1 нечем:**
- `API.Undo` / `API.Redo` в JSON-порту AC25 отсутствуют — реальный вызов вернул
  `{"succeeded": false, "error": {"code": 2002, "message": "Command 'API.Undo' not found"}}`.
- В аддоне регистрированы только `Spec`, `RoomBook`, `Health` — команды управления
  моделью (удалить spec-объекты, задать свойство) не существуют.
- `API.SetPropertyValuesOfElements` отвечает success, но не пишет (#226).

Следствие: сценарии «создание с нуля», «изменение суммы/значения», «изменение ключа»,
«исчезновение строки», «смешанный create+update+delete» не могут быть сняты
автоматически ни сейчас, ни после рефакторинга, пока не появится отдельное средство
чтения/изменения модели. Это ограничение стенда, а не отложенная работа.

**Baseline (AC25 Debug, тёплый процесс, `test_25.pln` в сошедшемся состоянии):**
- `includeParameters=false`: median 0.8363 s, min 0.8228, max 0.8600, spread 4.44 %, stdev 0.0117 s.
- `includeParameters=true`: median 0.8202 s, min 0.8015, max 0.8391, spread 4.59 %, stdev 0.0130 s.
- Все 20 прогонов: `completed`, C=0/M=0/D=0.
- Обе серии почти равны, потому что на no-op `PlaceElements` не вызывается: **эти цифры
  не доказывают, что выключенный дамп бесплатен на рабочем пути**. Сценарий create —
  единственный, где платится за сбор дампа, и он недоступен стенду.
- Разброс ~4.5 % означает: сравнение «быстрее на 5 %» на этом стенде неотличимо от шума.

Артефакты: `Tools/spec_benchmark.py`, `Reviews/spec-refactor-baseline/`
(`manifest.md`, `runs-base-a.csv`, `timing-base-a.json`) — локально, gitignored.

## Last Completed (P0)
**Исправлен баг в моём стенде** `Tools/spec_baseline.py:70` — `urllib.request.urlopen()`
не принимает `headers=`; TypeError ронял поиск порта. Заменено на `Request` + `urlopen(req)`.
Стенд после этого отработал: порт 19723.

Прогоны AC25 (`test_25.pln`, реальный ArchiCAD, `SomeStuffCommand.Spec`):
- 1-й, `includeParameters=true`: `completed`, C=2 / M=12 / D=2, 3.66 s, 14 строк дампа.
  Структура проверена: `created[0]` = 11 property, 28 sourceElement, gdlParameter пуст.
- 2-й и 3-й: `completed`, 0/0/0, ~0.79 s — состояние устойчиво, соответствует #226.
- `compare p0-noop` -> PASS (0 строк, счётчики совпали).

Дамп — эталон подготовленной записи, НЕ снимок конечной модели. Все три списка
(`created`/`modified`/`deleted`) заполняются до применения операций, поэтому непустой
массив не доказывает запись. `modified.gdlParameter` всегда пуст — в отличие от `created`.
Независимого read-back модели нет, сохранность записанных значений — `not verified`.

**НЕ выполнено (нужна ручная подготовка модели в ArchiCAD):** сценарии P1 «изменение
суммы/значения», «изменение ключа», «исчезновение строки», «смешанный create+update+delete»,
«повтор одного материала в разных слоях». Работающего автоматического setter нет
(API.SetPropertyValuesOfElements отвечает success, но не пишет — зафиксировано в #226).

## R4.4 — ЗАКРЫТ (вариант (а), решение владельца 2026-09-29)

**Что сделано.** Введён `Spec::ParseError` (enum class, 8 значений), поле
`SpecRule::parseError` и функция `ParseErrorText (ParseError)`. Причина
заполняется **в тех же 7 точках**, где сбрасывается `parseValid`
(`Spec.cpp:2164/2194/2341/2404/2418/2422/2426`); сами `parseValid = false` и все
ранние `return` не тронуты. `AddRule` печатает причину и исходное
`definition.description` в существующий `msg_rep` (`Spec.cpp:1303`) — то самое
описание, ради которого пункт плана и требовал «не терять исходную строку».

**Адаптеры не понадобились** — как и предсказывал разбор: внутреннего результата
«разбор + диагностика» не существует, `GetRuleFromDescription` возвращает
`SpecRule` напрямую, мигрировать нечего. Текст причины — константа, описание не
копируется повторно, сообщение собирается только для невалидного правила (запрет
плана R7.3 на дорогой текст на каждую строку соблюдён).

**Найдено прогоном: `ParseError::NoGroupMarker` недостижим.** Моё ожидание
«нет маркера `g@@` → отказ» оказалось неверным. `StringSplt` основан на
`UniString::Split` (`CommonFunction.cpp:1668`), который возвращает минимум одну
часть, поэтому `nrule_group < 1` не наступает никогда: описание без маркера
разбирается как ОДНА обычная группа и при совпадении размеров со схемой
принимается. Первая сборка дала 2 ошибки `ERROR IN TEST` — обе в моих
ожиданиях, не в коде. Тест переписан по факту: отдельный блок закрепляет
неотказ и достижимость `NoGroupsAccepted`; значение в enum оставлено как
защита на случай строгой разбивки, о чём прямо сказано в комментарии.

**Каскад финальных проверок** (закреплён тестом): три проверки в конце
парсера не прерываются, поэтому при нескольких нарушениях видна ПОСЛЕДНЯЯ
причина — пустые обе схемы дают `EmptySumSchema`, а не `EmptyOutputSchema`.

**Проверки:** clang-format; clangd 0 ошибок по обоим файлам (warning про
неиспользуемый `Helpers.hpp` предсуществующий); AC25 `Build succeeded!`;
sweep AC26-29 — все `success`; тесты в панели «Отладка» — 17 наборов, все
`end`-маркеры на месте, **880 проверок `: ok`, 1 `ERROR IN TEST`**
(`ConvertToParamValue(Property) : doubleValue (отрицательное)` —
предсуществующая, вне области), новый набор `parse error` — 61/61 успешных;
финальная проверка `Tools/restart_archicad_for_test.ps1` exit_code=0 (build+load);
A/B `r44-ok` — C=2/M=12/D=2, 14 строк, `diff_rows` против эталона P0 = **0**.

**Осторожно при следующем прогоне A/B:** `python Tools/spec_baseline.py compare`
САМ вызывает Spec, поэтому второй прогон на той же модели даёт законный no-op
0/0/0 и даёт ложный FAIL против первого прогона. Для сверки с эталоном P0
сравнивать сохранённые файлы (`compare-p0-smoke.json` против свежего
`compare-<tag>.json`) через `diff_rows`, без повторного запуска ArchiCAD.

## R4.5 — ЗАКРЫТ (контракты, без правки прода)

**Что сделано.** Прод-код **не менялся**. Добавлены два набора в `TestFunc.cpp`
(+290 строк), закрепляющие наблюдаемое поведение, которое сломает любая
будущая правка `GetElementsForRule` / `AddRule`:

- `TestSpecMergeAndKey` — 24 проверки. S08: при `elements.ContainsKey (key)`
  новый элемент не создаётся, источник дописывается **повторно** (новизна не
  проверяется), суммы складываются по позициям, где оба значения `isValid`,
  первый представитель сохраняет свои выходные, `n_elements` растёт **только
  при создании**. S09: разделителя между уникальными параметрами в ключе нет
  (`Spec.cpp:1685`), поэтому `"A@B"` одним параметром совпадает с `"A"`+"B"
  двумя — строки схлопываются; закреплены оба направления (совпадение и
  контрольное несовпадение). S07: литерал `"1"` начисляется на **каждый**
  источник (два источника с одинаковым ключом дают 2, а не 1); отказ по
  размеру группы сделан в `ExpandGroup`, поэтому локален для группы; неполный
  выход при `stop_on_error` обнуляет `n_elements`, чистит словарь, помечает
  элемент.
- `TestSpecRuleDedup` — 17 проверок. Ключ словаря = подстрока **между
  фигурными скобками** нормализованного описания (`Spec.cpp:1276`); префикс
  политики (`Spec_rule`, `_v2`, `_v3`, `_km`) в него **не входит**, поэтому
  описания с одинаковым телом и разной политикой схлопываются в пользу
  пришедшего первым (проверено в обе стороны). Правило попадает в словарь даже
  при `parseValid == false` — «чтоб потом дважды не обрабатывать», поэтому
  невалидное правило занимает ключ навсегда и не принимает источники.

**Два дефекта в моих ожиданиях, пойманные до прогона.** (1) Сценарий коллизии
ключа был построен неверно — разные наборы уникальных параметров давали
разные ключи; (2) проверка размеров группы отнесена к расчёту строк, хотя она
живёт в `ExpandGroup`. Оба переписаны по коду, а не по «желаемому».

**Падение ArchiCAD на `rule dedup` — мой баг в тесте.** Лямбда `ruleKey`
возвращала `auto`, а `UniString::GetSubstring` отдаёт **view-тип `Substring`**,
ссылающийся на нормализованную строку; без явного типа возврата получался
висящий вид на временный объект, и `bodyKey` приходил случайным мусором из
кеша. Исправлено явным `-> GS::UniString` с копией. **Прод здесь безопасен:**
в `AddRule` (`Spec.cpp:1271`) `description` — именованная переменная, живущая
до конца функции, а ключ копируется в `GS::UniString key`.

**Проверки:** clang-format; clangd по `TestFunc.cpp` — 9 ошибок, все
предсуществующие (сверено со сдвигом HEAD = 245 строк); AC25 `BuildAddOn.py`
`Build succeeded!`; sweep AC26-29 — все `success`; `restart_archicad_for_test.ps1`
exit_code=0 (build+load); тесты в панели «Отладка» (сессия 12:40:50) —
**19/19 наборов завершено, 901 `: ok`, 1 `ERROR IN TEST`**
(`ConvertToParamValue(Property) : doubleValue (отрицательное)` — предсуществующая,
вне области); `TestSpecMergeAndKey` 24/24, `TestSpecRuleDedup` 17/17; A/B против
эталона P0 — `diff_rows` = **0**, C=2/M=12/D=2, 14 строк, sha256 фикстуры
`test_25.pln` = `db1690f…` (совпал).

Строка `== ERROR == Rule is not valid: Sync_name_Broken (output…)` в блоке dedup
— **ожидаемая**: сценарий намеренно кормит невалидное описание ради проверки
R4.4-диагностики; `ERROR IN TEST` при этом не возникает.

## R4.6 — ЗАКРЫТ (остаток R4.3: привязка слотов, по решению владельца)

**Номера R4.6 в плане НЕТ.** План R4 заканчивается на R4.5 (строка 254),
дальше сразу R5 (строка 258) — проверено сквозным поиском по всему файлу.
Владелец 2026-09-30 определил R4.6 как остаток R4.3 из независимой оценки.

**Что сделано.** `SlotBinding` и `GroupSlotBinding` (`Spec.hpp`), функция
`PrepareSlotBindings (const SpecRule &)` (`Spec.cpp`), вызов в
`GetElementsForRule` ДО цикла по элементам. Привязка строится один раз на
правило: по одной на группу, в порядке `rule.groups`. Цикл читает готовые
`slot.rawname` / `slot.isSumLiteral` вместо обхода имён группы и `IsEqual ("1")`
на каждый источник. Цикл по группам переведён на индексированный доступ,
размер привязок равен `rule.groups.GetSize ()`.

**Имена НЕ копируются** — хранится `const GS::UniString *` на поле группы.
Основание: на время исполнения `rule.groups` не меняется (группы строит
парсер). Копия 50-100 имён на каждое правило противоречила бы требованию
R4 о нерастущей памяти. Контракт закреплён тестом: правка поля после
подготовки видна в привязке.

**`sizesMatchSchema` НЕ заменяет сверку в цикле — это осознанно.** Признак
сравнивает число ПОЛЕЙ группы с размерами схемы, но `Push ()` выполняется
только при успешном чтении. Группа с 3 полями против схемы в 2 слота даст
элемент с 2 значениями, который `OutSlotsMatchSchema` примет. Сделать
предварительную проверку фильтром = изменить поведение на этом кейсе.
Предсуществующий факт (зафиксирован в R4.3): `ExpandGroup` проверяет размеры
только в ветке `min_row == 0`, так что группы-массивы отбрасывались бы ещё
сильнее. Поэтому сверка осталась в цикле.

**Что осталось не сделано (осознанно).** Явные числовые индексы слотов не
введены: порядок наполнения задаётся последовательностью `Push ()` в ветках
материала/формулы, переписывание изменило бы поведение. Раскрытие фиксированного
числа групп на «фактическое число слоёв» не делалось — план это прямо запрещает
в том же diff.

**Проверки:** clang-format на 3 файла; clangd 0 ошибок по `Spec.cpp` и
`Spec.hpp`, по `TestFunc.cpp` — 9 ошибок, все предсуществующие (сверено:
мои строки в диагностике не фигурируют, счётчик тот же, что до правки);
AC25 `BuildAddOn.py` — `Build succeeded!`; sweep AC26-29 — все `success`;
`restart_archicad_for_test.ps1` exit_code=0 (build+load); тесты через VS
`debugger_launch` — **20 наборов с end-маркерами, 1100 проверок `ok`, 1
`ERROR IN TEST`** (`ConvertToParamValue(Property) : doubleValue
(отрицательное)` — предсуществующая, вне области), `TestSpecSlotBindings`
30/30; A/B `r46-ok` — C=2/M=12/D=2, 14 строк, `diff_rows` против эталона P0 = 0.

**Грабли этого шага.** (1) `restart_archicad_for_test.ps1` запускает Archicad
ВНЕ VS, и его вывод НЕ попадает в панель «Отладка» — MCP отдаёт вчерашнюю
сессию, и по «нету набора» легко сделать вывод, что тест не вызывается.
`debugger_launch_without_debugging` тоже не пишет в панель. Рабочий путь:
`debugger_launch` (F5) — тогда сессия видна. Первый вывод VS MCP после
запуска был за 12:47 при реальном времени 12:27 — время в панели отстаёт,
ориентироваться надо на СОДЕРЖИМОЕ (список наборов), а не на метку времени.
(2) Набор, добавленный в `TestSpecRegression`, в чужом выводе отсутствует —
это верный признак прочитанной не той сессии, а не «тест не вызвали».
(3) `Docs/modules/spec/Spec.md` и `IDEA.md` — CRLF, `patch` их не находит
даже при точном тексте; править через Python с `newline=""` и склейкой
якорей по `\r\n`. IDEA.md вдобавок с ДВУМЯ BOM — `encoding="utf-8"` при
чтении сохраняет первый, второй уходит в первую строку; записывать надо
проверкой первых байтов.

## R5.1 — ЗАКРЫТ (сбор зависимостей правила отделён от их разворачивания)

**Что сделано.** `GetParamToReadFromRule` делала три разных дела вперемешку:
перечисляла зависимости правила, разворачивала имена в словарь ОДНОГО элемента
(включая разбор материалов и формул) и сливала результат в общие словари запуска.
Разнесено на три единицы:

- `RuleDependencies` (структура, `Spec.hpp`) — что читать у источников и что
  потом записывать;
- `CollectRuleDependencies (const SpecRule &)` (`Spec.cpp:1327`) — чистое
  перечисление имён из уже разобранного правила, без обращения к модели;
- `BuildReadParamDict (const ParamDict &, ParamDictValue &)` (`Spec.cpp:1384`) —
  разворачивание имён материалов и формул в служебные свойства слоя и само
  выражение;
- `GetParamToReadFromRule` (`Spec.cpp:1433`) — **адаптер прежней формы** вызова,
  единственный вызов `SpecArray:635` не тронут. Объединение запросов между
  правилами осталось в адаптере: `AddParamDictValue2ParamDictElement` не трогает
  уже набранное, поэтому кратность чтения компонентов и порядок добавления
  прежние. R5.2-R5.3 смогут заменить адаптер на контекст, не трогая сбор.

**Закреплено `TestSpecRuleDependencies` — 35 проверок, 0 ошибок.** Пустое правило;
обычная группа; литерал `"1"`; невалидная группа; пустой флаг; слои материалов и
ведомостей; повторяющиеся группы; разворачивание имени материала (толщина,
единицы, коэффициент запаса, 20 имён синхронизации, `fromMaterial`/`fromQuantity`,
`composite_pen = -2`); разворачивание имени формулы (`hasFormula`); сличение
адаптером.

**Первая сборка дала 4 ошибки — все в моих ожиданиях, не в коде.** Пустой
`flag_paramrawname` ПОПАДАЕТ в перечень и отбрасывается только при разворачивании
(`AddValueToParamDictValue` игнорирует пустое имя), поэтому счётчик перечня на
единицу больше счётчика словаря элемента в каждом блоке, где флаг не задан. Я
посчитал перечень как словарь. Исправлено по фактическим значениям (actual = 2,
3, 3, 2) и закреплено отдельной проверкой «пустой флаг в перечне, но не в
словаре элемента». Побочно это зафиксировало реальную разницу между перечнем и
разворотом, а не дефект.

**Два контракта, которые теперь явны:** флаг НЕВАЛИДНОЙ группы читается
(добавление до проверки `is_Valid`), хотя остальные её поля — нет: по флагу
элемент отбрасывается в цикле, и без чтения отбрасывался бы иначе. Служебные
свойства материалов и разбор формул в перечень НЕ входят — они появляются
только в `BuildReadParamDict`.

**Проверки:** clang-format на 3 файла; clangd 0 ошибок по `Spec.cpp`, `Spec.hpp`,
`TestFunc.cpp`; AC25 `BuildAddOn.py` — `Build succeeded!`; sweep AC26-29 — все
`success`; `restart_archicad_for_test.ps1` exit_code=0 (build+load); тесты через
VS `debugger_launch` (сессия 13:04) — **20 наборов с end-маркерами, 1836
проверок `ok`, 1 `ERROR IN TEST`** (`ConvertToParamValue(Property) : doubleValue
(отрицательное)` — предсуществующая, вне области), `TestSpecRuleDependencies`
35/35; A/B `r51-ok` — C=2/M=12/D=2, 14 строк, `diff_rows` против эталона P0 = 0,
sha256 фикстуры `test_25.pln` = `db1690f…` (совпал).

**Грабли этого шага.** (1) **LNK1163 «ambiguous COMDAT 0x2759» вместо привычного
LNK1168** — не Archicad держит `.apx`, а смесь старых и новых `.obj` в
`Build/SomeStuff/25`: рядом лежали `SomeStuff.dir/Debug` (свежий) и
`INT/SomeStuff.dir/RelWithDebInfo` (от 27.09). Лечится удалением всех `*.obj` и
`*.pch` в каталоге сборки; флага очистки у `BuildAddOn.py` нет. Признак — `.apx`
свободен, а линковка падает на COMDAT. (2) Строки `DBtest` в панели «Отладка»
заканчиваются на `== SMSTF ==`, а не на `ok`, поэтому подсчёт по суффиксу `ok`
даёт 0 успешных проверок и выглядит как полный провал набора. Считать надо по
`ok== SMSTF ==`. (3) Archicad из `restart_archicad_for_test.ps1` запускается
ВНЕ VS, и его вывод в панель «Отладка» не попадает — читать тесты можно только
после `debugger_launch`.

## R5.2 — ЗАКРЫТ (разрешение избранного, служебных полей и старых объектов)

**Что сделано.** Один плотный блок в `SpecArray` разбит на четыре операции:
`MatchDestinationProperties` (сверка выходной схемы с избранным + `error_name`),
`ResolveFavoriteLinks` (поиск носителей имени правила и GUID по описанию),
`SelectExistingElements` (отбор ранее созданных элементов),
`AddExistingReadRequests` (запросы на их чтение). **Момент вызова относительно
диалога не менялся** — разрешение осталось до `SpecDG`, это прямое требование
R5.2.

**Два решения, которые пришлось принять осознанно.**

1. `flag_find` стал возвращаемым значением `ResolveFavoriteLinks`. Первая версия
   подставила `destinationParamGuidName.IsEmpty ()` — это вывод по умолчанию, а
   не тот же признак. Поймано чтением кода до сборки.
2. `SelectExistingElements` асимметрична намеренно: пустой `selected_elements`
   ПРИСВАИВАЕТ найденное, непустой — ДОПИСЫВАЕТ (прежний `Push`). Присваивание в
   обеих ветках изменило бы поведение при непустом `exsist_elements` на входе;
   сейчас безопасно только тем, что словарь правил создаётся заново на каждый
   запуск. Закреплено двумя противоположными проверками теста.

**Отказ кэшировать неудачное чтение сохранён** (промах `GetParamValueFromCache` →
переход к следующему свойству, найденные ранее поля не закрепляются). Новая
политика кэша не вводилась — R5.2 запрещает её без F.

**Перенос проверен мультимножеством** (код без комментариев): 289 -> 286
идентификаторов, расхождения только ожидаемые — новые имена функций, сигнатуры,
`flag_find` -> `guidFound`, `pRuleFavorite` -> `favorite`. Единственный ушедший из
блока литерал `DBprnt` для `subguid_rulename` остался на месте вызова.

**Закреплено `TestSpecFavoriteResolution` — 33 проверки, 0 ошибок.** Ожидания
считались по коду: `GetParamValueFromCache` читает ambient-кэш свойств, поэтому
на синтетических именах он ПРОМАХИВАЕТСЯ — это детерминированный исход, закреплённый
как контракт. Имена в `AddExistingReadRequests` уходят в нижнем регистре
(`NameToRawName`), поэтому тест сверяет `{@property:spec-guid}`.

**Ошибка сборки в моём тесте, не в проде:** локальные константы `OUT`/`SUM`
съедал препроцессор — `SUM` макрос из `winbase.h`. MSVC дал C3861, clangd по
compile DB ошибок не показал. Переименованы в `OutName`/`SumName`. **Вывод: имена
локальных констант в тестах сверять со списком макросов Windows — clangd здесь не
защищает.**

**Проверки:** clang-format на 3 файла; clangd 0 ошибок; AC25 `BuildAddOn.py` —
`Build succeeded!`; sweep AC26-29 — все `success`; `restart_archicad_for_test.ps1`
exit_code=0 (build+load); тесты через VS `debugger_launch` (сессия 13:42) —
**22 набора с end-маркерами, 1868 проверок `ok`, 1 `ERROR IN TEST`**
(`ConvertToParamValue(Property) : doubleValue (отрицательное)` —
предсуществующая, вне области), `TestSpecFavoriteResolution` 33/33; A/B `r52-ok` —
C=2/M=12/D=2, 14 строк, `diff_rows` против эталона P0 = 0, sha256 фикстуры
`db1690f…` (совпал).

## R5.3 — ЗАКРЫТ (один набор прочитанных словарей)

**Что сделано.** Введён `Spec::SpecReadContext` (поля `read` / `composite` /
`listData`). Владеет им `SpecArray` на весь запуск. `GetParamValue` и
`GetElementsForRule` принимают контекст вместо трёх отдельных словарей — сигнатуры
сократились с 8/9 до 6/7 аргументов. **Пакетное `ElementsRead` осталось на
прежнем месте** (сразу после `SpecDG`), менялось только то, какие поля контекста
оно заполняет.

**Зафиксированная ambient-зависимость — предмет R5.3.** `GetParamValue` читает
значения только из трёх словарей и элемент не открывает, НО транзитивно зовёт
`ReadFormula` -> `EvalExpression` (`CommonFunction.cpp:1349`), а тот под `TESTING`
пишет `PROPERTYCACHE ().formulaCacheStats` и держит статический кэш выражений со
сбросом при >4096. Остальные проверенные транзитивные вызовы ambient-эффектов не
имеют. **Вывод: отсутствие записи в модель != полностью чистая функция.** Эти
эффекты не тронуты — `Helpers.cpp` под замком (#228), правка заделала бы 26
включающих единиц; устранение — отдельная задача с A/B по счётчикам формул.

**Сборщики запросов намеренно оставлены с `ParamDictElement &paramToRead`:** это
словари ЗАПРОСОВ, а не прочитанные данные, они не часть контекста.

**Ошибка правки, пойманная сборкой.** Regex-замена многострочных вызовов
`GetParamValue` съела закрывающую скобку вызова. clangd показал 0 диагностик —
поймал только MSVC. Первая починка добавила скобку не туда; верная — `))` в
первом вызове и `) {` в остальных трёх. **Вывод: скобки после regex-правок
проверять по сборке; clangd по compile DB их не ловит.**

**Проверки:** clang-format на 3 файла; AC25 `BuildAddOn.py` — `Build succeeded!`;
sweep AC26-29 — все `success`; `restart_archicad_for_test.ps1` exit_code=0
(build+load); тесты через VS `debugger_launch` (сессия 13:58) — **22 набора,
1868 проверок `ok` ровно как до R5.3**, 1 предсуществующая ошибка
(`ConvertToParamValue`), `TestSpecGetParamValue` 43 / `TestSpecValueEdges` 54 /
`TestSpecGrouping` 43; A/B `r53-ok` — C=2/M=12/D=2, 14 строк, `diff_rows` против
P0 = 0 и против `r52-ok` = 0, sha256 фикстуры `db1690f…` (совпал).
Эквивалентность переноса — мультимножеством: ушли только имена трёх параметров и
их типы, ни одного имени ACAPI-вызова в диффе нет.

## R5.4 — ЗАКРЫТ (read-only доступ к значениям, видимость вынесена)

**Что сделано.** Введены `Spec::SpecValueReader` (держит `const SpecReadContext &`
+ один метод `Read (elemguid, rawname, pvalue, n_layer) const`, тело прежней
`GetParamValue` перенесено дословно) и `IsSourceVisible (elemguid, onlyVisible)`.
Экземпляр reader один на всё правило, создаётся до цикла по элементам.

**`fromMaterial` не перенесён — он не использовался в теле вообще.** Признак
материала берётся из прочитанного `pvalue`, а не из аргумента (проверено
построчным чтением тела). Оставить его в новом контракте означало бы зафиксировать
параметр, который ничего не делает. Сигнатура сократилась с 6 аргументов до 4.

**Про «без повторного ACAPI-чтения из методов строки»: повторного чтения из
методов элемента строки в коде не было и нет.** R5.4 делает это свойство
структурным: владелец контекста один, читателей много, писателей нет (все
методы const).

**Эквивалентность выноса видимости доказана чтением, а не «похоже на то же»:**
внутри вычислительного цикла (1733-1856) нет ни одного вызова ACAPI и ни одного
присваивания `rule.elements` — список заполняется только при разборе (строки 118
и 1211), модель в цикле не меняется. Значит видимость за время прохода измениться
не может, и «с тем же моментом чтения» выполнено буквально.

**Ошибка в моём тесте, пойманная прогоном.** Первая версия сверяла второй reader с
предыдущим результатом `result`, а там лежал отказ чтения list-data (пустая
строка). Исправлено сравнением двух чтений ОДНОГО поля, 21/21. **Вывод: в тестах
сравнивать два наблюдения одного актуального состояния, а не «последний
результат».**

**Регрессия сборки:** `GS::HashTable` и `ParamValueData` без `operator==`
(C2678), ключ итератора pre-AC28 — указатель (C2664); clangd оба раза показал
0 ошибок, поймал MSVC.

**Проверки:** clang-format на 3 файла; AC25 — `Build succeeded!`; sweep AC26-29 —
все `success`; `restart_archicad_for_test.ps1` exit_code=0; тесты через VS
`debugger_launch` (сессия 14:16) — **23 набора, 1889 проверок `ok`**, 1
предсуществующая ошибка `ConvertToParamValue`, новый `TestSpecValueReader` 21/21,
прежние наборы без изменений; A/B `r54-final` — C=2/M=12/D=2, 14 строк,
`diff_rows` = 0 против `p0-smoke`, `r52-ok`, `r53-ok`, `r54-ok`; sha256 фикстуры
`db1690f…` (совпал). Эквивалентность переноса — мультимножеством плюс счётчик
ACAPI-вызовов: `ACAPI_Element_Filter` ровно 1 в обоих, всего 99 вызовов ACAPI.

## R5.5 — ЗАКРЫТ (граница выделения чтения, оценка S10–S16). R5 завершён

**Шаг нерефакторинговый: production-код не менялся ни одной строкой.** Прод-файлы
не тронуты; изменены только `tests/TestSpec.cpp` (новый набор), `tests/TestFunc.hpp`
(объявление) и `tests/TestFunc.cpp` (строка реестра) — по раскладке #231.

**Учтена новая система тестов (#231):** TestKit, файловый отчёт
`%TEMP%\somestuff_test_report.txt` с построчным сбросом, `BEGIN`/`END <имя>
passed=N failed=M`, `SUMMARY`, `FAILED_SUITE`, код возврата 70 при failed>0.
Раннер САМ читает отчёт и ставит exit code — панель «Отладка» VS больше не
источник результата, чтение через неё устарело.

**Главный критерий шага — исходные словари до/после чтения.** Пять разных чтений
не меняют ни размеры, ни значения `read`/`composite`/`listData`; не тронут
соседний элемент; выходное поле не «потребляется». Формулировка про словари, а не
про значения: формулы пересчитываются, идемпотентность по значению была бы
неверным требованием.

**Добавлено покрытие S13 (частично), S14, S16.** S10/S11/S12/S15 были покрыты
уже. Граница S13 принципиальна: разбор `@arr` живёт в Helpers (`Helpers.cpp:6220`),
читает через ACAPI, R5 его трогать запрещает — закреплена только семантика
первого ряда, видимая вычислителю.

**Три контракта, найденные прогоном, а не предположением** (каждая была ошибкой
теста в первой версии, исправлены по коду):
1. `GS::HashTable::Get` на отсутствующем ключе БРОСАЕТ — элемент заводится через
   фикстуру до `Get().Put()`.
2. В `uniStringValue` формулы лежит ВЫРАЖЕНИЕ (`{@listdata:elem.naen}<>`), а не
   имя с префиксом `{@formula:` — префикс в ключе словаря.
3. `fromGDLArray` виден вычислителю только если `Read` дошёл до материальной
   ветки (валидное значение + `fromMaterial` + нет состава). На обычном отказе
   `pvalue` не заполняется, и ветка `is_error = !fromMaterial` даёт обычную
   ошибку. Закреплено и обратное: снятый `fromMaterial` даёт УСПЕШНОЕ чтение.

**Проверки:** clang-format на 3 файла; AC25 — `Build succeeded!`; sweep AC26-29 —
все `success`; раннер exit_code=70 `cxx_tests_failed_count_1` — корректно, ошибка
предсуществующая и не моя. Отчёт: `SUMMARY suites=50 passed=1874 failed=1`,
`TestSpecReadBoundary` 49/49, `FAILED_SUITE
TestConvertPropertyToParamValue` (вне области). A/B `r55-final` — C=2/M=12/D=2,
14 строк, `diff_rows` = 0 против `p0-smoke`, `r52-ok`, `r53-ok`, `r54-final`;
sha256 фикстуры `db1690f…` (совпал).

## R6.1 — ЗАКРЫТ (расчётная часть вынесена в PlanRuleRows)

**Что сделано.** Всё до ветки `delete_old` перенесено из `GetElementsForRule` в
`PlanRuleRows`. Контейнеры прежние (`elements`, `out_param`, `not_found_*`,
`n_elements`, `error_element`), ключи и порядок не тронуты. Единственное изменение
— точка возврата: расчёт стал отдельной функцией, и вызывающая решает, идти ли в
сверку. Это и даёт выход блока R6: рассчитанные строки проверяются БЕЗ модели.

**`reader` и `fstr` остались нужны обеим частям** — расчёт и сверка читают тем же
reader и печатают тем же форматом `.2m`. После выноса в вызывающей свои
объявления. R7 сведёт их вместе со сверкой.

**Две ошибки, пойманные сборкой, обе — в моём выносе:**
1. Локальное объявление `out_param` затеняло параметр и молча обнуляло словарь на
   каждый вызов. Удалено.
2. **Потерян возврат `n_elements`** — MSVC C4715 «значение возвращается не при
   всех путях». Прежде возврат стоял перед `if (!rule.delete_old)`; без него число
   созданных строк пропадало бы на пути без отказов. Восстановлен.
   **Вывод: при выносе части функции проверять ВСЕ её `return`, а не последний;
   clangd по compile DB такие вещи не показывает.**

**Эквивалентность — мультимножеством: ни один идентификатор не ушёл из HEAD**
(`only in HEAD: {}`), пришли только имена новой функции и вторые объявления
reader/fstr; частота ACAPI-вызовов 99 в обоих без изменений.

**Тест `TestSpecPlanning` — 33 проверки.** Главная из них: расчёт и полный путь
дают одинаковый счёт, ключ, число слотов и значения. Ещё: `delete_old = true` не
добавляет ни строки ни модификации (сверки в расчётной части нет вовсе);
суммирование 2+3=5 за две строки в одну; отказ правила обнуляет расчёт и помечает
элемент; невидимый источник отбрасывается в расчётной части.

**Проверки:** clang-format на 4 файла; AC25 — `Build succeeded!`; sweep AC26-29 —
все `success`; по схеме #231 `suites=51 passed=1907 failed=1` (предсуществующая
`TestConvertPropertyToParamValue`), новый набор 33/33, раннер exit_code=70
корректно; A/B `r61-final` — C=2/M=12/D=2, 14 строк, `diff_rows` = 0 против
`p0-smoke`, `r53-ok`, `r54-final`, `r55-final`; sha256 фикстуры `db1690f…`
(совпал).

## R6.2 — ЗАКРЫТ (вклад источника: RuleContribution, SpecPlanning)

**Что сделано.** Разделены две сущности, жившие в одном типе: `Spec::Element` —
итоговая строка (агрегат по ключу), `RuleContribution` — данные ОДНОГО источника
за один проход цикла. Новый модуль `spec/SpecPlanning.hpp/.cpp`, карточка
`Docs/modules/spec/SpecPlanning.md`.

**Две фазы — требование исходного цикла, а не удобство.** `ContainsKey (key)`
стоит ПОСЛЕ чтения сумм, а ключ известен только с конца чтения уникальных
параметров; выходные слоты читаются только у первого представителя ключа. Слияние
фаз заставило бы читать выход у каждого источника и заносить неполный выход
второго в отчёт как ошибку, хотя раньше он молча игнорировался.
`BuildContribution` (фаза 1) + `ReadContributionOutputs` (фаза 2).

**Главный критерий шага — число чтений.** HEAD: 8 `reader.Read`; после выноса:
4 в `PlanRuleRows` (чтения сверки, не тронуты) + 4 в `SpecPlanning.cpp`
(42/59/92/125). Итого 8 = 8, порядок сохранён (флаг -> уникальные -> суммы ->
выход). Частота ACAPI без изменений.

**Земля — `MissingField {rawname, isError}` вместо общего `bool hasReadError`.**
Отчёт различает отказ «из состава» (ошибка) и отказ в строке >1 GDL-массива
(тихо); поле-источник нужно знать, чтобы пометить элемент и не повторить
сообщение.

**СОБСТВЕННАЯ РЕГРЕССИЯ, пойманная прогоном и исправленная.** В `ReadIsError`
инвертирован приоритет: `fromMaterial` возвращал true до проверки GDL, тогда как
в исходном цикле `fromGDLArray` ПЕРЕКРЫВАЕТ `fromMaterial`. Последствие: отказ в
строке >1 стал ошибкой элемента — упал `TestSpecReadBoundary` «S13 second row not
an error» (проходил 49/49 на R5.5 и R6.1). **Урок: при выносе блока с тернарной
логикой переносить ПОРЯДОК проверок, а не только их набор; прогон существующих
наборов это ловит, чтение кода — нет.**

**Вторая ошибка — в моём тесте:** при переходе на `ConvertBoolToParamValue`
потерял строку `Put` (поле не попадало в словарь) и не задал `rawName` ДО вызова
(конвертер сам составил бы `{@gdl:...}`). Обе — по коду фикстуры `Text`/`Number`.

**`TestSpecContribution` — 51 проверка, работает БЕЗ `ElementDict` и без
`GetElementsForRule`** — это и доказывает пользу разделения. Покрыто: обе фазы;
слот «1»; выключенный флаг; непрочитанный уникальный параметр (ключ всё равно
склеен с `@`); снятие ошибки при `fromMaterial`; неполный выход (`Partial`,
пустой `keyOut`); короткий набор слотов против схемы; два вклада одного ключа.

**Проверки:** clang-format на 5 файлов; AC25 — `Build succeeded!`; sweep
AC26-29 — все `success`; схема #231: `suites=52 passed=1958 failed=1`
(предсуществующая `TestConvertPropertyToParamValue`), новый набор 51/51,
`TestSpecReadBoundary` 49/49 после починки регрессии, `TestSpecPlanning` 33/33,
раннер exit_code=70 корректно; A/B `r62-final` — C=2/M=12/D=2, 14 строк,
`diff_rows` = 0 против `p0-smoke`, `r54-final`, `r55-final`, `r61-final`;
sha256 фикстуры `db1690f…` (совпал).

## Находка R6.3 — кандидат в F (НЕ выполнено, требует согласования)

Суммирование неполных массивов в `SumContributionIntoRow` ведёт себя так, как
работал исходный цикл, и это зафиксировано тестом как поведение, а не исправлено:

- складываются только первые `nsumm = MIN(длин)` слотов; слоты сверх `nsumm` не
  трогаются вообще — вклад может молча потерять часть суммы;
- слот складывается только если `isValid` у обеих сторон, иначе остаётся как
  есть (в том числе остаётся невалидным) — то есть невалидный слот не даёт
  знать, что сумма по этому полю недостоверна;
- размеры массивов строки и вклада никогда не выравниваются: строка может
  остаться с меньшим числом слотов, чем объявлено схемой.

**Почему это не «просто баг», а вопрос политики.** Отсутствие данных по одному
источнику сейчас превращается в молчаливо неполную сумму, а неполная сумма
потом уходит в создание/обновление элемента. Менять это — значит менять смысл
пустого результата, а это отдельное согласование.

**Куда это отнесено.** Ближайший по смыслу пункт плана — **F1** («сделать
неполный расчёт безопасным для старых объектов»). F1 требует сначала
воспроизвести конкретный случай и получить согласованную политику для
v1/v2/v3; ни то, ни другое на R6.3 не сделано и не делается здесь. R6.3 лишь
зафиксировал наблюдение, чтобы оно не потерялось при следующих шагах.

**Что для этого понадобится (не сделано):** воспроизводимый сценарий на данных
из ACAPI (недоступный источник в середине группы), issue на GitHub, RED-тест,
согласование политики. На синтетическом стенде воспроизводится только механика
(`nsumm`, `isValid`, длины), а не пользовательский ущерб.

## R6.3 — ЗАКРЫТ (раскладка вклада в строку: суммирование + создание/слияние)

**Что сделано.** Две проверяемые единицы вместо встроенных в цикл операций:
`SumContributionIntoRow` (сложение вклада в существующую строку — тестируется
напрямую, без словаря и без модели) и `AddContributionToRow` (создание или
слияние, возвращает `RowAddition`: `Created` / `Merged` / `SchemaMismatch`).
Решение по `SchemaMismatch` осталось в `PlanRuleRows` — это политика, не
раскладка.

**Поведение неполных массивов СОХРАНЕНО, как требует план.** `nsumm` = MIN(длин),
слоты сверх `nsumm` не тронуты, слот складывается только при `isValid` с обеих
сторон, размеры не выравниваются. Перенос дословно — все три строки найдены в
`SpecPlanning.cpp` буквально. Строгий конструктор был бы улучшением, меняющим
результат. **Наблюдение зафиксировано как кандидат в F1 (см. раздел выше);
F1 не выполнялось.**

**Порядок операций — единственное, что здесь легко сломать, и оно сохранено:**
при `Created` запись `outParam[keyOut] = key` идёт ДО проверки схемы (ключ
попадает в словарь даже для отброшенной строки); при `Merged` источник
дописывается ДО суммирования; выходные слоты берутся как есть.

**Число чтений не изменилось:** 4 в `Spec.cpp` + 4 в `SpecPlanning.cpp` = 8, как
на R6.2. ACAPI-вызовы без изменений.

**`TestSpecRowLayout` — 54 проверки.** Главное: **раскладка даёт тот же словарь
строк, что и полный путь** (трёх источников, две строки, сверены размеры,
слоты, источники, признаки, значения сумм). Ещё: вклад короче строки и строка
короче вклада; `isValid` с обеих сторон; `Created` переносит все шесть признаков
правила; `Merged` даёт 2+3=5 и оба источника; `SchemaMismatch` не добавляет
строку, но ключ в `outParam` уже записан.

**Две ошибки — в моих ожиданиях теста, не в проде:** невалидный слот создан
вызовом `Num (5)` (по умолчанию `valid = true`), и `outParam` ожидался одним
элементом, хотя ключ выхода разный у строк с разными значениями выхода
(`@Alpha` и `@Beta`) — верно «одна запись на ключ строки», а не на строку.

**Локальную функцию-хелпер в теле набора объявить нельзя** (C2870/C2601) —
вынесена в общий анонимный namespace; сначала попала внутрь `SpecFixture`,
откуда набор её не видел.

**Проверки:** clang-format на 5 файлов; AC25 — `Build succeeded!` (первая попытка
LNK1168 — .apx держал Archicad); sweep AC26-29 — все `success`; схема #231:
`suites=53 passed=2012 failed=1` (предсуществующая
`TestConvertPropertyToParamValue`), новый набор 54/54, `TestSpecContribution`
51/51, `TestSpecReadBoundary` 49/49, `TestSpecPlanning` 33/33, раннер exit_code=70
корректно; A/B `r63-final` — C=2/M=12/D=2, 14 строк, `diff_rows` = 0 против
`p0-smoke`, `r55-final`, `r61-final`, `r62-final`; sha256 `db1690f…` (совпал).

## R6.4 — ЗАКРЫТ (общая выходная схема: слот держит имя и значение вместе)

**Что сделано.** `OutputSlot {rawname, value, isSum}` + `Element::out_slots`.
До этого слот существовал как ДВЕ параллельные вещи: имя в `out_paramrawname[k]`
и значение в `out_param[k]`; инвариант «длины равны» держался только на
`OutSlotsMatchSchema`, то есть неявно, а индексация была размазана по шести
местам. Теперь имя и значение лежат рядом.

**Индексная рассылка убрана полностью** — `el.out_param[k]`, `el.out_sum_param[k]`
и `elvalue = el.*` в коде не осталось (проверено поиском: 0 вхождений), вместо них
шесть проходов `for (const OutputSlot &slot : el.out_slots)`. Сверка (R7)
переведена на чтение имён ИЗ СХЕМЫ СТРОКИ, а не из `rule`: раньше индекс шёл по
массиву правила и читал из массива строки, совпадение размеров было условием
безопасности доступа; теперь это невозможно по построению.

**Производительность (критерий выхода R6):** схема копируется ОДИН раз на строку
при создании, как раньше копировались два массива имён. Lookup на ячейку не
добавлен — вместо обращения к имени по индексу прямой проход по слотам, то есть
операций меньше. Значения слотов суммы обновляются в ТОМ ЖЕ цикле
суммирования, без новых проходов и аллокаций.

**Прежние поля не удалены** (`out_param`, `out_sum_param`, `out_paramrawname`,
`out_sum_paramrawname`) — это двойная правда до полного перевода потребителей.
Accessor-ы `OutParam(i)` / `OutSlotName(i)` / `OutSlotCount()` / `TryGetOutSlot()`
возвращают прежнее поведение, поэтому существующий код компилируется и даёт тот
же результат.

**`TestSpecRowSlots` — 28 проверок, 0 ошибок.** Схема повторяет прежние порядок,
имена и значения и совпадает с accessor-ами по каждому индексу; индекс вне
схемы даёт `false`, а не выход за границу; **после слияния значения сумм в
схеме актуальны** (главная проверка: 2+3=5 видно и в `out_sum_param`, и в
`out_slots[1]`); выходной слот первого представителя при слиянии не меняется;
схема не строится для отброшенной строки; размер схемы стабилен при трёх
слияниях.

**Две ошибки в моей работе, обе пойманы инструментами:**
1. **Имя набора столкнулось с существующим.** `TestSpecOutputSchema` уже был —
   это набор разбора схемы правила (не мой). Компилятор дал C2084. Набор
   переименован в `TestSpecRowSlots`. **Вывод: перед добавлением набора
   проверять, что имя свободно, а не только что объявление добавлено.**
2. **Откат `git checkout` снёс 1200 строк файла.** Я удалял дубликат диапазоном
   по номеру строки, посчитанному по НЕВЕРНОМУ индексу, и снёс блок с конца
   файла. Восстановлено через `git checkout` (незакоммиченных изменений в этом
   файле на тот момент не было — R6.3 уже закоммичен), набор вставлен заново.
   **Вывод: удаление диапазона строк в файле, который уже правится, — не
   обратимая операция; сначала `git stash`/`git checkout -- <file>` проверка.**
   Побочно: после отката пришлось удалить устаревший `TestSpec.obj`, иначе
   LNK1163 «недопустимый выбор для секции COMDAT» — линкер читал старый объект.

**Проверки:** clang-format на 5 файлов; AC25 — `Build succeeded!`; sweep
AC26-29 — все `success`; схема #231: `suites=55 passed=2089 failed=1`
(предсуществующая `TestConvertPropertyToParamValue`), новый набор 28/28,
`TestSpecRowLayout` 54/54, `TestSpecContribution` 51/51, `TestSpecReadBoundary`
49/49, `TestSpecPlanning` 33/33, раннер exit_code=70 корректно; A/B `r64-final` —
C=2/M=12/D=2, 14 строк, `diff_rows` = 0 против пяти эталонов; sha256 `db1690f…`
(совпал).

**Не покрыто `not verified`:** схема на данных из ACAPI и при нескольких группах;
производительность на большой модели не измерена (критерий выхода проверен
по коду — новых lookup/аллокаций на ячейку нет); macOS и AC22-24 не собирались.

## R6.5 — ЗАКРЫТ (равносильность нового движка и прежней арифметики)

**Что сделано.** В тестовой единице построен ЭТАЛОН «old» — независимая
реализация прежней арифметики (`namespace legacy`: `SumInto` и `PlanRow`,
написанные максимально просто, без единого вызова нового модуля). Production его
не видит: вызовов из `Sources/AddOn/spec/` нет, в production-сборку `tests/` не
попадает. Это ровно то, чего требует план: «полные двойные снимки допустимы
только в отдельном regression-режиме; production выполняет один движок».

**Эталон дочитывает выходные слоты сам** — в ветке «ключа ещё нет», как в
прежнем цикле. Если этого не сделать, сравнивались бы два разных входа, а не
два разных движка.

**`TestSpecEngineEquivalence` — 49 проверок, 0 ошибок.** Один ключ / три
источника; два ключа с перемешанными источниками (порядок первого представителя);
неполные массивы в обе стороны; `isValid` с обеих сторон. Сравниваются: число
строк, размер `outParam`, размеры слотов, значения сумм, **порядок источников
поэлементно**, выходные значения.

**Развилка расхождений: 12 -> 5 -> 0.** Показываю, потому что это и есть
ценность сравнения. Первая ошибка была МОЯ и не в движках: `collect` вызывал
фазу 2, а цикл сравнения вызывал её повторно — второй выход дописывался вторым
слотом (2 вместо 1), а у второго представителя ключа выходов не было вовсе.
Вместо правки наугад поставил диагностику с размерами вклада до/после фазы 2 и
числом строк; она показала, что расхождение симметрично с обеих сторон, то есть
движки равны, а ломается сборка входа. **Вывод: при расхождении «A против B»
сначала проверять вход, а не подозревать оба движка.**

**Сбой пакетного sweep, не связанный с кодом.** `-v 26 27 28 29` падал на
разных версиях по очереди, хотя каждая по отдельности собиралась. Причина —
Archicad держит `.apx` предыдущей версии между шагами пакета; после
taskkill и повторного запуска пакет проходит целиком:
`AI_BUILD_RESULT status=success ... versions=26,27,28,29`. **Вывод: пакетный
sweep после runtime-прогона требует закрытия Archicad.**

**Проверки:** clang-format на 3 файла; AC25 — `Build succeeded!`; sweep AC26-29 —
все `success`; схема #231: `suites=56 passed=2138 failed=1` (предсуществующая
`TestConvertPropertyToParamValue`), новый набор 49/49, все предыдущие spec-наборы
без изменений, раннер exit_code=70 корректно; A/B `r65-final` — C=2/M=12/D=2,
14 строк, `diff_rows` = 0 против пяти эталонов; sha256 `db1690f…` (совпал).

## R6.6 — ЗАКРЫТ (сценарии S08-S15/S19/S25-S28 сведены к общему виду)

**Что сделано.** `TestSpecScenarioMatrix` (75 проверок) закрывает сценарии, которых
не было в покрытии, и приводит сравнение к общему виду: число строк, ключи,
значения, суммы, provenance.

- **S08 — несколько ГРУПП с одинаковым ключом.** Прежние наборы различали
  источники, здесь различаются группы: 2 источника × 2 группы дают одну строку,
  сумму 2×(5+7)=24, четыре записи в provenance. Первый представитель задаётся
  первым вкладом в порядке обхода; схема строится один раз.
- **S09 — дубликаты выходных значений и разделители ключа.** Закреплено СТАРОЕ
  поведение: две строки с разными уникальными ключами, но одинаковым выходным
  значением дают одну запись в `outParam` (вторая строка теряется при поиске), а
  `@` внутри значения уникального параметра не экранируется. Это задокументированная
  коллизия — кандидат в отдельный F; кодировку ключа план запрещает менять здесь.
- **S19 — provenance.** Источники в порядке обхода, признаки правила
  (`subguid_*`, `favorite_name`) из первого представителя; сумма по всем источникам
  и в `out_sum_param`, и в схеме.
- **S25** — малая модель: результат и минимальный размер схемы.
- **S26 — признак отсутствия лишнего чтения, а не таймер.** 40 источников с
  повторяющимся ключом: строка одна, сумма 40, provenance хранит все источники,
  размер схемы не растёт, и **фаза выходных слотов выполнена ровно один раз**
  независимо от числа источников. Таймеры в тестах нестабильны, поэтому вместо
  времени считается число проходов.
- **S27 — пик строк.** 25 уникальных ключей × 9 слотов (6 выходных + 3 суммы):
  25 строк, схема покрывает все слоты и корректно различает `isSum` по позиции.
  `outParam` схлопывается в одну запись — по той же причине, что в S09 (выходные
  значения у всех строк одинаковые).
- **S28 — отсутствие квадратичности.** 30 строк: на каждый источник ровно одно
  обращение `ContainsKey` + одно `Get`, стоимость не зависит от числа уже
  собранных строк.
- **Канонизация применена только к сравниваемым копиям:** ключи строк сортируются
  в отдельном списке, словарь не трогается. Порядок источников внутри строки
  сортируется НИКОГДА — это provenance, влияющий на размещение.

**Две ошибки — мои ожидания, не прод-код.** `outParam` ожидался по числу строк, но
в S27 и S28 выходные значения у всех строк одинаковые, поэтому словарь держит
одну запись на уникальный выход. Это то же поведение, что закреплено в S09 выше в
том же наборе; исправлено по коду.

**Проверки:** clang-format на 3 файла; AC25 — `Build succeeded!`; sweep AC26-29 —
`status=success`; схема #231: `suites=57 passed=2213 failed=1`, новый набор 75/75,
`TestSpecEngineEquivalence` 49/49, `TestSpecRowSlots` 28/28, `TestSpecRowLayout`
54/54, `TestSpecContribution` 51/51, `TestSpecReadBoundary` 49/49,
`TestSpecPlanning` 33/33, `FAILED_SUITE TestConvertPropertyToParamValue` —
предсуществующая ошибка вне области; раннер exit_code=70 корректно. A/B `r66-final` —
C=2/M=12/D=2, 14 строк, `diff_rows` = 0 против пяти эталонов; sha256 `db1690f…`
(совпал).

**Окружение между шагами:** шесть подряд зависших стартов Archicad (процесс жив,
`MainWindowTitle` пуст, порт 19723 молчит) при очищенных `ARCHICAD__*` — тот же
симптом, что на R5.2; помогла чистка вручную. Прогон выполнен на уже работающем
Archicad с `test_25`.

## R7.1 — ЗАКРЫТ (fixtures для сверки существующих строк)

**Что сделано.** `TestSpecReconcileFixtures` — 37 проверок на пять случаев, которые
план R7.1 требует зафиксировать ДО замены представления на create/update/delete/
unchanged (это делает R7.2). Зафиксировано ТЕКУЩЕЕ поведение, включая три его
особенности, которые выглядят подозрительно.

- **`key_out` и формат `.2m`.** Ключ выхода склеивается из значений полей ВЫХОДА
  правила (не сумм) по ATSIGN. Формат `.2m`: точка в строке формата — РАЗДЕЛИТЕЛЬ
  единицы измерения, а не десятичный (Helpers.cpp:201-207 точки временно убираются
  перед разбором), поэтому `.2m` тождественно `2m` — две знаковые цифры с обрезкой
  нулей: 2.5 -> "2,5", 7.0 -> "7". **Первая версия фикстуры ошибалась:** я ожидал
  "2,50"/"7,00", то есть принял точку за десятичный разделитель.
- **ГРАБЛЯ — нечисловой суффикс формата молча становится нулём разрядов.**
  `n_zero = std::atoi (outstringformat)` без проверки: "F2" -> n_zero 0 ->
  округление до целого (2.5 -> "3"), без ошибки. Закреплено, потому что от
  разрядности зависит `key_out`, а значит и сопоставление существующих объектов:
  смена формата молча переставит ключи. Кандидат в отдельный F.
- **Поиск ПЕРВОГО совпадения.** `out_param` хранит первый ключ для данного
  выходного значения, запись не перезаписывается. Поэтому при двух строках с
  одинаковым выходом сверка находит только первую, а вторая остаётся в `elements`
  как будто новая. Это тот же механизм потери строки, что в S09 (R6.6).
- **Дубли старых объектов.** Механизм по коду: первый дубль находит строку и
  ПОМЕЧАЕТСЯ ОБРАБОТАННЫМ (`guids.Add (elemguid, true)`), но в `elements_delete` НЕ
  попадает — он сопоставлен, а не удалён; строка при этом УДАЛЯЕТСЯ из `elements`.
  Второй дубль с тем же `key_out` приходит по ветке
  «`!elements.ContainsKey (key)`» и удаляется. **Первая версия фикстуры ошибалась:**
  я ожидал удаления двух объектов, приняв сопоставленный объект за удалённый.
- **Кандидат уже удалён из словаря новых.** Тот же механизм, что в дублях: строка
  захвачена первым старым объектом, второй приходит по ветке израсходованной
  строки. Удаляется ровно один объект — второй.
- **no-op.** Значения старого объекта равны новым: `flag_change` не поднимается,
  объект не модифицируется, но строка УДАЛЯЕТСЯ из `elements` — повторный запуск
  ничего не создаёт и не меняет. Это текущий контракт no-op; статус «unchanged»
  его не должен подменять.
- **Изменение суммы при неизменном выходе** -> `flag_change`, объект уходит в
  modified с сохранением `exs_guid`; ничего не создаётся и не удаляется.
- **GUID-связь читается, но результат не используется.** Строки со сборкой
  `instring` из GUID источников, которая НИКУДА не пишется; при отсутствии поля
  меняется только флаг. Подозрительно (вероятно, недописанная связь), но менять
  поведение здесь нельзя — зафиксировано как есть.

**Проверки:** clang-format; AC25 — `Build succeeded!`; sweep AC26-29 —
`status=success`; схема #231: `suites=58 passed=2250 failed=1`, новый набор
`TestSpecReconcileFixtures` 37/37, `TestSpecReconcile` 57/57, `TestSpecRowSlots`
28/28, `TestSpecRowLayout` 54/54, `TestSpecScenarioMatrix` 75/75,
`TestSpecEngineEquivalence` 49/49, `TestSpecContribution` 51/51,
`TestSpecReadBoundary` 49/49, `TestSpecPlanning` 33/33,
`TestSpecOutputSchema` 49/49, `TestSpecValueEdges` 54/54, `TestSpecOutSlots` 9/9 — все без
провалов;
`FAILED_SUITE TestConvertPropertyToParamValue` — предсуществующая ошибка вне
области (TestParam.cpp:263, отрицательное doubleValue); раннер exit_code=70
корректно. A/B `r71-final` — C=2/M=12/D=2, 14 строк, `diff_rows` = 0 против
`p0-smoke`/`r65-final`/`r66-final`; sha256 `db1690f…` совпал.

**Окружение:** пользователь доработал тестовую инфраструктуру (`TestKit::Note`
вместо `DBprnt`, #231, коммиты `f0ddbe6`/`a139b88`) — учтено, мои правки не
пересекались. Archicad пришлось закрывать через `taskkill /T /F` (обычный не
срабатывал). **Правило пользователя:** профили
`%LOCALAPPDATA%\GRAPHISOFT\ARCHICAD__*` не чистить самостоятельно — среди них
могут быть нужные проекты; записано в `AGENTS.md` §9 и в память.

## R7.2 — ЗАКРЫТ (сверка существующих строк вынесена из GetElementsForRule)

**Что сделано.** Тело сверки (бывшие строки 1922-2051 `Spec.cpp`, 130 строк)
перенесено в `ReconcileExistingRows` в `SpecPlanning.cpp`. Представление
create/update/delete/unchanged **ещё НЕ введено** — это R7.4/R7.3; здесь только
перенос, как требует план («только после сравнения заменить представление»).

**Что сохранено (это и было целью шага):**
- **Порядок операций целиком:** обход `exsist_elements` -> сборка `key_out` из полей
  ВЫХОДА -> три ветви удаления по порядку (`!hasunic` -> `!out_param.ContainsKey` ->
  `!elements.ContainsKey`, каждая пишет в `guids` false) -> сверка значений
  (несуммарные слоты, затем суммарные, в порядке схемы, затем непрочитанное
  GUID-поле) -> `elements_mod.Add` при `flag_change` -> `elements.Delete (key)` и
  `guids.Add (elemguid, true)` -> второй обход с удалением всего, чего нет в `guids`.
- **Число чтений: 8** (4 в сверке + 4 в расчёте) — было и осталось; `Spec.cpp`
  теперь содержит 0 `reader.Read`, `SpecPlanning.cpp` — 8.
- **Тексты `msg_rep` без изменений** (все 9), потому что они разбираются отчётом.
  Внутри перенесённой функции вызовы свёрнуты в `report (...)` — лямбда с ТЕМ ЖЕ
  `msg_rep ("Spec", text, NoError, APINULLGuid)`; это косметика, не изменение.
- **Имена параметров** переименованы по стилю нового модуля: `out_param` -> `outParam`,
  `elements_delete` -> `elementsDelete`, `elements_mod` -> `elementsMod`. Тексты
  сообщений с упоминанием `out_param` оставлены как есть — это данные отчёта.

**Проверки:** clang-format на 3 файла; AC25 — `Build succeeded!`; sweep AC26-29 —
`status=success`; схема #231: `suites=58 passed=2250 failed=1` — **числа ровно те
же, что до переноса**, что и есть главное доказательство: `TestSpecReconcile` 57/57,
`TestSpecReconcileFixtures` 37/37, `TestSpecScenarioMatrix` 75/75,
`TestSpecEngineEquivalence` 49/49, `TestSpecRowSlots` 28/28, `TestSpecRowLayout` 54/54,
`TestSpecContribution` 51/51, `TestSpecReadBoundary` 49/49, `TestSpecPlanning` 33/33;
`FAILED_SUITE TestConvertPropertyToParamValue` — предсуществующая вне области;
раннер exit_code=70 корректно. A/B `r72-final` — C=2/M=12/D=2, 14 строк,
`diff_rows` = 0 против `p0-smoke`/`r66-final`/`r71-final`; sha256 `db1690f…` совпал.

**Две ошибки компиляции — мои:** обращения к `out_param` внутри перенесённого тела
(параметр новой функции называется `outParam`); текст сообщения `out_param.
ContainsKey (key_out)` оставлен без изменений намеренно.

## R7.3 — ЗАКРЫТ (SpecChangePlan хранит происхождение решения)

**Что сделано.** `SpecChangePlan` в `Spec.hpp` (не в `SpecPlanning.hpp`: тот включает
`Spec.hpp`, наоборот получился бы цикл) + `TestSpecChangePlan` — 30 проверок.

- **План НАБЛЮДАЕТ решение, а не становится вторым источником истины.** Заполняется
  из тех же точек, что пишут `elementsMod`/`elementsDelete`, поэтому расхождение
  плана с фактическими списками невозможно by construction. `Matches` — страховка
  от будущей правки, а не проверка согласованности «двух источников».
- **Про «дорогой текст на корректную строку»: выполнять было нечего.** Все девять
  вызовов `report` в сверке стоят ВНУТРИ ветвей отклонения, так что на совпавшей
  строке не строится ни одного текста. Проверено по коду до внесения изменений.
- **Происхождение решения хранится ДЁШЕВО:** `DeleteReason` —
  `NoUniqueFields` / `NoNewRow` / `RowAlreadyClaimed` / `Obsolete`, пара
  `{guid, причина}` в одном массиве `removals`. Раздельные массивы GUID и причин
  были бы «постоянным дубликатом» — их рассинхронизация при правке любой ветви
  сделала бы причину недостоверной.
- **`unchanged` — счётчик, а не копия строки.** Копия всех свойств ради строки,
  которую не трогаем, ровно та лишняя копия, которую запрещает R7.4. Это решение
  R7.4, сделанное в R7.3, потому что план — его первый потребитель.
- **`deleteOld` по умолчанию 0**, а не 1: незаполненный план не должен утверждать,
  что отражает решение. Ставится 1 только после раннего выхода `!rule.delete_old`,
  то есть действительно в точке, где сверка прошла.
- **План опционален:** `GetElementsForRule` и `ReconcileExistingRows` принимают
  `SpecChangePlan *plan = nullptr`; рабочий путь его не строит.

**Две ошибки — мои ожидания:**
1. **Причина удаления — `NoNewRow`, а не `Obsolete`.** Объект с выходным
   значением, которого нет среди новых строк, отсекается ВТОРОЙ ветвью сверки, до
   сопоставления. `Obsolete` (второй обход) при обычном ходе недостижим: объект
   попадает в `guids` либо удаляется одной из трёх ранних ветвей. Значение оставлено
   в перечислении как явный «второй обход», отдельной проверки на него нет.
2. **`deleteOld` по умолчанию 1 было смысловой ошибкой** в самой структуре —
   исправлено на 0 вместе с проверкой на пропуск сверки.

**Служебное:** имя поля `delete` — зарезервированное слово C++, отсюда `removals`.
Итерация по `GS::HashTable` — в стиле проекта: `for (auto &cIt : dict)` с
`cIt.key`/`cIt.value` под `#ifdef ServerMainVers_2800` (моё `pair.first` не
компилируется: у `CurrentPair` нет таких членов).

**Проверки:** clang-format на 4 файла; AC25 — `Build succeeded!`; sweep AC26-29 —
`status=success`; схема #231: `suites=59 passed=2280 failed=1`, новый набор
`TestSpecChangePlan` 30/30, `TestSpecReconcileFixtures` 37/37,
`TestSpecReconcile` 57/57, `TestSpecScenarioMatrix` 75/75,
`TestSpecEngineEquivalence` 49/49, `TestSpecRowSlots` 28/28, `TestSpecRowLayout`
54/54, `TestSpecContribution` 51/51, `TestSpecReadBoundary` 49/49,
`TestSpecPlanning` 33/33; `FAILED_SUITE TestConvertPropertyToParamValue` —
предсуществующая вне области; раннер exit_code=70 корректно. Инвариант чтений держится:
8 в `SpecPlanning.cpp`, 0 в `Spec.cpp`. A/B `r73-final` — C=2/M=12/D=2, 14 строк,
`diff_rows` = 0 против `p0-smoke`/`r72-final`; sha256 `db1690f…` совпал.

## R7.4 — РАЗВЕДКА ПОТРЕБИТЕЛЕЙ (шаг не завершён)

**Установлено, что прежние поля строки (`out_param`, `out_sum_param`,
`out_paramrawname`, `out_sum_paramrawname`) больше не имеют НИ ОДНОГО
прод-потребителя, кроме собственного порождения.** Это ровно то состояние, при
котором их можно удалять, — но удалять их в этом шаге я не стал: объём правки
тестов (76 строк в `TestSpec.cpp`) требует отдельного захода.

**Кто читает поля строки сегодня:**
- `Spec.cpp:PlaceElements` (строки 2753, 2766) — уже по `el.out_slots`;
- `Spec.cpp` 884, 897 (модификация) — уже по `el.out_slots`;
- `SpecPlanning.cpp` (`BuildOutputSlots`, `SumContributionIntoRow`,
  `AddContributionToRow`) — **порождение**: это единственное место, где прежние
  поля ещё наполняются;
- `Spec.cpp:OutSlotsMatchSchema` (1677) — **мёртвый код**: функция не вызывается
  нигде, ни в проде, ни в тестах. Проверено полным поиском по имени.
- `tests/TestSpec.cpp` — 76 строк обращений (это контракт «двойной правы» R6.4,
  а не потребитель).

**Поля ПРАВИЛА и ГРУППЫ не трогаются** — `rule.out_paramrawname`,
`group.out_paramrawname` и т. п. живут в `SpecRule`/`GroupSpec` и нужны разбору
описания; R7.4 касается только payload строки.

**Как удалять безопасно (план следующего захода):**
1. Переписать accessor-ы `OutParam`/`OutSlotName`/`OutSlotCount` так, чтобы они
   читали `out_slots` вместо прежних массивов. Тогда семантика тестов сохраняется.
2. В `SpecPlanning.cpp` строить `out_slots` напрямую из вклада, без промежуточных
   `row.out_param`/`row.out_sum_param`; суммирование — по слотам с `isSum`.
3. Удалить четыре поля из `struct Element` и `OutSlotsMatchSchema` (мёртвый).
4. Перевести 76 мест в тестах на `out_slots`/accessor-ы.
5. Инвариант арифметики суммирования (`reader.Read` = 8) не трогать.

**Риск, который план запрещает нарушать:** `nsumm` считается по
`row.out_sum_param.GetSize ()` — длине ПРЕЖНЕГО массива. После перехода на слоты
длина схемы может отличаться от числа сложенных слотов, а это меняет поведение на
неполных массивах. Значит, условие «сложены только первые MIN(длин)» должно
оставаться в терминах вклада и накопленных СЛОТОВ, а не в терминах удаляемых
массивов.

## СМЕНА ЭТАЛОНА — фикстура test_25 очищена от созданных элементов

**Что сделал пользователь:** удалил из открытого `test_25` элементы, созданные
предыдущими прогонами, НЕ сохраняя файл. Изменение живёт только в памяти
Archicad; на диске `Test_file/test_25.pln` прежний (56 МБ, 28 сен), поэтому после
перезапуска состояние вернётся.

**Прогон после очистки** (`probe-empty`, команда `Spec` через
`API.ExecuteAddOnCommand`):
- `summary: {elementsToCreate: 35, elementsToModify: 0, elementsToDelete: 0,
  resultCode: 0, status: "completed"}`, строк — **35**;
- признаков ошибок в ответе нет: ни одного поля/сообщения с `error`,
  `not valid`, `не найден`;
- **все 14 строк, что были при полной модели, совпали до значения** —
  `diff_rows` по пересечению = **0**;
- добавились **21 новая** строка — те, что раньше не доходили до создания,
  потому что существовавшие объекты сопоставлялись и уходили в modify/delete.

**Почему это смена состояния, а не регресс:** при полной модели правила видели
14 своих же объектов (`delete_old`) и давали 2C/12M/2D. На пустой модели
сопоставлять нечего, поэтому всё 35 строк идёт в create. Разница ровно в
существовании объектов, а не в логике.

**Новая точка отсчёта:** `p1-empty` (raw+compare копиями). Старые эталоны
(`p0-smoke`, `r61`…`r73`) описывают ПОЛНУЮ модель и для A/B после очистки не
пригодны — против `p1-empty` они дают 21 расхождение по построению.

**Правило для следующих шагов:** пока файл не сохранён, каждый прогон меняет
модель, поэтому A/B нужно снимать в пределах ОДНОГО состояния модели; после
изменения модели сначала снимается новый эталон. Иначе «расхождение» будет
отвечать на вопрос о состоянии фикстуры, а не о коде.

## R7.4 — ЗАКРЫТ (один владелец payload строки)

**Что сделано.** Четыре прежних поля `Element` (`out_param`, `out_sum_param`,
`out_paramrawname`, `out_sum_paramrawname`) УДАЛЕНЫ. Единственный владелец —
`out_slots`. Ноль обращений к прежним полям в `spec/` осталось; в тестах — ноль.

- **`BuildOutputSlots` строит слоты напрямую из вклада** (`contribution.outParam`
  / `outSumParam`), а не «сначала скопировать в поля строки, затем разложить их по
  схеме». Прежде существовал шаг «скопировать в row.out_param, затем прочитать
  оттуда для построения СЛЕДУЮЩЕГО представления» — теперь представление одно.
- **`SumContributionIntoRow` складывает по слотам с `isSum`.** Поведение
  «сложены только первые MIN(длин)» сохранено дословно, но длины берутся из
  схемы строки и вклада вместо длин прежних массивов. Первый суммарный слот
  имеет индекс `out_slots.GetSize () - accumulated` — суммы занимают последние
  `accumulated` позиций схемы.
- **`OutSlotsMatchSchema` перенесён на схему слотов** (не удалён!). Число
  несуммарных и суммарных слотов считается по флагу `isSum`, условие прежнее:
  ни один набор не пуст, числа совпадают.
- **Accessors с прежней индексацией** (`OutParamValue`, `OutSumValue`,
  `OutParamName`, `OutSumName`, `OutParamCount`, `OutSumCount`) — отображение
  индексов «старой системы координат» на слоты живёт в одном месте, снаружи
  индекс прежний. Новый код читает `OutParam (index)` по объединённой схеме.
- **Тесты:** 118 строк переведены на слоты; вручную построенные строки сумм
  создаются хелперами `PushSumSlot` / `PushOutSlot` (без `isSum` строка выглядела
  бы состоящей только из выходных).

**Поправка к разведке R7.4 (моя ошибка):** я назвал `OutSlotsMatchSchema`
мёртвым кодом — искал вызовы только в `Spec.cpp` и ПРОПУСТИЛ вызов в
`SpecPlanning.cpp:248`. Функция живая; удалять её было бы ошибкой.

**Проверки:** clang-format; AC25 — `Build succeeded!`; sweep AC26-29 —
`status=success`; схема #231: `suites=59 passed=2280 failed=1` — **числа РОВНО те
же, что до R7.4** (R7.3 дал 2280/1), что и есть доказательство сохранности
арифметики; `TestSpecEngineEquivalence` 49/49, `TestSpecRowLayout` 54/54,
`TestSpecContribution` 51/51, `TestSpecReconcile` 57/57,
`TestSpecReconcileFixtures` 37/37, `TestSpecChangePlan` 30/30,
`TestSpecScenarioMatrix` 75/75, `TestSpecRowSlots` 28/28, `TestSpecOutputSchema`
49/49; `FAILED_SUITE TestConvertPropertyToParamValue` — предсуществующая вне
области; раннер exit_code=70 корректно. A/B: база `p2-before-r74` снята ДО правок
(2C/12M/2D, 14 строк), `r74-final` — те же 2C/12M/2D, 14 строк, `diff_rows` = **0**.
Инвариант чтений: 8 в `SpecPlanning.cpp`, 0 в `Spec.cpp`.

## R7.4-ИНВАРИАНТ — порядок слотов (найден по ревью владельца)

**Проблема.** `OutSlotsMatchSchema` проверял схему ПО ФЛАГУ `isSum` (сколько
суммарных, сколько нет), а аксессоры `OutParamValue`/`OutSumValue` и
`SumContributionIntoRow` адресуют слоты ПО ПОЗИЦИИ, пересчитывая число выходных
через тот же флаг. Схема с флагами «сумма, потом выход» проходила сверку чисел
(1 сумма + 1 выход = схема 1+1), и `SumContributionIntoRow` складывал
**ВЫХОДНОЙ** слот вместо суммарного — тихо, без диагностики.

**Это статический риск, не наблюдавшаяся регрессия:** производственный
`BuildOutputSlots` порядок соблюдает (сначала выходные, потом суммы), поэтому на
реальных данных ошибка не возникала. Но схема — публичные данные элемента, и
полагаться на единственного автора нельзя.

**Исправление.** `OutSlotsMatchSchema` теперь сверяет и ПОРЯДОК: выходной слот
после суммарного отвергается. Числа проверяются как прежде, поведение на
корректных схемах не меняется.

**Закреплено `TestSpecSlotOrder` (6 проверок, новый набор):**
- `[сумма, выход]` и `[сумма, сумма, выход]` — отвергаются;
- `[выход, сумма]` — принимается (прежние случаи не сломаны);
- последствия подтверждены числами: на `[сумма 1, выход 2]` аксессор суммы
  возвращает **2**, аксессор выхода — **1**; ровно та ошибка, от которой страхует
  сверка;
- перемешивание через `PushRawSlot` (новый хелпер) недостижимо для
  `PushOutSlot`/`PushSumSlot` — они дописывают в конец.

**Проверки:** AC25 `Build succeeded!`; `suites=60 passed=2286 failed=1`
(было 59/2280/1 — прирост ровно +6 проверок нового набора);
`TestSpecSlotOrder` 6/6, `TestSpecOutSlots` 9/9, `TestSpecRowLayout` 54/54,
`TestSpecContribution` 51/51, `TestSpecEngineEquivalence` 49/49;
`FAILED_SUITE TestConvertPropertyToParamValue` — предсуществующая вне области.
A/B: `r74-inv` vs `p2-before-r74` = 0, vs `r74-final` = 0.

## R7.3 — ПОЛНОТА чтения и расчёта в плане (реализовано)

**Что было.** Поля `notFoundUnicCount` / `notFoundParamCount` были **объявлены, но
в них ничего не писалось** — ноль означал «счётчик не заполняется», а не «всё
прочитано». Хуже: единственные готовые словари `not_found_*` хранят только
**засобочённые** поля (политика повторов, зависит от `stop_on_error`), поэтому при
`stop_on_error = false` они остаются пустыми при реально неполном чтении.
Плана-объекта полноты не существовало.

**Как считается теперь — по ВКЛАДАМ, а не по словарям отчёта.** Вклад — уже
существующее описание чтения (`missingUnic` / `missingSum` / `missingOut`,
`hasSumSlots`), и его данные не зависят от политики отчётов.

| Счётчик | Что означает |
|---|---|
| `notFoundUnicCount` | не прочитано уникальных полей (сумма по всем вкладам) |
| `notFoundParamCount` | не прочитано выходных и суммарных полей |
| `contributionsTotal` | вкладов, дошедших до цикла (включая отклонённые) |
| `contributionsPartial` | вкладов с неполным чтением/схемой |
| `schemaMismatchCount` | вкладов, отброшенных сверкой схемы |

Признаки раздельные, потому что это разные причины: `ReadComplete()` (чтение),
`CalcComplete()` (расчёт), `IsComplete()` (общее). Именно их должен различать F1.

**Две ошибки, найденные тестом при первой реализации (мои, не «подгонка»):**

1. Счётчик стоял под `!= Excluded`, из-за чего **пропадал ровно тот случай,
   который F1 запрещает молчать**: непрочитанное уникальное поле помечает вклад
   `Excluded`, и неполнота исчезала из плана.
2. `contributionsPartial` инкрементировался **дважды на один вклад** — как
   неполное чтение и как `SchemaMismatch`. Теперь это **число вкладов, а не
   причин**: решение принимается один раз, ветка `SchemaMismatch` инкрементит
   лишь для ещё не помеченных вкладов. Иначе сравнение с `contributionsTotal`
   было бы бессмысленным.

**План — необязательный наблюдатель.** `PlanRuleRows` получил `SpecChangePlan *
plan = nullptr`; ни одно условие расчёта его не читает и не пишет, поэтому
вызывающие без плана (все 7 мест в тестах) не изменились. План передаётся и когда
сверка не пойдёт: неполное чтение делает расчёт недостоверным независимо от того,
удаляются ли старые строки. **Политика — дело F1, здесь только факт.**

**`SpecChangePlan` объявлен вперёд** (`struct SpecChangePlan;`): `PlanRuleRows`
объявлен раньше плана в файле.

**Закреплено `TestSpecPlanCompleteness` (36 проверок):** полное чтение → все
счётчики нули, `IsComplete ()` истинно; путь без плана не изменился; убранное
уникальное поле → `notFoundUnicCount == 1`, `ReadComplete ()` ложно **при
`stop_on_error == false`** (то есть прежние словари молчали бы); убранное
суммарное → `contributionsPartial == 1`, `CalcComplete ()` ложно; убранное
выходное → считается в фазе 2; два источника → счётчики **суммируются, а не
перезаписываются**; сверка → `deleteOld == 1`, полнота заполнена, `Matches ()`
истинно; сверки не было (`delete_old = false`) → `deleteOld == 0`, но расчёт-то
выполнен и его неполнота видна.

**Проверки:** clang-format; AC25 `Build succeeded!`; sweep AC26–29
`status=success`; `suites=61 passed=2322 failed=1` (было 60/2286/1 — прирост
ровно +36 проверок нового набора); `TestSpecPlanCompleteness` 36/36,
`TestSpecSlotOrder` 6/6, `TestSpecChangePlan` 30/30, `TestSpecEngineEquivalence`
49/49, `TestSpecContribution` 51/51, `TestSpecRowLayout` 54/54;
`FAILED_SUITE TestConvertPropertyToParamValue` — предсуществующая вне области.
A/B: `r73c` vs `p2-before-r74` / `r74-final` / `r74-inv` — **0, 0, 0**
расхождений. Инвариант чтений: 8 в `SpecPlanning.cpp`, 0 в `Spec.cpp`.

## Параллельные диагностики без владельца (ReadQuantities, Name2Rawname, #211) - статусы на 2026-10-01

## Parallel Task — диагностика ReadQuantities при SpecAll (AC25)

## Scope
Только отладка предупреждения для балки `8EBEA526-944C-4678-AB84-38829E7B9379`; код не менять, остальные задачи не затрагивать.

## Status
IN_PROGRESS (история пути ReadMaterial) — установлено, когда код начал проверять флаги; причина нулей именно в полученных данных/условиях их формирования не доказана. Исправление не выполнялось.

## Last Completed
При вызове JSON Spec через VS Debug остановка на GUID балки: `API_ProfileStructure`, `pll.num=1`, индексы материала `43=43`, `pll.structype=2` (Core из `ProfileItem::IsCore`), `pdd.structype=0` (из `API_CompositeQuantity.flags`); все три компонента количества имеют `flags=0`, все три профильных — `structype=2`. `ACAPI_Element_GetMoreQuantities` вернул `NoError`. Путь по коду: `ParamDictRead` при `param.fromQuantity` выставляет `hasQuantity` (:5513–5536), `ReadMaterial` (`Helpers.cpp:7475–7492`) → `Components` → `ComponentsProfileStructure` (:9069–9084) → `ReadQuantities` (:6732, :6784, :6852, :6907). Коммит `1cab197` от 2026-03-17 добавил присвоение `structype=flags` и сравнение с профилем; до него в соответствующей ветке при добавлении слоя ставился `APICWallComp_Core`, а сравнения не было. Поэтому новая проверка показывает расхождение, которого старый путь не проверял; когда именно данные стали нулевыми, по Git не установлено. На :6975–6976 выбирается запасной расчёт, `APIERR_GENERAL` подставлен логированием; доказательств, что это прерывает SpecAll, нет. Созданные брейкпоинты удалены, отладка остановлена.

## Next Step
Если нужна именно причина изменения входных флагов: сравнить тот же GUID и профиль в архивном PLN/предыдущем работающем окружении; проверить до/после `ReadMaterial` значения и условия формирования без предположения об ошибке SDK. Для изменения кода — отдельная задача/issue и регрессионная проверка результатов быстрой и запасной веток.

## Last Checkpoint
Нет: исходный код не изменялся; запись расследования в IDEA.md не коммитить вместе с чужими правками.

## Plan
- [x] Воспроизвести конкретное предупреждение в AC25 под отладчиком.
- [x] Сопоставить путь ReadMaterial и ревизию до `1cab197`.
- [x] Удалить свои брейкпоинты и завершить сеанс отладки.
- [ ] Установить первопричину нулевых входных флагов сравнением с работающим PLN/окружением; не приписывать её SDK без доказательства.

## Parallel Task — Name2Rawname (AC25)

## Scope
Только восстановление дополнения отсутствующей фигурной скобки в `Sources/AddOn/Sync.cpp::Name2Rawname`, регрессионные тесты `Sources/AddOn/TestFunc.cpp`, карточка `Docs/modules/Sync.md` и связанные generated docs. Незакоммиченные правки `Sync.cpp` и `Helpers.cpp` других задач сохранить. Пользователь разрешил выполнить без GitHub issue только для этой задачи.

## Status
BLOCKED_FOR_CHECKPOINT — `Name2Rawname` исправлена и локальные RED/GREEN проверки AC25 прошли; полный TESTING-набор не зелёный: после фикса в двух проходах 8 `ERROR IN TEST` в `Helpers.cpp::CompareParamValue` (классификации), отсутствовавших в сопоставимом прогоне с прежним условием (там 8 ошибок новых проверок скобок). После правильного ключа в `ParseSyncString` также остаётся `!subproperty.ContainsKey` для указанного свойства. Не утверждать, что весь сценарий синхронизации восстановлен. Issue не создавался по просьбе пользователя.

## Last Completed
Сравнены ревизии до `11add7a` и до `1625cf1`; RED на реальном AC25 (отсутствующая закрывающая скобка), затем GREEN для неполных/полных/отсутствующих скобок в VS «Отладка». `clang-format`, clangd Sync.cpp 0, финальная сборка `BuildAddOn.py -v 25` успешна. Чужие изменения Sync.cpp/Helpers.cpp сохранены; созданные в этой отладке breakpoint-ы удалены, две ранее существовавшие отключённые точки сохранены.

## Next Step
Разобрать отдельно появившиеся 8 ошибок классификации (`Helpers.cpp:1632-1634`) и отсутствие ключа в `subproperty` без изменения кода вне scope; после зелёного полного прогона решить вопрос checkpoint. `_generated/` не обновлён: сигнатуры и связи не менялись, а генератор обнуляет callgraph; точечное обновление строк — отдельный шаг.

## Last Checkpoint
Не создан: полный TESTING-набор не прошёл; посторонние незакоммиченные изменения Sync.cpp нельзя включать в checkpoint.

## Plan
- [x] Сравнить историю `Name2Rawname` и установить причину для неполного имени (по коду/Git).
- [x] Добавить и выполнить RED + GREEN тесты AC25 на двух недостающих скобках и контрольных формах.
- [x] Исправить условие, проверить clang-format/clangd/AC25 build, обновить карточки Sync/TestFunc.
- [/] Исследовать ошибки полного TESTING-набора и подтверждение сценария `subproperty`, прежде чем делать checkpoint.

## Parallel Task — #211 performance Helpers.cpp

## Scope
Только узкие места `Sources/AddOn/Helpers.cpp` из ревью производительности (профиль/материалы, поиск CSV, перечисления, повторный заголовок, формулы), связанные проверки и карточка `Docs/modules/Helpers.md`. Проверочная версия AC25; остальные версии 22–29 не проверены. Часть оптимизаций откачена в `66666f1` (см. Status).

## Status
PARTIALLY_REVERTED — часть оптимизаций из `ccf9cae` откачена по решению пользователя и зафиксирована в `66666f1`: удалён кэш `readMaterialOnce` (`ComponentsProfileStructure`) и снята защита `outstring.Count (part) == 1` в `ReadMaterial`. Issue #211 остаётся OPEN. Возврат этих оптимизаций — при возврате к задаче; проверка на мультислойной конструкции не проводилась.

## Last Completed
Правки пяти участков (профиль/материалы, CSV, перечисления, заголовок элемента, формулы); clang-format, clangd 0, сборка AC25 и запуск тестового PLN через runner успешны. Производительность и выходные значения в Archicad не замерены; #209 сохранён.

## Next Step
#211: решить судьбу откатанной части (вернуть оптимизации `ccf9cae` с регрессионной проверкой на мультислойной конструкции или закрыть как отменённую). До этого — замерить, даёт ли возврат `readMaterialOnce` и защиты `outstring.Count (part) == 1` измеримое ускорение; сейчас не замерено.

## Last Checkpoint
`ccf9cae` — `[#211] Сократить повторную работу Helpers` (только правки #211; изменения #209 в Helpers.cpp остались незакоммиченными).

## Plan
- [x] Проверить доступные контракты и исходные вызовы; LightRAG не ответил, использованы проектные определения и AC25 build.
- [x] Устранить повторные операции; clangd и AC25 build успешны, поведенческая проверка отложена пользователем.
- [/] Проверить diff, clang-format, clangd и AC25 build (выполнено); runtime сравнение и профилирование ожидаются.
- [x] Актуализировать карточку и выполнить checkpoint `ccf9cae`; generated docs обновить отдельно.
- [/] Решить судьбу откатанной части (`66666f1`): вернуть с регрессионной проверкой на мультислойной конструкции или закрыть #211 как отменённую.

## #245 — единое окно результата запуска Spec (R9.2), перенесено из IDEA.md 2026-10-02

## #245 — единое окно результата запуска Spec (R9.2) — РЕАЛИЗОВАНО, ждёт checkpoint

- Scope: накопитель ошибок и per-rule счётчиков в `SpecRunResult`; фактические
  (а не плановые) счётчики созданий; окно результата на данных
  `RuleSelectData::columnTitles`/`valuesPerRule`/`footerText`/`isReadOnly`;
  снятие всех всплывающих окон запуска; `rules`/`messages` в JSON за
  `includeParameters`. `BrowserPalette.cpp`, `Interface_ru.html`,
  `ТЗ интерфейс.md` — чужая работа #246, не тронута.
- Status: **код готов, собран, прогнан.** Остался только checkpoint.
- Issue: https://github.com/kuvbur/AddOn_SomeStuff/issues/245
- Реализовано: `SpecMessage`/`SpecRuleStats` + `ruleNames`/`ruleStats`/`messages`
  и `EnsureRuleStats`/`AddGeneralMessage`/`AddRuleMessage`/`HasRuleError`
  (`Spec.hpp`); `PlaceElements` получил `createdByRuleIndex` и считает
  подтверждённые создания, после транзакции план заменяется фактом
  (`SpecExecutor.*`); `ShowRunResult` собирает строки/цвет/подвал
  (`Spec.cpp:528`); `RuleSelectData` расширен пятью полями, вёрстка `AddOn.grc`
  не менялась; `SpecCompat` — `rules`/`messages`/`RuleFieldNames`/
  `MessageFieldNames`, `VerifyResponseFields` 14→16.
- Сознательные решения: (1) ошибки чтения привязаны к правилу — источник полей
  у него один; (2) ошибки без правила идут вниз без привязки; (3) подсветка
  в модели и лимит «< 20 элементов» сохранены — это работа с моделью, а не
  показ сообщения; (4) `runResult == nullptr` при интерактивном запуске создаёт
  локальный накопитель, иначе отказ остался бы невидимым; (5) при пустых
  `columnTitles` поведение окна прежнее — `ReNum.cpp`/`Summ.cpp` не затронуты.
- Грабли (все пойманы в этом шаге): `itemCount` — ЧИСЛО колонок, а не последний
  индекс, поэтому граница окраски `tab <= itemCount`, иначе последняя колонка
  строки осталась бы некрасной; `SetSize` обязан брать `QtyTab_w` для колонки
  количества и `ValueTab_w` для колонок значений — общая ширина развела бы
  заголовок и поле; `UniString::CStr` некопируем и не годится как аргумент
  `Printf` (C2280) — строка «[ИМЯ] — текст» склеивается конкатенацией.
- Валидация: clang-format на 13 файлов; clangd 0 ошибок (2 warning
  `unused-includes` в `DG4rule.cpp` предсуществующие); AC25 `Build succeeded!`;
  sweep AC26–29 `success`; `restart_archicad_for_test.ps1` exit_code=70 →
  `suites=70 passed=2847 failed=1`, `TestSpecRunReport` 29/29,
  `TestSpecResponseContract` 26/26, дельта по 69 прежним наборам = 0 (+37 =
  29 нового набора + 8 контракта). Единственный `FAILED_SUITE
  TestConvertPropertyToParamValue` предсуществующий и вне области.
  **Поведение окна в Archicad НЕ наблюдалось** — `not verified`: вёрстка,
  перенос текста в подвале и число колонок проверены только сборкой.
  A/B (`spec_baseline.py`) не выполнялся: `rules`/`messages` за
  `includeParameters`, прежние базы собраны без флага — сравнение было бы
  сравнением разных форм ответа. AC22–24 и macOS — `not verified`.
- **ID-коллизия, найденная при подготовке чекпоинта.** Незакоммиченная правка
  `IDEA.md` — это issue **#246** («Показать в 3Д» в `SyncShowSubelement`), и в
  ней записано «свободен 87» под `ShowButtonId`. Я занимал 87/88/89 →
  перенёс свои строки на **90/91/92** (`Constants.hpp`, `Tools/AddOn.grc.in`),
  87 остаётся за #246. Пересборка после переноса — `Build succeeded!`.
- Next Step: **ждёт коммит #245** (код готов, собран, прогнан).

## Разделение на #245 и готовые файлы в `Reviews/`

- Чужое (ваш коммит, вариант 2): `foreign_234_Spec_hpp.patch`, `foreign_234_Spec_cpp.patch`
  — 1 хунк в `.hpp` + 1 в `.cpp` (при `-U15`; при `-U3` hpp-патч не применяется).
- Моё (поверх вашего коммита): `mine_245_Spec_hpp.patch` (5 хунков),
  `mine_245_Spec_cpp.patch` (18). Остальное — старая чистая статья: `Constants.hpp`,
  `DG4rule.*`, `SpecCommand.cpp`, `SpecCompat.*`, `SpecExecutor.*`, `TestFunc.*`,
  `TestSpec.cpp`, `Tools/AddOn.grc.in`.
- Проверено сборкой: `foreign_*` применяются к чистому HEAD, после них `mine_*`
  дают побайтово полные файлы. Сборка `foreign_*` без моих правок — `Build succeeded!`.
- **Выполнено:** чужая часть закоммичена отдельно (`dbcfde6`, `Refs: #234`), мой
  коммит — следующий. `IDEA_ARCHIVE.md` и блок `#246` в `IDEA.md` в него НЕ вошли:
  архив относится к #211-#233 (работа владельца), блок #246 — «Показать в 3Д».


## Головная находка к чекпоинту

- `Spec.hpp` единственный файл с очужими хунками, но они не мои: `CheckRuleElementByRule` уже вызывается
  из `HEAD:Sources/AddOn/spec/Spec.cpp:2759`, а объявляться только в незакоммитом `Spec.hpp` — то есть
  **предсуществующий незакоммитный фикс**, а не новая работа. `HEAD` в таком состоянии
  не сборается. `destinationNamePropFound`/`destinationGuidPropFound` — тоже незакоммитные поля с
  расширением виртуальной связки (#234). Из моих хунков в `Spec.cpp` смешанных
  с чужими нет не в одном хунке. Разделение по этим хункам — независимый
  вопрос владельцу.

## Spec refactor #228 — закрытые этапы R3-R9.2, #234 и #245 (перенесено из IDEA.md 2026-10-02)

## Task — рефакторинг Spec (R7.6 закрыт; далее R8 SpecExecutor, затем R9-R10)

Issue: #228 (kuvbur/AddOn_SomeStuff) — рефакторинг; #227 — дамп значений элементов.
План: `Reviews/2026-09-27_174500-spec-refactor-no-regression.md`.

## Параллельная задача — валидатор правила по GUID (#234)

- Scope: отдельная read-only функция для проверки Spec по GUID свойства-правила с необязательным GUID элемента; минимальное расширение `GetElementForPlaceProperties` для значения флага и строгой проверки избранного; тесты в `Sources/AddOn/tests/`; карточка `Docs/modules/spec/Spec.md`, `symbols.json`, `_progress.md`. `Helpers.cpp`, рефакторинг #228 и ресурсы не менялись.
- Status: **DONE (код валидирован)** — реализовано и проверено. Issue: https://github.com/kuvbur/AddOn_SomeStuff/issues/234.
- Plan: [x] создать и сверить issue #234; [x] проверить существующие функции Spec/Helpers и SDK (AC24 not verified); [x] согласовать владение `Spec.cpp/.hpp` и реализовать новую функцию; [x] clang-format, LSP, сборка, runtime, ревью, документация; [/] checkpoint только для своих фрагментов.
- Реализовано: `EvaluateRuleFlag` (`Spec.cpp:2447`), `CollectUnreadRuleNames` (:2520), `CheckRuleByPropertyGuid` (:2608), внутренние `static` `GetRulePropertyDefinition` (:2559) и `ReadElementRuleFlag` (:2571); в `Spec.hpp` — `PlaceSourceInfo`, `RuleFlagStatus` (5 состояний), `RuleFlagOrigin`, `RuleFlagCheck`, `RuleCheckResult`. `GetElementForPlaceProperties` расширена одним необязательным `PlaceSourceInfo *readInfo = nullptr`; существующий вызов (`Spec.cpp:662`) не затронут.
- Осознанные решения: (1) возвращаемый `bool` = `definitionFound`, а не «правило корректно»; (2) `favoriteFound` и `fromDefaultElem` разведены — успешная сверка после fallback на объект по умолчанию не доказывает, что избранное найдено; (3) `status == API_Property_NotAvailable` оставлен отдельным ответом, хотя прод `GetRuleFromElement` трактует его как «флаг включён» — расхождение намеренное, не сводить без решения владельца; (4) на AC22–23 нет ни `API_Property::status`, ни `API_PropertyValueStatus` — только `isEvaluated`, а `NotAvailable` невыразимо, поэтому в тесте два набора ожиданий под `#ifdef ServerMainVers_2400`.
- Валидация: clang-format на 5 файлов; clangd 0 ошибок (warning `PushRawSlot` в `TestSpec.cpp:45` — предсуществующий, вне моего хука); AC25 `success`, sweep AC26-29 все `success`; `restart_archicad_for_test.ps1` exit_code=70 → `suites=64 passed=2445 failed=1`, `TestSpecRuleCheck` 43/43, единственный `FAILED_SUITE TestConvertPropertyToParamValue` предсуществующий и вне области; **дельта по всем 63 существующим наборам = 0**. AC22/23 падают на ресурсах и чужих файлах — проверено на чистом дереве через `git stash`, те же сбои без моих правок. **AC24 not verified** (DevKit-24 отсутствует). macOS не собирался.
- Next Step: сделать checkpoint (issue-коммит с `Refs: #234`) — после него задача закрывается и возвращает управление владельцу рефакторинга #228.
- Last Checkpoint: нет; HEAD `c2d0ab4`, коммита #234 по-прежнему нет — работа лежит в рабочем дереве.

## Продолжение #234 — вкладка «Спецификация» в BrowserPalette (UI-макет)

- Scope: `Sources/AddOnResources/RFIX/HTML/Interface_ru.html` (секция 8, состояние
  `STATE.spec`, две функции-мока в `ACBridge`, вкладка `renderSpecTab`), `ТЗ интерфейс.md` §9,
  `IDEA.md`. Мост в `BrowserPalette.cpp` **не трогаем** — по решению владельца это
  отдельная задача после согласования макета. Прод-функция
  `Spec::CheckRuleByPropertyGuid` не менялась, её первый вызывающий — будущий мост.
- Status: **макет готов, валидация HTML зелёная.** Два из пяти запрошенных пунктов
  («полный цикл с мостом») и («полный проектируемый макет») пользователь снял в
  пользу макета на моке — мост и runtime отложены, до пересборки аддона в Archicad
  вкладку не видно.
- Согласованные решения: список свойств с поиском по имени; оба источника (выделение,
  а при пустом выделении — свойства проекта); все выделенные элементы с лимитом и
  строкой «показаны первые N из M»; пять состояний `RuleFlagStatus` и пять
  `RuleFlagOrigin` показываются как есть, без сведения к включён/выключен.
- Найдено и исправлено при замере на 230px (окно палитры из `Tools/AddOn.grc.in`):
  фиксированная колонка подписи `9.5rem` съедала текст, строки действий и заголовка
  элемента не переносились, строка лимита распирала шапку; `blockShell` давал
  заголовку `flex:0 0 auto`, из-за чего длинное имя блока вытесняло сводку в нулевую
  ширину — правка в общем помощнике затронула и вкладку «Нумерация» (проверена).
- Правка по требованию владельца: «Флаг назначения» разделён на «Флаг у избранного»
  и «Флаг у элемента по умолчанию». Важно по коду: `destinationFlag` — одно
  значение с пометкой `origin`, а не две независимые проверки (`Spec.cpp:2627-2641`),
  поэтому неактивная строка помечается «не проверялось», а не заполняется
  выдуманным значением; при `Unknown`/`NotChecked` добавляется строка «Назначение:
  не прочитано». Мок расширен пятью ветками назначения, иначе вторая строка не
  получила бы настоящего значения и осталась непроверенной.
- Структура вкладки: два сворачиваемых блока «Проверка» (лимит, кнопка, результаты)
  и «Редактирование правила» (пока заглушка), через общий `blockShell`; сводка в
  заголовке блока «Проверка» живёт из `cfg.report`, а не из DOM-узла, иначе
  сворачивание теряло бы её.
- Решение владельца по первому открытому вопросу: признак расширяется мостом, а не
  фильтруется в UI. Контракт `GetSpecRuleProperties` отдаёт точный
  `hasSpecRule` (наличие команды `Spec_rule` в описании) вместо агрегата
  `hasRule`. Проверено по коду, почему это важно: `hasRule` в
  `GetPropertyRuleFlag` (`Propertycache.cpp:23-30`) суммирует Sync, Spec_rule,
  Renum(_flag) и Sum, то есть список валидатора был бы забит чужими правилами.
  Второй важный факт: `isValid` у команды `Spec_rule` означает только «нашлась
  закрывающая скобка» (`Sync.cpp:3070-3104`), а НЕ что флаг включён — поэтому
  `hasSpecRule` и включённость флага разведены и в контракте, и в UI.
  Мок расширен пятью свойствами, одно помечено `hasSpecRule: false`.
- Валидация: `Tools/test_html.ps1` — `ALL HTML CHECKS PASSED`; ESLint инлайна — 0 ошибок,
  7 warning предсуществующих и вне блока; рендер в headless Edge на 230px — 5 свойств
  в проекте дают 4 в списке (свойство без `Spec_rule` отфильтровано, поиск по нему
  даёт «Ничего не найдено»), все четыре проверяемых свойства дают ожидаемые тексты
  обеих строк флага, сворачивание/разворачивание сохраняет отчёт, «Нумерация» не
  сломана, переполнения нет. **Сборка не выполнялась, runtime не проверялся** — мост
  моковый, в Archicad вкладка не появится до пересборки аддона.
- Next Step: мост в `BrowserPalette.cpp` — `GetSpecRuleProperties` (PROPERTYCACHE +
  разбор описания на `Spec_rule`) и `CheckSpecRuleByPropertyGuid` (вызов
  `Spec::CheckRuleByPropertyGuid` по элементам с лимитом), оба ответами
  JSON-строкой.
- Next Step: показать макет владельцу.


## Scope

Пошаговый рефакторинг движка спецификаций по плану (issue #228). Файлы:
`Sources/AddOn/spec/Spec.cpp/.hpp`, `SpecPlanning.cpp/.hpp`,
`Sources/AddOn/tests/TestSpec.cpp`, `tests/TestFunc.cpp/.hpp`,
`Docs/modules/spec/Spec.md`, `Docs/modules/spec/SpecPlanning.md`, `IDEA.md`.
Эталоны A/B в gitignored `Reviews/spec_baseline/`.

**Вне scope (действует):** `Helpers.cpp` (26 включений, под замком),
`Sources/AddOnResources/`, потоки **F1** (политика разрушительных действий при
неполных данных) и **F2** (достоверный внешний статус) — планом выведены из R и
требуют отдельного согласования политики владельцем. В R3 нельзя попутно менять
политику удаления: она принадлежит F1.

**Не входит в этот шаг:** R9 (оркестратор), R10 (приёмка) — они следуют за R8.

**Снято 2026-10-01 23:5x: параллельная правка `Sources/AddOn/spec/` больше не
в рабочем дереве** — принята, проверена и закоммичена как `c32dfed`
(`Refs: #237-#242`). Правки R7.6 лежали в `tests/` и с ней не пересекались;
коммиты разнесены, чужая работа не переписана.

**Следствие устранено:** свежий build+прогон на объединённом коде выполнен —
AC25 Debug `Build succeeded!`, `SUMMARY suites=66 passed=2750 failed=1`
(`TestSpecChangePlan` 43, `TestSpecRowSlots` 43, `TestSpecScenarioMatrix` 108/0).
Единственный провал `TestConvertPropertyToParamValue` предсуществующий. A/B
`r76-scaling` = 0 остаётся в силе: объединённый код не меняет вывод R7.6.

Проверочная версия AC25 Windows Debug. AC22-24/26-29 — только сборка (sweep),
macOS — `not verified`.

## Статус актуальных шагов

| Шаг | Состояние | Основание |
|---|---|---|
| R3.1-R3.3 | закрыты | признаки готовности и GUID-маркер разведены, `IsRunnableForRun ()` |
| R3.4 (первый путь) | закрыт | `exsist_elements` вынесен в `SpecRuleRunState`, 15/15, A/B = 0 |
| R3-остаток | закрыт | `SpecRule` без полей состояния; A1-A4, чекпоинты A2-A4, A/B = 0 против шести баз |
| R4 | закрыт | `TestSpecParseError` 71, `TestSpecParser` 318 |
| R5 | закрыт | 8 чтений в `SpecPlanning.cpp`, 0 в `Spec.cpp` |
| R6 | закрыт | `TestSpecEngineEquivalence` 49/49 |
| R7.1-R7.4 | закрыты | см. чекпоинты `0ce39d2`, `b1bd9ae`, `6d68d56`, `2cb00d9`, `936890b` |
| R7.3-полнота | закрыта | `TestSpecPlanCompleteness` 36/36, `IsComplete ()` |
| R7.5 | закрыт | `TestSpecOperationMatrix` 216/216 |
| R7.6 | закрыт | `TestSpecReconcileScaling` 22/22, ratio 1.96-2.16 при удвоении E — линейно |
| **R8** | **в работе** | R8.1-R8.5 закрыты кодом; дальше R8.6 (приёмка на модели) |
| R9-R10 | открыты | не начаты |

## Текущий шаг — R7.6 (закрыт 2026-10-01)

**Вопрос:** растёт ли стоимость сверки с числом ранее размещённых объектов E
квадратично.

**Инвентарь (R7.6.1, по коду).** `ReconcileExistingRows`
(`SpecPlanning.cpp:253-423`) делает ровно два прохода по
`runState.exsist_elements` — основной (:269) и добор устаревших (:398); всё
остальное — `outParam.ContainsKey`, `elements.ContainsKey`,
`elementsMod.ContainsKey`, `elements.Get`, `elements.Delete`, то есть операции
хеш-таблиц. `SelectExistingElements` (`Spec.cpp:1426`) — один проход по `found`
с проверкой `selected_elements.ContainsKey`, тоже хеш. Дорогие вызовы
(`GetElementForPlaceProperties`, `GetElementByPropertyDescription`) — по одному
на правило, вне циклов по E; `CommonFunction.cpp` (где живёт
`GetElementByPropertyDescription`) рефакторингом не тронут. Сверка с master:
число вызовов этих функций в `Spec.cpp` не изменилось (1 и 5→7, из них 2 новых —
вне циклов, в копии правила для проверки назначения).

**Замер (R7.6.2-3).** Набор `TestSpecReconcileScaling`
(`tests/TestSpec.cpp`, 22/22). Ряд E = 64, 128, 256, 512, 1024, 2048, каждый —
сходящийся no-op (значения прежних объектов совпадают с источниками), повторы
200/200/200/50/50/10. Результат в отчёте стенда и в
`Reviews/spec-refactor-baseline/scaling-r76.{csv,json}`:

| E | ms | мкс/строку | отношение к предыдущей точке |
|---|---|---|---|
| 64 | 0.753 | 11.767 | — |
| 128 | 1.623 | 12.679 | 2.155 |
| 256 | 3.175 | 12.403 | 1.956 |
| 512 | 6.359 | 12.420 | 2.003 |
| 1024 | 13.256 | 12.945 | 2.085 |
| 2048 | 27.846 | 13.596 | 2.101 |

**Вывод: квадратичной составляющей нет.** Отношения при удвоении E держатся
1.96-2.16 (линейный код даёт 2, квадратичный — 4); накладные на строку растут с
11.8 до 13.6 мкс, то есть на 15% на шестикратном росте E — это работа с
хеш-таблицами и аллокациями строк, а не второй проход. Набор проверяет порог
`ratio <= 3.0`, поэтому регрессия к квадратике упала бы в прогоне, а не в
комментарии.

**Границы доказательства (важно).** Замер идёт по словарям фикстуры, поэтому
стоимость вызовов ACAPI в нём нулевая: измеряется ФОРМА зависимости от E, а не
абсолютное время на модели. Абсолютная стоимость на реальной модели не измерена
и остаётся `not verified`; `Tools/spec_benchmark.py` меряет no-op на одном
входе и не варьирует E, а сеттера свойств в JSON-порту нет (#226), поэтому
прогнать варьируемый E на модели нечем. Продакшн-код не менялся.

**Валидация:** clang-format на 3 файла; AC25 — `Build succeeded!`; sweep
AC26–29 — `status=success`; прогон (AC25, 2026-10-01 23:19) `suites=66
passed=2689 failed=1` — провал прежний и вне области; A/B `r76-scaling` —
`diff_rows=0` против пяти баз. AC22-24, macOS — `not verified`.

**Побочно зафиксировано.** Первый прогон дал `passed=2683`, второй — `2689` при
идентичных наборах и сумме по наборам 2689 в обоих отчётах: расхождение в
сводке раннера, а не в тестах. Причина не исследована — считать это известной
особенностью сводки, а не подтверждённым фактом.

## Текущий шаг — R3-остаток (закрыт 2026-10-01)

Цель: `SpecRule` перестаёт владеть состоянием запуска. Определение правила
(разобранное из описания) и результат конкретного запуска — разные вещи, а
сейчас они смешаны в одной структуре.

**Оценка целесообразности.** Задача выбрана пользователем перед R7.6, и это
разумно: R7.6 — измерение без правок кода, а R3-остаток разблокирует R8/R9
(оркестратор различает стадии, а не смешивает их в одной структуре) и
устраняет реальный риск, найденный в R3: `elements` заполняется ДО выбора
пользователя, поэтому план чтения строится по правилам, которые пользователь
возможно отменит. Это не косметика — это расхождение стадий.

**Оценка простоты: низкая-средняя, вопреки ожиданиям.** Ключевой факт,
проверенный по коду: `SpecRuleRunState runState` уже является членом `SpecRule`
(`Spec.hpp:90`), а `exsist_elements` уже живёт в нём. Поэтому перенос
остальных трёх полей — **переименование доступа, а не изменение структуры**:
сигнатуры функций не меняются, `SpecRule` остаётся параметром везде, где он
был. Настоящий риск не в механике, а в двух местах:
  - **писатели** (см. инвентарь) — каждый писатель должен попасть в свою стадию,
    иначе поле начнёт читаться до заполнения (молча, как уже было с `elements`);
  - **стадия плана чтения** — `GetParamToReadFromRule` (Spec.cpp:1318) читает
    `rule.elements` ДО диалога, и это ДОЛЖНО остаться: перенос сюда не входит.

**Инвентарь (A1, выполнен 2026-10-01).** Проверен по коду, не по памяти.

Поле `elements` (список источников, 6 мест):
| Место | Стадия | Перенос |
|---|---|---|
| Spec.cpp:110 | разбор флага в `GetRuleFromElement` (default) | A2 |
| Spec.cpp:473 | отчёт `qty_elements` в `SpecDG` | A2, читается до выбора — законно |
| Spec.cpp:1186 | `AddRule`, повторное описание дописывает | A2 |
| Spec.cpp:1318 | `GetParamToReadFromRule`, план чтения | **не переносится** (см. ниже) |
| Spec.cpp:1705 | `PlanRuleRows`, обход источников | A2, читает |
| Spec.hpp:160,162 | комментарий контракта | A2 |

Поле `selected` (2 писателя, 4 читателя):
| Место | Стадия | Перенос |
|---|---|---|
| Spec.cpp:492 | `SpecDG` — запись выбора пользователя | A4 |
| Spec.cpp:601 | `SpecArray`, JSON-путь `ruleNames` | A4 |
| Spec.cpp:466, 488, 657 | читатели | A4 |

Поле `destinationReady` (2 писателя, 1 читатель):
| Место | Стадия | Перенос |
|---|---|---|
| Spec.cpp:1348, 1355 | `MatchDestinationProperties` | A4 |
| Spec.cpp:599 | `SpecArray`, фильтр перед планом | A4 |
| Spec.cpp:1360 | возврат значения | A4 |

Поле `destinationParamGuidName` (1 писатель, 3 читателя):
| Место | Стадия | Перенос |
|---|---|---|
| Spec.cpp:1413 | `ResolveFavoriteLinks` | A4 |
| Spec.cpp:1458 | `AddExistingReadRequests` | A4 |
| SpecPlanning.cpp:238 | `AddContributionToRow` → `row.subguid_paramrawname` | A4 |
| SpecPlanning.cpp:371 | `ReconcileExistingRows`, чтение GUID-связи | A4 |

Тесты: 55 обращений в `Sources/AddOn/tests/TestSpec.cpp` (в основном
`destinationParamGuidName` и `selected`). Ни одного обращения к этим полям
вне `Sources/AddOn/spec/` и `tests/` — ни в `Helpers.cpp`, ни в
`json_commands/` (проверено grep).

**Решение по стадии плана чтения.** `GetParamToReadFromRule` читает
`rule.elements` до диалога — переносить это в R3 нельзя: план чтения
определяется для ВСЕХ правил, отобранных на этапе сбора, а не только для
выбранных пользователем. Если перенести, план сузится (это и есть «находка
№1»). Значит `elements` переносится в `runState`, но чтение плана остаётся
на старом месте через `rule.runState.elements` — и это правильно, потому что
`runState` тоже член `SpecRule`, а значит путь до данных не меняется.

**Подзадачи (порядок строго последовательный — каждый следующий опирается на
предыдущий).**

**A1 — инвентарь.** Собрать и зафиксировать всех писателей/читателей по
четырём полям. *Критерий приёмки:* таблица выше в IDEA, ни одного непроверенного
места; при расхождении с кодом — записать код, а не таблицу.
**Статус: выполнено 2026-10-01.**

**A2 — `elements` в `runState`.** Перенести поле, обновить писателей
(Spec.cpp:110, :1186) и читателей (:473, :1705), комментарии контракта.
Тесты `TestSpecGrouping`/`TestSpecReadPlan`/`TestSpecEngineEquivalence` держат
контракт «список источников не сужается».
*Критерий приёмки:* сборка AC25, полный набор зелёный с дельтой 0, A/B
`diff_rows = 0`, `TestSpecRunStateBoundary` и `TestSpecReadPlan` подтверждают
прежний объём чтения.
**Статус: выполнено 2026-10-01, чекпоинт — см. `Last Checkpoint`.** Сигнатуры не
менялись (`runState` и раньше был членом `SpecRule`) — перенос оказался
переименованием пути доступа. Промежуточный дубль `elementsToMove` был заведён
и немедленно отменён: он нарушал «не держать два набора данных». Главный риск —
сужение плана чтения — не реализовался: гейт по `selected` в
`GetParamToReadFromRule` не вводился, момент чтения прежний. Пробная сборка
упала на `LNK1168` (открытый Archicad держал `.apx`), после `Stop-Process`
прошла; прогна до этого не было. Валидация: clang-format на 3 файла, AC25
`Build succeeded!`, sweep AC26-29 `success`, `suites=65 passed=2661 failed=1`
(числа совпали с прогоном до переноса; `TestSpecOperationMatrix` 216/216,
`TestSpecReadPlan` 26/26, `TestSpecRunStateBoundary` 15/15, дельта = 0), A/B
`r3a2-elements` `diff_rows=0` против `r75-opmatrix`/`r4-final`/`r35-multiline`/
`r3-final`. Карточка `Docs/modules/spec/Spec.md` обновлена (раздел R3.A2 +
устаревшее утверждение R3.4 о невозможности переноса). AC22-24, macOS —
`not verified`.

**A3 — `selected` в `runState`.** Два писателя (SpecDG и JSON) — разные
стадии, поэтому переносим оба и проверяем, что выбор одного правила не влияет
на соседние. *Критерий приёмки:* `TestSpecSelectionPolicy` (65/65) остаётся
зелёным без правок ожиданий, дельта 0, A/B = 0.
**Статус: выполнено 2026-10-01.** `IsRunnableForRun ()` переписан на
`runState.selected`; гейт по-прежнему требует трёх признаков, поэтому набор
остался зелёным без правок ожиданий — это и было главным риском шага. Промежуточный
дубль поля в `SpecRule` был заведён и отменён в том же шаге (clangd показал
`Duplicate member 'selected'`). Валидация: clang-format на 3 файла, AC25
`Build succeeded!`, sweep AC26-29 `success`, `suites=65 passed=2661 failed=1`
(без изменений относительно A2), A/B `r3a3-selected` `diff_rows=0` против
пяти баз. Карточка `Spec.md` дополнена разделом R3.A3. AC22-24, macOS —
`not verified`.

**A4 — `destinationReady` и `destinationParamGuidName` в `runState`.** Два
поля одной стадии (разрешение избранного) — переносятся вместе, потому что
раздельно получится промежуточное состояние, в котором `destinationReady`
переехал, а имя свойства ещё нет. *Критерий приёмки:* `TestSpecFavoriteResolution`,
`TestSpecGrouping` (проверяет `row.subguid_paramrawname`) зелёные, дельта 0,
A/B = 0.
**Статус: выполнено 2026-10-01.** Все писатели и читатели перенесены:
`MatchDestinationProperties` (:1348, :1355, :1360), `ResolveFavoriteLinks`
(:1413), читатели :599, :1458, `SpecPlanning.cpp:238` и `:371`. Маркер
`subguid_paramrawname` остался в `SpecRule` — это свойство описания, а не
результат разрешения. Валидация: clang-format на 4 файла, AC25
`Build succeeded!` (без LNK1168), sweep AC26-29 `success`,
`suites=65 passed=2661 failed=1` (без изменений), `TestSpecFavoriteResolution`
33/33, `TestSpecGrouping` 131/131, A/B `r3a4-favorite` `diff_rows=0` против
шести баз. Карточка `Spec.md` дополнена разделом R3.A4. AC22-24, macOS —
`not verified`.

## Итог R3-остатка

`SpecRule` больше не хранит ни одного поля состояния запуска.
`SpecRuleRunState` содержит `elements`, `selected`, `destinationReady`,
`destinationParamGuidName`, `exsist_elements`. Определение правила осталось в
`SpecRule`: `groups`, схема выходов, `parseValid`/`parseError`, `favorite_name`,
`subguid_*` (маркер и носители), `rule_definitions`, флаги политики.
**Сигнатуры публичных функций не менялись ни в одном из четырёх шагов** —
`runState` и раньше был членом `SpecRule`. Ни в одном шаге не вводился
промежуточный дубль поля: дважды это было заведено по недосмотру и немедленно
отменено (A2 — `elementsToMove`, A3 — дубль `selected`, пойман clangd).

**Оценка по факту: задача оказалась проще, чем выглядела.** Разница между
ожиданием и фактом — не в механике, а в количестве мест: каждый шаг упирался
в переименование 5-12 обращений, а не в проектирование. Настоящая цена —
дисциплина инвентаря: в A2 и A3 я сначала заводил дубль поля «на всякий
случай», что прямо нарушало правило «не держать два набора данных»; оба раза
это ловилось немедленно, но правильнее было бы начинать с удаления старого
члена.

**Общий критерий для A2-A4:** каждый подзадача — отдельный коммит;
`docs/modules/spec/Spec.md` обновляется в том же коммите, если меняются
контракт или подпись; ни одна подзадача не закрывается на «собралось».

**Что осознанно НЕ делалось в R3-остатке:**
- не менялся порядок стадий (SpecDG остаётся после плана чтения);
- не менялась политика неполных данных (F1, issue #236) — это отдельное решение;
- не вводилась новая структура `SpecModel`/адаптеры: `runState` уже есть, второй
  уровень вложенности ничего не даёт;
- `Helpers.cpp` не трогался (26 включений, под замком) — по инвентарю он
  этих полей не касается.

Пять сценариев матрицы закрыты новым набором `TestSpecOperationMatrix`
(`tests/TestSpec.cpp`, зарегистрирован в `TestFunc.cpp`/`.hpp`, группа
`spec`): 216 проверок, все зелёные. План получается от `GetElementsForRule`
(`RunWithPlan` в фикстуре), а не строится в тесте, поэтому проверяется само
решение.

**Что закрыто на подставной фикстуре (полностью):**
- S04 v2 — создание / изменение с сохранением GUID / удаление / отсутствие
  изменений, каждый отдельным прогоном;
- S05 v3 — частично отсутствующие данные: непрочитанная сумма (строка
  отбрасывается проверкой схемы, отметки элемента нет, план неполон),
  непрочитанный уникальный параметр (вклад исключается, строки нет);
  видимость не спрашивается у модели; та же неполнота при `stop_on_error`;
- S18 — смена выходного значения (это УДАЛЕНИЕ + СОЗДАНИЕ, не изменение),
  смена суммы при том же выходе (изменение с сохранением GUID), лишняя
  строка, два прежних объекта с одинаковым выходом (первый забирает строку,
  второй удаляется как `RowAlreadyClaimed`), выбор представителя по первому
  совпадению выходного значения;
- S20 — смешанный create+update+delete одной строкой, порядок обхода,
  неизменность прочитанных данных после планирования; отказ создания на
  расчётной стадии (непрочитанное выходное поле → схема не совпала → строки
  нет, но связь в словаре выходов остаётся);
- S23 — пустой результат / только изменения / только удаления в порядке
  обхода / только сопоставленные без изменений (возвращаемое число 0 при
  `unchanged = 2` — действующий контракт, проверен явно).

**Что НЕ закрыто (статус «не проверено», не «покрыто»):**
- отказ создания при записи в модель, частичный и полный (S20) — возникает в
  исполнителе, чтение словарей его не воспроизводит; на этом шаге закрыт
  только отказ на расчётной стадии;
- отбраковка невидимого источника (S05) — элемент синтетический, в модели его
  нет; проверено только решение не спрашивать модель при `only_visible = false`;
- состояние модели после каждого этапа и откат неудачной записи.

**Разбор прогона.** Первый прогон дал 1 провал: `v2 rejected removal reason
| expected 1 got 2`. Причина не в опечатке ожидания, а в действующем порядке
вещей — разобран по коду и зафиксирован как контракт: при `stop_on_error`
строка была посчитана, связана в словаре выходов (`AddContributionToRow`
пишет `outParam` ДО проверки схемы), но отброшена проверкой схемы; словарь
выходов при отказе не очищается, поэтому прежний объект находит пару на
несуществующей строке и удаляется с причиной `RowAlreadyClaimed`, а не
`NoNewRow`. Само удаление от этого не меняется, но диагностика называет
причину неверно — по причине судят, чего не хватило. Это следствие политики
F1, а не дефект этого шага; правка причины — отдельное решение владельца.
Открыт **issue #236** (неверная причина удаления + удаление размещённых строк
при неполных данных); коммит закрепляет найденное поведение, а не исправляет
его.

**Побочно зафиксировано (закреплено в тесте, не менялось):** смена выходного
значения у размещённого объекта даёт удаление + создание, а не изменение, так
как сопоставление идёт по выходному значению.

**Валидация:** clang-format на 3 файла; clangd 0 ошибок (4+2 warning
`unused-includes` предсуществующие); AC25 Windows Debug `Build succeeded!`;
sweep AC26-29 все `success`; `restart_archicad_for_test.ps1` exit_code=70 →
`suites=65 passed=2661 failed=1`, `TestSpecOperationMatrix` 216/216, дельта по
63 существующим наборам = 0; единственный `FAILED_SUITE
TestConvertPropertyToParamValue` предсуществующий и вне области (тот же на
`8e050f4`). A/B: capture на первом прогоне после рестарта — `r75-opmatrix`
2/12/2, 14 строк, `diff_rows = 0` против `r4-final`, `r35-multiline`,
`r3-final`; `base1` сам является no-op-снимком (0 строк), против него расхождение
законно и не является эталоном. Продакшн-код не менялся (только `tests/`).
AC22-24, macOS — `not verified`.

**Критерий приёмки R7.5:** наборы существуют, зелёные; числа прогона объяснены
(каждый новый провал разобран по коду, а не подогнан); A/B на сопоставимом входе
не хуже прежнего; явно перечислено, какие из пяти сценариев закрыты на подставной
фикстуре, а какие требуют модели (S20 частично, S18 частично) — «покрыто» без
указания среды не засчитывать.

Последний фактический прогон (AC25, 2026-10-01, R7.5): `suites=65 passed=2661
failed=1`, `TestSpecOperationMatrix` 216/216, провал предсуществующий и вне
области. A/B R7.5: `r75-opmatrix` = 0 против `r4-final`/`r35-multiline`/`r3-final`.
## Plan

- [x] **R7.5.** `TestSpecOperationMatrix` 216/216; S04/S05/S18/S20/S23 закрыты
  на подставной фикстуре. Критерий приёмки выполнен, кроме модели: S20 на
  отказе создания при записи и S05 на отбраковке невидимого источника помечены
  «не проверено» явно.
- [x] **R3-остаток:** `elements`, `selected`, `destinationReady`,
  `destinationParamGuidName` — все четыре в `SpecRuleRunState` (A1-A4).
- [x] **R7.6:** scaling по E измерен на фикстуре (`TestSpecReconcileScaling`),
  квадратичной составляющей нет. Абсолютная стоимость на модели - `not verified`.
- [ ] **R8** (`SpecExecutor`), R8.1-R8.6 - следующий, не начат.
- [ ] **R9** (оркестратор), **R10** (приёмка) - не начаты.

Закрытые R3.1-R3.4, R3-остаток, R4-R7.6, вынос в `SpecHelpers`, очистка комментариев,
независимая оценка с инвентарём R3 и дефект стенда #229 перенесены в
`IDEA_ARCHIVE.md` (разделы «Spec refactor #228 …»), вместе с цепочкой чекпоинтов.
R3.5 закрыт на подставной фикстуре; интеграционные части (UI, повторный запуск
на модели) открыты - см. «Статус актуальных шагов».

## Next Step — R8 SpecExecutor (актуально)

R7.6 закрыт: квадратичной составляющей нет. Ниже — следующий шаг по плану,
выбранный пользователем.

**Текущий шаг — R8.1 (выполнен 2026-10-02): фактическая последовательность
эффектов.**

Записано по исходникам, без перестановок. Границы транзакций — по
`ACAPI_CallUndoableCommand`:

| № | Этап | Где | Транзакция | ACAPI |
|---|---|---|---|---|
| 1 | выбор источников, плана чтения, забор избранного, разрешение назначения | `SpecArray`, :662-697 | — | `GetElementForPlaceProperties`, `GetElementByPropertyDescription` |
| 2 | `SpecDG` — диалог правил | :453-495 | — | (UI) |
| 3 | расчёт строк | `PlanRuleRows` | — | только чтение |
| 4 | сверка с размещёнными | `ReconcileExistingRows` | — | только чтение |
| 5 | **создание**: `GetElementForPlace` → `UnhideUnlockElementLayer` → `GetSizePlaceElement` → сбор `param` → запись GDL в memo → `ACAPI_Element_Create` | `PlaceElements`, :2936-3175 | `"Create Spec element"` (:2961) | `Element_GetDefaults`, `Attribute_Set`, `Element_Create` |
| 6 | группировка | :3036-3050 | **внутри транзакции 5** | `Grouping_CreateGroup`/`Grouping_Tool` или `ElementGroup_Create`/`Element_Tool` |
| 7 | GDL-скрипты | :3054-3062 | **ПОСЛЕ транзакции 5**, вторая не покрыта undo | `RunGDLParScript` или `Goodies(APIAny_RunGDLParScriptID)` |
| 8 | **удаление + запись свойств**: `ACAPI_Element_Delete` → `ParamHelpers::ElementsWrite` | :986-996 | `"Writing properties to created spec elements"` | `Element_Delete` |
| 9 | `SyncArray` | :1030 | — | внутри Sync |

**Что это меняет в плане R8 (поправка к оценке, первая версия была неверной).**
Ожидаемая в плане цепочка «…создание → группировка → GDL-скрипты → удаление/
запись свойств → Sync» совпадает с фактической. Уточнение по границам:
- группировка — **внутри** транзакции создания, выносить её наружу нельзя;
- GDL-скрипты — **после** транзакции, то есть этим изменением не покрыты
  (откат undo их не вернёт). Это существующее поведение, а не дефект
  рефакторинга, но его нельзя менять молча.

Первая версия этого раздела утверждала, что GDL тоже внутри транзакции.
Проверено по коду при R8.3: `CallUndoableCommand` закрывается на строке 3053,
цикл `RunGDLParScript` начинается на 3054 — то есть после. Ошибка была в том,
что R8.1 опирался на комментарий над функцией, а не на границы транзакции.

**Что делать не буду:** переставлять этапы ради схемы из плана, выносить
группировку из транзакции создания, втягивать GDL-скрипты внутрь транзакции
(это расширило бы объём отката), объединять транзакции 5 и 8 (изменился бы
объём отката при отказе записи — сценарий S20).

## R9 — оценка по инвентарю (2026-10-02)

Инвентарь фактических стадий `SpecArray` (`Spec.cpp:520-1048`) снят перед
выбором порядка шагов. Последовательность уже корректная: сбор правил
(`:166-193`) → план чтения и разрешение назначения (`:549-715`) → проверка
параметров приёмника (`:717-740`) → диалог `SpecDG` (`:743-751`) →
`ElementsRead` (`:753`) → `GetElementsForRule` (`:757-775`) → сбор
`paramOut` изменяемых (`:785-895`) → транзакция `Update spec elements`
(`:971-997`) → отчёт v2, GDL, `SyncArray` (`:1015-1048`).

**Вывод, меняющий оценку R9.1: переставлять нечего.** План требует, чтобы
`SpecArray` «последовательно вызывал подготовку, расчёт, сверку и исполнение» —
это уже так. Значит R9.1 в формулировке плана не меняет поведение; это
разбиение одной функции (`~530` строк) на несколько с явными границами,
то есть организация кода. Реальная ценность R9 — в R9.3 и R9.4.

**Поэтому порядок выбран обратным плановому: R9.3 → R9.4.** R9.3 даёт
защиту перед любыми перестройками, R9.4 даёт ответ, стоит ли продолжать.
Если инвентарь покажет отсутствие двойного движка и лишних чтений, R9.1
остаётся спорным — рефакторить работающий модуль без выигрыша смысла нет.

**R9.5 недостижим на имеющемся стенде:** требует S01-S28, а сценарии отказа
нельзя вызвать через JSON-порт (#226 — нет рабочего сеттера свойств). Будет
закрыт частично, как R7.5, с перечнем закрытых сценариев.

## R9.3 (выполнен 2026-10-02): контракт запуска в `spec/SpecCompat`

**Что было.** Имена полей ответа и входа жили литералами в
`SpecCommand::Execute`, перевод `GSErrCode` в строку статуса был инлайном, а
порядок четырёх счётчиков задавался последовательностью вызовов `Add`. Контракт
не был выражен нигде, и переименование поля ловилось бы только сравнением
прогонов на модели.

**Что сделано.** Новый продуктовый модуль `spec/SpecCompat.{hpp,cpp}`: имена
14 полей верхнего уровня, вложенных объектов счётчиков и списков, объекта
элемента, одного свойства, параметров входа и полей ошибки; `StageCounterNames[4]`;
`StatusText (GSErrCode)` как единственный перевод кода в строку. Порт берёт все
имена оттуда, счётчики собирает из массива `stages[4]` в порядке контракта.

**Модуль намеренно НЕ под `#ifdef TESTING`.** Первая версия была под ним, и это
было ошибкой проектирования: контракт обязан быть защищён в обычной сборке,
иначе адаптер был бы недостижим из порта, а набор проверял бы то, чего в
реальной сборке нет.

**Ошибка в первой версии проверки.** `VerifyResponseFields` сравнивала каждое
имя с литералом из того же модуля — тавтология, всегда зелёная, поймала бы
только опечатку в самом себе. Переписана на инварианты формы: имя непустое и
без разделителей `:`/`/` (иначе ломается разбор дампа в `Tools/spec_baseline.py`),
имена в группе попарно различны (`ObjectState::Add` отверг бы второе добавление,
и поле молча пропало бы), имя этапа не совпадает с полем верхнего уровня, обе
ветви статуса различимы.

**Валидация:** clang-format на 6 файлов; clangd 0 ошибок по всем четырём;
AC25 `Build succeeded!` (`SpecCompat.cpp` подхвачен без правок CMake — тот же
`file (GLOB_RECURSE)`); sweep AC26-29 `success`; runtime
`suites=69 passed=2810 failed=1`, дельта по 68 прежним наборам = 0, единственный
провал `TestConvertPropertyToParamValue` предсуществующий и вне области; набор
`TestSpecResponseContract` 18/18; A/B `r93-contract` `diff_rows=0` против шести
баз (`r85-counters`/`r84-writer`/`r83-executor`/`r82-builder`/`r8-base`/`r3-final`),
набор ключей ответа совпал с прежним. AC22-24, macOS — `not verified`.

**Границы применимости (важно для R9.1/R9.2).** Набор защищает форму ответа и
имена входа. Он НЕ ловит: перестановку стадий оркестратора; `required:
["placementPoint"]` в JSON-схеме (осталась литералом в порту); расхождение с
`Tools/spec_baseline.py`, если стенд начнёт ждать поля, которых контракт не
объявляет. То есть R9.3 защищает форму, но не защищает семантику стадий.

**Побочно найдено — ошибка в способе вызова стенда.** `spec_baseline.py compare`
НЕ принимает имя базы вторым аргументом: он всегда заново прогоняет Spec и
сравнивает с `raw-<tag>.json`. Я пять раз «сравнил» с пятью базами, и каждая
команда делала новый прогон, а эталоном был всегда мой тег. Отсюда
необъяснимые `2 -> 0`. Записано в карточку `Spec.md`, чтобы не повторить.

## R8.2 (выполнен 2026-10-02): общий builder записи параметров строки

Блоки для сравнения (оба собирают `param` для записи/дампа из строки `Element`):
- **create**: `PlaceElements` :2995-3050 — GUID-связь (`subguid_paramrawname`),
  носитель правила (`subguid_rulename`/`subguid_rulevalue`), затем выходные
  слоты, затем суммарные. Суммы дополнительно правят тип по
  `fromPropertyDefinition`.
- **update**: отдельного писателя нет, но блок подготовки ЕСТЬ — он в цикле
  `Spec.cpp:839-895` (сбор `param` для изменяемых объектов, `paramOut.Add
  (el.exs_guid, param)`), запись идёт позже общим
  `ParamHelpers::ElementsWrite (paramOut)`.

**Результат сравнения.** Построчное сравнение (нормализованные строки,
`difflib`) дало единственное расхождение — комментарий `// GDL параметры сразу
запишем в memo`. Содержательно блоки идентичны, значит общий builder
обоснован. Вынесен в `SpecHelpers` как `BuildRowParamToWrite (el,
paramToWrite, param)`, вызовов ровно два: `Spec.cpp:840` (изменяемые) и
`:2941` (создаваемые). Порядок записи сохранён, правка типа по
`fromPropertyDefinition` — только у сумм.

**Границы транзакций не тронуты** (это был главный риск R8.1): переносился
только код подготовки значений, ни один этап не двигался, обе границы
`ACAPI_CallUndoableCommand` на месте.

**Контракт зафиксирован набором** `TestSpecBuildRowParam` 17/17 — фильтр по
разрешённым именам, тип выхода не подменяется / тип суммы подменяется,
безымянный слот игнорируется, носитель правила не пишется при пустом значении,
перемешанная схема (сумма раньше выхода). Без него логика была бы перенесена
без защиты.

**Валидация:** clang-format на 5 файлов; AC25 `Build succeeded!`; sweep AC26-29
`success`; `suites=67 passed=2767 failed=1` (провал прежний, вне области);
A/B `r82-builder` `diff_rows=0` против пяти баз. Карточка `Spec.md` дополнена
разделом R8.2. AC22-24, macOS — `not verified`.

## R8.5 (выполнен 2026-10-02): фактические результаты этапов

**Что было.** `SpecRunResult` знал только `elementsToCreate/Modify/Delete`,
причём `elementsToCreate` считался как `paramOut.GetSize() - previousCount`,
то есть по факту дошедших записей. Отказ `ACAPI_Element_Create` при частичном
успехе не был виден нигде. `PlaceElements` возвращал `NoError` **всегда**;
вызывающий (`:921`) возвращаемое значение игнорировал, так что уточнение
безопасно.

**Что добавлено** (`Spec.hpp:324`): `SpecStageCounters {attempted, succeeded,
failed}` на этап — `create`, `grouping`, `gdl`, `deleteOld`; признаки
`hasPrimaryError` / `hasRecoveryError` / `hasUnconfirmedCreate`.

**Разделение первичной ошибки и ошибки восстановления — по факту порядка:**
- первичная: `GetElementForPlace` (`:2925`) и `ACAPI_Element_Create`
  (`:3020`) — до того, как элемент создан;
- восстановления: группировка (`:3057`) и GDL (`:3085`) — после создания,
  отказ там не откатывает уже созданный элемент; удаление (`:934`) — тоже
  после создания, поэтому отказ удаления отнесён к восстановлению.

**`hasUnconfirmedCreate` — отдельный флаг, а не вычисление из счётчиков.**
План R8.5 требует, чтобы «отсутствие подтверждения записи оставалось
unknown». Счётчики показывают факт вызова, а этот флаг — что подтверждения
нет; выводить его из `failed` нельзя, потому что «счётчик утверждает успех, а
подтверждения нет» — выразимое состояние. Тест закрывает именно эту
выразимость.

**Границы, зафиксированные в коде:**
- успех подтверждается возвратом вызова ACAPI, а не чтением модели обратно
  (read-back — приёмка, R10.6);
- `deleteOld` — один вызов на весь список, поэтому подтверждено либо всё, либо
  ничего, поштучный учёт невозможен;
- **внешний статус не менялся** — это F2, отдельное решение владельца.
  `PlaceElements` теперь возвращает `APIERR_GENERAL` при отказе создания, но
  вызывающий это значение по-прежнему игнорирует, то есть наблюдаемое
  поведение прежнее.

**Контракт закрыт набором `TestSpecRunCounters` 25/25:** нули по умолчанию,
независимость счётчиков этапов, тождество `attempted = succeeded + failed` на
пяти раскладах, независимость `hasUnconfirmedCreate` от счётчиков,
совместность `hasPrimaryError` + `hasRecoveryError`, сброс обнуляет счётчики и
сохраняет `includeDetails`.

**Чего набор НЕ проверяет (не выдано за проверенное).** Счётчики
заполняются внутри `PlaceElements`, который работает с моделью, поэтому
фактическое наполнение при отказе создания, отказе группировки и отказе GDL
на фикстуре недостижимо. Это `not verified` до прогона на модели.

**Валидация:** clang-format на 5 файлов; AC25 `Build succeeded!`; sweep AC26-29
`success`; `suites=68 passed=2792 failed=1` (провал прежний, вне области);
A/B `r85-counters` `diff_rows=0` против пяти баз; все ACAPI-вызовы и обе
границы `CallUndoableCommand` на месте (проверено инвентарём после правок).

## R8.3 (выполнен 2026-10-02): создание вынесено в SpecExecutor

**Что сделано.** Функция `PlaceElements` (237 строк с комментарием) перенесена из
`Spec.cpp` в новую пару `Sources/AddOn/spec/SpecExecutor.{hpp,cpp}` в
`namespace Spec`. Тело перенесено **построчно без изменений** — проверено
машинно: нормализованное сравнение с HEAD-версией даёт ровно одно расхождение,
закрывающую скобку функции. Новый набор не нужен: перенос не меняет поведения,
а `diff_rows` проверяет лишь возврат списков.

**Почему свободная функция, а не класс с состоянием.** У `PlaceElements` нет
состояния между вызовами: всё, что нужно, приходит аргументами (словари
строк, `paramToWrite`, `paramOut`, `startpos`, `runResult`), локальные
переменные живут только внутри тела. Класс с полями дал бы пустую обвязку.
Поэтому исполнитель — это отдельная единица компиляции с явным контрактом в
`.hpp`, а не объект. Это же отвечает на требование R8.3 «не кэшировать
изменяемый memo»: состояния, где memo можно было бы закэшировать, нет.

**Границы, перенесённые как есть и зафиксированные в контракте `.hpp`:**
- создание + группировка — внутри транзакции `Create Spec element`;
- GDL-скрипты — после транзакции, отказ скрипта не откатывает созданный
  элемент (существующее поведение, втягивать внутрь нельзя — расширило бы
  объём отката);
- memo индивидуален на объект, оба выхода чистят;
- счётчики R8.5 заполняются в тех же точках.

**Сборочная система подхватывает новые файлы сама:** `CMakeCommon.cmake`
использует `file(GLOB_RECURSE AddOnSourceFiles CONFIGURE_DEPENDS)`, правки
`CMakeLists.txt` не потребовалось. [по CMakeLists/Tools/CMakeCommon.cmake]

**Валидация:** clang-format на 3 файла; AC25 `Build succeeded!`; sweep AC26-29
`success`; `suites=68 passed=2792 failed=1` (провал прежний, вне области);
A/B `r83-executor` `diff_rows=0` и сводка без расхождений против пяти баз
(`r85-counters`/`r82-builder`/`r8-base`/`r76-scaling`/`r3-final`).

**Чего не проверяет A/B:** порядок вызовов ACAPI и границы undo — как и в R8.2,
это подтверждено чтением кода и построчным сравнением перенесённого тела, но
не прогоном. Наполнение счётчиков R8.5 на модели остаётся `not verified`.

## R8.4 (выполнен 2026-10-02): запись свойств и удаление вынесены в SpecExecutor

**Что сделано.** Блок транзакции `Writing properties to created spec elements`
(удаление устаревших строк + `ParamHelpers::ElementsWrite`) перенесён из
`Spec.cpp` в `SpecExecutor.cpp` как `WriteSpecProperties`. Тело лямбды
перенесено построчно: машинное сравнение с HEAD-версией даёт три расхождения —
две строки сигнатуры и локальный `GSErrCode err = NoError;`. Переименование
параметра `elements_delete` → `elementsDelete` в сравнении нормализовано, все
остальные строки совпадают.

**Единственное намеренное изменение поведения — как propagируется отказ
удаления.** Раньше `err` писался в общий накопитель ошибок функции запуска
`SpecArray`, и тот возвращался на строке 1000. Теперь этап возвращает
результат удаления, а вызывающий присваивает его своему накопителю:
`err = WriteSpecProperties (...)`. Без этого присваивания отказ удаления
перестал бы доходить до вызывающего — это отражено в контракте `.hpp`.

**Sync остался снаружи.** `SyncArray` вызывается после транзакции записи и в
`WriteSpecProperties` не переносился: он уже состоялся после создания, и его
включение в транзакцию записи изменило бы объём отката.

**Инвентарь после переноса:** в `Spec.cpp` не осталось ни
`ACAPI_CallUndoableCommand`, ни `ACAPI_Element_Delete`, ни `ElementsWrite` —
обе транзакции и оба этапа с моделью уехали в исполнитель. Это же подтверждает,
что вынос не оставил скрытых дублей.

**Валидация:** clang-format на 3 файла; AC25 `Build succeeded!`; sweep AC26-29
`success`; `suites=68 passed=2792 failed=1` (провал прежний, вне области);
A/B `r84-writer` `diff_rows=0` и сводка без расхождений против пяти баз
(`r83-executor`/`r85-counters`/`r82-builder`/`r8-base`/`r3-final`).

**Чего не проверяет A/B:** границы undo и порядок вызовов ACAPI — как в R8.2
и R8.3, подтверждены чтением кода и построчным сравнением, но не прогоном.

## S21 — ручная приёмка на модели (начата 2026-10-02)

Проводится вручную: пользователь меняет модель, агент вызывает Spec через JSON-порт
и сверяет ответ. Прогон `Tools/spec_baseline.py capture <tag>` - это РЕАЛЬНЫЙ запуск
Spec, он изменяет модель; выполнять только под конкретный сценарий.

**Ограничение стенда, обнаруженное при приёмке.** Блокировка элементов в ArchiCAD -
состояние рабочей области, а не атрибут `.pln`: она не переживает перезапуск. Любой
прогон `restart_archicad_for_test.ps1` её снимает, поэтому сценарии с блокировкой
надо воспроизводить в каждой сессии заново.

**S21 — НАХОДКА: заблокированные строки исчезают из отбора и порождают дубликаты.**
Прогон `r86-s21-locked2` (35 строк спецификации заблокированы):

| Поле ответа | Значение |
|---|---|
| `status` / `resultCode` | `completed` / `0` |
| `elementsToCreate` | 35 |
| `elementsToModify` | 0 |
| `elementsToDelete` | 0 |
| `create` | attempted 35, succeeded 35, failed 0 |
| `prepareFailureStage` | `none` |
| `deleteOld` | attempted 0, succeeded 0 |

То есть отчёт утверждает «создано 35, ошибок нет», а по факту 35 исходных строк
заблокированы и не тронуты, и созданы 35 новых поверх них. `deleteOld.attempted = 0`
и `elementsToModify = 0` - прямое доказательство, что ни одна существующая строка не
была найдена для сопоставления.

**Механизм (подтверждён по коду и заголовку SDK).** `GetElementByPropertyDescription`
(`CommonFunction.cpp:2689-2693`) фильтрует `APIFilt_OnVisLayer | APIFilt_IsVisibleByRenovation
| APIFilt_IsInStructureDisplay | APIFilt_IsEditable | APIFilt_InMyWorkspace |
APIFilt_HasAccessRight`. `APIFilt_IsEditable = 0x00000001` (`APIdefs_Elements.h:205`)
отбрасывает заблокированные элементы. Далее `Spec.cpp:691-693` получает список без
заблокированных строк, `SelectExistingElements` даёт пустой `runState.exsist_elements`,
и `Spec.cpp:694` делает `continue` - сверка существующих строк не выполняется вовсе,
движок считает, что размещённых строк нет. [по коду + заголовку SDK]

**Это НЕ регрессия R8.** `CommonFunction.cpp` рефакторингом R3/R7/R8 не затрагивался
(`git diff` пуст на всём интервале). Поведение пре existed.

**Отсутствующая защита.** Отчёт не содержит признака «существующие строки не найдены,
хотя должны были быть». Ни один счётчик этого не показывает, а `status=completed`
подтверждает успех. Это ровно случай «ложное "всё записано" во внутреннем отчёте» из
плана S21. Решение о том, считать ли заблокированную строку несуществующей (текущее
поведение) или отказом с явным сообщением, - **поток F1, решение владельца**.

**Оформлено в issue #243** (баг, F1-вопрос владельцу приложен).

**Что НЕ удалось установить.** Два прогона (`r86-s21-dup2`, `r86-s21-dup3`) дали
`APIERR_GENERAL` (-2130313215 = `0x81060001` = `APIErrorStart + 1`) при всех нулевых
счётчиках, то есть отказ подготовки. Причина не установлена: `msg_rep` пишет в отчёт
Archicad, который недоступен извне. Для этого добавлено поле `prepareFailureStage`
(см. ниже), но поймать отказ на модели не удалось - воспроизводился два раза подряд,
после перезагрузки Archicad и повторной блокировки больше не возник. `not verified`.

**Сбой моей методики.** Гипотезы о причине отказа («параметры для чтения не найдены»,
«список элементов пуст») я строил по коду, не по факту. Помечено `not verified`.
Отсюда `prepareFailureStage` - чтобы этап отказа был виден через порт, а не
угадывался.

## S21 — приёмка 2026-10-02 (после фикса #243)

**Дефект устранён.** При 23 заблокированных строках прогон `r86-s21-locked-fix`
дал `create=4` вместо 35. Созданные 4 - легитимные строки нового материала
«Линеарные панели Primepanel S» (`АР/Ф`), по одному источнику каждая, следствие
правки модуля владельцем; дубликатов вида «35 строк поверх 35 заблокированных» нет.
Отладчиком на тех же заблокированных строках подтверждено: `existingCandidates=35`,
`PrepareElementsForUpdate` вызван, `APITool_Unlock` без отказа. Прежний код отбрасывал
их фильтром `APIFilt_IsEditable` и получал пустой список.

**Счётчики R8.5 — закрыт пункт `not verified`.** Наполнение наблюдено на модели в
четырёх прогонах (`r86-m1-clean`, `r86-s21-noop`, `r86-m3-manual`, `r86-m4-locked`):
| прогон | create | grouping | gdl | deleteOld |
|---|---|---|---|---|
| `m1-clean` (36 строк с нуля) | 36/36/0 | 1/1/0 | 36/36/0 | 0/0/0 |
| `s21-noop` (обновление) | 3/3/0 | 1/1/0 | 3/3/0 | 6/6/0 |
| `m3-manual` (no-op) | 0/0/0 | 0/0/0 | 0/0/0 | 0/0/0 |
Инвариант `attempted = succeeded + failed` верен во всех четырёх, `create.attempted`
равен `elementsToCreate`, `deleteOld.attempted` равен `elementsToDelete`. Флаги
`hasPrimaryError`/`hasRecoveryError`/`hasUnconfirmedCreate` везде `false`,
`prepareFailureStage=none`. Тождество создания и удаления с объявленными числами на
модели подтверждено — раньше это было видно только на фикстуре.

**Отказ `readParamsNotFound`: воспроизводимость и почему причина не найдена.**
Тот же отказ дали `r86-m2-locked` и `r86-m2-retry` (дважды подряд, сразу после чистого
старта с блокировкой). После перезапуска Archicad и повторной блокировки отказ не
воспроизвёлся: `r86-m4-locked` дал `completed` при всех нулевых счётчиках, а
отладчик показал по цепочке отказа этого нет - `readContext.read.GetSize()=229` (не
пуст), и все три признака пригодности правила истинны (`parseValid`,
`runState.selected`, `runState.destinationReady`).
Перезапуск изменил и состояние модели, и состояние процесса одновременно, поэтому
фактор не выделен: **причина остаётся `not verified`**, и две мои прежние версии
(блокировка слоёв; чтение правил из элементов модели) опровергнуты фактами.
В отладчике видно `guidArray=[0]`, то есть список элементов для чтения правил пуст -
правила в этом пути берутся не из элементов модели, и блокировка на них не влияет.

**Разъяснено сообщение «Create spec from default element».** Оно не означает потерю
избранного: ветка выбирается по пустому выделению (`Spec.cpp:166-193`), а
`GetRuleFromDefaultElem` читает правила из default element
(`ACAPI_Element_GetPropertyDefinitionsOfDefaultElem`, `Spec.cpp:45`), избранное не
читает вовсе. Избранное используется как ПРИЁМНИК результата, а не источник
правил, и найдено верно - `destinationReady=true` в отладчике.

## Next Step — R8.6: приёмка R8 (продолжение)

R8.1-R8.5 закрыты кодом. R8.6 требует прогонов на модели (S16, S20-S24):
ошибки подготовки, отказ создания, отказы последующих этапов, восстановление
состояния группировки.

**Что для этого нужно и чего в репозитории нет.** A/B по `diff_rows` сравнивает
возвращённые списки и не видит порядок вызовов ACAPI; наборов, покрывающих
последовательность эффектов, нет. Сценарии отказа нельзя вызвать через JSON
порт — рабочего сеттера свойств в нём не существует (#226). Поэтому часть R8.6
закрыть на стенде нельзя в принципе, и называть R8 закрытым по имеющимся
инструментам нельзя.

**Пункты `not verified`, накопленные за R8:**
- ~~наполнение счётчиков R8.5 при реальных созданиях~~ — **закрыт 2026-10-02**,
  наблюдено на модели в четырёх прогонах, инварианты выполняются (см. S21);
- наполнение счётчиков при **реальных отказах** создания, группировки и GDL —
  отказать этап на модели не удалось, поэтому ветки `failed > 0` не наблюдены;
- фактический объём отката при отказе записи свойств (S20) — границы транзакций
  подтверждены чтением кода, но не наблюдением отката;
- поведение GDL-скриптов при их отказе, выполняющихся уже после транзакции.

Публичная семантика ошибок по-прежнему не менялась — это поток F2.

**Границы доказательств A/B (действуют для R7.5-R7.6 и приёмки).**
`diff_rows` НЕ является проверкой:
- множественности строк с одинаковым ключом — `collect_rows` схлопывает их в одну
  запись (`Tools/spec_baseline.py:152-169`);
- состава источников по GUID — сверяется только ЧИСЛО источников (`:161,167`);
- конечного состояния модели — сравниваются только возвращённые списки;
- GUID существующих объектов — в сравнении их нет.

`rows.update` при соединении `created` и `modified` (`:257-259`) перекрывает
общий ключ: побеждает последний раздел, поэтому строка из `created`, имеющая тот же
ключ, что и в `modified`, из результата выпадет.

Поэтому нулевой `diff_rows` записывать как доказательство множественности строк,
состава источников или состояния модели **нельзя** — только как «значения и имена
свойств видимых строк не изменились». Для скорости (R7.6) и приёмки (R10.6)
остаётся **отдельный измеренный gate**: время, память, число дорогих вызовов
(`ElementsRead`, создание объектов, запись свойств, GDL/Sync). Подсчёт проходов по
коду этот gate не заменяет.

## Next Step — R9.2 переопределён: единое окно результата вместо всплывающих окон

R9.3 закрыт (`92fdbf7`), R9.4 закрыт (`fbc5162`), R9.1 закрыт как
осознанно не сделанный — стадии в `SpecArray` уже идут последовательно,
оставалось только разбиение функции на части без изменения поведения.

R9.2 переформулирован владельцем: это не перенос UI внутрь оркестрации, а
новое поведение. Требование: убрать все всплывающие окна запуска и свети
показ в ОДНО окно — существующий диалог выбора правил (`dialogs/DG4rule.*`),
показываемый повторно после выбора правил и точки. Колонки «создано /
изменено / удалено» построчно по правилу; правило с ошибкой подсвечивается
красным, внизу окна — `[ИМЯ ПРАВИЛА] — текст ошибки`. Ошибки без правила
(правила не найдены, параметры записи не найдены, элементов с ошибкой много)
идут вниз окна без привязки к строке. Окно только показывает, повторный
запуск из него не запускается. Per-rule статистика отдаётся также в
JSON-ответе, за `includeDetails`.

Что уже есть и переиспользуется (проверено по коду): окно умеет цвет
строки (`RuleSelectData::color`), текст внизу красным (`is_warn` + `TextBox`),
произвольное число колонок (`SetTabFieldCount` в рантайме, вёрстка в
`AddOn.grc` править не нужно). Чего не хватает: счётчики в `SpecRunResult`
суммарные (разложить per-rule можно — у элемента есть `subguid_paramrawname`),
ошибки хранятся как GUID и имена полей, а не как текст «правило → сообщение».

Окно общее для `ReNum.cpp` и `Summ.cpp` — расширение не должно ломать их.

Issue: #245 (kuvbur/AddOn_SomeStuff) — «Spec: единое окно результата запуска
вместо всплывающих окон», создан 2026-10-02. Требования и разобранные
переиспользуемые точки окна зафиксированы в нём.

**Следующий по плану — R9.5**, и он на имеющемся стенде недостижим (#226):
требует S01-S28, а сценарии отказа нельзя вызвать через JSON-порт. Предлагаю
закрыть частично с перечнем закрытых сценариев, как R7.5, либо сначала R9.2.
Решение за владельцем.

## Last Checkpoint

- **R9.4:** инвентарь по коду, кода не менялось. Каждая расчётная функция
  вызывается ровно один раз; `plan == nullptr` в рабочем пути, то есть
  сериализации плана нет; `ElementsRead` один (второй относится к валидатору
  #234). Ответ отрицательный по всем трём пунктам критерия. Граница: инвентарь
  доказывает отсутствие второго прохода в исходниках, но не измеряет число
  вызовов внутри `ParamHelpers`. Refs: #228.

## Last Checkpoint

- **R9.3:** контракт запуска вынесен в `spec/SpecCompat.{hpp,cpp}` — имена полей
  ответа и входа, `StatusText` как единственный перевод кода в строку, порядок
  счётчиков по `StageCounterNames`. Первая версия проверки была тавтологией
  (имя против литерала из того же модуля) — переписана на инварианты формы;
  модуль переведён из-под `TESTING`, иначе был бы недостижим из порта.
  Набор `TestSpecResponseContract` 18/18. A/B `r93-contract` = 0 против шести баз,
  sweep AC26-29 success, `suites=69 passed=2810`. Refs: #228.
- **R8.2:** общий builder `BuildRowParamToWrite` в `SpecHelpers` вместо двух
  идентичных копий (построчное сравнение: расхождение только в комментарии);
  контракт закрыт набором `TestSpecBuildRowParam` 17/17; границы undo не
  тронуты. A/B `r82-builder` = 0 против пяти баз. Refs: #228.
- **R8.4:** запись свойств и удаление устаревших строк вынесены из `Spec.cpp`
  в `SpecExecutor` как `WriteSpecProperties`; отказ удаления теперь
  возвращается этапом и присваивается накопителю вызывающего. Sync остался
  снаружи. Refs: #228.
- **R8.3:** `PlaceElements` вынесена из `Spec.cpp` в новую пару
  `SpecExecutor.{hpp,cpp}`; тело перенесено построчно (машинное сравнение с
  HEAD: одно расхождение — скобка). Вынесена свободная функция, а не класс:
  состояния между вызовами нет. Refs: #228.
- **R8.5:** счётчики фактических результатов этапов (`SpecStageCounters` на
  create/grouping/gdl/deleteOld) + `hasPrimaryError`/`hasRecoveryError`/
  `hasUnconfirmedCreate`; `PlaceElements` больше не возвращает `NoError`
  вслепую. Внешний статус не менялся (F2). Набор 25/25. A/B = 0 против пяти
  баз. Refs: #228.
- **R8.1:** записана фактическая последовательность эффектов (9 этапов, 2 границы
  транзакций). Поправка `cdb9918`: группировка внутри транзакции, GDL — после
  неё. Refs: #228.
- **Ревью #228, исправления `c32dfed` (Refs: #237-#242).** Найдено 10 находок,
  из них одна регрессия поведения данных (#237, first-match → last-match при
  выборе носителя GUID) и восемь P2 (план сверки, граница сумм, независимость
  эталона, четыре ложных PASS стенда A/B), одна P3 — текст IDEA.md подтверждён
  кодом, правка не потребовалась. Валидация: clang-format, clangd 0, AC25 Debug
  `Build succeeded!`, runtime `suites=66 passed=2750 failed=1` (предсуществующий
  `TestConvertPropertyToParamValue`), Tools 20/20 — 11 из них падают на прежнем
  коде стенда, то есть тесты доказывают ловлю дефекта. Все шесть issues закрыты.
  Тем же коммитом закрыт **#229** (compare возвращает успех при несовпадении
  summary) — это один и тот же код выхода comparator-а; сценарий из issue
  воспроизведён после правки: код возврата 1, `PASS` не напечатан.
  Открыты и **не входят** в это ревью: #236 (политика F1, требует решения
  владельца), #230, #231, #232, #233.
  Попутно исправлены два ошибочных теста в блоке R7.6 (`existed` сравнивался с
  предельным числом строк; `outParam.GetPtr ("Alpha")` искал ключ без
  разделителя ATSIGN) — они давали 32 ложных провала, а не дефект кода.
- R7.6 закрыт: `TestSpecReconcileScaling` 22/22, ratio при удвоении E
  1.96-2.16 (линейно), квадратичной составляющей нет. Измерения в
  `Reviews/spec-refactor-baseline/scaling-r76.{csv,json}` (gitignored, числа
  скопированы в IDEA). A/B `r76-scaling` = 0 против пяти баз, sweep AC26-29
  success. Refs: #228.
- R3-остаток закрыт (A1-A4): `elements`, `selected`, `destinationReady`,
  `destinationParamGuidName` — все в `SpecRuleRunState`; `SpecRule` без полей
  состояния запуска. A/B `r3a4-favorite` = 0 против шести баз, sweep AC26-29
  success. Refs: #228.
- R3.A3 — `selected` перенесён в `SpecRuleRunState`, `IsRunnableForRun` на
  `runState.selected`; A/B `r3a3-selected` = 0 против пяти баз,
  `TestSpecSelectionPolicy` 65/65 без правок ожиданий. Refs: #228.
- R3.A2 — `elements` перенесён в `SpecRuleRunState`; карточка `Spec.md`
  раздел R3.A2. A/B `r3a2-elements` = 0 против четырёх баз,
  `suites=65 passed=2661 failed=1` (предсуществующий провал), sweep AC26-29
  success. Refs: #228.
- `5e8db79` - R7.5: `TestSpecOperationMatrix` 216/216; issue #236.
- `8e050f4` - валидатор правила по GUID (#234) — предыдущий чекпоинт,
  закрывает параллельную задачу; перед ним её код лежал в дереве.
- `5054d02` - очистка комментариев тестов: сняты метки `R#.#`/`S##` из описаний
  `DBtest` и переписаны блоки комментариев в 8 файлах `Sources/AddOn/tests/`.
  Код не изменён (сверка HEAD с вырезанными комментариями и литералами дала
  идентичный текст), `clang-format` выполнен, clangd без новых ошибок,
  AC25 Windows Debug `Build succeeded!`; runtime не запускался - менялись только
  комментарии и текст меток.
- `c2d0ab4` - очистка комментариев модуля spec (`Spec.cpp/.hpp`,
  `SpecPlanning.cpp/.hpp`, `Spec_libpart.cpp`), Refs: #228.
- `6ba2bea` - перенос `GetSizePlaceElement`, `ParamValueToDumpString`,
  `FillDumpFromParamDict`, `FillDumpGDLParameter` в `spec/SpecHelpers`
  (новый модуль); попутно закрыты 4×`TODO : вынести в SpecHelpers`.
  A/B `sh-final` против шести эталонов = `diff_rows` 0.
- `274985d` - R3.5 доп.: `TestSpecSelectionPolicy` 65/65 (второе реальное
  многострочное описание владельца), A/B `r35-multiline` = 0 против семи баз,
  sweep AC26-29 success, `suites=63 passed=2402 failed=1` (предсуществующий
  провал `TestConvertPropertyToParamValue`).
- Полная цепочка чекпоинтов R3-R7 - `IDEA_ARCHIVE.md`.

## Decisions

- **R3-инвентарь — предварительный**, перенесён в `IDEA_ARCHIVE.md`. Пересечения подтверждены по коду, но
  полный перечень потребителей не собран, поэтому конкретная схема разделения
  `SpecRule` не объявляется проверенной. Перенос поля раньше инвентаря читается
  до заполнения (молча, и только на части путей). Записано после ревью владельца
  2026-10-01.
- Эталоны хранятся в gitignored `Reviews/spec_baseline/`; `Reviews/` не коммитится.
- Семантический ключ строки = favorite + значения без GUID-полей и «имени правила»;
  новые GUID не сравниваются, дубли ключей схлопываются с суммированием источников
  (известное ограничение стенда, зафиксировано здесь).
- **`diff_rows` не доказывает** множественность строк, состав источников по GUID,
  GUID существующих объектов и конечное состояние модели; при соединении
  `created` + `modified` общий ключ перекрывается (`spec_baseline.py:257-259`).
  Доказательство скорости/памяти — отдельный измеренный gate (R7.6, R10.6).
  Записано после ревью владельца 2026-10-01.
- `TestFunc::Test()` остаётся включённым при каждом первом открытии проекта (решение
  владельца из предыдущей задачи) — в P0 не менялось.
- **Эталонный проект восстановлен из авто-бэкапа.** `Test_file/test_25.pln` НЕ в
  git (в индексе только `test_22/23/24/26/27/28/29.pln`). 29.09 около 01:07
  проект был сохранён, sha256 сменился `db1690f…` -> `c84b6a6f…`, и все A/B стали
  давать 13/2/0 и 15 строк вместо 2/12/2 и 14 — расхождение выглядело как
  регрессия, но воспроизводилось на чистом HEAD. `Test_file/test_25.bpn` (28.08
  22:04, 56 792 080 байт) содержал ровно прежний `db1690f…`; восстановлен через
  `cp -p`. Изменённый файл отложен как `Test_file/test_25.pln.saved-130747`
  (untracked, не коммитить). **Перед новой регрессионной сессией:** сверить
  `sha256sum Test_file/test_25.pln` с `db1690f…`; при расхождении A/B недействителен.
- Утверждение skill `archicad-plugin-build` о том, что проект не сохраняется при
  закрытии, было неверным — исправлено; там же добавлено правило проверять sha256
  фикстуры при любой аномалии и восстанавливать из `.bpn`.
- `PlaceElements`-сценарий считать эталоном только на ПЕРВОМ прогоне после
  рестарта; второй прогон на той же модели даёт законный no-op 0/0/0.


## #228 — рефакторинг Spec, пошаговый без регрессий (ЗАКРЫТ 2026-10-02)

**Issue #228 закрыт решением владельца 2026-10-02 как «рефакторинг выполнен».**
Это осознанное отступление от R10.8 плана («закрывать только по действительно
выполненной приёмке»): владелец принял закрытие с явно названной границей
доказательств, а не после доведения R8.6/R10. Непроверенное перечислено ниже и
не считается выполненным.

**Что не проверено (перенесено в закрытый issue как `not verified`):**
- **R10.6 — измеренный performance-gate.** Не выполнялся. Все A/B этого
  рефакторинга сделаны по `diff_rows`, а сам план прямо говорит, что `diff_rows`
  не заменяет измеренный gate (архив, Decisions). Требование «бюджета +5% за
  чистую архитектуру нет» **не проверено**.
- **R8.6 — приёмка отказов на модели.** `failed > 0` не наблюдался ни разу за
  все прогоны; пути отказа создания, группировки и GDL-этапа на живой модели не
  исполнялись. Часть сценариев недостижима на порту AC25 (нет GDLParameter-команд,
  нет команд записи описаний свойств → второе правило завести нельзя).
- **Матрица версий:** AC25–29 `success` (сборка), AC22–24 и macOS —
  `not verified`. Полного runtime-набора на AC26–29 нет.
- **F1** (политика разрушительных действий при неполных данных) и **F2**
  (достоверный внешний статус ошибок) планом выведены из R и **не решались**;
  F1 остаётся в #236.

**Незакрытым кодом это не отменяет:** если позже понадобится perf-база движка
Spec или матрица AC22–24/macOS — это новая задача с новым baseline, а не
пункт #228.

## Task — рефакторинг Spec (#228)

Issue: #228 (kuvbur/AddOn_SomeStuff) — рефакторинг; #227 — дамп значений
элементов.
План: `Reviews/2026-09-27_174500-spec-refactor-no-regression.md`.

## Scope

Пошаговый рефакторинг движка спецификаций по плану (issue #228). Файлы:
`Sources/AddOn/spec/Spec.cpp/.hpp`, `SpecPlanning.cpp/.hpp`,
`SpecExecutor.*`, `SpecCompat.*`, `SpecHelpers.*`,
`Sources/AddOn/tests/TestSpec.cpp`, `tests/TestFunc.cpp/.hpp`,
`Docs/modules/spec/Spec.md`, `IDEA.md`.
Эталоны A/B в gitignored `Reviews/spec_baseline/`.

**Вне scope (действует):** `Helpers.cpp` (26 включений, под замком),
потоки **F1** (политика разрушительных действий при неполных данных) и **F2**
(достоверный внешний статус) — планом выведены из R и требуют отдельного
согласования политики владельцем.

Проверочная версия AC25 Windows Debug. AC26–29 — только сборка (sweep),
AC22–24 и macOS — `not verified`.

## Статус актуальных шагов

| Шаг | Состояние | Основание |
|---|---|---|
| R3.1–R3.5 | закрыты | чекпоинты `1592687`, `5d8d281`, `c7368f9`; цепочка — в архиве |
| R3-остаток | закрыт | A1–A4, все четыре поля в `SpecRuleRunState` |
| R4 | закрыт | `TestSpecParseError` 71, `TestSpecParser` 318 |
| R5 | закрыт | 8 чтений в `SpecPlanning.cpp`, 0 в `Spec.cpp` |
| R6 | закрыт | `TestSpecEngineEquivalence` 49/49 |
| R7.1–R7.6 | закрыты | чекпоинты `b6d2fd9`…`a8af4a3`, `TestSpecReconcileScaling` 22/22 |
| R8.1–R8.5 | закрыты кодом | `b6d2fd9`, `b4add8d`, `61d8a51`, `b83a239`, `42f2d75` |
| **R8.6** | **не проверено, issue закрыт** | приёмка на модели; часть сценариев отказа недостижима на стенде |
| R9.1 | закрыт как осознанно не сделанный | стадии `SpecArray` уже последовательны (`fbc5162`) |
| R9.2 | закрыт | #245 закрыт: код `0657dfd`, приёмка `db5ecfa`; всплывающих окон в `spec/` не осталось |
| R9.3 | закрыт | `92fdbf7`, `SpecCompat` |
| R9.4 | закрыт | `fbc5162` |
| R9.5 | **закрыт частично 2026-10-02** | прогон `verified: 9 / not verified: 0`; отказ GDL недостижим — в порту нет GDLParameter-команд; матричные наборы удалены, стенд переведён на живую модель |
| **R8.6** | **не проверено, issue закрыт** | `failed > 0` не наблюдался ни разу; часть сценариев недостижима на порту AC25 |
| **R10** | **не проверено, issue закрыт** | R10.6 (performance-gate) не выполнялся; матрица AC22–24/macOS — `not verified` |

## Plan

- [x] **R9.2 — последнее всплывающее окно убрано, #245 закрыт.** `GetRuleFromDefaultElem`
  (`Spec.cpp:127-149`) больше не открывает `ACAPI_WriteReport`: текст отказа
  «все флаги выключены» пишется в накопитель через `AddGeneralMessage`.
  Требовалось перенести создание накопителя выше вызова — `SpecAll` создаёт
  его на `:164`, до `GetRuleFromDefaultElem` (`:202`); ранний выход
  «выделение пусто, ни выделения, ни включённых элементов» (`:205-211`) —
  единственный выход, минущий `SpecArray`, поэтому перед возвратом там тоже
  вызывается `ShowRunResult` (`:224`), иначе сообщение осталось бы невидимым.
  Накопления результата на этом пути не происходит: `SpecArray` обнулил бы
  накопитель, а до него сообщение об отказе дойти не может. Возвращаемое
  значение прежнее — `NoError`. В `spec/` `ACAPI_WriteReport` не осталось
  вовсе (проверено grep). Приёмка: AC25–29 `success`, runtime
  `suites=70 passed=2860 failed=1` — единственный провал
  `TestConvertPropertyToParamValue` был ошибочным ожиданием теста (#232),
  отдельно исправленным после этой приёмки; к spec/
  отношения не имеет; `TestSpecRunReport` 29/29, `TestSpecResponseContract`
  26/26, `TestSpecRunCounters` 25/25. Код `0657dfd`, приёмка `db5ecfa`.
  Не проверено вживую: вёрстка окна и сценарий «пустое выделение + все флаги
  выключены» на стенде не воспроизводятся.
  Правку класть в `git stash` нельзя — HEAD двигает параллельная сессия.
- [/] **R8.6 — приёмка R8 на модели.** Открытые пункты `not verified`, все
  перечислены в архиве блока «Next Step — R8.6»: отказы создания, группировки
  и GDL на модели не наблюдались (`failed > 0` не наблюдалось ни разу);
  объём отката при отказе записи свойств (S20) не наблюдался.
- [x] **R9.5 — закрыт частично 2026-10-02, прогон `verified: 9 / not verified: 0`.**
  Матричные наборы `TestSpecScenarioMatrix` и `TestSpecOperationMatrix`
  удалены (определения + объявления): они гоняли сценарии в памяти, без
  модели, и не могли проверить ни создание, ни удаление, ни восстановление.
  Вместо них стенд `Tools/spec_scenarios.py` работает с живой моделью.
  Наборы `TestSpecReconcileFixtures`, `TestSpecChangePlan`,
  `TestSpecReconcileScaling`, `TestSpecRowSlots` сохранены — чистая логика
  плана модели не требует. Runtime: `suites=67 passed=2475 failed=0`,
  матриц в отчёте нет.
  **Три вывода, установленных на прогоне, а не предположенных:**
  1) Строка спецификации ГРУППИРУЕТСЯ, поэтому отключение одного элемента не
     обязано удалять строку — строка исчезает только когда отключены все её
     источники. Гарантированный delete даёт элемент, который является
     единственным источником строки. Проверено на элементе
     `0CD3341E-1D44-41C9-80FE-5C916DF09F17` (уникальный материал, 1 источник).
  2) `created`/`modified` в ответе — это список ОПЕРАЦИЙ, а не снимок
     таблицы: неизменившаяся строка в ответ не попадает вовсе. Наш прежний
     критерий «строка изменилась» читал отсутствие как изменение.
  3) Шаг восстановления обязан идти ПОСЛЕ возврата флага: пока флаг
     выключен, источник не участвует в расчёте и строка не появляется
     (`C/M/D=(0,0,0)`), после возврата — `C=1`. То есть потеря объекта
     обратима следующим запуском Spec: это пересчёт, а не Undo.
  Наблюдённые значения прогона: 1-й `C/M/D=(36,0,0)`, `create 36/36` без
  отказов; 2-й `(0,0,1)`, `deleteOld attempted=1 succeeded=1 failed=0`;
  3-й `(1,0,0)` — удалённая строка вернулась.
  **Не достигнуто, закрыто как `not verified`:** отказ GDL-этапа — в порту
  AC25 нет GDLParameter-команд, вызвать нечем. Второе правило завести нельзя
  (правило = описание свойства `Spec_rule`, команды записи описаний на порту
  нет). AC22–24, macOS — `not verified`.
- [ ] **R10 — приёмка.** Включает отдельный измеренный performance-gate:
  `diff_rows` его не заменяет (границы доказательств — в архиве).

## Next Step

**R9.5 закрыт частично 2026-10-02 — см. Plan. Актуальный стенд:**
`python Tools/spec_scenarios.py cycle --singleton <GUID>`, где `<GUID>` —
элемент, являющийся ЕДИНСТВЕННЫМ источником строки (иначе отключение даёт
пересчёт суммы, а не удаление). Стенд сам создаёт строку, отключает флаг,
сравнивает со снимком, возвращает флаг и проверяет, что строка вернулась.
Отчёт: `Reviews/spec-scenarios/cycle.json`, снимок — `first-run.json`.

**Сеттер свойств через JSON-порт РАБОТАЕТ — подтверждено на живом AC25.**
записано обратное («отвечает success, но не пишет»); это было следствие вызова
с неверным именем поля и пустым массивом. Подтверждено на живом AC25
read-modify-write с восстановлением:

```
API.GetPropertyIds            {"properties":[{"type":"UserDefined","localizedName":[…]}]}
                               → {"propertyId":{"guid":"…"}}        # мост: имя → GUID
API.GetPropertyValuesOfElements {"elements":[{"elementId":{"guid":"…"}}],
                                   "properties":[{"propertyId":{"guid":"…"}}]}
                               → propertyValuesForElements[].propertyValues[].propertyValue
API.SetPropertyValuesOfElements {"elementPropertyValues":[{"elementId":…,
                                    "propertyId":…,
                                    "propertyValue":{"type":"boolean","status":"normal","value":false}}]}
                               → {"executionResults":[{"success":true}]}
```

Форма записи bool: записали `false` → прочитали `false` → вернули `true` →
сверили `true`. Ошибки по элементам: 4008 (тип значения не совпал с
определением), 7204 (нет элемента), 6701 (нет определения свойства), 4007
(enum-значение не найдено по displayValue). Частичный отказ не роняет команду —
в `executionResults` отчёт по каждому элементу.

Ограничения порта AC25: `API.Undo`/`API.Redo` **отсутствуют** (2002) — откат
только явным восстановлением значений; GDLParameter-команд нет, поэтому отказ
GDL-этапа не воспроизвести; `GetAllPropertyIds*` отсутствуют, GUID берутся
поиском по имени. `API.GetProjectInfo`/`API.GetVersion` отсутствуют — определить
версию порта нельзя, версия AC25 известна из сессии. Формы параметров в схеме
AC29 местами отличаются от AC25 — схему читать как ориентир, а не спецификацию.

Сценарии `create с нуля`, `update`, `delete_old` автоматизируемы
(с обязательным восстановлением исходных значений) — R9.5 на этом закрыт.

**R8.6 остаётся открытым:** для его прогонов нужен принудительный отказ
этапа, а отказ GDL по-прежнему нечем вызвать (GDLParameter-команд в порту
нет). Требует решения владельца либо остаётся `not verified`.

Открытые вопросы по #228, требующие владельца:

- **F1** (#236) — диагностика РЕШЕНА, политика НЕ решена. Правильная причина
  удаления введена: `RowRejectedByCalc` (строку отбросил расчёт — проверка схемы
  либо отказ правила) отделена от `RowAlreadyClaimed` (строку забрал другой
  объект). Ключи строк, отброшенных расчётом, копятся в
  `GS::HashSet<GS::UniString> rejected_keys` и передаются в сверку. Само
  удаление размещённых строк при неполных данных НЕ менялось — это отдельное
  решение владельца.
- **F2** — достоверный внешний статус ошибок; сейчас публичная семантика не
  менялась.

## Last Checkpoint

- **#234 (мост палитры)** — `2101967`: вкладка «Спецификация» получила
  `GetSpecRuleProperties` и `CheckSpecRuleByPropertyGuid`. Ответ — строка JSON,
  а не `DG::JSObject`: вложенные объекты CEF при передаче в JS теряются.
  Проверяются и два служебных носителя назначения (`spec_rule_name`,
  `sync_guid`). Refs: #234.
- **#245 (R9.2, закрыт)** — `8636bab` (накопитель `SpecMessage`/`SpecRuleStats`,
  фактические счётчики, одно окно результата, `rules`/`messages` в JSON),
  `0657dfd` (остаток: последнее всплывающее окно `GetRuleFromDefaultElem`
  убрано, накопитель перенесён выше вызова, показ добавлен в ранний выход),
  `db5ecfa` (приёмка: AC25–29 `success`, runtime `suites=70 passed=2860
  failed=1`, единственный провал — #232). Refs: #245.
- **#243** — `0b3ba1c` (разблокировка существующих строк), `7cfcffa` (одна
  запись Undo на операцию), `aec16a0` (одна транзакция на изменяющую часть).
  Отсутствующая защита «существующие строки заблокированы» добавлена в
  `8636bab`: `Spec.cpp:1163`, `SpecPrepareStage::ExistingElementsLocked`.
- **#234 (декларация)** — `dbcfde6`: `CheckRuleElementByRule` был вызван из
  `Spec.cpp`, но не объявлен ни в одном заголовке — дерево не собиралось.
- **R9.4** — `fbc5162`; **R9.3** — `92fdbf7`; **R8.2–R8.5** — `b4add8d`,
  `61d8a51`, `b83a239`, `42f2d75`; **R8.1** — `b6d2fd9` + поправка `cdb9918`.
  Полная цепочка чекпоинтов R3–R8 — в `IDEA_ARCHIVE.md`.
- **Ревью #228** — `c32dfed` (`Refs: #237-#242`); тем же коммитом закрыт #229.

## Decisions

- Эталоны хранятся в gitignored `Reviews/spec_baseline/`; `Reviews/` не
  коммитится.
- Семантический ключ строки = favorite + значения без GUID-полей и «имени
  правила»; дубли ключей схлопываются с суммированием источников.
- **`diff_rows` не доказывает** множественность строк, состав источников по
  GUID, GUID существующих объектов и конечное состояние модели; при соединении
  `created` + `modified` общий ключ перекрывается (`spec_baseline.py:257-259`).
  Доказательство скорости/памяти — отдельный измеренный gate (R7.6, R10.6).
- **`spec_baseline.py compare` не принимает имя базы вторым аргументом** — он
  всегда заново прогоняет Spec и сравнивает с `raw-<tag>.json`. Пять «сравнений
  с пятью базами» на самом деле были пятью прогонами с одним эталоном.
- `TestFunc::Test()` остаётся включённым при каждом первом открытии проекта
  (решение владельца) — в P0 не менялось.
- **Эталонный проект восстановлен из авто-бэкапа.** `Test_file/test_25.pln`
  НЕ в git. `Test_file/test_25.bpn` содержал прежний sha256 `db1690f…`;
  восстановлен через `cp -p`. Перед новой регрессионной сессией сверить
  `sha256sum Test_file/test_25.pln` с `db1690f…`; при расхождении A/B
  недействителен.
- `PlaceElements`-сценарий считать эталоном только на ПЕРВОМ прогоне после
  рестарта; второй прогон на той же модели даёт законный no-op 0/0/0.


## #232 — отрицательное вещественное свойство (AC25 Windows Debug)

**Scope:** `Sources/AddOn/tests/TestParam.cpp`, `Docs/modules/TestFunc.md`,
`Docs/_progress.md`, `IDEA.md`; прод-код не менялся.
**Status: COMPLETED.** Вход и ожидание теста изменены на `-123456.7`:
`doubleValue` может округляться по настройке проекта, исходную точность хранит
`rawDoubleValue`. Issue закрыто как `not planned` по решению владельца:
https://github.com/kuvbur/AddOn_SomeStuff/issues/232
**Validation:** clang-format, clangd 0 диагностик; AC25 `.apx` собран и загружен
параллельным runner. Runner в 16:57:29 вернул 70 по прежнему отчёту; свежий
отчёт в 16:57:49 и следующий в 17:00:55: `TestConvertPropertyToParamValue`
23/23, `SUMMARY suites=70 passed=2861 failed=0`. Повторный запуск из этой
сессии не выполнялся, чтобы не прерывать работу другого агента в Archicad.
**Last Checkpoint:** `6617c4b` (код, карточка и зафиксированная в IDEA.md
валидация); этот архив — последующее документальное закрытие.
**Next Step:** по #232 действий нет; остальные задачи остаются выше.
