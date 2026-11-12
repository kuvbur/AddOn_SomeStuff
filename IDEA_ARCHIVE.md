# IDEA Archive
Завершённые задачи и отменённые направления из `IDEA.md`. Новая запись — сверху (сразу под этим абзацем),
старые — ниже.
---
## Адресная приёмка #314/#319 — 2026-11-08

Status: COMPLETED для #314/#319; #315/#316/#318/#320 остаются OPEN, их остаток — в активной IDEA.md.
Scope: проверка существующего кода AC25 Windows, изменения только несохранённой тестовой модели, final runner и GitHub. Production-правки, commit/push, ветки и другие SDK/macOS не выполнялись.

- #314 (a28fe78): 25 зон/1172 подготовленных записей; отключение двух зон вернуло 20 null-GUID записей, включение создало 47 элементов. True no-op наблюдался через live guard Wall writer и сопоставлен с JSON. Пустой валидный prepared-набор создан очисткой четырёх массивов в отладчике и вернул types:[]. Введённый отказ callback при 27 pendingResults дал rooms:[] и восстановление полного native GUID-инвентаря. Инъекция проверяет SDK undo/публикацию, не реальный отказ SDK writer и не hosted-wall восстановление #317. Раскрытие составов/свойств вне #314. CLOSED подтверждён GitHub read-back, closing comment6724099209.
- #319 (99238e6, b588359): девять native-сценариев — оба непустые, каждый вид пустой отдельно с повтором, оба пустые с повтором, восстановление и повтор. Проверены prepared-роли 10/11, успешные native GUID, исчезновение старых GUID отключённого вида, сохранение другого вида и отсутствие churn на повторах. Тестовые описания/значения свойств восстановлены независимым read-back. CLOSED подтверждён GitHub read-back, closing comment6724099593.
- #315: по исходникам нестрогий stoi принимает числовые префиксы и fallback 0; SyncAll-матрица оставила sentinel даже для валидных индексов, поэтому native Story-ветка не считается исполненной. #316: в текущем fixture нет Object-избранного. #318: Spec вернул readParamsNotFound с пустыми rules, записи нет. #320: адресная native-матрица min_area не исполнена. Все четыре OPEN и точные тексты комментариев проверены повторным чтением.
- Final Tools/restart_archicad_for_test.ps1 выполнен: canonical AC25 Windows Debug build успешен, Archicad открыл Test_file/test_25.pln и загрузил Build/SomeStuff/25/Debug/SomeStuff.apx; SHA256 cb05c96272d7857aa049ded7214f2978ce5cb6ec8ecc83a7836c0142d0236adb совпал с адресной приёмкой. Exit70: свежий изолированный файловый отчёт содержит 207 отказов в тех же шести наборах, что предыдущий аудит; общий suite НЕ принят, свежий успешный прогон через панель Visual Studio «Отладка» не подтверждён. JSON-автотесты runner пропущены: Tools/test_json_commands.py отсутствует. Build/load не заменяют адресные native-сценарии.
- Оба PLN восстановлены побайтово к защищённым baseline. Hash manifest production-исходников совпал; Save, commit/push и новые ветки не выполнялись. Исходные breakpoints восстановлены, собственные удалены; финальный runner оставил Archicad без отладчика. Модульные карточки/generated не обновлялись без команды на документацию/коммит; историческую native-оговорку #319 нужно актуализировать при таком обновлении.
- Evidence: C:/Users/da-rogojin/AppData/Local/hermes/cache/scratch/accept-314-320-20261108/ — acceptance-source-manifest.json, skirting-results.json, final-runner.log, final-runner-result.json, final-runner-temp/somestuff_test_report.txt, fixture-preservation-final.json, github-final-readback.json. Процедура сохранена в archicad-json-commands/references/roombook-native-acceptance.md.
Last Checkpoint: e0dfc7d; новый коммит не запрошен. Next Step: только открытые критерии #315/#316/#318/#320; закрытые #314/#319 не возвращать в active backlog.

## Аудит открытых issues SomeStuff — 2026-11-08

Снимок кода: e0dfc7da5ed7d1abfa093ee5c5febafb77c92b9f. Исходный список: 47 открытых issues. Проверены полные тела и комментарии GitHub, исходники и история; независимые read-only проверки выполнены тремя агентами, выводы о закрытии дополнительно сверены родителем. Подтверждённые закрытия: #321 и #143; остальные 45 оставлены открытыми. Пути в таблице относительно Sources/AddOn/, кроме явно указанных Docs/HTML.

### Финальная проверка

По разрешению владельца выполнен Tools/restart_archicad_for_test.ps1: canonical AC25 Windows Debug build успешен, Archicad PID17448 открыл Test_file/test_25.pln, SomeStuff.apx обнаружен среди его загруженных модулей по пути Build/SomeStuff/25/Debug/SomeStuff.apx. Runner завершился exit70, сообщив о провалах по файловому отчёту; отчёт продолжал обновляться после завершения скрипта. Свежая панель Visual Studio «Отладка» отсутствует. Поэтому успешный текущий C++ прогон НЕ подтверждён; приведённые runner счётчики не принимаются за окончательные результаты текущего запуска. JSON-тесты пропущены: Tools/test_json_commands.py отсутствует. Эта проверка не подменяет адресные native-сценарии.

Обе сохранённые PLN-копии защищены и проверены по SHA256: Test_file/test_25.pln = b92a5a0c0cbe70589493337ded4a85ff1cee82019b7fd018290fbac1f9fdf369; Build/SomeStuff/25/test_25.pln = 1a1b53b6fa05cd7ff12b7e26e8c8241a717fd4e0bb514da925570ebd1af074f7. Build-копия восстановлена после штатного перезаписывания сборщиком. Save, commit, push, новые ветки и исправления C++ не выполнялись.

### Индивидуальные вердикты

| Issue | Итог | Основание и остаток |
|---|---|---|
| #321 | CLOSED / completed | e0dfc7d; RoombookDraw.cpp:2032–2107,2762–2778 и deferred cleanup. Исторические native-сценарии исходного отказа, продолжения остальных зон и сохранения старых Roof прочитаны из roof-nonblocking/validation/parent-acceptance.json. Финальный runner выполнен; read-back GitHub CLOSED. Другие SDK/macOS, инъекции Create/Change и успешный общий C++ прогон не подтверждены. Применимость многослойки владелец меняет в проекте самостоятельно. |
| #320 | OPEN — код готов, приёмка не завершена | d066500; RoombookData.cpp:5767–5801,5892,5928. Controlled RED/GREEN 232 проверки сохранены; нужен native меньше/равно/больше min_area, отверстия/уклон, предупреждения и повтор с удалением старых GUID. Общий прогон RoomBook эти границы не доказывает. |
| #319 | OPEN — код готов, приёмка не завершена | 99238e6/b588359; RoombookData.cpp:1478,1549,1596–1597,9227–9228. Controlled 62/0; нужна native-матрица обоих типов и повтор после очистки. |
| #318 | OPEN — код готов, приёмка не завершена | e84e042 + 4813d74; SpecPlanning.cpp:280–335, Spec.cpp:2140. Сохранены RED/GREEN и успешные сборки. Нужны реальные Integer/String/Real назначения, create/update и повтор Spec. Утверждение body «не закоммичено» устарело. |
| #317 | OPEN — production не исправлен | Wall_Draw в RoombookDraw.cpp:1299–1304,1433–1460 удаляет проёмы/родителя без требуемого различения; caller:2739–2741 продолжает выполнение. rollback_required отсутствует. Scratch RED/GREEN-кандидат не применён и не доказывает native-откат. |
| #316 | OPEN — код готов, приёмка не завершена | a28fe78; RoombookDraw.cpp:542–549,1016,1042–1044. Нужны реальный Wall→Object, повторное сохранение Object GUID и native read-back. |
| #315 | OPEN — частичная приёмка/семантика аргумента | 78635c8/873c58f; CommonFunction.cpp:99–121, Propertycache.hpp:400–414, Helpers.cpp:7384+, Sync.cpp:1962–1991. Кэш по отображаемому ui_index. Нужен настоящий Sync для обоих правил и граничных индексов; catch/stoi могут выбрать 0 либо принять числовой префикс некорректного аргумента, политика требует решения/проверки. |
| #314 | OPEN — часть native-сценариев остаётся | a28fe78; RoombookDraw.cpp:2820–2875,2961+, RoomBookCommand.cpp:115–141. Native #321 подтверждает existing/no-op и ошибочные null-GUID записи, но не новое создание, отключённые writers, пустую комнату и rollback без публикации. |
| #311 | OPEN — не реализовано | Dimensions.cpp:156–199: обычный dimrule.pen_rounded, отдельного механизма подсветки свободной точки не найдено. |
| #308 | OPEN — алгоритм готов, границы поставки требуют сверки | Roombook.hpp:387, RoombookData.cpp:9220–9274 — reset, обе роли, индексы, 3D length и finite guards. Но RoombookQuantities.cpp:310–313 уже включает значение в ведомость, тогда как body ограничивает работу полем. До закрытия сверить дальнейшее решение владельца и адресные значения/повтор; автоматически откатывать интеграцию нельзя. |
| #298 | OPEN — живой дефект | CommonFunction.cpp:2406–2408 по-прежнему копирует дуги в сторону временного polygon2DData, не memo.parcs. Не путать с корректным обратным converter. |
| #295 | OPEN — частичная приёмка | RoombookDraw.cpp:745–750,878–879: компенсация Beam по anchorPoint есть; нужна полная согласованная native-матрица Wall/Beam, create/update/repeat. |
| #292 | OPEN — живой дефект | tests/TestKit.hpp:193–195 и206–207: условные bool/pointer формы передают постоянные actual false/null. Нужны адресные проверки вывода. |
| #284 | OPEN — есть конкретный пробел | RoombookData.cpp:5625–5713 реализует основную политику граней, но RoombookDraw.cpp:357–372 для горизонтального slab Wall_Main не применяет create_wall_elements (default=true). Gate есть у wall, но это не закрывает slab. |
| #260 | OPEN — umbrella частично выполнен | 59b4f95/73e1963 и поздние инкременты; выделенные стадии и writers есть, но полная поставка/документы/native-эквивалентность umbrella не приняты. Не распространять выполненность отдельных шагов на весь issue. |
| #258 | OPEN — исправление не подтверждено | Constants.hpp:151–153 сохраняет ID90/91/92, Spec.cpp:567–569 их использует. Известный сдвиг позиционных STR# не закрывается изменениями ReNum/#256; адресная приёмка подписей отсутствует. |
| #217 | OPEN — реализация удалена | В CommonFunction.cpp:1352–1368,1424–1426 нет требуемых счётчиков; FormulaCacheStats/ReportFormulaCacheStats не определены. Исторический ec299fd не является текущей поставкой; поздние удаления 73e1963/1fd5466. Для закрытия как not planned нужно решение владельца. |
| #198 | OPEN — измерения не выполнены | Рефакторинг 59b4f95/73e1963 не заменяет baseline/hotspots и сопоставление времени/результата на одном PLN. |
| #189 | OPEN — режим выбора не реализован | Удаление inline reset в2213388 выполнено, но отдельный этап чекбоксов групп/свойств и красной кнопки остаётся. В HTML resetPropertyToDefault:673–674 по-прежнему mock. |
| #187 | OPEN — не реализовано | HTML Interface_ru.html:2370–2454: редактор стартует с DEFAULT_SYNC_SCRIPT; выбора редактируемого свойства и автоформата нет, выполнение помечено TODO. C++ parse-мост не равен полному UI-сценарию. |
| #186 | OPEN — горизонтальный режим не найден | В HTML нет переключателя ориентации/отношения width-height; BrowserPalette.cpp:305–310,1721–1735 управляет размерами, но не требуемой перекомпоновкой. Native UI не проверен. |
| #177 | OPEN — удалено, не completed | ef35c87 удалил TableRenderer и его подключения. Удаление не доказывает выполненную поставку, оснований закрыть completed нет. |
| #176 | OPEN — удалено, не completed | ef35c87 удалил TablesNavigator и подключение. Для закрытия как снятого направления нужно отдельное решение владельца. |
| #171 | OPEN — runtime-приёмка отсутствует | Нужна сверка исходных296 срабатываний CompareParamValue с baseline на одинаковой сцене; наличие072f975/11add7a недостаточно. |
| #168 | OPEN — код есть, runtime-проверка остаётся | SomeStuff_Main.cpp:408–419 и489–491 переключают флаги и обновляют меню без PROPERTYCACHE().Update; issue требует наблюдения подписей/флагов/последующей работы. |
| #166 | OPEN — сравнительные PERF-замеры отсутствуют | Наличие оптимизаций не подтверждает предусмотренное сравнение четырёх направлений и допустимый регресс. CommonFunction.cpp:1352–1359 содержит TODO оценки EvalExpression. |
| #164 | OPEN — сравнение с прежней сценой отсутствует | Текущий Roombook существенно изменён; требуется именно сопоставление с прежним PLN/результатом, не просто успешный запуск. |
| #163 | OPEN — observer-приёмка отсутствует | SomeStuff_Main.cpp:234 → Sync.cpp:791 сохраняет SetAutoclass.072f975 убрал вложенную undo-область, но отсутствие runtime-ошибок этим не доказано. |
| #159 | OPEN — требуемое состояние не передаётся | BrowserPalette.cpp:563–566 и HTML:2185–2188 используют hasRule/наличие правила, не flagfindspec=false и активность флага на элементе. Подсветка наличия правила — другая функция. |
| #156 | OPEN — интеграция не обнаружена | Нет production-запроса/классификатора LightRAG, batch-сценария подтверждения и записи результата. ZIP и галочки в issue не доказывают интеграцию. |
| #155 | OPEN — backend частично, пользовательский путь отсутствует | BrowserPalette.cpp:726–805 содержит reset-мост, но HTML:673–674 возвращает mock success; режим выбора остаётся #189. Sync_reset/ALL также не доказаны. |
| #149 | OPEN — разделение теряется в конвейере | RoombookConstants.hpp:62–63 содержит два маркера, но RoombookData.cpp:8931–8938 сводит их в Reveal_Main;8825–8831 выбирает общий класс,8160 задаёт API_WindowID. |
| #143 | CLOSED / completed | Helpers.hpp:764, Helpers.cpp:9880–9904,10100–10130 читают BuildingMaterial и его свойства по API_AttributeIndex; production-callsite:7025–7048. Основная реализация a6feb208, поздние правки сохранены. Финальные build/load AC25 выполнены; native-чтение по индексу отдельно не исполнялось. GitHub CLOSED прочитан обратно. |
| #142 | OPEN — нужный вход по имени не доказан | Helpers.cpp:6198–6257 читает назначенный элементу composite/material по индексу.2da2264 о составе/материале не доказывает произвольное чтение многослойки по имени. |
| #141 | OPEN — соответствующий алгоритм не изменён | RoombookDraw.cpp:282–300 использует thickness родительской стены, не требуемую толщину из откоса. Удаление TODO при переносе не является реализацией. |
| #136 | OPEN — не реализовано | Не найден путь проверки неверных координат slab с окружностями на его слое пером60. Другие геометрические проверки не эквивалентны требованию. |
| #130 | OPEN — не реализовано | Sync.cpp:2549 сохраняет TODO фильтра;2550–2568 проверяет GUID без требуемого типа/слоя/свойства. |
| #129 | OPEN — не реализовано в нужной ветви | Propertycache.cpp:769–776, AC28+: точный TODO GeneralElemProperties; чтение по имени в старых версиях не заменяет GUID этой ветви. |
| #128 | OPEN — не реализовано | ClassificationFunction.cpp:122–132,187–200: ключ по lower(item.id), нет алиасов item.name и ID+имя. |
| #115 | OPEN — исправление не доказано | Helpers.cpp:2440–2454,2621–2687: мировые и локальные координаты публикуются разными ключами; нужен проблемный ключ и runtime-проверка окон/дверей, не автоматическое закрытие по смежной функции. |
| #103 | OPEN — дублируемый алгоритм остался | spec/Spec.cpp:90–112,1233–1256 и Helpers.cpp:4498,4563 сохраняют отдельные проверки; удаление TODO в00f8e019 не доказывает выделение общего helper. |
| #93 | OPEN — не реализовано | Sync_group и путь синхронизации свойств участников группы не найдены; API_Grouping для создания групп — другая функция. |
| #72 | OPEN — дефект не принят как исправленный | Dimensions.cpp:166 сохраняет TODO. Исключение размеров из observer в9d68f16 относится к Undo.dll и не доказывает исправление привязки к колонне. |
| #69 | OPEN — не реализовано | SomeStuff_Main.cpp:182–234 фильтрует notifID/throttle и вызывает общий SyncElement, не выбирает правила по API_ActTranPars/acttype. |
| #62 | OPEN — исходный участок не установлен | Локально не разрешился объект60e521acd9ce62dc9409ce50293e8be70b848725 из body. Удаление TODO не доказывает нужную декомпозицию; нужна идентификация исходной функции. |
| #48 | OPEN — не реализовано | third_party/alphanum.h:52–53 сохраняет TODO decimal/hex traits. Исторический авто-комментарий при удалении TODO не доказывает реализацию. |
| #2 | OPEN — реализация не найдена | Нет команды обхода страниц PDF и создания новых макетов под каждую страницу в разрешённом кодовом дереве. |

### Очистка IDEA

В прежней IDEA было39 разных issue-ссылок;25 из них уже CLOSED на GitHub. Устаревшие активные блоки, дубли и прежний внутренний Archive перенесены в IDEA_ARCHIVE.md → «Снимок IDEA до аудита 2026-11-08». Перенос сохраняет историю, не переписывая прежние результаты и не объявляя незавершённое завершённым. Раздел «Грабли» сохранён в IDEA. Байтовый контроль доказал сохранение всех прежних байтов архива; проверка мультимножества строк доказала разбиение полного исходного содержания на архивируемую историю и сохраняемые «Грабли». Git diff --check выполнен отдельно.

### Источники исполнения

- before-manifest.json, idea-issue-states.json, archive-proof.json.
- final-runner.log, final-runner-result.json.
- Полные независимые read-only трассы: cache/delegation/live/deleg_bfc1f572/task-0.log, task-1.log, task-2.log.
- Исторические адресные артефакты, указанные в прежней IDEA, прочитаны при аудите; они не выдаются за новые тесты.

Главная граница: audit завершён, реализация оставшихся45 задач не выполнялась. Повторный запуск Archicad и исправление найденных дефектов не входят в эту поставку.

---
## Снимок IDEA до аудита 2026-11-08

Status: HISTORICAL_SNAPSHOT. Это прежнее состояние, а не актуальный план. Закрытые и частично выполненные потоки намеренно сохранены без переписывания истории; актуальные статусы — в IDEA.md и GitHub. Перенос не означает завершения оставшихся задач.

### Current Task Roombook: исправление плинтусов и surface skips (#270/#252)

#### #320 — минимальная площадь всей площадной отделки

Scope: AC25 Windows; использовать существующий min_area для окончательных стен/колонн/откосов/полов/потолков и малых surface-диагностик. Площадь строго меньше порога игнорируется, равная сохраняется; Mesh — площадь поверхности. Линейные плинтусы без самостоятельного площадного фильтра. Константу владельца, исходное покрытие/provenance и чужие правки сохранить; без commit/push/Save/restart и остановки активной VS Run.
Status: WAITING_FOR_TEST.
Plan: [x] 1. Scope, dirty-baseline и issue/read-back; [x] 2. Проверить безопасную финальную точку фильтра и RED-регрессию; [x] 3. Минимальная правка; [x] 4. Format/fresh C++ проверка/регрессии/review; [x] 5. Адресные документы и итоговые границы native; [ ] 6. После разрешённого завершения активной отладки — canonical build и native-приёмка.
Last Completed: финальная фильтрация после преобразований/provenance до индексов плинтусов, включая floor-only; малые contacts без Ambiguous/adapter warning, старые slab GUID возвращаются в штатное удаление. Константа владельца и #319 WIP сохранены. Data SHA256 1d64becff1ee5a8a5a4322484532f1197616fc7fe68a3579dfa9659eac9e97fc. Одинаковые standalone C++14 assertions RED232/57FAIL → GREEN232/0, повторены parent; controlled SDK/geometry seams, не native. Полный TU SDK25/MSVC /std:c++14 /c /WX exit0 в scratch obj. Fresh clangd: новых диагностик нет, восемь прежних (три unused-variable при /WX, пять unused-include). Независимое ревью без замечаний в дельте. Format/diff-check, карточка и семь generated-записей проверены; прочие generated/callgraph исторические. Evidence: Hermes scratch/min-area-20261106/regression/, validation/. Ошибка zero-reveal fixture (width=0 выбирал обычную стену) исправлена только в harness, оба снимка повторены с одинаковыми assertions.
Next Step: canonical addon build/link и native-проверка после согласованного завершения активной VS Run; сейчас не выполнялись. Проверить отсутствие мелких результатов, старые GUID и повторный прогон. Предупреждение исходной зоны может остаться: четыре стеновых неоднозначных контакта ~0.18183 м² выше min_area=0.01 и этой политикой не исключаются. Issue320 OPEN; Save/restart/Stop/commit/push не выполнялись.
Last Checkpoint: d066500 — снимок правок #320 (код Roombook, константа, docs/generated-slice); native-приёмка отложена.
Issue: https://github.com/kuvbur/AddOn_SomeStuff/issues/320.

#### #318 — применение шага округления к итоговым суммам

Scope: AC25 Windows/C++14; R318-1/R318-2/R318-3 по команде владельца 2026-11-07. Только RoundFinalSums, его контракт и TestSpecRounding, адресные docs/generated. Helpers.cpp, общий is_equal, агрегация и существующая Integer/fractional-step политика read-only. Без веток/push/restart/Save/остановки Archicad.
Status: WAITING_FOR_TEST — три замечания исправлены и офлайн-проверены; новая сборка в Archicad не загружалась.
Plan: [x] 1. Исходная реализация e84e042 и review/baseline; [x] 2. Постоянный RED на неизменённом production; [x] 3. Минимальное исправление трёх замечаний и GREEN; [x] 4. Format/fresh LSP/полные TU/canonical AC25 build; [x] 5. Независимое ревью дельты; [x] 6. Адресные docs/generated/issue/память; [ ] 7. Согласованная native-приёмка записи и повторного запуска.
Last Completed (2026-11-07): implementation error — широкий is_equal мог выбрать нижнее кратное, int/text оставались старыми, raw-ноль обходил согласование. SpecPlanning.cpp:280–335: ближайшее целое частное принимается только при равенстве/одном соседнем double через round/nextafter; positive quotient underflow не даёт ноль; double/raw/int/text обновляются из результата с ToString по формату суммы, включая raw-ноль. Политика Integer3/шаг0.7→3 и общий is_equal не менялись. TestSpecRounding136: знаки/масштабы/точные кратные/underflow, Integer/String payload при Int/Real источниках для create/update, второй no-op, единицы формата и реальная raw-ноль компенсация после форматирования вкладов.
Validation: одинаковый SDK25/MSVC/C++14 stand RED649/24FAIL → GREEN649/0, соседние513/0; parser815/0. Пропущены прежний native-localized Bool case и4 AddRule/native assertions. Три полных TU SpecPlanning/Spec/TestSpec скомпилированы в scratch obj; финальный TestSpec повторён. Fresh version1 LSP четырёх файлов:0ошибок,5прежних unused-includes;169range/name audit принят с явной классификацией3anonymous namespace и7header-declarations, где semicolon вне range (первоначальный строгий assert был ошибкой checker, не source).
Compiled: Tools/BuildAddOn.py -c config.json -v25 -r -l INT — AC25 Windows RelWithDebInfo INT, exit0 после финальной тестовой правки. Первый build перелинковал APX с LNK4099; финальный повтор с TestSpec успешен. Загруженный Debug APX и оба test_25.pln по SHA256 неизменны; host/модель не тронуты. Native, другие SDK/macOS not verified.
Review: deleg_f11bee23 завершён — No issues found in inspected scope; guard, underflow, representations, raw-ноль и downstream writers прочитаны независимо. Production с момента ревью не менялся; последние3табличных случая точных кратных/большого целого добавлены после ревью и проверены основным агентом через финальные RED/GREEN/TU/build.
Documentation: карточка SpecPlanning — актуальный контракт/таблица9определений; symbols.json85 scoped records из fresh isolated clangd,983внешних записей/порядок сохранены (1066→1068, добавлены недостающие Real/TestSpecRounding). Shared MCP stale (RoundFinalSums отсутствует); callgraph не пересобирался, актуальность не заявляется. Evidence: Hermes scratch/spec318-fix/ (baseline-manifest.json, regression/{red-final,green-final}, validation/, parser/green/, build-ac25-final.log, symbols-acceptance.json). Процедура: archicad-cpp-tdd-patterns/references/numeric-step-rounding-review.md.
Next Step: только по отдельному согласованию загрузить новую сборку AC25 и проверить фактическую запись Integer/String/Real назначений и повторный запуск. #318 OPEN; body и итоговый comment6704764673 прочитаны обратно. Push не выполнялся.
Last Checkpoint: снимок исправлений #318 — текущий коммит; native-приёмка отложена.
Issue: https://github.com/kuvbur/AddOn_SomeStuff/issues/318 (OPEN); исходные замечания comment6704340591, итог исправления comment6704764673.

#### #319 — пустой тип отключает соответствующий плинтус

Scope: FloorSkirtingSegments_Build, SkirtingSegments_Build и подготовка faces в OtdBeam_ProcessFinishes; AC25 Windows. Сохранить пользовательскую обработку типов; без изменения геометрии, SDK-вызовов, commit/push/restart и остановки активной VS Run.
Status: WAITING_FOR_NATIVE.
Plan: [x] 1. Исходники, dirty-baseline и issue с read-back; [x] 2. Независимые guards по типам и корректный потолочный beam.tip; [x] 3. Controlled-регрессия, format/full-TU syntax check и адресная карточка; [ ] 4. Canonical build/native и fresh generated при согласованной приёмке.
Last Completed: причина implementation error — builders не проверяли пустой тип; добавлены guards в обоих builders и до подготовки faces, потолочный beam.tip теперь из tip_ceil_cornice. Пользовательское чтение параметров сохранено; tests fixtures получили явные типы и новые проверки. clang-format/diff-check PASS. Controlled C++14 regression дословных блоков с подменными типами: baseline62/14FAIL → live62/0; parent повторил оба exe и сверил live SHA256 6e0d7f342cb106bfe0c55ca68c9edf9be89a194365391a993af3854bc08d7155. Это не полные функции/native geometry; repository suite не запускался. Full-TU SDK25 clang-cl /Zs: strict baseline/live имеют те же три unused-variable errors; с единственным Wno-error=unused-variable live exit0. Object compilation/relink не выполнялись. Read-only review пяти production-hunks: дефектов не найдено. Evidence: Hermes scratch/skirting-empty-types/validation. MCP stale; generated не обновлён, актуальность не заявляется.
Next Step: по согласованию безопасно собрать/загрузить AC25 и проверить оба вида и повтор с очисткой типов; выполнить repository geometry suite и обновить generated свежим LSP. Активная VS Run не остановлена, модель не изменялась. Issue319 открыт; коммит снимка выполнен (99238e6), push/restart не выполнялись.
Last Checkpoint: снимок правок #319 коммитом 99238e6; native-приёмка отложена.
Issue: https://github.com/kuvbur/AddOn_SomeStuff/issues/319.

#### JSON-команды — страница вики

Scope: только wiki/ru/JSON-Commands-ru.md (новая), wiki/ru/_Sidebar.md и Sources/AddOn/json_commands/How JSON Commands work.md (приведён к 9 командам); код, Docs и другие задачи не затрагиваются. Статус: страница и внутренний док созданы/обновлены по текущим исходникам json_commands, UTF-8/CRLF, diff --check чист. Коммит и push не выполнялись.

#### #317 — R2: откат разрушительного обновления стены и проёмов

Scope: R2 из Reviews/roombook.md, Wall_Draw и передача ошибки через OtdWall/OtdBeam/OtdRoom, внутреннее состояние OtdDrawData; AC25 Windows. Сохранить R1/#316, #314 и чужие правки. Геометрия, общий deletion/matcher, K1 и native-проект вне scope. Без commit/push/restart/Save/веток. Issue: https://github.com/kuvbur/AddOn_SomeStuff/issues/317.
Status: IN_PROGRESS.
Plan: [x] 1. Dirty-baseline, issue/read-back, исходники и SDK25 undo-signature; [/] 2. Независимая оценка риска и standalone RED/GREEN кандидата; [ ] 3. Минимальная правка writer/callers; [ ] 4. Format/fresh LSP/canonical AC25 build/review; [ ] 5. Итоговое состояние и границы native-приёмки.
Last Completed: baseline Draw13b75f60…/hpp39e332ab… сохранён в Hermes scratch/roombook-r2-20261106. Причина по исходникам: implementation error — после Delete старых проёмов ошибки могут быть проглочены; failure нового проёма удаляет также reused parent. Candidate только в scratch: явный rollback_required до попытки существующего Delete/Change/opening, без удаления reused parent; отказ cleanup новой стены тоже fatal. Обычные ошибки до мутации сохраняют continue. Полный runtime-откат в Archicad не проверен.
Next Step: подтвердить RED и unchanged GREEN на candidate, принять design review, применить только адресную дельту и проверить AC25. Источники SDK: ACAPinc.h:345, установленная AC25 HTML CallUndoableCommand:99–115; LightRAG вернул примеры, но явный текст о native-rollback не найден.
Last Checkpoint: нет; commit не запрошен.

#### #316 — R1: тип surface matcher и Object-writer

Scope: только R1 из Reviews/roombook.md; RoombookDraw.cpp, адресные regression/docs. AC25 Windows; другие версии/macOS not verified. Сохранить чужую дельту #314; R2/дуги/геометрию/политику удаления не менять. Без commit/push/restart/Save и новых веток. Issue: https://github.com/kuvbur/AddOn_SomeStuff/issues/316.
Status: WAITING_FOR_NATIVE — код R1 исправлен и офлайн-проверки завершены; новая сборка в Archicad не загружалась.
Plan: [x] 1. Baseline/контракты/issue с read-back; [x] 2. Matcher по favorite.type и отказ несовместимого Object GUID; [x] 3. RED/GREEN, format, fresh LSP, canonical AC25 build и independent review; [x] 4. Адресные docs/generated и итоговое состояние; [ ] 5. Согласованная native-приёмка перехода Wall→Object и повтора.
Last Completed: baseline Draw b5726d5d… сохранён в Hermes scratch/roombook-r1-20261106; applied13b75f60… отличается ровно условием matcher и двумя присваиваниями в Object_Draw_Linear, чужая #314-дельта сохранена. Причина implementation error; новых ACAPI нет, LightRAG вернул примеры Get/type. clang-format и scoped diff-check PASS. Изолированная fresh LSP version1 baseline/live: те же шесть диагностик,48selection/name joins подтверждены; stale MCP отвергнут. Адресная карточка и две symbols-записи обновлены, прочий callgraph не объявляется актуальным. VS Design, Archicad/PLN не тронуты.
Validation: canonical BuildAddOn.py -c config.json -v25 -r -l INT exit0; Draw перекомпилирован, APX перелинкован (d2b61d21…). Родитель сверил live Draw с applied13b75f60…; прежний Debug APX и оба test_25.pln SHA256 неизменны. Лог build-ac25.log и build-acceptance.json. Native не выполнялся.
Review: No issues found in inspected scope; родитель прочитал report.md, сверил семь hashes manifest с артефактами/live. Native boundary untested; R2/K1 вне scope.
Regression: parent прочитал build_test.py/results/manifest и повторил оба run.cmd: baseline75checks/19FAIL → candidate75/0, все26hashes manifest подтверждены, candidate byte-identical live Draw. Исполнены реальные matcher/TakeExistingFinish/ShouldDraw и точный Object-префикс до Point2D wbegC; настоящие SDK25-типы/GS-контейнеры, облегчённые domain fixtures и controlled API/readers. Полный writer Create/Change/no-op этим стендом не выполнялся. Новых регрессий в проверенной области нет. Memory procedure: archicad-cpp-tdd-patterns/references/roombook-native-type-matcher.md; добавление ссылки в oversized SKILL.md отклонено атомарно, сохранён только reference.
Next Step: по отдельному разрешению загрузить новую сборку и проверить Wall→Object, повторное сохранение Object GUID и native-геометрию/read-back. R2/K1 остаются отдельными задачами. Issue316 открыт; commit/push/restart/Save не выполнялись. Существующие shared HEAD/index изменения других сессий не принадлежат R1.
Last Checkpoint: нет; commit не запрошен.

#### #315 — индексный кэш этажей

##### Текущий снимок для коммита [story-315]

Scope: только код команд Story и кэша, русская вики, соответствующие карточки/срез symbols; AC25 Windows. Без рефакторинга SyncString, RoomBook #314, удаления FormulaCacheStats, push, restart и закрытия issue.
Status: WAITING_FOR_NATIVE — коммит разрешён владельцем, изолированная проверка снимка завершена.
Plan: [x] 1. Изолировать оставшуюся дельту #315 поверх HEAD873c58f без замены рабочих исходников; [x] 2. Повторно проверить четыре полных TU SDK25/MSVC C++14 /WX и controlled-регрессию; [x] 3. Подготовить адресные docs/generated и точечный коммит; [ ] 4. Canonical build и native-приёмка по отдельному согласованию.
Validation: после параллельного коммита873c58f четыре полных TU CommonFunction/Helpers/Propertycache/Sync повторно скомпилированы в scratch .obj, все exit0; addon целиком не собирался/не линковался. Controlled SDK-регрессия:64checks/0failed, оба skipNullFloor, нижний/верхний и виртуальный уровни, UI-ключи, lazy/miss/API failure; parent повторил запуск exit0. Parent проверил SHA восьми исходников;236cpp-записей CommonFunction/Helpers/Propertycache сверены со свежими clangd symbols, остальные947записей/порядок сохранены из873c58f. Полный parent-аудит header selectionRange остановлен после двух ошибок проверяющего скрипта на destructor/operator=; опубликованные cpp-записи проверены отдельно.
Evidence: C:/Users/da-rogojin/AppData/Local/hermes/cache/scratch/story-315-only-commit/validation/ (final-results.json, snapshot-final, *-final.cmd/log/obj, fresh-lsp-no-pch, regression-ui-index). Первый снимок24dc70b сохранён отдельно; после движения HEAD защита остановила staging без изменения index, финальный снимок пересобран поверх873c58f. Sync.cpp и его карточка с Story уже вошли в параллельный коммит, повторно не включаются. LightRAG timeout28; fallback SDK25 API_StoryInfo.html:70–93/headers.
Next Step: отдельная canonical сборка/загрузка и наблюдение обеих команд в согласованном проекте; issue #315 остаётся открытым. EN и расширение нечислового index вне приёмки.
Last Checkpoint: [story-315] Add indexed story level and height synchronization; Refs: IDEA.md step 3; Refs: #315. Только файлы/фрагменты #315; push не запрошен.

##### Предыдущий этап кэша — исторические проверки до подключения команд

Scope: кэш уровня и высоты по short-индексу в PropertyCache; минимальное дополнение Story/GetStories для сохранения высоты, включая верхний этаж через виртуальный уровень SDK. Проверка AC25 Windows; другие версии/macOS not verified. Пользовательские изменения разбора Story/ReadStory, Helpers.cpp и Sync.cpp не менять. Без commit/push/restart/Save.
Status: PREPARING_COMMIT — пользователь завершил подключение Story и разрешил коммит только #315 с документацией. Прежний отказ сборки ниже — исторический результат, не утверждение о текущем Sync.cpp.
Plan: [x] 1. Прочитать пользовательскую дельту, GetStories и lifecycle PropertyCache; проверить высоту в установленном SDK25; [x] 2. Добавить высоту в данные GetStories и индексный словарь с ленивым чтением/обновлением; [x] 3. Проверить регрессии/format/LSP и дополнить карточки (generated отложен явно); [ ] 4. Успешная полная сборка после завершения пользовательского Sync.cpp.
Last Completed: Story.height и GetStories используют соседний/виртуальный уровень (SDK25 API_StoryInfo.html, Environment_Control.c:963–968); storyCache<short,Story>, ReadStories/GetStory и обновление в Update добавлены. clang-format/diff-check PASS. Fresh LSP publishDiagnostics version1 получен для трёх файлов: Propertycache.hpp0; CommonFunction.cpp7/CommonFunction.hpp5 diagnostic вне изменённых строк. Parent прочитал summary/raw classification и подтвердил byte-identical source snapshots. Полный повторный parent-аудит symbols остановлен после ошибок проверяющего скрипта (JSON wrapper/destructor selectionRange, затем неверный ожидаемый счётчик); весь symbols-аудит parent not verified. Карточки CommonFunction/Propertycache дополнены адресно; generated не изменён: старый срез неоднозначно представляет перегрузки, полноценное обновление отложено.
Validation: BuildAddOn.py -c config.json -v25 -r реально выполнен, exit1: все57 строк compiler errors относятся к Sync.cpp:1562+ (synctypefind/paramNamePrefix/param, затем синтаксические ошибки). Sync.cpp изменился относительно baseline без моих записей; не исправлялся/не откатывался. Compiled addon: no. Debug APX и оба test_25.pln по SHA256 сохранены. Evidence: C:/Users/da-rogojin/AppData/Local/hermes/cache/scratch/story-cache-315/.
Regression: parent прочитал independent/report.md, manifest и оба TU-лога, сверил все source/artifact SHA256, повторно выполнил run.cmd: checks75/failed0, api_calls8/disposed_handles6. Настоящие SDK25 GS::Array/HashTable; controlled host API, constructor/Update projections и sparse-reader явно обозначены. Полный CommonFunction.cpp и include-Propertycache.hpp TU скомпилированы MSVC C++14 /WX в scratch obj exit0; это не linked addon/native. Первое parent-прямое выполнение exe без Archicad DLL PATH завершилось127; штатный run.cmd с PATH реально прошёл. Независимое review: No issues found in inspected scope.
Next Step: проверить изолированный снимок HEAD + только Story в scratch/story-315-only-commit, обновить адресные карточки/generated и создать точечный коммит. Декомпозиция SyncString, RoomBook #314 и удаление FormulaCacheStats исключены; общий рабочий исходник не подменять. Canonical build/native отдельно, Archicad не запускать.
Last Checkpoint: подготовка [story-315]; commit только #315 разрешён владельцем, push не разрешён. Issue: https://github.com/kuvbur/AddOn_SomeStuff/issues/315.

#### #314 — вся подготовленная отделка в JSON RoomBook

Scope: AC25 Windows; RoomBookFinishResult, AppendRoomFinish/CollectRoomFinishes и сериализация RoomBook. Выводить весь подготовленный набор обработанных валидных зон, включая no-op/обновлённые и отключённые writers; не менять геометрию, запись элементов/свойств, отбор зон или прежнюю защиту rollback. Полный native-инвентарь вне массивов, раскрытие состава и обход ранних отказов вне scope. Без commit/push/restart/Save. Ветка roombook-json-all-results от master. Issue: https://github.com/kuvbur/AddOn_SomeStuff/issues/314.
Status: WAITING_FOR_TEST — реализация и офлайн-проверки завершены; native-приёмка новой версии не выполнена.
Plan: [x] 1. Текущий контракт, docs/guards, чистый baseline и issue/read-back; [x] 2. RED и полный сбор/сериализация с явным sentinel и состояниями; [x] 3. Регрессии, format/fresh LSP/AC25 build и review; [x] 4. Адресная документация/generated и итоговые свидетельства; [ ] 5. Согласованная загрузка и native-приёмка.
Last Completed: фильтры newly-created/null GUID сняты только в сборщике/сериализаторе; явный is_element сохраняет sentinel, is_valid/drawing_enabled/draw_error дополняют прежние write_succeeded/was_created. NaN/Inf дают unavailable/0, а не невалидный JSON. Состав base_composite и свойства не изменялись. LightRAG timeout (curl28), fallback source/SDK25 headers; новых ACAPI нет.
Validation: parent повторил SDK25/MSVC C++14 exact extracted collector/serializer: baseline RED277checks/99FAIL → GREEN1287/0, executed source SHA совпадают с live. Геометрические fixtures controlled: wall length*height, пустой SDK slab; это не проверка net/mesh/native. Contract8/0; format/diff-check PASS. Fresh publishDiagnostics version1, source hashes и563symbol/name joins проверены parent: JSON0diagnostics, hpp1unused-include, Draw5unused-variable severity1+1unused-include вне дельты. MCP stale, его пустые diagnostics не приняты. Независимое review до конечного finite guard: No issues found in inspected scope; guard проверен parent и повторными tests/LSP/build. Canonical AC25 RelWithDebInfo BuildAddOn.py -c config.json -v25 -r успешна, финальный Draw перекомпилирован и APX перелинкован в25/INT; Debug APX и оба PLN по SHA256 сохранены. Открытый test_25 использует прежний Debug APX, host не остановлен/не сохранён.
Documentation: карточки Roombook/json_commands обновлены адресно; symbols8записей сверены с fresh LSP,1122остальные записи/порядок сохранены. Унаследованный callgraph и прочие stale-координаты не пересобирались и не объявляются актуальными. Evidence: C:/Users/da-rogojin/AppData/Local/hermes/cache/scratch/roombook-all-results-20261106/ (regression, validation, review, final-build.log, acceptance.json, SHA256-manifest.json). Процедура сохранена в archicad-json-commands/references/roombook-all-results.md.
Next Step: по отдельному разрешению безопасно загрузить новую сборку и проверить явный zones-список, повтор/no-op, отключённое построение, native read-back и сохранение изоляции. Затем отдельно продолжить JSON↔свойства: полный набор сам по себе не раскрывает base_composite/привязки result-свойств и не доказывает успешную запись свойств. Issue314 открыт; commit/push/restart/Save не выполнялись.
Last Checkpoint: нет; commit не запрошен.

#### CI macOS — диагностика 2026-11-06

Scope: Tools/CMakeCommon.cmake: исключение unnecessary-virtual-specifier для macOS AC25–30 с сохранением общего -Werror; локальные проверки и коммит разрешены. Без issue по прямому указанию владельца; без push/перезапуска CI, правок C++ и workflow.
Status: WAITING_FOR_CI — локальное исправление и проверки завершены; полная macOS-сборка требует отдельного CI-запуска.
Plan: [x] 1. Диагностика jobs/logs и настроек; [x] 2. Исключение предупреждения и локальная валидация; [ ] 3. Проверить AC25–30 macOS в Actions после разрешённого push.
Last Completed: run42105791876, HEAD5cfb333: все шесть build_mac падают на exprtk.h:7111/7113, -Werror,-Wunnecessary-virtual-specifier (Xcode26.6); Windows25–30 success. Tools/CMakeCommon.cmake:165 включает -Wall -Wextra -Werror. Предыдущий run42104672988 падал раньше, на Setup Xcode; 14c6de3 сменил macos-14 на macos-latest, 5cfb333 убрал выбор Xcode15.1. Исходник string_range_node final содержит два новых virtual ref().
Validation: Tools/CMakeCommon.cmake добавляет только -Wno-unnecessary-virtual-specifier в существующий non-Windows список; общий -Werror сохранён, exprtk/workflow не изменены. Локальный Clang22.1.8: минимальный final/ref reproducer RED exit1 (две точные диагностики) → GREEN exit0. Реальная SetCompilerOptions выполнена через cmake -P с перехватом target-команд: все шесть macOS25–30 наборов отличаются от HEAD ровно одним флагом, все шесть Windows наборов идентичны. Это проверка функции CMake/диагностики, не полная конфигурация/сборка аддона и не Apple Clang. git diff --check PASS. Артефакты: Hermes scratch/mac-ci-warning-fix/{verify.py,report.json,baseline.txt,candidate.txt}.
Next Step: после отдельного разрешения на push проверить полную матрицу AC25–30 macOS в Actions; compiled/tested addon not performed. Issue не создаётся по явному указанию владельца.
Last Checkpoint: локальный коммит [CI-mac] Suppress exprtk final-class virtual warning (IDEA.md step 2); push не разрешён.


#### #312 — тип слоя в шаблоне состава
Scope: подстановка %structype% в Sync_from{Material:Layers} с существующими RU/ENG ресурсами; сохранить незакоммиченные изменения пользователя. Проверка AC25 Windows; другие версии и macOS отдельно не проверяются. Владелец разрешил финальный запуск скрипта и закрытие issue; push не разрешён.
Status: ISSUE CLOSED по запросу владельца; native-приёмка не выполнена.
Plan: [x] изучить пользовательские правки и маршрут разбора/подстановки, сохранить baseline; [x] дополнить реализацию; [x] проверить формат, диагностики, AC25 сборку и изолированное выполнение блока; [ ] проверить полный шаблон в Archicad и обновить generated-документацию.
Last Completed: ReadMaterial заполняет локализованный structype по composite[indx] до подстановки; сохранены предварительные правки пользователя. AC25 Debug BuildAddOn.py exit 0, LNK4099; clang-format и diff --check успешны. Извлечённый блок с подменными типами проверен для трёх значений, обоих порядков и null; полный маршрут и native не выполнялись. LSP возвращает 19 диагностик неиспользуемых значений/переменных и две unused-include, в добавленном блоке нет. LightRAG запрос превысил таймаут; использован существующий код. Docs/modules/Helpers.md обновлён, generated не обновлялся. PLN после сборки по SHA совпадает с исходным.
Финальная проверка: restart_archicad_for_test.ps1 собрал AC25, но завершился exit 70; отчёт C++ от 17:42 старше APX 21:17, поэтому 203 ошибки не относятся достоверно к этому запуску. Процесс Archicad был обнаружен скриптом, затем не обнаружен; загрузка аддона не подтверждена. JSON-тесты пропущены: скрипт отсутствует; Debug output pane не найден. Issue #312 закрыт по запросу, с указанием непроверенной загрузки и шаблона.
Next Step: при отдельной проверке запустить полный Sync_from{Material:Layers; "%structype%"} в Archicad на разных типах и порядках слоёв и на обоих языках; обновить generated-документацию.
Last Checkpoint: коммит #312 (проверка Archicad остаётся открытой).

#### #310 — линия и текст объекта отделки по реальным точкам полигона

Scope: AC25 Windows; только RoombookDraw.cpp: новые static-хелперы (Line_Cross, Segments_ProperCross, Poly_PickLineEnds) и блок pos_x1/pos_y1/pos_x2/pos_y2/pos_xtxt/pos_ytxt в Object_Draw_Poly. GDL-избранное, поле fieldorig, geometрия и чужие дельты вне scope. Без commit/push/restart. Issue: https://github.com/kuvbur/AddOn_SomeStuff/issues/310.
Status: WAITING_FOR_BUILD — правка внесена при активной VS Run (Archicad под отладчиком), canonical build/native не выполнялись.
Plan: [x] issue #310 и baseline (SHA d9207722…, scratch/object-line-poly-20261105); [x] правка: Poly_PickLineEnds ищет самую нижнюю левую вершину внешнего контура (минимум x+y) и верхнюю правую по убыванию x+y, отрезок которой не пересекает граней контура и дырок (касание в общих вершинах не считается); [x] Ceil: pos2 = выбранная вершина, текст = середина отрезка; Floor: pos1 = нижняя левая; fallback на прежний расчёт по боксу, если пары нет; [x] clang-format (чужие регионы не задеты — diff только свои hunks), diff-check 0, LSP AC25 0 diagnostics; [x] standalone-проверка алгоритма (прямоугольник/вогнутый L/дырка/вырожденный); [ ] canonical AC25 build и native-приёмка после согласованного завершения отладки.
Next Step: собрать AC25 (BuildAddOn.py) и проверить в Archicad линию/текст на потолках и полах разной формы.
Last Checkpoint: нет; commit не запрошен.

#### #309 — список GUID зон в JSON RoomBook

Scope: AC25 Windows; входные параметры RoomBookCommand и перегрузка RoomBook_Run, адресные регрессии и документация. Геометрия/избранное/writers/PLN и чужие правки вне scope. Без commit/push/Save/restart; активную VS Run не останавливать. Issue: https://github.com/kuvbur/AddOn_SomeStuff/issues/309.
Status: WAITING_FOR_BUILD — код и офлайн-проверки завершены; canonical build/reload/native при активной VS Run не выполнялись.
Plan: [x] IDEA/module cards/dirty baseline/callers/SDK lookup и issue; [x] JSON zones с проверкой всего списка и рабочая копия зон; [x] адресные регрессии, format/LSP/full-TU/review; [x] адресная документация/generated/память; [ ] согласованные canonical build/reload и native-приёмка.
Last Completed: RoomBookCommand принимает zones:string[] и проверяет форму/нулевой GUID/GetHeader/Zone/прежние selection-фильтры до запуска; дубли исключены. RoomBook_Run принимает const-массив и копирует его перед прежними Clear/Push. Без zones передаётся пустой массив, прежний selection/fallback сохранён. Format/diff-check PASS; Python Draft4 schema/source invariants21/0, serializer/sibling commands и5защищённых файлов неизменны. SDK25 headers подтверждают ObjectState::Get/errorCode, GetHeader/Filter и IsASCIIHexaDigit; начальная неоднозначность UniChar>=char устранена SDK-предикатом. MCP diagnostics0, но Roombook documentSymbols stale (одна перегрузка/старый end), не принят для generated. Baseline/evidence: Hermes scratch/roombook-zones-20261105/. VS Run AC25, host не тронут.
Validation: full-TU MSVC/SDK25 C++14 /WX → scratch obj для обоих cpp exit0; родитель прочитал логи и сверил весь manifest/исходники. Fresh LSP version1: JSON0, Run2unused-includes, hpp1unused-include, ошибок нет; parent independently проверил517symbol ranges/text hashes. Независимое review: No issues found in inspected scope. Scoped generated15joins,1115внешних записей/порядок сохранены; старый callgraph не обновлён и явно помечен как неактуальный. Каноническая сборка/relink/native не выполнялись. Процедура сохранена в archicad-json-commands/references/optional-zone-guid-list.md.
Regression: parent реально повторил JSON SDK25/MSVC C++14 RED105checks/60FAIL → GREEN105/0 и input-protection полного production Run34/3 →34/0. Все139GREEN проверок проходят, Python schema/invariants21/0 отдельно. Read-only GetHeader/Filter корректного префикса допустимы до позднего ошибочного GUID: leaf ошибочно потребовал0reads; исправлен только scratch expectation в RED/GREEN, начальные106/1 evidence сохранены. SDK25 JSON conversion удаляет zones:null; Python schema его отклоняет, native enforcement not verified. Артефакты/body/source hashes приняты в regression-acceptance.json. Final format/diff-check PASS, чужие5файлов/serializer byte-identical. Issue309 статус обновлён и read-back6659803148 принят; issue открыт. Memory: процедура в skill и источник передан OpenViking extraction.
Next Step: с разрешения владельца безопасно собрать/загрузить аддон и проверить выбор одной/нескольких зон, отсутствие обновлений посторонних зон, повтор и native schema-отказ null/пустого списка. На старом loaded binary malformed request не пробовать: он игнорирует zones и может запустить весь legacy набор. Не Stop/Save/restart/commit/push/IssueClose без разрешения; другие версии/macOS не проверялись.
Last Checkpoint: нового нет; commit не запрошен.

#### #308 — длина плинтусов каждого OtdSlab

Scope: AC25 Windows; подсчёт готовых нижних/верхних отрезков в OtdSlab.length_skirting (метры), через OtdBeam.slabIndex и geom.Length(). Геометрия/дедуп/избранное/native-writers не меняются; свойства/GDL и room-level суммы не подключены. Без commit/push/restart/Save. Issue: https://github.com/kuvbur/AddOn_SomeStuff/issues/308.
Status: WAITING_FOR_BUILD — подсчёт реализован, offline-проверки завершены; canonical build/native не выполнялись при активной VS Run.
Plan: [x] живой producer/caller/индексы и baseline; [x] сброс до ранних отказов, сумма конечных 3D-отрезков; [x] RED/GREEN, format/diff-check, full-TU SDK25, fresh LSP и независимое review; [x] собственная дельта/issue read-back/память; [ ] согласованная canonical build/native-приёмка.
Last Completed: изменена только OtdBeam_ProcessFinishes (Data9118–9186): сброс всех slab.length_skirting; после успешного builder сумма конечных geom.Length по неотрицательному/in-range slabIndex, нечисловые/бесконечные длины пропускаются. Считается подготовленная геометрия, не результат native-записи. Exact-production function/geom structs с controlled builder/Favorite/lightweight boundary types: MSVC C++14 baseline18PASS/12FAIL → GREEN30/0, все18 прежних GREEN неизменны. Разные Floor/Ceil/split индексы, уклон/Mesh, повтор/исчезновение контактов, неверные индексы/nonfinite и ранние отказы проверены; настоящая SDK-геометрия в этом стенде не выполняется.
Validation: весь Data.cpp MSVC/SDK25 C++14→scratch obj exit0, НЕ linked addon. Свежий publishDiagnostics version1, source-text SHA и namespace end9682 подтверждены; в изменённой функции diagnostics0, вне неё3unused-variable severity1+5unused-includes на прежних baseline-строках. Независимое review: No issues found in inspected scope. Parent прочитал артефакты и принял source/header/LSP/obj hashes; Data SHA f977eac0c5f978424deadeb9d0ec633b99452c83b5f59efca803084458cd63fc. Format/diff-check PASS, Data вне функции нормализованно неизменен, остальные6 baseline файлов byte-identical. Evidence: Hermes scratch/skirting-length-count-308/{own.diff,red,green,parent-check.json,accepted.json,independent-validation/}. Issue308 обновлён и exact read-back принят. Процедура: archicad-cpp-tdd-patterns/references/per-slab-skirting-lengths.md.
Next Step: по разрешению выполнить canonical build и при необходимости проверить заполненные значения в Archicad; текущую VS/модель не останавливать. Docs/generated отложены до отдельной pre-checkpoint команды, актуальность не заявляется. Свойства/GDL не подключать без следующего этапа. Появившийся во время проверки untracked wiki/files/Отделка_25.pln не тронут, происхождение не проверялось.
Last Checkpoint: нового нет; commit не запрошен.

#### Beam_Draw — синхронизация anchorPoint существующей балки

Scope: AC25 Windows; только сравнение anchorPoint существующей балки с прочитанными настройками избранного в Beam_Draw, присваивание и Change mask до расчёта upper-reveal offset. Геометрические формулы, остальные writers, избранное и PLN не менять. Без issue по прямому указанию владельца; без commit/push/restart.
Status: WAITING_FOR_TEST — код реализован и standalone проверен, native-приёмка не выполнялась.
Plan: [x] текущий writer/default reader и SDK25 field/Change mask; [x] адресная RED/GREEN и минимальная правка; [x] format, свежая LSP, отдельная TU и собственное source-review; [ ] согласованная native-приёмка.
Last Completed: только пять строк в Beam_Draw: сравнение anchorPoint с settings, маска, присваивание и needupdate до upper-reveal offset. SDK25 APIdefs_Elements.h:1187 и ACAPI_Element_Change.html подтверждают поле short и mask-path, числовая интерпретация не менялась. Точное production-тело + реальные SDK25 types/helpers при контролируемых readers/Create/Change/Dispose: RED1145/631FAIL → GREEN1145/0; все пары0..8 обычных/upper, row-only/column changes, точная маска, повтор no-op, создание, ошибки и сохранение memo. MSVC C++14 SDK25 полная TU→scratch obj exit0; это НЕ canonical addon build. clang-format/dry-run/diff-check PASS; diff против dirty-baseline строго +5строк, executed-source hash совпал. MCP stale (Beam end928 вместо933), его0 не принят. Изолированный fresh LSP version1/namespace end2753: пять unused-variable и один unused-include вне правки; изменённый блок без diagnostics. Собственное review: No issues found in inspected scope. Evidence: Hermes scratch/beam-anchor-update-20261105/{before.cpp,own.diff,red,green,tu,fresh-lsp.json,lsp-messages.json}. Memory: lightrag/SKILL.md — anchor перед offset; archicad-cpp-tdd-patterns reference — не извлекать тело от forward declaration.
Next Step: при согласованной сборке/загрузке проверить изменение привязки существующей балки и повтор в Archicad. Полная сборка/relink/native not verified; host/PLN не тронуты. Docs/generated отложены до отдельной pre-checkpoint команды по действующему порядку, актуальность не заявляется.
Last Checkpoint: нового нет; commit не запрошен.

#### Финальное ревью перед релизом — 2026-11-05

Scope: read-only ревью текущего HEAD13dfb0c и dirty-дельты; Spec, Roombook, общие интеграционные изменения, ресурсы и упаковка. CI-матрица по текущему workflow: AC25–30 Windows/macOS; локальные офлайн-проверки без native host. Production-код, PLN, commit/push, публикация и закрытие issues вне scope.
Status: IN_PROGRESS.
Plan: [x] baseline SHA/dirty manifest и границы; [x] HTML/i18n/реестр/Spec offline; [x] чтение CI точного HEAD; [/] независимые ревью Spec/Roombook/упаковки/интеграции; [ ] проверка замечаний родителем и итоговый отчёт.
Last Completed: все семь офлайн-команд exit0 (HTML RU/EN; npm validate — 0 errors/8 warnings RU; i18n159 ключей; i18n tests46/0; rule picker PASS; реестр90/90/90; Python Spec comparator20/0). Git diff --check PASS. CI41074549200 success для13dfb0c: AC25–30 Win/macOS, но без dirty-дельты. VS Design. Новая C++ сборка/runtime не выполнялись; BuildAddOn Debug копирует тестовый PLN, ограничения прошлых задач сохранены.
Next Step: завершить параллельные ревью и независимо подтвердить замечания; до этого финальный релизный вердикт не вынесен. Артефакты: C:/Users/da-rogojin/AppData/Local/hermes/cache/scratch/release-review-20261105/.
Last Checkpoint: нового нет; commit/push не запрошены.

#### #307 — Spec: исправления по spec_review.md

Scope: AC25 Windows; Sources/AddOn/spec/Spec.cpp, SpecPlanning.cpp/.hpp, при необходимости SpecHelpers.cpp/.hpp; tests/TestSpec.cpp и адресная документация. Без изменения группировки, политики удаления при неполных данных (F1), fallback нечитаемого флага и общего Helpers.cpp. Владелец выбрал работу непосредственно в roombook_refactor вместо отдельной ветки. Без commit/push, Save/Restart/IssueClose.
Status: WAITING_FOR_TEST — исправления в Spec.cpp, SpecPlanning.cpp/.hpp и TestSpec.cpp; canonical AC25 Debug build exit0, SDK25 standalone GREEN164/0 и расширенные suite513/0, свежая LSP/review приняты. Отмена в полном SpecArray и native-приёмка не выполнены. Baseline HEAD13dfb0c и protected PLN — scratch/spec-review-fixes-20261104/{before,before-manifest.json,protected}; финальные hashes — applied-manifest.json. Чужие Roombook/IDEA-правки сохранены. План: .hermes/plans/2026-11-04_100446-spec-review-remediation.md. Issue: https://github.com/kuvbur/AddOn_SomeStuff/issues/307.
Plan: [x] baseline/SDK/RED; [x] R1 суффикс удалений; [/] R4 код единой отмены/локальной фазы внесён, native execution not verified; [x] R2 позиции сумм и отказ неполной первой строки; [x] R3 счётчик SchemaMismatch; [x] R6 GUID-связь/payload/no-op; [x] R5 диагностика без смены fallback; [x] R8 отказ схемы без сумм; [x] R7 измерение без правок Helpers; [x] свежая LSP, независимое review и расширенные standalone suite с явным native-resource skip; [x] адресные документы/symbols и финальные границы native, callgraph не обновлён.
Last Completed: baseline RED131checks/39FAIL → live GREEN164/0 (условные assert ветки дают разное число проверок; входы одинаковые); реальные SDK25 reader, positional merge, plan/reconcile и builder payload, без native API. clang-format/diff-check PASS. Canonical BuildAddOn.py -c config.json -v25 exit0: Spec.cpp/SpecPlanning.cpp/TestSpec.cpp перекомпилированы, SomeStuff.apx перелинкован; оба PLN по SHA256 без изменений, hosts0/VSDesign перед build. Logs: scratch/spec-review-fixes-20261104/{canonical-build.log,regression/{red,green},objects}. MCP force-refresh показал0diagnostics всех четырёх owned files; прямой clangd --check дал ошибки code-action ExtractFunction, не compiler diagnostics, а TestSpec timed out — это НЕ свежая чистая LSP-приёмка. Отдельный cancellation harness две разные сборки не прошёл (дублирование ProcessWindowGuard, затем неполные SDK include): retries остановлены; отмена runtime not verified, не использовать старый cancel-live.log от baseline как GREEN.
Validation: последующий изолированный LSP publishDiagnostics успешно завершился: Spec.cpp0errors/2unused-includes, SpecPlanning.cpp/.hpp0/0, TestSpec.cpp0errors/3unused-includes; версии1 и hashes live совпали, свежие namespace ranges3151/475/5743 подтверждены. Это заменяет непригодный CLI --check как LSP-приёмку (fresh-lsp/publish-manifest.json и publish-raw.json). Независимое read-only review: No issues found in inspected scope; parent принял functional snapshot и повторно сверил final hashes (review-acceptance.md). Сборка и регрессии не являются native-приёмкой.
Regression: расширенный SDK25 standalone принят и повторно выполнен parent: Grouping193/0, Reconcile84/0, MergeAndKey24/0, Contribution122/0, RowLayout54/0, PlanCompleteness36/0, всего513/0. Contribution пропускает один case с ConvertBoolToParamValue/native localized resources (4 assertions), не выдаёт поддельное значение; остальные пять suite целиком. Totals пересчитаны по stdout, input hashes совпали с live. Артефакты: regression/existing/{report.txt,parent-run.log,input-hashes.json,extracted-manifest.json}; это не native-приёмка. Процедурные выводы сохранены в skill archicad-cpp-tdd-patterns/references/positional-aggregation-regressions.md и переданы OpenViking extraction.
Performance: R7 завершён: C++14/O2, настоящий извлечённый ParamValueData::operator+ и SDK25 GSRoot, 27 измерений на 1k/10k/100k строковых вкладов, три повторения; compile0/run0, equality PASS. Медианы исходного ядра6.3571/278.7642/125455.07ms; Append0.0549/0.3394/5.0143ms, UniString+0.6734/55.5415/63616.7865ms. Размер итоговой строки отдельно от peak-memory:73998/739998/7399998 UTF16bytes. Это microbenchmark, не время полного Spec; оптимизация общего Helpers.cpp не внесена и требует отдельного согласования. Артефакты: cancellation-perf/{perf-run.log,performance-manifest.json,parent-performance-summary.json}; Helpers SHA256 совпал с baseline.
Documentation: приняты адресные контракты Spec.md/SpecPlanning.md, _progress.md и symbols.json. Parent проверил119/119 symbol/source joins и соответствие live hashes;1009 внешних записей/порядок неизменны. MCP TestSpec stale, fallback на независимый свежий LSP явно отмечен. Callgraph не обновлён: унаследованные call_sites не подтверждены, это остаётся отдельным пробелом документации. Evidence: docs/{collection-audit-manifest.json,parent-symbols-acceptance.json}. Финальные format/diff-check PASS; исходные Spec hashes и оба PLN без изменений.
Next Step: по отдельному разрешению выполнить host-приёмку, все три отмены в настоящем SpecArray и пропущенный native-resource case. До неё задача WAITING_FOR_TEST и issue307 открыт. Другие AC/macOS и текущие координаты callgraph остаются not verified. Пользовательский Archicad не закрывать/не перезапускать, PLN не сохранять; commit/push/IssueClose не выполнять. Общий оператор суммирования не оптимизировать без отдельного scope.
Last Checkpoint: нового нет; commit/push не запрошены.

#### #260 — минимальный маршрут после ревью, без измерений

Scope: AC25 Windows; сохранить четыре cpp. Только предметная декомпозиция OtdRoom_BuildSurfaceFinishes внутри RoombookData.cpp, явное владение favdict в RoomProcessingContext с обновлением callers/тестов и перемещение локального итогового opw вместо копии. План: .hermes/plans/2026-11-04_100425-roombook-improvement-plan.md. По прямому указанию владельца P1/замеры/профилирование пропускаются; новых кэшей, CommonFunction, перестройки writers, геометрических правил/допусков, PLN, commit/push и закрытия issue нет. Issue: https://github.com/kuvbur/AddOn_SomeStuff/issues/260.
Status: WAITING_FOR_TEST — минимальные изменения кода реализованы и проверены; native-приёмка и документационная поставка не завершены.
Plan: [x] baseline и canonical AC25 build; [x] четыре локальных этапа сборки отделки; [x] favdict в контексте, восемь функций и native-fixture; [x] terminal move opw; [x] format/scoped diff/token-equivalence/canonical build/standalone/независимый review; [ ] Docs/modules/Roombook.md + generated/progress перед checkpoint; [ ] согласованная native-приёмка.
Last Completed: canonical AC25 Debug build после всех трёх дельт — exit0, Data/Run/Draw/Quantities/JSON/tests перекомпилированы, SomeStuff.apx перелинкован (proc_6bfa53c45507). Parent повторно скомпилировал и выполнил exact-production standalone surface2102PASS/0FAIL и favorites+SDK Array move1711PASS/0FAIL; специально испорченные варианты дали310 и343FAIL соответственно, оба compiled/run реально выполнены. Это управляемые reader/geometry/API boundaries, не native-приёмка и не all-member equality. Независимый review: No issues found in inspected scope; hashes всех шести проверенных source-файлов совпали с live. Повторный verify_structure.py, clang-format --dry-run --Werror и scoped git diff --check — PASS; оба PLN совпали с исходным SHA256. Исходные RoomAnalytic46/0, FloorCore41/0, CoordinateSearchPASS и attribution старого TestMeshFloor unused-lambda-capture сохранены в scratch. Свидетельства: scratch/roombook-minimal-20261104/{structure-proof,surface-regression,favorites-regression,final-review,diagnostic-attribution,baseline,applied-source.json}. Параллельные Spec/IDEA-правки не принадлежат260 и не изменялись.
Next Step: с разрешения владельца выполнить native-создание/повтор/обновление на безопасной фикстуре. До checkpoint адресно обновить карточку Roombook, generated и progress: кандидаты scoped-docs НЕ приняты и НЕ применены (acceptance.json), сначала устранить неоднозначное сопоставление declarations/definitions и сохранить формат/внешние записи; не повторять полный генератор. Карточка пока не описывает context.favdict/четыре новых этапа; координаты generated унаследованы. Native host, другие AC версии/macOS и ускорение — not verified. Измерения пропущены по указанию владельца; новые кэши/commit/push/закрытие issue не выполнялись.
Last Checkpoint: исходный13dfb0c; новых коммитов не запрошено.

#### #306 — GUI roombook_setup.py в дизайне somestuff_updater

Scope: только GUI-слой python_scripts/roombook_setup.py (класс App переписан по образцу somestuff_updater: палитра C_BG/C_CARD/C_ACCENT, карточки, badge-статусы, clam, кнопки Accent/Ghost, очередь queue.Queue/_pump вместо after-опроса, DPI-awareness не добавлялся — tk.Tk сам, smoke прошёл). Логика проверок и вкладка «Свойства» (#305: «Текст (маркер)» + сохранение блока {...}) не менялись. Добавлены импорты ctypes/queue. CLI (--cli, --create-classes) не тронут. Issue: https://github.com/kuvbur/AddOn_SomeStuff/issues/306.
Status: WAITING_FOR_TEST — py_compile OK, GUI smoke 2 c OK (24 строки маркеров), --cli на живом порту 19723 отработал (результат как до правки: FAIL из-за отсутствующих зон-свойств, не связано с правкой).
Дополнение 2026-10-30 (запрос владельца): добавлены (1) автовыбор темы по реестру Windows (AppsUseLightTheme) + переключатель «ТЕМА» в шапке, палитра THEMES light/dark, _apply_theme перекрашивает живые tk-виджеты; (2) секция проверки скрыта по умолчанию, разворачивается внизу окна после нажатия «Проверка» (report_outer.pack в run_check); (3) человекочитаемые имена свойств в GUI и в отчётах (MARKER_DESCRIPTIONS, «Наличие потолка» вместо some_stuff_fin_has_ceil, маркер — мелкой строкой). Smoke2 прошёл.
Next Step: визуальная проверка владельцем.
Last Checkpoint: aaa8276 [python-setup] GUI: тема, скрытая секция проверки, человекочитаемые имена, ссылка на сайт.


#### #305 — понятные описания свойств отделки в roombook_setup.py

Scope: AC25; только python_scripts/roombook_setup.py (+Readme.md); описания ставятся через SetPropertyDescription (#304). Маркер обязан остаться в описании (аддон ищет по Contains(marker)), формат: «Понятный текст (маркер)», тексты из wiki/ru/Finishing-ru.md (MARKER_DESCRIPTIONS, 24 маркера); блок форматирования результата «{ 30 ; 15 ; ... }» сохраняется (extract_format_block). Классы --create-classes не тронуты. Без commit/push/закрытия issue. Issue: https://github.com/kuvbur/AddOn_SomeStuff/issues/305.
Status: WAITING_FOR_TEST — правка внесена, py_compile OK; runtime на живом test_25 (порт 19723): 8 найденных result-свойств получили понятные описания, read-back OK, штатная проверка check_properties находит те же 8 маркеров, перекрёстных совпадений нет; блоки форматирования сохранены. Остальные 16 маркеров в тест-проекте свойств не имеют (создание свойств — вне скоупа).
Next Step: (по желанию владельца) прогнать GUI «Применить выбранные» на своём проекте; коммит по запросу.
Last Checkpoint: нет; коммит не запрошен.


#### #304 — JSON-команда SetPropertyDescription (описание свойства)

Scope: AC25 Windows; новая add-on JSON-команда RoomBookSetPropertyDescriptionCommand ("SetPropertyDescription") в json_commands/RoomBookCommand.hpp/.cpp, регистрация в JsonCommandRegistrar.cpp. Поиск свойства максимально через PropertyCache (cache.property → ReadPropertyDefinition → запасной FindPropertyGuidByGroups по всем группам с дозаполнением кэша); перед записью определение перечитывается по GUID (Change пишет структуру целиком); запись в ACAPI_CallUndoableCommand; после записи — UpdatePropertyDescriptionInCache (обновление ParamValue.definition + GetPropertyRuleFlag (сам пересобирает propertyRuleFlags/addonrulesbytype) + ReadFileFromDefinition); read-back в ответе. Без изменения PLN, commit/push/закрытия issue. Issue: https://github.com/kuvbur/AddOn_SomeStuff/issues/304.
Status: WAITING_FOR_TEST — код внесён, clang-format/LSP 0; AC25 canonical build OK (16:25, restart_archicad_for_test.ps1; «203 failed» в CXX_TESTS — отчёт предыдущего прогона 16:15, mtime-ловушка). Runtime на живом порту 19723 (test_25) проверено: schema-reject 4002; неизвестное имя → APIERR_BADID; запись по GUID и по имени (через кэш PropertyCache) → changed:true + read-back "#304 test"; idempotent-повторы → changed:false; возврат исходного пустого описания и повтор → changed:false. Модель возвращена в исходное состояние (свойство Спецификации общестрой/Поз, GUID 409302A5-560C-424C-8C73-44B7D616C017), in-memory, без save.
Plan: [x] проверка штатного JSON API (редактирования описания нет) и SDK-контракта (ChangePropertyDefinition, Property_Test, HTML AC25); [x] issue #304; [x] команда + регистрация + clang-format; [x] LSP 0; [x] AC25 build; [x] runtime через JSON-порт (запись, read-back, idempotent no-op, ошибка несуществующего имени, restore).
Next Step: перезапустить Archicad (с согласия владельца), собрать, проверить SetPropertyDescription на живом порту.
Last Checkpoint: нет; коммит не запрошен.


#### #303 — создание классов отделки: аддонная JSON-команда + режим --create-classes
Scope: AC25 Windows; новая JSON-команда создания элемента классификации (обёртка ACAPI_Classification_CreateClassificationItem) в RoomBookCommand.hpp; режим --create-classes в python_scripts/roombook_setup.py: выбор существующей системы и родительской ветви, создание 13 классов из RoombookConstants.hpp. id = маркер, name = человекочитаемое, description = маркер. Систему не создаём. Без изменения PLN, без commit/push/закрытия issue. Issue: https://github.com/kuvbur/AddOn_SomeStuff/issues/303.
Status: WAITING_FOR_TEST — команда CreateClassificationItem и режим --create-classes реализованы и проверены на живом AC25; коммит 21e84ca (Refs: #303, #300, #301), push не выполнялся.
Plan: [x] исследование штатного JSON API (menutree AC30, Tapir: создания классификаций нет) и SDK-контракта; [x] issue #303; [x] аддонная команда + регистрация; [x] build + runtime; [x] python-режим и Readme; [x] idempotent-проверка.
Last Completed: runtime AC25 (порт 19723, test_25): IsAddOnCommandAvailable=true; создание тестового элемента под «АР/Отделка» (21D00C33…) прочитано обратно штатным GetAllClassificationsInSystem; --create-classes idempotent (13/13, создано 0); негативные ветки: без параметров 4002, несуществующая система — ошибка API возвращена; read-only проверка не задета. Тестовый элемент только в памяти (PLN не сохранялся).
Next Step: реальный сценарий с недостающими классами на проекте пользователя; при необходимости — проверка выбора родительской ветви не из корня.
Last Checkpoint: 21e84ca.

#### Docs: полная регенерация документации Roombook (2026-10-30)

Scope: AC25 Windows; Docs/ и Docs/_generated/ только. Снимок кода — коммит b199125 (Sources/AddOn/roombook, C3-WIP). Без правок C++, без push. Документация переписывается с нуля: только актуальное состояние, без истории изменений; ключевая секция — карта создания/изменения отделочных элементов от входа до ACAPI_Element_Create/Change.
Status: DONE — docs переписаны и закоммичены `c40954f` `[docs]` в ветви roombook_refactor (базовый снимок кода `b199125`; при мерже в master Docs уедут вместе с кодом).
Plan: [x] коммит-снимок roombook + свежий compile_commands (--lsp AC25); [x] регенерация symbols.json (clangd MCP, батчи, слияние + дисковая привязка строк); [x] пересбор callgraph roombook-среза (grep-метод + входные рёбра Main/JSON); [x] карта create/change (scratch/docregen/create-change-map.md); [x] Roombook.md с нуля; [x] сопутствующие карточки (Helpers/CommonFunction/json_commands), DISCREPANCIES; [x] ARCHITECTURE/REPOMAP/SEQUENCES; [x] _progress.md переписан (компактное текущее состояние); [x] [docs]-коммит c40954f.
Next Step: задача завершена. Хвосты — вне Scope: не-spec срезы callgraph (140 устаревших рёбер), MEPv1.cpp в symbols.json.
Last Checkpoint: c40954f [docs] регенерация документации roombook-среза.

#### #302 — пустые beamPart и предупреждения Surface finish

Scope: AC25 Windows; только причины диагностик в RoomBeamFinish_Build/OtdRoom_BuildSurfaceFinishes, адресные регрессии и карточка Roombook. Сохранить конечные контакты и реальные отказы; не выбирать владельца Ambiguous произвольно. Без изменения PLN, активной VS-сессии, избранного, writers, допусков, commit/push/закрытия issue. Issue: https://github.com/kuvbur/AddOn_SomeStuff/issues/302.
Status: WAITING_FOR_TEST — production-правка внесена, standalone RED/GREEN 25/0, полная TU и fresh LSP пройдены, независимый review — No issues; canonical build/relink и native-приёмка отложены (активная VS-сессия, live модель не останавливать).
Plan: [x] сохранить dirty baseline и зарегистрировать отдельную issue; [x] проверить минимальные маршруты и SDK25-контракты; [x] GREEN-характеризация и адресная RED; [x] конечное сечение зоны для нулевой beamPart, без дублирования повторных relations; [x] отделить штатный Unmatched от Ambiguous и отказов adapter; [x] format/TU/fresh LSP/независимый review; [x] документация и итоговые границы native/build; [ ] native-приёмка после согласованной пересборки.
Last Completed: frozen Data5baba096… и связанные файлы сохранены в scratch/surface-finish-warnings-fix/before + before-manifest.json; issue302 создана и прочитана обратно. #252 фактически CLOSED, не переоткрывалась. VS Run/PID40324 сохранён. LightRAG naive вернул chunks без подтверждения семантики нулевого контакта; установленная SDK25 API_BeamPart.html определяет начало/конец, но не политику нулевого интервала. Числовой пересчёт t из ранее наблюдённого маршрута не меняется без новых доказательств.
Decision: для нулевой конечной relation с полной зоной использовать существующее пересечение исходной конечной грани с Zone_BuildSection, без искусственного расширения оси; обработать такую балку один раз независимо от порядка её relations. Без зоны сохранить прежний отказ нулевого интервала. Unmatched без claims не является отказом; Ambiguous не подавлять по одним типам claims, поскольку наличие альтернативного маршрута не доказывает покрытие: оставить отдельное честное предупреждение без выбора владельца. Реальные ошибки builder/adapter сохраняются.
Validation baseline: Data ccdd29ba… до собственной правки не проходит полную TU: C2561 Favorite_GetDefultName:9148 (возврат без значения); это посторонняя текущая дельта, не исправлять в302. Родитель перепроверил: frozen baseline 5baba096… RED 25 checks/10 fails (repo-red), live 692e1d… и d6fadadc… GREEN 25/0 (repo-green/repo-green2) — тестируются извлечённые реальные диапазоны RoombookData.cpp, не копия алгоритма. Новые постоянные регрессии в TestRoombookGeometry.cpp: полная TU MSVC C++14/SDK25 exit0, fresh LSP 0 diagnostics; формат чист. Review 692e1d…: No issues found in inspected scope (pointer lifetime, dedup/order, predicate parity, fallback geometry, Ambiguous warning, fixtures).
Next Step: native-приёмка после согласованной пересборки/перезагрузки (canonical build + Archicad прогон на test_25); live модель/VS-сессию не останавливать без разрешения владельца. Дополнительно: caller-level routing-перестановки и warning dispatch покрыты структурным review, не runtime; harness routing/ (deleg_173d8fc2 task-2) не завершён — провайдер rate-limited (HTTP 429).
Last Checkpoint: нового нет; commit не запрошен.

#### #301 — создание недостающего избранного отделки
Scope: AC25 Windows; новая JSON-команда Roombook в общем RoomBookCommand.hpp, не изменять существующее избранное; без запуска на пользовательском проекте или прерывания отладки. Issue: https://github.com/kuvbur/AddOn_SomeStuff/issues/301.
Status: CLOSED — 2026-10-30. Решение: массовое создание недостающего делает roombook_setup.py (python); C++ — одна запись за вызов. Команды Favorite_CreateOtd и Roombook_CheckFinishConfiguration добавлены и собраны AC25; финальная проверка ранером: build OK, Archicad запущен.
Plan: [x] проверить доступный код и зарегистрировать запрос; [x] подтвердить базовую сборку и SDK-контракты (LightRAG: Favorite_Test Do_ExportReadCreateFavorites); [x] команды Favorite_CreateOtd и Roombook_CheckFinishConfiguration добавлены в RoomBookCommand.cpp/.hpp, регистрация в JsonCommandRegistrar.cpp, clang-format/LSP/canonical AC25 build OK; [x] финальная проверка restart_archicad_for_test.ps1 (build OK, 203 провала в 4 сьюитах вне скоупа — TestClearZoneGuid/TestOtdWallAddOne/TestOpeningRevealsCreateOne/TestRoomSurfacePartition, грязный worktree); [x] закрыть issue.
Last Completed: issue закрыт с комментарием; массовое создание отдано python-скрипту.
Next Step: нет.
Last Checkpoint: нет; коммит не запрошен.

#### #300 — Python Roombook setup: имена избранного и классов
Scope: AC25 JSON; FavoritesList read-only endpoint через Roombook::Favorite_GetDict, регистрация и подключение `python_scripts/roombook_setup.py`; RoomBook algorithm/model/other dirty work не менять, без commit/push/закрытия issue.
Status: WAITING_FOR_TEST. Issue: https://github.com/kuvbur/AddOn_SomeStuff/issues/300.
Plan: [x] обновить имена Python checker; [/] объединить команды Roombook в одном hpp, подключить FavoritesList и проверить достоверность отчёта; [ ] AC25 build/runtime и проверки.
Last Completed: объявления команд находятся в RoomBookCommand.hpp; Python обработка полного, частичного, ошибочного и отсутствующего ответа проверена с подменой. Favorite_GetDict пропускает ошибки отдельных типов и может выводить msg_rep; ненайденные имена показываются как непроверенные.
Next Step: после завершения активной отладки выполнить AC25 build и проверить FavoritesList на живом проекте; подтвердить загруженный бинарник.
Last Checkpoint: нет; commit не запрошен.

#### #299 — пропуск обновления без отличий

Scope: AC25 Windows; сравнения управляемых данных в Beam/Wall/Roof/Slab/Object writers RoombookDraw.cpp и выбранные header-поля из избранного: floorInd из геометрии, layer/renovationStatus из настроек. Настройки/memo избранного читать отдельно от существующего элемента; весь элемент/memo не подменять. Поиск/reuse, геометрию, hosted-проёмы, shared GDL helper и сохранённые PLN не менять; без commit/push/закрытия issue. 2026-10-30 пользователь запустил test_25 через VS и разрешил native-проверку в текущем процессе; только in-memory, без save/build/restart.
Status: WAITING_FOR_TEST (оба дефекта исправлены; RED/GREEN, полная TU/fresh LSP и независимый review пройдены; native-приёмка новой версии ещё не проведена).
Plan: [x] исходные per-field guards и standalone regression; [x] подключить Elems_Check_Update во всех шести writers с раздельным чтением избранного/current и переносом layer/renovationStatus; [x] format, SDK25 regression/TU/LSP и независимый review прежней дельты; [x] выполнить native-проверку в запущенной пользователем VS-сессии и зафиксировать отказы; [x] проверить эффективный default renovationStatus и исправить его сравнение; [x] ограничить структурные атрибуты/маски Wall/Roof активным типом (Slab уже не сравнивает их); [x] RED/GREEN, полная TU/fresh LSP и независимый review исправлений; [ ] повторная native-приёмка после согласованной перезагрузки, включая Roof/Object.
Last Completed: Draw 5e92adb042fbecbd6dd08047a0f5d20bbf48251bd9e7259e5bce252c69a9c779: полная TU MSVC C++14/SDK25 exit0; format/dry-run/diff-check0; fresh direct LSP — те же прежние5unused-variable+1include. Родитель выполнил standalone GREEN746/0 против RED703/43 на точном baseline f3bcdf2d…; проверены все6writers, отдельные header поля/маски, сохранение GUID/unmanaged GDL/Beam memo, создание, повторный no-op и отказ чтения элемента. SDK-вызовы/defaults/persistence контролируемые, не native. Прежний beamShape-тест исключён: соответствующую запись владелец убрал до этой дельты. Evidence: scratch/favorite-header-299 и scratch/favorite-header-299-tests/{red-verified,green}; исходные no-op evidence: scratch/noop-update-299.
Review: No issues found in inspected scope — независимый review header/Favorite-дельты для того же 5e92adb0…; родитель сверил live hash с compile/regression/review. Оба memo имеют отдельное владение, маски header не очищаются после helper. Правило сохранено в lightrag/SKILL.md.
Native 2026-10-30: пользовательский VS Debug x64 действительно compile/relink Draw, загружен APX00f8928b…; PID40324/test_25/AC25.7000 RUS. Три RoomBook прохода для зоныA2100887…: созданы33 finish GUID (20Wall/11Beam/2Slab), все независимо найдены; оба повтора сохранили их GUID, без новых finish. Change существующих Wall/Beam/Slab возвращал0. Но Beam52F18810… на обоих повторах сравнивает Existing с входным псевдостатусом Default; WallF5F2FCC7… Basic повторно сравнивает неактивный composite131→54, наблюдено needupdate false→true именно на этой ветке. Контракты подтверждены установленными SDK25 HTML; standalone746/0 не моделировал эту host-нормализацию. No-op приёмка НЕ пройдена. Roof/оба Object update gates не покрыты. В панели также OtdBeam_Draw BADPARS/unsupported geometry; причины не исследованы. !ConvertToProperty err не считать доказательством отказа записи: helper возвращаетfalse и при равенстве. После прогона все собственные точки удалены, исходные2disabled сохранены, VS Run. Draw/APX/оба saved PLN hashes unchanged; модель менялась только in-memory. Evidence: scratch/favorite-header-299/runtime/report.md + JSON inventories/read-backs/log delta. Итог issue299 comment6510594216 прочитан обратно.
Decision 2026-10-30: GetDefaults не гарантирует конкретный header.renovationStatus — этот вариант не использовать. Для Default у существующего GUID сравнить category-value GUID (RenovationStatus) текущего элемента и default того же type/variation через документированные SDK25 GetCategoryValue/GetCategoryValueDefault; при различии записать исходный Default, без преобразования GUID в enum. Ошибка/пустой category GUID — консервативно сохранить прежнюю запись, а не считать no-op. Explicit status/create-путь не требуют этих чтений. Структурные buildingMaterial/composite сравнивать по целевому Basic/Composite; profileType — cross-section, не profileAttr, его guard не менять. Slab не сравнивает эти атрибуты: production-правка там не требуется.
Correction 2026-10-30: Draw f84ecbf501b39fe13e1ce788ee1f7ef7595e731f20d8a66b42dfecc93fcc2bbb — общий header helper сравнивает category GUID для Default; Wall/Roof проверяют и маскируют только активные buildingMaterial/composite. Production-дельта только в Draw, profileType сохранён. Родитель выполнил GREEN1051/0 против authoritative RED919/132 на frozen5e92adb0…; последовательность1051assertions и hashes всех3harness-файлов совпадают, все919прежнихPASS сохранены, все132RED сталиPASS. Standalone SDK25/C++14, не host; обе ошибки category API, оба nullGUID, все6writers, три реальных статуса/default/создание/второй no-op, active Composite Roof и inactive masks покрыты. Полная TU object MSVC14.29 exit0; format/dry-run/diff-check0; fresh LSP имеет те же прежние5unused-variable+1include, без новых. Evidence: scratch/favorite-header-299/correction/{own.diff,acceptance.json,final-tu,final-lsp,tests/{green-characterization,red-category,green-category}}. GetDefaults-эксперимент tests/red не является итоговой RED. APX/оба saved PLN SHA совпадают с baseline, VS Run не тронута. Canonical addon build/relink и native новой версии НЕ выполнялись. Итог issue299 comment6510594216 обновлён и точный read-back сверён, issue OPEN. Процедурный контракт дополнен в lightrag/SKILL.md.
Correction Review: No issues found in inspected scope — независимый read-only review двух исправлений и всех6helper-callers, creation/explicit/default/error/nullGUID/structure transitions. Hash f84ecbf501b39fe13e1ce788ee1f7ef7595e731f20d8a66b42dfecc93fcc2bbb родитель повторно сверил с live source/regression/TU/fresh LSP; APX и оба PLN по-прежнему byte-identical. Текущая задача исправления кода завершена, исходная #299 ждёт native-приёмки.
Next Step: согласовать безопасную пересборку/перезагрузку и native-приёмку, включая Roof/оба Object и истинный no-op, а не только отсутствие новых GUID. Не Stop/Save/Restart/Commit/Push/IssueClose без разрешения. Baseline Draw/IDEA и dirty diff — scratch/favorite-header-299/correction/before + manifest-before.json; итог — correction/report.md. Docs/generated отложены до явного pre-checkpoint запроса.
Last Checkpoint: нового нет; commit/push не запрошены. Issue: https://github.com/kuvbur/AddOn_SomeStuff/issues/299.

#### #285 — заход полов до середины коробки

Scope: AC25 Windows; OtdOpening/Param_SetToWindows, FloorContour_ExtendToOpening и согласованный mesh-demand; адресные регрессии и карточка Roombook. Глубины откосов, writers, PLN, чужие изменения не менять; без commit/push/закрытия issue. Generated требует свежего clangd, не подменять устаревшим AST.
Status: IN_PROGRESS.
Plan: [x] текущий маршрут/контракты и baseline; [/] адресная RED/GREEN и минимальная дельта; [ ] format/LSP/доступная SDK25 проверка и review; [ ] native-приёмка после безопасного разрешения сборки/перезапуска.
Last Completed: исходная глубина исключает всю коробку; новая глубина = base_reveal_width + base_frame_width/2 отдельно от откосов. Issue285 OPEN, уточнение6500047745 прочитано обратно. Baseline в Hermes scratch/floor-door-midframe-285. LightRAG вернул нерелевантные сведения, опора — текущий reader gs_frame_thk/gs_wido_sill и подтверждённый контракт #148.
Next Step: добавить сохранение толщины коробки и адресные проверки стыка обоих полов; текущая VS Run, сборку/перезапуск/модель не трогать.
Last Checkpoint: нового нет; commit/push не разрешены.


#### #295 — привязка избранного и сторона верхнего откоса

##### Уточнение нижней левой привязки — 2026-11-05

Scope: AC25 Windows; только горизонтальная компенсация UpperReveal_GetBeamOffset в RoombookDraw.cpp и адресная standalone-регрессия. Для нижней левой привязки дополнительный offset=0 по уточнению владельца. Не менять anchorPoint, высоту, begC/endC, Wall_Draw, избранное, PLN и чужие дельты; без commit/push/закрытия issue, остановки VS и перезапуска.
Status: WAITING_FOR_TEST — минимальная правка и независимый review завершены; addon build/native не выполнены из-за активной отладки и запрета перезапуска без согласования.
Plan: [x] текущий writer/producer, issue295 OPEN и dirty-baseline; [x] RED-регрессия и минимальная коррекция столбцов; [x] format, свежая LSP/отдельная TU; [x] независимый review и итоговые границы native; [ ] согласованные canonical build и native-приёмка.
Last Completed: в Draw заменены только формула и комментарий: левый столбец0, центр-width/2, правый-width; anchor/высота/ось/Wall_Draw неизменны. Точная production-функция + caller-блок, реальные SDK25 types/masks и is_equal, MSVC C++14: RED654checks/90fail → GREEN654/0, включая reset прежнего offset, mask, повтор, другие роли и отказы. Весь Draw скомпилирован в scratch .obj exit0, не linked addon. clang-format/dry-run/diff-check0. Свежий direct clangd: те же5 unused-variable,64 инструментальных ExtractFunction failures; MCP0 не принят как свежий GREEN. SHA Draw6b38edab… совпал при компиляции/прогоне/LSP; оба PLN и чужие исходники SHA сохранены, VS Run не тронута. SDK25 не расшифровывает short anchorPoint; LightRAG naive beam-семантику не подтвердил, онлайн сетка относится к AC30. Evidence: Hermes scratch/upper-reveal-anchor-20261105/{baseline.json,own.diff,red,green,tu,lsp-summary.json}; независимый review deleg_e61a1cbb: No issues found in inspected scope. Parent после review повторно сверил live SHA с GREEN/TU и hashes защищённых файлов.
Next Step: согласовать безопасные canonical build/reload и native create/update/repeat с нижней левой привязкой. Нативные знаки остальных привязок/направлений AC25 not verified. Не останавливать отладку, не трогать PLN/избранное. Docs/generated отложены до отдельной документационной поставки, символы/координаты этой двухстрочной дельтой не меняются. Memory: lightrag/SKILL.md дополнён независимым zero-offset oracle; источник передан OpenViking extraction.
Last Checkpoint: нового нет; commit/push не запрошены. Backlink: #295 kuvbur/AddOn_SomeStuff.

##### Предыдущая реализация — 2026-10-29

Scope: AC25 Windows; Beam_Draw / Wall_Draw, коррекция размещения верхних откосов без смены линии привязки и оси. Минимальный флаг геометрической роли в OtdDrawData/подготовке нужен, поскольку Reveal_Up — высотная полоса, а не верх проёма. PLN, избранное, расчётные координаты geom, прочие роли и чужие изменения не менять; commit/push не выполнять. Issue #295 создана и прочитана обратно.
Status: WAITING_FOR_TEST — код внесён, review-замечание устранено и воспроизведено адресной RED/GREEN-регрессией. Нативная SDK25-матрица flipped/sign не проверена; сдвиг оси запрещён. Центральные/core wall-привязки теперь отклоняются до native-вызова, без подмены привязки и без использования старого offset. Повторная компиляция Data заблокирована посторонним новым блоком FavoriteDefaultOtd с дублирующими полями; его не изменять в #295. Native/restart не выполнять.
Plan: [x] writers/default reader/подготовка и issue; [x] ограничения SDK25 wall.offset; [x] ручные baseline скриншоты и запрет сдвига оси; [x] внесена адресная правка, format/diff-check и standalone checks144/0; [x] независимый source-review и исправление замечания; [ ] устранение внешней compile-блокировки, canonical build и полная native-матрица знаков/привязок AC25 по отдельному разрешению.
Last Completed: Header — default-false draw.is_upper_reveal; Data — единственная установка для геометрического верха перед высотной разбивкой (копирование сохраняет флаг). Draw — Beam offset по столбцу anchor и итоговой ширине, в mask для update; только Basic rectangle без profileAngle. Wall верх читает raw GetDefault, сохраняет referenceLineLocation/flipped, задаёт depth как thickness и учитывает Inside/Outside+flipped; Center/Core исключены из offset-table. Удалена запись output-only offsetFromOutside. Shared Wall_GetDefault не изменён. Native-правильность полной таблицы not verified: номера Beam найдены в официальном онлайн Doxygen AC30, не в SDK25 HTML; соответствие UI/API и flipped знаков AC25 не наблюдалось.
Validation: MSVC SDK25 C++14: Draw/Data полные TU→scratch .obj exit0 (не linked addon); точные production offset-helpers с SDK25 types→exe127checks/0fail. Это проверка таблицы и исключений, НЕ доказательство native размещения. Первый harness link не включал GSRootImp.LIB; исправлено только в scratch, следующий compile/run0. clang-format/dry-run/diff-check0. Clangd: Draw5 прежних unused-variable+1unused-include, Data3+5, header0errors+1unused-include; весь LSP GREEN не заявлять. Header/Data до добавлений восстановлены в scratch и SHA совпали с baseline; собственный diff Draw проверен, остальные roombook файлы baseline SHA неизменны. Evidence: Hermes scratch/reveal-offset-295/implementation/{before.cpp,baseline.json,validate.py,validation.json,own.diff,*.own.diff,*.log}; addon build/restart/native не выполнялись.
Review/late validation: deleg_53dd949b подтвердил propagation роли, сохранение reference/flip и Beam mask; найденный игнорируемый bool в Wall_Draw903 исправлен на ранний return. Выполнен точный upper-wall production block + helpers + реальный is_equal с SDK25: RED144checks/9fail → GREEN144/0. После правки Draw полная TU MSVC C++14 compile0 (SHAfa8a54e3…), повтор Data compile2: C2086 favorite_floor_wall/favorite_reveal_side в новом FavoriteDefaultOtd, которого не было в SHA baseline76b1a4f1…. Concurrent diff сохранён отдельно, собственная единственная Data-вставка сохранена; посторонние правки не откатывались. Нынешний Data не считать скомпилированным. Draw MCP0 после правки может быть stale; прежние diagnostics не объявлять исправленными. Diff-check0. Evidence: implementation/{caller-red.json,validation.json,concurrent-data.diff,RoombookDraw.log,RoombookData.log}. Это не linked addon/native.
Next Step: вне #295 устранить дубли FavoriteDefaultOtd (Data текущие63/65), затем отдельно согласовать build/native AC25 и вручную проверить все поддерживаемые привязки/flip, create/update. Не считать checks144/0 доказательством native-знаков offset. Canonical build перезаписывает build-copy PLN, без согласования не запускать. Docs/generated не обновлены (вне текущей дельты); #295 OPEN.
Last Checkpoint: нового нет; commit/push запрещены, чужие изменения сохранены. Memory: lightrag/SKILL.md — геометрический верх != Reveal_Up, роль/маска/границы native-проверки.

#### #294 — вычитание профильных стен из полов зоны010

Scope: AC25 Windows; коррекция производных горизонтальных полов по сечению прямых профильных стен. Не менять исходную зону/стены, PLN, обычные стены/потолки, Helpers и чужие изменения; commit/push не выполнять. Issue294 создана и прочитана обратно; связана с252.
Status: WAITING_FOR_TEST — VS Debug x64 собрал и загрузил новый APX в Archicad25 PID3404, два запуска RoomBook вернулись; подготовка010 наблюдена до/после вычитания. Полная независимая приёмка созданных элементов и проходов не завершена. Отметка сечения — низ зоны (round(GetzPos(zone.roomBaseLev,...)*1000)/1000), не floor.zBottom. Baseline11файлов: Hermes scratch/profile-floor-294/before-manifest.json.
Plan: [x] прочитаны текущий маршрут и образец profile-reader; [x] dedup/issue294 с алгоритмом и границами; [x] согласована отметка сечения — низ зоны; [x] источник отметки и transform текущей flipped-стены010 проверены source+VS/native; [x] минимальная коррекция и адресные SDK25-регрессии; [x] свежий LSP/review; [ ] native-приёмка нового кода после разрешения сборки/перезапуска. На текущей стене local x направлен влево от beg→end, local y=worldZ-wallBase; offset повторно не добавлять. Все15штриховок сняты отдельно; при zoneBottom0/wallBase-0.1 крайние offsets -0.37/+0.075 совпали с независимым native-сечением. Иные orientations/modifiers отдельно not verified. Вычитание ставить после final provenance validation и до dedup/skirting: индексы MixedFloorProvenance после дробления пола больше не используются.
Last Completed: Data3983–4145 — signed-сечение SyHatch/Intersect из memo.stretchedProfile, конечные прямоугольники с вырезами открытых окон/дверей; raw opening.lower переводится в wallBase+lower. Data5735+ — вычитание после core/provenance и до dedup/skirting. Сохраняются все части и metadata; reused GUID остаётся только у первой части, полностью вырезанный reused Floor возвращается в deletion pool. Header — два TESTING declarations; TestRoombookGeometry2110+ — адресные регрессии. Helpers/прочие production/оба saved PLN совпали с baseline; прежний APX33e96487… заменён сборкой 31c9cbbc… (см. Live verification).
Validation: MSVC C++14 SDK25 — обе полные TU скомпилированы в scratch .obj (exit0), не linked addon. Исполнены точные kernels+repo checks+23строчный production orchestration block:28pass/0fail, включая captured15-hatch профиль и независимый native-section oracle, split/repeat/metadata/retiredGUID. Не host-приёмка нового кода. Shared clangd stale (старые ends), его0отброшен; отдельный direct clangd version1 с актуальными ends/SHA: Data прежние3unused-variable+5unused-includes, test0. clang-format/diff-check0. LightRAG не дал точного placement contract; SDK25 docs+VS/native использованы fallback. Evidence: Hermes scratch/profile-floor-294/{validation.json,object-results.json,captured-result.json,fresh-lsp-result.json,*own.diff}. Memory: lightrag/SKILL.md обновлён.
Next Step: независимо проверить итоговые GUID и native-геометрию пола010, покрытие прохода и повтор без накопления; проверить что native-сечение и проёмы правильно задают exclusion для этого случая. Не считать API status returned доказательством записи. VS Run PID3404 оставлен открытым, собственных breakpoints нет; перед перезапуском защитить изменения модели. Canonical BuildAddOn не запускать без защиты build-copy PLN. Mesh/наклонные полы пропускаются; иные native orientations/modifiers, API_OpeningID и end-to-end native нового кода not verified. Docs/generated не обновлены; issue294 OPEN, commit/push/закрытие не выполнять. Review deleg_5f4c6c83: No issues found in inspected scope; поздняя GUID-коррекция отдельно проверена родителем и выполненным production-block regression.
Live verification 2026-10-29: VS F5 Debug x64 2 success/0 failed, реально перекомпилированы Data/TestRoombookGeometry и перелинкован APX SHA256 31c9cbbc79e35db0e6acad6791ac06a2d2f4f49366a2a9fd58c1fdf02726de22; именно он загружен PID3404. Оба source PLN и проверенные исходники SHA неизменны. Два API RoomBook вернули succeeded=true/status=returned. Breakpoint Data5774, зона E8A428C1-40C9-40C2-B778-76E3DB250062, zBottom0: profileExclusion size1/area1.797800084281042; после stepOver 10 slabs→5: площади первых двух Floor 1.668024879885209→1.668024879885209 и 15.771616833940129→14.761616694247799; три Ceil сохранили площади 4.024700448362097,0.637840110922891,6.416334125596165. Те же значения в повторе; оставшиеся пять Floor не исследованы, причины их исчезновения и actual writer not verified. Первый debug pane не показал запуска TestKit; второй запрос pane завершился SSE error, отсутствие ошибок повторного writer не доказано. Canonical build не проводился; исходная/копия PLN на диске не изменены, модель в памяти изменена командой. Своя точка снята, исходные disabled сохранены.
Исследование исчезновения 2026-10-29: дополнительный RoomBook в той же модели; breakpoint Data5774 и step over 5777: 10→5. Пять исключённых Floor до операции индексы 2,6,7,8,9; прямоугольники без дуг площадью 0.7877999665, 0.8080000195, 0.9291999910, 0.0201999254, 0.0404000953 м². Снятые из памяти вершины и контур exclusion — Hermes scratch/profile-floor-294/disappearance-before.json; exclusion прямоугольник [-46.8513476192,-42.8113476192]×[18.6739612733,19.1189612942], все пять находятся внутри с погрешностью крайних координат не более 3.76e-8 м. Участок 2 пересекает 6–9: суммы их площадей не складывать как неперекрывающиеся. Сохранились два Floor и три Ceil. Это объясняет исчезновение при вычитании, но не доказывает верность самого exclusion, проёмов и записи созданных элементов. Запрос завершился returned, модель в памяти изменена; точка снята, исходные disabled сохранены.
Last Checkpoint: нового нет, commit/push не выполнялись.

#### #270 — нижние плинтусы: пропуски во всех тестовых зонах

Scope: AC25 Windows, текущая Build/SomeStuff/25/test_25.pln. Владелец разрешил «Давай исправим обе проблемы»: наклонные полы и ролевые классы/ownership плинтусов. Минимальная C++-дельта с регрессиями и настройка текущей тестовой модели без Save. Не подменять сохранённые PLN/избранное; не удалять старые балки неизвестного происхождения; commit/push/закрытие не выполнять. Собственный VS-сеанс допускает остановку для сборки, с сохранением точных дисковых fixture.
Status: WAITING_FOR_CASE — AC25 test_25 PID37868 открыт под VS, порт19723. Все24lower соответствуют независимо рассчитанным контактам подготовленных поверхностей; дополнительный дефект расчёта не доказан. Для продолжения исправления оставшихся пропусков нужен конкретный ожидаемый отсутствующий участок в текущем результате. Полноценная native-проверка поверхностей участников не завершена. Dirty-baseline сохранён в Hermes scratch/lower-skirting-fix-270/before.
Plan: [x] source/issue270 OPEN и dirty-baseline; [/] slope и classification/ownership preflight: адресные стенды проходят, полная геометрия не принята; [x] оба класса и реальные ownership доступны на созданных плинтусах; [/] форматирование/свежий LSP/AC25 build; [/] native-приёмка наклонов, связей и повтора; [ ] документы/generated/память и собственная дельта. Остальные пропуски зоны010 не считать автоматически исправленными.
Last Completed (2026-10-28): AC25 PID47840, VS Run, canonical загруженный APX057e7a21… не изменился при F5. По зонам004/006/010/009-колонна/009-уклоны подготовлено и реально создано7/5/1/10/11нижних (всего34), каждый GUID найден в native added-set, draw_error0,3D bbox прочитан. Всех Beam144→195, added51/removed0; это НЕ полнота стыков. C3B96249…: шесть наклонных Floor faces valid=false и шесть свежих сообщений Skirting skipped; source gate Data8831–8834. Полное native дерево не содержит some_stuff_plintus_down/up; у34новых и11существующих Beam-оснований только Автокласс, оба GUID-свойства notAvailable. Эти11Beam снова попали в подготовленные стены и дали11нижних отрезков (2/2/0/3/4). В зоне с уклонами4отрезка короче2мм. Создание не останавливается при отсутствии ролевого класса (Data8403–8411;Draw27–35,1740–1748). Обобщённая гипотеза «пол ниже стен» НЕ подтверждена для всех пропусков; в зоне010 причина единственного результата ещё не установлена.
Validation прежней диагностики: Verified — реальные VS-снимки preparation/writers, native GUID/classification/property/bbox read-back и scoped source до исправлений. Тогда canonical build не выполнялся; один RoomBook не доказал полноту геометрии/повтора/Undo. Указанный PID47840 больше не работает. Предыдущие точки Propertycache995 и Draw1730 disabled сохранены.
Current Work (2026-10-28): Data OtdBeam_ProcessFinishes учитывает толщину по нормали для подготовленного Mesh→Roof; Roombook_CheckSkirtingConfiguration проверяет используемые ролевые классы и определения ownership до OtdRooms_Draw. Повторены реальные SDK25 standalone-стенды: slope 29/0, preflight 44/0; границы — извлечённые функции и контролируемые зависимости, не Archicad. Текущий Data SHA 633c1a378f5bfffbcb74a1343f073c0ed7936c4983ed1f8f7be9ed6e05024e90 совпадает с прогонами. Нативная конфигурация не настроена, геометрия и повтор не приняты. Evidence: scratch/lower-skirting-fix-270/ и scratch/skirting-preflight-270/.
Validation текущего состояния: VS Debug x64 build SomeStuff успешно перекомпилировал Data, Roombook и SkirtingRegression и перелинковал APX f1d7b62fd62c5494d946ce6c073f5b9c883bf49834323fa1735fae66757b0aa7, warning SDK LNK4099. Это VS build, не canonical BuildAddOn. AC25 PID52764 загрузил APX из Debug и открыл test_25, порт19723; по SHA оба PLN и Data после запуска не изменились. RoomBook после правки не вызывался. До запуска свободный lock сохранён как C:/Users/DA-ROG~1/AppData/Local/Temp/skirting270-lock-1793212505907380900.lck; не удалён. Существующий Python setup checker использует устаревшие имена ролей и не подтверждает настройки плинтусов.
Current model read-back: пользователь добавил два класса в систему ПК1; JSON подтвердил ID «АР/Плинтус» с описанием some_stuff_fin_plintus_down и «АР/Плинтус (1)» с some_stuff_fin_plintus_up. Текущий код Data:47–48/8550–8565 ищет some_stuff_plintus_down/up без fin_, поэтому оба новых класса пока не распознаются. Свойства группы «АР/Отделка элементов» «GUID зоны» и «GUID базовой конструкции» имеют нужные маркеры Sync_GUID; тип и availability в JSON не представлены. Проверка scratch/lower-skirting-270/current_config.py и отдельный targeted read-back. Модель агентом не изменялась.
Review: независимый read-only reviewer: No issues found in inspected scope; preflight расположен до удаления/создания отделки, но не до существующего переключения комбинации слоёв. Родитель подтвердил расположение LayerCombination_ApplyFinishIfExists и preflight по текущему Roombook.cpp. clang-format dry-run и git diff --check для четырёх файлов exit0; свежий LSP остаётся непроверенным.
Update: пользователь изменил сохранённый RoombookData.cpp (SHA bec80194ab1abbdcebed791fea8a197c5f7928ad2470a603c160db204daf3d44): строки 47–48 теперь содержат some_stuff_fin_plintus_down/up. VS всё ещё исполняет предыдущий APX. Пользователь попросил перезапустить; перезапуск требует остановки Archicad и может потерять классы, добавленные в несохранённую модель. Сохранение PLN ранее запрещено; до решения владельца процесс не останавливать.
Restart: F5 VS Debug x64 21:48 собрал RoombookData.cpp и перелинковал SomeStuff.apx SHA 33e96487affe84cca2da8f4366faec0916a2557e4ac292a43423a5b14b3f16f6; VS build 2 успешно, 0 ошибок. PID37868 открыл test_25, порт19723 принадлежит ему, загруженный модуль — Build/SomeStuff/25/Debug/SomeStuff.apx. Read-back после запуска подтвердил оба класса с some_stuff_fin_plintus_down/up; доступность свойств ещё не проверена. После F5 Data SHA bec80194ab1abbdcebed791fea8a197c5f7928ad2470a603c160db204daf3d44, PLN build fd3b30f39594c1c3eae12f414d88cff2d1e66f0b796bf9492f6022a856ac6ec2 и исходный 882d8de2bf80366380b65c355f27a0030649ea2dcd664c9f4423ab8d6099d5d6 неизменны относительно состояния перед F5. RoomBook не запущен.
Native update (2026-10-28): APX33e96487…/Data bec80194…; run1 lower7/5/1/10/12 + upper4/3/3/4/3 =52новых Beam, run2 lower5/3/1/7/8 + те же17upper =41новый Beam. Все52run1 удалены приrun2; ни одна из144исходных Beam не удалена. Для41run2 exact added-set совпал с writer GUID,82ownership normal и верные, классы по ролям верные, bbox всех41 прочитан. Исчезнувшие11prepared walls основаны на старых Beam; сами11Beam по-прежнему только Автокласс и оба ownership notAvailable — причина исчезновения из подготовки не установлена. Object baseline62→after1 62→after2 51, removed baseline objects26: не заявлять неизменность остальных ролей. Полные finite Floor polygons без дуг, стены и проёмы сняты в run2-zone{1..5}-geometry.json. Независимый первичный reviewer нового source-дефекта не доказал; новый oracle по полным контурам делегирован. Evidence: Hermes scratch/skirting-native-current/. Своих активных точек послеrun2 нет; исходные Propertycache995/Draw1730 disabled сохранены.
Repeat update: run3 завершён. run2→run3:41Beam удалён/41создан, survivors0, исходные144Beam все сохранены. Во всех5зонах counts, owners, participant indices, оси и independently прочитанные native3D bbox совпали без residual. Для41run3 оба ownership normal и ожидаемые, классы верные, added-set exact. Другие роли сохраняют итоговые количества, но Slab22/Roof8/Object15 пересозданы — не no-op. Source/APX/оба сохранённых PLN SHA неизменны отbefore доafter3. Своя Draw1830 снята, VS Run. Evidence: compare-repeat.py/repeat2-3.json и run3-readback-summary.json. Это доказательство повтора послеrun2, не полноты контактов. Два участка около1мм в зоне уклонов повторяются; без геометрического основания не удалять их порогом.
Oracle update: deleg_025550e1 завершён; родитель прочитал полный stdlib-only lower_contact_oracle.py и6selftests, самостоятельно повторил оба запуска:6/0 и все5зон без missing/extra/offPlane/unmatchedProduced при1e-6м. Lower5/3/1/7/8; длины14.920000/12.100000/0.821690/14.992154/10.936077м. Проверены все finite пары eligible prepared walls/floors с проёмами, rise и толщиной; граница — подготовленные поверхности, не полнота исходной подготовки/native body/profile. Предположения oracle: writers enabled и Favorite name непустое там, где влияет на выбор маршрута. Наклонный новый контакт зоны5 и два≈1мм участка воспроизведены независимо.
Native participants update:42уникальных созданных wall/floor/ceil GUID прочитаны отдельно; для30из41оси выходят поZ за bbox соответствующего Object-wall: API.Get3DBoundingBoxes возвращает zMin=zMax=0 для [gjs]-МИ раскладка - стены_22_1_2. General_LibraryPartName normal, ModelView_LibraryPartMissing=false, General_Height положительная. Это расхождение каналов/геометрии, НЕ доказанный баг writer или библиотеки; семантика bbox именно для этих GDL и фактические 3D-тела не подтверждены. Полы/потолки такого bbox-расхождения не дали. Не заменять избранное/тип и не подавлять30плинтусов по этому checker. Evidence: run3-participant-contact-report.json и participant-info-values.
Next Step: получить от владельца зону/стену или скриншот конкретного отсутствующего плинтуса в текущем результате; проследить именно этот исходный контакт до подготовки и native-представления. Исчезновение11старых Beam-кандидатов run1→run2 не объяснено. Свежий LSP и полная native геометрия ещё не приняты; новых C++-правок в этой диагностике нет. BuildAddOn.py не запускать: он перезаписывает текущий Build/SomeStuff/25/test_25.pln даже при ошибке. Не подменять fixture и не удалять старые неидентифицированные Beam; без Save/commit/push.
Last Checkpoint: нового нет; HEAD6f131e1, commit/push/закрытие не выполнялись. Issue270 OPEN.

#### #293 — исключение отделочных балок из beamParts

Scope: AC25 Windows; только Class_IsElementFinClass в начале beamParts в OtdRoom_BuildSurfaceFinishes и передача context.finclassguids от единственного caller. Геометрия, writers, PLN, Docs/generated и чужие изменения вне дельты. Commit/push не выполнять. Issue: #293 (kuvbur/AddOn_SomeStuff).
Status: WAITING_FOR_BUILD.
Plan: [x] source/стеновой образец/LightRAG и issue; [x] минимальная проверка; [x] format/адресная регрессия/locality/source-review и свежий LSP в указанном объёме; [ ] canonical build/native после разрешения владельца.
Last Completed: guard Class_IsElementFinClass до source lookup, тот же cache и существующая семантика ошибок, как у стен. Только три изменения: signature/guard/caller; остаток Data совпал с snapshot. MSVC C++14 SDK25: извлечённый production loop-prefix + полный Class_IsElementFinClass при контролируемом Class_ReadItems, RED4pass/6fail → GREEN10pass/0fail (compile/run exit0). Оба класса плинтуса, другой finish, обычный/пустой/ошибочный classification read, смешанный порядок/повторные parts и пустой finish-set проверены. Не полное тело builder и не host. clang-format/dry-run и scoped diff-check PASS. Fresh clangd: три прежних unused-variable, новых source ошибок нет; внутренний ExtractFunction отказ, exit3, полный GREEN LSP не заявлять. Review: No issues found in inspected scope (собственное source-only). Evidence: Hermes scratch/beam-finish-class/{before.cpp,own.diff,run.py,result.json,lsp.log,review.txt}. Memory: lightrag/SKILL.md дополнен независимым фильтром beamParts.
Next Step: canonical AC25 build и native-проверка повтора по разрешению владельца; действующий запрет сборки/перезапуска сохранён. Docs/generated не обновлялись согласно scope.
Last Checkpoint: нового нет; commit/push не разрешены, issue #293 OPEN.

#### #260 — общая проверка geom перед линейным построением

Scope: AC25 Windows; только RoombookDraw.cpp: общий чистый validator OtdLinearElement и его применение в Wall_Draw, Beam_Draw, Object_Draw_Linear и существующем guard OtdWall_Draw. Координаты/плановая длина конечны, длина >= min_dim; заданная высота конечна и >= min_dim, нулевая допустима только для Beam (сохранить избранное). Проверять итоговую story-relative отметку до SDK-чтений; геометрию/избранное не подменять. PLN, остальные writers, Helpers/Data/header, Docs/generated и commit/push вне scope.
Status: WAITING_FOR_BUILD.
Plan: [x] контракты/issue/SDK context; [x] общий validator и четыре потребителя; [x] MSVC C++14 SDK25 baseline/RED425/243 → GREEN425/0 и прежний Beam132/0; [x] format/diff-check/fresh LSP/locality/self-review; [x] статус и процедурная память; [ ] canonical AC25 build/native после разрешения владельца.
Last Completed: static OtdLinearElement_CanDraw проверяет обе XYZ, конечную плановую длину >= min_dim, конечный dz и height policy. Wall/Object требуют высоту; Beam допускает0 без изменения сечения Favorite. В OtdWall_Draw общий guard больше не запрещает нулевую высоту Beam. Итоговый story-relative level проверяется до чтения настроек каждого writer; исходные способы вычисления отметок сохранены. Все12 позитивных trace rows create/update/reversed равны baseline; NaN/±Inf, вырожденные/слишком короткие оси, переполнение разностей, невалидная отметка этажа/высота, min_dim и optional0 пройдены. Полные реальные bodies+SDK25, native/read/GDL/classification/opening/link boundaries контролируемы: это standalone, не host. Scratch-only снятие object guard: compile0/run1,114 failures. Другие roombook snapshots неизменны; в Draw изменены только helper и четыре согласованные функции. Fresh CLI AST сохранил пять прежних unused-variable, новых diagnostics нет; check-lines=1-1, не полный all-lines tweak audit. MCP0 отдельно, не доказательство freshness. Review: No issues found in inspected scope (собственное source-only, не независимое). Evidence: Hermes scratch/linear-geom-validation/{run.py,checks.cpp.txt,red-result.json,green-result.json,mutation-result.json,beam-regression/,own.diff,validation.json,verify.py,*-lsp.log}. Memory: cpp-code-refactoring/SKILL.md дополнен общим geom-validator и height policy.
Next Step: canonical AC25 build/native после отдельного разрешения; открытый ARCHICAD PID26892 и его проект не тронуты. Полная сборка/перезапуск не выполнялись по действующему ограничению. Docs/generated остаются вне разрешённой дельты и не обновлены.
Last Checkpoint: нового нет; commit/push не разрешены, issue #260 OPEN.

#### #260 — Beam_Draw: общий GetOffsetFromStory

Scope: AC25 Windows; заменить только ручной поиск этажа в Beam_Draw общим трёхаргументным GetOffsetFromStory. Сохранить Z начала наклонной балки; общий helper оставляет исходную Z при отсутствии этажа. CommonFunction, остальные writers, PLN, Docs/generated и commit/push вне дельты.
Status: WAITING_FOR_BUILD.
Plan: [x] сверены оба overload и точное тело writer; [x] ручной цикл заменён; [x] format/diff-check и SDK25 C++14 regression 132 checks/0 failures; [x] свежая CLI AST-дельта без новых diagnostics (те же пять unused-variable; MCP 0 не подменяет свежую CLI); [ ] canonical build/native после разрешения владельца.
Last Completed: единственная замена в Beam_Draw: GetOffsetFromStory(geom.seg.begC.z, geom.floorInd, storyLevels). Полный remainder файла побайтно эквивалентен baseline после нормализации line endings. Исполнено точное production-тело writer и общего helper с SDK25; readers/native Create/Change/Dispose контролируемы. RED132/2 → GREEN132/0; наклон/разворот, create/update, fallback пустого/отсутствующего этажа проверены standalone, не в Archicad. Причина: дублирование существующего project helper; перегрузка с geom использует среднюю Z и для начала балки не подходит. Evidence: Hermes scratch/beam-story-helper/{own.diff,red-result.json,green-result.json,validation.json,*-lsp.log}.
Next Step: canonical AC25 build/native после отдельного разрешения владельца; ARCHICAD PID26892 остаётся открытым, сборка и перезапуск не выполнялись. Docs/generated вне текущего scope и не обновлены.
Last Checkpoint: нового нет; commit/push не разрешены. Issue #260 OPEN прочитана, отдельная issue не требуется.


#### #252 — дубли отделки за один запуск, 2026-10-25

Scope: AC25 Windows; только разделение окончательных стен и служебных legacy-владельцев проёмов в OtdWall_ProcessFinishes. Не менять геометрию/материальные правила, writers, поиск прежней отделки, исходные PLN, чужие правки; commit/push/закрытие и Docs/generated вне текущей дельты.
Status: PARTIAL_NATIVE. Блокер #291 устранён одной строкой и проверен на AC25; новые стены создаются, повтор не накапливает совпадающую отделку по typed bbox/property/link inventory. Полная solid/contact-приёмка #252 и остальные skips/плинтуса остаются открытыми. Итог #291 — в Archive.
Plan: [x] source-диагностика и независимая сверка; [x] SDK25 RED16/5 → GREEN16/0 на полном production-теле; [x] минимальная правка; [x] format/fresh LSP/canonical AC25 build; [x] независимое source review; [/] native-приёмка на согласованной модели.
Last Completed: finishWallCount отделяет окончательные стены от legacy-хвоста. Основной Delim проходит только префикс; откосы/расширение пола выполняются по legacy, при пустом legacy сохранён прежний final-only маршрут. Param_SetToWindows остаётся на обеих копиях; core-offset источники не удалены. Parent MSVC SDK25 compile/run: прежние7/0 с побайтно тем же ordered output; итог16/0, раньше5fail. Весь Data вне этой функции побайтно совпал с dirty-baseline. Canonical AC25 Debug compile/link exit0, APX e20979dc4ba2ea924aec3de00e523fd5bfa75ba1922b6cac3ee3806f97ec7bb7; source ba8d500005b8be1b0cdc23ba98218f0827f0cab3f88e0e3ee6c9fa9b66230226. Fresh CLI: те же3 unused-variable, новых source diagnostics нет; exit3 из-за внутренних ExtractFunction probes (13→14 после нового continue), не полностью GREEN LSP. Format/scoped diff-check PASS. Это не host-тест, не полная геометрическая/повторная приёмка #252.
Native Result (до исправления #291): PID6268, загружен canonical APX e20979dc…; пять production source hashes и APX неизменны через F5/все прогоны. Первый/второй RoomBook: total124/124, created27/22, во втором removed22; из первого created сохранились8 (3Object+5Beam, все Reveal), их типы/классы/связи проверены. Нормализованный полный inventory (тип+класс+3D bbox+выбранные свойства/связи) первого/второго совпал; совпадающих 3D bbox у классифицированной отделки0, невалидных owner/zone ссылок0. Но новых Wall в первых двух прогонах0; каждый прогон сообщает20 OtdWall_Draw и61 OtdBeam_Draw ошибок. Это НЕ native PASS. Диагностический третий прогон: total124, Wall40 вместо46, Object40 вместо34, added56/removed56; поэтому стабильность всего native-выхода не принята. Полная solid-геометрия/независимая ожидаемая полнота не проверены.
Diagnosis (до исправления #291; исправление и новые прогоны позднее записаны в Archive): breakpoint RoombookDraw.cpp:1607 реально остановил Wall_Main: draw_type=API_SlabID, favorite.type=API_SlabID, draw_error=-2130313112, height0.099999924242648586, конечный отрезок около2.01м. Header-migration удалила прежний OtdWall draw_type=API_WallID, общий OtdDrawData:372 оставляет API_SlabID; неподдерживаемый тип отклоняется dispatch:1565–1578. Отдельный bug #291 создан и прочитан обратно; исправление вне текущего scope не выполнялось. Полная причина всех Beam/skips отдельно not verified.
Next Step: по отдельной команде независимая полная геометрическая приёмка #252 и диагностика оставшихся surface skips/плинтусов. #291 исправлен; новые17Wall и повтор136→136, owner/zone ссылки и typed bbox/property inventory проверены, полная solid-геометрия not verified. Исторические сведения проверки до исправления #291: Своя точка1607 удалена, исходные Propertycache995/Draw1644 enabled восстановлены и сверены fresh list; VS Run, тестовая модель оставлена открытой. Команды Save не отправлялись, но build-copy PLN на диске изменился в23:02:22; причина not verified, изменённая копия сохранена native/observed-build-copy.pln и самовольно не откатывалась. Оригинальный Test_file/test_25.pln hash неизменен. Комментарий #252/6402405282 обновлён статусом блокировки. Memory: visualstudio-cpp-debugger/references/archicad-somestuff.md дополнен per-owner defaults/границами bbox-проверки/контролем двух PLN. Evidence: scratch/single-run-duplicates/native/{before-launch.json,before.json,after1.json,after2.json,after-diagnostic.json,repeat-analysis.json,debugger-observations.json,debug-log-slices.json,*-debug.txt}.
Last Checkpoint: нового нет; commit/push/закрытие не выполнялись. Evidence: Hermes scratch/single-run-duplicates/{before-manifest.json,parent-locality.json,own.diff,red/,green/,baseline/,final/,validation.json}. Docs/generated остаются вне текущего разрешённого scope.

#### #260 — структура отрисовки перекрытий и плинтусов, 2026-10-25

Scope: AC25 Windows. По текущему сохранённому образцу стен разделить dispatch/связи и native writers перекрытий/плинтусов; длины перенести в расчёты. Владелец отдельно разрешил необходимые исправления остаточных ошибок общих helpers/стеновых writers, без изменения их структуры. Геометрическое ядро контактов, Mesh, исходные PLN, чужие правки, commit/push вне scope. Issue: https://github.com/kuvbur/AddOn_SomeStuff/issues/260.
Status: IN_PROGRESS.
Plan: [x] прочитан актуальный сохранённый образец; [x] универсальный Beam_Draw без Prepare применён/SDK25 standalone PASS, независимое source-only ревью без замечаний; [x] сверка разделения OtdSlab dispatch/writers; [ ] длины в расчётах — место хранения требует согласования после удаления владельцем length_beam_floor/ceil; [/] format/fresh LSP/адресные проверки PASS, canonical AC25 build заблокирована VS Run.
Last Completed: удалены SkirtingBeam_Prepare, объявление и три устаревших прямых тестовых блока. В Beam_Draw нет проверки роли отделки; исправлены create/update, отметка начала относительно этажа, прямой/наклонный отрезок и memo/mask обновления. Нулевые размеры/материал оставляют настройки шаблона. Live-тело writer собрано MSVC C++14 с SDK25 и выполнено с контролируемыми readers/Create/Change/Dispose: 129 checks/0 failures; scratch-only negative control неверной отметки дал ожидаемые9 failures. clang-format/dry-run и scoped git diff --check PASS; fresh clangd Draw/Data/tests0 diagnostics, header1 unused-includes warning. Это НЕ host-приёмка. Evidence: Hermes scratch/draw-universal/{before,own.diff,run.py,green-result.json,mutation-result.json}. Quantities/coordinator не изменены относительно нового baseline.
Review: No issues found in inspected scope. Родитель сверил hashes всех3 reviewed production files с live и результатом standalone-прогона; весь Draw вне Beam_Draw совпадает с baseline. Evidence: scratch/draw-universal/{review.md,parent-review-check.json}; review SHA2569c759a35bec067b35c49e9e9b9c203c9a3760a8830b2fd3aeef9ac41752989f0. Memory: обновлён cpp-code-refactoring/SKILL.md — универсальные writers без role-specific Prepare, тестирование реального writer, повторная проверка модели данных после правок владельца.
Next Step: после окончания чужой отладки выполнить canonical AC25 build. По расчёту длин требуется выбрать результат: ранее добавленные length_beam_floor/ceil владелец удалил, helper отключил; clarify истёк без ответа, возвращать поля самовольно нельзя. Native runtime не выполнялся, отладка и модель не тронуты.
Last Checkpoint: нового нет; commit/push не разрешены.

#### Python GUI — исправления после ревью

Scope: только `python_scripts/roombook_setup.py` и его Readme; AC25 JSON, C++ и модель не изменять. По команде владельца — без дальнейшей работы с issue, без commit/push.
Status: WAITING_FOR_ACCEPTANCE. Исправлены вложенный addOnCommandResponse, видимость кнопок, выбранный порт по умолчанию, неполный результат (CLI 3), отображение ошибок worker и блокировка повторных задач.
Plan: [x] исправления; [x] GUI/CLI регрессии на контролируемых ответах с настоящим tkinter mainloop; [ ] повторный live-прогон после запуска Archicad.
Last Completed: синтаксис, размеры окна 780/640, refresh/reentry, выбор порта, ошибки обоих worker, пустой результат, неполная проверка и приоритет CLI-кодов проверены. Health разобран на точном ранее полученном ответе; Favorites — только fixture. При новом live-вызове порт 19723 отказал в соединении, live после правок не подтверждён.
EXE: по команде владельца собран `python_scripts/dist/roombook_setup.exe`, onefile/windowed Windows x64, Python3.10.11 + PyInstaller6.22.3 в изолированном scratch-venv; исходник не менялся. SHA256: 5a0068c5166adccd90b772d2390082e013b87de583b96c2ea7f331f5b5d2ce7a. Запуск EXE подтверждён видимым окном «Проверка отделки (Roombook) v1.0» и путём процесса; окно оставлено открытым. Работа с Archicad из EXE не проверена.
Next Step: при работающем Archicad проверить обновление списка с именем и версией, затем отчёт выбранного проекта.
Last Checkpoint JSON: 9a668eb — регистрация Health и projectName; только собственные добавления в HealthCommand.cpp/JsonCommandRegistrar.cpp, без удаления диагностики и без RoomBookCommand.cpp. Версия VersionId существовала в HEAD. Состав коммита и оставшиеся изменения проверены; новой сборки/runtime при коммите нет, ранее AC25 build и живой Health подтверждены в сессии. Push не выполнялся.
Previous Python Checkpoint: 981d11f — по команде владельца закоммичены только `python_scripts/roombook_setup.py` и `Readme.md`; чужой staged C++ сохранён. EXE, прежний build_exe.bat (ссылается на отсутствующий main.spec), nul и общая IDEA.md в коммит не включены. Push не выполнялся.

#### Last Checkpoint

0795e2d [C1-baseline] Сохранить текущие изменения Roombook перед рефакторингом; текущие дельты C1–C9 не закоммичены.
Предыдущий checkpoint: b55d12f [278] IDEA.md: состояние задачи отписки наблюдателей Refs: #278
(код отписки: 0dd7ba3; правки вылетов Undo.dll: 9d68f16 [#277]; чистка мёртвого кода Roombook: be55b78).

#### Plan

- [ ] C1–C9: продолжить план «Читаемость и минимальная структура» параллельно функциональным участкам; открытые шаги перечислены выше. Очистка и переносы выполнены в объёме C4/C5/C6/C7a/C7b/C8/C9, build/host-приёмка отложена.
- [ ] #252 BLOCKED_INPUT: Unsupported beam geometry 51037AF1-1476-4A3A-BB18-4201022519C0 — live чтение вернуло headerError-2130313115, snapshot отсутствует; новый RoomBook на текущем проекте ветку не достиг. Нужна модель/запуск с этой балкой или актуальный отказ; предикаты без RED не ослаблять.
- [/] Z9 #252 diagnostic: минимальный read-only RoomGeometryDiagnostic по optional favoriteName; прямой ACAPI_Favorite_Get с memo, экспорт типа/library identity и scalar numeric GDL params. Без изменения модели/defaults/fallback/UnhideUnlock; source/format/LSP проверять отдельно, build/runtime — родитель.
- [ ] По отдельному решению владельца исследовать точное создание двух целых диагональных контактов в зоне41BE148A и отсутствующего отделочного проёма4F6A150D; прямоугольный writer их отвергает без приближённого заполнения.
- [/] Завершить Z9 по команде владельца «Закончи починку построения откосов»: отдельные openingWalls передаются только в подготовку откосов, surface-вырезы не создают повторных откосов. Direction GREEN; остаётся классифицировать 5 строк без верхнего контакта, нестандартную глубину и параметрические/материальные пробелы. #252 OPEN.
- [ ] Три торцевых участка многослойных стен: определить контактные слои и назначение материала без произвольного base_flipped; полный охват стен не достигнут.
- [/] Пункт 2: все 28 in-scope пропусков классифицированы; четыре трапеции 2.375999498 м² требуют согласованного расширения прежнего wall writer; две микрополосы ниже неизменного порога. Неизвестным основаниям GUID не назначать; полное закрытие пункта2 не заявляется.
- [ ] Остатки SDK-error/deleted/multiple-body и остальных неподдержанных геометрий/избранного — отдельным этапом.

#### Decisions
- Issue #315 — https://github.com/kuvbur/AddOn_SomeStuff/issues/315 — OPEN: отметка и высота этажа по индексу: Sync_from{Story:level; index}, Sync_from{Story:height; index}. Постановка создана и прочитана обратно; следующий шаг — реализация по отдельному запросу. Last Checkpoint: нет, commit не запрошен.
- Issue #312 — Синхронизация: добавить вывод типа слоя (ядро/отделка/другое) в Sync_from{Material:Layers} по плейсхолдеру %structype% (2026-11-05).
- Issues #311 — Обработка размеров: подсветка непривязанных элементов размерной цепочки цветом (2026-11-05).

- #285 https://github.com/kuvbur/AddOn_SomeStuff/issues/285 — OPEN. Расширение контура перекрытия пола в дверные проёмы на глубину откоса (base_reveal_width). Проёмы берутся из отделочных стен (otdw.openings) с отметкой низа <= отметки низа зоны (op.zBottom <= otd.zBottom + Tolerance). Расширяется то перекрытие пола (OtdSlab с type == Floor), которое примыкает к проёму, без создания отдельного участка, независимо от флага создания откосов (op.create_reveal).

- #284 https://github.com/kuvbur/AddOn_SomeStuff/issues/284 — OPEN. Правила отделки балок: отделка верха при нахождении в зоне; если верх балки ниже потолка — отделка всех граней балки (верх, бока, низ) учитывается как отделка стен (тип Wall_Main, материал om_main, some_stuff_fin_walls); если верх балки выше зоны / в потолке — бок и низ отделываются как потолок (тип Ceil, материал om_ceil, some_stuff_fin_ceils), верх не отделывается.


- #271 https://github.com/kuvbur/AddOn_SomeStuff/issues/271 — OPEN, тело/метки прочитаны обратно; поиск Mesh под стенами и многослойный наклонный пол, Roof рекомендована для прототипа. Issue создана без production-правок/build/runtime; следующий шаг — прототип по отдельной команде. 2026-10-13 Mesh→наклонный пол, AC25 Windows: владелец согласовал несколько элементов, по одному плоскому элементу на плоскость сетки, и предложил рассмотреть Roof вместо Shell. SDK25 подтверждает coordinate search для API_MeshID, composite Shell/Roof и API_PlaneRoofID с XY-полигоном, angle/baseLine/posSign. Рекомендован прототип односкатной Roof как более прямое представление плоской грани; окончательная замена Shell и production-реализация ещё не выполнялись. Native Mesh reader, нижняя контактная поверхность Roof/порядок слоёв/стыки/отверстия/учёт и повтор пока not verified; build/runtime не выполнялись. Next Step: native-прототип односкатной многослойной Roof по реальной mesh-грани после согласования запуска; issue до production-правок. Подробности: skill lightrag/references/somestuff-mesh-shell-ac25.md. Production/модель/активные задачи/Scope сохранены; нового checkpoint/commit/push нет.


- Native AC25 Inside/flipped=false строит тело слева от линии. Адаптер ориентирует левый бок внутрь зоны; slantAlpha=atan2(1,-dot(riseXY,rightXY)). Обе семантики подтверждены полным чтением созданных тел, а не численным тестом угла.

- Точечный допуск 1e-6 м не менялся; проверка площади представимого параллелограмма учитывает погрешность точек вдоль периметра. Исправлен нестабильный отказ одной грани при повторе 51→52; финальные запуски оба 52.


- Избранное и правила назначения материала сохранены; толщина slab-отделки может быть толщиной избранного (в фикстуре 0.131 м), не обязательно 1 мм. Нового writer/Morph/GDL нет.

- Переиспользуемые геометрические уроки: skill lightrag, references/somestuff-surface-finish-ac25.md. Scoped Docs/generated обновлены перед текущим checkpoint по явной команде владельца.


- Плинтусы оформлены в #270: https://github.com/kuvbur/AddOn_SomeStuff/issues/270 (OPEN, тело/метки прочитаны обратно). Краткое описание обоих видов, реальные стыки только созданной отделки, схема Draw_Elements → ReadRoomSurfaces → Polygon2D::CutElem, погонные количества и проверки; AC25 Windows. Дубли по «плинтус», «skirting», «карниз», «стык отделка» не найдены. Issue создана до реализации; следующее действие по отдельной команде — прототип контактов и выбор типа/избранного плинтуса. C++/модель/build/runtime не затронуты, checkpoint/commit/push нет.

- Расширение требования плинтуса: создавать также потолочный плинтус только по реальному стыку нижней обращённой в помещение поверхности СОЗДАННОГО аддоном потолка с созданной отделочной стеной. Нет потолка в зоне/на участке либо нет 3D-контакта — нет потолочного плинтуса; верх зоны/стены не fallback. Напольный и потолочный маршруты используют общий алгоритм конечных контактов с отверстиями, но отдельные результаты/погонные количества и настройки. Scope текущего обсуждения — исследование, реализация не начата; следующее действие — read-only прототип обоих видов контактов, build/runtime not verified. Активная задача и checkpoint не изменены.

- Исследование плинтуса AC25 Windows: окончательное требование владельца — только реальный стык СОЗДАННОГО аддоном пола с созданной отделочной стеной; нет пола в зоне/на участке или нет 3D-контакта — нет плинтуса, без fallback на контур зоны/низ стены. XY отсекать Polygon2D::CutElem(Sector, Boundary_Closed) с отверстиями; Z определяется общим 3D-стыком, не произвольным OtdWall.zBottom. SDK25 CorePolygon2D.hpp:3011 подтверждает InsideOrOn; :3406 IntersectElemBorderOnly ищет совпадающие граничные части, не внутренние линии. Лучшее место окончательного построения — отдельный этап Draw_Elements после успешного создания полов, стен и проёмов в существующей undo-команде; предварительный расчёт после ProcessWallFinishes возможен, но не доказывает создание/контакт. Погонную длину не смешивать с площадями. Подробности: C:/Users/da-rogojin/AppData/Local/hermes/cache/scratch/skirting-polygon2d-ac25-research.md; reusable reference в skill lightrag. Scope — исследование, C++/модель не менялись; build/runtime/скорость not verified. Следующее действие по отдельной команде — read-only прототип реальных стыков созданных элементов, затем issue перед реализацией; активный Z9 и checkpoint18d05b7 не изменены, commit/push нет.

- #270, исследование 2026-10-20 завершено (AC25 Windows, без C++/модели/build/runtime): рекомендация — общий конечный 3D-стык с CutElem в плоскостях обоих участников, отдельные погонные количества и два вида плинтуса; первый writer предложен как профильная Beam из двух избранных, владельцем ещё не утверждён. Отчёт: C:/Users/da-rogojin/AppData/Local/hermes/cache/scratch/issue270-research/recommendation.ru.md. Учтены текущие #288, отдельный post-write cache, отсутствие статуса floor-writer, перезапись второго parent через base element и SyncArray после geometry undo; immediate native read внутри undo/профильные углы/exposed GDL faces — not verified. LightRAG после исходного curl56/7 восстановлен: health healthy и пять успешных запросов с Document Chunks. Next Step: утвердить writer/избранные, затем read-only native прототип обоих контактов и адресная transaction/placement-проверка на безопасной фикстуре. Last Checkpoint: 6e54fa2, нового нет; #270 OPEN, issue/commit/push не менялись. Остальные активные задачи сохранены.


#### #297 — GUID полов, потолков и плинтусов

Scope: AC25 Windows; final matching по стеновому образцу, surface Slab/Object, Mesh Roof и оба Beam-плинтуса. Разрешены standalone/TU проверки в scratch; без модели/PLN/canonical build/restart/commit/push.
Status: WAITING_FOR_TEST.
Plan: [x] dirty baseline, issue297 и SDK25 contracts; [x] однократный подбор surface Slab/Object/Roof/Beam, общий used и deletion cleanup; [x] Roof Change и оба Beam dispatch; [x] format, полная TU-компиляция, свежий LSP; [x] точные production assignment/dispatch/writer регрессии и negative controls; [x] source review, исправление stale flags и повторная валидация; [ ] docs/generated перед коммитом по команде владельца; [ ] canonical build/native после отдельного разрешения.
Last Completed: собственный C++ diff — RoombookDraw.cpp/TestFinishReuse.cpp; исходный dirty baseline сохранён в Hermes scratch/finish-reuse-297-resume/before. Assignment+collect-unused: RED 40 PASS/26 FAIL, GREEN 66 PASS/0 FAIL. Реальные dispatch/writers SDK25: 234 PASS/0 FAIL; negative controls stripping GUID и Roof Change→Create дают соответственно 66 и 6 ожидаемых FAIL при успешной компиляции. Дополнительный RED 4 stale-flag FAIL устранён четырьмя строками сброса success/created на входе Slab/Beam dispatch. После последней правки оба полных TU с TESTING компилируются MSVC C++14 SDK25 (object-results.json); свежий LSP: прежние 6 diagnostics Draw, 0 Test, новых нет. Scoped git diff --check 0. Child review сверён по хешам, его последующая дельта отдельно проверена родителем. final-verification.json фиксирует результаты, исходные хеши и неизменность обоих PLN/CDB. VS остаётся Run; canonical build/native не выполнялись.
Next Step: после безопасного завершения текущей VS-сессии получить отдельное разрешение на canonical AC25 build с защитой build-copy PLN и два прогона на согласованной тестовой копии с независимым inventory GUID/type/class/zone/base/geometry/holes/materials; отдельно проверить отказы/undo. Docs/generated пока отложены до команды на коммит согласно workflow, не считать актуализированными. Доказательства и runnable regression сохраняются в .hermes/reports/2026-10-29-finish-reuse-297/; issue #297 не закрывать без final runner/приёмки.
Decisions: GUID выбирается не по геометрии; JSON остаётся created-only. Ошибки обычных writers сохраняют прежнее продолжение, Roof error завершает undo-пакет; атомарность временных контейнеров не обещать. Roof только API_PlaneRoofID, leaf masks, новые vertexIDs=0 с прежним максимумом, edge memo reset по SDK25 sample. Старые Slab/Object/Beam настройки избранного не переопределяются новым способом. Общий converter не менять: отдельный подтверждённый дефект направления копирования дуг зарегистрирован и прочитан обратно в OPEN #298; дуги/native результат не объявлять проверенными.
Last Checkpoint: нет; чужие правки сохраняются.

#### Archive

##### #321 — неблокирующий отказ Roof при построении отделки (2026-11-07)

Scope: AC25 Windows; test_25.pln, Roof preflight/matcher/writers/deferred Mesh-cleanup, диагностика применимости многослойки. Без смены избранного/атрибута, веток/push/Save; #317 не менялся. Владелец разрешил коммит всех текущих изменений.
Status: DONE_WITH_LIMITS — исходный отказ воспроизведён, исправлен и проверен natively; дополнительный controlled stand blocked.
Plan: [x] 1. Живое воспроизведение; [x] 2. Подтвердить причину и безопасный дизайн; [x] 3. Минимальное исправление; [x] 4. Format/fresh LSP/canonical AC25 build/native/review/docs; [ ] Необязательный controlled stand — не исполнялся, two-failure guard.
Last Completed: изменённая владельцем зона1139C2FDFDA… воспроизвела Roof_Prepare APIERR_BADPARS до Create: composite153 flags2, без Roof4, избранное ар/пол_тип_1_smstf. Старый общий preflight давал rooms[]. Это input-data problem и отдельный implementation error общей блокировки. Теперь per-patch draw_error, failed Mesh не идёт в matcher/writer; runtime Roof error продолжает комнату; старый unused-остаток zone/base/role удаляется после writers только при успехе набора. Никаких ложных success/created/новых связей; конфигурация не меняется. Сводные количества остаются расчётными.
Validation: canonical Tools/BuildAddOn.py-c config.json-v25 Debug Windows exit0, clang-format/diff-check PASS. Source a24c88e8/APXfc7c51f2/build-PLN1a1b53b6 неизменны через F5 и оба native-прогона; repo-PLNb92a5a0c также сохранён. GREEN113:26rows/22success/4error, все4прежнихRoofGUID сохранены. GREENall:25зон/1172rows/1168success/4error только113,24другихзоны обработаны; все successful GUID прочитаны независимо. Debug pane подтвердил новую причину/избранное; payload clipped, полного TestKit verdict из него нет.
Review/LSP: No issues found in inspected scope; fresh version1 имеет те же5unused-variable+1unused-include, новых нет. Parent проверил manifest,16hashes и49диапазонов. Только Draw generated42→48,общий1068→1074,1026внешних записей/порядок неизменны; callgraph не пересобирался. Полная геометрия, произвольные native Create/Change failure/rollback, другие SDK/macOS not verified.
Additional stand: scratch-only SDK25/C++14 harness с точными production-функциями не запущен — два failed preparation cycles, последняя линковка LNK2019 ACAPI_Element_Tool/LNK1120. RED/GREEN и исполненные assertions НЕ заявляются. Повторов после guard не было; canonical/native доказательства независимы от стенда.
Next Step: владелец может разрешить применение composite153 для крыш и повторить113; commit всех текущих изменений разрешён; закрытие issue только отдельно по команде, перед закрытием final runner с защитой сохранённого build-PLN. Дополнительный стенд — только отдельный согласованный retry; текущая исходная задача не заблокирована им.
Last Checkpoint: текущий коммит [roombook-321] Неблокирующие ошибки построения крыш (#321), IDEA step4. Issue321OPEN; push/закрытие не запрошены, итог comment6705435769 прочитан обратно.
State: Archicad оставлен Run с изменениями только в памяти; Save не выполнялся, оба PLN неизменны, все8исходныхBP восстановлены, собственных нет. Evidence: Hermes scratch/roof-nonblocking/ (baseline/saved-fixtures, red113*, green113/all-acceptance, green-postrun-hashes, validation/parent-acceptance.json, regression/blocked-report.json). Урок сохранён в lightrag/references/somestuff-mesh-shell-ac25.md.

Commit validation: Перед коммитом владелец оставил только английский текст диагностики. Единственная дельта к native-снимку a24c88e8 — это сообщение; текущий Draw11bfe814 проверен повторным canonical AC25 Debug build exit0, APXaa2e7e14. Новый native-прогон не выполнялся, host закрыт; оба PLN восстановлены побайтово. 48 Draw-координат перенесены по точному diff и проверены source/name join, внешний generated-срез и порядок неизменны; fresh LSP выше относится к прежнему native-снимку.

##### Диагностика Ambiguous — зона A2100887-E07B-4DF1-9834-50A4D23FF992, 2026-11-06

Scope: AC25 Windows, запуск через VS и построение только целевой зоны по разрешению владельца; чтение sources неоднозначных участков. Без правок C++, Save, commit/push и изменения допусков.
Status: DONE — диагностика, не исправление алгоритма.
Plan: [x] 1. Baseline VS/исходников/PLN; [x] 2. F5 и живое воспроизведение; [x] 3. Sources/геометрия/read-back, удаление своих точек и восстановление прежних.
Last Completed: F5 действительно перекомпилировал RoombookData/SkirtingRegression и перелинковал Debug APX; canonical BuildAddOn не выполнялся. PID36960, test_25, loaded APX57fbd526…; три исходника и оба PLN по SHA256 неизменны. Свободный orphan lock сохранён в scratch после zero-process/exclusive-handle проверки. JSON остановлен после валидации ровно одной зоны (Command:97), затем в Ambiguous (Data:5507) и на emit (5727). Partition complete, 22 участка:9Matched/5Unmatched/8Ambiguous. ReadProcessMemory с проверенными в текущем binary offsets сохранил ВСЕ sources/contours/vertices.
Findings: четыре стеновых участка площадью около0.18183м² каждый (ширина55.1мм, высота3.3м), пары969E77C1…/9C8802CE… и969E77C1…/2C304250…; аналитические грани разных стен. Два совпадающих контакта балок3DEA1CF9…/E39BE854… площадями0.00922168054/0.00307241862м². Два микротреугольника стен/перекрытия51B8136E…:8.5589e-8/8.6285e-8м². Это не единый численный шум и не доказательство дублирования элементов. Flag ставится до исключений альтернативных маршрутов; ordinaryWalls8 и итоговый otdwall8 совпали по записанным geometry/owner/validity полям, все три конфликтующих стеновых владельца сохранены. На emit hasAmbiguous=true, hasSkipped=false, otdslab2.
Validation: предупреждение зоны115 записано в свежую панель Отладка. Команда завершилась, JSON41уникальный GUID, все41write_succeeded/was_created, draw_error0; независимый API.GetAllElements подтвердил наличие всех41 и всех6оснований, Get3DBoundingBoxes сохранён. Это existence/read-back, НЕ доказательство полноты/контуров/проёмов или корректности всех контактов. Модель изменена в памяти, Save не вызывался, PLN неизменны. Все3свои точки удалены, исходные9точек восстановлены с прежним disabled у Propertycache; Archicad оставлен Run. Evidence: Hermes scratch/ambiguous-contacts-20261106-202710 (partition-capture.json, prepared-walls.json, raw memory, response/read-back/debug-output, hashes).
Дополнительная проверка: все4стеновых Ambiguous-контура целиком входят в соответствующие подготовленные аналитические прямоугольники (плоскость/продольные пределы/z с допуском1e-6); доказательство wall-strip-prepared-coverage.json. Это покрытие подготовленной геометрией, не Boolean-сверка native тел. Все47bbox (41результат+6оснований) прочитаны без per-element errors. Финальные hashes равны snapshot после F5, включая APX и оба PLN; scoped diff-check PASS.
Next Step: отдельная адресная задача — отделить диагностику реально потерянной отделки от неоднозначности неиспользуемого partition; не скрывать beam/geometry проблемы без проверки покрытия. Production не менялся, исправление not verified.
Last Checkpoint: нет; commit не запрошен.

##### Удаление остатков TableRenderer / TablesNavigator — 2026-11-06

Scope: подключения в SomeStuff_Main.cpp и документация удалённого владельцем table-модуля; AC25 Windows. Остальной код, Archicad/PLN, commit/push вне scope. Без issue по прямому указанию владельца.
Status: DONE.
Plan: [x] 1. Baseline и поиск остатков; [x] 2. Удалить include, закомментированные подключения и устаревшие документы/индексы; [x] 3. Format, LSP, canonical build и проверка дельты.
Last Completed: убраны include и закомментированные подключения, четыре документа table, ссылки из обзоров/wiki; удалены 87 symbols и 10 callgraph edges. Родитель подтвердил точное равенство сохранённых 1061 symbols/1319 edges с baseline, их порядок и остальные поля callgraph. Непрокомментированные токены Main равны baseline за вычетом include; исполняемый код сохранён. Format/diff-check PASS; MCP clangd: 0 errors, 2 unused-include warnings. BuildAddOn.py -c config.json -v25 -r -l INT exit0: Main перекомпилирован, APX перелинкован. SHA Main после сборки совпал. Артефакты: Hermes scratch/table-cleanup-20261106, build-ac25.log/verification.json/source-check.json. Teamwork-ограничение сообщено владельцем, runtime не воспроизводился. Прочие унаследованные координаты docs не пересобирались.
Next Step: работа завершена; commit разрешён владельцем, push и native-проверка не запрошены. Archicad не перезапускался.
Last Checkpoint: [table-remove] Remove unused table renderer and navigator; Refs: IDEA.md step 3. Только файлы/фрагменты удаления table; без issue по указанию владельца.

##### SyncString — вынос распознавания источников

Scope: декомпозиция блоков распознавания `SyncString` в `Sources/AddOn/Sync.cpp`; AC25 Windows. Пользовательские `SyncStringFile`, `SyncStringAttribute` и реализация `SyncStringStory` сохранены; разбор Story, проверки применимости и общий разбор параметров не менялись этим выносом. Issue не создавался по прямому указанию владельца.
Status: COMPLETED.
Plan: [x] 1. Прочитать baseline, карточку Sync и зависимости; [x] 2. Вынести ID/GDL/Material/Formula/Morph/Coord/Property/Info/IFC/Glob/Class/Element/MEP/ListData; [x] 3. Проверить формат, адресную документацию, structural review/regression и собрать текущий AC25 coordinator.
Last Completed: вынесены 14 распознавателей. Тела веток и coordinator applied-снимка проверены отдельно: structural review — без замечаний; standalone differential regression — 7184 одинаковых случая и 44/0 адресных проверки helper-веток. Текущий coordinator после внешней перестановки сохранён как есть; свежая canonical сборка подтверждает его компиляцию, но не эквивалентность прежнему порядку.
Validation: `clang-format --dry-run -Werror Sources/AddOn/Sync.cpp` и `git diff --check` для файлов задачи — PASS. `python Tools/BuildAddOn.py -c config.json -v 25 -r -l INT` — exit 0: `Sync.cpp` перекомпилирован, APX перелинкован; только предупреждение `LNK4099` об отсутствующем SDK PDB. Native-приёмка, macOS и другие версии AC не проверялись. clangd MCP вернул противоречащий baseline нулевой набор диагностик и не использован как приёмка. Evidence: `C:/Users/da-rogojin/AppData/Local/hermes/cache/scratch/syncstring-extract-20261106/`.
Next Step: обязательных действий нет; runtime-приёмка порядка распознавания — отдельная задача.
Last Checkpoint: `[syncstring-extract] Extract SyncString recognizers`; Refs: IDEA.md step 3. Push не запрошен.

##### #315 — раздел команд этажей в русской вики

Scope: новый раздел в wiki/ru/Property-Commands-List-ru.md и запись состояния; C++, EN, сборка и runtime вне задачи. Версия AC для документационной правки не применима.
Status: COMPLETED.
Plan: [x] 1. Сверить текущие SyncStringStory/ReadStory и ключ storyCache; [x] 2. Описать level/height, номер этажа из интерфейса с учётом skipNullFloor и примеры; [x] 3. Проверить дельту, кодировку, ссылки и синтаксис.
Last Completed: добавлены 11 строк перед обработкой координат. Источники: Sync.cpp:1962–1992, Helpers.cpp:7384–7417, Propertycache.hpp:400–415 (ключ ui_index), CommonFunction.cpp:99–121. Автопроверка: удаление нового раздела даёт byte-identical исходную страницу; ссылки сохранены, UTF-8 без BOM/U+FFFD, CRLF; все четыре примера проходят проверку синтаксиса. git diff --check пройден. Сборка и runtime не выполнялись.
Next Step: документационная задача завершена; проверка выполнения команд и перевод EN — отдельно по запросу.
Last Checkpoint: [story-wiki] Document story level and height commands; Refs: IDEA.md step 3; Refs: #315. Коммит разрешён владельцем; push не запрошен.

- Комплект `wiki/llm/archicad-zone-finish`: скилл, JSON-справки и read-only поиск зон; проверены JSON-примеры, ссылки, mock-сценарии и поиск на AC25. Создание классификаций и избранного в runtime не проверялось. Checkpoint: нет, коммит не запрошен. Следующее действие: при отдельном разрешении проверить создание в тестовом проекте.

##### #313 — безопасный переводчик вики RU → EN

Scope: только wiki/tools/{translate_ru2eng.py,wiki_translation.py,test_translate_ru2eng.py,requirements.txt,README.md,.gitignore} и запись состояния. RU/EN страницы, legacy translation_hashes.json, C++, PLN и чужие дельты не менялись. Версия AC не применима; без платных API-запросов, push/IssueClose. Коммит разрешён владельцем.
Status: COMPLETED — реализация и офлайн-приёмка; live-перевод отдельно не проверен.
Plan: [x] baseline/issue; [x] RED9/9; [x] защищённый Markdown/ссылки и точный кэш переводов; [x] атомарная запись EN перед кэшем, CLI/lazy client/HTTP validation/retries; [x] GREEN46/0 и corpus20; [x] разобрать пять замечаний независимого review, проверить регрессии и документацию.
Last Completed: прежний CLI entrypoint делегирует wiki_translation.py. Сохраняются преамбула/код/технические маркеры/email/reference IDs, проверяется весь документ; новые картинки локализуются из raw-URL кэша без повторного перевода. Общий validator проверяет схему/контрольные суммы кэша и в --check. Новый translation_cache.v2.json не доверяет legacy-хэшам. Final unit46/0; py_compile трёх Python-файлов→scratch; все20RU реально прошли pipeline с identity callback и0callback на повторе, code blocks сохранены.40исходных RU/EN/legacy-cache файлов совпали с baselineSHA. CLI из scratch: dry-run exit0, обычный check stale20 exit1, check временного корпуса current20 exit0; всё без API/записей в repo wiki.
Review: leaf проверял старый SHA9dd8d6c1…; коллизии имён/reference links/вложенные картинки уже исправлены к возврату. Parent подтвердил остававшиеся email-local-part и неверный CURRENT при повреждённых blocks/checksum, добавил RED и исправил. Все5адресных регрессий повторены parent на принятом SHA c4b911c0c4dc3dae6a7627617804fb37b7987df92f3a3bc0029c61b699083e9e. Это разбор замечаний и собственная приёмка правок, не независимый clean-verdict финального snapshot. Evidence: Hermes scratch/wiki-translation-fix/{final-tests.log,review-red.log,email-red.log,review-acceptance.json,cli-report.json,final-manifest.json,corpus-final/corpus-report.json}.
Next Step: при отдельном согласовании — пробный платный перевод выбранной страницы и лингвистическая проверка. До этого live API/model quality not verified; HTML-блоки и shortcut reference IDs намеренно не переводятся. README описывает ограничения, стоимость первого перевода с новым кэшем и запрет одновременных процессов с одним кэшем. Issue313 открыт; итоговый комментарий6665478786 прочитан обратно и совпал с отправленным текстом. Процедура сохранена в somestuff-wiki-docs/references/translation-script-review.md.
Last Checkpoint: [wiki-translation], Refs: #313 (коммит по команде владельца); push/IssueClose не запрошены. Issue: https://github.com/kuvbur/AddOn_SomeStuff/issues/313.

##### Python README — RU/EN

Scope: python_scripts/Readme.md и эта запись; по уточнению владельца — краткое назначение и общая логика обеих EXE-утилит, без Python-команд, CLI/API-инструкций. Скрипты, EXE и проект Archicad не изменять; версия AC не применима к правке документа.
Status: COMPLETED.
Plan: [x] сверить оба исходника; [x] описать roombook_setup и somestuff_updater на русском и английском с запуском через EXE; [x] проверить соответствие коду и diff.
Last Completed: описаны проверка/настройка свойств проекта и обновление существующего аддона Release/Test с резервной копией; ограничения отделены от возможностей. Проверка структуры RU/EN, имён и наличия обоих EXE и отсутствие CLI-примеров — PASS; git diff --check PASS, diff скриптов/spec пуст. Сборка и runtime EXE не выполнялись; соответствие готовых EXE текущим исходникам не проверялось. Уточнение владельца передано OpenViking extraction.
Next Step: обязательных действий нет.
Last Checkpoint: нет; commit/push не запрошены.

##### Переводчик вики — read-only ревью (2026-11-05)

Scope: анализ wiki/tools/translate_ru2eng.py, текущих Markdown/image fixtures; сам скрипт, хэши и страницы вики не менять. Без API-запросов, установки зависимостей, commit/push; версия AC не применима.
Status: COMPLETED — ревью, не исправление.
Plan: [x] прочитать реализацию и примеры; [x] воспроизвести дефекты в scratch; [x] определить приоритеты улучшения.
Last Completed: 14 офлайн-сценариев на реальных FunctionDef скрипта с identity/failure переводчиком: 13 демонстрируют нежелательное поведение, 1 подтверждает сохранность старого EN при API-ошибке. Подтверждены преждевременный cache commit, потеря преамбулы/строк, разрезание fences, незначащие коллизии нормализации хэша, пропуск удаления хвоста, глобальная замена имени внутри inline code, необработанные heading URLs и потеря регистра PNG. Реальные Home.md/Home-en.md и report-en.PNG согласуются с замечаниями; авторство существующего EN не устанавливалось. Source SHA после проверок неизменен. Evidence: C:/Users/da-rogojin/AppData/Local/hermes/cache/scratch/wiki-translation-review/{probe.py,report.json}. Процедура сохранена в somestuff-wiki-docs/references/translation-script-review.md.
Next Step: после согласования отдельной реализации — issue/dedup, регрессии, сначала безопасная запись+кэш и Markdown/protected tokens, затем CLI/сетевые отказы. Live-провайдер и качество перевода not verified.
Last Checkpoint: нового нет; commit/push не запрошены.

##### Вики: состав конструкций — редактура и актуализация

Scope: wiki/ru/Construction-Composition-ru.md и запись состояния; сверка исходников, версионного ограничения — по установленным SDK29/30. C++, ресурсы, Docs/generated, PLN и Archicad не изменялись этой задачей. Параллельная разработка %structype% (#312) сюда не включена.
Status: COMPLETED.
Plan: [x] сверить поведение; [x] обновить текст с сохранением разделов; [x] проверить документ и собственный diff.
Last Completed: пояснены настройка профиля, шаблоны/Info, нумерация, спецтекст, формулы, all/unic и свойства количества; добавлены th_min/area_section с ограничениями. Исправлены скобки классификаций, единицы вывода length/width, описание some_stuff_th и ограничение спецсимволов AC29/30. Все прежние URL и исторический список встроенных свойств сохранены; UTF-8 без BOM/CRLF, синтаксис 17 примеров и diff-check прошли. Артефакты: Hermes scratch/wiki-construction-20261105/validation.json. Это статическая проверка документа, не сборка/исполнение аддона; совместимость BeArt и доступность внешних ссылок не проверялись.
Next Step: обязательных действий нет; проверка примеров в Archicad и совместимости BeArt — отдельная приёмка при необходимости.
Last Checkpoint: текущий коммит [wiki-composition] по запросу владельца; push не запрошен.

---
## #278 — отписка наблюдателей (DetachObserver) (CLOSED)

https://github.com/kuvbur/AddOn_SomeStuff/issues/278. AC_ALL.
Commit 0dd7ba3 (Refs: #278), b55d12f.
Helpers: `DetachObserver(guid)` + `DetachAllObservedElements()` (AC22 — массив заголовков с BMKillHandle, AC23–26 — `ACAPI_Element_GetObservedElements`, AC27+ — `ACAPI_Notification_GetObservedElements`).
ReservationChangeHandler — `DetachObserver` по `released`/`deleted`; ProjectEventHandlerProc — `DetachAllObservedElements()` при `APINotify_Close`/`APINotify_Quit` перед снятием глобального монитора.
clang-format PASS; LSP: SomeStuff_Main 0 диагностик, Helpers.cpp только прежние, Helpers.hpp 3 прежних unused-includes. Задача закрыта.

---
## #276 — неверная высота потолка, создаваемого GDL-объектом (CLOSED)

Scope: Floor_Draw_Object, только размещение потолка и ac_ref_height; AC25 Windows. Issue: https://github.com/kuvbur/AddOn_SomeStuff/issues/276. Commit 0312878.
Для Ceil ac_ref_height=0, object.level=GetOffsetFromStory(zBottom); ветка пола и остальные регионы не изменены. SDK25 API_ObjectType.html:155 и установленный Обычный Потолок25.xml Script_3D:123 подтверждают оба слагаемых высоты.
Прочитаны пользовательские HSF из kuvbur_Помещение с отделкой/hsf (Потолок v2, Потолок по плите v2, Ceiling Editor INT_v2+): ac_ref_height=0 и story-relative level подходят этим путям, сохраняя h_otn как понижение. check_user_gdl.py: 72/72 расчётных проверок PASS. Задача закрыта.

---
## #268 — доведение контура пола до ядра базовой стены (CLOSED)

Scope: AC25 Windows, Sources/AddOn/roombook/Roombook.cpp, RoomSurfaceFinish.cpp. Issue: https://github.com/kuvbur/AddOn_SomeStuff/issues/268. Commit 9963693.
Исправлена причина отсутствия изменения пола: расчёт смещения ошибочно зависел от `ParamComposite::isValid`, который штатная читалка не выставляет при заполненных слоях. Проверка заменена на наличие слоёв; регрессии обеих сторон теперь используют это реальное состояние.
Canonical сборка AC25 Debug exit0; 37/37 проверок пола PASS. Два штатных RoomBook и обратное чтение native-полигонов на test_25 подтвердили для тамбура 116 доведение на 20 мм до ядра и fallback 12.5 мм при отсутствии ядра. Повтор сохраняет точные контуры двух фрагментов пола и потолка; четыре базовые стены неизменны. Задача закрыта.

---
## #192 — регрессия Object-избранного пола/потолка (CLOSED)

Issue: https://github.com/kuvbur/AddOn_SomeStuff/issues/192. Commit ef5c6e7 (Refs: #192).
AC25 Windows; восстановлен минимальный Object/Slab dispatch в surface_geometry.
Extracted production-branch regression RED 4/12 FAIL → GREEN 12/12 PASS (writers/link — doubles, роли не участвуют в dispatch). Clang-format/diff-check и clangd force_refresh 0 diagnostics. Canonical BuildAddOn.py AC25 Debug success. Standalone dispatch regression 12/12 PASS. Задача закрыта.

---
## #267 — поиск черновых полов и потолков по низу и верху стен (CLOSED)

Issue: https://github.com/kuvbur/AddOn_SomeStuff/issues/267. Commits: dbafebc, 021ae3b.
Floor_FindAll пересекает исходные 2D-полигоны видимых неотделочных перекрытий и зон с сохранением отверстий. Floor_FindInOneRoom распределяет роли slab по аналитическим высотам: bottom<=room floor — пол, bottom>room floor — потолок. Nominal-потолок зоны участвует в выборе кандидатов BuildProjectedSlabFinishes наравне с перекрытиями (по возрастанию), перекрытие выше отметки зоны больше не становится потолком при наличии промежуточного nominal потолка зоны.
Canonical BuildAddOn.py AC25 Debug compile/link exit0; 214/214 host checks PASS; два штатных RoomBook на зоне 101 создали по 2 slab finish (пол и потолок), native прямоугольные контактные контуры проверены против footprint зоны при 1e-6 м. Issue закрыта по команде владельца.

---
## #277 — вылеты Undo.dll при переключении окон и в ElementEventHandlerProc (2026-10-15)

Scope: Sources/AddOn/SomeStuff_Main.cpp, Sources/AddOn/Dimensions.cpp, Sources/AddOn/Dimensions.hpp.
Факты и реализация:
1. Создан issue: https://github.com/kuvbur/AddOn_SomeStuff/issues/277
2. Полностью убрана обработка `API_DimensionID` из `ElementEventHandlerProc` (восстановлено безопасное поведение v1.77, исключающее интерференцию с обновлением разрезов/осей).
3. Добавлена ранняя фильтрация `APIVarId_GridElement` на старте `ElementEventHandlerProc`.
4. Исправлен вынос Undo-транзакции: `ACAPI_CallUndoableCommand` убран из глобального `DimRoundAll` (где он открывался при смене окон) и перенесён внутрь `DimAutoRound` точечно для изменяемого размера (только при `flag_write == true` и `!isUndo`).
5. Добавлена процедура очистки `cleanupMemoContentUStr ()` (освобождение `delete (*memo.dimElems)[i].note.contentUStr` и обнуление) при ранних выходах из `DimAutoRound` (проёмы, колонны) и в конце функции.
6. clang-format PASS, LSP clangd 0 ошибок, BuildAddOn.py AC25 Debug `Build succeeded!`.

## Совместимость SDK AC26–30 — #275

Scope: только SDK-адаптация RoomSurfaces.cpp/.hpp, Roombook.cpp, SomeStuff_Main.cpp, SyncSettings.cpp и TestRoombookGeometry.cpp, Windows Debug AC26–30. План: [x] прочитаны issue/документы/SDK25–30 и сохранены baseline; [x] минимальные изменения и clang-format; [x] review дельты; [x] финальные canonical сборки всех пяти версий exit0, CMAKE_BUILD_OK/Build succeeded; [x] APX свежие, hashes/размеры сохранены, все шесть затронутых файлов не новее каждого APX. Причина — implementation error: AC25-поля/сигнатуры без ветвей новых SDK.

Изменено: GetElemTypeID/SetElemTypeID вместо прямого typeID; variation через type с AC26; relation-snapshot хранит SDK-тип ключей; connected query сохраняет type+variation/filter/renovation при новом AC26 parameter shape и имени Grouping с AC27; preferences Platform переименован с AC27. Диагностика не отключена: APIOptional override (без чтения отсутствующего value), integer JSON для AttributeIndex, новые profile/hatch accessors с проверкой bounds и ADB-конверсии AC27/28+.

Validation: clang-format --dry-run --Werror и scoped git diff --check PASS. MCP force_refresh шести source/header файлов выполнен в имеющемся AC25 compile DB; промежуточный SomeStuff_Main unused err находился в baseline вне дельты; финальные ответы source-файлов 0 diagnostics, межверсионный LSP-clean этим не заявляется. Финальные BuildAddOn.py AC26/27/28/29/30 compile+link PASS; результаты/логи/APX SHA256 — C:/Users/da-rogojin/AppData/Local/hermes/cache/scratch/compat-275/results.json и ac26.log…ac30.log. Backup/final.diff/source-manifest.json в том же каталоге; final.diff содержит также сохранённые параллельные routeReason/log edits, не приписывать их этой задаче. Рабочее дерево и индекс менялись другими сессиями; откатов/стэшей/stage/commit/push нами не выполнялось. AC25/macOS/Release сборки и host/runtime тесты в этой задаче не выполнялись; стандартная установка GRAPHISOFT содержит AC25, не AC26–30. Issue275 OPEN; Docs/generated по прежнему Scope отложены. Next Step: runtime-приёмка по отдельной команде. Last Checkpoint: нового нет.


## routeReason удалён — AC25 Windows, #272

Scope: RoomSurfacePartition.hpp, RoomSurfaceCandidates.cpp, Roombook.cpp, TestRoombookGeometry.cpp. [x] удалены поле RoomSurfaceSource::routeReason, присваивания и JSON-member; DBprnt сохраняет причины как сообщения source:, без фиктивного имени поля. [x] Diagnostic Wall/Slab fallback выбирается по baseType: discovery для них добавляет pending-заготовки, остальные типы читает отдельно. Тест профильной стены сохраняет проверки false/пустого bases/неполного snapshot без строки. [x] поиск routeReason по всем Sources/AddOn:0; clang-format/diff-check PASS; clangd четырёх файлов0 diagnostics; canonical BuildAddOn.py-v25 exit0. [x] реальный AC25 partitionSelfTest254/254 PASS, включая ранее падавшую проверку profile explicitly requests cached native fallback. Evidence scratch/274-cleanup/{routeReason-build.log,routeReason-selftests.json}; APX загружен из Build/SomeStuff/25/Debug. Native writer и diagnostic partitionSurfaces отдельно не выполнялись. Соседние правки совместимости SDK сохранены; чужие поля/геометрия не менялись. Issue272 comment6189011570 прочитан обратно; issue не закрыта. Last Checkpoint9963693; нового commit нет. Docs/generated отложены по Scope. Next Step: задача удаления завершена; остальные задачи только по отдельной команде.


## #274 — очистка потолков: выполнено, AC25 Windows

Scope: только условие очистки Floor/Ceil, его комментарий и две регрессии потолка. [x] CleanCollinearPolygonVertices вызывается для конечных полов и потолков; доведение до ядра по-прежнему только Floor. [x] clang-format/diff-check, clangd обеих cpp 0 diagnostics, canonical BuildAddOn.py AC25 Debug exit0. [x] реальный partitionSelfTest: потолок2/2 и вся очистка7/7 PASS, общий253/254 с прежним FAIL272 profile explicitly requests cached native fallback вне scope. Потолок5→4 вершины, площадь12м² и ссылки/избранное сохранены; повтор стабилен. Native writer не проверялся. Evidence scratch/274-cleanup/{ceiling-build.log,ceiling-selftests.json}. По новой прямой команде владельца точки RoomSurfacePartition.cpp333/349 удалены, списокbreakpoints[] проверен; не восстанавливать. VS launch сообщил отказ, но реально запустил PID23180 под отладчиком; timeout curl вызван остановкой349, после удаления точек и continue повтор завершился exit0. Issue274 OPEN, comment6188774049 проверен. Last Checkpoint: нового нет, commit/Docs/generated отложены. Next Step: функция завершена; native-приёмка только отдельным запуском при необходимости.


## #274 — отдельная очистка коллинеарных вершин (AC25 Windows)

Scope: RoomSurfaceFinish.cpp/.hpp и пять проверок в TestRoombookGeometry.cpp; вызов только для конечных Floor-полигонов после ExtendFloorContoursToCore. CleanCollinearPolygonVertices возвращает число удалённых вершин, использует SDK RemoveSameDirEdgesWithSameEdgeUserData; сохраняет разрывы edge user data и геометрию дуг (соседние дуги одной окружности SDK также может объединить). Исходные стены/зоны/потолки не меняются. Issue https://github.com/kuvbur/AddOn_SomeStuff/issues/274 OPEN.
План: [x] SDK25 headers после неполезного LightRAG; [x] функция/интеграция/регрессии; [x] clang-format, clangd 0 диагностик (новый helper есть в documentSymbols), canonical BuildAddOn.py Debug exit0; [x] реальный AC25 JSON partitionSelfTest: новая очистка5/5 PASS, всего251/252, прежний FAIL272 profile explicitly requests cached native fallback вне scope. Проверены пустой вход, цепочка и замкнутый стык, повтор, отверстие, дуга. Native writer RoomBook в этой задаче не запускался. Фикстуры Test_file и Build/test_25.pln SHA25657c10b0480dbf3af624223d63830c647a27137aac21e30709fd1835239550413 неизменны. Evidence: Hermes scratch/274-cleanup/build.log и selftests.json. Host PID6288 test_25 без отладчика, APX из Build/SomeStuff/25/Debug; после VS launch бинарник SHA25680803bd3fc812c59246142ff01821c97c05e7b05a1c580679cca08aa81d2d9cd (каноническая сборка до launch SHA256dc226469a8cff395ee9f0fbbcf4ec3a9fa2b5480cac06bc0bce7b44099a5e191). Пользователь разрешил закрыть предыдущий test_25 без сохранения; диалог восстановления нового процесса закрыл сам. Пользовательские breakpoints333/349 не менялись. Docs/generated отложены по действующему Scope.
Last Checkpoint: нового нет, commit/push не разрешены. Next Step: при необходимости отдельная native-приёмка; эта функция реализована и проверена, остальных активных задач не продолжать автоматически.


## 2026-10-15 — Docs roombook-срез (ARCHITECTURE/REPOMAP/SEQUENCES/Roombook.md/generated)

Обновлена документация под коммиты 18d05b7..68efad7 (команды владельца «Обнови документацию по roombook», затем «Обнови также ARCHITECTURE REPOMAP SEQUENCES»), шаги 1–4 UPDATE_PROCEDURE.md. symbols.json 1471→1578, callgraph.json 696→699 (378 координат, 3 ребра на `CommonFunction.cpp:2653` `AdvanceProcessPhase`, 21 метод разрешён суффиксным матчингом, 7 неоднозначных рёбер — в т.ч. `Set` у `Frame`/`BeamAxis` — оставлены со старыми координатами; call_sites 1-based, но НЕ пересобирались, meta `roombook_scoped_2026_10_15`; рёбра `GetSurfaceFinishLayer` удалены). Roombook.md: секция актуализации коммитов, «Точка входа», «Зависимости/Инварианты», массовая коррекция систематического сдвига +1 (~160 координат в четырёх секциях). ARCHITECTURE.md: шапка/дерево/deps/Mermaid RB→8 целей/сценарий 11. REPOMAP.md: шапка/дерево/28 карточек/статистика compile_commands (46→51)/«Изменения 2026-10-15». SEQUENCES.md: шапка + сценарии 1–10 координатно сверены + новая 11-я диаграмма RoomBook. _progress.md: запись сессии. Build/debugger/commit не выполнялись; незакоммиченные правки параллельной сессии (callgraph meta `legacy_preferences_scope`, #273) сохранены. Артефакты: Hermes scratch/docs-roombook-20261015/ (sym-*.json, roombook-*.json, new-*.json, edges.json, scan.json, call-scan.json).

## Углы стен / API_WallPart — локальная приёмка завершена (2026-10-15, #252)

Scope: ClampSurfaceWallToParts, production-интеграция и адресные тесты, AC25 Windows. Исправлены привязка guid+roomEdge, legacy zero bounds, union интервалов, доведение внешних концов после сбора всех контактов, сохранение внутренних разрывов/вырезов и координат проёмов; curved-base chord correction исключена. Canonical BuildAddOn.py compile/link ранее выполнен; extracted helper21/21 PASS (прежний helper12FAIL/21). Последний actual SDK-host на APX26485d00d3f207bb13655295e5d4e9310a8ce3c3388b1a2a1b9107abb591e9b9: wallPart21/21 PASS; общий247checks/246PASS/1FAIL — прежний profile fallback #272 вне исправления углов, общий GREEN не заявляется.

После завершения владельцем запуска PID59092/test_25 штатный RoomBook создал2664GUID и удалил56 (1608→4216); повтор удалил2664 и создал2664 (4216→4216), survivors первого создания0. Зона101/0AB1934B-44F0-444A-93C5-F1A6424C2345:43wall finishes; четыре угла в трёх высотных поясах,12/12пар native-тел имеют общую3D-вершину, max gap0 в обоих запусках. Независимо прочитаны native-полигоны; не только оси. Параметры и множества native-вершин43стен совпадают с округлением6знаков; полная топологическая/площадная идентичность не заявляется. Проверка расстояния до номинального угла зоны давала ложные отказы1мм из-за native-подрезки толщины; реальные соседние тела соприкасаются. Остальные зоны/сложные углы not verified.

Диагностическая гипотеза отключённого создания опровергнута в Draw_Elements:5191: create_all_elements=true, create_wall_elements=true, isValid=true,43walls. Собственная точка5191 удалена; две исходные333/349 после SDK-test восстановлены enabled=true. Последний SDK запрос сначала остановился на исходной349/таймаут curl; после временного отключения по разрешению владельца выполнен и сохранён полный ответ. PLN hashes и APX совпали до/после обоих native-прогонов; PLN не сохранялся, VS оставлен работающим. Source/прочие задачи/Docs/generated сохранены; commit/push по запрету владельца не выполнялись. Last Checkpoint: нового нет.

Evidence: Hermes scratch/wallpart-fix/native-final-validation.json, native-{restart,repeat}-*.json, native-current-selftests-complete.json, writer-flags.json. Next Step: владелец проверяет оставленный результат; по отдельной команде расширенная приёмка других зон, Docs/checkpoint. #252 остаётся открытой на общий scope.

## Аналитический отбор пола и потолка — локальное исправление #267, AC25 Windows

Status: COMPLETED в адресном объёме зоны101; общая #267 OPEN. Floor_FindAll теперь пересекает исходные finite2D-полигоны видимых неотделочных перекрытий и зон, сохраняя отверстия; native collisions и wall-tip discovery удалены. Floor_FindInOneRoom сохраняет slab роли по аналитическим высотам: bottom<=room floor — пол (включая ниже лежащие опоры), bottom>room floor — потолок независимо от верха зоны. Prepared источники/контуры переиспользуются, прежние material/Favorite/writers/links/by_slab residual не менялись. Floor descending top и ceiling ascending bottom уже были в текущем дереве, повторно не разворачивались.

Canonical BuildAddOn.py AC25 Debug compile/link exit0, APX SHA256a9ce80d1aef021fd13d75ae8cd6fdfa5cd607c88beccfda5ccb9cd00b11b4486. Parent заново выполнил standalone production-helper27/27; host selftests214/214 failed0, включая подготовленные основания, верхний пол/ближний потолок и обратный порядок. MCP diagnostics Roombook/Test0; сборка — compiler arbiter. Format/diff-check PASS. Отладка сохранена частично: инструмент вырезал середину длинного буфера, полное отсутствие production warnings по ней не заявляется; retained route: analyticWall385/analyticSlab14/nativeFallback1, wall3DReads0/slab3DReads1, coordinate calls0.

Два штатных RoomBook118.5966072с/132.3448936с; первый2667созданы/56удалены, повтор2667созданы/2667удалены. Parent проверил равенство конкретных first-created и repeat-removed GUID множеств, не только чисел; старых first-created не осталось. На зоне0AB1934B-44F0-444A-93C5-F1A6424C2345 оба раза ровно2slab finish с independently read links: пол base34D1F75D-7C59-4CA5-AF26-137E1A1ECF1A/contactZ0, потолок baseD67B5A4F-4116-47B1-8E81-6434344CBE37/contactZ3.3. Нижний4299BE46 и nominal zone residual не созданы, дальнийA7558175 не выбран. Полные native прямоугольные контактные контуры без отверстий проверены против footprint зоны при1e-6м, max corner drift6.149186947921065e-7м; все ориентированные грани двух конечных тел при повторе совпали, max drift0. Полное покрытие/геометрия остальных2665результатов не проверялись, время не A/B benchmark. Оба saved PLN SHA25657c10b0480dbf3af624223d63830c647a27137aac21e30709fd1835239550413 и APX unchanged; loaded module path проверен D:/SomeStuff_addon/Build/SomeStuff/25/Debug/SomeStuff.apx. VS Run/PID45788 оставлен с повтором без сохранения, две pre-existing breakpoint333/349 восстановлены, своих нет.

Evidence: Hermes scratch/267-analytic-fix/{build.log,built-manifest.json,selftests.json,first/repeat-*.json,validation.json,capture.py,verify.py}. Первое standalone-компилирование без ROOM_ANALYTIC_REGRESSION не имело main; после чтения testTU выполнена корректная команда с macro и include path. Startup HTTP до открытия окна дал connection refused; WaitForInputIdle подтвердил test_25 и следующие запросы успешны. Production не менялся для обхода этих ошибок. Next Step общего scope: приёмка владельцем оставленного результата и отдельные сложные/другие зоны. Docs/generated/коммит/push не выполнялись по Scope; checkpoint прежний, issue не закрывать без команды.

## #273 — только старые preferences (AC25 Windows, 2026-10-14)

Scope: ClearLegacyProjectPreferences без user data/ElementSet/AddOnObject/обхода баз; JSON, модель, слои и права не меняются. [x] Scope сокращён по команде владельца; [x] probe/условная запись/read-back и возврат GSErrCode в callback; [x] format/diff-check, canonical AC25 Windows Debug build success, 8/8 SDK-double проверок текущего extracted helper. Стенд не является host-проверкой. Владелец сообщил о самостоятельном успешном runner и явно потребовал закрыть #273; повтор собственного runner отменён после защитного exit20/non_test_archicad_process. Независимый host/save-reopen not verified. Docs/card и scoped generated обновлены; stale MCP и унаследованные координаты отмечены. Status: ACCEPTED_BY_OWNER; закрытие issue разрешено владельцем. Next Step: отдельная работа по оставшимся активным задачам; #273 принят, push не запрошен. Last Checkpoint: текущий checkpoint [273-C2], hash определяется git log после commit; остальные активные задачи сохранены.

## 2026-10-15 — 5-е и 6-е падения семейства Undo.dll / корреляция с SomeStuff (PID 48212, PID 23508)

Scope: только чтение логов/отчётов/дампа.
Факты:
1. PID 48212 (17:15:13): триггер `OpenSectionInNewWindowCommand` → `ContinueEditing`. Исключение C0000005 чтение 0x1060, fault `Undo.dll` RVA 0xA000. В стеке те же 5 фреймов `Grid Tool.apx`.
2. PID 23508 (17:45:36): триггер деактивация режима навесной стены → `OpenSectionInActWindowCommand` → `ContinueEditing`. Исключение C0000005 чтение 0xFFFFFFFFFFFFFFFF, fault `Undo.dll` RVA 0xA006. В стеке те же 5 фреймов `Grid Tool.apx`.
3. Пользователь сообщил: **после отключения SomeStuff вылеты прекратились**.
4. В обоих стеках SomeStuff.apx отсутствует в момент сбоя.
Вывод/гипотеза: Это важнейшее эмпирическое наблюдение. Поскольку SomeStuff не виден в конечном стеке исключения, речь идёт о косвенном влиянии (side effects): регистрация хуков/нотификаций (ACAPI_Notify_Catch* / ElementSet / Undo транзакции), модификация данных/свойств, приводящая к порче цепочки Undo или кэша элементов, на которых позже спотыкаются Grid Tool и Undo.dll при перестроении видов/разрезов.
Evidence: `scratch/crash-20261015-48212/evidence.json`, `scratch/crash-20261015-23508/evidence.json`. Код/модель не менялись. Last Checkpoint 18d05b7.

## 2026-10-15 — четвёртое падение семейства Undo.dll/Grid Tool/разрез, PID33372

Scope: только чтение логов/отчётов/дампа. Завершено: сессия 14:39:59–15:43:25 (63м); команды многократное Переместить/Отменить Выборку; RPT: MoveVBElementsCommand×2→OpenSectionInActWindowCommand→ContinueEditing; FastLogger: Open S/E/IE/3DD STEP /21→/22 непосредственно перед аварией. Исключение C0000005 чтение 0xFFFFFFFFFFFFFFFF, fault Undo.dll RVA0xA006, main thread17844 — ПОЛНОСТЬЮ идентично падению 2026-10-14 14:39 (тот же RVA, тот же двухфреймовый top Undo.dll, те же пять фреймов Grid Tool.apx с точными смещениями +573671/+453406/+241352/+243848/+278152, TOS Undo.dll+0x104A2). SomeStuff загружен, в стеке отсутствует. Итого семейства: 4 падения за 4 дня (12.10, 14.10×2, 15.10), ВСЕ вокруг открытия/перехода на разрез, ВСЕ с фреймами Grid Tool и fault Undo.dll, ВСЕ чтение −1 (0x9FE4 дважды, 0xA006 дважды). Решающая проверка теперь обязательна: переход на разрез с ОТКЛЮЧЁННЫМ Grid Tool на защищённой копии; повреждение данных модели, питающих оба модуля, не исключено. Evidence:scratch/crash-20261015-33372/evidence.json. Next Step: тест с отключённым Grid Tool; чистая TW-версия; дампы — Graphisoft support/WinDbg с символами. Код/модель/профили/процессы не изменены; build/runtime/restart/commit/push нет. Last Checkpoint18d05b7 сохранён.

## 2026-10-14 — третье падение семейства Undo.dll при переходе на разрез, PID65864

Scope: только чтение логов/отчётов/дампа. Завершено: сессия 13:44:55–14:39:16 (54м); владелец подтвердил триггер — переход на разрез; RPT: Синхронизировать выделенные→OpenSectionInNewWindowCommand→ContinueEditing; FastLogger: Open S/E/IE/3DD, Grid Tool активно выполнял CGridElement::UpdateSectionGridLines и CreateGridElement непосредственно перед аварией. Исключение C0000005 чтение 0xFFFFFFFFFFFFFFFF, fault Undo.dll RVA0xA006 (+0x1C от сайта 0x9FE4), main thread65868; тот же двухфреймовый top Undo.dll и те же пять фреймов Grid Tool.apx. ТРЕТЬЕ падение в семействе Undo.dll/Grid Tool/разрез; Grid Tool присутствует во всех трёх стеках и в этом случае активно исполнял свой код — сильнейший лид, но присутствие в стеке ≠ доказательство вины. SomeStuff загружен, в стеках отсутствует. Evidence:scratch/crash-20261014-65864/evidence.json. Next Step: проверить переход на разрез с ОТКЛЮЧЁННЫМ Grid Tool на копии проекта; параллельно чистая TW-версия (ранние ODB-assert'ы External/HadInternalLinksFlag); дампы — Graphisoft support/WinDbg с символами. Код/модель/профили/процессы не изменены; build/runtime/restart/commit/push нет. Last Checkpoint18d05b7 сохранён.

## 2026-10-14 — повтор hotlink-сайта через 3 минуты, PID60288 (команда «Отслеживать»)

Scope: только чтение логов/отчётов/дампа. Завершено: сессия 13:41:17–13:43:26; последняя команда Отслеживать; RPT: UpdateHotlinkNodeCommand→UpdateHotlinkNodesCommand_CheckAndSendIfNeeded, TWReservationChanged. Исключение C0000005 чтение 0x98, fault ARCHICAD.exe RVA0x1BEAC7A, main thread59736 — ПОЛНОСТЬЮ идентично падению PID57740 (тот же RVA, тот же стек UpdateHotlinkNodesCore+614). FastLogger-хвост: слияние hotlink-кэша с массивом Beam/Wall Junction-данных (CSG::LinkProperties, JCTN::JunctionSet, BeamSegment-BeamBeam/WallBeam/BeamColumn, VBD::BeamSegmentVRDData, PointerLineListTable). Два падения подряд → сайт воспроизводится при каждом обновлении hotlink-узлов в редакторе модуля. Ранее виденные ODB-assert'ы External/HadInternalLinksFlag намекают на повреждение external-link данных проекта — параллельно стоит восстановить чистую TW-версию/копию. Доказательств вины аддона нет. Evidence:scratch/crash-20261014-60288/evidence.json. Next Step: прекратить редактирование модулей до разбирательства; WinDbg с символами либо защищённое воспроизведение; Graphisoft support с дампами. Код/модель/профили/процессы не изменены; build/runtime/restart/commit/push нет. Last Checkpoint18d05b7 сохранён.

## 2026-10-14 — третий сайт: hotlink-обновление при «Редактировать Модуль в отдельном ARCHICAD», PID57740

Scope: только чтение логов/отчётов/дампа. Завершено: сессия 13:35:20–13:40:24; команды Резервировать/Параметры Выбранного Элемента Оси/Синхронизировать выделенные/Временно Разгруппировать/Редактировать Модуль в отдельном ARCHICAD(дважды); RPT: UpdateHotlinkNodeCommand→UpdateHotlinkNodesCommand_CheckAndSendIfNeeded. Исключение C0000005 чтение 0x98, fault ARCHICAD.exe RVA0x1BEAC7A, main thread42464 — НЕ Undo.dll; стек через ReferenceElemManager.dll SRM::ReferenceElemManager::UpdateHotlinkNodesCore+614; FastLogger-хвост подтверждает путь ReferenceElemNode::UpdateFromPreparedSourceProject→mergeDelta→CreateProxyObjects::mapIdSpace→SRM::HotlinkNode/VBES::ReferenceElemCache/VBElem::Hotlink. Чтение 0x98 похоже на смещение поля от невалидного указателя — наблюдение. Grid Tool и SomeStuff загружены, в стеке отсутствуют. Общий контекст всех сессий: Teamwork-резервирование и пути external-link/hotlink. Evidence:scratch/crash-20261014-57740/evidence.json. Next Step: WinDbg с символами ARCHICAD.exe либо защищённое воспроизведение редактирования модуля на копии; код/модель/профили/процессы не изменены; build/runtime/restart/commit/push нет. Last Checkpoint18d05b7 сохранён.

## 2026-10-14 — повторное падение Undo.dll RVA0x9FE4 при открытии разреза, PID12460

Scope: только чтение логов/отчётов/дампа. Завершено: сессия 13:08:35–13:20:46; последние команды Переместить/Параметры Модельного Вида/Открыть с Текущими Параметрами Вида; RPT: OpenSectionInNewWindowCommand→ContinueEditing. Исключение C0000005 чтение 0x00007FF90000003C, fault Undo.dll RVA0x9FE4, main thread42632 — ТОТ ЖЕ RVA, что и падение 2026-10-12, с тем же контекстом команд и фреймами Grid Tool.apx; SomeStuff загружен, в стеках отсутствует. Параметры[d0,0x00007FF90000003C] похожи на повреждённый/усечённый указатель — наблюдение, не доказательство автора. Второй экземпляр PID49848: нефатальные ODB assertion ObjectInfo.cpp:569 «Loading of the object failed» при LoadAllDataFromServerCommand (Teamwork), сессия продолжилась — отдельный симптом данных. Падение 2026-10-13(BMAllocatePtr −7байт) — другой путь, не смешивать. Evidence:scratch/crash-20261014/evidence.json; skill archicad-fastlog/references/undo-section-window-crash-repeat.md. Next Step: по разрешению защищённое воспроизведение открытия разрезов на копии проекта до/после отключения Grid Tool/SomeStuff либо debugger+символы Undo.dll. Код/модель/профили/процессы не изменены; build/runtime/restart/commit/push нет. Last Checkpoint18d05b7 и активные задачи сохранены.

## 2026-10-13 — анализ падения AC25 Windows, PID49660

Scope: только чтение логов/отчётов/дампа. Завершено:20:00:13 RebuildListWindowCommand/ListZoneText при открытии V2_Ведомость отделки;20:00:17 BMAllocatePtr запрос −7байт;20:00:20 Application crash. Исходные строки/аргументы/F7метки проверены. RPT:170–171 и dump:C0000005 write0, ARCHICAD.exe RVA0x15299DE, main thread15776. Diagnostic-стек GSRoot/BMAllocatePtr и exception-стек ARCHICAD не содержат SomeStuff, хотя модуль загружен. Первопричина и прежнее влияние аддона not verified; не приписывать прежнему Undo/Grid Tool crash. Evidence:scratch/crash-20261013/evidence.json; skill archicad-fastlog/references/list-rebuild-negative-allocation.md. Next Step: по разрешению защищённое воспроизведение ведомости до/после операции аддона либо debugger/symbols. Код/модель/профили/процессы не изменены; build/runtime/restart/commit/push нет. Last Checkpoint18d05b7 и активные задачи сохранены.

## Z9 direction — верхний откос Wall-fallback, AC25 Windows

Status: COMPLETED для ошибки направления на24воспроизведённых окнах; общий Z9 PARTIAL. Native RED подтвердил противоположный боковым знак глубины. В OtdWall_Draw_Wall удалён дополнительный разворот begC/endC; shared OpeningReveals_Create_One и Beam/object writers не менялись. Минимальный diff относительно pre-edit снимка: две assignment-строки и русский комментарий.

Canonical BuildAddOn.py AC25 Debug реально скомпилировал Roombook.cpp и перелинковал APX,exit0;SHA2562a8ffe51e414e4e54ec9a49e1089c0433b0f6712c65152a844358c1344b7aff7. clang-format/diff-check PASS; MCP refresh сохранил13известных unused диагностик, отдельный fresh direct LSP в этом ходе не запускался. Bare python не имел requests: использован уже установленный Hermes venv, без установки пакетов. VS стартовал после подтверждения Design/отсутствия Archicad; чужой процесс не останавливался.

Два реальных RoomBook:selftests75/75failed0;335созданы,повтор335удалены/335созданы,bodywise max drift9.848857823102728e-8м при1e-6м. Проверка24конечных native верхних граней тех же окон:RED→GREEN в обоих запусках,интервалы глубины/связи/overrides сохранены. Сохранённые PLN hashes unchanged;121исходный GUID сохранён,представимые основные контакты PASS. Актуальная Отладка ERROR IN TEST0;полный TestKit не заявляется.

Остатки общего Z9:5строк без найденного верхнего результата по full-aperture checker;один неполный owner-match,параметры refSide/frame/sill и ожидаемые материальные правила не экспортированы. Некоторые боковые full-aperture проверки дают полосу1мм на контакте с исправленным верхом;ещё один нестандартный проём не проходит ожидаемую frame-depth проверку. Эти результаты не объявлены PASS и не исправлялись данной локальной правкой. Другие favorites/reference modes,двери,Undo/Redo и SDK/macOS not verified.

Evidence:scratch/z9-reveal-final/upper-direction.json (RED),green/{direction-validation.json,report.json,first/repeat-geometry.json,final-validation.json};исполняемые verify-direction.py/reveal-audit.py/verify-repeat.py. Archicad оставлен VS Run с повтором,PLN не сохранялся. #252 остаётся открытой;Docs/generated отложены по Scope,commit/push нет;checkpoint18d05b7. Следующее действие общего Z9 — отдельно классифицировать отсутствующие/нестандартные верхи и подтвердить параметры/материальные правила,не расширяя эту исправленную ошибку знака.

## 2026-10-12 — Одна стена с проёмами вместо дробления, #252, AC25 Windows

Status: COMPLETED в проверенном ортогональном объёме; общий охват #252 PARTIAL. Запрос: https://github.com/kuvbur/AddOn_SomeStuff/issues/252#issuecomment-6125898377. Итог записан и прочитан обратно: https://github.com/kuvbur/AddOn_SomeStuff/issues/252#issuecomment-6129822046; issue OPEN.

BuildSurfaceWalls готовит одну оболочку в плоскости контакта, пустое дополнение задаёт прямоугольными OtdOpening через Opening_Add_One и штатный Opening_Draw. Материальные/высотные полосы и границы единственного основания сохранены. OtdWall_GetArea вычитает неперекрывающиеся отверстия до поправки на наклон; относительный lower задан и пересчитывается при подрезке. Непредставимый малый вырез не заполняется молча, полностью пустая полоса не создаётся. Ошибка создания проёма запускает удаление новой неполной стены; принудительная проверка ошибки SDK/ошибки удаления не выполнялась.

Поправка владельца: первоначальное назначение null всем вырезам потеряло связь с настоящими окнами. LinkSurfaceOpenings проецирует legacy-проёмы того же основания, при необходимости разбивает только пустоту по их границам и сохраняет реальный opening base_guid. Два разных GUID на одной ячейке — отказ, не выбор первого. OtdOpening.surface_geometry отделяет геометрию выреза от его provenance; surface-стены не проходят подготовку откосов, legacy-openingWalls остаётся отдельным входом. Текущий ProcessElementsForRoomData вызывает OtdWall_Create_FromWall для стен с реальными проёмами. Синтетическим геометрическим пустотам фиктивный GUID стены не присваивается.

Исправлен обнаруженный недетерминированный отказ: representability-gate перед same-source union позволял первому снимку создать прямоугольную часть, отбросив диагональный хвост, а объединённому повторному снимку — пропустить всю грань. Теперь полное provenance/ориентированная плоскость группируются до проверки writer. Точный диагональный край не приближается прямоугольниками: вся такая объединённая грань остаётся неподдержанной. Это ограничение текущего writer, не доказанная невозможность всех SDK-путей.

Финальная canonical BuildAddOn.py AC25 Windows Debug реально скомпилировала RoomSurfaceFinish/TestRoombookGeometry/Roombook и перелинковала APX; SHA25696402943858d5d76f7c4145caad5f5ed05e0d1fe3b7e7fdda63327c046f4ad30. clang-format и git diff --check PASS. Fresh direct LSP version1: адаптер и тесты0, Roombook13 прежних диагностик (8unused-variable/5unused-includes), новых нет. Selftests75/75, failed0; в свежей Отладка225 ok-маркеров за три вызова, ERROR IN TEST0, ошибок создания/ownership нет. Полный TestKit этим числом не заявляется.

Два реальных RoomBook:335→335, повтор удалил335 и создал335.149основных surface-стен,121элемент откосов,55проёмов,10перекрытий; раскрытые типы:230Wall/40Beam/55Window/10Slab. Native-контуры всех335результатов сопоставлены по типу/реальным связям/ориентированным контурам; фактический max vertex drift9.848857823102728e-8м при неизменном допуске1e-6м. У46отделочных проёмов прочитана реальная связь с30исходными окнами;9вырезов не имеют отдельного реального проёма.149стен имеют по одному действительному основанию/зоне.121исходный GUID сохранён. Главные грани отделены от откосов по фактической классификации; торцы поперечных стен не включены в покрытие. Обе направленные polygon-разности для представимых полных контактов PASS; число raw непрямоугольных фрагментов37→36 меняется при объединении входов, а не при изменении результата.

Остаток: две целые грани зоны41BE148A-678C-4105-8743-03BCA8D1957D, основания8C917E05-1282-41C5-ACDC-37A1C1A47DED и49D00609-3F3E-49E0-8B44-AD972D92FEB9, содержат диагональный край; их суммарное полное покрытие20.498776333164123м² не создано. Окно4F6A150D-A60B-4ABA-927D-0FC46F0D3FE7 на таком основании без отделочного проёма. Прежние четыре трапеции и остальные явно неподдержанные/неразрешённые контакты не объявляются выполненными. Наклон с отверстиями проверен геометрическими selftests; native-отверстие наклонной стены, реальные двери, принудительный SDK-failure, Undo/Redo и другие AC/macOS not verified. Адресная проверка всех глубин/сторон откосов Z9 остаётся отдельной.

Финальный контролируемый прогон использует актуальную сохранённую фикстуру Test_file/test_25.pln и её build-copy, обе SHA256a12f5aed6de0d356bba5d279cef49ead94a69ac1409844638aaf841913977795; после запуска/повтора hashes источников, APX и обеих PLN равны manifest. В течение прерванных частей сессии модели/рабочее дерево менялись; прежние GUID/manifest не выдаются за текущую фикстуру, причина внешнего изменения не приписывается без доказательства. Сохранение проекта не вызывалось. Archicad оставлен в VS Run, собственных breakpoint нет.

Evidence: C:/Users/da-rogojin/AppData/Local/hermes/cache/scratch/surface-openings/final/{build.json,first-audit.json,repeat-audit.json,final-validation.json,verify-repeat.py}, сохранённые JSON геометрии/связей/классификаций и актуальная Отладка. Ошибочные промежуточные гипотезы checker-а (фильтр по одному слою, включение откосов/торцов, проверка ортогональности до полного union) не считаются провалом окончательного покрытия. ReNum/CommonFunction и пользовательские прочие изменения не правились этой реализацией; commit/push/закрытие issue нет. Docs/generated по Scope отложены до отдельной команды, карточка Roombook до такого обновления описывает прежнее дробление. Нового checkpoint нет, остаётся18d05b7. Переиспользуемые выводы сохранены в skill lightrag, references/somestuff-surface-openings-ac25.md.

## 2026-10-12 — исследование поиска перекрытий вне тела зоны, AC25 Windows

Status: COMPLETED (исследование исходников и SDK; реализация не выполнялась). APIAny_SearchElementByCoordID уже используется для стен в CollectRoomInfo (Sources/AddOn/roombook/Roombook.cpp:1610–1630). По установленной документации SDK25 API_ElemSearchPars поиск работает по внутренней XY-точке полигона и возвращает один GUID с наибольшим уровнем ниже z; пересечения с телом зоны не требует. Для потолка выше зоны нужен заданный верхний предел поиска, не поиск вверх. Один запрос/конечная сетка не доказывает полноту; одинаковые уровни и несколько перекрытий требуют независимого геометрического отбора.

Замена только Floor_FindAll (:2102, GetCollisions) недостаточна: Floor_FindInOneRoom (:2169–2176) требует перекрытия отметки пола/потолка диапазоном slab, новый RoomSurfaceCandidates.cpp:151–176 отдельно получает основания из RoomRelations, RoomSurfacePartition.cpp:231–235 требует копланарности. Рекомендация, не утверждённая реализация: coordinate search как поиск кандидатов с проверкой роли/зазора и XY-покрытия, отдельная привязка через зазор без ослабления точного contact matcher. Полное покрытие — независимый отбор перекрытий и разбиение зоны по их проекциям. Следующее действие по отдельной команде — read-only AC25 проверка уровней z, пола/потолка с зазором, нескольких/одноуровневых перекрытий, отверстий и отделки, затем выбор алгоритма. Compiled/Tested: not performed. Код/модель не менялись; checkpoint остаётся18d05b7, активный Z9 не изменён. Переиспользуемые выводы: skill lightrag, references/somestuff-slab-coordinate-search.md.


## Z8 #252 — проверка откосов двух новых окон, AC25 Windows

Status: COMPLETED (проверка), результат FAIL; исправление не выполнялось. На текущей открытой модели выполнены два штатных RoomBook без restart/save PLN. Окна 6D845F13-B537-41D6-AA71-ED35579287F7 и E5547C5D-0082-49DA-9D26-5C575FD3CD61: боковые и верхний откосы отсутствуют у обоих. Возле каждого окна прочитаны шесть стен основной отделки; их базы соответственно 536DEA6F-E111-446D-B66D-EDDE9D15E377 и FA9703B0-FFB5-4F06-9803-541458525A37. Это не откосы. Глубина/сторона/материалы откосов not verified: требуемых элементов нет.

Оба запуска пересоздали 107 элементов (93 стены, 14 перекрытий). Независимое сравнение всех ориентированных контуров native-тел и прочитанных source/zone links подтвердило bodywise equality при допуске 1e-6 м, max vertex drift 0; стабильность не устраняет пропуск. 17 исходных GUID сохранены, геометрия обоих окон before/repeat идентична. Hash двух сохранённых PLN и APX не изменился. Новая сборка, TestKit и runner не выполнялись; production-код read-only, ReNum.cpp не менялся этой проверкой. Archicad оставлен с повторным результатом; модель не сохранена.

Причина по исходникам, без breakpoint-наблюдения: BuildRoomSurfaceFinishes очищает прежние OtdWall (:571–573), новые surface-стены не получают openings (:530–555; OtdWall_Add_One :1964–1987), а ProcessWallFinishes пропускает пустые openings (:340–341). Ранее Opening_Add_One заполнял их при legacy-подготовке стен (:1812/:1886). Отсутствие двойного вычитания отверстий не заменяет отдельную подготовку откосов. Классификация: implementation error при переносе consumer-пути.

Артефакты: C:/Users/da-rogojin/AppData/Local/hermes/cache/scratch/z8-window-check/ (check.py, readback.py, audit.py, raw before/first/repeat, final-audit.json). Результат записан в существующую #252, комментарий прочитан обратно: https://github.com/kuvbur/AddOn_SomeStuff/issues/252#issuecomment-6113733853. Issue OPEN; коммит/push не выполнялись. Следующее действие — только по отдельной команде восстановить reveal-подготовку, не добавляя openings для повторной прорезки основной surface-отделки.


## Z7 #252 — документация, комментарии и checkpoint AC25 Windows

Проверенный объём текущего checkpoint: production reader/partition/beamFinish, surface-based подготовка штатной отделки и canonical same-source grouping; #252 не закрывается. Русские комментарии добавлены в пяти файлах roombook; code/ordered-literals/balance равны сохранённому pre-comment снимку. clang-format и fresh direct LSP пройдены, новых диагностик нет.

Final restart_archicad_for_test.ps1: build/load SUCCESS после комментариев. Первый запуск остановился до сборки на информационном npm notice; повтор с process-local npm_config_update_notifier=false/npm_config_loglevel=error прошёл реальные HTMLHint/custom verify/i18n и canonical AC25 build/load. Свежий live C++ selftest60/60, failed0; оба saved PLN hashes unchanged. APX SHA256468b5d48e8d072342fb7437f62182073b509b87fe3438a280580003eeb92ed30. Optional JSON runner отсутствует и пропущен; старые TestKit85/39418 не приняты как свежая проверка. Native63→63/bodywise/max drift6.5868125745221e-7м — предыдущий behavior-equivalent запуск, не новый writer после комментариев.

Docs/modules/Roombook.md и json_commands.md, REPOMAP, _progress и generated обновлены:1470symbols/695edges. Родитель независимо сверил15source hashes/6doc hashes,651live ranges,579symbol joins и равенство JSON проверенным candidates;891посторонних symbols/414edges/order сохранены. Четыре inherited external stale endpoints/один site помечены; operator!= и cross-TU/SDK полнота не заявляются. При review исправлены устаревшие TESTING+AC25 подписи регистратора и координата заглушки в карточке json_commands. Артефакты scratch/z7-docs-scoped/final и z7-comment-checkpoint.

Checkpoint объединяет только связанные #252 исходники, тесты, Docs и IDEA; ReNum.cpp исключён. Без push и закрытия issue; сохранение PLN не выполнялось. Остаток: четыре трапеции, неизвестные основания и прочие явно неподдержанные случаи; решения за владельцем.


## Пункт 3 #252 — детерминированное разбиение отделки (AC25 Windows)

2026-10-11: старые beam-gap/beam-gap-repeat — offline RED по ориентированным native-контурам; у base536DEA6F три тела меняли вертикальный шов на горизонтальные полосы при практически одинаковом покрытии. Реальная историческая причина изменения native-входов не установлена: точные production snapshots не сохранены. Чувствительность per-piece BuildSurfaceWalls воспроизведена новым C++ regression. GroupSurfaceWallPieces выполняет union до прежнего level/interval sweep только для представимых стен с одинаковым полным набором source-face provenance, zoneBody и направлением нормали; обе polygon-разности проверяются, произвольные structural components не объединяются. При отказе выход grouping не меняется.

Canonical BuildAddOn.py25 Windows Debug compile/link success; fresh direct LSP без новых диагностик; self-tests60/60, восемь новых regressions, актуальная Отладка ERROR IN TEST=0. Runtime deterministic/deterministic-repeat:63/63=47стен+16перекрытий, удалены ровно63 предыдущих GUID, созданы63 новых. Независимый ориентированный native-contour multiset совпадает с округлением5знаков и попарно в неизменном допуске1e-6м; max corresponding drift6.5868125745221e-7м. Покрытие трёх крупных стен и шести контактных плоскостей двух балок проверено в обоих запусках;126 base/zone links,63 layer-properties/group0,15 исходных GUID и hashes двух saved PLN сохранены. deterministic-final-summary.json/deterministic-final-readback.py; APX SHA2564f4fb65acf699f79db6669d63931fd6a9f7f6cf965074fe1c122e3ed3b9f95c4. Архикад оставлен с повторным результатом в VS Run, своих точек нет. Docs/generated, final runner, commit/push не выполнялись по ограничению владельца; #252 этим не закрывается.

Пункт2 остаётся частичным: исторические31пропуск=22Unmatched(419.699797722м²)+4диагональные трапеции колонн(2.375999498м²)+2микрополосы(0.000001419м²)+3торца внеscope(0.143677025м²). Синхронные source/zone снимки опровергли wall-owner у60/71: сохранённые старые грани и актуальные вырезы стены FA9703B0 различаются, контакт со старой геометрией не устанавливает сегодняшнее основание. Ни bbox, ни изменение slantAlpha не представляет четыре трапеции в действующем scalar writer; это не утверждение о невозможности SDK. Расширение writer требует отдельного решения; неизвестным baseGUID и двум микрополосам ложных элементов не назначено. Существующий runtimecounter31 сохранён, полного покрытия всех контактов не заявляется.

## Примыкание балки к профильной стене #252 — AC25 Windows

2026-10-11: BuildRoomBeamFinish получает surfaceSnapshot зоны; пересекает реальные конечные грани балки с сечением замкнутого 3D-тела. beamPart выбирает балку/связную часть, не ограничивает все высоты нижним контуром. Устранён зазор 0.250107 м: прежний missing низа 0.075033 м² и каждого бока около 0.155835 м²; верхняя полоса учитывает следующую ступень профиля. Независимый check-beam-junction.py: baseline RED, beam-gap и beam-gap-repeat GREEN по всем шести контактным плоскостям двух балок. Предварительный вывод о лишней отделке оказался ошибкой checker-а: были включены торцевые грани толщины перекрытий/стен; типы контактных элементов исправлены, baseline перепроверен.

Новый тест сначала упал при подготовке пустой служебной грани: VS ExceptionThrown, GS::Array this=0x8, TestRoombookGeometry.cpp:399. Небезопасный доступ устранён проверкой пустых контуров/вершин; повторная canonical build и новые self-tests 52/52 Passed, обе новые проверки ok в актуальной Отладка, ERROR IN TEST=0. Fresh direct LSP version1: TestRoombookGeometry0; RoomSurfacePartition один прежний unused-includes; Roombook13 прежних диагностик. Создано63 элемента (47 стен,16 перекрытий), повтор удалил63/создал63,126 links подтверждены;63 element-layer properties и intersectionGroupNr=0 прочитаны;15 исходных GUID сохранены; оба saved PLN hashes unchanged. Макс. gap контакта4.58e-7м. BodywiseGeometryEqual=false, покрытие обоих запусков подтверждено независимо. Открытые/разветвлённые сечения и внутренние отверстия beamPiece — явный атомарный отказ, не bounds-подстановка.

Артефакты scratch/z7-writer/beam-gap{,-repeat}-{beam-junction,wall-coverage,verification}.json, beam-gap-final-summary.json. APX SHA-256 b121cd842148b0248298896070d66a1a3dccc929b051267b7b073f21efb01f79. Archicad оставлен в VS Run с последним результатом. Docs/generated/final runner и commit/push не выполнялись по прежнему ограничению владельца; #252 OPEN. Следующее действие — оставшиеся торцевые стеновые участки по отдельному запросу, PLN не сохранять.

## Этап 1 #252 — типизированный reader и снимок OtdRoom (AC25 Windows)

Status: COMPLETED в проверенном объёме; #252 этим не закрывается.

Первый этап утверждённого постепенного переноса: общий типизированный reader 3D-поверхностей, снимок в OtdRoom и вызов из CollectRoomInfo. Issue: https://github.com/kuvbur/AddOn_SomeStuff/issues/252.

#### Decisions

- Реализацию сейчас писать по SDK AC25 и проверять только AC25 Windows. Адаптация нового кода к остальным версиям — отдельный последующий этап. Это не разрешение возвращать #if defined(AC_25) или заглушки для остальных версий. Существующие средства совместимости не расширять без необходимости для текущего этапа.

#### Scope

AC25 Windows; Roombook.cpp/.hpp, общий reader в Sources/AddOn/, TESTING-диагностика и непосредственно связанные Docs/generated. Старое создание отделки/материалы/Sync не переключать, PLN и исходные конструкции не менять. Следующие этапы обязаны разбивать каждую исходную грань на участки с единственным основанием и включать балки; здесь выполняется только получение/хранение. План: .hermes/plans/2026-10-11_115207-roombook-zone-surfaces.md.


- [x] Проверить reader/SDK/текущее состояние и актуализировать задачу.
- [x] Выделить типизированный reader, добавить снимок OtdRoom и вызов CollectRoomInfo по SDK AC25. Реальное хранение 37 граней зоны подтверждено; ограничения AC_25 убраны, адаптация других версий отложена.
- [x] JSON-диагностика использует тот же reader: 16 GUID совпали с legacy baseline и повтором; снимок OtdRoom повторяется точно, ошибочный GUID/тип зоны отклоняются. 28 явных C++ transform checks Passed. SDK error/deleted/multiple-body fixtures not verified.
- [x] canonical AC25 build/runtime, formatting and compile DB validated; fresh direct LSP: reader/diagnostic/registrar/command0; Roombook13 inherited diagnostics, severity/code/message unchanged from Z4.
- [x] scoped Docs/generated accepted by parent:1114symbols/555edges;467raw MCP ranges/32successful anchors;908unrelated symbols/415edges/order preserved. Review: No issues found in inspected scope.

Canonical AC25 Windows Debug compile/link success после добавления C++ transform tests; финальный APX SHA-256 fdb0d8750c476b80554d96b9002f38b90b78cf98d22b7f5331521fe3dbe6a827, реально загружен из Debug (VS Run, PID38880, порт19723, своих breakpoint нет). Runtime: 37 граней зоны сохранены в OtdRoom; 16 GUID legacy/repeat identical; 28/28 C++ transform checks Passed и 28 ok-маркеров в текущей панели Отладка. Проверены 125 полигонов/594 world-вершины/4 отверстия, max plane gap4.809999989419111e-7 м при пороге1e-6. Диагностика отделку не создаёт; доступный API inventory14 GUID before/after совпал. Source/build PLN hashes unchanged, модель не сохранялась. SDK failure/deleted/multibody fixtures и другие версии not verified. Артефакты scratch/z5-roombook-surfaces-20261011-121537/runtime-current/final-summary.json и validate_saved_runtime.py (PASS). #252 не закрыта; commit/push нет.

Evidence: scratch/z5-docs-scoped/parent-validation.json, parent-lsp/, raw-manifest.json; current source hashes match collected/runtime artifacts. No new checkpoint; HEAD c323a2f, no commit/push.

## Предыдущий TESTING-этап Z4 — историческое состояние

**Текущий приоритет (новый запрос владельца):** строить отделку по уже полученным плоскостям и реальным контурам, AC25 Windows. Остатки Z3 не блокируют геометрический этап; Unresolved/Ambiguous структурная принадлежность не подменяется покрытием.

**Scope построения:** максимально переиспользовать OtdWall_Add_One → OtdWall_Draw → OtdWall_Draw_Wall и существующее получение избранного/настроек, отметок, покрытий и связей. Владелец разрешил отдельный новый слой только для отделки с группой пересечения0 и проверку всего батча; существующие слои не менять. Минимальное расширение OtdWall/writer для наклона, явный TESTING-драйвер полученных плоскостей и связанные тесты/документация; исходные конструкции не менять. Morph, новый GDL и второй writer не добавлять (решения владельца).

**Plan построения:** [x] выбор владельца: штатные наклонные стены; [x] минимальный TESTING-адаптер прежнего writer; [x] отдельный слой group0:24/24 точных тел,2наклонные; [x] штатная классификация/безопасная запись GUID зоны/основания:48/48 фактических свойств, повтор24тел/48свойств; [/] наблюдение Undo/Redo.16непрямоугольных/горизонтальных контактов явно пропущены; обычный RoomBook на3D-контакты не переведён.

**Status построения:** IN_PROGRESS / PARTIAL, 2026-10-11, AC25 Windows. Linked-batch24/24 точных3D-тел на SomeStuff surface finish (no intersections), group0: контур/плоскость/сторона/нормальная толщина1мм/оба surface override;2наклонные. Max gap2.3820259009710298e−7м, symdiff1.622498112887025e−8м², пороги1e−6. Прежние7превышений исчезли при изоляции, после принятия коллинеарных вершин24кандидата/16явных пропусков. writeLinks использует Class_FindFinClass/Class_SetClass и SyncSetSubelementScope→ElementsWrite только новых GUID, без полного SetSyncOtdWall/SyncArray. API.GetPropertyValuesOfElements подтвердил48/48 настоящих zone/base GUID со status=normal; повтор24тел/48свойств PASS. Undo/Redo not verified. По запросу владельца новый батч повторён под VS Debug: PID39760/Run,24тела/48свойств и повтор PASS.

**Next Step построения:** debug оставлен запущенным (VS Run, PID39760/test_25),24созданные стены доступны владельцу; своих breakpoint нет. По следующему запросу продолжить наблюдение Undo/Redo, сейчас not verified. Не сохранять PLN и не повторять создание автоматически. Новый подтверждённый запуск: scratch/z4-native-finish/debug-new-way/{create,verification,repeat-verification,debug-run-summary}.json.

**Last Checkpoint построения:** нового нет; основа c323a2f, roombook_refactor; commit/push не разрешены и не выполнялись. Canonical AC25 BuildAddOn compile/link success, APX SHA-25643f5b1c990abca45bec243a7a92b23da6ed9a819f10b5fe5949c5ff128dc6ded, реально загружен из Debug PID40136. Fresh direct LSP diagnostic/command0, Roombook прежние13; fresh MCP/generated1065symbols/522edges,195endpoint checks/126sites,417посторонних edges/order сохранены. Два inherited external endpoint stale отмечены в meta, не исправлены. Startup4001 устранён после чтения orphan-lock диалога и безопасного обратимого перемещения свободного LCK; профили не трогались. Runner пропущен по согласованному исключению, #252 OPEN. Новый VS debug запуск: breakpoint660 реально hit, isolateFinish/writeLinks=true, walls=24; вызов из RoomBookCommand::Execute:42. Точка удалена, Run сохранён; APX SHA тот же, C++ не менялся и новая canonical сборка не запускалась.

**Scope Z0/Z2/Z3 (предыдущая диагностическая фаза):** AC25 Windows, TESTING-диагностика `tests/TestRoombookGeometry.cpp/.hpp` и её регистрация; read-only finite-contact/material prototype в Hermes scratch. https://github.com/kuvbur/AddOn_SomeStuff/issues/252. Production Roombook/Helpers/Sync и модель — read-only. Покрытие не определяет skin/component; неразрешённая принадлежность остаётся явным статусом.

**Plan Z0/Z2/Z3:** [x] Z0 SDK-обход/диагностика; [x] конечные контакты, отверстия, сегменты Z0.6/1.2 и короткая колонна до Z3.0; [x] расширенный экспорт UMAT/body.parent/profile hatch+edge/skins/overrides/veneer, AC25 canonical build и fresh direct LSP; [x] real surface resolve 36/36, basic-segment join13 и profile-wall join6, явная неоднозначность1; [x] scoped docs/generated и воспроизводимый offline artifact; [/] actual segment-profile placement/ownership для16 фрагментов профильных колонн; [ ] runtime skin mapping многослойки и активных override/veneer на адресной фикстуре. Production не реализовывался; #260/P4t остаётся на паузе.

**Состояние:** Z3_PARTIAL, 2026-10-10. После согласованного перезапуска итоговые16 real JSON и их16 повторов идентичны. 37граней зоны/72источника/36контактов:7Matched/16Partial/14Unmatched; contactArea94.09417686075932м². Реальные UMAT дали36 `ResolvedAttribute`; structural statuses13 `ResolvedBasicSegment`,6 `ResolvedByInstanceProfileGeometry`,16 `Unresolved`,1 `Ambiguous`. Ambiguous component area1.9282988833881512e−7м² сохранена, не сглажена. Положение instance-профиля стены принято только для этого fixture по3полным сечениям, зеркальный кандидат отвергнут; компоненты0/1 различены при одинаковом surface5. Assembly `stretchedProfiles` колонн пусты — это не доказательство отсутствия SDK-пути. Composite/active override/veneer не представлены в runtime fixture, их экспорт скомпилирован, поведение not verified.

**Validation Z3 (историческая):** final canonical `BuildAddOn.py -v25` success с реальным compile/link; APX SHA-256926b67ce156ba49d2b01663d3100c2a11d84c0c8f60880c613d39d4c8ad78004, загружен из `Build/SomeStuff/25/Debug`. Fresh direct AC25 publishDiagnostics пусты у diagnostic/registrar; выполнены finite-contact self-tests и реальные/synthetic-negative offline checks. Полный TestKit в этом ходе not verified; runner не запускался по согласованному исключению. VS Run/PID956, собственных точек нет. Оба saved PLN равны baseline: build3b25c965…/source6e06c1c6…; BuildAddOn временно заменял build-copy старым Test_file, перед запуском восстановлена точная резервная копия, не сохранение модели. Exact zone-contour equality после перезапуска не заявляется:4контура отличаются, Matched→Partial одной микрополосы; итоговый повтор стабилен. Все15 source-geometry snapshots совпали после cyclic-start normalization. Причина zone numeric difference not verified.

**Next Step:** продолжить отдельно текущий остаток Z3: установить и проверить actual-профиль каждого сегмента и его размещение, затем геометрический join; не присваивать компонент по attribute-template/body/surface. Для многослойки/активных override/veneer нужна адресная runtime фикстура. Артефакты: `scratch/z3-material-20261010-200402` (raw+repeat, scripts, result, build/LSP/source hashes); раздаваемый `scratch/z3-material-diagnostic.zip`. Production/PLN не менять, runner не запускать без нового согласования; #252 не закрывать. Дополнение async-исследования, проверенное родителем на final JSON: у первых profile-сегментов F9B99FD9…/5D5BE0DD… верхняя X-граница сдвигается на0.9м за Z0..0.6 — rigid extrusion для них недопустима; механизм деформации not verified. Начать rigid-placement проверку с E4441373-3950-447E-8BD3-08397B26C89B (Z0.6..1.2, крайние XY-bounds совпадают в1e−6м), но равенство bounds ещё не доказывает постоянное сечение. Нужны actual segment profile и полные промежуточные сечения; deforming-сегменты оставлять Unresolved. Исследовательский отчёт scratch/z3-profile-placement.md относится к прежнему exporter-у; его вывод об отсутствии любого verified join не переносить на final wall/basic joins. Parent evidence: z3-material-20261010-200402/profile-placement-review.json.

**Last Checkpoint:** нового нет, commit/push не разрешены и не выполнялись; основа c323a2f. Незакоммичены TESTING cpp/hpp, registrar, связанные docs/generated и этот блок IDEA.md; остальные изменения IDEA.md сохранены. Generated1064symbols/513edges:19diagnostic symbols/25edges сопряжены с финальным кодом,1045посторонних symbols/488edges и порядок сохранены; полнота чужого графа не заявляется.

---
## Комментарии функций Roombook

### Status / Scope
COMPLETED — русские комментарии в Sources/AddOn/Roombook.cpp, без изменения кода/сигнатур/предсуществующего рефакторинга. Добавлены89 блоков;110определений покрыты новыми или сохранёнными описаниями. AC22–29 условные ветви описаны по исходнику; LSP проверен для AC25 Windows.

### Plan / Validation
- [x] C1. Прочитаны определения, сохранён точный снимок текущего дерева.
- [x] C2. Комментарии применены, все исходные строки/разделители/баннеры сохранены.
- [x] C3. Clang-format новых строк, code/literals идентичны снимку, fresh LSP без новых диагностик, diff/coverage/generated source joins проверены.
131symbols/126endpoints/71sites сопряжены с кодом;108active functions совпали с fresh direct LSP.914unrelated symbols/418edges сохранены. Сборка/runtime не выполнялись: правка только комментариев. Shared MCP AST stale; независимый direct LSP использован вместо него. Снимки и verification.json — Hermes scratch/roombook-comments/.

### Next Step / Last Checkpoint
По комментариям действий нет. Рефакторинг#260 отдельный, не продолжался автоматически; состояние прочих потоков сохранено. Checkpoint по запросу владельца: [C3] Добавить комментарии функций Roombook; Refs: IDEA.md step C3. Push не запрошен.

---
## #148 — глубина и направление дверных откосов (AC25 Windows)

### Status
COMPLETED: владелец принял результат и запросил commit/закрытие issue. https://github.com/kuvbur/AddOn_SomeStuff/issues/148

### Scope / Solution
Roombook.cpp/.hpp, TestCore.cpp и карточка Roombook; ветка roombook_refactor. Признак refSide переносится до нормализации слоёв. Глубина со стороны sill — sill, противоположная — base_th − sill − frame. Шесть GDL-флагов и подрезка размеров по штукатурке удалены. При построении верхнего откоса стеной из избранного разворачивается линия; боковые и ветки настоящей балки/объекта не менялись.

### Validation
AC25 BuildAddOn success; свежий TestKit84/32650/0 EXIT0. Контрольная дверь 8860F8B4-4A67-4113-B323-B1E4A4D1B15E: боковые интервалы y=[15.25,15.37] и[15.11,15.15], верхние после RED→GREEN — [15.2505,15.3705] и[15.1095,15.1495]. Глубины120/40 мм, направление в проём подтверждено JSON read-back. Смещение линии привязки0.5 мм сохранено. APX SHA-256 f25f2f1a2c7a9de9e59bbb2f133f7521797ed92297b52d586927e7e345fef14e. Финальный обязательный runner07.10.2026: exit0/build=True/Archicad running; JSON-тесты пропущены из-за отсутствия Tools/test_json_commands.py. Успех runner не принимается за новый модельный прогон; модельный GREEN получен ранее на том же коде. PLN не сохранялся, SHA-2566e06c1c67cd7418c7901e306b3923bf89bae9211ebf9f4d63d1bec424d3dce88 сохранён.

### Not verified / Decisions
Свежий LSP/generated из-за stale MCP AST, все реальные библиотечные варианты окон/дверей, другие версии/платформы — not verified. Закрытие принято владельцем при этих оговорках, полная формальная проверка D148.5 не заявляется. Docs/_progress.md с параллельной правкой не включать в checkpoint. Предыдущий модельный прогон содержал126 диагностик gdl:tip_pot вне фикса; отсутствие всех production-ошибок не заявляется.

### Last Checkpoint / Next Step
Основной checkpoint: 7d0874b — [D148] Исправить глубину и направление откосов; Refs: IDEA.md step D148.4, D148.6; Refs: #148. Issue #148 CLOSED — подтверждено чтением GitHub после закрытия по запросу владельца. Next Step: нет по этой задаче. Push не выполнялся; дальнейшие изменения только отдельной задачей.

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

## 2026-10-23 — Перенос закрытых блоков из IDEA.md при компакции

Ниже — блоки, удалённые из активного `IDEA.md` как закрытые/завершённые и неактуальные. Тексты сохранены без изменений.

## Исследование первичной настройки отделки через Python/JSON — 2026-10-22

Scope: read-only аудит текущего Roombook и штатного JSON AC25 для свойств/классификаций/избранного/комбинаций слоёв; скрипт `python_scripts/roombook_setup.py` (проверка + отчёт) создан по явному запросу владельца, C++ не трогается, build/запуск отделки и изменения модели — вне scope.

Status: COMPLETED. Результаты live-проверки (порт 19723, проект с частичной настройкой): свойства — 8/24 маркеров some_stuff_fin_* найдено в описаниях 940 свойств UserDefined (найдены *_result-маркеры, отсутствуют has_*, материалы, высоты, type_*); классификации — 7/10 классов some_stuff_fin_* найдены, отсутствуют windows_reveals, doors_reveals, beams; системы some_stuff_fin_side/surface присутствуют. Избранное и комбинация слоёв smstf_fin: штатный JSON AC25 перечислять не умеет — требуется аддонная команда (кандидаты: ACAPI_Favorite_GetFavorites, ACAPI_LayerCombination_GetList). Аддон SomeStuff на обоих портах не загружен (Health 4010) — аддонные команды не проверялись live.

Last Completed: скрипт создан и запущен на реальном проекте; исправлена путаница в сводке классификаций (было len(found)=114 вместо числа совпадений 7).
Next Step: создание отсутствующих 17 свойств и 3 классификаций через штатный JSON; аддонная команда для избранного/слоёв — отдельная задача.
Last Checkpoint: не выполнялся (python_scripts не коммитился; по команде владельца).

## Закрытие issues по команде владельца — 2026-10-22

#287, #288, #272, #280, #286 и #195 закрыты как completed; состояние каждого подтверждено повторным чтением GitHub API. Эта запись имеет приоритет над историческими OPEN/«не закрыта» ниже. Закрытие по решению владельца не означает завершения всей ранее указанной приёмки: #287 — исходный аварийный сценарий и AC28 runtime; #288 — полная геометрия/материалы/ownership links/отказы; #272 — сравнительное ускорение и полная текущая приёмка остаются not verified. #280 и #286 имеют ранее выполненные адресные проверки; #195 закрыта как исторический этап, продолжение рефакторинга — #260.

Финальный runner остановлен защитой multiple_archicad_processes, exit20: build=False, тесты не начались, процессы не закрывались. Закрытие выполнено с перечисленными оговорками, не как подтверждение полной приёмки. C++/PLN не менялись. Last Checkpoint:0795e2d; новый commit/push не выполнялся. Next Step: продолжать только оставшиеся задачи по новой команде владельца; закрытые issues автоматически не возобновлять.

### #288 — обновление отделочных стен и откосов на месте

Проверка 2026-10-21 после сообщения владельца «Подгрузил библиотеку, проверь» — COMPLETED в scope библиотеки/повтора; общая #288 PARTIAL_RUNTIME_VERIFIED. В собственном test_25/PID48004, порт19723: «плитка» libInd4919, OtdWall_Draw_Object после ACAPI_Element_Change вернул err0; исходный GUID3EC8D9DB-F46C-41DF-95A6-55DD9C0E5F40 прочитан после обоих RoomBook. Два штатных RoomBook завершились без сообщений rollback/нулевой библиотеки; GUID-наборы всех79 Wall и58 Object одинаковы до/после/повтора. Текущие finish-классы:62 Wall и56 Object; в каждом запуске18 Slab и12 Window заменены новыми GUID при неизменном общем составе188 элементов. Это не независимая приёмка всей геометрии/материалов/ownership links; Beam/Roof отсутствуют в фикстуре. C++ не менялся. VS F5 реально пересобрал Roombook.cpp и APX Debug x64 (Сборка16:48, failed0); отдельный canonical BuildAddOn не запускался. Loaded APX SHA2567372f522899fc28a3d41b95225aa11224b4d9c9d1e281767c7eea82f41cebfd6; build-copy fixture ee39d093319c24f6d56c066738c32bc6588cba2a12dfe7dc6ee806f951821a6a. Все три сохранённых PLN byte-identical исходному снимку, source identities неизменны. Evidence: Hermes scratch/finish-reuse-library-1792590514088529500/{before-manifest.json,loaded-binary.json,tile-debug-observation.json,types-before.json,types-after-first.json,types-after-repeat.json,classifications-after-repeat.json,verification-summary.json}; Отладка spillover/call_fVBXGLtnlf8yFuRtnyGq5q3O.txt. Собственная VS-сессия остановлена, свои точки удалены; исходная Propertycache.cpp:1011 восстановлена enabled (подтверждено свежим списком). Last Checkpoint6e54fa2 без изменений, commit/push/закрытие не выполнялись.

Дополнение 2026-10-21, сообщение об ошибке — WAITING_FOR_BUILD_AND_RUNTIME. Scope: только английская диагностика Object с libInd=0 в OtdWall_Draw_Object, без смены writer/создания/отката; issue #288, комментарий6317506685. Plan: [x] добавлены имя избранного, причина и действие, GUID основания передаётся в существующий msg_rep (Roombook.cpp:4941–4950); [x] source assertions: остальной текст файла и ordered ACAPI inventory совпадают с исходным снимком, English literals/Favorite/base GUID проверены, clang-format и git diff --check PASS; [ ] свежий LSP; [ ] canonical AC25 build и отображение сообщения в host. MCP вернул0 diagnostics, но AST устарел (OtdWall_Draw_Object5100 вместо4802) — LSP not verified. Активная VS Run-сессия сохранена, build/runtime не выполнялись; C++ ошибка не воспроизводилась заново. Evidence: Hermes scratch/roombook-error-message/{Roombook.before.cpp,own-delta.diff,validation.json}, SHA25675374a908ddfc25e4bd5624d1bb51a6898a3f8926cc582b27753db909a78d24b. Last Checkpoint без изменений, commit/push не выполнялись. Docs/generated остаются отложенными согласно общему Scope. Next Step: после безопасного завершения текущей отладки — canonical build AC25 и проверка английской диагностики на проблемном Object; общий откат и иные ошибки не изменены.

Scope: AC25 Windows; окончательный расчёт без изменений, pool matching зона/база/эффективный класс/совместимый native writer, существующие поля otd_guid, Wall/Object/Beam и представимые surface-стены. Проёмы пересоздавать один раз на стену. Семантическая привязка GUID к конкретной стороне окна внутри общего класса не обещается. Незавершённая #271, исходные конструкции, сохранённые PLN и чужие правки вне scope. Issue: https://github.com/kuvbur/AddOn_SomeStuff/issues/288. План: .hermes/plans/2026-10-19_175905-roombook-existing-finish-update.md §6. Коммит/push/закрытие только отдельной командой.
Историческая проверка 2026-10-20 (блокер GDL снят новой проверкой 2026-10-21 выше). Реализация pool matching и update Wall/Object/Beam присутствует; stale GUID не публикуется, ошибки Wall/Beam/Object и удаления доходят до undo scope. Canonical AC25 Windows Debug compile/link PASS на снимке final-build-source-manifest.json; native finishReuse38/0, transform28/0 и partition301/0. Проверены GUID стены/балки, изменённые native-параметры, повтор с двумя новыми GUID проёмов без накопления, удаление старых проёмов при пустом новом наборе и откат временных элементов. Полная приёмка #288 НЕ выполнена: штатный RoomBook откатывается на Object-избранном «плитка» (favoriteError=0, libInd=0, libpartError=-2130313112); до/после отказа одинаковый набор134 GUID. Подмена Object другим writer не выполнялась.
Plan:
- [x] Перепроверить исправленный план, сохранить исходное дерево, создать/прочитать issue #288.
- [x] Реализовать one-to-one pool matching окончательных результатов и update writers; compile/link и адресные Wall/Beam/проёмы GREEN. Native Object остаётся отдельным пунктом приёмки ниже.
- [/] Адресные проверки и clang-format/canonical AC25 build PASS; свежий CLI clangd прочёл текущие TU: в двух тестах source diagnostics0, в Roombook семь прежних unused-variable. Полный --check exit3 из-за внутренних ExtractFunction probes; shared MCP AST теста устарел. Не выдавать это за полностью GREEN LSP.
- [/] Валидная библиотека, штатный RoomBook+repeat, native Object и GUID/class inventories проверены2026-10-21. Остались независимый read-back геометрии/материалов/ownership links, оставшиеся отказные пути и полная scope-приёмка.
Evidence: Hermes scratch/finish-reuse-20261020/{final-build.log,final-build-source-manifest.json,finish-reuse-native-final.json,final-transformSelfTest.json,final-partitionSelfTest.json,favorite-tile-read.json,validation-summary.json,fresh-clangd-result.json}. APX SHA25682bb23c568830e0a228ca067fba849297997fd3c51cdf950425256b68ca61236. Structured38/0; в Отладка найдены37 из38 подписей (single segment beam can be reused отсутствует), поэтому лог — частичное подтверждение, не отдельный полный итог. Ошибка новой used.Set при отсутствующем ключе подтверждена стеком/SDK и исправлена на Add; последующий полный вызов прошёл matcher, но откатился по указанным данным GDL. Saved PLN не сохранялись; исходная build-copy aa019da3 сохранена в fixtures. После проверенного запуска Roombook.hpp и build-copy PLN изменились относительно manifest: текущий общий снимок не объявлять проверенным и не восстанавливать поверх новых изменений. Собственная VS-сессия остановлена (hosts0), собственная точка удалена, Propertycache.cpp:1011 восстановлена enabled.
Last Checkpoint: 6e54fa2 — существующий HEAD, нового коммита нет; commit/push/закрытие не запрошены. Docs/generated отложены согласно Scope.
Next Step: библиотечный блокер снят; для общей приёмки #288 — canonical AC25 build на согласованном снимке, независимые geometry/material/ownership-link и отказные проверки. Существующий floor/Mesh-маршрут не переписывать; не повторять поиск отсутствующей библиотеки по историческим отчётам.

Диагностика запроса владельца 2026-10-21 «Finish update rolled back APIERR_BADPARS»: в текущей панели VS «Отладка», последняя сессия 16:29:40–16:32:54, непосредственно перед откатами (в том числе16:32:02) присутствуют `APICreate_SL :: APIERR_BADPARS :: bad library part, index is 0` и `OtdWall_Draw_Object err: ACAPI_Element_Create`. Это подтверждает непосредственный отказ создания Object из-за нулевого индекса библиотечной части; не ошибка группировки/undo. Live source: Roombook.cpp:6525–6529 копирует favorite.element; :4958–4960 принимает Object только по типу без проверки libInd; :4811/:4929 получает шаблон и передаёт его в Create без присваивания libInd. Историческое имя «плитка» и favoriteError0/libInd0 подтверждены сохранённым favorite-tile-read.json от20октября; имя текущего отказавшего Favorite отдельно не считано и не выдаётся за новое live-наблюдение. На момент проверки VS Design, ArchicadCount0; новую модель/host не запускали, production/PLN/breakpoints не меняли, build/test не выполняли. Evidence: Hermes spillover/call_DjgTSEFvtM4MLqtVGzpGAsIt.txt; scratch/roombook-badpars-rag.json (SDK examples, не проверка текущего проекта). Классификация: невалидный Object-шаблон/библиотечная ссылка; почему библиотека не разрешается (не загружена либо неверная ссылка избранного) — not verified. Дефект не объявлен исправленным. Next action: в исходном проблемном проекте восстановить/перевыбрать GDL-объект избранного, затем штатный RoomBook+repeat и read-back; не подменять Object другим writer. Last Checkpoint без изменений, коммит не выполнялся.

### #268 — прямоугольные пропуски в углах полов

Scope: AC25 Windows; только FloorCoreGeometry::Extensions и общие standalone/host проверки FloorCoreChecks.hpp. Стены, зоны, потолки, writers, параметры слоёв и прочие незакоммиченные изменения не менять. Docs/generated и commit/push отложены по текущему Scope. Issue: https://github.com/kuvbur/AddOn_SomeStuff/issues/268 (CLOSED; регрессия регистрируется комментарием, статус без команды владельца не меняется).
Status: WAITING_FOR_BUILD_AND_NATIVE_TEST. Классификация: implementation error. Подтверждён и минимально исправлен микросегмент с нулевым shift перед miter: принятые контакты длиной > Tolerance привязываются концами к вершинам ребра только в пределах неизменного Tolerance=1e-6 м (FloorCoreGeometry.hpp:99–109). Короткие контакты отсекаются до нормализации; реальные разрывы и неоднозначность сохранены. Связь с конкретным пользовательским полигоном not verified.
Plan: [x] прочитать #268/source/history и воспроизвести на production kernel; [x] добавить GREEN/RED и минимальный fix; [x] format/LSP/standalone; [ ] canonical AC25 build; [ ] native-приёмка проблемного пола с повтором.
Validation: прежние 23 kernel checks PASS; расширенный suite до fix 31 PASS/10 FAIL, после fix 41/41 PASS. Дополнительный independent point-in-patch repro: точные концы и 1e-12/1e-9/5e-7 м закрывают угол, 2e-6 м сохраняет разрыв. clang-format dry-run и git diff --check PASS; MCP force_refresh RoomSurfaceFinish.cpp/TestRoombookGeometry.cpp — 0 diagnostics. Compiled: standalone clang++ C++17 -Wall -Wextra -Werror — yes; addon AC25 — not performed. Tested: kernel — yes; SDK/native — not performed. Evidence: Hermes scratch/floor268-corner-repro.cpp и floor268-corner-baseline/manifest.json. Docs/generated отложены по Scope; чужие source-правки не менялись.
Last Checkpoint: f86e0de — существующий, нового нет.
Next Step: после освобождения текущей VS Run-сессии — canonical AC25 build и проверка native-контура проблемного пола с повтором; сначала получить зону/пол с пропуском. Archicad/debugger не останавливали, модель не меняли. #268 остаётся CLOSED; новый коммит/push не запрошены.

### #272 — ускорение 3D RoomBook и API-кэши

#272 API-кэши — реализованы частично; прежнее заявление о полном завершении снято по результатам review ниже. Команда владельца: все кэши, кроме ACAPI_Favorite_Get и ACAPI_Attribute_Get.

### #272 — review реализации API-кэшей (AC25 Windows)

Scope: только добавленные Element/Classification/ZoneRelations/Connected caches и их production-callers. По команде владельца RC1–RC4 исправлены; AC25 Windows. План: [x] checked connected-result/error и полный ключ; [x] callers и итоговый отчёт; [x] targeted regression/format/LSP/canonical build; [/] общий host-прогон (один FAIL вне cache checks, описан ниже). Helpers/Favorite/Attribute и чужие изменения сохранены; нового checkpoint нет.
- Исправлено: RoomSurfaces.hpp/.cpp теперь кэширует raw GetConnectedElements по GUID/type/filter/variation/renovationFilterGuid вместе с error и исходными items, включая пустые/частичные ошибочные ответы. ReadConnectedElementsCached — общий production storage/helper, тесты вызывают его с callback без настоящих SDK queries. CollectRoomInfo читает Window/Door с прежними editable/access/workspace фильтрами, проверяет error и освобождает zonememo при отказе; void GetRelationsElement больше не используется. PrepareSource и индекс Window/Door используют ту же raw-читалку с renovation-only фильтром. Итоговый ReportRoomReadCounters вызван после ProcessRoomFinishes, до writes. connectedReads теперь соответствует одному реальному connected SDK query на miss. ReadZoneRelations по-прежнему кэширует только успешные snapshots.
- Validation исправления: canonical BuildAddOn.py -c config.json -v25 Debug success; APX SHA256 2f804c5c5b2c74b3c3f5527cf6f6df6303ad2782c8094464ae0d201b88f58415, все пять изменённых source/test файлов не новее APX. clang-format --dry-run --Werror и git diff --check PASS; MCP force_refresh RoomSurfaces/Roombook/TestRoombookGeometry 0 diagnostics. В уже работающем VS Run/PID56976/test_25 загружен Debug/SomeStuff.apx, API port19723 принадлежит этому PID. Нашими действиями Archicad не запускался/не закрывался; read-only RoomGeometryDiagnostic partitionSelfTest выполнил 12/12 новых connected-cache checks PASS (JSON и текущие DBtest строки панели «Отладка» подтверждены).
- Ограничение общего прогона: 247 checks, 246 PASS, 1 FAIL — «272 profile explicitly requests cached native fallback». TestRoombookGeometry.cpp:285 проверяет routeReason==ProfileWall, а текущий BuildPreparedWallSource при отказе печатает эту строку через DBprnt вместо присваивания routeReason (:234–247); cache-fix этот участок не менял. Профильную геометрию/чужие правки не исправлять в этой задаче. Общий selftest НЕ GREEN. Runtime native connected-query semantics, полный RoomBook final counters/геометрия/повтор и A/B not verified; callback-regression их не заменяет.
- Evidence: Hermes scratch/fix272-selftests.json и fix272-manifest.json; новая regression tests/ConnectedElementsCacheChecks.hpp подключена в RunSurfacePartitionChecks. Next Step: отдельно согласовать/profile-test FAIL и production host-приёмку кэшей; issue #272 остаётся открытой. Last Checkpoint: нового нет, commit/push/Docs/generated по Scope не выполнялись; две исходные disabled breakpoint не менялись.
- P2 / RC1: RoomSurfaces.cpp:421–436 безусловно кэширует результат void GetRelationsElement; Helpers.cpp:875–877 не возвращает ошибку GetTypeByGUID, :971–997 не передаёт ошибки Window/Door queries вызывающему. Пустой или частичный ответ после сбоя становится обычным cache hit, повтор не обращается к SDK. Минимальный fix: checked чтение с сохранением кодов ошибок/полноты, без изменения общего Helpers API; consumer должен явно отличать ошибку от успешного пустого списка. Фактический сбой SDK/потеря геометрии в runtime не воспроизводились.
- P2 / RC2: Roombook.cpp:479–484 (API_OpeningID в PrepareSource) и :595–598 (Window/Door) остаются прямыми GetConnectedElements. PrepareSource вызывается для общего основания в нескольких комнатах (:695), поэтому повторные Opening queries не устранены. Минимальный fix: кэш сырого результата с полным ключом GUID/type/filter/variation/дополнительных SDK-параметров и error; не смешивать renovation-only запросы с editable/access/workspace запросами Helpers.cpp. Отсрочка этих кэшей не была отдельным решением владельца; требование «все кроме двух» пока не выполнено.
- P2 / RC3: Roombook.cpp:727–756 печатает counters в PrepareRoomPartitions; позднее Otd_GetOtd_ByZone (:1561) читает classification cache, а ProcessRoomFinishes (:1423) вызывает ReadElement для surface walls (:1159) и beams (:1298). Отчёт не содержит этих поздних reads/hits и не является итогом прогона. Минимальный fix: итоговый отчёт после ProcessRoomFinishes, до writers; connectedReads сейчас считает вызовы helper, не число raw SDK calls (обычно type lookup + Window + Door).
- P3 / RC4: clang-format --dry-run --Werror FAILED для RoomSurfaces.cpp/.hpp (включая новый блок :358–439); прежнее общее format PASS неверно. Минимальный fix отдельным шагом: clang-format только нужных файлов, проверить diff на посторонние изменения. git diff --check для roombook PASS.
- Дополнительная сверка документации: ReadZoneRelations (:407–409) НЕ кэширует ошибки; таблица relations хранит только успешные RoomRelationValues, в отличие от elements/classifications. Не утверждать, что все helpers сохраняют SDK error. AddSurfaceFinishPiece/BuildRoomSurfaceFinishes находятся в Roombook.cpp, не RoomSurfaceFinish.cpp.
- Проверено без findings в этой границе: GUID-кэши локальны RoomProcessingContext одного RoomBook; явные writers идут после cached readers; error semantics IsFinish и Class_IsElementFinClass различаются и сохранены; value-поля wallPart/beamPart/grouped GUIDs скопированы до disposal (DevKit25 APIdefs_Elements.h:4904–4932 + LightRAG); nullptr-path имеет guards; Favorite/Attribute чтения остались прямыми.
- Validation: MCP force_refresh вернул 0 diagnostics для RoomSurfaces.cpp, Roombook.cpp, RoomSurfaceCandidates.cpp; это не runtime-проверка. Новая сборка/тесты/A-B в review не выполнялись; предыдущая canonical AC25 Debug сборка в этой беседе успешна. Регрессии новых четырёх helpers в TestRoombookGeometry.cpp поиском не найдены; отсутствие всех тестов во всём проекте не утверждается.
- Next Step: по отдельной команде исправить RC1–RC4 и добавить targeted regression cache/no-cache, errors/empty, shared source между комнатами, новый run и final counters; затем canonical build и host-приёмка. Last Checkpoint: нового нет, commit/push/Docs/generated не выполнялись, прочие задачи/правки сохранены.

### #272 — предыдущая запись реализации (уточнения review выше имеют приоритет)
**Добавлено/изменено:**
- RoomSurfaces.hpp/.cpp: общие читалки ReadElement/ReadClassificationItems/ReadZoneRelations/GetRelationsElement через RoomSurfaceCache; ElementRead/ClassificationRead/RoomRelationValues хранят точный SDK-результат, включая ошибки и пустые ответы; все обращения к таблицам защищены `cache != nullptr` (nullptr = прямое чтение без кэша, диагностический путь).
- Roombook.hpp/cpp: Otd_GetOtd_ByZone, Class_IsElementFinClass, OtdWall_Create_FromWall получили `RoomSurfaceCache*` (default nullptr); все production-callers передают `&context.surfaceCache`; CollectRoomInfo читает relations зоны через ReadZoneRelations (RelationGuard удалён — дублирование с ReadRoomSurfaceCandidates устранено общим value-снимком); отчёт «RoomBook source route» расширен счётчиками element/classification/relation/connected (reads/hits).
- RoomSurfaceCandidates.cpp: ReadBase и IsFinish — через ReadElement/ReadClassificationItems; ReadRoomSurfaceCandidates читает relations зоны через ReadZoneRelations; beamParts копируются из снимка до disposal.
- RoomSurfaceFinish.cpp: AddSurfaceFinishPiece и BuildRoomSurfaceFinishes (балки) — через ReadElement. Floor_FindInOneRoom — ACAPI_Element_Get через ReadElement.
**На что обратить внимание при review:**
1. Ошибки/пустые ответы кэшируются на весь прогон — неудачный ACAPI_Element_Get внутри одного run не повторяется; если какой-то путь полагался на повторную попытку, это изменение поведения.
2. RoomRelationValues — value-снимок (вектора скопированы), разделяется CollectRoomInfo и ReadRoomSurfaceCandidates; мутация через const& невозможна, но сверить, что нигде не снимают неконстантную ссылку.
3. ACAPI_Element_GetConnectedElements с фильтром типа (Roombook.cpp ~479/595) НЕ кэширован: таблица connectedElements ключируется только по GUID, а повторы только на стенах, граничащих с несколькими комнатами — нужен составной ключ (guid,type); отложено, не регрессия.
4. Не кэшировано намеренно: однократные чтения (зона в CollectRoomInfo, Opening_Create_One), ACAPI_Element_GetMemo (владеет handle'ами — нельзя хранить сырые), dead code (OtdWall_Create_FromColumn, Floor_Create_All/One), write/draw-стадия.
5. ACAPI_Favorite_Get и ACAPI_Attribute_Get не трогались — по команде владельца.
6. Формат отчёта «RoomBook source route» изменился (новые counters) — если отчёт парсится внешним скриптом, обновить парсер.
7. Диагностический caller TestRoombookGeometry.cpp:2345 использует default nullptr — компилируется и читает без кэша (проверено: обращения в RoomSurfaces.cpp защищены `cache != nullptr`, RoomSurfaceCandidates.cpp только пробрасывает указатель).
8. Runtime НЕ выполнялся: поведение в Archicad не проверено; canonical BuildAddOn.py AC25 Debug success, LSP 0 по 4 файлам, clang-format PASS. Скорость не измерялась (A/B — следующий шаг владельца).

#272 AC25 Windows, 2026-10-14: canonical BuildAddOn.py compile/link exit0; actual host selftests195/195 failed0, standalone analytic20/20 PASS. Исправлены четыре SDK FAIL объединением coplanar analytic cells в цельные contours с отверстиями; необоснованный TopLinkedWall fallback снят после live API/native проверки высоты3.3м и отдельной SDK-regression. Фактический маршрут: analyticWall569/slab24/faces3694, nativeFallback2, wall3DReads0/slab3DReads1, snapshotReads64/hits1, boundsReads0; coordinate calls1522/hits3220/refused0. Два штатных RoomBook:114.963с и120.374с для63комнат; API.GetAllElements4209→4209, повтор удалил2657GUID и создал2657. Это подтверждает native запись/повтор, НЕ независимое покрытие/идентичность каждого тела и НЕ контролируемый A/B benchmark. Последняя MCP LSP проверка Roombook/TestRoombookGeometry0диагностик; format/diff-check PASS. Production-прогоны выше относятся к предыдущему APX SHA2568e8dd987e9cf060cd1d75bd4412fafd9065a91467e430bde1fb23c6b74cd93f3; saved repo/build PLN hashes unchanged, результат повтора оставлен в VS Run без сохранения; четыре исходные breakpoint восстановлены. Evidence:scratch/272-parent-host/final-runtime.json, selftests-toplinked.json, all-first.json/all-repeat.json, build-toplinked.log. Диагностический consumer исправлен локально: ReadZoneSurfacePartition гидратирует PendingPreparedGeometry native-снимками, явно выдаёт geometryRoute=DiagnosticNative3D; production-discovery не менялся. Адресный RED PartitionSourceSnapshot→GREEN: зонаB69820A5 partitionComplete=true,12pieces,ambiguity0,errors0,4sources Extracted. Последняя canonical AC25 сборка build-diagnostic.log exit0, APX SHA256c7df1f97b15ec872800ad160710b5c56b2292469cc799ba128926910b7ea882d; её host195/195 PASS и штатный RoomBook115.6987875с для63комнат, actual wall3DReads0/slab3DReads1, coordinate calls1522/hits3220. Native inventory1608→4209,2657added/56removed; saved PLN unchanged, исходные4breakpoint восстановлены. Новый artifact:scratch/272-parent-host/diagnostic-final-runtime.json. Next Step: независимый native geometry/link audit и контролируемый A/B при необходимости; native-диагностику не считать доказательством analytic production equality (SEO/trims намеренно игнорируются). #272 OPEN; commit/push/Docs/generated не выполнялись по Scope; общего завершения задачи не заявлять.

#272 coordinate-cache реализован на обоих discovery-маршрутах, общий run-local helper сохраняет exact XY/Z/type/filter, ошибки/пустые ответы; NaN/inf не кэшируются. Родитель прочитал helper/diff и заново скомпилировал standalone production-helper regression clang++ C++17 -Wall -Wextra -Werror: exit0, PASS calls29/hits10/refused18/newRun1. Только callback/key/result tests, не SDK/runtime/скорость. Fresh child clangd baseline/final:8прежних unused, новых0; parent исправил новые English комментарии helper/test на русский и clang-format/diff-check PASS. Сборка аддона/host-приёмка НЕ выполнялись. Native→analytic Otd route ещё выполняется отдельным этапом; IDEА/Scope/прочие задачи сохранены, commit/push нет.

Решение владельца #272 после уточнения: SEO и подрезки обычных стен/перекрытий ИГНОРИРОВАТЬ в аналитическом маршруте. Это сознательная разница с native телом, не ошибка matcher; её не маскировать под доказанную native equality. Для обычных представимых оснований — finite prepared OtdWall/OtdSlab; для профилей сохраняется native reader/cache. Зона остаётся native 3D, реальные GUID/проёмы/состав/политика полов/потолков сохраняются. Scope теперь включает минимальное изменение порядка подготовки/partition и явное analytic provenance; остальные API-кэши отложены. Next Step: реализация analytic route + exact coordinate cache, regression/static checks; canonical build/runtime после освобождения VS Run. Без commit/push/Docs/generated, связанные незавершённые задачи сохранить.

Коррекция владельца #272: для обычных стен/перекрытий не читать native 3D-плоскости оснований; сопоставлять 3D-грани зоны с заранее подготовленными конечными OtdWall/OtdSlab. Native 3D оставить для сложных профилей/непредставимых случаев после проверки границ. Подтверждено по source: ReadBase Wall/Slab безусловно читает snapshot, partition идёт ДО ProcessElementsForRoomData, BuildRoomSurfaceFinishes очищает промежуточные массивы. Поэтому требуется перенос подготовки base representations до partition, без использования конечной отделки как основания. Расширение 3D-cache приостановлено; exact coordinate-cache остаётся полезным, остальные API-кэши отложены. Next Step: read-only проверка минимального analytic маршрута/типовых gates/provenance/material/openings, затем реализация по уточнённому плану. SDK/build/runtime нового маршрута NOT VERIFIED; без commit/push/Docs/generated, остальная работа сохранена.

Read-only аудит дополнительных API-кэшей #272: подтверждены повторные Element_Get (кандидаты/discovery/legacy/каждый surface-piece), GetRelations дважды на зону, классификации между этапами/зонами, Favorite/Defaults на каждый writer, mapping building material→surface, connected windows/doors на каждый найденный wall-edge и повторный slab-list. Наибольший алгоритмический кандидат — coordinate-search внутри probe×slab-level; одинаковые XY/Z/type/filter запросы можно объединять, разные Z/фильтры не смешивать. Favorite_ReadComposite и ElementsRead уже имеют batch/guard, Floor_Create_All/One и Favorite_GetType live production callers не найдены. Runtime-стоимость/приоритеты NOT VERIFIED, код аудитом не изменён. Следующий шаг по отдельной команде — staged кэши с сохранением ошибки/ownership/UnhideUnlock, затем измерение. Подробности: skill lightrag/references/somestuff-room-api-cache-audit.md.

#272 — AC25 Windows: реализация кэша выполнена в RoomSurfaces/RoomSurfaceCandidates/Roombook cpp+hpp; общий geometry-GUID cache на этап чтения, включая ошибки/пустые; body-local component cache; bounds используют полный cached snapshot либо vertex-only world-Z без полигонов. Система координат float bbox по SDK/LightRAG NOT VERIFIED, bbox не используется. Parent review шести delta и hash manifest PASS; fresh clangd reader/candidates без compiler diagnostics, Roombook те же8unused baseline; tweak failures не объявлены clean exit0. Format/diff-check PASS. Артефакты: Hermes scratch/272-implementation. Compiled/Tested: not performed — VS по-прежнему Break в PartitionRoomSurfaces:334, отладчик не трогали. Следующий шаг: после разрешения владельца освободить debug-сессию, canonical build, cache/uncached проверки и одинаковый native сценарий с измерением времени; скорость пока NOT VERIFIED. Копии snapshots между комнатами пока сохраняются. #272 OPEN; checkpoint/commit/push/Docs/generated нет, карточка Roombook пока не описывает новый cache.

PartitionInvalidSourcePlane, AC25 Windows: по решению владельца PlaneTolerance=1e-4 м (0,1мм, равен Roombook::min_dim); точечный DistanceTolerance/AreaTolerance не менялись. Устранена собственная регрессия малых граней: вектор площади нельзя сравнивать с линейным допуском; согласованная stored-плоскость сохранена, fallback Ньюэлла считается относительно origin. Canonical BuildAddOn compile/link exit0, format/diff-check и MCP LSP0 PASS. Live selftests156/156 failed0; зона0AB1934B-44F0-444A-93C5-F1A6424C2345: partitionComplete=true,17pieces,errors0 (раньше source4/body1/polygon94 имел20мкмнеплоскость). RoomBook returned96.61с, но Отладка подтвердил следующий blocker Unsupported beam geometry51037AF1-1476-4A3A-BB18-4201022519C0: создание всей отделки НЕ подтверждено. Next Step: отдельно диагностировать эту балку; не считать returned доказательством создания. По разрешению владельца debug-процесс закрыт без сохранения и перезапущен; свободный orphan build-copy lock обратимо перенесён в scratch/partition-lock-78512947-f35d-4ca0-8f7c-36b5e5825b49.lck после exclusive-check. VS оставлен Run, пять исходных breakpoint восстановлены. Evidence: scratch/plane-tolerance-{selftests,live,roombook}.json и spillover/call_988feb13a9c04e32b4bcf39f23d2b47c.txt. #252 OPEN; Docs/generated/commit/push по Scope не выполнялись; прочие задачи сохранены.

Текущий шаг Z9 (2026-10-13): read-only favoriteName exporter добавлен; BuildAddOn AC25 success, APX SHA256131478ba92212656591bafcac746f8641b104a253efb664aced7a60184af6699. VS Run/PID41460/test_25, breakpoint нет. favoriteName=«плитка» пока отвечает API4001 «ongoing user input»: следующий шаг — завершить диалог/ввод в Archicad и прочитать реальный libpart/контракт. Обе сохранённые PLN SHA256a12f5aed6de0d356bba5d279cef49ead94a69ac1409844638aaf841913977795 совпадают; PLN не сохранялся. Новый RoomBook не выполнялся; Object-writer и профильный зазор не исправлены, Z9 не завершён; commit/push нет.

Принять проверенный ортогональный вариант либо отдельно согласовать точное представление двух диагональных граней20.498776м² и прежних четырёх трапеций. Полного охвата всех контактов не заявлять. При разрешении обновить Docs/modules/Roombook.md и scoped generated: BuildSurfaceWalls теперь создаёт оболочку с проёмами, GroupSurfaceWallPieces объединяет до проверки представимости, добавлен LinkSurfaceOpenings. Z9 direction завершён:24адресных RED→GREEN после удаления повторного разворота Wall-fallback,два запуска335→335проверены. Следующий шаг общего Z9 по команде владельца «Заканчивай починку всех откосов» — полная матрица всех исходных окон/дверей и трёх откосов, классификация каждого отсутствующего/нестандартного контакта и проверка параметрического/material-rule контракта. Два независимых исследования уточнили:четыре верхних native-результата отсутствуют,пятый существует с40ммзазором на профильной стене;ещё два окна потерял прежний checker,полосы1мм закрываются торцами верхов. Live VS подтвердил у отсутствующих верхов draw_type=API_BeamID/favorite.type=API_ObjectID:они доходят до пустого OtdBeam_Draw_Object. Владелец выбрал сохранить Object-избранное и реализовать горизонтальный аксессуар,без Wall-fallback;контракт существующих object/slab writers исследуется отдельно до правки. Live избранное у Object-ветки:«плитка»,API_ObjectID,libInd516. Чтение char-массивов через VS evaluate не показало имена параметров;для установления точного libpart/контракта добавляется read-only TESTING-экспорт Object/Window library identity и scalar GDL параметров,не создание нового GDL. Важная поправка аудита:SDK25 type6=API_ObjectID,а не Beam;прежнее название40Beam в отчётах неверно. Для окна1A6C live Param_SetToWindows:frame=.05,sill=-.05,reveal_from_sill=false,base_th=.25;причина профильного reference/frame зазора ещё не установлена. SDK25 содержит top/ends overrides Beam,но writer их не назначает — минимальная исправляемая ошибка. Общий Z9 IN_PROGRESS;исправленная ошибка знака остаётся GREEN. Archicad оставлен в VS Run с повторным результатом, сохранение PLN не выполнялось; полный TestKit, Undo/Redo, принудительный отказ SDK и другие версии not verified.

### #287 — очистка API_Attribute перед чтением материала

Scope: только Param_SetComposite и полная инициализация локального API_Attribute ДО первого ACAPI_Attribute_Get; общий код AC25/AC28, выполненная сборка только AC25 Windows. По команде «коммить изменения» подготовить отдельный чекпоинт исправления и собственных crash-analysis записей; краткие Docs/generated обновляются в нём же. Посторонние незакоммиченные Roombook/тесты не включать и не менять; живой Archicad/Debug APX/PLN не трогать. Issue: https://github.com/kuvbur/AddOn_SomeStuff/issues/287. Push и закрытие issue не запрошены.
Status: WAITING_FOR_NATIVE_TEST. Минимальное исправление внесено: API_Attribute attrib = {}; перед первым чтением строительного материала. Координаты чекпоинта без постороннего WIP: Roombook.cpp:3999/:4002; текущего worktree:4175/:4178. Тело Param_SetComposite совпадает с базой после единственной инициализации. Причина в коде — implementation error; конкретная передача id→UniString через APICore остаётся not verified.
Plan: [x] 287-1 — issue/dedup, SDK-контракт, snapshot, RED2/4→минимальная инициализация→GREEN6/0 (четыре нулевых строковых указателя, typeID/index сохранены), clang-format/fresh MCP LSP0 и canonical AC25 Windows x64 INT/RelWithDebInfo compile+link exit0; [x] scoped Docs/generated и изоляция одного code hunk от постороннего WIP; [ ] native-проверка исходного сценария на исправленной загруженной сборке. AC28 build/runtime и полный TestKit не выполнялись.
Validation: выполненный BuildAddOn.py -c config.json -v 25 --release скомпилировал Roombook.cpp и создал INT/RelWithDebInfo/SomeStuff.apx SHA256 65b65caccb0021ff65ca30d4af573e14112b5bd242c52dc5325de60e5b5c809b. Это сборка рабочего дерева с прежними пользовательскими изменениями, не изолированного staged-снимка. Standalone SDK25 probe проверяет инициализацию, не воспроизведение падения/host API. Debug APX не менялся, host/runner не запускались, #287 остаётся OPEN. Evidence: Hermes scratch/finish-crash-20261019/fix-attribute-init/{before-manifest.json,regression/{red,green}-result.json,canonical-ac25-release.txt,build-evidence.json}; снимок пользовательских файлов перед частичным commit — scratch/commit287-20261019-133537/before-manifest.json.
Next Step: после согласованной загрузки исправленной сборки повторить проблемную операцию в Archicad; отдельно получить RPT/RPTA/DMP нового разрезного падения PID35784, не объединять его с #287 без доказательств. Last Checkpoint: этот чекпоинт [287-1], база f86e0de; точный hash — git log. Push/закрытие issue не выполнять.

### #275 — совместимость AC27–30, финальная проверка 2026-10-22

Scope: Windows, только penOverride (AC<=26) и совместимость API_AttributeIndex/Attribute_Delete/GetNum/сортировки в TestMeshFloor.cpp и TestFinishReuse.cpp. Issue: https://github.com/kuvbur/AddOn_SomeStuff/issues/275. Большой незакоммиченный рефакторинг, Health/projectName и Python не включены; исправление penOverride в index применено к прежнему Roombook.cpp, в worktree сохранён перенесённый RoombookDraw.cpp. Публичные сигнатуры и граф вызовов не менялись; Docs/generated рефакторинга остаются отложены по его Scope.
Status: COMPLETED в scope compile/link. Plan: [x] 275-build — изолировать совместимость, проверить именно staged source snapshot, выполнить review и checkpoint по команде владельца.
Validation: canonical BuildAddOn.py -c config.json -v 27 28 29 30 (Debug), затем тот же список --release --package: оба exit0, все восемь CMAKE_BUILD_OK. Четыре пакета сохранены одновременно, SHA256 каждого совпадает с соответствующим INT/RelWithDebInfo/SomeStuff.apx. Исходники проверочного каталога совпадают с index после штатного Git clean-filter; пять затронутых рабочих файлов сохранены byte-identical. clang-format с явным repo style для отдельных staged hunks, git diff --cached --check PASS. Clangd с новой CDB27: семь unused-variable в Roombook, один unused-lambda-capture в TestMeshFloor вне изменённых участков; дополнительные внутренние tweak failures дают exit3, полный LSP GREEN не заявляется. Runtime/macOS не выполнялись. Сборки изолированного snapshot имеют display commit unknown (Git archive), что не меняет проверенные исходники.
Evidence: Hermes scratch/ac27-30-final/{tree.txt,compatibility.patch,debug-final.log,release-final.log,validation.json,lsp-results.json}; пакеты checkout/Build/Package/SomeStuff/{27,28,29,30}/SomeStuff.apx. No issues found in inspected compatibility delta.
Next Step: при необходимости отдельно push и macOS/host-приёмка; исходную #275 не переоткрывать и не закрывать повторно.
Last Checkpoint: данный checkpoint [275-build], родитель 981d11f (отдельный Python-коммит; C++ и build-конфигурация не менялись); точный hash в git log. Push не выполнялся.

### Новые логи перехода на разрез — 2026-10-19

Scope: read-only восемь новых файлов из D:/Рогожин ДА/rec/Telegram Desktop/logs; без C++/SDK/build/host/model. Status: ANALYSIS_COMPLETE; faulting module/function/root cause — not verified без RPT/RPTA/DMP. Plan: [x] все файлы и hashes; [x] разбор пяти FastLogger и трёх UsageLogger (один пуст); [x] независимые байтовые проверки/сравнение; [x] отчёт и границы вывода.
Verified: AC25.0.0/PID35784, 2026-10-19 12:41:48 UTC+3 — один явный CRASH во второй ротации; перед ним резервирование12:41:31, «Изменение Размеров»/Stretch/VBElem::SectionSegment/LeaveEditing12:41:35, Open S/E/IE/3DD /51 и /52 в12:41:45. Последний явно названный АР.2 был в12:38:23, финальные Open S/E не называют вид. Последние Grid Tool записи12:38:26, за202с, а не в момент аварии. Сходство со старым Undo.dll-сценарием не доказывает тот же модуль или связь с #287. Версия загруженного SomeStuff/наличие фикса не установлены. Части логов и параллельные PID33360/36268 не выданы за дополнительные падения. 12/12 u16/UTF8/F7 проверок, PID из header и связанный аргумент Application crash подтверждены; все8SHA256 unchanged, parser26/26PASS. Compiled: not performed; Tested: разбор/байты, не host reproduction.
Evidence: Hermes scratch/section-crash-20261019-130656/{analysis.ru.md,input-manifest.json,byte-evidence.json,validation-summary.json}. Memory: archicad-fastlog/references/section-crash-20261019-no-dump.md. Исходники, модели, профили и процессы не менялись; подготовка коммита #287 прервана новым запросом, индекс пуст/коммит/push не выполнялись.
Next Step: получить соответствующие RPT/RPTA/DMP с рабочей станции для PID35784, 19 октября около12:41:48 UTC+3; затем установить модуль/RVA/стек, не переносить прежний диагноз. Возвращение к коммиту #287 — отдельным запросом. Last Checkpoint: существующий f86e0de, нового нет.

### Локализация строковых падений AC25/AC28 — 2026-10-19

Scope: только присланные логи/дампы AC25 PID13768 и обновлённого AC28 build7006 PID39084, точные CI APX и read-only исходники; C++/модель/живой debugger не менялись. Status: ANALYSIS_COMPLETE. Plan: [x] найти совпадающие APX; [x] установить addon call site; [x] независимо проверить и сохранить выводы. APX из CI run38909896330 совпали с дампами отдельно по GUID/Age/timestamp/ImageSize. Машинный код первого ACAPI_Attribute_Get в Param_SetComposite соответствует исторической строке4002/текущей4178; тела функций совпали. Это вызов, при выполнении которого упал хост, не адрес упавшей инструкции и не PDB line-symbolization. В AC28 type12/index111, buildingMaterial.id=1, header.uniStringNamePtr=nullptr; RCX=1, захваченная инструкция UniString::Set читает[RCX+8]→read9. В AC25 type17/index7, header.uniStringNamePtr в кодеGSRoot совпадает с сохранённым this. Подтверждён дефект инициализации API_Attribute ДО первого вызова; его связь с авариями сильно поддерживается, но прямая передача конкретных полей через APICore — not verified. Смещения SDK25/28 проверены отдельно пятью static_assert/MSVCv142 /Zs(exit0). Шесть подряд addon-кадров AC28 и следующий Archicad caller совпали при bounded unwind; полного host unwind/PDB нет. Независимое ревью подтвердило call-site атрибуцию и уточнило границы причинного вывода. Отчёт/числовые доказательства/пять неизменных SHA256: Hermes scratch/finish-crash-20261019/updated-pid39084/analysis.ru.md и соседние JSON. Compiled(аддон): not performed; Tested(исправление/воспроизведение): not performed. Memory: skill archicad-fastlog/references/somestuff-uninitialized-attribute-string-crash.md.
Next Step: по отдельной команде — issue и минимальная очистка всего attrib перед первым вызовом, затем проверка на целевых версиях/модели; не ограничиваться header.uniStringNamePtr (в AC28 он уже null). Last Checkpoint: нового нет, HEAD f86e0de; commit/push/публикация issue не запрошены. Остальные задачи сохранены.

### Анализ присланных аварий AC25 — 2026-10-19

Status: ANALYSIS_COMPLETE; первопричина not verified. Scope: только восемь файлов из пользовательской папки logs; C++/модели/host не менялись. Plan: [x] инвентаризация и hashes; [x] RPT/RPTA/DMP и хронология; [x] независимая проверка ключевых байтов и отчёт. PID29040: C0000005 write0x10 внутри SomeStuff_AddOn.AC25.WIN.apx, RVA0x11D73F; PID13768: C0000005 read0xFFFFFFFFFFFFFFFF в GSRoot.dll/UniString::Set, ниже API и шесть кадров SomeStuff.apx. В обоих RPT/RPTA стеки и исключения совпадают, дампы подтверждают адреса/модули. Второй FastLogger: после ведомости есть Property Manager и Sync; непосредственная причина в RoomBook не доказана. Исходные восемь SHA256 unchanged; regression парсера legacy-имён APX RED→GREEN,26/26PASS. Compiled: not performed; Tested: разбор логов/парсер, не воспроизведение Archicad. Отчёт/manifest: Hermes scratch/finish-crash-20261019/analysis.ru.md и input-manifest.json; санитизированные выводы: skill archicad-fastlog/references/somestuff-two-crashes-20261019.md.

Next Step: получить аварийные APX и соответствующие PDB (GUID/Age записаны в отчёте); локальный Debug PDB не совпадает ни с одним дампом. Затем расшифровать source frames, не переносить текущие адреса на другую сборку. Last Checkpoint: нового нет; существующий HEAD f86e0de, commit/push/issue не выполнялись. Другие активные задачи сохранены.

Завершённые задачи перенесены в `IDEA_ARCHIVE.md`:
- #278 — отписка наблюдателей (DetachObserver в Helpers/ReservationChangeHandler/APINotify)
- #277 — вылеты Undo.dll при переключении окон и в ElementEventHandlerProc
- #276 — неверная высота потолка-объекта (level от уровня этажа, ac_ref_height=0)
- #275 — совместимость SDK AC26–30
- #274 — очистка коллинеарных вершин полигонов полов и потолков
- #273 — очистка старых preferences проекта
- #269 — сигнатура AdvanceProcessPhase в CommonFunction
- #268 — доведение контура пола до ядра базовой стены
- #267 — поиск черновых полов и потолков по низу и верху стен
- #192 — регрессия Object-избранного пола/потолка
- Локальная приёмка углов стен (API_WallPart), срез документации Roombook 2026-10-15, расследования падений Undo.dll, этапы Z7–Z9.

## Классы плинтусов и карнизов: маркеры — 2026-10-29

Scope: только маркеры ролевых классов плинтуса/карниза в документации `wiki/ru/Finishing-ru.md` и `wiki/en/Finishing-en.md`. Код, PLN, избранное и прочие классы не трогать; без commit/push.
Status: DONE (docs), build/native не выполнялись.
Plan: [x] сверить имена с текущим кодом; [x] синхронизировать обе вики; [ ] проверить сборкой AC25 и в модели.
Last Completed: `RoombookConstants.hpp:51-52,68-69` содержит `some_stuff_fin_skirting` / `some_stuff_fin_cornice` и избранное `smstf skirting` / `smstf cornice` — везде латинская `c`. RU и EN вики используют эти имена без исторических примечаний. EN вики дополнена разделами о плинтусах/карнизах, строгих свойствах Sync_GUID и избранном.
Last Completed (JSON): в конец RU и EN вики добавлен раздел с JSON-запросом `API.ExecuteAddOnCommand` (`SomeStuffCommand.RoomBook`, без параметров), обёрткой ответа `result.addOnCommandResponse`, таблицей полей `rooms/types/elements` (`write_succeeded`, `was_created`, `length/area` + флаги доступности, material/bmaterial/structtype), таблицей значений `TypeOtd` 0–13 и оговоркой об откате пакета записи. Сверено с `RoomBookCommand.cpp:34-83` и реальным ответом `scratch/roombook-json-response.json`; оба JSON-примера проходят `json.loads`.
Next Step: собрать AC25 и убедиться, что классы модели с описанием `some_stuff_fin_skirting` / `some_stuff_fin_cornice` и избранное `smstf skirting` / `smstf cornice` распознаются.
Last Checkpoint: нового нет; commit/push не разрешены.

## #296 — данные отделки в ответе JSON RoomBook

Scope: AC25 Windows; расширение ответа RoomBook по помещениям и TypeOtd полями otd_guid, base_guid, length, area, material, bmaterial, structtype и запрошенными write_succeeded, was_created. Геометрию, пользовательские изменения и PLN не менять.
Status: WAITING_FOR_BUILD — в JSON добавлены два флага; активный VS/Archicad не останавливать, сборка и проверка новых полей отложены. Предыдущее чтение модели заблокировано модальным окном «Параметры Выбранной 3D-сетки»; не закрывать окно за пользователя и не сохранять PLN.
Plan: [x] изучить вход команды и структуру данных; [x] зарегистрировать issue #296; [x] проверить сборку и поля; [/] добавить в JSON write_succeeded и was_created без изменения фильтра результатов; [ ] проверить группированный ответ и созданные элементы в Archicad, отдельно повторный запуск и rollback; обновить _generated.
Last Completed: RoomBook вернул status=returned, 63 зоны, 404 группы, 3507 уникальных GUID; все требуемые поля есть, до вызова ни одного GUID из результата в GetAllElements не было. После вызова GetAllElements вернул 4001 из-за модального окна, наличие GUID в модели не проверено. JSON и baseline сохранены в scratch/roombook-json-response.json и scratch/roombook-json-before.json. VS Debug при F5 пересобрал RoombookDraw и перелинковал APX; оба PLN-хеша не изменились относительно запуска.
Current: RoomBookCommand.cpp выводит текущие bool write_succeeded и was_created; фильтр AppendRoomFinish пропускает только true/true. clang-format, diff-check и clangd по актуальному диапазону Execute выполнены; новый исходник не собран и не запущен. SDK25 ObjectState.hpp:191 подтверждает Add(bool).
Next Step: после безопасного завершения VS/Archicad по решению владельца собрать новую версию и проверить новый JSON; получить GetAllElements для сравнения с первым результатом по возможности до закрытия модели. Повторный вызов и rollback требуют согласования сценария. _generated не обновлён. Не закрывать issue без указания владельца.
Last Checkpoint: нет; commit/push не выполнялись.

## Диагностика предупреждений на новом тестовом проекте

Scope: AC25 Windows, запуск Archicad через открытый VS Debug x64 и один запуск создания отделки на проекте, указанном конфигурацией VS; анализ свежего отчёта. Исходники и сохранённый PLN не менять, commit/push не выполнять.
Status: BLOCKED. F5 в VS выполнил Debug x64 сборку, но SomeStuff не собрался; Archicad не запущен, VS Design.
Plan: [x] проверить состояние VS, процессов и тестового файла; [/] запустить Archicad через VS и создание отделки после устранения сборочного блокера; [ ] прочитать свежие предупреждения и установить доступные причины.
Last Completed: F5 compile FAILED, SkirtingRegression.cpp:379–380 C2065 Skirting_Up и Skirting_Down не объявлены. Существующие две точки отключены, C++ и PLN не изменялись. VS F5 не является подтверждением запуска; отчёт не получен.
Next Step: согласовать отдельное исправление тестового сборочного блокера или дождаться его исправления владельцем; повторить VS F5 и проверить проект/APX перед вызовом RoomBook.
Last Checkpoint: нового нет; commit/push не выполняются.

## #270 — перестали строиться плинтусы: живая диагностика

Scope: AC25 Windows, текущая test_25; диагностика подготовки/создания без изменения PLN/избранного/чужого кода. Владелец ранее разрешил VS запуск. Commit/push/закрытие не выполнять.
Status: BLOCKED — два модальных окна VS Hot Reload, clarify истёк без ответа.
Plan: [x] source и запуск VS/test_25; [x] адресные live-наблюдения preparation/favorite/первого upper writer; [ ] воспроизвести полный пропуск и установить root cause; [ ] исправление/приёмка только после подтверждения.
Last Completed: PID39596/APX d6a8ec7eda3a9c72cfba03911f9156f4fc3e300c8e7bf1867442bdfeccb6be0e; первая зона valid, faces2/walls10, SkirtingSegments_Build вернул4lower+4upper; оба первых favorite.type=API_BeamID. Первый upper Beam_Draw вернул draw_error0 и ненулевой guid.time_low3869729320. Это НЕ whole-run/native read-back и не доказательство отсутствия бага. Во время прогона Data изменялся извне, Draw/APX не менялись; runtime относится только к загруженному APX. Continue/StepOut timeout300s; EnumWindows подтвердил два окна devenv «Горячее перезагрузка — не поддерживается проектом». Код этим агентом не изменён, сборки/сохранения PLN не было. Все свои точки удалены; исходные Propertycache995/Draw1644 disabled не менялись, исходная Data8193 сдвинута редактором на8293 и restored enabled; требуется fresh list (получен отдельным вызовом). Evidence: scratch/skirting-stopped/live-investigation.md, before-launch.json/after-launch.json и session tools.
Next Step: владелец закрывает оба Hot Reload без применения изменений и прекращает редактирование Data на время наблюдения; затем завершить команду, независимо прочитать созданные Beam и проверить lower writer/остальные зоны. Не перезапускать/пересобирать/сохранять текущую модель до разрешения блокировки. Причина пользовательского пропуска not verified.
Last Checkpoint: нового нет, commit/push не разрешены. Memory: visualstudio-cpp-debugger/references/archicad-somestuff.md — Hot Reload блокирует MCP stepping.

## Исправление плинтусов и пропусков — 2026-10-25

## Scope
AC25 Windows. По прямой команде владельца исправить наблюдавшиеся OtdBeam_Draw ошибки и прежние пропуски surface-отделки; только подтверждённые причины в Roombook. Сохранить исправления #252/#291 и все пользовательские изменения. Не менять сохранённые PLN, избранное/настройки проекта, общие Helpers/Sync, не выполнять commit/push/закрытие issues. Docs/generated вне текущей разрешённой дельты.

## Status
WAITING_FOR_BUILD. Локальная правка толщины внесена и проверена SDK25 C++14-регрессией (31/0); native-приёмка не выполнена. Штатная сборка не началась: защитная проверка обнаружила ARCHICAD PID26892 с загруженным canonical APX. Владелец выбрал оставить Archicad открытым, пока без сборки (2026-10-26). VS-сеанс остановлен, оставшийся процесс не тронут. Dirty-baseline и доказательства: Hermes scratch/favorite-thickness-270/.

## Last Completed
Толщина добавлена в MatarialToFavorite; Favorite_GetDict читает Basic из Slab и полную Composite из общего compositeCache через существующий ComponentsCompositeStructure. В расчёте плинтуса удалён thickness=1; без избранного используется otd_thickness. Object-подготовка согласована с существующим параметром writer 0.02; тело авторского GDL не проверено. Helpers/Sync и другие production-файлы не менялись.

SDK25 MSVC C++14: исходный RED 22/10; cache-aware RED 28/14 → GREEN 28/0; финальная регрессия 31/0, compile/run exit 0. clang-format и diff --check выполнены. Свежий clangd: новых исходных ошибок нет, три прежних unused-variable; exit 3 из-за внутренних проверок code actions, не чистый общий проход LSP. Build/native: не выполнены по решению владельца. PLN и все остальные исходники совпадают со snapshot; own.diff и validation.json сохранены. Docs/generated вне разрешённой дельты; commit/push/закрытие issues не выполнялись.

## Plan
- [/] #270: по решению владельца читать толщину Slab в Favorite_GetDict и сохранять в MatarialToFavorite; использовать её вместо thickness=1 при подготовке плинтусов. Для многослоек повторно использовать PROPERTYCACHE().compositeCache (Propertycache.hpp:211), заполняемый ParamHelpers::ComponentsCompositeStructure (Helpers.cpp:9080–9150), а не отдельные ACAPI-запросы/новый кэш. Полная толщина — сумма всех CachedLayer.fillThick до фильтрации отделочных слоёв. Сначала SDK25 RED/GREEN (включая одинаковую многослойку у разных Favorites), затем AC25 build/native. Snapshot: Hermes scratch/favorite-thickness-270/before. Object/GDL и surface skips — отдельные незакрытые проверки.
- [ ] Сверить surface skips с актуальным кодом и выполнить адресную RED-регрессию (#252).
- [ ] Минимальные исправления по одному, GREEN-регрессии, форматирование, свежий LSP.
- [ ] Штатная сборка AC25 и native-проверка обоих дефектов, геометрии/связей и повтора.
- [ ] Собственная дельта/сохранность baseline; актуализировать статус и память.

## Next Step
Сборку и перезапуск не выполнять до новой команды владельца: PID26892 оставлен открытым. После разрешения — штатно собрать AC25 с сохранением build-copy PLN (SHA256 90bcd41b…), затем проверить native-отметки плинтусов и повтор. У #252 остаётся отдельная задача profile surface skips; не считать её решённой правкой толщины. Артефакты: Hermes scratch/favorite-thickness-270/validation.json, own.diff и tests/result.json.

## Last Checkpoint
Нового нет; HEAD 6f131e1, commit/push не разрешены. Issues #270 и #252 уже существуют и прочитаны; новая issue не требуется.


## Текущая диагностика: Surface finish skips

Scope: по команде владельца живая диагностика `Unresolved or unsupported finish pieces were skipped`, AC25 Windows, существующий VS debug PID54296/test_25. Без правок production, сохранения PLN, перезапуска, commit/push; параллельные задачи сохраняются.
Status: DIAGNOSED / WAITING_FOR_FIX (#252).
Plan: [x] найден emit и три ветки установки hasSkippedPieces; [x] живое воспроизведение двух разных источников; [x] свои точки удалены, штатный RoomBook returned, исходная Propertycache.cpp:995 enabled сохранена, VS Run.
Last Completed: AC25/PID54296/APX1f7773f0...: (1) Unmatched пола/потолка без slab sources попадают в общий skip, хотя nominal маршрут подготовил2slabs; отдельно есть два вертикальных Unmatched участка около4м² (их связь с проёмами not verified). (2) Профиль B2294E8B-5985-4585-B4C3-6002D3A5D073: SurfaceWall_ClampToParts hasParts=true, intervals/merged/result=0. Уступ y18.873961278980868 сравнивается с roomEdge4 y18.673961284517752; signed distance -0.19999999446311634м при source tolerance1e-6. Step-over onLine привёл к continue. Это не отказ по min_dim и не ошибка native writer. Полная геометрическая приёмка/все другие причины skips не проверены. Source параллельно изменялся, APX hash остался прежним; runtime относится именно к нему. Отладка не остановлена, PLN не сохранялся, production этим агентом не менялся.
Next Step: отдельная исправляющая дельта #252 — отделить штатные Unmatched/no-contact от реальных отказов и корректно ограничить профильные уступы без требования совпадения с плановой линией, сохранив конечные контакты/внутренние разрывы. Не поднимать tolerance до0.2м. Итоговый комментарий #252/6389558564 прочитан обратно; исправление/сборка/повтор после исправления ещё не выполнялись.
Last Checkpoint: нового нет. Evidence: Hermes scratch/finish-skips-debug/{baseline.json,binary-source.json,observations.json,roombook-response.json,final-manifest.json}. Memory: обновлён visualstudio-cpp-debugger/references/archicad-somestuff.md.


## Диагностика WallPart contact rejected — 2026-10-25

Scope: только выяснение причины, AC25 Windows; production/PLN/отладочные точки/управление чужой VS-сессией не менять. Status: DIAGNOSED, исправление не выполнялось.
Plan: [x] найти emit и контракт API_WallPart; [x] прочитать live-вход B2294E8B-5985-4585… в SurfaceWall_ClampToParts; [x] проверить конкретный predicate по числам.
Last Completed: parts[8] задаёт roomEdge4, tBeg0/tEnd4.04; единственный совпадающий time_low среди11 parts. Контакт y18.873961278980868 против roomEdges[3].y18.673961284517752; rise_y6.123e-17 при dz0.555 почти нулевой. Расстояние0.19999999446311634м при DistanceTolerance1e-6м: onLine=false, поэтому matching part не даёт интервала. По исходнику это ведёт к пустому corrected и сообщению с вручную переданным APIERR_BADPARS; это не SDK-код отказа создания. Полный GUID tail не снимался. Evidence: Hermes scratch/wallpart-contact-diagnosis/observed.json. Проверено по live-входам и исходнику; сам переход в ветку не исполнялся агентом этой диагностики. Compiled/Tested: не выполнялись; чужая debug-сессия сохранена. Во время чтения параллельно менялись source/позиция отладчика: весь текущий source не объявляется равным загруженному APX; соответствующий helper повторно прочитан, predicate сохранён. VS output_read завершился SSE error.
Next Step: отдельная задача исправления — согласовать применение 2D wallPart к смещённым 3D-граням сложных стен; не увеличивать допуск до200мм и не вводить безусловный fallback без проверки границ/проёмов. Исходный контур зоны и 3D-грани на разных высотах не обязаны совпадать в XY.
Last Checkpoint: нового нет, commit/push не выполнялись.

## #260 — продолжение рефакторинга, 2026-10-23

Scope: только сохранение поведения Roombook, AC25 Windows, текущие четыре cpp и общий контракт. Поддельта перехода общих полей OtdWall/OtdSlab/OtdBeam в draw: начать с TESTING-потребителей, production read-only на этой фазе; затем продолжить production только отдельным явным шагом. Функциональные #252/#270/#271/#284/#285, CommonFunction/Helpers/Sync, ресурсы и модель вне scope. Docs/generated и commit/push/закрытие issue по-прежнему требуют отдельной команды.
Status: IN_PROGRESS общего #260; текущая Draw-поддельта OtdDrawData: canonical AC25 Debug build выполнена, но FAILED на ошибках компиляции текущего объединённого дерева: SkirtingBeam_Prepare не объявлена, в RoombookDraw.cpp есть неразрешённые вызовы и рассогласование Wall_Draw; VS debugger сейчас Design. C2 Quantities и предыдущая пара C2 Data/Draw завершены в проверенном объёме. Предыдущая canonical AC25 Debug build PASS относится к Data/Draw, не к новой C2 remaining; независимый cross-review Data/Draw: No issues found in inspected scope. C1 inventory завершён в source/text scope. Host88/38985/168 относится к прежнему Quantities APX, не к последней паре. Тест и геометрия откосов вне scope; нативная приёмка C5/C6/C8 остаётся открытой. Разрешены необходимые сборки/проверки с открытой VS; чужую debug-сессию не останавливать.
Plan:
- [x] Draw-поддельта, фаза TESTING: обращения к полям OtdWall/OtdSlab/OtdBeam переведены на `.draw`; исправлено ещё `overlapCut.draw.base_guid` в TestRoombookGeometry.cpp. Свежий clangd: 5/5 тестовых файлов без diagnostics. Canonical AC25 build после исправления не сообщает оставшихся ошибок прямого доступа к полям Otd, но не завершает компиляцию из-за unresolved symbols в текущем объединённом дереве; runtime не выполнялся.
- [/] Draw-поддельта, production: миграция полей частично применена к RoombookData.cpp/RoombookDraw.cpp/RoombookQuantities.cpp; fresh clangd по этим трём файлам — 0 diagnostics каждый. Canonical AC25 build FAILED: в текущем дереве не объявлены SkirtingBeam_Prepare и ряд вызовов RoombookDraw.cpp, также тело Wall_Draw расходится с параметрами; устранить эти blockers и повторить scoped source review/build. Test runtime и остальные AC/macOS — not verified.
- [/] C2 reader-guards: Param_Property_Read — плоский null/invalid continue перед неизменными vots/vots_fill/first-valid/fromProperty правилами. По явному повторному разрешению владельца canonical-сборка на этот шаг отложена. Предварительный и повторный после применения MSVC SDK25 compile/link0, executable2019 checks/0fail (1009 неизменных assertions TestCachedParameterReader на каждую версию +1 сравнение labels). Реальные полные old/new функции; Param_Property_Read и SDK-контейнеры не подменены. Проверены типы/все поля ParamValueData и формат, aliases/порядок/false-vots/уже valid/несколько GUID/независимость копий существующими тестами; это не host. Scratch-only negative mutation (отключение fromProperty type override) действительно поймана прежним assertion: compile0/run1 FAIL Cached reader complete value and type. clang-format/dry-run PASS; свежий CLI AST — те же3 unused-variable, check-lines=1-1 ограничивает tweaks. Live Data5552e43f1b5fe94786dd69731f04d96219c09ddeffc225583d50760def3d3192; old/new extracts совпали с before/live, весь остаток файла и27 остальных snapshot files неизменны. Независимое source-only review: APPROVE, no blocking defect found in inspected scope; reviewer проверял предложенную границу, не applied diff. Родитель сверил reviewed baseline с before.cpp, три dependency hashes и точную применённую границу повторным verify-applied.py PASS; parent-review-check.json. Review SHA256 f33bd3c724a18bbdbbbfa99bb7357f37beeaa9c4de2dc5b31261e5e2615c6c81; evidence e5a2d9bd106ddf2e59ab132d567e300af1fdb762843afb2c077b5618c56695a3. Осталась deferred canonical AC25 build; шаг целиком не закрыт. Evidence: Hermes scratch/260-c2-reader; missing-base precondition/aliases/уже valid/смысл bool не менять.
- [/] C2 opening-guards применён: Param_SetToWindows — плоские null/valid guards frame/sill; формула, fallback, порядок и read boundary сохранены. Родитель повторил MSVC SDK25 compile/link и executable:294912 случаев/4890624 checks/0 failures; полные old/new функции, контролируемая Param_Property_Read, не host. Exact applied-function proof PASS; весь код вне функции и27 остальных snapshot files неизменны. clang-format/dry-run PASS; fresh before/after CLI AST те же3 unused-variable (check-lines=1-1, token/tweak scope ограничен). Live Data SHA25624aa4430722acb8c474fa352ed496af5145ef60ac5bb3d3c9a58ea6b4a02570c. Независимое source-only ревью завершено: No issues found in inspected scope. Родитель сверил review hash50033a15ed462bddf97803904131da60d56fef904ddb9c551a5f3af45a2334f1 и live/source/extract hashes; parent-review-check.json. Reviewer snapshot тестового label/result исторический; финальный parent-run после исправления label PASS, поведение стенда не менялось. Покрытие ReadParams частичное; формула проверяется дифференциально, reader и host не проверены. Canonical addon build отложена по явному разрешению владельца ДЛЯ ЭТОГО продолжения; чужая отладка/модель не тронуты. Evidence: Hermes scratch/260-c2-remaining/opening-candidate/{result.json,applied-proof.json,verify-applied.py,before-lsp.log,after-lsp.log,own.diff}. Commit/push нет.
- [/] C2 remaining, 2026-10-23: применены только BodyComponent_Read (явный invalid-index guard, прежние cache/error semantics) и OtdSlab_FindInRoom (явное совпадение подготовленного основания и break). clang-format/dry-run PASS; final fresh CLI AST: те же3 unused-variable, новых source ошибок нет (check-lines=1-1 ограничивает tweak-проверки, не полный semantic audit). Побайтная locality: два участка Data, остальные27 source/test snapshots неизменны; live=proposal ebd6b40e593f8e8940d3298e27514301a1820931ebe7e4594efbf4ccd44c53a3. C++/SDK25 paired regression: parent compile/run exit0, 5361360 checks/0 failures, Body11520 обращений и Slab1920 исполнений фрагмента. Реальные SDK/API_Component3D и Roombook.hpp/Otd-типы; только ACAPI_3D_GetComponent заменён контролируемым callback. Это не host и не полная OtdSlab_FindInRoom; slab geometry/composites пустые fixtures. Parent исправил signed overflow index+mode в scratch callback через widening, перекомпилировал/повторил; старые5353596 checks не использовать. Итоговые inputs/live hashes совпали. Evidence: paired-regression/{run.py,result.json,parent-run.log}. Независимое source-only ревью: No issues found in inspected scope; родитель повторил reproduce.py exit0, hashes review4d906e1e775cbd6710bf3aa418c94fff8839d4e946176029a052b45592147088/evidence f136159731f7f6dd4bdbc9d71e2bf0e6a2b9cb4a5914468f8f94f2bb52dcfdfd совпали; весь остаток Data вне двух замен побайтно идентичен. Canonical build НЕ началась: preflight host-count обнаружил PID25748, VS Run; чужая сессия не остановлена. Временная ошибка ParsedPropertyCommand воспроизвелась и после точного снятия обеих собственных правок; пользователь исправил внешний header, обновлённый baseline без неё. Затем по команде владельца «roombook не трогаю, работай» возвращена точная proposal. testWall/testSlabs — действующие проверки, не probes; staged Mesh и откосы не тронуты. Evidence: Hermes scratch/260-c2-remaining/{own.diff,parent-locality.json,*current-lsp.log}; host/модель/commit/push не выполнялись.
- [x] C1 source inventory: Data153 conditional signature rows/152 тела, Draw29 функций, Common3 функции/4 conditional definition sites, Quantities9, header68 callable contracts. Родитель проверил live hashes всех5 файлов,83 точных тела первой половины Data,69 полных signatures второй; одна сокращённая signature сверена name/range. Caller map source/text bounded, устаревший MCP не выдавать за свежую semantic verification. Evidence: scratch/260-census-next/{data-first,data-second,draw,quantities-contract}; scratch/260-next/census-parent-reconciliation.json.
- [x] C2 Data/Draw: Push/break до первого точного Core и плоский opening-failure cleanup. Real C++/SDK25 regression183594/0, все16 data fields; parent rerun PASS. Exact proposal/live hashes совпали, остальные26 snapshots неизменны. clang-format/dry-run PASS; fresh CLI прежние7 unused-variable, внутренние tweak failures/exit3 отдельно. Canonical AC25 Debug compile/link PASS, APX982c1ec0118ef7b827fae93a1dc74688a1267742211df7c32f4a822a384c1850; source/оба PLN неизменны. Независимый cross-review: No issues found in inspected scope; review hash5df47bce280a06beca71db80890c4d93d6a66c556a8bd2513975589a3a8db214 и финальные source/APX сверены родителем. Runtime отказа проёма/rollback и полный host после пары not verified. Evidence: scratch/260-next-batch/{parent-merge-proof.json,data/,draw/,review/}, scratch/260-resume/c2-data-draw-final/{result.json,build.log}. No-index --check exit gate not verified (несовместимость опций установлена по локальному git-diff.html); formatter gate отдельно PASS. Python behavioral simulations, scope expansion, commit/push отсутствуют.
- [x] C2 Quantities: только ранние выходы OtdData_AddRoomToMaterialSummary применены. Parent rerun196608 paired model traces PASS; aliases/rawname14 pushes/warning/cache/calc tokens сохранены, вне функции побайтно идентично. clang-format/dry-run/scoped diff-check PASS; fresh CLI source diagnostics0/other errors0, прежние10 ExtractFunction failures/exit3. Canonical AC25 Debug compile/link PASS (реальные RoombookQuantities.cpp и target link в log), APX1833bc575e79c1a5b5f57632f079458e9446e67031983efcdb32783ce646d60e. Host TestKit после дельты88/38985/168, все88 END-row равны baseline. Независимый source-only review: No blocking defect found; artifact hash9002f2f1b8d239e55f773b68f0a14190e66babdc1e9ff9b7a452984f082652be родитель сверил. Model повторяет ту же комнату, не гетерогенные последовательности; reviewer не запускал model/host, parent запускал. Финальные source/APX совпадают с протестированными.
- [x] Canonical AC25 Debug baseline compile/link PASS; свежая CDB и CLI clangd baseline сохранены. Четыре независимых read-only аудита завершены и опорные ветви сверены родителем.
- [x] C5: единый обход комнат/отмена/отбор зон перенесён в общий сценарий; Quantities содержит OtdData_AddRoomToMaterialSummary. Расчётный блок, порядок обхода/коротких замыканий и остальной код сверены по токенам. nPhase локален; четыре пустых forwarding headers удалены, retired identifiers — 0. C2: два guard-условия Data раскрыты, остальной Data побайтно сохранён.
- [x] clang-format/dry-run и scoped diff-check PASS; fresh CLI clangd сохранил прежние 7 unused-variable, новых source diagnostics нет. CLI exit3: внутренние tweak failures, полностью GREEN LSP не заявлять. Canonical AC25 Debug compile/link PASS после объединения, APX 2b6e39e31933b9661cfb3c7eb772a5ac92bc56aa25b444e60c0d3d58999ab11e. Независимый source-only review baseline/proposal/coordinator: No issues found in inspected scope; родитель отдельно доказал соответствие применённого кода и неизменность source/APX после сборки. Локальный nPhase живёт дольше guard; вложенные запуски теперь изолируют счётчик, полная реентерабельность не заявляется.
- [ ] Host сравнение площадей/свойств/геометрии/связей и штатный RoomBook + повтор после этой дельты; A/B времени и остальные SDK/macOS not verified.
Last Completed: standalone RoomAnalytic46/0, FloorCore41/0, CoordinateSearchCache PASS, parent повторил запуск всех трёх и сверил hashes. Извлечённые predicates: 8448 truth/short-circuit traces + 308 IEEE cases PASS, parent rerun PASS. Это не host-приёмка C5/RoomBook/повтора и не A/B производительности. Исходный и build-copy PLN неизменны; Archicad не запускался. 24 остальных файла source/test snapshot неизменны. Общий git diff --check выявил прежнюю пустую строку EOF в IDEA_ARCHIVE.md:4693; вне scope, не исправлялась. Сбой первого собственного manifest-check был различием backslash/slash в allowlist, не изменением постороннего исходника; исправлена только нормализация ключей проверочного скрипта.
Evidence: Hermes scratch/260-resume/{merge-proof.json,lsp-comparison.json,data-parent-verification.json,baseline/,final/,*.own.diff}, 260-c5-proposal/applied/, 260-c2-data/applied/, 260-standalone/manifest.json. Memory: дополнен cpp-code-refactoring — interleaving/cancel, независимые eligibility, predicate traces и Windows manifest paths.
Current Work: C2 host через VS/PID14124 на test_25 завершён:88 suites/38985pass/168fail, sole failed suite TestOpeningRevealsCreateOne3586/168; каждый END-row совпал с до-C2 baseline. Freshness/report totals/loaded APX path+hash/source Q hash связаны в scratch/260-next/c2-startup-result.json; панель «Отладка» сохранена отдельно, TestKit counters взяты из его фактического файлового канала. Родитель остановил только свой debuggee; Design/hosts0 и прежняя Propertycache1011 enabled подтверждены. Оба PLN неизменны;28 source/test snapshots неизменны после build/host. Первую попытку build пресёк host-count guard ДО компиляции (чужой PID20104 «Без имени»); владелец ответил «Закрыл», затем count0 и реальная сборка PASS. Перед F5 подтверждены exclusive handles на test PLN/lock; оставшийся lock не удалён, а сохранён в уникальный scratch c2-orphan-*.pln.lck. Полный GREEN и production RoomBook/repeat не заявлять.
Decisions: Data ещё не полностью read-only: оба Favorite_GetByName вызывают UnhideUnlockElementLayer (Data7868/7877), а OtdWall_GetDefault_Wall из Draw имеет Data caller. Это исключение явно учтено; перенос/разделение SDK/Favorite в этой дельте не выполнять. Param_GetForRooms и Param_GetForWindowParams требуются header initializers; favdict/reducededges не менять. Следующий C2 выбран только для OtdData_AddRoomToMaterialSummary: ранние выходы invalid/null/missing names вместо вложенности, без новых helpers и без повторения завершённого C5.
Baseline diagnosis: production верхний откос left→right, TestCore966–967/1180–1182 ожидает right→left; source-table census120+48=168 (не host rerun). Изменение production вошло9963693 от15.10.2026; прежний tool report и issue267 уже содержат3586/168. Родитель сверил TestCore побайтно с pre-C5 и весь63554-byte suffix Data от точного определения OtdWall_InitializeOpeningReveal: идентичен. Agent diagnose.py выбирал первые текстовые упоминания некоторых имён (calls), поэтому его per-function hashes не использовать; независимое suffix-доказательство — scratch/260-next/reveal-parent-verification.json. Нативный контракт направления отдельно не перепринят; GREEN не заявлять. Новых issues/comments нет, тест/production не менялись.
Next Step: владелец переходит к ручной проверке; автоматический запуск Archicad/остановка отладки не разрешены этой командой. reader-guards source review получено и сверено с baseline/dependencies/live5552e43f...; source/locality/standalone-regression gates PASS, повторно не реализовывать. После окончания чужой отладки выполнить canonical AC25 build четырёх текущих C2 изменений (BodyComponent_Read/OtdSlab_FindInRoom/Param_SetToWindows/Param_Property_Read), отложенную по повторному явному разрешению на этот шаг. Далее продолжить оставшиеся пункты согласованного C1–C9 по полной transfer map, отдельно от нативной приёмки C5/C6/C8. Текущие Quantities, ParamComposite_CollectLayersToCore и opening cleanup завершены, повторно не реализовывать. До следующего source-среза сверить актуальный scope и исключения Favorite/defaults: Data не полностью read-only, переносы без отдельной проверки не делать. Evidence последней пары: scratch/260-next-batch/{parent-merge-proof.json,baseline/,before-lsp.json,after-lsp.json,data/,draw/,review/}, scratch/260-resume/c2-data-draw-final/{result.json,build.log}. Прежний Quantities/runtime: scratch/260-next/quantities-c2 и c2-startup-result.json. Memory: cpp-code-refactoring дополнен правилами untracked/no-index проверки и сверки числа data members. Docs/generated и commit/push не выполнялись.
Last Checkpoint: WIP #260 перед ручной проверкой — текущий коммит с сообщением [C2-WIP]; точный hash: git log -1 --format=%h -- Sources/AddOn/roombook/RoombookData.cpp. По команде владельца фиксируется только рефакторинг, параллельные правки остаются вне коммита. Это не подтверждение canonical-сборки последних четырёх C2 правок или host-приёмки. Push не разрешён и не выполняется.



## Исследование первичной настройки отделки через Python/JSON — 2026-10-22 (COMPLETED)

Read-only аудит и скрипт `python_scripts/roombook_setup.py` созданы и проверены на реальном проекте (порт19723). Найдено 8/24 маркеров свойств и 7/10 классов some_stuff_fin_*; избранное и комбинация слоёв smstf_fin штатным JSON AC25 не перечисляются — нужна аддонная команда. Аддон на обоих портах не был загружен (Health4010), аддонные команды live не проверялись. Next Step: создание 17 отсутствующих свойств и 3 классификаций через штатный JSON; команда для избранного/слоёв — отдельная задача. Last Checkpoint не создавался. Подробности — IDEA_ARCHIVE.md.

## Task

Продолжение переноса на поверхности 3D-зоны, SDK AC25 Windows. Reader, partition/beamPart и подключение production-создания/расчёта выполнены в проверенном объёме. Issue: https://github.com/kuvbur/AddOn_SomeStuff/issues/252. План: .hermes/plans/2026-10-11_115207-roombook-zone-surfaces.md.

## Scope

По команде владельца scope текущего рефакторинга дополнен очисткой ненужной диагностики, упрощением данных/ветвлений и минимальной осмысленной организацией файлов Roombook. Подробный план C1–C9 — в Next Step / «Читаемость и минимальная структура»; новая issue не нужна. Реализация идёт отдельными дельтами ниже; текущая build/host-приёмка отложена по ограничению владельца.

Создание и расчёт отделки из проверенных участков: стены, колонны/сегменты, перекрытия и балки (низ и бока), штатные writers и материальные правила. Один участок — одно основание; неоднозначные/непредставимые участки не создавать. Представимые негоризонтальные грани — штатные наклонные стены на отдельном слое с группой пересечения 0. Исходные конструкции и сохранённые PLN не менять. AC25 Windows; остальные SDK отдельно, AC_25 gates не возвращать. Docs/generated и коммит только по отдельной команде. По явному решению владельца дополнительно завершить восемь однопараметрических вызовов AdvanceProcessPhase в Roombook.cpp, передав существующий nPhase (завершено). Пользовательский перенос CommonFunction.cpp/.hpp не менять.

## Status

WAITING_FOR_ACCEPTANCE: переход на одну стену с прямоугольными вырезами и реальными base_guid проверен на AC25 Windows; selftests75/75, native335→335, покрытие представимых контактов и повтор в1e-6м PASS. Ограничение: две полные грани с диагональным краем20.498776м² пропускаются целиком, исходный проём4F6A150D-A60B-4ABA-927D-0FC46F0D3FE7 пока не имеет отделочного проёма. Подробности Archive/«Одна стена с проёмами». #252 OPEN; Docs/generated и commit по Scope отложены до команды владельца.

## Last Completed

- 2026-10-23, _CallUndoableCommand: гарантирует 3-точечную линию выноски; сборка AC25 успешна; готово к ручной проверке в Archicad.
- 2026-10-22, #288 (закрыта владельцем): библиотечный блокер снят — «плитка» libInd4919, `OtdWall_Draw_Object` после `ACAPI_Element_Change` вернул err0; два штатных RoomBook и повтор прошли без отказа. Полная геометрия/материалы/ownership links/отказы остаются not verified; подробности — IDEA_ARCHIVE.md.
- 2026-10-16, #285: реализация расширения контура пола в проёмы и 27 адресных host checks PASS; canonical AC25 build и свежий LSP 0. Общий selftest не GREEN по трём прежним FAIL вне scope; native-приёмка не выполнялась.

## Next Step

### #270 — нижний и верхний плинтусы (AC25 Windows)

Новая дельта (2026-10-25): IN_PROGRESS — владелец сообщает, что на стенах с дверными/оконными проёмами до пола плинтус отсутствует. Scope: AC25 Windows, только подтверждённый путь подготовки/вырезания конечных отрезков; Beam-only имена/материалы и существующие разрывы сохранять. Plan: [/] воспроизвести точный пропуск в текущем источнике/модели; [ ] адресный RED; [ ] минимальное исправление и GREEN; [ ] format/fresh LSP/locality/build/native по доступности сохранённого проекта. Причина пока not verified. Текущую модель не сохранять, перезапуск без проверки сохранения новых правок владельца не выполнять. Docs/generated, commit/push/закрытие не выполнять. Next Step: проследить SkirtingSegments_Build на реальной стене с таким проёмом; прежние218/0 подтверждают другой объём, не полноту этого случая. Last Checkpoint: нового нет, HEAD6f131e1.

Текущая Beam-only дельта COMPLETED в согласованном scope (2026-10-25); подробности — Archive / «#270: Beam-only подбор плинтуса». AC25 Windows: материал зоны не переносится, отдельные smstf plinth down/up принимаются только как Beam; неподходящее/отсутствующее избранное не подменяется. Legacy Wall/Object распознаются только для замены прежних результатов. Plan: [x] RED175/43 → GREEN218/0 на SDK/C++14; [x] минимальный fix, format/locality/review/registry90; [x] canonical AC25 Debug build0, свежая LSP-дельта без новых diagnostics (прежние3/4 unused-variable, exit3); [x] два native запуска:23 нижних+21 верхний, все Beam,88 ownership values/run, старые GUID удалены. Next Step #270: ручная геометрическая приёмка контактов/профиля/смещений; +4 Wall общей отделки на повторе диагностировать отдельно, не объявлять весь RoomBook идемпотентным. Модель оставлена открытой, PLN не сохранялся. Last Checkpoint: нового нет, HEAD6f131e1; Docs/generated/commit/push/закрытие не выполнялись.

Исторический source audit выявил приоритет material/@name и smstf wall fallback над dedicated Beam-именами; исправление выше и в Archive подтверждено новым native запуском. Ниже — история прежних multi-type этапов, а не действующий план создания плинтуса; новая политика создания — только Beam.

Scope: AC25 Windows, #270: SkirtingBeam_Draw/prepare по favorite.type (Beam/Wall/Object), общий draw gate, native whitelist и подбор избранного для двух ролей; адресные регрессии. Object по выбору владельца — обычный объект вдоль локальной X, НЕ стеновой аксессуар. Prepared-array архитектура и геометрическое ядро сохранены. Чужие native writers, NoSet default OtdBeam, текущие классы some_stuff_plintus_down/up и fallback smstf wall не менять; Docs/generated, ресурсы/PLN, commit/push/закрытие вне scope.

Status: WAITING_FOR_TEST — discovery исправлен, canonical AC25 Debug build0 и live creation подтверждены: 23 нижних + 21 верхний новый элемент. Полная геометрическая приёмка #270 ещё открыта; creation не считать доказательством размеров/контакта/повтора.
Plan: [x] прежний Beam/Wall/Object dispatch и SDK193/0; [x] VS repro: 4/4 кандидата первой комнаты → null-class → continue; [x] Class_FindFinClass RED6/10 → GREEN16/0, минимальный discovery guard; [x] владелец добавил оба класса, сохранил проект и разрешил перезапуск; [x] VS Debug compile/link и canonical BuildAddOn.py -c config.json -v25 exit0; [x] native creation: классы0/0 → 23/21, все44 GUID новые и роли не пересекаются; [x] независимый JSON read-back двух GUID-свойств у44 элементов:88 normal строк с существующими родителями; [ ] геометрическая приёмка обычного Object/Wall/Beam, ожидаемые родители по каждому контакту и повтор без дублей. Evidence: Hermes scratch/skirting270-host-debug/{native-green-build-manifest.json,discovery-build.log,green-before-inventory.json,green-after-inventory.json,native-creation-validation.json,native-skirting-ownership-readback.json,native-skirting-ownership-summary.json,current-session-debug.txt}. LSP ранее:3 unused-variable isvisible/flag_find/dlower, exit3, не полностью GREEN; format и scoped git diff --check PASS.

Last Completed (2026-10-24, native creation): Sources/AddOn/roombook/RoombookData.cpp:7900 пропускает оба маркера some_stuff_plintus_down/up. Новый APX1f7773f0c95203d712660fa58697633b528d19de9e62e630dd69ce4e3e601da5 загружен из Build/SomeStuff/25/Debug; VS остановился в SkirtingBeam_Draw:56, favorite.type=API_ObjectID, libInd4919; шаг через ACAPI_Element_Create дал createError0. После returned инвентарь188→232:74 новых/30 удалённых GUID, включая23 нижних+21 верхний. Оба GUID-свойства44 плинтусов прочитаны независимо:88/88 normal и ссылки существуют в текущем инвентаре. Это не полная проверка правильности выбора родителей/геометрии.

Previous validated delta (2026-10-24): SkirtingBeam_Draw читает выбранное favorite через общий helper, сверяет native type и создаёт выбранный тип без подмены defaults. Beam сохраняет прежний профиль/наклон; Wall — высоту/сечение/материалы, top-story link отключён; Object — обычный локальный X, ширина/высота/GDL memo сохраняются. Исходный A библиотечной части читается ACAPI_LibPart_GetParams, additive handle освобождается на всех выходах; xRatio преобразован согласно установленным header/HTML SDK25, НЕ нативной 3D-проверке. Actual extracted helpers и Favorite_FindName: RED143/5 → final GREEN193/0, compile/run0. Контролируемы только проектные зависимости избранного/состава/отчёта; console не исполняет native API. Scratch-only Object scale mutation compile0/run1:189pass/4fail; repository mutation не применялась. Тесты актуализированы под пользовательские маркеры/NoSet и фактический контракт unresolved Favorite (имя остаётся, тип Zombie); ошибочный fixture refLineType исправлен по SDK25 referenceLineLocation, это не product defect. Canonical BuildAddOn.py -c config.json -v25 Windows Debug: compile/link exit0; APX75ccb5212619bc3e075b40a6d0d556f5ac92727bbc8e24a08ba62b38bd55a54a. Source/test/APX hashes связаны с проверками, оба PLN неизменны. Fresh CLI Data3/Draw4 прежних unused-variable, новых diagnostics нет; exit3/check-lines=1-1, полностью GREEN LSP не заявлять. Format/scoped diff/registry90 PASS. Код вне трёх Data/двух Draw функций и header prepare-контракта совпал с before; шесть прежних writers и геометрические helpers неизменны. Собственное source-review: No issues found in inspected scope; не независимое ревью. Evidence: Hermes scratch/skirting270-dispatch-types/{validation.json,locality-result.json,*-result.json,run-sdk-regression.py,build.log,lsp-result.json,*.own.diff}.

Ограничения: Wall/Object требуют горизонтального контакта в1e-6; наклон не уплощается молча. Geometry/repeat/undo — not verified. У25 из44 native bbox нулевая высота (18 lower+7 upper); без исследования выбранных GDL это не доказательство ошибки writer и не геометрическая приёмка. В текущем debug-логе есть ParseSyncString/ConvertToProperty ошибки для GUID-свойств общей отделки; не приписывать их плинтусам: фактические88 свойств плинтусов прочитаны успешно. Next Step: ручная геометрическая проверка в оставленном открытым test_25, затем адресная проверка размеров/контактов/родителей и только после корректности — repeat. Собственные точки удалены; Propertycache.cpp:995 restored enabled по baseline, проверено fresh list. PLN агент не сохранял; owner-saved build-copy43fa9e0740d2030617d90fa720aadb1b231e731c9724cc4a17770b565e327fe3 восстановлена после canonical-копирования, repository Test_file не менялся. Last Checkpoint: нового нет; HEAD6f131e1, commit/push/закрытие и Docs/generated не выполнялись по scope. Memory: saved — lightrag/reference, native creation и границы проверки.

Ниже — исторические этапы #270; описания отдельного post-write dispatch больше не являются действующим планом.

Предыдущее разделение типов (до допуска аксессуара): нижний сохраняет маркер some_stuff_fin_beams; верхний — отдельный some_stuff_fin_up_beams. Class_FindFinClass регистрирует оба, Class_GetClassGuid выбирает по beam.type без fallback в другой/generic класс; обратный lookup и Beam-only whitelist различают роли. Default избранные только в Skirting_Down/Skirting_Up ветках Favorite_FindName; Floor/Ceil остаются ролями перекрытий. Реальные извлечённые SDK25 функции: RED108pass/6fail → GREEN114pass/0fail, compile/run0; прежние геометрическое ядро/prepare/нижний collector/helper hashes совпали. Python checker с контролируемым JSON:6 cases/0 failures, включая независимое отсутствие каждого класса; не live API. Canonical BuildAddOn.py -c config.json -v25: Debug compile/link exit0, APX2958f56fa0befd35b1e1effa80e10260edd74f1fbe210c4924c5a6c893054902. Fresh CLI Data3/Draw4 прежних unused-variable, новые diagnostics0; exit3, полностью GREEN LSP не заявлять. Format/scoped diff/registry90 PASS. Sources до/после build и оба PLN hashes совпали. Собственное source-review: No issues found in inspected scope; не независимое ревью. Evidence: Hermes scratch/skirting270-split/{validation.json,own.diff,green-result.json,red-result.json,python-classification-result.json,build.log,*-lsp.log}.

Предыдущая архитектурная дельта (до разделения типов): SkirtingSegments_Build сохраняет конечные отверстия/дуги Polygon2D, обе Z, индексы итоговых участников, независимые flags/массивы; устраняет совместимые коллинеарные дубли швов. OtdBeam_ProcessFinishes учитывает толщину выбранного Slab без изменения исходных OtdSlab. Общий writer создаёт плинтус только после записи обоих участников, проверяет классификацию и native zone/base ownership внутри общей undo-команды. Линейные свойства не публикуются. Шесть существующих native writers/default helpers побайтно совпали с before. Повторный standalone SDK25 compile0/run0:103 PASS/0 FAIL; mutation без slab clipping: compile0/run1, пойманы finite coverage/holes. Это не host и не проверка rollback/повтора в модели. Canonical BuildAddOn.py -c config.json -v25 после всех C++ правок exit0, APX02acd04247985d2860620b019ec338ce60d4dff692d218a834b7326340c9eedc. Fresh CLI Data3 и Draw4 прежних unused-variable, exit3; полностью GREEN LSP не заявляется. PLN hashes неизменны. Независимый reviewer недоступен (HTTP429), выполнен собственный source-review; не выдавать за независимое ревью.

Коррекция по замечанию владельца завершена: [x] OtdBeam.favorite (MatarialToFavorite), выбор до writer; [x] SkirtingBeam_Draw принимает OtdBeam и использует Favorite_GetByName вместо прямого ACAPI_Favorite_Get; [x] три новые проверки хранения/независимости выбранного избранного, SDK-console84/0; [x] canonical AC25 Windows Debug compile/link exit0 после всех правок. Existing native writers и общий Favorite_GetByName не менялись, проверено сравнением тел с before. Ownership P1: обязательные base/zone подготовлены, записываются и читаются обратно через ACAPI_Element_GetPropertyValue внутри create/classify undo до удаления старых; ошибка/отмена regular Sync останавливает dispatch. Проверены source-контракты, не runtime rollback. Evidence: scratch/skirting270-dispatch/{verify-corrected.py,validation-corrected.json,build-corrected.log,regression-corrected.log,lsp-corrected.log}. Fresh CLI: прежние4 unused-variable, exit3; clean LSP не заявляется. Status: WAITING_FOR_TEST. Next Step: ручная проверка построения и повторного запуска владельцем, включая реальные грани, уклон/anchor и отмену/ownership. Last Checkpoint: нового нет; commit/push/закрытие не выполнялись.

Продолжение подключения, 2026-10-24: по прямой команде владельца «Подключи построение, проверю сам» включить production dispatch AC25, не блокируя его на прежнем native placement gate. Scope: финальные комнатные грани успешно записанных стен и пола/потолка, SkirtingContact_Build → SkirtingBeam_Draw, точные избранные, классификация/связи и повтор без дублей; без изменения существующих writers и публикации линейных свойств. Plan подключения: [x] production dispatch после отделки и Sync; [x] исправлен review P1 (source+build); [x] повторная canonical AC25 build и проверки. Исторические сведения о первой сборке ниже; актуальные артефакты — validation-corrected.json. Runtime-приёмка оставлена владельцу, прежняя 3D placement/sign/anchor не проверена. Docs/generated/commit/push/закрытие отложены. Current: Skirting_ReadFaces выбирает внешнюю плоскость по ориентированной нормали, сохраняет конечные контуры/отверстия; чтение после Sync вне undo без кэша подготовки. Нет snapshot/контакта/избранного/класса — нет подмены геометрией зоны. Старые классифицированные балки исключены из раннего удаления; замена после успешной подготовки одной транзакцией на комнату. Длины считаются по успешно созданным 3D-отрезкам, в свойства не публикуются. Требуется класс с описанием some_stuff_fin_beams и доступные свойства Sync_GUID zone/base element. Evidence: scratch/skirting270-dispatch/{before/,build.log,lsp.log,dispatch.diff}. Clangd MCP отдаёт устаревшие диапазоны (новые функции отсутствуют), его0 не использовать; свежий CLI exit3, диагностика отдельно. Next Step: завершить source-review и передать собранный APX на ручную проверку. Last Checkpoint: нового нет.

Продолжение writer, 2026-10-24: владелец согласовал балки; default избранные ТОЧНО `smstf plinth down` / `smstf plinth up`. Scope дельты — настройки этих имён, native writer по явному 3D-отрезку с сохранением сечения/профиля/материалов/привязки избранного, адресные проверки. Никакого fallback на beam defaults при отсутствии/неверном типе избранного. Существующий writer откосов read-only: он перезаписывает размеры/материалы и здесь не подходит. Автоматический сбор финальных контактов/dispatch, классификация/связи/cleanup-repeat и публикация длин — следующий отдельный проверяемый шаг #270; не выдавать writer за готовую функцию RoomBook. Docs/generated/commit/push/закрытие остаются отложенными.
Plan writer: [x] source/SDK review preparation/native writer (замечаний в проверенном объёме нет); [x] SDK-backed регрессии сохранения избранного и отказов; [x] canonical AC25 build и свежая LSP-дельта; [ ] BLOCKED: native 3D placement/sign/anchor acceptance. Last Checkpoint writer: нового нет.
Current Work writer: SkirtingBeam_Prepare и SkirtingBeam_Draw добавлены, production callers ещё нет. SDK25 console baseline59/0 → current81/0, прежние59 ordered rows совпали; после снятия native draft повтор current81/0. Scratch-only mutation story-relative→absolute: compile0/run1, 3 FAIL — ожидаемый negative control. Последняя canonical BuildAddOn.py AC25 Windows Debug после снятия native draft: compile/link exit0, APX c1e8c412e124c678f6b30727070e766165d5a36dfbab770b2b75d8677139b53d. Draw fresh CLI AST до/после: те же4 unused-variable, exit3; не полностью GREEN LSP. Вне новых функций Data/Draw и defaults/declarations header прежнее содержимое сохранено (проверка без пустых строк); TestFunc.cpp/.hpp восстановлены побайтно. Начальные неверные std begin/end для GS::Array и enum APIBeam_Straight сняты и заменены проверенными range-for/API_StraightBeam до успешных проверок.
Native gate BLOCKED: два контролируемых startup TestSkirtingBeamNative — 12pass/3fail внутри undo и 13pass/3fail при чтении после undo. Native create, чтение типа/этажа/level/anchor и удаление временной балки/избранного прошли; RoomSurfaces_Read не выдал Extracted, поэтому знак уклона и actual 3D placement not verified. Причина snapshot-отказа не установлена: stage/code в этих черновиках не записаны; не объявлять SDK limitation. Повторы остановлены. Native draft сохранён в Hermes scratch/skirting270-beam/TestSkirtingBeam.native-draft.cpp, из сборки и реестра снят; только свой тестовый PID32844 завершён, затем подтверждено отсутствие Archicad. Исходный PLN не сохранялся; оба fixture SHA256 ad99afc4a08d8617fe720b1927716ad18cd8c519f20f04bbc0e8e8486db72dce неизменны. Runner дважды вернул70 по СТАРОМУ TEMP отчёту168fail, это НЕ результат адресного набора: свежие отчёты записаны в контролируемый TMPDIR и привязаны к source/APX/starttime.
Evidence writer: scratch/skirting270-beam/{verify-result.py,validation.json,final-canonical-build.log,green-result.json,before-lsp.log,after-lsp.log,native-red-report.txt,native-red-report-2.txt,native-red-evidence.json,native-red-evidence-2.json,native-source-manifest-2.json}. SDK-review: реальные header/contracts проверены; examples не доказывают знак slantAngle/anchor semantics. Независимое source-only review завершено: No issues found in inspected scope; родитель сверил текущие source hashes с исполненной SDK-console регрессией, итог в source-review.md. Next Step writer: отдельная диагностика native snapshot должна первым делом записать status/errors stage+code в доступном после открытия проекта контексте, без слепого третьего повтора. До этого не подключать автоматический dispatch, не публиковать длины и не считать writer окончательно принятым. Last Checkpoint writer: нового нет; commit/push/закрытие не выполнялись.

Продолжение 2026-10-24: AC25 Windows, https://github.com/kuvbur/AddOn_SomeStuff/issues/270. Текущая дельта — геометрическое ядро конечного 3D-стыка двух явно выбранных граней отделки, с отверстиями обоих участников. Повторную подрезку подготовленных стен не добавлять. Существующий сборщик кандидатов и writers сохраняются. Автоматический выбор комнатной грани, чтение созданных элементов в undo-команде, native writer/избранное, публикация длин и host-приёмка не входят в эту дельту; они остаются следующей интеграцией. Docs/generated, commit/push и закрытие issue не выполнять.
Plan текущей дельты: [x] точный расчёт линии пересечения и отсечение в локальных плоскостях обеих конечных граней; [x] SDK-backed адресные регрессии, повтор/отказы и прежний сборщик; [x] clang-format, свежая LSP-дельта, canonical AC25 build и независимое source-review. Дельта ядра завершена в этом объёме; #270 остаётся IN_PROGRESS.
Current Work: SkirtingSegment3D и SkirtingContact_Build добавлены в Roombook.hpp/RoombookData.cpp; новых production callers нет. SDK25 console baseline24/0, current59/0, прежние24 ordered rows совпали. Scratch-only mutation без отсечения конечным полом: compile0/run1, семь FAIL, прежние24 PASS. Начальный compile-error нового тестового блока (два разных broken в одной области) исправлен отдельной областью видимости; не считать behavioral RED. Canonical BuildAddOn.py -c config.json -v25 Windows Debug compile/link exit0, APX ddadd14769d4a708a4be0658aa8bfddaeb17c5f8761ecfa56d9d3eb68eac4e0f. clang-format/dry-run/scoped diff-check PASS. Fresh CLI AST: до/после те же3 unused-variable, CLI exit3, check-lines=1-1 ограничивает tweaks; полностью GREEN LSP не заявляется. Тело Data вне добавленной функции и header вне двух добавлений побайтно совпадают с before; остальные6 roombook snapshots неизменны. Host не запускался, исходные PLN не сохранялись; canonical build штатно копировала test PLN в Build. Evidence: Hermes scratch/skirting270-contact/{run-sdk-regression.py,verify-result.py,validation.json,baseline-result.json,green-result.json,mutation-result.json,canonical-build.log,after-lsp.log}. Review: независимое source-only — No issues found in inspected scope; родитель сверил неизменность текущих source hashes с исполненным green-result.json, результат в source-review.md. Next Step: подключать только успешные созданные поверхности с доказанным выбором комнатной грани и согласовать writer/избранное; totals не считать по предварительным кандидатам.
Last Checkpoint текущей дельты: нового нет; HEAD6f131e1 ([C2-WIP], не коммит #270). Commit/push/закрытие issue не выполнялись, чужие изменения сохраняются.

Scope: AC25 Windows; OtdBeam и два массива в OtdRoom, первый согласованный этап — чистые нижние отрезки итогового otdwall. Без повторной подрезки/rise и 3D-чтения. Исходные стены/проёмы read-only; чужие #288/#271 сохранены. Docs/generated, commit/push и закрытие остаются отложенными согласно общему Scope.
Status: IN_PROGRESS для #270; согласованный подготовительный этап реализован и проверен. Native-плинтус пока НЕ создаётся; потолочный массив добавлен, но алгоритм его заполнения не реализован.
Last Completed: OtdBeam и otdbeam_floor/otdbeam_ceil — Sources/AddOn/roombook/Roombook.hpp:172,231–232; BuildFloorSkirtingSegments — RoomSurfaceFinish.cpp:15, вызов в ProcessRoomFinishes после окончательной обработки стен/дедупликации. Выбор по равенству низов в допуске1e-6, независимые wall/column flags; откосы и slab-типы не берутся. CutWallBottom в RoomAnalyticGeometry.hpp:86 вычитает объединение нижних проёмов, включая null base_guid. Метаданные: этаж, GUID/тип основания, индекс итоговой стены; созданный GUID не выдумывается. Повтор заменяет массив; ошибка оставляет напольный массив пустым, без частичного результата.
Validation: canonical BuildAddOn.py -c config.json -v 25 Windows Debug compile/link PASS после всех source edits (final-build-all.log); предупреждение SDK LNK4099 об отсутствующем API_c.pdb. Analytic regression46/0 (новые случаи сначала36/10 RED, после реализации46/0 GREEN); SDK-backed console regression23/0 с реальными OtdRoom/GS и AC25 DLL. Runner механически извлекает точное текущее тело production-функции и фиксирует hashes; весь RoomSurfaceFinish TU отдельно с console не линкуется из-за несвязанных addon-only функций, заглушки не добавлялись. Это НЕ runtime-приёмка RoomBook/создания элементов в Archicad.
LSP: свежий CLI прочитал текущие production TU; RoomSurfaceFinish source diagnostics0, Roombook семь unused-variable, code/message полностью совпали с предыдущим реальным логом #288. CLI exit3 из-за внутренних ExtractFunction probes; полностью GREEN LSP не заявлять, устаревшие MCP0 не использованы как доказательство.
Plan:
- [x] В OtdRoom добавлены length_beam_floor и length_beam_ceil (double, начальное значение0, метры; Roombook.hpp:233–234). Только поля данных; подсчёт/публикация длин в этом шаге не подключены.
  Проверка текущего снимка: clang-format и scope-diff PASS; SDK console regression24/0, включая нулевую инициализацию обоих полей; canonical BuildAddOn.py -c config.json -v 25 Windows Debug compile/link PASS. В приложении Archicad не запускалось. Evidence: Hermes scratch/skirting270-length-fields/{tests.log,build.log,validation.json,before-manifest.json}. Last Checkpoint остаётся6e54fa2, нового коммита нет. Next Step: подключение расчёта длин к согласованным результирующим отрезкам при продолжении #270; прежние source/artifact manifests относятся к предыдущему этапу, актуальный снимок указан в validation.json этого шага.
- [x] OtdBeam, два массива, чистые напольные отрезки и production-вызов.
- [x] Исполнить регрессии геометрии/metadata/повтора/отказов, clang-format, проверить LSP-дельту и canonical AC25 build.
- [x] Review собственной дельты и сохранение состояния; посторонние изменения не откатывать.
- [ ] Следующие этапы #270: окончательное ограничение по реальному полу, согласованный потолочный алгоритм, выбор избранного/native writer, длины, классификация/связи, обновление/повтор и host-приёмка. Наличие полей/кандидатов этого не доказывает.
Evidence: Hermes scratch/skirting270-implementation/{before/,before-manifest.json,final-source-manifest.json,final-build-all.log,analytic-test.log,native-result.json,native-test.log,lsp-result.json,own-delta.diff}. Воспроизводимый SDK console runner: run-sdk-regression.py; тесты в Sources/AddOn/tests/SkirtingRegression.cpp. Native host не запускался, PLN не сохранялись. Memory: обновлён lightrag/references/somestuff-skirting-polygon2d-ac25.md.
Last Checkpoint: 6e54fa2 — существующий HEAD; нового коммита нет, #270 не закрыта.
Next Step: по команде владельца продолжить использование подготовленных нижних отрезков и согласовать следующий этап; не выдавать candidates за созданные плинтусы/подтверждённый контакт.

### #271 — наклонные многослойные полы по Mesh

Scope: AC25 Windows; поиск Mesh совместно с существующими Slab, конечные верхние грани сеток, подготовка наклонного пола, native writer, материалы/площади, классификация и связи, удаление прежних результатов и повтор. Потолки по Mesh, исходные конструкции, сохранённые PLN, незавершённые #252/#268/#272/#285 и реорганизация C1–C9 вне этой задачи; чужие незакоммиченные правки сохраняются. Issue #271 OPEN. Коммит/push/закрытие без отдельной команды не выполнять. Scoped Docs/generated нужны после проверенной реализации, не вместо неё. Отдельная tests-only правка #286 согласована и выполнена; issue остаётся OPEN по правилу владельца.

Status: IN_PROGRESS. Владелец закрыл Archicad и запросил сборку через VS. AC25 Windows Debug x64: SomeStuff target реально перекомпилировал RoombookData.cpp и перелинковал APX; Done/FailedProjects0, Error List0. Main-tree canonical BuildAddOn.py и native-повтор Z не выполнялись. Mixed provenance проверен только в scratch; production Mesh OFF.
Last Completed (2026-10-18): 271-4c-b. MixedFloorGeometry (RoomSurfaceFinish.hpp:59–78), staged BuildFiniteMixedFloorGeometry (RoomSurfaceFinish.cpp:1731–1799), все компоненты legacy\D, raw role-qualified Slab supports и changedDomain. Старые тела/декларации/тестовые строки сохранены: относительно before-mixed-floor-merge только вставки 142/22/300 строк в cpp/hpp/test. Новые18 checks: RED837/8→GREEN837/0, prior819 и все имена RED/GREEN совпадают порядково; adjacent301/0+28/0 exact. Независимые Slab steps/metadata сохранены; production callers, Roombook.cpp, потолки и исходные PLN не менялись.

Current Work (2026-10-26): MeshFloorFinishes_BuildProjectedImpl/WithProvenance сохраняет original/admitted/winner finite domains с owner/face refs и append offsets; старый nullptr-маршрут сохранён. SDK-console33/0 и broad57/0, mutation32/1; независимое source-review No issues found in inspected scope, report SHA256 f1553aaee690449b560cafee6fd5fceaa5b29cc5a34638492fae4a33db6e5511. Canonical BuildAddOn.py AC25 Debug Windows exit0 (11:35:51), APX76494e5ca828a021a6650c7aeaf45a531caaba4b3d3bc72d4d95c236e82e6b93. Native PID38116: TestMeshFloorNative848/0, все9 новых provenance PASS; transform28/0; partition319/1, общий1195/1 — не GREEN. Источник соседнего падения подтверждён: SurfaceWall_ClampToParts обнуляет Z; helper побайтно совпал с историческим RED-source от2026-10-25. Источники/APX/оба PLN сверены через F5 и остановку; своя точка удалена, две прежние disabled сохранены. Evidence: Hermes scratch/271-resume-parent/{review,canonical-20261026-113545,native/result.json,native/after-stop-result.json,native/somestuff_test_report.txt,native/controlled-debug.txt}. Свой debuggee остановлен, VS Design.
Validation/Follow-up (2026-10-26):
Владелец через clarify отдельно разрешил исправить потерю Z в SurfaceWall_ClampToParts и продолжить #271. В RoombookData.cpp:2197–2200 к двум XYZ-присваиваниям добавлены исходные Z начала/конца; XY/интервалы/проёмы/контактная политика не менялись. Весь Data вне helper побайтно совпал с pre-fix snapshot. Issue #252 уже покрывает helper (dedup подтвердил); отдельного дубля нет. Полные old/new функции с реальными SDK25/Otd-типами и21 неизменной постоянной wallPart-проверкой плюс24 независимых signed-height/направление checks: MSVC C++14 compile0, RED45/11 → GREEN45/0, все старые PASS сохранились порядково. Первоначальный parser IndexError вызван пустыми строками CRLF в scratch-log; исправлен только фильтр записей, обе компиляции/исполнения повторены успешно. clang-format/scoped diff-check PASS. Свежий clangd CLI: те же3 unused-variable, новых source errors нет; exit3/check-lines1-1, не объявлять полностью чистый LSP. После правки Data SHA e789887bc0d1438388098b9c6f888b017ae5ef12dc99e86009aee2522a04ccfe.
Plan: [x] native RED/source-диагностика; [x] разрешённая минимальная правка и SDK25 RED/GREEN/locality/format/fresh LSP; [ ] canonical build/native GREEN после освобождения host; [ ] вернуться к mixed provenance #271.
Повторный canonical wrapper остановлен до компиляции/PLN copy: обнаружен рабочий ARCHICAD PID33512, BIMcloud-проект ВОС_Алеут_АБК. VS Design, процесс не запускался и не останавливался этим агентом. Clarify истёк без ответа; разрешения трогать рабочий host нет. Compiled после Z: SDK25 standalone и полная изолированная AC25 Debug Windows compile+link (exit0); addon canonical в рабочем дереве не выполнена. Tested после Z: только standalone, native не выполнен. Изолированная сборка 26.10.2026 через неизменённый BuildAddOn.py собрала точный снимок текущих sources и APX9125e23099363876662df5d0f8ba8c439bf8250d5f881a94733f7c1461c68298; реальные compile/link подтверждены build.log, LNK4099 missing SDK PDB сохранён как warning. Live sources, canonical APX и оба PLN не изменились; рабочий PID33512 остался открыт. Это не main-tree canonical/native acceptance и не разрешение включать Mesh. Evidence: Hermes scratch/271-isolated-z-20261026-154317/{preflight.json,result.json,build.log}, 271-resume-parent/isolated-z-acceptance.md. Mixed candidate проходит отдельные scratch SDK25 regression/negative-controls и независимый review; live source-шаг не применяется до проверки gates. Предыдущая успешная canonical/native относится к Mesh-provenance до Z. Evidence: Hermes scratch/271-resume-parent/native/z-fix/{before.cpp,own.diff,run.py,standalone-result.json,red,green,source-hashes.json}; fresh log — 271-resume-parent/z-fix-lsp.json.
Отдельная ошибка представления PASS=false в TestKit заведена как #292 (прочитана обратно); исправление вне scope. Результат определяется реальным cond и reconciled PASS/FAIL/END/SUMMARY, не напечатанной строкой actual. Memory: archicad-debug-workflow/references/testkit-report-vs-debug-channel.md — exact native-filter через CRT только в своём debuggee, scratch report и граница PASS=false.
Last Checkpoint: нового нет; commit/push/закрытие не выполнялись. Scoped Docs/generated отложены до проверенной интеграции #271; они не обновлялись этим шагом.

Plan:
- [x] Прочитать точную issue/предыдущее обсуждение, правила и карточку модуля; проверить live consumers и установленный SDK25.
- [x] Принятый 2026-10-16 контракт: односкатные Roof; состав/толщина/общие настройки выбранного Slab-избранного без нового обязательного имени; Object — функциональный отказ до изменения модели без подмены; локальная верхняя отметка опоры; совпадающие разные основания — неоднозначность, не GUID tiebreak.
- [x] Native-прототип: контакт, finite exterior/holes, две разности нижнего покрытия, нормальная толщина, материалы/слои против независимого Slab, старые и новые стыки, удаление временных GUID. Девять исходных сценариев 526/0, затем расширение до 778/0, включая single-slope skirt=2/3.
- [x] Mesh-admissibility: solid — локальный низ тела<=полу зоны; без дна — верх<=полу. SlabRole/допуск Slab не меняются.
- [x] Геометрическое ядро и typed discovery: canonical/fresh LSP/host 778/0; baseline после tests-only #286 partition301/0 и transform28/0.
- [/] Production интеграция: общий Mesh functional preflight до cleanup/activation; не включать Mesh geometry до готовности consumers и dispatch.
  - [x] 271-4a: выделить Prepare/Create без изменения публичной сигнатуры/алгоритма; fresh LSP0, canonical exit0, host778/0+301/0+28/0.
  - [x] 271-4b: read-only Favorite lookup без defaults/activation; missing-request RED808/2→GREEN808/0, PRESENT-invalid empty-template extension GREEN819/0; writer/attempt-flag unchanged, full prior808 и adjacent301/0+28/0 preserved.
  - [/] 271-4c: конечный merge Mesh+raw Slab supports, Slab-only/ceil exact delegation, все компоненты legacy\D; сохранить независимые Slab steps; affected seam/extension без доказанного adapter — explicit refusal, не blanket suppression.
    - [x] 271-4c-a: private raw Slab collector выделен; ordered prefix/tail/public signature/literals/outside scope unchanged. Fresh isolated LSP0/0, canonical AC25 compile+link exit0; native819/0 + partition301/0 + transform28/0 с полными идентичными ordered inventories.
    - [x] 271-4c-b: staged mixed merge и18 независимых checks hidden Mesh/holes/raw contacts/metadata/atomic refusal/реальных independent Slab steps. RED837/8→GREEN837/0; prior819 и adjacent301/0+28/0 exact; свежий LSP0/0/0, canonical AC25 exit0. Production Mesh route OFF.
    - [/] 271-4c-c: Mesh finite provenance source/canonical/native GREEN в указанном объёме; full mixed Slab provenance/remap и staging opening/core rules ещё не интегрированы. Соседний partition RED привёл к отдельно согласованной минимальной Z-правке (#252); standalone GREEN, canonical/native после неё отложены. Сохранить legacy opening caller после Param_SetToWindows и до reveal guards, objLoc/wall rise/нулевые core contacts и ambiguity/miter policy; singular intersection→Unresolved. Не добавлять surface-opening caller; scalar zBottom/пустая delta недостаточны. Финальные core bands — до activation; отказ координатору до cleanup.
  - [ ] Подготовленный native Roof batch/dispatch, совместимые составы/толщины и корректное распространение новых отказов до destructive write.
  - [ ] Маркированные Favorite слои и прежние material rules, площадь наклонного contact footprint.
  - [ ] Floor-классификация, реальные Mesh/zone links и native Roof whitelist; поиск/удаление прежних результатов и повтор.
- [ ] Geometry/host GREEN+RED, Slab/ceiling regression, canonical AC25 build и end-to-end runtime/repeat; затем scoped Docs/generated и статус приёмки.

Verified: SDK25 ACAPinc.h:1346 — ACAPI_Favorite_Get(API_Favorite*) -> GSErrCode; APIdefs_Favorite.h:37–55 и shipped HTML: по имени читаются параметры без создания элемента. Сверены live source/call sites, модульная карточка/DISCREPANCIES и LightRAG Document Chunks. На 22:42 2026-10-18 LightRAG снова доступен; favorite-get-readonly-rag.json содержит ответ, прежний outage — исторический. SDK25 Dispose header/HTML discrepancy сохраняет приоритет header. No issues found in inspected scope; не verdict для всего проекта.
Validation 2026-10-18 23:42: Compiled yes — canonical BuildAddOn.py -c config.json -v 25 Windows Debug x64, реальный RoomSurfaceFinish.cpp compile+APX link exit0 после GREEN. Fresh isolated LSP0/0/0, актуальные AST1731–1799/MixedFloorGeometry/RunMeshFloorNativeChecks и source/CDB hashes. Tested SDK-host native837/0, partition301/0, transform28/0 в PID12700; prior819 и все adjacent ordered rows exact. В актуальной Отладка18/18 новых mixed DBtest PASS; это не полный TestKit и не production/end-to-end #271. Loaded APX0fb368eb72afb8a3c607fcc1cd8414749c4757da27b6da61aeb1b5ea0ed9fd05; все9 source hashes exact, тесты между RED/GREEN не менялись. Scoped clang-format zero replacements/diff-check PASS. Оба PLN exact; build-copy восстановлен после canonical copy при hosts0. Своя VS-сессия остановлена, Design/hosts0; preexisting Propertycache.cpp:1011 восстановлен и подтверждён свежим списком. Review: No issues found in inspected scope.
Evidence: Hermes scratch/271-native-prototype/{before-mixed-floor-merge/manifest.json,mixed-floor-{red,green}-fresh-lsp.json,mixed-floor-native-{red,green}.json,mixed-floor-native-{red,green}.manifest.json,mixed-floor-partitionSelfTest.json,mixed-floor-transformSelfTest.json,mixed-floor-regression-summary.json,mixed-floor-review.json,mixed-floor-breakpoint-baseline.json,mesh-extension-applicability-proposal.cpp,mesh-extension-applicability-evidence.ru.md,projection-fixtures-20261018/fixture-manifest.json}. Current GREEN Debug spillover call_RulXb6vi4UEWYHhBD03L4crm.txt (23:41:51 launch). Исходный MSVC C3493 нового lambda — ошибка тестового черновика, не behavioral RED; черновик сохранён, только новый блок откатан, restored canonical build PASS, явный capture→PASS без смены expectations. Memory: saved в lightrag somestuff-mesh-shell-ac25.md и archicad-cpp-tdd-patterns first-red-of-new-suite.md. Старые extraction/Favorite reports сохраняют исторический статус.
Limits: GREEN относится к выполненным SDK-host geometry/native fixtures. Production Mesh route, full finite support provenance, opening/core preflight, mixed native seams, arbitrary saddle/mixed узлы/разные толщины, material/area/links/classification/cleanup-repeat — not verified. PRESENT-invalid coverage не расширяется на arbitrary templates/element-dependent values/поздний !isValid. A/B времени не выполнялся; staged helper production не вызывается. Scoped Docs/generated отложены до реализации по Scope и остаются в271-5. Scratch-only23 applicability checks ещё не применены и не запускались.
Last Checkpoint: нового нет; текущий HEAD6f131e1 — WIP #260, не checkpoint #271. Коммит/push/закрытие не выполнялись и требуют отдельной команды.
Current Work 2026-10-26: mixed provenance доведён в отдельном scratch-кандидате до integrated SDK25 C++14 GREEN97/0; все51 исходных ordered PASS сохранены. Полные prefix/domain/steps/rejectedArea сравнения, native Slab holes/face IDs, per-floor finite coverage/plane identity, одинаковый Slab owner/XY на разных отметках, Zone residual, D.empty и disconnected indices выполнены. Предположение review о RemoveHoles вне D опровергнуто отдельными положительными fixtures: готовый Slab сохраняет отверстие; hole-removal относится к remainder. Arc F1 устранён только в scratch: прежний подтверждённый RED69/3→GREEN69/0 сохраняется. С теми же97 test rows/SHA отрицательные контроли дали runtime RED: offsets4, inside reset2, Slab-plane gate1. Дополнительный arc-RED97 не принят: после несовпадения needle и C2660 неверной арности helper ветка стенда снята по loop guard, логи сохранены; это не behavioral RED и не дефект кандидата.
Validation 2026-10-26: свежие отдельные clangd CLI для baseline/candidate дали одинаковые inventories:3 unused-variable в Data и1 unused-lambda-capture в Test; каждый exit3, clean LSP не заявляется. Финальный исправленный кандидат с текущими параллельными beam eligibility hunks собран неизменённым BuildAddOn.py в отдельном полном workspace: AC25 Windows Debug compile+link exit0, APX92de599e6e649bf0d4f66c3ba30ab43f944414ca3f4cb9bff2bd6b487c40e438. Подтверждены реальные Data/TestMeshFloor/TestRoombookGeometry compile и APX link, остаётся SDK LNK4099. Live source hashes, canonical APX и оба PLN сохранены. Scoped candidate patch проходит git apply --check --whitespace=error-all; в live C++ не применён. Production caller остаётся OtdSlab_BuildProjectedFinishes, Mesh route OFF.
Evidence: Hermes scratch/271-mixed-provenance-parent-run/{candidate-final.patch,candidate-final-manifest.json,final-acceptance.json,integrated-green,integrated-no-offsets,integrated-bad-inside,integrated-bad-slab-plane,fresh-lsp-summary.json,arc-red,arc-green-minimal}; полный build:271-isolated-mixed-final-20261026-162637/{build.log,result.json}. Независимое заключительное source-review завершено: No issues found in inspected scope, 271-final-focused-review/review.ru.md SHA503e96ea28ce95a90187001a88b24f79dd46aebd9fc6f530b17b84b263e53379. Parent review-source-binding.json точно воспроизвёл извлечённые production.cpp/tests.cpp всех4 прогонов из текущего candidate; SHA полного TestMeshFloor.cpp и generated tests.cpp относятся к разным артефактам. Это не live/native acceptance.
VS validation 2026-10-26 16:44: SomeStuff.sln/SomeStuff.vcxproj, Debug x64, успешных2/ошибок0; реальная компиляция Data и APX link, не up-to-date. APXd6a8ec7eda3a9c72cfba03911f9156f4fc3e300c8e7bf1867442bdfeccb6be0e, mtime обновлён. Четыре проверенных исходника и оба PLN сохранили SHA; scratch mixed не применён, Archicad/тесты не запускались. Evidence: Hermes scratch/271-vs-build-20261026-164416/{before.json,result.json}. Это VS build, не canonical/native acceptance; коммитов нет.
VS launch 2026-10-26: по команде владельца текущая AC25 Debug x64 запущена через debugger_launch на test_25.pln. Подтверждены один PID48096, открытый test_25, VS Run без остановки, загрузка D:/SomeStuff_addon/Build/SomeStuff/25/Debug/SomeStuff.apx с прежним SHAd6a8ec7eda3a9c72cfba03911f9156f4fc3e300c8e7bf1867442bdfeccb6be0e. Девять исходников, APX и оба PLN hash-stable. Свободный старый .lck после hosts0/exclusive-open перенесён в scratch, не удалён; две прежние disabled точки сохранены, своих нет. Evidence: Hermes scratch/271-vs-launch-20261026-165137/{before.json,result.json,breakpoints-before.json,stale-test_25.pln.lck}. Подтверждён запуск/загрузка, не прохождение тестов.
Next Step: Archicad PID48096 оставлен работающим под VS для проверки владельцем. Результаты функциональных/native-тестов ещё не оценены; без отдельной команды не останавливать процесс и не запускать runner/build параллельно отладке. Mixed provenance остаётся в scratch, Production Mesh OFF. Адресная проверка Z и canonical BuildAddOn.py остаются отдельными gates перед переносом mixed; коммит/push не выполнялись.



### Ревью wallPart-маршрута обычных стен — #252, 2026-10-16

Status: REVIEW_COMPLETE / WAITING_FOR_FIX. Найдены R1 (дублирование ordinary-стен/откосов через `otd.otdwall` + openingWalls, Roombook.cpp:867–868/:1315/:1470–1473), R2 (legacy-участки сложных стен доходят до основной отделки, :957–976), R3 (`IsOrdinaryWall` обходит native-fallback generic-проёмов API_OpeningID, :496–505/:1273–1277). Plan: [x] чтение и проверка потребителей; [x] SDK25 `Array::Append` подтверждён; [ ] исправление R1–R3 по отдельной команде; [ ] canonical build и интеграционная проверка кратности/площадей/проёмов/повтора. LSP0, diff-check PASS; build/test в ревью не выполнялись. Next Step: отделить основные стены от служебных opening owners и сохранить неподдержанные аналитикой вырезы. Last Checkpoint: f86e0de.

### #285 — вход существующего пола в проёмы

Scope: AC25 Windows. Только контуры OtdSlab типа Floor, проёмы подготовленных отделочных стен; глубина base_reveal_width. Отдельных OtdSlab не добавлять, исходные конструкции и сохранённые PLN не менять. Остальные незакоммиченные изменения сохраняются. Docs/generated и commit отложены согласно текущему Scope.

Status: WAITING_FOR_NATIVE_TEST. Реализация и адресные SDK-host проверки выполнены; полноценный RoomBook с чтением созданных native-плит и повтором не выполнялся. #285 OPEN; полного завершения/приёмки не заявляется.

Plan:
- [x] Уточнить требования и проверить issue #285.
- [x] Исправить потерю рассчитанной глубины: Param_SetToWindows меняет копию в wallsForRules, поэтому ExtendFloorContourToOpening вызывается сразу после него, до обоих reveal-enable guards (Roombook.cpp:905–911).
- [x] Расширять только существующий floor.poly на конечных участках пересечения линии проёма с этим полом (CutElem/Boundary_Closed → Unify); не создавать плит и не выбирать первый пол по площади. При неуспешном объединении сохранить исходный полигон. Определение/объявление/callers согласованы (RoomSurfaceFinish.cpp:22–72, RoomSurfaceFinish.hpp).
- [x] 27 адресных host checks PASS: низ равен/ниже/выше зоны, оба флага откосов, неверные/нулевые данные, несколько полов и частичный контакт, отверстие, поворот/разворот, повтор и metadata (TestRoombookGeometry.cpp, блок floor opening).
- [x] clang-format и git diff --check PASS; свежие MCP clangd reports для трёх cpp — 0 diagnostics; current document symbols соответствуют новому helper. Canonical BuildAddOn.py AC25 Debug compile/link success 2026-10-16, PYTHONPATH= с проверенным C:/Program Files/Python310/python.exe. APX SHA256: 005fccfe553ea532432cd63064201e8f785d190c2b1c0b2b6b6dabcc651afe8c.
- [x] В одном собственном VS debug экземпляре test_25/PID5948 проверены loaded module path Debug/SomeStuff.apx и JSON port19723; partitionSelfTest вернул 283 checks, 3 FAIL, все 27 новых PASS. DBtest подтверждены актуальной панелью «Отладка».
- [ ] Общий selftest НЕ GREEN: FAIL «3D zone section extends beam bottom, top and both sides beyond lower-profile beamPart» (:747), «zero-source ceiling residual uses supplied main-plus-down nominal plane» (:1555), «ceiling internal step spans source lower face to nominal plane» (:1623). Они выполняются до нового helper, предшествующий тестовый код byte-equivalent после нормализации переводов строк с pre-resume snapshot; причины не расследовались и здесь не исправлялись. Отдельного runtime baseline до изменений нет — не выдавать это за доказанный before/after.
- [ ] На согласованной безопасной фикстуре: штатный RoomBook + repeat; независимо прочитать native-полигон существующего пола, глубину захода, сохранение hole/links/favorite, отсутствие отдельного участка и исходных изменений. Только после этого возможна native-приёмка; runner перед закрытием issue, Docs/generated/commit — по отдельной команде.

Evidence: Hermes scratch/floor-opening-285-resume/{build-evidence.json,partition-selftest.json,validation-summary.json,debugger-baseline.json}; актуальные DBtest в spillover/call_a6C7eAZeDeCYitob9la7Enwv.txt. Предсборочная build-copy PLN сохранена и её SHA256 ffd6af0dddeb61bdbe1cc5e293f13d3b5a341b2ca72b1814e2136376447289cd проверен перед запуском. RoomBook не запускался, PLN не сохранялся. Собственная debug-сессия завершена; исходная breakpoint Propertycache.cpp:1011 восстановлена enabled, посторонние точки не удалялись.
Last Checkpoint: f86e0de (существующий; нового коммита нет).
Next Step: по команде владельца — native-проверка #285; общие FAIL балки/потолков рассматривать отдельно, не расширять scope #285 автоматически.

### Читаемость и минимальная структура — часть текущего рефакторинга

#### Цель и границы

По решению владельца целевая структура — четыре файла реализации по назначению: общий; сбор и анализ данных; расчёт площадей для записи в свойства; отрисовка элементов. Предыдущая схема сохранения отдельных Surfaces/Partition/Finish как самостоятельных production-модулей отменена. Новая issue не нужна: это часть текущего рефакторинга #252 и связанных #272 (ранее также #267/#268). Проверяемая платформа — AC25 Windows, остальные SDK/macOS отдельно.

Реализация начата с C1: инвентаризация, карта переноса и baseline без изменения поведения. Взаимодействие с Archicad, коммит/push и закрытие issue не выполняются без отдельной необходимости/команды. Текущие пользовательские/параллельные изменения и незавершённые функциональные задачи сохраняются. CommonFunction/Helpers/Sync/ReNum, ресурсы, исходные конструкции и сохранённые PLN вне scope; внешние callers можно минимально поправить при будущей смене include/границы. Docs/generated обновляются только по ранее установленному отдельному разрешению.

#### Главная цель: поиск ошибок человеком с небольшим опытом C++

Рефакторинг нужен не ради архитектурной чистоты, количества файлов или коротких функций. Человек, знающий предметную задачу отделки, но мало разбирающийся в C++, должен по видимому симптому найти ответственное правило, проследить значения и понять, почему элемент пропущен, изменён или создан неверно. Четыре файла — средство навигации, а не самостоятельный критерий успеха.

Для каждой предлагаемой правки сначала назвать конкретную трудность чтения и показать, как правка её устраняет. Если после переноса приходится делать больше переходов и держать в голове больше скрытых зависимостей, задача не решена. Не вводить ради рефакторинга шаблонные конструкции, цепочки callback/lambda, менеджеры, универсальные обработчики и дополнительные слои. Существующие SDK-конструкции и необходимые тестируемые ядра сохранять; не переписывать их только из-за сложности C++.

- Путь обработки одной комнаты и одного элемента читается последовательно: откуда взяли данные → какое правило применили → что подготовили → что создали → что записали в свойства. Реальный порядок выполнения сохраняется, даже если он не совпадает с этой поясняющей схемой.
- Предметные участки имеют короткие русские заголовки: стены, проёмы и откосы, полы, потолки, балки, материалы, площади, создание и связи. Имена и комментарии позволяют найти участок обычным текстовым поиском; массовое переименование не требуется.
- Для неочевидной функции/структуры пояснять смысл входа и результата, единицы и систему координат, изменяемые данные и значение отказа/пустого результата. Не пересказывать синтаксис и не заменять понятный код длинными комментариями.
- У каждого существенного отсева должно быть понятно, какое предметное условие не выполнено и что происходит дальше: пропускается элемент, сохраняется исходный контур или прекращается весь запуск. Различия между ошибкой, отсутствием данных и неприменимостью правила не скрывать.
- Значения, определяющие решение, должны быть доступны как обычные именованные переменные в отладчике. Не прятать предметное условие в длинном выражении, побочном эффекте или цепочке вызовов. Раскрывать сложный участок только отдельной проверяемой дельтой без изменения порядка вычислений.
- Не дробить понятный последовательный алгоритм на однострочные wrappers ради размера функции. Выделять функцию, когда она выражает самостоятельное предметное действие или убирает реально мешающий повтор; связанные helpers держать рядом.
- Удаление диагностической обвязки по C3 сохраняется. Вместо нового logging/JSON-слоя — понятные данные, условия, обычные точки останова и необходимые пользователю сообщения об отказах. Тесты остаются независимой страховкой поведения.

#### Целевая организация файлов

Все пути ниже относительно Sources/AddOn/roombook. Названия новых файлов — предложение; смысл четырёх частей задан владельцем.

| Файл | Содержание | Чего в нём не должно быть |
|---|---|---|
| Roombook.cpp — общий | RoomBook, последовательность этапов, общий прогресс/отмена, управление жизненным циклом запуска, координация финальной записи | Обхода полигонов, расчёта площадей, тел writers и длинных предметных алгоритмов |
| RoombookData.cpp — сбор и анализ | Чтение зон/оснований/проёмов/свойств/избранного, run-local кэши, классификация входов, выбор оснований, analytic/native, сопоставление поверхностей, подготовка геометрии OtdWall/OtdSlab, правила материалов/высот/откосов | Создания/удаления элементов и записи рассчитанных площадей в модель |
| RoombookQuantities.cpp — площади и свойства | Расчёт площадей готовой отделки, группировка по материалу/виду/типу помещения, форматы столбцов и подготовка значений свойств | Повторного сбора геометрии из SDK, изменения подготовленной геометрии или создания элементов |
| RoombookDraw.cpp — отрисовка | Draw_Elements, стены/балки/объекты/проёмы/перекрытия, defaults для создания, удаление старой отделки, классификация результата, формирование связей с зоной/основанием | Повторного выбора оснований, новых правил разбиения геометрии или собственного подсчёта ведомости |

Roombook.hpp — общий контракт: существующие Otd-типы и действительно необходимые межфайловые объявления. Не создавать автоматически ещё три hpp, новый файл типов, классы-менеджеры или новый общий framework. Локальные helpers остаются в своём cpp; наружу выходят только необходимые границы этапов. Общий файл не становится местом для произвольных «общих функций».

Старые RoomSurfaces/RoomSurfaceCandidates/RoomSurfacePartition/RoomSurfaceFinish cpp/hpp поглощаются частью сбора и анализа и общим контрактом, затем удаляются после проверки callers и сборки. Matcher и адаптеры остаются понятными секциями RoombookData.cpp, но не отдельными production-файлами. Коллизии anonymous helpers разрешать минимально: одинаковый алгоритм оставить один; похожие с разными инвариантами не объединять вслепую. Линейный/плоскостной/площадной допуски не менять.

Четыре файла — четыре cpp реализации, а не обещание удалить любой вспомогательный заголовок ценой потери тестов. CoordinateSearchCache.hpp/RoomAnalyticGeometry.hpp/FloorCoreGeometry.hpp сейчас используются production и SDK-независимыми проверками; их судьба — отдельный пункт census. Не сохранять дополнительные headers автоматически, не вводить предлагаемый прежде RoomGeometry.hpp автоматически, не копировать algorithms в tests. Если чистый тестируемый kernel требует отдельного header, объяснить конкретную необходимость до переноса; это не пятый production-этап.

#### Проверенные границы и важные оговорки

- RoomBook (Roombook.cpp:1486–1558) сохраняет последовательность: сбор → параметры/правила и подготовка отделки → сводка площадей → проверка классов → Draw_Elements. SetSyncOtdWall выполняется внутри общего пути отрисовки/завершения, не в Quantities. Порядок и ранние выходы не изменены.
- Расчёт перенесён: RoombookQuantities.cpp — BuildMaterialSummaryForRooms (:24), WriteRoomMaterialData (:119), OtdData_GetColumnfFormat (:147), CollectMaterialAreasForRoom (:242), OtdData_CalcForRoom (:294), CollectAreasForFinishTypes (:342), OtdData_WriteToRoom (:377), OtdData_AddValueToDict (:495), OtdWall_GetArea (:524). MaterialSummary расположен в существующем Roombook.hpp; наружу добавлены только две необходимые границы.
- WriteRoomMaterialData подготавливает значения в summary.paramToWrite; название не означает немедленную запись в модель. Не переносить фактический commit свойств раньше создания и привязки элементов. Разделить владение расчётом значений и общий момент их применения, не вводя второй независимый writer.
- SetSyncOtdWall (Roombook.cpp:6389) обрабатывает связи созданной отделки и общий paramToWrite; он не относится к арифметике площадей. Размещение в части отрисовки/завершения записи; общий файл сохраняет момент вызова. Уточнить весь хвост функции до реализации, не менять транзакции по предположению.
- OtdWall_GetArea перенесён в Quantities в единственном экземпляре. Два существующих production-потребителя в подготовке геометрии продолжают вызывать его через прежнее объявление Roombook.hpp; новые копии и SDK-чтения не вводились.
- RoomSurfaceSource.routeReason и его диагностические consumers уже удалены по проверенному census; profile fallback проверяется по поведению. Ранее поле использовалось диагностическим consumer для PendingPreparedGeometry, profile assertion и JSON, поэтому изолированное удаление было недопустимо. По новому решению удалить диагностический consumer/export вместе с полем, если все его потребители только диагностические; если census найдёт необходимое production-состояние, сохранить только это состояние без текстовой трассировки. Проверку реального fallback сохранить поведенческим тестом. Комментарий «не читается» не считать доказательством ненужности.
- Tools/CMakeCommon.cmake:376–386 собирает cpp/hpp через GLOB_RECURSE CONFIGURE_DEPENDS: ручной source list не нужен; после переноса требуется reconfigure и проверка состава translation units.
- Ранее обнаруженный include-цикл относится к наивной попытке сохранить Finish.hpp → Roombook.hpp и добавить обратный include. Он не доказывает невозможность выбранной владельцем структуры. Общие типы объявить в существующем Roombook.hpp в порядке зависимостей, старые циклические includes убрать. Размер файла сам по себе также не отменяет выбранную структуру: сначала удаление лишнего, затем предметные секции.

#### План C1–C9

Завершённые пункты и детали их проверки перенесены в `IDEA_ARCHIVE.md` (блок 2026-10-22); ниже остаются только открытые шаги.

##### C1. Маршруты поиска ошибок, карта переноса и исходная проверенная точка

- [ ] Составить полную таблицу существующих функций/типов: текущий файл/символ → одна из четырёх частей → callers → локальная/межфайловая граница. Включить private/static helpers, caches, defaults, Favorite/Class, OtdData, legacy и TESTING-exports. Сначала generated callgraph, затем clangd/targeted source; miss графа не означает отсутствие вызовов.
- [ ] Отдельно разнести read-only получение избранного/классификаций и defaults/классификацию при создании. Не делить функции по префиксу, если побочные эффекты относятся к другому этапу.
- [ ] Установить текущий baseline и действительные FAIL. Исправление подтверждённого функционального дефекта — отдельная проверяемая дельта в существующей задаче; не маскировать его переносами.

##### C2. Упростить чтение предметных правил и убрать лишнее

- [ ] По маршрутам C1 выбрать конкретные трудные участки: вложенные условия, скрытые изменения входов, непонятные флаги/единицы, лишние копии и переходы между helpers. У каждого изменения записать, что человеку стало легче проверить. Предпочитать прямые условия и именованные промежуточные значения, не менять алгоритм/очерёдность и не начинать массовый стиль-рефакторинг.
- [ ] Отделять механический перенос от упрощения условий/данных: сначала доказать сохранение поведения переноса, затем проверять упрощение отдельной дельтой. Не маскировать исправление дефекта изменением структуры.

- [ ] По мере проверки пола/откосов/кэшей удалять временные probes, повторные данные и комментарии истории именно этого участка. Не объявлять незавершённые задачи принятыми из-за чистоты кода.
- [ ] Упростить ветвления и структуры только по доказанной ненужности; сохранить реальные body/contour/hole, происхождение основания, неоднозначность, cache key/error/lifetime. Не вводить дополнительные уровни контейнеров или универсальные абстракции.

##### C3. Диагностика

Все перечисленные удаления диагностики и counters выполнены и статически проверены; подробности и evidence — IDEA_ARCHIVE.md. Открытых шагов по C3 нет.

##### C4. Общий контракт и общий файл

- [x] Canonical compile/link общего контракта AC25 Windows Debug подтверждён 2026-10-23 после объединения текущей дельты; другие SDK/macOS not verified.

##### C5. Выделить расчёт площадей и свойств

- [ ] Сохранить правила учёта проёмов/уклона/материальных слоёв, форматы, порядок группировки и rawname/rawname_bytype. Не заменять расчёт геометрии итоговой ведомостью и не пересчитывать входы из SDK.
- [x] Отбор зон вынесен из Quantities в существующий общий сценарий: один обход с прежним interleaving отмены/отбора/расчёта, per-room OtdData_AddRoomToMaterialSummary. Сохранение расчётного блока/порядка доказано исходником и AC25 compile/link; host сравнение значений остаётся отдельным открытым пунктом ниже.
- [ ] Проверить площади и подготовленные значения свойств до/после, включая общий тип помещения, отсутствие свойств/материала, отверстия и повтор. Пока фактическая запись остаётся в прежнем месте.

##### C6. Выделить отрисовку

  Карта C6b дополнена независимым read-only census и родительской сверкой текущего source: RemoveUnusedFinishingElements вызывается только из Draw_Elements; его переносить вместе с orchestration. Прямые зависимости nPhase — Draw_Elements и SetSyncOtdWall, typeinzone — SetSyncOtdWall; общий файл продолжает использовать оба состояния. cls/favdict/reducededges перенесённому C6a напрямую не нужны. Новых extern или дубликатов typeinzone не добавлено; способ передачи прогресса/порядка типов реализован в C6b явно, без копии mutable state. Координаты ниже относятся к историческому снимку C6a. C6a final-source-manifest повторно совпал по пяти целевым файлам после получения census. Координаты subagent относятся к снимку до переноса; текущий SetSyncOtdWall — Roombook.cpp:4792. Текущий код C6a inspected review: No issues found in inspected scope; build/runtime остаются not performed.
- [ ] Проверить compile/link и адресные writer-regressions. Затем в разрешённом host-сценарии проверить реальные тела/классификацию/связи, отсутствие лишних записей исходных конструкций, удаление старых GUID и повтор без дублей.

##### C7. Собрать единый файл сбора и анализа

- [/] C7a/C7b: реализации сгруппированы в одном Data по чтению/геометрии/подготовке/данным и материалам, новых wrappers/копий/adapter-файлов нет. Шесть пользовательских маршрутов пройдены чтением исходника в C8 ниже; удобство поиска владельцем ещё не подтверждено. Одна файловая перегруппировка не завершает все C2 упрощения.
- [/] C7a: source/test consumers старых путей отсутствуют; четыре прежних cpp и четыре пустых forwarding headers удалены. Canonical AC25 compile/link после удаления headers PASS 2026-10-23. Общие testable kernels сохранены и исполнены standalone; остаток C7b callers/state и пользовательская читаемость не считаются полностью принятыми.

##### C8. Проверка поведения и читаемости

- [ ] После каждого проверяемого переноса/изменения: clang-format затронутых cpp/hpp/h с сохранением SDK include-order, scoped diff/diff-check, fresh clangd baseline/final; canonical python Tools/BuildAddOn.py -c config.json -v 25 при разрешённом состоянии VS. Новая ошибка или регрессия — остановка и отмена только собственной дельты, не пользовательского файла целиком.
- [ ] Исполнить имеющиеся host/standalone regressions по проверенным командам. Если перенос затрагивает SDK-независимый executable, подтвердить сохранение общей production-реализации и его исполняемости; не считать компиляцию выполнением тестов.
- [ ] С разрешения на безопасной фикстуре выполнить host/selftests и RoomBook + повтор, прочитать актуальную VS «Отладка». Не останавливать чужую debug-сессию и не сохранять PLN.
- [ ] Сравнить native геометрию/покрытие/проёмы, материалы, настоящие base/zone links и значения итоговых свойств, а не только количество элементов. Покрыть стены/профили/балки/откосы/полы/потолки/перепады/ядро в реально доступных сценариях. Полноту reader подтвердить; непроверенное оставить not verified.
- [ ] Прочитать конечные четыре файла как пользователь: из общего виден сценарий; Data отвечает за подготовку; Quantities — за свойства; Draw — за создание. Проверить понятность русских предметных секций и неочевидных единиц/координат/флагов. Нет временного шума, лишних переходов, исторических названий или обратного дробления на технологические файлы. Понятность для владельца подтверждает сам владелец; собственное ревью агента не выдавать за пользовательскую приёмку.

###### C8 — актуальные маршруты поиска ошибок

Снимок этой comment-only дельты; пути относительно Sources/AddOn/roombook. D = RoombookData.cpp, Q = RoombookQuantities.cpp, W = RoombookDraw.cpp. Старую карту C1 выше читать только как исходное состояние.

| Симптом | Где проследить данные и отсев | Точка останова / значения | Что стало проще и что осталось |
|---|---|---|---|
| Нет отделки | Roombook.cpp:143–146 → D:4291 ProcessRoomFinishes: параметры комнаты → isValid → BuildRoomSurfaceFinishes → правила → W:238 Draw_Elements_OneRoom, отдельные create_* guards | D:4322–4333: zoneGuid, roomParamsWork, otd.isValid, openingWalls; W:250–264: type/create_*/draw_error | Координатор отделён от подготовки и writer; пропуск комнаты, отдельного участка и отказ writer всё ещё различаются внутренними ветвями. |
| Высота/глубина откоса | D:3886 Param_SetToWindows → D:6289: сначала base_th, затем sill/frame/сторона; D:6538/6554: высотное пересечение и координаты → D:6582: guards глубины/высоты → высотные полосы | D:6329/6602/6607: base_th, frame, sill, base_reveal_width, create_reveal, zDown, height | Связанные правила находятся в Data, расширение пола до reveal-guard явно отмечено. Итоговая геометрия writer остаётся отдельной точкой проверки. |
| Материал/избранное | D:6960 SelectWallFinishMaterial меняет material и type → D:7069–7072 учитывает base_composite → D:3824–3828 Favorite_FindName и добавление состава Favorite → Q:242 площади | D:6960/3824/3827: type, material, om_*, favorite, base_composite | Подготовка и создание разнесены по назначению. type, draw_type и favorite.type по-прежнему разные значения; материал может уточняться после первого выбора. |
| Контур пола/потолка | D:2836 BuildProjectedSlabFinishes: roles/sources/nominal → candidates/remainder/accepted; пустые/малые/неоднозначные пересечения отбрасываются. Позднее D:3888 расширяет пол в проёмы, D:4356 — до ядра; окончательный массив идёт в W:303 Floor_Draw | D:2850/2887/2909: footprint, candidates, processed, accepted, remainder; D:4356: otd.otdslab | Вместо переходов между старым общим и Finish cpp — Data; поздние изменения контура теперь видны в последовательности ProcessRoomFinishes. Первичный builder не является окончательной точкой готовности. |
| Площадь/свойства | Q:24/119 → Q:242/524: готовые Otd-данные, отверстия/уклон/составы → Q:495 порог/IGNORENAME → Q:485 сравнение old/new → W:338 SetSyncOtdWall, W:390 фактическая ElementsWrite | Q:249/272/485: area, base_composite, old_val, new_val; W:390: paramToWrite | Арифметика/форматы собраны в Q; комментарии больше не называют подготовку записью в модель. Общая очередь связей и ведомости остаётся в Draw и может не дойти до записи при отмене/ошибке. |
| Дубли при повторе | D:4396/4402 Otd_GetOtd_ByZone: zone/base/type индекс → W:123 AssignExistingWallFinishes → W:84 TakeExistingWallFinish: совместимость/одноразовое использование → W:425 список остатка → W:439 удаление → W:464 связи | W:199/425: wall.base_guid, classGuid, candidates, used, otd_guid, deletelist; W:372: subguids/suffix | Подбор стен, удаление и публикация связей рядом в Draw; комментарии отличают список от удаления. Legacy- и surface-перекрытия идут разными путями, общей приёмки повтора это не доказывает. |

Результаты двух независимых read-only census использованы как подсказки; родитель сверил указанные опорные ветви с текущим исходником и подтвердил неизменность source hashes. Это не полный bug-review, не проверка внутренних Sync/ElementsWrite и не успешный host-запуск. Review comment-only дельты: No issues found in inspected scope. Memory: правило проверки reused containers и границы prepare/create/persist сохранено в cpp-source-comment-maintenance; новую диагностику не вводить.

##### C9. Завершение

- [ ] По отдельному разрешению обновить Docs/modules/Roombook.md и scoped generated; архивировать только завершённое/проверенное, сохранить остальные активные задачи и ограничения.
- [ ] Новую issue не создавать. Коммит/чекпоинт/push только по отдельной команде, только intended files.

###### C9 — назначение оставшихся headers

Исторический census до дельты 2026-10-23; координаты и количества ниже не текущие. Актуальное решение: четыре forwarding headers удалены после повторного zero-consumer census и canonical AC25 compile/link PASS; три чистых kernel сохранены, RoomAnalytic46/0, FloorCore41/0 и CoordinateSearchCache PASS исполнены standalone. Roombook.hpp остаётся общим контрактом. Внешние проекты/скрипты не проверялись. Пути относительно Sources/AddOn/roombook.

| Header | Назначение и реальные потребители | Решение в текущем scope |
|---|---|---|
| Roombook.hpp | Общие снимки/кэши/Otd-типы, MaterialSummary, RoomProcessingContext и границы этапов. Подключён четырьмя cpp, SomeStuff_Main.cpp:18, json_commands/RoomBookCommand.cpp:5, тестами и четырьмя временными forwarding headers; всего16 прямых include-consumers. | Остаётся общим контрактом. В1113 строках также есть прежние helper declarations; нужность каждого межфайлового экспорта этим census не установлена — отдельная C4/C1 дельта, не повод объявлять header уже минимальным. |
| CoordinateSearchCache.hpp | SDK-независимый шаблон ExactCoordinateSearchCache и полный ключ запроса. Roombook.hpp:13/22 задаёт SDK-result alias, context владеет кэшем (:1070); Data:3150–3156 использует тот же SearchOrRead. tests/CoordinateSearchCacheRegression.cpp:7/25 проверяет этот шаблон, не копию. | Сохранить общую реализацию. Перенос внутрь Data.cpp лишит независимый regression доступа к production-шаблону; включение через общий SDK-контракт уничтожит независимость от SDK. Новый cache-manager не нужен. |
| RoomAnalyticGeometry.hpp | WallSides/OrdinaryWall/SubtractOpenings/CutWallBottom/SlabRole/SlabLevels; только стандартные includes. Production: Data:1339,3250–3392,5171; tests/RoomAnalyticChecks.hpp:4 использует те же функции; checks вызываются из TestRoombookGeometry.cpp:115 и отдельного RoomAnalyticRegression.cpp:9. | Сохранить одно testable kernel для аналитической подготовки и нижних отрезков. Не копировать алгоритмы в tests и не добавлять RoomGeometry.hpp. Новое исполнение обоих runners не выполнялось. |
| FloorCoreGeometry.hpp | Offset/Extensions: глубина до ядра и конечные геометрические полосы/стыки, без SDK includes/calls. Production: Data:6148 и1419–1488; tests/FloorCoreChecks.hpp:4 вызывает те же Offset/Extensions, host-путь подключён в TestRoombookGeometry.cpp:1803. Tolerance также используется Data:1307/1368. | Сохранить отдельное общее kernel. Подтверждён существующий host-test consumer, не новый standalone-прогон; отдельный FloorCoreRegression.cpp в проверенном дереве отсутствует и как runner не заявляется. |
| RoomSurfaces.hpp | Только pragma/guard ROOMBOOK_SURFACE_HPP и include Roombook.hpp. Прямых consumers пути0; guard упоминается только внутри этого файла. | Временный кандидат на удаление после разрешённой сборочной проверки; новых объявлений/reader-модуля здесь нет. |
| RoomSurfaceCandidates.hpp | Только pragma/guard ROOMBOOK_SURFACE_CANDIDATES_HPP и include Roombook.hpp. Прямых consumers пути0; guard только внутри файла. | То же условное удаление; не сохранять как самостоятельную production-границу. |
| RoomSurfacePartition.hpp | Только pragma/guard ROOMBOOK_SURFACE_PART_HPP и include Roombook.hpp. Прямых consumers пути0; guard только внутри файла. | То же условное удаление; типы partition уже в общем контракте, cpp — в Data. |
| RoomSurfaceFinish.hpp | Только pragma once и include Roombook.hpp. Прямых consumers пути0. | То же условное удаление, без нового replacement-header. |

Review: No issues found in inspected scope — это проверка структуры/includes, не всего алгоритма. Memory: метод census residual headers и проверки точных include-координат сохранён в idea-md-maintenance. Удаление временных headers и canonical AC25 build завершены 2026-10-23; общая C9 остаётся незавершённой на Docs/generated и полной runtime-приёмке.

#### Приёмка

Главный критерий: по каждому симптому из C1 человек с небольшим опытом C++ может найти ответственное место и понять, какие данные и условия привели к результату. В приёмке показать конкретный путь по коду и что стало проще относительно исходного состояния. Четыре cpp, уменьшение числа строк и зелёная сборка сами по себе не доказывают достижение этой цели. Сохранение поведения и понятность проверяются отдельно; до обратной связи владельца пользовательская понятность — not verified.

Основная реализация разделена по четырём пользовательским обязанностям, а не по внутренним стадиям геометрического алгоритма. Общий файл показывает порядок работы; сбор/анализ не пишет модель; расчёт площадей готовит правильные значения; отрисовка сохраняет существующий контракт создания/удаления/связей и момент применения свойств. В рабочих алгоритмах и их TESTING-ветках нет диагностических сообщений, dumps/exports, counters и диагностических полей/обвязки; исключение «оставить для будущей отладки» запрещено. Разбор причин выполняется отладчиком. Функциональные проверки ошибок, необходимые пользовательские сообщения и самостоятельные автоматические тесты сохранены. Тестируемая логика не продублирована в тестах. Ранее проверенные поведение, правила и repeat сохраняются в реально выполненном объёме. SDK/macOS и неисполненные сценарии — not verified.

## Archive

### Surface finish — причина сообщений для E39BE854 / зоны115, 2026-10-30

Status: COMPLETED (диагностика, не исправление). Scope: AC25 Windows/test_25/PID40324, разрешённые владельцем временные точки и один in-memory RoomBook; без C++/Save/build/restart/commit/push.
Plan: [x] код/JSON конкретных GUID; [x] обе реальные beam-error ветки и источник summary; [x] завершить RoomBook, убрать свои точки, проверить Data/APX/оба saved PLN.
Verified: E39BE854-1F13-4BF8-8450-D396C1BE4823 и 3DEA1CF9-C1FB-43CA-BB2C-7F8478C802E6 в зоне A2100887-E07B-4DF1-9834-50A4D23FF992 имеют tBeg=tEnd=6.907071282619067; result.errors[0].stage=beamEmptyPart, code=-2130313112, finish.pieces0. Data1336–1341 отвергает нулевой интервал до обхода граней, caller5541 обобщает ошибку как Unsupported beam geometry. Сложный профиль КЖ_Канал не является установленной причиной этого отказа; отсутствие конечного face-contact по нулевому осевому интервалу не доказано.
Verified summary: Data5462 действительно меняет hasSkippedPieces false→true на Unmatched/baseGuid0/area0.040000005722057252; grouped22=9Matched+5Unmatched+8Ambiguous. Из13non-Matched:4вертикальных+1нижний Unmatched;4вертикальных Ambiguous с разными analytic Wall owners;2нижних Ambiguous с одновременными claims обеих указанных Beam;2микроучастка сверху с Wall/Slab claims. Header/types/stride/offsets проверены перед paused-memory capture. На выходе5605 подготовлены8walls/2slabs; alternate analytic/nominal маршруты означают, что эти13не равны13пропускам готовой отделки. Полнота/native-геометрия результата не проверялась.
Validation: один RoomBook succeeded=true/status=returned/rooms63, exit0; native inventory895→4512 (added3621/removed4), все3целевыхGUID сохранены. Команда изменила модель в памяти; Data/APX00f8928b…/оба saved PLN hashes unchanged. Свои точки удалены, исходные2disabled по функциям сохранены, VS Run. Свежий output_read(Отладка) отказал SSE stream ended without a response, не повторялся; доказательства — реальные VS locals/evaluate и memory captures. Compiled: не выполнялась; Tested: реальный диагностический прогон/причины отказов наблюдены, исправлений нет.
Next Step: отдельная задача исправления — различить пустую beamPart и неподдерживаемую геометрию; проверить конечный контакт перед сменой zero-interval policy. Summary отделить от штатного fallback, неоднозначные основания не подавлять и элементы модели не удалять по одному совпадению claims. Last Checkpoint: нового нет, commit не запрошен. Memory: visualstudio-cpp-debugger/references/archicad-somestuff.md обновлён. Evidence: Hermes scratch/surface-beam-diagnosis/runtime/report.md и {observations,zone115-beam-parts,zone115-grouped-provenance,before-manifest,after-manifest,response}.json.

### #291 — восстановлен стеновой default, AC25 Windows

Status: COMPLETED в границах исправления; GitHub issue остаётся открытой до решения владельца. Scope: одна строка OtdWall.draw в Roombook.hpp:404; общий Slab-default, Favorite, writers, геометрия и плинтуса не менялись.
Plan: [x] dirty backup; [x] real-header SDK25 RED18/2 → GREEN18/0; [x] one-line locality/format; [x] fresh LSP; [x] canonical build; [x] native RoomBook+повтор; [x] итоговая source/PLN/BP проверка.
Verified: реальный C++14/SDK25 header; сохранены остальные defaults/копирование/Beam override/Object Favorite. Fresh CLI Data/Draw оставляет8 прежних unused-variable, новых initializer diagnostics нет; exit3, полностью GREEN LSP не заявляется. Own review: No issues found in inspected scope; независимое ревью не заявляется.
Compiled: BuildAddOn.py -c config.json -v25 Debug exit0, реальная компиляция и link. APX3d0ec4ad6f4fba26e9810a1a31a50311aa468f4d1ee51bd7597d5093e19206c5; header3ae2808b8ccb711521a832161cf8e08d8b7efbdbda3eaf6eaa11a0099c832602.
Tested: PID48984/test_25, штатные два RoomBook returned. Первый создаёт17 новых Wall (7стен+10откосов), оба прогона0 OtdWall_Draw ошибок; повтор использует эти17GUID. Total136→136, added33/removed33 на повторе; все35 сохранившихся first-created (включая17Wall) проверены по типам/классам/связям. Полный нормализованный bbox/property/link inventory совпал, coincident-bbox candidates0, bad links0. Это не независимая solid/contact-приёмка всего #252.
Boundary:61 OtdBeam_Draw ошибок на каждом прогоне и прежние surface skips не исправлялись. Production-diff — ровно1строка;107 остальных tracked C++/headers неизменны. Точная owner build-copy восстановлена после build, оба saved PLN hashes неизменны через F5/два RoomBook. Свои точки не добавлялись, исходные995/1644 enabled восстановлены и прочитаны обратно; VS Run, модель оставлена открытой.
Next Step: по отдельной команде полная геометрическая приёмка #252 и диагностика плинтусов. Last Checkpoint: нового нет, HEAD6f131e1; commit/push/закрытие не выполнялись. Docs/generated вне однострочной дельты; сигнатуры/положение строк не менялись, общую документацию рефакторинга не переписывали.
Evidence: Hermes scratch/draw-defaults-291/{before-manifest.json,before/,own.diff,harness/*-result.json,*-lsp.log,build.log,build-result.json,final-proof.json,native/}. Issue291/comment6402880628 и обновление252/comment6402405282. Memory: archicad-plugin-build/references/field-move-refactor.md дополнен проверкой owner-specific defaults.

### #270: Beam-only подбор плинтуса — 2026-10-25

Scope: только отсутствие zone-material inheritance, dedicated Beam selection и строгий native writer; AC25 Windows. Legacy распознавание Wall/Object сохранено для cleanup; геометрическое ядро, остальные writers, ресурсы и Test_file не менялись. Status: COMPLETED этой дельты; полная геометрическая приёмка #270 открыта.
Plan: [x] реальный SDK/C++14 RED175pass/43fail → GREEN218pass/0fail; [x] format/diff/locality/registry90 и source-review; [x] canonical BuildAddOn.py -c config.json -v25 Windows Debug compile/link0; [x] native selector/writer и независимый read-back; [x] замена прежних GUID и ограниченный повтор.
Verified: VS Favorite_FindName:8241/8242 показал smstf plinth down/up, API_BeamID и пустой material; SkirtingBeam_Draw:40 — createError0, native type Beam, nSegments1. Независимый JSON:23 lower+21 upper, все44 новые Beam; предыдущие44 (19Beam+25Object) удалены. Оба native GUID-свойства88/88 normal, base существует, zone имеет native Zone type. На повторе все44 GUID первого запуска удалены/заменены; ссылки и bbox устойчивы в1e-6, ненулевая высота у всех44. Это не независимая проверка правильности каждого родителя/контакта/наклона/профиля.
Boundary: общий инвентарь232→232→236; при повторе Wall71→75 вне классов плинтуса. Все74 элемента, созданные первым запуском, удалены на втором; накопления прежних плинтусов нет. Причина дополнительных четырёх обычных стен не установлена и не исправлялась; полная повторяемость RoomBook не заявляется. Панель «Отладка» вернула SSE stream ended; полный startup TestKit этим шагом не объявляется GREEN. SDK218/0 и native read-back — отдельные доказательства.
Compiled: final canonical build0; загружен Build/SomeStuff/25/Debug/SomeStuff.apx SHA2566bc19e485dcb15464b273cbf38d47eed0b097f09fa2a5f8d024a4a868fd3beef. До/после F5 и двух команд исходники/APX/оба сохранённых PLN совпали с manifest. Owner build-copy c131c84e4731cb39c97146306b7aed7647b3a0d209900b312f2eeec031679189 восстановлена после штатного копирования canonical build; исходный Test_file не менялся. Data вне SkirtingSegments_Build/Favorite_FindName и Draw вне SkirtingBeam_Draw побайтно совпадают с before; header неизменён. Fresh CLI Data3/Draw4 прежних unused-variable, exit3, не fully-green LSP.
Evidence: Hermes scratch/skirting270-beam-only/{red-result.json,green-result.json,build-final.log,final-build-manifest.json,post-launch-hash-check.json,before-current-native.json,after-current-native.json,repeat-current-native.json,native-validation.json,final-data-lsp.log,final-draw-lsp.log}. Source recovery отдельной не принятой F5 попытки — stale-launch-source-recovery.json; её результаты не смешаны с итоговым запуском. Собственный source-review: No issues found in inspected scope; независимое review не заявляется.
Next Step: ручная геометрическая приёмка #270; отдельно расследовать четыре дополнительные обычные Wall на повторе, если владелец поручит. Last Checkpoint: нового нет, HEAD6f131e1; Docs/generated/commit/push/issue close не выполнялись. Memory: saved — lightrag skirting reference, VS source/binary identity, build/TDD dialect rules.

### Предрелизный аудит — 2026-10-23

Scope: отчёт о готовности текущего рабочего дерева к выпуску AC25 Windows; исходники и модель read-only, без исправлений, commit/push и закрытия issues. Остальные версии/macOS не проверяются. Существующие задачи ниже не возобновляются.
Status: COMPLETED — аудит завершён, выпуск не одобрен.
Plan: [x] снять git/source baseline; [x] canonical AC25 --release и Debug build, HTML/npm/i18n; [x] startup TestKit и Health в test_25 через VS; [x] сверить read-only ревью и сохранить приоритетный отчёт.
Evidence: Hermes scratch/release-audit-20261023; HEAD 6f131e173de117bf3987552b61f5a6b08fd1fa57, исходно 14 изменённых файлов. Свежий TestKit:88 suites/39007 passed/168 failed, единственный failed suite TestOpeningRevealsCreateOne3586/168; SUMMARY сверена с END. Health status=ok/projectName=test_25. Loaded Debug APX0c808369b577f3a29e3f8426c9d4e52ccdbe10227e22cd9e2afe8ac1c44aef4b; release APX38f66f041248a50e1cce7df756780c464b79da3b1ef6931ad66d603fb7f416a3. Release runtime не проверен. HTML/npm PASS, i18n46/0,159 keys без рассогласований; ESLint8 warnings/0 errors. Исходники и оба PLN по hashes неизменны, собственный debuggee остановлен; Design и восстановленная breakpoint995 enabled подтверждены. Старый отчёт по вручную составленному long-user-path не использован, свежий взят из фактического TEMP.
Report: Hermes scratch/release-audit-20261023/RELEASE_REVIEW_RU.md. JS-заглушки Sync/Numbering воспроизведены в Node на точных телах; CEF clicks не проверены. Статически сверены вложенный Sync_flag и условный отказ Mesh-discovery; runtime этих двух сценариев not verified.
Next Step: отдельными согласованными задачами убрать ложные UI-результаты, принять направление откосов, исправить подписи Spec и проверить parser; затем smoke именно распространяемого APX.
Last Checkpoint: не создавался; коммиты не запрошены.


### Кэш правил — исправления после ревью, 2026-10-23

Scope: AC25 Windows; Propertycache.cpp/.hpp и адресные тесты TestParam.cpp/TestSync.cpp. Пользовательские HashTable и перенос парсера сохранены. По явной команде владельца — без issue, commit/push отсутствуют.
Status: COMPLETED в offline/source/build scope; host not verified.
Plan: [x] текущий снимок; [x] RED и минимальная правка; [x] GREEN, clang-format, LSP, canonical AC25 build и review своей дельты.
Last Completed: старые memberships удаляются при смене описания; ReadPropertyDefinition очищает оба кэша; Put обновляет definition даже без смены description с повторным использованием parsed. Sync_flag распознаётся с необязательной открывающей скобкой, не заимствует чужую закрывающую; GUID-prefix comparison нормализует обе стороны. Новый распознанный маркер остаётся otherCommands, а не executable syncRules; прежнее ожидание теста обновлено под этот контракт.
Validation: SDK25/MSVC executable26 checks: RED16fail → GREEN0fail. Реальные извлечённые GetPropertyRuleFlag/оба parser/helper/StringSplt и два repo-test helpers; GUID metadata branch/reset prefix проверены отдельно, SyncString abort-sentinel не вызывался. Это не SDK enumeration/host/full GUID validation. Canonical BuildAddOn.py -c config.json -v25 Debug exit0, реальные TestSync/Propertycache/TestParam compilation и APX link. clang-format dry-run/diff-check PASS. MCP Propertycache0, TestSync0errors; TestParam7 прежних ambiguous-assignment на неизменных первых1100 строках, известны в DISCREPANCIES. Посторонние9 snapshot files и BOM своих исходников неизменны. APX e26ed08918460ed15581a8e0a446e391d416bee46edd36d2f79f4bb49cf6a0e6.
Next Step: ручная приёмка в палитре/Archicad при необходимости; Docs/generated по действующему порядку — перед отдельной командой на коммит, сейчас не обновлялись. Чужие активные задачи не закрывать.
Last Checkpoint: 6f131e1 — существующий; нового нет.
Evidence: Hermes scratch/cache-fix/{before.json,red/,green/,build25.log,validation.json,*.own.diff}. Memory: archicad-cpp-code-review/references/property-rule-reverse-index.md.

Завершённые задачи и снятые материалы перенесены в `IDEA_ARCHIVE.md`:
- 2026-10-22: закрытые по команде владельца #287, #288, #272, #280, #286, #195; закрытие не означает полной приёмки (см. оговорки в Archive).
- #288 — обновление существующих стен отделки и откосов на месте (CLOSED); полная приёмка остаётся not verified.
- #268 — прямоугольные пропуски в углах полов (CLOSED); микросегмент с нулевым shift исправлен, canonical build/native-приёмка не выполнялись.
- #272 — ускорение 3D RoomBook и API-кэши (CLOSED); кэши реализованы, контролируемый A/B и полная приёмка not verified.
- #287 — очистка API_Attribute перед чтением материала (CLOSED); native-проверка исходного сценария не выполнялась.
- Детальные журналы дельт C2/C3/C4/C6/C7 (2026-10-21/22) и исторические записи #272/#288.
- #278 — отписка наблюдателей (DetachObserver в Helpers/ReservationChangeHandler/APINotify)
- #277 — вылеты Undo.dll при переключении окон и в ElementEventHandlerProc
- #276 — неверная высота потолка-объекта (level от уровня этажа, ac_ref_height=0)
- #275 — совместимость SDK AC26–30
- #274 — очистка коллинеарных вершин полигонов полов и потолков
- #273 — очистка старых preferences проекта
- #269 — сигнатура AdvanceProcessPhase в CommonFunction
- #267 — поиск черновых полов и потолков по низу и верху стен
- #192 — регрессия Object-избранного пола/потолка
- Локальная приёмка углов стен (API_WallPart), срез документации Roombook 2026-10-15, расследования падений Undo.dll, этапы Z7–Z9.


## Палитра: сохранение пользовательского скрытия (#324, 2026-11-12)

Scope: AC25 Windows, BrowserPalette.cpp/.hpp и ветка Pallete_CommandID в SomeStuff_Main.cpp; только видимость, без защитной правки чтения модели #323.
Result: SetUserVisible сохраняет явные меню/крестик/Open/Close, отменяет временное восстановление; Begin/End восстанавливают только временно скрытую палитру без reload. MenuCommandHandler перечитывает настройки перед общей записью и обновлением галочки.
Validation: controlled exact palette RED22/9 → GREEN22/0; exact внешний handler RED7/2 → GREEN7/0. Полные TU SDK25/MSVC/C++14 /WX скомпилированы в scratch, fresh LSP без новых сообщений относительно baseline; независимое повторное ревью без замечаний в scope. Native-проверку подтвердил владелец: «коммить и закрывай issue - я проверил». Отдельный final runner агента не выполнялся; другие SDK/macOS не проверены. Commit разрешён, push не запрошен.
Issue: https://github.com/kuvbur/AddOn_SomeStuff/issues/324.
Evidence: Hermes scratch/palette-visibility/, commit/ — адресный snapshot без #323.
