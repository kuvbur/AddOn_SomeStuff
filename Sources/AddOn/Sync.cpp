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
		if (ACAPI_Goodies(APIAny_GetElemTypeNameID, (void*)elementType, &subtitle) == NoError) {
			nLib += 1;
			ACAPI_Interface(APIIo_SetNextProcessPhaseID, &subtitle, &nLib);
			GS::UniString intString = GS::UniString::Printf(" %d", guidArray.GetSize());
			msg_rep("SyncByType", subtitle + intString, NoError, APINULLGuid);
		}
		for (UInt32 i = 0; i < guidArray.GetSize(); i++) {
			if (syncSettings.syncAll) {

				// Если попадается навесная стена - обработаем также входящие в неё панели
				if (elementType == API_CurtainWallID && syncSettings.cwallS) {
					GS::Array<API_Guid> panelGuid;
					err = GetCWElementsForCWall(guidArray[i], panelGuid);
					if (err == NoError) {
						for (UInt32 i = 0; i < panelGuid.GetSize(); ++i) {
							SyncData(panelGuid[i], syncSettings);
						}
					}
				}
				SyncData(guidArray[i], syncSettings);
			}
			if (syncSettings.syncMon) {
				err = AttachObserver(guidArray[i], syncSettings);
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

// -----------------------------------------------------------------------------
// Запускает обработку выбранных, заданных в настройке
// -----------------------------------------------------------------------------
void SyncSelected(const SyncSettings& syncSettings) {
	GS::UniString undoString = RSGetIndString(AddOnStringsID, UndoSyncId, ACAPI_GetOwnResModule());
	ACAPI_CallUndoableCommand(undoString, [&]() -> GSErrCode {
		CallOnSelectedElemSettings(SyncData, false, true, syncSettings);
		return NoError;
		});
}

// --------------------------------------------------------------------
// Синхронизация привязанных к двери зон
// --------------------------------------------------------------------
GSErrCode SyncRelationsToWindow(const API_Guid& elemGuid, const SyncSettings& syncSettings) {
	GSErrCode			err = NoError;

	//Обновляем объекты, если их обработка включена
	API_WindowRelation            relData;
	err = ACAPI_Element_GetRelations(elemGuid, API_ZoneID, &relData);
	if (err == NoError) {
		if (relData.fromRoom != APINULLGuid) SyncData(relData.fromRoom, syncSettings);
		if (relData.toRoom != APINULLGuid) SyncData(relData.toRoom, syncSettings);
	}
	return err;
}

// --------------------------------------------------------------------
// Синхронизация привязанных к окну зон
// --------------------------------------------------------------------
GSErrCode SyncRelationsToDoor(const API_Guid& elemGuid, const SyncSettings& syncSettings) {
	GSErrCode			err = NoError;

	//Обновляем объекты, если их обработка включена
	API_DoorRelation            relData;
	err = ACAPI_Element_GetRelations(elemGuid, API_ZoneID, &relData);
	if (err == NoError) {
		if (relData.fromRoom != APINULLGuid) SyncData(relData.fromRoom, syncSettings);
		if (relData.toRoom != APINULLGuid) SyncData(relData.toRoom, syncSettings);
	}
	return err;
}

// --------------------------------------------------------------------
// Синхронизация привязанных навесной стене элементов
// --------------------------------------------------------------------
GSErrCode SyncRelationsToCWall(const API_Guid& elemGuid, const SyncSettings& syncSettings) {
	GSErrCode			err = NoError;
	GS::Array<API_Guid> panelGuid;
	err = GetCWElementsForCWall(elemGuid, panelGuid);
	if (err == NoError) {
		for (UInt32 i = 0; i < panelGuid.GetSize(); ++i) {
			SyncData(panelGuid[i], syncSettings);
		}
	}
	return err;
}

// --------------------------------------------------------------------
// Поиск и синхронизация свойств связанных элементов
// --------------------------------------------------------------------
void SyncRelationsElement(const API_Guid& elemGuid, const SyncSettings& syncSettings) {
	GSErrCode	err = NoError;
	API_ElemTypeID elementType;
	err = GetTypeByGUID(elemGuid, elementType);
	switch (elementType) {
	case API_WindowID:
		if (syncSettings.objS) err = SyncRelationsToWindow(elemGuid, syncSettings);
		break;
	case API_DoorID:
		if (syncSettings.objS) err = SyncRelationsToDoor(elemGuid, syncSettings);
		break;
	case API_CurtainWallID:
		if (syncSettings.cwallS) err = SyncRelationsToCWall(elemGuid, syncSettings);
		break;
	default:
		break;
	}
}

// --------------------------------------------------------------------
// Поиск и синхронизация свойств связанных элементов
// --------------------------------------------------------------------
void SyncGetRelationsElement(const API_Guid& elemGuid, GS::Array<API_Guid>& subelemGuid) {
	GSErrCode	err = NoError;
	API_ElemTypeID elementType;
	err = GetTypeByGUID(elemGuid, elementType);
	switch (elementType) {
	case API_CurtainWallID:
		err = GetCWElementsForCWall(elemGuid, subelemGuid);
		break;
	case API_CurtainWallPanelID:
		API_Elem_Head elementHead;
		BNZeroMemory(&elementHead, sizeof(API_Elem_Head));
		elementHead.guid = elemGuid;
		err = ACAPI_Element_GetHeader(&elementHead);
		break;
	default:
		break;
	}
}

// --------------------------------------------------------------------
// Синхронизация данных элемента согласно указаниям в описании свойств
// --------------------------------------------------------------------
void SyncData(const API_Guid& elemGuid, const SyncSettings& syncSettings) {
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
		for (UInt32 i = 0; i < syncRules.GetSize(); i++) {
			if (SyncOneRule(elemGuid, elementType, definition, syncRules[i])) break; // Если синхронизация успешная - выходим из цикла
		}
	}
	return err;
}

// --------------------------------------------------------------------
// Синхронизация одного правила для свойства
// Если синхронизация успешна, возвращает True
// Если свойство или параметр не найдены, либо содержат игнорируемые символы - возращает False
// --------------------------------------------------------------------
bool SyncOneRule(const API_Guid& elemGuid, const API_ElemTypeID& elementType, const API_PropertyDefinition& definition, SyncRule syncRule) {
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
bool SyncString(GS::UniString& description_string, GS::Array <SyncRule>& syncRules) {
	if (description_string.IsEmpty()) {
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
			// Копировать параметр в свойство или свойство в параметр
			if (rule.syncdirection == SYNC_NO && rulestring_one.Contains("from")) {
				rule.syncdirection = SYNC_FROM;
			}
			if (rule.syncdirection == SYNC_NO && rulestring_one.Contains("to")) {
				rule.syncdirection = SYNC_TO;
			}

			// Копировать в субэлементы или из субэлементов
			if (rule.syncdirection == SYNC_TO && rulestring_one.Contains("sub")) {
				rule.syncdirection = SYNC_TO_SUB;
			}
			if (rule.syncdirection == SYNC_FROM && rulestring_one.Contains("sub")) {
				rule.syncdirection = SYNC_FROM_SUB;
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
//Ищет свойство со значение "Sync_flag" в описании и по значению определяет - нужно ли синхронизировать параметры элемента
//--------------------------------------------------------------------------------------------------------------------------
bool SyncState(const API_Guid& elemGuid, const GS::Array<API_PropertyDefinition> definitions) {
	GSErrCode	err = NoError;

	// Проверяем - не отключена ли синхронизация у данного объекта
	for (UInt32 i = 0; i < definitions.GetSize(); i++) {
		if (definitions[i].description.Contains("Sync_flag")) {
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
GSErrCode SyncPropAndProp(const API_Guid& elemGuid_from, const API_Guid& elemGuid_to, const SyncRule& syncRule, const API_PropertyDefinition& definition)
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
GSErrCode SyncMorphAndProp(const API_Guid& elemGuid, const SyncRule& syncRule, const API_PropertyDefinition& definition) {
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
GSErrCode SyncIFCAndProp(const API_Guid& elemGuid, const SyncRule& syncRule, const API_PropertyDefinition& definition)
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
GSErrCode SyncParamAndProp(const API_Guid& elemGuid_from, const API_Guid& elemGuid_to, SyncRule& syncRule, const API_PropertyDefinition& definition)
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

GS::UniString GetPropertyENGName(GS::UniString& name) {
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
GSErrCode  SyncPropAndMatParseString(const GS::UniString& templatestring, GS::UniString& outstring, GS::Array<API_PropertyDefinition>& outdefinitions) {
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

GSErrCode  SyncPropAndMatOneGetComponent(const API_AttributeIndex& constrinx, const double& fillThick, LayerConstr& component) {
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
GSErrCode  SyncPropAndMatGetComponents(const API_Guid& elemGuid, GS::Array<LayerConstr>& components) {
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
	eltype = elem_head.typeID;
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

void SyncPropAndMatReplaceValue(const double& var, const GS::UniString& patternstring, GS::UniString& outstring) {
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

void SyncPropAndMatReplaceValue(const API_Property& property, const GS::UniString& patternstring, GS::UniString& outstring) {
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
GSErrCode  SyncPropAndMatWriteOneString(const API_Attribute& attrib, const double& fillThick, const GS::Array<API_PropertyDefinition>& outdefinitions, const GS::UniString& templatestring, GS::UniString& outstring, UInt32& n) {
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
GSErrCode SyncPropAndMat(const API_Guid& elemGuid, const API_ElemTypeID elementType, const SyncRule syncRule, const API_PropertyDefinition& definition) {
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
bool SyncCheckIgnoreVal(const SyncRule& syncRule, const GS::UniString& val) {
	bool ignore_flag = CheckIgnoreVal(syncRule.ignorevals, val);
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
// Проверяем - содержит ли свойство игнорируемые значеения
// -----------------------------------------------------------------------------
bool SyncCheckIgnoreVal(const SyncRule& syncRule, const API_IFCProperty& property) {
	bool ignore_flag = false;

	// TODO добавить игнорируемые значения для IFC
	//if (syncRule.ignorevals.GetSize() > 0) {
	//	GS::UniString val = PropertyTestHelpers::ToString(property);
	//	ignore_flag = SyncCheckIgnoreVal(syncRule, val);
	//}
	return ignore_flag;
}