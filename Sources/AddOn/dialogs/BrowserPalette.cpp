// *****************************************************************************
// Source code for the BrowserPalette class
// *****************************************************************************

// ---------------------------------- Includes ---------------------------------

#include "dialogs/BrowserPalette.hpp"

#include "CommonFunction.hpp"
#include "dialogs/CommandHelpers.hpp"
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
    DBprnt ("BrowserPalette::BrowserPalette () — calling InitBrowserControl");
    InitBrowserControl ();
    DBprnt ("BrowserPalette::BrowserPalette () — InitBrowserControl done");
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
    DBprnt ("BrowserPalette::Show () called");
    DG::Palette::Show ();
    SyncSettings syncSettings;
    LoadSyncSettingsFromPreferences (syncSettings, true);
    syncSettings.SetShowPalette (true);
    MenuItemCheckAC (Menu_Pallete, syncSettings.GetShowPalette ());
    WriteSyncSettingsToPreferences (syncSettings);
    browser.ReloadIgnoreCache ();
    DBprnt ("BrowserPalette::Show () — after ReloadIgnoreCache");
    // Обновляем информацию о выделении после загрузки страницы
    // onLoadingStateChange вызовет RegisterACAPIJavaScriptObject и UpdateSelectionInfoInUI
}

void BrowserPalette::Hide () {
    DG::Palette::Hide ();
    SyncSettings syncSettings;
    LoadSyncSettingsFromPreferences (syncSettings, true);
    syncSettings.SetShowPalette (false);
    MenuItemCheckAC (Menu_Pallete, syncSettings.GetShowPalette ());
    WriteSyncSettingsToPreferences (syncSettings);
}

void BrowserPalette::UpdateSelectionInfoInUI () {
    GS::Array<API_Guid> selectedElements = GetSelectedElements2 (false, true);
    Int32 count = (Int32)selectedElements.GetSize ();
    DBprnt ("UpdateSelectionInfoInUI: count=" + GS::ValueToUniString (count));
    // Пушим данные в JS через ExecuteJS
    GS::UniString jsCall =
        GS::UniString ("if (typeof refreshSelectionInfoText === 'function') refreshSelectionInfoText(") +
        GS::ValueToUniString (count) + GS::UniString (");");
    browser.ExecuteJS (jsCall.ToCStr ().Get ());
    DBprnt ("UpdateSelectionInfoInUI: executed JS: " + jsCall);
}

void BrowserPalette::InitBrowserControl () {
    DBprnt ("BrowserPalette::InitBrowserControl () — loading HTML");
    // Загружаем HTML
    browser.LoadHTML (LoadHtmlFromResource ());
    DBprnt ("BrowserPalette::InitBrowserControl () — HTML load started");

    // Подписываемся на событие завершения загрузки страницы
    // Используем event notifier браузера
    browser.onLoadingStateChange +=
        [this] (const DG::BrowserBase & /*source*/, const DG::BrowserLoadingStateChangeArg &eventArg) {
            DBprnt ("BrowserPalette::onLoadingStateChange () — isLoading=" + GS::ValueToUniString (eventArg.isLoading) +
                    " canGoBack=" + GS::ValueToUniString (eventArg.canGoBack) +
                    " canGoForward=" + GS::ValueToUniString (eventArg.canGoForward));

            // Когда загрузка завершена — регистрируем JS-объект
            if (!eventArg.isLoading) {
                DBprnt ("BrowserPalette::onLoadingStateChange () — page loaded, registering JS");
                RegisterACAPIJavaScriptObject ();
                // Сразу обновляем информацию о выделении
                UpdateSelectionInfoInUI ();
            }
        };
}

void BrowserPalette::RegisterACAPIJavaScriptObject () {
    DBprnt ("BrowserPalette::RegisterACAPIJavaScriptObject () — starting");
    DG::JSObject *jsACAPI = new DG::JSObject ("ACAPI");

    // Регистрируем функцию для получения свойств выделенных элементов (возвращает JSON-строку для обхода ограничений
    // pull-паттерна)
    jsACAPI->AddItem (new DG::JSFunction ("GetPropertiesList", [] (GS::Ref<DG::JSBase>) -> GS::Ref<DG::JSBase> {
        // Собираем данные свойств
        GS::Array<API_Guid> selectedElements = GetSelectedElements2 (false, true);

        // Формируем JSON строку вручную
        GS::UniString jsonStr = "{ \"elements\": [";

        bool firstElement = true;
        if (!selectedElements.IsEmpty ()) {
            for (const API_Guid &elemGuid : selectedElements) {
                GS::Array<API_PropertyDefinition> definitions;
                GSErrCode err = ACAPI_Element_GetPropertyDefinitions (
                    elemGuid, API_PropertyDefinitionFilter_UserDefined, definitions);
                if (err != NoError || definitions.IsEmpty ()) {
                    continue;
                }

                GS::Array<API_Property> properties;
                err = ACAPI_Element_GetPropertyValues (elemGuid, definitions, properties);
                if (err != NoError) {
                    continue;
                }

                if (!firstElement) {
                    jsonStr += GS::UniString (",");
                }
                firstElement = false;

                jsonStr += GS::UniString ("{ \"guid\": \"") + APIGuidToString (elemGuid).ToCStr ().Get () +
                           GS::UniString ("\", \"properties\": [");

                bool firstProp = true;
                for (const API_Property &prop : properties) {
                    if (!firstProp) {
                        jsonStr += GS::UniString (",");
                    }
                    firstProp = false;

                    jsonStr += GS::UniString ("{");

                    if (!prop.definition.name.IsEmpty ()) {
                        jsonStr += GS::UniString ("\"name\": \"") + prop.definition.name.ToCStr ().Get () +
                                   GS::UniString ("\",");
                    } else {
                        jsonStr += GS::UniString ("\"name\": \"\",");
                    }

                    ParamValue pvalue;
                    if (ParamHelpers::ConvertToParamValue (pvalue, prop)) {
                        GS::UniString valueStr;
                        switch (pvalue.val.type) {
                        case API_PropertyIntegerValueType:
                            valueStr = GS::UniString::Printf ("%d", pvalue.val.intValue);
                            break;
                        case API_PropertyRealValueType:
                            valueStr = GS::UniString::Printf ("%.3f", pvalue.val.doubleValue);
                            break;
                        case API_PropertyStringValueType:
                            valueStr = pvalue.val.uniStringValue;
                            break;
                        case API_PropertyBooleanValueType:
                            valueStr = pvalue.val.boolValue ? "true" : "false";
                            break;
                        case API_PropertyGuidValueType:
                            valueStr = APIGuidToString (pvalue.val.guidval).ToCStr ().Get ();
                            break;
                        default:
                            valueStr = "unknown";
                            break;
                        }
                        jsonStr += GS::UniString ("\"value\": \"") + valueStr.ToCStr ().Get () + GS::UniString ("\",");
                    } else {
                        jsonStr += "\"value\": \"\",";
                    }

                    const char *typeStr = "unknown";
                    switch (prop.definition.valueType) {
                    case API_PropertyIntegerValueType:
                        typeStr = "integer";
                        break;
                    case API_PropertyRealValueType:
                        typeStr = "real";
                        break;
                    case API_PropertyStringValueType:
                        typeStr = "string";
                        break;
                    case API_PropertyBooleanValueType:
                        typeStr = "boolean";
                        break;
                    case API_PropertyGuidValueType:
                        typeStr = "guid";
                        break;
                    case API_PropertyUndefinedValueType:
                        typeStr = "undefined";
                        break;
                    }
                    jsonStr += GS::UniString ("\"valueType\": \"") + GS::UniString (typeStr) + GS::UniString ("\",");

                    jsonStr += GS::UniString ("\"propertyGuid\": \"") +
                               APIGuidToString (prop.definition.guid).ToCStr ().Get () + GS::UniString ("\"");

                    jsonStr += "}";
                }

                jsonStr += "]}";
            }
        }

        jsonStr += "], \"count\": 0 }";

        return new DG::JSValue (jsonStr);
    }));

    // Регистрируем функцию для получения количества выделенных элементов
    jsACAPI->AddItem (new DG::JSFunction ("GetSelectionInfo", [] (GS::Ref<DG::JSBase>) {
        DBprnt ("GetSelectionInfo: function called from JS");
        GS::Array<API_Guid> selectedElements = GetSelectedElements2 (false, true);
        Int32 count = (Int32)selectedElements.GetSize ();
        DBprnt ("GetSelectionInfo: GetSelectedElements2 returned " + GS::ValueToUniString (count) + " elements");
        GS::Ref<DG::JSObject> result = new DG::JSObject ();
        result->AddItem ("count", new DG::JSValue (count));
        DBprnt ("GetSelectionInfo: returning count=" + GS::ValueToUniString (count));
        return result;
    }));

    // Отладочная функция — проверяет, что JS-мост работает
    jsACAPI->AddItem (new DG::JSFunction ("Ping", [] (GS::Ref<DG::JSBase>) {
        DBprnt ("Ping: JavaScript Bridge is working!");
        GS::Ref<DG::JSObject> result = new DG::JSObject ();
        result->AddItem ("ok", new DG::JSValue (true));
        result->AddItem ("message", new DG::JSValue ("Bridge is alive"));
        return result;
    }));

    // Обновление количества выделенных элементов в UI (вызывается из JS)
    jsACAPI->AddItem (new DG::JSFunction ("RefreshSelectionInfoUI", [this] (GS::Ref<DG::JSBase>) {
        DBprnt ("RefreshSelectionInfoUI: called from JS");
        // Обновляем UI через push (ExecuteJS)
        UpdateSelectionInfoInUI ();
        // Возвращаем пустое значение, так как UI уже обновлён
        return GS::Ref<DG::JSBase> (nullptr);
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

    // Регистрируем функцию для парсинга описания свойства с привязкой к элементу
    jsACAPI->AddItem (new DG::JSFunction ("ParsePropertyForElement", [] (GS::Ref<DG::JSBase> args) {
        // args[0] = description string, args[1] = elemGuid string
        if (args == nullptr) {
            GS::Ref<DG::JSObject> errorObj = new DG::JSObject ();
            errorObj->AddItem ("ok", new DG::JSValue (false));
            errorObj->AddItem ("error",
                               new DG::JSValue ("Invalid arguments: expected array with [description, elemGuid]"));
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
        if (argsItems.GetSize () < 2) {
            GS::Ref<DG::JSObject> errorObj = new DG::JSObject ();
            errorObj->AddItem ("ok", new DG::JSValue (false));
            errorObj->AddItem ("error", new DG::JSValue ("Missing arguments: expected [description, elemGuid]"));
            return errorObj;
        }

        // Получаем строку описания
        GS::Ref<DG::JSValue> descValue = GS::DynamicCast<DG::JSValue> (argsItems[0]);
        if (descValue == nullptr) {
            GS::Ref<DG::JSObject> errorObj = new DG::JSObject ();
            errorObj->AddItem ("ok", new DG::JSValue (false));
            errorObj->AddItem ("error", new DG::JSValue ("First argument must be a string (description)"));
            return errorObj;
        }

        // Получаем GUID элемента
        GS::Ref<DG::JSValue> guidValue = GS::DynamicCast<DG::JSValue> (argsItems[1]);
        if (guidValue == nullptr) {
            GS::Ref<DG::JSObject> errorObj = new DG::JSObject ();
            errorObj->AddItem ("ok", new DG::JSValue (false));
            errorObj->AddItem ("error", new DG::JSValue ("Second argument must be a string (elemGuid)"));
            return errorObj;
        }

        GS::UniString description = descValue->GetString ();
        GS::UniString elemGuidStr = guidValue->GetString ();
        API_Guid elemGuid = APIGuidFromString (elemGuidStr.ToCStr (0, MaxUSize, GChCode));

        if (elemGuid == APINULLGuid) {
            GS::Ref<DG::JSObject> errorObj = new DG::JSObject ();
            errorObj->AddItem ("ok", new DG::JSValue (false));
            errorObj->AddItem ("error", new DG::JSValue ("Invalid GUID format"));
            return errorObj;
        }

        // Парсим описание через новую функцию
        ParsePropertyResult parseResult = ParsePropertyDescriptionToRules (description);

        // Формируем результат
        GS::Ref<DG::JSObject> result = new DG::JSObject ();
        result->AddItem ("ok", new DG::JSValue (true));
        result->AddItem ("hasSyncRules", new DG::JSValue (parseResult.hasSyncRules));
        result->AddItem ("hasOtherCommands", new DG::JSValue (parseResult.hasOtherCommands));
        result->AddItem ("remainingText", new DG::JSValue (parseResult.remainingText.ToCStr ().Get ()));

        // syncRules массив
        GS::Ref<DG::JSArray> syncRulesArray = new DG::JSArray ();
        for (const auto &rule : parseResult.syncRules) {
            GS::Ref<DG::JSObject> ruleObj = new DG::JSObject ();
            ruleObj->AddItem ("commandType", new DG::JSValue (rule.commandType.ToCStr ().Get ()));
            ruleObj->AddItem ("fullCommand", new DG::JSValue (rule.fullCommand.ToCStr ().Get ()));
            ruleObj->AddItem ("parameters", new DG::JSValue (rule.parameters.ToCStr ().Get ()));
            ruleObj->AddItem ("sourceType", new DG::JSValue (rule.sourceType.ToCStr ().Get ()));
            ruleObj->AddItem ("sourceName", new DG::JSValue (rule.sourceName.ToCStr ().Get ()));
            ruleObj->AddItem ("targetType", new DG::JSValue (rule.targetType.ToCStr ().Get ()));
            ruleObj->AddItem ("targetName", new DG::JSValue (rule.targetName.ToCStr ().Get ()));
            ruleObj->AddItem ("formatString", new DG::JSValue (rule.formatString.ToCStr ().Get ()));
            ruleObj->AddItem ("isValid", new DG::JSValue (rule.isValid));
            ruleObj->AddItem ("errorMessage", new DG::JSValue (rule.errorMessage.ToCStr ().Get ()));
            ruleObj->AddItem ("hasSub", new DG::JSValue (rule.hasSub));
            ruleObj->AddItem ("hasGUID", new DG::JSValue (rule.hasGUID));
            ruleObj->AddItem ("guidSourceProperty", new DG::JSValue (rule.guidSourceProperty.ToCStr ().Get ()));

            // ignoreVals
            GS::Ref<DG::JSArray> ignoreValsArray = new DG::JSArray ();
            for (const auto &iv : rule.ignoreVals) {
                ignoreValsArray->AddItem (new DG::JSValue (iv.ToCStr ().Get ()));
            }
            ruleObj->AddItem ("ignoreVals", ignoreValsArray);

            syncRulesArray->AddItem (ruleObj);
        }
        result->AddItem ("syncRules", syncRulesArray);

        // otherCommands массив
        GS::Ref<DG::JSArray> otherCommandsArray = new DG::JSArray ();
        for (const auto &cmd : parseResult.otherCommands) {
            GS::Ref<DG::JSObject> cmdObj = new DG::JSObject ();
            cmdObj->AddItem ("commandType", new DG::JSValue (cmd.commandType.ToCStr ().Get ()));
            cmdObj->AddItem ("fullCommand", new DG::JSValue (cmd.fullCommand.ToCStr ().Get ()));
            cmdObj->AddItem ("parameters", new DG::JSValue (cmd.parameters.ToCStr ().Get ()));
            cmdObj->AddItem ("isValid", new DG::JSValue (cmd.isValid));
            cmdObj->AddItem ("errorMessage", new DG::JSValue (cmd.errorMessage.ToCStr ().Get ()));
            otherCommandsArray->AddItem (cmdObj);
        }
        result->AddItem ("otherCommands", otherCommandsArray);

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