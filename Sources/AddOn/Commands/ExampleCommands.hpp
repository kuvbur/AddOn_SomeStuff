#pragma once

#include "CommandBase.hpp"

/**
 * Пример READ-ONLY команды (только чтение, БЕЗ undo).
 */
class ExampleReadOnlyCommand : public ReadOnlyCommand
{
public:
    ExampleReadOnlyCommand();
    
    virtual GS::String GetName() const override;
    virtual GS::Optional<GS::UniString> GetInputParametersSchema() const override;
    virtual GS::Optional<GS::UniString> GetResponseSchema() const override;
    virtual GS::ObjectState Execute(const GS::ObjectState& parameters,
                                   GS::ProcessControl& processControl) const override;
};

/**
 * Пример MODIFY команды (изменяет проект, С undo).
 */
class ExampleModifyCommand : public ModifyCommand
{
public:
    ExampleModifyCommand();
    
    virtual GS::String GetName() const override;
    virtual GS::Optional<GS::UniString> GetInputParametersSchema() const override;
    virtual GS::Optional<GS::UniString> GetResponseSchema() const override;
    virtual GS::ObjectState Execute(const GS::ObjectState& parameters,
                                   GS::ProcessControl& processControl) const override;
};
