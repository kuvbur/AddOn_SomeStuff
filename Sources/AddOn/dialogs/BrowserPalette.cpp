// *****************************************************************************
// Source code for the BrowserPalette class
// *****************************************************************************

// ---------------------------------- Includes ---------------------------------

#include "dialogs/BrowserPalette.hpp"

#include "CommonFunction.hpp"
#include "dialogs/SyncSettings.hpp"
#include "Propertycache.hpp"
#include "Sync.hpp"

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

    // Регистрируем функцию для парсинга описания свойства
    jsACAPI->AddItem (new DG::JSFunction ("ParsePropertyDescription", [] (GS::Ref<DG::JSBase> args) {
        // args[0] = description string
        if (args == nullptr) {
            GS::Ref<DG::JSObject> errorObj = new DG::JSObject ();
            errorObj->AddItem ("ok", new DG::JSValue (false));
            errorObj->AddItem ("error", new DG::JSValue ("Invalid arguments: expected array with description string"));
            return errorObj;
        }

        // Проверяем, что args - это JSArray
        GS::Ref<DG::JSArray> argsArray = GS::DynamicCast<DG::JSArray> (args);
        if (argsArray == nullptr) {
            GS::Ref<DG::JSObject> errorObj = new DG::JSObject ();
            errorObj->AddItem ("ok", new DG::JSValue (false));
            errorObj->AddItem ("error", new DG::JSValue ("First argument must be an array"));
            return errorObj;
        }

        const GS::Array<GS::Ref<DG::JSBase>> &argsItems = argsArray->GetItemArray ();
        if (argsItems.GetSize () < 1) {
            GS::Ref<DG::JSObject> errorObj = new DG::JSObject ();
            errorObj->AddItem ("ok", new DG::JSValue (false));
            errorObj->AddItem ("error", new DG::JSValue ("Missing description argument"));
            return errorObj;
        }

        // Получаем строку из JSValue
        GS::Ref<DG::JSValue> descValue = GS::DynamicCast<DG::JSValue> (argsItems[0]);
        if (descValue == nullptr) {
            GS::Ref<DG::JSObject> errorObj = new DG::JSObject ();
            errorObj->AddItem ("ok", new DG::JSValue (false));
            errorObj->AddItem ("error", new DG::JSValue ("First argument must be a string"));
            return errorObj;
        }

        GS::UniString description = descValue->GetString ();

        // Парсим описание
        GS::Array<ParsedPropertyCommand> commands;
        GS::UniString remainingText;
        bool hasCommands = ParsePropertyDescription (description, commands, remainingText);

        // Формируем результат
        GS::Ref<DG::JSObject> result = new DG::JSObject ();
        result->AddItem ("ok", new DG::JSValue (true));
        result->AddItem ("hasCommands", new DG::JSValue (hasCommands));
        result->AddItem ("remainingText", new DG::JSValue (remainingText.ToCStr ().Get ()));

        GS::Ref<DG::JSArray> commandsArray = new DG::JSArray ();
        for (const auto &cmd : commands) {
            GS::Ref<DG::JSObject> cmdObj = new DG::JSObject ();
            cmdObj->AddItem ("commandType", new DG::JSValue (cmd.commandType.ToCStr ().Get ()));
            cmdObj->AddItem ("fullCommand", new DG::JSValue (cmd.fullCommand.ToCStr ().Get ()));
            cmdObj->AddItem ("parameters", new DG::JSValue (cmd.parameters.ToCStr ().Get ()));
            cmdObj->AddItem ("isValid", new DG::JSValue (cmd.isValid));
            cmdObj->AddItem ("errorMessage", new DG::JSValue (cmd.errorMessage.ToCStr ().Get ()));
            commandsArray->AddItem (cmdObj);
        }
        result->AddItem ("commands", commandsArray);

        return result;
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