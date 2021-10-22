#include "APIEnvir.h"
#include "ACAPinc.h"
#include "Sync.hpp"
#include "Helpers.hpp"
#include "Log.hpp"

const int ERRIGNOREVAL = 10;
Int32 nLib = 0;

// -----------------------------------------------------------------------------
// Отслеживание резервируемых элементов
// -----------------------------------------------------------------------------
void	SyncReservation(short type, const API_Guid objectId)
{
	if (type == 1) {
		SyncPrefs prefsData;
		SyncSettingsGet(prefsData);
		if (prefsData.syncMon || prefsData.logMon) {
			AttachObserver(objectId);
		}
	}
	return;
}		/* PrintReservationInfo */


// -----------------------------------------------------------------------------
// Парсит описание свойства
// Результат
//	имя параметра (свойства)
//	тип синхронизации (читаем из параметра GDL - 1, из свойства - 2)
//	направление синхронизации для работы с GDL (читаем из параметра - 1, записываем в параметр - 2)
// -----------------------------------------------------------------------------
bool SyncString(GS::UniString& description_string, GS::Array <SyncRule>& syncRules) {
	bool flag_sync = false;
	if (description_string.IsEmpty()) {
		return flag_sync;
	}
	if (description_string.Contains("Sync_") && description_string.Contains("{") && description_string.Contains("}")) {
		flag_sync = true;
		GS::Array<GS::UniString> rulestring;
		UInt32 nrule = StringSplt(description_string, "Sync_", rulestring, "{"); // Проверяем количество правил
		for (UInt32 i = 0; i < nrule; i++) {
			GS::UniString rulestring_one = rulestring[i];
			SyncRule rule;
			rule.syncdirection = 0;
			// Копировать параметр в свойство или свойство в параметр
			if (rulestring_one.Contains("from")) rule.syncdirection = 1;
			if (rulestring_one.Contains("to")) rule.syncdirection = 2;
			 //Копировать параметр или свойство
			rule.synctype = 1;
			if (rulestring_one.Contains("Property:")) {
				rule.synctype = 2;
				rulestring_one.ReplaceAll("Property:", "");
			}
			GS::UniString paramName = rulestring_one.GetSubstring('{', '}',0);
			UInt32 nparam = 0;
			GS::Array<GS::UniString> params;
			nparam = StringSplt(paramName, ";", params);
			if (nparam == 0) rule.syncdirection = 0;
			if (nparam > 0) rule.paramName = params[0];
			if (nparam > 1) {
				for (UInt32 j = 1; j < nparam; j++) {
					GS::UniString ignoreval;
					if (params[j].Contains('"')) {
						ignoreval = params[j].GetSubstring('"', '"', 0);
					}
					else {
						ignoreval = params[j];
					}
					rule.ignorevals.Push(ignoreval);
				}
			}
			if (rule.syncdirection>0) {
				syncRules.Push(rule);
			}
		}
	}
	if (syncRules.GetSize() == 0) flag_sync = false;
	return flag_sync;
}

// -----------------------------------------------------------------------------
// Проверяем - содержит ли строка игнорируемые значения
// -----------------------------------------------------------------------------
bool SyncCheckIgnoreVal(const SyncRule& syncRule, const GS::UniString& val) {
	bool ignore_flag = false;
	if (syncRule.ignorevals.GetSize() > 0) {
		for (UInt32 i = 0; i < syncRule.ignorevals.GetSize(); i++) {
			if ((syncRule.ignorevals[i].ToLowerCase()=="empty" || syncRule.ignorevals[i].ToLowerCase() == "пусто") && val.GetLength()<1) {
				ignore_flag = true;
				return ignore_flag;
			}
			if (val == syncRule.ignorevals[i]) {
				ignore_flag = true;
				return ignore_flag;
			}
		}
	}
	return ignore_flag;
}

// -----------------------------------------------------------------------------
// Проверяем - содержит ли свойство игнорируемые значеения
// -----------------------------------------------------------------------------
bool SyncCheckIgnoreVal(const SyncRule& syncRule, const API_Property& property) {
	bool ignore_flag = false;
	if (syncRule.ignorevals.GetSize() > 0) {
		GS::UniString val = PropertyTestHelpers::ToString(property);
		ignore_flag = SyncCheckIgnoreVal(syncRule, val);
	}
	return ignore_flag;
}

// -----------------------------------------------------------------------------
// Запись значения свойства в другое свойство
// -----------------------------------------------------------------------------
GSErrCode SyncPropAndProp(const API_Guid& elemGuid, const SyncRule& syncRule, API_Property& property)
{
	GSErrCode	err = NoError;
	API_Property propertyfrom;
	err = GetPropertyByName(elemGuid, syncRule.paramName, propertyfrom);
	if (err == NoError) {
		if (!SyncCheckIgnoreVal(syncRule, propertyfrom)) {
			err = WriteProp2Prop(elemGuid, propertyfrom, property);
		}
		else {
			err = APIERR_MISSINGCODE; // Игнорируем значение
		}
	}
	return err;
}

// -----------------------------------------------------------------------------
// Синхронизация значений свойства и параметра
// -----------------------------------------------------------------------------
GSErrCode SyncParamAndProp(const API_Guid& elemGuid, SyncRule& syncRule, API_Property& property)
{
	GSErrCode		err = NoError;
	GS::UniString param_string = "";
	GS::Int32 param_int = 0;
	bool param_bool = false;
	double param_real = 0;
	if (syncRule.syncdirection == 1) {
		if (GetLibParam(elemGuid, syncRule.paramName, param_string, param_int, param_bool, param_real))
		{
			if (!SyncCheckIgnoreVal(syncRule, param_string)) {
				err = WriteProp(elemGuid, property, param_string, param_int, param_bool, param_real);
			}
			else {
				err = APIERR_MISSINGCODE; // Игнорируем значение
			}
		}
		else {
			err = APIERR_MISSINGCODE; // Параметр не найден
		}
	}
	return err;
}

 //--------------------------------------------------------------------------------------------------------------------------
 //Ищет свойство со значение "Sync_flag" в описании и по значению определяет - нужно ли синхронизировать параметры элемента
 //--------------------------------------------------------------------------------------------------------------------------
bool SyncState(const API_Guid& elemGuid, const GS::Array<API_PropertyDefinition> definitions) {
	GSErrCode	err = NoError;
	// Проверяем - не отключена ли синхронизация у данного объекта
	bool isSync = false;
	for (UInt32 i = 0; i < definitions.GetSize(); i++) {
		if (definitions[i].description.Contains("Sync_flag")) {
			API_Property propertyflag = {};
			err = ACAPI_Element_GetPropertyValue(elemGuid, definitions[i].guid, propertyflag);
			if (err != NoError) msg_rep("SyncState", "ACAPI_Element_GetPropertyValue "+ definitions[i].name, err, elemGuid);
			if (err == NoError) {
				if (propertyflag.isDefault) {
					isSync = propertyflag.definition.defaultValue.basicValue.singleVariant.variant.boolValue;
					break;
				}
				else
				{
					isSync = propertyflag.value.singleVariant.variant.boolValue;
					break;
				}
			}
		}
	}
	return isSync;
}

GSErrCode SyncRelationsToWindow (const API_Guid& elemGuid) {
	GSErrCode			err = NoError;
	//Обновляем объекты, если их обработка включена
	API_WindowRelation            relData;
	err = ACAPI_Element_GetRelations(elemGuid, API_ZoneID, &relData);
	if (err == NoError) {
		if (relData.fromRoom != APINULLGuid) SyncData(relData.fromRoom);
		if (relData.toRoom != APINULLGuid) SyncData(relData.toRoom);
	}
	return err;
}

GSErrCode SyncRelationsToDoor(const API_Guid& elemGuid) {
	GSErrCode			err = NoError;
	//Обновляем объекты, если их обработка включена
	API_DoorRelation            relData;
	err = ACAPI_Element_GetRelations(elemGuid, API_ZoneID, &relData);
	if (err == NoError) {
		if (relData.fromRoom != APINULLGuid) SyncData(relData.fromRoom);
		if (relData.toRoom != APINULLGuid) SyncData(relData.toRoom);
	}
	return err;
}

// --------------------------------------------------------------------
// Поиск и синхронизация свойств связанных элементов
// --------------------------------------------------------------------
void SyncRelationsElement(const API_Guid& elemGuid) {
	GSErrCode	err = NoError;
	API_ElemTypeID elementType;
	SyncPrefs prefsData;
	SyncSettingsGet(prefsData);
	err = GetTypeByGUID(elemGuid, elementType);
	switch (elementType) {
		case API_WindowID:
			if (prefsData.objS) err = SyncRelationsToWindow(elemGuid);
			break;
		case API_DoorID:
			if (prefsData.objS) err = SyncRelationsToDoor(elemGuid);
			break;
		default:
			break;
	}
}

// --------------------------------------------------------------------
// Синхронизация данных элемента согласно указаниям в описании свойств
// --------------------------------------------------------------------
void SyncData(const API_Guid& elemGuid) {
	GSErrCode	err = NoError;
	API_ElemTypeID elementType;
	err = GetTypeByGUID(elemGuid, elementType);
	if (CheckElementType(elementType)) // Сверяемся с настройками - нужно ли этот тип обрабатывать
	{
		GS::Array<API_PropertyDefinition> definitions;
		err = ACAPI_Element_GetPropertyDefinitions(elemGuid, API_PropertyDefinitionFilter_UserDefined, definitions);
		if (err != NoError) msg_rep("SyncData", "ACAPI_Element_GetPropertyDefinitions", err, elemGuid);
		if (SyncState(elemGuid, definitions)) { // Проверяем - не отключена ли синхронизация у данного объекта
			for (UInt32 i = 0; i < definitions.GetSize(); i++) {
				err = SyncOneProperty(elemGuid, elementType, definitions[i]);
			}
		}
	}
}

// --------------------------------------------------------------------
// Синхронизация правил для одного свойства
// --------------------------------------------------------------------
GSErrCode SyncOneProperty(const API_Guid& elemGuid, const API_ElemTypeID elementType, API_PropertyDefinition definition) {
	GSErrCode	err = NoError;
	GS::Array <SyncRule> syncRules;
	if (SyncString(definition.description, syncRules)) { // Парсим описание свойства
		API_Property property = {};
		err = ACAPI_Element_GetPropertyValue(elemGuid, definition.guid, property);
		if (err != NoError) msg_rep("SyncData", "ACAPI_Element_GetPropertyValue " + definition.name, err, elemGuid);
		if (err == NoError) {
			for (UInt32 i = 0; i < syncRules.GetSize(); i++) {
				if (SyncOneRule(elemGuid, elementType, property, syncRules[i])) break; // Если синхронизация успешная - выходим из цикла
			}
		}
	}
	return err;
}

// --------------------------------------------------------------------
// Синхронизация одного правила для свойства
// Если синхронизация успешна, возвращает True
// Если свойство или параметр не найдены, либо содержат игнорируемые символы - возращает False
// --------------------------------------------------------------------
bool SyncOneRule(const API_Guid& elemGuid, const API_ElemTypeID elementType, API_Property property, SyncRule syncRule) {
	GSErrCode	err = NoError;
	switch (syncRule.synctype) {
	case 1:
		if (elementType == API_ObjectID || elementType == API_WindowID || elementType == API_DoorID || elementType == API_ZoneID || elementType == API_LampID) {
			err = SyncParamAndProp(elemGuid, syncRule, property); //Синхронизация свойства и параметра
		}
		break;
	case 2:
		err = SyncPropAndProp(elemGuid, syncRule, property); //Синхронизация свойств
		break;
	default:
		break;
	}
	if (err != NoError) {
		return false;
	}
	else {
		return true;
	}
}

// --------------------------------------------------------------------
// Проверяет - попадает ли тип элемента в под настройки синхронизации
// --------------------------------------------------------------------
bool CheckElementType(const API_ElemTypeID& elementType) {
	SyncPrefs prefsData;
	SyncSettingsGet(prefsData);
	bool flag_type = false;
	if ((elementType == API_WallID || elementType == API_ColumnID || elementType == API_BeamID || elementType == API_SlabID ||
		elementType == API_RoofID || elementType == API_MeshID || elementType == API_ZoneID || elementType == API_CurtainWallID ||
		elementType == API_CurtainWallSegmentID || elementType == API_CurtainWallFrameID || elementType == API_CurtainWallPanelID ||
		elementType == API_CurtainWallJunctionID || elementType == API_CurtainWallAccessoryID || elementType == API_ShellID ||
		elementType == API_MorphID || elementType == API_StairID || elementType == API_RiserID ||
		elementType == API_TreadID || elementType == API_StairStructureID ||
		elementType == API_RailingID || elementType == API_RailingToprailID || elementType == API_RailingHandrailID ||
		elementType == API_RailingRailID || elementType == API_RailingPostID || elementType == API_RailingInnerPostID ||
		elementType == API_RailingBalusterID || elementType == API_RailingPanelID || elementType == API_RailingSegmentID ||
		elementType == API_RailingNodeID ||
		elementType == API_RailingBalusterSetID ||
		elementType == API_RailingPatternID ||
		elementType == API_RailingToprailEndID ||
		elementType == API_RailingHandrailEndID ||
		elementType == API_RailingRailEndID ||
		elementType == API_RailingToprailConnectionID ||
		elementType == API_RailingHandrailConnectionID ||
		elementType == API_RailingRailConnectionID ||
		elementType == API_RailingEndFinishID ||
		elementType == API_BeamSegmentID ||
		elementType == API_ColumnSegmentID ||
		elementType == API_OpeningID) && prefsData.wallS) flag_type = true;
	if ((elementType == API_ObjectID || elementType == API_ZoneID || elementType == API_LampID) && prefsData.objS) flag_type = true;
	if ((elementType == API_WindowID || elementType == API_DoorID || elementType == API_SkylightID) && prefsData.widoS) flag_type = true;
	return flag_type;
}

// -----------------------------------------------------------------------------
// Запускает обработку всех объектов, заданных в настройке
// -----------------------------------------------------------------------------
void SyncAndMonAll(void) {
	GS::UniString	title("Sync All");
	nLib += 1;
	ACAPI_Interface(APIIo_InitProcessWindowID, &title, &nLib);
	bool flag_chanel = false;
	SyncPrefs prefsData;
	SyncSettingsGet(prefsData);
	GS::UniString undoString = RSGetIndString(AddOnStringsID, UndoSyncId, ACAPI_GetOwnResModule());
	ACAPI_CallUndoableCommand(undoString, [&]() -> GSErrCode {
		if (!flag_chanel && prefsData.objS) flag_chanel = SyncByType(API_ObjectID);
		if (!flag_chanel && prefsData.widoS) flag_chanel = SyncByType(API_WindowID);
		if (!flag_chanel && prefsData.widoS) flag_chanel = SyncByType(API_DoorID);
		if (!flag_chanel && prefsData.objS) flag_chanel = SyncByType(API_ZoneID);
		if (!flag_chanel && prefsData.wallS) flag_chanel = SyncByType(API_WallID);
		if (!flag_chanel && prefsData.wallS) flag_chanel = SyncByType(API_SlabID);
		if (!flag_chanel && prefsData.wallS) flag_chanel = SyncByType(API_ColumnID);
		if (!flag_chanel && prefsData.wallS) flag_chanel = SyncByType(API_BeamID);
		if (!flag_chanel && prefsData.wallS) flag_chanel = SyncByType(API_RoofID);
		if (!flag_chanel && prefsData.wallS) flag_chanel = SyncByType(API_MeshID);
		if (!flag_chanel && prefsData.wallS) flag_chanel = SyncByType(API_MorphID);
		return NoError;
		});
}

// -----------------------------------------------------------------------------
// Запускает обработку всех элементов заданного типа
// -----------------------------------------------------------------------------
bool SyncByType(const API_ElemTypeID& elementType) {
	SyncPrefs prefsData;
	SyncSettingsGet(prefsData);
	GS::UniString	subtitle;
	GSErrCode		err = NoError;
	GS::Array<API_Guid> guidArray;
	bool flag_chanel = false;
	ACAPI_Element_GetElemList(elementType, &guidArray, APIFilt_IsEditable);

	if (!guidArray.IsEmpty()) {
		if (ACAPI_Goodies(APIAny_GetElemTypeNameID, (void*)elementType, &subtitle) == NoError) {
			nLib += 1;
			ACAPI_Interface(APIIo_SetNextProcessPhaseID, &subtitle, &nLib);
			GS::UniString intString = GS::UniString::Printf(" %d", guidArray.GetSize());
			msg_rep("SyncByType", subtitle + intString, NoError, APINULLGuid);
		}
		for (UInt32 i = 0; i < guidArray.GetSize(); i++) {
			if (prefsData.syncAll) SyncData(guidArray[i]);
			if (prefsData.syncMon) {
				err = AttachObserver(guidArray[i]);
				if (err == APIERR_LINKEXIST)
					err = NoError;
			}
			ACAPI_Interface(APIIo_SetProcessValueID, &i, nullptr);
			if (ACAPI_Interface(APIIo_IsProcessCanceledID, nullptr, nullptr)) {
				flag_chanel = true;
				msg_rep("SyncByType", subtitle + " - отмена", NoError, APINULLGuid);
				return flag_chanel;
			}
		}
	}
	return flag_chanel;
}

void SyncSelected(void) {
	GS::UniString undoString = RSGetIndString(AddOnStringsID, UndoSyncId, ACAPI_GetOwnResModule());
	ACAPI_CallUndoableCommand(undoString, [&]() -> GSErrCode {
		CallOnSelectedElem(SyncData);
		return NoError;
		});
}

//void WriteParam() 
//{
//	API_GetParamsType theParams = {};
//	API_ParamOwnerType paramOwner = {};
//	GS::uchar_t uStrBuffer[1024];
//
//	paramOwner.typeID = API_ObjectID;
//	paramOwner.libInd = libPartIndex;
//
//	GSErrCode err = ACAPI_Goodies(APIAny_OpenParametersID, &paramOwner);
//	err |= ACAPI_Goodies(APIAny_GetActParametersID, &theParams);
//	if (err == NoError) {
//		API_ChangeParamType changeParam = {};
//
//		CHTruncate(paramName, changeParam.name, API_NameLen);
//		GS::ucsncpy(uStrBuffer, newStrValue.ToUStr().Get(), BUFFER_SIZE);
//		changeParam.uStrValue = uStrBuffer;
//
//		err = ACAPI_Goodies(APIAny_ChangeAParameterID, &changeParam);
//		if (err == NoError) {
//			API_ElementMemo memo = {};
//			memo.params = theParams.params;
//			API_Element	mask;
//			ACAPI_ELEMENT_MASK_CLEAR(mask);
//			API_Element	elem = {};
//			elem.header.guid = elemGuid;
//			ACAPI_Element_Get(&elem);
//			err = ACAPI_Element_Change(&elem, &mask, &memo, APIMemoMask_AddPars, true);
//		}
//	}
//	ACAPI_DisposeAddParHdl(&theParams.params);
//	ACAPI_Goodies(APIAny_CloseParametersID);
//}
