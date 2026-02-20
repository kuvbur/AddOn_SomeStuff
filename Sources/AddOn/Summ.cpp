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
    PROPERTYCACHE ().Update ();
    GS::HashTable<API_Guid, API_PropertyDefinition> rule_definitions;
    if (guidArray.GetSize () == 1) {
        if (GetSumRuleFromSelected (guidArray[0], rule_definitions)) GetSumElementForPropertyDefinition (rule_definitions, guidArray);
    }
    GS::UniString undoString = RSGetIndString (ID_ADDON_STRINGS + isEng (), UndoSumId, ACAPI_GetOwnResModule ());
    bool flag_write = true;
    UInt32 qtywrite = 0;
    ACAPI_CallUndoableCommand (undoString, [&]() -> GSErrCode {
        GS::UniString subtitle = GS::UniString::Printf ("Reading data from %d elements", guidArray.GetSize ());; short i = 1;
        #if defined(AC_27) || defined(AC_28) || defined(AC_29)
        Int32 maxval = 2;
        ACAPI_ProcessWindow_SetNextProcessPhase (&subtitle, &maxval, &showPercent);
        #else
        ACAPI_Interface (APIIo_SetNextProcessPhaseID, &subtitle, &i);
        #endif
        ParamDictElement paramToWriteelem;
        if (!GetSumValuesOfElements (guidArray, paramToWriteelem, rule_definitions)) {
            flag_write = false;
            msg_rep ("SumSelected", "No data to write", NoError, APINULLGuid);
            #if defined(AC_27) || defined(AC_28) || defined(AC_29)
            ACAPI_ProcessWindow_CloseProcessWindow ();
            #else
            ACAPI_Interface (APIIo_CloseProcessWindowID, nullptr, nullptr);
            #endif
            return NoError;
        }
        subtitle = GS::UniString::Printf ("Writing data to %d elements", paramToWriteelem.GetSize ()); i = 2;
        #if defined(AC_27) || defined(AC_28) || defined(AC_29)
        ACAPI_ProcessWindow_SetNextProcessPhase (&subtitle, &maxval, &showPercent);
        #else
        ACAPI_Interface (APIIo_SetNextProcessPhaseID, &subtitle, &i);
        #endif
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
    if (flag_write) {
        SyncArray (syncSettings, guidArray);
    }
    finish = clock ();
    duration = (double) (finish - start) / CLOCKS_PER_SEC;
    GS::UniString time = GS::UniString::Printf (" %.3f s", duration);
    GS::UniString intString = GS::UniString::Printf ("Qty elements - %d ", guidArray.GetSize ()) + GS::UniString::Printf ("wrtite to - %d", qtywrite) + time;
    msg_rep ("SumSelected", intString, NoError, APINULLGuid);
    return NoError;
}


// -----------------------------------------------------------------------------------------------------------------------
// В случае, если выбран только один элемент - обрабатываем ТОЛЬКО правила, видимые у него
// Функция для сбора правил с элемента
// -----------------------------------------------------------------------------------------------------------------------
bool GetSumRuleFromSelected (const API_Guid& elemguid, GS::HashTable<API_Guid, API_PropertyDefinition>& definitions)
{
    #if defined(AC_22)
    return false;
    #else
    GS::Array<API_PropertyDefinition> definitions_;
    GSErrCode err = ACAPI_Element_GetPropertyDefinitions (elemguid, API_PropertyDefinitionFilter_UserDefined, definitions_);
    if (err == NoError && !definitions_.IsEmpty ()) {
        for (UInt32 i = 0; i < definitions_.GetSize (); i++) {
            if (!definitions_[i].description.IsEmpty ()) {
                if (definitions_[i].description.Contains ("Sum") && definitions_[i].description.Contains ("{") && definitions_[i].description.Contains ("}")) {
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
void GetSumElementForPropertyDefinition (const GS::HashTable<API_Guid, API_PropertyDefinition>& definitions, GS::Array<API_Guid>& guidArray)
{
    #if defined(AC_22)
    return;
    #else
    for (auto& cIt : definitions) {
        #if defined(AC_28) || defined(AC_29)
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

bool GetSumValuesOfElements (const GS::Array<API_Guid> guidArray, ParamDictElement& paramToWriteelem, GS::HashTable<API_Guid, API_PropertyDefinition>& rule_definitions)
{
    if (guidArray.IsEmpty ()) return false;
    SumRules rules;
    ParamDictElement paramToReadelem;
    GS::UniString subtitle = GS::UniString::Printf ("Reading data from %d elements", guidArray.GetSize ());

    // Получаем список правил суммирования
    bool hasSum = false;
    for (UInt32 i = 0; i < guidArray.GetSize (); i++) {
        #if defined(AC_27) || defined(AC_28) || defined(AC_29)
        bool showPercent = true;
        Int32 maxval = guidArray.GetSize ();
        if (i % 10 == 0) ACAPI_ProcessWindow_SetNextProcessPhase (&subtitle, &maxval, &showPercent);
        #else
        if (i % 10 == 0) ACAPI_Interface (APIIo_SetNextProcessPhaseID, &subtitle, &i);
        #endif
        #if defined(AC_27) || defined(AC_28) || defined(AC_29)
        if (ACAPI_ProcessWindow_IsProcessCanceled ()) return false;
        #else
        if (ACAPI_Interface (APIIo_IsProcessCanceledID, nullptr, nullptr)) return false;
        #endif
        ParamDictValue paramToRead;
        if (Sum_GetElement (guidArray[i], paramToRead, rules, rule_definitions)) {
            ClassificationFunc::SystemDict systemdict;
            ParamHelpers::Read (guidArray[i], paramToRead);
            ParamHelpers::AddParamDictValue2ParamDictElement (guidArray[i], paramToRead, paramToReadelem);
            hasSum = true;
        }
    }

    if (!hasSum) return false;

    // Суммируем, заполняе словарь для записи
    for (GS::HashTable<API_Guid, SumRule>::PairIterator cIt = rules.EnumeratePairs (); cIt != NULL; ++cIt) {
        #if defined(AC_28) || defined(AC_29)
        const SumRule& rule = cIt->value;
        #else
        const SumRule& rule = *cIt->value;
        #endif
        if (!rule.elemts.IsEmpty ()) Sum_OneRule (rule, paramToReadelem, paramToWriteelem);
    }
    return !paramToWriteelem.IsEmpty ();
}

// -----------------------------------------------------------------------------------------------------------------------
// Функция распределяет элемент в таблицу с правилами нумерации
// -----------------------------------------------------------------------------------------------------------------------
bool Sum_GetElement (const API_Guid& elemGuid, ParamDictValue& paramToRead, SumRules& rules, GS::HashTable<API_Guid, API_PropertyDefinition>& rule_definitions)
{
    bool has_sum = false;
    if (!ParamHelpers::isPropertyDefinitionRead ()) return false;
    ParamDictValue& propertyParams = PROPERTYCACHE ().property;
    for (ParamDictValue::PairIterator cIt = propertyParams.EnumeratePairs (); cIt != NULL; ++cIt) {
        #if defined(AC_28) || defined(AC_29)
        ParamValue& param = cIt->value;
        #else
        ParamValue& param = *cIt->value;
        #endif
        API_PropertyDefinition& definition = param.definition;
        // Является ли свойство описанием системы суммирования?
        if (!definition.description.Contains ("Sum")) continue;
        if (!definition.description.Contains ("{")) {
            msg_rep ("SumSelected", definition.name + " Sum: check the opening bracket, there should be {", APIERR_GENERAL, APINULLGuid);
            continue;
        }
        if (!definition.description.Contains ("}")) {
            msg_rep ("SumSelected", definition.name + " Sum: check the closing bracket, there should be }", APIERR_GENERAL, APINULLGuid);
            continue;
        }
        bool flag_add = false;
        bool flag_hasrule = true;
        if (!rule_definitions.IsEmpty ()) {
            if (!rule_definitions.ContainsKey (definition.guid)) flag_hasrule = false;
        }
        if (flag_hasrule) {
            if (!rules.ContainsKey (definition.guid)) {
                SumRule paramtype = {};
                if (Sum_Rule (elemGuid, definition, paramtype)) {
                    paramtype.position = param.rawName;
                    rules.Add (definition.guid, paramtype);
                    flag_add = true;
                }
            } else {
                flag_add = true;
            }
        }
        if (flag_add) {

            // Дописываем элемент в правило
            rules.Get (definition.guid).elemts.Push (elemGuid);

            // Добавляем свойства для чтения в словарь
            SumRule paramtype = rules.Get (definition.guid);
            if (!paramtype.position.IsEmpty ()) ParamHelpers::AddParamValue2ParamDict (elemGuid, propertyParams.Get (paramtype.position), paramToRead);
            if (!paramtype.value.IsEmpty ()) ParamHelpers::AddParamValue2ParamDict (elemGuid, propertyParams.Get (paramtype.value), paramToRead);
            if (!paramtype.criteria.IsEmpty ()) ParamHelpers::AddParamValue2ParamDict (elemGuid, propertyParams.Get (paramtype.criteria), paramToRead);
            has_sum = true;
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
bool Sum_Rule (const API_Guid& elemGuid, const API_PropertyDefinition& definition, SumRule& paramtype)
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
        msg_rep ("SumSelected", "Check that the property name is correct and must begin with Property. " + definition.name, NoError, elemGuid);
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
} // ReNumRule

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
}
