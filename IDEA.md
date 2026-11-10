# Current Task — #317: безопасный откат Wall writer

## #308 — просмотр записи плинтусов в свойства

Scope: AC25 Windows; по команде «Исправь» устранить замечания OtdSlab_WriteSkirt: независимые свойства, запись нуля/очистка типа и guard успешности целевого пола/потолка; адресная регрессия и документация. Геометрию/подсчёт, остальные writers и пользовательские изменения сохранить; Archicad не перезапускать, push не запрошен.
Status: WAITING_FOR_NATIVE — правка, offline-проверки, независимое ревью и документация завершены; native не запускался.
Plan: [x] 308.R1 — проверить заготовку и потребителя; [x] 308.R2 — проследить подготовленные длины, guards writers и свойства; [x] 308.P1 — RED и минимальная правка трёх замечаний; [x] 308.P2 — GREEN/характеризация, форматирование, свежая LSP/компиляция без остановки host и ревью; [x] 308.P3 — документация и issue; [x] 308.W — по запросу владельца добавить описания свойств результата плинтуса в таблицу wiki/ru/Finishing-ru.md, сверить литералы/сохранность страницы; [ ] 308.N — canonical build/native после отдельного разрешения при активной отладке.
Last Completed: владелец исправил == " " до baseline. Агент изменил только OtdSlab_WriteSkirt: guard write_succeeded, независимые свойства и запись 0/очистка типа. Дословный helper + AddParamValue2ParamDictElement, controlled GS/типы/конвертеры, MSVC C++14 /WX: родитель повторил RED295/101FAIL → GREEN295/0, characterization40/0 и строки результатов одинаковы. Это не SDK/native-приёмка. SHA Draw a6bb81c479f77b3791075e8549d73521923cfea35e2f075b111229c3c38d5ff3; вне helper байты равны пользовательскому baseline, Constants/Data неизменны; clang-format/diff-check успешны. Полная TU SDK25/MSVC C++14 /WX скомпилирована родителем в scratch OBJ exit0; APX не собирался. Fresh isolated publishDiagnostics version1 точно совпадают с baseline: пять unused-variable и один unused-include вне helper. Проверены source/OBJ/478 header hashes каждого compile. Roombook.md/_progress и Draw-срез symbols48→49 обновлены, 1031 внешняя запись/порядок сохранены, callgraph исторический. #308 body/comment обновлены с точным read-back OPEN. Evidence: Hermes scratch/skirting-properties-308/.
Next Step: по отдельному разрешению выполнить canonical build/load и native-сохранение свойств/повтор; активный VS Run не останавливать автоматически. Независимое ревью deleg_1f408400 — No issues found in inspected scope, проверенный SHA Draw совпадает с текущим. Wiki RU дополнена; литералы результата в wiki/карточке/#308 сверены с propdesc_prefix_element: some_stuff_element_tip_skirting и some_stuff_element_length_skirting. Issue #308 OPEN: https://github.com/kuvbur/AddOn_SomeStuff/issues/308. Push не запрошен.
Last Checkpoint: коммит текущего этапа #308; родитель 00c6dcc.

## Приёмка #314–316 / #318–320

Scope: адресная проверка шести issues на AC25 Windows; закрытие только по подтверждённым критериям. Владелец разрешил изменения открытой тестовой модели и перезапуски, включая final runner, без сохранения PLN. Production-исправления, commit/push, ветки и чужие изменения вне scope.
Status: PARTIALLY_ACCEPTED — #314/#319 CLOSED; #315/#316/#318/#320 OPEN, критерии не подтверждены полностью.
Plan: [x] 1. Сверить шесть issues и доступные native-сценарии; [x] 2. Принять #314/#319, остальные четыре оставить открытыми с конкретными пробелами; [x] 3. Выполнить final runner, закрытия и read-back GitHub; [ ] 4. Отдельно завершить оставшиеся четыре приёмки на подходящих fixtures.
Last Completed: #314 принят, включая наблюдавшийся no-op, пустой prepared-набор и native undo при введённом отказе callback после накопления 27 pendingResults; #319 прошёл девять native-сценариев с независимым чтением и проверкой исчезновения старых GUID. Final runner: AC25 Windows Debug build/load успешны, APX SHA256 cb05c96272d7857aa049ded7214f2978ce5cb6ec8ecc83a7836c0142d0236adb; exit70, свежий файловый отчёт содержит 207 отказов в тех же шести наборах, что предыдущий аудит. Общий suite не принят. Состояния всех шести issues и опубликованные комментарии прочитаны обратно. Оба PLN совпадают с защищёнными baseline; production, commit/push и ветки не менялись. Подробности закрытых задач — IDEA_ARCHIVE.md → «Адресная приёмка #314/#319 — 2026-11-08». Evidence: Hermes scratch/accept-314-320-20261108/.
Next Step: #315 — разрешить нестрогий stoi/fallback 0 и добиться положительного native-контроля Story; #316 — fixture с Object-избранным и Wall→Object/reuse; #318 — действующий Spec_rule и фактическая запись Integer/String/Real/повтор; #320 — native-границы min_area/отверстия/уклон/старые GUID/повтор. Исправления production вне текущей проверки. Docs/generated не обновлялись: исходники не менялись, обновление карточек отложено до команды на документацию/коммит; native-оговорка #319 в Roombook.md теперь историческая.
Last Checkpoint: e0dfc7d; коммит не запрошен.

## #298 — сохранение дуг при конвертации Polygon2D

Scope: `CommonFunction.cpp::ConvertPolygon2DToAPIPolygon`, адресная регрессия и регистрация, документация. Проверка AC25 Windows; остальные SDK/macOS не проверяются. Чужие правки, PLN, ветки, commit/push и перезапуск Archicad вне scope.
Status: WAITING_FOR_NATIVE.
Plan: [x] 1. Проверить SDK/потребителей и выполнить RED на реальных дугах; [x] 2. Минимально исправить направление копирования и проверить GREEN; [x] 3. clang-format, LSP, canonical AC25 build и документация; [ ] 4. Native-приёмка writers отдельно по согласованию.
Last Completed: одна строка dest/source исправлена, +1 сохранён. Реальная SDK25 Geometry/BM: baseline RED110/22 → GREEN134/0, дополнительные 24 обратные проверки на RED безопасно пропущены. Прямые/разнознаковые/замыкающие дуги, отверстие, reuse, исходник и обратная конвертация проверены; постоянный suite зарегистрирован. Canonical AC25 Windows RelWithDebInfo INT BuildAddOn.py exit0, CommonFunction перекомпилирован/APX перелинкован; полные TU теста/registry с TESTING MSVC/C++14 /WX exit0. Fresh clangd: у converter те же 5 unused-диагностик и 2 include hints, что baseline; тест/registry без ошибок. Отдельное открытие не-self-contained TestFunc.hpp без PCH даёт унаследованные API-type errors; через реальные TU компиляция успешна. Карточки и адресные symbols обновлены; callgraph исторический, не пересобран. Оба PLN совпадают с baseline SHA256. Evidence: Hermes scratch/issue298/.
Next Step: по отдельному разрешению выполнить native create/update затронутых Roof/Slab writers на дуговом контуре и повтор; полный TestKit в Archicad и другие SDK/macOS not verified. Issue остаётся открыта, commit/push не выполнялись.
Last Checkpoint: e0dfc7d; новый коммит не запрошен.
Issue: https://github.com/kuvbur/AddOn_SomeStuff/issues/298.

## #292 — корректный вывод проверок TestKit

Scope: `tests/TestKit` и независимая регрессия вывода bool и указателей; документация по необходимости. Проверка AC25 Windows; другие версии и macOS не проверяются. Чужие изменения, PLN, ветки, commit/push и перезапуск Archicad не затрагивать.
Status: WAITING_FOR_NATIVE.
Plan: [x] 1. Проверить маршрут вывода и воспроизвести ошибку независимой регрессией; [x] 2. Минимальное исправление и постоянная регрессия; [x] 3. Форматирование, сборка, проверки и документация; [ ] 4. Native-приёмка отдельно по разрешению владельца.
Last Completed: bool и const void* передают фактические значения в Report. Standalone MSVC с реальными TestKit.hpp/cpp и подменой GS: RED на двух PASS-строках, GREEN 6 проверок вывода и счётчиков; registry 91/91/91. clang-format, git diff --check, clangd 0 ошибок; AC25 Windows RelWithDebInfo INT BuildAddOn.py exit 0, но его лог не подтверждает компиляцию TestKit. Карточка TestFunc дополнена; generated не регенерирован. Archicad не перезапускался.
Next Step: после отдельного согласования проверить вывод в Archicad и завершить issue; другие версии и macOS не проверялись.
Last Checkpoint: e0dfc7d; новый коммит не запрошен.
Issue: https://github.com/kuvbur/AddOn_SomeStuff/issues/292.

## Task

Реализовать #317: не удалять существующую стену при отказе hosted-проёма; передать требование отката после начала разрушительной записи через OtdRoom_Draw в существующую undo-команду. Аудит issues завершён; история в IDEA_ARCHIVE.md.

## Scope

AC25 Windows. Roombook.hpp (OtdDrawData), RoombookDraw.cpp (Wall writer, dispatch и три вызывающих цикла), адресные регрессии и документация. Сохранить R1/#316, #314, обычный continue до изменения модели и cleanup новой стены. Геометрия, JSON-схема, чужие hosted-элементы, другие writers, PLN, commit/push и ветки вне scope. Перезапуск Archicad требует отдельного разрешения.

## Status

WAITING_FOR_NATIVE — production #317 реализован, controlled RED/GREEN, сборка и независимое ревью пройдены. Native-восстановление стены/проёмов не проверено; issue остаётся открытой.

## Last Completed

- #317: родитель проверил 41 hash manifest и пять полных extracted-функций против live source, повторил оба suite на baseline/live: characterization105/0 в обоих вариантах, regression RED205/117FAIL → GREEN205/0. SDK25 C++14 MSVC. Проверены маркер до API-попытки, все три caller-а, новые/существующие стены, stale flags и отсутствие публикации pendingResults/Sync при ошибке callback. Undo/API — controlled seams, не native. Независимое ревью production-дельты без замечаний.
- #317: маркер до destructive Delete/Change/hosted Create, существующий parent больше не удаляется при отказе проёма; cleanup новой стены сохраняется, его отказ пробрасывается. Все три вызывающих цикла передают ошибку. Canonical AC25 Windows RelWithDebInfo INT build exit0, source hashes неизменны; оба PLN и загруженный Debug APX по SHA256 неизменны. Fresh isolated clangd: те же шесть диагностик, что baseline; MCP AST устарел, его нулевой результат не принят. clang-format и diff --check успешны. Generated: 48 Draw joins, 22 координаты обновлены, внешний срез сохранён; callgraph не пересобран. Evidence: Hermes scratch/roombook-317-20261108/.
- Проверены актуальные состояния всех 39 issue-ссылок прежней IDEA: 25 уже CLOSED; их устаревшие OPEN/WAITING-записи исключены из активного состояния.
- Закрыты #321 (e0dfc7d, исходный native-сценарий отказа Roof) и #143 (a6feb208 и поздние правки, чтение свойств стройматериала по индексу с production-callsite). Ограничения проверки указаны в closing comments; итоговый GitHub read-back подтверждает CLOSED.
- Финальный restart_archicad_for_test.ps1 выполнен 2026-11-08: canonical AC25 Windows Debug build успешен, Archicad PID17448 открыл Test_file/test_25.pln; модуль Build/SomeStuff/25/Debug/SomeStuff.apx прочитан из списка загруженных модулей.
- Runner exit70: он сообщил об ошибках по файловому отчёту, который продолжал обновляться после завершения скрипта. Свежая панель Visual Studio «Отладка» отсутствует; успешный C++ прогон not verified. JSON-тесты пропущены: Tools/test_json_commands.py отсутствует. Build/load не выдаются за поведенческую приёмку.
- Test_file/test_25.pln не изменён; Build/SomeStuff/25/test_25.pln восстановлен из защищённой копии после штатного перезаписывания сборщиком. Оба SHA256 совпали с baseline.

## Next Step

По отдельному разрешению выполнить native-сценарий #317: отказ Change после удаления старых проёмов и отказ нового hosted-проёма должны восстановить прежнюю стену/проёмы; проверить оба вида плинтусов и отсутствие результатов откатившегося пакета. Final runner также согласовать. Старый общий RoomBook не является этой приёмкой; commit/push не запрошены.

## Last Checkpoint

e0dfc7d — последний существующий checkpoint. Новый коммит #317 не запрошен; изменения оставлены в рабочем дереве, push не выполняется.

## Plan

- [x] 317.1 — проверить контракт отката и завершить минимальную production-правку; SDK25/source и canonical build подтверждены.
- [x] 317.2 — выполнить регрессии baseline/live и ревью; форматирование, свежий LSP и AC25 build завершены.
- [x] 317.3 — обновить карточку модуля, generated-координаты и состояние с проверкой адресного среза.
- [ ] 317.4 — native-приёмка и final runner после отдельного разрешения; закрытие issue отдельно.

## Незавершённые рабочие потоки

| Issue | Подтверждённое исходное состояние / следующий шаг |
|---|---|
| #320 | Фильтр min_area закоммичен d066500. Отдельно проверить границы порога, старые GUID и повтор в native; исходный controlled stand не равен native-приёмке. |
| #318 | Исправления повторного ревью закоммичены 4813d74 поверх e84e042. Отдельно проверить фактическую запись Integer/String/Real и повторный запуск Spec. |
| #317 | Production R2 применён; canonical AC25 Windows RelWithDebInfo INT, controlled RED/GREEN и независимое ревью пройдены. Осталась native-приёмка восстановления стены/проёмов. |
| #316 | R1 входит в a28fe78. Осталась native-приёмка перехода Wall→Object и повторного GUID reuse; общий RoomBook не доказывает этот переход. |
| #315 | Story level/height входят в 78635c8 и последующие правки; ключ — отображаемый ui_index. Нужны native-проверки обоих правил и граничных индексов; отдельно определить/проверить некорректный аргумент: catch/stoi может выбрать 0 или принять числовой префикс. |
| #308 | length_skirting реализован в OtdBeam_ProcessFinishes. RoombookQuantities.cpp:310–313 уже включает его в ведомость, хотя body ограничивает задачу полем: сверить дальнейшее решение владельца и итоговые значения/повтор, не откатывать интеграцию автоматически. |
| #295 | Исправления привязок верхних откосов требуют native-матрицы create/update/repeat. Старые блокировки компиляции не считать действующими после успешной общей сборки. |
| #260 | Рефакторинг остаётся частичным. Исторические C1–C9 и отдельные инкременты сохранены в архивном снимке; нужна сверка оставшихся критериев, документов и native-эквивалентности. Старые PID/VS Run и фразы «не закоммичено» не являются актуальными блокерами. |
| #284 | Основная классификация граней есть, но RoombookDraw.cpp:357–372 для горизонтального slab Wall_Main не применяет create_wall_elements (default=true). Исправление и адресная приёмка — отдельная задача. |
| #298 | Копирование дуг исправлено в рабочем дереве; SDK25 регрессия и AC25 Windows сборка пройдены. Осталась native-приёмка writers, см. блок #298. |
| #292 | Actual bool/const void* исправлен и проверен standalone; native-приёмка остаётся. |
| #258 | Позиционные ID ресурсов подписей Spec — отдельный открытый дефект; не смешивать с исправлениями ReNum/#256. |
| #311 | Подсветка свободных точек размерной цепочки: постановка, реализация не подтверждена. |

Остальные открытые задачи (полные индивидуальные вердикты — в архивном отчёте):

- Приёмочные сценарии: #171 CompareParamValue/baseline; #168 тумблеры меню; #166 PERF; #164 сравнение Roombook с прежней сценой; #163 observer/SetAutoclass.
- Палитра и сброс: #159 активность флагов, #155 быстрый сброс, #189 выборочный сброс, #187 выбор свойства/автоформат, #186 горизонтальный режим. HTML содержит mock сброса и незавершённый редактор, поэтому наличия C++-моста недостаточно.
- Исследование/оптимизация: #156 классификатор LightRAG, #198 производительность; #217 счётчики кэшей удалены, для закрытия как снятого направления нужно решение владельца.
- Удалённые TableRenderer/TablesNavigator: #177/#176 не являются выполненной поставкой и не закрыты как completed.
- Остальной функциональный backlog: #149 разделение откосов; #142 состав по имени; #141 толщина из откоса; #136 проверка slab; #130 фильтр; #129 GUID GeneralElemProperties; #128 поиск класса; #115 координаты проёмов; #103 общий helper флагов; #93 Sync_group; #72 размеры колонн; #69 выбор правил по acttype; #62 исходный участок декомпозиции не установлен; #48 hex alphanum; #2 PDF → макеты.

## Decisions

- Не создавать ветки; не коммитить и не пушить без явной команды.
- Завершённые на GitHub #252/#270 и другие закрытые потоки не возвращать в активный план по историческим записям.
- История до аудита сохранена целиком, кроме «Грабли», которые остаются ниже в IDEA; старые утверждения о OPEN/коммитах/блокировках в архивном снимке относятся к моменту записи.
- Evidence: C:/Users/da-rogojin/AppData/Local/hermes/cache/scratch/issue-audit-20261108/ (before-manifest.json, idea-issue-states.json, final-runner.log, final-runner-result.json).

## Archive

Завершённый аудит: IDEA_ARCHIVE.md → «Аудит открытых issues SomeStuff — 2026-11-08» (все 47 индивидуальных вердиктов, два закрытия, проверки и ограничения). Прежние подробные записи: «Снимок IDEA до аудита 2026-11-08». Активный статус берётся из текущей IDEA и GitHub, не из архивных Next Step.

## Грабли


- **ID контрола в `'GDLG'` = позиция строки, а не число в комментарии.**
  `/* [ n] */` — пояснение для человека, компилятор его не читает: кнопка,
  объявленная раньше списка, получает ID 3, а список — ID 4. Симптомы
  неочевидые: список пуст (нет заголовков/строк), а подпись из `DGSetItemText`
  уходит «в никуда», и кнопка показывает дефолтный ресурсный текст.
  Компилятор ресурсов такое НЕ диагностирует — молчит. Всегда держать
  `[n]` по возрастанию в порядке строк; при добавлении кнопки в середину —
  дописывать её последней строкой. То же для `'DLGH'`.
- **Выделение в 3D-окне требует СНАЧАЛА перехода в это окно.**
  `ACAPI_Selection_Select` / `ACAPI_Element_Select` выделяют элементы только
  ТЕКУЩЕЙ БД: до перехода в 3D элементы чужой базы выделить нельзя
  (документирован `APIERR_BADDATABASE`). Рабочий порядок для показа элементов
  из другой базы: `ShowAllIn3D` → выделение → подсветка → `ZoomToElements` →
  redraw. Подсветка цветом, в отличие от выделения, работает и для элементов
  чужой базы (контракт — «in the 2D … and 3D window»), поэтому она и нужна
  поверх выделения. Отказ выделения/зума не должен прерывать остальные шаги.
  Ошибка воспроизведения: «выделить GUID'ы, потом уйти в 3D» — выделение
  срабатывает в плане, но в 3D элементы не выделены; «сначала перенести окно»
  — элементы становятся недоступны вовсе.
- **Собранный `.apx` может содержать устаревший ресурс.** `AddOn.grc`
  генерируется из `Tools/AddOn.grc.in` на этапе конфигурации CMake, поэтому
  правка шаблона без пересборки не попадает в аддон; при отладке GUI всегда
  сверять mtime `.apx` и `ResourceObjects/AddOn.grc.rc2` с текущим временем.
  Побочный эффект: временная подмена шаблола ради проверки гипотезы
  «сбивает» сгенерированный `.grc` — восстановить нужно и то, и другое.
- **HTML вшит в ресурс** (`'DATA' ID_ADDON_HTML`) — правки `Interface_ru.html`
  требуют пересборки; после каждой правки `Tools/test_html.ps1`.
- **Дубли строк классификации**: C++ кладёт словарь и под `systemname`, и под
  `systemname_full` → записи ×2 (JS-дедуп `uniqueClassificationOptions`).
- **Ранний `return` по `data.common`** прятал выбор классификации и «Применить
  ко всем» при единой классификации/одном элементе.
- **prefs проекта ломают Teamwork** — настройки только в локальный
  `…/GRAPHISOFT/SomeStuff/SyncSettings.dat`; `ReadSyncSettingsFromFile`
  отвергает чужую `PreferencesVersion` → новый массив в настройках требует bump
  версии.
- **Undo-области на каждый элемент** (ResetProperty.cpp) — сотни undo-шагов;
  док DevKit требует одну область на действие.
- **Ключи кэша параметров всегда в нижнем регистре** (prefix + ToLowerCase +
  BRACEEND) — имя без нормализации регистра = правило молча не срабатывает.
- **Мост = инлайн-функции `RegisterACAPIJavaScriptObject`**; аргументы
  `JSFunction` парсить через `DynamicCast<JSValue>` — `DynamicCast<JSArray>`
  ронял ArchiCAD. Ответ моста — строка JSON: вложенные `DG::JSObject`/`JSArray`
  при передаче в JS теряются.
- **Подсветка**: `APIIo_HighlightElementsID` + `APIDo_ZoomToElementsID`
  триггерят SelectionChangeHandler → `static suppressSelectionRefresh`.
  `SetElementHighlight` только AC26+/27+; AC25 — прямой `ACAPI_Interface`,
  сброс = вызов без par1, Clear перед Set.
- **Данные «Монитора» — только из `PROPERTYCACHE()`**, значения — по
  элементам отдельно; в кэше `property` — только определения.
- **`gh` CLI не в PATH** — `export PATH="$PATH:/c/Program Files/GitHub CLI"`;
  Reviews/ в .gitignore (BOM+LF, править через python `utf-8-sig`).
- **Постоянный MSBuild-узел VS держит `.tlog`** (`nodeReuse:true`,
  `MSB4018 … read.1.tlog`) — процесс живёт часами и при этом **не** означает
  идущую сборку: проверить `Get-CimInstance Win32_Process` по `CommandLine` и
  просто повторить `BuildAddOn.py`, а не ждать и не убивать узел.
- **Править `_generated/*.json` можно только вручную**: генератор обнуляет
  `callgraph.json`, а `json.dump(..., indent=2)` из Windows-скрипта даёт CRLF —
  стиль HEAD: LF, без BOM, **без завершающего LF**.
- **Снято автором (не фиксить)**: `Dimensions.cpp:158` `pen_original`;
  пересоздание элементов отделки в Roombook; `Sync.cpp:419-428`
  накопительный `epm`.
- **MSVC не выдаёт часть диагнозов clang — локальная зелёная сборка Windows
  не означает, что mac-сборка в CI зелёная.** mac собирает с `-Werror` и
  расширенным набором `-W…`, поэтому диагностики, молчащие в MSVC, роняют
  mac-job'ы. Поймано два диагноза подряд: `-Wunused-lambda-capture`
  (`SpecPlanning.cpp:283`) и неоднозначный `operator=` при `GS::Array = {}`
  (`CommonFunction.cpp:2769`). Windows при этом зелёная во всех пяти версиях —
  и это выглядит как проблема кода под macOS, хотя дело в инструменте.
  **`GS::Array` очищать только `Clear ()`, не `= {};`:** у него есть шаблонный
  `operator=` через `ConversionEnumerator`, и пустой список инициализации
  неоднозначен. `= {};` допустимо для POD-структур и `time_point`.
- **Имитация mac-сборки через clang-cl не работает — не тратить на неё время.**
  Проверено: у `clang-cl` нет части флагов mac (`-std=gnu++20` → `/std:c++20`),
  а локальные заглушки `Sources/AddOn/api_headers/` (только под `_MSC_VER`, в
  mac-сборку не входят) дают диагностики `-Wreserved-macro-identifier`/
  `-Wc++98-compat`/`-Wundef`, которых в mac-логе нет. Отличить свои артефакты
  от дефектов не вышло. Такие классы ошибок надёжнее закрывать по логу CI.
  Если проверка всё-таки нужна, `-W…` нельзя давать после исходника (clang-cl
  считает их файлами), `/Zs` несовместим с `-c`, PCH из БД на диске
  отсутствует, `-D вида -DADDON_NAME=\"X\"` нельзя переразбирать `shlex`
  (кавычки теряются → `missing terminating '"'`), нужны `-DACExtension`,
  `-DAC_25`, `/EHsc` (иначе падает `exprtk.h`).
