// *****************************************************************************
// Header file for BrowserPalette class
// *****************************************************************************

#ifndef BROWSERPALETTE_HPP
#define BROWSERPALETTE_HPP

// ---------------------------------- Includes ---------------------------------

#include	"APIEnvir.h"
#include	"ACAPinc.h"		// also includes APIdefs.h
#include	"DGModule.hpp"

#define BrowserPaletteResId 32500
#define BrowserPaletteMenuResId 32500
#define BrowserPaletteMenuItemIndex 1

// --- Class declaration: BrowserPalette ------------------------------------------

class LogShowPalette final : public DG::Palette,
							 public DG::PanelObserver
{
public:
	enum SelectionModification { RemoveFromSelection, AddToSelection };

	struct ElementInfo {
		GS::UniString	guidStr;
		GS::UniString	typeName;
		GS::UniString	elemID;
	};

protected:
	enum {
		BrowserId = 1
	};

	DG::Browser		browser;

	void InitBrowserControl ();
	void RegisterACAPIJavaScriptObject ();
	void UpdateSelectedElementsOnHTML ();
	void SetMenuItemCheckedState (bool);

	virtual void PanelResized (const DG::PanelResizeEvent& ev) override;
	virtual	void PanelCloseRequested (const DG::PanelCloseRequestEvent& ev, bool* accepted) override;

	static GS::Array<LogShowPalette::ElementInfo> GetSelectedElements ();
	static void ModifySelection (const GS::UniString& elemGuidStr, SelectionModification modification);

	static GSErrCode __ACENV_CALL	PaletteControlCallBack (Int32 paletteId, API_PaletteMessageID messageID, GS::IntPtr param);

	static GS::Ref<LogShowPalette> instance;

	LogShowPalette ();

public:
	virtual ~LogShowPalette ();

	static bool				HasInstance ();
	static void				CreateInstance ();
	static LogShowPalette&	GetInstance ();

	void Show ();
	void Hide ();

	static GSErrCode				RegisterPaletteControlCallBack ();
	static GSErrCode __ACENV_CALL	SelectionChangeHandler (const API_Neig*);
};

#endif // BROWSERPALETTE_HPP
