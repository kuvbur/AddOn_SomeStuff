#if !defined(SYNCALLCOMMAND_HPP)
#define SYNCALLCOMMAND_HPP
#include "CommandBase.hpp"
#include "dialogs/SyncSettings.hpp"

// -----------------------------------------------------------------------------
// Команда запуска полной синхронизации всех элементов (SyncAll).
//
// Эквивалент выбора "Sync All" в меню аддона и выполняет полный проход синхронизации.
// -----------------------------------------------------------------------------
class SyncAllCommand : public ModifyCommand
{
public:
    SyncAllCommand();
    
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
    // Выполняет команду и запускает полный синхронизатор.
    // -------------------------------------------------------------------------
    virtual GS::ObjectState Execute(const GS::ObjectState& parameters,
                                   GS::ProcessControl& processControl) const override;
};

#endif // SYNCALLCOMMAND_HPP
