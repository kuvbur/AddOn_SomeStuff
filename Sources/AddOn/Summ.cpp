#include	"APIEnvir.h"
#include	"ACAPinc.h"
#include	"Summ.hpp"
#include	"Helpers.hpp"
#include	<map>
#include	<unordered_map>
#include	"alphanum.hpp"

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
// 4. Средни них ищем уни кальные значения
// 5. Записываем их через разделитель
// TODO Попробовать сделать диапазоны 1...10 и т.д.
// -----------------------------------------------------------------------------------------------------------------------

GSErrCode SumSelected(void) {
	GSErrCode err = NoError;
	SumRules rules;
	GS::Array<API_Guid> guidArray = GetSelectedElements(true, true);
	GS::UniString undoString = RSGetIndString(AddOnStringsID, UndoReNumId, ACAPI_GetOwnResModule());
	ACAPI_CallUndoableCommand(undoString, [&]() -> GSErrCode {
		if (!guidArray.IsEmpty()) {
			// Получаем список правил суммирования
			for (UInt32 i = 0; i < guidArray.GetSize(); i++) {
				err = Sum_GetElement(guidArray[i], rules);
			}
			// Есть список правил, в каждом правиле - список элементов. Прохоим по правилам и обрабатываем каждое
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
GSErrCode Sum_GetElement(const API_Guid& elemGuid, SumRules& rules) {
	GSErrCode							err = NoError;
	GS::Array<API_PropertyDefinition>	definitions;
	SumElement el = {};
	err = ACAPI_Element_GetPropertyDefinitions(elemGuid, API_PropertyDefinitionFilter_UserDefined, definitions);
	if (err != NoError) {
		return err;
	}
	for (UInt32 j = 0; j < definitions.GetSize(); j++) {
		// Является ли свойство описанием системы суммирования?
		if (definitions[j].description.Contains("Sum")) {
			bool flag_add = false;
			if (!rules.ContainsKey(definitions[j].guid)) {
				SumRule paramtype = {};
				if(Sum_Rule(elemGuid, definitions[j], paramtype)){
					rules.Add(definitions[j].guid, paramtype);
					flag_add = true;
				}
			}
			else {
				flag_add = true;
			}
			if (flag_add == true) {
				// Дописываем элемент в правило
				SumElement el = {};
				el.guid = elemGuid;
				rules.Get(definitions[j].guid).elemts.Push(el);
			}
		} // definitions[j].description.Contains("Sumnum")
	}//for
	return err;
}// Sum_GetElement

// -----------------------------------------------------------------------------------------------------------------------
// Функция возвращает заполненное правило суммирования
// -----------------------------------------------------------------------------------------------------------------------
bool Sum_Rule(const API_Guid& elemGuid, const API_PropertyDefinition& definition, SumRule& paramtype) {
	bool flag = false;
	GS::UniString description_string = definition.description;
	if (description_string.Contains("{") && description_string.Contains("}")) {
		GS::Array<GS::UniString>	partstring;
		API_PropertyDefinition		value = {};
		API_PropertyDefinition		criteria = {};
		std::string					delimetr = "; ";
		std::string					ignore_val = "";
		GSErrCode					err = NoError;
		GS::UniString paramName = description_string.GetSubstring('{', '}', 0);
		paramName.ReplaceAll("Property:", "");
		int nparam = StringSplt(paramName, ";", partstring);
		if (nparam < 2) return false;
		// Ищём определение свойства-значения
		err = GetPropertyDefinitionByName(elemGuid, partstring[0], value);
		if (err != NoError) return false;
		paramtype.value = value;
		// Ищём определение свойства-критерия
		err = GetPropertyDefinitionByName(elemGuid, partstring[1], criteria);
		if (err != NoError) return false;
		paramtype.criteria = criteria;
		//Если дошли сюда - то уже всё хорошо. Отметим этот момент
		flag = true;
		paramtype.position = definition.guid;
		// Если задан и разделитель - пропишем его
		if (nparam == 3) delimetr = partstring[3].ToCStr().Get();
		paramtype.delimetr = delimetr;
		// Если заданы игнорируемые значения
		if (nparam == 4) ignore_val = partstring[4].ToCStr().Get();
		paramtype.ignore_val = ignore_val;
	}
	return flag;
} // ReNumRule


GSErrCode Sum_OneRule(const SumRule& rule) {
	GSErrCode							err = NoError;
	GS::Array<SumElement>				elemArray = rule.elemts;
	GS::Array<API_PropertyDefinition>	definitions;
	SumCriteria							criteriaList;
	GS::UniString delimetr = GS::UniString(rule.delimetr.c_str());
	definitions.Push(rule.criteria);
	definitions.Push(rule.value);
	// Прочитаем значения свойств значения и критерия
	for (UInt32 i = 0; i < elemArray.GetSize(); i++) {
		GS::Array<API_Property>  properties;
		err = ACAPI_Element_GetPropertyValues(elemArray[i].guid, definitions, properties);
		if (err != NoError) {
			msg_rep("ReNumOneRule", "ACAPI_Element_GetPropertyValues", err, elemArray[i].guid);
		}
		else {
			GS::UniString val = PropertyTestHelpers::ToString(properties[1]);
			if (!CheckIgnoreVal(rule.ignore_val, val)) {
				elemArray[i].criteria = PropertyTestHelpers::ToString(properties[0]).ToCStr(0, MaxUSize, CC_Cyrillic).Get();
				elemArray[i].value = val.ToCStr(0, MaxUSize, CC_Cyrillic).Get();
				criteriaList[elemArray[i].criteria].inx.Push(i);
			}
		}
	}
	// Идём по критериям и ищем повторяющиеся свойства
	for (SumCriteria::iterator i = criteriaList.begin(); i != criteriaList.end(); ++i) {
		// Повторяем процедуру поиска уникальных значений, теперь для значения
		SumValues valueList;
		GS::Array<UInt32> eleminpos = i->second.inx;
		for (UInt32 j = 0; j < eleminpos.GetSize(); j++) {
			UInt32 elem_inx = eleminpos[j];
			valueList[elemArray[elem_inx].value];
		}
		GS::UniString param_string;
		for (SumValues::iterator k = valueList.begin(); k != valueList.end(); ++k) {
			std::string s = k->first;
			GS::UniString unis = GS::UniString(s.c_str());
			if (param_string.IsEmpty()){
				param_string = unis;
			}
			else {
				param_string = param_string + delimetr + unis;
			}
		}
		for (UInt32 j = 0; j < eleminpos.GetSize(); j++) {
			API_Guid		elemGuid = elemArray[eleminpos[j]].guid;
			API_Property	positionproperty = {};
			err = ACAPI_Element_GetPropertyValue(elemGuid, rule.position, positionproperty);
			if (err != NoError) {
				msg_rep("Sum_OneRule", "ACAPI_Element_GetPropertyValues", err, elemGuid);
			}
			else {
				err = WriteProp(elemGuid, positionproperty, param_string);
				if (err != NoError) msg_rep("Sum_OneRule", "WriteProp", err, elemGuid);
			}
		}
	}
	return err;
}


