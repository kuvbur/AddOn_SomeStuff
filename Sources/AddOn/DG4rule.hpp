// ------------ kuvbur 2022 ------------
#pragma once
#if !defined (DG4RULE_HPP)
#define	DG4RULE_HPP
#include "DGModule.hpp"

// --- AttributeListDialog -----------------------------------------------------

class TestListDialog : public DG::ModalDialog,
    public DG::PanelObserver,
    public DG::ListBoxObserver,
    public DG::ButtonItemObserver
{
private:

    DG::Button					okButton;
    DG::SingleSelListBox		ListBox;
    DG::PopUp					PopupEdit;

public:

    void ButtonClicked (const DG::ButtonClickEvent& ev) override;
    void PanelResized (const DG::PanelResizeEvent& ev) override;
    void SetTabData (short dialId, short item);
    void TestInitListBox (short dialId);

    TestListDialog ();
    ~TestListDialog ();
};


#endif
