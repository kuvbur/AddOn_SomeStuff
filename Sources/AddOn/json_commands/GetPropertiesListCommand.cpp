#include "json_commands/GetPropertiesListCommand.hpp"

#include "CommonFunction.hpp"
#include "Propertycache.hpp"

GetPropertiesListCommand::GetPropertiesListCommand () : ReadOnlyCommand () {}

GS::String GetPropertiesListCommand::GetName () const { return "GetPropertiesList"; }

GS::Optional<GS::UniString> GetPropertiesListCommand::GetInputParametersSchema () const { return GS::NoValue; }

GS::Optional<GS::UniString> GetPropertiesListCommand::GetResponseSchema () const { return GS::NoValue; }

GS::ObjectState GetPropertiesListCommand::Execute (const GS::ObjectState & /*parameters*/,
                                                   GS::ProcessControl & /*processControl*/) const {
    auto &cache = PROPERTYCACHE ();

    // Получаем GUID-ы выделенных элементов
    GS::Array<API_Guid> selectedElements = GetSelectedElements2 (false, true);

    GS::ObjectState response;
    GS::Array<GS::ObjectState> elementsArray;

    DBprnt ("GetPropertiesList: selectedElements=" + GS::ValueToUniString (selectedElements.GetSize ()));
    DBprnt ("GetPropertiesList: cacheValid=" + GS::ValueToUniString (cache.selectionPropertiesCacheValid));
    DBprnt ("GetPropertiesList: cacheSize=" + GS::ValueToUniString (cache.selectionPropertiesCache.GetSize ()));

    // Если есть выделенные элементы и кэш валиден
    if (!selectedElements.IsEmpty () && cache.selectionPropertiesCacheValid) {
        // Проверяем, все ли выделенные элементы есть в кэше
        bool allInCache = true;
        for (const API_Guid &elemGuid : selectedElements) {
            if (!cache.selectionPropertiesCache.ContainsKey (elemGuid)) {
                allInCache = false;
                DBprnt ("GetPropertiesList: element NOT in cache: " + APIGuidToString (elemGuid));
                break;
            }
        }

        if (allInCache) {
            DBprnt ("GetPropertiesList: RETURNING FROM CACHE");
            // Возвращаем из кэша
            for (const API_Guid &elemGuid : selectedElements) {
                const GS::Array<GS::ObjectState> &cachedProps = cache.selectionPropertiesCache[elemGuid];
                GS::ObjectState elementObj;
                elementObj.Add ("guid", APIGuidToString (elemGuid).ToCStr ().Get ());
                elementObj.Add ("properties", cachedProps);
                elementsArray.Push (elementObj);
            }

            response.Add ("elements", elementsArray);
            response.Add ("count", elementsArray.GetSize ());
            response.Add ("status", "ok");
            response.Add ("fromCache", true);

            return response;
        }
    }

    DBprnt ("GetPropertiesList: READING FRESH (cache invalid or elements not in cache)");
    // Кэш невалиден или элементы не в кэше — читаем заново
    cache.selectionPropertiesCache.Clear ();
    cache.selectionPropertiesCacheValid = false;

    // Для каждого выделенного элемента читаем свойства
    for (const API_Guid &elemGuid : selectedElements) {
        GS::Array<API_PropertyDefinition> definitions;
        GSErrCode err =
            ACAPI_Element_GetPropertyDefinitions (elemGuid, API_PropertyDefinitionFilter_UserDefined, definitions);
        if (err != NoError) {
            continue; // Пропускаем элемент при ошибке
        }

        if (definitions.IsEmpty ()) {
            continue;
        }

        GS::Array<API_Property> properties;
        err = ACAPI_Element_GetPropertyValues (elemGuid, definitions, properties);
        if (err != NoError) {
            continue;
        }

        GS::ObjectState elementObj;
        elementObj.Add ("guid", APIGuidToString (elemGuid).ToCStr ().Get ());

        GS::Array<GS::ObjectState> propertiesArray;

        // Проходим по полученным свойствам
        for (const API_Property &prop : properties) {
            GS::ObjectState propertyObj;

            // Имя свойства
            if (!prop.definition.name.IsEmpty ()) {
                propertyObj.Add ("name", prop.definition.name.ToCStr ().Get ());
            }

            // Значение свойства - используем ParamValue (полную структуру)
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
                propertyObj.Add ("value", valueStr.ToCStr ().Get ());
            }

            // Тип значения
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
            propertyObj.Add ("valueType", typeStr);

            // GUID свойства
            propertyObj.Add ("propertyGuid", APIGuidToString (prop.definition.guid).ToCStr ().Get ());

            propertiesArray.Push (propertyObj);
        }

        elementObj.Add ("properties", propertiesArray);
        elementsArray.Push (elementObj);

        // Сохраняем в кэш
        cache.selectionPropertiesCache.Put (elemGuid, propertiesArray);
    }

    cache.selectionPropertiesCacheValid = true;
    DBprnt ("GetPropertiesList: cached " + GS::ValueToUniString (cache.selectionPropertiesCache.GetSize ()) +
            " elements");

    response.Add ("elements", elementsArray);
    response.Add ("count", elementsArray.GetSize ());
    response.Add ("status", "ok");
    response.Add ("fromCache", false);

    return response;
}