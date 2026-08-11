#if !defined(PARSEPROPERTYFORELEMENTCOMMAND_HPP)
#define PARSEPROPERTYFORELEMENTCOMMAND_HPP
#include "CommandBase.hpp"
#include "dialogs/CommandHelpers.hpp"
#include "CommonFunction.hpp"

// -----------------------------------------------------------------------------
// Команда парсинга описания свойства с привязкой к элементу.
//
// Принимает текстовое описание и GUID элемента, возвращает структурированные
// правила синхронизации и другие найденные команды.
// -----------------------------------------------------------------------------
class ParsePropertyForElementCommand : public ReadOnlyCommand
{
public:
    ParsePropertyForElementCommand();

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
    // Выполняет парсинг описания свойства и возвращает список правил и команд.
    // -------------------------------------------------------------------------
    virtual GS::ObjectState Execute(const GS::ObjectState& parameters,
                                   GS::ProcessControl& processControl) const override;
};

#endif // PARSEPROPERTYFORELEMENTCOMMAND_HPP