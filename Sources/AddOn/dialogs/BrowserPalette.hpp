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
// Show or Hide Browser Palette
// -----------------------------------------------------------------------------
void ShowOrHideBrowserPalette ();

// --- Class definition: BrowserPalette ----------------------------------------

class BrowserPalette final : public DG::Palette, public DG::PanelObserver {
  public:
    enum SelectionModification { RemoveFromSelection, AddToSelection };

    struct ElementInfo {
        GS::UniString guidStr;
        GS::UniString typeName;
        GS::UniString elemID;
    };

  protected:
    enum { BrowserId = 1 };

    DG::Browser browser;

    void InitBrowserControl ();
    void RegisterACAPIJavaScriptObject ();
    void UpdateSelectionInfoInUI ();
    void Command_Helth ();

    virtual void PanelResized (const DG::PanelResizeEvent &ev) override;
    virtual void PanelCloseRequested (const DG::PanelCloseRequestEvent &ev, bool *accepted) override;

    static GSErrCode __ACENV_CALL PaletteControlCallBack (Int32 paletteId,
                                                          API_PaletteMessageID messageID,
                                                          GS::IntPtr param);

    static GS::Ref<BrowserPalette> instance;

    BrowserPalette ();

  public:
    virtual ~BrowserPalette ();

    static bool HasInstance ();
    static void CreateInstance ();
    static BrowserPalette &GetInstance ();

    void Show ();
    void Hide ();

    static GSErrCode RegisterPaletteControlCallBack ();
    static GSErrCode __ACENV_CALL SelectionChangeHandler (const API_Neig *);
};

#endif // BROWSERPALETTE_HPP
