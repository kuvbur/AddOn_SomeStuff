// ------------ kuvbur 2022 ------------
#if !defined(DG4RULE_HPP)
    #define DG4RULE_HPP
    #include <DGStaticItem.hpp>

    #include "DGModule.hpp"

// -----------------------------------------------------------------------------
// Данные для диалога выбора правил спецификации.
// Хранятся имена правил, их состояние флажка, дополнительное текстовое
// значение количества и цветовая информация для отображения.
// -----------------------------------------------------------------------------
struct RuleSelectData {
    // Набор правил и его состояние (включено/выключено).
    GS::HashTable<GS::UniString, bool> rules;

    // Дополнительные строковые значения, связанные с правилом (например, количество).
    GS::HashTable<GS::UniString, GS::UniString> qty_elements;

    // Цвет, с которым должно отображаться правило в списке.
    GS::HashTable<GS::UniString, Gfx::Color> color;

    // Идентификатор строкового ресурса заголовка диалога.
    Int32 titleResID = 0;

    // Если true, диалог отображается в предупреждающем стиле.
    bool is_warn = false;

    // Окно только показывает результат: флажков нет, клик по строке ничего
    // не переключает, повторный запуск из окна не запускается. Используется
    // окном итогов спецификации, где список — отчёт, а не вопрос.
    bool isReadOnly = false;

    // Заголовки колонок значений. Пусто — прежнее поведение: единственная
    // колонка из qty_elements со своим заголовком из ресурса. Непусто — список
    // получает столько колонок значений, сколько их здесь, поэтому число
    // колонок задаёт вызывающий, а не диалог.
    GS::Array<GS::UniString> columnTitles = {};

    // Значения по правилам, по колонкам: valuesPerRule[имя правила][колонка].
    // Плоский список читать неудобно, а соответствие колонок проверяется по
    // границе — лишняя ячейка молча пропала бы из показа.
    GS::HashTable<GS::UniString, GS::Array<GS::UniString>> valuesPerRule;

    // Текст под списком. Показывается, только если непуст.
    GS::UniString footerText;

    // Текст под списком показывается красным (is_warn). Тот же признак, что и
    // у заголовка окна: сообщение об ошибке, а не описание результата.
    bool footerIsWarn = false;
};

// -----------------------------------------------------------------------------
// RuleSelectDialog представляет модальный диалог с таблицей правил
// спецификации/нумерации/суммирования, позволяющий менять состояние чекбоксов и подтверждать выбор.
// -----------------------------------------------------------------------------
class RuleSelectDialog : public DG::ModalDialog,
                         public DG::PanelObserver,
                         public DG::ListBoxObserver,
                         public DG::ButtonItemObserver,
                         public DG::CheckItemObserver,
                         public DG::CompoundItemObserver,
                         public DG::StaticTextObserver {
  public:
    // Идентификаторы элементов диалога.
    enum DialogResourceID { CloseButtonId = 1, OkButtonId = 2, ListBoxId = 3, TextId = 4 };

  private:
    DG::Button closeButton;
    DG::Button okButton;
    DG::SingleSelListBox ListBox;
    DG::CenterText TextBox;

    // Данные правил, используемые для построения списка и обновления состояния.
    RuleSelectData &rulelist;
    short ChekboxTab = 1;
    short NameTab = 2;
    short QtyTab = 3;

    short ChekboxTab_w = 30;
    short QtyTab_w = 50;
    // Ширина колонки, названной вызывающим (columnTitles). Отдельна от QtyTab_w:
    // прежняя колонка количества — узкая, а колонки счётов созданы/изменено/
    // удалено должны читаться, не обрезаясь.
    short ValueTab_w = 60;

    // Число колонок в прежнем режиме: флажок + имя + количество. В режиме
    // columnTitles число колонок вычисляется в InitListBox, и это поле служит
    // только базой для вычитания.
    short itemCount = QtyTab;

    // Режим колонки из qty_elements. Пустой columnTitles означает прежнее
    // поведение — его используют ReNum и Summ, поэтому переключение должно
    // быть автоматическим, а не отдельным вызовом.
    bool useQtyColumn = true;

  public:
    // Обрабатывает событие нажатия на кнопку диалога.
    virtual void ButtonClicked (const DG::ButtonClickEvent &ev) override;

    // Пересчитывает размеры элементов окна после изменения формы.
    virtual void PanelResized (const DG::PanelResizeEvent &ev) override;

    // Обрабатывает щелчок по строке списка правил.
    virtual void ListBoxClicked (const DG::ListBoxClickEvent &ev) override;

    // Создаёт диалог выбора правил и связывает его с данными для отображения.
    RuleSelectDialog (RuleSelectData &rulelist);

    // Освобождает ресурсы диалога.
    ~RuleSelectDialog ();

    // Настраивает размеры и расположение элементов пользовательского интерфейса.
    void SetSize ();

    // Заполняет список доступных правил и выставляет состояния чекбоксов.
    void InitListBox ();

    // Переключает иконку чекбокса выбранного элемента списка.
    void SetIcon (short dwListItem);
};

#endif
