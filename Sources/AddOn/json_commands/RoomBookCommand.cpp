#include <chrono>

#include "ACAPinc.h"

#include "json_commands/RoomBookCommand.hpp"

#include "Roombook.hpp"

#if defined(ServerMainVers_2500)

// -----------------------------------------------------------------------------
// Создаёт JSON-команду запуска построения отделки.
// -----------------------------------------------------------------------------
RoomBookCommand::RoomBookCommand () : CommandBase (CommonSchema::NotUsed) {}

// -----------------------------------------------------------------------------
// Возвращает имя команды запуска построения отделки.
// -----------------------------------------------------------------------------
GS::String RoomBookCommand::GetName () const { return "RoomBook"; }

// -----------------------------------------------------------------------------
// Указывает, что команда построения отделки не принимает параметры.
// -----------------------------------------------------------------------------
GS::Optional<GS::UniString> RoomBookCommand::GetInputParametersSchema () const { return GS::NoValue; }

// -----------------------------------------------------------------------------
// Указывает, что команда построения отделки возвращает свободный ответ.
// -----------------------------------------------------------------------------
GS::Optional<GS::UniString> RoomBookCommand::GetResponseSchema () const { return GS::NoValue; }

// -----------------------------------------------------------------------------
// Запускает построение отделки и возвращает длительность вызова.
// -----------------------------------------------------------------------------
GS::ObjectState RoomBookCommand::Execute (const GS::ObjectState & /*parameters*/,
                                          GS::ProcessControl & /*processControl*/) const {
    const auto start = std::chrono::steady_clock::now ();
    Roombook::RoomBook ();
    const auto finish = std::chrono::steady_clock::now ();

    const double elapsedSeconds = std::chrono::duration<double> (finish - start).count ();
    GS::ObjectState response;
    response.Add ("status", "returned");
    response.Add ("elapsedSeconds", elapsedSeconds);
    return response;
}

#endif // defined (ServerMainVers_2500)
