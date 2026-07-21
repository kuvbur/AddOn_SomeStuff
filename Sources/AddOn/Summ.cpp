//------------ kuvbur 2022 ------------
#include "ACAPinc.h"
#include "APIEnvir.h"
#include "DG4rule.hpp"
#include "Propertycache.hpp"
#include "Summ.hpp"
#include "Sync.hpp"
#include <map>

typedef std::unordered_map<std::string, SortInx> SumCriteria;

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
GSErrCode SumSelected (SyncSettings &syncSettings) {
    clock_t start, finish;
    double duration;
    start = clock ();
    GS::UniString funcname = "Summation";
    GS::Int32 nPhase = 1;
#if defined(AC_27) || defined(AC_28) || defined(AC_29)
    bool showPercent = true;
    Int32 maxval = 6;
#endif
    ProcessWindowGuard pwGuard (funcname, nPhase);
    GS::Array<API_Guid> guidArray = GetSelectedElements (true, true, syncSettings, true, false, false);
    if (guidArray.IsEmpty ())
        return NoError;
    ParamDictElement paramToWriteelem = {};
    if (!GetSumValuesOfElements (guidArray, paramToWriteelem)) {
        msg_rep ("SumSelected", "No data to write", NoError, APINULLGuid);
        return NoError;
    }
    GS::UniString subtitle = GS::UniString::Printf ("Writing data to %d elements", paramToWriteelem.GetSize ());
    short i = 6;
#if defined(AC_27) || defined(AC_28) || defined(AC_29)
    ACAPI_ProcessWindow_SetNextProcessPhase (&subtitle, &maxval, &showPercent);
#else
    ACAPI_Interface (APIIo_SetNextProcessPhaseID, &subtitle, &i);
#endif
    const Int32 iseng = ID_ADDON_STRINGS + isEng ();
    GS::UniString undoString = RSGetIndString (iseng, UndoSumId, ACAPI_GetOwnResModule ());
    UInt32 qtywrite = 0;
    ACAPI_CallUndoableCommand (undoString, [&] () -> GSErrCode {
        bool suspGrp = false;
#ifndef AC_22
    #if defined(AC_27) || defined(AC_28) || defined(AC_29)
        ACAPI_View_IsSuspendGroupOn (&suspGrp);
        if (!suspGrp)
            ACAPI_Grouping_Tool (guidArray, APITool_SuspendGroups, nullptr);
    #else
        ACAPI_Environment (APIEnv_IsSuspendGroupOnID, &suspGrp);
        if (!suspGrp) ACAPI_Element_Tool (guidArray, APITool_SuspendGroups, nullptr);
    #endif
#endif
        ParamHelpers::ElementsWrite (paramToWriteelem);
        qtywrite = paramToWriteelem.GetSize ();
        return NoError;
    });
    ParamHelpers::WriteInfo (paramToWriteelem);
    SyncArray (syncSettings, guidArray);
    finish = clock ();
    duration = (double)(finish - start) / CLOCKS_PER_SEC;
    GS::UniString time = GS::UniString::Printf (" %.3f s", duration);
    GS::UniString intString = GS::UniString::Printf ("Qty elements - %d ", guidArray.GetSize ()) +
                              GS::UniString::Printf ("wrtite to - %d", qtywrite) + time;
    msg_rep ("SumSelected", intString, NoError, APINULLGuid);
    return NoError;
}

bool SumDG (SumRules &sum_rules, bool &rule_from_one) {
    RuleSelectData rules = {};
    for (GS::HashTable<API_Guid, SumRule>::PairIterator cIt = sum_rules.EnumeratePairs (); cIt != NULL; ++cIt) {
#if defined(AC_28) || defined(AC_29)
        const SumRule &rule = cIt->value;
#else
        const SumRule &rule = *cIt->value;
#endif
        if (!rule.state)
            continue;
        if (rules.rules.ContainsKey (rule.rule_name))
            continue;
        if (rules.qty_elements.ContainsKey (rule.rule_name))
            continue;
        rules.rules.Add (rule.rule_name, true);
        rules.qty_elements.Add (rule.rule_name, GS::UniString::Printf ("%d", rule.elemts.GetSize ()));
        if (rule.write_to == SumToInfo)
            rules.color.Add (rule.rule_name, Gfx::Color::Blue);
    }
    rules.is_warn = rule_from_one;
    rules.titleResID = UndoSumId;
    RuleSelectDialog dialog (rules);
    if (!dialog.Invoke ())
        return false;
    bool has_true_state = false;
    for (GS::HashTable<API_Guid, SumRule>::PairIterator cIt = sum_rules.EnumeratePairs (); cIt != NULL; ++cIt) {
#if defined(AC_28) || defined(AC_29)
        SumRule &rule = cIt->value;
#else
        SumRule &rule = *cIt->value;
#endif
        if (!rule.state)
            continue;
        if (!rules.rules.ContainsKey (rule.rule_name))
            continue;
        rule.state = rules.rules.Get (rule.rule_name);
        if (rule.state)
            has_true_state = true;
    }
    return has_true_state;
}

bool GetSumValuesOfElements (GS::Array<API_Guid> &guidArray, ParamDictElement &paramToWriteelem) {
    GS::HashTable<API_Guid, API_PropertyDefinition> rule_definitions = {};
    SumRules rules = {};
    ParamDictElement paramToRead = {};
    GS::UniString subtitle = GS::UniString::Printf ("Get rule from %d elements", guidArray.GetSize ());
    short i = 6;
#if defined(AC_27) || defined(AC_28) || defined(AC_29)
    bool showPercent = true;
    Int32 maxval = 6;
    ACAPI_ProcessWindow_SetNextProcessPhase (&subtitle, &maxval, &showPercent);
#else
    ACAPI_Interface (APIIo_SetNextProcessPhaseID, &subtitle, &i);
#endif
    bool rule_from_one = (guidArray.GetSize () == 1);
    if (!GetRuleFromSelected (guidArray, rule_definitions, "Sum", true)) {
        msg_rep ("SumSelected",
                 "No sum rule found.\nCheck that the description of the user property contains Sum and the name of the "
                 "property",
                 NoError,
                 APINULLGuid,
                 true);
        return false;
    }
    subtitle = GS::UniString::Printf ("Calc rule from %d elements", guidArray.GetSize ());
#if defined(AC_27) || defined(AC_28) || defined(AC_29)
    ACAPI_ProcessWindow_SetNextProcessPhase (&subtitle, &maxval, &showPercent);
#else
    ACAPI_Interface (APIIo_SetNextProcessPhaseID, &subtitle, &i);
#endif
    if (!Sum_GetElement (guidArray, rule_definitions, paramToRead, rules)) {
        msg_rep ("SumSelected",
                 "No data to read.\nCheck that the properties specified in the rule exist and have values",
                 NoError,
                 APINULLGuid,
                 true);
        return false;
    }
    if (!SumDG (rules, rule_from_one)) {
        msg_rep ("SumSelected", "Execution interrupted by user", NoError, APINULLGuid);
        return false;
    }
    subtitle = GS::UniString::Printf ("Read data from %d elements", guidArray.GetSize ());
#if defined(AC_27) || defined(AC_28) || defined(AC_29)
    ACAPI_ProcessWindow_SetNextProcessPhase (&subtitle, &maxval, &showPercent);
#else
    ACAPI_Interface (APIIo_SetNextProcessPhaseID, &subtitle, &i);
#endif
    ParamHelpers::ElementsRead (paramToRead);
    subtitle = GS::UniString::Printf ("Sum data from %d elements", paramToRead.GetSize ());
#if defined(AC_27) || defined(AC_28) || defined(AC_29)
    ACAPI_ProcessWindow_SetNextProcessPhase (&subtitle, &maxval, &showPercent);
#else
    ACAPI_Interface (APIIo_SetNextProcessPhaseID, &subtitle, &i);
#endif
    // Суммируем, заполняе словарь для записи
    for (GS::HashTable<API_Guid, SumRule>::PairIterator cIt = rules.EnumeratePairs (); cIt != NULL; ++cIt) {
#if defined(AC_28) || defined(AC_29)
        SumRule &rule = cIt->value;
#else
        SumRule &rule = *cIt->value;
#endif
        if (!rule.state)
            continue;
        if (rule.elemts.IsEmpty ())
            continue;
        Sum_OneRule (rule, paramToRead, paramToWriteelem);
    }
    return !paramToWriteelem.IsEmpty ();
}

// ----------------------------------------------------------------------------
// Функция распределяет элемент в таблицу с правилами нумерации
// ----------------------------------------------------------------------------
bool Sum_GetElement (const GS::Array<API_Guid> &guidArray,
                     const GS::HashTable<API_Guid, API_PropertyDefinition> &rule_definitions,
                     ParamDictElement &paramToRead,
                     SumRules &rules) {
    for (const auto &cIt : rule_definitions) {
#if defined(AC_28) || defined(AC_29)
        const API_PropertyDefinition definition = cIt.value;
#else
        const API_PropertyDefinition definition = *cIt.value;
#endif
        if (!rules.ContainsKey (definition.guid)) {
            SumRule paramtype = {};
            if (!Sum_Rule (definition, paramtype))
                continue;
            rules.Add (definition.guid, paramtype);
        }
        if (!rules.ContainsKey (definition.guid))
            continue;
        SumRule &paramtype = rules.Get (definition.guid);
        ParamValue pvalue_position;
        ParamValue pvalue_value;
        ParamValue pvalue_criteria;
        bool has_position = false;
        bool has_value = false;
        bool has_criteria = false;
        if (!paramtype.position.IsEmpty ())
            has_position = ParamHelpers::GetParamValueFromCache (paramtype.position, pvalue_position);
        if (!paramtype.value.IsEmpty ())
            has_value = ParamHelpers::GetParamValueFromCache (paramtype.value, pvalue_value);
        if (!paramtype.criteria.IsEmpty ())
            has_criteria = ParamHelpers::GetParamValueFromCache (paramtype.criteria, pvalue_criteria);
        if (paramtype.write_to == SumToInfo) {
            GS::HashTable<API_Guid, API_PropertyDefinition> definitions = {};
            definitions.Add (definition.guid, definition);
            GS::Array<API_Guid> _guidArray = {};
            GetElementForPropertyDefinition (definitions, _guidArray);
            if (SumRule *rule = rules.GetPtr (definition.guid)) {
                for (const auto &elemGuid : _guidArray) {
                    // Дописываем элемент в правило
                    rule->elemts.Push (elemGuid);
                    // Добавляем свойства для чтения в словарь
                    pvalue_position.fromGuid = elemGuid;
                    pvalue_value.fromGuid = elemGuid;
                    pvalue_criteria.fromGuid = elemGuid;
                    if (has_position)
                        ParamHelpers::AddParamValue2ParamDictElement (elemGuid, pvalue_position, paramToRead);
                    if (has_value)
                        ParamHelpers::AddParamValue2ParamDictElement (elemGuid, pvalue_value, paramToRead);
                    if (has_criteria)
                        ParamHelpers::AddParamValue2ParamDictElement (elemGuid, pvalue_criteria, paramToRead);
                }
            }
        } else {
            if (SumRule *rule = rules.GetPtr (definition.guid)) {
                for (const auto &elemGuid : guidArray) {
                    if (!ACAPI_Element_Filter (elemGuid,
                                               APIFilt_IsEditable | APIFilt_HasAccessRight | APIFilt_InMyWorkspace)) {
                        rule->n_ignore += 1;
                        msg_rep ("GetSumRuleFromSelected", "Element not editable", NoError, elemGuid);
                        continue;
                    }
                    // Дописываем элемент в правило
                    rule->elemts.Push (elemGuid);
                    // Добавляем свойства для чтения в словарь
                    pvalue_position.fromGuid = elemGuid;
                    pvalue_value.fromGuid = elemGuid;
                    pvalue_criteria.fromGuid = elemGuid;
                    if (has_position)
                        ParamHelpers::AddParamValue2ParamDictElement (elemGuid, pvalue_position, paramToRead);
                    if (has_value)
                        ParamHelpers::AddParamValue2ParamDictElement (elemGuid, pvalue_value, paramToRead);
                    if (has_criteria)
                        ParamHelpers::AddParamValue2ParamDictElement (elemGuid, pvalue_criteria, paramToRead);
                }
            }
        }
    }
    return (!rules.IsEmpty ());
} // Sum_GetElement

// -----------------------------------------------------------------------------------------------------------------------
// Функция возвращает заполненное правило суммирования SumRule
// SumRule.sum_type - тип суммирования (TextSum / NumSum)
// SumRule.value - имя свойства в формате rawname
// SumRule.criteria - критерий суммирования (разбивки)
// SumRule.delimetr - разделитель для текстовой суммы (конкатенации)
// SumRule.ignore_val - игнорируемые значения
// -----------------------------------------------------------------------------------------------------------------------
bool Sum_Rule (const API_PropertyDefinition &definition, SumRule &paramtype) {
    // По типу данных свойства определим тим суммирования
    // Если строковый тип - объединяем уникальные значения, если тип числовой - суммируем
    paramtype.sum_type = 0;
    if (definition.valueType == API_PropertyStringValueType)
        paramtype.sum_type = TextSum;
    if (definition.valueType == API_PropertyRealValueType || definition.valueType == API_PropertyIntegerValueType)
        paramtype.sum_type = NumSum;
    if (!paramtype.sum_type)
        return false;
    GS::UniString paramName = definition.description;
    GS::Array<GS::UniString> partstring = {};
    if (StringSplt (paramName.ToLowerCase (), BRACEEND, partstring, "sum") > 0) {
        paramName = partstring[0] + BRACEEND;
    }
    paramName = paramName.GetSubstring (CHARBRACESTART, CHARBRACEEND, 0);
    paramName.ReplaceAll (SLASHEKR, SLASH);
    partstring.Clear ();
    int nparam = StringSplt (paramName.ToLowerCase (), SEMICOLON, partstring, true);
    if (nparam == 0)
        return false;
    GS::UniString key = PVALPREFIX + partstring[0] + BRACEEND;
    GS::UniString rawName_rule = "";
    GetPropertyFullName (definition, rawName_rule);
    paramtype.rule_name = rawName_rule;
    rawName_rule = PROPERTYNAMEPREFIX + rawName_rule.ToLowerCase () + BRACEEND;
    if (ParamHelpers::isCacheContainsParamValue (key)) {
        // Если свойство ведёт в информцию о проекте - до складываем свойство, в котором указано правило
        if (key.Contains (INFONAMEPREFIX)) {
            if (ParamHelpers::isCacheContainsParamValue (rawName_rule))
                paramtype.value = rawName_rule;
            paramtype.write_to = SumToInfo;
            paramtype.position = key;
            paramtype.value = rawName_rule;
        } else {
            paramtype.position = rawName_rule;
            paramtype.value = key;
        }
    }
    if (paramtype.value.IsEmpty ()) {
        msg_rep ("SumSelected",
                 "Check that the property name is correct and must begin with Property. " + definition.name,
                 NoError,
                 APINULLGuid);
        return false;
    }
    // Ищём определение свойства-критерия
    if (nparam > 1) {
        GS::UniString key = PVALPREFIX + partstring[1] + BRACEEND;
        if (ParamHelpers::isCacheContainsParamValue (key) && paramtype.write_to != SumToInfo) {
            paramtype.criteria = key;
        } else {
            if (partstring[1].Contains ("min") && paramtype.sum_type == NumSum)
                paramtype.sum_type = MinSum;
            if (partstring[1].Contains ("max") && paramtype.sum_type == NumSum)
                paramtype.sum_type = MaxSum;
            if (paramtype.sum_type == NumSum || paramtype.sum_type == TextSum)
                paramtype.delimetr = partstring[1].ToCStr ().Get ();
        }
    }
    // Если задан и разделитель - пропишем его
    if (nparam > 2) {
        GS::UniString key = PVALPREFIX + partstring[2] + BRACEEND;
        if (ParamHelpers::isCacheContainsParamValue (key) && paramtype.write_to != SumToInfo) {
            paramtype.criteria = key;
        } else {
            if (partstring[2].Contains ("min") && paramtype.sum_type == NumSum)
                paramtype.sum_type = MinSum;
            if (partstring[2].Contains ("max") && paramtype.sum_type == NumSum)
                paramtype.sum_type = MaxSum;
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
        if (partstring[3].Contains ("min") && paramtype.sum_type == NumSum)
            paramtype.sum_type = MinSum;
        if (partstring[3].Contains ("max") && paramtype.sum_type == NumSum)
            paramtype.sum_type = MaxSum;
        if (paramtype.ignore_val.empty () && (paramtype.sum_type == NumSum || paramtype.sum_type == TextSum))
            paramtype.ignore_val = partstring[3].ToCStr ().Get ();
    }
    return true;
} // Sum_Rule

void Sum_OneRule (SumRule &rule, ParamDictElement &paramToReadelem, ParamDictElement &paramToWriteelem) {
    SumCriteria criteriaList;
    GS::UniString delimetr = GS::UniString (rule.delimetr.c_str ());
    // Выбираем значения критериев
    for (UInt32 i = 0; i < rule.elemts.GetSize (); i++) {
        const ParamDictValue *params = paramToReadelem.GetPtr (rule.elemts[i]);
        if (params == nullptr)
            continue;
        const ParamValue *paramcriteria = params->GetPtr (rule.criteria);

        if (!rule.criteria.IsEmpty () && paramcriteria != nullptr) {
            GSCharCode chcode = GetCharCode (paramcriteria->val.uniStringValue);
            std::string criteria = paramcriteria->val.uniStringValue.ToCStr (0, MaxUSize, chcode).Get ();
            criteriaList[criteria].inx.Push (i);
        } else {
            criteriaList["all"].inx.Push (i);
        }
    }
    // Проходим по словарю с критериями и суммируем
    for (SumCriteria::iterator i = criteriaList.begin (); i != criteriaList.end (); ++i) {
        GS::Array<UInt32> eleminpos = i->second.inx;
        ParamValue summ; // Для суммирования числовых значений
        bool has_sum = false;
        for (UInt32 j = 0; j < eleminpos.GetSize (); j++) {
            const API_Guid &elemGuid = rule.elemts[eleminpos[j]];

            const ParamDictValue *params = paramToReadelem.GetPtr (elemGuid);
            if (params == nullptr)
                continue;
            const ParamValue *paramvalue = params->GetPtr (rule.value);

            if (paramvalue == nullptr) {
                rule.n_ignore += 1;
                continue;
            }
            // Проверяем - было ли считано значение
            if (!paramvalue->isValid) {
                msg_rep ("Sum_OneRule", "Param not valid :" + rule.value, NoError, elemGuid);
                rule.n_ignore += 1;
                continue;
            }
            if (rule.write_to == SumToInfo) {
                summ.type = paramvalue->type;
                summ.val.type = paramvalue->val.type;
            }
            if (rule.sum_type == TextSum) {
                summ.val.uniStringValue = summ.val.uniStringValue + paramvalue->val.uniStringValue;
                if (j < eleminpos.GetSize () - 1)
                    summ.val.uniStringValue = summ.val.uniStringValue + delimetr;
            } else {
                if (rule.sum_type == NumSum) {
                    summ.val.doubleValue = summ.val.doubleValue + paramvalue->val.doubleValue;
                    summ.val.rawDoubleValue = summ.val.rawDoubleValue + paramvalue->val.rawDoubleValue;
                    summ.val.intValue = summ.val.intValue + paramvalue->val.intValue;
                    summ.val.boolValue = summ.val.boolValue && paramvalue->val.boolValue;
                } else {
                    if (!has_sum && (rule.sum_type == MinSum || rule.sum_type == MaxSum)) {
                        summ.val.doubleValue = paramvalue->val.doubleValue;
                        summ.val.rawDoubleValue = paramvalue->val.rawDoubleValue;
                        summ.val.intValue = paramvalue->val.intValue;
                        summ.val.boolValue = paramvalue->val.boolValue;
                    } else {
                        if (rule.sum_type == MinSum) {
                            summ.val.doubleValue = fmin (summ.val.doubleValue, paramvalue->val.doubleValue);
                            summ.val.rawDoubleValue = fmin (summ.val.rawDoubleValue, paramvalue->val.rawDoubleValue);
                            summ.val.intValue = summ.val.intValue > paramvalue->val.intValue ? paramvalue->val.intValue
                                                                                             : summ.val.intValue;
                            summ.val.boolValue = summ.val.boolValue || paramvalue->val.boolValue;
                        }
                        if (rule.sum_type == MaxSum) {
                            summ.val.doubleValue = fmax (summ.val.doubleValue, paramvalue->val.doubleValue);
                            summ.val.rawDoubleValue = fmax (summ.val.rawDoubleValue, paramvalue->val.rawDoubleValue);
                            summ.val.intValue = summ.val.intValue < paramvalue->val.intValue ? paramvalue->val.intValue
                                                                                             : summ.val.intValue;
                            summ.val.boolValue = summ.val.boolValue && paramvalue->val.boolValue;
                        }
                    }
                }
            }
            has_sum = true;
        }
        // Заполнение словаря записи
        if (!has_sum) {
            rule.n_ignore += 1;
            continue;
        }
        // Для конкатенации текста определим уникальные значения
        if (rule.sum_type == TextSum) {
            GS::UniString unic = StringUnic (summ.val.uniStringValue, delimetr);
            summ.val.uniStringValue = unic;
        }
        for (UInt32 j = 0; j < eleminpos.GetSize (); j++) {
            const API_Guid elemGuid = rule.elemts[eleminpos[j]];

            const ParamDictValue *params = paramToReadelem.GetPtr (elemGuid);
            if (params == nullptr)
                continue;
            const ParamValue *parampositionPtr = params->GetPtr (rule.position);
            if (parampositionPtr == nullptr)
                continue;
            ParamValue paramposition = *parampositionPtr;
            paramposition.isValid = true;
            if (rule.write_to != SumToInfo)
                summ.val.type = paramposition.val.type;
            if (rule.sum_type != TextSum) {
                summ.val.formatstring = paramposition.val.formatstring;
                summ.val.uniStringValue = ParamHelpers::ToString (summ);
            }
            // Записываем только изменённые значения
            if (paramposition != summ) {
                paramposition.val = summ.val;
                ParamHelpers::AddParamValue2ParamDictElement (elemGuid, paramposition, paramToWriteelem);
                rule.n_write += 1;
            }
            // Если нужно записать в информацию о проекте - то достаточно записать один раз, так как свойство для записи
            // будет одинаковое для всех элементов
            if (rule.write_to == SumToInfo) {
                rule.n_write = (int)eleminpos.GetSize ();
                break;
            }
        }
    }
    return;
} // Sum_OneRule
