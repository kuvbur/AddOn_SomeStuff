//------------ kuvbur 2022 ------------
#ifdef TESTING
    #include "ACAPinc.h"

    #include "api_headers/APIEnvir.h"

    #include "Helpers.hpp"
    #include "ReNum.hpp"
    #include "tests/TestFunc.hpp"
    #include "tests/TestKit.hpp"

namespace TestFunc {

    // -----------------------------------------------------------------------------
    // Регрессии логики нумерации: RenumPos, GetMostFrequentPos, ReNumGetFlag.
    // Все проверки фиксируют зафиксированное поведение: правки не должны
    // менять результат.
    // -----------------------------------------------------------------------------
    void TestRenumPosLogic () {
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

        return;
    }

    // -----------------------------------------------------------------------------
    // Разбор формулы в Renum{...}: детектор - двойные кавычки, всё между ними -
    // шаблон. Регистр литерала сохраняется, имя свойства между процентами
    // приводится к нижнему регистру (ReplaceProcToBrace).
    // -----------------------------------------------------------------------------
    void TestRenumFormulaParse () {
        GS::UniString templatestring;

        // ---- обычная часть без кавычек - это свойство, не формула ----
        DBtest (!GetFormulaTemplate (GS::UniString ("Property:Этаж"), templatestring),
                "GetFormulaTemplate no quotes -> not a formula");
        DBtest (templatestring.IsEmpty (), "GetFormulaTemplate no quotes -> output cleared");

        // ---- непарные кавычки - трактуем как обычное свойство ----
        DBtest (!GetFormulaTemplate (GS::UniString ("\"%Помещение%x<%Этаж%>"), templatestring),
                "GetFormulaTemplate unpaired quote -> not a formula");

        // ---- пустые кавычки - не формула (иначе пустой критерий у всех) ----
        DBtest (!GetFormulaTemplate (GS::UniString ("\"\""), templatestring),
                "GetFormulaTemplate empty quotes -> not a formula");

        // ---- обычная формула: шаблон вырезается целиком ----
        DBtest (GetFormulaTemplate (GS::UniString ("\"%Помещение%x<%Этаж%-%h%>\""), templatestring),
                "GetFormulaTemplate plain formula -> true");
        DBtest (
            templatestring, GS::UniString ("%Помещение%x<%Этаж%-%h%>"), "GetFormulaTemplate plain formula -> value");

        // ---- берётся первая пара кавычек, вторая - литерал ----
        DBtest (GetFormulaTemplate (GS::UniString ("\"%A%\" + \"%B%\""), templatestring),
                "GetFormulaTemplate two pairs -> true");
        DBtest (templatestring, GS::UniString ("%A%"), "GetFormulaTemplate two pairs -> first pair only");

        // ---- пробелы вокруг шаблона отбрасываются ----
        DBtest (GetFormulaTemplate (GS::UniString (" \" %A% \" "), templatestring),
                "GetFormulaTemplate spaces around quotes -> true");
        DBtest (templatestring, GS::UniString ("%A%"), "GetFormulaTemplate spaces around quotes -> trimmed");

        // ---- имя параметра-формулы: роли различаются, шаблон входит в имя ----
        GS::UniString critName = RenumFormulaName (RenumPart::Criteria);
        GS::UniString delimName = RenumFormulaName (RenumPart::Delimetr);
        DBtest (critName != delimName, "RenumFormulaName criteria != delimetr");

        GS::UniString rawOne = GetFormulaRawName (critName, GS::UniString ("%A%"));
        GS::UniString rawTwo = GetFormulaRawName (critName, GS::UniString ("%B%"));
        GS::UniString rawDelim = GetFormulaRawName (delimName, GS::UniString ("%A%"));
        // Разные шаблоны одного правила не должны слиться в одно имя
        DBtest (rawOne != rawTwo, "GetFormulaRawName different templates -> different names");
        // Одинаковый шаблон критерия и разбивки тоже не должен слиться
        DBtest (rawOne != rawDelim, "GetFormulaRawName same template different role -> different names");
        // Имя начинается с префикса формулы и закрывается скобкой - по нему
        // GetTypeInxByRawnamePrefix определяет FORMULATYPEINX
        DBtest (rawOne.BeginsWith (FORMULANAMEPREFIX), "GetFormulaRawName begins with FORMULANAMEPREFIX");
        DBtest (rawOne.EndsWith (BRACEEND), "GetFormulaRawName ends with BRACEEND");
        DBtest (ParamHelpers::GetTypeInxByRawnamePrefix (rawOne) == FORMULATYPEINX,
                "GetFormulaRawName resolves to FORMULATYPEINX");

        // ---- разбор зависимостей шаблона: %имя% -> {@property:имя} ----
        // ReplaceProcToBrace переписывает ВСЕ пары %…%, в том числе %h% внутри
        // <…>: «h» здесь - обычное свойство, как и в Sync/Spec.
        ParamDictValue deps = {};
        GS::UniString formula = GS::UniString ("%Помещение%x<%Этаж%-%h%>");
        DBtest (ParamHelpers::ParseParamNameMaterial (formula, deps), "ParseParamNameMaterial formula");
        // Имя свойства между процентами уходит в нижний регистр (ключи словаря тоже)
        DBtest (deps.ContainsKey ("{@property:помещение}"), "ParseParamNameMaterial adds criteria dependency");
        DBtest (deps.ContainsKey ("{@property:этаж}"), "ParseParamNameMaterial adds delimetr dependency");
        DBtest (deps.ContainsKey ("{@property:h}"), "ParseParamNameMaterial rewrites % inside formula");
        // Литерал шаблона не приводится к нижнему регистру
        DBtest (formula,
                GS::UniString ("{@property:помещение}x<{@property:этаж}-{@property:h}>"),
                "ParseParamNameMaterial keeps literal case");

        // ---- шаблон без зависимостей: разбирать нечего, значение остаётся шаблоном ----
        ParamDictValue nodeps = {};
        GS::UniString plain = GS::UniString ("Помещение");
        DBtest (!ParamHelpers::ParseParamNameMaterial (plain, nodeps),
                "ParseParamNameMaterial plain text -> nothing parsed");

        return;
    }

} // namespace TestFunc
#endif
