#include "SyncAllCommand.hpp"
#include "Sync.hpp"
#include "Propertycache.hpp"

SyncAllCommand::SyncAllCommand()
    : ModifyCommand()
{
}

GS::String SyncAllCommand::GetName() const
{
    return "SyncAll";
}

GS::Optional<GS::UniString> SyncAllCommand::GetInputParametersSchema() const
{
    return GS::NoValue;
}

GS::Optional<GS::UniString> SyncAllCommand::GetResponseSchema() const
{
    return GS::NoValue;
}

GS::ObjectState SyncAllCommand::Execute(const GS::ObjectState& /*parameters*/,
                                       GS::ProcessControl& /*processControl*/) const
{
    // Загружаем настройки
    SyncSettings syncSettings;
    LoadSyncSettingsFromPreferences(syncSettings, true);
    
    // Обновляем кэш свойств
    PROPERTYCACHE().Update();
    
    // Выполняем полную синхронизацию
    syncSettings.SetSyncAll(true);
    SyncAndMonAll(syncSettings);
    syncSettings.SetSyncAll(false);
    
    GS::ObjectState response;
    response.Add("status", "ok");
    response.Add("message", "SyncAll completed successfully");
    
    return response;
}
