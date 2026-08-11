#if !defined(GETSETTINGSCOMMAND_HPP)
#define GETSETTINGSCOMMAND_HPP
#include "CommandBase.hpp"
#include "dialogs/SyncSettings.hpp"

// -----------------------------------------------------------------------------
// Команда для чтения текущих настроек синхронизации (SyncSettings).
//
// Возвращает состояние всех флагов, связанных с синхронизацией и мониторингом.
// -----------------------------------------------------------------------------
class GetSettingsCommand : public ReadOnlyCommand
{
public:
    GetSettingsCommand();
    
    // -------------------------------------------------------------------------
    // @return Имя команды JSON.
    // -------------------------------------------------------------------------
    virtual GS::String GetName() const override;

    // -------------------------------------------------------------------------
    // @return Схема входных параметров. В этой команде нет входных параметров.
    // -------------------------------------------------------------------------
    virtual GS::Optional<GS::UniString> GetInputParametersSchema() const override;

    // -------------------------------------------------------------------------
    // @return Схема ответа. В этой команде схема не задаётся.
    // -------------------------------------------------------------------------
    virtual GS::Optional<GS::UniString> GetResponseSchema() const override;

    // -------------------------------------------------------------------------
    // Выполняет команду и возвращает текущее состояние настроек синхронизации.
    // -------------------------------------------------------------------------
    virtual GS::ObjectState Execute(const GS::ObjectState& parameters,
                                   GS::ProcessControl& processControl) const override;
};

#endif // GETSETTINGSCOMMAND_HPP
