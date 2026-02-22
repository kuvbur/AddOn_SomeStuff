//------------ kuvbur 2022 ------------
#include	"ACAPinc.h"
#include	"APIEnvir.h"
#include	"Propertycache.hpp"
#include	"Summ.hpp"
#include	"Sync.hpp"
#include	<map>
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
GSErrCode SumSelected (SyncSettings& syncSettings)
{
    clock_t start, finish;
    double  duration;
    start = clock ();
    GS::UniString funcname = "Summation";
    GS::Int32 nPhase = 1;
    #if defined(AC_27) || defined(AC_28) || defined(AC_29)
    bool showPercent = true;
    ACAPI_ProcessWindow_InitProcessWindow (&funcname, &nPhase);
    #else
    ACAPI_Interface (APIIo_InitProcessWindowID, &funcname, &nPhase);
    #endif
    GS::Array<API_Guid> guidArray = GetSelectedElements (true, true, syncSettings, true, false, false);
    if (guidArray.IsEmpty ()) return NoError;

    ParamDictElement paramToWriteelem = {};
    if (!GetSumValuesOfElements (guidArray, paramToWriteelem)) {
        msg_rep ("SumSelected", "No data to write", NoError, APINULLGuid);
        #if defined(AC_27) || defined(AC_28) || defined(AC_29)
        ACAPI_ProcessWindow_CloseProcessWindow ();
        #else
        ACAPI_Interface (APIIo_CloseProcessWindowID, nullptr, nullptr);
        #endif
        return NoError;
    }
    GS::UniString subtitle = GS::UniString::Printf ("Writing data to %d elements", paramToWriteelem.GetSize ());
    short i = 6;
    #if defined(AC_27) || defined(AC_28) || defined(AC_29)
    ACAPI_ProcessWindow_SetNextProcessPhase (&subtitle, &maxval, &showPercent);
    #else
    ACAPI_Interface (APIIo_SetNextProcessPhaseID, &subtitle, &i);
    #endif

    GS::UniString undoString = RSGetIndString (ID_ADDON_STRINGS + isEng (), UndoSumId, ACAPI_GetOwnResModule ());
    UInt32 qtywrite = 0;
    ACAPI_CallUndoableCommand (undoString, [&]() -> GSErrCode {
        bool suspGrp = false;
        #ifndef AC_22
        #if defined(AC_27) || defined(AC_28) || defined(AC_29)
        ACAPI_View_IsSuspendGroupOn (&suspGrp);
        if (!suspGrp) ACAPI_Grouping_Tool (guidArray, APITool_SuspendGroups, nullptr);
        #else
        ACAPI_Environment (APIEnv_IsSuspendGroupOnID, &suspGrp);
        if (!suspGrp) ACAPI_Element_Tool (guidArray, APITool_SuspendGroups, nullptr);
        #endif
        #endif
        ParamHelpers::ElementsWrite (paramToWriteelem);
        qtywrite = paramToWriteelem.GetSize ();
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
    GS::UniString time = GS::UniString::Printf (" %.3f s", duration);
    GS::UniString intString = GS::UniString::Printf ("Qty elements - %d ", guidArray.GetSize ()) + GS::UniString::Printf ("wrtite to - %d", qtywrite) + time;
    msg_rep ("SumSelected", intString, NoError, APINULLGuid);
    return NoError;
}

bool GetSumValuesOfElements (GS::Array<API_Guid>& guidArray, ParamDictElement& paramToWriteelem)
{
    GS::HashTable<API_Guid, API_PropertyDefinition> rule_definitions = {};
    SumRules rules = {};
    ParamDictElement paramToRead = {};
    GS::UniString subtitle = GS::UniString::Printf ("Get rule from %d elements", guidArray.GetSize ()); short i = 6;
    #if defined(AC_27) || defined(AC_28) || defined(AC_29)
    Int32 maxval = 2;
    ACAPI_ProcessWindow_SetNextProcessPhase (&subtitle, &maxval, &showPercent);
    #else
    ACAPI_Interface (APIIo_SetNextProcessPhaseID, &subtitle, &i);
    #endif
    if (!GetRuleFromSelected (guidArray, rule_definitions, "Sum", true)) {
        msg_rep ("SumSelected", "No sum rule found. Check that the description of the user property contains Sum and the name of the property", NoError, APINULLGuid);
        return false;
    }
    subtitle = GS::UniString::Printf ("Calc rule from %d elements", guidArray.GetSize ());
    #if defined(AC_27) || defined(AC_28) || defined(AC_29)
    Int32 maxval = 2;
    ACAPI_ProcessWindow_SetNextProcessPhase (&subtitle, &maxval, &showPercent);
    #else
    ACAPI_Interface (APIIo_SetNextProcessPhaseID, &subtitle, &i);
    #endif
    if (!Sum_GetElement (guidArray, rule_definitions, paramToRead, rules)) {
        msg_rep ("SumSelected", "No data to read. Check that the properties specified in the rule exist and have values", NoError, APINULLGuid);
        return false;
    }
    subtitle = GS::UniString::Printf ("Read data from %d elements", guidArray.GetSize ());
    #if defined(AC_27) || defined(AC_28) || defined(AC_29)
    Int32 maxval = 2;
    ACAPI_ProcessWindow_SetNextProcessPhase (&subtitle, &maxval, &showPercent);
    #else
    ACAPI_Interface (APIIo_SetNextProcessPhaseID, &subtitle, &i);
    #endif
    ParamHelpers::ElementsRead (paramToRead);

    subtitle = GS::UniString::Printf ("Sum data from %d elements", paramToRead.GetSize ());
    #if defined(AC_27) || defined(AC_28) || defined(AC_29)
    Int32 maxval = 2;
    ACAPI_ProcessWindow_SetNextProcessPhase (&subtitle, &maxval, &showPercent);
    #else
    ACAPI_Interface (APIIo_SetNextProcessPhaseID, &subtitle, &i);
    #endif
    // Суммируем, заполняе словарь для записи
    for (GS::HashTable<API_Guid, SumRule>::PairIterator cIt = rules.EnumeratePairs (); cIt != NULL; ++cIt) {
        #if defined(AC_28) || defined(AC_29)
        const SumRule& rule = cIt->value;
        #else
        const SumRule& rule = *cIt->value;
        #endif
        if (rule.elemts.IsEmpty ()) continue;
        Sum_OneRule (rule, paramToRead, paramToWriteelem);
    }
    return !paramToWriteelem.IsEmpty ();
}

// ----------------------------------------------------------------------------
// Функция распределяет элемент в таблицу с правилами нумерации
// ----------------------------------------------------------------------------
bool Sum_GetElement (const GS::Array<API_Guid>& guidArray, const GS::HashTable<API_Guid, API_PropertyDefinition>& rule_definitions, ParamDictElement& paramToRead, SumRules& rules)
{
    for (const auto& cIt : rule_definitions) {
        #if defined(AC_28) || defined(AC_29)
        const API_PropertyDefinition definition = cIt.value;
        #else
        const API_PropertyDefinition definition = *cIt.value;
        #endif
        SumRule paramtype = {};
        if (!rules.ContainsKey (definition.guid)) {
            if (Sum_Rule (definition, paramtype)) {
                GS::UniString fname = "";
                GS::UniString rawName = "";
                GetPropertyFullName (definition, fname);
                rawName = PROPERTYNAMEPREFIX;
                rawName.Append (fname.ToLowerCase ());
                rawName.Append ("}");
                paramtype.position = rawName;
                rules.Add (definition.guid, paramtype);
            }
        } else {
            paramtype = rules.Get (definition.guid);
        }
        ParamValue pvalue_position;
        ParamValue pvalue_value;
        ParamValue pvalue_criteria;
        bool has_position = false;
        bool has_value = false;
        bool has_criteria = false;
        if (!paramtype.position.IsEmpty ()) has_position = ParamHelpers::GetParamValueFromCache (paramtype.position, pvalue_position);
        if (!paramtype.value.IsEmpty ()) has_value = ParamHelpers::GetParamValueFromCache (paramtype.value, pvalue_value);
        if (!paramtype.criteria.IsEmpty ()) has_criteria = ParamHelpers::GetParamValueFromCache (paramtype.criteria, pvalue_criteria);
        for (const auto& elemGuid : guidArray) {
            if (!ACAPI_Element_Filter (elemGuid, APIFilt_IsEditable)) {
                msg_rep ("GetSumRuleFromSelected", "Element not editable", NoError, elemGuid);
                continue;
            }
            // Дописываем элемент в правило
            rules.Get (definition.guid).elemts.Push (elemGuid);
            // Добавляем свойства для чтения в словарь
            pvalue_position.fromGuid = elemGuid;
            pvalue_value.fromGuid = elemGuid;
            pvalue_criteria.fromGuid = elemGuid;
            if (has_position) ParamHelpers::AddParamValue2ParamDictElement (elemGuid, pvalue_position, paramToRead);
            if (has_value) ParamHelpers::AddParamValue2ParamDictElement (elemGuid, pvalue_value, paramToRead);
            if (has_criteria) ParamHelpers::AddParamValue2ParamDictElement (elemGuid, pvalue_criteria, paramToRead);
        }
    }
    return (!rules.IsEmpty ());
}// Sum_GetElement

// -----------------------------------------------------------------------------------------------------------------------
// Функция возвращает заполненное правило суммирования SumRule
// SumRule.sum_type - тип суммирования (TextSum / NumSum)
// SumRule.value - имя свойства в формате rawname
// SumRule.criteria - критерий суммирования (разбивки)
// SumRule.delimetr - разделитель для текстовой суммы (конкатенации)
// SumRule.ignore_val - игнорируемые значения
// -----------------------------------------------------------------------------------------------------------------------
bool Sum_Rule (const API_PropertyDefinition& definition, SumRule& paramtype)
{
    // По типу данных свойства определим тим суммирования
    // Если строковый тип - объединяем уникальные значения, если тип числовой - суммируем
    paramtype.sum_type = 0;
    if (definition.valueType == API_PropertyStringValueType) paramtype.sum_type = TextSum;
    if (definition.valueType == API_PropertyRealValueType || definition.valueType == API_PropertyIntegerValueType) paramtype.sum_type = NumSum;
    if (!paramtype.sum_type) return false;
    GS::UniString paramName = definition.description;
    GS::Array<GS::UniString> partstring = {};
    if (StringSplt (paramName.ToLowerCase (), "}", partstring, "sum") > 0) {
        paramName = partstring[0] + "}";
    }
    paramName = paramName.GetSubstring ('{', '}', 0);
    paramName.ReplaceAll ("\\/", "/");
    partstring.Clear ();
    int nparam = StringSplt (paramName.ToLowerCase (), ";", partstring);
    if (nparam == 0) return false;
    GS::UniString key = "{@" + partstring[0] + "}";
    if (ParamHelpers::isCacheContainsParamValue (key)) {
        paramtype.value = key;
    } else {
        msg_rep ("SumSelected", "Check that the property name is correct and must begin with Property. " + definition.name, NoError, APINULLGuid);
        return false;
    }
    // Ищём определение свойства-критерия
    if (nparam > 1) {
        GS::UniString key = "{@" + partstring[1] + "}";
        if (ParamHelpers::isCacheContainsParamValue (key)) {
            paramtype.criteria = key;
        } else {
            if (partstring[1].Contains ("min") && paramtype.sum_type == NumSum) paramtype.sum_type = MinSum;
            if (partstring[1].Contains ("max") && paramtype.sum_type == NumSum) paramtype.sum_type = MaxSum;
            if (paramtype.sum_type == NumSum || paramtype.sum_type == TextSum) paramtype.delimetr = partstring[1].ToCStr ().Get ();
        }
    }
    // Ищём определение свойства-критерия
    if (nparam > 1) {
        GS::UniString key = "{@" + partstring[1] + "}";
        if (ParamHelpers::isCacheContainsParamValue (key)) {
            paramtype.criteria = "{@" + partstring[1] + "}";
        } else {
            if (partstring[1].Contains ("min") && paramtype.sum_type == NumSum) paramtype.sum_type = MinSum;
            if (partstring[1].Contains ("max") && paramtype.sum_type == NumSum) paramtype.sum_type = MaxSum;
            if (paramtype.delimetr.empty () && (paramtype.sum_type == NumSum || paramtype.sum_type == TextSum)) paramtype.delimetr = partstring[1].ToCStr ().Get ();
        }
    }
    // Если задан и разделитель - пропишем его
    if (nparam > 2) {
        GS::UniString key = "{@" + partstring[2] + "}";
        if (ParamHelpers::isCacheContainsParamValue (key)) {
            paramtype.criteria = key;
        } else {
            if (partstring[2].Contains ("min") && paramtype.sum_type == NumSum) paramtype.sum_type = MinSum;
            if (partstring[2].Contains ("max") && paramtype.sum_type == NumSum) paramtype.sum_type = MaxSum;
            if (paramtype.sum_type == NumSum || paramtype.sum_type == TextSum) {
                if (paramtype.delimetr.empty ()) {
                    paramtype.delimetr = partstring[2].ToCStr ().Get ();
                } else {
                    paramtype.ignore_val = partstring[2].ToCStr ().Get ();
                }
            }
        }
    }
    // Если заданы игнорируемые значения
    if (nparam > 3) {
        if (partstring[3].Contains ("min") && paramtype.sum_type == NumSum) paramtype.sum_type = MinSum;
        if (partstring[3].Contains ("max") && paramtype.sum_type == NumSum) paramtype.sum_type = MaxSum;
        if (paramtype.ignore_val.empty () && (paramtype.sum_type == NumSum || paramtype.sum_type == TextSum))
            paramtype.ignore_val = partstring[3].ToCStr ().Get ();
    }
    return true;
} // Sum_Rule

void Sum_OneRule (const SumRule& rule, ParamDictElement& paramToReadelem, ParamDictElement& paramToWriteelem)
{
    SumCriteria criteriaList;
    GS::UniString delimetr = GS::UniString (rule.delimetr.c_str ());
    // Выбираем значения критериев
    for (UInt32 i = 0; i < rule.elemts.GetSize (); i++) {
        if (paramToReadelem.ContainsKey (rule.elemts[i])) {
            ParamDictValue params = paramToReadelem.Get (rule.elemts[i]);
            if (!rule.criteria.IsEmpty () && params.ContainsKey (rule.criteria)) {
                GSCharCode chcode = GetCharCode (params.Get (rule.criteria).val.uniStringValue);
                std::string criteria = params.Get (rule.criteria).val.uniStringValue.ToCStr (0, MaxUSize, chcode).Get ();
                criteriaList[criteria].inx.Push (i);
            } else {
                criteriaList["all"].inx.Push (i);
            }
        }
    }

    // Проходим по словарю с критериями и суммируем
    for (SumCriteria::iterator i = criteriaList.begin (); i != criteriaList.end (); ++i) {
        GS::Array<UInt32> eleminpos = i->second.inx;
        ParamValue summ; // Для суммирования числовых значений
        bool has_sum = false;
        for (UInt32 j = 0; j < eleminpos.GetSize (); j++) {
            API_Guid elemGuid = rule.elemts[eleminpos[j]];
            ParamDictValue params = paramToReadelem.Get (elemGuid);
            if (params.ContainsKey (rule.value)) {

                // Проверяем - было ли считано значение
                ParamValue param = params.Get (rule.value);
                if (param.isValid) {
                    if (rule.sum_type == TextSum) {
                        summ.val.uniStringValue = summ.val.uniStringValue + param.val.uniStringValue;
                        if (j < eleminpos.GetSize () - 1) summ.val.uniStringValue = summ.val.uniStringValue + delimetr;
                    } else {
                        if (rule.sum_type == NumSum) {
                            summ.val.doubleValue = summ.val.doubleValue + param.val.doubleValue;
                            summ.val.rawDoubleValue = summ.val.rawDoubleValue + param.val.rawDoubleValue;
                            summ.val.intValue = summ.val.intValue + param.val.intValue;
                            summ.val.boolValue = summ.val.boolValue && param.val.boolValue;
                        } else {
                            if (!has_sum && (rule.sum_type == MinSum || rule.sum_type == MaxSum)) {
                                summ.val.doubleValue = param.val.doubleValue;
                                summ.val.rawDoubleValue = param.val.rawDoubleValue;
                                summ.val.intValue = param.val.intValue;
                                summ.val.boolValue = param.val.boolValue;
                            } else {
                                if (rule.sum_type == MinSum) {
                                    summ.val.doubleValue = fmin (summ.val.doubleValue, param.val.doubleValue);
                                    summ.val.rawDoubleValue = fmin (summ.val.rawDoubleValue, param.val.rawDoubleValue);
                                    summ.val.intValue = summ.val.intValue > param.val.intValue ? param.val.intValue : summ.val.intValue;
                                    summ.val.boolValue = summ.val.boolValue || param.val.boolValue;
                                }
                                if (rule.sum_type == MaxSum) {
                                    summ.val.doubleValue = fmax (summ.val.doubleValue, param.val.doubleValue);
                                    summ.val.rawDoubleValue = fmax (summ.val.rawDoubleValue, param.val.rawDoubleValue);
                                    summ.val.intValue = summ.val.intValue < param.val.intValue ? param.val.intValue : summ.val.intValue;
                                    summ.val.boolValue = summ.val.boolValue && param.val.boolValue;
                                }
                            }
                        }
                    }
                    has_sum = true;
                } else {
                    msg_rep ("Sum_OneRule", "Param not valid :" + rule.value, NoError, elemGuid);
                }
            }
        }
        // Заполнение словаря записи
        if (has_sum) {
            // Для конкатенации текста определим уникальные значения
            if (rule.sum_type == TextSum) {
                GS::UniString unic = StringUnic (summ.val.uniStringValue, delimetr);
                summ.val.uniStringValue = unic;
            }
            for (UInt32 j = 0; j < eleminpos.GetSize (); j++) {
                API_Guid elemGuid = rule.elemts[eleminpos[j]];
                ParamDictValue params = paramToReadelem.Get (elemGuid);
                if (params.ContainsKey (rule.position)) {
                    ParamValue param = params.Get (rule.position);
                    param.isValid = true;
                    summ.val.type = param.val.type;
                    if (rule.sum_type != TextSum) {
                        summ.val.formatstring = param.val.formatstring;
                        summ.val.uniStringValue = ParamHelpers::ToString (summ);
                    }
                    // Записываем только изменённые значения
                    if (param != summ) {
                        param.val = summ.val;
                        ParamHelpers::AddParamValue2ParamDictElement (elemGuid, param, paramToWriteelem);
                    }
                }
            }
        }
    }
    return;
}//Sum_OneRule
