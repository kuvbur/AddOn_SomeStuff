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
// Указывает, что команда построения спецификации не принимает параметры.
// -----------------------------------------------------------------------------
GS::Optional<GS::UniString> SpecCommand::GetInputParametersSchema () const { return GS::NoValue; }

// -----------------------------------------------------------------------------
// Указывает, что команда построения спецификации возвращает свободный ответ.
// -----------------------------------------------------------------------------
GS::Optional<GS::UniString> SpecCommand::GetResponseSchema () const { return GS::NoValue; }

// -----------------------------------------------------------------------------
// Загружает настройки, запускает построение спецификации и возвращает длительность вызова.
// -----------------------------------------------------------------------------
GS::ObjectState SpecCommand::Execute (const GS::ObjectState & /*parameters*/,
                                      GS::ProcessControl & /*processControl*/) const {
    const auto start = std::chrono::steady_clock::now ();
    SyncSettings syncSettings;
    LoadSyncSettingsFromPreferences (syncSettings);
    Spec::SpecAll (syncSettings);
    const auto finish = std::chrono::steady_clock::now ();

    const double elapsedSeconds = std::chrono::duration<double> (finish - start).count ();
    GS::ObjectState response;
    response.Add ("status", "returned");
    response.Add ("elapsedSeconds", elapsedSeconds);
    return response;
}

#endif // defined (ServerMainVers_2500)