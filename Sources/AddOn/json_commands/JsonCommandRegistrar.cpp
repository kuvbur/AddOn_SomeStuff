#include "ACAPinc.h"

#include "json_commands/JsonCommandRegistrar.hpp"

#include "json_commands/GetPropertyDefinitionsCommand.hpp"
#include "json_commands/HealthCommand.hpp"
#include "json_commands/GetSettingsCommand.hpp"
#include "json_commands/SetSettingsCommand.hpp"
#include "json_commands/SyncAllCommand.hpp"
#include "json_commands/MonAllCommand.hpp"

void RegisterJsonCommands () {
    // Регистрация команды GetPropertyDefinitions
    {
        GS::Owner<GetPropertyDefinitionsCommand> cmd = GS::NewOwned<GetPropertyDefinitionsCommand> ();
        GSErrCode err = ACAPI_Install_AddOnCommandHandler (cmd.Pass ());
        if (err != NoError) {
            DBprnt ("Failed to register GetPropertyDefinitionsCommand, error: " + GS::ValueToUniString (err));
        } else {
            DBprnt ("GetPropertyDefinitionsCommand registered successfully!");
        }
    }

    // Регистрация команды Health
    {
        GS::Owner<HealthCommand> cmd = GS::NewOwned<HealthCommand> ();
        GSErrCode err = ACAPI_Install_AddOnCommandHandler (cmd.Pass ());
        if (err != NoError) {
            DBprnt ("Failed to register HealthCommand, error: " + GS::ValueToUniString (err));
        } else {
            DBprnt ("HealthCommand registered successfully!");
        }
    }

    // Сюда добавлять регистрацию новых команд
    // Пример:
    // {
    //     GS::Owner<AnotherCommand> cmd = GS::NewOwned<AnotherCommand>();
    //     GSErrCode err = ACAPI_Install_AddOnCommandHandler(cmd.Pass());
    //     ...
    // }
    
    // Регистрация команды GetSettings
    {
        GS::Owner<GetSettingsCommand> cmd = GS::NewOwned<GetSettingsCommand>();
        GSErrCode err = ACAPI_Install_AddOnCommandHandler(cmd.Pass());
        if (err != NoError) {
            DBprnt("Failed to register GetSettingsCommand, error: " + GS::ValueToUniString(err));
        } else {
            DBprnt("GetSettingsCommand registered successfully!");
        }
    }
    
    // Регистрация команды SetSettings
    {
        GS::Owner<SetSettingsCommand> cmd = GS::NewOwned<SetSettingsCommand>();
        GSErrCode err = ACAPI_Install_AddOnCommandHandler(cmd.Pass());
        if (err != NoError) {
            DBprnt("Failed to register SetSettingsCommand, error: " + GS::ValueToUniString(err));
        } else {
            DBprnt("SetSettingsCommand registered successfully!");
        }
    }
    
    // SyncAllCommand временно отключена (требует исправления зависимостей)
    // {
    //     GS::Owner<SyncAllCommand> cmd = GS::NewOwned<SyncAllCommand>();
    //     GSErrCode err = ACAPI_Install_AddOnCommandHandler(cmd.Pass());
    //     ...
    // }
    
    // Регистрация команды SyncAll
    {
        GS::Owner<SyncAllCommand> cmd = GS::NewOwned<SyncAllCommand>();
        GSErrCode err = ACAPI_Install_AddOnCommandHandler(cmd.Pass());
        if (err != NoError) {
            DBprnt("Failed to register SyncAllCommand, error: " + GS::ValueToUniString(err));
        } else {
            DBprnt("SyncAllCommand registered successfully!");
        }
    }
    
    // Регистрация команды MonAll
    {
        GS::Owner<MonAllCommand> cmd = GS::NewOwned<MonAllCommand>();
        GSErrCode err = ACAPI_Install_AddOnCommandHandler(cmd.Pass());
        if (err != NoError) {
            DBprnt("Failed to register MonAllCommand, error: " + GS::ValueToUniString(err));
        } else {
            DBprnt("MonAllCommand registered successfully!");
        }
    }
}
