# Класс отрисовки таблиц (TableRenderer) — проверенная редакция для Archicad 25

Дата проверки: 2026-09-18. Проект: AddOn_SomeStuff. Issue: #177. Целевая версия: **AC25**.

Исходник: черновик ТЗ `table-renderer-tz.md` (вложение пользователя, 219 строк).
Это проверка и уточнение черновика по установленному SDK, а не его переписывание.
Правки относительно черновика отмечены как **Коррекция**.

Поиск: LightRAG `http://127.0.0.1:9621/query` (`only_need_context: true`, режимы `local`/`naive`;
`hybrid` по этим идентификаторам пуст — это сигнал режима, не отсутствия данных), затем
сверка с заголовками и примерами DevKit-25.

Обозначения: **Verified (SDK25)** — контракт заголовка/документации/примера; **Предложение** —
проектное решение; **not verified** — нет доказательств, нужен runtime.

Сокращения путей, все относительно `D:/SomeStuff_addon/Build/DevKit/APIDevKit-25/`:
- `H/` = `Support/Inc/`
- `F/` = `Documentation/HTML/APIDevKit/APIHTMLLibrary/Functions/`
- `S/` = `Documentation/HTML/APIDevKit/APIHTMLLibrary/Structures/`

---

## 1. Коррекция архитектурного предположения черновика (§1 ТЗ)

Черновик: «рисует таблицу линиями/текстом/заливками через существующий рендер-пайплайн
(`ACAPI_Drawing_StartDrawingData` / `Line` / `Text` / `Hatch` / `ACAPI_Drawing_StopDrawingData`)».

**Коррекция.** В AC25 таких функций нет. Механизм отрисовки — «redirect element creation»:

1. `ACAPI_Database (APIDb_StartDrawingDataID, [double* scale], [API_PenType** pens])` — открывает
   внутренний store; **дальнейшее создание 2D-элементов идёт в него**, а не в базу проекта.
   `APIERR_NESTING`, если сессия уже открыта (вложенность запрещена). Источник:
   `H/APIdefs_Database.h:91`, `F/APIDb_StartDrawingDataID.html` — Verified (SDK25).
2. Внутри сессии — `ACAPI_Element_Create (&element, &memo)` обычными 2D-элементами:
   `API_LineID`, `API_HatchID`, `API_TextID`. Источник: `Examples/Element_Test/Src/Element_Basics.cpp:1175-1193`
   (линия/окружность в drawing data), `Navigator_Test/Src/NavigatorCallbackInterface.cpp:38`
   (реальный путь CreateIDFStore) — Verified (SDK25).
3. `ACAPI_Database (APIDb_StopDrawingDataID, GSPtr* idfMem, API_Box* boundBox)` — возвращает
   сериализованный IDF. Память `idfMem` освобождает вызывающая сторона (`BMKillPtr`). Источник:
   `H/APIdefs_Database.h:92`, `F/APIDb_StopDrawingDataID.html` — Verified (SDK25).

Следствия для границ класса:

- `Draw` **не открывает и не закрывает** сессию: владелец сессии — вызывающая сторона
  (`NavigatorCallbackInterface::CreateIDFStore`). Класс рисует в уже открытый store.
- «Only 2D elements (including figures/pictures) are allowed» (`F/APIDb_StartDrawingDataID.html`) —
  текст, линии и заливки допустимы; 3D/MEP-элементы — нет.
- Порядок примитивов внутри сессии = порядок создания элементов, поэтому §5 черновика
  (заливки → сетка → текст) остаётся в силе.

## 2. Единицы измерения (коррекция, §2–§3 ТЗ)

**Verified (SDK25):**

- `APIAny_GetTextLineLengthID` возвращает длину строки **в мм** (`F/APIAny_GetTextLineLengthID.html`).
- `API_TextLinePars.wSize` — «Character height in mms»; `drvScaleCorr`: true — «scale the text to the
  model», false — «in mms» (`S/API_TextLinePars.html`, `H/APIdefs_Goodies.h:304-318`).
- `API_TextType.size` — «char height in mm» (`H/APIdefs_Elements.h`).
- `APIDb_StartDrawingDataID.dScale` — «defines the scaling from paper to model. For example, for
  1:100 scaling, pass 100» (`F/APIDb_StartDrawingDataID.html`).

**Предложение (принято в реализации).** Все правила форматирования задаются в **миллиметрах
бумаги** (кегль, паддинги, фиксированные ширины/высоты) — это единицы SDK для текста.

- `ComputeLayout` считает геометрию в мм и переводит её в модельные единицы
  (мм → м) коэффициентом `scale / 1000`, где `scale` — то же значение, что передано в
  `APIDb_StartDrawingDataID`.
- `GetTotalWidth / GetTotalHeight / GetColumnWidth / GetRowHeight` возвращают **модельные
  единицы** (то, в чём работает `Draw`).
- Измерение текста выполняется с `drvScaleCorr = false` (мм), чтобы не зависеть от масштаба.

Точное соответствие «мм правил ↔ модельные единицы drawing data» проверяется визуально:
**not verified** (см. §8).

## 3. Входные данные (§2 ТЗ, с уточнениями)

### 3.1 Данные ячеек (что рисовать)

Черновик: `int row/col`, `GS::UniString text`, `fontNameOverride`, `hAlignOverride`.

**Коррекции:**

- Типы — проектные: `Int32`, `GS::Array` (в проекте используются `GS::Array`, а не `std::vector`;
  черновик сам разрешает зафиксировать сигнатуры после сверки с типами проекта).
- `fontNameOverride` → **индекс шрифта** (short). В SDK текст адресует шрифт индексом атрибута
  (`API_TextType.font` — «font index»), а не именем. Разрешение имени в индекс — отдельный
  helper (`ACAPI_Attribute_Search`, §4.5) и только на уровне правил.
- `HAlign` получает значение `Default` — «взять из правил», как и было задумано в черновике.
- Добавлен `VAlign vAlignOverride` по симметрии (в SDK вертикального выравнивания у текста нет,
  см. §5.3 — его эмулирует рендерер).

Sparse-представление сохраняется: ячейка, отсутствующая в массиве, считается пустой.

### 3.2 Правила (как рисовать)

Состав полей черновика сохранён; уточнения:

- `ColumnRule.fixedWidth / minWidth / maxWidth` — в мм.
- `RowRule.fixedHeight / minHeight` — в мм.
- `gridLineWeight` → **`gridPenIndex` (short)**: толщина линии в ArchiCAD задаётся пером
  (`API_LineType.linePen.penIndex` — Verified (SDK25), `Element_Basics.cpp:4136-4147`), а не
  «весом» в мм.
- Добавлены: `layerIndex`, `lineTypeIndex`, `textPenIndex`, `fillPenIndex`, `fillContourPenIndex`,
  `defaultFontIndex`, `defaultFontName`, `defaultFaceBits`. Значение `0` = «взять из
  `ACAPI_Element_GetDefaults`»; `layerIndex = 0` не угадывается вручную.
- Заливки — **списком диапазонов** (решение открытого вопроса §6 ТЗ), по аналогии с
  `mergedRanges`: `FillRange { MergedRange range; API_AttributeIndex fillIndex; short fillPen;
  short contourPen; }`. `fillIndex` невалиден → заливка диапазона не рисуется.

## 4. Алгоритм ComputeLayout (§4 ТЗ — принят, с уточнением измерения)

4.1 Проверка входных данных: `rows/cols` против фактического диапазона ячеек; `MergedRange` —
валидный прямоугольник в границах таблицы; пересекающиеся объединения → ошибка входных данных
(рендерер их не разруливает). Принято без изменений.

4.2 Ширины столбцов (`Auto`): максимум требуемой ширины по необъединённым ячейкам
(текст + 2×`cellPaddingH`) → дораспределение под многостолбцовые объединения пропорционально
текущим ширинам, `Fixed`-столбцы не трогаются → финальный клип `minWidth`/`maxWidth`.
`Fixed` — `fixedWidth` напрямую. **Принято без изменений.**

4.3 Высоты строк (`Auto`): после ширин столбцов, с учётом числа строк после переноса.
**Принято без изменений.** Порядок «сначала ширины, потом высоты» — обязателен.

4.4 Перенос текста. **Уточнение (решение открытого вопроса §6 ТЗ):**

- Измерение ширины строки — `ACAPI_Goodies (APIAny_GetTextLineLengthID, &pars, &lenMm)`,
  `pars.index = -1` (последняя строка / нет терминатора), `drvScaleCorr = false` (мм),
  `wSize` = кегль в мм, `wFont` = индекс шрифта, `wSlant = PI/2` (plain), `lineUniStr` = текст.
  Источник: `F/APIAny_GetTextLineLengthID.html`, `S/API_TextLinePars.html` — Verified (SDK25).
- Перенос выполняет **рендерер**, а не Archicad: разбивка по словам, затем посимвольно, если
  слово шире доступной ширины. Число строк из этого шага идёт в высоты строк (4.3).
- `API_TextType.nonBreaking = false` («wrap around destination rect») существует, но его
  взаимодействие с `width`/`height`/`multiStyle` в drawing data не проверено — **not verified**,
  поэтому на первом слое перенос считается самим рендерером.

## 5. Порядок отрисовки (§5 ТЗ — принят, с уточнением способа)

1. Заливки (`API_HatchID`) — по одному элементу на диапазон.
2. Сетка (`API_LineID`) — сегменты между узлами сетки, кроме строго внутренних для объединений.
3. Текст (`API_TextID`).

### 5.1 Сетка и объединения

Алгоритм черновика (полный список сегментов → вычесть внутренние для `MergedRange`) принят.
Проверка «строго внутри» — по узлам: сегмент не рисуется, если оба его конца лежат внутри
прямоугольника объединения (не на его границе).

### 5.2 Элементы: проверенные поля (Verified (SDK25))

- Линия: `element.header.typeID = API_LineID`, `line.linePen.penIndex`, `line.ltypeInd`,
  `line.begC`, `line.endC`; `ACAPI_Element_Create (&element, nullptr)`.
  Источник: `Element_Basics.cpp:4136-4147`.
- Заливка: `header.typeID = API_HatchID`, `hatch.fillInd`, `hatch.fillBGPen`,
  `hatch.fillPen.penIndex`, `hatch.ltypeInd`, `hatch.contPen.penIndex`,
  `hatch.poly.nCoords/nSubPolys/nArcs`, `memo.coords`/`memo.pends` (BMhAllClear, 1-based,
  замыкающая точка дублирует первую) → `ACAPI_Element_Create` → `ACAPI_DisposeElemMemoHdls`.
  Источник: `Element_Basics.cpp:177-211`.
- Текст: `header.typeID = API_TextID`,
  `memo.textContent = BMhAllClear ((len + 1) * sizeof (GS::uchar_t))` + `GS::ucscpy`
  (`text.ToUStr ()`), `text.loc`, `text.size` (мм), `text.font`, `text.pen`, `text.anchor`,
  `text.just`, `text.nonBreaking` → `ACAPI_Element_Create` → `ACAPI_DisposeElemMemoHdls`.
  Источник: `Element_Basics.cpp:3985-4010`.
- `API_JustID`: `APIJust_Left = 0`, `APIJust_Center`, `APIJust_Right`, `APIJust_Full`
  (`H/APIdefs_Elements.h:431-434`).
- `API_AnchorID`: `APIAnc_LT/MT/RT/LM/MM/RM/LB/MB/RB` (`H/APIdefs_Elements.h:2195-2205`).
- `header.layer`: значение из `ACAPI_Element_GetDefaults` — не выдумывается вручную
  (в примерах встречается «магическая» константа `header.layer = 5`, повторять её нельзя).

### 5.3 Вертикальное выравнивание — эмуляция рендерером

**В `API_TextType` нет поля вертикального выравнивания** (полный список полей проверен по
`H/APIdefs_Elements.h`) — есть только `anchor` («kind of text center») и `just` («justification
of text»).

**Предложение.** Поскольку рендерер сам знает число строк и их высоту, он сам считает позицию:
одна текстовая строка = один `API_TextID` с `anchor = APIAnc_LB`, а `loc.x`/`loc.y` вычисляются
из `hAlign`/`vAlign` и измеренной ширины строки. Это детерминированно и не зависит от
непроверенной семантики комбинации `just` + `anchor`.

Цена: элементов больше, чем «один текст на ячейку»; многострочный текст одним элементом —
возможная оптимизация после визуальной проверки (**not verified**).

## 6. Единицы в правилах: соответствие черновику

| Черновик | Реализация | Причина |
|---|---|---|
| `int row/col` | `Int32` | проектный стиль |
| `std::vector<T>` | `GS::Array<T>` | проектный стиль |
| `fontNameOverride` | `fontIndexOverride` (short) | `API_TextType.font` — индекс |
| `HAlign` (без Default) | `HAlign::Default` + `..._Override` | «Default → из правил» |
| `gridLineWeight` (мм) | `gridPenIndex` (short) | толщина линии = перо |
| заливки «схематично» | `GS::Array<FillRange>` | диапазоны, как `mergedRanges` |
| `void Draw` | `GSErrCode Draw` | создание элемента может вернуть ошибку |

## 7. Мини-тест (§7 ТЗ) — перенесён в self-test под `TESTING`

Сценарий из черновика используется как runtime-проверка (`TableRenderer::RunSelfTest`),
вызов — из `TablesNavigator::EnsureNavigatorRoot` под `#ifdef TESTING`, один раз за сессию:

1. таблица 3×3 без объединений, `Auto` — ширины = максимум текста + паддинги;
2. объединение (0,0)-(0,1) с длинным текстом — ширины столбцов 0/1 растут, столбец 2 не меняется;
3. столбец 1 `Fixed`, текст не влезает → перенос по словам, высота строки растёт;
4. сетка: внутренняя линия внутри объединения отсутствует, периметр рисуется.

Self-test логирует измерение строки (мм), итоговые ширины/высоты и bounding box drawing data
(`DBprnt`), затем освобождает IDF-память. В базу проекта ничего не пишется.

## 8. not verified (нужен runtime/визуальная проверка)

- Точное соответствие коэффициента «мм → модельные единицы» при `scale != 1.0` в drawing data
  (проверяется глазами на реальной таблице в MyDraw/IDF).
- **Габарит (bounding box) drawing data не совпадает с расчётным.** Наблюдение self-test
  2026-09-18: таблица 0.0606 × 0.018 модельных единиц, а `APIDb_StopDrawingDataID` вернул
  1.528275 × 0.181667. Сессия из одной линии 10 мм даёт корректный габарит 0.01, значит дело
  не в самом механизме drawing data.
  **Послойная изоляция (прогон 2026-09-18 14:05) локализовала причину — текстовый слой:**
  `fills` = 0.055552 × 0.018 (внутри таблицы), `grid` = 0.060582 × 0.018 (ровно таблица),
  `texts` = 1.528275 × 0.181667 (габарит целиком определяется ими).
  Отсюда: каждый текстовый элемент уносит бокс порядка 1.5 м × 0.18 м, то есть размер бокса
  заметно больше заданного нами `API_TextType.width/height` (35.55 мм × 2.5 мм) либо заданный
  бокс не учитывается вообще.
  Отрицательный результат: явное задание `API_TextType.width/height` (гипотеза «элемент уносит
  дефолтный бокс из `ACAPI_Element_GetDefaults`») габарит не изменило.
  Следующая гипотеза (не проверена): текстовому элементу нужен `memo.paragraphs` — в примере
  `Element_Basics.cpp:3985-4010` для многостилевого текста он выделяется, а без него ArchiCAD
  может брать параметры абзаца/бокса из дефолтов. Проверка: сессия с одним текстовым элементом
  при явно малых `width/height`.
  До выяснения таблицу стоит смотреть в MyDraw: там кадр задаёт окно, а не габарит IDF.
- Поведение `API_TextType.anchor` + `just` для многострочного текста одним элементом.
- Доступность `ACAPI_Element_GetDefaults` внутри открытой сессии drawing data
  (если недоступна — рендерер сообщает ошибку, а не подставляет поля наугад).
- Разрешение имени шрифта через `ACAPI_Attribute_Search` для не-латинских имён
  (`API_Attr_Head.name` — `char[API_AttrNameLen]`; документирован поиск по GUID → Unicode-имени →
  имени; наличие Unicode-поля у встроенных шрифтов не проверено).
- Порядок наложения заливок/линий/текста в готовом IDF при одинаковом порядке создания элементов.
- Поведение в Teamwork и при merge (вне этой задачи).

## 9. Статус реализации

- `Sources/AddOn/table/TableRenderer.hpp/.cpp` — класс (geometry + render).
- `TableRenderer::SetPrototypeContent ()` — демонстрационная таблица 3×3 (объединение в заголовке,
  узкий `Fixed`-столбец). Это временный контент-прототип, пока не подключены строки ведомости
  отделки.
- Self-test — `TableRenderer::RunSelfTest ()`, `#ifdef TESTING`, вызывается из
  `TablesNavigator::EnsureNavigatorRoot` (один раз за сессию, после открытия проекта).
- Обвязка Navigator в `TablesNavigator.cpp`:
  - `OpenView` → окно MyDraw (`APIDb_NewWindowID` + `API_NewWindowPars.userRefId`,
    сброс текущей БД окна → отрисовка → `APIDb_RebuildCurrentDatabaseID`);
  - `CreateIDFStore` → `APIDb_StartDrawingDataID` → `TableRenderer::Draw` → `APIDb_StopDrawingDataID`
    с корректной очисткой и приоритетом ошибки рисования над ошибкой cleanup;
  - `GetElemsForDrawingCheck` больше ничего не рисует (только проверка свежести).
- Runtime-подтверждено (self-test, AC25 Windows Debug, 2026-09-18 12:16): `Draw` err 0,
  `StopDrawingData` err 0; layout: столбец `Fixed` = ровно 0.020, перенос длинного текста дал
  строку высотой 0.011.
- Визуальная проверка таблицы в окне MyDraw — за пользователем (требуется перезапуск Archicad
  со свежим аддоном).
- Содержимое ведомости отделки и генерация строк по модели — следующая задача.
