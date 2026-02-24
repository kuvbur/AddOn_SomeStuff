//------------ kuvbur 2022 ------------
#include	"ACAPinc.h"
#include	"APIEnvir.h"
#include	"Dimensions.hpp"
#include	"MEPv1.hpp"
#include	"Propertycache.hpp"
#include	"ResetProperty.hpp"
#include	"Sync.hpp"
#include	<stdlib.h> /* atoi */
#include	<time.h>
#ifdef TESTING
#include "TestFunc.hpp"
#endif
#if defined (AC_28)
#include <ACAPI/MEPAdapter.hpp>
#include	"Propertycache.hpp"
#endif
Int32 nLib = 0;
// -----------------------------------------------------------------------------
// Подключение мониторинга
// -----------------------------------------------------------------------------
void MonAll (SyncSettings& syncSettings)
{
    if (!syncSettings.syncMon) return;
    #if defined(TESTING)
    DBprnt ("MonAll start");
    #endif
    clock_t start, finish;
    double  duration = 0;
    start = clock ();
    MonByType (API_ObjectID, syncSettings);
    MonByType (API_WindowID, syncSettings);
    MonByType (API_DoorID, syncSettings);
    MonByType (API_ZoneID, syncSettings);
    MonByType (API_WallID, syncSettings);
    MonByType (API_SlabID, syncSettings);
    MonByType (API_ColumnID, syncSettings);
    MonByType (API_BeamID, syncSettings);
    MonByType (API_RoofID, syncSettings);
    MonByType (API_MeshID, syncSettings);
    MonByType (API_MorphID, syncSettings);
    MonByType (API_CurtainWallID, syncSettings);
    MonByType (API_RailingID, syncSettings);
    #if defined (AC_28)
    MonByType (API_ExternalElemID, syncSettings);
    #endif
    if (PROPERTYCACHE ().hasDimAutotext) MonByType (API_DimensionID, syncSettings);
    finish = clock ();
    duration = (double) (finish - start) / CLOCKS_PER_SEC;
    GS::UniString time = GS::UniString::Printf (" %.3f s", duration);
    msg_rep ("MonAll", time, NoError, APINULLGuid);
    #if defined(TESTING)
    DBprnt ("MonAll end");
    #endif
}

// -----------------------------------------------------------------------------
// Подключение мониторинга по типам
// -----------------------------------------------------------------------------
void MonByType (const API_ElemTypeID& elementType, const SyncSettings& syncSettings)
{
    GS::Array<API_Guid>	guidArray;
    #if defined(TESTING)
    DBprnt ("MonByType");
    #endif
    GSErrCode err = ACAPI_Element_GetElemList (elementType, &guidArray, APIFilt_IsEditable | APIFilt_HasAccessRight | APIFilt_InMyWorkspace);
    if (err != NoError || guidArray.IsEmpty ()) return;
    for (UInt32 i = 0; i < guidArray.GetSize (); i++) {
        err = AttachObserver (guidArray[i], syncSettings);
        if (err == APIERR_LINKEXIST)
            err = NoError;
        if (err != NoError) {
            msg_rep ("MonByType", "AttachObserver", err, guidArray[i]);
            return;
        }
        // Получаем список связанных элементов
        if (syncSettings.cwallS) {
            GS::Array<API_Guid> subelemGuids;
            GetRelationsElement (guidArray[i], elementType, syncSettings, subelemGuids, true, true);
            for (UInt32 j = 0; j < subelemGuids.GetSize (); j++) {
                err = AttachObserver (subelemGuids[j], syncSettings);
                if (err == APIERR_LINKEXIST)
                    err = NoError;
                if (err != NoError) {
                    msg_rep ("MonByType", "AttachObserver", err, subelemGuids[j]);
                    return;
                }
            }
        }
    }
}

// -----------------------------------------------------------------------------
// Запускает обработку всех объектов, заданных в настройке
// -----------------------------------------------------------------------------
void SyncAndMonAll (SyncSettings& syncSettings)
{
    #if defined(TESTING)
    DBprnt ("SyncAndMonAll start");
    #endif
    if (ResetProperty ()) return;
    GS::UniString funcname ("Sync All");
    bool flag_chanel = false;
    ParamDictElement paramToWrite = {};
    GS::Int32 nPhase = 1;
    #if defined(AC_27) || defined(AC_28) || defined(AC_29)
    ACAPI_ProcessWindow_InitProcessWindow (&funcname, &nPhase);
    #else
    ACAPI_Interface (APIIo_InitProcessWindowID, &funcname, &nPhase);
    #endif
    clock_t start, finish;
    double  duration = 0;
    start = clock ();
    int dummymode = IsDummyModeOn ();
    UnicGuid syncedelem = {};
    if (!flag_chanel && syncSettings.objS) flag_chanel = SyncByType (API_ObjectID, syncSettings, nPhase, paramToWrite, dummymode, syncedelem);
    nPhase = nPhase + 1;
    if (!flag_chanel && syncSettings.objS) flag_chanel = SyncByType (API_LampID, syncSettings, nPhase, paramToWrite, dummymode, syncedelem);
    nPhase = nPhase + 1;
    if (!flag_chanel && syncSettings.widoS) flag_chanel = SyncByType (API_WindowID, syncSettings, nPhase, paramToWrite, dummymode, syncedelem);
    nPhase = nPhase + 1;
    if (!flag_chanel && syncSettings.widoS) flag_chanel = SyncByType (API_DoorID, syncSettings, nPhase, paramToWrite, dummymode, syncedelem);
    nPhase = nPhase + 1;
    if (!flag_chanel && syncSettings.widoS) flag_chanel = SyncByType (API_SkylightID, syncSettings, nPhase, paramToWrite, dummymode, syncedelem);
    nPhase = nPhase + 1;
    if (!flag_chanel && syncSettings.objS) flag_chanel = SyncByType (API_ZoneID, syncSettings, nPhase, paramToWrite, dummymode, syncedelem);
    nPhase = nPhase + 1;
    if (!flag_chanel && syncSettings.wallS) flag_chanel = SyncByType (API_WallID, syncSettings, nPhase, paramToWrite, dummymode, syncedelem);
    nPhase = nPhase + 1;
    if (!flag_chanel && syncSettings.wallS) flag_chanel = SyncByType (API_SlabID, syncSettings, nPhase, paramToWrite, dummymode, syncedelem);
    nPhase = nPhase + 1;
    if (!flag_chanel && syncSettings.wallS) flag_chanel = SyncByType (API_ColumnID, syncSettings, nPhase, paramToWrite, dummymode, syncedelem);
    nPhase = nPhase + 1;
    if (!flag_chanel && syncSettings.wallS) flag_chanel = SyncByType (API_BeamID, syncSettings, nPhase, paramToWrite, dummymode, syncedelem);
    nPhase = nPhase + 1;
    if (!flag_chanel && syncSettings.wallS) flag_chanel = SyncByType (API_RoofID, syncSettings, nPhase, paramToWrite, dummymode, syncedelem);
    nPhase = nPhase + 1;
    if (!flag_chanel && syncSettings.wallS) flag_chanel = SyncByType (API_MeshID, syncSettings, nPhase, paramToWrite, dummymode, syncedelem);
    nPhase = nPhase + 1;
    if (!flag_chanel && syncSettings.wallS) flag_chanel = SyncByType (API_MorphID, syncSettings, nPhase, paramToWrite, dummymode, syncedelem);
    nPhase = nPhase + 1;

    if (!flag_chanel && syncSettings.wallS) flag_chanel = SyncByType (API_ShellID, syncSettings, nPhase, paramToWrite, dummymode, syncedelem);
    nPhase = nPhase + 1;

    if (!flag_chanel && syncSettings.cwallS) flag_chanel = SyncByType (API_CurtainWallID, syncSettings, nPhase, paramToWrite, dummymode, syncedelem);
    nPhase = nPhase + 1;
    if (!flag_chanel && syncSettings.cwallS) flag_chanel = SyncByType (API_RailingID, syncSettings, nPhase, paramToWrite, dummymode, syncedelem);
    #if defined (AC_28)
    if (!flag_chanel && syncSettings.objS) flag_chanel = SyncByType (API_ExternalElemID, syncSettings, nPhase, paramToWrite, dummymode, syncedelem);
    #endif
    finish = clock ();
    duration = (double) (finish - start) / CLOCKS_PER_SEC;
    GS::UniString time = GS::UniString::Printf (" %.3f s", duration);
    msg_rep ("SyncAll - read", time, NoError, APINULLGuid);
    GS::Array<API_Guid> rereadelem = {};
    start = clock ();
    if (!paramToWrite.IsEmpty ()) {
        GS::UniString undoString = RSGetIndString (ID_ADDON_STRINGS + isEng (), UndoSyncId, ACAPI_GetOwnResModule ());
        ACAPI_CallUndoableCommand (undoString, [&]() -> GSErrCode {
            GS::UniString title = GS::UniString::Printf ("Writing data to %d elements : ", paramToWrite.GetSize ()); short i = 1;
            #if defined(AC_27) || defined(AC_28) || defined(AC_29)
            bool showPercent = false;
            Int32 maxval = 2;
            ACAPI_ProcessWindow_SetNextProcessPhase (&title, &maxval, &showPercent);
            #else
            ACAPI_Interface (APIIo_SetNextProcessPhaseID, &title, &i);
            #endif
            bool suspGrp = false;
            #ifndef AC_22
            #if defined(AC_27) || defined(AC_28) || defined(AC_29)
            ACAPI_View_IsSuspendGroupOn (&suspGrp);
            if (!suspGrp) ACAPI_Grouping_Tool (rereadelem, APITool_SuspendGroups, nullptr);
            #else
            ACAPI_Environment (APIEnv_IsSuspendGroupOnID, &suspGrp);
            if (!suspGrp) ACAPI_Element_Tool (rereadelem, APITool_SuspendGroups, nullptr);
            #endif
            #endif
            rereadelem = ParamHelpers::ElementsWrite (paramToWrite);
            finish = clock ();
            duration = (double) (finish - start) / CLOCKS_PER_SEC;
            GS::UniString time = title + GS::UniString::Printf (" %.3f s", duration);
            msg_rep ("SyncAll - write", time, NoError, APINULLGuid);
            return NoError;
        });
    } else {
        msg_rep ("SyncAll - write", "No data to write", NoError, APINULLGuid);
    }
    ParamHelpers::InfoWrite (paramToWrite);
    if (!rereadelem.IsEmpty ()) {
        #if defined(TESTING)
        DBprnt ("===== REREAD =======");
        #endif
        SyncArray (syncSettings, rereadelem);
    }
    #if defined(TESTING)
    DBprnt ("SyncAndMonAll end");
    #endif
    #if defined(AC_27) || defined(AC_28) || defined(AC_29)
    ACAPI_ProcessWindow_CloseProcessWindow ();
    #else
    ACAPI_Interface (APIIo_CloseProcessWindowID, nullptr, nullptr);
    #endif
}

// -----------------------------------------------------------------------------
// Синхронизация элементов по типу
// -----------------------------------------------------------------------------
bool SyncByType (const API_ElemTypeID& elementType, const SyncSettings& syncSettings, GS::Int32& nPhase, ParamDictElement& paramToWrite, int dummymode, UnicGuid& syncedelem)
{
    #if defined(TESTING)
    DBprnt ("    SyncByType start");
    #endif
    GS::UniString subtitle = "";
    GSErrCode err = NoError;
    GS::Array<API_Guid>	guidArray = {};
    clock_t start, finish;
    double  duration = 0;
    start = clock ();
    ACAPI_Element_GetElemList (elementType, &guidArray, APIFilt_IsEditable | APIFilt_HasAccessRight | APIFilt_InMyWorkspace);
    #if defined (AC_28)
    if (elementType == API_ExternalElemID) {
        guidArray = ACAPI::MEP::CollectAllMEPElements ();
    }
    #endif
    if (guidArray.IsEmpty ()) return false;
    //#ifdef TESTING
    //TestFunc::ResetSyncPropertyArray (guidArray);
    //#endif
    #if defined(AC_26) || defined(AC_27) || defined(AC_28) || defined(AC_29)
    API_ElemType elemType;
    elemType.typeID = elementType;
    #if defined(AC_27) || defined(AC_28) || defined(AC_29)
    bool showPercent = true;
    Int32 maxval = guidArray.GetSize ();
    ACAPI_Element_GetElemTypeName (elemType, subtitle);
    #else
    ACAPI_Goodies_GetElemTypeName (elemType, subtitle);
    #endif
    #else
    ACAPI_Goodies (APIAny_GetElemTypeNameID, (void*) elementType, &subtitle);
    #endif // AC_26
    GS::UniString subtitle_ = GS::UniString::Printf ("Reading data from %d elements : ", guidArray.GetSize ()) + subtitle;
    bool flag_chanel = false;
    for (UInt32 i = 0; i < guidArray.GetSize (); i++) {
        SyncElement (guidArray[i], syncSettings, paramToWrite, dummymode, syncedelem);
        #if defined(AC_27) || defined(AC_28) || defined(AC_29)
        if (i % 10 == 0) ACAPI_ProcessWindow_SetNextProcessPhase (&subtitle, &maxval, &showPercent);
        #else
        if (i % 10 == 0) ACAPI_Interface (APIIo_SetNextProcessPhaseID, &subtitle, &i);
        #endif
        #if defined(AC_27) || defined(AC_28) || defined(AC_29)
        if (ACAPI_ProcessWindow_IsProcessCanceled ()) return true;
        #else
        if (ACAPI_Interface (APIIo_IsProcessCanceledID, nullptr, nullptr)) return true;
        #endif
    }
    GS::UniString intString = GS::UniString::Printf (" %d qty", guidArray.GetSize ());
    finish = clock ();
    duration = (double) (finish - start) / CLOCKS_PER_SEC;
    GS::UniString time = GS::UniString::Printf (" %.3f s", duration);
    msg_rep ("SyncByType", subtitle + intString + time, NoError, APINULLGuid);
    return flag_chanel;
    #if defined(TESTING)
    DBprnt ("    SyncByType end");
    #endif
}

// -----------------------------------------------------------------------------
// Синхронизация элемента и его подэлементов
// -----------------------------------------------------------------------------
bool SyncElement (const API_Guid& elemGuid, const SyncSettings& syncSettings, ParamDictElement& paramToWrite, int dummymode)
{
    UnicGuid syncedelem = {};
    return SyncElement (elemGuid, syncSettings, paramToWrite, dummymode, syncedelem);
}

bool SyncElement (const API_Guid& elemGuid, const SyncSettings& syncSettings, ParamDictElement& paramToWrite, int dummymode, UnicGuid& syncedelem)
{
    #if defined(TESTING)
    DBprnt ("      SyncElement start");
    #endif
    API_ElemTypeID elementType;
    GSErrCode err = GetTypeByGUID (elemGuid, elementType);
    if (err != NoError) return false;
    API_Guid elemGuid_ = elemGuid;
    if (elementType == API_SectElemID) {
        GetParentGUIDSectElem (elemGuid, elemGuid_, elementType);
    }
    // Получаем список связанных элементов
    GS::Array<API_Guid> subelemGuids = {};
    GetRelationsElement (elemGuid_, elementType, syncSettings, subelemGuids, true, true);
    bool needResync = false;
    if (!syncedelem.ContainsKey (elemGuid_)) {
        needResync = SyncData (elemGuid_, APINULLGuid, syncSettings, subelemGuids, paramToWrite, dummymode);
        syncedelem.Add (elemGuid_, needResync);
    }
    GS::Array<API_Guid> epm = {};
    if (!subelemGuids.IsEmpty () && SyncRelationsElement (elementType, syncSettings)) {
        for (const auto& subelemGuid : subelemGuids) {
            if (subelemGuid == elemGuid_) continue;
            if (syncedelem.ContainsKey (subelemGuid)) continue;
            if (SyncData (subelemGuid, elemGuid_, syncSettings, epm, paramToWrite, dummymode)) needResync = true;
            syncedelem.Add (subelemGuid, needResync);
        }
    }
    #if defined(TESTING)
    DBprnt ("      SyncElement end");
    #endif
    return needResync;
}


// -----------------------------------------------------------------------------
// Запускает обработку выбранных, заданных в настройке
// -----------------------------------------------------------------------------
void SyncSelected (const SyncSettings& syncSettings)
{
    GS::UniString fmane = "Sync Selected";
    GS::Array<API_Guid> guidArray = GetSelectedElements (false, true, syncSettings, false, false, false);
    if (guidArray.IsEmpty ()) return;
    ClassificationFunc::SystemDict systemdict = {};
    ParamDictValue propertyParams = {};
    GS::Array<API_Guid> rereadelem = SyncArray (syncSettings, guidArray);
    if (!rereadelem.IsEmpty ()) {
        #if defined(TESTING)
        DBprnt ("===== REREAD =======");
        #endif
        SyncArray (syncSettings, rereadelem);
    }
}

// -----------------------------------------------------------------------------
// Запускает обработку переданного массива
// -----------------------------------------------------------------------------
GS::Array<API_Guid> SyncArray (const SyncSettings& syncSettings, GS::Array<API_Guid>& guidArray)
{
    GS::Array<API_Guid> rereadelem = {};
    if (guidArray.IsEmpty ()) return rereadelem;
    GS::UniString funcname = "Sync Selected";
    ParamDictElement paramToWrite = {};
    GS::UniString subtitle = GS::UniString::Printf ("Reading data from %d elements", guidArray.GetSize ());
    GS::Int32 nPhase = 1;
    int dummymode = IsDummyModeOn ();
    #if defined(AC_27) || defined(AC_28) || defined(AC_29)
    bool showPercent = true;
    Int32 maxval = guidArray.GetSize ();
    ACAPI_ProcessWindow_InitProcessWindow (&funcname, &nPhase);
    #else
    ACAPI_Interface (APIIo_InitProcessWindowID, &funcname, &nPhase);
    #endif
    clock_t start, finish;
    double  duration = 0;
    start = clock ();
    bool needResync = false;
    UnicGuid syncedelem = {};
    for (UInt32 i = 0; i < guidArray.GetSize (); i++) {
        if (SyncElement (guidArray[i], syncSettings, paramToWrite, dummymode, syncedelem)) rereadelem.Push (guidArray[i]);
        #if defined(AC_27) || defined(AC_28) || defined(AC_29)
        if (i % 10 == 0) ACAPI_ProcessWindow_SetNextProcessPhase (&subtitle, &maxval, &showPercent);
        #else
        if (i % 10 == 0) ACAPI_Interface (APIIo_SetNextProcessPhaseID, &subtitle, &i);
        #endif
        #if defined(AC_27) || defined(AC_28) || defined(AC_29)
        if (ACAPI_ProcessWindow_IsProcessCanceled ()) return rereadelem;
        #else
        if (ACAPI_Interface (APIIo_IsProcessCanceledID, nullptr, nullptr)) return rereadelem;
        #endif
    }
    GS::UniString intString = GS::UniString::Printf (" %d qty", guidArray.GetSize ());
    finish = clock ();
    duration = (double) (finish - start) / CLOCKS_PER_SEC;
    GS::UniString time = GS::UniString::Printf (" %.3f s", duration);
    msg_rep ("SyncSelected - read", subtitle + intString + time, NoError, APINULLGuid);
    if (!paramToWrite.IsEmpty ()) {
        GS::UniString undoString = RSGetIndString (ID_ADDON_STRINGS + isEng (), UndoSyncId, ACAPI_GetOwnResModule ());
        ACAPI_CallUndoableCommand (undoString, [&]() -> GSErrCode {
            start = clock ();
            GS::UniString title = GS::UniString::Printf ("Writing data to %d elements : ", paramToWrite.GetSize ()); short i = 1;
            #if defined(AC_27) || defined(AC_28) || defined(AC_29)
            ACAPI_ProcessWindow_SetNextProcessPhase (&subtitle, &maxval, &showPercent);
            #else
            ACAPI_Interface (APIIo_SetNextProcessPhaseID, &subtitle, &i);
            #endif
            GS::Array<API_Guid> rereadelem_ = ParamHelpers::ElementsWrite (paramToWrite);
            if (!rereadelem_.IsEmpty ()) rereadelem.Append (rereadelem_);
            finish = clock ();
            duration = (double) (finish - start) / CLOCKS_PER_SEC;
            GS::UniString time = GS::UniString::Printf (" %.3f s", duration);
            msg_rep ("SyncSelected - write", time, NoError, APINULLGuid);
            return NoError;
        });
    } else {
        msg_rep ("SyncSelected - write", "No data to write", NoError, APINULLGuid);
    }
    ParamHelpers::InfoWrite (paramToWrite);
    #if defined(AC_27) || defined(AC_28) || defined(AC_29)
    ACAPI_ProcessWindow_CloseProcessWindow ();
    #else
    ACAPI_Interface (APIIo_CloseProcessWindowID, nullptr, nullptr);
    #endif
    return rereadelem;
}

// -----------------------------------------------------------------------------
// Запуск скрипта параметров выбранных элементов
// -----------------------------------------------------------------------------
void RunParamSelected (const SyncSettings& syncSettings)
{
    clock_t start, finish;
    double duration = 0;
    start = clock ();
    GS::UniString fmane = "Run parameter script";
    // Запомним номер текущей БД и комбинацию слоёв для восстановления по окончанию работы
    API_AttributeIndex layerCombIndex;
    API_DatabaseInfo databaseInfo;
    BNZeroMemory (&databaseInfo, sizeof (API_DatabaseInfo));
    GSErrCode err = NoError;
    #if defined(AC_27) || defined(AC_28) || defined(AC_29)
    err = ACAPI_Navigator_GetCurrLayerComb (&layerCombIndex);
    #else
    err = ACAPI_Environment (APIEnv_GetCurrLayerCombID, &layerCombIndex);
    #endif
    if (err != NoError) {
        msg_rep (fmane, "APIEnv_GetCurrLayerCombID", err, APINULLGuid);
        return;
    }
    #if defined(AC_27) || defined(AC_28) || defined(AC_29)
    err = ACAPI_Database_GetCurrentDatabase (&databaseInfo);
    #else
    err = ACAPI_Database (APIDb_GetCurrentDatabaseID, &databaseInfo, nullptr);
    #endif

    if (err != NoError) {
        msg_rep (fmane, "APIDb_GetCurrentDatabaseID", err, APINULLGuid);
        return;
    }
    CallOnSelectedElemSettings (RunParam, false, true, syncSettings, fmane, false);
    SyncSelected (syncSettings);
    #if defined(AC_27) || defined(AC_28) || defined(AC_29)
    if (layerCombIndex.IsPositive ()) err = ACAPI_Navigator_ChangeCurrLayerComb (&layerCombIndex); // Устанавливаем комбинацию слоёв
    err = ACAPI_Database_ChangeCurrentDatabase (&databaseInfo);
    #else
    if (layerCombIndex != 0) err = ACAPI_Environment (APIEnv_ChangeCurrLayerCombID, &layerCombIndex); // Устанавливаем комбинацию слоёв
    err = ACAPI_Database (APIDb_ChangeCurrentDatabaseID, &databaseInfo, nullptr);
    #endif
    finish = clock ();
    duration = (double) (finish - start) / CLOCKS_PER_SEC;
    GS::UniString time = GS::UniString::Printf (" %.3f s", duration);
    msg_rep (fmane, time, err, APINULLGuid);
}

// -----------------------------------------------------------------------------
// Запуск скрипта параметра элемента
// -----------------------------------------------------------------------------
void RunParam (const API_Guid& elemGuid, const SyncSettings& syncSettings)
{
    #if defined(TESTING)
    DBprnt ("RunParam");
    #endif
    API_Elem_Head tElemHead = {};
    BNZeroMemory (&tElemHead, sizeof (API_Elem_Head));
    tElemHead.guid = elemGuid;
    GSErrCode	err = ACAPI_Element_GetHeader (&tElemHead);
    if (err != NoError) return;
    API_DatabaseInfo databaseInfo;
    API_DatabaseInfo dbInfo;
    #if defined(AC_27) || defined(AC_28) || defined(AC_29)
    err = ACAPI_Database_GetContainingDatabase (&tElemHead.guid, &dbInfo);
    #else
    err = ACAPI_Database (APIDb_GetContainingDatabaseID, &tElemHead.guid, &dbInfo);
    #endif
    if (err != NoError) return;
    #if defined(AC_27) || defined(AC_28) || defined(AC_29)
    err = ACAPI_Database_GetCurrentDatabase (&databaseInfo);
    #else
    err = ACAPI_Database (APIDb_GetCurrentDatabaseID, &databaseInfo, nullptr);
    #endif
    if (err != NoError) return;
    if (dbInfo.databaseUnId != databaseInfo.databaseUnId) {
        #if defined(AC_27) || defined(AC_28) || defined(AC_29)
        err = ACAPI_Database_ChangeCurrentDatabase (&dbInfo);
        #else
        err = ACAPI_Database (APIDb_ChangeCurrentDatabaseID, &dbInfo, nullptr);
        #endif
        if (err != NoError) return;
    }
    API_Element element, mask;
    ACAPI_ELEMENT_MASK_CLEAR (mask);
    ACAPI_ELEMENT_MASK_SET (mask, API_Elem_Head, renovationStatus);
    element.header = tElemHead;
    err = ACAPI_Element_Get (&element);
    if (err != NoError) {
        msg_rep ("RunParam", "APIAny_RunGDLParScriptID", err, elemGuid);
        return;
    }
    #if defined(AC_27) || defined(AC_28) || defined(AC_29)
    err = ACAPI_LibraryManagement_RunGDLParScript (&tElemHead, 0);
    #else
    err = ACAPI_Goodies (APIAny_RunGDLParScriptID, &tElemHead, 0);
    #endif
    if (err != NoError) {
        msg_rep ("RunParam", "APIAny_RunGDLParScriptID", err, elemGuid);
        return;
    }
}

// --------------------------------------------------------------------
// Поиск и синхронизация свойств связанных элементов
// --------------------------------------------------------------------
bool SyncRelationsElement (const API_ElemTypeID& elementType, const SyncSettings& syncSettings)
{
    bool flag_sync = false;
    switch (elementType) {
        case API_WindowID:
        case API_DoorID:
            if (syncSettings.widoS) flag_sync = true;
            break;
        case API_CurtainWallSegmentID:
        case API_CurtainWallFrameID:
        case API_CurtainWallJunctionID:
        case API_CurtainWallAccessoryID:
        case API_CurtainWallPanelID:
        case API_CurtainWallID:
            if (syncSettings.cwallS) flag_sync = true;
            break;
        default:
            if (syncSettings.wallS) flag_sync = true;
            break;
    }
    return flag_sync;
}

// --------------------------------------------------------------------
// Синхронизация данных элемента согласно указаниям в описании свойств
// --------------------------------------------------------------------
bool SyncData (const API_Guid& elemGuid, const API_Guid& rootGuid, const SyncSettings& syncSettings, GS::Array<API_Guid>& subelemGuids, ParamDictElement& paramToWrite, int dummymode)
{
    GSErrCode	err = NoError;
    API_ElemTypeID elementType;
    if (!IsElementEditable (elemGuid, syncSettings, true, elementType)) return false;
    // Если включён мониторинг - привязываем элемент к отслеживанию
    if (syncSettings.syncMon) {
        err = AttachObserver (elemGuid, syncSettings);
        if (err == APIERR_LINKEXIST)
            err = NoError;
        if (err != NoError) {
            msg_rep ("SyncData", "AttachObserver", err, elemGuid);
            return false;
        }
    }
    if (elementType == API_DimensionID) return false;
    ClassificationFunc::SetAutoclass (elemGuid);
    GS::Array<API_PropertyDefinition> definitions = {};
    err = ACAPI_Element_GetPropertyDefinitions (elemGuid, API_PropertyDefinitionFilter_UserDefined, definitions);
    if (err != NoError) {
        msg_rep ("SyncData", "ACAPI_Element_GetPropertyDefinitions", err, elemGuid);
        return false;
    }
    if (definitions.IsEmpty ()) return false;
    // Синхронизация данных
    // Проверяем - не отключена ли синхронизация у данного объекта
    if (dummymode == DUMMY_MODE_UNDEF) dummymode = IsDummyModeOn ();
    bool syncall = true; bool flagfindall = true;
    bool synccoord = true; bool flagfindcoord = true;
    bool syncclass = true; bool flagfindclass = true;
    if (dummymode == DUMMY_MODE_ON) {
        syncall = GetElemStateReverse (elemGuid, definitions, "Sync_flag", flagfindall);
        synccoord = GetElemStateReverse (elemGuid, definitions, "Sync_correct_flag", flagfindcoord);
        syncclass = GetElemStateReverse (elemGuid, definitions, "Sync_class_flag", flagfindclass);
    } else {
        syncall = GetElemState (elemGuid, definitions, "Sync_flag", flagfindall, false);
        synccoord = GetElemState (elemGuid, definitions, "Sync_correct_flag", flagfindcoord, false);
        syncclass = GetElemState (elemGuid, definitions, "Sync_class_flag", flagfindclass, false);
    }
    if (!syncall && !synccoord && !syncclass) return false; //Если оба свойства-флага ложь - выходим
    if (syncall && !flagfindcoord) synccoord = true; //Если флаг координат не найден - проверку всё равно делаем
    if (syncall && !flagfindclass) syncclass = true;
    GS::Array <WriteData> mainsyncRules = {};
    bool hassubguid = false;
    ParamDictValue subproperty = {};
    if (syncall) hassubguid = ParamHelpers::SubGuid_GetParamValue (elemGuid, definitions, subproperty);
    ParamDictElement paramToRead = {}; // Словарь с параметрами для чтения
    bool hasSub = false;

    for (auto& definition : definitions) {
        // Получаем список правил синхронизации из всех свойств
        ParseSyncString (elemGuid, elementType, definition, mainsyncRules, paramToRead, hasSub, syncall, synccoord, syncclass, subproperty); // Парсим описание свойства
    }
    if (mainsyncRules.IsEmpty ()) return false;
    if (!(PROPERTYCACHE ().isPropertyDefinitionRead_full && PROPERTYCACHE ().isPropertyDefinition_OK)) {
        PROPERTYCACHE ().AddPropertyDefinition (definitions);
    }
    // Заполняем правила синхронизации с учётом субэлементов, попутно заполняем словарь параметров для чтения/записи
    WriteDict syncRules; // Словарь с правилами для каждого элемента
    SyncAddSubelement (subelemGuids, mainsyncRules, syncRules, paramToRead);
    mainsyncRules.Clear ();
    subelemGuids.Push (elemGuid); // Это теперь список всех элементов для синхронизации

    // Читаем все возможные свойства
    if (paramToRead.IsEmpty ()) return false;
    ParamHelpers::ElementsRead (paramToRead);
    GS::HashTable<API_Guid, GS::UniString> property_write_guid = {}; // Словарь GUID свойств, в которые могла быть осуществлена запись
    SyncCalcRule (syncRules, subelemGuids, paramToRead, paramToWrite, property_write_guid);
    if (paramToWrite.IsEmpty ()) return false;
    // Некоторые свойства, возможно, ссылались на изменённые. Чтоб не запускать полную синхронизацию ещё раз
    ParamHelpers::CompareParamDictElement (paramToWrite, paramToRead);
    SyncCalcRule (syncRules, subelemGuids, paramToRead, paramToWrite, property_write_guid);
    return SyncNeedResync (paramToRead, property_write_guid);
}

bool SyncNeedResync (ParamDictElement& paramToRead, GS::HashTable<API_Guid, GS::UniString> property_write_guid)
{
    if (property_write_guid.IsEmpty ()) return false;
    if (paramToRead.IsEmpty ()) return false;
    for (ParamDictElement::PairIterator cIt = paramToRead.EnumeratePairs (); cIt != NULL; ++cIt) {
        #if defined(AC_28) || defined(AC_29)
        ParamDictValue& params = cIt->value;
        API_Guid elemGuid = cIt->key;
        #else
        ParamDictValue& params = *cIt->value;
        API_Guid elemGuid = *cIt->key;
        #endif
        if (params.IsEmpty ()) continue;
        for (ParamDictValue::PairIterator cItt = params.EnumeratePairs (); cItt != NULL; ++cItt) {
            #if defined(AC_28) || defined(AC_29)
            ParamValue& param = cItt->value;
            #else
            ParamValue& param = *cItt->value;
            #endif
            if (!param.definition.defaultValue.hasExpression) continue;
            if (property_write_guid.ContainsKey (param.definition.guid)) continue;
            for (const GS::UniString& expr : param.definition.defaultValue.propertyExpressions) {
                if (!expr.Contains ("###Property:")) continue;
                GS::Array<GS::UniString> partstring = {};
                if (StringSplt (expr, "###", partstring, "Property:") > 0) {
                    for (const GS::UniString& part : partstring) {
                        GS::UniString h = part.GetSuffix (36);
                        API_Guid pguid = APIGuidFromString (h.ToCStr (0, MaxUSize, GChCode));
                        if (property_write_guid.ContainsKey (pguid)) {
                            msg_rep ("Find property in expression for resync", property_write_guid.Get (pguid) + " => " + param.rawName, NoError, elemGuid);
                            return true;
                        }
                    }
                }

            }
        }
    }
    return false;
}

void SyncCalcRule (const WriteDict& syncRules, const GS::Array<API_Guid>& subelemGuids, const ParamDictElement& paramToRead, ParamDictElement& paramToWrite, GS::HashTable<API_Guid, GS::UniString>& property_write_guid)
{
    GS::HashTable<API_Guid, ParamDict> reset_property = {}; // Словарь со сбрасываемыми значениями
    // Выбираем по-элементно параметры для чтения и записи, формируем словарь
    for (const API_Guid& elemGuid : subelemGuids) {
        if (!syncRules.ContainsKey (elemGuid)) continue;
        GS::Array <WriteData> writeSubs = syncRules.Get (elemGuid);
        if (writeSubs.IsEmpty ()) continue;
        // Заполняем значения параметров чтения/записи из словаря
        for (const WriteData& writeSub : writeSubs) {
            const API_Guid elemGuidTo = writeSub.guidTo;
            const API_Guid elemGuidFrom = writeSub.guidFrom;
            // Проверяем - есть ли вообще эти элементы в словаре параметров
            if (!paramToRead.ContainsKey (elemGuidTo)) continue;
            if (!paramToRead.ContainsKey (elemGuidFrom)) continue;
            GS::UniString rawNameTo = writeSub.paramTo.rawName;
            ParamDictValue paramsTo = paramToRead.Get (elemGuidTo);
            if (!paramsTo.ContainsKey (rawNameTo)) continue;
            GS::UniString rawNameFrom = writeSub.paramFrom.rawName;
            ParamDictValue paramsFrom = {};
            if (elemGuidFrom == elemGuidTo) {
                paramsFrom = paramsTo;
            } else {
                paramsFrom = paramToRead.Get (elemGuidFrom);
            }
            if (!paramsFrom.ContainsKey (rawNameFrom)) continue;
            // Проверяем наличие имён в словаре параметров
            ParamValue paramFrom = paramsFrom.Get (rawNameFrom);
            ParamValue paramTo = paramsTo.Get (rawNameTo);
            FormatString formatstring = writeSub.formatstring;
            //Сопоставляем и записываем, если значения отличаются
            bool is_ignore = false;
            if (ParamHelpers::CompareParamValue (paramFrom, paramTo, formatstring, writeSub.ignorevals, is_ignore)) {
                ParamHelpers::AddParamValue2ParamDictElement (paramTo, paramToWrite);
                // Если это свойство и в него планировалась запись - сохраним GUID в словарь
                if (paramTo.definition.guid != APINULLGuid) {
                    if (!property_write_guid.ContainsKey (paramTo.definition.guid)) property_write_guid.Add (paramTo.definition.guid, paramTo.rawName);
                }
            }
            if (is_ignore && writeSub.ignorevals.reset_to_def && paramTo.definition.guid != APINULLGuid) {
                if (reset_property.ContainsKey (elemGuid)) {
                    if (!reset_property.Get (elemGuid).ContainsKey (paramTo.rawName)) {
                        reset_property.Get (elemGuid).Add (paramTo.rawName, true);
                    }
                } else {
                    ParamDict u = {};
                    u.Add (paramTo.rawName, true);
                    reset_property.Add (elemGuid, u);
                }
            }
        }
    }
    // Добавляем свойства для сброса
    if (reset_property.IsEmpty ()) return;
    for (GS::HashTable<API_Guid, ParamDict>::PairIterator cIt = reset_property.EnumeratePairs (); cIt != NULL; ++cIt) {
        #if defined(AC_28) || defined(AC_29)
        const API_Guid elemGuid = cIt->key;
        ParamDict& r = cIt->value;
        #else
        const API_Guid elemGuid = *cIt->key;
        ParamDict& r = *cIt->value;
        #endif
        if (!paramToRead.ContainsKey (elemGuid)) continue;
        for (ParamDict::PairIterator cIt = r.EnumeratePairs (); cIt != NULL; ++cIt) {
            #if defined(AC_28) || defined(AC_29)
            const GS::UniString& rawName = cIt->key;
            #else
            const GS::UniString& rawName = *cIt->key;
            #endif
            if (paramToWrite.ContainsKey (elemGuid)) {
                if (paramToWrite.Get (elemGuid).ContainsKey (rawName)) continue;
            }
            if (!paramToRead.Get (elemGuid).ContainsKey (rawName)) continue;
            ParamValue paramTo = paramToRead.Get (elemGuid).Get (rawName);
            paramTo.needResetToDef = true;
            paramTo.isValid = true;
            ParamHelpers::AddParamValue2ParamDictElement (paramTo, paramToWrite);
        }
    }
    return;
}

// --------------------------------------------------------------------
// Добавление подэлементов и их параметров в правила синхорнизации
// --------------------------------------------------------------------
void SyncAddSubelement (const GS::Array<API_Guid>& subelemGuids, const GS::Array <WriteData>& mainsyncRules, WriteDict& syncRules, ParamDictElement& paramToRead)
{
    for (UInt32 i = 0; i < mainsyncRules.GetSize (); i++) {
        if (!mainsyncRules[i].fromSub && !mainsyncRules[i].toSub) {
            WriteData writeSub = mainsyncRules.Get (i);
            SyncAddRule (writeSub, syncRules, paramToRead);
        }

        // Если есть субэлементы - обработаем их
        if (!subelemGuids.IsEmpty ()) {

            // Для записи из дочернего в родительский возьмём только один, первый элемент
            if (mainsyncRules[i].fromSub) {
                WriteData writeSub = mainsyncRules.Get (i);
                API_Guid subelemGuid = subelemGuids.Get (0);
                writeSub.fromSub = false;
                writeSub.guidFrom = subelemGuid;
                writeSub.paramFrom.fromGuid = subelemGuid;
                SyncAddRule (writeSub, syncRules, paramToRead);
            }
            if (mainsyncRules[i].toSub) {
                for (UInt32 j = 0; j < subelemGuids.GetSize (); j++) {
                    WriteData writeSub = mainsyncRules.Get (i);
                    API_Guid subelemGuid = subelemGuids.Get (j);
                    writeSub.toSub = false;
                    writeSub.guidTo = subelemGuid;
                    writeSub.paramTo.fromGuid = subelemGuid;
                    SyncAddRule (writeSub, syncRules, paramToRead);
                }
            }
        }
    }
}

// --------------------------------------------------------------------
// Запись правила в словарь, попутно заполняем словарь с параметрами
// --------------------------------------------------------------------
void SyncAddRule (const WriteData& writeSub, WriteDict& syncRules, ParamDictElement& paramToRead)
{
    API_Guid elemGuid = writeSub.guidTo;
    if (syncRules.ContainsKey (elemGuid)) {
        syncRules.Get (elemGuid).Push (writeSub);
    } else {
        GS::Array <WriteData> rules = {};
        rules.Push (writeSub);
        syncRules.Add (elemGuid, rules);
    }
    ParamHelpers::AddParamValue2ParamDictElement (writeSub.paramFrom, paramToRead);
    ParamHelpers::AddParamValue2ParamDictElement (writeSub.paramTo, paramToRead);
}

// -----------------------------------------------------------------------------
// Парсит описание свойства, заполняет массив с правилами (GS::Array <WriteData>)
// -----------------------------------------------------------------------------
bool ParseSyncString (const API_Guid& elemGuid, const API_ElemTypeID& elementType, const API_PropertyDefinition& definition, GS::Array <WriteData>& syncRules, ParamDictElement& paramToRead, bool& hasSub, bool syncall, bool synccoord, bool syncclass, ParamDictValue& subproperty)
{
    // TODO Попробовать отключать часть синхронизаций в зависимости от изменённых параметров (API_ActTranPars acttype)
    GS::UniString description_string = definition.description;
    if (description_string.IsEmpty ()) {
        return false;
    }
    if (description_string.Contains ("Sync_flag")) {
        return false;
    }
    if (description_string.Contains ("Sync_correct_flag")) {
        ParamDictValue paramDict = {};
        ParamValue paramdef = {}; //Свойство, из которого получено правило
        ParamHelpers::ConvertToParamValue (paramdef, definition);
        paramdef.rawName = "{@property:sync_correct_flag}";
        paramdef.fromGuid = elemGuid;
        paramDict.Add (paramdef.rawName, paramdef);
        ParamHelpers::AddParamDictValue2ParamDictElement (elemGuid, paramDict, paramToRead);
        return true;
    }

    // Если указан сброс данных - синхронизировать не будем
    #ifndef AC_27
    if (description_string.Contains ("Sync_reset")) {
        return false;
    }
    #endif
    bool hasRule = false;
    if (description_string.Contains ("Sync_") && description_string.Contains ("{") && description_string.Contains ("}")) {
        if (description_string.Contains (" {")) {
            description_string.ReplaceAll ("\n", "");
            description_string.ReplaceAll ("\r", "");
            description_string.ReplaceAll ("\t", "");
            description_string.ReplaceAll ("from {", "from{");
            description_string.ReplaceAll ("from  {", "from{");
            description_string.ReplaceAll ("to {", "to{");
            description_string.ReplaceAll ("to  {", "to{");
            description_string.ReplaceAll ("to_sub {", "to_sub{");
            description_string.ReplaceAll ("to_sub  {", "to_sub{");
            description_string.ReplaceAll ("from_sub {", "from_sub{");
            description_string.ReplaceAll ("from_sub  {", "from_sub{");
            description_string.ReplaceAll ("from_GUID {", "from_GUID{");
            description_string.ReplaceAll ("from_GUID  {", "from_GUID{");
            description_string.ReplaceAll ("to_GUID {", "to_GUID{");
            description_string.ReplaceAll ("to_GUID  {", "to_GUID{");
        }
        GS::Array<GS::UniString> rulestring = {};
        UInt32 nrule = StringSplt (description_string, "Sync_", rulestring, "{"); // Проверяем количество правил
        ParamValue paramdef = {}; //Свойство, из которого получено правило
        ParamHelpers::ConvertToParamValue (paramdef, definition);
        paramdef.fromGuid = elemGuid;
        //Проходим по каждому правилу и извлекаем из него правило синхронизации (WriteDict syncRules)
        //и словарь уникальных параметров для чтения/записи (ParamDictElement paramToRead)
        for (UInt32 i = 0; i < nrule; i++) {
            ParamValue param;
            int syncdirection = SYNC_NO; // Направление синхронизации
            GS::UniString rawparamName = ""; //Имя параметра/свойства с указанием типа синхронизации, для ключа словаря
            SkipValues ignorevals = {}; //Игнорируемые значения
            FormatString stringformat = {};
            GS::UniString rulestring_one = rulestring[i];
            API_Guid elemGuidfrom = elemGuid; // Элемент, из которого читаем данные
            API_Guid elemGuidto = elemGuid; // Элемент, в котороый записываем данные
            API_ElemTypeID elementType_from = elementType; // Тип элемента, из которого читаем данные
            // Копировать из другого элемента
            if (rulestring_one.Contains ("from_GUID{")) {
                GS::Array<GS::UniString> params = {};
                rulestring_one.ReplaceAll ("from_GUID", "");
                UInt32 nparams = StringSplt (rulestring_one, ";", params);
                if (nparams == 2) {
                    GS::UniString rawname = "";
                    Name2Rawname (params[0], rawname);
                    if (subproperty.ContainsKey (rawname)) {
                        if (subproperty.Get (rawname).isValid) {
                            elemGuidfrom = APIGuidFromString (subproperty.Get (rawname).val.uniStringValue.ToCStr (0, MaxUSize, GChCode));
                            rulestring_one = "Sync_from{" + params[1];
                            elementType_from = GetElemTypeID (elemGuidfrom);
                        } else {
                            return false;
                        }
                    } else {
                        return false;
                    }
                } else {
                    return false;
                }
            }
            // Копировать в другой элемент
            if (rulestring_one.Contains ("to_GUID{")) {
                GS::Array<GS::UniString> params = {};
                rulestring_one.ReplaceAll ("to_GUID", "");
                UInt32 nparams = StringSplt (rulestring_one, ";", params);
                if (nparams == 2) {
                    GS::UniString rawname = "";
                    Name2Rawname (params[0], rawname);
                    if (subproperty.ContainsKey (rawname)) {
                        if (subproperty.Get (rawname).isValid) {
                            elemGuidto = APIGuidFromString (subproperty.Get (rawname).val.uniStringValue.ToCStr (0, MaxUSize, GChCode));
                            param.fromGuid = elemGuidto;
                            rulestring_one = "Sync_to{" + params[1];
                        } else {
                            return false;
                        }
                    } else {
                        return false;
                    }
                } else {
                    return false;
                }
            }
            if (SyncString (elementType_from, rulestring_one, syncdirection, param, ignorevals, stringformat, syncall, synccoord, syncclass)) {
                hasRule = true;
                WriteData writeOne = {};
                writeOne.formatstring = stringformat;
                writeOne.ignorevals = ignorevals;
                if (param.fromCoord && (param.rawName.Contains ("north_dir") || param.rawName.Contains ("_sp_"))) {
                    ParamDictValue paramDict = {};
                    ParamHelpers::AddValueToParamDictValue (paramDict, "@glob:glob_north_dir");
                    ParamHelpers::AddParamDictValue2ParamDictElement (elemGuid, paramDict, paramToRead);
                }

                // Вытаскиваем параметры для материалов, если такие есть
                if (param.fromMaterial) {
                    ParamDictValue paramDict = {};
                    GS::UniString templatestring = param.val.uniStringValue; //Строка с форматом числа
                    if (ParamHelpers::ParseParamNameMaterial (templatestring, paramDict)) {
                        param.val.uniStringValue = templatestring;

                        // Свойств со спецтекстом может быть несколько (случайно)
                        // Тут будут костыли, которые хорошо бы убрать
                        for (UInt32 inx = 0; inx < 20; inx++) {
                            ParamHelpers::AddValueToParamDictValue (paramDict, "@property:sync_name" + GS::UniString::Printf ("%d", inx));
                        }
                        if (param.fromQuantity) {
                            ParamHelpers::AddValueToParamDictValue (paramDict, "@property:buildingmaterialproperties/some_stuff_th");
                            ParamHelpers::AddValueToParamDictValue (paramDict, "@property:buildingmaterialproperties/some_stuff_units");
                            ParamHelpers::AddValueToParamDictValue (paramDict, "@property:buildingmaterialproperties/some_stuff_kzap");
                        }
                        ParamHelpers::AddParamDictValue2ParamDictElement (elemGuid, paramDict, paramToRead);
                        hasSub = true; // Нужно будет прочитать все свойства
                    }
                }
                if (!param.fromMaterial && param.val.hasFormula) {
                    ParamDictValue paramDict = {};
                    GS::UniString templatestring = param.val.uniStringValue; //Строка с форматом числа
                    param.type = definition.valueType;
                    if (ParamHelpers::ParseParamNameMaterial (templatestring, paramDict, false)) {
                        param.val.uniStringValue = templatestring;
                        ParamHelpers::AddParamDictValue2ParamDictElement (elemGuid, paramDict, paramToRead);
                        hasSub = true;
                    }
                }
                if (syncdirection == SYNC_TO || syncdirection == SYNC_TO_SUB) {
                    if (syncdirection == SYNC_TO_SUB) {
                        hasSub = true;
                        writeOne.toSub = true;
                    } else {
                        writeOne.guidTo = elemGuidto;
                        if (param.fromGuid == APINULLGuid) param.fromGuid = elemGuidfrom;
                    }
                    writeOne.guidFrom = elemGuidfrom;
                    writeOne.paramFrom = paramdef;
                    writeOne.paramTo = param;
                }
                if (syncdirection == SYNC_FROM || syncdirection == SYNC_FROM_SUB) {
                    if (syncdirection == SYNC_FROM_SUB) {
                        hasSub = true;
                        writeOne.fromSub = true;
                    } else {
                        writeOne.guidFrom = elemGuidfrom;
                        if (param.fromGuid == APINULLGuid) param.fromGuid = elemGuidfrom;
                    }
                    writeOne.guidTo = elemGuidto;
                    writeOne.paramTo = paramdef;
                    writeOne.paramFrom = param;
                }
                syncRules.Push (writeOne);
            }
        }
    }
    return hasRule;
}

bool Name2Rawname (GS::UniString& name, GS::UniString& rawname)
{
    if (name.IsEmpty ()) return false;
    GS::UniString paramNamePrefix = "";
    if (!name.Contains ("}")) name = name + "}";
    if (!name.Contains ("{")) name = name + "{";
    bool synctypefind = false;
    if (name.Contains ("symb_pos_x") || name.Contains ("symb_pos_y") || name.Contains ("symb_pos_z")) {
        name.ReplaceAll ("{symb_pos_", "{Coord:symb_pos_");
    }

    if (name.Contains ("{id}") || name.Contains ("{ID}")) {
        paramNamePrefix = IDNAMEPREFIX;
        synctypefind = true;
    }
    if (synctypefind == false) {
        if (!name.Contains (":") || name.Contains ("escription:") || name.Contains ("esc:")) {
            if (name.Contains ("escription:") || name.Contains ("esc:")) {
                name.ReplaceAll ("description:", "");
                name.ReplaceAll ("Description:", "");
                name.ReplaceAll ("desc:", "");
                name.ReplaceAll ("Desc:", "");
            }
            paramNamePrefix = GDLNAMEPREFIX;
            synctypefind = true;
        }
    }
    if (synctypefind == false) {
        if (name.Contains ("Property:")) {
            name.ReplaceAll ("Property:", "");
            paramNamePrefix = PROPERTYNAMEPREFIX;
            synctypefind = true;
        }
    }
    if (synctypefind == false) {
        if (name.Contains ("Morph:")) {
            name.ReplaceAll ("Morph:", "");
            paramNamePrefix = MORPHNAMEPREFIX;
            synctypefind = true;
        }
    }
    if (synctypefind == false) {
        if (name.Contains ("Coord:")) {
            name.ReplaceAll ("Coord:", "");
            paramNamePrefix = COORDNAMEPREFIX;
            synctypefind = true;
        }
    }
    if (synctypefind == false) {
        if (name.Contains ("Info:")) {
            name.ReplaceAll ("Info:", "");
            paramNamePrefix = INFONAMEPREFIX;
            synctypefind = true;
        }
    }
    if (synctypefind == false) {
        if (name.Contains ("IFC:")) {
            name.ReplaceAll ("IFC:", "");
            paramNamePrefix = IFCNAMEPREFIX;
            synctypefind = true;
        }
    }
    if (synctypefind == false) {
        if (name.Contains ("Glob:")) {
            name.ReplaceAll ("Glob:", "");
            paramNamePrefix = GLOBNAMEPREFIX;
            synctypefind = true;
        }
    }
    if (synctypefind == false) {
        if (name.Contains ("Class:")) {
            name.ReplaceAll ("Class:", "");
            paramNamePrefix = CLASSNAMEPREFIX;
            synctypefind = true;
        }
    }
    if (synctypefind == false) {
        if (name.Contains ("Attribute:")) {
            synctypefind = true;
            name.ReplaceAll ("Attribute:", "");
            paramNamePrefix = ATTRIBNAMEPREFIX;
        }
    }
    if (synctypefind == false) {
        if (name.Contains ("Element:")) {
            synctypefind = true;
            name.ReplaceAll ("Element:", "");
            paramNamePrefix = ELEMENTNAMEPREFIX;
        }
    }
    if (synctypefind == false) {
        if (name.Contains ("MEP:")) {
            synctypefind = true;
            name.ReplaceAll ("MEP:", "");
            paramNamePrefix = MEPNAMEPREFIX;
        }
    }
    if (synctypefind == false) {
        if (name.Contains ("Listdata:")) {
            synctypefind = true;
            name.ReplaceAll ("Listdata:", "");
            paramNamePrefix = LISTDATANAMEPREFIX;
        }
    }
    if (synctypefind == false) return false;
    GS::Array<GS::UniString> params;
    GS::UniString tparamName = name.GetSubstring ('{', '}', 0);
    UInt32 nparam = StringSplt (tparamName, ";", params);
    if (nparam == 0) return false;
    GS::UniString paramName = params.Get (0);
    FormatStringFunc::GetFormatString (paramName);
    paramName.ReplaceAll ("\\/", "/");
    rawname = paramNamePrefix + paramName.ToLowerCase () + "}";
    return true;
}

// -----------------------------------------------------------------------------
// Парсит описание свойства
// -----------------------------------------------------------------------------
bool SyncString (const  API_ElemTypeID& elementType, GS::UniString rulestring_one, int& syncdirection, ParamValue& param, SkipValues& ignorevals, FormatString& stringformat, bool syncall, bool synccoord, bool syncclass)
{
    syncdirection = SYNC_NO;
    // Выбор направления синхронизации
    // Копировать в субэлементы или из субэлементов
    if (syncdirection == SYNC_NO && rulestring_one.Contains ("to_sub{")) syncdirection = SYNC_TO_SUB;
    if (syncdirection == SYNC_NO && rulestring_one.Contains ("from_sub{")) syncdirection = SYNC_FROM_SUB;

    // Копировать параметр в свойство или свойство в параметр
    if (syncdirection == SYNC_NO && rulestring_one.Contains ("from{")) syncdirection = SYNC_FROM;
    if (syncdirection == SYNC_NO && rulestring_one.Contains ("to{")) syncdirection = SYNC_TO;

    //Если направление синхронизации не нашли - выходим
    if (syncdirection == SYNC_NO) return false;
    GS::UniString paramNamePrefix = "";
    bool synctypefind = false;

    //Выбор типа копируемого свойства
    //Я не очень понял - умеет ли с++ в ленивые вычисления, поэтому сделаю вложенные условия, чтобы избежать ненужного поиска по строке
    if (rulestring_one.Contains ("symb_pos_x") || rulestring_one.Contains ("symb_pos_y") || rulestring_one.Contains ("symb_pos_z")) {
        rulestring_one.ReplaceAll ("{symb_pos_", "{Coord:symb_pos_");
    }
    bool hasformula = rulestring_one.Contains (char_formula_start) && rulestring_one.Contains (char_formula_end);
    if (rulestring_one.Contains ("QRCode:")) {
        rulestring_one.ReplaceAll ("QRCode:", "");
        param.toQRCode = true;
    }
    if (synctypefind == false) {
        if (rulestring_one.Contains ("{id}") || rulestring_one.Contains ("{ID}")) {
            paramNamePrefix = IDNAMEPREFIX;
            param.typeinx = IDTYPEINX;
            param.fromID = true;
            synctypefind = true;
        }
    }
    if (synctypefind == false) {
        if ((!rulestring_one.Contains (":") || rulestring_one.Contains ("escription:") || rulestring_one.Contains ("esc:")) && !hasformula) {
            if (rulestring_one.Contains ("escription:") || rulestring_one.Contains ("esc:")) {
                param.fromGDLdescription = true;
                rulestring_one.ReplaceAll ("description:", "");
                rulestring_one.ReplaceAll ("Description:", "");
                rulestring_one.ReplaceAll ("desc:", "");
                rulestring_one.ReplaceAll ("Desc:", "");
            }
            paramNamePrefix = GDLNAMEPREFIX;
            param.typeinx = GDLTYPEINX;
            param.fromGDLparam = true;
            synctypefind = true;
        }
    }
    GS::UniString stringformat_raw = "";

    // TODO Дописать чтение состава многослойной конструкции по её имени
    if (synctypefind == false) {
        if (rulestring_one.Contains ("Attribute:")) {
            synctypefind = true;
            rulestring_one.ReplaceAll ("Attribute:", "");
            paramNamePrefix = ATTRIBNAMEPREFIX;
            param.typeinx = ATTRIBTYPEINX;
            param.fromAttribElement = true;
            syncdirection = SYNC_TO;
        }
    }
    if (synctypefind == false) {
        if (rulestring_one.Contains ("Material:") && (rulestring_one.Contains ('"') || hasformula)) {
            synctypefind = true;
            rulestring_one.ReplaceAll ("Material:", "");
            rulestring_one.ReplaceAll ("{Layers;", "{Layers,20;");
            rulestring_one.ReplaceAll ("{Layers_inv;", "{Layers_inv,20;");
            rulestring_one.ReplaceAll ("{Layers_auto;", "{Layers_auto,20;");
            paramNamePrefix = MATERIALNAMEPREFIX;
            param.typeinx = MATERIALTYPEINX;
            GS::UniString templatestring = "";
            if (hasformula) {
                if (rulestring_one.Contains ('"')) {
                    templatestring = rulestring_one.GetSubstring ('"', '"', 0);
                    FormatStringFunc::GetFormatStringFromFormula (rulestring_one, templatestring, stringformat_raw);
                    stringformat = FormatStringFunc::ParseFormatString (stringformat_raw);
                    param.val.uniStringValue = templatestring;
                } else {
                    templatestring = rulestring_one.GetSubstring (char_formula_start, char_formula_end, 0);
                    FormatStringFunc::GetFormatStringFromFormula (rulestring_one, templatestring, stringformat_raw);
                    stringformat = FormatStringFunc::ParseFormatString (stringformat_raw);
                    param.val.uniStringValue = str_formula_start;
                    param.val.uniStringValue.Append (templatestring);
                    param.val.uniStringValue.Append (str_formula_end);
                }
                rulestring_one.ReplaceAll (templatestring, "");
                rulestring_one.ReplaceAll (stringformat_raw, "");
                param.val.hasFormula = true;
            } else {
                templatestring = rulestring_one.GetSubstring ('"', '"', 0);
                param.val.uniStringValue = templatestring;
                param.val.hasFormula = false;
                rulestring_one.ReplaceAll (templatestring, "");
            }
            param.val.formatstring = stringformat;
            param.fromMaterial = true;
            param.composite_pen = 20;
            rulestring_one.ReplaceAll (" ", "");
            if (rulestring_one.Contains (",") && rulestring_one.Contains (";")) {
                GS::UniString penstring = rulestring_one.GetSubstring (',', ';', 0);
                if (penstring.Contains ("all")) {
                    param.composite_pen = -1;
                    param.fromQuantity = true;
                } else {
                    if (penstring.Contains ("unic")) {
                        param.composite_pen = -2;
                        param.fromQuantity = true;
                    } else {
                        short pen = std::atoi (penstring.ToCStr ());
                        if (pen > 0) param.composite_pen = pen;
                    }
                }
            }
            if (!templatestring.IsEmpty ()) syncdirection = SYNC_FROM;
        }
    }
    if (synctypefind == false) {
        if (hasformula) {
            GS::UniString templatestring = "";
            if (rulestring_one.Contains ('"')) {
                templatestring = rulestring_one.GetSubstring ('"', '"', 0);
                FormatStringFunc::GetFormatStringFromFormula (rulestring_one, templatestring, stringformat_raw);
                stringformat = FormatStringFunc::ParseFormatString (stringformat_raw);
                param.val.uniStringValue = templatestring;
            } else {
                templatestring = rulestring_one.GetSubstring (char_formula_start, char_formula_end, 0);
                FormatStringFunc::GetFormatStringFromFormula (rulestring_one, templatestring, stringformat_raw);
                stringformat = FormatStringFunc::ParseFormatString (stringformat_raw);
                param.val.uniStringValue = str_formula_start;
                param.val.uniStringValue.Append (templatestring);
                param.val.uniStringValue.Append (str_formula_end);
            }
            rulestring_one.ReplaceAll (templatestring, "");
            rulestring_one.ReplaceAll (stringformat_raw, "");
            param.val.formatstring = stringformat;
            synctypefind = true;
            paramNamePrefix = FORMULANAMEPREFIX;
            param.typeinx = FORMULATYPEINX;
            param.val.hasFormula = true;
            syncdirection = SYNC_FROM;
        }
    }
    if (synctypefind == false) {
        if (rulestring_one.Contains ("Morph:")) {
            synctypefind = true;
            rulestring_one.ReplaceAll ("Morph:", "");
            paramNamePrefix = MORPHNAMEPREFIX;
            param.typeinx = MORPHTYPEINX;
            param.fromMorph = true;
            syncdirection = SYNC_FROM;
        }
    }
    if (synctypefind == false) {
        if (rulestring_one.Contains ("Coord:")) {
            synctypefind = true;
            rulestring_one.ReplaceAll ("Coord:", "");
            paramNamePrefix = COORDNAMEPREFIX;
            param.typeinx = COORDTYPEINX;
            param.fromCoord = true;
            if (rulestring_one.Contains ("orth")) param.fromGlob = true;
            if (!rulestring_one.Contains ("symb_pos_") && !rulestring_one.Contains ("symb_rotangle")) syncdirection = SYNC_FROM;
        }
    }
    if (synctypefind == false) {
        if (rulestring_one.Contains ("Property:")) {
            synctypefind = true;
            rulestring_one.ReplaceAll ("Property:", "");
            paramNamePrefix = PROPERTYNAMEPREFIX;
            param.typeinx = PROPERTYTYPEINX;
            param.fromProperty = true;
        }
    }
    if (synctypefind == false) {
        if (rulestring_one.Contains ("Info:")) {
            synctypefind = true;
            rulestring_one.ReplaceAll ("Info:", "");
            paramNamePrefix = INFONAMEPREFIX;
            param.typeinx = INFOTYPEINX;
            param.fromInfo = true;
        }
    }
    if (synctypefind == false) {
        if (rulestring_one.Contains ("IFC:")) {
            synctypefind = true;
            rulestring_one.ReplaceAll ("IFC:", "");
            paramNamePrefix = IFCNAMEPREFIX;
            param.typeinx = IFCTYPEINX;
            param.fromIFCProperty = true;
            syncdirection = SYNC_FROM;
        }
    }
    if (synctypefind == false) {
        if (rulestring_one.Contains ("Glob:")) {
            synctypefind = true;
            rulestring_one.ReplaceAll ("Glob:", "");
            paramNamePrefix = GLOBNAMEPREFIX;
            param.typeinx = GLOBTYPEINX;
            param.fromGlob = true;
            syncdirection = SYNC_FROM;
        }
    }
    if (synctypefind == false) {
        if (rulestring_one.Contains ("Class:")) {
            synctypefind = true;
            rulestring_one.ReplaceAll ("Class:", "");
            paramNamePrefix = CLASSNAMEPREFIX;
            param.typeinx = CLASSTYPEINX;
            param.fromClassification = true;
        }
    }

    if (synctypefind == false) {
        if (rulestring_one.Contains ("Element:")) {
            synctypefind = true;
            rulestring_one.ReplaceAll ("Element:", "");
            paramNamePrefix = ELEMENTNAMEPREFIX;
            param.typeinx = ELEMENTTYPEINX;
            param.fromElement = true;
            syncdirection = SYNC_FROM;
        }
    }
    if (synctypefind == false) {
        if (rulestring_one.Contains ("MEP:")) {
            synctypefind = true;
            rulestring_one.ReplaceAll ("MEP:", "");
            paramNamePrefix = MEPNAMEPREFIX;
            param.typeinx = MEPTYPEINX;
            param.fromMEP = true;
            syncdirection = SYNC_FROM;
        }
    }
    if (synctypefind == false) {
        if (rulestring_one.Contains ("Listdata:")) {
            synctypefind = true;
            rulestring_one.ReplaceAll ("Listdata:", "");
            paramNamePrefix = LISTDATANAMEPREFIX;
            param.typeinx = LISTDATATYPEINX;
            param.fromListData = true;
            syncdirection = SYNC_FROM;
        }
    }
    if (synctypefind == false) return false;
    param.eltype = elementType;

    //Проверка допустимости правила для типа элемента
    if (param.fromGDLparam) {
        if (elementType == API_WallID ||
           elementType == API_SlabID ||
           elementType == API_ColumnID ||
           elementType == API_BeamID ||
           elementType == API_RoofID ||
           elementType == API_ShellID ||
           elementType == API_BeamSegmentID ||
           elementType == API_ColumnSegmentID ||
           elementType == API_MorphID) synctypefind = false;
    }
    if (param.fromGDLdescription) {
        if (elementType != API_ObjectID) synctypefind = false;
    }
    if (param.fromListData) {
        if (elementType != API_ObjectID) synctypefind = false;
    }
    if (param.fromMaterial) {
        if (elementType != API_WallID &&
           elementType != API_SlabID &&
           elementType != API_ColumnID &&
           elementType != API_BeamID &&
           elementType != API_RoofID &&
           elementType != API_BeamSegmentID &&
           elementType != API_ColumnSegmentID &&
           elementType != API_MeshID &&
           elementType != API_MorphID &&
           elementType != API_ShellID) synctypefind = false;
    }
    if (param.fromMorph) {
        if (elementType != API_MorphID) synctypefind = false;
    }

    if (syncdirection == SYNC_FROM_SUB && elementType == API_ObjectID) synctypefind = false;

    // Проверка включенных флагов
    if (!syncall) {
        if (!param.fromCoord && !param.fromClassification) synctypefind = false;
    }
    if (!synccoord) {
        if (param.fromCoord) synctypefind = false;
    }
    if (!syncclass) {
        if (param.fromClassification && syncdirection == SYNC_TO) synctypefind = false;
    }

    //Если тип свойства не нашли - выходим
    if (synctypefind == false) return false;

    GS::UniString tparamName = rulestring_one.GetSubstring ('{', '}', 0);
    GS::Array<GS::UniString> params;
    UInt32 nparam = StringSplt (tparamName, ";", params);

    // Параметры не найдены - выходим
    if (nparam == 0) return false;
    GS::UniString paramName = params.Get (0);
    paramName.ReplaceAll ("\\/", "/");
    if (param.fromMaterial || param.val.hasFormula) {
        param.rawName = paramNamePrefix;
        param.rawName.Append (paramName.ToLowerCase ());
        param.rawName.Append (";");
        param.rawName.Append (param.val.uniStringValue);
        param.rawName.Append (".");
        param.rawName.Append (stringformat.stringformat);
        param.rawName.Append ("}");
    } else {
        stringformat_raw = FormatStringFunc::GetFormatString (paramName);
        stringformat = FormatStringFunc::ParseFormatString (stringformat_raw);
    }
    UInt32 start_ignore = 0;
    if (param.fromClassification) {
        param.name = params.Get (0).ToLowerCase ();
        param.rawName = paramNamePrefix;
        param.rawName.Append (param.name);
        if (nparam > 1) {
            param.val.uniStringValue = params.Get (1);
            param.rawName.Append (";");
            param.rawName.Append (param.val.uniStringValue.ToLowerCase ());
        }
        param.rawName.Append ("}");
        start_ignore = 1 + nparam;
    }
    if (param.rawName.IsEmpty ()) {
        param.rawName = paramNamePrefix;
        param.rawName.Append (paramName.ToLowerCase ());
        param.rawName.Append ("}");
    }
    if (param.name.IsEmpty ()) param.name = paramName;
    if (nparam > 1) {
        // Обработка данных о размерах массива и типе чтения
        if (start_ignore == 0) start_ignore = 1;
        GS::UniString arrtype = params[1].ToLowerCase ();
        bool hasArray = false;
        if (!hasArray && (arrtype.Contains ("uniq") || arrtype.Contains ("unic"))) {
            arrtype.ReplaceAll ("uniq", "");
            arrtype.ReplaceAll ("unic", "");
            param.val.array_format_out = ARRAY_UNIC;
            hasArray = true;
        }
        if (!hasArray && arrtype.Contains ("sum")) {
            arrtype.ReplaceAll ("sum", "");
            param.val.array_format_out = ARRAY_SUM;
            hasArray = true;
        }
        if (!hasArray && arrtype.Contains ("min")) {
            arrtype.ReplaceAll ("min", "");
            param.val.array_format_out = ARRAY_MIN;
            hasArray = true;
        }
        if (!hasArray && arrtype.Contains ("max")) {
            arrtype.ReplaceAll ("max", "");
            param.val.array_format_out = ARRAY_MAX;
            hasArray = true;
        }
        if (hasArray && !param.fromListData && !param.fromGDLparam) hasArray = false;
        if (hasArray) {
            int array_row_start = 0;
            int array_row_end = 0;
            int array_column_start = 0;
            int array_column_end = 0;
            GS::UniString rawName_row_start = "";// Имя параметра со значением начала диапазона чтения строк
            GS::UniString rawName_row_end = "";	 // Имя параметра со значением конца диапазона чтения строк
            GS::UniString rawName_col_start = "";// Имя параметра со значением начала диапазона чтения столбцов
            GS::UniString rawName_col_end = "";	 // Имя параметра со значением конца диапазона чтения столбцов
            double p;
            if (params[1].Contains ("(")) {
                GS::Array<GS::UniString> sr;
                UInt32 nsr = StringSplt (arrtype, ")", sr);
                if (nsr > 0) {
                    GS::UniString sr1 = sr.Get (0);
                    sr1.Trim ('(');
                    if (sr1.Contains (",")) {
                        GS::Array<GS::UniString> dim;
                        UInt32 ndim = StringSplt (sr1, ",", dim);
                        if (ndim > 0) {
                            GS::UniString dim0 = dim.Get (0);
                            if (UniStringToDouble (dim0, p)) {
                                array_row_start = (int) p;
                            } else {
                                rawName_row_start = paramNamePrefix;
                                rawName_row_start.Append (dim0);
                                rawName_row_start.Append ("}");
                            }
                        }
                        if (ndim > 1) {
                            GS::UniString dim1 = dim.Get (1);
                            if (UniStringToDouble (dim1, p)) {
                                array_row_end = (int) p;
                            } else {
                                rawName_row_end = paramNamePrefix;
                                rawName_row_end.Append (dim1);
                                rawName_row_end.Append ("}");
                            }
                        }
                    } else {
                        if (UniStringToDouble (sr1, p)) {
                            array_row_start = (int) p;
                            array_row_end = (int) p;
                        } else {
                            rawName_row_start = paramNamePrefix;
                            rawName_row_start.Append (sr1);
                            rawName_row_start.Append ("}");
                            rawName_row_end = rawName_col_start;
                        }
                    }
                }
                if (nsr > 1) {
                    GS::UniString sr1 = sr.Get (1);
                    sr1.Trim ('(');
                    if (sr1.Contains (",")) {
                        GS::Array<GS::UniString> dim;
                        UInt32 ndim = StringSplt (sr1, ",", dim);
                        if (ndim > 0) {
                            GS::UniString dim0 = dim.Get (0);
                            if (UniStringToDouble (dim0, p)) {
                                array_column_start = (int) p;
                            } else {
                                rawName_col_start = paramNamePrefix;
                                rawName_col_start.Append (dim0);
                                rawName_col_start.Append ("}");
                            }
                        }
                        if (ndim > 1) {
                            GS::UniString dim1 = dim.Get (1);
                            if (UniStringToDouble (dim1, p)) {
                                array_column_end = (int) p;
                            } else {
                                rawName_col_end = paramNamePrefix;
                                rawName_col_end.Append (dim1);
                                rawName_col_end.Append ("}");
                            }
                        }
                    } else {
                        if (UniStringToDouble (sr1, p)) {
                            array_column_start = (int) p;
                            array_column_end = (int) p;
                        } else {
                            rawName_col_start = paramNamePrefix;
                            rawName_col_start.Append (sr1);
                            rawName_col_start.Append ("}");
                            rawName_col_end = rawName_col_start;
                        }
                    }
                }
            }
            if (array_row_start < 0) array_row_start = 0;
            if (array_row_end < 0) array_row_end = 0;
            if (array_column_start < 0) array_column_start = 0;
            if (array_column_end < 0) array_column_end = 0;
            param.val.array_row_start = array_row_start;
            param.val.array_row_end = array_row_end;
            param.val.array_column_start = array_column_start;
            param.val.array_column_end = array_column_end;
            param.fromGDLArray = true;

            param.rawName_row_start = rawName_row_start;
            param.rawName_row_end = rawName_row_end;
            param.rawName_col_start = rawName_col_start;
            param.rawName_col_end = rawName_col_end;
            if (!rawName_row_start.IsEmpty () || !rawName_row_end.IsEmpty () || !rawName_col_start.IsEmpty () || !rawName_col_end.IsEmpty ()) param.needPreRead = true;
            param.rawName = paramNamePrefix;
            param.rawName.Append (paramName.ToLowerCase ());
            param.rawName.Append (GS::UniString::Printf ("@arr_%d_%d_%d_%d_%d", array_row_start, array_row_end, array_column_start, array_column_end, param.val.array_format_out));
            param.rawName.Append (rawName_row_start);
            param.rawName.Append ("_");
            param.rawName.Append (rawName_row_end);
            param.rawName.Append ("_");
            param.rawName.Append (rawName_col_start);
            param.rawName.Append ("_");
            param.rawName.Append (rawName_col_end);
            param.rawName.Append ("}");
            start_ignore = 2;
        }
        // Обработка игнорируемых значений
        if (nparam > start_ignore) {
            for (UInt32 j = start_ignore; j < nparam; j++) {
                GS::UniString ignoreval = "";
                if (params[j].Contains ('"')) {
                    ignoreval = params[j].GetSubstring ('"', '"', 0);
                } else {
                    ignoreval = params[j];
                }
                if (ignoreval.IsEmpty ()) continue;
                if (ignoreval.IsEqual (ignorevals_emp)) {
                    ignorevals.skip_empty = true;
                    continue;
                }
                if (ignoreval.IsEqual (ignorevals_trim_emp)) {
                    ignorevals.skip_trim_empty = true;
                    continue;
                }
                if (ignoreval.IsEqual (ignorevals_def)) {
                    ignorevals.reset_to_def = true;
                    continue;
                }
                ignorevals.ignorevals.Push (ignoreval.ToLowerCase ());
            }
        }
    }
    return true;
}

// -----------------------------------------------------------------------------
// Связывает элементы, прописывая в основной элемент GUID привязанных элементов
// -----------------------------------------------------------------------------
void SyncSetSubelement (SyncSettings& syncSettings)
{
    GSErrCode err = NoError;
    GS::UniString fmane = "Set SubElement";
    GS::Array<API_Guid> subguidArray_ = GetSelectedElements (true, true, syncSettings, false, false, false); // Дочерние элементы, в которые будет записана информация об основном элементе
    if (subguidArray_.IsEmpty ()) return;

    API_Element parentelement; // Родительский элемент
    BNZeroMemory (&parentelement, sizeof (API_Element));
    API_ElemTypeID parentelementtype;
    #if defined(AC_27) || defined(AC_28) || defined(AC_29) || defined(AC_26)
    if (!ClickAnElem ("Click an parent elem", API_ZombieElemID, nullptr, &parentelement.header.type, &parentelement.header.guid)) {
        return;
    }
    #else
    if (!ClickAnElem ("Click an parent elem", API_ZombieElemID, nullptr, &parentelement.header.typeID, &parentelement.header.guid)) {
        return;
    }
    #endif
    parentelementtype = GetElemTypeID (parentelement);
    clock_t start, finish;
    double  duration = 0;
    start = clock ();
    // При работе на резрезах возможно попадание по виртуальному элементу, обработаем
    API_Elem_Head parentelementhead = {};
    if (parentelementtype == API_SectElemID) {
        API_Guid parentguid;
        API_ElemTypeID elementType;
        GetParentGUIDSectElem (parentelement.header.guid, parentguid, elementType);
        parentelementhead.guid = parentguid;
        parentelementtype = elementType;
        SetElemTypeID (parentelementhead, parentelementtype);
    } else {
        parentelementhead = parentelement.header;
        parentelementtype = GetElemTypeID (parentelementhead);
    }
    if (!CheckElementType (parentelementtype, syncSettings)) {
        return;
    }
    // Проверяем доступность для редактирования
    GS::Array<API_Guid> subguidArray; // Массив редактируемых и почищенных от SectElem дочерних элементов
    for (UInt32 i = 0; i < subguidArray_.GetSize (); i++) {
        API_ElemTypeID elementType;
        if (IsElementEditable (subguidArray_[i], syncSettings, false, elementType)) {
            if (elementType != API_LabelID) {
                if ((CheckElementType (elementType, syncSettings) && elementType != API_DimensionID)) {
                    if (subguidArray_[i] != parentelementhead.guid) subguidArray.Push (subguidArray_[i]);
                } else {
                    if (elementType == API_SectElemID) {
                        API_Guid parentguid;
                        GetParentGUIDSectElem (subguidArray_[i], parentguid, elementType);
                        if (parentguid != parentelementhead.guid) subguidArray.Push (parentguid);
                    }
                }
            }
        }
    }
    if (subguidArray.IsEmpty ()) return;
    ParamDictValue propertyParams;
    GSFlags flag = 0;
    ParamDictElement paramToWrite = {};
    SyncSetSubelementScope (parentelementhead, subguidArray, paramToWrite, "", true);
    if (!paramToWrite.IsEmpty ()) {
        err = ACAPI_CallUndoableCommand ("SetSubelement",
                                         [&]() -> GSErrCode {
            ParamHelpers::ElementsWrite (paramToWrite);
            return NoError;
        });
        SyncSelected (syncSettings);
    }
    finish = clock ();
    duration = (double) (finish - start) / CLOCKS_PER_SEC;
    GS::UniString time = GS::UniString::Printf (" %.3f s", duration);
    msg_rep (fmane, time, err, APINULLGuid);
}

// -----------------------------------------------------------------------------
// Запись Guid связанных элементов
// Функция для вызова из ACAPI_CallUndoableCommand
// -----------------------------------------------------------------------------
bool SyncSetSubelementScope (const API_Elem_Head& parentelementhead, GS::Array<API_Guid>& subguidArray, ParamDictElement& paramToWrite, const  GS::UniString& suffix, const bool& check_guid)
{
    GSErrCode err = NoError;
    if (subguidArray.IsEmpty ()) return false;
    if (parentelementhead.guid == APINULLGuid) return false;
    ParamDictElement paramToRead;
    if (!SyncGetSyncGUIDProperty (subguidArray, paramToRead, suffix)) return false;
    GS::UniString parentguidtxt = APIGuidToString (parentelementhead.guid);
    // Записываем в дочерние элементы GUID родительского
    bool has_element = false;
    for (UInt32 i = 0; i < subguidArray.GetSize (); i++) {
        if (paramToRead.ContainsKey (subguidArray[i])) {
            ParamDictValue subparams = paramToRead.Get (subguidArray[i]);
            bool flag_write = false;
            for (ParamDictValue::PairIterator cIt = subparams.EnumeratePairs (); cIt != NULL; ++cIt) {
                #if defined(AC_28) || defined(AC_29)
                ParamValue& param = cIt->value;
                #else
                ParamValue& param = *cIt->value;
                #endif
                //TODO Дописать фильтр по типу, слою, свойству
                if (!param.val.uniStringValue.IsEmpty () && check_guid) {
                    API_Elem_Head elementHead = {}; BNZeroMemory (&elementHead, sizeof (API_Elem_Head));
                    elementHead.guid = APIGuidFromString (param.val.uniStringValue.ToCStr (0, MaxUSize, GChCode));
                    err = ACAPI_Element_GetHeader (&elementHead);
                    if (err != NoError) {
                        ParamValue paramtow = subparams.Get (param.rawName);
                        paramtow.val.uniStringValue = parentguidtxt;
                        paramtow.isValid = true;
                        ParamHelpers::AddParamValue2ParamDictElement (subguidArray[i], paramtow, paramToWrite);
                        flag_write = true;
                        has_element = true;
                    }
                } else {
                    ParamValue paramtow = subparams.Get (param.rawName);
                    paramtow.val.uniStringValue = parentguidtxt;
                    paramtow.isValid = true;
                    ParamHelpers::AddParamValue2ParamDictElement (subguidArray[i], paramtow, paramToWrite);
                    flag_write = true;
                    has_element = true;
                }
                if (flag_write) break;
            }
        }
    }
    return has_element;
}


// --------------------------------------------------------------------
// Подсвечивает элементы, GUID которых указан в свойстве с описанием Sync_GUID
// --------------------------------------------------------------------
void SyncShowSubelement (const SyncSettings& syncSettings)
{
    clock_t start, finish;
    double  duration;
    start = clock ();
    GS::UniString fmane = "";
    GSErrCode err = NoError;
    #ifndef AC_22
    GS::Array<API_Guid> guidArray_all = GetSelectedElements (true, false, syncSettings, false, false, false);
    GS::Array<API_Guid> guidArray = {};
    for (UInt32 i = 0; i < guidArray_all.GetSize (); i++) {
        API_ElemTypeID elementType;
        if (GetTypeByGUID (guidArray_all[i], elementType) == NoError) {
            if (elementType == API_LabelID) {

            } else {
                if (elementType == API_SectElemID) {
                    API_Guid parentguid;
                    GetParentGUIDSectElem (guidArray_all[i], parentguid, elementType);
                    guidArray.Push (parentguid);
                } else {
                    guidArray.Push (guidArray_all[i]);
                }
            }
        }
    }
    if (guidArray.IsEmpty ()) return;
    Int32 bisEng = isEng ();
    GS::Array<API_Neig> selNeigs = {};
    UnicGuidByGuid parentGuid = {};
    ParamDictValue propertyParams = {};
    int errcode = 0;
    if (!SyncGetSubelement (guidArray, parentGuid, "", errcode)) {
        if (SyncGetParentelement (guidArray, parentGuid, "", errcode)) {
            fmane = "Show Sub Element";
        } else {
            GS::UniString SubElementHotFoundIdString = RSGetIndString (ID_ADDON_STRINGS + bisEng, SubElementHotFoundId, ACAPI_GetOwnResModule ());
            if (errcode > 0) {
                SubElementHotFoundIdString = SubElementHotFoundIdString + "\n" + RSGetIndString (ID_ADDON_STRINGS + bisEng, SubElementHotFoundId + errcode, ACAPI_GetOwnResModule ());
            }
            ACAPI_WriteReport (SubElementHotFoundIdString, true);
            return;
        }
    } else {
        fmane = "Show Parent Element";
    }
    API_DatabaseInfo homedatabaseInfo;
    API_DatabaseInfo elementdatabaseInfo;
    BNZeroMemory (&homedatabaseInfo, sizeof (API_DatabaseInfo));
    bool checkdb = false; bool isfloorplan = false;
    #if defined(AC_27) || defined(AC_28) || defined(AC_29)
    err = ACAPI_Database_GetCurrentDatabase (&homedatabaseInfo);
    #else
    err = ACAPI_Database (APIDb_GetCurrentDatabaseID, &homedatabaseInfo, nullptr);
    #endif
    if (err == NoError) {
        checkdb = (homedatabaseInfo.typeID != APIWind_3DModelID);
        isfloorplan = (homedatabaseInfo.typeID == APIWind_FloorPlanID);
    } else {
        msg_rep ("SyncShowSubelement", "APIDb_GetCurrentDatabaseID", err, APINULLGuid);
    }
    GS::UniString pname = GetDBName (homedatabaseInfo);
    int count_inv = 0; int count_all = 0; int count_otherplan = 0; int count_del = 0;
    API_Elem_Head tElemHead = {}; BNZeroMemory (&tElemHead, sizeof (API_Elem_Head));
    for (GS::HashTable<API_Guid, UnicGuid>::PairIterator cIt = parentGuid.EnumeratePairs (); cIt != NULL; ++cIt) {
        #if defined(AC_28) || defined(AC_29)
        UnicGuid guids = cIt->value;
        #else
        UnicGuid guids = *cIt->value;
        #endif
        for (UnicGuid::PairIterator cItt = guids.EnumeratePairs (); cItt != NULL; ++cItt) {
            #if defined(AC_28) || defined(AC_29)
            API_Guid guid = cItt->key;
            bool isvisible = cItt->value;
            #else
            API_Guid guid = *cItt->key;
            bool isvisible = *cItt->value;
            #endif
            count_all++;
            if (!isvisible) {
                BNZeroMemory (&tElemHead, sizeof (API_Elem_Head));
                tElemHead.guid = guid;
                err = ACAPI_Element_GetHeader (&tElemHead, 0);
                if (err != NoError) {
                    msg_rep ("ShowSubelement", "Has been delete", err, guid);
                    count_del++;
                    continue;
                } else {
                    UnhideUnlockElementLayer (tElemHead);
                    if (!ACAPI_Element_Filter (tElemHead.guid, APIFilt_OnVisLayer)) {
                        msg_rep ("ShowSubelement", "Element hide by Layer", err, guid);
                        count_inv++;
                        continue;
                    }
                    if (!ACAPI_Element_Filter (tElemHead.guid, APIFilt_IsVisibleByRenovation)) {
                        msg_rep ("ShowSubelement", "Element hide by Renovation", err, guid);
                        count_inv++;
                        continue;
                    }
                    if (!ACAPI_Element_Filter (tElemHead.guid, APIFilt_IsInStructureDisplay)) {
                        msg_rep ("ShowSubelement", "Element hide by StructureDisplay", err, guid);
                        count_inv++;
                        continue;
                    }
                }
            }
            if (isfloorplan) {
                if (!ACAPI_Element_Filter (guid, APIFilt_OnActFloor)) {
                    BNZeroMemory (&tElemHead, sizeof (API_Elem_Head));
                    tElemHead.guid = guid; GS::UniString name = "";
                    if (ACAPI_Element_GetHeader (&tElemHead, 0) == NoError) {
                        name = GS::UniString::Printf ("%d", tElemHead.floorInd);
                    }
                    msg_rep ("ShowSubelement", "Diff floor: " + name, err, guid);
                    selNeigs.PushNew (guid);
                    count_otherplan++;
                    continue;
                }
            }
            if (checkdb) {
                BNZeroMemory (&elementdatabaseInfo, sizeof (API_DatabaseInfo));
                #if defined(AC_27) || defined(AC_28) || defined(AC_29)
                err = ACAPI_Database_GetContainingDatabase (&guid, &elementdatabaseInfo);
                #else
                err = ACAPI_Database (APIDb_GetContainingDatabaseID, &guid, &elementdatabaseInfo);
                #endif
                if (err == NoError) {
                    if (elementdatabaseInfo.databaseUnId != homedatabaseInfo.databaseUnId) {
                        selNeigs.PushNew (guid);
                        GS::UniString name = GetDBName (elementdatabaseInfo);
                        msg_rep ("ShowSubelement", "Diff DB: " + pname + " <-> " + name, err, guid);
                        count_otherplan++;
                        continue;
                    }
                } else {
                    selNeigs.PushNew (guid);
                    count_otherplan++;
                    msg_rep ("ShowSubelement", "APIDb_GetCurrentDatabaseID", err, guid);
                    continue;
                }
            }
            selNeigs.PushNew (guid);
        }
    }
    fmane = fmane + GS::UniString::Printf (": %d total elements find", count_all);
    GS::UniString errmsg = "";

    if (count_otherplan > 0) {
        GS::UniString SubElementOtherPlanString = RSGetIndString (ID_ADDON_STRINGS + bisEng, SubElementOtherPlanId, ACAPI_GetOwnResModule ());
        errmsg = errmsg + GS::UniString::Printf (" %d ", count_otherplan) + SubElementOtherPlanString + "\n";
        fmane = fmane + GS::UniString::Printf (", %d on other floorplan", count_otherplan);
    }

    if (count_del > 0) {
        GS::UniString SubElementNotExsistString = RSGetIndString (ID_ADDON_STRINGS + bisEng, SubElementNotExsistId, ACAPI_GetOwnResModule ());
        errmsg = errmsg + GS::UniString::Printf (" %d ", count_del) + SubElementNotExsistString + "\n";
        fmane = fmane + GS::UniString::Printf (", %d not exsist", count_del);
    }
    if (count_inv > 0) {
        GS::UniString SubElementHiddenString = RSGetIndString (ID_ADDON_STRINGS + bisEng, SubElementHiddenId, ACAPI_GetOwnResModule ());
        errmsg = errmsg + GS::UniString::Printf (" %d ", count_inv) + SubElementHiddenString + "\n";
        fmane = fmane + GS::UniString::Printf (", %d on hidden layers/filters", count_inv);
    }
    if (!errmsg.IsEmpty ()) {
        GS::UniString SubElementTotalString = RSGetIndString (ID_ADDON_STRINGS + bisEng, SubElementTotalId, ACAPI_GetOwnResModule ());
        errmsg = SubElementTotalString + GS::UniString::Printf (" %d, ", count_all) + "\n" + errmsg;
    }
    if (selNeigs.IsEmpty ()) {
        GS::UniString SubElementNoSelectString = RSGetIndString (ID_ADDON_STRINGS + bisEng, SubElementNoSelectId, ACAPI_GetOwnResModule ());
        errmsg = SubElementNoSelectString + "\n" + errmsg;
        fmane = fmane + " Nothing to select - all hide!";
        finish = clock ();
        duration = (double) (finish - start) / CLOCKS_PER_SEC;
        GS::UniString time = GS::UniString::Printf (" %.3f s", duration);
        msg_rep (fmane, time, err, APINULLGuid);
        if (!errmsg.IsEmpty ()) ACAPI_WriteReport (errmsg, true);
        return;
    }
    #if defined(AC_27) || defined(AC_28) || defined(AC_29)
    err = ACAPI_Selection_Select (selNeigs, true);
    if (err == NoError && errmsg.IsEmpty ()) ACAPI_View_ZoomToSelected ();
    #else
    err = ACAPI_Element_Select (selNeigs, true);
    if (err == NoError && errmsg.IsEmpty ()) ACAPI_Automate (APIDo_ZoomToSelectedID);
    #endif
    if (!errmsg.IsEmpty ()) {
        GS::UniString SubElementHalfString = RSGetIndString (ID_ADDON_STRINGS + bisEng, SubElementHalfId, ACAPI_GetOwnResModule ());
        errmsg = SubElementHalfString + "\n" + errmsg;
        ACAPI_WriteReport (errmsg, true);
    }
    #else
    fmane = fmane + " not work in AC22";
    ACAPI_WriteReport ("Function not work in AC22", true);
    #endif
    finish = clock ();
    duration = (double) (finish - start) / CLOCKS_PER_SEC;
    GS::UniString time = GS::UniString::Printf (" %.3f s", duration);
    msg_rep (fmane, time, err, APINULLGuid);
    return;
}

// --------------------------------------------------------------------
// Получение словаря с GUID дочерних объектов для массива объектов
// --------------------------------------------------------------------
bool SyncGetParentelement (const GS::Array<API_Guid>& guidArray, UnicGuidByGuid& parentGuid, const GS::UniString& suffix, int& errcode)
{
    #ifdef AC_22
    return false;
    #else
    GSErrCode err = NoError;
    if (guidArray.IsEmpty ()) return false;
    if (!ParamHelpers::isPropertyDefinitionRead ()) return false;
    ParamDictValue& propertyParams = PROPERTYCACHE ().property;
    GS::HashTable<API_Guid, GS::Array<API_Guid>> classificationforread; //классификация найденных элементов
    for (auto& cItt : propertyParams) {
        #if defined(AC_28) || defined(AC_29)
        ParamValue param = cItt.value;
        #else
        ParamValue param = *cItt.value;
        #endif
        if (!param.definition.description.Contains ("Sync_GUID")) continue;
        if (!(suffix.IsEmpty () || param.definition.description.Contains (suffix))) continue;
        if (param.definition.valueType != API_PropertyStringValueType) continue;
        for (const auto& cls : param.definition.availability) {
            if (!classificationforread.ContainsKey (cls)) {
                GS::Array<API_Guid> un;
                classificationforread.Add (cls, un);
            }
            if (classificationforread.ContainsKey (cls)) {
                classificationforread.Get (cls).Push (param.definition.guid);
            }
        }
    }
    if (classificationforread.IsEmpty ()) {
        errcode = 1;
        return false;
    }
    // Переписываем родительские GUID
    for (const auto& guid : guidArray) {
        if (!parentGuid.ContainsKey (guid)) {
            UnicGuid un;
            parentGuid.Add (guid, un);
        }
    }
    bool find = false;
    ParamDictElement paramToRead;
    for (const auto& cls : classificationforread) {
        #if defined(AC_28) || defined(AC_29)
        const API_Guid& classificationItemGuid = cls.key;
        const GS::Array<API_Guid>& propertyDefinitions = cls.value;
        #else
        const API_Guid& classificationItemGuid = *cls.key;
        const GS::Array<API_Guid>& propertyDefinitions = *cls.value;
        #endif
        GS::Array<API_Guid> elemGuids;
        err = ACAPI_Element_GetElementsWithClassification (classificationItemGuid, elemGuids);
        if (err != NoError) {
            msg_rep ("SyncGetParentelement", "ACAPI_Element_GetElementsWithClassification", err, classificationItemGuid);
            continue;
        }
        if (elemGuids.IsEmpty ()) continue;
        for (UInt32 i = 0; i < elemGuids.GetSize (); i++) {
            API_Guid subguid = elemGuids.Get (i);
            GS::Array<API_Property> properties;
            err = ACAPI_Element_GetPropertyValuesByGuid (subguid, propertyDefinitions, properties);
            if (err != NoError) {
                msg_rep ("SyncGetParentelement", "ACAPI_Element_GetPropertyValuesByGuid", err, subguid);
                continue;
            }
            for (const auto& prop : properties) {
                #if defined(AC_22) || defined(AC_23)
                if (!prop.isEvaluated) continue;
                #else
                if (prop.status != API_Property_HasValue) continue;
                #endif
                if (prop.isDefault) continue;
                if (prop.value.singleVariant.variant.uniStringValue.IsEmpty ()) continue;
                GS::Array<GS::UniString> rulestring_param;
                UInt32 nrule_param = StringSplt (prop.value.singleVariant.variant.uniStringValue, ";", rulestring_param);
                if (nrule_param > 0) {
                    for (UInt32 i = 0; i < nrule_param; i++) {
                        API_Guid guid = APIGuidFromString (rulestring_param[i].ToCStr (0, MaxUSize, GChCode));
                        if (guid != APINULLGuid && parentGuid.ContainsKey (guid)) {
                            find = true;
                            if (!parentGuid.Get (guid).ContainsKey (subguid)) {
                                bool isvisible = true;
                                if (!ACAPI_Element_Filter (subguid, APIFilt_OnVisLayer)) { isvisible = false; } else {
                                    if (!ACAPI_Element_Filter (subguid, APIFilt_IsVisibleByRenovation)) { isvisible = false; } else {
                                        if (!ACAPI_Element_Filter (subguid, APIFilt_IsInStructureDisplay)) isvisible = false;
                                    }
                                }
                                parentGuid.Get (guid).Add (subguid, isvisible);
                            }
                        }
                    }
                }
            }
        }
    }
    if (!find) {
        parentGuid.Clear ();
        errcode = 2;
    }
    return find;
    #endif
}


// --------------------------------------------------------------------
// Получение словаря с GUID родительских объектов для массива объектов
// --------------------------------------------------------------------
bool SyncGetSubelement (const GS::Array<API_Guid>& guidArray, UnicGuidByGuid& parentGuid, const GS::UniString& suffix, int& errcode)
{
    if (guidArray.IsEmpty ()) return false;
    ParamDictElement paramToRead = {};
    if (!SyncGetSyncGUIDProperty (guidArray, paramToRead, suffix)) {
        errcode = 1;
        return false;
    }
    for (auto& cIt : paramToRead) {
        #if defined(AC_28) || defined(AC_29)
        API_Guid subguid = cIt.key;
        ParamDictValue params = cIt.value;
        #else
        API_Guid subguid = *cIt.key;
        ParamDictValue params = *cIt.value;
        #endif
        for (auto& cItt : params) {
            #if defined(AC_28) || defined(AC_29)
            ParamValue param = cItt.value;
            #else
            ParamValue param = *cItt.value;
            #endif
            if (param.isValid && !param.val.uniStringValue.IsEmpty ()) {
                GS::Array<GS::UniString> rulestring_param = {};
                UInt32 nrule_param = StringSplt (param.val.uniStringValue, ";", rulestring_param);
                if (nrule_param > 0) {
                    for (UInt32 i = 0; i < nrule_param; i++) {
                        API_Guid guid = APIGuidFromString (rulestring_param[i].ToCStr (0, MaxUSize, GChCode));
                        if (guid != APINULLGuid) {
                            if (!parentGuid.ContainsKey (subguid)) {
                                UnicGuid un = {};
                                parentGuid.Add (subguid, un);
                            }
                            if (parentGuid.ContainsKey (subguid)) {
                                if (!parentGuid.Get (subguid).ContainsKey (guid)) {
                                    bool isvisible = true;
                                    if (!ACAPI_Element_Filter (guid, APIFilt_OnVisLayer)) { isvisible = false; } else {
                                        if (!ACAPI_Element_Filter (guid, APIFilt_IsVisibleByRenovation)) { isvisible = false; } else {
                                            if (!ACAPI_Element_Filter (guid, APIFilt_IsInStructureDisplay)) isvisible = false;
                                        }
                                    }
                                    parentGuid.Get (subguid).Add (guid, isvisible);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    if (parentGuid.IsEmpty ()) errcode = 2;
    return !parentGuid.IsEmpty ();
}

// --------------------------------------------------------------------
// Получение прочитанных свойств Sync_GUID для массива элементов
// --------------------------------------------------------------------
bool SyncGetSyncGUIDProperty (const GS::Array<API_Guid>& guidArray, ParamDictElement& paramToRead, const GS::UniString& suffix)
{
    ParamDictValue paramDict = {};
    for (auto& cItt : PROPERTYCACHE ().property) {
        #if defined(AC_28) || defined(AC_29)
        ParamValue param = cItt.value;
        #else
        ParamValue param = *cItt.value;
        #endif
        if (param.definition.description.Contains ("Sync_GUID") && (suffix.IsEmpty () || param.definition.description.Contains (suffix))) {
            if (!paramDict.ContainsKey (param.rawName)) {
                for (UInt32 i = 0; i < guidArray.GetSize (); i++) {
                    if (ACAPI_Element_IsPropertyDefinitionAvailable (guidArray[i], param.definition.guid)) {
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
    if (paramDict.IsEmpty ()) return false;
    for (UInt32 i = 0; i < guidArray.GetSize (); i++) {
        ParamHelpers::AddParamDictValue2ParamDictElement (guidArray[i], paramDict, paramToRead);
    }
    ParamHelpers::ElementsRead (paramToRead);
    return !paramToRead.IsEmpty ();
}
