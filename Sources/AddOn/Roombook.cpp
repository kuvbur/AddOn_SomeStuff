//------------ kuvbur 2022 ------------
#ifdef PK_1
#include	"ACAPinc.h"
#include	"APIEnvir.h"
#include	"Roombook.hpp"
#include    "VectorImageIterator.hpp"

namespace AutoFunc
{

OtdRoom* reducededges = nullptr;

void RoomBook ()
{
    GS::Array<API_Guid> zones;
    GSErrCode            err;
    API_SelectionInfo    selectionInfo;
    GS::Array<API_Neig>  selNeigs;
    err = ACAPI_Selection_Get (&selectionInfo, &selNeigs, true);
    BMKillHandle ((GSHandle*) &selectionInfo.marquee.coords);
    if (err != APIERR_NOSEL && selectionInfo.typeID != API_SelEmpty) {
        for (const API_Neig& neig : selNeigs) {
            API_NeigID neigID = neig.neigID;
            API_ElemTypeID elementType;
            err = ACAPI_Goodies (APIAny_NeigIDToElemTypeID, &neigID, &elementType);
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

    //--- Чтение данных о зоне, создание словаря с элементами для чтения
    GS::HashTable < API_Guid, OtdRoom> roomsinfo;
    UnicElementByType elementToRead;
    for (API_Guid zoneGuid : zones) {
        OtdRoom roominfo;
        if (CollectRoomInfo (zoneGuid, roominfo, elementToRead)) {
            roomsinfo.Add (zoneGuid, roominfo);
        }
    }

    GS::HashTable<API_Guid, GS::Array<OtdOpening>> openinginwall;
    GS::Array<API_ElemTypeID> typeinzone;
    typeinzone.Push (API_WindowID);
    typeinzone.Push (API_DoorID);
    typeinzone.Push (API_ColumnID);
    typeinzone.Push (API_WallID);
    API_Guid wallguid;
    OtdOpening op;
    for (const API_ElemTypeID& typeelem : typeinzone) {
        if (elementToRead.ContainsKey (typeelem)) {
            for (GS::HashTable<API_Guid, GS::Array<API_Guid>>::PairIterator cIt = elementToRead.Get (typeelem).EnumeratePairs (); cIt != NULL; ++cIt) {
#if defined(AC_28)
                API_Guid guid = cIt->key;
                GS::Array<API_Guid> zoneGuids = cIt->value;
#else
                API_Guid guid = *cIt->key;
                GS::Array<API_Guid> zoneGuids = *cIt->value;
#endif
                switch (typeelem) {
                    case API_WindowID:
                    case API_DoorID:
                        if (ReadOneWinDoor (guid, wallguid, op)) {
                            if (openinginwall.ContainsKey (wallguid)) {
                                openinginwall.Get (wallguid).Push (op);
                            } else {
                                GS::Array<OtdOpening> openings;
                                openings.Push (op);
                                openinginwall.Add (wallguid, openings);
                            }
                        }
                        break;
                    case API_WallID:
                        ReadOneWall (guid, zoneGuids, roomsinfo, openinginwall);
                        break;
                    case API_ColumnID:
                        ReadOneColumn (guid, zoneGuids, roomsinfo);
                        break;
                    default:
                        break;
                }
            }
        }
    }


    // Подготовка параметров
    ParamDictValue propertyParams;
    ParamDictValue paramDict;
    ParamValue param_composite;
    ParamHelpers::AllPropertyDefinitionToParamDict (propertyParams);
    GetparamToRead (propertyParams, paramDict, param_composite);

    // Получение информации из зоны
    ParamDictElement paramToRead;

    //for (API_Guid zoneGuid : zones) {
    //    GS::Array<OtdWall> otd;
    //    ParseRoom (zoneGuid, otd, paramToRead, param_composite, paramDict);
    //    zoneelements.Add (zoneGuid, otd);
    //}
    // Чтение параметров базовых стен и зон
//    ClassificationFunc::SystemDict systemdict;
//    ParamHelpers::ElementsRead (paramToRead, propertyParams, systemdict);
//    for (API_Guid zoneGuid : zones) {
//        if (!zoneelements.ContainsKey (zoneGuid)) {
//#if defined(TESTING)
//            DBprnt ("RoomBook err", "!zoneelements.ContainsKey (zoneGuid)");
//#endif
//            continue;
//        }
//        // Массив отделочных поверхностей в зоне
//        GS::Array<OtdWall>& zoneelement = zoneelements.Get (zoneGuid);
//        for (UInt32 i = 0; i < zoneelement.GetSize (); i++) {
//            // Отделочная поверхность
//            OtdWall& otd = zoneelement[i];
//
//            if (!paramToRead.ContainsKey (otd.base_guid)) {
//#if defined(TESTING)
//                DBprnt ("RoomBook err", "!paramToRead.ContainsKey (otd.base_guid)");
//#endif
//                continue;
//            }
//            // Прочитанные параметры базового компонента
//            ParamDictValue& baseparam = paramToRead.Get (otd.base_guid);
//            if (!baseparam.ContainsKey (param_composite.rawName)) {
//#if defined(TESTING)
//                DBprnt ("RoomBook err", "!baseparam.ContainsKey (param_composite.rawName)");
//#endif
//                continue;
//            }
//            // Состав базового компонента
//            ParamValue base_composite = baseparam.Get (param_composite.rawName);
//            // Отфильтрованные слои отделки
//            GS::Array<ParamValueComposite> otdcpmpoosite;
//            //TODO Дописать определение вывода слоя в ведомость отделки исходя из свойств
//            Int32 ncomp = base_composite.composite.GetSize ();
//            if (otd.base_flipped) {
//                for (Int32 j = ncomp - 1; j >= 0; j = j--) {
//                    if (base_composite.composite[j].structype != APICWallComp_Core) {
//                        otdcpmpoosite.Push (base_composite.composite[j]);
//                    } else {
//                        break;
//                    }
//                }
//            } else {
//                for (Int32 j = 0; j < ncomp; j = j++) {
//                    if (base_composite.composite[j].structype != APICWallComp_Core) {
//                        otdcpmpoosite.Push (base_composite.composite[j]);
//                    } else {
//                        break;
//                    }
//                }
//            }
//            otd.base_composite = otdcpmpoosite;
//        }
//    }
    DrawEdges (roomsinfo);
}

bool CollectRoomInfo (API_Guid& zoneGuid, OtdRoom& roominfo, UnicElementByType& elementToRead)
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
    GetZoneEdges (zoneGuid, roominfo);
    if (roominfo.isEmpty) {
#if defined(TESTING)
        DBprnt ("GetRoomEdges err", "roomedges.isEmpty");
#endif
        return false;
    }

    API_RoomRelation relData;
    err = ACAPI_Element_GetRelations (zoneGuid, API_ZombieElemID, &relData);
    if (err != NoError) {
        ACAPI_DisposeRoomRelationHdls (&relData);
#if defined(TESTING)
        DBprnt ("ParseRoom err", "ACAPI_Element_GetRelations");
#endif
        return false;
    }
    GS::Array<API_ElemTypeID> typeinzone;
    typeinzone.Push (API_WindowID);
    typeinzone.Push (API_DoorID);
    typeinzone.Push (API_WallID);
    typeinzone.Push (API_ColumnID);
    bool flag = false;
    for (const API_ElemTypeID& typeelem : typeinzone) {
        if (relData.elementsGroupedByType.ContainsKey (typeelem)) {
            GS::Array<API_Guid> guids = relData.elementsGroupedByType.Get (typeelem);
            if (!elementToRead.ContainsKey (typeelem)) {
                UnicElement el;
                elementToRead.Add (typeelem, el);
            }
            if (elementToRead.ContainsKey (typeelem)) {
                for (const API_Guid& elGuid : guids) {
                    if (!elementToRead.Get (typeelem).ContainsKey (elGuid)) {
                        GS::Array<API_Guid> el;
                        el.Push (zoneGuid);
                        elementToRead.Get (typeelem).Add (elGuid, el);
                    } else {
                        if (typeelem == API_WallID || typeelem == API_ColumnID) elementToRead.Get (typeelem).Get (elGuid).Push (zoneGuid);
                    }
                }
            }
            flag = true;
            //switch (typeelem) {
            //    case API_WindowID:
            //        roominfo.windows.Append (guids);
            //        break;
            //    case API_DoorID:
            //        roominfo.doors.Append (guids);
            //        break;
            //    case API_WallID:
            //        roominfo.walls.Append (guids);
            //        break;
            //    case API_ColumnID:
            //        roominfo.columns.Append (guids);
            //        break;
            //    default:
            //        break;
            //}
        }
    }
    roominfo.wallPart = relData.wallPart;
    roominfo.beamPart = relData.beamPart;
    roominfo.cwSegmentPart = relData.cwSegmentPart;
    roominfo.niches = relData.niches;
    ACAPI_DisposeRoomRelationHdls (&relData);
    return flag;
}

void ReadOneWall (API_Guid& elGuid, GS::Array<API_Guid>& zoneGuids, GS::HashTable < API_Guid, OtdRoom>& roomsinfo, GS::HashTable<API_Guid, GS::Array<OtdOpening>>& openinginwall)
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
    // Пропуск элементов на слое Архикад
    if (element.header.layer == 1) {
#if defined(TESTING)
        DBprnt ("ReadOneWall err", "element.header.layer == 1");
#endif
        return;
    }
    if (element.wall.zoneRel == APIZRel_None) {
#if defined(TESTING)
        DBprnt ("ReadOneWall err", "element.wall.zoneRel == APIZRel_None");
#endif
        return;
    }
    Point2D wbegC = { element.wall.begC.x, element.wall.begC.y };
    Point2D wendC = { element.wall.endC.x, element.wall.endC.y };
    Sector walledge = { wbegC , wendC };
    double dx = -element.wall.endC.x + element.wall.begC.x;
    double dy = -element.wall.endC.y + element.wall.begC.y;
    double dr = sqrt (dx * dx + dy * dy);
    if (dr < 0.0001) {
#if defined(TESTING)
        DBprnt ("ReadOneWall err", "walledge.GetLength ()<0.0001");
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
        if (roomsinfo.ContainsKey (zoneGuid)) {
            OtdRoom& roominfo = roomsinfo.Get (zoneGuid);
            for (API_WallPart& wpart : roominfo.wallPart) {
                if (wpart.guid != elGuid) continue;
                UInt32 inxedge = wpart.roomEdge - 1;
                if (inxedge > roominfo.restedges.GetSize ()) {
#if defined(TESTING)
                    DBprnt ("ReadOneWall err", "inxedge > roomedges.restedges.GetSize ()");
#endif
                    continue;
                }
                Sector roomedge = roominfo.restedges[inxedge];
                GS::Optional<Geometry::Line2D> roomeline = roomedge.AsLine ();
                if (!roomeline.HasValue ()) {
#if defined(TESTING)
                    DBprnt ("ReadOneWall err", "!roomeline.HasValue ()");
#endif
                    continue;
                }
                GS::Optional<UnitVector_2D>	roomedgedir = roominfo.restedges[inxedge].GetDirection ();
                if (!roomedgedir.HasValue ()) {
#if defined(TESTING)
                    DBprnt ("ReadOneWall err", "!roomedgedir.HasValue");
#endif
                    continue;
                }
                bool is_fliped = false;
                Point2D begedge = wbegC;
                Point2D endedge = wendC;
                OtdWall wallotd;
                if (walldir.Get ().IsParallelWith (roomedgedir.Get ())) {
                    if (!is_equal (wpart.tEnd, 0) || !is_equal (wpart.tBeg, 0)) {
                        if (!is_equal (wpart.tBeg, 0) && !is_equal (wpart.tBeg, dr)) {
                            double lambda = wpart.tBeg / dr;
                            begedge.x = element.wall.begC.x - dx * lambda;
                            begedge.y = element.wall.begC.y - dy * lambda;
                        }
                        if (!is_equal (wpart.tEnd, 0) && !is_equal (wpart.tEnd, dr)) {
                            double lambda = wpart.tEnd / dr;
                            endedge.x = element.wall.begC.x - dx * lambda;
                            endedge.y = element.wall.begC.y - dy * lambda;
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
                            if (op.objLoc <= wpart.tEnd - op.width / 2 && op.objLoc >= wpart.tBeg + op.width / 2) {
                                OtdOpening opinwall;
                                opinwall.base_guid = op.base_guid;
                                if (!is_fliped) {
                                    opinwall.objLoc = op.objLoc - wpart.tBeg;
                                } else {
                                    //TODO Проём в отделке ставится некорректно
                                    opinwall.objLoc = -dr - op.objLoc + wpart.tBeg;
                                }
                                opinwall.height = op.height;
                                opinwall.width = op.width;
                                opinwall.lower = op.lower;
                                wallotd.openings.Push (opinwall);
                            }
                        }
                    }
                    if (element.wall.flipped) is_fliped = !is_fliped;
                } else {
                    walledge = roominfo.restedges[inxedge]; // Торец стены
                }
                if (!walledge.IsZeroLength ()) {
                    wallotd.base_guid = elGuid;
                    wallotd.bottomOffset = element.wall.bottomOffset;
                    wallotd.height = element.wall.height;
                    wallotd.base_flipped = is_fliped;
                    wallotd.begC = { walledge.c1.x, walledge.c1.y };
                    wallotd.endC = { walledge.c2.x, walledge.c2.y };
                    roominfo.otd.Push (wallotd);
                }
            }
        }
    }
}

void ReadOneColumn (API_Guid& elGuid, GS::Array<API_Guid>& zoneGuids, GS::HashTable < API_Guid, OtdRoom>& roomsinfo)
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
        for (Sector& coledge : columnedges) {
            for (API_Guid zoneGuid : zoneGuids) {
                if (roomsinfo.ContainsKey (zoneGuid)) {
                    OtdRoom& roominfo = roomsinfo.Get (zoneGuid);
                    Sector cdge;
                    if (FindOnEdge (coledge, roominfo.restedges, cdge)) {
                        OtdWall wallotd;
                        wallotd.base_guid = elGuid;
                        wallotd.bottomOffset = element.column.bottomOffset;
                        wallotd.height = element.column.height;
                        wallotd.begC = { cdge.c1.x, cdge.c1.y };
                        wallotd.endC = { cdge.c2.x, cdge.c2.y };
                        roominfo.otd.Push (wallotd);
                    }
                }
            }
        }
    }
    ACAPI_DisposeElemMemoHdls (&segmentmemo);
}

void DrawEdges (GS::HashTable < API_Guid, OtdRoom>& zoneelements)
{
    API_Element wallelement;
    wallelement.header.typeID = API_WallID;
    if (ACAPI_Element_GetDefaults (&wallelement, nullptr) != NoError) {
        return;
    }
    wallelement.header.layer = 1;
    GS::Array<API_Guid> walllist;
    GS::Array<API_Guid> walllist_;
    ACAPI_Element_GetElemList (API_WallID, &walllist);
    API_Elem_Head elem_head = {};
    for (UInt32 i = 0; i < walllist.GetSize (); i++) {
        elem_head.guid = walllist[i];
        if (ACAPI_Element_GetHeader (&elem_head) == NoError) {
            if (elem_head.layer == wallelement.header.layer) walllist_.Push (walllist[i]);
        }
    }
    ACAPI_CallUndoableCommand ("Create Element", [&]() -> GSErrCode {
        if (!walllist_.IsEmpty ()) ACAPI_Element_Delete (walllist_);
        for (GS::HashTable < API_Guid, OtdRoom>::PairIterator cIt = zoneelements.EnumeratePairs (); cIt != NULL; ++cIt) {
#if defined(AC_28)
            GS::Array<OtdWall >& otd = cIt->value;
#else
            OtdRoom& otd = *cIt->value;
#endif
            for (UInt32 i = 0; i < otd.otd.GetSize (); i++) {
                DrawEdge (otd.otd[i], wallelement);
            }
        }
        return NoError;
    });
}

void DrawEdge (OtdWall& edges, API_Element& wallelement)
{
    wallelement.wall.begC = edges.begC;
    wallelement.wall.endC = edges.endC;
    wallelement.wall.height = edges.height;
    wallelement.wall.bottomOffset = edges.bottomOffset;
    if (ACAPI_Element_Create (&wallelement, nullptr) != NoError) {
        return;
    }
    edges.otd_guid = wallelement.header.guid;
    for (OtdOpening& op : edges.openings) {
        Do_CreateWindow (wallelement, op);
    }
}

// -----------------------------------------------------------------------------
// Чтение данных об одном проёме
// -----------------------------------------------------------------------------
bool ReadOneWinDoor (const API_Guid& elGuid, API_Guid& wallguid, OtdOpening& op)
{
    API_Element element = {};
    element.header.guid = elGuid;
    if (ACAPI_Element_Get (&element) != NoError) return false;
    wallguid = APINULLGuid;
    op.base_guid = elGuid;
    switch (element.header.typeID) {
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
    return (wallguid != APINULLGuid);
}

void Do_CreateWindow (API_Element& wallelement, OtdOpening& op)
{
    API_ElementMemo		memo;
    GSErrCode			err = NoError;
    API_Element windowelement;
    if (wallelement.wall.type == APIWtyp_Poly) return;
    BNClear (memo);
    windowelement.header.typeID = API_WindowID;
    err = ACAPI_Element_GetDefaults (&windowelement, &memo);
    if (err != NoError) {
        ACAPI_DisposeElemMemoHdls (&memo);
        return;
    }
    windowelement.window.objLoc = op.objLoc;
    windowelement.window.owner = wallelement.header.guid;
    windowelement.window.openingBase.width = op.width;
    windowelement.window.openingBase.height = op.height;
    windowelement.window.lower = op.lower;
    if (ACAPI_Element_Create (&windowelement, &memo) == NoError) {
        op.otd_guid = windowelement.header.guid;
    }
    ACAPI_DisposeElemMemoHdls (&memo);
    return;
}		// Do_CreateWindow


// -----------------------------------------------------------------------------
// Подготовка словаря с параметрами для чтения из элементов
// -----------------------------------------------------------------------------
void GetparamToRead (ParamDictValue& propertyParams, ParamDictValue& paramDict, ParamValue& param_composite)
{
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
}

// -----------------------------------------------------------------------------
// Полчение полигона зоны (в том числе стен, колонн)
// -----------------------------------------------------------------------------
static void	__ACENV_CALL	RoomRedProc (const API_RoomReductionPolyType* roomRed)
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
        if (roomRed->type == APIRoomReduction_Rest) {
            // При подсчёте полигонов архикад выдаёт сдвижку на 1 для каждого субполигона
            begC = { 0,0 }; endC = { 0, 0 };
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
void GetZoneEdges (API_Guid& zoneGuid, OtdRoom& roomedges)
{
    reducededges = &roomedges;
    if (ACAPI_Database (APIDb_RoomReductionsID, &zoneGuid, (void*) (GS::IntPtr) RoomRedProc) != NoError) {
#if defined(TESTING)
        DBprnt ("GetZoneEdges err", "APIDb_RoomReductionsID");
#endif
        return;
    }
    // Уберём из rest участки, которые есть в других частях

    GS::Array<Sector> edges;
    if (!roomedges.columnedges.IsEmpty ()) {
        for (UInt32 j = 0; j < roomedges.columnedges.GetSize (); j++) {
            Sector tedge = roomedges.columnedges[j];
            if (FindEdge (tedge, roomedges.restedges)) edges.Push (tedge);
        }
        roomedges.columnedges = edges;
        edges.Clear ();
    }
    if (!roomedges.walledges.IsEmpty ()) {
        for (UInt32 j = 0; j < roomedges.walledges.GetSize (); j++) {
            Sector tedge = roomedges.walledges[j];
            if (FindEdge (tedge, roomedges.restedges)) edges.Push (tedge);
        }
        roomedges.walledges = edges;
        edges.Clear ();
    }
    if (!roomedges.gableedges.IsEmpty ()) {
        for (UInt32 j = 0; j < roomedges.gableedges.GetSize (); j++) {
            Sector tedge = roomedges.gableedges[j];
            if (FindEdge (tedge, roomedges.restedges)) edges.Push (tedge);
        }
        roomedges.gableedges = edges;
        edges.Clear ();
    }
    roomedges.isEmpty = (roomedges.restedges.IsEmpty () && roomedges.columnedges.IsEmpty () && roomedges.walledges.IsEmpty () && roomedges.gableedges.IsEmpty ());
}

bool FindOnEdge (Sector& edge, GS::Array<Sector>& edges, Sector& findedge)
{
    const double toler = 0.001;
    for (UInt32 i = 0; i < edges.GetSize (); i++) {
        if (edge.ContainsPoint (edges[i].c1) || edge.ContainsPoint (edges[i].c2) || edges[i].ContainsPoint (edge.c1) || edges[i].ContainsPoint (edge.c2)) {
            GS::Optional<UnitVector_2D>	edgedir = edge.GetDirection ();
            if (edgedir.HasValue ()) {
                GS::Optional<UnitVector_2D>	roomedgedir = edges[i].GetDirection ();
                if (roomedgedir.HasValue ()) {
                    if (edgedir.Get ().IsParallelWith (roomedgedir.Get ())) {
                        findedge = edges[i];
                        return true;
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
bool FindEdge (Sector& edge, GS::Array<Sector>& edges)
{
    const double toler = 0.001;
    for (UInt32 i = 0; i < edges.GetSize (); i++) {
        if (edges[i].c1.IsNear (edge.c1, toler) && edges[i].c2.IsNear (edge.c2, toler)) {
            return true;
        } else {
            if (edges[i].c1.IsNear (edge.c2, toler) && edges[i].c2.IsNear (edge.c1, toler)) {
                edge = edge.InvertDirection ();
                return true;
            }
        }
    }
    return false;
}
}
#endif
