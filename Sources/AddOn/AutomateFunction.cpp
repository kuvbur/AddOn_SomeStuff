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

double angle (API_Coord3D& begC1, API_Coord3D& endC1, API_Coord3D& begC2, API_Coord3D& endC2)
{
    double x1 = endC1.x - begC1.x;
    double y1 = endC1.y - begC1.y;
    double x2 = endC2.x - begC2.x;
    double y2 = endC2.y - begC2.y;
    double t = (x1 * x2 + y1 * y2) / (sqrt (x1 * x1 + y1 * y1) * sqrt (x2 * x2 + y2 * y2));
    if (t < -1.0) t = -1.0;
    else if (t > 1.0) t = 1.0;
    return Geometry::ArcCos (t);
}

GSErrCode GetCuplane (const SSectLine sline, API_3DCutPlanesInfo& cutInfo)
{
    BNZeroMemory (&cutInfo, sizeof (API_3DCutPlanesInfo));
    GSErrCode err = ACAPI_Environment (APIEnv_Get3DCuttingPlanesID, &cutInfo, nullptr);
    if (err != NoError) {
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
        (*cutInfo.shapes)[inx].cutMater = 11;
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
        (*cutInfo.shapes)[inx].cutMater = 11;
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
        (*cutInfo.shapes)[inx].cutMater = 11;
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
        (*cutInfo.shapes)[inx].cutMater = 11;
        (*cutInfo.shapes)[inx].pa = sin (-sline.angz_2);
        (*cutInfo.shapes)[inx].pb = cos (-sline.angz_2);
        (*cutInfo.shapes)[inx].pc = 0;
        (*cutInfo.shapes)[inx].pd = x;
        return err;
    }
    return err;
}

GSErrCode Get3DProjectionInfo (API_3DProjectionInfo& proj3DInfo, double& angz)
{
    BNZeroMemory (&proj3DInfo, sizeof (API_3DProjectionInfo));
    GSErrCode err = ACAPI_Environment (APIEnv_Get3DProjectionSetsID, &proj3DInfo, nullptr, nullptr);
    if (err != NoError) {
        return err;
    }
    proj3DInfo.isPersp = false;
    proj3DInfo.u.axono.azimuth = angz * RADDEG + 90;
    proj3DInfo.u.axono.projMod = 1;
    err = ACAPI_Environment (APIEnv_Change3DProjectionSetsID, &proj3DInfo, nullptr, nullptr);
    if (err != NoError) {
        return err;
    }
    BNZeroMemory (&proj3DInfo, sizeof (API_3DProjectionInfo));
    err = ACAPI_Environment (APIEnv_Get3DProjectionSetsID, &proj3DInfo, nullptr, nullptr);
    if (err != NoError) {
        return err;
    }
    proj3DInfo.u.axono.azimuth = angz * RADDEG + 90;
    proj3DInfo.u.axono.projMod = 15;
    proj3DInfo.u.axono.tranmat.tmx[0] = proj3DInfo.u.axono.tranmat.tmx[0] * 0.5;
    proj3DInfo.u.axono.tranmat.tmx[4] = proj3DInfo.u.axono.tranmat.tmx[4] * 0.5;
    proj3DInfo.u.axono.tranmat.tmx[1] = proj3DInfo.u.axono.tranmat.tmx[1] * 0.5;
    proj3DInfo.u.axono.tranmat.tmx[5] = proj3DInfo.u.axono.tranmat.tmx[5] * 0.5;
    err = ACAPI_Environment (APIEnv_Change3DProjectionSetsID, &proj3DInfo, nullptr, nullptr);
    if (err != NoError) {
        return err;
    }
    return err;
}

GSErrCode Get3DDocument (API_DatabaseInfo& dbInfo, const GS::UniString& name, const GS::UniString& id)
{
    GSErrCode err = NoError;
    GS::Array<API_DatabaseUnId> dbases;
    err = ACAPI_Database (APIDb_GetDocumentFrom3DDatabasesID, nullptr, &dbases);
    if (err == NoError) {
        for (const auto& dbUnId : dbases) {
            BNZeroMemory (&dbInfo, sizeof (API_DatabaseInfo));
            dbInfo.databaseUnId = dbUnId;
            err = ACAPI_Database (APIDb_GetDatabaseInfoID, &dbInfo, nullptr);
            if (err != NoError) {
                return err;
            }
            GS::UniString n = GS::UniString (dbInfo.name);
            GS::UniString i = GS::UniString (dbInfo.ref);
            if (n == name && i == id) {
                return err;
            }
        }
    }
    if (err != NoError) {
        return err;
    }
    BNZeroMemory (&dbInfo, sizeof (API_DatabaseInfo));
    dbInfo.typeID = APIWind_DocumentFrom3DID;
    GS::snuprintf (dbInfo.name, sizeof (dbInfo.name), name.ToUStr ().Get ());
    GS::snuprintf (dbInfo.ref, sizeof (dbInfo.ref), id.ToUStr ().Get ());
    err = ACAPI_Database (APIDb_NewDatabaseID, &dbInfo);
    if (err != NoError) {
        return err;
    }
    return err;
}

GSErrCode GetSectLine (API_Guid& elemguid, GS::Array<SSectLine>& lines, GS::UniString& id, Point2D& startpos)
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
            int hh = 1;
        }
        ACAPI_DisposeElemMemoHdls (&memo);
    } else {
        ACAPI_DisposeElemMemoHdls (&memo);
        return err;
    }
    return err;
}

GSErrCode DoSect (SSectLine& sline, const GS::UniString& name, const GS::UniString& id)
{
    GSErrCode err = NoError;
    // Назначение секущих плоскостей по краям отрезка
    API_3DCutPlanesInfo cutInfo;
    if (GetCuplane (sline, cutInfo) != NoError) {
        err = ACAPI_Environment (APIEnv_Change3DCuttingPlanesID, &cutInfo, nullptr);
        if (err != NoError) {
            BMKillHandle ((GSHandle*) &(cutInfo.shapes));
            return err;
        }
    }

    // Установка камеры перпендикулярно отрезку
    API_3DProjectionInfo  proj3DInfo;
    if (Get3DProjectionInfo (proj3DInfo, sline.angz) != NoError) {
        BMKillHandle ((GSHandle*) &(cutInfo.shapes));
        return err;
    }

    // Создание 3д документа
    API_DatabaseInfo dbInfo = {};
    if (Get3DDocument (dbInfo, name, id) != NoError) {
        BMKillHandle ((GSHandle*) &(cutInfo.shapes));
        return err;
    }

    API_DocumentFrom3DType documentFrom3DType;
    err = ACAPI_Environment (APIEnv_GetDocumentFrom3DSettingsID, &dbInfo.databaseUnId, &documentFrom3DType);
    if (err != NoError) {
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
    err = ACAPI_Environment (APIEnv_ChangeDocumentFrom3DSettingsID, &dbInfo.databaseUnId, &documentFrom3DType);
    BMKillHandle (reinterpret_cast<GSHandle*> (&documentFrom3DType.cutSetting.shapes));
    BMKillHandle ((GSHandle*) &(cutInfo.shapes));
    if (err != NoError) {
        return err;
    }
    sline.tm = proj3DInfo.u.axono.tranmat;
    sline.databaseUnId = dbInfo.databaseUnId;
    return err;
}

GSErrCode PlaceDocSect (SSectLine& sline, API_Element& elemline)
{
    GSErrCode err = NoError;
    API_WindowInfo windowInfo = {};
    API_DatabaseInfo dbInfo = {};
    windowInfo.typeID = APIWind_DocumentFrom3DID;
    windowInfo.databaseUnId = sline.databaseUnId;
    err = ACAPI_Automate (APIDo_ChangeWindowID, &windowInfo, nullptr);
    if (err != NoError) {
        return err;
    }
    dbInfo.typeID = APIWind_DocumentFrom3DID;
    dbInfo.databaseUnId = sline.databaseUnId;
    err = ACAPI_Database (APIDb_ChangeCurrentDatabaseID, &dbInfo, nullptr);
    if (err != NoError) {
        return err;
    }
    GS::Array<API_Guid> elemList;
    err = ACAPI_Element_GetElemList (API_HotspotID, &elemList);
    if (err != NoError) {
        return err;
    }
    err = ACAPI_CallUndoableCommand ("Create text",
    [&]() -> GSErrCode {
        GSErrCode err = NoError;
        if (!elemList.IsEmpty ()) err = ACAPI_Element_Delete (elemList);
        if (err != NoError) {
            return err;
        }
        Point2D endC = GetWordPoint2DTM (sline.s.c1, sline.tm);
        elemline.hotspot.pos.x = endC.x;
        elemline.hotspot.pos.y = 0;
        err = ACAPI_Element_Create (&elemline, nullptr);
        if (err != NoError) {
            return err;
        }
        endC = GetWordPoint2DTM (sline.s.c2, sline.tm);
        elemline.hotspot.pos.x = endC.x;
        elemline.hotspot.pos.y = 0;
        err = ACAPI_Element_Create (&elemline, nullptr);
        return err;
    });
    if (err != NoError) {
        return err;
    }
    return err;
}

void ProfileByLine ()
{
    GSErrCode err = NoError;
    GS::Array<API_Guid> elems = GetSelectedElements2 (true, true);

    Point2D startpos;

    if (!ClickAPoint ("Click point", &startpos))
        return;

    // Получаем настройку хотспотов, которые будем расставлять на краях
    API_Element elemline;
    BNZeroMemory (&elemline, sizeof (API_Element));
    elemline.header.typeID = API_HotspotID;
    err = ACAPI_Element_GetDefaults (&elemline, nullptr);
    if (err != NoError) {
        return;
    }
    elemline.header.layer = 1;

    API_DatabaseInfo databasestart;
    BNZeroMemory (&databasestart, sizeof (API_DatabaseInfo));
    err = ACAPI_Database (APIDb_GetCurrentDatabaseID, &databasestart, nullptr);
    if (err != NoError) {
        return;
    }
    API_WindowInfo windowstart;
    BNZeroMemory (&windowstart, sizeof (API_WindowInfo));
    err = ACAPI_Database (APIDb_GetCurrentWindowID, &windowstart, nullptr);
    if (err != NoError) {
        return;
    }

    GS::Array<SSectLine> lines;
    GS::UniString id = "";
    GS::UniString name = "Участок ";
    if (GetSectLine (elems[0], lines, id, startpos) != NoError) {
        return;
    }
    for (UInt32 i = 0; i < lines.GetSize (); i++) {
        GS::UniString id_ = id + GS::UniString::Printf (".%d", i + 1);
        if (DoSect (lines[i], name, id_) != NoError) {
            return;
        }
    }
    for (UInt32 i = 0; i < lines.GetSize (); i++) {
        if (PlaceDocSect (lines[i], elemline) != NoError) {
            // Возвращение на исходную БД и окно
            err = ACAPI_Database (APIDb_ChangeCurrentDatabaseID, &databasestart, nullptr);
            if (err != NoError) {
                return;
            }
            err = ACAPI_Automate (APIDo_ChangeWindowID, &windowstart, nullptr);
            if (err != NoError) {
                return;
            }
        }
    }
    // Возвращение на исходную БД и окно
    err = ACAPI_Database (APIDb_ChangeCurrentDatabaseID, &databasestart, nullptr);
    if (err != NoError) {
        return;
    }
    err = ACAPI_Automate (APIDo_ChangeWindowID, &windowstart, nullptr);
    if (err != NoError) {
        return;
    }
}

void KM_ListUpdate ()
{
    GSErrCode err = NoError;
    GS::Array<API_Guid> elems = GetSelectedElements2 (true, true);
    for (UInt32 i = 0; i < elems.GetSize (); i++) {
        API_Element element, mask;
        BNZeroMemory (&element, sizeof (API_Element));
        element.header.guid = elems[i];
        err = ACAPI_Element_Get (&element);
        if (err != NoError) return;
        if (element.header.typeID == API_DrawingID) {
            API_Coord pos = { 0,0 };
            ACAPI_ELEMENT_MASK_CLEAR (mask);
            ACAPI_ELEMENT_MASK_SET (mask, API_DrawingType, pos);
            ACAPI_ELEMENT_MASK_SET (mask, API_DrawingType, useOwnOrigoAsAnchor);
            ACAPI_ELEMENT_MASK_SET (mask, API_DrawingType, isCutWithFrame);
            element.drawing.pos = { 0,0 };
            element.drawing.useOwnOrigoAsAnchor = true;
            element.drawing.isCutWithFrame = false;
            err = ACAPI_CallUndoableCommand ("Create text",
            [&]() -> GSErrCode {
                GSErrCode err = NoError;
                err = ACAPI_Element_Change (&element, &mask, nullptr, 0, true);
                return err;
            });
        }
    }
    return;
    //GS::Array<API_Guid> guidArray = GetSelectedElements (true, true, false);
    //GS::Array<API_Guid> elements;
    //GS::Array<API_Guid> lines;
    //for (UInt32 i = 0; i < guidArray.GetSize (); i++) {
    //    API_Elem_Head elem_head;
    //    elem_head.guid = guidArray[i];
    //    if (ACAPI_Element_GetHeader (&elem_head) == NoError) {
    //        API_ElemTypeID elementType = elem_head.typeID;
    //        if (elementType == API_ObjectID) elements.Push (guidArray[i]);
    //        if (elementType == API_PolyLineID || elementType == API_LineID) lines.Push (guidArray[i]);
    //    }
    //}
    //if (elements.IsEmpty () || lines.IsEmpty ()) return;
    //GS::Array<API_Coord> coords;
    //for (UInt32 i = 0; i < lines.GetSize (); i++) {
    //    API_Element element = {};
    //    BNZeroMemory (&element, sizeof (API_Element));
    //    element.header.guid = lines[i];
    //    if (ACAPI_Element_Get (&element) == NoError) {
    //        coords.Push (element.line.begC);
    //    }
    //}
    //ACAPI_CallUndoableCommand ("undoString", [&]() -> GSErrCode {
    //    GSErrCode err = KM_WriteGDLValues (elements.Get (0), coords);

    //    return err;
    //});
}

GSErrCode KM_WriteGDLValues (API_Guid elemGuid, GS::Array<API_Coord>& coords)
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
    apiOwner.typeID = elem_head.typeID;
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
            err = ACAPI_Goodies (APIAny_ChangeAParameterID, &chgParam, nullptr); if (err != NoError) return err;
        }
        if (name.IsEqual ("n_t")) {
            BNZeroMemory (&chgParam, sizeof (API_ChangeParamType));
            chgParam.index = (*apiParams.params)[i].index;
            CHTruncate ((*apiParams.params)[i].name, chgParam.name, API_NameLen);
            chgParam.realValue = n_t;
            err = ACAPI_Goodies (APIAny_ChangeAParameterID, &chgParam, nullptr); if (err != NoError) return err;
        }
    }
    err = ACAPI_Goodies (APIAny_GetActParametersID, &apiParams); if (err != NoError) return err;
    err = ACAPI_Goodies (APIAny_CloseParametersID, nullptr, nullptr); if (err != NoError) return err;

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
