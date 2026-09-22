# table/TableRenderer — Рендерер таблиц

> Хеш коммита: 493caf5 (2026-09-22).

## Назначение
Отрисовка таблиц для ведомостей: считает геометрию (ширины/высоты/перенос), рисует в уже открытый drawing data store; данные не добывает. [из комментария, TableRenderer.hpp:8-24]

## Границы класса [из комментария, TableRenderer.hpp:11-24]
- Вход: данные ячеек + правила объединения/форматирования
- Рисует в УЖЕ ОТКРЫТЫЙ drawing data store: вложенность сессий Archicad запрещена (APIERR_NESTING), сессией владеет вызывающий
- НЕ добывает данные (модель/формулы/итоги — TablesNavigator)
- Единицы: правила в мм бумаги; наружу — модельные единицы (мм * drawingScale / 1000)

## Ключевые типы [из комментария, TableRenderer.hpp:27-108]
`CellData`, `MergedRange`, `ColumnRule`, `RowRule`, `FillRange`, `TableFormattingRules`; внутренний `CellLayout`.

## Публичный API

| Функция | Назначение |
|---------|------------|
| `SetCells` / `SetFormattingRules` / `SetDrawingScale` | Входные данные [из комментария] |
| `ComputeLayout` | Считает layout без рисования (для reflow/разбиения) [из комментария] — карточка |
| `SetPrototypeContent` | Демо 3x3 (issue #177) [из комментария] |
| `GetTotalWidth/Height`, `GetColumnWidth/RowHeight` | Габариты после ComputeLayout (модельные ед.) [из комментария] |
| `Draw(const API_Coord &origin)` | Рисование; origin — левый нижний угол; layout пересчитывается сам [из комментария] — карточка |
| `RunSelfTest` (TESTING) | Runtime-проверка §7 ТЗ, пишет в DBprnt, в базу не пишет [из комментария, hpp:140-143] |

## Карточки

### `TableRenderer::Draw(const API_Coord &origin) -> GSErrCode`
- Расположение: `Sources/AddOn/table/TableRenderer.cpp:598` (1-based; clangd 597)
- Вызывает: `ComputeLayout` (:600), `DrawFills` (:607), `DrawGrid` (:611), `DrawTexts` (:615). [из callgraph.json]
- Вызывается из: `CreateIDFStore` (TablesNavigator.cpp:543), `RenderScheduleIntoWindow` (TablesNavigator.cpp:207), `RunSelfTest` (:1024). [из callgraph.json]
- Назначение: рисует таблицу в текущий открытый drawing data store. [из комментария]
- Контракт: сессия drawing data должна быть открыта вызывающим (APIDb_StartDrawingDataID); пересчитывает layout при изменении входа. [из комментария]
- Побочные эффекты: **создание элементов в базе проекта** (линии/заливки/тексты через CreateLineElement/CreateFillElement/CreateTextElement). [по коду, hpp:192-205]

### `TableRenderer::ComputeLayout() -> GSErrCode`
- Расположение: `Sources/AddOn/table/TableRenderer.cpp:278` (1-based; clangd 277)
- Назначение: считает геометрию без рисования (columnWidths/rowHeights/CellLayouts в мм). [из комментария + по коду, hpp:163-190]
- Вызывает: `BuildCellLayouts` (:317), `ComputeColumnWidthsMm` (:319), `ComputeRowHeightsMm` (:325), `WrapCellLayouts` (:323), `ResolveFontIndex` (:303), `RangesOverlap` (:298). [из callgraph.json]
- Вызывается из: `CreateIDFStore` (TablesNavigator.cpp:525), `RenderScheduleIntoWindow` (TablesNavigator.cpp:200), `RunSelfTest` (:1000), `Draw` (:600). [из callgraph.json]
- Побочные эффекты: мутирует внутреннюю геометрию; без записи в проект. [по коду]

## Зависимости
- `ACAPinc.h`, текстовые утилиты CommonFunction [по коду]

## Зависимости (используется в)
`TablesNavigator`, ведомости (issue #177) [по коду]

## Инварианты
- Шрифт — индекс атрибута (API_TextType.font) [из комментария, hpp:41-43]
- defaultFontIndex=0/пустое имя → из ACAPI_Element_GetDefaults [из комментария, hpp:89-90, 102-107]
- `defaultFontSize = 2.5` мм [из комментария, hpp:92]
