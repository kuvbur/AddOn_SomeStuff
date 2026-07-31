#include "GetSettingsCommand.hpp"

GetSettingsCommand::GetSettingsCommand()
    : ReadOnlyCommand()
{
}

GS::String GetSettingsCommand::GetName() const
{
    return "GetSettings";
}

GS::Optional<GS::UniString> GetSettingsCommand::GetInputParametersSchema() const
{
    return GS::NoValue;
}

GS::Optional<GS::UniString> GetSettingsCommand::GetResponseSchema() const
{
    return GS::NoValue;
}

GS::ObjectState GetSettingsCommand::Execute(const GS::ObjectState& /*parameters*/,
                                           GS::ProcessControl& /*processControl*/) const
{
    // Загружаем настройки из кэша (без принудительной перезагрузки)
    SyncSettings syncSettings;
    LoadSyncSettingsFromPreferences(syncSettings, false);
    
    GS::ObjectState response;
    
    // Заполняем ответ всеми флагами SyncSettings
    response.Add("syncAll", syncSettings.syncAll);
    response.Add("syncMon", syncSettings.syncMon);
    response.Add("wallS", syncSettings.wallS);
    response.Add("widoS", syncSettings.widoS);
    response.Add("objS", syncSettings.objS);
    response.Add("cwallS", syncSettings.cwallS);
    response.Add("logMon", syncSettings.logMon);
    response.Add("showpalette", syncSettings.showpalette);
    
    response.Add("status", "ok");
    
    return response;
}
