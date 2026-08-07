#include "ACAPinc.h"

#include "json_commands/JsonCommandRegistrar.hpp"

#include "json_commands/GetPropertiesListCommand.hpp"
#include "json_commands/GetPropertyValueCommand.hpp"
#include "json_commands/GetPropertyDefinitionsCommand.hpp"
#include "json_commands/GetSelectionInfoCommand.hpp"
#include "json_commands/GetSettingsCommand.hpp"
#include "json_commands/HealthCommand.hpp"
#include "json_commands/MonAllCommand.hpp"
#include "json_commands/ParsePropertyForElementCommand.hpp"
#include "json_commands/SetSettingsCommand.hpp"
#include "json_commands/SyncAllCommand.hpp"
#include "json_commands/GetClassificationCommand.hpp"
#include "json_commands/SetClassificationCommand.hpp"

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
        GS::Owner<GetSettingsCommand> cmd = GS::NewOwned<GetSettingsCommand> ();
        GSErrCode err = ACAPI_Install_AddOnCommandHandler (cmd.Pass ());
        if (err != NoError) {
            DBprnt ("Failed to register GetSettingsCommand, error: " + GS::ValueToUniString (err));
        } else {
            DBprnt ("GetSettingsCommand registered successfully!");
        }
    }

    // Регистрация команды SetSettings
    {
        GS::Owner<SetSettingsCommand> cmd = GS::NewOwned<SetSettingsCommand> ();
        GSErrCode err = ACAPI_Install_AddOnCommandHandler (cmd.Pass ());
        if (err != NoError) {
            DBprnt ("Failed to register SetSettingsCommand, error: " + GS::ValueToUniString (err));
        } else {
            DBprnt ("SetSettingsCommand registered successfully!");
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
        GS::Owner<SyncAllCommand> cmd = GS::NewOwned<SyncAllCommand> ();
        GSErrCode err = ACAPI_Install_AddOnCommandHandler (cmd.Pass ());
        if (err != NoError) {
            DBprnt ("Failed to register SyncAllCommand, error: " + GS::ValueToUniString (err));
        } else {
            DBprnt ("SyncAllCommand registered successfully!");
        }
    }

    // Регистрация команды MonAll
    {
        GS::Owner<MonAllCommand> cmd = GS::NewOwned<MonAllCommand> ();
        GSErrCode err = ACAPI_Install_AddOnCommandHandler (cmd.Pass ());
        if (err != NoError) {
            DBprnt ("Failed to register MonAllCommand, error: " + GS::ValueToUniString (err));
        } else {
            DBprnt ("MonAllCommand registered successfully!");
        }
    }

    // Регистрация команды ParsePropertyForElement
    {
        GS::Owner<ParsePropertyForElementCommand> cmd = GS::NewOwned<ParsePropertyForElementCommand> ();
        GSErrCode err = ACAPI_Install_AddOnCommandHandler (cmd.Pass ());
        if (err != NoError) {
            DBprnt ("Failed to register ParsePropertyForElementCommand, error: " + GS::ValueToUniString (err));
        } else {
            DBprnt ("ParsePropertyForElementCommand registered successfully!");
        }
    }

    // Регистрация команды GetSelectionInfo
    {
        GS::Owner<GetSelectionInfoCommand> cmd = GS::NewOwned<GetSelectionInfoCommand> ();
        GSErrCode err = ACAPI_Install_AddOnCommandHandler (cmd.Pass ());
        if (err != NoError) {
            DBprnt ("Failed to register GetSelectionInfoCommand, error: " + GS::ValueToUniString (err));
        } else {
            DBprnt ("GetSelectionInfoCommand registered successfully!");
        }
    }

    // Регистрация команды GetPropertiesList
    {
        GS::Owner<GetPropertiesListCommand> cmd = GS::NewOwned<GetPropertiesListCommand> ();
        GSErrCode err = ACAPI_Install_AddOnCommandHandler (cmd.Pass ());
        if (err != NoError) {
            DBprnt ("Failed to register GetPropertiesListCommand, error: " + GS::ValueToUniString (err));
        } else {
            DBprnt ("GetPropertiesListCommand registered successfully!");
        }
    }

    // Регистрация команды GetPropertyValue
    {
        GS::Owner<GetPropertyValueCommand> cmd = GS::NewOwned<GetPropertyValueCommand> ();
        GSErrCode err = ACAPI_Install_AddOnCommandHandler (cmd.Pass ());
        if (err != NoError) {
            DBprnt ("Failed to register GetPropertyValueCommand, error: " + GS::ValueToUniString (err));
        } else {
            DBprnt ("GetPropertyValueCommand registered successfully!");
        }
    }

    // Регистрация команды GetClassification
    {
        GS::Owner<GetClassificationCommand> cmd = GS::NewOwned<GetClassificationCommand> ();
        GSErrCode err = ACAPI_Install_AddOnCommandHandler (cmd.Pass ());
        if (err != NoError) {
            DBprnt ("Failed to register GetClassificationCommand, error: " + GS::ValueToUniString (err));
        } else {
            DBprnt ("GetClassificationCommand registered successfully!");
        }
    }

    // Регистрация команды SetClassification
    {
        GS::Owner<SetClassificationCommand> cmd = GS::NewOwned<SetClassificationCommand> ();
        GSErrCode err = ACAPI_Install_AddOnCommandHandler (cmd.Pass ());
        if (err != NoError) {
            DBprnt ("Failed to register SetClassificationCommand, error: " + GS::ValueToUniString (err));
        } else {
            DBprnt ("SetClassificationCommand registered successfully!");
        }
    }
}
