#include <chrono>

#include "ACAPinc.h"

#include "json_commands/SpecCommand.hpp"

#include "dialogs/SyncSettings.hpp"
#include "spec/Spec.hpp"

#if defined(ServerMainVers_2500)

// -----------------------------------------------------------------------------
// Создаёт JSON-команду запуска построения спецификации.
// -----------------------------------------------------------------------------
SpecCommand::SpecCommand () : CommandBase (CommonSchema::NotUsed) {}

// -----------------------------------------------------------------------------
// Возвращает имя команды запуска построения спецификации.
// -----------------------------------------------------------------------------
GS::String SpecCommand::GetName () const { return "Spec"; }

// -----------------------------------------------------------------------------
// Возвращает схему параметров non-interactive запуска спецификации.
// -----------------------------------------------------------------------------
GS::Optional<GS::UniString> SpecCommand::GetInputParametersSchema () const {
    return R"({
        "type": "object",
        "properties": {
            "ruleNames": {
                "type": "array",
                "items": { "type": "string" },
                "minItems": 1
            },
            "placementPoint": {
                "type": "object",
                "properties": {
                    "x": { "type": "number" },
                    "y": { "type": "number" }
                },
                "additionalProperties": false,
                "required": ["x", "y"]
            }
        },
        "additionalProperties": false,
        "required": ["placementPoint"]
    })";
}

// -----------------------------------------------------------------------------
// Указывает, что команда построения спецификации возвращает свободный ответ.
// -----------------------------------------------------------------------------
GS::Optional<GS::UniString> SpecCommand::GetResponseSchema () const { return GS::NoValue; }

// -----------------------------------------------------------------------------
// Загружает настройки, запускает non-interactive построение спецификации и возвращает его результат.
// -----------------------------------------------------------------------------
GS::ObjectState SpecCommand::Execute (const GS::ObjectState &parameters,
                                      GS::ProcessControl & /*processControl*/) const {
    const GS::ObjectState *placementPointObject = parameters.Get ("placementPoint");
    if (placementPointObject == nullptr)
        return CreateErrorResponse (APIERR_BADPARS, "placementPoint is missing");

    Point2D placementPoint = {};
    if (!placementPointObject->Get ("x", placementPoint.x) || !placementPointObject->Get ("y", placementPoint.y))
        return CreateErrorResponse (APIERR_BADPARS, "placementPoint.x and placementPoint.y are required");

    GS::Array<GS::UniString> ruleNames;
    const bool hasRuleNames = parameters.Get ("ruleNames", ruleNames);
    const auto start = std::chrono::steady_clock::now ();
    SyncSettings syncSettings;
    LoadSyncSettingsFromPreferences (syncSettings);
    Spec::SpecRunResult runResult;
    const GSErrCode err =
        Spec::SpecAll (syncSettings, hasRuleNames ? &ruleNames : nullptr, &placementPoint, &runResult);
    const auto finish = std::chrono::steady_clock::now ();

    const double elapsedSeconds = std::chrono::duration<double> (finish - start).count ();
    GS::ObjectState response;
    response.Add ("status", err == NoError ? "completed" : "failed");
    response.Add ("resultCode", static_cast<GS::Int32> (err));
    response.Add ("elementsToCreate", static_cast<GS::Int32> (runResult.elementsToCreate));
    response.Add ("elementsToModify", static_cast<GS::Int32> (runResult.elementsToModify));
    response.Add ("elementsToDelete", static_cast<GS::Int32> (runResult.elementsToDelete));
    response.Add ("elapsedSeconds", elapsedSeconds);
    return response;
}

#endif // defined (ServerMainVers_2500)