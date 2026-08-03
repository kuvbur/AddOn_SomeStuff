// *****************************************************************************
// Source code for the BrowserPalette class
// *****************************************************************************

// ---------------------------------- Includes ---------------------------------

#include "dialogs/BrowserPalette.hpp"

#include "CommonFunction.hpp"
#include "dialogs/SyncSettings.hpp"
#include "Propertycache.hpp"

static const GS::Guid paletteGuid ("{FEE27B6B-3873-5844-88B6-F0083AA4CD49}");

GS::Ref<BrowserPalette> BrowserPalette::instance;

// -----------------------------------------------------------------------------
// Show or Hide Browser Palette
// -----------------------------------------------------------------------------
void ShowOrHideBrowserPalette () {
    if (BrowserPalette::HasInstance () && BrowserPalette::GetInstance ().IsVisible ()) {
        BrowserPalette::GetInstance ().Hide ();
    } else {
        if (!BrowserPalette::HasInstance ())
            BrowserPalette::CreateInstance ();
        BrowserPalette::GetInstance ().Show ();
    }
}

static GS::UniString LoadHtmlFromResource () {
    GS::UniString resourceData;
    const Int32 bisEng = ID_ADDON_HTML + isEng ();
    GSHandle data = RSLoadResource ('DATA', ACAPI_GetOwnResModule (), bisEng);
    GSSize handleSize = BMhGetSize (data);
    if (data != nullptr) {
        resourceData.Append (*data, handleSize);
        BMhKill (&data);
    }
    return resourceData;
}

// --- Class definition: BrowserPalette ----------------------------------------

BrowserPalette::BrowserPalette ()
    : DG::Palette (ACAPI_GetOwnResModule (), BrowserPaletteResId, ACAPI_GetOwnResModule (), paletteGuid),
      browser (GetReference (), BrowserId) {
    Attach (*this);
    BeginEventProcessing ();
    InitBrowserControl ();
}

BrowserPalette::~BrowserPalette () {
    EndEventProcessing ();
    Detach (*this);
    instance = nullptr;
}

bool BrowserPalette::HasInstance () { return instance != nullptr; }

void BrowserPalette::CreateInstance () {
    DBASSERT (!HasInstance ());
    instance = new BrowserPalette ();
    ACAPI_KeepInMemory (true);
}

BrowserPalette &BrowserPalette::GetInstance () {
    DBASSERT (HasInstance ());
    return *instance;
}

void BrowserPalette::Show () {
    DG::Palette::Show ();
    SyncSettings syncSettings;
    LoadSyncSettingsFromPreferences (syncSettings, true);
    syncSettings.SetShowPalette (true);
    MenuItemCheckAC (Menu_Pallete, syncSettings.GetShowPalette ());
    WriteSyncSettingsToPreferences (syncSettings);
    browser.ReloadIgnoreCache ();
}

void BrowserPalette::Hide () {
    DG::Palette::Hide ();
    SyncSettings syncSettings;
    LoadSyncSettingsFromPreferences (syncSettings, true);
    syncSettings.SetShowPalette (false);
    MenuItemCheckAC (Menu_Pallete, syncSettings.GetShowPalette ());
    WriteSyncSettingsToPreferences (syncSettings);
}

void BrowserPalette::InitBrowserControl () {
    // Загружаем HTML
    browser.LoadHTML (LoadHtmlFromResource ());

    // Регистрируем JavaScript объект для взаимодействия с ArchiCAD
    RegisterACAPIJavaScriptObject ();
}

void BrowserPalette::RegisterACAPIJavaScriptObject () {
    DG::JSObject *jsACAPI = new DG::JSObject ("ACAPI");

    // Регистрируем функцию для получения свойств (вызывается из HTML кнопки)
    jsACAPI->AddItem (new DG::JSFunction ("GetPropertyDefinitions", [] (GS::Ref<DG::JSBase>) {
        // Получаем свойства из кэша аддона
        if (!ParamHelpers::isPropertyDefinitionRead ()) {
            DBprnt ("Property cache not loaded");
            // Возвращаем пустой массив
            GS::Ref<DG::JSArray> emptyArray = new DG::JSArray ();
            return emptyArray;
        }

        auto &cache = PROPERTYCACHE ();

        // Создаем JS массив для возврата свойств
        GS::Ref<DG::JSArray> jsArray = new DG::JSArray ();

        // Итерируем по свойствам в кэше (GS::HashTable возвращает пары указателей)
        for (const auto &pair : cache.property) {
            // pair.key и pair.value — это указатели, нужно разыменовать
            const GS::UniString &rawName = *pair.key;
            const ParamValue &paramValue = *pair.value;

            // Добавляем имя свойства в массив
            if (!paramValue.name.IsEmpty ()) {
                jsArray->AddItem (new DG::JSValue (paramValue.name.ToCStr ().Get ()));
            } else {
                jsArray->AddItem (new DG::JSValue (rawName.ToCStr ().Get ()));
            }
        }

        DBprnt ("GetPropertyDefinitions: returned " + GS::ValueToUniString (cache.property.GetSize ()) + " properties");
        return jsArray;
    }));

    browser.RegisterAsynchJSObject (jsACAPI);
}

void BrowserPalette::Command_Helth () { browser.ExecuteJS ("Command_Helth ()"); }

void BrowserPalette::PanelResized (const DG::PanelResizeEvent &ev) {
    BeginMoveResizeItems ();
    browser.Resize (ev.GetHorizontalChange (), ev.GetVerticalChange ());
    EndMoveResizeItems ();
}

void BrowserPalette::PanelCloseRequested (const DG::PanelCloseRequestEvent &, bool *accepted) {
    Hide ();
    *accepted = true;
}

GSErrCode __ACENV_CALL BrowserPalette::PaletteControlCallBack (Int32,
                                                               API_PaletteMessageID messageID,
                                                               GS::IntPtr param) {
    switch (messageID) {
    case APIPalMsg_OpenPalette:
        if (!HasInstance ())
            CreateInstance ();
        GetInstance ().Show ();
        break;

    case APIPalMsg_ClosePalette:
        if (!HasInstance ())
            break;
        GetInstance ().Hide ();
        break;

    case APIPalMsg_HidePalette_Begin:
        if (HasInstance () && GetInstance ().IsVisible ())
            GetInstance ().Hide ();
        break;

    case APIPalMsg_HidePalette_End:
        if (HasInstance () && !GetInstance ().IsVisible ())
            GetInstance ().Show ();
        break;

    case APIPalMsg_DisableItems_Begin:
        if (HasInstance () && GetInstance ().IsVisible ())
            GetInstance ().DisableItems ();
        break;

    case APIPalMsg_DisableItems_End:
        if (HasInstance () && GetInstance ().IsVisible ())
            GetInstance ().EnableItems ();
        break;

    case APIPalMsg_IsPaletteVisible:
        *(reinterpret_cast<bool *> (param)) = HasInstance () && GetInstance ().IsVisible ();
        break;

    default:
        break;
    }

    return NoError;
}

GSErrCode BrowserPalette::RegisterPaletteControlCallBack () {
    return ACAPI_RegisterModelessWindow (GS::CalculateHashValue (paletteGuid),
                                         PaletteControlCallBack,
                                         API_PalEnabled_FloorPlan + API_PalEnabled_Section + API_PalEnabled_Elevation +
                                             API_PalEnabled_InteriorElevation + API_PalEnabled_3D +
                                             API_PalEnabled_Detail + API_PalEnabled_Worksheet + API_PalEnabled_Layout +
                                             API_PalEnabled_DocumentFrom3D,
                                         GSGuid2APIGuid (paletteGuid));
}
