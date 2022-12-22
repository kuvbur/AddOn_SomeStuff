//------------ kuvbur 2022 ------------
#include	<stdlib.h> /* atoi */
#include	"APIEnvir.h"
#include	"ACAPinc.h"
#include	"Sync.hpp"
#include	"Helpers.hpp"
#include	"ResetProperty.hpp"
#include	"Dimensions.hpp"

#define SYNC_GDL 1
#define SYNC_PROPERTY 2
#define SYNC_MATERIAL 3
#define SYNC_INFO 4
#define SYNC_IFC 5
#define SYNC_MORPH 6
#define SYNC_CLASS 7

#define SYNC_NO 0
#define SYNC_FROM 1
#define SYNC_TO 2
#define SYNC_TO_SUB 3
#define SYNC_FROM_SUB 4

Int32 nLib = 0;

// -----------------------------------------------------------------------------
// Запускает обработку всех объектов, заданных в настройке
// -----------------------------------------------------------------------------
void SyncAndMonAll(SyncSettings& syncSettings) {
	if (ResetAllProperty()) syncSettings.syncAll = false;
	GS::UniString	title("Sync All");
	nLib += 1;
	ACAPI_Interface(APIIo_InitProcessWindowID, &title, &nLib);
	bool flag_chanel = false;
	GS::UniString undoString = RSGetIndString(AddOnStringsID, UndoSyncId, ACAPI_GetOwnResModule());
	ACAPI_CallUndoableCommand(undoString, [&]() -> GSErrCode {
		if (!flag_chanel && syncSettings.objS) flag_chanel = SyncByType(API_ObjectID, syncSettings);
		if (!flag_chanel && syncSettings.widoS) flag_chanel = SyncByType(API_WindowID, syncSettings);
		if (!flag_chanel && syncSettings.widoS) flag_chanel = SyncByType(API_DoorID, syncSettings);
		if (!flag_chanel && syncSettings.objS) flag_chanel = SyncByType(API_ZoneID, syncSettings);
		if (!flag_chanel && syncSettings.wallS) flag_chanel = SyncByType(API_WallID, syncSettings);
		if (!flag_chanel && syncSettings.wallS) flag_chanel = SyncByType(API_SlabID, syncSettings);
		if (!flag_chanel && syncSettings.wallS) flag_chanel = SyncByType(API_ColumnID, syncSettings);
		if (!flag_chanel && syncSettings.wallS) flag_chanel = SyncByType(API_BeamID, syncSettings);
		if (!flag_chanel && syncSettings.wallS) flag_chanel = SyncByType(API_RoofID, syncSettings);
		if (!flag_chanel && syncSettings.wallS) flag_chanel = SyncByType(API_MeshID, syncSettings);
		if (!flag_chanel && syncSettings.wallS) flag_chanel = SyncByType(API_MorphID, syncSettings);
		if (!flag_chanel && syncSettings.cwallS) flag_chanel = SyncByType(API_CurtainWallID, syncSettings);
		return NoError;
		});
}

// -----------------------------------------------------------------------------
// Запускает обработку всех элементов заданного типа
// -----------------------------------------------------------------------------
bool SyncByType(const API_ElemTypeID& elementType, const SyncSettings& syncSettings) {
	GS::UniString		subtitle;
	GSErrCode			err = NoError;
	GS::Array<API_Guid>	guidArray;
	bool flag_chanel = false;
	ACAPI_Element_GetElemList(elementType, &guidArray, APIFilt_IsEditable | APIFilt_HasAccessRight);
	if (!guidArray.IsEmpty()) {
#ifdef AC_26
		API_ElemType elemType;
		elemType.typeID = elementType;
		if (ACAPI_Goodies_GetElemTypeName(elemType, subtitle) == NoError) {
#else
		if (ACAPI_Goodies(APIAny_GetElemTypeNameID, (void*)elementType, &subtitle) == NoError) {
#endif // AC_26
			nLib += 1;
			ACAPI_Interface(APIIo_SetNextProcessPhaseID, &subtitle, &nLib);
			GS::UniString intString = GS::UniString::Printf(" %d", guidArray.GetSize());
			msg_rep("SyncByType", subtitle + intString, NoError, APINULLGuid);
		}
		for (UInt32 i = 0; i < guidArray.GetSize(); i++) {
			if (syncSettings.syncMon) {
				err = AttachObserver(guidArray[i], syncSettings);
				if (err == APIERR_LINKEXIST)
					err = NoError;
			}
			else {
				SyncElement(guidArray[i], syncSettings);
			}
			ACAPI_Interface(APIIo_SetProcessValueID, &i, nullptr);
			if (ACAPI_Interface(APIIo_IsProcessCanceledID, nullptr, nullptr)) {
				flag_chanel = true;
				msg_rep("SyncByType", subtitle + u8" - отмена", NoError, APINULLGuid);
				return flag_chanel;
			}
		}
	}
	return flag_chanel;
}

void SyncElement(const API_Guid & objectId, const SyncSettings & syncSettings) {
	GSErrCode		err = NoError;
	if (syncSettings.syncMon) {
		err = AttachObserver(objectId, syncSettings);
		if (err == APIERR_LINKEXIST)
			err = NoError;
	}
	if (err == NoError) {
		SyncData(objectId, syncSettings);
		SyncRelationsElement(objectId, syncSettings);
	}
}

// -----------------------------------------------------------------------------
// Запускает обработку выбранных, заданных в настройке
// -----------------------------------------------------------------------------
void SyncSelected(const SyncSettings & syncSettings) {
	GS::UniString undoString = RSGetIndString(AddOnStringsID, UndoSyncId, ACAPI_GetOwnResModule());
	ACAPI_CallUndoableCommand(undoString, [&]() -> GSErrCode {
		CallOnSelectedElemSettings(SyncElement, false, true, syncSettings);
		return NoError;
		});
}

// --------------------------------------------------------------------
// Поиск и синхронизация свойств связанных элементов
// --------------------------------------------------------------------
void SyncRelationsElement(const API_Guid & elemGuid, const SyncSettings & syncSettings) {
	GSErrCode	err = NoError;
	API_ElemTypeID elementType;
	bool flag_sync = false;
	err = GetTypeByGUID(elemGuid, elementType);
	if (err != NoError) { return; }
	switch (elementType) {
	case API_WindowID:
	case API_DoorID:
		if (syncSettings.objS) flag_sync = true;
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
	if (flag_sync) {
		GS::Array<API_Guid> subelemGuid;
		SyncGetRelationsElement(elemGuid, subelemGuid);
		if (subelemGuid.GetSize() > 0) {
			for (UInt32 i = 0; i < subelemGuid.GetSize(); ++i) {
				SyncData(subelemGuid[i], syncSettings);
			}
		}
	}
}

// --------------------------------------------------------------------
// Поиск связанных элементов
// --------------------------------------------------------------------
void SyncGetRelationsElement(const API_Guid & elemGuid, GS::Array<API_Guid>&subelemGuid) {
	GSErrCode	err = NoError;
	API_ElemTypeID elementType;
	API_RoomRelation	relData;
	GS::Array<API_ElemTypeID> typeinzone;
	err = GetTypeByGUID(elemGuid, elementType);
	if (err != NoError) { 
		ACAPI_DisposeRoomRelationHdls(&relData);
		return;
	}
	switch (elementType) {
	case API_RailingID:
		err = GetRElementsForRailing(elemGuid, subelemGuid);
		break;
	case API_CurtainWallID:
		err = GetCWElementsForCWall(elemGuid, subelemGuid);
		break;
	case API_CurtainWallSegmentID:
	case API_CurtainWallFrameID:
	case API_CurtainWallJunctionID:
	case API_CurtainWallAccessoryID:
	case API_CurtainWallPanelID:
		API_CWPanelRelation crelData;
		err = ACAPI_Element_GetRelations(elemGuid, API_ZoneID, &crelData);
		if (err == NoError) {
			if (crelData.fromRoom != APINULLGuid) subelemGuid.Push(crelData.fromRoom);
			if (crelData.toRoom != APINULLGuid) subelemGuid.Push(crelData.toRoom);
		}
		break;
	case API_DoorID:
		API_DoorRelation drelData;
		err = ACAPI_Element_GetRelations(elemGuid, API_ZoneID, &drelData);
		if (err == NoError) {
			if (drelData.fromRoom != APINULLGuid) subelemGuid.Push(drelData.fromRoom);
			if (drelData.toRoom != APINULLGuid) subelemGuid.Push(drelData.toRoom);
		}
		break;
	case API_WindowID:
		API_WindowRelation wrelData;
		err = ACAPI_Element_GetRelations(elemGuid, API_ZoneID, &wrelData);
		if (err == NoError) {
			if (wrelData.fromRoom != APINULLGuid) subelemGuid.Push(wrelData.fromRoom);
			if (wrelData.toRoom != APINULLGuid) subelemGuid.Push(wrelData.toRoom);
		}
		break;
	case API_ZoneID:
		err = ACAPI_Element_GetRelations(elemGuid, API_ZombieElemID, &relData);
		if (err == NoError) {
			typeinzone.Push(API_WallID);
			typeinzone.Push(API_SlabID);
			typeinzone.Push(API_ColumnID);
			typeinzone.Push(API_BeamID);
			typeinzone.Push(API_RoofID);
			typeinzone.Push(API_ShellID);
			typeinzone.Push(API_MorphID);

			typeinzone.Push(API_WindowID);
			typeinzone.Push(API_DoorID);

			typeinzone.Push(API_ObjectID);
			typeinzone.Push(API_LampID);
			typeinzone.Push(API_StairID);
			typeinzone.Push(API_RiserID);
			typeinzone.Push(API_TreadID);
			typeinzone.Push(API_StairStructureID);
			typeinzone.Push(API_RailingID);
			typeinzone.Push(API_RailingToprailID);
			typeinzone.Push(API_RailingHandrailID);
			typeinzone.Push(API_RailingRailID);
			typeinzone.Push(API_RailingPostID);
			typeinzone.Push(API_RailingInnerPostID);
			typeinzone.Push(API_RailingBalusterID);
			typeinzone.Push(API_RailingPanelID);
			typeinzone.Push(API_RailingSegmentID);
			typeinzone.Push(API_RailingToprailEndID);
			typeinzone.Push(API_RailingHandrailEndID);
			typeinzone.Push(API_RailingRailEndID);
			typeinzone.Push(API_RailingToprailConnectionID);
			typeinzone.Push(API_RailingHandrailConnectionID);
			typeinzone.Push(API_RailingRailConnectionID);
			typeinzone.Push(API_RailingNodeID);

			typeinzone.Push(API_CurtainWallID);
			typeinzone.Push(API_CurtainWallFrameID);
			typeinzone.Push(API_CurtainWallPanelID);
			typeinzone.Push(API_CurtainWallJunctionID);
			typeinzone.Push(API_CurtainWallAccessoryID);
			typeinzone.Push(API_CurtainWallSegmentID);
			typeinzone.Push(API_SkylightID);
			for (const API_ElemTypeID& typeelem : typeinzone) {
				if (relData.elementsGroupedByType.ContainsKey(typeelem)) {
					for (const API_Guid& elGuid : relData.elementsGroupedByType[typeelem]) {
						subelemGuid.Push(elGuid);
					}
				}
			}
		}
		break;
	default:
		break;
	}
	ACAPI_DisposeRoomRelationHdls(&relData);
}

// --------------------------------------------------------------------
// Синхронизация данных элемента согласно указаниям в описании свойств
// --------------------------------------------------------------------
void SyncData(const API_Guid & elemGuid, const SyncSettings & syncSettings) {
	GSErrCode	err = NoError;
	if (!IsElementEditable(elemGuid, syncSettings, true))
		return;
	API_ElemTypeID elementType;
	err = GetTypeByGUID(elemGuid, elementType);
	if (err != NoError) {
		msg_rep("SyncData", "GetTypeByGUID", err, elemGuid);
		return;
	}
	GS::Array<API_PropertyDefinition> definitions;
	err = ACAPI_Element_GetPropertyDefinitions(elemGuid, API_PropertyDefinitionFilter_UserDefined, definitions);
	if (err != NoError) msg_rep("SyncData", "ACAPI_Element_GetPropertyDefinitions", err, elemGuid);
	if (err == NoError) {
		// Установка правильной классификации
		if (SyncState(elemGuid, definitions, "Sync_class_flag")) { // Проверяем - не отключена ли проставление классификации
			GS::Array<GS::Pair<API_Guid, API_Guid>> systemItemPairs;
			err = ACAPI_Element_GetClassificationItems(elemGuid, systemItemPairs);
			GS::Array<API_ClassificationSystem> systems;
			ClassificationDict allclassification;
			err = ACAPI_Classification_GetClassificationSystems(systems);
			for (UInt32 i = 0; i < systems.GetSize(); i++) {
				GS::UniString sname = systems[i].name;
				API_Guid sguid = systems[i].guid;
				allclassification.Add(sname, sguid);

				GS::Array<API_ClassificationItem> rootitems;
				err = ACAPI_Classification_GetClassificationSystemRootItems(sguid, rootitems);
				for (UInt32 j = 0; j < rootitems.GetSize(); j++) {
					GS::UniString rname = sname + "@" + rootitems[j].name;
					API_Guid rguid = rootitems[j].guid;
					allclassification.Add(rname, sguid);
					GS::Array<API_ClassificationItem> children;
					err = ACAPI_Classification_GetClassificationItemChildren(rguid, children);
				}
			}
		}
		// Синхронизация данных
		if (SyncState(elemGuid, definitions, "Sync_flag")) { // Проверяем - не отключена ли синхронизация у данного объекта
			for (UInt32 i = 0; i < definitions.GetSize(); i++) {
				err = SyncOneProperty(elemGuid, elementType, definitions[i]);
			}
		}
	}
}

// --------------------------------------------------------------------
// Синхронизация правил для одного свойства
// --------------------------------------------------------------------
GSErrCode SyncOneProperty(const API_Guid & elemGuid, const API_ElemTypeID elementType, API_PropertyDefinition definition) {
	GSErrCode	err = NoError;
	GS::Array <SyncRule> syncRules;
	if (SyncString(definition.description, syncRules)) { // Парсим описание свойства
		for (UInt32 i = 0; i < syncRules.GetSize(); i++) {
			SyncOneRule(elemGuid, elementType, definition, syncRules[i]); // Если синхронизация успешная - выходим из цикла
		}
	}
	return err;
}

// --------------------------------------------------------------------
// Синхронизация одного правила для свойства
// Если синхронизация успешна, возвращает True
// Если свойство или параметр не найдены, либо содержат игнорируемые символы - возращает False
// --------------------------------------------------------------------
bool SyncOneRule(const API_Guid & elemGuid, const API_ElemTypeID & elementType, const API_PropertyDefinition & definition, SyncRule syncRule) {
	GSErrCode	err = NoError;
	GS::Array<API_Guid> elemGuid_from;
	GS::Array<API_Guid> elemGuid_to;
	switch (syncRule.syncdirection)
	{
	case SYNC_FROM:
		elemGuid_from.Push(elemGuid);
		elemGuid_to.Push(elemGuid);
		break;
	case SYNC_TO:
		elemGuid_from.Push(elemGuid);
		elemGuid_to.Push(elemGuid);
		break;
	case SYNC_FROM_SUB:
		SyncGetRelationsElement(elemGuid, elemGuid_from);
		if (elemGuid_from.GetSize() > 0) {
			for (UInt32 i = 0; i < elemGuid_from.GetSize(); i++) {
				elemGuid_to.Push(elemGuid);
			}
			syncRule.syncdirection = SYNC_FROM;
		}
		break;
	case SYNC_TO_SUB:
		SyncGetRelationsElement(elemGuid, elemGuid_to);
		if (elemGuid_to.GetSize() > 0) {
			for (UInt32 i = 0; i < elemGuid_to.GetSize(); i++) {
				elemGuid_from.Push(elemGuid);
			}
			syncRule.syncdirection = SYNC_TO;
		}
		break;
	default:
		break;
	}
	if (elemGuid_from.GetSize() == 0 || elemGuid_to.GetSize() == 0) return false;
	API_Property property = {};
	switch (syncRule.synctype) {
	case SYNC_GDL:
		for (UInt32 i = 0; i < elemGuid_to.GetSize(); i++) {
			err = SyncParamAndProp(elemGuid_from[i], elemGuid_to[i], syncRule, definition); //Синхронизация свойства и параметра
		}
		break;
	case SYNC_PROPERTY:
		for (UInt32 i = 0; i < elemGuid_to.GetSize(); i++) {
			err = SyncPropAndProp(elemGuid_from[i], elemGuid_to[i], syncRule, definition); //Синхронизация свойств
		}
		break;
	case SYNC_MATERIAL:
		err = SyncPropAndMat(elemGuid, elementType, syncRule, definition); //Синхронизация свойств и данных о конструкции
		break;
	case SYNC_IFC:
		err = SyncIFCAndProp(elemGuid, syncRule, definition); //Синхронизация IFC свойств с архикадовскими свойствами
		break;
	case SYNC_MORPH:
		err = SyncMorphAndProp(elemGuid, syncRule, definition); //Запись данных о морфе
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

// -----------------------------------------------------------------------------
// Парсит описание свойства
// Результат
//	имя параметра (свойства)
//	тип синхронизации (читаем из параметра GDL - 1, из свойства - 2, из состава конструкции - 3)
//	направление синхронизации для работы с GDL (читаем из параметра - 1, записываем в параметр - 2)
// -----------------------------------------------------------------------------
bool SyncString(GS::UniString & description_string, GS::Array <SyncRule>&syncRules) {
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
	if (description_string.Contains("Sync_") && description_string.Contains("{") && description_string.Contains("}")) {
		GS::Array<GS::UniString> rulestring;
		UInt32 nrule = StringSplt(description_string, "Sync_", rulestring, "{"); // Проверяем количество правил
		for (UInt32 i = 0; i < nrule; i++) {
			GS::UniString rulestring_one = rulestring[i];
			SyncRule rule;
			rule.syncdirection = SYNC_NO;

			// Выбор направления синхронизации
			// Копировать в субэлементы или из субэлементов
			if (rule.syncdirection == SYNC_NO && rulestring_one.Contains("to_sub{")) {
				rule.syncdirection = SYNC_TO_SUB;
			}
			if (rule.syncdirection == SYNC_NO && rulestring_one.Contains("from_sub{")) {
				rule.syncdirection = SYNC_FROM_SUB;
			}

			// Копировать параметр в свойство или свойство в параметр
			if (rule.syncdirection == SYNC_NO && rulestring_one.Contains("from{")) {
				rule.syncdirection = SYNC_FROM;
			}
			if (rule.syncdirection == SYNC_NO && rulestring_one.Contains("to{")) {
				rule.syncdirection = SYNC_TO;
			}
			if (rule.syncdirection != SYNC_NO) {

				//Выбор типа копируемого свойства
				rule.synctype = SYNC_NO;
				if (rule.synctype == SYNC_NO && rulestring_one.Contains("Property:")) {
					rule.synctype = SYNC_PROPERTY;
					rulestring_one.ReplaceAll("Property:", "");
				}
				if (rule.synctype == SYNC_NO && rulestring_one.Contains("Material:") && rulestring_one.Contains('"')) {
					rule.synctype = SYNC_MATERIAL;
					rulestring_one.ReplaceAll("Material:", "");
				}
				if (rule.synctype == SYNC_NO && rulestring_one.Contains("Info:") && rulestring_one.Contains('"')) {
					rule.synctype = SYNC_INFO;
					rulestring_one.ReplaceAll("Info:", "");
				}
				if (rule.synctype == SYNC_NO && rulestring_one.Contains("IFC:")) {
					rule.synctype = SYNC_IFC;
					rulestring_one.ReplaceAll("IFC:", "");
				}
				if (rule.synctype == SYNC_NO && rulestring_one.Contains("Morph:")) {
					rule.synctype = SYNC_MORPH;
				}
				if (rule.synctype == SYNC_NO) {
					rule.synctype = SYNC_GDL;
				}
				GS::UniString paramName = rulestring_one.GetSubstring('{', '}', 0);

				// Для материалов вытащим строку с шаблоном, чтоб ненароком не разбить её
				if (rule.synctype == SYNC_MATERIAL) {
					GS::UniString templatestring = rulestring_one.GetSubstring('"', '"', 0);
					rule.templatestring = templatestring;
					rulestring_one.ReplaceAll(templatestring, "");
				}
				UInt32 nparam = 0;
				GS::Array<GS::UniString> params;
				nparam = StringSplt(paramName, ";", params);
				if (nparam == 0) rule.syncdirection = SYNC_NO;
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
				syncRules.Push(rule);
			}
		}
	}
	if (syncRules.GetSize() == 0) {
		return false;
	}
	else {
		return true;
	}
}

//--------------------------------------------------------------------------------------------------------------------------
//Ищет свойство property_flag_name в описании и по значению определяет - нужно ли обрабатывать элемент
//--------------------------------------------------------------------------------------------------------------------------
bool SyncState(const API_Guid & elemGuid, const GS::Array<API_PropertyDefinition> definitions, GS::UniString property_flag_name) {
	GSErrCode	err = NoError;
	for (UInt32 i = 0; i < definitions.GetSize(); i++) {
		if (definitions[i].description.Contains(property_flag_name)) {
			API_Property propertyflag = {};
			err = ACAPI_Element_GetPropertyValue(elemGuid, definitions[i].guid, propertyflag);
			if (err != NoError) msg_rep("SyncState", "ACAPI_Element_GetPropertyValue " + definitions[i].name, err, elemGuid);
			if (err == NoError) {
				if (propertyflag.isDefault) {
					return propertyflag.definition.defaultValue.basicValue.singleVariant.variant.boolValue;
				}
				else
				{
					return propertyflag.value.singleVariant.variant.boolValue;
				}
			}
		}
	}
	return false;
}


// -----------------------------------------------------------------------------
// Запись значения свойства в другое свойство
// -----------------------------------------------------------------------------
GSErrCode SyncPropAndProp(const API_Guid & elemGuid_from, const API_Guid & elemGuid_to, const SyncRule & syncRule, const API_PropertyDefinition & definition)
{
	API_Property property_from;
	API_Property property_to;
	if (syncRule.syncdirection == SYNC_FROM) {
		if (GetPropertyByName(elemGuid_from, syncRule.paramName, property_from) != NoError) {
			msg_rep("SyncParamAndProp", "GetPropertyByName " + definition.name, APIERR_MISSINGCODE, elemGuid_from);
			return APIERR_MISSINGCODE;
		}
		if (ACAPI_Element_GetPropertyValue(elemGuid_to, definition.guid, property_to) != NoError) {
			msg_rep("SyncParamAndProp", "ACAPI_Element_GetPropertyValue " + definition.name, APIERR_MISSINGCODE, elemGuid_to);
			return APIERR_MISSINGCODE;
		}
	}
	if (syncRule.syncdirection == SYNC_TO) {
		if (GetPropertyByName(elemGuid_to, syncRule.paramName, property_to) != NoError) {
			msg_rep("SyncParamAndProp", "GetPropertyByName " + definition.name, APIERR_MISSINGCODE, elemGuid_to);
			return APIERR_MISSINGCODE;
		}
		if (ACAPI_Element_GetPropertyValue(elemGuid_from, definition.guid, property_from) != NoError) {
			msg_rep("SyncParamAndProp", "ACAPI_Element_GetPropertyValue " + definition.name, APIERR_MISSINGCODE, elemGuid_from);
			return APIERR_MISSINGCODE;
		}
	}
	if (!SyncCheckIgnoreVal(syncRule, property_from)) {
		return WriteProp2Prop(elemGuid_to, property_from, property_to);
	}
	else {
		return APIERR_MISSINGCODE; // Игнорируем значение
	}
	return NoError;
}

// -----------------------------------------------------------------------------
// Запись данных о морфе в свойство
// -----------------------------------------------------------------------------
GSErrCode SyncMorphAndProp(const API_Guid & elemGuid, const SyncRule & syncRule, const API_PropertyDefinition & definition) {
	API_Property property;
	GSErrCode err = ACAPI_Element_GetPropertyValue(elemGuid, definition.guid, property);
	if (err == NoError) {
		ParamDictValue pdictvalue;
		err = GetMorphParam(elemGuid, pdictvalue);
		if (err == NoError) {
			if (pdictvalue.ContainsKey(syncRule.paramName)) {
				err = WriteProp(elemGuid, property, pdictvalue.Get(syncRule.paramName));
			}
		}
	}
	return err;
}

// -----------------------------------------------------------------------------
// Запись значения IFC свойства в другое свойство
// -----------------------------------------------------------------------------
GSErrCode SyncIFCAndProp(const API_Guid & elemGuid, const SyncRule & syncRule, const API_PropertyDefinition & definition)
{
	API_IFCProperty property_from;
	API_Property property_to;
	GSErrCode err = GetIFCPropertyByName(elemGuid, syncRule.paramName, property_from);
	if (err == NoError) {
		err = ACAPI_Element_GetPropertyValue(elemGuid, definition.guid, property_to);
		if (err != NoError) {
			msg_rep("SyncParamAndProp", "ACAPI_Element_GetPropertyValue " + definition.name, err, elemGuid);
			return APIERR_MISSINGCODE;
		}
		GS::UniString param_string = "";
		GS::Int32 param_int = 0;
		bool param_bool = false;
		double param_real = 0;
		if (property_from.head.propertyType == API_IFCPropertySingleValueType) {
			switch (property_from.singleValue.nominalValue.value.primitiveType)
			{
			case API_IFCPropertyAnyValueStringType:
				param_string = property_from.singleValue.nominalValue.value.stringValue;
				param_bool = (param_string.GetLength() > 0);
				break;
			case API_IFCPropertyAnyValueRealType:
				param_real = round(property_from.singleValue.nominalValue.value.doubleValue * 1000) / 1000;
				if (property_from.singleValue.nominalValue.value.doubleValue - param_real > 0.001) param_real += 0.001;
				param_int = (GS::Int32)param_real;
				if (param_int / 1 < param_real) param_int += 1;
				param_string = GS::UniString::Printf("%.3f", param_real);
				break;
			default:
				break;
			}
		}
		if (param_real > 0) param_bool = true;

		// Реализовано только чтение, чтоб не сбивать с толку штатный транслятор
		if (syncRule.syncdirection == SYNC_FROM) {
			if (!SyncCheckIgnoreVal(syncRule, property_from)) {
				return  WriteProp(elemGuid, property_to, param_string, param_int, param_bool, param_real);
			}
			else {
				return  APIERR_MISSINGCODE; // Игнорируем значение
			}
		}
	}
	return err;
}

// -----------------------------------------------------------------------------
// Синхронизация значений свойства и параметра
// -----------------------------------------------------------------------------
GSErrCode SyncParamAndProp(const API_Guid & elemGuid_from, const API_Guid & elemGuid_to, SyncRule & syncRule, const API_PropertyDefinition & definition)
{
	GSErrCode		err = NoError;
	GS::UniString param_string = "";
	GS::Int32 param_int = 0;
	bool param_bool = false;
	double param_real = 0;
	API_Property property;
	if (syncRule.syncdirection == SYNC_FROM) {
		err = ACAPI_Element_GetPropertyValue(elemGuid_to, definition.guid, property);
		if (err != NoError) {
			msg_rep("SyncParamAndProp", "ACAPI_Element_GetPropertyValue " + definition.name, err, elemGuid_to);
			return APIERR_MISSINGCODE;
		}
		if (GetLibParam(elemGuid_from, syncRule.paramName, param_string, param_int, param_bool, param_real))
		{
			if (!SyncCheckIgnoreVal(syncRule, param_string)) {
				return WriteProp(elemGuid_to, property, param_string, param_int, param_bool, param_real);
			}
			else {
				return APIERR_MISSINGCODE; // Игнорируем значение
			}
		}
		else {
			return APIERR_MISSINGCODE; // Параметр не найден
		}
	}
	if (syncRule.syncdirection == SYNC_TO) {
		err = ACAPI_Element_GetPropertyValue(elemGuid_from, definition.guid, property);
		if (err != NoError) {
			msg_rep("SyncParamAndProp", "ACAPI_Element_GetPropertyValue " + definition.name, err, elemGuid_from);
			return APIERR_MISSINGCODE;
		}
		if (!SyncCheckIgnoreVal(syncRule, property)) {
			return WriteProp2Param(elemGuid_to, syncRule.paramName, property);
		}
		else {
			return APIERR_MISSINGCODE; // Игнорируем значение
		}
	}
	return err;
}

GS::UniString GetPropertyENGName(GS::UniString & name) {
	if (name == u8"наименование") return "BuildingMaterialProperties/Building Material Name";
	if (name == u8"описание") return "BuildingMaterialProperties/Building Material Description";
	if (name == u8"id") return "BuildingMaterialProperties/Building Material ID";
	if (name == u8"плотность") return "BuildingMaterialProperties/Building Material Density";
	if (name == u8"производитель") return "BuildingMaterialProperties/Building Material Manufacturer";
	return name;
}

// -----------------------------------------------------------------------------
// Ищем в строке - шаблоне свойства и возвращаем массив определений
// Строка шаблона на входе
//			%Имя свойства% текст %Имя группы/Имя свойства.5mm%
// Строка шаблона на выходе
//			@1@ текст @2@#.5mm#
// Если свойство не найдено, %Имя свойства% заменяем на пустоту ("")
// -----------------------------------------------------------------------------
GSErrCode  SyncPropAndMatParseString(const GS::UniString & templatestring, GS::UniString & outstring, GS::Array<API_PropertyDefinition>&outdefinitions) {
	GSErrCode					err = NoError;
	outstring = templatestring;
	outstring.ReplaceAll("Property:", "");
	outstring.ReplaceAll("{", "");
	outstring.ReplaceAll("}", "");
	GS::UniString part = "";

	//Переводим все имена свойств в нижний регистр, так потом искать проще
	for (UInt32 i = 0; i < templatestring.Count('%') / 2; i++) {
		part = outstring.GetSubstring('%', '%', 0);
		part = part.ToLowerCase();

		//Обработка количества нулей и единиц измерения
		GS::UniString formatstring = "";
		if (part.Contains(".")) {
			GS::Array<GS::UniString> partstring;
			UInt32 n = StringSplt(part, ".", partstring);
			if (StringSplt(part, ".", partstring) > 1) {
				if (partstring[n - 1].Contains("m") || partstring[n - 1].Contains(u8"м")) {
					formatstring = partstring[n - 1];
					part.ReplaceAll("." + formatstring, "");
					formatstring.ReplaceAll(u8"м", "m");
					formatstring.ReplaceAll(u8"д", "d");
					formatstring.ReplaceAll(u8"с", "c");
					formatstring = "#" + formatstring + "#";
				}
			}
		}
		outstring.ReplaceAll("%" + outstring.GetSubstring('%', '%', 0) + "%", "@" + part + "@" + formatstring);
	}
	outstring.ReplaceAll("@", "%");

	//Заменяем толщину
	outstring.ReplaceAll(u8"%толщина%", "@t@");
	outstring.ReplaceAll("%n%", "@n@");
	UInt32 n_param = 0; // Количество успешно найденных свойств
	for (UInt32 i = 0; i < templatestring.Count('%') / 2; i++) {
		part = outstring.GetSubstring('%', '%', 0);

		//Ищем свойство по названию
		API_PropertyDefinition definition = {};
		if (GetPropertyDefinitionByName(GetPropertyENGName(part), definition) == NoError) {
			outstring.ReplaceAll("%" + part + "%", "@" + GS::UniString::Printf("%d", n_param) + "@");
			outdefinitions.Push(definition);
			n_param += 1;
		}
		else {
			outstring.ReplaceAll("%" + part + "%", ""); // Если не нашли - стираем
		}
	}
	return err;
}

GSErrCode  SyncPropAndMatOneGetComponent(const API_AttributeIndex & constrinx, const double& fillThick, LayerConstr & component) {
	GSErrCode		err = NoError;
	API_Attribute	attrib = {};
	BNZeroMemory(&attrib, sizeof(API_Attribute));
	attrib.header.typeID = API_BuildingMaterialID;
	attrib.header.index = constrinx;
	err = ACAPI_Attribute_Get(&attrib);
	if (err != NoError) {
		msg_rep("SyncPropAndMatOneGetComponent", "ACAPI_Attribute_Get", err, APINULLGuid);
		return err;
	};

	// Поищем свойство со строкой-шаблоном
	GS::Array<API_PropertyDefinition> definitions;
	err = ACAPI_Attribute_GetPropertyDefinitions(attrib.header, API_PropertyDefinitionFilter_UserDefined, definitions);
	if (err != NoError) {
		msg_rep("SyncPropAndMatOneGetComponent", "ACAPI_Attribute_GetPropertyDefinitions", err, APINULLGuid);
		return err;
	}
	GS::Array <API_Property> propertys;
	if (definitions.GetSize() > 0) {
		err = ACAPI_Attribute_GetPropertyValues(attrib.header, definitions, propertys);
		if (err != NoError) {
			msg_rep("SyncPropAndMatOneGetComponent", "ACAPI_Attribute_GetPropertyDefinitions", err, APINULLGuid);
			return err;
		}
		for (UInt32 i = 0; i < propertys.GetSize(); i++) {
			if (propertys[i].definition.description.Contains("Sync_name")) {
				if (!propertys[i].isDefault) component.templatestring = propertys[i].value.singleVariant.variant.uniStringValue;
			}
		}
	}
	component.buildingMaterial = attrib;
	component.fillThick = fillThick;
	return err;
}

// --------------------------------------------------------------------
// Вытаскивает всё, что может, из информации о составе элемента
// --------------------------------------------------------------------
GSErrCode  SyncPropAndMatGetComponents(const API_Guid & elemGuid, GS::Array<LayerConstr>&components) {
	GSErrCode					err = NoError;
	API_Element					element = {};
	API_ModelElemStructureType	structtype = {};
	API_AttributeIndex			constrinx = {};
	double						fillThick = 0;
	BNZeroMemory(&element, sizeof(API_Element));
	element.header.guid = elemGuid;
	err = ACAPI_Element_Get(&element);
	if (err != NoError) {
		msg_rep("SyncPropAndMatGetComponents", "ACAPI_Element_Get", err, elemGuid);
		return err;
	}

	// Получаем данные о составе конструкции. Т.к. для разных типов элементов
	// информация храница в разных местах - запишем всё в одни переменные
	API_ElemTypeID eltype;
#ifdef AC_26
	eltype = element.header.type.typeID;
#else
	eltype = element.header.typeID;
#endif
	switch (eltype) {
	case API_WallID:
		structtype = element.wall.modelElemStructureType;
		if (structtype == API_CompositeStructure) constrinx = element.wall.composite;
		if (structtype == API_BasicStructure) constrinx = element.wall.buildingMaterial;
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
		return APIERR_MISSINGCODE;
		break;
	}

	// Получим индексы строительных материалов и толщины
	// Для однослойной конструкции
	if (structtype == API_BasicStructure) {
		LayerConstr l = {};
		if (SyncPropAndMatOneGetComponent(constrinx, fillThick, l) == NoError)
			components.Push(l);
	}

	// Для многослойной конструкции
	if (structtype == API_CompositeStructure) {
		API_Attribute						attrib;
		API_AttributeDef					defs;
		BNZeroMemory(&attrib, sizeof(API_Attribute));
		BNZeroMemory(&defs, sizeof(API_AttributeDef));
		attrib.header.typeID = API_CompWallID;
		attrib.header.index = constrinx;
		err = ACAPI_Attribute_Get(&attrib);
		if (err != NoError) {
			msg_rep("SyncPropAndMat", " ACAPI_Attribute_Get", err, elemGuid);
			return err;
		}
		err = ACAPI_Attribute_GetDef(attrib.header.typeID, attrib.header.index, &defs);
		if (err != NoError) {
			msg_rep("SyncPropAndMat", " ACAPI_Attribute_GetDef", err, elemGuid);
			return err;
		}
		for (short i = 0; i < attrib.compWall.nComps; i++) {
			LayerConstr l = {};
			if (SyncPropAndMatOneGetComponent((*defs.cwall_compItems)[i].buildingMaterial, (*defs.cwall_compItems)[i].fillThick, l) == NoError)
				components.Push(l);
		}
		ACAPI_DisposeAttrDefsHdls(&defs);
	}
	return err;
}

void SyncPropAndMatReplaceValue(const double& var, const GS::UniString & patternstring, GS::UniString & outstring) {
	GS::UniString stringformat = "";
	if (outstring.Contains(patternstring + "#")) {
		UIndex startpos = outstring.FindFirst(patternstring + "#");
		stringformat = outstring.GetSubstring('#', '#', startpos);
	}
	GS::UniString t = PropertyTestHelpers::NumToString(var, stringformat);
	if (!stringformat.IsEmpty()) {
		stringformat = "#" + stringformat + "#";
	}
	outstring.ReplaceAll(patternstring + stringformat, t);
}

void SyncPropAndMatReplaceValue(const API_Property & property, const GS::UniString & patternstring, GS::UniString & outstring) {
	GS::UniString stringformat = "";
	if (outstring.Contains(patternstring + "#")) {
		UIndex startpos = outstring.FindFirst(patternstring + "#");
		stringformat = outstring.GetSubstring('#', '#', startpos);
	}
	GS::UniString t = PropertyTestHelpers::ToString(property, stringformat);
	if (!stringformat.IsEmpty()) {
		stringformat = "#" + stringformat + "#";
	}
	outstring.ReplaceAll(patternstring + stringformat, t);
}

// --------------------------------------------------------------------
// Заменяем в строке templatestring все вхождения @1@...@n@ на значения свойств
// --------------------------------------------------------------------
GSErrCode  SyncPropAndMatWriteOneString(const API_Attribute & attrib, const double& fillThick, const GS::Array<API_PropertyDefinition>&outdefinitions, const GS::UniString & templatestring, GS::UniString & outstring, UInt32 & n) {
	GSErrCode					err = NoError;
	GS::Array <API_Property>	propertys;
	outstring = templatestring;
	SyncPropAndMatReplaceValue(fillThick, "@t@", outstring);
	SyncPropAndMatReplaceValue(n + 1, "@n@", outstring);
	if (ACAPI_Attribute_GetPropertyValues(attrib.header, outdefinitions, propertys) == NoError) {
		for (UInt32 j = 0; j < propertys.GetSize(); j++) {
			GS::UniString stringformat = "";
			GS::UniString patternstring = "@" + GS::UniString::Printf("%d", j) + "@";
			SyncPropAndMatReplaceValue(propertys[j], patternstring, outstring);
		}
	}
	outstring.TrimLeft();

	//Ищем указание длины строки
	Int32 stringlen = 0;
	GS::UniString part = "";
	if (outstring.Contains('~')) {
		part = outstring.GetSubstring('~', ' ', 0);
		if (!part.IsEmpty() && part.GetLength() < 4)
			stringlen = std::atoi(part.ToCStr());
		if (stringlen > 0) part = "~" + part;
	}
	if (stringlen > 0) {
		Int32 modlen = outstring.GetLength() - part.GetLength() - 1;
		Int32 addspace = stringlen - modlen;
		if (modlen > stringlen) {
			addspace = modlen % stringlen;
		}
		outstring.ReplaceAll(part + " ", GS::UniString::Printf("%*s", addspace, " "));
	}
	return err;
}

// -----------------------------------------------------------------------------
// Запись в свойство данных о материале
// -----------------------------------------------------------------------------
GSErrCode SyncPropAndMat(const API_Guid & elemGuid, const API_ElemTypeID elementType, const SyncRule syncRule, const API_PropertyDefinition & definition) {
	if (elementType != API_WallID && elementType != API_SlabID && elementType != API_RoofID && elementType != API_ShellID) return APIERR_MISSINGCODE;
	GSErrCode err = NoError;
	API_Property property;
	err = ACAPI_Element_GetPropertyValue(elemGuid, definition.guid, property);
	if (err != NoError) {
		msg_rep("SyncPropAndMat", "ACAPI_Element_GetPropertyValue " + definition.name, err, elemGuid);
		return err;
	}
	GS::Array<LayerConstr>	components;
	err = SyncPropAndMatGetComponents(elemGuid, components);
	if (err != NoError) return err;

	// Разбираем основную строку-шаблон
	if (err != NoError) return err;
	GS::UniString outstring = "";
	GS::Array<API_PropertyDefinition> outdefinitions;
	err = SyncPropAndMatParseString(syncRule.templatestring, outstring, outdefinitions);
	GS::UniString param_string = "";

	// Финишная прямая, идём по компонентам и подставляем значения в строки
	for (UInt32 i = 0; i < components.GetSize(); i++) {

		// Стандартная строка
		GS::UniString one_string = "";
		if (components[i].templatestring.IsEmpty()) {
			if (SyncPropAndMatWriteOneString(components[i].buildingMaterial, components[i].fillThick, outdefinitions, outstring, one_string, i) == NoError)
				param_string = param_string + " " + one_string;
		}

		// Индивидуальная строка
		else {
			GS::UniString uqoutstring = "";
			GS::Array<API_PropertyDefinition> uqoutdefinitions;
			if (SyncPropAndMatParseString(components[i].templatestring, uqoutstring, uqoutdefinitions) == NoError) {
				if (SyncPropAndMatWriteOneString(components[i].buildingMaterial, components[i].fillThick, uqoutdefinitions, uqoutstring, one_string, i) == NoError) {
					param_string = param_string + " " + one_string;
				}
			}
		}
	}
	if (!param_string.IsEmpty())
		param_string.TrimLeft();
	err = WriteProp(elemGuid, property, param_string);
	return err;
}

// -----------------------------------------------------------------------------
// Проверяем - содержит ли строка игнорируемые значения
// -----------------------------------------------------------------------------
bool SyncCheckIgnoreVal(const SyncRule & syncRule, const GS::UniString & val) {
	bool ignore_flag = CheckIgnoreVal(syncRule.ignorevals, val);
	return ignore_flag;
}

// -----------------------------------------------------------------------------
// Проверяем - содержит ли свойство игнорируемые значеения
// -----------------------------------------------------------------------------
bool SyncCheckIgnoreVal(const SyncRule & syncRule, const API_Property & property) {
	bool ignore_flag = false;
	if (syncRule.ignorevals.GetSize() > 0) {
		GS::UniString val = PropertyTestHelpers::ToString(property);
		ignore_flag = SyncCheckIgnoreVal(syncRule, val);
	}
	return ignore_flag;
}

// -----------------------------------------------------------------------------
// Проверяем - содержит ли свойство игнорируемые значеения
// -----------------------------------------------------------------------------
bool SyncCheckIgnoreVal(const SyncRule & syncRule, const API_IFCProperty & property) {
	bool ignore_flag = false;
	(void)property;
	(void)syncRule;
	// TODO добавить игнорируемые значения для IFC
	//if (syncRule.ignorevals.GetSize() > 0) {
	//	GS::UniString val = PropertyTestHelpers::ToString(property);
	//	ignore_flag = SyncCheckIgnoreVal(syncRule, val);
	//}
	return ignore_flag;
}