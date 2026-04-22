
#include "ACAPinc.h"					// also includes APIdefs.h
#include "APIdefs.h"
#include "APIEnvir.h"
#include "DG4rule.hpp"

short ruledialogResId = 32590;

RuleSelectDialog::RuleSelectDialog (RuleSelectData& rulelist) :
    DG::ModalDialog (ACAPI_GetOwnResModule (), ruledialogResId, ACAPI_GetOwnResModule ()),
    closeButton (GetReference (), CloseButtonId),
    okButton (GetReference (), OkButtonId),
    ListBox (GetReference (), ListBoxId),
    rulelist (rulelist)
{
    DGSetDialogTitle (ruledialogResId, rulelist.title);
    const DG::Icon& icon = DG::Icon (SysResModule, rulelist.is_warn ? DG_WARNING_ICON : DG_INFORMATION_ICON);
    DGSetDialogIcon (ruledialogResId, icon);
    okButton.Attach (*this);
    closeButton.Attach (*this);
    Attach (*this);
    AttachToAllItems (*this);
    InitListBox ();
}

RuleSelectDialog::~RuleSelectDialog ()
{
    okButton.Detach (*this);
    closeButton.Detach (*this);
    DetachFromAllItems (*this);
    Detach (*this);
}

void RuleSelectDialog::SetSize ()
{
    short width = ListBox.GetItemWidth ();
    short NameTab_w = width - ChekboxTab_w - QtyTab_w;

    ListBox.SetHeaderItemSize (NameTab, NameTab_w);

    short pos = 0;
    ListBox.SetTabFieldProperties (ChekboxTab, pos, pos + ChekboxTab_w, DG::ListBox::Center, DG::ListBox::NoTruncate, false, true);
    pos += ChekboxTab_w;
    ListBox.SetTabFieldProperties (NameTab, pos, pos + NameTab_w, DG::ListBox::Left, DG::ListBox::NoTruncate, false, true);
    pos += NameTab_w;
    ListBox.SetTabFieldProperties (QtyTab, pos, pos + QtyTab_w, DG::ListBox::Center, DG::ListBox::NoTruncate, false, true);
}

void RuleSelectDialog::InitListBox ()
{
    ListBox.SetTabFieldCount (itemCount);
    ListBox.SetHeaderItemCount (itemCount);
    ListBox.SetHeaderSynchronState (true);
    ListBox.SetHeaderPushableButtons (false);

    ListBox.SetHeaderItemText (ChekboxTab, "");
    ListBox.SetHeaderItemText (NameTab, "Rule name");
    ListBox.SetHeaderItemText (QtyTab, "Qty");

    ListBox.SetHeaderItemSize (ChekboxTab, ChekboxTab_w);
    ListBox.SetHeaderItemSizeableFlag (ChekboxTab, false);

    ListBox.SetHeaderItemSize (QtyTab, QtyTab_w);
    ListBox.SetHeaderItemSizeableFlag (QtyTab, false);

    ListBox.SetHeaderItemStyle (ChekboxTab, DG::ListBox::Center, DG::ListBox::NoTruncate);
    ListBox.SetHeaderItemStyle (QtyTab, DG::ListBox::Center, DG::ListBox::NoTruncate);
    ListBox.SetHeaderItemStyle (NameTab, DG::ListBox::Center, DG::ListBox::NoTruncate);

    ListBox.SetHeaderItemMinSize (NameTab, 200);
    ListBox.SetHeaderItemSizeableFlag (NameTab, true);

    SetSize ();
    if (ListBox.GetItemCount () != 0) ListBox.DeleteItem (DG::ListBox::AllItems);
    const DG::Icon& icon = DG::Icon (SysResModule, DG::ListBox::CheckedIcon);
    for (const auto& rulename : rulelist.rules) {
        #if defined(AC_28) || defined(AC_29)
        const GS::UniString& rname = rulename.key;
        #else
        const GS::UniString& rname = *rulename.key;
        #endif
        ListBox.AppendItem ();
        ListBox.SetTabItemIcon (DG::ListBox::BottomItem, ChekboxTab, icon);
        ListBox.SetTabItemText (DG::ListBox::BottomItem, NameTab, rname);
        if (!rulelist.qty_elements.ContainsKey (rname)) continue;
        ListBox.SetTabItemText (DG::ListBox::BottomItem, QtyTab, rulelist.qty_elements.Get (rname));
    }
}

void RuleSelectDialog::SetIcon (short dwListItem)
{
    DG::Icon myIcon = ListBox.GetTabItemIcon (dwListItem, ChekboxTab);
    bool bWasChecked = (myIcon.GetResourceId () == DG::ListBox::CheckedIcon);
    if (!bWasChecked) {
        ListBox.EnableItem (dwListItem);
    } else {
        ListBox.GrayItem (dwListItem);
    }
    const GS::UniString& rname = ListBox.GetTabItemText (dwListItem, NameTab);
    if (rulelist.rules.ContainsKey (rname)) {
        rulelist.rules.Set (rname, !bWasChecked);
    }
    const DG::Icon& icon = DG::Icon (SysResModule, bWasChecked ? DG::ListBox::UncheckedIcon : DG::ListBox::CheckedIcon);
    ListBox.SetTabItemIcon (dwListItem, ChekboxTab, icon);
    ListBox.DeselectItem (dwListItem);
}

void RuleSelectDialog::ListBoxClicked (const DG::ListBoxClickEvent& ev)
{
    short pos = ev.GetMouseOffset ().GetX ();
    short begCheckBox = ListBox.GetTabFieldBeginPosition (ChekboxTab);
    short endCheckBox = ListBox.GetTabFieldEndPosition (ChekboxTab);
    if (pos > begCheckBox && pos < endCheckBox) {
        short dwListItem = ev.GetListItem ();
        SetIcon (dwListItem);
    }
}

void RuleSelectDialog::PanelResized (const DG::PanelResizeEvent& ev)
{
    short dh = ev.GetHorizontalChange ();
    short dv = ev.GetVerticalChange ();
    if (dh != 0 || dv != 0) {
        okButton.Move (dh, dv);
        closeButton.Move (0, dv);
        ListBox.Resize (dh, dv);
        SetSize ();
    }
}

void RuleSelectDialog::ButtonClicked (const DG::ButtonClickEvent& ev)
{
    if (ev.GetSource () == &closeButton) {
        PostCloseRequest (Cancel);
    } else if (ev.GetSource () == &okButton) {
        PostCloseRequest (Accept);
    }
}
