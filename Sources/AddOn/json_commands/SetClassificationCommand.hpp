// ---------------------------------------------------------------
// SetClassificationCommand - назначение классификации выделенным элементам
// ---------------------------------------------------------------
#pragma once

#include "json_commands/CommandBase.hpp"

class SetClassificationCommand : public ModifyCommand {
public:
    SetClassificationCommand () : ModifyCommand () {}

    GS::String GetName () const override { return "SetClassification"; }

    GS::Optional<GS::UniString> GetInputParametersSchema () const override {
        return GS::NoValue;
    }

    GS::Optional<GS::UniString> GetResponseSchema () const override {
        return GS::NoValue;
    }

    GS::ObjectState Execute (const GS::ObjectState &parameters,
                             GS::ProcessControl &processControl) const override;
};