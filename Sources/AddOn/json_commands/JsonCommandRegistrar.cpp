#include "ACAPinc.h"

#include "json_commands/JsonCommandRegistrar.hpp"

#include "CommonFunction.hpp"

#if defined(ServerMainVers_2500)

    #include "json_commands/RoomBookCommand.hpp"
    #include "json_commands/SpecCommand.hpp"

// -----------------------------------------------------------------------------
// Регистрирует доступные JSON-команды аддона в Archicad.
// -----------------------------------------------------------------------------
void RegisterJsonCommands () {
    GS::Owner<RoomBookCommand> roomBookCommand = GS::NewOwned<RoomBookCommand> ();
    const GSErrCode roomBookErr = ACAPI_Install_AddOnCommandHandler (roomBookCommand.Pass ());
    if (roomBookErr != NoError)
        DBprnt ("Failed to register RoomBookCommand, error: " + GS::ValueToUniString (roomBookErr));

    GS::Owner<SpecCommand> specCommand = GS::NewOwned<SpecCommand> ();
    const GSErrCode specErr = ACAPI_Install_AddOnCommandHandler (specCommand.Pass ());
    if (specErr != NoError)
        DBprnt ("Failed to register SpecCommand, error: " + GS::ValueToUniString (specErr));
}

#else

// -----------------------------------------------------------------------------
// Ничего не регистрирует в версиях Archicad без поддержки JSON-команд.
// -----------------------------------------------------------------------------
void RegisterJsonCommands () {}

#endif // defined (ServerMainVers_2500)
