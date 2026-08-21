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
    // Проверка на null ДО вызова BMhGetSize — иначе UB при отсутствии ресурса
    if (data == nullptr) {
        DBprnt ("LoadHtmlFromResource: resource not found");
        return resourceData;
    }
    const GSSize handleSize = BMhGetSize (data);
    resourceData.Append (*data, handleSize);
    BMhKill (&data);
    return resourceData;
}

// -----------------------------------------------------------------------------
// Экранирование строки для безопасной вставки в JSON вручную собранных ответов.
// Без экранирования кавычки/обратные слэши/переводы строк в значениях ломают JSON.
// -----------------------------------------------------------------------------
static GS::UniString EscapeJsonString (const GS::UniString &s) {
    GS::UniString r;
    r.SetCapacity (s.GetLength ());
    for (UIndex i = 0; i < s.GetLength (); ++i) {
        const GS::UniChar c = s[i];
        if (c == '"') {
            r.Append ("\\\"");
        } else if (c == '\\') {
            r.Append ("\\\\");
        } else if (c == '\n') {
            r.Append ("\\n");
        } else if (c == '\r') {
            r.Append ("\\r");
        } else if (c == '\t') {
            r.Append ("\\t");
        } else {
            r.Append (c);
        }
    }
    return r;
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

    // Также перерисовываем классификацию при смене выделения
    GS::UniString jsCallClassif =
        "if (typeof renderClassificationBlock === 'function') renderClassificationBlock(document.getElementById('monitor-classification-block'));";
    browser.ExecuteJS (jsCallClassif.ToCStr ().Get ());
    DBprnt ("UpdateSelectionInfoInUI: executed JS for classification refresh");
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
    jsACAPI->AddItem (new DG::JSFunction ("GetPropertiesList", [this] (GS::Ref<DG::JSBase>) -> GS::Ref<DG::JSBase> {
        // Собираем данные свойств
        GS::Array<API_Guid> selectedElements = GetSelectedElements2 (false, true);
        // Ограничение количества отображаемых элементов задаётся из HTML (≤ select)
        selectedElements = FilterElementsByType (selectedElements, maxSelectionCount);

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
                        jsonStr += GS::UniString ("\"name\": \"") +
                                   EscapeJsonString (prop.definition.name).ToCStr ().Get () + GS::UniString ("\",");
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
                    jsonStr += GS::UniString ("\"group\": \"") + EscapeJsonString (groupName).ToCStr ().Get () +
                               GS::UniString ("\",");

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
                        jsonStr += GS::UniString ("\"value\": \"") + EscapeJsonString (valueStr).ToCStr ().Get () +
                                   GS::UniString ("\",");
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
    jsACAPI->AddItem (new DG::JSFunction ("GetPropertyValue", [this] (GS::Ref<DG::JSBase> args) -> GS::Ref<DG::JSBase> {
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
        // propertyId из HTML — GUID определения свойства (prop.propertyGuid)

        // Читаем значения свойства по каждому выделенному элементу.
        // ВАЖНО: cache.property хранит только ОПРЕДЕЛЕНИЯ свойств (без значений по элементам),
        // поэтому читаем значения напрямую через ACAPI_Element_GetPropertyValue.
        API_Guid propertyGuid = APIGuidFromString (propertyId.ToCStr ().Get ());

        // Inline implementation (like GetPropertiesList)
        GS::Array<API_Guid> selectedElements = GetSelectedElements2 (false, true);
        // Ограничение количества отображаемых элементов задаётся из HTML (≤ select)
        selectedElements = FilterElementsByType (selectedElements, maxSelectionCount);

        GS::UniString jsonStr = "{ \"values\": [";

        bool firstValue = true;
        GS::HashTable<GS::UniString, Int32> valueCounts;

        for (const API_Guid &elemGuid : selectedElements) {
            API_Property property;
            GSErrCode err = ACAPI_Element_GetPropertyValue (elemGuid, propertyGuid, property);
            if (err != NoError || property.status != API_Property_HasValue) {
                continue;
            }
            // Значение может быть одиночным или списочным — берём первый вариант
            const API_Variant *variant = nullptr;
            if (property.value.variantStatus == API_VariantStatusNormal &&
                property.value.singleVariant.variant.type != API_PropertyUndefinedValueType) {
                variant = &property.value.singleVariant.variant;
            } else if (!property.value.listVariant.variants.IsEmpty ()) {
                variant = &property.value.listVariant.variants[0];
            }
            if (variant == nullptr) {
                continue;
            }
            GS::UniString valueStr;
            switch (variant->type) {
            case API_PropertyIntegerValueType:
                valueStr = GS::UniString::Printf ("%d", variant->intValue);
                break;
            case API_PropertyRealValueType:
                valueStr = GS::UniString::Printf ("%.3f", variant->doubleValue);
                break;
            case API_PropertyStringValueType:
                valueStr = variant->uniStringValue;
                break;
            case API_PropertyBooleanValueType:
                valueStr = variant->boolValue ? "true" : "false";
                break;
            case API_PropertyGuidValueType:
                valueStr = APIGuidToString (variant->guidValue);
                break;
            default:
                continue; // значение отсутствует/не поддерживается
            }

            const Int32 *currentCountPtr = valueCounts.GetPtr (valueStr);
            Int32 currentCount = currentCountPtr ? *currentCountPtr : 0;
            valueCounts.Put (valueStr, currentCount + 1);
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
            jsonStr += GS::UniString ("{\"value\":\"") + EscapeJsonString (key).ToCStr ().Get () +
                       GS::UniString ("\",\"count\":") + GS::ValueToUniString (value) + GS::UniString ("}");
            totalCount += value;
            uniqueValuesCount++;
        }

        jsonStr += "], ";

        bool isCommon = (uniqueValuesCount == 1 && totalCount == selectedElements.GetSize ());

        jsonStr += GS::UniString ("\"common\": ") + GS::UniString (isCommon ? "true" : "false") + GS::UniString (", ");
        jsonStr += GS::UniString ("\"propertyName\": \"") + EscapeJsonString (propertyId).ToCStr ().Get () +
                   GS::UniString ("\", ");
        jsonStr += GS::UniString ("\"status\": \"ok\" }");

        DBprnt (GS::UniString ("GetPropertyValue: returning JSON: ") + jsonStr);

        return new DG::JSValue (jsonStr);
    }));

    // Регистрируем функцию для получения количества выделенных элементов
    jsACAPI->AddItem (new DG::JSFunction ("GetSelectionInfo", [this] (GS::Ref<DG::JSBase>) {
        DBprnt ("GetSelectionInfo: function called from JS");
        GS::Array<API_Guid> selectedElements = GetSelectedElements2 (false, true);
        // Ограничение количества отображаемых элементов задаётся из HTML (≤ select)
        selectedElements = FilterElementsByType (selectedElements, maxSelectionCount);
        Int32 count = (Int32)selectedElements.GetSize ();
        DBprnt ("GetSelectionInfo: GetSelectedElements2 returned " + GS::ValueToUniString (count) + " elements");
        GS::Ref<DG::JSObject> result = new DG::JSObject ();
        result->AddItem ("count", new DG::JSValue (count));
        DBprnt ("GetSelectionInfo: returning count=" + GS::ValueToUniString (count));
        return result;
    }));

    // Задание ограничения количества отображаемых элементов из HTML (≤ select).
    // ВАЖНО: возвращаем DG::JSValue (не nullptr) — nullptr из JSFunction роняет CEF-мост.
    jsACAPI->AddItem (
        new DG::JSFunction ("SetMaxSelectionCount", [this] (GS::Ref<DG::JSBase> args) -> GS::Ref<DG::JSBase> {
            UInt32 value = 0; // 0 = без ограничения
            if (args != nullptr) {
                GS::Ref<DG::JSArray> argsArray = GS::DynamicCast<DG::JSArray> (args);
                if (argsArray != nullptr && argsArray->GetItemArray ().GetSize () >= 1) {
                    GS::Ref<DG::JSValue> val = GS::DynamicCast<DG::JSValue> (argsArray->GetItemArray ()[0]);
                    if (val != nullptr && val->GetType () == DG::JSValue::UINTEGER) {
                        value = val->GetUInteger ();
                    } else if (val != nullptr) {
                        value = (UInt32)val->GetInteger ();
                    }
                }
            }
            maxSelectionCount = value;
            DBprnt ("SetMaxSelectionCount: limit set to " + GS::ValueToUniString (value));
            return GS::Ref<DG::JSBase> (new DG::JSValue (true));
        }));

    // Включение/выключение автообновления при смене выделения
    jsACAPI->AddItem (
        new DG::JSFunction ("SetCatchSelectionChanges", [] (GS::Ref<DG::JSBase> args) -> GS::Ref<DG::JSBase> {
            bool enable = false;
            if (args != nullptr) {
                GS::Ref<DG::JSArray> argsArray = GS::DynamicCast<DG::JSArray> (args);
                if (argsArray != nullptr && argsArray->GetItemArray ().GetSize () >= 1) {
                    GS::Ref<DG::JSValue> val = GS::DynamicCast<DG::JSValue> (argsArray->GetItemArray ()[0]);
                    if (val != nullptr) {
                        enable = val->GetBool ();
                    }
                }
            }
            SyncSettings syncSettings;
            LoadSyncSettingsFromPreferences (syncSettings, true);
            syncSettings.SetCatchSelectionChanges (enable);
            WriteSyncSettingsToPreferences (syncSettings);
            DBprnt ("SetCatchSelectionChanges: " + GS::UniString (enable ? "true" : "false"));
            return GS::Ref<DG::JSBase> (new DG::JSValue (true));
        }));

    // Отладочная функция — проверяет, что JS-мост работает
    jsACAPI->AddItem (new DG::JSFunction ("Ping", [] (GS::Ref<DG::JSBase>) {
        DBprnt ("Ping: JavaScript Bridge is working!");
        GS::Ref<DG::JSObject> result = new DG::JSObject ();
        result->AddItem ("ok", new DG::JSValue (true));
        result->AddItem ("message", new DG::JSValue ("Bridge is alive"));
        return result;
    }));

    // Обновление количества выделенных элементов в UI (вызывается из JS).
    // Возвращаем DG::JSValue (не nullptr) — nullptr из JSFunction роняет CEF-мост.
    jsACAPI->AddItem (new DG::JSFunction ("RefreshSelectionInfoUI", [this] (GS::Ref<DG::JSBase>) {
        DBprnt ("RefreshSelectionInfoUI: called from JS");
        // Обновляем UI через push (ExecuteJS)
        GS::Array<API_Guid> selectedElements;
        UpdateSelectionInfoInUI (selectedElements);
        return GS::Ref<DG::JSBase> (new DG::JSValue (true));
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
    jsACAPI->AddItem (new DG::JSFunction ("GetClassification", [this] (GS::Ref<DG::JSBase>) -> GS::Ref<DG::JSBase> {
        try {
            DBprnt ("GetClassification: [1] function called from JS");

            GS::Array<API_Guid> selectedElements = GetSelectedElements2 (false, true);
            // Ограничение количества отображаемых элементов задаётся из HTML (≤ select)
            selectedElements = FilterElementsByType (selectedElements, maxSelectionCount);
            DBprnt (GS::UniString::Printf ("GetClassification: [2] selectedElements count = %d",
                                           selectedElements.GetSize ()));

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
            DBprnt (
                GS::UniString::Printf ("GetClassification: [5] ReadSystemDict result = %d, isClassification_OK = %d",
                                       readResult,
                                       cache.isClassification_OK));

            if (!readResult) {
                DBprnt ("GetClassification: [5] failed to load classification system");
                GS::Ref<DG::JSObject> errorObj = new DG::JSObject ();
                errorObj->AddItem ("common", new DG::JSValue (true));
                errorObj->AddItem ("commonPath", new DG::JSArray ());
                errorObj->AddItem ("differing", new DG::JSArray ());
                errorObj->AddItem ("options", new DG::JSArray ());
                errorObj->AddItem ("status", new DG::JSValue ("error"));
                errorObj->AddItem ("debug_error", new DG::JSValue ("ReadSystemDict failed"));
                errorObj->AddItem ("diag_isClassificationRead", new DG::JSValue (cache.isClassificationRead));
                errorObj->AddItem ("diag_isClassification_OK", new DG::JSValue (cache.isClassification_OK));
                return errorObj;
            }
            DBprnt ("GetClassification: [6] ReadSystemDict succeeded");

            DBprnt (GS::UniString::Printf ("GetClassification: [7] systemdict size = %d", cache.systemdict.GetSize ()));
            DBprnt (GS::UniString::Printf ("GetClassification: [8] reversesystemdict size = %d",
                                           cache.reversesystemdict.GetSize ()));

            // Собираем классификацию для каждого выделенного элемента
            GS::HashTable<GS::Pair<API_Guid, API_Guid>, Int32> classificationCounts; // systemGuid+itemGuid -> count
            GS::HashTable<GS::Pair<API_Guid, API_Guid>, GS::UniString>
                classificationDisplayNames; // systemGuid+itemGuid -> display name

            for (const API_Guid &elemGuid : selectedElements) {
                GS::Array<GS::Pair<API_Guid, API_Guid>> systemItemPairs;
                GSErrCode err = ACAPI_Element_GetClassificationItems (elemGuid, systemItemPairs);
                DBprnt (GS::UniString::Printf (
                    "GetClassification: [9] element has %d classifications, err=%d", systemItemPairs.GetSize (), err));
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
                        for (const auto &sysPair : cache.systemdict) {
                            for (const auto &classPair : *sysPair.value) {
                                if (classPair.value->item.guid == pair.second) {
                                    ClassificationFunc::GetFullName (
                                        classPair.value->item, *sysPair.value, displayName);
                                    classificationDisplayNames.Put (key, displayName);
                                    found = true;
                                    break;
                                }
                            }
                            if (found)
                                break;
                        }
                        if (!found) {
                            classificationDisplayNames.Put (key, GS::UniString ("Unknown"));
                        }
                        DBprnt (GS::UniString::Printf ("GetClassification: [10] found class for key -> %s",
                                                       displayName.ToCStr ().Get ()));
                    }
                }
            }

            DBprnt (GS::UniString::Printf ("GetClassification: [11] classificationCounts size = %d",
                                           classificationCounts.GetSize ()));

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

                DBprnt (GS::UniString::Printf ("GetClassification: [12] iterating classificationCounts, size=%d",
                                               classificationCounts.GetSize ()));

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
                                                   APIGuidToString (key.second).ToCStr ().Get (),
                                                   count,
                                                   selectedElements.GetSize (),
                                                   displayName.ToCStr ().Get ()));

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
                    DBprnt (GS::UniString::Printf ("GetClassification: commonPathSegments count = %d",
                                                   commonPathSegments.GetSize ()));
                }
            }

            // Собираем список всех доступных классификаций (опции для выбора)
            GS::Array<GS::UniString> options;
            auto &systemdict = cache.systemdict;
            for (const auto &sysPair : systemdict) {
                const ClassificationFunc::ClassificationDict *classDict = sysPair.value;
                if (classDict == nullptr)
                    continue;
                for (const auto &classPair : *classDict) {
                    const ClassificationFunc::ClassificationValues *cv = classPair.value;
                    if (cv == nullptr)
                        continue;
                    GS::UniString fullName;
                    ClassificationFunc::GetFullName (cv->item, *classDict, fullName);
                    options.Push (fullName);
                }
            }

            DBprnt (
                GS::UniString::Printf ("GetClassification: [result] isCommon=%d, commonPathSegments=%d, differing=%d",
                                       isCommon,
                                       commonPathSegments.GetSize (),
                                       differing.GetSize ()));

            // Отладка: выводим первые элементы differing
            for (const auto &d : differing) {
                GS::UniString cls;
                Int32 cnt, tot;
                d.Get ("classification", cls);
                d.Get ("count", cnt);
                d.Get ("total", tot);
                DBprnt (GS::UniString::Printf (
                    "  differing item: classification=%s, count=%d, total=%d", cls.ToCStr ().Get (), cnt, tot));
            }

            // Формируем JS результат (возвращаем JSON-строку, как делает GetPropertiesList)
            GS::UniString jsonStr = "{";

            jsonStr += "\"common\": " + GS::UniString (isCommon ? "true" : "false") + ",";

            jsonStr += "\"commonPath\": [";
            for (UIndex i = 0; i < commonPathSegments.GetSize (); ++i) {
                if (i > 0)
                    jsonStr += ",";
                jsonStr += GS::UniString ("\"") + EscapeJsonString (commonPathSegments[i]).ToCStr ().Get () + "\"";
            }
            jsonStr += "],";

            jsonStr += "\"differing\": [";
            for (UIndex i = 0; i < differing.GetSize (); ++i) {
                if (i > 0)
                    jsonStr += ",";
                jsonStr += "{";
                GS::UniString classification;
                Int32 count, total;
                differing[i].Get ("classification", classification);
                differing[i].Get ("count", count);
                differing[i].Get ("total", total);
                jsonStr += GS::UniString ("\"classification\": \"") +
                           EscapeJsonString (classification).ToCStr ().Get () + GS::UniString ("\",");
                jsonStr += "\"count\": " + GS::ValueToUniString (count) + ",";
                jsonStr += "\"total\": " + GS::ValueToUniString (total);
                jsonStr += "}";
            }
            jsonStr += "],";

            jsonStr += "\"options\": [";
            for (UIndex i = 0; i < options.GetSize (); ++i) {
                if (i > 0)
                    jsonStr += ",";
                jsonStr += GS::UniString ("\"") + EscapeJsonString (options[i]).ToCStr ().Get () + "\"";
            }
            jsonStr += "],";

            jsonStr += "\"status\": \"ok\"";
            jsonStr += "}";

            DBprnt ("GetClassification: returning JSON: " + jsonStr);
            return new DG::JSValue (jsonStr);
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
            return errorObj;
        }
    }));

    // Регистрируем функцию для назначения классификации выделенным элементам (inline implementation)
    jsACAPI->AddItem (
        new DG::JSFunction ("SetClassification", [this] (GS::Ref<DG::JSBase> args) -> GS::Ref<DG::JSBase> {
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
                // Ограничение количества отображаемых элементов задаётся из HTML (≤ select)
                selectedElements = FilterElementsByType (selectedElements, maxSelectionCount);

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

                // Назначаем классификацию каждому выделенному элементу.
                // Изменение модели — оборачиваем в undo-команду (Ctrl+Z отменяет разом).
                Int32 successCount = 0;
                Int32 errorCount = 0;

                const GSErrCode undoErr = ACAPI_CallUndoableCommand ("Set Classification", [&] () -> GSErrCode {
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
                    return NoError;
                });
                if (undoErr != NoError) {
                    DBprnt ("SetClassification: ACAPI_CallUndoableCommand error " + GS::ValueToUniString (undoErr));
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