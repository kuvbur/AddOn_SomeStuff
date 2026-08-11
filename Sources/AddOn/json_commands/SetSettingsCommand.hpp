#if !defined(SETSETTINGSCOMMAND_HPP)
#define SETSETTINGSCOMMAND_HPP
#include "CommandBase.hpp"
#include "dialogs/SyncSettings.hpp"

// -----------------------------------------------------------------------------
// Команда для записи настроек синхронизации (SyncSettings).
//
// Принимает JSON с флагами и сохраняет их в Preferences.
// -----------------------------------------------------------------------------
class SetSettingsCommand : public ModifyCommand
{
public:
    SetSettingsCommand();
    
    // -------------------------------------------------------------------------
    // @return Имя команды JSON.
    // -------------------------------------------------------------------------
    virtual GS::String GetName() const override;

    // -------------------------------------------------------------------------
    // @return Схема входных параметров. В этой команде нет схемы.
    // -------------------------------------------------------------------------
    virtual GS::Optional<GS::UniString> GetInputParametersSchema() const override;

    // -------------------------------------------------------------------------
    // @return Схема ответа. В этой команде схема не задаётся.
    // -------------------------------------------------------------------------
    virtual GS::Optional<GS::UniString> GetResponseSchema() const override;

    // -------------------------------------------------------------------------
    // Выполняет команду и сохраняет переданные настройки синхронизации.
    // -------------------------------------------------------------------------
    virtual GS::ObjectState Execute(const GS::ObjectState& parameters,
                                   GS::ProcessControl& processControl) const override;
};

#endif // SETSETTINGSCOMMAND_HPP
