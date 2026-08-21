// *****************************************************************************
// Header file for BrowserPalette class
// *****************************************************************************

#ifndef BROWSERPALETTE_HPP
#define BROWSERPALETTE_HPP

// ---------------------------------- Includes ---------------------------------

#include "ACAPinc.h" // also includes APIdefs.h

#include "api_headers/APIEnvir.h"

#include "DGModule.hpp"
#include "Sync.hpp"

#define BrowserPaletteResId 32580
#define BrowserPaletteMenuResId 32580

// -----------------------------------------------------------------------------
// Toggle Browser Palette visibility.
// -----------------------------------------------------------------------------
void ShowOrHideBrowserPalette ();

bool ElementCanHaveProperty (const API_ElemTypeID &eltype);

GS::Array<API_Guid> FilterElementsByType (const GS::Array<API_Guid> &elements, USize maxSelectionCount);

// -----------------------------------------------------------------------------
// BrowserPalette управляет браузерной палитрой, HTML-интерфейсом и мостом JS.
// -----------------------------------------------------------------------------
class BrowserPalette final : public DG::Palette, public DG::PanelObserver {
  public:
    enum SelectionModification { RemoveFromSelection, AddToSelection };

  protected:
    enum { BrowserId = 1 };

    DG::Browser browser;

    // -------------------------------------------------------------------------
    // Инициализация браузерного контролла и подключение HTML страницы.
    // -------------------------------------------------------------------------
    void InitBrowserControl ();

    // -------------------------------------------------------------------------
    // Регистрация JavaScript объекта ACAPI для вызовов из HTML.
    // -------------------------------------------------------------------------
    void RegisterACAPIJavaScriptObject ();

    // -------------------------------------------------------------------------
    // Обновление представления выделения в HTML-интерфейсе.
    // -------------------------------------------------------------------------
    void UpdateSelectionInfoInUI (GS::Array<API_Guid> &selectedElements);

    virtual void PanelResized (const DG::PanelResizeEvent &ev) override;
    virtual void PanelCloseRequested (const DG::PanelCloseRequestEvent &ev, bool *accepted) override;

    static GSErrCode __ACENV_CALL PaletteControlCallBack (Int32 paletteId,
                                                          API_PaletteMessageID messageID,
                                                          GS::IntPtr param);

    static GS::Ref<BrowserPalette> instance;

    // -------------------------------------------------------------------------
    // Ограничение количества отображаемых элементов (задаётся из HTML, ≤ select).
    // -------------------------------------------------------------------------
    UInt32 maxSelectionCount = 10;

    BrowserPalette ();

  public:
    virtual ~BrowserPalette ();

    static bool HasInstance ();
    static void CreateInstance ();
    static BrowserPalette &GetInstance ();

    void Show ();
    void Hide ();

    GSErrCode ManualGetSelection ();

    static GSErrCode RegisterPaletteControlCallBack ();
    static GSErrCode __ACENV_CALL SelectionChangeHandler (const API_Neig *);
};

#endif // BROWSERPALETTE_HPP
