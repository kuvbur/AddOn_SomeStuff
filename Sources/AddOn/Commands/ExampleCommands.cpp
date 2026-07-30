#include "ExampleCommands.hpp"
#include <stdio.h>

ExampleReadOnlyCommand::ExampleReadOnlyCommand()
    : ReadOnlyCommand()
{
}

GS::String ExampleReadOnlyCommand::GetName() const
{
    return "ExampleReadOnly";
}

GS::Optional<GS::UniString> ExampleReadOnlyCommand::GetInputParametersSchema() const
{
    return GS::Optional<GS::UniString>();  // Пустой = нет схемы
}

GS::Optional<GS::UniString> ExampleReadOnlyCommand::GetResponseSchema() const
{
    return GS::Optional<GS::UniString>();  // Пустой = нет схемы
}

GS::ObjectState ExampleReadOnlyCommand::Execute(const GS::ObjectState& parameters,
                                               GS::ProcessControl& /*processControl*/) const
{
    GS::UniString filter;
    parameters.Get("filter", filter);
    
    GS::ObjectState response;
    response.Add("status", "ok");
    return response;
}

ExampleModifyCommand::ExampleModifyCommand()
    : ModifyCommand()
{
}

GS::String ExampleModifyCommand::GetName() const
{
    return "ExampleModify";
}

GS::Optional<GS::UniString> ExampleModifyCommand::GetInputParametersSchema() const
{
    return GS::Optional<GS::UniString>();  // Пустой = нет схемы
}

GS::Optional<GS::UniString> ExampleModifyCommand::GetResponseSchema() const
{
    return GS::Optional<GS::UniString>();  // Пустой = нет схемы
}

GS::ObjectState ExampleModifyCommand::Execute(const GS::ObjectState& parameters,
                                              GS::ProcessControl& /*processControl*/) const
{
    // Пример использования ACAPI_CallUndoableCommand
    GSErrCode err = ACAPI_CallUndoableCommand("Example Modify", [&]() -> GSErrCode {
        // Здесь код изменения проекта
        return NoError;
    });
    
    if (err != NoError) {
        return CreateErrorResponse(err, "Failed");
    }
    
    return CreateSuccessResponse();
}
