//------------ kuvbur 2022 ------------
#ifdef TESTING
#include "ACAPinc.h"
#include "APIEnvir.h"
#include  "Helpers.hpp"
#include "TestFunc.hpp"

namespace TestFunc
{

void Test ()
{
    DBprnt ("TEST", "start");
    TestFormatString ();
    TestCalc ();
    TestFormula ();
    DBprnt ("TEST", "end");
}

void TestCalc ()
{
    DBprnt ("TEST", "TestCalc");
    bool usl = false;
    GS::UniString test_expression = "";
    GS::UniString rep = "";

    test_expression = "2*2"; rep = test_expression;
    DBtest (!EvalExpression (test_expression), rep, true); DBtest (test_expression, rep, rep, true);

    test_expression = "<2*2>"; rep = test_expression;
    DBtest (EvalExpression (test_expression), rep, true); DBtest (test_expression, "4", rep, true);

    test_expression = "<2*2>+<2*2>"; rep = test_expression;
    DBtest (EvalExpression (test_expression), rep, true); DBtest (test_expression, "4+4", rep, true);

    test_expression = "<0.001+0.001>.0mm"; rep = test_expression;
    DBtest (EvalExpression (test_expression), rep, true); DBtest (test_expression, "2", rep, true);

    test_expression = "<0,001+0,001>.0mm"; rep = test_expression;
    DBtest (EvalExpression (test_expression), rep, true); DBtest (test_expression, "2", rep, true);

    test_expression = "<0.001+0.001>.3m"; rep = test_expression;
    DBtest (EvalExpression (test_expression), rep, true); DBtest (test_expression, "0,002", rep, true);

    test_expression = "<0.001+0.001>.3mp"; rep = test_expression;
    DBtest (EvalExpression (test_expression), rep, true); DBtest (test_expression, "0.002", rep, true);

    test_expression = "<0.001+0.001>.3mp+<0.001+0.001>.03mm"; rep = test_expression;
    DBtest (EvalExpression (test_expression), rep, true); DBtest (test_expression, "0.002+2,000", rep, true);

    return;
}

void TestFormula ()
{
    DBprnt ("TEST", "TestReadFormula");
    ParamDictValue paramByType;
    ParamDictValue params;
    ParamValue pvalue;
    pvalue.val.hasFormula = true;
    pvalue.isValid = true;
    GS::Array<ParamValue> formula;

    pvalue.name = "<%ac_postWidth%*1000> * <%ac_postThickness%*1000>";
    pvalue.val.uniStringValue = "53 * 3";
    pvalue.val.intValue = 159; pvalue.val.doubleValue = 159;
    formula.Push (pvalue);

    pvalue.name = "<%ac_postWidth.3m%*2>";
    pvalue.val.uniStringValue = "0,106";
    pvalue.val.intValue = 1; pvalue.val.doubleValue = 0.106;
    formula.Push (pvalue);

    pvalue.name = "<%ac_postWidth.2m%*2>.03mp";
    pvalue.val.uniStringValue = "0.100";
    pvalue.val.intValue = 1; pvalue.val.doubleValue = 0.100;
    formula.Push (pvalue);

    pvalue.name = "<%ac_postWidth.3m%*3>.2m";
    pvalue.val.uniStringValue = "0,16";
    pvalue.val.intValue = 1; pvalue.val.doubleValue = 0.160;
    formula.Push (pvalue);

    pvalue.name = "<%ac_postWidth.2m%*3>.03m";
    pvalue.val.uniStringValue = "0,150";
    pvalue.val.intValue = 1; pvalue.val.doubleValue = 0.150;
    formula.Push (pvalue);

    pvalue.name = "<%ac_postWidth.2m%*3>.03mp";
    pvalue.val.uniStringValue = "0.150";
    pvalue.val.intValue = 1; pvalue.val.doubleValue = 0.150;
    formula.Push (pvalue);


    for (UInt32 j = 0; j < formula.GetSize (); j++) {
        GS::UniString f = formula.Get (j).name;
        pvalue.name = f;
        pvalue.val.uniStringValue = f;
        pvalue.rawName = "{@formula:" + f.ToLowerCase () + ";" + pvalue.val.uniStringValue + "}";
        GS::UniString templatestring = pvalue.val.uniStringValue;
        DBtest (ParamHelpers::ParseParamNameMaterial (templatestring, params, false), templatestring, true);
        pvalue.val.uniStringValue = templatestring;
        formula[j].rawName = pvalue.rawName;
        paramByType.Add (pvalue.rawName, pvalue);
    }

    DBtest (params.ContainsKey ("{@gdl:ac_postwidth}"), "{@gdl:ac_postwidth}", true);
    pvalue.name = ""; pvalue.rawName = "";
    ParamHelpers::ConvertDoubleToParamValue (pvalue, "ac_postWidth", 0.053);
    DBtest (params.ContainsKey (pvalue.rawName), pvalue.rawName, true);
    params.Set (pvalue.rawName, pvalue);

    DBtest (params.ContainsKey ("{@gdl:ac_postthickness}"), "{@gdl:ac_postthickness}", true);
    pvalue.name = ""; pvalue.rawName = "";
    ParamHelpers::ConvertDoubleToParamValue (pvalue, "ac_postThickness", 0.003);
    DBtest (params.ContainsKey (pvalue.rawName), pvalue.rawName, true);
    params.Set (pvalue.rawName, pvalue);

    DBtest (ParamHelpers::ReadFormula (paramByType, params), "ReadFormula", true);

    for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = paramByType.EnumeratePairs (); cIt != NULL; ++cIt) {
#if defined(AC_28)
        ParamValue& param = cIt->value;
#else
        ParamValue& param = *cIt->value;
#endif
        if (param.isValid && param.val.canCalculate) {
            ParamHelpers::ConvertByFormatString (param);
        }
    }

    for (UInt32 j = 0; j < formula.GetSize (); j++) {
        ParamValue rezult = formula.Get (j);
        DBtest (paramByType.ContainsKey (rezult.rawName), rezult.rawName, true);
        ParamValue test = paramByType.Get (rezult.rawName);
        DBtest (test.val.formatstring.stringformat, rezult.val.formatstring.stringformat, "formatstring", true);
        DBtest (test.isValid == rezult.isValid, "isValid", true);
        DBtest (test.val.uniStringValue, rezult.val.uniStringValue, "uniStringValue", true);
        DBtest (test.val.intValue, rezult.val.intValue, "intValue", true);
        DBtest (test.val.doubleValue, rezult.val.doubleValue, "doubleValue", true);
    }
    return;
}

void TestFormatString ()
{
    DBprnt ("TEST", "TestFormatString");
    GS::Array<GS::UniString> tests;
    GS::Array<FormatString> rezult_format;
    GS::Array<GS::UniString> rezult_name;

    GS::UniString test_expression = "";
    GS::UniString test_name = "";
    FormatString fstring;

    // =============================================================================
    fstring.stringformat = ""; fstring.n_zero = 3;
    // =============================================================================
    test_expression = "ConWidth_1"; test_name = "ConWidth_1";
    tests.Push (test_expression); rezult_format.Push (fstring); rezult_name.Push (test_name);

    // =============================================================================
    fstring.isEmpty = false; fstring.isRead = true; fstring.koeff = 1000;
    fstring.stringformat = "0mm"; fstring.n_zero = 0;
    // =============================================================================
    test_expression = "ConWidth_1.0mm"; test_name = "ConWidth_1";
    tests.Push (test_expression); rezult_format.Push (fstring); rezult_name.Push (test_name);

    DBtest (tests.GetSize () == rezult_format.GetSize (), "tests.GetSize() == rezult_format.GetSize ()", true);
    DBtest (tests.GetSize () == rezult_name.GetSize (), "tests.GetSize() == rezult_name.GetSize ()", true);
    for (UInt32 j = 0; j < tests.GetSize (); j++) {
        GS::UniString test_name = tests.Get (j);
        GS::UniString expressiont = tests.Get (j);
        GS::UniString expressionr = rezult_name.Get (j);
        GS::UniString stringformat_raw = "";
        FormatString fstringt;
        if (expressiont.Contains ('"')) {
            GS::UniString templatestring = expressiont.GetSubstring ('"', '"', 0);
            FormatStringFunc::GetFormatStringFromFormula (expressiont, templatestring, stringformat_raw);
            fstringt = FormatStringFunc::ParseFormatString (stringformat_raw);
        }
        if (expressiont.Contains ('<') && expressiont.Contains ('>')) {
            GS::UniString templatestring = expressiont.GetSubstring ('<', '>', 0);
            FormatStringFunc::GetFormatStringFromFormula (expressiont, templatestring, stringformat_raw);
            fstringt = FormatStringFunc::ParseFormatString (stringformat_raw);
        }
        if (!expressiont.Contains ('"') && !expressiont.Contains ('<') && !expressiont.Contains ('>')) {
            stringformat_raw = FormatStringFunc::GetFormatString (expressiont);
            fstringt = FormatStringFunc::ParseFormatString (stringformat_raw);
        }
        FormatString fstringr = rezult_format.Get (j);
        DBtest (expressiont, expressionr, "expression", true);
        DBtest (fstringt.isEmpty == fstringr.isEmpty, "isEmpty", true);
        DBtest (fstringt.isRead == fstringr.isRead, "isRead", true);
        DBtest (fstringt.forceRaw == fstringr.forceRaw, "forceRaw", true);
        DBtest (fstringt.trim_zero == fstringr.trim_zero, "trim_zero", true);
        DBtest (fstringt.koeff, fstringr.koeff, "koeff", true);
        DBtest (fstringt.delimetr, fstringr.delimetr, "delimetr", true);
        DBtest (fstringt.n_zero, fstringr.n_zero, "n_zero", true);
        DBtest (fstringt.stringformat, fstringr.stringformat, "stringformat", true);
    }
    return;
}

void ResetSyncPropertyArray (GS::Array<API_Guid> guidArray)
{
    if (guidArray.IsEmpty ()) return;
    for (UInt32 j = 0; j < guidArray.GetSize (); j++) {
        ResetSyncPropertyOne (guidArray[j]);
    }
    DBprnt ("TEST", "ResetSyncPropertyArray");
}

void ResetSyncPropertyOne (const API_Guid& elemGuid)
{
    GSErrCode	err = NoError;
    GS::Array<API_Property> propertywrite;
    ResetSyncPropertyOne (elemGuid, propertywrite);
    if (propertywrite.IsEmpty ()) return;
    GS::UniString undoString = RSGetIndString (ID_ADDON_STRINGS + isEng (), UndoSyncId, ACAPI_GetOwnResModule ());
    ACAPI_CallUndoableCommand (undoString, [&]() -> GSErrCode {
        err = ACAPI_Element_SetProperties (elemGuid, propertywrite);
        return NoError;
    });
}


void ResetSyncPropertyOne (const API_Guid& elemGuid, GS::Array<API_Property>& propertywrite)
{
    GSErrCode	err = NoError;
    GS::Array<API_PropertyDefinition> definitions;
    err = ACAPI_Element_GetPropertyDefinitions (elemGuid, API_PropertyDefinitionFilter_UserDefined, definitions);
    if (err != NoError) {
        msg_rep ("ResetSyncProperty", "ACAPI_Element_GetPropertyDefinitions", err, elemGuid);
        return;
    }
    for (UInt32 i = 0; i < definitions.GetSize (); i++) {
        if (!definitions[i].description.IsEmpty ()) {
            if (definitions[i].description.Contains ("Sync_from")) {
                API_Property property = {};
                if (ACAPI_Element_GetPropertyValue (elemGuid, definitions[i].guid, property) == NoError) {
                    if (!property.isDefault) {
                        property.isDefault = true;
                        propertywrite.Push (property);
                    }
                } else {
                    msg_rep ("ResetSyncProperty", "ACAPI_Element_GetPropertyValue", err, elemGuid);
                }
            }
        }
    }

}

}
#endif
