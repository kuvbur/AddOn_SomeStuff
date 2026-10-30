#include "ACAPinc.h" // ServerMainVers_* определены здесь - гвард версии должен идти после него

#if defined(ServerMainVers_2500)

    #include "ACAPinc.h"

    #include "json_commands/RoomBookCommand.hpp"
    #include "roombook/Roombook.hpp"
    #include "Propertycache.hpp"
    #include "CommonFunction.hpp"

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

// -----------------------------------------------------------------------------
// Создаёт JSON-команду создания избранного Roombook.
// -----------------------------------------------------------------------------
RoomBookCreateFavoriteCommand::RoomBookCreateFavoriteCommand () : CommandBase (CommonSchema::NotUsed) {}

// -----------------------------------------------------------------------------
// Создаёт JSON-команду проверки конфигурации отделки Roombook.
// -----------------------------------------------------------------------------
RoomBookCheckFinishConfigCommand::RoomBookCheckFinishConfigCommand () : ReadOnlyCommand () {}

// -----------------------------------------------------------------------------
// Указывает, что команда создания избранного принимает имя, тип и папку.
// -----------------------------------------------------------------------------
GS::Optional<GS::UniString> RoomBookCreateFavoriteCommand::GetInputParametersSchema () const { return GS::NoValue; }

// -----------------------------------------------------------------------------
// Указывает, что команда создания избранного возвращает свободный ответ.
// -----------------------------------------------------------------------------
GS::Optional<GS::UniString> RoomBookCreateFavoriteCommand::GetResponseSchema () const { return GS::NoValue; }

GS::String RoomBookCreateFavoriteCommand::GetName () const { return "Favorite_CreateOtd"; }

// -----------------------------------------------------------------------------
// Создаёт избранное Roombook с заданными именем, типом и папкой из настроек
// инструмента типа отделки. Команда ничего не доказывает сверх результата
// Favorite_CreateOtd, поэтому сообщает только факт успеха создания.
// -----------------------------------------------------------------------------
GS::ObjectState RoomBookCreateFavoriteCommand::Execute (const GS::ObjectState &parameters,
                                                        GS::ProcessControl & /*processControl*/) const {
    Int32 type = 0;
    GS::UniString name;
    GS::UniString folder;
    const bool hasType = parameters.Get ("type", type);
    const bool hasName = parameters.Get ("name", name);
    const bool hasFolder = parameters.Get ("folder", folder);
    if (!hasType || !hasName || !hasFolder)
        return CreateErrorResponse (APIERR_BADPARS, "type, name and folder are required");
    if (name.IsEmpty ())
        return CreateErrorResponse (APIERR_BADPARS, "name must not be empty");

    const bool created = Roombook::Favorite_CreateOtd (static_cast<Roombook::TypeOtd> (type), name, folder);
    if (!created)
        return CreateErrorResponse (APIERR_GENERAL, "Favorite_CreateOtd failed");
    return CreateSuccessResponse ();
}

// -----------------------------------------------------------------------------
// Указывает, что команда проверки конфигурации не принимает параметры.
// -----------------------------------------------------------------------------
GS::Optional<GS::UniString> RoomBookCheckFinishConfigCommand::GetInputParametersSchema () const { return GS::NoValue; }

// -----------------------------------------------------------------------------
// Указывает, что команда проверки конфигурации возвращает свободный ответ.
// -----------------------------------------------------------------------------
GS::Optional<GS::UniString> RoomBookCheckFinishConfigCommand::GetResponseSchema () const { return GS::NoValue; }

GS::String RoomBookCheckFinishConfigCommand::GetName () const { return "Roombook_CheckFinishConfiguration"; }

// -----------------------------------------------------------------------------
// Проверяет классы отделки и свойства Sync_GUID и возвращает признак корректности
// со списком ошибок (по одной на строку, как их собирает Roombook_CheckFinishConfiguration).
// -----------------------------------------------------------------------------
GS::ObjectState RoomBookCheckFinishConfigCommand::Execute (const GS::ObjectState & /*parameters*/,
                                                           GS::ProcessControl & /*processControl*/) const {
    GS::UniString errors;
    const bool ok = Roombook::Roombook_CheckFinishConfiguration (errors);
    GS::ObjectState response;
    response.Add ("succeeded", ok);
    response.Add ("errors", errors);
    return response;
}

// -----------------------------------------------------------------------------
// Создаёт JSON-команду создания элемента классификации.
// -----------------------------------------------------------------------------
RoomBookCreateClassificationItemCommand::RoomBookCreateClassificationItemCommand ()
    : CommandBase (CommonSchema::NotUsed) {}

// -----------------------------------------------------------------------------
// Возвращает имя команды создания элемента классификации.
// -----------------------------------------------------------------------------
GS::String RoomBookCreateClassificationItemCommand::GetName () const { return "CreateClassificationItem"; }

// -----------------------------------------------------------------------------
// Указывает, что команда принимает систему, родителя и атрибуты элемента.
// -----------------------------------------------------------------------------
GS::Optional<GS::UniString> RoomBookCreateClassificationItemCommand::GetInputParametersSchema () const {
    return R"({
    "type": "object",
    "properties": {
        "systemGuid": {
            "type": "string",
            "description": "GUID of the classification system."
        },
        "parentItemGuid": {
            "type": "string",
            "description": "GUID of the parent classification item; empty or omitted creates a root item."
        },
        "itemId": {
            "type": "string",
            "description": "Id of the new classification item."
        },
        "itemName": {
            "type": "string",
            "description": "Name of the new classification item."
        },
        "itemDescription": {
            "type": "string",
            "description": "Description of the new classification item."
        }
    },
    "additionalProperties": false,
    "required": ["systemGuid", "itemId", "itemName", "itemDescription"]
})";
}

// -----------------------------------------------------------------------------
// Указывает, что команда возвращает GUID созданного элемента.
// -----------------------------------------------------------------------------
GS::Optional<GS::UniString> RoomBookCreateClassificationItemCommand::GetResponseSchema () const { return GS::NoValue; }

// -----------------------------------------------------------------------------
// Создаёт элемент классификации через ACAPI_Classification_CreateClassificationItem.
// Команда ничего не доказывает сверх результата ACAPI-вызова, поэтому сообщает
// только GUID созданного элемента или ошибку вызова.
// -----------------------------------------------------------------------------
GS::ObjectState RoomBookCreateClassificationItemCommand::Execute (const GS::ObjectState &parameters,
                                                                  GS::ProcessControl & /*processControl*/) const {
    GS::UniString systemGuidStr;
    GS::UniString parentGuidStr;
    GS::UniString itemId;
    GS::UniString itemName;
    GS::UniString itemDescription;
    const bool hasSystem = parameters.Get ("systemGuid", systemGuidStr);
    const bool hasParent = parameters.Get ("parentItemGuid", parentGuidStr);
    const bool hasId = parameters.Get ("itemId", itemId);
    const bool hasName = parameters.Get ("itemName", itemName);
    const bool hasDescription = parameters.Get ("itemDescription", itemDescription);
    if (!hasSystem || !hasId || !hasName || !hasDescription)
        return CreateErrorResponse (APIERR_BADPARS, "systemGuid, itemId, itemName and itemDescription are required");

    // Пустой guid строки или нечитаемый формат APIGuidFromString превращает в APINULLGuid.
    API_Guid systemGuid = APIGuidFromString (systemGuidStr.ToCStr (0, MaxUSize, GChCode));
    API_Guid parentGuid = hasParent ? APIGuidFromString (parentGuidStr.ToCStr (0, MaxUSize, GChCode)) : APINULLGuid;
    if (systemGuid == APINULLGuid)
        return CreateErrorResponse (APIERR_BADPARS, "systemGuid is invalid");

    API_ClassificationItem item = {};
    item.guid = APINULLGuid;
    item.id = itemId;
    item.name = itemName;
    item.description = itemDescription;

    // Родитель или следующий элемент не заданы: APINULLGuid означает корневой/последний элемент.
    GSErrCode err = ACAPI_Classification_CreateClassificationItem (item, systemGuid, parentGuid, APINULLGuid);
    if (err != NoError)
        return CreateErrorResponse (err, "ACAPI_Classification_CreateClassificationItem failed");
    GS::ObjectState response;
    response.Add ("itemGuid", APIGuidToString (item.guid));
    return response;
}

#endif // ServerMainVers_2500
