// ------------ kuvbur 2022 ------------
#pragma once
#if !defined (DG4RULE_HPP)
#define	DG4RULE_HPP
#include "DGModule.hpp"


struct RuleSelectData
{
    GS::HashTable<GS::UniString, bool> rules;
    GS::HashTable<GS::UniString, GS::UniString> qty_elements;
    GS::UniString msg = "Test";
    GS::UniString title = "Test";
    bool is_warn = false;
};

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

    RuleSelectData& rulelist;
    short ChekboxTab = 1;
    short NameTab = 2;
    short QtyTab = 3;

    short ChekboxTab_w = 30;
    short QtyTab_w = 30;

    short itemCount = QtyTab;
public:

    virtual void ButtonClicked (const DG::ButtonClickEvent& ev) override;
    virtual void PanelResized (const DG::PanelResizeEvent& ev) override;
    virtual void ListBoxClicked (const DG::ListBoxClickEvent& ev) override;
    RuleSelectDialog (RuleSelectData& rulelist);
    ~RuleSelectDialog ();
    void SetSize ();
    void InitListBox ();
    void SetIcon (short dwListItem);
};

#endif

