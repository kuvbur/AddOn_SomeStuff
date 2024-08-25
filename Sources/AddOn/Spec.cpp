//------------ kuvbur 2022 ------------
#include	"ACAPinc.h"
#include	"APIEnvir.h"
#include	"Spec.hpp"

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
    GSErrCode err = ACAPI_Element_Select (selNeigs, true);
    return;
}

GSErrCode SpecAll (const SyncSettings& syncSettings)
{
    GS::Array<API_Guid> guidArray;
    ACAPI_Element_GetElemList (API_ZombieElemID, &guidArray, APIFilt_OnVisLayer | APIFilt_IsVisibleByRenovation | APIFilt_IsInStructureDisplay);
    if (guidArray.IsEmpty ()) return NoError;
    return SpecArray (syncSettings, guidArray);
}

GSErrCode SpecArray (const SyncSettings& syncSettings, GS::Array<API_Guid>& guidArray)
{
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
    if (!flagfindspec) return NoError;

    // Теперь пройдём по прочитанным правилам и сформируем список параметров для чтения
    ParamDictElement paramToRead; // Словарь с параметрами для чтения
    ParamDictValue paramToWrite;
    ParamDictValue propertyParams;
    ParamHelpers::AllPropertyDefinitionToParamDict (propertyParams);
    GetParamToReadFromRule (rules, propertyParams, paramToRead, paramToWrite);

    subtitle = GS::UniString::Printf ("Reading parameters from %d elements", paramToRead.GetSize ());
#if defined(AC_27) || defined(AC_28)
    maxval = 2; ACAPI_ProcessWindow_SetNextProcessPhase (&subtitle, &maxval, &showPercent);
#else
    i = 2; ACAPI_Interface (APIIo_SetNextProcessPhaseID, &subtitle, &i);
#endif

    // Читаем все возможные свойства
    ClassificationFunc::SystemDict systemdict;
    ParamHelpers::ElementsRead (paramToRead, propertyParams, systemdict);
    if (!propertyParams.IsEmpty ()) ParamHelpers::CompareParamDictValue (propertyParams, paramToWrite);

    GS::Array<ElementDict> elementstocreate;
    Int32 n_elements = 0;
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


    Point2D startpos;
    if (!ClickAPoint ("Click point", &startpos))
        return NoError;
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
                API_Property propertyflag = {};
                if (ACAPI_Element_GetPropertyValue (elemguid, definitions[i].guid, propertyflag) == NoError) {
                    // TODO Добавить проверку propertyflag.definition.defaultValue.hasExpression в остальные функции
                    if (propertyflag.isDefault && !propertyflag.definition.defaultValue.hasExpression) {
                        flagfindspec = propertyflag.definition.defaultValue.basicValue.singleVariant.variant.boolValue;
                    } else {
                        flagfindspec = propertyflag.value.singleVariant.variant.boolValue;
                    }
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
                        GS::UniString fname;
                        GetPropertyFullName (definitions[i], fname);
                        rule.subguid_paramrawname = fname;
                        rule.elements.Push (elemguid);
                        rules.Add (key, rule);
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
                }
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
            if (!propertyParams.IsEmpty ()) ParamHelpers::CompareParamDictValue (propertyParams, paramDict);
            // Добавляем параметры для каждого элемента
            for (UInt32 i = 0; i < rule.elements.GetSize (); i++) {
                API_Guid elemguid = rule.elements[i];
                ParamHelpers::AddParamDictValue2ParamDictElement (elemguid, paramDict, paramToRead);
            }

            ParamDict paramswrite;
            for (UInt32 j = 0; j < rule.out_sum_paramrawname.GetSize (); j++) {
                GS::UniString rawname = rule.out_sum_paramrawname[j];
                if (!paramswrite.ContainsKey (rawname)) paramswrite.Add (rawname, true);
            }
            for (UInt32 j = 0; j < rule.out_unic_paramrawname.GetSize (); j++) {
                GS::UniString rawname = rule.out_unic_paramrawname[j];
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
            }

Int32 GetElementsForRule (const SpecRule& rule, const ParamDictElement& paramToRead, ElementDict& elements)
{
    Int32 n_elements = 0;
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
                    element.out_unic_param.Push (pvalue);
                }

                for (UInt32 j = 0; j < rule.groups[i].sum_paramrawname.GetSize (); j++) {
                    GS::UniString rawname = rule.groups[i].sum_paramrawname[j];
                    ParamValue pvalue;
                    if (!ParamHelpers::GetParamValueForElements (elemguid, rawname, paramToRead, pvalue)) {
                        ParamHelpers::ConvertIntToParamValue (pvalue, rawname, 1);
                    }
                    element.out_sum_param.Push (pvalue);
                }

                if (elements.ContainsKey (key)) {
                    elements.Get (key).elements.Push (elemguid);
                    for (UInt32 j = 0; j < elements.Get (key).out_sum_param.GetSize (); j++) {
                        elements.Get (key).out_sum_param[j].val = elements.Get (key).out_sum_param[j].val + element.out_sum_param[j].val;
                    }
                } else {
                    if (!element.out_sum_param.IsEmpty () && !element.out_unic_param.IsEmpty ()) {
                        element.out_sum_paramrawname = rule.out_sum_paramrawname;
                        element.out_unic_paramrawname = rule.out_unic_paramrawname;
                        element.subguid_paramrawname = rule.subguid_paramrawname;
                        element.elements.Push (elemguid);
                        elements.Add (key, element);
                        n_elements += 1;
                    }
                }
            }
        }
    }
    return n_elements;
}

// --------------------------------------------------------------------
// Разбивает строку на части. Будем постепенно заменять на пустоту обработанные части 
// Критерий - значение, по которому будет сгруппированы элементы
// g(P1, P2, P3; Q1, Q2) - P1...P3 параметры, уникальные для вложенного элемента,
//                         Q1...Q2 параметры или значения количества, будут просуммированы
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
    GS::Array<GS::UniString> rulestring_group;
    GS::Array<GS::UniString> rulestring_summ;
    GS::Array<GS::UniString> paramss;
    // Разбивка на группы и итог
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
                GS::UniString rawName = ParamHelpers::NameToRawName (name, formatstring);
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
    UInt32 nrule_group = StringSplt (rulestring_summ[0], "g%", rulestring_group);
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
                    GS::UniString name = rulestring_param[i];
                    name.Trim (')');
                    name.Trim ('(');
                    name.Trim ('{');
                    name.Trim ('}');
                    name.Trim ();
                    FormatString formatstring;
                    GS::UniString rawName = ParamHelpers::NameToRawName (name, formatstring);
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
        // TODO Добавить сообщение об ошибках
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

GSErrCode PlaceElements (GS::Array<ElementDict>& elementstocreate, ParamDictValue& paramToWrite, ParamDictElement& paramOut, Point2D& startpos)
{
    API_Element element = {};
    API_ElementMemo memo = {};
    element.header.typeID = API_ElemTypeID::API_ObjectID;
    GSErrCode err = ACAPI_Element_GetDefaults (&element, &memo);
    double dx = 0; double dy = 0;
    GS::Array <API_AddParType> params;
    GS::Array <API_Elem_Head> elemsheader;
    const GSSize nParams = BMGetHandleSize ((GSHandle) memo.params) / sizeof (API_AddParType);
    for (GSIndex ii = 0; ii < nParams; ++ii) {
        API_AddParType& actParam = (*memo.params)[ii];
        params.Push ((*memo.params)[ii]);
        const GS::String name (actParam.name);
        if (name.IsEqual ("A")) {
            dx = actParam.value.real;
        }
        if (name.IsEqual ("B")) {
            dy = actParam.value.real;
        }
    }
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
                // GDL параметры сразу запишем в memo
                for (UInt32 k = 0; k < el.out_unic_paramrawname.GetSize (); k++) {
                    GS::UniString rawname = el.out_unic_paramrawname[k];
                    if (paramToWrite.ContainsKey (rawname)) {
                        FormatString stringformat;
                        ParamValue paramFrom = el.out_unic_param[k];
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
                    actParam = params[ii];
                    const GS::String name (actParam.name);
                    GS::UniString rawname = "{@gdl:" + name.ToLowerCase () + "}";
                    if (param.ContainsKey (rawname)) {
                        ParamValueData paramfrom = param.Get (rawname).val;
                        switch (actParam.typeID) {
                            case APIParT_ColRGB:
                            case APIParT_Intens:
                            case APIParT_Length:
                            case APIParT_RealNum:
                            case APIParT_Angle:
                                actParam.value.real = paramfrom.doubleValue;	break;
                            case APIParT_Boolean:
                                actParam.value.real = paramfrom.doubleValue;	break;
                            case APIParT_Integer:
                            case APIParT_PenCol:
                            case APIParT_LineTyp:
                            case APIParT_Mater:
                            case APIParT_FillPat:
                            case APIParT_BuildingMaterial:
                            case APIParT_Profile:
                                actParam.value.real = paramfrom.intValue;	break;
                            case APIParT_CString:
                            case APIParT_Title:
                                GS::ucscpy (actParam.value.uStr, paramfrom.uniStringValue.ToUStr (0, GS::Min (paramfrom.uniStringValue.GetLength (), (USize) API_UAddParStrLen)).Get ());
                            default:
                            case APIParT_Dictionary:
                                break;
                        }
                        param.Delete (rawname);
                    }
                }
                element.object.pos = pos;
                ACAPI_Element_Create (&element, &memo);
                elemsheader.Push (element.header);
                n_elem += 1;
                if (n_elem % 5 == 0) {
                    pos.x = startpos.x;
                    pos.y += dy;
                } else {
                    pos.x += dx;
                }
                paramOut.Add (element.header.guid, param);
                group.Push (element.header.guid);
            }
            pos.y += 2 * dy;
            API_Guid groupGuid = APINULLGuid;
            err = ACAPI_ElementGroup_Create (group, &groupGuid);
    }
        return NoError;
    });
    ACAPI_DisposeElemMemoHdls (&memo);
    for (UInt32 i = 0; i < elemsheader.GetSize (); i++) {
#if defined(AC_27) || defined(AC_28)
        err = ACAPI_LibraryManagement_RunGDLParScript (&elemsheader[i], 0);
#else
        err = ACAPI_Goodies (APIAny_RunGDLParScriptID, &elemsheader[i], 0);
#endif
    }
    return NoError;
}
        }
