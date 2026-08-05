#include "json_commands/GetPropertyValueCommand.hpp"

#include "CommonFunction.hpp"
#include "Propertycache.hpp"

GetPropertyValueCommand::GetPropertyValueCommand () : ReadOnlyCommand () {}

GS::String GetPropertyValueCommand::GetName () const { return "GetPropertyValue"; }

GS::Optional<GS::UniString> GetPropertyValueCommand::GetInputParametersSchema () const { return GS::NoValue; }

GS::Optional<GS::UniString> GetPropertyValueCommand::GetResponseSchema () const { return GS::NoValue; }

GS::ObjectState GetPropertyValueCommand::Execute (const GS::ObjectState &parameters,
                                                  GS::ProcessControl & /*processControl*/) const {
    // Получаем propertyId из параметров
    GS::UniString propertyId;
    if (!parameters.Get ("propertyId", propertyId) || propertyId.IsEmpty ()) {
        return CreateErrorResponse (APIERR_BADPARS, "Missing or empty propertyId parameter");
    }

    auto &cache = PROPERTYCACHE ();

    // Получаем GUID-ы выделенных элементов
    GS::Array<API_Guid> selectedElements = GetSelectedElements2 (false, true);

    GS::ObjectState response;
    
    if (selectedElements.IsEmpty ()) {
        response.Add ("propertyName", "");
        response.Add ("common", true);
        response.Add ("values", GS::Array<GS::ObjectState> ());
        return response;
    }

    // Используем кэш для получения значений
    GS::HashTable<GS::UniString, Int32> valueCounts;
    
    for (const API_Guid &elemGuid : selectedElements) {
        GS::UniString rawName = "Property:" + propertyId; // Формат для ParamHelpers
        ParamValue pvalue;
        if (ParamHelpers::GetParamValueFromCache (rawName, pvalue)) {
            // Получаем строковое представление значения
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
            
            // Увеличиваем счетчик для этого значения
            const Int32* currentCountPtr = valueCounts.GetPtr (valueStr);
            Int32 currentCount = currentCountPtr ? *currentCountPtr : 0;
            valueCounts.Put (valueStr, currentCount + 1);
        }
    }

    // Формируем ответ (используем EnumeratePairs с PairIterator для совместимости с AC25)
    GS::Array<GS::ObjectState> valuesArray;
    Int32 totalCount = 0;
    for (GS::HashTable<GS::UniString, Int32>::PairIterator it = valueCounts.EnumeratePairs (); it != nullptr; ++it) {
#if defined(ServerMainVers_2800) || defined(ServerMainVers_2900)
        const GS::UniString &key = it->key;
        Int32 value = it->value;
#else
        const GS::UniString &key = *it->key;
        Int32 value = *it->value;
#endif
        GS::ObjectState valueObj;
        valueObj.Add ("value", key.ToCStr ().Get ());
        valueObj.Add ("count", value);
        valuesArray.Push (valueObj);
        totalCount += value;
    }

    // Определяем, общее ли значение (все элементы имеют одно значение)
    bool isCommon = (valuesArray.GetSize () == 1 && totalCount == selectedElements.GetSize ());

    // Имя свойства - используем propertyId
    GS::UniString propertyName = propertyId;

    response.Add ("propertyName", propertyName.ToCStr ().Get ());
    response.Add ("common", isCommon);
    response.Add ("values", valuesArray);
    response.Add ("status", "ok");

    return response;
}