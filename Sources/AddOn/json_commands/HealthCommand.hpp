#pragma once
#include "CommandBase.hpp"
#include "CommonFunction.hpp"

/**
 * Health команда для проверки работоспособности аддона.
 * Возвращает версию аддона и статус.
 */
class HealthCommand : public ReadOnlyCommand
{
public:
    HealthCommand();
    
    virtual GS::String GetName() const override;
    virtual GS::Optional<GS::UniString> GetInputParametersSchema() const override;
    virtual GS::Optional<GS::UniString> GetResponseSchema() const override;
    virtual GS::ObjectState Execute(const GS::ObjectState& parameters,
                                     GS::ProcessControl& processControl) const override;
};