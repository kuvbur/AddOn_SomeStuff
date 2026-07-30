#pragma once
#include "CommandBase.hpp"
#include "Propertycache.hpp"
#include "CommonFunction.hpp"

/**
 * Команда для получения списка всех доступных свойств из кэша.
 * Возвращает свойства, уже загруженные в PROPERTYCACHE().property
 */
class GetPropertyDefinitionsCommand : public ReadOnlyCommand
{
public:
    GetPropertyDefinitionsCommand();
    
    virtual GS::String GetName() const override;
    virtual GS::Optional<GS::UniString> GetInputParametersSchema() const override;
    virtual GS::Optional<GS::UniString> GetResponseSchema() const override;
    virtual GS::ObjectState Execute(const GS::ObjectState& parameters,
                                   GS::ProcessControl& processControl) const override;
};
