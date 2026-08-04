#pragma once
#include "CommandBase.hpp"
#include "dialogs/CommandHelpers.hpp"
#include "CommonFunction.hpp"

/**
 * Команда для парсинга описания свойства с привязкой к элементу.
 * Возвращает структурированные правила синхронизации и другие команды.
 */
class ParsePropertyForElementCommand : public ReadOnlyCommand
{
public:
    ParsePropertyForElementCommand();

    virtual GS::String GetName() const override;
    virtual GS::Optional<GS::UniString> GetInputParametersSchema() const override;
    virtual GS::Optional<GS::UniString> GetResponseSchema() const override;
    virtual GS::ObjectState Execute(const GS::ObjectState& parameters,
                                   GS::ProcessControl& processControl) const override;
};