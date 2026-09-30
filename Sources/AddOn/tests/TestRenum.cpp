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
    // GREEN-регрессии логики нумерации: RenumPos, GetMostFrequentPos, ReNumGetFlag.
    // Все проверки фиксируют ТЕКУЩЕЕ поведение — до и после любых правок
    // результаты обязаны совпадать.
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

} // namespace TestFunc
#endif
