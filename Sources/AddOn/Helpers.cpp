//------------ kuvbur 2022 ------------
#include    "ACAPinc.h"
#include    "APIEnvir.h"
#include    <cmath>
#include    <limits>
#include    <math.h>
#if defined(AC_27)|| defined (AC_28)
#include	"MEPv1.hpp"
#endif // AC_27
#ifdef TESTING
#include "TestFunc.hpp"
#endif
#include    "Helpers.hpp"
#include    "Model3D/model.h"
#include    "Model3D/MeshBody.hpp"
#include    "VectorImageIterator.hpp"
#include    "ProfileVectorImageOperations.hpp"
#include    "ProfileAdditionalInfo.hpp"

int IsDummyModeOn ()
{
    //    GS::Array<GS::ArrayFB<GS::UniString, 3> >	autotexts;
    //    API_AutotextType	type = APIAutoText_Custom;
    //    GSErrCode	err = NoError;
    //#if defined(AC_27) || defined(AC_28)
    //    err = ACAPI_AutoText_GetAutoTexts (&autotexts, type);
    //#else
    //    err = ACAPI_Goodies (APIAny_GetAutoTextsID, &autotexts, (void*) (GS::IntPtr) type);
    //#endif
    //    if (err != NoError) return false;
    //    for (UInt32 i = 0; i < autotexts.GetSize (); i++) {
    //        if (autotexts[i][0].ToLowerCase ().Contains ("somestuff_dummymode")) {
    //            if (autotexts[i][2].ToLowerCase ().Contains ("on")) {
    //                return DUMMY_MODE_ON;
    //            } else {
    //                return DUMMY_MODE_OFF;
    //            }
    //        }
    //    }
    return DUMMY_MODE_OFF;
}

// -----------------------------------------------------------------------------
// Добавление отслеживания (для разных версий)
// -----------------------------------------------------------------------------
GSErrCode	AttachObserver (const API_Guid& objectId, const SyncSettings& syncSettings)
{
    GSErrCode err = NoError;
    if (IsElementEditable (objectId, syncSettings, false)) {
        #ifdef AC_22
        API_Elem_Head elemHead;
        elemHead.guid = objectId;
        err = ACAPI_Element_AttachObserver (&elemHead, 0);
        #else
        err = ACAPI_Element_AttachObserver (objectId);
        #endif
    }
    return err;
}

// --------------------------------------------------------------------
// Проверяет - попадает ли тип элемента в под настройки синхронизации
// --------------------------------------------------------------------
bool CheckElementType (const API_ElemTypeID& elementType, const SyncSettings& syncSettings)
{
    if (elementType == API_GroupID)
        return false;
    if (elementType == API_DimensionID)
        return true;
    if (syncSettings.wallS &&
       (elementType == API_WallID || elementType == API_ColumnID || elementType == API_BeamID || elementType == API_SlabID ||
           elementType == API_RoofID || elementType == API_MeshID || elementType == API_ShellID ||
           elementType == API_MorphID ||
           elementType == API_BeamSegmentID ||
           elementType == API_ColumnSegmentID))
        return true;
    if (syncSettings.objS &&
       (elementType == API_StairID || elementType == API_RiserID ||
           elementType == API_TreadID || elementType == API_StairStructureID ||
           elementType == API_ObjectID ||
           elementType == API_ZoneID ||
           elementType == API_LampID))
        return true;
    if (syncSettings.cwallS &&
       (elementType == API_RailingID || elementType == API_RailingToprailID || elementType == API_RailingHandrailID ||
           elementType == API_RailingRailID || elementType == API_RailingPostID || elementType == API_RailingInnerPostID ||
           elementType == API_RailingBalusterID || elementType == API_RailingPanelID || elementType == API_RailingSegmentID ||
           elementType == API_RailingNodeID || elementType == API_RailingBalusterSetID || elementType == API_RailingPatternID ||
           elementType == API_RailingToprailEndID || elementType == API_RailingHandrailEndID ||
           elementType == API_RailingRailEndID ||
           elementType == API_RailingToprailConnectionID ||
           elementType == API_RailingHandrailConnectionID ||
           elementType == API_RailingRailConnectionID ||
           elementType == API_RailingEndFinishID))
        return true;
    #if defined(AC_27) || defined(AC_28)
    if (syncSettings.objS && elementType == API_ExternalElemID) return true;
    #endif

    if (syncSettings.cwallS &&
       (elementType == API_CurtainWallSegmentID ||
           elementType == API_CurtainWallFrameID ||
           elementType == API_CurtainWallJunctionID ||
           elementType == API_CurtainWallAccessoryID ||
           elementType == API_CurtainWallID ||
           elementType == API_CurtainWallPanelID))
        return true;
    if (syncSettings.widoS &&
       (elementType == API_WindowID ||
           elementType == API_DoorID ||
           elementType == API_SkylightID ||
           elementType == API_OpeningID))
        return true;
    return false;
}

// -----------------------------------------------------------------------------
// Проверяет возможность редактирования объекта (не находится в модуле, разблокирован, зарезервирован)
// -----------------------------------------------------------------------------
bool IsElementEditable (const API_Guid& objectId, const SyncSettings& syncSettings, const bool needCheckElementType)
{
    API_ElemTypeID eltype;
    bool res = IsElementEditable (objectId, syncSettings, needCheckElementType, eltype);
    UNUSED_VARIABLE (eltype);
    return res;
}

bool IsElementEditable (const API_Elem_Head& tElemHead, const SyncSettings& syncSettings, const bool needCheckElementType)
{
    if (tElemHead.guid == APINULLGuid) return false;
    // Проверяем - на находится ли объект в модуле
    if (tElemHead.hotlinkGuid != APINULLGuid) return false;
    API_ElemTypeID eltype = GetElemTypeID (tElemHead);
    if (needCheckElementType && !CheckElementType (eltype, syncSettings)) return false;
    // Проверяем - зарезервирован ли объект
    if (!ACAPI_Element_Filter (tElemHead.guid, APIFilt_InMyWorkspace)) return false;
    if (!ACAPI_Element_Filter (tElemHead.guid, APIFilt_HasAccessRight)) return false;
    if (!ACAPI_Element_Filter (tElemHead.guid, APIFilt_IsEditable)) return false;
    return true;
}

// -----------------------------------------------------------------------------
// Проверяет возможность редактирования объекта (не находится в модуле, разблокирован, зарезервирован)
// Возвращает тип элемента
// -----------------------------------------------------------------------------
bool IsElementEditable (const API_Guid& objectId, const SyncSettings& syncSettings, const bool needCheckElementType, API_ElemTypeID& eltype)
{
    // Проверяем - зарезервирован ли объект
    if (objectId == APINULLGuid) return false;
    if (!ACAPI_Element_Filter (objectId, APIFilt_InMyWorkspace)) return false;
    if (!ACAPI_Element_Filter (objectId, APIFilt_HasAccessRight)) return false;
    if (!ACAPI_Element_Filter (objectId, APIFilt_IsEditable)) return false;
    // Проверяем - на находится ли объект в модуле
    API_Elem_Head	tElemHead = {};
    BNZeroMemory (&tElemHead, sizeof (API_Elem_Head));
    tElemHead.guid = objectId;
    if (ACAPI_Element_GetHeader (&tElemHead) != NoError) return false;
    if (tElemHead.hotlinkGuid != APINULLGuid) return false;
    eltype = GetElemTypeID (tElemHead);
    if (needCheckElementType && !CheckElementType (eltype, syncSettings)) return false;
    return true;
}

// -----------------------------------------------------------------------------
// Получить массив Guid выбранных элементов
// Настройки будут считаны при вызове функции
// -----------------------------------------------------------------------------
GS::Array<API_Guid>	GetSelectedElements (bool assertIfNoSel /* = true*/, bool onlyEditable /*= true*/, bool addSubelement /*= true*/)
{
    SyncSettings syncSettings (false, false, true, true, true, true, false);
    LoadSyncSettingsFromPreferences (syncSettings);
    return GetSelectedElements (assertIfNoSel, onlyEditable, syncSettings, addSubelement);
}

// -----------------------------------------------------------------------------
// Получить массив Guid выбранных элементов в соответсвии с настройками обработки
// -----------------------------------------------------------------------------
GS::Array<API_Guid>	GetSelectedElements (bool assertIfNoSel /* = true*/, bool onlyEditable /*= true*/, const SyncSettings& syncSettings, bool addSubelement)
{
    GSErrCode err;
    API_SelectionInfo selectionInfo = {};
    GS::UniString errorString = RSGetIndString (ID_ADDON_STRINGS + isEng (), ErrorSelectID, ACAPI_GetOwnResModule ());
    #ifdef AC_22
    API_Neig** selNeigs;
    #else
    GS::Array<API_Neig> selNeigs = {};
    #endif
    err = ACAPI_Selection_Get (&selectionInfo, &selNeigs, onlyEditable);
    BMKillHandle ((GSHandle*) &selectionInfo.marquee.coords);
    if (err == APIERR_NOSEL || selectionInfo.typeID == API_SelEmpty) {
        if (assertIfNoSel) {
            DGAlert (DG_ERROR, "Error", errorString, "", "Ok");
        }
    }
    if (err != NoError) {
        #ifdef AC_22
        BMKillHandle ((GSHandle*) &selNeigs);
        #endif // AC_22
        return GS::Array<API_Guid> ();
    }
    GS::Array<API_Guid> guidArray = {};
    #ifdef AC_22
    USize nSel = BMGetHandleSize ((GSHandle) selNeigs) / sizeof (API_Neig);
    for (USize i = 0; i < nSel; i++) {
        guidArray.Push ((*selNeigs)[i].guid);
    }
    BMKillHandle ((GSHandle*) &selNeigs);
    #else
    for (const API_Neig& neig : selNeigs) {
        API_Guid elemguid = neig.guid;
        guidArray.Push (elemguid);
        if (addSubelement) {
            API_ElemTypeID elementType;
            API_NeigID neigID = neig.neigID;
            GSErrCode err = NoError;
            #if defined AC_26 || defined AC_27 || defined AC_28
            API_ElemType elemType26;
            #if defined(AC_27) || defined(AC_28)
            err = ACAPI_Element_NeigIDToElemType (neigID, elemType26);
            #else
            err = ACAPI_Goodies_NeigIDToElemType (neigID, elemType26);
            #endif
            elementType = elemType26.typeID;
            #else
            err = ACAPI_Goodies (APIAny_NeigIDToElemTypeID, &neigID, &elementType);
            #endif // AC_26
            #if defined(AC_27) || defined(AC_28)
            if (err != NoError && neig.guid != APINULLGuid) { // На МЕР элементах функция ACAPI_Element_NeigIDToElemType не работает(
                err = GetTypeByGUID (neig.guid, elementType);
            }
            #endif // AC_27
            if (err == NoError) GetRelationsElement (elemguid, elementType, syncSettings, guidArray);
        }
    }
    #endif // AC_22
    return guidArray;

}

// -----------------------------------------------------------------------------
// Возвращает GUID родительского элемента для API_SectElemType
// -----------------------------------------------------------------------------
void GetParentGUIDSectElem (const API_Guid& sectElemguid, API_Guid& parentguid, API_ElemTypeID& parentType)
{
    API_Element elem = {};
    elem.header.guid = sectElemguid;
    GSErrCode err = ACAPI_Element_Get (&elem);
    if (err != NoError || elem.sectElem.parentGuid == APINULLGuid) {
        msg_rep ("GetParentGUIDSectElem", "ACAPI_Element_Get", err, sectElemguid);
    } else {
        parentguid = elem.sectElem.parentGuid;
        #if defined AC_26 || defined AC_27 || defined AC_28
        parentType = elem.sectElem.parentType.typeID;
        #else
        parentType = elem.sectElem.parentID;
        #endif
    }
}

// -----------------------------------------------------------------------------
// Вызов функции для выбранных элементов
//	(функция должна принимать в качетве аргумента API_Guid SyncSettings
// -----------------------------------------------------------------------------
void CallOnSelectedElemSettings (void (*function)(const API_Guid&, const SyncSettings&), bool assertIfNoSel /* = true*/, bool onlyEditable /* = true*/, const SyncSettings& syncSettings, GS::UniString& funcname, bool addSubelement)
{
    GS::Array<API_Guid> guidArray = GetSelectedElements (assertIfNoSel, onlyEditable, addSubelement);
    if (!guidArray.IsEmpty ()) {
        GS::UniString subtitle ("working...");
        GS::Int32 nPhase = 1;
        #if defined(AC_27) || defined(AC_28)
        bool showPercent = true;
        Int32 maxval = guidArray.GetSize ();
        ACAPI_ProcessWindow_InitProcessWindow (&funcname, &nPhase);
        #else
        ACAPI_Interface (APIIo_InitProcessWindowID, &funcname, &nPhase);
        #endif
        long time_start = clock ();
        for (UInt32 i = 0; i < guidArray.GetSize (); i++) {
            function (guidArray[i], syncSettings);
            #if defined(AC_27) || defined(AC_28)
            if (i % 10 == 0) ACAPI_ProcessWindow_SetNextProcessPhase (&subtitle, &maxval, &showPercent);
            #else
            if (i % 10 == 0) ACAPI_Interface (APIIo_SetNextProcessPhaseID, &subtitle, &i);
            #endif
            #if defined(AC_27) || defined(AC_28)
            if (ACAPI_ProcessWindow_IsProcessCanceled ()) return;
            #else
            if (ACAPI_Interface (APIIo_IsProcessCanceledID, nullptr, nullptr)) return;
            #endif
        }
        long time_end = clock ();
        GS::UniString time = GS::UniString::Printf (" %.3f s", (time_end - time_start) / 1000);
        GS::UniString intString = GS::UniString::Printf (" %d qty", guidArray.GetSize ());
        msg_rep (funcname + " Selected", intString + time, NoError, APINULLGuid);
        #if defined(AC_27) || defined(AC_28)
        ACAPI_ProcessWindow_CloseProcessWindow ();
        #else
        ACAPI_Interface (APIIo_CloseProcessWindowID, nullptr, nullptr);
        #endif
    }
}

// -----------------------------------------------------------------------------
// Вызов функции для выбранных элементов
//	(функция должна принимать в качетве аргумента API_Guid
// -----------------------------------------------------------------------------
void CallOnSelectedElem (void (*function)(const API_Guid&), bool assertIfNoSel /* = true*/, bool onlyEditable /* = true*/, GS::UniString& funcname, bool addSubelement)
{
    GS::Array<API_Guid> guidArray = GetSelectedElements (assertIfNoSel, onlyEditable, addSubelement);
    if (!guidArray.IsEmpty ()) {
        long time_start = clock ();
        GS::UniString subtitle ("working...");
        GS::Int32 nPhase = 1;
        #if defined(AC_27) || defined(AC_28)
        bool showPercent = true;
        Int32 maxval = guidArray.GetSize ();
        ACAPI_ProcessWindow_InitProcessWindow (&funcname, &nPhase);
        #else
        ACAPI_Interface (APIIo_InitProcessWindowID, &funcname, &nPhase);
        #endif
        for (UInt32 i = 0; i < guidArray.GetSize (); i++) {
            #if defined(AC_27) || defined(AC_28)
            if (i % 10 == 0) ACAPI_ProcessWindow_SetNextProcessPhase (&subtitle, &maxval, &showPercent);
            #else
            if (i % 10 == 0) ACAPI_Interface (APIIo_SetNextProcessPhaseID, &subtitle, &i);
            #endif
            function (guidArray[i]);
            #if defined(AC_27) || defined(AC_28)
            if (ACAPI_ProcessWindow_IsProcessCanceled ()) return;
            #else
            if (ACAPI_Interface (APIIo_IsProcessCanceledID, nullptr, nullptr)) return;
            #endif
        }
        long time_end = clock ();
        GS::UniString time = GS::UniString::Printf (" %d ms", (time_end - time_start) / 1000);
        GS::UniString intString = GS::UniString::Printf (" %d qty", guidArray.GetSize ());
        msg_rep (funcname + " Selected", intString + time, NoError, APINULLGuid);
        #if defined(AC_27) || defined(AC_28)
        ACAPI_ProcessWindow_CloseProcessWindow ();
        #else
        ACAPI_Interface (APIIo_CloseProcessWindowID, nullptr, nullptr);
        #endif
    } else if (!assertIfNoSel) {
        function (APINULLGuid);
    }
}

// --------------------------------------------------------------------
// Поиск связанных элементов
// --------------------------------------------------------------------
void GetRelationsElement (const API_Guid& elemGuid, const SyncSettings& syncSettings, GS::Array<API_Guid>& subelemGuid)
{
    API_ElemTypeID elementType;
    if (GetTypeByGUID (elemGuid, elementType) != NoError) return;
    GetRelationsElement (elemGuid, elementType, syncSettings, subelemGuid);
}

// --------------------------------------------------------------------
// Поиск связанных элементов для определённого типа
// --------------------------------------------------------------------
void GetRelationsElement (const API_Guid& elemGuid, const  API_ElemTypeID& elementType, const SyncSettings& syncSettings, GS::Array<API_Guid>& subelemGuid)
{
    GSErrCode	err = NoError;
    API_RoomRelation	relData;
    GS::Array<API_ElemTypeID> typeinzone = {};
    API_Guid ownerElemApiGuid = APINULLGuid;
    API_Guid ownerElemApiGuid_root = APINULLGuid;
    API_Guid elemGuid_t = elemGuid;

    #ifndef AC_22
    API_HierarchicalOwnerType hierarchicalOwnerType = API_ParentHierarchicalOwner;
    API_HierarchicalElemType hierarchicalElemType = API_SingleElem;
    API_HierarchicalElemType hierarchicalElemType_root = API_SingleElem;
    #if defined(AC_27) || defined(AC_28)
    if (syncSettings.objS && elementType == API_ExternalElemID) {
        MEPv1::GetSubElement (elemGuid, subelemGuid);
        ACAPI_DisposeRoomRelationHdls (&relData);
        return;
    }
    err = ACAPI_HierarchicalEditing_GetHierarchicalElementOwner (&elemGuid_t, &hierarchicalOwnerType, &hierarchicalElemType, &ownerElemApiGuid);
    #else
    err = ACAPI_Goodies (APIAny_GetHierarchicalElementOwnerID, &elemGuid_t, &hierarchicalOwnerType, &hierarchicalElemType, &ownerElemApiGuid);
    #endif
    hierarchicalOwnerType = API_RootHierarchicalOwner;
    #if defined(AC_27) || defined(AC_28)
    err = ACAPI_HierarchicalEditing_GetHierarchicalElementOwner (&elemGuid_t, &hierarchicalOwnerType, &hierarchicalElemType, &ownerElemApiGuid_root);
    #else
    err = ACAPI_Goodies (APIAny_GetHierarchicalElementOwnerID, &elemGuid_t, &hierarchicalOwnerType, &hierarchicalElemType, &ownerElemApiGuid_root);
    #endif
    #endif
    API_Element element = {}; BNZeroMemory (&element, sizeof (API_Element));
    API_ElementMemo memo = {}; BNZeroMemory (&memo, sizeof (API_ElementMemo));
    switch (elementType) {
        #ifndef AC_22
        case API_ColumnID:
            if (syncSettings.cwallS) {
                element.header.guid = elemGuid;
                err = ACAPI_Element_Get (&element);
                if (err != NoError) {
                    msg_rep ("GetRelationsElement", "ACAPI_Element_Get", err, elemGuid);
                    return;
                }
                if (element.column.nSegments == 0) return;
                err = ACAPI_Element_GetMemo (element.header.guid, &memo, APIMemoMask_ColumnSegment);
                if (err == NoError && memo.columnSegments != nullptr) {
                    for (UInt32 i = 0; i < element.column.nSegments; i++) {
                        subelemGuid.Push (memo.columnSegments[0].head.guid);
                    }
                }
                ACAPI_DisposeElemMemoHdls (&memo);
            }
            break;
        case API_BeamID:
            if (syncSettings.cwallS) {
                element.header.guid = elemGuid;
                err = ACAPI_Element_Get (&element);
                if (err != NoError) {
                    msg_rep ("GetRelationsElement", "ACAPI_Element_Get", err, elemGuid);
                    return;
                }
                if (element.beam.nSegments == 0) return;
                err = ACAPI_Element_GetMemo (element.header.guid, &memo, APIMemoMask_BeamSegment);
                if (err == NoError && memo.beamSegments != nullptr) {
                    for (UInt32 i = 0; i < element.beam.nSegments; i++) {
                        subelemGuid.Push (memo.beamSegments[0].head.guid);
                    }
                }
                ACAPI_DisposeElemMemoHdls (&memo);
            }
            break;
            #endif
        case API_WallID:
            if (syncSettings.widoS) {
                GS::Array<API_Guid> windows = {};
                #if defined(AC_27) || defined(AC_28)
                err = ACAPI_Grouping_GetConnectedElements (elemGuid, API_WindowID, &windows, APIFilt_None, APINULLGuid);
                #else
                err = ACAPI_Element_GetConnectedElements (elemGuid, API_WindowID, &windows);
                #endif
                if (!windows.IsEmpty ()) {
                    for (UInt32 i = 0; i < windows.GetSize (); i++) {
                        subelemGuid.Push (windows[i]);
                    }
                }
                GS::Array<API_Guid> doors = {};
                #if defined(AC_27) || defined(AC_28)
                err = ACAPI_Grouping_GetConnectedElements (elemGuid, API_DoorID, &doors, APIFilt_None, APINULLGuid);
                #else
                err = ACAPI_Element_GetConnectedElements (elemGuid, API_DoorID, &doors);
                #endif
                if (!doors.IsEmpty ()) {
                    for (UInt32 i = 0; i < doors.GetSize (); i++) {
                        subelemGuid.Push (doors[i]);
                    }
                }
                break;
            }
        case API_RailingID:
            if (syncSettings.cwallS) {
                err = GetRElementsForRailing (elemGuid, subelemGuid);
            }
            break;
        case API_CurtainWallID:
            if (syncSettings.cwallS) {
                err = GetRElementsForCWall (elemGuid, subelemGuid);
            }
            break;
        case API_CurtainWallSegmentID:
        case API_CurtainWallFrameID:
        case API_CurtainWallJunctionID:
        case API_CurtainWallAccessoryID:
        case API_CurtainWallPanelID:
            if (syncSettings.cwallS) {
                API_CWPanelRelation crelData = {};
                err = ACAPI_Element_GetRelations (elemGuid, API_ZoneID, &crelData);
                if (err == NoError) {
                    if (crelData.fromRoom != APINULLGuid) subelemGuid.Push (crelData.fromRoom);
                    if (crelData.toRoom != APINULLGuid) subelemGuid.Push (crelData.toRoom);
                }
                #ifndef AC_22
                if (ownerElemApiGuid != APINULLGuid && hierarchicalElemType == API_ChildElemInMultipleElem) subelemGuid.Push (ownerElemApiGuid);
                if (ownerElemApiGuid_root != ownerElemApiGuid && ownerElemApiGuid_root != APINULLGuid && hierarchicalElemType_root == API_ChildElemInMultipleElem) subelemGuid.Push (ownerElemApiGuid_root);
                #endif
            }
            break;
        case API_DoorID:
            if (syncSettings.widoS) {
                API_DoorRelation drelData = {};
                err = ACAPI_Element_GetRelations (elemGuid, API_ZoneID, &drelData);
                if (err == NoError) {
                    if (drelData.fromRoom != APINULLGuid) subelemGuid.Push (drelData.fromRoom);
                    if (drelData.toRoom != APINULLGuid) subelemGuid.Push (drelData.toRoom);
                }
                #ifndef AC_22
                if (ownerElemApiGuid != APINULLGuid && hierarchicalElemType == API_ChildElemInMultipleElem) subelemGuid.Push (ownerElemApiGuid);
                if (ownerElemApiGuid_root != ownerElemApiGuid && ownerElemApiGuid_root != APINULLGuid && hierarchicalElemType_root == API_ChildElemInMultipleElem) subelemGuid.Push (ownerElemApiGuid_root);
                #endif
            }
            break;
        case API_WindowID:
            if (syncSettings.widoS) {
                API_WindowRelation wrelData = {};
                err = ACAPI_Element_GetRelations (elemGuid, API_ZoneID, &wrelData);
                if (err == NoError) {
                    if (wrelData.fromRoom != APINULLGuid) subelemGuid.Push (wrelData.fromRoom);
                    if (wrelData.toRoom != APINULLGuid) subelemGuid.Push (wrelData.toRoom);
                }
                #ifndef AC_22
                if (ownerElemApiGuid != APINULLGuid && hierarchicalElemType == API_ChildElemInMultipleElem) subelemGuid.Push (ownerElemApiGuid);
                if (ownerElemApiGuid_root != ownerElemApiGuid && ownerElemApiGuid_root != APINULLGuid && hierarchicalElemType_root == API_ChildElemInMultipleElem) subelemGuid.Push (ownerElemApiGuid_root);
                #endif
            }
            break;
        case API_ZoneID:
            if (syncSettings.objS) {
                err = ACAPI_Element_GetRelations (elemGuid, API_ZombieElemID, &relData);
                if (err == NoError) {
                    #if defined(AC_23) || defined(AC_22)
                    for (Int32 i = 0; i < relData.nObject; i++) {
                        API_Guid elGuid = (*relData.objects)[i];
                        subelemGuid.Push (elGuid);
                    }
                    if (syncSettings.widoS) {
                        for (Int32 i = 0; i < relData.nWindow; i++) {
                            API_Guid elGuid = (*relData.windows)[i];
                            subelemGuid.Push (elGuid);
                        }
                        for (Int32 i = 0; i < relData.nDoor; i++) {
                            API_Guid elGuid = (*relData.doors)[i];
                            subelemGuid.Push (elGuid);
                        }
                        for (Int32 i = 0; i < relData.nSkylight; i++) {
                            API_Guid elGuid = (*relData.skylights)[i];
                            subelemGuid.Push (elGuid);
                        }
                    }
                    if (syncSettings.wallS) {
                        for (Int32 i = 0; i < relData.nColumn; i++) {
                            API_Guid elGuid = (*relData.columns)[i];
                            subelemGuid.Push (elGuid);
                        }
                        for (Int32 i = 0; i < relData.nWallPart; i++) {
                            API_Guid elGuid = (*relData.wallPart)[i].guid;
                            subelemGuid.Push (elGuid);
                        }
                        for (Int32 i = 0; i < relData.nBeamPart; i++) {
                            API_Guid elGuid = (*relData.beamPart)[i].guid;
                            subelemGuid.Push (elGuid);
                        }
                        for (Int32 i = 0; i < relData.nMorph; i++) {
                            API_Guid elGuid = (*relData.morphs)[i];
                            subelemGuid.Push (elGuid);
                        }
                    }
                    #else
                    typeinzone.Push (API_ObjectID);
                    typeinzone.Push (API_LampID);
                    typeinzone.Push (API_StairID);
                    typeinzone.Push (API_RiserID);
                    typeinzone.Push (API_TreadID);
                    typeinzone.Push (API_StairStructureID);
                    if (syncSettings.wallS) {
                        typeinzone.Push (API_WallID);
                        typeinzone.Push (API_SlabID);
                        typeinzone.Push (API_ColumnID);
                        typeinzone.Push (API_BeamID);
                        typeinzone.Push (API_RoofID);
                        typeinzone.Push (API_ShellID);
                        typeinzone.Push (API_MorphID);
                    }
                    if (syncSettings.widoS) {
                        typeinzone.Push (API_WindowID);
                        typeinzone.Push (API_DoorID);
                        typeinzone.Push (API_SkylightID);
                    }
                    if (syncSettings.cwallS) typeinzone.Push (API_CurtainWallID);
                    for (const API_ElemTypeID& typeelem : typeinzone) {
                        if (relData.elementsGroupedByType.ContainsKey (typeelem)) {
                            for (const API_Guid& elGuid : relData.elementsGroupedByType[typeelem]) {
                                subelemGuid.Push (elGuid);
                            }
                        }
                    }
                    #endif
                }
                ACAPI_DisposeRoomRelationHdls (&relData);
            }
            break;
        default:
            break;
    }

}

// -----------------------------------------------------------------------------
// Получение размеров Морфа
// Формирует словарь ParamDictValue& pdictvalue со значениями
// -----------------------------------------------------------------------------
bool ParamHelpers::ReadMorphParam (const API_Element& element, ParamDictValue& pdictvalue)
{
    if (!element.header.hasMemo) return NoError;
    #if defined(TESTING)
    DBprnt ("      ReadMorphParam");
    #endif
    API_ElementMemo memo;
    BNZeroMemory (&memo, sizeof (API_ElementMemo));
    GSErrCode err = ACAPI_Element_GetMemo (element.header.guid, &memo);
    if (err != NoError || memo.morphBody == nullptr) {
        ACAPI_DisposeElemMemoHdls (&memo);
        return false;
    }
    double L = 0;
    double Lx = 0;
    double Ly = 0;
    double Lz = 0;
    double Max_x = 0;
    double Max_y = 0;
    double Max_z = 0;
    double Min_x = 0;
    double Min_y = 0;
    double Min_z = 0;
    double A = 0;
    double B = 0;
    double ZZYZX = 0;
    if (memo.morphBody->IsWireBody () && !memo.morphBody->IsSolidBody ()) {
        Int32 edgeCnt = memo.morphBody->GetEdgeCount ();
        for (Int32 iEdge = 0; iEdge < edgeCnt; iEdge++) {
            const EDGE& edge = memo.morphBody->GetConstEdge (iEdge);
            const VERT& vtx1 = memo.morphBody->GetConstVertex (edge.vert1);
            const VERT& vtx2 = memo.morphBody->GetConstVertex (edge.vert2);
            double x1 = vtx1.x;
            double x2 = vtx2.x;
            double y1 = vtx1.y;
            double y2 = vtx2.y;
            double z1 = vtx1.z;
            double z2 = vtx2.z;
            double dx = pow (x2 - x1, 2);
            double dy = pow (y2 - y1, 2);
            double dz = pow (z2 - z1, 2);
            double dl = DoubleM2IntMM (sqrt (dx + dy + dz)) / 1000.0;
            double dlx = DoubleM2IntMM (sqrt (dy + dx)) / 1000.0;
            double dly = DoubleM2IntMM (sqrt (dx + dz)) / 1000.0;
            double dlz = DoubleM2IntMM (sqrt (dx + dy)) / 1000.0;
            L = L + dl;
            Lx = Lx + dlx;
            Ly = Ly + dly;
            Lz = Lz + dlz;
            Max_x = fmax (Max_x, x1);
            Max_x = fmax (Max_x, x2);
            Max_y = fmax (Max_y, y1);
            Max_y = fmax (Max_y, y2);
            Max_z = fmax (Max_z, z1);
            Max_z = fmax (Max_z, z2);
            Min_x = fmin (Min_x, x1);
            Min_x = fmin (Min_x, x2);
            Min_y = fmin (Min_y, y1);
            Min_y = fmin (Min_y, y2);
            Min_z = fmin (Min_z, z1);
            Min_z = fmin (Min_z, z2);
        }
        Max_x = DoubleM2IntMM (Max_x) / 1000.0;
        Max_y = DoubleM2IntMM (Max_y) / 1000.0;
        Max_z = DoubleM2IntMM (Max_z) / 1000.0;
        Min_x = DoubleM2IntMM (Min_x) / 1000.0;
        Min_y = DoubleM2IntMM (Min_y) / 1000.0;
        Min_z = DoubleM2IntMM (Min_z) / 1000.0;
        A = Max_x - Min_x;
        B = Max_y - Min_y;
        ZZYZX = Max_z - Min_z;
        ParamDictValue pdictvaluemorph;
        ParamHelpers::AddLengthValueToParamDictValue (pdictvaluemorph, element.header.guid, "morph:", "l", L);
        ParamHelpers::AddLengthValueToParamDictValue (pdictvaluemorph, element.header.guid, "morph:", "lx", Lx);
        ParamHelpers::AddLengthValueToParamDictValue (pdictvaluemorph, element.header.guid, "morph:", "ly", Ly);
        ParamHelpers::AddLengthValueToParamDictValue (pdictvaluemorph, element.header.guid, "morph:", "lz", Lz);
        ParamHelpers::AddLengthValueToParamDictValue (pdictvaluemorph, element.header.guid, "morph:", "max_x", Max_x);
        ParamHelpers::AddLengthValueToParamDictValue (pdictvaluemorph, element.header.guid, "morph:", "min_x", Min_x);
        ParamHelpers::AddLengthValueToParamDictValue (pdictvaluemorph, element.header.guid, "morph:", "max_y", Max_y);
        ParamHelpers::AddLengthValueToParamDictValue (pdictvaluemorph, element.header.guid, "morph:", "min_y", Min_y);
        ParamHelpers::AddLengthValueToParamDictValue (pdictvaluemorph, element.header.guid, "morph:", "max_z", Max_z);
        ParamHelpers::AddLengthValueToParamDictValue (pdictvaluemorph, element.header.guid, "morph:", "min_z", Min_z);
        ParamHelpers::AddLengthValueToParamDictValue (pdictvaluemorph, element.header.guid, "morph:", "a", A);
        ParamHelpers::AddLengthValueToParamDictValue (pdictvaluemorph, element.header.guid, "morph:", "b", B);
        ParamHelpers::AddLengthValueToParamDictValue (pdictvaluemorph, element.header.guid, "morph:", "zzyzx", ZZYZX);
        ParamHelpers::CompareParamDictValue (pdictvaluemorph, pdictvalue);
        ACAPI_DisposeElemMemoHdls (&memo);
        return true;
    } else {
        ACAPI_DisposeElemMemoHdls (&memo);
        return false;
    }
}

// -----------------------------------------------------------------------------
// Назначает флаги источника чтения по rawName параметра
// -----------------------------------------------------------------------------
void ParamHelpers::SetParamValueSourseByName (ParamValue& pvalue)
{
    if (pvalue.fromCoord ||
       pvalue.fromGDLparam ||
       pvalue.fromGDLdescription ||
       pvalue.fromProperty ||
       pvalue.fromMaterial ||
       pvalue.fromInfo ||
       pvalue.fromIFCProperty ||
       pvalue.fromMorph ||
       pvalue.fromPropertyDefinition ||
       pvalue.fromClassification ||
       pvalue.fromGDLArray ||
       pvalue.fromAttribDefinition ||
       pvalue.fromAttribElement ||
       pvalue.fromID
       ) return;
    if (pvalue.rawName.Contains ("{@coord:")) pvalue.fromCoord = true;
    if (pvalue.rawName.Contains ("{@gdl")) pvalue.fromGDLparam = true;
    if (pvalue.rawName.Contains ("{@description:")) {
        pvalue.fromGDLparam = true;
        pvalue.fromGDLdescription = true;
    }
    if ((pvalue.fromGDLparam || pvalue.fromGDLdescription) && pvalue.rawName.Contains ("@arr_")) pvalue.fromGDLArray = true;
    if (pvalue.fromGDLparam && (!pvalue.rawName_row_start.IsEmpty () || !pvalue.rawName_row_end.IsEmpty () || !pvalue.rawName_col_start.IsEmpty () || !pvalue.rawName_col_end.IsEmpty ())) pvalue.needPreRead = true;
    if (pvalue.rawName.Contains ("{@property")) pvalue.fromProperty = true;
    if (pvalue.rawName.Contains ("{@material")) pvalue.fromMaterial = true;
    if (pvalue.rawName.Contains ("{@info")) pvalue.fromInfo = true;
    if (pvalue.rawName.Contains ("{@ifc")) pvalue.fromIFCProperty = true;
    if (pvalue.rawName.Contains ("{@morph")) pvalue.fromMorph = true;
    if (pvalue.rawName.Contains ("{@id")) pvalue.fromID = true;
    if (pvalue.rawName.Contains ("{@attrib")) pvalue.fromAttribElement = true;
    if (pvalue.rawName.Contains ("{@mep")) pvalue.fromMEP = true;
    if (pvalue.rawName.Contains ("{@listdata")) pvalue.fromListData = true;
    if (pvalue.rawName.Contains ("{@property:buildingmaterialproperties/")) pvalue.fromAttribDefinition = true;
}

GS::UniString ParamHelpers::NameToRawName (const GS::UniString& name, FormatString& formatstring)
{
    if (name.IsEmpty ()) return "";
    GS::UniString rawname_prefix = "";
    GS::UniString name_ = name.ToLowerCase ();
    if (name_.Contains ("{") && name_.Contains ("{")) name_ = name_.GetSubstring ('{', '}', 0);
    // Ищём строку с указанием формата вывода (метры/миллиметры)
    GS::UniString stringformat = FormatStringFunc::GetFormatString (name_);
    formatstring = FormatStringFunc::ParseFormatString (stringformat);
    name_ = GetPropertyENGName (name_).ToLowerCase ();

    // Проверяем - есть ли указатель на тип параметра (GDL, Property, IFC)
    if (name_.Contains (":")) {
        GS::Array<GS::UniString> partstring;
        UInt32 n = StringSplt (name_, ":", partstring);
        if (n > 1) {
            rawname_prefix = partstring[0] + ":";
            name_ = partstring[1].ToLowerCase ();
        }
    }
    if (rawname_prefix.IsEmpty () && name_.IsEqual ("id")) rawname_prefix = "@id:";
    if (rawname_prefix.IsEmpty ()) rawname_prefix = "@gdl:";
    if (!rawname_prefix.Contains ("@")) {
        rawname_prefix = "@" + rawname_prefix;
    }
    name_.ReplaceAll ("\\/", "/");
    GS::UniString rawName = "{" + rawname_prefix + name_ + "}";
    return rawName;
}

// -----------------------------------------------------------------------------
// Добавление пустого значения в словарь ParamDictValue
// -----------------------------------------------------------------------------
void ParamHelpers::AddValueToParamDictValue (ParamDictValue& params, const GS::UniString& name)
{
    if (name.IsEmpty ()) return;
    FormatString formatstring;
    GS::UniString rawName = ParamHelpers::NameToRawName (name, formatstring);
    if (!params.ContainsKey (rawName)) {
        ParamValue pvalue;
        GS::UniString name_ = name.ToLowerCase ();
        pvalue.rawName = rawName;
        pvalue.name = name_;
        pvalue.val.formatstring = formatstring;
        ParamHelpers::SetParamValueSourseByName (pvalue);
        params.Add (rawName, pvalue);
    }
}

// -----------------------------------------------------------------------------
// Проверяет необходимость добавления в словарь параметров
// Если в имени параметра содержится информация о номере аттрибута и имя такого параметра есть в словаре - вернёт истину
// -----------------------------------------------------------------------------
bool ParamHelpers::needAdd (ParamDictValue& params, GS::UniString& rawName)
{
    bool addNew = false;
    if (rawName.Contains (CharENTER)) {
        UInt32 n = rawName.FindFirst (CharENTER);
        GS::UniString rawName_ = rawName.GetSubstring (0, n) + "}";
        addNew = params.ContainsKey (rawName_);
    }
    return addNew;
}

// --------------------------------------------------------------------
// Запись параметра ParamValue в словарь ParamDict, если его там прежде не было
// --------------------------------------------------------------------
void ParamHelpers::AddParamValue2ParamDict (const API_Guid& elemGuid, ParamValue& param, ParamDictValue& paramToRead)
{
    GS::UniString rawName = param.rawName;
    if (!paramToRead.ContainsKey (rawName)) {
        if (param.fromGuid == APINULLGuid) param.fromGuid = elemGuid;
        paramToRead.Add (rawName, param);
    }
}

// --------------------------------------------------------------------
// Запись параметра ParamValue в словарь элементов ParamDictElement, если его там прежде не было
// --------------------------------------------------------------------
void ParamHelpers::AddParamValue2ParamDictElement (const ParamValue& param, ParamDictElement& paramToRead)
{
    ParamHelpers::AddParamValue2ParamDictElement (param.fromGuid, param, paramToRead);
}

// --------------------------------------------------------------------
// Сопоставляет параметры
// --------------------------------------------------------------------
bool ParamHelpers::CompareParamValue (ParamValue& paramFrom, ParamValue& paramTo, FormatString stringformat)
{
    #ifdef TESTING
    if (!paramTo.isValid) {
        DBtest (paramTo.isValid, paramTo.rawName + " paramTo.isValid");
    } else {
        if (!paramTo.val.hasrawDouble && paramTo.val.type != API_PropertyStringValueType) DBtest (paramTo.val.hasrawDouble, paramTo.rawName + " CompareParamValue::paramTo.val.hasrawDouble");
    }
    if (!paramFrom.isValid) {
        DBtest (paramFrom.isValid, paramFrom.rawName + " paramFrom.isValid");
    } else {
        if (!paramFrom.val.hasrawDouble && paramFrom.val.type != API_PropertyStringValueType) DBtest (paramFrom.val.hasrawDouble, paramFrom.rawName + " CompareParamValue::paramFrom.val.hasrawDouble");
    }
    #endif
    if (!paramFrom.isValid) return false;
    if (paramFrom.fromClassification && paramTo.fromClassification && paramFrom.val.guidval == APINULLGuid) {
        return false;
    }
    if (paramFrom.fromClassification && paramTo.fromClassification) {
        if (paramFrom.val.guidval != paramTo.val.guidval) {
            paramTo.val = paramFrom.val;
            paramTo.isValid = true;
            return true;
        } else {
            return false;
        }
    }
    if (paramTo.isValid || paramTo.fromProperty || paramTo.fromPropertyDefinition) {
        if (stringformat.isEmpty) stringformat = paramTo.val.formatstring;
        if (stringformat.isEmpty) stringformat = paramFrom.val.formatstring;
        // Приводим к единому виду перед проверкой
        if (!stringformat.isEmpty) {
            // Если в правиле задана более высокая точность - нужно использовать значение до предыдущего округления.
            if (paramTo.val.formatstring.n_zero < stringformat.n_zero || (!stringformat.needRound && paramTo.val.formatstring.needRound) || paramTo.val.formatstring.forceRaw != stringformat.forceRaw) {
                paramTo.val.doubleValue = paramTo.val.rawDoubleValue;
            }
            if (paramFrom.val.formatstring.n_zero < stringformat.n_zero || (!stringformat.needRound && paramFrom.val.formatstring.needRound) || paramFrom.val.formatstring.forceRaw != stringformat.forceRaw) {
                paramFrom.val.doubleValue = paramFrom.val.rawDoubleValue;
            }
            paramTo.val.formatstring = stringformat;
            paramFrom.val.formatstring = stringformat;
            ParamHelpers::ConvertByFormatString (paramTo);
            ParamHelpers::ConvertByFormatString (paramFrom);
        }
        //Сопоставляем и записываем, если значения отличаются
        if (paramFrom != paramTo) {
            paramTo.val = paramFrom.val; // Записываем только значения
            paramTo.isValid = true;
            return true;
        }
    }
    return false;
}

// --------------------------------------------------------------------
// Запись параметра ParamValue в словарь элементов ParamDictElement, если его там прежде не было
// --------------------------------------------------------------------
void ParamHelpers::AddParamValue2ParamDictElement (const API_Guid& elemGuid, const ParamValue& param, ParamDictElement& paramToRead)
{
    GS::UniString rawName = param.rawName;
    if (paramToRead.ContainsKey (elemGuid)) {
        if (!paramToRead.Get (elemGuid).ContainsKey (rawName)) {
            paramToRead.Get (elemGuid).Add (rawName, param);
        } else {
            paramToRead.Get (elemGuid).Set (rawName, param);
        }
    } else {
        ParamDictValue params;
        params.Add (rawName, param);
        paramToRead.Add (elemGuid, params);
    }
}

// --------------------------------------------------------------------
// Запись словаря ParamDictValue в словарь элементов ParamDictElement
// --------------------------------------------------------------------
void ParamHelpers::AddParamDictValue2ParamDictElement (const API_Guid& elemGuid, ParamDictValue& param, ParamDictElement& paramToRead)
{
    if (paramToRead.ContainsKey (elemGuid)) {
        for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = param.EnumeratePairs (); cIt != NULL; ++cIt) {
            #if defined(AC_28)
            GS::UniString rawName = cIt->key;
            #else
            GS::UniString rawName = *cIt->key;
            #endif
            if (!paramToRead.Get (elemGuid).ContainsKey (rawName)) {
                #if defined(AC_28)
                ParamValue param = cIt->value;
                #else
                ParamValue param = *cIt->value;
                #endif
                if (param.fromGuid == APINULLGuid) {
                    param.fromGuid = elemGuid;
                } else {
                    #if defined(TESTING)
                    if (param.fromGuid != elemGuid) DBprnt ("err AddParamDictValue2ParamDictElement - different GUID ", rawName);
                    #endif
                }
                paramToRead.Get (elemGuid).Add (rawName, param);
            }
        }
    } else {
        paramToRead.Add (elemGuid, param);
    }
}

// -----------------------------------------------------------------------------
// Добавление массива свойств в словарь
// -----------------------------------------------------------------------------
bool ParamHelpers::AddProperty (ParamDictValue& params, GS::Array<API_Property>& properties)
{
    UInt32 nparams = params.GetSize ();
    if (nparams < 1) return false;
    bool flag_find = false;
    for (API_Property& property : properties) {
        ParamValue pvalue;
        ParamHelpers::SetrawNameFromProperty (pvalue, property);
        GS::UniString rawName = pvalue.rawName;
        bool hasname = params.ContainsKey (rawName);
        bool needAdd = ParamHelpers::needAdd (params, rawName);
        // Заполняем stringformat
        FormatString fstring;
        if (hasname || needAdd) {
            if (hasname) {
                fstring = params.Get (rawName).val.formatstring;
                if (!fstring.isEmpty) pvalue.val.formatstring = fstring;
            }
            if (ParamHelpers::ConvertToParamValue (pvalue, property)) {
                if (hasname) {
                    params.Set (rawName, pvalue);
                    nparams--;
                    flag_find = true;
                    if (nparams == 0) {
                        return flag_find;
                    }
                } else {
                    if (needAdd) {
                        params.Add (rawName, pvalue);
                        flag_find = true;
                    }
                }
            }
        } else {
            #if defined(TESTING)
            DBprnt ("Property not added", rawName);
            #endif
        }
    }
    return flag_find;
}

// -----------------------------------------------------------------------------
// Добавление значения в словарь ParamDictValue
// -----------------------------------------------------------------------------
void ParamHelpers::AddBoolValueToParamDictValue (ParamDictValue& params, const API_Guid& elemGuid, const GS::UniString& rawName_prefix, const GS::UniString& name, const bool val)
{
    ParamValue pvalue = {};
    pvalue.rawName = "{@";
    pvalue.rawName.Append (rawName_prefix);
    pvalue.rawName.Append (name.ToLowerCase ());
    pvalue.rawName.Append ("}");
    if (params.ContainsKey (pvalue.rawName)) {
        #if defined(TESTING)
        DBprnt ("AddToParamDictValue err", pvalue.rawName + " ContainsKey");
        #endif
        return;
    }
    pvalue.name = name.ToLowerCase ();
    ParamHelpers::ConvertBoolToParamValue (pvalue, "", val);
    params.Add (pvalue.rawName, pvalue);
}


// -----------------------------------------------------------------------------
// Добавление значения длины в словарь ParamDictValue
// -----------------------------------------------------------------------------
void ParamHelpers::AddLengthValueToParamDictValue (ParamDictValue& params, const API_Guid& elemGuid, const GS::UniString& rawName_prefix, const GS::UniString& name, const double val)
{
    ParamValue pvalue;
    pvalue.rawName = "{@";
    pvalue.rawName.Append (rawName_prefix);
    pvalue.rawName.Append (name.ToLowerCase ());
    pvalue.rawName.Append ("}");
    if (params.ContainsKey (pvalue.rawName)) {
        #if defined(TESTING)
        DBprnt ("AddToParamDictValue err", pvalue.rawName + " ContainsKey");
        #endif
        return;
    }
    pvalue.name = name.ToLowerCase ();
    pvalue.val.formatstring = FormatStringFunc::ParseFormatString ("1mm");
    ParamHelpers::ConvertDoubleToParamValue (pvalue, "", val);
    params.Add (pvalue.rawName, pvalue);
}


// -----------------------------------------------------------------------------
// Добавление значения в словарь ParamDictValue
// rawName_prefix без скобок
// -----------------------------------------------------------------------------
void ParamHelpers::AddDoubleValueToParamDictValue (ParamDictValue& params, const API_Guid& elemGuid, const GS::UniString& rawName_prefix, const GS::UniString& name, const double val)
{
    ParamValue pvalue;
    pvalue.rawName = "{@";
    pvalue.rawName.Append (rawName_prefix);
    pvalue.rawName.Append (name.ToLowerCase ());
    pvalue.rawName.Append ("}");
    if (params.ContainsKey (pvalue.rawName)) {
        #if defined(TESTING)
        DBprnt ("AddToParamDictValue err", pvalue.rawName + " ContainsKey");
        #endif
        return;
    }
    pvalue.name = name.ToLowerCase ();
    ParamHelpers::ConvertDoubleToParamValue (pvalue, "", val);
    params.Add (pvalue.rawName, pvalue);
}

// -----------------------------------------------------------------------------
// Добавление значения в словарь ParamDictValue
// -----------------------------------------------------------------------------
void ParamHelpers::AddStringValueToParamDictValue (ParamDictValue& params, const API_Guid& elemGuid, const GS::UniString& rawName_prefix, const GS::UniString& name, const GS::UniString val)
{
    ParamValue pvalue;
    pvalue.rawName = "{@";
    pvalue.rawName.Append (rawName_prefix);
    pvalue.rawName.Append (name.ToLowerCase ());
    pvalue.rawName.Append ("}");
    if (params.ContainsKey (pvalue.rawName)) {
        #if defined(TESTING)
        DBprnt ("AddToParamDictValue err", pvalue.rawName + " ContainsKey");
        #endif
        return;
    }
    pvalue.name = name.ToLowerCase ();
    ParamHelpers::ConvertStringToParamValue (pvalue, "", val);
    params.Add (pvalue.rawName, pvalue);
}

// -----------------------------------------------------------------------------
// Получение координат объекта
// symb_pos_x , symb_pos_y, symb_pos_z
// Для панелей навесной стены возвращает центр панели
// Для колонны или объекта - центр колонны и отм. низа
// Для зоны - центр зоны (без отметки, symb_pos_z = 0)
// -----------------------------------------------------------------------------
bool ParamHelpers::ReadCoords (const API_Element& element, ParamDictValue& params)
{
    #if defined(TESTING)
    DBprnt ("      ReadCoords");
    #endif
    ParamDictValue pdictvaluecoord;
    bool isFliped = false;
    double x = 0; double y = 0; double z = 0; double angz = 0;
    double sx = 0; double sy = 0;
    double ww = 0; double hw = 0;  //Размеры проёма
    double ex = 0; double ey = 0;
    double dx = 0; double dy = 0;
    // Координаты относительно пользовательского начала
    double xu = 0; double yu = 0; double zu = 0;
    double sxu = 0; double syu = 0;
    double exu = 0; double eyu = 0;
    double zw = 0; //Высота стены, в которой размещён проём
    double lox = 0; double loy = 0; double loz = 0;
    double offx = 0; double offy = 0;
    GS::UniString locorig = "{@coord:locorigin_x}";
    if (params.ContainsKey (locorig)) lox = params.Get (locorig).val.rawDoubleValue;
    locorig = "{@coord:locorigin_y}";
    if (params.ContainsKey (locorig)) loy = params.Get (locorig).val.rawDoubleValue;
    locorig = "{@coord:locorigin_z}";
    if (params.ContainsKey (locorig)) loz = params.Get (locorig).val.rawDoubleValue;
    locorig = "{@coord:offsetorigin_x}";
    if (params.ContainsKey (locorig)) offx = params.Get (locorig).val.rawDoubleValue;
    locorig = "{@coord:offsetorigin_y}";
    if (params.ContainsKey (locorig)) offy = params.Get (locorig).val.rawDoubleValue;

    if (is_equal (offx, 0) && is_equal (offy, 0)) {
        ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "has_distant_element", false);
    } else {
        ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "has_distant_element", true);
    }
    double tolerance_coord = 0.001;
    double tolerance_coord_hard = 0.000001;
    double tolerance_ang = 0.00001;
    bool hasSymbpos = false; bool hasLine = false;
    double angznorth = -1.0; double north = -1.0; bool skip_north = false;

    double symb_rotangle_fraction = 0.0;
    bool bsymb_rotangle_correct = false;
    bool bsymb_rotangle_correct_1000 = false;

    double slantDirectionAngle = 0;
    double axisRotationAngle = 0;
    GS::UniString angznorthtxt = "UNDEF"; GS::UniString angznorthtxteng = "UNDEF";
    GS::UniString globnorthkey = "{@glob:glob_north_dir}";
    if (!params.ContainsKey (globnorthkey)) {
        skip_north = true;
    } else {
        north = params.Get (globnorthkey).val.doubleValue;
    }
    bool bsync_coord_correct = true;
    GS::UniString sync_coord_correctkey = "{@property:sync_correct_flag}";
    if (params.ContainsKey (sync_coord_correctkey)) {
        if (params.Get (sync_coord_correctkey).isValid) {
            bsync_coord_correct = params.Get (sync_coord_correctkey).val.boolValue;
        }
    }
    API_ElemTypeID eltype = GetElemTypeID (element);
    API_Element owner;
    // Обработка навесной стены- случай особый, т.к. у неё может быть несколькор сегментов
    if (eltype == API_CurtainWallID && element.header.hasMemo) {
        double aang = fabs (fmod (element.curtainWall.angle, 180.0));
        if (aang > 90.0) aang = 180.0 - aang;
        if (aang < 5.0) skip_north = true;
        hasLine = true;
        isFliped = element.curtainWall.flipped;
        double sx_ = 0; double sy_ = 0;
        double ex_ = 0; double ey_ = 0;
        double sxu_ = 0; double syu_ = 0;
        double exu_ = 0; double eyu_ = 0;
        GS::UniString angznorthtxteng_ = "";
        bool bsymb_pos_sx_correct_ = false;
        bool bsymb_pos_sy_correct_ = false;
        bool bsymb_pos_ex_correct_ = false;
        bool bsymb_pos_ey_correct_ = false;
        bool bsymb_pos_e_correct_ = false;
        bool bsymb_pos_s_correct_ = false;
        bool bsymb_pos_correct_ = false;
        bool bsymb_rotangle_correct_ = false;
        bool bsymb_rotangle_correct_1000_ = false;
        bool bsymb_pos_sx_correctu_ = false;
        bool bsymb_pos_sy_correctu_ = false;
        bool bsymb_pos_ex_correctu_ = false;
        bool bsymb_pos_ey_correctu_ = false;
        bool bsymb_pos_e_correctu_ = false;
        bool bsymb_pos_s_correctu_ = false;
        bool bsymb_pos_correctu_ = false;
        API_ElementMemo  memo;
        if (ACAPI_Element_GetMemo (element.header.guid, &memo, APIMemoMask_CWallSegments) == NoError) {
            Int32 size = BMGetPtrSize (reinterpret_cast<GSPtr>(memo.cWallSegments)) / sizeof (API_CWSegmentType);
            for (Int32 inx_segment = 0; inx_segment < size; ++inx_segment) {
                sx = memo.cWallSegments[inx_segment].begC.x + offx;
                sy = memo.cWallSegments[inx_segment].begC.y + offy;
                ex = memo.cWallSegments[inx_segment].endC.x + offx;
                ey = memo.cWallSegments[inx_segment].endC.y + offy;
                sxu = sx - lox; syu = sy - loy;
                exu = ex - lox; eyu = ey - loy;
                if (inx_segment == 0) {
                    sx_ = sx; sy_ = sy;
                    ex_ = ex; ey_ = ey;
                    sxu_ = sxu; syu_ = syu;
                    exu_ = exu; eyu_ = eyu;
                } else {
                    ex_ = ex; ey_ = ey;
                    exu_ = exu; eyu_ = eyu;
                }
                CoordRotAngle (sx, sy, ex, ey, isFliped, angz);
                bsymb_rotangle_correct = CoordCorrectAngle (angz, tolerance_ang, symb_rotangle_fraction, bsymb_rotangle_correct_1000);
                if (!skip_north) CoordNorthAngle (north, angz, angznorth, angznorthtxt, angznorthtxteng);
                if (angznorthtxteng_.IsEmpty () && !angznorthtxteng.IsEmpty ()) angznorthtxteng_ = angznorthtxteng;
                if (!angznorthtxteng.IsEqual (angznorthtxteng_) && !angznorthtxteng.IsEmpty ()) {
                    skip_north = true; //Несколько сегментов с разным направлением. Определить север не получится
                }
                bool bsymb_pos_sx_correct = check_accuracy (sx, tolerance_coord);
                bool bsymb_pos_sy_correct = check_accuracy (sy, tolerance_coord);
                bool bsymb_pos_ex_correct = check_accuracy (ex, tolerance_coord);
                bool bsymb_pos_ey_correct = check_accuracy (ey, tolerance_coord);
                bool bsymb_pos_e_correct = bsymb_pos_sx_correct && bsymb_pos_sy_correct;
                bool bsymb_pos_s_correct = bsymb_pos_ex_correct && bsymb_pos_ey_correct;
                bool bsymb_pos_correct = bsymb_pos_e_correct && bsymb_pos_s_correct;

                bool bsymb_pos_sx_correctu = check_accuracy (sxu, tolerance_coord);
                bool bsymb_pos_sy_correctu = check_accuracy (syu, tolerance_coord);
                bool bsymb_pos_ex_correctu = check_accuracy (exu, tolerance_coord);
                bool bsymb_pos_ey_correctu = check_accuracy (eyu, tolerance_coord);
                bool bsymb_pos_e_correctu = bsymb_pos_sx_correctu && bsymb_pos_sy_correctu;
                bool bsymb_pos_s_correctu = bsymb_pos_ex_correctu && bsymb_pos_ey_correctu;
                bool bsymb_pos_correctu = bsymb_pos_e_correctu && bsymb_pos_s_correctu;

                if (bsymb_rotangle_correct) bsymb_rotangle_correct_ = true;
                if (bsymb_rotangle_correct_1000) bsymb_rotangle_correct_1000_ = true;
                if (bsymb_pos_sx_correct) bsymb_pos_sx_correct_ = true;
                if (bsymb_pos_sy_correct) bsymb_pos_sy_correct_ = true;
                if (bsymb_pos_ex_correct) bsymb_pos_ex_correct_ = true;
                if (bsymb_pos_ey_correct) bsymb_pos_ey_correct_ = true;
                if (bsymb_pos_e_correct) bsymb_pos_e_correct_ = true;
                if (bsymb_pos_s_correct) bsymb_pos_s_correct_ = true;
                if (bsymb_pos_correct) bsymb_pos_correct_ = true;

                if (bsymb_pos_sx_correctu) bsymb_pos_sx_correctu_ = true;
                if (bsymb_pos_sy_correctu) bsymb_pos_sy_correctu_ = true;
                if (bsymb_pos_ex_correctu) bsymb_pos_ex_correctu_ = true;
                if (bsymb_pos_ey_correctu) bsymb_pos_ey_correctu_ = true;
                if (bsymb_pos_e_correctu) bsymb_pos_e_correctu_ = true;
                if (bsymb_pos_s_correctu) bsymb_pos_s_correctu_ = true;
                if (bsymb_pos_correctu) bsymb_pos_correctu_ = true;
            }
        }
        ACAPI_DisposeElemMemoHdls (&memo);
        if (skip_north) {
            angznorth = -1.0;
            angznorthtxt = "UNDEF";
            angznorthtxteng = "UNDEF";
        }
        ParamHelpers::AddDoubleValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "north_dir", angznorth);
        ParamHelpers::AddStringValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "north_dir_str", angznorthtxt);
        ParamHelpers::AddStringValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "north_dir_eng", angznorthtxteng);

        ParamHelpers::AddLengthValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_x", sx_);
        ParamHelpers::AddLengthValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_y", sy_);
        ParamHelpers::AddLengthValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_sx", sx_);
        ParamHelpers::AddLengthValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_sy", sy_);
        ParamHelpers::AddLengthValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_ex", ex_);
        ParamHelpers::AddLengthValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_ey", ey_);

        ParamHelpers::AddLengthValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_x", sxu_);
        ParamHelpers::AddLengthValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_y", syu_);
        ParamHelpers::AddLengthValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_sx", sxu_);
        ParamHelpers::AddLengthValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_sy", syu_);
        ParamHelpers::AddLengthValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_ex", exu_);
        ParamHelpers::AddLengthValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_ey", eyu_);

        ParamHelpers::AddDoubleValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_rotangle", angz);
        ParamHelpers::AddDoubleValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_rotangle_mod5", fmod (angz, 5.0));
        ParamHelpers::AddDoubleValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_rotangle_mod10", fmod (angz, 10.0));
        ParamHelpers::AddDoubleValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_rotangle_mod45", fmod (angz, 45.0));
        ParamHelpers::AddDoubleValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_rotangle_mod90", fmod (angz, 90.0));
        ParamHelpers::AddDoubleValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_rotangle_mod180", fmod (angz, 180.0));
        if (bsync_coord_correct) {
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_s_correct", bsymb_pos_s_correctu_);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_e_correct", bsymb_pos_e_correctu_);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_x_correct", bsymb_pos_sx_correctu_);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_y_correct", bsymb_pos_sy_correctu_);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_sx_correct", bsymb_pos_sx_correctu_);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_sy_correct", bsymb_pos_sy_correctu_);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_ex_correct", bsymb_pos_ex_correctu_);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_ey_correct", bsymb_pos_ey_correctu_);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_correct", bsymb_pos_correctu_);

            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_s_correct", bsymb_pos_s_correct_);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_e_correct", bsymb_pos_e_correct_);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_x_correct", bsymb_pos_sx_correct_);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_y_correct", bsymb_pos_sy_correct_);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_sx_correct", bsymb_pos_sx_correct_);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_sy_correct", bsymb_pos_sy_correct_);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_ex_correct", bsymb_pos_ex_correct_);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_ey_correct", bsymb_pos_ey_correct_);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_correct", bsymb_pos_correct_);
            ParamHelpers::AddDoubleValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_rotangle_fraction", symb_rotangle_fraction);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_rotangle_correct", bsymb_rotangle_correct_);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_rotangle_correct_1000", bsymb_rotangle_correct_1000_);
        } else {
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_s_correct", true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_e_correct", true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_x_correct", true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_y_correct", true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_sx_correct", true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_sy_correct", true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_ex_correct", true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_ey_correct", true);

            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_s_correct", true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_e_correct", true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_x_correct", true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_y_correct", true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_sx_correct", true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_sy_correct", true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_ex_correct", true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_ey_correct", true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_correct", true);

            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_correct", true);
            ParamHelpers::AddDoubleValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_rotangle_fraction", 0.0);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_rotangle_correct", true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_rotangle_correct_1000", true);
        }
        ParamHelpers::CompareParamDictValue (pdictvaluecoord, params);
        return true;
    }

    // Если нужно определить направление окон или дверей - запрашиваем родительский элемент
    if (eltype == API_WindowID || eltype == API_DoorID || (!skip_north && eltype == API_CurtainWallPanelID)) {
        BNZeroMemory (&owner, sizeof (API_Element));
        if (eltype == API_CurtainWallPanelID) owner.header.guid = element.cwPanel.owner;
        if (eltype == API_WindowID) owner.header.guid = element.window.owner;
        if (eltype == API_DoorID) owner.header.guid = element.door.owner;
        if (ACAPI_Element_Get (&owner) != NoError) return false;
        API_ElemTypeID ownereltype = GetElemTypeID (owner);
        if (ownereltype == API_WallID) {
            sx = owner.wall.begC.x + offx;
            sy = owner.wall.begC.y + offy;
            ex = owner.wall.endC.x + offx;
            ey = owner.wall.endC.y + offy;
            zw = owner.wall.height;
            isFliped = owner.wall.flipped;
            hasLine = true;
        }
        #ifndef AC_22
        if (eltype == API_CurtainWallPanelID && ownereltype == API_CurtainWallID && owner.header.hasMemo) {
            double aang = fabs (fmod (owner.curtainWall.angle, 180.0));
            if (aang > 90.0) aang = 180.0 - aang;
            if (aang < 5.0) skip_north = true;
            Int32 inx_segment = element.cwPanel.segmentID;
            API_ElementMemo  memo;
            if (ACAPI_Element_GetMemo (owner.header.guid, &memo, APIMemoMask_CWallSegments) == NoError) {
                Int32 size = BMGetPtrSize (reinterpret_cast<GSPtr> (memo.cWallSegments)) / sizeof (API_CWSegmentType);
                if (size >= inx_segment) {
                    sx = memo.cWallSegments[inx_segment].begC.x + offx;
                    sy = memo.cWallSegments[inx_segment].begC.y + offy;
                    ex = memo.cWallSegments[inx_segment].endC.x + offx;
                    ey = memo.cWallSegments[inx_segment].endC.y + offy;
                    hasLine = true;
                    isFliped = owner.curtainWall.flipped;
                }
            }
            ACAPI_DisposeElemMemoHdls (&memo);
        }
        #endif
    } else {
        UNUSED_VARIABLE (owner);
    }
    switch (eltype) {
        case API_WindowID:
            x = element.window.objLoc;
            y = 0;
            z = element.window.lower;
            ww = element.window.openingBase.width;
            hw = element.window.openingBase.height;
            hasSymbpos = true;
            break;
        case API_DoorID:
            x = element.door.objLoc;
            y = 0;
            z = element.door.lower;
            ww = element.window.openingBase.width;
            hw = element.window.openingBase.height;
            hasSymbpos = true;
            break;
        case API_CurtainWallPanelID:
            #ifndef AC_22
            x = element.cwPanel.centroid.x + offx;
            y = element.cwPanel.centroid.y + offy;
            z = element.cwPanel.centroid.z;
            hasSymbpos = true;
            #endif
            break;
        case API_ObjectID:
            x = element.object.pos.x + offx;
            y = element.object.pos.y + offy;
            z = element.object.level;
            angz = element.object.angle;
            skip_north = true;
            hasSymbpos = true;
            break;
        case API_ZoneID:
            x = element.zone.pos.x + offx;
            y = element.zone.pos.y + offy;
            skip_north = true;
            hasSymbpos = true;
            break;
        case API_ColumnID:
            x = element.column.origoPos.x + offx;
            y = element.column.origoPos.y + offy;
            slantDirectionAngle = element.column.slantDirectionAngle;
            #ifdef AC_22
            axisRotationAngle = element.column.angle;
            #else
            axisRotationAngle = element.column.axisRotationAngle;
            #endif
            if (slantDirectionAngle < 0) slantDirectionAngle = 2 * PI + slantDirectionAngle;
            if (axisRotationAngle < 0) axisRotationAngle = 2 * PI + axisRotationAngle;
            angz = slantDirectionAngle + axisRotationAngle;
            hasSymbpos = true;
            break;
        case API_WallID:
            sx = element.wall.begC.x + offx;
            sy = element.wall.begC.y + offy;
            ex = element.wall.endC.x + offx;
            ey = element.wall.endC.y + offy;
            isFliped = element.wall.flipped;
            hasLine = true;
            break;
        case API_BeamID:
            sx = element.beam.begC.x + offx;
            sy = element.beam.begC.y + offy;
            ex = element.beam.endC.x + offx;
            ey = element.beam.endC.y + offy;
            isFliped = element.beam.isFlipped;
            hasLine = true;
            break;
        default:
            x = 0;
            y = 0;
            z = 0;
            sx = 0;
            sy = 0;
            ex = 0;
            ey = 0;
            angz = 0;
            hasLine = true;
            hasSymbpos = true;
            skip_north = true;
            bsync_coord_correct = false;
            break;
    }
    if (eltype == API_ColumnID) {
        double k = 100000.0;
        if (fabs (slantDirectionAngle) > 0.0000001) { slantDirectionAngle = fmod (round ((slantDirectionAngle * 180.0 / PI) * k) / k, 360.0); } else {
            slantDirectionAngle = 0.0;
        }
        if (fabs (axisRotationAngle) > 0.0000001) {
            slantDirectionAngle = fmod (round ((axisRotationAngle * 180.0 / PI) * k) / k, 360.0);
        } else {
            axisRotationAngle = 0.0;
        }
        ParamHelpers::AddDoubleValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_rotangle_slant", slantDirectionAngle);
        bsymb_rotangle_correct = CoordCorrectAngle (slantDirectionAngle, tolerance_ang, symb_rotangle_fraction, bsymb_rotangle_correct_1000);
        ParamHelpers::AddDoubleValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_rotangle_slant_fraction", symb_rotangle_fraction);
        ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_rotangle_slant_correct", bsymb_rotangle_correct);
        ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_rotangle_slant_correct_1000", bsymb_rotangle_correct_1000);
        ParamHelpers::AddDoubleValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_rotangle_axis", axisRotationAngle);
        bsymb_rotangle_correct = CoordCorrectAngle (slantDirectionAngle, tolerance_ang, symb_rotangle_fraction, bsymb_rotangle_correct_1000);
        ParamHelpers::AddDoubleValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_rotangle_axis_fraction", symb_rotangle_fraction);
        ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_rotangle_axis_correct", bsymb_rotangle_correct);
        ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_rotangle_axis_correct_1000", bsymb_rotangle_correct_1000);
    } else {
        ParamHelpers::AddDoubleValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_rotangle_slant", 0);
        ParamHelpers::AddDoubleValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_rotangle_slant_fraction", true);
        ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_rotangle_slant_correct", true);
        ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_rotangle_slant_correct_1000", true);
        ParamHelpers::AddDoubleValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_rotangle_axis", 0);
        ParamHelpers::AddDoubleValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_rotangle_axis_fraction", true);
        ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_rotangle_axis_correct", true);
        ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_rotangle_axis_correct_1000", true);
    }
    if (hasSymbpos) {
        if (fabs (angz) > 0.0000001) {
            double k = 100000.0;
            angz = fmod (round ((angz * 180.0 / PI) * k) / k, 360.0);
        } else {
            angz = 0.0;
        }
        xu = x - lox; yu = y - loy; zu = z - loz;
        if (eltype == API_WindowID || eltype == API_DoorID) {
            yu = 0; zu = 0;
            if (isFliped) {
                dx = ex - sx;
                dy = ey - sy;
            } else {
                dx = sx - ex;
                dy = sy - ey;
            }
            double l_wall = sqrt (dx * dx + dy * dy);
            double koeff = x / l_wall;
            double swx = sx + (ex - sx) * koeff; double swy = sy + (ey - sy) * koeff; //Абсолютные координаты середины проёма
            double swx_lo = swx - lox; double swy_lo = swy - loy; //Координаты относительно ПН середины проёма
            bool windoor_in_wall = true;
            if (x + ww / 2 < tolerance_coord_hard) windoor_in_wall = false;
            if (x - ww / 2 - l_wall > tolerance_coord_hard) windoor_in_wall = false;
            if (z + hw < tolerance_coord_hard) windoor_in_wall = false;
            if (z - zw > tolerance_coord_hard) windoor_in_wall = false;
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "windoor_in_wall", windoor_in_wall);
            ParamHelpers::AddLengthValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_sx", swx);
            ParamHelpers::AddLengthValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_sy", swy);
            ParamHelpers::AddLengthValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_sx", swx_lo);
            ParamHelpers::AddLengthValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_sy", swy_lo);
        } else {
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "windoor_in_wall", true);
        }
        ParamHelpers::AddLengthValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_x", x);
        ParamHelpers::AddLengthValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_y", y);
        ParamHelpers::AddLengthValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_z", z);
        ParamHelpers::AddLengthValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_x", xu);
        ParamHelpers::AddLengthValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_y", yu);
        ParamHelpers::AddLengthValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_z", zu);
        if (bsync_coord_correct) {
            bool bsymb_pos_x_correct = check_accuracy (x, tolerance_coord);
            bool bsymb_pos_y_correct = check_accuracy (y, tolerance_coord);
            bool bsymb_pos_correct = bsymb_pos_x_correct && bsymb_pos_y_correct;
            bool bsymb_pos_x_correct_hard = check_accuracy (x, tolerance_coord_hard);
            bool bsymb_pos_y_correct_hard = check_accuracy (y, tolerance_coord_hard);
            bool bsymb_pos_correct_hard = bsymb_pos_x_correct_hard && bsymb_pos_y_correct_hard;

            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_x_correct", bsymb_pos_x_correct);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_y_correct", bsymb_pos_y_correct);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_sx_correct", bsymb_pos_x_correct);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_sy_correct", bsymb_pos_y_correct);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_s_correct", bsymb_pos_correct);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_correct", bsymb_pos_correct);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_x_correct_hard", bsymb_pos_x_correct_hard);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_y_correct_hard", bsymb_pos_y_correct_hard);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_sx_correct_hard", bsymb_pos_x_correct_hard);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_sy_correct_hard", bsymb_pos_y_correct_hard);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_s_correct_hard", bsymb_pos_correct_hard);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_correct_hard", bsymb_pos_correct_hard);
            bool bsymb_pos_x_correctu = check_accuracy (xu, tolerance_coord);
            bool bsymb_pos_y_correctu = check_accuracy (yu, tolerance_coord);
            bool bsymb_pos_correctu = bsymb_pos_x_correctu && bsymb_pos_y_correctu;
            bool bsymb_pos_x_correct_hardu = check_accuracy (xu, tolerance_coord_hard);
            bool bsymb_pos_y_correct_hardu = check_accuracy (yu, tolerance_coord_hard);
            bool bsymb_pos_correct_hardu = bsymb_pos_x_correct_hardu && bsymb_pos_y_correct_hardu;
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_x_correct", bsymb_pos_x_correctu);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_y_correct", bsymb_pos_y_correctu);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_sx_correct", bsymb_pos_x_correctu);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_sy_correct", bsymb_pos_y_correctu);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_s_correct", bsymb_pos_correctu);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_correct", bsymb_pos_correctu);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_x_correct_hard", bsymb_pos_x_correct_hardu);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_y_correct_hard", bsymb_pos_y_correct_hardu);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_sx_correct_hard", bsymb_pos_x_correct_hardu);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_sy_correct_hard", bsymb_pos_y_correct_hardu);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_s_correct_hard", bsymb_pos_correct_hardu);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_correct_hard", bsymb_pos_correct_hardu);
        } else {
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_x_correct", true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_y_correct", true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_sx_correct", true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_sy_correct", true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_s_correct", true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_correct", true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_x_correct_hard", true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_y_correct_hard", true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_sx_correct_hard", true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_sy_correct_hard", true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_s_correct_hard", true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_correct_hard", true);

            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_x_correct", true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_y_correct", true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_sx_correct", true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_sy_correct", true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_s_correct", true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_correct", true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_x_correct_hard", true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_y_correct_hard", true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_sx_correct_hard", true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_sy_correct_hard", true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_s_correct_hard", true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_correct_hard", true);
        }
    }
    if (hasLine) CoordRotAngle (sx, sy, ex, ey, isFliped, angz);
    sxu = sx - lox; syu = sy - loy;
    exu = ex - lox; eyu = ey - loy;
    if (hasLine && !hasSymbpos) {
        ParamHelpers::AddLengthValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_x", sx);
        ParamHelpers::AddLengthValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_y", sy);
        ParamHelpers::AddLengthValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_sx", sx);
        ParamHelpers::AddLengthValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_sy", sy);
        ParamHelpers::AddLengthValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_ex", ex);
        ParamHelpers::AddLengthValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_ey", ey);

        ParamHelpers::AddLengthValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_x", sxu);
        ParamHelpers::AddLengthValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_y", syu);
        ParamHelpers::AddLengthValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_sx", sxu);
        ParamHelpers::AddLengthValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_sy", syu);
        ParamHelpers::AddLengthValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_ex", exu);
        ParamHelpers::AddLengthValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_ey", eyu);
        ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "windoor_in_wall", true);
        if (bsync_coord_correct) {
            if (isFliped) {
                dx = ex - sx;
                dy = ey - sy;
            } else {
                dx = sx - ex;
                dy = sy - ey;
            }
            double l = sqrt (dx * dx + dy * dy);
            bool bl_correct = check_accuracy (l, tolerance_coord);
            bool bsymb_pos_sx_correct = check_accuracy (sx, tolerance_coord);
            bool bsymb_pos_sy_correct = check_accuracy (sy, tolerance_coord);
            bool bsymb_pos_ex_correct = check_accuracy (ex, tolerance_coord);
            bool bsymb_pos_ey_correct = check_accuracy (ey, tolerance_coord);
            bool bsymb_pos_e_correct = bsymb_pos_sx_correct && bsymb_pos_sy_correct;
            bool bsymb_pos_s_correct = bsymb_pos_ex_correct && bsymb_pos_ey_correct;
            bool bsymb_pos_correct = bsymb_pos_e_correct && bsymb_pos_s_correct;
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "l_correct", bl_correct);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_s_correct", bsymb_pos_s_correct);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_e_correct", bsymb_pos_e_correct);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_x_correct", bsymb_pos_sx_correct);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_y_correct", bsymb_pos_sy_correct);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_sx_correct", bsymb_pos_sx_correct);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_sy_correct", bsymb_pos_sy_correct);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_ex_correct", bsymb_pos_ex_correct);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_ey_correct", bsymb_pos_ey_correct);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_correct", bsymb_pos_correct);

            bool bsymb_pos_sx_correctu = check_accuracy (sxu, tolerance_coord);
            bool bsymb_pos_sy_correctu = check_accuracy (syu, tolerance_coord);
            bool bsymb_pos_ex_correctu = check_accuracy (exu, tolerance_coord);
            bool bsymb_pos_ey_correctu = check_accuracy (eyu, tolerance_coord);
            bool bsymb_pos_e_correctu = bsymb_pos_sx_correctu && bsymb_pos_sy_correctu;
            bool bsymb_pos_s_correctu = bsymb_pos_ex_correctu && bsymb_pos_ey_correctu;
            bool bsymb_pos_correctu = bsymb_pos_e_correctu && bsymb_pos_s_correctu;
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_s_correct", bsymb_pos_s_correctu);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_e_correct", bsymb_pos_e_correctu);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_x_correct", bsymb_pos_sx_correctu);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_y_correct", bsymb_pos_sy_correctu);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_sx_correct", bsymb_pos_sx_correctu);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_sy_correct", bsymb_pos_sy_correctu);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_ex_correct", bsymb_pos_ex_correctu);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_ey_correct", bsymb_pos_ey_correctu);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_correct", bsymb_pos_correctu);

            bool bl_correct_hard = check_accuracy (l, tolerance_coord_hard);
            bool bsymb_pos_sx_correct_hard = check_accuracy (sx, tolerance_coord_hard);
            bool bsymb_pos_sy_correct_hard = check_accuracy (sy, tolerance_coord_hard);
            bool bsymb_pos_ex_correct_hard = check_accuracy (ex, tolerance_coord_hard);
            bool bsymb_pos_ey_correct_hard = check_accuracy (ey, tolerance_coord_hard);
            bool bsymb_pos_e_correct_hard = bsymb_pos_sx_correct_hard && bsymb_pos_sy_correct_hard;
            bool bsymb_pos_s_correct_hard = bsymb_pos_ex_correct_hard && bsymb_pos_ey_correct_hard;
            bool bsymb_pos_correct_hard = bsymb_pos_e_correct_hard && bsymb_pos_s_correct_hard;
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "l_correct_hard", bl_correct);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_s_correct_hard", bsymb_pos_s_correct_hard);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_e_correct_hard", bsymb_pos_e_correct_hard);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_x_correct_hard", bsymb_pos_sx_correct_hard);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_y_correct_hard", bsymb_pos_sy_correct_hard);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_sx_correct_hard", bsymb_pos_sx_correct_hard);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_sy_correct_hard", bsymb_pos_sy_correct_hard);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_ex_correct_hard", bsymb_pos_ex_correct_hard);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_ey_correct_hard", bsymb_pos_ey_correct_hard);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_correct_hard", bsymb_pos_correct_hard);
            bool bsymb_pos_sx_correctu_hard = check_accuracy (sxu, tolerance_coord_hard);
            bool bsymb_pos_sy_correctu_hard = check_accuracy (syu, tolerance_coord_hard);
            bool bsymb_pos_ex_correctu_hard = check_accuracy (exu, tolerance_coord_hard);
            bool bsymb_pos_ey_correctu_hard = check_accuracy (eyu, tolerance_coord_hard);
            bool bsymb_pos_e_correctu_hard = bsymb_pos_sx_correctu_hard && bsymb_pos_sy_correctu_hard;
            bool bsymb_pos_s_correctu_hard = bsymb_pos_ex_correctu_hard && bsymb_pos_ey_correctu_hard;
            bool bsymb_pos_correctu_hard = bsymb_pos_e_correctu_hard && bsymb_pos_s_correctu_hard;
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_s_correct_hard", bsymb_pos_s_correctu_hard);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_e_correct_hard", bsymb_pos_e_correctu_hard);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_x_correct_hard", bsymb_pos_sx_correctu_hard);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_y_correct_hard", bsymb_pos_sy_correctu_hard);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_sx_correct_hard", bsymb_pos_sx_correctu_hard);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_sy_correct_hard", bsymb_pos_sy_correctu_hard);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_ex_correct_hard", bsymb_pos_ex_correctu_hard);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_ey_correct_hard", bsymb_pos_ey_correctu_hard);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_correct_hard", bsymb_pos_correctu_hard);
        } else {
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "l_correct", true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_s_correct", true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_e_correct", true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_x_correct", true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_y_correct", true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_sx_correct", true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_sy_correct", true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_ex_correct", true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_ey_correct", true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_correct", true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "l_correct_hard", true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_s_correct_hard", true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_e_correct_hard", true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_x_correct_hard", true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_y_correct_hard", true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_sx_correct_hard", true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_sy_correct_hard", true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_ex_correct_hard", true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_ey_correct_hard", true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_correct_hard", true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_s_correct", true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_e_correct", true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_x_correct", true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_y_correct", true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_sx_correct", true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_sy_correct", true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_ex_correct", true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_ey_correct", true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_correct", true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_s_correct_hard", true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_e_correct_hard", true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_x_correct_hard", true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_y_correct_hard", true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_sx_correct_hard", true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_sy_correct_hard", true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_ex_correct_hard", true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_ey_correct_hard", true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_pos_lo_correct_hard", true);
        }
    }

    if (!skip_north) CoordNorthAngle (north, angz, angznorth, angznorthtxt, angznorthtxteng);
    ParamHelpers::AddDoubleValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "north_dir", angznorth);
    ParamHelpers::AddStringValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "north_dir_str", angznorthtxt);
    ParamHelpers::AddStringValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "north_dir_eng", angznorthtxteng);
    ParamHelpers::AddDoubleValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_rotangle", angz);
    ParamHelpers::AddDoubleValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_rotangle_mod5", fmod (angz, 5.0));
    ParamHelpers::AddDoubleValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_rotangle_mod10", fmod (angz, 10.0));
    ParamHelpers::AddDoubleValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_rotangle_mod45", fmod (angz, 45.0));
    ParamHelpers::AddDoubleValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_rotangle_mod90", fmod (angz, 90.0));
    ParamHelpers::AddDoubleValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_rotangle_mod180", fmod (angz, 180.0));
    if (bsync_coord_correct) {
        bsymb_rotangle_correct = CoordCorrectAngle (angz, tolerance_ang, symb_rotangle_fraction, bsymb_rotangle_correct_1000);
        ParamHelpers::AddDoubleValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_rotangle_fraction", symb_rotangle_fraction);
        ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_rotangle_correct", bsymb_rotangle_correct);
        ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_rotangle_correct_1000", bsymb_rotangle_correct_1000);
    } else {
        ParamHelpers::AddDoubleValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_rotangle_fraction", 0.0);
        ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_rotangle_correct", true);
        ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord, element.header.guid, "coord:", "symb_rotangle_correct_1000", true);
    }
    ParamHelpers::CompareParamDictValue (pdictvaluecoord, params);
    return true;
}

// -----------------------------------------------------------------------------
// Проверяет наличие дробной части у угла с заданной точностью
// -----------------------------------------------------------------------------
bool CoordCorrectAngle (double angz, double& tolerance_ang, double& symb_rotangle_fraction, bool& bsymb_rotangle_correct_1000)
{
    symb_rotangle_fraction = 1000.0 * fabs (fabs (angz) - floor (fabs (angz))) / tolerance_ang;
    double angz_ = angz / 1000.0;
    bool bsymb_rotangle_correct = check_accuracy (angz_, tolerance_ang);
    bsymb_rotangle_correct_1000 = check_accuracy (angz_, 0.001);
    return bsymb_rotangle_correct;
}

// -----------------------------------------------------------------------------
// По заданному углу поворота и глобальному углу направления на север возвращает ориентацию объекта
// и текст с обозначением стороны света (RUS+ENG)
// -----------------------------------------------------------------------------
void CoordNorthAngle (double north, double angz, double& angznorth, GS::UniString& angznorthtxt, GS::UniString& angznorthtxteng)
{
    double k = 100000.0;
    angznorth = fmod (angz - north + 90.0, 360.0);
    if (fabs (angznorth) < 0.0000001) {
        angznorth = 0.0;
    } else {
        if (angznorth < 0.0) angznorth = 360.0 + angznorth;
        angznorth = round (angznorth * k) / k;
    }
    double n = 0.0;		//"С"
    double nw = 45.0;	//"СЗ"
    double w = 90.0;	//"З"
    double sw = 135.0;	//"ЮЗ"
    double s = 180.0;	//"Ю"
    double se = 225.0;	//"ЮВ"
    double e = 270.0;	//"В";
    double ne = 315.0;	//"СB";
    double nn = 360.0;
    if (angznorth > nn - 22.5 || angznorth < n + 22.5) angznorthtxt = RSGetIndString (ID_ADDON_STRINGS + isEng (), N_StringID, ACAPI_GetOwnResModule ());
    if (angznorth > ne - 22.5 && angznorth < ne + 22.5) angznorthtxt = RSGetIndString (ID_ADDON_STRINGS + isEng (), NE_StringID, ACAPI_GetOwnResModule ());
    if (angznorth > e - 22.5 && angznorth < e + 22.5) angznorthtxt = RSGetIndString (ID_ADDON_STRINGS + isEng (), E_StringID, ACAPI_GetOwnResModule ());
    if (angznorth > se - 22.5 && angznorth < se + 22.5) angznorthtxt = RSGetIndString (ID_ADDON_STRINGS + isEng (), SE_StringID, ACAPI_GetOwnResModule ());
    if (angznorth > s - 22.5 && angznorth < s + 22.5) angznorthtxt = RSGetIndString (ID_ADDON_STRINGS + isEng (), S_StringID, ACAPI_GetOwnResModule ());
    if (angznorth > sw - 22.5 && angznorth < sw + 22.5) angznorthtxt = RSGetIndString (ID_ADDON_STRINGS + isEng (), SW_StringID, ACAPI_GetOwnResModule ());
    if (angznorth > w - 22.5 && angznorth < w + 22.5) angznorthtxt = RSGetIndString (ID_ADDON_STRINGS + isEng (), W_StringID, ACAPI_GetOwnResModule ());
    if (angznorth > nw - 22.5 && angznorth < nw + 22.5) angznorthtxt = RSGetIndString (ID_ADDON_STRINGS + isEng (), NW_StringID, ACAPI_GetOwnResModule ());
    if (is_equal (angznorth, n + 22.5)) angznorthtxt = RSGetIndString (ID_ADDON_STRINGS + isEng (), N_StringID, ACAPI_GetOwnResModule ());
    if (is_equal (angznorth, ne + 22.5)) angznorthtxt = RSGetIndString (ID_ADDON_STRINGS + isEng (), NE_StringID, ACAPI_GetOwnResModule ());
    if (is_equal (angznorth, e + 22.5)) angznorthtxt = RSGetIndString (ID_ADDON_STRINGS + isEng (), E_StringID, ACAPI_GetOwnResModule ());
    if (is_equal (angznorth, se + 22.5)) angznorthtxt = RSGetIndString (ID_ADDON_STRINGS + isEng (), SE_StringID, ACAPI_GetOwnResModule ());
    if (is_equal (angznorth, s + 22.5)) angznorthtxt = RSGetIndString (ID_ADDON_STRINGS + isEng (), S_StringID, ACAPI_GetOwnResModule ());
    if (is_equal (angznorth, sw + 22.5)) angznorthtxt = RSGetIndString (ID_ADDON_STRINGS + isEng (), SW_StringID, ACAPI_GetOwnResModule ());
    if (is_equal (angznorth, w + 22.5)) angznorthtxt = RSGetIndString (ID_ADDON_STRINGS + isEng (), W_StringID, ACAPI_GetOwnResModule ());
    if (is_equal (angznorth, nn - 22.5)) angznorthtxt = RSGetIndString (ID_ADDON_STRINGS + isEng (), NW_StringID, ACAPI_GetOwnResModule ());

    if (angznorth > nn - 22.5 || angznorth < n + 22.5) angznorthtxteng = "N";
    if (angznorth > ne - 22.5 && angznorth < ne + 22.5) angznorthtxteng = "NE";
    if (angznorth > e - 22.5 && angznorth < e + 22.5) angznorthtxteng = "E";
    if (angznorth > se - 22.5 && angznorth < se + 22.5) angznorthtxteng = "SE";
    if (angznorth > s - 22.5 && angznorth < s + 22.5) angznorthtxteng = "S";
    if (angznorth > sw - 22.5 && angznorth < sw + 22.5) angznorthtxteng = "SW";
    if (angznorth > w - 22.5 && angznorth < w + 22.5) angznorthtxteng = "W";
    if (angznorth > nw - 22.5 && angznorth < nw + 22.5) angznorthtxteng = "NW";
    if (is_equal (angznorth, n + 22.5)) angznorthtxteng = "N";
    if (is_equal (angznorth, ne + 22.5)) angznorthtxteng = "NE";
    if (is_equal (angznorth, e + 22.5)) angznorthtxteng = "E";
    if (is_equal (angznorth, se + 22.5)) angznorthtxteng = "SE";
    if (is_equal (angznorth, s + 22.5)) angznorthtxteng = "S";
    if (is_equal (angznorth, sw + 22.5)) angznorthtxteng = "SW";
    if (is_equal (angznorth, w + 22.5)) angznorthtxteng = "W";
    if (is_equal (angznorth, nn - 22.5)) angznorthtxteng = "NW";
}

// -----------------------------------------------------------------------------
// Вычисляет уголв поворота элемента по координатам его начала и конца
// -----------------------------------------------------------------------------
void CoordRotAngle (double sx, double sy, double ex, double ey, bool isFliped, double& angz)
{
    double k = 1000000000.0;
    double dx; double dy;
    if (isFliped) {
        dx = ex - sx;
        dy = ey - sy;
    } else {
        dx = sx - ex;
        dy = sy - ey;
    }
    if (is_equal (dx, 0.0) && is_equal (dy, 0.0)) {
        angz = 0.0;
    } else {
        angz = atan2 (dy, dx) + PI;
    }
    if (fabs (angz) > 0.000000001) {
        angz = fmod (round ((angz * 180.0 / PI) * k) / k, 360.0);
    } else {
        angz = 0.0;
    }
}

// -----------------------------------------------------------------------------
// Получение имени внутренних свойств по русскому имени
// -----------------------------------------------------------------------------
GS::UniString GetPropertyENGName (GS::UniString& name)
{

    if (!name.Contains ("@property:")) return name;
    if (name.Contains ("@property:sync_name")) return name;
    if (name.IsEqual ("@property:id")) return "@property:BuildingMaterialProperties/Building Material ID";
    if (name.IsEqual ("@property:n")) return "@material:n";
    if (name.IsEqual ("@property:ns")) return "@material:ns";
    if (name.IsEqual ("@property:layer_thickness")) return "@material:layer thickness";
    if (name.IsEqual ("@property:th")) return "@material:layer thickness";
    if (name.IsEqual ("@property:bmat_inx")) return "@material:bmat_inx";
    if (name.IsEqual ("@property:cutfill_inx")) return "@material:cutfill_inx";
    if (name.IsEqual ("@property:some_stuff_th")) return "@property:buildingmaterialproperties/some_stuff_th";
    if (name.IsEqual ("@property:some_stuff_units")) return "@property:buildingmaterialproperties/some_stuff_units";
    if (name.IsEqual ("@property:unit")) return "@property:buildingmaterialproperties/some_stuff_units";
    if (name.IsEqual ("@property:kzap")) return "@property:buildingmaterialproperties/some_stuff_kzap";
    if (name.IsEqual ("@property:area")) return "@material:area";
    if (name.IsEqual ("@property:volume")) return "@material:volume";
    if (name.IsEqual ("@property:qty")) return "@material:qty";
    if (name.IsEqual ("@property:unit_prefix")) return "@material:unit_prefix";
    if (name.IsEqual ("@property:length")) return "@material:length";
    GS::UniString nameproperty = "";
    const Int32 iseng = ID_ADDON_STRINGS + isEng ();
    nameproperty = "@property:" + RSGetIndString (iseng, BuildingMaterialNameID, ACAPI_GetOwnResModule ());
    if (name.IsEqual (nameproperty)) {
        return "@property:BuildingMaterialProperties/Building Material Name";
    }

    nameproperty = "@property:" + RSGetIndString (iseng, BuildingMaterialDescriptionID, ACAPI_GetOwnResModule ());
    if (name.IsEqual (nameproperty)) {
        return "@property:BuildingMaterialProperties/Building Material Description";
    }

    nameproperty = "@property:" + RSGetIndString (iseng, BuildingMaterialDensityID, ACAPI_GetOwnResModule ());
    if (name.IsEqual (nameproperty)) {
        return "@property:BuildingMaterialProperties/Building Material Density";
    }

    nameproperty = "@property:" + RSGetIndString (iseng, BuildingMaterialManufacturerID, ACAPI_GetOwnResModule ());
    if (name.IsEqual (nameproperty)) {
        return "@property:BuildingMaterialProperties/Building Material Manufacturer";
    }

    nameproperty = "@property:" + RSGetIndString (iseng, BuildingMaterialCutFillID, ACAPI_GetOwnResModule ());
    if (name.IsEqual (nameproperty)) {
        return "@property:BuildingMaterialProperties/Building Material CutFill";
    }

    nameproperty = "@property:" + RSGetIndString (iseng, ThicknessID, ACAPI_GetOwnResModule ());
    if (name.IsEqual (nameproperty)) {
        return "@material:layer thickness";
    }
    return name;
}

// -----------------------------------------------------------------------------
// Извлекает из строки все имена свойств или параметров, заключенные в знаки %
// -----------------------------------------------------------------------------
bool ParamHelpers::ParseParamNameMaterial (GS::UniString& expression, ParamDictValue& paramDict, bool fromMaterial)
{
    GS::UniString part = "";
    bool flag_change = true;
    while (expression.Count ('%') > 1 && flag_change) {
        GS::UniString expression_old = expression;
        part = expression.GetSubstring ('%', '%', 0);
        if (!part.IsEmpty ()) {
            if (fromMaterial) {
                expression.ReplaceAll ('%' + part + '%', "{@property:" + part.ToLowerCase () + '}');
            } else {
                expression.ReplaceAll ('%' + part + '%', "{" + part.ToLowerCase () + '}');
            }
        }
        if (expression_old.IsEqual (expression)) flag_change = false;
    }
    return ParamHelpers::ParseParamName (expression, paramDict);
}

// -----------------------------------------------------------------------------
// Извлекает из строки все имена свойств или параметров, заключенные в знаки { }
// -----------------------------------------------------------------------------
bool ParamHelpers::ParseParamName (GS::UniString& expression, ParamDictValue& paramDict)
{
    GS::UniString tempstring = expression;
    if (!tempstring.Contains ('{')) return false;
    GS::UniString part = "";
    bool flag_change = true;
    while (tempstring.Contains ('{') && tempstring.Contains ('}') && flag_change) {
        GS::UniString expression_old = tempstring;
        part = tempstring.GetSubstring ('{', '}', 0);
        GS::UniString part_clean = part;
        // TODO Переписать всю эту хреноту - отделить парсинг от добавления в словарь
        FormatString formatstring;
        GS::UniString part_ = ParamHelpers::NameToRawName (part_clean, formatstring);
        if (!paramDict.ContainsKey (part_)) {
            ParamValue pvalue;
            GS::UniString name_ = part_clean.ToLowerCase ();
            pvalue.rawName = part_;
            if (part_.Contains ("{@property:")) pvalue.fromPropertyDefinition = true;
            if (part_.Contains ("{@coord:")) pvalue.fromCoord = true;
            if (part_.Contains ("{@gdl:")) pvalue.fromGDLparam = true;
            if (part_.Contains ("{@info:")) pvalue.fromInfo = true;
            if (part_.Contains ("{@ifc:")) pvalue.fromIFCProperty = true;
            if (part_.Contains ("{@morph:")) pvalue.fromMorph = true;
            if (part_.Contains ("{@material:")) pvalue.fromMaterial = true;
            if (part_.Contains ("{@glob:")) pvalue.fromGlob = true;
            if (part_.Contains ("{@id:")) pvalue.fromID = true;
            if (part_.Contains ("{@class:")) pvalue.fromClassification = true;
            if (part_.Contains ("{@formula:")) pvalue.val.hasFormula = true;
            if (part_.Contains ("{@element:")) pvalue.fromElement = true;
            if (part_.Contains ("{@mep:")) pvalue.fromMEP = true;
            if (part_.Contains ("{@attrib:")) pvalue.fromAttribElement = true;
            if (part_.Contains ("{@listdata:")) pvalue.fromListData = true;
            pvalue.name = name_;
            pvalue.val.formatstring = formatstring;
            paramDict.Add (part_, pvalue);
        }
        if (!formatstring.isEmpty) {
            part_.ReplaceAll ("}", "." + formatstring.stringformat + "}");
        }
        expression.ReplaceAll ('{' + part + '}', part_);
        tempstring.ReplaceAll ('{' + part + '}', "");
        if (expression_old.IsEqual (tempstring)) {
            flag_change = false;
        }
    }
    return true;
}

// -----------------------------------------------------------------------------
// Замена имен параметров на значения в выражении
// Значения передаются словарём, вычисление значений см. GetParamValueDict
// -----------------------------------------------------------------------------
bool ParamHelpers::ReplaceParamInExpression (const ParamDictValue& pdictvalue, GS::UniString& expression)
{
    if (pdictvalue.IsEmpty ()) return false;
    if (expression.IsEmpty ()) return false;
    if (!expression.Contains ('{')) return true;
    bool flag_find = false;
    GS::UniString attribsuffix = "";
    GS::UniString attribsuffix_old = "";
    GS::UniString part = "";
    GS::UniString part_clean = "";
    GS::UniString partc = "";
    GS::UniString val = "";
    GS::UniString findstring = "";
    bool flag_change = true;
    while (expression.Contains ('{') && expression.Contains ('}') && flag_change) {
        val.Clear ();
        GS::UniString expression_old = expression;
        // Выделяем часть, являющуюся шаблоном параметра
        part = expression.GetSubstring ('{', '}', 0);
        if (!part.IsEmpty ()) {
            part_clean = part;
            // Поищем в ней указание на номер аттрибута
            attribsuffix = "";
            if (part.Contains (CharENTER)) {
                UIndex attribinx = part.FindLast (CharENTER);
                USize partlen = part_clean.GetLength () - attribinx;
                attribsuffix = part_clean.GetSubstring (attribinx, partlen) + '}';
                part_clean = part_clean.GetSubstring (0, attribinx);
            }
            // Проверяем наличие строки формата
            FormatString formatstring;
            GS::UniString stringformat = FormatStringFunc::GetFormatString (part_clean);
            if (!stringformat.IsEmpty ()) formatstring = FormatStringFunc::ParseFormatString (stringformat);
            bool flag = false;
            if (!flag) {
                partc = "{"; // Строка без формата
                partc.Append (part_clean);
                partc.Append ("}");
                if (pdictvalue.ContainsKey (partc)) {
                    ParamValue pvalue = pdictvalue.Get (partc);
                    if (pvalue.isValid) {
                        if (!formatstring.isEmpty) pvalue.val.formatstring = formatstring;
                        val = ParamHelpers::ToString (pvalue);
                        flag_find = true;
                        flag = true;
                    }
                }
            }
            if (!flag && !attribsuffix.IsEmpty ()) {
                partc = "{"; // Строка без формата
                partc.Append (part_clean);
                partc.Append (attribsuffix);
                if (pdictvalue.ContainsKey (partc)) {
                    ParamValue pvalue = pdictvalue.Get (partc);
                    if (pvalue.isValid) {
                        if (!formatstring.isEmpty) pvalue.val.formatstring = formatstring;
                        val = ParamHelpers::ToString (pvalue);
                        flag_find = true;
                        flag = true;
                    } else {
                        #if defined(TESTING)
                        DBprnt ("ReplaceParamInExpression err pvalue.isValid", partc);
                        #endif
                    }
                }
            }
            if (!flag && !attribsuffix_old.IsEmpty ()) {
                partc = "{"; // Строка без формата
                partc.Append (part_clean);
                partc.Append (attribsuffix_old);
                if (pdictvalue.ContainsKey (partc)) {
                    ParamValue pvalue = pdictvalue.Get (partc);
                    if (pvalue.isValid) {
                        if (!formatstring.isEmpty) pvalue.val.formatstring = formatstring;
                        val = ParamHelpers::ToString (pvalue);
                        flag_find = true;
                        flag = true;
                    } else {
                        #if defined(TESTING)
                        DBprnt ("ReplaceParamInExpression err pvalue.isValid", partc);
                        #endif
                    }
                }
            }
            if (!flag) {
                #if defined(TESTING)
                DBprnt ("ReplaceParamInExpression err not found parametr", partc);
                #endif
            }
        }
        findstring = "{";
        findstring.Append (part);
        findstring.Append ("}");
        expression.ReplaceAll (findstring, val);
        if (expression_old.IsEqual (expression)) flag_change = false;
        if (!attribsuffix.IsEmpty ()) attribsuffix_old = attribsuffix;
    }
    return flag_find;
}


bool ParamHelpers::GetParamValueForElements (const API_Guid& elemguid, const GS::UniString& rawname, const ParamDictElement& paramToRead, ParamValue& pvalue)
{
    if (!paramToRead.ContainsKey (elemguid)) return false;
    if (!paramToRead.Get (elemguid).ContainsKey (rawname)) return false;
    pvalue = paramToRead.Get (elemguid).Get (rawname);
    return pvalue.isValid;
}

// -----------------------------------------------------------------------------
// Список возможных префиксов полных имён параметров
// -----------------------------------------------------------------------------
void ParamHelpers::GetParamTypeList (GS::Array<GS::UniString>& paramTypesList)
{
    if (!paramTypesList.IsEmpty ()) paramTypesList.Clear ();
    paramTypesList.Push ("{@property:");
    paramTypesList.Push ("{@coord:");
    paramTypesList.Push ("{@gdl:");
    paramTypesList.Push ("{@info:");
    paramTypesList.Push ("{@ifc:");
    paramTypesList.Push ("{@morph:");
    paramTypesList.Push ("{@attrib:");
    paramTypesList.Push ("{@material:");
    paramTypesList.Push ("{@glob:");
    paramTypesList.Push ("{@id:");
    paramTypesList.Push ("{@class:");
    paramTypesList.Push ("{@formula:");
    paramTypesList.Push ("{@element:");
    paramTypesList.Push ("{@mep:");
    paramTypesList.Push ("{@listdata:");
}

GS::UniString PropertyHelpers::ToString (const API_Variant& variant, const FormatString& stringformat)
{
    switch (variant.type) {
        case API_PropertyIntegerValueType: return  FormatStringFunc::NumToString (variant.intValue, stringformat);
        case API_PropertyRealValueType: return FormatStringFunc::NumToString (variant.doubleValue, stringformat);
        case API_PropertyStringValueType: return variant.uniStringValue;
        case API_PropertyBooleanValueType: return GS::ValueToUniString (variant.boolValue);
        case API_PropertyGuidValueType: return APIGuid2GSGuid (variant.guidValue).ToUniString ();
        case API_PropertyUndefinedValueType:
            return "@Undefined Value@";
        default: DBBREAK ();
            return "@Invalid Value@";
    }
}

GS::UniString PropertyHelpers::ToString (const API_Variant& variant)
{
    FormatString f;
    return PropertyHelpers::ToString (variant, f);
}

GS::UniString PropertyHelpers::ToString (const API_Property& property)
{
    FormatString f;
    return PropertyHelpers::ToString (property, f);
}

GS::UniString PropertyHelpers::ToString (const API_Property& property, const FormatString& stringformat)
{
    GS::UniString string;
    const API_PropertyValue* value;
    #if defined(AC_22) || defined(AC_23)
    if (!property.isEvaluated) {
        return string;
    }
    if (property.isDefault && !property.isEvaluated) {
        value = &property.definition.defaultValue.basicValue;
    } else {
        value = &property.value;
    }
    #else
    if (property.status == API_Property_NotAvailable) {
        return string;
    }
    if (property.isDefault && property.status == API_Property_NotEvaluated) {
        value = &property.definition.defaultValue.basicValue;
    } else {
        value = &property.value;
    }
    #endif
    switch (property.definition.collectionType) {
        case API_PropertySingleCollectionType:
            {
                string += ToString (value->singleVariant.variant, stringformat);
            } break;
        case API_PropertyListCollectionType:
            {
                for (UInt32 i = 0; i < value->listVariant.variants.GetSize (); i++) {
                    string.Append (ToString (value->listVariant.variants[i], stringformat));
                    if (i != value->listVariant.variants.GetSize () - 1) {
                        string.Append ("; ");
                    }
                }
            } break;
        case API_PropertySingleChoiceEnumerationCollectionType:
            {
                #if defined(AC_25) || defined(AC_26) || defined(AC_27) || defined(AC_28)
                API_Guid guidValue = value->singleVariant.variant.guidValue;
                GS::Array<API_SingleEnumerationVariant> possibleEnumValues = property.definition.possibleEnumValues;
                for (UInt32 i = 0; i < possibleEnumValues.GetSize (); i++) {
                    if (possibleEnumValues[i].keyVariant.guidValue == guidValue) {
                        string.Append (ToString (possibleEnumValues[i].displayVariant, stringformat));
                        break;
                    }
                }
                #else // AC_25
                string += ToString (value->singleEnumVariant.displayVariant, stringformat);
                #endif
            } break;
        case API_PropertyMultipleChoiceEnumerationCollectionType:
            {
                #if defined(AC_25) || defined(AC_26) || defined(AC_27) || defined(AC_28)
                GS::Array<API_SingleEnumerationVariant> possibleEnumValues = property.definition.possibleEnumValues;
                UInt32 qty_finded_values = value->listVariant.variants.GetSize ();
                for (UInt32 i = 0; i < possibleEnumValues.GetSize (); i++) {
                    API_Guid guidValue = possibleEnumValues[i].keyVariant.guidValue;
                    for (UInt32 j = 0; j < value->listVariant.variants.GetSize (); j++) {
                        if (value->listVariant.variants[j].guidValue == guidValue) {
                            string.Append (ToString (possibleEnumValues[i].displayVariant, stringformat));
                            qty_finded_values = qty_finded_values - 1;
                            if (qty_finded_values != 0) string.Append ("; ");
                            break;
                        }
                    }
                    if (qty_finded_values == 0) break;
                }
                #else // AC_25
                for (UInt32 i = 0; i < value->multipleEnumVariant.variants.GetSize (); i++) {
                    string += ToString (value->multipleEnumVariant.variants[i].displayVariant, stringformat);
                    if (i != value->multipleEnumVariant.variants.GetSize () - 1) {
                        string.Append ("; ");
                    }
                }
                #endif
            } break;
        default:
            {
                break;
            }
    }
    return string;
}

ParamValueData operator+ (const ParamValueData& lhs, const ParamValueData& rhs)
{
    ParamValueData out = lhs;
    if (!lhs.hasrawDouble || !rhs.hasrawDouble) out.hasrawDouble = false;
    if (lhs.hasrawDouble && rhs.hasrawDouble) out.hasrawDouble = true;
    out.doubleValue = lhs.doubleValue + rhs.doubleValue;
    if (out.hasrawDouble) out.rawDoubleValue = lhs.rawDoubleValue + rhs.rawDoubleValue;
    out.uniStringValue = lhs.uniStringValue;
    out.uniStringValue.Append (";");
    out.uniStringValue.Append (rhs.uniStringValue);
    out.intValue = lhs.intValue + rhs.intValue;
    return out;
}

bool operator== (const ParamValue& lhs, const ParamValue& rhs)
{
    double lhsd = 0.0;
    double rhsd = 0.0;
    if (!lhs.isValid) return false;
    if (!rhs.isValid) return false;
    switch (rhs.val.type) {
        case API_PropertyIntegerValueType:
            return lhs.val.intValue == rhs.val.intValue;
        case API_PropertyRealValueType:
            rhsd = rhs.val.doubleValue;
            if (!rhs.val.formatstring.needRound && !is_equal (rhs.val.rawDoubleValue, 0) && rhs.val.hasrawDouble) {
                rhsd = rhs.val.rawDoubleValue;
            }
            lhsd = lhs.val.doubleValue;
            if (!lhs.val.formatstring.needRound && !is_equal (lhs.val.rawDoubleValue, 0) && lhs.val.hasrawDouble) {
                lhsd = lhs.val.rawDoubleValue;
            }
            if (lhs.val.type == API_PropertyStringValueType && lhs.val.canCalculate) {
                Int32 n_zero = lhs.val.formatstring.n_zero;
                Int32 krat = lhs.val.formatstring.krat;
                double koeff = lhs.val.formatstring.koeff;
                bool trim_zero = lhs.val.formatstring.trim_zero;
                if (koeff != 1) n_zero = n_zero + (GS::Int32) log10 (koeff);
                lhsd = round (lhsd * pow (10, n_zero)) / pow (10, n_zero);
            }
            return is_equal (lhsd, rhsd);
        case API_PropertyStringValueType:
            return lhs.val.uniStringValue == rhs.val.uniStringValue;
        case API_PropertyBooleanValueType:
            return lhs.val.boolValue == rhs.val.boolValue;
        case API_PropertyGuidValueType:
            return lhs.val.guidval == rhs.val.guidval;
        default:
            return false;
    }
}

bool operator== (const API_Variant& lhs, const API_Variant& rhs)
{
    if (lhs.type != rhs.type) {
        return false;
    }

    switch (lhs.type) {
        case API_PropertyIntegerValueType:
            return lhs.intValue == rhs.intValue;
        case API_PropertyRealValueType:
            return lhs.doubleValue == rhs.doubleValue;
        case API_PropertyStringValueType:
            return lhs.uniStringValue == rhs.uniStringValue;
        case API_PropertyBooleanValueType:
            return lhs.boolValue == rhs.boolValue;
        case API_PropertyGuidValueType:
            return lhs.guidValue == rhs.guidValue;
        default:
            return false;
    }
}

bool operator== (const API_SingleVariant& lhs, const API_SingleVariant& rhs)
{
    return lhs.variant == rhs.variant;
}

bool operator== (const API_ListVariant& lhs, const API_ListVariant& rhs)
{
    return lhs.variants == rhs.variants;
}

bool operator== (const API_SingleEnumerationVariant& lhs, const API_SingleEnumerationVariant& rhs)
{
    return lhs.keyVariant == rhs.keyVariant && lhs.displayVariant == rhs.displayVariant;
}

#if !defined(AC_25) && !defined(AC_26) && !defined(AC_27) && !defined(AC_28)
bool operator== (const API_MultipleEnumerationVariant& lhs, const API_MultipleEnumerationVariant& rhs)
{
    return lhs.variants == rhs.variants;
}
#endif

bool Equals (const API_PropertyDefaultValue& lhs, const API_PropertyDefaultValue& rhs, API_PropertyCollectionType collType)
{
    if (lhs.hasExpression != rhs.hasExpression) {
        return false;
    }

    if (lhs.hasExpression) {
        return lhs.propertyExpressions == rhs.propertyExpressions;
    } else {
        return Equals (lhs.basicValue, rhs.basicValue, collType);
    }
}

bool Equals (const API_PropertyValue& lhs, const API_PropertyValue& rhs, API_PropertyCollectionType collType)
{
    if (lhs.variantStatus != rhs.variantStatus) {
        return false;
    }

    if (lhs.variantStatus != API_VariantStatusNormal) {
        return true;
    }

    switch (collType) {
        case API_PropertySingleCollectionType:
            return lhs.singleVariant == rhs.singleVariant;
        case API_PropertyListCollectionType:
            return lhs.listVariant == rhs.listVariant;
            #if defined(AC_25) || defined(AC_26) || defined(AC_27) || defined(AC_28)
        case API_PropertySingleChoiceEnumerationCollectionType:
            return lhs.singleVariant == rhs.singleVariant;
        case API_PropertyMultipleChoiceEnumerationCollectionType:
            return lhs.listVariant == rhs.listVariant;
            #else
        case API_PropertySingleChoiceEnumerationCollectionType:
            return lhs.singleEnumVariant == rhs.singleEnumVariant;
        case API_PropertyMultipleChoiceEnumerationCollectionType:
            return lhs.multipleEnumVariant == rhs.multipleEnumVariant;
            #endif
        default:
            DBBREAK ();
            return false;
    }
}

bool operator== (const API_PropertyGroup& lhs, const API_PropertyGroup& rhs)
{
    return lhs.guid == rhs.guid &&
        lhs.name == rhs.name;
}

bool operator== (const API_PropertyDefinition& lhs, const API_PropertyDefinition& rhs)
{
    return lhs.guid == rhs.guid &&
        lhs.groupGuid == rhs.groupGuid &&
        lhs.name == rhs.name &&
        lhs.description == rhs.description &&
        lhs.collectionType == rhs.collectionType &&
        lhs.valueType == rhs.valueType &&
        lhs.measureType == rhs.measureType &&
        Equals (lhs.defaultValue, rhs.defaultValue, lhs.collectionType) &&
        lhs.availability == rhs.availability &&
        lhs.possibleEnumValues == rhs.possibleEnumValues;
}

bool operator== (const API_Property& lhs, const API_Property& rhs)
{
    if (lhs.definition != rhs.definition || lhs.isDefault != rhs.isDefault) {
        return false;
    }
    if (!lhs.isDefault) {
        return Equals (lhs.value, rhs.value, lhs.definition.collectionType);
    } else {
        return true;
    }
}

// -----------------------------------------------------------------------------
// Конвертация значений ParamValue в свойства, находящиеся в нём
// Возвращает true если значения отличались
// -----------------------------------------------------------------------------
//bool ParamHelpers::ConvertToProperty (ParamValue& pvalue)
//{
//    if (!pvalue.isValid) return false;
//    if (!pvalue.fromPropertyDefinition) return false;
//    API_Property property = pvalue.property;
//    if (ParamHelpers::ConvertToProperty (pvalue, property)) {
//        pvalue.property = property;
//        return true;
//    } else {
//        return false;
//    }
//}

// -----------------------------------------------------------------------------
// Синхронизация ParamValue и API_Property
// Возвращает true и подготовленное для записи свойство в случае отличий
// -----------------------------------------------------------------------------
bool ParamHelpers::ConvertToProperty (const ParamValue& pvalue, API_Property& property)
{
    if (!property.definition.canValueBeEditable) {
        #if defined(TESTING)
        DBprnt ("ParamHelpers::ConvertToProperty err", "!property.definition.canValueBeEditable");
        #endif
        return false;
    }
    if (!pvalue.isValid) {
        #if defined(TESTING)
        DBprnt ("ParamHelpers::ConvertToProperty err", "!pvalue.isValid");
        #endif
        return false;
    }
    bool flag_rec = false;
    GS::UniString val = "";
    API_PropertyValue value;
    bool isEval = true;
    bool isDefult = false;
    #if defined(AC_22) || defined(AC_23)
    if (property.isDefault && !property.isEvaluated) {
        value = property.definition.defaultValue.basicValue;
        isDefult = true;
    } else {
        value = property.value;
    }
    isEval = property.isEvaluated;
    #else
    if (property.isDefault && property.status == API_Property_NotEvaluated) {
        value = property.definition.defaultValue.basicValue;
        isDefult = true;
    } else {
        value = property.value;
    }
    isEval = (property.status == API_Property_HasValue);
    #endif

    double dval = pvalue.val.doubleValue;
    if (pvalue.val.formatstring.forceRaw && pvalue.val.hasrawDouble) dval = pvalue.val.rawDoubleValue;

    switch (property.definition.valueType) {
        case API_PropertyIntegerValueType:
            if (value.singleVariant.variant.intValue != pvalue.val.intValue || !isEval) {
                property.value.singleVariant.variant.intValue = pvalue.val.intValue;
                flag_rec = true;
            } else {
                if (isDefult) {
                    property.value.singleVariant.variant.intValue = pvalue.val.intValue;
                    flag_rec = true;
                }
            }
            break;
        case API_PropertyRealValueType:
            // Конвертация угла из радиан в градусы
            if (property.definition.measureType == API_PropertyAngleMeasureType) {
                if (!is_equal (dval * PI / 180.0, value.singleVariant.variant.doubleValue) || !isEval) {
                    property.value.singleVariant.variant.doubleValue = dval * PI / 180.0;
                    flag_rec = true;
                } else {
                    if (isDefult) {
                        property.value.singleVariant.variant.doubleValue = dval * PI / 180.0;
                        flag_rec = true;
                    }
                }
            } else {
                if (!is_equal (value.singleVariant.variant.doubleValue, dval) || !isEval) {
                    property.value.singleVariant.variant.doubleValue = dval;
                    flag_rec = true;
                } else {
                    if (isDefult) {
                        property.value.singleVariant.variant.doubleValue = dval;
                        flag_rec = true;
                    }
                }
            }
            break;
        case API_PropertyBooleanValueType:
            if (value.singleVariant.variant.boolValue != pvalue.val.boolValue || !isEval) {
                property.value.singleVariant.variant.boolValue = pvalue.val.boolValue;
                flag_rec = true;
            } else {
                if (isDefult) {
                    property.value.singleVariant.variant.boolValue = pvalue.val.boolValue;
                    flag_rec = true;
                }
            }
            break;
        case API_PropertyStringValueType:
            val = ParamHelpers::ToString (pvalue);
            ReplaceCR (val, true);
            if (value.singleVariant.variant.uniStringValue != val || !isEval) {
                property.value.singleVariant.variant.uniStringValue = val;
                flag_rec = true;
            } else {
                if (isDefult) {
                    property.value.singleVariant.variant.uniStringValue = val;
                    flag_rec = true;
                }
            }
            break;
        default:
            break;
    }
    if (flag_rec && value.singleVariant.variant.type == API_PropertyGuidValueType && property.definition.collectionType == API_PropertySingleChoiceEnumerationCollectionType) {
        API_Guid guidValue = APINULLGuid;
        GS::Array<API_SingleEnumerationVariant> possibleEnumValues = property.definition.possibleEnumValues;

        // Для свойств с набором параметров необходимо задавать не само значение, а его GUID
        for (UInt32 i = 0; i < possibleEnumValues.GetSize (); i++) {
            switch (property.definition.valueType) {
                case API_PropertyIntegerValueType:
                    if (property.value.singleVariant.variant.intValue == possibleEnumValues[i].displayVariant.intValue) {
                        guidValue = possibleEnumValues[i].keyVariant.guidValue;
                    }
                    break;
                case API_PropertyRealValueType:
                    if (!is_equal (property.value.singleVariant.variant.doubleValue, possibleEnumValues[i].displayVariant.doubleValue)) {
                        guidValue = possibleEnumValues[i].keyVariant.guidValue;
                    }
                    break;
                case API_PropertyBooleanValueType:
                    if (property.value.singleVariant.variant.boolValue == possibleEnumValues[i].displayVariant.boolValue) {
                        guidValue = possibleEnumValues[i].keyVariant.guidValue;
                    }
                    break;
                case API_PropertyStringValueType:
                    if (property.value.singleVariant.variant.uniStringValue == possibleEnumValues[i].displayVariant.uniStringValue) {
                        guidValue = possibleEnumValues[i].keyVariant.guidValue;
                    }
                    break;
                default:
                    break;
            }
            if (guidValue != APINULLGuid) {
                property.value.singleVariant.variant.guidValue = guidValue;
                break;
            }
        }
        if (guidValue == APINULLGuid) {
            #if defined(TESTING)
            DBprnt ("ParamHelpers::ConvertToProperty err", "guidValue == APINULLGuid");
            #endif
            flag_rec = false;
        }
    }
    if (flag_rec) {
        property.isDefault = false;
        if (property.value.variantStatus != API_VariantStatusNormal) property.value.variantStatus = API_VariantStatusNormal;
        #if defined(AC_22) || defined(AC_23)
        if (!property.isEvaluated) property.isEvaluated = true;
        #else
        if (property.status != API_Property_HasValue) {
            property.status = API_Property_HasValue;
            if (property.definition.collectionType == API_PropertySingleCollectionType && property.value.singleVariant.variant.type == API_PropertyUndefinedValueType) {
                property.value.singleVariant.variant.type = property.definition.valueType;
            }
        }
        #endif
    }
    return flag_rec;
}

//--------------------------------------------------------------------------------------------------------------------------
//Ищет свойство property_flag_name в описании и по значению определяет - нужно ли обрабатывать элемент
//--------------------------------------------------------------------------------------------------------------------------
bool GetElemState (const API_Guid& elemGuid, const GS::Array<API_PropertyDefinition>& definitions, GS::UniString property_flag_name, bool& flagfind, bool check)
{
    flagfind = false;
    bool flag = false;
    if (definitions.IsEmpty ()) return false;
    GSErrCode	err = NoError;
    short n = 0; GS::UniString flag_name = "";
    if (check) {
        for (UInt32 i = 0; i < definitions.GetSize (); i++) {
            if (!definitions[i].description.IsEmpty ()) {
                if (definitions[i].description.Contains (property_flag_name)) {
                    n++;
                    flag_name.Append (definitions[i].name); flag_name.Append ("; ");
                }
            }
        }
        if (n == 0) return false;
        if (n > 1) msg_rep ("There are several sync flags as an element. This can lead to errors.", flag_name, APIERR_GENERAL, elemGuid);
    }
    for (UInt32 i = 0; i < definitions.GetSize (); i++) {
        if (!definitions[i].description.IsEmpty ()) {
            if (definitions[i].description.Contains (property_flag_name)) {
                API_Property propertyflag = {};
                err = ACAPI_Element_GetPropertyValue (elemGuid, definitions[i].guid, propertyflag);
                if (err == NoError) {
                    flagfind = true;
                    #if defined(AC_22) || defined(AC_23)
                    if (!propertyflag.isEvaluated) {
                        return false;
                    }
                    if (propertyflag.isDefault && !propertyflag.isEvaluated) {
                        return propertyflag.definition.defaultValue.basicValue.singleVariant.variant.boolValue;
                    } else {
                        return propertyflag.value.singleVariant.variant.boolValue;
                    }
                    #else
                    if (propertyflag.status == API_Property_NotAvailable) {
                        return false;
                    }
                    if (propertyflag.isDefault && propertyflag.status == API_Property_NotEvaluated) {
                        return propertyflag.definition.defaultValue.basicValue.singleVariant.variant.boolValue;
                    } else {
                        return propertyflag.value.singleVariant.variant.boolValue;
                    }
                    #endif
                } else {
                    return false;
                }
            }
        }
    }
    return false;
}

bool GetElemStateReverse (const API_Guid& elemGuid, const GS::Array<API_PropertyDefinition>& definitions, GS::UniString property_flag_name, bool& flagfind)
{
    if (definitions.IsEmpty ()) return false;
    GSErrCode	err = NoError;
    for (UInt32 i = 0; i < definitions.GetSize (); i++) {
        if (!definitions[i].description.IsEmpty ()) {
            if (definitions[i].description.Contains (property_flag_name)) {
                API_Property propertyflag = {};
                err = ACAPI_Element_GetPropertyValue (elemGuid, definitions[i].guid, propertyflag);
                if (err == NoError) {
                    flagfind = true;
                    #if defined(AC_22) || defined(AC_23)
                    if (!propertyflag.isEvaluated) {
                        return true;
                    }
                    if (propertyflag.isDefault && !propertyflag.isEvaluated) {
                        return propertyflag.definition.defaultValue.basicValue.singleVariant.variant.boolValue;
                    } else {
                        return propertyflag.value.singleVariant.variant.boolValue;
                    }
                    #else
                    if (propertyflag.status == API_Property_NotAvailable) {
                        return true;
                    }
                    if (propertyflag.isDefault && propertyflag.status == API_Property_NotEvaluated) {
                        return propertyflag.definition.defaultValue.basicValue.singleVariant.variant.boolValue;
                    } else {
                        return propertyflag.value.singleVariant.variant.boolValue;
                    }
                    #endif
                } else {
                    return true;
                }
            }
        }
    }
    return true;
}


// --------------------------------------------------------------------
// Запись словаря параметров для множества элементов
// --------------------------------------------------------------------
GS::Array<API_Guid> ParamHelpers::ElementsWrite (ParamDictElement& paramToWrite)
{
    GS::Array<API_Guid> rereadelem;
    if (paramToWrite.IsEmpty ()) return rereadelem;
    #if defined(TESTING)
    DBprnt ("ElementsWrite start");
    #endif
    for (GS::HashTable<API_Guid, ParamDictValue>::PairIterator cIt = paramToWrite.EnumeratePairs (); cIt != NULL; ++cIt) {
        #if defined(AC_28)
        ParamDictValue& params = cIt->value;
        API_Guid elemGuid = cIt->key;
        #else
        ParamDictValue& params = *cIt->value;
        API_Guid elemGuid = *cIt->key;
        #endif
        if (!params.IsEmpty ()) {
            if (ParamHelpers::Write (elemGuid, params)) rereadelem.Push (elemGuid);
        }
    }
    #if defined(TESTING)
    if (!rereadelem.IsEmpty ()) DBprnt ("ElementsWrite ReRead");
    DBprnt ("ElementsWrite end");
    #endif
    return rereadelem;
}

// --------------------------------------------------------------------
// Запись ParamDictValue в один элемент
// --------------------------------------------------------------------
bool ParamHelpers::Write (const API_Guid& elemGuid, ParamDictValue& params)
{
    if (params.IsEmpty ()) return false;
    if (elemGuid == APINULLGuid) return false;
    bool needReread = false;
    // Получаем список возможных префиксов
    ParamDictValue paramByType;
    GS::Array<GS::UniString> paramTypesList;
    paramTypesList.Push ("{@property:");
    paramTypesList.Push ("{@gdl:");
    paramTypesList.Push ("{@id:");
    paramTypesList.Push ("{@class:");
    paramTypesList.Push ("{@attrib:");
    paramTypesList.Push ("{@coord:");
    // Проходим поиском, специфичным для каждого типа
    // Для каждого типа - свой способ получения данных. Поэтому разбиваем по типам и обрабатываем по-отдельности
    for (UInt32 i = 0; i < paramTypesList.GetSize (); i++) {
        GS::UniString paramType = paramTypesList[i];
        ParamDictValue paramByType;
        for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = params.EnumeratePairs (); cIt != NULL; ++cIt) {
            #if defined(AC_28)
            ParamValue& param = cIt->value;
            #else
            ParamValue& param = *cIt->value;
            #endif
            if (param.rawName.Contains (paramType)) paramByType.Add (param.rawName, param);
        }
        if (!paramByType.IsEmpty ()) {

            // Проходим поиском, специфичным для каждого типа
            if (paramType.IsEqual ("{@property:")) {
                ParamHelpers::WriteProperty (elemGuid, paramByType);
            }
            if (paramType.IsEqual ("{@gdl:")) {
                ParamHelpers::WriteGDL (elemGuid, paramByType);
            }
            if (paramType.IsEqual ("{@id:")) {
                ParamHelpers::WriteID (elemGuid, paramByType);
            }
            if (paramType.IsEqual ("{@class:")) {
                needReread = ParamHelpers::WriteClassification (elemGuid, paramByType);
            }
            if (paramType.IsEqual ("{@attrib:")) {
                ParamHelpers::WriteAttribute (elemGuid, paramByType);
            }
            if (paramType.IsEqual ("{@coord:")) {
                ParamHelpers::WriteCoord (elemGuid, paramByType);
            }
        }
    }
    return needReread;
}

// --------------------------------------------------------------------
// Запись ParamDictValue в автотекст
// --------------------------------------------------------------------
void ParamHelpers::InfoWrite (ParamDictElement& paramToWrite)
{
    if (paramToWrite.IsEmpty ()) return;
    clock_t start, finish;
    double  duration;
    start = clock ();
    ParamDictValue paramsinfo;
    for (GS::HashTable<API_Guid, ParamDictValue>::PairIterator cIt = paramToWrite.EnumeratePairs (); cIt != NULL; ++cIt) {
        #if defined(AC_28)
        ParamDictValue& params = cIt->value;
        #else
        ParamDictValue& params = *cIt->value;
        #endif
        if (!params.IsEmpty ()) {
            for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = params.EnumeratePairs (); cIt != NULL; ++cIt) {
                #if defined(AC_28)
                ParamValue& param = cIt->value;
                #else
                ParamValue& param = *cIt->value;
                #endif
                if (param.fromInfo && !paramsinfo.ContainsKey (param.rawName)) paramsinfo.Add (param.rawName, param);
            }
        }
    }
    if (paramsinfo.IsEmpty ()) return;
    #if defined(TESTING)
    DBprnt ("InfoWrite start");
    #endif
    for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = paramsinfo.EnumeratePairs (); cIt != NULL; ++cIt) {
        #if defined(AC_28)
        ParamValue& param = cIt->value;
        #else
        ParamValue& param = *cIt->value;
        #endif
        GS::UniString dbKey = param.name;
        GS::UniString value = ParamHelpers::ToString (param, param.val.formatstring);
        GSErrCode err = NoError;
        #if defined(AC_27) || defined(AC_28)
        err = ACAPI_AutoText_SetAnAutoText (&dbKey, &value);
        #else
        err = ACAPI_Goodies (APIAny_SetAnAutoTextID, &dbKey, &value);
        #endif
        if (err != NoError) msg_rep ("InfoWrite", "APIAny_SetAnAutoTextID", err, APINULLGuid);
    }
    finish = clock ();
    duration = (double) (finish - start) / CLOCKS_PER_SEC;
    GS::UniString time = GS::UniString::Printf (" %.3f s", duration);
    msg_rep ("InfoWrite", "write " + time, NoError, APINULLGuid);
}


bool ParamHelpers::WriteClassification (const API_Guid& elemGuid, ParamDictValue& params)
{
    bool needReread = false;
    if (params.IsEmpty ()) return false;
    #if defined(TESTING)
    DBprnt ("    WriteClassification");
    #endif
    GSErrCode err = NoError;
    for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = params.EnumeratePairs (); cIt != NULL; ++cIt) {
        #if defined(AC_28)
        ParamValue& param = cIt->value;
        #else
        ParamValue& param = *cIt->value;
        #endif
        if (param.val.guidval != APINULLGuid) {
            err = ACAPI_Element_AddClassificationItem (elemGuid, param.val.guidval);
            if (err == NoError) {
                needReread = true;
            }
            if (err != NoError) msg_rep ("WriteClassification", "ACAPI_Element_AddClassificationItem", err, APINULLGuid);
        }
    }
    return needReread;
}

// --------------------------------------------------------------------
// Запись ParamDictValue в ID
// --------------------------------------------------------------------
void ParamHelpers::WriteID (const API_Guid& elemGuid, ParamDictValue& params)
{
    #ifdef AC_22
    msg_rep ("WriteID - ID", "Write ID not work in AC 22", NoError, elemGuid);
    #else
    if (params.IsEmpty ()) return;
    if (elemGuid == APINULLGuid) return;
    if (!params.ContainsKey ("{@id:id}")) return;
    #if defined(TESTING)
    DBprnt ("    WriteID");
    #endif
    GS::UniString val = ParamHelpers::ToString (params.Get ("{@id:id}"));
    GSErrCode err = NoError;
    #if defined(AC_27) || defined(AC_28)
    err = ACAPI_Element_ChangeElementInfoString (&elemGuid, &val);
    #else
    err = ACAPI_Database (APIDb_ChangeElementInfoStringID, (void*) &elemGuid, (void*) &val);
    #endif
    if (err != NoError) {
        msg_rep ("WriteID - ID", "ACAPI_Database(APIDb_ChangeElementInfoStringID", err, elemGuid);
    }
    #endif
}

// --------------------------------------------------------------------
// Запись ParamDictValue в аттрибуты элемента (слой)
// --------------------------------------------------------------------
void ParamHelpers::WriteAttribute (const API_Guid& elemGuid, ParamDictValue& params)
{
    GSErrCode err = NoError;
    if (params.IsEmpty ()) return;
    #if defined(TESTING)
    DBprnt ("    WriteAttribute");
    #endif
    if (elemGuid == APINULLGuid) return;
    if (!params.ContainsKey ("{@attrib:layer}")) {
        #if defined(TESTING)
        DBprnt ("WriteAttribute err", "{ @attrib:layer } not found");
        #endif
        return;
    }
    if (!params.Get ("{@attrib:layer}").isValid) {
        #if defined(TESTING)
        DBprnt ("WriteAttribute err", "{ @attrib:layer } not valid");
        #endif
        return;
    }
    // Поиск номера слоя по имени, если номер не найден
    API_AttributeIndex newlayer;
    if (!API_AttributeIndexFindByName (params.Get ("{@attrib:layer}").val.uniStringValue, API_LayerID, newlayer)) {
        msg_rep ("ParamHelpers::WriteAttribute", "ACAPI_Attribute_Search - " + params.Get ("{@attrib:layer}").val.uniStringValue, err, elemGuid);
        return;
    }
    API_Element element = {};
    API_Element elementMask = {};
    element.header.guid = elemGuid;
    err = ACAPI_Element_Get (&element);
    if (err != NoError) {
        msg_rep ("ParamHelpers::WriteAttribute", "ACAPI_Element_Get", err, elemGuid);
        return;
    }
    if (newlayer == element.header.layer) {
        #if defined(TESTING)
        DBprnt ("      WriteAttribute not need");
        #endif
        return;
    }
    ACAPI_ELEMENT_MASK_CLEAR (elementMask);
    ACAPI_ELEMENT_MASK_SET (elementMask, API_Elem_Head, layer);
    element.header.layer = newlayer;
    err = ACAPI_Element_Change (&element, &elementMask, nullptr, 0, true);
    if (err != NoError) {
        msg_rep ("ParamHelpers::WriteAttribute", "ACAPI_Element_Change", err, elemGuid);
        return;
    }
}

// --------------------------------------------------------------------
// Запись ParamDictValue в координаты элемента
// --------------------------------------------------------------------
void ParamHelpers::WriteCoord (const API_Guid& elemGuid, ParamDictValue& params)
{
    if (params.IsEmpty ()) return;
    #if defined(TESTING)
    DBprnt ("      WriteCoord");
    #endif
    if (elemGuid == APINULLGuid) return;
    GSErrCode err = NoError;
    API_Elem_Head elem_head = {};
    API_Element element = {};
    elem_head.guid = elemGuid;
    err = ACAPI_Element_GetHeader (&elem_head);
    if (err != NoError) {
        msg_rep ("ParamHelpers::WriteCoord", "ACAPI_Element_GetHeader", err, elem_head.guid);
        return;
    }
    API_ElemTypeID elemType = GetElemTypeID (elem_head);
    element.header.guid = elemGuid;
    err = ACAPI_Element_Get (&element);
    if (err != NoError) {
        msg_rep ("ParamHelpers::WriteCoord", "ACAPI_Element_Get", err, elem_head.guid);
        return;
    }
    API_Element mask;
    ACAPI_ELEMENT_MASK_CLEAR (mask);
    bool flag_write = false;
    double dval = 0;
    ParamValueData pval;
    switch (elemType) {
        case API_WindowID:
            if (params.ContainsKey ("{@coord:symb_pos_x}")) {
                flag_write = true;
                ACAPI_ELEMENT_MASK_SET (mask, API_WindowType, objLoc);
                pval = params.Get ("{@coord:symb_pos_x}").val;
                if (pval.formatstring.forceRaw) {
                    dval = pval.rawDoubleValue;
                } else {
                    dval = pval.doubleValue;
                }
                element.window.objLoc = dval;
            }
            break;
        case API_DoorID:
            if (params.ContainsKey ("{@coord:symb_pos_x}")) {
                flag_write = true;
                ACAPI_ELEMENT_MASK_SET (mask, API_DoorType, objLoc);
                pval = params.Get ("{@coord:symb_pos_x}").val;
                if (pval.formatstring.forceRaw) {
                    dval = pval.rawDoubleValue;
                } else {
                    dval = pval.doubleValue;
                }
                element.door.objLoc = dval;
            }
            break;
        case API_ObjectID:
            if (params.ContainsKey ("{@coord:symb_pos_x}")) {
                flag_write = true;
                ACAPI_ELEMENT_MASK_SET (mask, API_ObjectType, pos);
                pval = params.Get ("{@coord:symb_pos_x}").val;
                if (pval.formatstring.forceRaw) {
                    dval = pval.rawDoubleValue;
                } else {
                    dval = pval.doubleValue;
                }
                element.object.pos.x = dval;
            }
            if (params.ContainsKey ("{@coord:symb_pos_y}")) {
                flag_write = true;
                ACAPI_ELEMENT_MASK_SET (mask, API_ObjectType, pos);
                pval = params.Get ("{@coord:symb_pos_y}").val;
                if (pval.formatstring.forceRaw) {
                    dval = pval.rawDoubleValue;
                } else {
                    dval = pval.doubleValue;
                }
                element.object.pos.y = dval;
            }
            if (params.ContainsKey ("{@coord:symb_pos_z}")) {
                flag_write = true;
                ACAPI_ELEMENT_MASK_SET (mask, API_ObjectType, level);
                pval = params.Get ("{@coord:symb_pos_z}").val;
                if (pval.formatstring.forceRaw) {
                    dval = pval.rawDoubleValue;
                } else {
                    dval = pval.doubleValue;
                }
                element.object.level = dval;
            }
            if (params.ContainsKey ("{@coord:symb_rotangle}")) {
                flag_write = true;
                ACAPI_ELEMENT_MASK_SET (mask, API_ObjectType, angle);
                pval = params.Get ("{@coord:symb_rotangle}").val;
                if (pval.formatstring.forceRaw) {
                    dval = pval.rawDoubleValue;
                } else {
                    dval = pval.doubleValue;
                }
                element.object.angle = dval * PI / 180.0;
            }
            break;
        case API_ColumnID:
            if (params.ContainsKey ("{@coord:symb_pos_x}")) {
                flag_write = true;
                ACAPI_ELEMENT_MASK_SET (mask, API_ColumnType, origoPos);
                pval = params.Get ("{@coord:symb_pos_x}").val;
                if (pval.formatstring.forceRaw) {
                    dval = pval.rawDoubleValue;
                } else {
                    dval = pval.doubleValue;
                }
                element.column.origoPos.x = dval;
            }
            if (params.ContainsKey ("{@coord:symb_pos_y}")) {
                flag_write = true;
                ACAPI_ELEMENT_MASK_SET (mask, API_ColumnType, origoPos);
                pval = params.Get ("{@coord:symb_pos_y}").val;
                if (pval.formatstring.forceRaw) {
                    dval = pval.rawDoubleValue;
                } else {
                    dval = pval.doubleValue;
                }
                element.column.origoPos.y = dval;
            }
            if (params.ContainsKey ("{@coord:symb_rotangle}")) {
                flag_write = true;
                ACAPI_ELEMENT_MASK_SET (mask, API_ColumnType, slantAngle);
                pval = params.Get ("{@coord:symb_rotangle}").val;
                if (pval.formatstring.forceRaw) {
                    dval = pval.rawDoubleValue;
                } else {
                    dval = pval.doubleValue;
                }
                element.column.slantAngle = dval * PI / 180.0;
            }
            break;
        case API_WallID:
            if (params.ContainsKey ("{@coord:symb_pos_sx}")) {
                flag_write = true;
                ACAPI_ELEMENT_MASK_SET (mask, API_WallType, begC);
                pval = params.Get ("{@coord:symb_pos_sx}").val;
                if (pval.formatstring.forceRaw) {
                    dval = pval.rawDoubleValue;
                } else {
                    dval = pval.doubleValue;
                }
                element.wall.begC.x = dval;
            }
            if (params.ContainsKey ("{@coord:symb_pos_sy}")) {
                flag_write = true;
                ACAPI_ELEMENT_MASK_SET (mask, API_WallType, begC);
                pval = params.Get ("{@coord:symb_pos_sy}").val;
                if (pval.formatstring.forceRaw) {
                    dval = pval.rawDoubleValue;
                } else {
                    dval = pval.doubleValue;
                }
                element.wall.begC.y = dval;
            }
            if (params.ContainsKey ("{@coord:symb_pos_ex}")) {
                flag_write = true;
                ACAPI_ELEMENT_MASK_SET (mask, API_WallType, endC);
                pval = params.Get ("{@coord:symb_pos_ex}").val;
                if (pval.formatstring.forceRaw) {
                    dval = pval.rawDoubleValue;
                } else {
                    dval = pval.doubleValue;
                }
                element.wall.endC.x = dval;
            }
            if (params.ContainsKey ("{@coord:symb_pos_ey}")) {
                flag_write = true;
                ACAPI_ELEMENT_MASK_SET (mask, API_WallType, endC);
                pval = params.Get ("{@coord:symb_pos_ey}").val;
                if (pval.formatstring.forceRaw) {
                    dval = pval.rawDoubleValue;
                } else {
                    dval = pval.doubleValue;
                }
                element.wall.endC.y = dval;
            }
            break;
        case API_BeamID:
            if (params.ContainsKey ("{@coord:symb_pos_sx}")) {
                flag_write = true;
                ACAPI_ELEMENT_MASK_SET (mask, API_BeamType, begC);
                pval = params.Get ("{@coord:symb_pos_sx}").val;
                if (pval.formatstring.forceRaw) {
                    dval = pval.rawDoubleValue;
                } else {
                    dval = pval.doubleValue;
                }
                element.beam.begC.x = dval;
            }
            if (params.ContainsKey ("{@coord:symb_pos_sy}")) {
                flag_write = true;
                ACAPI_ELEMENT_MASK_SET (mask, API_BeamType, begC);
                pval = params.Get ("{@coord:symb_pos_sy}").val;
                if (pval.formatstring.forceRaw) {
                    dval = pval.rawDoubleValue;
                } else {
                    dval = pval.doubleValue;
                }
                element.beam.begC.y = dval;
            }
            if (params.ContainsKey ("{@coord:symb_pos_ex}")) {
                flag_write = true;
                ACAPI_ELEMENT_MASK_SET (mask, API_BeamType, endC);
                pval = params.Get ("{@coord:symb_pos_ex}").val;
                if (pval.formatstring.forceRaw) {
                    dval = pval.rawDoubleValue;
                } else {
                    dval = pval.doubleValue;
                }
                element.beam.endC.x = dval;
            }
            if (params.ContainsKey ("{@coord:symb_pos_ey}")) {
                flag_write = true;
                ACAPI_ELEMENT_MASK_SET (mask, API_BeamType, endC);
                pval = params.Get ("{@coord:symb_pos_ey}").val;
                if (pval.formatstring.forceRaw) {
                    dval = pval.rawDoubleValue;
                } else {
                    dval = pval.doubleValue;
                }
                element.beam.endC.y = dval;
            }
            break;
        default:
            #if defined(TESTING)
            DBprnt ("WriteCoord err", "wrong type element");
            #endif
            break;
    }
    if (flag_write) {
        err = ACAPI_Element_Change (&element, &mask, nullptr, 0, true);
        if (err != NoError) {
            msg_rep ("ParamHelpers::WriteCoord", "ACAPI_Element_Change", err, elem_head.guid);
            return;
        }
    } else {
        #if defined(TESTING)
        DBprnt ("      WriteCoord no data");
        #endif
    }
}

// --------------------------------------------------------------------
// Запись ParamDictValue в GDL параметры
// --------------------------------------------------------------------
void ParamHelpers::WriteGDL (const API_Guid& elemGuid, ParamDictValue& params)
{
    if (params.IsEmpty ()) return;
    #if defined(TESTING)
    DBprnt ("    WriteGDL\n");
    #endif
    if (elemGuid == APINULLGuid) return;
    API_Elem_Head elem_head = {};
    API_Element element = {};
    API_ElemTypeID	elemType;
    API_Guid		elemGuidt;
    API_ParamOwnerType	apiOwner = {};
    API_GetParamsType	apiParams = {};
    API_ChangeParamType	chgParam;
    elem_head.guid = elemGuid;
    GSErrCode err = ACAPI_Element_GetHeader (&elem_head);
    if (err != NoError) {
        msg_rep ("ParamHelpers::WriteGDL", "ACAPI_Element_GetHeader", err, elem_head.guid);
        return;
    }
    element.header.guid = elemGuid;
    err = ACAPI_Element_Get (&element);
    if (err != NoError) {
        msg_rep ("ParamHelpers::WriteGDL", "ACAPI_Element_Get", err, elem_head.guid);
        return;
    }
    API_ElemTypeID eltype = GetElemTypeID (elem_head);
    GetGDLParametersHead (element, elem_head, elemType, elemGuidt);
    BNZeroMemory (&apiOwner, sizeof (API_ParamOwnerType));
    BNZeroMemory (&apiParams, sizeof (API_GetParamsType));
    apiOwner.guid = elemGuidt;
    #if defined AC_26 || defined AC_27 || defined AC_28
    apiOwner.type.typeID = elemType;
    #else
    apiOwner.typeID = elemType;
    #endif
    #if defined(AC_27) || defined(AC_28)
    err = ACAPI_LibraryPart_OpenParameters (&apiOwner);
    #else
    err = ACAPI_Goodies (APIAny_OpenParametersID, &apiOwner, nullptr);
    #endif
    if (err != NoError) {
        msg_rep ("ParamHelpers::WriteGDL", "APIAny_OpenParametersID", err, elem_head.guid);
        return;
    }
    #if defined(AC_27) || defined(AC_28)
    err = ACAPI_LibraryPart_GetActParameters (&apiParams);
    #else
    err = ACAPI_Goodies (APIAny_GetActParametersID, &apiParams);
    #endif
    if (err != NoError) {
        msg_rep ("ParamHelpers::WriteGDL", "APIAny_GetActParametersID", err, elem_head.guid);
        #if defined(AC_27) || defined(AC_28)
        err = ACAPI_LibraryPart_CloseParameters ();
        #else
        err = ACAPI_Goodies (APIAny_CloseParametersID);
        #endif
        if (err != NoError) {
            msg_rep ("ParamHelpers::WriteGDL", "APIAny_CloseParametersID", err, elem_head.guid);
            return;
        }
    }

    // TODO Оптимизировать, разнести по функциям
    bool flagFind = false;
    Int32	addParNum = BMGetHandleSize ((GSHandle) apiParams.params) / sizeof (API_AddParType);
    Int32 nfind = params.GetSize ();
    for (Int32 i = 0; i < addParNum; ++i) {
        API_AddParType& actualParam = (*apiParams.params)[i];
        if (actualParam.typeMod != API_ParSimple) continue;
        GS::UniString name = actualParam.name;
        GS::UniString rawname = "{@gdl:" + name.ToLowerCase () + "}";
        if (!params.ContainsKey (rawname)) continue;

        ParamValueData paramfrom = params.Get (rawname).val;
        BNZeroMemory (&chgParam, sizeof (API_ChangeParamType));
        chgParam.index = actualParam.index;
        CHTruncate (actualParam.name, chgParam.name, API_NameLen);
        // Поиск индекса аттрибута при необходимости
        API_AttributeIndex attribinx; Int32 attribinxint = 0;
        API_AttrTypeID type = API_ZombieAttrID;
        if (actualParam.typeID == APIParT_LineTyp) type = API_LinetypeID;
        if (actualParam.typeID == APIParT_Profile) type = API_ProfileID;
        if (actualParam.typeID == APIParT_BuildingMaterial) type = API_BuildingMaterialID;
        if (actualParam.typeID == APIParT_FillPat) type = API_FilltypeID;
        if (actualParam.typeID == APIParT_Mater) type = API_MaterialID;
        if (type != API_ZombieAttrID) {
            if (paramfrom.type == API_PropertyStringValueType) {
                if (API_AttributeIndexFindByName (paramfrom.uniStringValue, type, attribinx)) {
                    #if defined(AC_27) || defined(AC_28)
                    attribinxint = attribinx.ToInt32_Deprecated ();
                    #else
                    attribinxint = attribinx;
                    #endif
                } else {
                    attribinxint = paramfrom.intValue;
                }
            } else {
                attribinxint = paramfrom.intValue;
            }
        }
        if (actualParam.typeID == APIParT_CString) {
            GS::uchar_t* buffer = new GS::uchar_t[256];
            GS::ucsncpy (buffer, paramfrom.uniStringValue.ToUStr ().Get (), 256);
            chgParam.uStrValue = buffer;
        }
        if (actualParam.typeID == APIParT_Integer) {
            chgParam.realValue = paramfrom.intValue;
        }
        if (actualParam.typeID == APIParT_PenCol) {
            if (paramfrom.intValue > 0 && paramfrom.intValue < 255) chgParam.realValue = paramfrom.intValue;
        }
        if (type != API_ZombieAttrID && attribinxint > 0) {
            chgParam.realValue = attribinxint;
        }
        if (actualParam.typeID == APIParT_Length) {
            if (paramfrom.formatstring.forceRaw) {
                chgParam.realValue = paramfrom.rawDoubleValue;
            } else {
                chgParam.realValue = paramfrom.doubleValue;
            }
        }
        if (actualParam.typeID == APIParT_Angle) {
            if (paramfrom.formatstring.forceRaw) {
                chgParam.realValue = paramfrom.rawDoubleValue;
            } else {
                chgParam.realValue = paramfrom.doubleValue;
            }
        }
        if (actualParam.typeID == APIParT_RealNum ||
            actualParam.typeID == APIParT_ColRGB ||
            actualParam.typeID == APIParT_Intens) {
            if (paramfrom.formatstring.forceRaw) {
                chgParam.realValue = paramfrom.rawDoubleValue;
            } else {
                chgParam.realValue = paramfrom.doubleValue;
            }
        }
        if (actualParam.typeID == APIParT_Boolean) {
            if (paramfrom.boolValue) {
                chgParam.realValue = 1;
            } else {
                chgParam.realValue = 0;
            }
        }
        #if defined(AC_27) || defined(AC_28)
        err = ACAPI_LibraryPart_ChangeAParameter (&chgParam);
        #else
        err = ACAPI_Goodies (APIAny_ChangeAParameterID, &chgParam, nullptr);
        #endif
        if (err != NoError) {
            msg_rep ("ParamHelpers::WriteGDL", "APIAny_ChangeAParameterID", err, elem_head.guid);
            return;
        }
    }
    #if defined(AC_27) || defined(AC_28)
    err = ACAPI_LibraryPart_GetActParameters (&apiParams);
    #else
    err = ACAPI_Goodies (APIAny_GetActParametersID, &apiParams);
    #endif
    if (err != NoError) {
        msg_rep ("ParamHelpers::WriteGDL", "APIAny_GetActParametersID", err, elem_head.guid);
        return;
    }
    #if defined(AC_27) || defined(AC_28)
    err = ACAPI_LibraryPart_CloseParameters ();
    #else
    err = ACAPI_Goodies (APIAny_CloseParametersID);
    #endif
    if (err != NoError) {
        msg_rep ("ParamHelpers::WriteGDL", "APIAny_CloseParametersID", err, elem_head.guid);
        return;
    }
    API_ElementMemo	elemMemo = {};
    elemMemo.params = apiParams.params;
    err = ACAPI_Element_ChangeMemo (elemGuidt, APIMemoMask_AddPars, &elemMemo);
    if (err != NoError) msg_rep ("ParamHelpers::WriteGDL", "ACAPI_Element_ChangeMemo", err, elem_head.guid);
    ACAPI_DisposeAddParHdl (&apiParams.params);
    if (err == NoError) {
        #if defined(AC_27) || defined(AC_28)
        err = ACAPI_LibraryManagement_RunGDLParScript (&elem_head, 0);
        #else
        err = ACAPI_Goodies (APIAny_RunGDLParScriptID, &elem_head, 0);
        #endif
        if (err != NoError) {
            msg_rep ("ParamHelpers::WriteGDL", "APIAny_RunGDLParScriptID", err, elemGuid);
            return;
        }
    }
}

// --------------------------------------------------------------------
// Запись ParamDictValue в свойства
// --------------------------------------------------------------------
void ParamHelpers::WriteProperty (const API_Guid& elemGuid, ParamDictValue& params)
{
    if (params.IsEmpty ()) return;
    if (elemGuid == APINULLGuid) return;
    GSErrCode error = NoError;
    #if defined(TESTING)
    DBprnt ("    WriteProperty");
    #endif
    // Если для свойств известно только определение, но не было получено свойство - самое время это сделать
    GS::Array<API_PropertyDefinition> propertyDefinitions;
    for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = params.EnumeratePairs (); cIt != NULL; ++cIt) {
        #if defined(AC_28)
        ParamValue& param = cIt->value;
        #else
        ParamValue& param = *cIt->value;
        #endif
        if (param.isValid && param.property.definition.guid == APINULLGuid && param.definition.guid != APINULLGuid) {
            API_PropertyDefinition definition = param.definition;
            propertyDefinitions.Push (definition);
        }
    }

    if (!propertyDefinitions.IsEmpty ()) {
        #if defined(TESTING)
        DBprnt ("    WriteProperty", "!propertyDefinitions.IsEmpty()");
        #endif
        GS::Array<API_Property> properties;
        error = ACAPI_Element_GetPropertyValues (elemGuid, propertyDefinitions, properties);
        if (error != NoError) {
            msg_rep ("WriteProperty", "ACAPI_Element_GetPropertyValues", error, elemGuid);
        } else {
            for (UInt32 i = 0; i < properties.GetSize (); i++) {
                GS::UniString fname;
                GetPropertyFullName (properties[i].definition, fname);
                GS::UniString rawName = "{@property:" + fname.ToLowerCase () + "}";
                if (params.ContainsKey (rawName)) {
                    params.Get (rawName).property = properties[i];
                    params.Get (rawName).fromPropertyDefinition = true;
                }
            }
        }
    }

    for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = params.EnumeratePairs (); cIt != NULL; ++cIt) {
        #if defined(AC_28)
        ParamValue& param = cIt->value;
        #else
        ParamValue& param = *cIt->value;
        #endif
        if (param.isValid && param.property.definition.guid != APINULLGuid) {
            API_Property property = param.property;
            if (ParamHelpers::ConvertToProperty (param, property)) {
                error = ACAPI_Element_SetProperty (elemGuid, property);
                if (error != NoError) {
                    msg_rep ("WriteProperty err", "ACAPI_Element_SetProperty: " + property.definition.name, error, elemGuid);
                }
            }
        } else {
            #if defined(TESTING)
            if (!param.isValid) DBprnt ("WriteProperty err", "!param.isValid" + param.rawName);
            if (param.property.definition.guid == APINULLGuid) DBprnt ("WriteProperty err", "definition.guid == APINULLGuid" + param.rawName);
            #endif
        }
    }
}

bool ParamHelpers::hasUnreadProperyDefinition (ParamDictElement& paramToRead)
{
    for (GS::HashTable<API_Guid, ParamDictValue>::PairIterator cIt = paramToRead.EnumeratePairs (); cIt != NULL; ++cIt) {
        #if defined(AC_28)
        ParamDictValue& params = cIt->value;
        #else
        ParamDictValue& params = *cIt->value;
        #endif
        if (!params.IsEmpty ()) {
            for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = params.EnumeratePairs (); cIt != NULL; ++cIt) {
                #if defined(AC_28)
                ParamValue& param = cIt->value;
                #else
                ParamValue& param = *cIt->value;
                #endif
                if (param.fromProperty && !param.fromPropertyDefinition && !param.fromAttribDefinition) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool ParamHelpers::hasUnreadCoord (ParamDictElement& paramToRead)
{
    for (GS::HashTable<API_Guid, ParamDictValue>::PairIterator cIt = paramToRead.EnumeratePairs (); cIt != NULL; ++cIt) {
        #if defined(AC_28)
        ParamDictValue& params = cIt->value;
        #else
        ParamDictValue& params = *cIt->value;
        #endif
        if (!params.IsEmpty ()) {
            for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = params.EnumeratePairs (); cIt != NULL; ++cIt) {
                #if defined(AC_28)
                ParamValue& param = cIt->value;
                #else
                ParamValue& param = *cIt->value;
                #endif
                if (param.fromCoord) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool ParamHelpers::hasUnreadAttribute (ParamDictElement& paramToRead)
{
    for (GS::HashTable<API_Guid, ParamDictValue>::PairIterator cIt = paramToRead.EnumeratePairs (); cIt != NULL; ++cIt) {
        #if defined(AC_28)
        ParamDictValue& params = cIt->value;
        #else
        ParamDictValue& params = *cIt->value;
        #endif
        if (!params.IsEmpty ()) {
            for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = params.EnumeratePairs (); cIt != NULL; ++cIt) {
                #if defined(AC_28)
                ParamValue& param = cIt->value;
                #else
                ParamValue& param = *cIt->value;
                #endif
                if (param.fromAttribElement) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool ParamHelpers::hasUnreadInfo (ParamDictElement& paramToRead, ParamDictValue& propertyParams)
{
    for (GS::HashTable<API_Guid, ParamDictValue>::PairIterator cIt = paramToRead.EnumeratePairs (); cIt != NULL; ++cIt) {
        #if defined(AC_28)
        ParamDictValue& params = cIt->value;
        #else
        ParamDictValue& params = *cIt->value;
        #endif
        if (!params.IsEmpty ()) {
            for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = params.EnumeratePairs (); cIt != NULL; ++cIt) {
                #if defined(AC_28)
                ParamValue& param = cIt->value;
                #else
                ParamValue& param = *cIt->value;
                #endif
                if (param.fromInfo) {
                    // Проверим - есть ли уже считанный параметр
                    if (propertyParams.ContainsKey (param.rawName)) {
                        if (!propertyParams.Get (param.rawName).isValid) {
                            return true; // Прочитан криво, перечитываем
                        }
                    } else {
                        return true; // В списке общих параметров не найден, перечитаем
                    }
                }
            }
        }
    }
    return false;
}

bool ParamHelpers::hasGlob (ParamDictValue& propertyParams)
{
    if (propertyParams.IsEmpty ()) return false;
    if (!propertyParams.ContainsKey ("{@flag:has_glob}")) return false;
    return true;
}

bool ParamHelpers::hasInfo (ParamDictValue& propertyParams)
{
    if (propertyParams.IsEmpty ()) return false;
    if (!propertyParams.ContainsKey ("{@flag:has_info}")) return false;
    return true;
}

bool ParamHelpers::has_LocOrigin (ParamDictValue& propertyParams)
{
    if (propertyParams.IsEmpty ()) return false;
    if (!propertyParams.ContainsKey ("{@flag:has_LocOrigin}")) return false;
    return true;
}

bool ParamHelpers::hasAttribute (ParamDictValue& propertyParams)
{
    if (propertyParams.IsEmpty ()) return false;
    if (!propertyParams.ContainsKey ("{@flag:has_attrib}")) return false;
    return true;
}

bool ParamHelpers::hasProperyDefinition (ParamDictValue& propertyParams)
{
    if (propertyParams.IsEmpty ()) return false;
    if (!propertyParams.ContainsKey ("{@flag:has_properydefinition}")) return false;
    return true;
}

bool ParamHelpers::hasUnreadGlob (ParamDictElement& paramToRead, ParamDictValue& propertyParams)
{
    for (GS::HashTable<API_Guid, ParamDictValue>::PairIterator cIt = paramToRead.EnumeratePairs (); cIt != NULL; ++cIt) {
        #if defined(AC_28)
        ParamDictValue& params = cIt->value;
        #else
        ParamDictValue& params = *cIt->value;
        #endif
        if (!params.IsEmpty ()) {
            for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cItt = params.EnumeratePairs (); cItt != NULL; ++cItt) {
                #if defined(AC_28)
                ParamValue& param = cItt->value;
                #else
                ParamValue& param = *cItt->value;
                #endif
                if (param.fromGlob) {
                    // Проверим - есть ли уже считанный параметр
                    if (propertyParams.ContainsKey (param.rawName)) {
                        if (!propertyParams.Get (param.rawName).isValid) {
                            return true; // Прочитан криво, перечитываем
                        }
                    } else {
                        return true; // В списке общих параметров не найден, перечитаем
                    }
                }
            }
        }
    }
    return false;
}

// --------------------------------------------------------------------
// Заполнение словаря параметров для множества элементов
// --------------------------------------------------------------------
void ParamHelpers::ElementsRead (ParamDictElement& paramToRead, ParamDictValue& propertyParams, ClassificationFunc::SystemDict& systemdict)
{
    if (paramToRead.IsEmpty ()) return;
    #if defined(TESTING)
    DBprnt ("ElementsRead start");
    #endif
    if (!ParamHelpers::hasInfo (propertyParams) && !propertyParams.ContainsKey ("{@flag:no_info}")) {
        if (ParamHelpers::hasUnreadInfo (paramToRead, propertyParams)) ParamHelpers::GetAllInfoToParamDict (propertyParams);
    }
    if (!ParamHelpers::hasGlob (propertyParams) && !propertyParams.ContainsKey ("{@flag:no_glob}")) {
        if (ParamHelpers::hasUnreadGlob (paramToRead, propertyParams)) ParamHelpers::GetAllGlobToParamDict (propertyParams);
    }
    if (!ParamHelpers::hasProperyDefinition (propertyParams) && !propertyParams.ContainsKey ("{@flag:no_properydefinition}")) {
        if (ParamHelpers::hasUnreadProperyDefinition (paramToRead)) ParamHelpers::AllPropertyDefinitionToParamDict (propertyParams);
    }
    if (!ParamHelpers::has_LocOrigin (propertyParams)) {
        if (ParamHelpers::hasUnreadCoord (paramToRead)) ParamHelpers::GetLocOriginToParamDict (propertyParams);
    }

    // Выбираем по-элементно параметры для чтения
    for (GS::HashTable<API_Guid, ParamDictValue>::PairIterator cIt = paramToRead.EnumeratePairs (); cIt != NULL; ++cIt) {
        #if defined(AC_28)
        ParamDictValue& params = cIt->value;
        API_Guid elemGuid = cIt->key;
        #else
        ParamDictValue& params = *cIt->value;
        API_Guid elemGuid = *cIt->key;
        #endif
        if (!params.IsEmpty ()) {
            if (!propertyParams.IsEmpty ()) ParamHelpers::CompareParamDictValue (propertyParams, params); // Сопоставляем свойства
            ParamHelpers::Read (elemGuid, params, propertyParams, systemdict);
        }
    }
    #if defined(TESTING)
    DBprnt ("ElementsRead end");
    #endif
}

// --------------------------------------------------------------------
// Заполнение словаря с параметрами
// --------------------------------------------------------------------
void ParamHelpers::Read (const API_Guid& elemGuid, ParamDictValue& params, ParamDictValue& propertyParams, ClassificationFunc::SystemDict& systemdict)
{
    if (params.IsEmpty ()) return;
    if (elemGuid == APINULLGuid) {
        msg_rep ("ParamDictRead", "elemGuid == APINULLGuid", APIERR_GENERAL, elemGuid);
        return;
    }
    API_Elem_Head elem_head = {}; BNZeroMemory (&elem_head, sizeof (API_Elem_Head));
    elem_head.guid = elemGuid;
    GSErrCode err = ACAPI_Element_GetHeader (&elem_head);
    if (err != NoError) {
        msg_rep ("ParamDictRead", "ACAPI_Element_GetHeader", err, elem_head.guid);
        return;
    }
    API_ElemTypeID eltype = GetElemTypeID (elem_head);
    bool can_read_fromMaterial = true;
    if (eltype != API_WallID &&
       eltype != API_SlabID &&
       eltype != API_ColumnID &&
       eltype != API_BeamID &&
       eltype != API_RoofID &&
       eltype != API_BeamSegmentID &&
       eltype != API_ColumnSegmentID &&
       eltype != API_ShellID) can_read_fromMaterial = false;
    bool can_read_fromGDL = !can_read_fromMaterial;
    if (eltype == API_MorphID) can_read_fromGDL = false;
    // Для некоторых типов элементов есть общая информация, которая может потребоваться
    // Пройдём по параметрам и посмотрим - что нам нужно заранее прочитать
    bool needGetElement = false;
    bool needGetAllDefinitions = false;

    // Получаем список возможных префиксов
    GS::Array<GS::UniString> paramTypesList = {};
    GetParamTypeList (paramTypesList);

    for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = params.EnumeratePairs (); cIt != NULL; ++cIt) {
        #if defined(AC_28)
        ParamValue& param = cIt->value;
        #else
        ParamValue& param = *cIt->value;
        #endif
        if (param.fromGuid == APINULLGuid) param.fromGuid = elemGuid;
        if (param.fromGuid == elemGuid && param.fromGuid != APINULLGuid) {

            // Когда нужно получить весь элемент
            if (param.fromElement || (param.fromGDLdescription && can_read_fromGDL) || param.fromCoord || (param.fromMorph && eltype == API_MorphID) || param.fromAttribDefinition) {
                needGetElement = true;
            }
            if (can_read_fromMaterial && param.fromMaterial) {
                needGetElement = true;
            }
            if (eltype == API_CurtainWallPanelID || eltype == API_CurtainWallFrameID
               || eltype == API_CurtainWallJunctionID
               || eltype == API_CurtainWallAccessoryID
               || eltype == API_RailingToprailID
               || eltype == API_RailingHandrailID
               || eltype == API_RailingRailID
               || eltype == API_RailingPostID
               || eltype == API_RailingInnerPostID
               || eltype == API_RailingBalusterID
               || eltype == API_RailingPanelID
               || eltype == API_RailingNodeID
               || eltype == API_RailingToprailEndID
               || eltype == API_RailingHandrailEndID
               || eltype == API_RailingRailEndID
               || eltype == API_RailingToprailConnectionID
               || eltype == API_RailingHandrailConnectionID
               || eltype == API_RailingRailConnectionID
               || eltype == API_RailingEndFinishID
               ) {
                needGetElement = true;
            }
            if (param.fromProperty && !param.fromPropertyDefinition && !param.fromAttribDefinition) {
                if (!param.rawName.Contains ("{@property:sync_name")) {
                    needGetAllDefinitions = true; // Нужно проверить соответсвие описаний имени свойства
                    #if defined(TESTING)
                    DBprnt ("Read err - needGetAllDefinitions", param.rawName);
                    #endif
                }
            }
        }
    }

    if (needGetAllDefinitions) {
        AllPropertyDefinitionToParamDict (params, elemGuid);  // Проверим - для всех ли свойств подобраны определения
    }
    API_Element element = {}; BNZeroMemory (&element, sizeof (API_Element));
    if (needGetElement) {
        element.header.guid = elemGuid;
        err = ACAPI_Element_Get (&element);
        if (err != NoError) {
            msg_rep ("ParamDictRead", "ACAPI_Element_Get", err, elem_head.guid);
            return;
        }
    } else {
        UNUSED_VARIABLE (element);
    }
    // Для каждого типа - свой способ получения данных. Поэтому разбиваем по типам и обрабатываем по-отдельности
    for (UInt32 i = 0; i < paramTypesList.GetSize (); i++) {
        GS::UniString paramType = paramTypesList.Get (i);
        if (paramType.IsEqual ("{@info:")) continue;
        ParamDictValue paramByType;
        // Для некоторых параметров выборка не требуется
        for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = params.EnumeratePairs (); cIt != NULL; ++cIt) {
            #if defined(AC_28)
            ParamValue& param = cIt->value;
            #else
            ParamValue& param = *cIt->value;
            #endif
            // Выбираем элементы с подходящим Guid
            if (param.fromGuid == elemGuid) {
                // Для свойств с формулами основной признак - флаг, т.к. вычислению подлежат и свойства с составом конструкций.
                if (param.val.hasFormula && paramType.IsEqual ("{@formula:")) {
                    paramByType.Add (param.rawName, param);
                } else {
                    if (param.rawName.Contains (paramType)) paramByType.Add (param.rawName, param);
                }
            }
        }
        if (paramByType.IsEmpty ()) continue;

        bool needCompare = false; // Флаг необходимости записи в словарь. Поднимается только при наличии результата на каждом этапе

        // Проходим поиском, специфичным для каждого типа
        if (paramType.IsEqual ("{@property:")) {
            needCompare = ParamHelpers::ReadProperty (elemGuid, paramByType);
            // Среди прочитанных свойств могут быть свойства с классификацией. Поищем классификацию по имени
            if (needCompare) {
                for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cItt = paramByType.EnumeratePairs (); cItt != NULL; ++cItt) {
                    #if defined(AC_28)
                    ParamValue& parambt = cItt->value;
                    #else
                    ParamValue& parambt = *cItt->value;
                    #endif
                    if (parambt.fromClassification) {
                        if (systemdict.IsEmpty ()) err = ClassificationFunc::GetAllClassification (systemdict);
                        GS::UniString systemname = parambt.val.uniStringValue.ToLowerCase ();
                        API_Guid classguid = ClassificationFunc::FindClass (systemdict, parambt.name, systemname);
                        if (classguid != APINULLGuid && parambt.val.guidval == APINULLGuid) {
                            paramByType.Get (parambt.rawName).val.guidval = classguid;
                        } else {
                            #if defined(TESTING)
                            if (parambt.val.guidval != APINULLGuid) DBprnt ("Compare classification err - double class");
                            if (classguid == APINULLGuid) DBprnt ("Compare classification err - empty class");
                            #endif
                        }
                    }
                }
            }
        }
        if (paramType.IsEqual ("{@coord:")) {
            // Для определения угла к северу нам потребуется значение направления на север.
            // Оно должно быть в Info, проверим и добавим, если оно есть
            GS::UniString globnorthkey = "{@glob:glob_north_dir}";
            if (propertyParams.ContainsKey (globnorthkey) && !paramByType.ContainsKey (globnorthkey)) {
                paramByType.Add (globnorthkey, propertyParams.Get (globnorthkey));
            }
            GS::UniString sync_coord_correctkey = "{@property:sync_correct_flag}";
            if (params.ContainsKey (sync_coord_correctkey) && !paramByType.ContainsKey (sync_coord_correctkey)) {
                paramByType.Add (sync_coord_correctkey, params.Get (sync_coord_correctkey));
            }
            GS::UniString locorig = "{@coord:locorigin_x}";
            if (propertyParams.ContainsKey (locorig) && !paramByType.ContainsKey (locorig)) paramByType.Add (locorig, propertyParams.Get (locorig));
            locorig = "{@coord:locorigin_y}";
            if (propertyParams.ContainsKey (locorig) && !paramByType.ContainsKey (locorig)) paramByType.Add (locorig, propertyParams.Get (locorig));
            locorig = "{@coord:locorigin_z}";
            if (propertyParams.ContainsKey (locorig) && !paramByType.ContainsKey (locorig)) paramByType.Add (locorig, propertyParams.Get (locorig));
            locorig = "{@coord:offsetorigin_x}";
            if (propertyParams.ContainsKey (locorig) && !paramByType.ContainsKey (locorig)) paramByType.Add (locorig, propertyParams.Get (locorig));
            locorig = "{@coord:offsetorigin_y}";
            if (propertyParams.ContainsKey (locorig) && !paramByType.ContainsKey (locorig)) paramByType.Add (locorig, propertyParams.Get (locorig));
            needCompare = ParamHelpers::ReadCoords (element, paramByType);
        }
        if (paramType.IsEqual ("{@gdl:") && can_read_fromGDL) {
            needCompare = ParamHelpers::ReadGDL (element, elem_head, paramByType);
        }
        if (paramType.IsEqual ("{@ifc:")) {
            needCompare = ParamHelpers::ReadIFC (elemGuid, paramByType);
        }
        if (paramType.IsEqual ("{@morph:") && eltype == API_MorphID) {
            needCompare = ParamHelpers::ReadMorphParam (element, paramByType);
        }
        if (paramType.IsEqual ("{@id:")) {
            needCompare = ParamHelpers::ReadID (elem_head, paramByType);
        }
        if (paramType.IsEqual ("{@class:")) {
            if (systemdict.IsEmpty ()) err = ClassificationFunc::GetAllClassification (systemdict);
            needCompare = ParamHelpers::ReadClassification (elemGuid, systemdict, paramByType);
        }
        if (paramType.IsEqual ("{@material:") && can_read_fromMaterial) {
            ParamHelpers::ReadMaterial (element, params, propertyParams);
        }
        if (paramType.IsEqual ("{@formula:")) {
            needCompare = ParamHelpers::ReadFormula (paramByType, params);
        }
        if (paramType.IsEqual ("{@attrib:")) {
            needCompare = ParamHelpers::ReadAttributeValues (elem_head, propertyParams, paramByType);
        }
        if (paramType.IsEqual ("{@element:")) {
            needCompare = ParamHelpers::ReadElementValues (element, paramByType);
        }
        if (paramType.IsEqual ("{@mep:")) {
            #if defined (AC_28)
            needCompare = MEPv1::ReadMEP (elem_head, paramByType);
            #endif
        }
        if (paramType.IsEqual ("{@listdata:") && eltype == API_ObjectID) {
            needCompare = ParamHelpers::ReadListData (elem_head, paramByType);
        }
        if (needCompare) {
            #if defined(TESTING)
            DBprnt ("        CompareParamDictValue");
            #endif
            ParamHelpers::CompareParamDictValue (paramByType, params);
        } else {
            if (!paramType.IsEqual ("{@material:")) {
                #if defined(TESTING)
                DBprnt ("Read err", "not found" + paramType);
                #endif
            }
        }
    }
    #if defined(TESTING)
    DBprnt ("        ConvertByFormatString");
    #endif
    for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = params.EnumeratePairs (); cIt != NULL; ++cIt) {
        #if defined(AC_28)
        ParamValue& param = cIt->value;
        #else
        ParamValue& param = *cIt->value;
        #endif
        if (param.isValid && param.val.canCalculate && !param.val.hasFormula) {
            ParamHelpers::ConvertByFormatString (param);
        }
        if (param.fromPropertyDefinition && param.isValid) {
            if (param.definition.description.Contains ("Sync_to{Attribute:Layer}")) {
                if (ParamHelpers::hasAttribute (propertyParams)) {
                    GS::UniString key = "{@attrib:layer_name_" + param.val.uniStringValue.ToLowerCase () + "}";
                    if (propertyParams.ContainsKey (key)) {
                        param.val = propertyParams.Get (key).val;
                    } else {
                        param.isValid = false;
                    }
                } else {
                    param.val.intValue = 0;
                }
                param.fromAttribElement = true;
            }
        }
        if (param.isValid && param.toQRCode) {
            GS::UniString qr = TextToQRCode (param.val.uniStringValue);
            param.val.uniStringValue = qr;
        }
    }
}

// --------------------------------------------------------------------
// Заполнение информации о локальном начале координат
// --------------------------------------------------------------------
void ParamHelpers::GetLocOriginToParamDict (ParamDictValue& propertyParams)
{
    #if defined(TESTING)
    DBprnt ("GetLocOriginToParamDict start");
    #endif
    //Пользовательское начало
    API_Coord3D locOrigin;
    API_Coord offset;
    GSErrCode err = NoError;
    #if defined AC_27 || defined AC_28
    err = ACAPI_Database_GetLocOrigo (&locOrigin);
    #else
    err = ACAPI_Database (APIDb_GetLocOrigoID, &locOrigin);
    #endif
    if (err != NoError) {
        msg_rep ("GetLocOriginToParamDict", "APIDb_GetLocOrigoID", err, APINULLGuid);
        return;
    }
    #if defined AC_27 || defined AC_28
    err = ACAPI_ProjectSetting_GetOffset (&offset);
    #else
    err = ACAPI_Database (APIDb_GetOffsetID, &offset);
    #endif
    if (err != NoError) {
        msg_rep ("GetLocOriginToParamDict", "APIDb_GetOffsetID", err, APINULLGuid);
        return;
    }
    GS::UniString prefix = "{@coord:";
    GS::UniString suffix = "}";
    ParamValue pvalue = {};
    pvalue.val.formatstring = FormatStringFunc::ParseFormatString ("1mm");
    pvalue.fromCoord = true;

    pvalue.name = "locOrigin_x";
    pvalue.rawName = prefix; pvalue.rawName.Append (pvalue.name.ToLowerCase ()); pvalue.rawName.Append (suffix);
    ParamHelpers::ConvertDoubleToParamValue (pvalue, "", locOrigin.x + offset.x);
    propertyParams.Add (pvalue.rawName, pvalue);

    pvalue.name = "locOrigin_y";
    pvalue.rawName = prefix; pvalue.rawName.Append (pvalue.name.ToLowerCase ()); pvalue.rawName.Append (suffix);
    ParamHelpers::ConvertDoubleToParamValue (pvalue, "", locOrigin.y + offset.y);
    propertyParams.Add (pvalue.rawName, pvalue);

    pvalue.name = "locOrigin_z";
    pvalue.rawName = prefix; pvalue.rawName.Append (pvalue.name.ToLowerCase ()); pvalue.rawName.Append (suffix);
    ParamHelpers::ConvertDoubleToParamValue (pvalue, "", locOrigin.z);
    propertyParams.Add (pvalue.rawName, pvalue);

    pvalue.name = "offsetOrigin_x";
    pvalue.rawName = prefix; pvalue.rawName.Append (pvalue.name.ToLowerCase ()); pvalue.rawName.Append (suffix);
    ParamHelpers::ConvertDoubleToParamValue (pvalue, "", offset.x);
    propertyParams.Add (pvalue.rawName, pvalue);

    pvalue.name = "offsetOrigin_y";
    pvalue.rawName = prefix; pvalue.rawName.Append (pvalue.name.ToLowerCase ()); pvalue.rawName.Append (suffix);
    ParamHelpers::ConvertDoubleToParamValue (pvalue, "", offset.y);
    propertyParams.Add (pvalue.rawName, pvalue);

    ParamHelpers::AddValueToParamDictValue (propertyParams, "flag:has_LocOrigin");
    #if defined(TESTING)
    DBprnt ("  GetLocOriginToParamDict end");
    #endif
}

// --------------------------------------------------------------------
// Заполнение информации о проекте
// --------------------------------------------------------------------
void ParamHelpers::GetAllInfoToParamDict (ParamDictValue& propertyParams)
{
    #if defined(TESTING)
    DBprnt ("GetAllInfoToParamDict start");
    #endif
    GS::Array<GS::ArrayFB<GS::UniString, 3> >	autotexts;
    API_AutotextType	type = APIAutoText_Custom;
    GSErrCode err = NoError;
    #if defined(AC_27) || defined(AC_28)
    err = ACAPI_AutoText_GetAutoTexts (&autotexts, type);
    #else
    err = ACAPI_Goodies (APIAny_GetAutoTextsID, &autotexts, (void*) (GS::IntPtr) type);
    #endif
    if (err != NoError) {
        msg_rep ("GetAllInfoToParamDict", "APIAny_GetAutoTextsID", err, APINULLGuid);
        return;
    }
    GS::UniString rawName = "";
    GS::UniString prefix = "{@info:";
    GS::UniString suffix = "}";
    for (UInt32 i = 0; i < autotexts.GetSize (); i++) {
        rawName = prefix;
        rawName.Append (autotexts[i][0].ToLowerCase ());
        rawName.Append (suffix);
        if (!propertyParams.ContainsKey (rawName)) {
            ParamValue pvalue;
            pvalue.name = autotexts[i][1];
            pvalue.rawName = rawName;
            pvalue.fromInfo = true;
            ParamHelpers::ConvertStringToParamValue (pvalue, rawName, autotexts[i][2]);
            propertyParams.Add (rawName, pvalue);
        }
    }
    ParamHelpers::AddValueToParamDictValue (propertyParams, "flag:has_Info");
    #if defined(TESTING)
    DBprnt ("  GetAllInfoToParamDict end");
    #endif
}

// --------------------------------------------------------------------
// Получение списка аттрибутов (имён слоёв, материалов)
// --------------------------------------------------------------------
void ParamHelpers::GetAllAttributeToParamDict (ParamDictValue& propertyParams)
{
    #if defined(TESTING)
    DBprnt ("GetAllAttributeToParamDict start");
    #endif
    API_Attribute attrib = {};
    GSErrCode err = NoError;
    #if defined(AC_27) || defined(AC_28)
    UInt32 nAttr;
    err = ACAPI_Attribute_GetNum (API_LayerID, nAttr);
    #else
    API_AttributeIndex  nAttr;
    err = ACAPI_Attribute_GetNum (API_LayerID, &nAttr);
    #endif
    if (err != NoError) {
        msg_rep ("GetAllAttributeToParamDict", "ACAPI_Attribute_GetNum", err, APINULLGuid);
        return;
    }
    #if defined(AC_27) || defined(AC_28)
    for (UInt32 i = 1; i <= nAttr && err == NoError; i++) {
        #else
    for (API_AttributeIndex i = 1; i <= nAttr && err == NoError; i++) {
        #endif
        BNZeroMemory (&attrib, sizeof (API_Attribute));
        attrib.header.typeID = API_LayerID;
        #if defined(AC_27) || defined(AC_28)
        attrib.header.index = ACAPI_CreateAttributeIndex (i);
        #else
        attrib.header.index = i;
        #endif
        GS::UniString attribname = "";
        attrib.header.uniStringNamePtr = &attribname;
        err = ACAPI_Attribute_Get (&attrib);
        if (err == NoError) {
            ParamValue pvalue;
            GS::UniString rawName = "layer_name_" + attribname;
            ParamHelpers::ConvertAttributeToParamValue (pvalue, rawName, attrib);
            propertyParams.Add (pvalue.rawName, pvalue);
            pvalue.name = "";
            pvalue.rawName = "";
            rawName = "layer_inx_" + GS::UniString::Printf ("%d", attrib.header.index);
            ParamHelpers::ConvertAttributeToParamValue (pvalue, rawName, attrib);
            propertyParams.Add (pvalue.rawName, pvalue);
        } else {
            if (err == APIERR_DELETED)
                err = NoError;
            if (err != NoError) {
                msg_rep ("GetAllAttributeToParamDict", "ACAPI_Attribute_Get", err, APINULLGuid);
                return;
            }
        }
    }
    ParamHelpers::AddValueToParamDictValue (propertyParams, "flag:has_attrib");
    #if defined(TESTING)
    DBprnt ("  GetAllAttributeToParamDict end");
    #endif
}

// --------------------------------------------------------------------
// Получение списка глобальных переменных о местоположении проекта, солнца
// --------------------------------------------------------------------
void ParamHelpers::GetAllGlobToParamDict (ParamDictValue & propertyParams)
{
    #if defined(TESTING)
    DBprnt ("GetAllGlobToParamDict start");
    #endif
    GS::UniString name = "";
    GS::UniString rawName = "";
    GS::UniString prefix = "{@glob:";
    GS::UniString suffix = "}";
    ParamValue pvalue;
    API_PlaceInfo placeInfo = {};
    GSErrCode err = NoError;
    #if defined(AC_27) || defined(AC_28)
    err = ACAPI_GeoLocation_GetPlaceSets (&placeInfo);
    #else
    err = ACAPI_Environment (APIEnv_GetPlaceSetsID, &placeInfo, nullptr);
    #endif
    if (err != NoError) {
        msg_rep ("GetAllGlobToParamDict", "APIEnv_GetPlaceSetsID", err, APINULLGuid);
        return;
    }
    name = "GLOB_NORTH_DIR"; rawName = prefix; rawName.Append (name.ToLowerCase ()); rawName.Append (suffix);
    pvalue.name = name; pvalue.rawName = rawName;
    ParamHelpers::ConvertDoubleToParamValue (pvalue, rawName, round ((placeInfo.north * 180 / PI) * 1000.0) / 1000.0);
    propertyParams.Add (rawName, pvalue);
    name = "GLOB_PROJECT_LONGITUDE"; rawName = prefix; rawName.Append (name.ToLowerCase ()); rawName.Append (suffix);
    pvalue.name = name; pvalue.rawName = rawName;
    ParamHelpers::ConvertDoubleToParamValue (pvalue, rawName, placeInfo.longitude);
    propertyParams.Add (rawName, pvalue);
    name = "GLOB_PROJECT_LATITUDE"; rawName = prefix; rawName.Append (name.ToLowerCase ()); rawName.Append (suffix);
    pvalue.name = name; pvalue.rawName = rawName;
    ParamHelpers::ConvertDoubleToParamValue (pvalue, rawName, placeInfo.latitude);
    propertyParams.Add (rawName, pvalue);
    name = "GLOB_PROJECT_ALTITUDE"; rawName = prefix; rawName.Append (name.ToLowerCase ()); rawName.Append (suffix);
    pvalue.name = name; pvalue.rawName = rawName;
    ParamHelpers::ConvertDoubleToParamValue (pvalue, rawName, placeInfo.altitude);
    propertyParams.Add (rawName, pvalue);
    name = "GLOB_SUN_AZIMUTH"; rawName = prefix; rawName.Append (name.ToLowerCase ()); rawName.Append (suffix);
    pvalue.name = name; pvalue.rawName = rawName;
    ParamHelpers::ConvertDoubleToParamValue (pvalue, rawName, round ((placeInfo.sunAngXY * 180 / PI) * 1000.0) / 1000.0);
    propertyParams.Add (rawName, pvalue);
    name = "GLOB_SUN_ALTITUDE"; rawName = prefix; rawName.Append (name.ToLowerCase ()); rawName.Append (suffix);
    pvalue.name = name; pvalue.rawName = rawName;
    ParamHelpers::ConvertDoubleToParamValue (pvalue, rawName, round ((placeInfo.sunAngZ * 180 / PI) * 1000.0) / 1000.0);
    propertyParams.Add (rawName, pvalue);
    ParamHelpers::AddValueToParamDictValue (propertyParams, "flag:has_Glob");
    #if defined(TESTING)
    DBprnt ("  GetAllGlobToParamDict end");
    #endif
}

// --------------------------------------------------------------------
// Заполнение свойств для элемента
// --------------------------------------------------------------------
void ParamHelpers::AllPropertyDefinitionToParamDict (ParamDictValue & propertyParams, const API_Guid & elemGuid)
{
    #if defined(TESTING)
    DBprnt ("AllPropertyDefinitionToParamDict GUID start");
    #endif
    if (elemGuid == APINULLGuid) {
        if (!ParamHelpers::hasProperyDefinition (propertyParams)) {
            ParamHelpers::AllPropertyDefinitionToParamDict (propertyParams);
        }
    } else {
        GS::Array<API_PropertyDefinition> definitions = {};
        GSErrCode err = ACAPI_Element_GetPropertyDefinitions (elemGuid, API_PropertyDefinitionFilter_All, definitions);
        if (err != NoError) {
            msg_rep ("GetAllPropertyDefinitionToParamDict", "ACAPI_Element_GetPropertyDefinitions", err, elemGuid);
            return;
        }
        if (definitions.IsEmpty ()) return;
        ParamHelpers::AllPropertyDefinitionToParamDict (propertyParams, definitions);
    }
    #if defined(TESTING)
    DBprnt ("  AllPropertyDefinitionToParamDict GUID end");
    #endif
}

// --------------------------------------------------------------------
// Перевод GS::Array<API_PropertyDefinition> в ParamDictValue
// --------------------------------------------------------------------
void ParamHelpers::AllPropertyDefinitionToParamDict (ParamDictValue & propertyParams, GS::Array<API_PropertyDefinition>&definitions)
{
    if (definitions.IsEmpty ()) return;
    #if defined(TESTING)
    DBprnt ("AllPropertyDefinitionToParamDict definition start");
    #endif
    UInt32 nparams = propertyParams.GetSize ();
    bool needAddNew = (nparams == 0);
    for (const auto& definition : definitions) {
        ParamValue pvalue = {};
        ParamHelpers::ConvertToParamValue (pvalue, definition);
        bool changeExs = propertyParams.ContainsKey (pvalue.rawName);
        if (needAddNew && !changeExs) {
            propertyParams.Add (pvalue.rawName, pvalue);
        } else {
            if (changeExs) {
                pvalue.fromGuid = propertyParams.Get (pvalue.rawName).fromGuid;
                propertyParams.Get (pvalue.rawName) = pvalue;
                nparams--;
                if (nparams == 0) {
                    #if defined(TESTING)
                    DBprnt ("    AllPropertyDefinitionToParamDict definition return");
                    #endif
                    return;
                }
            }
        }
    }
    #if defined(TESTING)
    DBprnt ("  AllPropertyDefinitionToParamDict definition end");
    #endif
}

// --------------------------------------------------------------------
// Получение массива описаний свойств с указанием GUID родительского объекта
// --------------------------------------------------------------------
bool ParamHelpers::SubGuid_GetDefinition (const GS::Array<API_PropertyDefinition>&definitions, GS::Array<API_PropertyDefinition>&definitionsout)
{
    if (definitions.IsEmpty ()) return false;
    GS::HashTable<GS::UniString, API_PropertyDefinition> GuidDefinition = {};
    bool flag_find = false;
    for (UInt32 i = 0; i < definitions.GetSize (); i++) {
        if (!definitions[i].description.IsEmpty ()) {
            if (definitions[i].description.Contains ("Sync_GUID")) {
                definitionsout.Push (definitions[i]);
                flag_find = true;
            }
        }
    }
    return flag_find;
}

// --------------------------------------------------------------------
// Получение словаря значений свойств с указанием GUID родительского объекта
// --------------------------------------------------------------------
bool ParamHelpers::SubGuid_GetParamValue (const API_Guid & elemGuid, ParamDictValue & propertyParams, const GS::Array<API_PropertyDefinition>&definitions, ParamDictValue & subproperty)
{
    if (definitions.IsEmpty ()) return false;
    GS::Array<API_PropertyDefinition> subdefinitions = {};
    if (!SubGuid_GetDefinition (definitions, subdefinitions)) return false;
    ParamHelpers::AllPropertyDefinitionToParamDict (subproperty, subdefinitions);
    ClassificationFunc::SystemDict systemdict = {};
    ParamHelpers::Read (elemGuid, subproperty, propertyParams, systemdict);
    #if defined(TESTING)
    if (subproperty.IsEmpty ()) {
        DBprnt ("SubGuid_GetParamValue not found");
    } else {
        DBprnt ("SubGuid_GetParamValue FOUND");
    }
    #endif
    if (propertyParams.IsEmpty ()) {
        propertyParams = subproperty;
    } else {
        ParamHelpers::CompareParamDictValue (subproperty, propertyParams, true);
    }
    return true;
}

// --------------------------------------------------------------------
// Получить все доступные свойства в формарте ParamDictValue
// --------------------------------------------------------------------
void ParamHelpers::AllPropertyDefinitionToParamDict (ParamDictValue & propertyParams)
{
    GS::Array<API_PropertyGroup> groups = {};
    #if defined(TESTING)
    DBprnt ("AllPropertyDefinitionToParamDict start");
    #endif
    if (ParamHelpers::hasProperyDefinition (propertyParams)) {
        #if defined(TESTING)
        DBprnt ("  AllPropertyDefinitionToParamDict READ BEFORE end");
        #endif
        return;
    }
    GSErrCode err = ACAPI_Property_GetPropertyGroups (groups);
    if (err != NoError) {
        msg_rep ("GetAllPropertyDefinitionToParamDict", "ACAPI_Property_GetPropertyGroups", err, APINULLGuid);
        return;
    }
    UInt32 nparams = propertyParams.GetSize ();
    GS::UniString name = "";
    GS::UniString rawName = "";
    GS::UniString prefix = "{@property:";
    // Созданим словарь с определением всех свойств
    for (UInt32 i = 0; i < groups.GetSize (); i++) {
        bool filter = true;
        #if defined(AC_28)
        GS::UniString strguid = APIGuidToString (groups[i].guid);
        filter = (strguid.IsEqual ("3CF63E55-AA52-4AB4-B1C3-0920B2F352BF") || strguid.IsEqual ("6EE946D2-E840-4909-8EF1-F016AE905C52") || strguid.IsEqual ("BF31D3E0-A2B1-4543-A3DA-C1191D059FD8"));
        //TODO Дописать Guid группы "GeneralElemProperties"
        #else
        filter = (groups[i].name.Contains ("Material") || groups[i].name.IsEqual ("GeneralElemProperties"));
        #endif
        if (groups[i].groupType == API_PropertyCustomGroupType || (groups[i].groupType == API_PropertyStaticBuiltInGroupType && filter)) {
            GS::Array<API_PropertyDefinition> definitions = {};
            err = ACAPI_Property_GetPropertyDefinitions (groups[i].guid, definitions);
            if (err != NoError) msg_rep ("GetPropertyByName", "ACAPI_Property_GetPropertyDefinitions", err, APINULLGuid);
            if (err == NoError) {
                for (UInt32 j = 0; j < definitions.GetSize (); j++) {
                    if (definitions[j].availability.IsEmpty () && groups[i].groupType == API_PropertyCustomGroupType) {
                        #if defined(TESTING)
                        DBprnt ("AllPropertyDefinitionToParamDict skip " + definitions[j].name);
                        #endif
                        continue;
                    }
                    // TODO Когда в проекте есть два и более свойств с описанием Sync_name возникает ошибка
                    if (definitions[j].description.Contains ("Sync_name")) {
                        for (UInt32 inx = 0; inx < 20; inx++) {
                            GS::UniString strinx = GS::UniString::Printf ("%d", inx);
                            rawName = "{@property:sync_name";
                            rawName.Append (strinx);
                            rawName.Append ("}");
                            name = "sync_name";
                            name.Append (strinx);
                            if (!propertyParams.ContainsKey (rawName)) break;
                        }
                        definitions[j].name = name;
                        ParamValue pvalue = {};
                        pvalue.rawName = rawName;
                        pvalue.name = groups[i].name;
                        pvalue.name.Append ("/");
                        pvalue.name.Append (definitions[j].name);
                        ParamHelpers::ConvertToParamValue (pvalue, definitions[j]);
                        propertyParams.Add (pvalue.rawName, pvalue);
                    } else {
                        #if defined(AC_28)
                        name = GetPropertyNameByGUID (definitions[j].guid);
                        if (name.IsEmpty ()) {
                            name = groups[i].name;
                            name.Append ("/");
                            name.Append (definitions[j].name);
                        }
                        #else
                        name = groups[i].name;
                        name.Append ("/");
                        name.Append (definitions[j].name);
                        #endif
                        rawName = prefix;
                        rawName.Append (name.ToLowerCase ());
                        rawName.Append ("}");
                        if (!propertyParams.ContainsKey (rawName)) {
                            ParamValue pvalue;
                            pvalue.rawName = rawName;
                            pvalue.name = name;
                            ParamHelpers::ConvertToParamValue (pvalue, definitions[j]);
                            propertyParams.Add (pvalue.rawName, pvalue);
                        } else {
                            ParamValue pvalue = propertyParams.Get (rawName);
                            FormatString fstring = pvalue.val.formatstring;
                            if (!pvalue.fromPropertyDefinition && !pvalue.fromAttribDefinition) {
                                pvalue.rawName = rawName;
                                pvalue.name = name;
                                ParamHelpers::ConvertToParamValue (pvalue, definitions[j]);
                                if (!fstring.isEmpty) {
                                    pvalue.val.formatstring = fstring;
                                }
                                propertyParams.Get (pvalue.rawName) = pvalue;
                                nparams--;
                                if (nparams == 0) {
                                    #if defined(TESTING)
                                    DBprnt ("    AllPropertyDefinitionToParamDict return");
                                    #endif
                                    return;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    ParamHelpers::AddValueToParamDictValue (propertyParams, "flag:has_properydefinition");
    #if defined(TESTING)
    DBprnt ("  AllPropertyDefinitionToParamDict end");
    #endif
}

// --------------------------------------------------------------------
// Сопоставление двух словарей ParamDictElement
// --------------------------------------------------------------------
void ParamHelpers::CompareParamDictElement (ParamDictElement & paramsFrom, ParamDictElement & paramsTo)
{
    if (paramsFrom.IsEmpty () || paramsTo.IsEmpty ()) return;
    for (auto& cIt : paramsTo) {
        #if defined(AC_28)
        ParamDictValue& paramTo = cIt.value;
        API_Guid elemGuid = cIt.key;
        #else
        ParamDictValue& paramTo = *cIt.value;
        API_Guid elemGuid = *cIt.key;
        #endif
        if (paramsFrom.ContainsKey (elemGuid)) {
            ParamHelpers::CompareParamDictValue (paramsFrom.Get (elemGuid), paramTo, false);
        }
    }
}

// --------------------------------------------------------------------
// Сопоставление двух словарей ParamDictValue 
// Не добавляет отсутствующие в paramsTo элементы
// --------------------------------------------------------------------
void ParamHelpers::CompareParamDictValue (ParamDictValue & paramsFrom, ParamDictValue & paramsTo)
{
    ParamHelpers::CompareParamDictValue (paramsFrom, paramsTo, false);
}

// --------------------------------------------------------------------
// Сопоставление двух словарей ParamDictValue
// --------------------------------------------------------------------
void ParamHelpers::CompareParamDictValue (ParamDictValue & paramsFrom, ParamDictValue & paramsTo, bool addInNotEx)
{
    if (paramsFrom.IsEmpty () || paramsTo.IsEmpty ()) return;
    for (auto& cIt : paramsFrom) {
        #if defined(AC_28)
        GS::UniString k = cIt.key;
        #else
        GS::UniString k = *cIt.key;
        #endif
        if (paramsTo.ContainsKey (k)) {
            ParamValue paramFrom = paramsFrom.Get (k);
            paramFrom.fromGuid = paramsTo.Get (k).fromGuid; // Чтоб GUID не перезаписался
            paramFrom.toQRCode = paramsTo.Get (k).toQRCode;
            paramsTo.Set (k, paramFrom);
        } else {
            if (addInNotEx) {
                ParamValue paramFrom = paramsFrom.Get (k);
                paramsTo.Add (k, paramFrom);
            }
        }
    }
    return;
}

// --------------------------------------------------------------------
// Чтение значений свойств в ParamDictValue
// --------------------------------------------------------------------
bool ParamHelpers::ReadProperty (const API_Guid & elemGuid, ParamDictValue & params)
{
    if (params.IsEmpty ()) return false;
    #if defined(TESTING)
    DBprnt ("    ReadProperty");
    #endif
    // Определения и свойста для элементов
    GS::Array<API_PropertyDefinition> propertyDefinitions = {};
    for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = params.EnumeratePairs (); cIt != NULL; ++cIt) {
        #if defined(AC_28)
        ParamValue& param = cIt->value;
        #else
        ParamValue& param = *cIt->value;
        #endif
        if (param.fromPropertyDefinition) {
            API_PropertyDefinition definition = param.definition;
            #ifdef TESTING
            if (!param.rawName.Contains ("nosyncname") && definition.guid == APINULLGuid) DBtest (definition.guid != APINULLGuid, param.rawName, false);
            #endif 
            if (definition.guid != APINULLGuid) {
                propertyDefinitions.Push (definition);
            }
        }
    }
    if (!propertyDefinitions.IsEmpty ()) {
        GS::Array<API_Property> properties = {};
        GSErrCode error = ACAPI_Element_GetPropertyValues (elemGuid, propertyDefinitions, properties);
        if (error != NoError) {
            msg_rep ("ParamDictGetPropertyValues", "ACAPI_Element_GetPropertyValues", error, elemGuid);
            return false;
        }
        return (ParamHelpers::AddProperty (params, properties));
    }
    #if defined(TESTING)
    DBprnt ("ReadProperty err", "no property");
    #endif
    return false;
}

// -----------------------------------------------------------------------------
// Получение значения IFC свойств
// -----------------------------------------------------------------------------
bool ParamHelpers::ReadIFC (const API_Guid & elemGuid, ParamDictValue & params)
{
    if (params.IsEmpty ()) return false;
    #if defined(TESTING)
    DBprnt ("    ReadIFC");
    #endif
    GS::Array<API_IFCProperty> properties = {};
    GSErrCode err = ACAPI_Element_GetIFCProperties (elemGuid, false, &properties);
    if (err != NoError) {
        msg_rep ("ParamDictGetIFCValues", "ACAPI_Element_GetIFCProperties", err, elemGuid);
        return false;
    }
    bool flag_find = false;
    UInt32 nparams = params.GetSize ();
    GS::UniString fname = "";
    GS::UniString rawName = "";
    for (UInt32 i = 0; i < properties.GetSize (); i++) {
        API_IFCProperty property = properties.Get (i);
        fname = properties.Get (i).head.propertySetName;
        fname.Append ("/");
        fname.Append (properties.Get (i).head.propertyName);

        rawName = "{@ifc:";
        rawName.Append (fname.ToLowerCase ());
        rawName.Append ("}");
        if (params.ContainsKey (rawName)) {
            ParamValue pvalue;
            if (ParamHelpers::ConvertToParamValue (pvalue, property)) {
                params.Get (rawName) = pvalue;
                flag_find = true;
            }
            nparams--;
            if (nparams == 0) {
                return flag_find;
            }
        } else {
            fname = properties[i].head.propertyName;
            rawName = "{@ifc:";
            rawName.Append (fname.ToLowerCase ());
            rawName.Append ("}");
            if (params.ContainsKey (rawName)) {
                ParamValue pvalue = {};
                if (ParamHelpers::ConvertToParamValue (pvalue, property)) {
                    params.Get (rawName) = pvalue;
                    flag_find = true;
                }
                nparams--;
                if (nparams == 0) {
                    return flag_find;
                }
            }
        }
    }
    return flag_find;
}

// -----------------------------------------------------------------------------
// Обработка данных о классификации
// -----------------------------------------------------------------------------
bool ParamHelpers::ReadClassification (const API_Guid & elemGuid, const ClassificationFunc::SystemDict & systemdict, ParamDictValue & paramByType)
{
    #if defined(TESTING)
    DBprnt ("    ReadClassification");
    #endif
    if (systemdict.IsEmpty ()) return false;
    GSErrCode err = NoError;
    GS::HashTable<GS::UniString, API_Guid> elementsystem = {};
    for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = paramByType.EnumeratePairs (); cIt != NULL; ++cIt) {
        #if defined(AC_28)
        ParamValue& param = cIt->value;
        #else
        ParamValue& param = *cIt->value;
        #endif
        if (systemdict.ContainsKey (param.name)) {
            if (systemdict.Get (param.name).ContainsKey ("@system@")) {
                API_Guid systemguid = systemdict.Get (param.name).Get ("@system@").system.guid;
                elementsystem.Add (param.name, systemguid);
            }
        } else {
            param.val.uniStringValue = ""; // Если система не найдена - обнулим значение
            msg_rep ("System not found", param.name, NoError, APINULLGuid);
        }
    }
    if (elementsystem.IsEmpty ()) return false;
    bool flag_find = false;
    for (GS::HashTable<GS::UniString, API_Guid>::PairIterator cIt = elementsystem.EnumeratePairs (); cIt != NULL; ++cIt) {
        #if defined(AC_28)
        API_Guid systemguid = cIt->value;
        GS::UniString systemname = cIt->key;
        #else
        API_Guid systemguid = *cIt->value;
        GS::UniString systemname = *cIt->key;
        #endif
        API_ClassificationItem item = {};
        err = ACAPI_Element_GetClassificationInSystem (elemGuid, systemguid, item);
        if (err == NoError) {
            GS::UniString rawname = "{@class:" + systemname + ";fullname}";
            if (paramByType.ContainsKey (rawname)) {
                GS::UniString fullname = "";
                ClassificationFunc::GetFullName (item, systemdict.Get (systemname), fullname);
                paramByType.Get (rawname).isValid = true;
                paramByType.Get (rawname).val.uniStringValue = fullname;
                paramByType.Get (rawname).val.type = API_PropertyStringValueType;
                flag_find = true;
            }
            rawname = "{@class:" + systemname + "}";
            if (paramByType.ContainsKey (rawname)) {
                paramByType.Get (rawname).isValid = true;
                paramByType.Get (rawname).val.guidval = item.guid;
                GS::UniString fullname = "";
                ClassificationFunc::GetFullName (item, systemdict.Get (systemname), fullname);
                paramByType.Get (rawname).val.uniStringValue = fullname;
                paramByType.Get (rawname).val.type = API_PropertyGuidValueType;
                flag_find = true;
            }
        } else {
            msg_rep ("ReadClassification", "ACAPI_Element_GetClassificationInSystem", err, systemguid);
        }
    }
    return flag_find;
}

// -----------------------------------------------------------------------------
// Получение аттрибутов элемента
// -----------------------------------------------------------------------------
bool ParamHelpers::ReadAttributeValues (const API_Elem_Head & elem_head, ParamDictValue & propertyParams, ParamDictValue & params)
{
    if (params.IsEmpty ()) return false;
    #if defined(TESTING)
    DBprnt ("    ReadAttributeValues");
    #endif
    if (!params.ContainsKey ("{@attrib:layer}")) return false;
    if (!ParamHelpers::hasAttribute (propertyParams)) ParamHelpers::GetAllAttributeToParamDict (propertyParams);
    if (ParamHelpers::hasAttribute (propertyParams)) {
        #if defined(AC_27) || defined(AC_28)
        GS::Int32 intValue = elem_head.layer.ToInt32_Deprecated ();
        #else
        GS::Int32 intValue = elem_head.layer;
        #endif
        GS::UniString name = "{@attrib:layer_inx_" + GS::UniString::Printf ("%d", intValue) + "}";
        if (propertyParams.ContainsKey (name)) {
            params.Get ("{@attrib:layer}").val = propertyParams.Get (name).val;
            params.Get ("{@attrib:layer}").isValid = true;
            params.Get ("{@attrib:layer}").fromAttribElement = true;
            return true;
        } else {
            msg_rep ("ParamHelpers::ReadAttributeValues", "Layer not found - " + name, NoError, elem_head.guid);
        }
    } else {
        API_Attribute attrib = {};
        GS::UniString name = "";
        BNZeroMemory (&attrib, sizeof (API_Attribute));
        attrib.header.typeID = API_LayerID;
        attrib.header.index = elem_head.layer;
        GSErrCode error = ACAPI_Attribute_Get (&attrib);
        if (error != NoError) {
            msg_rep ("ParamHelpers::ReadAttributeValues", "ACAPI_Attribute_Get", error, elem_head.guid);
            return false;
        };
        ParamValue pvalue = {};
        ParamHelpers::ConvertAttributeToParamValue (pvalue, "layer", attrib);
        params.Get ("{@attrib:layer}").val = pvalue.val;
        params.Get ("{@attrib:layer}").isValid = true;
        params.Get ("{@attrib:layer}").fromAttribElement = true;
        return true;
    }
    return false;
}

// -----------------------------------------------------------------------------
// Получение ID элемента
// -----------------------------------------------------------------------------
bool ParamHelpers::ReadID (const API_Elem_Head & elem_head, ParamDictValue & params)
{
    if (params.IsEmpty ()) return false;
    #if defined(TESTING)
    DBprnt ("    ReadID");
    #endif
    if (!params.ContainsKey ("{@id:id}")) return false;
    ParamValue& param = params.Get ("{@id:id}");
    GS::UniString infoString = "";
    API_Guid elguid = elem_head.guid;
    GSErrCode err = NoError;
    #if defined(AC_27) || defined(AC_28)
    err = ACAPI_Element_GetElementInfoString (&elguid, &infoString);
    #else
    err = ACAPI_Database (APIDb_GetElementInfoStringID, &elguid, &infoString);
    #endif
    if (err != NoError) {
        msg_rep ("ReadID - ID", "ACAPI_Database(APIDb_GetElementInfoStringID", err, elguid);
        return false;
    } else {
        param.isValid = true;
        param.val.type = API_PropertyStringValueType;
        param.type = API_PropertyStringValueType;
        param.val.boolValue = !infoString.IsEmpty ();
        if (UniStringToDouble (infoString, param.val.doubleValue)) {
            param.val.intValue = (GS::Int32) param.val.doubleValue;
            param.val.canCalculate = true;
        } else {
            param.val.intValue = !infoString.IsEmpty ();
            param.val.doubleValue = param.val.intValue * 1.0;
        }
        param.val.rawDoubleValue = param.val.doubleValue;
        param.val.hasrawDouble = true;
        param.val.uniStringValue = infoString;
        params.Set (param.rawName, param);
        return true;
    }
}

// -----------------------------------------------------------------------------
// Получить значение GDL параметра по его имени или описанию в ParamValue
// -----------------------------------------------------------------------------
bool ParamHelpers::ReadGDL (const API_Element & element, const API_Elem_Head & elem_head, ParamDictValue & params)
{
    if (params.IsEmpty ()) return false;
    #if defined(TESTING)
    DBprnt ("    ReadGDL");
    #endif
    API_ElemTypeID eltype = GetElemTypeID (elem_head);
    // Обрабатываем только вложенные элементы иерархических структур (навесных стен и ограждений)
    if (eltype == API_RailingID || eltype == API_CurtainWallID) {
        return false;
    }
    ParamDictValue paramBydescription = {};
    ParamDictValue paramByName = {};
    GS::HashTable<GS::UniString, GS::Array<GS::UniString>> paramnamearray;

    // Если диапазоны массивов хранятся в параметра х - прочитаем сначала их
    ParamDictValue paramdiap = {};
    for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = params.EnumeratePairs (); cIt != NULL; ++cIt) {
        #if defined(AC_28)
        ParamValue& param = cIt->value;
        #else
        ParamValue& param = *cIt->value;
        #endif
        if (param.needPreRead) {
            if (!param.rawName_row_start.IsEmpty ()) {
                ParamValue arr_row_start = {};
                arr_row_start.fromGDLparam = true;
                arr_row_start.rawName = param.rawName_row_start;
                paramdiap.Add (arr_row_start.rawName, arr_row_start);
            }
            if (!param.rawName_row_end.IsEmpty ()) {
                ParamValue arr_row_end = {};
                arr_row_end.fromGDLparam = true;
                arr_row_end.rawName = param.rawName_row_end;
                paramdiap.Add (arr_row_end.rawName, arr_row_end);
            }
            if (!param.rawName_col_start.IsEmpty ()) {
                ParamValue arr_col_start = {};
                arr_col_start.fromGDLparam = true;
                arr_col_start.rawName = param.rawName_col_start;
                paramdiap.Add (arr_col_start.rawName, arr_col_start);
            }
            if (!param.rawName_col_end.IsEmpty ()) {
                ParamValue arr_col_end = {};
                arr_col_end.fromGDLparam = true;
                arr_col_end.rawName = param.rawName_col_end;
                paramdiap.Add (arr_col_end.rawName, arr_col_end);
            }
        }
    }
    if (!paramdiap.IsEmpty ()) {
        if (ParamHelpers::GDLParamByName (element, elem_head, paramdiap, paramnamearray)) {
            for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = params.EnumeratePairs (); cIt != NULL; ++cIt) {
                #if defined(AC_28)
                ParamValue& param = cIt->value;
                #else
                ParamValue& param = *cIt->value;
                #endif
                if (param.needPreRead) {
                    if (!param.rawName_row_start.IsEmpty ()) {
                        if (paramdiap.ContainsKey (param.rawName_row_start)) params.Get (param.rawName).val.array_row_start = paramdiap.Get (param.rawName_row_start).val.intValue;
                    }
                    if (!param.rawName_row_end.IsEmpty ()) {
                        if (paramdiap.ContainsKey (param.rawName_row_end)) params.Get (param.rawName).val.array_row_end = paramdiap.Get (param.rawName_row_end).val.intValue;
                    }
                    if (!param.rawName_col_start.IsEmpty ()) {
                        if (paramdiap.ContainsKey (param.rawName_col_start)) params.Get (param.rawName).val.array_column_start = paramdiap.Get (param.rawName_col_start).val.intValue;
                    }
                    if (!param.rawName_col_end.IsEmpty ()) {
                        if (paramdiap.ContainsKey (param.rawName_col_end)) params.Get (param.rawName).val.array_column_end = paramdiap.Get (param.rawName_col_end).val.intValue;
                    }
                }
            }
        }
    }

    // Разбиваем по типам поиска - по описанию/по имени
    for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = params.EnumeratePairs (); cIt != NULL; ++cIt) {
        #if defined(AC_28)
        ParamValue& param = cIt->value;
        #else
        ParamValue& param = *cIt->value;
        #endif
        GS::UniString rawName = param.rawName;
        if (param.fromGDLArray) {
            GS::Array<GS::UniString> tparams = {};
            UInt32 nparam = StringSplt (rawName, "@arr", tparams);

            // Проверим - были ли заданы числовые параметры для чтения (диапазоны и тип обработки массива)
            if ((param.val.array_row_start == 0 || param.val.array_row_end == 0 || param.val.array_column_start == 0 || param.val.array_column_end == 0 || param.val.array_format_out == ARRAY_UNDEF) && nparam > 1) {
                GS::Array<GS::UniString> tarray;
                UInt32 narray = StringSplt (tparams[1], "_", tarray);
                if (param.val.array_row_start == 0 && narray > 0) {
                    double doubleValue = 0;
                    if (UniStringToDouble (tarray[0], doubleValue)) {
                        param.val.array_row_start = (int) doubleValue;
                    }
                }
                if (param.val.array_row_end == 0 && narray > 1) {
                    double doubleValue = 0;
                    if (UniStringToDouble (tarray[1], doubleValue)) {
                        param.val.array_row_end = (int) doubleValue;
                    }
                }
                if (param.val.array_column_start == 0 && narray > 2) {
                    double doubleValue = 0;
                    if (UniStringToDouble (tarray[2], doubleValue)) {
                        param.val.array_column_start = (int) doubleValue;
                    }
                }
                if (param.val.array_column_end == 0 && narray > 3) {
                    double doubleValue = 0;
                    if (UniStringToDouble (tarray[3], doubleValue)) {
                        param.val.array_column_end = (int) doubleValue;
                    }
                }
                if (param.val.array_format_out == ARRAY_UNDEF && narray > 4) {
                    double doubleValue = 0;
                    if (UniStringToDouble (tarray[4], doubleValue)) {
                        param.val.array_format_out = (int) doubleValue;
                    }
                }
            }
            if (param.val.array_format_out == ARRAY_UNDEF) param.val.array_format_out = ARRAY_SUM;
            rawName = tparams[0] + "}";
            if (!paramnamearray.ContainsKey (rawName)) {
                GS::Array<GS::UniString> t = {};
                paramnamearray.Add (rawName, t);
            }
            paramnamearray.Get (rawName).Push (param.rawName);
            if (param.fromGDLdescription && eltype == API_ObjectID) {
                paramBydescription.Add (rawName, param);
            } else {
                if (param.fromGDLparam) paramByName.Add (rawName, param);
            }
        }
        if (param.fromGDLdescription && eltype == API_ObjectID) {
            paramBydescription.Add (param.rawName, param);
        } else {
            if (param.fromGDLparam) paramByName.Add (param.rawName, param);
        }
    }
    if (paramBydescription.IsEmpty () && paramByName.IsEmpty ()) return false;

    // Поиск по описанию
    bool flag_find_desc = false;
    bool flag_find_name = false;
    if (!paramBydescription.IsEmpty ()) {
        flag_find_desc = ParamHelpers::GDLParamByDescription (element, paramBydescription, paramByName, paramnamearray);
    }
    if (!paramByName.IsEmpty ()) flag_find_name = ParamHelpers::GDLParamByName (element, elem_head, paramByName, paramnamearray);
    if (flag_find_desc && flag_find_name) {
        for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = paramBydescription.EnumeratePairs (); cIt != NULL; ++cIt) {
            #if defined(AC_28)
            ParamValue& param_by_desc = cIt->value;
            #else
            ParamValue& param_by_desc = *cIt->value;
            #endif
            GS::UniString rawname = param_by_desc.name;
            if (paramByName.ContainsKey (rawname)) {
                ParamValue param_by_name = paramByName.Get (rawname);
                GS::UniString desc_name = param_by_desc.val.uniStringValue;
                GS::UniString desc_rawname = param_by_desc.rawName;
                param_by_name.name = desc_name;
                param_by_name.rawName = desc_rawname;
                #if defined(AC_28)
                paramByName.Add (cIt->key, param_by_name);
                #else
                paramByName.Add (*cIt->key, param_by_name);
                #endif
            }
        }
    }
    if (flag_find_name) ParamHelpers::CompareParamDictValue (paramByName, params);
    return (flag_find_name);
}

// -----------------------------------------------------------------------------
// Поиск по описанию GDL параметра
// Данный способ не работает с элементами навесных стен
// -----------------------------------------------------------------------------
bool ParamHelpers::GDLParamByDescription (const API_Element & element, ParamDictValue & params, ParamDictValue & find_params, GS::HashTable<GS::UniString, GS::Array<GS::UniString>>&paramnamearray)
{
    API_LibPart libpart = {};
    BNZeroMemory (&libpart, sizeof (libpart));
    libpart.index = element.object.libInd;
    GSErrCode err = NoError;
    #if defined(AC_27) || defined(AC_28)
    err = ACAPI_LibraryPart_Get (&libpart);
    #else
    err = ACAPI_LibPart_Get (&libpart);
    #endif
    if (err != NoError) {
        msg_rep ("ParamHelpers::GDLParamByDescription", "ACAPI_LibPart_Get", err, element.header.guid);
        return false;
    }
    double aParam = 0.0;
    double bParam = 0.0;
    Int32 addParNum = 0;
    API_AddParType** addPars = NULL;
    #if defined(AC_27) || defined(AC_28)
    err = ACAPI_LibraryPart_GetParams (libpart.index, &aParam, &bParam, &addParNum, &addPars);
    #else
    err = ACAPI_LibPart_GetParams (libpart.index, &aParam, &bParam, &addParNum, &addPars);
    #endif
    if (err != NoError) {
        ACAPI_DisposeAddParHdl (&addPars);
        msg_rep ("ParamHelpers::GDLParamByDescription", "ACAPI_LibPart_GetParams", err, element.header.guid);
        return false;
    }

    if (addPars == nullptr || *addPars == nullptr) {
        ACAPI_DisposeAddParHdl (&addPars);
        msg_rep ("FindGDLParametersByDescription", "ACAPI_LibPart_GetParams", err, element.header.guid);
        return false;
    }

    bool flagFind = false;
    Int32 nfind = params.GetSize ();
    // Ищем описание параметров
    for (Int32 i = 0; i < addParNum; ++i) {
        API_AddParType& actualParam = (*addPars)[i];
        GS::UniString desc_name = actualParam.uDescname;
        GS::UniString desc_rawname = "{@gdl:" + desc_name.ToLowerCase () + "}";
        if (params.ContainsKey (desc_rawname)) {

            // Получаем имя параметра
            GS::UniString name = actualParam.name;
            GS::UniString rawname = "{@gdl:" + name.ToLowerCase () + "}";

            // Если в словаре для чтения по имени параметра такого параметра нет - добавим
            if (!find_params.ContainsKey (rawname)) {
                ParamValue pvalue = params.Get (desc_rawname);
                pvalue.rawName = rawname;
                find_params.Add (rawname, pvalue);
            }

            // Описание на время сохраним в val.uniStringValue
            params.Get (desc_rawname).val.uniStringValue = params.Get (desc_rawname).name;

            // rawname с именем параметра для дальнейшего сопоставления
            params.Get (desc_rawname).name = rawname;
            flagFind = true;
            nfind--;
            if (nfind == 0) {
                ACAPI_DisposeAddParHdl (&addPars);
                return flagFind;
            }
        }
    }
    ACAPI_DisposeAddParHdl (&addPars);
    return flagFind;
}

// -----------------------------------------------------------------------------
// Поиск по имени GDL параметра
// -----------------------------------------------------------------------------
bool ParamHelpers::GDLParamByName (const API_Element & element, const API_Elem_Head & elem_head, ParamDictValue & params, GS::HashTable<GS::UniString, GS::Array<GS::UniString>>&paramnamearray)
{
    API_ElemTypeID	elemType;
    API_Guid		elemGuid;
    GetGDLParametersHead (element, elem_head, elemType, elemGuid);
    API_AddParType** addPars = NULL;
    GSErrCode err = GetGDLParameters (elemType, elemGuid, addPars);
    if (err != NoError) {
        msg_rep ("ParamHelpers::GDLParamByName", "GetGDLParameters", err, elemGuid);
        ACAPI_DisposeAddParHdl (&addPars);
        return false;
    }
    if (addPars == nullptr || *addPars == nullptr) {
        msg_rep ("ParamHelpers::GDLParamByName", "GetGDLParameters", err, elemGuid);
        ACAPI_DisposeAddParHdl (&addPars);
        return false;
    }
    bool flagFind = false;
    Int32	addParNum = BMGetHandleSize ((GSHandle) addPars) / sizeof (API_AddParType);
    Int32 nfind = params.GetSize ();
    for (Int32 i = 0; i < addParNum; ++i) {
        API_AddParType& actualParam = (*addPars)[i];
        GS::UniString name = actualParam.name;
        GS::UniString rawname = "{@gdl:" + name.ToLowerCase () + "}";
        if (params.ContainsKey (rawname)) {

            // Проверим - нет ли подходящих параметров-массивов?
            if (paramnamearray.ContainsKey (rawname)) {
                GS::Array<GS::UniString> paramarray = paramnamearray.Get (rawname);
                for (UInt32 j = 0; j < paramarray.GetSize (); ++j) {
                    nfind--;
                    ParamValue pvalue = params.Get (paramarray[j]);
                    FormatString fstring = pvalue.val.formatstring;
                    ParamHelpers::ConvertToParamValue (pvalue, actualParam);
                    if (pvalue.isValid) {
                        if (!fstring.isEmpty) pvalue.val.formatstring = fstring;
                        params.Set (pvalue.rawName, pvalue);
                        flagFind = true;
                    }
                }
                if (params.ContainsKey (rawname)) {
                    ParamValue pvalue = params.Get (rawname);
                    FormatString fstring = pvalue.val.formatstring;
                    ParamHelpers::ConvertToParamValue (pvalue, actualParam);
                    if (pvalue.isValid) {
                        if (!fstring.isEmpty) pvalue.val.formatstring = fstring;
                        params.Set (rawname, pvalue);
                        flagFind = true;
                    }
                    nfind--;
                } else {
                    #if defined(TESTING)
                    DBprnt ("ParamHelpers::GDLParamByName err", "params.ContainsKey(rawname) " + rawname);
                    #endif
                }
            } else {
                if (params.ContainsKey (rawname)) {
                    ParamValue pvalue = params.Get (rawname);
                    FormatString fstring = pvalue.val.formatstring;
                    ParamHelpers::ConvertToParamValue (pvalue, actualParam);
                    if (pvalue.isValid) {
                        if (!fstring.isEmpty) pvalue.val.formatstring = fstring;
                        params.Set (rawname, pvalue);
                        flagFind = true;
                    }
                    nfind--;
                } else {
                    #if defined(TESTING)
                    DBprnt ("ParamHelpers::GDLParamByName err", "params.ContainsKey(rawname) " + rawname);
                    #endif
                }
            }
            if (nfind == 0) {
                ACAPI_DisposeAddParHdl (&addPars);
                return flagFind;
            }
        }
    }
    ACAPI_DisposeAddParHdl (&addPars);
    return flagFind;
}

// -----------------------------------------------------------------------------
// Обработка свойств с формулами
// -----------------------------------------------------------------------------
bool ParamHelpers::ReadFormula (ParamDictValue & paramByType, ParamDictValue & params)
{
    #if defined(TESTING)
    DBprnt ("    ReadFormula");
    #endif
    bool flag_find = false;
    for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = paramByType.EnumeratePairs (); cIt != NULL; ++cIt) {
        #if defined(AC_28)
        ParamValue& param = cIt->value;
        GS::UniString key = cIt->key;
        #else
        ParamValue& param = *cIt->value;
        GS::UniString key = *cIt->key;
        #endif
        GS::UniString expression = param.val.uniStringValue;
        FormatString f = param.val.formatstring;
        if (!param.val.formatstring.isEmpty) expression = expression + '.' + param.val.formatstring.stringformat;
        if (ParamHelpers::ReplaceParamInExpression (params, expression)) {
            if (EvalExpression (expression)) {
                flag_find = true;
                ParamValue pvalue = {};
                ParamHelpers::ConvertStringToParamValue (pvalue, key, expression);
                pvalue.val.formatstring = f;
                paramByType.Get (key).val = pvalue.val;
                // Если выражение можно вычислить ещё раз - запишем в чиловое значение результат, текст трогать не будем
                GS::UniString expression_ = "";
                if (!param.val.formatstring.isEmpty) {
                    expression_ = str_formula_start + expression + str_formula_end + '.' + param.val.formatstring.stringformat;
                } else {
                    expression_ = str_formula_start + expression + str_formula_end;
                }
                if (EvalExpression (expression_)) {
                    ParamHelpers::ConvertStringToParamValue (pvalue, key, expression_);
                    pvalue.val.formatstring = f;
                    paramByType.Get (key).val = pvalue.val;
                    paramByType.Get (key).val.uniStringValue = expression;
                }
                paramByType.Get (key).isValid = true;
            }
        }
    }
    return flag_find;
}

bool ParamHelpers::ReadListData (const API_Elem_Head & elem_head, ParamDictValue & params)
{
    GSErrCode err = NoError;
    Int32 nComp = 0;
    GS::Array<ParamValueComposite> composites = {};
    #if defined(AC_22) || defined(AC_23) || defined(AC_24)
    API_ComponentRefType** compRefs;
    err = ACAPI_Element_GetComponents (&elem_head, &compRefs, &nComp);
    #else
    API_Obsolete_ComponentRefType** compRefs;
    err = ACAPI_Element_GetComponents_Obsolete (&elem_head, &compRefs, &nComp);
    #endif
    if (err != NoError) {
        msg_rep ("ReadListData", "ACAPI_Element_GetComponents_Obsolete", err, elem_head.guid);
        return false;
    }
    GS::UniString out = "";
    for (Int32 i = 0; i < nComp; i++) {
        if ((*compRefs)[i].status == APIDBRef_Deleted) continue;
        API_ListData listdata = {};
        BNZeroMemory (&listdata, sizeof (API_ListData));
        #if defined(AC_22) || defined(AC_23) || defined(AC_24)
        listdata.header.typeID = API_ComponentID;
        #else
        listdata.header.typeID = API_Obsolete_ComponentID;
        #endif
        listdata.header.index = (*compRefs)[i].index;
        listdata.header.setIndex = (*compRefs)[i].setIndex;
        switch ((*compRefs)[i].status) {
            case APIDBRef_Normal:
                #if defined(AC_28) || defined(AC_27)
                err = ACAPI_OldListing_Get (&listdata);
                #else
                err = ACAPI_ListData_Get (&listdata);
                #endif
                break;
            case APIDBRef_Local:
                #if defined(AC_28) || defined(AC_27)
                err = ACAPI_OldListing_GetLocal ((*compRefs)[i].libIndex, &elem_head, &listdata);
                #else
                err = ACAPI_ListData_GetLocal ((*compRefs)[i].libIndex, &elem_head, &listdata);
                #endif
                break;
            default:
                continue;
        }
        if (err != NoError) {
            msg_rep ("ReadListData", "ACAPI_ListData_Get", err, elem_head.guid);
            continue;
        }
        ParamValueComposite p = {};
        char tname[API_DBNameLen];
        CHTruncate (listdata.component.name, tname, API_DBNameLen);
        p.val = GS::UniString (tname);
        p.qty = listdata.component.quantity;
        char tunitcode[API_DBCodeLen];
        CHTruncate (listdata.component.unitcode, tunitcode, API_DBCodeLen);
        p.unit = GS::UniString (tunitcode);
        out += p.val + GS::UniString::Printf (" %.3f ", p.qty) + p.unit + ";                             ";
        composites.Push (p);
    }
    BMKillHandle ((GSHandle*) &compRefs);
    for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = params.EnumeratePairs (); cIt != NULL; ++cIt) {
        #if defined(AC_28)
        ParamValue& param = cIt->value;
        #else
        ParamValue& param = *cIt->value;
        #endif
        if (!param.fromListData) continue;
        param.composite = composites;
        param.val.uniStringValue = out;
        param.isValid = true;
        param.type = API_PropertyStringValueType;
        param.val.type = API_PropertyStringValueType;
    }
    return !composites.IsEmpty ();
}

// -----------------------------------------------------------------------------
// Получение информации о элементе
// -----------------------------------------------------------------------------
void ParamHelpers::ReadQuantities (const API_Guid & elemGuid, ParamDictValue & params, ParamDictValue & propertyParams, GS::HashTable<API_AttributeIndex, bool>&existsmaterial, ParamDictValue & paramlayers)
{
    #if defined(TESTING)
    DBprnt ("        Quantities");
    #endif
    API_ElementQuantity quantity = {};
    API_QuantityPar paramq = {}; BNZeroMemory (&paramq, sizeof (API_QuantityPar));
    paramq.minOpeningSize = EPS;
    GSErrCode err = NoError;
    GS::Array <API_CompositeQuantity> composites = {};
    GS::Array <API_ElemPartQuantity> elemPartQuantities = {};
    GS::Array <API_ElemPartCompositeQuantity> elemPartComposites = {};
    API_QuantitiesMask mask;
    GS::HashTable<API_AttributeIndex, ParamValueComposite> composites_quantity = {};
    ACAPI_ELEMENT_QUANTITY_MASK_CLEAR (mask);
    ACAPI_ELEMENT_COMPOSITES_QUANTITY_MASK_SETFULL (mask);
    GS::Array<API_Quantities> quantities = {}; quantities.Push (API_Quantities ());
    quantities[0].elements = &quantity;
    quantities[0].composites = &composites;
    quantities[0].elemPartQuantities = &elemPartQuantities;
    quantities[0].elemPartComposites = &elemPartComposites;
    GS::Array<API_Guid> elemGuids = {}; elemGuids.Push (elemGuid);
    err = ACAPI_Element_GetMoreQuantities (&elemGuids, &paramq, &quantities, &mask);
    if (err != NoError) {
        msg_rep ("ReadQuantities", "ACAPI_Element_GetMoreQuantities", err, elemGuid);
        return;
    }
    ParamDictValue paramsAdd = {};
    GS::UniString rawname_th = "@property:buildingmaterialproperties/some_stuff_th";
    GS::UniString rawname_unit = "@property:buildingmaterialproperties/some_stuff_units";
    GS::UniString rawname_kzap = "@property:buildingmaterialproperties/some_stuff_kzap";
    GS::UniString units = ""; double kzap = 1;
    ParamHelpers::AddValueToParamDictValue (params, rawname_th); rawname_th = "{" + rawname_th;
    ParamHelpers::AddValueToParamDictValue (params, rawname_unit); rawname_unit = "{" + rawname_unit;
    ParamHelpers::AddValueToParamDictValue (params, rawname_kzap); rawname_kzap = "{" + rawname_kzap;
    bool flag_find = false;
    for (UInt32 i = 0; i < composites.GetSize (); i++) {
        API_AttributeIndex constrinx = composites[i].buildMatIndices;
        double volume = composites[i].volumes;
        if (composites_quantity.ContainsKey (constrinx)) {
            ParamValueComposite& p = composites_quantity.Get (constrinx);
            p.volume += volume;
        } else {
            if (!existsmaterial.ContainsKey (constrinx)) {
                if (ParamHelpers::GetAttributeValues (constrinx, params, paramsAdd)) {
                    existsmaterial.Add (constrinx, true);
                }
            }
            double th = 0; units = "";
            GS::UniString attribsuffix = CharENTER + GS::UniString::Printf ("%d", constrinx) + "}";
            if (params.ContainsKey (rawname_unit + attribsuffix)) {
                if (params.Get (rawname_unit + attribsuffix).isValid) {
                    units = params.Get (rawname_unit + attribsuffix).val.uniStringValue;
                } else {
                    units = "";
                }
            }
            kzap = 1;
            if (params.ContainsKey (rawname_kzap + attribsuffix)) {
                if (params.Get (rawname_kzap + attribsuffix).isValid) {
                    kzap = params.Get (rawname_kzap + attribsuffix).val.doubleValue;
                    if (is_equal (kzap, 0) || kzap < 0) kzap = 1;
                } else {
                    kzap = 1;
                }
            }
            if (params.ContainsKey (rawname_th + attribsuffix)) {
                if (params.Get (rawname_th + attribsuffix).isValid) {
                    th = params.Get (rawname_th + attribsuffix).val.doubleValue;
                } else {
                    th = 0;
                }
            }
            ParamValueComposite p = {};
            p.inx = constrinx;
            p.volume = volume;
            p.fillThick = th;
            p.num = i;
            p.unit = units;
            p.kzap = kzap;
            composites_quantity.Add (p.inx, p);
        }
        flag_find = true;
    }
    GS::HashTable<API_AttributeIndex, ParamValueComposite> composites_quantity_param = {};
    for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = paramlayers.EnumeratePairs (); cIt != NULL; ++cIt) {
        #if defined(AC_28)
        ParamValue& param = cIt->value;
        #else
        ParamValue& param = *cIt->value;
        #endif
        if (param.composite_pen < 0) {
            for (auto& p : param.composite) {
                if (composites_quantity_param.ContainsKey (p.inx)) {
                    ParamValueComposite& pc = composites_quantity_param.Get (p.inx);
                    pc.area_fill += p.area_fill;
                    pc.fillThick += p.fillThick;
                    if (composites_quantity.ContainsKey (p.inx)) pc.volume += composites_quantity.Get (p.inx).volume;
                } else {
                    composites_quantity_param.Add (p.inx, p);
                }
            }
            break;
        }
    }
    for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = paramlayers.EnumeratePairs (); cIt != NULL; ++cIt) {
        #if defined(AC_28)
        ParamValue& param = cIt->value;
        #else
        ParamValue& param = *cIt->value;
        #endif
        for (auto& p : param.composite) {
            if (!composites_quantity.ContainsKey (p.inx))continue;
            if (!composites_quantity_param.ContainsKey (p.inx))continue;
            ParamValueComposite& qty_param = composites_quantity_param.Get (p.inx);
            double volume_total = composites_quantity.Get (p.inx).volume;
            double area_fill_total = qty_param.area_fill;
            double fillThick_total = qty_param.fillThick;
            // Определяем толщину
            double fillThick = composites_quantity.Get (p.inx).fillThick;
            if (is_equal (fillThick, 0)) {
                fillThick = p.fillThick;
            } else {

                if (!is_equal (fillThick, p.fillThick)) {
                    GS::UniString msg = GS::UniString::Printf ("%f", fillThick);
                    msg += " <-> ";
                    msg += GS::UniString::Printf ("%f", p.fillThick);
                    msg += GS::UniString::Printf (" attrib inx: %d", p.inx);
                    msg_rep ("Warning : Layer thickness", "Different thickness in property and model : " + msg, APIERR_GENERAL, elemGuid);
                }
            }
            double proc = 0;
            // Определяем долю площади проекции для текущего слоя
            if (is_equal (p.fillThick, fillThick_total) && is_equal (proc, 0) && !is_equal (p.fillThick, 0)) {
                proc = 1; // Если площадь слоя совпадает с суммарной площадью материала
            }
            if (is_equal (area_fill_total, p.area_fill) && is_equal (proc, 0) && !is_equal (p.area_fill, 0)) {
                proc = 1; // Если площадь слоя совпадает с суммарной площадью материала
            }
            // Определяем долю площади проекции для текущего слоя
            if (!is_equal (area_fill_total, 0) && is_equal (proc, 0)) {
                proc = p.area_fill / area_fill_total;
            }
            // Если не удалось - возьмём долю по общей толщине
            if (!is_equal (fillThick_total, 0) && is_equal (proc, 0)) {
                proc = p.fillThick / fillThick_total;
            }
            p.unit = composites_quantity.Get (p.inx).unit;
            p.volume = volume_total * proc;
            if (!is_equal (fillThick, 0)) p.area = p.volume / fillThick;
            if (!is_equal (p.area_fill, 0)) p.length = p.volume / p.area_fill;
            ParamHelpers::SetUnitsAndQty2ParamValueComposite (p);
        }
    }
    return;
}

// -----------------------------------------------------------------------------
// Получение информации о элементе
// -----------------------------------------------------------------------------
bool ParamHelpers::ReadElementValues (const API_Element & element, ParamDictValue & params)
{
    #if defined(TESTING)
    DBprnt ("    ReadElement");
    #endif
    API_ElemTypeID eltype = GetElemTypeID (element);
    GS::UniString rawname = "";
    bool flag_find = false;
    rawname = "{@element:material overridden}";
    if (params.ContainsKey (rawname)) {
        ParamValue& param = params.Get (rawname);
        switch (eltype) {
            case API_WindowID:
                ParamHelpers::ConvertBoolToParamValue (param, param.name, !element.window.openingBase.useObjMaterials);
                break;
            case API_DoorID:
                ParamHelpers::ConvertBoolToParamValue (param, param.name, !element.door.openingBase.useObjMaterials);
                break;
            case API_ObjectID:
                ParamHelpers::ConvertBoolToParamValue (param, param.name, !element.object.useObjMaterials);
            case API_LampID:
                ParamHelpers::ConvertBoolToParamValue (param, param.name, !element.lamp.useObjMaterials);
                break;
            default:
                param.isValid = false;
                break;
        }
        if (param.isValid) flag_find = true;
    }
    return flag_find;
}

GS::UniString ParamHelpers::GetUnitsPrefix (GS::UniString & unit)
{
    if (unit.IsEmpty ()) {
        return "";
    }
    const Int32 iseng = ID_ADDON_STRINGS + isEng ();
    GS::UniString nameunits = RSGetIndString (iseng, 58, ACAPI_GetOwnResModule ());
    GS::UniString units = unit.ToLowerCase ();
    if (units.Contains (nameunits)) {
        return "";
    }
    nameunits = RSGetIndString (iseng, 53, ACAPI_GetOwnResModule ());
    if (units.Contains (nameunits)) {
        return "S";
    }
    nameunits = RSGetIndString (iseng, 54, ACAPI_GetOwnResModule ());
    if (units.Contains (nameunits)) {
        return "S";
    }
    nameunits = RSGetIndString (iseng, 55, ACAPI_GetOwnResModule ());
    if (units.Contains (nameunits)) {
        return "V";
    }
    nameunits = RSGetIndString (iseng, 56, ACAPI_GetOwnResModule ());
    if (units.Contains (nameunits)) {
        return "V";
    }
    nameunits = RSGetIndString (iseng, 57, ACAPI_GetOwnResModule ());
    if (units.Contains (nameunits)) {
        return "L";
    }
    return "";
}

void ParamHelpers::SetUnitsAndQty2ParamValueComposite (ParamValueComposite & comp)
{
    if (comp.unit.IsEmpty ()) {
        comp.qty = 0;
        return;
    }
    const Int32 iseng = ID_ADDON_STRINGS + isEng ();
    GS::UniString nameunits = RSGetIndString (iseng, 58, ACAPI_GetOwnResModule ());
    GS::UniString units = comp.unit.ToLowerCase ();
    if (units.Contains (nameunits)) {
        comp.qty = 0;
        return;
    }
    nameunits = RSGetIndString (iseng, 64, ACAPI_GetOwnResModule ());
    if (units.Contains (nameunits)) {
        comp.qty = comp.area * comp.kzap;
        return;
    }
    nameunits = RSGetIndString (iseng, 53, ACAPI_GetOwnResModule ());
    if (units.Contains (nameunits)) {
        comp.qty = comp.area * comp.kzap;
        return;
    }
    nameunits = RSGetIndString (iseng, 54, ACAPI_GetOwnResModule ());
    if (units.Contains (nameunits)) {
        comp.qty = comp.area * comp.kzap;
        return;
    }
    nameunits = RSGetIndString (iseng, 55, ACAPI_GetOwnResModule ());
    if (units.Contains (nameunits)) {
        comp.qty = comp.volume * comp.kzap;
        return;
    }
    nameunits = RSGetIndString (iseng, 56, ACAPI_GetOwnResModule ());
    if (units.Contains (nameunits)) {
        comp.qty = comp.volume * comp.kzap;
        return;
    }
    nameunits = RSGetIndString (iseng, 57, ACAPI_GetOwnResModule ());
    if (units.Contains (nameunits)) {
        comp.qty = comp.length * comp.kzap;
        return;
    }
    nameunits = RSGetIndString (iseng, 59, ACAPI_GetOwnResModule ());
    if (units.Contains (nameunits)) {
        comp.qty = comp.area * comp.kzap;
        return;
    }
    nameunits = RSGetIndString (iseng, 60, ACAPI_GetOwnResModule ());
    if (units.Contains (nameunits)) {
        comp.qty = comp.area * comp.kzap;
        return;
    }
    nameunits = RSGetIndString (iseng, 61, ACAPI_GetOwnResModule ());
    if (units.Contains (nameunits)) {
        comp.qty = comp.volume * comp.kzap;
        return;
    }
    nameunits = RSGetIndString (iseng, 62, ACAPI_GetOwnResModule ());
    if (units.Contains (nameunits)) {
        comp.qty = comp.volume * comp.kzap;
        return;
    }
    nameunits = RSGetIndString (iseng, 63, ACAPI_GetOwnResModule ());
    if (units.Contains (nameunits)) {
        comp.qty = comp.length * comp.kzap;
        return;
    }
}


// -----------------------------------------------------------------------------
// Получение информации о материалах и составе конструкции
// -----------------------------------------------------------------------------
bool ParamHelpers::ReadMaterial (const API_Element & element, ParamDictValue & params, ParamDictValue & propertyParams)
{
    #if defined(TESTING)
    DBprnt ("    ReadMaterial");
    #endif
    // Получим состав элемента, добавив в словарь требуемые параметры
    ParamDictValue paramsAdd = {};
    GS::HashTable<API_AttributeIndex, bool> existsmaterial; // Словарь с уже прочитанными материалами
    if (!ParamHelpers::Components (element, params, paramsAdd, existsmaterial)) return false;

    bool needReadQuantities = false;
    ParamDictValue paramlayers = {};
    for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = params.EnumeratePairs (); cIt != NULL; ++cIt) {
        #if defined(AC_28)
        ParamValue& param = cIt->value;
        #else
        ParamValue& param = *cIt->value;
        #endif
        if (param.fromMaterial) {
            if (param.rawName.Contains ("{@material:layers")) {
                paramlayers.Add (param.rawName, param);
            }
        }
        if (param.fromQuantity) needReadQuantities = true;
    }
    if (paramlayers.IsEmpty ()) return true;
    // В свойствах могли быть ссылки на другие свойста. Проверим, распарсим
    if (!paramsAdd.IsEmpty ()) {
        if (needReadQuantities) {
            ParamHelpers::AddValueToParamDictValue (paramsAdd, "@property:buildingmaterialproperties/some_stuff_th");
            ParamHelpers::AddValueToParamDictValue (paramsAdd, "@property:buildingmaterialproperties/some_stuff_units");
            ParamHelpers::AddValueToParamDictValue (paramsAdd, "@property:buildingmaterialproperties/some_stuff_kzap");
        }
        ParamHelpers::CompareParamDictValue (propertyParams, paramsAdd);
        for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = paramlayers.EnumeratePairs (); cIt != NULL; ++cIt) {
            #if defined(AC_28)
            ParamValue& param_composite = cIt->value;
            #else
            ParamValue& param_composite = *cIt->value;
            #endif
            ParamDictValue paramsAdd_1 = {};
            if (param_composite.val.uniStringValue.Contains ("{")) {
                Int32 nlayers = param_composite.composite.GetSize ();
                bool flag = false;
                for (Int32 i = 0; i < nlayers; ++i) {
                    API_AttributeIndex constrinx = param_composite.composite[i].inx;
                    if (ParamHelpers::GetAttributeValues (constrinx, paramsAdd, paramsAdd_1)) {
                        if (!paramsAdd_1.IsEmpty ()) {
                            ParamHelpers::GetAttributeValues (constrinx, paramsAdd_1, paramsAdd_1);
                        }
                        if (!existsmaterial.ContainsKey (constrinx)) existsmaterial.Add (constrinx, true);
                        flag = true;
                    }
                }
                if (flag) {
                    for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = paramsAdd.EnumeratePairs (); cIt != NULL; ++cIt) {
                        #if defined(AC_28)
                        if (!params.ContainsKey (cIt->key)) {
                            params.Add (cIt->key, cIt->value);
                            #else
                        if (!params.ContainsKey (*cIt->key)) {
                            params.Add (*cIt->key, *cIt->value);
                            #endif
                        }
                    }
                }
                if (!paramsAdd_1.IsEmpty ()) {
                    for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = paramsAdd_1.EnumeratePairs (); cIt != NULL; ++cIt) {
                        #if defined(AC_28)
                        if (!params.ContainsKey (cIt->key)) {
                            params.Add (cIt->key, cIt->value);
                            #else
                        if (!params.ContainsKey (*cIt->key)) {
                            params.Add (*cIt->key, *cIt->value);
                            #endif
                        }
                    }
                }
            }
        }
    }
    bool flag_add = false;
    if (needReadQuantities) ParamHelpers::ReadQuantities (element.header.guid, params, propertyParams, existsmaterial, paramlayers);
    // Если есть строка-шаблон - заполним её
    for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = paramlayers.EnumeratePairs (); cIt != NULL; ++cIt) {
        bool flag = false;
        #if defined(AC_28)
        ParamValue& param_composite = cIt->value;
        GS::UniString rawName = cIt->key;
        #else
        ParamValue& param_composite = *cIt->value;
        GS::UniString rawName = *cIt->key;
        #endif
        GS::UniString outstring = "";
        GS::UniString stringformat = "";
        if (param_composite.val.uniStringValue.Contains ("{")) {
            bool inverse = rawName.Contains ("{@material:layers_inv");
            if (rawName.Contains ("{@material:layers_auto")) {
                if (param_composite.composite_type != API_ProfileStructure) {
                    if (param_composite.eltype == API_WallID) inverse = true;
                }
            }
            bool ignore_sync = false;
            if (param_composite.val.uniStringValue.Contains ("{@property:nosyncname}")) {
                param_composite.val.uniStringValue.ReplaceAll ("{@property:nosyncname}", "");
                ignore_sync = true;
            }
            if (param_composite.composite_pen == -2) ParamHelpers::ComponentsGetUnic (param_composite.composite);
            Int32 nlayers = param_composite.composite.GetSize ();
            if (param_composite.val.hasFormula) {
                //Если есть формула - заменим повторим все участки, заключенные в <> по количеству слоёв
                // Например, 1+<толщина> -> 1+<&2><&1><&0>
                outstring = param_composite.val.uniStringValue;
                GS::UniString part = outstring.GetSubstring (char_formula_start, char_formula_end, 0);
                stringformat = "";
                FormatStringFunc::GetFormatStringFromFormula (outstring, part, stringformat);
                for (Int32 i = 0; i < nlayers; ++i) {
                    if (i == nlayers - 1) {
                        outstring.ReplaceAll (part, GS::UniString::Printf ("&%d&", i));
                    } else {
                        outstring.ReplaceAll (part, part + GS::UniString::Printf ("&%d&", i));
                    }
                }
            }
            Int32 ns = 0;
            for (Int32 i = 0; i < nlayers; ++i) {
                GS::UniString templatestring = param_composite.val.uniStringValue;
                // Для формул возьмём только часть в <>, только она повторяется для каждого слоя
                if (param_composite.val.hasFormula) {
                    if (!stringformat.IsEmpty ()) templatestring.ReplaceAll (str_formula_end + stringformat, str_formula_end);
                    templatestring = param_composite.val.uniStringValue.GetSubstring (char_formula_start, char_formula_end, 0);
                }
                Int32 indx = i;
                if (inverse) indx = nlayers - i - 1;
                API_AttributeIndex constrinx = param_composite.composite[indx].inx;
                // Если для материала было указано уникальное наименование - заменим его
                GS::UniString attribsuffix = CharENTER + GS::UniString::Printf ("%d", constrinx) + "}";

                if (!ignore_sync) {
                    for (UInt32 inx = 0; inx < 20; inx++) {
                        GS::UniString syncname = "{@property:sync_name" + GS::UniString::Printf ("%d", inx) + attribsuffix;
                        if (params.ContainsKey (syncname)) {
                            if (params.Get (syncname).isValid && !params.Get (syncname).property.isDefault) {
                                templatestring = params.Get (syncname).val.uniStringValue;
                                break;
                            }
                        }
                    }
                }
                // Если нужно заполнить толщину
                GS::UniString layer_thickness = "{@material:layer thickness}";
                GS::UniString defult_formatstring = "1mm";
                if (params.ContainsKey (layer_thickness)) {
                    double fillThick = param_composite.composite[indx].fillThick;
                    FormatString formatsting = params.Get (layer_thickness).val.formatstring;
                    if (formatsting.isEmpty) {
                        formatsting = FormatStringFunc::ParseFormatString (defult_formatstring);
                        params.Get (layer_thickness).val.formatstring = formatsting;
                    }
                    params.Get (layer_thickness).val.doubleValue = fillThick;
                    params.Get (layer_thickness).val.rawDoubleValue = fillThick;
                    params.Get (layer_thickness).val.hasrawDouble = true;
                    params.Get (layer_thickness).val.type = API_PropertyRealValueType;
                    params.Get (layer_thickness).isValid = true;
                }
                if (param_composite.composite_pen < 0) {
                    layer_thickness = "{@material:area}"; defult_formatstring = "2m";
                    if (params.ContainsKey (layer_thickness)) {
                        double val = param_composite.composite[indx].area;
                        FormatString formatsting = params.Get (layer_thickness).val.formatstring;
                        if (formatsting.isEmpty) {
                            formatsting = FormatStringFunc::ParseFormatString (defult_formatstring);
                            params.Get (layer_thickness).val.formatstring = formatsting;
                        }
                        params.Get (layer_thickness).val.doubleValue = val;
                        params.Get (layer_thickness).val.rawDoubleValue = val;
                        params.Get (layer_thickness).val.hasrawDouble = true;
                        params.Get (layer_thickness).val.type = API_PropertyRealValueType;
                        params.Get (layer_thickness).isValid = true;
                    }
                    layer_thickness = "{@material:volume}"; defult_formatstring = "2m";
                    if (params.ContainsKey (layer_thickness)) {
                        double val = param_composite.composite[indx].volume;
                        FormatString formatsting = params.Get (layer_thickness).val.formatstring;
                        if (formatsting.isEmpty) {
                            formatsting = FormatStringFunc::ParseFormatString (defult_formatstring);
                            params.Get (layer_thickness).val.formatstring = formatsting;
                        }
                        params.Get (layer_thickness).val.doubleValue = val;
                        params.Get (layer_thickness).val.rawDoubleValue = val;
                        params.Get (layer_thickness).val.hasrawDouble = true;
                        params.Get (layer_thickness).val.type = API_PropertyRealValueType;
                        params.Get (layer_thickness).isValid = true;
                    }
                    layer_thickness = "{@material:qty}"; defult_formatstring = "2m";
                    if (params.ContainsKey (layer_thickness)) {
                        double val = param_composite.composite[indx].qty;
                        FormatString formatsting = params.Get (layer_thickness).val.formatstring;
                        if (formatsting.isEmpty) {
                            formatsting = FormatStringFunc::ParseFormatString (defult_formatstring);
                            params.Get (layer_thickness).val.formatstring = formatsting;
                        }
                        params.Get (layer_thickness).val.doubleValue = val;
                        params.Get (layer_thickness).val.rawDoubleValue = val;
                        params.Get (layer_thickness).val.hasrawDouble = true;
                        params.Get (layer_thickness).val.type = API_PropertyRealValueType;
                        params.Get (layer_thickness).isValid = true;
                    }
                    layer_thickness = "{@material:unit_prefix}"; defult_formatstring = "2m";
                    if (params.ContainsKey (layer_thickness)) {
                        GS::UniString val = ParamHelpers::GetUnitsPrefix (param_composite.composite[indx].unit);
                        params.Get (layer_thickness).val.uniStringValue = val;
                        params.Get (layer_thickness).val.type = API_PropertyStringValueType;
                        params.Get (layer_thickness).isValid = true;
                    }
                }
                GS::UniString n_txt = GS::UniString::Printf ("%d", i + 1);
                templatestring.ReplaceAll ("{@material:n}", n_txt);

                layer_thickness = "{@material:n}"; defult_formatstring = "0m";
                if (params.ContainsKey (layer_thickness)) {
                    params.Get (layer_thickness).val.uniStringValue = n_txt;
                    params.Get (layer_thickness).val.intValue = i + 1;
                    params.Get (layer_thickness).val.doubleValue = i + 1;
                    params.Get (layer_thickness).val.rawDoubleValue = i + 1;
                    params.Get (layer_thickness).val.type = API_PropertyStringValueType;
                    params.Get (layer_thickness).isValid = true;
                } else {
                    ParamHelpers::AddStringValueToParamDictValue (params, element.header.guid, "material:", "n", n_txt);
                }

                n_txt = GS::UniString::Printf ("%d", ns + 1);
                templatestring.ReplaceAll ("{@material:ns}", n_txt);
                layer_thickness = "{@material:ns}"; defult_formatstring = "0m";
                if (params.ContainsKey (layer_thickness)) {
                    params.Get (layer_thickness).val.uniStringValue = n_txt;
                    params.Get (layer_thickness).val.intValue = i + 1;
                    params.Get (layer_thickness).val.doubleValue = i + 1;
                    params.Get (layer_thickness).val.rawDoubleValue = i + 1;
                    params.Get (layer_thickness).val.type = API_PropertyStringValueType;
                    params.Get (layer_thickness).isValid = true;
                } else {
                    ParamHelpers::AddStringValueToParamDictValue (params, element.header.guid, "material:", "ns", n_txt);
                }
                templatestring.ReplaceAll ("}", attribsuffix);
                if (ParamHelpers::ReplaceParamInExpression (params, templatestring)) {
                    flag = true;
                    flag_add = true;
                    ReplaceSymbSpase (templatestring);
                    param_composite.composite[indx].val = templatestring;
                    if (param_composite.val.hasFormula) {
                        outstring.ReplaceAll (GS::UniString::Printf ("&%d&", i), templatestring);
                    } else {
                        outstring.Append (templatestring);
                    }
                    templatestring.Trim ();
                    if (!templatestring.IsEmpty ()) ns += 1;
                }
            }
        }
        if (flag) {
            if (params.Get (rawName).val.hasFormula) {
                if (outstring.Contains ("{")) ParamHelpers::ReplaceParamInExpression (params, outstring);
                if (!stringformat.IsEmpty ()) {
                    GS::UniString expression = outstring.GetSubstring (char_formula_start, char_formula_end, 0);
                    GS::UniString first_char = expression.GetSubstring (0, 1);
                    expression = str_formula_start + expression + str_formula_end + stringformat;
                    GS::UniString expression_clean = expression;
                    EvalExpression (expression);
                    expression = first_char + expression;
                    outstring.ReplaceAll (expression_clean, expression);
                }
                if (outstring.Contains (char_formula_start) && outstring.Contains (char_formula_end)) {
                    outstring.ReplaceAll (str_formula_start, "");
                    outstring.ReplaceAll (str_formula_end, "");
                }
                outstring = str_formula_start + outstring + str_formula_end;
            }
            params.Get (rawName).val.uniStringValue = outstring;
            params.Get (rawName).isValid = true;
            params.Get (rawName).val.type = API_PropertyStringValueType;
            params.Get (rawName).composite = param_composite.composite;
        }
    }
    return flag_add;
}


void ParamHelpers::Array2ParamValue (GS::Array<ParamValueData>&pvalue, ParamValueData & pvalrezult)
{
    if (pvalue.IsEmpty ()) return;
    GS::UniString delim = ";";
    int array_format_out = pvalrezult.array_format_out;
    GS::UniString param_string = "";
    double param_real = 0;
    bool param_bool = false;
    if (array_format_out == ARRAY_MIN) param_bool = true;
    GS::Int32 param_int = 0;
    bool canCalculate = false;
    int array_column_end = pvalrezult.array_column_end;
    int array_column_start = pvalrezult.array_column_start;
    int array_row_end = pvalrezult.array_row_end;
    int array_row_start = pvalrezult.array_row_start;
    if (array_format_out == ARRAY_MAX || array_format_out == ARRAY_MIN) {
        param_real = pvalue.Get (0).doubleValue;
        param_int = pvalue.Get (0).intValue;
        param_bool = pvalue.Get (0).boolValue;
        param_string = pvalue.Get (0).uniStringValue;
    }

    for (UInt32 i = 0; i < pvalue.GetSize (); i++) {
        ParamValueData pval = pvalue.Get (i);
        if (pval.canCalculate) {
            canCalculate = true;
            if (array_format_out == ARRAY_SUM || array_format_out == ARRAY_UNIC) {
                param_real = param_real + pval.doubleValue;
                param_int = param_int + pval.intValue;
                if (pval.boolValue) param_bool = true;
            }
            if (array_format_out == ARRAY_MAX) {
                param_real = fmax (param_real, pval.doubleValue);
                if (pval.intValue > param_int) param_int = pval.intValue;
                if (pval.boolValue) param_bool = true;
            }
            if (array_format_out == ARRAY_MIN) {
                param_real = fmin (param_real, pval.doubleValue);
                if (pval.intValue < param_int) param_int = pval.intValue;
                if (!pval.boolValue) param_bool = false;
            }
        }
        if (array_format_out == ARRAY_SUM || array_format_out == ARRAY_UNIC) {
            if (param_string.IsEmpty ()) {
                param_string = pval.uniStringValue;
            } else {
                param_string.Append (delim);
                param_string.Append (pval.uniStringValue);
            }
        }
        if (array_format_out == ARRAY_MAX) {
            GSCharCode chcode = GetCharCode (pval.uniStringValue);
            std::string s = pval.uniStringValue.ToCStr (0, MaxUSize, chcode).Get ();
            std::string p = param_string.ToCStr (0, MaxUSize, chcode).Get ();
            if (doj::alphanum_comp (s, p) > 0) param_string = GS::UniString (s.c_str (), chcode);
        }
        if (array_format_out == ARRAY_MIN) {
            GSCharCode chcode = GetCharCode (pval.uniStringValue);
            std::string s = pval.uniStringValue.ToCStr (0, MaxUSize, chcode).Get ();
            std::string p = param_string.ToCStr (0, MaxUSize, chcode).Get ();
            if (doj::alphanum_comp (s, p) < 0) param_string = GS::UniString (s.c_str (), chcode);
        }
    }
    pvalrezult = pvalue.Get (0);
    if (array_format_out == ARRAY_UNIC) {
        pvalrezult.uniStringValue = StringUnic (param_string, delim);
    } else {
        pvalrezult.uniStringValue = param_string;
    }
    pvalrezult.array_column_end = array_column_end;
    pvalrezult.array_column_start = array_column_start;
    pvalrezult.array_row_end = array_row_end;
    pvalrezult.array_row_start = array_row_start;
    pvalrezult.array_format_out = array_format_out;
    pvalrezult.boolValue = param_bool;
    pvalrezult.doubleValue = param_real;
    pvalrezult.rawDoubleValue = param_real;
    pvalrezult.hasrawDouble = true;
    pvalrezult.intValue = param_int;
    pvalrezult.canCalculate = canCalculate;
}

// -----------------------------------------------------------------------------
// Конвертация одиночного параметра библиотечного элемента (тип API_ParSimple) в ParamValue
// -----------------------------------------------------------------------------
bool ParamHelpers::ConvertToParamValue (ParamValueData & pvalue, const API_AddParID & typeIDr, const GS::UniString & pstring, const double& preal)
{

    // TODO добавить округления на основе настроек проекта
    GS::UniString param_string = pstring;
    double param_real = preal;
    bool param_bool = false;
    GS::Int32 param_int = 0;
    pvalue.canCalculate = false;
    if (typeIDr == APIParT_CString) {
        pvalue.type = API_PropertyStringValueType;
        param_bool = (!param_string.IsEmpty ());

        if (UniStringToDouble (param_string, param_real)) {
            param_real = round (param_real * 100000.0) / 100000.0;
            param_int = (GS::Int32) param_real;
            if (param_int / 1 < param_real) param_int += 1;
            pvalue.canCalculate = true;
        } else {
            if (param_bool) {
                param_int = 1;
                param_real = 1.0;
            }
        }
    } else {
        param_real = round (param_real * 100000) / 100000;
        if (preal - param_real > 0.00001) param_real += 0.00001;
        param_int = (GS::Int32) param_real;
        if (param_int / 1 < param_real) param_int += 1;
    }
    if (fabs (param_real) > std::numeric_limits<double>::epsilon ()) param_bool = true;

    // Если параметр не строковое - определяем текстовое значение конвертацией
    if (typeIDr != APIParT_CString) {
        API_AttrTypeID attrType = API_ZombieAttrID;
        #if defined(AC_27) || defined(AC_28)
        API_AttributeIndex attrInx = ACAPI_CreateAttributeIndex (param_int);
        #else
        short attrInx = (short) param_int;
        #endif
        switch (typeIDr) {
            case APIParT_PenCol:
            case APIParT_Integer:
                param_string = GS::UniString::Printf ("%d", param_int);
                pvalue.type = API_PropertyIntegerValueType;
                pvalue.canCalculate = true;
                pvalue.formatstring = FormatStringFunc::ParseFormatString ("0m");
                break;
            case APIParT_Boolean:
                if (param_bool) {
                    param_string = RSGetIndString (ID_ADDON_STRINGS + isEng (), TrueId, ACAPI_GetOwnResModule ());
                    param_int = 1;
                    param_real = 1.0;
                } else {
                    param_string = RSGetIndString (ID_ADDON_STRINGS + isEng (), FalseId, ACAPI_GetOwnResModule ());
                    param_int = 0;
                    param_real = 0.0;
                }
                pvalue.formatstring = FormatStringFunc::ParseFormatString ("0m");
                pvalue.canCalculate = true;
                pvalue.type = API_PropertyBooleanValueType;
                break;
            case APIParT_Length:
                param_string = GS::UniString::Printf ("%.0f", param_real * 1000);
                pvalue.canCalculate = true;
                pvalue.type = API_PropertyRealValueType;
                pvalue.formatstring = FormatStringFunc::ParseFormatString ("1mm");
                break;
            case APIParT_Angle:
                param_real = round ((preal * 180.0 / PI) * 100000.0) / 100000.0;
                if (preal - param_real > 0.00001) param_real += 0.00001;
                param_int = (GS::Int32) param_real;
                if (param_int / 1 < param_real) param_int += 1;
                param_string = GS::UniString::Printf ("%.1f", param_real);
                pvalue.canCalculate = true;
                pvalue.type = API_PropertyRealValueType;
                pvalue.formatstring = FormatStringFunc::ParseFormatString ("2m");
                break;
            case APIParT_ColRGB:
            case APIParT_Intens:
            case APIParT_RealNum:
                param_string = GS::UniString::Printf ("%.3f", param_real);
                pvalue.canCalculate = true;
                pvalue.type = API_PropertyRealValueType;
                pvalue.formatstring = FormatStringFunc::ParseFormatString ("3m");
                break;

                // Для реквезитов в текст выведем имена
            case APIParT_LineTyp:
                attrType = API_LinetypeID;
                break;
            case APIParT_Profile:
                attrType = API_ProfileID;
                break;
            case APIParT_BuildingMaterial:
                attrType = API_BuildingMaterialID;
                break;
            case APIParT_FillPat:
                attrType = API_FilltypeID;
                break;
            case APIParT_Mater:
                attrType = API_MaterialID;
                break;
            default:
                return false;
                break;
        }
        if (attrType != API_ZombieAttrID) {
            API_Attribute	attrib = {};
            attrib.header.typeID = attrType;
            attrib.header.index = attrInx;
            attrib.header.uniStringNamePtr = &param_string;
            GSErrCode err = ACAPI_Attribute_Get (&attrib);
            if (err == NoError) {
                pvalue.type = API_PropertyStringValueType;
                #if defined(AC_27) || defined(AC_28)
                param_bool = (attrInx.ToInt32_Deprecated () != 0);
                param_int = attrInx.ToInt32_Deprecated ();
                #else
                param_bool = (attrInx != 0);
                param_int = attrInx;
                #endif
                param_real = param_int / 1.0;
                pvalue.formatstring = FormatStringFunc::ParseFormatString ("0m");
            } else {
                if (err != APIERR_BADNAME) return false;
            }
        }
    }
    pvalue.boolValue = param_bool;
    pvalue.doubleValue = param_real;
    pvalue.rawDoubleValue = param_real;
    pvalue.hasrawDouble = true;
    pvalue.intValue = param_int;
    pvalue.uniStringValue = param_string;
    return true;
}

// -----------------------------------------------------------------------------
// Конвертация параметра-массива библиотечного элемента (тип API_ParArray) в ParamValue
// -----------------------------------------------------------------------------
bool ParamHelpers::ConvertToParamValue (ParamValueData & pvalue, const API_AddParID & typeIDr, const GS::Array<GS::UniString>&pstring, const GS::Array<double>&preal, const GS::Int32 & dim1, const GS::Int32 & dim2)
{

    // TODO Добавить обработку игнорируемых значений
    GS::Array<ParamValueData> pvalues;
    GS::UniString param_string = "";
    double param_real = 0.0;
    GS::Int32 inx_row = 1;
    GS::Int32 inx_col = 0;
    int array_row_start = pvalue.array_row_start;
    int array_row_end = pvalue.array_row_end;
    int array_column_start = pvalue.array_column_start;
    int array_column_end = pvalue.array_column_end;
    if (array_row_end < 1) array_row_end = dim1;
    if (array_column_end < 1) array_column_end = dim2;

    UInt32 n = 0;
    if (typeIDr == APIParT_CString) {
        if (pstring.IsEmpty ()) return false;
        n = pstring.GetSize ();
    } else {
        if (preal.IsEmpty ()) return false;
        n = preal.GetSize ();
    }
    for (UInt32 i = 0; i < n; i++) {
        inx_col += 1;
        if (inx_col > dim2) {
            inx_col = 1;
            inx_row += 1;
        }
        if (inx_col <= array_column_end && inx_col >= array_column_start && inx_row >= array_row_start && inx_row <= array_row_end) {
            ParamValueData pval = {};
            if (typeIDr == APIParT_CString) {
                param_string = pstring.Get (i);
            } else {
                param_real = preal.Get (i);
            }
            if (ParamHelpers::ConvertToParamValue (pval, typeIDr, param_string, param_real)) {
                pval.array_row_start = inx_row;
                pval.array_row_end = inx_row;
                pval.array_column_start = inx_col;
                pval.array_column_end = inx_col;
                pvalues.Push (pval);
            }
        }
    }
    if (pvalues.IsEmpty ()) {
        #if defined(TESTING)
        DBprnt ("ParamHelpers::ConvertToParamValue", "Empty array");
        #endif
        return false;
    } else {
        ParamHelpers::Array2ParamValue (pvalues, pvalue);
        return true;
    }
}

// -----------------------------------------------------------------------------
// Конвертация параметра библиотечного элемента в ParamValue
// -----------------------------------------------------------------------------
bool ParamHelpers::ConvertToParamValue (ParamValue & pvalue, const API_AddParType & nthParameter)
{
    GS::UniString param_string = "";
    double param_real = 0.0;
    API_AddParID typeIDr = nthParameter.typeID;
    ParamValueData pval = pvalue.val;
    if (nthParameter.typeMod == API_ParArray) {
        size_t ind = 0;
        GS::Array<GS::UniString> arr_param_string;
        GS::Array<double> arr_param_real;
        if (nthParameter.typeID != APIParT_CString) {
            for (Int32 i = 1; i <= nthParameter.dim1; i++) {
                for (Int32 j = 1; j <= nthParameter.dim2; j++) {
                    param_real = ((double*) *nthParameter.value.array)[ind];
                    arr_param_real.Push (param_real);
                    ind++;
                }
            }
        } else {
            Int32 arrayIndex = 0;
            for (Int32 i = 1; i <= nthParameter.dim1; i++) {
                for (Int32 j = 1; j <= nthParameter.dim2; j++) {
                    GS::uchar_t* uValueStr = (reinterpret_cast<GS::uchar_t*>(*nthParameter.value.array)) + arrayIndex;
                    arrayIndex += GS::ucslen32 (uValueStr) + 1;
                    arr_param_string.Push (GS::UniString (uValueStr));
                }
            }
        }
        if (!ParamHelpers::ConvertToParamValue (pval, typeIDr, arr_param_string, arr_param_real, nthParameter.dim1, nthParameter.dim2)) return false;
    }
    if (nthParameter.typeMod == API_ParSimple) {
        param_string = nthParameter.value.uStr;
        param_real = nthParameter.value.real;
        if (!ParamHelpers::ConvertToParamValue (pval, typeIDr, param_string, param_real)) return false;
    }
    if (pvalue.rawName.IsEmpty ()) {
        pvalue.rawName = "{@gdl:";
        pvalue.rawName.Append (GS::UniString (nthParameter.name).ToLowerCase ());
        pvalue.rawName.Append ("}");
    }
    if (pvalue.name.IsEmpty ()) pvalue.name = nthParameter.name;
    pvalue.val = pval;
    pvalue.type = pval.type;
    pvalue.fromGDLparam = true;
    pvalue.isValid = true;
    return true;
}

// -----------------------------------------------------------------------------
// Заполнение rawName для ParamValue по описанию в API_Property
// -----------------------------------------------------------------------------
void ParamHelpers::SetrawNameFromProperty (ParamValue & pvalue, const API_Property & property)
{
    if (pvalue.rawName.IsEmpty () || pvalue.name.IsEmpty ()) {
        GS::UniString fname;
        GetPropertyFullName (property.definition, fname);
        if (pvalue.rawName.IsEmpty ()) {
            pvalue.rawName = "{@property:";
            pvalue.rawName.Append (fname.ToLowerCase ());
            pvalue.rawName.Append ("}");
        }
        if (pvalue.name.IsEmpty ()) pvalue.name = fname;
    }
    GS::UniString description = property.definition.description.ToLowerCase ();
    if (description.Contains ("sync_correct_flag")) {
        pvalue.rawName = "{@property:sync_correct_flag}";
        return;
    }
    if (description.Contains ("some_stuff_th")) {
        // Заданная толщина в материале. Используется, если не удалось вычислить из профиля
        GS::UniString inx = "";
        if (pvalue.rawName.Contains (CharENTER)) inx = CharENTER + pvalue.rawName.GetSubstring (CharENTER, '}', 0);
        pvalue.rawName = "{@property:buildingmaterialproperties/some_stuff_th" + inx + "}";
        pvalue.name = "some_stuff_th";
        pvalue.val.formatstring = FormatStringFunc::ParseFormatString ("1mm");
        return;
    }
    if (description.Contains ("some_stuff_units")) {
        // Единицы измерения
        GS::UniString inx = "";
        if (pvalue.rawName.Contains (CharENTER)) inx = CharENTER + pvalue.rawName.GetSubstring (CharENTER, '}', 0);
        pvalue.rawName = "{@property:buildingmaterialproperties/some_stuff_units" + inx + "}";
        pvalue.name = "some_stuff_units";
        return;
    }
    if (description.Contains ("some_stuff_kzap")) {
        // Единицы измерения
        GS::UniString inx = "";
        if (pvalue.rawName.Contains (CharENTER)) inx = CharENTER + pvalue.rawName.GetSubstring (CharENTER, '}', 0);
        pvalue.rawName = "{@property:buildingmaterialproperties/some_stuff_kzap" + inx + "}";
        pvalue.name = "some_stuff_kzap";
        return;
    }
}


// -----------------------------------------------------------------------------
// Конвертация свойства в ParamValue
// -----------------------------------------------------------------------------
bool ParamHelpers::ConvertToParamValue (ParamValue & pvalue, const API_Property & property)
{
    if (pvalue.rawName.IsEmpty () || pvalue.name.IsEmpty ()) ParamHelpers::SetrawNameFromProperty (pvalue, property);
    if (property.definition.description.Contains ("Sync_correct_flag")) pvalue.rawName = "{@property:sync_correct_flag}";
    API_PropertyValue value = {};
    #if defined(AC_22) || defined(AC_23)
    pvalue.isValid = property.isEvaluated;
    if (property.isDefault && !property.isEvaluated) {
        value = property.definition.defaultValue.basicValue;
    } else {
        value = property.value;
    }
    #else
    pvalue.isValid = (property.status == API_Property_HasValue);
    if (property.isDefault && property.status == API_Property_NotEvaluated) {
        value = property.definition.defaultValue.basicValue;
    } else {
        value = property.value;
    }
    #endif
    pvalue.fromProperty = true;

    pvalue.definition = property.definition;
    pvalue.property = property;
    ParamHelpers::ConvertToParamValue_CheckAttrib (pvalue, property.definition);
    if (!pvalue.fromAttribDefinition) pvalue.fromPropertyDefinition = true;
    if (!pvalue.isValid && property.definition.guid == APINULLGuid) {
        return false;
    }
    pvalue.val.uniStringValue = PropertyHelpers::ToString (property);
    if (pvalue.val.uniStringValue.IsEqual ("@Undefined Value@") && property.isDefault) {
        pvalue.isValid = false;
        return false;
    }
    FormatStringDict formatstringdict = {};
    switch (property.definition.valueType) {
        case API_PropertyIntegerValueType:
            pvalue.val.intValue = value.singleVariant.variant.intValue;
            pvalue.val.doubleValue = value.singleVariant.variant.intValue * 1.0;
            pvalue.val.rawDoubleValue = pvalue.val.doubleValue;
            pvalue.val.hasrawDouble = true;
            if (pvalue.val.intValue > 0) pvalue.val.boolValue = true;
            pvalue.val.type = API_PropertyIntegerValueType;
            pvalue.val.canCalculate = true;
            pvalue.val.formatstring = FormatStringFunc::ParseFormatString ("0m");
            pvalue.val.uniStringValue = PropertyHelpers::ToString (property, pvalue.val.formatstring);
            break;
        case API_PropertyRealValueType:

            // Конвертация угла из радиан в градусы
            if (property.definition.measureType == API_PropertyAngleMeasureType) {
                double ang = value.singleVariant.variant.doubleValue * 180.0 / PI;
                pvalue.val.rawDoubleValue = ang;
                pvalue.val.hasrawDouble = true;
                pvalue.val.doubleValue = round (ang * 100000.0) / 100000.0;
            } else {
                pvalue.val.rawDoubleValue = value.singleVariant.variant.doubleValue;
                pvalue.val.hasrawDouble = true;
                pvalue.val.doubleValue = round (value.singleVariant.variant.doubleValue * 100000.0) / 100000.0;
                if (value.singleVariant.variant.doubleValue - pvalue.val.doubleValue > 0.001) pvalue.val.doubleValue += 0.001;
            }
            if (pvalue.rawName.IsEqual ("{@property:buildingmaterialproperties/some_stuff_th}")) {
                if (property.definition.measureType != API_PropertyLengthMeasureType) {
                    pvalue.val.rawDoubleValue = pvalue.val.rawDoubleValue / 1000.0;
                    pvalue.val.doubleValue = pvalue.val.doubleValue / 1000.0;
                    pvalue.val.doubleValue = round (pvalue.val.doubleValue * 100000.0) / 100000.0;
                    pvalue.val.rawDoubleValue = round (pvalue.val.rawDoubleValue * 100000.0) / 100000.0;
                }
            }
            pvalue.val.intValue = (GS::Int32) pvalue.val.doubleValue;
            if (pvalue.val.intValue / 1 < pvalue.val.doubleValue) pvalue.val.intValue += 1;
            if (fabs (pvalue.val.doubleValue) > std::numeric_limits<double>::epsilon ()) pvalue.val.boolValue = true;
            pvalue.val.type = API_PropertyRealValueType;
            formatstringdict = FormatStringFunc::GetFotmatStringForMeasureType ();
            if (formatstringdict.ContainsKey (property.definition.measureType)) {
                int n_zero = formatstringdict.Get (property.definition.measureType).n_zero;
                GS::UniString stringformat = formatstringdict.Get (property.definition.measureType).stringformat;
                bool needRound = formatstringdict.Get (property.definition.measureType).needRound;
                if (pvalue.rawName.IsEqual ("{@property:buildingmaterialproperties/some_stuff_th}")) needRound = false;
                if (needRound && property.definition.measureType != API_PropertyLengthMeasureType) {
                    double l = pow (10, n_zero);
                    pvalue.val.doubleValue = round (pvalue.val.doubleValue * pow (10, n_zero)) / pow (10, n_zero);
                    pvalue.val.intValue = (GS::Int32) pvalue.val.doubleValue;
                    if (fabs (pvalue.val.doubleValue) > std::numeric_limits<double>::epsilon ()) pvalue.val.boolValue = true;
                }
                if (pvalue.val.formatstring.isEmpty) {
                    pvalue.val.formatstring = FormatStringFunc::ParseFormatString (stringformat);
                    pvalue.val.formatstring.needRound = needRound;
                }
                pvalue.val.uniStringValue = ParamHelpers::ToString (pvalue);
            }
            pvalue.val.canCalculate = true;
            break;
        case API_PropertyBooleanValueType:
            pvalue.val.boolValue = value.singleVariant.variant.boolValue;
            if (pvalue.val.boolValue) {
                pvalue.val.intValue = 1;
                pvalue.val.doubleValue = 1.0;
            }
            pvalue.val.type = API_PropertyBooleanValueType;
            pvalue.val.canCalculate = true;
            pvalue.val.formatstring = FormatStringFunc::ParseFormatString ("0m");
            pvalue.val.rawDoubleValue = pvalue.val.doubleValue;
            pvalue.val.hasrawDouble = true;
            pvalue.val.uniStringValue = PropertyHelpers::ToString (property);
            break;
        case API_PropertyStringValueType:
        case API_PropertyGuidValueType:
            pvalue.val.uniStringValue = PropertyHelpers::ToString (property);
            pvalue.val.type = API_PropertyStringValueType;
            pvalue.val.boolValue = !pvalue.val.uniStringValue.IsEmpty ();
            if (UniStringToDouble (pvalue.val.uniStringValue, pvalue.val.doubleValue)) {
                pvalue.val.intValue = (GS::Int32) pvalue.val.doubleValue;
                if (pvalue.val.intValue / 1 < pvalue.val.doubleValue) pvalue.val.intValue += 1;
                pvalue.val.canCalculate = true;
            } else {
                if (pvalue.val.boolValue) {
                    pvalue.val.intValue = 1;
                    pvalue.val.doubleValue = 1.0;
                }
            }
            pvalue.val.rawDoubleValue = pvalue.val.doubleValue;
            pvalue.val.hasrawDouble = true;
            break;
        case API_PropertyUndefinedValueType:
            pvalue.isValid = false;
            return false;
            break;
        default:
            pvalue.isValid = false;
            return false;
            break;
    }
    pvalue.type = pvalue.val.type;
    return true;
}

void ParamHelpers::ConvertToParamValue_CheckAttrib (ParamValue & pvalue, const API_PropertyDefinition & definition)
{
    GS::UniString description = definition.description.ToLowerCase ();
    if (description.Contains ("to{Class:")) {
        GS::Array<GS::UniString> params = {};
        UInt32 nparam = StringSplt (definition.description, "to{Class", params);
        if (nparam > 1) {
            GS::UniString systemname = params.Get (1).GetSubstring (':', '}', 0);
            pvalue.name = systemname.ToLowerCase ();
        }
        pvalue.fromClassification = true;
        return;
    }
    if (description.Contains ("some_stuff_th")) {
        // Заданная толщина в материале. Используется, если не удалось вычислить из профиля
        pvalue.rawName = "{@property:buildingmaterialproperties/some_stuff_th}";
        pvalue.name = "some_stuff_th";
        pvalue.fromAttribDefinition = true;
        pvalue.val.formatstring = FormatStringFunc::ParseFormatString ("1mm");
        return;
    }
    if (description.Contains ("some_stuff_units")) {
        // Единицы измерения
        pvalue.rawName = "{@property:buildingmaterialproperties/some_stuff_units}";
        pvalue.name = "some_stuff_units";
        pvalue.fromAttribDefinition = true;
        return;
    }
    if (description.Contains ("some_stuff_kzap")) {
        // Единицы измерения
        pvalue.rawName = "{@property:buildingmaterialproperties/some_stuff_kzap}";
        pvalue.name = "some_stuff_kzap";
        pvalue.fromAttribDefinition = true;
        return;
    }
    if (description.Contains ("ync_name")) {
        if (!pvalue.rawName.Contains ("{@property:sync_name")) {
            pvalue.rawName = "{@property:sync_name0}";
            pvalue.name = "Sync_name0";
        }
        pvalue.fromAttribDefinition = true;
        return;
    }

    if (pvalue.rawName.Contains ("buildingmaterial")) {
        pvalue.fromAttribDefinition = true;
        return;
    }
    if (pvalue.rawName.Contains ("component")) {
        pvalue.fromAttribDefinition = true;
        return;
    }
    if (description.Contains ("{@property:buildingmaterialproperties}")) {
        pvalue.fromAttribDefinition = true;
        return;
    }
    if (description.Contains ("some_stuff_layer_onoff")) {
        pvalue.fromAttribDefinition = true;
        return;
    }
    if (description.Contains ("some_stuff_layer_has_finish")) {
        pvalue.fromAttribDefinition = true;
        return;
    }
    if (description.Contains ("some_stuff_layer_description")) {
        pvalue.fromAttribDefinition = true;
        return;
    }
    if (description.Contains ("some_stuff_layer_favorite_name")) {
        pvalue.fromAttribDefinition = true;
        return;
    }
}

// -----------------------------------------------------------------------------
// Конвертация определения свойства в ParamValue
// -----------------------------------------------------------------------------
bool ParamHelpers::ConvertToParamValue (ParamValue & pvalue, const API_PropertyDefinition & definition)
{
    if (pvalue.rawName.IsEmpty () || pvalue.name.IsEmpty ()) {
        GS::UniString fname = "";
        GetPropertyFullName (definition, fname);
        if (pvalue.rawName.IsEmpty ()) {
            pvalue.rawName = "{@property:";
            pvalue.rawName.Append (fname.ToLowerCase ());
            pvalue.rawName.Append ("}");
        }

        if (pvalue.name.IsEmpty ()) pvalue.name = fname;
    }
    ParamHelpers::ConvertToParamValue_CheckAttrib (pvalue, definition);
    pvalue.fromProperty = true;
    if (!pvalue.fromAttribDefinition) pvalue.fromPropertyDefinition = true;
    pvalue.definition = definition;
    return true;
}

// -----------------------------------------------------------------------------
// Конвертация строки в ParamValue
// -----------------------------------------------------------------------------
bool ParamHelpers::ConvertStringToParamValue (ParamValue & pvalue, const GS::UniString & paramName, const GS::UniString strvalue)
{
    if (pvalue.name.IsEmpty ()) pvalue.name = paramName;
    if (pvalue.rawName.IsEmpty ()) {
        pvalue.rawName = "{@gdl:";
        pvalue.rawName.Append (paramName.ToLowerCase ());
        pvalue.rawName.Append ("}");
    }
    pvalue.val.uniStringValue = strvalue;
    pvalue.val.boolValue = !pvalue.val.uniStringValue.IsEmpty ();
    if (UniStringToDouble (pvalue.val.uniStringValue, pvalue.val.doubleValue)) {
        pvalue.val.intValue = (GS::Int32) pvalue.val.doubleValue;
        if (pvalue.val.intValue / 1 < pvalue.val.doubleValue) pvalue.val.intValue += 1;
        pvalue.val.canCalculate = true;
    } else {
        if (pvalue.val.boolValue) {
            pvalue.val.intValue = 1;
            pvalue.val.doubleValue = 1.0;
        }
    }
    pvalue.val.rawDoubleValue = pvalue.val.doubleValue;
    pvalue.val.hasrawDouble = true;
    pvalue.val.type = API_PropertyStringValueType;
    pvalue.type = API_PropertyStringValueType;
    pvalue.isValid = true;
    return true;
}

// -----------------------------------------------------------------------------
// Конвертация целого числа в ParamValue
// -----------------------------------------------------------------------------
bool ParamHelpers::ConvertBoolToParamValue (ParamValue & pvalue, const GS::UniString & paramName, const bool boolValue)
{
    if (pvalue.name.IsEmpty ()) pvalue.name = paramName;
    if (pvalue.rawName.IsEmpty ()) {
        pvalue.rawName = "{@gdl:";
        pvalue.rawName.Append (paramName.ToLowerCase ());
        pvalue.rawName.Append ("}");
    }
    pvalue.val.type = API_PropertyBooleanValueType;
    pvalue.type = pvalue.val.type;
    pvalue.val.boolValue = boolValue;
    if (pvalue.val.boolValue) {
        pvalue.val.uniStringValue = RSGetIndString (ID_ADDON_STRINGS + isEng (), TrueId, ACAPI_GetOwnResModule ());
        pvalue.val.intValue = 1;
        pvalue.val.doubleValue = 1.0;
    } else {
        pvalue.val.uniStringValue = RSGetIndString (ID_ADDON_STRINGS + isEng (), FalseId, ACAPI_GetOwnResModule ());
        pvalue.val.intValue = 0;
        pvalue.val.doubleValue = 0.0;
    }
    pvalue.val.rawDoubleValue = pvalue.val.doubleValue;
    pvalue.val.hasrawDouble = true;
    pvalue.val.canCalculate = true;
    pvalue.isValid = true;
    if (pvalue.val.formatstring.isEmpty) pvalue.val.formatstring = FormatStringFunc::ParseFormatString ("0m");
    return true;
}

// -----------------------------------------------------------------------------
// Конвертация аттрибута в ParamValue
// -----------------------------------------------------------------------------
bool ParamHelpers::ConvertAttributeToParamValue (ParamValue & pvalue, const GS::UniString & paramName, const API_Attribute attr)
{
    if (pvalue.name.IsEmpty ()) pvalue.name = paramName;
    if (pvalue.rawName.IsEmpty ()) {
        pvalue.rawName = "{@attrib:";
        pvalue.rawName.Append (paramName.ToLowerCase ());
        pvalue.rawName.Append ("}");
    }
    #if defined(AC_27) || defined(AC_28)
    pvalue.val.intValue = attr.header.index.ToInt32_Deprecated ();
    #else
    pvalue.val.intValue = attr.header.index;
    #endif
    pvalue.val.doubleValue = pvalue.val.intValue * 1.0;
    pvalue.val.rawDoubleValue = pvalue.val.doubleValue;
    pvalue.val.hasrawDouble = true;
    if (pvalue.val.intValue > 0) pvalue.val.boolValue = true;
    pvalue.val.uniStringValue = GS::UniString (attr.header.name);
    pvalue.val.type = API_PropertyStringValueType;
    pvalue.type = API_PropertyStringValueType;
    pvalue.isValid = true;
    pvalue.fromAttribElement = true;
    return true;
}

// -----------------------------------------------------------------------------
// Конвертация целого числа в ParamValue
// -----------------------------------------------------------------------------
bool ParamHelpers::ConvertIntToParamValue (ParamValue & pvalue, const GS::UniString & paramName, const Int32 intValue)
{
    if (pvalue.name.IsEmpty ()) pvalue.name = paramName;
    if (pvalue.rawName.IsEmpty ()) {
        pvalue.rawName = "{@gdl:";
        pvalue.rawName.Append (paramName.ToLowerCase ());
        pvalue.rawName.Append ("}");
    }
    pvalue.val.type = API_PropertyIntegerValueType;
    pvalue.type = pvalue.val.type;
    pvalue.val.canCalculate = true;
    pvalue.val.intValue = intValue;
    pvalue.val.doubleValue = intValue * 1.0;
    pvalue.val.rawDoubleValue = pvalue.val.doubleValue;
    pvalue.val.hasrawDouble = true;
    if (pvalue.val.intValue > 0) pvalue.val.boolValue = true;
    pvalue.val.uniStringValue = GS::UniString::Printf ("%d", intValue);
    pvalue.isValid = true;
    if (pvalue.val.formatstring.isEmpty) pvalue.val.formatstring = FormatStringFunc::ParseFormatString ("0m");
    return true;
}

// -----------------------------------------------------------------------------
// Конвертация double в ParamValue
// -----------------------------------------------------------------------------
bool ParamHelpers::ConvertDoubleToParamValue (ParamValue & pvalue, const GS::UniString & paramName, const double doubleValue)
{
    if (pvalue.name.IsEmpty ()) pvalue.name = paramName;
    if (pvalue.rawName.IsEmpty ()) {
        pvalue.rawName = "{@gdl:";
        pvalue.rawName.Append (paramName.ToLowerCase ());
        pvalue.rawName.Append ("}");
    }
    pvalue.val.type = API_PropertyRealValueType;
    pvalue.type = pvalue.val.type;
    pvalue.val.canCalculate = true;
    pvalue.val.intValue = (GS::Int32) doubleValue;
    pvalue.val.doubleValue = doubleValue;
    pvalue.val.rawDoubleValue = pvalue.val.doubleValue;
    pvalue.val.hasrawDouble = true;
    pvalue.val.boolValue = false;
    if (fabs (pvalue.val.doubleValue) > std::numeric_limits<double>::epsilon ()) pvalue.val.boolValue = true;
    pvalue.val.uniStringValue = GS::UniString::Printf ("%.3f", doubleValue);
    pvalue.isValid = true;
    return true;
}

// -----------------------------------------------------------------------------
// Конвертация API_IFCProperty в ParamValue
// -----------------------------------------------------------------------------
bool ParamHelpers::ConvertToParamValue (ParamValue & pvalue, const API_IFCProperty & property)
{
    if (pvalue.rawName.IsEmpty () || pvalue.name.IsEmpty ()) {
        GS::UniString fname = property.head.propertySetName + "/" + property.head.propertyName;
        if (pvalue.rawName.IsEmpty ()) pvalue.rawName = "{@ifc:" + fname.ToLowerCase () + "}";
        if (pvalue.name.IsEmpty ()) pvalue.name = fname;
    }
    pvalue.isValid = true;
    if (property.head.propertyType == API_IFCPropertySingleValueType) {
        switch (property.singleValue.nominalValue.value.primitiveType) {
            case API_IFCPropertyAnyValueStringType:
                pvalue.val.type = API_PropertyStringValueType;
                pvalue.val.uniStringValue = property.singleValue.nominalValue.value.stringValue;
                pvalue.val.boolValue = !pvalue.val.uniStringValue.IsEmpty ();
                if (UniStringToDouble (pvalue.val.uniStringValue, pvalue.val.doubleValue)) {
                    pvalue.val.intValue = (GS::Int32) pvalue.val.doubleValue;
                    if (pvalue.val.intValue / 1 < pvalue.val.doubleValue) pvalue.val.intValue += 1;
                    pvalue.val.canCalculate = true;
                } else {
                    if (pvalue.val.boolValue) {
                        pvalue.val.intValue = 1;
                        pvalue.val.doubleValue = 1.0;
                    }
                }
                pvalue.val.rawDoubleValue = pvalue.val.doubleValue;
                pvalue.val.hasrawDouble = true;
                break;
            case API_IFCPropertyAnyValueRealType:
                pvalue.val.canCalculate = true;
                pvalue.val.type = API_PropertyRealValueType;
                pvalue.val.doubleValue = round (property.singleValue.nominalValue.value.doubleValue * 1000) / 1000;
                if (property.singleValue.nominalValue.value.doubleValue - pvalue.val.doubleValue > 0.001) pvalue.val.doubleValue += 0.001;
                pvalue.val.intValue = (GS::Int32) pvalue.val.doubleValue;
                if (pvalue.val.intValue / 1 < pvalue.val.doubleValue) pvalue.val.intValue += 1;
                if (fabs (pvalue.val.doubleValue) > std::numeric_limits<double>::epsilon ()) pvalue.val.boolValue = true;
                pvalue.val.uniStringValue = GS::UniString::Printf ("%.3f", pvalue.val.doubleValue);
                pvalue.val.rawDoubleValue = pvalue.val.doubleValue;
                pvalue.val.hasrawDouble = true;
                break;
            case API_IFCPropertyAnyValueIntegerType:
                pvalue.val.canCalculate = true;
                pvalue.val.type = API_PropertyIntegerValueType;
                pvalue.val.intValue = (GS::Int32) property.singleValue.nominalValue.value.intValue;
                pvalue.val.doubleValue = pvalue.val.intValue * 1.0;
                if (pvalue.val.intValue > 0) pvalue.val.boolValue = true;
                pvalue.val.uniStringValue = GS::UniString::Printf ("%d", pvalue.val.intValue);
                pvalue.val.formatstring = FormatStringFunc::ParseFormatString ("0m");
                pvalue.val.rawDoubleValue = pvalue.val.doubleValue;
                pvalue.val.hasrawDouble = true;
                break;
            case API_IFCPropertyAnyValueBooleanType:
                pvalue.val.canCalculate = true;
                pvalue.val.type = API_PropertyBooleanValueType;
                pvalue.val.boolValue = property.singleValue.nominalValue.value.boolValue;
                if (pvalue.val.boolValue) {
                    pvalue.val.uniStringValue = RSGetIndString (ID_ADDON_STRINGS + isEng (), TrueId, ACAPI_GetOwnResModule ());
                    pvalue.val.intValue = 1;
                    pvalue.val.doubleValue = 1.0;
                } else {
                    pvalue.val.uniStringValue = RSGetIndString (ID_ADDON_STRINGS + isEng (), FalseId, ACAPI_GetOwnResModule ());
                    pvalue.val.intValue = 0;
                    pvalue.val.doubleValue = 0.0;
                }
                pvalue.val.rawDoubleValue = pvalue.val.doubleValue;
                pvalue.val.hasrawDouble = true;
                break;
            case API_IFCPropertyAnyValueLogicalType:
                pvalue.val.formatstring = FormatStringFunc::ParseFormatString ("0m");
                pvalue.val.type = API_PropertyBooleanValueType;
                if (property.singleValue.nominalValue.value.intValue == 0) pvalue.val.boolValue = false;
                if (property.singleValue.nominalValue.value.intValue == 1) pvalue.isValid = false;
                if (property.singleValue.nominalValue.value.intValue == 2) pvalue.val.boolValue = true;
                if (pvalue.val.boolValue) {
                    pvalue.val.uniStringValue = RSGetIndString (ID_ADDON_STRINGS + isEng (), TrueId, ACAPI_GetOwnResModule ());
                    pvalue.val.intValue = 1;
                    pvalue.val.doubleValue = 1.0;
                } else {
                    pvalue.val.uniStringValue = RSGetIndString (ID_ADDON_STRINGS + isEng (), FalseId, ACAPI_GetOwnResModule ());
                    pvalue.val.intValue = 0;
                    pvalue.val.doubleValue = 0.0;
                }
                pvalue.val.rawDoubleValue = pvalue.val.doubleValue;
                pvalue.val.hasrawDouble = true;
                break;
            default:
                pvalue.val.canCalculate = false;
                pvalue.isValid = false;
                break;
        }
    }
    pvalue.type = pvalue.val.type;
    return pvalue.isValid;
}
void ParamHelpers::ConvertByFormatString (ParamValue & pvalue)
{
    if (pvalue.val.type == API_PropertyRealValueType || pvalue.val.type == API_PropertyIntegerValueType) {
        Int32 n_zero = pvalue.val.formatstring.n_zero;
        Int32 krat = pvalue.val.formatstring.krat;
        double koeff = pvalue.val.formatstring.koeff;
        bool trim_zero = pvalue.val.formatstring.trim_zero;
        if (!pvalue.val.hasrawDouble) {
            pvalue.val.rawDoubleValue = pvalue.val.doubleValue;
            pvalue.val.hasrawDouble = true;
        }
        pvalue.val.uniStringValue = ParamHelpers::ToString (pvalue);
        if (koeff != 1) n_zero = n_zero + (GS::Int32) log10 (koeff);
        pvalue.val.doubleValue = round (pvalue.val.doubleValue * pow (10, n_zero)) / pow (10, n_zero);
        pvalue.val.intValue = (GS::Int32) pvalue.val.doubleValue;
        if (fabs (pvalue.val.doubleValue) > std::numeric_limits<double>::epsilon ()) pvalue.val.boolValue = true;
    }
}

GS::UniString ParamHelpers::ToString (const ParamValue & pvalue)
{
    FormatString stringformat = pvalue.val.formatstring;
    return ParamHelpers::ToString (pvalue, stringformat);
}

GS::UniString ParamHelpers::ToString (const ParamValue & pvalue, const FormatString stringformat)
{
    switch (pvalue.val.type) {
        case API_PropertyIntegerValueType: return  FormatStringFunc::NumToString (pvalue.val.intValue, stringformat);
        case API_PropertyRealValueType:
            if (stringformat.needRound) {
                return FormatStringFunc::NumToString (pvalue.val.doubleValue, stringformat);
            } else {
                return FormatStringFunc::NumToString (pvalue.val.rawDoubleValue, stringformat);
            }
        case API_PropertyStringValueType: return pvalue.val.uniStringValue;
        case API_PropertyBooleanValueType: return GS::ValueToUniString (pvalue.val.boolValue);
        default:
            DBBREAK ();
            return "Invalid Value";
    }
}

// --------------------------------------------------------------------
// Получение данных из однородной конструкции
// --------------------------------------------------------------------
bool ParamHelpers::ComponentsBasicStructure (const API_AttributeIndex & constrinx, const double& fillThick, const API_AttributeIndex & constrinx_ven, const double& fillThick_ven, ParamDictValue & params, ParamDictValue & paramlayers, ParamDictValue & paramsAdd, short& structype_ven)
{
    #if defined(TESTING)
    DBprnt ("        ComponentsBasicStructure\n");
    #endif
    ParamValue param_composite = {};
    if (fillThick_ven > 0.0001) {
        ParamValueComposite layer = {};
        layer.inx = constrinx_ven;
        layer.fillThick = fillThick_ven;
        layer.num = 2;
        layer.structype = structype_ven;
        param_composite.composite.Push (layer);
        ParamHelpers::GetAttributeValues (constrinx_ven, params, paramsAdd);
    }
    ParamValueComposite layer = {};
    layer.inx = constrinx;
    layer.fillThick = fillThick;
    layer.num = 1;
    layer.structype = APICWallComp_Core;
    param_composite.composite.Push (layer);
    ParamHelpers::GetAttributeValues (constrinx, params, paramsAdd);
    for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = paramlayers.EnumeratePairs (); cIt != NULL; ++cIt) {
        #if defined(AC_28)
        paramlayers.Get (cIt->key).composite = param_composite.composite;
        #else
        paramlayers.Get (*cIt->key).composite = param_composite.composite;
        #endif
    }
    ParamHelpers::CompareParamDictValue (paramlayers, params);
    return true;
}

void ParamHelpers::ComponentsGetUnic (GS::Array<ParamValueComposite>&composite)
{
    GS::Array<ParamValueComposite> p = {};
    GS::HashTable<GS::UniString, ParamValueComposite> existsmaterial = {};
    for (const auto& c : composite) {
        GS::UniString key = GS::UniString::Printf ("%d", c.inx) + GS::UniString::Printf ("_%.4f", c.fillThick);
        if (existsmaterial.ContainsKey (key)) {
            ParamValueComposite& e = existsmaterial.Get (key);
            e.area += c.area;
            e.volume += c.volume;
            e.area_fill += c.area_fill;
            e.length += c.length;
            e.qty += c.qty;
        } else {
            existsmaterial.Add (key, c);
        }
    }
    if (existsmaterial.IsEmpty ()) return;
    for (GS::HashTable<GS::UniString, ParamValueComposite>::PairIterator cIt = existsmaterial.EnumeratePairs (); cIt != NULL; ++cIt) {
        #if defined(AC_28)
        ParamValueComposite c = cIt->value;
        #else
        ParamValueComposite c = *cIt->value;
        #endif
        p.Push (c);
    }
    composite.Clear ();
    composite = p;
    return;
}

// --------------------------------------------------------------------
// Получение данных из многослойной конструкции
// --------------------------------------------------------------------
bool ParamHelpers::ComponentsCompositeStructure (const API_Guid & elemguid, API_AttributeIndex & constrinx, ParamDictValue & params, ParamDictValue & paramlayers, ParamDictValue & paramsAdd, GS::HashTable<API_AttributeIndex, bool>&existsmaterial)
{
    #if defined(TESTING)
    DBprnt ("        ComponentsCompositeStructure");
    #endif
    API_Attribute attrib = {};
    BNZeroMemory (&attrib, sizeof (API_Attribute));

    API_AttributeDef defs = {};
    BNZeroMemory (&defs, sizeof (API_AttributeDef));

    attrib.header.index = constrinx;
    attrib.header.typeID = API_CompWallID;
    GSErrCode err = ACAPI_Attribute_Get (&attrib);
    if (err != NoError) {
        msg_rep ("materialString::ComponentsCompositeStructure", " ACAPI_Attribute_Get", err, elemguid);
        return false;
    }
    err = ACAPI_Attribute_GetDef (attrib.header.typeID, attrib.header.index, &defs);
    if (err != NoError) {
        msg_rep ("materialString::ComponentsCompositeStructure", " ACAPI_Attribute_GetDef", err, elemguid);
        ACAPI_DisposeAttrDefsHdls (&defs);
        return false;
    }
    ParamValue param_composite = {};
    for (short i = 0; i < attrib.compWall.nComps; i++) {
        API_AttributeIndex	constrinxL = (*defs.cwall_compItems)[i].buildingMaterial;
        double	fillThickL = (*defs.cwall_compItems)[i].fillThick;
        ParamValueComposite layer = {};
        layer.inx = constrinxL;
        layer.fillThick = fillThickL;
        layer.num = i + 1;
        layer.structype = (*defs.cwall_compItems)[i].flagBits;
        param_composite.composite.Push (layer);
        if (!existsmaterial.ContainsKey (constrinxL)) {
            ParamHelpers::GetAttributeValues (constrinxL, params, paramsAdd);
            existsmaterial.Add (constrinxL, true);
        }
    }
    for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = paramlayers.EnumeratePairs (); cIt != NULL; ++cIt) {
        #if defined(AC_28)
        paramlayers.Get (cIt->key).composite = param_composite.composite;
        #else
        paramlayers.Get (*cIt->key).composite = param_composite.composite;
        #endif
    }
    ParamHelpers::CompareParamDictValue (paramlayers, params);
    ACAPI_DisposeAttrDefsHdls (&defs);
    return true;
}

// --------------------------------------------------------------------
// Получение данных из сложного профиля
// --------------------------------------------------------------------

bool ParamHelpers::ComponentsProfileStructure (ProfileVectorImage & profileDescription, ParamDictValue & params, ParamDictValue & paramlayers, ParamDictValue & paramsAdd, GS::HashTable<API_AttributeIndex, bool>&existsmaterial)
{
    #if !defined(AC_22) && !defined(AC_23)
    #if defined(TESTING)
    DBprnt ("        ComponentsProfileStructure");
    #endif
    ConstProfileVectorImageIterator profileDescriptionIt (profileDescription);
    GS::HashTable<short, OrientedSegments> lines = {}; // Для хранения точки начала сечения и линии сечения
    GS::HashTable<short, GS::Array<Sector>> segment = {}; // Для хранения отрезков линий сечения и последующего объединения
    GS::HashTable<short, ParamValue> param_composite = {};

    // Получаем список перьев в параметрах
    bool needReadQuantities = false;
    for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = paramlayers.EnumeratePairs (); cIt != NULL; ++cIt) {
        #if defined(AC_28)
        GS::UniString rawName = cIt->key;
        #else
        GS::UniString rawName = *cIt->key;
        #endif
        short pen = paramlayers.Get (rawName).composite_pen;
        if (pen < 0) needReadQuantities = true;
        OrientedSegments s = {};
        GS::Array<Sector> segments = {};
        lines.Add (pen, s);
        segment.Add (pen, segments);
        ParamValue p = {};
        param_composite.Add (pen, p);
        if (!needReadQuantities) {
            if (paramlayers.Get (rawName).fromQuantity) needReadQuantities = true;
        }
    }
    #if defined(TESTING)
    if (needReadQuantities) DBprnt ("        Quantities ProfileStructure");
    #endif
    bool hasLine = !lines.IsEmpty ();
    bool profilehasLine = false;
    GS::Array<ParamValueComposite> composite_all = {};
    // Ищем полилинию с нужным цветом
    while (!profileDescriptionIt.IsEOI ()) {
        switch (profileDescriptionIt->item_Typ) {
            case SyArc: // Указателем начала линии служит окружность с тем же пером
                {
                    const Sy_ArcType* pSyArc = static_cast <const Sy_ArcType*> (profileDescriptionIt);
                    short pen = pSyArc->GetExtendedPen ().GetIndex ();
                    if (lines.ContainsKey (pen)) {
                        Point2D s = { pSyArc->origC };
                        lines.Get (pen).start = s;
                    }
                }
                break;
            case SyLine: // Поиск линий-сечений
                {
                    const Sy_LinType* pSyPolyLine = static_cast <const Sy_LinType*> (profileDescriptionIt);
                    short pen = pSyPolyLine->GetExtendedPen ().GetIndex ();
                    if (segment.ContainsKey (pen)) {
                        Sector line = { pSyPolyLine->begC, pSyPolyLine->endC };
                        segment.Get (pen).Push (line);
                        profilehasLine = true;
                    }
                }
                break;
        }
        ++profileDescriptionIt;
    }

    // Если линии сечения не найдены - создадим парочку - вертикальную и горизонтальную
    if (!profilehasLine) {
        Point2D p1 = { -1000, 0 };
        Point2D p2 = { 1000, 0 };
        Sector cut1 = { p1, p2 };
        OrientedSegments d = {};
        d.start = p2;
        d.cut_start = p1;
        d.cut_direction = Geometry::SectorVector (cut1);
        if (lines.ContainsKey (20)) {
            lines.Set (20, d);
        } else {
            lines.Add (20, d);
        }
        Point2D p3 = { 0, -1000 };
        Point2D p4 = { 0, 1000 };
        Sector cut2 = { p3, p4 };
        OrientedSegments d2 = {};
        d2.start = p3;
        d2.cut_start = p4;
        d2.cut_direction = Geometry::SectorVector (cut2);
        if (lines.ContainsKey (6)) {
            lines.Set (6, d2);
        } else {
            lines.Add (6, d2);
        }
        ParamValue p = {};
        param_composite.Add (20, p);
        param_composite.Add (6, p);
    } else {
        // Проходим по сегментам, соединяем их в одну линию
        for (GS::HashTable<short, GS::Array<Sector>>::PairIterator cIt = segment.EnumeratePairs (); cIt != NULL; ++cIt) {
            #if defined(AC_28)
            GS::Array<Sector>& segment = cIt->value;
            Point2D pstart = lines.Get (cIt->key).start;
            #else
            GS::Array<Sector>& segment = *cIt->value;
            Point2D pstart = lines.Get (*cIt->key).start;
            #endif
            Sector cutline;
            double max_r = 0; double min_r = 300000;
            for (UInt32 j = 0; j < segment.GetSize (); j++) {
                double r = Geometry::Dist (pstart, segment[j].c1);
                if (r > max_r) {
                    cutline.c1 = segment[j].c1;
                    max_r = r;
                }
                if (r < min_r) {
                    cutline.c2 = segment[j].c1;
                    min_r = r;
                }
                r = Geometry::Dist (pstart, segment[j].c2);
                if (r > max_r) {
                    cutline.c1 = segment[j].c2;
                    max_r = r;
                }
                if (r < min_r) {
                    cutline.c2 = segment[j].c2;
                    min_r = r;
                }
            }
            #if defined(AC_28)
            lines.Get (cIt->key).cut_start = cutline.c2;
            lines.Get (cIt->key).cut_direction = Geometry::SectorVector (cutline);
            #else
            lines.Get (*cIt->key).cut_start = cutline.c2;
            lines.Get (*cIt->key).cut_direction = Geometry::SectorVector (cutline);
            #endif
        }
    }
    bool hasData = false;
    ConstProfileVectorImageIterator profileDescriptionIt1 (profileDescription);
    Point2D startp = { -10000, 0 };
    while (!profileDescriptionIt1.IsEOI ()) {
        switch (profileDescriptionIt1->item_Typ) {
            case SyHatch:
                {
                    const HatchObject& syHatch = profileDescriptionIt1;
                    short structype = 0;
                    #if defined(AC_27) || defined(AC_28)
                    const ProfileItem profileItemInfo = syHatch.GetProfileItem ();
                    if (profileItemInfo.IsFinish ()) structype = APICWallComp_Finish;
                    if (profileItemInfo.IsCore ()) structype = APICWallComp_Core;
                    #else
                    const ProfileItem* profileItemInfo = syHatch.GetProfileItemPtr ();
                    if (profileItemInfo->IsFinish ()) structype = APICWallComp_Finish;
                    if (profileItemInfo->IsCore ()) structype = APICWallComp_Core;
                    #endif
                    Geometry::MultiPolygon2D result;

                    // Получаем полигон штриховки
                    if (syHatch.ToPolygon2D (result, HatchObject::VertexAndEdgeData::Omit) == NoError) {
                        #if defined(AC_27) || defined(AC_28)
                        #if defined(AC_28)
                        API_AttributeIndex	constrinxL = ACAPI_CreateAttributeIndex ((GS::Int32) syHatch.GetBuildMatIdx ().ToGSAttributeIndex_Deprecated ());
                        #else
                        API_AttributeIndex	constrinxL = ACAPI_CreateAttributeIndex ((GS::Int32) syHatch.GetBuildMatIdx ().ToGSAttributeIndex ());
                        #endif
                        #else
                        API_AttributeIndex	constrinxL = (API_AttributeIndex) syHatch.GetBuildMatIdx ();
                        #endif
                        // Проходим по полигонам
                        for (UInt32 i = 0; i < result.GetSize (); i++) {
                            // Запись всех слоёв
                            double area_fill = result[i].CalcArea ();
                            Point2D centr = result[i].GetCenter ();
                            if (needReadQuantities) {
                                Coord pbegC, pendC; double pdx = 0; double pdy = 0; double plen = 0; double th = 0;
                                for (UIndex edgeIndex = 1; edgeIndex <= result[i].GetEdgeNum (); ++edgeIndex) {
                                    typename Geometry::Polygon2D::ConstEdgeIterator edgeIt = result[i].GetEdgeIterator (edgeIndex);
                                    Coord begC, endC;
                                    double _a = 0.0;
                                    result[i].GetSector (edgeIt, begC, endC, _a);
                                    double dx = endC.x - begC.x;
                                    double dy = endC.y - begC.y;
                                    double len = std::sqrt (dx * dx + dy * dy);
                                    if (edgeIndex == 1) {
                                        pdx = dx; pdy = dy; plen = len;
                                        pbegC = begC, pendC = endC;
                                        continue;
                                    }
                                    if (!(begC == pendC || begC == pbegC || endC == pbegC || endC == pendC) || is_equal (len, 0)) {
                                        pdx = dx; pdy = dy; plen = len;
                                        pbegC = begC, pendC = endC;
                                        continue;
                                    }
                                    double dot = dx * pdx + dy * pdy;
                                    double cosAngle = std::abs (dot) / (len * plen);
                                    if (cosAngle > 1.0) cosAngle = 1.0;
                                    double angle = RADDEG * std::acos (cosAngle);
                                    if (is_equal (angle, 90.0) || fabs (angle - 90.0) < 0.0001) {
                                        if (is_equal (th, 0)) {
                                            th = plen;
                                        } else {
                                            th = fmin (th, plen);
                                        }
                                    }
                                    if (is_equal (angle, 0)) {
                                        if (pendC == endC) {
                                            int hh = 1;
                                        }
                                        if (pbegC == begC) {
                                            int hh = 1;
                                        }
                                        if (pendC == begC) {
                                            begC = pbegC;
                                            int hh = 1;
                                        }
                                        if (pbegC == endC) {
                                            endC = pendC;
                                            int hh = 1;
                                        }
                                        dx = endC.x - begC.x;
                                        dy = endC.y - begC.y;
                                        len = plen + len;
                                    }
                                    pdx = dx; pdy = dy; plen = len;
                                    pbegC = begC, pendC = endC;
                                }
                                ParamValueComposite layer = {};
                                layer.inx = constrinxL;
                                layer.fillThick = th;
                                layer.rfromstart = Geometry::Dist (startp, centr);
                                layer.area_fill = area_fill;
                                layer.structype = structype;
                                if (!existsmaterial.ContainsKey (constrinxL)) {
                                    ParamHelpers::GetAttributeValues (constrinxL, params, paramsAdd);
                                    existsmaterial.Add (constrinxL, true);
                                }
                                composite_all.Push (layer);
                            }
                            // Находим пересечения каждого полигона с линиями
                            for (GS::HashTable<short, OrientedSegments>::PairIterator cIt = lines.EnumeratePairs (); cIt != NULL; ++cIt) {
                                #if defined(AC_28)
                                OrientedSegments& l = cIt->value;
                                short pen = cIt->key;
                                #else
                                OrientedSegments& l = *cIt->value;
                                short pen = *cIt->key;
                                #endif
                                GS::Array<Sector> resSectors;
                                if (pen < 0) continue;
                                bool h = result[i].Intersect (l.cut_start, l.cut_direction, &resSectors);
                                if (!resSectors.IsEmpty ()) {
                                    for (UInt32 k = 0; k < resSectors.GetSize (); k++) {
                                        double fillThickL = resSectors[k].GetLength ();
                                        double rfromstart = Geometry::Dist (l.start, resSectors[k].GetMidPoint ()); // Расстояние до окружности(начала порядка слоёв)
                                        ParamValueComposite layer = {};
                                        layer.inx = constrinxL;
                                        layer.fillThick = fillThickL;
                                        layer.rfromstart = rfromstart;
                                        layer.area_fill = area_fill;
                                        layer.structype = structype;
                                        param_composite.Get (pen).composite.Push (layer);
                                        if (!existsmaterial.ContainsKey (constrinxL)) {
                                            ParamHelpers::GetAttributeValues (constrinxL, params, paramsAdd);
                                            existsmaterial.Add (constrinxL, true);
                                        }
                                        hasData = true;
                                    }
                                }
                            }
                        }
                    } else {
                        #if defined(TESTING)
                        DBprnt ("ERR == syHatch.ToPolygon2D ====================");
                        #endif
                    }
                }
                break;
        }
        ++profileDescriptionIt1;
    }
    if (needReadQuantities && !composite_all.IsEmpty ()) {
        short pen = -1;
        if (param_composite.ContainsKey (pen)) {
            param_composite.Get (pen).composite = composite_all;
            hasData = true;
        }
        pen = -2;
        if (param_composite.ContainsKey (pen)) {
            param_composite.Get (pen).composite = composite_all;
            hasData = true;
        }
    }
    if (hasData) {
        for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = paramlayers.EnumeratePairs (); cIt != NULL; ++cIt) {
            #if defined(AC_28)
            short pen = paramlayers.Get (cIt->key).composite_pen;
            #else
            short pen = paramlayers.Get (*cIt->key).composite_pen;
            #endif
            if (param_composite.ContainsKey (pen)) {
                if (pen < 0) {
                    #if defined(AC_28)
                    paramlayers.Get (cIt->key).composite = param_composite.Get (pen).composite;
                    #else
                    paramlayers.Get (*cIt->key).composite = param_composite.Get (pen).composite;
                    #endif
                } else {
                    // Теперь нам надо отсортировать слои по параметру rfromstart
                    std::map<double, ParamValueComposite> comps;
                    GS::Array<ParamValueComposite> param = param_composite.Get (pen).composite;
                    for (UInt32 i = 0; i < param.GetSize (); i++) {
                        ParamValueComposite comp = param.Get (i);
                        comps[comp.rfromstart] = comp;
                    }
                    GS::Array<ParamValueComposite> paramout;
                    for (std::map<double, ParamValueComposite>::iterator k = comps.begin (); k != comps.end (); ++k) {
                        paramout.Push (k->second);
                    }
                    for (UInt32 i = 0; i < paramout.GetSize (); i++) {
                        paramout[i].num = i + 1;
                    }
                    #if defined(AC_28)
                    paramlayers.Get (cIt->key).composite = paramout;
                    #else
                    paramlayers.Get (*cIt->key).composite = paramout;
                    #endif
                }
            }
        }
        ParamHelpers::CompareParamDictValue (paramlayers, params);
    }
    return hasData;
    #else
    return false;
    #endif
}

// --------------------------------------------------------------------
// Вытаскивает всё, что может, из информации о составе элемента
// --------------------------------------------------------------------
bool ParamHelpers::Components (const API_Element & element, ParamDictValue & params, ParamDictValue & paramsAdd, GS::HashTable<API_AttributeIndex, bool>&existsmaterial)
{
    #if defined(TESTING)
    DBprnt ("        Components");
    #endif
    API_ModelElemStructureType structtype = API_BasicStructure;
    API_AttributeIndex constrinx = {};
    double fillThick = 0;

    // Отделка колонн
    API_AttributeIndex constrinx_ven = {};
    double fillThick_ven = 0; short structype_ven = 0;
    API_Elem_Head elemhead = element.header;
    // Получаем данные о составе конструкции. Т.к. для разных типов элементов
    // информация храница в разных местах - запишем всё в одни переменные
    API_ElemTypeID eltype = GetElemTypeID (elemhead);
    API_ElementMemo	memo = {}; BNZeroMemory (&memo, sizeof (API_ElementMemo));
    switch (eltype) {
        #ifndef AC_22
        case API_ColumnSegmentID:
            structtype = element.columnSegment.assemblySegmentData.modelElemStructureType;
            if (structtype == API_BasicStructure) {
                constrinx = element.columnSegment.assemblySegmentData.buildingMaterial;
                fillThick = element.columnSegment.assemblySegmentData.nominalHeight;
                constrinx_ven = element.columnSegment.venBuildingMaterial;
                fillThick_ven = element.columnSegment.venThick;
                if (element.columnSegment.venType == APIVeneer_Core) structype_ven = APICWallComp_Core;
                if (element.columnSegment.venType == APIVeneer_Finish) structype_ven = APICWallComp_Finish;
            }
            if (structtype == API_ProfileStructure) constrinx = element.columnSegment.assemblySegmentData.profileAttr;
            break;
        case API_BeamSegmentID:
            structtype = element.beamSegment.assemblySegmentData.modelElemStructureType;
            if (structtype == API_BasicStructure) {
                constrinx = element.beamSegment.assemblySegmentData.buildingMaterial;
                fillThick = element.beamSegment.assemblySegmentData.nominalHeight;
            }
            if (structtype == API_ProfileStructure) constrinx = element.beamSegment.assemblySegmentData.profileAttr;
            break;
            #endif
        case API_ColumnID:
            #ifdef AC_22
            constrinx = element.column.buildingMaterial;
            #else
            if (element.header.guid == APINULLGuid) {
                return false;
            }
            if (element.column.nSegments == 1) {
                BNZeroMemory (&memo, sizeof (API_ElementMemo));
                GSErrCode err = ACAPI_Element_GetMemo (element.header.guid, &memo, APIMemoMask_ColumnSegment);
                if (err == NoError && memo.columnSegments != nullptr) {
                    elemhead = memo.columnSegments[0].head;
                    structtype = memo.columnSegments[0].assemblySegmentData.modelElemStructureType;
                    if (structtype == API_BasicStructure) {
                        constrinx = memo.columnSegments[0].assemblySegmentData.buildingMaterial;
                        fillThick = memo.columnSegments[0].assemblySegmentData.nominalHeight;
                        constrinx_ven = memo.columnSegments[0].venBuildingMaterial;
                        fillThick_ven = memo.columnSegments[0].venThick;
                        if (memo.columnSegments[0].venType == APIVeneer_Core) structype_ven = APICWallComp_Core;
                        if (memo.columnSegments[0].venType == APIVeneer_Finish) structype_ven = APICWallComp_Finish;
                    }
                    if (structtype == API_ProfileStructure) constrinx = memo.columnSegments[0].assemblySegmentData.profileAttr;
                } else {
                    msg_rep ("ParamHelpers::Components", "ACAPI_Element_GetMemo - ColumnSegment", err, element.header.guid);
                    ACAPI_DisposeElemMemoHdls (&memo);
                    return false;
                }
            } else {
                msg_rep ("ParamHelpers::Components", "Multisegment column not supported", NoError, element.header.guid);
                return false;
            }
            #endif
            break;
        case API_BeamID:
            #ifdef AC_22
            constrinx = element.beam.buildingMaterial;
            #else
            if (element.header.guid == APINULLGuid) {
                return false;
            }
            if (element.beam.nSegments == 1) {
                BNZeroMemory (&memo, sizeof (API_ElementMemo));
                GSErrCode err = ACAPI_Element_GetMemo (element.header.guid, &memo, APIMemoMask_BeamSegment);
                if (err == NoError && memo.beamSegments != nullptr) {
                    elemhead = memo.beamSegments[0].head;
                    structtype = memo.beamSegments[0].assemblySegmentData.modelElemStructureType;
                    if (structtype == API_BasicStructure) {
                        constrinx = memo.beamSegments[0].assemblySegmentData.buildingMaterial;
                        fillThick = memo.beamSegments[0].assemblySegmentData.nominalHeight;
                    }
                    if (structtype == API_ProfileStructure) constrinx = memo.beamSegments[0].assemblySegmentData.profileAttr;
                } else {
                    msg_rep ("ParamHelpers::Components", "ACAPI_Element_GetMemo - BeamSegment", err, element.header.guid);
                    ACAPI_DisposeElemMemoHdls (&memo);
                    return false;
                }
            } else {
                msg_rep ("ParamHelpers::Components", "Multisegment beam not supported", NoError, element.header.guid);
                return false;
            }
            #endif
            break;
        case API_WallID:
            structtype = element.wall.modelElemStructureType;
            if (structtype == API_CompositeStructure) constrinx = element.wall.composite;
            if (structtype == API_BasicStructure) constrinx = element.wall.buildingMaterial;
            if (structtype == API_ProfileStructure) constrinx = element.wall.profileAttr;
            fillThick = element.wall.thickness;
            break;
        case API_SlabID:
            structtype = element.slab.modelElemStructureType;
            if (structtype == API_CompositeStructure) constrinx = element.slab.composite;
            if (structtype == API_BasicStructure) constrinx = element.slab.buildingMaterial;
            fillThick = element.slab.thickness;
            break;
        case API_RoofID:
            structtype = element.roof.shellBase.modelElemStructureType;
            if (structtype == API_CompositeStructure) constrinx = element.roof.shellBase.composite;
            if (structtype == API_BasicStructure) constrinx = element.roof.shellBase.buildingMaterial;
            fillThick = element.roof.shellBase.thickness;
            break;
        case API_ShellID:
            structtype = element.shell.shellBase.modelElemStructureType;
            if (structtype == API_CompositeStructure) constrinx = element.shell.shellBase.composite;
            if (structtype == API_BasicStructure) constrinx = element.shell.shellBase.buildingMaterial;
            fillThick = element.shell.shellBase.thickness;
            break;
        default:
            return false;
            break;
    }
    ACAPI_DisposeElemMemoHdls (&memo);

    // Типов вывода слоёв может быть насколько - для сложных профилей, для учёта несущих/ненесущих слоёв
    // Получим словарь исключительно с определениями состава
    ParamDictValue paramlayers = {};;
    for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = params.EnumeratePairs (); cIt != NULL; ++cIt) {
        #if defined(AC_28)
        ParamValue& param = cIt->value;
        #else
        ParamValue& param = *cIt->value;
        #endif
        if (param.fromMaterial) {
            if (param.rawName.Contains ("{@material:layers")) {
                param.composite_type = structtype;
                paramlayers.Add (param.rawName, param);
            }
        }
    }

    // Если ничего нет - слои нам всё равно нужны
    if (paramlayers.IsEmpty ()) {
        ParamValue param_composite = {};
        param_composite.fromGuid = element.header.guid;
        param_composite.isValid = true;
        param_composite.composite_pen = 20;
        param_composite.composite_type = structtype;
        param_composite.eltype = eltype;
        paramlayers.Add ("{@material:layers,20}", param_composite);
    }

    bool hasData = false;

    // Получим индексы строительных материалов и толщины
    if (structtype == API_BasicStructure) {
        hasData = ParamHelpers::ComponentsBasicStructure (constrinx, fillThick, constrinx_ven, fillThick_ven, params, paramlayers, paramsAdd, structype_ven);
    }
    if (structtype == API_CompositeStructure) hasData = ParamHelpers::ComponentsCompositeStructure (elemhead.guid, constrinx, params, paramlayers, paramsAdd, existsmaterial);
    #ifndef AC_23
    if (structtype == API_ProfileStructure) {
        API_ElementMemo	memo = {}; BNZeroMemory (&memo, sizeof (API_ElementMemo));
        UInt64 mask = APIMemoMask_StretchedProfile;
        GSErrCode err = ACAPI_Element_GetMemo (elemhead.guid, &memo, mask);
        if (err != NoError) {
            ACAPI_DisposeElemMemoHdls (&memo);
            msg_rep ("ParamHelpers::Components", "err", err, element.header.guid);
            return false;
        }
        if (memo.stretchedProfile == nullptr) {
            ACAPI_DisposeElemMemoHdls (&memo);
            msg_rep ("ParamHelpers::Components", "Profile not found, missing attribute", NoError, element.header.guid);
            return false;
        }
        ProfileVectorImage profileDescription = *memo.stretchedProfile;
        hasData = ParamHelpers::ComponentsProfileStructure (profileDescription, params, paramlayers, paramsAdd, existsmaterial);
        ACAPI_DisposeElemMemoHdls (&memo);
    }
    #endif
    return hasData;
}

// --------------------------------------------------------------------
// Заполнение данных для одного слоя
// --------------------------------------------------------------------
bool ParamHelpers::GetAttributeValues (const API_AttributeIndex & constrinx, ParamDictValue & params, ParamDictValue & paramsAdd)
{
    API_Attribute	attrib = {};
    GS::UniString name = "";
    BNZeroMemory (&attrib, sizeof (API_Attribute));
    attrib.header.typeID = API_BuildingMaterialID;
    attrib.header.index = constrinx;
    attrib.header.uniStringNamePtr = &name;
    GS::UniString attribsuffix = GS::UniString::Printf ("%d", constrinx);
    GSErrCode error = ACAPI_Attribute_Get (&attrib);
    if (error != NoError) {
        msg_rep ("materialString::GetAttributeValues", "ACAPI_Attribute_Get", error, APINULLGuid);
        return false;
    };

    if (params.ContainsKey ("{@material:cutfill_inx}")) {
        ParamValue pvalue_bmat = {};
        pvalue_bmat.rawName = "{@material:cutfill_inx}";
        pvalue_bmat.name = "cutfill_inx";
        pvalue_bmat.rawName.ReplaceAll ("}", CharENTER + attribsuffix + "}");
        #if defined(AC_27) || defined(AC_28)
        ParamHelpers::ConvertIntToParamValue (pvalue_bmat, pvalue_bmat.name, constrinx.ToInt32_Deprecated ());
        #else
        ParamHelpers::ConvertIntToParamValue (pvalue_bmat, pvalue_bmat.name, (Int32) constrinx);
        #endif
        pvalue_bmat.fromMaterial = true;
        ParamHelpers::AddParamValue2ParamDict (params.Get ("{@material:cutfill_inx}").fromGuid, pvalue_bmat, params);
    }

    if (params.ContainsKey ("{@material:bmat_inx}")) {
        ParamValue pvalue_bmat = {};
        pvalue_bmat.rawName = "{@material:bmat_inx}";
        pvalue_bmat.name = "bmat_inx";
        pvalue_bmat.rawName.ReplaceAll ("}", CharENTER + attribsuffix + "}");
        #if defined(AC_27) || defined(AC_28)
        ParamHelpers::ConvertIntToParamValue (pvalue_bmat, pvalue_bmat.name, constrinx.ToInt32_Deprecated ());
        #else
        ParamHelpers::ConvertIntToParamValue (pvalue_bmat, pvalue_bmat.name, (Int32) constrinx);
        #endif
        pvalue_bmat.fromMaterial = true;
        ParamHelpers::AddParamValue2ParamDict (params.Get ("{@material:bmat_inx}").fromGuid, pvalue_bmat, params);
    }

    // Определения и свойста для элементов
    bool flag_find = false;
    GS::Array<API_PropertyDefinition> propertyDefinitions;
    for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = params.EnumeratePairs (); cIt != NULL; ++cIt) {
        #if defined(AC_28)
        ParamValue& param = cIt->value;
        #else
        ParamValue& param = *cIt->value;
        #endif
        if (param.fromAttribDefinition) {
            bool flag_add = true;

            // Если в списке есть штриховка или покрытие - получим их имена.
            if (param.rawName.Contains ("buildingmaterialproperties/building material cutfill")) {
                GS::UniString namet = "";
                API_Attribute	attribt = {};
                BNZeroMemory (&attribt, sizeof (API_Attribute));
                attribt.header.typeID = API_FilltypeID;
                attribt.header.index = attrib.buildingMaterial.cutFill;
                attribt.header.uniStringNamePtr = &namet;
                error = ACAPI_Attribute_Get (&attribt);
                ParamValue pvalue = {};
                GS::UniString rawName = param.rawName;
                rawName.ReplaceAll ("}", CharENTER + attribsuffix + "}");
                pvalue.name = param.name + CharENTER + attribsuffix;
                pvalue.rawName = rawName;
                ParamHelpers::ConvertStringToParamValue (pvalue, pvalue.name, namet);
                pvalue.fromMaterial = true;
                ParamHelpers::AddParamValue2ParamDict (param.fromGuid, pvalue, params);
                flag_add = false;
                flag_find = true;
            }
            if (flag_add) {
                API_PropertyDefinition definition = param.definition;
                if (!definition.name.Contains (CharENTER) && definition.guid != APINULLGuid) propertyDefinitions.Push (definition);
            }
        }
    }
    #ifndef AC_22
    if (!propertyDefinitions.IsEmpty ()) {
        GS::Array<API_Property> properties;
        error = ACAPI_Attribute_GetPropertyValues (attrib.header, propertyDefinitions, properties);
        if (error != NoError) {
            msg_rep ("materialString::GetAttributeValues", "ACAPI_Attribute_GetPropertyValues", error, APINULLGuid);
            return flag_find;
        };
        for (UInt32 i = 0; i < properties.GetSize (); i++) {
            properties[i].definition.name = properties[i].definition.name + CharENTER + attribsuffix;
            GS::UniString val = PropertyHelpers::ToString (properties[i]);
            if (val.Count ("%") > 1 || (val.Contains ("{") && val.Contains ("}"))) {
                if (ParamHelpers::ParseParamNameMaterial (val, paramsAdd)) {
                    properties[i].value.singleVariant.variant.uniStringValue = val;
                    properties[i].isDefault = false;
                }
            }
        }
        return (ParamHelpers::AddProperty (params, properties) || flag_find);
    }
    #endif
    return flag_find;
}
