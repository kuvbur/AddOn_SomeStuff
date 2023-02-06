//------------ kuvbur 2022 ------------
#include	<stdlib.h> /* atoi */
#include <time.h>
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
		long time_start = clock();
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
		long time_end = clock();
		GS::UniString time = GS::UniString::Printf(" %d ms", time_end - time_start);
		msg_rep("SyncAll", time, NoError, APINULLGuid);
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
	long time_start = clock();
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
		}
		for (UInt32 i = 0; i < guidArray.GetSize(); i++) {
			SyncElement(guidArray[i], syncSettings);
			ACAPI_Interface(APIIo_SetProcessValueID, &i, nullptr);
			if (ACAPI_Interface(APIIo_IsProcessCanceledID, nullptr, nullptr)) {
				flag_chanel = true;
				msg_rep("SyncByType", subtitle + u8" - отмена", NoError, APINULLGuid);
				return flag_chanel;
			}
		}
		GS::UniString intString = GS::UniString::Printf(" %d qty", guidArray.GetSize());
		long time_end = clock();
		GS::UniString time = GS::UniString::Printf(" %d ms", time_end - time_start);
		msg_rep("SyncByType", subtitle + intString + time, NoError, APINULLGuid);
	}
	return flag_chanel;
}

void SyncElement(const API_Guid & objectId, const SyncSettings & syncSettings) {
	GSErrCode		err = NoError;
	if (err == NoError) {
		SyncData(objectId, syncSettings);
		//SyncRelationsElement(objectId, syncSettings);
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
	if (flag_sync) {
		GS::Array<API_Guid> subelemGuid;
		GetRelationsElement(elemGuid, elementType, syncSettings, subelemGuid);
		if (subelemGuid.GetSize() > 0) {
			for (UInt32 i = 0; i < subelemGuid.GetSize(); ++i) {
				SyncData(subelemGuid[i], syncSettings);
			}
		}
	}
}


// --------------------------------------------------------------------
// Синхронизация данных элемента согласно указаниям в описании свойств
// --------------------------------------------------------------------
void SyncData(const API_Guid & elemGuid, const SyncSettings & syncSettings) {
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
	if (GetElemState(elemGuid, definitions, "Sync_flag")) { // Проверяем - не отключена ли синхронизация у данного объекта
		GS::Array <WriteData> mainsyncRules;
		bool hasSub = false;
		for (UInt32 i = 0; i < definitions.GetSize(); i++) {
			// Получаем список правил синхронизации из всех свойств
			ParseSyncString(elemGuid, definitions[i], mainsyncRules, hasSub); // Парсим описание свойства
		}
		if (mainsyncRules.IsEmpty()) return;
		// Получаем список связанных элементов
		GS::Array<API_Guid> subelemGuids;
		if (hasSub) {
			GetRelationsElement(elemGuid, syncSettings, subelemGuids);
		}
		// Заполняем правила синхронизации с учётом субэлементов, попутно заполняем словарь параметров для чтения/записи
		WriteDict syncRules; // Словарь с правилами для каждого элемента
		ParamDictElement paramToRead; // Словарь с параметрами для чтения
		SyncAddSubelement(subelemGuids, mainsyncRules, syncRules, paramToRead);
		mainsyncRules.Clear();
		subelemGuids.Push(elemGuid); // Это теперь список всех элементов для синхронизации

		// Читаем все возможные свойства
		ParamDictElementRead(paramToRead);
		ParamDictElement paramToWrite; // Словарь с параметрами для записи
		// Выбираем по-элементно параметры для чтения и записи, формируем словарь
		for (UInt32 i = 0; i < subelemGuids.GetSize(); i++) {
			API_Guid elemGuid = subelemGuids[i];
			GS::Array <WriteData> writeSubs;
			if (syncRules.ContainsKey(elemGuid)) writeSubs = syncRules.Get(elemGuid);
			if (!writeSubs.IsEmpty()) {
				// Заполняем значения параметров чтения/записи из словаря
				for (UInt32 j = 0; j < writeSubs.GetSize(); j++) {
					WriteData writeSub = writeSubs.Get(i);
					API_Guid elemGuid = writeSub.paramFrom.fromGuid;
					GS::UniString rawname = writeSub.paramFrom.rawName;
					bool flagfindFrom = false;
					if (paramToRead.ContainsKey(elemGuid)) {
						if(paramToRead.Get(elemGuid).ContainsKey(rawname)) {
							ParamValue param = paramToRead.Get(elemGuid).Get(rawname);
							if (param.isValid) { // Записываем только корректные значения
								writeSubs[i].paramFrom = param;
								flagfindFrom = true;
							}
						}
					}
					if (flagfindFrom) {
						elemGuid = writeSub.paramTo.fromGuid;
						rawname = writeSub.paramTo.rawName;
						if (paramToRead.ContainsKey(elemGuid)) {
							if (paramToRead.Get(elemGuid).ContainsKey(rawname)) {
								writeSubs[i].paramTo = paramToRead.Get(elemGuid).Get(rawname);
							}
						}
					}
					// Если оба свойства были найдены - сверим их значения и установим флаг записи
					if (flagfindFrom){
						if (writeSubs[i].paramTo != writeSubs[i].paramFrom) {
							ParamValue param = writeSubs[i].paramTo;
							param.uniStringValue = writeSubs[i].paramFrom.uniStringValue;
							param.intValue = writeSubs[i].paramFrom.intValue;
							param.boolValue = writeSubs[i].paramFrom.boolValue;
							param.doubleValue = writeSubs[i].paramFrom.doubleValue;
							SyncAddParam(param, paramToWrite);
						}
					}
				}
			}
		}
		if (!paramToWrite.IsEmpty()) {
			ParamDictElementWrite(paramToWrite);
		}
	}
}


void SyncAddSubelement(const GS::Array<API_Guid>& subelemGuids, const GS::Array <WriteData>& mainsyncRules, WriteDict& syncRules, ParamDictElement& paramToRead) {
	for (UInt32 i = 0; i < mainsyncRules.GetSize(); i++) {
		if (!mainsyncRules[i].fromSub && !mainsyncRules[i].toSub) {
			WriteData writeSub = mainsyncRules.Get(i);
			SyncAddRule(writeSub, syncRules, paramToRead);
		}
		if (mainsyncRules[i].fromSub) {
			for (UInt32 j = 0; j < subelemGuids.GetSize(); j++) {
				WriteData writeSub = mainsyncRules.Get(i);
				API_Guid subelemGuid = subelemGuids.Get(j);
				writeSub.fromSub = false;
				writeSub.guidFrom = subelemGuid;
				writeSub.paramFrom.fromGuid = subelemGuid;
				SyncAddRule(writeSub, syncRules, paramToRead);
			}
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

// --------------------------------------------------------------------
// Запись правила в словарь
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
	SyncAddParam(writeSub.paramFrom, paramToRead);
	SyncAddParam(writeSub.paramTo, paramToRead);
}

// --------------------------------------------------------------------
// Запись параметра в словарь
// --------------------------------------------------------------------
void SyncAddParam(const ParamValue& param, ParamDictElement& paramToRead) {
	API_Guid elemGuid = param.fromGuid;
	GS::UniString rawName = param.rawName;
	if (paramToRead.ContainsKey(elemGuid)) {
		if (!paramToRead.Get(elemGuid).ContainsKey(rawName)) {
			paramToRead.Get(elemGuid).Add(rawName, param);
		}
	}
	else {
		ParamDictValue params;
		params.Add(rawName, param);
		paramToRead.Add(elemGuid, params);
	}
}

// -----------------------------------------------------------------------------
// Парсит описание свойства
// -----------------------------------------------------------------------------
bool ParseSyncString(const API_Guid& elemGuid, const API_PropertyDefinition& definition, GS::Array <WriteData>& syncRules, bool& hasSub) {
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
			if (SyncString(rulestring[i], syncdirection, param, ignorevals)) {
				hasRule = true;
				ParamValue paramdef; //Свойство, из которого получено правило
				ConvParamValue(paramdef, definition);
				paramdef.fromGuid = elemGuid;
				WriteData writeOne;
				writeOne.ignorevals = ignorevals;
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
bool SyncString(GS::UniString rulestring_one, int& syncdirection, ParamValue& param, GS::Array<GS::UniString> ignorevals) {
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
	//Выбор типа копируемого свойства
	bool synctypefind = false;

	//GS::Array<GS::UniString> paramTypesList;
	//GetParamTypeList(paramTypesList);
	//Я не очень понял - умеет ли с++ в ленивые вычисления, поэтому сделаю вложенные условия, чтобы избежать ненужного поиска по строке
	//TODO переписать всё это с использованием GetParamTypeList
	if (synctypefind == false) {
		if (!rulestring_one.Contains(":") || rulestring_one.Contains("description:")) {
			if (rulestring_one.Contains("description:")) {
				param.fromGDLdescription = true;
				rulestring_one.ReplaceAll("description:", "");
			}
			paramNamePrefix = "{GDL:";
			param.fromGDLparam = true;
			synctypefind = true;
		}
	}
	if (synctypefind == false) {
		if (rulestring_one.Contains("Property:")) {
			synctypefind = true;
			rulestring_one.ReplaceAll("Property:", "");
			paramNamePrefix = "{Property:";
			param.fromProperty = true;
		}
	}
	if (synctypefind == false) {
		if (rulestring_one.Contains("Material:") && rulestring_one.Contains('"')) {
			//TODO Проверить на файле из видео обработку материалов
			synctypefind = true;
			rulestring_one.ReplaceAll("Material:", "");
			paramNamePrefix = "{Material:";
			GS::UniString templatestring = rulestring_one.GetSubstring('"', '"', 0);
			param.uniStringValue = templatestring;
			rulestring_one.ReplaceAll(templatestring, "");
			param.fromMaterial = true;
		}
	}
	if (synctypefind == false) {
		if (rulestring_one.Contains("Morph:")) {
			synctypefind = true;
			rulestring_one.ReplaceAll("Morph:", "");
			paramNamePrefix = "{Morph:";
			param.fromMorph = true;
		}
	}
	if (synctypefind == false) {
		if (rulestring_one.Contains("Coord:")) {
			synctypefind = true;
			rulestring_one.ReplaceAll("Coord:", "");
			paramNamePrefix = "{Coord:";
			param.fromCoord = true;
		}
	}
	if (synctypefind == false) {
		if (rulestring_one.Contains("Info:") && rulestring_one.Contains('"')) {
			synctypefind = true;
			rulestring_one.ReplaceAll("Info:", "");
			paramNamePrefix = "{Info:";
			param.fromInfo = true;
		}
	}
	if (synctypefind == false) {
		if (rulestring_one.Contains("IFC:")) {
			synctypefind = true;
			rulestring_one.ReplaceAll("IFC:", "");
			paramNamePrefix = "{IFC:";
			param.fromIFCProperty = true;
		}
	}
	//Если тип свойства не нашли - выходим
	if (synctypefind == false) return false;

	GS::UniString tparamName = rulestring_one.GetSubstring('{', '}', 0);
	UInt32 nparam = 0;
	GS::Array<GS::UniString> params;
	nparam = StringSplt(tparamName, ";", params);

	// Параметры не найдены - выходим
	if (nparam == 0) return false;
	param.rawName = paramNamePrefix + params[0].ToLowerCase() + "}";
	param.name = params[0];
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


// -----------------------------------------------------------------------------
// Запись значения IFC свойства в другое свойство
// -----------------------------------------------------------------------------
GSErrCode SyncIFCAndProp(const API_Guid & elemGuid, const SyncRule & syncRule, const API_PropertyDefinition & definition)
{
	API_IFCProperty property_from;
	API_Property property_to;
	GSErrCode err = GetIFCPropertyByName(elemGuid, syncRule.paramNameFrom, property_from);
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
		//if (syncRule.syncdirection == SYNC_FROM) {
		//	if (!SyncCheckIgnoreVal(syncRule, property_from)) {
		//		return  WriteProp(elemGuid, property_to, param_string, param_int, param_bool, param_real);
		//	}
		//	else {
		//		return  APIERR_MISSINGCODE; // Игнорируем значение
		//	}
		//}
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
	//err = WriteProp(elemGuid, property, param_string);
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