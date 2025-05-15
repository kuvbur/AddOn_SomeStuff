//------------ kuvbur 2022 ------------
#include	"ACAPinc.h"
#include	"APIEnvir.h"
#include	"Roombook.hpp"
#include	"Sync.hpp"
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

// -----------------------------------------------------------------------------
// Запись в зону информации об отделке
// -----------------------------------------------------------------------------
void RoomBook ()
{
    clock_t start, finish;
    double  duration;
    start = clock ();
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
                        OtdWall_Create_One (storyLevels, guid, zoneGuids, roomsinfo, openinginwall, guidselementToRead);
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
                Param_ToParamDict (paramDict, windowParams);
            }
            if (typeelem == API_WallID || typeelem == API_ColumnID || typeelem == API_SlabID) {
                Param_GetForBase (propertyParams, paramDict, param_composite);
            }
            if (typeelem == API_WindowID) {
                Param_ToParamDict (paramDict, roomParams);
            }
            if (!paramDict.IsEmpty ()) {
                for (const API_Guid& guid : guidselementToRead[typeelem]) {
                    ParamHelpers::AddParamDictValue2ParamDictElement (guid, paramDict, paramToRead);
                }
            }
        }
    }
    // Читаем свойства всех элементов
    ParamHelpers::ElementsRead (paramToRead, propertyParams, systemdict);

    for (OtdRooms::PairIterator cIt = roomsinfo.EnumeratePairs (); cIt != NULL; ++cIt) {
        #if defined(AC_28)
        OtdRoom& otd = cIt->value;
        API_Guid zoneGuid = cIt->key;
        #else
        OtdRoom& otd = *cIt->value;
        API_Guid zoneGuid = *cIt->key;
        #endif
        if (!paramToRead.ContainsKey (zoneGuid)) continue;
        // Заполняем данные для зон
        Param_SetToRooms (otd, paramToRead, roomParams);
        // Заполняем данные для отделочных стен (состав)
        Param_SetToBase (otd, paramToRead, param_composite);
        // Заполняем данные для окон
        Param_SetToWindows (otd, paramToRead, windowParams);
    }
    // Расчёт пола и потолка
    Floor_Create_All (storyLevels, roomsinfo, guidselementToRead, paramToRead);
    // Создание стенок для откосов
    OpeningReveals_Create_All (roomsinfo);
    // Разделение стенок по высотам зон
    OtdWall_Delim_All (roomsinfo);
    // Назначение материала
    SetMaterialByType (roomsinfo);
    GS::Array<API_Guid> deletelist; // Список элементов для удаления
    // Получаем список существующих элементов отделки для обрабатываемых зон
    zones.Clear ();
    ParamDictElement paramToWrite; // Параметры для записи в зоны и элементы отделки
    for (OtdRooms::PairIterator cIt = roomsinfo.EnumeratePairs (); cIt != NULL; ++cIt) {
        #if defined(AC_28)
        OtdRoom& otd = cIt->value;
        #else
        OtdRoom& otd = *cIt->value;
        #endif
        if (otd.isValid && otd.create_all_elements) {
            zones.Push (otd.zone_guid);
        }
        if (otd.isValid) WriteOtdDataToRoom (otd, paramToWrite, paramToRead);
    }
    GS::HashTable<API_Guid, UnicGuid> exsistotdelements;
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
    GS::UniString time = GS::UniString::Printf (" %.3f s", duration);
    msg_rep ("RoomBook", time, NoError, APINULLGuid);
}

void WriteOtdDataToRoom (OtdRoom& otd, ParamDictElement& paramToWrite, ParamDictElement& paramToRead)
{
    if (!paramToRead.ContainsKey (otd.zone_guid)) {
        return;
    }
    OtdMaterialAreaDictByType dct; // Основной словарь
    // Разбивка по отделочным слоям, вычисление площадей и добавление их в словарь по типам отделки
    for (UInt32 i = 0; i < otd.otdwall.GetSize (); i++) {
        OtdWall& otdw = otd.otdwall[i];
        TypeOtd t = otdw.type;
        double area = OtdWall_GetArea (otdw);
        for (UInt32 j = 0; j < otdw.base_composite.GetSize (); j++) {
            GS::UniString mat = otdw.base_composite[j].val;
            WriteOtdDataToRoom_AddValue (dct, t, mat, area);
        }
    }
    GS::HashTable<TypeOtd, GS::UniString> paramnamebytype;
    if (dct.ContainsKey (Wall_Up)) paramnamebytype.Add (Wall_Up, otd.om_up.rawname); // Отделка стен выше потолка
    paramnamebytype.Add (Wall_Main, otd.om_main.rawname); // Отделка стен основная
    paramnamebytype.Add (Wall_Down, otd.om_down.rawname); // Отделка низа стен
    paramnamebytype.Add (Column, otd.om_column.rawname); // Отделка колонн
    if (dct.ContainsKey (Reveal_Main)) paramnamebytype.Add (Reveal_Main, otd.om_reveals.rawname); // Отделка откосов
    if (dct.ContainsKey (Floor)) paramnamebytype.Add (Floor, otd.om_floor.rawname); // Отделка пола
    paramnamebytype.Add (Ceil, otd.om_ceil.rawname); // Отделка потолка

    // Настройки для форматирования текста в таблицу
    short font = GetFontIndex ();
    double fontsize = 2.5;
    double width_mat = 40;
    double width_area = 20;
    double width = width_mat + width_area;
    Int32 addspace = 0;
    GS::UniString space = " ";
    double width_space = GetTextWidth (font, fontsize, space);
    addspace = (Int32) (width_area / width_space);
    GS::UniString space_line = GS::UniString::Printf ("%s", std::string (addspace, ' ').c_str ());
    GS::UniString delim = "-";
    double width_delim = GetTextWidth (font, fontsize, delim);
    addspace = (Int32) ((width - width_space) / width_delim);
    GS::UniString delim_line = GS::UniString::Printf ("%s ", std::string (addspace, '-').c_str ());

    for (auto& cItt : paramnamebytype) {
        #if defined(AC_28)
        TypeOtd t = cItt.key;
        GS::UniString rawname = cItt.value;
        #else
        TypeOtd t = *cItt.key;
        GS::UniString rawname = *cItt.value;
        #endif
        // Проверяем - есть ли считанное свойство в зоне для записи
        if (!paramToRead.Get (otd.zone_guid).ContainsKey (rawname)) continue;
        ParamValue paramtow = paramToRead.Get (otd.zone_guid).Get (rawname);
        GS::UniString old_val = paramtow.val.uniStringValue;
        GS::UniString new_val = "";
        // Если такой тип отделки есть в словаре - записываем послойно материалы и площади
        if (dct.ContainsKey (t)) {
            OtdMaterialAreaDict& dcta = dct.Get (t);
            for (auto& cIt : dcta) {
                #if defined(AC_28)
                double area = cIt.value;
                GS::UniString mat = cIt.key;
                #else
                double area = *cIt.value;
                GS::UniString mat = *cIt.key;
                #endif
                GS::UniString area_sring = GS::UniString::Printf ("%.2f", area);
                double w_area = GetTextWidth (font, fontsize, area_sring);
                if (w_area < width_area) {
                    Int32 addspace = (Int32) ((width_area - w_area) / width_space);
                    if (addspace > 1) {
                        area_sring = GS::UniString::Printf ("%s", std::string (addspace - 1, ' ').c_str ()) + area_sring;
                    }
                    area_sring = area_sring + " ";
                }
                if (!new_val.IsEmpty ()) new_val = new_val + delim_line;
                GS::Array<GS::UniString> lines_mat = DelimTextLine (font, fontsize, width_mat, mat);
                for (UInt32 j = 0; j < lines_mat.GetSize (); j++) {
                    new_val = new_val + lines_mat[j];
                    if (j == lines_mat.GetSize () - 1) {
                        new_val = new_val + area_sring;
                    } else {
                        new_val = new_val + space_line;
                    }
                }
            }
        }
        // Если уже есть такая запись - проверим, не затрётся ли она
        if (paramToWrite.ContainsKey (otd.zone_guid)) {
            if (paramToWrite.Get (otd.zone_guid).ContainsKey (rawname)) {
                if (!new_val.IsEmpty ()) {
                    new_val = new_val + delim_line + paramToWrite.Get (otd.zone_guid).Get (rawname).val.uniStringValue;
                } else {
                    new_val = paramToWrite.Get (otd.zone_guid).Get (rawname).val.uniStringValue;
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

void WriteOtdDataToRoom_AddValue (OtdMaterialAreaDictByType& dct, const TypeOtd& t, GS::UniString& mat, const double& area)
{
    if (area < 0.000001) return;
    if (!dct.ContainsKey (t)) {
        OtdMaterialAreaDict dcta;
        dcta.Add (mat, area);
        dct.Add (t, dcta);
        return;
    } else {
        if (!dct.Get (t).ContainsKey (mat)) {
            dct.Get (t).Add (mat, area);
        } else {
            double area_sum = dct.Get (t).Get (mat) + area;
            dct.Get (t).Set (mat, area_sum);
        }
    }
}

// -----------------------------------------------------------------------------
// Получение информации из зоны о полгионах и находящейся в ней элементах
// -----------------------------------------------------------------------------
bool CollectRoomInfo (const Stories& storyLevels, API_Guid& zoneGuid, OtdRoom& roominfo, UnicElementByType& elementToRead, GS::HashTable<API_Guid, GS::Array<API_Guid>>& slabsinzone)
{
    GSErrCode err;
    API_Element element = {};
    API_Element zoneelement = {};
    BNZeroMemory (&zoneelement, sizeof (API_Element));
    zoneelement.header.guid = zoneGuid;
    err = ACAPI_Element_Get (&zoneelement);
    if (err != NoError) {
        msg_rep ("CollectRoomInfo err", "ACAPI_Element_Get zone", err, zoneGuid);
        return false;
    }
    API_ElementMemo zonememo;
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
    API_RoomRelation relData;
    err = ACAPI_Element_GetRelations (zoneGuid, API_ZombieElemID, &relData);
    if (err != NoError) {
        msg_rep ("CollectRoomInfo err", "ACAPI_Element_GetRelations zone", err, zoneGuid);
        return false;
    }
    roominfo.height = zoneelement.zone.roomHeight;
    roominfo.zBottom = GetzPos (zoneelement.zone.roomBaseLev, zoneelement.header.floorInd, storyLevels);

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
            searchPars.z = BiggestDouble;
            #if defined(AC_27) || defined(AC_28) || defined(AC_26)
            searchPars.type.typeID = typeelem;
            #else
            searchPars.typeID = typeelem;
            #endif
            #if defined(AC_27) || defined(AC_28)
            err = ACAPI_Element_SearchElementByCoord (&searchPars, &elGuid);
            #else
            err = ACAPI_Goodies (APIAny_SearchElementByCoordID, &searchPars, &elGuid);
            #endif
            if (err == NoError) {
                flag = true;
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
            break;
        case API_DoorID:
            wallguid = element.door.owner;
            op.height = element.door.openingBase.height;
            op.width = element.door.openingBase.width;
            op.lower = element.door.lower;
            op.objLoc = element.door.objLoc;
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
void OtdWall_Create_One (const Stories& storyLevels, API_Guid& elGuid, GS::Array<API_Guid>& zoneGuids, OtdRooms& roomsinfo, GS::HashTable<API_Guid, GS::Array<OtdOpening>>& openinginwall, UnicGUIDByType& guidselementToRead)
{
    GSErrCode err;
    API_Element element = {};
    element.header.guid = elGuid;
    err = ACAPI_Element_Get (&element);
    if (err != NoError) {
        msg_rep ("OtdWall_Create_One err", "ACAPI_Element_Get", err, elGuid);
        return;
    }
    Point2D wbegC = { element.wall.begC.x, element.wall.begC.y };
    Point2D wendC = { element.wall.endC.x, element.wall.endC.y };
    Sector walledge = { wbegC , wendC };
    double dx = -element.wall.endC.x + element.wall.begC.x;
    double dy = -element.wall.endC.y + element.wall.begC.y;
    double dr = sqrt (dx * dx + dy * dy);
    double zBottom = GetzPos (element.wall.bottomOffset, element.header.floorInd, storyLevels);
    if (dr < min_dim) {
        #if defined(TESTING)
        DBprnt ("OtdWall_Create_One err", "walledge.GetLength ()<min_dim");
        #endif
        return;
    }
    GS::Optional<UnitVector_2D>	walldir = walledge.GetDirection ();
    if (!walldir.HasValue ()) {
        #if defined(TESTING)
        DBprnt ("OtdWall_Create_One err", "!walldir.HasValue");
        #endif
        return;
    }
    for (API_Guid zoneGuid : zoneGuids) {
        if (!roomsinfo.ContainsKey (zoneGuid)) {
            #if defined(TESTING)
            DBprnt ("OtdWall_Create_One err", "!roomsinfo.ContainsKey (zoneGuid)");
            #endif
            continue;
        }
        OtdRoom& roominfo = roomsinfo.Get (zoneGuid);
        if (!roominfo.isValid) continue;
        bool flag_find = false;
        if (roominfo.walledges.ContainsKey (elGuid)) {
            GS::Array<Sector>& walledges = roominfo.walledges.Get (elGuid);
            for (Sector& roomedge : roominfo.walledges[elGuid]) {
                bool is_fliped = false;
                OtdWall wallotd;
                GS::Optional<Geometry::Line2D> roomeline = roomedge.AsLine ();
                if (!roomeline.HasValue ()) {
                    #if defined(TESTING)
                    DBprnt ("OtdWall_Create_One err", "!roomeline.HasValue ()");
                    #endif
                    continue;
                }
                GS::Optional<UnitVector_2D>	roomedgedir = roomedge.GetDirection ();
                if (!roomedgedir.HasValue ()) {
                    #if defined(TESTING)
                    DBprnt ("OtdWall_Create_One err", "!roomedgedir.HasValue");
                    #endif
                    continue;
                }
                walledge = roomedge;
                double walledgedx = -walledge.c1.x + walledge.c2.x;
                double walledgedy = -walledge.c1.y + walledge.c2.y;
                double walledgedr = sqrt (walledgedx * walledgedx + walledgedy * walledgedy);
                if (walledgedr > min_dim) {
                    wallotd.base_guid = elGuid;
                    wallotd.height = element.wall.height;
                    wallotd.zBottom = zBottom;
                    wallotd.base_flipped = is_fliped;
                    wallotd.begC = { walledge.c1.x, walledge.c1.y };
                    wallotd.endC = { walledge.c2.x, walledge.c2.y };
                    wallotd.material = roominfo.om_zone;
                    wallotd.base_type = API_WallID;
                    wallotd.type = Wall_Main;
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
                    DBprnt ("OtdWall_Create_One err", "inxedge > roomedges.restedges.GetSize ()");
                    #endif
                    continue;
                }
                Sector roomedge = roominfo.edges.Get (inxedge);
                GS::Optional<Geometry::Line2D> roomeline = roomedge.AsLine ();
                if (!roomeline.HasValue ()) {
                    #if defined(TESTING)
                    DBprnt ("OtdWall_Create_One err", "!roomeline.HasValue ()");
                    #endif
                    continue;
                }
                GS::Optional<UnitVector_2D>	roomedgedir = roomedge.GetDirection ();
                if (!roomedgedir.HasValue ()) {
                    #if defined(TESTING)
                    DBprnt ("OtdWall_Create_One err", "!roomedgedir.HasValue");
                    #endif
                    continue;
                }
                bool is_fliped = false;
                OtdWall wallotd;
                if (walldir.Get ().IsParallelWith (roomedgedir.Get ())) {
                    Point2D begedge = wbegC;
                    Point2D endedge = wendC;
                    if (!is_equal (wpart.tEnd, 0) || !is_equal (wpart.tBeg, 0)) {
                        if (!is_equal (wpart.tBeg, 0) && !is_equal (wpart.tBeg, dr)) {
                            double lambda = wpart.tBeg / dr;
                            begedge.x = wbegC.x - dx * lambda;
                            begedge.y = wbegC.y - dy * lambda;
                        }
                        if (!is_equal (wpart.tEnd, 0) && !is_equal (wpart.tEnd, dr)) {
                            double lambda = wpart.tEnd / dr;
                            endedge.x = wbegC.x - dx * lambda;
                            endedge.y = wbegC.y - dy * lambda;
                        }
                    }
                    begedge = roomeline.Get ().ProjectPoint (begedge);
                    endedge = roomeline.Get ().ProjectPoint (endedge);
                    is_fliped = !walldir.Get ().IsEqualWith (roomedgedir.Get ());
                    if (is_fliped) {
                        walledge = { endedge , begedge };
                    } else {
                        walledge = { begedge , endedge };
                    }
                    // Ищем проёмы в этом участке стены
                    if (openinginwall.ContainsKey (elGuid)) {
                        for (const OtdOpening& op : openinginwall[elGuid]) {
                            // Проверяем - находится ли этот проём на заданном участке стены
                            if (op.objLoc <= wpart.tEnd + op.width / 2 && op.objLoc >= wpart.tBeg - op.width / 2) {
                                OtdOpening opinwall;
                                opinwall.base_guid = op.base_guid;
                                opinwall.width = op.width;
                                opinwall.base_reveal_width = op.base_reveal_width;
                                opinwall.lower = op.lower;
                                opinwall.zBottom = zBottom + op.lower;
                                double dw = 0; // Уменьшение ширины в случае, если стена попадает на часть проёма
                                if (op.objLoc > wpart.tEnd - op.width / 2) {
                                    dw = wpart.tEnd - op.objLoc - op.width / 2;
                                    opinwall.width = opinwall.width + dw;
                                }
                                if (op.objLoc < wpart.tBeg + op.width / 2) {
                                    dw = wpart.tBeg - op.objLoc + op.width / 2;
                                    opinwall.width = opinwall.width - dw;
                                }
                                opinwall.height = op.height;
                                if (!is_fliped) {
                                    opinwall.objLoc = op.objLoc - wpart.tBeg + dw / 2;
                                } else {
                                    opinwall.objLoc = dr - op.objLoc - (dr - wpart.tEnd) + dw / 2;
                                }
                                wallotd.openings.Push (opinwall);
                            }
                        }
                    }
                    if (element.wall.flipped) is_fliped = !is_fliped;
                } else {
                    walledge = roomedge; // Торец стены
                }
                double walledgedx = -walledge.c1.x + walledge.c2.x;
                double walledgedy = -walledge.c1.y + walledge.c2.y;
                double walledgedr = sqrt (walledgedx * walledgedx + walledgedy * walledgedy);
                if (walledgedr > min_dim) {
                    wallotd.base_guid = elGuid;
                    wallotd.height = element.wall.height;
                    wallotd.base_th = element.wall.thickness;
                    wallotd.zBottom = zBottom;
                    wallotd.base_flipped = is_fliped;
                    wallotd.begC = { walledge.c1.x, walledge.c1.y };
                    wallotd.endC = { walledge.c2.x, walledge.c2.y };
                    wallotd.material = roominfo.om_main;
                    wallotd.base_type = API_WallID;
                    wallotd.type = Wall_Main;
                    roominfo.otdwall.Push (wallotd);
                    flag_find = true;
                }
            }
        }
        if (flag_find) Param_AddUnicGUIDByType (elGuid, API_WallID, guidselementToRead);
    }
}

// -----------------------------------------------------------------------------
// Создание стен-отделок для колонны
// -----------------------------------------------------------------------------
void Column_Create_One (const Stories& storyLevels, API_Guid& elGuid, GS::Array<API_Guid>& zoneGuids, OtdRooms& roomsinfo, UnicGUIDByType& guidselementToRead)
{
    GSErrCode err;
    API_Element element = {};
    API_ElementMemo segmentmemo = {};
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
                if (roomsinfo.ContainsKey (zoneGuid)) {
                    OtdRoom& roominfo = roomsinfo.Get (zoneGuid);
                    if (!roominfo.isValid) continue;
                    Sector cdge;
                    bool find = Edge_FindOnEdge (coledge, roominfo.edges, cdge);
                    if (!find) find = Edge_FindOnEdge (coledge, roominfo.columnedges, cdge);
                    if (find) {
                        OtdWall wallotd;
                        wallotd.base_guid = elGuid;
                        wallotd.height = element.column.height;
                        wallotd.zBottom = zBottom;
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
        }
    }
    if (flag_find) Param_AddUnicGUIDByType (elGuid, API_ColumnID, guidselementToRead);
    ACAPI_DisposeElemMemoHdls (&segmentmemo);
}

// -----------------------------------------------------------------------------
// Обработка полов и потолков
// -----------------------------------------------------------------------------
void Floor_FindInOneRoom (const Stories& storyLevels, API_Guid& elGuid, GS::Array<API_Guid>& zoneGuids, OtdRooms& roomsinfo, UnicGUIDByType& guidselementToRead)
{
    GSErrCode err;
    API_Element element = {};
    API_ElementMemo memo = {};
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

    Param_FindPropertyInParams (propertyParams, zoneparams);
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
    GS::UniString rawName_onoff = "";
    GS::UniString rawName_desc = "";
    for (auto& cItt : propertyParams) {
        #if defined(AC_28)
        ParamValue param = cItt.value;
        #else
        ParamValue param = *cItt.value;
        #endif
        if (param.definition.description.Contains (propdesc_onoff)) rawName_onoff = param.rawName;
        if (param.definition.description.Contains (propdesc_desc)) rawName_desc = param.rawName;
        if (!rawName_onoff.IsEmpty () && !rawName_desc.IsEmpty ()) break;
    }
    if (rawName_onoff.IsEmpty () || rawName_desc.IsEmpty ()) return;
    // Подготавливаем свойство для чтения из слоёв многослойки
    GS::UniString rawName = "{@material:layers,20}";
    param_composite.rawName = rawName;
    param_composite.val.uniStringValue = "l[" + rawName_onoff + ":" + rawName_desc + "]";
    param_composite.fromMaterial = true;
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
    zoneparam_name = "material_main";
    zoneparam.rawnames.Push ("{@gdl:votw}");
    zoneparam.rawnames.Push ("some_stuff_fin_main_material");
    zoneparams.Add (zoneparam_name, zoneparam);
    zoneparam.rawnames.Clear ();

    // Высота до подвесного потолка
    zoneparam_name = "height_main";
    zoneparam.rawnames.Push ("{@gdl:hroom_pot}");
    zoneparam.rawnames.Push ("some_stuff_fin_main_height");
    zoneparams.Add (zoneparam_name, zoneparam);
    zoneparam.rawnames.Clear ();

    // Отделка стен выше потолка
    zoneparam_name = "material_up";
    zoneparam.rawnames.Push ("{@gdl:votw2}");
    zoneparam.rawnames.Push ("some_stuff_fin_up_material");
    zoneparams.Add (zoneparam_name, zoneparam);
    zoneparam.rawnames.Clear ();

    // Отделка низа стен/колонн
    zoneparam_name = "material_down";
    zoneparam.rawnames.Push ("{@gdl:votp}");
    zoneparam.rawnames.Push ("some_stuff_fin_down_material");
    zoneparams.Add (zoneparam_name, zoneparam);
    zoneparam.rawnames.Clear ();

    // Высота нижних панелей
    zoneparam_name = "height_down";
    zoneparam.rawnames.Push ("{@gdl:hpan}");
    zoneparam.rawnames.Push ("{@gdl:z17}");
    zoneparam.rawnames.Push ("some_stuff_fin_down_height");
    zoneparams.Add (zoneparam_name, zoneparam);
    zoneparam.rawnames.Clear ();

    // Отделка колонн
    zoneparam_name = "material_column";
    zoneparam.rawnames.Push ("{@gdl:votc}");
    zoneparam.rawnames.Push ("some_stuff_fin_column_material");
    zoneparams.Add (zoneparam_name, zoneparam);
    zoneparam.rawnames.Clear ();

    // Отделка потолка
    zoneparam_name = "om_ceil";
    zoneparam.rawnames.Push ("{@gdl:vots}");
    zoneparam.rawnames.Push ("some_stuff_fin_ceil_material");
    zoneparams.Add (zoneparam_name, zoneparam);
    zoneparam.rawnames.Clear ();

    // Отделка потолка - запись результатов
    zoneparam_name = "om_ceil.rawname";
    zoneparam.rawnames.Push ("some_stuff_fin_ceil_result");
    zoneparams.Add (zoneparam_name, zoneparam);
    zoneparam.rawnames.Clear ();

    // Отделка откосов - запись результатов
    zoneparam_name = "om_reveals.rawname";
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

    Param_FindPropertyInParams (propertyParams, zoneparams);
    return zoneparams;
}

void Param_FindPropertyInParams (ParamDictValue& propertyParams, ReadParams& zoneparams)
{
    if (zoneparams.IsEmpty ()) {
        return;
    }
    GS::UniString param_name = "";
    for (auto& p : zoneparams) {
        #if defined(AC_28)
        ReadParam param = p.value;
        #else
        ReadParam param = *p.value;
        #endif
        UInt32 n = param.rawnames.GetSize ();
        for (UInt32 i = 0; i < n; ++i) {
            param_name = param.rawnames[i];
            if (!param_name.Contains ("{@")) {
                bool flag_find = false;
                for (auto& cItt : propertyParams) {
                    #if defined(AC_28)
                    ParamValue parameters = cItt.value;
                    #else
                    ParamValue parameters = *cItt.value;
                    #endif
                    if (parameters.definition.description.Contains (param_name)) {
                        param.rawnames[i] = parameters.rawName;
                        flag_find = true;
                        break;
                    }
                }
                if (!flag_find) {
                    param.rawnames.Delete (i);
                }
            }
        }
    }
}

void Param_ReadParams (const API_Guid& elGuid, ParamDictElement& paramToRead, ReadParams& zoneparams)
{
    if (!paramToRead.ContainsKey (elGuid)) {
        return;
    }
    if (zoneparams.IsEmpty ()) {
        return;
    }
    GS::UniString param_name = "";
    for (auto& p : zoneparams) {
        #if defined(AC_28)
        ReadParam param = p.value;
        #else
        ReadParam param = *p.value;
        #endif
        param.isValid = false;
        for (UInt32 i = 0; i < param.rawnames.GetSize (); ++i) {
            param_name = param.rawnames[i];
            if (!paramToRead.Get (elGuid).ContainsKey (param_name)) continue;
            if (paramToRead.Get (elGuid).Get (param_name).isValid) {
                param.val = paramToRead.Get (elGuid).Get (param_name).val;
                break;
            }
        }
    }
}

// -----------------------------------------------------------------------------
// Запись прочитанных свойств в зону
// -----------------------------------------------------------------------------
void Param_SetToRooms (OtdRoom& roominfo, ParamDictElement& paramToRead, ReadParams& readparams)
{
    for (OtdRooms::PairIterator cIt = roomsinfo.EnumeratePairs (); cIt != NULL; ++cIt) {
        #if defined(AC_28)
        OtdRoom& otd = cIt->value;
        API_Guid zoneGuid = cIt->key;
        #else
        OtdRoom& otd = *cIt->value;
        API_Guid zoneGuid = *cIt->key;
        #endif
        bool isParamRead = false;
        // Чтение прочитанных свойств
        if (paramToRead.ContainsKey (zoneGuid)) {
            ParamDictValue& params = paramToRead.Get (zoneGuid);
            isParamRead = Param_SetToRoomsMyZone (params, otd);
            if (!isParamRead) isParamRead = Param_SetToRoomsFromProperty (params, otd);
            if (!isParamRead) {
                msg_rep ("Param_SetToRooms err", "!isParamRead", NoError, zoneGuid);
                continue;
            }
            int n_param = 12;
            for (auto& cItt : params) {
                #if defined(AC_28)
                ParamValue& param = cItt.value;
                #else
                ParamValue& param = *cItt.value;
                #endif
                if (!param.isValid) continue;
                if (param.definition.description.Contains ("some_stuff_fin_ceil_result")) {
                    otd.om_ceil.rawname = param.rawName;
                    n_param -= 1;
                }
                if (param.definition.description.Contains ("some_stuff_fin_reveals_result")) {
                    otd.om_reveals.rawname = param.rawName;
                    n_param -= 1;
                }
                if (param.definition.description.Contains ("some_stuff_fin_up_result")) {
                    otd.om_up.rawname = param.rawName;
                    n_param -= 1;
                }
                if (param.definition.description.Contains ("some_stuff_fin_main_result")) {
                    otd.om_main.rawname = param.rawName;
                    n_param -= 1;
                }
                if (param.definition.description.Contains ("some_stuff_fin_down_result")) {
                    otd.om_down.rawname = param.rawName;
                    n_param -= 1;
                }
                if (param.definition.description.Contains ("some_stuff_fin_column_result")) {
                    otd.om_column.rawname = param.rawName;
                    n_param -= 1;
                }
                if (param.definition.description.Contains ("some_stuff_fin_create_elements")) {
                    otd.create_all_elements = param.val.boolValue;
                    n_param -= 1;
                }
                if (param.definition.description.Contains ("some_stuff_fin_create_column")) {
                    otd.create_column_elements = param.val.boolValue;
                    n_param -= 1;
                }
                if (param.definition.description.Contains ("some_stuff_fin_create_wall")) {
                    otd.create_wall_elements = param.val.boolValue;
                    n_param -= 1;
                }
                if (param.definition.description.Contains ("some_stuff_fin_create_floor")) {
                    otd.create_floor_elements = param.val.boolValue;
                    n_param -= 1;
                }
                if (param.definition.description.Contains ("some_stuff_fin_create_ceil")) {
                    otd.create_ceil_elements = param.val.boolValue;
                    n_param -= 1;
                }
                if (param.definition.description.Contains ("some_stuff_fin_create_reveal")) {
                    otd.create_reveal_elements = param.val.boolValue;
                    n_param -= 1;
                }
                if (n_param < 1) {
                    break;
                }
            }
        } else {
            msg_rep ("Param_SetToRooms err", "!paramToRead.ContainsKey (zoneGuid)", NoError, zoneGuid);
        }
        if (otd.create_all_elements) {
            otd.create_ceil_elements = true; // Создавать элементы отделки потолка
            otd.create_floor_elements = true; // Создавать элементы отделки пола
            otd.create_wall_elements = true; // Создавать элементы отделки стен
            otd.create_column_elements = true; // Создавать элементы отделки колонн
            otd.create_reveal_elements = true; // Создавать элементы отделки откосов
        }
        // Заполнение непрочитанных
        // Высоты
        if (otd.height_main > otd.height) otd.height_main = otd.height;
        if (otd.height_main < 0.1) {
            otd.height_main = otd.height - otd.height_down;
            otd.height_up = 0;
        } else {
            otd.height_up = otd.height - otd.height_main;
            otd.height_main = otd.height_main - otd.height_down;
        }
        if (!isParamRead) otd.isValid = false;
        if (otd.height < 0.0001) otd.isValid = false;
    }
}

// -----------------------------------------------------------------------------
// Запись прочитанных свойств в отделочные стены
// -----------------------------------------------------------------------------
void Param_SetToBase (OtdRoom& roominfo, ParamDictElement& paramToRead, ParamValue& param_composite)
{
    GS::Array<OtdWall>& otd = roominfo.otdwall; // Массив отделочных поверхностей в зоне
    for (UInt32 i = 0; i < otd.GetSize (); i++) {
        OtdWall& otdw = otd[i]; // Стена-отделка
        API_Guid base_guid = otdw.base_guid;
        if (!paramToRead.ContainsKey (base_guid)) {
            #if defined(TESTING)
            DBprnt ("Param_SetToBase err", "!paramToRead.ContainsKey(base_guid)");
            #endif
            continue;
        }
        // Прочитанные параметры базового компонента
        ParamDictValue& baseparam = paramToRead.Get (otdw.base_guid);
        // Состав базового компонента
        ParamValue& base_composite = baseparam.Get (param_composite.rawName);
        // Отфильтрованные слои отделки
        GS::Array<ParamValueComposite> otdcpmpoosite;
        //TODO Дописать определение вывода слоя в ведомость отделки исходя из свойств
        Int32 ncomp = base_composite.composite.GetSize ();
        bool flag_core = false;
        if (otdw.base_flipped) {
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
        for (UInt32 j = 0; j < otdcpmpoosite.GetSize (); j++) {
            GS::UniString v = otdcpmpoosite[j].val.GetPrefix (otdcpmpoosite[j].val.GetLength () - 1);
            v = v.GetSuffix (v.GetLength () - 4);
            otdcpmpoosite[j].val = v;
        }
        otdw.base_composite = otdcpmpoosite;
    }
}

// -----------------------------------------------------------------------------
// Задание прочитанных параметров для окон
// -----------------------------------------------------------------------------
void Param_SetToWindows (OtdRoom& roominfo, ParamDictElement& paramToRead, ReadParams& readparams)
{
    if (!roominfo.otdwall.IsEmpty ()) {
        return;
    }
    #if defined(TESTING)
    DBprnt ("Param_SetToWindows", "start");
    #endif
    double sill = 0;
    double frame = 0;
    double max_plaster_th = 0;
    for (OtdWall& otdw : roominfo.otdwall) {
        if (otdw.openings.IsEmpty ()) continue;
        for (OtdOpening& op : otdw.openings) {
            API_Guid base_guid = op.base_guid;
            if (!paramToRead.ContainsKey (base_guid)) {
                #if defined(TESTING)
                DBprnt ("Param_SetToWindows err", "!paramToRead.ContainsKey(base_guid)");
                #endif
                continue;
            }
            if (!paramToRead.Get (op.base_guid).ContainsKey (g.frame)) break;
            if (!paramToRead.Get (op.base_guid).ContainsKey (g.sill)) break;
            if (!paramToRead.Get (op.base_guid).Get (g.frame).isValid) break;
            if (!paramToRead.Get (op.base_guid).Get (g.sill).isValid) break;
            sill = paramToRead.Get (op.base_guid).Get (g.sill).val.doubleValue;
            frame = paramToRead.Get (op.base_guid).Get (g.frame).val.doubleValue;
            // Расчёт ширины откосы
            op.base_reveal_width = otdw.base_th - sill - frame;

            if (!paramToRead.Get (op.base_guid).ContainsKey (g.useWallFinishSkin)) break;
            if (!paramToRead.Get (op.base_guid).ContainsKey (g.maxPlasterThk)) break;
            if (!paramToRead.Get (op.base_guid).ContainsKey (g.AutoTurnIn)) break;
            if (!paramToRead.Get (op.base_guid).ContainsKey (g.bOverIn)) break;
            if (!paramToRead.Get (op.base_guid).ContainsKey (g.plaster_show_3D)) break;
            if (!paramToRead.Get (op.base_guid).ContainsKey (g.plaster_show_2D)) break;
            if (!paramToRead.Get (op.base_guid).Get (g.useWallFinishSkin).isValid) break;
            if (!paramToRead.Get (op.base_guid).Get (g.maxPlasterThk).isValid) break;
            if (!paramToRead.Get (op.base_guid).Get (g.AutoTurnIn).isValid) break;
            if (!paramToRead.Get (op.base_guid).Get (g.bOverIn).isValid) break;
            if (!paramToRead.Get (op.base_guid).Get (g.plaster_show_3D).isValid) break;
            if (!paramToRead.Get (op.base_guid).Get (g.plaster_show_2D).isValid) break;

            // Поправка на поворот отделки
            if (paramToRead.Get (op.base_guid).Get (g.plaster_show_3D).val.boolValue && paramToRead.Get (op.base_guid).Get (g.plaster_show_2D).val.boolValue) {
                max_plaster_th = 0;
                UInt32 nsl = 0;
                if (!paramToRead.Get (op.base_guid).Get (g.AutoTurnIn).val.boolValue) {
                    nsl = paramToRead.Get (op.base_guid).Get (g.bOverIn).val.intValue;
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
    }
    #if defined(TESTING)
    DBprnt ("Param_SetToWindows", "end");
    #endif
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
// Обработка потолков и полов в зоне
// -----------------------------------------------------------------------------
void Floor_Create_All (const Stories& storyLevels, OtdRooms& roomsinfo, UnicGUIDByType& guidselementToRead, ParamDictElement& paramToRead)
{
    if (!guidselementToRead.ContainsKey (API_ZoneID)) {
        #if defined(TESTING)
        DBprnt ("Floor_Create_All err", "!guidselementToRead.ContainsKey (API_ZoneID)");
        #endif
        return;
    }
    for (const API_Guid& guid : guidselementToRead[API_ZoneID]) {
        if (!roomsinfo.ContainsKey (guid)) {
            #if defined(TESTING)
            DBprnt ("Floor_Create_All err", "!roomsinfo.ContainsKey (guid)");
            #endif
            continue;
        }
        OtdRoom& roominfo = roomsinfo.Get (guid);
        if (!roominfo.has_floor || !roominfo.has_ceil) continue;
        if (roominfo.has_floor) {
            roominfo.poly.zBottom = roominfo.zBottom;
            if (!roominfo.floorslab.IsEmpty ()) {
                Floor_Create_One (storyLevels, roominfo.poly, roominfo.floorslab, roominfo.otdslab, roominfo.otdwall, paramToRead, true);
            } else {
                roominfo.otdslab.Push (roominfo.poly);
            }
        }
        if (roominfo.has_ceil) {
            roominfo.poly.zBottom = roominfo.zBottom + roominfo.height_main + roominfo.height_down;
            roominfo.poly.material = roominfo.om_ceil;
            if (!roominfo.ceilslab.IsEmpty ()) {
                Floor_Create_One (storyLevels, roominfo.poly, roominfo.ceilslab, roominfo.otdslab, roominfo.otdwall, paramToRead, false);
            } else {
                roominfo.otdslab.Push (roominfo.poly);
            }
        }
    }
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

void Floor_Create_One (const Stories& storyLevels, OtdSlab& poly, GS::Array<API_Guid>& slabGuids, GS::Array<OtdSlab>& otdslabs, GS::Array<OtdWall>& otdwall, ParamDictElement& paramToRead, bool on_top)
{
    Geometry::Polygon2D roompolygon = poly.poly;
    for (API_Guid& slabGuid : slabGuids) {
        GSErrCode err;
        API_Element element = {};
        API_ElementMemo memo = {};
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
                            if (on_top) {
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
                            wallotd.type = Wall_Main;
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
            otdslab.zBottom = zBottom;
            if (on_top) {
                otdslab.zBottom += otd_thickness;
            } else {
                otdslab.zBottom -= otd_thickness;
            }
            otdslab.material = poly.material;
            otdslab.type = Floor;
            if (on_top) {
                otdslab.type = Ceil;
                otdslab.zBottom = zUp;
            }
            otdslabs.Push (otdslab);
        }
        ACAPI_DisposeElemMemoHdls (&memo);
    }
    OtdSlab otdslab = poly;
    if (on_top) {
        otdslab.type = Ceil;
        otdslab.zBottom += otd_thickness;
    } else {
        otdslab.type = Floor;
        otdslab.zBottom -= otd_thickness;
    }
    otdslab.poly = roompolygon;
    otdslabs.Push (otdslab);
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
// Получение площади отделочной стены с учётом отверстий
// -----------------------------------------------------------------------------
double OtdWall_GetArea (const OtdWall& otdw)
{
    double dx = -otdw.endC.x + otdw.begC.x;
    double dy = -otdw.endC.y + otdw.begC.y;
    double dr = sqrt (dx * dx + dy * dy);
    double area = dr * otdw.height;
    if (is_equal (dr, 0)) return 0;
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
// Создание стенок для откосов проёмов
// -----------------------------------------------------------------------------
void OpeningReveals_Create_All (OtdRooms& roomsinfo)
{
    #if defined(TESTING)
    DBprnt ("OpeningReveals_Create_All", "start");
    #endif
    for (OtdRooms::PairIterator cIt = roomsinfo.EnumeratePairs (); cIt != NULL; ++cIt) {
        #if defined(AC_28)
        OtdRoom& otd = cIt->value;
        API_Guid zoneGuid = cIt->key;
        #else
        OtdRoom& otd = *cIt->value;
        API_Guid zoneGuid = *cIt->key;
        #endif
        if (!otd.isValid) continue;
        if (!otd.otdwall.IsEmpty ()) {
            GS::Array<OtdWall> opw; // Массив созданных откосов
            for (OtdWall& otdw : otd.otdwall) {
                if (!otdw.openings.IsEmpty ()) {
                    Point2D wbegC = { otdw.begC.x, otdw.begC.y };
                    Point2D wendC = { otdw.endC.x, otdw.endC.y };
                    Sector walledge = { wbegC , wendC };
                    GS::Optional<UnitVector_2D>	walldir = walledge.GetDirection ();
                    if (walldir.HasValue ()) {
                        double angz = -DEGRAD * 90;
                        double co = cos (angz);
                        double si = sin (angz);
                        Geometry::Vector2<double> walldir_perp = walldir.Get ().ToVector2D ().Rotate (si, co);
                        for (OtdOpening& op : otdw.openings) {
                            OpeningReveals_Create_One (otdw, op, walldir_perp, opw);
                        }
                    } else {
                        #if defined(TESTING)
                        DBprnt ("OtdWall_Create_One err", "!walldir.HasValue");
                        #endif
                    }

                }
            }
            if (!opw.IsEmpty ()) otd.otdwall.Append (opw);
        }
    }
    #if defined(TESTING)
    DBprnt ("OpeningReveals_Create_All", "end");
    #endif
}

// -----------------------------------------------------------------------------
// Создание стенок для откосов одного проёма
// -----------------------------------------------------------------------------
void OpeningReveals_Create_One (const OtdWall& otdw, OtdOpening& op, const Geometry::Vector2<double>& walldir_perp, GS::Array<OtdWall>& opw)
{
    if (op.base_reveal_width < min_dim) return;
    double zDup = fmin (op.zBottom + op.height, otdw.zBottom + otdw.height);
    double zDown = fmax (op.zBottom, otdw.zBottom);
    double height = fmin (op.height, zDup - zDown);
    if (height < min_dim) return;
    Point2D begedge;
    Point2D endedge;
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
    endedge.x = begedge.x + walldir_perp.x * op.base_reveal_width;
    endedge.y = begedge.y + walldir_perp.y * op.base_reveal_width;
    walledge = { begedge , endedge };
    if (!walledge.IsZeroLength ()) {
        OtdWall wallotd;
        wallotd.base_guid = op.base_guid;
        wallotd.height = height;
        wallotd.zBottom = zDown;
        wallotd.begC = { walledge.c1.x, walledge.c1.y };
        wallotd.endC = { walledge.c2.x, walledge.c2.y };
        wallotd.material = otdw.material;
        wallotd.base_type = API_WindowID;
        wallotd.type = Reveal_Main;
        op.has_reveal = true;
        opw.Push (wallotd);
    }
    lambda = (op.objLoc + op.width / 2) / dr;
    begedge.x = otdw.begC.x - dx * lambda;
    begedge.y = otdw.begC.y - dy * lambda;
    endedge.x = begedge.x + walldir_perp.x * op.base_reveal_width;
    endedge.y = begedge.y + walldir_perp.y * op.base_reveal_width;
    walledge = { begedge , endedge };
    if (!walledge.IsZeroLength ()) {
        OtdWall wallotd;
        wallotd.base_guid = op.base_guid;
        wallotd.height = height;
        wallotd.zBottom = zDown;
        wallotd.begC = { walledge.c2.x, walledge.c2.y };
        wallotd.endC = { walledge.c1.x, walledge.c1.y };
        wallotd.material = otdw.material;
        wallotd.base_type = API_WindowID;
        wallotd.type = Reveal_Main;
        op.has_reveal = true;
        opw.Push (wallotd);
    }
}

void SetMaterialByType (OtdRooms& roomsinfo)
{
    #if defined(TESTING)
    DBprnt ("SetMeterialByType", "start");
    #endif
    for (OtdRooms::PairIterator cIt = roomsinfo.EnumeratePairs (); cIt != NULL; ++cIt) {
        #if defined(AC_28)
        OtdRoom& otd = cIt->value;
        API_Guid zoneGuid = cIt->key;
        #else
        OtdRoom& otd = *cIt->value;
        API_Guid zoneGuid = *cIt->key;
        #endif
        if (!otd.isValid) continue;
        if (!otd.otdwall.IsEmpty ()) {
            for (OtdWall& otdw : otd.otdwall) {
                OtdMaterial material;
                // Проверим существование свойств для записи, при необходимости - поменяем тип отделки
                switch (otdw.type) {
                    case NoSet:
                        material.material = 0;
                        break;
                    case Wall_Main:
                        material = otd.om_main;
                        break;
                    case Wall_Up:
                        if (otd.om_up.material > 0) {
                            material = otd.om_up;
                        } else {
                            material = otd.om_main;
                        }
                        if (otd.om_up.rawname.IsEqual (otd.om_main.rawname)) otdw.type = Wall_Main;
                        break;
                    case Wall_Down:
                        if (otd.om_down.material > 0) {
                            material = otd.om_down;
                        } else {
                            material = otd.om_main;
                        }
                        if (otd.om_down.rawname.IsEqual (otd.om_main.rawname)) otdw.type = Wall_Main;
                        break;
                    case Reveal_Main:
                        if (otd.om_reveals.material > 0) {
                            material = otd.om_reveals;
                        } else {
                            material = otd.om_main;
                        }
                        if (material.material == 0) material = otd.om_main;
                        otdw.type = Reveal_Main;
                        if (otd.om_reveals.rawname.IsEqual (otd.om_main.rawname)) otdw.type = Wall_Main;
                        break;
                    case Reveal_Up:
                        if (otd.om_reveals.material > 0) {
                            material = otd.om_reveals;
                        } else {
                            material = otd.om_up;
                        }
                        if (material.material == 0) material = otd.om_main;
                        otdw.type = Reveal_Main;
                        if (otd.om_reveals.rawname.IsEqual (otd.om_main.rawname)) otdw.type = Wall_Main;
                        break;
                    case Reveal_Down:
                        if (otd.om_reveals.material > 0) {
                            material = otd.om_reveals;
                        } else {
                            material = otd.om_down;
                        }
                        if (material.material == 0) material = otd.om_main;
                        otdw.type = Reveal_Main;
                        if (otd.om_reveals.rawname.IsEqual (otd.om_main.rawname)) otdw.type = Wall_Main;
                        break;
                    case Column:
                        if (otd.om_column.material > 0) {
                            material = otd.om_column;
                        } else {
                            material = otd.om_main;
                        }
                        if (otd.om_column.rawname.IsEqual (otd.om_main.rawname)) otdw.type = Wall_Main;
                        break;
                    case Floor:
                        material = otd.om_floor;
                        break;
                    case Ceil:
                        material = otd.om_ceil;
                        break;
                    default:
                        material.material = 0;
                        break;
                }
                if (material.material == 0) material = otd.om_zone;
                otdw.material = material;
                ParamValueComposite p;
                #if defined(AC_27) || defined(AC_28)
                p.inx = ACAPI_CreateAttributeIndex (material.material);
                #else
                p.inx = material.material;
                #endif
                p.val = material.smaterial;
                p.num = -1;
                p.structype = APICWallComp_Finish;
                otdw.base_composite.Push (p);
            }
        }
    }
    #if defined(TESTING)
    DBprnt ("SetMeterialByType", "end");
    #endif
}

// -----------------------------------------------------------------------------
// Разбивка созданных стен по высотам на основании информации из зоны
// -----------------------------------------------------------------------------
void OtdWall_Delim_All (OtdRooms& roomsinfo)
{
    #if defined(TESTING)
    DBprnt ("OtdWall_Delim_All", "start");
    #endif
    for (OtdRooms::PairIterator cIt = roomsinfo.EnumeratePairs (); cIt != NULL; ++cIt) {
        #if defined(AC_28)
        OtdRoom& otd = cIt->value;
        API_Guid zoneGuid = cIt->key;
        #else
        OtdRoom& otd = *cIt->value;
        API_Guid zoneGuid = *cIt->key;
        #endif
        if (!otd.isValid) continue;
        if (!otd.otdwall.IsEmpty ()) {
            GS::Array<OtdWall> opw; // Массив созданных стен
            for (OtdWall& otdw : otd.otdwall) {
                TypeOtd type = otdw.type;
                bool has_delim = false; // Найдена разбивка
                double height = 0; // Высота элемента
                double zBottom = 0; // Отметка низа
                // Панели
                if (otd.height_down > 0) {
                    height = otd.height_down;
                    zBottom = otd.zBottom;
                    type = Wall_Down;
                    if (otdw.base_type == API_SlabID) type = Floor;
                    if (otdw.base_type == API_WindowID) type = Reveal_Down;
                    if (OtdWall_Delim_One (otdw, opw, height, zBottom, type)) has_delim = true;
                }
                // Основная часть
                if (otd.height_main > 0) {
                    type = Wall_Main;
                    if (otdw.base_type == API_ColumnID) type = Column;
                    if (otdw.base_type == API_WindowID) type = Reveal_Main;
                    height = otd.height_main;
                    zBottom = otd.zBottom + otd.height_down;
                    if (OtdWall_Delim_One (otdw, opw, height, zBottom, type)) has_delim = true;
                }
                // Пространство за потолком
                if (otd.height_up > 0) {
                    height = otd.height_up;
                    zBottom = otd.zBottom + otd.height_down + otd.height_main;
                    type = Wall_Up;
                    if (otdw.base_type == API_ColumnID) type = Column;
                    if (otdw.type == Ceil) type = Ceil;
                    if (otdw.base_type == API_WindowID) {
                        type = Reveal_Up;
                    }
                    if (OtdWall_Delim_One (otdw, opw, height, zBottom, type)) has_delim = true;
                }
                //Если высоты заданы не были - подгоним стенку под зону
                if (!has_delim) {
                    height = otd.height;
                    zBottom = otd.zBottom;
                    if (otdw.base_type == API_ColumnID) type = Column;
                    if (otdw.base_type == API_WindowID) type = Reveal_Main;
                    OtdWall_Delim_One (otdw, opw, height, zBottom, type);
                }
            }
            otd.otdwall = opw;
        }
    }
    #if defined(TESTING)
    DBprnt ("OtdWall_Delim_All", "end");
    #endif
}

// -----------------------------------------------------------------------------
// Добавляет стену с заданной высотой
// Удаляет отверстия, не попадающие в диапазон
// Подгоняет размер отверсий
// -----------------------------------------------------------------------------
bool OtdWall_Delim_One (OtdWall otdn, GS::Array<OtdWall>& opw, double height, double zBottom, TypeOtd& type)
{
    const double dh = 0.002; // Дополнительное увеличение высоты проёма для исключения линии на стыке
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
    double zDup = fmin (zBottom + height, otdn.zBottom + otdn.height);
    double zDown = fmax (zBottom, otdn.zBottom);
    height = fmin (otdn.height, zDup - zDown);
    if (height < min_dim || is_equal (height, 0)) {
        return false;
    }
    zBottom = zDown;
    // Удаляем лишние окна, подстраиваем высоту
    if (!otdn.openings.IsEmpty ()) {
        GS::Array<OtdOpening> newopenings;
        double dlower = otdn.zBottom - zBottom;
        for (OtdOpening& op : otdn.openings) {
            // Проём выше стенки
            if (op.zBottom >= zBottom + height) {
                continue;
            }
            // Проём ниже стенки, обнуляем
            if (op.zBottom + op.height <= zBottom) {
                continue;
            }
            zDup = fmin (zBottom + height, op.zBottom + op.height);
            zDown = fmax (zBottom, op.zBottom);
            op.lower = op.lower + dlower;
            if (op.lower < dh) {
                op.has_side = true;
                op.lower = -dh;
                op.height += dh;
            }
            if (op.height >= zDup - zDown || is_equal (op.height, zDup - zDown)) {
                op.height = fmin (op.height, zDup - zDown) + dh;
                if (op.has_side) op.height += dh;
                op.has_side = true;
            }
            if (op.height > min_dim && op.width > min_dim) newopenings.PushNew (op);
        }
        otdn.openings = newopenings;
    }
    otdn.zBottom = zBottom;
    otdn.height = height;
    otdn.type = type;
    opw.PushNew (otdn);
    return true;
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

void Draw_Elements (const Stories& storyLevels, OtdRooms& zoneelements, UnicElementByType& subelementByparent, ClassificationFunc::ClassificationDict& finclass, GS::Array<API_Guid>& deletelist)
{
    #if defined(TESTING)
    DBprnt ("Draw_Elements", "start");
    #endif
    API_Element wallelement = {};
    API_Element slabelement = {};
    API_Element wallobjelement = {}; API_ElementMemo wallobjmemo;
    API_Element slabobjelement = {}; API_ElementMemo slabobjmemo;
    API_Element windowelement = {}; API_ElementMemo windowmemo;
    GS::UniString UndoString = RSGetIndString (ID_ADDON_STRINGS + isEng (), RoombookId, ACAPI_GetOwnResModule ());
    ACAPI_CallUndoableCommand (UndoString, [&]() -> GSErrCode {
        if (!deletelist.IsEmpty ()) ACAPI_Element_Delete (deletelist);

        for (OtdRooms::PairIterator cIt = zoneelements.EnumeratePairs (); cIt != NULL; ++cIt) {
            #if defined(AC_28)
            OtdRoom& otd = cIt->value;
            #else
            OtdRoom& otd = *cIt->value;
            #endif
            if (!otd.isValid) continue;
            if (!otd.create_all_elements) continue;
            API_Guid class_guid = APINULLGuid;
            GS::Array<API_Guid> group;
            if (otd.create_wall_elements || otd.create_column_elements || otd.create_reveal_elements) {
                for (UInt32 i = 0; i < otd.otdwall.GetSize (); i++) {
                    if ((otd.otdwall[i].type == Wall_Main || otd.otdwall[i].type == Wall_Up || otd.otdwall[i].type == Wall_Down) && !otd.create_wall_elements) continue;
                    if ((otd.otdwall[i].type == Column) && !otd.create_column_elements) continue;
                    if ((otd.otdwall[i].type == Reveal_Main || otd.otdwall[i].type == Reveal_Down || otd.otdwall[i].type == Reveal_Up) && !otd.create_reveal_elements) continue;
                    OtdWall_Draw (storyLevels, otd.otdwall[i], wallelement, wallobjelement, wallobjmemo, windowelement, windowmemo, subelementByparent);
                    Param_AddUnicElementByType (otd.zone_guid, otd.otdwall[i].otd_guid, API_ZoneID, subelementByparent);// Привязка отделочных стен к зоне
                    for (OtdOpening& op : otd.otdwall[i].openings) {
                        Param_AddUnicElementByType (otd.zone_guid, op.otd_guid, API_ZoneID, subelementByparent);// Привязка отделочных проёмов к зоне
                    }
                    // Классификация созданных стен
                    Class_SetClass (otd.otdwall[i], finclass);
                    for (OtdOpening& op : otd.otdwall[i].openings) {
                        Class_SetClass (op, finclass);
                    }
                    group.Push (otd.otdwall[i].otd_guid);
                }
            }
            if (otd.create_ceil_elements || otd.create_floor_elements) {
                for (UInt32 i = 0; i < otd.otdslab.GetSize (); i++) {
                    if (otd.otdslab[i].type == Ceil && !otd.create_ceil_elements) continue;
                    if (otd.otdslab[i].type == Floor && !otd.create_floor_elements) continue;
                    Floor_Draw (storyLevels, slabelement, slabobjelement, slabobjmemo, otd.otdslab[i], subelementByparent);
                    Param_AddUnicElementByType (otd.zone_guid, otd.otdslab[i].otd_guid, API_ZoneID, subelementByparent); // Привязка перекрытий к зоне
                    Class_SetClass (otd.otdslab[i], finclass);
                    group.Push (otd.otdslab[i].otd_guid);
                }
            }
            if (group.GetSize () > 1) {
                API_Guid groupGuid = APINULLGuid;
                #if defined(AC_27) || defined(AC_28)
                ACAPI_Grouping_CreateGroup (group, &groupGuid);
                #else
                ACAPI_ElementGroup_Create (group, &groupGuid);
                #endif
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

void OtdWall_Draw (const Stories& storyLevels, OtdWall& edges, API_Element& wallelement, API_Element& wallobjelement, API_ElementMemo& wallobjmemo, API_Element& windowelement, API_ElementMemo& windowmemo, UnicElementByType& subelementByparent)
{
    if (edges.height < min_dim) return;
    double dx = -edges.endC.x + edges.begC.x;
    double dy = -edges.endC.y + edges.begC.y;
    double dr = sqrt (dx * dx + dy * dy);
    if (dr < min_dim) return;
    if (Favorite_GetType (edges.favorite_name) == API_ObjectID) {
        OtdWall_Draw_Object (storyLevels, edges, wallobjelement, wallobjmemo, subelementByparent);
    } else {
        OtdWall_Draw_Wall (storyLevels, edges, wallelement, windowelement, windowmemo, subelementByparent);
    }
    Param_AddUnicElementByType (edges.base_guid, edges.otd_guid, API_WallID, subelementByparent); // Привязка отделочной стены к базовой
}

void OtdWall_Draw_Object (const Stories& storyLevels, OtdWall& edges, API_Element& wallobjelement, API_ElementMemo& wallobjmemo, UnicElementByType& subelementByparent)
{
    GSErrCode err = NoError;
    if (!OtdWall_GetDefult_Object (edges.favorite_name, wallobjelement, wallobjmemo)) {
        return;
    }
    Point2D wbegC = { edges.begC.x, edges.begC.y };
    Point2D wendC = { edges.endC.x, edges.endC.y };
    Sector walledge = { wbegC , wendC };
    GS::Optional<UnitVector_2D>	walldir = walledge.GetDirection ();
    if (!walldir.HasValue ()) {
        #if defined(TESTING)
        DBprnt ("OtdWall_Create_One err", "!walldir.HasValue");
        #endif
        return;
    }
    double ac_wall_length = walledge.GetLength ();
    GDLHelpers::ParamDict accsessoryparams;
    GDLHelpers::Param p;
    p.num = 1; accsessoryparams.Add ("{@gdl:ac_refside}", p);
    p.num = edges.material.material; accsessoryparams.Add ("{@gdl:gs_bw_mat}", p);
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
    p.arr_num.Push (45 * DEGRAD);
    p.arr_num.Push (45 * DEGRAD);
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
    const auto floorIndexAndOffset = GetFloorIndexAndOffset (edges.zBottom, storyLevels);
    wallobjelement.header.floorInd = floorIndexAndOffset.first;
    wallobjelement.object.level = floorIndexAndOffset.second;
    err = ACAPI_Element_Create (&wallobjelement, &wallobjmemo);
    if (err != NoError) {
        return;
    }
    edges.otd_guid = wallobjelement.header.guid;
}

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

void OtdWall_Draw_Wall (const Stories& storyLevels, OtdWall& edges, API_Element& wallelement, API_Element& windowelement, API_ElementMemo& windowmemo, UnicElementByType& subelementByparent)
{
    GSErrCode err = NoError;
    if (!OtdWall_GetDefult_Wall (edges.favorite_name, wallelement)) {
        return;
    }
    wallelement.wall.begC = edges.begC;
    wallelement.wall.endC = edges.endC;
    wallelement.wall.height = edges.height;
    const auto floorIndexAndOffset = GetFloorIndexAndOffset (edges.zBottom, storyLevels);
    wallelement.header.floorInd = floorIndexAndOffset.first;
    wallelement.wall.bottomOffset = floorIndexAndOffset.second;
    #if defined(AC_27) || defined(AC_28)
    API_AttributeIndex ematerial = ACAPI_CreateAttributeIndex (edges.material.material);
    wallelement.wall.refMat.value = ematerial;
    wallelement.wall.oppMat.value = ematerial;
    wallelement.wall.sidMat.value = ematerial;
    #else
    wallelement.wall.refMat.attributeIndex = edges.material.material;
    wallelement.wall.oppMat.attributeIndex = edges.material.material;
    wallelement.wall.sidMat.attributeIndex = edges.material.material;
    #endif
    err = ACAPI_Element_Create (&wallelement, nullptr);
    if (err != NoError) {
        return;
    }
    edges.otd_guid = wallelement.header.guid;
    for (OtdOpening& op : edges.openings) {
        Opening_Draw (wallelement, windowelement, windowmemo, op, subelementByparent);
    }
}

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
    wallelement.wall.materialsChained = true;
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

void Opening_Draw (API_Element& wallelement, API_Element& windowelement, API_ElementMemo& windowmemo, OtdOpening& op, UnicElementByType& subelementByparent)
{
    GSErrCode err = NoError;
    if (wallelement.wall.type == APIWtyp_Poly) return;
    if (!Opening_GetDefult ("", windowelement, windowmemo)) {
        return;
    }
    windowelement.window.objLoc = op.objLoc;
    windowelement.window.owner = wallelement.header.guid;
    if (op.has_reveal) {
        op.width -= wallelement.wall.thickness * 2;
        op.height -= wallelement.wall.thickness;
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

void Floor_Draw (const Stories& storyLevels, API_Element& slabelement, API_Element& slabobjelement, API_ElementMemo& slabobjmemo, OtdSlab& otdslab, UnicElementByType& subelementByparent)
{
    if (Favorite_GetType (otdslab.favorite_name) == API_ObjectID) {
        Floor_Draw_Object (storyLevels, slabobjelement, slabobjmemo, otdslab, subelementByparent);
    } else {
        Floor_Draw_Slab (storyLevels, slabelement, otdslab, subelementByparent);
    }
    Param_AddUnicElementByType (otdslab.base_guid, otdslab.otd_guid, API_SlabID, subelementByparent); // Привязка отделочного перекрытия к базовому
}

void Floor_Draw_Slab (const Stories& storyLevels, API_Element& slabelement, OtdSlab& otdslab, UnicElementByType& subelementByparent)
{
    GSErrCode err = NoError;
    if (!Floor_GetDefult_Slab (otdslab.favorite_name, slabelement)) {
        return;
    }
    slabelement.slab.thickness = otd_thickness;
    const auto floorIndexAndOffset = GetFloorIndexAndOffset (otdslab.zBottom, storyLevels);
    slabelement.header.floorInd = floorIndexAndOffset.first;
    slabelement.slab.level = floorIndexAndOffset.second;
    #if defined(AC_27) || defined(AC_28)
    API_AttributeIndex ematerial = ACAPI_CreateAttributeIndex (otdslab.material.material);
    slabelement.slab.topMat.value = ematerial;
    slabelement.slab.sideMat.value = ematerial;
    slabelement.slab.botMat.value = ematerial;
    #else
    slabelement.slab.topMat.attributeIndex = otdslab.material.material;
    slabelement.slab.sideMat.attributeIndex = otdslab.material.material;
    slabelement.slab.botMat.attributeIndex = otdslab.material.material;
    #endif
    API_ElementMemo memo;
    BNZeroMemory (&memo, sizeof (API_ElementMemo));
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

void Floor_Draw_Object (const Stories& storyLevels, API_Element& slabobjelement, API_ElementMemo& slabobjmemo, OtdSlab& otdslab, UnicElementByType& subelementByparent)
{
    GSErrCode err = NoError;
    if (!OtdWall_GetDefult_Object (otdslab.favorite_name, slabobjelement, slabobjmemo)) {
        return;
    }
    GDLHelpers::ParamDict accsessoryparams;
    GDLHelpers::Param p;
    p.num = 0.02; accsessoryparams.Add ("{@gdl:ac_thickness}", p);

    p.num = otdslab.zBottom; accsessoryparams.Add ("{@gdl:ac_ref_height}", p);
    p.num = 0; accsessoryparams.Add ("{@gdl:ac_pitch}", p);
    p.num = 0; accsessoryparams.Add ("{@gdl:ac_ceilling_side}", p);
    p.num = 0; accsessoryparams.Add ("{@gdl:ac_ceiling_type}", p);
    p.num = otdslab.material.material; accsessoryparams.Add ("{@gdl:ceil_mat}", p);
    p.num = 0.02; accsessoryparams.Add ("{@gdl:ceil_thk}", p);
    p.num = 0; accsessoryparams.Add ("{@gdl:ac_slab_side}", p);


    //p.num = edges.material; accsessoryparams.Add ("{@gdl:gs_bw_mat}", p);
    //p.num = edges.height; accsessoryparams.Add ("{@gdl:ac_wall_height}", p);
    //p.num = ac_wall_length; accsessoryparams.Add ("{@gdl:ac_wall_length}", p);
    //p.num = 0; accsessoryparams.Add ("{@gdl:ac_wall_radius}", p);




    GDLHelpers::ParamToMemo (slabobjmemo, accsessoryparams);
    const auto floorIndexAndOffset = GetFloorIndexAndOffset (otdslab.zBottom, storyLevels);
    slabobjelement.header.floorInd = floorIndexAndOffset.first;
    slabobjelement.object.level = floorIndexAndOffset.second;
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
    slabelement.slab.modelElemStructureType = API_BasicStructure;
    return true;
}


void Class_SetClass (const OtdWall& op, const ClassificationFunc::ClassificationDict& finclass)
{
    if (op.otd_guid == APINULLGuid) return;
    API_Guid class_guid = Class_GetClassGuid (op.base_type, finclass);
    if (class_guid != APINULLGuid) ACAPI_Element_AddClassificationItem (op.otd_guid, class_guid);
}

void Class_SetClass (const OtdOpening& op, const ClassificationFunc::ClassificationDict& finclass)
{
    if (op.otd_guid == APINULLGuid) return;
    API_Guid class_guid = Class_GetClassGuid (API_WindowID, finclass);
    if (class_guid != APINULLGuid) ACAPI_Element_AddClassificationItem (op.otd_guid, class_guid);
}

void Class_SetClass (const OtdSlab& op, const ClassificationFunc::ClassificationDict& finclass)
{
    if (op.otd_guid == APINULLGuid) return;
    API_Guid class_guid = Class_GetClassGuid (op.base_type, finclass);
    if (class_guid != APINULLGuid) ACAPI_Element_AddClassificationItem (op.otd_guid, class_guid);
}

API_Guid Class_GetClassGuid (const API_ElemTypeID& base_type, const ClassificationFunc::ClassificationDict& finclass)
{
    if (base_type == API_ColumnID && finclass.ContainsKey (cls.column_class)) return finclass.Get (cls.column_class).item.guid;
    if (base_type == API_WallID && finclass.ContainsKey (cls.otdwall_class)) return finclass.Get (cls.otdwall_class).item.guid;
    if (base_type == API_WindowID && finclass.ContainsKey (cls.all_class)) return finclass.Get (cls.all_class).item.guid;
    if (base_type == API_SlabID && finclass.ContainsKey (cls.floor_class)) return finclass.Get (cls.floor_class).item.guid;
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
                    if (!finclassguids.ContainsKey (clas.item.guid)) finclassguids.Add (clas.item.guid, true);
                }
                if (desc.Contains (cls.ceil_class)) {
                    findict.Add (cls.ceil_class, clas);
                    if (!finclassguids.ContainsKey (clas.item.guid)) finclassguids.Add (clas.item.guid, true);
                }
                if (desc.Contains (cls.column_class)) {
                    findict.Add (cls.column_class, clas);
                    if (!finclassguids.ContainsKey (clas.item.guid)) finclassguids.Add (clas.item.guid, true);
                }
                if (desc.Contains (cls.floor_class)) {
                    findict.Add (cls.floor_class, clas);
                    if (!finclassguids.ContainsKey (clas.item.guid)) finclassguids.Add (clas.item.guid, true);
                }
                if (desc.Contains (cls.otdwall_class)) {
                    findict.Add (cls.otdwall_class, clas);
                    if (!finclassguids.ContainsKey (clas.item.guid)) finclassguids.Add (clas.item.guid, true);
                }
                if (desc.Contains (cls.reveal_class)) {
                    findict.Add (cls.reveal_class, clas);
                    if (!finclassguids.ContainsKey (clas.item.guid)) finclassguids.Add (clas.item.guid, true);
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
    typeinzone.Push (API_ZoneID);
    typeinzone.Push (API_WindowID);
    typeinzone.Push (API_WallID);
    typeinzone.Push (API_SlabID);
    GS::UniString suffix = "";
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
        ACAPI_CallUndoableCommand ("Write property",
                [&]() -> GSErrCode {
            ParamHelpers::ElementsWrite (paramToWrite);
            return NoError;
        });
    }
    if (!syncguidsdict.IsEmpty ()) {
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
#endif
}
