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

    // -----------------------------------------------------------------------------
    // Накопитель результата запуска и имя правила.
    //
    // Окно результата ничего не вычисляет: оно показывает то, что накопил запуск.
    // Поэтому набор закрывает именно накопитель - то, что без него окно показало
    // бы пустоту или, что хуже, привязала бы ошибку не к той строке.
    //
    // Чего набор НЕ проверяет: сбор данных в ходе реального запуска. Он идёт
    // после обращения к модели (ReNumOneRule), а фикстуры модели здесь нет.
    // Проверяется контракт структуры, решения накопителя и разбор имени правила.
    // -----------------------------------------------------------------------------
    void TestRenumRunReport () {
        // 1. Пустой результат: ни строк, ни сообщений. Окно по такому не
        // показывается вовсе - ShowRenumResult проверяет это перед показом.
        {
            RenumRunResult result;
            DBtest (result.ruleNames.IsEmpty (), true, "no rules in empty result");
            DBtest (result.messages.IsEmpty (), true, "no messages in empty result");
            DBtest (result.elementsToWrite, 0u, "no written elements in empty result");
        }

        // 2. Повторная регистрация правила не создаёт второй строки: окно строит
        // строки по индексам, и дубль дал бы две строки об одном правиле плюс
        // рассинхронизацию с именами.
        {
            RenumRunResult result;
            const UIndex first = result.EnsureRuleStats ("RuleA");
            const UIndex second = result.EnsureRuleStats ("RuleA");
            DBtest (first, second, "repeat registration returns same index");
            DBtest (result.ruleNames.GetSize (), 1u, "repeat registration keeps one row");
            DBtest (result.ruleStats.GetSize (), 1u, "stats array matches names");
        }

        // 3. Порядок регистрации сохраняется: иначе строки окна приходили бы в
        // произвольном порядке между запусками.
        {
            RenumRunResult result;
            result.EnsureRuleStats ("First");
            result.EnsureRuleStats ("Second");
            result.EnsureRuleStats ("Third");
            DBtest (result.ruleNames.GetSize (), 3u, "three rules registered");
            DBtest (GS::UniString (result.ruleNames[0]), GS::UniString ("First"), "registration order 0");
            DBtest (GS::UniString (result.ruleNames[1]), GS::UniString ("Second"), "registration order 1");
            DBtest (GS::UniString (result.ruleNames[2]), GS::UniString ("Third"), "registration order 2");
        }

        // 4. Счётчики пишутся по индексу правила и не путаются между правилами:
        // переуказка индекса переносила бы числа чужой строке.
        {
            RenumRunResult result;
            const UIndex a = result.EnsureRuleStats ("RuleA");
            const UIndex b = result.EnsureRuleStats ("RuleB");
            result.ruleStats[a].elements = 12;
            result.ruleStats[a].written = 5;
            result.ruleStats[b].elements = 3;
            result.ruleStats[b].written = 0;
            DBtest (result.ruleStats[a].elements, 12u, "rule A elements kept");
            DBtest (result.ruleStats[a].written, 5u, "rule A written kept");
            DBtest (result.ruleStats[b].elements, 3u, "rule B elements kept");
            DBtest (result.ruleStats[b].written, 0u, "rule B written untouched");
        }

        // 5. Счётчики НАКАПЛИВАЮТСЯ при повторной регистрации: одно правило может
        // быть описано несколькими свойствами, и строка обязана показывать их
        // объединённый итог, а не последнего.
        {
            RenumRunResult result;
            const UIndex a = result.EnsureRuleStats ("RuleA");
            result.ruleStats[a].elements += 4;
            result.ruleStats[a].written += 2;
            const UIndex b = result.EnsureRuleStats ("RuleA");
            result.ruleStats[b].elements += 6;
            result.ruleStats[b].written += 1;
            DBtest (result.ruleNames.GetSize (), 1u, "same rule keeps one row");
            DBtest (result.ruleStats[0].elements, 10u, "elements accumulate across properties");
            DBtest (result.ruleStats[0].written, 3u, "written accumulate across properties");
        }

        // 6. Сообщение без правила не создаёт строки: у него нет правила,
        // которому оно соответствовало бы, и строка была бы пустой.
        {
            RenumRunResult result;
            result.AddGeneralMessage ("Rules not found");
            DBtest (result.ruleNames.IsEmpty (), true, "general message adds no rule row");
            DBtest (result.messages.GetSize (), 1u, "general message stored");
            DBtest (GS::UniString (result.messages[0].ruleName).IsEmpty (), true, "general message has no rule");
            DBtest (GS::UniString (result.messages[0].text),
                    GS::UniString ("Rules not found"),
                    "general message keeps text");
        }

        // 7. Сообщение по правилу регистрирует правило само: вызывающий не обязан
        // объявлять участие правила отдельным вызовом, иначе правило с ошибкой
        // осталось бы без строки, то есть без подсветки.
        {
            RenumRunResult result;
            result.AddRuleMessage ("RuleA", "Can't find property: этаж");
            DBtest (result.ruleNames.GetSize (), 1u, "rule message registers its rule");
            DBtest (GS::UniString (result.messages[0].ruleName), GS::UniString ("RuleA"), "message carries rule name");
            DBtest (result.HasRuleError (0), true, "rule with message is marked as error");
        }

        // 8. Подсветка определяется сообщением, а не счётчиками: нули в колонках
        // при отказе подготовки - законное состояние, и по ним ошибку не увидеть.
        {
            RenumRunResult result;
            const UIndex ok = result.EnsureRuleStats ("Good");
            const UIndex bad = result.EnsureRuleStats ("Bad");
            result.ruleStats[bad].elements = 7;
            result.ruleStats[bad].written = 0;
            result.AddRuleMessage ("Bad", "Skipped due to property errors - 3");
            DBtest (result.HasRuleError (ok), false, "silent rule is not marked");
            DBtest (result.HasRuleError (bad), true, "rule with message is marked");
        }

        // 9. Граница индекса: индекс вне диапазона даёт "без ошибки", а не выход за
        // границу. Проверяется именно выразимость состояния.
        {
            RenumRunResult result;
            result.EnsureRuleStats ("Only");
            DBtest (result.HasRuleError (99), false, "index beyond range is safe");
        }

        // 10. Порядок сообщений сохраняется, и типы сообщений различимы по имени
        // правила: окно разводит их по привязке, а читатель обязан различить те же
        // два случая.
        {
            RenumRunResult result;
            result.AddGeneralMessage ("first general");
            result.AddRuleMessage ("RuleA", "rule message");
            result.AddGeneralMessage ("second general");
            DBtest (result.messages.GetSize (), 3u, "all messages kept in order");
            DBtest (GS::UniString (result.messages[0].text), GS::UniString ("first general"), "message 0 order");
            DBtest (GS::UniString (result.messages[1].ruleName), GS::UniString ("RuleA"), "message 1 has rule");
            DBtest (GS::UniString (result.messages[2].text), GS::UniString ("second general"), "message 2 order");
        }

        // 11. Сброс структуры уносит и сообщения, и строки, и итог: запуски идут
        // один за другим, и данные прошлого запуска были бы ложью.
        {
            RenumRunResult result;
            result.AddRuleMessage ("RuleA", "stale message");
            result.EnsureRuleStats ("RuleA");
            result.ruleStats[0].elements = 9;
            result.ruleStats[0].written = 4;
            result.elementsToWrite = 4;
            result = {};
            DBtest (result.messages.IsEmpty (), true, "reset clears messages");
            DBtest (result.ruleNames.IsEmpty (), true, "reset clears rule rows");
            DBtest (result.elementsToWrite, 0u, "reset clears run total");
        }

        // 12. Отсутствующие свойства копятся ПО ПРАВИЛУ, и повтор под одним и тем
        // же именем игнорируется: одно свойство может отсутствовать у многих
        // элементов, а пользователю нужен один список.
        {
            RenumMissingProps missing = {};
            ParamDictValue props = {};
            ParamHelpers::AddValueToParamDictValue (props, "{@property:этаж}");
            AddMissingProp (missing, "Позиция", "{@property:квартал}", EMPTYSTRING, props);
            AddMissingProp (missing, "Позиция", "{@property:квартал}", EMPTYSTRING, props);        // повтор
            AddMissingProp (missing, "Позиция", "{@property:этаж}", EMPTYSTRING, props);           // есть в кэше
            AddMissingProp (missing, "Позиция", "{@property:метр}", GS::UniString ("%A%"), props); // формула
            AddMissingProp (missing, "Позиция", EMPTYSTRING, EMPTYSTRING, props);                  // пустое имя
            AddMissingProp (missing, "Раздел", "{@property:квартал}", EMPTYSTRING, props);
            DBtest (missing.GetSize (), 2u, "missing props grouped by rule");
            const GS::Array<GS::UniString> *list = missing.GetPtr ("Позиция");
            DBtest (list != nullptr, true, "missing props stored for rule");
            if (list != nullptr) {
                DBtest (list->GetSize (), 1u, "duplicate/known/formula/empty names not added");
                // Сырое имя сохраняется как есть: читаемый вид получается при
                // сборке сообщения, иначе пришлось бы хранить два имени.
                DBtest (GS::UniString ((*list)[0]), GS::UniString ("{@property:квартал}"), "raw name kept as given");
            }
            // Отсутствие относится к ПРАВИЛУ, а не к элементу: то же свойство
            // под другим правилом попадает в другой список.
            const GS::Array<GS::UniString> *other = missing.GetPtr ("Раздел");
            DBtest (other != nullptr && other->GetSize () == 1u, true, "same property missing for another rule");
        }

        // 13. Имя правила в окне - текст между первой парой скобок описания
        // свойства-флага. Оно вычисляется ДО разбора на годность, поэтому и
        // повреждённое описание должно давать пригодное имя.
        {
            GS::UniString name = RenumRuleDisplayName ("Renum_flag{Property:Позиция}");
            DBtest (name, GS::UniString ("Позиция"), "rule name from Renum_flag description");
            name = RenumRuleDisplayName ("Свойство: Renum_flag{Позиция}");
            DBtest (name, GS::UniString ("Позиция"), "rule name ignores leading text");
            // Часть после «;» (флаги нулей) отбрасывается: имя берётся МЕЖДУ
            // первой парой скобок. Это прежнее поведение, на котором построен
            // диалог выбора правил - одно свойство с разными флагами нулей
            // даёт одну строку, а не несколько.
            name = RenumRuleDisplayName ("Renum_flag{Позиция; null_3}");
            DBtest (name, GS::UniString ("Позиция"), "rule name stops at first semicolon");
            // Без фигурных скобок обрезать нечего, и GetSubstring даёт пустую
            // строку. Это тоже прежнее поведение (та же строка в диалоге), и
            // пустое имя не опасно: сообщение уходит вниз окна без привязки к
            // строке списка.
            name = RenumRuleDisplayName ("Renum_flag без скобок");
            DBtest (name.IsEmpty (), true, "rule name without braces is empty");
        }

        // 14. Читаемое имя свойства: служебные приставки убираются целиком.
        {
            DBtest (
                RenumReadableName ("{@property:этаж}"), GS::UniString ("этаж"), "readable name drops property prefix");
            DBtest (RenumReadableName ("{@gdl:тип}"), GS::UniString ("тип"), "readable name drops gdl prefix");
            // Двоеточие внутри имени сохраняется: у формул оно часть самого
            // имени, и его удаление сделало бы имя нечитаемым.
            DBtest (RenumReadableName ("{@formula:renum_criteria;%A%}"),
                    GS::UniString ("renum_criteria;%A%"),
                    "readable name keeps inner colon");
            DBtest (
                RenumReadableName ("{@unknown:имя}"), GS::UniString ("unknown:имя"), "unknown prefix keeps its text");
            DBtest (RenumReadableName (EMPTYSTRING), EMPTYSTRING, "empty readable name stays empty");
        }

        return;
    }

} // namespace TestFunc
#endif
