// *****************************************************************************
// Source code for the BrowserPalette class
// *****************************************************************************

// ---------------------------------- Includes ---------------------------------

#include "ACAPinc.h"

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

void BrowserPalette::UpdateSelectionInfoInUI (GS::Array<API_Guid> &selectedElements) {
    // Если массив пуст — получаем текущее выделение
    if (selectedElements.IsEmpty ()) {
        selectedElements = GetSelectedElements2 (false, true);
    }
    Int32 count = (Int32)selectedElements.GetSize ();
    DBprnt ("UpdateSelectionInfoInUI: count=" + GS::ValueToUniString (count));
    // Пушим данные в JS через ExecuteJS
    GS::UniString jsCall =
        GS::UniString ("if (typeof refreshSelectionInfoText === 'function') refreshSelectionInfoText(") +
        GS::ValueToUniString (count) + GS::UniString (");");
    browser.ExecuteJS (jsCall.ToCStr ().Get ());
    DBprnt ("UpdateSelectionInfoInUI: executed JS: " + jsCall);

    // Также перерисовываем список свойств при смене выделения
    GS::UniString jsCallProps =
        "if (typeof renderPropertyList === 'function') renderPropertyList(document.getElementById('monitor-list-block'), document.querySelector('[data-role=\"value-block\"]'), '');";
    browser.ExecuteJS (jsCallProps.ToCStr ().Get ());
    DBprnt ("UpdateSelectionInfoInUI: executed JS for property list refresh");
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
                GS::Array<API_Guid> selectedElements;
                UpdateSelectionInfoInUI (selectedElements);
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

                    // Получаем имя группы свойства
                    GS::UniString groupName = "Без группы";
                    if (prop.definition.groupGuid != APINULLGuid) {
                        API_PropertyGroup group;
                        group.guid = prop.definition.groupGuid;
                        GSErrCode groupErr = ACAPI_Property_GetPropertyGroup (group);
                        DBprnt (GS::UniString ("GetPropertiesList: prop=") + prop.definition.name.ToCStr ().Get () +
                                GS::UniString (", groupGuid=") +
                                APIGuidToString (prop.definition.groupGuid).ToCStr ().Get () +
                                GS::UniString (", groupErr=") + GS::ValueToUniString (groupErr) +
                                GS::UniString (", groupName=") + group.name.ToCStr ().Get ());
                        if (groupErr == NoError && !group.name.IsEmpty ()) {
                            groupName = group.name;
                        }
                    }
                    jsonStr += GS::UniString ("\"group\": \"") + groupName.ToCStr ().Get () + GS::UniString ("\",");

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

        jsonStr += "], \"count\": " + GS::ValueToUniString ((Int32)selectedElements.GetSize ()) + GS::UniString (" }");

        DBprnt (GS::UniString ("GetPropertiesList: returning JSON: ") + jsonStr);

        return new DG::JSValue (jsonStr);
    }));

    // Регистрируем функцию для получения значения свойства для выделенных элементов
    jsACAPI->AddItem (new DG::JSFunction ("GetPropertyValue", [] (GS::Ref<DG::JSBase> args) -> GS::Ref<DG::JSBase> {
        if (args == nullptr) {
            GS::Ref<DG::JSObject> errorObj = new DG::JSObject ();
            errorObj->AddItem ("status", new DG::JSValue ("error"));
            errorObj->AddItem ("message", new DG::JSValue ("Invalid arguments: expected array with propertyId"));
            return errorObj;
        }

        GS::Ref<DG::JSArray> argsArray = GS::DynamicCast<DG::JSArray> (args);
        if (argsArray == nullptr) {
            GS::Ref<DG::JSObject> errorObj = new DG::JSObject ();
            errorObj->AddItem ("status", new DG::JSValue ("error"));
            errorObj->AddItem ("message", new DG::JSValue ("First argument must be an array"));
            return errorObj;
        }

        const GS::Array<GS::Ref<DG::JSBase>> &argsItems = argsArray->GetItemArray ();
        if (argsItems.GetSize () < 1) {
            GS::Ref<DG::JSObject> errorObj = new DG::JSObject ();
            errorObj->AddItem ("status", new DG::JSValue ("error"));
            errorObj->AddItem ("message", new DG::JSValue ("Expected propertyId as first argument"));
            return errorObj;
        }

        GS::Ref<DG::JSValue> propertyIdVal = GS::DynamicCast<DG::JSValue> (argsItems[0]);
        if (propertyIdVal == nullptr) {
            GS::Ref<DG::JSObject> errorObj = new DG::JSObject ();
            errorObj->AddItem ("status", new DG::JSValue ("error"));
            errorObj->AddItem ("message", new DG::JSValue ("propertyId must be a string"));
            return errorObj;
        }

        GS::UniString propertyId = propertyIdVal->GetString ();

        // Inline implementation (like GetPropertiesList)
        GS::Array<API_Guid> selectedElements = GetSelectedElements2 (false, true);

        GS::UniString jsonStr = "{ \"values\": [";

        bool firstValue = true;
        GS::HashTable<GS::UniString, Int32> valueCounts;

        for (const API_Guid &elemGuid : selectedElements) {
            (void)elemGuid; // подавление warning unused variable
            GS::UniString rawName = "Property:" + propertyId;
            ParamValue pvalue;
            if (ParamHelpers::GetParamValueFromCache (rawName, pvalue)) {
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

                const Int32 *currentCountPtr = valueCounts.GetPtr (valueStr);
                Int32 currentCount = currentCountPtr ? *currentCountPtr : 0;
                valueCounts.Put (valueStr, currentCount + 1);
            }
        }

        // Формируем JSON
        Int32 totalCount = 0;
        Int32 uniqueValuesCount = 0;
        for (auto it = valueCounts.EnumeratePairs (); it != nullptr; ++it) {
#if defined(ServerMainVers_2800) || defined(ServerMainVers_2900)
            const GS::UniString &key = it->key;
            Int32 value = it->value;
#else
                                    const GS::UniString &key = *it->key;
                                    Int32 value = *it->value;
#endif
            if (!firstValue) {
                jsonStr += ",";
            }
            firstValue = false;
            jsonStr += GS::UniString ("{\"value\":\"") + key.ToCStr ().Get () + GS::UniString ("\",\"count\":") +
                       GS::ValueToUniString (value) + GS::UniString ("}");
            totalCount += value;
            uniqueValuesCount++;
        }

        jsonStr += "], ";

        bool isCommon = (uniqueValuesCount == 1 && totalCount == selectedElements.GetSize ());

        jsonStr += GS::UniString ("\"common\": ") + GS::UniString (isCommon ? "true" : "false") + GS::UniString (", ");
        jsonStr += GS::UniString ("\"propertyName\": \"") + propertyId.ToCStr ().Get () + GS::UniString ("\", ");
        jsonStr += GS::UniString ("\"status\": \"ok\" }");

        DBprnt (GS::UniString ("GetPropertyValue: returning JSON: ") + jsonStr);

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
        GS::Array<API_Guid> selectedElements;
        UpdateSelectionInfoInUI (selectedElements);
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

    // Регистрируем функцию для получения классификации выделенных элементов (inline implementation)
        jsACAPI->AddItem (new DG::JSFunction ("GetClassification", [] (GS::Ref<DG::JSBase>) -> GS::Ref<DG::JSBase> {
            try {
                DBprnt ("GetClassification: [1] function called from JS");

                            GS::Array<API_Guid> selectedElements = GetSelectedElements2 (false, true);
                            DBprnt (GS::UniString::Printf ("GetClassification: [2] selectedElements count = %d", selectedElements.GetSize ()));

                            if (selectedElements.IsEmpty ()) {
                                DBprnt ("GetClassification: [3] no selected elements, returning empty");
                                GS::Ref<DG::JSObject> jsResult = new DG::JSObject ();
                                jsResult->AddItem ("common", new DG::JSValue (true));
                                jsResult->AddItem ("commonPath", new DG::JSArray ());
                                jsResult->AddItem ("differing", new DG::JSArray ());
                                jsResult->AddItem ("options", new DG::JSArray ());
                                jsResult->AddItem ("status", new DG::JSValue ("ok"));
                                return jsResult;
                            }

                            // Убедимся, что классификации загружены в кэш
                            DBprnt ("GetClassification: [4] calling ReadSystemDict");
                            auto &cache = PROPERTYCACHE ();
                            // Принудительно инициализируем классификацию, если она не загружена
                            if (!cache.isClassificationRead) {
                                DBprnt ("GetClassification: [4.1] forcing ReadClassification");
                                cache.ReadClassification ();
                            }
                            bool readResult = ClassificationFunc::ReadSystemDict ();
                            DBprnt (GS::UniString::Printf ("GetClassification: [5] ReadSystemDict result = %d, isClassification_OK = %d",
                                                           readResult, cache.isClassification_OK));

                            // ПРЯМАЯ ПРОВЕРКА: вызываем GetAllClassification и смотрим результат
                            GS::HashTable<GS::UniString, ClassificationFunc::ClassificationDict> testSystemDict;
                            GSErrCode directErr = ClassificationFunc::GetAllClassification (testSystemDict);
                            DBprnt (GS::UniString::Printf ("GetClassification: [5.1] DIRECT GetAllClassification err=%d, size=%d",
                                                           directErr, testSystemDict.GetSize ()));

                            if (!readResult) {
                                DBprnt ("GetClassification: [5] failed to load classification system");
                                GS::Ref<DG::JSObject> errorObj = new DG::JSObject ();
                                errorObj->AddItem ("common", new DG::JSValue (true));
                                errorObj->AddItem ("commonPath", new DG::JSArray ());
                                errorObj->AddItem ("differing", new DG::JSArray ());
                                errorObj->AddItem ("options", new DG::JSArray ());
                                errorObj->AddItem ("status", new DG::JSValue ("error"));
                                errorObj->AddItem ("debug_error", new DG::JSValue ("ReadSystemDict failed"));
                                // Диагностика для ошибки
                                errorObj->AddItem ("_build_id", new DG::JSValue ("BUILD_2026_08_11_13_38_V3"));
                                errorObj->AddItem ("diag_isClassificationRead", new DG::JSValue (cache.isClassificationRead));
                                errorObj->AddItem ("diag_isClassification_OK", new DG::JSValue (cache.isClassification_OK));
                                return errorObj;
                            }
                            DBprnt ("GetClassification: [6] ReadSystemDict succeeded");

                            // auto &cache = PROPERTYCACHE (); // УЖЕ ОБЪЯВЛЕНО ВЫШЕ
                            DBprnt (GS::UniString::Printf ("GetClassification: [7] systemdict size = %d", cache.systemdict.GetSize ()));
                            DBprnt (GS::UniString::Printf ("GetClassification: [8] reversesystemdict size = %d", cache.reversesystemdict.GetSize ()));

                            // Собираем классификацию для каждого выделенного элемента
                            GS::HashTable<GS::Pair<API_Guid, API_Guid>, Int32> classificationCounts; // systemGuid+itemGuid -> count
                            GS::HashTable<GS::Pair<API_Guid, API_Guid>, GS::UniString>
                                classificationDisplayNames; // systemGuid+itemGuid -> display name

                            for (const API_Guid &elemGuid : selectedElements) {
                                GS::Array<GS::Pair<API_Guid, API_Guid>> systemItemPairs;
                                GSErrCode err = ACAPI_Element_GetClassificationItems (elemGuid, systemItemPairs);
                                DBprnt (GS::UniString::Printf ("GetClassification: [9] element has %d classifications, err=%d", systemItemPairs.GetSize (), err));
                                if (err != NoError || systemItemPairs.IsEmpty ()) {
                                    continue; // Элемент без классификации или ошибка
                                }

                                for (const auto &pair : systemItemPairs) {
                                    const GS::Pair<API_Guid, API_Guid> key = pair;
                                    const Int32 *currentCountPtr = classificationCounts.GetPtr (key);
                                    Int32 currentCount = currentCountPtr ? *currentCountPtr : 0;
                                    classificationCounts.Put (key, currentCount + 1);

                                    // Получаем отображаемое имя для этого класса
                                    if (!classificationDisplayNames.ContainsKey (key)) {
                                        // Ищем класс в systemdict по GUID
                                        GS::UniString displayName;
                                        bool found = false;
                                        for (const auto& sysPair : cache.systemdict) {
                                            for (const auto& classPair : *sysPair.value) {
                                                if (classPair.value->item.guid == pair.second) {
                                                    ClassificationFunc::GetFullName(
                                                        classPair.value->item, *sysPair.value, displayName);
                                                    classificationDisplayNames.Put(key, displayName);
                                                    found = true;
                                                    break;
                                                }
                                            }
                                            if (found) break;
                                        }
                                        if (!found) {
                                            classificationDisplayNames.Put(key, GS::UniString("Unknown"));
                                        }
                                        DBprnt (GS::UniString::Printf ("GetClassification: [10] found class for key -> %s", displayName.ToCStr().Get()));
                                    }
                                }
                            }

                            DBprnt (GS::UniString::Printf ("GetClassification: [11] classificationCounts size = %d", classificationCounts.GetSize ()));

            // Определяем общий путь классификации (все элементы имеют одинаковые классы)
            bool isCommon = true;
            GS::Array<GS::UniString> commonPathSegments; // сегменты общего пути (для дерева)
            GS::Array<GS::ObjectState> differing;

            if (classificationCounts.IsEmpty ()) {
                // Ни у одного элемента нет классификации
                isCommon = true;
                commonPathSegments = GS::Array<GS::UniString> ();
                DBprnt ("GetClassification: [12] classificationCounts is EMPTY - no classifications found");
            } else {
                // Собираем классы, которые есть у ВСЕХ элементов (count == selectedElements.GetSize())
                GS::Array<GS::UniString> commonFullNames; // полные имена общих классов

                DBprnt (GS::UniString::Printf ("GetClassification: [12] iterating classificationCounts, size=%d", classificationCounts.GetSize ()));

                // Итерация по classificationCounts — правильный паттерн для GS::HashTable
                for (auto it = classificationCounts.EnumeratePairs (); it != nullptr; ++it) {
#if defined(ServerMainVers_2800) || defined(ServerMainVers_2900)
                    const GS::Pair<API_Guid, API_Guid> &key = it->key;
                    Int32 count = it->value;
#else
                    const GS::Pair<API_Guid, API_Guid> &key = *it->key;
                    Int32 count = *it->value;
#endif
                    GS::UniString displayName;
                    if (classificationDisplayNames.GetPtr (key)) {
                        displayName = *classificationDisplayNames.GetPtr (key);
                    } else {
                        displayName = "Unknown";
                    }

                    DBprnt (GS::UniString::Printf ("GetClassification: key=%s, count=%d, total=%d, name=%s",
                                                   APIGuidToString (key.second).ToCStr ().Get (), count,
                                                   selectedElements.GetSize (), displayName.ToCStr ().Get ()));

                    if (count == selectedElements.GetSize ()) {
                        commonFullNames.Push (displayName);
                    } else {
                        isCommon = false;
                        GS::ObjectState diffObj;
                        diffObj.Add ("classification", displayName);
                        diffObj.Add ("count", count);
                        diffObj.Add ("total", selectedElements.GetSize ());
                        differing.Push (diffObj);
                    }
                }

                // Формируем commonPathSegments как сегменты пути из первого общего класса
                if (!commonFullNames.IsEmpty ()) {
                    // GetFullName возвращает "System > Group > Item", разбиваем по " > "
                    GS::UniString firstName = commonFullNames[0];
                    GS::Array<GS::UniString> segments;
                    firstName.Split (" > ", &segments);
                    for (const GS::UniString &seg : segments) {
                        if (!seg.IsEmpty ()) {
                            commonPathSegments.Push (seg);
                        }
                    }
                    DBprnt (GS::UniString::Printf ("GetClassification: commonPathSegments count = %d", commonPathSegments.GetSize ()));
                }
            }

            // Собираем список всех доступных классификаций (опции для выбора)
            GS::Array<GS::UniString> options;
            auto &systemdict = cache.systemdict;
            for (const auto &sysPair : systemdict) {
                const ClassificationFunc::ClassificationDict *classDict = sysPair.value;
                if (classDict == nullptr) continue;
                for (const auto &classPair : *classDict) {
                    const ClassificationFunc::ClassificationValues *cv = classPair.value;
                    if (cv == nullptr) continue;
                    GS::UniString fullName;
                    ClassificationFunc::GetFullName (cv->item, *classDict, fullName);
                    options.Push (fullName);
                }
            }

            DBprnt (GS::UniString::Printf ("GetClassification: [result] isCommon=%d, commonPathSegments=%d, differing=%d",
                                             isCommon, commonPathSegments.GetSize (), differing.GetSize ()));

            // Отладка: выводим первые элементы differing
            for (const auto& d : differing) {
                GS::UniString cls;
                Int32 cnt, tot;
                d.Get ("classification", cls);
                d.Get ("count", cnt);
                d.Get ("total", tot);
                DBprnt (GS::UniString::Printf ("  differing item: classification=%s, count=%d, total=%d",
                                               cls.ToCStr ().Get (), cnt, tot));
            }

            // Формируем JS результат
            GS::Ref<DG::JSObject> jsResult = new DG::JSObject ();
            jsResult->AddItem ("common", new DG::JSValue (isCommon));
            // Количество выделенных элементов — для отладки
            jsResult->AddItem ("selectedCount", new DG::JSValue ((Int32)selectedElements.GetSize ()));
            // Диагностика системы классификации
            jsResult->AddItem ("diag_systemdictSize", new DG::JSValue ((Int32)cache.systemdict.GetSize ()));
            jsResult->AddItem ("diag_reversesystemdictSize", new DG::JSValue ((Int32)cache.reversesystemdict.GetSize ()));
            jsResult->AddItem ("diag_isClassification_OK", new DG::JSValue (cache.isClassification_OK));
            jsResult->AddItem ("diag_isClassificationRead", new DG::JSValue (cache.isClassificationRead));

            // ВРЕМЕННАЯ МЕТКА ВЕРСИИ — чтобы убедиться, что новый код загружен
            jsResult->AddItem ("_build_id", new DG::JSValue ("BUILD_2026_08_11_13_20_V2"));

            // DEBUG: добавляем отладочные поля в ответ
            jsResult->AddItem ("debug_classificationCountsSize", new DG::JSValue ((Int32)classificationCounts.GetSize ()));
            jsResult->AddItem ("debug_systemdictSize", new DG::JSValue ((Int32)cache.systemdict.GetSize ()));
            jsResult->AddItem ("debug_reversesystemdictSize", new DG::JSValue ((Int32)cache.reversesystemdict.GetSize ()));
            jsResult->AddItem ("debug_selectedCount", new DG::JSValue ((Int32)selectedElements.GetSize ()));

            GS::Ref<DG::JSArray> commonPathArray = new DG::JSArray ();
            for (const GS::UniString &s : commonPathSegments) {
                commonPathArray->AddItem (new DG::JSValue (s.ToCStr ().Get ()));
            }
            jsResult->AddItem ("commonPath", commonPathArray);

            GS::Ref<DG::JSArray> differingArray = new DG::JSArray ();
            for (const GS::ObjectState &diff : differing) {
                GS::Ref<DG::JSObject> diffObj = new DG::JSObject ();
                GS::UniString classification;
                Int32 count, total;
                if (diff.Get ("classification", classification))
                    diffObj->AddItem ("classification", new DG::JSValue (classification.ToCStr ().Get ()));
                if (diff.Get ("count", count))
                    diffObj->AddItem ("count", new DG::JSValue (count));
                if (diff.Get ("total", total))
                    diffObj->AddItem ("total", new DG::JSValue (total));
                differingArray->AddItem (diffObj);
            }
            jsResult->AddItem ("differing", differingArray);

            GS::Ref<DG::JSArray> optionsArray = new DG::JSArray ();
            for (const GS::UniString &s : options) {
                optionsArray->AddItem (new DG::JSValue (s.ToCStr ().Get ()));
            }
            jsResult->AddItem ("options", optionsArray);

            jsResult->AddItem ("status", new DG::JSValue ("ok"));

            DBprnt ("GetClassification: returning result");
            return jsResult;
        } catch (const std::exception &e) {
            DBprnt (GS::UniString ("GetClassification: std::exception: ") + e.what ());
            GS::Ref<DG::JSObject> errorObj = new DG::JSObject ();
            errorObj->AddItem ("common", new DG::JSValue (true));
            errorObj->AddItem ("commonPath", new DG::JSArray ());
            errorObj->AddItem ("differing", new DG::JSArray ());
            errorObj->AddItem ("options", new DG::JSArray ());
            errorObj->AddItem ("status", new DG::JSValue ("error"));
            errorObj->AddItem ("debug_error", new DG::JSValue ("std::exception"));
            errorObj->AddItem ("debug_exception", new DG::JSValue (e.what ()));
            // Диагностика в catch
            errorObj->AddItem ("_build_id", new DG::JSValue ("BUILD_2026_08_11_14_10_CATCH"));
            return errorObj;
        } catch (...) {
            DBprnt ("GetClassification: unknown exception caught");
            GS::Ref<DG::JSObject> errorObj = new DG::JSObject ();
            errorObj->AddItem ("common", new DG::JSValue (true));
            errorObj->AddItem ("commonPath", new DG::JSArray ());
            errorObj->AddItem ("differing", new DG::JSArray ());
            errorObj->AddItem ("options", new DG::JSArray ());
            errorObj->AddItem ("status", new DG::JSValue ("error"));
            errorObj->AddItem ("debug_error", new DG::JSValue ("unknown exception"));
            // Диагностика в catch
            errorObj->AddItem ("_build_id", new DG::JSValue ("BUILD_2026_08_11_14_10_CATCH2"));
            return errorObj;
        }
    }));

    // Регистрируем функцию для назначения классификации выделенным элементам (inline implementation)
    jsACAPI->AddItem (new DG::JSFunction ("SetClassification", [] (GS::Ref<DG::JSBase> args) -> GS::Ref<DG::JSBase> {
        try {
            if (args == nullptr) {
                GS::Ref<DG::JSObject> errorObj = new DG::JSObject ();
                errorObj->AddItem ("success", new DG::JSValue (false));
                errorObj->AddItem ("message",
                                   new DG::JSValue ("Invalid arguments: expected array with classificationValue"));
                return errorObj;
            }

            GS::Ref<DG::JSArray> argsArray = GS::DynamicCast<DG::JSArray> (args);
            if (argsArray == nullptr) {
                GS::Ref<DG::JSObject> errorObj = new DG::JSObject ();
                errorObj->AddItem ("success", new DG::JSValue (false));
                errorObj->AddItem ("message", new DG::JSValue ("First argument must be an array"));
                return errorObj;
            }

            const GS::Array<GS::Ref<DG::JSBase>> &argsItems = argsArray->GetItemArray ();
            if (argsItems.GetSize () < 1) {
                GS::Ref<DG::JSObject> errorObj = new DG::JSObject ();
                errorObj->AddItem ("success", new DG::JSValue (false));
                errorObj->AddItem ("message", new DG::JSValue ("Expected classificationValue as first argument"));
                return errorObj;
            }

            GS::Ref<DG::JSValue> classificationValueVal = GS::DynamicCast<DG::JSValue> (argsItems[0]);
            if (classificationValueVal == nullptr) {
                GS::Ref<DG::JSObject> errorObj = new DG::JSObject ();
                errorObj->AddItem ("success", new DG::JSValue (false));
                errorObj->AddItem ("message", new DG::JSValue ("classificationValue must be a string"));
                return errorObj;
            }

            GS::UniString classificationValue = classificationValueVal->GetString ();
            DBprnt ("SetClassification: called with classificationValue=" + classificationValue);

            // Получаем GUID-ы выделенных элементов
            GS::Array<API_Guid> selectedElements = GetSelectedElements2 (false, true);

            if (selectedElements.IsEmpty ()) {
                GS::Ref<DG::JSObject> errorObj = new DG::JSObject ();
                errorObj->AddItem ("success", new DG::JSValue (false));
                errorObj->AddItem ("message", new DG::JSValue ("No elements selected"));
                return errorObj;
            }

            // Убедимся, что классификации загружены в кэш
            if (!ClassificationFunc::ReadSystemDict ()) {
                GS::Ref<DG::JSObject> errorObj = new DG::JSObject ();
                errorObj->AddItem ("success", new DG::JSValue (false));
                errorObj->AddItem ("message", new DG::JSValue ("Failed to load classification systems"));
                return errorObj;
            }

            auto &cache = PROPERTYCACHE ();

            // Ищем класс по полному имени во всех системах
            API_Guid targetSystemGuid = APINULLGuid;
            API_Guid targetItemGuid = APINULLGuid;
            bool found = false;

            for (const auto &sysPair : cache.systemdict) {
                const ClassificationFunc::ClassificationDict *classDict = sysPair.value;
                for (const auto &classPair : *classDict) {
                    const ClassificationFunc::ClassificationValues *cv = classPair.value;
                    GS::UniString fullName;
                    ClassificationFunc::GetFullName (cv->item, *classDict, fullName);
                    if (fullName == classificationValue) {
                        targetSystemGuid = cv->system.guid;
                        targetItemGuid = cv->item.guid;
                        found = true;
                        break;
                    }
                }
                if (found)
                    break;
            }

            if (!found) {
                // Попробуем найти по GUID, если передан в формате "systemGuid:itemGuid"
                GS::Array<GS::UniString> parts;
                classificationValue.Split (":", &parts);
                if (parts.GetSize () == 2) {
                    targetSystemGuid = APIGuidFromString (parts[0].ToCStr ().Get ());
                    targetItemGuid = APIGuidFromString (parts[1].ToCStr ().Get ());
                    if (targetSystemGuid != APINULLGuid && targetItemGuid != APINULLGuid) {
                        found = true;
                    }
                }
            }

            if (!found) {
                GS::Ref<DG::JSObject> errorObj = new DG::JSObject ();
                errorObj->AddItem ("success", new DG::JSValue (false));
                errorObj->AddItem ("message",
                                   new DG::JSValue (GS::UniString ("Classification not found: ") +
                                                    classificationValue.ToCStr ().Get ()));
                return errorObj;
            }

            // Назначаем классификацию каждому выделенному элементу
            Int32 successCount = 0;
            Int32 errorCount = 0;

            for (const API_Guid &elemGuid : selectedElements) {
                // Сначала проверяем, есть ли уже эта классификация у элемента
                GS::Array<GS::Pair<API_Guid, API_Guid>> systemItemPairs;
                GSErrCode err = ACAPI_Element_GetClassificationItems (elemGuid, systemItemPairs);
                if (err != NoError) {
                    // Если ошибка при чтении, пробуем просто добавить
                }

                bool alreadyHas = false;
                for (const auto &pair : systemItemPairs) {
                    if (pair.first == targetSystemGuid && pair.second == targetItemGuid) {
                        alreadyHas = true;
                        break;
                    }
                }

                if (alreadyHas) {
                    successCount++;
                    continue;
                }

                // Добавляем классификацию
                err = ACAPI_Element_AddClassificationItem (elemGuid, targetItemGuid);
                if (err == NoError) {
                    successCount++;
                } else {
                    errorCount++;
                    msg_rep ("SetClassificationCommand", "ACAPI_Element_AddClassificationItem", err, elemGuid);
                }
            }

            GS::Ref<DG::JSObject> jsResult = new DG::JSObject ();
            jsResult->AddItem ("success", new DG::JSValue (errorCount == 0));
            jsResult->AddItem ("successCount", new DG::JSValue (successCount));
            jsResult->AddItem ("errorCount", new DG::JSValue (errorCount));
            jsResult->AddItem ("status", new DG::JSValue (errorCount == 0 ? "ok" : "partial"));

            DBprnt ("SetClassification: returning result");
            return jsResult;
        } catch (const std::exception &e) {
            DBprnt (GS::UniString ("SetClassification: std::exception: ") + e.what ());
            GS::Ref<DG::JSObject> errorObj = new DG::JSObject ();
            errorObj->AddItem ("success", new DG::JSValue (false));
            errorObj->AddItem ("message", new DG::JSValue (e.what ()));
            return errorObj;
        } catch (...) {
            DBprnt ("SetClassification: unknown exception caught");
            GS::Ref<DG::JSObject> errorObj = new DG::JSObject ();
            errorObj->AddItem ("success", new DG::JSValue (false));
            errorObj->AddItem ("message", new DG::JSValue ("Unknown error"));
            return errorObj;
        }
    }));

    browser.RegisterAsynchJSObject (jsACAPI);
}

void BrowserPalette::Command_Helth () { browser.ExecuteJS ("Command_Helth ()"); }

// -----------------------------------------------------------------------------
// Принудительное обновление информации о выделении из C++.
// Вызывается из JS (RefreshSelectionInfoUI) или по таймеру.
// -----------------------------------------------------------------------------
GSErrCode BrowserPalette::ManualGetSelection () {
    DBprnt ("BrowserPalette::ManualGetSelection ()");
    GS::Array<API_Guid> selectedElements = GetSelectedElements2 (false, true);
    UpdateSelectionInfoInUI (selectedElements);
    return NoError;
}

// -----------------------------------------------------------------------------
// Статический обработчик изменения выделения.
// Вызывается из глобального обработчика SelectionChangeHandlerProc.
// -----------------------------------------------------------------------------
GSErrCode __ACENV_CALL BrowserPalette::SelectionChangeHandler (const API_Neig * /*selElemNeig*/) {
    DBprnt ("BrowserPalette::SelectionChangeHandler ()");
    if (!HasInstance () || !GetInstance ().IsVisible ())
        return NoError;

    SyncSettings syncSettings;
    LoadSyncSettingsFromPreferences (syncSettings, true);
    if (!syncSettings.GetCatchSelectionChanges ())
        return NoError;

    // Получаем текущее выделение и обновляем UI
    GetInstance ().ManualGetSelection ();
    return NoError;
}

// -----------------------------------------------------------------------------
// Проверка, может ли тип элемента иметь свойства.
// Возвращает true для всех типов, кроме служебных.
bool ElementCanHaveProperty (const API_ElemTypeID &eltype) { return eltype != API_ZombieElemID; }

// -----------------------------------------------------------------------------
// Фильтрация массива GUID по типам элементов с ограничением количества.
// -----------------------------------------------------------------------------
GS::Array<API_Guid> FilterElementsByType (const GS::Array<API_Guid> &elements, USize maxSelectionCount) {
    GS::Array<API_Guid> result;
    result.SetCapacity (elements.GetSize ());
    for (const API_Guid &guid : elements) {
        if (maxSelectionCount > 0 && result.GetSize () >= maxSelectionCount)
            break;
        API_Element element;
        BNZeroMemory (&element, sizeof (API_Element));
        element.header.guid = guid;
        if (ACAPI_Element_Get (&element) != NoError)
            continue;
        if (!ElementCanHaveProperty (element.header.typeID))
            continue;
        result.Push (guid);
    }
    return result;
}

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