#include	"APIEnvir.h"
#include	"ACAPinc.h"
#include	"ReNum.hpp"
#include	"Helpers.hpp"
#include	<map>
#include	<unordered_map>
#include	"alphanum.hpp"

typedef std::map<std::string, SortGUID, doj::alphanum_less<std::string> > Values;
typedef std::unordered_map <std::string, SortInx> Delimetr;
#define RENUM_IGNORE 0
#define RENUM_ADD 1
#define RENUM_NORMAL 2

// -----------------------------------------------------------------------------------------------------------------------
// Итак, задача, суть такова:
// 1. Получаем список объектов, в свойствах которых ищем
//		Флаг включения нумерации в формате
//					Renum_flag{*имя свойства с правилом*}
//						Тип данных свойства-флага: Набор параметров с вариантами "Включить", "Исключить", "Не менять"
//		Правило нумерации в одном из форматов
//					Renum{*имя свойства-критерия*; *имя свойства-разбивки*}
//	 				Renum{*имя свойства-критерия*}
// 2. Записываем для каждого элемента в структуру свойства, участвующие в правиле
// 3. Откидываем все с вфключенным флагом
// 4. По количеству уникальных имён свойств-правил, взятых из Renum_flag{*имя свойства с правилом*}, разбиваем элементы
// -----------------------------------------------------------------------------------------------------------------------

GSErrCode ReNumSelected(void) {
	GSErrCode err = NoError;
	Rules rules;
	GS::Array<API_Guid> guidArray = GetSelectedElements(true, true);
	GS::UniString undoString = RSGetIndString(AddOnStringsID, UndoReNumId, ACAPI_GetOwnResModule());
	ACAPI_CallUndoableCommand(undoString, [&]() -> GSErrCode {
		if (!guidArray.IsEmpty()) {
			for (UInt32 i = 0; i < guidArray.GetSize(); i++) {
				err = ReNum_GetElement(guidArray[i], rules);
			}
			if (!rules.IsEmpty()) {
				// Теперь у нас есть списк правил. Можем пройти по каждому правилу и обработать элементы
				for (GS::HashTable<API_Guid, RenumRule>::PairIterator cIt = rules.EnumeratePairs(); cIt != NULL; ++cIt) {
					const RenumRule& rule = *cIt->value;
					if (rule.state && !rule.elemts.IsEmpty()) {
						err = ReNumOneRule(rule);
					}
				}
			}
		}
		GS::UniString intString = GS::UniString::Printf("Qty elements - %d", guidArray.GetSize());
		msg_rep("ReNumSelected", intString, err, APINULLGuid);
		rules.Clear();
		return err;
		});
	return err;
}

// -----------------------------------------------------------------------------------------------------------------------
// Функция распределяет элемент в таблицу с правилами нумерации
// -----------------------------------------------------------------------------------------------------------------------
GSErrCode ReNum_GetElement(const API_Guid& elemGuid, Rules& rules) {
	GSErrCode							err = NoError;
	GS::Array<API_PropertyDefinition>	definitions;
	RenumElement el = {};
	err = ACAPI_Element_GetPropertyDefinitions(elemGuid, API_PropertyDefinitionFilter_UserDefined, definitions);
	if (err == NoError) {
		for (UInt32 j = 0; j < definitions.GetSize(); j++) {
			// Является ли свойство описанием системы нумерации?
			if (definitions[j].description.Contains("Renum_flag")) {
				// Проверяем содержит ли описание свойства флаг нумерации со ссылкой на правило нумерации
				API_PropertyDefinition  propertydefrule = {};
				UInt32 state = ReNumGetRule(definitions[j], elemGuid, propertydefrule);
				if (state != RENUM_IGNORE) {
					// Если содержит - заполняем словарь с правилами и словарь элементов для этого правила
					// Если, конечно, раньше его не было
					if (!rules.ContainsKey(definitions[j].guid)) {
						RenumRule rulecritetia = {};
						rulecritetia.position = propertydefrule;
						rulecritetia.state = ReNumRule(elemGuid, propertydefrule.description, rulecritetia);;
						rules.Add(definitions[j].guid, rulecritetia);
					} //rules.ContainsKey(definitions[j].guid)
					// Дописываем элемент в правило
					RenumElement el = {};
					el.guid = elemGuid;
					el.state = state;
					rules.Get(definitions[j].guid).elemts.Push(el);
				} // ReNumGetRule
			} // definitions[j].description.Contains("Renum")
		}//for
	}//ACAPI_Element_GetPropertyDefinitions
	return err;
}// ReNum_GetElement

GSErrCode ReNumOneRule(const RenumRule& rule) {
	GSErrCode					err = NoError;
	GS::Array<RenumElement>		elemArray = rule.elemts;
	API_PropertyDefinition		position = rule.position; // В это свойство ставим позицию
	API_PropertyDefinition		sort = rule.sort;
	Delimetr delimetrList;
	bool hasdelimetr = (rule.delimetr.guid != APINULLGuid);
	std::string delimetr = "";
	// Итак, у нас есть - свойство для разбивки и свойство для критерия.
	// Поскольку чтение свойств - затратный процесс - прочитаем значение в массив и далее будем работать с ним
	GS::Array<API_PropertyDefinition> definitions;
	definitions.Push(rule.criteria);
	if (hasdelimetr) definitions.Push(rule.delimetr);
	// Собираем значения свойств из criteria. Нам нужны только уникальные значения.
	for (UInt32 i = 0; i < elemArray.GetSize(); i++) {
		GS::Array<API_Property>  properties;
		err = ACAPI_Element_GetPropertyValues(elemArray[i].guid, definitions, properties);
		if (err != NoError) msg_rep("ReNumOneRule", "ACAPI_Element_GetPropertyValues", err, elemArray[i].guid);
		if (err == NoError) {
			elemArray[i].criteria = PropertyTestHelpers::ToString(properties[0]).ToCStr(0, MaxUSize, CC_Cyrillic).Get();
			if (hasdelimetr) {
				delimetr = PropertyTestHelpers::ToString(properties[1]).ToCStr(0, MaxUSize, CC_Cyrillic).Get();
				elemArray[i].delimetr = delimetr;
			}
			// Ну и чтоб дважды не вставать - сделаем список с уникальными значениями разделителя.
			//
			delimetrList[delimetr].inx.Push(i);
		}
	}
	// Теперь последовательно идём по словарю c разделителями, вытаскиваем оттуда guid и нумеруем
	// Тут бы применить вложенные словари, но я не умею
	for (Delimetr::iterator i = delimetrList.begin(); i != delimetrList.end(); ++i) {
		// Повторяем процедуру поиска уникальных значений, теперь для критерия
		Values criteriaList;
		GS::Array<UInt32> eleminpos = i->second.inx;
		for (UInt32 j = 0; j < eleminpos.GetSize(); j++) {
			UInt32 elem_inx = eleminpos[j];
			criteriaList[elemArray[elem_inx].criteria].guid.Push(elemArray[elem_inx].guid);
		}
		// TODO Добавить исключаемые позиции
		Int32 npos = 0;
		bool last_error = false;
		for (Values::iterator k = criteriaList.begin(); k != criteriaList.end(); ++k) {
			if (!last_error) npos = npos + 1;
			last_error = true; // Если будет хотяб одна успешная запись - здесь будет false
			GS::Array<API_Guid> eleminpos = k->second.guid;
			for (UInt32 m = 0; m < eleminpos.GetSize(); m++) {
				API_Property positionproperty = {};
				err = ACAPI_Element_GetPropertyValue(eleminpos[m], position.guid, positionproperty);
				if (err != NoError) msg_rep("ReNumOneRule", "ACAPI_Element_GetPropertyValue", err, eleminpos[m]);
				if (err == NoError) {
					positionproperty.isDefault = false;
					positionproperty.value.singleVariant.variant.intValue = npos;
					err = ACAPI_Element_SetProperty(eleminpos[m], positionproperty);
					if (err == NoError) last_error = false;
					if (err != NoError) msg_rep("ReNumOneRule", "ACAPI_Element_SetProperty", err, eleminpos[m]);
				}
			}
		}
	}
	return err;
}

// -----------------------------------------------------------------------------------------------------------------------
// Функция возвращает режим нумерации (RENUM_IGNORE, RENUM_ADD, RENUM_NORMAL) и описание свойства с правилом 
// -----------------------------------------------------------------------------------------------------------------------
bool ReNumRule(const API_Guid& elemGuid, const GS::UniString& description_string, RenumRule& paramtype) {
	bool flag = false;
	if (description_string.IsEmpty()) return flag;
	if (description_string.Contains("{") && description_string.Contains("}")) {
		GS::Array<GS::UniString>	partstring;
		API_PropertyDefinition		criteria = {};
		API_PropertyDefinition		delimetr = {};
		GSErrCode					err = NoError;
		GS::UniString paramName = description_string.GetSubstring('{', '}', 0);
		paramName.ReplaceAll("Property:", "");
		int nparam = StringSplt(paramName, ";", partstring);
		if (nparam == 0) return flag;
		err = GetPropertyDefinitionByName(elemGuid, partstring[0], criteria);
		if (err == NoError) {
			paramtype.criteria = criteria;
			flag = true;
		}
		if (nparam == 2) {
			err = GetPropertyDefinitionByName(elemGuid, partstring[1], delimetr);
			if (err == NoError) {
				paramtype.delimetr = delimetr;
				flag = true;
			}
		}
	} // description_string.Contains("Renum")
	return flag;
} // ReNumRule

// -----------------------------------------------------------------------------------------------------------------------
// Функция возвращает режим нумерации (RENUM_IGNORE, RENUM_ADD, RENUM_NORMAL) и описание свойства с правилом 
// -----------------------------------------------------------------------------------------------------------------------
Int32 ReNumGetFlag(const API_Property& propertyflag) {
	UInt32			flag = RENUM_IGNORE;
#if defined(AC_22) || defined(AC_23)
	bool isnoteval = (!propertyflag.isEvaluated);
#else
	bool isnoteval = (propertyflag.status != API_Property_HasValue);
#endif
	if (isnoteval) {
		return flag;
	}
	GS::UniString		state = "";
	API_PropertyValue	val;
	if (propertyflag.isDefault) {
		val = propertyflag.definition.defaultValue.basicValue;
	}
	else {
		val = propertyflag.value;
	}
	if (propertyflag.definition.valueType == API_PropertyBooleanValueType && val.singleVariant.variant.boolValue) {
		flag = RENUM_NORMAL;
	}
	else {
#ifdef AC_25
		state = val.singleVariant.variant.uniStringValue;
#else
		state = val.singleEnumVariant.displayVariant.uniStringValue;
#endif
		flag = RENUM_NORMAL;
		if (state.Contains("Исключить") || state.IsEmpty()) flag = RENUM_IGNORE;
		if (state.Contains("Не менять")) flag = RENUM_ADD;
		// Если флаг поднят - ищем свойство с правилом
	}
	return flag;
}

// -----------------------------------------------------------------------------------------------------------------------
// Функция возвращает режим нумерации (RENUM_IGNORE, RENUM_ADD, RENUM_NORMAL) и описание свойства с правилом 
// -----------------------------------------------------------------------------------------------------------------------
UInt32 ReNumGetRule(const API_PropertyDefinition definitionflag, const API_Guid& elemGuid, API_PropertyDefinition& propertdefyrule) {
	UInt32 flag = RENUM_IGNORE;
	if (definitionflag.description.Contains("{") && definitionflag.description.Contains("}")) {
		// Получаем значение флага
		GSErrCode err = NoError;
		GS::Array<API_PropertyDefinition> definitions;
		definitions.Push(definitionflag);
		GS::Array<API_Property>  propertyflag;
		err = ACAPI_Element_GetPropertyValues(elemGuid, definitions, propertyflag);
		if (err == NoError) {
			flag = ReNumGetFlag(propertyflag[0]);
			if (flag != RENUM_IGNORE){
				GS::UniString paramName = definitionflag.description.GetSubstring('{', '}', 0);
				paramName.ReplaceAll("Property:", "");
				err = GetPropertyDefinitionByName(elemGuid, paramName, propertdefyrule);
				if (err != NoError) flag = RENUM_IGNORE;
			}// flag != RENUM_IGNORE
		}// ACAPI_Element_GetPropertyValues
	} // description_string.Contains("Renum_flag")
	return flag;
} // ReNumGetRule



