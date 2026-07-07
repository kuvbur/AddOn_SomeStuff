//------------ kuvbur 2022 ------------
#ifdef TESTING
#include "ACAPinc.h"
#include "APIEnvir.h"
#include "Helpers.hpp"
#include "Propertycache.hpp"
#include "TestFunc.hpp"
namespace TestFunc
{

void Test ()
{
    DBprnt ("TEST", "start");
    TestFormatString ();
    TestCalc ();
    TestFormula ();
    TestConvertToParamValue ();
    TestConvertAttributeToParamValue ();
    TestConvertPropertyToParamValue ();
    TestConvertPropertyDefinitionToParamValue ();
    TestSetParamValueSourseByName ();
    TestSetrawNameFromProperty ();
    TestCheckIgnoreVal ();
    TestReadProperty ();
    TestAddProperty ();
    GS::UniString var = " ";
    TestGetTextLineLength (var);
    var = u8"\u2007"; TestGetTextLineLength (var);
    var = ""; TestGetTextLineLength (var);
    DBprnt ("TEST", "end");
}

void TestGetTextLineLength (GS::UniString& var)
{
    GSErrCode err = NoError;
    GS::UniString fontname = "Arial";
    double fontsize = 2.5;
    short font_inx = 0; double width = 0.0;
    API_TextLinePars tlp = {};
    #if defined(AC_27) || defined(AC_28) || defined(AC_29)
    API_FontType font; BNZeroMemory (&font, sizeof (API_FontType));
    font.head.index = 0;
    font.head.uniStringNamePtr = &fontname;
    err = ACAPI_Font_SearchFont (font);
    font_inx = font.head.index;
    #else
    API_Attribute attrib = {};
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
    ParamDictValue params;
    ParamValue pvalue;
    pvalue.val.hasFormula = true;
    pvalue.isValid = false;
    GS::Array<ParamValue> formula = {};

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
        params.Add (pvalue.rawName, pvalue);
    }
    pvalue.val.hasFormula = false;
    pvalue.isValid = true;
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

    DBtest (ParamHelpers::ReadFormula (params, false), "ReadFormula", true);

    for (ParamDictValue::PairIterator cIt = params.EnumeratePairs (); cIt != NULL; ++cIt) {
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
        ParamValue& rezult = formula.Get (j);
        DBtest (params.ContainsKey (rezult.rawName), rezult.rawName, true);
        ParamValue& test = params.Get (rezult.rawName);
        DBtest (test.val.formatstring.stringformat, rezult.val.formatstring.stringformat, "formatstring", true);
        DBtest (test.isValid == true, "isValid", true);
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

// -----------------------------------------------------------------------------
// Простые тесты функций конвертации базовых типов в ParamValue (Helpers.hpp/cpp)
// -----------------------------------------------------------------------------
void TestConvertToParamValue ()
{
    DBprnt ("TEST", "TestConvertToParamValue");
    ParamValue pvalue;

    // ---- ConvertIntToParamValue ----
    pvalue = ParamValue ();
    DBtest (ParamHelpers::ConvertIntToParamValue (pvalue, "TestInt", 42), "ConvertIntToParamValue : return", true);
    DBtest (pvalue.name, GS::UniString ("TestInt"), "ConvertIntToParamValue : name", true);
    DBtest (!pvalue.rawName.IsEmpty (), "ConvertIntToParamValue : rawName не пуст", true);
    DBtest (pvalue.val.type == API_PropertyIntegerValueType, "ConvertIntToParamValue : type", true);
    DBtest (pvalue.val.intValue, (Int32) 42, "ConvertIntToParamValue : intValue", true);
    DBtest (is_equal (pvalue.val.doubleValue, 42.0), "ConvertIntToParamValue : doubleValue", true);
    DBtest (pvalue.val.boolValue, "ConvertIntToParamValue : boolValue (42 > 0)", true);
    DBtest (pvalue.val.uniStringValue, GS::UniString ("42"), "ConvertIntToParamValue : uniStringValue", true);
    DBtest (pvalue.isValid, "ConvertIntToParamValue : isValid", true);

    pvalue = ParamValue ();
    ParamHelpers::ConvertIntToParamValue (pvalue, "TestIntNeg", -5);
    DBtest (!pvalue.val.boolValue, "ConvertIntToParamValue : boolValue (-5 не > 0)", true);
    DBtest (pvalue.val.intValue, (Int32) (-5), "ConvertIntToParamValue : intValue (-5)", true);

    pvalue = ParamValue ();
    ParamHelpers::ConvertIntToParamValue (pvalue, "TestIntZero", 0);
    DBtest (!pvalue.val.boolValue, "ConvertIntToParamValue : boolValue (0 не > 0)", true);

    // ---- ConvertDoubleToParamValue ----
    pvalue = ParamValue ();
    DBtest (ParamHelpers::ConvertDoubleToParamValue (pvalue, "TestDouble", 3.14), "ConvertDoubleToParamValue : return", true);
    DBtest (pvalue.val.type == API_PropertyRealValueType, "ConvertDoubleToParamValue : type", true);
    DBtest (pvalue.val.intValue, (Int32) 3, "ConvertDoubleToParamValue : intValue (усечение 3.14 -> 3)", true);
    DBtest (is_equal (pvalue.val.doubleValue, 3.14), "ConvertDoubleToParamValue : doubleValue", true);
    DBtest (pvalue.val.boolValue, "ConvertDoubleToParamValue : boolValue (!= 0)", true);
    DBtest (pvalue.val.uniStringValue, GS::UniString ("3.140"), "ConvertDoubleToParamValue : uniStringValue (%.3f)", true);
    DBtest (pvalue.isValid, "ConvertDoubleToParamValue : isValid", true);

    pvalue = ParamValue ();
    ParamHelpers::ConvertDoubleToParamValue (pvalue, "TestDoubleZero", 0.0);
    DBtest (!pvalue.val.boolValue, "ConvertDoubleToParamValue : boolValue (== 0)", true);

    pvalue = ParamValue ();
    ParamHelpers::ConvertDoubleToParamValue (pvalue, "TestDoubleNeg", -2.5);
    DBtest (pvalue.val.intValue, (Int32) (-2), "ConvertDoubleToParamValue : intValue (усечение -2.5 -> -2)", true);
    DBtest (pvalue.val.boolValue, "ConvertDoubleToParamValue : boolValue (-2.5 != 0)", true);

    // ---- ConvertBoolToParamValue ----
    pvalue = ParamValue ();
    DBtest (ParamHelpers::ConvertBoolToParamValue (pvalue, "TestBoolTrue", true), "ConvertBoolToParamValue : return", true);
    DBtest (pvalue.val.type == API_PropertyBooleanValueType, "ConvertBoolToParamValue : type", true);
    DBtest (pvalue.val.boolValue, "ConvertBoolToParamValue : boolValue (true)", true);
    DBtest (pvalue.val.intValue, (Int32) 1, "ConvertBoolToParamValue : intValue (true -> 1)", true);
    DBtest (is_equal (pvalue.val.doubleValue, 1.0), "ConvertBoolToParamValue : doubleValue (true -> 1.0)", true);
    DBtest (!pvalue.val.uniStringValue.IsEmpty (), "ConvertBoolToParamValue : uniStringValue не пуст (true)", true);

    pvalue = ParamValue ();
    ParamHelpers::ConvertBoolToParamValue (pvalue, "TestBoolFalse", false);
    DBtest (!pvalue.val.boolValue, "ConvertBoolToParamValue : boolValue (false)", true);
    DBtest (pvalue.val.intValue, (Int32) 0, "ConvertBoolToParamValue : intValue (false -> 0)", true);
    DBtest (is_equal (pvalue.val.doubleValue, 0.0), "ConvertBoolToParamValue : doubleValue (false -> 0.0)", true);

    // ---- ConvertStringToParamValue ----
    pvalue = ParamValue ();
    DBtest (ParamHelpers::ConvertStringToParamValue (pvalue, "TestStrNum", "123.5"), "ConvertStringToParamValue : return", true);
    DBtest (pvalue.val.type == API_PropertyStringValueType, "ConvertStringToParamValue : type", true);
    DBtest (pvalue.val.canCalculate, "ConvertStringToParamValue : canCalculate (\"123.5\" - число)", true);
    DBtest (is_equal (pvalue.val.doubleValue, 123.5), "ConvertStringToParamValue : doubleValue (123.5)", true);
    DBtest (pvalue.val.intValue, (Int32) 124, "ConvertStringToParamValue : intValue (округление вверх 123.5 -> 124)", true);
    DBtest (pvalue.val.boolValue, "ConvertStringToParamValue : boolValue (непустая строка)", true);

    pvalue = ParamValue ();
    ParamHelpers::ConvertStringToParamValue (pvalue, "TestStrInt", "10");
    DBtest (pvalue.val.canCalculate, "ConvertStringToParamValue : canCalculate (\"10\" - число)", true);
    DBtest (pvalue.val.intValue, (Int32) 10, "ConvertStringToParamValue : intValue (\"10\" -> 10, без округления)", true);

    pvalue = ParamValue ();
    ParamHelpers::ConvertStringToParamValue (pvalue, "TestStrText", "abc");
    DBtest (!pvalue.val.canCalculate, "ConvertStringToParamValue : canCalculate (\"abc\" - не число)", true);
    DBtest (pvalue.val.boolValue, "ConvertStringToParamValue : boolValue (\"abc\" непустая)", true);
    DBtest (pvalue.val.intValue, (Int32) 1, "ConvertStringToParamValue : intValue (\"abc\" -> 1)", true);
    DBtest (is_equal (pvalue.val.doubleValue, 1.0), "ConvertStringToParamValue : doubleValue (\"abc\" -> 1.0)", true);

    pvalue = ParamValue ();
    ParamHelpers::ConvertStringToParamValue (pvalue, "TestStrEmpty", "");
    DBtest (!pvalue.val.boolValue, "ConvertStringToParamValue : boolValue (пустая строка)", true);
    DBtest (!pvalue.val.canCalculate, "ConvertStringToParamValue : canCalculate (пустая строка)", true);

    DBprnt ("TEST", "TestConvertToParamValue : done");
    return;
}

// -----------------------------------------------------------------------------
// Тест ConvertAttributeToParamValue на граничных значениях API_Attribute
// -----------------------------------------------------------------------------
void TestConvertAttributeToParamValue ()
{
    DBprnt ("TEST", "TestConvertAttributeToParamValue");
    ParamValue pvalue;
    API_Attribute attrib;

    // ---- Граница: индекс атрибута = 0 (несуществующий/неинициализированный индекс) ----
    pvalue = ParamValue ();
    attrib = {};
    attrib.header.typeID = API_LayerID;
    attrib.header.index = 0;
    ParamHelpers::ConvertAttributeToParamValue (pvalue, "TestAttrZero", attrib);
    DBtest (pvalue.val.intValue, (Int32) 0, "ConvertAttributeToParamValue : intValue (index == 0)", true);
    DBtest (!pvalue.val.boolValue, "ConvertAttributeToParamValue : boolValue (index == 0, не > 0)", true);
    DBtest (is_equal (pvalue.val.doubleValue, 0.0), "ConvertAttributeToParamValue : doubleValue (index == 0)", true);
    DBtest (pvalue.val.type == API_PropertyStringValueType, "ConvertAttributeToParamValue : type", true);
    DBtest (pvalue.isValid, "ConvertAttributeToParamValue : isValid", true);
    DBtest (pvalue.fromAttribElement, "ConvertAttributeToParamValue : fromAttribElement", true);
    DBtest (pvalue.val.uniStringValue.IsEmpty (), "ConvertAttributeToParamValue : uniStringValue пуст (имя не задано)", true);

    // ---- Граница: минимально возможный положительный индекс (1) ----
    pvalue = ParamValue ();
    attrib = {};
    attrib.header.typeID = API_LayerID;
    attrib.header.index = 1;
    ParamHelpers::ConvertAttributeToParamValue (pvalue, "TestAttrOne", attrib);
    DBtest (pvalue.val.intValue, (Int32) 1, "ConvertAttributeToParamValue : intValue (index == 1)", true);
    DBtest (pvalue.val.boolValue, "ConvertAttributeToParamValue : boolValue (index == 1 > 0)", true);
    DBtest (is_equal (pvalue.val.doubleValue, 1.0), "ConvertAttributeToParamValue : doubleValue (index == 1)", true);

    // ---- Граница: максимальный short-индекс (граница типа для старых версий ACAPI) ----
    pvalue = ParamValue ();
    attrib = {};
    attrib.header.typeID = API_LayerID;
    attrib.header.index = std::numeric_limits<short>::max ();
    ParamHelpers::ConvertAttributeToParamValue (pvalue, "TestAttrMax", attrib);
    DBtest (pvalue.val.intValue, (Int32) std::numeric_limits<short>::max (), "ConvertAttributeToParamValue : intValue (index == SHRT_MAX)", true);
    DBtest (pvalue.val.boolValue, "ConvertAttributeToParamValue : boolValue (SHRT_MAX > 0)", true);
    DBtest (is_equal (pvalue.val.doubleValue, (double) std::numeric_limits<short>::max ()), "ConvertAttributeToParamValue : doubleValue (index == SHRT_MAX)", true);

    // ---- Граница: rawName уже задан вызывающей стороной -> не должен перезаписываться ----
    pvalue = ParamValue ();
    pvalue.rawName = "{@attrib:custom_rawname}";
    attrib = {};
    attrib.header.typeID = API_LayerID;
    attrib.header.index = 5;
    ParamHelpers::ConvertAttributeToParamValue (pvalue, "TestAttrCustomRaw", attrib);
    DBtest (pvalue.rawName, GS::UniString ("{@attrib:custom_rawname}"), "ConvertAttributeToParamValue : rawName не перезаписывается, если не пуст", true);

    DBprnt ("TEST", "TestConvertAttributeToParamValue : done");
    // Примечание: конкретное имя атрибута (attr.header.name) в данном тесте не заполняется -
    // это отдельное низкоуровневое поле фиксированного размера в API_AttributeHeader,
    // корректно заполняемое реальным ACAPI_Attribute_Get. Пустое имя - тоже граничный случай,
    // корректно обрабатываемый функцией (см. проверку uniStringValue.IsEmpty () выше).
    return;
}

// -----------------------------------------------------------------------------
// Тест ConvertToParamValue (API_Property) на граничных значениях
// -----------------------------------------------------------------------------
void TestConvertPropertyToParamValue ()
{
    DBprnt ("TEST", "TestConvertPropertyToParamValue");
    ParamValue pvalue;
    API_Property property;

    // ---- Integer: граничные значения INT32_MIN / 0 / INT32_MAX ----
    pvalue = ParamValue ();
    pvalue.rawName = "{@property:testint_min}";
    pvalue.name = "TestIntMin";
    property = {};
    property.isDefault = false;
    #if defined(AC_22) || defined(AC_23)
    property.isEvaluated = true;
    #else
    property.status = API_Property_HasValue;
    #endif
    property.definition.collectionType = API_PropertySingleCollectionType;
    property.definition.valueType = API_PropertyIntegerValueType;
    property.value.singleVariant.variant.type = API_PropertyIntegerValueType;
    property.value.singleVariant.variant.intValue = std::numeric_limits<Int32>::min ();
    DBtest (ParamHelpers::ConvertToParamValue (pvalue, property), "ConvertToParamValue(Property) : return (Integer INT32_MIN)", true);
    DBtest (pvalue.val.intValue, std::numeric_limits<Int32>::min (), "ConvertToParamValue(Property) : intValue (INT32_MIN)", true);
    DBtest (!pvalue.val.boolValue, "ConvertToParamValue(Property) : boolValue (INT32_MIN не > 0)", true);
    DBtest (pvalue.val.type == API_PropertyIntegerValueType, "ConvertToParamValue(Property) : type (Integer)", true);
    DBtest (pvalue.isValid, "ConvertToParamValue(Property) : isValid (HasValue)", true);

    pvalue = ParamValue ();
    pvalue.rawName = "{@property:testint_zero}";
    pvalue.name = "TestIntZero";
    property.value.singleVariant.variant.intValue = 0;
    ParamHelpers::ConvertToParamValue (pvalue, property);
    DBtest (!pvalue.val.boolValue, "ConvertToParamValue(Property) : boolValue (0 не > 0)", true);

    pvalue = ParamValue ();
    pvalue.rawName = "{@property:testint_max}";
    pvalue.name = "TestIntMax";
    property.value.singleVariant.variant.intValue = std::numeric_limits<Int32>::max ();
    ParamHelpers::ConvertToParamValue (pvalue, property);
    DBtest (pvalue.val.intValue, std::numeric_limits<Int32>::max (), "ConvertToParamValue(Property) : intValue (INT32_MAX)", true);
    DBtest (pvalue.val.boolValue, "ConvertToParamValue(Property) : boolValue (INT32_MAX > 0)", true);

    // ---- Real: 0.0 (граница bool == false), максимум double, отрицательное значение ----
    pvalue = ParamValue ();
    pvalue.rawName = "{@property:testreal_zero}";
    pvalue.name = "TestRealZero";
    property = {};
    #if defined(AC_22) || defined(AC_23)
    property.isEvaluated = true;
    #else
    property.status = API_Property_HasValue;
    #endif
    property.definition.collectionType = API_PropertySingleCollectionType;
    property.definition.valueType = API_PropertyRealValueType;
    property.definition.measureType = API_PropertyUndefinedMeasureType;
    property.value.singleVariant.variant.type = API_PropertyRealValueType;
    property.value.singleVariant.variant.doubleValue = 0.0;
    ParamHelpers::ConvertToParamValue (pvalue, property);
    DBtest (!pvalue.val.boolValue, "ConvertToParamValue(Property) : boolValue (0.0 == 0)", true);
    DBtest (pvalue.val.type == API_PropertyRealValueType, "ConvertToParamValue(Property) : type (Real)", true);

    pvalue = ParamValue ();
    pvalue.rawName = "{@property:testreal_neg}";
    pvalue.name = "TestRealNeg";
    property.value.singleVariant.variant.doubleValue = -123456.789;
    ParamHelpers::ConvertToParamValue (pvalue, property);
    DBtest (pvalue.val.boolValue, "ConvertToParamValue(Property) : boolValue (отрицательное != 0)", true);
    DBtest (is_equal (pvalue.val.doubleValue, -123456.789), "ConvertToParamValue(Property) : doubleValue (отрицательное)", true);

    pvalue = ParamValue ();
    pvalue.rawName = "{@property:testreal_max}";
    pvalue.name = "TestRealMax";
    property.value.singleVariant.variant.doubleValue = std::numeric_limits<double>::max ();
    ParamHelpers::ConvertToParamValue (pvalue, property);
    DBtest (pvalue.val.boolValue, "ConvertToParamValue(Property) : boolValue (DBL_MAX != 0)", true);

    // ---- Boolean: true / false ----
    pvalue = ParamValue ();
    pvalue.rawName = "{@property:testbool_true}";
    pvalue.name = "TestBoolTrue";
    property = {};
    #if defined(AC_22) || defined(AC_23)
    property.isEvaluated = true;
    #else
    property.status = API_Property_HasValue;
    #endif
    property.definition.collectionType = API_PropertySingleCollectionType;
    property.definition.valueType = API_PropertyBooleanValueType;
    property.value.singleVariant.variant.type = API_PropertyBooleanValueType;
    property.value.singleVariant.variant.boolValue = true;
    ParamHelpers::ConvertToParamValue (pvalue, property);
    DBtest (pvalue.val.boolValue, "ConvertToParamValue(Property) : boolValue (true)", true);
    DBtest (pvalue.val.intValue, (Int32) 1, "ConvertToParamValue(Property) : intValue (true -> 1)", true);
    DBtest (pvalue.val.type == API_PropertyBooleanValueType, "ConvertToParamValue(Property) : type (Boolean)", true);

    pvalue = ParamValue ();
    pvalue.rawName = "{@property:testbool_false}";
    pvalue.name = "TestBoolFalse";
    property.value.singleVariant.variant.boolValue = false;
    ParamHelpers::ConvertToParamValue (pvalue, property);
    DBtest (!pvalue.val.boolValue, "ConvertToParamValue(Property) : boolValue (false)", true);

    // ---- String: пустая строка / длинная юникод-строка ----
    pvalue = ParamValue ();
    pvalue.rawName = "{@property:teststr_empty}";
    pvalue.name = "TestStrEmpty";
    property = {};
    #if defined(AC_22) || defined(AC_23)
    property.isEvaluated = true;
    #else
    property.status = API_Property_HasValue;
    #endif
    property.definition.collectionType = API_PropertySingleCollectionType;
    property.definition.valueType = API_PropertyStringValueType;
    property.value.singleVariant.variant.type = API_PropertyStringValueType;
    property.value.singleVariant.variant.uniStringValue = "";
    ParamHelpers::ConvertToParamValue (pvalue, property);
    DBtest (!pvalue.val.boolValue, "ConvertToParamValue(Property) : boolValue (пустая строка)", true);
    DBtest (pvalue.val.type == API_PropertyStringValueType, "ConvertToParamValue(Property) : type (String)", true);

    pvalue = ParamValue ();
    pvalue.rawName = "{@property:teststr_long}";
    pvalue.name = "TestStrLong";
    GS::UniString longString = "";
    for (UInt32 i = 0; i < 200; i++) longString.Append ("Ё");
    property.value.singleVariant.variant.uniStringValue = longString;
    ParamHelpers::ConvertToParamValue (pvalue, property);
    DBtest (pvalue.val.boolValue, "ConvertToParamValue(Property) : boolValue (длинная строка непустая)", true);
    DBtest (pvalue.val.uniStringValue.GetLength () == 200, "ConvertToParamValue(Property) : uniStringValue длина сохранена", true);

    // ---- Undefined: функция должна вернуть false и isValid == false ----
    pvalue = ParamValue ();
    pvalue.rawName = "{@property:testundef}";
    pvalue.name = "TestUndefined";
    property = {};
    #if defined(AC_22) || defined(AC_23)
    property.isEvaluated = true;
    #else
    property.status = API_Property_HasValue;
    #endif
    property.definition.collectionType = API_PropertySingleCollectionType;
    property.definition.valueType = API_PropertyUndefinedValueType;
    DBtest (!ParamHelpers::ConvertToParamValue (pvalue, property), "ConvertToParamValue(Property) : return (Undefined -> false)", true);
    DBtest (!pvalue.isValid, "ConvertToParamValue(Property) : isValid (Undefined -> false)", true);

    DBprnt ("TEST", "TestConvertPropertyToParamValue : done");
    return;
}

// -----------------------------------------------------------------------------
// Тест ConvertToParamValue (API_PropertyDefinition) на граничных значениях
// -----------------------------------------------------------------------------
void TestConvertPropertyDefinitionToParamValue ()
{
    DBprnt ("TEST", "TestConvertPropertyDefinitionToParamValue");
    ParamValue pvalue;
    API_PropertyDefinition definition;

    // ---- Обычное определение без специальных маркеров в description ----
    pvalue = ParamValue ();
    pvalue.rawName = "{@property:testdef_plain}";
    pvalue.name = "TestDefPlain";
    definition = {};
    definition.guid = APINULLGuid;
    definition.description = "";
    definition.valueType = API_PropertyRealValueType;
    DBtest (ParamHelpers::ConvertToParamValue (pvalue, definition), "ConvertToParamValue(Definition) : return", true);
    DBtest (pvalue.val.type == API_PropertyRealValueType, "ConvertToParamValue(Definition) : val.type", true);
    DBtest (pvalue.type == API_PropertyRealValueType, "ConvertToParamValue(Definition) : type", true);
    DBtest (pvalue.fromProperty, "ConvertToParamValue(Definition) : fromProperty", true);
    DBtest (pvalue.fromPropertyDefinition, "ConvertToParamValue(Definition) : fromPropertyDefinition (нет спецмаркеров)", true);

    // ---- Граница: description содержит "sync_name" -> особый rawName ----
    pvalue = ParamValue ();
    pvalue.rawName = "{@property:anything}";
    pvalue.name = "TestDefSyncName";
    definition = {};
    definition.guid = APINULLGuid;
    definition.description = "sync_name";
    definition.valueType = API_PropertyStringValueType;
    ParamHelpers::ConvertToParamValue (pvalue, definition);
    DBtest (pvalue.rawName, GS::UniString ("{@property:sync_name0}"), "ConvertToParamValue(Definition) : rawName (sync_name)", true);
    DBtest (pvalue.fromAttribDefinition, "ConvertToParamValue(Definition) : fromAttribDefinition (sync_name)", true);

    // ---- Граница: rawName содержит "buildingmaterial" -> fromAttribDefinition, без изменения rawName ----
    pvalue = ParamValue ();
    pvalue.rawName = "{@property:buildingmaterialproperties/density}";
    pvalue.name = "TestDefBuildingMaterial";
    definition = {};
    definition.guid = APINULLGuid;
    definition.description = "";
    definition.valueType = API_PropertyRealValueType;
    ParamHelpers::ConvertToParamValue (pvalue, definition);
    DBtest (pvalue.fromAttribDefinition, "ConvertToParamValue(Definition) : fromAttribDefinition (buildingmaterial in rawName)", true);
    DBtest (!pvalue.fromPropertyDefinition, "ConvertToParamValue(Definition) : fromPropertyDefinition == false (attrib имеет приоритет)", true);

    // ---- Граница: пустой guid и пустое valueType (Undefined) - функция всё равно возвращает true ----
    pvalue = ParamValue ();
    pvalue.rawName = "{@property:testdef_undef}";
    pvalue.name = "TestDefUndefined";
    definition = {};
    definition.guid = APINULLGuid;
    definition.description = "";
    definition.valueType = API_PropertyUndefinedValueType;
    DBtest (ParamHelpers::ConvertToParamValue (pvalue, definition), "ConvertToParamValue(Definition) : return (Undefined valueType всё равно true)", true);
    DBtest (pvalue.val.type == API_PropertyUndefinedValueType, "ConvertToParamValue(Definition) : val.type (Undefined)", true);

    DBprnt ("TEST", "TestConvertPropertyDefinitionToParamValue : done");
    return;
}

// -----------------------------------------------------------------------------
// Тест SetParamValueSourseByName - назначение флагов источника по префиксу rawName
// -----------------------------------------------------------------------------
void TestSetParamValueSourseByName ()
{
    DBprnt ("TEST", "TestSetParamValueSourseByName");
    ParamValue pvalue;

    // ---- PROPERTYNAMEPREFIX ----
    pvalue = ParamValue ();
    pvalue.rawName = PROPERTYNAMEPREFIX + GS::UniString ("testprop") + BRACEEND;
    ParamHelpers::SetParamValueSourseByName (pvalue);
    DBtest (pvalue.fromPropertyDefinition, "SetParamValueSourseByName : fromPropertyDefinition (PROPERTYNAMEPREFIX)", true);
    DBtest (pvalue.typeinx, (short) PROPERTYTYPEINX, "SetParamValueSourseByName : typeinx (PROPERTYNAMEPREFIX)", true);
    DBtest (!pvalue.fromAttribDefinition, "SetParamValueSourseByName : fromAttribDefinition == false (обычное свойство)", true);

    // ---- Граница: PROPERTYNAMEPREFIX + "buildingmaterialproperties/" -> дополнительно fromAttribDefinition ----
    pvalue = ParamValue ();
    pvalue.rawName = PROPERTYNAMEPREFIX + GS::UniString ("buildingmaterialproperties/density") + BRACEEND;
    ParamHelpers::SetParamValueSourseByName (pvalue);
    DBtest (pvalue.fromPropertyDefinition, "SetParamValueSourseByName : fromPropertyDefinition (buildingmaterial)", true);
    DBtest (pvalue.fromAttribDefinition, "SetParamValueSourseByName : fromAttribDefinition (buildingmaterialproperties/)", true);

    // ---- GDLNAMEPREFIX ----
    pvalue = ParamValue ();
    pvalue.rawName = GDLNAMEPREFIX + GS::UniString ("testgdl") + BRACEEND;
    ParamHelpers::SetParamValueSourseByName (pvalue);
    DBtest (pvalue.fromGDLparam, "SetParamValueSourseByName : fromGDLparam (GDLNAMEPREFIX)", true);
    DBtest (!pvalue.fromGDLdescription, "SetParamValueSourseByName : fromGDLdescription == false (обычный GDL)", true);
    DBtest (pvalue.typeinx, (short) GDLTYPEINX, "SetParamValueSourseByName : typeinx (GDLNAMEPREFIX)", true);

    // ---- GDLDESCNAMEPREFIX ----
    pvalue = ParamValue ();
    pvalue.rawName = GDLDESCNAMEPREFIX + GS::UniString ("testdesc") + BRACEEND;
    ParamHelpers::SetParamValueSourseByName (pvalue);
    DBtest (pvalue.fromGDLparam, "SetParamValueSourseByName : fromGDLparam (GDLDESCNAMEPREFIX)", true);
    DBtest (pvalue.fromGDLdescription, "SetParamValueSourseByName : fromGDLdescription (GDLDESCNAMEPREFIX)", true);
    DBtest (pvalue.typeinx, (short) GDLDESCTYPEINX, "SetParamValueSourseByName : typeinx (GDLDESCNAMEPREFIX)", true);

    // ---- COORDNAMEPREFIX ----
    pvalue = ParamValue ();
    pvalue.rawName = COORDNAMEPREFIX + GS::UniString ("testcoord") + BRACEEND;
    ParamHelpers::SetParamValueSourseByName (pvalue);
    DBtest (pvalue.fromCoord, "SetParamValueSourseByName : fromCoord (COORDNAMEPREFIX)", true);

    // ---- GLOBNAMEPREFIX ----
    pvalue = ParamValue ();
    pvalue.rawName = GLOBNAMEPREFIX + GS::UniString ("testglob") + BRACEEND;
    ParamHelpers::SetParamValueSourseByName (pvalue);
    DBtest (pvalue.fromGlob, "SetParamValueSourseByName : fromGlob (GLOBNAMEPREFIX)", true);

    // ---- INFONAMEPREFIX ----
    pvalue = ParamValue ();
    pvalue.rawName = INFONAMEPREFIX + GS::UniString ("testinfo") + BRACEEND;
    ParamHelpers::SetParamValueSourseByName (pvalue);
    DBtest (pvalue.fromInfo, "SetParamValueSourseByName : fromInfo (INFONAMEPREFIX)", true);

    // ---- MEPNAMEPREFIX ----
    pvalue = ParamValue ();
    pvalue.rawName = MEPNAMEPREFIX + GS::UniString ("testmep") + BRACEEND;
    ParamHelpers::SetParamValueSourseByName (pvalue);
    DBtest (pvalue.fromMEP, "SetParamValueSourseByName : fromMEP (MEPNAMEPREFIX)", true);

    // ---- FORMULANAMEPREFIX (устанавливает val.hasFormula, а не from*-флаг) ----
    pvalue = ParamValue ();
    pvalue.rawName = FORMULANAMEPREFIX + GS::UniString ("testformula") + BRACEEND;
    ParamHelpers::SetParamValueSourseByName (pvalue);
    DBtest (pvalue.val.hasFormula, "SetParamValueSourseByName : val.hasFormula (FORMULANAMEPREFIX)", true);

    // ---- IDNAMEPREFIX ----
    pvalue = ParamValue ();
    pvalue.rawName = IDNAMEPREFIX + GS::UniString ("testid") + BRACEEND;
    ParamHelpers::SetParamValueSourseByName (pvalue);
    DBtest (pvalue.fromID, "SetParamValueSourseByName : fromID (IDNAMEPREFIX)", true);

    // ---- IFCNAMEPREFIX ----
    pvalue = ParamValue ();
    pvalue.rawName = IFCNAMEPREFIX + GS::UniString ("testifc") + BRACEEND;
    ParamHelpers::SetParamValueSourseByName (pvalue);
    DBtest (pvalue.fromIFCProperty, "SetParamValueSourseByName : fromIFCProperty (IFCNAMEPREFIX)", true);

    // ---- MORPHNAMEPREFIX ----
    pvalue = ParamValue ();
    pvalue.rawName = MORPHNAMEPREFIX + GS::UniString ("testmorph") + BRACEEND;
    ParamHelpers::SetParamValueSourseByName (pvalue);
    DBtest (pvalue.fromMorph, "SetParamValueSourseByName : fromMorph (MORPHNAMEPREFIX)", true);

    // ---- ATTRIBNAMEPREFIX ----
    pvalue = ParamValue ();
    pvalue.rawName = ATTRIBNAMEPREFIX + GS::UniString ("testattrib") + BRACEEND;
    ParamHelpers::SetParamValueSourseByName (pvalue);
    DBtest (pvalue.fromAttribElement, "SetParamValueSourseByName : fromAttribElement (ATTRIBNAMEPREFIX)", true);

    // ---- LISTDATANAMEPREFIX ----
    pvalue = ParamValue ();
    pvalue.rawName = LISTDATANAMEPREFIX + GS::UniString ("testlistdata") + BRACEEND;
    ParamHelpers::SetParamValueSourseByName (pvalue);
    DBtest (pvalue.fromListData, "SetParamValueSourseByName : fromListData (LISTDATANAMEPREFIX)", true);

    // ---- MATERIALNAMEPREFIX ----
    pvalue = ParamValue ();
    pvalue.rawName = MATERIALNAMEPREFIX + GS::UniString ("testmaterial") + BRACEEND;
    ParamHelpers::SetParamValueSourseByName (pvalue);
    DBtest (pvalue.fromMaterial, "SetParamValueSourseByName : fromMaterial (MATERIALNAMEPREFIX)", true);

    // ---- CLASSNAMEPREFIX ----
    pvalue = ParamValue ();
    pvalue.rawName = CLASSNAMEPREFIX + GS::UniString ("testclass") + BRACEEND;
    ParamHelpers::SetParamValueSourseByName (pvalue);
    DBtest (pvalue.fromClassification, "SetParamValueSourseByName : fromClassification (CLASSNAMEPREFIX)", true);

    // ---- ELEMENTNAMEPREFIX ----
    pvalue = ParamValue ();
    pvalue.rawName = ELEMENTNAMEPREFIX + GS::UniString ("testelement") + BRACEEND;
    ParamHelpers::SetParamValueSourseByName (pvalue);
    DBtest (pvalue.fromElement, "SetParamValueSourseByName : fromElement (ELEMENTNAMEPREFIX)", true);

    // ---- FILENAMEPREFIX ----
    pvalue = ParamValue ();
    pvalue.rawName = FILENAMEPREFIX + GS::UniString ("testfile") + BRACEEND;
    ParamHelpers::SetParamValueSourseByName (pvalue);
    DBtest (pvalue.fromFile, "SetParamValueSourseByName : fromFile (FILENAMEPREFIX)", true);

    // ---- Граница: неизвестный префикс -> typeinx == 0, ни один флаг не установлен ----
    pvalue = ParamValue ();
    pvalue.rawName = "{@unknownprefix:test}";
    ParamHelpers::SetParamValueSourseByName (pvalue);
    DBtest (pvalue.typeinx, (short) 0, "SetParamValueSourseByName : typeinx (неизвестный префикс)", true);
    DBtest (!pvalue.fromProperty && !pvalue.fromGDLparam && !pvalue.fromCoord && !pvalue.fromElement, "SetParamValueSourseByName : флаги не установлены (неизвестный префикс)", true);

    // ---- Граница: ранний выход, если уже установлен любой from*-флаг (например, fromProperty) ----
    pvalue = ParamValue ();
    pvalue.rawName = GDLNAMEPREFIX + GS::UniString ("shouldnotchange") + BRACEEND;
    pvalue.fromProperty = true; // Уже определён источник
    ParamHelpers::SetParamValueSourseByName (pvalue);
    DBtest (!pvalue.fromGDLparam, "SetParamValueSourseByName : ранний выход - fromGDLparam не выставляется, если fromProperty уже true", true);
    DBtest (pvalue.typeinx, (short) 0, "SetParamValueSourseByName : ранний выход - typeinx не пересчитывается", true);

    // ---- Граница: GDL rawName с индексами массива (@arr_row_start_row_end_col_start_col_end) ----
    pvalue = ParamValue ();
    pvalue.rawName = GDLNAMEPREFIX + GS::UniString ("myarrparam@arr_3_5_7_9") + BRACEEND;
    ParamHelpers::SetParamValueSourseByName (pvalue);
    DBtest (pvalue.fromGDLparam, "SetParamValueSourseByName : fromGDLparam (GDL с @arr_)", true);
    DBtest (pvalue.val.array_row_start, 3, "SetParamValueSourseByName : val.array_row_start (@arr_3_5_7_9)", true);
    DBtest (pvalue.val.array_row_end, 5, "SetParamValueSourseByName : val.array_row_end (@arr_3_5_7_9)", true);
    DBtest (pvalue.val.array_column_start, 7, "SetParamValueSourseByName : val.array_column_start (@arr_3_5_7_9)", true);
    DBtest (pvalue.val.array_column_end, 9, "SetParamValueSourseByName : val.array_column_end (@arr_3_5_7_9)", true);

    DBprnt ("TEST", "TestSetParamValueSourseByName : done");
    return;
}

// -----------------------------------------------------------------------------
// Тест SetrawNameFromProperty - переопределение rawName/name по описанию свойства
// -----------------------------------------------------------------------------
void TestSetrawNameFromProperty ()
{
    DBprnt ("TEST", "TestSetrawNameFromProperty");
    ParamValue pvalue;
    API_Property property;

    // ---- Обычное свойство, rawName/name уже заданы вызывающей стороной, описание без спецмаркеров ----
    // (rawName/name предзаданы, чтобы не зависеть от внешней GetPropertyFullName)
    pvalue = ParamValue ();
    pvalue.rawName = "{@property:testplain}";
    pvalue.name = "TestPlain";
    property = {};
    property.definition.description = "";
    ParamHelpers::SetrawNameFromProperty (pvalue, property);
    DBtest (pvalue.rawName, GS::UniString ("{@property:testplain}"), "SetrawNameFromProperty : rawName не меняется (обычное свойство)", true);
    DBtest (pvalue.name, GS::UniString ("TestPlain"), "SetrawNameFromProperty : name не меняется (обычное свойство)", true);

    // ---- Граница: rawName без CharENTER (fromAttrib == false) -> спецветки some_stuff_* не срабатывают,
    //      даже если описание их содержит ----
    pvalue = ParamValue ();
    pvalue.rawName = "{@property:notfromattrib}";
    pvalue.name = "NotFromAttrib";
    property = {};
    property.definition.description = "some_stuff_th";
    ParamHelpers::SetrawNameFromProperty (pvalue, property);
    DBtest (pvalue.rawName, GS::UniString ("{@property:notfromattrib}"), "SetrawNameFromProperty : rawName не меняется без CharENTER (fromAttrib == false)", true);
    DBtest (pvalue.name, GS::UniString ("NotFromAttrib"), "SetrawNameFromProperty : name не меняется без CharENTER (fromAttrib == false)", true);

    // ---- Граница: rawName содержит CharENTER (fromAttrib == true) + описание "some_stuff_th" ----
    pvalue = ParamValue ();
    pvalue.rawName = GS::UniString ("{@property:some_stuff_th") + CharENTER + GS::UniString ("7") + BRACEEND;
    pvalue.name = "Original";
    property = {};
    property.definition.description = "Some_Stuff_TH"; // Проверка регистронезависимости (ToLowerCase внутри)
    ParamHelpers::SetrawNameFromProperty (pvalue, property);
    DBtest (pvalue.rawName.BeginsWith ("{@property:buildingmaterialproperties/some_stuff_th"), "SetrawNameFromProperty : rawName переписан на some_stuff_th", true);
    DBtest (pvalue.rawName.Contains ("7"), "SetrawNameFromProperty : сохранён индекс атрибута (some_stuff_th)", true);
    DBtest (pvalue.name, GS::UniString ("some_stuff_th"), "SetrawNameFromProperty : name == some_stuff_th", true);
    DBtest (!pvalue.val.formatstring.isEmpty, "SetrawNameFromProperty : formatstring задан для some_stuff_th", true);

    // ---- Граница: fromAttrib == true + описание "some_stuff_units" ----
    pvalue = ParamValue ();
    pvalue.rawName = GS::UniString ("{@property:some_stuff_units") + CharENTER + GS::UniString ("2") + BRACEEND;
    pvalue.name = "Original";
    property = {};
    property.definition.description = "some_stuff_units";
    ParamHelpers::SetrawNameFromProperty (pvalue, property);
    DBtest (pvalue.rawName.BeginsWith ("{@property:buildingmaterialproperties/some_stuff_units"), "SetrawNameFromProperty : rawName переписан на some_stuff_units", true);
    DBtest (pvalue.rawName.Contains ("2"), "SetrawNameFromProperty : сохранён индекс атрибута (some_stuff_units)", true);
    DBtest (pvalue.name, GS::UniString ("some_stuff_units"), "SetrawNameFromProperty : name == some_stuff_units", true);

    // ---- Граница: fromAttrib == true + описание "some_stuff_kzap" ----
    pvalue = ParamValue ();
    pvalue.rawName = GS::UniString ("{@property:some_stuff_kzap") + CharENTER + GS::UniString ("1") + BRACEEND;
    pvalue.name = "Original";
    property = {};
    property.definition.description = "some_stuff_kzap";
    ParamHelpers::SetrawNameFromProperty (pvalue, property);
    DBtest (pvalue.rawName.BeginsWith ("{@property:buildingmaterialproperties/some_stuff_kzap"), "SetrawNameFromProperty : rawName переписан на some_stuff_kzap", true);
    DBtest (pvalue.name, GS::UniString ("some_stuff_kzap"), "SetrawNameFromProperty : name == some_stuff_kzap", true);

    DBprnt ("TEST", "TestSetrawNameFromProperty : done");
    // Примечание: ветка description.Contains (SYNCCORRECTFLAG) не покрыта тестом - значение
    // константы SYNCCORRECTFLAG не определено в Helpers.hpp/cpp (внешний заголовок), поэтому
    // корректную тестовую строку для срабатывания этой ветки составить нельзя.
    return;
}

// -----------------------------------------------------------------------------
// Тест CheckIgnoreVal на граничных значениях
// -----------------------------------------------------------------------------
void TestCheckIgnoreVal ()
{
    DBprnt ("TEST", "TestCheckIgnoreVal");
    ParamValue param;
    SkipValues ignorevals;

    // ---- Граница: пустая строка + skip_empty == true -> игнорировать ----
    param = ParamValue ();
    param.val.type = API_PropertyStringValueType;
    param.val.uniStringValue = "";
    ignorevals = {};
    ignorevals.skip_empty = true;
    DBtest (ParamHelpers::CheckIgnoreVal (ignorevals, param), "CheckIgnoreVal : пустая строка + skip_empty", true);

    // ---- Граница: строка из пробелов + skip_empty == true (без skip_trim_empty) -> НЕ игнорировать ----
    param = ParamValue ();
    param.val.type = API_PropertyStringValueType;
    param.val.uniStringValue = "   ";
    ignorevals = {};
    ignorevals.skip_empty = true;
    DBtest (!ParamHelpers::CheckIgnoreVal (ignorevals, param), "CheckIgnoreVal : строка из пробелов + только skip_empty -> не игнорируется", true);

    // ---- Граница: строка из пробелов + skip_trim_empty == true -> игнорировать ----
    param = ParamValue ();
    param.val.type = API_PropertyStringValueType;
    param.val.uniStringValue = "   ";
    ignorevals = {};
    ignorevals.skip_trim_empty = true;
    DBtest (ParamHelpers::CheckIgnoreVal (ignorevals, param), "CheckIgnoreVal : строка из пробелов + skip_trim_empty", true);

    // ---- Граница: непустая строка + skip_empty/skip_trim_empty == true -> НЕ игнорировать ----
    param = ParamValue ();
    param.val.type = API_PropertyStringValueType;
    param.val.uniStringValue = "непустое значение";
    ignorevals = {};
    ignorevals.skip_empty = true;
    ignorevals.skip_trim_empty = true;
    DBtest (!ParamHelpers::CheckIgnoreVal (ignorevals, param), "CheckIgnoreVal : непустая строка -> не игнорируется", true);

    // ---- Граница: числовой (не строковый, не булевый) тип, doubleValue == 0 + skip_empty -> игнорировать ----
    param = ParamValue ();
    param.val.type = API_PropertyRealValueType;
    param.val.doubleValue = 0.0;
    ignorevals = {};
    ignorevals.skip_empty = true;
    DBtest (ParamHelpers::CheckIgnoreVal (ignorevals, param), "CheckIgnoreVal : Real == 0.0 + skip_empty", true);

    // ---- Граница: числовой тип, doubleValue != 0 + skip_empty -> НЕ игнорировать ----
    param = ParamValue ();
    param.val.type = API_PropertyRealValueType;
    param.val.doubleValue = 1.0;
    ignorevals = {};
    ignorevals.skip_empty = true;
    DBtest (!ParamHelpers::CheckIgnoreVal (ignorevals, param), "CheckIgnoreVal : Real == 1.0 + skip_empty -> не игнорируется", true);

    // ---- Граница: булевый тип НЕ подчиняется skip_empty/skip_trim_empty, даже если doubleValue == 0 ----
    param = ParamValue ();
    param.val.type = API_PropertyBooleanValueType;
    param.val.doubleValue = 0.0;
    param.val.boolValue = false;
    ignorevals = {};
    ignorevals.skip_empty = true;
    ignorevals.skip_trim_empty = true;
    DBtest (!ParamHelpers::CheckIgnoreVal (ignorevals, param), "CheckIgnoreVal : Boolean игнорирует skip_empty/skip_trim_empty", true);

    // ---- Граница: пустой список ignorevals.ignorevals -> false, даже без skip-флагов ----
    param = ParamValue ();
    param.val.type = API_PropertyStringValueType;
    param.val.uniStringValue = "любое значение";
    ignorevals = {};
    DBtest (!ParamHelpers::CheckIgnoreVal (ignorevals, param), "CheckIgnoreVal : пустой ignorevals и все флаги false -> false", true);

    // ---- Граница: точное совпадение со значением из списка ignorevals ----
    param = ParamValue ();
    param.val.type = API_PropertyStringValueType;
    param.val.uniStringValue = "ignoreme";
    ignorevals = {};
    ignorevals.ignorevals.Push ("ignoreme");
    DBtest (ParamHelpers::CheckIgnoreVal (ignorevals, param), "CheckIgnoreVal : точное совпадение со списком", true);

    // ---- Граница: список с шаблоном "*суффикс" - совпадение по окончанию строки ----
    param = ParamValue ();
    param.val.type = API_PropertyStringValueType;
    param.val.uniStringValue = "значение_суффикс";
    ignorevals = {};
    ignorevals.ignorevals.Push ("*суффикс");
    DBtest (ParamHelpers::CheckIgnoreVal (ignorevals, param), "CheckIgnoreVal : шаблон *суффикс (EndsWith)", true);

    // ---- Граница: список с шаблоном "префикс*" - совпадение по началу строки ----
    param = ParamValue ();
    param.val.type = API_PropertyStringValueType;
    param.val.uniStringValue = "префикс_значение";
    ignorevals = {};
    ignorevals.ignorevals.Push ("префикс*");
    DBtest (ParamHelpers::CheckIgnoreVal (ignorevals, param), "CheckIgnoreVal : шаблон префикс* (BeginsWith)", true);

    // ---- Граница: список задан, но ничего не совпадает -> false ----
    param = ParamValue ();
    param.val.type = API_PropertyStringValueType;
    param.val.uniStringValue = "совершенно другое значение";
    ignorevals = {};
    ignorevals.ignorevals.Push ("ignoreme");
    ignorevals.ignorevals.Push ("*суффикс");
    DBtest (!ParamHelpers::CheckIgnoreVal (ignorevals, param), "CheckIgnoreVal : список задан, но нет совпадений -> false", true);

    // ---- Граница: сравнение со списком идёт по обрезанной (trim) строке ----
    param = ParamValue ();
    param.val.type = API_PropertyStringValueType;
    param.val.uniStringValue = "  ignoreme  ";
    ignorevals = {};
    ignorevals.ignorevals.Push ("ignoreme");
    DBtest (ParamHelpers::CheckIgnoreVal (ignorevals, param), "CheckIgnoreVal : сравнение по trim-строке", true);

    DBprnt ("TEST", "TestCheckIgnoreVal : done");
    return;
}

// -----------------------------------------------------------------------------
// Тест ReadProperty - чтение значений свойств для элемента в ParamDictValue
// -----------------------------------------------------------------------------
// Примечание: ReadProperty обращается к живому ACAPI_Element_GetPropertyValues,
// поэтому полноценно проверить "успешный" путь (реальные значения свойств)
// без реального элемента на плане нельзя. Ниже проверяются детерминированные
// граничные случаи: обе защитные проверки в начале функции, и вызов с
// заведомо невалидным APINULLGuid, для которого ACAPI_Element_GetPropertyValues
// гарантированно не найдёт элемент и вернёт ошибку.
void TestReadProperty ()
{
    DBprnt ("TEST", "TestReadProperty");
    ParamDictValue params;
    GS::Array<API_PropertyDefinition> propertyDefinitions;

    // ---- Граница: params пуст -> немедленный false, независимо от propertyDefinitions ----
    params = {};
    propertyDefinitions = {};
    DBtest (!ParamHelpers::ReadProperty (APINULLGuid, params, propertyDefinitions), "ReadProperty : params пуст -> false", true);

    API_PropertyDefinition dummyDefinition = {};
    dummyDefinition.guid = APINULLGuid;
    propertyDefinitions.Push (dummyDefinition);
    DBtest (!ParamHelpers::ReadProperty (APINULLGuid, params, propertyDefinitions), "ReadProperty : params пуст + propertyDefinitions не пуст -> всё равно false", true);

    // ---- Граница: params не пуст, но propertyDefinitions пуст -> false ----
    params = {};
    ParamValue pvalue = {};
    pvalue.rawName = PROPERTYNAMEPREFIX + GS::UniString ("testreadproperty") + BRACEEND;
    pvalue.name = "TestReadProperty";
    params.Put (pvalue.rawName, pvalue);
    propertyDefinitions = {};
    DBtest (!ParamHelpers::ReadProperty (APINULLGuid, params, propertyDefinitions), "ReadProperty : propertyDefinitions пуст -> false", true);

    // ---- Граница: params и propertyDefinitions не пусты, но elemGuid == APINULLGuid ----
    // ACAPI_Element_GetPropertyValues не найдёт элемент по несуществующему guid и вернёт ошибку,
    // поэтому ReadProperty должен вернуть false, а словарь params - остаться без изменений.
    params = {};
    params.Put (pvalue.rawName, pvalue);
    propertyDefinitions = {};
    propertyDefinitions.Push (dummyDefinition);
    ParamValue pvalueBefore = *params.GetPtr (pvalue.rawName);
    DBtest (!ParamHelpers::ReadProperty (APINULLGuid, params, propertyDefinitions), "ReadProperty : APINULLGuid -> ACAPI-ошибка -> false", true);
    DBtest (params.GetPtr (pvalue.rawName)->isValid, pvalueBefore.isValid, "ReadProperty : значение в словаре не изменилось после ошибки ACAPI", true);

    DBprnt ("TEST", "TestReadProperty : done");
    return;
}

// -----------------------------------------------------------------------------
// Тест AddProperty - добавление/слияние массива API_Property в словарь ParamDictValue
// -----------------------------------------------------------------------------
// Примечание: rawName для "новых" свойств строится через SetrawNameFromProperty,
// которая для пустого pvalue (как в AddProperty) использует внешнюю GetPropertyFullName -
// её точный результат нам не известен. Чтобы не зависеть от этого, для сценария
// "совпадение по имени" ключ в params формируется той же функцией SetrawNameFromProperty,
// а не догадкой о содержимом rawName - так тест остаётся корректным независимо от
// конкретной реализации GetPropertyFullName.
void TestAddProperty ()
{
    DBprnt ("TEST", "TestAddProperty");
    ParamDictValue params;
    GS::Array<API_Property> properties;

    // ---- Строим свойство типа Integer, которого нет в словаре ----
    API_PropertyDefinition definition = {};
    definition.guid = APINULLGuid;
    definition.name = "TestAddPropertyInt";
    definition.description = "";
    definition.collectionType = API_PropertySingleCollectionType;
    definition.valueType = API_PropertyIntegerValueType;

    API_Property property = {};
    property.definition = definition;
    property.isDefault = false;
    #if defined(AC_22) || defined(AC_23)
    property.isEvaluated = true;
    #else
    property.status = API_Property_HasValue;
    #endif
    property.value.singleVariant.variant.type = API_PropertyIntegerValueType;
    property.value.singleVariant.variant.intValue = 55;

    // ---- Граница: свойства нет в params и needAdd == false -> пропускается, возвращает false ----
    params = {};
    properties = {};
    properties.Push (property);
    DBtest (!ParamHelpers::AddProperty (params, properties, APINULLGuid), "AddProperty : свойство отсутствует в словаре -> false", true);
    DBtest (params.GetSize () == 0, "AddProperty : словарь остаётся пустым, если совпадений нет", true);

    // ---- Вычисляем ожидаемый rawName той же функцией, что использует AddProperty внутри ----
    ParamValue probe = {};
    ParamHelpers::SetrawNameFromProperty (probe, property);
    GS::UniString expectedRawName = probe.rawName;

    // ---- Граница: свойство есть в params (совпадение по rawName) -> значение обновляется, возвращает true ----
    params = {};
    ParamValue placeholder = {};
    placeholder.rawName = expectedRawName;
    placeholder.name = probe.name;
    params.Put (expectedRawName, placeholder);
    properties = {};
    properties.Push (property);
    DBtest (ParamHelpers::AddProperty (params, properties, APINULLGuid), "AddProperty : совпадение по rawName -> true", true);
    if (const ParamValue* stored = params.GetPtr (expectedRawName)) {
        DBtest (stored->val.intValue, (Int32) 55, "AddProperty : значение свойства записано в словарь (intValue == 55)", true);
        DBtest (stored->val.type == API_PropertyIntegerValueType, "AddProperty : тип значения (Integer)", true);
        DBtest (stored->isValid, "AddProperty : isValid == true после успешной конвертации", true);
        DBtest (stored->fromGuid == APINULLGuid, "AddProperty : fromGuid == APINULLGuid (elemguid == APINULLGuid)", true);
    } else {
        DBtest (false, "AddProperty : значение должно быть найдено в словаре после Put", true);
    }

    // ---- Граница: свойство есть в params, но ConvertToParamValue не может его сконвертировать
    //      (valueType == Undefined) -> запись в словаре не меняется, AddProperty возвращает false ----
    API_PropertyDefinition undefDefinition = {};
    undefDefinition.guid = APINULLGuid;
    undefDefinition.name = "TestAddPropertyUndefined";
    undefDefinition.description = "";
    undefDefinition.collectionType = API_PropertySingleCollectionType;
    undefDefinition.valueType = API_PropertyUndefinedValueType;

    API_Property undefProperty = {};
    undefProperty.definition = undefDefinition;
    #if defined(AC_22) || defined(AC_23)
    undefProperty.isEvaluated = true;
    #else
    undefProperty.status = API_Property_HasValue;
    #endif

    ParamValue undefProbe = {};
    ParamHelpers::SetrawNameFromProperty (undefProbe, undefProperty);
    GS::UniString undefRawName = undefProbe.rawName;

    params = {};
    ParamValue undefPlaceholder = {};
    undefPlaceholder.rawName = undefRawName;
    undefPlaceholder.name = undefProbe.name;
    undefPlaceholder.isValid = false; // Значение ещё не заполнено - как это обычно бывает до первого чтения
    params.Put (undefRawName, undefPlaceholder);
    properties = {};
    properties.Push (undefProperty);
    DBtest (!ParamHelpers::AddProperty (params, properties, APINULLGuid), "AddProperty : Undefined valueType -> ConvertToParamValue не проходит -> false", true);
    if (const ParamValue* storedUndef = params.GetPtr (undefRawName)) {
        DBtest (!storedUndef->isValid, "AddProperty : запись в словаре не изменилась (осталась isValid == false)", true);
    }

    // ---- Граница: несколько свойств в одном вызове - одно совпадает, другое нет.
    //      Итоговый результат должен быть true (найдено хотя бы одно совпадение) ----
    params = {};
    params.Put (expectedRawName, placeholder);
    properties = {};
    properties.Push (property); // совпадёт
    API_Property noMatchProperty = {};
    API_PropertyDefinition noMatchDefinition = {};
    noMatchDefinition.guid = APINULLGuid;
    noMatchDefinition.name = "SomeCompletelyDifferentPropertyNameNoMatch";
    noMatchDefinition.description = "";
    noMatchDefinition.collectionType = API_PropertySingleCollectionType;
    noMatchDefinition.valueType = API_PropertyIntegerValueType;
    noMatchProperty.definition = noMatchDefinition;
    #if defined(AC_22) || defined(AC_23)
    noMatchProperty.isEvaluated = true;
    #else
    noMatchProperty.status = API_Property_HasValue;
    #endif
    noMatchProperty.value.singleVariant.variant.type = API_PropertyIntegerValueType;
    noMatchProperty.value.singleVariant.variant.intValue = 1;
    properties.Push (noMatchProperty); // не совпадёт (отдельное описание -> другой rawName)
    DBtest (ParamHelpers::AddProperty (params, properties, APINULLGuid), "AddProperty : несколько свойств, есть хотя бы одно совпадение -> true", true);
    DBtest (params.GetSize () == 1, "AddProperty : несовпавшее свойство не добавляется в словарь (needAdd == false)", true);

    DBprnt ("TEST", "TestAddProperty : done");
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
    const Int32 iseng = ID_ADDON_STRINGS + isEng ();
    GS::UniString undoString = RSGetIndString (iseng, UndoSyncId, ACAPI_GetOwnResModule ());
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
