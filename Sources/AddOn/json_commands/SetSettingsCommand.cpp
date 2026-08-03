#include "SetSettingsCommand.hpp"

SetSettingsCommand::SetSettingsCommand()
    : ModifyCommand()
{
}

GS::String SetSettingsCommand::GetName() const
{
    return "SetSettings";
}

GS::Optional<GS::UniString> SetSettingsCommand::GetInputParametersSchema() const
{
    return GS::NoValue;
}

GS::Optional<GS::UniString> SetSettingsCommand::GetResponseSchema() const
{
    return GS::NoValue;
}

GS::ObjectState SetSettingsCommand::Execute(const GS::ObjectState& parameters,
                                           GS::ProcessControl& /*processControl*/) const
{
    // Создаём объект настроек с значениями по умолчанию
    SyncSettings syncSettings;
    
    // Читаем текущие настройки как базу (чтобы не сбросить непереданные поля)
    LoadSyncSettingsFromPreferences(syncSettings, false);
    
    // Обновляем поля из параметров запроса (если они переданы)
    bool paramValue;
    
    if (parameters.Get("syncAll", paramValue)) {
        syncSettings.SetSyncAll(paramValue);
    }
    if (parameters.Get("syncMon", paramValue)) {
        syncSettings.SetSyncMon(paramValue);
    }
    if (parameters.Get("wallS", paramValue)) {
        syncSettings.SetWallS(paramValue);
    }
    if (parameters.Get("widoS", paramValue)) {
        syncSettings.SetWidoS(paramValue);
    }
    if (parameters.Get("objS", paramValue)) {
        syncSettings.SetObjS(paramValue);
    }
    if (parameters.Get("cwallS", paramValue)) {
        syncSettings.SetCwallS(paramValue);
    }
    if (parameters.Get("logMon", paramValue)) {
        syncSettings.SetLogMon(paramValue);
    }
    if (parameters.Get("showpalette", paramValue)) {
        syncSettings.SetShowPalette(paramValue);
    }
    
    // Сохраняем настройки в Preferences
    bool success = WriteSyncSettingsToPreferences(syncSettings);
    
    GS::ObjectState response;
    
    if (success) {
        response.Add("status", "ok");
        response.Add("message", "Settings saved successfully");
    } else {
        response.Add("status", "error");
        response.Add("message", "Failed to save settings");
    }
    
    return response;
}
