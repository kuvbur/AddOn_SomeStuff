#include	"APIEnvir.h"
#include	"ACAPinc.h"
#include "ReNum.hpp"
#include "Helpers.hpp"

#define RENUM_IGNORE 0
#define RENUM_ADD 1
#define RENUM_NORMAL 2

typedef struct {
	API_Guid		guid;
	GS::UniString	val;
	int	state;
} RenumElement;

typedef struct {
	bool						state;
	API_PropertyDefinition		position;
	API_PropertyDefinition		criteria;
	API_PropertyDefinition		delimetr;
	GS::Array <RenumElement>	elemts;
} RenumRule;

typedef GS::HashTable<API_Guid, RenumRule> Rules;

// -----------------------------------------------------------------------------------------------------------------------
// Итак, задача, суть такова:
// 1. Получаем список объектов, с войствах которых ищем
//		Флаг включения нумерации в формате
//					Renum_flag{*имя свойства с правилом*}
//		Правило нумерации в одном из форматов
//					Renum{*имя свойства-критерия*; *имя свойства-разбивки*}
//	 				Renum{*имя свойства-критерия*}
// 2. Записываем для каждого элемента в структуру свойства, участвующие в правиле
// 3. Откидываем все с вфключенным флагом
// 4. По количеству уникальных имён свойств-правил, взятых из Renum_flag{*имя свойства с правилом*}, разбиваем элементы
// -----------------------------------------------------------------------------------------------------------------------

// -----------------------------------------------------------------------------------------------------------------------
// Функция возвращает режим нумерации (RENUM_IGNORE, RENUM_ADD, RENUM_NORMAL) и описание свойства с правилом 
// -----------------------------------------------------------------------------------------------------------------------
bool ReNumRule(const API_Guid& elemGuid, const GS::UniString& description_string, RenumRule& paramtype) {
	bool flag = false;
	if (description_string.IsEmpty()) return flag;
	if (description_string.Contains("{") && description_string.Contains("}")) {
		GS::Array<GS::UniString> partstring;
		GS::UniString paramName = description_string.GetSubstring('{', '}', 0);
		paramName.ReplaceAll("Property:", "");
		int nparam = StringSplt(paramName, ";", partstring);
		API_PropertyDefinition  criteria = {};
		API_PropertyDefinition  delimetr = {};
		GSErrCode err = NoError;
		GSErrCode err_1 = NoError;
		switch (nparam)
		{
		case 0:
			err = GetPropertyDefinitionByName(elemGuid, paramName, criteria);
			if (err == NoError) {
				paramtype.criteria = criteria;
				flag = true;
			}
			break;
		case 2:
			err = GetPropertyDefinitionByName(elemGuid, partstring[0], criteria);
			err_1 = GetPropertyDefinitionByName(elemGuid, partstring[1], delimetr);
			if (err == NoError && err_1 == NoError) {
				paramtype.criteria = criteria;
				paramtype.delimetr = delimetr;
				flag = true;
			}
			break;
		default:
			break;
		} // switch (nparam)
	} // description_string.Contains("Renum")
	return flag;
} // ReNumRule

// -----------------------------------------------------------------------------------------------------------------------
// Функция возвращает режим нумерации (RENUM_IGNORE, RENUM_ADD, RENUM_NORMAL) и описание свойства с правилом 
// -----------------------------------------------------------------------------------------------------------------------
UInt32 ReNumGetRule(const API_PropertyDefinition definitionflag, const API_Guid& elemGuid, API_PropertyDefinition& propertdefyrule) {
	UInt32 flag = RENUM_IGNORE;
	GS::UniString state = "";
	if (definitionflag.description.Contains("{") && definitionflag.description.Contains("}")) {
		// Получаем значение флага
		GSErrCode err = NoError;
		GS::Array<API_PropertyDefinition> definitions;
		definitions.Push(definitionflag);
		GS::Array<API_Property>  propertyflag;
		err = ACAPI_Element_GetPropertyValues(elemGuid, definitions, propertyflag);
		if (err == NoError) {
			if (propertyflag[0].isDefault) {
				state = propertyflag[0].definition.defaultValue.basicValue.singleEnumVariant.displayVariant.uniStringValue;
			}
			else
			{
				state = propertyflag[0].value.singleEnumVariant.displayVariant.uniStringValue;
			} //propertyrule.isDefault
			flag = RENUM_NORMAL;
			if (state.Contains("Исключить") || state.IsEmpty()) flag = RENUM_IGNORE;
			if (state.Contains("Не менять")) flag = RENUM_ADD;
			// Если флаг поднят - ищем свойство с правилом
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

// -----------------------------------------------------------------------------------------------------------------------
// Функция возвращает режим нумерации (RENUM_IGNORE, RENUM_ADD, RENUM_NORMAL) и описание свойства с правилом 
// -----------------------------------------------------------------------------------------------------------------------
GSErrCode ReNum_GetElement(const API_Guid& elemGuid, Rules &rules) {
	GSErrCode err = NoError;
	GS::Array<API_PropertyDefinition> definitions;
	RenumElement el = {};
	err = ACAPI_Element_GetPropertyDefinitions(elemGuid, API_PropertyDefinitionFilter_UserDefined, definitions);
	if (err == NoError) {
		for (UInt32 j = 0; j < definitions.GetSize(); j++) {
			// Является ли ствойсво описанием системы нумерации?
			if(definitions[j].description.Contains("Renum_flag")){
			/*if (!rules.ContainsKey(definitions[j].guid)) {*/
				// Проверяем содержит ли описание свойства флаг нумерации со ссылкой на правило нумерации
				API_PropertyDefinition  propertydefrule = {};
				UInt32 state = ReNumGetRule(definitions[j], elemGuid, propertydefrule);
				if (state != RENUM_IGNORE) {
					// Если содержит - заполняем словарь с правилами и словарь элементов для этого правила
					// Если, конечно, раньше его не было
					if (!rules.ContainsKey(definitions[j].guid)) {
						RenumRule rulecritetia = {};
						rulecritetia.position = propertydefrule;
						bool state = ReNumRule(elemGuid, propertydefrule.description, rulecritetia);
						rulecritetia.state = state;
						rules.Add(definitions[j].guid, rulecritetia);
					} //rules.ContainsKey(definitions[j].guid)
					// Дописываем элемен в правило
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

GSErrCode ReNum_OneRule(const RenumRule& rule) {
	GSErrCode err = NoError;
	GS::Array<RenumElement> elemArray = rule.elemts;
	API_PropertyDefinition		position= rule.position;
	API_PropertyDefinition		criteria = rule.criteria;
	API_PropertyDefinition		delimetr = rule.delimetr;

	GS::Array<API_PropertyDefinition> definitions;
	definitions.Push(criteria);

	for (UInt32 i = 0; i < elemArray.GetSize(); i++) {
		GS::Array<API_Property>  properties;
		err = ACAPI_Element_GetPropertyValues(elemArray[i].guid, definitions, properties);
		GS::UniString val = PropertyTestHelpers::ToString(properties[0]);
		elemArray[i].val = val;
	}
	// Собираем значения свойств из criteria. Нам нужны только уникальные значения.
	return err;
}

GSErrCode ReNum_Selected(void) {
	GSErrCode err = NoError;
	Rules rules;
	GS::Array<API_Guid> guidArray = GetSelectedElements(true, true);
	if (!guidArray.IsEmpty()) {
		for (UInt32 i = 0; i < guidArray.GetSize(); i++) {
			err = ReNum_GetElement(guidArray[i], rules);
		}
		if (rules.IsEmpty()) return APIERR_MISSINGCODE;
		// Теперь у нас есть списк правил. Можем пройти по каждому правилу и обработать элементы
		for (GS::HashTable<API_Guid, RenumRule>::PairIterator cIt = rules.EnumeratePairs(); cIt != NULL; ++cIt) {
			const RenumRule& rule = *cIt->value;
			if (rule.state && !rule.elemts.IsEmpty()) {
				err = ReNum_OneRule(rule);
			}
		}
	}
	rules.Clear();
	return err;
}