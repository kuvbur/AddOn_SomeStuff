//------------ kuvbur 2022 ------------
#include	<map>
#include	<unordered_map>
#include	"3dpart/alphanum.hpp"
#include	"APIEnvir.h"
#include	"ACAPinc.h"
#include	"ReNum.hpp"
#include	"Helpers.hpp"

typedef std::map<std::string, SortGUID, doj::alphanum_less<std::string> > Values;

typedef std::unordered_map <std::string, SortInx> Delimetr;

// -----------------------------------------------------------------------------------------------------------------------
// Итак, задача, суть такова:
// 1. Получаем список объектов, в свойствах которых ищем
//		Флаг включения нумерации в формате
//					Renum_flag{*имя свойства с правилом*}
//					Renum_flag{*имя свойства с правилом*; NULL}
//					Renum_flag{*имя свойства с правилом*; ALLNULL}
//						Тип данных свойства-флага: Набор параметров с вариантами "Включить", "Исключить", "Не менять"
//		Правило нумерации в одном из форматов
//	 				Renum{*имя свойства-критерия*}
//					Renum{*имя свойства-критерия*; *имя свойства-разбивки*}
// 2. Записываем для каждого элемента в структуру свойства, участвующие в правиле
// 3. Откидываем все с вфключенным флагом
// 4. По количеству уникальных имён свойств-правил, взятых из Renum_flag{*имя свойства с правилом*}, разбиваем элементы
// -----------------------------------------------------------------------------------------------------------------------

GSErrCode ReNumSelected(void) {
	GS::Array<API_Guid> guidArray = GetSelectedElements(true, true);
	if (guidArray.IsEmpty()) return NoError;
	long time_start = clock();
	ParamDictElement paramToWriteelem;
	if (!GetRenumElements(guidArray, paramToWriteelem)) {
		msg_rep("ReNumSelected", "No data to write", NoError, APINULLGuid);
		return NoError;
	}
	GS::UniString undoString = RSGetIndString(AddOnStringsID, UndoReNumId, ACAPI_GetOwnResModule());
	ACAPI_CallUndoableCommand(undoString, [&]() -> GSErrCode {
		ParamHelpers::ElementsWrite(paramToWriteelem);
		long time_end = clock();
		GS::UniString time = GS::UniString::Printf(" %d s", (time_end - time_start) / 1000);
		GS::UniString intString = GS::UniString::Printf("Qty elements - %d ", guidArray.GetSize()) + GS::UniString::Printf("wrtite to - %d", paramToWriteelem.GetSize()) + time;
		msg_rep("ReNumSelected", intString, NoError, APINULLGuid);
		return NoError;
							  });
	return NoError;
}

bool GetRenumElements(const GS::Array<API_Guid> guidArray, ParamDictElement& paramToWriteelem) {
	// Получаем список правил суммирования
	bool hasRenum = false;
	Rules rules;
	ParamDictElement paramToReadelem;
	for (UInt32 i = 0; i < guidArray.GetSize(); i++) {
		GS::Array<API_PropertyDefinition>	definitions;
		GSErrCode err = ACAPI_Element_GetPropertyDefinitions(guidArray[i], API_PropertyDefinitionFilter_UserDefined, definitions);
		if (err == NoError && !definitions.IsEmpty() && ReNumHasFlag(definitions)) {
			ParamDictValue propertyParams;
			ParamHelpers::GetAllPropertyDefinitionToParamDict(propertyParams, definitions);
			if (ReNum_GetElement(guidArray[i], propertyParams, paramToReadelem, rules)) hasRenum = true;
		}
	}
	if (!hasRenum) return false;
	ParamDictValue propertyParams; // Все свойства уже считаны, поэтому словарь просто пустой
	ParamHelpers::ElementsRead(paramToReadelem, propertyParams); // Читаем значения
	// Теперь выясняем - какой режим нумерации у элементов и распределяем позиции
	return hasRenum;
}

// -----------------------------------------------------------------------------------------------------------------------
// Функция распределяет элемент в таблицу с правилами нумерации
// -----------------------------------------------------------------------------------------------------------------------
bool ReNum_GetElement(const API_Guid& elemGuid, ParamDictValue& propertyParams, ParamDictElement& paramToReadelem, Rules& rules) {
	bool hasRenum = false;
	for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = propertyParams.EnumeratePairs(); cIt != NULL; ++cIt) {
		bool flag = false;
		ParamValue& param = *cIt->value;
		API_PropertyDefinition& definition = param.definition;
		if (definition.description.Contains("Renum_flag") && definition.description.Contains("{") && definition.description.Contains("}")) {
			if (!rules.ContainsKey(definition.guid)) {
				RenumRule rulecritetia = {};
				// Разбираем - что записано в свойстве с флагом
				// В нём должно быть имя свойства и, возможно, флаг добавления нулей
				GS::UniString paramName = definition.description.ToLowerCase().GetSubstring('{', '}', 0);
				GS::UniString rawNameposition = "{";
				if (paramName.Contains(";")) { // Есть указание на нули
					GS::Array<GS::UniString>	partstring;
					int nparam = StringSplt(paramName, ";", partstring);
					rawNameposition = rawNameposition + partstring[0]+"}";
					if (nparam > 1) {
						if (partstring[1] == "null") rulecritetia.nulltype = ADDZEROS;
						if (partstring[1] == "allnull") rulecritetia.nulltype = ADDMAXZEROS;
					};
				}
				else {
					rawNameposition = rawNameposition + paramName + "}";
				}
				// Проверяем - есть ли у объекта такое свойство-правило
				if (propertyParams.ContainsKey(rawNameposition)) {
					// В описании правила может быть указано имя свойства-критерия и, возможно, имя свойства-разбивки
					GS::UniString ruleparamName = propertyParams.Get(rawNameposition).definition.description;
					if (ruleparamName.Contains("Renum") && ruleparamName.Contains("{") && ruleparamName.Contains("}")) {
						ruleparamName = ruleparamName.ToLowerCase().GetSubstring('{', '}', 0);
						GS::UniString rawNamecriteria = "{";
						GS::UniString rawNamedelimetr = "";
						if (ruleparamName.Contains(";")) { // Есть указание на нули
							GS::Array<GS::UniString>	partstring;
							int nparam = StringSplt(ruleparamName, ";", partstring);
							rawNamecriteria = rawNamecriteria + partstring[0] + "}";
							if (nparam > 1) rawNamedelimetr = "{" + rawNamedelimetr + partstring[1] + "}";
						}
						else {
							rawNamecriteria = rawNamecriteria + paramName + "}";
						}
						// Если такие свойства есть - записываем правило
						if (propertyParams.ContainsKey(rawNamecriteria) && (propertyParams.ContainsKey(rawNamedelimetr) || rawNamedelimetr.IsEmpty())) {
							rulecritetia.position = rawNameposition;
							rulecritetia.flag = param.rawName;
							rulecritetia.criteria = rawNamecriteria;
							rulecritetia.delimetr = rawNamedelimetr;
							rules.Add(definition.guid, rulecritetia);
							flag = true;
							hasRenum = true;
						}
					}
				}
			}
			else {
				flag = true; // Правило уже существует, просто доабим свойства в словарь чтения и id в правила
			}
			if (flag) {
				RenumElement el = {};
				el.guid = elemGuid;
				rules.Get(definition.guid).elemts.Push(el);
				RenumRule rulecritetia = rules.Get(definition.guid);
				if (!rulecritetia.position.IsEmpty()) ParamHelpers::AddParamValue2ParamDictElement(elemGuid, propertyParams.Get(rulecritetia.position), paramToReadelem);
				if (!rulecritetia.flag.IsEmpty()) ParamHelpers::AddParamValue2ParamDictElement(elemGuid, propertyParams.Get(rulecritetia.flag), paramToReadelem);
				if (!rulecritetia.criteria.IsEmpty()) ParamHelpers::AddParamValue2ParamDictElement(elemGuid, propertyParams.Get(rulecritetia.criteria), paramToReadelem);
				if (!rulecritetia.delimetr.IsEmpty()) ParamHelpers::AddParamValue2ParamDictElement(elemGuid, propertyParams.Get(rulecritetia.delimetr), paramToReadelem);
			}
		}
	}
	return hasRenum;
}

// -----------------------------------------------------------------------------------------------------------------------
// Запись позиции в свойство
// -----------------------------------------------------------------------------------------------------------------------
GSErrCode ReNumSetPos(const Int32 npos, const Int32 maxnpos, API_Property& positionproperty, bool& flag_write) {
	GSErrCode	err = NoError;
	if (positionproperty.definition.valueType == API_PropertyRealValueType) {
		if (!is_equal(positionproperty.value.singleVariant.variant.doubleValue, npos / 1.0) || positionproperty.isDefault) {
			positionproperty.value.singleVariant.variant.doubleValue = npos;
			flag_write = true;
		}
	}
	if (positionproperty.definition.valueType == API_PropertyIntegerValueType) {
		if (positionproperty.value.singleVariant.variant.intValue != npos || positionproperty.isDefault) {
			positionproperty.value.singleVariant.variant.intValue = npos;
			flag_write = true;
		}
	}
	if (positionproperty.definition.valueType == API_PropertyStringValueType) {

		//Тут должен быть десятичный логарифм, но мне лениво
		GS::UniString strnpos;
		if (maxnpos < 10) strnpos = GS::UniString::Printf("%d", npos);
		if (maxnpos < 100 && strnpos.IsEmpty()) strnpos = GS::UniString::Printf("%02d", npos);
		if (maxnpos < 1000 && strnpos.IsEmpty()) strnpos = GS::UniString::Printf("%03d", npos);
		if (!positionproperty.value.singleVariant.variant.uniStringValue.IsEqual(strnpos) || positionproperty.isDefault) {
			positionproperty.value.singleVariant.variant.uniStringValue = strnpos;
			flag_write = true;
		}
	}
	return err;
}

GSErrCode ReNumOneRule(const RenumRule& rule) {
	GSErrCode					err = NoError;
	//GS::Array<RenumElement>		elemArray = rule.elemts;
	//API_PropertyDefinition		position = rule.position; // В это свойство ставим позицию
	//Delimetr delimetrList;
	//bool hasdelimetr = (rule.delimetr.guid != APINULLGuid);
	//std::string delimetr = "";

	//// Итак, у нас есть - свойство для разбивки и свойство для критерия.
	//// Поскольку чтение свойств - затратный процесс - прочитаем значение в массив и далее будем работать с ним
	//GS::Array<API_PropertyDefinition> definitions;
	//definitions.Push(rule.criteria);
	//if (hasdelimetr) definitions.Push(rule.delimetr);

	//// Собираем значения свойств из criteria. Нам нужны только уникальные значения.
	//for (UInt32 i = 0; i < elemArray.GetSize(); i++) {
	//	GS::Array<API_Property>  properties;
	//	err = ACAPI_Element_GetPropertyValues(elemArray[i].guid, definitions, properties);
	//	if (err != NoError) msg_rep("ReNumOneRule", "ACAPI_Element_GetPropertyValues", err, elemArray[i].guid);
	//	if (err == NoError) {
	//		elemArray[i].criteria = PropertyHelpers::ToString(properties[0]).ToCStr(0, MaxUSize, CC_Cyrillic).Get();
	//		if (hasdelimetr) {
	//			delimetr = PropertyHelpers::ToString(properties[1]).ToCStr(0, MaxUSize, CC_Cyrillic).Get();
	//			elemArray[i].delimetr = delimetr;
	//		}

	//		// Ну и чтоб дважды не вставать - сделаем список с уникальными значениями разделителя.
	//		delimetrList[delimetr].inx.Push(i);
	//	}
	//}

	//// Теперь последовательно идём по словарю c разделителями, вытаскиваем оттуда guid и нумеруем
	//// Тут бы применить вложенные словари, но я не умею

	//// Если нужно общее количество строк с учётом разделителя
	//// TODO Плохая идея - дважды массивы гонять
	//Int32 maxnpos = 1;
	//if (rule.nulltype == ADDMAXZEROS) {
	//	for (Delimetr::iterator i = delimetrList.begin(); i != delimetrList.end(); ++i) {
	//		Values criteriaList;
	//		GS::Array<UInt32> eleminpos = i->second.inx;
	//		for (UInt32 j = 0; j < eleminpos.GetSize(); j++) {
	//			UInt32 elem_inx = eleminpos[j];
	//			criteriaList[elemArray[elem_inx].criteria].guid.Push(elemArray[elem_inx].guid);
	//		}
	//		for (Values::iterator k = criteriaList.begin(); k != criteriaList.end(); ++k) {
	//			maxnpos += 1;
	//		}
	//	}
	//}
	//for (Delimetr::iterator i = delimetrList.begin(); i != delimetrList.end(); ++i) {

	//	// Повторяем процедуру поиска уникальных значений, теперь для критерия
	//	Values criteriaList;
	//	GS::Array<UInt32> eleminpos = i->second.inx;
	//	for (UInt32 j = 0; j < eleminpos.GetSize(); j++) {
	//		UInt32 elem_inx = eleminpos[j];
	//		criteriaList[elemArray[elem_inx].criteria].guid.Push(elemArray[elem_inx].guid);
	//	}

	//	// TODO Добавить исключаемые позиции
	//	if (rule.nulltype == ADDZEROS) {
	//		maxnpos = 1;
	//		for (Values::iterator k = criteriaList.begin(); k != criteriaList.end(); ++k) {
	//			maxnpos += 1;
	//		}
	//	}
	//	Int32 npos = 0;
	//	for (Values::iterator k = criteriaList.begin(); k != criteriaList.end(); ++k) {
	//		npos += 1;
	//		GS::Array<API_Guid> eleminpos = k->second.guid;
	//		for (UInt32 m = 0; m < eleminpos.GetSize(); m++) {
	//			API_Property positionproperty = {};
	//			err = ACAPI_Element_GetPropertyValue(eleminpos[m], position.guid, positionproperty);
	//			if (err != NoError) msg_rep("ReNumOneRule", "ACAPI_Element_GetPropertyValue", err, eleminpos[m]);
	//			if (err == NoError) {
	//				bool flag_write = false;
	//				if (ReNumSetPos(npos, maxnpos, positionproperty, flag_write) == NoError) {
	//					if (flag_write == true) {
	//						positionproperty.isDefault = false;
	//						err = ACAPI_Element_SetProperty(eleminpos[m], positionproperty);
	//						if (err != NoError) msg_rep("ReNumOneRule", "ACAPI_Element_SetProperty", err, eleminpos[m]);
	//					}
	//				}
	//			}
	//		}
	//	}
	//}
	return err;
}

// -----------------------------------------------------------------------------------------------------------------------
// Функция возвращает режим нумерации (RENUM_IGNORE, RENUM_ADD, RENUM_NORMAL) и описание свойства с правилом
// -----------------------------------------------------------------------------------------------------------------------
bool ReNumRule(const API_Guid& elemGuid, const GS::UniString& description_string, RenumRule& paramtype) {
	bool flag = false;
	if (description_string.IsEmpty()) return flag;
	if (description_string.Contains("{") && description_string.Contains("}")) {
		//GS::Array<GS::UniString>	partstring;
		//API_PropertyDefinition		criteria = {};
		//API_PropertyDefinition		delimetr = {};
		//GSErrCode					err = NoError;
		//GS::UniString paramName = description_string.GetSubstring('{', '}', 0);
		//paramName.ReplaceAll("property:", "");
		//int nparam = StringSplt(paramName, ";", partstring);
		//if (nparam == 0) return flag;
		//err = GetPropertyDefinitionByName(elemGuid, partstring[0], criteria);
		//if (err == NoError) {
		//	paramtype.criteria = criteria;
		//	flag = true;
		//}
		//if (nparam == 2) {
		//	err = GetPropertyDefinitionByName(elemGuid, partstring[1], delimetr);
		//	if (err == NoError) {
		//		paramtype.delimetr = delimetr;
		//		flag = true;
		//	}
		//}
	} // description_string.Contains("Renum")
	return flag;
} // ReNumRule

//--------------------------------------------------------------------------------------------------------------------------
// Проверяет - есть ли хоть одно описание флага
//--------------------------------------------------------------------------------------------------------------------------
bool ReNumHasFlag(const GS::Array<API_PropertyDefinition> definitions) {
	if (definitions.IsEmpty()) return false;
	for (UInt32 i = 0; i < definitions.GetSize(); i++) {
		if (!definitions[i].description.IsEmpty()) {
			if (definitions[i].description.Contains("Renum_flag")) {
				return true;
			}
		}
	}
	return false;
}

// -----------------------------------------------------------------------------------------------------------------------
// Функция возвращает режим нумерации (RENUM_IGNORE, RENUM_ADD, RENUM_NORMAL)
// -----------------------------------------------------------------------------------------------------------------------
Int32 ReNumGetFlagValue(const API_Property& propertyflag) {
#if defined(AC_22) || defined(AC_23)
	bool isnoteval = (!propertyflag.isEvaluated);
#else
	bool isnoteval = (propertyflag.status != API_Property_HasValue);
#endif
	if (isnoteval) {
		return RENUM_IGNORE;
	}
	UInt32			flag = RENUM_IGNORE;
	GS::UniString		state = "";
	API_PropertyValue	val;
	if (propertyflag.isDefault) {
		val = propertyflag.definition.defaultValue.basicValue;
	}
	else {
		val = propertyflag.value;
	}
	if (propertyflag.definition.valueType == API_PropertyBooleanValueType) {
		if (val.singleVariant.variant.boolValue) flag = RENUM_NORMAL;
	}
	else {
#if defined(AC_25) || defined(AC_26)
		state = val.singleVariant.variant.uniStringValue;
#else
		state = val.singleEnumVariant.displayVariant.uniStringValue;
#endif
		flag = RENUM_NORMAL;
		if (state.Contains(u8"Исключить") || state.IsEmpty()) flag = RENUM_IGNORE;
		if (state.Contains(u8"Не менять")) flag = RENUM_ADD;

		// Если флаг поднят - ищем свойство с правилом
	}
	return flag;
}

// -----------------------------------------------------------------------------------------------------------------------
// Функция возвращает режим нумерации (RENUM_IGNORE, RENUM_ADD, RENUM_NORMAL) и описание свойства с правилом
// -----------------------------------------------------------------------------------------------------------------------
UInt32 ReNumGetRule(const API_PropertyDefinition definitionflag, const API_Guid& elemGuid, API_PropertyDefinition& propertdefyrule, short& nulltype) {
	UInt32 flag = RENUM_IGNORE;
	if (definitionflag.description.Contains("{") && definitionflag.description.Contains("}") && definitionflag.description.Contains("property:")) {

		// Получаем значение флага
		GSErrCode err = NoError;
		GS::Array<API_PropertyDefinition> definitions;
		definitions.Push(definitionflag);
		GS::Array<API_Property>  propertyflag;
		err = ACAPI_Element_GetPropertyValues(elemGuid, definitions, propertyflag);
		if (err == NoError) {
			flag = ReNumGetFlagValue(propertyflag[0]);
			if (flag != RENUM_IGNORE) {
				GS::UniString paramName = definitionflag.description.GetSubstring('{', '}', 0);
				paramName.ReplaceAll("property:", "");
				if (paramName.Contains(";")) {
					GS::Array<GS::UniString>	partstring;
					int nparam = StringSplt(paramName, ";", partstring);
					if (nparam == 0) return RENUM_IGNORE;
					paramName = partstring[0];
					if (nparam > 1) {
						if (partstring[1].ToLowerCase() == "null") nulltype = ADDZEROS;
						if (partstring[1].ToLowerCase() == "allnull") nulltype = ADDMAXZEROS;
					};
				}

				// TODO вернуть функцию GetPropertyDefinitionByName
				//err = GetPropertyDefinitionByName(elemGuid, paramName, propertdefyrule);
				if (err != NoError) flag = RENUM_IGNORE;
			}// flag != RENUM_IGNORE
		}// ACAPI_Element_GetPropertyValues
	} // description_string.Contains("Renum_flag")
	return flag;
} // ReNumGetRule