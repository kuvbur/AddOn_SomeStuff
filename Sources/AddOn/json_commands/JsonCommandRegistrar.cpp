#include "ACAPinc.h"

#include "json_commands/JsonCommandRegistrar.hpp"

#include "json_commands/GetPropertyDefinitionsCommand.hpp"
#include "json_commands/HealthCommand.hpp"

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
}
