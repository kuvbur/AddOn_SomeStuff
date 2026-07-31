// ------------ kuvbur 2022 ------------
#pragma once
#if !defined(DG4RULE_HPP)
    #define DG4RULE_HPP
    #include "DGModule.hpp"
    #include <DGStaticItem.hpp>

struct RuleSelectData {
    GS::HashTable<GS::UniString, bool> rules;
    GS::HashTable<GS::UniString, GS::UniString> qty_elements;
    GS::HashTable<GS::UniString, Gfx::Color> color;
    Int32 titleResID = 0;
    bool is_warn = false;
};

class RuleSelectDialog : public DG::ModalDialog,
                         public DG::PanelObserver,
                         public DG::ListBoxObserver,
                         public DG::ButtonItemObserver,
                         public DG::CheckItemObserver,
                         public DG::CompoundItemObserver,
                         public DG::StaticTextObserver {
  public:
    // Идентификаторы элементов диалога выбора правил спецификации.
    enum DialogResourceID { CloseButtonId = 1, OkButtonId = 2, ListBoxId = 3, TextId = 4 };

  private:
    DG::Button closeButton;
    DG::Button okButton;
    DG::SingleSelListBox ListBox;
    DG::CenterText TextBox;

    RuleSelectData &rulelist;
    short ChekboxTab = 1;
    short NameTab = 2;
    short QtyTab = 3;

    short ChekboxTab_w = 30;
    short QtyTab_w = 50;

    short itemCount = QtyTab;

  public:
    // Обрабатывает нажатие на кнопку диалога.
    virtual void ButtonClicked (const DG::ButtonClickEvent &ev) override;

    // Пересчитывает размеры элементов окна после изменения формы.
    virtual void PanelResized (const DG::PanelResizeEvent &ev) override;

    // Обрабатывает выбор строки в списке правил.
    virtual void ListBoxClicked (const DG::ListBoxClickEvent &ev) override;

    // Создаёт диалог выбора правил и связывает его с данными для отображения.
    RuleSelectDialog (RuleSelectData &rulelist);

    // Освобождает ресурсы диалога.
    ~RuleSelectDialog ();

    // Настраивает размеры и расположение элементов диалога.
    void SetSize ();

    // Заполняет список доступных правил.
    void InitListBox ();

    // Устанавливает иконку для выбранного элемента списка.
    void SetIcon (short dwListItem);
};

#endif
