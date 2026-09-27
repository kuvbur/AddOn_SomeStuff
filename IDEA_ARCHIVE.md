# IDEA Archive

Завершённые задачи и отменённые направления из `IDEA.md`. Новая запись — сверху (сразу под этим абзацем),
старые — ниже.

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
