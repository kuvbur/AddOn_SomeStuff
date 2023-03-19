//------------ kuvbur 2022 ------------
#include	<map>
#include	"APIEnvir.h"
#include	"ACAPinc.h"
#include	"Helpers.hpp"
#include	"Summ.hpp"
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
					if (!rule.elemts.IsEmpty()) err = Sum_OneRule(rule, paramToReadelem);
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
					rules.Add(definition.guid, paramtype);
					flag_add = true;
				}
			}
			else {
				flag_add = true;
			}
			if (flag_add == true) {
				// Дописываем элемент в правило
				rules.Get(definition.guid).elemts.Push(elemGuid);
				// Добавляем свойства для чтения в словарь
				SumRule paramtype = rules.Get(definition.guid);
				if (!paramtype.position.IsEmpty()) ParamHelpers::AddParamValue2ParamDictElement(elemGuid, propertyParams.Get(paramtype.position), paramToReadelem);
				if (!paramtype.value.IsEmpty()) ParamHelpers::AddParamValue2ParamDictElement(elemGuid, propertyParams.Get(paramtype.value), paramToReadelem);
				if (!paramtype.criteria.IsEmpty()) ParamHelpers::AddParamValue2ParamDictElement(elemGuid, propertyParams.Get(paramtype.criteria), paramToReadelem);
				has_sum = true;
			}
		}
	}
	return has_sum;
}// Sum_GetElement


// -----------------------------------------------------------------------------------------------------------------------
// Функция возвращает заполненное правило суммирования SumRule
// SumRule.sum_type - тип суммирования (TextSum / NumSum)
// SumRule.value - имя свойства в формате rawname
// SumRule.criteria - критерий суммирования (разбивки)
// SumRule.delimetr - разделитель для текстовой суммы (конкатенации)
// SumRule.ignore_val - игнорируемые значения
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
	if (nparam == 0) return false;

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

GSErrCode Sum_OneRule(const SumRule& rule, ParamDictElement& paramToReadelem) {
	GSErrCode							err = NoError;
	SumCriteria							criteriaList;
	GS::UniString delimetr = GS::UniString(rule.delimetr.c_str());

	// Выбираем значения критериев
	for (UInt32 i = 0; i < rule.elemts.GetSize(); i++) {
		if (paramToReadelem.ContainsKey(rule.elemts[i])) {
			ParamDictValue params = paramToReadelem.Get(rule.elemts[i]);
			if (!rule.criteria.IsEmpty() && params.ContainsKey(rule.criteria)) {
				std::string criteria = params.Get(rule.criteria).val.uniStringValue.ToCStr(0, MaxUSize, CC_Cyrillic).Get();
				criteriaList[criteria].inx.Push(i);
			}
			else {
				criteriaList["all"].inx.Push(i);
			}
		}
	}
	ParamDictElement paramToWriteelem;

	// Проходим по словарю с критериями и суммируем
	for (SumCriteria::iterator i = criteriaList.begin(); i != criteriaList.end(); ++i) {
		GS::Array<UInt32> eleminpos = i->second.inx;
		ParamValueData summ; // Для суммирования числовых значений
		bool has_sum = false;
		for (UInt32 j = 0; j < eleminpos.GetSize(); j++) {
			API_Guid elemGuid = rule.elemts[eleminpos[j]];
			ParamDictValue params = paramToReadelem.Get(elemGuid);
			if (params.ContainsKey(rule.value)) {

				// Проверяем - было ли считано значение
				ParamValue param = params.Get(rule.value);
				if (param.isValid) {
					has_sum = true;
					// Дописать сложение с округлением, считывать настройки архикада
					summ.doubleValue = summ.doubleValue + param.val.doubleValue;
					summ.intValue = summ.intValue + param.val.intValue;
					summ.boolValue = summ.boolValue && param.val.boolValue;
					if (rule.sum_type == TextSum) {
						summ.uniStringValue = summ.uniStringValue + param.val.uniStringValue + delimetr;
					}
				}
			}
		}
		if (rule.sum_type == TextSum) {
			GS::UniString unic = StringUnic(summ.uniStringValue, delimetr);
			summ.uniStringValue = unic;
		}
		// Заполнение словаря записи
		if (has_sum == true) {
			for (UInt32 j = 0; j < eleminpos.GetSize(); j++) {
				API_Guid elemGuid = rule.elemts[eleminpos[j]];
				ParamDictValue params = paramToReadelem.Get(elemGuid);
				if (params.ContainsKey(rule.position)) {
					ParamValue param = params.Get(rule.position);
					param.isValid = true;
					summ.type = param.val.type;
					param.val = summ;
					ParamHelpers::AddParamValue2ParamDictElement(elemGuid, param, paramToWriteelem);
				}
			}
		}
	}
	if (!paramToWriteelem.IsEmpty()) {
		ParamHelpers::ElementsWrite(paramToWriteelem);
	}
	else {
		msg_rep("SyncByType", "No data to write", NoError, APINULLGuid);
	}
	return err;
}