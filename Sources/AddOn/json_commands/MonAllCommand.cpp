#include "MonAllCommand.hpp"
#include "Sync.hpp"
#include "Propertycache.hpp"

MonAllCommand::MonAllCommand()
    : ModifyCommand()
{
}

GS::String MonAllCommand::GetName() const
{
    return "MonAll";
}

GS::Optional<GS::UniString> MonAllCommand::GetInputParametersSchema() const
{
    return GS::NoValue;
}

GS::Optional<GS::UniString> MonAllCommand::GetResponseSchema() const
{
    return GS::NoValue;
}

GS::ObjectState MonAllCommand::Execute(const GS::ObjectState& parameters,
                                      GS::ProcessControl& /*processControl*/) const
{
    // Загружаем настройки
    SyncSettings syncSettings;
    LoadSyncSettingsFromPreferences(syncSettings, true);
    
    // Обновляем кэш свойств
    PROPERTYCACHE().Update();
    
    // Проверяем параметр enable (включить или выключить мониторинг)
    bool enable = true;
    parameters.Get("enable", enable);
    
    if (enable) {
        // Включаем мониторинг
        syncSettings.SetSyncMon(true);
        MonAll(syncSettings);
    } else {
        // Выключаем мониторинг (syncMon = false)
        syncSettings.SetSyncMon(false);
    }
    
    GS::ObjectState response;
    response.Add("status", "ok");
    response.Add("monitoring", syncSettings.GetSyncMon());
    response.Add("message", syncSettings.GetSyncMon() ? "Monitoring enabled" : "Monitoring disabled");
    
    return response;
}
