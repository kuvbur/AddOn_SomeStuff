#include "CommandBase.hpp"

GS::String CommandNamespace() { return "SomeStuffCommand"; }

CommandBase::CommandBase(CommonSchema commonSchema) 
    : mCommonSchema(commonSchema)
{
}

GS::String CommandBase::GetNamespace() const
{
    return CommandNamespace();
}

API_AddOnCommandExecutionPolicy CommandBase::GetExecutionPolicy() const
{
    return API_AddOnCommandExecutionPolicy::ScheduleForExecutionOnMainThread;
}

void CommandBase::OnResponseValidationFailed(const GS::ObjectState& /*response*/) const
{
}

GS::Optional<GS::UniString> CommandBase::GetSchemaDefinitions() const
{
    return GS::Optional<GS::UniString>();
}

GS::Optional<GS::UniString> CommandBase::GetInputParametersSchema() const
{
    return GS::Optional<GS::UniString>();
}

GS::Optional<GS::UniString> CommandBase::GetResponseSchema() const
{
    return GS::Optional<GS::UniString>();
}

GS::ObjectState CreateErrorResponse(GSErrCode errorCode, const GS::UniString& errorMessage)
{
    GS::ObjectState os;
    os.Add("error", GS::ObjectState("code", errorCode));
    os.Add("message", errorMessage);
    return os;
}

GS::ObjectState CreateSuccessResponse()
{
    return GS::ObjectState("success", true);
}

GS::ObjectState ReadOnlyCommand::CreateErrorResponse(GSErrCode errorCode, const GS::UniString& errorMessage) const
{
    GS::ObjectState os;
    os.Add("error", GS::ObjectState("code", errorCode));
    os.Add("message", errorMessage);
    return os;
}

GS::ObjectState ReadOnlyCommand::CreateSuccessResponse() const
{
    return GS::ObjectState("success", true);
}

GS::ObjectState ModifyCommand::CreateErrorResponse(GSErrCode errorCode, const GS::UniString& errorMessage) const
{
    GS::ObjectState os;
    os.Add("error", GS::ObjectState("code", errorCode));
    os.Add("message", errorMessage);
    return os;
}

GS::ObjectState ModifyCommand::CreateSuccessResponse() const
{
    return GS::ObjectState("success", true);
}
