#include	"APIEnvir.h"
#include	"ACAPinc.h"
#include	"Sync.hpp"
#include	"Helpers.hpp"

const int ERRIGNOREVAL = 10;
Int32 nLib = 0;

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
		if (!flag_chanel && prefsData.objS) flag_chanel = SyncByType(API_CurtainWallID);
		return NoError;
		});
}

// -----------------------------------------------------------------------------
// Запускает обработку всех элементов заданного типа
// -----------------------------------------------------------------------------
bool SyncByType(const API_ElemTypeID& elementType) {
	SyncPrefs			prefsData;
	GS::UniString		subtitle;
	GSErrCode			err = NoError;
	GS::Array<API_Guid>	guidArray;
	bool flag_chanel = false;
	ACAPI_Element_GetElemList(elementType, &guidArray, APIFilt_IsEditable);
	if (!guidArray.IsEmpty()) {
		if (ACAPI_Goodies(APIAny_GetElemTypeNameID, (void*)elementType, &subtitle) == NoError) {
			nLib += 1;
			ACAPI_Interface(APIIo_SetNextProcessPhaseID, &subtitle, &nLib);
			GS::UniString intString = GS::UniString::Printf(" %d", guidArray.GetSize());
			msg_rep("SyncByType", subtitle + intString, NoError, APINULLGuid);
		}
		SyncSettingsGet(prefsData);
		for (UInt32 i = 0; i < guidArray.GetSize(); i++) {
			if (prefsData.syncAll) {
				if (elementType == API_CurtainWallID) {
					GS::Array<API_Guid> panelGuid;
					err = GetCWPanelsForCWall(guidArray[i], panelGuid);
					if (err == NoError) {
						for (UInt32 i = 0; i < panelGuid.GetSize(); ++i) {
							SyncData(panelGuid[i]);
						}
					}
				}
				SyncData(guidArray[i]);
			}
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

// -----------------------------------------------------------------------------
// Запускает обработку выбранных, заданных в настройке
// -----------------------------------------------------------------------------
void SyncSelected(void) {
	GS::UniString undoString = RSGetIndString(AddOnStringsID, UndoSyncId, ACAPI_GetOwnResModule());
	ACAPI_CallUndoableCommand(undoString, [&]() -> GSErrCode {
		CallOnSelectedElem(SyncData);
		return NoError;
		});
}

// --------------------------------------------------------------------
// Синхронизация привязанных к двери зон
// --------------------------------------------------------------------
GSErrCode SyncRelationsToWindow(const API_Guid& elemGuid) {
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

// --------------------------------------------------------------------
// Синхронизация привязанных к окну зон
// --------------------------------------------------------------------
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
// Синхронизация привязанных навесной стене панелей
// --------------------------------------------------------------------
GSErrCode SyncRelationsToCWall(const API_Guid& elemGuid) {
	GSErrCode			err = NoError;
	GS::Array<API_Guid> panelGuid;
	err = GetCWPanelsForCWall(elemGuid, panelGuid);
	if (err == NoError) {
		for (UInt32 i = 0; i < panelGuid.GetSize(); ++i) {
			SyncData(panelGuid[i]);
		}
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
	case API_CurtainWallID:
		if (prefsData.objS) err = SyncRelationsToCWall(elemGuid);
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
	if (!IsElementEditable(elemGuid))
		return;
	API_ElemTypeID elementType;
	err = GetTypeByGUID(elemGuid, elementType);
	if (err != NoError) {
		msg_rep("SyncData", "GetTypeByGUID", err, elemGuid);
		return;
	}
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
		if (elementType == API_ObjectID ||
			elementType == API_WindowID ||
			elementType == API_DoorID ||
			elementType == API_ZoneID ||
			elementType == API_LampID ||
			elementType == API_CurtainWallPanelID ||
			syncRule.paramName.ToLowerCase() == "id") {
			err = SyncParamAndProp(elemGuid, syncRule, property); //Синхронизация свойства и параметра
		}
		break;
	case 2:
		err = SyncPropAndProp(elemGuid, syncRule, property); //Синхронизация свойств
		break;
	case 3:
		err = SyncPropAndMat(elemGuid, elementType, syncRule, property); //Синхронизация свойств и данных о конструкции
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
			if (rulestring_one.Contains("Material:") && rulestring_one.Contains('"')) {
				rule.synctype = 3;
				rulestring_one.ReplaceAll("Material:", "");
			}
			GS::UniString paramName = rulestring_one.GetSubstring('{', '}',0);
			// Для материалов вытащим строку с шаблоном, чтоб ненароком не разбить её
			if (rule.synctype == 3) {
				GS::UniString templatestring = rulestring_one.GetSubstring('"', '"', 0);
				rule.templatestring = templatestring;
				rulestring_one.ReplaceAll(templatestring, "");
			}
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
			if (err != NoError) msg_rep("SyncState", "ACAPI_Element_GetPropertyValue " + definitions[i].name, err, elemGuid);
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

// -----------------------------------------------------------------------------
// Запись значения свойства в другое свойство
// -----------------------------------------------------------------------------
GSErrCode SyncPropAndProp(const API_Guid& elemGuid, const SyncRule& syncRule, API_Property& property)
{
	GSErrCode	err = NoError;
	API_Property propertyfrom;
	err = GetPropertyByName(elemGuid, syncRule.paramName, propertyfrom);
	if (err == NoError) {
		if (syncRule.syncdirection == 1) {
			if (!SyncCheckIgnoreVal(syncRule, propertyfrom)) {
				err = WriteProp2Prop(elemGuid, propertyfrom, property);
			}
			else {
				err = APIERR_MISSINGCODE; // Игнорируем значение
			}
		}
		if (syncRule.syncdirection == 2) {
			if (!SyncCheckIgnoreVal(syncRule, property)) {
				err = WriteProp2Prop(elemGuid, property, propertyfrom);
			}
			else {
				err = APIERR_MISSINGCODE; // Игнорируем значение
			}
		}
	}
	return err;
}

GSErrCode WriteProp2Param(const API_Guid& elemGuid, GS::UniString paramName, API_Property& property) {
	GSErrCode		err = NoError;
	if (paramName.ToLowerCase() == "id") {
		GS::UniString val = PropertyTestHelpers::ToString(property);
		err = ACAPI_Database(APIDb_ChangeElementInfoStringID, (void*)&elemGuid, (void*)&val);
		if (err != NoError) msg_rep("WriteProp2Param - ID", "ACAPI_Database(APIDb_ChangeElementInfoStringID", err, elemGuid);
		return err;
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
	if (syncRule.syncdirection == 2) {
		if (!SyncCheckIgnoreVal(syncRule, property)) {
			err = WriteProp2Param(elemGuid, syncRule.paramName, property);
		}
		else {
			err = APIERR_MISSINGCODE; // Игнорируем значение
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
//			%Имя свойства% текст %Имя группы/Имя свойства%
// Строка шаблона на выходе
//			%1% текст %2%
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
		outstring.ReplaceAll("%" + part + "%", "@" + part.ToLowerCase() + "@");
	}
	outstring.ReplaceAll("@", "%");
	//Заменяем толщину
	outstring.ReplaceAll(u8"%толщина%", "@t@");
	UInt32 n_param = 0; // Количество успешно найденных свойств
	for (UInt32 i = 0; i < templatestring.Count('%') / 2; i++) {
		part = outstring.GetSubstring('%', '%', 0);
		//Ищем свойство по названию
		API_PropertyDefinition definition = {};
		if (GetPropertyDefinitionByName(GetPropertyENGName(part), definition) == NoError){
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
	component.fillThick = DoubleM2IntMM(fillThick);
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
	// Получаем данные о состапве конструкции. Т.к. для разных типов элементов
	// информация храница в разхных местах - запишем всё в одни переменные
	switch (element.header.typeID) {
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

// --------------------------------------------------------------------
// Заменяем в строке templatestring все вхождения @1@...@n@ на значения свойств
// --------------------------------------------------------------------
GSErrCode  SyncPropAndMatWriteOneString(const API_Attribute& attrib, const UInt32 fillThick, const GS::Array<API_PropertyDefinition>& outdefinitions, const GS::UniString& templatestring, GS::UniString& outstring) {
	GSErrCode					err = NoError;
	GS::Array <API_Property>	propertys;
	outstring = templatestring;
	outstring.ReplaceAll("@t@", GS::UniString::Printf("%d", fillThick));
	if (ACAPI_Attribute_GetPropertyValues(attrib.header, outdefinitions, propertys) == NoError) {
		for (UInt32 j = 0; j < propertys.GetSize(); j++) {
			GS::UniString stringformat = "";
			GS::UniString patternstring = "@" + GS::UniString::Printf("%d", j) + "@";
			//if (outstring.Contains(patternstring + ".")) {
			//	UIndex startpos = outstring.FindFirst(patternstring + ".")-1;
			//	UIndex endpos = min(outstring.FindFirst(" ", startpos)+1, outstring.GetLength());
			//	stringformat = outstring.GetSubstring(startpos, endpos);
			//}
			GS::UniString t = PropertyTestHelpers::ToString(propertys[j]);
			outstring.ReplaceAll(patternstring+stringformat, t);
		}
	}
	outstring.Trim();
	return err;
}

// -----------------------------------------------------------------------------
// Синхронизация значений свойства и параметра
// -----------------------------------------------------------------------------
GSErrCode SyncPropAndMat(const API_Guid& elemGuid, const API_ElemTypeID elementType, const SyncRule syncRule, API_Property property) {
	if (elementType != API_WallID && elementType != API_SlabID && elementType != API_RoofID && elementType != API_ShellID) return APIERR_MISSINGCODE;
	GSErrCode					err = NoError;
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
			if (SyncPropAndMatWriteOneString(components[i].buildingMaterial, components[i].fillThick, outdefinitions, outstring, one_string) == NoError)
				param_string = param_string + " " + one_string;
		}
		// Индивидуальная строка
		else {
			GS::UniString uqoutstring = "";
			GS::Array<API_PropertyDefinition> uqoutdefinitions;
			if (SyncPropAndMatParseString(components[i].templatestring, uqoutstring, uqoutdefinitions) == NoError) {
				if (SyncPropAndMatWriteOneString(components[i].buildingMaterial, components[i].fillThick, uqoutdefinitions, uqoutstring, one_string) == NoError)
					param_string = param_string + " " + one_string;
			}
		}
	}
	if (!param_string.IsEmpty())
		param_string.Trim();
		err = WriteProp(elemGuid, property, param_string);
	return err;
}

// --------------------------------------------------------------------
// Проверяет - попадает ли тип элемента в под настройки синхронизации
// --------------------------------------------------------------------
bool CheckElementType(const API_ElemTypeID& elementType) {
	SyncPrefs prefsData;
	SyncSettingsGet(prefsData);
	bool flag_type = false;
	if ((elementType == API_WallID || elementType == API_ColumnID || elementType == API_BeamID || elementType == API_SlabID ||
		elementType == API_RoofID || elementType == API_MeshID || elementType == API_ZoneID  ||
		elementType == API_CurtainWallSegmentID || elementType == API_CurtainWallFrameID ||
		elementType == API_CurtainWallJunctionID || elementType == API_CurtainWallAccessoryID || elementType == API_ShellID ||
		elementType == API_MorphID || elementType == API_StairID || elementType == API_RiserID ||
		elementType == API_TreadID || elementType == API_StairStructureID ||
		elementType == API_RailingID || elementType == API_RailingToprailID || elementType == API_RailingHandrailID ||
		elementType == API_RailingRailID || elementType == API_RailingPostID || elementType == API_RailingInnerPostID ||
		elementType == API_RailingBalusterID || elementType == API_RailingPanelID || elementType == API_RailingSegmentID ||
		elementType == API_RailingNodeID || elementType == API_RailingBalusterSetID || elementType == API_RailingPatternID ||
		elementType == API_RailingToprailEndID || elementType == API_RailingHandrailEndID ||
		elementType == API_RailingRailEndID ||
		elementType == API_RailingToprailConnectionID ||
		elementType == API_RailingHandrailConnectionID ||
		elementType == API_RailingRailConnectionID ||
		elementType == API_RailingEndFinishID ||
		elementType == API_BeamSegmentID ||
		elementType == API_ColumnSegmentID ||
		elementType == API_OpeningID) && prefsData.wallS) flag_type = true;
	if ((elementType == API_ObjectID ||
		elementType == API_ZoneID ||
		elementType == API_LampID ||
		elementType == API_CurtainWallID ||
		elementType == API_CurtainWallPanelID) && prefsData.objS) flag_type = true;
	if ((elementType == API_WindowID || elementType == API_DoorID || elementType == API_SkylightID) && prefsData.widoS) flag_type = true;
	return flag_type;
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