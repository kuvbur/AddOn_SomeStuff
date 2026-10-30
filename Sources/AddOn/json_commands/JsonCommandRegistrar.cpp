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

    GS::Owner<RoomBookCreateFavoriteCommand> createFavoriteCommand = GS::NewOwned<RoomBookCreateFavoriteCommand> ();
    #ifdef ServerMainVers_2700
    const GSErrCode createFavoriteErr =
        ACAPI_AddOnAddOnCommunication_InstallAddOnCommandHandler (createFavoriteCommand.Pass ());
    #else
    const GSErrCode createFavoriteErr = ACAPI_Install_AddOnCommandHandler (createFavoriteCommand.Pass ());
    #endif
    if (createFavoriteErr != NoError)
        DBprnt ("Failed to register Favorite_CreateOtdCommand, error: " + GS::ValueToUniString (createFavoriteErr));

    GS::Owner<RoomBookCheckFinishConfigCommand> checkFinishConfigCommand =
        GS::NewOwned<RoomBookCheckFinishConfigCommand> ();
    #ifdef ServerMainVers_2700
    const GSErrCode checkFinishConfigErr =
        ACAPI_AddOnAddOnCommunication_InstallAddOnCommandHandler (checkFinishConfigCommand.Pass ());
    #else
    const GSErrCode checkFinishConfigErr = ACAPI_Install_AddOnCommandHandler (checkFinishConfigCommand.Pass ());
    #endif
    if (checkFinishConfigErr != NoError)
        DBprnt ("Failed to register Roombook_CheckFinishConfigurationCommand, error: " +
                GS::ValueToUniString (checkFinishConfigErr));

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

    GS::Owner<RoomBookCreateClassificationItemCommand> createClassificationItemCommand =
        GS::NewOwned<RoomBookCreateClassificationItemCommand> ();
    #ifdef ServerMainVers_2700
    const GSErrCode createClassificationItemErr =
        ACAPI_AddOnAddOnCommunication_InstallAddOnCommandHandler (createClassificationItemCommand.Pass ());
    #else
    const GSErrCode createClassificationItemErr =
        ACAPI_Install_AddOnCommandHandler (createClassificationItemCommand.Pass ());
    #endif
    if (createClassificationItemErr != NoError)
        DBprnt ("Failed to register RoomBookCreateClassificationItemCommand, error: " +
                GS::ValueToUniString (createClassificationItemErr));

    GS::Owner<RoomBookSetPropertyDescriptionCommand> setPropertyDescriptionCommand =
        GS::NewOwned<RoomBookSetPropertyDescriptionCommand> ();
    #ifdef ServerMainVers_2700
    const GSErrCode setPropertyDescriptionErr =
        ACAPI_AddOnAddOnCommunication_InstallAddOnCommandHandler (setPropertyDescriptionCommand.Pass ());
    #else
    const GSErrCode setPropertyDescriptionErr =
        ACAPI_Install_AddOnCommandHandler (setPropertyDescriptionCommand.Pass ());
    #endif
    if (setPropertyDescriptionErr != NoError)
        DBprnt ("Failed to register RoomBookSetPropertyDescriptionCommand, error: " +
                GS::ValueToUniString (setPropertyDescriptionErr));
#endif // ServerMainVers_2500
}
