#include "APIEnvir.h"
#include "ACAPinc.h"
#include "Sync.hpp"
#include "Helpers.hpp"
#include "Log.hpp"

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
			ACAPI_Element_AttachObserver(objectId);
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
bool SyncString(GS::UniString& description_string, SyncRule &syncRule) {
	bool flag_sync = false;
	if (description_string.IsEmpty()) {
		return flag_sync;
	}
	if (description_string.Contains("Sync_") && description_string.Contains("{") && description_string.Contains("}")) {
		flag_sync = true;
		syncRule.synctype = 1;
		syncRule.syncdirection = 2;
		description_string.Trim('\r');
		description_string.Trim('\n');
		description_string.Trim();
		if (description_string.Contains("Sync_from")) syncRule.syncdirection = 1;
		if (description_string.Contains("Property:")) {
			syncRule.synctype = 2;
			description_string.ReplaceAll("Property:", "");
		}
		GS::UniString paramName = description_string.GetSubstring('{', '}', 0);
		paramName.ReplaceAll("Property:", "");
		GS::Array<GS::UniString> partstring;
		//int nparam = StringSplt(paramName, ";", partstring);
		//TODO Добавить исключаемые значения
		syncRule.paramName = paramName;
	}
	return flag_sync;
}

// -----------------------------------------------------------------------------
// Запись значения свойства в другое свойство
// -----------------------------------------------------------------------------
GSErrCode SyncPropAndProp(const API_Guid& elemGuid, API_Property& property, SyncRule& syncRule)
{
	GSErrCode		err = NoError;
	API_Property propertyfrom;
	err = GetPropertyByName(elemGuid, syncRule.paramName, propertyfrom);
	if (err == NoError) {
		err = WriteProp2Prop(elemGuid, property, propertyfrom);
	}
	return err;
}

// -----------------------------------------------------------------------------
// Синхронизация значений свойства и параметра
// -----------------------------------------------------------------------------
GSErrCode SyncParamAndProp(const API_Guid& elemGuid, SyncRule& syncRule, API_Property& property)
{
	GSErrCode		err = NoError;
	if (syncRule.syncdirection == 1) {
		err = WriteParam2Prop(elemGuid, property, syncRule.paramName);
	}
	return err;
}

 //--------------------------------------------------------------------------------------------------------------------------
 //Ищет свойство со значение "Sync_flag" в описании и по значению определяет - нужно ли синхронизировать параметры элемента
 //--------------------------------------------------------------------------------------------------------------------------
bool SyncState(const API_Guid& elemGuid, const GS::Array<API_PropertyDefinition> definitions) {
	GSErrCode		err = NoError;
	// Проверяем - не отключена ли синхронизация у данного объекта
	bool isSync = false;
	for (UInt32 i = 0; i < definitions.GetSize(); i++) {
		if (definitions[i].description.Contains("Sync_flag")) {
			API_Property propertyflag = {};
			err = ACAPI_Element_GetPropertyValue(elemGuid, definitions[i].guid, propertyflag);
			if (err != NoError) msg_rep("SyncState", "CAPI_Element_GetPropertyValue "+ definitions[i].name, err, elemGuid);
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

// --------------------------------------------------------------------
// Синхронизация данных элемента согласно указаниям в описании свойств
// --------------------------------------------------------------------
void SyncData(const API_Guid& elemGuid) {
	GSErrCode		err = NoError;
	API_ElemTypeID elementType;
	err = GetTypeByGUID(elemGuid, elementType);
	if (CheckElementType(elementType)) // Сверяемся с настройками - нужно ли этот тип обрабатывать
	{
		GS::Array<API_PropertyDefinition> definitions;
		err = ACAPI_Element_GetPropertyDefinitions(elemGuid, API_PropertyDefinitionFilter_UserDefined, definitions);
		if (err != NoError) msg_rep("SyncData", "ACAPI_Element_GetPropertyDefinitions", err, elemGuid);
		if (SyncState(elemGuid, definitions)) { // Проверяем - не отключена ли синхронизация у данного объекта
			for (UInt32 i = 0; i < definitions.GetSize(); i++) {
				SyncRule syncRule;
				if (SyncString(definitions[i].description, syncRule)) { // Парсим описание свойства
					API_Property property = {};
					err = ACAPI_Element_GetPropertyValue(elemGuid, definitions[i].guid, property);
					if (err != NoError) msg_rep("SyncData", "ACAPI_Element_GetPropertyValue " + definitions[i].name, err, elemGuid);
					if (err == NoError) {
						switch (syncRule.synctype) {
						case 1:
							if (elementType == API_ObjectID || elementType == API_WindowID || elementType == API_DoorID || elementType == API_ZoneID) {
								err = SyncParamAndProp(elemGuid, syncRule, property); //Синхронизация свойства и параметра
							}
							break;
						case 2:
							err = SyncPropAndProp(elemGuid, property, syncRule); //Синхронизация свойств
							break;
						default:
							break;
						}//switch (synctype) 
					} // ACAPI_Element_GetPropertyValue
				} // if (isSync)
			} // for
		} // if (SyncState(elemGuid, definitions))
	} // if (CheckElementType(elementType))
} // SyncData

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
	if ((elementType == API_ObjectID || elementType == API_ZoneID) && prefsData.objS) flag_type = true;
	if ((elementType == API_WindowID || elementType == API_DoorID) && prefsData.widoS) flag_type = true;
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
				err = ACAPI_Element_AttachObserver(guidArray[i]);
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
