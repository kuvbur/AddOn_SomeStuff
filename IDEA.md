# Current Task Roombook: отделка по поверхностям (#252)

## Task

Продолжение переноса на поверхности 3D-зоны, SDK AC25 Windows. Reader, partition/beamPart и подключение production-создания/расчёта выполнены в проверенном объёме. Issue: https://github.com/kuvbur/AddOn_SomeStuff/issues/252. План: .hermes/plans/2026-10-11_115207-roombook-zone-surfaces.md.

## Scope

Создание и расчёт отделки из проверенных участков: стены, колонны/сегменты, перекрытия и балки (низ и бока), штатные writers и материальные правила. Один участок — одно основание; неоднозначные/непредставимые участки не создавать. Представимые негоризонтальные грани — штатные наклонные стены на отдельном слое с группой пересечения 0. Исходные конструкции и сохранённые PLN не менять. AC25 Windows; остальные SDK отдельно, AC_25 gates не возвращать. Docs/generated и коммит только по отдельной команде.

## Status

WAITING_FOR_DECISION для остатка #252: четыре трапеции требуют решения владельца; issue остаётся OPEN. Проверенный reader/partition/writer результат, русские комментарии и scoped Docs/generated включаются в текущий checkpoint Z7. ReNum.cpp — посторонняя незакоммиченная правка, исключена. PLN не сохранялся; push не запрошен.

## Last Completed

2026-10-11: пункт 3 исправлен и проверен. GroupSurfaceWallPieces объединяет представимые копланарные стеновые контакты только с одинаковым полным source-face provenance (base/type/geometry/body/polygon/material), zoneBody и направлением нормали, затем применяется прежний BuildSurfaceWalls. AC25 Windows Debug canonical BuildAddOn.py compile/link success; свежий direct LSP без новых диагностик; self-tests 60/60, восемь новых regressions и ERROR IN TEST=0 подтверждены в актуальной Отладка. Реальные deterministic/deterministic-repeat:63→63 (47 стен+16 перекрытий),126 links,63 layer-properties/group0,15 исходных GUID сохранены. Все ориентированные native-контуры отдельных тел совпадают в допуске1e-6 м (max drift6.5868126e-7 м), bodywiseEqualRounded5=true; независимое покрытие прежних трёх стен и шести контактных плоскостей двух балок PASS в обоих запусках. Сохранённые PLN hashes unchanged. Артефакт scratch/z7-writer/deterministic-final-summary.json; APX SHA2564f4fb65acf699f79db6669d63931fd6a9f7f6cf965074fe1c122e3ed3b9f95c4. Пункт 2 классифицирован, но четыре трапеции остаются непредставимыми прежним writer; полное создание всех контактов не заявляется. Commit/push нет.

Предыдущий проверенный этап:

2026-10-11: исправлены три крупные пропущенные стеновые грани (536DEA6F, FA9703B0, 2F066511): точное разложение контуров с вырезами/отверстиями через BuildSurfaceWalls, прежние writers и материальные правила. Восстановлено 98.701516 м²; обе направленные разности проверены по исходным контактам в двух реальных запусках AC25. Canonical BuildAddOn.py Debug compile/link success; fresh direct LSP: адаптер/тесты 0, Roombook прежние 13 диагностик. Self-test 50/50 Passed, новые пять ok подтверждены в Отладка. Создано 61 элемент (45 стен, 16 перекрытий); повтор удалил 61 и создал 61, исходные 15 GUID сохранены. 122 связи и 61 элемент на слое с intersectionGroupNr=0 прочитаны. Контакт/сторона толщины проверены; max plane gap 4.81e-7 м. Покрытие сохраняется, но разбиение трёх стен на отдельные тела между запусками изменилось — идентичность каждого тела НЕ подтверждена. Оба сохранённых PLN hashes unchanged. Артефакты scratch/z7-writer/coverage{,-repeat}-wall-coverage.json и coverage-final-summary.json; APX SHA-256 0574aab3c19ee8eaae36c7a30847fe873970605d5200fda8cb61358ddd19b4be. Archicad оставлен в VS Run с результатом; commit/push нет.

## Next Step

Пункт 3 завершён и проверен: bodywise=true, независимое покрытие при повторе. Пункт 2 частичен: 28 in-scope пропусков классифицированы, четыре реальных трапециевидных контакта колонн (2.375999498 м²) требуют отдельного решения о расширении штатного wall writer. Синхронные снимки опровергли owner60/71: это вырезы актуальных стеновых граней. Неизвестным основаниям GUID не назначать, допуски не ослаблять; три торца вне scope. Следующее действие — решение владельца о четырёх трапециях, не автоматическое расширение writer. Docs/generated и final AC25 build/load выполнены; полный TestKit, Undo/Redo и другие SDK not verified.

## Last Checkpoint

Текущий checkpoint: [Z7] Отделка по поверхностям зоны и устойчивое разбиение; ветка roombook_refactor, parent c323a2f. SHA текущего checkpoint определяется git log -1 --format=%h после commit. Push не выполняется; #252 остаётся открыта.

## Plan

- [x] Reader, снимок OtdRoom, read-only matcher/partition и beamPart проверены на AC25; завершённые reader-факты в Archive.
- [x] Production OtdWall/OtdSlab подключены к существующим правилам материалов, избранного и writers; площади учитывают уклон, высотные полосы остаются на плоскости, проёмы повторно не вычитаются.
- [x] clang-format, fresh direct LSP, canonical AC25 build; реальный RoomBook, независимое чтение тел/свойств/слоя и повтор без сохранения PLN выполнены.
- [x] Исправить три крупные пропущенные стеновые грани: разложение вырезов/отверстий, canonical AC25 build, self-tests 50/50, оба запуска и независимое покрытие относительно исходных контактов проверены.
- [x] Исправить отделку балки у профильной стены: грани пересекаются с фактическим сечением 3D-зоны вместо нижнего beamPart-контура. AC25 canonical build success; self-tests 52/52, новые оба ok в Отладка; runtime/повтор 63→63, все шесть контактных плоскостей двух балок прошли независимые missing/excess проверки, прежние три стеновые грани также PASS. Подробности в Archive.
- [ ] Три торцевых участка многослойных стен: определить контактные слои и назначение материала без произвольного base_flipped; полный охват стен пока не достигнут.
- [x] Пункт 3: same-source coplanar union перед прежним sweep; AC25 canonical build, self-tests60/60 (8 новых), native runtime63→63 и независимое bodywise-сравнение всех ориентированных контуров при1e-6 м PASS, покрытие стен/балок PASS. Подробности в Archive.
- [/] Пункт 2: все28 in-scope пропусков классифицированы; синхронные source-снимки отвергли ложный owner60/71. Четыре трапеции2.375999498м² требуют согласованного расширения прежнего wall writer; две микрополосы ниже неизменного порога. Неизвестным основаниям GUID не назначать. Три торца вне scope; полное закрытие пункта2 не заявляется.
- [ ] Остатки SDK-error/deleted/multiple-body и остальных неподдержанных геометрий/избранного — отдельным этапом.
- [x] По команде владельца добавлены русские комментарии, доказан comment-only характер правок, актуализированы и независимо проверены scoped Docs/generated, выполнены final AC25 build/load и свежие live self-tests60/60. Результат — текущий checkpoint Z7, подробности в Archive; без ReNum.cpp, push и закрытия #252.

## Decisions

- Native AC25 Inside/flipped=false строит тело слева от линии. Адаптер ориентирует левый бок внутрь зоны; slantAlpha=atan2(1,-dot(riseXY,rightXY)). Обе семантики подтверждены полным чтением созданных тел, а не численным тестом угла.
- Точечный допуск 1e-6 м не менялся; проверка площади представимого параллелограмма учитывает погрешность точек вдоль периметра. Исправлен нестабильный отказ одной грани при повторе 51→52; финальные запуски оба 52.
- Избранное и правила назначения материала сохранены; толщина slab-отделки может быть толщиной избранного (в фикстуре 0.131 м), не обязательно 1 мм. Нового writer/Morph/GDL нет.
- Переиспользуемые геометрические уроки: skill lightrag, references/somestuff-surface-finish-ac25.md. Scoped Docs/generated обновлены перед текущим checkpoint по явной команде владельца.

## Archive

### Z7 #252 — документация, комментарии и checkpoint AC25 Windows

Проверенный объём текущего checkpoint: production reader/partition/beamFinish, surface-based подготовка штатной отделки и canonical same-source grouping; #252 не закрывается. Русские комментарии добавлены в пяти файлах roombook; code/ordered-literals/balance равны сохранённому pre-comment снимку. clang-format и fresh direct LSP пройдены, новых диагностик нет.

Final restart_archicad_for_test.ps1: build/load SUCCESS после комментариев. Первый запуск остановился до сборки на информационном npm notice; повтор с process-local npm_config_update_notifier=false/npm_config_loglevel=error прошёл реальные HTMLHint/custom verify/i18n и canonical AC25 build/load. Свежий live C++ selftest60/60, failed0; оба saved PLN hashes unchanged. APX SHA256468b5d48e8d072342fb7437f62182073b509b87fe3438a280580003eeb92ed30. Optional JSON runner отсутствует и пропущен; старые TestKit85/39418 не приняты как свежая проверка. Native63→63/bodywise/max drift6.5868125745221e-7м — предыдущий behavior-equivalent запуск, не новый writer после комментариев.

Docs/modules/Roombook.md и json_commands.md, REPOMAP, _progress и generated обновлены:1470symbols/695edges. Родитель независимо сверил15source hashes/6doc hashes,651live ranges,579symbol joins и равенство JSON проверенным candidates;891посторонних symbols/414edges/order сохранены. Четыре inherited external stale endpoints/один site помечены; operator!= и cross-TU/SDK полнота не заявляются. При review исправлены устаревшие TESTING+AC25 подписи регистратора и координата заглушки в карточке json_commands. Артефакты scratch/z7-docs-scoped/final и z7-comment-checkpoint.

Checkpoint объединяет только связанные #252 исходники, тесты, Docs и IDEA; ReNum.cpp исключён. Без push и закрытия issue; сохранение PLN не выполнялось. Остаток: четыре трапеции, неизвестные основания и прочие явно неподдержанные случаи; решения за владельцем.


### Пункт 3 #252 — детерминированное разбиение отделки (AC25 Windows)

2026-10-11: старые beam-gap/beam-gap-repeat — offline RED по ориентированным native-контурам; у base536DEA6F три тела меняли вертикальный шов на горизонтальные полосы при практически одинаковом покрытии. Реальная историческая причина изменения native-входов не установлена: точные production snapshots не сохранены. Чувствительность per-piece BuildSurfaceWalls воспроизведена новым C++ regression. GroupSurfaceWallPieces выполняет union до прежнего level/interval sweep только для представимых стен с одинаковым полным набором source-face provenance, zoneBody и направлением нормали; обе polygon-разности проверяются, произвольные structural components не объединяются. При отказе выход grouping не меняется.

Canonical BuildAddOn.py25 Windows Debug compile/link success; fresh direct LSP без новых диагностик; self-tests60/60, восемь новых regressions, актуальная Отладка ERROR IN TEST=0. Runtime deterministic/deterministic-repeat:63/63=47стен+16перекрытий, удалены ровно63 предыдущих GUID, созданы63 новых. Независимый ориентированный native-contour multiset совпадает с округлением5знаков и попарно в неизменном допуске1e-6м; max corresponding drift6.5868125745221e-7м. Покрытие трёх крупных стен и шести контактных плоскостей двух балок проверено в обоих запусках;126 base/zone links,63 layer-properties/group0,15 исходных GUID и hashes двух saved PLN сохранены. deterministic-final-summary.json/deterministic-final-readback.py; APX SHA2564f4fb65acf699f79db6669d63931fd6a9f7f6cf965074fe1c122e3ed3b9f95c4. Архикад оставлен с повторным результатом в VS Run, своих точек нет. Docs/generated, final runner, commit/push не выполнялись по ограничению владельца; #252 этим не закрывается.

Пункт2 остаётся частичным: исторические31пропуск=22Unmatched(419.699797722м²)+4диагональные трапеции колонн(2.375999498м²)+2микрополосы(0.000001419м²)+3торца внеscope(0.143677025м²). Синхронные source/zone снимки опровергли wall-owner у60/71: сохранённые старые грани и актуальные вырезы стены FA9703B0 различаются, контакт со старой геометрией не устанавливает сегодняшнее основание. Ни bbox, ни изменение slantAlpha не представляет четыре трапеции в действующем scalar writer; это не утверждение о невозможности SDK. Расширение writer требует отдельного решения; неизвестным baseGUID и двум микрополосам ложных элементов не назначено. Существующий runtimecounter31 сохранён, полного покрытия всех контактов не заявляется.

### Примыкание балки к профильной стене #252 — AC25 Windows

2026-10-11: BuildRoomBeamFinish получает surfaceSnapshot зоны; пересекает реальные конечные грани балки с сечением замкнутого 3D-тела. beamPart выбирает балку/связную часть, не ограничивает все высоты нижним контуром. Устранён зазор 0.250107 м: прежний missing низа 0.075033 м² и каждого бока около 0.155835 м²; верхняя полоса учитывает следующую ступень профиля. Независимый check-beam-junction.py: baseline RED, beam-gap и beam-gap-repeat GREEN по всем шести контактным плоскостям двух балок. Предварительный вывод о лишней отделке оказался ошибкой checker-а: были включены торцевые грани толщины перекрытий/стен; типы контактных элементов исправлены, baseline перепроверен.

Новый тест сначала упал при подготовке пустой служебной грани: VS ExceptionThrown, GS::Array this=0x8, TestRoombookGeometry.cpp:399. Небезопасный доступ устранён проверкой пустых контуров/вершин; повторная canonical build и новые self-tests 52/52 Passed, обе новые проверки ok в актуальной Отладка, ERROR IN TEST=0. Fresh direct LSP version1: TestRoombookGeometry0; RoomSurfacePartition один прежний unused-includes; Roombook13 прежних диагностик. Создано63 элемента (47 стен,16 перекрытий), повтор удалил63/создал63,126 links подтверждены;63 element-layer properties и intersectionGroupNr=0 прочитаны;15 исходных GUID сохранены; оба saved PLN hashes unchanged. Макс. gap контакта4.58e-7м. BodywiseGeometryEqual=false, покрытие обоих запусков подтверждено независимо. Открытые/разветвлённые сечения и внутренние отверстия beamPiece — явный атомарный отказ, не bounds-подстановка.

Артефакты scratch/z7-writer/beam-gap{,-repeat}-{beam-junction,wall-coverage,verification}.json, beam-gap-final-summary.json. APX SHA-256 b121cd842148b0248298896070d66a1a3dccc929b051267b7b073f21efb01f79. Archicad оставлен в VS Run с последним результатом. Docs/generated/final runner и commit/push не выполнялись по прежнему ограничению владельца; #252 OPEN. Следующее действие — оставшиеся торцевые стеновые участки по отдельному запросу, PLN не сохранять.

### Этап 1 #252 — типизированный reader и снимок OtdRoom (AC25 Windows)

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
