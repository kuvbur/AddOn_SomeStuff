//------------ kuvbur 2022 ------------
#ifdef PK_1
#include	"ACAPinc.h"
#include	"APIEnvir.h"
#include	"AutomateFunction.hpp"
#include	"Helpers.hpp"

namespace AutoFunc
{
void ProfileByLine ()
{
    GSErrCode err = NoError;
    GS::Array<API_Guid> elems = GetSelectedElements2 (true, true);
    API_Element element;
    BNZeroMemory (&element, sizeof (API_Element));
    element.header.guid = elems[0];
    err = ACAPI_Element_Get (&element);
    double angz = 0; double angzr = 0;
    double dx = element.line.endC.x - element.line.begC.x;
    double dy = element.line.endC.y - element.line.begC.y;
    if (is_equal (dx, 0.0) && is_equal (dy, 0.0)) {
        angzr = 0.0;
        angz = 0.0;
    } else {
        angzr = atan2 (dy, dx) + PI;
        angz = (angzr * 180.0 / PI);
    }
    double sx = element.line.begC.x * cos (angzr) - element.line.begC.y * sin (angzr);
    double sy = element.line.begC.x * sin (angzr) + element.line.begC.y * cos (angzr);
    double ex = element.line.endC.x * cos (angzr) - element.line.endC.y * sin (angzr);
    double ey = element.line.endC.x * sin (angzr) + element.line.endC.y * cos (angzr);
    double x = 0;
    dx = fabs (sx) - fabs (ex);
    dy = fabs (sy) - fabs (ey);
    if (fabs (dx) < 0.00001 && fabs (sx) > std::numeric_limits<double>::epsilon ()) x = sx;
    if (fabs (dy) < 0.00001 && fabs (sy) > std::numeric_limits<double>::epsilon ()) x = sy;
    double pa = 0;
    double pb = 0;
    double pc = 0;
    double pd = 0;
    API_3DCutPlanesInfo cutInfo;
    BNZeroMemory (&cutInfo, sizeof (API_3DCutPlanesInfo));
    err = ACAPI_Environment (APIEnv_Get3DCuttingPlanesID, &cutInfo, nullptr);
    if (err == NoError) {
        if (cutInfo.shapes != nullptr) BMKillHandle ((GSHandle*) &(cutInfo.shapes));
        cutInfo.isCutPlanes = true;
        cutInfo.useCustom = false;
        cutInfo.nShapes = 1;
        cutInfo.shapes = reinterpret_cast<API_3DCutShapeType**> (BMAllocateHandle (cutInfo.nShapes * sizeof (API_3DCutShapeType), ALLOCATE_CLEAR, 0));
        pa = cos (angzr);
        pb = sin (angzr);
        pc = 0;
        pd = x;
        if (cutInfo.shapes != nullptr) {
            (*cutInfo.shapes)[0].cutStatus = 0;
            (*cutInfo.shapes)[0].cutPen = 3;
            (*cutInfo.shapes)[0].cutMater = 11;
            (*cutInfo.shapes)[0].pa = pa;
            (*cutInfo.shapes)[0].pb = pb;
            (*cutInfo.shapes)[0].pc = pc;
            (*cutInfo.shapes)[0].pd = pd;
        }
        err = ACAPI_Environment (APIEnv_Change3DCuttingPlanesID, &cutInfo, nullptr);
    }
    API_3DProjectionInfo  proj3DInfo;
    BNZeroMemory (&proj3DInfo, sizeof (API_3DProjectionInfo));
    err = ACAPI_Environment (APIEnv_Get3DProjectionSetsID, &proj3DInfo, nullptr, nullptr);
    if (err != NoError) return;
    proj3DInfo.isPersp = false;
    proj3DInfo.u.axono.azimuth = angz + 90;
    proj3DInfo.u.axono.projMod = 1;
    err = ACAPI_Environment (APIEnv_Change3DProjectionSetsID, &proj3DInfo, nullptr, nullptr);
    if (err != NoError) return;
    API_DatabaseInfo dbInfo = {};
    dbInfo.typeID = APIWind_DocumentFrom3DID;
    GS::snuprintf (dbInfo.name, sizeof (dbInfo.name), "Document3D");
    GS::snuprintf (dbInfo.ref, sizeof (dbInfo.ref), "3D");
    err = ACAPI_Database (APIDb_NewDatabaseID, &dbInfo);
    if (err != NoError) return;
    API_DocumentFrom3DType documentFrom3DType;
    err = ACAPI_Environment (APIEnv_GetDocumentFrom3DSettingsID, &dbInfo.databaseUnId, &documentFrom3DType);
    if (err != NoError) {
        BMKillHandle (reinterpret_cast<GSHandle*> (&documentFrom3DType.cutSetting.shapes));
        BMKillHandle ((GSHandle*) &(cutInfo.shapes));
        return;
    }
    documentFrom3DType.cutSetting.isCutPlanes = cutInfo.isCutPlanes;
    documentFrom3DType.cutSetting.useCustom = cutInfo.useCustom;
    if (documentFrom3DType.cutSetting.shapes != nullptr) BMKillHandle ((GSHandle*) &(documentFrom3DType.cutSetting.shapes));
    documentFrom3DType.cutSetting.shapes = reinterpret_cast<API_3DCutShapeType**> (BMAllocateHandle (cutInfo.nShapes * sizeof (API_3DCutShapeType), ALLOCATE_CLEAR, 0));
    if (documentFrom3DType.cutSetting.shapes != nullptr) {
        (*documentFrom3DType.cutSetting.shapes)[0].cutStatus = 0;
        (*documentFrom3DType.cutSetting.shapes)[0].cutPen = 3;
        (*documentFrom3DType.cutSetting.shapes)[0].cutMater = 11;
        (*documentFrom3DType.cutSetting.shapes)[0].pa = pa;
        (*documentFrom3DType.cutSetting.shapes)[0].pb = pb;
        (*documentFrom3DType.cutSetting.shapes)[0].pc = pc;
        (*documentFrom3DType.cutSetting.shapes)[0].pd = pd;
    }
    documentFrom3DType.projectionSetting = proj3DInfo;
    err = ACAPI_Environment (APIEnv_ChangeDocumentFrom3DSettingsID, &dbInfo.databaseUnId, &documentFrom3DType);
    BMKillHandle (reinterpret_cast<GSHandle*> (&documentFrom3DType.cutSetting.shapes));
    BMKillHandle ((GSHandle*) &(cutInfo.shapes));
    if (err != NoError) return;
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
