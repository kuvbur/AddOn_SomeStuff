
#include "ACAPinc.h"					// also includes APIdefs.h
#include "APIdefs.h"
#include "APIEnvir.h"
#include "DG4rule.hpp"

short ruledialogResId = 32590;

RuleSelectDialog::RuleSelectDialog (GS::Array<GS::UniString>& rulelist, GS::Array<bool>& enableRules) :
    DG::ModalDialog (ACAPI_GetOwnResModule (), ruledialogResId, ACAPI_GetOwnResModule ()),
    closeButton (GetReference (), CloseButtonId),
    okButton (GetReference (), OkButtonId),
    ListBox (GetReference (), ListBoxId),
    rulelist (rulelist),
    enableRules (enableRules)
{
    enableRules.Clear ();
    for (const auto& rulename : rulelist) {
        enableRules.Push (true);
    }
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

void RuleSelectDialog::PanelClosed (const DG::PanelCloseEvent& ev)
{
    int hh = 1;
}

void RuleSelectDialog::PanelOpened (const DG::PanelOpenEvent& /*ev*/)
{
    int hh = 1;
}


void RuleSelectDialog::SetTabData (short item)
{
    DGListTabData lTabData[2];
    API_Rect lBox;

    DGGetItemRect (ruledialogResId, item, &lBox.left, &lBox.top, &lBox.right, &lBox.bottom);
    lBox.right -= 10;
    short width = (short) (lBox.right - lBox.left);

    lTabData[0].begPos = 0;
    lTabData[0].endPos = (short) (width);
    lTabData[0].justFlag = DG_IS_LEFT;
    lTabData[0].truncFlag = DG_IS_TRUNCEND;
    lTabData[0].hasSeparator = false;
    lTabData[0].disabled = false;

    lTabData[1].begPos = lTabData[0].endPos;;
    lTabData[1].endPos = (short) (lBox.right);//250;
    lTabData[1].justFlag = DG_IS_LEFT;
    lTabData[1].truncFlag = DG_IS_TRUNCEND;
    lTabData[1].hasSeparator = false;
    lTabData[1].disabled = false;

    DGListSetTabData (ruledialogResId, item, 2, &lTabData[0]);
}


void RuleSelectDialog::InitListBox ()
{
    short NameTab = 1;
    short ControlTab = 2;
    SetTabData (ListBoxId);
    ListBox.SetHeaderSynchronState (true);
    ListBox.SetHeaderItemCount (2);

    //setting the text of the tab item
    ListBox.SetHeaderItemText (NameTab, "On/Off");
    ListBox.SetHeaderItemText (ControlTab, "Rule name");

    //setting the header size of the tab
    ListBox.SetHeaderItemSize (NameTab, 20);
    ListBox.SetHeaderItemMinSize (NameTab, 20);
    ListBox.SetHeaderItemSize (ControlTab, 445);
    ListBox.SetHeaderItemMinSize (ControlTab, 445);

    ListBox.SetHeaderItemSizeableFlag (NameTab, false);
    ListBox.SetHeaderItemSizeableFlag (ControlTab, true);

    ListBox.SetHeaderItemStyle (NameTab, DG::ListBox::Center, DG::ListBox::EndTruncate);
    ListBox.SetHeaderItemStyle (ControlTab, DG::ListBox::Center, DG::ListBox::EndTruncate);

    ListBox.SetTabFieldCount (4);

    ////setting properties of the tab
    short k = ListBox.GetItemWidth ();
    short pos = 20;
    ListBox.SetTabFieldProperties (NameTab, pos, pos + 20, DG::ListBox::Center, DG::ListBox::NoTruncate, true);
    pos += 20;
    ListBox.SetTabFieldProperties (ControlTab, pos, pos + 440, DG::ListBox::Left, DG::ListBox::NoTruncate, true);

    ListBox.SetHeaderPushableButtons (false);

    if (ListBox.GetItemCount () != 0) {
        ListBox.DeleteItem (DG::ListBox::AllItems);
    }
    const DG::Icon& icon = DG::Icon (SysResModule, DG::ListBox::CheckedIcon);
    for (const auto& rulename : rulelist) {
        ListBox.AppendItem ();
        ListBox.SetTabItemIcon (DG::ListBox::BottomItem, NameTab, icon);
        ListBox.SetTabItemText (DG::ListBox::BottomItem, ControlTab, rulename);
    }
}

void RuleSelectDialog::SetIcon (short dwListItem)
{
    if (dwListItem < 1) return;
    if (dwListItem > (short) enableRules.GetSize ()) return;
    DG::Icon myIcon = ListBox.GetTabItemIcon (dwListItem, 1);
    bool bWasChecked = (myIcon.GetResourceId () == DG::ListBox::CheckedIcon);
    enableRules[dwListItem - 1] = !bWasChecked;
    if (!bWasChecked) {
        ListBox.EnableItem (dwListItem);
    } else {
        ListBox.GrayItem (dwListItem);
    }
    const DG::Icon& icon = DG::Icon (SysResModule, bWasChecked ? DG::ListBox::UncheckedIcon : DG::ListBox::CheckedIcon);
    ListBox.SetTabItemIcon (dwListItem, 1, icon);
    ListBox.DeselectItem (dwListItem);
}

void RuleSelectDialog::ListBoxClicked (const DG::ListBoxClickEvent& ev)
{
    short pos = ev.GetMouseOffset ().GetX ();
    short begCheckBox = ListBox.GetTabFieldBeginPosition (1);
    short endCheckBox = ListBox.GetTabFieldEndPosition (1);
    if (pos > begCheckBox && pos < endCheckBox) {
        short dwListItem = ev.GetListItem ();
        SetIcon (dwListItem);
    }
}

//void RuleSelectDialog::ListBoxSelectionChanged (const DG::ListBoxSelectionEvent& ev)
//{
//    GS::Array<short> sel = ListBox.GetSelectedItems ();
//    if (sel.IsEmpty ()) return;
//    for (const auto& dwListItem : sel) {
//        RuleSelectDialog::SetIcon (dwListItem);
//    }
//}

void RuleSelectDialog::PanelResized (const DG::PanelResizeEvent& ev)
{
    short dh = ev.GetHorizontalChange ();
    short dv = ev.GetVerticalChange ();
    if (dh != 0 || dv != 0) {
        BeginMoveResizeItems ();
        okButton.Move (dh, dv);
        closeButton.Move (0, dv);
        ListBox.Resize (dh, dv);
        ListBox.SetTabFieldBeginEndPosition (2, ListBox.GetTabFieldBeginPosition (2), ListBox.GetItemWidth ());
        ListBox.SetHeaderItemSize (2, (ListBox.GetWidth () - ListBox.GetHeaderItemSize (1)));
        EndMoveResizeItems ();
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

void TestDG ()
{
    GSErrCode err = NoError;
    GS::Array<GS::UniString> rulelist = {};
    GS::Array<bool> enableRules = {};
    rulelist.PushNew ("Test 1");
    rulelist.PushNew ("Test 2");
    RuleSelectDialog dialog (rulelist, enableRules);
    if (dialog.Invoke ()) {
        int hh = 1;
    }
    int hh = 1;
}
