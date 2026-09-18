//------------ kuvbur 2026 ------------
#pragma once
#ifndef TABLE_RENDERER_HPP
    #define TABLE_RENDERER_HPP

    #include "ACAPinc.h"

// Общий класс отрисовки таблиц для ведомостей SomeStuff.
// Проверенная редакция ТЗ: Docs/TableRenderer_AC25.md (AC25, issue #177).
//
// Границы класса:
//  - вход: данные ячеек + отдельно правила объединения/форматирования;
//  - класс сам считает геометрию (ширины столбцов, высоты строк, перенос текста);
//  - класс рисует таблицу в УЖЕ ОТКРЫТЫЙ drawing data store. Открытие/закрытие
//    сессии делает вызывающая сторона (APIDb_StartDrawingDataID /
//    APIDb_StopDrawingDataID): вложенность сессий Archicad запрещает
//    (APIERR_NESTING), поэтому рендерер не владеет сессией.
//  - класс НЕ добывает данные: модель, формулы, выборка строк и накопление
//    итогов — задача вызывающей стороны (см. TablesNavigator).
//
// Единицы: правила форматирования задаются в миллиметрах бумаги (кегль, паддинги,
// фиксированные размеры) — в мм работает SDK-измерение текста. Layout наружу
// отдаётся в модельных единицах: мм * drawingScale / 1000, где drawingScale —
// то же значение, что передано в APIDb_StartDrawingDataID.
namespace TableRenderer {

    enum class HAlign { Default, Left, Center, Right };
    enum class VAlign { Default, Top, Middle, Bottom };

    enum class ColumnWidthMode { Auto, Fixed };
    enum class RowHeightMode { Auto, Fixed };

    // Данные одной ячейки. Sparse-представление: ячейка, отсутствующая в наборе,
    // считается пустой. Для объединённого диапазона используется текст ячейки
    // из его левого верхнего угла.
    struct CellData {
        Int32 row = 0;
        Int32 col = 0;
        GS::UniString text;

        // 0 = взять шрифт из правил. В SDK текст адресует шрифт индексом атрибута
        // (API_TextType.font), поэтому имя шрифта на уровне ячейки не хранится.
        short fontIndexOverride = 0;

        HAlign hAlignOverride = HAlign::Default;
        VAlign vAlignOverride = VAlign::Default;
    };

    // Прямоугольник объединённых ячеек, границы включительно.
    struct MergedRange {
        Int32 rowFrom = 0;
        Int32 rowTo = 0;
        Int32 colFrom = 0;
        Int32 colTo = 0;
    };

    struct ColumnRule {
        ColumnWidthMode mode = ColumnWidthMode::Auto;
        double fixedWidth = 0.0; // мм, используется при mode == Fixed
        double minWidth = 0.0;   // мм
        double maxWidth = 0.0;   // мм, 0 = без ограничения
    };

    struct RowRule {
        RowHeightMode mode = RowHeightMode::Auto;
        double fixedHeight = 0.0; // мм
        double minHeight = 0.0;   // мм
    };

    // Заливка диапазона: задаётся списком диапазонов, как и объединения.
    // Невалидный fillIndex (APIInvalidAttributeIndex) означает «не заливать».
    struct FillRange {
        MergedRange range;
        API_AttributeIndex fillIndex = APIInvalidAttributeIndex;
        short fillPen = 0;
        short contourPen = 0;
    };

    struct TableFormattingRules {
        Int32 rows = 0;
        Int32 cols = 0;

        GS::Array<MergedRange> mergedRanges;
        GS::Array<ColumnRule> columns; // ожидаемый размер == cols
        GS::Array<RowRule> rowRules;   // ожидаемый размер == rows
        GS::Array<FillRange> fillRanges;

        // Типографика (мм). defaultFontIndex == 0 и пустое имя -> взять из
        // ACAPI_Element_GetDefaults (без выдуманных значений по умолчанию).
        short defaultFontIndex = 0;
        GS::UniString defaultFontName;
        double defaultFontSize = 2.5;                   // мм (как RoombookSettings::fontsize)
        unsigned short defaultFaceBits = APIFace_Plain; // APIFace_*
        double defaultLineSpacing = 1.0;                // множитель высоты строки текста

        HAlign defaultHAlign = HAlign::Left;
        VAlign defaultVAlign = VAlign::Middle;

        double cellPaddingH = 1.0; // мм
        double cellPaddingV = 0.5; // мм

        // Атрибуты отрисовки. 0 = взять из ACAPI_Element_GetDefaults;
        // слои и перья вручную не выдумываются.
        short layerIndex = 0;
        short gridPenIndex = 0;
        short textPenIndex = 0;
        short lineTypeIndex = 0;
    };

    class TableRenderer {
      public:
        void SetCells (const GS::Array<CellData> &cells);
        void SetFormattingRules (const TableFormattingRules &rules);

        // Масштаб drawing data (то же значение, что параметр dScale у
        // APIDb_StartDrawingDataID; для 1:100 передаётся 100).
        void SetDrawingScale (double scale);

        // Считает layout без рисования: вызывающей стороне нужен итоговый габарит
        // заранее (reflow/разбиение на блоки/повтор заголовка).
        GSErrCode ComputeLayout ();
        bool IsLayoutComputed () const;

        // Демонстрационное содержимое-прототип: 3x3 с объединением в заголовке и
        // узким Fixed-столбцом. Нужно, пока не подключены строки реальной ведомости
        // отделки (issue #177); даёт видимую таблицу в окне ведомости и материал self-test.
        void SetPrototypeContent ();

        // Доступно после успешного ComputeLayout (модельные единицы).
        double GetTotalWidth () const;
        double GetTotalHeight () const;
        double GetColumnWidth (Int32 col) const;
        double GetRowHeight (Int32 row) const;

        // Рисует таблицу в текущий открытый drawing data store,
        // origin — левый нижний угол таблицы. Layout пересчитывается сам,
        // если входные данные менялись.
        GSErrCode Draw (const API_Coord &origin);

    #ifdef TESTING
        // Runtime-проверка сценария §7 ТЗ (пишет в DBprnt, в базу проекта не пишет).
        static GSErrCode RunSelfTest ();
    #endif

      private:
        // Разложенная ячейка: строки после переноса и вычисленные отступы.
        struct CellLayout {
            GS::Array<GS::UniString> lines;
            double textWidthMm = 0.0;  // самая широкая строка
            double textHeightMm = 0.0; // lines * fontSize * lineSpacing
            short fontIndex = 0;
            double fontSizeMm = 2.5;
            HAlign hAlign = HAlign::Left;
            VAlign vAlign = VAlign::Middle;
            bool present = false;
        };

        GS::Array<CellData> cells;
        TableFormattingRules rules;
        double drawingScale = 1.0;
        bool layoutComputed = false;

        // Геометрия в мм: узлы сетки — накопленные размеры от левого/верхнего края.
        GS::Array<double> columnWidthsMm;  // размер cols
        GS::Array<double> rowHeightsMm;    // размер rows
        GS::Array<double> columnNodesMm;   // размер cols + 1
        GS::Array<double> rowNodesMm;      // размер rows + 1
        GS::Array<CellLayout> cellLayouts; // размер rows * cols

        double ToModel (double mm) const;
        double TotalWidthMm () const;
        double TotalHeightMm () const;

        Int32 SlotIndex (Int32 row, Int32 col) const;
        CellLayout &SlotLayout (Int32 row, Int32 col);
        const CellLayout &SlotLayout (Int32 row, Int32 col) const;
        const CellData *FindCell (Int32 row, Int32 col) const;
        bool IsSlotCoveredByMerge (Int32 row, Int32 col, const MergedRange *&range) const;

        short ResolveFontIndex () const;
        double MeasureWidthMm (const GS::UniString &text, short fontIndex, double fontSizeMm) const;
        GS::Array<GS::UniString> WrapText (const GS::UniString &text,
                                           short fontIndex,
                                           double fontSizeMm,
                                           double maxWidthMm) const;

        GSErrCode ComputeColumnWidthsMm ();
        GSErrCode ComputeRowHeightsMm ();
        void BuildCellLayouts (short fontIndex, double fontSizeMm);
        void WrapCellLayouts ();

        GSErrCode DrawFills (const API_Coord &origin) const;
        GSErrCode DrawGrid (const API_Coord &origin) const;
        GSErrCode DrawTexts (const API_Coord &origin) const;

        GSErrCode CreateLineElement (const API_Coord &beg, const API_Coord &end) const;
        GSErrCode CreateFillElement (const API_Coord &bottomLeft,
                                     double widthMm,
                                     double heightMm,
                                     const FillRange &fill) const;
        GSErrCode CreateTextElement (const CellLayout &layout,
                                     const GS::UniString &line,
                                     const API_Coord &bottomLeft,
                                     double boxWidthMm,
                                     double lineHeightMm) const;
    };

} // namespace TableRenderer

#endif
