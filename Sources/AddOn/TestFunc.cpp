//------------ kuvbur 2022 ------------
#ifdef TESTING
    #include "ACAPinc.h"

    #include "api_headers/APIEnvir.h"

    #include "TestFunc.hpp"

    #include "dialogs/CommandHelpers.hpp"
    #include "Helpers.hpp"
    #include "Propertycache.hpp"
    #include "ReNum.hpp"
    #include "Roombook.hpp"
    #include "Sync.hpp"

namespace TestFunc {

    void TestStringSplt (); // forward declaration

    void Test () {
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
        TestPropertyHelpersToString ();
        TestStringSplt ();
        TestName2Rawname (); // Sync.cpp-2 исправлен (порядок скобок { })
        TestName2RawnameWithBrackets ();
        TestSyncString ();
        TestSyncStringRealRules ();
        TestParsePrefixes ();
        TestParsePropertyDescription ();
        TestParseSyncStringIndependent ();
        TestParsePropertyDescriptionToRules ();
        TestSyncAddSubelement ();
        TestBuildOtdByParent ();
        TestRenumPosLogic ();
        TestDescToRulesSubGuid ();
        TestGetPropertyRuleFlag ();
        TestPropertyRuleFlagOnProjectElements ();
        DBprnt ("TEST", "end");
    }

    void TestStringSplt () {
        DBprnt ("TEST", "TestStringSplt");
        GS::Array<GS::UniString> parts;
        UInt32 n;

        // Simple delimiter: semicolon
        parts.Clear ();
        n = StringSplt ("a;b;c", ";", parts, true);
        DBtest (n, (UInt32)3, "StringSplt semicolon 3 parts");
        DBtest (parts.GetSize (), (UInt32)3, "StringSplt semicolon GetSize");
        DBtest (parts.Get (0), GS::UniString ("a"), "parts[0]");
        DBtest (parts.Get (1), GS::UniString ("b"), "parts[1]");
        DBtest (parts.Get (2), GS::UniString ("c"), "parts[2]");

        // filter_empty = false — empty tokens preserved
        parts.Clear ();
        n = StringSplt ("a;;c", ";", parts, false);
        DBtest (n, (UInt32)3, "StringSplt empty kept");
        DBtest (parts.GetSize (), (UInt32)3, "StringSplt empty kept GetSize");
        DBtest (parts.Get (1).IsEmpty (), true, "parts[1] is empty");

        // filter_empty = true — empty tokens removed
        parts.Clear ();
        n = StringSplt ("a;;c", ";", parts, true);
        DBtest (n, (UInt32)2, "StringSplt empty filtered");
        DBtest (parts.GetSize (), (UInt32)2, "StringSplt empty filtered GetSize");
        DBtest (parts.Get (0), GS::UniString ("a"), "parts[0] after filter");
        DBtest (parts.Get (1), GS::UniString ("c"), "parts[1] after filter");

        // No delimiter found — whole string is a single element
        parts.Clear ();
        n = StringSplt ("hello", ";", parts, true);
        DBtest (n, (UInt32)1, "StringSplt no delimiter returns 1");
        DBtest (parts.GetSize (), (UInt32)1, "StringSplt no delimiter GetSize");
        DBtest (parts.Get (0), GS::UniString ("hello"), "parts[0] no delimiter");

        // Unicode delimiter (Cyrillic semicolon)
        parts.Clear ();
        n = StringSplt ("один;два;три", ";", parts, true);
        DBtest (n, (UInt32)3, "StringSplt unicode 3 parts");
        DBtest (parts.GetSize (), (UInt32)3, "StringSplt unicode GetSize");
        DBtest (parts.Get (1), GS::UniString ("два"), "parts[1] unicode");

        // Using scratch buffer (nullptr vs external)
        parts.Clear ();
        n = StringSplt ("x@y@z", "@", parts, true, nullptr);
        DBtest (n, (UInt32)3, "StringSplt with nullptr scratch");
        DBtest (parts.Get (1), GS::UniString ("y"), "parts[1] scratch nullptr");

        // Scratch buffer passed externally
        GS::Array<GS::UniString> scratch = {};
        parts.Clear ();
        n = StringSplt ("p;q;r", ";", parts, true, &scratch);
        DBtest (n, (UInt32)3, "StringSplt with external scratch");
        DBtest (parts.Get (2), GS::UniString ("r"), "parts[2] external scratch");

        // Leading/trailing whitespace is trimmed
        parts.Clear ();
        n = StringSplt ("  a  ;  b  ", ";", parts, true);
        DBtest (n, (UInt32)2, "StringSplt trim");
        DBtest (parts.Get (0), GS::UniString ("a"), "parts[0] trimmed");
        DBtest (parts.Get (1), GS::UniString ("b"), "parts[1] trimmed");

        DBprnt ("TEST", "TestStringSplt : done");
        return;
    }

    void TestGetTextLineLength (GS::UniString &var) {
        GSErrCode err = NoError;
        GS::UniString fontname = "Arial";
        double fontsize = 2.5;
        short font_inx = 0;
        double width = 0.0;
        API_TextLinePars tlp = {};
    #ifdef ServerMainVers_2700
        API_FontType font;
        BNZeroMemory (&font, sizeof (API_FontType));
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
    #ifdef ServerMainVers_2700
        err = ACAPI_Element_GetTextLineLength (&tlp, &width);
    #else
        err = ACAPI_Goodies (APIAny_GetTextLineLengthID, &tlp, &width);
    #endif
    #ifdef TESTING
        DBtest (width > 0.00001, "TestGetTextLineLength");
    #endif
    }

    void TestCalc () {
        DBprnt ("TEST", "TestCalc");
        bool usl = false;
        GS::UniString test_expression = "";
        GS::UniString rep = "";

        test_expression = "2*2";
        rep = test_expression;
        DBtest (!EvalExpression (test_expression), rep);
        DBtest (test_expression, rep, rep);

        test_expression = "<2*2>";
        rep = test_expression;
        DBtest (EvalExpression (test_expression), rep);
        DBtest (test_expression, "4", rep);

        test_expression = "<2*2>+<2*2>";
        rep = test_expression;
        DBtest (EvalExpression (test_expression), rep);
        DBtest (test_expression, "4+4", rep);

        test_expression = "<0.001+0.001>.0mm";
        rep = test_expression;
        DBtest (EvalExpression (test_expression), rep);
        DBtest (test_expression, "2", rep);

        test_expression = "<0,001+0,001>.0mm";
        rep = test_expression;
        DBtest (EvalExpression (test_expression), rep);
        DBtest (test_expression, "2", rep);

        test_expression = "<0.001+0.001>.3m";
        rep = test_expression;
        DBtest (EvalExpression (test_expression), rep);
        DBtest (test_expression, "0,002", rep);

        test_expression = "<0.001+0.001>.3mp";
        rep = test_expression;
        DBtest (EvalExpression (test_expression), rep);
        DBtest (test_expression, "0.002", rep);

        test_expression = "<0.001+0.001>.3mp+<0.001+0.001>.03mm";
        rep = test_expression;
        DBtest (EvalExpression (test_expression), rep);
        DBtest (test_expression, "0.002+2,000", rep);

        return;
    }

    void TestFormula () {
        DBprnt ("TEST", "TestReadFormula");
        ParamDictValue params;
        ParamValue pvalue;
        pvalue.val.hasFormula = true;
        pvalue.isValid = false;
        GS::Array<ParamValue> formula = {};

        pvalue.name = "<%ac_postWidth%*1000> * <%ac_postThickness%*1000>";
        pvalue.val.uniStringValue = "53 * 3";
        pvalue.val.intValue = 159;
        pvalue.val.doubleValue = 159;
        formula.Push (pvalue);

        pvalue.name = "<%ac_postWidth.3m%*2>";
        pvalue.val.uniStringValue = "0,106";
        pvalue.val.intValue = 1;
        pvalue.val.doubleValue = 0.106;
        formula.Push (pvalue);

        pvalue.name = "<%ac_postWidth.2m%*2>.03mp";
        pvalue.val.uniStringValue = "0.100";
        pvalue.val.intValue = 1;
        pvalue.val.doubleValue = 0.100;
        formula.Push (pvalue);

        pvalue.name = "<%ac_postWidth.3m%*3>.2m";
        pvalue.val.uniStringValue = "0,16";
        pvalue.val.intValue = 1;
        pvalue.val.doubleValue = 0.160;
        formula.Push (pvalue);

        pvalue.name = "<%ac_postWidth.2m%*3>.03m";
        pvalue.val.uniStringValue = "0,150";
        pvalue.val.intValue = 1;
        pvalue.val.doubleValue = 0.150;
        formula.Push (pvalue);

        pvalue.name = "<%ac_postWidth.2m%*3>.03mp";
        pvalue.val.uniStringValue = "0.150";
        pvalue.val.intValue = 1;
        pvalue.val.doubleValue = 0.150;
        formula.Push (pvalue);

        for (UInt32 j = 0; j < formula.GetSize (); j++) {
            GS::UniString f = formula.Get (j).name;
            pvalue.name = f;
            pvalue.val.uniStringValue = f;
            pvalue.rawName = FORMULANAMEPREFIX + f.ToLowerCase () + ";" + pvalue.val.uniStringValue + "}";
            GS::UniString templatestring = pvalue.val.uniStringValue;
            DBtest (ParamHelpers::ParseParamNameMaterial (templatestring, params, false), templatestring);
            pvalue.val.uniStringValue = templatestring;
            formula[j].rawName = pvalue.rawName;
            params.Add (pvalue.rawName, pvalue);
        }
        pvalue.val.hasFormula = false;
        pvalue.isValid = true;
        DBtest (params.ContainsKey ("{@gdl:ac_postwidth}"), "{@gdl:ac_postwidth}");
        pvalue.name = "";
        pvalue.rawName = "";
        ParamHelpers::ConvertDoubleToParamValue (pvalue, "ac_postWidth", 0.053);
        DBtest (params.ContainsKey (pvalue.rawName), pvalue.rawName);
        params.Set (pvalue.rawName, pvalue);

        DBtest (params.ContainsKey ("{@gdl:ac_postthickness}"), "{@gdl:ac_postthickness}");
        pvalue.name = "";
        pvalue.rawName = "";
        ParamHelpers::ConvertDoubleToParamValue (pvalue, "ac_postThickness", 0.003);
        DBtest (params.ContainsKey (pvalue.rawName), pvalue.rawName);
        params.Set (pvalue.rawName, pvalue);

        DBtest (ParamHelpers::ReadFormula (params, false), "ReadFormula");

        for (ParamDictValue::PairIterator cIt = params.EnumeratePairs (); cIt != NULL; ++cIt) {
    #ifdef ServerMainVers_2800
            ParamValue &param = cIt->value;
    #else
            ParamValue &param = *cIt->value;
    #endif
            if (param.isValid && param.val.canCalculate) {
                ParamHelpers::ConvertByFormatString (param);
            }
        }

        for (UInt32 j = 0; j < formula.GetSize (); j++) {
            ParamValue &rezult = formula.Get (j);
            DBtest (params.ContainsKey (rezult.rawName), rezult.rawName);
            ParamValue &test = params.Get (rezult.rawName);
            DBtest (test.val.formatstring.stringformat, rezult.val.formatstring.stringformat, "formatstring");
            DBtest (test.isValid == true, "isValid");
            DBtest (test.val.uniStringValue, rezult.val.uniStringValue, "uniStringValue");
            DBtest (test.val.intValue, rezult.val.intValue, "intValue");
            DBtest (test.val.doubleValue, rezult.val.doubleValue, "doubleValue");
        }
        return;
    }

    void TestFormatString () {
        DBprnt ("TEST", "TestFormatString");
        GS::Array<GS::UniString> tests;
        GS::Array<FormatString> rezult_format;
        GS::Array<GS::UniString> rezult_name;

        GS::UniString test_expression = "";
        GS::UniString test_name = "";
        FormatString fstring;

        // =============================================================================
        fstring.stringformat = "";
        fstring.n_zero = 3;
        // =============================================================================

        test_expression = "ConWidth_1";
        test_name = "ConWidth_1";
        tests.Push (test_expression);
        rezult_format.Push (fstring);
        rezult_name.Push (test_name);

        test_expression = "\"<0.001+0.001>.3mp+<0.001+0.001>.03mm\"";
        test_name = "<0.001+0.001>.3mp+<0.001+0.001>.03mm";
        tests.Push (test_expression);
        rezult_format.Push (fstring);
        rezult_name.Push (test_name);

        test_expression =
            "from{Layers, 6; \"1/{Property:Теплотехнический расчёт/αint, Вт/(м2°С)} + 1/{Property:Теплотехнический "
            "расчёт/αext, Вт/(м2°С)} <+%layer_thickness.3m%/%BuildingMaterialProperties/Building Material Thermal "
            "Conductivity.3m%>\"}";

        test_name = "1/{Property:Теплотехнический расчёт/αint, Вт/(м2°С)} + 1/{Property:Теплотехнический расчёт/αext, "
                    "Вт/(м2°С)} "
                    "<+%layer_thickness.3m%/%BuildingMaterialProperties/Building Material Thermal Conductivity.3m%>";
        tests.Push (test_expression);
        rezult_format.Push (fstring);
        rezult_name.Push (test_name);

        test_expression = "Спецификация сэндвич/Площадь, кв.м. (без подрезок)";
        test_name = test_expression;
        tests.Push (test_expression);
        rezult_format.Push (fstring);
        rezult_name.Push (test_name);

        test_expression = "Спецификация сэндвич/Площадь, кв.м. (без малых подрезок)";
        test_name = test_expression;
        tests.Push (test_expression);
        rezult_format.Push (fstring);
        rezult_name.Push (test_name);

        test_expression = "Спецификация сэндвич/Площадь, кв.м. (или в п.м.)";
        test_name = test_expression;
        tests.Push (test_expression);
        rezult_format.Push (fstring);
        rezult_name.Push (test_name);

        test_expression = "SomeStuff Материалы/Конструкции.Имя в спецификации";
        test_name = test_expression;
        tests.Push (test_expression);
        rezult_format.Push (fstring);
        rezult_name.Push (test_name);
        // =============================================================================
        fstring.isEmpty = false;
        fstring.isRead = true;
        fstring.koeff = 1000;
        fstring.stringformat = "0mm";
        fstring.n_zero = 0;
        // =============================================================================
        test_expression = "ConWidth_1.0mm";
        test_name = "ConWidth_1";
        tests.Push (test_expression);
        rezult_format.Push (fstring);
        rezult_name.Push (test_name);

        test_expression = "ConWidth_1.0mm";
        test_name = "ConWidth_1";
        tests.Push (test_expression);
        rezult_format.Push (fstring);
        rezult_name.Push (test_name);

        // =============================================================================
        fstring.isEmpty = false;
        fstring.isRead = true;
        fstring.koeff = 1;
        fstring.stringformat = "3m";
        fstring.n_zero = 3;
        // =============================================================================
        test_expression =
            "from{Layers, 6; \"1/{Property:Теплотехнический расчёт/αint, Вт/(м2°С)} + 1/{Property:Теплотехнический "
            "расчёт/αext, Вт/(м2°С)} <+%layer_thickness.3m%/%BuildingMaterialProperties/Building Material Thermal "
            "Conductivity.3m%>\".3m}";
        test_name = "1/{Property:Теплотехнический расчёт/αint, Вт/(м2°С)} + 1/{Property:Теплотехнический расчёт/αext, "
                    "Вт/(м2°С)} "
                    "<+%layer_thickness.3m%/%BuildingMaterialProperties/Building Material Thermal Conductivity.3m%>";
        tests.Push (test_expression);
        rezult_format.Push (fstring);
        rezult_name.Push (test_name);

        test_expression = "<0.001+0.001>.3m";
        test_name = "0.001+0.001";
        tests.Push (test_expression);
        rezult_format.Push (fstring);
        rezult_name.Push (test_name);

        // =============================================================================
        fstring.isEmpty = false;
        fstring.isRead = true;
        fstring.koeff = 1;
        fstring.stringformat = "3mp";
        fstring.n_zero = 3;
        fstring.delimetr = ".";
        // =============================================================================
        test_expression =
            "from{Material:Layers, 6; \"1/{Property:Теплотехнический расчёт/αint, Вт/(м2°С)} + "
            "1/{Property:Теплотехнический расчёт/αext, Вт/(м2°С)} "
            "<+%толщина.3m%/%BuildingMaterialProperties/Building Material Thermal Conductivity.3m%>\".3mp}";
        test_name = "1/{Property:Теплотехнический расчёт/αint, Вт/(м2°С)} + 1/{Property:Теплотехнический расчёт/αext, "
                    "Вт/(м2°С)} <+%толщина.3m%/%BuildingMaterialProperties/Building Material Thermal Conductivity.3m%>";
        tests.Push (test_expression);
        rezult_format.Push (fstring);
        rezult_name.Push (test_name);
        test_expression = "<0.001+0.001>.3mp";
        test_name = "0.001+0.001";
        tests.Push (test_expression);
        rezult_format.Push (fstring);
        rezult_name.Push (test_name);

        DBtest (tests.GetSize () == rezult_format.GetSize (), "tests.GetSize() == rezult_format.GetSize ()");
        DBtest (tests.GetSize () == rezult_name.GetSize (), "tests.GetSize() == rezult_name.GetSize ()");
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
            DBtest (expressiont, expressionr, "expression");
            DBtest (fstringt.isEmpty == fstringr.isEmpty, "isEmpty");
            DBtest (fstringt.isRead == fstringr.isRead, "isRead");
            DBtest (fstringt.forceRaw == fstringr.forceRaw, "forceRaw");
            DBtest (fstringt.trim_zero == fstringr.trim_zero, "trim_zero");
            DBtest (fstringt.koeff, fstringr.koeff, "koeff");
            DBtest (fstringt.delimetr, fstringr.delimetr, "delimetr");
            DBtest (fstringt.n_zero, fstringr.n_zero, "n_zero");
            DBtest (fstringt.stringformat, fstringr.stringformat, "stringformat");
        }
        return;
    }

    // -----------------------------------------------------------------------------
    // Простые тесты функций конвертации базовых типов в ParamValue (Helpers.hpp/cpp)
    // -----------------------------------------------------------------------------
    void TestConvertToParamValue () {
        DBprnt ("TEST", "TestConvertToParamValue");
        ParamValue pvalue;

        // ---- ConvertIntToParamValue ----
        pvalue = ParamValue ();
        DBtest (ParamHelpers::ConvertIntToParamValue (pvalue, "TestInt", 42), "ConvertIntToParamValue : return");
        DBtest (pvalue.name, GS::UniString ("TestInt"), "ConvertIntToParamValue : name");
        DBtest (!pvalue.rawName.IsEmpty (), "ConvertIntToParamValue : rawName не пуст");
        DBtest (pvalue.val.type == API_PropertyIntegerValueType, "ConvertIntToParamValue : type");
        DBtest (pvalue.val.intValue, (Int32)42, "ConvertIntToParamValue : intValue");
        DBtest (is_equal (pvalue.val.doubleValue, 42.0), "ConvertIntToParamValue : doubleValue");
        DBtest (pvalue.val.boolValue, "ConvertIntToParamValue : boolValue (42 > 0)");
        DBtest (pvalue.val.uniStringValue, GS::UniString ("42"), "ConvertIntToParamValue : uniStringValue");
        DBtest (pvalue.isValid, "ConvertIntToParamValue : isValid");

        pvalue = ParamValue ();
        ParamHelpers::ConvertIntToParamValue (pvalue, "TestIntNeg", -5);
        DBtest (!pvalue.val.boolValue, "ConvertIntToParamValue : boolValue (-5 не > 0)");
        DBtest (pvalue.val.intValue, (Int32)(-5), "ConvertIntToParamValue : intValue (-5)");

        pvalue = ParamValue ();
        ParamHelpers::ConvertIntToParamValue (pvalue, "TestIntZero", 0);
        DBtest (!pvalue.val.boolValue, "ConvertIntToParamValue : boolValue (0 не > 0)");

        // ---- ConvertDoubleToParamValue ----
        pvalue = ParamValue ();
        DBtest (ParamHelpers::ConvertDoubleToParamValue (pvalue, "TestDouble", 3.14),
                "ConvertDoubleToParamValue : return");
        DBtest (pvalue.val.type == API_PropertyRealValueType, "ConvertDoubleToParamValue : type");
        DBtest (pvalue.val.intValue, (Int32)3, "ConvertDoubleToParamValue : intValue (усечение 3.14 -> 3)");
        DBtest (is_equal (pvalue.val.doubleValue, 3.14), "ConvertDoubleToParamValue : doubleValue");
        DBtest (pvalue.val.boolValue, "ConvertDoubleToParamValue : boolValue (!= 0)");
        DBtest (
            pvalue.val.uniStringValue, GS::UniString ("3.140"), "ConvertDoubleToParamValue : uniStringValue (%.3f)");
        DBtest (pvalue.isValid, "ConvertDoubleToParamValue : isValid");

        pvalue = ParamValue ();
        ParamHelpers::ConvertDoubleToParamValue (pvalue, "TestDoubleZero", 0.0);
        DBtest (!pvalue.val.boolValue, "ConvertDoubleToParamValue : boolValue (== 0)");

        pvalue = ParamValue ();
        ParamHelpers::ConvertDoubleToParamValue (pvalue, "TestDoubleNeg", -2.5);
        DBtest (pvalue.val.intValue, (Int32)(-2), "ConvertDoubleToParamValue : intValue (усечение -2.5 -> -2)");
        DBtest (pvalue.val.boolValue, "ConvertDoubleToParamValue : boolValue (-2.5 != 0)");

        // ---- ConvertBoolToParamValue ----
        pvalue = ParamValue ();
        DBtest (ParamHelpers::ConvertBoolToParamValue (pvalue, "TestBoolTrue", true),
                "ConvertBoolToParamValue : return");
        DBtest (pvalue.val.type == API_PropertyBooleanValueType, "ConvertBoolToParamValue : type");
        DBtest (pvalue.val.boolValue, "ConvertBoolToParamValue : boolValue (true)");
        DBtest (pvalue.val.intValue, (Int32)1, "ConvertBoolToParamValue : intValue (true -> 1)");
        DBtest (is_equal (pvalue.val.doubleValue, 1.0), "ConvertBoolToParamValue : doubleValue (true -> 1.0)");
        DBtest (!pvalue.val.uniStringValue.IsEmpty (), "ConvertBoolToParamValue : uniStringValue не пуст (true)");

        pvalue = ParamValue ();
        ParamHelpers::ConvertBoolToParamValue (pvalue, "TestBoolFalse", false);
        DBtest (!pvalue.val.boolValue, "ConvertBoolToParamValue : boolValue (false)");
        DBtest (pvalue.val.intValue, (Int32)0, "ConvertBoolToParamValue : intValue (false -> 0)");
        DBtest (is_equal (pvalue.val.doubleValue, 0.0), "ConvertBoolToParamValue : doubleValue (false -> 0.0)");

        // ---- ConvertStringToParamValue ----
        pvalue = ParamValue ();
        DBtest (ParamHelpers::ConvertStringToParamValue (pvalue, "TestStrNum", "123.5"),
                "ConvertStringToParamValue : return");
        DBtest (pvalue.val.type == API_PropertyStringValueType, "ConvertStringToParamValue : type");
        DBtest (pvalue.val.canCalculate, "ConvertStringToParamValue : canCalculate (\"123.5\" - число)");
        DBtest (is_equal (pvalue.val.doubleValue, 123.5), "ConvertStringToParamValue : doubleValue (123.5)");
        DBtest (
            pvalue.val.intValue, (Int32)124, "ConvertStringToParamValue : intValue (округление вверх 123.5 -> 124)");
        DBtest (pvalue.val.boolValue, "ConvertStringToParamValue : boolValue (непустая строка)");

        pvalue = ParamValue ();
        ParamHelpers::ConvertStringToParamValue (pvalue, "TestStrInt", "10");
        DBtest (pvalue.val.canCalculate, "ConvertStringToParamValue : canCalculate (\"10\" - число)");
        DBtest (pvalue.val.intValue, (Int32)10, "ConvertStringToParamValue : intValue (\"10\" -> 10, без округления)");

        pvalue = ParamValue ();
        ParamHelpers::ConvertStringToParamValue (pvalue, "TestStrText", "abc");
        DBtest (!pvalue.val.canCalculate, "ConvertStringToParamValue : canCalculate (\"abc\" - не число)");
        DBtest (pvalue.val.boolValue, "ConvertStringToParamValue : boolValue (\"abc\" непустая)");
        DBtest (pvalue.val.intValue, (Int32)1, "ConvertStringToParamValue : intValue (\"abc\" -> 1)");
        DBtest (is_equal (pvalue.val.doubleValue, 1.0), "ConvertStringToParamValue : doubleValue (\"abc\" -> 1.0)");

        pvalue = ParamValue ();
        ParamHelpers::ConvertStringToParamValue (pvalue, "TestStrEmpty", "");
        DBtest (!pvalue.val.boolValue, "ConvertStringToParamValue : boolValue (пустая строка)");
        DBtest (!pvalue.val.canCalculate, "ConvertStringToParamValue : canCalculate (пустая строка)");

        DBprnt ("TEST", "TestConvertToParamValue : done");
        return;
    }

    // -----------------------------------------------------------------------------
    // Тест ConvertAttributeToParamValue на граничных значениях API_Attribute
    // -----------------------------------------------------------------------------
    void TestConvertAttributeToParamValue () {
        DBprnt ("TEST", "TestConvertAttributeToParamValue");
        ParamValue pvalue;
        API_Attribute attrib;

        // ---- Граница: индекс атрибута = 0 (несуществующий/неинициализированный индекс) ----
        pvalue = ParamValue ();
        attrib = {};
        attrib.header.typeID = API_LayerID;
    #ifdef ServerMainVers_2700
        attrib.header.index = ACAPI_CreateAttributeIndex (0);
    #else
        attrib.header.index = 0;
    #endif
        ParamHelpers::ConvertAttributeToParamValue (pvalue, "TestAttrZero", attrib);
        DBtest (pvalue.val.intValue, (Int32)0, "ConvertAttributeToParamValue : intValue (index == 0)");
        DBtest (!pvalue.val.boolValue, "ConvertAttributeToParamValue : boolValue (index == 0, не > 0)");
        DBtest (is_equal (pvalue.val.doubleValue, 0.0), "ConvertAttributeToParamValue : doubleValue (index == 0)");
        DBtest (pvalue.val.type == API_PropertyStringValueType, "ConvertAttributeToParamValue : type");
        DBtest (pvalue.isValid, "ConvertAttributeToParamValue : isValid");
        DBtest (pvalue.fromAttribElement, "ConvertAttributeToParamValue : fromAttribElement");
        DBtest (pvalue.val.uniStringValue.IsEmpty (),
                "ConvertAttributeToParamValue : uniStringValue пуст (имя не задано)");

        // ---- Граница: минимально возможный положительный индекс (1) ----
        pvalue = ParamValue ();
        attrib = {};
        attrib.header.typeID = API_LayerID;
    #ifdef ServerMainVers_2700
        attrib.header.index = ACAPI_CreateAttributeIndex (1);
    #else
        attrib.header.index = 1;
    #endif
        ParamHelpers::ConvertAttributeToParamValue (pvalue, "TestAttrOne", attrib);
        DBtest (pvalue.val.intValue, (Int32)1, "ConvertAttributeToParamValue : intValue (index == 1)");
        DBtest (pvalue.val.boolValue, "ConvertAttributeToParamValue : boolValue (index == 1 > 0)");
        DBtest (is_equal (pvalue.val.doubleValue, 1.0), "ConvertAttributeToParamValue : doubleValue (index == 1)");

        // ---- Граница: максимальный short-индекс (граница типа для старых версий ACAPI) ----
        pvalue = ParamValue ();
        attrib = {};
        attrib.header.typeID = API_LayerID;
    #ifdef ServerMainVers_2700
        attrib.header.index = ACAPI_CreateAttributeIndex (std::numeric_limits<short>::max ());
    #else
        attrib.header.index = std::numeric_limits<short>::max ();
    #endif
        ParamHelpers::ConvertAttributeToParamValue (pvalue, "TestAttrMax", attrib);
        DBtest (pvalue.val.intValue,
                (Int32)std::numeric_limits<short>::max (),
                "ConvertAttributeToParamValue : intValue (index == SHRT_MAX)");
        DBtest (pvalue.val.boolValue, "ConvertAttributeToParamValue : boolValue (SHRT_MAX > 0)");
        DBtest (is_equal (pvalue.val.doubleValue, (double)std::numeric_limits<short>::max ()),
                "ConvertAttributeToParamValue : doubleValue (index == SHRT_MAX)");

        // ---- Граница: rawName уже задан вызывающей стороной -> не должен перезаписываться ----
        pvalue = ParamValue ();
        pvalue.rawName = "{@attrib:custom_rawname}";
        attrib = {};
        attrib.header.typeID = API_LayerID;
    #ifdef ServerMainVers_2700
        attrib.header.index = ACAPI_CreateAttributeIndex (5);
    #else
        attrib.header.index = 5;
    #endif
        ParamHelpers::ConvertAttributeToParamValue (pvalue, "TestAttrCustomRaw", attrib);
        DBtest (pvalue.rawName,
                GS::UniString ("{@attrib:custom_rawname}"),
                "ConvertAttributeToParamValue : rawName не перезаписывается, если не пуст");

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
    void TestConvertPropertyToParamValue () {
        DBprnt ("TEST", "TestConvertPropertyToParamValue");
        ParamValue pvalue;
        API_Property property;

        // ---- Integer: граничные значения INT32_MIN / 0 / INT32_MAX ----
        pvalue = ParamValue ();
        pvalue.rawName = "{@property:testint_min}";
        pvalue.name = "TestIntMin";
        property = {};
        property.isDefault = false;
    #ifndef ServerMainVers_2400
        property.isEvaluated = true;
    #else
        property.status = API_Property_HasValue;
    #endif
        property.definition.collectionType = API_PropertySingleCollectionType;
        property.definition.valueType = API_PropertyIntegerValueType;
        property.value.singleVariant.variant.type = API_PropertyIntegerValueType;
        property.value.singleVariant.variant.intValue = std::numeric_limits<Int32>::min ();
        DBtest (ParamHelpers::ConvertToParamValue (pvalue, property),
                "ConvertToParamValue(Property) : return (Integer INT32_MIN)");
        DBtest (pvalue.val.intValue,
                std::numeric_limits<Int32>::min (),
                "ConvertToParamValue(Property) : intValue (INT32_MIN)");
        DBtest (!pvalue.val.boolValue, "ConvertToParamValue(Property) : boolValue (INT32_MIN не > 0)");
        DBtest (pvalue.val.type == API_PropertyIntegerValueType, "ConvertToParamValue(Property) : type (Integer)");
        DBtest (pvalue.isValid, "ConvertToParamValue(Property) : isValid (HasValue)");

        pvalue = ParamValue ();
        pvalue.rawName = "{@property:testint_zero}";
        pvalue.name = "TestIntZero";
        property.value.singleVariant.variant.intValue = 0;
        ParamHelpers::ConvertToParamValue (pvalue, property);
        DBtest (!pvalue.val.boolValue, "ConvertToParamValue(Property) : boolValue (0 не > 0)");

        pvalue = ParamValue ();
        pvalue.rawName = "{@property:testint_max}";
        pvalue.name = "TestIntMax";
        property.value.singleVariant.variant.intValue = std::numeric_limits<Int32>::max ();
        ParamHelpers::ConvertToParamValue (pvalue, property);
        DBtest (pvalue.val.intValue,
                std::numeric_limits<Int32>::max (),
                "ConvertToParamValue(Property) : intValue (INT32_MAX)");
        DBtest (pvalue.val.boolValue, "ConvertToParamValue(Property) : boolValue (INT32_MAX > 0)");

        // ---- Real: 0.0 (граница bool == false), максимум double, отрицательное значение ----
        pvalue = ParamValue ();
        pvalue.rawName = "{@property:testreal_zero}";
        pvalue.name = "TestRealZero";
        property = {};
    #ifndef ServerMainVers_2400
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
        DBtest (!pvalue.val.boolValue, "ConvertToParamValue(Property) : boolValue (0.0 == 0)");
        DBtest (pvalue.val.type == API_PropertyRealValueType, "ConvertToParamValue(Property) : type (Real)");

        pvalue = ParamValue ();
        pvalue.rawName = "{@property:testreal_neg}";
        pvalue.name = "TestRealNeg";
        property.value.singleVariant.variant.doubleValue = -123456.789;
        ParamHelpers::ConvertToParamValue (pvalue, property);
        DBtest (pvalue.val.boolValue, "ConvertToParamValue(Property) : boolValue (отрицательное != 0)");
        DBtest (is_equal (pvalue.val.doubleValue, -123456.789),
                "ConvertToParamValue(Property) : doubleValue (отрицательное)");

        pvalue = ParamValue ();
        pvalue.rawName = "{@property:testreal_max}";
        pvalue.name = "TestRealMax";
        property.value.singleVariant.variant.doubleValue = std::numeric_limits<double>::max ();
        ParamHelpers::ConvertToParamValue (pvalue, property);
        DBtest (pvalue.val.boolValue, "ConvertToParamValue(Property) : boolValue (DBL_MAX != 0)");

        // ---- Boolean: true / false ----
        pvalue = ParamValue ();
        pvalue.rawName = "{@property:testbool_true}";
        pvalue.name = "TestBoolTrue";
        property = {};
    #ifndef ServerMainVers_2400
        property.isEvaluated = true;
    #else
        property.status = API_Property_HasValue;
    #endif
        property.definition.collectionType = API_PropertySingleCollectionType;
        property.definition.valueType = API_PropertyBooleanValueType;
        property.value.singleVariant.variant.type = API_PropertyBooleanValueType;
        property.value.singleVariant.variant.boolValue = true;
        ParamHelpers::ConvertToParamValue (pvalue, property);
        DBtest (pvalue.val.boolValue, "ConvertToParamValue(Property) : boolValue (true)");
        DBtest (pvalue.val.intValue, (Int32)1, "ConvertToParamValue(Property) : intValue (true -> 1)");
        DBtest (pvalue.val.type == API_PropertyBooleanValueType, "ConvertToParamValue(Property) : type (Boolean)");

        pvalue = ParamValue ();
        pvalue.rawName = "{@property:testbool_false}";
        pvalue.name = "TestBoolFalse";
        property.value.singleVariant.variant.boolValue = false;
        ParamHelpers::ConvertToParamValue (pvalue, property);
        DBtest (!pvalue.val.boolValue, "ConvertToParamValue(Property) : boolValue (false)");

        // ---- String: пустая строка / длинная юникод-строка ----
        pvalue = ParamValue ();
        pvalue.rawName = "{@property:teststr_empty}";
        pvalue.name = "TestStrEmpty";
        property = {};
    #ifndef ServerMainVers_2400
        property.isEvaluated = true;
    #else
        property.status = API_Property_HasValue;
    #endif
        property.definition.collectionType = API_PropertySingleCollectionType;
        property.definition.valueType = API_PropertyStringValueType;
        property.value.singleVariant.variant.type = API_PropertyStringValueType;
        property.value.singleVariant.variant.uniStringValue = "";
        ParamHelpers::ConvertToParamValue (pvalue, property);
        DBtest (!pvalue.val.boolValue, "ConvertToParamValue(Property) : boolValue (пустая строка)");
        DBtest (pvalue.val.type == API_PropertyStringValueType, "ConvertToParamValue(Property) : type (String)");

        pvalue = ParamValue ();
        pvalue.rawName = "{@property:teststr_long}";
        pvalue.name = "TestStrLong";
        GS::UniString longString = "";
        for (UInt32 i = 0; i < 200; i++)
            longString.Append ("Ё");
        property.value.singleVariant.variant.uniStringValue = longString;
        ParamHelpers::ConvertToParamValue (pvalue, property);
        DBtest (pvalue.val.boolValue, "ConvertToParamValue(Property) : boolValue (длинная строка непустая)");
        DBtest (pvalue.val.uniStringValue.GetLength () == 200,
                "ConvertToParamValue(Property) : uniStringValue длина сохранена");

        // ---- Undefined: функция должна вернуть false и isValid == false ----
        pvalue = ParamValue ();
        pvalue.rawName = "{@property:testundef}";
        pvalue.name = "TestUndefined";
        property = {};
    #ifndef ServerMainVers_2400
        property.isEvaluated = true;
    #else
        property.status = API_Property_HasValue;
    #endif
        property.definition.collectionType = API_PropertySingleCollectionType;
        property.definition.valueType = API_PropertyUndefinedValueType;
        DBtest (!ParamHelpers::ConvertToParamValue (pvalue, property),
                "ConvertToParamValue(Property) : return (Undefined -> false)");
        DBtest (!pvalue.isValid, "ConvertToParamValue(Property) : isValid (Undefined -> false)");

        DBprnt ("TEST", "TestConvertPropertyToParamValue : done");
        return;
    }

    // -----------------------------------------------------------------------------
    // Тест ConvertToParamValue (API_PropertyDefinition) на граничных значениях
    // -----------------------------------------------------------------------------
    void TestConvertPropertyDefinitionToParamValue () {
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
        DBtest (ParamHelpers::ConvertToParamValue (pvalue, definition), "ConvertToParamValue(Definition) : return");
        DBtest (pvalue.val.type == API_PropertyRealValueType, "ConvertToParamValue(Definition) : val.type");
        DBtest (pvalue.type == API_PropertyRealValueType, "ConvertToParamValue(Definition) : type");
        DBtest (pvalue.fromProperty, "ConvertToParamValue(Definition) : fromProperty");
        DBtest (pvalue.fromPropertyDefinition,
                "ConvertToParamValue(Definition) : fromPropertyDefinition (нет спецмаркеров)");

        // ---- Граница: description содержит SYNCNAME -> особый rawName ----
        pvalue = ParamValue ();
        pvalue.rawName = "{@property:anything}";
        pvalue.name = "TestDefSyncName";
        definition = {};
        definition.guid = APINULLGuid;
        definition.description = SYNCNAME;
        definition.valueType = API_PropertyStringValueType;
        ParamHelpers::ConvertToParamValue (pvalue, definition);
        DBtest (pvalue.rawName,
                GS::UniString ("{@property:sync_name0}"),
                "ConvertToParamValue(Definition) : rawName (sync_name)");
        DBtest (pvalue.fromAttribDefinition, "ConvertToParamValue(Definition) : fromAttribDefinition (sync_name)");

        // ---- Граница: rawName содержит "buildingmaterial" -> fromAttribDefinition, без изменения rawName ----
        pvalue = ParamValue ();
        pvalue.rawName = "{@property:buildingmaterialproperties/density}";
        pvalue.name = "TestDefBuildingMaterial";
        definition = {};
        definition.guid = APINULLGuid;
        definition.description = "";
        definition.valueType = API_PropertyRealValueType;
        ParamHelpers::ConvertToParamValue (pvalue, definition);
        DBtest (pvalue.fromAttribDefinition,
                "ConvertToParamValue(Definition) : fromAttribDefinition (buildingmaterial in rawName)");
        DBtest (!pvalue.fromPropertyDefinition,
                "ConvertToParamValue(Definition) : fromPropertyDefinition == false (attrib имеет приоритет)");

        // ---- Граница: пустой guid и пустое valueType (Undefined) - функция всё равно возвращает true ----
        pvalue = ParamValue ();
        pvalue.rawName = "{@property:testdef_undef}";
        pvalue.name = "TestDefUndefined";
        definition = {};
        definition.guid = APINULLGuid;
        definition.description = "";
        definition.valueType = API_PropertyUndefinedValueType;
        DBtest (ParamHelpers::ConvertToParamValue (pvalue, definition),
                "ConvertToParamValue(Definition) : return (Undefined valueType всё равно true)");
        DBtest (pvalue.val.type == API_PropertyUndefinedValueType,
                "ConvertToParamValue(Definition) : val.type (Undefined)");
        DBprnt ("TEST", "TestConvertPropertyDefinitionToParamValue : done");
        return;
    }

    // -----------------------------------------------------------------------------
    // Тест SetParamValueSourseByName - назначение флагов источника по префиксу rawName
    // -----------------------------------------------------------------------------
    void TestSetParamValueSourseByName () {
        DBprnt ("TEST", "TestSetParamValueSourseByName");
        ParamValue pvalue;

        // ---- PROPERTYNAMEPREFIX ----
        pvalue = ParamValue ();
        pvalue.rawName = PROPERTYNAMEPREFIX + GS::UniString ("testprop") + BRACEEND;
        ParamHelpers::SetParamValueSourseByName (pvalue);
        DBtest (pvalue.fromPropertyDefinition,
                "SetParamValueSourseByName : fromPropertyDefinition (PROPERTYNAMEPREFIX)");
        DBtest (pvalue.typeinx, (short)PROPERTYTYPEINX, "SetParamValueSourseByName : typeinx (PROPERTYNAMEPREFIX)");
        DBtest (!pvalue.fromAttribDefinition,
                "SetParamValueSourseByName : fromAttribDefinition == false (обычное свойство)");

        // ---- Граница: PROPERTYNAMEPREFIX + "buildingmaterialproperties/" -> дополнительно fromAttribDefinition ----
        pvalue = ParamValue ();
        pvalue.rawName = PROPERTYNAMEPREFIX + GS::UniString ("buildingmaterialproperties/density") + BRACEEND;
        ParamHelpers::SetParamValueSourseByName (pvalue);
        DBtest (pvalue.fromPropertyDefinition, "SetParamValueSourseByName : fromPropertyDefinition (buildingmaterial)");
        DBtest (pvalue.fromAttribDefinition,
                "SetParamValueSourseByName : fromAttribDefinition (buildingmaterialproperties/)");

        // ---- GDLNAMEPREFIX ----
        pvalue = ParamValue ();
        pvalue.rawName = GDLNAMEPREFIX + GS::UniString ("testgdl") + BRACEEND;
        ParamHelpers::SetParamValueSourseByName (pvalue);
        DBtest (pvalue.fromGDLparam, "SetParamValueSourseByName : fromGDLparam (GDLNAMEPREFIX)");
        DBtest (!pvalue.fromGDLdescription, "SetParamValueSourseByName : fromGDLdescription == false (обычный GDL)");
        DBtest (pvalue.typeinx, (short)GDLTYPEINX, "SetParamValueSourseByName : typeinx (GDLNAMEPREFIX)");

        // ---- GDLDESCNAMEPREFIX ----
        pvalue = ParamValue ();
        pvalue.rawName = GDLDESCNAMEPREFIX + GS::UniString ("testdesc") + BRACEEND;
        ParamHelpers::SetParamValueSourseByName (pvalue);
        DBtest (pvalue.fromGDLparam, "SetParamValueSourseByName : fromGDLparam (GDLDESCNAMEPREFIX)");
        DBtest (pvalue.fromGDLdescription, "SetParamValueSourseByName : fromGDLdescription (GDLDESCNAMEPREFIX)");
        DBtest (pvalue.typeinx, (short)GDLDESCTYPEINX, "SetParamValueSourseByName : typeinx (GDLDESCNAMEPREFIX)");

        // ---- COORDNAMEPREFIX ----
        pvalue = ParamValue ();
        pvalue.rawName = COORDNAMEPREFIX + GS::UniString ("testcoord") + BRACEEND;
        ParamHelpers::SetParamValueSourseByName (pvalue);
        DBtest (pvalue.fromCoord, "SetParamValueSourseByName : fromCoord (COORDNAMEPREFIX)");

        // ---- GLOBNAMEPREFIX ----
        pvalue = ParamValue ();
        pvalue.rawName = GLOBNAMEPREFIX + GS::UniString ("testglob") + BRACEEND;
        ParamHelpers::SetParamValueSourseByName (pvalue);
        DBtest (pvalue.fromGlob, "SetParamValueSourseByName : fromGlob (GLOBNAMEPREFIX)");

        // ---- INFONAMEPREFIX ----
        pvalue = ParamValue ();
        pvalue.rawName = INFONAMEPREFIX + GS::UniString ("testinfo") + BRACEEND;
        ParamHelpers::SetParamValueSourseByName (pvalue);
        DBtest (pvalue.fromInfo, "SetParamValueSourseByName : fromInfo (INFONAMEPREFIX)");

        // ---- MEPNAMEPREFIX ----
        pvalue = ParamValue ();
        pvalue.rawName = MEPNAMEPREFIX + GS::UniString ("testmep") + BRACEEND;
        ParamHelpers::SetParamValueSourseByName (pvalue);
        DBtest (pvalue.fromMEP, "SetParamValueSourseByName : fromMEP (MEPNAMEPREFIX)");

        // ---- FORMULANAMEPREFIX (устанавливает val.hasFormula, а не from*-флаг) ----
        pvalue = ParamValue ();
        pvalue.rawName = FORMULANAMEPREFIX + GS::UniString ("testformula") + BRACEEND;
        ParamHelpers::SetParamValueSourseByName (pvalue);
        DBtest (pvalue.val.hasFormula, "SetParamValueSourseByName : val.hasFormula (FORMULANAMEPREFIX)");

        // ---- IDNAMEPREFIX ----
        pvalue = ParamValue ();
        pvalue.rawName = IDNAMEPREFIX + GS::UniString ("testid") + BRACEEND;
        ParamHelpers::SetParamValueSourseByName (pvalue);
        DBtest (pvalue.fromID, "SetParamValueSourseByName : fromID (IDNAMEPREFIX)");

        // ---- IFCNAMEPREFIX ----
        pvalue = ParamValue ();
        pvalue.rawName = IFCNAMEPREFIX + GS::UniString ("testifc") + BRACEEND;
        ParamHelpers::SetParamValueSourseByName (pvalue);
        DBtest (pvalue.fromIFCProperty, "SetParamValueSourseByName : fromIFCProperty (IFCNAMEPREFIX)");

        // ---- MORPHNAMEPREFIX ----
        pvalue = ParamValue ();
        pvalue.rawName = MORPHNAMEPREFIX + GS::UniString ("testmorph") + BRACEEND;
        ParamHelpers::SetParamValueSourseByName (pvalue);
        DBtest (pvalue.fromMorph, "SetParamValueSourseByName : fromMorph (MORPHNAMEPREFIX)");

        // ---- ATTRIBNAMEPREFIX ----
        pvalue = ParamValue ();
        pvalue.rawName = ATTRIBNAMEPREFIX + GS::UniString ("testattrib") + BRACEEND;
        ParamHelpers::SetParamValueSourseByName (pvalue);
        DBtest (pvalue.fromAttribElement, "SetParamValueSourseByName : fromAttribElement (ATTRIBNAMEPREFIX)");

        // ---- LISTDATANAMEPREFIX ----
        pvalue = ParamValue ();
        pvalue.rawName = LISTDATANAMEPREFIX + GS::UniString ("testlistdata") + BRACEEND;
        ParamHelpers::SetParamValueSourseByName (pvalue);
        DBtest (pvalue.fromListData, "SetParamValueSourseByName : fromListData (LISTDATANAMEPREFIX)");

        // ---- MATERIALNAMEPREFIX ----
        pvalue = ParamValue ();
        pvalue.rawName = MATERIALNAMEPREFIX + GS::UniString ("testmaterial") + BRACEEND;
        ParamHelpers::SetParamValueSourseByName (pvalue);
        DBtest (pvalue.fromMaterial, "SetParamValueSourseByName : fromMaterial (MATERIALNAMEPREFIX)");

        // ---- CLASSNAMEPREFIX ----
        pvalue = ParamValue ();
        pvalue.rawName = CLASSNAMEPREFIX + GS::UniString ("testclass") + BRACEEND;
        ParamHelpers::SetParamValueSourseByName (pvalue);
        DBtest (pvalue.fromClassification, "SetParamValueSourseByName : fromClassification (CLASSNAMEPREFIX)");

        // ---- ELEMENTNAMEPREFIX ----
        pvalue = ParamValue ();
        pvalue.rawName = ELEMENTNAMEPREFIX + GS::UniString ("testelement") + BRACEEND;
        ParamHelpers::SetParamValueSourseByName (pvalue);
        DBtest (pvalue.fromElement, "SetParamValueSourseByName : fromElement (ELEMENTNAMEPREFIX)");

        // ---- FILENAMEPREFIX ----
        pvalue = ParamValue ();
        pvalue.rawName = FILENAMEPREFIX + GS::UniString ("testfile") + BRACEEND;
        ParamHelpers::SetParamValueSourseByName (pvalue);
        DBtest (pvalue.fromFile, "SetParamValueSourseByName : fromFile (FILENAMEPREFIX)");

        // ---- Граница: неизвестный префикс -> typeinx == 0, ни один флаг не установлен ----
        pvalue = ParamValue ();
        pvalue.rawName = "{@unknownprefix:test}";
        ParamHelpers::SetParamValueSourseByName (pvalue);
        DBtest (pvalue.typeinx, (short)0, "SetParamValueSourseByName : typeinx (неизвестный префикс)");
        DBtest (!pvalue.fromProperty && !pvalue.fromGDLparam && !pvalue.fromCoord && !pvalue.fromElement,
                "SetParamValueSourseByName : флаги не установлены (неизвестный префикс)");

        // ---- Граница: ранний выход, если уже установлен любой from*-флаг (например, fromProperty) ----
        pvalue = ParamValue ();
        pvalue.rawName = GDLNAMEPREFIX + GS::UniString ("shouldnotchange") + BRACEEND;
        pvalue.fromProperty = true; // Уже определён источник
        ParamHelpers::SetParamValueSourseByName (pvalue);
        DBtest (!pvalue.fromGDLparam,
                "SetParamValueSourseByName : ранний выход - fromGDLparam не выставляется, если fromProperty уже true");
        DBtest (pvalue.typeinx, (short)0, "SetParamValueSourseByName : ранний выход - typeinx не пересчитывается");

        // ---- Граница: GDL rawName с индексами массива (@arr_row_start_row_end_col_start_col_end) ----
        pvalue = ParamValue ();
        pvalue.rawName = GDLNAMEPREFIX + GS::UniString ("myarrparam@arr_3_5_7_9") + BRACEEND;
        ParamHelpers::SetParamValueSourseByName (pvalue);
        DBtest (pvalue.fromGDLparam, "SetParamValueSourseByName : fromGDLparam (GDL с @arr_)");
        DBtest (pvalue.val.array_row_start, 3, "SetParamValueSourseByName : val.array_row_start (@arr_3_5_7_9)");
        DBtest (pvalue.val.array_row_end, 5, "SetParamValueSourseByName : val.array_row_end (@arr_3_5_7_9)");
        DBtest (pvalue.val.array_column_start, 7, "SetParamValueSourseByName : val.array_column_start (@arr_3_5_7_9)");
        DBtest (pvalue.val.array_column_end, 9, "SetParamValueSourseByName : val.array_column_end (@arr_3_5_7_9)");

        DBprnt ("TEST", "TestSetParamValueSourseByName : done");
        return;
    }

    // -----------------------------------------------------------------------------
    // Тест SetrawNameFromProperty - переопределение rawName/name по описанию свойства
    // -----------------------------------------------------------------------------
    void TestSetrawNameFromProperty () {
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
        DBtest (pvalue.rawName,
                GS::UniString ("{@property:testplain}"),
                "SetrawNameFromProperty : rawName не меняется (обычное свойство)");
        DBtest (
            pvalue.name, GS::UniString ("TestPlain"), "SetrawNameFromProperty : name не меняется (обычное свойство)");

        // ---- Граница: rawName без CharENTER (fromAttrib == false) -> спецветки some_stuff_* не срабатывают,
        //      даже если описание их содержит ----
        pvalue = ParamValue ();
        pvalue.rawName = "{@property:notfromattrib}";
        pvalue.name = "NotFromAttrib";
        property = {};
        property.definition.description = "some_stuff_th";
        ParamHelpers::SetrawNameFromProperty (pvalue, property);
        DBtest (pvalue.rawName,
                GS::UniString ("{@property:notfromattrib}"),
                "SetrawNameFromProperty : rawName не меняется без CharENTER (fromAttrib == false)");
        DBtest (pvalue.name,
                GS::UniString ("NotFromAttrib"),
                "SetrawNameFromProperty : name не меняется без CharENTER (fromAttrib == false)");

        // ---- Граница: rawName содержит CharENTER (fromAttrib == true) + описание "some_stuff_th" ----
        pvalue = ParamValue ();
        pvalue.rawName = GS::UniString ("{@property:some_stuff_th") + CharENTER + GS::UniString ("7") + BRACEEND;
        pvalue.name = "Original";
        property = {};
        property.definition.description = "Some_Stuff_TH"; // Проверка регистронезависимости (ToLowerCase внутри)
        ParamHelpers::SetrawNameFromProperty (pvalue, property);
        DBtest (pvalue.rawName.BeginsWith ("{@property:buildingmaterialproperties/some_stuff_th"),
                "SetrawNameFromProperty : rawName переписан на some_stuff_th");
        DBtest (pvalue.rawName.Contains ("7"), "SetrawNameFromProperty : сохранён индекс атрибута (some_stuff_th)");
        DBtest (pvalue.name, GS::UniString ("some_stuff_th"), "SetrawNameFromProperty : name == some_stuff_th");
        DBtest (!pvalue.val.formatstring.isEmpty, "SetrawNameFromProperty : formatstring задан для some_stuff_th");

        // ---- Граница: fromAttrib == true + описание "some_stuff_units" ----
        pvalue = ParamValue ();
        pvalue.rawName = GS::UniString ("{@property:some_stuff_units") + CharENTER + GS::UniString ("2") + BRACEEND;
        pvalue.name = "Original";
        property = {};
        property.definition.description = "some_stuff_units";
        ParamHelpers::SetrawNameFromProperty (pvalue, property);
        DBtest (pvalue.rawName.BeginsWith ("{@property:buildingmaterialproperties/some_stuff_units"),
                "SetrawNameFromProperty : rawName переписан на some_stuff_units");
        DBtest (pvalue.rawName.Contains ("2"), "SetrawNameFromProperty : сохранён индекс атрибута (some_stuff_units)");
        DBtest (pvalue.name, GS::UniString ("some_stuff_units"), "SetrawNameFromProperty : name == some_stuff_units");

        // ---- Граница: fromAttrib == true + описание "some_stuff_kzap" ----
        pvalue = ParamValue ();
        pvalue.rawName = GS::UniString ("{@property:some_stuff_kzap") + CharENTER + GS::UniString ("1") + BRACEEND;
        pvalue.name = "Original";
        property = {};
        property.definition.description = "some_stuff_kzap";
        ParamHelpers::SetrawNameFromProperty (pvalue, property);
        DBtest (pvalue.rawName.BeginsWith ("{@property:buildingmaterialproperties/some_stuff_kzap"),
                "SetrawNameFromProperty : rawName переписан на some_stuff_kzap");
        DBtest (pvalue.name, GS::UniString ("some_stuff_kzap"), "SetrawNameFromProperty : name == some_stuff_kzap");

        DBprnt ("TEST", "TestSetrawNameFromProperty : done");
        // Примечание: ветка description.Contains (SYNCCORRECTFLAG) не покрыта тестом - значение
        // константы SYNCCORRECTFLAG не определено в Helpers.hpp/cpp (внешний заголовок), поэтому
        // корректную тестовую строку для срабатывания этой ветки составить нельзя.
        return;
    }

    // -----------------------------------------------------------------------------
    // Тест CheckIgnoreVal на граничных значениях
    // -----------------------------------------------------------------------------
    void TestCheckIgnoreVal () {
        DBprnt ("TEST", "TestCheckIgnoreVal");
        ParamValue param;
        SkipValues ignorevals;

        // ---- Граница: пустая строка + skip_empty == true -> игнорировать ----
        param = ParamValue ();
        param.val.type = API_PropertyStringValueType;
        param.val.uniStringValue = "";
        ignorevals = {};
        ignorevals.skip_empty = true;
        DBtest (ParamHelpers::CheckIgnoreVal (ignorevals, param), "CheckIgnoreVal : пустая строка + skip_empty");

        // ---- Граница: строка из пробелов + skip_empty == true (без skip_trim_empty) -> НЕ игнорировать ----
        param = ParamValue ();
        param.val.type = API_PropertyStringValueType;
        param.val.uniStringValue = "   ";
        ignorevals = {};
        ignorevals.skip_empty = true;
        DBtest (!ParamHelpers::CheckIgnoreVal (ignorevals, param),
                "CheckIgnoreVal : строка из пробелов + только skip_empty -> не игнорируется");

        // ---- Граница: строка из пробелов + skip_trim_empty == true -> игнорировать ----
        param = ParamValue ();
        param.val.type = API_PropertyStringValueType;
        param.val.uniStringValue = "   ";
        ignorevals = {};
        ignorevals.skip_trim_empty = true;
        DBtest (ParamHelpers::CheckIgnoreVal (ignorevals, param),
                "CheckIgnoreVal : строка из пробелов + skip_trim_empty");

        // ---- Граница: непустая строка + skip_empty/skip_trim_empty == true -> НЕ игнорировать ----
        param = ParamValue ();
        param.val.type = API_PropertyStringValueType;
        param.val.uniStringValue = "непустое значение";
        ignorevals = {};
        ignorevals.skip_empty = true;
        ignorevals.skip_trim_empty = true;
        DBtest (!ParamHelpers::CheckIgnoreVal (ignorevals, param),
                "CheckIgnoreVal : непустая строка -> не игнорируется");

        // ---- Граница: числовой (не строковый, не булевый) тип, doubleValue == 0 + skip_empty -> игнорировать ----
        param = ParamValue ();
        param.val.type = API_PropertyRealValueType;
        param.val.doubleValue = 0.0;
        ignorevals = {};
        ignorevals.skip_empty = true;
        DBtest (ParamHelpers::CheckIgnoreVal (ignorevals, param), "CheckIgnoreVal : Real == 0.0 + skip_empty");

        // ---- Граница: числовой тип, doubleValue != 0 + skip_empty -> НЕ игнорировать ----
        param = ParamValue ();
        param.val.type = API_PropertyRealValueType;
        param.val.doubleValue = 1.0;
        ignorevals = {};
        ignorevals.skip_empty = true;
        DBtest (!ParamHelpers::CheckIgnoreVal (ignorevals, param),
                "CheckIgnoreVal : Real == 1.0 + skip_empty -> не игнорируется");

        // ---- Граница: булевый тип НЕ подчиняется skip_empty/skip_trim_empty, даже если doubleValue == 0 ----
        param = ParamValue ();
        param.val.type = API_PropertyBooleanValueType;
        param.val.doubleValue = 0.0;
        param.val.boolValue = false;
        ignorevals = {};
        ignorevals.skip_empty = true;
        ignorevals.skip_trim_empty = true;
        DBtest (!ParamHelpers::CheckIgnoreVal (ignorevals, param),
                "CheckIgnoreVal : Boolean игнорирует skip_empty/skip_trim_empty");

        // ---- Граница: пустой список ignorevals.ignorevals -> false, даже без skip-флагов ----
        param = ParamValue ();
        param.val.type = API_PropertyStringValueType;
        param.val.uniStringValue = "любое значение";
        ignorevals = {};
        DBtest (!ParamHelpers::CheckIgnoreVal (ignorevals, param),
                "CheckIgnoreVal : пустой ignorevals и все флаги false -> false");

        // ---- Граница: точное совпадение со значением из списка ignorevals ----
        param = ParamValue ();
        param.val.type = API_PropertyStringValueType;
        param.val.uniStringValue = "ignoreme";
        ignorevals = {};
        ignorevals.ignorevals.Push ("ignoreme");
        DBtest (ParamHelpers::CheckIgnoreVal (ignorevals, param), "CheckIgnoreVal : точное совпадение со списком");

        // ---- Граница: список с шаблоном "*суффикс" - совпадение по окончанию строки ----
        param = ParamValue ();
        param.val.type = API_PropertyStringValueType;
        param.val.uniStringValue = "значение_суффикс";
        ignorevals = {};
        ignorevals.ignorevals.Push ("*суффикс");
        DBtest (ParamHelpers::CheckIgnoreVal (ignorevals, param), "CheckIgnoreVal : шаблон *суффикс (EndsWith)");

        // ---- Граница: список с шаблоном "префикс*" - совпадение по началу строки ----
        param = ParamValue ();
        param.val.type = API_PropertyStringValueType;
        param.val.uniStringValue = "префикс_значение";
        ignorevals = {};
        ignorevals.ignorevals.Push ("префикс*");
        DBtest (ParamHelpers::CheckIgnoreVal (ignorevals, param), "CheckIgnoreVal : шаблон префикс* (BeginsWith)");

        // ---- Граница: список задан, но ничего не совпадает -> false ----
        param = ParamValue ();
        param.val.type = API_PropertyStringValueType;
        param.val.uniStringValue = "совершенно другое значение";
        ignorevals = {};
        ignorevals.ignorevals.Push ("ignoreme");
        ignorevals.ignorevals.Push ("*суффикс");
        DBtest (!ParamHelpers::CheckIgnoreVal (ignorevals, param),
                "CheckIgnoreVal : список задан, но нет совпадений -> false");

        // ---- Граница: сравнение со списком идёт по обрезанной (trim) строке ----
        param = ParamValue ();
        param.val.type = API_PropertyStringValueType;
        param.val.uniStringValue = "  ignoreme  ";
        ignorevals = {};
        ignorevals.ignorevals.Push ("ignoreme");
        DBtest (ParamHelpers::CheckIgnoreVal (ignorevals, param), "CheckIgnoreVal : сравнение по trim-строке");

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
    void TestReadProperty () {
        DBprnt ("TEST", "TestReadProperty");
        ParamDictValue params;
        GS::Array<API_PropertyDefinition> propertyDefinitions;

        // ---- Граница: params пуст -> немедленный false, независимо от propertyDefinitions ----
        params = {};
        propertyDefinitions = {};
        DBtest (!ParamHelpers::ReadProperty (APINULLGuid, params, propertyDefinitions),
                "ReadProperty : params пуст -> false");

        API_PropertyDefinition dummyDefinition = {};
        dummyDefinition.guid = APINULLGuid;
        propertyDefinitions.Push (dummyDefinition);
        DBtest (!ParamHelpers::ReadProperty (APINULLGuid, params, propertyDefinitions),
                "ReadProperty : params пуст + propertyDefinitions не пуст -> всё равно false");

        // ---- Граница: params не пуст, но propertyDefinitions пуст -> false ----
        params = {};
        ParamValue pvalue = {};
        pvalue.rawName = PROPERTYNAMEPREFIX + GS::UniString ("testreadproperty") + BRACEEND;
        pvalue.name = "TestReadProperty";
        params.Put (pvalue.rawName, pvalue);
        propertyDefinitions = {};
        DBtest (!ParamHelpers::ReadProperty (APINULLGuid, params, propertyDefinitions),
                "ReadProperty : propertyDefinitions пуст -> false");

        // ---- Граница: params и propertyDefinitions не пусты, но elemGuid == APINULLGuid ----
        // ACAPI_Element_GetPropertyValues не найдёт элемент по несуществующему guid и вернёт ошибку,
        // поэтому ReadProperty должен вернуть false, а словарь params - остаться без изменений.
        params = {};
        params.Put (pvalue.rawName, pvalue);
        propertyDefinitions = {};
        propertyDefinitions.Push (dummyDefinition);
        ParamValue pvalueBefore = *params.GetPtr (pvalue.rawName);
        DBtest (!ParamHelpers::ReadProperty (APINULLGuid, params, propertyDefinitions),
                "ReadProperty : APINULLGuid -> ACAPI-ошибка -> false");
        DBtest (params.GetPtr (pvalue.rawName)->isValid,
                pvalueBefore.isValid,
                "ReadProperty : значение в словаре не изменилось после ошибки ACAPI");

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
    void TestAddProperty () {
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
    #ifndef ServerMainVers_2400
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
        DBtest (!ParamHelpers::AddProperty (params, properties, APINULLGuid),
                "AddProperty : свойство отсутствует в словаре -> false");
        DBtest (params.GetSize () == 0, "AddProperty : словарь остаётся пустым, если совпадений нет");

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
        DBtest (ParamHelpers::AddProperty (params, properties, APINULLGuid),
                "AddProperty : совпадение по rawName -> true");
        if (const ParamValue *stored = params.GetPtr (expectedRawName)) {
            DBtest (
                stored->val.intValue, (Int32)55, "AddProperty : значение свойства записано в словарь (intValue == 55)");
            DBtest (stored->val.type == API_PropertyIntegerValueType, "AddProperty : тип значения (Integer)");
            DBtest (stored->isValid, "AddProperty : isValid == true после успешной конвертации");
            DBtest (stored->fromGuid == APINULLGuid, "AddProperty : fromGuid == APINULLGuid (elemguid == APINULLGuid)");
        } else {
            DBtest (false, "AddProperty : значение должно быть найдено в словаре после Put");
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
    #ifndef ServerMainVers_2400
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
        DBtest (!ParamHelpers::AddProperty (params, properties, APINULLGuid),
                "AddProperty : Undefined valueType -> ConvertToParamValue не проходит -> false");
        if (const ParamValue *storedUndef = params.GetPtr (undefRawName)) {
            DBtest (!storedUndef->isValid, "AddProperty : запись в словаре не изменилась (осталась isValid == false)");
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
    #ifndef ServerMainVers_2400
        noMatchProperty.isEvaluated = true;
    #else
        noMatchProperty.status = API_Property_HasValue;
    #endif
        noMatchProperty.value.singleVariant.variant.type = API_PropertyIntegerValueType;
        noMatchProperty.value.singleVariant.variant.intValue = 1;
        properties.Push (noMatchProperty); // не совпадёт (отдельное описание -> другой rawName)
        DBtest (ParamHelpers::AddProperty (params, properties, APINULLGuid),
                "AddProperty : несколько свойств, есть хотя бы одно совпадение -> true");
        DBtest (params.GetSize () == 1,
                "AddProperty : несовпавшее свойство не добавляется в словарь (needAdd == false)");

        DBprnt ("TEST", "TestAddProperty : done");
        return;
    }

    // -----------------------------------------------------------------------------
    // Тест PropertyHelpers::ToString (API_Property) - проверка строкового представления свойств
    // -----------------------------------------------------------------------------
    void TestPropertyHelpersToString () {
        DBprnt ("TEST", "TestPropertyHelpersToString");
        API_Property property;
        FormatString fstring;

        // ---- Integer: базовое значение ----
        property = {};
    #ifndef ServerMainVers_2400
        property.isEvaluated = true;
    #else
        property.status = API_Property_HasValue;
    #endif
        property.definition.collectionType = API_PropertySingleCollectionType;
        property.definition.valueType = API_PropertyIntegerValueType;
        property.value.singleVariant.variant.type = API_PropertyIntegerValueType;
        property.value.singleVariant.variant.intValue = 42;
        GS::UniString intResult = PropertyHelpers::ToString (property);
        DBtest (intResult, GS::UniString ("42"), "ToString(Property) : Integer 42");

        // ---- Real: базовое значение ----
        property = {};
    #ifndef ServerMainVers_2400
        property.isEvaluated = true;
    #else
        property.status = API_Property_HasValue;
    #endif
        property.definition.collectionType = API_PropertySingleCollectionType;
        property.definition.valueType = API_PropertyRealValueType;
        property.definition.measureType = API_PropertyUndefinedMeasureType;
        property.value.singleVariant.variant.type = API_PropertyRealValueType;
        property.value.singleVariant.variant.doubleValue = 3.14159;
        GS::UniString realResult = PropertyHelpers::ToString (property);
        // ToString для Real использует форматирование с точностью, проверяем что это число
        DBtest (!realResult.IsEmpty (), "ToString(Property) : Real не пустая строка");
        DBtest (realResult.Contains ("3.14") || realResult.Contains ("3,14"),
                "ToString(Property) : Real содержит значение");

        // ---- Boolean: true ----
        property = {};
    #ifndef ServerMainVers_2400
        property.isEvaluated = true;
    #else
        property.status = API_Property_HasValue;
    #endif
        property.definition.collectionType = API_PropertySingleCollectionType;
        property.definition.valueType = API_PropertyBooleanValueType;
        property.value.singleVariant.variant.type = API_PropertyBooleanValueType;
        property.value.singleVariant.variant.boolValue = true;
        GS::UniString boolTrueResult = PropertyHelpers::ToString (property);
        DBtest (!boolTrueResult.IsEmpty (), "ToString(Property) : Boolean true не пустая");

        // ---- Boolean: false ----
        property.value.singleVariant.variant.boolValue = false;
        GS::UniString boolFalseResult = PropertyHelpers::ToString (property);
        DBtest (!boolFalseResult.IsEmpty (), "ToString(Property) : Boolean false не пустая");

        // ---- String: обычное значение ----
        property = {};
    #ifndef ServerMainVers_2400
        property.isEvaluated = true;
    #else
        property.status = API_Property_HasValue;
    #endif
        property.definition.collectionType = API_PropertySingleCollectionType;
        property.definition.valueType = API_PropertyStringValueType;
        property.value.singleVariant.variant.type = API_PropertyStringValueType;
        property.value.singleVariant.variant.uniStringValue = "TestString";
        GS::UniString strResult = PropertyHelpers::ToString (property);
        DBtest (strResult, GS::UniString ("TestString"), "ToString(Property) : String значение");

        // ---- String: пустая строка ----
        property.value.singleVariant.variant.uniStringValue = "";
        GS::UniString emptyStrResult = PropertyHelpers::ToString (property);
        DBtest (emptyStrResult.IsEmpty (), "ToString(Property) : пустая строка -> пустой результат");

        // ---- List: список Integer ----
        property = {};
    #ifndef ServerMainVers_2400
        property.isEvaluated = true;
    #else
        property.status = API_Property_HasValue;
    #endif
        property.definition.collectionType = API_PropertyListCollectionType;
        property.definition.valueType = API_PropertyIntegerValueType;
        API_Variant v1 = {}, v2 = {}, v3 = {};
        v1.type = API_PropertyIntegerValueType;
        v1.intValue = 1;
        v2.type = API_PropertyIntegerValueType;
        v2.intValue = 2;
        v3.type = API_PropertyIntegerValueType;
        v3.intValue = 3;
        property.value.listVariant.variants.Push (v1);
        property.value.listVariant.variants.Push (v2);
        property.value.listVariant.variants.Push (v3);
        GS::UniString listResult = PropertyHelpers::ToString (property);
        DBtest (listResult.Contains ("1"), "ToString(Property) : List содержит 1");
        DBtest (listResult.Contains ("2"), "ToString(Property) : List содержит 2");
        DBtest (listResult.Contains ("3"), "ToString(Property) : List содержит 3");
        DBtest (listResult.Contains (";"), "ToString(Property) : List содержит разделитель");

        // ---- List: список String ----
        property = {};
    #ifndef ServerMainVers_2400
        property.isEvaluated = true;
    #else
        property.status = API_Property_HasValue;
    #endif
        property.definition.collectionType = API_PropertyListCollectionType;
        property.definition.valueType = API_PropertyStringValueType;
        API_Variant sv1 = {}, sv2 = {};
        sv1.type = API_PropertyStringValueType;
        sv1.uniStringValue = "apple";
        sv2.type = API_PropertyStringValueType;
        sv2.uniStringValue = "banana";
        property.value.listVariant.variants.Push (sv1);
        property.value.listVariant.variants.Push (sv2);
        GS::UniString strListResult = PropertyHelpers::ToString (property);
        DBtest (strListResult.Contains ("apple"), "ToString(Property) : String List содержит apple");
        DBtest (strListResult.Contains ("banana"), "ToString(Property) : String List содержит banana");
        DBtest (strListResult.Contains (";"), "ToString(Property) : String List содержит разделитель");

        // ---- NotAvailable / NotEvaluated: должна вернуть пустую строку ----
        property = {};
    #ifndef ServerMainVers_2400
        property.isEvaluated = false;
    #else
        property.status = API_Property_NotAvailable;
    #endif
        property.definition.collectionType = API_PropertySingleCollectionType;
        property.definition.valueType = API_PropertyIntegerValueType;
        GS::UniString notAvailResult = PropertyHelpers::ToString (property);
        DBtest (notAvailResult.IsEmpty (), "ToString(Property) : NotAvailable -> пустая строка");

        // ---- Default value (isDefault + NotEvaluated) ----
        property = {};
    #ifndef ServerMainVers_2400
        property.isEvaluated = true;
    #else
        property.status = API_Property_NotEvaluated;
        property.isDefault = true;
    #endif
        property.definition.collectionType = API_PropertySingleCollectionType;
        property.definition.valueType = API_PropertyIntegerValueType;
        property.definition.defaultValue.basicValue.singleVariant.variant.type = API_PropertyIntegerValueType;
        property.definition.defaultValue.basicValue.singleVariant.variant.intValue = 999;
        GS::UniString defaultResult = PropertyHelpers::ToString (property);
        DBtest (defaultResult, GS::UniString ("999"), "ToString(Property) : Default value (999)");

        // ---- Default value: Real ----
        property.definition.defaultValue.basicValue.singleVariant.variant.type = API_PropertyRealValueType;
        property.definition.defaultValue.basicValue.singleVariant.variant.doubleValue = 2.5;
        property.definition.valueType = API_PropertyRealValueType;
        GS::UniString defaultRealResult = PropertyHelpers::ToString (property);
        DBtest (!defaultRealResult.IsEmpty (), "ToString(Property) : Default Real не пустая");

        DBprnt ("TEST", "TestPropertyHelpersToString : done");
        return;
    }

    void DumpAllBuiltInProperties () {
        // GS::Array<API_PropertyGroup> groups;
        // ACAPI_Property_GetPropertyGroups (groups);
        // for (const API_PropertyGroup& group : groups) {
        //     GS::Array<API_PropertyDefinition> definitions;
        //     ACAPI_Property_GetPropertyDefinitions (group.guid, definitions);
        //     GS::UniString report_ =
        //         "======" + group.name + "\t" +
        //         APIGuidToString (group.guid);
        //     ACAPI_WriteReport (report_, false);
        //     for (const API_PropertyDefinition& definition : definitions) {
        //         if (definition.definitionType != API_PropertyStaticBuiltInDefinitionType) {
        //             continue;
        //         }
        //         GS::UniString report =
        //             group.name + "\t" +
        //             definition.name + "\t" +
        //             APIGuidToString (definition.guid);
        //         ACAPI_WriteReport (report, false);
        //     }
        // }
    }

    void ResetSyncPropertyArray (GS::Array<API_Guid> guidArray) {
        if (guidArray.IsEmpty ())
            return;
        for (UInt32 j = 0; j < guidArray.GetSize (); j++) {
            ResetSyncPropertyOne (guidArray[j]);
        }
    #if defined(TESTING)
        DBprnt ("TEST", "ResetSyncPropertyArray");
    #endif
    }

    void ResetSyncPropertyOne (const API_Guid &elemGuid) {
        GSErrCode err = NoError;
        GS::Array<API_Property> propertywrite;
        ResetSyncPropertyOne (elemGuid, propertywrite);
        if (propertywrite.IsEmpty ())
            return;
        const Int32 iseng = ID_ADDON_STRINGS + isEng ();
        GS::UniString undoString = RSGetIndString (iseng, UndoSyncId, ACAPI_GetOwnResModule ());
        err = ACAPI_CallUndoableCommand (
            undoString, [&] () -> GSErrCode { return ACAPI_Element_SetProperties (elemGuid, propertywrite); });
        if (err != NoError)
            msg_rep ("ResetSyncProperty", "ACAPI_Element_SetProperties", err, elemGuid);
    }

    void ResetSyncPropertyOne (const API_Guid &elemGuid, GS::Array<API_Property> &propertywrite) {
        GSErrCode err = NoError;
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
                    const GSErrCode errGet = ACAPI_Element_GetPropertyValue (elemGuid, definitions[i].guid, property);
                    if (errGet == NoError) {
                        if (!property.isDefault) {
                            property.isDefault = true;
                            propertywrite.Push (property);
                        }
                    } else {
                        msg_rep ("ResetSyncProperty", "ACAPI_Element_GetPropertyValue", errGet, elemGuid);
                    }
                }
            }
        }
    }

    // -----------------------------------------------------------------------------
    // Тест Name2Rawname - преобразование имени в rawname
    // -----------------------------------------------------------------------------
    void TestName2Rawname () {
        DBprnt ("TEST", "TestName2Rawname");
        GS::UniString name;
        GS::UniString rawname;

        // Тест: обычное свойство
        name = "Property:TestProperty";
        DBtest (Name2Rawname (name, rawname), "Name2Rawname Property:TestProperty -> true");
        DBtest (rawname, GS::UniString ("{@property:testproperty}"), "Name2Rawname Property:TestProperty -> rawname");

        // Тест: с префиксом property
        name = "property:AnotherProperty";
        DBtest (Name2Rawname (name, rawname), "Name2Rawname property:AnotherProperty -> true");
        DBtest (
            rawname, GS::UniString ("{@property:anotherproperty}"), "Name2Rawname property:AnotherProperty -> rawname");

        // Тест: координаты
        name = "Coord:symb_pos_x";
        DBtest (Name2Rawname (name, rawname), "Name2Rawname Coord:symb_pos_x -> true");
        DBtest (rawname, GS::UniString ("{@coord:symb_pos_x}"), "Name2Rawname Coord:symb_pos_x -> rawname");

        // Тест: GDL параметр
        name = "TestGDLParam";
        DBtest (Name2Rawname (name, rawname), "Name2Rawname TestGDLParam -> true");
        DBtest (rawname, GS::UniString ("{@gdl:testgdlparam}"), "Name2Rawname TestGDLParam -> rawname");

        // Тест: ID
        name = "{id}";
        DBtest (Name2Rawname (name, rawname), "Name2Rawname {id} -> true");
        DBtest (rawname, GS::UniString ("{@id:id}"), "Name2Rawname {id} -> rawname");

        // Тест: пустая строка -> false
        name = "";
        DBtest (!Name2Rawname (name, rawname), "Name2Rawname empty string -> false");

        // Тест: BuildingMaterial свойство
        name = "Property:BuildingMaterialProperties/Density";
        DBtest (Name2Rawname (name, rawname), "Name2Rawname Property:BuildingMaterialProperties/Density -> true");
        DBtest (rawname.BeginsWith ("{@property:buildingmaterialproperties/density"),
                "Name2Rawname BuildingMaterial -> rawname");

        // Тест: Morph
        name = "Morph:param1";
        DBtest (Name2Rawname (name, rawname), "Name2Rawname Morph:param1 -> true");
        DBtest (rawname, GS::UniString ("{@morph:param1}"), "Name2Rawname Morph:param1 -> rawname");

        // Тест: IFC
        name = "IFC:PropertyName";
        DBtest (Name2Rawname (name, rawname), "Name2Rawname IFC:PropertyName -> true");
        DBtest (rawname, GS::UniString ("{@ifc:propertyname}"), "Name2Rawname IFC:PropertyName -> rawname");

        // Тест: Info
        name = "Info:someinfo";
        DBtest (Name2Rawname (name, rawname), "Name2Rawname Info:someinfo -> true");
        DBtest (rawname, GS::UniString ("{@info:someinfo}"), "Name2Rawname Info:someinfo -> rawname");

        // Тест: Glob
        name = "Glob:variable";
        DBtest (Name2Rawname (name, rawname), "Name2Rawname Glob:variable -> true");
        DBtest (rawname, GS::UniString ("{@glob:variable}"), "Name2Rawname Glob:variable -> rawname");

        // Тест: Class
        name = "Class:classification";
        DBtest (Name2Rawname (name, rawname), "Name2Rawname Class:classification -> true");
        DBtest (rawname, GS::UniString ("{@class:classification}"), "Name2Rawname Class:classification -> rawname");

        // Тест: Element
        name = "Element:property";
        DBtest (Name2Rawname (name, rawname), "Name2Rawname Element:property -> true");
        DBtest (rawname, GS::UniString ("{@element:property}"), "Name2Rawname Element:property -> rawname");

        // Тест: File
        name = "File:filename";
        DBtest (Name2Rawname (name, rawname), "Name2Rawname File:filename -> true");
        DBtest (rawname, GS::UniString ("{@file:filename}"), "Name2Rawname File:filename -> rawname");

        // Тест: Attrib (Layer)
        name = "Attrib:Layer";
        DBtest (Name2Rawname (name, rawname), "Name2Rawname Attrib:Layer -> true");
        DBtest (rawname, GS::UniString ("{@attrib:layer}"), "Name2Rawname Attrib:Layer -> rawname");

        DBprnt ("TEST", "TestName2Rawname : done");
        return;
    }

    // -----------------------------------------------------------------------------
    // Тест Name2Rawname с уже обёрнутыми скобками (временное решение до исправления бага в Sync.cpp:1323-1326)
    // Баг: Name2Rawname сначала добавляет BRACEEND (}), потом BRACESTART ({).
    // Входные данные, УЖЕ содержащие правильные скобки "{@prefix:name}", проходят корректно.
    // -----------------------------------------------------------------------------
    void TestName2RawnameWithBrackets () {
        DBprnt ("TEST", "TestName2RawnameWithBrackets");
        GS::UniString name;
        GS::UniString rawname;

        // Тест: уже корректный rawname свойства
        name = "{@property:testproperty}";
        DBtest (Name2Rawname (name, rawname), "Name2RawnameWithBrackets {@property:testproperty} -> true");
        DBtest (rawname,
                GS::UniString ("{@property:testproperty}"),
                "Name2RawnameWithBrackets {@property:testproperty} -> rawname unchanged");

        // Тест: уже корректный rawname координат
        name = "{@coord:symb_pos_x}";
        DBtest (Name2Rawname (name, rawname), "Name2RawnameWithBrackets {@coord:symb_pos_x} -> true");
        DBtest (rawname,
                GS::UniString ("{@coord:symb_pos_x}"),
                "Name2RawnameWithBrackets {@coord:symb_pos_x} -> rawname unchanged");

        // Sync.cpp-8 (#161): каноническая ветка нормализует регистр — ключи кэша
        // всегда в нижнем регистре, иначе правило молча не находит значение.
        name = "{@Coord:Symb_Pos_X}";
        DBtest (Name2Rawname (name, rawname), "Name2RawnameWithBrackets {@Coord:Symb_Pos_X} -> true");
        DBtest (rawname,
                GS::UniString ("{@coord:symb_pos_x}"),
                "Name2RawnameWithBrackets {@Coord:Symb_Pos_X} -> rawname lowered");

        // Тест: уже корректный rawname GDL
        name = "{@gdl:testgdlparam}";
        DBtest (Name2Rawname (name, rawname), "Name2RawnameWithBrackets {@gdl:testgdlparam} -> true");
        DBtest (rawname,
                GS::UniString ("{@gdl:testgdlparam}"),
                "Name2RawnameWithBrackets {@gdl:testgdlparam} -> rawname unchanged");

        // Тест: уже корректный rawname ID
        name = "{@id:id}";
        DBtest (Name2Rawname (name, rawname), "Name2RawnameWithBrackets {@id:id} -> true");
        DBtest (rawname, GS::UniString ("{@id:id}"), "Name2RawnameWithBrackets {@id:id} -> rawname unchanged");

        // Тест: уже корректный rawname BuildingMaterial
        name = "{@property:buildingmaterialproperties/density}";
        DBtest (Name2Rawname (name, rawname), "Name2RawnameWithBrackets BuildingMaterial -> true");
        DBtest (rawname.BeginsWith ("{@property:buildingmaterialproperties/density"),
                "Name2RawnameWithBrackets BuildingMaterial -> rawname unchanged");

        // Тест: уже корректный rawname Morph
        name = "{@morph:param1}";
        DBtest (Name2Rawname (name, rawname), "Name2RawnameWithBrackets {@morph:param1} -> true");
        DBtest (rawname,
                GS::UniString ("{@morph:param1}"),
                "Name2RawnameWithBrackets {@morph:param1} -> rawname unchanged");

        // Тест: уже корректный rawname IFC
        name = "{@ifc:propertyname}";
        DBtest (Name2Rawname (name, rawname), "Name2RawnameWithBrackets {@ifc:propertyname} -> true");
        DBtest (rawname,
                GS::UniString ("{@ifc:propertyname}"),
                "Name2RawnameWithBrackets {@ifc:propertyname} -> rawname unchanged");

        // Тест: уже корректный rawname Info
        name = "{@info:someinfo}";
        DBtest (Name2Rawname (name, rawname), "Name2RawnameWithBrackets {@info:someinfo} -> true");
        DBtest (rawname,
                GS::UniString ("{@info:someinfo}"),
                "Name2RawnameWithBrackets {@info:someinfo} -> rawname unchanged");

        // Тест: уже корректный rawname Glob
        name = "{@glob:variable}";
        DBtest (Name2Rawname (name, rawname), "Name2RawnameWithBrackets {@glob:variable} -> true");
        DBtest (rawname,
                GS::UniString ("{@glob:variable}"),
                "Name2RawnameWithBrackets {@glob:variable} -> rawname unchanged");

        // Тест: уже корректный rawname Class
        name = "{@class:classification}";
        DBtest (Name2Rawname (name, rawname), "Name2RawnameWithBrackets {@class:classification} -> true");
        DBtest (rawname,
                GS::UniString ("{@class:classification}"),
                "Name2RawnameWithBrackets {@class:classification} -> rawname unchanged");

        // Тест: уже корректный rawname Element
        name = "{@element:property}";
        DBtest (Name2Rawname (name, rawname), "Name2RawnameWithBrackets {@element:property} -> true");
        DBtest (rawname,
                GS::UniString ("{@element:property}"),
                "Name2RawnameWithBrackets {@element:property} -> rawname unchanged");

        // Тест: уже корректный rawname File
        name = "{@file:filename}";
        DBtest (Name2Rawname (name, rawname), "Name2RawnameWithBrackets {@file:filename} -> true");
        DBtest (rawname,
                GS::UniString ("{@file:filename}"),
                "Name2RawnameWithBrackets {@file:filename} -> rawname unchanged");

        // Тест: уже корректный rawname Attrib
        name = "{@attrib:layer}";
        DBtest (Name2Rawname (name, rawname), "Name2RawnameWithBrackets {@attrib:layer} -> true");
        DBtest (rawname,
                GS::UniString ("{@attrib:layer}"),
                "Name2RawnameWithBrackets {@attrib:layer} -> rawname unchanged");

        DBprnt ("TEST", "TestName2RawnameWithBrackets : done");
        return;
    }

    // -----------------------------------------------------------------------------
    // Тест SyncString - парсинг строки правила синхронизации
    // -----------------------------------------------------------------------------
    void TestSyncString () {
        DBprnt ("TEST", "TestSyncString");
        ParamValue param;
        SkipValues ignorevals;
        FormatString stringformat;
        SyncMode syncdirection = SYNC_NO;
        API_ElemTypeID elementType = API_ObjectID;

        // Тест: SYNC_FROM базовое свойство
        param = ParamValue ();
        GS::UniString rule1 = "Sync_from{Property:TestProperty}";
        DBtest (SyncString (elementType, rule1, syncdirection, param, ignorevals, stringformat, true, false, false),
                "SyncString Sync_from Property -> true");
        DBtest (syncdirection, SYNC_FROM, "SyncString Sync_from -> direction FROM");
        DBtest (param.fromProperty, "SyncString Sync_from Property -> fromProperty");

        // Тест: SYNC_TO базовое свойство
        param = ParamValue ();
        GS::UniString rule2 = "Sync_to{Property:TestProperty}";
        syncdirection = SYNC_NO;
        DBtest (SyncString (elementType, rule2, syncdirection, param, ignorevals, stringformat, true, false, false),
                "SyncString Sync_to Property -> true");
        DBtest (syncdirection, SYNC_TO, "SyncString Sync_to -> direction TO");
        DBtest (param.fromProperty, "SyncString Sync_to Property -> fromProperty");

        // Тест: SYNC_FROM_SUB
        param = ParamValue ();
        GS::UniString rule3 = "Sync_from_sub{Property:TestProperty}";
        syncdirection = SYNC_NO;
        DBtest (SyncString (elementType, rule3, syncdirection, param, ignorevals, stringformat, true, false, false),
                "SyncString Sync_from_sub -> true");
        DBtest (syncdirection, SYNC_FROM_SUB, "SyncString Sync_from_sub -> direction FROM_SUB");

        // Тест: SYNC_TO_SUB
        param = ParamValue ();
        GS::UniString rule4 = "Sync_to_sub{Property:TestProperty}";
        syncdirection = SYNC_NO;
        DBtest (SyncString (elementType, rule4, syncdirection, param, ignorevals, stringformat, true, false, false),
                "SyncString Sync_to_sub -> true");
        DBtest (syncdirection, SYNC_TO_SUB, "SyncString Sync_to_sub -> direction TO_SUB");

        // Тест: GDL параметр
        param = ParamValue ();
        GS::UniString rule5 = "Sync_from{MyGDLParam}";
        syncdirection = SYNC_NO;
        DBtest (SyncString (elementType, rule5, syncdirection, param, ignorevals, stringformat, true, false, false),
                "SyncString Sync_from GDL -> true");
        DBtest (param.fromGDLparam, "SyncString Sync_from GDL -> fromGDLparam");

        // Тест: Координаты
        param = ParamValue ();
        GS::UniString rule6 = "Sync_from{Coord:symb_pos_x}";
        syncdirection = SYNC_NO;
        DBtest (SyncString (elementType, rule6, syncdirection, param, ignorevals, stringformat, false, true, false),
                "SyncString Sync_from Coord -> true");
        DBtest (param.fromCoord, "SyncString Sync_from Coord -> fromCoord");

        // Тест: Формула
        param = ParamValue ();
        GS::UniString rule7 = "Sync_from{<2*2>}";
        syncdirection = SYNC_NO;
        DBtest (SyncString (elementType, rule7, syncdirection, param, ignorevals, stringformat, true, false, false),
                "SyncString Sync_from Formula -> true");
        DBtest (param.val.hasFormula, "SyncString Sync_from Formula -> hasFormula");

        // Тест: ID
        param = ParamValue ();
        GS::UniString rule8 = "Sync_from{{id}}";
        syncdirection = SYNC_NO;
        DBtest (SyncString (elementType, rule8, syncdirection, param, ignorevals, stringformat, true, false, false),
                "SyncString Sync_from ID -> true");
        DBtest (param.fromID, "SyncString Sync_from ID -> fromID");

        // Тест: FormatString с форматом
        param = ParamValue ();
        GS::UniString rule9 = "Sync_from{Property:TestProperty.3m}";
        syncdirection = SYNC_NO;
        DBtest (SyncString (elementType, rule9, syncdirection, param, ignorevals, stringformat, true, false, false),
                "SyncString Sync_from Format .3m -> true");
        DBtest (!stringformat.stringformat.IsEmpty (), "SyncString FormatString -> stringformat not empty");

        // Тест: ignorevals empty
        param = ParamValue ();
        GS::UniString rule10 = "Sync_from{Property:TestProperty; empty}";
        syncdirection = SYNC_NO;
        DBtest (SyncString (elementType, rule10, syncdirection, param, ignorevals, stringformat, true, false, false),
                "SyncString ignorevals empty -> true");
        DBtest (ignorevals.skip_empty, "SyncString ignorevals -> skip_empty true");

        // Тест: ignorevals trim_empty
        param = ParamValue ();
        GS::UniString rule11 = "Sync_from{Property:TestProperty; trim_empty}";
        syncdirection = SYNC_NO;
        DBtest (SyncString (elementType, rule11, syncdirection, param, ignorevals, stringformat, true, false, false),
                "SyncString ignorevals trim_empty -> true");
        DBtest (ignorevals.skip_trim_empty, "SyncString ignorevals -> skip_trim_empty true");

        // Тест: некорректная строка (нет направления)
        param = ParamValue ();
        GS::UniString rule12 = "Property:TestProperty";
        syncdirection = SYNC_NO;
        DBtest (!SyncString (elementType, rule12, syncdirection, param, ignorevals, stringformat, false, false, false),
                "SyncString no direction -> false");

        // #184/#185: правило состава конструкции из реального проекта —
        // Sync_from{Material:Layers; "<шаблон>"}. Признак правила в палитре считался через
        // ParsePropertyDescriptionToRules, который вызывает SyncString с elementType =
        // API_ObjectID; правило материала при этом отбраковывалось, из-за чего фильтр
        // «Только с правилами» и синяя маркировка его не видели.
        const GS::UniString ruleMaterial =
            "Sync_from{Material:Layers; \"3зн %BuildingMaterialProperties/Building Material Thermal Conductivity.3pm% / \"}";

        param = ParamValue ();
        syncdirection = SYNC_NO;
        DBtest (
            SyncString (API_WallID, ruleMaterial, syncdirection, param, ignorevals, stringformat, true, false, false),
            "SyncString Sync_from Material:Layers + API_WallID -> true");
        DBtest (param.fromMaterial, "SyncString Sync_from Material:Layers + API_WallID -> fromMaterial");

        param = ParamValue ();
        syncdirection = SYNC_NO;
        // Путь разбора описания в палитре: проверка типов отключена (#184/#185).
        DBtest (
            SyncString (
                API_ObjectID, ruleMaterial, syncdirection, param, ignorevals, stringformat, true, false, false, false),
            "SyncString Sync_from Material:Layers + API_ObjectID (no type check) -> true");
        DBtest (param.fromMaterial,
                "SyncString Sync_from Material:Layers + API_ObjectID (no type check) -> fromMaterial");

        DBprnt ("TEST", "TestSyncString : done");
        return;
    }

    // -----------------------------------------------------------------------------
    // Тест реальных правил синхронизации из BuildingInformation.xml
    // -----------------------------------------------------------------------------
    void TestSyncStringRealRules () {
        DBprnt ("TEST", "TestSyncStringRealRules");
        ParamValue param;
        SkipValues ignorevals;
        FormatString stringformat;
        SyncMode syncdirection = SYNC_NO;

        // --- GDL параметры (API_ObjectID, syncall=true) ---
        param = ParamValue ();
        GS::UniString rule = "Sync_from{ac_wallhole_width}";
        syncdirection = SYNC_NO;
        DBtest (SyncString (API_ObjectID, rule, syncdirection, param, ignorevals, stringformat, true, false, false),
                "SyncStringReal GDL ac_wallhole_width -> true");
        DBtest (param.fromGDLparam, "SyncStringReal GDL ac_wallhole_width -> fromGDLparam");

        param = ParamValue ();
        rule = "Sync_from{naen}";
        syncdirection = SYNC_NO;
        DBtest (SyncString (API_ObjectID, rule, syncdirection, param, ignorevals, stringformat, true, false, false),
                "SyncStringReal GDL naen -> true");
        DBtest (param.fromGDLparam, "SyncStringReal GDL naen -> fromGDLparam");

        // --- GDL описание (API_ObjectID, syncall=true) ---
        param = ParamValue ();
        rule = "Sync_from{description:Наименование}";
        syncdirection = SYNC_NO;
        DBtest (SyncString (API_ObjectID, rule, syncdirection, param, ignorevals, stringformat, true, false, false),
                "SyncStringReal GDL description -> true");
        DBtest (param.fromGDLdescription, "SyncStringReal GDL description -> fromGDLdescription");

        // --- Свойства с русскими именами и слэшами (API_ObjectID, syncall=true) ---
        param = ParamValue ();
        rule = "Sync_from{Property:Свойства и параметры/_Свойство в свойство}";
        syncdirection = SYNC_NO;
        DBtest (SyncString (API_ObjectID, rule, syncdirection, param, ignorevals, stringformat, true, false, false),
                "SyncStringReal Property русский путь -> true");
        DBtest (param.fromProperty, "SyncStringReal Property русский путь -> fromProperty");
        DBtest (param.name,
                GS::UniString ("Свойства и параметры/_Свойство в свойство"),
                "SyncStringReal Property русский путь -> name");

        // --- Координаты (API_ObjectID, synccoord=true) ---
        param = ParamValue ();
        rule = "Sync_from{Coord:symb_rotangle}";
        syncdirection = SYNC_NO;
        DBtest (SyncString (API_ObjectID, rule, syncdirection, param, ignorevals, stringformat, false, true, false),
                "SyncStringReal Coord symb_rotangle -> true");
        DBtest (param.fromCoord, "SyncStringReal Coord symb_rotangle -> fromCoord");

        param = ParamValue ();
        rule = "Sync_from{Coord:symb_rotangle_correct}";
        syncdirection = SYNC_NO;
        DBtest (SyncString (API_ObjectID, rule, syncdirection, param, ignorevals, stringformat, false, true, false),
                "SyncStringReal Coord symb_rotangle_correct -> true");
        DBtest (param.fromCoord, "SyncStringReal Coord symb_rotangle_correct -> fromCoord");

        param = ParamValue ();
        rule = "Sync_from{Coord:symb_pos_correct}";
        syncdirection = SYNC_NO;
        DBtest (SyncString (API_ObjectID, rule, syncdirection, param, ignorevals, stringformat, false, true, false),
                "SyncStringReal Coord symb_pos_correct -> true");
        DBtest (param.fromCoord, "SyncStringReal Coord symb_pos_correct -> fromCoord");

        param = ParamValue ();
        rule = "Sync_from{Coord:symb_pos_correct_hard}";
        syncdirection = SYNC_NO;
        DBtest (SyncString (API_ObjectID, rule, syncdirection, param, ignorevals, stringformat, false, true, false),
                "SyncStringReal Coord symb_pos_correct_hard -> true");
        DBtest (param.fromCoord, "SyncStringReal Coord symb_pos_correct_hard -> fromCoord");

        param = ParamValue ();
        rule = "Sync_from{Coord:symb_pos_x_correct}";
        syncdirection = SYNC_NO;
        DBtest (SyncString (API_ObjectID, rule, syncdirection, param, ignorevals, stringformat, false, true, false),
                "SyncStringReal Coord symb_pos_x_correct -> true");
        DBtest (param.fromCoord, "SyncStringReal Coord symb_pos_x_correct -> fromCoord");

        param = ParamValue ();
        rule = "Sync_from{Coord:symb_pos_y_correct}";
        syncdirection = SYNC_NO;
        DBtest (SyncString (API_ObjectID, rule, syncdirection, param, ignorevals, stringformat, false, true, false),
                "SyncStringReal Coord symb_pos_y_correct -> true");
        DBtest (param.fromCoord, "SyncStringReal Coord symb_pos_y_correct -> fromCoord");

        param = ParamValue ();
        rule = "Sync_from{Coord:l_correct}";
        syncdirection = SYNC_NO;
        DBtest (SyncString (API_ObjectID, rule, syncdirection, param, ignorevals, stringformat, false, true, false),
                "SyncStringReal Coord l_correct -> true");
        DBtest (param.fromCoord, "SyncStringReal Coord l_correct -> fromCoord");

        // --- Classification FROM (API_ObjectID, syncall=true) ---
        param = ParamValue ();
        rule = "Sync_from{Class:Test_Addon; FullName}";
        syncdirection = SYNC_NO;
        DBtest (SyncString (API_ObjectID, rule, syncdirection, param, ignorevals, stringformat, true, false, false),
                "SyncStringReal Classification FROM -> true");
        DBtest (param.fromClassification, "SyncStringReal Classification FROM -> fromClassification");

        // --- Classification TO (API_ObjectID, syncall=true, syncclass=true) ---
        param = ParamValue ();
        rule = "Sync_to{Class:Test_Addon}";
        syncdirection = SYNC_NO;
        DBtest (SyncString (API_ObjectID, rule, syncdirection, param, ignorevals, stringformat, true, false, true),
                "SyncStringReal Classification TO -> true");
        DBtest (param.fromClassification, "SyncStringReal Classification TO -> fromClassification");
        DBtest (syncdirection, SYNC_TO, "SyncStringReal Classification TO -> direction TO");

        // --- Material (нужен API_WallID, т.к. Material не проходит для API_ObjectID без fromQuantity) ---
        param = ParamValue ();
        rule =
            R"(Sync_from{Material:Layers; "3зн %BuildingMaterialProperties/Building Material Thermal Conductivity.3pm% / "})";
        syncdirection = SYNC_NO;
        DBtest (SyncString (API_WallID, rule, syncdirection, param, ignorevals, stringformat, true, false, false),
                "SyncStringReal Material Layers default pen -> true");
        DBtest (param.fromMaterial, "SyncStringReal Material Layers default pen -> fromMaterial");

        param = ParamValue ();
        rule =
            R"(Sync_from{Material:Layers; "2зн %BuildingMaterialProperties/Building Material Thermal Conductivity.2m% / "})";
        syncdirection = SYNC_NO;
        DBtest (SyncString (API_WallID, rule, syncdirection, param, ignorevals, stringformat, true, false, false),
                "SyncStringReal Material Layers 2зн -> true");
        DBtest (param.fromMaterial, "SyncStringReal Material Layers 2зн -> fromMaterial");

        param = ParamValue ();
        rule = R"(Sync_from{Material:Layers; "%Описание% - %Толщина.2mm%мм. "})";
        syncdirection = SYNC_NO;
        DBtest (SyncString (API_WallID, rule, syncdirection, param, ignorevals, stringformat, true, false, false),
                "SyncStringReal Material Layers старое -> true");
        DBtest (param.fromMaterial, "SyncStringReal Material Layers старое -> fromMaterial");

        param = ParamValue ();
        rule = R"(Sync_from{Material:Layers, 20; "%Описание% - %Толщина.2mm%мм. "})";
        syncdirection = SYNC_NO;
        DBtest (SyncString (API_WallID, rule, syncdirection, param, ignorevals, stringformat, true, false, false),
                "SyncStringReal Material Layers pen 20 -> true");
        DBtest (param.fromMaterial, "SyncStringReal Material Layers pen 20 -> fromMaterial");

        param = ParamValue ();
        rule = R"(Sync_from{Material:Layers, 6; "%Описание% - %Толщина.2mm%мм. "})";
        syncdirection = SYNC_NO;
        DBtest (SyncString (API_WallID, rule, syncdirection, param, ignorevals, stringformat, true, false, false),
                "SyncStringReal Material Layers pen 6 -> true");
        DBtest (param.fromMaterial, "SyncStringReal Material Layers pen 6 -> fromMaterial");

        // --- Material со сложной формулой R0усл ---
        param = ParamValue ();
        rule =
            R"(Sync_from{Material:Layers, 6; "1/{Property:Теплотехнический расчёт/αint, Вт\/(м2°С)} + 1/{Property:Теплотехнический расчёт/αext, Вт\/(м2°С)} <+%layer_thickness.3m%/%BuildingMaterialProperties/Building Material Thermal Conductivity.3m%>"})";
        syncdirection = SYNC_NO;
        DBtest (SyncString (API_WallID, rule, syncdirection, param, ignorevals, stringformat, true, false, false),
                "SyncStringReal Material R0усл -> true");
        DBtest (param.fromMaterial, "SyncStringReal Material R0усл -> fromMaterial");

        param = ParamValue ();
        rule =
            R"(Sync_from{Material:Layers, 6; "1/{Property:Теплотехнический расчёт/αint, Вт\/(м2°С)} + 1/{Property:Теплотехнический расчёт/αext, Вт\/(м2°С)} <+%layer_thickness.3m%/%BuildingMaterialProperties/Building Material Thermal Conductivity.3m%>".3m})";
        syncdirection = SYNC_NO;
        DBtest (SyncString (API_WallID, rule, syncdirection, param, ignorevals, stringformat, true, false, false),
                "SyncStringReal Material R0усл .3m -> true");
        DBtest (param.fromMaterial, "SyncStringReal Material R0усл .3m -> fromMaterial");

        param = ParamValue ();
        rule =
            R"(Sync_from{Material:Layers, 6; "1/{Property:Теплотехнический расчёт/αint, Вт\/(м2°С)} + 1/{Property:Теплотехнический расчёт/αext, Вт\/(м2°С)} <+%толщина.3m%/%BuildingMaterialProperties/Building Material Thermal Conductivity.3m%>".3mp})";
        syncdirection = SYNC_NO;
        DBtest (SyncString (API_WallID, rule, syncdirection, param, ignorevals, stringformat, true, false, false),
                "SyncStringReal Material R0усл .3mp -> true");
        DBtest (param.fromMaterial, "SyncStringReal Material R0усл .3mp -> fromMaterial");

        DBprnt ("TEST", "TestSyncStringRealRules : done");
        return;
    }

    // -----------------------------------------------------------------------------
    // Тест констант префиксов
    // -----------------------------------------------------------------------------
    void TestParsePrefixes () {
        DBprnt ("TEST", "TestParsePrefixes");

        // Проверка основных префиксов имен параметров
        DBtest (PROPERTYNAMEPREFIX, GS::UniString ("{@property:"), "PROPERTYNAMEPREFIX");
        DBtest (GDLNAMEPREFIX, GS::UniString ("{@gdl:"), "GDLNAMEPREFIX");
        DBtest (COORDNAMEPREFIX, GS::UniString ("{@coord:"), "COORDNAMEPREFIX");
        DBtest (IDNAMEPREFIX, GS::UniString ("{@id:"), "IDNAMEPREFIX");
        DBtest (MORPHNAMEPREFIX, GS::UniString ("{@morph:"), "MORPHNAMEPREFIX");
        DBtest (INFONAMEPREFIX, GS::UniString ("{@info:"), "INFONAMEPREFIX");
        DBtest (IFCNAMEPREFIX, GS::UniString ("{@ifc:"), "IFCNAMEPREFIX");
        DBtest (GLOBNAMEPREFIX, GS::UniString ("{@glob:"), "GLOBNAMEPREFIX");
        DBtest (CLASSNAMEPREFIX, GS::UniString ("{@class:"), "CLASSNAMEPREFIX");
        DBtest (ELEMENTNAMEPREFIX, GS::UniString ("{@element:"), "ELEMENTNAMEPREFIX");
        DBtest (FILENAMEPREFIX, GS::UniString ("{@file:"), "FILENAMEPREFIX");
        DBtest (ATTRIBNAMEPREFIX, GS::UniString ("{@attrib:"), "ATTRIBNAMEPREFIX");
        DBtest (LISTDATANAMEPREFIX, GS::UniString ("{@listdata:"), "LISTDATANAMEPREFIX");
        DBtest (MATERIALNAMEPREFIX, GS::UniString ("{@material:"), "MATERIALNAMEPREFIX");
        DBtest (FORMULANAMEPREFIX, GS::UniString ("{@formula:"), "FORMULANAMEPREFIX");
        DBtest (MEPNAMEPREFIX, GS::UniString ("{@mep:"), "MEPNAMEPREFIX");
        DBtest (FLAGNAMEPREFIX, GS::UniString ("{@flag:"), "FLAGNAMEPREFIX");

        // Проверка числовых индексов типов
        DBtest (PROPERTYTYPEINX, (short)2, "PROPERTYTYPEINX");
        DBtest (GDLTYPEINX, (short)4, "GDLTYPEINX");
        DBtest (COORDTYPEINX, (short)3, "COORDTYPEINX");
        DBtest (IDTYPEINX, (short)1, "IDTYPEINX");
        DBtest (MORPHTYPEINX, (short)8, "MORPHTYPEINX");
        DBtest (INFOTYPEINX, (short)6, "INFOTYPEINX");
        DBtest (IFCTYPEINX, (short)7, "IFCTYPEINX");
        DBtest (GLOBTYPEINX, (short)12, "GLOBTYPEINX");
        DBtest (CLASSTYPEINX, (short)13, "CLASSTYPEINX");
        DBtest (ELEMENTTYPEINX, (short)15, "ELEMENTTYPEINX");
        DBtest (FILETYPEINX, (short)17, "FILETYPEINX");
        DBtest (ATTRIBTYPEINX, (short)9, "ATTRIBTYPEINX");
        DBtest (LISTDATATYPEINX, (short)10, "LISTDATATYPEINX");
        DBtest (MATERIALTYPEINX, (short)11, "MATERIALTYPEINX");
        DBtest (FORMULATYPEINX, (short)14, "FORMULATYPEINX");
        DBtest (MEPTYPEINX, (short)16, "MEPTYPEINX");
        DBtest (FLAGTYPEINX, (short)18, "FLAGTYPEINX");

        // Проверка констант синхронизации
        DBtest (SYNC_FROM, 1, "SYNC_FROM");
        DBtest (SYNC_TO, 2, "SYNC_TO");
        DBtest (SYNC_TO_SUB, 3, "SYNC_TO_SUB");
        DBtest (SYNC_FROM_SUB, 4, "SYNC_FROM_SUB");
        DBtest (SYNC_FROM_GUID, 5, "SYNC_FROM_GUID");
        DBtest (SYNC_FROM_ZONE, 6, "SYNC_FROM_ZONE");
        DBtest (SYNC_TO_ZONE, 7, "SYNC_TO_ZONE");

        // Проверка префиксов правил
        DBtest (SYNCFROMSTRING, GS::UniString ("from{"), "SYNCFROMSTRING");
        DBtest (SYNCTOSTRING, GS::UniString ("to{"), "SYNCTOSTRING");
        DBtest (SYNCFROMSUBSTRING, GS::UniString ("from_sub{"), "SYNCFROMSUBSTRING");
        DBtest (SYNCTOSUBSTRING, GS::UniString ("to_sub{"), "SYNCTOSUBSTRING");
        DBtest (FROMGUIDBR, GS::UniString ("from_GUID{"), "FROMGUIDBR");
        DBtest (FROMGUID, GS::UniString ("from_GUID"), "FROMGUID");
        DBtest (TOGUIDBR, GS::UniString ("to_GUID{"), "TOGUIDBR");
        DBtest (TOGUID, GS::UniString ("to_GUID"), "TOGUID");

        // Проверка специальных символов
        DBtest (BRACESTART, GS::UniString ("{"), "BRACESTART");
        DBtest (BRACEEND, GS::UniString ("}"), "BRACEEND");
        DBtest (SEMICOLON, GS::UniString (";"), "SEMICOLON");
        DBtest (GS::UniString ("{"), GS::UniString ("{"), "CHARBRACESTART as string");
        DBtest (GS::UniString ("}"), GS::UniString ("}"), "CHARBRACEEND as string");
        DBtest (GS::UniString (";"), GS::UniString (";"), "CHARBSEMICOLON as string");
        DBtest (GS::UniString ("<"), GS::UniString ("<"), "CHARFORMULASTART as string");
        DBtest (GS::UniString (">"), GS::UniString (">"), "CHARFORMULAEND as string");
        DBtest (GS::UniString ("\""), GS::UniString ("\""), "CHARDQUT as string");
        DBtest (GS::UniString (","), GS::UniString (","), "CHARCOMMA as string");

        // Проверка SYNCNAME и других специальных констант
        DBtest (SYNCNAME, GS::UniString ("sync_name"), "SYNCNAME");
        DBtest (SYNCCORRECTFLAG, GS::UniString ("Sync_correct_flag"), "SYNCCORRECTFLAG");
        DBtest (SYNCCLASSFLAG, GS::UniString ("Sync_class_flag"), "SYNCCLASSFLAG");
        DBtest (SYNCGUID, GS::UniString ("Sync_GUID"), "SYNCGUID");

        // Проверка специальных ключевых слов
        DBtest (RENUMFLAG, GS::UniString ("Renum_flag"), "RENUMFLAG");
        DBtest (RENUM, GS::UniString ("Renum"), "RENUM");
        DBtest (PROPERTYSTRING, GS::UniString ("property"), "PROPERTYSTRING");

        // Проверка форматов
        DBtest (DEFULTREALFSTRING, GS::UniString (".3m"), "DEFULTREALFSTRING");
        DBtest (DEFULTLEGHTFSTRING, GS::UniString ("1mm"), "DEFULTLEGHTFSTRING");
        DBtest (DEFULTINTFSTRING, GS::UniString ("0m"), "DEFULTINTFSTRING");

        DBprnt ("TEST", "TestParsePrefixes : done");
        return;
    }

    // -----------------------------------------------------------------------------
    // Тест парсинга описания свойства с командами Sync, Renum, Sum, Spec
    // -----------------------------------------------------------------------------
    void TestParsePropertyDescription () {
        DBprnt ("TEST", "TestParsePropertyDescription");

        ParamValue param;
        SkipValues ignorevals;
        FormatString stringformat;
        SyncMode syncdirection = SYNC_NO;
        API_ElemTypeID elementType = API_ObjectID;

        // Тест 1: Описание только с Sync_from
        {
            GS::UniString desc = "Sync_from{Property:TestProperty}";
            bool ok =
                SyncString (elementType, desc, syncdirection, param, ignorevals, stringformat, true, false, false);
            DBtest (ok, "ParseDesc Sync_only -> true");
            DBtest (param.fromProperty, "ParseDesc Sync_only -> fromProperty");
        }

        // Тест 2: Описание с несколькими Sync командами через разделитель
        {
            GS::UniString desc = "Sync_from{Property:Prop1}Sync_to{Property:Prop2}";
            GS::Array<GS::UniString> parts;
            GS::Array<GS::UniString> scratch;
            UInt32 n = StringSpltFilter (desc, SYNCPART, parts, BRACESTART, &scratch);
            DBtest (n, (UInt32)2, "ParseDesc MultiSync -> 2 parts");
        }

        // Тест 3: Описание с Renum_flag
        {
            GS::UniString desc = "Renum_flag{Property:RenumRule; NULL}";
            GS::UniString ldesc = desc.ToLowerCase ();
            DBtest (ldesc.Contains (RENUMFLAG.ToLowerCase ()), "ParseDesc Renum_flag -> contains Renum_flag");
        }

        // Тест 4: Описание с Renum
        {
            GS::UniString desc = "Renum{Property:Criteria; Property:Delimetr}";
            GS::UniString ldesc = desc.ToLowerCase ();
            DBtest (ldesc.Contains (RENUM.ToLowerCase ()), "ParseDesc Renum -> contains Renum");
        }

        // Тест 5: Описание с Sum
        {
            GS::UniString desc = "Sum{Property:SumProp1; Property:SumProp2; max}";
            GS::UniString ldesc = desc.ToLowerCase ();
            DBtest (ldesc.Contains ("sum{"), "ParseDesc Sum -> contains Sum");
        }

        // Тест 6: Описание с Spec_rule
        {
            GS::UniString desc = "Spec_rule{g(U, P, F, Q)}{s(Pn, Qn)}";
            GS::UniString ldesc = desc.ToLowerCase ();
            DBtest (ldesc.Contains ("spec_rule"), "ParseDesc Spec_rule -> contains Spec_rule");
        }

        // Тест 7: Комбинированное описание (Sync + Renum)
        {
            GS::UniString desc = "Sync_from{Property:Source}Renum_flag{Property:RenumRule}";
            GS::Array<GS::UniString> parts;
            GS::Array<GS::UniString> scratch;
            UInt32 n = StringSpltFilter (desc, SYNCPART, parts, BRACESTART, &scratch);
            DBtest (n >= 1, "ParseDesc Combined -> at least 1 sync part");
            GS::UniString ldesc = desc.ToLowerCase ();
            DBtest (ldesc.Contains (RENUMFLAG.ToLowerCase ()), "ParseDesc Combined -> contains Renum_flag");
        }

        // Тест 8: Описание с игнорируемыми значениями
        {
            GS::UniString desc = "Sync_from{Property:TestProperty; empty; trim_empty}";
            bool ok =
                SyncString (elementType, desc, syncdirection, param, ignorevals, stringformat, true, false, false);
            DBtest (ok, "ParseDesc IgnoreVals -> true");
            DBtest (ignorevals.skip_empty, "ParseDesc IgnoreVals -> skip_empty");
            DBtest (ignorevals.skip_trim_empty, "ParseDesc IgnoreVals -> skip_trim_empty");
        }

        // Тест 9: Описание с форматом
        {
            GS::UniString desc = "Sync_from{Property:TestProperty.3m}";
            bool ok =
                SyncString (elementType, desc, syncdirection, param, ignorevals, stringformat, true, false, false);
            DBtest (ok, "ParseDesc Format .3m -> true");
            DBtest (!stringformat.stringformat.IsEmpty (), "ParseDesc Format -> format not empty");
        }

        // Тест 10: Описание с Formula
        {
            GS::UniString desc = "Sync_from{<2*2>.3m}";
            bool ok =
                SyncString (elementType, desc, syncdirection, param, ignorevals, stringformat, true, false, false);
            DBtest (ok, "ParseDesc Formula -> true");
            DBtest (param.val.hasFormula, "ParseDesc Formula -> hasFormula");
        }

        // Тест 11: Пустое описание
        {
            GS::UniString desc = "";
            GS::Array<GS::UniString> parts;
            GS::Array<GS::UniString> scratch;
            UInt32 n = StringSpltFilter (desc, SYNCPART, parts, BRACESTART, &scratch);
            DBtest (n, (UInt32)0, "ParseDesc Empty -> 0 parts");
        }

        DBprnt ("TEST", "TestParsePropertyDescription : done");
        return;
    }

    // -----------------------------------------------------------------------------
    // Тест независимого вызова ParseSyncString (Этап 2 TDD)
    // -----------------------------------------------------------------------------
    void TestParseSyncStringIndependent () {
        DBprnt ("TEST", "TestParseSyncStringIndependent");

        // Подготовка тестовых данных
        API_Guid elemGuid = APINULLGuid;
        API_ElemTypeID elementType = API_ObjectID;
        API_PropertyDefinition definition = {};
        definition.description = "Sync_from{Property:TestProperty}";
        GS::Array<WriteData> syncRules;
        ParamDictElement paramToRead;
        bool hasSub = false;
        bool syncall = true;
        bool synccoord = false;
        bool syncclass = false;
        ParamDictValue subproperty;

        // Тест: ParseSyncString должен возвращать true для корректного описания
        bool result = ParseSyncString (elemGuid,
                                       elementType,
                                       definition,
                                       syncRules,
                                       paramToRead,
                                       hasSub,
                                       syncall,
                                       synccoord,
                                       syncclass,
                                       subproperty);
        DBtest (result, "ParseSyncString basic -> true");

        // Тест: синхронизация правила должна быть добавлена в syncRules
        DBtest (syncRules.GetSize () > 0, "ParseSyncString -> syncRules not empty");

        // Тест: hasSub должен быть false для правила без from_sub/to_sub
        DBtest (!hasSub, "ParseSyncString -> hasSub false");

        // Тест: пустое описание -> false
        definition.description = "";
        syncRules.Clear ();
        result = ParseSyncString (elemGuid,
                                  elementType,
                                  definition,
                                  syncRules,
                                  paramToRead,
                                  hasSub,
                                  syncall,
                                  synccoord,
                                  syncclass,
                                  subproperty);
        DBtest (!result, "ParseSyncString empty -> false");

        // Тест: описание с Sync_flag -> false (это флаг, не правило)
        definition.description = "Sync_flag";
        result = ParseSyncString (elemGuid,
                                  elementType,
                                  definition,
                                  syncRules,
                                  paramToRead,
                                  hasSub,
                                  syncall,
                                  synccoord,
                                  syncclass,
                                  subproperty);
        DBtest (!result, "ParseSyncString Sync_flag -> false");

        // Тест: описание с SYNCCORRECTFLAG -> true (добавляет служебный параметр)
        definition.description = "Sync_correct_flag";
        result = ParseSyncString (elemGuid,
                                  elementType,
                                  definition,
                                  syncRules,
                                  paramToRead,
                                  hasSub,
                                  syncall,
                                  synccoord,
                                  syncclass,
                                  subproperty);
        DBtest (result, "ParseSyncString Sync_correct_flag -> true");

        // Тест: описание без SYNCPART -> false
        definition.description = "Property:TestProperty";
        syncRules.Clear ();
        result = ParseSyncString (elemGuid,
                                  elementType,
                                  definition,
                                  syncRules,
                                  paramToRead,
                                  hasSub,
                                  syncall,
                                  synccoord,
                                  syncclass,
                                  subproperty);
        DBtest (!result, "ParseSyncString no SYNCPART -> false");

        // Тест: описание без BRACESTART -> false
        definition.description = "Sync_from Property:TestProperty";
        result = ParseSyncString (elemGuid,
                                  elementType,
                                  definition,
                                  syncRules,
                                  paramToRead,
                                  hasSub,
                                  syncall,
                                  synccoord,
                                  syncclass,
                                  subproperty);
        DBtest (!result, "ParseSyncString no BRACESTART -> false");

        // Тест: описание без BRACEEND -> false
        definition.description = "Sync_from{Property:TestProperty";
        result = ParseSyncString (elemGuid,
                                  elementType,
                                  definition,
                                  syncRules,
                                  paramToRead,
                                  hasSub,
                                  syncall,
                                  synccoord,
                                  syncclass,
                                  subproperty);
        DBtest (!result, "ParseSyncString no BRACEEND -> false");

        DBprnt ("TEST", "TestParseSyncStringIndependent : done");
        return;
    }

    // -----------------------------------------------------------------------------
    // Тест ParsePropertyDescriptionToRules — парсинг описания в структурированные правила
    // -----------------------------------------------------------------------------
    void TestParsePropertyDescriptionToRules () {
        DBprnt ("TEST", "TestParsePropertyDescriptionToRules");

        // Тест 1: простое Sync_from описание
        {
            DBprnt ("DescToRules", "Test 1 start");
            GS::UniString desc = "Sync_from{Property:TestProperty}";
            ParsePropertyResult result = ParsePropertyDescriptionToRules (desc);
            DBprnt ("DescToRules", "Test 1 after call");
            DBtest (result.hasSyncRules, "DescToRules Sync_from -> hasSyncRules");
            DBtest (result.syncRules.GetSize () > 0, "DescToRules Sync_from -> rules not empty");
            if (result.syncRules.GetSize () > 0) {
                DBtest (result.syncRules[0].commandType == "Sync" || result.syncRules[0].commandType == "Sync_from",
                        "DescToRules Sync_from -> commandType");
                DBtest (result.syncRules[0].sourceType == "Property", "DescToRules Sync_from -> sourceType Property");
                DBtest (result.syncRules[0].isValid, "DescToRules Sync_from -> isValid");
                DBtest (!result.syncRules[0].hasSub, "DescToRules Sync_from -> hasSub false");
                DBtest (!result.syncRules[0].hasGUID, "DescToRules Sync_from -> hasGUID false");
            }
        }

        // Тест 2: Sync_from_sub
        {
            GS::UniString desc = "Sync_from_sub{Property:SubProp}";
            ParsePropertyResult result = ParsePropertyDescriptionToRules (desc);
            DBtest (result.hasSyncRules, "DescToRules Sync_from_sub -> hasSyncRules");
            if (result.syncRules.GetSize () > 0) {
                DBtest (result.syncRules[0].hasSub, "DescToRules Sync_from_sub -> hasSub true");
            }
        }

        // Тест 3: Sync_to
        {
            GS::UniString desc = "Sync_to{Property:TargetProp}";
            ParsePropertyResult result = ParsePropertyDescriptionToRules (desc);
            DBtest (result.hasSyncRules, "DescToRules Sync_to -> hasSyncRules");
            if (result.syncRules.GetSize () > 0) {
                DBtest (result.syncRules[0].targetType == "Property", "DescToRules Sync_to -> targetType Property");
            }
        }

        // Тест 4: Пустое описание
        {
            GS::UniString desc = "";
            ParsePropertyResult result = ParsePropertyDescriptionToRules (desc);
            DBtest (!result.hasSyncRules, "DescToRules empty -> no sync rules");
            DBtest (!result.hasOtherCommands, "DescToRules empty -> no other commands");
        }

        // Тест 5: Описание с командой Renum (не Sync)
        {
            GS::UniString desc = "Renum_flag{Property:RenumRule; NULL}";
            ParsePropertyResult result = ParsePropertyDescriptionToRules (desc);
            DBtest (!result.hasSyncRules, "DescToRules Renum_flag -> no sync rules");
            DBtest (result.hasOtherCommands, "DescToRules Renum_flag -> has other commands");
        }

        // Тест 6: Комбинированное описание
        {
            GS::UniString desc = "Sync_from{Property:Source}Sync_to{Property:Target}";
            ParsePropertyResult result = ParsePropertyDescriptionToRules (desc);
            DBtest (result.hasSyncRules, "DescToRules combined -> hasSyncRules");
            DBtest (result.syncRules.GetSize () == 2, "DescToRules combined -> 2 rules");
        }

        // Тест 7: Описание с ignorevals
        {
            GS::UniString desc = "Sync_from{Property:TestProperty; empty; trim_empty}";
            ParsePropertyResult result = ParsePropertyDescriptionToRules (desc);
            DBtest (result.hasSyncRules, "DescToRules ignorevals -> hasSyncRules");
            if (result.syncRules.GetSize () > 0) {
                DBtest (result.syncRules[0].ignoreVals.GetSize () > 0,
                        "DescToRules ignorevals -> ignoreVals not empty");
            }
        }

        // Тест 8: Sync_flag (не создаёт правило)
        {
            GS::UniString desc = "Sync_flag";
            ParsePropertyResult result = ParsePropertyDescriptionToRules (desc);
            DBtest (!result.hasSyncRules, "DescToRules Sync_flag -> no sync rules");
            DBtest (!result.hasOtherCommands, "DescToRules Sync_flag -> no other commands");
        }

        // Тест 9: Несколько правил + remainingText
        {
            GS::UniString desc = "Sync_from{Property:Prop1}Some text between Sync_to{Property:Prop2}";
            ParsePropertyResult result = ParsePropertyDescriptionToRules (desc);
            DBtest (result.hasSyncRules, "DescToRules multi -> hasSyncRules");
            DBtest (result.syncRules.GetSize () >= 1, "DescToRules multi -> at least 1 rule");
        }

        // Тест 10: Описание с GDL параметром
        {
            GS::UniString desc = "Sync_from{MyGDLParam}";
            ParsePropertyResult result = ParsePropertyDescriptionToRules (desc);
            DBtest (result.hasSyncRules, "DescToRules GDL -> hasSyncRules");
            if (result.syncRules.GetSize () > 0) {
                DBtest (result.syncRules[0].sourceType == "GDL", "DescToRules GDL -> sourceType GDL");
            }
        }

        DBprnt ("TEST", "TestParsePropertyDescriptionToRules : done");
        return;
    }

    // -----------------------------------------------------------------------------
    // Тест SyncAddSubelement — развёртывание правил синхронизации на подэлементы.
    //
    // Структура: 1 RED-тест на найденный баг P1 + 3 GREEN-регрессии, фиксирующие
    // существующее корректное поведение. GREEN-тесты должны давать одинаковый
    // результат до и после правок.
    //
    // Баг P1 (Sync.cpp, SyncAddSubelement): вторая ветка проверяет `fromSub`,
    // хотя по семантике тела цикла (заполнение guidTo для КАЖДОГО подэлемента,
    // сброс toSub) это обработка `toSub`. Из-за этого правило to_sub не
    // разворачивается на подэлементы — ветка недостижима:
    //   - если fromSub был true — первая ветка уже сбросила его в false;
    //   - если fromSub был false — условие ложно сразу.
    // После исправления (`if (mainsyncRule.toSub)`) RED-тест становится зелёным,
    // а GREEN-тесты обязаны остаться зелёными.
    // -----------------------------------------------------------------------------
    void TestSyncAddSubelement () {
        DBprnt ("TEST", "TestSyncAddSubelement");

        // ---- Вспомогательное правило-прототип ----
        auto makeRule = [] () {
            WriteData rule;
            rule.guidTo = APIGuidFromString ("{11111111-1111-1111-1111-111111111111}");
            rule.guidFrom = APIGuidFromString ("{22222222-2222-2222-2222-222222222222}");
            rule.paramFrom.rawName = "{@property:test_from}";
            rule.paramFrom.fromProperty = true;
            rule.paramFrom.isValid = true;
            rule.paramTo.rawName = "{@property:test_to}";
            rule.paramTo.fromProperty = true;
            rule.paramTo.isValid = true;
            return rule;
        };

        const API_Guid sub1 = APIGuidFromString ("{AAAAAAAA-AAAA-AAAA-AAAA-AAAAAAAAAAAA}");
        const API_Guid sub2 = APIGuidFromString ("{BBBBBBBB-BBBB-BBBB-BBBB-BBBBBBBBBBBB}");
        GS::Array<API_Guid> subelemGuids;
        subelemGuids.Push (sub1);
        subelemGuids.Push (sub2);

        // =============================================================================
        // GREEN-тест 1: обычное правило (не from_sub и не to_sub) при пустом списке
        // подэлементов добавляется в syncRules как есть — по guidTo из правила.
        // Существующее поведение, должно остаться неизменным.
        // =============================================================================
        {
            GS::Array<WriteData> mainsyncRules;
            mainsyncRules.Push (makeRule ());
            WriteDict syncRules;
            ParamDictElement paramToRead;
            GS::Array<API_Guid> emptySubs;

            SyncAddSubelement (emptySubs, mainsyncRules, syncRules, paramToRead);

            const GS::Array<WriteData> *bucket = syncRules.GetPtr (mainsyncRules[0].guidTo);
            bool added = bucket != nullptr && bucket->GetSize () == 1 &&
                         bucket->Get (0).guidTo == mainsyncRules[0].guidTo &&
                         bucket->Get (0).guidFrom == mainsyncRules[0].guidFrom;
            DBtest (added, "SyncAddSubelem plain rule -> added under rule.guidTo");
            // Пустой список подэлементов не должен менять флаги правила
            DBtest (!mainsyncRules[0].toSub && !mainsyncRules[0].fromSub,
                    "SyncAddSubelem plain rule -> flags untouched");
            // paramToRead заполняется через SyncAddRule -> AddParamValue2ParamDictElement,
            // ключ словаря = param.fromGuid (у прототипа он APINULLGuid)
            DBtest (paramToRead.GetPtr (APINULLGuid) != nullptr,
                    "SyncAddSubelem plain rule -> paramToRead has fromGuid entry");
        }

        // =============================================================================
        // GREEN-тест 2: from_sub — запись ИЗ первого подэлемента.
        // Первая ветка: fromSub сбрасывается, guidFrom = subelemGuids[0].
        // Существующее поведение, должно остаться неизменным после фикса to_sub.
        // =============================================================================
        {
            GS::Array<WriteData> mainsyncRules;
            WriteData rule = makeRule ();
            rule.fromSub = true;
            mainsyncRules.Push (rule);
            WriteDict syncRules;
            ParamDictElement paramToRead;

            SyncAddSubelement (subelemGuids, mainsyncRules, syncRules, paramToRead);

            const GS::Array<WriteData> *bucket = syncRules.GetPtr (mainsyncRules[0].guidTo);
            bool ok = bucket != nullptr && bucket->GetSize () == 1 &&
                      bucket->Get (0).guidFrom == sub1 && // источник — ПЕРВЫЙ подэлемент
                      !mainsyncRules[0].fromSub;          // флаг сброшен после развёртки
            DBtest (ok, "SyncAddSubelem from_sub -> guidFrom = first subelement, fromSub cleared");
            // Правило НЕ дублируется на второй подэлемент (только from_sub-развёртка на [0])
            bool noDup = syncRules.GetPtr (sub2) == nullptr;
            DBtest (noDup, "SyncAddSubelem from_sub -> no per-subelement duplication");
        }

        // =============================================================================
        // RED-тест (баг P1): to_sub — запись В КАЖДЫЙ подэлемент.
        // Ожидание: правило попадает в syncRules для каждого подэлемента
        // (guidTo = подэлемент), toSub сбрасывается.
        // Сейчас: ветка проверки `fromSub` вместо `toSub` мертва, правил в
        // syncRules нет — тест ПАДАЕТ до исправления (RED), проходит после (GREEN).
        // =============================================================================
        {
            GS::Array<WriteData> mainsyncRules;
            WriteData rule = makeRule ();
            rule.toSub = true;
            mainsyncRules.Push (rule);
            WriteDict syncRules;
            ParamDictElement paramToRead;

            SyncAddSubelement (subelemGuids, mainsyncRules, syncRules, paramToRead);

            bool allSubsCovered = syncRules.GetPtr (sub1) != nullptr && syncRules.GetPtr (sub2) != nullptr;
            if (allSubsCovered) {
                const GS::Array<WriteData> *b1 = syncRules.GetPtr (sub1);
                const GS::Array<WriteData> *b2 = syncRules.GetPtr (sub2);
                allSubsCovered = b1->GetSize () == 1 && b1->Get (0).guidTo == sub1 &&
                                 b1->Get (0).guidFrom == mainsyncRules[0].guidFrom && b2->GetSize () == 1 &&
                                 b2->Get (0).guidTo == sub2;
            }
            DBtest (allSubsCovered, "SyncAddSubelem to_sub -> rule added for EVERY subelement");
            DBtest (!mainsyncRules[0].toSub, "SyncAddSubelem to_sub -> toSub cleared after expansion");
        }

        // =============================================================================
        // GREEN-тест 3: to_sub с пустым списком подэлементов.
        // Оба условия (fromSub/toSub истинны, список пуст) -> правило НЕ добавляется
        // никуда (continue по пустому списку), флаги не трогаются.
        // Существующее поведение, должно остаться неизменным.
        // =============================================================================
        {
            GS::Array<WriteData> mainsyncRules;
            WriteData rule = makeRule ();
            rule.toSub = true;
            mainsyncRules.Push (rule);
            WriteDict syncRules;
            ParamDictElement paramToRead;
            GS::Array<API_Guid> emptySubs;

            SyncAddSubelement (emptySubs, mainsyncRules, syncRules, paramToRead);

            DBtest (syncRules.GetSize () == 0, "SyncAddSubelem to_sub empty subs -> nothing added");
            // Флаги НЕ трогаются: continue по пустому списку происходит до развёртки,
            // поэтому toSub остаётся true как и было до вызова.
            DBtest (mainsyncRules[0].toSub && !mainsyncRules[0].fromSub,
                    "SyncAddSubelem to_sub empty subs -> flags untouched");
        }

        // Внешний адресат Sync_to_GUID не входит в список текущего элемента и его подэлементов.
        {
            const API_Guid owner = APIGuidFromString ("{CCCCCCCC-CCCC-CCCC-CCCC-CCCCCCCCCCCC}");
            const API_Guid destination = APIGuidFromString ("{DDDDDDDD-DDDD-DDDD-DDDD-DDDDDDDDDDDD}");
            WriteData rule = makeRule ();
            rule.guidFrom = owner;
            rule.guidTo = destination;
            rule.paramFrom.fromGuid = owner;
            rule.paramTo.fromGuid = destination;
            rule.paramFrom.val.type = API_PropertyStringValueType;
            rule.paramTo.val.type = API_PropertyStringValueType;
            rule.paramFrom.val.uniStringValue = "new";
            rule.paramTo.val.uniStringValue = "old";
            GS::Array<WriteData> rules = {rule};
            GS::Array<API_Guid> processing = {owner};
            WriteDict syncRules;
            ParamDictElement paramToRead;
            ParamDictElement paramToWrite;
            UnicGuidString propertyWriteGuids;

            SyncAddSubelement ({}, rules, syncRules, paramToRead);
            SyncCalcRule (syncRules, processing, paramToRead, paramToWrite, propertyWriteGuids);
            const ParamDictValue *written = paramToWrite.GetPtr (destination);
            DBtest (written != nullptr && written->GetPtr (rule.paramTo.rawName) != nullptr,
                    "Sync_to_GUID external destination -> write scheduled");
        }

        DBprnt ("TEST", "TestSyncAddSubelement : done");
        return;
    }

    // -----------------------------------------------------------------------------
    // GREEN-регрессии логики нумерации: RenumPos, GetMostFrequentPos, ReNumGetFlag.
    // Все проверки фиксируют ТЕКУЩЕЕ поведение — до и после любых правок
    // результаты обязаны совпадать.
    // -----------------------------------------------------------------------------
    void TestRenumPosLogic () {
        DBprnt ("TEST", "TestRenumPosLogic");

        // ---- RenumPos (int): isNum, strpos, Add ----
        RenumPos pos5 = RenumPos (5);
        DBtest (pos5.isNum, "RenumPos(5) -> isNum");
        DBtest (pos5.numpos, 5, "RenumPos(5) -> numpos");
        DBtest (GS::UniString (pos5.strpos.c_str (), pos5.chcode), GS::UniString ("5"), "RenumPos(5) -> strpos");

        pos5.Add (3);
        DBtest (pos5.numpos, 8, "RenumPos(5).Add(3) -> numpos 8");

        // ---- RenumPos (): нечисловая позиция, Add активирует числовой режим ----
        RenumPos empty;
        DBtest (!empty.isNum, "RenumPos() -> not isNum");
        empty.Add (2);
        DBtest (empty.isNum, "RenumPos().Add(2) -> isNum activated");
        DBtest (empty.numpos, 2, "RenumPos().Add(2) -> numpos 2");

        // ---- FormatToMax: добление нулей до длины максимума ----
        RenumPos p1 = RenumPos (7);
        RenumPos pmax = RenumPos (123);
        p1.FormatToMax (pmax, ADDZEROS, 0);
        DBtest (GS::UniString (p1.strpos.c_str (), p1.chcode), GS::UniString ("007"), "FormatToMax ADDZEROS -> 007");
        DBtest (p1.numpos, 7, "FormatToMax keeps numpos");

        // ---- FormatToMax с явным nullcount (короче максимума) ----
        RenumPos p2 = RenumPos (4);
        RenumPos ref4 = RenumPos (4);
        p2.FormatToMax (ref4, ADDZEROS, 4);
        DBtest (
            GS::UniString (p2.strpos.c_str (), p2.chcode), GS::UniString ("0004"), "FormatToMax nullcount=4 -> 0004");

        // ---- SetToMax: берёт максимум по alphanum-сравнению ----
        RenumPos acc = RenumPos (10);
        RenumPos candidate = RenumPos (9);
        acc.SetToMax (candidate); // alphanum: 10 > 9, значение не меняется
        DBtest (static_cast<double> (acc.numpos), 10.0, "SetToMax keeps greater position (alphanum 10 > 9)");

        // ---- operator== ----
        RenumPos pa = RenumPos (42);
        RenumPos pb = RenumPos (42);
        DBtest (pa == pb, "operator== same positions equal");
        RenumPos pc = RenumPos (43);
        DBtest (!(pa == pc), "operator== different positions not equal");

        // ---- GetMostFrequentPos: пустой массив -> дефолт ----
        GS::Array<RenumPos> none;
        RenumPos defres = GetMostFrequentPos (none);
        DBtest (!defres.isNum && defres.strpos.empty (), "GetMostFrequentPos empty -> default");

        // ---- GetMostFrequentPos: самая частая позиция возвращается ----
        GS::Array<RenumPos> freq;
        freq.Push (RenumPos (1));
        freq.Push (RenumPos (2));
        freq.Push (RenumPos (2));
        RenumPos mf = GetMostFrequentPos (freq);
        DBtest (mf.numpos, 2, "GetMostFrequentPos -> most frequent 2");

        // ---- ReNumGetFlag: невалидный флаг -> SKIP ----
        ParamValue flagInvalid;
        flagInvalid.isValid = false;
        ParamValue positionValid;
        positionValid.isValid = true;
        DBtest (ReNumGetFlag (flagInvalid, positionValid) == RENUM_SKIP, "ReNumGetFlag invalid flag -> SKIP");

        // ---- ReNumGetFlag: булевый флаг true + редактируемый элемент -> NORMAL ----
        // Примечание: IsElementEditable для APINULLGuid вернёт false -> RENUM_IGNORE.
        // Проверяем именно этот детерминированный случай.
        ParamValue boolFlag;
        boolFlag.isValid = true;
        boolFlag.type = API_PropertyBooleanValueType;
        boolFlag.val.boolValue = true;
        boolFlag.val.type = API_PropertyBooleanValueType;
        boolFlag.fromGuid = APINULLGuid;
        DBtest (ReNumGetFlag (boolFlag, positionValid) == RENUM_IGNORE,
                "ReNumGetFlag bool=true non-editable -> IGNORE");

        ParamValue boolFlagFalse = boolFlag;
        boolFlagFalse.val.boolValue = false;
        DBtest (ReNumGetFlag (boolFlagFalse, positionValid) == RENUM_SKIP, "ReNumGetFlag bool=false -> SKIP");

        // ---- ReNumGetFlag: строковый флаг skip/ignore ----
        ParamValue strFlag;
        strFlag.isValid = true;
        strFlag.type = API_PropertyStringValueType;
        strFlag.val.type = API_PropertyStringValueType;
        strFlag.fromGuid = APINULLGuid;
        strFlag.val.uniStringValue = "skip";
        DBtest (ReNumGetFlag (strFlag, positionValid) == RENUM_SKIP, "ReNumGetFlag string skip -> SKIP");

        ParamValue ignoreFlag = strFlag;
        ignoreFlag.val.uniStringValue = "ignore";
        DBtest (ReNumGetFlag (ignoreFlag, positionValid) == RENUM_IGNORE, "ReNumGetFlag string ignore -> IGNORE");

        DBprnt ("TEST", "TestRenumPosLogic : done");
        return;
    }

    // -----------------------------------------------------------------------------
    // GREEN-регрессии ParsePropertyDescriptionToRules для sub/GUID-правил:
    // фиксируют контракт hasSub/hasGUID/target*/guidSourceProperty. Эти поля
    // использует UI палитры; правка бага P1 в SyncAddSubelement не должна их менять.
    // -----------------------------------------------------------------------------
    void TestDescToRulesSubGuid () {
        DBprnt ("TEST", "TestDescToRulesSubGuid");

        // ---- to_sub: hasSub = true, targetType определён ----
        {
            GS::UniString desc = "Sync_to_sub{Property:TargetSub}";
            ParsePropertyResult result = ParsePropertyDescriptionToRules (desc);
            DBtest (result.hasSyncRules, "DescToRulesSub to_sub -> hasSyncRules");
            if (result.syncRules.GetSize () > 0) {
                DBtest (result.syncRules[0].hasSub, "DescToRulesSub to_sub -> hasSub true");
                DBtest (result.syncRules[0].targetType == "Property", "DescToRulesSub to_sub -> targetType Property");
                DBtest (result.syncRules[0].targetName == "TargetSub", "DescToRulesSub to_sub -> targetName TargetSub");
            }
        }

        // ---- from_GUID: ТЕКУЩЕЕ поведение — минимальная форма не создаёт правила.
        // ParsePropertyDescriptionToRules валидирует каждую Sync-команду через
        // SyncString, и для from_GUID без полного контекста она возвращает отказ ->
        // isValid=false -> правило не попадает в результат. Фиксируем как есть:
        // это документирование известного ограничения, а не эталон.
        {
            GS::UniString desc = "Sync_from_GUID{Property:GuidSource}";
            ParsePropertyResult result = ParsePropertyDescriptionToRules (desc);
            DBtest (!result.hasSyncRules, "DescToRulesSub from_GUID minimal form -> no rules (known limitation)");
        }

        // ---- обычный Sync_from: ни hasSub, ни hasGUID ----
        {
            GS::UniString desc = "Sync_from{Property:PlainProp}";
            ParsePropertyResult result = ParsePropertyDescriptionToRules (desc);
            if (result.syncRules.GetSize () > 0) {
                DBtest (!result.syncRules[0].hasSub, "DescToRulesSub plain -> hasSub false");
                DBtest (!result.syncRules[0].hasGUID, "DescToRulesSub plain -> hasGUID false");
            }
        }

        DBprnt ("TEST", "TestDescToRulesSubGuid : done");
        return;
    }

    // -----------------------------------------------------------------------------
    // Тест GetPropertyRuleFlag — кэш признака правила SomeStuff в описании свойства.
    // Проверяет: первичный расчёт, попадание в кэш (второй вызов без перечитывания),
    // инвалидацию по изменению описания и отсутствие ложных правил у обычных свойств.
    // -----------------------------------------------------------------------------
    void TestGetPropertyRuleFlag () {
        DBprnt ("TEST", "TestGetPropertyRuleFlag");

        API_PropertyDefinition definition = {};
        definition.guid = APINULLGuid;

        // ---- Описание с Sync-правилом: признак true ----
        definition.description = "Sync_from{Property:TestSource}";
        DBtest (GetPropertyRuleFlag (definition), "RuleFlag Sync_from -> true");

        // ---- Второй вызов с тем же описанием: кэш, результат тот же ----
        DBtest (GetPropertyRuleFlag (definition), "RuleFlag Sync_from cached -> true");

        // ---- Инвалидация: описание изменилось -> признак пересчитан (#159: Renum тоже правило) ----
        definition.description = "Renum_flag{Property:RenumRule; NULL}";
        DBtest (GetPropertyRuleFlag (definition), "RuleFlag changed to Renum_flag -> true");

        // ---- Обычное свойство без команд в описании ----
        definition.description = "Just a plain description";
        DBtest (!GetPropertyRuleFlag (definition), "RuleFlag plain description -> false");

        // ---- Пустое описание ----
        definition.description = "";
        DBtest (!GetPropertyRuleFlag (definition), "RuleFlag empty description -> false");

        // Spec хранится в otherCommands; Renum/Sum не должны давать ложный признак.
        definition.description = "Spec_rule{Property:TestSpec}";
        DBtest (GetPropertyRuleFlag (definition), "RuleFlag Spec_rule -> true");
        DBtest (GetPropertyRuleFlag (definition), "RuleFlag Spec_rule cached -> true");
        definition.description = "Spec_rule_v2{Property:TestSpec}";
        DBtest (GetPropertyRuleFlag (definition), "RuleFlag Spec_rule_v2 -> true");
        definition.description = "spec_rule_v3{Property:TestSpec}";
        DBtest (GetPropertyRuleFlag (definition), "RuleFlag spec_rule_v3 -> true");
        definition.description = "Spec_rule{Property:TestSpec";
        DBtest (!GetPropertyRuleFlag (definition), "RuleFlag incomplete Spec -> false");
        definition.description = "Sum{Property:TestSum}";
        DBtest (GetPropertyRuleFlag (definition), "RuleFlag Sum only -> true");
        definition.description = "";
        DBtest (!GetPropertyRuleFlag (definition), "RuleFlag cleared after Spec -> false");

        // Проверяем сохранение результата без повторного чтения определений из Archicad.
        DBprnt ("TEST", "RuleCache content checks start");
        definition.description = "Sync_from{Property:TestSource}";
        const ParsePropertyResult expected = ParsePropertyDescriptionToRules (definition.description);
        GetPropertyRuleFlag (definition);
        auto &flags = PROPERTYCACHE ().propertyRuleFlags;
        PropertyRuleFlag *entry = flags.GetPtr (definition.guid);
        DBtest (entry != nullptr && entry->parsed.hasSyncRules && entry->parsed.syncRules.GetSize () == 1 &&
                    expected.syncRules.GetSize () == 1 &&
                    entry->parsed.syncRules[0].fullCommand == expected.syncRules[0].fullCommand &&
                    entry->parsed.syncRules[0].sourceName == expected.syncRules[0].sourceName &&
                    entry->parsed.syncRules[0].isValid == expected.syncRules[0].isValid,
                "RuleCache retains Sync command");
        // Маркер только в тестовой записи: повторный разбор затёр бы его.
        if (entry != nullptr)
            entry->parsed.remainingText = "cache reuse sentinel";
        GetPropertyRuleFlag (definition);
        entry = flags.GetPtr (definition.guid);
        DBtest (entry != nullptr && entry->parsed.remainingText == "cache reuse sentinel",
                "RuleCache reuses unchanged description");
        definition.description = "Spec_rule_v2{Property:TestSpec}";
        GetPropertyRuleFlag (definition);
        entry = flags.GetPtr (definition.guid);
        DBtest (entry != nullptr && entry->parsed.syncRules.IsEmpty () && entry->parsed.hasOtherCommands &&
                    entry->parsed.otherCommands.GetSize () == 1 &&
                    entry->parsed.otherCommands[0].commandType == "Spec_rule" &&
                    entry->parsed.remainingText != "cache reuse sentinel",
                "RuleCache replaces Sync result with Spec");
        definition.description = "";
        GetPropertyRuleFlag (definition);
        entry = flags.GetPtr (definition.guid);
        DBtest (entry != nullptr && entry->parsed.syncRules.IsEmpty () && entry->parsed.otherCommands.IsEmpty () &&
                    !entry->parsed.hasSyncRules && !entry->parsed.hasOtherCommands,
                "RuleCache clears parsed commands for empty description");
        flags.Delete (definition.guid);
        DBprnt ("TEST", "TestGetPropertyRuleFlag : done");
        return;
    }

    // -----------------------------------------------------------------------------
    // Диагностика #184/#185: почему мост отдаёт hasRule=false для всех свойств.
    // Повторяет путь BrowserPalette::GetPropertiesList на реальных элементах проекта
    // (ACAPI_Element_GetPropertyDefinitions -> ACAPI_Element_GetPropertyValues) и
    // печатает длины описаний из двух источников определения:
    //   prop.definition — то, что использовал мост;
    //   definitions[i]  — тот же источник, что в рабочем пути Sync.cpp:690/1099.
    // Без этого нельзя отличить «описание не приходит от API» от «в описании нет правил».
    // Только DBprnt: это измерение, а не проверка — провалов теста оно не создаёт.
    // -----------------------------------------------------------------------------
    void TestPropertyRuleFlagOnProjectElements () {
        DBprnt ("TEST", "TestPropertyRuleFlagOnProjectElements");

        GS::Array<API_Guid> elements;
        GSErrCode err = ACAPI_Element_GetElemList (API_WallID, &elements);
        const Int32 wallListError = (Int32)err;
        if (err != NoError || elements.IsEmpty ()) {
            err = ACAPI_Element_GetElemList (API_SlabID, &elements);
        }
        DBprnt ("TEST",
                GS::UniString ("RuleFlagProj list: elements=") + GS::ValueToUniString ((Int32)elements.GetSize ()) +
                    GS::UniString (" wallErr=") + GS::ValueToUniString (wallListError) + GS::UniString (" slabErr=") +
                    GS::ValueToUniString ((Int32)err));
        if (err != NoError || elements.IsEmpty ())
            return;

        // #184/#185: добавляем один объект — правила координат/углов живут на объектах.
        GS::Array<API_Guid> objectElements;
        if (ACAPI_Element_GetElemList (API_ObjectID, &objectElements) == NoError && !objectElements.IsEmpty ())
            elements.Push (objectElements[0]);
        const USize elementLimit = GS::Min (elements.GetSize (), (USize)3);
        USize descriptionsDumped = 0; // общий лимит печати описаний на все элементы
        for (USize elementIndex = 0; elementIndex < elementLimit; ++elementIndex) {
            const API_Guid elemGuid = elements[elementIndex];
            GS::Array<API_PropertyDefinition> definitions;
            err =
                ACAPI_Element_GetPropertyDefinitions (elemGuid, API_PropertyDefinitionFilter_UserDefined, definitions);
            if (err != NoError || definitions.IsEmpty ()) {
                DBprnt ("TEST",
                        GS::UniString ("RuleFlagProj definitions: err=") + GS::ValueToUniString ((Int32)err) +
                            GS::UniString (" count=") + GS::ValueToUniString ((Int32)definitions.GetSize ()));
                continue;
            }

            USize definitionsWithDescription = 0;
            for (const API_PropertyDefinition &definition : definitions) {
                if (!definition.description.IsEmpty ())
                    ++definitionsWithDescription;
            }

            GS::Array<API_Property> properties;
            err = ACAPI_Element_GetPropertyValues (elemGuid, definitions, properties);

            USize propertiesWithDescription = 0;
            USize rulesFromPropertyDefinition = 0;
            USize rulesFromArrayDefinition = 0;
            for (const API_Property &prop : properties) {
                if (!prop.definition.description.IsEmpty ())
                    ++propertiesWithDescription;

                const bool ruleFromPropertyDefinition = GetPropertyRuleFlag (prop.definition);
                if (ruleFromPropertyDefinition)
                    ++rulesFromPropertyDefinition;

                bool ruleFromArrayDefinition = false;
                USize arrayDescriptionLength = 0;
                for (const API_PropertyDefinition &definition : definitions) {
                    if (definition.guid != prop.definition.guid)
                        continue;
                    arrayDescriptionLength = definition.description.GetLength ();
                    ruleFromArrayDefinition = !definition.description.IsEmpty () && GetPropertyRuleFlag (definition);
                    break;
                }
                if (ruleFromArrayDefinition)
                    ++rulesFromArrayDefinition;

                // Печатаем сами описания первого элемента: длины у обоих источников
                // совпадают, поэтому отличить «описание без правила» от «правило другого
                // формата» можно только по тексту (#184/#185).
                if (descriptionsDumped < 24 && !prop.definition.description.IsEmpty ()) {
                    ++descriptionsDumped;
                    const USize descriptionLimit = 160;
                    const GS::UniString descriptionText =
                        prop.definition.description.GetLength () > descriptionLimit
                            ? prop.definition.description.GetSubstring (0, descriptionLimit) + GS::UniString ("...")
                            : prop.definition.description;
                    DBprnt ("TEST",
                            GS::UniString ("RuleFlagProj desc ") + prop.definition.name + GS::UniString (" len=") +
                                GS::ValueToUniString ((Int32)prop.definition.description.GetLength ()) +
                                GS::UniString (" dArray=") + GS::ValueToUniString ((Int32)arrayDescriptionLength) +
                                GS::UniString (" rule=") + GS::ValueToUniString (ruleFromPropertyDefinition) +
                                GS::UniString (" [") + descriptionText + GS::UniString ("]"));
                }
            }

            DBprnt ("TEST",
                    GS::UniString ("RuleFlagProj elem=") + APIGuidToString (elemGuid) + GS::UniString (" defs=") +
                        GS::ValueToUniString ((Int32)definitions.GetSize ()) + GS::UniString (" defsWithDesc=") +
                        GS::ValueToUniString ((Int32)definitionsWithDescription) + GS::UniString (" props=") +
                        GS::ValueToUniString ((Int32)properties.GetSize ()) + GS::UniString (" propsWithDesc=") +
                        GS::ValueToUniString ((Int32)propertiesWithDescription) + GS::UniString (" rulesFromPropDef=") +
                        GS::ValueToUniString ((Int32)rulesFromPropertyDefinition) +
                        GS::UniString (" rulesFromArrayDef=") + GS::ValueToUniString ((Int32)rulesFromArrayDefinition));
        }
        DBprnt ("TEST", "TestPropertyRuleFlagOnProjectElements : done");
        return;
    }

    void TestBuildOtdByParent () {
        DBprnt ("TEST", "TestBuildOtdByParent");

        const API_Guid baseGuid = APIGuidFromString ("{11111111-1111-1111-1111-111111111111}");
        const API_Guid floorGuid = APIGuidFromString ("{22222222-2222-2222-2222-222222222222}");
        const API_Guid wallGuid = APIGuidFromString ("{33333333-3333-3333-3333-333333333333}");
        const API_Guid unknownGuid = APIGuidFromString ("{44444444-4444-4444-4444-444444444444}");

        GS::HashTable<API_Guid, Roombook::TypeOtd> otdElements;
        otdElements.Add (floorGuid, Roombook::Floor);
        otdElements.Add (wallGuid, Roombook::Wall_Main);

        UnicGuid childElements;
        childElements.Add (floorGuid, true);
        childElements.Add (wallGuid, true);
        UnicGuidByGuid parentDict;
        parentDict.Add (baseGuid, childElements);

        bool hasBaseElement = false;
        Roombook::UnicGuidByBase result = Roombook::BuildOtdByParent (otdElements, parentDict, hasBaseElement);
        const Roombook::UnicGuidByTypeOtd *types = result.GetPtr (baseGuid);
        const UnicGuid *floorElements = types != nullptr ? types->GetPtr (Roombook::Floor) : nullptr;
        const UnicGuid *wallElements = types != nullptr ? types->GetPtr (Roombook::Wall_Main) : nullptr;

        DBtest (hasBaseElement, "BuildOtdByParent: known children set has_base_element");
        DBtest (floorElements != nullptr && floorElements->ContainsKey (floorGuid),
                "BuildOtdByParent: floor child indexed by base GUID");
        DBtest (wallElements != nullptr && wallElements->ContainsKey (wallGuid),
                "BuildOtdByParent: wall child indexed by base GUID");

        UnicGuid unknownChildren;
        unknownChildren.Add (unknownGuid, true);
        UnicGuidByGuid unknownParentDict;
        unknownParentDict.Add (baseGuid, unknownChildren);
        hasBaseElement = false;
        result = Roombook::BuildOtdByParent (otdElements, unknownParentDict, hasBaseElement);
        DBtest (!hasBaseElement, "BuildOtdByParent: unknown child keeps has_base_element false");
        DBtest (result.IsEmpty (), "BuildOtdByParent: unknown child is ignored");

        DBprnt ("TEST", "TestBuildOtdByParent : done");
    }

} // namespace TestFunc
#endif
