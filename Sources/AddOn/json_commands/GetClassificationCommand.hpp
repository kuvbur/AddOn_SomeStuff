// ---------------------------------------------------------------
// GetClassificationCommand - получение дерева классификации для выделения
// ---------------------------------------------------------------
#pragma once

#include "json_commands/CommandBase.hpp"
#include "ClassificationFunction.hpp"

class GetClassificationCommand : public ReadOnlyCommand {
public:
    GetClassificationCommand () : ReadOnlyCommand () {}

    GS::String GetName () const override { return "GetClassification"; }

    GS::Optional<GS::UniString> GetInputParametersSchema () const override {
        return GS::NoValue;
    }

    GS::Optional<GS::UniString> GetResponseSchema () const override {
        return GS::NoValue;
    }

    GS::ObjectState Execute (const GS::ObjectState &parameters,
                             GS::ProcessControl &processControl) const override;
};