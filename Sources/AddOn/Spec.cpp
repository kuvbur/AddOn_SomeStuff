//------------ kuvbur 2022 ------------
#include	"ACAPinc.h"
#include	"APIEnvir.h"
#include	"Spec.hpp"

namespace Spec
{

GSErrCode SpecAll (const SyncSettings& syncSettings)
{
    long time_start = clock ();
    GS::UniString funcname = "SpecAll";
    GS::Int32 nPhase = 1;
#if defined(AC_27) || defined(AC_28)
    bool showPercent = true;
    ACAPI_ProcessWindow_InitProcessWindow (&funcname, &nPhase);
#else
    ACAPI_Interface (APIIo_InitProcessWindowID, &funcname, &nPhase);
#endif
    GS::Array<API_Guid> guidArray;
    ACAPI_Element_GetElemList (API_ZombieElemID, &guidArray, APIFilt_OnVisLayer | APIFilt_IsVisibleByRenovation | APIFilt_IsInStructureDisplay);
    if (guidArray.IsEmpty ()) return NoError;
    return SpecArray (syncSettings, guidArray);
    //    GS::UniString undoString = RSGetIndString (ID_ADDON_STRINGS + isEng (), UndoReNumId, ACAPI_GetOwnResModule ());
    //    bool flag_write = true;
    //    ACAPI_CallUndoableCommand (undoString, [&]() -> GSErrCode {
    //        GS::UniString subtitle = GS::UniString::Printf ("Reading data from %d elements", guidArray.GetSize ());; short i = 1;
    //#if defined(AC_27) || defined(AC_28)
    //        Int32 maxval = 2;
    //        ACAPI_ProcessWindow_SetNextProcessPhase (&subtitle, &maxval, &showPercent);
    //#else
    //        ACAPI_Interface (APIIo_SetNextProcessPhaseID, &subtitle, &i);
    //#endif
    //        ParamDictElement paramToWriteelem;
    //        if (!GetSumValuesOfElements (guidArray, paramToWriteelem)) {
    //            flag_write = false;
    //            msg_rep ("SpecAll", "No data to write", NoError, APINULLGuid);
    //#if defined(AC_27) || defined(AC_28)
    //            ACAPI_ProcessWindow_CloseProcessWindow ();
    //#else
    //            ACAPI_Interface (APIIo_CloseProcessWindowID, nullptr, nullptr);
    //#endif
    //            return NoError;
    //        }
    //        subtitle = GS::UniString::Printf ("Writing data to %d elements", paramToWriteelem.GetSize ()); i = 2;
    //#if defined(AC_27) || defined(AC_28)
    //        ACAPI_ProcessWindow_SetNextProcessPhase (&subtitle, &maxval, &showPercent);
    //#else
    //        ACAPI_Interface (APIIo_SetNextProcessPhaseID, &subtitle, &i);
    //#endif
    //        ParamHelpers::ElementsWrite (paramToWriteelem);
    //        long time_end = clock ();
    //        GS::UniString time = GS::UniString::Printf (" %d s", (time_end - time_start) / 1000);
    //        GS::UniString intString = GS::UniString::Printf ("Qty elements - %d ", guidArray.GetSize ()) + GS::UniString::Printf ("wrtite to - %d", paramToWriteelem.GetSize ()) + time;
    //        msg_rep ("SpecAll", intString, NoError, APINULLGuid);
    //#if defined(AC_27) || defined(AC_28)
    //        ACAPI_ProcessWindow_CloseProcessWindow ();
    //#else
    //        ACAPI_Interface (APIIo_CloseProcessWindowID, nullptr, nullptr);
    //#endif
    //        return NoError;
    //    });
    //    if (flag_write) {
    //        ClassificationFunc::SystemDict systemdict;
    //        SyncArray (syncSettings, guidArray, systemdict);
    //    }
    //    return NoError;
}

GSErrCode SpecArray (const SyncSettings& syncSettings, GS::Array<API_Guid>& guidArray)
{
    int dummymode = IsDummyModeOn ();
    GS::UniString funcname = "Sync Selected";
    GSErrCode err = NoError;
    SpecRuleDict rules;
    bool flagfindspec = false;
    for (UInt32 i = 0; i < guidArray.GetSize (); i++) {
        err = GetRuleFromElement (guidArray[i], rules);
        if (err == NoError) flagfindspec = true;
    }
    // Теперь пройдём по прочитанным правилам и сформируем список параметров для чтения
    ParamDictElement paramToRead; // Словарь с параметрами для чтения
    for (auto& cIt : rules) {
#if defined(AC_28)
        SpecRule rule = cIt.value;
#else
        SpecRule rule = *cIt.value;
#endif
        // Читаем только параметры групп
        if (rule.is_Valid) {
            ParamDict params;
            for (UInt32 i = 0; i < rule.groups.GetSize (); i++) {
                if (rule.groups[i].is_Valid) {
                    for (UInt32 j = 0; j < rule.groups[i].sum_paramrawname.GetSize (); j++) {
                        GS::UniString rawname = rule.groups[i].sum_paramrawname[j];
                        if (!params.ContainsKey (rawname)) params.Add (rawname, true);
                    }
                    for (UInt32 j = 0; j < rule.groups[i].unic_paramrawname.GetSize (); j++) {
                        GS::UniString rawname = rule.groups[i].unic_paramrawname[j];
                        if (!params.ContainsKey (rawname)) params.Add (rawname, true);
                    }
                }
            }
            // Добавляем параметры для каждого элемента
            for (UInt32 i = 0; i < rule.elements.GetSize (); i++) {
                API_Guid elemguid = rule.elements[i];
                for (auto& cItt : params) {
#if defined(AC_28)
                    GS::UniString rawname = cItt.key;
#else
                    GS::UniString rawname = *cItt.key;
#endif
                    ParamDictValue paramDict;
                    ParamHelpers::AddValueToParamDictValue (paramDict, rawname);
                    ParamHelpers::AddParamDictValue2ParamDictElement (elemguid, paramDict, paramToRead);
                }
            }
        }
    }
    // Читаем все возможные свойства
    ClassificationFunc::SystemDict systemdict;
    ParamDictValue propertyParams;
    ParamHelpers::ElementsRead (paramToRead, propertyParams, systemdict);

    for (auto& cIt : rules) {
#if defined(AC_28)
        SpecRule rule = cIt.value;
#else
        SpecRule rule = *cIt.value;
#endif
        // Каждая группа - это уникальный элемент, который предстоит создать.
        if (rule.is_Valid) {
            ElementDict elements;
            for (UInt32 i = 0; i < rule.elements.GetSize (); i++) {
                API_Guid elemguid = rule.elements[i];
                for (UInt32 i = 0; i < rule.groups.GetSize (); i++) {
                    if (rule.groups[i].is_Valid) {
                        // Принадлежность субэлемента к группе определим по ключу - сцепке значени уникальных параметров
                        GS::UniString key = "";
                        Element element;
                        for (UInt32 j = 0; j < rule.groups[i].unic_paramrawname.GetSize (); j++) {
                            GS::UniString rawname = rule.groups[i].unic_paramrawname[j];
                            ParamValue pvalue;
                            if (ParamHelpers::GetParamValueForElements (elemguid, rawname, paramToRead, pvalue)) {
                                key = key + "@" + pvalue.val.uniStringValue;
                            }
                            element.out_unic_paramrawname.Push (pvalue);
                        }
                        if (elements.ContainsKey (key)) {
                            elements.Get (key).elements.Push (elemguid);
                        } else {
                            element.elements.Push (elemguid);
                            elements.Add (key, element);
                        }
                    }
                }
            }
            // На данном этапе у нас есть словарь elements, где скруппированы элементы по критериям
            // Можно создавать
            int hh = 1;
        }
    }
    return err;
}

GSErrCode GetRuleFromElement (const API_Guid& elemguid, SpecRuleDict& rules)
{
    GSErrCode err = NoError;
    GS::Array<API_PropertyDefinition> definitions;
    err = ACAPI_Element_GetPropertyDefinitions (elemguid, API_PropertyDefinitionFilter_UserDefined, definitions);
    if (err != NoError || definitions.IsEmpty ()) return err;
    for (UInt32 i = 0; i < definitions.GetSize (); i++) {
        GS::UniString description = definitions[i].description;
        if (!description.IsEmpty ()) {
            if (description.Contains ("Spec_rule") && description.Contains ("{") && description.Contains ("}")) {
                bool flagfindspec = false;
                // Проверяем - включено ли свойство
                API_Property propertyflag = {};
                if (ACAPI_Element_GetPropertyValue (elemguid, definitions[i].guid, propertyflag) == NoError) {
                    if (propertyflag.isDefault) {
                        flagfindspec = propertyflag.definition.defaultValue.basicValue.singleVariant.variant.boolValue;
                    } else {
                        flagfindspec = propertyflag.value.singleVariant.variant.boolValue;
                    }
                }
                if (flagfindspec) {
                    // Чистим описание
                    description.ReplaceAll ("\n", "");
                    description.ReplaceAll ("\r", "");
                    description.ReplaceAll ("  ", "");
                    description.ReplaceAll (" {", "{");
                    description.ReplaceAll ("{ ", "{");
                    description.ReplaceAll (" }", "}");
                    description.ReplaceAll ("} ", "}");
                    description.ReplaceAll (" (", "(");
                    description.ReplaceAll ("( ", "(");
                    description.ReplaceAll (" )", ")");
                    description.ReplaceAll (") ", ")");
                    GS::UniString key = description.GetSubstring ('{', '}', 0);
                    if (rules.ContainsKey (key)) {
                        if (rules.Get (key).is_Valid) rules.Get (key).elements.Push (elemguid);
                    } else {
                        // Добавление группы и элемента
                        SpecRule rule = GetRuleFromDescription (description);
                        rule.elements.Push (elemguid);
                        rules.Add (key, rule);
                    }
                }
            }
        }
    }
    return NoError;
}

SpecRule GetRuleFromDescription (GS::UniString& description)
{
    // Разбиваем строку на части. Будем постепенно заменять на пустоту обработанные части 
    // Критерий - значение, по которому будет сгруппированы элементы
    // g(P1, P2, P3; Q1, Q2) - P1...P3 параметры, уникальные для вложенного элемента,
    //                         Q1...Q2 параметры или значения количества, будут просуммированы
    // s(Pn1, Pn2, Pn3; Qn1, Qn2) - Pn1...Pn3 параметры размещаемых объектов,
    //                              Qn1...Qn2 параметры для записи количества
    // Spec_rule {КРИТЕРИЙ ;g(P1, P2, P3; Q1, Q2) g(P4, P5, P6; Q3, Q4) s(Pn1, Pn2, Pn3; Qn1, Qn2)}
    // Получаем критерий
    SpecRule rule;
    GS::UniString criteria = description.GetSubstring ('{', ';', 0);
    description = description.GetSubstring ('{', '}', 0);
    description.ReplaceAll (criteria + ";", "");
    GS::Array<GS::UniString> rulestring_group;
    GS::Array<GS::UniString> rulestring_summ;
    GS::Array<GS::UniString> paramss;
    // Разбивка на группы и итог
    if (StringSplt (description, "s(", rulestring_summ, ")") < 1) {
        rule.is_Valid = false;
        return rule;
    }

    // Параметры для записи
    // До точки с запятой - уникальные параметры , после - параметры для суммы
    GS::Array<GS::UniString> rulestring_write;
    UInt32 nrule_write = StringSplt (rulestring_summ[1], ";", rulestring_write);
    if (nrule_write != 2) {
        rule.is_Valid = false;
        return rule;
    }

    for (UInt32 part = 0; part < nrule_write; part++) {
        GS::Array<GS::UniString> rulestring_param;
        UInt32 nrule_param = StringSplt (rulestring_write[part], ",", rulestring_param);
        if (nrule_param > 0) {
            for (UInt32 i = 0; i < nrule_param; i++) {
                FormatString formatstring;
                GS::UniString rawName = ParamHelpers::NameToRawName (rulestring_param[i], formatstring);
                if (part == 0) {
                    // Уникальные параметры
                    rule.out_unic_paramrawname.Push (rawName);
                } else {
                    // Сумма
                    rule.out_sum_paramrawname.Push (rawName);
                }
            }
        }
    }

    // Количество уникальных параметров в группе
    UInt32 n_unic = rule.out_unic_paramrawname.GetSize ();
    // Количество суммируемых параметров в группе
    UInt32 n_sum = rule.out_sum_paramrawname.GetSize ();
    // Параметры для чтения
    // До точки с запятой - уникальные парамерты , после - параметры для суммы
    UInt32 nrule_group = StringSplt (rulestring_summ[0], "g(", rulestring_group, ")");
    if (nrule_group < 1) {
        rule.is_Valid = false;
        return rule;
    }

    for (UInt32 igroup = 0; igroup < nrule_group; igroup++) {
        GroupSpec group;
        GS::Array<GS::UniString> rulestring_read;
        UInt32 nrule_read = StringSplt (rulestring_group[igroup], ";", rulestring_read);
        if (nrule_read != 2) {
            rule.is_Valid = false;
            return rule;
        }

        for (UInt32 part = 0; part < nrule_read; part++) {
            GS::Array<GS::UniString> rulestring_param;
            UInt32 nrule_param = StringSplt (rulestring_read[part], ",", rulestring_param);
            if (nrule_param > 0) {
                for (UInt32 i = 0; i < nrule_param; i++) {
                    FormatString formatstring;
                    GS::UniString rawName = ParamHelpers::NameToRawName (rulestring_param[i], formatstring);
                    if (part == 0) {
                        // Уникальные параметры
                        group.unic_paramrawname.Push (rawName);
                    } else {
                        // Сумма
                        group.sum_paramrawname.Push (rawName);
                    }
                }
            }
        }
        // Количество уникальных параметров должно быть равноколичеству параметров для запси
        if (group.unic_paramrawname.GetSize () != n_unic) {
            group.is_Valid = false;
        }
        if (group.sum_paramrawname.GetSize () != n_sum) {
            group.is_Valid = false;
        }
        if (group.is_Valid) {
            rule.groups.Push (group);
        } else {
            rule.is_Valid = false;
            return rule;
        }
    }
    return rule;
}

}
