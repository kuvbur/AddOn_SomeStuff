//------------ kuvbur 2022 ------------
#ifdef PK_1
#include	"ACAPinc.h"
#include	"APIEnvir.h"
#include	"AutomateFunction.hpp"
#include	"Helpers.hpp"
#include	"Model3D/MeshBody.hpp"
#include	"Model3D/model.h"

namespace AutoFunc
{

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
        if (err != NoError || zones.IsEmpty ()) return;
    }
    for (API_Guid zoneGuid : zones) {
        ParseRoom (zoneGuid);
    }
}

void ParseRoom (API_Guid& zoneGuid)
{
    GSErrCode err;
    API_RoomRelation relData;
    err = ACAPI_Element_GetRelations (zoneGuid, API_ZombieElemID, &relData);
    if (err != NoError) {
        ACAPI_DisposeRoomRelationHdls (&relData);
        return;
    }
    API_Element element = {};
    for (UInt32 i = 0; i < relData.wallPart.GetSize (); i++) {
        BNZeroMemory (&element, sizeof (API_Element));
        element.header.guid = relData.wallPart[i].guid;
        err = ACAPI_Element_Get (&element);
        if (err != NoError) continue;
        if (!is_equal (relData.wallPart[i].tEnd, 0) || !is_equal (relData.wallPart[i].tBeg, 0)) {
            API_Coord begC_ = element.wall.begC;
            API_Coord endC_ = element.wall.endC;
            double dr = sqrt ((element.wall.endC.x - element.wall.begC.x) * (element.wall.endC.x - element.wall.begC.x) + (element.wall.endC.y - element.wall.begC.y) * (element.wall.endC.y - element.wall.begC.y));
            if (!is_equal (relData.wallPart[i].tBeg, 0) && !is_equal (relData.wallPart[i].tBeg, dr)) {
                double lambda = dr / (relData.wallPart[i].tBeg - dr);
                begC_.x = (element.wall.endC.x + element.wall.begC.x * lambda) / (1 + lambda);
                begC_.y = (element.wall.endC.y + element.wall.begC.y * lambda) / (1 + lambda);
            }
            if (!is_equal (relData.wallPart[i].tEnd, 0)) {
                double lambda = dr / (relData.wallPart[i].tEnd - dr);
                endC_.x = (element.wall.begC.x + element.wall.endC.x * lambda) / (1 + lambda);
                endC_.y = (element.wall.begC.y + element.wall.begC.y * lambda) / (1 + lambda);
            }
        }
        //double dr = sqrt ((endC_.x - begC_.x) * (endC_.x - begC_.x) + (endC_.y - begC_.y) * (endC_.y - begC_.y));
        //
        //    if from_dot = 1 then
        //        if abs (1 + lambda) > EPS then
        //            dx = (x2 + x1 * lambda) / (1 + lambda)
        //            dy = (y2 + y1 * lambda) / (1 + lambda)

        //            dx = (x1 + x2 * lambda) / (1 + lambda)
        //            dy = (y1 + y2 * lambda) / (1 + lambda)
    }
    ACAPI_DisposeRoomRelationHdls (&relData);
}


// -----------------------------------------------------------------------------
// Ищет в массиве отрезок, начало или конец которого находятся возле точки start
// Возвращает индекс inx элемента в массиве, если точка была концом отрезка - поднимает флаг isend
// -----------------------------------------------------------------------------
bool GetNear (const GS::Array<Sector>& k, const Point2D& start, UInt32& inx, bool& isend)
{
    for (UInt32 i = 0; i < k.GetSize (); i++) {
        if (start.IsNear (k[i].c1)) {
            inx = i;
            isend = false;
            return true;
        }
        if (start.IsNear (k[i].c2)) {
            inx = i;
            isend = true;
            return true;
        }
    }
    return false;
}
// -----------------------------------------------------------------------------
// Устанавливает подрезку по отрезку, возвращает  API_3DCutPlanesInfo cutInfo
// -----------------------------------------------------------------------------
GSErrCode GetCuplane (const SSectLine sline, API_3DCutPlanesInfo& cutInfo)
{
    BNZeroMemory (&cutInfo, sizeof (API_3DCutPlanesInfo));
    GSErrCode err = NoError;
#if defined(AC_27) || defined(AC_28)
    err = ACAPI_View_Get3DCuttingPlanes (&cutInfo);
#else
    err = ACAPI_Environment (APIEnv_Get3DCuttingPlanesID, &cutInfo, nullptr);
#endif
    if (err != NoError) {
        msg_rep ("GetCuplane", "APIEnv_Get3DCuttingPlanesID", err, APINULLGuid);
        return err;
    }
    if (cutInfo.shapes != nullptr) BMKillHandle ((GSHandle*) &(cutInfo.shapes));
    cutInfo.isCutPlanes = true;
    cutInfo.useCustom = false;
    cutInfo.nShapes = 4;
    cutInfo.shapes = reinterpret_cast<API_3DCutShapeType**> (BMAllocateHandle (cutInfo.nShapes * sizeof (API_3DCutShapeType), ALLOCATE_CLEAR, 0));
    double pa = 0; double pb = 0; double pc = 0; double pd = 0;
    if (cutInfo.shapes != nullptr) {
        double co = cos (sline.angz);
        double si = sin (sline.angz);
        double sx = sline.s.c1.x * co + sline.s.c1.y * si;
        double sy = -sline.s.c1.x * si + sline.s.c1.y * co;
        double ex = sline.s.c2.x * co + sline.s.c2.y * si;
        double ey = -sline.s.c2.x * si + sline.s.c2.y * co;
        double x = 0;
        double dx = fabs (sx) - fabs (ex);
        double dy = fabs (sy) - fabs (ey);
        bool isx = false;
        if (fabs (dx) < 0.00001 && fabs (sx) > std::numeric_limits<double>::epsilon ()) {
            isx = true;
            x = sx;
        }
        if (fabs (dy) < 0.00001 && fabs (sy) > std::numeric_limits<double>::epsilon ()) {
            x = sy;
        }
        Int32 inx = 0;
        (*cutInfo.shapes)[inx].cutStatus = 0;
        (*cutInfo.shapes)[inx].cutPen = 3;
        //(*cutInfo.shapes)[inx].cutMater = 11;
        (*cutInfo.shapes)[inx].pa = sin (-sline.angz);
        (*cutInfo.shapes)[inx].pb = cos (-sline.angz);
        (*cutInfo.shapes)[inx].pc = 0;
        (*cutInfo.shapes)[inx].pd = x;

        inx += 1;
        co = cos (sline.angz + 180 * DEGRAD);
        si = sin (sline.angz + 180 * DEGRAD);
        sx = sline.s.c1.x * co + sline.s.c1.y * si;
        sy = -sline.s.c1.x * si + sline.s.c1.y * co;
        ex = sline.s.c2.x * co + sline.s.c2.y * si;
        ey = -sline.s.c2.x * si + sline.s.c2.y * co;
        double x2 = 0;
        dx = fabs (sx) - fabs (ex);
        dy = fabs (sy) - fabs (ey);
        if (fabs (dx) < 0.00001 && fabs (sx) > std::numeric_limits<double>::epsilon ()) {
            x2 = sx;
        }
        if (fabs (dy) < 0.00001 && fabs (sy) > std::numeric_limits<double>::epsilon ()) {
            x2 = sy;
        }
        (*cutInfo.shapes)[inx].cutStatus = 0;
        (*cutInfo.shapes)[inx].cutPen = 3;
        //(*cutInfo.shapes)[inx].cutMater = 11;
        (*cutInfo.shapes)[inx].pa = sin (-sline.angz + 180 * DEGRAD);
        (*cutInfo.shapes)[inx].pb = cos (-sline.angz + 180 * DEGRAD);
        (*cutInfo.shapes)[inx].pc = 0;
        (*cutInfo.shapes)[inx].pd = x2 + 1;

        co = cos (sline.angz_1);
        si = sin (sline.angz_1);
        if (isx) {
            x = sline.s.c1.x * co + sline.s.c1.y * si;
        } else {
            x = -sline.s.c1.x * si + sline.s.c1.y * co;
        }
        inx += 1;
        (*cutInfo.shapes)[inx].cutStatus = 0;
        (*cutInfo.shapes)[inx].cutPen = 3;
        //(*cutInfo.shapes)[inx].cutMater = 11;
        (*cutInfo.shapes)[inx].pa = sin (-sline.angz_1);
        (*cutInfo.shapes)[inx].pb = cos (-sline.angz_1);
        (*cutInfo.shapes)[inx].pc = 0;
        (*cutInfo.shapes)[inx].pd = x;

        co = cos (sline.angz_2);
        si = sin (sline.angz_2);
        if (isx) {
            x = sline.s.c2.x * co + sline.s.c2.y * si;
        } else {
            x = -sline.s.c2.x * si + sline.s.c2.y * co;
        }
        inx += 1;
        (*cutInfo.shapes)[inx].cutStatus = 0;
        (*cutInfo.shapes)[inx].cutPen = 3;
        //(*cutInfo.shapes)[inx].cutMater = 11;
        (*cutInfo.shapes)[inx].pa = sin (-sline.angz_2);
        (*cutInfo.shapes)[inx].pb = cos (-sline.angz_2);
        (*cutInfo.shapes)[inx].pc = 0;
        (*cutInfo.shapes)[inx].pd = x;
        return err;
    }
    return err;
}

// -----------------------------------------------------------------------------
// Ищет 3д документ с именем и id. Если не находит - создаёт
// Возвращает информацию о БД API_DatabaseInfo& dbInfo
// -----------------------------------------------------------------------------
GSErrCode Get3DProjectionInfo (API_3DProjectionInfo& proj3DInfo, const double& angz, const double& koeff)
{
    BNZeroMemory (&proj3DInfo, sizeof (API_3DProjectionInfo));
    GSErrCode err = NoError;
#if defined(AC_27) || defined(AC_28)
    err = ACAPI_View_Get3DProjectionSets (&proj3DInfo);
#else
    err = ACAPI_Environment (APIEnv_Get3DProjectionSetsID, &proj3DInfo, nullptr, nullptr);
#endif
    if (err != NoError) {
        msg_rep ("Get3DProjectionInfo", "APIEnv_Get3DProjectionSetsID", err, APINULLGuid);
        return err;
}
    proj3DInfo.isPersp = false;
    proj3DInfo.u.axono.azimuth = angz * RADDEG + 90;
    proj3DInfo.u.axono.projMod = 1;
#if defined(AC_27) || defined(AC_28)
    err = ACAPI_View_Change3DProjectionSets (&proj3DInfo, nullptr);
#else
    err = ACAPI_Environment (APIEnv_Change3DProjectionSetsID, &proj3DInfo, nullptr, nullptr);
#endif
    if (err != NoError) {
        msg_rep ("Get3DProjectionInfo", "APIEnv_Change3DProjectionSetsID", err, APINULLGuid);
        return err;
    }
    BNZeroMemory (&proj3DInfo, sizeof (API_3DProjectionInfo));
#if defined(AC_27) || defined(AC_28)
    err = ACAPI_View_Get3DProjectionSets (&proj3DInfo);
#else
    err = ACAPI_Environment (APIEnv_Get3DProjectionSetsID, &proj3DInfo, nullptr, nullptr);
#endif
    if (err != NoError) {
        msg_rep ("Get3DProjectionInfo", "APIEnv_Get3DProjectionSetsID", err, APINULLGuid);
        return err;
    }
    proj3DInfo.u.axono.azimuth = angz * RADDEG + 90;
    proj3DInfo.u.axono.projMod = 15;
    proj3DInfo.u.axono.tranmat.tmx[0] = proj3DInfo.u.axono.tranmat.tmx[0] * koeff;
    proj3DInfo.u.axono.tranmat.tmx[4] = proj3DInfo.u.axono.tranmat.tmx[4] * koeff;
    proj3DInfo.u.axono.tranmat.tmx[1] = proj3DInfo.u.axono.tranmat.tmx[1] * koeff;
    proj3DInfo.u.axono.tranmat.tmx[5] = proj3DInfo.u.axono.tranmat.tmx[5] * koeff;
#if defined(AC_27) || defined(AC_28)
    err = ACAPI_View_Change3DProjectionSets (&proj3DInfo, nullptr);
#else
    err = ACAPI_Environment (APIEnv_Change3DProjectionSetsID, &proj3DInfo, nullptr, nullptr);
#endif
    if (err != NoError) {
        msg_rep ("Get3DProjectionInfo", "APIEnv_Change3DProjectionSetsID", err, APINULLGuid);
        return err;
    }
    if (err != NoError) {
        msg_rep ("Get3DProjectionInfo", "APIEnv_Get3DProjectionSetsID", err, APINULLGuid);
        return err;
    }
    return err;
    }
// -----------------------------------------------------------------------------
// Создание 3д документа для одного отрезка
// -----------------------------------------------------------------------------
GSErrCode Get3DDocument (API_DatabaseInfo& dbInfo, const GS::UniString& name, const GS::UniString& id)
{
    GSErrCode err = NoError;
    GS::Array<API_DatabaseUnId> dbases;

#if defined(AC_27) || defined(AC_28)
    err = ACAPI_Database_GetDocumentFrom3DDatabases (nullptr, &dbases);
#else
    err = ACAPI_Database (APIDb_GetDocumentFrom3DDatabasesID, nullptr, &dbases);
#endif
    if (err == NoError) {
        for (const auto& dbUnId : dbases) {
            BNZeroMemory (&dbInfo, sizeof (API_DatabaseInfo));
            dbInfo.databaseUnId = dbUnId;
#if defined(AC_27) || defined(AC_28)
            err = ACAPI_Window_GetDatabaseInfo (&dbInfo);
#else
            err = ACAPI_Database (APIDb_GetDatabaseInfoID, &dbInfo, nullptr);
#endif
            if (err != NoError) {
                msg_rep ("Get3DDocument", "APIDb_GetDatabaseInfoID", err, APINULLGuid);
                return err;
            }
            GS::UniString n = GS::UniString (dbInfo.name);
            GS::UniString i = GS::UniString (dbInfo.ref);
            if (n == name && i == id) {
                return err;
            }
        }
    } else {
        msg_rep ("Get3DDocument", "APIDb_GetDocumentFrom3DDatabasesID", err, APINULLGuid);
        return err;
    }
    BNZeroMemory (&dbInfo, sizeof (API_DatabaseInfo));
    dbInfo.typeID = APIWind_DocumentFrom3DID;
    GS::snuprintf (dbInfo.name, sizeof (dbInfo.name), name.ToUStr ().Get ());
    GS::snuprintf (dbInfo.ref, sizeof (dbInfo.ref), id.ToUStr ().Get ());
#if defined(AC_27) || defined(AC_28)
    err = ACAPI_Database_NewDatabase (&dbInfo);
#else
    err = ACAPI_Database (APIDb_NewDatabaseID, &dbInfo);
#endif
    if (err != NoError) {
        msg_rep ("Get3DDocument", "APIDb_NewDatabaseID", err, APINULLGuid);
        return err;
    }
    return err;
}
// -----------------------------------------------------------------------------
// Извлекает из морфа отрезки, сортирует их по удалению от startpos
// Возвращает массив отрезков и ID морфа
// -----------------------------------------------------------------------------
GSErrCode GetSectLine (API_Guid& elemguid, GS::Array<SSectLine>& lines, GS::UniString& id, const Point2D& startpos)
{
    API_Element element;
    BNZeroMemory (&element, sizeof (API_Element));
    element.header.guid = elemguid;
    GSErrCode err = ACAPI_Element_Get (&element);
    if (err != NoError) return err;
    if (!element.header.hasMemo) return err;
    API_ElementMemo memo;
    BNZeroMemory (&memo, sizeof (API_ElementMemo));
    err = ACAPI_Element_GetMemo (element.header.guid, &memo);
    if (err != NoError || memo.morphBody == nullptr) {
        msg_rep ("GetSectLine", "ACAPI_Element_GetMemo", err, APINULLGuid);
        ACAPI_DisposeElemMemoHdls (&memo);
        return err;
    }

    id = *memo.elemInfoString;
    if (memo.morphBody->IsWireBody () && !memo.morphBody->IsSolidBody ()) {
        Int32 nVertices = memo.morphBody->GetVertexCount ();
        GS::Array<Sector> k;
        GS::Array<Point2D> p;
        Int32 edgeCnt = memo.morphBody->GetEdgeCount ();
        API_Tranmat tm = element.morph.tranmat;
        for (Int32 iEdge = 0; iEdge < edgeCnt; iEdge++) {
            const EDGE& edge = memo.morphBody->GetConstEdge (iEdge);
            const VERT& vtx1 = memo.morphBody->GetConstVertex (edge.vert1);
            const VERT& vtx2 = memo.morphBody->GetConstVertex (edge.vert2);
            Point2D c1 = GetWordPoint2DTM ({ vtx1.x, vtx1.y }, tm);
            Point2D c2 = GetWordPoint2DTM ({ vtx2.x, vtx2.y }, tm);
            p.Push (c1); p.Push (c2);
            Sector s = { c1, c2 };
            k.Push (s);
        }

        Point2D start; Point2D end;
        bool find_start = false; bool find_end = false;
        for (UInt32 i = 0; i < p.GetSize (); i++) {
            if (p.Count (p[i]) == 1) {
                if (!find_start) {
                    start = p[i];
                    find_start = true;
                } else {
                    if (!find_end) {
                        end = p[i];
                        find_end = true;
                    } else {
                        int hh = 1;
                    }
                }
            }
        }
        Sector tostart = { startpos , start };
        Sector toend = { startpos , end };
        if (tostart.GetLength () > toend.GetLength ()) start = end;
        bool isend = false;
        UInt32 n = k.GetSize ();
        for (UInt32 i = 0; i < n; i++) {
            UInt32 inx = 0;
            if (GetNear (k, start, inx, isend)) {
                SSectLine s;

                if (isend) {
                    start = k[inx].c1;
                    s.s = { k[inx].c1, k[inx].c2 };
                } else {
                    start = k[inx].c2;
                    s.s = { k[inx].c2, k[inx].c1 };
                }
                double dx = s.s.c1.x - s.s.c2.x;
                double dy = s.s.c1.y - s.s.c2.y;
                double angz = 0.0;
                if (is_equal (dx, 0.0) && is_equal (dy, 0.0)) {
                    angz = 0.0;
                } else {
                    angz = atan2 (dy, dx) + PI;
                }
                s.angz = angz;
                s.angz_1 = angz + 0.5 * PI;
                s.angz_2 = angz - 0.5 * PI;
                lines.Push (s);
                k.Delete (inx);
            }
        }
        if (lines.GetSize () != n) {
            msg_rep ("GetSectLine", "Diff size lines", err, APINULLGuid);
        }
        ACAPI_DisposeElemMemoHdls (&memo);
    } else {
        msg_rep ("GetSectLine", "memo.morphBody->IsWireBody", err, APINULLGuid);
        ACAPI_DisposeElemMemoHdls (&memo);
        return err;
    }
    return err;
}
// -----------------------------------------------------------------------------
// Построение 3д документов вдоль морфа
// -----------------------------------------------------------------------------
GSErrCode DoSect (SSectLine& sline, const GS::UniString& name, const GS::UniString& id, const double& koeff)
{
    GSErrCode err = NoError;
    // Назначение секущих плоскостей по краям отрезка
    API_3DCutPlanesInfo cutInfo;
    if (GetCuplane (sline, cutInfo) == NoError) {
#if defined(AC_27) || defined(AC_28)
        err = ACAPI_View_Change3DCuttingPlanes (&cutInfo);
#else
        err = ACAPI_Environment (APIEnv_Change3DCuttingPlanesID, &cutInfo, nullptr);
#endif
        if (err != NoError) {
            msg_rep ("DoSect", "APIEnv_Change3DCuttingPlanesID", err, APINULLGuid);
            BMKillHandle ((GSHandle*) &(cutInfo.shapes));
            return err;
        }
    }

    // Установка камеры перпендикулярно отрезку
    API_3DProjectionInfo  proj3DInfo;
    if (Get3DProjectionInfo (proj3DInfo, sline.angz, koeff) != NoError) {
        msg_rep ("DoSect", "Get3DProjectionInfo", err, APINULLGuid);
        BMKillHandle ((GSHandle*) &(cutInfo.shapes));
        return err;
    }

    // Создание 3д документа
    API_DatabaseInfo dbInfo = {};
    if (Get3DDocument (dbInfo, name, id) != NoError) {
        msg_rep ("DoSect", "Get3DDocument", err, APINULLGuid);
        BMKillHandle ((GSHandle*) &(cutInfo.shapes));
        return err;
}

    API_DocumentFrom3DType documentFrom3DType;
#if defined(AC_27) || defined(AC_28)
    err = ACAPI_View_GetDocumentFrom3DSettings (&dbInfo.databaseUnId, &documentFrom3DType);
#else
    err = ACAPI_Environment (APIEnv_GetDocumentFrom3DSettingsID, &dbInfo.databaseUnId, &documentFrom3DType);
#endif
    if (err != NoError) {
        msg_rep ("DoSect", "APIEnv_GetDocumentFrom3DSettingsID", err, APINULLGuid);
        BMKillHandle (reinterpret_cast<GSHandle*> (&documentFrom3DType.cutSetting.shapes));
        BMKillHandle ((GSHandle*) &(cutInfo.shapes));
        return err;
    }
    documentFrom3DType.cutSetting.isCutPlanes = cutInfo.isCutPlanes;
    documentFrom3DType.cutSetting.useCustom = cutInfo.useCustom;
    documentFrom3DType.cutSetting.nShapes = cutInfo.nShapes;
    if (documentFrom3DType.cutSetting.shapes != nullptr) BMKillHandle ((GSHandle*) &(documentFrom3DType.cutSetting.shapes));
    documentFrom3DType.cutSetting.shapes = reinterpret_cast<API_3DCutShapeType**> (BMAllocateHandle (cutInfo.nShapes * sizeof (API_3DCutShapeType), ALLOCATE_CLEAR, 0));
    if (documentFrom3DType.cutSetting.shapes != nullptr) {
        for (short i = 0; i < cutInfo.nShapes; i++) {
            (*documentFrom3DType.cutSetting.shapes)[i] = (*cutInfo.shapes)[i];
        }
}
    documentFrom3DType.projectionSetting = proj3DInfo;
#if defined(AC_27) || defined(AC_28)
    err = ACAPI_View_ChangeDocumentFrom3DSettings (&dbInfo.databaseUnId, &documentFrom3DType);
#else
    err = ACAPI_Environment (APIEnv_ChangeDocumentFrom3DSettingsID, &dbInfo.databaseUnId, &documentFrom3DType);
#endif
    BMKillHandle (reinterpret_cast<GSHandle*> (&documentFrom3DType.cutSetting.shapes));
    BMKillHandle ((GSHandle*) &(cutInfo.shapes));
    if (err != NoError) {
        msg_rep ("DoSect", "APIEnv_ChangeDocumentFrom3DSettingsID", err, APINULLGuid);
        return err;
    }
    sline.tm = proj3DInfo.u.axono.tranmat;
    sline.databaseUnId = dbInfo.databaseUnId;
    return err;
}
// -----------------------------------------------------------------------------
// Размещение на созданном отрезке элементов оформления
// По краям отрезка устанавливаются hotspot
// -----------------------------------------------------------------------------
GSErrCode PlaceDocSect (SSectLine& sline, API_Element& elemline)
{
    GSErrCode err = NoError;
    API_WindowInfo windowInfo = {};
    API_DatabaseInfo dbInfo = {};
    windowInfo.typeID = APIWind_DocumentFrom3DID;
    windowInfo.databaseUnId = sline.databaseUnId;
#if defined(AC_27) || defined(AC_28)
    err = ACAPI_Window_ChangeWindow (&windowInfo);
#else
    err = ACAPI_Automate (APIDo_ChangeWindowID, &windowInfo, nullptr);
#endif
    if (err != NoError) {
        msg_rep ("PlaceDocSect", "APIDo_ChangeWindowID", err, APINULLGuid);
        return err;
    }
    dbInfo.typeID = APIWind_DocumentFrom3DID;
    dbInfo.databaseUnId = sline.databaseUnId;
#if defined(AC_27) || defined(AC_28)
    err = ACAPI_Database_ChangeCurrentDatabase (&dbInfo);
#else
    err = ACAPI_Database (APIDb_ChangeCurrentDatabaseID, &dbInfo, nullptr);
#endif
    if (err != NoError) {
        msg_rep ("PlaceDocSect", "APIDb_ChangeCurrentDatabaseID", err, APINULLGuid);
        return err;
    }
    GS::Array<API_Guid> elemList;
    err = ACAPI_Element_GetElemList (API_HotspotID, &elemList);
    if (err != NoError) {
        msg_rep ("PlaceDocSect", "ACAPI_Element_GetElemList", err, APINULLGuid);
        return err;
    }
    err = ACAPI_CallUndoableCommand ("Create text",
    [&]() -> GSErrCode {
        GSErrCode err = NoError;
        if (!elemList.IsEmpty ()) err = ACAPI_Element_Delete (elemList);
        if (err != NoError) {
            msg_rep ("PlaceDocSect", "ACAPI_Element_Delete", err, APINULLGuid);
            return err;
        }
        Point2D endC = GetWordPoint2DTM (sline.s.c1, sline.tm);
        elemline.hotspot.pos.x = endC.x;
        elemline.hotspot.pos.y = 0;
        err = ACAPI_Element_Create (&elemline, nullptr);
        if (err != NoError) {
            msg_rep ("PlaceDocSect", "ACAPI_Element_Create", err, APINULLGuid);
            return err;
        }
        endC = GetWordPoint2DTM (sline.s.c2, sline.tm);
        elemline.hotspot.pos.x = endC.x;
        elemline.hotspot.pos.y = 0;
        err = ACAPI_Element_Create (&elemline, nullptr);
        if (err != NoError) {
            msg_rep ("PlaceDocSect", "ACAPI_Element_Create", err, APINULLGuid);
            return err;
        }
        return err;
    });
    if (err != NoError) {
        return err;
    }
    return err;
}
// -----------------------------------------------------------------------------
// Построение 3д документов вдоль морфа
// -----------------------------------------------------------------------------
void ProfileByLine ()
{
    GSErrCode err = NoError;
    GS::Array<API_Guid> elems = GetSelectedElements2 (true, true);
    if (elems.IsEmpty ()) return;
    Point2D startpos;

    if (!ClickAPoint ("Click point", &startpos))
        return;

    API_Element elemline;
    GS::Array<SSectLine> lines;
    API_DatabaseInfo databasestart;
    API_WindowInfo windowstart;
    GS::IntPtr	store = 1;
#if defined(AC_27) || defined(AC_28)
    err = ACAPI_View_StoreViewSettings (store);
#else
    err = ACAPI_Database (APIDb_StoreViewSettingsID, (void*) store);
#endif
    if (err != NoError) {
        store = -1;
        err = NoError;
    }
    err = ACAPI_CallCommand ("Create 3d",
    [&]() -> GSErrCode {
        GSErrCode err = NoError;
        // Получаем настройку хотспотов, которые будем расставлять на краях
        BNZeroMemory (&elemline, sizeof (API_Element));
        API_AttributeIndex layer;
#if defined AC_26 || defined AC_27 || defined AC_28
        elemline.header.type.typeID = API_HotspotID;
        layer = ACAPI_CreateAttributeIndex (1);
#else
        elemline.header.typeID = API_HotspotID;
        layer = 1;
#endif
        err = ACAPI_Element_GetDefaults (&elemline, nullptr);
        if (err != NoError) {
            msg_rep ("ProfileByLine", "ACAPI_Element_GetDefaults", err, APINULLGuid);
            return err;
    }
        elemline.header.layer = layer;
        elemline.hotspot.pen = 143;
        BNZeroMemory (&databasestart, sizeof (API_DatabaseInfo));
#if defined(AC_27) || defined(AC_28)
        err = ACAPI_Database_GetCurrentDatabase (&databasestart);
#else
        err = ACAPI_Database (APIDb_GetCurrentDatabaseID, &databasestart, nullptr);
#endif
        if (err != NoError) {
            msg_rep ("ProfileByLine", "APIDb_GetCurrentDatabaseID", err, APINULLGuid);
            return err;
        }
        BNZeroMemory (&windowstart, sizeof (API_WindowInfo));
#if defined(AC_27) || defined(AC_28)
        err = ACAPI_Window_GetCurrentWindow (&windowstart);
#else
        err = ACAPI_Database (APIDb_GetCurrentWindowID, &windowstart, nullptr);
#endif
        if (err != NoError) {
            msg_rep ("ProfileByLine", "APIDb_GetCurrentWindowID", err, APINULLGuid);
            return err;
        }

#ifdef AC_25
        API_3DImageInfo imageInfo;
        err = ACAPI_Environment (APIEnv_Get3DImageSetsID, &imageInfo);
        if (err != NoError) {
            msg_rep ("ProfileByLine", "APIEnv_Get3DImageSetsID", err, APINULLGuid);
            return err;
        }
        bool setMustConvert = true;
        imageInfo.allStories = true;
        imageInfo.trimToMark = false;
        imageInfo.trimToStoryRange = false;
        err = ACAPI_Environment (APIEnv_Change3DImageSetsID, &imageInfo, &setMustConvert, nullptr);
        if (err != NoError) {
            msg_rep ("ProfileByLine", "APIEnv_Change3DImageSetsID", err, APINULLGuid);
        }
#endif
#if defined(AC_27) || defined(AC_28)
        err = ACAPI_View_ShowAllIn3D ();
#else
        err = ACAPI_Automate (APIDo_ShowAllIn3DID, nullptr, nullptr);
#endif
        if (err != NoError) {
            msg_rep ("ProfileByLine", "APIDo_ShowAllIn3DID", err, APINULLGuid);
        }
        GS::UniString id = "";
        GS::UniString name = "Участок ";
        err = GetSectLine (elems[0], lines, id, startpos);
        if (err != NoError) {
            msg_rep ("ProfileByLine", "GetSectLine", err, APINULLGuid);
            return err;
        }
        double koeff = 0.2;
        if (id.Contains ("@")) {
            GS::Array<GS::UniString> sr;
            UInt32 nsr = StringSplt (id, "@", sr);
            if (nsr > 1) {
                double x;
                if (UniStringToDouble (sr[1], x)) {
                    if (x > 0) koeff = (1 / x) * 100.0;
                }
            }
            id = sr[0];
        }
        for (UInt32 i = 0; i < lines.GetSize (); i++) {
            GS::UniString id_ = id + GS::UniString::Printf (".%d", i + 1);
            err = DoSect (lines[i], name + GS::UniString::Printf (" %d", i + 1), id_, koeff);
            if (err != NoError) return err;
        }
        return err;
});

    for (UInt32 i = 0; i < lines.GetSize (); i++) {
        if (PlaceDocSect (lines[i], elemline) != NoError) {
            msg_rep ("ProfileByLine", "PlaceDocSect", err, APINULLGuid);
            // Возвращение на исходную БД и окно
            //err = ACAPI_Database (APIDb_ChangeCurrentDatabaseID, &databasestart, nullptr);
            //if (err != NoError) {
            //    msg_rep ("ProfileByLine", "APIDb_ChangeCurrentDatabaseID", err, APINULLGuid);
            //    return;
            //}
            //err = ACAPI_Automate (APIDo_ChangeWindowID, &windowstart, nullptr);
            //if (err != NoError) {
            //    msg_rep ("ProfileByLine", "APIDo_ChangeWindowID", err, APINULLGuid);
            //    return;
            //}
        }
    }
    // Возвращение на исходную БД и окно
#if defined(AC_27) || defined(AC_28)
    err = ACAPI_Database_ChangeCurrentDatabase (&databasestart);
#else
    err = ACAPI_Database (APIDb_ChangeCurrentDatabaseID, &databasestart, nullptr);
#endif
    if (err != NoError) {
        msg_rep ("ProfileByLine", "APIDb_ChangeCurrentDatabaseID", err, APINULLGuid);
        return;
    }
#if defined(AC_27) || defined(AC_28)
    err = ACAPI_Window_ChangeWindow (&windowstart);
#else
    err = ACAPI_Automate (APIDo_ChangeWindowID, &windowstart, nullptr);
#endif
    if (err != NoError) {
        msg_rep ("ProfileByLine", "APIDo_ChangeWindowID", err, APINULLGuid);
        return;
    }
    API_3DCutPlanesInfo cutInfo;
    BNZeroMemory (&cutInfo, sizeof (API_3DCutPlanesInfo));
#if defined(AC_27) || defined(AC_28)
    err = ACAPI_View_Get3DCuttingPlanes (&cutInfo);
#else
    err = ACAPI_Environment (APIEnv_Get3DCuttingPlanesID, &cutInfo, nullptr);
#endif
    if (err != NoError) {
        msg_rep ("ProfileByLine", "APIEnv_Get3DCuttingPlanesID", err, APINULLGuid);
    } else {
        cutInfo.isCutPlanes = false;
#if defined(AC_27) || defined(AC_28)
        err = ACAPI_View_Change3DCuttingPlanes (&cutInfo);
#else
        err = ACAPI_Environment (APIEnv_Change3DCuttingPlanesID, &cutInfo, nullptr);
#endif
        if (err != NoError) {
            msg_rep ("ProfileByLine", "APIEnv_Change3DCuttingPlanesID", err, APINULLGuid);
    }
    }
    BMKillHandle ((GSHandle*) &(cutInfo.shapes));
    if (store == 1) {
        store = 0;
#if defined(AC_27) || defined(AC_28)
        ACAPI_View_StoreViewSettings (store);
#else
        ACAPI_Database (APIDb_StoreViewSettingsID, (void*) store);
#endif
    }
}
// -----------------------------------------------------------------------------
// Выравнивание одного чертежа
// Возвращает сдвинутую на ширину чертежа координату
// -----------------------------------------------------------------------------
GSErrCode AlignOneDrawingsByPoints (const API_Guid& elemguid, API_DatabaseInfo& databasestart, API_WindowInfo& windowstart, const API_Coord& zeropos, API_Coord& startpos, API_Coord& drawingpos)
{
    GSErrCode err = NoError;
    API_Element element;
    BNZeroMemory (&element, sizeof (API_Element));
    element.header.guid = elemguid;
    err = ACAPI_Element_Get (&element);
    if (err != NoError) {
        msg_rep ("AlignOneDrawingsByPoints", "ACAPI_Element_Get", err, APINULLGuid);
        return err;
    }
#if defined AC_26 || defined AC_27 || defined AC_28
    if (element.header.type.typeID != API_DrawingID) return APIERR_GENERAL;
#else
    if (element.header.typeID != API_DrawingID) return APIERR_GENERAL;
#endif
    API_DatabaseInfo	dbInfo;
    BNZeroMemory (&dbInfo, sizeof (API_DatabaseInfo));
    dbInfo.typeID = APIWind_DrawingID;
    dbInfo.linkedElement = element.header.guid;
#if defined(AC_27) || defined(AC_28)
    err = ACAPI_Database_ChangeCurrentDatabase (&dbInfo);
#else
    err = ACAPI_Database (APIDb_ChangeCurrentDatabaseID, &dbInfo, nullptr);
#endif
    if (err != NoError) {
        msg_rep ("AlignOneDrawingsByPoints", "APIDb_ChangeCurrentDatabaseID", err, APINULLGuid);
        return err;
    }
    GS::Array<API_Guid> hotspotList;
    err = ACAPI_Element_GetElemList (API_HotspotID, &hotspotList);
    if (err != NoError) {
        msg_rep ("AlignOneDrawingsByPoints", "ACAPI_Element_GetElemList", err, APINULLGuid);
        return err;
    }
    if (hotspotList.IsEmpty ()) return APIERR_GENERAL;
    //Вычисление новых координат
    GS::Array<API_Coord> hotspotcoord;
    API_Element hotspotelem;
    bool flag_find = false;
    API_AttributeIndex layer;
#if defined AC_26 || defined AC_27 || defined AC_28
    layer = ACAPI_CreateAttributeIndex (1);
#else
    layer = 1;
#endif
    for (UInt32 i = 0; i < hotspotList.GetSize (); i++) {
        BNZeroMemory (&hotspotelem, sizeof (API_Element));
        hotspotelem.header.guid = hotspotList[i];
        err = ACAPI_Element_Get (&hotspotelem);
        if (err == NoError) {
            // Точки для чертежей, которые должны совпадать с началом
            if (hotspotelem.header.layer == layer && hotspotelem.hotspot.pen == 163) {
                flag_find = true;
            }
            // Автоматически расставленные точки по краям склейки
            if (hotspotelem.header.layer == layer && (hotspotelem.hotspot.pen == 143 || hotspotelem.hotspot.pen == 163)) {
                hotspotcoord.Push (hotspotelem.hotspot.pos);
            }
        }
    }
    if (hotspotcoord.GetSize () < 2 && !flag_find) return APIERR_GENERAL;
    double kscale = element.drawing.drawingScale;
    API_Coord leftpos = hotspotcoord[0]; API_Coord rightpos = hotspotcoord[0];
    for (UInt32 i = 1; i < hotspotcoord.GetSize (); i++) {
        if (hotspotcoord[i].x < leftpos.x) leftpos = hotspotcoord[i];
        if (hotspotcoord[i].x > rightpos.x) rightpos = hotspotcoord[i];
    }
    double l = fabs (rightpos.x - leftpos.x);
    // Для чертежей с неподвижным началом не будем добавлять длину чертежа для последующих
    if (flag_find) {
        drawingpos = { zeropos.x - leftpos.x * kscale ,zeropos.y };
    } else {
        drawingpos = { startpos.x - leftpos.x * kscale ,startpos.y };
        startpos.x = startpos.x + l * kscale;
    }
    // Возвращение на исходную БД и окно
#if defined(AC_27) || defined(AC_28)
    err = ACAPI_Database_ChangeCurrentDatabase (&databasestart);
#else
    err = ACAPI_Database (APIDb_ChangeCurrentDatabaseID, &databasestart, nullptr);
#endif
    if (err != NoError) {
        msg_rep ("AlignOneDrawingsByPoints", "APIDb_ChangeCurrentDatabaseID", err, APINULLGuid);
        return err;
    }
#if defined(AC_27) || defined(AC_28)
    err = ACAPI_Window_ChangeWindow (&windowstart);
#else
    err = ACAPI_Automate (APIDo_ChangeWindowID, &windowstart, nullptr);
#endif
    if (err != NoError) {
        msg_rep ("AlignOneDrawingsByPoints", "APIDo_ChangeWindowID", err, APINULLGuid);
        return err;
    }
    return err;
}
// -----------------------------------------------------------------------------
// Возвращает список API_Guid чертежей, отсортированных по именам
// -----------------------------------------------------------------------------
GS::Array<API_Guid> GetDrawingsSort (const GS::Array<API_Guid>& elems)
{
    GSErrCode err = NoError;
    SortByName drawingId;
    API_DrawingLinkInfo drawingLinkInfo;
    GS::Array<API_Guid> drawings;
    for (UInt32 i = 0; i < elems.GetSize (); i++) {
        BNZeroMemory (&drawingLinkInfo, sizeof (API_DrawingLinkInfo));
#if defined(AC_27) || defined(AC_28)
        err = ACAPI_Drawing_GetDrawingLink (&(elems[i]), &drawingLinkInfo);
#else
        err = ACAPI_Database (APIDb_GetDrawingLinkID, (void*) (&(elems[i])), &drawingLinkInfo);
#endif
        if (err == NoError) {
            if (drawingLinkInfo.linkPath != nullptr) delete drawingLinkInfo.linkPath;
            if (drawingLinkInfo.viewPath != nullptr) BMKillPtr (&drawingLinkInfo.viewPath);
            if (!drawingLinkInfo.viewDeleted && drawingLinkInfo.linkTypeID == API_DrawingLink_InternalViewID) {
                GS::UniString name = GS::UniString (drawingLinkInfo.number) + GS::UniString (drawingLinkInfo.name);
                std::string key = name.ToCStr (0, MaxUSize, GChCode).Get ();
                drawingId[key] = elems[i];
            }
        }
    }
    for (SortByName::iterator i = drawingId.begin (); i != drawingId.end (); ++i) {
        API_Guid& elemguid = i->second;
        drawings.Push (elemguid);
    }
    return drawings;
}
// -----------------------------------------------------------------------------
// Выравнивание чертежей по расположенным в них hotspot
// -----------------------------------------------------------------------------
void AlignDrawingsByPoints ()
{
    GSErrCode err = NoError;
    GS::Array<API_Guid> elems = GetSelectedElements2 (true, true);
    if (elems.IsEmpty ()) return;
    Point2D startpos_;
    if (!ClickAPoint ("Click point", &startpos_))
        return;
    API_Coord startpos = { startpos_.x, startpos_.y };
    API_Coord zeropos = startpos;
    // Запоминаем настройки отображения 
    GS::IntPtr	store = 1;
#if defined(AC_27) || defined(AC_28)
    err = ACAPI_View_StoreViewSettings (store);
#else
    err = ACAPI_Database (APIDb_StoreViewSettingsID, (void*) store);
#endif
    if (err != NoError) {
        store = -1;
        err = NoError;
}
    API_DatabaseInfo databasestart;
    API_WindowInfo windowstart;
    BNZeroMemory (&databasestart, sizeof (API_DatabaseInfo));
#if defined(AC_27) || defined(AC_28)
    err = ACAPI_Database_GetCurrentDatabase (&databasestart);
#else
    err = ACAPI_Database (APIDb_GetCurrentDatabaseID, &databasestart, nullptr);
#endif
    if (err != NoError) {
        msg_rep ("AlignDrawingsByPoints", "APIDb_GetCurrentDatabaseID", err, APINULLGuid);
        return;
    }
    BNZeroMemory (&windowstart, sizeof (API_WindowInfo));
#if defined(AC_27) || defined(AC_28)
    err = ACAPI_Window_GetCurrentWindow (&windowstart);
#else
    err = ACAPI_Database (APIDb_GetCurrentWindowID, &windowstart, nullptr);
#endif
    if (err != NoError) {
        msg_rep ("AlignDrawingsByPoints", "APIDb_GetCurrentWindowID", err, APINULLGuid);
        return;
    }
    // Важно расставлять чертежи по-порядку.
    GS::Array<API_Guid> drawingId = GetDrawingsSort (elems);
    if (drawingId.IsEmpty ()) return;
    GS::Array<API_Coord> coords;
    GS::Array<API_Guid> gooddrawings;
    for (UInt32 i = 0; i < drawingId.GetSize (); i++) {
        API_Coord drawingpos = startpos;
        err = AlignOneDrawingsByPoints (drawingId[i], databasestart, windowstart, zeropos, startpos, drawingpos);
        if (err != NoError) {
            msg_rep ("AlignDrawingsByPoints", "AlignOneDrawingsByPoints", err, APINULLGuid);
        } else {
            coords.Push (drawingpos);
            gooddrawings.Push (drawingId[i]);
        }
    }
    // Возвращение на исходную БД и окно
#if defined(AC_27) || defined(AC_28)
    err = ACAPI_Database_ChangeCurrentDatabase (&databasestart);
#else
    err = ACAPI_Database (APIDb_ChangeCurrentDatabaseID, &databasestart, nullptr);
#endif
    if (err != NoError) {
        msg_rep ("AlignOneDrawingsByPoints", "APIDb_ChangeCurrentDatabaseID", err, APINULLGuid);
        return;
    }
#if defined(AC_27) || defined(AC_28)
    err = ACAPI_Window_ChangeWindow (&windowstart);
#else
    err = ACAPI_Automate (APIDo_ChangeWindowID, &windowstart, nullptr);
#endif
    if (err != NoError) {
        msg_rep ("AlignOneDrawingsByPoints", "APIDo_ChangeWindowID", err, APINULLGuid);
        return;
    }
    if (gooddrawings.IsEmpty ()) return;
    if (coords.IsEmpty ()) return;
    API_Element element, mask;
    err = ACAPI_CallUndoableCommand ("Move drawing",
    [&]() -> GSErrCode {
        GSErrCode err = NoError;
        for (UInt32 i = 0; i < gooddrawings.GetSize (); i++) {
            BNZeroMemory (&element, sizeof (API_Element));
            element.header.guid = gooddrawings[i];
            err = ACAPI_Element_Get (&element);
            ACAPI_ELEMENT_MASK_CLEAR (mask);
            ACAPI_ELEMENT_MASK_SET (mask, API_DrawingType, pos);
            ACAPI_ELEMENT_MASK_SET (mask, API_DrawingType, useOwnOrigoAsAnchor);
            ACAPI_ELEMENT_MASK_SET (mask, API_DrawingType, isCutWithFrame);
            element.drawing.pos = coords[i];
            element.drawing.useOwnOrigoAsAnchor = true;
            element.drawing.isCutWithFrame = false;
            err = ACAPI_Element_Change (&element, &mask, nullptr, 0, true);
        }
        return err;
    });
    if (store == 1) {
        store = 0;
#if defined(AC_27) || defined(AC_28)
        ACAPI_View_StoreViewSettings (store);
#else
        ACAPI_Database (APIDb_StoreViewSettingsID, (void*) store);
#endif
    }
#if defined(AC_27) || defined(AC_28)
    ACAPI_View_Zoom (nullptr, nullptr);
#else
    ACAPI_Automate (APIDo_ZoomID, nullptr, nullptr);
#endif
    if (err != NoError) {
        msg_rep ("AlignOneDrawingsByPoints", "ACAPI_Element_Change", err, APINULLGuid);
        return;
    }
    return;
}

void KM_ListUpdate ()
{
    GS::Array<API_Guid> guidArray = GetSelectedElements (true, true, false);
    GS::Array<API_Guid> elements;
    GS::Array<API_Guid> lines;
    for (UInt32 i = 0; i < guidArray.GetSize (); i++) {
        API_Elem_Head elem_head;
        elem_head.guid = guidArray[i];
        if (ACAPI_Element_GetHeader (&elem_head) == NoError) {
#if defined AC_26 || defined AC_27 || defined AC_28
            API_ElemTypeID elementType = elem_head.type.typeID;
#else
            API_ElemTypeID elementType = elem_head.typeID;
#endif
            if (elementType == API_ObjectID) elements.Push (guidArray[i]);
            if (elementType == API_PolyLineID || elementType == API_LineID) lines.Push (guidArray[i]);
        }
    }
    if (elements.IsEmpty () || lines.IsEmpty ()) return;
    GS::Array<API_Coord> coords;
    for (UInt32 i = 0; i < lines.GetSize (); i++) {
        API_Element element = {};
        BNZeroMemory (&element, sizeof (API_Element));
        element.header.guid = lines[i];
        if (ACAPI_Element_Get (&element) == NoError) {
            coords.Push (element.line.begC);
        }
    }
    ACAPI_CallUndoableCommand ("undoString", [&]() -> GSErrCode {
        GSErrCode err = KM_WriteGDL (elements.Get (0), coords);
        return err;
    });
}

GSErrCode KM_WriteGDL (API_Guid elemGuid, GS::Array<API_Coord>& coords)
{
    if (coords.IsEmpty ()) return APIERR_GENERAL;
    if (elemGuid == APINULLGuid) return APIERR_GENERAL;
    GSErrCode err = NoError;
    API_Elem_Head elem_head = {};
    API_Element element = {};
    API_Element mask = {};
    elem_head.guid = elemGuid;
    err = ACAPI_Element_GetHeader (&elem_head); if (err != NoError) return err;
    element.header.guid = elemGuid;
    err = ACAPI_Element_Get (&element); if (err != NoError) return err;
    API_ParamOwnerType	apiOwner = {};
    API_GetParamsType	apiParams = {};
    BNZeroMemory (&apiOwner, sizeof (API_ParamOwnerType));
    BNZeroMemory (&apiParams, sizeof (API_GetParamsType));
    apiOwner.guid = elemGuid;
#if defined AC_26 || defined AC_27 || defined AC_28
    apiOwner.type = elem_head.type;
#else
    apiOwner.typeID = elem_head.typeID;
#endif
#if defined(AC_27) || defined(AC_28)
    err = ACAPI_LibraryPart_OpenParameters (&apiOwner);
    if (err != NoError) {
        ACAPI_LibraryPart_CloseParameters ();
        return err;
}
    err = ACAPI_LibraryPart_GetActParameters (&apiParams);
    if (err != NoError) {
        ACAPI_LibraryPart_CloseParameters ();
        return err;
    }
#else
    err = ACAPI_Goodies (APIAny_OpenParametersID, &apiOwner, nullptr);
    if (err != NoError) {
        ACAPI_Goodies (APIAny_CloseParametersID, nullptr, nullptr);
        return err;
    }
    err = ACAPI_Goodies (APIAny_GetActParametersID, &apiParams);
    if (err != NoError) {
        ACAPI_Goodies (APIAny_CloseParametersID, nullptr, nullptr);
        return err;
    }
#endif
    Int32 n_t = coords.GetSize ();
    Int32 inx_kontur = 0;
    API_ChangeParamType	chgParam;
    Int32 addParNum = BMGetHandleSize ((GSHandle) apiParams.params) / sizeof (API_AddParType);
    for (Int32 i = 0; i < addParNum; ++i) {
        GS::UniString name = (*apiParams.params)[i].name;
        if (name.IsEqual ("kontur") && (*apiParams.params)[i].typeMod == API_ParArray) {
            inx_kontur = i;
            BNZeroMemory (&chgParam, sizeof (API_ChangeParamType));
            chgParam.index = (*apiParams.params)[i].index;
            CHTruncate ((*apiParams.params)[i].name, chgParam.name, API_NameLen);
            chgParam.realValue = 0;
            chgParam.ind1 = (*apiParams.params)[i].dim1;
            chgParam.ind2 = (*apiParams.params)[i].dim2;
#if defined(AC_27) || defined(AC_28)
            err = ACAPI_LibraryPart_ChangeAParameter (&chgParam); if (err != NoError) return err;
#else
            err = ACAPI_Goodies (APIAny_ChangeAParameterID, &chgParam, nullptr); if (err != NoError) return err;
#endif
        }
        if (name.IsEqual ("n_t")) {
            BNZeroMemory (&chgParam, sizeof (API_ChangeParamType));
            chgParam.index = (*apiParams.params)[i].index;
            CHTruncate ((*apiParams.params)[i].name, chgParam.name, API_NameLen);
            chgParam.realValue = n_t;
#if defined(AC_27) || defined(AC_28)
            err = ACAPI_LibraryPart_ChangeAParameter (&chgParam); if (err != NoError) return err;
#else
            err = ACAPI_Goodies (APIAny_ChangeAParameterID, &chgParam, nullptr); if (err != NoError) return err;
#endif
        }
        }
#if defined(AC_27) || defined(AC_28)
    err = ACAPI_LibraryPart_GetActParameters (&apiParams); if (err != NoError) return err;
    err = ACAPI_LibraryPart_CloseParameters (); if (err != NoError) return err;
#else
    err = ACAPI_Goodies (APIAny_GetActParametersID, &apiParams); if (err != NoError) return err;
    err = ACAPI_Goodies (APIAny_CloseParametersID, nullptr, nullptr); if (err != NoError) return err;
#endif
    Int32 inDim1 = (*apiParams.params)[inx_kontur].dim1;
    Int32 inDim2 = (*apiParams.params)[inx_kontur].dim2;
    size_t ind = 0;
    double** newArrHdl = (double**) (*apiParams.params)[inx_kontur].value.array;
    for (Int32 i = 0; i < inDim1; i++) {
        if (i < n_t) {
            (*newArrHdl)[ind] = coords.Get (i).x;
            ind++;
            (*newArrHdl)[ind] = coords.Get (i).y;
            ind++;
        } else {
            (*newArrHdl)[ind] = 0;
            ind++;
            (*newArrHdl)[ind] = 0;
            ind++;
        }
    }
    API_ElementMemo	elemMemo = {};
    BNZeroMemory (&elemMemo, sizeof (elemMemo));
    (*apiParams.params)[inx_kontur].value.array = (GSHandle) newArrHdl;
    elemMemo.params = apiParams.params;
    ACAPI_ELEMENT_MASK_CLEAR (mask);
    err = ACAPI_Element_Change (&element, &mask, &elemMemo, APIMemoMask_AddPars, true);
    ACAPI_DisposeAddParHdl (&apiParams.params);
    return err;
    }
}
#endif
