#pragma once

#include "ACAPinc.h" // ServerMainVers_* определены здесь - гвард версии должен идти после него

#if defined(ServerMainVers_2500)
    #include "json_commands/CommandBase.hpp"

class RoomBookCommand : public CommandBase {
  public:
    // -----------------------------------------------------------------------------
    // Создаёт JSON-команду запуска построения отделки.
    // -----------------------------------------------------------------------------
    RoomBookCommand ();

    // -----------------------------------------------------------------------------
    // Возвращает имя команды запуска построения отделки.
    // -----------------------------------------------------------------------------
    GS::String GetName () const override;

    // -----------------------------------------------------------------------------
    // Возвращает схему входных параметров команды.
    // -----------------------------------------------------------------------------
    GS::Optional<GS::UniString> GetInputParametersSchema () const override;

    // -----------------------------------------------------------------------------
    // Возвращает схему ответа команды.
    // -----------------------------------------------------------------------------
    GS::Optional<GS::UniString> GetResponseSchema () const override;

    // -----------------------------------------------------------------------------
    // Запускает построение отделки и возвращает длительность вызова.
    // -----------------------------------------------------------------------------
    GS::ObjectState Execute (const GS::ObjectState &parameters, GS::ProcessControl &processControl) const override;
};

class RoomBookGetFavoritesCommand : public ReadOnlyCommand {
  public:
    // Возвращает имя команды, которая перечисляет избранное Roombook.
    GS::String GetName () const override;
    // Входных параметров нет; контракт ответа намеренно не ограничен схемой.
    GS::Optional<GS::UniString> GetInputParametersSchema () const override;
    GS::Optional<GS::UniString> GetResponseSchema () const override;
    GS::ObjectState Execute (const GS::ObjectState &parameters, GS::ProcessControl &processControl) const override;
};
#endif // ServerMainVers_2500
