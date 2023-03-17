//------------ kuvbur 2022 ------------
#include	<map>
#include	<unordered_map>
#include	"3dpart/alphanum.hpp"
#include	"APIEnvir.h"
#include	"ACAPinc.h"
#include	"Helpers.hpp"
#include	"Summ.hpp"
typedef std::map<std::string, int, doj::alphanum_less<std::string> > SumValues;
typedef std::unordered_map <std::string, SortInx> SumCriteria;

// -----------------------------------------------------------------------------------------------------------------------
// Объединение уникальных значений свойств выделенных элементов
// Во многом похоже на модуль ReNum
// Флаг суммирования делать не буду - ибо смысла не вижу.
// Будем суммировать все выделенные элементы, у которых задано правило
// 1. Получаем список объектов, в свойствах которых ищем
//					Sum{*имя свойства-значения*; *имя свойства-критерия*}
// 2. Собираем правила суммирования
// 3. Для каждого правила отбираем элементы
// 4. Средни них ищем уникальные значения
// 5. Записываем их через разделитель
// TODO Попробовать сделать диапазоны 1...10 и т.д.
// -----------------------------------------------------------------------------------------------------------------------

GSErrCode SumSelected(void) {
	GSErrCode err = NoError;
	SumRules rules;
	GS::Array<API_Guid> guidArray = GetSelectedElements(true, true);
	if (guidArray.IsEmpty()) return err;
	ParamDictElement paramToReadelem;
	// Получаем список правил суммирования
	bool hasSum = false;
	for (UInt32 i = 0; i < guidArray.GetSize(); i++) {
		GS::Array<API_PropertyDefinition>	definitions;
		err = ACAPI_Element_GetPropertyDefinitions(guidArray[i], API_PropertyDefinitionFilter_UserDefined, definitions);
		if (err == NoError && !definitions.IsEmpty()) {
			ParamDictValue propertyParams;
			ParamHelpers::GetAllPropertyDefinitionToParamDict(propertyParams, definitions);
			if (!propertyParams.IsEmpty()) {
				if (Sum_GetElement(guidArray[i], propertyParams, paramToReadelem, rules)) hasSum = true;
			}
		}
	}
	if (!hasSum) return err;
	ParamDictValue propertyParams; // Все свойства уже считаны, поэтому словарь просто пустой
	ParamHelpers::ElementsRead(paramToReadelem, propertyParams);
	GS::UniString undoString = RSGetIndString(AddOnStringsID, UndoReNumId, ACAPI_GetOwnResModule());
	ACAPI_CallUndoableCommand(undoString, [&]() -> GSErrCode {
		if (!guidArray.IsEmpty()) {
			// Есть список правил, в каждом правиле - список элементов. Прохдоим по правилам и обрабатываем каждое
			if (!rules.IsEmpty()) {
				for (GS::HashTable<API_Guid, SumRule>::PairIterator cIt = rules.EnumeratePairs(); cIt != NULL; ++cIt) {
					const SumRule& rule = *cIt->value;
					if (!rule.elemts.IsEmpty()) {
						err = Sum_OneRule(rule);
					}
				}
			}
		}
		GS::UniString intString = GS::UniString::Printf("Qty elements - %d", guidArray.GetSize());
		msg_rep("SumSelected", intString, err, APINULLGuid);
		rules.Clear();
		return err;
							  });
	return err;
}

// -----------------------------------------------------------------------------------------------------------------------
// Функция распределяет элемент в таблицу с правилами нумерации
// -----------------------------------------------------------------------------------------------------------------------
bool Sum_GetElement(const API_Guid& elemGuid, ParamDictValue& propertyParams, ParamDictElement& paramToReadelem, SumRules& rules) {
	SumElement el = {};
	bool has_sum = false;
	for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = propertyParams.EnumeratePairs(); cIt != NULL; ++cIt) {
		ParamValue& param = *cIt->value;
		API_PropertyDefinition& definition = param.definition;
		// Является ли свойство описанием системы суммирования?
		if (param.definition.description.Contains("Sum") && param.definition.description.Contains("{") && param.definition.description.Contains("}")) {
			bool flag_add = false;
			if (!rules.ContainsKey(definition.guid)) {
				SumRule paramtype = {};
				if (Sum_Rule(elemGuid, definition, propertyParams, paramtype)) {
					paramtype.position = param.rawName;
					ParamHelpers::AddParamValue2ParamDictElement(elemGuid, propertyParams.Get(paramtype.position), paramToReadelem);
					ParamHelpers::AddParamValue2ParamDictElement(elemGuid, propertyParams.Get(paramtype.value), paramToReadelem);
					if (!paramtype.criteria.IsEmpty()) ParamHelpers::AddParamValue2ParamDictElement(elemGuid, propertyParams.Get(paramtype.criteria), paramToReadelem);
					rules.Add(definition.guid, paramtype);
					flag_add = true;
				}
			}
			else {
				flag_add = true;
			}
			if (flag_add == true) {// Дописываем элемент в правило
				SumElement el = {};
				el.guid = elemGuid;
				rules.Get(definition.guid).elemts.Push(el);
				has_sum = true;
			}
		}
	}
	return has_sum;
}// Sum_GetElement

// -----------------------------------------------------------------------------------------------------------------------
// Функция возвращает заполненное правило суммирования
// -----------------------------------------------------------------------------------------------------------------------
bool Sum_Rule(const API_Guid& elemGuid, const API_PropertyDefinition& definition, ParamDictValue& propertyParams, SumRule& paramtype) {
	// По типу данных свойства определим тим суммирования
	// Если строковый тип - объединяем уникальные значения, если тип числовой - суммируем
	paramtype.sum_type = 0;
	if (definition.valueType == API_PropertyStringValueType) paramtype.sum_type = TextSum;
	if (definition.valueType == API_PropertyRealValueType || definition.valueType == API_PropertyIntegerValueType) paramtype.sum_type = NumSum;
	if (!paramtype.sum_type) return false;

	GS::UniString paramName = definition.description.GetSubstring('{', '}', 0);
	GS::Array<GS::UniString>	partstring;
	int nparam = StringSplt(paramName.ToLowerCase(), ";", partstring);
	if (nparam==0) return false;

	if (propertyParams.ContainsKey("{" + partstring[0] + "}")) {
		paramtype.value = "{" + partstring[0] + "}";
	}
	else {
		return false;
	}
	// Ищём определение свойства-критерия
	if (nparam > 1) {
		if (propertyParams.ContainsKey("{" + partstring[1] + "}")) paramtype.criteria = "{" + partstring[1] + "}";
	}
	// Если задан и разделитель - пропишем его
	if (nparam == 3) paramtype.delimetr = partstring[3].ToCStr().Get();
	// Если заданы игнорируемые значения
	if (nparam == 4) paramtype.ignore_val = partstring[4].ToCStr().Get();
	return true;
} // ReNumRule

GSErrCode Sum_OneRule(const SumRule& rule) {
	GSErrCode							err = NoError;
	//GS::Array<SumElement>				elemArray = rule.elemts;
	//GS::Array<API_PropertyDefinition>	definitions;
	//SumCriteria							criteriaList;
	//GS::UniString delimetr = GS::UniString(rule.delimetr.c_str());
	//definitions.Push(rule.value);
	//if (rule.criteria.guid != APINULLGuid) definitions.Push(rule.criteria);

	//// Прочитаем значения свойств значения и критерия
	//for (UInt32 i = 0; i < elemArray.GetSize(); i++) {
	//	GS::Array<API_Property>  properties;
	//	err = ACAPI_Element_GetPropertyValues(elemArray[i].guid, definitions, properties);
	//	if (err != NoError) {
	//		msg_rep("Sum_OneRule", "ACAPI_Element_GetPropertyValues", err, elemArray[i].guid);
	//	}
	//	else {
	//		if (rule.sum_type == TextSum) {
	//			GS::UniString val = PropertyHelpers::ToString(properties[0]);
	//			if (!CheckIgnoreVal(rule.ignore_val, val)) {
	//				elemArray[i].string_value = val.ToCStr(0, MaxUSize, CC_Cyrillic).Get();
	//				if (rule.criteria.guid != APINULLGuid) {
	//					elemArray[i].criteria = PropertyHelpers::ToString(properties[1]).ToCStr(0, MaxUSize, CC_Cyrillic).Get();
	//					criteriaList[elemArray[i].criteria].inx.Push(i);
	//				}
	//				else {
	//					criteriaList["all"].inx.Push(i);
	//				}
	//			}
	//		}
	//		if (rule.sum_type == NumSum) {
	//			elemArray[i].num_value = properties[0].value.singleVariant.variant.doubleValue;
	//			if (rule.criteria.guid != APINULLGuid) {
	//				elemArray[i].criteria = PropertyHelpers::ToString(properties[1]).ToCStr(0, MaxUSize, CC_Cyrillic).Get();
	//				criteriaList[elemArray[i].criteria].inx.Push(i);
	//			}
	//			else {
	//				criteriaList["all"].inx.Push(i);
	//			}
	//		}
	//	}
	//}

	//// Идём по критериям и ищем повторяющиеся свойства
	//for (SumCriteria::iterator i = criteriaList.begin(); i != criteriaList.end(); ++i) {

	//	// Повторяем процедуру поиска уникальных значений, теперь для значения
	//	SumValues valueList;
	//	GS::Array<UInt32> eleminpos = i->second.inx;
	//	for (UInt32 j = 0; j < eleminpos.GetSize(); j++) {
	//		UInt32 elem_inx = eleminpos[j];
	//		valueList[elemArray[elem_inx].string_value];
	//	}
	//	GS::UniString param_string;
	//	for (SumValues::iterator k = valueList.begin(); k != valueList.end(); ++k) {
	//		std::string s = k->first;
	//		GS::UniString unis = GS::UniString(s.c_str());
	//		if (param_string.IsEmpty()) {
	//			param_string = unis;
	//		}
	//		else {
	//			param_string = param_string + delimetr + unis;
	//		}
	//	}
	//	for (UInt32 j = 0; j < eleminpos.GetSize(); j++) {
	//		API_Guid		elemGuid = elemArray[eleminpos[j]].guid;
	//		API_Property	positionproperty = {};
	//		err = ACAPI_Element_GetPropertyValue(elemGuid, rule.position, positionproperty);
	//		if (err != NoError) {
	//			msg_rep("Sum_OneRule", "ACAPI_Element_GetPropertyValues", err, elemGuid);
	//		}
	//		else {

	//			//TODO добавить запись в свойство
	//			//err = WriteProp(elemGuid, positionproperty, param_string);
	//			if (err != NoError) msg_rep("Sum_OneRule", "WriteProp", err, elemGuid);
	//		}
	//	}
	//}
	return err;
}