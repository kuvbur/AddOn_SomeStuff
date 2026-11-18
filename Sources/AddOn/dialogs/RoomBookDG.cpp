// *****************************************************************************
// Source code for the RoomBookDG class — палитра настройки типов отделки.
// Сейчас это скелет: HTML-страница загружается, мост ACAPI регистрируется,
// но методы возвращают пустые заглушки. Реальная логика (JSON-хранилище типов,
// поиск и запись свойств зон) подключается отдельным шагом — см. issue #335.
// *****************************************************************************

// ---------------------------------- Includes ---------------------------------

#include "ACAPinc.h"

#include "api_headers/ResourceIds.hpp"

#include "dialogs/RoomBookDG.hpp"

#include "CommonFunction.hpp"

// GUID палитры: должен быть уникальным и стабильным между запусками.
static const GS::Guid roomBookDGPaletteGuid ("{7A1C4B2E-9D38-4C6F-A1B5-2E8F53D7C904}");

GS::Ref<RoomBookDG> RoomBookDG::instance;

// -----------------------------------------------------------------------------
// HTML палитры вшит ресурсом 'DATA' и берётся из того же модуля, что и диалог.
// -----------------------------------------------------------------------------
static GS::UniString LoadFinishHtmlFromResource () {
    GS::UniString resourceData;
    GSHandle data = RSLoadResource ('DATA', ACAPI_GetOwnResModule (), ID_ADDON_HTML_FINISH);
    // Проверка на null ДО вызова BMhGetSize — иначе UB при отсутствии ресурса.
    if (data == nullptr) {
        DBprnt ("RoomBookDG: Finish html resource not found");
        return resourceData;
    }
    const GSSize handleSize = BMhGetSize (data);
    resourceData.Append (*data, handleSize);
    BMhKill (&data);
    return resourceData;
}

// -----------------------------------------------------------------------------
// Show or Hide RoomBook finish palette
// -----------------------------------------------------------------------------
void ShowRoomBookFinishPalette () {
    if (!RoomBookDG::HasInstance ())
        RoomBookDG::CreateInstance ();
    RoomBookDG::GetInstance ().Show ();
}

// --- Class definition: RoomBookDG --------------------------------------------

RoomBookDG::RoomBookDG ()
    : DG::Palette (ACAPI_GetOwnResModule (), ID_ADDON_FINISH_DLG, ACAPI_GetOwnResModule (), roomBookDGPaletteGuid),
      browser (GetReference (), BrowserId) {
    Attach (*this);
    BeginEventProcessing ();
    InitBrowserControl ();
}

RoomBookDG::~RoomBookDG () {
    EndEventProcessing ();
    Detach (*this);
}

void RoomBookDG::CreateInstance () {
    DBASSERT (!HasInstance ());
    instance = new RoomBookDG ();
}

void RoomBookDG::DestroyInstance () { instance = nullptr; }

bool RoomBookDG::HasInstance () { return instance != nullptr; }

RoomBookDG &RoomBookDG::GetInstance () {
    DBASSERT (HasInstance ());
    return *instance;
}

void RoomBookDG::Show () { DG::Palette::Show (); }

void RoomBookDG::Hide () { DG::Palette::Hide (); }

void RoomBookDG::InitBrowserControl () {
    // Мост регистрируется до загрузки страницы: иначе первые вызовы ACAPI.*
    // из HTML придут в пустой объект.
    RegisterACAPIJavaScriptObject ();
    browser.LoadHTML (LoadFinishHtmlFromResource ());
    DBprnt ("RoomBookDG::InitBrowserControl () — HTML load started");
}

// -----------------------------------------------------------------------------
// Заглушки моста. Одна строка-ответ на вызов; nullptr из JSFunction роняет
// CEF-мост, поэтому каждая ветка возвращает DG::JSValue.
// -----------------------------------------------------------------------------
void RoomBookDG::RegisterACAPIJavaScriptObject () {
    DG::JSObject *jsACAPI = new DG::JSObject ("ACAPI");

    // TODO(#335): списки значений полей типа отделки (избранные/материалы проекта).
    jsACAPI->AddItem (new DG::JSFunction ("GetFinishOptions", [] (GS::Ref<DG::JSBase>) -> GS::Ref<DG::JSBase> {
        return new DG::JSValue (
            "{\"ceilingTypes\":[],\"ceilingPlinths\":[],\"aboveCeiling\":[],\"floorTypes\":[],"
            "\"floorPlinths\":[],\"wallFinishes\":[],\"columnFinishes\":[],\"wallBottomFinishes\":[]}");
    }));

    // TODO(#335): чтение типов из JSON-файла и подсчёт зон по some_stuff_fin_type.
    jsACAPI->AddItem (new DG::JSFunction ("GetFinishState", [] (GS::Ref<DG::JSBase>) -> GS::Ref<DG::JSBase> {
        return new DG::JSValue ("{\"types\":[]}");
    }));

    // TODO(#335): запись типа в JSON-файл (аргумент — JSON FinishType).
    jsACAPI->AddItem (new DG::JSFunction ("SaveFinishType", [] (GS::Ref<DG::JSBase>) -> GS::Ref<DG::JSBase> {
        return new DG::JSValue ("{\"success\":false}");
    }));

    // TODO(#335): создание нового типа с пустыми значениями.
    jsACAPI->AddItem (new DG::JSFunction ("AddFinishType", [] (GS::Ref<DG::JSBase>) -> GS::Ref<DG::JSBase> {
        return new DG::JSValue ("{\"id\":\"\",\"name\":\"\",\"zoneCount\":0}");
    }));

    // TODO(#335): удаление типа из JSON-файла (аргумент — id строкой). Зоны не меняются.
    jsACAPI->AddItem (new DG::JSFunction ("DeleteFinishType", [] (GS::Ref<DG::JSBase>) -> GS::Ref<DG::JSBase> {
        return new DG::JSValue ("{\"success\":false}");
    }));

    // TODO(#335): поиск зон со свойством some_stuff_fin_type == имя типа и запись
    // значений типа в свойства зон одной undo-транзакцией (аргумент — JSON FinishType).
    jsACAPI->AddItem (new DG::JSFunction ("UpdateFinishZones", [] (GS::Ref<DG::JSBase>) -> GS::Ref<DG::JSBase> {
        return new DG::JSValue ("{\"updated\":0,\"locked\":0}");
    }));

    // TODO(#335): обновление всех сохранённых типов (типы уже сохранены SaveFinishType).
    jsACAPI->AddItem (new DG::JSFunction ("UpdateAllFinishZones", [] (GS::Ref<DG::JSBase>) -> GS::Ref<DG::JSBase> {
        return new DG::JSValue ("{\"results\":[]}");
    }));

    jsObjectRegistered = browser.RegisterAsynchJSObject (jsACAPI);
    if (!jsObjectRegistered)
        DBprnt ("RoomBookDG: browser.RegisterAsynchJSObject failed for object 'ACAPI'");
}

void RoomBookDG::PanelResized (const DG::PanelResizeEvent &ev) {
    BeginMoveResizeItems ();
    browser.Resize (ev.GetHorizontalChange (), ev.GetVerticalChange ());
    EndMoveResizeItems ();
}

void RoomBookDG::PanelCloseRequested (const DG::PanelCloseRequestEvent &, bool *accepted) {
    Hide ();
    *accepted = true;
}
