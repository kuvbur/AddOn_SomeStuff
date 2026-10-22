#include "ACAPinc.h"

#include "json_commands/JsonCommandRegistrar.hpp"

#include "CommonFunction.hpp"

#if defined(ServerMainVers_2500)

    #include "json_commands/HealthCommand.hpp"
    #include "json_commands/RoomBookCommand.hpp"
    #include "json_commands/SpecCommand.hpp"
    #include "json_commands/SyncAllCommand.hpp"

    #if defined(TESTING) && defined(ServerMainVers_2500)
        #include "tests/TestRoombookGeometry.hpp"
    #endif

// -----------------------------------------------------------------------------
// Регистрирует доступные JSON-команды аддона в Archicad.
// -----------------------------------------------------------------------------
void RegisterJsonCommands () {
    #if defined(TESTING) && defined(ServerMainVers_2500)
    GS::Owner<RoomGeometryDiagnosticCommand> geometryCommand = GS::NewOwned<RoomGeometryDiagnosticCommand> ();
        #ifdef ServerMainVers_2700
    const GSErrCode geometryErr = ACAPI_AddOnAddOnCommunication_InstallAddOnCommandHandler (geometryCommand.Pass ());
        #else
    const GSErrCode geometryErr = ACAPI_Install_AddOnCommandHandler (geometryCommand.Pass ());
        #endif
    if (geometryErr != NoError)
        DBprnt ("Z0 registration error: " + GS::ValueToUniString (geometryErr));
    #endif

    GS::Owner<RoomBookCommand> roomBookCommand = GS::NewOwned<RoomBookCommand> ();
    #ifdef ServerMainVers_2700
    const GSErrCode roomBookErr = ACAPI_AddOnAddOnCommunication_InstallAddOnCommandHandler (roomBookCommand.Pass ());
    #else
    const GSErrCode roomBookErr = ACAPI_Install_AddOnCommandHandler (roomBookCommand.Pass ());
    #endif
    if (roomBookErr != NoError)
        DBprnt ("Failed to register RoomBookCommand, error: " + GS::ValueToUniString (roomBookErr));

    GS::Owner<SpecCommand> specCommand = GS::NewOwned<SpecCommand> ();
    #ifdef ServerMainVers_2700
    const GSErrCode specErr = ACAPI_AddOnAddOnCommunication_InstallAddOnCommandHandler (specCommand.Pass ());
    #else
    const GSErrCode specErr = ACAPI_Install_AddOnCommandHandler (specCommand.Pass ());
    #endif
    if (specErr != NoError)
        DBprnt ("Failed to register SpecCommand, error: " + GS::ValueToUniString (specErr));

    GS::Owner<SyncAllCommand> syncAllCommand = GS::NewOwned<SyncAllCommand> ();
    #ifdef ServerMainVers_2700
    const GSErrCode syncAllErr = ACAPI_AddOnAddOnCommunication_InstallAddOnCommandHandler (syncAllCommand.Pass ());
    #else
    const GSErrCode syncAllErr = ACAPI_Install_AddOnCommandHandler (syncAllCommand.Pass ());
    #endif
    if (syncAllErr != NoError)
        DBprnt ("Failed to register SyncAllCommand, error: " + GS::ValueToUniString (syncAllErr));

    GS::Owner<HealthCommand> healthCommand = GS::NewOwned<HealthCommand> ();
    #ifdef ServerMainVers_2700
    const GSErrCode healthErr = ACAPI_AddOnAddOnCommunication_InstallAddOnCommandHandler (healthCommand.Pass ());
    #else
    const GSErrCode healthErr = ACAPI_Install_AddOnCommandHandler (healthCommand.Pass ());
    #endif
    if (healthErr != NoError)
        DBprnt ("Failed to register HealthCommand, error: " + GS::ValueToUniString (healthErr));
}

#else

// -----------------------------------------------------------------------------
// Ничего не регистрирует в версиях Archicad без поддержки JSON-команд.
// -----------------------------------------------------------------------------
void RegisterJsonCommands () {}

#endif // defined (ServerMainVers_2500)
