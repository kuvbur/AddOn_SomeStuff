
#include "ACAPinc.h" // also includes APIdefs.h

#include "api_headers/APIEnvir.h"
#include "api_headers/ResourceIds.hpp"

#include "dialogs/DG4rule.hpp"

#include "APIdefs.h"
#include "Propertycache.hpp"

// -----------------------------------------------------------------------------
// Реализация диалога выбора правил спецификации.
// -----------------------------------------------------------------------------
RuleSelectDialog::RuleSelectDialog (RuleSelectData &rulelist)
    : DG::ModalDialog (ACAPI_GetOwnResModule (), ID_ADDON_RULE_DLG, ACAPI_GetOwnResModule ()),
      closeButton (GetReference (), CloseButtonId), okButton (GetReference (), OkButtonId),
      ListBox (GetReference (), ListBoxId), TextBox (GetReference (), TextId), rulelist (rulelist) {
    const Int32 iseng = ID_ADDON_STRINGS + isEng ();
    GS::UniString text = RSGetIndString (iseng, rulelist.titleResID, ACAPI_GetOwnResModule ());
    GS::UniString version = RSGetIndString (ID_ADDON_STRINGS, 49, ACAPI_GetOwnResModule ());
    DGSetDialogTitle (ID_ADDON_RULE_DLG, version + " " + text);
    text = RSGetIndString (iseng, 76, ACAPI_GetOwnResModule ());
    DGSetItemText (ID_ADDON_RULE_DLG, OkButtonId, text);
    text = RSGetIndString (iseng, 77, ACAPI_GetOwnResModule ());
    DGSetItemText (ID_ADDON_RULE_DLG, CloseButtonId, text);
    const DG::Icon &icon = DG::Icon (SysResModule, rulelist.is_warn ? DG_WARNING_ICON : DG_INFORMATION_ICON);
    DGSetDialogIcon (ID_ADDON_RULE_DLG, icon);
    // Непустой columnTitles переключает список на колонки вызывающего.
    // Присваивается здесь, до InitListBox, потому что от него зависит и число
    // колонок, и ширина колонки, и источник значений.
    useQtyColumn = rulelist.columnTitles.IsEmpty ();
    // Текст под списком задаёт вызывающий. Он важнее заголовка из ресурса
    // (строка 80 — общее предупреждение), поэтому при заданном тексте
    // заголовок не показывается: два красных сообщения подряд сбивают с толку.
    const bool hasFooter = !rulelist.footerText.IsEmpty ();
    if (rulelist.is_warn && !hasFooter) {
        text = RSGetIndString (iseng, 80, ACAPI_GetOwnResModule ());
        TextBox.SetText (text);
        TextBox.SetTextColor (Gfx::Color::Red);
    } else if (hasFooter) {
        TextBox.SetText (rulelist.footerText);
        if (rulelist.footerIsWarn)
            TextBox.SetTextColor (Gfx::Color::Red);
    } else {
        TextBox.Hide ();
        short lx = ListBox.GetPosition ().GetY () - TextBox.GetHeight () + 5;
        short lh = ListBox.GetHeight () + TextBox.GetHeight ();
        ListBox.SetPosition (ListBox.GetPosition ().GetX (), lx);
        ListBox.SetHeight (lh);
    }
    okButton.Attach (*this);
    closeButton.Attach (*this);
    Attach (*this);
    AttachToAllItems (*this);
    InitListBox ();
}

RuleSelectDialog::~RuleSelectDialog () {
    okButton.Detach (*this);
    closeButton.Detach (*this);
    DetachFromAllItems (*this);
    Detach (*this);
}

// -----------------------------------------------------------------------------
// Обновление размеров вкладок и позиционирования элементов списка.
// -----------------------------------------------------------------------------
void RuleSelectDialog::SetSize () {
    short width = ListBox.GetItemWidth ();
    // Ширина колонки флажка в режиме показа нулевая: флажков нет, и место под
    // них иначе съедало бы ширину колонок значений.
    const short leadWidth = rulelist.isReadOnly ? 0 : ChekboxTab_w;
    const short valueCount = useQtyColumn ? 1 : static_cast<short> (rulelist.columnTitles.GetSize ());
    // Ширина колонки значений. У колонки из qty_elements она своя (QtyTab_w),
    // и у колонок columnTitles тоже своя (ValueTab_w): брать одну и ту же
    // нельзя, заголовок и поле разошлись бы по ширине, и это было бы видно
    // ещё и в обычном диалоге выбора правил.
    const short defaultValueWidth = useQtyColumn ? QtyTab_w : ValueTab_w;
    const short valueWidth = rulelist.valueColumnWidth > 0 ? rulelist.valueColumnWidth : defaultValueWidth;
    const short fixedWidth = leadWidth + static_cast<short> (valueCount * valueWidth);
    short NameTab_w = width - fixedWidth;
    if (rulelist.is_warn || !rulelist.footerText.IsEmpty ())
        TextBox.SetWidth (width);
    ListBox.SetHeaderItemSize (NameTab, NameTab_w);

    short pos = 0;
    ListBox.SetTabFieldProperties (
        ChekboxTab, pos, pos + leadWidth, DG::ListBox::Center, DG::ListBox::NoTruncate, false, true);
    pos += leadWidth;
    ListBox.SetTabFieldProperties (
        NameTab, pos, pos + NameTab_w, DG::ListBox::Left, DG::ListBox::NoTruncate, false, true);
    pos += NameTab_w;
    // Колонки значений идут подряд; их число задаёт вызывающий.
    for (short i = 0; i < valueCount; ++i) {
        const short tab = useQtyColumn ? QtyTab : static_cast<short> (NameTab + 1 + i);
        ListBox.SetTabFieldProperties (
            tab, pos, pos + valueWidth, DG::ListBox::Center, DG::ListBox::NoTruncate, false, true);
        pos += valueWidth;
    }
}

// -----------------------------------------------------------------------------
// Инициализация таблицы правил и заполнение списка значениями из RuleSelectData.
// -----------------------------------------------------------------------------
void RuleSelectDialog::InitListBox () {
    // Число колонок: флажок + имя + колонки значений. В режиме показа флажок
    // не занимает места, но индексы табов от него не сдвигаются — иначе
    // пришлось бы пересчитывать NameTab в двух местах.
    const short valueCount = useQtyColumn ? 1 : static_cast<short> (rulelist.columnTitles.GetSize ());
    // Табов нужно на один больше, чем используется: последний таб колонки
    // значений имеет индекс NameTab + valueCount, а прежняя формула
    // (itemCount - 1 + valueCount) давала ровно NameTab + valueCount. Лишний
    // таб пустой и находится за краем списка, поэтому не виден, зато у
    // последней колонки не пропадает заголовок: колонки рисуются по индексам
    // табов, и без запаса заголовок уходил за SetHeaderItemCount, хотя данные
    // в ячейке оставались.
    const short totalCount = static_cast<short> (NameTab + valueCount + 1);
    ListBox.SetTabFieldCount (totalCount);
    ListBox.SetHeaderItemCount (totalCount);
    ListBox.SetHeaderSynchronState (true);
    ListBox.SetHeaderPushableButtons (false);

    const Int32 iseng = ID_ADDON_STRINGS + isEng ();
    GS::UniString text = RSGetIndString (iseng, 78, ACAPI_GetOwnResModule ());
    ListBox.SetHeaderItemText (NameTab, text);
    if (useQtyColumn) {
        text = RSGetIndString (iseng, 79, ACAPI_GetOwnResModule ());
        ListBox.SetHeaderItemText (QtyTab, text);
    } else {
        for (short i = 0; i < valueCount; ++i)
            ListBox.SetHeaderItemText (static_cast<short> (NameTab + 1 + i), rulelist.columnTitles[i]);
    }

    ListBox.SetHeaderItemText (ChekboxTab, "");
    ListBox.SetHeaderItemSize (ChekboxTab, rulelist.isReadOnly ? 0 : ChekboxTab_w);
    ListBox.SetHeaderItemSizeableFlag (ChekboxTab, false);

    const short valueWidth =
        rulelist.valueColumnWidth > 0 ? rulelist.valueColumnWidth : (useQtyColumn ? QtyTab_w : ValueTab_w);
    for (short i = 0; i < valueCount; ++i) {
        const short tab = useQtyColumn ? QtyTab : static_cast<short> (NameTab + 1 + i);
        ListBox.SetHeaderItemSize (tab, valueWidth);
        ListBox.SetHeaderItemSizeableFlag (tab, false);
        ListBox.SetHeaderItemStyle (tab, DG::ListBox::Center, DG::ListBox::NoTruncate);
    }

    ListBox.SetHeaderItemStyle (ChekboxTab, DG::ListBox::Center, DG::ListBox::NoTruncate);
    ListBox.SetHeaderItemStyle (NameTab, DG::ListBox::Center, DG::ListBox::NoTruncate);

    ListBox.SetHeaderItemMinSize (NameTab, 200);
    ListBox.SetHeaderItemSizeableFlag (NameTab, true);

    SetSize ();
    if (ListBox.GetItemCount () != 0)
        ListBox.DeleteItem (DG::ListBox::AllItems);
    const DG::Icon &icon = DG::Icon (SysResModule, DG::ListBox::CheckedIcon);
    const DG::Icon &unicon = DG::Icon (SysResModule, DG::ListBox::UncheckedIcon);

    for (const auto &rulename : rulelist.rules) {
#if defined(ServerMainVers_2800) || defined(ServerMainVers_2900)
        const GS::UniString &rname = rulename.key;
#else
        const GS::UniString &rname = *rulename.key;
#endif
        ListBox.AppendItem ();
        // В режиме показа иконки нет: строка — отчёт, её нельзя переключить.
        if (!rulelist.isReadOnly) {
            if (rulelist.rules[rname]) {
                ListBox.SetTabItemIcon (DG::ListBox::BottomItem, ChekboxTab, icon);
            } else {
                ListBox.SetTabItemIcon (DG::ListBox::BottomItem, ChekboxTab, unicon);
            }
        }
        ListBox.SetTabItemText (DG::ListBox::BottomItem, NameTab, rname);
        if (useQtyColumn) {
            if (rulelist.qty_elements.ContainsKey (rname))
                ListBox.SetTabItemText (DG::ListBox::BottomItem, QtyTab, rulelist.qty_elements.Get (rname));
        } else if (rulelist.valuesPerRule.ContainsKey (rname)) {
            const GS::Array<GS::UniString> &values = rulelist.valuesPerRule.Get (rname);
            for (short i = 0; i < valueCount && i < static_cast<short> (values.GetSize ()); ++i)
                ListBox.SetTabItemText (DG::ListBox::BottomItem, static_cast<short> (NameTab + 1 + i), values[i]);
        }
        if (rulelist.color.ContainsKey (rname)) {
            // Окрашиваются все колонки строки, иначе ошибка правила была бы
            // видна не везде. Граница ВКЛЮЧИТЕЛЬНАЯ: totalCount — это ЧИСЛО
            // колонок, а последний индекс на единицу меньше.
            for (short tab = QtyTab; tab <= totalCount; ++tab)
                ListBox.SetTabItemColor (DG::ListBox::BottomItem, tab, rulelist.color.Get (rname));
        } else {
            if (rulelist.is_warn)
                ListBox.SetTabItemColor (DG::ListBox::BottomItem, QtyTab, Gfx::Color::Red);
        }
    }
}

// -----------------------------------------------------------------------------
// Переключение иконки чекбокса и обновление состояния правила.
// -----------------------------------------------------------------------------
void RuleSelectDialog::SetIcon (short dwListItem) {
    // В режиме показа переключать нечего: иконки нет.
    if (rulelist.isReadOnly)
        return;
    DG::Icon myIcon = ListBox.GetTabItemIcon (dwListItem, ChekboxTab);
    bool bWasChecked = (myIcon.GetResourceId () == DG::ListBox::CheckedIcon);
    if (!bWasChecked) {
        ListBox.EnableItem (dwListItem);
    } else {
        ListBox.GrayItem (dwListItem);
    }
    const GS::UniString &rname = ListBox.GetTabItemText (dwListItem, NameTab);
    if (rulelist.rules.ContainsKey (rname)) {
        rulelist.rules.Set (rname, !bWasChecked);
    }
    const DG::Icon &icon = DG::Icon (SysResModule, bWasChecked ? DG::ListBox::UncheckedIcon : DG::ListBox::CheckedIcon);
    ListBox.SetTabItemIcon (dwListItem, ChekboxTab, icon);
    ListBox.DeselectItem (dwListItem);
}

// -----------------------------------------------------------------------------
// Обработка клика по элементу списка правил. Если клик выполнен по чекбоксу,
// переключается состояние правила.
// -----------------------------------------------------------------------------
void RuleSelectDialog::ListBoxClicked (const DG::ListBoxClickEvent &ev) {
    // В режиме только показа переключение не работает: окно ничего не меняет,
    // поэтому и флажков у него нет.
    if (rulelist.isReadOnly)
        return;
    short pos = ev.GetMouseOffset ().GetX ();
    short begCheckBox = ListBox.GetTabFieldBeginPosition (ChekboxTab);
    short endCheckBox = ListBox.GetTabFieldEndPosition (ChekboxTab);
    if (pos > begCheckBox && pos < endCheckBox) {
        short dwListItem = ev.GetListItem ();
        SetIcon (dwListItem);
    }
}

// -----------------------------------------------------------------------------
// Обработка изменения размера диалога. Перемещает кнопки и изменяет размеры
// списка с правилами.
// -----------------------------------------------------------------------------
void RuleSelectDialog::PanelResized (const DG::PanelResizeEvent &ev) {
    short dh = ev.GetHorizontalChange ();
    short dv = ev.GetVerticalChange ();
    if (dh != 0 || dv != 0) {
        okButton.Move (dh, dv);
        closeButton.Move (0, dv);
        ListBox.Resize (dh, dv);
        SetSize ();
    }
}

// -----------------------------------------------------------------------------
// Обработка нажатий кнопок диалога: закрыть или подтвердить выбор.
// -----------------------------------------------------------------------------
void RuleSelectDialog::ButtonClicked (const DG::ButtonClickEvent &ev) {
    if (ev.GetSource () == &closeButton) {
        PostCloseRequest (Cancel);
    } else if (ev.GetSource () == &okButton) {
        PostCloseRequest (Accept);
    }
}
