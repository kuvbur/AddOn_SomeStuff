#pragma once
#include "CommandBase.hpp"
#include "dialogs/SyncSettings.hpp"

/**
 * Команда для запуска полной синхронизации всех элементов (SyncAll).
 * Эквивалент выбора "Sync All" в меню аддона.
 */
class SyncAllCommand : public ModifyCommand
{
public:
    SyncAllCommand();
    
    virtual GS::String GetName() const override;
    virtual GS::Optional<GS::UniString> GetInputParametersSchema() const override;
    virtual GS::Optional<GS::UniString> GetResponseSchema() const override;
    virtual GS::ObjectState Execute(const GS::ObjectState& parameters,
                                   GS::ProcessControl& processControl) const override;
};
