#if !defined(JSONCOMMANDREGISTRAR_HPP)
#define JSONCOMMANDREGISTRAR_HPP

// -----------------------------------------------------------------------------
// Регистрирует все JSON команды аддона в ArchiCAD.
//
// Вызов выполняется при инициализации аддона, чтобы клиент мог
// впоследствии вызывать команды через JSON API.
// -----------------------------------------------------------------------------
void RegisterJsonCommands();

#endif // JSONCOMMANDREGISTRAR_HPP
