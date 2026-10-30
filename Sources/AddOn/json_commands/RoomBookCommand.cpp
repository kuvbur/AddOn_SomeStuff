#include "ACAPinc.h" // ServerMainVers_* определены здесь - гвард версии должен идти после него

#if defined(ServerMainVers_2500)

    #include "ACAPinc.h"

    #include "json_commands/RoomBookCommand.hpp"
    #include "roombook/Roombook.hpp"

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
// Запускает построение отделки и возвращает статус завершения вызова.
// -----------------------------------------------------------------------------
GS::ObjectState RoomBookCommand::Execute (const GS::ObjectState &parameters,
                                          GS::ProcessControl & /*processControl*/) const {
    (void)parameters;
    GS::Array<Roombook::RoomBookFinishResult> results;
    Roombook::RoomBook_Run (&results);
    GS::ObjectState response;
    response.Add ("status", "returned");
    const auto addRoom = response.AddList<GS::ObjectState> ("rooms");
    for (UIndex i = 0; i < results.GetSize (); ++i) {
        bool seenRoom = false;
        for (UIndex j = 0; j < i; ++j) {
            if (results[j].zone_guid == results[i].zone_guid) {
                seenRoom = true;
                break;
            }
        }
        if (seenRoom)
            continue;
        GS::ObjectState room;
        room.Add ("zone_guid", APIGuidToString (results[i].zone_guid));
        const auto addGroup = room.AddList<GS::ObjectState> ("types");
        for (UIndex j = i; j < results.GetSize (); ++j) {
            if (results[j].zone_guid != results[i].zone_guid || results[j].draw.otd_guid == APINULLGuid)
                continue;
            bool seenType = false;
            for (UIndex k = i; k < j; ++k) {
                if (results[k].zone_guid == results[i].zone_guid && results[k].draw.otd_guid != APINULLGuid &&
                    results[k].draw.type == results[j].draw.type) {
                    seenType = true;
                    break;
                }
            }
            if (seenType)
                continue;
            GS::ObjectState group;
            group.Add ("TypeOtd", static_cast<Int32> (results[j].draw.type));
            const auto addElement = group.AddList<GS::ObjectState> ("elements");
            for (UIndex k = j; k < results.GetSize (); ++k) {
                const auto &result = results[k];
                if (result.zone_guid != results[i].zone_guid || result.draw.otd_guid == APINULLGuid ||
                    result.draw.type != results[j].draw.type)
                    continue;
                GS::ObjectState element;
                element.Add ("otd_guid", APIGuidToString (result.draw.otd_guid));
                element.Add ("base_guid", APIGuidToString (result.draw.base_guid));
                element.Add ("write_succeeded", result.draw.write_succeeded);
                element.Add ("was_created", result.draw.was_created);
                element.Add ("length", result.length);
                element.Add ("length_available", result.has_length);
                element.Add ("area", result.area);
                element.Add ("area_available", result.has_area);
                element.Add ("material", static_cast<Int32> (result.draw.material.material));
                element.Add ("bmaterial", static_cast<Int32> (result.draw.material.bmaterial));
                element.Add ("structtype", static_cast<Int32> (result.draw.material.structtype));
                addElement (element);
            }
            addGroup (group);
        }
        addRoom (room);
    }
    return response;
}

GS::String RoomBookGetFavoritesCommand::GetName () const { return "FavoritesList"; }

GS::Optional<GS::UniString> RoomBookGetFavoritesCommand::GetInputParametersSchema () const { return GS::NoValue; }

GS::Optional<GS::UniString> RoomBookGetFavoritesCommand::GetResponseSchema () const { return GS::NoValue; }

// -----------------------------------------------------------------------------
// Собирает текущий словарь избранного Roombook и возвращает имя и тип каждого элемента.
// -----------------------------------------------------------------------------
GS::ObjectState RoomBookGetFavoritesCommand::Execute (const GS::ObjectState & /*parameters*/,
                                                      GS::ProcessControl & /*processControl*/) const {
    Roombook::MatarialToFavoriteDict favoritesByName;
    Roombook::Favorite_GetDict (favoritesByName);

    GS::ObjectState response;
    const auto addFavorite = response.AddList<GS::ObjectState> ("favorites");
    for (Roombook::MatarialToFavoriteDict::ConstPairIterator it = favoritesByName.EnumeratePairs (); it != nullptr;
         ++it) {
        GS::ObjectState favorite;
    #ifdef ServerMainVers_2800
        favorite.Add ("name", it->key);
        favorite.Add ("type", static_cast<Int32> (it->value.type));
    #else
        favorite.Add ("name", *it->key);
        favorite.Add ("type", static_cast<Int32> (it->value->type));
    #endif
        addFavorite (favorite);
    }
    return response;
}

#endif // ServerMainVers_2500
