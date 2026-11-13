# Current Task — #323: безопасное чтение свойств палитрой

## #326 — сложный профиль существующих плинтусов

Scope: по «Исправляй» — только Beam_Draw, адресная регрессия/документация, AC25 Windows/C++14. GUID, расчётную геометрию, несвязанные настройки и чужие изменения сохранить. Модель/избранное/PLN/host, APX/build/load, commit/push/ветки вне scope.
Status: WAITING_FOR_NATIVE — реализация и офлайн-проверки завершены, issue OPEN.
Plan: [x] 326.D — диагноз; [x] 326.F1 — SDK25/baseline/RED; [x] 326.F2 — минимальная правка/GREEN; [x] 326.F3 — форматирование/TU/LSP/ревью; [x] 326.F4 — Docs/generated/issue/память; [ ] 326.N — согласованные APX build/load и native-приёмка.
Root Cause: implementation error — старый Beam_Draw применял Favorite memo только при создании; существующий GUID оставался со старым modelElemStructureType/profileAttr даже при Change из-за геометрии.
Last Completed: в Beam_Draw:772–973 синхронизированы только управляемые поля section, активные profileAttr/материал/end-размеры; итог сравнивается после geom override. Существующий memo/GUID, inactive и output-only relativeHomeStoryLevel сохранены; вне Beam_Draw байты и инвентарь ACAPI-вызовов идентичны baseline.
Validation: parent-run exact writer/C++14, 19 случаев RED11FAIL → GREEN0FAIL (GREEN745assertions); второй проход no-op. Типы/API/helpers/persistence controlled, не SDK/native. Guards/upper-reveal — static review only. Полная TU SDK25/MSVC14.44 C++14 /WX /c exit0 в scratch OBJ; свежий isolated LSP version1 — те же 5 unused-variable + 1 unused-include, новых нет. Все2146headers baseline/current совпали. clang-format/diff-check пройдены. Ревью deleg_af16a33b: No issues found in inspected scope. Source56140f81241fd2931b2ea9531cd91526b34c20f1898ed1dedabcd9e9e6ace390 совпал с GREEN/TU/LSP/ревью/live.
Docs/Memory: карточка Roombook и _progress обновлены; все49Draw-symbols сверены,26координат обновлены,1037внешних записей/порядок сохранены; callgraph исторический. Body issue прочитан обратно и совпал с отправленным, OPEN. Итог передан OpenViking; методика дополнена в lightrag/SKILL.md.
Next Step: после отдельного разрешения собрать/загрузить новый APX и проверить исходный Basic→Profile, A→B, Profile→Basic, оба вида плинтуса, GUID/повтор/no-op; отдельно custom/stretched profile payload/модификаторы. Нативная регенерация assemblySegmentProfiles по смене profileAttr — not verified; массив/nProfiles не подменяются.
Last Checkpoint: c51ff77; commit/push не выполнялись и не запрошены.
Issue: https://github.com/kuvbur/AddOn_SomeStuff/issues/326.
Evidence: Hermes scratch/skirting-profile-diagnosis/, skirting-profile-fix/, skirting-profile-offline/, skirting-profile-parent/ (review-acceptance.json, symbols-acceptance.json, issue326-after-final.json).

## #315 — Story: свойство как индекс и собственный этаж

Scope: утверждённая реализация SyncStringStory, существующая подготовка зависимости в ParseSyncString, ReadStory/кэш, адресные тесты и документация; AC25 Windows/C++14. Rawname — val.uniStringValue через ParamHelpers::NameToRawName, литерал — optval.intval/hasintval, не composite_pen. Нового поля rawname и pre-read в ParamHelpers::Read нет. Пользовательские правки сохранить; ветки/commit/push, перезапуск Archicad и изменения PLN вне scope.
Status: WAITING_FOR_TEST — реализация, локальные проверки, независимое ревью и адресная документация завершены и сверены родителем. Native-приёмка отдельно, не разрешена текущей командой; issue остаётся открытым.
Plan: [x] 315.1 — проверить issue и текущий маршрут; [x] 315.2 — согласовать синтаксис и поля; [x] 315.3 — план утверждён; [x] 315.4 — baseline RED, минимальная реализация и GREEN; [x] 315.5 — форматирование, свежий LSP/AC25 build, независимое ревью; [x] 315.6 — документация/generated/wiki и состояние; [ ] 315.N — native-приёмка по отдельному разрешению.
Last Completed: внешний Story имеет адресный приоритет; литерал/ссылка/self различимы, пустая позиция индекса сохранена. Пользовательский dependency route сохранён с !hasintval; ReadStory использует готовую зависимость и уже прочитанный elem_head, self — Story.index того же кэша. Невалидность без fallback; новых ACAPI-вызовов нет. RED178/74 → GREEN181/0; permanent parser63/0; extra reader/cache98/0; независимый probe56/0 повторён parent. GS SDK25 реальные, Property/GetStories/converter/часть форматирования controlled; полного ParseSyncString и native нет. TestSync TU /DTESTING и Sync/Helpers TU SDK25/MSVC C++14 /WX compile0; fresh LSP без новых диагностик. Canonical AC25 Windows INT RelWithDebInfo build0, APX скомпонован (SDK PDB LNK4099 warning). Ревью без замечаний. Карточки CommonFunction/Helpers/Propertycache/Sync, русская страница команд и progress обновлены; parent проверил symbols180/0, 908 внешних записей/порядок сохранены, исходные хеши совпадают с build/review. Английская wiki, AI-context и общий callgraph не обновлялись.
Next Step: по отдельному разрешению native-проверка обоих режимов нулевого этажа, значения свойства на другом home-floor, изменения свойства между запусками и отказа без старого результата. Не перезапускать Archicad и не менять PLN самостоятельно; commit/push/закрытие issue не разрешены. Промежуточные отказы scratch-обвязки и ложная insertion-only предпосылка progress-аудита исправлены только в проверяющем коде; исходники продукта ими не затронуты.
Last Checkpoint: cc6f000 — [315] Индекс этажа из свойства и собственный этаж в Sync_from{Story:level/height}; checkpoint #326-восстановления: c47a137.
Issue: https://github.com/kuvbur/AddOn_SomeStuff/issues/315 (OPEN).
Evidence: .hermes/plans/2026-11-12_181956-story315.md; Hermes scratch/story315-implementation/: baseline/manifest.json, regression/parent-green1/summary.json, persistent-tests/result.json, extra-reader/result.json, review/{review.md,report.json}, offline/comparison.json, docs/{docs-acceptance,symbols-acceptance}.json, build-ac25.log, scope-proof.json, parent-final-acceptance.json. Исходные пользовательские правки сохранены в baseline; прежние story315-api.json/story315-plan.md — исследование, не доказательство реализации.

## #325 — разбор уровня коррекции QR

Scope: только разбор и запись уровня в пользовательской заготовке SyncStringQRCode; AC25 Windows для проверки. Существующие пользовательские правки Helpers.cpp и CommonFunction.hpp сохранять; интеграция передачи через кэш и native-приёмка — отдельный следующий этап.
Status: PARTIAL — запрошенный разбор реализован и проверен; сквозная функциональность issue не завершена.
Plan: [x] 325.1 — изучить заготовку и парсер; [x] 325.2 — разбор и запись уровня; [x] 325.3 — адресная регрессия и сборка; [x] 325.4 — перенос optval в путях копирования (Propertycache.cpp:126, Helpers.cpp:1829/1871/6025/6558), clang-format/diff-check и AC25 RelWithDebInfo INT build успешны; [ ] 325.N — native-приёмка в Archicad.
Last Completed: SyncStringQRCode извлекает 1–4 и удаляет параметр из правила. Стенд с дословной функцией и заглушками MSVC C++14: 10 проверок, 0 ошибок. Передача уровня исправлена в пяти точках копирования ParamValue: SetParamValueFromCache, AddParamDictValue-пути и GDL-перенос; BuildAddOn.py -v25 -r -l INT exit 0 (Helpers/Propertycache перекомпилированы, APX перелинкован). Clangd-диагностики вне функции не сравнивались с baseline. Issue #325 OPEN. Evidence: Hermes scratch/qr325-child/.
Next Step: native-приёмка уровней 1–4 и автоподбора в Archicad; поведение некорректных уровней и переполнения QR не согласовано (открытые вопросы issue). Generated-координаты Sync не обновлялись.
Wiki #325: в wiki/ru/Property-Commands-List-ru.md и wiki/ru/QR-Code-Output-ru.md добавлены синтаксис QR и уровни с оговоркой о незавершённой интеграции; runtime не проверялся.
Last Checkpoint: fcee5ea; коммит не запрошен.

## #323 — BrowserPalette / ODB assertion

Scope: AC25 Windows/C++14; BrowserPalette.cpp/.hpp и регистрация callback в SomeStuff_Main.cpp при необходимости, адресные регрессии и документация. Сохранить формат JS-ответов и жизненный цикл палитры. Не менять Helpers/RoomBook, PLN, настройки хоста; restart/commit/push/ветки вне scope. Другие SDK/macOS — только проверка контрактов, без заявления о сборке.
Status: WAITING_FOR_NATIVE — защитная правка, регрессия, финальные сборка/LSP и независимое ревью завершены; исходный crash не принят в host.
Plan: [x] 323.1 — перепроверить assertion/exception, точный PDB callback и SDK-контракт; [x] 323.2 — RED, минимальная правка и GREEN; [x] 323.3 — форматирование, свежий LSP, AC25 build и независимое ревью; [x] 323.4 — документация и границы native-приёмки; [ ] 323.N — отдельный native-повтор после разрешения загрузки новой сборки.
Last Completed: matching dump/APX/PDB и source MD5 установили GetPropertiesList (:460 до правки); assertion и CLR exception различены. Wrapper четырёх readonly callbacks, modeless-depth/reentry, Reset read-stage до отдельного undo, pending/PanelIdle без синхронного чтения Begin/End. Ревью нашло hidden DisableEnd и Reset selection/filter до admission: добавлены пять regression-checks, оба дефекта исправлены. Parent-run одинаковый MSVC C++14 checker108: original79FAIL → pre-review5FAIL → final0FAIL, все compile_exit0; model callbacks/DG/GS/ACAPI/undo controlled, не native. После финальной правки повторены canonical AC25 Windows RelWithDebInfo INT BuildAddOn.py exit0 (BrowserPalette compile/APX link), формат/diff-check и fresh isolated publishDiagnostics version1/documentSymbol. LSP те же три прежних сообщения, два severity1 и одно severity2, новых нет. Финальный cpp3bb8dbca238f1a9dcf9b11ea2d85c2f8f8bfdce6955a8a43c0811ea5c2d59c50/header d42375c4; нормальные тела четырёх readers и undo токены сохранены. Карточка/срез symbols36→41 обновлены по fresh AST,1045внешних записей/порядок сохранены, callgraph исторический. Debug APX и оба PLN неизменны, #324 сохранена.
Next Step: только по отдельному разрешению загрузить финальный APX и повторить исходный сценарий с property expressions/3D: успешный CallCommand admission не доказан как универсальный nested-3D barrier, modeless/PanelIdle не гарантируют безопасность. Исходный crash/native и другие SDK/macOS not verified; #323 OPEN, push/перезапуск не запрошены; коммит этапа выполняется по команде владельца. Follow-up ревью deleg_b5f33ae5: No issues found in inspected scope, оба P2 устранены, cpp/header hashes совпали с финальным source/build/GREEN. Итоговые артефакты: parent-final-acceptance.json, build-ac25-final.log; APX Build/SomeStuff/25/INT/RelWithDebInfo/SomeStuff.apx SHA2565bc21ca3e14436f525539aa3a28881c105bbfc28022942cb428c66048e3d70a1. Процедура и границы сохранены в archicad-fastlog/references/odb-thread-modification-assert.md.
Last Checkpoint: настоящий коммит [323.4] (4465089); предыдущий 7366144.
Issue: https://github.com/kuvbur/AddOn_SomeStuff/issues/323.
Evidence: C:/Users/da-rogojin/AppData/Local/hermes/cache/scratch/browser-odb-20261112/.

## #322 — RoomSurfaces_Partition

Scope: AC25 Windows/C++14. Локализовать ошибки источников/граней в RoomSurfaces_Partition и передать их в отчёт без общего отказа RoomProcessingContext_PreparePartitions; сохранить пригодные участки, пользовательские continue, допуски и штатные writers. Адресные регрессии и документация входят; изменение discovery/read-ошибок, политики удаления, геометрических допусков, PLN, перезапуск host, commit/push/ветки — вне scope.
Status: WAITING_FOR_NATIVE — код, standalone-проверки, сборка, LSP и независимое ревью завершены.
Plan: [x] 322.1 — прочитать пользовательскую дельту, контракт и потребителей; зарегистрировать issue; [x] 322.2 — RED, минимальная локализация отказов и GREEN; [x] 322.3 — форматирование/LSP, AC25 build, ревью и документация; [ ] 322.N — native-повтор исходной зоны после согласования.
Last Completed: baseline с четырьмя continue владельца сохранён (Data8bc42870). Реализованы локальный source-Boolean, исключение конфликтной группы, изоляция zone-face и честный complete; caller выводит все errors без abort partition. Parent-run SDK25 Geometry/GSRoot/C++14 exact helpers/partition/caller RED71/24→GREEN71/0; preparation/report и throw zone-face controlled seams. Семь постоянных assertions исполнены exact extraction RED7/7→GREEN7/0; полный test TU TESTING /WX скомпилирован. Canonical BuildAddOn.py -c config.json -v25 -r -l INT exit0, Data compile/APX link подтверждены; Data SHA57ad0a0aa0d67435517a1f42a1cd1451fb511cae49a62bd0ba708c613557a320. Формат/diff-check успешны. Fresh isolated LSP version1 baseline/live имеют одинаковые восемь диагностик (три unused-variable, пять unused-includes), test без диагностик, header проверен в TU; parent сверил hashes и сравнение. Baseline --check timeout на token features не считается pass. Независимое ревью deleg_c2267fc8: No issues found in inspected scope. Карточка и три адресные записи symbols обновлены, две test-координаты сверены; остальной generated/callgraph исторический. Оба PLN и загруженный Debug APX неизменны по SHA256, host не прерывался. Штатный cleanup может удалить старую отделку отсутствующего участка: политика не менялась и сохранность не гарантируется.
Next Step: после отдельного согласования загрузить новую сборку и повторить исходную зону со сложным профилем: ошибки в отчёте, независимые элементы созданы; затем проверить повтор и штатный cleanup. Native-выполнение и полный host suite не проверены; #322 оставить открытой.
Last Checkpoint: a3dc151; commit/push не запрошены.
Issue: https://github.com/kuvbur/AddOn_SomeStuff/issues/322.
Evidence: C:/Users/da-rogojin/AppData/Local/hermes/cache/scratch/partition-nonblocking-20261111/.

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

- #325 — опциональный уровень коррекции QR в Sync_from; постановка: https://github.com/kuvbur/AddOn_SomeStuff/issues/325. Реализация не начата.
- Не создавать ветки; не коммитить и не пушить без явной команды.
- Завершённые на GitHub #252/#270 и другие закрытые потоки не возвращать в активный план по историческим записям.
- История до аудита сохранена целиком, кроме «Грабли», которые остаются ниже в IDEA; старые утверждения о OPEN/коммитах/блокировках в архивном снимке относятся к моменту записи.
- Evidence: C:/Users/da-rogojin/AppData/Local/hermes/cache/scratch/issue-audit-20261108/ (before-manifest.json, idea-issue-states.json, final-runner.log, final-runner-result.json).

## Archive

Палитра #324 завершена и проверена владельцем; коммит fcee5ea, issue CLOSED. См. IDEA_ARCHIVE.md → «Палитра: сохранение пользовательского скрытия». Push не выполнялся.

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
