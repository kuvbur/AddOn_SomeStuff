//------------ kuvbur 2022 ------------
#include	"ACAPinc.h"
#include	"APIEnvir.h"
#include	"Dimensions.hpp"
#include	"MEPv1.hpp"
#include	"ResetProperty.hpp"
#include	"Sync.hpp"
#include	<stdlib.h> /* atoi */
#include	<time.h>
#ifdef TESTING
#include "TestFunc.hpp"
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
    long time_start = clock ();
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
    if (HasDimAutotext ()) MonByType (API_DimensionID, syncSettings);
    long time_end = clock ();
    GS::UniString time = GS::UniString::Printf (" %d s", (time_end - time_start) / 1000);
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
            GetRelationsElement (guidArray[i], elementType, syncSettings, subelemGuids);
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
    // Сразу прочитаем свойства и разложим их по элементам
    ParamDictValue propertyParams;
    ParamHelpers::AllPropertyDefinitionToParamDict (propertyParams);
    ParamHelpers::GetAllInfoToParamDict (propertyParams);
    ParamHelpers::GetAllGlobToParamDict (propertyParams);
    ClassificationFunc::SystemDict systemdict;
    ClassificationFunc::GetAllClassification (systemdict);
    if (propertyParams.IsEmpty ()) return;
    if (ResetProperty (propertyParams)) return;
    GS::UniString	funcname ("Sync All");
    bool flag_chanel = false;
    ParamDictElement paramToWrite;
    GS::Int32 nPhase = 1;
#if defined(AC_27) || defined(AC_28)
    ACAPI_ProcessWindow_InitProcessWindow (&funcname, &nPhase);
#else
    ACAPI_Interface (APIIo_InitProcessWindowID, &funcname, &nPhase);
#endif
    long time_start = clock ();
    int dummymode = IsDummyModeOn ();
    if (!flag_chanel && syncSettings.objS) flag_chanel = SyncByType (API_ObjectID, syncSettings, nPhase, propertyParams, paramToWrite, dummymode, systemdict);
    nPhase = nPhase + 1;
    if (!flag_chanel && syncSettings.objS) flag_chanel = SyncByType (API_LampID, syncSettings, nPhase, propertyParams, paramToWrite, dummymode, systemdict);
    nPhase = nPhase + 1;
    if (!flag_chanel && syncSettings.widoS) flag_chanel = SyncByType (API_WindowID, syncSettings, nPhase, propertyParams, paramToWrite, dummymode, systemdict);
    nPhase = nPhase + 1;
    if (!flag_chanel && syncSettings.widoS) flag_chanel = SyncByType (API_DoorID, syncSettings, nPhase, propertyParams, paramToWrite, dummymode, systemdict);
    nPhase = nPhase + 1;
    if (!flag_chanel && syncSettings.widoS) flag_chanel = SyncByType (API_SkylightID, syncSettings, nPhase, propertyParams, paramToWrite, dummymode, systemdict);
    nPhase = nPhase + 1;
    if (!flag_chanel && syncSettings.objS) flag_chanel = SyncByType (API_ZoneID, syncSettings, nPhase, propertyParams, paramToWrite, dummymode, systemdict);
    nPhase = nPhase + 1;
    if (!flag_chanel && syncSettings.wallS) flag_chanel = SyncByType (API_WallID, syncSettings, nPhase, propertyParams, paramToWrite, dummymode, systemdict);
    nPhase = nPhase + 1;
    if (!flag_chanel && syncSettings.wallS) flag_chanel = SyncByType (API_SlabID, syncSettings, nPhase, propertyParams, paramToWrite, dummymode, systemdict);
    nPhase = nPhase + 1;
    if (!flag_chanel && syncSettings.wallS) flag_chanel = SyncByType (API_ColumnID, syncSettings, nPhase, propertyParams, paramToWrite, dummymode, systemdict);
    nPhase = nPhase + 1;
    if (!flag_chanel && syncSettings.wallS) flag_chanel = SyncByType (API_BeamID, syncSettings, nPhase, propertyParams, paramToWrite, dummymode, systemdict);
    nPhase = nPhase + 1;
    if (!flag_chanel && syncSettings.wallS) flag_chanel = SyncByType (API_RoofID, syncSettings, nPhase, propertyParams, paramToWrite, dummymode, systemdict);
    nPhase = nPhase + 1;
    if (!flag_chanel && syncSettings.wallS) flag_chanel = SyncByType (API_MeshID, syncSettings, nPhase, propertyParams, paramToWrite, dummymode, systemdict);
    nPhase = nPhase + 1;
    if (!flag_chanel && syncSettings.wallS) flag_chanel = SyncByType (API_MorphID, syncSettings, nPhase, propertyParams, paramToWrite, dummymode, systemdict);
    nPhase = nPhase + 1;
    if (!flag_chanel && syncSettings.cwallS) flag_chanel = SyncByType (API_CurtainWallID, syncSettings, nPhase, propertyParams, paramToWrite, dummymode, systemdict);
    nPhase = nPhase + 1;
    if (!flag_chanel && syncSettings.cwallS) flag_chanel = SyncByType (API_RailingID, syncSettings, nPhase, propertyParams, paramToWrite, dummymode, systemdict);
    nPhase = nPhase + 1;
    if (!flag_chanel) flag_chanel = SyncByType (API_SectElemID, syncSettings, nPhase, propertyParams, paramToWrite, dummymode, systemdict);
    nPhase = nPhase + 1;
    if (!flag_chanel) flag_chanel = SyncByType (API_LabelID, syncSettings, nPhase, propertyParams, paramToWrite, dummymode, systemdict);
    long time_end = clock ();
    GS::UniString time = GS::UniString::Printf (" %d s", (time_end - time_start) / 1000);
    msg_rep ("SyncAll - read", time, NoError, APINULLGuid);
    GS::Array<API_Guid> rereadelem;
    if (!paramToWrite.IsEmpty ()) {
        GS::UniString undoString = RSGetIndString (ID_ADDON_STRINGS + isEng (), UndoSyncId, ACAPI_GetOwnResModule ());
        ACAPI_CallUndoableCommand (undoString, [&]() -> GSErrCode {
            long time_start = clock ();
            GS::UniString title = GS::UniString::Printf ("Writing data to %d elements : ", paramToWrite.GetSize ()); short i = 1;
#if defined(AC_27) || defined(AC_28)
            bool showPercent = false;
            Int32 maxval = 2;
            ACAPI_ProcessWindow_SetNextProcessPhase (&title, &maxval, &showPercent);
#else
            ACAPI_Interface (APIIo_SetNextProcessPhaseID, &title, &i);
#endif
            rereadelem = ParamHelpers::ElementsWrite (paramToWrite);
            long time_end = clock ();
            GS::UniString time = title + GS::UniString::Printf (" %d s", (time_end - time_start) / 1000);
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
        SyncArray (syncSettings, rereadelem, systemdict);
    }
#if defined(TESTING)
    DBprnt ("SyncAndMonAll end");
#endif
#if defined(AC_27) || defined(AC_28)
    ACAPI_ProcessWindow_CloseProcessWindow ();
#else
    ACAPI_Interface (APIIo_CloseProcessWindowID, nullptr, nullptr);
#endif
}

// -----------------------------------------------------------------------------
// Синхронизация элементов по типу
// -----------------------------------------------------------------------------
bool SyncByType (const API_ElemTypeID& elementType, const SyncSettings& syncSettings, GS::Int32& nPhase, ParamDictValue& propertyParams, ParamDictElement& paramToWrite, int dummymode, ClassificationFunc::SystemDict& systemdict)
{
    GS::UniString		subtitle;
    GSErrCode			err = NoError;
    GS::Array<API_Guid>	guidArray;
    long time_start = clock ();
    ACAPI_Element_GetElemList (elementType, &guidArray, APIFilt_IsEditable | APIFilt_HasAccessRight | APIFilt_InMyWorkspace);
    if (guidArray.IsEmpty ()) return false;
#ifdef TESTING
    TestFunc::ResetSyncPropertyArray (guidArray);
#endif
#if defined AC_26 || defined AC_27 || defined AC_28
    API_ElemType elemType;
    elemType.typeID = elementType;
#if defined(AC_27) || defined(AC_28)
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

    // Словарь со всеми возможными определениями свойств
    if (systemdict.IsEmpty ()) ClassificationFunc::GetAllClassification (systemdict);
    if (!ParamHelpers::hasProperyDefinition (propertyParams)) ParamHelpers::AllPropertyDefinitionToParamDict (propertyParams);
    if (!ParamHelpers::hasInfo (propertyParams)) ParamHelpers::GetAllInfoToParamDict (propertyParams);
    if (!ParamHelpers::hasGlob (propertyParams)) ParamHelpers::GetAllGlobToParamDict (propertyParams);
    if (propertyParams.IsEmpty ()) return true;
    bool flag_chanel = false;
    for (UInt32 i = 0; i < guidArray.GetSize (); i++) {
        SyncElement (guidArray[i], syncSettings, propertyParams, paramToWrite, dummymode, systemdict);
#if defined(AC_27) || defined(AC_28)
        if (i % 10 == 0) ACAPI_ProcessWindow_SetNextProcessPhase (&subtitle, &maxval, &showPercent);
#else
        if (i % 10 == 0) ACAPI_Interface (APIIo_SetNextProcessPhaseID, &subtitle, &i);
#endif
    }
    GS::UniString intString = GS::UniString::Printf (" %d qty", guidArray.GetSize ());
    long time_end = clock ();
    GS::UniString time = GS::UniString::Printf (" %d s", (time_end - time_start) / 1000);
    msg_rep ("SyncByType", subtitle + intString + time, NoError, APINULLGuid);
    return flag_chanel;
}

// -----------------------------------------------------------------------------
// Синхронизация элемента и его подэлементов
// -----------------------------------------------------------------------------
void SyncElement (const API_Guid& elemGuid, const SyncSettings& syncSettings, ParamDictValue& propertyParams, ParamDictElement& paramToWrite, int dummymode, ClassificationFunc::SystemDict& systemdict)
{
    API_ElemTypeID elementType;
    GSErrCode err = GetTypeByGUID (elemGuid, elementType);
    if (err != NoError) return;
    if (elementType == API_LabelID) {
        SyncLabelScope (elemGuid, propertyParams, paramToWrite);
        return;
    }
    API_Guid elemGuid_ = elemGuid;
    if (elementType == API_SectElemID) {
        GetParentGUIDSectElem (elemGuid, elemGuid_, elementType);
    }
    // Получаем список связанных элементов
    GS::Array<API_Guid> subelemGuids;
    GetRelationsElement (elemGuid_, elementType, syncSettings, subelemGuids);
    if (systemdict.IsEmpty ()) ClassificationFunc::GetAllClassification (systemdict);
    SyncData (elemGuid_, syncSettings, subelemGuids, propertyParams, paramToWrite, dummymode, systemdict);
    if (!subelemGuids.IsEmpty () && SyncRelationsElement (elementType, syncSettings)) {
        if (subelemGuids.GetSize () > 3) {
            if (!ParamHelpers::hasProperyDefinition (propertyParams)) ParamHelpers::AllPropertyDefinitionToParamDict (propertyParams);
            if (!ParamHelpers::hasInfo (propertyParams)) ParamHelpers::GetAllInfoToParamDict (propertyParams);
            if (!ParamHelpers::hasGlob (propertyParams)) ParamHelpers::GetAllGlobToParamDict (propertyParams);
        }
        for (UInt32 i = 0; i < subelemGuids.GetSize (); ++i) {
            API_Guid subelemGuid = subelemGuids[i];
            if (subelemGuid != elemGuid_) {
                GS::Array<API_Guid> epm;
                SyncData (subelemGuid, syncSettings, epm, propertyParams, paramToWrite, dummymode, systemdict);
            }
        }
    }
}

// -----------------------------------------------------------------------------
// Запускает обработку выбранных, заданных в настройке
// -----------------------------------------------------------------------------
void SyncSelected (const SyncSettings& syncSettings)
{
    GS::UniString fmane = "Sync Selected";
    GS::Array<API_Guid> guidArray = GetSelectedElements (false, true, true);
    if (guidArray.IsEmpty ()) return;
    ClassificationFunc::SystemDict systemdict;
    ClassificationFunc::GetAllClassification (systemdict);
    GS::Array<API_Guid> rereadelem = SyncArray (syncSettings, guidArray, systemdict);
    if (!rereadelem.IsEmpty ()) {
#if defined(TESTING)
        DBprnt ("===== REREAD =======");
#endif
        SyncArray (syncSettings, rereadelem, systemdict);
    }
}

// -----------------------------------------------------------------------------
// Запускает обработку переданного массива
// -----------------------------------------------------------------------------
GS::Array<API_Guid> SyncArray (const SyncSettings& syncSettings, GS::Array<API_Guid>& guidArray, ClassificationFunc::SystemDict& systemdict)
{
    GS::Array<API_Guid> rereadelem;
    if (guidArray.IsEmpty ()) return rereadelem;
    GS::UniString funcname = "Sync Selected";
    ParamDictValue propertyParams = {};
    ParamHelpers::AllPropertyDefinitionToParamDict (propertyParams);
    ParamHelpers::GetAllInfoToParamDict (propertyParams);
    ParamHelpers::GetAllGlobToParamDict (propertyParams);
    if (systemdict.IsEmpty ()) ClassificationFunc::GetAllClassification (systemdict);
    ParamDictElement paramToWrite;
    GS::UniString subtitle = GS::UniString::Printf ("Reading data from %d elements", guidArray.GetSize ());
    GS::Int32 nPhase = 1;
    int dummymode = IsDummyModeOn ();
#if defined(AC_27) || defined(AC_28)
    bool showPercent = true;
    Int32 maxval = guidArray.GetSize ();
    ACAPI_ProcessWindow_InitProcessWindow (&funcname, &nPhase);
#else
    ACAPI_Interface (APIIo_InitProcessWindowID, &funcname, &nPhase);
#endif
    long time_start = clock ();
    for (UInt32 i = 0; i < guidArray.GetSize (); i++) {
        SyncElement (guidArray[i], syncSettings, propertyParams, paramToWrite, dummymode, systemdict);
#if defined(AC_27) || defined(AC_28)
        if (i % 10 == 0) ACAPI_ProcessWindow_SetNextProcessPhase (&subtitle, &maxval, &showPercent);
#else
        if (i % 10 == 0) ACAPI_Interface (APIIo_SetNextProcessPhaseID, &subtitle, &i);
#endif
#if defined(AC_27) || defined(AC_28)
        if (ACAPI_ProcessWindow_IsProcessCanceled ()) return rereadelem;
#else
        if (ACAPI_Interface (APIIo_IsProcessCanceledID, nullptr, nullptr)) return rereadelem;
#endif
    }
    GS::UniString intString = GS::UniString::Printf (" %d qty", guidArray.GetSize ());
    long time_end = clock ();
    GS::UniString time = GS::UniString::Printf (" %d s", (time_end - time_start) / 1000);
    msg_rep ("SyncSelected - read", subtitle + intString + time, NoError, APINULLGuid);
    if (!paramToWrite.IsEmpty ()) {
        GS::UniString undoString = RSGetIndString (ID_ADDON_STRINGS + isEng (), UndoSyncId, ACAPI_GetOwnResModule ());
        ACAPI_CallUndoableCommand (undoString, [&]() -> GSErrCode {
            long time_start = clock ();
            GS::UniString title = GS::UniString::Printf ("Writing data to %d elements : ", paramToWrite.GetSize ()); short i = 1;
#if defined(AC_27) || defined(AC_28)
            ACAPI_ProcessWindow_SetNextProcessPhase (&subtitle, &maxval, &showPercent);
#else
            ACAPI_Interface (APIIo_SetNextProcessPhaseID, &subtitle, &i);
#endif
            rereadelem = ParamHelpers::ElementsWrite (paramToWrite);
            long time_end = clock ();
            GS::UniString time = title + GS::UniString::Printf (" %d s", (time_end - time_start) / 1000);
            msg_rep ("SyncSelected - write", time, NoError, APINULLGuid);
            return NoError;
        });
    } else {
        msg_rep ("SyncSelected - write", "No data to write", NoError, APINULLGuid);
    }
    ParamHelpers::InfoWrite (paramToWrite);
#if defined(AC_27) || defined(AC_28)
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
    GS::UniString fmane = "Run parameter script";

    // Запомним номер текущей БД и комбинацию слоёв для восстановления по окончанию работы
    API_AttributeIndex layerCombIndex;
    API_DatabaseInfo databaseInfo;
    BNZeroMemory (&databaseInfo, sizeof (API_DatabaseInfo));
    GSErrCode err = NoError;
#if defined(AC_27) || defined(AC_28)
    err = ACAPI_Navigator_GetCurrLayerComb (&layerCombIndex);
#else
    err = ACAPI_Environment (APIEnv_GetCurrLayerCombID, &layerCombIndex);
#endif
    if (err != NoError) return;
#if defined(AC_27) || defined(AC_28)
    err = ACAPI_Database_GetCurrentDatabase (&databaseInfo);
#else
    err = ACAPI_Database (APIDb_GetCurrentDatabaseID, &databaseInfo, nullptr);
#endif

    if (err != NoError) return;
    CallOnSelectedElemSettings (RunParam, false, true, syncSettings, fmane, false);
    SyncSelected (syncSettings);
#if defined(AC_27) || defined(AC_28)
    if (layerCombIndex.IsPositive ()) err = ACAPI_Navigator_ChangeCurrLayerComb (&layerCombIndex); // Устанавливаем комбинацию слоёв
    err = ACAPI_Database_ChangeCurrentDatabase (&databaseInfo);
#else
    if (layerCombIndex != 0) err = ACAPI_Environment (APIEnv_ChangeCurrLayerCombID, &layerCombIndex); // Устанавливаем комбинацию слоёв
    err = ACAPI_Database (APIDb_ChangeCurrentDatabaseID, &databaseInfo, nullptr);
#endif
}

// -----------------------------------------------------------------------------
// Запуск скрипта параметра элемента
// -----------------------------------------------------------------------------
void RunParam (const API_Guid& elemGuid, const SyncSettings& syncSettings)
{
#if defined(TESTING)
    DBprnt ("RunParam");
#endif
    API_Elem_Head	tElemHead;
    BNZeroMemory (&tElemHead, sizeof (API_Elem_Head));
    tElemHead.guid = elemGuid;
    GSErrCode	err = ACAPI_Element_GetHeader (&tElemHead);
    if (err != NoError) return;
    API_DatabaseInfo databaseInfo;
    API_DatabaseInfo dbInfo;
#if defined(AC_27) || defined(AC_28)
    err = ACAPI_Database_GetContainingDatabase (&tElemHead.guid, &dbInfo);
#else
    err = ACAPI_Database (APIDb_GetContainingDatabaseID, &tElemHead.guid, &dbInfo);
#endif
    if (err != NoError) return;
#if defined(AC_27) || defined(AC_28)
    err = ACAPI_Database_GetCurrentDatabase (&databaseInfo);
#else
    err = ACAPI_Database (APIDb_GetCurrentDatabaseID, &databaseInfo, nullptr);
#endif
    if (err != NoError) return;
    if (dbInfo.databaseUnId != databaseInfo.databaseUnId) {
#if defined(AC_27) || defined(AC_28)
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
#if defined(AC_27) || defined(AC_28)
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
void SyncData (const API_Guid& elemGuid, const SyncSettings& syncSettings, GS::Array<API_Guid>& subelemGuids, ParamDictValue& propertyParams, ParamDictElement& paramToWrite, int dummymode, ClassificationFunc::SystemDict& systemdict)
{
    GSErrCode	err = NoError;
    API_ElemTypeID elementType;
    if (!IsElementEditable (elemGuid, syncSettings, true, elementType)) return;
    // Если включён мониторинг - привязываем элемент к отслеживанию
    if (syncSettings.syncMon) {
        err = AttachObserver (elemGuid, syncSettings);
        if (err == APIERR_LINKEXIST)
            err = NoError;
        if (err != NoError) {
            msg_rep ("SyncData", "AttachObserver", err, elemGuid);
            return;
        }
    }
    if (elementType == API_DimensionID) return;
    ClassificationFunc::SetAutoclass (systemdict, elemGuid);
    GS::Array<API_PropertyDefinition> definitions;
    err = ACAPI_Element_GetPropertyDefinitions (elemGuid, API_PropertyDefinitionFilter_UserDefined, definitions);
    if (err != NoError) {
        msg_rep ("SyncData", "ACAPI_Element_GetPropertyDefinitions", err, elemGuid);
        return;
    }

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
        syncall = GetElemState (elemGuid, definitions, "Sync_flag", flagfindall);
        synccoord = GetElemState (elemGuid, definitions, "Sync_correct_flag", flagfindcoord);
        syncclass = GetElemState (elemGuid, definitions, "Sync_class_flag", flagfindclass);
    }
    if (!syncall && !synccoord && !syncclass) return; //Если оба свойства-флага ложь - выходим
    if (syncall && !flagfindcoord) synccoord = true; //Если флаг координат не найден - проверку всё равно делаем
    if (syncall && !flagfindclass) syncclass = true;
    GS::Array <WriteData> mainsyncRules;
    bool hassubguid = false;
    if (syncall) hassubguid = ParamHelpers::SubGuid_GetParamValue (elemGuid, propertyParams, definitions);
    ParamDictElement paramToRead; // Словарь с параметрами для чтения
    bool hasSub = false;
    for (UInt32 i = 0; i < definitions.GetSize (); i++) {
        // Получаем список правил синхронизации из всех свойств
        ParseSyncString (elemGuid, elementType, definitions[i], mainsyncRules, paramToRead, hasSub, propertyParams, syncall, synccoord, syncclass); // Парсим описание свойства
    }
    if (mainsyncRules.IsEmpty ()) return;
    if (propertyParams.IsEmpty () || hasSub) {
        if (hasSub) {
            if (!ParamHelpers::hasProperyDefinition (propertyParams)) ParamHelpers::AllPropertyDefinitionToParamDict (propertyParams);
        } else {
            ParamHelpers::AllPropertyDefinitionToParamDict (propertyParams, definitions);
        }
    }
    if (propertyParams.IsEmpty ()) return;

    // Заполняем правила синхронизации с учётом субэлементов, попутно заполняем словарь параметров для чтения/записи
    WriteDict syncRules; // Словарь с правилами для каждого элемента
    SyncAddSubelement (subelemGuids, mainsyncRules, syncRules, paramToRead);
    mainsyncRules.Clear ();
    subelemGuids.Push (elemGuid); // Это теперь список всех элементов для синхронизации

    // Читаем все возможные свойства
    ParamHelpers::ElementsRead (paramToRead, propertyParams, systemdict);

    SyncCalcRule (syncRules, subelemGuids, paramToRead, paramToWrite);

    // Некоторые свойства, возможно, ссылались на изменённые. Чтоб не запускать полную синхронизацию ещё раз
    ParamHelpers::CompareParamDictElement (paramToWrite, paramToRead);
    SyncCalcRule (syncRules, subelemGuids, paramToRead, paramToWrite);
    return;
}

void SyncCalcRule (const WriteDict& syncRules, const GS::Array<API_Guid>& subelemGuids, const ParamDictElement& paramToRead, ParamDictElement& paramToWrite)
{
    // Выбираем по-элементно параметры для чтения и записи, формируем словарь
    for (UInt32 i = 0; i < subelemGuids.GetSize (); i++) {
        API_Guid elemGuid = subelemGuids[i];
        GS::Array <WriteData> writeSubs;
        if (syncRules.ContainsKey (elemGuid)) writeSubs = syncRules.Get (elemGuid);
        if (!writeSubs.IsEmpty ()) {

            // Заполняем значения параметров чтения/записи из словаря
            for (UInt32 j = 0; j < writeSubs.GetSize (); j++) {
                WriteData writeSub = writeSubs.Get (j);
                API_Guid elemGuidTo = writeSub.guidTo;
                API_Guid elemGuidFrom = writeSub.guidFrom;

                // Проверяем - есть ли вообще эти элементы в словаре параметров
                if (paramToRead.ContainsKey (elemGuidTo) && paramToRead.ContainsKey (elemGuidFrom)) {
                    GS::UniString rawNameFrom = writeSub.paramFrom.rawName;
                    GS::UniString rawNameTo = writeSub.paramTo.rawName;
                    ParamDictValue paramsTo = paramToRead.Get (elemGuidTo);
                    ParamDictValue paramsFrom;
                    if (elemGuidFrom == elemGuidTo) {
                        paramsFrom = paramsTo;
                    } else {
                        paramsFrom = paramToRead.Get (elemGuidFrom);
                    }

                    // Проверяем наличие имён в словаре параметров
                    if (paramsTo.ContainsKey (rawNameTo) && paramsFrom.ContainsKey (rawNameFrom)) {
                        ParamValue paramFrom = paramsFrom.Get (rawNameFrom);
                        ParamValue paramTo = paramsTo.Get (rawNameTo);
                        FormatString formatstring = writeSub.formatstring;

                        //Сопоставляем и записываем, если значения отличаются
                        if (ParamHelpers::CompareParamValue (paramFrom, paramTo, formatstring)) {
                            ParamHelpers::AddParamValue2ParamDictElement (paramTo, paramToWrite);
                        }
                    }
                }
            }
        }
    }
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
        GS::Array <WriteData> rules;
        rules.Push (writeSub);
        syncRules.Add (elemGuid, rules);
    }
    ParamHelpers::AddParamValue2ParamDictElement (writeSub.paramFrom, paramToRead);
    ParamHelpers::AddParamValue2ParamDictElement (writeSub.paramTo, paramToRead);
}

// -----------------------------------------------------------------------------
// Парсит описание свойства, заполняет массив с правилами (GS::Array <WriteData>)
// -----------------------------------------------------------------------------
bool ParseSyncString (const API_Guid& elemGuid, const  API_ElemTypeID& elementType, const API_PropertyDefinition& definition, GS::Array <WriteData>& syncRules, ParamDictElement& paramToRead, bool& hasSub, ParamDictValue& propertyParams, bool syncall, bool synccoord, bool syncclass)
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
        ParamDictValue paramDict;
        ParamValue paramdef; //Свойство, из которого получено правило
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
        GS::Array<GS::UniString> rulestring;
        UInt32 nrule = StringSplt (description_string, "Sync_", rulestring, "{"); // Проверяем количество правил
        ParamValue paramdef; //Свойство, из которого получено правило
        ParamHelpers::ConvertToParamValue (paramdef, definition);
        paramdef.fromGuid = elemGuid;
        //Проходим по каждому правилу и извлекаем из него правило синхронизации (WriteDict syncRules)
        //и словарь уникальных параметров для чтения/записи (ParamDictElement paramToRead)
        for (UInt32 i = 0; i < nrule; i++) {
            ParamValue param;
            int syncdirection = SYNC_NO; // Направление синхронизации
            GS::UniString rawparamName = ""; //Имя параметра/свойства с указанием типа синхронизации, для ключа словаря
            GS::Array<GS::UniString> ignorevals; //Игнорируемые значения
            FormatString stringformat;
            GS::UniString rulestring_one = rulestring[i];
            API_Guid elemGuidfrom = elemGuid; // Элемент, из которого читаем данные
            API_Guid elemGuidto = elemGuid; // Элемент, в котороый записываем данные
            // Копировать из другого элемента
            if (rulestring_one.Contains ("from_GUID{")) {
                GS::Array<GS::UniString> params;
                rulestring_one.ReplaceAll ("from_GUID", "");
                UInt32 nparams = StringSplt (rulestring_one, ";", params);
                if (nparams == 2) {
                    GS::UniString rawname = "";
                    Name2Rawname (params[0], rawname);
                    if (propertyParams.ContainsKey (rawname)) {
                        if (propertyParams.Get (rawname).isValid) {
                            elemGuidfrom = APIGuidFromString (propertyParams.Get (rawname).val.uniStringValue.ToCStr (0, MaxUSize, GChCode));
                            rulestring_one = "Sync_from{" + params[1];
                        }
                    }
                }
            }
            // Копировать в другой элемент
            if (rulestring_one.Contains ("to_GUID{")) {
                GS::Array<GS::UniString> params;
                rulestring_one.ReplaceAll ("to_GUID", "");
                UInt32 nparams = StringSplt (rulestring_one, ";", params);
                if (nparams == 2) {
                    GS::UniString rawname = "";
                    Name2Rawname (params[0], rawname);
                    if (propertyParams.ContainsKey (rawname)) {
                        if (propertyParams.Get (rawname).isValid) {
                            elemGuidto = APIGuidFromString (propertyParams.Get (rawname).val.uniStringValue.ToCStr (0, MaxUSize, GChCode));
                            param.fromGuid = elemGuidto;
                            rulestring_one = "Sync_to{" + params[1];
                        }
                    }
                }
            }
            if (SyncString (elementType, rulestring_one, syncdirection, param, ignorevals, stringformat, syncall, synccoord, syncclass)) {
                hasRule = true;
                WriteData writeOne;
                writeOne.formatstring = stringformat;
                writeOne.ignorevals = ignorevals;
                if (param.fromCoord && param.rawName.Contains ("north_dir")) {
                    ParamDictValue paramDict;
                    ParamHelpers::AddValueToParamDictValue (paramDict, "@info:glob_north_dir");
                    ParamHelpers::AddParamDictValue2ParamDictElement (elemGuid, paramDict, paramToRead);
                }

                // Вытаскиваем параметры для материалов, если такие есть
                if (param.fromMaterial) {
                    ParamDictValue paramDict;
                    GS::UniString templatestring = param.val.uniStringValue; //Строка с форматом числа
                    if (ParamHelpers::ParseParamNameMaterial (templatestring, paramDict)) {
                        param.val.uniStringValue = templatestring;

                        // Свойств со спецтекстом может быть несколько (случайно)
                        // Тут будут костыли, которые хорошо бы убрать
                        for (UInt32 inx = 0; inx < 20; inx++) {
                            ParamHelpers::AddValueToParamDictValue (paramDict, "@property:sync_name" + GS::UniString::Printf ("%d", inx));
                        }
                        ParamHelpers::AddParamDictValue2ParamDictElement (elemGuid, paramDict, paramToRead);
                        hasSub = true; // Нужно будет прочитать все свойства
                    }
                }
                if (!param.fromMaterial && param.val.hasFormula) {
                    ParamDictValue paramDict;
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
        paramNamePrefix = "{@id:";
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
            paramNamePrefix = "{@gdl:";
            synctypefind = true;
        }
    }
    if (synctypefind == false) {
        if (name.Contains ("Property:")) {
            name.ReplaceAll ("Property:", "");
            paramNamePrefix = "{@property:";
            synctypefind = true;
        }
    }
    if (synctypefind == false) {
        if (name.Contains ("Morph:")) {
            name.ReplaceAll ("Morph:", "");
            paramNamePrefix = "{@morph:";
            synctypefind = true;
        }
    }
    if (synctypefind == false) {
        if (name.Contains ("Coord:")) {
            name.ReplaceAll ("Coord:", "");
            paramNamePrefix = "{@coord:";
            synctypefind = true;
        }
    }
    if (synctypefind == false) {
        if (name.Contains ("Info:")) {
            name.ReplaceAll ("Info:", "");
            paramNamePrefix = "{@info:";
            synctypefind = true;
        }
    }
    if (synctypefind == false) {
        if (name.Contains ("IFC:")) {
            name.ReplaceAll ("IFC:", "");
            paramNamePrefix = "{@ifc:";
            synctypefind = true;
        }
    }
    if (synctypefind == false) {
        if (name.Contains ("Glob:")) {
            name.ReplaceAll ("Glob:", "");
            paramNamePrefix = "{@glob:";
            synctypefind = true;
        }
    }
    if (synctypefind == false) {
        if (name.Contains ("Class:")) {
            name.ReplaceAll ("Class:", "");
            paramNamePrefix = "{@class:";
            synctypefind = true;
        }
    }
    if (synctypefind == false) {
        if (name.Contains ("Attribute:")) {
            synctypefind = true;
            name.ReplaceAll ("Attribute:", "");
            paramNamePrefix = "{@attrib:";
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
bool SyncString (const  API_ElemTypeID& elementType, GS::UniString rulestring_one, int& syncdirection, ParamValue& param, GS::Array<GS::UniString>& ignorevals, FormatString& stringformat, bool syncall, bool synccoord, bool syncclass)
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
    bool hasformula = rulestring_one.Contains ('<') && rulestring_one.Contains ('>');
    if (rulestring_one.Contains ("QRCode:")) {
        rulestring_one.ReplaceAll ("QRCode:", "");
        param.toQRCode = true;
    }
    if (synctypefind == false) {
        if (rulestring_one.Contains ("{id}") || rulestring_one.Contains ("{ID}")) {
            paramNamePrefix = "{@id:";
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
            paramNamePrefix = "{@gdl:";
            param.fromGDLparam = true;
            synctypefind = true;
        }
    }
    GS::UniString stringformat_raw = "";
    if (synctypefind == false) {
        if (rulestring_one.Contains ("Material:") && (rulestring_one.Contains ('"') || hasformula)) {
            synctypefind = true;
            rulestring_one.ReplaceAll ("Material:", "");
            rulestring_one.ReplaceAll ("{Layers;", "{Layers,20;");
            rulestring_one.ReplaceAll ("{Layers_inv;", "{Layers_inv,20;");
            rulestring_one.ReplaceAll ("{Layers_auto;", "{Layers_auto,20;");
            paramNamePrefix = "{@material:";
            GS::UniString templatestring = "";
            if (hasformula) {
                if (rulestring_one.Contains ('"')) {
                    templatestring = rulestring_one.GetSubstring ('"', '"', 0);
                    FormatStringFunc::GetFormatStringFromFormula (rulestring_one, templatestring, stringformat_raw);
                    stringformat = FormatStringFunc::ParseFormatString (stringformat_raw);
                    param.val.uniStringValue = templatestring;
                } else {
                    templatestring = rulestring_one.GetSubstring ('<', '>', 0);
                    FormatStringFunc::GetFormatStringFromFormula (rulestring_one, templatestring, stringformat_raw);
                    stringformat = FormatStringFunc::ParseFormatString (stringformat_raw);
                    param.val.uniStringValue = '<' + templatestring + '>';
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
                short pen = std::atoi (penstring.ToCStr ());
                if (pen > 0) param.composite_pen = pen;
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
                templatestring = rulestring_one.GetSubstring ('<', '>', 0);
                FormatStringFunc::GetFormatStringFromFormula (rulestring_one, templatestring, stringformat_raw);
                stringformat = FormatStringFunc::ParseFormatString (stringformat_raw);
                param.val.uniStringValue = '<' + templatestring + '>';
            }
            rulestring_one.ReplaceAll (templatestring, "");
            rulestring_one.ReplaceAll (stringformat_raw, "");
            param.val.formatstring = stringformat;
            synctypefind = true;
            paramNamePrefix = "{@formula:";
            param.val.hasFormula = true;
            syncdirection = SYNC_FROM;
        }
    }
    if (synctypefind == false) {
        if (rulestring_one.Contains ("Morph:")) {
            synctypefind = true;
            rulestring_one.ReplaceAll ("Morph:", "");
            paramNamePrefix = "{@morph:";
            param.fromMorph = true;
            syncdirection = SYNC_FROM;
        }
    }
    if (synctypefind == false) {
        if (rulestring_one.Contains ("Coord:")) {
            synctypefind = true;
            rulestring_one.ReplaceAll ("Coord:", "");
            paramNamePrefix = "{@coord:";
            param.fromCoord = true;
            if (rulestring_one.Contains ("orth")) param.fromGlob = true;
            if (!rulestring_one.Contains ("symb_pos_") && !rulestring_one.Contains ("symb_rotangle")) syncdirection = SYNC_FROM;
        }
    }
    if (synctypefind == false) {
        if (rulestring_one.Contains ("Property:")) {
            synctypefind = true;
            rulestring_one.ReplaceAll ("Property:", "");
            paramNamePrefix = "{@property:";
            param.fromProperty = true;
        }
    }
    if (synctypefind == false) {
        if (rulestring_one.Contains ("Info:")) {
            synctypefind = true;
            rulestring_one.ReplaceAll ("Info:", "");
            paramNamePrefix = "{@info:";
            param.fromInfo = true;
        }
    }
    if (synctypefind == false) {
        if (rulestring_one.Contains ("IFC:")) {
            synctypefind = true;
            rulestring_one.ReplaceAll ("IFC:", "");
            paramNamePrefix = "{@ifc:";
            param.fromIFCProperty = true;
            syncdirection = SYNC_FROM;
        }
    }
    if (synctypefind == false) {
        if (rulestring_one.Contains ("Glob:")) {
            synctypefind = true;
            rulestring_one.ReplaceAll ("Glob:", "");
            paramNamePrefix = "{@glob:";
            param.fromGlob = true;
            syncdirection = SYNC_FROM;
        }
    }
    if (synctypefind == false) {
        if (rulestring_one.Contains ("Class:")) {
            synctypefind = true;
            rulestring_one.ReplaceAll ("Class:", "");
            paramNamePrefix = "{@class:";
            param.fromClassification = true;
        }
    }

    if (synctypefind == false) {
        if (rulestring_one.Contains ("Attribute:")) {
            synctypefind = true;
            rulestring_one.ReplaceAll ("Attribute:", "");
            paramNamePrefix = "{@attrib:";
            param.fromAttribElement = true;
            syncdirection = SYNC_TO;
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
    if (param.fromMaterial) {
        if (elementType != API_WallID &&
           elementType != API_SlabID &&
           elementType != API_ColumnID &&
           elementType != API_BeamID &&
           elementType != API_RoofID &&
           elementType != API_BeamSegmentID &&
           elementType != API_ColumnSegmentID &&
           elementType != API_ShellID) synctypefind = false;
    }
    if (param.fromMorph) {
        if (elementType != API_MorphID) synctypefind = false;
    }

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
        param.rawName = paramNamePrefix + paramName.ToLowerCase () + ";" + param.val.uniStringValue + "." + stringformat.stringformat + "}";
    } else {
        stringformat_raw = FormatStringFunc::GetFormatString (paramName);
        stringformat = FormatStringFunc::ParseFormatString (stringformat_raw);
    }
    UInt32 start_ignore = 0;
    if (param.fromClassification) {
        param.name = params.Get (0).ToLowerCase ();
        param.rawName = paramNamePrefix + param.name;
        if (nparam > 1) {
            param.val.uniStringValue = params.Get (1);
            param.rawName = param.rawName + ";" + param.val.uniStringValue.ToLowerCase ();
        }
        param.rawName = param.rawName + "}";
        start_ignore = 1 + nparam;
    }
    if (param.rawName.IsEmpty ()) param.rawName = paramNamePrefix + paramName.ToLowerCase () + "}";
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
                                rawName_row_start = "{@gdl:" + dim0 + "}";
                            }
                        }
                        if (ndim > 1) {
                            GS::UniString dim1 = dim.Get (1);
                            if (UniStringToDouble (dim1, p)) {
                                array_row_end = (int) p;
                            } else {
                                rawName_row_end = "{@gdl:" + dim1 + "}";
                            }
                        }
                    } else {
                        if (UniStringToDouble (sr1, p)) {
                            array_row_start = (int) p;
                            array_row_end = (int) p;
                        } else {
                            rawName_row_start = "{@gdl:" + sr1 + "}";
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
                                rawName_col_start = "{@gdl:" + dim0 + "}";
                            }
                        }
                        if (ndim > 1) {
                            GS::UniString dim1 = dim.Get (1);
                            if (UniStringToDouble (dim1, p)) {
                                array_column_end = (int) p;
                            } else {
                                rawName_col_end = "{@gdl:" + dim1 + "}";
                            }
                        }
                    } else {
                        if (UniStringToDouble (sr1, p)) {
                            array_column_start = (int) p;
                            array_column_end = (int) p;
                        } else {
                            rawName_col_start = "{@gdl:" + sr1 + "}";
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
            param.rawName = paramNamePrefix + paramName.ToLowerCase () + GS::UniString::Printf ("@arr_%d_%d_%d_%d_%d", array_row_start, array_row_end, array_column_start, array_column_end, param.val.array_format_out) + rawName_row_start + "_" + rawName_row_end + "_" + rawName_col_start + "_" + rawName_col_end + "}";
            start_ignore = 2;
        }

        // Обработка игнорируемых значений
        if (nparam > start_ignore) {
            for (UInt32 j = start_ignore; j < nparam; j++) {
                GS::UniString ignoreval;
                if (params[j].Contains ('"')) {
                    ignoreval = params[j].GetSubstring ('"', '"', 0);
                } else {
                    ignoreval = params[j];
                }
                ignorevals.Push (ignoreval);
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
    GS::UniString fmane = "SyncSetSubelement";
    GS::Array<API_Guid> subguidArray_ = GetSelectedElements (true, true, syncSettings, false); // Дочерние элементы, в которые будет записана информация об основном элементе
    if (subguidArray_.IsEmpty ()) return;

    API_Element parentelement; // Родительский элемент
    BNZeroMemory (&parentelement, sizeof (API_Element));
    API_ElemTypeID parentelementtype;
#if defined(AC_27) || defined(AC_28) || defined(AC_26)
    if (!ClickAnElem ("Click an parent elem", API_ZombieElemID, nullptr, &parentelement.header.type, &parentelement.header.guid)) {
        return;
    }
    parentelementtype = parentelement.header.type.typeID;
#else
    if (!ClickAnElem ("Click an parent elem", API_ZombieElemID, nullptr, &parentelement.header.typeID, &parentelement.header.guid)) {
        return;
    }
    parentelementtype = parentelement.header.typeID;
#endif
    // При работе на резрезах возможно попадание по виртуальному элементу, обработаем
    API_Elem_Head parentelementhead;
    if (parentelementtype == API_SectElemID) {
        API_Guid parentguid;
        API_ElemTypeID elementType;
        GetParentGUIDSectElem (parentelement.header.guid, parentguid, elementType);
        parentelementhead.guid = parentguid;
#if defined(AC_27) || defined(AC_28) || defined(AC_26)
        parentelementhead.type.typeID = elementType;
        parentelementtype = elementType;
#else
        parentelementhead.typeID = elementType;
        parentelementtype = elementType;
#endif
    } else {
        parentelementhead = parentelement.header;
#if defined(AC_27) || defined(AC_28) || defined(AC_26)
        parentelementtype = parentelement.header.type.typeID;
#else
        parentelementtype = parentelement.header.typeID;
#endif
    }
    if (!CheckElementType (parentelementtype, syncSettings)) {
        return;
    }
    // Проверяем доступность для редактирования
    GS::Array<API_Guid> subguidArray; // Массив редактируемых и почищенных от SectElem дочерних элементов
    GS::Array<API_Guid> labelArray; // Массив выносок
    for (UInt32 i = 0; i < subguidArray_.GetSize (); i++) {
        API_ElemTypeID elementType;
        if (IsElementEditable (subguidArray_[i], syncSettings, false, elementType)) {
            if (elementType == API_LabelID) {
                if (subguidArray_[i] != parentelementhead.guid) labelArray.Push (subguidArray_[i]);
            } else {
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
    if (subguidArray.IsEmpty () && labelArray.IsEmpty ()) {
        return;
    }
    ParamDictValue propertyParams;
    GSFlags flag = 0;
    ParamDictElement paramToWrite;
    SyncSetSubelementScope (parentelementhead, subguidArray, propertyParams, paramToWrite);
    if (!paramToWrite.IsEmpty ()) {
        err = ACAPI_CallUndoableCommand ("SetSubelement",
                [&]() -> GSErrCode {
            ParamHelpers::ElementsWrite (paramToWrite);
            return NoError;
        });
        SyncSelected (syncSettings);
    }

}

// -----------------------------------------------------------------------------
// Запись Guid связанных элементов
// Функция для вызова из ACAPI_CallUndoableCommand
// -----------------------------------------------------------------------------
void SyncSetSubelementScope (const API_Elem_Head& parentelementhead, GS::Array<API_Guid>& subguidArray, ParamDictValue& propertyParams, ParamDictElement& paramToWrite)
{
    GSErrCode err = NoError;
    if (propertyParams.IsEmpty ()) ParamHelpers::AllPropertyDefinitionToParamDict (propertyParams);
    ParamDictValue propertyParamselem;
    ParamHelpers::AllPropertyDefinitionToParamDict (propertyParamselem, parentelementhead.guid);
    ParamDictValue paramDict;
    for (auto& cItt : propertyParamselem) {
#if defined(AC_28)
        ParamValue param = cItt.value;
#else
        ParamValue param = *cItt.value;
#endif
        if (param.definition.description.Contains ("Sync_GUID")) {
            paramDict.Add (param.rawName, param);
        }
}
    if (paramDict.IsEmpty ()) return;
    ParamDictElement paramToRead;
    for (UInt32 i = 0; i < subguidArray.GetSize (); i++) {
        ParamHelpers::AddParamDictValue2ParamDictElement (subguidArray[i], paramDict, paramToRead);
    }
    ClassificationFunc::SystemDict systemdict;
    ParamHelpers::ElementsRead (paramToRead, propertyParams, systemdict);
    GS::UniString parentguidtxt = APIGuidToString (parentelementhead.guid);
    // Записываем в дочерние элементы GUID родительского
    for (UInt32 i = 0; i < subguidArray.GetSize (); i++) {
        if (paramToRead.ContainsKey (subguidArray[i])) {
            ParamDictValue subparams = paramToRead.Get (subguidArray[i]);
            bool flag_write = false;
            for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = subparams.EnumeratePairs (); cIt != NULL; ++cIt) {
#if defined(AC_28)
                ParamValue& param = cIt->value;
#else
                ParamValue& param = *cIt->value;
#endif
                //TODO Дописать фильтр по типу, слою, свойству
                if (!param.val.uniStringValue.IsEmpty ()) {
                    API_Elem_Head elementHead;
                    elementHead.guid = APIGuidFromString (param.val.uniStringValue.ToCStr (0, MaxUSize, GChCode));
                    err = ACAPI_Element_GetHeader (&elementHead);
                    if (err != NoError) {
                        ParamValue paramtow = subparams.Get (param.rawName);
                        paramtow.val.uniStringValue = parentguidtxt;
                        ParamHelpers::AddParamValue2ParamDictElement (subguidArray[i], paramtow, paramToWrite);
                        flag_write = true;
                    }
                } else {
                    ParamValue paramtow = subparams.Get (param.rawName);
                    paramtow.val.uniStringValue = parentguidtxt;
                    ParamHelpers::AddParamValue2ParamDictElement (subguidArray[i], paramtow, paramToWrite);
                    flag_write = true;
                }
                if (flag_write) break;
        }
    }
    }
}

// -----------------------------------------------------------------------------
// Обновление данных в выноске c записью в выноску
// -----------------------------------------------------------------------------
void SyncLabel (const API_Guid& guid, ParamDictValue& propertyParams)
{
    GSErrCode err = NoError;
    ParamDictElement paramToWrite;
    SyncLabelScope (guid, propertyParams, paramToWrite);
    if (!paramToWrite.IsEmpty ()) {
        err = ACAPI_CallUndoableCommand ("SyncLabel",
                [&]() -> GSErrCode {
            ParamHelpers::ElementsWrite (paramToWrite);
            return NoError;
        });
    }
}

// -----------------------------------------------------------------------------
// Обновление данных в выноске
// -----------------------------------------------------------------------------
void SyncLabelScope (const API_Guid& guid, ParamDictValue& propertyParams, ParamDictElement& paramToWrite)
{
    GSErrCode err = NoError;
    UInt32 max_guid = 3;
    ParamDictValue params;
    ParamHelpers::AddValueToParamDictValue (params, "@gdl:somestuff_property_name");
    ParamHelpers::AddValueToParamDictValue (params, "@gdl:somestuff_property_guid");
    ParamHelpers::AddValueToParamDictValue (params, "@gdl:somestuff_read");
    for (UInt32 i = 0; i < max_guid; i++) {
        GS::UniString name = GS::UniString::Printf ("@gdl:somestuff_subguid_%d", i + 1);
        ParamHelpers::AddValueToParamDictValue (params, name);
        name = GS::UniString::Printf ("@gdl:somestuff_subtext_%d", i + 1);
        ParamHelpers::AddValueToParamDictValue (params, name);
        name = GS::UniString::Printf ("@gdl:somestuff_infostr_%d", i + 1);
        ParamHelpers::AddValueToParamDictValue (params, name);
    }
    ClassificationFunc::SystemDict systemdict;
    ParamHelpers::Read (guid, params, propertyParams, systemdict);
    GS::Array<API_Guid> somestuff_subguids;
    for (UInt32 i = 0; i < max_guid; i++) {
        GS::UniString name_guid = GS::UniString::Printf ("{@gdl:somestuff_subguid_%d}", i + 1);
        if (params.Get (name_guid).isValid && !params.Get (name_guid).val.uniStringValue.IsEmpty ()) {
            API_Guid elemGuidfrom = APIGuidFromString (params.Get (name_guid).val.uniStringValue.ToCStr (0, MaxUSize, GChCode));
            if (elemGuidfrom != APINULLGuid) somestuff_subguids.Push (elemGuidfrom);
        }
    }
    if (somestuff_subguids.IsEmpty ()) return;
    if (!params.Get ("{@gdl:somestuff_read}").val.boolValue) return;
    ParamValue param;
    GS::UniString somestuff_property_name = "";
    if (!params.Get ("{@gdl:somestuff_property_name}").val.uniStringValue.IsEmpty ()) {
        somestuff_property_name = "{@property:" + params.Get ("{@gdl:somestuff_property_name}").val.uniStringValue.ToLowerCase () + "}";
        if (propertyParams.IsEmpty ()) ParamHelpers::AllPropertyDefinitionToParamDict (propertyParams);
        if (!propertyParams.ContainsKey (somestuff_property_name)) {
#if defined(TESTING)
            DBprnt ("SyncLabelScope", "!propertyParams.ContainsKey");
#endif
        } else {
            param = propertyParams.Get (somestuff_property_name);
        }
    }
    if (param.definition.guid == APINULLGuid) {
        if (!params.Get ("{@gdl:somestuff_property_guid}").isValid) return;
        if (params.Get ("{@gdl:somestuff_property_guid}").val.uniStringValue.IsEmpty ()) return;
        API_PropertyDefinition definition;
        definition.guid = APIGuidFromString (params.Get ("{@gdl:somestuff_property_guid}").val.uniStringValue.ToCStr (0, MaxUSize, GChCode));
        err = ACAPI_Property_GetPropertyDefinition (definition);
        if (err != NoError) return;
        if (!ParamHelpers::ConvertToParamValue (param, definition)) return;
        somestuff_property_name = param.rawName;
    }
    if (param.definition.guid == APINULLGuid) return;
    ParamDictElement paramToRead;
    ParamValue pvaluelayer;
    pvaluelayer.name = "composite name";
    pvaluelayer.rawName = "{@property:generalelemproperties/composite name}";
    pvaluelayer.fromProperty = true;
    for (UInt32 i = 0; i < somestuff_subguids.GetSize (); i++) {
        ParamHelpers::AddParamValue2ParamDictElement (somestuff_subguids[i], param, paramToRead);
        ParamHelpers::AddParamValue2ParamDictElement (somestuff_subguids[i], pvaluelayer, paramToRead);
    }
    ParamHelpers::ElementsRead (paramToRead, propertyParams, systemdict);
    for (UInt32 i = 0; i < max_guid; i++) {
        if (paramToRead.ContainsKey (somestuff_subguids[i])) {
            GS::UniString name = GS::UniString::Printf ("{@gdl:somestuff_subtext_%d}", i + 1);
            ParamValue paramFrom = paramToRead.Get (somestuff_subguids[i]).Get (somestuff_property_name);
            ParamValue paramTo = params.Get (name);
            FormatString formatstring = paramFrom.val.formatstring;
            if (ParamHelpers::CompareParamValue (paramFrom, paramTo, formatstring)) {
                paramTo.fromGuid = guid;
                ParamHelpers::AddParamValue2ParamDictElement (paramTo, paramToWrite);
            }
            name = GS::UniString::Printf ("{@gdl:somestuff_infostr_%d}", i + 1);
            paramFrom.val.uniStringValue = paramToRead.Get (somestuff_subguids[i]).Get (pvaluelayer.rawName).val.uniStringValue;
            paramTo = params.Get (name);
            formatstring = paramFrom.val.formatstring;
            if (ParamHelpers::CompareParamValue (paramFrom, paramTo, formatstring)) {
                paramTo.fromGuid = guid;
                ParamHelpers::AddParamValue2ParamDictElement (paramTo, paramToWrite);
            }
        }
    }
}

// --------------------------------------------------------------------
// Подсвечивает элементы, GUID которых указан в свойстве с описанием Sync_GUID
// --------------------------------------------------------------------
void SyncShowSubelement (const SyncSettings& syncSettings)
{
#ifndef AC_22
    GS::Array<API_Guid> guidArray_all = GetSelectedElements (true, false, syncSettings, false);
    GS::Array<API_Guid> guidArray;
    GS::Array<API_Guid> guidArray_label;
    for (UInt32 i = 0; i < guidArray_all.GetSize (); i++) {
        API_ElemTypeID elementType;
        if (GetTypeByGUID (guidArray_all[i], elementType) == NoError) {
            if (elementType == API_LabelID) {
                guidArray_label.Push (guidArray_all[i]);
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
    GS::Array<API_Neig> selNeigs;
    if (!guidArray_label.IsEmpty ()) {
        UInt32 max_guid = 3;
        ParamDictValue params;
        ParamDictElement paramToRead;
        ParamDictValue propertyParams;
        ClassificationFunc::SystemDict systemdict;
        ParamHelpers::AddValueToParamDictValue (params, "@gdl:somestuff_read");
        for (UInt32 i = 0; i < max_guid; i++) {
            GS::UniString name = GS::UniString::Printf ("@gdl:somestuff_subguid_%d", i + 1);
            ParamHelpers::AddValueToParamDictValue (params, name);
        }
        for (UInt32 i = 0; i < guidArray_label.GetSize (); i++) {
            ParamHelpers::AddParamDictValue2ParamDictElement (guidArray_label[i], params, paramToRead);
        }
        ParamHelpers::ElementsRead (paramToRead, propertyParams, systemdict);
        for (UInt32 j = 0; j < guidArray_label.GetSize (); j++) {
            if (paramToRead.ContainsKey (guidArray_label[j])) {
                if (paramToRead.Get (guidArray_label[j]).Get ("{@gdl:somestuff_read}").val.boolValue) {
                    for (UInt32 i = 0; i < max_guid; i++) {
                        GS::UniString name_guid = GS::UniString::Printf ("{@gdl:somestuff_subguid_%d}", i + 1);
                        if (paramToRead.Get (guidArray_label[j]).Get (name_guid).isValid && !paramToRead.Get (guidArray_label[j]).Get (name_guid).val.uniStringValue.IsEmpty ()) {
                            API_Guid elemGuidfrom = APIGuidFromString (paramToRead.Get (guidArray_label[j]).Get (name_guid).val.uniStringValue.ToCStr (0, MaxUSize, GChCode));
                            if (elemGuidfrom != APINULLGuid) selNeigs.PushNew (elemGuidfrom);
                        }
                    }
                }
            }
        }
        if (selNeigs.IsEmpty ()) {
            API_Element element;
            for (UInt32 j = 0; j < guidArray_label.GetSize (); j++) {
                BNZeroMemory (&element, sizeof (API_Element));
                element.header.guid = guidArray_label[j];
                if (ACAPI_Element_Get (&element) == NoError) {
                    if (element.label.parent != APINULLGuid) {
                        if (element.label.parentType == API_SectElemID) {
                            API_Guid parentguid;
                            API_ElemTypeID elementType;
                            GetParentGUIDSectElem (element.label.parent, parentguid, elementType);
                            guidArray.Push (parentguid);
                        } else {
                            guidArray.Push (element.label.parent);
                        }
                    }
                }
            }
        }
    }
    if (!guidArray.IsEmpty ()) {
        ParamDictValue propertyParams;
        ParamHelpers::AllPropertyDefinitionToParamDict (propertyParams);
        ParamDictValue paramDict;
        for (auto& cItt : propertyParams) {
#if defined(AC_28)
            ParamValue param = cItt.value;
#else
            ParamValue param = *cItt.value;
#endif
            if (param.definition.description.Contains ("Sync_GUID")) {
                paramDict.Add (param.rawName, param);
            }
    }
        if (paramDict.IsEmpty ()) return;
        ParamDictElement paramToRead;
        for (UInt32 i = 0; i < guidArray.GetSize (); i++) {
            ParamHelpers::AddParamDictValue2ParamDictElement (guidArray[i], paramDict, paramToRead);
        }
        ClassificationFunc::SystemDict systemdict;
        ParamHelpers::ElementsRead (paramToRead, propertyParams, systemdict);
        for (auto& cIt : paramToRead) {
#if defined(AC_28)
            ParamDictValue params = cIt.value;
#else
            ParamDictValue params = *cIt.value;
#endif
            for (auto& cItt : params) {
#if defined(AC_28)
                ParamValue param = cItt.value;
#else
                ParamValue param = *cItt.value;
#endif
                if (param.isValid && !param.val.uniStringValue.IsEmpty ()) {
                    GS::Array<GS::UniString> rulestring_param;
                    UInt32 nrule_param = StringSplt (param.val.uniStringValue, ";", rulestring_param);
                    if (nrule_param > 0) {
                        for (UInt32 i = 0; i < nrule_param; i++) {
                            API_Guid guid = APIGuidFromString (rulestring_param[i].ToCStr (0, MaxUSize, GChCode));
                            selNeigs.PushNew (guid);
                        }
                    }
                }
}
        }
    }
    if (selNeigs.IsEmpty ()) return;
#if defined(AC_27) || defined(AC_28)
    GSErrCode err = ACAPI_Selection_Select (selNeigs, true);
#else
    //TODO Проверить - почему выделение субэлементов не работает на окнах и дверях
    GSErrCode err = ACAPI_Element_Select (selNeigs, true);
#endif
#endif
    return;
}

