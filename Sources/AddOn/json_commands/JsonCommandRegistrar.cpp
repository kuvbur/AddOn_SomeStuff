#include "ACAPinc.h"

#include "json_commands/JsonCommandRegistrar.hpp"

#include "CommonFunction.hpp"
#include "json_commands/HealthCommand.hpp"
#include "json_commands/RoomBookCommand.hpp"
#include "json_commands/SpecCommand.hpp"
#include "json_commands/SyncAllCommand.hpp"

// -----------------------------------------------------------------------------
// Регистрирует доступные JSON-команды аддона в Archicad.
// -----------------------------------------------------------------------------
void RegisterJsonCommands () {
#if defined(ServerMainVers_2500)
    GS::Owner<RoomBookGetFavoritesCommand> favoritesListCommand = GS::NewOwned<RoomBookGetFavoritesCommand> ();
    #ifdef ServerMainVers_2700
    const GSErrCode error = ACAPI_AddOnAddOnCommunication_InstallAddOnCommandHandler (favoritesListCommand.Pass ());
    #else
    const GSErrCode error = ACAPI_Install_AddOnCommandHandler (favoritesListCommand.Pass ());
    #endif
    if (error != NoError)
        DBprnt ("Failed to register FavoritesListCommand, error: " + GS::ValueToUniString (error));

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
#endif // ServerMainVers_2500
}
