#include "GetPropertyDefinitionsCommand.hpp"

GetPropertyDefinitionsCommand::GetPropertyDefinitionsCommand()
    : ReadOnlyCommand()
{
}

GS::String GetPropertyDefinitionsCommand::GetName() const
{
    return "GetPropertyDefinitions";
}

GS::Optional<GS::UniString> GetPropertyDefinitionsCommand::GetInputParametersSchema() const
{
    return GS::NoValue;
}

GS::Optional<GS::UniString> GetPropertyDefinitionsCommand::GetResponseSchema() const
{
    return GS::NoValue;
}

GS::ObjectState GetPropertyDefinitionsCommand::Execute(const GS::ObjectState& /*parameters*/,
                                                      GS::ProcessControl& /*processControl*/) const
{
    if (!ParamHelpers::isPropertyDefinitionRead()) {
        GS::ObjectState error;
        error.Add("error", "Failed to load property definitions");
        return error;
    }
    
    auto& cache = PROPERTYCACHE();
    
    GS::ObjectState response;
    GS::Array<GS::ObjectState> propertiesArray;
    
    // Проходим по всем свойствам в кэше
    for (const auto& pair : cache.property) {
        const GS::UniString& rawName = *pair.key;  // Разыменовываем указатель
        const ParamValue& paramValue = *pair.value;  // Разыменовываем указатель
        
        GS::ObjectState propertyObj;
        
        // rawName (ключ в кэше)
        propertyObj.Add("rawName", rawName.ToCStr().Get());
        
        // name (очищенное имя)
        if (!paramValue.name.IsEmpty()) {
            propertyObj.Add("name", paramValue.name.ToCStr().Get());
        }
        
        // displayName (имя свойства из определения)
        if (!paramValue.definition.name.IsEmpty()) {
            propertyObj.Add("displayName", paramValue.definition.name.ToCStr().Get());
        }
        
        // description (описание свойства)
        if (!paramValue.definition.description.IsEmpty()) {
            propertyObj.Add("description", paramValue.definition.description.ToCStr().Get());
        }
        
        // type (тип значения свойства)
        const char* typeStr = "unknown";
        switch (paramValue.val.type) {
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
        }
        propertyObj.Add("type", typeStr);
        
        propertiesArray.Push(propertyObj);
    }
    
    response.Add("properties", propertiesArray);
    
    // Добавляем количество свойств
    char countStr[32];
    sprintf(countStr, "%d", propertiesArray.GetSize());
    response.Add("count", countStr);
    response.Add("status", "ok");
    
    return response;
}
