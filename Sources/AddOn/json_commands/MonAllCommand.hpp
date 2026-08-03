#pragma once
#include "CommandBase.hpp"
#include "dialogs/SyncSettings.hpp"

/**
 * Команда для включения/выключения мониторинга элементов (MonAll).
 * Эквивалент выбора "Mon All" в меню аддона.
 */
class MonAllCommand : public ModifyCommand
{
public:
    MonAllCommand();
    
    virtual GS::String GetName() const override;
    virtual GS::Optional<GS::UniString> GetInputParametersSchema() const override;
    virtual GS::Optional<GS::UniString> GetResponseSchema() const override;
    virtual GS::ObjectState Execute(const GS::ObjectState& parameters,
                                   GS::ProcessControl& processControl) const override;
};
