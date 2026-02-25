//------------ kuvbur 2022 ------------
#ifndef AC_22
#include	"ACAPinc.h"
#include	"APIEnvir.h"
#include	"Helpers.hpp"
#include	"Propertycache.hpp"
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
    #if defined(AC_27) || defined(AC_28) || defined(AC_29)
    ACAPI_ProcessWindow_InitProcessWindow (&funcname, &nPhase);
    #else
    ACAPI_Interface (APIIo_InitProcessWindowID, &funcname, &nPhase);
    #endif
    clock_t start, finish;
    double  duration;
    start = clock ();
    GS::Array<API_Guid> guidArray = GetSelectedElements (true, false, syncSettings, true, false, false);
    if (guidArray.IsEmpty ()) return NoError;
    GS::HashTable<API_Guid, API_PropertyDefinition> rule_definitions = {};
    if (!GetRuleFromSelected (guidArray, rule_definitions, "Renum_flag", false)) {
        msg_rep ("SumSelected", "No Num rule found. Check that the description of the user property contains Renum_flag", NoError, APINULLGuid);
        #if defined(AC_27) || defined(AC_28) || defined(AC_29)
        ACAPI_ProcessWindow_CloseProcessWindow ();
        #else
        ACAPI_Interface (APIIo_CloseProcessWindowID, nullptr, nullptr);
        #endif
        return NoError;
    }

    ParamDictElement paramToWriteelem = {};
    if (!GetRenumElements (guidArray, paramToWriteelem, rule_definitions)) {
        msg_rep ("ReNumSelected", "No data to write", NoError, APINULLGuid);
        #if defined(AC_27) || defined(AC_28) || defined(AC_29)
        ACAPI_ProcessWindow_CloseProcessWindow ();
        #else
        ACAPI_Interface (APIIo_CloseProcessWindowID, nullptr, nullptr);
        #endif
        return NoError;
    }
    const Int32 iseng = ID_ADDON_STRINGS + isEng ();
    GS::UniString undoString = RSGetIndString (iseng, UndoReNumId, ACAPI_GetOwnResModule ());
    UInt32 qtywrite = paramToWriteelem.GetSize ();
    ACAPI_CallUndoableCommand (undoString, [&]() -> GSErrCode {
        GS::UniString subtitle = GS::UniString::Printf ("Writing data to %d elements", qtywrite); short i = 2;
        #if defined(AC_27) || defined(AC_28) || defined(AC_29)
        bool showPercent = false;
        Int32 maxval = 2;
        ACAPI_ProcessWindow_SetNextProcessPhase (&subtitle, &maxval, &showPercent);
        #else
        ACAPI_Interface (APIIo_SetNextProcessPhaseID, &subtitle, &i);
        #endif
        #ifndef AC_22
        bool suspGrp = false;
        #if defined(AC_27) || defined(AC_28) || defined(AC_29)
        ACAPI_View_IsSuspendGroupOn (&suspGrp);
        if (!suspGrp) ACAPI_Grouping_Tool (guidArray, APITool_SuspendGroups, nullptr);
        #else
        ACAPI_Environment (APIEnv_IsSuspendGroupOnID, &suspGrp);
        if (!suspGrp) ACAPI_Element_Tool (guidArray, APITool_SuspendGroups, nullptr);
        #endif
        #endif
        ParamHelpers::ElementsWrite (paramToWriteelem);
        #if defined(AC_27) || defined(AC_28) || defined(AC_29)
        ACAPI_ProcessWindow_CloseProcessWindow ();
        #else
        ACAPI_Interface (APIIo_CloseProcessWindowID, nullptr, nullptr);
        #endif
        return NoError;
    });
    SyncArray (syncSettings, guidArray);
    finish = clock ();
    duration = (double) (finish - start) / CLOCKS_PER_SEC;
    msg_rep ("ReNumSelected", GS::UniString::Printf ("Time spent %.3f s", duration), NoError, APINULLGuid);
    return NoError;
}

bool GetRenumElements (GS::Array<API_Guid>& guidArray, ParamDictElement& paramToWriteelem, GS::HashTable<API_Guid, API_PropertyDefinition>& rule_definitions)
{
    // Получаем список правил суммирования
    Rules rules = {};
    ParamDictElement paramToReadelem = {};
    bool hasRule = !rule_definitions.IsEmpty ();
    const Int32 iseng = ID_ADDON_STRINGS + isEng ();
    GS::UniString subtitle = GS::UniString::Printf ("Reading data from %d elements", guidArray.GetSize ());
    ParamDict error_propertyname = {};

    for (UInt32 i = 0; i < guidArray.GetSize (); i++) {
        GS::Array<API_PropertyDefinition> definitions = {};
        GSErrCode err = ACAPI_Element_GetPropertyDefinitions (guidArray[i], API_PropertyDefinitionFilter_UserDefined, definitions);
        if (err != NoError) {
            msg_rep ("GetRenumElements", "ACAPI_Element_GetPropertyDefinitions", err, APINULLGuid);
            continue;
        }
        if (definitions.IsEmpty ()) {
            continue;
        }
        bool hasDef = false;
        if (hasRule) {
            for (UInt32 i = 0; i < definitions.GetSize (); i++) {
                if (definitions[i].description.IsEmpty ()) continue;
                if (definitions[i].description.Contains ("Renum_flag")) {
                    if (!rule_definitions.ContainsKey (definitions[i].guid)) {
                        definitions.Delete (i);
                    } else {
                        hasDef = true;
                    }
                }
            }
        } else {
            hasDef = ReNumHasFlag (definitions);
        }
        #if defined(AC_27) || defined(AC_28) || defined(AC_29)
        bool showPercent = true;
        Int32 maxval = guidArray.GetSize ();
        ACAPI_ProcessWindow_SetNextProcessPhase (&subtitle, &maxval, &showPercent);
        #else
        ACAPI_Interface (APIIo_SetNextProcessPhaseID, &subtitle, &i);
        #endif
        #if defined(AC_27) || defined(AC_28) || defined(AC_29)
        if (ACAPI_ProcessWindow_IsProcessCanceled ()) return false;
        #else
        if (ACAPI_Interface (APIIo_IsProcessCanceledID, nullptr, nullptr)) return false;
        #endif
        if (!hasDef) continue;
        ReNum_GetElement (guidArray[i], paramToReadelem, rules, error_propertyname);
    }
    if (!error_propertyname.IsEmpty ()) {
        GS::UniString out = ":\n";
        for (auto& cIt : error_propertyname) {
            #if defined(AC_28) || defined(AC_29)
            GS::UniString s = cIt.key;
            #else
            GS::UniString s = *cIt.key;
            #endif
            s.ReplaceAll ("{@", "");
            s.ReplaceAll ("}", "");
            s.ReplaceAll (":", " : ");
            out.Append (s); out.Append ("\n");
        }
        msg_rep ("ReNumSelected", "Can't find property, check name: " + out, APIERR_GENERAL, APINULLGuid);
        GS::UniString SpecEmptyListdString = RSGetIndString (iseng, 71, ACAPI_GetOwnResModule ());
        ACAPI_WriteReport (SpecEmptyListdString + out, true);
        return false;
    }
    if (!paramToReadelem.IsEmpty ()) msg_rep ("ReNumSelected", GS::UniString::Printf ("Find elements - %d ", paramToReadelem.GetSize ()), NoError, APINULLGuid);
    if (!rules.IsEmpty ()) msg_rep ("ReNumSelected", GS::UniString::Printf ("Find rules - %d ", rules.GetSize ()), NoError, APINULLGuid);
    if (paramToReadelem.IsEmpty () || rules.IsEmpty ()) {
        if (paramToReadelem.IsEmpty ()) msg_rep ("ReNumSelected", "Parameters for read not found", NoError, APINULLGuid);
        if (rules.IsEmpty ()) msg_rep ("ReNumSelected", "Rules not found", NoError, APINULLGuid);
        GS::UniString SpecEmptyListdString = RSGetIndString (iseng, 75, ACAPI_GetOwnResModule ());
        ACAPI_WriteReport (SpecEmptyListdString, true);
        return false;
    }
    ParamHelpers::ElementsRead (paramToReadelem); // Читаем значения
    bool has_error_ones = false;
    GS::UniString error_rule_name = "";
    GS::UniString ok_rule_name = "";
    // Теперь выясняем - какой режим нумерации у элементов и распределяем позиции
    for (GS::HashTable<API_Guid, RenumRule>::PairIterator cIt = rules.EnumeratePairs (); cIt != NULL; ++cIt) {
        #if defined(AC_28) || defined(AC_29)
        const RenumRule& rule = cIt->value;
        #else
        const RenumRule& rule = *cIt->value;
        #endif
        bool has_error = false;
        if (!rule.elemts.IsEmpty ()) ReNumOneRule (rule, paramToReadelem, paramToWriteelem, has_error);
        if (has_error) {
            has_error_ones = true;
            error_rule_name.Append (rule.position);
            error_rule_name.Append (";\n");
        } else {
            ok_rule_name.Append (rule.position);
            ok_rule_name.Append (";\n");
        }
    }
    if (!error_rule_name.IsEmpty ()) {
        error_rule_name.ReplaceAll ("{@", "");
        error_rule_name.ReplaceAll ("}", "");
        error_rule_name.ReplaceAll (":", "");
        error_rule_name.ReplaceAll ("property", "");
        error_rule_name.Trim ();
        error_rule_name = "\n" + error_rule_name;
    }
    if (!ok_rule_name.IsEmpty ()) {
        ok_rule_name.ReplaceAll ("{@", "");
        ok_rule_name.ReplaceAll ("}", "");
        ok_rule_name.ReplaceAll (":", "");
        ok_rule_name.ReplaceAll ("property", "");
        ok_rule_name.Trim ();
        ok_rule_name = "\n" + ok_rule_name;
    }
    if (has_error_ones) {
        GS::UniString SpecEmptyListdString = RSGetIndString (iseng, 72, ACAPI_GetOwnResModule ()) + error_rule_name;
        ACAPI_WriteReport (SpecEmptyListdString, true);
    }
    if (paramToWriteelem.IsEmpty ()) {
        msg_rep ("ReNumSelected", "No position changes required", NoError, APINULLGuid);
        GS::UniString SpecEmptyListdString = RSGetIndString (iseng, 73, ACAPI_GetOwnResModule ()) + error_rule_name + ok_rule_name;
        ACAPI_WriteReport (SpecEmptyListdString, true);
        return false;
    }
    GS::UniString msg = GS::UniString::Printf ("%d", paramToReadelem.GetSize ()) + ok_rule_name;
    GS::UniString SpecEmptyListdString = RSGetIndString (iseng, 74, ACAPI_GetOwnResModule ());
    ACAPI_WriteReport (SpecEmptyListdString + msg, true);
    msg_rep ("ReNumSelected", GS::UniString::Printf ("Elements with new position  - %d ", paramToReadelem.GetSize ()), NoError, APINULLGuid);
    return !paramToWriteelem.IsEmpty ();
}

// -----------------------------------------------------------------------------------------------------------------------
// Функция распределяет элемент в таблицу с правилами нумерации
// -----------------------------------------------------------------------------------------------------------------------
bool ReNum_GetElement (const API_Guid& elemGuid, ParamDictElement& paramToRead, Rules& rules, ParamDict& error_propertyname)
{
    bool hasRenum = false;
    if (!ParamHelpers::isPropertyDefinitionRead ()) return false;
    ParamDictValue& propertyParams = PROPERTYCACHE ().property;
    for (ParamDictValue::PairIterator cIt = propertyParams.EnumeratePairs (); cIt != NULL; ++cIt) {
        bool flag = false;
        #if defined(AC_28) || defined(AC_29)
        ParamValue& param = cIt->value;
        #else
        ParamValue& param = *cIt->value;
        #endif
        API_PropertyDefinition& definition = param.definition;
        if (!definition.description.Contains ("Renum_flag")) continue;
        if (!definition.description.Contains ("{")) {
            msg_rep ("ReNumSelected", definition.name + " Renum_flag: check the opening bracket, there should be {", APIERR_GENERAL, APINULLGuid);
            continue;
        }
        if (!definition.description.Contains ("}")) {
            msg_rep ("ReNumSelected", definition.name + " Renum_flag: check the closing bracket, there should be }", APIERR_GENERAL, APINULLGuid);
            continue;
        }
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
                    if (partstring[1].Contains ("null")) rulecritetia.nulltype = ADDZEROS;
                    if (partstring[1].Contains ("allnull")) rulecritetia.nulltype = ADDMAXZEROS;
                    if (partstring[1].Contains ("space")) rulecritetia.nulltype = ADDSPACE;
                    if (partstring[1].Contains ("allspace")) rulecritetia.nulltype = ADDMAXSPACE;
                    // Добавление нужного количества знаков
                    if (rulecritetia.nulltype != NOZEROS && partstring[1].Contains ("_")) {
                        GS::Array<GS::UniString> partstring_;
                        if (StringSplt (partstring[1], "_", partstring_) > 0) {
                            double nullcount = 0;
                            if (UniStringToDouble (partstring_[0], nullcount)) rulecritetia.nullcount = (int) nullcount;
                        }
                    }
                }
            } else {
                rawNameposition = rawNameposition + paramName + "}";
            }
            if (!rawNameposition.Contains ("property")) rawNameposition.ReplaceAll ("{", GDLNAMEPREFIX);
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
                    if (!rawNamecriteria.Contains ("property")) rawNamecriteria.ReplaceAll ("{", GDLNAMEPREFIX);
                    if (!rawNamedelimetr.IsEmpty () && !rawNamedelimetr.Contains ("property")) rawNamedelimetr.ReplaceAll ("{", GDLNAMEPREFIX);

                    // Если такие свойства есть - записываем правило
                    if (propertyParams.ContainsKey (rawNamecriteria) && (propertyParams.ContainsKey (rawNamedelimetr) || rawNamedelimetr.IsEmpty ())) {
                        rulecritetia.state = true;
                        rulecritetia.oldalgoritm = false;
                        rulecritetia.position = rawNameposition;
                        rulecritetia.flag = param.rawName;
                        rulecritetia.criteria = rawNamecriteria;
                        rulecritetia.delimetr = rawNamedelimetr;
                        rulecritetia.guid = definition.guid;
                        flag = true;
                    } else {
                        if (!propertyParams.ContainsKey (rawNamecriteria) && !error_propertyname.ContainsKey (rawNamecriteria)) error_propertyname.Add (rawNamecriteria, false);
                        if (!rawNamedelimetr.IsEmpty () && !propertyParams.ContainsKey (rawNamedelimetr) && !error_propertyname.ContainsKey (rawNamedelimetr)) error_propertyname.Add (rawNamedelimetr, false);
                    }
                } else {
                    if (ruleparamName.Contains ("Renum")) {
                        GS::UniString msg = ", in property description " + propertyParams.Get (rawNameposition).definition.name;
                        if (!ruleparamName.Contains ("{")) msg_rep ("ReNumSelected", "Renum: check the closing bracket, there should be }" + msg, APIERR_GENERAL, APINULLGuid);
                        if (!ruleparamName.Contains ("}")) msg_rep ("ReNumSelected", "Renum: check the opening bracket, there should be {" + msg, APIERR_GENERAL, APINULLGuid);
                    } else {
                        msg_rep ("ReNumSelected", "Renum_flag must refer to a property with the description Renum, check property description " + definition.name, APIERR_GENERAL, APINULLGuid);
                    }
                }
            } else {
                if (!error_propertyname.ContainsKey (rawNameposition)) error_propertyname.Add (rawNameposition, false);
            }
            rules.Add (definition.guid, rulecritetia);
        } else {
            flag = true; // Правило уже существует, просто добавим свойства в словарь чтения и id в правила
        }
        if (flag) {
            RenumRule& rulecritetia = rules.Get (definition.guid);
            if (rulecritetia.guid != APINULLGuid) hasRenum = true;
            rulecritetia.elemts.Push (elemGuid);
            ParamValue pvalue_position;
            ParamValue pvalue_flag;
            ParamValue pvalue_criteria;
            ParamValue pvalue_delimetr;
            bool has_position = false;
            bool has_flag = false;
            bool has_criteria = false;
            bool has_delimetr = false;
            if (!rulecritetia.position.IsEmpty ()) has_position = ParamHelpers::GetParamValueFromCache (rulecritetia.position, pvalue_position);
            if (!rulecritetia.flag.IsEmpty ()) has_flag = ParamHelpers::GetParamValueFromCache (rulecritetia.flag, pvalue_flag);
            if (!rulecritetia.criteria.IsEmpty ()) has_criteria = ParamHelpers::GetParamValueFromCache (rulecritetia.criteria, pvalue_criteria);
            if (!rulecritetia.delimetr.IsEmpty ()) has_delimetr = ParamHelpers::GetParamValueFromCache (rulecritetia.delimetr, pvalue_delimetr);
            pvalue_position.fromGuid = elemGuid;
            pvalue_flag.fromGuid = elemGuid;
            pvalue_criteria.fromGuid = elemGuid;
            pvalue_delimetr.fromGuid = elemGuid;
            if (has_position) ParamHelpers::AddParamValue2ParamDictElement (elemGuid, pvalue_position, paramToRead);
            if (has_flag) ParamHelpers::AddParamValue2ParamDictElement (elemGuid, pvalue_flag, paramToRead);
            if (has_criteria) ParamHelpers::AddParamValue2ParamDictElement (elemGuid, pvalue_criteria, paramToRead);
            if (has_delimetr) ParamHelpers::AddParamValue2ParamDictElement (elemGuid, pvalue_delimetr, paramToRead);
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

void ReNumOneRule (const RenumRule& rule, ParamDictElement& paramToReadelem, ParamDictElement& paramToWriteelem, bool& has_error)
{

    // Рассортируем элементы по разделителю, типу нумерации и критерию.
    Delimetr delimetrList;
    if (!ElementsSeparation (rule, paramToReadelem, delimetrList, has_error)) return;

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
                    pos.FormatToMax (maxposdelim, rule.nulltype, rule.nullcount);
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

bool ElementsSeparation (const RenumRule& rule, const  ParamDictElement& paramToReadelem, Delimetr& delimetrList, bool& has_error)
{
    if (!rule.state) return false;
    GS::Array<API_Guid> elemArray = rule.elemts;
    GS::Array<API_Guid> elemrenumArray = {};
    bool needAddSet = false;
    bool flag = false;

    // Собираем значения свойств из criteria. Нам нужны только уникальные значения.
    for (UInt32 i = 0; i < elemArray.GetSize (); i++) {
        if (!paramToReadelem.ContainsKey (elemArray[i])) continue;

        // Сразу проверим режим нумерации элемента
        short state = RENUM_SKIP;
        RenumPos pos;
        const ParamDictValue& params = paramToReadelem.Get (elemArray[i]);
        if (params.ContainsKey (rule.flag) && params.ContainsKey (rule.position)) {
            const ParamValue& paramflag = params.Get (rule.flag);
            const ParamValue& paramposition = params.Get (rule.position);
            if (!paramflag.isValid) {
                msg_rep ("ReNumSelected", "Skip element with not valid value in flag: " + rule.flag, APIERR_GENERAL, elemArray[i]);
                has_error = true;
            }
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
                const ParamValue& param = params.Get (rule.delimetr);
                if (param.isValid) {
                    GSCharCode chcode = GetCharCode (param.val.uniStringValue);
                    delimetr = param.val.uniStringValue.ToCStr (0, MaxUSize, chcode).Get ();
                } else {
                    msg_rep ("ReNumSelected", "Skip element with not valid value in delimetr: " + rule.delimetr, APIERR_GENERAL, elemArray[i]);
                    state = RENUM_SKIP;
                    has_error = true;
                }
            }

            // Получаем критерий, если он есть
            std::string criteria = "";
            if (params.ContainsKey (rule.criteria)) {
                const ParamValue& param = params.Get (rule.criteria);
                if (param.isValid) {
                    if (param.val.uniStringValue.IsEmpty ()) {
                        msg_rep ("ReNumSelected", "Skip element with empty value in criteria: " + rule.criteria, APIERR_GENERAL, elemArray[i]);
                        state = RENUM_SKIP;
                        has_error = true;
                    } else {
                        GSCharCode chcode = GetCharCode (param.val.uniStringValue);
                        criteria = param.val.uniStringValue.ToCStr (0, MaxUSize, chcode).Get ();
                    }
                } else {
                    msg_rep ("ReNumSelected", "Skip element with not valid value in criteria: " + rule.criteria, APIERR_GENERAL, elemArray[i]);
                    state = RENUM_SKIP;
                    has_error = true;
                }
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
    return flag;
}

//--------------------------------------------------------------------------------------------------------------------------
// Проверяет - есть ли хоть одно описание флага
//--------------------------------------------------------------------------------------------------------------------------
bool ReNumHasFlag (const GS::Array<API_PropertyDefinition> definitions)
{
    if (definitions.IsEmpty ()) return false;
    for (const auto& definition : definitions) {
        if (definition.description.IsEmpty ()) continue;
        if (definition.description.Contains ("Renum_flag")) {
            return true;
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
        const Int32 iseng = ID_ADDON_STRINGS + isEng ();
        // Исключаемые позиции
        GS::UniString txtypenum = RSGetIndString (iseng, RenumSkipID, ACAPI_GetOwnResModule ());
        if (flag.Contains (txtypenum) || flag.Contains ("skip")) return RENUM_SKIP;

        // У нередактируемых элементов нет возможности поменять позицию - просто учтём её
        if (!isEditable) return RENUM_IGNORE;

        // Неизменные позиции
        txtypenum = RSGetIndString (iseng, RenumIgnoreID, ACAPI_GetOwnResModule ());
        if (flag.Contains (txtypenum) || flag.Contains ("ignore")) return RENUM_IGNORE;

        // Пустые позиции (если строка пустая - значение ноль.)
        if (paramposition.val.intValue != 0) return RENUM_ADD;

        // Все прочие
        return RENUM_NORMAL;
    }
    return RENUM_SKIP;
}
#endif
