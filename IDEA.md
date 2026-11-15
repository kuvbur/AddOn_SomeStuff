# Current Task — #323: безопасное чтение свойств палитрой


## #333 — исключение граней профильной стены по материалу

Scope: AC25 Windows; только 3D-грани стен сложного профиля с именем покрытия, содержащим PARTIGNOREMATERIAL (!NOFIN!). Остальные основания и пользовательские изменения сохранить.
Status: WAITING_FOR_NATIVE — владелец разрешил адресные правки общих исходников; реализация и изолированная проверка завершены, host/APX/PLN не трогать.
Plan: [x] найти константу и зарегистрировать issue; [x] проверить SDK25 и GUID-кэш, реализовать профильный фильтр и исполняемые регрессии; [x] clang-format, полный изолированный Data TU C++14 /WX /c и fresh LSP; [ ] отдельно согласованная APX build/load и native-приёмка.
Last Completed: 2026-11-15 — RoomSurfaces_Read определяет API_ProfileStructure через один Element_Read; только Native3D-покрытия профильной Wall проверяются по Contains(PARTIGNOREMATERIAL). SurfaceMaterialNameRead владеет UniString и GSErrCode; RoomSurfaceCache.surfaceMaterialNames лениво кэширует успешные и неуспешные чтения по API_AttributeIndex. При nullptr локальный кэш разделяет индексы между гранями/телами одного snapshot. Индексы, тела, обычные/Composite Wall и другие типы сохранены; для не-Wall новых element/material API-чтений нет.
Decisions: ошибка определения Wall или имени покрытия не считается отсутствием маркера: snapshot получает profileWall/surfaceMaterialName и ReadError; неизвестные грани не публикуются. ВАЖНО: существующий RoomSurfaces_Partition отвергает весь неполный source через IsComplete(), даже если в снимке остались пригодные грани; это не обещание локального отказа одной грани. Штатный cleanup не изменён и сохранность прежней отделки исключённого источника не гарантируется.
Validation: exact production reader/Element_Read/material reader + реальные SDK25/GS/project types, controlled 3D/polygon/API seams: RED105checks/24FAIL → GREEN155checks/0FAIL (дополнительные проверки вызовов/ошибок исполняются только при соответствующих чтениях; fixed expectations одинаковы). Проверены повторные индексы/ошибки, GUID/new/null cache, длинное Unicode-имя, partial failure, ordinary/Composite Wall и другие типы. Полный Data TU MSVC14.44 C++14 /WX /c exit0; fresh version1 clangd — те же8diagnostics с diff-mapped ranges, новых0.2146headers неизменны во время каждого TU-прогона. clang-format/diff-check пройдены, formatter не внёс посторонней дельты. Data SHA4664a9a92f90ce5105ebc4e9cf6dc57e1151a47ae97dbd98294b2fea3786f6f3. Не выполнялись native/partition/writers/full host suite/APX build, commit/push; tests source и generated/progress не менялись в этом этапе.
Evidence: Hermes scratch/profile-ignore-333/{baseline-manifest.json,baseline/,run-regression.py,red-final/{report.json,results.log},green-final/{report.json,results.log},tu/{baseline,current}/{result.json,publishDiagnostics.json,documentSymbol.json},tu/current/comparison.json}; повтор: python C:/Users/da-rogojin/AppData/Local/hermes/cache/scratch/profile-ignore-333/run-regression.py green-repeat.
Parent Validation: прочитаны точная production-дельта и исходник стенда; повтор green-parent — compile/link/run exit0, TOTAL155FAIL0. Live Data/header SHA совпали с verification.json; git diff --check exit0. Это родительское статическое ревью и SDK-стенд с подменёнными API/polygon, не независимая native-приёмка.
Next Step: по отдельному разрешению canonical build/load/native, проверить реальную профильную и обычную Wall с generic Opening и повтор без накопления. Generated-координаты не обновлялись в этом этапе, что явно отмечено в карточке модуля.
Last Checkpoint: нет — commit/push не запрошены.
Issue: #333 https://github.com/kuvbur/AddOn_SomeStuff/issues/333

## #331 / #329 — исправление после новой native-проверки

Scope: AC25 Windows/C++14. Новый запрос владельца «Исправь код, чтоб работало как надо» разрешает исправление выявленных дефектов. #331: высотный допуск Mesh относительно исходного низа зоны с пределом 500 мм, без замены отдельного Mesh/Slab-арбитража и без nominal во втором проходе. #329: сначала доказать источник дефицитов101–103, не восстанавливать площадь слепым fallback. Сохранить отверстия, onlyOnSlab, допуски, ClampToParts, Helpers/Sync/Spec/_progress, исходные dirty-правки и PLN. Без commit/push/веток и остановки открытого host.
Status: NATIVE_FLOOR102_PASSED — разрешённая проверка через VS завершена: новый Debug APX, один native Slab-пол102 без разрыва/отверстия, точный контур прочитан обратно; повтор без новых/удалённых GUID. Это не полная приёмка стен/всех зон и не устранение разницы с Zone_MeasuredArea. Host оставлен Run; модель не сохранена, исходники/тесты/оба PLN неизменны. Issues не закрывались.
Plan: [x] F.1 — baseline/issue/scope; [x] F.2 — Mesh gate и повтор33/0; [x] F.102 — отдельный zoneFootprint и объединение совместимых Slab-частей, SDK RED/GREEN; [x] F.4a — clang-format, три TU C++14 /WX /c и fresh LSP; [x] F.4b — независимое production-ревью и отдельное ревью поздних test-добавлений родителем; [x] F.5a — адресные Docs/generated и память; [x] F.5b/NI.5 — native-приёмка именно пола102 и отсутствие накопления при повторе. Причины всех дефицитов101/103/106 и полная стеновая геометрия не объявляются установленными/исправленными.
Last Completed: 2026-11-15 — одинаковые646assertions: baseline61FAIL→current0FAIL; baseline имеет только три адаптера сигнатур, игнорирующие новый zoneFootprint. Полный собственный контур, один компонент, отверстия, обе разности покрытия, native/prepared/by_slab, обе mixed-обёртки, реальные разрывы и Mesh index/prefix проверены на SDK25. Узкий Mesh gate33/0; исторический общий Mesh suite65/5 остаётся отдельным непрошедшим набором. Data/TestMesh/TestRoombook TU /WX /c exit0; fresh diagnostics version1 те же8/1/0, новых нет. Data754a7ac7/header94490ddb/test3dec26f4 совпали с frozen TU/GREEN. Helpers/Sync/Spec/_progress/TestMesh byte-identical baseline; новых ACAPI-вызовов нет. symbols106координат+1helper,27унаследованных неоднозначных записей сохранены,867внешних неизменны; карточка обновлена. Открытый host и PLN не менялись.
Native Read-only: AC25 RUS7000/test_25; все5целевыхGUID существуют.102=6F2DC5F2-362E-445A-87F9-2EA2618DFE48 («Электрощитовая»), Zone_MeasuredArea9.970928903. Два Slab BB59E8E3/F15A50DE одного основания92E8F002, низ−0.05/верх0.05, площади8.171379336+0.684795630=8.856174966; у обоих Slab_HolesSurfaceArea=0. Bbox-разрыв поY11.454999971…11.850000076; точные контуры ещё не сняты, разность площадей1.114753937 не является контурным oracle. Loaded RSDS GUID2688F276-C658-41DF-827B-55B881BE226B/age27 совпал с APX/PDB;6sourceMD5 совпали. Код/APX/PDB/оба PLN после чтения byte-identical baseline. Панель «Отладка» после завершения сессии E_FAIL; причину завершения не утверждать. Брейкпоинты не менялись.
Native Trace: PID22844/PDB age27 на Data:5634 — floor_by_slab=true, Mesh-source1; входной room.poly — один прямоугольный контур без отверстий, площадь9.879225155435499м²; подготовленные3Slab и provenance сохранены в restart/stage-before-horizontal*.json. После прежнего прерывания свои3точки удалены (с учётом автоматического сдвига строк), остальные11сохранены. В34304 loaded/source/PDB повторно согласованы,304исходныхэлемента,0отделки102, config ready. Изменения видимости оснований из соседней задачи сохранены, агент их не редактировал.
Native Root Evidence: после horizontal и перед profile subtraction —1Floor/1контур без отверстий,9.879225155435499м². Сразу после subtraction —2прямоугольника8.171379336201717+0.6847956302016343, убрано1.0230501890321477м²; native-запись воспроизвела эти контуры. Первый успешный адресный проход:304→327,23GUID добавлены/0удалены, все23 возвращены и принадлежат102. Второй адресный проход снял per-wall профили/полосы4стен и259native-граней55250DEE: её сечение наZ0 лежит поY12.129999996…12.545, тогда как exclusionY11.455…11.850 пересекает102 (верхroom.poly12.1144). Стенаflipped=false,offset−0.4; offset не объявлять причиной. Сильная гипотеза — неверное направление поперечной оси в sign=flipped?1:−1; постоянная левая normal требует SDK-регрессии всех4захваченных случаев, не менять знак/offset вслепую. Счётчики второго прохода/отсутствие накопления пока не сверены. Свои4точки удаляются; host оставить Run. Состояние/модель не сохранять.
Profile Fix Validation: [x] NI.3 — native-derived RED15FAIL→GREEN0FAIL на полном raw profile/дугах четырёх стен; старые2части/8.8561749664м² →1компонент/9.8792251554м², обе разности покрытия0. Первый запуск checker остановился на срезе через native-проём, это не product RED; выбраны заранее сохранённые внутренние .1/.4/.8, сопоставляются фактические face-plane отрезки, не bbox. Полное совпадение outer native/profile не утверждается (20мм между гранями), проверены containment+midpoint. Постоянные35assertions TestRoomSurfacePartition: baseline9FAIL→current0FAIL с одинаковым хешем assertions. Исправлена прежняя synthetic-ожидаемая зеркальность, не подтверждённая native. Прежние646assertions0FAIL; полные Data/TestMesh/TestRoombook TU SDK25 MSVC C++14 /WX /c exit0, fresh LSPversion1 прежние8/1/0 без новых диагностик. Data3bf67b403dc1eaf3491e3a940ebdd854b32e19df52f8eff9f8bf39b0026a8911, testdc0eb9f821aaf14427f713dbe6a1591fa153b95416a8ed6823992d696f3bf691 совпали с frozen TU. [x] Формат/diff-check; [x] module card и3targeted symbols по fresh clangd; [x] procedural lesson в lightrag. Производственная дельта только3строки normal/comment; Helpers/Sync/Spec/_progress и оба PLN неизменны. Canonical `BuildAddOn.py -c config.json -v 25 -r -l INT` exit0, APXfdd1ba04b2ff9bbaa3e612b6a245e778e1987a160f04f31f50e2712a45d0c289; предупреждениеSDK LNK4099 оAPI_c.pdb. Debug-build не запускался, чтобы не перезаписывать build-copy PLN.
Native Oracle Audit: deleg_bf9a3463 принят после чтения audit.md и parent-повтора audit.py/sdk_probe.py: compile/link/run0/0/0, все4×3внутренних native-среза подтверждают левую нормаль. Wall2 старое исключение выходит за native на80мм, wall4 лежит с противоположной стороны. Аналитика дуг/native1–3 совпала на6/4/8границах, max residual4.0896468e-9м. Уточнение20мм: полная аналитика raw hatches совпадает с native; SDK Intersect наZ0 пропускает краевые штриховки около нижней границы (1–2нм), наZ=.01 возвращает их. Это не доказанная материальная политика; допуски/сечение не менять в sign-only задаче. Высокие сечения wall4 с нечётными crossings не приняты как oracle. Неподтверждённую фразу harness о rotation/reversal parent уже убрал до аудита; toggle — только synthetic helper-contract, не native mirror test.
Exact Delta Review: [x] NI.4 — deleg_3795d0e7: No issues found in inspected scope. Проверены только3строки production normal/comment и профильная test-дельта к pre-edit55e9e3bf/3dec26f4; прежние visibility/floor-изменения вне этого ревью. Родитель прочитал independent-exact-delta-review.md и повторно сверил Data3bf67b40/testdc0eb9f8/APXfdd1ba04; совпали. Reviewer не запускал сборку/тесты/host, execution — родитель. verification.json обновлён. Единственный существующий комментарий #294 обновлён итогом и прочитан обратно с точным совпадением body: https://github.com/kuvbur/AddOn_SomeStuff/issues/294#issuecomment-6897053670; статус issue не менялся.
Native Acceptance NI.5: 2026-11-15 23:30–23:35, PID12272, AC25 RUS7000/test_25. F5 реально скомпилировал Data/TestRoombookGeometry и перелинковал Debug|x64 INT, failures0. Loaded module `Build/SomeStuff/25/Debug/SomeStuff.apx` SHA256 fd4bdae6cc3c19df8c311bbae0718c02144b8236efbdf1ceecf86ed43754354f; source3bf67b40/testdc0eb9f8 и оба PLN совпали до/после. Все5GUID/config прошли gate. Data6124→6126:1Floor/1контур/5координат/0дуг, точные координаты и9.879225155435499м² неизменны. Draw2381:Create err0. После commit команды native JSON:Slab0F521F68-D296-4F4F-A7A2-E153524A9B93, base92E8F002, bottom−0.05/top+0.05, holes0, площадь9.879225155. На повторе Draw2192 после успешных Get/GetMemo и до conversion снят native memo:1контур/5координат/0дуг, координаты дословно равны входным/writer. Первый прогон304→326,22созданы/0удалены, все22успешны и принадлежат102, старые ownership неизменны. Повтор326→326,22успешных/0созданы/0added/0removed; GUID/площадь/отметки/bbox пола и всеownership неизменны.17стеновых GUID поменяли bbox; unordered ближайшие bbox совпадают до1.3e-8м, но это не доказательство равенства тел/полной стеновой стабильности. В обоих прогонах остался Unsupported finish pieces were skipped. Полный TestKit не заявляется. Все свои4точки удалены; исходные11точек и enabled-состояния восстановлены и прочитаны обратно; VS=Run. PLN не сохранялся, commit/push нет. Evidence:scratch/profile-native-acceptance-20261115/{baseline.json,after-f5.json,acceptance.json,final-hashes.json,*coordinates.json,room102-*-response.json,two-runs-debug-delta.txt,after-first,after-second}. Начальный checker-prefix отказ был переводом CRLF при локальной записи; сравнение исходных JSON-log snapshots подтвердило точный prefix, не product FAIL.
Next Step: после общего снимка по команде «коммить всё дерево» отдельно согласовать оставшееся предупреждение Unsupported finish pieces, стеновое сопоставление GUID и разницу Zone_MeasuredArea−floor=0.0917037476м²; другие зоны101/103/001/106 не пересчитывались. Host оставить Run, без push/закрытия issues. Native-приёмка относится к source3bf67b40; включённый позднее #333 не загружался. Текущий общий source/header snapshot проверен тремя полными TU SDK25 C++14 /WX /c и fresh LSP, exit0/новых диагностик0; live inputs совпали с frozen. Карточка дополнена native-итогом; generated для поздней #333 сохранён историческим по scope этого этапа. Evidence:scratch/zone102-unified-20261115/tu-precommit-all-20261115/result.json и scratch/commit-all-20261115/.
Last Checkpoint: `[NI.5-all] RoomBook: полы, профильные стены и регрессии` — текущий общий снимок всех11изменённых файлов по прямой команде владельца; точный SHA в Git. Это checkpoint кода/документации, не закрытие #329/#331/#333 и не новая native-приёмка позднего #333.
Issues: #331 https://github.com/kuvbur/AddOn_SomeStuff/issues/331; #329 https://github.com/kuvbur/AddOn_SomeStuff/issues/329 (OPEN). Профильное вычитание уже покрыто #294 https://github.com/kuvbur/AddOn_SomeStuff/issues/294 (CLOSED); после dedup добавлен комментарий о дефекте размещения/незавершённой приёмке, дубликат не создавался и статус issue не менялся.
Review: deleg_46a4a39a — No issues found in inspected scope; production/header hashes совпали. Test d074a591→3dec26f4 дополнен после ревью: родитель просмотрел точную дельту, а замена только этого блока восстановила полный reviewed SHA d074a591. Новые mixed/Mesh-prefix/disconnected assertions покрыты текущим646/0 и TU; это не независимое ревью более поздних тестов. Evidence: review.md, review-test-addendum.diff, review-acceptance.md. Legacy PopLargest сохраняется и не восстанавливает ранее отброшенные компоненты.
Root Cause: implementation error — 3D-проекция использовалась как единственная рамка пола, а соседние части одного Slab после разбиения/временного Mesh-domain не объединялись. Эти механизмы доказаны SDK-регрессией; их полный вклад в прежние6native-частей не доказан. Nominal остаётся конечным кандидатом, не рамкой: первая попытка неявного clipping по nominal сломала5старых assertions, была точечно отменена; явный zoneFootprint сохраняет прежний null-контракт. Новый Mesh-survivor fixture исправлен по действующему no-bottom clipping (опора ниже исходного roomFloor), production ради теста не менялся. Методика сохранена в systematic-debugging/SKILL.md.

## #329 — отбор перекрытий в пределах 500 мм

Scope: AC25 Windows/C++14; admission перекрытий пола/потолка, собственный полигон зоны в общей сортировке по отметке, адресные регрессии и документация. Пол: пересечение исходного низа зоны либо плита ниже с зазором от её верха <=0.5 м; потолок зеркально от исходного верха зоны. Собственный полигон пола сортируется по убыванию, потолка — по возрастанию; заниженный номинальный потолок закрывает основания выше него. Сохранить onlyOnSlab, Mesh, writers, Helpers, допуски и чужие изменения; без повторного номинального остатка. Владелец разрешил APX build/load и создание отделки в памяти для прежних зон 101/102/103/001/106 с независимой проверкой отметок и native-площадей полов. Без Save/правки PLN, закрытия чужого host, commit/push/веток; перед сборкой оба PLN сохранены в scratch, VS Design и Archicad processes=0.
Status: NATIVE_NOT_ACCEPTED — 2026-11-15 новый fixture PID23488: один явный RoomBook выполнен и независимо прочитан обратно. Отбор/контакт всех26Floor-Slab+1Ceil-Slab соответствуют правилу; общей приёмки нет: дефициты площадей101/102/103/106, два Mesh/Roof106 около−4.3м при низе0. Mesh по решению владельца не менять; причины дефицитов read-only проверяет deleg_d035aa4a. Ревью исправления deleg_606d560f принято, live SHA совпали. Host оставлен Run,5baseline-точек восстановлены и прочитаны обратно.
Plan: [x] 329.1 — dirty baseline/issue и уточнённый контракт; [x] 329.2 — общий admission и согласованный приоритет реальной плиты над nominal, RED/GREEN; [x] 329.3 — SDK25 TU/LSP и доставленное независимое ревью приняты, source SHA повторно сверены; P2 снят решением сохранить отдельную Mesh-политику, не исправлением арбитража; [/] 329.4 — адресные Docs/generated/память, итог после native; [/] 329.N — текущий новый fixture: fresh before/after GUID, реальный RoomBook для101/102/103/001/106, native-отметки и площади с разницей к зонам.
Offline Validation: P1 исправлен: явный isNominal, при точном равенстве реальная плита сортируется раньше зоны, nominal не оспаривает владельца; прежняя неоднозначность между реальными плитами сохранена. Одинаковые assertions SDK25 Geometry/C++14: RED527/20 → GREEN527/0; prepared/native × Floor/Ceil × onlyOnSlab × полные/частичные полигоны, GUID/площадь/номинальный остаток/отсутствие перепадов. Полные Data/test TU и analytic header SDK25 MSVC C++14 /WX /c exit0, fresh LSP8/0/0 без новых диагностик, live-хеши сверены с frozen snapshot. Mesh и 11 защищённых файлов byte-identical к tie-before. Это офлайн-результаты, не native-приёмка.
Last Completed: 2026-11-15 — native before304→after635:327finish+4Window106,0removed. Все327GUID уникальны, существуют и имеют правильные zone/base; Floor/Ceil-классификация прочитана независимо. Прежние ownership не изменились,4исходных bbox XY изменились на5мм — полную неизменность геометрии не заявлять. Loaded APX2adb8496/PDB7e9c876d согласованы с7sourceMD5; источники/APX/PDB/оба PLN после прогона совпали с run-artifacts.json. 12baseline-позиций/enabled/conditions/hit rules восстановлены (5enabled/7disabled), host Run. Без Save/restart/rebuild/commit/push; прежний canonical BuildAddOn exit0/APX80d4d462 — отдельный исторический снимок.
Fixture Recovery: оба PLN уже ДО этой сборки имели ee6e36b1.../24680496bytes, а прежний native-fixture —1d7fd9590db61c56d4045a2d1314b336f33926399109765e6cfaaa04d4eedbd2/291922160bytes. Время/виновник прежней замены не установлены. Две защищённые копии с точным историческим SHA найдены: Hermes scratch/profile-contact-fix-20261113/baseline/Build/SomeStuff/25/test_25.pln и finish-mesh-diagnosis-20261113/baseline/Build/SomeStuff/25/test_25.pln. Не перезаписывать текущие PLN; открывать копию только после разрешения и заново проверять GUID/config.
Native Resume (2026-11-15): новый fixture открыт владельцем, старые backups не открывать/не восстанавливать. Source PLN dcb4bb82..., build-copy b0bfd5a5... различны, оба защищены в scratch/slab-height-fix-20261114/native-20261115/protected/; не копировать один поверх другого. Read-only preflight подтвердил101/102/103/001/106 с прежними GUID и native-площадями3.131705159/9.970928903/8.810260666/99.799999814/66.562911053м². Первая scratch-проба ошибочно вызвала несуществующее CheckFinishConfig (4010); исправлена только обвязка по живому GetName на Roombook_CheckFinishConfiguration и поле succeeded. Повторный read-only gate прошёл, RoomBook ещё не вызывался. PDB/source-check и gate.json сохранены.
Native Result (2026-11-15): зона→сумма native-проекций (м²):101 3.131705159→2.583001200 (−17.5209%);102 9.970928903→9.192860319 (−7.8034%);103 8.810260666→8.101857658 (−8.0407%);001 99.799999814→99.799999311;106 66.562911053→59.625603407 (−10.4222%). Slab_GrossTopSurfaceAreaWithHoles + Roof_NetBottomSurfaceArea*cos(Pitch),12вертикальных Floor/Wall001 исключены. Native thickness/cos(Pitch)=verticalThickness подтвердил радианы. Сумма001 совпала, но union/контакт/полнота не доказаны. Все27slab admission/contact checks пройдены; прежних +3.7м/удалённого slab101−4.4м нет. Два Floor/Roof106 изMeshEB1CAB9D: bbox−4.36…−4.10 при низе0 — общий elevation FAIL. Debug-delta: coverage106×1,WallPart×6,ConvertToProperty типа плинтуса×16; новых source-правок/повтора нет.
Next Step: принять read-only вывод deleg_d035aa4a о финальных подрезках и сверить конкретные live строки; отдельно согласовать изменение замороженной Mesh-политики, не расширяя #329 молча. Дефициты не объявлять доказанными дырками без контурного oracle. До этого RoomBook повторно не вызывать, исходники/настройки не менять, host не перезапускать. native-acceptance.json/native-verification-report.md/debug-first-delta.txt/after-native-integrity.json и первичные API сохранены в scratch/slab-height-fix-20261114/native-20261115/. Docs/generated не пересобирать для одного runtime-прогона без изменений C++; историческую карточку актуализировать при запрошенном коммите. Data2b77b1358d15facd5be01f496433af895c7e79491a8cee63f3a1307d0626b10a,Testccf74cd6db1e384a02d797b9da811919e5f1ee30d30468c18bad7e2a6803f11e совпали с ревью.
Last Checkpoint: нет — commit не запрошен.
Issue: https://github.com/kuvbur/AddOn_SomeStuff/issues/329 (OPEN).
Separate Mesh Bug: https://github.com/kuvbur/AddOn_SomeStuff/issues/331 (OPEN, body/labels прочитаны обратно). XY-only Mesh_FindAll:6672 и неограниченный по дальности MeshFloorFinishes_BuildProjectedImpl:3078 подтверждены по коду; неверный Roof106 подтверждён native. Изменение Mesh-политики не разрешено; #329-код не расширять автоматически.
Evidence: Hermes scratch/slab-height-fix-20261114/.


## Профильные стены: смещённые контакты (#252) и мелкие исходные грани (#327)

Scope: AC25 Windows/C++14; profile-only согласование конечных wallPart и ранний фильтр исходных негоризонтальных Wall-граней высотой строго <5мм. Реализация/адресные регрессии/Docs завершены ранее; текущая native-проверка по «Я запустил архикад в дебаге vs. Убедись, что всё работает без ошибок» — только открытый test_25 PID17152 и5явных Zone GUID. Не менять допуски/проёмы/внутренние разрывы, Helpers/материалы/избранное/PLN; без rebuild/restart/commit/push/веток и новых production-правок.
Status: WAITING_FOR_FIXES — проверка выполнена, «без ошибок» не подтверждено; полная приёмка PF.N/#327 остаётся открыта.
Plan: [x] PF.1–PF.5 — baseline/SDK, clamp/filter, RED→GREEN, TU/LSP/review, Docs/generated/память; [/] PF.N — native-проверка выполнена с оставшимися отказами, canonical build/runner и полная геометрическая приёмка не закрыты.
Last Completed: 2026-11-14 — два реальных RoomBook с теми же5Zone, независимые GUID/ownership/native type/bbox read-back, targeted TestKit и решающий breakpoint; собственные точки сняты, исходные12позиций/enabled восстановлены и прочитаны обратно; host оставлен Run. Production/tests не изменялись во время native-проверки, оба PLN/APX/PDB и8source/header совпали с baseline по SHA.
Offline Validation (предыдущий этап): exact SDK25/C++14 clamp+reader RED55/13FAIL→GREEN55/0, persistent54/0, combined88/0, полные TU MSVC /WX /c exit0, fresh LSP без новых диагностик; независимое ревью — No issues found in inspected scope. Это не native-приёмка.
Native Validation: loaded APX/PDB identity и6PDB source MD5 подтверждены. Startup90suites/38813passed/223failed в8наборах; отдельный реальный TestRoomSurfacePartition395/8. Восьми отказам соответствует дефект fixture draw_type (статически подтверждён review+родителем); #328 OPEN, исправление не выполнялось. Прочие причины startup-отказов не локализованы.
Native Result: первый ответ276новых finish, второй287 (273reuse/14created), все реально существуют, write_succeeded=true/draw_error0, zone/base связи совпали. По101/102/103/001/106:20/22/17/63/154→20/22/17/63/165. В106 подготовка изменилась, причина/дальнейшая стабилизация не установлены. Второй inventory +20/-7:14новыхfinish+6Window;3старыхfinish+4Window удалены, прежних unreported finish-survivors0. Исходные1567GUID/ownership и1493native bbox не изменились; остальные свойства/точная топология не проверены. Mesh EB1CAB9D в001 дал4новых Roof, те жеGUID сохранились на повторе.
Native Paths: на Data:410 реально пропущена14-вершинная Native3D Wall-грань высотой0.002535209269845м; на:2240/:5629 профильный offset-contact принят (strict0/shifted1/corrected1, height0.075858923683854). Это не полное покрытие стены и не доказанное совпадение с прежним контактом0.131м.
Remaining: Debug первого/второго —4/3WallPart rejects, coverage106, Ambiguous001 и16/16EWT Node propagation diagnostics; writer rollback/unsupported beam не наблюдались. На Data:5631 отказ17630DA9 в106: X10.145…10.335 против фактических wallPart X0.25…5.75 и6.25…10.025; native corrected0 и replay frozen inputs подтверждают отсутствие продольного перекрытия. Не обходить конечные интервалы ради этого контакта. Верность исходного wallPart и причины других отказов не установлены.
Next Step: отдельная правка fixture #328 с повтором полного suite; отдельно разобрать coverage/изменение подготовки106, затем полное покрытие/повтор без накопления. Третий мутирующий вызов не выполнялся. Нужен новый согласованный этап; не закрывать #327 и не перезапускать текущий host автоматически.
Last Checkpoint: нового нет — commit/push не запрошены; native-проверка привязана к HEAD1d2a740 и сохранённому dirty snapshot.
Decisions: #328 https://github.com/kuvbur/AddOn_SomeStuff/issues/328 — дефект тестового входа, не доказанная ошибка Boolean/регрессия профильного фильтра; не менять production ради fixture.
Evidence: Hermes scratch/profile-contact-native-20261114/native-verification-report.md; первичные JSON/Debug-delta/PDB/source manifests и frozen native inputs рядом. Полный результат и границы — там; счётчики/повтор не заменяют независимого oracle полного контакта.

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
- Issue #330 (RoomBook): при обновлении выбранных зон включать все зоны того же типа отделки, если расчёт идёт по типам (2026-11-14)

- #325 — опциональный уровень коррекции QR в Sync_from; постановка: https://github.com/kuvbur/AddOn_SomeStuff/issues/325. Реализация не начата.
- Не создавать ветки; не коммитить и не пушить без явной команды.
- Завершённые на GitHub #252/#270 и другие закрытые потоки не возвращать в активный план по историческим записям.
- История до аудита сохранена целиком, кроме «Грабли», которые остаются ниже в IDEA; старые утверждения о OPEN/коммитах/блокировках в архивном снимке относятся к моменту записи.
- Evidence: C:/Users/da-rogojin/AppData/Local/hermes/cache/scratch/issue-audit-20261108/ (before-manifest.json, idea-issue-states.json, final-runner.log, final-runner-result.json).

## Archive

### Видимость и классификация оснований RoomBook — закрыто владельцем

Status: CLOSED_BY_OWNER; проверка видимости слоя/реконструкции перед проверкой класса добавлена для стен, перекрытий, Mesh, колонн и балок; отдельный фильтр окон также остаётся в коде. Штатное удаление прежней отделки скрытого основания при пересчёте сохранено по решению владельца. Generic Opening не изменён.
Validation: Wall/Slab controlled RED 8/2 → GREEN 8/0, Column/Beam 16/0; SDK и часть зависимостей подменены. RoombookData.cpp SHA256 55e9e3bfababe07891312da8ef5ff71e46f6f0fe27e9b49144f2411d5b7a6996 компилировался SDK25 MSVC /WX /c в scratch OBJ, не APX. Native-приёмка, Window/Door и generic Opening не проверены; legacy ошибка класса остаётся fail-open. Карточка Roombook дополнена; generated не обновлён. Код не закоммичен; checkpoint нет. По решению владельца — без issue; ранее по ошибке созданный #332 не изменялся после уточнения.

### Диагностика отсутствующей отделки и Mesh-пола — 2026-11-13

Scope: только AC25 Windows / Build/SomeStuff/25/test_25.pln, VS и разрешённое построение в памяти без Save; production, настройки/избранное, commit/push/ветки и изменение состояния issues вне scope.
Status: DIAGNOSED — причина части стен подтверждена; отсутствие Mesh-пола зоны001 не воспроизвелось. Исправление не запрошено, полная геометрическая приёмка не выполнена.
Plan: [x] D1 — baseline/host; [x] D2 — один явный JSON-прогон пяти зон, native-отладчик и независимый инвентарь; [x] D3 — сверка кода/результатов, сохранность, восстановление точек и фиксация ограничений.
Last Completed: SurfaceWall_ClampToParts (RoombookData.cpp:2148–2167,2253) отбрасывает параллельный высотный контакт профильной стены 17630DA9… с поперечным смещением ~0.131м от планового ребра зоны; найденные wallPart оставляют пустые интервалы/результат. Wrapper:5573–5577 удаляет этот prepared-участок до writer. Wrong assumption/implementation error, не отказ избранного/API. Журнал:99 WallPart contact rejected /9GUID. Тот же дефект ранее описан в #252; опубликован один краткий комментарий, прочитан обратно, CLOSED сохранён, дубликат не создан.
Validation: один прогон101/102/103/001/106 создал4/11/6/63/94 =178новых GUID; все178независимо существуют, removed0, все возвращённые записи write_succeeded. В001 поMesh EB1CAB9D… созданы4новых nativeRoof с рольюFloor8 и ненулевымиXYZbox. Полный контакт/покрытие/повтор не проверены. ДругиеMesh не приняты этим результатом; верхниеMesh3.7/6.3м и zoneFloor0 сопоставлены с кодовым roomFloor admission (:3051–3123), но их hasBottom/eligibility в debugger не проверены. Дополнительно PartitionInvalidSourcePlane у102/103/106 и coverage106 — отдельный root не локализован; общего abort и Mesh floor ошибок в этом прогоне нет.
Build/Safety: F5 реально перекомпилировал существующую Spec.cpp и перелинковал Debug APX, VS2success/0failed; canonical BuildAddOn.py/runner/TestKit не запускались. Loaded APX2707e011…; обаPLN и все сохранённыеC++baselineхеши неизменны. Все собственныеbp удалены, исходные9восстановлены и список сверён. Host50788 оставленRun с результатом в памяти безStop/Save. ЧужиеHelpers/Sync/Spec/progress сохранены.
Next Step: по отдельной команде исправлять согласование wallPart с высотным сечением профиля, сохраняя конечные границы/разрывы; если пропуск Mesh относится не к001/EB1CAB9D, сначала уточнить конкретную зону/основание. Не увеличивать допуск и не выдавать native-box за solid-приёмку. Нового исправления/коммита нет.
Last Checkpoint: 5b15c05; commit/push не запрошены.
Evidence/Memory: Hermes scratch/finish-mesh-diagnosis-20261113/report.md, first-run*.json, mesh-native-check.json, debug-final.txt, final-preservation.json; результат отправленOpenViking, методика дополнена в visualstudio-cpp-debugger/references/archicad-somestuff.md. #252 comment6855219420.

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
