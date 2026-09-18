//------------ kuvbur 2026 ------------
#include <math.h>

#include "TableRenderer.hpp"

#include "CommonFunction.hpp"

// Реализация общего класса отрисовки таблиц.
// Проверенная редакция ТЗ и источники SDK: Docs/TableRenderer_AC25.md (issue #177).
namespace TableRenderer {

    namespace {

        constexpr double kMinWidthMm = 0.01;
        constexpr Int32 kMaxWrappedLines = 512; // защита от вырожденной ширины столбца

        API_JustID ToApiJust (HAlign align) {
            switch (align) {
            case HAlign::Center:
                return APIJust_Center;
            case HAlign::Right:
                return APIJust_Right;
            case HAlign::Left:
            default:
                // HAlign::Default сюда не доходит: он разрешается в правилах до отрисовки.
                return APIJust_Left;
            }
        }

        HAlign ResolveHAlign (HAlign overrideValue, HAlign defaultValue) {
            return (overrideValue == HAlign::Default) ? defaultValue : overrideValue;
        }

        VAlign ResolveVAlign (VAlign overrideValue, VAlign defaultValue) {
            return (overrideValue == VAlign::Default) ? defaultValue : overrideValue;
        }

        bool RangesOverlap (const MergedRange &a, const MergedRange &b) {
            return !(a.rowTo < b.rowFrom || b.rowTo < a.rowFrom || a.colTo < b.colFrom || b.colTo < a.colFrom);
        }

        double VerticalShiftFactor (VAlign align) {
            switch (align) {
            case VAlign::Top:
                return 0.0;
            case VAlign::Bottom:
                return 1.0;
            case VAlign::Middle:
            default:
                return 0.5;
            }
        }

        double HorizontalShiftFactor (HAlign align) {
            switch (align) {
            case HAlign::Center:
                return 0.5;
            case HAlign::Right:
                return 1.0;
            case HAlign::Left:
            default:
                return 0.0;
            }
        }

        // Self-test берёт первый атрибут заливки проекта: гадать конкретный индекс
        // (например «сплошная заливка») нельзя, он не документирован.
        API_AttributeIndex FindFirstFillIndex () {
            API_AttributeIndex count = 0;
            if (ACAPI_Attribute_GetNum (API_FilltypeID, &count) != NoError || count < 1)
                return APIInvalidAttributeIndex;

            return 1;
        }

    } // namespace

    // -------------------------------------------------------------------------
    // Входные данные
    // -------------------------------------------------------------------------
    void TableRenderer::SetCells (const GS::Array<CellData> &newCells) {
        cells = newCells;
        layoutComputed = false;
    }

    void TableRenderer::SetFormattingRules (const TableFormattingRules &newRules) {
        rules = newRules;
        layoutComputed = false;
    }

    void TableRenderer::SetDrawingScale (double scale) { drawingScale = (scale > 0.0) ? scale : 1.0; }

    bool TableRenderer::IsLayoutComputed () const { return layoutComputed; }

    // -------------------------------------------------------------------------
    // Единицы: правила в мм, наружу — модельные единицы drawing data.
    // -------------------------------------------------------------------------
    double TableRenderer::ToModel (double mm) const { return mm * drawingScale / 1000.0; }

    double TableRenderer::TotalWidthMm () const {
        double total = 0.0;
        for (const double width : columnWidthsMm)
            total += width;
        return total;
    }

    double TableRenderer::TotalHeightMm () const {
        double total = 0.0;
        for (const double height : rowHeightsMm)
            total += height;
        return total;
    }

    double TableRenderer::GetTotalWidth () const { return ToModel (TotalWidthMm ()); }

    double TableRenderer::GetTotalHeight () const { return ToModel (TotalHeightMm ()); }

    double TableRenderer::GetColumnWidth (Int32 col) const {
        if (col < 0 || static_cast<USize> (col) >= columnWidthsMm.GetSize ())
            return 0.0;
        return ToModel (columnWidthsMm[col]);
    }

    double TableRenderer::GetRowHeight (Int32 row) const {
        if (row < 0 || static_cast<USize> (row) >= rowHeightsMm.GetSize ())
            return 0.0;
        return ToModel (rowHeightsMm[row]);
    }

    // -------------------------------------------------------------------------
    // Вспомогательное
    // -------------------------------------------------------------------------
    Int32 TableRenderer::SlotIndex (Int32 row, Int32 col) const { return row * rules.cols + col; }

    TableRenderer::CellLayout &TableRenderer::SlotLayout (Int32 row, Int32 col) {
        return cellLayouts[SlotIndex (row, col)];
    }

    const TableRenderer::CellLayout &TableRenderer::SlotLayout (Int32 row, Int32 col) const {
        return cellLayouts[SlotIndex (row, col)];
    }

    const CellData *TableRenderer::FindCell (Int32 row, Int32 col) const {
        for (const CellData &cell : cells) {
            if (cell.row == row && cell.col == col)
                return &cell;
        }
        return nullptr;
    }

    bool TableRenderer::IsSlotCoveredByMerge (Int32 row, Int32 col, const MergedRange *&range) const {
        for (const MergedRange &merged : rules.mergedRanges) {
            const bool inside =
                (row >= merged.rowFrom && row <= merged.rowTo && col >= merged.colFrom && col <= merged.colTo);
            if (!inside)
                continue;

            // Левый верхний угол объединения — это его содержимое, остальные ячейки диапазона
            // считаются покрытыми: свой текст они не рисуют.
            if (row == merged.rowFrom && col == merged.colFrom)
                continue;

            range = &merged;
            return true;
        }
        return false;
    }

    short TableRenderer::ResolveFontIndex () const {
        if (rules.defaultFontIndex > 0)
            return rules.defaultFontIndex;

        if (!rules.defaultFontName.IsEmpty ()) {
            GS::UniString fontName = rules.defaultFontName; // GetFontIndex принимает неконстантную ссылку
            const short resolved = GetFontIndex (fontName);
            if (resolved > 0)
                return resolved;
#ifdef TESTING
            DBprnt ("TableRenderer::ResolveFontIndex", "font by name not found: " + rules.defaultFontName);
#endif
        }

        // Последний источник — дефолт текста из проекта: своё значение шрифта не выдумываем.
        API_Element element;
        BNZeroMemory (&element, sizeof (API_Element));
        element.header.typeID = API_TextID;
        if (ACAPI_Element_GetDefaults (&element, nullptr) == NoError && element.text.font > 0)
            return element.text.font;

        return 0;
    }

    double TableRenderer::MeasureWidthMm (const GS::UniString &text, short fontIndex, double fontSizeMm) const {
        if (text.IsEmpty ())
            return 0.0;

        GS::UniString measurable = text; // GetTextWidth принимает неконстантную ссылку
        return GetTextWidth (fontIndex, fontSizeMm, measurable);
    }

    // Жадный перенос: сначала по словам, при необходимости — по символам.
    // Ширина каждой строки не превышает maxWidthMm.
    GS::Array<GS::UniString> TableRenderer::WrapText (const GS::UniString &text,
                                                      short fontIndex,
                                                      double fontSizeMm,
                                                      double maxWidthMm) const {
        GS::Array<GS::UniString> lines;
        if (text.IsEmpty ())
            return lines;

        if (maxWidthMm <= kMinWidthMm) {
            lines.Push (text);
            return lines;
        }

        GS::Array<GS::UniString> paragraphs;
        text.Split (GS::UniString ("\n"), &paragraphs);

        GS::UniString space = " ";
        for (const GS::UniString &paragraph : paragraphs) {
            GS::Array<GS::UniString> words;
            paragraph.Split (GS::UniString (" "), &words);

            GS::UniString currentLine;
            for (const GS::UniString &word : words) {
                if (word.IsEmpty ())
                    continue;

                // Слово шире полосы — режем по символам, иначе строка никогда не влезет.
                if (MeasureWidthMm (word, fontIndex, fontSizeMm) > maxWidthMm) {
                    if (!currentLine.IsEmpty ()) {
                        lines.Push (currentLine);
                        currentLine.Clear ();
                    }

                    GS::UniString chunk;
                    for (USize i = 0; i < word.GetLength () && lines.GetSize () < static_cast<USize> (kMaxWrappedLines);
                         ++i) {
                        GS::UniString candidate = chunk + word[i];
                        if (!chunk.IsEmpty () && MeasureWidthMm (candidate, fontIndex, fontSizeMm) > maxWidthMm) {
                            lines.Push (chunk);
                            chunk.Clear ();
                        }
                        chunk += word[i];
                    }
                    currentLine = chunk;
                    continue;
                }

                GS::UniString candidate = currentLine.IsEmpty () ? word : currentLine + space + word;
                if (MeasureWidthMm (candidate, fontIndex, fontSizeMm) > maxWidthMm && !currentLine.IsEmpty ()) {
                    lines.Push (currentLine);
                    currentLine = word;
                } else {
                    currentLine = candidate;
                }

                if (lines.GetSize () >= static_cast<USize> (kMaxWrappedLines))
                    break;
            }

            if (!currentLine.IsEmpty ())
                lines.Push (currentLine);

            if (lines.GetSize () >= static_cast<USize> (kMaxWrappedLines))
                break;
        }

        if (lines.IsEmpty ())
            lines.Push (text);

        return lines;
    }

    // -------------------------------------------------------------------------
    // Layout
    // -------------------------------------------------------------------------
    GSErrCode TableRenderer::ComputeLayout () {
        layoutComputed = false;

        if (rules.rows <= 0 || rules.cols <= 0)
            return APIERR_BADPARS;

        // Ячейки вне таблицы и битые объединения — ошибка входных данных:
        // рендерер их не «угадывает» (см. §4.1 ТЗ).
        for (const CellData &cell : cells) {
            if (cell.row < 0 || cell.row >= rules.rows || cell.col < 0 || cell.col >= rules.cols)
                return APIERR_BADPARS;
        }

        for (USize i = 0; i < rules.mergedRanges.GetSize (); ++i) {
            const MergedRange &merged = rules.mergedRanges[i];
            if (merged.rowFrom < 0 || merged.colFrom < 0 || merged.rowTo >= rules.rows || merged.colTo >= rules.cols ||
                merged.rowFrom > merged.rowTo || merged.colFrom > merged.colTo)
                return APIERR_BADPARS;

            for (USize j = i + 1; j < rules.mergedRanges.GetSize (); ++j) {
                if (RangesOverlap (merged, rules.mergedRanges[j]))
                    return APIERR_BADPARS;
            }
        }

        const short fontIndex = ResolveFontIndex ();
        if (fontIndex <= 0) {
#ifdef TESTING
            DBprnt ("TableRenderer::ComputeLayout err", "font index unresolved");
#endif
            return APIERR_BADID;
        }

#ifdef TESTING
        // Шрифт берётся из дефолтов текста ТЕКУЩЕЙ базы: в окне ведомости layout нужно
        // считать до сброса БД окна, иначе здесь окажется шрифт пустого окна.
        DBprnt (static_cast<double> (fontIndex), "TableRenderer layout font index");
#endif

        BuildCellLayouts (fontIndex, rules.defaultFontSize);

        GSErrCode err = ComputeColumnWidthsMm ();
        if (err != NoError)
            return err;

        WrapCellLayouts ();

        err = ComputeRowHeightsMm ();
        if (err != NoError)
            return err;

        columnNodesMm.SetSize (static_cast<USize> (rules.cols + 1));
        columnNodesMm[0] = 0.0;
        for (Int32 col = 0; col < rules.cols; ++col)
            columnNodesMm[col + 1] = columnNodesMm[col] + columnWidthsMm[col];

        rowNodesMm.SetSize (static_cast<USize> (rules.rows + 1));
        rowNodesMm[0] = 0.0;
        for (Int32 row = 0; row < rules.rows; ++row)
            rowNodesMm[row + 1] = rowNodesMm[row] + rowHeightsMm[row];

        layoutComputed = true;
        return NoError;
    }

    // Первый проход: типографика ячейки и «естественная» ширина текста без переноса.
    void TableRenderer::BuildCellLayouts (short fontIndex, double fontSizeMm) {
        cellLayouts.SetSize (static_cast<USize> (rules.rows * rules.cols));

        for (Int32 row = 0; row < rules.rows; ++row) {
            for (Int32 col = 0; col < rules.cols; ++col) {
                CellLayout &slot = SlotLayout (row, col);
                slot = CellLayout ();

                const CellData *cell = FindCell (row, col);
                if (cell == nullptr)
                    continue;

                slot.present = true;
                slot.fontIndex = (cell->fontIndexOverride > 0) ? cell->fontIndexOverride : fontIndex;
                slot.fontSizeMm = fontSizeMm;
                slot.hAlign = ResolveHAlign (cell->hAlignOverride, rules.defaultHAlign);
                slot.vAlign = ResolveVAlign (cell->vAlignOverride, rules.defaultVAlign);

                GS::Array<GS::UniString> paragraphs;
                cell->text.Split (GS::UniString ("\n"), &paragraphs);
                for (const GS::UniString &paragraph : paragraphs) {
                    slot.lines.Push (paragraph);
                    const double width = MeasureWidthMm (paragraph, slot.fontIndex, slot.fontSizeMm);
                    if (width > slot.textWidthMm)
                        slot.textWidthMm = width;
                }
            }
        }
    }

    // Ширины столбцов (§4.2 ТЗ): максимум текста по столбцу, затем распределение
    // под многостолбцовые объединения, затем клип min/max. Fixed не участвует.
    GSErrCode TableRenderer::ComputeColumnWidthsMm () {
        columnWidthsMm.SetSize (static_cast<USize> (rules.cols));

        for (Int32 col = 0; col < rules.cols; ++col) {
            const ColumnRule rule =
                (col < static_cast<Int32> (rules.columns.GetSize ())) ? rules.columns[col] : ColumnRule ();

            if (rule.mode == ColumnWidthMode::Fixed) {
                columnWidthsMm[col] = (rule.fixedWidth > 0.0) ? rule.fixedWidth : kMinWidthMm;
                continue;
            }

            double width = 0.0;
            for (Int32 row = 0; row < rules.rows; ++row) {
                const MergedRange *merged = nullptr;
                if (IsSlotCoveredByMerge (row, col, merged))
                    continue;

                const CellLayout &slot = SlotLayout (row, col);
                if (!slot.present)
                    continue;

                // Ячейка, растянутая на несколько столбцов, учитывается отдельным шагом.
                bool spansColumns = false;
                for (const MergedRange &range : rules.mergedRanges) {
                    if (range.rowFrom == row && range.colFrom == col && range.colTo > col) {
                        spansColumns = true;
                        break;
                    }
                }
                if (spansColumns)
                    continue;

                const double required = slot.textWidthMm + 2.0 * rules.cellPaddingH;
                if (required > width)
                    width = required;
            }

            columnWidthsMm[col] = (width > kMinWidthMm) ? width : kMinWidthMm;
        }

        // Объединения, растянутые по столбцам: недостающую ширину раздаём пропорционально
        // текущим ширинам, не трогая Fixed-столбцы.
        for (const MergedRange &range : rules.mergedRanges) {
            if (range.colTo <= range.colFrom)
                continue;

            const CellLayout &slot = SlotLayout (range.rowFrom, range.colFrom);
            if (!slot.present)
                continue;

            const double required = slot.textWidthMm + 2.0 * rules.cellPaddingH;

            double current = 0.0;
            double flexible = 0.0;
            for (Int32 col = range.colFrom; col <= range.colTo; ++col) {
                current += columnWidthsMm[col];
                const ColumnRule rule =
                    (col < static_cast<Int32> (rules.columns.GetSize ())) ? rules.columns[col] : ColumnRule ();
                if (rule.mode != ColumnWidthMode::Fixed)
                    flexible += columnWidthsMm[col];
            }

            const double deficit = required - current;
            if (deficit <= 0.0 || flexible <= 0.0)
                continue; // все столбцы диапазона Fixed — текст будет перенесён, ширина не растёт

            for (Int32 col = range.colFrom; col <= range.colTo; ++col) {
                const ColumnRule rule =
                    (col < static_cast<Int32> (rules.columns.GetSize ())) ? rules.columns[col] : ColumnRule ();
                if (rule.mode == ColumnWidthMode::Fixed)
                    continue;
                columnWidthsMm[col] += deficit * (columnWidthsMm[col] / flexible);
            }
        }

        // Финальный клип min/max (Fixed тоже клипуется: границы заданы явно).
        for (Int32 col = 0; col < rules.cols; ++col) {
            const ColumnRule rule =
                (col < static_cast<Int32> (rules.columns.GetSize ())) ? rules.columns[col] : ColumnRule ();
            if (rule.minWidth > 0.0 && columnWidthsMm[col] < rule.minWidth)
                columnWidthsMm[col] = rule.minWidth;
            if (rule.maxWidth > 0.0 && columnWidthsMm[col] > rule.maxWidth)
                columnWidthsMm[col] = rule.maxWidth;
        }

        return NoError;
    }

    // Второй проход: перенос текста в уже посчитанную ширину ячейки.
    void TableRenderer::WrapCellLayouts () {
        for (Int32 row = 0; row < rules.rows; ++row) {
            for (Int32 col = 0; col < rules.cols; ++col) {
                CellLayout &slot = SlotLayout (row, col);
                if (!slot.present)
                    continue;

                double available = 0.0;
                Int32 colTo = col;
                for (const MergedRange &range : rules.mergedRanges) {
                    if (range.rowFrom == row && range.colFrom == col) {
                        colTo = range.colTo;
                        break;
                    }
                }
                for (Int32 c = col; c <= colTo; ++c)
                    available += columnWidthsMm[c];

                available -= 2.0 * rules.cellPaddingH;

                GS::Array<GS::UniString> paragraphs = slot.lines; // BuildCellLayouts положил туда абзацы
                GS::Array<GS::UniString> wrapped;
                for (const GS::UniString &paragraph : paragraphs) {
                    const GS::Array<GS::UniString> paragraphLines =
                        WrapText (paragraph, slot.fontIndex, slot.fontSizeMm, available);
                    for (const GS::UniString &line : paragraphLines)
                        wrapped.Push (line);
                }

                slot.lines = wrapped;

                double widest = 0.0;
                for (const GS::UniString &line : slot.lines) {
                    const double width = MeasureWidthMm (line, slot.fontIndex, slot.fontSizeMm);
                    if (width > widest)
                        widest = width;
                }
                slot.textWidthMm = widest;
                slot.textHeightMm =
                    static_cast<double> (slot.lines.GetSize ()) * slot.fontSizeMm * rules.defaultLineSpacing;
            }
        }
    }

    // Высоты строк (§4.3 ТЗ): после ширин столбцов, с учётом числа строк текста.
    GSErrCode TableRenderer::ComputeRowHeightsMm () {
        rowHeightsMm.SetSize (static_cast<USize> (rules.rows));

        for (Int32 row = 0; row < rules.rows; ++row) {
            const RowRule rule =
                (row < static_cast<Int32> (rules.rowRules.GetSize ())) ? rules.rowRules[row] : RowRule ();

            if (rule.mode == RowHeightMode::Fixed) {
                rowHeightsMm[row] = (rule.fixedHeight > 0.0) ? rule.fixedHeight : rules.defaultFontSize;
                continue;
            }

            double height = 0.0;
            for (Int32 col = 0; col < rules.cols; ++col) {
                const MergedRange *merged = nullptr;
                if (IsSlotCoveredByMerge (row, col, merged))
                    continue;

                const CellLayout &slot = SlotLayout (row, col);
                if (!slot.present)
                    continue;

                // Многострочные объединения растягивают сумму высот — учитываются отдельно ниже.
                bool spansRows = false;
                for (const MergedRange &range : rules.mergedRanges) {
                    if (range.rowFrom == row && range.colFrom == col && range.rowTo > row) {
                        spansRows = true;
                        break;
                    }
                }
                if (spansRows)
                    continue;

                const double required = slot.textHeightMm + 2.0 * rules.cellPaddingV;
                if (required > height)
                    height = required;
            }

            if (height <= kMinWidthMm)
                height = rules.defaultFontSize * rules.defaultLineSpacing + 2.0 * rules.cellPaddingV;
            if (rule.minHeight > 0.0 && height < rule.minHeight)
                height = rule.minHeight;

            rowHeightsMm[row] = height;
        }

        // Многострочное объединение: если суммы охваченных строк не хватает,
        // растягиваем Auto-строки пропорционально (Fixed-строки не трогаем).
        for (const MergedRange &range : rules.mergedRanges) {
            if (range.rowTo <= range.rowFrom)
                continue;

            const CellLayout &slot = SlotLayout (range.rowFrom, range.colFrom);
            if (!slot.present)
                continue;

            const double required = slot.textHeightMm + 2.0 * rules.cellPaddingV;

            double current = 0.0;
            double flexible = 0.0;
            for (Int32 row = range.rowFrom; row <= range.rowTo; ++row) {
                current += rowHeightsMm[row];
                const RowRule rule =
                    (row < static_cast<Int32> (rules.rowRules.GetSize ())) ? rules.rowRules[row] : RowRule ();
                if (rule.mode != RowHeightMode::Fixed)
                    flexible += rowHeightsMm[row];
            }

            const double deficit = required - current;
            if (deficit <= 0.0 || flexible <= 0.0)
                continue;

            for (Int32 row = range.rowFrom; row <= range.rowTo; ++row) {
                const RowRule rule =
                    (row < static_cast<Int32> (rules.rowRules.GetSize ())) ? rules.rowRules[row] : RowRule ();
                if (rule.mode == RowHeightMode::Fixed)
                    continue;
                rowHeightsMm[row] += deficit * (rowHeightsMm[row] / flexible);
            }
        }

        return NoError;
    }

    // -------------------------------------------------------------------------
    // Отрисовка
    // -------------------------------------------------------------------------
    GSErrCode TableRenderer::Draw (const API_Coord &origin) {
        if (!layoutComputed) {
            const GSErrCode err = ComputeLayout ();
            if (err != NoError)
                return err;
        }

        // Порядок важен: заливка не должна перекрывать сетку и текст.
        // Внутри сессии drawing data порядок = порядок создания элементов.
        GSErrCode err = DrawFills (origin);
        if (err != NoError)
            return err;

        err = DrawGrid (origin);
        if (err != NoError)
            return err;

        return DrawTexts (origin);
    }

    GSErrCode TableRenderer::DrawFills (const API_Coord &origin) const {
        for (const FillRange &fill : rules.fillRanges) {
            if (fill.fillIndex == APIInvalidAttributeIndex)
                continue; // заливка не задана вызывающей стороной — не рисуем

            const MergedRange &range = fill.range;
            if (range.rowFrom < 0 || range.colFrom < 0 || range.rowTo >= rules.rows || range.colTo >= rules.cols ||
                range.rowFrom > range.rowTo || range.colFrom > range.colTo)
                return APIERR_BADPARS;

            const API_Coord bottomLeft = {origin.x + ToModel (columnNodesMm[range.colFrom]),
                                          origin.y + ToModel (TotalHeightMm () - rowNodesMm[range.rowTo + 1])};
            const double widthMm = columnNodesMm[range.colTo + 1] - columnNodesMm[range.colFrom];
            const double heightMm = rowNodesMm[range.rowTo + 1] - rowNodesMm[range.rowFrom];

            const GSErrCode err = CreateFillElement (bottomLeft, widthMm, heightMm, fill);
            if (err != NoError)
                return err;
        }

        return NoError;
    }

    GSErrCode TableRenderer::DrawGrid (const API_Coord &origin) const {
        const double tableHeightMm = TotalHeightMm ();

        const auto xAt = [&] (Int32 node) { return origin.x + ToModel (columnNodesMm[node]); };
        const auto yTop = [&] (Int32 row) { return origin.y + ToModel (tableHeightMm - rowNodesMm[row]); };
        const auto yBottom = [&] (Int32 row) { return origin.y + ToModel (tableHeightMm - rowNodesMm[row + 1]); };

        // Проходит ли отрезок строго внутри какого-нибудь объединения.
        const auto insideVerticalInner = [&] (Int32 boundary, Int32 row) {
            for (const MergedRange &range : rules.mergedRanges) {
                if (range.colFrom <= boundary - 1 && range.colTo >= boundary && range.rowFrom <= row &&
                    range.rowTo >= row)
                    return true;
            }
            return false;
        };
        const auto insideHorizontalInner = [&] (Int32 boundary, Int32 col) {
            for (const MergedRange &range : rules.mergedRanges) {
                if (range.rowFrom <= boundary - 1 && range.rowTo >= boundary && range.colFrom <= col &&
                    range.colTo >= col)
                    return true;
            }
            return false;
        };

        // Горизонтальные линии: внешние границы рисуются целиком, внутренние — по столбцам.
        for (Int32 node = 0; node <= rules.rows; ++node) {
            const double y = origin.y + ToModel (tableHeightMm - rowNodesMm[node]);

            if (node == 0 || node == rules.rows) {
                const GSErrCode err = CreateLineElement ({xAt (0), y}, {xAt (rules.cols), y});
                if (err != NoError)
                    return err;
                continue;
            }

            for (Int32 col = 0; col < rules.cols; ++col) {
                if (insideHorizontalInner (node, col))
                    continue;

                const GSErrCode err = CreateLineElement ({xAt (col), y}, {xAt (col + 1), y});
                if (err != NoError)
                    return err;
            }
        }

        // Вертикальные линии: внешние границы целиком, внутренние — по строкам.
        for (Int32 node = 0; node <= rules.cols; ++node) {
            const double x = xAt (node);

            if (node == 0 || node == rules.cols) {
                const GSErrCode err = CreateLineElement ({x, yBottom (0)}, {x, yTop (rules.rows - 1)});
                if (err != NoError)
                    return err;
                continue;
            }

            for (Int32 row = 0; row < rules.rows; ++row) {
                if (insideVerticalInner (node, row))
                    continue;

                const GSErrCode err = CreateLineElement ({x, yBottom (row)}, {x, yTop (row)});
                if (err != NoError)
                    return err;
            }
        }

        return NoError;
    }

    GSErrCode TableRenderer::DrawTexts (const API_Coord &origin) const {
        const double tableHeightMm = TotalHeightMm ();

        for (Int32 row = 0; row < rules.rows; ++row) {
            for (Int32 col = 0; col < rules.cols; ++col) {
                const MergedRange *covered = nullptr;
                if (IsSlotCoveredByMerge (row, col, covered))
                    continue;

                const CellLayout &layout = SlotLayout (row, col);
                if (!layout.present || layout.lines.IsEmpty ())
                    continue;

                Int32 rowTo = row;
                Int32 colTo = col;
                for (const MergedRange &range : rules.mergedRanges) {
                    if (range.rowFrom == row && range.colFrom == col) {
                        rowTo = range.rowTo;
                        colTo = range.colTo;
                        break;
                    }
                }

                const double cellLeftMm = columnNodesMm[col];
                const double cellWidthMm = columnNodesMm[colTo + 1] - cellLeftMm;
                const double cellBottomMm = tableHeightMm - rowNodesMm[rowTo + 1];
                const double cellHeightMm = rowNodesMm[rowTo + 1] - rowNodesMm[row];

                const double innerWidthMm = cellWidthMm - 2.0 * rules.cellPaddingH;
                const double innerHeightMm = cellHeightMm - 2.0 * rules.cellPaddingV;
                const double lineHeightMm = layout.fontSizeMm * rules.defaultLineSpacing;
                const double blockHeightMm = static_cast<double> (layout.lines.GetSize ()) * lineHeightMm;

                const double vShift = (innerHeightMm - blockHeightMm) * VerticalShiftFactor (layout.vAlign);

                for (USize index = 0; index < layout.lines.GetSize (); ++index) {
                    const GS::UniString &line = layout.lines[index];
                    const double lineWidthMm = MeasureWidthMm (line, layout.fontIndex, layout.fontSizeMm);
                    const double hShift = (innerWidthMm - lineWidthMm) * HorizontalShiftFactor (layout.hAlign);

                    const double xMm = cellLeftMm + rules.cellPaddingH + hShift;
                    // Строки идут сверху вниз: первая строка блока — самая верхняя.
                    const double lineBottomMm =
                        cellBottomMm + rules.cellPaddingV + vShift +
                        (static_cast<double> (layout.lines.GetSize () - index - 1)) * lineHeightMm;

                    const API_Coord bottomLeft = {origin.x + ToModel (xMm), origin.y + ToModel (lineBottomMm)};

                    const GSErrCode err = CreateTextElement (layout, line, bottomLeft, innerWidthMm, lineHeightMm);
                    if (err != NoError)
                        return err;
                }
            }
        }

        return NoError;
    }

    GSErrCode TableRenderer::CreateLineElement (const API_Coord &beg, const API_Coord &end) const {
        API_Element element;
        BNZeroMemory (&element, sizeof (API_Element));
        element.header.typeID = API_LineID;

        GSErrCode err = ACAPI_Element_GetDefaults (&element, nullptr);
        if (err != NoError)
            return err;

        element.line.begC = beg;
        element.line.endC = end;

        if (rules.layerIndex != 0)
            element.header.layer = rules.layerIndex;
        if (rules.gridPenIndex != 0)
            element.line.linePen.penIndex = rules.gridPenIndex;
        if (rules.lineTypeIndex != 0)
            element.line.ltypeInd = rules.lineTypeIndex;

        return ACAPI_Element_Create (&element, nullptr);
    }

    GSErrCode TableRenderer::CreateFillElement (const API_Coord &bottomLeft,
                                                double widthMm,
                                                double heightMm,
                                                const FillRange &fill) const {
        API_Element element;
        API_ElementMemo memo;
        BNZeroMemory (&element, sizeof (API_Element));
        BNZeroMemory (&memo, sizeof (API_ElementMemo));

        element.header.typeID = API_HatchID;

        GSErrCode err = ACAPI_Element_GetDefaults (&element, nullptr);
        if (err != NoError)
            return err;

        if (rules.layerIndex != 0)
            element.header.layer = rules.layerIndex;

        element.hatch.fillInd = fill.fillIndex;
        if (fill.fillPen != 0)
            element.hatch.fillPen.penIndex = fill.fillPen;
        if (fill.contourPen != 0)
            element.hatch.contPen.penIndex = fill.contourPen;
        if (rules.lineTypeIndex != 0)
            element.hatch.ltypeInd = rules.lineTypeIndex;

        // Прямоугольник заливки: 5 точек, где последняя замыкает контур (см. Element_Test).
        element.hatch.poly.nCoords = 5;
        element.hatch.poly.nSubPolys = 1;
        element.hatch.poly.nArcs = 0;

        const double width = ToModel (widthMm);
        const double height = ToModel (heightMm);

        memo.coords =
            reinterpret_cast<API_Coord **> (BMhAllClear ((element.hatch.poly.nCoords + 1) * sizeof (API_Coord)));
        memo.pends = reinterpret_cast<Int32 **> (BMhAllClear ((element.hatch.poly.nSubPolys + 1) * sizeof (Int32)));

        if (memo.coords == nullptr || memo.pends == nullptr) {
            ACAPI_DisposeElemMemoHdls (&memo);
            return APIERR_MEMFULL;
        }

        (*memo.coords)[1] = bottomLeft;
        (*memo.coords)[2] = {bottomLeft.x + width, bottomLeft.y};
        (*memo.coords)[3] = {bottomLeft.x + width, bottomLeft.y + height};
        (*memo.coords)[4] = {bottomLeft.x, bottomLeft.y + height};
        (*memo.coords)[5] = (*memo.coords)[1];

        (*memo.pends)[0] = 0;
        (*memo.pends)[1] = element.hatch.poly.nCoords;

        err = ACAPI_Element_Create (&element, &memo);
        ACAPI_DisposeElemMemoHdls (&memo);

        return err;
    }

    GSErrCode TableRenderer::CreateTextElement (const CellLayout &layout,
                                                const GS::UniString &line,
                                                const API_Coord &bottomLeft,
                                                double boxWidthMm,
                                                double lineHeightMm) const {

        API_Element element;
        API_ElementMemo memo;
        BNZeroMemory (&element, sizeof (API_Element));
        BNZeroMemory (&memo, sizeof (API_ElementMemo));

        memo.textContent = BMhAllClear ((line.GetLength () + 1) * sizeof (GS::uchar_t));
        if (memo.textContent == nullptr)
            return APIERR_MEMFULL;

        GS::ucscpy (reinterpret_cast<GS::uchar_t *> (*memo.textContent), line.ToUStr ());

        element.header.typeID = API_TextID;

        GSErrCode err = ACAPI_Element_GetDefaults (&element, nullptr);
        if (err != NoError) {
            ACAPI_DisposeElemMemoHdls (&memo);
            return err;
        }

        if (rules.layerIndex != 0)
            element.header.layer = rules.layerIndex;

        // Одна строка текста = один элемент: вертикальное выравнивание в API_TextType
        // отсутствует, поэтому позицию строки считает рендерер (см. §5.3 документа).
        element.text.loc = bottomLeft;
        element.text.anchor = APIAnc_LB;
        element.text.just = ToApiJust (layout.hAlign);
        element.text.size = layout.fontSizeMm;
        element.text.spacing = rules.defaultLineSpacing;
        element.text.font = layout.fontIndex;
        element.text.faceBits = rules.defaultFaceBits;
        element.text.nonBreaking = true; // перенос уже сделан рендерером

        // charCode у текста — «only for font handling» (API_TextType). Оставлять значение
        // из ACAPI_Element_GetDefaults нельзя: дефолты берутся из ТЕКУЩЕЙ базы, а в окне
        // ведомости это уже сброшенная база окна — тогда код не соответствует гарнитуре и
        // кириллица выводится чужими глифами (наблюдалось в окне MyDraw 2026-09-18).
        // Берём код из самой гарнитуры.
        API_Attribute fontAttr;
        BNZeroMemory (&fontAttr, sizeof (API_Attribute));
        fontAttr.header.typeID = API_FontID;
        fontAttr.header.index = layout.fontIndex;
        if (ACAPI_Attribute_Get (&fontAttr) == NoError)
            element.text.charCode = fontAttr.font.charCode;

#ifdef TESTING
        // Печатаем при смене гарнитуры: нужно сравнить путь self-test (база плана)
        // и путь окна ведомости, а не только первый вызов.
        static short lastLoggedFontIndex = -1;
        if (lastLoggedFontIndex != layout.fontIndex) {
            lastLoggedFontIndex = layout.fontIndex;
            DBprnt (GS::UniString::Printf ("font=%d charCode=%d size=%.2f",
                                           static_cast<int> (layout.fontIndex),
                                           static_cast<int> (element.text.charCode),
                                           layout.fontSizeMm),
                    "TableRenderer text params");
        }
#endif

        // Бокс текста задаём явно: иначе элемент уносит дефолтный бокс из
        // ACAPI_Element_GetDefaults, и габарит drawing data оказывается в разы
        // больше самой таблицы (наблюдалось в self-test: bbox 1.53 x 0.18 при
        // таблице 0.06 x 0.018). Единицы width/height — мм (см. API_TextType).
        element.text.width = (boxWidthMm > 0.0) ? boxWidthMm : layout.fontSizeMm;
        element.text.height = (lineHeightMm > 0.0) ? lineHeightMm : layout.fontSizeMm;
        if (rules.textPenIndex != 0)
            element.text.pen = rules.textPenIndex;

        err = ACAPI_Element_Create (&element, &memo);
        ACAPI_DisposeElemMemoHdls (&memo);

        return err;
    }

    // -------------------------------------------------------------------------
    // Демонстрационное содержимое-прототип (§7 ТЗ) для узла ведомости.
    // -------------------------------------------------------------------------
    void TableRenderer::SetPrototypeContent () {
        TableFormattingRules prototypeRules;
        prototypeRules.rows = 3;
        prototypeRules.cols = 3;
        prototypeRules.defaultFontSize = 2.5; // мм, как RoombookSettings::fontsize
        prototypeRules.cellPaddingH = 1.0;
        prototypeRules.cellPaddingV = 0.5;
        prototypeRules.defaultVAlign = VAlign::Middle;
        prototypeRules.defaultHAlign = HAlign::Left;

        MergedRange mergedHeader;
        mergedHeader.rowFrom = 0;
        mergedHeader.rowTo = 0;
        mergedHeader.colFrom = 0;
        mergedHeader.colTo = 1;
        prototypeRules.mergedRanges.Push (mergedHeader);

        ColumnRule autoRule;
        ColumnRule fixedRule;
        fixedRule.mode = ColumnWidthMode::Fixed;
        fixedRule.fixedWidth = 20.0; // мм — по сценарию §7 ТЗ

        prototypeRules.columns.Push (autoRule);
        prototypeRules.columns.Push (fixedRule);
        prototypeRules.columns.Push (autoRule);

        FillRange headerFill;
        headerFill.range = mergedHeader;
        headerFill.fillIndex = FindFirstFillIndex ();
        if (headerFill.fillIndex == APIInvalidAttributeIndex)
            DBprnt ("TableRenderer::SetPrototypeContent", "fills skipped: no fill attribute");
        else
            prototypeRules.fillRanges.Push (headerFill);

        GS::Array<CellData> prototypeCells;
        const auto addCell = [&prototypeCells] (Int32 row, Int32 col, const char *text) {
            CellData cell;
            cell.row = row;
            cell.col = col;
            cell.text = text;
            prototypeCells.Push (cell);
        };

        addCell (0, 0, "Объединённая ячейка с длинным текстом");
        addCell (0, 2, "C1");
        addCell (1, 0, "A2");
        addCell (1, 1, "Текст не влезает в 20 мм и должен перенестись");
        addCell (1, 2, "C2");
        addCell (2, 0, "A3");
        addCell (2, 1, "B3");
        addCell (2, 2, "C3");

        SetFormattingRules (prototypeRules);
        SetCells (prototypeCells);
        SetDrawingScale (1.0);
    }

#ifdef TESTING

    // Self-test (§7 ТЗ). Пишется в DBprnt, в базу проекта ничего не создаётся:
    // элементы уходят в drawing data и освобождаются вместе с ней.
    GSErrCode TableRenderer::RunSelfTest () {
        TableRenderer renderer;
        renderer.SetPrototypeContent ();

        const Int32 prototypeRows = renderer.rules.rows;
        const Int32 prototypeCols = renderer.rules.cols;

        GSErrCode err = renderer.ComputeLayout ();
        if (err != NoError) {
            DBprnt ("TableRenderer::RunSelfTest err", "ComputeLayout failed");
            DBprnt (static_cast<double> (err), "ComputeLayout err code");
            return err;
        }

        for (Int32 col = 0; col < prototypeCols; ++col)
            DBprnt (renderer.GetColumnWidth (col), GS::UniString::Printf ("self-test column %d width (model)", col));
        for (Int32 row = 0; row < prototypeRows; ++row)
            DBprnt (renderer.GetRowHeight (row), GS::UniString::Printf ("self-test row %d height (model)", row));

        DBprnt (renderer.GetTotalWidth (), "self-test total width (model)");
        DBprnt (renderer.GetTotalHeight (), "self-test total height (model)");

        double scale = 1.0;
        err = ACAPI_Database (APIDb_StartDrawingDataID, &scale);
        if (err != NoError) {
            DBprnt ("TableRenderer::RunSelfTest err", "StartDrawingData failed");
            DBprnt (static_cast<double> (err), "StartDrawingData err code");
            return err;
        }

        API_Coord origin = {0.0, 0.0};
        const GSErrCode drawErr = renderer.Draw (origin);

        GSPtr idfStore = nullptr;
        API_Box boundingBox = {};
        const GSErrCode stopErr = ACAPI_Database (APIDb_StopDrawingDataID, &idfStore, &boundingBox);

        if (idfStore != nullptr)
            BMKillPtr (&idfStore);

        DBprnt (static_cast<double> (drawErr), "self-test Draw err code");
        DBprnt (static_cast<double> (stopErr), "self-test StopDrawingData err code");
        DBprnt (boundingBox.xMin, "self-test bbox xMin");
        DBprnt (boundingBox.xMax, "self-test bbox xMax");
        DBprnt (boundingBox.yMin, "self-test bbox yMin");
        DBprnt (boundingBox.yMax, "self-test bbox yMax");

        // Диагностика: по слоям в отдельных сессиях. Габарит всей таблицы оказался
        // в разы больше расчётного, поэтому нужно знать, какой слой его раздувает.
        const auto isolateLayer = [&renderer] (const char *label,
                                               GSErrCode (TableRenderer::*drawLayer) (const API_Coord &) const) {
            double layerScale = 1.0;
            if (ACAPI_Database (APIDb_StartDrawingDataID, &layerScale) != NoError) {
                DBprnt ("TableRenderer::RunSelfTest err", GS::UniString ("StartDrawingData failed for ") + label);
                return;
            }

            const API_Coord layerOrigin = {0.0, 0.0};
            (renderer.*drawLayer) (layerOrigin);

            GSPtr layerStore = nullptr;
            API_Box layerBox = {};
            if (ACAPI_Database (APIDb_StopDrawingDataID, &layerStore, &layerBox) == NoError) {
                DBprnt (layerBox.xMax, GS::UniString ("self-test layer bbox xMax ") + label);
                DBprnt (layerBox.yMax, GS::UniString ("self-test layer bbox yMax ") + label);
            } else {
                DBprnt ("TableRenderer::RunSelfTest err", GS::UniString ("StopDrawingData failed for ") + label);
            }
            if (layerStore != nullptr)
                BMKillPtr (&layerStore);
        };

        isolateLayer ("fills", &TableRenderer::DrawFills);
        isolateLayer ("grid", &TableRenderer::DrawGrid);
        isolateLayer ("texts", &TableRenderer::DrawTexts);

        return stopErr;
    }
#endif

} // namespace TableRenderer
