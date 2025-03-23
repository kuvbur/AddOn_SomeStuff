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
            if (elementType == API_ZoneID) zones.Push (neig.guid);
        }
    }
    if (zones.IsEmpty ()) {
        err = ACAPI_Element_GetElemList (API_ZoneID, &zones, APIFilt_IsEditable | APIFilt_OnVisLayer | APIFilt_HasAccessRight | APIFilt_InMyWorkspace | APIFilt_IsVisibleByRenovation);
        if (err != NoError || zones.IsEmpty ()) {
#if defined(TESTING)
            DBprnt ("RoomBook err", "zones.IsEmpty ()");
#endif
            return;
        }
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
    GS::Array<API_Guid> allslabs_;
    GS::Array<API_Guid> allslabs;
    err = ACAPI_Element_GetElemList (API_SlabID, &allslabs_, APIFilt_OnVisLayer | APIFilt_IsVisibleByRenovation);
    if (err != NoError) {
        return;
    }
    GS::HashTable<API_Guid, GS::Array<API_Guid>> slabsinzone;
    if (!allslabs_.IsEmpty ()) {
        for (auto guid : allslabs_) {
            if (!Class_IsElementFinClass (guid, finclassguids)) allslabs.Push (guid);
        }
    }
    if (!allslabs.IsEmpty ()) {
        GS::Array<GS::Pair<API_CollisionElem, API_CollisionElem>> collisions;
        API_CollisionDetectionSettings colDetSettings;
        colDetSettings.volumeTolerance = 0.0001;
        colDetSettings.performSurfaceCheck = true;
        colDetSettings.surfaceTolerance = 0.0001;
        if (ACAPI_Element_GetCollisions (zones, allslabs, collisions, colDetSettings) == NoError) {
            for (const auto& pair : collisions.AsConst ()) {
                if (Class_IsElementFinClass (pair.second.collidedElemGuid, finclassguids)) {
                    continue;
                }
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
                        ReadOneWinDoor (storyLevels, guid, openinginwall, guidselementToRead);
                        break;
                    case API_WallID:
                        ReadOneWall (storyLevels, guid, zoneGuids, roomsinfo, openinginwall, guidselementToRead);
                        break;
                    case API_ColumnID:
                        ReadOneColumn (storyLevels, guid, zoneGuids, roomsinfo, guidselementToRead);
                        break;
                    case API_SlabID:
                        ReadOneSlab (storyLevels, guid, zoneGuids, roomsinfo, guidselementToRead);
                        break;
                    default:
                        break;
                }
            }
        }
    }

    ParamDictElement paramToRead; // Прочитанные из элементов свойства
    ParamValue param_composite; // Состав базовых конструкций
    typeinzone.Push (API_ZoneID);
    for (const API_ElemTypeID& typeelem : typeinzone) {
        if (guidselementToRead.ContainsKey (typeelem)) {
            ParamDictValue paramDict;
            if (typeelem == API_ZoneID) {
                Param_GetForRooms (guidselementToRead[typeelem][0], propertyParams, paramDict);
            }
            if (typeelem == API_WallID || typeelem == API_ColumnID || typeelem == API_SlabID) {
                Param_GetForBase (guidselementToRead[typeelem][0], propertyParams, paramDict, param_composite);
            }
            if (typeelem == API_WindowID) {
                Param_GetForWindows (guidselementToRead[typeelem][0], propertyParams, paramDict);
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
    // Заполняем данные для зон
    Param_SetToRooms (roomsinfo, paramToRead);
    // Заполняем данные для отделочных стен (состав)
    Param_SetToBase (roomsinfo, paramToRead, guidselementToRead, param_composite);
    // Заполняем данные для окон
    Param_SetToWindows (roomsinfo, paramToRead);
    // Расчёт пола и потолка
    FloorRooms (storyLevels, roomsinfo, guidselementToRead, paramToRead);
    paramToRead.Clear ();
    // Создание стенок для откосов
    ReadAllOpeningReveals (roomsinfo);
    // Разделение стенок по высотам зон
    DelimOtdWalls (roomsinfo);

    UnicGuid parentGuid;
    GS::Array<API_Guid> deletelist;

    UnicElementByType subelementByparent; // Словарь с созданными родительскими и дочерними элементами
    // Отросовка элементов отделки
    Draw_Edges (storyLevels, roomsinfo, subelementByparent, finclass, deletelist);
    // Привязка отделочных элементов к базовым
    SetSyncOtdWall (subelementByparent, propertyParams);
    finish = clock ();
    duration = (double) (finish - start) / CLOCKS_PER_SEC;
    GS::UniString time = GS::UniString::Printf (" %.3f s", duration);
    msg_rep ("RoomBook", time, NoError, APINULLGuid);
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
#if defined(TESTING)
        DBprnt ("GetRoomEdges err", "ACAPI_Element_Get zone");
#endif
        return false;
    }
    API_ElementMemo zonememo;
    err = ACAPI_Element_GetMemo (zoneelement.header.guid, &zonememo);
    if (err != NoError) {
#if defined(TESTING)
        DBprnt ("GetRoomEdges err", "ACAPI_Element_GetMemo zone");
#endif
        return false;
    }
    GS::Array<Sector> walledges; // Границы стен, не явяющихся границей зоны
    GS::Array<Sector> columnedges; // Границы колонн, не явяющихся границей зоны
    GS::Array<Sector> restedges; // Границы зоны
    GS::Array<Sector> gableedges;
    GetZoneEdges (zonememo, zoneelement, walledges, columnedges, restedges, gableedges);
    roominfo.edges = restedges;
    roominfo.columnedges = columnedges;
    API_RoomRelation relData;
    err = ACAPI_Element_GetRelations (zoneGuid, API_ZombieElemID, &relData);
    if (err != NoError) {
        ACAPI_DisposeRoomRelationHdls (&relData);
#if defined(TESTING)
        DBprnt ("ParseRoom err", "ACAPI_Element_GetRelations");
#endif
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
    otdslab.material = zoneelement.zone.material.ToInt32_Deprecated ();
#else
    otdslab.material = zoneelement.zone.material;
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
#if defined(TESTING)
                DBprnt ("ReadOneWinDoor err", "ACAPI_Element_Get windor");
#endif
            }
        }
    }
    roominfo.wallPart = relData.wallPart;
    roominfo.beamPart = relData.beamPart;
    roominfo.cwSegmentPart = relData.cwSegmentPart;
    roominfo.niches = relData.niches;
    roominfo.zone_guid = zoneGuid;
#if defined(AC_27) || defined(AC_28)
    roominfo.material = zoneelement.zone.material.ToInt32_Deprecated ();
#else
    roominfo.material = zoneelement.zone.material;
#endif
    ACAPI_DisposeRoomRelationHdls (&relData);
    ACAPI_DisposeElemMemoHdls (&zonememo);
    return flag;
}

// -----------------------------------------------------------------------------
// Чтение данных о проёме
// -----------------------------------------------------------------------------
void ReadOneWinDoor (const Stories& storyLevels, const API_Guid& elGuid, GS::HashTable<API_Guid, GS::Array<OtdOpening>>& openinginwall, UnicGUIDByType& guidselementToRead)
{
    GSErrCode err;
    API_Element element;
    element.header.guid = elGuid;
    err = ACAPI_Element_Get (&element);
    if (err != NoError) {
#if defined(TESTING)
        DBprnt ("ReadOneWinDoor err", "ACAPI_Element_Get windor");
#endif
        return;
    }
    API_Guid wallguid = APINULLGuid;
    OtdOpening op;
#if defined(AC_27) || defined(AC_28) || defined(AC_26)
    switch (element.header.type.typeID) {
#else
    switch (element.header.typeID) {
#endif
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
void ReadOneWall (const Stories & storyLevels, API_Guid & elGuid, GS::Array<API_Guid>&zoneGuids, OtdRooms & roomsinfo, GS::HashTable<API_Guid, GS::Array<OtdOpening>>&openinginwall, UnicGUIDByType & guidselementToRead)
{
    GSErrCode err;
    API_Element element;
    element.header.guid = elGuid;
    err = ACAPI_Element_Get (&element);
    if (err != NoError) {
#if defined(TESTING)
        DBprnt ("ReadOneWall err", "ACAPI_Element_Get wall");
#endif
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
        DBprnt ("ReadOneWall err", "walledge.GetLength ()<min_dim");
#endif
        return;
    }
    GS::Optional<UnitVector_2D>	walldir = walledge.GetDirection ();
    if (!walldir.HasValue ()) {
#if defined(TESTING)
        DBprnt ("ReadOneWall err", "!walldir.HasValue");
#endif
        return;
    }
    for (API_Guid zoneGuid : zoneGuids) {
        if (!roomsinfo.ContainsKey (zoneGuid)) {
#if defined(TESTING)
            DBprnt ("ReadOneWall err", "!roomsinfo.ContainsKey (zoneGuid)");
#endif
            continue;
        }
        OtdRoom& roominfo = roomsinfo.Get (zoneGuid);
        bool flag_find = false;
        if (roominfo.walledges.ContainsKey (elGuid)) {
            GS::Array<Sector>& walledges = roominfo.walledges.Get (elGuid);
            for (Sector& roomedge : roominfo.walledges[elGuid]) {
                bool is_fliped = false;
                OtdWall wallotd;
                GS::Optional<Geometry::Line2D> roomeline = roomedge.AsLine ();
                if (!roomeline.HasValue ()) {
#if defined(TESTING)
                    DBprnt ("ReadOneWall err", "!roomeline.HasValue ()");
#endif
                    continue;
                }
                GS::Optional<UnitVector_2D>	roomedgedir = roomedge.GetDirection ();
                if (!roomedgedir.HasValue ()) {
#if defined(TESTING)
                    DBprnt ("ReadOneWall err", "!roomedgedir.HasValue");
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
                    wallotd.material = roominfo.material;
                    wallotd.base_type = API_WallID;
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
                    DBprnt ("ReadOneWall err", "inxedge > roomedges.restedges.GetSize ()");
#endif
                    continue;
                }
                Sector roomedge = roominfo.edges.Get (inxedge);
                GS::Optional<Geometry::Line2D> roomeline = roomedge.AsLine ();
                if (!roomeline.HasValue ()) {
#if defined(TESTING)
                    DBprnt ("ReadOneWall err", "!roomeline.HasValue ()");
#endif
                    continue;
                }
                GS::Optional<UnitVector_2D>	roomedgedir = roomedge.GetDirection ();
                if (!roomedgedir.HasValue ()) {
#if defined(TESTING)
                    DBprnt ("ReadOneWall err", "!roomedgedir.HasValue");
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
                    wallotd.material = roominfo.material;
                    wallotd.base_type = API_WallID;
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
void ReadOneColumn (const Stories & storyLevels, API_Guid & elGuid, GS::Array<API_Guid>&zoneGuids, OtdRooms & roomsinfo, UnicGUIDByType & guidselementToRead)
{
    GSErrCode err;
    API_Element element = {};
    API_ElementMemo segmentmemo = {};
    element.header.guid = elGuid;
    err = ACAPI_Element_Get (&element);
    if (err != NoError || !element.header.hasMemo) {
#if defined(TESTING)
        DBprnt ("ReadOneColumn err", "ACAPI_Element_Get column");
#endif
        return;
    }
    if (element.column.zoneRel == APIZRel_None) {
#if defined(TESTING)
        DBprnt ("ReadOneColumn err", "element.column.zoneRel == APIZRel_None");
#endif
        return;
    }
    err = ACAPI_Element_GetMemo (elGuid, &segmentmemo, APIMemoMask_ColumnSegment | APIMemoMask_AssemblySegmentScheme | APIMemoMask_AssemblySegmentProfile);
    if (err != NoError || segmentmemo.columnSegments == nullptr) {
#if defined(TESTING)
        DBprnt ("ReadOneColumn err", "ACAPI_Element_GetMemo column");
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
                        wallotd.material = roominfo.material;
                        wallotd.base_type = API_ColumnID;
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
void ReadOneSlab (const Stories & storyLevels, API_Guid & elGuid, GS::Array<API_Guid>&zoneGuids, OtdRooms & roomsinfo, UnicGUIDByType & guidselementToRead)
{
    GSErrCode err;
    API_Element element = {};
    API_ElementMemo memo = {};
    element.header.guid = elGuid;
    err = ACAPI_Element_Get (&element);
    if (err != NoError || !element.header.hasMemo) {
#if defined(TESTING)
        DBprnt ("ReadOneSlab err", "ACAPI_Element_Get slab");
#endif
        return;
    }
    bool flag_find = false;
    double zBottom = GetzPos (element.slab.level + element.slab.offsetFromTop - element.slab.thickness, element.header.floorInd, storyLevels);
    double zUp = zBottom + element.slab.thickness;
    for (API_Guid zoneGuid : zoneGuids) {
        if (roomsinfo.ContainsKey (zoneGuid)) {
            OtdRoom& roominfo = roomsinfo.Get (zoneGuid);
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
// Подготовка словаря с параметрами для чтения из стен/колонн/балок
// -----------------------------------------------------------------------------
void Param_GetForBase (const API_Guid & elGuid, ParamDictValue & propertyParams, ParamDictValue & paramDict, ParamValue & param_composite)
{
    Param_AddProperty (elGuid, propertyParams, paramDict);
    // Подготавливаем свойство для чтения из слоёв многослойки
    GS::UniString rawName = "{@material:layers,20}";
    param_composite.rawName = rawName;
    param_composite.val.uniStringValue = "l[{@property:параметры ведомости отделки/выводить в ведомость отделки}:{@property:параметры ведомости отделки/наименование в ведомости отделки}]";
    param_composite.fromMaterial = true;
    if (ParamHelpers::ParseParamNameMaterial (param_composite.val.uniStringValue, paramDict)) {
        for (UInt32 inx = 0; inx < 20; inx++) {
            ParamHelpers::AddValueToParamDictValue (paramDict, "@property:sync_name" + GS::UniString::Printf ("%d", inx));
        }
        ParamHelpers::CompareParamDictValue (propertyParams, paramDict);
    }
    ParamHelpers::AddParamValue2ParamDict (APINULLGuid, param_composite, paramDict);
}

// -----------------------------------------------------------------------------
// Подготовка словаря с параметрами для чтения из зон
// -----------------------------------------------------------------------------
void Param_GetForRooms (const API_Guid & elGuid, ParamDictValue & propertyParams, ParamDictValue & paramDict)
{
    Param_AddProperty (elGuid, propertyParams, paramDict);
    GDLParamRoom g;
    ParamHelpers::AddValueToParamDictValue (paramDict, g.material_main);
    ParamHelpers::AddValueToParamDictValue (paramDict, g.material_up);
    ParamHelpers::AddValueToParamDictValue (paramDict, g.material_down);
    ParamHelpers::AddValueToParamDictValue (paramDict, g.material_column);
    ParamHelpers::AddValueToParamDictValue (paramDict, g.height_down);
    ParamHelpers::AddValueToParamDictValue (paramDict, g.height_main);
    ParamHelpers::AddValueToParamDictValue (paramDict, g.material_pot);
    ParamHelpers::AddValueToParamDictValue (paramDict, g.has_ceil);
    ParamHelpers::AddValueToParamDictValue (paramDict, g.tip_pot);
    ParamHelpers::AddValueToParamDictValue (paramDict, g.tip_pol);
}

// -----------------------------------------------------------------------------
// Подготовка словаря с параметрами для чтения из окон
// -----------------------------------------------------------------------------
void Param_GetForWindows (const API_Guid & elGuid, ParamDictValue & propertyParams, ParamDictValue & paramDict)
{
    GDLParamWindow g;
    Param_AddProperty (elGuid, propertyParams, paramDict);
    ParamHelpers::AddValueToParamDictValue (paramDict, g.frame);
    ParamHelpers::AddValueToParamDictValue (paramDict, g.sill);
    ParamHelpers::AddValueToParamDictValue (paramDict, g.useWallFinishSkin);
    ParamHelpers::AddValueToParamDictValue (paramDict, g.maxPlasterThk);
    ParamHelpers::AddValueToParamDictValue (paramDict, g.AutoTurnIn);
    ParamHelpers::AddValueToParamDictValue (paramDict, g.bOverIn);
    ParamHelpers::AddValueToParamDictValue (paramDict, g.plaster_show_3D);
    ParamHelpers::AddValueToParamDictValue (paramDict, g.plaster_show_2D);
}

// -----------------------------------------------------------------------------
// Запись прочитанных свойств в зону
// -----------------------------------------------------------------------------
void Param_SetToRooms (OtdRooms & roomsinfo, ParamDictElement & paramToRead)
{
    GDLParamRoom g;
    for (OtdRooms::PairIterator cIt = roomsinfo.EnumeratePairs (); cIt != NULL; ++cIt) {
#if defined(AC_28)
        OtdRoom& otd = cIt->value;
        API_Guid zoneGuid = cIt->key;
#else
        OtdRoom& otd = *cIt->value;
        API_Guid zoneGuid = *cIt->key;
#endif
        // Чтение прочитанных свойств
        if (paramToRead.ContainsKey (zoneGuid)) {
            ParamDictValue& params = paramToRead.Get (zoneGuid);
            for (auto& cItt : params) {
#if defined(AC_28)
                ParamValue& param = cItt.value;
#else
                ParamValue& param = *cItt.value;
#endif
                if (!param.isValid) continue;
                if (param.rawName.IsEqual (g.material_column) || param.definition.description.Contains ("some_stuff_fin_column_material")) {
                    otd.material_column = param.val.intValue;
                    otd.smaterial_column = param.val.uniStringValue;
                }
                if (param.rawName.IsEqual (g.height_down) || param.definition.description.Contains ("some_stuff_fin_down_height")) {
                    otd.height_down = param.val.doubleValue;
                }
                if (param.rawName.IsEqual (g.material_down) || param.definition.description.Contains ("some_stuff_fin_down_material")) {
                    otd.material_down = param.val.intValue;
                    otd.smaterial_down = param.val.uniStringValue;
                }
                if (param.rawName.IsEqual (g.height_main) || param.definition.description.Contains ("some_stuff_fin_main_height")) {
                    otd.height_main = param.val.doubleValue;
                }
                if (param.rawName.IsEqual (g.material_main) || param.definition.description.Contains ("some_stuff_fin_main_material")) {
                    otd.material_main = param.val.intValue;
                    otd.smaterial_main = param.val.uniStringValue;
                }
                if (param.rawName.IsEqual (g.material_pot) || param.definition.description.Contains ("some_stuff_fin_ceil_material")) {
                    otd.material_pot = param.val.intValue;
                    otd.smaterial_pot = param.val.uniStringValue;
                }
                if (param.rawName.IsEqual (g.material_up) || param.definition.description.Contains ("some_stuff_fin_up_material")) {
                    otd.material_up = param.val.intValue;
                    otd.smaterial_up = param.val.uniStringValue;
                }
                if (param.rawName.IsEqual (g.has_ceil) || param.definition.description.Contains ("some_stuff_fin_has_ceil")) {
                    otd.has_ceil = param.val.boolValue;
                }
                if (param.rawName.IsEqual (g.tip_pot) || param.definition.description.Contains ("some_stuff_fin_ceil_type")) {
                    otd.tip_pot = param.val.uniStringValue;
                }
                if (param.rawName.IsEqual (g.tip_pol) || param.definition.description.Contains ("some_stuff_fin_floor_type")) {
                    otd.tip_pot = param.val.uniStringValue;
                }
                if (param.definition.description.Contains ("some_stuff_fin_up_result")) {
                    otd.rawname_up = param.rawName;
                }
                if (param.definition.description.Contains ("some_stuff_fin_main_result")) {
                    otd.rawname_main = param.rawName;
                }
                if (param.definition.description.Contains ("some_stuff_fin_down_result")) {
                    otd.rawname_down = param.rawName;
                }
                if (param.definition.description.Contains ("some_stuff_fin_column_result")) {
                    otd.rawname_column = param.rawName;
                }
            }
        } else {
#if defined(TESTING)
            DBprnt ("Param_SetToRooms err", "!paramToRead.ContainsKey (zoneGuid)");
#endif
        }
        // Заполнение непрочитанных
        // Высоты
        if (otd.height_main < 0.1) {
            otd.height_main = otd.height - otd.height_down;
            otd.height_up = 0;
        } else {
            otd.height_up = otd.height - otd.height_main;
            otd.height_main = otd.height_main - otd.height_down;
        }
    }
}

// -----------------------------------------------------------------------------
// Запись прочитанных свойств в отделочные стены
// -----------------------------------------------------------------------------
void Param_SetToBase (OtdRooms & roomsinfo, ParamDictElement & paramToRead, UnicGUIDByType & guidselementToRead, ParamValue & param_composite)
{
    if (!guidselementToRead.ContainsKey (API_ZoneID)) {
#if defined(TESTING)
        DBprnt ("Param_SetToBase err", "!guidselementToRead.ContainsKey (API_ZoneID)");
#endif
        return;
    }
    for (const API_Guid& guid : guidselementToRead[API_ZoneID]) {
        if (!roomsinfo.ContainsKey (guid)) {
#if defined(TESTING)
            DBprnt ("Param_SetToBase err", "!roomsinfo.ContainsKey (guid)");
#endif
            continue;
        }
        GS::Array<OtdWall>& otd = roomsinfo.Get (guid).otdwall; // Массив отделочных поверхностей в зоне
        for (UInt32 i = 0; i < otd.GetSize (); i++) {
            OtdWall& otdw = otd[i]; // Стена-отделка
            if (!paramToRead.ContainsKey (otdw.base_guid)) {
#if defined(TESTING)
                DBprnt ("Param_SetToBase err", "!paramToRead.ContainsKey (otdw.base_guid)");
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
                        otdcpmpoosite.Push (base_composite.composite[j]);
                        if (base_composite.composite[j].structype == APICWallComp_Core) flag_core = true;
                    }
                }
            } else {
                for (Int32 j = 0; j < ncomp; j++) {
                    if (!flag_core) {
                        otdcpmpoosite.Push (base_composite.composite[j]);
                        if (base_composite.composite[j].structype == APICWallComp_Core) flag_core = true;
                    }
                }
            }
            otdw.base_composite = otdcpmpoosite;
        }
    }
}

// -----------------------------------------------------------------------------
// Задание прочитанных параметров для окон
// -----------------------------------------------------------------------------
void Param_SetToWindows (OtdRooms & roomsinfo, ParamDictElement & paramToRead)
{
#if defined(TESTING)
    DBprnt ("Param_SetToWindows", "start");
#endif
    GDLParamWindow g;
    double sill = 0;
    double frame = 0;
    double max_plaster_th = 0;
    for (OtdRooms::PairIterator cIt = roomsinfo.EnumeratePairs (); cIt != NULL; ++cIt) {
#if defined(AC_28)
        OtdRoom& otd = cIt->value;
        API_Guid zoneGuid = cIt->key;
#else
        OtdRoom& otd = *cIt->value;
        API_Guid zoneGuid = *cIt->key;
#endif
        if (!otd.otdwall.IsEmpty ()) {
            for (OtdWall& otdw : otd.otdwall) {
                if (!otdw.openings.IsEmpty ()) {
                    for (OtdOpening& op : otdw.openings) {
                        if (!paramToRead.ContainsKey (op.base_guid)) break;
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
void RoomReductionPolyProc (const API_RoomReductionPolyType * roomRed)
#else
static void	__ACENV_CALL RoomRedProc (const API_RoomReductionPolyType * roomRed)
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
void GetZoneEdges (const API_ElementMemo & zonememo, API_Element & zoneelement, GS::Array<Sector>&walledges, GS::Array<Sector>&columnedges, GS::Array<Sector>&restedges, GS::Array<Sector>&gableedges)
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
        DBprnt ("GetZoneEdges err", "APIDb_RoomReductionsID");
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
void FloorRooms (const Stories & storyLevels, OtdRooms & roomsinfo, UnicGUIDByType & guidselementToRead, ParamDictElement & paramToRead)
{
    if (!guidselementToRead.ContainsKey (API_ZoneID)) {
#if defined(TESTING)
        DBprnt ("FloorRooms err", "!guidselementToRead.ContainsKey (API_ZoneID)");
#endif
        return;
    }
    for (const API_Guid& guid : guidselementToRead[API_ZoneID]) {
        if (!roomsinfo.ContainsKey (guid)) {
#if defined(TESTING)
            DBprnt ("FloorRooms err", "!roomsinfo.ContainsKey (guid)");
#endif
            continue;
        }
        OtdRoom& roominfo = roomsinfo.Get (guid);
        if (!roominfo.has_floor || !roominfo.has_ceil) continue;
        if (roominfo.has_floor) {
            roominfo.poly.zBottom = roominfo.zBottom;
            if (!roominfo.floorslab.IsEmpty ()) {
                FloorOneRoom (storyLevels, roominfo.poly, roominfo.floorslab, roominfo.otdslab, roominfo.otdwall, paramToRead, true);
            } else {
                roominfo.otdslab.Push (roominfo.poly);
            }
        }
        if (roominfo.has_ceil) {
            roominfo.poly.zBottom = roominfo.zBottom + roominfo.height_main + roominfo.height_down;
            roominfo.poly.material = roominfo.material_pot;
            if (!roominfo.ceilslab.IsEmpty ()) {
                FloorOneRoom (storyLevels, roominfo.poly, roominfo.ceilslab, roominfo.otdslab, roominfo.otdwall, paramToRead, false);
            } else {
                roominfo.otdslab.Push (roominfo.poly);
            }
        }
    }
}

void FloorOneRoom (const Stories & storyLevels, OtdSlab & poly, GS::Array<API_Guid>&slabGuids, GS::Array<OtdSlab>&otdslabs, GS::Array<OtdWall>&otdwall, ParamDictElement & paramToRead, bool on_top)
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
            DBprnt ("FloorOneRoom err", "ACAPI_Element_Get slab");
#endif
            continue;
        }
        err = ACAPI_Element_GetMemo (slabGuid, &memo, APIMemoMask_Polygon);
        if (err != NoError || memo.coords == nullptr) {
#if defined(TESTING)
            DBprnt ("FloorOneRoom err", "ACAPI_Element_GetMemo slab");
#endif
            continue;
        }
        Geometry::Polygon2D slabpolygon;
        err = ConstructPolygon2DFromElementMemo (memo, slabpolygon);
        if (err != NoError) {
            msg_rep ("FloorOneRoom err", "ConstructPolygon2DFromElementMemo", err, slabGuid);
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
                        msg_rep ("FloorOneRoom - error", "polygon2DData.contourEnds == nullptr", NoError, slabGuid);
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
            if (on_top) otdslab.zBottom = zUp;
            otdslabs.Push (otdslab);
        }
        ACAPI_DisposeElemMemoHdls (&memo);
    }
    OtdSlab otdslab = poly;
    if (on_top) {
        otdslab.zBottom += otd_thickness;
    } else {
        otdslab.zBottom -= otd_thickness;
    }
    otdslab.poly = roompolygon;
    otdslabs.Push (otdslab);
}

// -----------------------------------------------------------------------------
// Убираем задвоение Guid зон у элементов
// -----------------------------------------------------------------------------
void ClearZoneGUID (UnicElementByType & elementToRead, GS::Array<API_ElemTypeID>&typeinzone)
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
double GetAreaOtdWall (const OtdWall & otdw)
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
void ReadAllOpeningReveals (OtdRooms & roomsinfo)
{
#if defined(TESTING)
    DBprnt ("ReadAllOpeningReveals", "start");
#endif
    for (OtdRooms::PairIterator cIt = roomsinfo.EnumeratePairs (); cIt != NULL; ++cIt) {
#if defined(AC_28)
        OtdRoom& otd = cIt->value;
        API_Guid zoneGuid = cIt->key;
#else
        OtdRoom& otd = *cIt->value;
        API_Guid zoneGuid = *cIt->key;
#endif
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
                            ReadOneOpeningReveals (otdw, op, walldir_perp, opw);
                        }
                    } else {
#if defined(TESTING)
                        DBprnt ("ReadOneWall err", "!walldir.HasValue");
#endif
                    }

                }
            }
            if (!opw.IsEmpty ()) otd.otdwall.Append (opw);
        }
    }
#if defined(TESTING)
    DBprnt ("ReadAllOpeningReveals", "end");
#endif
}

// -----------------------------------------------------------------------------
// Создание стенок для откосов одного проёма
// -----------------------------------------------------------------------------
void ReadOneOpeningReveals (const OtdWall & otdw, OtdOpening & op, const Geometry::Vector2<double>&walldir_perp, GS::Array<OtdWall>&opw)
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
        op.has_reveal = true;
        opw.Push (wallotd);
    }
}

// -----------------------------------------------------------------------------
// Разбивка созданных стен по высотам на основании информации из зоны
// -----------------------------------------------------------------------------
void DelimOtdWalls (OtdRooms & roomsinfo)
{
#if defined(TESTING)
    DBprnt ("DelimOtdWalls", "start");
#endif
    for (OtdRooms::PairIterator cIt = roomsinfo.EnumeratePairs (); cIt != NULL; ++cIt) {
#if defined(AC_28)
        OtdRoom& otd = cIt->value;
        API_Guid zoneGuid = cIt->key;
#else
        OtdRoom& otd = *cIt->value;
        API_Guid zoneGuid = *cIt->key;
#endif
        if (!otd.otdwall.IsEmpty ()) {
            GS::Array<OtdWall> opw; // Массив созданных стен
            for (OtdWall& otdw : otd.otdwall) {
                bool has_delim = false; // Найдена разбивка
                double height = 0; // Высота элемента
                double zBottom = 0; // Отметка низа
                short material = 0;
                GS::UniString smaterial = "";
                // Панели
                if (otd.height_down > 0) {
                    height = otd.height_down;
                    zBottom = otd.zBottom;
                    material = otd.material_down;
                    smaterial = otd.smaterial_down;
                    if (otdw.base_type == API_SlabID) {
                        material = otd.material_main;
                        smaterial = otd.smaterial_main;
                    }
                    if (otdw.base_type == API_WindowID) {
                        if (otd.material_reveal > 0) {
                            material = otd.material_reveal;
                        } else {
                            material = otd.material_down;
                            smaterial = otd.smaterial_down;
                        }
                    }
                    if (material == 0) material = otd.material;
                    if (DelimOneWall (otdw, opw, height, zBottom, material, smaterial)) has_delim = true;
                }
                // Основная часть
                if (otd.height_main > 0) {
                    if (otdw.base_type == API_ColumnID) {
                        material = otd.material_column;
                        smaterial = otd.smaterial_column;
                    }
                    if (otdw.base_type == API_WallID) {
                        material = otd.material_main;
                        smaterial = otd.smaterial_main;
                    }
                    if (otdw.base_type == API_SlabID) {
                        material = otd.material_main;
                        smaterial = otd.smaterial_main;
                    }
                    if (otdw.base_type == API_WindowID) {
                        if (otd.material_reveal > 0) {
                            material = otd.material_reveal;
                        } else {
                            material = otd.material_main;
                            smaterial = otd.smaterial_main;
                        }
                    }
                    if (material == 0) material = otd.material;
                    height = otd.height_main;
                    zBottom = otd.zBottom + otd.height_down;
                    if (DelimOneWall (otdw, opw, height, zBottom, material, smaterial)) has_delim = true;
                }
                // Пространство за потолком
                if (otd.height_up > 0) {
                    height = otd.height_up;
                    zBottom = otd.zBottom + otd.height_down + otd.height_main;
                    material = otd.material_up;
                    smaterial = otd.smaterial_up;
                    if (otdw.base_type == API_SlabID) {
                        material = otd.material_main;
                        smaterial = otd.smaterial_main;
                    }
                    if (otdw.base_type == API_WindowID) {
                        if (otd.material_reveal > 0) {
                            material = otd.material_reveal;
                        } else {
                            material = otd.material_up;
                            smaterial = otd.smaterial_up;
                        }
                    }
                    if (material == 0) material = otd.material;
                    if (DelimOneWall (otdw, opw, height, zBottom, material, smaterial)) has_delim = true;
                }
                //Если высоты заданы не были - подгоним стенку под зону
                if (!has_delim) {
                    height = otd.height;
                    zBottom = otd.zBottom;
                    material = otd.material_main;
                    smaterial = otd.smaterial_main;
                    if (otdw.base_type == API_WindowID) material = otd.material_reveal;
                    if (material == 0) material = otd.material;
                    DelimOneWall (otdw, opw, height, zBottom, material, smaterial);
                }
            }
            otd.otdwall = opw;
        }
    }
#if defined(TESTING)
    DBprnt ("DelimOtdWalls", "end");
#endif
}

// -----------------------------------------------------------------------------
// Добавляет стену с заданной высотой
// Удаляет отверстия, не попадающие в диапазон
// Подгоняет размер отверсий
// -----------------------------------------------------------------------------
bool DelimOneWall (OtdWall otdn, GS::Array<OtdWall>&opw, double height, double zBottom, short& material, GS::UniString & smaterial)
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
    otdn.material = material;
    ParamValueComposite p;
#if defined(AC_27) || defined(AC_28)
    p.inx = ACAPI_CreateAttributeIndex (material);
#else
    p.inx = material;
#endif
    p.val = smaterial;
    p.num = -1;
    p.structype = APICWallComp_Finish;
    otdn.base_composite.Push (p);
    opw.PushNew (otdn);
    return true;
}

bool Edge_FindOnEdge (Sector & edge, GS::Array<Sector>&edges, Sector & findedge)
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
bool Edge_FindEdge (Sector & edge, GS::Array<Sector>&edges)
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

void Draw_Edges (const Stories & storyLevels, OtdRooms & zoneelements, UnicElementByType & subelementByparent, ClassificationFunc::ClassificationDict & finclass, GS::Array<API_Guid>&deletelist)
{
#if defined(TESTING)
    DBprnt ("Draw_Edges", "start");
#endif
    API_Element wallelement;
#if defined AC_26 || defined AC_27 || defined AC_28
    wallelement.header.type.typeID = API_WallID;
#else
    wallelement.header.typeID = API_WallID;
#endif
    if (ACAPI_Element_GetDefaults (&wallelement, nullptr) != NoError) {
        return;
    }
    wallelement.wall.zoneRel = APIZRel_None;
    wallelement.wall.referenceLineLocation = APIWallRefLine_Inside;
    wallelement.wall.flipped = false;
    wallelement.wall.materialsChained = true;
#if defined AC_26 || defined AC_27 || defined AC_28
#else
    wallelement.header.layer = 1;
    wallelement.wall.refMat.overridden = true;
    wallelement.wall.oppMat.overridden = true;
    wallelement.wall.sidMat.overridden = true;
#endif
    wallelement.wall.modelElemStructureType = API_BasicStructure;
    wallelement.wall.thickness = otd_thickness;
    API_Element slabelement;
    BNZeroMemory (&slabelement, sizeof (API_Element));
#if defined AC_26 || defined AC_27 || defined AC_28
    slabelement.header.type.typeID = API_SlabID;
#else
    slabelement.header.typeID = API_SlabID;
#endif
    if (ACAPI_Element_GetDefaults (&slabelement, nullptr) != NoError) {
        return;
    }
    slabelement.header.layer = wallelement.header.layer;
    slabelement.slab.offsetFromTop = 0;
    slabelement.slab.materialsChained = true;
#if defined AC_26 || defined AC_27 || defined AC_28
#else
    slabelement.slab.sideMat.overridden = true;
    slabelement.slab.topMat.overridden = true;
    slabelement.slab.botMat.overridden = true;
#endif
    slabelement.slab.modelElemStructureType = API_BasicStructure;
    GS::UniString UndoString = RSGetIndString (ID_ADDON_STRINGS + isEng (), RoombookId, ACAPI_GetOwnResModule ());
    ACAPI_CallUndoableCommand (UndoString, [&]() -> GSErrCode {
        if (!deletelist.IsEmpty ()) ACAPI_Element_Delete (deletelist);
        for (OtdRooms::PairIterator cIt = zoneelements.EnumeratePairs (); cIt != NULL; ++cIt) {
#if defined(AC_28)
            OtdRoom& otd = cIt->value;
#else
            OtdRoom& otd = *cIt->value;
#endif
            API_Guid class_guid = APINULLGuid;
            GS::Array<API_Guid> group;
            for (UInt32 i = 0; i < otd.otdwall.GetSize (); i++) {
                Draw_Edge (storyLevels, otd.otdwall[i], wallelement, subelementByparent);
                Param_AddUnicElementByType (otd.zone_guid, otd.otdwall[i].otd_guid, API_ZoneID, subelementByparent);// Привязка отделочных стен к зоне
                for (OtdOpening& op : otd.otdwall[i].openings) {
                    Param_AddUnicElementByType (otd.zone_guid, op.otd_guid, API_ZoneID, subelementByparent);// Привязка отделочных проёмов к зоне
                }
                // Классификация созданных стен
                class_guid = APINULLGuid;
                if (otd.otdwall[i].base_type == API_ColumnID && finclass.ContainsKey (cls.column_class)) class_guid = finclass.Get (cls.column_class).item.guid;
                if (otd.otdwall[i].base_type == API_WallID && finclass.ContainsKey (cls.otdwall_class)) class_guid = finclass.Get (cls.otdwall_class).item.guid;
                if (class_guid == APINULLGuid && finclass.ContainsKey (cls.all_class)) class_guid = finclass.Get (cls.all_class).item.guid;
                if (class_guid != APINULLGuid) ACAPI_Element_AddClassificationItem (otd.otdwall[i].otd_guid, class_guid);
                // Классификация проёмов
                class_guid = APINULLGuid;
                if (finclass.ContainsKey (cls.all_class)) class_guid = finclass.Get (cls.all_class).item.guid;
                if (class_guid != APINULLGuid) {
                    for (OtdOpening& op : otd.otdwall[i].openings) {
                        ACAPI_Element_AddClassificationItem (op.otd_guid, class_guid);
                    }
                }
                group.Push (otd.otdwall[i].otd_guid);
            }
            for (UInt32 i = 0; i < otd.otdslab.GetSize (); i++) {
                Draw_Slab (storyLevels, slabelement, otd.otdslab[i], subelementByparent);
                Param_AddUnicElementByType (otd.zone_guid, otd.otdslab[i].otd_guid, API_ZoneID, subelementByparent); // Привязка перекрытий к зоне
                class_guid = APINULLGuid;
                if (finclass.ContainsKey (cls.floor_class)) class_guid = finclass.Get (cls.floor_class).item.guid;
                if (class_guid == APINULLGuid && finclass.ContainsKey (cls.all_class)) class_guid = finclass.Get (cls.all_class).item.guid;
                if (class_guid != APINULLGuid) ACAPI_Element_AddClassificationItem (otd.otdslab[i].otd_guid, class_guid);
                group.Push (otd.otdslab[i].otd_guid);
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
#if defined(TESTING)
    DBprnt ("Draw_Edges", "end");
#endif
}

void Draw_Edge (const Stories & storyLevels, OtdWall & edges, API_Element & wallelement, UnicElementByType & subelementByparent)
{
    if (edges.height < min_dim) {
        return;
    }
    double dx = -edges.endC.x + edges.begC.x;
    double dy = -edges.endC.y + edges.begC.y;
    double dr = sqrt (dx * dx + dy * dy);
    if (dr < min_dim) {
        return;
    }
    wallelement.wall.begC = edges.begC;
    wallelement.wall.endC = edges.endC;
    wallelement.wall.height = edges.height;
    const auto floorIndexAndOffset = GetFloorIndexAndOffset (edges.zBottom, storyLevels);
    wallelement.header.floorInd = floorIndexAndOffset.first;
    wallelement.wall.bottomOffset = floorIndexAndOffset.second;
#if defined(AC_27) || defined(AC_28)
    API_AttributeIndex ematerial = ACAPI_CreateAttributeIndex (edges.material);
    wallelement.wall.refMat.value = ematerial;
    wallelement.wall.oppMat.value = ematerial;
    wallelement.wall.sidMat.value = ematerial;
#else
    wallelement.wall.refMat.attributeIndex = edges.material;
    wallelement.wall.oppMat.attributeIndex = edges.material;
    wallelement.wall.sidMat.attributeIndex = edges.material;
#endif
    if (ACAPI_Element_Create (&wallelement, nullptr) == NoError) {
        edges.otd_guid = wallelement.header.guid;
        Param_AddUnicElementByType (edges.base_guid, edges.otd_guid, API_WallID, subelementByparent); // Привязка отделочной стены к базовой
        for (OtdOpening& op : edges.openings) {
            Draw_Window (wallelement, op, subelementByparent);
        }
    }
}

void Draw_Window (API_Element & wallelement, OtdOpening & op, UnicElementByType & subelementByparent)
{
    API_ElementMemo memo;
    GSErrCode err = NoError;
    API_Element windowelement;
    if (wallelement.wall.type == APIWtyp_Poly) return;
    BNClear (memo);
#if defined AC_26 || defined AC_27 || defined AC_28
    windowelement.header.type.typeID = API_WindowID;
#else
    windowelement.header.typeID = API_WindowID;
#endif
    err = ACAPI_Element_GetDefaults (&windowelement, &memo);
    if (err != NoError) {
        ACAPI_DisposeElemMemoHdls (&memo);
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
    if (ACAPI_Element_Create (&windowelement, &memo) == NoError) {
        op.otd_guid = windowelement.header.guid;
        Param_AddUnicElementByType (op.base_guid, op.otd_guid, API_WindowID, subelementByparent); // Привязка отделочного проёма к базовому
    }
    ACAPI_DisposeElemMemoHdls (&memo);
    return;
}		// Draw_Window

// -----------------------------------------------------------------------------
// Create a special shaped slab with custom edge trims
// -----------------------------------------------------------------------------
void Draw_Slab (const Stories & storyLevels, API_Element & slabelement, OtdSlab & otdslab, UnicElementByType & subelementByparent)
{
    slabelement.slab.thickness = otd_thickness;
    const auto floorIndexAndOffset = GetFloorIndexAndOffset (otdslab.zBottom, storyLevels);
    slabelement.header.floorInd = floorIndexAndOffset.first;
    slabelement.slab.level = floorIndexAndOffset.second;
#if defined(AC_27) || defined(AC_28)
    API_AttributeIndex ematerial = ACAPI_CreateAttributeIndex (otdslab.material);
    slabelement.slab.topMat.value = ematerial;
    slabelement.slab.sideMat.value = ematerial;
    slabelement.slab.botMat.value = ematerial;
#else
    slabelement.slab.topMat.attributeIndex = otdslab.material;
    slabelement.slab.sideMat.attributeIndex = otdslab.material;
    slabelement.slab.botMat.attributeIndex = otdslab.material;
#endif
    API_ElementMemo memo;
    BNZeroMemory (&memo, sizeof (API_ElementMemo));
    if (ConvertPolygon2DToAPIPolygon (otdslab.poly, slabelement.slab.poly, memo) == NoError) {
        if (ACAPI_Element_Create (&slabelement, &memo) == NoError) {
            otdslab.otd_guid = slabelement.header.guid;
            Param_AddUnicElementByType (otdslab.base_guid, otdslab.otd_guid, API_SlabID, subelementByparent); // Привязка отделочного перекрытия к базовому
        }
    }
    ACAPI_DisposeElemMemoHdls (&memo);
    return;
}		// Draw_Slab

// -----------------------------------------------------------------------------
// Поиск классов для отделочных стен (some_stuff_fin_ в описании класса)
// -----------------------------------------------------------------------------
void Class_FindFinClass (ClassificationFunc::SystemDict & systemdict, ClassificationFunc::ClassificationDict & findict, UnicGuid & finclassguids)
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
void SetSyncOtdWall (UnicElementByType & subelementByparent, ParamDictValue & propertyParams)
{
    SyncSettings syncSettings (false, false, true, true, true, true, false);
    LoadSyncSettingsFromPreferences (syncSettings);
    ParamDictElement paramToWrite;
    API_Elem_Head parentelementhead;
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
        ACAPI_CallUndoableCommand ("SetSubelement",
                [&]() -> GSErrCode {
            ParamHelpers::ElementsWrite (paramToWrite);
            return NoError;
        });
        ClassificationFunc::SystemDict systemdict;
        ClassificationFunc::GetAllClassification (systemdict);
        GS::Array<API_Guid> syncguids;

        GS::Array<API_Guid> rereadelem = SyncArray (syncSettings, syncguids, systemdict, propertyParams);
        if (!rereadelem.IsEmpty ()) {
#if defined(TESTING)
            DBprnt ("===== REREAD =======");
#endif
            SyncArray (syncSettings, rereadelem, systemdict, propertyParams);
        }
    }
}

bool Class_IsElementFinClass (const API_Guid & elGuid, const UnicGuid & finclassguids)
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

void Param_AddUnicElementByType (const API_Guid & parentguid, const API_Guid & guid, API_ElemTypeID elemtype, UnicElementByType & elementToRead)
{
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

void Param_AddUnicGUIDByType (const API_Guid & elGuid, API_ElemTypeID elemtype, UnicGUIDByType & guidselementToRead)
{
    if (guidselementToRead.ContainsKey (elemtype)) {
        guidselementToRead.Get (elemtype).Push (elGuid);
    } else {
        GS::Array<API_Guid> z;
        z.Push (elGuid);
        guidselementToRead.Add (elemtype, z);
    }
}

void Param_AddProperty (const API_Guid & elGuid, ParamDictValue & propertyParams, ParamDictValue & paramDict)
{
    GS::UniString propdesc = "some_stuff_fin_";
    for (auto& cItt : propertyParams) {
#if defined(AC_28)
        ParamValue param = cItt.value;
#else
        ParamValue param = *cItt.value;
#endif
        if (param.definition.description.Contains (propdesc)) {
            if (!paramDict.ContainsKey (param.rawName)) {
                if (ACAPI_Element_IsPropertyDefinitionAvailable (elGuid, param.definition.guid)) {
                    if (!paramDict.ContainsKey (param.rawName)) {
                        paramDict.Add (param.rawName, param);
                    } else {
                        break;
                    }
                }
            }
        }
    }
}
#endif
}
