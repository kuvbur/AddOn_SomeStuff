//------------ kuvbur 2022 ------------
#ifndef AC_22
#include	"ACAPinc.h"
#include	"APIEnvir.h"
#include	"Helpers.hpp"
#include	"ReNum.hpp"
#include	"Sync.hpp"
#include	<map>
#include	<unordered_map>

// -----------------------------------------------------------------------------------------------------------------------
// 1. Получаем список объектов, в свойствах которых ищем
//		Флаг включения нумерации в формате
//					Renum_flag{*имя свойства с правилом*}
//					Renum_flag{*имя свойства с правилом*; NULL}
//					Renum_flag{*имя свойства с правилом*; ALLNULL}
//					Renum_flag{*имя свойства с правилом*; SPACE}
//					Renum_flag{*имя свойства с правилом*; ALLSPACE}
//					Тип данных свойства-флага: Набор параметров с вариантами "Включить", "Исключить", "Не менять"
//		Правило нумерации в одном из форматов
//	 				Renum{*имя свойства-критерия*}
//					Renum{*имя свойства-критерия*; *имя свойства-разбивки*}
// 2. Записываем для каждого элемента в структуру свойства, участвующие в правиле
// 3. Откидываем все с вфключенным флагом
// 4. По количеству уникальных имён свойств-правил, взятых из Renum_flag{*имя свойства с правилом*}, разбиваем элементы
// -----------------------------------------------------------------------------------------------------------------------
GSErrCode ReNumSelected (SyncSettings& syncSettings)
{
    GS::UniString funcname ("Numbering");
    GS::Int32 nPhase = 1;
#if defined(AC_27) || defined(AC_28)
    ACAPI_ProcessWindow_InitProcessWindow (&funcname, &nPhase);
#else
    ACAPI_Interface (APIIo_InitProcessWindowID, &funcname, &nPhase);
#endif
    long time_start = clock ();
    GS::Array<API_Guid> guidArray = GetSelectedElements (true, false, syncSettings, true);
    if (guidArray.IsEmpty ()) return NoError;
    GS::HashTable<API_Guid, API_PropertyDefinition> rule_definitions;
    if (guidArray.GetSize () == 1) {
        if (GetRenumRuleFromSelected (guidArray[0], rule_definitions)) GetElementForPropertyDefinition (rule_definitions, guidArray);
    }
    GS::UniString undoString = RSGetIndString (ID_ADDON_STRINGS + isEng (), UndoReNumId, ACAPI_GetOwnResModule ());
    bool flag_write = true;
    ACAPI_CallUndoableCommand (undoString, [&]() -> GSErrCode {
        ParamDictElement paramToWriteelem;
        if (!GetRenumElements (guidArray, paramToWriteelem, rule_definitions)) {
            flag_write = false;
            msg_rep ("ReNumSelected", GS::UniString::Printf ("Qty elements - %d ", guidArray.GetSize ()) + "No data to write", NoError, APINULLGuid);
#if defined(AC_27) || defined(AC_28)
            ACAPI_ProcessWindow_CloseProcessWindow ();
#else
            ACAPI_Interface (APIIo_CloseProcessWindowID, nullptr, nullptr);
#endif
            return NoError;
        }
        GS::UniString subtitle = GS::UniString::Printf ("Writing data to %d elements", paramToWriteelem.GetSize ()); short i = 2;
#if defined(AC_27) || defined(AC_28)
        bool showPercent = false;
        Int32 maxval = 2;
        ACAPI_ProcessWindow_SetNextProcessPhase (&subtitle, &maxval, &showPercent);
#else
        ACAPI_Interface (APIIo_SetNextProcessPhaseID, &subtitle, &i);
#endif
        ParamHelpers::ElementsWrite (paramToWriteelem);
        long time_end = clock ();
        GS::UniString time = GS::UniString::Printf (" %d s", (time_end - time_start) / 1000);
        GS::UniString intString = GS::UniString::Printf ("Qty elements - %d ", guidArray.GetSize ()) + GS::UniString::Printf ("wrtite to - %d", paramToWriteelem.GetSize ()) + time;
        msg_rep ("ReNumSelected", intString, NoError, APINULLGuid);
#if defined(AC_27) || defined(AC_28)
        ACAPI_ProcessWindow_CloseProcessWindow ();
#else
        ACAPI_Interface (APIIo_CloseProcessWindowID, nullptr, nullptr);
#endif
        return NoError;
    });
    if (flag_write) {
        ClassificationFunc::SystemDict systemdict;
        SyncArray (syncSettings, guidArray, systemdict);
    }
    return NoError;
}

// -----------------------------------------------------------------------------------------------------------------------
// В случае, если выбран только один элемент - обрабатываем ТОЛЬКО правила, видимые у него
// Функция для сбора правил с элемента
// -----------------------------------------------------------------------------------------------------------------------
bool GetRenumRuleFromSelected (const API_Guid& elemguid, GS::HashTable<API_Guid, API_PropertyDefinition>& definitions)
{
#if defined(AC_22)
    return false;
#else
    GS::Array<API_PropertyDefinition> definitions_;
    GSErrCode err = ACAPI_Element_GetPropertyDefinitions (elemguid, API_PropertyDefinitionFilter_UserDefined, definitions_);
    if (err == NoError && !definitions_.IsEmpty ()) {
        for (UInt32 i = 0; i < definitions_.GetSize (); i++) {
            if (!definitions_[i].description.IsEmpty ()) {
                if (definitions_[i].description.Contains ("Renum_flag")) {
                    if (!definitions.ContainsKey (definitions_[i].guid)) definitions.Add (definitions_[i].guid, definitions_[i]);
                }
            }
        }
    }
    return (!definitions.IsEmpty ());
#endif
}

// -----------------------------------------------------------------------------------------------------------------------
// Функция для выбора элементов, в которых видимо выбранное свойство
// -----------------------------------------------------------------------------------------------------------------------
void GetElementForPropertyDefinition (const GS::HashTable<API_Guid, API_PropertyDefinition>& definitions, GS::Array<API_Guid>& guidArray)
{
#if defined(AC_22)
    return;
#else
    for (auto& cIt : definitions) {
#if defined(AC_28)
        API_PropertyDefinition definition = cIt.value;
#else
        API_PropertyDefinition definition = *cIt.value;
#endif
        for (UInt32 i = 0; i < definition.availability.GetSize (); i++) {
            GS::Array<API_Guid> elemGuids;
            API_Guid classificationItemGuid = definition.availability[i];
            if (ACAPI_Element_GetElementsWithClassification (classificationItemGuid, elemGuids) == NoError) {
                for (UInt32 j = 0; j < elemGuids.GetSize (); j++) {
                    if (ACAPI_Element_Filter (elemGuids[j], APIFilt_OnVisLayer)) guidArray.Push (elemGuids[j]);
                }
            }
        }
    }
#endif
}

bool GetRenumElements (GS::Array<API_Guid> guidArray, ParamDictElement& paramToWriteelem, GS::HashTable<API_Guid, API_PropertyDefinition>& rule_definitions)
{
    // Получаем список правил суммирования
    Rules rules;
    ParamDictElement paramToReadelem;
    bool hasRule = !rule_definitions.IsEmpty ();
    GS::UniString subtitle = GS::UniString::Printf ("Reading data from %d elements", guidArray.GetSize ());
    for (UInt32 i = 0; i < guidArray.GetSize (); i++) {
        GS::Array<API_PropertyDefinition>	definitions;
        GSErrCode err = ACAPI_Element_GetPropertyDefinitions (guidArray[i], API_PropertyDefinitionFilter_UserDefined, definitions);
        bool hasDef = false;
        if (err == NoError && !definitions.IsEmpty ()) {
            if (hasRule) {
                for (UInt32 i = 0; i < definitions.GetSize (); i++) {
                    if (!definitions[i].description.IsEmpty ()) {
                        if (definitions[i].description.Contains ("Renum_flag")) {
                            if (!rule_definitions.ContainsKey (definitions[i].guid)) {
                                definitions.Delete (i);
                            } else {
                                hasDef = true;
                            }
                        }
                    }
                }
            } else {
                hasDef = ReNumHasFlag (definitions);
            }
        }
        if (hasDef) {
#if defined(AC_27) || defined(AC_28)
            bool showPercent = true;
            Int32 maxval = guidArray.GetSize ();
            ACAPI_ProcessWindow_SetNextProcessPhase (&subtitle, &maxval, &showPercent);
#else
            ACAPI_Interface (APIIo_SetNextProcessPhaseID, &subtitle, &i);
#endif
#if defined(AC_27) || defined(AC_28)
            if (ACAPI_ProcessWindow_IsProcessCanceled ()) return false;
#else
            if (ACAPI_Interface (APIIo_IsProcessCanceledID, nullptr, nullptr)) return false;
#endif
            ParamDictValue propertyParams;
            ParamDictValue paramToRead;
            ParamHelpers::AllPropertyDefinitionToParamDict (propertyParams, definitions);
            if (ReNum_GetElement (guidArray[i], propertyParams, paramToRead, rules)) ParamHelpers::AddParamDictValue2ParamDictElement (guidArray[i], paramToRead, paramToReadelem);
        }
    }
    if (paramToReadelem.IsEmpty () || rules.IsEmpty ()) return false;
    ParamDictValue propertyParams; // Все свойства уже считаны, поэтому словарь просто пустой
    ClassificationFunc::SystemDict systemdict;
    ParamHelpers::ElementsRead (paramToReadelem, propertyParams, systemdict); // Читаем значения

    // Теперь выясняем - какой режим нумерации у элементов и распределяем позиции
    for (GS::HashTable<API_Guid, RenumRule>::PairIterator cIt = rules.EnumeratePairs (); cIt != NULL; ++cIt) {
#if defined(AC_28)
        const RenumRule& rule = cIt->value;
#else
        const RenumRule& rule = *cIt->value;
#endif
        if (!rule.elemts.IsEmpty ()) ReNumOneRule (rule, paramToReadelem, paramToWriteelem);
    }
    return !paramToWriteelem.IsEmpty ();
}

// -----------------------------------------------------------------------------------------------------------------------
// Функция распределяет элемент в таблицу с правилами нумерации
// -----------------------------------------------------------------------------------------------------------------------
bool ReNum_GetElement (const API_Guid& elemGuid, ParamDictValue& propertyParams, ParamDictValue& paramToRead, Rules& rules)
{
    bool hasRenum = false;
    for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = propertyParams.EnumeratePairs (); cIt != NULL; ++cIt) {
        bool flag = false;
#if defined(AC_28)
        ParamValue& param = cIt->value;
#else
        ParamValue& param = *cIt->value;
#endif
        API_PropertyDefinition& definition = param.definition;
        if (definition.description.Contains ("Renum_flag") && definition.description.Contains ("{") && definition.description.Contains ("}")) {
            if (!rules.ContainsKey (definition.guid)) {
                RenumRule rulecritetia = {};

                // Разбираем - что записано в свойстве с флагом
                // В нём должно быть имя свойства и, возможно, флаг добавления нулей
                GS::UniString paramName = definition.description.ToLowerCase ();
                GS::Array<GS::UniString> partstring;
                if (StringSplt (paramName, "}", partstring, "enum_flag") > 0) {
                    paramName = partstring[0] + "}";
                }
                paramName = paramName.GetSubstring ('{', '}', 0);
                paramName.ReplaceAll ("\\/", "/");
                GS::UniString rawNameposition = "{@";
                if (paramName.Contains (";")) { // Есть указание на нули
                    GS::Array<GS::UniString>	partstring;
                    int nparam = StringSplt (paramName, ";", partstring);
                    rawNameposition = rawNameposition + partstring[0] + "}";
                    if (nparam > 1) {
                        if (partstring[1] == "null") rulecritetia.nulltype = ADDZEROS;
                        if (partstring[1] == "allnull") rulecritetia.nulltype = ADDMAXZEROS;
                        if (partstring[1] == "space") rulecritetia.nulltype = ADDSPACE;
                        if (partstring[1] == "allspace") rulecritetia.nulltype = ADDMAXSPACE;
                    };
                } else {
                    rawNameposition = rawNameposition + paramName + "}";
                }
                if (!rawNameposition.Contains ("property")) rawNameposition.ReplaceAll ("{", "{@gdl:");

                // Проверяем - есть ли у объекта такое свойство-правило
                if (propertyParams.ContainsKey (rawNameposition)) {

                    // В описании правила может быть указано имя свойства-критерия и, возможно, имя свойства-разбивки
                    GS::UniString ruleparamName = propertyParams.Get (rawNameposition).definition.description;
                    ruleparamName.ReplaceAll ("\\/", "/");
                    if (ruleparamName.Contains ("Renum") && ruleparamName.Contains ("{") && ruleparamName.Contains ("}")) {
                        GS::Array<GS::UniString> partstring;
                        if (StringSplt (ruleparamName, "}", partstring, "enum") > 0) {
                            ruleparamName = partstring[0] + "}";
                        }
                        ruleparamName = ruleparamName.ToLowerCase ().GetSubstring ('{', '}', 0);
                        GS::UniString rawNamecriteria = "{@";
                        GS::UniString rawNamedelimetr = "";
                        if (ruleparamName.Contains (";")) { // Есть указание на нули
                            GS::Array<GS::UniString>	partstring;
                            int nparam = StringSplt (ruleparamName, ";", partstring);
                            rawNamecriteria = rawNamecriteria + partstring[0] + "}";
                            if (nparam > 1) rawNamedelimetr = "{@" + rawNamedelimetr + partstring[1] + "}";
                        } else {
                            rawNamecriteria = rawNamecriteria + ruleparamName + "}";
                        }
                        if (!rawNamecriteria.Contains ("property")) rawNamecriteria.ReplaceAll ("{", "{@gdl:");
                        if (!rawNamedelimetr.IsEmpty () && !rawNamedelimetr.Contains ("property")) rawNamedelimetr.ReplaceAll ("{", "{@gdl:");

                        // Если такие свойства есть - записываем правило
                        if (propertyParams.ContainsKey (rawNamecriteria) && (propertyParams.ContainsKey (rawNamedelimetr) || rawNamedelimetr.IsEmpty ())) {
                            rulecritetia.state = true;
                            //if (definition.valueType != API_PropertyBooleanValueType)
                            rulecritetia.oldalgoritm = false;
                            rulecritetia.position = rawNameposition;
                            rulecritetia.flag = param.rawName;
                            rulecritetia.criteria = rawNamecriteria;
                            rulecritetia.delimetr = rawNamedelimetr;
                            rulecritetia.guid = definition.guid;
                            flag = true;
                        }
                    }
                }
                rules.Add (definition.guid, rulecritetia);
            } else {
                flag = true; // Правило уже существует, просто добавим свойства в словарь чтения и id в правила
            }
            if (flag) {
                hasRenum = true;
                rules.Get (definition.guid).elemts.Push (elemGuid);
                RenumRule& rulecritetia = rules.Get (definition.guid);
                if (!rulecritetia.position.IsEmpty ()) ParamHelpers::AddParamValue2ParamDict (elemGuid, propertyParams.Get (rulecritetia.position), paramToRead);
                if (!rulecritetia.flag.IsEmpty ()) ParamHelpers::AddParamValue2ParamDict (elemGuid, propertyParams.Get (rulecritetia.flag), paramToRead);
                if (!rulecritetia.criteria.IsEmpty ()) ParamHelpers::AddParamValue2ParamDict (elemGuid, propertyParams.Get (rulecritetia.criteria), paramToRead);
                if (!rulecritetia.delimetr.IsEmpty ()) ParamHelpers::AddParamValue2ParamDict (elemGuid, propertyParams.Get (rulecritetia.delimetr), paramToRead);
            }
        }
    }
    return hasRenum;
}

void GetCountPos (const GS::Array<RenumPos>& eleminpos, std::map<std::string, int>& npos)
{
    for (UInt32 j = 0; j < eleminpos.GetSize (); j++) {
        std::string pos = eleminpos[j].strpos;
        if (npos.count (pos) != 0) {
            npos[pos] = npos[pos] + 1;
        } else {
            npos[pos] = 1;
        }
    }
}

// Возвращает самое часто встречающееся значение позиции
// Необходима для подбора элементов с одинаковым критерием, но разной позицией
RenumPos GetMostFrequentPos (const GS::Array<RenumPos>& eleminpos)
{
    std::map<std::string, int> npos; // Словарь для подсчёта
    UInt32 inx = 0;
    int maxcont = 0;
    for (UInt32 j = 0; j < eleminpos.GetSize (); j++) {
        std::string pos = eleminpos[j].strpos;
        if (npos.count (pos) != 0) {
            int countpos = npos[pos] + 1;
            npos[pos] = countpos;
            if (countpos > maxcont) {
                maxcont = countpos;
                inx = j;
            }
        } else {
            npos[pos] = 1;
        }
    }
    RenumPos out = eleminpos.Get (inx);
    return out;
}

RenumPos GetPos (DRenumPosDict& unicpos, DStringDict& unicriteria, const std::string& delimetr, const std::string& criteria)
{
    RenumPos pos (1);
    while (unicpos[delimetr].count (pos.strpos) != 0) {
        pos.Add (1);
    }
    unicpos[delimetr][pos.strpos] = criteria;
    unicriteria[delimetr][criteria] = pos;
    return pos;
}

void ReNumOneRule (const RenumRule& rule, ParamDictElement& paramToReadelem, ParamDictElement& paramToWriteelem)
{

    // Рассортируем элементы по разделителю, типу нумерации и критерию.
    Delimetr delimetrList;
    if (!ElementsSeparation (rule, paramToReadelem, delimetrList)) return;

    DRenumPosDict unicpos;				// Словарь соответствия позиции критерию
    DStringDict unicriteria;			// Словарь соответствия критерия поизции (обратный предыдущему)
    std::map<std::string, RenumPos> maxpos;	// Словарь максимальных позиций
    RenumPos maxposall;

    //Определим часто встречающиеся позиции для критериев. Ищем только в игнорируемых и добавленных
    if (!rule.oldalgoritm) {
        for (Delimetr::iterator i = delimetrList.begin (); i != delimetrList.end (); ++i) {
            TypeValues& tv = i->second;
            std::string delimetr = i->first;
            RenumPos maxposdelim;
            if (tv.count (RENUM_IGNORE) != 0) {
                for (Values::iterator k = tv[RENUM_IGNORE].begin (); k != tv[RENUM_IGNORE].end (); ++k) {
                    GS::Array<RenumPos> eleminpos = k->second.elements;
                    std::string criteria = k->first;
                    RenumPos pos = GetMostFrequentPos (eleminpos);
                    unicriteria[delimetr][criteria] = pos;
                    unicpos[delimetr][pos.strpos] = criteria;

                    // Игнорируемые позиции нельзя занимать. Добавим их в словарь
                    for (UInt32 j = 0; j < eleminpos.GetSize (); j++) {
                        if (pos.strpos != eleminpos[j].strpos) unicpos[delimetr][eleminpos[j].strpos] = criteria;
                    }
                    maxposdelim.SetToMax (pos);
                }
            }
            if (tv.count (RENUM_ADD) != 0) {
                for (Values::iterator k = tv[RENUM_ADD].begin (); k != tv[RENUM_ADD].end (); ++k) {
                    GS::Array<RenumPos> eleminpos = k->second.elements;
                    std::string criteria = k->first;

                    // Отбираем элементы, не встретившиеся прежде в игнорируемых
                    if (unicriteria[delimetr].count (criteria) == 0) { // Если такой критерий уже есть в словаре - значит для него есть подходящая позиция
                        RenumPos pos = GetMostFrequentPos (eleminpos);
                        unicriteria[delimetr][criteria] = pos;
                    }
                }
            }
            maxposall.SetToMax (maxposdelim);
            maxpos[delimetr] = maxposdelim;
        }
    }

    // Предолагаемое поведение:
    // Игнорируемые позиции (RENUM_IGNORE) - не меняют значения.
    //						Остальные элементы, при совпадении критерия, могут принимать значения подходящих игнорируемых.
    //						Позиции игнорируемых могут совпадать.
    // Добавочные позиции (RENUM_ADD) - меняют значения в случаях:
    //						Сначала проверяем по критерию ищем подходящую позацию среди игнорируемых.
    //						В качестве подходящей для назначения выбирается самая часто встречающаяся позиция.
    // Новые позиции (RENUM_NORMAL)
    //						Сначала идёт поиск по подходящим позициям предыдущих типов. Если не нашли - ищем по-порядку свободную позицию.
    // Если критерий элемента совпадает с подходящим критерием игнорируемого - будет применена позиция игнорируемого

    //Теперь последовательно идём по словарю c разделителями, вытаскиваем оттуда guid и нумеруем
    for (Delimetr::iterator i = delimetrList.begin (); i != delimetrList.end (); ++i) {
        TypeValues& tv = i->second;
        std::string delimetr = i->first;
        RenumPos maxposdelim;

        // Получаем позиции добавляемых элементов
        if (tv.count (RENUM_ADD) != 0 && !rule.oldalgoritm) {
            for (Values::iterator k = tv[RENUM_ADD].begin (); k != tv[RENUM_ADD].end (); ++k) {
                std::string criteria = k->first;
                GS::Array<RenumPos> eleminpos = k->second.elements;

                // Расставляем позиции для элементов, для которых есть подходящая по критериям позиция
                if (unicriteria[delimetr].count (criteria) != 0) {
                    delimetrList[delimetr][RENUM_ADD][criteria].mostFrequentPos = unicriteria[delimetr][criteria];
                } else {

                    // Если такой позиции нет (например, она занята) - создадим новую позицию
                    RenumPos pos = GetPos (unicpos, unicriteria, delimetr, criteria);
                    delimetrList[delimetr][RENUM_ADD][criteria].mostFrequentPos = pos;
                    maxposdelim.SetToMax (pos);
                }
            }
        }

        // Получаем позиции новых элементов
        if (tv.count (RENUM_NORMAL) != 0) {
            for (Values::iterator k = tv[RENUM_NORMAL].begin (); k != tv[RENUM_NORMAL].end (); ++k) {
                std::string criteria = k->first;
                GS::Array<RenumPos> eleminpos = k->second.elements;

                // Расставляем позиции для элементов, для которых есть подходящая по критериям позиция
                if (unicriteria[delimetr].count (criteria) != 0) {
                    delimetrList[delimetr][RENUM_NORMAL][criteria].mostFrequentPos = unicriteria[delimetr][criteria];
                } else {

                    // Если такой позиции нет (например, она занята) - создадим новую позицию
                    RenumPos pos = GetPos (unicpos, unicriteria, delimetr, criteria);
                    delimetrList[delimetr][RENUM_NORMAL][criteria].mostFrequentPos = pos;
                    maxposdelim.SetToMax (pos);
                }
            }
        }
        maxposall.SetToMax (maxposdelim);
        maxpos[delimetr] = maxposdelim;
    }

    //Финишная прямая. Берём позиции из словаря и расставляем значения.
    GS::UniString rawname_position = rule.position;
    RenumPos maxposdelim;
    if (rule.nulltype == ADDMAXZEROS || rule.nulltype == ADDMAXSPACE) maxposdelim = maxposall;
    for (Delimetr::iterator i = delimetrList.begin (); i != delimetrList.end (); ++i) {
        TypeValues& tv = i->second;
        for (short renumType = RENUM_ADD; renumType <= RENUM_NORMAL; renumType++) {
            if (tv.count (renumType) != 0) {
                if (rule.nulltype == ADDZEROS || rule.nulltype == ADDSPACE) maxposdelim = maxpos[i->first];
                for (Values::iterator k = tv[renumType].begin (); k != tv[renumType].end (); ++k) {
                    std::string criteria = k->first;
                    GS::Array<RenumPos> eleminpos = k->second.elements;
                    RenumPos pos = k->second.mostFrequentPos;
                    pos.FormatToMax (maxposdelim, rule.nulltype);
                    ParamValue posvalue = pos.ToParamValue (rawname_position);
                    for (UInt32 j = 0; j < eleminpos.GetSize (); j++) {
                        ParamValue paramposition = paramToReadelem.Get (eleminpos[j].guid).Get (rawname_position);
                        paramposition.isValid = true;
                        posvalue.val.type = paramposition.val.type;

                        // Записываем только изменённые значения
                        if (paramposition != posvalue) {
                            paramposition.val = posvalue.val;
                            ParamHelpers::AddParamValue2ParamDictElement (eleminpos[j].guid, paramposition, paramToWriteelem);
                        }
                    }
                }
            }
        }
    }
    return;
}

bool ElementsSeparation (const RenumRule& rule, const  ParamDictElement& paramToReadelem, Delimetr& delimetrList)
{
    if (!rule.state) return false;
    GS::Array<API_Guid> elemArray = rule.elemts;
    GS::Array<API_Guid> elemrenumArray;
    bool needAddSet = false;
    bool flag = false;

    // Собираем значения свойств из criteria. Нам нужны только уникальные значения.
    for (UInt32 i = 0; i < elemArray.GetSize (); i++) {
        if (paramToReadelem.ContainsKey (elemArray[i])) {

            // Сразу проверим режим нумерации элемента
            short state = RENUM_SKIP;
            RenumPos pos;
            ParamDictValue params = paramToReadelem.Get (elemArray[i]);
            if (params.ContainsKey (rule.flag) && params.ContainsKey (rule.position)) {
                ParamValue paramflag = params.Get (rule.flag);
                ParamValue paramposition = params.Get (rule.position);
                state = ReNumGetFlag (paramflag, paramposition);

                // Получаем позицию, если она есть
                if (paramposition.isValid && state != RENUM_SKIP) pos = RenumPos (paramposition);
                if (state != RENUM_IGNORE && state != RENUM_SKIP) needAddSet = true;
            }
            if (state != RENUM_IGNORE && state != RENUM_SKIP && !rule.oldalgoritm) {
                elemrenumArray.Push (elemArray[i]);

                // Проверим - не нумеровался ли прежде этот элемент. Если нет - ставим флаг перенумерации
                //if (!rule.exselemts.IsEmpty() && !rule.exselemts.Contains(elemArray[i])) state = RENUM_NORMAL;
            }

            if (state != RENUM_SKIP) {

                // Получаем разделитель, если он есть
                std::string delimetr = "";
                if (params.ContainsKey (rule.delimetr)) {
                    ParamValue param = params.Get (rule.delimetr);
                    if (param.isValid) delimetr = param.val.uniStringValue.ToCStr (0, MaxUSize, GChCode).Get ();
                }

                // Получаем критерий, если он есть
                std::string criteria = "";
                if (params.ContainsKey (rule.criteria)) {
                    ParamValue param = params.Get (rule.criteria);
                    if (param.isValid) criteria = param.val.uniStringValue.ToCStr (0, MaxUSize, GChCode).Get ();
                }
                if (state != RENUM_SKIP) {
                    if (delimetrList.count (delimetr) == 0) delimetrList[delimetr] = {};
                    if (delimetrList[delimetr].count (state) == 0) delimetrList[delimetr][state] = {};
                    if (delimetrList[delimetr][state].count (criteria) == 0) delimetrList[delimetr][state][criteria] = {};
                    delimetrList[delimetr][state][criteria].elements.Push (pos);
                    flag = true;
                }
            }
        }
    }
    return flag;
}

//--------------------------------------------------------------------------------------------------------------------------
// Проверяет - есть ли хоть одно описание флага
//--------------------------------------------------------------------------------------------------------------------------
bool ReNumHasFlag (const GS::Array<API_PropertyDefinition> definitions)
{
    if (definitions.IsEmpty ()) return false;
    for (UInt32 i = 0; i < definitions.GetSize (); i++) {
        if (!definitions[i].description.IsEmpty ()) {
            if (definitions[i].description.Contains ("Renum_flag")) {
                return true;
            }
        }
    }
    return false;
}

// -----------------------------------------------------------------------------------------------------------------------
// Функция возвращает режим нумерации (RENUM_IGNORE, RENUM_ADD, RENUM_NORMAL)
// -----------------------------------------------------------------------------------------------------------------------
short ReNumGetFlag (const ParamValue& paramflag, const ParamValue& paramposition)
{
    if (!paramflag.isValid) return RENUM_SKIP;
    SyncSettings syncSettings;
    bool isEditable = IsElementEditable (paramflag.fromGuid, syncSettings, false);
    if (paramflag.type == API_PropertyBooleanValueType) {
        if (paramflag.val.boolValue) {
            if (isEditable) {
                return RENUM_NORMAL;
            } else {
                return RENUM_IGNORE;
            }
        } else {
            return RENUM_SKIP;
        }
    }
    if (paramflag.type == API_PropertyStringValueType) {
        GS::UniString flag = paramflag.val.uniStringValue.ToLowerCase ();

        // Исключаемые позиции
        GS::UniString txtypenum = RSGetIndString (ID_ADDON_STRINGS + isEng (), RenumSkipID, ACAPI_GetOwnResModule ());
        if (flag.Contains (txtypenum) || flag.Contains ("skip")) return RENUM_SKIP;

        // У нередактируемых элементов нет возможности поменять позицию - просто учтём её
        if (!isEditable) return RENUM_IGNORE;

        // Неизменные позиции
        txtypenum = RSGetIndString (ID_ADDON_STRINGS + isEng (), RenumIgnoreID, ACAPI_GetOwnResModule ());
        if (flag.Contains (txtypenum) || flag.Contains ("ignore")) return RENUM_IGNORE;

        // Пустые позиции (если строка пустая - значение ноль.)
        if (paramposition.val.intValue != 0) return RENUM_ADD;

        // Все прочие
        return RENUM_NORMAL;
    }
    return RENUM_SKIP;
}
#endif
