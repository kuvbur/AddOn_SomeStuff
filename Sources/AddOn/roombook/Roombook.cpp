//------------ kuvbur 2022 ------------
// SDK подключается раньше стандартных заголовков независимо от категорий сортировки проекта.
// clang-format off
#include "ACAPinc.h"
// clang-format on

#include "api_headers/APIEnvir.h"

#include "roombook/Roombook.hpp"

#include "CommonFunction.hpp"
#include "Helpers.hpp"
#include "Propertycache.hpp"

namespace Roombook

{

#ifndef ServerMainVers_2400
    // В AC22/AC23 сообщает, что расчёт RoomBook недоступен.
    void RoomBook_Run () { SafeWriteReport ("Function not work in AC22 and AC23", true); }
#else
    // Добавляет зоны из выделения, прошедшие фильтр доступности; если список пуст, запрашивает все зоны с тем же
    // фильтром. Возвращает false при ошибке общего запроса или отсутствии зон.
    static bool Zone_CollectTargets (GS::Array<API_Guid> &zones) {
        GSErrCode err = NoError;
        API_SelectionInfo selectionInfo = {};
        GS::Array<API_Neig> selNeigs;
        err = ACAPI_Selection_Get (&selectionInfo, &selNeigs, true);
        BMKillHandle ((GSHandle *)&selectionInfo.marquee.coords);
        if (err != APIERR_NOSEL && selectionInfo.typeID != API_SelEmpty) {
            for (const API_Neig &neig : selNeigs) {
                API_ElemTypeID elementType;
    #ifdef ServerMainVers_2600
                API_ElemType elementType_ = Neig_To_ElemID (neig.neigID);
                elementType = elementType_.typeID;
    #else
                elementType = Neig_To_ElemID (neig.neigID);
    #endif
                if (elementType == API_ZoneID) {
                    if (!ACAPI_Element_Filter (neig.guid,
                                               APIFilt_IsEditable | APIFilt_OnVisLayer | APIFilt_HasAccessRight |
                                                   APIFilt_InMyWorkspace | APIFilt_IsVisibleByRenovation))
                        continue;
                    zones.Push (neig.guid);
                }
            }
        }
        if (zones.IsEmpty ()) {
            err = ACAPI_Element_GetElemList (API_ZoneID,
                                             &zones,
                                             APIFilt_IsEditable | APIFilt_OnVisLayer | APIFilt_HasAccessRight |
                                                 APIFilt_InMyWorkspace | APIFilt_IsVisibleByRenovation);
            if (err != NoError) {
                msg_rep ("RoomBook err", "ACAPI_Element_GetElemList", err, APINULLGuid);
                return false;
            }
        }
        if (zones.IsEmpty ()) {
            msg_rep ("RoomBook err", "Zones not found or all not editable", NoError, APINULLGuid);
            return false;
        }
        return true;
    }

    // -----------------------------------------------------------------------------
    // Ищет комбинацию слоёв с именем, заканчивающимся на smstf_fin, и применяет её при наличии.
    static bool LayerCombination_ApplyFinishIfExists () {
        GS::Array<API_Attribute> layerCombinations;
    #ifdef ServerMainVers_2700
        if (ACAPI_Attribute_GetAttributesByType (API_LayerCombID, layerCombinations) != NoError)
            return false;
    #else
        if (ParamHelpers::ACAPI_Attribute_GetAttributesByType (API_LayerCombID, layerCombinations) != NoError)
            return false;
    #endif

        for (const API_Attribute &attr : layerCombinations) {
            GS::UniString name (attr.header.name);
            name.SetToLowerCase ();
            if (name.EndsWith ("smstf_fin")) {
                API_AttributeIndex targetComb = attr.header.index;
    #ifdef ServerMainVers_2700
                if (targetComb.IsPositive ()) {
                    GSErrCode err = ACAPI_Navigator_ChangeCurrLayerComb (&targetComb);
                    return (err == NoError);
                }
    #else
                if (targetComb != 0) {
                    GSErrCode err = ACAPI_Environment (APIEnv_ChangeCurrLayerCombID, &targetComb);
                    return (err == NoError);
                }
    #endif
            }
        }
        return false;
    }

    // Собирает данные зон и конструкций, рассчитывает отделку и подготавливает результаты по материалам.
    // Проверяет классификации и доступность существующей отделки, затем вызывает отрисовку и запись связей; при ошибках
    // или отмене выходит.
    void RoomBook_Run (GS::Array<RoomBookFinishResult> *results) {
        if (results != nullptr)
            results->Clear ();
        GS::UniString funcname ("RoomBook");
        GS::Int32 nPhase = 1;
        // Окно прогресса помогает видеть, на каком этапе выполняется расчёт отделки.
        ProcessWindowGuard pwGuard (funcname, nPhase);

        LayerCombination_ApplyFinishIfExists ();

        GS::Array<API_Guid> zones;
        if (!Zone_CollectTargets (zones))
            return;

        funcname = GS::UniString::Printf ("Collect info from %d room(s)", zones.GetSize ());
        // Сбор данных: основания, контакты поверхностей и параметры всех обрабатываемых комнат.
        // Контекст и его кэши сохраняются до завершения запуска.
        RoomProcessingContext context;
        if (!RoomProcessingContext_Prepare (context, zones))
            return;
        // После сбора связей временные GUID зон очищаются перед обработкой базовых элементов.
        if (!RoomProcessingContext_BuildElementReadIndex (zones, context, funcname, nPhase))
            return;
        if (!RoomProcessingContext_ProcessElementsForRoomData (context, funcname, nPhase) ||
            !RoomProcessingContext_PreparePartitions (context))
            return;
        // Необходимые для чтения параметры и свойства
        ReadParamsForRoomBook readParams{};
        RoomProcessingContext_ReadElementParameters (context, readParams);
        funcname = GS::UniString::Printf ("Read data from %d elements(s)", context.paramToRead.GetSize ());
        if (AdvanceProcessPhase (funcname, nPhase))
            return;
        // Читаем свойства всех элементов
        ParamDictCompositeElement paramCompositeToRead;
        ListData::LibElements paramListDataToRead;
        ParamHelpers::ElementsRead (context.paramToRead, paramCompositeToRead, paramListDataToRead, true, false);
        // Словарь избранного нужен для выбора подходящего шаблона отделки по материалу и типу поверхности.
        Favorite_GetDict (context.favdict);
        // Ищём уже существующие элементы отделки и определяем, к каким базовым элементам они привязаны.
        // Это нужно, чтобы не создавать дубли и правильно обновлять существующие элементы.
        bool has_base_element = false;
        UnicGuid reserv_elements; // Словарь незарезервированных или скрытых элементов
        context.exsistot_byzone = Otd_GetOtd_ByZone (
            zones, context.finclassguids, context.finclass, has_base_element, reserv_elements, &context.surfaceCache);
        // Подготовка отделки: геометрия, откосы, материалы и избранное. Native-элементы ещё не создаются.
        if (!OtdRooms_ProcessFinishes (context, readParams, paramCompositeToRead, has_base_element, funcname, nPhase))
            return;
        // Отбор зон для проверки классов не зависит от наличия свойств для записи площадей.
        // Один обход сохраняет порядок отмены, отбора и расчёта каждой комнаты.
        zones.Clear ();
        context.paramDict_favorite.Clear ();
        MaterialSummary summary;
        funcname = GS::UniString::Printf ("Calculate material for %d room(s)", context.roomsinfo.GetSize ());
        for (OtdRooms::PairIterator cIt = context.roomsinfo.EnumeratePairs (); cIt != NULL; ++cIt) {
    #ifdef ServerMainVers_2800
            OtdRoom &otd = cIt->value;
    #else
            OtdRoom &otd = *cIt->value;
    #endif
            if (AdvanceProcessPhase (funcname, nPhase))
                return;
            if (otd.isValid && (otd.create_ceil_elements || otd.create_floor_elements || otd.create_wall_elements ||
                                otd.create_column_elements || otd.create_reveal_elements)) {
                zones.Push (otd.zone_guid);
            }
            OtdData_AddRoomToMaterialSummary (otd, context.paramToRead, summary);
        }
        OtdData_WriteRoomMaterialData (context.roomsinfo, context.paramToRead, summary);
        auto &paramToWrite = summary.paramToWrite;
        context.paramToRead.Clear ();
        // Проверка существования классов и свойств
        if (!zones.IsEmpty ()) {
            if (!Roombook_CheckFinishConfiguration (context.finclass, context.finclassguids))
                return;
        }
        // Создание и обновление отделки, затем связи с основаниями/зонами и запись подготовленных свойств.
        OtdRooms_Draw (context.exsistot_byzone,
                       context.deletelist,
                       context.roomsinfo,
                       context.storyLevels,
                       context.finclass,
                       reserv_elements,
                       paramToWrite,
                       context.elementTypes,
                       context.elementTypeCount,
                       nPhase,
                       results);
    }

#endif
} // namespace Roombook
