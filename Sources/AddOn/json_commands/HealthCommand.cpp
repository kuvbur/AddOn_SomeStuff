#include "HealthCommand.hpp"
#include "api_headers/ResourceIds.hpp"
#include "ACAPinc.h"

HealthCommand::HealthCommand()
    : ReadOnlyCommand()
{
}

GS::String HealthCommand::GetName() const
{
    return "Health";
}

GS::Optional<GS::UniString> HealthCommand::GetInputParametersSchema() const
{
    return GS::NoValue;
}

GS::Optional<GS::UniString> HealthCommand::GetResponseSchema() const
{
    return GS::NoValue;
}

GS::ObjectState HealthCommand::Execute(const GS::ObjectState& /*parameters*/,
                                       GS::ProcessControl& /*processControl*/) const
{
    // Получаем версию из ресурсов (ID_ADDON_STRINGS = 32501, VersionId = 49)
    GS::UniString version = RSGetIndString (ID_ADDON_STRINGS, VersionId, ACAPI_GetOwnResModule ());
    
    GS::ObjectState response;
    response.Add("status", "ok");
    response.Add("version", version.ToCStr().Get());
    response.Add("addon", "SomeStuff");
    
    return response;
}