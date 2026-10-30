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

class RoomBookCreateFavoriteCommand : public CommandBase {
  public:
    // -----------------------------------------------------------------------------
    // Создаёт JSON-команду создания избранного Roombook.
    // -----------------------------------------------------------------------------
    RoomBookCreateFavoriteCommand ();
    // Возвращает имя команды, которая создаёт избранное Roombook с заданными именем, типом и папкой.
    GS::String GetName () const override;
    // Возвращает схему входных параметров команды.
    GS::Optional<GS::UniString> GetInputParametersSchema () const override;
    GS::Optional<GS::UniString> GetResponseSchema () const override;
    // Создаёт избранное по настройкам инструмента типа отделки; возвращает успех создания.
    GS::ObjectState Execute (const GS::ObjectState &parameters, GS::ProcessControl &processControl) const override;
};

class RoomBookCheckFinishConfigCommand : public ReadOnlyCommand {
  public:
    // -----------------------------------------------------------------------------
    // Создаёт JSON-команду проверки конфигурации отделки Roombook.
    // -----------------------------------------------------------------------------
    RoomBookCheckFinishConfigCommand ();
    // Возвращает имя команды, которая проверяет конфигурацию отделки Roombook.
    GS::String GetName () const override;
    // Входных параметров нет; контракт ответа намеренно не ограничен схемой.
    GS::Optional<GS::UniString> GetInputParametersSchema () const override;
    GS::Optional<GS::UniString> GetResponseSchema () const override;
    // Проверяет классы отделки и свойства Sync_GUID; возвращает признак и список ошибок.
    GS::ObjectState Execute (const GS::ObjectState &parameters, GS::ProcessControl &processControl) const override;
};

class RoomBookCreateClassificationItemCommand : public CommandBase {
  public:
    // -----------------------------------------------------------------------------
    // Создаёт JSON-команду создания элемента классификации.
    // -----------------------------------------------------------------------------
    RoomBookCreateClassificationItemCommand ();
    // Возвращает имя команды, которая создаёт элемент классификации в системе.
    GS::String GetName () const override;
    // Возвращает схему входных параметров команды.
    GS::Optional<GS::UniString> GetInputParametersSchema () const override;
    // Возвращает схему ответа команды.
    GS::Optional<GS::UniString> GetResponseSchema () const override;
    // Создаёт элемент классификации в системе под родительской ветвью; возвращает GUID элемента.
    GS::ObjectState Execute (const GS::ObjectState &parameters, GS::ProcessControl &processControl) const override;
};

class RoomBookSetPropertyDescriptionCommand : public CommandBase {
  public:
    // -----------------------------------------------------------------------------
    // Создаёт JSON-команду изменения описания свойства.
    // -----------------------------------------------------------------------------
    RoomBookSetPropertyDescriptionCommand ();
    // Возвращает имя команды, которая меняет описание определения свойства.
    GS::String GetName () const override;
    // Возвращает схему входных параметров команды.
    GS::Optional<GS::UniString> GetInputParametersSchema () const override;
    // Возвращает схему ответа команды.
    GS::Optional<GS::UniString> GetResponseSchema () const override;
    // Меняет описание определения свойства; свойство ищется в кэше PropertyCache
    // по GUID или полному имени, перед записью определение перечитывается по GUID.
    GS::ObjectState Execute (const GS::ObjectState &parameters, GS::ProcessControl &processControl) const override;
};
#endif // ServerMainVers_2500
