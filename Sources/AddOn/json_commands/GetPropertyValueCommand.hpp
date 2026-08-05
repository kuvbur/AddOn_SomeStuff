#pragma once
#include "CommonFunction.hpp"
#include "json_commands/CommandBase.hpp"

/**
 * Команда получения значения свойства для выделенных элементов.
 * Возвращает: { propertyName, common: bool, values: [{value, count}] }
 * Использует кэш для избежания повторного чтения.
 */
class GetPropertyValueCommand : public ReadOnlyCommand {
  public:
    GetPropertyValueCommand ();

    virtual GS::String GetName () const override;
    virtual GS::Optional<GS::UniString> GetInputParametersSchema () const override;
    virtual GS::Optional<GS::UniString> GetResponseSchema () const override;
    virtual GS::ObjectState Execute (const GS::ObjectState &parameters,
                                     GS::ProcessControl &processControl) const override;
};