# P2 — minor и PERF правки (SomeStuff addon, ревью 2026-09-12b)

Часть 2 из 2. Общие правила (clang-format по файлу, TDD-цикл, LightRAG-верификация SDK) — см. часть 1: P0P1-fixes-2026-09-12b.md в D:\SomeStuff_addon.

## P2 — minor и PERF (батчами по файлам, без RED-тестов, кроме отмеченных)

### Roombook.cpp
- 2138-2139: удалить dzBottom/dzUp (мёртвые) — если код планируют использовать,
  исправить dzUp = poly.zBottom - el.zUp и оставить с комментарием.
- 2742: `GSErrCode err = API_AttributeIndexFindByName (...); if (err == NoError)
  val.intValue = ...; else <оставить прежнее значение/0>` — решение по поведению
  «материал не найден» принять по месту (не перезаписывать мусором).
- 4460-4486: `UInt32 addedCount = 0;` — инкремент вместо continue-фильтра;
  `p.dim1 = addedCount;` после цикла.
- PERF 567-596: `const UnicGuidByBase &byzone = cIt->value;` (и bytype/byparent) —
  чистая замена, поведения не меняет (только чтение).
- PERF 3839: сигнатуру OtdWall_Delim_One → `const OtdWall &otdn`; внутри для
  изменяемой копии — локальная; финал `opw.Push (std::move (localCopy))`. Требует
  аккуратности: тело функции мутирует otdn — правку делать только после чтения тела
  (внутри создаётся копия уже есть? проверить). Согласовать объём.
- PERF hpp:346/365: ReadParams → по неконстантной ссылке `ReadParams &readparams` +
  подготовка рабочей копии один раз в вызывающем цикле (Param_Property_Read мутирует
  isValid/val — изоляция между зонами обязательна, копия должна остаться, но одна
  на цикл, а не на вызов).
- PERF 176-181: `const GS::Array<API_Guid> &zoneGuids = cIt->value;` обе ветки #ifdef.
- PERF 2075/2088: GuidByZ хранить API_Guid напрямую (сортировку заменить на
  сравнение GUID или сортировать после конвертации).
- PERF 521-527/1082-1084: `if (const ColumnFormat *cf = columnFormat.GetPtr (rawname))`
  / `if (const double *a = dcta.GetPtr (mat))`.

### Helpers.cpp
- 2566/2574: `true` → `0.0` в двух вызовах AddDoubleValueToParamDictValue.
- 823/865: `duration = (double)(t2 - t1) / CLOCKS_PER_SEC;` и подпись "s" (убрать
  /1000); CommonFunction.cpp:856-857 — то же самое, править парой.
- 2402-2413: проверить базу API_CWPanelType.segmentID по DevKit-25 (0-based?) —
  если да, `size > inx_segment`. До выяснения не трогать (uncertain).
- PERF 5978-5982: `const API_IFCProperty &property = properties[i];` + убрать два
  повторных properties.Get(i).
- PERF 6288: `const GS::UniString &rawName = param.rawName;` — копию только в ветке
  fromGDLArray, где rawName переиспользуется после модификации.

### CommonFunction.cpp
- 2608: `if (memo.params == nullptr) return false;` в ParamToMemo; заодно проверка
  pp->arr_num.GetSize() >= dim1*dim2 перед копированием [k*dim2+j].
- 2589: `GS::UniString valueLower = value.ToLowerCase ();` один раз до циклов.
- 740-742: сверить по DevKit-25 семантику ACAPI_WriteReport(msg, show) — пишет ли
  true-вызов строку в отчёт повторно; если да, первый вызов только при !show.

### Propertycache.cpp/.hpp
- 286-315: убрать ранние return false в isCacheContainsParamValue при isXXX_OK==false —
  повторить цепочку источников GetParamValueFromCache (или объединить функции через
  общий helper). RED-тест не нужен: по определению функции должны давать согласованные
  ответы для одного ключа (GREEN-тест: Contains==false ⇔ Get==false на всех ключах).
- 41-56: перенести GetDataLength после Open; проверить err от ReadBin, при ошибке
  `delete[] buff; return false;`.
- 672: `err = ACAPI_Attribute_GetAttributesByType (...); if (err != NoError)
  { msg_rep (...); return false; }`.
- Update (hpp:310-339): добавить `compositeCache.Clear (); parsedformatstring.Clear ();
  formatstringformeasuretype.Clear (); isFormatStringFormeasureTypeRead = false;` —
  перед правкой проверить точки вызова Update (открытие/смена проекта) и стоимость
  реального чтения этих словарей (нужен ли ленивый ре- read, который уже есть через
  флаги isXXX_OK — если да, достаточно сбросить флаги, не сам кэш).
- 6 (race): до правки выяснить модель потоков ACAPI_Install_AddOnCommandHandler —
  выполняются ли JSON-команды в главном потоке (тогда только комментарий в коде);
  если фоновые — мьютекс на selectionPropertiesCache.
- PERF: GetPtr-замены в ReadClassification/ReadInfo/AddFile.

### ClassificationFunction.cpp
- GetFullName: `if (parentname == itemname) break;` перед рекурсией (дешевле, чем
  ограничение глубины) + опционально счётчик глубины.
- GetAllClassification: `err = NoError;` после continue (сбрасывать, чтобы одна
  сбойная система не гасила весь кэш).
- FindClass(Pair): `if (!ReadSystemDict ()) return {};` первой строкой.
- PERF: std::move для второго Put; убрать ToLowerCase() в 138-139; GetPtr в GetFullName.

### Summ.cpp
- 432/445/452: согласовать с Дмитрием, участвуют ли bool в MIN/MAX/NUM_SUM; если да:
  MIN_SUM — `&&`, MAX_SUM — `||`, NUM_SUM — как intValue; если нет — оставить и
  пометить «баг неактивен» в отчёте.
- PERF 252-259: собрать результат фильтрации один раз на элемент (массив флагов или
  отфильтрованный список) до цикла по правилам.
- PERF 203-217: const& definition; `SumRule *rule = rules.GetPtr (definition.guid);`
  с Add при nullptr и повторным GetPtr.
- PERF 398/478/484: const& eleminpos; ToString(summ) до цикла; ParamValue копию
  только в момент записи.

### Dimensions.cpp
- 141-148/300-319: `GS::UniString originalContent = content;` до цикла правил;
  DimParse принимает/сравнивает с originalContent, мутирует только memo.
- 257: передавать measuredvalue отдельным аргументом DimParse (вместо копии всего
  paramDict) — точечная правка сигнатуры DimParse (внутренняя, не публичный API).
- 54-71: при переборе как layer-regex пропускать ключи, уже добавленные по kstr
  (сравнение ключа с kstr) — если ключи-числа и regex в одном dict не разделить.
- SomeStuff_Main.cpp:155 (Zombie/Group): в ElementEventHandlerProc вынести
  API_DimensionID отдельным case (DimAutoRoundOne только для размеров).

### ResetProperty.cpp
- 381: в AC26+-ветке `ACAPI_Element_SetPropertiesOfDefaultElem (type,
  properties_to_reset);` — выравнивание с AC25-веткой (для AC25-сборки неактивно,
  править для консистентности).
- 40-113: обернуть тело в ACAPI_CallUndoableCommand — согласовать с Дмитрием
  (могут быть причины не оборачивать сброс дефолтов).
- 63/81: удалить второй вызов ResetElementsInDB (строка 81).
- 273/297/303: оставить один вызов ResetOneElemenDefault(API_ObjectID,...).
- 245: `API_Property &property = properties[i];` + `properties_to_reset.Push
  (std::move (property))` — внимание: массив properties после move повторно не
  читается? проверить тело перед правкой.

### Spec.cpp
- 754: `return APIERR_CANCEL;`.
- 861/1754/2444: проверить в отладчике/тесте, идентичны ли форматы APIGuidToString и
  APIGuid2GSGuid().ToUniString(); если различаются — привести к одной функции.
- 2316-2318: `if (memot.params == nullptr) return false;`.
- PERF 624-641: GetPtr-замены; 1452: вынести ACAPI_Element_Filter из цикла по группам.

### Revision.cpp
- 94-111: перед каждым layout_note_guid.Add — ContainsKey-проверка или Put
  (заменить Add на Put везде в GetScheme: Put безопасен и для новых, и для дублей).
- 779: `Int32 code = 0; if (!UniStringToInt32 (code.ToCStr(), &codeN)...)` — или
  std::stoi в try/catch по образцу Sync.cpp; ошибку логировать msg_rep.
- 899/1004: `if (memo.params == nullptr) return false;`.
- 628/947: `for (auto &ch : changes)` в обеих ветках — безопасно и для AC25.
**Проверка Revision-правок:** ручной тест маркеров ревизии на макете (создание +
смена текста + дубликаты имён схем).

### AutomateFunction.cpp
- 914/935/944: вынести восстановление store/БД/окна в единый выход (лямбда-финализатор
  или флаг + общий return) — правка структурная, но локальная.
- 68-74: требует уточнения intended-логики (симметричный отрезок) — пометить, не
  править без решения.
- PERF 331-343: `GS::HashTable<GS::UniString, bool> seen;` с ключом
  Printf("%.3f:%.3f", p[i].x, p[i].y) — O(n) вместо O(n²).
- PERF 775-790: `break` при найденных обеих крайних точках; (батчинг опционально).

### MEPv1.cpp (AC27+; для AC25-сборки не горит)
- 360: заменить ветку «empty + IsOk» на сообщение без UnwrapErr: `ACAPI_WriteReport
  ("No physical system found", false);` — UnwrapErr только в IsErr-ветке.
- 240-242: `if (err != NoError) return;` после GetHeader.
- 152/168: Put → использовать возвращённое значение/указатель вместо повторного GetPtr.

### BrowserPalette.cpp (дополнение к P1 выше)
- 331: считать `count` по фактически добавленным в JSON элементам (счётчик в цикле)
  или оставить GetSize, но добавить поле `processedCount` — согласовать с ТЗ UI.
- 214-265: при вычислении имени группы использовать кэш
  PROPERTYCACHE().propertygroups через ParamHelpers::GetGroupFromCache (после его
  фикса — см. Propertycache) вместо ACAPI_Property_GetPropertyGroup на каждое
  свойство × элемент; выборка значений — через существующий
  selectionPropertiesCache-механизм (он уже инвалидируется в ElementsWrite).
- 438: передавать имя свойства (найти в cache.property по GUID) вместо propertyId,
  или переименовать поле в propertyId — согласовать с HTML (сейчас HTML поле не
  читает, правка безопасна).
- 435: `bool isCommon = (selectedElements.IsEmpty ()) || (uniqueValuesCount == 1
  && totalCount == ...);` — согласовать семантику с HTML-обработкой.
- 778-784/1017-1035: привести «no selection»/catch-ответы к JSON-строке того же
  формата, что и нормальный путь (1014) — HTML-гарды оставить.
- 963-968/990-993: `Int32 cnt = 0, tot = 0; Int32 count = 0, total = 0;`.
- Все мост-лямбды без try/catch (GetPropertiesList, GetPropertyValue,
  GetSelectionInfo, HighlightElements, Parse*): единый try/catch с JSON-ответом
  `{"status":"error"}` — по образцу GetClassification.
- FilterElementsByType (1234-1249): заменить ACAPI_Element_Get на
  ACAPI_Element_GetHeader (только header.typeID нужен) — верифицировать сигнатуру
  GetHeader через LightRAG (заполняет ли type без полного Get — в AC25 да,
  header-запрос).
- UpdateSelectionInfoInUI (147-148): до раннего return при пустом выделении
  выполнить ExecuteJS с count=0 (вынести построение jsCall выше по функции).
**Проверка BrowserPalette-правок:** ручной прогон палитры: выделение/снятие
выделения (счётчик сбрасывается в 0), свойства, классификация, подсветка, отмена
в середине операции; `powershell -File Tools/test_html.ps1` после любых правок HTML.

## Порядок внедрения (предложение)

1. **P0**: WriteGDL return, Floor_Create_One memo={}. — 2 правки, оба RED-теста.
2. **SuspendGroups (S1)**: 4 файла одним паттерном из Sync.cpp — систематический
   эффект, легко проверить вручную на группах.
3. **P1 краш-риски** (Dispose-утечки Roombook/Helpers/ReadListData, SelectionInfo
   `={}` ×2, Spec exsist_elements, Spec_libpart guards+subpos, Revision customData /
   uStr-обрезка / GetMarkerPos, AutomateFunction БД, ResetProperty APIDb, MEP UnwrapErr).
4. **P1 логика** (ReplaceCR, fall-through ×2, Door openingBase, l_wall==0,
   GetParentGUIDSectElem, axis angle, GetGroupFromCache, ElemHead_To_Neig — после
   уточнения вызовов, SumSelected undoErr, Spec APIERR_CANCEL).
5. **BrowserPalette P1** (guard suppressSelectionRefresh, UnregisterJSObject,
   Show/ReloadIgnoreCache — после согласования).
6. **PERF-батчи** по файлам: копии → const&, ContainsKey+Get → GetPtr, DimRoundAll
   горячие пути, FilterElementsByType → GetHeader.
7. **Uncertain-позиции** (18 шт.) — отдельным списком на решение Дмитрия: morph-формулы,
   bool-агрегация, Update-семантика, race-модель потоков, GUID-форматы, GetCuplane,
   segmentID-база, APIGuidToString vs GSGuid, msg_rep-дубль, DimRoundAll isUndo,
   ResetProperty undo, SetAutoclass-точки, Show-reload-намерение.

После каждого фикса — обновлять статус в этом файле (аналогично
review-dialogs-sync-renum-2026-08-24.md), чтобы следующий ревью не перепроходил
исправленное.
