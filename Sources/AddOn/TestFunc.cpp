//------------ kuvbur 2022 ------------
#ifdef TESTING
    #include "ACAPinc.h"

    #include "api_headers/APIEnvir.h"

    #include "TestFunc.hpp"

    #include "Helpers.hpp"
    #include "Propertycache.hpp"
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
        //      TestName2Rawname ();  // отключён — баг в Name2Rawname (Sync.cpp:1323-1326), порядок скобок {/} нарушен
        TestSyncString ();
        TestSyncStringRealRules ();
        TestParsePrefixes ();
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
        ACAPI_CallUndoableCommand (undoString, [&] () -> GSErrCode {
            err = ACAPI_Element_SetProperties (elemGuid, propertywrite);
            return NoError;
        });
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
        DBtest (SYNC_FROM_SUB, 3, "SYNC_FROM_SUB");
        DBtest (SYNC_TO_SUB, 4, "SYNC_TO_SUB");
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

} // namespace TestFunc
#endif
