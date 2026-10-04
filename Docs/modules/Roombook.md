# Roombook — Спецификация отделки

> Основа: 1e67983 (2026-09-22); дополнение #195 — после 0cfe635; P3a #260 — d8fb525; P3b #260 — 9e041dc; P3c #260 — рабочее дерево после 29a6606 (2026-10-03, AC25). P3d #260 — рабочее дерево поверх 6231b74 (2026-10-03, AC25). Номера строк остальных подсистем ниже относятся к основе и требуют обновления по живому коду. Назначения — по именам функций (при разборе модуля уточнять).

## Назначение
Спецификация отделки: генерация ведомостей отделочных материалов и работ из модели. [по коду]

## Точка входа
- `RoomBook` (Roombook.cpp:674; AC22–23 — заглушка :23) ← `MenuCommandHandler` (SomeStuff_Main.cpp; граф содержит старые номера строк) и `SomeStuffCommand.RoomBook` (JSON API AC25–29; вызывающий выбирает экземпляр Archicad по HTTP-порту). JSON-команда возвращает время до выхода из `RoomBook`; успех расчёта отдельно не подтверждается. [по коду / DevKit-25 API_AddOnCommand]
- `GetTargetZones` (Roombook.cpp:34) — выбор выделенных редактируемых зон или fallback на все доступные; возвращает false при ошибке/пустом списке [по коду]
- `PrepareRoomProcessingContext` (:92) — заполняет контекст: `Class_FindFinClass` → `GetStories` → `Floor_FindAll`; контейнеры живут до выхода из `RoomBook`. [по коду]
- `AdvanceProcessPhase` (:101) — единственная точка обновления прогресса: увеличивает `nPhase`, сообщает фазу и возвращает признак отмены; на AC27+ `ACAPI_ProcessWindow_SetNextProcessPhase`/`IsProcessCanceled`, иначе `ACAPI_Interface` с `APIIo_SetNextProcessPhaseID`/`APIIo_IsProcessCanceledID`. Раньше эти блоки были скопированы в девяти местах. `nPhase = 1` сбрасывается в `RoomBook` перед `ProcessWindowGuard`. [по коду]
- `BuildElementReadIndex` (:112), `ProcessElementsForRoomData` (:126) — индекс, прогресс/отмена, `ClearZoneGUID`, классификация и dispatch. [по коду]
- `PrepareReadParams` (:183), `ReadElementParameters` (:185) — подготовка и чтение параметров; `ReadParamsForRoomBook` хранит параметры окон и комнат. Временный `paramDict` создаётся в `ReadElementParameters`, очищается между типами и не хранится в `RoomProcessingContext`; `paramDict_favorite` остаётся в контексте. Порядок чтения не менялся. [по коду]
- `ProcessRoomFinishes` (:516) обходит комнаты; `ProcessSlabFinishes` (:209), `ProcessWallFinishes` (:289), `ApplyFavoriteAndMaterialData` (:272) сохраняют порядок создания/настройки отделки. [по коду]
- `BuildMaterialSummaryForRooms` (:380), `WriteRoomMaterialData` (:485) используют общий `MaterialSummary`; `paramToWrite` затем передаётся `SetSyncOtdWall`. [по коду]
- `RemoveUnusedFinishingElements` (:571), `PrepareElementsForUpdate` (:609) — сбор GUID на удаление и разблокировка/резервирование; отказ в резервировании завершает `RoomBook` до отрисовки. [по коду]
- Комментарий `REFACTOR PLAN` в Roombook.cpp:654-673 описывает этот рефакторинг. [из комментария]
- P3e #260 (рабочее дерево после b3cc8c2, AC25 Windows): в `ApplyFavoriteAndMaterialData` (:270), `WriteRoomMaterialData` (:475) и `RemoveUnusedFinishingElements` (:548) сняты десять локальных алиасов; используются прямые обращения `context.*`/`summary.*`. Список аргументов, порядок обходов и вызовов сохранены. Собран AC25, TestKit 69/2567/0; один запуск через VS подтвердил частичный read-back 1778 элементов без различий. Продолжение A/B отменено владельцем: замеры производительности только для крупных изменений. Полные контуры/материалы — not verified. [по коду + runtime]
- Правка P3d: в теле `RoomBook` (исторически :609) сняты пять `auto &`-алиасов на поля контекста
  (`storyLevels`, `deletelist`, `exsistot_byzone`, `roomsinfo`, `paramDict_favorite`) — обращения идут
  через `context.<поле>`. Оставшиеся алиасы (`finclass`, `finclassguids`, `paramToRead`) сохранены:
  поля используются в нескольких соседних вызовах. Порядок операций не менялся. [по коду]
- Исторический список вызовов из Docs/_generated/body_scan.json (96 до #195) устарел после извлечения helpers; актуальные вызовы сверять по `Sources/AddOn/Roombook.cpp`. Подсистемы: `Param_*`, `OtdData_*`, `OtdWall/OtdBeam/Opening/Floor_*`, `Class_*`, `Favorite_*`, Sync, PROPERTYCACHE и SDK ProcessWindow/Teamwork. [по коду; полный граф — не проверено]

## Функции по подсистемам [по коду / по имени]

### Зоны и сбор данных
`Otd_GetOtd_ByZone` (:758), `BuildOtdByParent` (:876), `Otd_GetOtd_Parent` (:917), `CollectRoomInfo` (:1206), `ClearZoneGUID` (:3394), `Edges_GetFromRoom` (:3325), `Edge_FindOnEdge` (:3944), `Edge_FindEdge` (:3975), `typeinzone`/`reducededges` — глобальные переменные namespace (:25/:28) [по коду]

`BuildOtdByParent` преобразует результат `SyncGetSubelement` из формы `GUID базового элемента -> GUID дочерней отделки` в индекс `база -> тип отделки -> GUID отделки`. Внешний ключ остаётся GUID базы, тип определяется по внутреннему GUID отделочного элемента; неизвестные дочерние GUID пропускаются. [по коду; #193]

### Данные отделки (OtdData)
`OtdData_GetColumnfFormat` (:812), `OtdData_CalcForRoom` (:903), `OtdData_WriteToRoom` (:996), `OtdData_AddValueToDict` (:1138), `OtdWall_GetArea` (:1167)

### Создание элементов отделки
`OtdWall_Create_FromWall` (:1412), `OtdWall_Create_FromColumn` (:1671), `OtdWall_Add_One` (:1642), `Opening_Create_One` (:1363), `Opening_Add_One` (:1610), `OpeningReveals_Create_One` (:3432), `Floor_FindAll` (:1768), `Floor_FindInOneRoom` (:1814), `Floor_Create_All` (:1859), `Floor_Create_One` (:1912)

### Параметры (Param)
`Param_ToParamDict` (:2080), `Param_GetForWindowParams` (:2094), `Param_GetForBase` (:2146), `Param_GetForRooms` (:2290), `Param_Property_FindInParams` (:2528), `Param_Property_Read` (:2577), `Param_Material_Get` (:2614), `Param_SetToRooms` (:2642), `Param_SetToBase` (:2963), `Param_SetComposite` (:3026), `Param_SetToWindows` (:3140), `Param_AddUnicElementByType` (:5287), `Param_AddUnicGUIDByType` (:5315)

### Разделение стен и материалы
`OtdWall_Delim_All` (:3589), `OtdWall_Delim_One` (:3725), `SetMaterialByType` (:3794), `SetMaterialFinish_ByComposite` (:3904), `SetMaterialFinish` (:3915), `RoomRedProc` (:3269)

### Отрисовка (Draw) и дефолты
`Draw_Elements` (:3992), `OtdWall_Draw` (:4152), `OtdBeam_Draw_Beam` (:4181), `OtdBeam_Draw_Object` (:4253), `OtdWall_Draw_Object` (:4291), `OtdWall_Draw_Wall` (:4450), `Opening_Draw` (:4538), `Floor_Draw` (:4619), `Floor_Draw_Slab` (:4650), `Floor_Draw_Object` (:4751); дефолты: `OtdBeam_GetDefult_Beam` (:4234), `OtdBeam_GetDefult_Object` (:4264), `OtdWall_GetDefult_Object` (:4423), `OtdWall_GetDefult_Wall` (:4509), `Opening_GetDefult` (:4597), `Floor_GetDefult_Object` (:4896), `Floor_GetDefult_Slab` (:4920)

### Классификация отделки (Class)
`Class_SetClass` ×3 (:4952/4963/4976), `Class_GetClassGuid` (:4979), `Class_GetOtdTypeByClass` (:5026), `Class_FindFinClass` (:5070), `Class_IsElementFinClass` (:5272), `SetSyncOtdWall` (:5141)

### Избранное (Favorite)
`Favorite_GetType` (:5327), `Favorite_GetDict` (:5342), `Favorite_ReadComposite` (:5412), `Favorite_FindName` (:5449), `Favorite_GetByName` ×2 (:5554/5572)

### Прочее
`Check` (:5594, до :5685 — [назначение не установлено; вероятно, итоговая проверка])

## P3f/P3g — явные зависимости helpers (#260)

- `ApplyFavoriteAndMaterialData` (`Roombook.cpp:270`) принимает `GS::Array<OtdWall>&`, `ParamDictValue&`, `ParamValue&`, `MatarialToFavoriteDict&`; вызывается из `ProcessWallFinishes`. Полный `RoomProcessingContext` не передаётся. `Favorite_FindName`, проверка composite и Append идут в прежнем порядке. [по живому коду]
- `WriteRoomMaterialData` (`Roombook.cpp:471`) принимает `OtdRooms&`, `ParamDictElement&`, `MaterialSummary&`; вызывается из `RoomBook`. Передаются те же контейнеры по ссылке без копий; последовательность формирования paramnamebytype и OtdData_WriteToRoom сохранена. [по живому коду]
- AC25 Windows Debug: оба шага собраны `BuildAddOn.py`, исполнены через VS на чистой PLN; read-back 1778 элементов без различий в box/доступных свойствах, исходные GUID сохранены. TestKit P3g: 69/2567/0 (свежий отчёт 23:27:48). Полные контуры/материалы и устранение сообщений paramTo.isValid — not verified.
- Clangd отдаёт stale documentSymbols (конец ApplyFavoriteAndMaterialData :288 вместо :284); hover timeout. LSP и обновление generated для этих шагов — not verified; старые координаты не переносятся.

## P3h — явные зависимости очистки (#260)

- `RemoveUnusedFinishingElements` (`Roombook.cpp:544`) принимает `GS::HashTable<API_Guid, UnicGuidByBase>&` и `GS::Array<API_Guid>&`; `RoomBook` передаёт прежние `context.exsistot_byzone`/`context.deletelist`. Тело вложенного обхода и порядок Push неизменны, копии не создаются. [по живому коду]
- AC25 Windows Debug: BuildAddOn.py success; VS read-back `p3h_vs_correctness1` — 1778 новых элементов, missing/extra=0 по box и доступным свойствам, потерь исходных GUID=0. Свежий TestKit 23:37:13 — 69/2567/0, EXIT 0. Панель VS содержит paramTo.isValid; отсутствие runtime-ошибок и полная эквивалентность контуров/материалов не заявляются.
- LSP/generated пропущены с явного разрешения владельца только в этом ходе, not verified. В установленном MCP FileTracker.ensureFileOpen не обновляет уже открытый файл; timeout диагностики превращается в []. Пакет MCP и чужие процессы не менялись.

## P3i — обработка отделки с конкретными входами (#260)

- `ProcessSlabFinishes` (:207) получает ссылки на paramToRead, param_composite, paramDict_favorite, exsistot_byzone вместо полного контекста; `ProcessWallFinishes` (:286) получает параметры и windowParams вместо контекста/ReadParamsForRoomBook. Перечень вычислений, порядок вызовов, присваивания и индивидуальная рабочая копия параметров проёма сохранены. [по живому коду]
- AC25 BuildAddOn success; VS p3i_vs_correctness1: 1778 новых элементов, missing/extra=0 по box/доступным свойствам, исходные GUID сохранены. TestKit 2026-10-04 00:13:42 — suites=69 passed=2567 failed=0, EXIT 0, TestBuildOtdByParent 12/0. Полные контуры/материалы not verified, сообщения paramTo.isValid остаются.
- Свежий изолированный MCP: endLine совпадает с 5672 строками текущего файла; прежние 8 unused-variable + 5 unused-includes до/после, новых диагностик нет. Все 112 записей Roombook symbols.json пересобраны из свежего documentSymbols; каждое имя присутствует на указанной строке. Остальные модули не регенерированы. Старые записи stale выше описывают исторические проверки, не текущую коллекцию.

## P2b1/P4a — расчёт проёма отдельно от добавления (#260)

- `CalculateOpeningForWall` (`Roombook.cpp:1679`) — локальный расчёт обрезанного интервала и координат в существующий OtdOpening. Арифметика и строгая проверка newLeft < newRight сохранены; публичный `Opening_Add_One` (:1711) только вызывает расчёт и добавляет результат в прежний массив. [по живому коду]
- Вызывается только из Opening_Add_One (:1719); исходящий project-callgraph пуст, подтверждено fresh MCP callHierarchy. SDK-вызовы не добавлены. [по clangd]
- `TestOpeningAddOne` (`tests/TestCore.cpp:157`): 18 сценариев, 114/0 до/после; исходные поля, порядок, отражение и края интервала закреплены. Реестр 70/70/70; TestKit 70/2681/0, EXIT 0. BuildAddOn AC25 success до/после; VS baseline/candidate по 1778 элементов missing/extra=0 и потерь GUID=0. Fresh LSP: прежние 8 unused-variable и 5 unused-includes, новых нет. Полные контуры/материалы и отсутствие paramTo.isValid не заявляются.

## P2b2a/P4b — вертикальная подрезка проёмов (#260)

- `TrimOpeningsToWallHeight` (`Roombook.cpp:3786`) выделяет прежний цикл из `OtdWall_Delim_One` (:3812). Единственный caller — OtdWall_Delim_One (:3847); outgoing project calls=0. [по коду и fresh clangd callHierarchy]
- Условия касания, min_dim, порядок проёмов и перезапись zDup/zDown внутри цикла сохранены. Исходная стена не меняется: работа на прежней localCopy, материал/тип и Push остаются в вызывающей функции. Публичная const-сигнатура не менялась в итоговом diff. [по коду]
- `TestOtdWallDelimOne` (`tests/TestCore.cpp:227`): 15 одиночных сценариев (обрезка снизу/сверху, перекрытие, касание, outside, нулевые/min_dim размеры, отказ стены и сохранность исходного массива), 136/0 до/после. Многопроёмный порядок ещё не характеризирован отдельным тестом, прежний цикл перенесён целиком. [по тестам и коду]
- TestKit 71/2817/0, EXIT 0 (00:41:52/00:44:33), реестр 71/71/71; BuildAddOn AC25 success. VS p4b_vs_candidate1: 1778 новых элементов, missing/extra=0 по box/доступным свойствам, потерь исходных GUID=0. Fresh LSP: прежние 8 unused-variable + 5 unused-includes в Roombook, TestCore 0 ошибок/3 unused-includes. Полная геометрическая эквивалентность not verified.

## P2b2b — многопроёмный RED (#261)

- Добавлены два порядка полностью внутренних проёмов в TestOtdWallDelimOne: стена [0,10], проёмы [1,3] и [5,7]. A→B теряет B; B→A сохраняет оба. Входный массив не меняется. TestOtdWallDelimOne 149/1, TestKit 71/2830/1, EXIT 1 (2026-10-04 00:56:03), AC25/VS. [по исполнившемуся тесту]
- TrimOpeningsToWallHeight (:3792/:3800): проверка начала следующего проёма использует zDup, уже перезаписанный верхом предыдущего. Тот же цикл присутствует в HEAD до P4b. [по коду и git show HEAD]
- Дефект https://github.com/kuvbur/AddOn_SomeStuff/issues/261 открыт и прочитан обратно. Production не исправлялся; RED-тест не отключён. Следующие извлечения остановлены; необходим отдельный bugfix с RED→GREEN. Модельное проявление на реальном проекте not verified. Fresh TestCore LSP: прежние 0 errors/3 unused-includes.

## #261 GREEN и P2b2c/P4c — тип полосы отдельно от разбиения

- #261: стабильный wallTop вместо перезаписанного zDup в отсеве следующего проёма; отдельный bugfix субагента принят родителем по точному diff, SHA-256 APX, свежести отчёта и диагностик. TestOtdWallDelimOne 149/1 → 158/0, TestKit 71/2839/0 (01:08:06). Issue открыта; прежний раздел RED выше — историческое наблюдение до фикса. [по diff и реально исполненным тестам]
- GetWallFinishBandType (`Roombook.cpp:3652`) вычисляет прежний тип для Down/Main/Up. Порядок if сохранён: у верхней полосы Window проверяется после Ceil/Floor, у нижней и основной — до. Fallback не переносился. Единственный caller OtdWall_Delim_All (:3688), call sites :3711/:3729/:3751, outgoing project calls=0. [по коду и fresh clangd callHierarchy]
- TestOtdWallDelimAll: 8 комбинаций Wall/Column/Slab/Window/Ceil/Floor и смешанных Window+Ceil/Floor плюс fallback без высот; проверены порядок трёх полос, границы, материалы/типы, сохранность sentinel и исходной стены. 149/0 до/после, TestKit 72/2988/0 (01:16:29/01:19:33), реестр 72/72/72. AC25 BuildAddOn success; VS p4c_vs_candidate1 1778 новых элементов missing/extra=0 по доступным box/свойствам, GUID сохранены. [по выполненным тестам/прогону]
- Fresh MCP: Roombook прежние 8 unused-variable + 5 unused-includes; TestCore 0 ошибок/3 unused-includes, TestFunc 0 ошибок/2 unused-includes. Полные контуры/материалы not verified. Symbols этих трёх файлов и 3 новые helper-границы обновлены точечно, унаследованная остальная часть не регенерировалась.

## P2b2d/P4d — fallback после отфильтрованных полос (#260)

- GetFallbackWallFinishType (`Roombook.cpp:3688`) получает текущий type последней попытки. Четыре независимых if перенесены в прежнем порядке Column → Window → Ceil → Floor; обычная стена не сбрасывается в Wall_Main. Caller OtdWall_Delim_All (:3700), call site :3783; incoming=1/outgoing=0 по fresh MCP. Весь production после обратной подстановки helper совпадает с pre-step снимком по токенам, без изменений вне этого извлечения. [по коду и clangd]
- TestOtdWallDelimAll (:368): добавлены fallback после отвергнутой нижней полосы (Wall_Down/down), одна пересекающая полоса без fallback-дубликата, отсутствие пересечений и два проёма [1,3]/[5,8] через три полосы в обоих порядках. Проверены абсолютные/относительные отметки, высоты, число и порядок проёмов; исходный массив не меняется. 197/0 до/после, TestKit 72/3036/0, EXIT 0 (07:34:51/07:37:19). [по исполненным AC25/VS тестам]
- AC25 BuildAddOn success до/после; свежий MCP Roombook прежние 8 unused-variable + 5 unused-includes, TestCore прежние 0 errors/3 unused-includes. VS p4d_vs_candidate1: 1778 новых элементов, missing/extra=0 по доступным box/свойствам, исходные GUID сохранены. A/B времени для локального извлечения не проводился; полные контуры/материалы not verified. [по инструментам]
- Текущие координаты после P4d: GetWallFinishBandType :3652; OtdWall_Delim_All :3700; TrimOpeningsToWallHeight :3805; OtdWall_Delim_One :3832. Координаты в предыдущих разделах относятся к соответствующим историческим шагам. [по fresh symbols]

## P2b2e/P4e — вертикальное пересечение стены (#260, 2026-10-04)

- `CalculateWallHeightIntersection` (`Roombook.cpp:3832`) — прежние guards и арифметика диапазона из `OtdWall_Delim_One` (:3855). Меняет только ссылки на четыре локальных double вызывающей функции; при false массив отделки и исходная стена не меняются. Публичный const-вход и material/type-сигнатура прежние, TrimOpeningsToWallHeight и SetMaterialByType остаются в caller. После inlining, удаления оболочки и восстановления двух локальных объявлений весь production равен pre-step снимку по токенам. Единственный caller/site :3855/:3870; outgoing=0 по fresh MCP. [по коду и clangd]
- `OtdWall_Add_One` (:1723) уже короткий инициализатор, не дробился. 11 сценариев `TestOtdWallAddOne` (:606): длина 0/ниже/ровно/выше min_dim, диагональ, обратное ребро, нулевая/отрицательная высота; сохранность входа, чужих полей и отсутствие мутации при отказе. При достаточной длине высота копируется без проверки знака, length/width/material/favorite/openings не перезаписываются. Это существующий контракт, не повод добавить в рефакторинге валидацию. 60/0 до/после. [по коду и исполненным AC25 тестам]
- `TestOtdWallDelimOne` (:227) дополнен восемью сценариями пересечения стены для width=0 и width=1: вложенность, подрезка сверху/снизу, касание, малый запрос и малое пересечение; проверены отметки/высоты и условное переназначение length. 246/0 до/после; DelimAll (:424) 197/0. Реестр 73/73/73, TestKit 73/3184/0, EXIT 0 (08:09:51/08:13:04), отчёты свежее соответствующих APX. [по штатным отчётам исполненных VS запусков]
- BuildAddOn AC25 Windows Debug success до/после; fresh Roombook прежние 8 unused-variable + 5 unused-includes, TestCore 0 errors/3 прежних warnings. VS p4e_vs_candidate1: 1778 новых элементов, missing/extra=0 по box/доступным свойствам, все исходные GUID сохранены. VS pane сохраняет 1196 ERROR IN TEST/paramTo.isValid вне TestKit; TestKit-сводки в pane не найдены, их источником служат свежие штатные отчёты. Полные контуры/материалы и отсутствие production-диагностик not verified; performance A/B для локального извлечения не проводился. [по инструментам]
- Symbols Roombook/TestCore/TestFunc пересобраны из свежих MCP-деревьев, 130/130 имён и namespace/function ends сверены с диском; у полей сохранён формат Class.field. Пять локальных helper-границ callgraph актуальны; остальная унаследованная часть графа не освежалась. [по clangd и joined audit]

## P2b2f/P4f — выбор материала отдельно от состава (#260, 2026-10-04)

- `SelectWallFinishMaterial` (`Roombook.cpp:3887`) принимает изменяемые TypeOtd и выходной OtdMaterial плюс восемь const-ссылок на настройки. Не получает OtdWall/base_composite и не вводит дополнительные копии. Прежние switch, fallback на main/zone, type-переходы и TESTING-логирование перенесены целиком. `SetMaterialByType` (:3995) оставляет создание локального material, вызов выбора (:4005), SetMaterialFinish и присваивание в otdw.material в прежнем порядке. Весь production после inlining и type→otdw.type совпадает с pre-step снимком по токенам. Incoming=1/outgoing=0 по fresh MCP; отсутствие побочных эффектов не заявляется — TESTING DBprnt сохранён. [по коду и clangd]
- При совпадении rawname тип Up/Down/Reveal/Column становится Wall_Main, но выбранный материал заново не выбирается. Сравнение регистрозависимое (UniString.hpp:619, DevKit25); совпадение пустых rawname также схлопывает тип. При пустом secondary имя берётся из main, но решение о типе остаётся по rawname исходной настройки. Floor/Ceil при пустом имени переходят на zone без изменения типа; NoSet/Sloped выбирают zone без нормализации типа. [по коду и исполненным тестам]
- Последний слой structype=-1 заменяет только material/smaterial по length/pos, сохраняя rawname/rawname_bytype уже выбранной настройки и тип. Затем добавляется прежняя finish-запись; исходный слой и входные настройки сохранены. Это текущий контракт, не исправление поведения. [по коду и исполненным тестам]
- `TestSetMaterialByType` (`TestCore.cpp:675`): 11 TypeOtd × 6 режимов (полные настройки, пустые secondary, равные rawname, zone fallback, пустые rawname, другой регистр) × structural/finish last-layer. Проверены четыре поля материала, тип, все восемь входных настроек, геометрия стены, исходный и добавленный слои. 1848/0 до/после; TestKit 74/5032/0, EXIT 0 (11:00:47/11:04:22), registry 74/74/74. [по штатным отчётам исполненных VS запусков]
- BuildAddOn AC25 Windows Debug success до/после; fresh Roombook прежние 8 unused-variable + 5 unused-includes, TestCore 0 errors/3 warnings, registry 0 errors/2 warnings. VS p4f_vs_candidate1:1778 новых элементов, missing/extra=0 по box/доступным свойствам, все исходные GUID сохранены. VS pane содержит 1195 ERROR IN TEST/paramTo.isValid; TestKit-сводки в pane не найдены, числа взяты из свежих штатных отчётов. Полные контуры/материалы, отсутствие production-ошибок и performance A/B not verified. [по инструментам]
- Fresh MCP symbols Roombook/TestCore/TestFunc:132/132 имён и namespace/function ends сверены с диском; шесть локальных helper-границ callgraph актуальны. Унаследованная часть графа и остальные symbol records не освежались. LightRAG-запрос UniString не дал точного контракта; const/CaseSensitive проверены по установленному DevKit25 UniString.hpp. [по инструментам и заголовку]

## P2b2g/P4g — общий инициализатор откосов (#260, 2026-10-04)

- `InitializeOpeningRevealWall` (`Roombook.cpp:3490`) переносит прежние 13 присваиваний в трёх ветвях `OpeningReveals_Create_One` (:3514). Получает const-источник стены, выходной OtdWall и уже рассчитанные height/zBottom/width/length/концы/тип отрисовки. Никаких новых расчётов, проверок или копий material/base_composite. Обратный порядок правого/верхнего откосов задаётся caller. Guard, арифметика, has_reveal и три Delim_All сохранены; после inlining трёх вызовов весь production совпадает с pre-step снимком. Caller один, sites :3564/:3592/:3614, outgoing=0 по fresh MCP. [по коду и clangd]
- `TestOpeningRevealsCreateOne` (`TestCore.cpp:795`): 13 сценариев × два начальных has_reveal — обычный проём, подрезка сверху/снизу, точный верх/касание, глубина ниже/ровно min_dim, нулевая стена/ширина/перпендикуляр, обратная и вертикальная стена. Проверены координаты и порядок, размеры, base_guid/floorInd/base_type/draw_type/type, material/composite, сохранение входных данных и незатрагиваемого массива slab. 662/0 до/после; TestKit 75/5694/0, EXIT 0, registry75/75/75. Оба отчёта свежее соответствующих APX; запуск через VS MCP. [по коду и исполнению]
- Это характеризация текущего поведения: точное совпадение верхов ещё допускает верхнюю балку, превышение верха — нет; нулевая ширина оставляет два боковых откоса; нулевой perpendicular — только верхнюю балку. При ненулевом width подрезка пере назначает length на height, включая верхний откос. Толщина и классификация не исправлялись. [по коду и тестам]
- AC25 Windows Debug BuildAddOn success до/после; fresh Roombook диагностики идентичны P4f по severity/code/message (8 unused-variable + 5 unused-includes), TestCore0 errors/3 warnings, registry0 errors/2 warnings. VS p4g_vs_candidate1:1778 новых элементов, missing/extra=0 по box/доступным свойствам, исходные GUID сохранены. Pane1195 ERROR IN TEST/paramTo.isValid, suite-маркер отсутствует; verdict из свежего штатного отчёта. Полные контуры/материалы и отсутствие production-диагностик not verified; A/B производительности не проводился. [по инструментам]
- Scoped symbols134/134 (Roombook119/TestCore8/registry7), namespace и function ends сверены с диском. Все семь локальных helper-границ заново собраны fresh MCP и joined audit, project_edges425; чужие записи и унаследованные edges сохранены. Ранний guard отверг валидное однострочное тело функции; guard уточнён до записи. Сдвиг старого callgraph не прошёл join — заменён прямым MCP-сбором, не подбором координат. При отказе write_file из-за paginated-read использован exact-content patch. Build-процесс первоначально попал в Python без requests; успешные сборки выполнены проверенным Hermes venv без установки пакетов. [по инструментам]

## Зависимости
- `CommonFunction.hpp`, `Helpers.hpp`, `Propertycache.hpp` [по include]

## Инварианты и подводные камни
- Утечка memo при ошибке GetMemo (:2028-2034) — **исправлена** (FIX 2026-09-12) [из комментария + проверено]
- AGENTS.md §16: recreation of finish elements — out of scope [из AGENTS.md]
- Глобальные переменные namespace (`min_dim` в ревью-заметках) — thread-safety ограничение [из ревью; не проверено в этой сессии]
- `reducededges` (:28) — адрес `RoomEdges` для callback `RoomRedProc`/`RoomReductionPolyProc`. Присваивается в `Edges_GetFromRoom` (:3398) перед вызовом SDK и обнуляется сразу после возврата (:3410); адрес локального `rdges` не переживает выход из функции. Владение глобалом соответствует контракту SDK: `RoomReductionPolyProc` — синхронный callback (`APIdefs_Callback.h`, AC25–29), пример DevKit использует тот же паттерн «указатель → вызов → чтение». Повторный вход в `Edges_GetFromRoom` до обнуления невозможен: вызов синхронный. [по коду + DevKit AC25–29 + LightRAG]
- Подсчёт пробелов/разделителей для выравнивания текста (`Roombook.cpp:1030`, `:1040`, `:1251`) с 2026-09-28 (#221) идёт через `CommonFunction::DoubleToInt32` — при выходе за диапазон Int32 подставляется граница и выводится сообщение `msg_rep`. [по коду]
## Версии
- AC27 (`#170`): материалы сегментов балки — `API_OverriddenAttribute` (`overridden` + `attributeIndex`) заменён на `APIOptional<API_AttributeIndex>` (`hasValue` + `value`, индекс через `ACAPI_CreateAttributeIndex`). [по коду DevKit + сборка AC27]
- AC27: `ACAPI_ElementGroup_Create` → `ACAPI_ElementSet_Create`; в новом API нет параметра родительской группы, поэтому аддон передаёт только список элементов и выходной GUID. [по коду DevKit]
