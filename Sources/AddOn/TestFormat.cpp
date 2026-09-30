//------------ kuvbur 2022 ------------
#ifdef TESTING
    #include "ACAPinc.h"

    #include "api_headers/APIEnvir.h"

    #include "dialogs/CommandHelpers.hpp"
    #include "Helpers.hpp"
    #include "Propertycache.hpp"
    #include "TestFunc.hpp"
    #include "TestKit.hpp"

namespace TestFunc {

    void TestCalc () {
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

} // namespace TestFunc
#endif
