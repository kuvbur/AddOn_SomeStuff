# table/TableRenderer — Рендерер таблиц

## Назначение
Класс отрисовки таблиц для ведомостей SomeStuff. Считывает геометрию, переносит текст, рисует в уже открытом drawing data store. Не добывает данные (модель, выборку, итогы — задача вызывающей стороны).

## Файлы
- `table/TableRenderer.cpp/hpp` — рендерер (~210 строк в hpp)

## Границы (из комментария в hpp)
- Вход: данные ячеек + правила форматирования
- Класс считает геометрию (ширины столбцов, высоты строк, перенос текста)
- Рисует в УЖЕ ОТКРЫТОМ drawing data store (сессия не принадлежит рендереру)
- НЕ добывает данные: модель, формулы, выборка — задача вызывающей стороны
- Единицы: правила в мм бумаги, layout отдаёт в модельных единицах (мм * drawingScale / 1000)

## Ключевые типы

| Тип | Описание |
|-----|----------|
| `CellData` | Ячейка: row, col, text, fontIndexOverride, hAlignOverride, vAlignOverride |
| `MergedRange` | Объединённый диапазон (rowFrom, rowTo, colFrom, colTo) |
| `ColumnRule` | Ширина столбца (Auto/Fixed, mm) |
| `RowRule` | Высота строки (Auto/Fixed, mm) |
| `FillRange` | Заливка: MergedRange + fillIndex + pens |
| `TableFormattingRules` | Полные правила форматирования (строки, столбцы, merged, fonts, pads) |

### Внутренние (CellLayout и др.)
- `CellLayout` — разложенная ячейка (lines, textWidthMm, textHeightMm, fontIndex)
- `columnWidthsMm`, `rowHeightsMm`, `columnNodesMm`, `rowNodesMm` — геометрия в мм
- `CellLayouts` — разложенные ячейки (rows * cols)

## Публичный API TableRenderer

| Функция | Назначение |
|---------|------------|
| `SetCells` | Установка данных ячеек |
| `SetFormattingRules` | Установка правил форматирования |
| `SetDrawingScale` | Масштаб drawing data |
| `ComputeLayout` | Счёт геометрии без рисования |
| `IsLayoutComputed` | Проверка готовности layout |
| `SetPrototypeContent` | Демо-содержимое 3x3 (issue #177) |
| `GetTotalWidth/Height` | Габариты (model units) |
| `GetColumnWidth/RowHeight` | Размер строки/столбца |
| `Draw` | Рисование в drawing data store (origin — левый нижний угол) |
| `RunSelfTest` (TESTING) | Runtime-проверка §7 ТЗ |

## Зависимости
- `ACAPinc.h`
- Внутренние: шрифтовые и текстовые утилиты из CommonFunction/Helpers

## Зависимости (используется в)
- `table/TablesNavigator.cpp` — каркас ведомостей
- `Roombook` (?) — ведомости отделки

## Инварианты
- `ComputeLayout` должен вызываться перед `Draw`
- Drawing scale: 100 = 1:100, mm → model units: mm * scale / 1000
- Ячейки — sparse-представление (отсутствующая = пустая)
- Шрифт задаётся индексом атрибута (API_TextType.font), не именем
- `defaultFontName` = "" → брать из ACAPI_Element_GetDefaults
- Типографика в мм: `defaultFontSize = 2.5` (как RoombookSettings::fontsize)
