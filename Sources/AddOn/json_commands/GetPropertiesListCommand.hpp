#pragma once
#include "CommonFunction.hpp"
#include "json_commands/CommandBase.hpp"

/**
 * Команда получения значений свойств для выделенных элементов.
 * Возвращает массив элементов с их свойствами (имя, значение, тип).
 * Использует кэш для избежания повторного чтения.
 */
class GetPropertiesListCommand : public ReadOnlyCommand {
  public:
    GetPropertiesListCommand ();

    virtual GS::String GetName () const override;
    virtual GS::Optional<GS::UniString> GetInputParametersSchema () const override;
    virtual GS::Optional<GS::UniString> GetResponseSchema () const override;
    virtual GS::ObjectState Execute (const GS::ObjectState &parameters,
                                     GS::ProcessControl &processControl) const override;
};