#if !defined(GETSELECTIONINFOCOMMAND_HPP)
#define GETSELECTIONINFOCOMMAND_HPP
#include "CommonFunction.hpp"
#include "json_commands/CommandBase.hpp"

// -----------------------------------------------------------------------------
// Команда получения информации о текущем выделении элементов.
//
// Возвращает количество выделенных элементов.
// -----------------------------------------------------------------------------
class GetSelectionInfoCommand : public ReadOnlyCommand {
  public:
    GetSelectionInfoCommand ();

    // -------------------------------------------------------------------------
    // @return Имя команды JSON.
    // -------------------------------------------------------------------------
    virtual GS::String GetName () const override;

    // -------------------------------------------------------------------------
    // @return Схема входных параметров. В этой команде нет входных параметров.
    // -------------------------------------------------------------------------
    virtual GS::Optional<GS::UniString> GetInputParametersSchema () const override;

    // -------------------------------------------------------------------------
    // @return Схема ответа. В этой команде схема не задаётся.
    // -------------------------------------------------------------------------
    virtual GS::Optional<GS::UniString> GetResponseSchema () const override;

    // -------------------------------------------------------------------------
    // Выполняет команду и возвращает информацию о текущем выделении.
    // -------------------------------------------------------------------------
    virtual GS::ObjectState Execute (const GS::ObjectState &parameters,
                                     GS::ProcessControl &processControl) const override;
};

#endif // GETSELECTIONINFOCOMMAND_HPP