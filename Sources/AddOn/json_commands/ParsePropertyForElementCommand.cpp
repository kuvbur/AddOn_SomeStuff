#include "ParsePropertyForElementCommand.hpp"

ParsePropertyForElementCommand::ParsePropertyForElementCommand()
    : ReadOnlyCommand()
{
}

GS::String ParsePropertyForElementCommand::GetName() const
{
    return "ParsePropertyForElement";
}

GS::Optional<GS::UniString> ParsePropertyForElementCommand::GetInputParametersSchema() const
{
    return GS::NoValue;
}

GS::Optional<GS::UniString> ParsePropertyForElementCommand::GetResponseSchema() const
{
    return GS::NoValue;
}

GS::ObjectState ParsePropertyForElementCommand::Execute(const GS::ObjectState& parameters,
                                                        GS::ProcessControl& /*processControl*/) const
{
    // Получаем описание свойства
    GS::UniString description;
    if (!parameters.Get("description", description)) {
        GS::ObjectState os;
        os.Add("error", "Missing 'description' parameter");
        os.Add("status", "error");
        return os;
    }

    // Получаем GUID элемента
    GS::UniString elemGuidStr;
    if (!parameters.Get("elemGuid", elemGuidStr)) {
        GS::ObjectState os;
        os.Add("error", "Missing 'elemGuid' parameter");
        os.Add("status", "error");
        return os;
    }

    // Парсим GUID
    API_Guid elemGuid = APIGuidFromString(elemGuidStr.ToCStr(0, MaxUSize, GChCode));
    if (elemGuid == APINULLGuid) {
        GS::ObjectState os;
        os.Add("error", "Invalid GUID format");
        os.Add("status", "error");
        return os;
    }

    // Парсим описание через новую функцию
    ParsePropertyResult parseResult = ParsePropertyDescriptionToRules(description);

    // Формируем ответ
    GS::ObjectState response;

    GS::Array<GS::ObjectState> syncRulesArray;
    for (const auto& rule : parseResult.syncRules) {
        GS::ObjectState ruleObj;
        ruleObj.Add("commandType", rule.commandType.ToCStr().Get());
        ruleObj.Add("fullCommand", rule.fullCommand.ToCStr().Get());
        ruleObj.Add("parameters", rule.parameters.ToCStr().Get());
        ruleObj.Add("sourceType", rule.sourceType.ToCStr().Get());
        ruleObj.Add("sourceName", rule.sourceName.ToCStr().Get());
        ruleObj.Add("targetType", rule.targetType.ToCStr().Get());
        ruleObj.Add("targetName", rule.targetName.ToCStr().Get());
        ruleObj.Add("formatString", rule.formatString.ToCStr().Get());
        ruleObj.Add("isValid", rule.isValid);
        ruleObj.Add("errorMessage", rule.errorMessage.ToCStr().Get());
        ruleObj.Add("hasSub", rule.hasSub);
        ruleObj.Add("hasGUID", rule.hasGUID);
        ruleObj.Add("guidSourceProperty", rule.guidSourceProperty.ToCStr().Get());

        // ignoreVals
        GS::Array<GS::ObjectState> ignoreValsArray;
        for (const auto& iv : rule.ignoreVals) {
            GS::ObjectState valObj;
            valObj.Add("value", iv.ToCStr().Get());
            ignoreValsArray.Push(valObj);
        }
        ruleObj.Add("ignoreVals", ignoreValsArray);

        syncRulesArray.Push(ruleObj);
    }

    GS::Array<GS::ObjectState> otherCommandsArray;
    for (const auto& cmd : parseResult.otherCommands) {
        GS::ObjectState cmdObj;
        cmdObj.Add("commandType", cmd.commandType.ToCStr().Get());
        cmdObj.Add("fullCommand", cmd.fullCommand.ToCStr().Get());
        cmdObj.Add("parameters", cmd.parameters.ToCStr().Get());
        cmdObj.Add("isValid", cmd.isValid);
        cmdObj.Add("errorMessage", cmd.errorMessage.ToCStr().Get());
        otherCommandsArray.Push(cmdObj);
    }

    response.Add("syncRules", syncRulesArray);
    response.Add("otherCommands", otherCommandsArray);
    response.Add("hasSyncRules", parseResult.hasSyncRules);
    response.Add("hasOtherCommands", parseResult.hasOtherCommands);
    response.Add("remainingText", parseResult.remainingText.ToCStr().Get());
    response.Add("status", "ok");

    return response;
}