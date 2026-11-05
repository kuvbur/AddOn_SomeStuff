// *****************************************************************************
// Header file for BrowserPalette class
// *****************************************************************************

#ifndef BROWSERPALETTE_HPP
#define BROWSERPALETTE_HPP

// ---------------------------------- Includes ---------------------------------

#include "ACAPinc.h" // also includes APIdefs.h

#include "api_headers/APIEnvir.h"

#include "DGModule.hpp"
// AC27 убрал DGBrowser.hpp из DGModule.hpp — браузерный контрол включаем явно.
#include "DGBrowser.hpp"
#include "Sync.hpp"

#ifdef ServerMainVers_2700
// AC27 перенёс JS-мост из DGLib в модуль JavascriptEngine (AC27/28/29):
// DG::JSBase/JSFunction/JSObject/JSArray/JSValue стали JS::Base/Function/Object/Array/Value.
// Псевдонимы сохраняют код моста без правки всех мест использования.
namespace DG {
    using JSBase = JS::Base;
    using JSFunction = JS::Function;
    using JSObject = JS::Object;
    using JSArray = JS::Array;
    using JSValue = JS::Value;
} // namespace DG
#endif

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
    bool jsObjectRegistered = false;

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

#ifdef ServerMainVers_2800
    static GSErrCode PaletteControlCallBack (Int32 paletteId, API_PaletteMessageID messageID, GS::IntPtr param);
#else
    static GSErrCode __ACENV_CALL PaletteControlCallBack (Int32 paletteId,
                                                          API_PaletteMessageID messageID,
                                                          GS::IntPtr param);
#endif

    static GS::Ref<BrowserPalette> instance;

    // -------------------------------------------------------------------------
    // Ограничение количества отображаемых элементов (задаётся из HTML, ≤ select).
    // -------------------------------------------------------------------------
    UInt32 maxSelectionCount = 10;

    // -------------------------------------------------------------------------
    // Ширина клиентской области палитры до свёртывания (пиксели DG).
    // 0 — окно развёрнуто. Используется кнопкой свёртывания (SetPaletteCollapsed).
    // -------------------------------------------------------------------------
    short expandedClientWidth = 0;

    // -------------------------------------------------------------------------
    // Минимальная ширина клиентской области до свёртывания (пиксели DG).
    // Растущий диалог по умолчанию нельзя ужать ниже исходной ширины — на время
    // свёртывания минимум ослабляется, а при развёртывании возвращается.
    // -------------------------------------------------------------------------
    short expandedMinClientWidth = 0;

    // -------------------------------------------------------------------------
    // Ширина свёрнутой палитры (пиксели DG). Пока палитра свёрнута, ручное
    // уменьшение принимается как новый компактный размер; ручное увеличение
    // только логируется и исправляется следующим явным сворачиванием.
    // -------------------------------------------------------------------------
    short collapsedClientWidth = 0;

    // -------------------------------------------------------------------------
    // Флаг подавления обновления палитры при программной подсветке/зуме:
    // APIIo_HighlightElementsID и APIDo_ZoomToElementsID транслируются как
    // смена выделения, и цепочка SelectionChangeHandler сбрасывает выделение.
    // -------------------------------------------------------------------------
    static bool suppressSelectionRefresh;

    BrowserPalette ();

  public:
    virtual ~BrowserPalette ();

    static bool HasInstance ();
    static void CreateInstance ();
    static void DestroyInstance ();
    static BrowserPalette &GetInstance ();

    // reloadContent = false — показ без перезагрузки HTML (путь
    // APIPalMsg_HidePalette_End: страница уже загружена, перезагрузка сбрасывала
    // активную вкладку/фильтр).
    void Show (bool reloadContent = true);
    void Hide ();

    GSErrCode ManualGetSelection ();

    static GSErrCode RegisterPaletteControlCallBack ();
#ifdef ServerMainVers_2800
    static GSErrCode SelectionChangeHandler (const API_Neig *);
#else
    static GSErrCode __ACENV_CALL SelectionChangeHandler (const API_Neig *);
#endif
};

#endif // BROWSERPALETTE_HPP
