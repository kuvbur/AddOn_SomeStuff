//------------ kuvbur 2022 ------------
#include	<map>
#include	<unordered_map>
#include	"3dpart/alphanum.hpp"
#include	"APIEnvir.h"
#include	"ACAPinc.h"
#include	"ReNum.hpp"
#include	"Helpers.hpp"

typedef std::map<std::string, SortGUID, doj::alphanum_less<std::string> > Values; // Словарь элементов по критериям

typedef std::unordered_map <short, Values> TypeValues; // Словарь по типам нумерации

typedef std::unordered_map <std::string, TypeValues> Delimetr; // Словарь по разделителю

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

GSErrCode ReNumSelected(SyncSettings& syncSettings) {
	GS::Array<API_Guid> guidArray = GetSelectedElements(true, true, syncSettings, true);
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
	Rules rules;
	ParamDictElement paramToReadelem;
	for (UInt32 i = 0; i < guidArray.GetSize(); i++) {
		GS::Array<API_PropertyDefinition>	definitions;
		GSErrCode err = ACAPI_Element_GetPropertyDefinitions(guidArray[i], API_PropertyDefinitionFilter_UserDefined, definitions);
		if (err == NoError && !definitions.IsEmpty() && ReNumHasFlag(definitions)) {
			ParamDictValue propertyParams;
			ParamHelpers::GetAllPropertyDefinitionToParamDict(propertyParams, definitions);
			ReNum_GetElement(guidArray[i], propertyParams, paramToReadelem, rules);
		}
	}
	if (paramToReadelem.IsEmpty() || rules.IsEmpty()) return false;
	ParamDictValue propertyParams; // Все свойства уже считаны, поэтому словарь просто пустой
	ParamHelpers::ElementsRead(paramToReadelem, propertyParams); // Читаем значения

	// Теперь выясняем - какой режим нумерации у элементов и распределяем позиции
	for (GS::HashTable<API_Guid, RenumRule>::PairIterator cIt = rules.EnumeratePairs(); cIt != NULL; ++cIt) {
		const RenumRule& rule = *cIt->value;
		if (!rule.elemts.IsEmpty()) ReNumOneRule(rule, paramToReadelem, paramToWriteelem);
	}
	return !paramToWriteelem.IsEmpty();
}

// -----------------------------------------------------------------------------------------------------------------------
// Функция распределяет элемент в таблицу с правилами нумерации
// -----------------------------------------------------------------------------------------------------------------------
void ReNum_GetElement(const API_Guid& elemGuid, ParamDictValue& propertyParams, ParamDictElement& paramToReadelem, Rules& rules) {
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
					rawNameposition = rawNameposition + partstring[0] + "}";
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
						}
					}
				}
			}
			else {
				flag = true; // Правило уже существует, просто добавим свойства в словарь чтения и id в правила
			}
			if (flag) {
				rules.Get(definition.guid).elemts.Push(elemGuid);
				RenumRule& rulecritetia = rules.Get(definition.guid);
				if (!rulecritetia.position.IsEmpty()) ParamHelpers::AddParamValue2ParamDictElement(elemGuid, propertyParams.Get(rulecritetia.position), paramToReadelem);
				if (!rulecritetia.flag.IsEmpty()) ParamHelpers::AddParamValue2ParamDictElement(elemGuid, propertyParams.Get(rulecritetia.flag), paramToReadelem);
				if (!rulecritetia.criteria.IsEmpty()) ParamHelpers::AddParamValue2ParamDictElement(elemGuid, propertyParams.Get(rulecritetia.criteria), paramToReadelem);
				if (!rulecritetia.delimetr.IsEmpty()) ParamHelpers::AddParamValue2ParamDictElement(elemGuid, propertyParams.Get(rulecritetia.delimetr), paramToReadelem);
			}
		}
	}
	return;
}

void ReNumOneRule(const RenumRule& rule, ParamDictElement& paramToReadelem, ParamDictElement& paramToWriteelem) {
	GS::Array<API_Guid>		elemArray = rule.elemts;
	Delimetr delimetrList;

	// Заполняем типы нумерации
	bool hasdelimetr = !rule.delimetr.IsEmpty();

	// Собираем значения свойств из criteria. Нам нужны только уникальные значения.
	for (UInt32 i = 0; i < elemArray.GetSize(); i++) {
		if (paramToReadelem.ContainsKey(elemArray[i])) {

			// Сразу проверим режим нумерации элемента
			short state = RENUM_IGNORE;
			ParamDictValue params = paramToReadelem.Get(elemArray[i]);
			if (params.ContainsKey(rule.flag) && params.ContainsKey(rule.position)) {
				ParamValue paramflag = params.Get(rule.flag);
				ParamValue paramposition = params.Get(rule.position);
				state = ReNumGetFlag(paramflag, paramposition);
			}
			if (state != RENUM_IGNORE) {

				// Получаем разделитель, если он есть
				std::string delimetr = "";
				if (params.ContainsKey(rule.delimetr)) {
					ParamValue param = params.Get(rule.delimetr);
					if (param.isValid) delimetr = param.val.uniStringValue.ToCStr(0, MaxUSize, CC_Cyrillic).Get();
				}

				// Получаем критерий, если он есть
				std::string criteria = "";
				if (params.ContainsKey(rule.criteria)) {
					ParamValue param = params.Get(rule.criteria);
					if (param.isValid) criteria = param.val.uniStringValue.ToCStr(0, MaxUSize, CC_Cyrillic).Get();
				}
				if (delimetrList.count(delimetr) == 0) delimetrList[delimetr] = {};
				if (delimetrList[delimetr].count(state) == 0) delimetrList[delimetr][state] = {};
				if (delimetrList[delimetr][state].count(criteria) == 0) delimetrList[delimetr][state][criteria] = {};
				delimetrList[delimetr][state][criteria].guid.Push(elemArray[i]);
			}
		}
	}

	//Теперь последовательно идём по словарю c разделителями, вытаскиваем оттуда guid и нумеруем
	for (Delimetr::iterator i = delimetrList.begin(); i != delimetrList.end(); ++i) {
		TypeValues& tv = i->second;

		// Проверим - есть ли элементы с исключаемыми позициями
		bool hasAdd = (tv.count(RENUM_ADD) != 0);
		bool hasNormal = (tv.count(RENUM_NORMAL) != 0);

		if (!hasAdd && hasNormal) {
			Int32 maxnpos = static_cast<Int32>(tv[RENUM_NORMAL].size());
			Int32 npos = 0;
			GS::UniString strnpos = "";
			for (Values::iterator k = tv[RENUM_NORMAL].begin(); k != tv[RENUM_NORMAL].end(); ++k) {
				npos += 1;
				if (maxnpos < 10) strnpos = GS::UniString::Printf("%d", npos);
				if (maxnpos < 100 && strnpos.IsEmpty()) strnpos = GS::UniString::Printf("%02d", npos);
				if (maxnpos < 1000 && strnpos.IsEmpty()) strnpos = GS::UniString::Printf("%03d", npos);
				ParamValue posvalue;
				ParamHelpers::ConvValue(posvalue, rule.position, npos);
				posvalue.val.uniStringValue = strnpos;
				GS::Array<API_Guid> eleminpos = k->second.guid;
				for (UInt32 j = 0; j < eleminpos.GetSize(); j++) {
					ParamValue paramposition = paramToReadelem.Get(eleminpos[j]).Get(rule.position);
					paramposition.isValid = true;
					posvalue.val.type = paramposition.val.type;

					// Записываем только изменённые значения
					if (paramposition != posvalue) {
						paramposition.val = posvalue.val;
						ParamHelpers::AddParamValue2ParamDictElement(eleminpos[j], paramposition, paramToWriteelem);
					}
				}
			}
		}

		// Если есть - то нужно будет сравнивать каждый элемент из
	}
	return;
}

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
short ReNumGetFlag(const ParamValue& paramflag, const ParamValue& paramposition) {
	if (!paramflag.isValid) return RENUM_IGNORE;
	short flag = RENUM_IGNORE;
	if (paramflag.type == API_PropertyBooleanValueType) {
		if (paramflag.val.boolValue) flag = RENUM_NORMAL;
	}
	if (paramflag.type == API_PropertyStringValueType) {
		flag = RENUM_NORMAL;
		GS::UniString txtypenum = RSGetIndString(AddOnStringsID, RenumIgnoreID, ACAPI_GetOwnResModule());
		if (paramflag.val.uniStringValue.Contains(txtypenum) || paramflag.val.uniStringValue.IsEmpty()) flag = RENUM_IGNORE;

		txtypenum = RSGetIndString(AddOnStringsID, RenumAddID, ACAPI_GetOwnResModule());
		if (!paramposition.val.uniStringValue.IsEmpty() && paramflag.val.uniStringValue.Contains(txtypenum)) flag = RENUM_ADD;
	}
	return flag;
}