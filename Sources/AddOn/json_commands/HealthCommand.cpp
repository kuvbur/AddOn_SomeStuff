#include "ACAPinc.h"

#include "api_headers/ResourceIds.hpp"

#include "json_commands/HealthCommand.hpp"

#include "Constants.hpp"

#if defined(ServerMainVers_2500)

// -----------------------------------------------------------------------------
// Создаёт команду проверки работоспособности аддона.
// -----------------------------------------------------------------------------
HealthCommand::HealthCommand () : CommandBase (CommonSchema::NotUsed) {}

// -----------------------------------------------------------------------------
// Возвращает имя команды проверки работоспособности.
// -----------------------------------------------------------------------------
GS::String HealthCommand::GetName () const { return "Health"; }

// -----------------------------------------------------------------------------
// Указывает, что команда проверки работоспособности не принимает параметры.
// -----------------------------------------------------------------------------
GS::Optional<GS::UniString> HealthCommand::GetInputParametersSchema () const { return GS::NoValue; }

// -----------------------------------------------------------------------------
// Указывает, что команда проверки работоспособности возвращает свободный ответ.
// -----------------------------------------------------------------------------
GS::Optional<GS::UniString> HealthCommand::GetResponseSchema () const { return GS::NoValue; }

// -----------------------------------------------------------------------------
// Возвращает статус и версию аддона для проверки его работоспособности.
// -----------------------------------------------------------------------------
GS::ObjectState HealthCommand::Execute (const GS::ObjectState & /*parameters*/,
                                        GS::ProcessControl & /*processControl*/) const {
    // Получаем версию из ресурсов (ID_ADDON_STRINGS = 32501, VersionId = 49)
    GS::UniString version = RSGetIndString (ID_ADDON_STRINGS, VersionId, ACAPI_GetOwnResModule ());

    GS::ObjectState response;
    response.Add ("status", "ok");
    response.Add ("version", version.ToCStr ().Get ());
    response.Add ("addon", "SomeStuff");

    return response;
}

#endif // defined (ServerMainVers_2500)