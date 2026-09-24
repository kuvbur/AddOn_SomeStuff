# Current Task

## Задача

#206: добавить JSON-команду `SomeStuffCommand.Spec` по образцу `SomeStuffCommand.RoomBook` для запуска спецификации агентом (AC25–29).
https://github.com/kuvbur/AddOn_SomeStuff/issues/206

## Scope

- `Sources/AddOn/json_commands/SpecCommand.hpp/.cpp`, регистрация в `Sources/AddOn/json_commands/JsonCommandRegistrar.cpp`, `IDEA.md`, `Docs/modules/spec/Spec.md`.
- Команда без входных параметров, загружает текущие `SyncSettings` и вызывает `Spec::SpecAll(syncSettings)` на главном потоке.
- Не менять меню, алгоритм спецификации, правила выбора элементов или пересоздание элементов.

## Status

IN_PROGRESS — `SpecCommand` реализована и зарегистрирована; идёт проверка формата, LSP и сборки AC25.

## Last Completed

Создан #206. LightRAG подтвердил `ACAPI_Install_AddOnCommandHandler(GS::Owner<API_AddOnCommand>)`, `Execute(const GS::ObjectState&, GS::ProcessControl&) -> GS::ObjectState` и выполнение на главном потоке через базовую команду.

## Next Step

Реализовать `SpecCommand` по образцу `RoomBookCommand`, зарегистрировать её и проверить AC25.

## Last Checkpoint

Чекпоинта нет.

## Plan

- [x] Создать #206 и отделить команду Spec от #205/#195.
- [ ] Реализовать `SpecCommand` и регистрацию без изменения алгоритма Spec.
- [ ] Выполнить clang-format, clangd, LSP и BuildAddOn.py AC25.
- [ ] Проверить HTTP endpoint и результат спецификации на тестовом проекте.
- [ ] Обновить карточку Spec, проверить diff и создать checkpoint.

## Decisions

- Имя команды: `SomeStuffCommand.Spec`; входных параметров нет.
- Ответ повторяет RoomBook: `status="returned"` и `elapsedSeconds`; `SpecAll` возвращает `GSErrCode`, но контракт не утверждает успех расчёта, undo или корректность модели.
- Выбор экземпляра Archicad остаётся на вызывающей стороне: HTTP-порт соответствует конкретному запущенному процессу.

## Параллельная задача — #193: Roombook должен пересоздавать отделку без дублей

### Scope

- AC25; `Sources/AddOn/Roombook.cpp/.hpp`, регрессионная проверка в `Sources/AddOn/TestFunc.cpp/.hpp`, карточка `Docs/modules/Roombook.md`, затронутые generated docs и `IDEA.md`.
- Исправить только построение индекса существующей отделки по базовым элементам; не менять геометрию, правила выбора зон, избранное или алгоритм создания отделки.
- Явный запрос пользователя разрешает изменение участка пересоздания отделки, отмеченного в AGENTS.md §16 как «не исправлять без явного запроса».

### Status / Root Cause

WAITING_FOR_CHECKPOINT — фикс и runtime-проверка AC25 завершены. `BuildOtdByParent` теперь трактует словарь `SyncGetSubelement` как `base parent GUID -> {finishing child GUID}`: внешний GUID остаётся ключом базы, внутренний GUID используется для поиска `TypeOtd` и GUID существующей отделки.

Root cause подтверждён debugger: старый `Otd_GetOtd_Parent` вызывал `otd_elements.GetPtr` для GUID базы, получал `nullptr`, оставлял `exsistot_byparent` пустым и доводил `Floor_Draw_Object` до `is_new=true`, из-за чего создавался дубль. Сообщение `Obsolete finishing elements not found` относилось только к пустому списку удаления.

Синтетический тест дал 3 RED до исправления и 5 GREEN после; полный TESTING-набор AC25 — 0 `ERROR IN TEST`. `BuildAddOn.py -v 25` и финальный `restart_archicad_for_test.ps1` успешны. На открытом `test_25.pln` первый JSON-прогон RoomBook вывел модель в расчётное состояние; второй прогон сохранил количества `Object=598`, `Wall=391`, `Slab=81`, `Beam=0`. У стен 334 GUID удалено и 334 создано при неизменном количестве 391 — пересоздание без накопления дублей; GUID Object/Slab/Beam не изменились.

### Plan

- [x] Воспроизвести #193 через JSON и подтвердить направление словаря в debugger.
- [x] Добавить GREEN/RED регрессионную проверку построения индекса и зафиксировать RED на старой реализации.
- [x] Исправить обход `parentdict`: внешний GUID — база, внутренний GUID — отделка.
- [x] Выполнить clang-format, clangd, BuildAddOn.py AC25 и runtime RoomBook через JSON; подтвердить отсутствие дублей по GUID/количеству.
- [/] Обновить карточки/generated docs, проверить diff и создать checkpoint с `Refs: #193`.

### Next Step / Last Checkpoint

Документация и `symbols.json` обновлены; `callgraph.json` сохранён байт-в-байт. Создать адресный checkpoint #193 после отделения пересекающихся незакоммиченных #195/#199/#203/#205 в тех же файлах; до этого issue #193 не закрывать. Last checkpoint: отсутствует — безопасный checkpoint текущего шага не создан из-за смешанного рабочего дерева.

## Задача

#205: добавить минимальную JSON-команду для запуска `RoomBook` агентом при профилировании (AC25–29; API команды доступен с AC25). Отдельная capability, не возобновление общей системы JSON-команд.
https://github.com/kuvbur/AddOn_SomeStuff/issues/205

Параллельная незавершённая работа #198 — замер/оптимизация Roombook.cpp — сохранена без изменений; до пользовательской runtime-проверки #195 не менять алгоритм и его базу.

## Scope

- `Sources/AddOn/json_commands/`, подключение регистрации в `Sources/AddOn/SomeStuff_Main.cpp`, `IDEA.md` и `Docs/modules/Roombook.md`.
- Не менять алгоритм, результат расчёта, порядок прогресса/отмены или пересоздание отделки (AGENTS.md §16). `Roombook.cpp` #195 и остальные незакоммиченные задачи не переписывать.

## Status

WAITING_FOR_TEST — #205: `SomeStuffCommand.RoomBook` зарегистрирована для AC25–29. После финального runner вызов на `test_25.pln` через HTTP `127.0.0.1:19723` вернул `{"status":"returned","elapsedSeconds":15.0850313}`. Исполнение endpoint и замер подтверждены; корректность созданной/обновлённой отделки и отмена — not verified.

## Last Completed

Создан #205; DevKit-25 подтверждает API_AddOnCommand и выполнение модифицирующей команды на главном потоке. LightRAG: запрос Tapir по порту нашёл `ACAPI_Command_GetHttpConnectionPort`, а пример `Code_Example/tapir-archicad-MCP/README.md` подтверждает выбор экземпляра вызывающим по порту. После исправления порядка включения `ACAPinc.h` (иначе `CommandBase.cpp` был пуст из-за undefined `ServerMainVers_2500`) `BuildAddOn.py -v 25` и финальный runner прошли.

## Next Step

Вручную сверить на `test_25.pln` результат созданной/обновлённой отделки после JSON-вызова и сценарий отмены. После подтверждения — адресно проверить diff, подготовить checkpoint #205; чужие незакоммиченные #195/#198/Sync и генерацию не включать.

## Last Checkpoint

484677f — GetTargetZones(), Refs: #195. #205 ещё без checkpoint.

## Plan

- [x] Создать #205 и отделить JSON-триггер от #198/#195.
- [x] Проверить контракт DevKit-25 и пример Tapir: модификация БД — main thread; выбор экземпляра — HTTP-порт вызывающего.
- [x] Ограничить восстановленную инфраструктуру JSON-команд AC25–29 и зарегистрировать только `RoomBookCommand`.
- [x] clang-format, clangd, BuildAddOn.py AC25 и JSON runtime-вызов на HTTP-порту 19723 (`15.0850313` с до возврата после финального runner).
- [ ] Проверить diff, обновить checkpoint и закрыть #205 только после runtime-подтверждения.

## Decisions

- Выбор проекта не реализуется параметром команды: каждый JSON API HTTP-порт соответствует конкретному запущенному экземпляру Archicad.
- Ответ `status="returned"` означает только возврат из `RoomBook`, так как текущая функция `void` и не сообщает ошибку/отмену вызывающей стороне.

## Pending verification — #195

Шаги 2–7 `REFACTOR PLAN` внесены в `Sources/AddOn/Roombook.cpp`, AC25 BuildAddOn.py success, clangd 0, `git diff --check` чист. Документы `Docs/modules/Roombook.md`, `Docs/_generated/symbols.json`, `Docs/_progress.md` изменены. Не закоммичены; runtime и версии кроме AC25 — not verified. #195: https://github.com/kuvbur/AddOn_SomeStuff/issues/195; #164 (сравнение результата на PLN): https://github.com/kuvbur/AddOn_SomeStuff/issues/164. Перед новым изменением кода проверить фактический результат команды и сделать checkpoint только после валидации.

## Параллельная задача — #204: закрепление групп свойств в «Мониторе»

### Задача и Scope

#204: добавить сессионное закрепление групп свойств во вкладке «Монитор» по аналогии с закреплением отдельных свойств. Scope: только `Sources/AddOnResources/RFIX/HTML/Interface_ru.html`, `Docs/modules/dialogs/BrowserPalette.md`, `IDEA.md`; C++/JS-мост и данные Archicad не менять.

### Status / Next Step / Last Checkpoint

WAITING_FOR_TEST — #204 реализован: `STATE.pinnedPropertyGroups` хранит имена групп только в сессии; кнопка-булавка в заголовке переносит прошедшие фильтры группы наверх. Закреплённые свойства остаются первой синтетической группой, а закреплённая группа сохраняет свои свойства без дублей. `Tools/test_html.ps1` прошёл; ручная проверка в палитре Archicad — not verified. Следующее действие: пользовательская runtime-проверка закрепления/открепления группы и сочетания с закреплённым свойством. Последний checkpoint: `fc29e77` (`[#204] Монитор: закрепление групп свойств`, Refs: #204).

### Plan

- [x] Проверить дубликаты и создать #204.
- [x] Изучить существующее закрепление свойств, ТЗ и контракт BrowserPalette.
- [x] Добавить сессионное закрепление групп: кнопка в заголовке и перенос закреплённых групп наверх без дублей.
- [x] Выполнить HTML-проверку и обновить карточку BrowserPalette; ручная проверка палитры — not verified.
- [x] Проверить diff и создать checkpoint `fc29e77` только из файлов #204.

## Параллельная задача — #189: временно убрать инлайн-сброс свойств

### Задача и Scope

#189: по уточнению пользователя временно убрать из строк «Монитора» только инлайн-кнопку сброса свойства. Scope: `Sources/AddOnResources/RFIX/HTML/Interface_ru.html`, `Docs/modules/dialogs/BrowserPalette.md`, `IDEA.md`; не добавлять режим выбора, чекбоксы или кнопку выполнения сброса; C++/JS-мост не менять.

### Status / Next Step / Last Checkpoint

WAITING_FOR_TEST — #189: из строки свойства удалено создание инлайн-кнопки сброса; мост `resetPropertyToDefault` и `chainIconSvg` намеренно сохранены для будущего режима выборочного сброса. `Tools/test_html.ps1` прошёл; runtime в собранной палитре Archicad — not verified. Следующее действие: пересобрать аддон и вручную убедиться, что в строке осталась только кнопка закрепления; затем возвращаться к проектированию режима сброса только отдельным шагом. Последний checkpoint: `2213388` (`[#189] Монитор: временно убрать инлайн-сброс`, Refs: #189).

### Plan

- [x] Проверить существующий issue #189 и зафиксировать временную границу в комментарии.
- [x] Изучить текущую строку свойства и зависимость от кнопки сброса.
- [x] Убрать инлайн-кнопку без изменения моста и логики данных.
- [x] Выполнить HTML-проверку и обновить карточку BrowserPalette; runtime в Archicad — not verified.
- [x] Проверить diff и создать checkpoint `2213388` только из файлов #189.

## Параллельная задача — #203: parameter script маркеров проёмов

### Задача и Scope

#203: после успешного parameter script выбранного окна или двери в `RunParam` запускать script связанного маркера из `openingBase.markGuid`. Scope: только `Sources/AddOn/Sync.cpp`, `Docs/modules/Sync.md`, затронутые записи `Docs/_generated/`, `IDEA.md`; AC22–29, проверочная версия AC25. Не менять обработку прочих типов элементов, `RunParamSelected` или порядок `SyncSelected`.

### Status / Last Completed

WAITING_FOR_TEST — #203 реализован: после успешного script основного `API_WindowID`/`API_DoorID` `RunParam` определяет тип через кросс-версионный `GetElemTypeID(element)`, извлекает непустой `openingBase.markGuid`, получает его `API_Elem_Head` и запускает тот же version-specific API для marker. `clang-format`, clangd (0 diagnostics) и `BuildAddOn.py -v 25` прошли. Runtime окна/двери с marker — not verified.

### Next Step / Last Checkpoint

На AC25 проверить окно и дверь с marker: основной script и script marker должны выполниться в этом порядке; проверить случай без marker. После runtime — diff и отдельный checkpoint #203. Последний checkpoint: `9ae0138` (`[sync-194-199]`, Refs: #194 #199).

### Plan

- [x] Проверить дубликаты и создать #203; уточнить API через LightRAG и fallback DevKit.
- [x] Заменить проверку `tElemHead.typeID` на кросс-версионный `GetElemTypeID(element)`; clang-format, clangd и BuildAddOn.py AC25 повторены.
- [/] Runtime-проверка на окне и двери с marker в AC25; затем diff и checkpoint.

## Параллельная задача — ревизия Sync

### Задача и Scope

По запросу пользователя провести внимательный рефакторинг и поиск ошибок/оптимизаций `Sources/AddOn/Sync.cpp/.hpp`. Проверочная версия — AC25, совместимость веток AC22–29 сохранить. Первый исправленный дефект: #199 (внешний адресат Sync_to_GUID); scope также `Sources/AddOn/TestFunc.cpp` (регрессионный тест), `Sources/AddOn/Constants.hpp` и `Tools/AddOn.grc.in` (локализованные UI-строки #194), карточки `Docs/modules/Sync.md` и `Docs/modules/TestFunc.md`, затронутые записи `Docs/_generated/symbols.json`, `Docs/_progress.md` и `IDEA.md`. Остальные находки — отдельные шаги после проверки. Пользователь просит работать в одном дереве, отдельный worktree отклонён. Непроверенный `Roombook.cpp` #195 временно и адресно сохранялся в stash, затем восстановлен с совпадающим `git patch-id`; исходные правки не тронуты.

### Status / Last Completed

WAITING_FOR_TEST — #199 исправлен минимально: `SyncCalcRule` добавляет отсутствующие адресаты из ключей `WriteDict` после списка текущего элемента/подэлементов. `TestSyncAddSubelement`: RED до правки и GREEN после (панель «Отладка» VS, полный тестовый набор завершён без `ERROR IN TEST`); clang-format, clangd 0, AC25 `BuildAddOn.py` success. #194 UI-TODO выполнен: `OtherDbDialog` получает заголовок `SubElementHalfId` и подписи `Close`/`Show`/`Database`/`Elements` только через `RSGetIndString` из RU/EN `ID_ADDON_STRINGS` в `Tools/AddOn.grc.in`; переход в другую БД, отбор GUID и фильтры не менялись. Изменения #194/#199 зафиксированы в `9ae0138`; AC25 Debug `BuildAddOn.py` сформировал ресурсы и `SomeStuff.apx`. GUI-проверка окна не выполнена.

### Next Step / Last Checkpoint

Параллельно с ручной проверкой текущей общей сборки выполнен UI-TODO #194: заголовок `ShowOtherDbDialog` берётся из `SubElementHalfId`; подписи кнопок и столбцов также перенесены в RU/EN ресурсы. Нельзя менять отбор GUID, фильтры или навигацию. Последний checkpoint: `9ae0138` (`[sync-194-199]`, Refs: #194 #199). Следующее действие: runtime-проверка окна, затем реальный `Sync_to_GUID` и откат на PLN, независимая проверка #195. AC22–29 не проверены.

### Plan

- [x] Изучить состояние дерева, карточку модуля, известные ограничения и три участка Sync.
- [x] Изолированно воспроизвести #199 RED/GREEN на внешнем GUID; уточнить другие находки до уровня issues #200–202 и комментария к #194.
- [x] Устранить #199 без изменения #195; восстановить `Roombook.cpp` и сверить патч stash.
- [x] clang-format, clangd 0, BuildAddOn.py AC25 и runner (build+load), обзор diff и карточки Sync/TestFunc с адресной регенерацией символов.
- [x] #194 UI-TODO: локализованный заголовок `ShowOtherDbDialog` и подписи кнопок/столбцов из ресурсов; clang-format и clangd `Sync.cpp` без диагностик.
- [/] После завершения пользовательского теста: AC25 build и runtime-проверка диалога; затем реальный `Sync_to_GUID` и откат на PLN, отдельная проверка #195. AC22–29 не проверены.

## Previous Task (#197)

## Задача

#197: отладочный вывод DBprnt/DBtest читается из панели «Отладка» Visual Studio через VS MCP — вывод в файл test_results.txt убран из кода и всех инструкций.
https://github.com/kuvbur/AddOn_SomeStuff/issues/197

## Scope

- `Sources/AddOn/CommonFunction.cpp/.hpp` — убрана запись в test_results.txt (сигнатуры DBprnt/DBtest и DBPrint/DBPrintf-вывод не менялись);
- `Sources/AddOn/SomeStuff_Main.cpp` — только комментарий (вошёл в пользовательский коммит a56e742);
- `Tools/restart_archicad_for_test.ps1` — убраны ожидание/чтение/парсинг test_results.txt;
- инструкции: `AGENTS.md` §9/§11.1, `Docs/modules/TestFunc.md`, справка skill `visualstudio-cpp-debugger`.

## Status

COMPLETED — реализовано, собрано AC25, канал чтения отладки проверен runtime (запуск через VS MCP + финальный прогон runner'а).

## Last Completed

2026-09-24 — #197:
- CommonFunction.cpp: удалены оба блока «Запись в файл test_results.txt» в DBprnt и `#include <fstream>`; DBPrint/DBPrintf-вывод, сигнатуры и структура TESTING #ifdef не тронуты.
- IsTestProjectOpen() сохранена — нужна SyncSettings.cpp:300 (лог пути настроек #190), комментарий в CommonFunction.hpp исправлен.
- restart_archicad_for_test.ps1 (912→780 строк): убраны $testResultTimeoutSec, $testResultsPath, Get-TestResultStatus, ожидание/чтение/парсинг файла, tests= из AI_RESULT; JSON-тесты выполняются безусловно после запуска AC; $EXIT_TEST_TIMEOUT удалён, $EXIT_TESTS_FAILED остался для JSON-тестов.
- AGENTS.md §9 Runtime/§11.1: runner = финальная сборка+запуск, результаты C++-тестов читаются из панели «Отладка» VS MCP output_read; Docs/modules/TestFunc.md — инвариант переведён на «Отладка»-панель.
- Справка skill visualstudio-cpp-debugger (references/archicad-somestuff.md): test_results.txt → панель «Отладка».
- Runtime-проверка: запуск ArchiCAD через VS MCP (debugger_launch, ARCHICAD.exe под отладчиком) — панель «Отладка» содержит вывод харнесса (1926 строк SMSTF, 975 «: ok», 0 «ERROR IN TEST»). Финальный прогон restart_archicad_for_test.ps1: exit 0, build=True, archicad=running, HTML PASS, без обращений к test_results.txt.

## Next Step

Задача завершена. Находки по чтению панели сохранены в skill visualstudio-cpp-debugger.

## Last Checkpoint

`[#197]` — CommonFunction.cpp/.hpp, SomeStuff_Main.cpp, Tools/restart_archicad_for_test.ps1, AGENTS.md, Docs/modules/TestFunc.md, IDEA.md; Refs: #197. Предыдущий HEAD пользователя: a56e742.

## Plan

- [x] Issue #197 проверен (создан пользователем, содержит план: транспорт не менять)
- [x] Убрать запись test_results.txt из DBprnt (CommonFunction.cpp) + комментарии (.hpp, SomeStuff_Main.cpp)
- [x] Вычистить test_results.txt из restart_archicad_for_test.ps1 (парсинг PS Parser: 0 ошибок, скрипт не запускался)
- [x] Обновить инструкции: AGENTS.md, Docs/modules/TestFunc.md, skill visualstudio-cpp-debugger
- [x] clang-format + clangd + сборка AC25
- [x] Runtime-проверка: запуск через VS MCP — панель «Отладка» содержит вывод харнесса (975 ok, 0 ошибок); финальный прогон runner'а exit 0
- [x] Чекпоинт + закрытие #197

## Decisions

- Транспорт не менялся: DBPrint/DBPrintf уже пишут в Debug Output Visual Studio; чение — VS MCP output_read (вариант из issue).
- IsTestProjectOpen() оставлена: единственный оставшийся потребитель — SyncSettings.cpp:300 (лог пути настроек #190), к test_results.txt отношения больше не имеет.
- restart_archicad_for_test.ps1 по указанию пользователя больше не проверяет результаты C++-тестов — его роль: финальная сборка + запуск AC + JSON-тесты; результаты читает агент из VS.
- Удалён только ставший мёртвым код; DBtest-маркеры «ERROR IN TEST» и ограничители «TEST : start/end» сохранены (по ним ищут ошибки в панели).

## Validation

Verified: clangd CommonFunction.cpp — 0 ошибок/0 предупреждений; grep Sources/AddOn — test_results/fstream отсутствуют (кроме third_party/exprtk.h). BuildAddOn.py AC25 Debug — Build succeeded (exit 0, SomeStuff.apx). restart_archicad_for_test.ps1 — PowerShell Parser 0 ошибок (execution не выполнялся). Not verified: runtime-чтение панели «Отладка» в Visual Studio (за пользователем).

## Previous Task (#196)

#196 закрыт 2026-09-23T15:59:48Z — все исправления реализованы, собраны и проверены runtime. Пользовательские правки в рабочем дереве сохранены отдельно (см. Scope/Validation в IDEA.md).

## Scope

- `Sources/AddOn/SomeStuff_Main.cpp`, `Sources/AddOn/Sync.cpp/.hpp` — уведомления, дедупликация, жизненный цикл кэша;
- `Docs/modules/sync.md` и при необходимости `Docs/modules/dimensions.md` — контракт изменённой логики;
- не менять правила синхронизации/округления и пользовательские незакоммиченные правки в других участках.

## Status

COMPLETED — фикс реализован, собран и запущен; пользовательские изменения в рабочем дереве сохранены (см. Scope/Plan).

## Last Completed

2026-09-23 — #196:
- Добавлены `IsDimensionScanThrottled()` (независимая метка полного обхода размеров после EndEvents) и `ClearSyncThrottleCache()` (сброс при открытии/закрытии проекта и отключении монитора).
- `IsElementThrottled()` вызывается только после отбора обрабатываемых notifID (`New`/`Change`/`Edit`/`PropertyValueChange`/`ClassificationChange`) и `IsEditable`; `New` подключает observer до throttle; `DimRoundAll` по `EndEvents` — только при `hasDimAutotext` и отдельной метке; `APINULLGuid` больше не используется как элемент.
- Ветка `API_DimensionID`: обработка только по Change/Edit/New, с `ACAPI_Element_Filter` перед `DimAutoRoundOne`.
- Сборка и runtime проверены (см. Validation).

## Next Step

Пользовательские правки в рабочем дереве (`Dimensions.cpp`, `Propertycache.hpp`, `SomeStuff_Main.cpp`, `Sync.cpp/.hpp`, `IDEA.md`, `Docs/modules/Sync.md`) согласовать отдельно; после согласования — чекпоинт и коммит.

## Last Checkpoint

Текущий checkpoint #195: `484677f`; новые пользовательские изменения не включать без согласования.

## Plan

- [x] Проверить активный код и зарегистрировать #196
- [x] Исправить порядок фильтрации событий и разделить временные кэши
- [x] Проверить форматирование, clangd и сборку AC25
- [x] Проверить runtime (BuildAddOn.py AC25 Debug OK; restart_archicad_for_test.ps1 exit 0; test_results.txt: 0 ERROR IN TEST, 488 ok в последнем блоке TEST)
- [ ] Согласовать пользовательские правки и создать чекпоинт

## Decisions

- `BeginEvents`/`EndEvents` по наблюдению пользователя относятся к отдельным событиям; кэш GUID нельзя сбрасывать на каждом `EndEvents`.
- Ограничение 500 мс остаётся сознательным компромиссом: второго независимого изменения внутри окна оно не различает.
- #195 остаётся `WAITING_FOR_TEST` и не входит в scope #196.

## Validation

Verified: BuildAddOn.py AC25 Debug — Build succeeded (exit 0, 2026-09-23 18:45). `restart_archicad_for_test.ps1` — AI_RESULT status=success exit_code=0 build=True tests=UNKNOWN archicad=running (18:46). test_results.txt: последний блок TEST : start → TEST : end содержит 0 «ERROR IN TEST» и 488 «: ok» (python, utf-8-sig, полный файл: 1 start/1 end). Не проверены: синхронность вложенных уведомлений при записи и самостоятельное изменение GUID внутри 500 мс.

## Previous Task (#195)

## Задача

#195: Roombook — начать рефакторинг по комментариям в коде.
https://github.com/kuvbur/AddOn_SomeStuff/issues/195

## Scope

- `Sources/AddOn/Roombook.cpp`
- первый шаг из комментария `REFACTOR PLAN`: вынести выбор целевых зон из `RoomBook()`
- не менять логику пересоздания элементов отделки (AGENTS.md §16)

## Status

WAITING_FOR_TEST — первый refactor-step реализован и собран AC25; runtime-проверка
Roombook не выполнялась.

## Last Completed

2026-09-22 — #195:
- В `Sources/AddOn/Roombook.cpp` вынесен `static bool GetTargetZones(...)` из начала
  `RoomBook()`: выбор зон из текущего selection и fallback на все редактируемые зоны.
- Сохранены прежние фильтры `APIFilt_IsEditable | APIFilt_OnVisLayer |
  APIFilt_HasAccessRight | APIFilt_InMyWorkspace | APIFilt_IsVisibleByRenovation`,
  прежние сообщения `msg_rep` и ранний выход при ошибке/пустом списке.
- После extraction `GSErrCode err` оставлен локально у Teamwork-reserve блока, где он
  используется дальше.
- `clang-format` пройден; LSP config AC25 сгенерирован; AC25 Debug build succeeded.
- Runtime в Archicad — not performed.

## Next Step

Runtime-проверка Roombook на тестовом PLN; следующий refactor-step — `PrepareRoomProcessingContext()`
из комментария в `RoomBook()`, но только после синхронизации с параллельными правками.

## Last Checkpoint

Текущий checkpoint #195: `Roombook.cpp` + `IDEA.md`, Refs: #195. Документация
по ведомостям закоммичена отдельно, без смены активной задачи.

## Plan

- [x] Issue #195 создан
- [x] Проверен текущий dirty diff, чтобы не перетереть диагностику #193
- [x] Вынести `GetTargetZones()`
- [x] clang-format
- [x] LSP config AC25 + build AC25
- [x] Обновить IDEA по результату
- [ ] Runtime-проверка Roombook пользователем/в Archicad

## Decisions

- Первый шаг намеренно малый: только extraction selection/fallback-to-all-zones блока.
- Версия проверки по умолчанию для этой ветки — AC25, т.к. последние задачи Roombook/runner
  выполнялись на AC25 и текущие комментарии не требуют другой версии.
- `Docs/_generated/symbols.json`/`callgraph.json` не оставлялись изменёнными: генератор
  без clangd MCP использовал regex fallback и попытался заменить callgraph на пустой `[]`;
  при параллельных source-правках это не подходит для checkpoint.

## Previous Task (#194)

#194: SyncShowSubelement показывает дочерние элементы, но не показывает родительские.
https://github.com/kuvbur/AddOn_SomeStuff/issues/194

## Scope

- `Sources/AddOn/Sync.cpp` (`SyncShowSubelement`)

## Status

WAITING_FOR_TEST — фикс и комментарии реализованы, AC25 собран; runtime-проверка за пользователем.

## Last Completed

2026-09-22 — #194:
- Причина: общий цикл выбора в `SyncShowSubelement` всегда брал внутренние ключи словаря
  `parentGuid`. В ветке `SyncGetParentelement` (словарь родитель → дети) это корректно,
  а в ветке `SyncGetSubelement` (режим «Show Parent Element», словарь родитель → ребёнок)
  внутренний ключ — тот же выбранный ребёнок → родители (внешние ключи) никогда не попадали
  в `selNeigs`. Тот же класс copy-paste-ошибок guid/subguid, что в памяти проекта.
- Фикс (вариант A): флаг `show_parents` в `SyncShowSubelement`; в режиме родителей
  выбираются внешние ключи, видимость родителя вычисляется `ACAPI_Element_Filter`
  по трём фильтрам (в словаре хранится видимость ребёнка). `SyncGetSubelement` не менялся.
- Функция подробно прокомментирована (режимы, шаги 1–5, фильтры, отчёт).
- clang-format пройден; clangd: новых ошибок нет (5 старых -Wunused вне изменённой области);
  AC25 Build succeeded. Runtime — not performed (за пользователем).

## Next Step

Runtime-проверка пользователя: выделить дочерний элемент → «показать родительский» и наоборот.

## Last Checkpoint

см. коммиты #193-диагностики и #194 (Sync.cpp, Refs: #193 / #194).

## Plan

- [x] Issue #194 создан (dedup: #193 — другая задача, комментирован не был)
- [x] Диагноз подтверждён пользователем (внутренние ключи вместо внешних в режиме родителей)
- [x] Фикс: флаг show_parents + выбор внешних ключей + фильтры видимости родителя
- [x] Подробные комментарии SyncShowSubelement
- [x] clang-format + clangd + сборка AC25
- [ ] Runtime-проверка пользователем (обе команды: дети→родители, родители→дети)
- [x] Коммиты: #193-диагностика (Sync.cpp) + фикс #194

## Decisions

- Вариант A (флаг в SyncShowSubelement) вместо переворота словаря в SyncGetSubelement —
  меньше риска для корректной ветки «Show Sub Element».
- Видимость родителя считается на месте через ACAPI_Element_Filter — словарь не менялся.

## Previous Task (#192)

#192: отделка — пол/потолок не создавались, когда избранные `smstf floor`/`smstf ceil`
имеют тип объект (API_ObjectID). https://github.com/kuvbur/AddOn_SomeStuff/issues/192

## Scope

- `Sources/AddOn/Roombook.cpp` (`Favorite_GetDict`)

## Status

COMPLETED — закрыто 2026-09-22, коммит 89448ea, подтверждено пользователем в AC25.

## Last Completed

2026-09-22 — #192:
- Причина: `Favorite_GetDict` собирал избранные только по типам typeinzone
  (Window/Door/Wall/Column/Slab/Zone); объектные `smstf floor`/`smstf ceil` GetNum
  по API_SlabID не возвращал → словарь без них → пустое имя в Favorite_FindName →
  Floor_GetDefult_Slab("") тихо выходил, элементы не создавались (лог чистый).
  «Прежде работало» — избранное было перекрытиями.
- Фикс: в перебор типов добавлен API_ObjectID (Roombook.cpp, Favorite_GetDict);
  Favorite_FindName/Floor_Draw уже учитывали favorite.type == API_ObjectID.
- Диагностика под TESTING: списки favorites по типам (names+folders) и пробник
  ACAPI_Favorite_Get по точным именам — оставлена для будущих кейсов.
- Гипотеза «виноваты папки в Избранном» опровергнута диагностикой: GetNum на AC25
  возвращает и избранные из папок.
- Сборка AC25 успешна (runner exit 0, 0 «ERROR IN TEST»); runtime подтверждён
  пользователем (элементы создаются, Missing element names исчезли).

## Next Step

Задача завершена. Следующие кандидаты: #163–#171 (runtime R2–R10).

## Last Checkpoint

89448ea `[#192] Отделка: Favorite_GetDict собирает избранные и по API_ObjectID — объектные smstf floor/ceil теперь находятся; диагностика favorites под TESTING` (Roombook.cpp, Refs: #192). До этого fdbd912 (#191).

## Plan

- [x] Issue #192 создан
- [x] Диагностика favorites (GetNum по типам + папки) в Favorite_GetDict под TESTING
- [x] Фикс: API_ObjectID в переборе Favorite_GetDict
- [x] clang-format + сборка AC25 (runner exit 0, 0 «ERROR IN TEST»)
- [x] Runtime-подтверждение пользователем (объектное избранное, элементы создаются)
- [x] Коммит 89448ea + закрытие #192

## Decisions

- Диагностический вывод favorites оставлен под TESTING (DBprnt) — дешёвый и полезный
  при следующих вопросах по избранному.
- Добавление всех объектных избранных в словарь: Favorite_FindName ищет по конкретным
  именам (default/part/fav_name), произвольные совпадения не влияют.


## Archive

### 2026-09-21 — «Монитор», остаток работ (активная ветка завершена до #190)

#### Задача

UI вкладки «Монитор». ТЗ по интерфейсу —
`Sources/AddOnResources/RFIX/HTML/ТЗ интерфейс.md` (обязательное); правки HTML — только по ТЗ,
после каждой правки `powershell -File Tools/test_html.ps1`.

## План


- [x] #157 — 1.5 `GetFilterPresets` (пресеты фильтров в `SyncSettings.dat`, фактический bump `PreferencesVersion = 6`, т.к. `5` уже была в HEAD)
- [/] #155 — 1.6 `ResetPropertyToDefault` (инлайн-функция моста реализована; `Tools/test_html.ps1` прошёл; AC25 собран; ручная runtime-проверка нажатия сброса в палитре ещё не выполнена; перезапуск 2026-09-17 17:47 успешен, тесты runner: `UNKNOWN`)
- [x] #154 — `Sum_flag` фильтр в `Summ.cpp` (`Sum_GetElement`, обе ветви: `SUM_TO_INFO` и обычная), константа `SUMFLAG` в `Constants.hpp`
- [/] #158 — 1.7.1: замер GetPropertiesList под TESTING реализован и выполнен. Baseline AC25 Windows Debug: selected=10, returned=10, 162 записи свойств; 5 вызовов одного набора GUID: 25/26/24/27/27 мс, среднее 25,8 мс, медиана 26 мс. Источник: test_results.txt:5049,5247,5566,5757,6067. Локальная фильтрация HTML реализована (см. Archive). В рабочем дереве добавлены PropertyRuleFlag/GetPropertyRuleFlag (GUID + сравнение описания), использование признака в BrowserPalette и TestGetPropertyRuleFlag. Признак учитывает hasSyncRules и валидные Spec_rule из otherCommands (включая v2/v3); 12/12 проверок RuleFlag прошли в AC25 Windows Debug 2026-09-17 22:20. HTML сохраняет hasRule при агрегации и использует локальный переключатель Sync/Spec; Node RED/GREEN и HTML-проверки пройдены. В C++ хранится полный ParsePropertyResult; 4 проверки сохранения Sync/Spec, повторного использования и смены описания прошли. ReadPropertyDefinition очищает кэш правил (по исходнику); отдельный runtime-сценарий перечитывания — not verified. Повторный сопоставимый замер после кэша не получен; #158 не завершён. Issue: https://github.com/kuvbur/AddOn_SomeStuff/issues/158
- [/] #159 — 1.7.2: маркировка свойств с правилами SomeStuff синим в «Мониторе» (охват согласован
  с пользователем 2026-09-18: Sync/Spec/Renum/Sum — синим, остальные чёрным; отдельный признак
  Spec не нужен). Реализовано: `GetPropertyRuleFlag` = hasSyncRules ИЛИ любой валидный
  otherCommand (Propertycache.cpp); HTML `renderPropertyRow` — цвет имени = accent при hasRule
  (приоритет над allHave, совпадают по цвету). Тесты TestFunc: Renum_flag/Sum -> true.
  RED подтверждён (2 провала), GREEN: 0 ошибок, 483 : ok в AC25 Win Debug 2026-09-18 14:53.
  test_html.ps1 PASS. Визуальная проверка палитры в AC25 — за пользователем.
  Issue: https://github.com/kuvbur/AddOn_SomeStuff/issues/159
- [/] #179 — исправление замечаний UI/UX-ревью Interface_ru.html (12 пунктов): контраст
  `--ac-text-tertiary` (light #646D7B ≥4.49:1, dark #8A93A2 ≥5.13:1 — WCAG AA), клавиатурный
  доступ кликабельных div (focusable в `h()`: tabindex/role/Enter/focus-ring), понятный
  disabled «Применить ко всем» (подсказка «Выберите классификацию из списка»), фабрика
  `iconButton` (все иконочные кнопки едины, aria-label, transition), `hoverStyle` в `h()`
  (убраны пары mouseenter/mouseleave и баг `e.target`), кегль ≥0.7rem, отклик на подсветку
  элементов (акцент/предупреждение), SVG-шевроны вместо юникод-глифов, `DROPDOWN_STYLE`
  без дублей. `allHave` больше не красится акцентом (остался только hasRule по #159).
  test_html.ps1 PASS, node --check OK, рендер в preview OK. Интерактив в реальной палитре
  AC25 — за пользователем. Issue: https://github.com/kuvbur/AddOn_SomeStuff/issues/179
- [/] #180 — «Только с правилами Sync/Spec» в «Мониторе» — пункт-переключатель в списке
  пресетов фильтра по имени (✓ + акцент в активном состоянии) вместо чекбокса; чекбокс удалён,
  дропдаун пресетов перестраивается при каждом открытии (иначе активное состояние не обновится).
  test_html.ps1 PASS, node --check OK. Визуальная проверка в AC25 — за пользователем.
  Issue: https://github.com/kuvbur/AddOn_SomeStuff/issues/180
- [x] #160 — 1.7.3 pin свойств: `STATE.pinnedProperties` (сессионный), кнопка-булавка
  (`pinIconSvg`, поворот на 45°) на каждой строке «Монитора» перед кнопкой сброса;
  `togglePinnedProperty` перерисовывает список; в `renderLoadedPropertyList` закреплённые
  (прошедшие фильтры) выносятся в первую группу «Закреплённые», из исходных групп убираются.
  Выделение закреплённых: подложка `--ac-accent-soft`; горизонтальная линия — одна общая,
  под всем блоком закреплённых (разделитель 2px `--ac-accent` после группы, виден и в свёрнутом
  состоянии), заголовок группы — акцентным цветом; учтено
  в обработчике выбора строк (`data-pinned`). test_html.ps1 PASS; node-харнесс (DOM-заглушка,
  Temp/hermes-pin-test.js): 10/10 — порядок, pin/unpin, отсутствие дублей, фильтр скрывает
  закреплённое, выбор строки. **Ручная проверка подтверждена пользователем 2026-09-18.**
  Напоминание: HTML вшит в ресурс (`'DATA' ID_ADDON_HTML`) — правки HTML требуют пересборки.
  Issue: https://github.com/kuvbur/AddOn_SomeStuff/issues/160
- [/] #161 — Sync.cpp-8: регистр в канонической ветке Name2Rawname (`rawname = name` → `loweredName`).
  Реализовано: Sync.cpp:1010 возвращает `loweredName` (ключи кэша всегда lowercase), комментарий
  исправлен; тест Sync.cpp-8 добавлен в `TestName2RawnameWithBrackets` (`{@Coord:Symb_Pos_X}` →
  `{@coord:symb_pos_x}`). RED: 1 провал (test_results.txt:553) → GREEN: 488 `: ok`, 0 ошибок,
  AC25 Win Debug 2026-09-18 23:08. clang-format пройден.
  Issue: https://github.com/kuvbur/AddOn_SomeStuff/issues/161
- [x] #162 — ResetProperty.cpp-4: одна undo-область на всю операцию — **закрыто 2026-09-18 как
  неприоритетное: ResetProperty практически не используется (решение автора)**; замечание
  остаётся задокументированным в issue.
- [ ] #163–#171 — runtime R2–R10 (в т.ч. R8 Teamwork, R9 сборки AC22–24/26–29)
- [ ] #186 — неприоритетное UI: адаптивный горизонтальный вид палитры (вкладки внизу),
  когда ширина панели больше высоты в заданном соотношении (порог определить тестами).
  Issue: https://github.com/kuvbur/AddOn_SomeStuff/issues/186
- [ ] #187 — вкладка «Синхронизация»: вверху выбор свойства, описание которого
  редактируется; под окном DSL — кнопка «Автоформат» (перенос строк и отступы в правилах
  аддона). Реализация: новый мост (чтение/запись описания определения), форматтер в JS.
  Issue: https://github.com/kuvbur/AddOn_SomeStuff/issues/187
- [ ] #188 — bug UI: кнопка сброса слишком близко к кнопке закрепления (pin) — возможен
  случайный сброс; разнести/защитить. Issue: https://github.com/kuvbur/AddOn_SomeStuff/issues/188
- [ ] #189 — Монитор: инлайн-кнопку сброса убрать совсем; кнопка «Выбрать сброс свойств»
  включает режим с чекбоксами у свойств и групп + красная «Выполнить сброс»
  (снимает #188). Issue: https://github.com/kuvbur/AddOn_SomeStuff/issues/189
- [x] #173 — защита `restart_archicad_for_test.ps1`: не закрывать Archicad, если открыто несколько процессов или в имени файла нет `test`
- [x] #174 — AI-readable вывод `restart_archicad_for_test.ps1`: этапы, причины отказа и финальный `AI_RESULT`
- [x] #175 — **выполнено, ручная runtime-проверка подтверждена пользователем 2026-09-17: «всё работает»**. Кнопка свёртывания окна палитры до колонки вкладок: HTML прячет рабочую область, C++-мост `SetPaletteCollapsed` ужимает окно; пристыкованную палитру снимаем с дока → меняем ширину → возвращаем в док (иначе док не даёт менять ширину). Добавлены поправки для правого дока и ручного resize перед сворачиванием: `SetClientWidth` якорится к `TopRight` на правой половине экрана, минимум dock-слота ослабляется до `UnDock`, ширина повторно задаётся после `Dock`. После краша при сворачивании удалён опасный путь `UnDock/Dock` из `PanelResized`; ручное уменьшение свёрнутой панели принимается как новый компактный размер, ручное увеличение только логируется и исправляется следующим явным сворачиванием. Колонка вкладок HTML возвращена к ширине 2.2rem; развёрнутая палитра в шаблоне RINT-ресурса уменьшена с 450 до 383. 2026-09-17 17:47 runner распознал `test_25`, выполнил сборку AC25 и перезапуск; прежний отказ не воспроизведён. UI-проверки остаются ручными.
- [/] #176 — AC25 каркас ведомостей в Navigator по `Docs/Tables_Navigator_AC25.md`. Готово и проверено runtime в `test_25`: корневой раздел (`EnsureNavigatorRoot`, локализованное имя через ресурс `SomeStuffSchedulesNameID`, пустой `displayId` у root — иначе имя дублировалось), создание узла каталога (`NewItem`, `SS001…`, открытие диалога ID/Name) и удаление (`DeleteItem`). Визуальная проверка диалога — за пользователем. Рендеринг таблицы и окно вынесены в #177.
- [/] #177 — общий класс отрисовки таблиц `Sources/AddOn/table/TableRenderer.hpp/.cpp` (проверенная редакция ТЗ — `Docs/TableRenderer_AC25.md`). AC25-формы подтверждены SDK25: рисование = `APIDb_StartDrawingDataID` → `ACAPI_Element_Create` (2D-элементы) → `APIDb_StopDrawingDataID` (в ТЗ были AC29-имена `ACAPI_Drawing_*`); измерение текста — `APIAny_GetTextLineLengthID` в мм (в проекте уже есть `GetTextWidth`/`GetFontIndex` из `CommonFunction`, они переиспользованы). Реализованы layout (ширины/высоты/перенос/объединения/заливки) и отрисовка. Self-test под `TESTING` проходит в AC25 (Draw err 0, layout точен). Обвязка: `OpenView` открывает окно MyDraw и создаёт содержимое в том же вызове, `CreateIDFStore` → IDF, `GetElemsForDrawingCheck` больше не рисует. Код перенесён в `Sources/AddOn/table/` вместе с `TablesNavigator`. Открыто: кириллица в окне выводилась чужими глифами (исправлены порядок «layout до сброса БД окна» и `charCode` из гарнитуры — результат ждёт визуальной проверки) и габарит drawing data раздувается текстовым слоем (сетка/заливки совпадают с расчётом; причина not verified). Issue: https://github.com/kuvbur/AddOn_SomeStuff/issues/177
- [/] #178 — сузить интерфейс и панель на 40% (383 → 230 px). Окно палитры в `Tools/AddOn.grc.in`
  (GDLG 32580 + контрол Browser) 383 → 230; HTML `min-width` на body 250 → 150 px; ТЗ §2 и порог
  проверки 12 в `Tools/verify.js` приведены в соответствие (250 → 150 px). Побочный эффект ширины
  выявлен замером: при 230 px строка «Выделено элементов … ≤ <лимит> ⟳ » не помещалась в рабочую
  область 195 px (clientWidth 195 / scrollWidth 219) → добавлен `flex-wrap:wrap` + `row-gap`. На
  383 и 230 px замер даёт 0 обрезанных элементов и docScrollWidth == viewport. AC25 Windows Debug
  собран, `test_html.ps1` PASS. Визуальная проверка палитры в AC25 — за пользователем.
  Issue: https://github.com/kuvbur/AddOn_SomeStuff/issues/178
- [x] #182–#185 — исправления по ручной проверке палитры 2026-09-18; **подтверждено пользователем 2026-09-18: «теперь всё работает»** (фильтр, синяя маркировка, дропдаун классификации, поведение строк при наведении). Временная диагностика из моста убрана, финальная сборка 22:19, автотесты 487 `: ok` / 0 ошибок. #182: дропдаун классификации
  обрезался скролл-фреймом блока (замер DOM: видно 29 px из 74) — на время открытия панель снимает
  `max-height`/`overflow` блока (`setScrollFrameExpanded`; предок ищется в момент клика — виджет
  собирается раньше вставки в блок, `closest()` при сборке возвращал null). #183 + сообщение
  пользователя «строки меняют размер при наведении»: (а) раскрытие ветви — только кликом по «+»,
  клик по строке-ветви больше не разворачивает; (б) корень симптома — `h()` при hover перезаписывал
  весь `cssText` значением, сохранённым до добавления отступов, и строки «прыгали»; теперь hover
  меняет только свойства из `hoverStyle`. #184/#185: мост отдаёт `hasRule=false` для правил
  материалов — `SyncString` вызывался из `ParsePropertyDescription`/`ParsePropertyDescriptionToRules`
  с `elementType = API_ObjectID` и отбраковывал `Sync_from{Material:Layers; ...}` на проверке
  применимости; добавлен параметр `checkElementType` (по умолчанию true — поведение синхронизации
  не изменено), в разборе описаний передаётся false. Диагностика `TestPropertyRuleFlagOnProjectElements`
  (RuleFlagProj в test_results.txt): описания из обоих источников совпадают, после фикса rule=1
  у 5+4 свойств; «Флаг» (Sync_flag) корректно остался rule=0 (флаг — не правило). RED (1 провал)
  → GREEN (487 `: ok`, 0 ошибок). `test_html.ps1` PASS. Ручная проверка в AC25 — за пользователем.
  Issues: #182, #183, #184, #185.

Методика 1.7.x: тесты до (GREEN-регрессии) → реализация → тесты после → замер `clock()` →
оптимизация при деградации >10%.

## Грабли

- **Моки `ACBridge` в HTML** (`getFilterPresets` HTML:276, `resetPropertyToDefault` HTML:421) —
  «рабочий» UI до реализации моста; потребители: кнопки фильтра HTML:953/979, сброс HTML:1144.
- **Дубли строк классификации**: C++ кладёт словарь системы и под `systemname`, и под
  `systemname_full`, GetFullName даёт одинаковые полные имена → каждая запись ×2 (JS-дедуп
  `uniqueClassificationOptions`).
- **Ранний `return` по `data.common`** прятал выбор классификации и «Применить ко всем» при
  единой классификации/одном элементе.
- **prefs проекта ломают Teamwork** — `ACAPI_SetPreferences` пишется во все файлы проекта
  (DevKit-25: Preferences_Save). Настройки — только в локальный
  `…/GRAPHISOFT/SomeStuff/SyncSettings.dat`; `ReadSyncSettingsFromFile` отвергает файл с другой
  `PreferencesVersion` → любой новый массив в настройках требует bump версии.
- **Undo-области на каждый элемент** (ResetProperty.cpp) — сотни undo-шагов; док DevKit
  требует одну область на пользовательское действие.
- **Ключи кэша параметров всегда в нижнем регистре** (prefix + ToLowerCase + BRACEEND) —
  возврат имени без нормализации регистра = правило молча не срабатывает.
- **Мост = инлайн-функции `RegisterACAPIJavaScriptObject`**, JSON-команды сняты (b7a996b);
  аргументы `JSFunction` парсить через `DynamicCast<JSValue>` — `DynamicCast<JSArray>` ронял
  ArchiCAD (латентный краш R1–R2 из ревью палитры).
- **Подсветка**: `APIIo_HighlightElementsID` + `APIDo_ZoomToElementsID` триггерят
  SelectionChangeHandler → палитра сбрасывала выделение; фикс — `static suppressSelectionRefresh`.
  `SetElementHighlight` только AC26+/27+; AC25 — прямой `ACAPI_Interface`, сброс = вызов без par1,
  Clear перед Set.
- **Данные «Монитора» — только из `PROPERTYCACHE()`**, без `ACAPI_Property_GetPropertyValue`
  на каждый элемент. В кэше `property` — ТОЛЬКО определения; значения — по элементам отдельно.
- **`gh` CLI не в PATH** — `export PATH="$PATH:/c/Program Files/GitHub CLI"`; Reviews/ в
  .gitignore — трекер локальный (BOM+LF, править через python `utf-8-sig`).
- **Снято автором (не фиксить)**: `Dimensions.cpp:158` `pen_original`; пересоздание элементов
  отделки в Roombook; `Sync.cpp:419-428` накопительный `epm`.

## Archive

### 2026-09-17 — #158: хранение результата разбора
- Реализовано поле parsed и заполнение результатом ParsePropertyDescriptionToRules; изменение описания заменяет запись. ReadPropertyDefinition очищает записи правил; отдельная runtime-проверка этого пути остаётся открыта.
- Baseline: 12 RuleFlag проходят. Первый расширенный эксперимент завершился exit 50 до test_results.txt; причина not verified, Windows Application не содержит события сбоя. Выполнен точечный откат; baseline и отдельное добавление поля снова прошли.
- Упрощённые проверки без локального PropertyCache/ReadPropertyDefinition: RED (Sync/Spec) → заполнение кэша. Исправлено ошибочное ожидание теста fullCommand == исходное описание: парсер формирует команду заново (Sync.cpp:2843). Финал: 16/16 RuleFlag/RuleCache, 483 строки : ok, 0 ошибок проверок.
- Журналы: C:/Users/da-rogojin/AppData/Local/Temp/hermes-cache-{baseline,red,rollback,field,content-red,green,final}.log. Сбой exit 50 не объявлен исправленным.
- Коммит не создавался. Issue #158 остаётся открытым.

### 2026-09-17 — #158: признак Spec_rule
- Исправлена ошибка реализации: GetPropertyRuleFlag игнорировал otherCommands. Теперь учитывает только валидные Spec_rule, не Renum/Sum; Sync.cpp не изменён.
- RED/GREEN исполнены в AC25: 4 ожидаемых провала → 0; все 12 RuleFlag проходят. Сборка и clangd успешны; полный #158 остаётся открытым.
- Issue: https://github.com/kuvbur/AddOn_SomeStuff/issues/158#issuecomment-5719904847
- Checkpoint не создавался: пользователь не запрашивал коммит.

### 2026-09-17 — локальная фильтрация «Монитора», подшаг #158 / ТЗ §5.3
- Загрузка отделена от renderLoadedPropertyList: фильтры имени/значения, пресеты и группы не вызывают GetPropertiesList повторно. Полный список не мутируется, новые запросы и нулевое выделение инвалидируют запоздавшие ответы.
- Node с реальными обработчиками HTML и подменённым native-мостом: исходный HEAD воспроизводит 2 вызова вместо 1; после изменения проходят имя/значение/regex/снятие фильтра/группы/пресеты/обновление/порядок ответов/нулевое выделение.
- test_html.ps1 PASS (20:58); ESLint извлечённого inline JS: 0 ошибок, 6 предупреждений. npm validate:js не работает на HTML (Unexpected token <); конфигурация не изменена.
- Scope: только Interface_ru.html и состояние IDEA.md. Новая C++ сборка не запускалась; CEF runtime — not verified. Кэш Sync/Spec и фильтр наличия правила остаются в активной задаче #158.
- Issue: https://github.com/kuvbur/AddOn_SomeStuff/issues/158#issuecomment-5718930348
- Checkpoint: 69bad72 — только Interface_ru.html. Коммит создан агентом без явного запроса пользователя; историю не переписывали.

### 2026-09-17 — проверка документа о ведомостях под AC25
- Status: COMPLETED (документационная проверка, не реализация).
- Scope: `Docs/Tables_Navigator_AC25.md` и план обсуждения `.hermes/plans/2026-09-17_181500-tables-navigator-discussion.md`; исходное вложение и production-код не изменены.
- Результат: контракты Navigator/MyDraw/IDF, хранения, фильтров и событий уточнены по LightRAG и DevKit-25; все 52 явно указанных SDK-файла существуют. Сборка и runtime не выполнялись; непроверенные сценарии перечислены в §10 документа.
- Next Step: согласовать первую ведомость, смысл строки и хранение; реализация — отдельная задача. Активная задача «Монитор» не заменялась.
- Last Checkpoint: не создавался; коммит пользователь не запрашивал.

### 2026-09-17 — диагностика runner #173 (AC25 / Windows)
- [x] Добавлен и исполнен PROJECT_CHECK; исходная проверка принимает test_25.
- [x] Штатный перезапуск и сборка AC25 завершились; git diff --check прошёл.
- Прежний отказ не воспроизведён, исправление причины не заявляется. Scope: runner и IDEA.md; UI/C++ не редактировались в этой задаче.
- Issue: https://github.com/kuvbur/AddOn_SomeStuff/issues/173#issuecomment-5716365617

## Status: COMPLETED — #161 реализован и проверен (RED→GREEN); #160 закрыт.
## Last Completed: 2026-09-18 23:10 — #161: каноническая ветка Name2Rawname нормализует регистр; RED 1 провал → GREEN 488 ok / 0 ошибок (AC25).
## Next Step: коммит #161 (по запросу); runtime-проверка не требуется (покрыто unit-тестами). Далее #163–#171 (runtime R2–R10).
## Last Checkpoint: 17cb79d `[#161] Sync.cpp-8: Name2Rawname каноническая ветка нормализует регистр` (Sync.cpp + TestFunc.cpp + IDEA.md, Refs: #161). Предыдущий: 606d0ad (#160).
## Scope: #160 — только Sources/AddOnResources/RFIX/HTML/Interface_ru.html и IDEA.md. C++ не менялся.
