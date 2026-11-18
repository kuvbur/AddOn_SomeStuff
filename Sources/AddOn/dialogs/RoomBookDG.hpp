// *****************************************************************************
// Header file for RoomBookDG class
// *****************************************************************************

#ifndef ROOMBOOKDG_HPP
#define ROOMBOOKDG_HPP

// ---------------------------------- Includes ---------------------------------

#include "ACAPinc.h" // also includes APIdefs.h

#include "api_headers/APIEnvir.h"

#include "DGModule.hpp"
// AC27 убрал DGBrowser.hpp из DGModule.hpp — браузерный контрол включаем явно.
#include "DGBrowser.hpp"

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

// Идентификатор ресурса диалога палитры «Отделка зон» — ID_ADDON_FINISH_DLG
// (api_headers/ResourceIds.hpp, Tools/AddOn.grc.in).

// -----------------------------------------------------------------------------
// Открывает палитру настройки типов отделки (HTML Finish_ru.html).
// -----------------------------------------------------------------------------
void ShowRoomBookFinishPalette ();

// -----------------------------------------------------------------------------
// RoomBookDG — палитра настроек типов отделки: HTML-интерфейс и мост JS.
// Точка входа: кнопка «Настроить» окна Roombook (подключение — отдельный шаг).
// -----------------------------------------------------------------------------
class RoomBookDG final : public DG::Palette, public DG::PanelObserver {
  public:
    enum { BrowserId = 1 };

  protected:
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

    virtual void PanelResized (const DG::PanelResizeEvent &ev) override;
    virtual void PanelCloseRequested (const DG::PanelCloseRequestEvent &ev, bool *accepted) override;

    static GS::Ref<RoomBookDG> instance;

    RoomBookDG ();

  public:
    virtual ~RoomBookDG ();

    static bool HasInstance ();
    static void CreateInstance ();
    static void DestroyInstance ();
    static RoomBookDG &GetInstance ();

    void Show ();
    void Hide ();
};

#endif // ROOMBOOKDG_HPP
