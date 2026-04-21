
#include "ACAPinc.h"					// also includes APIdefs.h
#include "APIdefs.h"
#include "APIEnvir.h"
#include "DG4rule.hpp"

RuleSelectDialog::RuleSelectDialog (GS::Array<GS::UniString>& rulelist, GS::Array<bool>& enableRules) :
    DG::ModalDialog (ACAPI_GetOwnResModule (), dialogResId, ACAPI_GetOwnResModule ()),
    closeButton (GetReference (), CloseButtonId),
    okButton (GetReference (), OkButtonId),
    checkBox (GetReference (), checkBoxID),
    ListBox (GetReference (), ListBoxId),
    allButton (GetReference (), AllButtonId),
    rulelist (rulelist),
    enableRules (enableRules)
{
    okButton.Attach (*this);
    closeButton.Attach (*this);
    allButton.Attach (*this);
    Attach (*this);
    AttachToAllItems (*this);
    InitListBox ();
}


RuleSelectDialog::~RuleSelectDialog ()
{
    okButton.Detach (*this);
    closeButton.Detach (*this);
    allButton.Detach (*this);
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

    DGGetItemRect (dialogResId, item, &lBox.left, &lBox.top, &lBox.right, &lBox.bottom);
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

    DGListSetTabData (dialogResId, item, 2, &lTabData[0]);
}


void RuleSelectDialog::InitListBox ()
{
    short NameTab = 1;
    short ControlTab = 2;
    SetTabData (ListBoxId);
    ListBox.SetHeaderSynchronState (false);
    ListBox.SetHeaderItemCount (2);

    ListBox.SetHeaderItemSizeableFlag (NameTab, true);
    ListBox.SetHeaderItemSizeableFlag (ControlTab, true);

    //setting the text of the tab item
    ListBox.SetHeaderItemText (NameTab, "Name");
    ListBox.SetHeaderItemText (ControlTab, "Template");

    //setting the header size of the tab
    ListBox.SetHeaderItemSize (NameTab, 200);
    ListBox.SetHeaderItemSize (ControlTab, 300);

    ListBox.SetHeaderItemSizeableFlag (NameTab, true);
    ListBox.SetHeaderItemSizeableFlag (ControlTab, true);

    ListBox.SetHeaderItemStyle (NameTab, DG::ListBox::DefaultJust, DG::ListBox::EndTruncate);
    ListBox.SetHeaderItemStyle (ControlTab, DG::ListBox::DefaultJust, DG::ListBox::EndTruncate);

    ListBox.SetTabFieldCount (2);

    ////setting properties of the tab
    //short k = ListBox.GetItemWidth ();
    //short pos = 0;
    //ListBox.SetTabFieldProperties (NameTab, pos, pos + 200, DG::ListBox::Left, DG::ListBox::NoTruncate, false);
    //pos += 200;
    //ListBox.SetTabFieldProperties (ControlTab, pos, pos + 200, DG::ListBox::Left, DG::ListBox::NoTruncate, false);

    ////Create List Box items
    //for (short i = 1; i <= 4; i++) {
    //    GS::UniString listItemName = "Test - " + GS::ValueToUniString (i);
    //    GS::UniString listItemName2 = "Semple Text No - " + GS::ValueToUniString (i);
    //    ListBox.AppendItem ();
    //    ListBox.SetTabItemText (i, NameTab, listItemName);
    //    ListBox.SetTabItemText (i, ControlTab, listItemName2);
    //}

    // -------------------------------------------------- Set the UserData in list items -----------------------------------------------------------------
    //DGListSetItemUserData(short  dialId, short  item, short  listItem, DGUserData   value);


    /*short appendedIndex = DGAppendDialogItem(dialId, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 10, 60, 80, 18);
    DG::Button btn(dialId, appendedIndex);*/
}

void RuleSelectDialog::ButtonClicked (const DG::ButtonClickEvent& ev)
{

    if (ev.GetSource () == &closeButton) {
        PostCloseRequest (Cancel);
    } else if (ev.GetSource () == &okButton) {
        PostCloseRequest (Accept);
    } else if (ev.GetSource () == &allButton) {
        PostCloseRequest (Accept);
    }
}

void TestDG ()
{
    GSErrCode err = NoError;
    GS::Array<GS::UniString> rulelist = {};
    GS::Array<bool> enableRules = {};
    rulelist.PushNew ("Test 1"); enableRules.Push (true);
    rulelist.PushNew ("Test 2"); enableRules.Push (true);
    RuleSelectDialog dialog (rulelist, enableRules);
    if (dialog.Invoke ()) {
        int hh = 1;
    }
    int hh = 1;
}
