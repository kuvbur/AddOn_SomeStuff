//------------ kuvbur 2022 ------------
#include	"ACAPinc.h"
#include	"APIEnvir.h"
#include	"Spec.hpp"
#include	"Sync.hpp"

namespace Spec
{

void ShowSub (const SyncSettings& syncSettings)
{
    ParamDictValue propertyParams;
    ParamHelpers::AllPropertyDefinitionToParamDict (propertyParams);
    ParamDictValue paramDict;
    for (auto& cItt : propertyParams) {
#if defined(AC_28)
        ParamValue param = cItt.value;
#else
        ParamValue param = *cItt.value;
#endif
        if (param.definition.description.Contains ("Sync_GUID")) {
            paramDict.Add (param.rawName, param);
        }
    }
    if (paramDict.IsEmpty ()) return;
    ParamDictElement paramToRead;
    GS::Array<API_Guid> guidArray = GetSelectedElements (true, false, syncSettings, false);
    for (UInt32 i = 0; i < guidArray.GetSize (); i++) {
        ParamHelpers::AddParamDictValue2ParamDictElement (guidArray[i], paramDict, paramToRead);
    }
    GS::Array<API_Neig> selNeigs;
    ClassificationFunc::SystemDict systemdict;
    ParamHelpers::ElementsRead (paramToRead, propertyParams, systemdict);
    for (auto& cIt : paramToRead) {
#if defined(AC_28)
        ParamDictValue params = cIt.value;
#else
        ParamDictValue params = *cIt.value;
#endif
        for (auto& cItt : params) {
#if defined(AC_28)
            ParamValue param = cItt.value;
#else
            ParamValue param = *cItt.value;
#endif
            if (param.isValid && !param.val.uniStringValue.IsEmpty ()) {
                GS::Array<GS::UniString> rulestring_param;
                UInt32 nrule_param = StringSplt (param.val.uniStringValue, ";", rulestring_param);
                if (nrule_param > 0) {
                    for (UInt32 i = 0; i < nrule_param; i++) {
                        API_Guid guid = APIGuidFromString (rulestring_param[i].ToCStr (0, MaxUSize, GChCode));
                        selNeigs.PushNew (guid);
                    }
                }
            }
        }
    }
#if defined(AC_27) || defined(AC_28)
    GSErrCode err = ACAPI_Selection_Select (selNeigs, true);
#else
    GSErrCode err = ACAPI_Element_Select (selNeigs, true);
#endif
    return;
}

GSErrCode SpecAll (const SyncSettings& syncSettings)
{
    GS::Array<API_Guid> guidArray = GetSelectedElements (false, false, syncSettings, false);
    if (guidArray.IsEmpty ()) ACAPI_Element_GetElemList (API_ZombieElemID, &guidArray, APIFilt_OnVisLayer | APIFilt_IsVisibleByRenovation | APIFilt_IsInStructureDisplay);
    if (guidArray.IsEmpty ()) return NoError;
    return SpecArray (syncSettings, guidArray);
}

GSErrCode SpecArray (const SyncSettings& syncSettings, GS::Array<API_Guid>& guidArray)
{
    long time_start = clock ();
    GS::UniString funcname = "SpecAll";
    GS::Int32 nPhase = 4;
    GS::UniString subtitle = ""; Int32 maxval = 1; short i = 1;

#if defined(AC_27) || defined(AC_28)
    bool showPercent = true;
    ACAPI_ProcessWindow_InitProcessWindow (&funcname, &nPhase);
#else
    ACAPI_Interface (APIIo_InitProcessWindowID, &funcname, &nPhase);
#endif

    subtitle = GS::UniString::Printf ("Get rule from %d elements", guidArray.GetSize ());
#if defined(AC_27) || defined(AC_28)
    maxval = 1; ACAPI_ProcessWindow_SetNextProcessPhase (&subtitle, &maxval, &showPercent);
#else
    i = 1; ACAPI_Interface (APIIo_SetNextProcessPhaseID, &subtitle, &i);
#endif
    int dummymode = IsDummyModeOn ();
    GSErrCode err = NoError;
    SpecRuleDict rules;
    bool flagfindspec = false;
    for (UInt32 i = 0; i < guidArray.GetSize (); i++) {
        err = GetRuleFromElement (guidArray[i], rules);
        if (err == NoError) flagfindspec = true;
    }
    if (!flagfindspec) {
        msg_rep ("Spec::SpecArray", "Rules not found", APIERR_GENERAL, APINULLGuid);
        return APIERR_GENERAL;
    }

    // Теперь пройдём по прочитанным правилам и сформируем список параметров для чтения
    ParamDictElement paramToRead; // Словарь с параметрами для чтения
    ParamDictValue paramToWrite;
    ParamDictValue propertyParams;
    ParamHelpers::AllPropertyDefinitionToParamDict (propertyParams);
    GetParamToReadFromRule (rules, propertyParams, paramToRead, paramToWrite);
    // TODO Добавить сообщения об ошибке
    if (paramToRead.IsEmpty ()) {
        msg_rep ("Spec::SpecArray", "Parameters for read not found", APIERR_GENERAL, APINULLGuid);
        return APIERR_GENERAL;
    }
    if (paramToWrite.IsEmpty ()) {
        msg_rep ("Spec::SpecArray", "Parameters for write not found", APIERR_GENERAL, APINULLGuid);
        return APIERR_GENERAL;
    }
    subtitle = GS::UniString::Printf ("Reading parameters from %d elements", paramToRead.GetSize ());
#if defined(AC_27) || defined(AC_28)
    maxval = 2; ACAPI_ProcessWindow_SetNextProcessPhase (&subtitle, &maxval, &showPercent);
#else
    i = 2; ACAPI_Interface (APIIo_SetNextProcessPhaseID, &subtitle, &i);
#endif

    ClassificationFunc::SystemDict systemdict;
    ParamHelpers::ElementsRead (paramToRead, propertyParams, systemdict);

    GS::Array<ElementDict> elementstocreate;
    Int32 n_elements = 0;

    GS::UniString subguid_rulename = "";
    for (auto& cItt : propertyParams) {
#if defined(AC_28)
        ParamValue param = cItt.value;
#else
        ParamValue param = *cItt.value;
#endif
        if (param.definition.description.Contains ("Spec_rule_name")) {
            paramToWrite.Add (param.rawName, param);
            subguid_rulename = param.rawName;
        }
    }

    for (auto& cIt : rules) {
#if defined(AC_28)
        SpecRule rule = cIt.value;
#else
        SpecRule rule = *cIt.value;
#endif
        if (rule.is_Valid) {
            if (!rule.subguid_paramrawname.IsEmpty ()) {
                // Ищем свойство, в которое нужно будет записать GUID
                for (auto& cItt : propertyParams) {
#if defined(AC_28)
                    ParamValue param = cItt.value;
#else
                    ParamValue param = *cItt.value;
#endif
                    if (param.definition.description.Contains (rule.subguid_paramrawname) &&
                        param.definition.description.Contains ("Sync_GUID")) {
                        paramToWrite.Add (param.rawName, param);
                        rule.subguid_paramrawname = param.rawName;
                        break;
                    }
                }
            }
            rule.subguid_rulename = subguid_rulename;
            ElementDict elements;
            n_elements += GetElementsForRule (rule, paramToRead, elements);
            if (!elements.IsEmpty ()) elementstocreate.Push (elements);
        }
    }

    subtitle = GS::UniString::Printf ("Create %d elements", n_elements);
#if defined(AC_27) || defined(AC_28)
    maxval = 3; ACAPI_ProcessWindow_SetNextProcessPhase (&subtitle, &maxval, &showPercent);
#else
    i = 3; ACAPI_Interface (APIIo_SetNextProcessPhaseID, &subtitle, &i);
#endif
    if (elementstocreate.IsEmpty ()) {
        msg_rep ("Spec::SpecArray", "Elements list empty", APIERR_GENERAL, APINULLGuid);
        return APIERR_GENERAL;
    }
    Point2D startpos;
    long time_end = clock ();
    if (!ClickAPoint ("Click point", &startpos)) return APIERR_CANCEL;
    long time_start_1 = clock ();
    ParamDictElement paramOut;
    PlaceElements (elementstocreate, paramToWrite, paramOut, startpos);
    ACAPI_CallUndoableCommand ("Write properties", [&]() -> GSErrCode {
        ParamHelpers::ElementsWrite (paramOut);
        return NoError;
    });

#if defined(AC_27) || defined(AC_28)
    ACAPI_ProcessWindow_CloseProcessWindow ();
#else
    ACAPI_Interface (APIIo_CloseProcessWindowID, nullptr, nullptr);
#endif
    GS::Array<API_Guid> guidArraysync;
    for (GS::HashTable<API_Guid, ParamDictValue>::PairIterator cIt = paramOut.EnumeratePairs (); cIt != NULL; ++cIt) {
#if defined(AC_28)
        API_Guid elemGuid = cIt->key;
#else
        API_Guid elemGuid = *cIt->key;
#endif
        guidArraysync.Push (elemGuid);
    }
    long time_end_1 = clock ();
    GS::UniString time = GS::UniString::Printf (" %d s", ((time_end - time_start) + (time_end_1 - time_start_1)) / 1000);
    GS::UniString intString = GS::UniString::Printf ("Qty elements - %d ", guidArray.GetSize ()) + GS::UniString::Printf ("wrtite to - %d", n_elements) + time;
    SyncArray (syncSettings, guidArraysync, systemdict);
    return err;
}

// --------------------------------------------------------------------
// Проверяет значение свойства с правилом и формрует правила
// --------------------------------------------------------------------
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
                // TODO Вынести это в отдельную функцию, убрать повторение в GetElemState
                API_Property propertyflag = {};
                if (ACAPI_Element_GetPropertyValue (elemguid, definitions[i].guid, propertyflag) == NoError) {
#if defined(AC_22) || defined(AC_23)
                    if (!propertyflag.isEvaluated) {
                        flagfindspec = true;
                    }
                    if (propertyflag.isDefault && !propertyflag.isEvaluated) {
                        flagfindspec = propertyflag.definition.defaultValue.basicValue.singleVariant.variant.boolValue;
                    } else {
                        flagfindspec = propertyflag.value.singleVariant.variant.boolValue;
                    }
#else
                    if (propertyflag.status == API_Property_NotAvailable) {
                        flagfindspec = true;
                    }
                    if (propertyflag.isDefault && propertyflag.status == API_Property_NotEvaluated) {
                        flagfindspec = propertyflag.definition.defaultValue.basicValue.singleVariant.variant.boolValue;
                    } else {
                        flagfindspec = propertyflag.value.singleVariant.variant.boolValue;
                    }
#endif
                }
                if (flagfindspec) {
                    // Чистим описание
                    description.ReplaceAll ("\n", "");
                    description.ReplaceAll ("\r", "");
                    description.ReplaceAll (" {", "{");
                    description.ReplaceAll ("{ ", "{");
                    description.ReplaceAll (" }", "}");
                    description.ReplaceAll ("} ", "}");
                    description.ReplaceAll (" ;", ";");
                    description.ReplaceAll ("; ", ";");
                    description.ReplaceAll ("g (", "g %");
                    description.ReplaceAll ("s (", "s %");
                    description.ReplaceAll ("g(", "g%");
                    description.ReplaceAll ("s(", "s%");
                    description.ReplaceAll (")s", "%s");
                    description.ReplaceAll (")g", "%g");
                    description.ReplaceAll ("))", ")%");
                    GS::UniString key = description.GetSubstring ('{', '}', 0);
                    if (rules.ContainsKey (key)) {
                        if (rules.Get (key).is_Valid) rules.Get (key).elements.Push (elemguid);
                    } else {
                        // Добавление группы и элемента
                        SpecRule rule = GetRuleFromDescription (description);
                        if (rule.is_Valid) {
                            GS::UniString fname;
                            GetPropertyFullName (definitions[i], fname);
                            rule.subguid_paramrawname = fname;
                            rule.subguid_rulevalue = fname;
                            rule.elements.Push (elemguid);
                        }
                        rules.Add (key, rule); //Добавляем в любом случае, чтоб потом дважды не обрабатывать
                    }
                }
            }
        }
    }
    return NoError;
}

// --------------------------------------------------------------------
// Выбирает из параметров групп имена свойств для дальнейшего чтения
// --------------------------------------------------------------------
void GetParamToReadFromRule (const SpecRuleDict& rules, ParamDictValue& propertyParams, ParamDictElement& paramToRead, ParamDictValue& paramToWrite)
{
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
                    for (UInt32 j = 0; j < rule.groups[i].out_paramrawname.GetSize (); j++) {
                        GS::UniString rawname = rule.groups[i].out_paramrawname[j];
                        if (!params.ContainsKey (rawname)) params.Add (rawname, true);
                    }
                }
                GS::UniString rawname = rule.groups[i].flag_paramrawname;
                if (!params.ContainsKey (rawname)) params.Add (rawname, true);
            }
            ParamDictValue paramDict;
            for (auto& cItt : params) {
#if defined(AC_28)
                GS::UniString rawname = cItt.key;
#else
                GS::UniString rawname = *cItt.key;
#endif
                ParamHelpers::AddValueToParamDictValue (paramDict, rawname);
            }
            // Добавляем параметры для каждого элемента
            for (UInt32 i = 0; i < rule.elements.GetSize (); i++) {
                API_Guid elemguid = rule.elements[i];
                ParamHelpers::AddParamDictValue2ParamDictElement (elemguid, paramDict, paramToRead);
            }
            // Добавляем параметры для записи
            ParamDict paramswrite;
            for (UInt32 j = 0; j < rule.out_sum_paramrawname.GetSize (); j++) {
                GS::UniString rawname = rule.out_sum_paramrawname[j];
                if (!paramswrite.ContainsKey (rawname)) paramswrite.Add (rawname, true);
            }
            for (UInt32 j = 0; j < rule.out_paramrawname.GetSize (); j++) {
                GS::UniString rawname = rule.out_paramrawname[j];
                if (!paramswrite.ContainsKey (rawname)) paramswrite.Add (rawname, true);
            }

            for (auto& cItt : paramswrite) {
#if defined(AC_28)
                GS::UniString rawname = cItt.key;
#else
                GS::UniString rawname = *cItt.key;
#endif
                ParamHelpers::AddValueToParamDictValue (paramToWrite, rawname);
            }
        }
    }
    if (!propertyParams.IsEmpty ()) ParamHelpers::CompareParamDictValue (propertyParams, paramToWrite);
}

Int32 GetElementsForRule (const SpecRule& rule, const ParamDictElement& paramToRead, ElementDict& elements)
{
    ParamDict not_found_paramname;
    Int32 n_elements = 0;
    for (UInt32 ielem = 0; ielem < rule.elements.GetSize (); ielem++) {
        API_Guid elemguid = rule.elements[ielem];
        for (UInt32 igroup = 0; igroup < rule.groups.GetSize (); igroup++) {
            if (rule.groups[igroup].is_Valid) {
                GroupSpec group = rule.groups[igroup];
                // Проверяем значение флага, если он не найден - всё равно добавляем
                bool flag = true;
                if (!group.flag_paramrawname.IsEmpty ()) {
                    ParamValue pvalue;
                    if (ParamHelpers::GetParamValueForElements (elemguid, group.flag_paramrawname, paramToRead, pvalue)) {
                        flag = pvalue.val.boolValue;
                    } else {
                        if (!not_found_paramname.ContainsKey ("flag:" + group.flag_paramrawname)) not_found_paramname.Add ("flag:" + group.flag_paramrawname, false);
                    }
                }
                if (flag) {
                    // Принадлежность субэлемента к группе определим по ключу - сцепке значений уникальных параметров
                    GS::UniString key = "";
                    Element element;
                    for (UInt32 j = 0; j < group.unic_paramrawname.GetSize (); j++) {
                        GS::UniString rawname = group.unic_paramrawname[j];
                        ParamValue pvalue;
                        if (!ParamHelpers::GetParamValueForElements (elemguid, rawname, paramToRead, pvalue)) {
                            if (!not_found_paramname.ContainsKey ("unic:" + rawname)) not_found_paramname.Add ("unic:" + rawname, false);
                        }
                        key = key + "@" + pvalue.val.uniStringValue;
                    }
                    for (UInt32 j = 0; j < group.sum_paramrawname.GetSize (); j++) {
                        GS::UniString rawname = group.sum_paramrawname[j];
                        ParamValue pvalue;
                        if (rawname.IsEqual ("1")) {
                            ParamHelpers::ConvertIntToParamValue (pvalue, rawname, 1);
                            element.out_sum_param.Push (pvalue);
                        } else {
                            if (ParamHelpers::GetParamValueForElements (elemguid, rawname, paramToRead, pvalue)) {
                                element.out_sum_param.Push (pvalue);
                            } else {
                                if (!not_found_paramname.ContainsKey ("sum:" + rawname)) not_found_paramname.Add ("sum:" + rawname, false);
                            }
                        }
                    }
                    if (elements.ContainsKey (key) && !key.IsEmpty ()) {
                        elements.Get (key).elements.Push (elemguid);
                        UInt32 nsumm = elements.Get (key).out_sum_param.GetSize ();
                        if (nsumm != element.out_sum_param.GetSize ()) {
                            nsumm = nsumm < element.out_sum_param.GetSize () ? nsumm : element.out_sum_param.GetSize ();
                        }
                        for (UInt32 j = 0; j < nsumm; j++) {
                            if (elements.Get (key).out_sum_param[j].isValid && element.out_sum_param[j].isValid)
                                elements.Get (key).out_sum_param[j].val = elements.Get (key).out_sum_param[j].val + element.out_sum_param[j].val;
                        }
                    } else {
                        for (UInt32 j = 0; j < group.out_paramrawname.GetSize (); j++) {
                            GS::UniString rawname = group.out_paramrawname[j];
                            ParamValue pvalue;
                            if (ParamHelpers::GetParamValueForElements (elemguid, rawname, paramToRead, pvalue)) {
                                element.out_param.Push (pvalue);
                            } else {
                                if (!not_found_paramname.ContainsKey (rawname)) not_found_paramname.Add ("out:" + rawname, false);
                            }
                        }
                        if (!element.out_sum_param.IsEmpty () && !element.out_param.IsEmpty () &&
                            element.out_sum_param.GetSize () == rule.out_sum_paramrawname.GetSize () &&
                            element.out_param.GetSize () == rule.out_paramrawname.GetSize ()
                            ) {
                            element.out_sum_paramrawname = rule.out_sum_paramrawname;
                            element.out_paramrawname = rule.out_paramrawname;
                            element.subguid_paramrawname = rule.subguid_paramrawname;
                            element.subguid_rulevalue = rule.subguid_rulevalue;
                            element.subguid_rulename = rule.subguid_rulename;
                            element.elements.Push (elemguid);
                            elements.Add (key, element);
                            n_elements += 1;
                        } else {
                            GS::UniString error = "";
                            if (element.out_sum_param.IsEmpty ()) error = "element.out_sum_param.IsEmpty";
                            if (!not_found_paramname.ContainsKey ("error:" + error)) not_found_paramname.Add ("error:" + error, false);
                            if (element.out_param.IsEmpty ()) error = "element.out_param.IsEmpty";
                            if (!not_found_paramname.ContainsKey ("error:" + error)) not_found_paramname.Add ("error:" + error, false);
                            if (element.out_sum_param.GetSize () < rule.out_sum_paramrawname.GetSize ()) "element.out_sum_param < rule.out_sum_paramrawname";
                            if (!not_found_paramname.ContainsKey ("error:" + error)) not_found_paramname.Add ("error:" + error, false);
                            if (element.out_sum_param.GetSize () > rule.out_sum_paramrawname.GetSize ()) "element.out_sum_param > rule.out_sum_paramrawname";
                            if (!not_found_paramname.ContainsKey ("error:" + error)) not_found_paramname.Add ("error:" + error, false);
                            if (element.out_param.GetSize () > rule.out_paramrawname.GetSize ()) "element.out_param > rule.out_paramrawname";
                            if (!not_found_paramname.ContainsKey ("error:" + error)) not_found_paramname.Add ("error:" + error, false);
                            if (element.out_param.GetSize () < rule.out_paramrawname.GetSize ()) "element.out_param < rule.out_paramrawname";
                            if (!not_found_paramname.ContainsKey ("error:" + error)) not_found_paramname.Add ("error:" + error, false);
                            n_elements = 0;
                        }
                    }
                }
            }
        }
    }
    if (!not_found_paramname.IsEmpty ()) {
        GS::UniString notfound_paramname = "";
        for (auto& cIt : not_found_paramname) {
#if defined(AC_28)
            GS::UniString name = cIt.key;
#else
            GS::UniString name = *cIt.key;
#endif
            if (notfound_paramname.IsEmpty ()) {
                notfound_paramname = name;
            } else {
                notfound_paramname = notfound_paramname + " ; " + name;
            }
        }
        notfound_paramname = "Error - " + notfound_paramname;
        msg_rep ("Spec::GetElementsForRule", notfound_paramname, APIERR_BADINDEX, APINULLGuid);
        n_elements = 0;
    }
    return n_elements;
}

// --------------------------------------------------------------------
// Разбивает строку на части. Будем постепенно заменять на пустоту обработанные части 
// Критерий - значение, по которому будет сгруппированы элементы
// g(P1, P2, P3; F; Q1, Q2) - P1...P3 параметры, уникальные для вложенного элемента,
// 
//                            F - флаг включения группы 1/0. Если не найден - всегда 1
//                            Q1...Q2 параметры или значения количества, будут просуммированы. Если их нет - запишем 1 для суммы.
// s(Pn1, Pn2, Pn3; Qn1, Qn2) - Pn1...Pn3 параметры размещаемых объектов,
//                              Qn1...Qn2 параметры для записи количества
// Spec_rule {КРИТЕРИЙ ;g(P1, P2, P3; Q1, Q2) g(P4, P5, P6; Q3, Q4) s(Pn1, Pn2, Pn3; Qn1, Qn2)}
// --------------------------------------------------------------------
SpecRule GetRuleFromDescription (GS::UniString& description)
{
    // Получаем критерий
    SpecRule rule;
    GS::UniString criteria = description.GetSubstring ('{', ';', 0);
    description = description.GetSubstring ('{', '}', 0);
    description.ReplaceAll (criteria + ";", "");
    description.Trim (')');
    description.Trim ();
    GS::Array<GS::UniString> paramss;
    // Разбивка на группы и итог
    GS::Array<GS::UniString> rulestring_summ;
    if (StringSplt (description, "s%", rulestring_summ) < 1) {
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
                GS::UniString name = rulestring_param[i];
                name.Trim ('%');
                name.Trim ('{');
                name.Trim ('}');
                name.Trim ();
                if (!name.IsEmpty ()) {
                    if (name.Contains ("[") && name.Contains ("]")) {
                        GS::UniString n_row_txt = name.GetSubstring ('[', ']', 0);
                        name.ReplaceFirst ("[" + n_row_txt + "]", "");
                    }
                    FormatString formatstring;
                    GS::UniString rawName = ParamHelpers::NameToRawName (name, formatstring);
                    if (part == 0) rule.out_paramrawname.Push (rawName);
                    if (part == 1) rule.out_sum_paramrawname.Push (rawName);
                }
            }
        }
    }
    // Параметры для чтения
    // До точки с запятой - уникальные парамерты , после - параметры для суммы
    GS::Array<GS::UniString> rulestring_group;
    UInt32 nrule_group = StringSplt (rulestring_summ[0], "g%", rulestring_group);
    if (nrule_group < 1) {
        rule.is_Valid = false;
        return rule;
    }


    for (UInt32 igroup = 0; igroup < nrule_group; igroup++) {
        GS::Array<GS::UniString> rulestring_read;
        UInt32 nrule_read = StringSplt (rulestring_group[igroup], ";", rulestring_read);
        if (nrule_read < 1) {
            rule.is_Valid = false;
            return rule;
        }
        GroupSpec group;
        Int32 min_row = 0;
        for (UInt32 part = 0; part < nrule_read; part++) {
            GS::Array<GS::UniString> rulestring_param;
            UInt32 nrule_param = StringSplt (rulestring_read[part], ",", rulestring_param);
            if (nrule_param > 0) {
                for (UInt32 i = 0; i < nrule_param; i++) {
                    GS::UniString name = rulestring_param[i];
                    name.Trim ('%');
                    name.Trim ('{');
                    name.Trim ('}');
                    name.Trim ();
                    if (part == 1 && name.IsEqual ("-")) {
                        name = "";
                    } else {
                        if (!name.IsEmpty ()) {
                            if (name.Contains ("[") && name.Contains ("]")) {
                                GS::UniString n_row_txt = name.GetSubstring ('[', ']', 0);
                                double doubleValue = 0;
                                Int32 n_row = 10;
                                if (UniStringToDouble (n_row_txt, doubleValue)) {
                                    n_row = (GS::Int32) doubleValue;
                                }
                                if (min_row == 0) min_row = n_row;
                                min_row = n_row < min_row ? n_row : min_row;
                            }
                            FormatString formatstring;
                            GS::UniString rawName = ParamHelpers::NameToRawName (name, formatstring);
                            if (part == 0) group.unic_paramrawname.Push (rawName);
                            if (part == 1) group.out_paramrawname.Push (rawName);
                            if (part == 2) group.flag_paramrawname = rawName;
                            if (part == 3) group.sum_paramrawname.Push (rawName);
                        }
                    }
                }
            }
        }
        // Добавим значения для суммы
        if (group.sum_paramrawname.GetSize () < rule.out_sum_paramrawname.GetSize ()) {
            for (UInt32 i = group.sum_paramrawname.GetSize (); i < rule.out_sum_paramrawname.GetSize (); i++) {
                group.sum_paramrawname.Push ("1");
            }
        }
        if (min_row > 0) {
            // создаём группы для параметров с массивами
            for (Int32 jj = 1; jj <= min_row; jj++) {
                GroupSpec group_add;

                for (UInt32 i = 0; i < group.out_paramrawname.GetSize (); i++) {
                    GS::UniString rawName = group.out_paramrawname[i];
                    if (rawName.Contains ("[") && rawName.Contains ("]")) {
                        GS::UniString n_row_txt = rawName.GetSubstring ('[', ']', 0);
                        rawName.ReplaceFirst ("[" + n_row_txt + "]", "");
                        rawName.ReplaceAll ("}", GS::UniString::Printf ("@arr_%d_%d_%d_%d_%d", jj, jj, 1, 1, ARRAY_UNIC) + "}");
                    }
                    group_add.out_paramrawname.Push (rawName);
                }

                for (UInt32 i = 0; i < group.unic_paramrawname.GetSize (); i++) {
                    GS::UniString rawName = group.unic_paramrawname[i];
                    if (rawName.Contains ("[") && rawName.Contains ("]")) {
                        GS::UniString n_row_txt = rawName.GetSubstring ('[', ']', 0);
                        rawName.ReplaceFirst ("[" + n_row_txt + "]", "");
                        rawName.ReplaceAll ("}", GS::UniString::Printf ("@arr_%d_%d_%d_%d_%d", jj, jj, 1, 1, ARRAY_UNIC) + "}");
                    }
                    group_add.unic_paramrawname.Push (rawName);
                }


                GS::UniString rawName = group.flag_paramrawname;
                if (rawName.Contains ("[") && rawName.Contains ("]")) {
                    GS::UniString n_row_txt = rawName.GetSubstring ('[', ']', 0);
                    rawName.ReplaceFirst ("[" + n_row_txt + "]", "");
                    rawName.ReplaceAll ("}", GS::UniString::Printf ("@arr_%d_%d_%d_%d_%d", jj, jj, 1, 1, ARRAY_UNIC) + "}");
                }
                group_add.flag_paramrawname = rawName;

                for (UInt32 i = 0; i < group.sum_paramrawname.GetSize (); i++) {
                    GS::UniString rawName = group.sum_paramrawname[i];
                    if (rawName.Contains ("[") && rawName.Contains ("]")) {
                        GS::UniString n_row_txt = rawName.GetSubstring ('[', ']', 0);
                        rawName.ReplaceFirst ("[" + n_row_txt + "]", "");
                        rawName.ReplaceAll ("}", GS::UniString::Printf ("@arr_%d_%d_%d_%d_%d", jj, jj, 1, 1, ARRAY_UNIC) + "}");
                    }
                    group_add.sum_paramrawname.Push (rawName);
                }
                rule.groups.Push (group_add);
            }
        } else {
            if (group.out_paramrawname.GetSize () != rule.out_paramrawname.GetSize ()) {
                group.is_Valid = false;
                msg_rep ("Spec::GetRuleFromDescription", "group.out_paramrawname.GetSize () != rule.out_paramrawname.GetSize ()", APIERR_BADINDEX, APINULLGuid);
            }
            if (group.sum_paramrawname.GetSize () != rule.out_sum_paramrawname.GetSize ()) {
                group.is_Valid = false;
                msg_rep ("Spec::GetRuleFromDescription", "group.sum_paramrawname.GetSize () != rule.out_sum_paramrawname.GetSize ()", APIERR_BADINDEX, APINULLGuid);
            }
            if (group.is_Valid) rule.groups.Push (group);
        }
    }
    if (rule.groups.IsEmpty ()) rule.is_Valid = false;
    if (rule.out_paramrawname.IsEmpty ()) rule.is_Valid = false;
    if (rule.out_sum_paramrawname.IsEmpty ()) rule.is_Valid = false;
    return rule;
}

GSErrCode PlaceElements (GS::Array<ElementDict>& elementstocreate, ParamDictValue& paramToWrite, ParamDictElement& paramOut, Point2D& startpos)
{
    API_Element elementt = {};
    API_ElementMemo memot = {};
#if defined AC_26 || defined AC_27 || defined AC_28
    elementt.header.type.typeID = API_ObjectID;
#else
    elementt.header.typeID = API_ObjectID;
#endif
    GSErrCode err = ACAPI_Element_GetDefaults (&elementt, &memot);
    if (err != NoError) {
        ACAPI_DisposeElemMemoHdls (&memot);
        msg_rep ("Spec::PlaceElements", "ACAPI_Element_GetDefaults", err, APINULLGuid);
        return err;
    }
    double dx = 0; double dy = 0;
    GS::Array <API_AddParType> params;
    GS::Array <API_Elem_Head> elemsheader;
    const GSSize nParams = BMGetHandleSize ((GSHandle) memot.params) / sizeof (API_AddParType);
    for (GSIndex ii = 0; ii < nParams; ++ii) {
        API_AddParType& actParam = (*memot.params)[ii];
        params.Push ((*memot.params)[ii]);
        const GS::String name (actParam.name);
        if (name.IsEqual ("A")) {
            dx = actParam.value.real;
        }
        if (name.IsEqual ("B")) {
            dy = actParam.value.real;
        }
    }
    ACAPI_DisposeElemMemoHdls (&memot);

    API_Coord pos = { startpos.x, startpos.y };
    ACAPI_CallUndoableCommand ("Create Element", [&]() -> GSErrCode {
        int n_elem = 0;
        for (UInt32 i = 0; i < elementstocreate.GetSize (); i++) {
            GS::Array<API_Guid> group;
            for (auto& cIt : elementstocreate[i]) {
#if defined(AC_28)
                Element el = cIt.value;
#else
                Element el = *cIt.value;
#endif

                API_Element element = {};
                API_ElementMemo memo = {};
#if defined AC_26 || defined AC_27 || defined AC_28
                element.header.type.typeID = API_ObjectID;
#else
                element.header.typeID = API_ObjectID;
#endif
                err = ACAPI_Element_GetDefaults (&element, &memo);
                if (err != NoError) {
                    ACAPI_DisposeElemMemoHdls (&memo);
                    msg_rep ("Spec::PlaceElements", "ACAPI_Element_GetDefaults", err, APINULLGuid);
                    return err;
                }
                // Запись параметров
                ParamDictValue param;
                if (!el.subguid_paramrawname.IsEmpty ()) {
                    if (paramToWrite.ContainsKey (el.subguid_paramrawname)) {
                        ParamValue paramTo = paramToWrite.Get (el.subguid_paramrawname);
                        GS::UniString instring = APIGuidToString (el.elements[0]);
                        for (UInt32 k = 1; k < el.elements.GetSize (); k++) {
                            instring = instring + ";" + APIGuid2GSGuid (el.elements[k]).ToUniString ();
                        }
                        paramTo.val.uniStringValue = StringUnic (instring, ";");
                        paramTo.isValid = true;
                        paramTo.val.type = API_PropertyStringValueType;
                        param.Add (el.subguid_paramrawname, paramTo);
                    }
                }
                if (!el.subguid_rulename.IsEmpty () && !el.subguid_rulevalue.IsEmpty ()) {
                    if (paramToWrite.ContainsKey (el.subguid_rulename)) {
                        ParamValue paramTo = paramToWrite.Get (el.subguid_rulename);
                        paramTo.val.uniStringValue = el.subguid_rulevalue;
                        paramTo.isValid = true;
                        paramTo.val.type = API_PropertyStringValueType;
                        param.Add (el.subguid_rulename, paramTo);
                    }
                }
                // GDL параметры сразу запишем в memo
                for (UInt32 k = 0; k < el.out_paramrawname.GetSize (); k++) {
                    GS::UniString rawname = el.out_paramrawname[k];
                    if (paramToWrite.ContainsKey (rawname)) {
                        FormatString stringformat;
                        ParamValue paramFrom = el.out_param[k];
                        ParamValue paramTo = paramToWrite.Get (rawname);
                        paramTo.val = paramFrom.val;
                        paramTo.isValid = true;
                        param.Add (rawname, paramTo);
                    }
                }
                for (UInt32 k = 0; k < el.out_sum_paramrawname.GetSize (); k++) {
                    GS::UniString rawname = el.out_sum_paramrawname[k];
                    if (paramToWrite.ContainsKey (rawname)) {
                        FormatString stringformat;
                        ParamValue paramFrom = el.out_sum_param[k];
                        ParamValue paramTo = paramToWrite.Get (rawname);
                        paramTo.val = paramFrom.val;
                        paramTo.val.uniStringValue = StringUnic (paramFrom.val.uniStringValue, ";");
                        paramTo.isValid = true;
                        param.Add (rawname, paramTo);
                    }
                }
                for (GSIndex ii = 0; ii < nParams; ++ii) {
                    API_AddParType& actParam = (*memo.params)[ii];
                    const GS::String name (actParam.name);
                    GS::UniString rawname = "";
                    bool flag_find = false;
                    if (actParam.typeMod == API_ParSimple) {
                        rawname = "{@gdl:" + name.ToLowerCase () + "}";
                        flag_find = param.ContainsKey (rawname);
                    }
                    //if (actParam.typeMod == API_ParArray) {
                    //    for (Int32 kdim1 = 0; kdim1 < actParam.dim1; kdim1++) {
                    //        for (Int32 jdim2 = 0; jdim2 < actParam.dim2; jdim2++) {
                    //            rawname = "{@gdl:" + name.ToLowerCase () + GS::UniString::Printf ("@arr_%d_%d_%d_%d_%d", kdim1 + 1, kdim1 + 1, jdim2 + 1, jdim2 + 1, ARRAY_UNIC) + "}";
                    //            if (param.ContainsKey (rawname)) {
                    //                flag_find = true;
                    //            }
                    //        }
                    //    } 
                    //}
                    if (actParam.typeMod == API_ParSimple && flag_find) {
                        ParamValueData paramfrom = param.Get (rawname).val;
                        switch (actParam.typeID) {
                            case APIParT_ColRGB:
                            case APIParT_Intens:
                            case APIParT_Length:
                            case APIParT_RealNum:
                            case APIParT_Angle:
                                if (actParam.typeMod == API_ParSimple) actParam.value.real = paramfrom.doubleValue;
                                if (actParam.typeMod == API_ParArray) {
                                    double** arrHdl = reinterpret_cast<double**>(actParam.value.array);
                                    for (Int32 kdim1 = 0; kdim1 < actParam.dim1; kdim1++) {
                                        for (Int32 jdim2 = 0; jdim2 < actParam.dim2; jdim2++) {
                                            if (kdim1 == paramfrom.array_row_start && jdim2 == paramfrom.array_column_start) {
                                                (*arrHdl)[kdim1 * actParam.dim2 + jdim2] = paramfrom.doubleValue;
                                            }
                                        }
                                    }
                                }
                                break;
                            case APIParT_Boolean:
                                if (actParam.typeMod == API_ParSimple) actParam.value.real = paramfrom.doubleValue;
                                break;
                            case APIParT_Integer:
                            case APIParT_PenCol:
                            case APIParT_LineTyp:
                            case APIParT_Mater:
                            case APIParT_FillPat:
                            case APIParT_BuildingMaterial:
                            case APIParT_Profile:
                                if (actParam.typeMod == API_ParSimple) actParam.value.real = paramfrom.intValue;
                                break;
                            case APIParT_CString:
                            case APIParT_Title:
                                if (actParam.typeMod == API_ParSimple) {
                                    GS::ucscpy (actParam.value.uStr, paramfrom.uniStringValue.ToUStr (0, GS::Min (paramfrom.uniStringValue.GetLength (), (USize) API_UAddParStrLen)).Get ());
                                }
                                break;
                            default:
                            case APIParT_Dictionary:
                                break;
                        }
                        param.Delete (rawname);
                    }
                    /*                   if (actParam.typeMod == API_ParArray && flag_find) {
                                           double** arrHdl = reinterpret_cast<double**>(actParam.value.array);
                                           switch (actParam.typeID) {
                                               case APIParT_ColRGB:
                                               case APIParT_Intens:
                                               case APIParT_Length:
                                               case APIParT_RealNum:
                                               case APIParT_Angle:
                                                   for (Int32 kdim1 = 0; kdim1 < actParam.dim1; kdim1++) {
                                                       for (Int32 jdim2 = 0; jdim2 < actParam.dim2; jdim2++) {
                                                           rawname = "{@gdl:" + name.ToLowerCase () + GS::UniString::Printf ("@arr_%d_%d_%d_%d_%d", kdim1 + 1, kdim1 + 1, jdim2 + 1, jdim2 + 1, ARRAY_UNIC) + "}";
                                                           if (param.ContainsKey (rawname)) {
                                                               (*arrHdl)[kdim1 * actParam.dim2 + jdim2] = param.Get (rawname).val.doubleValue;
                                                           }
                                                       }
                                                   }

                                                   break;
                                               case APIParT_Boolean:

                                                   break;
                                               case APIParT_Integer:
                                               case APIParT_PenCol:
                                               case APIParT_LineTyp:
                                               case APIParT_Mater:
                                               case APIParT_FillPat:
                                               case APIParT_BuildingMaterial:
                                               case APIParT_Profile:

                                                   break;
                                               case APIParT_CString:
                                               case APIParT_Title:
                                                   break;
                                               default:
                                               case APIParT_Dictionary:
                                                   break;
                                           }
                                       }*/
                }
                element.object.pos = pos;
                err = ACAPI_Element_Create (&element, &memo);
                if (err == NoError) {
                    elemsheader.Push (element.header);
                    n_elem += 1;
                    if (n_elem % 8 == 0) {
                        pos.x = startpos.x;
                        pos.y += dy;
                    } else {
                        pos.x += dx;
                    }
                    paramOut.Add (element.header.guid, param);
                    group.Push (element.header.guid);
                } else {
                    msg_rep ("Spec::PlaceElements", "ACAPI_Element_Create", err, APINULLGuid);
                }
                ACAPI_DisposeElemMemoHdls (&memo);
        }
            pos.y += 2 * dy;
            if (group.GetSize () > 1) {
                API_Guid groupGuid = APINULLGuid;
#if defined(AC_27) || defined(AC_28)
                err = ACAPI_Grouping_CreateGroup (group, &groupGuid);
#else
                err = ACAPI_ElementGroup_Create (group, &groupGuid);
#endif
                if (err != NoError) msg_rep ("Spec::PlaceElements", "ACAPI_ElementGroup_Create", err, APINULLGuid);
            }
    }

        return NoError;
});
    for (UInt32 i = 0; i < elemsheader.GetSize (); i++) {
#if defined(AC_27) || defined(AC_28)
        err = ACAPI_LibraryManagement_RunGDLParScript (&elemsheader[i], 0);
#else
        err = ACAPI_Goodies (APIAny_RunGDLParScriptID, &elemsheader[i], 0);
#endif
        if (err != NoError) msg_rep ("Spec::PlaceElements", "APIAny_RunGDLParScriptID", err, APINULLGuid);
}
    return NoError;
}
    }
