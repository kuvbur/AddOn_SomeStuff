// ------------ kuvbur 2022 ------------
#pragma once
#if !defined (DG4RULE_HPP)
#define	DG4RULE_HPP
#include "DGModule.hpp"

class RuleSelectDialog : public DG::ModalDialog,
    public DG::PanelObserver,
    public DG::ListBoxObserver,
    public DG::ButtonItemObserver,
    public DG::CheckItemObserver,
    public DG::CompoundItemObserver
{
public:
    enum DialogResourceID
    {
        CloseButtonId = 1,
        OkButtonId = 2,
        AllButtonId = 3,
        checkBoxID = 4,
        ListBoxId = 5
    };

private:
    short dialogResId = 32590;
    DG::Button closeButton;
    DG::Button okButton;
    DG::CheckBox checkBox;
    DG::SingleSelListBox ListBox;
    DG::Button allButton;

    GS::Array<GS::UniString>& rulelist;
    GS::Array<bool>& enableRules;

public:

    virtual void ButtonClicked (const DG::ButtonClickEvent& ev) override;
    virtual void PanelClosed (const DG::PanelCloseEvent& ev) override;
    virtual void PanelOpened (const DG::PanelOpenEvent& ev) override;

    RuleSelectDialog (GS::Array<GS::UniString>& rulelist, GS::Array<bool>& enableRules);
    ~RuleSelectDialog ();

    void SetTabData (short item);
    void InitListBox ();
};

#endif

void TestDG ();
