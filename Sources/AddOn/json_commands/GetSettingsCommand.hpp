#pragma once
#include "CommandBase.hpp"
#include "dialogs/SyncSettings.hpp"

/**
 * Команда для чтения текущих настроек синхронизации (SyncSettings).
 * Возвращает все флаги настроек в формате JSON.
 */
class GetSettingsCommand : public ReadOnlyCommand
{
public:
    GetSettingsCommand();
    
    virtual GS::String GetName() const override;
    virtual GS::Optional<GS::UniString> GetInputParametersSchema() const override;
    virtual GS::Optional<GS::UniString> GetResponseSchema() const override;
    virtual GS::ObjectState Execute(const GS::ObjectState& parameters,
                                   GS::ProcessControl& processControl) const override;
};
