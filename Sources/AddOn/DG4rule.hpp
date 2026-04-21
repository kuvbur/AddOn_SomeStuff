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
        ListBoxId = 3
    };

private:
    DG::Button closeButton;
    DG::Button okButton;
    DG::SingleSelListBox ListBox;

    GS::Array<GS::UniString>& rulelist;
    GS::Array<bool>& enableRules;

public:

    virtual void ButtonClicked (const DG::ButtonClickEvent& ev) override;
    virtual void PanelClosed (const DG::PanelCloseEvent& ev) override;
    virtual void PanelOpened (const DG::PanelOpenEvent& ev) override;
    virtual void PanelResized (const DG::PanelResizeEvent& ev);
    virtual void ListBoxClicked (const DG::ListBoxClickEvent& ev);
    //virtual void ListBoxSelectionChanged (const DG::ListBoxSelectionEvent& ev);
    RuleSelectDialog (GS::Array<GS::UniString>& rulelist, GS::Array<bool>& enableRules);
    ~RuleSelectDialog ();

    void SetTabData (short item);
    void InitListBox ();
    void SetIcon (short dwListItem);
};

#endif

void TestDG ();
