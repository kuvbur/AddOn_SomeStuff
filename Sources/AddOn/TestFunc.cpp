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

    //GS::UniString var = " ";
    //TestGetTextLineLength (var);
    //var = u8"\u2007"; TestGetTextLineLength (var);
}

void TestGetTextLineLength (GS::UniString& var)
{
    GSErrCode err = NoError;
    GS::UniString fontname = "Arial";
    double fontsize = 2.5;
    short font_inx = 0; double width = 0.0;
    API_TextLinePars tlp; BNZeroMemory (&tlp, sizeof (API_TextLinePars));
    #if defined(AC_27) || defined(AC_28) || defined(AC_29)
    API_FontType font; BNZeroMemory (&font, sizeof (API_FontType));
    font.head.index = 0;
    font.head.uniStringNamePtr = &fontname;
    err = ACAPI_Font_SearchFont (font);
    font_inx = font.head.index;
    #else
    API_Attribute attrib; BNZeroMemory (&attrib, sizeof (API_Attribute));
    attrib.header.typeID = API_FontID;
    attrib.header.index = 0;
    attrib.header.uniStringNamePtr = &fontname;
    err = ACAPI_Attribute_Search (&attrib.header);
    font_inx = attrib.header.index;
    #endif
    font_inx = 135;
    tlp.drvScaleCorr = false;
    tlp.index = 0;
    tlp.wantsLongestIndex = false;
    tlp.lineUniStr = &var;
    tlp.wFace = APIFace_Plain;
    tlp.wFont = font_inx;
    tlp.wSize = fontsize;
    tlp.wSlant = PI / 2.0;
    #if defined(AC_27) || defined(AC_28) || defined(AC_29)
    err = ACAPI_Element_GetTextLineLength (&tlp, &width);
    #else
    err = ACAPI_Goodies (APIAny_GetTextLineLengthID, &tlp, &width);
    #endif
    #ifdef TESTING
    DBtest (width > 0.00001, "TestGetTextLineLength", false);
    #endif
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
        pvalue.rawName = FORMULANAMEPREFIX + f.ToLowerCase () + ";" + pvalue.val.uniStringValue + "}";
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

    for (ParamDictValue::PairIterator cIt = paramByType.EnumeratePairs (); cIt != NULL; ++cIt) {
        #if defined(AC_28) || defined(AC_29)
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

    test_expression = "\"<0.001+0.001>.3mp+<0.001+0.001>.03mm\""; test_name = "<0.001+0.001>.3mp+<0.001+0.001>.03mm";
    tests.Push (test_expression); rezult_format.Push (fstring); rezult_name.Push (test_name);

    test_expression = "from{Layers, 6; \"1/{Property:Теплотехнический расчёт/αint, Вт/(м2°С)} + 1/{Property:Теплотехнический расчёт/αext, Вт/(м2°С)} <+%layer_thickness.3m%/%BuildingMaterialProperties/Building Material Thermal Conductivity.3m%>\"}";

    test_name = "1/{Property:Теплотехнический расчёт/αint, Вт/(м2°С)} + 1/{Property:Теплотехнический расчёт/αext, Вт/(м2°С)} <+%layer_thickness.3m%/%BuildingMaterialProperties/Building Material Thermal Conductivity.3m%>";
    tests.Push (test_expression); rezult_format.Push (fstring); rezult_name.Push (test_name);

    test_expression = "Спецификация сэндвич/Площадь, кв.м. (без подрезок)"; test_name = test_expression;
    tests.Push (test_expression); rezult_format.Push (fstring); rezult_name.Push (test_name);

    test_expression = "Спецификация сэндвич/Площадь, кв.м. (без малых подрезок)"; test_name = test_expression;
    tests.Push (test_expression); rezult_format.Push (fstring); rezult_name.Push (test_name);

    test_expression = "Спецификация сэндвич/Площадь, кв.м. (или в п.м.)"; test_name = test_expression;
    tests.Push (test_expression); rezult_format.Push (fstring); rezult_name.Push (test_name);

    test_expression = "SomeStuff Материалы/Конструкции.Имя в спецификации"; test_name = test_expression;
    tests.Push (test_expression); rezult_format.Push (fstring); rezult_name.Push (test_name);
    // =============================================================================
    fstring.isEmpty = false; fstring.isRead = true; fstring.koeff = 1000;
    fstring.stringformat = "0mm"; fstring.n_zero = 0;
    // =============================================================================
    test_expression = "ConWidth_1.0mm"; test_name = "ConWidth_1";
    tests.Push (test_expression); rezult_format.Push (fstring); rezult_name.Push (test_name);

    test_expression = "ConWidth_1.0mm"; test_name = "ConWidth_1";
    tests.Push (test_expression); rezult_format.Push (fstring); rezult_name.Push (test_name);

    // =============================================================================
    fstring.isEmpty = false; fstring.isRead = true; fstring.koeff = 1;
    fstring.stringformat = "3m"; fstring.n_zero = 3;
    // =============================================================================
    test_expression = "from{Layers, 6; \"1/{Property:Теплотехнический расчёт/αint, Вт/(м2°С)} + 1/{Property:Теплотехнический расчёт/αext, Вт/(м2°С)} <+%layer_thickness.3m%/%BuildingMaterialProperties/Building Material Thermal Conductivity.3m%>\".3m}";
    test_name = "1/{Property:Теплотехнический расчёт/αint, Вт/(м2°С)} + 1/{Property:Теплотехнический расчёт/αext, Вт/(м2°С)} <+%layer_thickness.3m%/%BuildingMaterialProperties/Building Material Thermal Conductivity.3m%>";
    tests.Push (test_expression); rezult_format.Push (fstring); rezult_name.Push (test_name);

    test_expression = "<0.001+0.001>.3m"; test_name = "0.001+0.001";
    tests.Push (test_expression); rezult_format.Push (fstring); rezult_name.Push (test_name);

    // =============================================================================
    fstring.isEmpty = false; fstring.isRead = true; fstring.koeff = 1;
    fstring.stringformat = "3mp"; fstring.n_zero = 3; fstring.delimetr = ".";
    // =============================================================================
    test_expression = "from{Material:Layers, 6; \"1/{Property:Теплотехнический расчёт/αint, Вт/(м2°С)} + 1/{Property:Теплотехнический расчёт/αext, Вт/(м2°С)} <+%толщина.3m%/%BuildingMaterialProperties/Building Material Thermal Conductivity.3m%>\".3mp}";
    test_name = "1/{Property:Теплотехнический расчёт/αint, Вт/(м2°С)} + 1/{Property:Теплотехнический расчёт/αext, Вт/(м2°С)} <+%толщина.3m%/%BuildingMaterialProperties/Building Material Thermal Conductivity.3m%>";
    tests.Push (test_expression); rezult_format.Push (fstring); rezult_name.Push (test_name);
    test_expression = "<0.001+0.001>.3mp"; test_name = "0.001+0.001";
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
            expressiont = templatestring;
        } else {
            if (expressiont.Contains ('<') && expressiont.Contains ('>')) {
                GS::UniString templatestring = expressiont.GetSubstring ('<', '>', 0);
                FormatStringFunc::GetFormatStringFromFormula (expressiont, templatestring, stringformat_raw);
                fstringt = FormatStringFunc::ParseFormatString (stringformat_raw);
                expressiont = templatestring;
            } else {
                stringformat_raw = FormatStringFunc::GetFormatString (expressiont);
                fstringt = FormatStringFunc::ParseFormatString (stringformat_raw);
            }
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

void DumpAllBuiltInProperties ()
{
    //GS::Array<API_PropertyGroup> groups;
    //ACAPI_Property_GetPropertyGroups (groups);
    //for (const API_PropertyGroup& group : groups) {
    //    GS::Array<API_PropertyDefinition> definitions;
    //    ACAPI_Property_GetPropertyDefinitions (group.guid, definitions);
    //    GS::UniString report_ =
    //        "======" + group.name + "\t" +
    //        APIGuidToString (group.guid);
    //    ACAPI_WriteReport (report_, false);
    //    for (const API_PropertyDefinition& definition : definitions) {
    //        if (definition.definitionType != API_PropertyStaticBuiltInDefinitionType) {
    //            continue;
    //        }
    //        GS::UniString report =
    //            group.name + "\t" +
    //            definition.name + "\t" +
    //            APIGuidToString (definition.guid);
    //        ACAPI_WriteReport (report, false);
    //    }
    //}
}

void ResetSyncPropertyArray (GS::Array<API_Guid> guidArray)
{
    if (guidArray.IsEmpty ()) return;
    for (UInt32 j = 0; j < guidArray.GetSize (); j++) {
        ResetSyncPropertyOne (guidArray[j]);
    }
    #if defined(TESTING)
    DBprnt ("TEST", "ResetSyncPropertyArray");
    #endif
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
