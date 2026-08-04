#pragma once
#include "CommonFunction.hpp"
#include "json_commands/CommandBase.hpp"

/**
 * Команда получения информации о текущем выделении элементов.
 * Возвращает количество выделенных элементов и базовую сводку.
 */
class GetSelectionInfoCommand : public ReadOnlyCommand {
  public:
    GetSelectionInfoCommand ();

    virtual GS::String GetName () const override;
    virtual GS::Optional<GS::UniString> GetInputParametersSchema () const override;
    virtual GS::Optional<GS::UniString> GetResponseSchema () const override;
    virtual GS::ObjectState Execute (const GS::ObjectState &parameters,
                                     GS::ProcessControl &processControl) const override;
};