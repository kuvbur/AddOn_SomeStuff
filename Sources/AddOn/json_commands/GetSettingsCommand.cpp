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
    response.Add("syncAll", syncSettings.GetSyncAll());
    response.Add("syncMon", syncSettings.GetSyncMon());
    response.Add("wallS", syncSettings.GetWallS());
    response.Add("widoS", syncSettings.GetWidoS());
    response.Add("objS", syncSettings.GetObjS());
    response.Add("cwallS", syncSettings.GetCwallS());
    response.Add("logMon", syncSettings.GetLogMon());
    response.Add("showpalette", syncSettings.GetShowPalette());
    
    response.Add("status", "ok");
    
    return response;
}
