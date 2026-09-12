# Инструкции для GLM 5.3 Flash — правки SomeStuff addon (AC25, ревью 2026-09-12b, верифицировано)

Общие ограничения (действуют для всех правок):
- Минимальная точечная правка; не менять публичные API и сигнатуры, используемые другими файлами.
- Не удалять существующую логику без явного основания, указанного в инструкции.
- Комментарии к изменениям — на русском языке.
- После правки файла: clang-format -i только по этому файлу.
- Целевая платформа Archicad 25: активные ветки — #else (не ServerMainVers_2700/2800).
- Каждая правка помечена комментарием «// FIX (ревью 2026-09-12): <краткая причина>».

Сортировка: critical BUG → major BUG → major PERF → minor BUG → minor PERF.

---

## CRITICAL BUG

### 1 ✅ ИСПРАВЛЕНО (2026-09-12b, clang-format применён, БИЛД ЕЩЁ НЕ ПРОВЕРЕН)
Файл: Sources/AddOn/Helpers.cpp
Строка/функция: ParamHelpers::WriteGDL, ~5199-5214
Что сделать: В блоке `if (err != NoError)` после вызова `ACAPI_LibraryPart_CloseParameters ()` сделать `return` безусловным — убрать условие, при котором return выполняется только при ошибке закрытия.
Почему: При ошибке GetActParameters err перезаписывается NoError, addParNum вычисляется по невалидному params-хендлу, цикл читает мусор.
Ограничения: Не трогать успешный путь; msg_rep перед return сохранить.

### 2 ✅ ИСПРАВЛЕНО (2026-09-12b, clang-format применён, БИЛД ЕЩЁ НЕ ПРОВЕРЕН)
Файл: Sources/AddOn/Roombook.cpp
Строка/функция: Floor_Create_One, 2089
Что сделать: Заменить `API_ElementMemo memo;` на `API_ElementMemo memo = {};`. Добавить `ACAPI_DisposeElemMemoHdls (&memo);` перед `continue` при err != NoError (~2095) и перед `continue` в ветке `polygon2DData.contourEnds == nullptr` (~2148, после FreePolygon2DData).
Почему: GetMemo с маской заполняет только polygon-поля; Dispose по неинициализированной структуре — краш; ранние выходы без Dispose — утечка.
Ограничения: BNZero-инициализацию не убирать; Dispose добавлять только на перечисленных путях выхода.

## MAJOR BUG

### 3 ✅ ИСПРАВЛЕНО (2026-09-12b, clang-format применён, БИЛД ЕЩЁ НЕ ПРОВЕРЕН)
Файл: Sources/AddOn/Roombook.cpp
Строка/функция: OtdWall_Create_FromColumn, ~1831-1836
Что сделать: Перед `return` в ветке `err != NoError || segmentmemo.columnSegments == nullptr` добавить `ACAPI_DisposeElemMemoHdls (&segmentmemo);`.
Почему: Утечка memo при частичной аллокации до ошибки.
Ограничения: Только эта ветка; успешный путь не трогать.

### 4 ✅ ИСПРАВЛЕНО (2026-09-12b, clang-format применён, БИЛД ЕЩЁ НЕ ПРОВЕРЕН)
Файл: Sources/AddOn/Roombook.cpp
Строка/функция: Floor_Draw_Slab, ~4753-4757
Что сделать: Перед `return` при err != NoError от ACAPI_Element_GetMemo добавить `ACAPI_DisposeElemMemoHdls (&memo);`.
Почему: Утечка memo при ошибке GetMemo.
Ограничения: Только ошибочная ветка.

### 5 ✅ ИСПРАВЛЕНО (2026-09-12b, clang-format применён, БИЛД ЕЩЁ НЕ ПРОВЕРЕН)
Файл: Sources/AddOn/Roombook.cpp
Строка/функция: RoomBook, ~66
Что сделать: `API_SelectionInfo selectionInfo;` → `API_SelectionInfo selectionInfo = {};`.
Почему: При ошибке ACAPI_Selection_Get BMKillHandle получает мусорный marquee.coords.
Ограничения: Только инициализация, логику не менять.

### 6 ✅ ИСПРАВЛЕНО (2026-09-12b, clang-format применён, БИЛД ЕЩЁ НЕ ПРОВЕРЕН)
Файл: Sources/AddOn/CommonFunction.cpp
Строка/функция: GetSelectedElements2, ~782
Что сделать: `API_SelectionInfo selectionInfo;` → `API_SelectionInfo selectionInfo = {};`.
Почему: Та же ловушка неинициализированной структуры перед BMKillHandle.
Ограничения: Только инициализация.

### 7 ✅ ИСПРАВЛЕНО (2026-09-12b, clang-format применён, БИЛД ЕЩЁ НЕ ПРОВЕРЕН)
Файл: Sources/AddOn/Helpers.cpp
Строка/функция: ParamHelpers::ReadListData, ~6721
Что сделать: Перед ранним `return` при `!needListData` добавить `BMKillHandle ((GSHandle *)&descRefs);`.
Почему: Хэндл дескрипторов от ACAPI_Element_GetDescriptors утекает на раннем выходе.
Ограничения: Убить хэндл до return, не менять условие ветвления.

### 8 ✅ ИСПРАВЛЕНО (2026-09-12b, clang-format применён, БИЛД ЕЩЁ НЕ ПРОВЕРЕН)
Файл: Sources/AddOn/Helpers.cpp
Строка/функция: GetRelationsElement, case API_WallID, ~973-1016
Что сделать: Добавить `break;` после закрывающей скобки `if (syncSettings.GetWidoS () && addConnect) { ... }` перед `case API_RailingID:`.
Почему: Fall-through вызывает GetRElementsForRailing с GUID стены при выключенной синхронизации окон.
Ограничения: Только break; тело if не менять.

### 9 ✅ ИСПРАВЛЕНО (2026-09-12b, clang-format применён, БИЛД ЕЩЁ НЕ ПРОВЕРЕН)
Файл: Sources/AddOn/Helpers.cpp
Строка/функция: ParamHelpers::ReadCoords, ~2610-2613
Что сделать: Перед `double koeff = x / l_wall;` добавить проверку `if (is_equal (l_wall, 0.0))` и в ней пропустить вычисление координат середины проёма (continue/аналог по структуре цикла).
Почему: Деление на нулевую длину стены даёт inf/NaN, которые записываются в словарь параметров.
Ограничения: Только вычисление середины проёма; остальные параметры не трогать.

### 10 ✅ ИСПРАВЛЕНО (2026-09-12b, clang-format применён, БИЛД ЕЩЁ НЕ ПРОВЕРЕН)
Файл: Sources/AddOn/Helpers.cpp
Строка/функция: GetParentGUIDSectElem, ~768-782
Что сделать: Первой строкой функции присвоить `parentguid = APINULLGuid; parentType = API_ZombieElemID;`.
Почему: При err != NoError выходные параметры не заполняются — вызывающий читает мусор.
Ограничения: Успешный путь (else-ветка) не меняется.

### 11 ✅ ИСПРАВЛЕНО (2026-09-12b, clang-format применён, БИЛД ЕЩЁ НЕ ПРОВЕРЕН)
Файл: Sources/AddOn/Helpers.cpp
Строка/функция: ParamHelpers::ReadCoords, ~2543
Что сделать: В вызове CoordCorrectAngle для symb_rotangle_axis_fraction/correct/correct_1000 передать `axisRotationAngle` вместо `slantDirectionAngle`.
Почему: «Осовые» параметры сейчас дублируют slant-значения.
Ограничения: Только аргумент этого вызова; slant-блок не трогать.

### 12 ✅ ИСПРАВЛЕНО (2026-09-12b, clang-format применён, БИЛД ЕЩЁ НЕ ПРОВЕРЕН)
Файл: Sources/AddOn/CommonFunction.cpp
Строка/функция: ReplaceCR, ~1052-1064
Что сделать: В цикле замены заменить пару `ReplaceFirst (p, EMPTYSTRING)` + `SetChar (inx, CharCR)` на: `val.SetChar (inx, CharCR);` затем `val.Delete (inx + 1, 1);` (перезаписать 'n' символом CR, затем удалить '\\').
Почему: Текущий код удаляет оба символа «\\n» и затирает следующий символ данных — каждый перенос съедает один символ.
Ограничения: Ветку `clear` (ReplaceAll на пустую) не трогать; итоговое поведение «\\n» → CR сохранить.

### 13 ✅ ИСПРАВЛЕНО (2026-09-12b, clang-format применён, БИЛД ЕЩЁ НЕ ПРОВЕРЕН)
Файл: Sources/AddOn/CommonFunction.cpp
Строка/функция: ElemHead_To_Neig, ~1887-1908
Что сделать: До проверки на Zombie инициализировать typeID из заголовка: в ветке ServerMainVers_2600 — `typeID = elemHead->type.typeID;`, в ветке #else — `typeID = elemHeadNonConst->typeID;`. Zombie-ветку (дозаполнение через ACAPI_Element_GetHeader) сохранить без изменений.
Почему: При валидном типе на входе typeID остаётся API_ZombieElemID и функция всегда возвращает false.
Ограничения: Поведение для незаполненных (zombie) заголовков не должно измениться.

### 14.5-примечание: правка ConvertPolygon2DToAPIPolygon (+1 смещение дуг) — ПОДТВЕРЖДЕНО ИСХОДНИКОМ DevKit (Polygon2DDataConv.h: arcs записываются с индекса 1). Применено.
### 14 ✅ ИСПРАВЛЕНО (2026-09-12b, clang-format применён, БИЛД ЕЩЁ НЕ ПРОВЕРЕН)
Файл: Sources/AddOn/CommonFunction.cpp
Строка/функция: ConstructPolygon2DFromElementMemo, ~2261/2280
Что сделать: Перед вычислением nVertices/nContours добавить: `if (memo.coords == nullptr || BMGetHandleSize ((GSHandle) memo.coords) < (GSSize) sizeof (API_Coord)) return APIERR_BADPARS;` и аналогичную проверку для memo.pends (sizeof (Int32)).
Почему: BMGetHandleSize на nullptr-хендле и вычитание 1 из нулевого размера → wraparound беззнакового счётчика.
Ограничения: Возвращаемый тип/код ошибки согласовать с сигнатурой функции; успешный путь не менять.

### 15 ✅ ИСПРАВЛЕНО (2026-09-12b, clang-format применён, БИЛД ЕЩЁ НЕ ПРОВЕРЕН)
Файл: Sources/AddOn/Propertycache.cpp
Строка/функция: ParamHelpers::GetGroupFromCache, ~160-166
Что сделать: После успешного `ACAPI_Property_GetPropertyGroup (group)` добавить `cache.propertygroups.Put (group.guid, group);` (до финального GetPtr).
Почему: Прочитанная группа не кладётся в кэш — функция возвращает false при успехе и повторяет API-запрос.
Ограничения: Условия isGroupPropertyRead/isGroupProperty_OK не менять.

### 16 ✅ ИСПРАВЛЕНО (2026-09-12b, clang-format применён, БИЛД ЕЩЁ НЕ ПРОВЕРЕН)
Файл: Sources/AddOn/spec/Spec.cpp
Строка/функция: SpecArray, 703 и 705
Что сделать: Заменить `exsist_elements[i]` на `exsist_element` в обеих строках.
Почему: Range-for переменная игнорируется; посторонний индекс i даёт out-of-bounds и дубли.
Ограничения: Переменную i (фазы прогресса) не трогать.

### 17 ✅ ИСПРАВЛЕНО (2026-09-12b, clang-format применён, БИЛД ЕЩЁ НЕ ПРОВЕРЕН)
Файл: Sources/AddOn/spec/Spec_libpart.cpp
Строка/функция: AddMat (~110), AddArm (~149), AddSubpos (~204), AddProkat (~244)
Что сделать: Поднять guards до maxIndex+1 по фактическим индексам в теле каждой функции: AddMat `<10` → `<11` (читается [10]); AddArm `<8` → `<10` (читается [9]); AddSubpos `<9` → `<10`; AddProkat `<14` → `<15`. Перед правкой сверить фактический максимальный индекс partstring в каждой функции.
Почему: При размере ровно N обращение к partstring[N] — выход за границы GS::Array.
Ограничения: Только константа в guard; логику разбора не менять.

### 18 ✅ ИСПРАВЛЕНО (2026-09-12b, clang-format применён, БИЛД ЕЩЁ НЕ ПРОВЕРЕН)
Файл: Sources/AddOn/spec/Spec_libpart.cpp
Строка/функция: AddMat, ~115
Что сделать: Убрать локальное объявление — `GS::UniString subpos = partstring[0];` заменить на `subpos = partstring[0];`.
Почему: Локальная subpos затеняет внешнюю; GetSubposKey всегда получает пустой ключ — материалы v3 смешиваются.
Ограничения: Внешнюю переменную subpos не переименовывать.

### 19 ✅ ИСПРАВЛЕНО (2026-09-12b, clang-format применён, БИЛД ЕЩЁ НЕ ПРОВЕРЕН)
Файл: Sources/AddOn/pk/ResetProperty.cpp
Строка/функция: ResetPropertyElement2Defult, ~88 (ветка #else, AC25)
Что сделать: До первого переключения БД добавить сохранение исходной: `API_DatabaseInfo origDB = {}; ACAPI_Database (APIDb_GetCurrentDatabaseID, &origDB, nullptr);`. Восстановление выполнять `ACAPI_Database (APIDb_ChangeCurrentDatabaseID, &origDB, nullptr);`. Передачу `&commandID` (API_DatabaseID*) в APIDb_ChangeCurrentDatabaseID убрать.
Почему: APIDb_ChangeCurrentDatabaseID принимает API_DatabaseInfo* (DevKit-25 APIdefs_Database.h:53); передача перечисления — чтение мусора, возврат в исходную БД никогда не выполняется.
Ограничения: Остальную логику обхода БД не менять; проверить, что восстановление комбинации слоёв после этого выполняется на корректном err.

### 20 ✅ ИСПРАВЛЕНО (2026-09-12b, clang-format применён, БИЛД ЕЩЁ НЕ ПРОВЕРЕН)
Файл: Sources/AddOn/pk/Revision.cpp
Строка/функция: GetAllChangesMarker, ~233
Что сделать: `API_LayoutInfo layoutInfo;` → `API_LayoutInfo layoutInfo = {};` и после ACAPI_Navigator_GetLayoutSets добавить `if (layoutInfo.customData == nullptr) continue;`.
Почему: Макет без customData даёт разыменование nullptr при ContainsKey/Put.
Ограничения: Копировать паттерн ChangeLayoutProperty:398-401; остальное не трогать.

### 21 ✅ ИСПРАВЛЕНО (2026-09-12b, clang-format применён, БИЛД ЕЩЁ НЕ ПРОВЕРЕН)
Файл: Sources/AddOn/pk/Revision.cpp
Строка/функция: GetMarkerPos, ~871-872
Что сделать: После GetMemo добавить: `if (memo.pends == nullptr || memo.coords == nullptr || BMGetHandleSize ((GSHandle) memo.coords) / (GSSize) sizeof (API_Coord) <= (GSSize) (*memo.pends)[0] + 1) { ACAPI_DisposeElemMemoHdls (&memo); return false; }`.
Почему: Разыменование nullptr-хендлов и доступ по begInd без проверки размера — краш на неполном полигоне.
Ограничения: Успешный путь не менять; Dispose добавить в новую ветку обязательно.

### 22 ✅ ИСПРАВЛЕНО (2026-09-12b, clang-format применён, БИЛД ЕЩЁ НЕ ПРОВЕРЕН)
Файл: Sources/AddOn/pk/Revision.cpp
Строка/функция: ChangeMarkerText, 1012 и 1020
Что сделать: Оба GS::ucscpy в value.uStr заменить на копирование с обрезкой до API_UAddParStrLen (256) по образцу Spec.cpp:2525-2530.
Почему: uStr — буфер фиксированной длины; ucscpy без обрезки переполняет его.
Ограничения: Использовать точно паттерн Spec.cpp (GS::Min с API_UAddParStrLen).

### 23 ✅ ИСПРАВЛЕНО (2026-09-12b, clang-format применён, БИЛД ЕЩЁ НЕ ПРОВЕРЕН)
Файл: Sources/AddOn/pk/AutomateFunction.cpp
Строка/функция: AlignOneDrawingsByPoints, 763-764 и 791-792
Что сделать: Перед обоими ранними `return APIERR_GENERAL` восстановить исходную БД (и окно), скопировав блок восстановления из конца функции (~813-829).
Почему: После смены БД ранний выход оставляет ArchiCAD в БД чертежа — следующие чертежи обрабатываются в чужой БД.
Ограничения: Блок восстановления копировать как есть; финальный блок не удалять.

### 24 ✅ ИСПРАВЛЕНО (2026-09-12b, clang-format применён, БИЛД ЕЩЁ НЕ ПРОВЕРЕН)
Файл: Sources/AddOn/pk/AutomateFunction.cpp
Строка/функция: GetSectLine, ~326
Что сделать: Перед `id = *memo.elemInfoString;` добавить `if (memo.elemInfoString == nullptr) { ACAPI_DisposeElemMemoHdls (&memo); return APIERR_GENERAL; }`.
Почему: elemInfoString — указатель GS::UniString*, для части элементов nullptr → краш.
Ограничения: Тип возвращаемой ошибки согласовать с сигнатурой; Dispose в новой ветке обязателен.

### 25
Файл: Sources/AddOn/dialogs/BrowserPalette.cpp
Строка/функция: HighlightElements, 485-516
Что сделать: Сброс `suppressSelectionRefresh = false` сделать exception-safe: ввести локальную RAII-структуру с деструктором, сбрасывающим флаг, сразу после установки true (или try/catch с сбросом в catch).
Почему: Любое исключение в теле оставляет флаг true навсегда — палитра перестаёт реагировать на выделение.
Ограничения: Логику подсветки/зума не менять; флаг сбрасывать ровно один раз на выходе.

### 26
Файл: Sources/AddOn/dialogs/BrowserPalette.cpp
Строка/функция: RegisterACAPIJavaScriptObject, ~1188
Что сделать: Перед `browser.RegisterAsynchJSObject (jsACAPI)` добавить `browser.UnregisterJSObject ("ACAPI");`; результат RegisterAsynchJSObject (bool) сохранить и при false вывести DBprnt с текстом ошибки регистрации.
Почему: Повторная регистрация на каждой загрузке страницы без снятия старой; не проверенный bool молчит при отказе.
Ограничения: Порядок «unregister → new JSObject → register» не менять.

### 27 ✅ ИСПРАВЛЕНО (2026-09-12b, clang-format применён, БИЛД ЕЩЁ НЕ ПРОВЕРЕН)
Файл: Sources/AddOn/Summ.cpp
Строка/функция: SumSelected, ~57-79
Что сделать: Результат ACAPI_CallUndoableCommand сохранить в переменную; при err != NoError — msg_rep и выход из функции до ParamHelpers::WriteInfo и SyncArray.
Почему: При откате команды пост-шаги и отчёт выполняются как при успешной записи.
Ограничения: Тело лямбды не менять.

### 28 ⚠️ ЧАСТИЧНО: Summ.cpp и Spec.cpp — ИСПРАВЛЕНО (clang-format применён); Roombook.cpp (4121 после CallUndoableCommand и 5286 SetSyncOtdWall — suspend вынести за цикл) — НЕ СДЕЛАНО. БИЛД ЕЩЁ НЕ ПРОВЕРЕН (SuspendGroups, 4 файла одним паттерном)
Файлы/строки:
- Sources/AddOn/Summ.cpp, ~61-66 (внутри лямбды, после ElementsWrite)
- Sources/AddOn/spec/Spec.cpp, ~951-956 (в конце лямбды, после ACAPI_Element_Delete)
- Sources/AddOn/Roombook.cpp, ~4121-4125 (после завершения CallUndoableCommand)
- Sources/AddOn/Roombook.cpp, ~5286-5290 (SetSyncOtdWall: suspend/restore вынести за цикл — один suspend до цикла, один restore после)
Что сделать: Добавить восстановление тумблера по образцу Sync.cpp:265-277: `if (!suspGrp) { bool suspNow = false; if (ACAPI_Environment (APIEnv_IsSuspendGroupOnID, &suspNow, nullptr) == NoError && suspNow) ACAPI_Element_Tool (<тот же массив>, APITool_SuspendGroups, nullptr); }` (в ветках ServerMainVers_2700 — ACAPI_View_IsSuspendGroupOn / ACAPI_Grouping_Tool).
Почему: APITool_SuspendGroups — тумблер (подтверждено DevKit/LightRAG); без восстановления группировка остаётся выключенной глобально после команды.
Ограничения: Восстанавливать только если suspend включали сами (флаг suspGrp==false на входе); в Roombook SetSyncOtdWall не допускать повторного toggle в каждой итерации цикла.

### 29
Файл: Sources/AddOn/MEPv1.cpp (только сборки AC27+; для AC25 файл пуст)
Строка/функция: GetMEPData, ~360
Что сделать: В ветке «systems.IsOk() == true, но результат пуст» убрать вызов systems.UnwrapErr() — заменить сообщение на ACAPI_WriteReport ("No physical system found", false). UnwrapErr оставить только в ветке IsErr.
Почему: UnwrapErr на успешном Result — UB (чтение мусора из union).
Ограничения: Ветку IsErr не менять.

## MAJOR PERF

### 30 ✅ ИСПРАВЛЕНО (2026-09-12b, clang-format применён, БИЛД ЕЩЁ НЕ ПРОВЕРЕН)
Файл: Sources/AddOn/SomeStuff_Main.cpp
Строка/функция: MenuCommandHandler, ~436
Что сделать: DimRoundAll вызывать только для команд, изменяющих элементы/свойства (SyncAll, SyncSelect, ReNum, Sum, Spec, RoomBook, SetRevision, RunParam, ShowSub, SetSub); для переключателей флагов (wallS/widoS/objS/cwallS, MonAll) и Pallete — не вызывать.
Почему: Полный пересчёт всех размеров проекта на каждую команду меню — O(N) скан без необходимости.
Ограничения: Сам вызов DimRoundAll не менять; только условие его вызова (перенести внутрь switch по itemIndex).

### 31 ✅ ИСПРАВЛЕНО (2026-09-12b, clang-format применён, БИЛД ЕЩЁ НЕ ПРОВЕРЕН)
Файл: Sources/AddOn/SomeStuff_Main.cpp
Строка/функция: ElementEventHandlerProc, ~140-144 (APINotifyElement_EndEvents)
Что сделать: Полный DimRoundAll в EndEvents выполнять только при `PROPERTYCACHE ().hasDimAutotext` (адресная обработка отдельных размеров уже выполняется через DimAutoRoundOne выше по обработчику).
Почему: EndEvents приходит пачками на каждое редактирование — полный скан всех размеров на каждую пачку.
Ограничения: Условие-фильтр только по hasDimAutotext; вызов не удалять полностью.

### 32
Файл: Sources/AddOn/Dimensions.cpp
Строка/функция: DimParse, ~257
Что сделать: Убрать копирование `ParamDictValue pdictvalue = dimrule.paramDict;` на каждый вызов — передавать значение measuredvalue отдельным параметром DimParse (внутренняя сигнатура) и подставлять его при вычислении выражения.
Почему: Полная копия HashTable<UniString, ParamValue> на каждый размер × каждое правило; копия нужна только ради одного значения.
Ограничения: Сигнатура DimParse — внутренняя для файла (не публичный API заголовков); поведение выражений не менять.

### 33 ✅ ИСПРАВЛЕНО (2026-09-12b, clang-format применён, БИЛД ЕЩЁ НЕ ПРОВЕРЕН)
Файл: Sources/AddOn/Summ.cpp
Строка/функция: Sum_GetElement, ~252-259
Что сделать: Вынести вызов ACAPI_Element_Filter из цикла по правилам — вычислить результат фильтрации один раз на элемент (до цикла по rule_definitions) и переиспользовать.
Почему: O(правила × элементы) дорогих ACAPI-вызовов; результат фильтра не зависит от правила.
Ограничения: Семантику фильтрации (те же флаги) сохранить.

### 34
Файл: Sources/AddOn/Roombook.cpp
Строка/функция: RoomBook, ~567-596
Что сделать: Копии `UnicGuidByBase byzone = cIt->value;`, `UnicGuidByTypeOtd bytype = ...`, `UnicGuid byparent = ...` заменить на const-ссылки (обе ветки #ifdef, где есть).
Почему: Глубокое копирование трёхуровневых словарей на каждой итерации при только чтении.
Ограничения: Если какая-то копия далее мутируется — её не трогать (проверить тело цикла).

### 35
Файл: Sources/AddOn/Roombook.cpp
Строка/функция: OtdWall_Delim_One, сигнатура ~3839
Что сделать: Параметр `OtdWall otdn` заменить на `const OtdWall &otdn`; внутри создать локальную копию только изменяемых полей; финальную вставку заменить на `opw.Push (std::move (localCopy))` вместо PushNew(otdn).
Почему: Копирование всей структуры (с base_composite и openings) на каждый вызов + вторая копия при PushNew.
Ограничения: Перед правкой убедиться, какие поля функции мутируются — копировать только их; результат на выходе должен быть идентичен.

### 36
Файл: Sources/AddOn/Roombook.cpp + Roombook.hpp
Строка/функция: Param_SetToRooms (hpp ~346), Param_SetToWindows (hpp ~365)
Что сделать: Параметр ReadParams передавать по неконстантной ссылке; в вызывающем коде (Roombook.cpp ~288 и ~378) подготовить одну рабочую копию словаря до цикла по зонам/проёмам и переиспользовать её (копия нужна: Param_Property_Read мутирует isValid/val).
Почему: Полная копия словаря параметров на каждую зону и каждый проём.
Ограничения: Изоляция между зонами обязательна (мутации не должны протекать в исходный словарь правил); поведение сохранить.

### 37
Файл: Sources/AddOn/dialogs/BrowserPalette.cpp
Строка/функция: GetPropertiesList, ~253-265
Что сделать: Имя группы свойства получать через ParamHelpers::GetGroupFromCache (кэш PROPERTYCACHE) вместо вызова ACAPI_Property_GetPropertyGroup на каждое свойство каждого элемента.
Почему: O(N·M) API-вызовов на каждый рефреш палитры; кэш групп уже существует.
Ограничения: Fallback «Без группы» при промахе кэша сохранить; формат JSON не менять.

### 38
Файл: Sources/AddOn/dialogs/BrowserPalette.cpp
Строка/функция: FilterElementsByType, ~1234-1249
Что сделать: Заменить ACAPI_Element_Get на ACAPI_Element_GetHeader: `API_Elem_Head head = {}; head.guid = guid; if (ACAPI_Element_GetHeader (&head) != NoError) continue; if (!ElementCanHaveProperty (head.typeID)) continue;` (в ветке ServerMainVers_2600 — head.type.typeID).
Почему: Для проверки типа достаточно заголовка; полный Get читает весь элемент на каждый GUID выделения в каждой мост-функции.
Ограничения: Semантику фильтра (те же типы отсекаются) сохранить.

## MINOR BUG

### 39
Файл: Sources/AddOn/Helpers.cpp
Строка/функция: ReadCoords, 2566 и 2574
Что сделать: В двух вызовах AddDoubleValueToParamDictValue заменить пятый аргумент-литерал `true` на `0.0`.
Почему: Логический литерал в double-параметре записывает 1.0 вместо 0.
Ограничения: Только эти два вызова.

### 40
Файлы: Sources/AddOn/Helpers.cpp (~823, ~865), Sources/AddOn/CommonFunction.cpp (~856)
Что сделать: Вычисление длительности заменить на `(double)(time_end - time_start) / CLOCKS_PER_SEC` и подпись вывода «s» (в CommonFunction — согласовать с «ms»: `(t2-t1) * 1000.0 / CLOCKS_PER_SEC`).
Почему: clock() возвращает тики CLOCKS_PER_SEC в секунду; деление на 1000 даёт неверные единицы в отчётах.
Ограничения: Только арифметика и подпись; формат вывода (%.3f/%d) поправить под тип.

### 41
Файл: Sources/AddOn/CommonFunction.cpp
Строка/функция: GDLHelpers::ParamToMemo, ~2608
Что сделать: Добавить `if (memo.params == nullptr) return false;` перед BMGetHandleSize; перед копированием массива в ветке arr_num проверить `pp->arr_num.GetSize () >= dim1 * dim2`.
Почему: Разыменование нулевого params-хендла и чтение за границей arr_num при несогласованных размерах.
Ограничения: Успешный путь не менять.

### 42
Файл: Sources/AddOn/CommonFunction.cpp
Строка/функция: GetElementByPropertyDescription, ~2589
Что сделать: `value.ToLowerCase ()` вычислить один раз до вложенных циклов в локальную переменную и сравнивать с ней.
Почему: Создание копии строки в самом внутреннем цикле по всем элементам.
Ограничения: Только подъём вычисления.

### 43
Файл: Sources/AddOn/Propertycache.cpp
Строка/функция: isCacheContainsParamValue, ~286-315
Что сделать: Убрать ранние `return false` при isXXX_OK == false — повторить цепочку источников в том же порядке, что и GetParamValueFromCache (GeoLocation → LocOrigin → PlaceSets и т.д.).
Почему: Функции дают противоречивые результаты для одного ключа: Contains=false при Get=true.
Ограничения: Список источников и порядок — точно как в GetParamValueFromCache; результат сравнить с ней на всех ветках.

### 44
Файл: Sources/AddOn/Propertycache.cpp
Строка/функция: ReadLibraryFile, ~41-56
Что сделать: Перенести вызов file.GetDataLength (&fSize) после успешного file.Open; проверить код возврата ReadBin и при ошибке `delete[] buff; return false;`.
Почему: GetDataLength на неоткрытом файле не гарантирован; при ошибке чтения парсится неинициализированный буфер.
Ограничения: Логику парсинга не менять.

### 45
Файл: Sources/AddOn/Propertycache.cpp
Строка/функция: GetAllAttributeToParamDict, ~672
Что сделать: Сохранить err от ACAPI_Attribute_GetAttributesByType; при err != NoError — msg_rep и return false.
Почему: Сейчас при ошибке возвращается true с пустым кэшем атрибутов.
Ограничения: Только обработка ошибки.

### 46
Файл: Sources/AddOn/ClassificationFunction.cpp
Строка/функция: GetAllClassification, ~48-88
Что сделать: После `continue` в обработке ошибки одной системы сбрасывать `err = NoError;` (локальную переменную цикла), чтобы ошибка последней системы не гасила весь кэш.
Почему: Одна сбойная система помечает весь кэш классификаций нечитанным.
Ограничения: msg_rep для отдельной системы сохранить.

### 47
Файл: Sources/AddOn/ClassificationFunction.cpp
Строка/функция: FindClass(const GS::Pair<API_Guid, API_Guid>), ~219
Что сделать: Первой строкой добавить `if (!ReadSystemDict ()) return {};` (по образцу перегрузки FindClass(systemname, classname)).
Почему: Перегрузка читает кэш без гарантии его загрузки — рассинхрон с остальными входами.
Ограничения: Остальное тело не менять.

### 48
Файл: Sources/AddOn/ClassificationFunction.cpp
Строка/функция: GetFullName, ~160
Что сделать: Перед рекурсивным вызовом добавить guard `if (parentname == itemname) break;` (выход из цикла/рекурсии по совпадению имени родителя с текущим).
Почему: При совпадении id родителя и потомка — бесконечная рекурсия и переполнение стека.
Ограничения: Guard безопасен при любых данных; основную логику не менять.

### 49
Файл: Sources/AddOn/ClassificationFunction.cpp
Строка/функция: AddClassificationItem, ~138-139
Что сделать: Три вызова `desc.ToLowerCase ().Contains (...)` заменить на `desc.Contains (...)` (desc уже приведён к нижнему регистру выше).
Почему: Три лишних копии строки на каждый элемент классификации.
Ограничения: Условие (те же три подстроки) не менять.

### 50
Файл: Sources/AddOn/spec/Spec.cpp
Строка/функция: SpecArray, ~754
Что сделать: `return false;` заменить на `return APIERR_CANCEL;`.
Почему: false в GSErrCode = NoError — отмена пользователя сообщается как успех.
Ограничения: Только эта строка.

### 51
Файл: Sources/AddOn/spec/Spec.cpp
Строка/функция: GetSizePlaceElement, ~2316-2318
Что сделать: Перед BMGetHandleSize добавить `if (memot.params == nullptr) return false;`.
Почему: Разыменование нулевого хэндла у объекта без GDL-параметров.
Ограничения: Только guard.

### 52
Файл: Sources/AddOn/pk/Revision.cpp
Строка/функция: GetChangesLayout, ~779
Что сделать: std::atoi заменить на std::stoi в try/catch (по образцу Sync.cpp) или UniStringToInt32 с проверкой; при ошибке разбора — msg_rep и использование 0 как сейчас.
Почему: atoi молча возвращает 0 — ошибка разбора неотличима от кода 0.
Ограничения: Поведение при корректном коде не менять.

### 53
Файл: Sources/AddOn/pk/Revision.cpp
Строка/функция: GetMarkerText ~899, ChangeMarkerText ~1004
Что сделать: Добавить `if (memo.params == nullptr) return false;` перед BMGetHandleSize.
Почему: Разыменование нулевого params-хендла.
Ограничения: Только guard; пути Dispose не трогать (они корректны).

### 54
Файл: Sources/AddOn/pk/Revision.cpp
Строка/функция: ~628-634 и ~947-951
Что сделать: `for (auto ch : changes)` заменить на `for (auto &ch : changes)` в обеих точках (обе версии веток).
Почему: Копирование пар; в ветке AC28+ модификации по ссылке на копию теряются.
Ограничения: Остальное тело цикла не менять.

### 55
Файл: Sources/AddOn/pk/AutomateFunction.cpp
Строка/функция: AlignDrawingsByPoints, ~914/935/944
Что сделать: Восстановление store-настроек/БД/окна (блок ~969-976) выполнять и перед ранними return — вынести в общий выход (флаг + единый return или дублирование блока в каждой ранней ветке).
Почему: Ранний выход оставляет сохранённые настройки вида активными.
Ограничения: Блок восстановления не дублировать в теле цикла — только на выходах.

### 56
Файл: Sources/AddOn/pk/AutomateFunction.cpp
Строка/функция: GetSectLine, ~331-343
Что сделать: Подсчёт вхождений точек p.Count(p[i]) в двойном цикле заменить на однократный проход с GS::HashTable<GS::UniString, UInt32> (ключ Printf координат, округлённых до 3 знаков).
Почему: O(n²) сравнений точек; на длинных морфах — секунды.
Ограничения: Итоговый набор lines должен быть идентичен (та же логика отбора).

### 57
Файл: Sources/AddOn/pk/ResetProperty.cpp
Строка/функция: ResetOneElemenDefault, ~381 (ветка ServerMainVers_2600)
Что сделать: Передать `properties_to_reset` вместо `properties` в ACAPI_Element_SetPropertiesOfDefaultElem.
Почему: AC26+-ветка перезаписывает все свойства, включая не подлежащие сбросу — рассинхрон с AC25-веткой.
Ограничения: AC25-ветку (#else) не трогать.

### 58
Файл: Sources/AddOn/pk/ResetProperty.cpp
Строка/функция: ResetPropertyElement2Defult, ~81
Что сделать: Удалить второй (дублирующий) вызов ResetElementsInDB с теми же аргументами (строка 81; первый — ~63).
Почему: Полный повторный обход всех БД фасадов без эффекта.
Ограничения: Удалить только дублирующий вызов; убедиться, что первый вызов остаётся.

### 59 ⚠️ ОТКЛОНЕНО при правке: вызовы ResetOneElemenDefault НЕ дубли — разные 3-и аргументы. Не удалять.
Файл: Sources/AddOn/pk/ResetProperty.cpp
Строка/функция: ResetElementsDefault, ~273/297/303
Что сделать: Оставить один из трёх идентичных вызовов ResetOneElemenDefault (API_ObjectID, definitions_to_reset, 0); два дубля удалить.
Почему: Тройное чтение/сброс дефолтных свойств одного инструмента.
Ограничения: Перед удалением сверить, что аргументы всех трёх вызовов действительно идентичны.

### 60
Файл: Sources/AddOn/Roombook.cpp
Строка/функция: Floor_Create_One, ~2138-2139
Что сделать: Удалить неиспользуемые локальные переменные dzBottom/dzUp (обе далее не читаются).
Почему: Мёртвый код с ошибочной формулой dzUp (копипаст dzBottom).
Ограничения: Убедиться grep'ом, что переменные действительно не читаются ниже.

### 61
Файл: Sources/AddOn/Roombook.cpp
Строка/функция: OtdWall_Draw_Object, ~4460-4486
Что сделать: Вести счётчик фактически добавленных в arr_num проёмов; после цикла присвоить `p.dim1 = счётчику` вместо edges.openings.GetSize().
Почему: Отфильтрованные continue-проёмы не попадают в массив — dim1*9 не равно фактическому размеру.
Ограничения: Порядок данных в arr_num не менять.

### 62
Файл: Sources/AddOn/Dimensions.cpp
Строка/функция: DimAutoRound ~141-148 + DimParse ~300-319
Что сделать: До цикла по правилам сохранить `GS::UniString originalContent = content;`; в DimParse сравнивать custom_txt с originalContent (а не с мутированным content); мутацию content первым правилом убрать (менять только memo).
Почему: Второе правило сравнивает с текстом, сгенерированным первым, а не с исходным пользовательским.
Ограничения: Итоговое поведение первого сработавшего правила сохранить.

### 63
Файл: Sources/AddOn/dialogs/BrowserPalette.cpp
Строка/функция: GetPropertiesList, ~331
Что сделать: Считать count по фактически добавленным в JSON элементам (счётчик в цикле) вместо selectedElements.GetSize().
Почему: Элементы с err/пустыми definitions пропускаются — count больше фактического числа в списке.
Ограничения: Поле count оставить (имя не менять — HTML его читает).

### 64
Файл: Sources/AddOn/dialogs/BrowserPalette.cpp
Строка/функция: GetClassification, ~963-968 и ~989-993
Что сделать: Инициализировать все Int32 (cnt, tot, count, total) нулём перед ObjectState::Get.
Почему: При отсутствующем ключе Get оставляет неинициализированное значение.
Ограничения: Только инициализация.

### 65
Файл: Sources/AddOn/dialogs/BrowserPalette.cpp
Строка/функция: UpdateSelectionInfoInUI, ~147-148
Что сделать: При пустом selectedElements выполнить ExecuteJS `refreshSelectionInfoText(0)` до раннего return (вынести формирование jsCall выше проверки пустоты; остальные js-вызовы при пустом выделении не делать).
Почему: При сбросе выделения счётчик UI не обновляется — остаётся устаревший.
Ограничения: Остальное поведение при пустом выделении не менять.

### 66
Файл: Sources/AddOn/dialogs/BrowserPalette.cpp
Строка/функция: GetPropertiesList, ~257-261
Что сделать: DBprnt с чтением group.name перенести после проверки `groupErr == NoError`.
Почему: При ошибке ACAPI_Property_GetPropertyGroup читается неинициализированная UniString (живой путь в TESTING-сборке).
Ограничения: Текст лога можно упростить (без group.name при ошибке).

### 67
Файл: Sources/AddOn/dialogs/BrowserPalette.cpp
Строка/функция: GetPropertyValue, ~438
Что сделать: Поле JSON «propertyName» переименовать в «propertyId» (значение не менять).
Почему: Поле содержит GUID, а не имя — вводит в заблуждение; HTML поле не читает, переименование безопасно.
Ограничения: После правки grep'ом проверить, что Interface_ru.html не использует старое имя.

### 68
Файл: Sources/AddOn/dialogs/BrowserPalette.cpp
Строка/функция: GetPropertyValue, ~435
Что сделать: `bool isCommon = (selectedElements.IsEmpty ()) || (uniqueValuesCount == 1 && totalCount == selectedElements.GetSize ());`
Почему: Для пустого выделения common=false, хотя семантика «общее значение» — true.
Ограничения: Проверить обработку в HTML (data.common) — поведение при непустом выделении не менять.

### 69
Файл: Sources/AddOn/dialogs/BrowserPalette.cpp
Строка/функция: GetClassification, ~778-784 и ~1017-1035
Что сделать: Ответы «нет выделения» и обоих catch привести к JSON-строке того же формата, что и нормальный путь (~972-1011): `{\"common\":true,\"commonPath\":[],\"differing\":[],\"options\":[],\"status\":\"error\",\"debug_error\":\"...\"}`.
Почему: Вложенные DG::JSArray в DG::JSObject не переживают CEF-мост — HTML получает undefined.
Ограничения: HTML-гарды (data.commonPath || []) оставить.

### 70
Файл: Sources/AddOn/dialogs/BrowserPalette.cpp
Строка/функция: все мост-лямбды без try/catch (GetPropertiesList, GetPropertyValue, GetSelectionInfo, HighlightElements, ParsePropertyDescription, ParsePropertyForElement, RefreshSelectionInfoUI)
Что сделать: Обернуть тело каждой лямбды в try/catch (const std::exception& + ...) с возвратом JSON-строки `{\"status\":\"error\",\"message\":\"...\"}` (для функций, возвращающих JSValue-строку) или `new DG::JSValue (false)` (для булевых) — по образцу GetClassification/SetClassification.
Почему: Исключение, пересекающее CEF-мост, роняет ArchiCAD.
Ограничения: Успешные пути не менять; в catch сбросить suppressSelectionRefresh, если лямбда — HighlightElements (см. правку 25).

### 71
Файл: Sources/AddOn/MEPv1.cpp (только AC27+)
Строка/функция: GetSubElement, ~240-242
Что сделать: После ACAPI_Element_GetHeader добавить `if (err != NoError) return;`.
Почему: При ошибке тип не заполнен, чтение elem_head.type.typeID — по невалидным данным.
Ограничения: Только guard.

### 72
Файл: Sources/AddOn/Dimensions.cpp
Строка/функция: DimAutoRound, ~54-71
Что сделать: При переборе dimrules как layer-regex пропускать ключи, уже добавленные в rules по числовому kstr (сравнение ключа с kstr или флаг).
Почему: Ключ-число может ложно совпасть как подстрока имени слоя — правило применяется дважды.
Ограничения: Логику самих правил не менять; только защита от дубля.

## MINOR PERF

### 73
Файл: Sources/AddOn/Roombook.cpp
Строка/функция: RoomBook, ~176-181
Что сделать: `zoneGuids = cIt->value` заменить на `const GS::Array<API_Guid> &zoneGuids = cIt->value;` (обе ветки #ifdef).
Почему: Копия массива на каждой итерации при только чтении.

### 74
Файл: Sources/AddOn/Roombook.cpp
Строка/функция: Floor_Create_One, ~2075/2088
Что сделать: В GuidByZ хранить API_Guid напрямую вместо std::string (убрать APIGuid2GSGuid().ToUniString().ToCStr(CC_Cyrillic) и обратный APIGuidFromString).
Почему: Двойная дорогая конвертация на каждый slab каждой зоны.
Ограничения: Сортировку заменить на сравнение GS::Guid или выполнять по другому ключу — итоговый порядок обработки сохранить.

### 75
Файл: Sources/AddOn/Roombook.cpp
Строка/функция: RoomBook ~521-527, OtdData_WriteToRoom ~1082-1084
Что сделать: Пары ContainsKey+Get заменить на `if (const auto *p = <table>.GetPtr (key)) { ... *p ... }`.
Почему: Двойной lookup хэш-таблицы в цикле по материалам.

### 76
Файл: Sources/AddOn/Helpers.cpp
Строка/функция: ParamHelpers::ReadIFC, ~5978-5982
Что сделать: `API_IFCProperty property = properties.Get (i);` → `const API_IFCProperty &property = properties[i];`; повторные properties.Get(i).head.* убрать.
Почему: Тяжёлая структура с UniString копируется трижды на итерацию.

### 77
Файл: Sources/AddOn/Helpers.cpp
Строка/функция: ParamHelpers::ReadGDL, ~6288
Что сделать: `GS::UniString rawName = param.rawName;` → `const GS::UniString &rawName = param.rawName;` (локальную копию оставить только в ветке, где rawName модифицируется).
Почему: Копия строки на каждый параметр каждого элемента.

### 78
Файл: Sources/AddOn/Propertycache.cpp
Строка/функция: ReadClassification ~610, ReadInfo ~505, AddFile ~411
Что сделать: Пары ContainsKey+Get заменить на GetPtr.
Почему: Двойной lookup по одному ключу.

### 79
Файл: Sources/AddOn/ClassificationFunction.cpp
Строка/функция: GetAllClassification, ~72-80
Что сделать: Последний Put в systemdict выполнить как `Put (systemname_full, std::move (classifications))` (autoclass-копию сформировать до move).
Почему: Две полные глубокие копии словаря из тысяч записей; move устраняет одну.
Ограничения: После move исходный classifications не читать.

### 80
Файл: Sources/AddOn/ClassificationFunction.cpp
Строка/функция: GetFullName, ~156-166
Что сделать: Значение по itemname получить один раз через GetPtr и работать через указатель; для parentname аналогично.
Почему: До 6-7 lookup хэш-таблицы на каждый уровень рекурсии.

### 81
Файл: Sources/AddOn/Summ.cpp
Строка/функция: Sum_GetElement, ~203-217
Что сделать: `const API_PropertyDefinition definition = cIt.value;` → const-ссылка (в старых ветках — разыменование указателя); тройной lookup ContainsKey/ContainsKey/Get заменить на GetPtr с Add при nullptr и повторным GetPtr.
Почему: Копия тяжёлой структуры + три поиска на каждый параметр.

### 82
Файл: Sources/AddOn/Summ.cpp
Строка/функция: Sum_OneRule, ~398/478/484
Что сделать: `eleminpos` брать по const-ссылке; ToString(summ) вычислить один раз до цикла записи; копию ParamValue делать только в момент записи.
Почему: Копии массива и структуры на каждой итерации; ToString перевычисляется в цикле без изменения summ.

### 83
Файл: Sources/AddOn/pk/ResetProperty.cpp
Строка/функция: ResetOneElemen, ~245
Что сделать: `API_Property property = properties.Get (i);` → ссылка `API_Property &property = properties[i];`; в properties_to_reset пушить копию этого поля.
Почему: Копия структуры с вариантными строками на каждое свойство каждого элемента.
Ограничения: Убедиться, что properties[i] после этого повторно не читается как неизменённый.

### 84
Файл: Sources/AddOn/spec/Spec.cpp
Строка/функция: ~624-641 и ~1452
Что сделать: Пары ContainsKey+Get по paramdict_favorite заменить на GetPtr; вызов ACAPI_Element_Filter вынести из цикла по группам (результат не зависит от группы).
Почему: Двойные lookup в цикле по параметрам; дорогой API-вызов умножается на число групп.

### 85
Файл: Sources/AddOn/pk/AutomateFunction.cpp
Строка/функция: AlignOneDrawingsByPoints, ~775-790
Что сделать: В цикле по хотспотам добавить выход (break) при найденных обеих крайних точках.
Почему: ACAPI_Element_Get на каждый хотспот чертежа, хотя нужные точки уже найдены.
Ограничения: Логику выбора крайних точек не менять.

### 86
Файл: Sources/AddOn/MEPv1.cpp (только AC27+)
Строка/функция: GetRoutingElementSharedDataCached ~152, GetSubElementOfRoutingCached ~168
Что сделать: После Put(key, data) не выполнять повторный GetPtr(key) — использовать перегрузку Add с out-параметром valueInContainer или GetPtr до вставки.
Почему: Двойной lookup хэш-таблицы в кэш-функциях.

### 87
Файл: Sources/AddOn/CommonFunction.cpp
Строка/функция: GetNumSymbSpase, ~1078
Что сделать: std::atoi заменить на std::stoi в try/catch (при ошибке — stringlen остаётся 0).
Почему: atoi молча даёт 0 — ошибка разбора маски длины неотличима от 0 (и поведение при 0 сейчас корректно, но неявно).
Ограничения: Только парсинг; логику выравнивания не менять.

---

## НЕ ВКЛЮЧЕНО (для справки, GLM не передавать)

REJECTED — ложные срабатывания ревью (опровергнуты DevKit-25):
- Helpers.cpp ReadCoords Door: чтение element.window.openingBase для двери — API_WindowType и API_DoorType один и тот же struct (APIdefs_Elements.h:1323), память идентична.
- Helpers.cpp ReadElementValues Object→Lamp fall-through — API_ObjectType и API_LampType один struct (APIdefs_Elements.h:1408), записывается то же значение.
- BrowserPalette HighlightElements: «Add с дублем GUID — краш» — GS::HashTable::Add возвращает bool, не ассертится (HashTable.hpp:811-894).
- Revision.cpp GetScheme: «двойной Add — краш» — та же причина, Add молча возвращает false.
- Roombook.cpp Param_Material_Get: «мусорный индекс при ненайденном материале» — API_AttributeIndexFindByName (функция самого аддона, CommonFunction.cpp:2430) делает BNZeroMemory attribinx на входе; при неудаче записывается 0, не мусор.

UNVERIFIED — требуют ручной проверки (в LightRAG нет контекста / контекст неоднозначен / нужно решение владельца):
- CommonFunction.cpp msg_rep:740 — пишет ли ACAPI_WriteReport(msg, true) строку в отчёт повторно (заголовок даёт только сигнатуру с withDial).
- CommonFunction.cpp ConvertPolygon2DToAPIPolygon:2339 — раскладка дуг Geometry::ConvertPolygon2DToPolygon2DData (индексация с 0 или 1) не подтверждена.
- Roombook.cpp OtdWall_Create_FromWall:1669 — inxedge == GetSize(): назначение фиктивного ребра в конце edges требует уточнения.
- Roombook.cpp Floor_Create_One:2138 — мёртвые dzBottom/dzUp включены (п.60); если формула понадобится — dzUp = poly.zBottom - el.zUp (не подтверждено).
- Propertycache.hpp Update:310 — семантика вызова Update (что реально протухает: compositeCache/filedata) требует уточнения точек вызова.
- Propertycache.cpp GetCache — модель потоков JSON-команд (нужен ли мьютекс на selectionPropertiesCache).
- Summ.cpp:432/445/452 — участвуют ли bool-значения в MIN/MAX/NUM_SUM (нужно решение).
- Dimensions.cpp DimRoundAll isUndo=true — возможно сознательная оптимизация против лишнего undo-шага (решение владельца).
- Dimensions.cpp DimAutoRoundOne для Zombie/Group (checktype=false) — чтение union element.dimension по не-размеру; фактический эффект не подтверждён.
- Dimensions.cpp DimAutoRound:54-71 — возможен ли в данных ключ-число, совпадающий с подстрокой имени слоя (включено защитой, п.72).
- Helpers.cpp ReadCoords:2402 — база индексации API_CWPanelType.segmentID (0/1-based) в DevKit не указана.
- Helpers.cpp ReadMorphParam:1204 — intended-оси проекций {@morph:ly}/{@morph:lz} (нужна формула по ТЗ).
- AutomateFunction.cpp GetCuplane:68-74 — intended-поведение при симметричном отрезке (isx).
- ResetProperty.cpp:40-113 — оборачивать ли массовые SetProperties в CallUndoableCommand (решение владельца).
- Spec.cpp:861/1754/2444 — идентичность форматов APIGuidToString и APIGuid2GSGuid().ToUniString() не проверена.
- BrowserPalette.cpp Show():115 — ReloadIgnoreCache на каждый показ: возможно сознательный reload по ТЗ (решение владельца).
- ClassificationFunction.cpp SetAutoclass — точки вызова (массовость) не изучены.
---

## Отчёт о применении (2026-09-12, сессия правок)

**Применено и проверено LSP/clang-format (БИЛД НЕ ЗАПУСКАЛСЯ):** п.1-19, 20-24, 27-28 (частично: Summ.cpp + Spec.cpp; Roombook.cpp suspend — НЕТ), 30, 31, 33, 14.5 (ConvertPolygon2D дуги +1 — подтверждено Polygon2DDataConv.h).

**Расхождения, выявленные при правке (ревью/инструкция скорректированы по факту кода):**
1. **«Тройные» ResetOneElemenDefault (инструкция 59)** — на самом деле НЕ дубли: вызовы имеют разные 3-и аргументы (1146245920/1145194016/...). НЕ тронуты. Ревьюер ошибся.
2. **axis-угол (п.11)** — реализовано через отдельные out-переменные (symb_rotangle_axis_fraction / bsymb_rotangle_axis_correct / ..._1000), а не перезапись slant-значений: CoordCorrectAngle использует out-параметры, переиспользование переменных slant-блока испортило бы их.
3. **п.9 (l_wall==0)** — guard охватывает и вычисление координат, и все 6 записей symb_pos_* (первая попытка случайно удалила записи — восстановлены внутри блока).
4. **ReplaceCR (п.12)** — реализовано как SetChar(CR)+Delete(inx+1,1) (ReplaceFirst на пустую строку в AC25 убрал бы оба символа); Count(p) зафиксирован до цикла.
5. **п.29 (MEPv1 UnwrapErr) — НЕ применён** (файл компилируется только в AC27+, для AC25-сборки пустой; правка отложена до сборки AC27+).
6. **п.25/26 (BrowserPalette) — НЕ начаты.**

**Не применено (следующие шаги):** Roombook suspend ×2 (4121, 5286 — цикл, вынести за цикл), BrowserPalette-блок, DimParse (32), minor/PERF-хвост (39-87).

**Валидация:** clang-format по каждому изменённому файлу выполнен; LSP-диагностика после правок — только pre-existing warnings (unused variables и т.п., все вне зон правок кроме контролируемых случаев). Билд и runtime-тест НЕ выполнялись — обязательны перед коммитом.

## Отчёт о ревью субагентами (2026-09-12, вторая сессия)

Проверены все ✅-пункты статическим ревью (3 субагента, сравнение с checkpoint bbbe71a + сверка с DevKit-25). Итог: правки 1-24, 27, 28 (частично), 30, 31, 33, 50, 52-54, 58 — корректны. Исправленные дефекты:

1. **CommonFunction.cpp ElemHead_To_Neig (AC26+-ветка):** `typeID = type;` → `typeID = type.typeID;` — присваивание структуры к enum не скомпилировалось бы под 2600+.
2. **SomeStuff_Main.cpp MenuCommandHandler:** восстановлен случайно удалённый при п.30 вызов `MenuSetState (syncSettings)` — без него галочки меню не обновляются после переключения флагов/палитры.
3. **pk/AutomateFunction.cpp AlignDrawingsByPoints (п.55 доделан):** сброс store (store=0 + StoreViewSettings) добавлен на двух поздних ранних выходах (gooddrawings/coords пусты) — раньше настройки вида оставались сохранёнными.
4. **pk/Revision.cpp GetMarkerPos:** BMGetHandleSize(memo.coords) вынесен после проверки coords != nullptr (гигиена, раньше вычислялся безусловно).

Принятые как есть (не блокируют): (a) ConvertPolygon2DToAPIPolygon копирование дуг с +1 — согласовано с ConstructPolygon2DFromElementMemo, конвенция Geometry::Polygon2DData.arcs[0] фиктивна; (b) ucscpy-обрезка допускает 256 символов в буфер 256 (off-by-one терминатора — унаследовано от одобренного паттерна Spec.cpp:2545); (c) Sum_GetElement вычисляет фильтры до цикла даже если все правила в первой ветке (лишний O(N) в редком случае).

**Статус сборки: БИЛД ПОСЛЕ ПРАВОК НЕ ЗАПУСКАЛСЯ — обязателен перед коммитом.**

## Отчёт о финализации (2026-09-12, третья сессия)

Реализованы все оставшиеся пункты (4 субагента + доводка вручную): 25/26/37/38/63-70 (BrowserPalette, повторный запуск после 503 OpenRouter), 28-остаток/34/35/36/60/73/74/75 (Roombook, cpp+hpp синхронно), 32/40/62/72/87 (Dimensions+hpp, Helpers, CommonFunction), 29/41/43-49/51/56/57/71/76-77/79-86 (MEPv1 — только AC27+, компиляцией AC25 не проверяется), вручную: 42, 61, 78 (Propertycache.hpp).

Примечания:
- П.61: p.dim1 = addedOpenings (счётчик фактических проёмов); переменная dim2 в блоке осталась неиспользуемой (pre-existing).
- П.78: API_Guid systemguid теперь копия (sysitem из GetPtr — const); ConstPairIterator в циклах RoomBook (594/600) после перехода на const-ссылки — скорректированы при сборке.
- Попутно исправлено субагентом в п.77: в мутирующей @arr-ветке ReadGDL была запись в const& — исправлено локальной копией (иначе ошибка компиляции).
- **Сборка AC25 (Win): Build succeeded** (Build/SomeStuff/25/Debug/SomeStuff.apx). LNK4099 (API_c.pdb) — pre-existing warning.
- Runtime-тест НЕ выполнялся. MEPv1.cpp (29/71/86) собран только через AC25 (файл пуст для AC25) — правки AC27+ не компилировались: not verified до сборки AC27+.
