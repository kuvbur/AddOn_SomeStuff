//------------ kuvbur 2022 ------------
#include "ACAPinc.h"
#include "alphanum.h"
#include "APIEnvir.h"
#include "CommonFunction.hpp"
#include "Helpers.hpp"
#include "Roombook.hpp"
#include "Sync.hpp"

namespace AutoFunc

{
#if defined(AC_22) || defined(AC_23)
void RoomBook ()
{
    ACAPI_WriteReport ("Function not work in AC22 and AC23", true);
}
#else
RoomEdges* reducededges = nullptr; // Указатель для обработки полигонов зоны
ClassOtd cls;
GS::Int32 nPhase = 1;

// -----------------------------------------------------------------------------
// Запись в зону информации об отделке
// -----------------------------------------------------------------------------
void RoomBook ()
{
    clock_t start, finish;
    double  duration;
    start = clock ();
    GS::UniString funcname ("RoomBook");
    nPhase = 1;
    #if defined(AC_27) || defined(AC_28)
    ACAPI_ProcessWindow_InitProcessWindow (&funcname, &nPhase);
    #else
    ACAPI_Interface (APIIo_InitProcessWindowID, &funcname, &nPhase);
    #endif

    GS::Array<API_Guid> zones;
    GSErrCode            err;
    API_SelectionInfo    selectionInfo;
    GS::Array<API_Neig>  selNeigs;
    err = ACAPI_Selection_Get (&selectionInfo, &selNeigs, true);
    BMKillHandle ((GSHandle*) &selectionInfo.marquee.coords);
    if (err != APIERR_NOSEL && selectionInfo.typeID != API_SelEmpty) {
        for (const API_Neig& neig : selNeigs) {
            API_ElemTypeID elementType;
            #if defined(AC_27) || defined(AC_28) || defined(AC_26)
            API_ElemType elementType_ = Neig_To_ElemID (neig.neigID);
            elementType = elementType_.typeID;
            #else
            elementType = Neig_To_ElemID (neig.neigID);
            #endif
            if (elementType == API_ZoneID) {
                if (!ACAPI_Element_Filter (neig.guid, APIFilt_InMyWorkspace)) continue;
                if (!ACAPI_Element_Filter (neig.guid, APIFilt_HasAccessRight)) continue;
                if (!ACAPI_Element_Filter (neig.guid, APIFilt_IsEditable)) continue;
                zones.Push (neig.guid);
            }
        }
    }
    if (zones.IsEmpty ()) {
        err = ACAPI_Element_GetElemList (API_ZoneID, &zones, APIFilt_IsEditable | APIFilt_OnVisLayer | APIFilt_HasAccessRight | APIFilt_InMyWorkspace | APIFilt_IsVisibleByRenovation);
        if (err != NoError) {
            msg_rep ("RoomBook err", "ACAPI_Element_GetElemList", err, APINULLGuid);
            return;
        }
    }
    if (zones.IsEmpty ()) {
        msg_rep ("RoomBook err", "Zones not found or all not editable", NoError, APINULLGuid);
        return;
    }

    funcname = GS::UniString::Printf ("Collect info from %d room(s)", zones.GetSize ());
    // Подготовка параметров
    ParamDictValue propertyParams; // Словарь общих параметров
    ParamHelpers::AllPropertyDefinitionToParamDict (propertyParams);
    ClassificationFunc::SystemDict systemdict; // Словарь все классов и систем
    ClassificationFunc::ClassificationDict finclass; // Словарь классов для отделочных стен
    UnicGuid finclassguids;
    ClassificationFunc::GetAllClassification (systemdict);
    Class_FindFinClass (systemdict, finclass, finclassguids);
    Stories storyLevels = GetStories (); // Уровни этажей в проекте
    // Поиск перекрытий в зонах
    GS::HashTable<API_Guid, GS::Array<API_Guid>> slabsinzone;
    Floor_FindAll (slabsinzone, finclassguids, zones);
    // Чтение данных о зоне, создание словаря с элементами для чтения
    OtdRooms roomsinfo; // Информация о всех зонах
    UnicElementByType elementToRead; // Список всех элементов в зоне
    for (API_Guid zoneGuid : zones) {
        nPhase += 1;
        #if defined(AC_27) || defined(AC_28)
        ACAPI_ProcessWindow_SetNextProcessPhase (&funcname, &nPhase);
        if (ACAPI_ProcessWindow_IsProcessCanceled ()) return;
        #else
        ACAPI_Interface (APIIo_SetNextProcessPhaseID, &funcname, &nPhase);
        if (ACAPI_Interface (APIIo_IsProcessCanceledID, nullptr, nullptr)) return;
        #endif
        OtdRoom roominfo;
        if (CollectRoomInfo (storyLevels, zoneGuid, roominfo, elementToRead, slabsinzone)) {
            roomsinfo.Add (zoneGuid, roominfo);
        }
    }

    UnicGUIDByType guidselementToRead; // Словарь элементов по типам для чтения свойств
    guidselementToRead.Add (API_ZoneID, zones);
    GS::Array<API_ElemTypeID> typeinzone;
    typeinzone.Push (API_WindowID);
    typeinzone.Push (API_DoorID);
    typeinzone.Push (API_ColumnID);
    typeinzone.Push (API_WallID);
    typeinzone.Push (API_SlabID);
    ClearZoneGUID (elementToRead, typeinzone);
    GS::HashTable<API_Guid, GS::Array<OtdOpening>> openinginwall; // Все проёмы в зонах
    for (const API_ElemTypeID& typeelem : typeinzone) {
        if (elementToRead.ContainsKey (typeelem)) {
            for (UnicElement::PairIterator cIt = elementToRead.Get (typeelem).EnumeratePairs (); cIt != NULL; ++cIt) {
                nPhase += 1;
                #if defined(AC_27) || defined(AC_28)
                ACAPI_ProcessWindow_SetNextProcessPhase (&funcname, &nPhase);
                if (ACAPI_ProcessWindow_IsProcessCanceled ()) return;
                #else
                ACAPI_Interface (APIIo_SetNextProcessPhaseID, &funcname, &nPhase);
                if (ACAPI_Interface (APIIo_IsProcessCanceledID, nullptr, nullptr)) return;
                #endif
                #if defined(AC_28)
                API_Guid guid = cIt->key;
                GS::Array<API_Guid> zoneGuids = cIt->value;
                #else
                API_Guid guid = *cIt->key;
                GS::Array<API_Guid> zoneGuids = *cIt->value;
                #endif
                // Проверяем классификацию, исключаем отделочные элементы
                if (Class_IsElementFinClass (guid, finclassguids))
                    continue;
                switch (typeelem) {
                    case API_WindowID:
                    case API_DoorID:
                        Opening_Create_One (storyLevels, guid, openinginwall, guidselementToRead);
                        break;
                    case API_WallID:
                        OtdWall_Create_FromWall (storyLevels, guid, zoneGuids, roomsinfo, openinginwall, guidselementToRead);
                        break;
                    case API_ColumnID:
                        Column_Create_One (storyLevels, guid, zoneGuids, roomsinfo, guidselementToRead);
                        break;
                    case API_SlabID:
                        Floor_FindInOneRoom (storyLevels, guid, zoneGuids, roomsinfo, guidselementToRead);
                        break;
                    default:
                        break;
                }
            }
        }
    }
    // Необходимые для чтения параметры и свойства
    ReadParams windowParams = Param_GetForWindowParams (propertyParams);
    ReadParams roomParams = Param_GetForZoneParams (propertyParams);

    ParamDictElement paramToRead; // Прочитанные из элементов свойства
    ParamValue param_composite; // Состав базовых конструкций
    typeinzone.Push (API_ZoneID);
    for (const API_ElemTypeID& typeelem : typeinzone) {
        if (guidselementToRead.ContainsKey (typeelem)) {
            ParamDictValue paramDict;
            if (typeelem == API_ZoneID) {
                Param_ToParamDict (paramDict, roomParams);
            }
            if (typeelem == API_WallID || typeelem == API_ColumnID || typeelem == API_SlabID) {
                Param_GetForBase (propertyParams, paramDict, param_composite);
            }
            if (typeelem == API_WindowID) {
                Param_ToParamDict (paramDict, windowParams);
            }
            if (!paramDict.IsEmpty ()) {
                for (const API_Guid& guid : guidselementToRead[typeelem]) {
                    ParamHelpers::AddParamDictValue2ParamDictElement (guid, paramDict, paramToRead);
                }
            }
        }
    }
    funcname = GS::UniString::Printf ("Read data from %d elements(s)", paramToRead.GetSize ()); nPhase += 1;
    #if defined(AC_27) || defined(AC_28)
    ACAPI_ProcessWindow_SetNextProcessPhase (&funcname, &nPhase);
    if (ACAPI_ProcessWindow_IsProcessCanceled ()) return;
    #else
    ACAPI_Interface (APIIo_SetNextProcessPhaseID, &funcname, &nPhase);
    if (ACAPI_Interface (APIIo_IsProcessCanceledID, nullptr, nullptr)) return;
    #endif
    // Читаем свойства всех элементов
    ParamHelpers::ElementsRead (paramToRead, propertyParams, systemdict);
    // Словарь избранного
    MatarialToFavoriteDict favdict = Favorite_GetDict ();
    // Заполняем данные для элементов

    funcname = GS::UniString::Printf ("Calculate finising elements for %d room(s)", roomsinfo.GetSize ());
    GS::HashTable<GS::UniString, GS::Int32> material_dict; // Словарь индексов покрытий
    for (OtdRooms::PairIterator cIt = roomsinfo.EnumeratePairs (); cIt != NULL; ++cIt) {
        #if defined(AC_28)
        OtdRoom& otd = cIt->value;
        API_Guid zoneGuid = cIt->key;
        #else
        OtdRoom& otd = *cIt->value;
        API_Guid zoneGuid = *cIt->key;
        #endif
        nPhase += 1;
        #if defined(AC_27) || defined(AC_28)
        ACAPI_ProcessWindow_SetNextProcessPhase (&funcname, &nPhase);
        if (ACAPI_ProcessWindow_IsProcessCanceled ()) return;
        #else
        ACAPI_Interface (APIIo_SetNextProcessPhaseID, &funcname, &nPhase);
        if (ACAPI_Interface (APIIo_IsProcessCanceledID, nullptr, nullptr)) return;
        #endif
        if (!paramToRead.ContainsKey (zoneGuid)) continue; // Если у зоны нет прочитанных параметров - дальше делать нечего
        // Заполняем данные для зон
        Param_SetToRooms (material_dict, otd, paramToRead, roomParams);
        if (!otd.isValid) continue;
        // Расчёт пола и потолка
        Floor_Create_All (storyLevels, otd, guidselementToRead, paramToRead);
        for (OtdSlab& otdslab : otd.otdslab) {
            bool base_flipped = false;
            GS::UniString fav_name;
            Param_SetToBase (otdslab.base_guid, base_flipped, otdslab.base_composite, paramToRead, param_composite, fav_name);
            otdslab.favorite.name = fav_name;
            SetMaterialFinish (otdslab.material, otdslab.type, otdslab.draw_type, favdict, otdslab.base_composite, otdslab.favorite);
        }
        if (otd.otdwall.IsEmpty ()) continue;
        GS::Array<OtdWall> opw; // Массив созданных стен
        for (OtdWall& otdw : otd.otdwall) {
            // Заполняем данные для отделочных стен (состав)
            GS::UniString fav_name;
            Param_SetToBase (otdw.base_guid, otdw.base_flipped, otdw.base_composite, paramToRead, param_composite, fav_name);
            otdw.favorite.name = fav_name;
            if (!otdw.openings.IsEmpty ()) {
                Point2D wbegC = { otdw.begC.x, otdw.begC.y };
                Point2D wendC = { otdw.endC.x, otdw.endC.y };
                Sector walledge = { wbegC , wendC };
                GS::Optional<UnitVector_2D>	walldir = walledge.GetDirection ();
                Geometry::Vector2<double> walldir_perp;
                if (walldir.HasValue ()) {
                    double angz = -DEGRAD * 90;
                    double co = cos (angz);
                    double si = sin (angz);
                    walldir_perp = walldir.Get ().ToVector2D ().Rotate (si, co);
                }
                for (OtdOpening& op : otdw.openings) {
                    // Заполняем данные для окон
                    Param_SetToWindows (op, paramToRead, windowParams, otdw);
                    if (walldir.HasValue ()) {
                        OpeningReveals_Create_One (favdict, otd.otdslab, otdw, op, walldir_perp, opw, otd.zBottom, otd.height_down, otd.height_main, otd.height_up, otd.height, otd.om_main, otd.om_up, otd.om_down, otd.om_reveals, otd.om_column, otd.om_floor, otd.om_ceil, otd.om_zone);
                    }
                }
            }
        }
        for (OtdWall& otdw : otd.otdwall) {
            // Разделяем стенки
            OtdWall_Delim_All (favdict, opw, otdw, otd.zBottom, otd.height_down, otd.height_main, otd.height_up, otd.height, otd.om_main, otd.om_up, otd.om_down, otd.om_reveals, otd.om_column, otd.om_floor, otd.om_ceil, otd.om_zone);
        }
        otd.otdwall = opw; // Заменяем на разбитые стены
    }
    // Получаем список существующих элементов отделки для обрабатываемых зон
    zones.Clear ();
    ParamDictElement paramToWrite; // Параметры для записи в зоны и элементы отделки
    ColumnFormatDict columnFormat; // Словарь с форматом текста для столбцов
    funcname = GS::UniString::Printf ("Calculate material for %d room(s)", roomsinfo.GetSize ());
    for (OtdRooms::PairIterator cIt = roomsinfo.EnumeratePairs (); cIt != NULL; ++cIt) {
        #if defined(AC_28)
        OtdRoom& otd = cIt->value;
        #else
        OtdRoom& otd = *cIt->value;
        #endif

        nPhase += 1;
        #if defined(AC_27) || defined(AC_28)
        ACAPI_ProcessWindow_SetNextProcessPhase (&funcname, &nPhase);
        if (ACAPI_ProcessWindow_IsProcessCanceled ()) return;
        #else
        ACAPI_Interface (APIIo_SetNextProcessPhaseID, &funcname, &nPhase);
        if (ACAPI_Interface (APIIo_IsProcessCanceledID, nullptr, nullptr)) return;
        #endif

        if (otd.isValid && (otd.create_all_elements || otd.create_ceil_elements || otd.create_floor_elements || otd.create_wall_elements || otd.create_column_elements || otd.create_reveal_elements)) {
            zones.Push (otd.zone_guid);
        }
        if (otd.isValid && paramToRead.ContainsKey (otd.zone_guid)) {
            GS::Array<GS::UniString> rawnames = { otd.om_up.rawname, otd.om_main.rawname, otd.om_down.rawname , otd.om_column.rawname , otd.om_reveals.rawname, otd.om_floor.rawname, otd.om_ceil.rawname };
            for (const GS::UniString& rawname : rawnames) {
                if (rawname.IsEmpty ()) continue;
                if (columnFormat.ContainsKey (rawname)) continue;
                if (!paramToRead.Get (otd.zone_guid).ContainsKey (rawname)) continue;
                WriteOtdData_GetColumnfFormat (paramToRead.Get (otd.zone_guid).Get (rawname).definition.description, rawname, columnFormat);
            }
            WriteOtdDataToRoom (columnFormat, otd, paramToWrite, paramToRead);
        }
    }

    // Проверка существования классов и свойств
    if (!zones.IsEmpty ()) {
        if (!Check (finclass, propertyParams, finclassguids)) {
            #if defined(AC_27) || defined(AC_28)
            ACAPI_ProcessWindow_CloseProcessWindow ();
            #else
            ACAPI_Interface (APIIo_CloseProcessWindowID, nullptr, nullptr);
            #endif
            return;
        }
    }

    // Поиск существующих элементов отделки
    GS::HashTable<API_Guid, UnicGuid> exsistotdelements; // Словарь существующих отделочных элементов с разбивкой по зонам
    GS::Array<API_Guid> deletelist; // Список элементов для удаления
    int errcode;
    if (SyncGetPatentelement (zones, exsistotdelements, propertyParams, "zone", errcode)) {
        for (GS::HashTable<API_Guid, UnicGuid>::PairIterator cIt = exsistotdelements.EnumeratePairs (); cIt != NULL; ++cIt) {
            #if defined(AC_28)
            UnicGuid guids = cIt->value;
            #else
            UnicGuid guids = *cIt->value;
            #endif
            for (UnicGuid::PairIterator cItt = guids.EnumeratePairs (); cItt != NULL; ++cItt) {
                #if defined(AC_28)
                API_Guid guid = cItt->key;
                bool isvisible = cItt->value;
                #else
                API_Guid guid = *cItt->key;
                bool isvisible = *cItt->value;
                #endif
                if (Class_IsElementFinClass (guid, finclassguids)) deletelist.Push (guid);
            }
        }
    }
    UnicElementByType subelementByparent; // Словарь с созданными родительскими и дочерними элементами
    // Отросовка элементов отделки
    Draw_Elements (storyLevels, roomsinfo, subelementByparent, finclass, deletelist);
    // Привязка отделочных элементов к базовым
    SetSyncOtdWall (subelementByparent, propertyParams, paramToWrite);
    finish = clock ();
    duration = (double) (finish - start) / CLOCKS_PER_SEC;
    GS::UniString time = GS::UniString::Printf ("Calculate complete for %d room(s) by", zones.GetSize ()) + GS::UniString::Printf (" %.3f s", duration);
    msg_rep ("RoomBook", time, NoError, APINULLGuid);
    #if defined(AC_27) || defined(AC_28)
    ACAPI_ProcessWindow_CloseProcessWindow ();
    #else
    ACAPI_Interface (APIIo_CloseProcessWindowID, nullptr, nullptr);
    #endif
}

// Настройки для форматирования текста в таблицу
void WriteOtdData_GetColumnfFormat (GS::UniString descripton, const GS::UniString& rawname, ColumnFormatDict& columnFormat)
{
    GS::UniString fontname = "GOST 2.304 type A";
    ColumnFormat c;
    if (isEng ()) {
        c.no_breake_space = " ";
        c.narow_space = " ";
    }
    if (descripton.Contains ("{") && descripton.Contains ("}") && descripton.Contains (";")) {
        descripton = descripton.GetSubstring ('{', '}', 0);
        GS::Array<GS::UniString> partstring;
        if (StringSplt (descripton, ";", partstring) != 4) {
            c.fontsize = 3;
            c.width_mat = 40;
            c.width_area = 20;
        } else {
            if (!UniStringToDouble (partstring[0], c.width_mat)) c.width_mat = 50;
            if (!UniStringToDouble (partstring[1], c.width_area)) c.width_area = 20;
            if (!UniStringToDouble (partstring[2], c.fontsize)) c.fontsize = 3;
            fontname = partstring[3];
        }
    } else {
        c.fontsize = 3;
        c.width_mat = 50;
        c.width_area = 20;
    }
    c.font = GetFontIndex (fontname);
    if (c.font == 0) {
        fontname = "GOST 2.304 type A";
        c.font = GetFontIndex (fontname);
    }
    if (c.font == 0) {
        fontname = "ISOCPEUR";
        c.font = GetFontIndex (fontname);
    }
    if (c.font == 0) {
        fontname = "Arial";
        c.font = GetFontIndex (fontname);
    }
    Int32 addspace = 0;
    GS::UniString delim = "-";
    c.width_no_breake_space = GetTextWidth (c.font, c.fontsize, c.no_breake_space);
    c.width_narow_space = GetTextWidth (c.font, c.fontsize, c.narow_space);
    addspace = (Int32) (c.width_area / c.width_narow_space);
    if (fabs (addspace * c.width_narow_space - c.width_area) > c.width_narow_space) addspace -= 1;
    c.space_line = "";
    for (Int32 j = 0; j < addspace; j++) { c.space_line.Append (c.narow_space); }
    c.width_area = GetTextWidth (c.font, c.fontsize, c.space_line);
    double width_delim = GetTextWidth (c.font, c.fontsize, delim);
    addspace = (Int32) ((c.width_mat + c.width_area) / width_delim);
    if (fabs (addspace * width_delim - c.width_mat - c.width_area) > c.width_narow_space) addspace -= 1;
    c.delim_line = "";
    for (Int32 j = 0; j < addspace; j++) { c.delim_line.Append (delim); }
    c.width_mat = GetTextWidth (c.font, c.fontsize, c.delim_line) - c.width_area;
    c.delim_line.Append (" ");
    c.space_line.Append (" ");
    columnFormat.Add (rawname, c);
}

void WriteOtdDataToRoom (const ColumnFormatDict& columnFormat, const OtdRoom& otd, ParamDictElement& paramToWrite, const ParamDictElement& paramToRead)
{

    GS::HashTable<TypeOtd, GS::UniString> paramnamebytype;
    paramnamebytype.Add (Wall_Up, otd.om_up.rawname); // Отделка стен выше потолка
    paramnamebytype.Add (Wall_Main, otd.om_main.rawname); // Отделка стен основная
    paramnamebytype.Add (Wall_Down, otd.om_down.rawname); // Отделка низа стен
    paramnamebytype.Add (Column, otd.om_column.rawname); // Отделка колонн
    paramnamebytype.Add (Reveal_Main, otd.om_reveals.rawname); // Отделка откосов
    paramnamebytype.Add (Floor, otd.om_floor.rawname); // Отделка пола
    paramnamebytype.Add (Ceil, otd.om_ceil.rawname); // Отделка потолка

    GS::HashTable<GS::UniString, bool> exsists_rawname;
    for (auto& cItt : paramnamebytype) {
        #if defined(AC_28)
        GS::UniString rawname = cItt.value;
        #else
        GS::UniString rawname = *cItt.value;
        #endif
        if (!exsists_rawname.ContainsKey (rawname)) exsists_rawname.Add (rawname, false);
    }

    OtdMaterialAreaDictByType dct; // Основной словарь
    // Разбивка по отделочным слоям, вычисление площадей и добавление их в словарь по типам отделки
    for (const OtdWall& otdw : otd.otdwall) {
        const TypeOtd& t = otdw.type;
        if (paramnamebytype.ContainsKey (t)) {
            double area = OtdWall_GetArea (otdw);
            GS::UniString rawname = paramnamebytype.Get (t);
            for (UInt32 j = 0; j < otdw.base_composite.GetSize (); j++) {
                GS::UniString mat = otdw.base_composite[j].val;
                WriteOtdDataToRoom_AddValue (dct, rawname, mat, area);
            }
        }
    }

    for (const OtdSlab& otdslab : otd.otdslab) {
        const TypeOtd& t = otdslab.type;
        if (paramnamebytype.ContainsKey (t)) {
            double area = otdslab.poly.CalcArea ();
            GS::UniString rawname = paramnamebytype.Get (t);
            for (UInt32 j = 0; j < otdslab.base_composite.GetSize (); j++) {
                GS::UniString mat = otdslab.base_composite[j].val;
                WriteOtdDataToRoom_AddValue (dct, rawname, mat, area);
            }
        }
    }

    for (auto& cItt : exsists_rawname) {
        #if defined(AC_28)
        GS::UniString rawname = cItt.key;
        #else
        GS::UniString rawname = *cItt.key;
        #endif
        // Проверяем - есть ли считанное свойство в зоне для записи
        if (!paramToRead.Get (otd.zone_guid).ContainsKey (rawname)) continue;
        ParamValue paramtow = paramToRead.Get (otd.zone_guid).Get (rawname);
        GS::UniString old_val = paramtow.val.uniStringValue;
        GS::UniString new_val = "";
        // Если такой тип отделки есть в словаре - записываем послойно материалы и площади
        if (dct.ContainsKey (rawname)) {
            ColumnFormat c;
            if (columnFormat.ContainsKey (rawname)) c = columnFormat.Get (rawname);
            OtdMaterialAreaDict& dcta = dct.Get (rawname);
            std::map<std::string, GS::UniString, doj::alphanum_less<std::string> > abc_material = {};
            for (auto& cIt : dcta) {
                #if defined(AC_28)
                GS::UniString mat = cIt.key;
                #else
                GS::UniString mat = *cIt.key;
                #endif
                GSCharCode chcode = GetCharCode (mat);
                std::string s = mat.ToCStr (0, MaxUSize, chcode).Get ();
                abc_material[s] = mat;
            }
            for (std::map<std::string, GS::UniString, doj::alphanum_less<std::string> >::iterator k = abc_material.begin (); k != abc_material.end (); ++k) {
                GS::UniString mat = k->second;
                if (!dcta.ContainsKey (mat)) continue;
                double area = dcta.Get (mat);
                mat.Trim ();
                mat.ReplaceAll ("  ", " ");
                mat.ReplaceAll ("0&#& ", "");
                mat.ReplaceAll (" ", c.no_breake_space);
                GS::UniString area_sring = GS::UniString::Printf ("%.2f", area);
                double w_area = GetTextWidth (c.font, c.fontsize, area_sring);
                if (w_area < c.width_area) {
                    Int32 addspace = (Int32) ((c.width_area - w_area) / c.width_narow_space);
                    if (fabs (addspace * c.width_narow_space - c.width_area + w_area) > 0.01) addspace -= 1;
                    if (addspace > 1) {
                        for (Int32 j = 0; j < addspace; j++) {
                            area_sring = c.narow_space + area_sring;
                        }
                    }
                    w_area = GetTextWidth (c.font, c.fontsize, area_sring);
                }
                if (w_area - c.width_area > 0.1) area_sring.Append (c.narow_space);
                area_sring.Append (" ");
                if (!new_val.IsEmpty ()) new_val.Append (c.delim_line);
                GS::Array<GS::UniString> lines_mat = DelimTextLine (c.font, c.fontsize, c.width_mat, mat, c.no_breake_space, c.narow_space);
                for (UInt32 j = 0; j < lines_mat.GetSize (); j++) {
                    new_val.Append (lines_mat[j]);
                    if (j == lines_mat.GetSize () - 1) {
                        new_val.Append (area_sring);
                    } else {
                        new_val.Append (c.space_line);
                    }
                }
            }
        }
        if (!old_val.IsEqual (new_val)) {
            paramtow.val.uniStringValue = new_val;
            paramtow.isValid = true;
            ParamHelpers::AddParamValue2ParamDictElement (otd.zone_guid, paramtow, paramToWrite);
        }
    }
}

void WriteOtdDataToRoom_AddValue (OtdMaterialAreaDictByType& dct, const GS::UniString& rawname, const GS::UniString& mat, const double& area)
{
    if (area < 0.000001) return;
    if (!dct.ContainsKey (rawname)) {
        OtdMaterialAreaDict dcta;
        dcta.Add (mat, area);
        dct.Add (rawname, dcta);
        return;
    } else {
        if (!dct.Get (rawname).ContainsKey (mat)) {
            dct.Get (rawname).Add (mat, area);
        } else {
            double area_sum = dct.Get (rawname).Get (mat) + area;
            dct.Get (rawname).Set (mat, area_sum);
        }
    }
}

// -----------------------------------------------------------------------------
// Получение площади отделочной стены с учётом отверстий
// -----------------------------------------------------------------------------
double OtdWall_GetArea (const OtdWall& otdw)
{
    double dx = -otdw.endC.x + otdw.begC.x;
    double dy = -otdw.endC.y + otdw.begC.y;
    double dr = sqrt (dx * dx + dy * dy);
    double area = dr * otdw.height;
    // Если задана ширина - это откос
    if (!is_equal (otdw.width, 0)) {
        if (!is_equal (otdw.length, 0)) {
            area = otdw.width * otdw.length;
        } else {
            area = otdw.width * otdw.height;
        }
        // В откосах проёмов быть не может - выходим
        if (area < 0) area = 0;
        return area;
    }
    if (is_equal (dr, 0)) return 0;
    if (is_equal (area, 0)) return 0;
    if (!otdw.openings.IsEmpty ()) {
        double area_op = 0;
        for (const OtdOpening& op : otdw.openings) {
            area_op += op.height * op.width;
        }
        area -= area_op;
        if (area < 0) area = 0;
    }
    return area;
}

// -----------------------------------------------------------------------------
// Получение информации из зоны о полгионах и находящейся в ней элементах
// -----------------------------------------------------------------------------
bool CollectRoomInfo (const Stories& storyLevels, API_Guid& zoneGuid, OtdRoom& roominfo, UnicElementByType& elementToRead, GS::HashTable<API_Guid, GS::Array<API_Guid>>& slabsinzone)
{
    SyncSettings syncSettings;
    syncSettings.wallS = true;
    syncSettings.syncAll = true;
    syncSettings.syncMon = true;
    syncSettings.wallS = true;
    syncSettings.widoS = true;
    syncSettings.objS = true;
    syncSettings.cwallS = true;
    GSErrCode err;
    API_Element element = {}; BNZeroMemory (&element, sizeof (API_Element));
    API_Element zoneelement = {}; BNZeroMemory (&zoneelement, sizeof (API_Element));
    zoneelement.header.guid = zoneGuid;
    err = ACAPI_Element_Get (&zoneelement);
    if (err != NoError) {
        msg_rep ("CollectRoomInfo err", "ACAPI_Element_Get zone", err, zoneGuid);
        return false;
    }
    roominfo.height = zoneelement.zone.roomHeight;
    if (roominfo.height < 0.0001) {
        msg_rep ("CollectRoomInfo err", "room.height < 0.0001", err, zoneGuid);
        roominfo.isValid = false;
        return false;
    }
    API_ElementMemo zonememo = {}; BNZeroMemory (&zonememo, sizeof (API_ElementMemo));
    err = ACAPI_Element_GetMemo (zoneelement.header.guid, &zonememo);
    if (err != NoError) {
        msg_rep ("CollectRoomInfo err", "ACAPI_Element_GetMemo zone", err, zoneGuid);
        return false;
    }
    GS::Array<Sector> walledges; // Границы стен, не явяющихся границей зоны
    GS::Array<Sector> columnedges; // Границы колонн, не явяющихся границей зоны
    GS::Array<Sector> restedges; // Границы зоны
    GS::Array<Sector> gableedges;
    Edges_GetFromRoom (zonememo, zoneelement, walledges, columnedges, restedges, gableedges);
    roominfo.edges = restedges;
    roominfo.columnedges = columnedges;
    API_RoomRelation relData = {};
    err = ACAPI_Element_GetRelations (zoneGuid, API_ZombieElemID, &relData);
    if (err != NoError) {
        msg_rep ("CollectRoomInfo err", "ACAPI_Element_GetRelations zone", err, zoneGuid);
        return false;
    }
    roominfo.zBottom = GetzPos (zoneelement.zone.roomBaseLev, zoneelement.header.floorInd, storyLevels);
    roominfo.floorInd = zoneelement.header.floorInd;
    GS::Array<API_ElemTypeID> typeinzone;
    typeinzone.Push (API_WindowID);
    typeinzone.Push (API_DoorID);
    typeinzone.Push (API_WallID);
    typeinzone.Push (API_ColumnID);
    typeinzone.Push (API_SlabID);
    bool flag = false;
    for (const API_ElemTypeID& typeelem : typeinzone) {
        if (relData.elementsGroupedByType.ContainsKey (typeelem)) {
            GS::Array<API_Guid> guids = relData.elementsGroupedByType.Get (typeelem);
            for (const API_Guid& elGuid : guids) {
                Param_AddUnicElementByType (elGuid, zoneGuid, typeelem, elementToRead);
            }
            flag = true;
        }
    }
    if (slabsinzone.ContainsKey (zoneGuid)) {
        for (const API_Guid& elGuid : slabsinzone.Get (zoneGuid)) {
            Param_AddUnicElementByType (elGuid, zoneGuid, API_SlabID, elementToRead);
        }
    }
    OtdSlab otdslab;
    ConstructPolygon2DFromElementMemo (zonememo, otdslab.poly);
    #if defined(AC_27) || defined(AC_28)
    otdslab.material.material = zoneelement.zone.material.ToInt32_Deprecated ();
    #else
    otdslab.material.material = zoneelement.zone.material;
    #endif
    otdslab.base_type = API_ZoneID;
    otdslab.base_guid = zoneGuid;
    otdslab.floorInd = zoneelement.header.floorInd;
    roominfo.poly = otdslab;
    if (!walledges.IsEmpty ()) {
        API_Guid elGuid;
        API_ElemSearchPars searchPars;
        API_ElemTypeID typeelem = API_WallID;
        for (UInt32 j = 0; j < walledges.GetSize (); j++) {
            Sector tedge = walledges[j];
            BNZeroMemory (&searchPars, sizeof (API_ElemSearchPars));
            searchPars.filterBits = APIFilt_OnVisLayer | APIFilt_In3D | APIFilt_IsVisibleByRenovation | APIFilt_IsInStructureDisplay;
            searchPars.loc.x = tedge.GetMidPoint ().x;
            searchPars.loc.y = tedge.GetMidPoint ().y;
            searchPars.z = roominfo.zBottom + roominfo.height;
            #if defined(AC_27) || defined(AC_28) || defined(AC_26)
            searchPars.type.typeID = typeelem;
            #else
            searchPars.typeID = typeelem;
            #endif
            #if defined(AC_27) || defined(AC_28)
            err = ACAPI_Element_SearchElementByCoord (&searchPars, &elGuid);
            #else
            elGuid = APINULLGuid;
            err = ACAPI_Goodies (APIAny_SearchElementByCoordID, &searchPars, &elGuid);
            #endif
            if (err == NoError && elGuid != APINULLGuid) {
                flag = true;
                GS::Array<API_Guid> subelemGuid;
                GetRelationsElement (elGuid, syncSettings, subelemGuid);
                for (const API_Guid& elGuid_ : subelemGuid) {
                    Param_AddUnicElementByType (elGuid_, zoneGuid, API_WindowID, elementToRead);
                }
                if (!roominfo.walledges.ContainsKey (elGuid)) {
                    GS::Array<Sector> t;
                    t.Push (tedge);
                    roominfo.walledges.Add (elGuid, t);
                } else {
                    roominfo.walledges.Get (elGuid).Push (tedge);
                }
                if (!elementToRead.ContainsKey (typeelem)) {
                    UnicElement el;
                    elementToRead.Add (typeelem, el);
                }
                if (elementToRead.ContainsKey (typeelem)) {
                    if (!elementToRead.Get (typeelem).ContainsKey (elGuid)) {
                        GS::Array<API_Guid> el;
                        el.Push (zoneGuid);
                        elementToRead.Get (typeelem).Add (elGuid, el);
                    } else {
                        if (typeelem == API_WallID || typeelem == API_ColumnID) elementToRead.Get (typeelem).Get (elGuid).Push (zoneGuid);
                    }
                }
            } else {
                msg_rep ("CollectRoomInfo err", "ACAPI_Element_SearchElementByCoord element not found", err, zoneGuid);
            }
        }
    }
    roominfo.wallPart = relData.wallPart;
    roominfo.beamPart = relData.beamPart;
    roominfo.cwSegmentPart = relData.cwSegmentPart;
    roominfo.niches = relData.niches;
    roominfo.zone_guid = zoneGuid;
    roominfo.isValid = flag;
    #if defined(AC_27) || defined(AC_28)
    roominfo.om_zone.material = zoneelement.zone.material.ToInt32_Deprecated ();
    #else
    roominfo.om_zone.material = zoneelement.zone.material;
    #endif
    ACAPI_DisposeRoomRelationHdls (&relData);
    ACAPI_DisposeElemMemoHdls (&zonememo);
    return flag;
}

// -----------------------------------------------------------------------------
// Чтение данных о проёме
// -----------------------------------------------------------------------------
void Opening_Create_One (const Stories& storyLevels, const API_Guid& elGuid, GS::HashTable<API_Guid, GS::Array<OtdOpening>>& openinginwall, UnicGUIDByType& guidselementToRead)
{
    GSErrCode err;
    API_Element element = {};
    element.header.guid = elGuid;
    err = ACAPI_Element_Get (&element);
    if (err != NoError) {
        msg_rep ("Opening_Create_One err", "ACAPI_Element_Get", err, elGuid);
        return;
    }
    API_Guid wallguid = APINULLGuid;
    OtdOpening op;
    API_ElemTypeID eltype = GetElemTypeID (element);
    switch (eltype) {
        case API_WindowID:
            wallguid = element.window.owner;
            op.height = element.window.openingBase.height;
            op.width = element.window.openingBase.width;
            op.lower = element.window.lower;
            op.objLoc = element.window.objLoc;
            op.reflected = element.window.openingBase.reflected;
            break;
        case API_DoorID:
            wallguid = element.door.owner;
            op.height = element.door.openingBase.height;
            op.width = element.door.openingBase.width;
            op.lower = element.door.lower;
            op.objLoc = element.door.objLoc;
            op.reflected = element.door.openingBase.reflected;
            break;
        default:
            break;
    }
    if (op.width < min_dim || op.height < min_dim) return;
    op.base_guid = elGuid;
    if (openinginwall.ContainsKey (wallguid)) {
        openinginwall.Get (wallguid).Push (op);
    } else {
        GS::Array<OtdOpening> openings;
        openings.Push (op);
        openinginwall.Add (wallguid, openings);
    }
    Param_AddUnicGUIDByType (elGuid, API_WindowID, guidselementToRead);
}

// -----------------------------------------------------------------------------
// Создание стен-отделок для стен
// -----------------------------------------------------------------------------
void OtdWall_Create_FromWall (const Stories& storyLevels, API_Guid& elGuid, GS::Array<API_Guid>& zoneGuids, OtdRooms& roomsinfo, GS::HashTable<API_Guid, GS::Array<OtdOpening>>& openinginwall, UnicGUIDByType& guidselementToRead)
{
    GSErrCode err;
    API_Element element = {};
    element.header.guid = elGuid;
    err = ACAPI_Element_Get (&element);
    if (err != NoError) {
        msg_rep ("OtdWall_Create_FromWall err", "ACAPI_Element_Get", err, elGuid);
        return;
    }
    Point2D wbegC = { element.wall.begC.x, element.wall.begC.y };
    Point2D wendC = { element.wall.endC.x, element.wall.endC.y };
    Sector walledge = { wbegC , wendC };
    double dx = -element.wall.endC.x + element.wall.begC.x;
    double dy = -element.wall.endC.y + element.wall.begC.y;
    double wallLength = sqrt (dx * dx + dy * dy);
    double zBottom = GetzPos (element.wall.bottomOffset, element.header.floorInd, storyLevels);
    if (wallLength < min_dim) {
        #if defined(TESTING)
        DBprnt ("OtdWall_Create_FromWall err", "walledge.GetLength ()<min_dim");
        #endif
        return;
    }
    GS::Optional<UnitVector_2D>	walldir = walledge.GetDirection ();
    if (!walldir.HasValue ()) {
        #if defined(TESTING)
        DBprnt ("OtdWall_Create_FromWall err", "!walldir.HasValue");
        #endif
        return;
    }
    for (API_Guid zoneGuid : zoneGuids) {
        if (!roomsinfo.ContainsKey (zoneGuid)) {
            #if defined(TESTING)
            DBprnt ("OtdWall_Create_FromWall err", "!roomsinfo.ContainsKey (zoneGuid)");
            #endif
            continue;
        }
        OtdRoom& roominfo = roomsinfo.Get (zoneGuid);
        if (!roominfo.isValid) continue;
        bool flag_find = false;
        if (roominfo.walledges.ContainsKey (elGuid)) {
            GS::Optional<Geometry::Line2D> wallline = walledge.AsLine ();
            if (!wallline.HasValue ()) {
                #if defined(TESTING)
                DBprnt ("OtdWall_Create_FromWall err", "!wallline.HasValue ()");
                #endif
                continue;
            }
            for (Sector& roomedge : roominfo.walledges[elGuid]) {
                GS::Optional<Geometry::Line2D> roomeline = roomedge.AsLine ();
                if (!roomeline.HasValue ()) {
                    #if defined(TESTING)
                    DBprnt ("OtdWall_Create_FromWall err", "!roomeline.HasValue ()");
                    #endif
                    continue;
                }
                GS::Optional<UnitVector_2D>	roomedgedir = roomedge.GetDirection ();
                if (!roomedgedir.HasValue ()) {
                    #if defined(TESTING)
                    DBprnt ("OtdWall_Create_FromWall err", "!roomedgedir.HasValue");
                    #endif
                    continue;
                }
                bool is_parallel = walldir.Get ().IsParallelWith (roomedgedir.Get ());
                bool is_fliped = false;
                if (is_parallel) is_fliped = !walldir.Get ().IsEqualWith (roomedgedir.Get ());
                OtdWall wallotd;
                if (is_parallel) {
                    Point2D begedge;
                    Point2D endedge;
                    if (is_fliped) {
                        begedge = wallline.Get ().ProjectPoint (roomedge.c2);
                        endedge = wallline.Get ().ProjectPoint (roomedge.c1);
                    } else {
                        begedge = wallline.Get ().ProjectPoint (roomedge.c1);
                        endedge = wallline.Get ().ProjectPoint (roomedge.c2);
                    }
                    double tBeg = sqrt ((wbegC.x - begedge.x) * (wbegC.x - begedge.x) + (wbegC.y - begedge.y) * (wbegC.y - begedge.y));
                    double tEnd = wallLength - sqrt ((wendC.x - endedge.x) * (wendC.x - endedge.x) + (wendC.y - endedge.y) * (wendC.y - endedge.y));
                    // Ищем проёмы в этом участке стены
                    if (openinginwall.ContainsKey (elGuid)) {
                        for (const OtdOpening& op : openinginwall[elGuid]) {
                            Opening_Add_One (op, is_fliped, zBottom, tBeg, tEnd, wallLength, wallotd);
                        }
                    }
                }
                walledge = roomedge;
                if (OtdWall_Add_One (elGuid, walledge, is_fliped, element.wall.height, zBottom, roominfo.floorInd, element.wall.thickness, wallotd)) {
                    roominfo.otdwall.Push (wallotd);
                    flag_find = true;
                }
            }
        }
        if (!roominfo.wallPart.IsEmpty () && element.wall.zoneRel != APIZRel_None) {
            for (API_WallPart& wpart : roominfo.wallPart) {
                if (wpart.guid != elGuid)  continue;
                UInt32 inxedge = wpart.roomEdge - 1;
                if (inxedge > roominfo.edges.GetSize ()) {
                    #if defined(TESTING)
                    DBprnt ("OtdWall_Create_FromWall err", "inxedge > roomedges.restedges.GetSize ()");
                    #endif
                    continue;
                }
                Sector roomedge = roominfo.edges.Get (inxedge);
                GS::Optional<Geometry::Line2D> roomeline = roomedge.AsLine ();
                if (!roomeline.HasValue ()) {
                    #if defined(TESTING)
                    DBprnt ("OtdWall_Create_FromWall err", "!roomeline.HasValue ()");
                    #endif
                    continue;
                }
                GS::Optional<UnitVector_2D>	roomedgedir = roomedge.GetDirection ();
                if (!roomedgedir.HasValue ()) {
                    #if defined(TESTING)
                    DBprnt ("OtdWall_Create_FromWall err", "!roomedgedir.HasValue");
                    #endif
                    continue;
                }

                bool is_parallel = walldir.Get ().IsParallelWith (roomedgedir.Get ());
                bool is_fliped = false;
                if (is_parallel) is_fliped = !walldir.Get ().IsEqualWith (roomedgedir.Get ());

                OtdWall wallotd;
                if (is_parallel) {
                    Point2D begedge = wbegC;
                    Point2D endedge = wendC;
                    if (!is_equal (wpart.tEnd, 0) || !is_equal (wpart.tBeg, 0)) {
                        if (!is_equal (wpart.tBeg, 0) && !is_equal (wpart.tBeg, wallLength)) {
                            double lambda = wpart.tBeg / wallLength;
                            begedge.x = wbegC.x - dx * lambda;
                            begedge.y = wbegC.y - dy * lambda;
                        }
                        if (!is_equal (wpart.tEnd, 0) && !is_equal (wpart.tEnd, wallLength)) {
                            double lambda = wpart.tEnd / wallLength;
                            endedge.x = wbegC.x - dx * lambda;
                            endedge.y = wbegC.y - dy * lambda;
                        }
                    }
                    begedge = roomeline.Get ().ProjectPoint (begedge);
                    endedge = roomeline.Get ().ProjectPoint (endedge);
                    if (is_fliped) {
                        walledge = { endedge , begedge };
                    } else {
                        walledge = { begedge , endedge };
                    }
                    // Ищем проёмы в этом участке стены
                    if (openinginwall.ContainsKey (elGuid)) {
                        for (const OtdOpening& op : openinginwall[elGuid]) {
                            Opening_Add_One (op, is_fliped, zBottom, wpart.tBeg, wpart.tEnd, wallLength, wallotd);
                        }
                    }
                    if (element.wall.flipped) is_fliped = !is_fliped;
                } else {
                    walledge = roomedge; // Торец стены
                }
                if (OtdWall_Add_One (elGuid, walledge, is_fliped, element.wall.height, zBottom, roominfo.floorInd, element.wall.thickness, wallotd)) {
                    roominfo.otdwall.Push (wallotd);
                    flag_find = true;
                }
            }
        }
        if (flag_find) Param_AddUnicGUIDByType (elGuid, API_WallID, guidselementToRead);
    }
}

void Opening_Add_One (const OtdOpening& op, const bool& is_fliped, const double& zBottom, const double& tBeg, const double& tEnd, const double& wallLength, OtdWall& wallotd)
{
    // Проверяем - находится ли этот проём на заданном участке стены
    if (op.objLoc <= tEnd + op.width / 2 && op.objLoc >= tBeg - op.width / 2) {
        OtdOpening opinwall;
        opinwall.base_guid = op.base_guid;
        opinwall.width = op.width;
        opinwall.base_reveal_width = op.base_reveal_width;
        opinwall.lower = op.lower;
        opinwall.zBottom = zBottom + op.lower;
        double dw = 0; // Уменьшение ширины в случае, если стена попадает на часть проёма
        if (op.objLoc > tEnd - op.width / 2) {
            dw = tEnd - op.objLoc - op.width / 2;
            opinwall.width = opinwall.width + dw;
        }
        if (op.objLoc < tBeg + op.width / 2) {
            dw = tBeg - op.objLoc + op.width / 2;
            opinwall.width = opinwall.width - dw;
        }
        opinwall.height = op.height;
        if (!is_fliped) {
            opinwall.objLoc = op.objLoc - tBeg + dw / 2;
        } else {
            opinwall.objLoc = wallLength - op.objLoc - (wallLength - tEnd) + dw / 2;
        }
        wallotd.openings.Push (opinwall);
    }
}

bool OtdWall_Add_One (const API_Guid& wallguid, const Sector& walledge, const bool& is_fliped, const double& height, const double& zBottom, const short& floorInd, const double& thickness, OtdWall& wallotd)
{
    double walledgedx = -walledge.c1.x + walledge.c2.x;
    double walledgedy = -walledge.c1.y + walledge.c2.y;
    double walledgedr = sqrt (walledgedx * walledgedx + walledgedy * walledgedy);
    if (walledgedr < min_dim) return false;
    wallotd.base_guid = wallguid;
    wallotd.height = height;
    wallotd.base_th = thickness;
    wallotd.zBottom = zBottom;
    wallotd.base_flipped = is_fliped;
    wallotd.floorInd = floorInd;
    wallotd.begC = { walledge.c1.x, walledge.c1.y };
    wallotd.endC = { walledge.c2.x, walledge.c2.y };
    wallotd.base_type = API_WallID;
    wallotd.type = Wall_Main;
    return true;
}

// -----------------------------------------------------------------------------
// Создание стен-отделок для колонны
// -----------------------------------------------------------------------------
void Column_Create_One (const Stories& storyLevels, API_Guid& elGuid, GS::Array<API_Guid>& zoneGuids, OtdRooms& roomsinfo, UnicGUIDByType& guidselementToRead)
{
    GSErrCode err;
    API_Element element = {};
    API_ElementMemo segmentmemo = {};
    BNZeroMemory (&segmentmemo, sizeof (API_ElementMemo));
    BNZeroMemory (&element, sizeof (API_Element));
    element.header.guid = elGuid;
    err = ACAPI_Element_Get (&element);
    if (err != NoError || !element.header.hasMemo) {
        msg_rep ("Column_Create_One err", "ACAPI_Element_Get", err, elGuid);
        return;
    }
    if (element.column.zoneRel == APIZRel_None) {
        #if defined(TESTING)
        DBprnt ("Column_Create_One err", "element.column.zoneRel == APIZRel_None");
        #endif
        return;
    }
    err = ACAPI_Element_GetMemo (elGuid, &segmentmemo, APIMemoMask_ColumnSegment | APIMemoMask_AssemblySegmentScheme | APIMemoMask_AssemblySegmentProfile);
    if (err != NoError || segmentmemo.columnSegments == nullptr) {
        #if defined(TESTING)
        DBprnt ("Column_Create_One err", "ACAPI_Element_GetMemo column");
        #endif
        return;
    }
    bool flag_find = false;
    Int32 inx = 0;
    Point2D orig = { element.column.origoPos.x,element.column.origoPos.y };
    Point2D begedge = { 0,0 }; Point2D endedge = { 0,0 };
    Sector columnedge = { begedge , endedge };
    if (segmentmemo.columnSegments[inx].assemblySegmentData.modelElemStructureType == API_BasicStructure) {
        double height = segmentmemo.columnSegments[inx].assemblySegmentData.nominalHeight / 2;
        double width = segmentmemo.columnSegments[inx].assemblySegmentData.nominalWidth / 2;
        height = height + segmentmemo.columnSegments[inx].venThick;
        width = width + segmentmemo.columnSegments[inx].venThick;
        // Построение отрезков границ
        GS::Array<Sector> columnedges;
        begedge = { orig.x + width,orig.y + height };
        endedge = { orig.x - width,orig.y + height };
        columnedge = { begedge , endedge };
        columnedges.Push (columnedge);
        begedge = { orig.x - width,orig.y + height };
        endedge = { orig.x - width,orig.y - height };
        columnedge = { begedge , endedge };
        columnedges.Push (columnedge);
        begedge = { orig.x - width,orig.y - height };
        endedge = { orig.x + width,orig.y - height };
        columnedge = { begedge , endedge };
        columnedges.Push (columnedge);
        begedge = { orig.x + width,orig.y - height };
        endedge = { orig.x + width,orig.y + height };
        columnedge = { begedge , endedge };
        columnedges.Push (columnedge);
        double zBottom = GetzPos (element.column.bottomOffset, element.header.floorInd, storyLevels);
        for (Sector& coledge : columnedges) {
            for (API_Guid zoneGuid : zoneGuids) {
                if (!roomsinfo.ContainsKey (zoneGuid)) continue;
                OtdRoom& roominfo = roomsinfo.Get (zoneGuid);
                if (!roominfo.isValid) continue;
                Sector cdge;
                bool find = Edge_FindOnEdge (coledge, roominfo.edges, cdge);
                if (!find) find = Edge_FindOnEdge (coledge, roominfo.columnedges, cdge);
                if (!find) continue;
                OtdWall wallotd;
                wallotd.base_guid = elGuid;
                wallotd.height = element.column.height;
                wallotd.zBottom = zBottom;
                wallotd.floorInd = roominfo.floorInd;
                wallotd.begC = { cdge.c1.x, cdge.c1.y };
                wallotd.endC = { cdge.c2.x, cdge.c2.y };
                wallotd.material = roominfo.om_main;
                wallotd.base_type = API_ColumnID;
                wallotd.type = Column;
                roominfo.otdwall.Push (wallotd);
                flag_find = true;
            }
        }
    }
    if (flag_find) Param_AddUnicGUIDByType (elGuid, API_ColumnID, guidselementToRead);
    ACAPI_DisposeElemMemoHdls (&segmentmemo);
}

// -----------------------------------------------------------------------------
// Находит все перекрытия, котрые пересекают зону и не являются элементами отделки
// -----------------------------------------------------------------------------
void Floor_FindAll (GS::HashTable<API_Guid, GS::Array<API_Guid>>& slabsinzone, const UnicGuid& finclassguids, const GS::Array<API_Guid>& zones)
{
    GS::Array<API_Guid> allslabs_;
    GS::Array<API_Guid> allslabs;
    GSErrCode err;
    err = ACAPI_Element_GetElemList (API_SlabID, &allslabs_, APIFilt_OnVisLayer | APIFilt_IsVisibleByRenovation);
    if (err != NoError) {
        return;
    }
    if (allslabs_.IsEmpty ()) return;
    for (auto guid : allslabs_) {
        if (!Class_IsElementFinClass (guid, finclassguids)) allslabs.Push (guid);
    }
    if (allslabs.IsEmpty ()) return;
    GS::Array<GS::Pair<API_CollisionElem, API_CollisionElem>> collisions;
    API_CollisionDetectionSettings colDetSettings = {};
    colDetSettings.volumeTolerance = 0.000000000001;
    colDetSettings.performSurfaceCheck = true;
    colDetSettings.surfaceTolerance = 0.000000000001;
    if (ACAPI_Element_GetCollisions (zones, allslabs, collisions, colDetSettings) == NoError) {
        for (const auto& pair : collisions.AsConst ()) {
            if (!Class_IsElementFinClass (pair.second.collidedElemGuid, finclassguids)) {
                if (!slabsinzone.ContainsKey (pair.first.collidedElemGuid)) {
                    GS::Array<API_Guid> s;
                    s.Push (pair.second.collidedElemGuid);
                    slabsinzone.Add (pair.first.collidedElemGuid, s);
                } else {
                    slabsinzone.Get (pair.first.collidedElemGuid).Push (pair.second.collidedElemGuid);
                }
            }
        }
    }
}

// -----------------------------------------------------------------------------
// Обработка полов и потолков
// -----------------------------------------------------------------------------
void Floor_FindInOneRoom (const Stories& storyLevels, API_Guid& elGuid, GS::Array<API_Guid>& zoneGuids, OtdRooms& roomsinfo, UnicGUIDByType& guidselementToRead)
{
    GSErrCode err;
    API_Element element = {};
    API_ElementMemo memo = {};
    BNZeroMemory (&memo, sizeof (API_ElementMemo));
    BNZeroMemory (&element, sizeof (API_Element));
    element.header.guid = elGuid;
    err = ACAPI_Element_Get (&element);
    if (err != NoError || !element.header.hasMemo) {
        msg_rep ("Floor_FindInOneRoom err", "ACAPI_Element_Get", err, elGuid);
        return;
    }
    bool flag_find = false;
    double zBottom = GetzPos (element.slab.level + element.slab.offsetFromTop - element.slab.thickness, element.header.floorInd, storyLevels);
    double zUp = zBottom + element.slab.thickness;
    for (API_Guid zoneGuid : zoneGuids) {
        if (roomsinfo.ContainsKey (zoneGuid)) {
            OtdRoom& roominfo = roomsinfo.Get (zoneGuid);
            if (!roominfo.isValid) continue;
            // Проверяем отметки
            bool is_floor = (zBottom <= roominfo.zBottom && zUp >= roominfo.zBottom);
            bool is_ceil = (zBottom <= roominfo.zBottom + roominfo.height);
            if (is_floor) {
                roominfo.floorslab.Push (elGuid);
            } else {
                if (is_ceil) roominfo.ceilslab.Push (elGuid);
            }
            flag_find = true;
        }
    }
    if (flag_find) Param_AddUnicGUIDByType (elGuid, API_SlabID, guidselementToRead);
}

// -----------------------------------------------------------------------------
// Обработка потолков и полов в зоне
// -----------------------------------------------------------------------------
void Floor_Create_All (const Stories& storyLevels, OtdRoom& roominfo, UnicGUIDByType& guidselementToRead, ParamDictElement& paramToRead)
{
    if (!roominfo.has_floor || !roominfo.has_ceil) return;
    if (roominfo.has_floor) {
        roominfo.poly.zBottom = roominfo.zBottom;
        if (!roominfo.floorslab.IsEmpty ()) {
            Floor_Create_One (storyLevels, roominfo.floorInd, roominfo.poly, roominfo.floorslab, roominfo.otdslab, roominfo.otdwall, paramToRead, Floor, roominfo.om_floor);
        } else {
            OtdSlab poly = roominfo.poly;
            poly.type = Floor;
            poly.material = roominfo.om_floor;
            poly.floorInd = roominfo.floorInd;
            roominfo.otdslab.Push (poly);
        }
    }
    if (roominfo.has_ceil) {
        roominfo.poly.zBottom = roominfo.zBottom + roominfo.height_main + roominfo.height_down;
        roominfo.poly.material = roominfo.om_ceil;
        if (!roominfo.ceilslab.IsEmpty ()) {
            Floor_Create_One (storyLevels, roominfo.floorInd, roominfo.poly, roominfo.ceilslab, roominfo.otdslab, roominfo.otdwall, paramToRead, Ceil, roominfo.om_ceil);
        } else {
            OtdSlab poly = roominfo.poly;
            poly.type = Ceil;
            poly.material = roominfo.om_ceil;
            poly.floorInd = roominfo.floorInd;
            roominfo.otdslab.Push (poly);
        }
    }
    roominfo.poly.poly.Clear ();
}

void Floor_Create_One (const Stories& storyLevels, const short& floorInd, OtdSlab& poly, GS::Array<API_Guid>& slabGuids, GS::Array<OtdSlab>& otdslabs, GS::Array<OtdWall>& otdwall, ParamDictElement& paramToRead, TypeOtd type, OtdMaterial& material)
{
    Geometry::Polygon2D roompolygon = poly.poly;
    GSErrCode err;
    API_Element element = {};
    for (API_Guid& slabGuid : slabGuids) {
        API_ElementMemo memo = {};
        BNZeroMemory (&memo, sizeof (API_ElementMemo));
        BNZeroMemory (&element, sizeof (API_Element));
        element.header.guid = slabGuid;
        err = ACAPI_Element_Get (&element);
        if (err != NoError || !element.header.hasMemo) {
            #if defined(TESTING)
            DBprnt ("Floor_Create_One err", "ACAPI_Element_Get slab");
            #endif
            continue;
        }
        err = ACAPI_Element_GetMemo (slabGuid, &memo, APIMemoMask_Polygon);
        if (err != NoError || memo.coords == nullptr) {
            #if defined(TESTING)
            DBprnt ("Floor_Create_One err", "ACAPI_Element_GetMemo slab");
            #endif
            continue;
        }
        Geometry::Polygon2D slabpolygon;
        err = ConstructPolygon2DFromElementMemo (memo, slabpolygon);
        if (err != NoError) {
            msg_rep ("Floor_Create_One err", "ConstructPolygon2DFromElementMemo", err, slabGuid);
            ACAPI_DisposeElemMemoHdls (&memo);
            continue;
        }
        double zBottom = GetzPos (element.slab.level + element.slab.offsetFromTop - element.slab.thickness, element.header.floorInd, storyLevels);
        double zUp = zBottom + element.slab.thickness;
        if (poly.zBottom >= zBottom && poly.zBottom <= zUp) {
            Geometry::Polygon2D reducedroom;
            Geometry::Polygon2D wallpolygon;
            Geometry::MultiPolygon2D resultPolys = roompolygon.Substract (slabpolygon);
            if (!resultPolys.IsEmpty ()) {
                reducedroom = resultPolys.PopLargest ();
                resultPolys.Clear ();
                resultPolys = slabpolygon.Intersect (roompolygon);
                if (!resultPolys.IsEmpty ()) {
                    wallpolygon = resultPolys.PopLargest ();
                    // Добавляем отделочные стенки по контуру
                    Geometry::Polygon2DData polygon2DData;
                    Geometry::InitPolygon2DData (&polygon2DData);
                    Geometry::ConvertPolygon2DToPolygon2DData (polygon2DData, wallpolygon);
                    if (polygon2DData.contourEnds == nullptr) {
                        msg_rep ("Floor_Create_One - error", "polygon2DData.contourEnds == nullptr", NoError, slabGuid);
                        Geometry::FreePolygon2DData (&polygon2DData);
                        continue;
                    }
                    UInt32 begInd = (*polygon2DData.contourEnds)[0] + 1;
                    UInt32 endInd = (*polygon2DData.contourEnds)[1];
                    for (UInt32 k = begInd; k < endInd; k++) {
                        OtdWall wallotd;
                        Point2D begC = { (*polygon2DData.vertices)[k].x, (*polygon2DData.vertices)[k].y };
                        Point2D endC = { (*polygon2DData.vertices)[k + 1].x, (*polygon2DData.vertices)[k + 1].y };
                        if (!(roompolygon.IsCoordOnEdge (begC, nullptr) && roompolygon.IsCoordOnEdge (endC, nullptr))) {
                            wallotd.base_guid = slabGuid;
                            if (type == Floor) {
                                wallotd.height = zUp - poly.zBottom;
                                wallotd.zBottom = poly.zBottom;
                            } else {
                                wallotd.height = poly.zBottom - zBottom;
                                wallotd.zBottom = zBottom;
                            }
                            wallotd.endC = { begC.x, begC.y };
                            wallotd.begC = { endC.x, endC.y };
                            wallotd.material = poly.material;
                            wallotd.base_type = API_SlabID;
                            wallotd.type = type;
                            wallotd.floorInd = floorInd;
                            otdwall.Push (wallotd);
                        }
                    }
                    Geometry::FreePolygon2DData (&polygon2DData);
                    slabpolygon = wallpolygon;
                }
                roompolygon = reducedroom;
            }
        }
        if (zBottom <= poly.zBottom) {
            OtdSlab otdslab;
            otdslab.poly = slabpolygon;
            otdslab.base_type = API_SlabID;
            otdslab.base_guid = slabGuid;
            if (type == Ceil) {
                otdslab.zBottom = zBottom - otd_thickness;
            } else {
                otdslab.zBottom = zUp + otd_thickness;
            }
            otdslab.floorInd = floorInd;
            otdslab.type = type;
            otdslab.material = poly.material;
            otdslabs.Push (otdslab);
        }
        ACAPI_DisposeElemMemoHdls (&memo);
    }
    OtdSlab otdslab = poly;
    otdslab.type = type;
    if (type == Ceil) {
        otdslab.zBottom -= otd_thickness;
    } else {
        otdslab.zBottom += otd_thickness;
    }
    otdslab.floorInd = floorInd;
    otdslab.poly = roompolygon;
    roompolygon.Clear ();
    otdslabs.Push (otdslab);
}

void Param_ToParamDict (ParamDictValue& paramDict, ReadParams& zoneparams)
{
    if (zoneparams.IsEmpty ()) {
        return;
    }
    for (auto& p : zoneparams) {
        #if defined(AC_28)
        ReadParam param = p.value;
        #else
        ReadParam param = *p.value;
        #endif
        for (UInt32 i = 0; i < param.rawnames.GetSize (); ++i) {
            ParamHelpers::AddValueToParamDictValue (paramDict, param.rawnames[i]);
        }
    }
}

ReadParams Param_GetForWindowParams (ParamDictValue& propertyParams)
{
    ReadParams zoneparams = {};
    ReadParam zoneparam = {};
    GS::UniString zoneparam_name = "";

    zoneparam_name = "frame";
    zoneparam.rawnames.Push ("{@gdl:gs_frame_thk}");
    zoneparams.Add (zoneparam_name, zoneparam);
    zoneparam.rawnames.Clear ();

    zoneparam_name = "sill";
    zoneparam.rawnames.Push ("{@gdl:gs_wido_sill}");
    zoneparams.Add (zoneparam_name, zoneparam);
    zoneparam.rawnames.Clear ();

    zoneparam_name = "useWallFinishSkin";
    zoneparam.rawnames.Push ("{@gdl:gs_usewallfinishskin}");
    zoneparams.Add (zoneparam_name, zoneparam);
    zoneparam.rawnames.Clear ();

    zoneparam_name = "maxPlasterThk";
    zoneparam.rawnames.Push ("{@gdl:gs_maxplasterthk}");
    zoneparams.Add (zoneparam_name, zoneparam);
    zoneparam.rawnames.Clear ();

    zoneparam_name = "AutoTurnIn";
    zoneparam.rawnames.Push ("{@gdl:gs_bautoturnin}");
    zoneparams.Add (zoneparam_name, zoneparam);
    zoneparam.rawnames.Clear ();

    zoneparam_name = "bOverIn";
    zoneparam.rawnames.Push ("{@gdl:gs_boverin}");
    zoneparams.Add (zoneparam_name, zoneparam);
    zoneparam.rawnames.Clear ();

    zoneparam_name = "plaster_show_3D";
    zoneparam.rawnames.Push ("{@gdl:gs_turn_plaster_show_3d}");
    zoneparams.Add (zoneparam_name, zoneparam);
    zoneparam.rawnames.Clear ();

    zoneparam_name = "plaster_show_2D";
    zoneparam.rawnames.Push ("{@gdl:gs_turn_plaster_dim_2d}");
    zoneparams.Add (zoneparam_name, zoneparam);
    zoneparam.rawnames.Clear ();

    Param_Property_FindInParams (propertyParams, zoneparams);
    return zoneparams;
}

// -----------------------------------------------------------------------------
// Подготовка словаря с параметрами для чтения из стен/колонн/балок
// -----------------------------------------------------------------------------
void Param_GetForBase (ParamDictValue& propertyParams, ParamDictValue& paramDict, ParamValue& param_composite)
{
    // Поиск свойств со включением слоя отделки и имени
    GS::UniString propdesc_onoff = "some_stuff_fin_onoff";
    GS::UniString propdesc_desc = "some_stuff_fin_description";
    GS::UniString propdesc_fav = "some_stuff_fin_favorite_name";
    GS::UniString rawName_onoff = "";
    GS::UniString rawName_desc = "";
    GS::UniString rawName_fav = "";
    for (auto& cItt : propertyParams) {
        #if defined(AC_28)
        ParamValue param = cItt.value;
        #else
        ParamValue param = *cItt.value;
        #endif
        if (param.definition.description.Contains (propdesc_onoff)) {
            rawName_onoff = param.rawName;
            continue;
        }
        if (param.definition.description.Contains (propdesc_desc)) {
            rawName_desc = param.rawName;
            continue;
        }
        if (param.definition.description.Contains (propdesc_fav)) {
            rawName_fav = param.rawName;
            continue;
        }
        if (!rawName_onoff.IsEmpty () && !rawName_desc.IsEmpty () && !rawName_fav.IsEmpty ()) break;
    }
    // Подготавливаем свойство для чтения из слоёв многослойки
    GS::UniString rawName = "{@material:layers,20}";
    param_composite.rawName = rawName;
    param_composite.fromMaterial = true;
    if (!rawName_onoff.IsEmpty () && !rawName_desc.IsEmpty ()) {
        param_composite.val.uniStringValue = "l[" + rawName_onoff + ":" + rawName_desc;
    } else {
        param_composite.val.uniStringValue = "l[1:%BuildingMaterialProperties/Building Material Name%";
    }
    param_composite.val.uniStringValue.Append ("]f[");
    if (!rawName_fav.IsEmpty ()) param_composite.val.uniStringValue.Append (rawName_fav);
    param_composite.val.uniStringValue.Append ("] %nosyncname%");
    if (ParamHelpers::ParseParamNameMaterial (param_composite.val.uniStringValue, paramDict)) {
        for (UInt32 inx = 0; inx < 20; inx++) {
            ParamHelpers::AddValueToParamDictValue (paramDict, "@property:sync_name" + GS::UniString::Printf ("%d", inx));
        }
        ParamHelpers::CompareParamDictValue (propertyParams, paramDict);
    }
    ParamHelpers::AddParamValue2ParamDict (APINULLGuid, param_composite, paramDict);
}

ReadParams Param_GetForZoneParams (ParamDictValue& propertyParams)
{
    ReadParams zoneparams = {};
    ReadParam zoneparam = {};
    GS::UniString zoneparam_name = "";

    // Основная отделка стен
    zoneparam_name = "om_main";
    zoneparam.rawnames.Push ("some_stuff_fin_main_material");
    zoneparam.rawnames.Push ("{@gdl:stwallmat}");
    zoneparam.rawnames.Push ("{@gdl:votw}");
    zoneparams.Add (zoneparam_name, zoneparam);
    zoneparam.rawnames.Clear ();

    // Высота до подвесного потолка
    zoneparam_name = "height_main";
    zoneparam.rawnames.Push ("some_stuff_fin_main_height");
    zoneparam.rawnames.Push ("{@gdl:hroom_pot}");
    zoneparams.Add (zoneparam_name, zoneparam);
    zoneparam.rawnames.Clear ();

    // Отделка стен выше потолка
    zoneparam_name = "om_up";
    zoneparam.rawnames.Push ("some_stuff_fin_up_material");
    zoneparam.rawnames.Push ("{@gdl:votw2}");
    zoneparams.Add (zoneparam_name, zoneparam);
    zoneparam.rawnames.Clear ();

    // Отделка низа стен/колонн
    zoneparam_name = "om_down";
    zoneparam.rawnames.Push ("some_stuff_fin_down_material");
    zoneparam.rawnames.Push ("{@gdl:stwalldownmat}");
    zoneparam.rawnames.Push ("{@gdl:votp}");
    zoneparams.Add (zoneparam_name, zoneparam);
    zoneparam.rawnames.Clear ();

    // Высота нижних панелей
    zoneparam_name = "height_down";
    zoneparam.rawnames.Push ("some_stuff_fin_down_height");
    zoneparam.rawnames.Push ("{@gdl:hpan}");
    zoneparam.rawnames.Push ("{@gdl:z17}");
    zoneparams.Add (zoneparam_name, zoneparam);
    zoneparam.rawnames.Clear ();

    // Наличие нижней отделки (химера)
    zoneparam_name = "him_has_height_down";
    zoneparam.rawnames.Push ("{@gdl:busewalldown}");
    zoneparams.Add (zoneparam_name, zoneparam);
    zoneparam.rawnames.Clear ();

    // Высота нижних панелей (химера)
    zoneparam_name = "him_height_down";
    zoneparam.rawnames.Push ("{@gdl:walldownhigh}");
    zoneparams.Add (zoneparam_name, zoneparam);
    zoneparam.rawnames.Clear ();

    // Отделка колонн
    zoneparam_name = "om_column";
    zoneparam.rawnames.Push ("some_stuff_fin_column_material");
    zoneparam.rawnames.Push ("{@gdl:stcolumnmat}");
    zoneparam.rawnames.Push ("{@gdl:votc}");
    zoneparams.Add (zoneparam_name, zoneparam);
    zoneparam.rawnames.Clear ();

    // Отделка потолка
    zoneparam_name = "om_ceil";
    zoneparam.rawnames.Push ("some_stuff_fin_ceil_material");
    zoneparam.rawnames.Push ("{@gdl:stupmat}");
    zoneparam.rawnames.Push ("{@gdl:vots}");
    zoneparams.Add (zoneparam_name, zoneparam);
    zoneparam.rawnames.Clear ();

    // Отделка потолка - запись результатов
    zoneparam_name = "om_ceil.rawname";
    zoneparam.rawnames.Push ("some_stuff_fin_ceil_result");
    zoneparams.Add (zoneparam_name, zoneparam);
    zoneparam.rawnames.Clear ();

    // Отделка откосов - запись результатов
    zoneparam_name = "om_reveals.rawname";
    zoneparam.rawnames.Push ("some_stuff_fin_reveal_result");
    zoneparam.rawnames.Push ("some_stuff_fin_reveals_result");
    zoneparams.Add (zoneparam_name, zoneparam);
    zoneparam.rawnames.Clear ();

    // Отделка выше потолка - запись результатов
    zoneparam_name = "om_up.rawname";
    zoneparam.rawnames.Push ("some_stuff_fin_up_result");
    zoneparams.Add (zoneparam_name, zoneparam);
    zoneparam.rawnames.Clear ();

    // Отделка стен - запись результатов
    zoneparam_name = "om_main.rawname";
    zoneparam.rawnames.Push ("some_stuff_fin_main_result");
    zoneparams.Add (zoneparam_name, zoneparam);
    zoneparam.rawnames.Clear ();

    // Отделка низа стен/колонн - запись результатов
    zoneparam_name = "om_down.rawname";
    zoneparam.rawnames.Push ("some_stuff_fin_down_result");
    zoneparams.Add (zoneparam_name, zoneparam);
    zoneparam.rawnames.Clear ();

    // Отделка низа колонн - запись результатов
    zoneparam_name = "om_column.rawname";
    zoneparam.rawnames.Push ("some_stuff_fin_column_result");
    zoneparams.Add (zoneparam_name, zoneparam);
    zoneparam.rawnames.Clear ();

    // Создание всех элементов
    zoneparam_name = "create_all_elements";
    zoneparam.rawnames.Push ("some_stuff_fin_create_elements");
    zoneparams.Add (zoneparam_name, zoneparam);
    zoneparam.rawnames.Clear ();

    // Создание отделки колонн
    zoneparam_name = "create_column_elements";
    zoneparam.rawnames.Push ("some_stuff_fin_create_column");
    zoneparams.Add (zoneparam_name, zoneparam);
    zoneparam.rawnames.Clear ();

    // Создание отделки стен
    zoneparam_name = "create_wall_elements";
    zoneparam.rawnames.Push ("some_stuff_fin_create_wall");
    zoneparams.Add (zoneparam_name, zoneparam);
    zoneparam.rawnames.Clear ();

    // Создание пола
    zoneparam_name = "create_ceil_elements";
    zoneparam.rawnames.Push ("some_stuff_fin_create_ceil");
    zoneparams.Add (zoneparam_name, zoneparam);
    zoneparam.rawnames.Clear ();

    // Создание пола
    zoneparam_name = "create_floor_elements";
    zoneparam.rawnames.Push ("some_stuff_fin_create_floor");
    zoneparams.Add (zoneparam_name, zoneparam);
    zoneparam.rawnames.Clear ();

    // Создание откосов
    zoneparam_name = "create_reveal_elements";
    zoneparam.rawnames.Push ("some_stuff_fin_create_reveal");
    zoneparams.Add (zoneparam_name, zoneparam);
    zoneparam.rawnames.Clear ();

    Param_Property_FindInParams (propertyParams, zoneparams);
    return zoneparams;
}

void Param_Property_FindInParams (ParamDictValue& propertyParams, ReadParams& zoneparams)
{
    if (zoneparams.IsEmpty ()) {
        return;
    }
    for (auto& p : zoneparams) {
        #if defined(AC_28)
        ReadParam& param = p.value;
        GS::UniString name = p.key;
        #else
        ReadParam& param = *p.value;
        GS::UniString name = *p.key;
        #endif
        GS::Array<GS::UniString> valid_rawnames; // список проверенных имён параметров
        for (const GS::UniString& param_name : param.rawnames) {
            if (param_name.Contains ("{@")) {
                // Если это gdl парметр - добавляем
                valid_rawnames.Push (param_name);
            } else {
                bool flag_find = false;
                for (auto& cItt : propertyParams) {
                    #if defined(AC_28)
                    ParamValue parameters = cItt.value;
                    #else
                    ParamValue parameters = *cItt.value;
                    #endif
                    if (parameters.definition.description.Contains (param_name)) {
                        // В дланном случае нам нужно только имя свойства, считывать его не нужно
                        if (name.Contains ("rawname")) {
                            param.val.uniStringValue = parameters.rawName;
                            param.isValid = true;
                        }
                        valid_rawnames.Push (parameters.rawName);
                        break;
                    }
                }
            }
        }
        param.rawnames = valid_rawnames;
    }
}

bool Param_Property_Read (const API_Guid& elGuid, ParamDictElement& paramToRead, ReadParams& zoneparams)
{
    if (zoneparams.IsEmpty ()) {
        return false;
    }
    GS::UniString param_name = "";
    bool flag = false;
    // Прочитанные параметры базового компонента
    ParamDictValue& baseparam = paramToRead.Get (elGuid);
    for (auto& p : zoneparams) {
        #if defined(AC_28)
        ReadParam& param = p.value;
        #else
        ReadParam& param = *p.value;
        #endif
        if (param.isValid) continue; // Пропуск уже прочитанных исвойств с именами свойств для записи
        for (UInt32 i = 0; i < param.rawnames.GetSize (); ++i) {
            param_name = param.rawnames[i];
            if (!baseparam.ContainsKey (param_name)) continue;
            if (baseparam.Get (param_name).isValid) {
                flag = true;
                param.isValid = true;
                param.val = baseparam.Get (param_name).val;
                if (baseparam.Get (param_name).fromProperty) {
                    param.val.type = API_PropertyStringValueType;
                }
                break;
            }
        }
    }
    return flag;
}

void Param_Material_Get (GS::HashTable<GS::UniString, GS::Int32>& material_dict, ParamValueData& val)
{
    API_AttrTypeID type = API_MaterialID;
    if (val.type == API_PropertyStringValueType) {
        if (material_dict.ContainsKey (val.uniStringValue)) {
            val.intValue = material_dict.Get (val.uniStringValue);
        } else {
            API_AttributeIndex attribinx;
            #if defined(AC_27) || defined(AC_28)
            attribinx = ACAPI_CreateAttributeIndex (val.intValue);
            #else
            attribinx = val.intValue;
            #endif
            API_AttributeIndexFindByName (val.uniStringValue, type, attribinx);
            #if defined(AC_27) || defined(AC_28)
            val.intValue = attribinx.ToInt32_Deprecated ();
            #else
            val.intValue = attribinx;
            #endif
            material_dict.Add (val.uniStringValue, val.intValue);
        }
    }
}

// -----------------------------------------------------------------------------
// Запись прочитанных свойств в зону
// -----------------------------------------------------------------------------
void Param_SetToRooms (GS::HashTable<GS::UniString, GS::Int32>& material_dict, OtdRoom& roominfo, ParamDictElement& paramToRead, ReadParams readparams)
{
    API_Guid base_guid = roominfo.zone_guid;
    if (!Param_Property_Read (base_guid, paramToRead, readparams)) {
        #if defined(TESTING)
        DBprnt ("Param_SetToRooms err", "!Param_Property_Read");
        #endif
        return;
    }
    GS::UniString param_name = "";
    ParamValueData val = {};
    param_name = "om_column";
    if (readparams.ContainsKey (param_name)) {
        if (readparams.Get (param_name).isValid) {
            val = readparams.Get (param_name).val;
            Param_Material_Get (material_dict, val);
            val.uniStringValue.Trim ();
            val.uniStringValue = "0&#& " + val.uniStringValue;
            roominfo.om_column.smaterial = val.uniStringValue;
            roominfo.om_column.material = val.intValue;
        }
    }
    param_name = "om_down";
    if (readparams.ContainsKey (param_name)) {
        if (readparams.Get (param_name).isValid) {
            val = readparams.Get (param_name).val;
            Param_Material_Get (material_dict, val);
            val.uniStringValue.Trim ();
            val.uniStringValue = "0&#& " + val.uniStringValue;
            roominfo.om_down.material = val.intValue;
            roominfo.om_down.smaterial = val.uniStringValue;
        }
    }
    param_name = "om_main";
    if (readparams.ContainsKey (param_name)) {
        if (readparams.Get (param_name).isValid) {
            val = readparams.Get (param_name).val;
            Param_Material_Get (material_dict, val);
            val.uniStringValue.Trim ();
            val.uniStringValue = "0&#& " + val.uniStringValue;
            roominfo.om_main.material = val.intValue;
            roominfo.om_main.smaterial = val.uniStringValue;
        }
    }
    param_name = "om_ceil";
    if (readparams.ContainsKey (param_name)) {
        if (readparams.Get (param_name).isValid) {
            val = readparams.Get (param_name).val;
            Param_Material_Get (material_dict, val);
            val.uniStringValue.Trim ();
            val.uniStringValue = "0&#& " + val.uniStringValue;
            roominfo.om_ceil.material = val.intValue;
            roominfo.om_ceil.smaterial = val.uniStringValue;
        }
    }
    param_name = "om_up";
    if (readparams.ContainsKey (param_name)) {
        if (readparams.Get (param_name).isValid) {
            val = readparams.Get (param_name).val;
            Param_Material_Get (material_dict, val);
            val.uniStringValue.Trim ();
            val.uniStringValue = "0&#& " + val.uniStringValue;
            roominfo.om_up.material = val.intValue;
            roominfo.om_up.smaterial = val.uniStringValue;
        }
    }

    bool has_height_down = false;
    param_name = "him_has_height_down";
    if (readparams.ContainsKey (param_name)) {
        if (readparams.Get (param_name).isValid) {
            val = readparams.Get (param_name).val;
            bool him_has_height_down = val.boolValue;
            param_name = "height_down";
            if (readparams.ContainsKey (param_name)) {
                if (readparams.Get (param_name).isValid) {
                    val = readparams.Get (param_name).val;
                    roominfo.height_down = val.doubleValue * him_has_height_down;
                    has_height_down = true;
                }
            }
        }
    }

    if (!has_height_down) {
        param_name = "height_down";
        if (readparams.ContainsKey (param_name)) {
            if (readparams.Get (param_name).isValid) {
                val = readparams.Get (param_name).val;
                roominfo.height_down = val.doubleValue;
            }
        }
    }
    param_name = "height_main";
    if (readparams.ContainsKey (param_name)) {
        if (readparams.Get (param_name).isValid) {
            val = readparams.Get (param_name).val;
            roominfo.height_main = val.doubleValue;
        }
    }
    param_name = "om_ceil.rawname";
    if (readparams.ContainsKey (param_name)) {
        if (readparams.Get (param_name).isValid) {
            val = readparams.Get (param_name).val;
            roominfo.om_ceil.rawname = val.uniStringValue;
        }
    }
    param_name = "om_reveals.rawname";
    if (readparams.ContainsKey (param_name)) {
        if (readparams.Get (param_name).isValid) {
            val = readparams.Get (param_name).val;
            roominfo.om_reveals.rawname = val.uniStringValue;
        }
    }
    param_name = "om_up.rawname";
    if (readparams.ContainsKey (param_name)) {
        if (readparams.Get (param_name).isValid) {
            val = readparams.Get (param_name).val;
            roominfo.om_up.rawname = val.uniStringValue;
        }
    }
    param_name = "om_main.rawname";
    if (readparams.ContainsKey (param_name)) {
        if (readparams.Get (param_name).isValid) {
            val = readparams.Get (param_name).val;
            roominfo.om_main.rawname = val.uniStringValue;
        }
    }
    param_name = "om_down.rawname";
    if (readparams.ContainsKey (param_name)) {
        if (readparams.Get (param_name).isValid) {
            val = readparams.Get (param_name).val;
            roominfo.om_down.rawname = val.uniStringValue;
        }
    }
    param_name = "om_column.rawname";
    if (readparams.ContainsKey (param_name)) {
        if (readparams.Get (param_name).isValid) {
            val = readparams.Get (param_name).val;
            roominfo.om_column.rawname = val.uniStringValue;
        }
    }
    bool find_create_all_elements = false;
    param_name = "create_all_elements";
    if (readparams.ContainsKey (param_name)) {
        if (readparams.Get (param_name).isValid) {
            val = readparams.Get (param_name).val;
            find_create_all_elements = true;
            roominfo.create_all_elements = val.boolValue;
        }
    }
    param_name = "create_column_elements";
    if (readparams.ContainsKey (param_name)) {
        if (readparams.Get (param_name).isValid) {
            val = readparams.Get (param_name).val;
            roominfo.create_column_elements = val.boolValue;
        }
    }
    param_name = "create_wall_elements";
    if (readparams.ContainsKey (param_name)) {
        if (readparams.Get (param_name).isValid) {
            val = readparams.Get (param_name).val;
            roominfo.create_wall_elements = val.boolValue;
        }
    }
    param_name = "create_floor_elements";
    if (readparams.ContainsKey (param_name)) {
        if (readparams.Get (param_name).isValid) {
            val = readparams.Get (param_name).val;
            roominfo.create_floor_elements = val.boolValue;
        }
    }
    param_name = "create_ceil_elements";
    if (readparams.ContainsKey (param_name)) {
        if (readparams.Get (param_name).isValid) {
            val = readparams.Get (param_name).val;
            roominfo.create_ceil_elements = val.boolValue;
        }
    }
    param_name = "create_reveal_elements";
    if (readparams.ContainsKey (param_name)) {
        if (readparams.Get (param_name).isValid) {
            val = readparams.Get (param_name).val;
            roominfo.create_reveal_elements = val.boolValue;
        }
    }
    if (roominfo.create_all_elements) {
        roominfo.create_ceil_elements = true; // Создавать элементы отделки потолка
        roominfo.create_wall_elements = true; // Создавать элементы отделки стен
        roominfo.create_column_elements = true; // Создавать элементы отделки колонн
        roominfo.create_reveal_elements = true; // Создавать элементы отделки откосов
        roominfo.create_floor_elements = true; // Создавать элементы отделки откосов
    } else {
        if (find_create_all_elements) {
            roominfo.create_ceil_elements = false; // Создавать элементы отделки потолка
            roominfo.create_wall_elements = false; // Создавать элементы отделки стен
            roominfo.create_column_elements = false; // Создавать элементы отделки колонн
            roominfo.create_reveal_elements = false; // Создавать элементы отделки откосов
            roominfo.create_floor_elements = false; // Создавать элементы отделки откосов
        }
    }
    // Заполнение непрочитанных
    // Высоты
    if (roominfo.height_main > roominfo.height) roominfo.height_main = roominfo.height;
    if (roominfo.height_main < 0.1) {
        roominfo.height_main = roominfo.height - roominfo.height_down;
        roominfo.height_up = 0;
    } else {
        roominfo.height_up = roominfo.height - roominfo.height_main;
        roominfo.height_main = roominfo.height_main - roominfo.height_down;
    }
    if (roominfo.height < 0.0001) roominfo.isValid = false;
}

// -----------------------------------------------------------------------------
// Запись прочитанных свойств в отделочные стены
// -----------------------------------------------------------------------------
void Param_SetToBase (const API_Guid& base_guid, const bool& base_flipped, GS::Array<ParamValueComposite>& otdcpmpoosite, ParamDictElement& paramToRead, ParamValue& param_composite, GS::UniString& fav_name)
{
    if (!paramToRead.ContainsKey (base_guid)) {
        #if defined(TESTING)
        DBprnt ("Param_SetToBase err", "!paramToRead.ContainsKey(base_guid)");
        #endif
        return;
    }
    // Прочитанные параметры базового компонента
    ParamDictValue& baseparam = paramToRead.Get (base_guid);
    // Состав базового компонента
    if (!baseparam.ContainsKey (param_composite.rawName)) return;
    ParamValue& base_composite = baseparam.Get (param_composite.rawName);
    //TODO Дописать определение вывода слоя в ведомость отделки исходя из свойств
    Int32 ncomp = base_composite.composite.GetSize ();
    if (ncomp == 0) return;
    bool flag_core = false;
    if (base_flipped) {
        for (Int32 j = ncomp - 1; j >= 0; j--) {
            if (!flag_core) {
                if (base_composite.composite[j].val.Contains ("l[1:")) {
                    otdcpmpoosite.Push (base_composite.composite[j]);
                }
                if (base_composite.composite[j].structype == APICWallComp_Core) flag_core = true;
            }
        }
    } else {
        for (Int32 j = 0; j < ncomp; j++) {
            if (!flag_core) {
                if (base_composite.composite[j].val.Contains ("l[1:")) {
                    otdcpmpoosite.Push (base_composite.composite[j]);
                }
                if (base_composite.composite[j].structype == APICWallComp_Core) flag_core = true;
            }
        }
    }
    fav_name.Clear ();
    for (UInt32 j = 0; j < otdcpmpoosite.GetSize (); j++) {
        GS::UniString v = otdcpmpoosite[j].val;
        if (v.Contains ("f[]")) {
            v = v.GetPrefix (v.GetLength () - 5);
            v = v.GetSuffix (v.GetLength () - 4);
        } else {
            v.ReplaceAll ("f[", "@");
            GS::UniString fav_part = v.GetSubstring ('@', ']', 0);
            fav_part.Trim ();
            if (!fav_part.IsEmpty ()) {
                if (fav_name.IsEmpty ()) {
                    fav_name = fav_part;
                } else {
                    fav_name.Append ("_");
                    fav_name.Append (fav_part);
                }
            }
            v.ReplaceAll ("l[1:", "@");
            v = v.GetSubstring ('@', ']', 0);
        }
        otdcpmpoosite[j].val = v;
    }
    if (!fav_name.IsEmpty ()) fav_name.SetToLowerCase ();
}

// -----------------------------------------------------------------------------
// Задание прочитанных параметров для окон
// -----------------------------------------------------------------------------
void Param_SetToWindows (OtdOpening& op, ParamDictElement& paramToRead, ReadParams readparams, const OtdWall& otdw)
{
    API_Guid base_guid = op.base_guid;
    if (!paramToRead.ContainsKey (base_guid)) {
        #if defined(TESTING)
        DBprnt ("Param_SetToWindows err", "!paramToRead.ContainsKey(base_guid)");
        #endif
        return;
    }
    op.base_reveal_width = otdw.base_th;
    if (!Param_Property_Read (base_guid, paramToRead, readparams)) {
        #if defined(TESTING)
        DBprnt ("Param_SetToWindows err", "!Param_Property_Read");
        #endif
        return;
    }
    GS::UniString param_name = "";

    param_name = "frame";
    if (!readparams.ContainsKey (param_name)) return; if (!readparams.Get (param_name).isValid) return;
    double frame = readparams.Get (param_name).val.doubleValue;

    param_name = "sill";
    if (!readparams.ContainsKey (param_name)) return; if (!readparams.Get (param_name).isValid) return;
    double sill = readparams.Get (param_name).val.doubleValue;

    param_name = "plaster_show_3D";
    if (!readparams.ContainsKey (param_name)) return; if (!readparams.Get (param_name).isValid) return;
    bool plaster_show_3D = readparams.Get (param_name).val.boolValue;

    param_name = "plaster_show_2D";
    if (!readparams.ContainsKey (param_name)) return; if (!readparams.Get (param_name).isValid) return;
    bool plaster_show_2D = readparams.Get (param_name).val.boolValue;

    param_name = "AutoTurnIn";
    if (!readparams.ContainsKey (param_name)) return; if (!readparams.Get (param_name).isValid) return;
    bool AutoTurnIn = readparams.Get (param_name).val.boolValue;

    param_name = "bOverIn";
    if (!readparams.ContainsKey (param_name)) return; if (!readparams.Get (param_name).isValid) return;
    int bOverIn = readparams.Get (param_name).val.boolValue;

    // Расчёт ширины откосы
    op.base_reveal_width = op.base_reveal_width - sill - frame;
    // Поправка на поворот отделки
    if (plaster_show_3D && plaster_show_2D) {
        double max_plaster_th = 0;
        UInt32 nsl = 0;
        if (!AutoTurnIn) {
            nsl = bOverIn;
        } else {
            nsl = otdw.base_composite.GetSize () - 1;
        }
        if (nsl > 0) {
            nsl = nsl - 1;
            if (nsl >= 0) {
                for (UInt32 j = 0; j <= nsl; j++) {
                    max_plaster_th += otdw.base_composite[j].fillThick;
                }
                op.width -= max_plaster_th * 2;
                op.height -= max_plaster_th;
            }
        }
    }
}

// -----------------------------------------------------------------------------
// Полчение полигона зоны (в том числе стен, колонн)
// -----------------------------------------------------------------------------
#if defined(AC_27) || defined(AC_28) 
void RoomReductionPolyProc (const API_RoomReductionPolyType* roomRed)
#else
static void	__ACENV_CALL RoomRedProc (const API_RoomReductionPolyType* roomRed)
#endif
{
    if (reducededges == nullptr) {
        #if defined(TESTING)
        DBprnt ("RoomRedProc err", "reducededges == nullptr");
        #endif
        return;
    }
    if (roomRed->nCoords < 4 || roomRed->coords == nullptr || roomRed->subPolys == nullptr) {
        #if defined(TESTING)
        DBprnt ("RoomRedProc err", "roomRed->coords == nullptr");
        #endif
        return;
    }
    Point2D begC = { 0,0 }; Point2D endC = { 0, 0 };
    Sector roomedge = { begC, endC };
    GS::Array<Sector> roomedges;
    for (Int32 j = 1; j <= roomRed->nSubPolys; j++) {
        UInt32 begInd = (*roomRed->subPolys)[j - 1] + 1;
        UInt32 endInd = (*roomRed->subPolys)[j];
        for (UInt32 k = begInd; k < endInd; k++) {
            begC = { (*roomRed->coords)[k].x, (*roomRed->coords)[k].y };
            endC = { (*roomRed->coords)[k + 1].x, (*roomRed->coords)[k + 1].y };
            roomedge = { begC, endC };
            roomedges.Push (roomedge);
        }
    }
    switch (roomRed->type) {
        case APIRoomReduction_Rest:
            reducededges->restedges.Append (roomedges);
            break;
        case APIRoomReduction_Wall:
            reducededges->walledges.Append (roomedges);
            break;
        case APIRoomReduction_Column:
            reducededges->columnedges.Append (roomedges);
            break;
        case APIRoomReduction_Hatch:
            break;
        case APIRoomReduction_Gable:
            reducededges->gableedges.Append (roomedges);
            break;
        default:
            break;
    }
    BMKillHandle ((GSHandle*) &roomRed->coords);
    BMKillHandle ((GSHandle*) &roomRed->subPolys);
    BMKillHandle ((GSHandle*) &roomRed->arcs);
    return;
}		// RoomRedProc

// -----------------------------------------------------------------------------
// Получение очищенного полигона зоны, включая стены, колонны
// -----------------------------------------------------------------------------
void Edges_GetFromRoom (const API_ElementMemo& zonememo, API_Element& zoneelement, GS::Array<Sector>& walledges, GS::Array<Sector>& columnedges, GS::Array<Sector>& restedges, GS::Array<Sector>& gableedges)
{
    RoomEdges rdges;
    reducededges = &rdges;
    GSErrCode err = NoError;
    #if defined(AC_27) || defined(AC_28) 
    err = ACAPI_Element_RoomReductions (&zoneelement.header.guid, RoomReductionPolyProc);
    #else
    err = ACAPI_Database (APIDb_RoomReductionsID, &zoneelement.header.guid, (void*) (GS::IntPtr) RoomRedProc);
    #endif
    if (err != NoError) {
        #if defined(TESTING)
        DBprnt ("Edges_GetFromRoom err", "APIDb_RoomReductionsID");
        #endif
        return;
    }
    Point2D begC = { 0,0 }; Point2D endC = { 0, 0 };
    Sector roomedge = { begC, endC };
    GS::Array<Sector> roomedges;
    for (Int32 j = 1; j <= zoneelement.zone.poly.nSubPolys; j++) {
        UInt32 begInd = (*zonememo.pends)[j - 1] + 1;
        UInt32 endInd = (*zonememo.pends)[j];
        for (UInt32 k = begInd; k < endInd; k++) {
            begC = { (*zonememo.coords)[k].x, (*zonememo.coords)[k].y };
            endC = { (*zonememo.coords)[k + 1].x, (*zonememo.coords)[k + 1].y };
            roomedge = { begC, endC };
            roomedges.Push (roomedge);
        }
        // При подсчёте полигонов архикад выдаёт сдвижку на 1 для каждого субполигона
        begC = { 0,0 }; endC = { 0, 0 };
        roomedge = { begC, endC };
        roomedges.Push (roomedge);
    }
    GS::Array<Sector> edges;
    if (!rdges.columnedges.IsEmpty ()) {
        for (UInt32 j = 0; j < rdges.columnedges.GetSize (); j++) {
            Sector tedge = rdges.columnedges[j];
            if (Edge_FindEdge (tedge, rdges.restedges)) edges.Push (tedge);
        }
        columnedges = edges;
        edges.Clear ();
    }
    if (!rdges.walledges.IsEmpty ()) {
        for (UInt32 j = 0; j < rdges.walledges.GetSize (); j++) {
            Sector tedge = rdges.walledges[j];
            if (Edge_FindEdge (tedge, rdges.restedges)) edges.Push (tedge);
        }
        walledges = edges;
        edges.Clear ();
    }
    if (!rdges.gableedges.IsEmpty ()) {
        for (UInt32 j = 0; j < rdges.gableedges.GetSize (); j++) {
            Sector tedge = rdges.gableedges[j];
            if (Edge_FindEdge (tedge, rdges.restedges)) edges.Push (tedge);
        }
        gableedges = edges;
        edges.Clear ();
    }
    restedges = roomedges;
}

// -----------------------------------------------------------------------------
// Убираем задвоение Guid зон у элементов
// -----------------------------------------------------------------------------
void ClearZoneGUID (UnicElementByType& elementToRead, GS::Array<API_ElemTypeID>& typeinzone)
{
    for (const API_ElemTypeID& typeelem : typeinzone) {
        if (elementToRead.ContainsKey (typeelem)) {
            for (UnicElement::PairIterator cIt = elementToRead.Get (typeelem).EnumeratePairs (); cIt != NULL; ++cIt) {
                #if defined(AC_28)
                API_Guid guid = cIt->key;
                GS::Array<API_Guid> zoneGuids_ = cIt->value;
                #else
                API_Guid guid = *cIt->key;
                GS::Array<API_Guid> zoneGuids_ = *cIt->value;
                #endif
                UnicGuid zoneGuidsd;
                GS::Array<API_Guid> zoneGuids;
                for (API_Guid zoneGuid : zoneGuids_) {
                    if (!zoneGuidsd.ContainsKey (zoneGuid)) zoneGuidsd.Add (zoneGuid, true);
                }
                for (UnicGuid::PairIterator cIt = zoneGuidsd.EnumeratePairs (); cIt != NULL; ++cIt) {
                    #if defined(AC_28)
                    API_Guid zoneGuid = cIt->key;
                    #else
                    API_Guid zoneGuid = *cIt->key;
                    #endif
                    zoneGuids.Push (zoneGuid);
                }
                elementToRead.Get (typeelem).Set (guid, zoneGuids);
            }
        }
    }
    return;
}

// -----------------------------------------------------------------------------
// Создание стенок для откосов одного проёма
// -----------------------------------------------------------------------------
void OpeningReveals_Create_One (const MatarialToFavoriteDict& favdict, GS::Array<OtdSlab>& otdslabs, const OtdWall& otdw, OtdOpening& op, const Geometry::Vector2<double>& walldir_perp, GS::Array<OtdWall>& opw, double& otd_zBottom, double& otd_height_down, double& otd_height_main, double& otd_height_up, double& otd_height, OtdMaterial& om_main, OtdMaterial& om_up, OtdMaterial& om_down,
        OtdMaterial& om_reveals, OtdMaterial& om_column, OtdMaterial& om_floor, OtdMaterial& om_ceil, OtdMaterial& om_zone)
{
    if (op.base_reveal_width < min_dim) return;
    bool is_upper_wall = false; // Проём заканчивается выше стены, построение верхнего откоса не требуется
    if ((op.zBottom + op.height) > (otdw.zBottom + otdw.height)) is_upper_wall = true;
    double zDup = fmin (op.zBottom + op.height, otdw.zBottom + otdw.height);
    double zDown = fmax (op.zBottom, otdw.zBottom);
    double height = fmin (op.height, zDup - zDown);
    if (height < min_dim) return;
    Point2D begedge = { 0,0 }; Point2D endedge = { 0,0 };
    Point2D begedge_up = { 0,0 }; Point2D endedge_up = { 0,0 };
    Sector walledge = { begedge , endedge };
    // Строим перпендикулярные стенки к окну
    double dx = -otdw.endC.x + otdw.begC.x;
    double dy = -otdw.endC.y + otdw.begC.y;
    double dr = sqrt (dx * dx + dy * dy);
    if (is_equal (dr, 0)) return;
    double lambda = 0;
    lambda = (op.objLoc - op.width / 2) / dr;
    begedge.x = otdw.begC.x - dx * lambda;
    begedge.y = otdw.begC.y - dy * lambda;
    begedge_up = begedge;
    endedge.x = begedge.x + walldir_perp.x * op.base_reveal_width;
    endedge.y = begedge.y + walldir_perp.y * op.base_reveal_width;
    walledge = { begedge , endedge };
    if (!walledge.IsZeroLength ()) {
        OtdWall wallotd;
        wallotd.base_guid = otdw.base_guid;
        wallotd.height = height;
        wallotd.zBottom = zDown;
        wallotd.width = op.base_reveal_width;
        wallotd.length = height;
        wallotd.floorInd = otdw.floorInd;
        wallotd.begC = { walledge.c1.x, walledge.c1.y };
        wallotd.endC = { walledge.c2.x, walledge.c2.y };
        wallotd.material = otdw.material;
        wallotd.base_composite = otdw.base_composite;
        wallotd.base_type = API_WindowID;
        wallotd.draw_type = API_ColumnID;
        wallotd.type = Reveal_Main;
        op.has_reveal = true;
        OtdWall_Delim_All (favdict, opw, wallotd, otd_zBottom, otd_height_down, otd_height_main, otd_height_up, otd_height, om_main, om_up, om_down, om_reveals, om_column, om_floor, om_ceil, om_zone);
    }
    lambda = (op.objLoc + op.width / 2) / dr;
    begedge.x = otdw.begC.x - dx * lambda;
    begedge.y = otdw.begC.y - dy * lambda;
    endedge_up = begedge; // Для горизнтальной части откоса
    endedge.x = begedge.x + walldir_perp.x * op.base_reveal_width;
    endedge.y = begedge.y + walldir_perp.y * op.base_reveal_width;
    walledge = { begedge , endedge };
    if (!walledge.IsZeroLength ()) {
        OtdWall wallotd;
        wallotd.base_guid = otdw.base_guid;
        wallotd.height = height;
        wallotd.zBottom = zDown;
        wallotd.width = op.base_reveal_width;
        wallotd.length = height;
        wallotd.floorInd = otdw.floorInd;
        wallotd.begC = { walledge.c2.x, walledge.c2.y };
        wallotd.endC = { walledge.c1.x, walledge.c1.y };
        wallotd.material = otdw.material;
        wallotd.base_composite = otdw.base_composite;
        wallotd.base_type = API_WindowID;
        wallotd.draw_type = API_ColumnID;
        wallotd.type = Reveal_Main;
        op.has_reveal = true;
        OtdWall_Delim_All (favdict, opw, wallotd, otd_zBottom, otd_height_down, otd_height_main, otd_height_up, otd_height, om_main, om_up, om_down, om_reveals, om_column, om_floor, om_ceil, om_zone);
    }
    walledge = { begedge_up , endedge_up };
    if (!walledge.IsZeroLength () && !is_upper_wall) {
        OtdWall wallotd;
        wallotd.base_guid = otdw.base_guid;
        wallotd.height = otd_thickness;
        wallotd.zBottom = zDown + height;
        wallotd.width = op.base_reveal_width;
        wallotd.length = walledge.GetLength ();
        wallotd.floorInd = otdw.floorInd;
        wallotd.begC = { walledge.c2.x, walledge.c2.y };
        wallotd.endC = { walledge.c1.x, walledge.c1.y };
        wallotd.material = otdw.material;
        wallotd.base_composite = otdw.base_composite;
        wallotd.base_type = API_WindowID;
        wallotd.draw_type = API_BeamID;
        wallotd.type = Reveal_Main;
        op.has_reveal = true;
        OtdWall_Delim_All (favdict, opw, wallotd, otd_zBottom, otd_height_down, otd_height_main, otd_height_up, otd_height, om_main, om_up, om_down, om_reveals, om_column, om_floor, om_ceil, om_zone);
    }
}

// -----------------------------------------------------------------------------
// Разбивка созданных стен по высотам на основании информации из зоны
// -----------------------------------------------------------------------------
void OtdWall_Delim_All (const MatarialToFavoriteDict& favdict, GS::Array<OtdWall>& opw, OtdWall& otdw, double& otd_zBottom, double& otd_height_down, double& otd_height_main, double& otd_height_up, double& otd_height, OtdMaterial& om_main, OtdMaterial& om_up, OtdMaterial& om_down,
        OtdMaterial& om_reveals, OtdMaterial& om_column, OtdMaterial& om_floor, OtdMaterial& om_ceil, OtdMaterial& om_zone)
{
    TypeOtd type = otdw.type;
    bool has_delim = false; // Найдена разбивка
    double height = 0; // Высота элемента
    double zBottom = 0; // Отметка низа
    // Панели
    if (otd_height_down > 0) {
        height = otd_height_down;
        zBottom = otd_zBottom;
        type = Wall_Down;
        if (otdw.base_type == API_SlabID) type = Floor;
        if (otdw.base_type == API_WindowID) type = Reveal_Down;
        if (otdw.type == Ceil) type = Ceil;
        if (otdw.type == Floor) type = Floor;
        if (OtdWall_Delim_One (favdict, otdw, opw, height, zBottom, type, om_main, om_up, om_down, om_reveals, om_column, om_floor, om_ceil, om_zone)) has_delim = true;
    }
    // Основная часть
    if (otd_height_main > 0) {
        type = Wall_Main;
        if (otdw.base_type == API_ColumnID) type = Column;
        if (otdw.base_type == API_WindowID) type = Reveal_Main;
        if (otdw.type == Ceil) type = Ceil;
        if (otdw.type == Floor) type = Floor;
        height = otd_height_main;
        zBottom = otd_zBottom + otd_height_down;
        if (OtdWall_Delim_One (favdict, otdw, opw, height, zBottom, type, om_main, om_up, om_down, om_reveals, om_column, om_floor, om_ceil, om_zone)) has_delim = true;
    }
    // Пространство за потолком
    if (otd_height_up > 0) {
        height = otd_height_up;
        zBottom = otd_zBottom + otd_height_down + otd_height_main;
        type = Wall_Up;
        if (otdw.type == Ceil) type = Ceil;
        if (otdw.type == Floor) type = Floor;
        if (otdw.base_type == API_WindowID) type = Reveal_Up;
        if (OtdWall_Delim_One (favdict, otdw, opw, height, zBottom, type, om_main, om_up, om_down, om_reveals, om_column, om_floor, om_ceil, om_zone)) has_delim = true;
    }
    //Если высоты заданы не были - подгоним стенку под зону
    if (!has_delim) {
        height = otd_height;
        zBottom = otd_zBottom;
        if (otdw.base_type == API_ColumnID) type = Column;
        if (otdw.base_type == API_WindowID) type = Reveal_Main;
        if (otdw.type == Ceil) type = Ceil;
        if (otdw.type == Floor) type = Floor;
        OtdWall_Delim_One (favdict, otdw, opw, height, zBottom, type, om_main, om_up, om_down, om_reveals, om_column, om_floor, om_ceil, om_zone);
    }
}

// -----------------------------------------------------------------------------
// Добавляет стену с заданной высотой
// Удаляет отверстия, не попадающие в диапазон
// Подгоняет размер отверсий
// -----------------------------------------------------------------------------
bool OtdWall_Delim_One (const MatarialToFavoriteDict& favdict, OtdWall otdn, GS::Array<OtdWall>& opw, double height, double zBottom, TypeOtd& type, OtdMaterial& om_main, OtdMaterial& om_up, OtdMaterial& om_down,
        OtdMaterial& om_reveals, OtdMaterial& om_column, OtdMaterial& om_floor, OtdMaterial& om_ceil, OtdMaterial& om_zone)
{
    if (height < min_dim || is_equal (height, 0)) {
        return false;
    }
    // Проверяем - находится ли изначальная конструкция в этом диапазоне?
    if (otdn.zBottom >= zBottom + height) {
        return false; // Конструкция начинается выше необходимого
    }
    if (otdn.zBottom + otdn.height <= zBottom) {
        return false; // Конструкция заканчивается ниже необходимого
    }
    double zDup = fmin (zBottom + height, otdn.zBottom + otdn.height); // Новая отметка верха стены
    double zDown = fmax (zBottom, otdn.zBottom); // Новая отметка низа стены
    height = fmin (otdn.height, zDup - zDown); // Новая высота стены
    if (height < min_dim || is_equal (height, 0)) {
        return false;
    }
    zBottom = zDown; // Переназначаем для расчётов окон
    zDup = zBottom + height; // Переназначаем для расчётов окон
    double dlower = otdn.zBottom - zBottom; // Разница отметок стены до и после подрезки
    // Удаляем лишние окна, подстраиваем высоту
    if (!otdn.openings.IsEmpty ()) {
        GS::Array<OtdOpening> newopenings;
        for (OtdOpening& op : otdn.openings) {
            // Проём начинается выше стенки
            if (op.zBottom > zDup || is_equal (op.zBottom, zDup)) {
                continue;
            }
            double zOpup = op.zBottom + op.height; // Отметка верха проёма
            // Проём заканчивается ниже стенки
            if (zOpup < zBottom || is_equal (zBottom, zOpup)) {
                continue;
            }
            zDup = fmin (zBottom + height, op.zBottom + op.height);
            zDown = fmax (zBottom, op.zBottom);
            op.zBottom = zDown;
            op.height = fmin (op.height, zDup - zDown);
            op.lower = op.zBottom - zBottom;
            if (op.height > min_dim && op.width > min_dim) newopenings.PushNew (op);
        }
        otdn.openings = newopenings;
    }
    otdn.zBottom = zBottom;
    otdn.height = height;
    if (!is_equal (otdn.width, 0)) otdn.length = height;
    otdn.type = type;
    SetMaterialByType (favdict, otdn, om_main, om_up, om_down, om_reveals, om_column, om_floor, om_ceil, om_zone);
    opw.PushNew (otdn);
    return true;
}

void SetMaterialByType (const MatarialToFavoriteDict& favdict, OtdWall& otdw, OtdMaterial& om_main, OtdMaterial& om_up, OtdMaterial& om_down,
        OtdMaterial& om_reveals, OtdMaterial& om_column, OtdMaterial& om_floor, OtdMaterial& om_ceil, OtdMaterial& om_zone)
{
    OtdMaterial material;
    // Проверим существование свойств для записи, при необходимости - поменяем тип отделки
    switch (otdw.type) {
        case NoSet:
            material.material = 0;
            material.smaterial = "";
            #if defined(TESTING)
            DBprnt ("SetMaterialByType err", "NoSet material.smaterial = "";");
            #endif
            break;
        case Wall_Main:
            material = om_main;
            break;
        case Wall_Up:
            if (!om_up.smaterial.IsEmpty ()) {
                material = om_up;
            } else {
                material = om_main;
            }
            if (om_up.rawname.IsEqual (om_main.rawname)) otdw.type = Wall_Main;
            break;
        case Wall_Down:
            if (!om_down.smaterial.IsEmpty ()) {
                material = om_down;
            } else {
                material = om_main;
            }
            if (om_down.rawname.IsEqual (om_main.rawname)) otdw.type = Wall_Main;
            break;
        case Reveal_Main:
            if (!om_reveals.smaterial.IsEmpty ()) {
                material = om_reveals;
            } else {
                material = om_main;
            }
            if (material.smaterial.IsEmpty ()) material = om_main;
            otdw.type = Reveal_Main;
            if (om_reveals.rawname.IsEqual (om_main.rawname)) otdw.type = Wall_Main;
            break;
        case Reveal_Up:
            if (!om_reveals.smaterial.IsEmpty ()) {
                material = om_reveals;
            } else {
                material = om_up;
            }
            if (material.smaterial.IsEmpty ()) material = om_main;
            otdw.type = Reveal_Main;
            if (om_reveals.rawname.IsEqual (om_main.rawname)) otdw.type = Wall_Main;
            break;
        case Reveal_Down:
            if (!om_reveals.smaterial.IsEmpty ()) {
                material = om_reveals;
            } else {
                material = om_down;
            }
            if (material.smaterial.IsEmpty ()) material = om_main;
            otdw.type = Reveal_Main;
            if (om_reveals.rawname.IsEqual (om_main.rawname)) otdw.type = Wall_Main;
            break;
        case Column:
            if (!om_column.smaterial.IsEmpty ()) {
                material = om_column;
            } else {
                material = om_main;
            }
            if (om_column.rawname.IsEqual (om_main.rawname)) otdw.type = Wall_Main;
            break;
        case Floor:
            material = om_floor;
            break;
        case Ceil:
            material = om_ceil;
            break;
        default:
            material.material = 0;
            material.smaterial = "";
            #if defined(TESTING)
            DBprnt ("SetMaterialByType err", "default material.smaterial = "";");
            #endif
            break;
    }
    if (material.smaterial.IsEmpty ()) material = om_zone;
    otdw.material = material;
    SetMaterialFinish (material, otdw.type, otdw.draw_type, favdict, otdw.base_composite, otdw.favorite);
}

void SetMaterialFinish (const OtdMaterial& material, const TypeOtd& type, const API_ElemTypeID& draw_type, const MatarialToFavoriteDict& favdict, GS::Array<ParamValueComposite>& base_composite, MatarialToFavorite& favorite)
{
    Favorite_FindName (favorite, material, type, draw_type, favdict);
    ParamValueComposite p;
    #if defined(AC_27) || defined(AC_28)
    p.inx = ACAPI_CreateAttributeIndex (material.material);
    #else
    p.inx = material.material;
    #endif
    GS::UniString part = material.smaterial;
    part.Trim ();
    part.ReplaceAll ("  ", " ");
    if (part.Contains ('@')) {
        part = '@' + part;
        part = part.GetSubstring ('@', '@', 0);
        part.Trim ();
        p.val = part;
    } else {
        p.val = material.smaterial;
    }
    p.num = -1;
    p.structype = APICWallComp_Finish;
    base_composite.Push (p);
}

bool Edge_FindOnEdge (Sector& edge, GS::Array<Sector>& edges, Sector& findedge)
{
    double dx; double dy; double dr;
    for (UInt32 i = 0; i < edges.GetSize (); i++) {
        if (edge.ContainsPoint (edges[i].c1) || edge.ContainsPoint (edges[i].c2) || edges[i].ContainsPoint (edge.c1) || edges[i].ContainsPoint (edge.c2)) {
            GS::Optional<UnitVector_2D>	edgedir = edge.GetDirection ();
            if (edgedir.HasValue ()) {
                GS::Optional<UnitVector_2D>	roomedgedir = edges[i].GetDirection ();
                if (roomedgedir.HasValue ()) {
                    if (edgedir.Get ().IsParallelWith (roomedgedir.Get ())) {
                        findedge = edges[i];
                        dx = -findedge.c1.x + findedge.c2.x;
                        dy = -findedge.c1.y + findedge.c2.y;
                        dr = sqrt (dx * dx + dy * dy);
                        if (dr > min_dim) return true;
                    }

                }
            }
        }
    }
    return false;
}

// -----------------------------------------------------------------------------
// Проверка наличия отрезка в массиве отрезков.
// В случае нахождение проверяется направление, при необходимости разворачивается
// -----------------------------------------------------------------------------
bool Edge_FindEdge (Sector& edge, GS::Array<Sector>& edges)
{
    for (UInt32 i = 0; i < edges.GetSize (); i++) {
        if (edges[i].c1.IsNear (edge.c1, min_dim) && edges[i].c2.IsNear (edge.c2, min_dim)) {
            return true;
        } else {
            if (edges[i].c1.IsNear (edge.c2, min_dim) && edges[i].c2.IsNear (edge.c1, min_dim)) {
                edge = edge.InvertDirection ();
                return true;
            }
        }
    }
    return false;
}

// -----------------------------------------------------------------------------
// Создание элементов отделки
// -----------------------------------------------------------------------------
void Draw_Elements (const Stories& storyLevels, OtdRooms& zoneelements, UnicElementByType& subelementByparent, ClassificationFunc::ClassificationDict& finclass, GS::Array<API_Guid>& deletelist)
{
    #if defined(TESTING)
    DBprnt ("Draw_Elements", "start");
    #endif

    GS::UniString funcname = "Draw finishing element(s)";

    GSErrCode err = NoError;
    API_Element wallelement = {}; BNZeroMemory (&wallelement, sizeof (API_Element));
    API_Element slabelement = {}; BNZeroMemory (&slabelement, sizeof (API_Element));
    API_Element wallobjelement = {}; API_ElementMemo wallobjmemo; BNZeroMemory (&wallobjmemo, sizeof (API_ElementMemo)); BNZeroMemory (&wallobjelement, sizeof (API_Element));
    API_Element slabobjelement = {}; API_ElementMemo slabobjmemo; BNZeroMemory (&slabobjmemo, sizeof (API_ElementMemo)); BNZeroMemory (&slabobjelement, sizeof (API_Element));
    API_Element windowelement = {}; API_ElementMemo windowmemo; BNZeroMemory (&windowmemo, sizeof (API_ElementMemo)); BNZeroMemory (&windowelement, sizeof (API_Element));
    GS::UniString UndoString = RSGetIndString (ID_ADDON_STRINGS + isEng (), RoombookId, ACAPI_GetOwnResModule ());
    ACAPI_CallUndoableCommand (UndoString, [&]() -> GSErrCode {
        if (!deletelist.IsEmpty ()) {
            err = ACAPI_Element_Delete (deletelist);
            msg_rep ("RoomBook", GS::UniString::Printf ("Removed %d obsolete finishing elements", deletelist.GetSize ()), err, APINULLGuid);
        } else {
            msg_rep ("RoomBook", "Obsolete finishing elements not found", NoError, APINULLGuid);
        }
        for (OtdRooms::PairIterator cIt = zoneelements.EnumeratePairs (); cIt != NULL; ++cIt) {
            #if defined(AC_28)
            OtdRoom& otd = cIt->value;
            #else
            OtdRoom& otd = *cIt->value;
            #endif
            nPhase += 1;
            #if defined(AC_27) || defined(AC_28)
            ACAPI_ProcessWindow_SetNextProcessPhase (&funcname, &nPhase);
            if (ACAPI_ProcessWindow_IsProcessCanceled ()) return NoError;
            #else
            ACAPI_Interface (APIIo_SetNextProcessPhaseID, &funcname, &nPhase);
            if (ACAPI_Interface (APIIo_IsProcessCanceledID, nullptr, nullptr)) return NoError;
            #endif
            if (!otd.isValid) continue;
            if (!otd.create_all_elements) continue;
            API_Guid class_guid = APINULLGuid;
            GS::Array<API_Guid> group;
            if (otd.create_wall_elements || otd.create_column_elements || otd.create_reveal_elements) {
                for (OtdWall& otdwall : otd.otdwall) {
                    if ((otdwall.type == Wall_Main || otdwall.type == Wall_Up || otdwall.type == Wall_Down) && !otd.create_wall_elements) continue;
                    if ((otdwall.type == Column) && !otd.create_column_elements) continue;
                    if ((otdwall.type == Reveal_Main || otdwall.type == Reveal_Down || otdwall.type == Reveal_Up) && !otd.create_reveal_elements) continue;
                    OtdWall_Draw (storyLevels, otdwall, wallelement, wallobjelement, wallobjmemo, windowelement, windowmemo, subelementByparent);
                    Param_AddUnicElementByType (otd.zone_guid, otdwall.otd_guid, API_ZoneID, subelementByparent);// Привязка отделочных стен к зоне
                    for (OtdOpening& op : otdwall.openings) {
                        Param_AddUnicElementByType (otd.zone_guid, op.otd_guid, API_ZoneID, subelementByparent);// Привязка отделочных проёмов к зоне
                    }
                    // Классификация созданных стен
                    Class_SetClass (otdwall, finclass);
                    for (OtdOpening& op : otdwall.openings) {
                        Class_SetClass (op, finclass);
                    }
                    group.Push (otdwall.otd_guid);
                }
            } else {
                otd.otdwall.Clear ();
            }
            if (otd.create_ceil_elements || otd.create_floor_elements) {
                for (OtdSlab& otdslab : otd.otdslab) {
                    if (otdslab.type == Ceil && !otd.create_ceil_elements) continue;
                    if (otdslab.type == Floor && !otd.create_floor_elements) continue;
                    Floor_Draw (storyLevels, slabelement, slabobjelement, slabobjmemo, otdslab, subelementByparent);
                    Param_AddUnicElementByType (otd.zone_guid, otdslab.otd_guid, API_ZoneID, subelementByparent); // Привязка перекрытий к зоне
                    Class_SetClass (otdslab, finclass);
                    group.Push (otdslab.otd_guid);
                }
            } else {
                otd.otdslab.Clear ();
            }
            if (group.GetSize () > 1) {
                #if defined(AC_27) || defined(AC_28)
                err = ACAPI_Grouping_Tool (group, APITool_Group, nullptr);
                #else
                API_Guid groupGuid = APINULLGuid;
                err = ACAPI_ElementGroup_Create (group, &groupGuid);
                #ifndef AC_22
                if (err != NoError) err = ACAPI_Element_Tool (group, APITool_Group, nullptr);
                #endif
                #endif
                if (err != NoError) msg_rep ("Draw_Elements", "ACAPI_Grouping_CreateGroup", err, APINULLGuid);
            }

        }
        return NoError;
    });
    if (GetElemTypeID (windowelement) == API_WindowID) ACAPI_DisposeElemMemoHdls (&windowmemo);
    if (GetElemTypeID (slabobjelement) == API_ObjectID) ACAPI_DisposeElemMemoHdls (&slabobjmemo);
    if (GetElemTypeID (wallobjelement) == API_ObjectID) ACAPI_DisposeElemMemoHdls (&wallobjmemo);
    #if defined(TESTING)
    DBprnt ("Draw_Elements", "end");
    #endif
}

// -----------------------------------------------------------------------------
// Построение отделочных стен (общая)
// -----------------------------------------------------------------------------
void OtdWall_Draw (const Stories& storyLevels, OtdWall& edges, API_Element& wallelement, API_Element& wallobjelement, API_ElementMemo& wallobjmemo, API_Element& windowelement, API_ElementMemo& windowmemo, UnicElementByType& subelementByparent)
{
    if (edges.height < min_dim) return;
    double dx = -edges.endC.x + edges.begC.x;
    double dy = -edges.endC.y + edges.begC.y;
    double dr = sqrt (dx * dx + dy * dy);
    if (dr < min_dim) return;
    if (edges.favorite.type == API_ObjectID) {
        // горизонтальную часть откоса строим в этом случае аксессуаром пола/потолка
        if (edges.draw_type == API_BeamID) {
            Reveal_Up_Draw_Object (edges.favorite.name, storyLevels, edges, wallobjelement, wallobjmemo);
        } else {
            OtdWall_Draw_Object (edges.favorite.name, storyLevels, edges, wallobjelement, wallobjmemo);
        }
    }
    if (edges.favorite.type == API_WallID) {
        OtdWall_Draw_Wall (edges.favorite.name, storyLevels, edges, wallelement, windowelement, windowmemo, subelementByparent);
    }
    if (edges.favorite.type == API_ColumnID) {
        //OtdWall_Draw_Object (edges.favorite.name, storyLevels, edges, wallobjelement, wallobjmemo, subelementByparent);
    }
    if (edges.favorite.type == API_BeamID) {
        //OtdWall_Draw_Wall (edges.favorite.name, storyLevels, edges, wallelement, windowelement, windowmemo, subelementByparent);
    }
    Param_AddUnicElementByType (edges.base_guid, edges.otd_guid, API_WallID, subelementByparent); // Привязка отделочной стены к базовой
}

// -----------------------------------------------------------------------------
// Построение отделочных стен аксессуаром
// -----------------------------------------------------------------------------
void OtdWall_Draw_Object (const GS::UniString& favorite_name, const Stories& storyLevels, OtdWall& edges, API_Element& wallobjelement, API_ElementMemo& wallobjmemo)
{
    GSErrCode err = NoError;
    if (!OtdWall_GetDefult_Object (favorite_name, wallobjelement, wallobjmemo)) {
        return;
    }
    Point2D wbegC = { edges.begC.x, edges.begC.y };
    Point2D wendC = { edges.endC.x, edges.endC.y };
    Sector walledge = { wbegC , wendC };
    GS::Optional<UnitVector_2D>	walldir = walledge.GetDirection ();
    if (!walldir.HasValue ()) {
        #if defined(TESTING)
        DBprnt ("OtdWall_Create_FromWall err", "!walldir.HasValue");
        #endif
        return;
    }
    double ac_wall_length = walledge.GetLength ();
    GDLHelpers::ParamDict accsessoryparams;
    GDLHelpers::Param p;
    p.num = 1; accsessoryparams.Add ("{@gdl:ac_refside}", p);
    p.num = edges.material.material; accsessoryparams.Add ("{@gdl:gs_bw_mat}", p);
    p.num = edges.material.material; accsessoryparams.Add ("{@gdl:matp1}", p);


    p.num = edges.height; accsessoryparams.Add ("{@gdl:ac_wall_height}", p);
    p.num = ac_wall_length; accsessoryparams.Add ("{@gdl:ac_wall_length}", p);
    p.num = 0; accsessoryparams.Add ("{@gdl:ac_wall_radius}", p);

    p.dim1 = 1; p.dim2 = 2;
    p.arr_num.Push (0);
    p.arr_num.Push (0);
    accsessoryparams.Add ("{@gdl:ac_side_poly}", p); p.arr_num.Clear ();

    p.dim1 = 2; p.dim2 = 2;
    p.arr_num.Push (0);
    p.arr_num.Push (edges.height);
    p.arr_num.Push (ac_wall_length);
    p.arr_num.Push (edges.height);
    accsessoryparams.Add ("{@gdl:ac_top_poly}", p); p.arr_num.Clear ();

    p.dim1 = 2; p.dim2 = 2;
    p.arr_num.Push (0);
    p.arr_num.Push (0);
    p.arr_num.Push (ac_wall_length);
    p.arr_num.Push (0);
    accsessoryparams.Add ("{@gdl:ac_bot_poly}", p); p.arr_num.Clear ();

    p.dim1 = 1; p.dim2 = 2;
    p.arr_num.Push (edges.ang_begC);
    p.arr_num.Push (edges.ang_endC);
    accsessoryparams.Add ("{@gdl:ac_angles}", p); p.arr_num.Clear ();

    if (edges.openings.IsEmpty ()) {
        p.dim1 = 1; p.dim2 = 2;
        p.arr_num.Push (0);
        p.arr_num.Push (0);
    } else {
        Int32 dim2 = 0;
        for (OtdOpening& op : edges.openings) {
            if (op.width < min_dim || op.height < min_dim) continue;
            double objLoc = ac_wall_length - op.objLoc;
            double halfwidth = op.width / 2;
            //if (op.has_reveal) {
            //    op.width -= wallelement.wall.thickness * 2;
            //    op.height -= wallelement.wall.thickness;
            //}
            // Начало отверстия
            p.arr_num.Push (0.106);
            p.arr_num.Push (objLoc - halfwidth);
            p.arr_num.Push (op.lower);

            p.arr_num.Push (objLoc - halfwidth);
            p.arr_num.Push (op.lower + op.height);

            p.arr_num.Push (objLoc + halfwidth);
            p.arr_num.Push (op.lower + op.height);

            p.arr_num.Push (objLoc + halfwidth);
            p.arr_num.Push (op.lower);
        }
        p.dim1 = edges.openings.GetSize (); p.dim2 = 9;
    }
    accsessoryparams.Add ("{@gdl:ac_wd_poly}", p); p.arr_num.Clear ();
    GDLHelpers::ParamToMemo (wallobjmemo, accsessoryparams);
    Vector2D ref = { -1, 0 };
    Vector2D wallvect = walldir.Get ().ToVector2D ();
    double ang = wallvect.CalcAngleToReference (ref);
    wallobjelement.object.angle = ang;
    wallobjelement.object.pos = edges.endC;
    wallobjelement.header.floorInd = edges.floorInd;
    #if defined(AC_27) || defined(AC_28)
    API_AttributeIndex ematerial = ACAPI_CreateAttributeIndex (edges.material.material);
    wallobjelement.object.mat = ematerial;
    #else
    wallobjelement.object.mat = edges.material.material;
    #endif
    wallobjelement.object.level = GetOffsetFromStory (edges.zBottom, edges.floorInd, storyLevels);
    err = ACAPI_Element_Create (&wallobjelement, &wallobjmemo);
    if (err != NoError) {
        return;
    }
    edges.otd_guid = wallobjelement.header.guid;
}

// -----------------------------------------------------------------------------
// Получение из избранного аксессуара стены для простроения 
// -----------------------------------------------------------------------------
bool OtdWall_GetDefult_Object (const GS::UniString& favorite_name, API_Element& wallobjelement, API_ElementMemo& wallobjmemo)
{
    GSErrCode err = NoError;
    BNZeroMemory (&wallobjmemo, sizeof (API_ElementMemo));
    BNZeroMemory (&wallobjelement, sizeof (API_Element));
    if (Favorite_GetByName (favorite_name, wallobjelement, wallobjmemo)) {
        if (GetElemTypeID (wallobjelement) == API_ObjectID) return true;
    }
    SetElemTypeID (wallobjelement, API_ObjectID);
    err = ACAPI_Element_GetDefaults (&wallobjelement, &wallobjmemo);
    if (err != NoError) {
        return false;
    }
    wallobjelement.object.angle = 0;
    wallobjelement.object.isAutoOnStoryVisibility = false;
    wallobjelement.object.useObjPens = true;
    wallobjelement.object.useObjLtypes = true;
    wallobjelement.object.useObjMaterials = true;
    return true;
}


void Reveal_Up_Draw_Object (const GS::UniString& favorite_name, const Stories& storyLevels, OtdWall& edges, API_Element& slabobjelement, API_ElementMemo& slabobjmemo)
{

}

// -----------------------------------------------------------------------------
// Построение отделочных стен стандартной стеной
// -----------------------------------------------------------------------------
void OtdWall_Draw_Wall (const GS::UniString& favorite_name, const Stories& storyLevels, OtdWall& edges, API_Element& wallelement, API_Element& windowelement, API_ElementMemo& windowmemo, UnicElementByType& subelementByparent)
{
    GSErrCode err = NoError;
    if (!OtdWall_GetDefult_Wall (favorite_name, wallelement)) {
        return;
    }
    // Откосы
    if (edges.draw_type == API_BeamID && edges.favorite.type == API_WallID) {
        edges.height = wallelement.wall.thickness;
        edges.zBottom -= edges.height;
        if (!is_equal (edges.width, 0) && edges.width > 0) wallelement.wall.thickness = edges.width;
    }
    wallelement.wall.begC = edges.begC;
    wallelement.wall.endC = edges.endC;
    wallelement.wall.height = edges.height;
    wallelement.header.floorInd = edges.floorInd;
    wallelement.wall.bottomOffset = GetOffsetFromStory (edges.zBottom, edges.floorInd, storyLevels);
    #if defined(AC_27) || defined(AC_28)
    API_AttributeIndex ematerial = ACAPI_CreateAttributeIndex (edges.material.material);
    wallelement.wall.refMat.hasValue = true;
    wallelement.wall.sidMat.hasValue = true;
    wallelement.wall.refMat.value = ematerial;
    wallelement.wall.oppMat.value = ematerial;
    wallelement.wall.sidMat.value = ematerial;
    #else
    wallelement.wall.refMat.overridden = true;
    wallelement.wall.sidMat.overridden = true;
    wallelement.wall.refMat.attributeIndex = edges.material.material;
    wallelement.wall.oppMat.attributeIndex = edges.material.material;
    wallelement.wall.sidMat.attributeIndex = edges.material.material;
    #endif
    wallelement.wall.zoneRel = APIZRel_None;
    wallelement.wall.flipped = false;
    err = ACAPI_Element_Create (&wallelement, nullptr);
    if (err != NoError) {
        return;
    }
    edges.otd_guid = wallelement.header.guid;
    for (OtdOpening& op : edges.openings) {
        Opening_Draw (wallelement, windowelement, windowmemo, op, subelementByparent, edges.zBottom);
    }
}

// -----------------------------------------------------------------------------
// Получение из избранного стены для простроения 
// -----------------------------------------------------------------------------
bool OtdWall_GetDefult_Wall (const GS::UniString& favorite_name, API_Element& wallelement)
{
    GSErrCode err = NoError;
    BNZeroMemory (&wallelement, sizeof (API_Element));
    if (Favorite_GetByName (favorite_name, wallelement)) {
        if (GetElemTypeID (wallelement) == API_WallID) return true;
    }
    SetElemTypeID (wallelement, API_WallID);
    err = ACAPI_Element_GetDefaults (&wallelement, nullptr);
    if (err != NoError) {
        return false;
    }
    wallelement.wall.zoneRel = APIZRel_None;
    wallelement.wall.referenceLineLocation = APIWallRefLine_Inside;
    wallelement.wall.flipped = false;
    #if defined AC_26 || defined AC_27 || defined AC_28
    #else
    wallelement.wall.refMat.overridden = true;
    wallelement.wall.oppMat.overridden = true;
    wallelement.wall.sidMat.overridden = true;
    #endif
    wallelement.wall.modelElemStructureType = API_BasicStructure;
    wallelement.wall.thickness = otd_thickness;
    return true;
}

// -----------------------------------------------------------------------------
// Построение проёмов в отделочных стенах 
// -----------------------------------------------------------------------------
void Opening_Draw (API_Element& wallelement, API_Element& windowelement, API_ElementMemo& windowmemo, OtdOpening& op, UnicElementByType& subelementByparent, const double& zBottom)
{
    GSErrCode err = NoError;
    if (wallelement.wall.type == APIWtyp_Poly) return;
    if (!Opening_GetDefult ("smstf window", windowelement, windowmemo)) {
        return;
    }
    windowelement.window.objLoc = op.objLoc;
    windowelement.window.owner = wallelement.header.guid;
    if (op.has_reveal) {
        // TODO Дописать определение толщины из откоса
        double th = wallelement.wall.thickness;
        double zUp_op = op.zBottom + op.height;
        double zUp_wall = zBottom + wallelement.wall.height;
        // Уменьшаем проём на толщину откоса по высоте
        if (!is_equal (zUp_op, zUp_wall)) {
            op.height -= th;
        }
        double lwall = sqrt ((wallelement.wall.begC.x - wallelement.wall.endC.x) * (wallelement.wall.begC.x - wallelement.wall.endC.x) + (wallelement.wall.begC.y - wallelement.wall.endC.y) * (wallelement.wall.begC.y - wallelement.wall.endC.y));
        if (!is_equal (op.objLoc + op.width / 2, lwall)) {
            op.width -= th;
            op.objLoc -= th;
        }
        if (!is_equal (op.objLoc - op.width / 2, 0)) {
            op.width -= th;
            op.objLoc += th;
        }
    }
    if (op.width < min_dim || op.height < min_dim) return;
    windowelement.window.openingBase.width = op.width;
    windowelement.window.openingBase.height = op.height;
    windowelement.window.lower = op.lower;
    err = ACAPI_Element_Create (&windowelement, &windowmemo);
    if (err != NoError) {
        return;
    }
    op.otd_guid = windowelement.header.guid;
    Param_AddUnicElementByType (op.base_guid, op.otd_guid, API_WindowID, subelementByparent); // Привязка отделочного проёма к базовому
    return;
}

// -----------------------------------------------------------------------------
// Получение настроек проёмов в отделочных стенах 
// -----------------------------------------------------------------------------
bool Opening_GetDefult (const GS::UniString& favorite_name, API_Element& windowelement, API_ElementMemo& windowmemo)
{
    GSErrCode err = NoError;
    BNZeroMemory (&windowmemo, sizeof (API_ElementMemo));
    BNZeroMemory (&windowelement, sizeof (API_Element));
    if (Favorite_GetByName (favorite_name, windowelement, windowmemo)) {
        if (GetElemTypeID (windowelement) == API_WindowID) return true;
    }
    SetElemTypeID (windowelement, API_WindowID);
    err = ACAPI_Element_GetDefaults (&windowelement, &windowmemo);
    if (err != NoError) {
        return false;
    }
    return true;
}

// -----------------------------------------------------------------------------
// Построение потолка/пола (общая)
// -----------------------------------------------------------------------------
void Floor_Draw (const Stories& storyLevels, API_Element& slabelement, API_Element& slabobjelement, API_ElementMemo& slabobjmemo, OtdSlab& otdslab, UnicElementByType& subelementByparent)
{
    if (otdslab.favorite.type == API_ObjectID) {
        Floor_Draw_Object (otdslab.favorite.name, storyLevels, slabobjelement, slabobjmemo, otdslab);
    } else {
        Floor_Draw_Slab (otdslab.favorite.name, storyLevels, slabelement, otdslab);
    }
    otdslab.poly.Clear ();
    Param_AddUnicElementByType (otdslab.base_guid, otdslab.otd_guid, API_SlabID, subelementByparent); // Привязка отделочного перекрытия к базовому
}

void Floor_Draw_Slab (const GS::UniString& favorite_name, const Stories& storyLevels, API_Element& slabelement, OtdSlab& otdslab)
{
    GSErrCode err = NoError;
    if (!Floor_GetDefult_Slab (favorite_name, slabelement)) {
        return;
    }
    slabelement.slab.thickness = otd_thickness;
    slabelement.header.floorInd = otdslab.floorInd;
    slabelement.slab.level = GetOffsetFromStory (otdslab.zBottom, otdslab.floorInd, storyLevels);
    #if defined(AC_27) || defined(AC_28)
    API_AttributeIndex ematerial = ACAPI_CreateAttributeIndex (otdslab.material.material);
    slabelement.slab.topMat.value = ematerial;
    slabelement.slab.sideMat.value = ematerial;
    slabelement.slab.botMat.value = ematerial;
    slabelement.slab.sideMat.hasValue = true;
    #else
    slabelement.slab.topMat.attributeIndex = otdslab.material.material;
    slabelement.slab.sideMat.attributeIndex = otdslab.material.material;
    slabelement.slab.botMat.attributeIndex = otdslab.material.material;
    slabelement.slab.sideMat.overridden = true;
    #endif
    if (otdslab.type == Ceil) {
        #if defined(AC_27) || defined(AC_28)
        slabelement.slab.botMat.hasValue = true;
        #else
        slabelement.slab.botMat.overridden = true;
        #endif
    }
    if (otdslab.type == Floor) {
        #if defined(AC_27) || defined(AC_28)
        slabelement.slab.topMat.hasValue = true;
        #else
        slabelement.slab.topMat.overridden = true;
        #endif
    }
    API_ElementMemo memo = {}; BNZeroMemory (&memo, sizeof (API_ElementMemo));
    err = ConvertPolygon2DToAPIPolygon (otdslab.poly, slabelement.slab.poly, memo);
    if (err != NoError) {
        ACAPI_DisposeElemMemoHdls (&memo);
        return;
    }
    err = ACAPI_Element_Create (&slabelement, &memo);
    if (err != NoError) {
        ACAPI_DisposeElemMemoHdls (&memo);
        return;
    }
    otdslab.otd_guid = slabelement.header.guid;
    ACAPI_DisposeElemMemoHdls (&memo);
    return;
}

void Floor_Draw_Object (const GS::UniString& favorite_name, const Stories& storyLevels, API_Element& slabobjelement, API_ElementMemo& slabobjmemo, OtdSlab& otdslab)
{
    GSErrCode err = NoError;
    if (!Floor_GetDefult_Object (favorite_name, slabobjelement, slabobjmemo)) {
        return;
    }
    GDLHelpers::ParamDict accsessoryparams;
    GDLHelpers::Param p;
    p.num = 0.02; accsessoryparams.Add ("{@gdl:ac_thickness}", p);
    if (otdslab.type == Ceil) {
        p.num = otdslab.zBottom; accsessoryparams.Add ("{@gdl:ac_ref_height}", p);
        slabobjelement.object.level = GetzPos (0, otdslab.floorInd, storyLevels);
    }
    if (otdslab.type == Floor) {
        p.num = otdslab.zBottom; accsessoryparams.Add ("{@gdl:ac_ref_height}", p);
        slabobjelement.object.level = GetOffsetFromStory (otdslab.zBottom, otdslab.floorInd, storyLevels);
    }
    p.num = 0; accsessoryparams.Add ("{@gdl:ac_pitch}", p);
    p.num = 0; accsessoryparams.Add ("{@gdl:ac_ceilling_side}", p);
    p.num = 0; accsessoryparams.Add ("{@gdl:ac_ceiling_type}", p);
    p.num = otdslab.material.material; accsessoryparams.Add ("{@gdl:ceil_mat}", p);
    p.num = 0.02; accsessoryparams.Add ("{@gdl:ceil_thk}", p);
    p.num = 0; accsessoryparams.Add ("{@gdl:ac_slab_side}", p);


    Geometry::Polygon2DData polygon2DData;
    Geometry::InitPolygon2DData (&polygon2DData);
    Geometry::ConvertPolygon2DToPolygon2DData (polygon2DData, otdslab.poly);
    Int32 nCoords = polygon2DData.nVertices;
    Int32 nSubPolys = polygon2DData.nContours;
    Box2DData bbox = otdslab.poly.GetBoundBox ();
    GDLHelpers::Param ac_coords; GDLHelpers::Param ac_whole_poly;
    for (Int32 j = 1; j <= nSubPolys; j++) {
        UInt32 begInd = (*polygon2DData.contourEnds)[j - 1] + 1;
        UInt32 endInd = (*polygon2DData.contourEnds)[j];
        double x_f = (*polygon2DData.vertices)[begInd].x - bbox.xMin;
        double y_f = (*polygon2DData.vertices)[begInd].y - bbox.yMin;
        for (UInt32 k = begInd; k < endInd; k++) {
            double x = (*polygon2DData.vertices)[k].x - bbox.xMin;
            double y = (*polygon2DData.vertices)[k].y - bbox.yMin;
            ac_coords.arr_num.Push (0.025); ac_coords.arr_num.Push (x); ac_coords.arr_num.Push (y);
            if (j == 1) {
                ac_whole_poly.arr_num.Push (x); ac_whole_poly.arr_num.Push (y);
            }
        }
        ac_coords.arr_num.Push (0.051); ac_coords.arr_num.Push (x_f); ac_coords.arr_num.Push (y_f);
    }
    ac_coords.dim1 = ac_coords.arr_num.GetSize () / 3; ac_coords.dim2 = 3;
    ac_whole_poly.dim1 = 1; ac_whole_poly.dim2 = ac_whole_poly.arr_num.GetSize ();
    accsessoryparams.Add ("{@gdl:ac_whole_poly}", ac_whole_poly);
    accsessoryparams.Add ("{@gdl:ac_coords}", ac_coords);
    Geometry::FreePolygon2DData (&polygon2DData);
    GDLHelpers::ParamToMemo (slabobjmemo, accsessoryparams);
    slabobjelement.header.floorInd = otdslab.floorInd;
    slabobjelement.object.pos.x = bbox.xMin;
    slabobjelement.object.pos.y = bbox.yMin;
    double y_start = 0;
    err = ACAPI_Element_Create (&slabobjelement, &slabobjmemo);
    if (err != NoError) {
        return;
    }
    otdslab.otd_guid = slabobjelement.header.guid;
}

bool Floor_GetDefult_Object (const GS::UniString& favorite_name, API_Element& slabobjelement, API_ElementMemo& slabobjmemo)
{
    GSErrCode err = NoError;
    BNZeroMemory (&slabobjmemo, sizeof (API_ElementMemo));
    BNZeroMemory (&slabobjelement, sizeof (API_Element));
    if (Favorite_GetByName (favorite_name, slabobjelement, slabobjmemo)) {
        if (GetElemTypeID (slabobjelement) == API_ObjectID) return true;
    }
    SetElemTypeID (slabobjelement, API_ObjectID);
    err = ACAPI_Element_GetDefaults (&slabobjelement, &slabobjmemo);
    if (err != NoError) {
        return false;
    }
    slabobjelement.object.angle = 0;
    slabobjelement.object.isAutoOnStoryVisibility = false;
    slabobjelement.object.useObjPens = true;
    slabobjelement.object.useObjLtypes = true;
    slabobjelement.object.useObjMaterials = true;
    return true;
}

bool Floor_GetDefult_Slab (const GS::UniString& favorite_name, API_Element& slabelement)
{
    GSErrCode err = NoError;
    BNZeroMemory (&slabelement, sizeof (API_Element));
    if (Favorite_GetByName (favorite_name, slabelement)) {
        if (GetElemTypeID (slabelement) == API_SlabID) return true;
    }
    SetElemTypeID (slabelement, API_SlabID);
    err = ACAPI_Element_GetDefaults (&slabelement, nullptr);
    if (err != NoError) {
        return false;
    }
    slabelement.slab.offsetFromTop = 0;
    slabelement.slab.materialsChained = true;
    #if defined AC_26 || defined AC_27 || defined AC_28
    #else
    slabelement.slab.sideMat.overridden = true;
    slabelement.slab.topMat.overridden = true;
    slabelement.slab.botMat.overridden = true;
    #endif
    slabelement.slab.modelElemStructureType = API_BasicStructure;
    return true;
}

// -----------------------------------------------------------------------------
// Назначение класса стене
// -----------------------------------------------------------------------------
void Class_SetClass (const OtdWall& op, const ClassificationFunc::ClassificationDict& finclass)
{
    if (op.otd_guid == APINULLGuid) return;
    API_Guid class_guid = Class_GetClassGuid (op.type, finclass);
    if (class_guid != APINULLGuid) ACAPI_Element_AddClassificationItem (op.otd_guid, class_guid);
}

// -----------------------------------------------------------------------------
// Назначение класса проёму
// -----------------------------------------------------------------------------
void Class_SetClass (const OtdOpening& op, const ClassificationFunc::ClassificationDict& finclass)
{
    if (op.otd_guid == APINULLGuid) return;
    API_Guid class_guid = Class_GetClassGuid (NoSet, finclass);
    if (class_guid == APINULLGuid) class_guid = Class_GetClassGuid (Wall_Main, finclass);
    if (class_guid != APINULLGuid) ACAPI_Element_AddClassificationItem (op.otd_guid, class_guid);
}

// -----------------------------------------------------------------------------
// Назначение класса полу/потолку
// -----------------------------------------------------------------------------
void Class_SetClass (const OtdSlab& op, const ClassificationFunc::ClassificationDict& finclass)
{
    if (op.otd_guid == APINULLGuid) return;
    API_Guid class_guid = Class_GetClassGuid (op.type, finclass);
    if (class_guid != APINULLGuid) ACAPI_Element_AddClassificationItem (op.otd_guid, class_guid);
}

API_Guid Class_GetClassGuid (const TypeOtd type, const ClassificationFunc::ClassificationDict& finclass)
{
    if (type == Column && finclass.ContainsKey (cls.column_class)) return finclass.Get (cls.column_class).item.guid;
    if ((type == Reveal_Main || type == Reveal_Up || type == Reveal_Down) && finclass.ContainsKey (cls.reveal_class)) return finclass.Get (cls.reveal_class).item.guid;
    if ((type == Wall_Down) && finclass.ContainsKey (cls.otdwall_down_class)) return finclass.Get (cls.otdwall_down_class).item.guid;
    if ((type == Wall_Main || type == Wall_Up || type == Wall_Down || type == Reveal_Main || type == Reveal_Up || type == Reveal_Down) && finclass.ContainsKey (cls.otdwall_class)) return finclass.Get (cls.otdwall_class).item.guid;
    if (type == Floor && finclass.ContainsKey (cls.floor_class)) return finclass.Get (cls.floor_class).item.guid;
    if (type == Ceil && finclass.ContainsKey (cls.ceil_class)) return finclass.Get (cls.ceil_class).item.guid;
    if (finclass.ContainsKey (cls.all_class)) return finclass.Get (cls.all_class).item.guid;
    return APINULLGuid;
}

// -----------------------------------------------------------------------------
// Поиск классов для отделочных стен (some_stuff_fin_ в описании класса)
// -----------------------------------------------------------------------------
void Class_FindFinClass (ClassificationFunc::SystemDict& systemdict, ClassificationFunc::ClassificationDict& findict, UnicGuid& finclassguids)
{
    if (systemdict.IsEmpty ()) return;
    for (GS::HashTable<GS::UniString, ClassificationFunc::ClassificationDict>::PairIterator cIt = systemdict.EnumeratePairs (); cIt != NULL; ++cIt) {
        #if defined(AC_28)
        GS::UniString system = cIt->key;
        ClassificationFunc::ClassificationDict& classes = cIt->value;
        #else
        GS::UniString system = *cIt->key;
        ClassificationFunc::ClassificationDict& classes = *cIt->value;
        #endif
        for (GS::HashTable<GS::UniString, ClassificationFunc::ClassificationValues>::PairIterator cIt = classes.EnumeratePairs (); cIt != NULL; ++cIt) {
            #if defined(AC_28)
            GS::UniString clasname = cIt->key;
            ClassificationFunc::ClassificationValues& clas = cIt->value;
            #else
            GS::UniString clasname = *cIt->key;
            ClassificationFunc::ClassificationValues& clas = *cIt->value;
            #endif
            GS::UniString desc = clas.item.description.ToLowerCase ();
            if (desc.Contains ("some_stuff_fin_")) {
                if (desc.Contains (cls.all_class)) {
                    findict.Add (cls.all_class, clas);
                    if (!finclassguids.ContainsKey (clas.item.guid)) finclassguids.Add (clas.item.guid, false);
                }
                if (desc.Contains (cls.ceil_class)) {
                    findict.Add (cls.ceil_class, clas);
                    if (!finclassguids.ContainsKey (clas.item.guid)) finclassguids.Add (clas.item.guid, false);
                }
                if (desc.Contains (cls.column_class)) {
                    findict.Add (cls.column_class, clas);
                    if (!finclassguids.ContainsKey (clas.item.guid)) finclassguids.Add (clas.item.guid, false);
                }
                if (desc.Contains (cls.floor_class)) {
                    findict.Add (cls.floor_class, clas);
                    if (!finclassguids.ContainsKey (clas.item.guid)) finclassguids.Add (clas.item.guid, false);
                }
                if (desc.Contains (cls.otdwall_class)) {
                    findict.Add (cls.otdwall_class, clas);
                    if (!finclassguids.ContainsKey (clas.item.guid)) finclassguids.Add (clas.item.guid, false);
                }
                if (desc.Contains (cls.reveal_class)) {
                    findict.Add (cls.reveal_class, clas);
                    if (!finclassguids.ContainsKey (clas.item.guid)) finclassguids.Add (clas.item.guid, false);
                }
                if (desc.Contains (cls.otdwall_down_class)) {
                    findict.Add (cls.otdwall_down_class, clas);
                    if (!finclassguids.ContainsKey (clas.item.guid)) finclassguids.Add (clas.item.guid, false);
                }
            }
        }
    }
}

// -----------------------------------------------------------------------------
// Связывание созданных элементов отделки с базовыми элементами
// -----------------------------------------------------------------------------
void SetSyncOtdWall (UnicElementByType& subelementByparent, ParamDictValue& propertyParams, ParamDictElement& paramToWrite)
{
    API_Elem_Head parentelementhead = {};
    GS::Array<API_ElemTypeID> typeinzone;
    UnicGuid syncguidsdict;
    // Типы родительских элементов
    typeinzone.Push (API_ZoneID);
    typeinzone.Push (API_WindowID);
    typeinzone.Push (API_WallID);
    typeinzone.Push (API_SlabID);
    typeinzone.Push (API_ColumnID);
    GS::UniString suffix = "";

    GS::UniString funcname = "Create link with base element";
    for (const API_ElemTypeID& typeelem : typeinzone) {
        if (subelementByparent.ContainsKey (typeelem)) {
            for (UnicElement::PairIterator cIt = subelementByparent.Get (typeelem).EnumeratePairs (); cIt != NULL; ++cIt) {
                #if defined(AC_28)
                API_Guid guid = cIt->key;
                GS::Array<API_Guid> subguids = cIt->value;
                #else
                API_Guid guid = *cIt->key;
                GS::Array<API_Guid> subguids = *cIt->value;
                #endif

                nPhase += 1;
                #if defined(AC_27) || defined(AC_28)
                ACAPI_ProcessWindow_SetNextProcessPhase (&funcname, &nPhase);
                if (ACAPI_ProcessWindow_IsProcessCanceled ()) return;
                #else
                ACAPI_Interface (APIIo_SetNextProcessPhaseID, &funcname, &nPhase);
                if (ACAPI_Interface (APIIo_IsProcessCanceledID, nullptr, nullptr)) return;
                #endif

                parentelementhead.guid = guid;
                if (typeelem == API_ZoneID) {
                    suffix = "zone";
                } else {
                    suffix = "base element";
                }
                if (SyncSetSubelementScope (parentelementhead, subguids, propertyParams, paramToWrite, suffix, false)) {
                    if (!syncguidsdict.ContainsKey (guid)) syncguidsdict.Add (guid, true);
                }
            }
        }
    }
    if (!paramToWrite.IsEmpty ()) {
        funcname = GS::UniString::Printf ("Write GUID base and GUID zone to %d finishing element(s)", paramToWrite.GetSize ());
        nPhase += 1;
        #if defined(AC_27) || defined(AC_28)
        ACAPI_ProcessWindow_SetNextProcessPhase (&funcname, &nPhase);
        if (ACAPI_ProcessWindow_IsProcessCanceled ()) return;
        #else
        ACAPI_Interface (APIIo_SetNextProcessPhaseID, &funcname, &nPhase);
        if (ACAPI_Interface (APIIo_IsProcessCanceledID, nullptr, nullptr)) return;
        #endif

        ACAPI_CallUndoableCommand ("Write property to finishing element",
                [&]() -> GSErrCode {
            ParamHelpers::ElementsWrite (paramToWrite);
            return NoError;
        });
    }
    if (!syncguidsdict.IsEmpty ()) {
        funcname = GS::UniString::Printf ("Sync %d finishing element with base and zone", paramToWrite.GetSize ());
        nPhase += 1;
        #if defined(AC_27) || defined(AC_28)
        ACAPI_ProcessWindow_SetNextProcessPhase (&funcname, &nPhase);
        if (ACAPI_ProcessWindow_IsProcessCanceled ()) return;
        #else
        ACAPI_Interface (APIIo_SetNextProcessPhaseID, &funcname, &nPhase);
        if (ACAPI_Interface (APIIo_IsProcessCanceledID, nullptr, nullptr)) return;
        #endif

        ClassificationFunc::SystemDict systemdict;
        ClassificationFunc::GetAllClassification (systemdict);
        GS::Array<API_Guid> syncguids;
        SyncSettings syncSettings (false, false, true, true, true, true, false);
        LoadSyncSettingsFromPreferences (syncSettings);
        GS::Array<API_Guid> rereadelem = SyncArray (syncSettings, syncguids, systemdict, propertyParams);
        if (!rereadelem.IsEmpty ()) {
            #if defined(TESTING)
            DBprnt ("===== REREAD =======");
            #endif
            SyncArray (syncSettings, rereadelem, systemdict, propertyParams);
        }
    }
}

bool Class_IsElementFinClass (const API_Guid& elGuid, const UnicGuid& finclassguids)
{
    GS::Array<GS::Pair<API_Guid, API_Guid>> systemItemPairs;
    if (ACAPI_Element_GetClassificationItems (elGuid, systemItemPairs) == NoError) {
        if (!systemItemPairs.IsEmpty ()) {
            for (const auto& cl : systemItemPairs) {
                if (finclassguids.ContainsKey (cl.second)) {
                    return true;
                }
            }
        }
    }
    return false;
}

void Param_AddUnicElementByType (const API_Guid& parentguid, const API_Guid& guid, API_ElemTypeID elemtype, UnicElementByType& elementToRead)
{
    if (parentguid == APINULLGuid) return;
    if (guid == APINULLGuid) return;
    if (!elementToRead.ContainsKey (elemtype)) {
        UnicElement el;
        elementToRead.Add (elemtype, el);
    }
    if (elementToRead.ContainsKey (elemtype)) {
        if (!elementToRead.Get (elemtype).ContainsKey (parentguid)) {
            GS::Array<API_Guid> el;
            el.Push (guid);
            elementToRead.Get (elemtype).Add (parentguid, el);
        } else {
            elementToRead.Get (elemtype).Get (parentguid).Push (guid);
        }
    }
}

void Param_AddUnicGUIDByType (const API_Guid& elGuid, API_ElemTypeID elemtype, UnicGUIDByType& guidselementToRead)
{
    if (elGuid == APINULLGuid) return;
    if (guidselementToRead.ContainsKey (elemtype)) {
        guidselementToRead.Get (elemtype).Push (elGuid);
    } else {
        GS::Array<API_Guid> z;
        z.Push (elGuid);
        guidselementToRead.Add (elemtype, z);
    }
}

API_ElemTypeID Favorite_GetType (const GS::UniString& favorite_name)
{
    GSErrCode err = NoError;
    #ifdef AC_22
    return API_ZombieElemID;
    #else
    if (favorite_name.IsEmpty ()) return API_ZombieElemID;
    API_Favorite favorite (favorite_name);
    err = ACAPI_Favorite_Get (&favorite);
    if (err == NoError) return GetElemTypeID (favorite.element.header);
    return API_ZombieElemID;
    #endif
}

MatarialToFavoriteDict Favorite_GetDict ()
{
    // Словарь с избранным
    MatarialToFavoriteDict favdict;
    MatarialToFavorite fav = {};
    // Поиск элементов по-умолчанию
    GS::Array<API_ElemTypeID> types;
    types.Push (API_WallID);
    types.Push (API_SlabID);
    types.Push (API_BeamID);
    types.Push (API_ColumnID);
    types.Push (API_ObjectID);
    GS::UniString name_ = ""; short count = 0; GS::Array<GS::UniString> names;
    for (API_ElemTypeID type : types) {
        if (Favorite_GetNum (type, &count, nullptr, &names) != NoError) continue;
        fav.type = type;
        for (GS::UniString& name : names) {
            name_ = name.ToLowerCase ();
            name_.Trim ();
            fav.name = name;
            if (!favdict.ContainsKey (name_)) favdict.Add (name_, fav);
        }
    }
    return favdict;
}

void Favorite_FindName (MatarialToFavorite& favorite, const OtdMaterial& material, const TypeOtd& type, const API_ElemTypeID& draw_type, const MatarialToFavoriteDict& favdict)
{
    API_ElemTypeID possibly_type = draw_type;
    GS::UniString fav_name = favorite.name;
    if (favdict.IsEmpty ()) return;
    if (material.smaterial.Contains ("@")) {
        GS::UniString part = material.smaterial.ToLowerCase () + '@';
        part = part.GetSubstring ('@', '@', 0);
        part.Trim ();
        if (!fav_name.IsEmpty ()) {
            GS::UniString part_full = part + "_" + fav_name;
            if (favdict.ContainsKey (part_full)) {
                if (favdict.Get (part_full).type == draw_type || favdict.Get (part_full).type == API_ObjectID) {
                    favorite = favdict.Get (part_full);
                    return;
                }
            }
        }
        if (favdict.ContainsKey (part)) {
            if (favdict.Get (part).type == draw_type || favdict.Get (part).type == API_ObjectID) {
                favorite = favdict.Get (part);
                return;
            }
        }
    }
    if (!fav_name.IsEmpty ()) {
        if (favdict.ContainsKey (fav_name)) {
            if (favdict.Get (fav_name).type == draw_type || favdict.Get (fav_name).type == API_ObjectID) {
                favorite = favdict.Get (fav_name);
                return;
            }
        }
    }
    if (favorite.type == API_ZombieElemID) {
        GS::UniString favorite_name_defult = "";
        switch (type) {
            case Wall_Main:
            case Wall_Up:
            case Wall_Down:
            case Column:
                favorite_name_defult = "smstf wall";
                break;
            case Reveal_Main:
            case Reveal_Up:
            case Reveal_Down:
                if (draw_type == API_BeamID) {
                    favorite_name_defult = "smstf reveal up";
                }
                if (draw_type == API_ColumnID) {
                    favorite_name_defult = "smstf reveal side";
                }
                if (!favdict.ContainsKey (favorite_name_defult)) {
                    favorite_name_defult = "smstf wall";
                    possibly_type = API_WallID;
                }
                break;
            case Floor:
                favorite_name_defult = "smstf floor";
                break;
            case Ceil:
                favorite_name_defult = "smstf ceil";
                break;
            default:
                return;
                break;
        }
        if (favdict.ContainsKey (favorite_name_defult)) {
            if (favdict.Get (favorite_name_defult).type == possibly_type || favdict.Get (favorite_name_defult).type == API_ObjectID) {
                favorite = favdict.Get (favorite_name_defult);
                return;
            }
        }
    }
    return;
}

bool Favorite_GetByName (const GS::UniString& favorite_name, API_Element& element)
{
    GSErrCode err = NoError;
    #ifdef AC_22
    return false;
    #else
    if (favorite_name.IsEmpty ()) return false;
    API_Favorite favorite (favorite_name);
    err = ACAPI_Favorite_Get (&favorite);
    if (err == NoError) {
        element = favorite.element;
        return true;
    }
    return false;
    #endif
}

bool Favorite_GetByName (const GS::UniString& favorite_name, API_Element& element, API_ElementMemo& memo)
{
    GSErrCode err = NoError;
    #ifdef AC_22
    return false;
    #else
    if (favorite_name.IsEmpty ()) return false;
    API_Favorite favorite (favorite_name);
    favorite.memo.New ();
    BNZeroMemory (&favorite.memo.Get (), sizeof (API_ElementMemo));
    err = ACAPI_Favorite_Get (&favorite);
    if (err == NoError) {
        element = favorite.element;
        memo = *favorite.memo;
        return true;
    }
    ACAPI_DisposeElemMemoHdls (&favorite.memo.Get ());
    return false;
    #endif
}

bool Check (const ClassificationFunc::ClassificationDict& finclass, const ParamDictValue& propertyParams, UnicGuid& finclassguids)
{
    // Проверка наличия классов
    if (finclass.IsEmpty ()) {
        GS::UniString msgString = RSGetIndString (ID_ADDON_STRINGS + isEng (), 50, ACAPI_GetOwnResModule ());
        msg_rep ("RoomBook err", msgString, APIERR_GENERAL, APINULLGuid);
        ACAPI_WriteReport (msgString, true);
        return false;
    }
    if (!finclass.ContainsKey (cls.all_class)) {
        bool msg = false;
        if (!finclass.ContainsKey (cls.floor_class)) msg = true;
        if (!finclass.ContainsKey (cls.ceil_class)) msg = true;
        if (!finclass.ContainsKey (cls.otdwall_class)) msg = true;
        if (msg) {
            GS::UniString msgString = RSGetIndString (ID_ADDON_STRINGS + isEng (), 51, ACAPI_GetOwnResModule ());
            msg_rep ("RoomBook err", msgString, APIERR_GENERAL, APINULLGuid);
        }
    }
    // Проверка наличия свойств `Sync_GUID zone`
    for (auto& cItt : propertyParams) {
        #if defined(AC_28)
        ParamValue param = cItt.value;
        #else
        ParamValue param = *cItt.value;
        #endif
        if (param.definition.description.Contains ("Sync_GUID") && param.definition.description.Contains ("zone")) {
            for (const API_Guid& classguid : param.definition.availability) {
                if (finclassguids.ContainsKey (classguid)) finclassguids.Set (classguid, true);
            }
            for (GS::HashTable<API_Guid, bool>::PairIterator cIt = finclassguids.EnumeratePairs (); cIt != NULL; ++cIt) {
                #if defined(AC_28)
                bool msg = cIt->value;
                #else
                bool msg = *cIt->value;
                #endif
                if (!msg) {
                    GS::UniString msgString = RSGetIndString (ID_ADDON_STRINGS + isEng (), 52, ACAPI_GetOwnResModule ());
                    msg_rep ("RoomBook err", msgString, APIERR_GENERAL, APINULLGuid);
                    ACAPI_WriteReport (msgString, true);
                    return false;
                }
            }
        }
    }
    return true;
}
#endif
}
