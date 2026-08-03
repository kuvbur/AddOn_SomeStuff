//------------ kuvbur 2022 ------------
#include "ACAPinc.h"
#include "APIEnvir.h"
#include "Dimensions.hpp"
#include "MEPv1.hpp"
#include "Propertycache.hpp"
#include "ResetProperty.hpp"
#include "Sync.hpp"
#include <string> // std::stoi
#include <time.h>
#ifdef TESTING
    #include "TestFunc.hpp"
#endif
#if defined(AC_28)
    #include <ACAPI/MEPAdapter.hpp>
#endif
Int32 nLib = 0;

#include <chrono>
// Кэш хранения времени последней синхронизации элемента
static GS::HashTable<API_Guid, std::chrono::steady_clock::time_point> g_ElementSyncCache;

// Окно тишины (200-300 мс обычно хватает с запасом, чтобы склеить дубликаты событий)
const std::chrono::milliseconds SYNC_THROTTLE_THRESHOLD (500);

bool IsElementThrottled (const API_Guid &guid) {
    auto now = std::chrono::steady_clock::now ();
    if (g_ElementSyncCache.GetSize () > 5000) {
        GS::Array<API_Guid> expiredGuids;
        g_ElementSyncCache.Enumerate ([&] (const API_Guid &k, const std::chrono::steady_clock::time_point &v) {
            if (now - v > SYNC_THROTTLE_THRESHOLD) {
                expiredGuids.Push (k);
            }
        });
        for (const auto &expiredGuid : expiredGuids) {
            g_ElementSyncCache.Delete (expiredGuid);
        }
        if (g_ElementSyncCache.GetSize () > 5000) {
            g_ElementSyncCache.Clear ();
        }
    }
    if (std::chrono::steady_clock::time_point *lastTime = g_ElementSyncCache.GetPtr (guid)) {
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds> (now - *lastTime);
        if (elapsed < SYNC_THROTTLE_THRESHOLD) {
#if defined(TESTING)
            DBprnt ("!IsElementThrottled on!");
#endif
            return true; // Элемент обрабатывался совсем недавно, игнорируем спам
        }
        *lastTime = now; // Обновляем время для следующего легитимного изменения
        return false;
    }
    g_ElementSyncCache.Put (guid, now);
    return false;
}

// -----------------------------------------------------------------------------
// Подключение мониторинга
// -----------------------------------------------------------------------------
void MonAll (SyncSettings &syncSettings) {
    if (!syncSettings.syncMon)
        return;
#if defined(TESTING)
    DBprnt ("MonAll start");
#endif
    clock_t start, finish;
    double duration = 0;
    start = clock ();
    GS::UniString funcname ("Monitor All");
    GS::Int32 nPhase = 1;
    ProcessWindowGuard pwGuard (funcname, nPhase);
    static const API_ElemTypeID monTypes[] = {API_ObjectID,
                                              API_WindowID,
                                              API_DoorID,
                                              API_ZoneID,
                                              API_WallID,
                                              API_SlabID,
                                              API_ColumnID,
                                              API_BeamID,
                                              API_RoofID,
                                              API_MeshID,
                                              API_MorphID,
                                              API_CurtainWallID,
                                              API_RailingID};
    for (const auto &type : monTypes) {
        if (!MonByType (type, syncSettings))
            return;
#if defined(AC_27) || defined(AC_28) || defined(AC_29)
        if (ACAPI_ProcessWindow_IsProcessCanceled ())
            return;
#else
        if (ACAPI_Interface (APIIo_IsProcessCanceledID, nullptr, nullptr))
            return;
#endif
    }
#if defined(AC_28)
    MonByType (API_ExternalElemID, syncSettings);
#endif
    if (PROPERTYCACHE ().hasDimAutotext)
        MonByType (API_DimensionID, syncSettings);
    finish = clock ();
    duration = (double)(finish - start) / CLOCKS_PER_SEC;
    GS::UniString time = GS::UniString::Printf (" %.3f s", duration);
    msg_rep ("MonAll", time, NoError, APINULLGuid);
#if defined(TESTING)
    DBprnt ("MonAll end");
#endif
}

// -----------------------------------------------------------------------------
// Подключение мониторинга по типам
// -----------------------------------------------------------------------------
bool MonByType (const API_ElemTypeID &elementType, const SyncSettings &syncSettings) {
    GS::Array<API_Guid> guidArray;
#if defined(TESTING)
    DBprnt ("MonByType");
#endif
    GSErrCode err = ACAPI_Element_GetElemList (
        elementType, &guidArray, APIFilt_IsEditable | APIFilt_HasAccessRight | APIFilt_InMyWorkspace);
    if (err != NoError || guidArray.IsEmpty ())
        return true;
    GS::Array<API_Guid> subelemGuids = {};
    for (const auto &guid : guidArray) {
        err = AttachObserver (guid, syncSettings);
        if (err == APIERR_LINKEXIST)
            err = NoError;
        if (err != NoError) {
            msg_rep ("MonByType", "AttachObserver", err, guid);
            continue;
        }
#if defined(AC_27) || defined(AC_28) || defined(AC_29)
        if (ACAPI_ProcessWindow_IsProcessCanceled ())
            return false;
#else
        if (ACAPI_Interface (APIIo_IsProcessCanceledID, nullptr, nullptr))
            return false;
#endif
        // Получаем список связанных элементов
        if (!syncSettings.cwallS)
            continue;
        subelemGuids.Clear ();
        GetRelationsElement (guid, elementType, syncSettings, subelemGuids, true, true);
        for (const auto &subelemGuid : subelemGuids) {
            err = AttachObserver (subelemGuid, syncSettings);
            if (err == APIERR_LINKEXIST)
                err = NoError;
            if (err != NoError) {
                msg_rep ("MonByType", "AttachObserver", err, subelemGuid);
                continue;
            }
#if defined(AC_27) || defined(AC_28) || defined(AC_29)
            if (ACAPI_ProcessWindow_IsProcessCanceled ())
                return false;
#else
            if (ACAPI_Interface (APIIo_IsProcessCanceledID, nullptr, nullptr))
                return false;
#endif
        }
    }
    return true;
}

// -----------------------------------------------------------------------------
// Запускает обработку всех объектов, заданных в настройке
// -----------------------------------------------------------------------------
void SyncAndMonAll (SyncSettings &syncSettings) {
#if defined(TESTING)
    DBprnt ("SyncAndMonAll start");
#endif
    if (ResetProperty ())
        return;
    GS::UniString funcname ("Sync All");
    bool flag_chanel = false;
    ParamDictElement paramToWrite = {};
    GS::Int32 nPhase = 1;
    ProcessWindowGuard pwGuard (funcname, nPhase);
    clock_t start, finish;
    double duration = 0;
    start = clock ();
    int dummymode = IsDummyModeOn ();
    UnicGuid syncedelem = {};
    if (!flag_chanel && syncSettings.objS) {
        static const API_ElemTypeID objTypes[] = {
            API_ObjectID, API_LampID, API_StairID, API_RiserID, API_TreadID, API_StairStructureID, API_ZoneID};
        for (const auto &type : objTypes) {
            nPhase = nPhase + 1;
            if (SyncByType (type, syncSettings, nPhase, paramToWrite, dummymode, syncedelem))
                flag_chanel = true;
            if (flag_chanel)
                break;
        }
    }
    if (!flag_chanel && syncSettings.wallS) {
        static const API_ElemTypeID wallTypes[] = {
            API_WallID, API_SlabID, API_ColumnID, API_BeamID, API_RoofID, API_ShellID, API_MorphID};
        for (const auto &type : wallTypes) {
            nPhase = nPhase + 1;
            if (SyncByType (type, syncSettings, nPhase, paramToWrite, dummymode, syncedelem))
                flag_chanel = true;
            if (flag_chanel)
                break;
        }
    }
    if (!flag_chanel && syncSettings.widoS) {
        static const API_ElemTypeID widoTypes[] = {API_WindowID, API_DoorID, API_SkylightID};
        for (const auto &type : widoTypes) {
            nPhase = nPhase + 1;
            if (SyncByType (type, syncSettings, nPhase, paramToWrite, dummymode, syncedelem))
                flag_chanel = true;
            if (flag_chanel)
                break;
        }
    }
    if (!flag_chanel && syncSettings.cwallS) {
        static const API_ElemTypeID cwallTypes[] = {API_RailingID, API_CurtainWallID};
        for (const auto &type : cwallTypes) {
            nPhase = nPhase + 1;
            if (SyncByType (type, syncSettings, nPhase, paramToWrite, dummymode, syncedelem))
                flag_chanel = true;
            if (flag_chanel)
                break;
        }
    }
    if (!flag_chanel && PROPERTYCACHE ().hasDimAutotext)
        flag_chanel = SyncByType (API_DimensionID, syncSettings, nPhase, paramToWrite, dummymode, syncedelem);
#if defined(AC_28) || defined(AC_29)
    if (!flag_chanel && syncSettings.objS)
        flag_chanel = SyncByType (API_ExternalElemID, syncSettings, nPhase, paramToWrite, dummymode, syncedelem);
#endif
    finish = clock ();
    duration = (double)(finish - start) / CLOCKS_PER_SEC;
    GS::UniString time = GS::UniString::Printf (" %.3f s", duration);
    GS::UniString paramch = CountUnreadGDLParams ();
    msg_rep ("SyncAll - read", time + paramch, NoError, APINULLGuid);
    GS::Array<API_Guid> rereadelem = {};
    start = clock ();
    if (!paramToWrite.IsEmpty ()) {
        const Int32 iseng = ID_ADDON_STRINGS + isEng ();
        GS::UniString undoString = RSGetIndString (iseng, UndoSyncId, ACAPI_GetOwnResModule ());
        ACAPI_CallUndoableCommand (undoString, [&] () -> GSErrCode {
            GS::UniString title = GS::UniString::Printf ("Writing data to %d elements : ", paramToWrite.GetSize ());
            short i = 1;
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
            if (!suspGrp)
                ACAPI_Grouping_Tool (rereadelem, APITool_SuspendGroups, nullptr);
    #else
            ACAPI_Environment (APIEnv_IsSuspendGroupOnID, &suspGrp);
            if (!suspGrp) ACAPI_Element_Tool (rereadelem, APITool_SuspendGroups, nullptr);
    #endif
#endif
            rereadelem = ParamHelpers::ElementsWrite (paramToWrite);
            finish = clock ();
            duration = (double)(finish - start) / CLOCKS_PER_SEC;
            GS::UniString time = title + GS::UniString::Printf (" %.3f s", duration);
            GS::UniString paramch = CountUnreadGDLParams ();
            msg_rep ("SyncAll - write", time + paramch, NoError, APINULLGuid);
            return NoError;
        });
    } else {
        GS::UniString paramch = CountUnreadGDLParams ();
        msg_rep ("SyncAll - write", "No data to write " + paramch, NoError, APINULLGuid);
    }
    ParamHelpers::WriteInfo (paramToWrite);
    if (!rereadelem.IsEmpty ()) {
#if defined(TESTING)
        DBprnt ("===== REREAD =======");
#endif
        SyncArray (syncSettings, rereadelem);
    }
#if defined(TESTING)
    DBprnt ("SyncAndMonAll end");
#endif
}

// -----------------------------------------------------------------------------
// Синхронизация элементов по типу
// -----------------------------------------------------------------------------
bool SyncByType (const API_ElemTypeID &elementType,
                 const SyncSettings &syncSettings,
                 GS::Int32 &nPhase,
                 ParamDictElement &paramToWrite,
                 int dummymode,
                 UnicGuid &syncedelem) {
#if defined(TESTING)
    DBprnt ("    SyncByType start");
#endif
    GS::UniString subtitle = "";
    GSErrCode err = NoError;
    GS::Array<API_Guid> guidArray = {};
    clock_t start, finish;
    double duration = 0;
    start = clock ();
    ACAPI_Element_GetElemList (
        elementType, &guidArray, APIFilt_IsEditable | APIFilt_HasAccessRight | APIFilt_InMyWorkspace);
#if defined(AC_28) || defined(AC_29)
    if (elementType == API_ExternalElemID) {
        MEPv1::ClearRoutingSubelemCache ();
        guidArray = ACAPI::MEP::CollectAllMEPElements ();
    }
#endif
    if (guidArray.IsEmpty ())
        return false;
// #ifdef TESTING
// TestFunc::ResetSyncPropertyArray (guidArray);
// #endif
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
    ACAPI_Goodies (APIAny_GetElemTypeNameID, (void *)elementType, &subtitle);
#endif // AC_26
    GS::UniString subtitle_ =
        GS::UniString::Printf ("Reading data from %d elements : ", guidArray.GetSize ()) + subtitle;
    bool flag_chanel = false;
    for (UInt32 i = 0; i < guidArray.GetSize (); i++) {
        SyncElement (guidArray[i], syncSettings, paramToWrite, dummymode, syncedelem);
#if defined(AC_27) || defined(AC_28) || defined(AC_29)
        if (i % 10 == 0)
            ACAPI_ProcessWindow_SetNextProcessPhase (&subtitle, &maxval, &showPercent);
#else
        if (i % 10 == 0)
            ACAPI_Interface (APIIo_SetNextProcessPhaseID, &subtitle, &i);
#endif
#if defined(AC_27) || defined(AC_28) || defined(AC_29)
        if (ACAPI_ProcessWindow_IsProcessCanceled ())
            return true;
#else
        if (ACAPI_Interface (APIIo_IsProcessCanceledID, nullptr, nullptr))
            return true;
#endif
    }
    GS::UniString intString = GS::UniString::Printf (" %d qty", guidArray.GetSize ());
    finish = clock ();
    duration = (double)(finish - start) / CLOCKS_PER_SEC;
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
bool SyncElement (const API_Guid &elemGuid,
                  const SyncSettings &syncSettings,
                  ParamDictElement &paramToWrite,
                  int dummymode) {
    UnicGuid syncedelem = {};
    return SyncElement (elemGuid, syncSettings, paramToWrite, dummymode, syncedelem);
}

bool SyncElement (const API_Guid &elemGuid,
                  const SyncSettings &syncSettings,
                  ParamDictElement &paramToWrite,
                  int dummymode,
                  UnicGuid &syncedelem) {
    if (syncedelem.ContainsKey (elemGuid))
        return false; // Элемент уже был синхронизирован
    API_ElemTypeID elementType = API_ZombieElemID;
    GSErrCode err = GetTypeByGUID (elemGuid, elementType);
    if (err != NoError)
        return false;
    API_Guid elemGuid_ = elemGuid;
    if (elementType == API_SectElemID) {
        GetParentGUIDSectElem (elemGuid, elemGuid_, elementType);
        if (syncedelem.ContainsKey (elemGuid_))
            return false; // Элемент уже был синхронизирован
    }
    // Получаем список связанных элементов
    GS::Array<API_Guid> subelemGuids;
    GetRelationsElement (elemGuid_, elementType, syncSettings, subelemGuids, false, true);
    bool needResync = SyncData (elemGuid_, syncSettings, subelemGuids, paramToWrite, dummymode);
    syncedelem.Put (elemGuid_, needResync);

    if (subelemGuids.IsEmpty ())
        return needResync;
    if (!SyncRelationsElement (elementType, syncSettings))
        return needResync;

    GS::Array<API_Guid> epm;
    for (const auto &subelemGuid : subelemGuids) {
        if (subelemGuid == elemGuid_)
            continue;
        if (syncedelem.ContainsKey (subelemGuid))
            continue;
        if (SyncData (subelemGuid, syncSettings, epm, paramToWrite, dummymode))
            needResync = true;
        syncedelem.Put (subelemGuid, needResync);
    }
    return needResync;
}

// -----------------------------------------------------------------------------
// Запускает обработку выбранных, заданных в настройке
// -----------------------------------------------------------------------------
void SyncSelected (const SyncSettings &syncSettings) {
    GS::UniString fmane = "Sync Selected";
    GS::Array<API_Guid> guidArray = GetSelectedElements (false, true, syncSettings, false, false, false);
    if (guidArray.IsEmpty ())
        return;
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
GS::Array<API_Guid> SyncArray (const SyncSettings &syncSettings, GS::Array<API_Guid> &guidArray) {
    GS::Array<API_Guid> rereadelem = {};
    if (guidArray.IsEmpty ())
        return rereadelem;
    GS::UniString funcname = "Sync Selected";
    ParamDictElement paramToWrite = {};
    GS::UniString subtitle = GS::UniString::Printf ("Reading data from %d elements", guidArray.GetSize ());
    GS::Int32 nPhase = 1;
    int dummymode = IsDummyModeOn ();
#if defined(AC_28) || defined(AC_29)
    MEPv1::ClearRoutingSubelemCache ();
#endif
#if defined(AC_27) || defined(AC_28) || defined(AC_29)
    bool showPercent = true;
    Int32 maxval = guidArray.GetSize ();
#endif
    ProcessWindowGuard pwGuard (funcname, nPhase);
    clock_t start, finish;
    double duration = 0;
    start = clock ();
    bool needResync = false;
    UnicGuid syncedelem = {};
    for (UInt32 i = 0; i < guidArray.GetSize (); i++) {
        if (SyncElement (guidArray[i], syncSettings, paramToWrite, dummymode, syncedelem))
            rereadelem.Push (guidArray[i]);
#if defined(AC_27) || defined(AC_28) || defined(AC_29)
        if (i % 10 == 0)
            ACAPI_ProcessWindow_SetNextProcessPhase (&subtitle, &maxval, &showPercent);
#else
        if (i % 10 == 0)
            ACAPI_Interface (APIIo_SetNextProcessPhaseID, &subtitle, &i);
#endif
#if defined(AC_27) || defined(AC_28) || defined(AC_29)
        if (ACAPI_ProcessWindow_IsProcessCanceled ()) {
#else
        if (ACAPI_Interface (APIIo_IsProcessCanceledID, nullptr, nullptr)) {
#endif
            return rereadelem;
        }
        // Если включён мониторинг - привязываем элемент к отслеживанию
        if (syncSettings.syncMon)
            AttachObserver (guidArray[i], syncSettings);
    }
    GS::UniString intString = GS::UniString::Printf (" %d qty", guidArray.GetSize ());
    finish = clock ();
    duration = (double)(finish - start) / CLOCKS_PER_SEC;
    GS::UniString time = GS::UniString::Printf (" %.3f s", duration);
    GS::UniString paramch = CountUnreadGDLParams ();
    msg_rep ("SyncSelected - read", subtitle + intString + paramch + time, NoError, APINULLGuid);
    if (!paramToWrite.IsEmpty ()) {
        const Int32 iseng = ID_ADDON_STRINGS + isEng ();
        GS::UniString undoString = RSGetIndString (iseng, UndoSyncId, ACAPI_GetOwnResModule ());
        ACAPI_CallUndoableCommand (undoString, [&] () -> GSErrCode {
            start = clock ();
            GS::UniString title = GS::UniString::Printf ("Writing data to %d elements : ", paramToWrite.GetSize ());
            short i = 1;
#if defined(AC_27) || defined(AC_28) || defined(AC_29)
            ACAPI_ProcessWindow_SetNextProcessPhase (&subtitle, &maxval, &showPercent);
#else
            ACAPI_Interface (APIIo_SetNextProcessPhaseID, &subtitle, &i);
#endif
            GS::Array<API_Guid> rereadelem_ = ParamHelpers::ElementsWrite (paramToWrite);
            if (!rereadelem_.IsEmpty ())
                rereadelem.Append (rereadelem_);
            finish = clock ();
            duration = (double)(finish - start) / CLOCKS_PER_SEC;
            GS::UniString time = title + GS::UniString::Printf (" %.3f s", duration);
            msg_rep ("SyncSelected - write", time, NoError, APINULLGuid);
            return NoError;
        });
    } else {
        msg_rep ("SyncSelected - write", "No data to write", NoError, APINULLGuid);
    }
    ParamHelpers::WriteInfo (paramToWrite);
    return rereadelem;
}

// -----------------------------------------------------------------------------
// Запуск скрипта параметров выбранных элементов
// -----------------------------------------------------------------------------
void RunParamSelected (const SyncSettings &syncSettings) {
    clock_t start, finish;
    double duration = 0;
    start = clock ();
    GS::UniString fmane = "Run parameter script";
    // Запомним номер текущей БД и комбинацию слоёв для восстановления по окончанию работы
    API_AttributeIndex layerCombIndex = {};
    API_DatabaseInfo databaseInfo = {};
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
    if (layerCombIndex.IsPositive ())
        err = ACAPI_Navigator_ChangeCurrLayerComb (&layerCombIndex); // Устанавливаем комбинацию слоёв
    err = ACAPI_Database_ChangeCurrentDatabase (&databaseInfo);
#else
    if (layerCombIndex != 0)
        err = ACAPI_Environment (APIEnv_ChangeCurrLayerCombID, &layerCombIndex); // Устанавливаем комбинацию слоёв
    err = ACAPI_Database (APIDb_ChangeCurrentDatabaseID, &databaseInfo, nullptr);
#endif
    finish = clock ();
    duration = (double)(finish - start) / CLOCKS_PER_SEC;
    GS::UniString time = GS::UniString::Printf (" %.3f s", duration);
    msg_rep (fmane, time, err, APINULLGuid);
}

// -----------------------------------------------------------------------------
// Запуск скрипта параметра элемента
// -----------------------------------------------------------------------------
void RunParam (const API_Guid &elemGuid, const SyncSettings &syncSettings) {
#if defined(TESTING)
    DBprnt ("RunParam");
#endif
    API_Elem_Head tElemHead = {};
    tElemHead.guid = elemGuid;
    GSErrCode err = ACAPI_Element_GetHeader (&tElemHead);
    if (err != NoError)
        return;
    API_DatabaseInfo databaseInfo;
    API_DatabaseInfo dbInfo;
#if defined(AC_27) || defined(AC_28) || defined(AC_29)
    err = ACAPI_Database_GetContainingDatabase (&tElemHead.guid, &dbInfo);
#else
    err = ACAPI_Database (APIDb_GetContainingDatabaseID, &tElemHead.guid, &dbInfo);
#endif
    if (err != NoError)
        return;
#if defined(AC_27) || defined(AC_28) || defined(AC_29)
    err = ACAPI_Database_GetCurrentDatabase (&databaseInfo);
#else
    err = ACAPI_Database (APIDb_GetCurrentDatabaseID, &databaseInfo, nullptr);
#endif
    if (err != NoError)
        return;
    if (dbInfo.databaseUnId != databaseInfo.databaseUnId) {
#if defined(AC_27) || defined(AC_28) || defined(AC_29)
        err = ACAPI_Database_ChangeCurrentDatabase (&dbInfo);
#else
        err = ACAPI_Database (APIDb_ChangeCurrentDatabaseID, &dbInfo, nullptr);
#endif
        if (err != NoError)
            return;
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
bool SyncRelationsElement (const API_ElemTypeID &elementType, const SyncSettings &syncSettings) {
    bool flag_sync = false;
    switch (elementType) {
    case API_WindowID:
    case API_DoorID:
        if (syncSettings.widoS)
            flag_sync = true;
        break;
    case API_CurtainWallSegmentID:
    case API_CurtainWallFrameID:
    case API_CurtainWallJunctionID:
    case API_CurtainWallAccessoryID:
    case API_CurtainWallPanelID:
    case API_CurtainWallID:
        if (syncSettings.cwallS)
            flag_sync = true;
        break;
    default:
        if (syncSettings.wallS)
            flag_sync = true;
        break;
    }
    return flag_sync;
}

// --------------------------------------------------------------------
// Синхронизация данных элемента согласно указаниям в описании свойств
// --------------------------------------------------------------------
bool SyncData (const API_Guid &elemGuid,
               const SyncSettings &syncSettings,
               GS::Array<API_Guid> &subelemGuids,
               ParamDictElement &paramToWrite,
               int dummymode) {
    auto &cache = PROPERTYCACHE ();
    GSErrCode err = NoError;
    GS::Array<API_PropertyDefinition> definitions = {};
    GS::Array<WriteData> mainsyncRules = {};
    ParamDictValue subproperty = {};
    ParamDictElement paramToRead = {}; // Словарь с параметрами для чтения
    UnicGuidString property_write_guid = {}; // Словарь GUID свойств, в которые могла быть осуществлена запись
    WriteDict syncRules = {}; // Словарь с правилами для каждого элемента
    API_ElemTypeID elementType;
    if (!IsElementEditable (elemGuid, syncSettings, true, elementType))
        return false;
    ClassificationFunc::SetAutoclass (elemGuid);
    err = ACAPI_Element_GetPropertyDefinitions (elemGuid, API_PropertyDefinitionFilter_UserDefined, definitions);
    if (err != NoError) {
        msg_rep ("SyncData", "ACAPI_Element_GetPropertyDefinitions", err, elemGuid);
        return false;
    }
    if (definitions.IsEmpty ())
        return false;
    // Синхронизация данных
    // Проверяем - не отключена ли синхронизация у данного объекта
    if (dummymode == DUMMY_MODE_UNDEF)
        dummymode = IsDummyModeOn ();
    bool syncall = true;
    bool flagfindall = true;
    bool synccoord = true;
    bool flagfindcoord = true;
    bool syncclass = true;
    bool flagfindclass = true;
    if (dummymode == DUMMY_MODE_ON) {
        syncall = GetElemStateReverse (elemGuid, definitions, SYNCFLAG, flagfindall);
        synccoord = GetElemStateReverse (elemGuid, definitions, SYNCCORRECTFLAG, flagfindcoord);
        syncclass = GetElemStateReverse (elemGuid, definitions, SYNCCLASSFLAG, flagfindclass);
    } else {
        syncall = GetElemState (elemGuid, definitions, SYNCFLAG, flagfindall, false);
        synccoord = GetElemState (elemGuid, definitions, SYNCCORRECTFLAG, flagfindcoord, false);
        syncclass = GetElemState (elemGuid, definitions, SYNCCLASSFLAG, flagfindclass, false);
    }
    if (!syncall && !synccoord && !syncclass)
        return false; // Если оба свойства-флага ложь - выходим
    if (syncall && !flagfindcoord)
        synccoord = true; // Если флаг координат не найден - проверку всё равно делаем
    if (syncall && !flagfindclass)
        syncclass = true;
    bool hassubguid = false;
    if (syncall)
        hassubguid = ParamHelpers::SubGuid_GetParamValue (elemGuid, definitions, subproperty);
    bool hasSub = false;

    for (auto &definition : definitions) {
        // Получаем список правил синхронизации из всех свойств
        ParseSyncString (elemGuid,
                         elementType,
                         definition,
                         mainsyncRules,
                         paramToRead,
                         hasSub,
                         syncall,
                         synccoord,
                         syncclass,
                         subproperty); // Парсим описание свойства
    }
    if (mainsyncRules.IsEmpty ())
        return false;
    if (!(cache.isPropertyDefinitionRead_full && cache.isPropertyDefinition_OK)) {
        cache.AddPropertyDefinition (definitions);
    }
    // Заполняем правила синхронизации с учётом субэлементов, попутно заполняем словарь параметров для чтения/записи
    SyncAddSubelement (subelemGuids, mainsyncRules, syncRules, paramToRead);
    mainsyncRules.Clear ();
    subelemGuids.Push (elemGuid); // Это теперь список всех элементов для синхронизации

    // Читаем все возможные свойства
    if (paramToRead.IsEmpty ())
        return false;
    ParamHelpers::ElementsRead (paramToRead);
    SyncCalcRule (syncRules, subelemGuids, paramToRead, paramToWrite, property_write_guid);
    if (paramToWrite.IsEmpty ())
        return false;
    // Некоторые свойства, возможно, ссылались на изменённые. Чтоб не запускать полную синхронизацию ещё раз
    ParamHelpers::CompareParamDictElement (paramToWrite, paramToRead);
    SyncCalcRule (syncRules, subelemGuids, paramToRead, paramToWrite, property_write_guid);
    return SyncNeedResync (paramToRead, property_write_guid);
}

bool SyncNeedResync (ParamDictElement &paramToRead, UnicGuidString property_write_guid) {
    if (property_write_guid.IsEmpty ())
        return false;
    if (paramToRead.IsEmpty ())
        return false;
    GS::Array<GS::UniString> partstring;
    GS::Array<GS::UniString> local_scratch;
    for (ParamDictElement::PairIterator cIt = paramToRead.EnumeratePairs (); cIt != NULL; ++cIt) {
#if defined(AC_28) || defined(AC_29)
        ParamDictValue &params = cIt->value;
        API_Guid elemGuid = cIt->key;
#else
        ParamDictValue &params = *cIt->value;
        API_Guid elemGuid = *cIt->key;
#endif
        if (params.IsEmpty ())
            continue;
        for (ParamDictValue::PairIterator cItt = params.EnumeratePairs (); cItt != NULL; ++cItt) {
#if defined(AC_28) || defined(AC_29)
            ParamValue &param = cItt->value;
#else
            ParamValue &param = *cItt->value;
#endif
            if (!param.definition.defaultValue.hasExpression)
                continue;
            if (property_write_guid.ContainsKey (param.definition.guid))
                continue;
            for (const GS::UniString &expr : param.definition.defaultValue.propertyExpressions) {
                if (!expr.Contains ("###Property:"))
                    continue;
                partstring.Clear ();
                if (StringSpltFilter (expr, "###", partstring, PROPERTYPREF, &local_scratch) > 0) {
                    for (const GS::UniString &part : partstring) {
                        GS::UniString h = part.GetSuffix (36);
                        API_Guid pguid = APIGuidFromString (h.ToCStr (0, MaxUSize, GChCode));
                        if (property_write_guid.ContainsKey (pguid)) {
                            msg_rep ("Find property in expression for resync",
                                     property_write_guid.Get (pguid) + " => " + param.rawName,
                                     NoError,
                                     elemGuid);
                            return true;
                        }
                    }
                }
            }
        }
    }
    return false;
}

void SyncCalcRule (const WriteDict &syncRules,
                   const GS::Array<API_Guid> &subelemGuids,
                   const ParamDictElement &paramToRead,
                   ParamDictElement &paramToWrite,
                   UnicGuidString &property_write_guid) {
    GS::HashSet<GS::UniString> resolvedProps; // Свойства, которые уже получили "полезное" значение
    GS::HashSet<GS::UniString> propsToReset; // Свойства, которые кандидаты на сброс к дефолту
    for (const API_Guid &elemGuid : subelemGuids) {
        const auto *writeSubs = syncRules.GetPtr (elemGuid);
        if (writeSubs == nullptr || writeSubs->IsEmpty ()) {
            continue;
        }
        resolvedProps.Clear ();
        propsToReset.Clear ();

        // Заполняем значения параметров чтения/записи из словаря правил
        for (const WriteData &writeSub : *writeSubs) {
            const GS::UniString &rawNameTo = writeSub.paramTo.rawName;

            if (resolvedProps.Contains (rawNameTo))
                continue; // для этого свойства уже найдено полезное значение
            const API_Guid &elemGuidTo = writeSub.guidTo;
            const API_Guid &elemGuidFrom = writeSub.guidFrom;

            const auto *paramsToPtr = paramToRead.GetPtr (elemGuidTo);
            if (paramsToPtr == nullptr)
                continue;

            const auto *paramToPtr = paramsToPtr->GetPtr (rawNameTo);
            if (paramToPtr == nullptr)
                continue;

            const auto *paramsFromPtr = (elemGuidFrom == elemGuidTo) ? paramsToPtr : paramToRead.GetPtr (elemGuidFrom);
            if (paramsFromPtr == nullptr)
                continue;

            const GS::UniString &rawNameFrom = writeSub.paramFrom.rawName;
            const auto *paramFromPtr = paramsFromPtr->GetPtr (rawNameFrom);
            if (paramFromPtr == nullptr)
                continue;

            ParamValue paramFrom = *paramFromPtr;
            ParamValue paramTo = *paramToPtr;
            FormatString formatstring = writeSub.formatstring;

            bool is_ignore = false;
            bool is_eq = false;

            if (ParamHelpers::CompareParamValue (
                    paramFrom, paramTo, formatstring, writeSub.ignorevals, is_ignore, is_eq)) {
                // СЛУЧАЙ 1: Найдено новое значение
                ParamHelpers::AddParamValue2ParamDictElement (paramTo, paramToWrite);

                if (paramTo.definition.guid != APINULLGuid) {
                    property_write_guid.Put (paramTo.definition.guid, paramTo.rawName);
                }
                resolvedProps.Add (rawNameTo);
                propsToReset.Delete (rawNameTo);
            } else if (is_eq && !is_ignore) {
                // СЛУЧАЙ 2: Значение совпадает с текущим и оно валидное не игнор
                resolvedProps.Add (rawNameTo);
                propsToReset.Delete (rawNameTo);
            } else if (is_ignore && writeSub.ignorevals.reset_to_def && paramTo.definition.guid != APINULLGuid) {
                // СЛУЧАЙ 3: Значение игнорируется, и правило требует сброса
                bool isDefault = false;
                if (paramTo.property.definition.guid == paramTo.definition.guid) {
                    isDefault = paramTo.property.isDefault;
                }
                if (!isDefault)
                    propsToReset.Add (rawNameTo);
            }
        }
        if (!propsToReset.IsEmpty ()) {
            if (const auto *pread = paramToRead.GetPtr (elemGuid)) {
                for (const GS::UniString &rawName : propsToReset) {
                    if (resolvedProps.Contains (rawName))
                        continue;
                    if (const auto *paramToPtr = pread->GetPtr (rawName)) {
                        ParamValue paramTo = *paramToPtr;
                        paramTo.needResetToDef = true;
                        paramTo.isValid = true;
                        ParamHelpers::AddParamValue2ParamDictElement (paramTo, paramToWrite);
                    }
                }
            }
        }
    }
}

// --------------------------------------------------------------------
// Добавление подэлементов и их параметров в правила синхорнизации
// --------------------------------------------------------------------
void SyncAddSubelement (const GS::Array<API_Guid> &subelemGuids,
                        GS::Array<WriteData> &mainsyncRules,
                        WriteDict &syncRules,
                        ParamDictElement &paramToRead) {
#if defined(TESTING)
    if (!subelemGuids.IsEmpty ()) {
        if (subelemGuids[0] == APINULLGuid) {
            DBprnt ("SyncAddSubelement err", "subelemGuid == APINULLGuid");
        }
    }
#endif
    for (auto &mainsyncRule : mainsyncRules) {
        if (!mainsyncRule.fromSub && !mainsyncRule.toSub) {
            SyncAddRule (mainsyncRule, syncRules, paramToRead);
        }

        // Если есть субэлементы - обработаем их
        if (subelemGuids.IsEmpty ())
            continue;

        // Для записи из дочернего в родительский возьмём только один, первый элемент
        if (mainsyncRule.fromSub) {
            mainsyncRule.fromSub = false;
            mainsyncRule.guidFrom = subelemGuids[0];
            mainsyncRule.paramFrom.fromGuid = subelemGuids[0];
            SyncAddRule (mainsyncRule, syncRules, paramToRead);
        }
        if (mainsyncRule.fromSub) {
            for (const API_Guid &elguid : subelemGuids) {
#if defined(TESTING)
                if (elguid == APINULLGuid) {
                    DBprnt ("SyncAddSubelement err", "elguid == APINULLGuid");
                }
#endif
                mainsyncRule.toSub = false;
                mainsyncRule.guidTo = elguid;
                mainsyncRule.paramTo.fromGuid = elguid;
                SyncAddRule (mainsyncRule, syncRules, paramToRead);
            }
        }
    }
}

// --------------------------------------------------------------------
// Запись правила в словарь, попутно заполняем словарь с параметрами
// --------------------------------------------------------------------
void SyncAddRule (const WriteData &writeSub, WriteDict &syncRules, ParamDictElement &paramToRead) {
    const API_Guid &elemGuid = writeSub.guidTo;
    if (GS::Array<WriteData> *rulesPtr = syncRules.GetPtr (elemGuid)) {
        rulesPtr->Push (writeSub);
    } else {
        GS::Array<WriteData> rules = {writeSub};
        syncRules.Put (elemGuid, std::move (rules));
    }
    ParamHelpers::AddParamValue2ParamDictElement (writeSub.paramFrom, paramToRead);
    ParamHelpers::AddParamValue2ParamDictElement (writeSub.paramTo, paramToRead);
#if defined(TESTING)
    if (elemGuid == APINULLGuid) {
        DBprnt ("SyncAddRule err", "elemGuid == APINULLGuid");
    }
    if (writeSub.guidFrom == APINULLGuid) {
        DBprnt ("SyncAddRule err", "writeSub.guidFrom == APINULLGuid");
    }
    if (writeSub.guidTo == APINULLGuid) {
        DBprnt ("SyncAddRule err", "writeSub.guidTo == APINULLGuid");
    }
    if (writeSub.paramFrom.fromGuid == APINULLGuid) {
        DBprnt ("SyncAddRule err", "writeSub.paramFrom.fromGuid == APINULLGuid");
    }
    if (writeSub.paramTo.fromGuid == APINULLGuid) {
        DBprnt ("SyncAddRule err", "writeSub.paramTo.fromGuid == APINULLGuid");
    }
#endif
}

// -----------------------------------------------------------------------------
// Парсит описание свойства, заполняет массив с правилами (GS::Array <WriteData>)
// -----------------------------------------------------------------------------
bool ParseSyncString (const API_Guid &elemGuid,
                      const API_ElemTypeID &elementType,
                      const API_PropertyDefinition &definition,
                      GS::Array<WriteData> &syncRules,
                      ParamDictElement &paramToRead,
                      bool &hasSub,
                      bool syncall,
                      bool synccoord,
                      bool syncclass,
                      ParamDictValue &subproperty) {
    GS::UniString description_string = definition.description;
    if (description_string.IsEmpty ()) {
        return false;
    }
    if (description_string.Contains (SYNCFLAG)) {
        return false;
    }
    if (description_string.Contains (SYNCCORRECTFLAG)) {
        ParamDictValue paramDict = {};
        ParamValue paramdef = {}; // Свойство, из которого получено правило
        ParamHelpers::ConvertToParamValue (paramdef, definition);
        paramdef.rawName = "{@property:sync_correct_flag}";
        paramdef.fromGuid = elemGuid;
        paramDict.Put (paramdef.rawName, std::move (paramdef));
        ParamHelpers::AddParamDictValue2ParamDictElement (elemGuid, paramDict, paramToRead);
        return true;
    }

// Если указан сброс данных - синхронизировать не будем
#ifndef AC_27
    if (description_string.Contains ("Sync_reset")) {
        return false;
    }
#endif

    if (!description_string.Contains (SYNCPART))
        return false;
    if (!description_string.Contains (BRACESTART))
        return false;
    if (!description_string.Contains (BRACEEND))
        return false;
    bool hasRule = false;
    UIndex length = description_string.GetLength ();
    GS::UniString dst;
    dst.SetCapacity (length);

    auto matchesStr = [&] (UIndex idx, const char *str, USize len) -> bool {
        if (idx + len > length)
            return false;
        for (USize j = 0; j < len; ++j) {
            if (static_cast<unsigned short> (description_string[idx + j]) != GS::UniChar (str[j]))
                return false;
        }
        return true;
    };

    auto matchesUStr = [&] (UIndex idx, const GS::UniString &uStr) -> bool {
        USize len = uStr.GetLength ();
        if (len == 0 || idx + len > length)
            return false;
        for (USize j = 0; j < len; ++j) {
            if (static_cast<unsigned short> (description_string[idx + j]) != GS::UniChar (uStr[j]))
                return false;
        }
        return true;
    };

    // Проверяет "ключевое_слово" + любое число пробелов (0 и более) + "{".
    // consumed - сколько символов всего "съедено" от idx (ключевое слово + пробелы + сама "{").
    auto matchesKeywordBrace = [&] (UIndex idx, const char *kw, USize kwLen, UIndex &consumed) -> bool {
        if (!matchesStr (idx, kw, kwLen))
            return false;
        UIndex j = idx + kwLen;
        while (j < length && static_cast<unsigned short> (description_string[j]) == ' ')
            ++j;
        if (j >= length || static_cast<unsigned short> (description_string[j]) != '{')
            return false;
        consumed = j + 1 - idx;
        return true;
    };

    for (UIndex i = 0; i < length; ++i) {
        if (matchesUStr (i, LINEBRAKE)) {
            i += LINEBRAKE.GetLength () - 1;
            continue;
        }
        if (matchesUStr (i, LINEBRAKER)) {
            i += LINEBRAKER.GetLength () - 1;
            continue;
        }
        if (matchesUStr (i, TABSTRING)) {
            i += TABSTRING.GetLength () - 1;
            continue;
        }

        UIndex consumed = 0;
        if (matchesKeywordBrace (i, "from_GUID", 9, consumed)) {
            dst.Append (FROMGUIDBR);
            i += consumed - 1;
            continue;
        }
        if (matchesKeywordBrace (i, "from_sub", 8, consumed)) {
            dst.Append (SYNCFROMSUBSTRING);
            i += consumed - 1;
            continue;
        }
        if (matchesKeywordBrace (i, "from", 4, consumed)) {
            dst.Append (SYNCFROMSTRING);
            i += consumed - 1;
            continue;
        }
        if (matchesKeywordBrace (i, "to_GUID", 7, consumed)) {
            dst.Append (TOGUIDBR);
            i += consumed - 1;
            continue;
        }
        if (matchesKeywordBrace (i, "to_sub", 6, consumed)) {
            dst.Append (SYNCTOSUBSTRING);
            i += consumed - 1;
            continue;
        }
        if (matchesKeywordBrace (i, "to", 2, consumed)) {
            dst.Append (SYNCTOSTRING);
            i += consumed - 1;
            continue;
        }
        dst.Append (description_string[i]);
    }
    description_string = std::move (dst);
    GS::Array<GS::UniString> rulestring;
    GS::Array<GS::UniString> local_scratch;
    ParamValue paramdef = {}; // Свойство, из которого получено правило
    ParamHelpers::ConvertToParamValue (paramdef, definition);
    paramdef.fromGuid = elemGuid;
    // Проходим по каждому правилу и извлекаем из него правило синхронизации (WriteDict syncRules)
    // и словарь уникальных параметров для чтения/записи (ParamDictElement paramToRead)
    StringSpltFilter (description_string,
                      SYNCPART,
                      rulestring,
                      BRACESTART,
                      &local_scratch); // Проверяем количество правил
    GS::Array<GS::UniString> params;
    for (auto &rulestring_one : rulestring) {
        ParamValue param;
        int syncdirection = SYNC_NO;     // Направление синхронизации
        GS::UniString rawparamName = ""; // Имя параметра/свойства с указанием типа синхронизации, для ключа словаря
        SkipValues ignorevals = {};      // Игнорируемые значения
        FormatString stringformat = {};
        API_Guid elemGuidfrom = elemGuid; // Элемент, из которого читаем данные
        API_Guid elemGuidto = elemGuid;   // Элемент, в котороый записываем данные
        API_ElemTypeID elementType_from = elementType; // Тип элемента, из которого читаем данные
        // Копировать из другого элемента
        if (rulestring_one.Contains (FROMGUIDBR)) {
            params.Clear ();
            rulestring_one.ReplaceAll (FROMGUID, EMPTYSTRING);
            UInt32 nparams = StringSplt (rulestring_one, SEMICOLON, params, true, &local_scratch);
            if (nparams == 2) {
                GS::UniString rawname = "";
                Name2Rawname (params[0], rawname);
                if (ParamValue *p = subproperty.GetPtr (rawname)) {
                    if (p->isValid) {
                        if (p->val.uniStringValue.IsEmpty ())
                            continue;
                        elemGuidfrom = APIGuidFromString (p->val.uniStringValue.ToCStr (0, MaxUSize, GChCode));
                        if (elemGuidfrom == APINULLGuid)
                            continue;
                        rulestring_one = "Sync_from{" + params[1];
                        elementType_from = GetElemTypeID (elemGuidfrom);
                        if (elementType_from == API_ZombieElemID) {
#if defined(TESTING)
                            DBprnt ("ParseSyncString err", "elementType_from == API_ZombieElemID");
#endif
                            continue;
                        }
                    } else {
#if defined(TESTING)
                        DBprnt ("ParseSyncString err", "p.isValid");
#endif
                        continue;
                    }
                } else {
#if defined(TESTING)
                    DBprnt ("ParseSyncString err", "!subproperty.ContainsKey (rawname) " + rawname);
#endif
                    continue;
                }
            } else {
#if defined(TESTING)
                DBprnt ("ParseSyncString err", "nparams == 2 ");
#endif
                continue;
            }
        } else {
            // Копировать в другой элемент
            if (rulestring_one.Contains (TOGUIDBR)) {
                params.Clear ();
                rulestring_one.ReplaceAll (TOGUID, EMPTYSTRING);
                UInt32 nparams = StringSplt (rulestring_one, SEMICOLON, params, true, &local_scratch);
                if (nparams == 2) {
                    GS::UniString rawname = "";
                    Name2Rawname (params[0], rawname);
                    if (ParamValue *p = subproperty.GetPtr (rawname)) {
                        if (p->isValid) {
                            if (p->val.uniStringValue.IsEmpty ())
                                continue;
                            elemGuidto = APIGuidFromString (p->val.uniStringValue.ToCStr (0, MaxUSize, GChCode));
                            if (elemGuidto == APINULLGuid)
                                continue;
                            param.fromGuid = elemGuidto;
                            rulestring_one = "Sync_to{" + params[1];
                        } else {
#if defined(TESTING)
                            DBprnt ("ParseSyncString err", "p.isValid");
#endif
                            continue;
                        }
                    } else {
#if defined(TESTING)
                        DBprnt ("ParseSyncString err", "!subproperty.ContainsKey (rawname) " + rawname);
#endif
                        continue;
                    }
                } else {
#if defined(TESTING)
                    DBprnt ("ParseSyncString err", "nparams == 2 ");
#endif
                    continue;
                }
            }
        }
        if (!SyncString (elementType_from,
                         rulestring_one,
                         syncdirection,
                         param,
                         ignorevals,
                         stringformat,
                         syncall,
                         synccoord,
                         syncclass))
            continue;
        hasRule = true;
        WriteData writeOne = {};
        writeOne.formatstring = stringformat;
        writeOne.ignorevals = ignorevals;
        if (param.fromCoord) {
            if (param.rawName.Contains ("north_dir") || param.rawName.Contains ("_sp_")) {
                ParamDictValue paramDict = {};
                ParamHelpers::AddValueToParamDictValue (paramDict, "@glob:glob_north_dir");
                ParamHelpers::AddParamDictValue2ParamDictElement (elemGuidfrom, paramDict, paramToRead);
            }
        }
        // Если требуется чтение из файла - добавим параметры для подбора
        if (param.fromFile) {
            ParamDictValue paramDict = {};
            if (!param.val.uniStringValue.Contains (CHARDQUT))
                ParamHelpers::ParseParamNameMaterial (param.val.uniStringValue, paramDict);
            if (!param.rawName_col_end.IsEmpty ())
                ParamHelpers::ParseParamNameMaterial (param.rawName_col_end, paramDict);
            if (!param.rawName_col_start.IsEmpty ())
                ParamHelpers::ParseParamNameMaterial (param.rawName_col_start, paramDict);
            if (!param.rawName_row_end.IsEmpty ())
                ParamHelpers::ParseParamNameMaterial (param.rawName_row_end, paramDict);
            if (!param.rawName_row_start.IsEmpty ())
                ParamHelpers::ParseParamNameMaterial (param.rawName_row_start, paramDict);
            ParamHelpers::AddParamDictValue2ParamDictElement (elemGuidfrom, paramDict, paramToRead);
        }
        // Вытаскиваем параметры для материалов, если такие есть
        if (param.fromMaterial) {
            ParamDictValue paramDict = {};
            GS::UniString templatestring = param.val.uniStringValue; // Строка с форматом числа
            if (ParamHelpers::ParseParamNameMaterial (templatestring, paramDict)) {
                param.val.uniStringValue = templatestring;
                // Свойств со спецтекстом может быть несколько (случайно)
                // Тут будут костыли, которые хорошо бы убрать
                for (UInt32 inx = 0; inx < 20; inx++) {
                    ParamHelpers::AddValueToParamDictValue (paramDict,
                                                            GS::UniString::Printf ("@property:sync_name%d", inx));
                }
                if (param.fromQuantity) {
                    ParamHelpers::AddValueToParamDictValue (paramDict, MAT_SOME_STUFF_TH);
                    ParamHelpers::AddValueToParamDictValue (paramDict, MAT_SOME_STUFF_UNITS);
                    ParamHelpers::AddValueToParamDictValue (paramDict, MAT_SOME_STUFF_KZAP);
                }
                ParamHelpers::AddParamDictValue2ParamDictElement (elemGuidfrom, paramDict, paramToRead);
            }
        }
        if (!param.fromMaterial && param.val.hasFormula) {
            ParamDictValue paramDict = {};
            GS::UniString templatestring = param.val.uniStringValue; // Строка с форматом числа
            param.type = definition.valueType;
            if (ParamHelpers::ParseParamNameMaterial (templatestring, paramDict, false)) {
                param.val.uniStringValue = templatestring;
                ParamHelpers::AddParamDictValue2ParamDictElement (elemGuidfrom, paramDict, paramToRead);
            }
        }
        if (syncdirection == SYNC_TO || syncdirection == SYNC_TO_SUB) {
            if (syncdirection == SYNC_TO_SUB) {
                hasSub = true;
                writeOne.toSub = true;
            } else {
                writeOne.guidTo = elemGuidto;
                if (param.fromGuid == APINULLGuid)
                    param.fromGuid = elemGuidfrom;
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
                if (param.fromGuid == APINULLGuid)
                    param.fromGuid = elemGuidfrom;
            }
            writeOne.guidTo = elemGuidto;
            writeOne.paramTo = paramdef;
            writeOne.paramFrom = param;
        }
        syncRules.Push (std::move (writeOne));
    }
    return hasRule;
}

bool Name2Rawname (GS::UniString &name, GS::UniString &rawname) {
    if (name.IsEmpty ())
        return false;
    GS::UniString paramNamePrefix = "";
    if (!name.Contains (BRACEEND))
        name = name + BRACEEND;
    if (!name.Contains (BRACESTART))
        name = name + BRACESTART;
    bool synctypefind = false;
    if (synctypefind == false) {
        if (name.Contains (PROPERTYPREF)) {
            name.ReplaceAll (PROPERTYPREF, EMPTYSTRING);
            paramNamePrefix = PROPERTYNAMEPREFIX;
            synctypefind = true;
        }
    }
    if (synctypefind == false) {
        if (name.Contains ("{symb_pos_"))
            name.ReplaceAll ("{symb_pos_", "{Coord:symb_pos_");
        if (name.Contains (COORDPREF)) {
            name.ReplaceAll (COORDPREF, EMPTYSTRING);
            paramNamePrefix = COORDNAMEPREFIX;
            synctypefind = true;
        }
    }
    if (synctypefind == false) {
        if (name.Contains ("{id}") || name.Contains ("{ID}")) {
            paramNamePrefix = IDNAMEPREFIX;
            synctypefind = true;
        }
    }
    if (synctypefind == false) {
        if (!name.Contains (":") || name.Contains ("escription:") || name.Contains ("esc:")) {
            if (name.Contains ("escription:") || name.Contains ("esc:")) {
                name.ReplaceAll ("description:", EMPTYSTRING);
                name.ReplaceAll ("Description:", EMPTYSTRING);
                name.ReplaceAll ("desc:", EMPTYSTRING);
                name.ReplaceAll ("Desc:", EMPTYSTRING);
            }
            paramNamePrefix = GDLNAMEPREFIX;
            synctypefind = true;
        }
    }

    if (synctypefind == false) {
        if (name.Contains (MORPHPREF)) {
            name.ReplaceAll (MORPHPREF, EMPTYSTRING);
            paramNamePrefix = MORPHNAMEPREFIX;
            synctypefind = true;
        }
    }
    if (synctypefind == false) {
        if (name.Contains (INFOPREF)) {
            name.ReplaceAll (INFOPREF, EMPTYSTRING);
            paramNamePrefix = INFONAMEPREFIX;
            synctypefind = true;
        }
    }
    if (synctypefind == false) {
        if (name.Contains (IFCPREF)) {
            name.ReplaceAll (IFCPREF, EMPTYSTRING);
            paramNamePrefix = IFCNAMEPREFIX;
            synctypefind = true;
        }
    }
    if (synctypefind == false) {
        if (name.Contains (GLOBPREF)) {
            name.ReplaceAll (GLOBPREF, EMPTYSTRING);
            paramNamePrefix = GLOBNAMEPREFIX;
            synctypefind = true;
        }
    }
    if (synctypefind == false) {
        if (name.Contains (CLASSPREF)) {
            name.ReplaceAll (CLASSPREF, EMPTYSTRING);
            paramNamePrefix = CLASSNAMEPREFIX;
            synctypefind = true;
        }
    }
    if (synctypefind == false) {
        if (name.Contains (ATTRIBPREF)) {
            synctypefind = true;
            name.ReplaceAll (ATTRIBPREF, EMPTYSTRING);
            paramNamePrefix = ATTRIBNAMEPREFIX;
        }
    }
    if (synctypefind == false) {
        if (name.Contains (ELEMENTPREF)) {
            synctypefind = true;
            name.ReplaceAll (ELEMENTPREF, EMPTYSTRING);
            paramNamePrefix = ELEMENTNAMEPREFIX;
        }
    }
    if (synctypefind == false) {
        if (name.Contains (MEPPREF)) {
            synctypefind = true;
            name.ReplaceAll (MEPPREF, EMPTYSTRING);
            paramNamePrefix = MEPNAMEPREFIX;
        }
    }
    if (synctypefind == false) {
        if (name.Contains (FILEPREF)) {
            synctypefind = true;
            name.ReplaceAll (FILEPREF, EMPTYSTRING);
            paramNamePrefix = FILENAMEPREFIX;
        }
    }
    if (synctypefind == false) {
        if (name.Contains (LISTDATAPREF)) {
            synctypefind = true;
            name.ReplaceAll (LISTDATAPREF, EMPTYSTRING);
            paramNamePrefix = LISTDATANAMEPREFIX;
        }
    }
    if (synctypefind == false)
        return false;
    GS::Array<GS::UniString> params;
    GS::UniString tparamName = name.GetSubstring (CHARBRACESTART, CHARBRACEEND, 0);
    UInt32 nparam = StringSplt (tparamName, SEMICOLON, params, true);
    if (nparam == 0)
        return false;
    GS::UniString paramName = params.Get (0);
    FormatStringFunc::GetFormatString (paramName);
    paramName.ReplaceAll (SLASHEKR, SLASH);
    rawname = paramNamePrefix + paramName.ToLowerCase () + BRACEEND;
    return true;
}

// -----------------------------------------------------------------------------
// Парсит описание свойства
// -----------------------------------------------------------------------------
bool SyncString (const API_ElemTypeID &elementType,
                 GS::UniString rulestring_one,
                 int &syncdirection,
                 ParamValue &param,
                 SkipValues &ignorevals,
                 FormatString &stringformat,
                 bool syncall,
                 bool synccoord,
                 bool syncclass) {
    syncdirection = SYNC_NO;
    // Выбор направления синхронизации
    // Копировать в субэлементы или из субэлементов
    if (rulestring_one.Contains (SYNCFROMSTRING)) {
        syncdirection = SYNC_FROM;
    } else {
        if (rulestring_one.Contains (SYNCTOSTRING)) {
            syncdirection = SYNC_TO;
        } else {
            if (rulestring_one.Contains (SYNCFROMSUBSTRING)) {
                syncdirection = SYNC_FROM_SUB;
            } else {
                if (rulestring_one.Contains (SYNCTOSUBSTRING))
                    syncdirection = SYNC_TO_SUB;
            }
        }
    }

    // Если направление синхронизации не нашли - выходим
    if (syncdirection == SYNC_NO)
        return false;
    GS::UniString paramNamePrefix = "";
    bool synctypefind = false;
    if (rulestring_one.Contains (LINEBRAKE))
        rulestring_one.ReplaceAll (LINEBRAKE, EMPTYSTRING);
    if (rulestring_one.Contains (LINEBRAKER))
        rulestring_one.ReplaceAll (LINEBRAKER, EMPTYSTRING);
    if (rulestring_one.Contains (TABSTRING))
        rulestring_one.ReplaceAll (TABSTRING, EMPTYSTRING);
    // Выбор типа копируемого свойства
    // Я не очень понял - умеет ли с++ в ленивые вычисления, поэтому сделаю вложенные условия, чтобы избежать ненужного
    // поиска по строке
    if (rulestring_one.Contains ("{symb_pos_")) {
        rulestring_one.ReplaceAll ("{symb_pos_", "{Coord:symb_pos_");
    }
    bool hasformula = rulestring_one.Contains (CHARFORMULASTART) && rulestring_one.Contains (CHARFORMULAEND);
    if (rulestring_one.Contains (QRPREF)) {
        rulestring_one.ReplaceAll (QRPREF, EMPTYSTRING);
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
        if (rulestring_one.Contains (FILEPREF)) {
            synctypefind = true;
            rulestring_one.ReplaceAll (FILEPREF, EMPTYSTRING);
            paramNamePrefix = FILENAMEPREFIX;
            param.typeinx = FILETYPEINX;
            param.fromFile = true;
            syncdirection = SYNC_FROM;
            GS::Array<GS::UniString> params;
            GS::Array<GS::UniString> local_scratch;
            GS::UniString rule = rulestring_one.GetSubstring (CHARBRACESTART, CHARBRACEEND, 0);
            param.rawName = paramNamePrefix;
            param.rawName.Append (rule.ToLowerCase ());
            param.name = rule;
            param.rawName.Append (BRACEEND);
            if (StringSplt (rule, SEMICOLON, params, true, &local_scratch) > 1) {
                GS::UniString type = params[0].ToLowerCase ();
                if (type.Contains ("lookup") || type.Contains ("lokup"))
                    param.val.array_format_out = FILE_LOOKUP;
                GS::Array<GS::UniString> params_2;
                UInt32 nparam = StringSplt (params[1], COMMA, params_2, false, &local_scratch);
                if (nparam > 2) {
                    param.val.uniStringValue = params_2[0]; // имя файла
                    if (!param.val.uniStringValue.Contains (CHARDQUT)) {
                        if (!param.val.uniStringValue.Contains (CHARBRACESTART))
                            param.val.uniStringValue = CHARBRACESTART + param.val.uniStringValue;
                        if (!param.val.uniStringValue.Contains (CHARBRACEEND))
                            param.val.uniStringValue = param.val.uniStringValue + CHARBRACEEND;
                    }
                    short col_out;
                    try {
                        col_out = std::stoi (params_2[1].ToCStr ().Get ());
                    } catch (const std::exception &) {
                        syncdirection = SYNC_NO;
                        col_out = 0;
                    }
                    param.composite_pen = col_out;
                    // имя свойства для поиска (1)
                    param.rawName_col_end = params_2[2];
                    if (!param.rawName_col_end.Contains (CHARBRACESTART))
                        param.rawName_col_end = CHARBRACESTART + param.rawName_col_end;
                    if (!param.rawName_col_end.Contains (CHARBRACEEND))
                        param.rawName_col_end = param.rawName_col_end + CHARBRACEEND;
                    rulestring_one.ReplaceAll (params_2[0], EMPTYSTRING);
                    rulestring_one.ReplaceAll (params_2[1], EMPTYSTRING);
                    rulestring_one.ReplaceAll (params_2[2], EMPTYSTRING);
                    if (nparam > 3) {
                        short col_find;
                        try {
                            col_find = std::stoi (params_2[3].ToCStr ().Get ());
                        } catch (const std::exception &) {
                            syncdirection = SYNC_NO;
                            col_find = 0;
                        }
                        if (col_find > 0)
                            param.val.array_column_end = col_find;
                        if (param.val.array_column_end == 0)
                            param.val.array_column_end = 1;
                        rulestring_one.ReplaceAll (params_2[3], EMPTYSTRING);
                    }
                    if (nparam > 5) {
                        short col_find;
                        try {
                            col_find = std::stoi (params_2[5].ToCStr ().Get ());
                        } catch (const std::exception &) {
                            syncdirection = SYNC_NO;
                            col_find = 0;
                        }
                        if (col_find > 0 && !params_2[4].IsEmpty ()) {
                            param.rawName_col_start = params_2[4];
                            if (!param.rawName_col_start.Contains (CHARBRACESTART))
                                param.rawName_col_start = CHARBRACESTART + param.rawName_col_start;
                            if (!param.rawName_col_start.Contains (CHARBRACEEND))
                                param.rawName_col_start = param.rawName_col_start + CHARBRACEEND;
                            param.val.array_column_start = col_find;
                            rulestring_one.ReplaceAll (params_2[4], EMPTYSTRING);
                            rulestring_one.ReplaceAll (params_2[5], EMPTYSTRING);
                        }
                    }
                    if (nparam > 7) {
                        short col_find;
                        try {
                            col_find = std::stoi (params_2[7].ToCStr ().Get ());
                        } catch (const std::exception &) {
                            syncdirection = SYNC_NO;
                            col_find = 0;
                        }
                        if (col_find > 0 && !params_2[6].IsEmpty ()) {
                            param.rawName_row_end = params_2[6];
                            if (!param.rawName_row_end.Contains (CHARBRACESTART))
                                param.rawName_row_end = CHARBRACESTART + param.rawName_row_end;
                            if (!param.rawName_row_end.Contains (CHARBRACEEND))
                                param.rawName_row_end = param.rawName_row_end + CHARBRACEEND;
                            param.val.array_row_end = col_find;
                            rulestring_one.ReplaceAll (params_2[6], EMPTYSTRING);
                            rulestring_one.ReplaceAll (params_2[7], EMPTYSTRING);
                        }
                    }
                    if (nparam > 9 && !params_2[8].IsEmpty ()) {
                        short col_find;
                        try {
                            col_find = std::stoi (params_2[9].ToCStr ().Get ());
                        } catch (const std::exception &) {
                            syncdirection = SYNC_NO;
                            col_find = 0;
                        }
                        if (col_find > 0) {
                            param.rawName_row_start = params_2[8];
                            if (!param.rawName_row_start.Contains (CHARBRACESTART))
                                param.rawName_row_start = CHARBRACESTART + param.rawName_row_start;
                            if (!param.rawName_row_start.Contains (CHARBRACEEND))
                                param.rawName_row_start = param.rawName_row_start + CHARBRACEEND;
                            param.val.array_row_start = col_find;
                            rulestring_one.ReplaceAll (params_2[8], EMPTYSTRING);
                            rulestring_one.ReplaceAll (params_2[9], EMPTYSTRING);
                        }
                    }
                    rulestring_one.ReplaceAll (COMMA, EMPTYSTRING);
                } else {
                    syncdirection = SYNC_NO;
                }
            } else {
                syncdirection = SYNC_NO;
            }
        }
    }

    if (synctypefind == false) {
        if ((!rulestring_one.Contains (":") || rulestring_one.Contains ("escription:") ||
             rulestring_one.Contains ("esc:")) &&
            !hasformula) {
            if (rulestring_one.Contains ("escription:") || rulestring_one.Contains ("esc:")) {
                param.fromGDLdescription = true;
                rulestring_one.ReplaceAll ("description:", EMPTYSTRING);
                rulestring_one.ReplaceAll ("Description:", EMPTYSTRING);
                rulestring_one.ReplaceAll ("desc:", EMPTYSTRING);
                rulestring_one.ReplaceAll ("Desc:", EMPTYSTRING);
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
        if (rulestring_one.Contains (ATTRIBPREF)) {
            synctypefind = true;
            rulestring_one.ReplaceAll (ATTRIBPREF, EMPTYSTRING);
            paramNamePrefix = ATTRIBNAMEPREFIX;
            param.typeinx = ATTRIBTYPEINX;
            param.fromAttribElement = true;
            syncdirection = SYNC_TO;
        }
    }
    if (synctypefind == false) {
        if (rulestring_one.Contains (MATERIALPREF) && (rulestring_one.Contains (CHARDQUT) || hasformula)) {
            synctypefind = true;
            rulestring_one.ReplaceAll (MATERIALPREF, EMPTYSTRING);
            rulestring_one.ReplaceAll ("{Layers;", "{Layers,20;");
            rulestring_one.ReplaceAll ("{Layers_inv;", "{Layers_inv,20;");
            rulestring_one.ReplaceAll ("{Layers_auto;", "{Layers_auto,20;");
            paramNamePrefix = MATERIALNAMEPREFIX;
            param.typeinx = MATERIALTYPEINX;
            GS::UniString templatestring = "";
            if (hasformula) {
                if (rulestring_one.Contains (CHARDQUT)) {
                    templatestring = rulestring_one.GetSubstring (CHARDQUT, CHARDQUT, 0);
                    FormatStringFunc::GetFormatStringFromFormula (rulestring_one, templatestring, stringformat_raw);
                    stringformat = FormatStringFunc::ParseFormatString (stringformat_raw);
                    param.val.uniStringValue = templatestring;
                } else {
                    templatestring = rulestring_one.GetSubstring (CHARFORMULASTART, CHARFORMULAEND, 0);
                    FormatStringFunc::GetFormatStringFromFormula (rulestring_one, templatestring, stringformat_raw);
                    stringformat = FormatStringFunc::ParseFormatString (stringformat_raw);
                    param.val.uniStringValue = STRFORMULASTART;
                    param.val.uniStringValue.Append (templatestring);
                    param.val.uniStringValue.Append (STRFORMULAEND);
                }
                rulestring_one.ReplaceAll (templatestring, EMPTYSTRING);
                rulestring_one.ReplaceAll (stringformat_raw, EMPTYSTRING);
                param.val.hasFormula = true;
            } else {
                templatestring = rulestring_one.GetSubstring (CHARDQUT, CHARDQUT, 0);
                param.val.uniStringValue = templatestring;
                param.val.hasFormula = false;
                rulestring_one.ReplaceAll (templatestring, EMPTYSTRING);
            }
            param.val.formatstring = stringformat;
            param.fromMaterial = true;
            param.composite_pen = 20;
            rulestring_one.ReplaceAll (SPACESTRING, EMPTYSTRING);
            if (rulestring_one.Contains (COMMA) && rulestring_one.Contains (SEMICOLON)) {
                GS::UniString penstring = rulestring_one.GetSubstring (CHARCOMMA, CHARBSEMICOLON, 0);
                if (penstring.Contains ("all")) {
                    param.composite_pen = -1;
                    param.fromQuantity = true;
                } else {
                    if (penstring.Contains ("unic")) {
                        param.composite_pen = -2;
                        param.fromQuantity = true;
                    } else {
                        short pen;
                        try {
                            pen = std::stoi (penstring.ToCStr ().Get ());
                        } catch (const std::exception &) {
                            syncdirection = SYNC_NO;
                            pen = 0;
                        }
                        if (pen > 0)
                            param.composite_pen = pen;
                    }
                }
            }
            if (!templatestring.IsEmpty ())
                syncdirection = SYNC_FROM;
        }
    }
    if (synctypefind == false) {
        if (hasformula) {
            GS::UniString templatestring = "";
            if (rulestring_one.Contains (CHARDQUT)) {
                templatestring = rulestring_one.GetSubstring (CHARDQUT, CHARDQUT, 0);
                FormatStringFunc::GetFormatStringFromFormula (rulestring_one, templatestring, stringformat_raw);
                stringformat = FormatStringFunc::ParseFormatString (stringformat_raw);
                param.val.uniStringValue = templatestring;
            } else {
                templatestring = rulestring_one.GetSubstring (CHARFORMULASTART, CHARFORMULAEND, 0);
                FormatStringFunc::GetFormatStringFromFormula (rulestring_one, templatestring, stringformat_raw);
                stringformat = FormatStringFunc::ParseFormatString (stringformat_raw);
                param.val.uniStringValue = STRFORMULASTART;
                param.val.uniStringValue.Append (templatestring);
                param.val.uniStringValue.Append (STRFORMULAEND);
            }
            rulestring_one.ReplaceAll (templatestring, EMPTYSTRING);
            rulestring_one.ReplaceAll (stringformat_raw, EMPTYSTRING);
            param.val.formatstring = stringformat;
            synctypefind = true;
            paramNamePrefix = FORMULANAMEPREFIX;
            param.typeinx = FORMULATYPEINX;
            param.val.hasFormula = true;
            syncdirection = SYNC_FROM;
        }
    }
    if (synctypefind == false) {
        if (rulestring_one.Contains (MORPHPREF)) {
            synctypefind = true;
            rulestring_one.ReplaceAll (MORPHPREF, EMPTYSTRING);
            paramNamePrefix = MORPHNAMEPREFIX;
            param.typeinx = MORPHTYPEINX;
            param.fromMorph = true;
            syncdirection = SYNC_FROM;
        }
    }
    if (synctypefind == false) {
        if (rulestring_one.Contains (COORDPREF)) {
            synctypefind = true;
            rulestring_one.ReplaceAll (COORDPREF, EMPTYSTRING);
            paramNamePrefix = COORDNAMEPREFIX;
            param.typeinx = COORDTYPEINX;
            param.fromCoord = true;
            if (rulestring_one.Contains ("orth"))
                param.fromGlob = true;
            if (!rulestring_one.Contains ("symb_pos_") && !rulestring_one.Contains ("symb_rotangle"))
                syncdirection = SYNC_FROM;
        }
    }

    if (synctypefind == false) {
        if (rulestring_one.Contains (PROPERTYPREF)) {
            synctypefind = true;
            rulestring_one.ReplaceAll (PROPERTYPREF, EMPTYSTRING);
            paramNamePrefix = PROPERTYNAMEPREFIX;
            param.typeinx = PROPERTYTYPEINX;
            param.fromProperty = true;
        }
    }
    if (synctypefind == false) {
        if (rulestring_one.Contains (INFOPREF)) {
            synctypefind = true;
            rulestring_one.ReplaceAll (INFOPREF, EMPTYSTRING);
            paramNamePrefix = INFONAMEPREFIX;
            param.typeinx = INFOTYPEINX;
            param.fromInfo = true;
        }
    }
    if (synctypefind == false) {
        if (rulestring_one.Contains (IFCPREF)) {
            synctypefind = true;
            rulestring_one.ReplaceAll (IFCPREF, EMPTYSTRING);
            paramNamePrefix = IFCNAMEPREFIX;
            param.typeinx = IFCTYPEINX;
            param.fromIFCProperty = true;
            syncdirection = SYNC_FROM;
        }
    }
    if (synctypefind == false) {
        if (rulestring_one.Contains (GLOBPREF)) {
            synctypefind = true;
            rulestring_one.ReplaceAll (GLOBPREF, EMPTYSTRING);
            paramNamePrefix = GLOBNAMEPREFIX;
            param.typeinx = GLOBTYPEINX;
            param.fromGlob = true;
            syncdirection = SYNC_FROM;
        }
    }
    if (synctypefind == false) {
        if (rulestring_one.Contains (CLASSPREF)) {
            synctypefind = true;
            rulestring_one.ReplaceAll (CLASSPREF, EMPTYSTRING);
            paramNamePrefix = CLASSNAMEPREFIX;
            param.typeinx = CLASSTYPEINX;
            param.fromClassification = true;
        }
    }
    if (synctypefind == false) {
        if (rulestring_one.Contains (ELEMENTPREF)) {
            synctypefind = true;
            rulestring_one.ReplaceAll (ELEMENTPREF, EMPTYSTRING);
            paramNamePrefix = ELEMENTNAMEPREFIX;
            param.typeinx = ELEMENTTYPEINX;
            param.fromElement = true;
            syncdirection = SYNC_FROM;
        }
    }
    if (synctypefind == false) {
        if (rulestring_one.Contains (MEPPREF)) {
            synctypefind = true;
            rulestring_one.ReplaceAll (MEPPREF, EMPTYSTRING);
            paramNamePrefix = MEPNAMEPREFIX;
            param.typeinx = MEPTYPEINX;
            param.fromMEP = true;
            syncdirection = SYNC_FROM;
        }
    }

    if (synctypefind == false) {
        if (rulestring_one.Contains (LISTDATAPREF)) {
            synctypefind = true;
            rulestring_one.ReplaceAll (LISTDATAPREF, EMPTYSTRING);
            paramNamePrefix = LISTDATANAMEPREFIX;
            param.typeinx = LISTDATATYPEINX;
            param.fromListData = true;
            syncdirection = SYNC_FROM;
        }
    }

    if (synctypefind == false)
        return false;
    param.eltype = elementType;

    // Проверка допустимости правила для типа элемента
    if (param.fromGDLparam) {
        if (elementType == API_WallID || elementType == API_SlabID || elementType == API_ColumnID ||
            elementType == API_BeamID || elementType == API_RoofID || elementType == API_ShellID ||
            elementType == API_BeamSegmentID || elementType == API_ColumnSegmentID || elementType == API_MorphID)
            synctypefind = false;
    }
    if (param.fromGDLdescription) {
        if (elementType != API_ObjectID)
            synctypefind = false;
    }
    if (param.fromListData) {
        if (elementType != API_ObjectID)
            synctypefind = false;
    }
    if (param.fromMaterial) {
        if (!(elementType == API_ObjectID && param.fromQuantity) && elementType != API_WallID &&
            elementType != API_SlabID && elementType != API_ColumnID && elementType != API_BeamID &&
            elementType != API_RoofID && elementType != API_BeamSegmentID && elementType != API_ColumnSegmentID &&
            elementType != API_MeshID && elementType != API_MorphID && elementType != API_ShellID)
            synctypefind = false;
    }
    if (param.fromMorph) {
        if (elementType != API_MorphID)
            synctypefind = false;
    }

    if (syncdirection == SYNC_FROM_SUB && elementType == API_ObjectID)
        synctypefind = false;

    // Проверка включенных флагов
    if (!syncall) {
        if (!param.fromCoord && !param.fromClassification)
            synctypefind = false;
    }
    if (!synccoord) {
        if (param.fromCoord)
            synctypefind = false;
    }
    if (!syncclass) {
        if (param.fromClassification && syncdirection == SYNC_TO)
            synctypefind = false;
    }

    // Если тип свойства не нашли - выходим
    if (synctypefind == false)
        return false;

    GS::UniString tparamName = rulestring_one.GetSubstring (CHARBRACESTART, CHARBRACEEND, 0);
    GS::Array<GS::UniString> params = {};
    GS::Array<GS::UniString> local_scratch;
    UInt32 nparam = StringSplt (tparamName, SEMICOLON, params, true, &local_scratch);

    // Параметры не найдены - выходим
    if (nparam == 0)
        return false;
    GS::UniString paramName = params.Get (0);
    paramName.ReplaceAll (SLASHEKR, SLASH);

    if (param.fromMaterial || param.val.hasFormula) {
        param.rawName = paramNamePrefix;
        param.rawName.Append (paramName.ToLowerCase ());
        param.rawName.Append (SEMICOLON);
        param.rawName.Append (param.val.uniStringValue);
        param.rawName.Append (DOT);
        param.rawName.Append (stringformat.stringformat);
        param.rawName.Append (BRACEEND);
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
            param.rawName.Append (SEMICOLON);
            param.rawName.Append (param.val.uniStringValue.ToLowerCase ());
        }
        param.rawName.Append (BRACEEND);
        start_ignore = 1 + nparam;
    }
    if (param.rawName.IsEmpty ()) {
        param.rawName = paramNamePrefix;
        param.rawName.Append (paramName.ToLowerCase ());
        param.rawName.Append (BRACEEND);
    }
    if (param.name.IsEmpty ())
        param.name = paramName;
    if (nparam > 1) {
        // Обработка данных о размерах массива и типе чтения
        if (start_ignore == 0)
            start_ignore = 1;
        GS::UniString arrtype = params[1].ToLowerCase ();
        bool hasArray = false;
        if (!hasArray && (arrtype.Contains ("uniq") || arrtype.Contains ("unic"))) {
            arrtype.ReplaceAll ("uniq", EMPTYSTRING);
            arrtype.ReplaceAll ("unic", EMPTYSTRING);
            param.val.array_format_out = ARRAY_UNIC;
            hasArray = true;
        }
        if (!hasArray && arrtype.Contains ("sum")) {
            arrtype.ReplaceAll ("sum", EMPTYSTRING);
            param.val.array_format_out = ARRAY_SUM;
            hasArray = true;
        }
        if (!hasArray && arrtype.Contains ("min")) {
            arrtype.ReplaceAll ("min", EMPTYSTRING);
            param.val.array_format_out = ARRAY_MIN;
            hasArray = true;
        }
        if (!hasArray && arrtype.Contains ("max")) {
            arrtype.ReplaceAll ("max", EMPTYSTRING);
            param.val.array_format_out = ARRAY_MAX;
            hasArray = true;
        }
        if (hasArray && !param.fromListData && !param.fromGDLparam && !param.fromGDLdescription && !param.fromGDLArray)
            hasArray = false;
        if (hasArray) {
            int array_row_start = 0;
            int array_row_end = 0;
            int array_column_start = 0;
            int array_column_end = 0;
            GS::UniString rawName_row_start = ""; // Имя параметра со значением начала диапазона чтения строк
            GS::UniString rawName_row_end = ""; // Имя параметра со значением конца диапазона чтения строк
            GS::UniString rawName_col_start = ""; // Имя параметра со значением начала диапазона чтения столбцов
            GS::UniString rawName_col_end = ""; // Имя параметра со значением конца диапазона чтения столбцов
            double p;
            if (params[1].Contains ("(")) {
                GS::Array<GS::UniString> sr;
                UInt32 nsr = StringSplt (arrtype, ")", sr, true, &local_scratch);
                if (nsr > 0) {
                    GS::UniString sr1 = sr.Get (0);
                    sr1.Trim ('(');
                    if (sr1.Contains (COMMA)) {
                        GS::Array<GS::UniString> dim;
                        UInt32 ndim = StringSplt (sr1, COMMA, dim, true, &local_scratch);
                        if (ndim > 0) {
                            GS::UniString dim0 = dim.Get (0);
                            if (UniStringToDouble (dim0, p)) {
                                array_row_start = (int)p;
                            } else {
                                rawName_row_start = paramNamePrefix;
                                rawName_row_start.Append (dim0);
                                rawName_row_start.Append (BRACEEND);
                            }
                        }
                        if (ndim > 1) {
                            GS::UniString dim1 = dim.Get (1);
                            if (UniStringToDouble (dim1, p)) {
                                array_row_end = (int)p;
                            } else {
                                rawName_row_end = paramNamePrefix;
                                rawName_row_end.Append (dim1);
                                rawName_row_end.Append (BRACEEND);
                            }
                        }
                    } else {
                        if (UniStringToDouble (sr1, p)) {
                            array_row_start = (int)p;
                            array_row_end = (int)p;
                        } else {
                            rawName_row_start = paramNamePrefix;
                            rawName_row_start.Append (sr1);
                            rawName_row_start.Append (BRACEEND);
                            rawName_row_end = rawName_col_start;
                        }
                    }
                }
                if (nsr > 1) {
                    GS::UniString sr1 = sr.Get (1);
                    sr1.Trim ('(');
                    if (sr1.Contains (COMMA)) {
                        GS::Array<GS::UniString> dim;
                        UInt32 ndim = StringSplt (sr1, COMMA, dim, true, &local_scratch);
                        if (ndim > 0) {
                            GS::UniString dim0 = dim.Get (0);
                            if (UniStringToDouble (dim0, p)) {
                                array_column_start = (int)p;
                            } else {
                                rawName_col_start = paramNamePrefix;
                                rawName_col_start.Append (dim0);
                                rawName_col_start.Append (BRACEEND);
                            }
                        }
                        if (ndim > 1) {
                            GS::UniString dim1 = dim.Get (1);
                            if (UniStringToDouble (dim1, p)) {
                                array_column_end = (int)p;
                            } else {
                                rawName_col_end = paramNamePrefix;
                                rawName_col_end.Append (dim1);
                                rawName_col_end.Append (BRACEEND);
                            }
                        }
                    } else {
                        if (UniStringToDouble (sr1, p)) {
                            array_column_start = (int)p;
                            array_column_end = (int)p;
                        } else {
                            rawName_col_start = paramNamePrefix;
                            rawName_col_start.Append (sr1);
                            rawName_col_start.Append (BRACEEND);
                            rawName_col_end = rawName_col_start;
                        }
                    }
                }
            }
            if (array_row_start < 0)
                array_row_start = 0;
            if (array_row_end < 0)
                array_row_end = 0;
            if (array_column_start < 0)
                array_column_start = 0;
            if (array_column_end < 0)
                array_column_end = 0;
            param.val.array_row_start = array_row_start;
            param.val.array_row_end = array_row_end;
            param.val.array_column_start = array_column_start;
            param.val.array_column_end = array_column_end;
            param.fromGDLArray = true;

            param.rawName_row_start = rawName_row_start;
            param.rawName_row_end = rawName_row_end;
            param.rawName_col_start = rawName_col_start;
            param.rawName_col_end = rawName_col_end;
            if (!rawName_row_start.IsEmpty () || !rawName_row_end.IsEmpty () || !rawName_col_start.IsEmpty () ||
                !rawName_col_end.IsEmpty ())
                param.needPreRead = true;
            param.rawName = paramNamePrefix;
            param.rawName.Append (paramName.ToLowerCase ());
            param.rawName.Append (GS::UniString::Printf ("@arr_%d_%d_%d_%d_%d",
                                                         array_row_start,
                                                         array_row_end,
                                                         array_column_start,
                                                         array_column_end,
                                                         param.val.array_format_out));
            param.rawName.Append (rawName_row_start);
            param.rawName.Append ("_");
            param.rawName.Append (rawName_row_end);
            param.rawName.Append ("_");
            param.rawName.Append (rawName_col_start);
            param.rawName.Append ("_");
            param.rawName.Append (rawName_col_end);
            param.rawName.Append (BRACEEND);
            start_ignore = 2;
        }
        // Обработка игнорируемых значений
        if (nparam > start_ignore) {
            for (UInt32 j = start_ignore; j < nparam; j++) {
                GS::UniString ignoreval = "";
                if (params[j].Contains (CHARDQUT)) {
                    ignoreval = params[j].GetSubstring (CHARDQUT, CHARDQUT, 0);
                } else {
                    ignoreval = params[j];
                }
                if (ignoreval.IsEmpty ())
                    continue;
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
void SyncSetSubelement (SyncSettings &syncSettings) {
    GSErrCode err = NoError;
    GS::UniString fmane = "Set SubElement";
    GS::Array<API_Guid> subguidArray_ =
        GetSelectedElements (true,
                             true,
                             syncSettings,
                             false,
                             false,
                             false); // Дочерние элементы, в которые будет записана информация об основном элементе
    if (subguidArray_.IsEmpty ())
        return;

    API_Element parentelement = {}; // Родительский элемент
    API_ElemTypeID parentelementtype = API_ZombieElemID;
#if defined(AC_27) || defined(AC_28) || defined(AC_29) || defined(AC_26)
    if (!ClickAnElem ("Click an parent elem",
                      API_ZombieElemID,
                      nullptr,
                      &parentelement.header.type,
                      &parentelement.header.guid)) {
        return;
    }
#else
    if (!ClickAnElem ("Click an parent elem",
                      API_ZombieElemID,
                      nullptr,
                      &parentelement.header.typeID,
                      &parentelement.header.guid)) {
        return;
    }
#endif
    parentelementtype = GetElemTypeID (parentelement);
    clock_t start, finish;
    double duration = 0;
    start = clock ();
    // При работе на резрезах возможно попадание по виртуальному элементу, обработаем
    API_Elem_Head parentelementhead = {};
    if (parentelementtype == API_SectElemID) {
        API_Guid parentguid = APINULLGuid;
        API_ElemTypeID elementType = API_ZombieElemID;
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
    GS::Array<API_Guid> subguidArray = {}; // Массив редактируемых и почищенных от SectElem дочерних элементов
    for (const auto &subguid : subguidArray_) {
        API_ElemTypeID elementType = API_ZombieElemID;
        if (!IsElementEditable (subguid, syncSettings, false, elementType))
            continue;
        if (elementType == API_LabelID)
            continue;
        if ((CheckElementType (elementType, syncSettings) && elementType != API_DimensionID)) {
            if (subguid != parentelementhead.guid)
                subguidArray.Push (subguid);
        } else {
            if (elementType == API_SectElemID) {
                API_Guid parentguid;
                GetParentGUIDSectElem (subguid, parentguid, elementType);
                if (parentguid != parentelementhead.guid)
                    subguidArray.Push (parentguid);
            }
        }
    }
    if (subguidArray.IsEmpty ())
        return;
    ParamDictValue propertyParams = {};
    GSFlags flag = 0;
    ParamDictElement paramToWrite = {};
    SyncSetSubelementScope (parentelementhead, subguidArray, paramToWrite, EMPTYSTRING, true);
    if (!paramToWrite.IsEmpty ()) {
        err = ACAPI_CallUndoableCommand ("SetSubelement", [&] () -> GSErrCode {
            ParamHelpers::ElementsWrite (paramToWrite);
            return NoError;
        });
        SyncSelected (syncSettings);
    }
    finish = clock ();
    duration = (double)(finish - start) / CLOCKS_PER_SEC;
    GS::UniString time = GS::UniString::Printf (" %.3f s", duration);
    msg_rep (fmane, time, err, APINULLGuid);
}

// -----------------------------------------------------------------------------
// Запись Guid связанных элементов
// Функция для вызова из ACAPI_CallUndoableCommand
// -----------------------------------------------------------------------------
bool SyncSetSubelementScope (const API_Elem_Head &parentelementhead,
                             const GS::Array<API_Guid> &subguidArray,
                             ParamDictElement &paramToWrite,
                             const GS::UniString &suffix,
                             const bool &check_guid) {
    GSErrCode err = NoError;
    if (subguidArray.IsEmpty ())
        return false;
    if (parentelementhead.guid == APINULLGuid)
        return false;
    ParamDictElement paramToRead = {};
    if (!SyncGetSyncGUIDProperty (subguidArray, paramToRead, suffix))
        return false;
    GS::UniString parentguidtxt = APIGuidToString (parentelementhead.guid);
    // Записываем в дочерние элементы GUID родительского
    bool has_element = false;
    for (const auto &subguid : subguidArray) {
        if (ParamDictValue *subparams = paramToRead.GetPtr (subguid)) {
            bool flag_write = false;
            for (ParamDictValue::PairIterator cIt = subparams->EnumeratePairs (); cIt != NULL; ++cIt) {
#if defined(AC_28) || defined(AC_29)
                ParamValue &param = cIt->value;
#else
                ParamValue &param = *cIt->value;
#endif
                // TODO Дописать фильтр по типу, слою, свойству
                if (!param.val.uniStringValue.IsEmpty () && check_guid) {
                    API_Elem_Head elementHead = {};
                    elementHead.guid = APIGuidFromString (param.val.uniStringValue.ToCStr (0, MaxUSize, GChCode));
                    if (ACAPI_Element_GetHeader (&elementHead) != NoError) { // Проверка - существует ли ещё элемент
                        if (ParamValue *paramtow = subparams->GetPtr (param.rawName)) {
                            paramtow->val.uniStringValue = parentguidtxt;
                            paramtow->isValid = true;
                            ParamHelpers::AddParamValue2ParamDictElement (subguid, *paramtow, paramToWrite);
                            flag_write = true;
                            has_element = true;
                        }
                    }
                } else {
                    if (ParamValue *paramtow = subparams->GetPtr (param.rawName)) {
                        paramtow->val.uniStringValue = parentguidtxt;
                        paramtow->isValid = true;
                        ParamHelpers::AddParamValue2ParamDictElement (subguid, *paramtow, paramToWrite);
                        flag_write = true;
                        has_element = true;
                    }
                }
                if (flag_write)
                    break;
            }
        }
    }
    return has_element;
}

// --------------------------------------------------------------------
// Подсвечивает элементы, GUID которых указан в свойстве с описанием Sync_GUID
// --------------------------------------------------------------------
void SyncShowSubelement (const SyncSettings &syncSettings) {
    clock_t start, finish;
    double duration;
    start = clock ();
    GS::UniString fmane = "";
    GSErrCode err = NoError;
#ifndef AC_22
    GS::Array<API_Guid> guidArray_all = GetSelectedElements (true, false, syncSettings, false, false, false);
    GS::Array<API_Guid> guidArray = {};
    guidArray.SetCapacity (guidArray_all.GetSize ());
    for (const auto &guid : guidArray_all) {
        API_ElemTypeID elementType;
        if (GetTypeByGUID (guid, elementType) != NoError)
            continue;
        if (elementType == API_LabelID)
            continue;
        if (elementType == API_SectElemID) {
            API_Guid parentguid;
            GetParentGUIDSectElem (guid, parentguid, elementType);
            guidArray.Push (parentguid);
        } else {
            guidArray.Push (guid);
        }
    }
    if (guidArray.IsEmpty ())
        return;
    GS::Array<API_Neig> selNeigs = {};
    UnicGuidByGuid parentGuid = {};
    ParamDictValue propertyParams = {};
    int errcode = 0;
    if (!SyncGetSubelement (guidArray, parentGuid, EMPTYSTRING, errcode)) {
        if (SyncGetParentelement (guidArray, parentGuid, EMPTYSTRING, errcode)) {
            fmane = "Show Sub Element";
        } else {
            const Int32 iseng = ID_ADDON_STRINGS + isEng ();
            GS::UniString SubElementHotFoundIdString =
                RSGetIndString (iseng, SubElementHotFoundId, ACAPI_GetOwnResModule ());
            if (errcode > 0) {
                SubElementHotFoundIdString =
                    SubElementHotFoundIdString + LINEBRAKE +
                    RSGetIndString (iseng, SubElementHotFoundId + errcode, ACAPI_GetOwnResModule ());
            }
            ACAPI_WriteReport (SubElementHotFoundIdString, true);
            return;
        }
    } else {
        fmane = "Show Parent Element";
    }
    API_DatabaseInfo homedatabaseInfo = {};
    API_DatabaseInfo elementdatabaseInfo = {};
    bool checkdb = false;
    bool isfloorplan = false;
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
    int count_inv = 0;
    int count_all = 0;
    int count_otherplan = 0;
    int count_del = 0;
    API_Elem_Head tElemHead = {};
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
                    tElemHead.guid = guid;
                    GS::UniString name = "";
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
    const Int32 iseng = ID_ADDON_STRINGS + isEng ();
    if (count_otherplan > 0) {
        GS::UniString SubElementOtherPlanString =
            RSGetIndString (iseng, SubElementOtherPlanId, ACAPI_GetOwnResModule ());
        errmsg = errmsg + GS::UniString::Printf (" %d ", count_otherplan) + SubElementOtherPlanString + LINEBRAKE;
        fmane = fmane + GS::UniString::Printf (", %d on other floorplan", count_otherplan);
    }

    if (count_del > 0) {
        GS::UniString SubElementNotExsistString =
            RSGetIndString (iseng, SubElementNotExsistId, ACAPI_GetOwnResModule ());
        errmsg = errmsg + GS::UniString::Printf (" %d ", count_del) + SubElementNotExsistString + LINEBRAKE;
        fmane = fmane + GS::UniString::Printf (", %d not exsist", count_del);
    }
    if (count_inv > 0) {
        GS::UniString SubElementHiddenString = RSGetIndString (iseng, SubElementHiddenId, ACAPI_GetOwnResModule ());
        errmsg = errmsg + GS::UniString::Printf (" %d ", count_inv) + SubElementHiddenString + LINEBRAKE;
        fmane = fmane + GS::UniString::Printf (", %d on hidden layers/filters", count_inv);
    }
    if (!errmsg.IsEmpty ()) {
        GS::UniString SubElementTotalString = RSGetIndString (iseng, SubElementTotalId, ACAPI_GetOwnResModule ());
        errmsg = SubElementTotalString + GS::UniString::Printf (" %d, ", count_all) + LINEBRAKE + errmsg;
    }
    if (selNeigs.IsEmpty ()) {
        GS::UniString SubElementNoSelectString = RSGetIndString (iseng, SubElementNoSelectId, ACAPI_GetOwnResModule ());
        errmsg = SubElementNoSelectString + LINEBRAKE + errmsg;
        fmane = fmane + " Nothing to select - all hide!";
        finish = clock ();
        duration = (double)(finish - start) / CLOCKS_PER_SEC;
        GS::UniString time = GS::UniString::Printf (" %.3f s", duration);
        msg_rep (fmane, time, err, APINULLGuid);
        if (!errmsg.IsEmpty ())
            ACAPI_WriteReport (errmsg, true);
        return;
    }
    #if defined(AC_27) || defined(AC_28) || defined(AC_29)
    err = ACAPI_Selection_Select (selNeigs, true);
    if (err == NoError && errmsg.IsEmpty ())
        ACAPI_View_ZoomToSelected ();
    #else
    err = ACAPI_Element_Select (selNeigs, true);
    if (err == NoError && errmsg.IsEmpty ())
        ACAPI_Automate (APIDo_ZoomToSelectedID);
    #endif
    if (!errmsg.IsEmpty ()) {
        GS::UniString SubElementHalfString = RSGetIndString (iseng, SubElementHalfId, ACAPI_GetOwnResModule ());
        errmsg = SubElementHalfString + LINEBRAKE + errmsg;
        ACAPI_WriteReport (errmsg, true);
    }
#else
    fmane = fmane + " not work in AC22";
    ACAPI_WriteReport ("Function not work in AC22", true);
#endif
    finish = clock ();
    duration = (double)(finish - start) / CLOCKS_PER_SEC;
    GS::UniString time = GS::UniString::Printf (" %.3f s", duration);
    msg_rep (fmane, time, err, APINULLGuid);
    return;
}

// --------------------------------------------------------------------
// Получение словаря с GUID дочерних объектов для массива объектов
// --------------------------------------------------------------------
bool SyncGetParentelement (const GS::Array<API_Guid> &guidArray,
                           UnicGuidByGuid &parentGuid,
                           const GS::UniString &suffix,
                           int &errcode) {
#ifdef AC_22
    return false;
#else
    GSErrCode err = NoError;
    if (guidArray.IsEmpty ())
        return false;
    if (!ParamHelpers::isPropertyDefinitionRead ())
        return false;
    ParamDictValue &propertyParams = PROPERTYCACHE ().property;
    GS::HashTable<API_Guid, GS::Array<API_Guid>> classificationforread; // классификация найденных элементов
    for (auto &cItt : propertyParams) {
    #if defined(AC_28) || defined(AC_29)
        ParamValue param = cItt.value;
    #else
        ParamValue param = *cItt.value;
    #endif
        if (!param.definition.description.Contains (SYNCGUID))
            continue;
        if (!(suffix.IsEmpty () || param.definition.description.Contains (suffix)))
            continue;
        if (param.definition.valueType != API_PropertyStringValueType)
            continue;
        for (const auto &cls : param.definition.availability) {
            if (GS::Array<API_Guid> *unPtr = classificationforread.GetPtr (cls)) {
                unPtr->Push (param.definition.guid);
            } else {
                GS::Array<API_Guid> un;
                un.Push (param.definition.guid);
                classificationforread.Put (cls, std::move (un));
            }
        }
    }
    if (classificationforread.IsEmpty ()) {
        errcode = 1;
        return false;
    }
    // Переписываем родительские GUID
    for (const auto &guid : guidArray) {
        if (!parentGuid.ContainsKey (guid)) {
            UnicGuid un;
            parentGuid.Put (guid, std::move (un));
        }
    }
    bool find = false;
    ParamDictElement paramToRead;
    GS::Array<API_Guid> elemGuids;
    GS::Array<API_Property> properties;
    GS::Array<GS::UniString> rulestring_param;
    GS::Array<GS::UniString> local_scratch;
    for (const auto &cls : classificationforread) {
    #if defined(AC_28) || defined(AC_29)
        const API_Guid &classificationItemGuid = cls.key;
        const GS::Array<API_Guid> &propertyDefinitions = cls.value;
    #else
        const API_Guid &classificationItemGuid = *cls.key;
        const GS::Array<API_Guid> &propertyDefinitions = *cls.value;
    #endif
        elemGuids.Clear ();
        err = ACAPI_Element_GetElementsWithClassification (classificationItemGuid, elemGuids);
        if (err != NoError) {
            msg_rep (
                "SyncGetParentelement", "ACAPI_Element_GetElementsWithClassification", err, classificationItemGuid);
            continue;
        }
        for (const auto &subguid : elemGuids) {
            properties.Clear ();
            err = ACAPI_Element_GetPropertyValuesByGuid (subguid, propertyDefinitions, properties);
            if (err != NoError) {
                msg_rep ("SyncGetParentelement", "ACAPI_Element_GetPropertyValuesByGuid", err, subguid);
                continue;
            }
            for (const auto &prop : properties) {
    #if defined(AC_22) || defined(AC_23)
                if (!prop.isEvaluated)
                    continue;
    #else
                if (prop.status != API_Property_HasValue)
                    continue;
    #endif
                if (prop.isDefault)
                    continue;
                if (prop.value.singleVariant.variant.uniStringValue.IsEmpty ())
                    continue;
                rulestring_param.Clear ();
                StringSplt (
                    prop.value.singleVariant.variant.uniStringValue, SEMICOLON, rulestring_param, true, &local_scratch);
                for (const auto &rulestring : rulestring_param) {
                    API_Guid guid = APIGuidFromString (rulestring.ToCStr (0, MaxUSize, GChCode));
                    if (guid == APINULLGuid)
                        continue;
                    if (auto *un = parentGuid.GetPtr (guid)) {
                        find = true;
                        if (!un->ContainsKey (subguid)) {
                            bool isvisible = ACAPI_Element_Filter (subguid,
                                                                   APIFilt_OnVisLayer | APIFilt_IsVisibleByRenovation |
                                                                       APIFilt_IsInStructureDisplay);
                            un->Put (subguid, isvisible);
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
bool SyncGetSubelement (const GS::Array<API_Guid> &guidArray,
                        UnicGuidByGuid &parentGuid,
                        const GS::UniString &suffix,
                        int &errcode) {
    if (guidArray.IsEmpty ())
        return false;
    ParamDictElement paramToRead = {};
    if (!SyncGetSyncGUIDProperty (guidArray, paramToRead, suffix)) {
        errcode = 1;
        return false;
    }
    GS::Array<GS::UniString> rulestring_param;
    GS::Array<GS::UniString> local_scratch;
    for (const auto &cIt : paramToRead) {
#if defined(AC_28) || defined(AC_29)
        API_Guid subguid = cIt.key;
        const ParamDictValue &params = cIt.value;
#else
        API_Guid subguid = *cIt.key;
        const ParamDictValue &params = *cIt.value;
#endif
        for (auto &cItt : params) {
#if defined(AC_28) || defined(AC_29)
            ParamValue param = cItt.value;
#else
            ParamValue param = *cItt.value;
#endif
            if (param.isValid && !param.val.uniStringValue.IsEmpty ()) {
                rulestring_param.Clear ();
                StringSplt (param.val.uniStringValue, SEMICOLON, rulestring_param, true, &local_scratch);
                for (const auto &part : rulestring_param) {
                    API_Guid guid = APIGuidFromString (part.ToCStr (0, MaxUSize, GChCode));
                    if (guid == APINULLGuid)
                        continue;
                    auto *un = parentGuid.GetPtr (subguid);
                    if (un == nullptr) {
                        UnicGuid u;
                        parentGuid.Put (guid, std::move (u));
                        un = parentGuid.GetPtr (subguid);
                        if (un == nullptr) {
                            continue;
                        }
                    }
                    if (!un->ContainsKey (guid)) {
                        bool isvisible = ACAPI_Element_Filter (
                            subguid, APIFilt_OnVisLayer | APIFilt_IsVisibleByRenovation | APIFilt_IsInStructureDisplay);
                        un->Put (guid, isvisible);
                    }
                }
            }
        }
    }
    if (parentGuid.IsEmpty ())
        errcode = 2;
    return !parentGuid.IsEmpty ();
}

// --------------------------------------------------------------------
// Получение прочитанных свойств Sync_GUID для массива элементов
// --------------------------------------------------------------------
bool SyncGetSyncGUIDProperty (const GS::Array<API_Guid> &guidArray,
                              ParamDictElement &paramToRead,
                              const GS::UniString &suffix) {
    ParamDictValue paramDict = {};
    const ParamDictValue &propertyParams = PROPERTYCACHE ().property;
    for (const auto &cItt : propertyParams) {
#if defined(AC_28) || defined(AC_29)
        const ParamValue &param = cItt.value;
#else
        const ParamValue &param = *cItt.value;
#endif
        if (!param.definition.description.Contains (SYNCGUID))
            continue; // Проверяем - есть ли у свойства в описании SYNCGUID
        if (!(suffix.IsEmpty () || param.definition.description.Contains (suffix)))
            continue; // Проверяем - есть ли у свойства в описании суффикс
        if (paramDict.ContainsKey (param.rawName))
            continue; // Проверяем - есть ли уже в словаре свойство с таким именем
        for (const auto &guid : guidArray) {
            if (!ACAPI_Element_IsPropertyDefinitionAvailable (guid, param.definition.guid))
                continue; // Доступность свойства для элемента
            if (!paramDict.ContainsKey (param.rawName)) {
                paramDict.Put (param.rawName, param);
            } else {
                break;
            }
        }
    }
    if (paramDict.IsEmpty ())
        return false;
    for (const auto &guid : guidArray) {
        ParamHelpers::AddParamDictValue2ParamDictElement (guid, paramDict, paramToRead);
    }
    ParamHelpers::ElementsRead (paramToRead);
    return !paramToRead.IsEmpty ();
}
