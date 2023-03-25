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
	GS::UniString funcname("Numbering"); short nPhase = 1;
	ACAPI_Interface(APIIo_InitProcessWindowID, &funcname, &nPhase);
	long time_start = clock();
	GS::Array<API_Guid> guidArray = GetSelectedElements(true, true, syncSettings, true);
	if (guidArray.IsEmpty()) return NoError;
	GS::UniString undoString = RSGetIndString(AddOnStringsID, UndoReNumId, ACAPI_GetOwnResModule());
	ACAPI_CallUndoableCommand(undoString, [&]() -> GSErrCode {
		ParamDictElement paramToWriteelem;
		if (!GetRenumElements(guidArray, paramToWriteelem)) {
			msg_rep("ReNumSelected", "No data to write", NoError, APINULLGuid);
			ACAPI_Interface(APIIo_CloseProcessWindowID, nullptr, nullptr);
			return NoError;
		}
		GS::UniString subtitle = GS::UniString::Printf("Writing data to %d elements", paramToWriteelem.GetSize()); short i = 2;
		ACAPI_Interface(APIIo_SetNextProcessPhaseID, &subtitle, &i);
		ParamHelpers::ElementsWrite(paramToWriteelem);
		long time_end = clock();
		GS::UniString time = GS::UniString::Printf(" %d s", (time_end - time_start) / 1000);
		GS::UniString intString = GS::UniString::Printf("Qty elements - %d ", guidArray.GetSize()) + GS::UniString::Printf("wrtite to - %d", paramToWriteelem.GetSize()) + time;
		msg_rep("ReNumSelected", intString, NoError, APINULLGuid);
		ACAPI_Interface(APIIo_CloseProcessWindowID, nullptr, nullptr);
		return NoError;
							  });
	return NoError;
}

bool GetRenumElements(const GS::Array<API_Guid> guidArray, ParamDictElement& paramToWriteelem) {

	// Получаем список правил суммирования
	Rules rules;
	ParamDictElement paramToReadelem;
	GS::UniString subtitle = GS::UniString::Printf("Reading data from %d elements", guidArray.GetSize());
	for (UInt32 i = 0; i < guidArray.GetSize(); i++) {
		GS::Array<API_PropertyDefinition>	definitions;
		GSErrCode err = ACAPI_Element_GetPropertyDefinitions(guidArray[i], API_PropertyDefinitionFilter_UserDefined, definitions);
		if (err == NoError && !definitions.IsEmpty() && ReNumHasFlag(definitions)) {
			ACAPI_Interface(APIIo_SetNextProcessPhaseID, &subtitle, &i);
			if (ACAPI_Interface(APIIo_IsProcessCanceledID, nullptr, nullptr)) {
				return false;
			}
			ParamDictValue propertyParams;
			ParamDictValue paramToRead;
			ParamHelpers::GetAllPropertyDefinitionToParamDict(propertyParams, definitions);
			if (ReNum_GetElement(guidArray[i], propertyParams, paramToRead, rules)) ParamHelpers::AddParamDictValue2ParamDictElement(guidArray[i], paramToRead, paramToReadelem);
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
bool ReNum_GetElement(const API_Guid& elemGuid, ParamDictValue& propertyParams, ParamDictValue & paramToRead, Rules& rules) {
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
				GS::UniString rawNameprefix = "{";
				if (paramName.Contains(";")) { // Есть указание на нули
					GS::Array<GS::UniString>	partstring;
					int nparam = StringSplt(paramName, ";", partstring);
					rawNameposition = rawNameposition + partstring[0] + "}";
					if (nparam > 1) {
						if (partstring[1] == "null") rulecritetia.nulltype = ADDZEROS;
						if (partstring[1] == "allnull") rulecritetia.nulltype = ADDMAXZEROS;
						if (rulecritetia.nulltype == NOZEROS) { // Префикс
							rawNameprefix = rawNameprefix + partstring[0] + "}";
						}
					};
					if (nparam > 2) {
						if (partstring[2] == "null") rulecritetia.nulltype = ADDZEROS;
						if (partstring[2] == "allnull") rulecritetia.nulltype = ADDMAXZEROS;
					}
				}
				else {
					rawNameposition = rawNameposition + paramName + "}";
				}
				if (!rawNameposition.Contains("property")) rawNameposition.ReplaceAll("{", "{gdl:");
				if (!rawNameprefix.IsEmpty() && !rawNameprefix.Contains("property")) rawNameprefix.ReplaceAll("{", "{gdl:");
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
							rawNamecriteria = rawNamecriteria + ruleparamName + "}";
						}
						if (!rawNamecriteria.Contains("property")) rawNamecriteria.ReplaceAll("{", "{gdl:");
						if (!rawNamedelimetr.IsEmpty() && !rawNamedelimetr.Contains("property")) rawNamedelimetr.ReplaceAll("{", "{gdl:");
						// Если такие свойства есть - записываем правило
						if (propertyParams.ContainsKey(rawNamecriteria) && (propertyParams.ContainsKey(rawNamedelimetr) || rawNamedelimetr.IsEmpty()) && (propertyParams.ContainsKey(rawNamedelimetr) || rawNamedelimetr.IsEmpty())) {
							rulecritetia.state = true;
							rulecritetia.position = rawNameposition;
							rulecritetia.flag = param.rawName;
							rulecritetia.criteria = rawNamecriteria;
							rulecritetia.delimetr = rawNamedelimetr;
							rulecritetia.prefix = rawNameprefix;
							flag = true;
						}
					}
				}
				rules.Add(definition.guid, rulecritetia);
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
				if (!rulecritetia.prefix.IsEmpty()) ParamHelpers::AddParamValue2ParamDictElement(elemGuid, propertyParams.Get(rulecritetia.prefix), paramToReadelem);
			}
		}
	}
	return;
}

void ReNumOneRule(const RenumRule& rule, ParamDictElement& paramToReadelem, ParamDictElement& paramToWriteelem) {
	if (!rule.state) return;
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
					if (param.isValid) delimetr = param.val.uniStringValue.ToCStr(0, MaxUSize, GChCode).Get();
				}

				// Получаем критерий, если он есть
				std::string criteria = "";
				if (params.ContainsKey(rule.criteria)) {
					ParamValue param = params.Get(rule.criteria);
					if (param.isValid) criteria = param.val.uniStringValue.ToCStr(0, MaxUSize, GChCode).Get();
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
		std::map<std::string, int, doj::alphanum_less<std::string> > unicpos;
		std::map<std::string, std::string, doj::alphanum_less<std::string> > unicriteria;
		TypeValues& tv = i->second;

		// Проверим - есть ли элементы с исключаемыми позициями
		bool hasAdd = (tv.count(RENUM_ADD) != 0);
		bool hasNormal = (tv.count(RENUM_NORMAL) != 0);

		// Все позиции с неизменной нумерацией должны иметь один номер. Если нет - меняем его сами.
		if (hasAdd) {
			for (Values::iterator k = tv[RENUM_ADD].begin(); k != tv[RENUM_ADD].end(); ++k) {
				GS::Array<API_Guid> eleminpos = k->second.guid;
				ParamValue posvalue = paramToReadelem.Get(eleminpos[0]).Get(rule.position);
				std::string pos = posvalue.val.uniStringValue.ToCStr(0, MaxUSize, GChCode).Get();
				unicpos[pos] = posvalue.val.intValue;
				unicriteria[k->first] = pos;
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
		hasAdd = !unicpos.empty() && !unicriteria.empty();
		if (hasNormal) {
			int maxpos = 1;
			int npos = 1;
			for (Values::iterator k = tv[RENUM_NORMAL].begin(); k != tv[RENUM_NORMAL].end(); ++k) {
				std::string criteria = k->first;
				std::string pos = "1";

				//Поищем среди неизменяемых позиций - есть ли позиции с такуим критерием
				if (hasAdd) {
					if (unicriteria.count(criteria) > 0) { // Есть существующая позиция с таким критерием
						pos = unicriteria[criteria];
					}
					else { // Ищем свободную позицию
						while (unicpos.count(pos) > 0)
						{
							npos += 1;
							pos = std::to_string(npos);
						}
						unicpos[pos] = npos;
					}
				}
				else {
					npos += 1; // Просто всё перенумеруем
					pos = std::to_string(npos);
				}
				unicriteria[criteria] = pos;
				maxpos = max(maxpos, static_cast<int>(pos.length()));
			}
			if (!unicriteria.empty()) {
				for (auto const& ent1 : unicriteria) {
					std::string criteria = ent1.first;
					if (tv[RENUM_NORMAL].count(criteria)) {
						std::string pos = ent1.second;
						GS::UniString unipos = GS::UniString(pos.c_str(), GChCode);
						ParamValue posvalue;
						ParamHelpers::ConvValue(posvalue, rule.position, unipos);
						GS::Array<API_Guid> eleminpos = tv[RENUM_NORMAL][criteria].guid;
						for (UInt32 j = 0; j < eleminpos.GetSize(); j++) {
							ParamValue paramposition = paramToReadelem.Get(eleminpos[j]).Get(rule.position);
							paramposition.isValid = true;
							posvalue.val.type = paramposition.val.type;

							// Записываем только изменённые значения
							if (paramposition != posvalue) {
								paramposition.val = posvalue.val;
								ParamHelpers::AddParamValue2ParamDictElement(eleminpos[j], paramposition, paramToWriteelem);
							}
							ParamValue paramflag = paramToReadelem.Get(eleminpos[j]).Get(rule.flag);
							if (paramflag.type == API_PropertyStringValueType) {
								// Переключаем флаг в режим Добавить
								GS::UniString txtypenum = RSGetIndString(AddOnStringsID, RenumAddID, ACAPI_GetOwnResModule());
								if (!paramflag.val.uniStringValue.Contains(txtypenum)) {
									paramflag.val.uniStringValue = txtypenum;
									ParamHelpers::AddParamValue2ParamDictElement(eleminpos[j], paramflag, paramToWriteelem);
								}
							}
						}
					}
				}
			}
		}
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
	if (paramflag.type == API_PropertyBooleanValueType) {
		if (paramflag.val.boolValue) {
			return RENUM_NORMAL;
		}
		else {
			return RENUM_IGNORE;
		}
	}
	if (paramflag.type == API_PropertyStringValueType) {

		// Исключаемые позиции
		GS::UniString txtypenum = RSGetIndString(AddOnStringsID, RenumIgnoreID, ACAPI_GetOwnResModule());
		if (paramflag.val.uniStringValue.Contains(txtypenum)) return RENUM_IGNORE;

		// Пустые позиции (если строка пустая - значение ноль.)
		if (paramposition.val.intValue == 0) return RENUM_NORMAL;

		// Добавочные позиции
		txtypenum = RSGetIndString(AddOnStringsID, RenumAddID, ACAPI_GetOwnResModule());
		if (paramflag.val.uniStringValue.Contains(txtypenum)) return RENUM_ADD;

		// Все прочие
		return RENUM_NORMAL;
	}
	return RENUM_IGNORE;
}