//------------ kuvbur 2022 ------------
#include	<stdlib.h> /* atoi */
#include	<time.h>
#include	"APIEnvir.h"
#include	"ACAPinc.h"
#include	"Sync.hpp"
#include	"ResetProperty.hpp"
#include	"Dimensions.hpp"

#define SYNC_NO 0
#define SYNC_FROM 1
#define SYNC_TO 2
#define SYNC_TO_SUB 3
#define SYNC_FROM_SUB 4

Int32 nLib = 0;

// -----------------------------------------------------------------------------
// Подключение мониторинга
// -----------------------------------------------------------------------------
void MonAll(SyncSettings& syncSettings) {
	if (!syncSettings.syncMon) return;
	DBPrintf("== SMSTF == MonAll start\n");
	long time_start = clock();
	MonByType(API_ObjectID, syncSettings);
	MonByType(API_WindowID, syncSettings);
	MonByType(API_DoorID, syncSettings);
	MonByType(API_ZoneID, syncSettings);
	MonByType(API_WallID, syncSettings);
	MonByType(API_SlabID, syncSettings);
	MonByType(API_ColumnID, syncSettings);
	MonByType(API_BeamID, syncSettings);
	MonByType(API_RoofID, syncSettings);
	MonByType(API_MeshID, syncSettings);
	MonByType(API_MorphID, syncSettings);
	MonByType(API_CurtainWallID, syncSettings);
	long time_end = clock();
	GS::UniString time = GS::UniString::Printf(" %d s", (time_end - time_start) / 1000);
	msg_rep("MonAll", time, NoError, APINULLGuid);
	DBPrintf("== SMSTF == MonAll end\n");
}

// -----------------------------------------------------------------------------
// Подключение мониторинга по типам
// -----------------------------------------------------------------------------
void MonByType(const API_ElemTypeID& elementType, const SyncSettings& syncSettings) {
	GS::Array<API_Guid>	guidArray;
	DBPrintf("== SMSTF == MonByType\n");
	GSErrCode err = ACAPI_Element_GetElemList(elementType, &guidArray, APIFilt_IsEditable | APIFilt_HasAccessRight | APIFilt_InMyWorkspace);
	if (err != NoError || guidArray.IsEmpty()) return;
	for (UInt32 i = 0; i < guidArray.GetSize(); i++) {
		err = AttachObserver(guidArray[i], syncSettings);
		if (err == APIERR_LINKEXIST)
			err = NoError;
		if (err != NoError) {
			msg_rep("MonByType", "AttachObserver", err, guidArray[i]);
			return;
		}
	}
}

// -----------------------------------------------------------------------------
// Запускает обработку всех объектов, заданных в настройке
// -----------------------------------------------------------------------------
void SyncAndMonAll(SyncSettings& syncSettings) {
	DBPrintf("== SMSTF == SyncAndMonAll start\n");

	// Сразу прочитаем свойства и разложим их по элементам
	ParamDictValue propertyParams;
	ParamHelpers::GetAllPropertyDefinitionToParamDict(propertyParams);
	ParamHelpers::GetAllInfoToParamDict(propertyParams);
	ParamHelpers::GetAllGlobToParamDict(propertyParams);
	if (propertyParams.IsEmpty()) return;
	if (ResetProperty(propertyParams)) return;
	GS::UniString	title("Sync All");
	bool flag_chanel = false;
	ParamDictElement paramToWrite;
	short nPhase = 1;
	ACAPI_Interface(APIIo_InitProcessWindowID, &title, &nPhase);
	long time_start = clock();
	if (!flag_chanel && syncSettings.objS) flag_chanel = SyncByType(API_ObjectID, syncSettings, nPhase, propertyParams, paramToWrite);
	nPhase = nPhase + 1;
	if (!flag_chanel && syncSettings.widoS) flag_chanel = SyncByType(API_WindowID, syncSettings, nPhase, propertyParams, paramToWrite);
	nPhase = nPhase + 1;
	if (!flag_chanel && syncSettings.widoS) flag_chanel = SyncByType(API_DoorID, syncSettings, nPhase, propertyParams, paramToWrite);
	nPhase = nPhase + 1;
	if (!flag_chanel && syncSettings.objS) flag_chanel = SyncByType(API_ZoneID, syncSettings, nPhase, propertyParams, paramToWrite);
	nPhase = nPhase + 1;
	if (!flag_chanel && syncSettings.wallS) flag_chanel = SyncByType(API_WallID, syncSettings, nPhase, propertyParams, paramToWrite);
	nPhase = nPhase + 1;
	if (!flag_chanel && syncSettings.wallS) flag_chanel = SyncByType(API_SlabID, syncSettings, nPhase, propertyParams, paramToWrite);
	nPhase = nPhase + 1;
	if (!flag_chanel && syncSettings.wallS) flag_chanel = SyncByType(API_ColumnID, syncSettings, nPhase, propertyParams, paramToWrite);
	nPhase = nPhase + 1;
	if (!flag_chanel && syncSettings.wallS) flag_chanel = SyncByType(API_BeamID, syncSettings, nPhase, propertyParams, paramToWrite);
	nPhase = nPhase + 1;
	if (!flag_chanel && syncSettings.wallS) flag_chanel = SyncByType(API_RoofID, syncSettings, nPhase, propertyParams, paramToWrite);
	nPhase = nPhase + 1;
	if (!flag_chanel && syncSettings.wallS) flag_chanel = SyncByType(API_MeshID, syncSettings, nPhase, propertyParams, paramToWrite);
	nPhase = nPhase + 1;
	if (!flag_chanel && syncSettings.wallS) flag_chanel = SyncByType(API_MorphID, syncSettings, nPhase, propertyParams, paramToWrite);
	nPhase = nPhase + 1;
	if (!flag_chanel && syncSettings.cwallS) flag_chanel = SyncByType(API_CurtainWallID, syncSettings, nPhase, propertyParams, paramToWrite);
	long time_end = clock();
	GS::UniString time = GS::UniString::Printf(" %d s", (time_end - time_start) / 1000);
	msg_rep("SyncAll - read", time, NoError, APINULLGuid);
	if (!paramToWrite.IsEmpty()) {
		GS::UniString undoString = RSGetIndString(AddOnStringsID, UndoSyncId, ACAPI_GetOwnResModule());
		ACAPI_CallUndoableCommand(undoString, [&]() -> GSErrCode {
			long time_start = clock();
			GS::UniString title = GS::UniString::Printf("Writing data to %d elements : ", paramToWrite.GetSize()); short i = 1;
			ACAPI_Interface(APIIo_SetNextProcessPhaseID, &title, &i);
			ParamHelpers::ElementsWrite(paramToWrite);
			long time_end = clock();
			GS::UniString time = title + GS::UniString::Printf(" %d s", (time_end - time_start) / 1000);
			msg_rep("SyncAll - write", time, NoError, APINULLGuid);
			return NoError;
			});
	}
	else {
		msg_rep("SyncAll - write", "No data to write", NoError, APINULLGuid);
	}
	ParamHelpers::InfoWrite(paramToWrite);
	ACAPI_Interface(APIIo_CloseProcessWindowID, nullptr, nullptr);
	DBPrintf("== SMSTF == SyncAndMonAll end\n");
}

// -----------------------------------------------------------------------------
// Синхронизация элементов по типу
// -----------------------------------------------------------------------------
bool SyncByType(const API_ElemTypeID& elementType, const SyncSettings& syncSettings, short& nPhase, ParamDictValue& propertyParams, ParamDictElement& paramToWrite) {
	GS::UniString		subtitle;
	GSErrCode			err = NoError;
	GS::Array<API_Guid>	guidArray;
	long time_start = clock();
	ACAPI_Element_GetElemList(elementType, &guidArray, APIFilt_IsEditable | APIFilt_HasAccessRight | APIFilt_InMyWorkspace);
	if (guidArray.IsEmpty()) return false;
#ifdef AC_26
	API_ElemType elemType;
	elemType.typeID = elementType;
	ACAPI_Goodies_GetElemTypeName(elemType, subtitle);
#else
	ACAPI_Goodies(APIAny_GetElemTypeNameID, (void*)elementType, &subtitle);
#endif // AC_26
	GS::UniString subtitle_ = GS::UniString::Printf("Reading data from %d elements : ", guidArray.GetSize()) + subtitle;

	// Словарь со всеми возможными определениями свойств
	if (propertyParams.IsEmpty()) {
		ParamHelpers::GetAllPropertyDefinitionToParamDict(propertyParams);
		ParamHelpers::GetAllInfoToParamDict(propertyParams);
		ParamHelpers::GetAllGlobToParamDict(propertyParams);
	}
	if (propertyParams.IsEmpty()) return true;
	bool flag_chanel = false;
	API_EditCmdID acttype = APIEdit_General;
	for (UInt32 i = 0; i < guidArray.GetSize(); i++) {
		SyncElement(guidArray[i], syncSettings, propertyParams, paramToWrite, acttype);
		if (i % 10 == 0) ACAPI_Interface(APIIo_SetNextProcessPhaseID, &subtitle_, &i);
	}
	GS::UniString intString = GS::UniString::Printf(" %d qty", guidArray.GetSize());
	long time_end = clock();
	GS::UniString time = GS::UniString::Printf(" %d s", (time_end - time_start) / 1000);
	msg_rep("SyncByType", subtitle + intString + time, NoError, APINULLGuid);
	return flag_chanel;
}

// -----------------------------------------------------------------------------
// Синхронизация элемента и его подэлементов
// -----------------------------------------------------------------------------
void SyncElement(const API_Guid& elemGuid, const SyncSettings& syncSettings, ParamDictValue& propertyParams, ParamDictElement& paramToWrite, API_EditCmdID& acttype) {
	API_ElemTypeID elementType;
	GSErrCode err = GetTypeByGUID(elemGuid, elementType);
	if (err != NoError) return;

	// Получаем список связанных элементов
	GS::Array<API_Guid> subelemGuids;
	GetRelationsElement(elemGuid, elementType, syncSettings, subelemGuids);
	SyncData(elemGuid, syncSettings, subelemGuids, propertyParams, paramToWrite, acttype);
	if (!subelemGuids.IsEmpty() && SyncRelationsElement(elementType, syncSettings)) {
		for (UInt32 i = 0; i < subelemGuids.GetSize(); ++i) {
			API_Guid subelemGuid = subelemGuids[i];
			if (subelemGuid != elemGuid) {
				GS::Array<API_Guid> epm;
				SyncData(subelemGuid, syncSettings, epm, propertyParams, paramToWrite, acttype);
			}
		}
	}
}

// -----------------------------------------------------------------------------
// Запускает обработку выбранных, заданных в настройке
// -----------------------------------------------------------------------------
void SyncSelected(const SyncSettings& syncSettings) {
	GS::UniString fmane = "Sync Selected";
	GS::Array<API_Guid> guidArray = GetSelectedElements(false, true, false);
	if (guidArray.IsEmpty()) return;
	ParamDictValue propertyParams = {};
	ParamHelpers::GetAllPropertyDefinitionToParamDict(propertyParams);
	ParamHelpers::GetAllInfoToParamDict(propertyParams);
	ParamHelpers::GetAllGlobToParamDict(propertyParams);
	ParamDictElement paramToWrite;
	GS::UniString subtitle = GS::UniString::Printf("Reading data from %d elements", guidArray.GetSize());
	short nPhase = 1;
	ACAPI_Interface(APIIo_InitProcessWindowID, &fmane, &nPhase);
	long time_start = clock();
	API_EditCmdID acttype = APIEdit_General;
	for (UInt32 i = 0; i < guidArray.GetSize(); i++) {
		SyncElement(guidArray[i], syncSettings, propertyParams, paramToWrite, acttype);
		if (i % 10 == 0) ACAPI_Interface(APIIo_SetNextProcessPhaseID, &subtitle, &i);
		if (ACAPI_Interface(APIIo_IsProcessCanceledID, nullptr, nullptr)) {
			return;
		}
	}
	GS::UniString intString = GS::UniString::Printf(" %d qty", guidArray.GetSize());
	long time_end = clock();
	GS::UniString time = GS::UniString::Printf(" %d s", (time_end - time_start) / 1000);
	msg_rep("SyncSelected - read", subtitle + intString + time, NoError, APINULLGuid);
	if (!paramToWrite.IsEmpty()) {
		GS::UniString undoString = RSGetIndString(AddOnStringsID, UndoSyncId, ACAPI_GetOwnResModule());
		ACAPI_CallUndoableCommand(undoString, [&]() -> GSErrCode {
			long time_start = clock();
			GS::UniString title = GS::UniString::Printf("Writing data to %d elements : ", paramToWrite.GetSize()); short i = 1;
			ACAPI_Interface(APIIo_SetNextProcessPhaseID, &title, &i);
			ParamHelpers::ElementsWrite(paramToWrite);
			long time_end = clock();
			GS::UniString time = title + GS::UniString::Printf(" %d s", (time_end - time_start) / 1000);
			msg_rep("SyncSelected - write", time, NoError, APINULLGuid);
			return NoError;
			});
	}
	else {
		msg_rep("SyncSelected - write", "No data to write", NoError, APINULLGuid);
	}
	ParamHelpers::InfoWrite(paramToWrite);
	ACAPI_Interface(APIIo_CloseProcessWindowID, nullptr, nullptr);
}

// -----------------------------------------------------------------------------
// Запуск скрипта параметров выбранных элементов
// -----------------------------------------------------------------------------
void RunParamSelected(const SyncSettings& syncSettings) {
	GS::UniString fmane = "Run parameter script";

	// Запомним номер текущей БД и комбинацию слоёв для восстановления по окончанию работы
	API_AttributeIndex layerCombIndex;
	API_DatabaseInfo databaseInfo;
	BNZeroMemory(&databaseInfo, sizeof(API_DatabaseInfo));
	GSErrCode err = ACAPI_Environment(APIEnv_GetCurrLayerCombID, &layerCombIndex);
	err = ACAPI_Database(APIDb_GetCurrentDatabaseID, &databaseInfo, nullptr);
	if (err != NoError) return;
	CallOnSelectedElemSettings(RunParam, false, true, syncSettings, fmane, false);
	if (layerCombIndex != 0) err = ACAPI_Environment(APIEnv_ChangeCurrLayerCombID, &layerCombIndex);
	err = ACAPI_Database(APIDb_ChangeCurrentDatabaseID, &databaseInfo, nullptr);
}

// -----------------------------------------------------------------------------
// Запуск скрипта параметра элемента
// -----------------------------------------------------------------------------
void RunParam(const API_Guid& elemGuid, const SyncSettings& syncSettings) {
	DBPrintf("== SMSTF == RunParam\n");
	API_Elem_Head	tElemHead;
	BNZeroMemory(&tElemHead, sizeof(API_Elem_Head));
	tElemHead.guid = elemGuid;
	GSErrCode	err = ACAPI_Element_GetHeader(&tElemHead);
	if (err != NoError) return;
	API_DatabaseInfo databaseInfo;
	API_DatabaseInfo dbInfo;
	err = ACAPI_Database(APIDb_GetContainingDatabaseID, &tElemHead.guid, &dbInfo);
	if (err != NoError) return;
	err = ACAPI_Database(APIDb_GetCurrentDatabaseID, &databaseInfo, nullptr);
	if (err != NoError) return;
	if (dbInfo.databaseUnId != databaseInfo.databaseUnId) {
		err = ACAPI_Database(APIDb_ChangeCurrentDatabaseID, &dbInfo, nullptr);
		if (err != NoError) return;
	}

	API_Element element, mask;
	ACAPI_ELEMENT_MASK_CLEAR(mask);
	ACAPI_ELEMENT_MASK_SET(mask, API_Elem_Head, renovationStatus);
	element.header = tElemHead;
	err = ACAPI_Element_Get(&element);
	if (err != NoError) {
		msg_rep("RunParam", "APIAny_RunGDLParScriptID", err, elemGuid);
		return;
	}
	err = ACAPI_Goodies(APIAny_RunGDLParScriptID, &tElemHead, 0);
	if (err != NoError) {
		msg_rep("RunParam", "APIAny_RunGDLParScriptID", err, elemGuid);
		return;
	}
}

// --------------------------------------------------------------------
// Поиск и синхронизация свойств связанных элементов
// --------------------------------------------------------------------
bool SyncRelationsElement(const API_ElemTypeID& elementType, const SyncSettings& syncSettings) {
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
void SyncData(const API_Guid& elemGuid, const SyncSettings& syncSettings, GS::Array<API_Guid>& subelemGuids, ParamDictValue& propertyParams, ParamDictElement& paramToWrite, API_EditCmdID& acttype) {
	GSErrCode	err = NoError;
	if (!IsElementEditable(elemGuid, syncSettings, true)) return;

	// Если включён мониторинг - привязываем элемент к отслеживанию
	if (syncSettings.syncMon) {
		err = AttachObserver(elemGuid, syncSettings);
		if (err == APIERR_LINKEXIST)
			err = NoError;
		if (err != NoError) {
			msg_rep("SyncData", "AttachObserver", err, elemGuid);
			return;
		}
	}
	GS::Array<API_PropertyDefinition> definitions;
	err = ACAPI_Element_GetPropertyDefinitions(elemGuid, API_PropertyDefinitionFilter_UserDefined, definitions);
	if (err != NoError) {
		msg_rep("SyncData", "ACAPI_Element_GetPropertyDefinitions", err, elemGuid);
		return;
	}

	// Синхронизация данных
	// Проверяем - не отключена ли синхронизация у данного объекта
	if (!GetElemState(elemGuid, definitions, "Sync_flag")) return;
	API_ElemTypeID elementType;
	err = GetTypeByGUID(elemGuid, elementType);
	if (err != NoError) { return; }
	GS::Array <WriteData> mainsyncRules;
	ParamDictElement paramToRead; // Словарь с параметрами для чтения
	bool hasSub = false;
	for (UInt32 i = 0; i < definitions.GetSize(); i++) {

		// Получаем список правил синхронизации из всех свойств
		ParseSyncString(elemGuid, elementType, definitions[i], mainsyncRules, paramToRead, hasSub, acttype); // Парсим описание свойства
	}
	if (mainsyncRules.IsEmpty()) return;
	if (propertyParams.IsEmpty()) {
		if (hasSub) {
			ParamHelpers::GetAllPropertyDefinitionToParamDict(propertyParams);
		}
		else {
			ParamHelpers::GetAllPropertyDefinitionToParamDict(propertyParams, definitions);
		}
	}
	if (propertyParams.IsEmpty()) return;

	// Заполняем правила синхронизации с учётом субэлементов, попутно заполняем словарь параметров для чтения/записи
	WriteDict syncRules; // Словарь с правилами для каждого элемента
	SyncAddSubelement(subelemGuids, mainsyncRules, syncRules, paramToRead);
	mainsyncRules.Clear();
	subelemGuids.Push(elemGuid); // Это теперь список всех элементов для синхронизации

	// Читаем все возможные свойства
	ParamHelpers::ElementsRead(paramToRead, propertyParams);

	// Выбираем по-элементно параметры для чтения и записи, формируем словарь
	for (UInt32 i = 0; i < subelemGuids.GetSize(); i++) {
		API_Guid elemGuid = subelemGuids[i];
		GS::Array <WriteData> writeSubs;
		if (syncRules.ContainsKey(elemGuid)) writeSubs = syncRules.Get(elemGuid);
		if (!writeSubs.IsEmpty()) {

			// Заполняем значения параметров чтения/записи из словаря
			for (UInt32 j = 0; j < writeSubs.GetSize(); j++) {
				WriteData writeSub = writeSubs.Get(j);
				API_Guid elemGuidTo = writeSub.guidTo;
				API_Guid elemGuidFrom = writeSub.guidFrom;

				// Проверяем - есть ли вообще эти элементы в словаре параметров
				if (paramToRead.ContainsKey(elemGuidTo) && paramToRead.ContainsKey(elemGuidFrom)) {
					GS::UniString rawNameFrom = writeSub.paramFrom.rawName;
					GS::UniString rawNameTo = writeSub.paramTo.rawName;
					ParamDictValue paramsTo = paramToRead.Get(elemGuidTo);
					ParamDictValue paramsFrom;
					if (elemGuidFrom == elemGuidTo) {
						paramsFrom = paramsTo;
					}
					else {
						paramsFrom = paramToRead.Get(elemGuidFrom);
					}

					// Проверяем наличие имён в словаре параметров
					if (paramsTo.ContainsKey(rawNameTo) && paramsFrom.ContainsKey(rawNameFrom)) {
						ParamValue paramFrom = paramsFrom.Get(rawNameFrom);
						if (paramFrom.isValid) {
							ParamValue paramTo = paramsTo.Get(rawNameTo);
							if (paramTo.isValid || paramTo.fromProperty || paramTo.fromPropertyDefinition) {
								GS::UniString stringformat = writeSub.stringformat;
								if (stringformat.IsEmpty()) stringformat = paramTo.val.stringformat;
								if (stringformat.IsEmpty()) stringformat = paramFrom.val.stringformat;

								// Приводим к единому виду перед проверкой
								if (!stringformat.IsEmpty()) {
									paramTo.val.stringformat = stringformat;
									paramFrom.val.stringformat = stringformat;
									ParamHelpers::ConvertByFormat(paramTo);
									ParamHelpers::ConvertByFormat(paramFrom);
								}

								//Сопоставляем и записываем, если значения отличаются
								if (paramFrom != paramTo) {
									paramTo.val = paramFrom.val; // Записываем только значения
									paramTo.isValid = true;
									ParamHelpers::AddParamValue2ParamDictElement(paramTo, paramToWrite);
								}
#ifdef DEBUG
								else {
									DBPrintf("== SMSTF == Write Debug\n");
									paramTo.val = paramFrom.val; // Записываем только значения
									paramTo.isValid = true;
									ParamHelpers::AddParamValue2ParamDictElement(paramTo, paramToWrite);
								}
#endif // DEBUG
							}
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
void SyncAddSubelement(const GS::Array<API_Guid>& subelemGuids, const GS::Array <WriteData>& mainsyncRules, WriteDict& syncRules, ParamDictElement& paramToRead) {
	for (UInt32 i = 0; i < mainsyncRules.GetSize(); i++) {
		if (!mainsyncRules[i].fromSub && !mainsyncRules[i].toSub) {
			WriteData writeSub = mainsyncRules.Get(i);
			SyncAddRule(writeSub, syncRules, paramToRead);
		}

		// Если есть субэлементы - обработаем их
		if (!subelemGuids.IsEmpty()) {

			// Для записи из дочернего в родительский возьмём только один, первый элемент
			if (mainsyncRules[i].fromSub) {
				WriteData writeSub = mainsyncRules.Get(i);
				API_Guid subelemGuid = subelemGuids.Get(0);
				writeSub.fromSub = false;
				writeSub.guidFrom = subelemGuid;
				writeSub.paramFrom.fromGuid = subelemGuid;
				SyncAddRule(writeSub, syncRules, paramToRead);
			}
			if (mainsyncRules[i].toSub) {
				for (UInt32 j = 0; j < subelemGuids.GetSize(); j++) {
					WriteData writeSub = mainsyncRules.Get(i);
					API_Guid subelemGuid = subelemGuids.Get(j);
					writeSub.toSub = false;
					writeSub.guidTo = subelemGuid;
					writeSub.paramTo.fromGuid = subelemGuid;
					SyncAddRule(writeSub, syncRules, paramToRead);
				}
			}
		}
	}
}

// --------------------------------------------------------------------
// Запись правила в словарь, попутно заполняем словарь с параметрами
// --------------------------------------------------------------------
void SyncAddRule(const WriteData& writeSub, WriteDict& syncRules, ParamDictElement& paramToRead) {
	API_Guid elemGuid = writeSub.guidTo;
	if (syncRules.ContainsKey(elemGuid)) {
		syncRules.Get(elemGuid).Push(writeSub);
	}
	else {
		GS::Array <WriteData> rules;
		rules.Push(writeSub);
		syncRules.Add(elemGuid, rules);
	}
	ParamHelpers::AddParamValue2ParamDictElement(writeSub.paramFrom, paramToRead);
	ParamHelpers::AddParamValue2ParamDictElement(writeSub.paramTo, paramToRead);
}

// -----------------------------------------------------------------------------
// Парсит описание свойства, заполняет массив с правилами (GS::Array <WriteData>)
// -----------------------------------------------------------------------------
bool ParseSyncString(const API_Guid& elemGuid, const  API_ElemTypeID& elementType, const API_PropertyDefinition& definition, GS::Array <WriteData>& syncRules, ParamDictElement& paramToRead, bool& hasSub, API_EditCmdID& acttype) {

	// TODO Попробовать отключать часть синхронизаций в зависимости от изменённых параметров (API_ActTranPars acttype)
	GS::UniString description_string = definition.description;
	if (description_string.IsEmpty()) {
		return false;
	}
	if (description_string.Contains("Sync_flag")) {
		return false;
	}

	// Если указан сброс данных - синхронизировать не будем
	if (description_string.Contains("Sync_reset")) {
		return false;
	}
	bool hasRule = false;
	if (description_string.Contains("Sync_") && description_string.Contains("{") && description_string.Contains("}")) {
		GS::Array<GS::UniString> rulestring;
		UInt32 nrule = StringSplt(description_string, "Sync_", rulestring, "{"); // Проверяем количество правил

		//Проходим по каждому правилу и извлекаем из него правило синхронизации (WriteDict syncRules) и словарь уникальных параметров для чтения/записи (ParamDictElement paramToRead)
		for (UInt32 i = 0; i < nrule; i++) {
			ParamValue param;
			int syncdirection = SYNC_NO; // Направление синхронизации
			GS::UniString rawparamName = ""; //Имя параметра/свойства с указанием типа синхронизации, для ключа словаря
			GS::Array<GS::UniString> ignorevals; //Игнорируемые значения
			GS::UniString stringformat = ""; //Строка с форматом числа
			if (SyncString(elementType, rulestring[i], syncdirection, param, ignorevals, stringformat)) {
				hasRule = true;
				ParamValue paramdef; //Свойство, из которого получено правило
				ParamHelpers::ConvValue(paramdef, definition);
				paramdef.fromGuid = elemGuid;
				WriteData writeOne;
				writeOne.stringformat = stringformat;
				writeOne.ignorevals = ignorevals;
				if (param.fromCoord && param.rawName.Contains("north_dir")) {
					ParamDictValue paramDict;
					ParamHelpers::AddVal(paramDict, "@info:glob_north_dir");
					ParamHelpers::AddParamDictValue2ParamDictElement(elemGuid, paramDict, paramToRead);
				}

				// Вытаскиваем параметры для материалов, если такие есть
				if (param.fromMaterial) {
					ParamDictValue paramDict;
					GS::UniString templatestring = param.val.uniStringValue; //Строка с форматом числа
					if (ParamHelpers::ParseParamNameMaterial(templatestring, paramDict)) {
						param.val.uniStringValue = templatestring;
						ParamHelpers::AddVal(paramDict, "@property:sync_name");
						ParamHelpers::AddParamDictValue2ParamDictElement(elemGuid, paramDict, paramToRead);
						hasSub = true; // Нужно будет прочитать все свойства
					}
				}
				if (syncdirection == SYNC_TO || syncdirection == SYNC_TO_SUB) {
					if (syncdirection == SYNC_TO_SUB) {
						hasSub = true;
						writeOne.toSub = true;
					}
					else {
						writeOne.guidTo = elemGuid;
						param.fromGuid = elemGuid;
					}
					writeOne.guidFrom = elemGuid;
					writeOne.paramFrom = paramdef;
					writeOne.paramTo = param;
				}
				if (syncdirection == SYNC_FROM || syncdirection == SYNC_FROM_SUB) {
					if (syncdirection == SYNC_FROM_SUB) {
						hasSub = true;
						writeOne.fromSub = true;
					}
					else {
						writeOne.guidFrom = elemGuid;
						param.fromGuid = elemGuid;
					}
					writeOne.guidTo = elemGuid;
					writeOne.paramTo = paramdef;
					writeOne.paramFrom = param;
				}
				syncRules.Push(writeOne);
			}
		}
	}
	return hasRule;
}

// -----------------------------------------------------------------------------
// Парсит описание свойства
// -----------------------------------------------------------------------------
bool SyncString(const  API_ElemTypeID& elementType, GS::UniString rulestring_one, int& syncdirection, ParamValue& param, GS::Array<GS::UniString>& ignorevals, GS::UniString& stringformat) {
	syncdirection = SYNC_NO;

	// Выбор направления синхронизации
	// Копировать в субэлементы или из субэлементов
	if (syncdirection == SYNC_NO && rulestring_one.Contains("to_sub{")) syncdirection = SYNC_TO_SUB;
	if (syncdirection == SYNC_NO && rulestring_one.Contains("from_sub{")) syncdirection = SYNC_FROM_SUB;

	// Копировать параметр в свойство или свойство в параметр
	if (syncdirection == SYNC_NO && rulestring_one.Contains("from{")) syncdirection = SYNC_FROM;
	if (syncdirection == SYNC_NO && rulestring_one.Contains("to{")) syncdirection = SYNC_TO;

	//Если направление синхронизации не нашли - выходим
	if (syncdirection == SYNC_NO) return false;

	GS::UniString paramNamePrefix = "";
	bool synctypefind = false;

	//Выбор типа копируемого свойства
	//Я не очень понял - умеет ли с++ в ленивые вычисления, поэтому сделаю вложенные условия, чтобы избежать ненужного поиска по строке
	if (rulestring_one.Contains("symb_pos_x") || rulestring_one.Contains("symb_pos_y") || rulestring_one.Contains("symb_pos_z")) {
		rulestring_one.ReplaceAll("{symb_pos_", "{Coord:symb_pos_");
	}

	if (synctypefind == false) {
		if (rulestring_one.Contains("{id}") || rulestring_one.Contains("{ID}")) {
			paramNamePrefix = "{@id:";
			param.fromID = true;
			synctypefind = true;
		}
	}
	if (synctypefind == false) {
		if (!rulestring_one.Contains(":") || rulestring_one.Contains("escription:") || rulestring_one.Contains("esc:")) {
			if (rulestring_one.Contains("escription:") || rulestring_one.Contains("esc:")) {
				param.fromGDLdescription = true;
				rulestring_one.ReplaceAll("description:", "");
				rulestring_one.ReplaceAll("Description:", "");
				rulestring_one.ReplaceAll("desc:", "");
				rulestring_one.ReplaceAll("Desc:", "");
			}
			paramNamePrefix = "{@gdl:";
			param.fromGDLparam = true;
			synctypefind = true;
		}
	}
	if (synctypefind == false) {
		if (rulestring_one.Contains("Property:")) {
			synctypefind = true;
			rulestring_one.ReplaceAll("Property:", "");
			paramNamePrefix = "{@property:";
			param.fromProperty = true;
		}
	}
	if (synctypefind == false) {
		if (rulestring_one.Contains("Material:") && rulestring_one.Contains('"')) {

			//TODO Проверить на файле из видео обработку материалов
			synctypefind = true;
			rulestring_one.ReplaceAll("Material:", "");
			rulestring_one.ReplaceAll("{Layers;", "{Layers,20;");
			paramNamePrefix = "{@material:";
			GS::UniString templatestring = rulestring_one.GetSubstring('"', '"', 0);
			param.val.uniStringValue = templatestring;
			rulestring_one.ReplaceAll(templatestring, "");
			param.fromMaterial = true;
			syncdirection = SYNC_FROM;
		}
	}
	if (synctypefind == false) {
		if (rulestring_one.Contains("Morph:")) {
			synctypefind = true;
			rulestring_one.ReplaceAll("Morph:", "");
			paramNamePrefix = "{@morph:";
			param.fromMorph = true;
			syncdirection = SYNC_FROM;
		}
	}
	if (synctypefind == false) {
		if (rulestring_one.Contains("Coord:")) {
			synctypefind = true;
			rulestring_one.ReplaceAll("Coord:", "");
			paramNamePrefix = "{@coord:";
			param.fromCoord = true;
			if (rulestring_one.Contains("orth")) param.fromGlob = true;
			syncdirection = SYNC_FROM;
		}
	}
	if (synctypefind == false) {
		if (rulestring_one.Contains("Info:")) {
			synctypefind = true;
			rulestring_one.ReplaceAll("Info:", "");
			paramNamePrefix = "{@info:";
			param.fromInfo = true;
		}
	}
	if (synctypefind == false) {
		if (rulestring_one.Contains("IFC:")) {
			synctypefind = true;
			rulestring_one.ReplaceAll("IFC:", "");
			paramNamePrefix = "{@ifc:";
			param.fromIFCProperty = true;
			syncdirection = SYNC_FROM;
		}
	}
	if (synctypefind == false) {
		if (rulestring_one.Contains("Glob:")) {
			synctypefind = true;
			rulestring_one.ReplaceAll("Glob:", "");
			paramNamePrefix = "{@glob:";
			param.fromGlob = true;
			syncdirection = SYNC_FROM;
		}
	}
	if (synctypefind == false) return false;

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

	//Если тип свойства не нашли - выходим
	if (synctypefind == false) return false;

	GS::UniString tparamName = rulestring_one.GetSubstring('{', '}', 0);
	GS::Array<GS::UniString> params;
	UInt32 nparam = StringSplt(tparamName, ";", params);

	// Параметры не найдены - выходим
	if (nparam == 0) return false;
	GS::UniString paramName = params.Get(0);
	stringformat = GetFormatString(paramName);
	paramName.ReplaceAll("\\/", "/");
	param.rawName = paramNamePrefix + paramName.ToLowerCase() + "}";
	param.name = paramName;
	if (nparam > 1) {
		for (UInt32 j = 1; j < nparam; j++) {
			GS::UniString ignoreval;
			if (params[j].Contains('"')) {
				ignoreval = params[j].GetSubstring('"', '"', 0);
			}
			else {
				ignoreval = params[j];
			}
			ignorevals.Push(ignoreval);
		}
	}
	return true;
}