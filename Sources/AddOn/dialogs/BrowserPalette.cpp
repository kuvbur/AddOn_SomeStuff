// *****************************************************************************
// Source code for the BrowserPalette class
// *****************************************************************************

// ---------------------------------- Includes ---------------------------------

#include <ctime>

#include "ACAPinc.h"

#if defined(WINDOWS)
    // ACAPinc.h уже подключает windows.h через Win32Interface.hpp, а тот тянет
    // shellapi.h; включаем явно, чтобы ShellExecuteW не зависел от транзитивности.
    #include <shellapi.h>
#elif defined(macintosh)
    #include "Process.hpp"
#endif

#include "dialogs/BrowserPalette.hpp"

#include "CommonFunction.hpp"
#include "dialogs/CommandHelpers.hpp"
#include "dialogs/SyncSettings.hpp"
#include "Propertycache.hpp"
#include "spec/Spec.hpp"
#include "Sync.hpp"

static const GS::Guid paletteGuid ("{FEE27B6B-3873-5844-88B6-F0083AA4CD49}");

// Минимальная ширина окна палитры в свёрнутом виде (пиксели DG).
static const short kCollapsedPaletteMinWidth = 14;

static DG::Dialog::FixPoint GetHorizontalResizeFixPoint (const DG::NativeRect &paletteRect) {
    const DG::NativeRect screenRect = DG::VisibleBoundingRectOfScreens ();
    const DG::NativeUnit paletteCenter = paletteRect.GetLeft () + paletteRect.GetWidth () / 2;
    const DG::NativeUnit screenCenter = screenRect.GetLeft () + screenRect.GetWidth () / 2;
    return paletteCenter > screenCenter ? DG::Dialog::TopRight : DG::Dialog::TopLeft;
}

GS::Ref<BrowserPalette> BrowserPalette::instance;
bool BrowserPalette::suppressSelectionRefresh = false;

// -----------------------------------------------------------------------------
// Show or Hide Browser Palette
// -----------------------------------------------------------------------------
void ShowOrHideBrowserPalette () {
    const bool wasVisible = BrowserPalette::HasInstance () && BrowserPalette::GetInstance ().IsVisible ();
    if (wasVisible) {
        BrowserPalette::GetInstance ().Hide ();
    } else {
        if (!BrowserPalette::HasInstance ())
            BrowserPalette::CreateInstance ();
        BrowserPalette::GetInstance ().Show ();
    }
    SyncSettings syncSettings;
    LoadSyncSettingsFromPreferences (syncSettings, true);
    syncSettings.SetShowPalette (!wasVisible);
    WriteSyncSettingsToPreferences (syncSettings);
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

// -----------------------------------------------------------------------------
// Открывает сайт в браузере по умолчанию. Отдельного ACAPI_* «открыть URL» в SDK
// нет (в APIdefs_Interface.h AC22–29 только файловые диалоги), поэтому платформа
// решается здесь: ShellExecuteW на Windows, `open` через GS::Process на macOS.
// Windows-путь проверен по заголовку Windows SDK 10.0.26100 (shellapi.h:96),
// macOS-путь не проверен вживую (нет mac-машины) — см. карточку BrowserPalette.md.
// -----------------------------------------------------------------------------
static bool OpenWebsiteInDefaultBrowser (const GS::UniString &address) {
    if (address.IsEmpty ())
        return false;

#if defined(WINDOWS)
    // UniString хранит UTF-16, а ShellExecuteW ждёт LPCWSTR. Тип результата
    // ToUStr — вложенный класс внутри приватной секции UniString, поэтому
    // называть его здесь нельзя: тип выводится через auto, а указатель берётся
    // у Get (). Сам UStr освобождает буфер в деструкторе.
    const auto wideAddress = address.ToUStr ();
    const HINSTANCE result = ShellExecuteW (nullptr, L"open", wideAddress.Get (), nullptr, nullptr, SW_SHOWNORMAL);
    // ShellExecuteW возвращает HINSTANCE только при успехе; при ошибке — код
    // (ERROR_FILE_NOT_FOUND и т.п.), поэтому > 32, а не «не nullptr».
    return (result != nullptr) && (reinterpret_cast<INT_PTR> (result) > 32);
#elif defined(macintosh)
    // GS::Process::Create в mac-сборке DevKit запускает утилиту `open`, которая
    // и передаёт URL системе (LaunchServices). Процесс не ждём: он живёт дольше
    // вызова, а ждать нечего — синхронного признака успеха у GS::Process нет.
    try {
        GS::Array<GS::UniString> argv;
        argv.Push (address);
        const GS::Process browser = GS::Process::Create (GS::UniString ("open"), argv);
        return browser.IsValid ();
    } catch (...) {
        return false;
    }
#else
    return false;
#endif
}

// -----------------------------------------------------------------------------

// JSON-сериализация результата проверки правила спецификации (#244).
//
// Ответ — строка, а не DG::JSObject: вложенные объекты и массивы CEF теряет при
// передаче в JS (BrowserPalette.cpp, комментарий к ParsePropertyDescription).
// Перечисления отдаются строками, потому что в JS они читаются как подписи
// состояний, а числовой код невозможно соотнести с надписью без второй таблицы.
// -----------------------------------------------------------------------------
static const char *RuleFlagStatusName (Spec::RuleFlagStatus status) {
    switch (status) {
    case Spec::RuleFlagStatus::Unknown:
        return "Unknown";
    case Spec::RuleFlagStatus::NotPresent:
        return "NotPresent";
    case Spec::RuleFlagStatus::NotAvailable:
        return "NotAvailable";
    case Spec::RuleFlagStatus::NotEvaluated:
        return "NotEvaluated";
    case Spec::RuleFlagStatus::HasValue:
        return "HasValue";
    }
    return "Unknown";
}

static const char *RuleFlagOriginName (Spec::RuleFlagOrigin origin) {
    switch (origin) {
    case Spec::RuleFlagOrigin::NotChecked:
        return "NotChecked";
    case Spec::RuleFlagOrigin::ElementValue:
        return "ElementValue";
    case Spec::RuleFlagOrigin::FavoriteValue:
        return "FavoriteValue";
    case Spec::RuleFlagOrigin::DefaultElemValue:
        return "DefaultElemValue";
    case Spec::RuleFlagOrigin::DefaultDefinition:
        return "DefaultDefinition";
    }
    return "NotChecked";
}

static const char *ParseErrorName (Spec::ParseError error) {
    switch (error) {
    case Spec::ParseError::None:
        return "None";
    case Spec::ParseError::NoGroupMarker:
        return "NoGroupMarker";
    case Spec::ParseError::GroupNotSplit:
        return "GroupNotSplit";
    case Spec::ParseError::OutputPartCount:
        return "OutputPartCount";
    case Spec::ParseError::NoSummary:
        return "NoSummary";
    case Spec::ParseError::NoGroupsAccepted:
        return "NoGroupsAccepted";
    case Spec::ParseError::EmptyOutputSchema:
        return "EmptyOutputSchema";
    case Spec::ParseError::EmptySumSchema:
        return "EmptySumSchema";
    }
    return "None";
}

static GS::UniString RuleFlagCheckToJson (const Spec::RuleFlagCheck &flag) {
    return GS::UniString ("{\"checked\":") + (flag.checked ? "true" : "false") + GS::UniString (",\"value\":") +
           (flag.value ? "true" : "false") + GS::UniString (",\"status\":\"") + RuleFlagStatusName (flag.status) +
           GS::UniString ("\",\"origin\":\"") + RuleFlagOriginName (flag.origin) +
           GS::UniString ("\",\"sourceName\":\"") + EscapeJsonString (flag.sourceName).ToCStr ().Get () +
           GS::UniString ("\"}");
}

static GS::UniString StringArrayToJson (const GS::Array<GS::UniString> &items) {
    GS::UniString json = GS::UniString ("[");
    for (UIndex i = 0; i < items.GetSize (); ++i) {
        if (i > 0)
            json += GS::UniString (",");
        json += GS::UniString ("\"") + EscapeJsonString (items[i]).ToCStr ().Get () + GS::UniString ("\"");
    }
    json += GS::UniString ("]");
    return json;
}

// Общая часть результата: описание правила, назначение и отсутствующие имена.
// Не включает elementFlag — тот относится к конкретному элементу.
static GS::UniString SpecRuleCommonToJson (const Spec::RuleCheckResult &result) {
    if (!result.definitionFound) {
        // Определение не найдено: разбор не выполнялся, поэтому и parseError
        // здесь не имеет смысла — его значение по умолчанию скрыло бы факт отказа.
        return GS::UniString ("{\"definitionFound\":false,\"ruleParsed\":false,\"parseError\":\"\",") +
               GS::UniString ("\"parseErrorText\":\"определение свойства не найдено в проекте\",") +
               GS::UniString ("\"propertyName\":\"\",\"favoriteFound\":false,\"fromDefaultElem\":false,") +
               GS::UniString ("\"destinationNamePropFound\":false,\"destinationGuidPropFound\":false,") +
               GS::UniString ("\"destinationFlag\":null,\"missingWrite\":[],\"unresolvedInProject\":[]}");
    }
    GS::UniString parseErrorText = EMPTYSTRING;
    if (!result.ruleParsed) {
        parseErrorText = Spec::ParseErrorText (result.parseError);
        parseErrorText = parseErrorText.IsEmpty () ? GS::UniString ("ошибка разбора описания") : parseErrorText;
    }
    GS::UniString json =
        GS::UniString ("{\"definitionFound\":true,\"ruleParsed\":") + (result.ruleParsed ? "true" : "false") +
        GS::UniString (",\"parseError\":\"") + ParseErrorName (result.parseError) +
        GS::UniString ("\",\"parseErrorText\":\"") + EscapeJsonString (parseErrorText).ToCStr ().Get () +
        GS::UniString ("\",\"propertyName\":\"") + EscapeJsonString (result.propertyName).ToCStr ().Get () +
        GS::UniString ("\",\"favoriteFound\":") + (result.favoriteFound ? "true" : "false") +
        GS::UniString (",\"fromDefaultElem\":") + (result.fromDefaultElem ? "true" : "false") +
        GS::UniString (",\"destinationNamePropFound\":") + (result.destinationNamePropFound ? "true" : "false") +
        GS::UniString (",\"destinationGuidPropFound\":") + (result.destinationGuidPropFound ? "true" : "false") +
        GS::UniString (",\"destinationFlag\":") + RuleFlagCheckToJson (result.destinationFlag) +
        GS::UniString (",\"missingWrite\":") + StringArrayToJson (result.missingWrite) +
        GS::UniString (",\"unresolvedInProject\":") + StringArrayToJson (result.unresolvedInProject) +
        GS::UniString ("}");
    return json;
}

static GS::UniString SpecRuleElementToJson (const API_Guid &elemGuid, const Spec::RuleCheckResult &result) {
    return GS::UniString ("{\"guid\":\"") + APIGuidToString (elemGuid).ToCStr ().Get () +
           GS::UniString ("\",\"checkedElement\":") + (result.checkedElement ? "true" : "false") +
           GS::UniString (",\"elementFlag\":") + RuleFlagCheckToJson (result.elementFlag) +
           GS::UniString (",\"missingRead\":") + StringArrayToJson (result.missingRead) + GS::UniString ("}");
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

void BrowserPalette::Show (bool reloadContent) {
    DBprnt ("BrowserPalette::Show () called");
    DG::Palette::Show ();
    MenuItemCheckAC (Menu_Pallete, true);
    if (reloadContent) {
        // HTML перезагружается и теряет состояние свёртывания — возвращаем окно
        // к развёрнутой ширине, иначе состояние окна и HTML разойдутся.
        if (expandedClientWidth > 0) {
            // Тот же приём, что в SetPaletteCollapsed: снять с дока, поменять ширину,
            // вернуть в док.
            const bool wasDocked = IsDocked ();
            const DG::Dialog::FixPoint resizeFixPoint = GetHorizontalResizeFixPoint (GetFrameRect ());
            if (wasDocked)
                UnDock ();
            SetClientWidth (expandedClientWidth, resizeFixPoint);
            if (expandedMinClientWidth > 0)
                SetMinClientWidth (expandedMinClientWidth);
            if (wasDocked)
                Dock ();
            expandedClientWidth = 0;
            expandedMinClientWidth = 0;
            collapsedClientWidth = 0;
        }
        // Перезагрузка HTML сбрасывает состояние вкладки/фильтра, поэтому при
        // показе из APIPalMsg_HidePalette_End (reloadContent=false) контент не
        // перезагружаем — страница уже загружена.
        browser.ReloadIgnoreCache ();
        DBprnt ("BrowserPalette::Show () — after ReloadIgnoreCache");
    } else {
        // Перезагрузки нет, поэтому данные в панели обновляем вручную — раньше
        // это делал onLoadingStateChange после ReloadIgnoreCache.
        GS::Array<API_Guid> selectedElements;
        UpdateSelectionInfoInUI (selectedElements);
    }
    // Обновляем информацию о выделении после загрузки страницы
    // onLoadingStateChange вызовет RegisterACAPIJavaScriptObject и UpdateSelectionInfoInUI
}

void BrowserPalette::Hide () {
    DG::Palette::Hide ();
    // Записи настроек здесь нет — см. комментарий в Show ().
    MenuItemCheckAC (Menu_Pallete, false);
}

void BrowserPalette::UpdateSelectionInfoInUI (GS::Array<API_Guid> &selectedElements) {
    // Если массив пуст — получаем текущее выделение
    if (selectedElements.IsEmpty ()) {
        selectedElements = GetSelectedElements2 (false, true);
    }
    // Защита: при переключении в окно секции/3D ArchiCAD может прислать
    // виртуальные элементы с APINULLGuid. Не пускаем такие GUID'ы в JS-мост —
    // иначе ACAPI_Element_GetPropertyDefinitions/Values упадёт на них.
    if (!selectedElements.IsEmpty ()) {
        GS::Array<API_Guid> filtered;
        filtered.SetCapacity (selectedElements.GetSize ());
        for (const API_Guid &g : selectedElements) {
            if (g != APINULLGuid)
                filtered.Push (g);
        }
        selectedElements = std::move (filtered);
    }
    Int32 count = (Int32)selectedElements.GetSize ();
    DBprnt ("UpdateSelectionInfoInUI: count=" + GS::ValueToUniString (count));
    // Пушим данные в JS через ExecuteJS
    GS::UniString jsCall =
        GS::UniString ("if (typeof refreshSelectionInfoText === 'function') refreshSelectionInfoText(") +
        GS::ValueToUniString (count) + GS::UniString (");");
    browser.ExecuteJS (jsCall.ToCStr ().Get ());
    DBprnt ("UpdateSelectionInfoInUI: executed JS: " + jsCall);

    if (selectedElements.IsEmpty ())
        return;

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

    jsACAPI->AddItem (new DG::JSFunction ("GetFilterPresets", [] (GS::Ref<DG::JSBase>) -> GS::Ref<DG::JSBase> {
        try {
            SyncSettings syncSettings;
            LoadSyncSettingsFromPreferences (syncSettings, false);

            GS::UniString jsonStr = "{\"presets\":[";
            bool firstPreset = true;
            for (const FilterPreset &preset : syncSettings.GetFilterPresets ()) {
                if (!firstPreset)
                    jsonStr += ",";
                firstPreset = false;
                jsonStr += GS::UniString ("{\"label\":\"") + EscapeJsonString (preset.label).ToCStr ().Get () +
                           GS::UniString ("\",\"query\":\"") + EscapeJsonString (preset.query).ToCStr ().Get () +
                           GS::UniString ("\"}");
            }
            jsonStr += "]}";
            return new DG::JSValue (jsonStr);
        } catch (const std::exception &e) {
            DBprnt (GS::UniString ("GetFilterPresets: std::exception: ") + e.what ());
        } catch (...) {
            DBprnt ("GetFilterPresets: unknown exception");
        }
        return new DG::JSValue ("{\"presets\":[]}");
    }));

    // Регистрируем функцию для получения свойств выделенных элементов (возвращает JSON-строку для обхода ограничений
    // pull-паттерна)
    jsACAPI->AddItem (new DG::JSFunction ("GetPropertiesList", [this] (GS::Ref<DG::JSBase>) -> GS::Ref<DG::JSBase> {
        try {
#if defined(TESTING)
            const std::clock_t propertiesListStart = std::clock ();
#endif
            // Собираем данные свойств
            GS::Array<API_Guid> selectedElements = GetSelectedElements2 (false, false);
            // Ограничение количества отображаемых элементов задаётся из HTML (≤ select)
            selectedElements = FilterElementsByType (selectedElements, maxSelectionCount);

            // Формируем JSON строку вручную
            GS::UniString jsonStr = "{ \"elements\": [";

            bool firstElement = true;
            Int32 addedElementCount = 0;
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
                            API_PropertyGroup group = {};
                            const bool groupOk = ParamHelpers::GetGroupFromCache (prop.definition.groupGuid, group);
                            DBprnt (GS::UniString ("GetPropertiesList: prop=") + prop.definition.name.ToCStr ().Get () +
                                    GS::UniString (", groupGuid=") +
                                    APIGuidToString (prop.definition.groupGuid).ToCStr ().Get () +
                                    GS::UniString (", groupOk=") + GS::ValueToUniString (groupOk) +
                                    (groupOk ? (GS::UniString (", groupName=") + group.name.ToCStr ().Get ())
                                             : GS::UniString (", groupName=<unavailable>")));
                            if (groupOk && !group.name.IsEmpty ()) {
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
                        jsonStr +=
                            GS::UniString ("\"valueType\": \"") + GS::UniString (typeStr) + GS::UniString ("\",");

                        jsonStr += GS::UniString ("\"propertyGuid\": \"") +
                                   APIGuidToString (prop.definition.guid).ToCStr ().Get () + GS::UniString ("\"");

                        // Признак наличия правила SomeStuff в описании — из кэша PROPERTYCACHE (#158)
                        const bool propertyHasRule = GetPropertyRuleFlag (prop.definition);
                        jsonStr += GS::UniString (", \"hasRule\": ") +
                                   (propertyHasRule ? GS::UniString ("true") : GS::UniString ("false"));

                        jsonStr += "}";
                    }

                    jsonStr += "]}";

                    ++addedElementCount;
                }
            }

            jsonStr +=
                GS::UniString ("], \"count\": ") + GS::ValueToUniString (addedElementCount) + GS::UniString (" }");

            DBprnt (GS::UniString ("GetPropertiesList: returning JSON: ") + jsonStr);
#if defined(TESTING)
            const double propertiesListSeconds =
                static_cast<double> (std::clock () - propertiesListStart) / CLOCKS_PER_SEC;
            DBprnt (propertiesListSeconds,
                    GS::UniString ("GetPropertiesList baseline seconds; selected=") +
                        GS::ValueToUniString (selectedElements.GetSize ()) + GS::UniString ("; returned=") +
                        GS::ValueToUniString (addedElementCount));
#endif

            return new DG::JSValue (jsonStr);
        } catch (const std::exception &e) {
            DBprnt (GS::UniString ("GetPropertiesList: std::exception: ") + e.what ());
            return new DG::JSValue (GS::UniString ("{\"status\":\"error\",\"message\":\"std::exception\"}"));
        } catch (...) {
            DBprnt ("GetPropertiesList: unknown exception");
            return new DG::JSValue (GS::UniString ("{\"status\":\"error\",\"message\":\"unknown exception\"}"));
        }
    }));

    // Регистрируем функцию для получения значения свойства для выделенных элементов
    jsACAPI->AddItem (new DG::JSFunction ("GetPropertyValue", [this] (GS::Ref<DG::JSBase> args) -> GS::Ref<DG::JSBase> {
        try {
            // Аргумент приходит как одиночная строка — GUID определения свойства.
            // ВАЖНО: DynamicCast<JSArray> на аргументе крашит мост (зонды 2026-08-24),
            // безопасен только каст к JSValue.
            GS::Ref<DG::JSValue> propertyIdVal = GS::DynamicCast<DG::JSValue> (args);
            if (propertyIdVal == nullptr) {
                GS::Ref<DG::JSObject> errorObj = new DG::JSObject ();
                errorObj->AddItem ("status", new DG::JSValue ("error"));
                errorObj->AddItem ("message", new DG::JSValue ("Expected propertyId as string argument"));
                return errorObj;
            }

            GS::UniString propertyId = propertyIdVal->GetString ();
            // propertyId из HTML — GUID определения свойства (prop.propertyGuid)

            // Читаем значения свойства по каждому выделенному элементу.
            // ВАЖНО: cache.property хранит только ОПРЕДЕЛЕНИЯ свойств (без значений по элементам),
            // поэтому читаем значения напрямую через ACAPI_Element_GetPropertyValue.
            API_Guid propertyGuid = APIGuidFromString (propertyId.ToCStr ().Get ());

            // Inline implementation (like GetPropertiesList)
            GS::Array<API_Guid> selectedElements = GetSelectedElements2 (false, false);
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

            bool isCommon =
                (selectedElements.IsEmpty ()) || (uniqueValuesCount == 1 && totalCount == selectedElements.GetSize ());

            jsonStr +=
                GS::UniString ("\"common\": ") + GS::UniString (isCommon ? "true" : "false") + GS::UniString (", ");
            jsonStr += GS::UniString ("\"propertyId\": \"") + EscapeJsonString (propertyId).ToCStr ().Get () +
                       GS::UniString ("\", ");
            jsonStr += GS::UniString ("\"status\": \"ok\" }");

            DBprnt (GS::UniString ("GetPropertyValue: returning JSON: ") + jsonStr);

            return new DG::JSValue (jsonStr);
        } catch (const std::exception &e) {
            DBprnt (GS::UniString ("GetPropertyValue: std::exception: ") + e.what ());
            return new DG::JSValue (GS::UniString ("{\"status\":\"error\",\"message\":\"std::exception\"}"));
        } catch (...) {
            DBprnt ("GetPropertyValue: unknown exception");
            return new DG::JSValue (GS::UniString ("{\"status\":\"error\",\"message\":\"unknown exception\"}"));
        }
    }));

    jsACAPI->AddItem (
        new DG::JSFunction ("ResetPropertyToDefault", [this] (GS::Ref<DG::JSBase> args) -> GS::Ref<DG::JSBase> {
            try {
                GS::Ref<DG::JSValue> propertyIdVal = GS::DynamicCast<DG::JSValue> (args);
                if (propertyIdVal == nullptr) {
                    GS::Ref<DG::JSObject> errorObj = new DG::JSObject ();
                    errorObj->AddItem ("success", new DG::JSValue (false));
                    errorObj->AddItem ("message", new DG::JSValue ("Expected propertyId as string argument"));
                    return errorObj;
                }

                const GS::UniString propertyId = propertyIdVal->GetString ();
                const API_Guid propertyGuid = APIGuidFromString (propertyId.ToCStr ().Get ());
                if (propertyGuid == APINULLGuid) {
                    GS::Ref<DG::JSObject> errorObj = new DG::JSObject ();
                    errorObj->AddItem ("success", new DG::JSValue (false));
                    errorObj->AddItem ("message", new DG::JSValue ("Invalid property GUID"));
                    return errorObj;
                }

                GS::Array<API_Guid> selectedElements = GetSelectedElements2 (false, true);
                selectedElements = FilterElementsByType (selectedElements, maxSelectionCount);
                if (selectedElements.IsEmpty ()) {
                    GS::Ref<DG::JSObject> errorObj = new DG::JSObject ();
                    errorObj->AddItem ("success", new DG::JSValue (false));
                    errorObj->AddItem ("message", new DG::JSValue ("No elements selected"));
                    return errorObj;
                }

                Int32 successCount = 0;
                Int32 alreadyDefaultCount = 0;
                Int32 errorCount = 0;
                GS::Array<API_Guid> resetElementGuids;
                GS::Array<API_Property> resetProperties;

                for (const API_Guid &elemGuid : selectedElements) {
                    API_Property property = {};
                    GSErrCode err = ACAPI_Element_GetPropertyValue (elemGuid, propertyGuid, property);
                    if (err != NoError) {
                        ++errorCount;
                        msg_rep ("ResetPropertyToDefault", "ACAPI_Element_GetPropertyValue", err, elemGuid);
                        continue;
                    }

                    if (property.isDefault) {
                        ++alreadyDefaultCount;
                        continue;
                    }

                    property.isDefault = true;
                    property.value.variantStatus = API_VariantStatusNormal;
                    property.status = API_Property_HasValue;

                    resetElementGuids.Push (elemGuid);
                    resetProperties.Push (property);
                }

                if (!resetProperties.IsEmpty ()) {
                    const GSErrCode undoErr =
                        ACAPI_CallUndoableCommand ("Reset property to default", [&] () -> GSErrCode {
                            for (UIndex i = 0; i < resetProperties.GetSize (); ++i) {
                                GS::Array<API_Property> propertiesToReset;
                                propertiesToReset.Push (resetProperties[i]);
                                const GSErrCode err =
                                    ACAPI_Element_SetProperties (resetElementGuids[i], propertiesToReset);
                                if (err == NoError) {
                                    ++successCount;
                                } else {
                                    ++errorCount;
                                    msg_rep ("ResetPropertyToDefault",
                                             "ACAPI_Element_SetProperties",
                                             err,
                                             resetElementGuids[i]);
                                }
                            }
                            return NoError;
                        });

                    if (undoErr != NoError) {
                        ++errorCount;
                        DBprnt ("ResetPropertyToDefault: ACAPI_CallUndoableCommand error " +
                                GS::ValueToUniString (undoErr));
                    }
                }

                GS::Ref<DG::JSObject> jsResult = new DG::JSObject ();
                jsResult->AddItem ("success", new DG::JSValue (errorCount == 0));
                jsResult->AddItem ("successCount", new DG::JSValue (successCount));
                jsResult->AddItem ("alreadyDefaultCount", new DG::JSValue (alreadyDefaultCount));
                jsResult->AddItem ("errorCount", new DG::JSValue (errorCount));
                jsResult->AddItem ("status", new DG::JSValue (errorCount == 0 ? "ok" : "partial"));
                return jsResult;
            } catch (const std::exception &e) {
                DBprnt (GS::UniString ("ResetPropertyToDefault: std::exception: ") + e.what ());
                GS::Ref<DG::JSObject> errorObj = new DG::JSObject ();
                errorObj->AddItem ("success", new DG::JSValue (false));
                errorObj->AddItem ("message", new DG::JSValue (e.what ()));
                return errorObj;
            } catch (...) {
                DBprnt ("ResetPropertyToDefault: unknown exception");
                GS::Ref<DG::JSObject> errorObj = new DG::JSObject ();
                errorObj->AddItem ("success", new DG::JSValue (false));
                errorObj->AddItem ("message", new DG::JSValue ("Unknown error"));
                return errorObj;
            }
        }));

    // Подсветка и приближение элементов из HTML БЕЗ смены выделения
    // (паттерн Spec.cpp: APIIo_HighlightElementsID).
    // Контракт: вход — JSON-массив GUID'ов в виде строки '["{...}","{...}"]';
    // выход — true. Элементы подсвечиваются цветом (HashTable GUID->API_RGBAColor),
    // камера зумится на них (APIDo_ZoomToElementsID, par1: const GS::Array<API_Guid>*).
    jsACAPI->AddItem (new DG::JSFunction ("HighlightElements", [] (GS::Ref<DG::JSBase> args) -> GS::Ref<DG::JSBase> {
        try {
            GS::Ref<DG::JSValue> payload = GS::DynamicCast<DG::JSValue> (args);
            if (payload == nullptr) {
                return GS::Ref<DG::JSBase> (new DG::JSValue (false));
            }
            const GS::UniString jsonGuids = payload->GetString ();

            // Разбираем GUID'ы без JSON-парсера: содержимое между кавычками.
            GS::Array<API_Guid> guids;
            USize searchFrom = 0;
            while (true) {
                const USize q1 = jsonGuids.FindFirst ('"', searchFrom);
                if (q1 == MaxUSize)
                    break;
                const USize q2 = jsonGuids.FindFirst ('"', q1 + 1);
                if (q2 == MaxUSize)
                    break;
                const GS::UniString guidStr = jsonGuids.GetSubstring (q1 + 1, q2 - q1 - 1);
                const API_Guid guid = APIGuidFromString (guidStr.ToCStr (0, MaxUSize, GChCode));
                if (guid != APINULLGuid)
                    guids.Push (guid);
                searchFrom = q2 + 1;
            }

            DBprnt (GS::UniString ("HighlightElements: parsed ") + GS::ValueToUniString ((Int32)guids.GetSize ()) +
                    " guids");
            if (guids.IsEmpty ()) {
                return GS::Ref<DG::JSBase> (new DG::JSValue (false));
            }

            // Подсветка и зум могут транслироваться как смена выделения — на время
            // операции подавляем обновление палитры, чтобы выделение пользователя
            // не сбрасывалось через цепочку SelectionChangeHandler.
            struct SuppressSelectionRefreshGuard {
                bool &flag;
                ~SuppressSelectionRefreshGuard () { flag = false; }
            };
            suppressSelectionRefresh = true;
            SuppressSelectionRefreshGuard suppressGuard{suppressSelectionRefresh};
            (void)suppressGuard;

            // Подсветка цветом, выделение не трогаем. Версионные обёртки как в Spec.cpp.
            GS::HashTable<API_Guid, API_RGBAColor> hlElems;
            const API_RGBAColor hlColor = {1.0, 0.65, 0.0, 1.0};
            for (const API_Guid &guid : guids)
                hlElems.Put (guid, hlColor);
            // С AC26 функции подсветки возвращают void, поэтому код ошибки от них
            // не получить и hlErr остаётся NoError.
            GSErrCode hlErr = NoError;
#ifdef ServerMainVers_2700
            ACAPI_UserInput_ClearElementHighlight ();
            ACAPI_UserInput_SetElementHighlight (hlElems);
#else
    #ifdef ServerMainVers_2600
            ACAPI_Interface_ClearElementHighlight ();
            ACAPI_Interface_SetElementHighlight (hlElems);
    #else
            // Вызов без par1 снимает предыдущую подсветку
            ACAPI_Interface (APIIo_HighlightElementsID);
            hlErr = ACAPI_Interface (APIIo_HighlightElementsID, &hlElems);
    #endif
#endif
            if (hlErr != NoError) {
                DBprnt (GS::UniString ("HighlightElements: highlight error ") + GS::ValueToUniString (hlErr));
            }

// Приближаем камеру к элементам без смены выделения.
#ifdef ServerMainVers_2700
            const GSErrCode zoomErr =
                ACAPI_View_ZoomToElements (reinterpret_cast<const GS::Array<API_Guid> *> (&guids));
#else
            const GSErrCode zoomErr = ACAPI_Automate (APIDo_ZoomToElementsID, &guids);
#endif
            if (zoomErr != NoError) {
                DBprnt (GS::UniString ("HighlightElements: zoom error ") + GS::ValueToUniString (zoomErr));
            }
            return GS::Ref<DG::JSBase> (new DG::JSValue (hlErr == NoError && zoomErr == NoError));
        } catch (const std::exception &e) {
            DBprnt (GS::UniString ("HighlightElements: std::exception: ") + e.what ());
            suppressSelectionRefresh = false;
        } catch (...) {
            DBprnt ("HighlightElements: unknown exception");
            suppressSelectionRefresh = false;
        }
        return GS::Ref<DG::JSBase> (new DG::JSValue (false));
    }));

    // Регистрируем функцию для получения количества выделенных элементов
    jsACAPI->AddItem (new DG::JSFunction ("GetSelectionInfo", [this] (GS::Ref<DG::JSBase>) {
        try {
            DBprnt ("GetSelectionInfo: function called from JS");
            GS::Array<API_Guid> selectedElements = GetSelectedElements2 (false, false);
            // Ограничение количества отображаемых элементов задаётся из HTML (≤ select)
            selectedElements = FilterElementsByType (selectedElements, maxSelectionCount);
            Int32 count = (Int32)selectedElements.GetSize ();
            DBprnt ("GetSelectionInfo: GetSelectedElements2 returned " + GS::ValueToUniString (count) + " elements");
            GS::Ref<DG::JSObject> result = new DG::JSObject ();
            result->AddItem ("count", new DG::JSValue (count));
            DBprnt ("GetSelectionInfo: returning count=" + GS::ValueToUniString (count));
            return result;
        } catch (const std::exception &e) {
            DBprnt (GS::UniString ("GetSelectionInfo: std::exception: ") + e.what ());
        } catch (...) {
            DBprnt ("GetSelectionInfo: unknown exception");
        }
        GS::Ref<DG::JSObject> errorObj = new DG::JSObject ();
        errorObj->AddItem ("count", new DG::JSValue ((Int32)0));
        return errorObj;
    }));

    // Задание ограничения количества отображаемых элементов из HTML (≤ select).
    // ВАЖНО: функции БЕЗ аргументов — передача аргументов через RegisterAsynchJSObject
    // роняет CEF-мост (краш до входа в лямбду). Набор опций селекта фиксирован,
    // поэтому регистрируем по одной функции на каждое значение.
    auto addLimitSetter = [this, &jsACAPI] (UInt32 limit) {
        const GS::UniString name = GS::UniString ("SetLimit") + GS::ValueToUniString ((Int32)limit);
        jsACAPI->AddItem (
            new DG::JSFunction (name.ToCStr ().Get (), [this, limit] (GS::Ref<DG::JSBase>) -> GS::Ref<DG::JSBase> {
                try {
                    maxSelectionCount = limit;
                    DBprnt ("SetLimit: max selection count set to " + GS::ValueToUniString (limit));
                } catch (...) {
                    DBprnt ("SetLimit: exception for limit " + GS::ValueToUniString (limit));
                }
                return GS::Ref<DG::JSBase> (new DG::JSValue (true));
            }));
    };
    addLimitSetter (10);
    addLimitSetter (50);
    addLimitSetter (100);
    addLimitSetter (200);
    addLimitSetter (1000);

    // Включение/выключение автообновления при смене выделения.
    // ВАЖНО: функции БЕЗ аргументов — передача аргументов в JSFunction через
    // RegisterAsynchJSObject роняет CEF-мост (краш до входа в лямбду),
    // поэтому регистрируем по одной функции на каждое значение.
    auto addCatchSetter = [&jsACAPI] (bool enable) {
        const GS::UniString name =
            GS::UniString (enable ? "EnableCatchSelectionChanges" : "DisableCatchSelectionChanges");
        jsACAPI->AddItem (
            new DG::JSFunction (name.ToCStr ().Get (), [enable] (GS::Ref<DG::JSBase>) -> GS::Ref<DG::JSBase> {
                try {
                    SyncSettings syncSettings;
                    LoadSyncSettingsFromPreferences (syncSettings, true);
                    syncSettings.SetCatchSelectionChanges (enable);
                    WriteSyncSettingsToPreferences (syncSettings);
                    DBprnt (GS::UniString (enable ? "EnableCatchSelectionChanges: ok"
                                                  : "DisableCatchSelectionChanges: ok"));
                } catch (...) {
                    DBprnt (GS::UniString (enable ? "EnableCatchSelectionChanges: exception"
                                                  : "DisableCatchSelectionChanges: exception"));
                }
                return GS::Ref<DG::JSBase> (new DG::JSValue (true));
            }));
    };
    addCatchSetter (true);
    addCatchSetter (false);

    // Обновление количества выделенных элементов в UI (вызывается из JS).
    // Возвращаем DG::JSValue (не nullptr) — nullptr из JSFunction роняет CEF-мост.
    jsACAPI->AddItem (new DG::JSFunction ("RefreshSelectionInfoUI", [this] (GS::Ref<DG::JSBase>) {
        try {
            DBprnt ("RefreshSelectionInfoUI: called from JS");
            GS::Array<API_Guid> selectedElements;
            UpdateSelectionInfoInUI (selectedElements);
            return GS::Ref<DG::JSBase> (new DG::JSValue (true));
        } catch (const std::exception &e) {
            DBprnt (GS::UniString ("RefreshSelectionInfoUI: std::exception: ") + e.what ());
        } catch (...) {
            DBprnt ("RefreshSelectionInfoUI: unknown exception");
        }
        return GS::Ref<DG::JSBase> (new DG::JSValue (false));
    }));

    // Открытие сайта автора в браузере по умолчанию (вызывается из HTML).
    // Аргумент приходит одиночным JSValue: DynamicCast<JSArray> роняет CEF-мост.
    // Возвращаем DG::JSValue (не nullptr) — nullptr из JSFunction роняет мост.
    jsACAPI->AddItem (new DG::JSFunction ("OpenWebsite", [] (GS::Ref<DG::JSBase> args) {
        try {
            GS::Ref<DG::JSValue> addressValue = GS::DynamicCast<DG::JSValue> (args);
            if (addressValue == nullptr)
                return GS::Ref<DG::JSBase> (new DG::JSValue (false));

            const GS::UniString address = addressValue->GetString ();
            const bool opened = OpenWebsiteInDefaultBrowser (address);
            DBprnt (GS::UniString ("OpenWebsite: ") + address + (opened ? " — opened" : " — FAILED"));
            return GS::Ref<DG::JSBase> (new DG::JSValue (opened));
        } catch (const std::exception &e) {
            DBprnt (GS::UniString ("OpenWebsite: std::exception: ") + e.what ());
        } catch (...) {
            DBprnt ("OpenWebsite: unknown exception");
        }
        return GS::Ref<DG::JSBase> (new DG::JSValue (false));
    }));

    // Регистрируем функцию для парсинга описания свойства.
    // Контракт для будущей вкладки «Синхронизация» (ТЗ интерфейс.md §10):
    // вход  — строка описания правила;
    // выход — JSON-строка {ok, hasCommands, remainingText, commands:[...]}.
    // ВАЖНО: аргумент приходит как одиночный JSValue (DynamicCast<JSArray>
    // крашит мост), ответ отдаём JSON-строкой — вложенные DG::JSObject/JSArray
    // CEF теряет при передаче в JS.
    jsACAPI->AddItem (new DG::JSFunction ("ParsePropertyDescription", [] (GS::Ref<DG::JSBase> args) {
        try {
            // args = description string
            GS::Ref<DG::JSValue> descValue = GS::DynamicCast<DG::JSValue> (args);
            if (descValue == nullptr) {
                GS::Ref<DG::JSObject> errorObj = new DG::JSObject ();
                errorObj->AddItem ("ok", new DG::JSValue (false));
                errorObj->AddItem ("error", new DG::JSValue ("Expected description as string argument"));
                return GS::Ref<DG::JSBase> (errorObj);
            }

            GS::UniString description = descValue->GetString ();

            // Парсим описание
            GS::Array<ParsedPropertyCommand> commands;
            GS::UniString remainingText;
            bool hasCommands = ParsePropertyDescription (description, commands, remainingText);

            // Формируем JSON-ответ (вложенные объекты не сериализуются через CEF)
            GS::UniString jsonStr =
                GS::UniString ("{ \"ok\": true, \"hasCommands\": ") + (hasCommands ? "true" : "false") + ", ";
            jsonStr += GS::UniString ("\"remainingText\": \"") + EscapeJsonString (remainingText).ToCStr ().Get () +
                       GS::UniString ("\", ");
            jsonStr += GS::UniString ("\"commands\": [");
            bool firstCmd = true;
            for (const auto &cmd : commands) {
                if (!firstCmd)
                    jsonStr += ",";
                firstCmd = false;
                jsonStr += GS::UniString ("{\"commandType\":\"") + EscapeJsonString (cmd.commandType).ToCStr ().Get () +
                           GS::UniString ("\"");
                jsonStr += GS::UniString (",\"fullCommand\":\"") + EscapeJsonString (cmd.fullCommand).ToCStr ().Get () +
                           GS::UniString ("\"");
                jsonStr += GS::UniString (",\"parameters\":\"") + EscapeJsonString (cmd.parameters).ToCStr ().Get () +
                           GS::UniString ("\"");
                jsonStr += GS::UniString (",\"isValid\":") + (cmd.isValid ? "true" : "false");
                jsonStr += GS::UniString (",\"errorMessage\":\"") +
                           EscapeJsonString (cmd.errorMessage).ToCStr ().Get () + GS::UniString ("\"}");
            }
            jsonStr += "]}";

            return GS::Ref<DG::JSBase> (new DG::JSValue (jsonStr));
        } catch (const std::exception &e) {
            DBprnt (GS::UniString ("ParsePropertyDescription: std::exception: ") + e.what ());
            return GS::Ref<DG::JSBase> (
                new DG::JSValue (GS::UniString ("{\"ok\":false,\"error\":\"std::exception\"}")));
        } catch (...) {
            DBprnt ("ParsePropertyDescription: unknown exception");
            return GS::Ref<DG::JSBase> (
                new DG::JSValue (GS::UniString ("{\"ok\":false,\"error\":\"unknown exception\"}")));
        }
    }));

    // Регистрируем функцию для парсинга описания свойства с привязкой к элементу.
    // Контракт для будущей вкладки «Синхронизация» (ТЗ интерфейс.md §10):
    // вход  — строка описания правила, строка GUID элемента;
    // выход — JSON-строка {ok, hasSyncRules, hasOtherCommands, remainingText,
    //         syncRules:[...], otherCommands:[...]}.
    // ВАЖНО: оба аргумента приходят как одиночные JSValue (DynamicCast<JSArray>
    // крашит мост), ответ отдаём JSON-строкой — вложенные DG::JSObject/JSArray
    // CEF теряет при передаче в JS.
    // Два аргумента передаются из JS одним JSON-массивом: ParsePropertyForElement(
    // JSON.stringify([description, elemGuid])) и разбираются здесь вручную.
    jsACAPI->AddItem (new DG::JSFunction ("ParsePropertyForElement", [] (GS::Ref<DG::JSBase> args) {
        try {
            GS::Ref<DG::JSValue> descValue = GS::DynamicCast<DG::JSValue> (args);
            if (descValue == nullptr) {
                GS::Ref<DG::JSObject> errorObj = new DG::JSObject ();
                errorObj->AddItem ("ok", new DG::JSValue (false));
                errorObj->AddItem ("error", new DG::JSValue ("Expected string argument with JSON payload"));
                return GS::Ref<DG::JSBase> (errorObj);
            }
            // payload = JSON.stringify([description, elemGuid]) — разбираем без JSON-парсера:
            // строка имеет вид ["<desc>","<guid>"]; извлекаем содержимое между кавычками.
            const GS::UniString payload = descValue->GetString ();
            const USize q1 = payload.FindFirst ('"');
            const USize q2 = (q1 == MaxUSize) ? MaxUSize : payload.FindFirst ('"', q1 + 1);
            const USize q3 = (q2 == MaxUSize) ? MaxUSize : payload.FindFirst ('"', q2 + 1);
            const USize q4 = (q3 == MaxUSize) ? MaxUSize : payload.FindFirst ('"', q3 + 1);
            if (q1 == MaxUSize || q2 == MaxUSize || q3 == MaxUSize || q4 == MaxUSize) {
                GS::Ref<DG::JSObject> errorObj = new DG::JSObject ();
                errorObj->AddItem ("ok", new DG::JSValue (false));
                errorObj->AddItem ("error", new DG::JSValue ("Payload must be a JSON array [description, elemGuid]"));
                return GS::Ref<DG::JSBase> (errorObj);
            }
            GS::UniString description = payload.GetSubstring (q1 + 1, q2 - q1 - 1);
            GS::UniString elemGuidStr = payload.GetSubstring (q3 + 1, q4 - q3 - 1);

            API_Guid elemGuid = APIGuidFromString (elemGuidStr.ToCStr (0, MaxUSize, GChCode));

            if (elemGuid == APINULLGuid) {
                GS::Ref<DG::JSObject> errorObj = new DG::JSObject ();
                errorObj->AddItem ("ok", new DG::JSValue (false));
                errorObj->AddItem ("error", new DG::JSValue ("Invalid GUID format"));
                return GS::Ref<DG::JSBase> (errorObj);
            }

            // Парсим описание через существующую функцию
            ParsePropertyResult parseResult = ParsePropertyDescriptionToRules (description);

            // Формируем JSON-ответ (вложенные объекты не сериализуются через CEF)
            GS::UniString jsonStr =
                GS::UniString ("{ \"ok\": true, \"hasSyncRules\": ") + (parseResult.hasSyncRules ? "true" : "false") +
                GS::UniString (", \"hasOtherCommands\": ") + (parseResult.hasOtherCommands ? "true" : "false") + ", ";
            jsonStr += GS::UniString ("\"remainingText\": \"") +
                       EscapeJsonString (parseResult.remainingText).ToCStr ().Get () + GS::UniString ("\", ");
            jsonStr += "\"syncRules\": [";
            bool firstRule = true;
            for (const auto &rule : parseResult.syncRules) {
                if (!firstRule)
                    jsonStr += ",";
                firstRule = false;
                jsonStr += GS::UniString ("{\"commandType\":\"") +
                           EscapeJsonString (rule.commandType).ToCStr ().Get () + GS::UniString ("\"");
                jsonStr += GS::UniString (",\"fullCommand\":\"") +
                           EscapeJsonString (rule.fullCommand).ToCStr ().Get () + GS::UniString ("\"");
                jsonStr += GS::UniString (",\"parameters\":\"") + EscapeJsonString (rule.parameters).ToCStr ().Get () +
                           GS::UniString ("\"");
                jsonStr += GS::UniString (",\"sourceType\":\"") + EscapeJsonString (rule.sourceType).ToCStr ().Get () +
                           GS::UniString ("\"");
                jsonStr += GS::UniString (",\"sourceName\":\"") + EscapeJsonString (rule.sourceName).ToCStr ().Get () +
                           GS::UniString ("\"");
                jsonStr += GS::UniString (",\"targetType\":\"") + EscapeJsonString (rule.targetType).ToCStr ().Get () +
                           GS::UniString ("\"");
                jsonStr += GS::UniString (",\"targetName\":\"") + EscapeJsonString (rule.targetName).ToCStr ().Get () +
                           GS::UniString ("\"");
                jsonStr += GS::UniString (",\"formatString\":\"") +
                           EscapeJsonString (rule.formatString).ToCStr ().Get () + GS::UniString ("\"");
                jsonStr += GS::UniString (",\"isValid\":") + (rule.isValid ? "true" : "false");
                jsonStr += GS::UniString (",\"errorMessage\":\"") +
                           EscapeJsonString (rule.errorMessage).ToCStr ().Get () + GS::UniString ("\"");
                jsonStr += GS::UniString (",\"hasSub\":") + (rule.hasSub ? "true" : "false");
                jsonStr += GS::UniString (",\"hasGUID\":") + (rule.hasGUID ? "true" : "false");
                jsonStr += GS::UniString (",\"guidSourceProperty\":\"") +
                           EscapeJsonString (rule.guidSourceProperty).ToCStr ().Get () + GS::UniString ("\"");
                jsonStr += ",\"ignoreVals\":[";
                bool firstIv = true;
                for (const auto &iv : rule.ignoreVals) {
                    if (!firstIv)
                        jsonStr += ",";
                    firstIv = false;
                    jsonStr += GS::UniString ("\"") + EscapeJsonString (iv).ToCStr ().Get () + GS::UniString ("\"");
                }
                jsonStr += "}";
            }
            jsonStr += "], ";
            jsonStr += "\"otherCommands\": [";
            bool firstOc = true;
            for (const auto &cmd : parseResult.otherCommands) {
                if (!firstOc)
                    jsonStr += ",";
                firstOc = false;
                jsonStr += GS::UniString ("{\"commandType\":\"") + EscapeJsonString (cmd.commandType).ToCStr ().Get () +
                           GS::UniString ("\"");
                jsonStr += GS::UniString (",\"fullCommand\":\"") + EscapeJsonString (cmd.fullCommand).ToCStr ().Get () +
                           GS::UniString ("\"");
                jsonStr += GS::UniString (",\"parameters\":\"") + EscapeJsonString (cmd.parameters).ToCStr ().Get () +
                           GS::UniString ("\"");
                jsonStr += GS::UniString (",\"isValid\":") + (cmd.isValid ? "true" : "false");
                jsonStr += GS::UniString (",\"errorMessage\":\"") +
                           EscapeJsonString (cmd.errorMessage).ToCStr ().Get () + GS::UniString ("\"}");
            }
            jsonStr += "]}";

            return GS::Ref<DG::JSBase> (new DG::JSValue (jsonStr));
        } catch (const std::exception &e) {
            DBprnt (GS::UniString ("ParsePropertyForElement: std::exception: ") + e.what ());
            return GS::Ref<DG::JSBase> (
                new DG::JSValue (GS::UniString ("{\"ok\":false,\"error\":\"std::exception\"}")));
        } catch (...) {
            DBprnt ("ParsePropertyForElement: unknown exception");
            return GS::Ref<DG::JSBase> (
                new DG::JSValue (GS::UniString ("{\"ok\":false,\"error\":\"unknown exception\"}")));
        }
    }));

    // Регистрируем функцию для получения классификации выделенных элементов (inline implementation)
    jsACAPI->AddItem (new DG::JSFunction ("GetClassification", [this] (GS::Ref<DG::JSBase>) -> GS::Ref<DG::JSBase> {
        try {
            DBprnt ("GetClassification: [1] function called from JS");

            GS::Array<API_Guid> selectedElements = GetSelectedElements2 (false, false);
            // Ограничение количества отображаемых элементов задаётся из HTML (≤ select)
            selectedElements = FilterElementsByType (selectedElements, maxSelectionCount);
            DBprnt (GS::UniString::Printf ("GetClassification: [2] selectedElements count = %d",
                                           selectedElements.GetSize ()));

            if (selectedElements.IsEmpty ()) {
                DBprnt ("GetClassification: [3] no selected elements, returning empty");
                return new DG::JSValue (GS::UniString (
                    "{\"common\":true,\"commonPath\":[],\"differing\":[],\"options\":[],\"status\":\"ok\"}"));
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
                return new DG::JSValue (
                    GS::UniString ("{\"common\":true,\"commonPath\":[],\"differing\":[],\"options\":[],"
                                   "\"status\":\"error\",\"debug_error\":\"ReadSystemDict failed\"}"));
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
#ifdef ServerMainVers_2800
                            // AC28: CurrentPair::value — ссылка (Value&), а не указатель как в ≤27.
                            for (const auto &classPair : sysPair.value) {
                                if (classPair.value.item.guid == pair.second) {
                                    ClassificationFunc::GetFullName (classPair.value.item, sysPair.value, displayName);
#else
                            for (const auto &classPair : *sysPair.value) {
                                if (classPair.value->item.guid == pair.second) {
                                    ClassificationFunc::GetFullName (
                                        classPair.value->item, *sysPair.value, displayName);
#endif
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

            // Собираем список всех доступных классификаций (опции для выбора).
            GS::Array<GS::UniString> &optionsCache = cache.classificationOptions;
            if (optionsCache.IsEmpty ()) {
                auto &systemdict = cache.systemdict;
                for (const auto &sysPair : systemdict) {
#ifdef ServerMainVers_2800
                    const ClassificationFunc::ClassificationDict *classDict = &sysPair.value;
#else
                    const ClassificationFunc::ClassificationDict *classDict = sysPair.value;
#endif
                    if (classDict == nullptr)
                        continue;
                    for (const auto &classPair : *classDict) {
#ifdef ServerMainVers_2800
                        const ClassificationFunc::ClassificationValues *cv = &classPair.value;
#else
                        const ClassificationFunc::ClassificationValues *cv = classPair.value;
#endif
                        if (cv == nullptr)
                            continue;
                        GS::UniString fullName;
                        ClassificationFunc::GetFullName (cv->item, *classDict, fullName);
                        optionsCache.Push (fullName);
                    }
                }
            }
            const GS::Array<GS::UniString> &options = optionsCache;

            DBprnt (
                GS::UniString::Printf ("GetClassification: [result] isCommon=%d, commonPathSegments=%d, differing=%d",
                                       isCommon,
                                       commonPathSegments.GetSize (),
                                       differing.GetSize ()));

            // Отладка: выводим первые элементы differing
            for (const auto &d : differing) {
                GS::UniString cls;
                Int32 cnt = 0, tot = 0;
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
                Int32 count = 0, total = 0;
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
            return new DG::JSValue (
                GS::UniString ("{\"common\":true,\"commonPath\":[],\"differing\":[],\"options\":[],"
                               "\"status\":\"error\",\"debug_error\":\"std::exception\",\"debug_exception\":\"") +
                EscapeJsonString (e.what ()).ToCStr ().Get () + "\"}");
        } catch (...) {
            DBprnt ("GetClassification: unknown exception caught");
            return new DG::JSValue (GS::UniString ("{\"common\":true,\"commonPath\":[],\"differing\":[],\"options\":[],"
                                                   "\"status\":\"error\",\"debug_error\":\"unknown exception\"}"));
        }
    }));

    // Регистрируем функцию для назначения классификации выделенным элементам (inline implementation)
    jsACAPI->AddItem (
        new DG::JSFunction ("SetClassification", [this] (GS::Ref<DG::JSBase> args) -> GS::Ref<DG::JSBase> {
            try {
                // Аргумент приходит как одиночная строка — полное имя классификации.
                // ВАЖНО: DynamicCast<JSArray> на аргументе крашит мост (зонды 2026-08-24),
                // безопасен только каст к JSValue.
                GS::Ref<DG::JSValue> classificationValueVal = GS::DynamicCast<DG::JSValue> (args);
                if (classificationValueVal == nullptr) {
                    GS::Ref<DG::JSObject> errorObj = new DG::JSObject ();
                    errorObj->AddItem ("success", new DG::JSValue (false));
                    errorObj->AddItem ("message", new DG::JSValue ("Expected classificationValue as string argument"));
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
#ifdef ServerMainVers_2800
                    const ClassificationFunc::ClassificationDict *classDict = &sysPair.value;
#else
                    const ClassificationFunc::ClassificationDict *classDict = sysPair.value;
#endif
                    for (const auto &classPair : *classDict) {
#ifdef ServerMainVers_2800
                        const ClassificationFunc::ClassificationValues *cv = &classPair.value;
#else
                        const ClassificationFunc::ClassificationValues *cv = classPair.value;
#endif
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

    // Свёртывание/развёртывание ОКНА палитры: окно ужимается до ширины колонки
    // вкладок, HTML при этом скрывает рабочую область (toggleNavCollapse).
    // Аргумент — строка "<0|1>|<промилле>": флаг свёртывания и доля ширины колонки
    // вкладок от ширины окна. Доля измеряется в HTML по фактическим пикселям, поэтому
    // не зависит от DPI и масштаба CEF: целевая ширина = текущая ширина * доля.
    jsACAPI->AddItem (
        new DG::JSFunction ("SetPaletteCollapsed", [this] (GS::Ref<DG::JSBase> args) -> GS::Ref<DG::JSBase> {
            try {
                GS::Ref<DG::JSValue> payload = GS::DynamicCast<DG::JSValue> (args);
                if (payload == nullptr)
                    return GS::Ref<DG::JSBase> (new DG::JSValue (false));

                const GS::UniString arg = payload->GetString ();
                const USize sep = arg.FindFirst ('|');
                if (sep == MaxUSize)
                    return GS::Ref<DG::JSBase> (new DG::JSValue (false));

                const bool collapsed = (arg.GetLength () > 0) && (arg[0] == '1');

                // Пошаговая диагностика: где именно теряется ширина при работе с доком.
                auto logState = [this] (const char *step) {
                    DBprnt (GS::UniString ("SetPaletteCollapsed[") + GS::UniString (step) +
                            "]: width=" + GS::ValueToUniString ((Int32)GetClientWidth ()) +
                            " px, min=" + GS::ValueToUniString ((Int32)GetMinClientWidth ()) +
                            " px, docked=" + (IsDocked () ? "yes" : "no"));
                };

                if (collapsed) {
                    // Промилле разбираем вручную: у GS::UniString нет ToInt32/ToNumber.
                    const GS::UniString ratioStr = arg.GetSubstring (sep + 1, MaxUSize);
                    Int32 perMille = 0;
                    for (UIndex i = 0; i < ratioStr.GetLength (); ++i) {
                        const GS::UniChar c = ratioStr[i];
                        // Только сравнения на равенство: у GS::UniChar нет operator< с char
                        // (арифметика и '<' неоднозначны из-за нескольких операторов приведения).
                        Int32 digit = -1;
                        if (c == '0')
                            digit = 0;
                        else if (c == '1')
                            digit = 1;
                        else if (c == '2')
                            digit = 2;
                        else if (c == '3')
                            digit = 3;
                        else if (c == '4')
                            digit = 4;
                        else if (c == '5')
                            digit = 5;
                        else if (c == '6')
                            digit = 6;
                        else if (c == '7')
                            digit = 7;
                        else if (c == '8')
                            digit = 8;
                        else if (c == '9')
                            digit = 9;
                        if (digit < 0)
                            break;
                        perMille = perMille * 10 + digit;
                        if (perMille > 1000)
                            break;
                    }
                    if (perMille <= 0 || perMille >= 1000) {
                        DBprnt ("SetPaletteCollapsed: invalid ratio, ignored");
                        return GS::Ref<DG::JSBase> (new DG::JSValue (false));
                    }

                    const short currentWidth = GetClientWidth ();
                    // Запоминаем ширину и минимум только при первом сворачивании, иначе
                    // повторный клик запомнил бы уже суженное окно.
                    if (expandedClientWidth == 0) {
                        expandedClientWidth = currentWidth;
                        expandedMinClientWidth = GetMinClientWidth ();
                    }

                    logState ("collapse/1 before");
                    DBprnt (GS::UniString ("SetPaletteCollapsed: collapse requested, target=") +
                            GS::ValueToUniString ((Int32)(short)(((Int32)currentWidth * perMille) / 1000)) +
                            " px, ratio=" + GS::ValueToUniString (perMille) + " / 1000");

                    // Шириной пристыкованной палитры распоряжается док-менеджер ArchiCAD
                    // (SetClientWidth там игнорируется), поэтому перед изменением размера
                    // палитру отстыковываем и сразу пристыковываем обратно — так она
                    // остаётся в доке, но уже нужной ширины.
                    short targetWidth = (short)(((Int32)currentWidth * perMille) / 1000);
                    if (targetWidth < kCollapsedPaletteMinWidth)
                        targetWidth = kCollapsedPaletteMinWidth;
                    collapsedClientWidth = targetWidth;

                    // Растущий диалог нельзя ужать ниже минимальной ширины (по умолчанию
                    // она равна исходной). После ручного изменения размера дока Archicad
                    // хранит широкий dock-слот, поэтому минимум ослабляем ДО UnDock.
                    const bool wasDocked = IsDocked ();
                    const DG::Dialog::FixPoint resizeFixPoint = GetHorizontalResizeFixPoint (GetFrameRect ());
                    SetMinClientWidth (targetWidth);
                    if (wasDocked) {
                        UnDock ();
                        logState ("collapse/2 after UnDock");
                    }

                    SetClientWidth (targetWidth, resizeFixPoint);
                    logState ("collapse/3 after SetClientWidth");
                    if (wasDocked) {
                        Dock ();
                        SetMinClientWidth (targetWidth);
                        SetClientWidth (targetWidth, resizeFixPoint);
                        logState ("collapse/4 after Dock");
                    }
                    // PanelResized придёт автоматически и подвинет браузерный контрол.
                } else if (expandedClientWidth > 0) {
                    // Состояние сбрасываем до вызовов: они могут дёрнуть панель повторно.
                    const short restoreWidth = expandedClientWidth;
                    const short restoreMinWidth = expandedMinClientWidth;
                    expandedClientWidth = 0;
                    expandedMinClientWidth = 0;
                    collapsedClientWidth = 0;

                    logState ("expand/1 before");
                    DBprnt (GS::UniString ("SetPaletteCollapsed: expand requested, restore=") +
                            GS::ValueToUniString ((Int32)restoreWidth) + " px");

                    // Пристыкованную палитру тоже освобождаем от дока на время изменения
                    // размера и возвращаем в док после — см. ветку сворачивания.
                    const bool wasDocked = IsDocked ();
                    const DG::Dialog::FixPoint resizeFixPoint = GetHorizontalResizeFixPoint (GetFrameRect ());
                    if (wasDocked) {
                        UnDock ();
                        logState ("expand/2 after UnDock");
                    }

                    SetClientWidth (restoreWidth, resizeFixPoint);
                    // Минимум возвращаем после ширины: иначе SetClientWidth упрётся в старый.
                    SetMinClientWidth (restoreMinWidth > 0 ? restoreMinWidth : restoreWidth);
                    logState ("expand/3 after SetClientWidth");
                    if (wasDocked) {
                        Dock ();
                        logState ("expand/4 after Dock");
                    }
                }
                return GS::Ref<DG::JSBase> (new DG::JSValue (true));
            } catch (const std::exception &e) {
                DBprnt (GS::UniString ("SetPaletteCollapsed: std::exception: ") + e.what ());
            } catch (...) {
                DBprnt ("SetPaletteCollapsed: unknown exception");
            }
            return GS::Ref<DG::JSBase> (new DG::JSValue (false));
        }));

    // =====================================================================
    // Вкладка «Спецификация»: список свойств-правил и их проверка (#244).
    //
    // Признак hasSpecRule — ТОЧНЫЙ: в описании свойства есть команда
    // Spec_rule. Агрегат hasRule из GetPropertyRuleFlag для этого не годится:
    // он суммирует Sync, Spec_rule, Renum(_flag) и Sum (Propertycache.cpp:23-30),
    // то есть в список валидатора попадали бы свойства с чужими правилами.
    // Признак НЕ означает включённый флаг — включённость зависит от элемента
    // и меняется после записи, поэтому её показывает только проверка по GUID.
    // =====================================================================

    // Разбор описания один раз на определение: и признак, и имя берутся оттуда.
    auto collectSpecRuleProperties = [] (const GS::Array<API_PropertyDefinition> &definitions, GS::UniString &jsonOut) {
        bool firstProperty = true;
        UInt32 withSpecRule = 0;
        for (const API_PropertyDefinition &definition : definitions) {
            // Признак — тот же, что и у запуска правила: подстрока "pec_rule" в
            // описании в нижнем регистре. Разбор команды как "Spec_rule{" не годится:
            // он требует, чтобы команда начиналась с префикса БУКВАЛЬНО с начала
            // описания, а правило может быть записано после другого текста и в любом
            // регистре — тогда признак молча оставался false для всех свойств.
            // Постфикс перед "pec_rule" (km, kzh, v2, v3) частью имени не считается:
            // все они начинаются с "pec_rule".
            const bool hasSpecRule =
                !definition.description.IsEmpty () && definition.description.ToLowerCase ().Contains ("pec_rule");
            GS::UniString name = EMPTYSTRING;
            GetPropertyFullName (definition, name);
            if (name.IsEmpty ())
                name = definition.name;
            if (hasSpecRule)
                ++withSpecRule;
            if (!firstProperty)
                jsonOut += GS::UniString (",");
            firstProperty = false;
            jsonOut += GS::UniString ("{\"id\":\"") + APIGuidToString (definition.guid).ToCStr ().Get () +
                       GS::UniString ("\",\"name\":\"") + EscapeJsonString (name).ToCStr ().Get () +
                       GS::UniString ("\",\"hasSpecRule\":") +
                       (hasSpecRule ? GS::UniString ("true") : GS::UniString ("false")) + GS::UniString ("}");
        }
    };

    jsACAPI->AddItem (new DG::JSFunction (
        "GetSpecRuleProperties", [collectSpecRuleProperties] (GS::Ref<DG::JSBase>) -> GS::Ref<DG::JSBase> {
            try {
                GS::Array<API_Guid> selectedElements = GetSelectedElements2 (false, false);
                GS::UniString jsonStr =
                    GS::UniString ("{\"source\":\"") +
                    (selectedElements.IsEmpty () ? GS::UniString ("project") : GS::UniString ("selection")) +
                    GS::UniString ("\",\"selectionCount\":") +
                    GS::ValueToUniString ((Int32)selectedElements.GetSize ()) + GS::UniString (",\"properties\":[");

                // Множество уже собранных свойств нужно обеим веткам: одно и то же
                // свойство встречается у нескольких элементов и в нескольких группах.
                GS::HashTable<API_Guid, bool> seen = {};

                if (!selectedElements.IsEmpty ()) {
                    // Свойства выделенных элементов. У каждого элемента свой набор,
                    // поэтому свойство собирается один раз и повторно не добавляется.
                    GS::Array<API_PropertyDefinition> merged = {};
                    for (const API_Guid &elemGuid : selectedElements) {
                        GS::Array<API_PropertyDefinition> definitions = {};
                        if (ACAPI_Element_GetPropertyDefinitions (
                                elemGuid, API_PropertyDefinitionFilter_UserDefined, definitions) != NoError) {
                            continue;
                        }
                        for (const API_PropertyDefinition &definition : definitions) {
                            if (seen.GetPtr (definition.guid) != nullptr)
                                continue;
                            seen.Put (definition.guid, true);
                            merged.Push (definition);
                        }
                    }
                    collectSpecRuleProperties (merged, jsonStr);
                } else {
                    // Все свойства проекта: те же группы, что читает PROPERTYCACHE,
                    // иначе список отличался бы от остального интерфейса.
                    auto &cache = PROPERTYCACHE ();
                    if (!cache.isGroupPropertyRead_full)
                        cache.ReadGroupProperty ();
                    GS::Array<API_PropertyDefinition> definitions = {};
                    for (const auto &cIt : cache.propertygroups) {
#ifdef ServerMainVers_2800
                        const API_PropertyGroup &group = cIt.value;
#else
                    const API_PropertyGroup &group = *cIt.value;
#endif
                        GS::Array<API_PropertyDefinition> groupDefinitions = {};
                        if (ACAPI_Property_GetPropertyDefinitions (group.guid, groupDefinitions) != NoError)
                            continue;
                        for (const API_PropertyDefinition &definition : groupDefinitions) {
                            if (seen.GetPtr (definition.guid) != nullptr)
                                continue;
                            seen.Put (definition.guid, true);
                            definitions.Push (definition);
                        }
                    }
                    collectSpecRuleProperties (definitions, jsonStr);
                }

                jsonStr += GS::UniString ("]}");
                return new DG::JSValue (jsonStr);
            } catch (const std::exception &e) {
                DBprnt (GS::UniString ("GetSpecRuleProperties: std::exception: ") + e.what ());
                return new DG::JSValue (GS::UniString ("{\"source\":\"none\",\"selectionCount\":0,\"properties\":[]}"));
            } catch (...) {
                DBprnt ("GetSpecRuleProperties: unknown exception");
                return new DG::JSValue (GS::UniString ("{\"source\":\"none\",\"selectionCount\":0,\"properties\":[]}"));
            }
        }));

    // Проверка правила спецификации по GUID свойства-правила. Аргументы приходят
    // одним значением — JSON-массивом [propertyGuid, limit] (паттерн
    // ParsePropertyForElement): DynamicCast<JSArray> на аргументах роняет мост.
    jsACAPI->AddItem (
        new DG::JSFunction ("CheckSpecRuleByPropertyGuid", [this] (GS::Ref<DG::JSBase> args) -> GS::Ref<DG::JSBase> {
            // Закрывающая скобка — ТОЛЬКО в конце: ранние выходы дописывают
            // common и elements, и скобка посреди строки ломала бы JSON
            // (парсер JS отверг бы весь ответ, а не только ранний выход).
            const GS::UniString emptyAnswer =
                GS::UniString ("{\"ok\":false,\"checked\":0,\"totalSelected\":0,\"truncated\":false") +
                GS::UniString (",\"common\":null,\"elements\":[]}");
            try {
                GS::Ref<DG::JSValue> value = GS::DynamicCast<DG::JSValue> (args);
                if (value == nullptr) {
                    return new DG::JSValue (emptyAnswer);
                }
                const GS::UniString payload = value->GetString ();
                const USize q1 = payload.FindFirst ('"');
                const USize q2 = (q1 == MaxUSize) ? MaxUSize : payload.FindFirst ('"', q1 + 1);
                const USize q3 = (q2 == MaxUSize) ? MaxUSize : payload.FindFirst ('"', q2 + 1);
                const USize q4 = (q3 == MaxUSize) ? MaxUSize : payload.FindFirst ('"', q3 + 1);
                if (q1 == MaxUSize || q2 == MaxUSize || q3 == MaxUSize || q4 == MaxUSize) {
                    return new DG::JSValue (emptyAnswer);
                }
                const GS::UniString propertyGuidStr = payload.GetSubstring (q1 + 1, q2 - q1 - 1);
                const API_Guid propertyGuid = APIGuidFromString (propertyGuidStr.ToCStr (0, MaxUSize, GChCode));
                if (propertyGuid == APINULLGuid) {
                    return new DG::JSValue (emptyAnswer);
                }

                GS::Array<API_Guid> selectedElements = GetSelectedElements2 (false, false);
                const Int32 totalSelected = (Int32)selectedElements.GetSize ();
                // Лимит ограничивает ЧИТАНИЕ, а не только вывод: на 200 выделенных
                // элементах проверка читает свойства каждого.
                const USize maxCheck = maxSelectionCount > 0 ? (USize)maxSelectionCount : 10;
                USize checkedCount = selectedElements.GetSize ();
                if (checkedCount > maxCheck)
                    checkedCount = maxCheck;

                if (checkedCount == 0) {
                    // Без элемента проверяется только проект — теми же функциями,
                    // что и при запуске, с APINULLGuid.
                    Spec::RuleCheckResult result = {};
                    const bool definitionFound = Spec::CheckRuleByPropertyGuid (propertyGuid, APINULLGuid, result);
                    GS::UniString jsonStr = GS::UniString ("{\"ok\":") + (definitionFound ? "true" : "false") +
                                            GS::UniString (",\"checked\":0,\"totalSelected\":0,\"truncated\":false") +
                                            GS::UniString (",\"common\":") + SpecRuleCommonToJson (result) +
                                            GS::UniString (",\"elements\":[]}");
                    return new DG::JSValue (jsonStr);
                }

                // Общий блок считается ОДИН раз: разбор правила и проверка назначения
                // не зависят от элемента, а на 100 элементах это 100 лишних
                // разборов описания и 100 чтений избранного.
                Spec::RuleCheckResult commonResult = {};
                const bool definitionFound = Spec::CheckRuleByPropertyGuid (propertyGuid, APINULLGuid, commonResult);
                // Правило уже разобрано — элементы проверяются по нему напрямую.
                // Повторный CheckRuleByPropertyGuid на каждый элемент заново читал бы
                // определение, разбирал описание и затирал бы общий результат.
                GS::UniString elementsJson = GS::UniString (",\"elements\":[");
                for (USize i = 0; i < checkedCount; ++i) {
                    Spec::RuleCheckResult elementResult = commonResult;
                    // Признак ставим здесь, а не наследуем из общего блока: там
                    // он не выставляется (CheckRuleByPropertyGuid ставит его только
                    // в своей ветке с элементом), и без этой строки JS считал бы
                    // каждый элемент непроверенным.
                    elementResult.checkedElement = true;
                    Spec::CheckRuleElementByRule (commonResult.rule, propertyGuid, selectedElements[i], elementResult);
                    if (i > 0)
                        elementsJson += GS::UniString (",");
                    elementsJson += SpecRuleElementToJson (selectedElements[i], elementResult);
                }
                elementsJson += GS::UniString ("]");

                GS::UniString jsonStr = GS::UniString ("{\"ok\":") + (definitionFound ? "true" : "false") +
                                        GS::UniString (",\"checked\":") + GS::ValueToUniString ((Int32)checkedCount) +
                                        GS::UniString (",\"totalSelected\":") + GS::ValueToUniString (totalSelected) +
                                        GS::UniString (",\"truncated\":") +
                                        (checkedCount < selectedElements.GetSize () ? "true" : "false") +
                                        GS::UniString (",\"common\":") + SpecRuleCommonToJson (commonResult) +
                                        elementsJson + GS::UniString ("}");
                return new DG::JSValue (jsonStr);
            } catch (const std::exception &e) {
                DBprnt (GS::UniString ("CheckSpecRuleByPropertyGuid: std::exception: ") + e.what ());
                return new DG::JSValue (emptyAnswer);
            } catch (...) {
                DBprnt ("CheckSpecRuleByPropertyGuid: unknown exception");
                return new DG::JSValue (emptyAnswer);
            }
        }));
#if defined(ServerMainVers_2700) || !defined(ServerMainVers_2600)
    // AC26 удалил UnregisterJSObject из DGLib, AC27 вернул его (перегрузку по имени)
    // уже в JavascriptEngine — компилируем вызов только там, где метод есть.
    browser.UnregisterJSObject (GS::UniString ("ACAPI"));
#endif
    const bool registerOk = browser.RegisterAsynchJSObject (jsACAPI);
    if (!registerOk) {
        DBprnt ("RegisterACAPIJavaScriptObject: browser.RegisterAsynchJSObject failed for object 'ACAPI'");
    }
}

// -----------------------------------------------------------------------------
// Принудительное обновление информации о выделении из C++.
// Вызывается из JS (RefreshSelectionInfoUI) или по таймеру.
// -----------------------------------------------------------------------------
GSErrCode BrowserPalette::ManualGetSelection () {
    DBprnt ("BrowserPalette::ManualGetSelection ()");
    GS::Array<API_Guid> selectedElements = GetSelectedElements2 (false, false);
    UpdateSelectionInfoInUI (selectedElements);
    return NoError;
}

// -----------------------------------------------------------------------------
// Статический обработчик изменения выделения.
// Вызывается из глобального обработчика SelectionChangeHandlerProc.
// -----------------------------------------------------------------------------
#ifdef ServerMainVers_2800
GSErrCode BrowserPalette::SelectionChangeHandler (const API_Neig * /*selElemNeig*/) {
#else
GSErrCode __ACENV_CALL BrowserPalette::SelectionChangeHandler (const API_Neig * /*selElemNeig*/) {
#endif
    DBprnt ("BrowserPalette::SelectionChangeHandler ()");
    // Программная подсветка/зум транслируются как смена выделения — игнорируем,
    // чтобы не сбрасывать пользовательское выделение.
    if (suppressSelectionRefresh)
        return NoError;
    if (!HasInstance () || !GetInstance ().IsVisible ())
        return NoError;

    SyncSettings syncSettings;
    LoadSyncSettingsFromPreferences (syncSettings, false);
    if (!syncSettings.GetCatchSelectionChanges ())
        return NoError;

    // Получаем текущее выделение и обновляем UI.
    // ManualGetSelection сам фильтрует APINULLGuid через UpdateSelectionInfoInUI.
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
        API_Elem_Head head = {};
        head.guid = guid;
        if (ACAPI_Element_GetHeader (&head) != NoError)
            continue;
        // В AC26 поле API_Elem_Head::typeID заменено на type (API_ElemType);
        // GetElemTypeID инкапсулирует различие версий.
        if (!ElementCanHaveProperty (GetElemTypeID (head)))
            continue;
        result.Push (guid);
    }
    return result;
}

void BrowserPalette::PanelResized (const DG::PanelResizeEvent &ev) {
    BeginMoveResizeItems ();
    browser.Resize (ev.GetHorizontalChange (), ev.GetVerticalChange ());
    EndMoveResizeItems ();

    if (expandedClientWidth == 0 || collapsedClientWidth == 0)
        return;

    const short currentWidth = GetClientWidth ();
    if (currentWidth <= collapsedClientWidth) {
        if (currentWidth >= kCollapsedPaletteMinWidth)
            collapsedClientWidth = currentWidth;
        return;
    }

    DBprnt (GS::UniString ("PanelResized: collapsed palette was manually widened from ") +
            GS::ValueToUniString ((Int32)collapsedClientWidth) + " px to " +
            GS::ValueToUniString ((Int32)currentWidth) + " px; deferred until next collapse");
}

void BrowserPalette::PanelCloseRequested (const DG::PanelCloseRequestEvent &, bool *accepted) {
    Hide ();
    *accepted = true;
}

#ifdef ServerMainVers_2800
GSErrCode BrowserPalette::PaletteControlCallBack (Int32, API_PaletteMessageID messageID, GS::IntPtr param) {
#else
GSErrCode __ACENV_CALL BrowserPalette::PaletteControlCallBack (Int32,
                                                               API_PaletteMessageID messageID,
                                                               GS::IntPtr param) {
#endif
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
        if (HasInstance () && !GetInstance ().IsVisible ()) {
            GetInstance ().Show (false);
        }
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
