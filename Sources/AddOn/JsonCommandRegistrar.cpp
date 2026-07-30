#include "JsonCommandRegistrar.hpp"
#include "GetPropertyDefinitionsCommand.hpp"
#include "ACAPinc.h"

void RegisterJsonCommands()
{
    // Регистрация команды GetPropertyDefinitions
    {
        GS::Owner<GetPropertyDefinitionsCommand> cmd = GS::NewOwned<GetPropertyDefinitionsCommand>();
        GSErrCode err = ACAPI_Install_AddOnCommandHandler(cmd.Pass());
        if (err != NoError) {
            DBprnt("Failed to register GetPropertyDefinitionsCommand");
        } else {
            DBprnt("GetPropertyDefinitionsCommand registered successfully!");
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
