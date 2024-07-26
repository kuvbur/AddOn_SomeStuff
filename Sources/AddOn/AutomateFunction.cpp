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

double angle (API_Coord3D& begC1, API_Coord3D& endC1, API_Coord3D& begC2, API_Coord3D& endC2)
{
    double x1 = endC1.x - begC1.x;
    double y1 = endC1.y - begC1.y;
    double x2 = endC2.x - begC2.x;
    double y2 = endC2.y - begC2.y;
    double t = (x1 * x2 + y1 * y2) / (sqrt (x1 * x1 + y1 * y1) * sqrt (x2 * x2 + y2 * y2));
    if (t < -1.0) t = -1.0;
    else if (t > 1.0) t = 1.0;
    return acos (t);
}

GSErrCode GetCuplane (const SSectLine sline, API_3DCutPlanesInfo& cutInfo)
{
    BNZeroMemory (&cutInfo, sizeof (API_3DCutPlanesInfo));
    GSErrCode err = ACAPI_Environment (APIEnv_Get3DCuttingPlanesID, &cutInfo, nullptr);
    if (err != NoError) return err;
    if (cutInfo.shapes != nullptr) BMKillHandle ((GSHandle*) &(cutInfo.shapes));
    cutInfo.isCutPlanes = true;
    cutInfo.useCustom = false;
    cutInfo.nShapes = 4;
    cutInfo.shapes = reinterpret_cast<API_3DCutShapeType**> (BMAllocateHandle (cutInfo.nShapes * sizeof (API_3DCutShapeType), ALLOCATE_CLEAR, 0));
    double pa = 0; double pb = 0; double pc = 0; double pd = 0;
    if (cutInfo.shapes != nullptr) {
        double co = cos (sline.angz);
        double si = sin (sline.angz);
        double sx = sline.begC.x * co + sline.begC.y * si;
        double sy = -sline.begC.x * si + sline.begC.y * co;
        double ex = sline.endC.x * co + sline.endC.y * si;
        double ey = -sline.endC.x * si + sline.endC.y * co;
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
        double an = sline.angz * RADDEG;
        double ann = sline.angz * RADDEG + 180;

        inx += 1;
        co = cos (sline.angz + 180 * DEGRAD);
        si = sin (sline.angz + 180 * DEGRAD);
        sx = sline.begC.x * co + sline.begC.y * si;
        sy = -sline.begC.x * si + sline.begC.y * co;
        ex = sline.endC.x * co + sline.endC.y * si;
        ey = -sline.endC.x * si + sline.endC.y * co;
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
            x = sline.begC.x * co + sline.begC.y * si;
        } else {
            x = -sline.begC.x * si + sline.begC.y * co;
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
            x = sline.endC.x * co + sline.endC.y * si;
        } else {
            x = -sline.endC.x * si + sline.endC.y * co;
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
    proj3DInfo.isPersp = err;
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
    return err;
}

GSErrCode Get3DDocument (API_DatabaseInfo& dbInfo, const GS::UniString& name, const GS::UniString& id)
{
    BNZeroMemory (&dbInfo, sizeof (API_DatabaseInfo));
    API_DatabaseUnId* dbases = NULL;
    GSErrCode err = NoError;
    API_WindowInfo windowInfo;
    windowInfo.typeID = APIWind_DocumentFrom3DID;
    err = ACAPI_Database (APIDb_GetDocumentFrom3DDatabasesID, &dbases, NULL);
    if (err != NoError) {
        if (dbases != nullptr) BMpFree (reinterpret_cast<GSPtr>(dbases));
        return err;
    }
    GSSize nDbases = BMpGetSize (reinterpret_cast<GSPtr>(dbases)) / Sizeof32 (API_DatabaseUnId);
    if (nDbases > 0) {
        API_DatabaseInfo dbPars = {};
        for (Int32 inx = 0; inx < nDbases; inx++) {
            dbInfo.databaseUnId = dbases[inx];
            err = ACAPI_Database (APIDb_GetDatabaseInfoID, &dbInfo, nullptr);
            if (err != NoError) {
                if (dbases != nullptr) BMpFree (reinterpret_cast<GSPtr>(dbases));
                return err;
            }
            GS::UniString n = GS::UniString (dbInfo.name);
            GS::UniString i = GS::UniString (dbInfo.ref);
            if (n == name && i == id) {
                if (dbases != nullptr) BMpFree (reinterpret_cast<GSPtr>(dbases));
                windowInfo.databaseUnId = dbInfo.databaseUnId;
                err = ACAPI_Automate (APIDo_ChangeWindowID, &windowInfo, nullptr);
                return err;
            }
        }
    }
    if (dbases != nullptr) BMpFree (reinterpret_cast<GSPtr>(dbases));
    dbInfo.typeID = APIWind_DocumentFrom3DID;
    GS::snuprintf (dbInfo.name, sizeof (dbInfo.name), name.ToUStr ().Get ());
    GS::snuprintf (dbInfo.ref, sizeof (dbInfo.ref), id.ToUStr ().Get ());
    err = ACAPI_Database (APIDb_NewDatabaseID, &dbInfo);
    if (err != NoError) {
        return err;
    }
    windowInfo.databaseUnId = dbInfo.databaseUnId;
    err = ACAPI_Automate (APIDo_ChangeWindowID, &windowInfo, nullptr);
    return err;
}

GSErrCode GetSectLine (API_Guid& elemguid, GS::Array<SSectLine>& lines, GS::UniString& id)
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
        Int32 edgeCnt = memo.morphBody->GetEdgeCount ();
        API_Tranmat tm = element.morph.tranmat;
        for (Int32 iEdge = 0; iEdge < edgeCnt; iEdge++) {
            const EDGE& edge = memo.morphBody->GetConstEdge (iEdge);
            const VERT& vtx1 = memo.morphBody->GetConstVertex (edge.vert1);
            const VERT& vtx2 = memo.morphBody->GetConstVertex (edge.vert2);
            API_Coord3D begC = GetWordCoordTM ({ vtx1.x, vtx1.y, vtx1.z }, tm); begC.z = 0;
            API_Coord3D endC = GetWordCoordTM ({ vtx2.x, vtx2.y, vtx2.z }, tm); endC.z = 0;
            double dx = endC.x - begC.x;
            double dy = endC.y - begC.y;
            double angz = 0.0;
            if (is_equal (dx, 0.0) && is_equal (dy, 0.0)) {
                angz = 0.0;
            } else {
                angz = atan2 (dy, dx) + PI;
            }
            SSectLine sline;
            sline.angz = angz;
            sline.begC = begC;
            sline.endC = endC;
            lines.Push (sline);
        }
        ACAPI_DisposeElemMemoHdls (&memo);
    } else {
        ACAPI_DisposeElemMemoHdls (&memo);
        return err;
    }
    for (UInt32 i = 0; i < lines.GetSize (); i++) {
        //if (i > 0) {
        //    lines[i].angz_1 = lines[i].angz + 0.5 * angle (lines[i].begC, lines[i].endC, lines[i - 1].begC, lines[i - 1].endC);
        //} else {
        //    lines[i].angz_1 = lines[i].angz - 0.5 * PI;
        //}
        //if (i < lines.GetSize () - 1) {
        //    lines[i].angz_2 = lines[i].angz - 0.5 * angle (lines[i + 1].begC, lines[i + 1].endC, lines[i].begC, lines[i].endC);
        //} else {
        //    lines[i].angz_2 = lines[i].angz + 0.5 * PI;
        //}
        lines[i].angz_1 = lines[i].angz - 0.5 * PI;
        lines[i].angz_2 = lines[i].angz + 0.5 * PI;
    }
    return err;
}



void ProfileByLine ()
{
    GSErrCode err = NoError;
    GS::Array<API_Guid> elems = GetSelectedElements2 (true, true);


    API_Element elemline;
    BNZeroMemory (&elemline, sizeof (API_Element));
    elemline.header.typeID = API_LineID;
    ACAPI_Element_GetDefaults (&elemline, nullptr);
    if (err != NoError) {
        return;
    }

    API_DatabaseInfo    databasestart;
    BNZeroMemory (&databasestart, sizeof (API_DatabaseInfo));
    err = ACAPI_Database (APIDb_GetCurrentDatabaseID, &databasestart, nullptr);
    if (err != NoError) {
        return;
    }
    GS::Array<SSectLine> lines;
    GS::UniString id = "";
    err = GetSectLine (elems[0], lines, id);
    if (err != NoError) {
        return;
    }
    for (UInt32 i = 0; i < lines.GetSize (); i++) {
        API_WindowInfo windowInfo;
        err = ACAPI_Database (APIDb_ChangeCurrentDatabaseID, &databasestart, nullptr);
        windowInfo.typeID = APIWind_FloorPlanID;
        windowInfo.databaseUnId = databasestart.databaseUnId;
        err = ACAPI_Automate (APIDo_ChangeWindowID, &windowInfo, nullptr);
        if (err != NoError) {
            return;
        }

        // Назначение секущих плоскостей по краям отрезка
        API_3DCutPlanesInfo cutInfo;
        double angz = 0;
        if (GetCuplane (lines[i], cutInfo)) {
            err = ACAPI_Environment (APIEnv_Change3DCuttingPlanesID, &cutInfo, nullptr);
            if (err != NoError) {
                BMKillHandle ((GSHandle*) &(cutInfo.shapes));
                return;
            }
        }

        // Установка камеры перпендикулярно отрезку
        API_3DProjectionInfo  proj3DInfo;
        if (Get3DProjectionInfo (proj3DInfo, lines[i].angz) != NoError) {
            BMKillHandle ((GSHandle*) &(cutInfo.shapes));
            return;
        }

        // Создание 3д документа
        API_DatabaseInfo dbInfo = {};
        GS::UniString name = GS::UniString::Printf ("Участок %d", i + 1);
        if (Get3DDocument (dbInfo, name, id + GS::UniString::Printf (".%d", i + 1))) {
            BMKillHandle ((GSHandle*) &(cutInfo.shapes));
            return;
        }
        windowInfo.typeID = APIWind_DocumentFrom3DID;
        windowInfo.databaseUnId = dbInfo.databaseUnId;
        err = ACAPI_Automate (APIDo_ChangeWindowID, &windowInfo, nullptr);
        if (err != NoError) {
            BMKillHandle ((GSHandle*) &(cutInfo.shapes));
            return;
        }
        err = ACAPI_Database (APIDb_ChangeCurrentDatabaseID, &dbInfo, nullptr);
        if (err != NoError) {
            BMKillHandle ((GSHandle*) &(cutInfo.shapes));
            return;
        }
        // Обновление 3д документа
        API_DocumentFrom3DType documentFrom3DType;
        err = ACAPI_Environment (APIEnv_GetDocumentFrom3DSettingsID, &dbInfo.databaseUnId, &documentFrom3DType);
        if (err != NoError) {
            BMKillHandle (reinterpret_cast<GSHandle*> (&documentFrom3DType.cutSetting.shapes));
            BMKillHandle ((GSHandle*) &(cutInfo.shapes));
            return;
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
        if (err != NoError) {
            BMKillHandle (reinterpret_cast<GSHandle*> (&documentFrom3DType.cutSetting.shapes));
            BMKillHandle ((GSHandle*) &(cutInfo.shapes));
            return;
        }

        err = ACAPI_CallUndoableCommand ("Create text",
        [&]() -> GSErrCode {
            API_Coord3D endC = GetWordCoordTM (lines[i].begC, proj3DInfo.u.axono.tranmat);
            elemline.line.begC.x = endC.x;
            elemline.line.begC.y = 0;
            elemline.line.endC.x = endC.x;
            elemline.line.endC.y = 1;
            GSErrCode err = ACAPI_Element_Create (&elemline, nullptr);
            endC = GetWordCoordTM (lines[i].endC, proj3DInfo.u.axono.tranmat);
            elemline.line.begC.x = endC.x;
            elemline.line.begC.y = 0;
            elemline.line.endC.x = endC.x;
            elemline.line.endC.y = 1;
            err = ACAPI_Element_Create (&elemline, nullptr);
            return err;
        });
        BMKillHandle (reinterpret_cast<GSHandle*> (&documentFrom3DType.cutSetting.shapes));
        BMKillHandle ((GSHandle*) &(cutInfo.shapes));
        if (err != NoError) {
            return;
        }
    }
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
            API_ElemTypeID elementType = elem_head.typeID;
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
        GSErrCode err = KM_WriteGDLValues (elements.Get (0), coords);

        return err;
    });
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
