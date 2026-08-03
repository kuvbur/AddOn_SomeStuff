#pragma once
#include "CommandBase.hpp"
#include "dialogs/SyncSettings.hpp"

/**
 * Команда для записи настроек синхронизации (SyncSettings).
 * Принимает JSON с флагами настроек и сохраняет их в Preferences.
 */
class SetSettingsCommand : public ModifyCommand
{
public:
    SetSettingsCommand();
    
    virtual GS::String GetName() const override;
    virtual GS::Optional<GS::UniString> GetInputParametersSchema() const override;
    virtual GS::Optional<GS::UniString> GetResponseSchema() const override;
    virtual GS::ObjectState Execute(const GS::ObjectState& parameters,
                                   GS::ProcessControl& processControl) const override;
};
