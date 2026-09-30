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
    #include "spec/Spec.hpp"
    #include "Sync.hpp"

namespace TestFunc {

    void TestStringSplt (); // forward declaration
    void TestSpecRegression ();

    // Тег функции в квадратных скобках (#230): печать и проверки помечаются именем
    // функции-владельца, чтобы в панели «Отладка» их можно было отфильтровать:
    //   ... [TestSpecMergeAndKey] test Spec merge sums add up ok ...
    // Тег добавляется к СОДЕРЖАТЕЛЬНОМУ тексту (msg у DBprnt, reportString у
    // DBtest), а не к выводимым значениям, поэтому он печатается в начале строки.
    // Обёртки объявлены ДО макросов: препроцессор идёт по файлу последовательно,
    // поэтому вызовы внутри самих обёрток макросами не подменяются.
    namespace {
        GS::UniString WithFuncTag (const char *tag, const GS::UniString &text) {
            return GS::UniString ("[") + tag + GS::UniString ("] ") + text;
        }

        void TaggedPrnt (const char *tag, GS::UniString msg, GS::UniString reportString) {
            DBprnt (WithFuncTag (tag, msg), reportString);
        }

        void TaggedTest (const char *tag, bool usl, GS::UniString reportString) {
            DBtest (usl, WithFuncTag (tag, reportString));
        }

        void TaggedTest (const char *tag, GS::UniString a, GS::UniString b, GS::UniString reportString) {
            DBtest (a, b, WithFuncTag (tag, reportString));
        }

        void TaggedTest (const char *tag, double a, double b, GS::UniString reportString) {
            DBtest (a, b, WithFuncTag (tag, reportString));
        }
    } // namespace

    // Подмена имён: набор перегрузок тот же, первым аргументом добавляется имя
    // вызывающей функции. Область действия макросов - только этот файл.
    #define DBprnt(...) TaggedPrnt (__func__, __VA_ARGS__)
    #define DBtest(...) TaggedTest (__func__, __VA_ARGS__)

    void Test () {
        DBprnt ("TEST", "start");
        TestSpecRegression ();
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

    void TestSpecGetParamValue () {
        DBprnt ("SpecGetParamValue", "start");
        const API_Guid guid = APINULLGuid;
        const GS::UniString rawname = "{@property:test-spec}";
        const GS::UniString libname = FORMULANAMEPREFIX + "{@listdata:elem.naen}<>";
        Spec::SpecReadContext context;
        ParamValue result;
        ParamValue source;
        source.isValid = true;
        source.val.type = API_PropertyStringValueType;
        source.val.uniStringValue = "original";
        source.val.canCalculate = true;
        auto read = [&] (const GS::UniString &name, GS::Int32 layer) {
            result = source;
            return Spec::SpecValueReader (context).Read (guid, name, result, layer);
        };
        DBtest (read (rawname, 0), false, "SpecGetParamValue missing element");
        DBtest (result.isValid, false, "SpecGetParamValue missing element invalidates");
        ParamDictValue params;
        context.read.Add (guid, params);
        DBtest (read (rawname, 0), false, "SpecGetParamValue missing key");
        DBtest (result.isValid, false, "SpecGetParamValue missing key invalidates");
        context.read.Get (guid).Add (rawname, source);
        DBtest (read (rawname, 0), true, "SpecGetParamValue ordinary success");
        DBtest (result.val.uniStringValue, GS::UniString ("original"), "SpecGetParamValue ordinary unchanged");
        context.read.Get (guid).Get (rawname).isValid = false;
        DBtest (read (rawname, 0), false, "SpecGetParamValue invalid input");
        DBtest (result.isValid, false, "SpecGetParamValue invalid input invalidates");
        context.read.Get (guid).Get (rawname) = source;
        context.read.Get (guid).Get (rawname).fromMaterial = true;
        DBtest (read (rawname, 0), false, "SpecGetParamValue missing composite element");
        DBtest (result.isValid, false, "SpecGetParamValue missing composite invalidates");
        ParamDictComposite layers;
        context.composite.Add (guid, layers);
        DBtest (read (rawname, 0), false, "SpecGetParamValue missing composite key");
        DBtest (result.isValid, false, "SpecGetParamValue composite key invalidates");
        ParamComposite composite;
        context.composite.Get (guid).Add (rawname, composite);
        DBtest (read (rawname, 0), false, "SpecGetParamValue empty composite");
        DBtest (result.isValid, false, "SpecGetParamValue empty composite invalidates");
        ParamValueComposite layer;
        layer.val = "12.5";
        context.composite.Get (guid).Get (rawname).composite.Push (layer);
        DBtest (read (rawname, 0), true, "SpecGetParamValue numeric layer");
        DBtest (result.val.doubleValue, 12.5, "SpecGetParamValue layer number");
        DBtest (result.val.canCalculate, true, "SpecGetParamValue layer calculable");
        DBtest (result.val.intValue, (GS::Int32)12, "SpecGetParamValue layer integer");
        DBtest (result.val.boolValue, true, "SpecGetParamValue layer nonzero");
        DBtest (read (rawname, 1), true, "SpecGetParamValue layer past end");
        DBtest (result.isValid, true, "SpecGetParamValue past end valid");
        DBtest (result.val.uniStringValue.IsEmpty (), true, "SpecGetParamValue past end empty");
        DBtest (result.val.canCalculate, false, "SpecGetParamValue past end not calculable");
        DBtest (result.val.boolValue, false, "SpecGetParamValue past end false");
        context.composite.Get (guid).Get (rawname).composite[0].val = "material";
        DBtest (read (rawname, 0), true, "SpecGetParamValue text layer");
        DBtest (result.val.canCalculate, false, "SpecGetParamValue text not calculable");
        DBtest (result.val.boolValue, true, "SpecGetParamValue text nonempty");
        DBtest (read (libname, 0), false, "SpecGetParamValue missing lib formula");
        DBtest (result.isValid, false, "SpecGetParamValue missing lib invalidates");
        ParamValue formula;
        formula.rawName = libname;
        formula.val.hasFormula = true;
        formula.val.uniStringValue = "{@listdata:elem.naen}<>";
        context.read.Get (guid).Add (libname, formula);
        DBtest (read (libname, 0), true, "SpecGetParamValue missing lib data empty success");
        DBtest (result.isValid, true, "SpecGetParamValue empty lib valid");
        DBtest (result.val.uniStringValue.IsEmpty (), true, "SpecGetParamValue empty lib text");
        DBtest (result.val.canCalculate, false, "SpecGetParamValue empty lib not calculable");
        DBtest (read (libname, -1), false, "SpecGetParamValue negative lib index");
        DBtest (result.isValid, false, "SpecGetParamValue negative lib invalidates");
        DBtest (read (rawname, -1), false, "SpecGetParamValue negative material index");
        DBtest (result.isValid, false, "SpecGetParamValue negative material invalidates");
        ListData::LibElement lib;
        ListData::Subpos subpos;
        ListData::Arm arm;
        arm.naen = "Rebar";
        subpos.arm.Add ("10@test", arm);
        lib.subpos.Add ("test", subpos);
        lib.keys.Push (GS::Pair<GS::UniString, GS::UniString> ("test", "10@test"));
        context.listData.Add (guid, lib);
        DBtest (read (libname, 0), true, "SpecGetParamValue lib formula success");
        DBtest (result.val.uniStringValue, GS::UniString ("Rebar"), "SpecGetParamValue lib formula result");
        DBtest (read (libname, 1), true, "SpecGetParamValue lib past end success");
        DBtest (result.val.uniStringValue.IsEmpty (), true, "SpecGetParamValue lib past end empty");
        DBtest (read (libname, -1), false, "SpecGetParamValue populated lib negative index");
        DBtest (result.isValid, false, "SpecGetParamValue populated lib negative invalidates");
        DBprnt ("SpecGetParamValue", "end");
    }

    namespace {
        // Синтетические GUID используются только как ключи словарей, не как элементы модели.
        struct SpecFixture {
            const API_Guid first = APIGuidFromString ("{11111111-1111-1111-1111-111111111111}");
            const API_Guid second = APIGuidFromString ("{22222222-2222-2222-2222-222222222222}");
            const API_Guid old = APIGuidFromString ("{33333333-3333-3333-3333-333333333333}");
            const API_Guid extra = APIGuidFromString ("{44444444-4444-4444-4444-444444444444}");
            const GS::UniString key = "{@property:spec-key}";
            const GS::UniString text = "{@property:spec-text}";
            const GS::UniString quantity = "{@property:spec-quantity}";
            const GS::UniString flag = "{@property:spec-flag}";
            const GS::UniString outText = "{@property:spec-out-text}";
            const GS::UniString outQuantity = "{@property:spec-out-quantity}";
            Spec::SpecRule rule;
            // R5.3: набор прочитанных словарей — один объект, как в SpecArray.
            Spec::SpecReadContext context;
            Spec::ElementDict created;
            Spec::ElementDict modified;
            GS::Array<API_Guid> deleted;
            UnicGuid errors;

            SpecFixture () {
                rule.only_visible = false;
                rule.stop_on_error = false;
                rule.favorite_name = "Spec fixture";
                rule.out_paramrawname.Push (outText);
                rule.out_sum_paramrawname.Push (outQuantity);
                Spec::GroupSpec group;
                group.unic_paramrawname.Push (key);
                group.out_paramrawname.Push (text);
                group.sum_paramrawname.Push (quantity);
                rule.groups.Push (group);
            }

            void Text (const API_Guid &guid, const GS::UniString &name, const GS::UniString &value) {
                ParamValue param;
                param.rawName = name;
                ParamHelpers::ConvertStringToParamValue (param, name, value);
                if (!context.read.ContainsKey (guid))
                    context.read.Add (guid, ParamDictValue ());
                context.read.Get (guid).Put (name, param);
            }

            void Number (const API_Guid &guid, const GS::UniString &name, Int32 value) {
                ParamValue param;
                param.rawName = name;
                ParamHelpers::ConvertIntToParamValue (param, name, value);
                if (!context.read.ContainsKey (guid))
                    context.read.Add (guid, ParamDictValue ());
                context.read.Get (guid).Put (name, param);
            }

            void Source (const API_Guid &guid, const GS::UniString &k, const GS::UniString &t, Int32 q) {
                rule.elements.Push (guid);
                Text (guid, key, k);
                Text (guid, text, t);
                Number (guid, quantity, q);
            }

            void Existing (const API_Guid &guid, const GS::UniString &t, Int32 q) {
                rule.delete_old = true;
                rule.exsist_elements.Push (guid);
                Text (guid, outText, t);
                Number (guid, outQuantity, q);
            }

            Int32 Run () {
                created.Clear ();
                modified.Clear ();
                deleted.Clear ();
                errors.Clear ();
                return Spec::GetElementsForRule (rule, context, created, modified, deleted, errors, false);
            }

            void Shape (Int32 result,
                        Int32 expected,
                        UInt32 ncreate,
                        UInt32 nmodify,
                        UInt32 ndelete,
                        const GS::UniString &label) const {
                DBtest (result, expected, label + " result");
                DBtest (created.GetSize (), ncreate, label + " create");
                DBtest (modified.GetSize (), nmodify, label + " modify");
                DBtest (deleted.GetSize (), ndelete, label + " delete");
            }
        };
    } // namespace

    void TestSpecGrouping () {
        DBprnt ("SpecRegression grouping", "start");
        {
            SpecFixture f;
            f.Shape (f.Run (), 0, 0, 0, 0, "Spec empty");
            f.Source (f.first, "A", "Alpha", 2);
            // Маркер из описания правила и разрешённое свойство избранного - разные
            // поля после R3.3. В Element копируется разрешённое (destinationParamGuidName),
            // маркер остаётся в правиле и в строку не попадает.
            f.rule.subguid_paramrawname = "fixture-link-marker";
            f.rule.destinationParamGuidName = "fixture-link";
            f.rule.subguid_rulename = "fixture-rule";
            f.rule.subguid_rulevalue = "fixture-value";
            f.Shape (f.Run (), 1, 1, 0, 0, "Spec single");
            const Spec::Element *row = f.created.GetPtr ("@A");
            DBtest (row != nullptr, "Spec single exact key");
            if (row != nullptr) {
                DBtest (row->out_param.GetSize (), 1, "Spec single text count");
                DBtest (row->out_sum_param.GetSize (), 1, "Spec single sum count");
                if (!row->out_param.IsEmpty ())
                    DBtest (row->out_param[0].val.uniStringValue, GS::UniString ("Alpha"), "Spec single text");
                if (!row->out_sum_param.IsEmpty ())
                    DBtest (row->out_sum_param[0].val.intValue, 2, "Spec single sum");
                DBtest (row->elements.GetSize (), 1, "Spec single source count");
                DBtest (!row->elements.IsEmpty () && row->elements[0] == f.first, "Spec single source GUID");
                DBtest (row->favorite_name, f.rule.favorite_name, "Spec favorite copied");
                DBtest (row->subguid_paramrawname, f.rule.destinationParamGuidName, "Spec link copied");
                DBtest (row->subguid_rulename, f.rule.subguid_rulename, "Spec rule name copied");
                DBtest (row->subguid_rulevalue, f.rule.subguid_rulevalue, "Spec rule value copied");
                DBtest (row->out_paramrawname[0], f.outText, "Spec text target copied");
                DBtest (row->out_sum_paramrawname[0], f.outQuantity, "Spec sum target copied");
                DBtest (row->exs_guid == APINULLGuid, "Spec new row no existing GUID");
            }
            DBtest (f.errors.IsEmpty (), "Spec valid problem list empty");
            DBtest (f.context.read.Get (f.first).Get (f.quantity).val.intValue, 2, "Spec source quantity unchanged");
            f.Source (f.second, "A", "Beta", 3);
            f.Shape (f.Run (), 1, 1, 0, 0, "Spec merge");
            row = f.created.GetPtr ("@A");
            if (row != nullptr && row->out_sum_param.GetSize () == 1 && row->out_param.GetSize () == 1) {
                DBtest (row->out_sum_param[0].val.intValue, 5, "Spec merged integer");
                DBtest (row->out_sum_param[0].val.doubleValue, 5, "Spec merged real");
                DBtest (row->out_sum_param[0].val.rawDoubleValue, 5, "Spec merged raw real");
                DBtest (row->out_param[0].val.uniStringValue, GS::UniString ("Alpha"), "Spec first output wins");
                DBtest (row->elements.GetSize (), 2, "Spec merged source count");
                DBtest (row->elements.GetSize () == 2 && row->elements[0] == f.first && row->elements[1] == f.second,
                        "Spec source order");
            }
            f.Text (f.second, f.key, "B");
            f.Shape (f.Run (), 2, 2, 0, 0, "Spec separate keys");
            DBtest (f.created.ContainsKey ("@A") && f.created.ContainsKey ("@B"), "Spec two exact keys");
        }
        {
            SpecFixture f;
            f.Source (f.first, "  A  B  ", "Alpha", 2);
            f.Source (f.second, "A B", "Alpha", -2);
            f.Shape (f.Run (), 1, 1, 0, 0, "Spec trim key");
            const Spec::Element *row = f.created.GetPtr ("@A B");
            DBtest (row != nullptr, "Spec normalized key");
            if (row != nullptr && row->out_sum_param.GetSize () == 1)
                DBtest (row->out_sum_param[0].val.intValue, 0, "Spec signed sum zero");
            f.rule.groups[0].sum_paramrawname[0] = "1";
            f.context.read.Get (f.first).Delete (f.quantity);
            f.context.read.Get (f.second).Delete (f.quantity);
            f.Shape (f.Run (), 1, 1, 0, 0, "Spec literal count");
            row = f.created.GetPtr ("@A B");
            if (row != nullptr && row->out_sum_param.GetSize () == 1)
                DBtest (row->out_sum_param[0].val.intValue, 2, "Spec literal count value");
            f.rule.groups[0].unic_paramrawname.Push (f.text);
            f.Run ();
            DBtest (f.created.ContainsKey ("@A B@Alpha"), "Spec composite key order");
            f.rule.groups[0].unic_paramrawname.Clear ();
            f.Run ();
            DBtest (f.created.ContainsKey (EMPTYSTRING), "Spec empty unique list key");
        }
        {
            SpecFixture f;
            f.Source (f.first, "A", "Alpha", 2);
            f.rule.groups[0].flag_paramrawname = f.flag;
            f.Shape (f.Run (), 1, 1, 0, 0, "Spec missing flag includes");
            f.Number (f.first, f.flag, 0);
            f.Shape (f.Run (), 0, 0, 0, 0, "Spec false flag skips");
            f.Number (f.first, f.flag, 1);
            f.Shape (f.Run (), 1, 1, 0, 0, "Spec true flag includes");
            f.context.read.Get (f.first).Get (f.flag).isValid = false;
            f.Shape (f.Run (), 1, 1, 0, 0, "Spec invalid flag includes");
            f.rule.groups[0].is_Valid = false;
            f.Shape (f.Run (), 0, 0, 0, 0, "Spec invalid group skips");
            f.rule.groups.Clear ();
            f.Shape (f.Run (), 0, 0, 0, 0, "Spec no groups");
        }
        {
            SpecFixture f;
            f.Source (f.first, "A", "Alpha", 2);
            const Spec::GroupSpec group = f.rule.groups[0];
            f.rule.groups.Push (group);
            f.Shape (f.Run (), 1, 1, 0, 0, "Spec groups share key");
            const Spec::Element *row = f.created.GetPtr ("@A");
            if (row != nullptr && row->out_sum_param.GetSize () == 1) {
                DBtest (row->out_sum_param[0].val.intValue, 4, "Spec groups merge sums");
                DBtest (row->elements.GetSize (), 2, "Spec group source multiplicity");
            }
            f.rule.groups[1].unic_paramrawname[0] = f.text;
            f.Shape (f.Run (), 2, 2, 0, 0, "Spec groups different keys");
            DBtest (f.created.ContainsKey ("@A") && f.created.ContainsKey ("@Alpha"), "Spec group keys");
        }
        // Каждый отказ проверяется на новой fixture; ошибки не маскируются остатками прошлого прогона.
        for (Int32 missing = 0; missing < 3; ++missing) {
            for (Int32 stop = 0; stop < 2; ++stop) {
                SpecFixture f;
                f.Source (f.first, "A", "Alpha", 2);
                f.Source (f.second, "B", "Beta", 3);
                f.rule.stop_on_error = stop != 0;
                const GS::UniString name = missing == 0 ? f.key : (missing == 1 ? f.text : f.quantity);
                f.context.read.Get (f.second).Delete (name);
                const GS::UniString label = GS::UniString::Printf ("Spec missing %d stop %d", missing, stop);
                f.Shape (f.Run (), stop != 0 ? 0 : 1, stop != 0 ? 0 : 1, 0, 0, label);
                DBtest (f.errors.ContainsKey (f.second), stop != 0, label + " blamed GUID");
                DBtest (!f.errors.ContainsKey (f.first), label + " valid source not blamed");
            }
        }
        {
            SpecFixture f;
            f.Source (f.first, "A", "Alpha", 2);
            f.rule.out_paramrawname.Push ("{@property:extra-output}");
            f.Shape (f.Run (), 0, 0, 0, 0, "Spec output arity mismatch");
            f.rule.out_paramrawname.Pop ();
            f.rule.out_sum_paramrawname.Push ("{@property:extra-sum}");
            f.Shape (f.Run (), 0, 0, 0, 0, "Spec sum arity mismatch");
        }
        DBprnt ("SpecRegression grouping", "end");
    }

    void TestSpecReconcile () {
        DBprnt ("SpecRegression reconcile", "start");
        {
            SpecFixture f;
            f.Source (f.first, "A", "Alpha", 2);
            f.Existing (f.old, "Alpha", 2);
            f.Shape (f.Run (), 0, 0, 0, 0, "Spec unchanged");
            f.Shape (f.Run (), 0, 0, 0, 0, "Spec unchanged repeat");
            DBtest (f.rule.elements.GetSize (), 1, "Spec rule sources preserved");
            DBtest (f.rule.exsist_elements.GetSize (), 1, "Spec existing list preserved");
            f.Number (f.first, f.quantity, 3);
            f.Shape (f.Run (), 1, 0, 1, 0, "Spec quantity changed");
            const Spec::Element *row = f.modified.GetPtr ("@A");
            DBtest (row != nullptr, "Spec update key");
            if (row != nullptr && row->out_sum_param.GetSize () == 1) {
                DBtest (row->exs_guid == f.old, "Spec update preserves target GUID");
                DBtest (row->out_sum_param[0].val.intValue, 3, "Spec update new sum");
                DBtest (row->elements.GetSize () == 1 && row->elements[0] == f.first, "Spec update source GUID");
            }
            f.context.read.Get (f.old).Delete (f.outQuantity);
            f.Shape (f.Run (), 1, 0, 1, 0, "Spec absent old sum updates");
            f.context.read.Get (f.old).Delete (f.outText);
            f.Shape (f.Run (), 2, 1, 0, 1, "Spec absent old text replaces");
            DBtest (f.deleted.GetSize () == 1 && f.deleted[0] == f.old, "Spec replacement deletes old GUID");
        }
        {
            SpecFixture f;
            f.Source (f.first, "A", "Alpha", 2);
            f.Existing (f.old, "Other", 2);
            f.Shape (f.Run (), 2, 1, 0, 1, "Spec changed output replaces");
            f.rule.delete_old = false;
            f.Shape (f.Run (), 1, 1, 0, 0, "Spec delete old disabled");
            f.rule.delete_old = true;
            f.rule.elements.Clear ();
            f.Shape (f.Run (), 1, 0, 0, 1, "Spec disappeared row");
        }
        for (Int32 changed = 0; changed < 2; ++changed) {
            SpecFixture f;
            f.Source (f.first, "A", "Alpha", 2);
            f.Existing (f.old, "Alpha", changed != 0 ? 1 : 2);
            f.Existing (f.extra, "Alpha", 2);
            const GS::UniString label = GS::UniString::Printf ("Spec duplicate old changed %d", changed);
            f.Shape (f.Run (), changed + 1, 0, changed, 1, label);
            DBtest (f.deleted.GetSize () == 1 && f.deleted[0] == f.extra, label + " duplicate GUID");
        }
        {
            SpecFixture f;
            f.Source (f.first, "A", "Alpha", 2);
            f.Source (f.second, "B", "Beta", 3);
            f.Existing (f.old, "Alpha", 1);
            f.Existing (f.extra, "Obsolete", 9);
            f.Shape (f.Run (), 3, 1, 1, 1, "Spec mixed actions");
            DBtest (f.created.ContainsKey ("@B"), "Spec mixed new key");
            DBtest (f.modified.ContainsKey ("@A"), "Spec mixed update key");
            DBtest (f.deleted.GetSize () == 1 && f.deleted[0] == f.extra, "Spec mixed delete GUID");
            DBtest (
                f.context.read.Get (f.old).Get (f.outQuantity).val.intValue, 1, "Spec planning does not write values");
        }
        DBprnt ("SpecRegression reconcile", "end");
    }

    // R4.5: сверка S03-S09/S13 и действующей дедупликации.
    // Ничего не меняется и не "исправляется": здесь закрепляются контракты
    // объединения строк и построения ключа, потому что именно их сломает
    // любая будущая правка. Ожидания рассчитаны воспроизведением цикла
    // GetElementsForRule, а не подгонкой под вывод.
    //   key = "@" + val для КАЖДОГО уникального параметра, разделителя между
    //   параметрами нет. При elements.ContainsKey (key) новый элемент НЕ
    //   создаётся: источник дописывается в существующую строку, суммы
    //   складываются по позициям, где оба значения isValid. Первый
    //   представитель (его out_param) не меняется - выходные значения не
    //   суммируются и не переписываются.
    //   n_elements растёт только при СОЗДАНИИ строки, поэтому объединение на
    //   существующем ключе не увеличивает результат.
    void TestSpecMergeAndKey () {
        DBprnt ("SpecRegression merge and key", "start");

        // S08: две группы с ОДИНАКОВЫМ ключом от одного элемента дают одну
        // строку. Сумма складывается (2 + 2 = 4), а список источников получает
        // ОДИН И ТОТ ЖЕ GUID дважды - объединение дописывает источник, не
        // проверяя его новизну. Первый представитель сохраняет свои выходные.
        {
            SpecFixture f;
            f.Source (f.first, "A", "Alpha", 2);
            // Копия, а не Push (f.rule.groups[0]): передача элемента того же
            // массива внутрь Push не гарантирована даже при достаточном запасе.
            const Spec::GroupSpec duplicate = f.rule.groups[0];
            f.rule.groups.Push (duplicate);
            DBtest (f.Run (), 1, "Spec merge same key result");
            DBtest (f.created.GetSize (), 1, "Spec merge same key one row");
            const Spec::Element *row = f.created.GetPtr ("@A");
            DBtest (row != nullptr, "Spec merge same key found");
            if (row != nullptr && row->out_sum_param.GetSize () == 1) {
                DBtest (row->out_sum_param[0].val.intValue, 4, "Spec merge sums add up");
                DBtest (row->out_param.GetSize () == 1 && row->out_param[0].val.uniStringValue == "Alpha",
                        "Spec merge keeps first representative");
                // Два вхождения одного GUID - наблюдаемый контракт, не описка:
                // список источников строки допускает повтор.
                DBtest (row->elements.GetSize () == 2, "Spec merge repeats source GUID");
            }
        }

        // Контроль к предыдущему: те же две группы, но уникальные значения
        // различаются - две строки. Различие только в значении уникального
        // поля, весь остальной код пути тот же.
        {
            SpecFixture f;
            f.Source (f.first, "A", "Alpha", 2);
            Spec::GroupSpec second = f.rule.groups[0];
            second.unic_paramrawname[0] = f.flag; // другой источник значения
            f.rule.groups.Push (second);
            f.Text (f.first, f.flag, "B");
            DBtest (f.Run (), 2, "Spec distinct keys result");
            DBtest (f.created.GetSize (), 2, "Spec distinct keys two rows");
            DBtest (f.created.ContainsKey ("@A") && f.created.ContainsKey ("@B"), "Spec distinct keys both present");
        }

        // S09: КОЛЛИЗИЯ КЛЮЧА. Один уникальный параметр со значением "A@B"
        // даёт ключ "@A@B" - ровно как ДВА параметра со значениями "A" и "B",
        // потому что разделителя между параметрами нет. Строки схлопываются,
        // суммы складываются. Это старый контракт, он НЕ исправляется здесь:
        // менять кодировку ключа в R запрещено планом (S09 - оформить как F).
        {
            SpecFixture f;
            f.Source (f.first, "A@B", "Alpha", 2);
            Spec::GroupSpec split = f.rule.groups[0];
            split.unic_paramrawname[0] = f.flag;
            split.unic_paramrawname.Push (f.outText);
            f.rule.groups.Push (split);
            f.Text (f.first, f.flag, "A");
            f.Text (f.first, f.outText, "B");
            DBtest (f.Run (), 1, "Spec key collision result");
            DBtest (f.created.GetSize (), 1, "Spec key collision collapses to one row");
            const Spec::Element *row = f.created.GetPtr ("@A@B");
            DBtest (row != nullptr, "Spec key collision shared key");
            if (row != nullptr && row->out_sum_param.GetSize () == 1)
                DBtest (row->out_sum_param[0].val.intValue, 4, "Spec key collision sums merged");
        }
        // Контроль к коллизии: при НЕсовпадающей паре ("A@C" против "A"+"B")
        // ключи не совпадают и строки остаются двумя.
        {
            SpecFixture f;
            f.Source (f.first, "A@C", "Alpha", 2);
            Spec::GroupSpec split = f.rule.groups[0];
            split.unic_paramrawname[0] = f.flag;
            split.unic_paramrawname.Push (f.outText);
            f.rule.groups.Push (split);
            f.Text (f.first, f.flag, "A");
            f.Text (f.first, f.outText, "B");
            DBtest (f.Run (), 2, "Spec no collision control result");
            DBtest (f.created.GetSize (), 2, "Spec no collision control two rows");
        }

        // S07 (часть): пропущенная сумма подставляется литералом "1", и этот
        // литерал начисляется на КАЖДЫЙ источник. Два источника с одинаковым
        // ключом дают 1 + 1 = 2, а не 1. Это принципиально отличается от
        // настоящей суммы, поэтому закреплено отдельно.
        {
            SpecFixture f;
            f.Source (f.first, "A", "Alpha", 7);
            f.rule.groups[0].sum_paramrawname.Clear ();
            f.rule.groups[0].sum_paramrawname.Push ("1");
            f.rule.elements.Push (f.second);
            f.Text (f.second, f.key, "A");
            f.Text (f.second, f.text, "Beta");
            DBtest (f.Run (), 1, "Spec literal count result");
            const Spec::Element *row = f.created.GetPtr ("@A");
            DBtest (row != nullptr, "Spec literal count row");
            if (row != nullptr && row->out_sum_param.GetSize () == 1)
                DBtest (row->out_sum_param[0].val.intValue, 2, "Spec literal count per source");
        }

        // S07 (часть): отказ по размеру группы сделан в ExpandGroup, поэтому
        // при расчёте строк отказ локален для группы - её снимает один флаг
        // is_Valid, а остальные группы правила обрабатываются как обычно.
        {
            SpecFixture f;
            f.Source (f.first, "A", "Alpha", 2);
            Spec::GroupSpec broken = f.rule.groups[0];
            broken.is_Valid = false;
            f.rule.groups.Push (broken);
            DBtest (f.Run (), 1, "Spec invalid group skipped result");
            DBtest (f.created.GetSize (), 1, "Spec invalid group only survivor");
            DBtest (f.created.ContainsKey ("@A"), "Spec invalid group kept valid key");
        }

        // S07 (часть): неполный выход отвергается сверкой слотов. Элемент, у
        // которого не набрано всех выходных значений, в словарь не попадает,
        // а n_elements обнуляется - но только при stop_on_error.
        {
            SpecFixture f;
            f.Source (f.first, "A", "Alpha", 2);
            f.rule.stop_on_error = true;
            f.rule.out_paramrawname.Push (f.outQuantity); // два слота против одного
            DBtest (f.Run (), 0, "Spec incomplete slots rejected");
            DBtest (f.created.IsEmpty (), "Spec incomplete slots no row");
            DBtest (f.errors.ContainsKey (f.first), "Spec incomplete slots marks element");
        }

        DBprnt ("SpecRegression merge and key", "end");
    }

    // R4.5: дедупликация AddRule по ключу = текст ВНУТРИ фигурных скобок.
    // Префикс правила ("Spec_rule", "_v2", "_v3", "_km") в ключ НЕ входит,
    // поэтому описания с одинаковым телом и разной политикой схлопываются, и
    // выигрывает ТО, ЧТО ПРИШЛО ПЕРВЫМ. Закрепляется как контракт: описания
    // в существующих моделях завязаны на такое поведение.
    void TestSpecRuleDedup () {
        DBprnt ("SpecRegression rule dedup", "start");
        SpecFixture f;
        // Ключ словаря - подстрока МЕЖДУ фигурными скобками нормализованного
        // описания (префикс политики в него не входит). Не хардкодим: повторяем
        // ровно ту же сборку, что делает AddRule.
        // Возврат ЯВНО GS::UniString обязателен: GetSubstring отдаёт view-тип
        // Substring, ссылающийся на нормализованную строку. Без явного типа
        // лямбда вернула бы висящий вид на временный объект.
        const auto ruleKey = [] (const GS::UniString &description) -> GS::UniString {
            const GS::UniString normalized = Spec::NormalizeRuleDescription (description);
            return GS::UniString (normalized.GetSubstring (CHARBRACESTART, CHARBRACEEND, 0));
        };
        const GS::UniString bodyKey = ruleKey ("Spec_rule{Fav;g(u;p;f;q)s(x;y)}");
        const GS::UniString brokenKey = ruleKey ("Spec_rule{Fav;g(u;p;f;q)s(x)}");

        // Префикс политики не входит в ключ: v1 и v2 с одним телом - одна запись.
        {
            Spec::SpecRuleDict rules;
            API_PropertyDefinition d = {};
            d.groupGuid = f.extra;
            d.guid = f.old;
            d.name = "Sync_name_Plain";
            d.description = "Spec_rule{Fav;g(u;p;f;q)s(x;y)}";
            Spec::AddRule (d, f.first, rules);
            d.name = "Sync_name_V2";
            d.guid = f.extra;
            d.description = "Spec_rule_v2{Fav;g(u;p;f;q)s(x;y)}";
            Spec::AddRule (d, f.second, rules);
            DBtest (rules.GetSize (), 1, "Spec dedup prefix not in key");
            const Spec::SpecRule *rule = rules.GetPtr (bodyKey);
            DBtest (rule != nullptr, "Spec dedup key found");
            if (rule != nullptr) {
                // Политика осталась от ПЕРВОГО (v1): delete_old не взведён.
                DBtest (!rule->delete_old, "Spec dedup first policy wins");
                DBtest (rule->rule_name == "Sync_name_Plain", "Spec dedup first property name wins");
                DBtest (rule->rule_definitions.guid == f.old, "Spec dedup first property GUID wins");
                // Второе свойство не попало ни в rule_name, ни в
                // rule_definitions, но его элемент добавлен в общий список.
                DBtest (rule->elements.GetSize () == 2, "Spec dedup second source appended");
            }
        }

        // Невалидное правило занимает ключ навсегда: оно попадает в словарь
        // даже при parseValid == false (чтобы не обработать дважды), и
        // последующее описание с тем же ключом правило уже не восстановит -
        // оно только попробует дополнить список элементов.
        {
            Spec::SpecRuleDict rules;
            API_PropertyDefinition d = {};
            d.groupGuid = f.extra;
            d.guid = f.old;
            d.name = "Sync_name_Broken";
            d.description = "Spec_rule{Fav;g(u;p;f;q)s(x)}"; // схема из одной части
            Spec::AddRule (d, f.first, rules);
            const Spec::SpecRule *first = rules.GetPtr (brokenKey);
            DBtest (first != nullptr && !first->parseValid, "Spec dedup invalid cached");
            DBtest (first != nullptr && first->parseError == Spec::ParseError::OutputPartCount,
                    "Spec dedup invalid keeps reason");
            d.name = "Sync_name_Valid";
            Spec::AddRule (d, f.second, rules);
            DBtest (rules.GetSize (), 1, "Spec dedup invalid not rebuilt");
            const Spec::SpecRule *after = rules.GetPtr (brokenKey);
            DBtest (after != nullptr && !after->parseValid, "Spec dedup stays invalid");
            // Источник не добавлен: элементы дописываются только валидному правилу.
            DBtest (after != nullptr && after->elements.IsEmpty (), "Spec dedup invalid takes no source");
        }

        // Одно и то же описание, написанное по-разному, даёт один ключ.
        {
            Spec::SpecRuleDict rules;
            API_PropertyDefinition d = {};
            d.groupGuid = f.extra;
            d.guid = f.old;
            d.name = "Sync_name_Compact";
            d.description = "Spec_rule{Fav;g(u;p;f;q)s(x;y)}";
            Spec::AddRule (d, f.first, rules);
            d.name = "Sync_name_Spaced";
            d.description = "Spec_rule { Fav ;\n\t g (u;p;f;q) s (x;y) }";
            Spec::AddRule (d, f.second, rules);
            DBtest (rules.GetSize (), 1, "Spec dedup normalizes to one key");
            const Spec::SpecRule *rule = rules.GetPtr (bodyKey);
            DBtest (rule != nullptr && rule->elements.GetSize () == 2, "Spec dedup normalized source appended");
            DBtest (rule != nullptr && rule->rule_name == "Sync_name_Compact", "Spec dedup normalized first wins");
        }

        // Порядок arrival определяет победителя: то же тело, но сначала v2.
        {
            Spec::SpecRuleDict rules;
            API_PropertyDefinition d = {};
            d.groupGuid = f.extra;
            d.guid = f.old;
            d.name = "Sync_name_V2First";
            d.description = "Spec_rule_v2{Fav;g(u;p;f;q)s(x;y)}";
            Spec::AddRule (d, f.first, rules);
            d.name = "Sync_name_PlainSecond";
            d.guid = f.extra;
            d.description = "Spec_rule{Fav;g(u;p;f;q)s(x;y)}";
            Spec::AddRule (d, f.second, rules);
            DBtest (rules.GetSize (), 1, "Spec dedup order one key");
            const Spec::SpecRule *rule = rules.GetPtr (bodyKey);
            DBtest (rule != nullptr && rule->delete_old, "Spec dedup first v2 policy kept");
            DBtest (rule != nullptr && rule->rule_name == "Sync_name_V2First", "Spec dedup order first name kept");
        }

        DBprnt ("SpecRegression rule dedup", "end");
    }

    void TestSpecReadPlan () {
        DBprnt ("SpecRegression read plan", "start");
        SpecFixture f;
        ParamDictElement read;
        ParamDictValue write;
        Spec::GetParamToReadFromRule (f.rule, read, write);
        DBtest (read.IsEmpty (), "Spec read no sources");
        DBtest (write.GetSize (), 2, "Spec write targets without sources");
        DBtest (write.ContainsKey (f.outText) && write.ContainsKey (f.outQuantity), "Spec write target names");
        f.rule.elements.Push (f.first);
        f.rule.elements.Push (f.second);
        f.rule.groups[0].flag_paramrawname = f.flag;
        f.rule.groups[0].sum_paramrawname.Push ("1");
        const Spec::GroupSpec duplicate = f.rule.groups[0];
        f.rule.groups.Push (duplicate);
        Spec::GetParamToReadFromRule (f.rule, read, write);
        DBtest (read.GetSize (), 2, "Spec read source count");
        for (const API_Guid &guid : f.rule.elements) {
            const ParamDictValue *params = read.GetPtr (guid);
            DBtest (params != nullptr, "Spec read source exists");
            if (params != nullptr) {
                DBtest (params->GetSize (), 4, "Spec read deduplicates names");
                DBtest (params->ContainsKey (f.key), "Spec read unique name");
                DBtest (params->ContainsKey (f.text), "Spec read output source");
                DBtest (params->ContainsKey (f.quantity), "Spec read sum source");
                DBtest (params->ContainsKey (f.flag), "Spec read flag");
                DBtest (!params->ContainsKey ("1") && !params->ContainsKey ("{@gdl:1}"), "Spec count not read");
            }
        }
        Spec::GetParamToReadFromRule (f.rule, read, write);
        DBtest (read.GetSize (), 2, "Spec repeated read source count");
        DBtest (write.GetSize (), 2, "Spec repeated write deduplicated");
        for (Int32 mode = 0; mode < 3; ++mode) {
            SpecFixture special;
            special.rule.elements.Push (special.first);
            special.rule.groups[0].flag_paramrawname = special.flag;
            special.rule.groups[0].is_Valid = mode != 0;
            special.rule.groups[0].fromMaterial = mode == 1;
            special.rule.groups[0].fromLibData = mode == 2;
            special.rule.groups[0].n_layer = 1;
            ParamDictElement specialRead;
            ParamDictValue specialWrite;
            Spec::GetParamToReadFromRule (special.rule, specialRead, specialWrite);
            if (mode == 0) {
                const ParamDictValue *params = specialRead.GetPtr (special.first);
                DBtest (params != nullptr && params->GetSize () == 1 && params->ContainsKey (special.flag),
                        "Spec invalid group reads flag only");
            } else {
                DBtest (specialRead.IsEmpty (), "Spec nonzero material/lib layer skipped");
            }
            DBtest (specialWrite.GetSize (), 2, "Spec skipped group keeps write targets");
        }
        DBprnt ("SpecRegression read plan", "end");
    }

    // R5.1: сбор зависимостей правила - что нужно прочитать у источников и что
    // потом записать. Сбор отделён от разворачивания имён в словарь элемента,
    // поэтому проверяется сам перечень (без обращения к модели), а сличение в
    // общие словари запуска остаётся отдельным контрактом адаптера.
    void TestSpecRuleDependencies () {
        DBprnt ("SpecRegression rule dependencies", "start");

        const GS::UniString P ("{@gdl:p}"), U ("{@gdl:u}"), F ("{@gdl:f}"), Q ("{@gdl:q}");

        // Пустое правило: ни читать, ни записывать нечего.
        {
            const Spec::SpecRule rule;
            const Spec::RuleDependencies dependencies = Spec::CollectRuleDependencies (rule);
            DBtest (dependencies.read.IsEmpty (), true, "empty rule reads nothing");
            DBtest (dependencies.write.IsEmpty (), true, "empty rule writes nothing");
        }

        // Обычная группа: флаг, уникальный, выходной и сумма - все читаются;
        // на запись идут только имена ВЫХОДНОЙ СХЕМЫ правила, а не поля группы.
        {
            Spec::SpecRule rule;
            rule.out_paramrawname.Push (P);
            rule.out_sum_paramrawname.Push (Q);
            Spec::GroupSpec group;
            group.flag_paramrawname = F;
            group.unic_paramrawname.Push (U);
            group.out_paramrawname.Push (P);
            group.sum_paramrawname.Push (Q);
            rule.groups.Push (group);
            const Spec::RuleDependencies dependencies = Spec::CollectRuleDependencies (rule);
            DBtest (dependencies.read.GetSize (), 4, "plain group reads four names");
            DBtest (dependencies.read.ContainsKey (F), true, "flag is read");
            DBtest (dependencies.read.ContainsKey (U), true, "unic is read");
            DBtest (dependencies.read.ContainsKey (P), true, "group out is read");
            DBtest (dependencies.read.ContainsKey (Q), true, "group sum is read");
            DBtest (dependencies.write.GetSize (), 2, "schema names are written");
            DBtest (dependencies.write.ContainsKey (P) && dependencies.write.ContainsKey (Q), true, "write names");
            DBtest (dependencies.write.ContainsKey (U) || dependencies.write.ContainsKey (F),
                    false,
                    "group fields are not written");
        }

        // Литерал "1" в суммах - счётчик источников, читать нечего.
        {
            Spec::SpecRule rule;
            Spec::GroupSpec group;
            group.out_paramrawname.Push (P);
            group.sum_paramrawname.Push ("1");
            rule.groups.Push (group);
            const Spec::RuleDependencies dependencies = Spec::CollectRuleDependencies (rule);
            DBtest (dependencies.read.ContainsKey ("1"), false, "literal one is not read");
            DBtest (dependencies.read.ContainsKey ("{@gdl:1}"), false, "literal one is not normalised");
            // Два имени, а не одно: пустой флаг группы тоже попадает в перечень
            // (см. блок про пустой флаг ниже) и отбрасывается только при разворачивании.
            DBtest (dependencies.read.GetSize (), 2, "only group out and empty flag are listed");
        }

        // Невалидная группа: её поля не читаются, но ФЛАГ читается - по нему
        // элемент отбрасывается в цикле, и без чтения отбрасывался бы иначе.
        {
            Spec::SpecRule rule;
            Spec::GroupSpec group;
            group.is_Valid = false;
            group.flag_paramrawname = F;
            group.unic_paramrawname.Push (U);
            group.out_paramrawname.Push (P);
            group.sum_paramrawname.Push (Q);
            rule.groups.Push (group);
            const Spec::RuleDependencies dependencies = Spec::CollectRuleDependencies (rule);
            DBtest (dependencies.read.GetSize (), 1, "invalid group reads only flag");
            DBtest (dependencies.read.ContainsKey (F), true, "invalid group flag is read");
        }

        // Пустой флаг попадает в перечень, но в словарь элемента не попадает:
        // AddValueToParamDictValue игнорирует пустое имя. Это различие между
        // перечнем и разворотом, а не отказ сбора.
        {
            Spec::SpecRule rule;
            Spec::GroupSpec group;
            group.flag_paramrawname = EMPTYSTRING;
            group.out_paramrawname.Push (P);
            rule.groups.Push (group);
            const Spec::RuleDependencies dependencies = Spec::CollectRuleDependencies (rule);
            DBtest (dependencies.read.ContainsKey (EMPTYSTRING), true, "empty flag reaches the list");
            ParamDictValue paramDict;
            Spec::BuildReadParamDict (dependencies.read, paramDict);
            DBtest (paramDict.GetSize (), 1, "empty flag is dropped when expanded");
        }

        // Слои материалов и ведомостей: читается только нулевой слой - остальные
        // копии одинаковы, повторное чтение того же набора не нужно.
        for (Int32 mode = 0; mode < 2; mode++) {
            Spec::SpecRule rule;
            for (Int32 layer = 0; layer < 3; layer++) {
                Spec::GroupSpec group;
                group.n_layer = layer;
                group.fromMaterial = mode == 0;
                group.fromLibData = mode == 1;
                group.flag_paramrawname = F;
                group.out_paramrawname.Push (P);
                group.sum_paramrawname.Push (Q);
                rule.groups.Push (group);
            }
            const Spec::RuleDependencies dependencies = Spec::CollectRuleDependencies (rule);
            // Три группы дают три различных имени (флаг, выход, сумма); слои 1 и 2
            // пропускаются, поэтому их имена в перечень не попадают.
            DBtest (dependencies.read.GetSize (), 3, "layered rule reads layer zero only");
        }

        // Несколько групп: имена объединяются, дубли не повторяются.
        {
            Spec::SpecRule rule;
            rule.out_paramrawname.Push (P);
            rule.out_sum_paramrawname.Push (Q);
            for (Int32 i = 0; i < 3; i++) {
                Spec::GroupSpec group;
                group.flag_paramrawname = F;
                group.unic_paramrawname.Push (U);
                group.out_paramrawname.Push (P);
                group.sum_paramrawname.Push (Q);
                rule.groups.Push (group);
            }
            const Spec::RuleDependencies dependencies = Spec::CollectRuleDependencies (rule);
            DBtest (dependencies.read.GetSize (), 4, "repeated groups do not duplicate names");
            DBtest (dependencies.write.GetSize (), 2, "repeated groups do not duplicate writes");
        }

        // Сбор - чистое перечисление: разворачивания имён в нём нет, поэтому
        // служебных свойств материала (толщина, единицы, коэффициент запаса,
        // двадцать имён синхронизации) в перечне не возникает. Они появляются
        // только в BuildReadParamDict.
        {
            Spec::SpecRule rule;
            const GS::UniString material = "{@material:layers_auto,all;" + P + "}";
            Spec::GroupSpec group;
            group.fromMaterial = true;
            group.out_paramrawname.Push (material);
            rule.groups.Push (group);
            const Spec::RuleDependencies dependencies = Spec::CollectRuleDependencies (rule);
            DBtest (dependencies.read.GetSize (), 2, "material name is listed as is plus empty flag");
            DBtest (dependencies.read.ContainsKey (material), true, "material name kept whole");
            ParamDictValue paramDict;
            Spec::BuildReadParamDict (dependencies.read, paramDict);
            DBtest (paramDict.ContainsKey (material), true, "material param added");
            DBtest (paramDict.ContainsKey ("{@property:buildingmaterialproperties/some_stuff_th}"),
                    true,
                    "material thickness helper added");
            DBtest (paramDict.ContainsKey ("{@property:buildingmaterialproperties/some_stuff_units}"),
                    true,
                    "material units helper added");
            DBtest (paramDict.ContainsKey ("{@property:buildingmaterialproperties/some_stuff_kzap}"),
                    true,
                    "material kzap helper added");
            DBtest (paramDict.ContainsKey ("{@property:sync_name19}"), true, "material sync name range added");
            if (paramDict.ContainsKey (material)) {
                const ParamValue &param = paramDict.Get (material);
                DBtest (param.fromMaterial, true, "material param flagged from material");
                DBtest (param.fromQuantity, true, "material param flagged from quantity");
                DBtest (param.composite_pen, -2, "material param pen");
            }
        }

        // Формула: из имени выражения получаются и служебные свойства слоя, и
        // сама формула с признаком hasFormula.
        {
            Spec::SpecRule rule;
            const GS::UniString formula = FORMULANAMEPREFIX + "0.5*" + P + STRFORMULASTART + STRFORMULAEND;
            Spec::GroupSpec group;
            group.out_paramrawname.Push (formula);
            rule.groups.Push (group);
            const Spec::RuleDependencies dependencies = Spec::CollectRuleDependencies (rule);
            ParamDictValue paramDict;
            Spec::BuildReadParamDict (dependencies.read, paramDict);
            DBtest (paramDict.ContainsKey (formula), true, "formula param added");
            if (paramDict.ContainsKey (formula)) {
                DBtest (paramDict.Get (formula).val.hasFormula, true, "formula param has formula flag");
            }
        }

        // Адаптер сливает зависимости в общие словари запуска: правило без
        // источников не читает ничего, но имена на запись всё равно попадают -
        // это проверка из старого TestSpecReadPlan, сохранённая по смыслу.
        {
            SpecFixture f;
            ParamDictElement read;
            ParamDictValue write;
            Spec::GetParamToReadFromRule (f.rule, read, write);
            DBtest (read.IsEmpty (), true, "adapter reads nothing without sources");
            DBtest (write.GetSize (), 2, "adapter writes schema without sources");
        }

        DBprnt ("SpecRegression rule dependencies", "end");
    }

    // R5.2: разрешение избранного, служебных полей и старых объектов. Четыре
    // вынесенные операции проверяются раздельно; MatchDestinationProperties,
    // SelectExistingElements и AddExistingReadRequests чисты, а
    // ResolveFavoriteLinks упирается в ambient-кэш свойств, поэтому для него
    // закреплён именно ПРОМАХ чтения: неудачное чтение не кэшируется и не
    // подменяет поля правила (R5.2 запрещает новую политику кэша без F).
    void TestSpecFavoriteResolution () {
        DBprnt ("SpecRegression favorite resolution", "start");

        const GS::UniString OutName ("{@property:spec-out}"), SumName ("{@property:spec-sum}");

        // MatchDestinationProperties: все имена найдены - признак остаётся.
        {
            Spec::SpecRule rule;
            rule.out_paramrawname.Push (OutName);
            rule.out_sum_paramrawname.Push (SumName);
            GS::HashTable<GS::UniString, GS::UniString> favorite;
            favorite.Add (OutName, EMPTYSTRING);
            favorite.Add (SumName, EMPTYSTRING);
            ParamDict errors;
            DBtest (Spec::MatchDestinationProperties (rule, favorite, errors), true, "favorite complete ready");
            DBtest (rule.destinationReady, true, "favorite complete flag kept");
            DBtest (errors.IsEmpty (), true, "favorite complete problem list empty");
        }

        // Отсутствие выходного имени снимает готовность и копит имя в error_name.
        {
            Spec::SpecRule rule;
            rule.out_paramrawname.Push (OutName);
            rule.out_sum_paramrawname.Push (SumName);
            GS::HashTable<GS::UniString, GS::UniString> favorite;
            favorite.Add (OutName, EMPTYSTRING);
            ParamDict errors;
            DBtest (Spec::MatchDestinationProperties (rule, favorite, errors), false, "missing sum not ready");
            DBtest (rule.destinationReady, false, "missing sum clears flag");
            DBtest (errors.GetSize (), 1, "missing sum recorded once");
            DBtest (errors.ContainsKey (SumName), true, "missing sum name recorded");
            DBtest (errors.ContainsKey (OutName), false, "present name not recorded");
        }

        // Отсутствие суммы - тот же исход. Проверяется отдельно, потому что
        // сверка идёт двумя проходами, и ошибка в первом не отменяет второй.
        {
            Spec::SpecRule rule;
            rule.out_paramrawname.Push (OutName);
            GS::HashTable<GS::UniString, GS::UniString> favorite;
            ParamDict errors;
            DBtest (Spec::MatchDestinationProperties (rule, favorite, errors), false, "no favorite names not ready");
            DBtest (errors.GetSize (), 1, "only out name recorded");
            DBtest (errors.ContainsKey (OutName), true, "out name recorded");
        }

        // Повторный проход по тому же правилу не дублирует запись в error_name.
        {
            Spec::SpecRule rule;
            rule.out_paramrawname.Push (OutName);
            GS::HashTable<GS::UniString, GS::UniString> favorite;
            ParamDict errors;
            Spec::MatchDestinationProperties (rule, favorite, errors);
            Spec::MatchDestinationProperties (rule, favorite, errors);
            DBtest (errors.GetSize (), 1, "repeat pass does not duplicate entry");
        }

        // SelectExistingElements: пустое выделение означает "взять все".
        {
            Spec::SpecRule rule;
            GS::Array<API_Guid> found;
            found.Push (APIGuidFromString ("{11111111-1111-1111-1111-111111111111}"));
            found.Push (APIGuidFromString ("{22222222-2222-2222-2222-222222222222}"));
            UnicGuid selected;
            Spec::SelectExistingElements (rule, found, selected);
            DBtest (rule.exsist_elements.GetSize (), 2, "empty selection takes all");
        }

        // Непустое выделение фильтрует по составу.
        {
            Spec::SpecRule rule;
            GS::Array<API_Guid> found;
            const API_Guid keep = APIGuidFromString ("{11111111-1111-1111-1111-111111111111}");
            found.Push (keep);
            found.Push (APIGuidFromString ("{22222222-2222-2222-2222-222222222222}"));
            UnicGuid selected;
            selected.Add (keep, true);
            Spec::SelectExistingElements (rule, found, selected);
            DBtest (rule.exsist_elements.GetSize (), 1, "selection filters found");
            DBtest (!rule.exsist_elements.IsEmpty () && rule.exsist_elements[0] == keep, "selection keeps chosen");
        }

        // Непустое выделение ДОПИСЫВАЕТ элементы, а не заменяет прежнее
        // содержимое поля: это прежнее поведение (Push), и оно безопасно только
        // тем, что словарь правил создаётся заново на каждый запуск.
        {
            Spec::SpecRule rule;
            const API_Guid before = APIGuidFromString ("{33333333-3333-3333-3333-333333333333}");
            rule.exsist_elements.Push (before);
            GS::Array<API_Guid> found;
            const API_Guid keep = APIGuidFromString ("{11111111-1111-1111-1111-111111111111}");
            found.Push (keep);
            UnicGuid selected;
            selected.Add (keep, true);
            Spec::SelectExistingElements (rule, found, selected);
            DBtest (rule.exsist_elements.GetSize (), 2, "selection appends to existing");
        }

        // Пустое выделение ПРИСВАИВАЕТ найденное, заменяя прежнее содержимое -
        // асимметрия с предыдущим случаем задана кодом и закреплена здесь.
        {
            Spec::SpecRule rule;
            const API_Guid before = APIGuidFromString ("{33333333-3333-3333-3333-333333333333}");
            rule.exsist_elements.Push (before);
            GS::Array<API_Guid> found;
            found.Push (APIGuidFromString ("{11111111-1111-1111-1111-111111111111}"));
            UnicGuid selected;
            Spec::SelectExistingElements (rule, found, selected);
            DBtest (rule.exsist_elements.GetSize (), 1, "empty selection replaces existing");
        }

        // Несовпадение выделения с найденным даёт пустой результат.
        {
            Spec::SpecRule rule;
            GS::Array<API_Guid> found;
            found.Push (APIGuidFromString ("{11111111-1111-1111-1111-111111111111}"));
            UnicGuid selected;
            selected.Add (APIGuidFromString ("{99999999-9999-9999-9999-999999999999}"), true);
            Spec::SelectExistingElements (rule, found, selected);
            DBtest (rule.exsist_elements.IsEmpty (), true, "selection misses all found");
        }

        // AddExistingReadRequests: запрашиваются выход, суммы и носитель GUID.
        // Имена уходят в НИЖНЕМ регистре (NameToRawName), поэтому сверять их с
        // out_paramrawname напрямую нельзя.
        {
            Spec::SpecRule rule;
            rule.out_paramrawname.Push (OutName);
            rule.out_sum_paramrawname.Push (SumName);
            rule.destinationParamGuidName = "{@property:Spec-Guid}";
            const API_Guid elem = APIGuidFromString ("{11111111-1111-1111-1111-111111111111}");
            GS::Array<API_Guid> elements;
            elements.Push (elem);
            ParamDictElement read;
            Spec::AddExistingReadRequests (rule, elements, read);
            const ParamDictValue *params = read.GetPtr (elem);
            DBtest (params != nullptr, true, "existing request element added");
            if (params != nullptr) {
                DBtest (params->GetSize (), 3, "existing request name count");
                DBtest (params->ContainsKey ("{@property:spec-out}"), true, "existing request out");
                DBtest (params->ContainsKey ("{@property:spec-sum}"), true, "existing request sum");
                DBtest (params->ContainsKey ("{@property:spec-guid}"), true, "existing request guid");
            }
        }

        // Повторный вызов на том же элементе не меняет набор имён.
        {
            Spec::SpecRule rule;
            rule.out_paramrawname.Push (OutName);
            rule.out_sum_paramrawname.Push (SumName);
            rule.destinationParamGuidName = "{@property:Spec-Guid}";
            const API_Guid elem = APIGuidFromString ("{11111111-1111-1111-1111-111111111111}");
            GS::Array<API_Guid> elements;
            elements.Push (elem);
            ParamDictElement read;
            Spec::AddExistingReadRequests (rule, elements, read);
            Spec::AddExistingReadRequests (rule, elements, read);
            const ParamDictValue *params = read.GetPtr (elem);
            DBtest (params != nullptr && params->GetSize () == 3, "repeated request is idempotent");
        }

        // Пустой носитель GUID не добавляет имени (AddValueToParamDictValue
        // игнорирует пустое имя) - правило без delete_old сюда не дойдёт,
        // но контракт разворачивания закреплён.
        {
            Spec::SpecRule rule;
            rule.out_paramrawname.Push (OutName);
            const API_Guid elem = APIGuidFromString ("{11111111-1111-1111-1111-111111111111}");
            GS::Array<API_Guid> elements;
            elements.Push (elem);
            ParamDictElement read;
            Spec::AddExistingReadRequests (rule, elements, read);
            const ParamDictValue *params = read.GetPtr (elem);
            DBtest (params != nullptr && params->GetSize () == 1, "empty guid name not added");
        }

        // ResolveFavoriteLinks: чтение из кэша свойств на синтетическом имени
        // промахивается, поэтому поля правила НЕ заполняются, а paramToWrite
        // остаётся пустым. Это и есть требование R5.2: неудачное чтение не
        // кэшируется и не подменяет результат.
        {
            Spec::SpecRule rule;
            rule.subguid_paramrawname = "fixture-marker";
            GS::HashTable<GS::UniString, GS::UniString> favorite;
            favorite.Add ("{@property:spec-rulename}", "spec_rule_name in name");
            favorite.Add ("{@property:spec-guid}", "fixture-marker sync_guid link");
            ParamDictValue write;
            const bool found = Spec::ResolveFavoriteLinks (rule, favorite, write);
            DBtest (found, false, "cache miss reports no guid link");
            DBtest (rule.subguid_rulename.IsEmpty (), true, "cache miss keeps rulename empty");
            DBtest (rule.destinationParamGuidName.IsEmpty (), true, "cache miss keeps guid name empty");
            DBtest (write.IsEmpty (), true, "cache miss writes nothing");
        }

        // Тот же промах при пустом маркере: поиск GUID-свойства не выполняется
        // вовсе, поэтому признак не может стать истинным никак.
        {
            Spec::SpecRule rule;
            GS::HashTable<GS::UniString, GS::UniString> favorite;
            favorite.Add ("{@property:spec-guid}", "sync_guid link without marker");
            ParamDictValue write;
            DBtest (Spec::ResolveFavoriteLinks (rule, favorite, write), false, "empty marker finds nothing");
        }

        // Избранное без служебных описаний не даёт ни одного признака.
        {
            Spec::SpecRule rule;
            rule.subguid_paramrawname = "fixture-marker";
            GS::HashTable<GS::UniString, GS::UniString> favorite;
            favorite.Add ("{@property:spec-out}", "ordinary property");
            ParamDictValue write;
            DBtest (Spec::ResolveFavoriteLinks (rule, favorite, write), false, "no marker in descriptions");
            DBtest (rule.subguid_rulename.IsEmpty (), true, "no rulename without description");
        }

        // Пустое избранное - тот же нулевой исход, без обращения к кэшу.
        {
            Spec::SpecRule rule;
            GS::HashTable<GS::UniString, GS::UniString> favorite;
            ParamDictValue write;
            DBtest (Spec::ResolveFavoriteLinks (rule, favorite, write), false, "empty favorite finds nothing");
        }

        DBprnt ("SpecRegression favorite resolution", "end");
    }

    // R5.4: контракт read-only доступа к значениям.
    //
    // Закрепляются три вещи, которые иначе остались бы незамеченными:
    //   1) reader только читает — прочитанные словари не меняются при чтении;
    //   2) n_layer < 0 не читает ни материал, ни list-data (тот же отказ, что
    //      и был в GetParamValue);
    //   3) два reader на одном контексте дают одинаковый результат — чтения
    //      не влияют друг на друга.
    // Ожидания для (1) считаются по факту: сравниваются снимки словарей ДО и
    // ПОСЛЕ серии чтений, а не заранее выписанные значения.
    void TestSpecValueReader () {
        DBprnt ("SpecRegression value reader", "start");
        SpecFixture f;
        f.Text (f.first, f.text, "Alpha");
        f.Number (f.first, f.quantity, 3);
        const GS::UniString libname = FORMULANAMEPREFIX + "{@listdata:elem.naen}<>";
        ParamValue formula;
        formula.isValid = true;
        formula.val.hasFormula = true;
        formula.val.type = API_PropertyStringValueType;
        formula.val.uniStringValue = "3";
        f.context.read.Get (f.first).Put (libname, formula);

        // Снимок состояния словарей до чтений.
        ParamDictValue readBefore = f.context.read.Get (f.first);
        ParamDictCompositeElement compositeBefore = f.context.composite;
        ListData::LibElements listDataBefore = f.context.listData;

        const Spec::SpecValueReader reader (f.context);
        ParamValue result;
        // (2) Обычное свойство читается при любом n_layer, в том числе отрицательном.
        DBtest (reader.Read (f.first, f.text, result, -1), "reader ordinary reads at negative layer");
        DBtest (result.val.uniStringValue, GS::UniString ("Alpha"), "reader ordinary value");
        DBtest (reader.Read (f.first, f.quantity, result, -1), "reader number reads at negative layer");
        DBtest (result.val.intValue, 3, "reader number value");
        // (2) list-data при n_layer < 0 отказывает, не падая.
        DBtest (reader.Read (f.first, libname, result, -1), false, "reader libdata refuses negative layer");
        DBtest (result.isValid, false, "reader libdata negative layer invalidates");
        // (3) Второй reader того же контекста даёт тот же результат. Сравниваются
        // два чтения ОДНОГО поля, а не предыдущий результат: к этому моменту в
        // result лежит отказ чтения list-data (пустая строка).
        const Spec::SpecValueReader second (f.context);
        ParamValue other;
        DBtest (second.Read (f.first, f.text, other, -1), "second reader same result");
        ParamValue againSame;
        DBtest (reader.Read (f.first, f.text, againSame, -1), "reader reads same field again");
        DBtest (other.val.uniStringValue, GS::UniString ("Alpha"), "second reader value");
        DBtest (other.val.uniStringValue, againSame.val.uniStringValue, "reader and second reader agree");
        // Отсутствующий элемент и отсутствующий ключ — тот же нулевой исход.
        DBtest (reader.Read (f.second, f.text, result, 0), false, "reader missing element");
        DBtest (reader.Read (f.first, f.key, result, 0), false, "reader missing key");
        DBtest (result.isValid, false, "reader missing key invalidates");

        // (1) Чтения ничего не пишут: словари совпадают со снимком ДО.
        // У GS::HashTable нет operator==, поэтому сравнение идёт перебором пар
        // (размер + наличие каждой пары) — это и есть наблюдаемое состояние.
        DBtest (f.context.read.GetSize (), 1, "reader did not add read entries");
        DBtest (f.context.composite.GetSize (), compositeBefore.GetSize (), "reader composite size unchanged");
        DBtest (f.context.listData.GetSize (), listDataBefore.GetSize (), "reader listdata size unchanged");
        DBtest (f.context.composite.GetSize (), 0, "reader did not populate composite");
        DBtest (f.context.listData.GetSize (), 0, "reader did not populate listdata");
        {
            bool same = true;
            for (ParamDictValue::PairIterator it = readBefore.EnumeratePairs (); it != NULL; ++it) {
    #ifdef ServerMainVers_2800
                const ParamValue &before = it->value;
    #else
                const ParamValue &before = *it->value;
    #endif
                // У ParamValueData нет operator==, а ключ итератора в pre-AC28 -
                // указатель, поэтому сверяются наблюдаемые поля поимённо.
    #ifdef ServerMainVers_2800
                const GS::UniString key = it->key;
    #else
                const GS::UniString key = *it->key;
    #endif
                const ParamValue *now = f.context.read.Get (f.first).GetPtr (key);
                if (now == nullptr || now->val.type != before.val.type ||
                    now->val.uniStringValue != before.val.uniStringValue || now->val.intValue != before.val.intValue ||
                    now->val.doubleValue != before.val.doubleValue || now->val.hasFormula != before.val.hasFormula) {
                    same = false;
                }
            }
            DBtest (same, true, "reader left every read value unchanged");
        }
        // Повторное чтение даёт тот же результат (нет скрытого кэша с побочным эффектом).
        ParamValue again;
        DBtest (reader.Read (f.first, f.text, again, -1), "reader repeat reads");
        DBtest (again.val.uniStringValue, GS::UniString ("Alpha"), "reader repeat same value");

        DBprnt ("SpecRegression value reader", "end");
    }

    void TestSpecValueEdges () {
        DBprnt ("SpecRegression values", "start");
        SpecFixture f;
        f.Text (f.first, f.text, "Alpha");
        ParamValue result;
        DBtest (Spec::SpecValueReader (f.context).Read (f.first, f.text, result, -1),
                "Spec ordinary ignores material argument and layer");
        DBtest (result.val.uniStringValue, GS::UniString ("Alpha"), "Spec ordinary original value");
        f.context.read.Get (f.first).Get (f.text).fromMaterial = true;
        ParamComposite composite;
        ParamValueComposite layer;
        const char *texts[] = {"0", "-2.5", "", "Material", "2147483648", "-2147483649"};
        for (const char *text : texts) {
            layer.val = text;
            composite.composite.Push (layer);
        }
        ParamDictComposite byName;
        byName.Add (f.text, composite);
        f.context.composite.Add (f.first, byName);
        const double numbers[] = {0, -2.5, 0, 0, 2147483648.0, -2147483649.0};
        const double integers[] = {0, -2, 0, 0, 2147483647.0, -2147483648.0};
        for (Int32 i = 0; i < 6; ++i) {
            const GS::UniString label = GS::UniString::Printf ("Spec material edge %d", i);
            DBtest (Spec::SpecValueReader (f.context).Read (f.first, f.text, result, i), label);
            DBtest (result.isValid, label + " valid");
            DBtest (result.val.uniStringValue, GS::UniString (texts[i]), label + " text");
            DBtest (result.val.doubleValue, numbers[i], label + " real");
            DBtest (result.val.rawDoubleValue, numbers[i], label + " raw real");
            DBtest (result.val.intValue, integers[i], label + " integer");
            DBtest (result.val.boolValue, i != 0 && i != 2, label + " bool");
            DBtest (result.val.canCalculate, i != 2 && i != 3, label + " calculable");
        }
        DBtest (Spec::SpecValueReader (f.context).Read (f.first, f.text, result, 100),
                "Spec past end after numeric result");
        DBtest (result.val.doubleValue == 0 && result.val.rawDoubleValue == 0 && result.val.intValue == 0 &&
                    !result.val.boolValue && !result.val.canCalculate && result.val.uniStringValue.IsEmpty (),
                "Spec past end clears all result fields");
        DBtest (f.context.read.Get (f.first).Get (f.text).val.uniStringValue,
                GS::UniString ("Alpha"),
                "Spec material source unchanged");
        DBtest (f.context.composite.Get (f.first).Get (f.text).composite.GetSize (), 6, "Spec layers unchanged");
        DBprnt ("SpecRegression values", "end");
    }

    // R4.1: нормализация описания - отдельная проверяемая единица.
    // Раньше она была телом AddRule и не тестировалась вовсе: все проверки били
    // по GetRuleFromDescription, который на входе уже ждёт НОРМАЛИЗОВАННУЮ строку.
    // Здесь фиксируется контракт: что именно считается "нормализованным".
    // R4.2: политика правила вынесена в ApplyRulePolicy. Проверяем её отдельно от
    // разбора групп - раньше политика была первым блоком парсера, и её нельзя было
    // проверить, не разбирая всё описание целиком.
    // Ожидания получены воспроизведением ветвления, а не подгонкой под вывод.
    // R4.2: выходная схема s() вынесена в ParseOutputSchema. Проверяем её отдельно:
    // требование ровно двух частей, срезание суффикса "[N]", пропуск пустых имён
    // и раздельное заполнение out_/out_sum_. Имена берутся в уже нормализованной
    // части после "s@@" - это контракт из R4.1.
    // Формат имён подтверждён существующим набором парсера: GDL:X -> {@gdl:x},
    // Property:Total -> {@property:total}.
    // Разбор группы g() после выноса в Spec::ParseGroups (). Ключевая особенность
    // контракта: часть ДО первого "g@@" (имя избранного) разбирается как группа с
    // одним параметром и всегда отбрасывается проверкой числа параметров - поэтому
    // тесты проверяют ЧИСЛО принятых групп, а не общее число частей.
    // Формат имён подтверждён существующим набором парсера: GDL:X -> {@gdl:x}.
    // Раскрытие группы после выноса в Spec::ExpandGroup (). Три независимых
    // исхода: разворот параметров-массивов ("[N]") в min_row групп, отбраковка
    // группы при несовпадении числа параметров с выходной схемой, размножение по
    // слоям для материалов и listdata.
    // Константы из Constants.hpp: max_group_mat = 50, max_group_lib = 100,
    // ARRAY_UNIC = 1, поэтому @arr_индекс имеет вид "@arr_1_1_1_1_1}".
    // Привязка выходных слотов элемента к схеме: Spec::OutSlotsMatchSchema ().
    // Слоты заполняются позиционно, по порядку Push (), поэтому единственная
    // доступная сверка - по числу набранных значений против размеров схемы,
    // вычисленных один раз до цикла. Ожидания сверены воспроизведением функции.
    void TestSpecOutSlots () {
        DBprnt ("SpecRegression out slots", "start");

        struct SlotCase {
            UInt32 out;      // сколько значений попало в out_param
            UInt32 sum;      // сколько значений попало в out_sum_param
            UInt32 outSlots; // ожидаемое число слотов выхода
            UInt32 sumSlots; // ожидаемое число слотов сумм
            bool ok;
            const char *label;
        };

        const SlotCase cases[] = {
            {1, 1, 1, 1, true, "exact match"},
            {2, 2, 2, 2, true, "two and two"},
            {2, 1, 2, 1, true, "two out one sum"},
            // Одна из сторон недобрана - привязка неполная, элемент не принимается.
            {1, 1, 2, 1, false, "out incomplete"},
            {2, 1, 2, 2, false, "sum incomplete"},
            {1, 1, 1, 2, false, "sum short in schema"},
            {2, 2, 1, 1, false, "schema smaller than element"},
            // Схема пустая: IsEmpty срабатывает раньше сравнения размеров.
            {0, 0, 0, 0, false, "empty schema"},
        };

        for (const SlotCase &test : cases) {
            Spec::Element element = {};
            for (UInt32 i = 0; i < test.out; i++) {
                ParamValue pv = {};
                element.out_param.Push (pv);
            }
            for (UInt32 i = 0; i < test.sum; i++) {
                ParamValue pv = {};
                element.out_sum_param.Push (pv);
            }
            const bool ok = Spec::OutSlotsMatchSchema (element, test.outSlots, test.sumSlots);
            DBtest (ok, test.ok, GS::UniString (test.label));
        }

        // Пустой элемент при непустой схеме - тот же отказ, но по другой причине.
        {
            Spec::Element element = {};
            DBtest (Spec::OutSlotsMatchSchema (element, 1, 1), false, "empty element");
        }

        DBprnt ("SpecRegression out slots", "end");
    }

    // Привязка слотов группы к полям: Spec::PrepareSlotBindings (). Это ОДИН
    // проход до цикла по элементам, поэтому контракт проверяется здесь, а не
    // по итогу GetElementsForRule. Ключевые свойства:
    //   - ровно одна привязка на каждую группу, в том же порядке;
    //   - имена НЕ копируются (хранится указатель на поле группы), поэтому
    //     правка поля группы после подготовки видна в привязке;
    //   - признак isSumLiteral ставится по имени "1" ОДИН раз, а не в цикле;
    //   - sizesMatchSchema отражает число ПОЛЕЙ группы, а не число
    //     фактически набранных значений элемента.
    void TestSpecSlotBindings () {
        DBprnt ("SpecRegression slot bindings", "start");

        // Пустое правило: ни групп, ни привязок.
        {
            Spec::SpecRule rule;
            const GS::Array<Spec::GroupSlotBinding> bindings = Spec::PrepareSlotBindings (rule);
            DBtest (bindings.GetSize (), 0, "empty rule no bindings");
        }

        // Обычная группа: одно выходное поле, одно поле суммы.
        {
            Spec::SpecRule rule;
            rule.out_paramrawname.Push ("{@property:out-a}");
            rule.out_paramrawname.Push ("{@property:out-b}");
            rule.out_sum_paramrawname.Push ("{@property:sum-a}");
            Spec::GroupSpec group;
            group.out_paramrawname.Push ("{@property:text}");
            group.sum_paramrawname.Push ("{@property:quantity}");
            rule.groups.Push (group);
            const GS::Array<Spec::GroupSlotBinding> bindings = Spec::PrepareSlotBindings (rule);
            DBtest (bindings.GetSize (), 1, "one group one binding");
            if (!bindings.IsEmpty ()) {
                const Spec::GroupSlotBinding &b = bindings[0];
                DBtest (b.outSlots.GetSize (), 1, "group has one out field");
                DBtest (b.sumSlots.GetSize (), 1, "group has one sum field");
                DBtest (b.schemaOutSlots, 2, "schema out slots");
                DBtest (b.schemaSumSlots, 1, "schema sum slots");
                // Одно поле против двух слотов схемы - группа не наполнит схему
                // целиком, но это предупреждение, а не запрет в цикле.
                DBtest (b.sizesMatchSchema, false, "field count differs from schema");
                if (!b.outSlots.IsEmpty ()) {
                    DBtest (b.outSlots[0].isSumLiteral, false, "out slot is not literal");
                    DBtest (*b.outSlots[0].rawname, GS::UniString ("{@property:text}"), "out slot name");
                }
                if (!b.sumSlots.IsEmpty ())
                    DBtest (b.sumSlots[0].isSumLiteral, false, "sum slot is not literal");
            }
        }

        // Совпадение числа полей со схемой + константная сумма "1".
        {
            Spec::SpecRule rule;
            rule.out_paramrawname.Push ("{@property:out-a}");
            rule.out_sum_paramrawname.Push ("{@property:sum-a}");
            rule.out_sum_paramrawname.Push ("{@property:sum-b}");
            Spec::GroupSpec group;
            group.out_paramrawname.Push ("{@property:text}");
            group.sum_paramrawname.Push ("1");
            group.sum_paramrawname.Push ("{@property:quantity}");
            rule.groups.Push (group);
            const GS::Array<Spec::GroupSlotBinding> bindings = Spec::PrepareSlotBindings (rule);
            if (!bindings.IsEmpty ()) {
                const Spec::GroupSlotBinding &b = bindings[0];
                DBtest (b.sizesMatchSchema, true, "field count equals schema");
                DBtest (b.outSlots.GetSize (), 1, "matched out fields");
                DBtest (b.sumSlots.GetSize (), 2, "matched sum fields");
                if (b.sumSlots.GetSize () == 2) {
                    DBtest (b.sumSlots[0].isSumLiteral, true, "literal one detected");
                    DBtest (b.sumSlots[1].isSumLiteral, false, "real sum not literal");
                    DBtest (*b.sumSlots[0].rawname, GS::UniString ("1"), "literal name kept");
                    DBtest (*b.sumSlots[1].rawname, GS::UniString ("{@property:quantity}"), "sum name kept");
                }
            }
        }

        // Несколько групп: порядок привязок совпадает с порядком групп,
        // в том числе для групп, созданных ExpandGroup по слоям.
        {
            Spec::SpecRule rule;
            rule.out_paramrawname.Push ("{@property:out-a}");
            rule.out_sum_paramrawname.Push ("{@property:sum-a}");
            for (UInt32 layer = 0; layer < 3; layer++) {
                Spec::GroupSpec group;
                group.n_layer = layer;
                group.fromMaterial = true;
                group.out_paramrawname.Push ("{@material:layer-auto-" + GS::UniString::Printf ("%u", layer) + "}");
                group.sum_paramrawname.Push ("{@property:quantity}");
                rule.groups.Push (group);
            }
            const GS::Array<Spec::GroupSlotBinding> bindings = Spec::PrepareSlotBindings (rule);
            DBtest (bindings.GetSize (), 3, "layered groups bindings");
            for (UInt32 i = 0; i < bindings.GetSize (); i++) {
                DBtest (bindings[i].sizesMatchSchema, true, "layered group sizes match");
                if (!bindings[i].outSlots.IsEmpty ()) {
                    const GS::UniString expected = "{@material:layer-auto-" + GS::UniString::Printf ("%u", i) + "}";
                    DBtest (*bindings[i].outSlots[0].rawname, expected, "layered group name order");
                }
            }
        }

        // Имена не копируются: привязка ссылается на поле группы, поэтому
        // изменение поля после PrepareSlotBindings видно в привязке. Это
        // контракт, а не оптимизация - цикл исполнения группы не меняет.
        {
            Spec::SpecRule rule;
            rule.out_paramrawname.Push ("{@property:out-a}");
            rule.out_sum_paramrawname.Push ("{@property:sum-a}");
            Spec::GroupSpec group;
            group.out_paramrawname.Push ("{@property:text}");
            group.sum_paramrawname.Push ("{@property:quantity}");
            rule.groups.Push (group);
            GS::Array<Spec::GroupSlotBinding> bindings = Spec::PrepareSlotBindings (rule);
            rule.groups[0].out_paramrawname[0] = "{@property:changed}";
            if (!bindings.IsEmpty () && !bindings[0].outSlots.IsEmpty ()) {
                DBtest (
                    *bindings[0].outSlots[0].rawname, GS::UniString ("{@property:changed}"), "binding is not a copy");
            }
            // Повторная подготовка обязана увидеть изменённое поле.
            bindings = Spec::PrepareSlotBindings (rule);
            if (!bindings.IsEmpty () && !bindings[0].outSlots.IsEmpty ()) {
                DBtest (
                    *bindings[0].outSlots[0].rawname, GS::UniString ("{@property:changed}"), "reprepare sees new name");
            }
        }

        // Пустые поля группы: ни одного слота, схема при этом может быть
        // непустой - sizesMatchSchema ложно, цикл элемент не примет.
        {
            Spec::SpecRule rule;
            rule.out_paramrawname.Push ("{@property:out-a}");
            rule.out_sum_paramrawname.Push ("{@property:sum-a}");
            Spec::GroupSpec group;
            rule.groups.Push (group);
            const GS::Array<Spec::GroupSlotBinding> bindings = Spec::PrepareSlotBindings (rule);
            DBtest (bindings.GetSize (), 1, "group without fields binding");
            if (!bindings.IsEmpty ()) {
                DBtest (bindings[0].outSlots.IsEmpty (), true, "no out slots");
                DBtest (bindings[0].sumSlots.IsEmpty (), true, "no sum slots");
                DBtest (bindings[0].sizesMatchSchema, false, "empty group does not match schema");
            }
        }

        DBprnt ("SpecRegression slot bindings", "end");
    }

    void TestSpecExpandGroup () {
        DBprnt ("SpecRegression expand group", "start");

        const GS::UniString P ("{@gdl:p}"), U ("{@gdl:u}"), F ("{@gdl:f}"), Q ("{@gdl:q}");

        // Обычная группа без массивов: ровно одна группа, n_layer не задан.
        {
            Spec::SpecRule rule = {};
            rule.out_paramrawname.Push (P);
            rule.out_sum_paramrawname.Push (Q);
            Spec::GroupSpec group = {};
            group.out_paramrawname.Push (P);
            group.unic_paramrawname.Push (U);
            group.flag_paramrawname = F;
            group.sum_paramrawname.Push (Q);

            Spec::ExpandGroup (group, 0, rule);

            DBtest (rule.groups.GetSize (), 1, "plain group count");
            DBtest (group.is_Valid, true, "plain group valid");
            if (rule.groups.GetSize () == 1) {
                DBtest (rule.groups[0].out_paramrawname[0], P, "plain group out");
                DBtest (rule.groups[0].flag_paramrawname, F, "plain group flag");
            }
        }

        // Несовпадение числа параметров для выхода: группа отбраковывается,
        // в rule.groups ничего не попадает, но сам парсер продолжает работу.
        {
            Spec::SpecRule rule = {};
            rule.out_paramrawname.Push (P);
            rule.out_paramrawname.Push (P); // ожидаем два, а в группе один
            rule.out_sum_paramrawname.Push (Q);
            Spec::GroupSpec group = {};
            group.out_paramrawname.Push (P);
            group.unic_paramrawname.Push (U);
            group.sum_paramrawname.Push (Q);

            Spec::ExpandGroup (group, 0, rule);

            DBtest (rule.groups.GetSize (), 0, "out mismatch adds nothing");
            DBtest (group.is_Valid, false, "out mismatch invalidates group");
        }

        // Несовпадение числа параметров количеств - тот же исход.
        {
            Spec::SpecRule rule = {};
            rule.out_paramrawname.Push (P);
            rule.out_sum_paramrawname.Push (Q);
            rule.out_sum_paramrawname.Push (Q); // ожидаем два, а в группе ни одного
            Spec::GroupSpec group = {};
            group.out_paramrawname.Push (P);
            group.unic_paramrawname.Push (U);

            Spec::ExpandGroup (group, 0, rule);

            DBtest (rule.groups.GetSize (), 0, "sum mismatch adds nothing");
            DBtest (group.is_Valid, false, "sum mismatch invalidates group");
        }

        // Материалы: группа размножается на max_group_mat слоёв с номерами 0..49.
        {
            Spec::SpecRule rule = {};
            rule.out_paramrawname.Push (P);
            rule.out_sum_paramrawname.Push (Q);
            Spec::GroupSpec group = {};
            group.out_paramrawname.Push (P);
            group.unic_paramrawname.Push (U);
            group.sum_paramrawname.Push (Q);
            group.fromMaterial = true;

            Spec::ExpandGroup (group, 0, rule);

            DBtest (rule.groups.GetSize (), 50, "material group count");
            if (rule.groups.GetSize () == 50) {
                DBtest (rule.groups[0].n_layer, 0, "material first layer");
                DBtest (rule.groups[49].n_layer, 49, "material last layer");
            }
        }

        // Данные ведомостей: max_group_lib слоёв с номерами 0..99.
        {
            Spec::SpecRule rule = {};
            rule.out_paramrawname.Push (P);
            rule.out_sum_paramrawname.Push (Q);
            Spec::GroupSpec group = {};
            group.out_paramrawname.Push (P);
            group.unic_paramrawname.Push (U);
            group.sum_paramrawname.Push (Q);
            group.fromLibData = true;

            Spec::ExpandGroup (group, 0, rule);

            DBtest (rule.groups.GetSize (), 100, "libdata group count");
            if (rule.groups.GetSize () == 100) {
                DBtest (rule.groups[99].n_layer, 99, "libdata last layer");
            }
        }

        // Разворот массива: min_row = 2 даёт две группы, по одной на строку массива.
        // Порядок задан кодом: сперва срезается суффикс "[N]" ("{@gdl:p[7]}" ->
        // "{@gdl:p}"), затем ReplaceAll подставляет вместо "} хвост
        // "@arr_строка_строка_1_1_ARRAY_UNIC}". Скобка в имени одна, поэтому в
        // результате она оказывается в конце: "{@gdl:p@arr_1_1_1_1_1}".
        {
            Spec::SpecRule rule = {};
            rule.out_paramrawname.Push (P);
            rule.out_sum_paramrawname.Push (Q);
            Spec::GroupSpec group = {};
            group.out_paramrawname.Push (P + "[7]");
            group.unic_paramrawname.Push (U);
            group.sum_paramrawname.Push (Q);

            Spec::ExpandGroup (group, 2, rule);

            DBtest (rule.groups.GetSize (), 2, "array group count");
            if (rule.groups.GetSize () == 2) {
                DBtest (rule.groups[0].out_paramrawname[0], "{@gdl:p@arr_1_1_1_1_1}", "array row 1");
                DBtest (rule.groups[1].out_paramrawname[0], "{@gdl:p@arr_2_2_1_1_1}", "array row 2");
            }
        }

        DBprnt ("SpecRegression expand group", "end");
    }

    void TestSpecGroups () {
        DBprnt ("SpecRegression groups", "start");

        struct GroupCase {
            const char *readPart;
            UInt32 nOut;   // сколько имён ждём в выходной схеме s()
            UInt32 nSum;   // сколько имён ждём в части количеств s()
            UInt32 groups; // сколько групп должно быть принято
            bool ok;       // завершился ли разбор (false = отказ внутри разбора)
            const char *label;
        };

        const GroupCase cases[] = {
            // Канон из TestSpecAddRule: 1 уникальный, 1 на выход, флаг, 1 количество.
            {"Fav;g@@u;p;f;q", 1, 1, 1, true, "canonical four parts"},
            // Суммы не хватает - дополняется "1", группа принимается.
            {"Fav;g@@u;p;f", 1, 1, 1, true, "sum padded with one"},
            // part трактуется ПО ИНДЕКСУ, а не по смыслу: здесь 3 части, значит
            // q попадает в flag, а не в sum. Это поведение зафиксировано.
            {"Fav;g@@u;p;q", 1, 1, 1, true, "third part is flag not sum"},
            // Число параметров для выхода не совпало со схемой -> группа отброшена.
            {"Fav;g@@u;p;w;z", 2, 1, 0, true, "out count mismatch dropped"},
            // Нет точки с запятой внутри группы -> разбор невозможен.
            {"Fav;g@@u", 1, 1, 0, false, "single part rejected"},
            // Две настоящие группы, обе валидны.
            {"Fav;g@@u1;p;f;q1@@g@@u2;p;f;q2", 1, 1, 2, true, "two groups"},
            // Ни одной части с ";" - отказ на первой же группе.
            {"Fav", 1, 1, 0, false, "no group marker rejected"},
        };

        for (const GroupCase &test : cases) {
            const GS::UniString label (test.label);
            Spec::SpecRule rule = {};
            // out_sum задаёт ожидаемое число параметров количеств; out_paramrawname
            // проверяется отдельно, здесь достаточно корректного размера массивов.
            for (UInt32 i = 0; i < test.nOut; i++)
                rule.out_paramrawname.Push ("{@gdl:o}");
            for (UInt32 i = 0; i < test.nSum; i++)
                rule.out_sum_paramrawname.Push ("{@gdl:s}");

            GS::Array<GS::UniString> scratch;
            const GS::UniString readPart (test.readPart);
            const bool ok = Spec::ParseGroups (readPart, scratch, rule);

            // ok - разбор выполнен, а не "группы приняты": группа с неверным числом
            // параметров отбрасывается, но разбор продолжается. Признак принятия -
            // число групп, его и проверяем отдельно.
            DBtest (ok, test.ok, label + " parse completed");
            DBtest (rule.groups.GetSize (), test.groups, label + " group count");
        }

        // Проверка состава группы на каноне: уникальный/выход/флаг/количество.
        {
            Spec::SpecRule rule = {};
            rule.out_paramrawname.Push ("{@gdl:o}");
            rule.out_sum_paramrawname.Push ("{@gdl:s}");
            GS::Array<GS::UniString> scratch;
            const GS::UniString readPart ("Fav;g@@u;p;f;q");
            const bool ok = Spec::ParseGroups (readPart, scratch, rule);
            DBtest (ok, true, "canonical accepted");
            DBtest (rule.groups.GetSize (), 1, "canonical group count");
            if (rule.groups.GetSize () == 1) {
                const Spec::GroupSpec &g = rule.groups[0];
                DBtest (g.unic_paramrawname.GetSize (), 1, "canonical unic count");
                DBtest (g.unic_paramrawname[0], GS::UniString ("{@gdl:u}"), "canonical unic name");
                DBtest (g.out_paramrawname.GetSize (), 1, "canonical out count");
                DBtest (g.out_paramrawname[0], GS::UniString ("{@gdl:p}"), "canonical out name");
                DBtest (g.flag_paramrawname, GS::UniString ("{@gdl:f}"), "canonical flag name");
                DBtest (g.sum_paramrawname.GetSize (), 1, "canonical sum count");
                DBtest (g.sum_paramrawname[0], GS::UniString ("{@gdl:q}"), "canonical sum name");
            }
        }

        // Часть до первого "g@@" (имя избранного) НЕ становится рабочей группой:
        // у неё нет точки с запятой, поэтому разбор прерывается на первой части.
        {
            Spec::SpecRule rule = {};
            rule.out_paramrawname.Push ("{@gdl:o}");
            rule.out_sum_paramrawname.Push ("{@gdl:s}");
            GS::Array<GS::UniString> scratch;
            const GS::UniString readPart ("Fav");
            const bool ok = Spec::ParseGroups (readPart, scratch, rule);
            DBtest (ok, false, "favorite name alone rejected");
        }

        DBprnt ("SpecRegression groups", "end");
    }

    void TestSpecOutputSchema () {
        DBprnt ("SpecRegression output schema", "start");

        struct SchemaCase {
            const char *writePart;
            bool ok;
            GS::Array<GS::UniString> out;
            GS::Array<GS::UniString> sum;
            const char *label;
        };

        const SchemaCase cases[] = {
            {"x;y", true, {"{@gdl:x}"}, {"{@gdl:y}"}, "single pair"},
            {"GDL:X;Property:Total", true, {"{@gdl:x}"}, {"{@property:total}"}, "typed names"},
            {"x,y;q,w", true, {"{@gdl:x}", "{@gdl:y}"}, {"{@gdl:q}", "{@gdl:w}"}, "two and two"},
            {"x[3];y[2]", true, {"{@gdl:x}"}, {"{@gdl:y}"}, "array suffix cut"},
            // filter_empty=true убирает пустые части ДО подсчёта, поэтому "x,,y"
            // даёт два имени, а не пустой слот посередине.
            {"x,,y;w", true, {"{@gdl:x}", "{@gdl:y}"}, {"{@gdl:w}"}, "empty name skipped"},
            // Пустая часть исчезает ДО проверки "ровно две", поэтому "x;" и ";y"
            // дают одну часть и отвергаются - пустой слот недопустим.
            {";y", false, {}, {}, "empty output part rejected"},
            {"x;", false, {}, {}, "empty sum part rejected"},
            {"x", false, {}, {}, "one part rejected"},
            {"x;y;z", false, {}, {}, "three parts rejected"},
            {"", false, {}, {}, "empty rejected"},
            // Хвостая запятая внутри части НЕ отвергает правило: часть непуста,
            // пустое имя уходит из неё, и схема остаётся из одного слота.
            {"x,;y", true, {"{@gdl:x}"}, {"{@gdl:y}"}, "trailing comma tolerated"},
        };
        for (const SchemaCase &test : cases) {
            Spec::SpecRule rule = {};
            GS::Array<GS::UniString> scratch;
            const bool result = Spec::ParseOutputSchema (test.writePart, scratch, rule);
            const GS::UniString label = GS::UniString ("Spec schema ") + test.label;
            DBtest (result, test.ok, label + " accepted");
            // parseValid обязан совпадать с принятием: отказ = невалидное правило.
            DBtest (rule.parseValid, test.ok, label + " parse flag");
            DBtest (rule.out_paramrawname == test.out, label + " output names");
            DBtest (rule.out_sum_paramrawname == test.sum, label + " sum names");
        }
        // Схема заполняет ТОЛЬКО выходные поля и не трогает уже разобранные части.
        {
            Spec::SpecRule rule = {};
            rule.favorite_name = "Fav";
            rule.isKM = true;
            GS::Array<GS::UniString> scratch;
            const bool result = Spec::ParseOutputSchema ("x;y", scratch, rule);
            DBtest (result, true, "Spec schema accepted plain");
            DBtest (rule.favorite_name, GS::UniString ("Fav"), "Spec schema favorite intact");
            DBtest (rule.isKM, true, "Spec schema KM flag intact");
        }
        // Вызывается из парсера: описание без части s@@ обязано дать parseValid=false.
        {
            GS::UniString description = "Spec_rule{Fav;g@@u;p;f;q}";
            const Spec::SpecRule rule = Spec::GetRuleFromDescription (description);
            DBtest (rule.parseValid, false, "Spec schema wired missing s@@");
            DBtest (rule.out_paramrawname.IsEmpty () && rule.out_sum_paramrawname.IsEmpty (),
                    "Spec schema wired no output");
        }
        DBprnt ("SpecRegression output schema", "end");
    }

    void TestSpecPolicy () {
        DBprnt ("SpecRegression policy", "start");

        struct PolicyCase {
            const char *prefix;
            bool delete_old;
            bool stop_on_error;
            bool only_visible;
            bool isKM;
            bool isKZH;
            const char *label;
        };

        // Значения по умолчанию в SpecRule: delete_old=false, stop_on_error=true,
        // only_visible=true, isKM=false, isKZH=false. Политика их перекрывает.
        const PolicyCase cases[] = {
            {"Spec_rule", false, true, true, false, false, "base defaults"},
            {"Spec_rule_v2", true, true, true, false, false, "v2 delete old"},
            {"Spec_rule_v3", true, false, false, false, false, "v3 delete old and no stop"},
            {"Spec_rule_km", false, false, true, true, false, "KM"},
            {"Spec_rule_kzh", false, false, true, false, true, "KZH"},
            // Порядок ветвления значим: v3 проверяется раньше KM, поэтому строка с
            // обоими маркерами получает политику v3, а не KM.
            {"Spec_rule_v3_km", true, false, false, false, false, "v3 wins over km"},
            {"Spec_rule_km_v2", false, false, true, true, false, "KM then overwritten policy"},
            // Сравнение идёт по МЕСТУ "pec_rule", а не по префиксу целиком,
            // поэтому вхождение в любой части строки тоже даёт политику.
            {"XXpec_rule_v2XX", true, true, true, false, false, "substring match"},
            {"Spec_rule_V2", true, true, true, false, false, "case insensitive"},
            {"Spec_rule_KZH", false, false, true, false, true, "case insensitive KZH"},
        };
        for (const PolicyCase &test : cases) {
            Spec::SpecRule rule = {};
            Spec::ApplyRulePolicy (test.prefix, rule);
            const GS::UniString label = GS::UniString ("Spec policy ") + test.label;
            DBtest (rule.delete_old, test.delete_old, label + " delete old");
            DBtest (rule.stop_on_error, test.stop_on_error, label + " stop flag");
            DBtest (rule.only_visible, test.only_visible, label + " only visible");
            DBtest (rule.isKM, test.isKM, label + " KM");
            DBtest (rule.isKZH, test.isKZH, label + " KZH");
        }
        // Политика не трогает разобранные части правила: функция работает по
        // описанию, но обязана оставить уже разобранное содержимое как есть.
        {
            Spec::SpecRule rule = {};
            rule.favorite_name = "Fav";
            rule.out_paramrawname.Push ("{@gdl:x}");
            Spec::ApplyRulePolicy ("Spec_rule_v3", rule);
            DBtest (rule.favorite_name, GS::UniString ("Fav"), "Spec policy keeps favorite");
            DBtest (rule.out_paramrawname.GetSize (), 1, "Spec policy keeps output");
            DBtest (rule.parseValid, true, "Spec policy keeps parse flag");
        }
        // Политика вызывается из парсера: тот же префикс через полный разбор даёт
        // те же признаки (проверка, что вызов не потерян при выделении).
        // Строка обязана быть НОРМАЛИЗОВАНА - это контракт GetRuleFromDescription,
        // закреплённый в R4.1; сырой "g(u;p;f;q)s(x;y)" парсер не принимает.
        {
            GS::UniString description = "Spec_rule_km{Fav;g@@u;p;f;q@@s@@x;y)}";
            const Spec::SpecRule rule = Spec::GetRuleFromDescription (description);
            DBtest (rule.parseValid, true, "Spec policy wired valid");
            DBtest (rule.isKM && !rule.isKZH, "Spec policy wired KM");
            DBtest (!rule.delete_old && !rule.stop_on_error && rule.only_visible, "Spec policy wired policy fields");
        }
        DBprnt ("SpecRegression policy", "end");
    }

    void TestSpecNormalize () {
        DBprnt ("SpecRegression normalize", "start");

        // Порядок замен существенен, поэтому проверяем и каждый вид вызова, и то,
        // что "gl("/ "gm(" не схлопываются в общий "g(" (иначе получилось бы
        // "g@@libdata@(" - группа с открытой скобкой в имени).
        struct NormCase {
            const char *source;
            const char *expected;
            const char *label;
        };

        // Ожидания получены НЕ умозрительно: тот же алгоритм воспроизведён
        // пошагово и сверен с ключом "Fav;g@@u;p;f;q@@s@@x;y)", который уже
        // закреплён в наборе TestSpecAddRule. Обратите внимание: закрывающая
        // скобка ")" остаётся в теле - нормализация её НЕ убирает, парсер
        // добивает её сам (description.Trim (')') в GetRuleFromDescription).
        const NormCase cases[] = {
            {"Spec_rule{Fav;g(a;b;c)}", "Spec_rule{Fav;g@@a;b;c)}", "plain group"},
            {"Spec_rule{Fav;g (a;b;c)}", "Spec_rule{Fav;g@@a;b;c)}", "space before paren trimmed"},
            {"Spec_rule{Fav; g (a;b;c)}", "Spec_rule{Fav;g@@a;b;c)}", "space before g and paren"},
            {"Spec_rule{Fav;s(a;b;c)}", "Spec_rule{Fav;s@@a;b;c)}", "summary marker"},
            {"Spec_rule{Fav;gl(a;b;c)}", "Spec_rule{Fav;g@@libdata@a;b;c)}", "libdata group"},
            {"Spec_rule{Fav;gm(a;b;c)}", "Spec_rule{Fav;g@@Material_all@a;b;c)}", "material group"},
            {"Spec_rule{Fav;g(a;b;c))s(a;b;c)}", "Spec_rule{Fav;g@@a;b;c)@@s@@a;b;c)}", "double close"},
            {"Spec_rule {Fav;g(a)}", "Spec_rule{Fav;g@@a)}", "space before brace start"},
            {"Spec_rule{Fav ;g(a)}", "Spec_rule{Fav;g@@a)}", "space before semicolon"},
            {"Spec_rule{Fav;g(a) }", "Spec_rule{Fav;g@@a)}", "space before brace end"},
            {"Spec_rule{Fav;g(a;b;c)", "Spec_rule{Fav;g@@a;b;c)", "unbalanced kept"},
            {"Spec_rule{Fav;\ng(a;b;c)\n}", "Spec_rule{Fav;g@@a;b;c)}", "line feeds removed"},
            {"Spec_rule{Fav;\rg(a;b;c)\r}", "Spec_rule{Fav;g@@a;b;c)}", "carriage returns removed"},
            {"Spec_rule{Fav;\tg(a;b;c)\t}", "Spec_rule{Fav;g@@a;b;c)}", "tabs removed"},
            {"Spec_rule{Fav;g(a)        s(b)}", "Spec_rule{Fav;g@@a@@s@@b)}", "many spaces collapse"},
            {"Spec_rule { Fav ;\n\t g (u;p;f;q) s (x;y) }", "Spec_rule{Fav;g@@u;p;f;q@@s@@x;y)}", "layout noise"},
            // Пробел перед скобкой после ")" - единственное место, где порядок
            // замен заметен: строка остаётся с " @@s@@", парсер такое терпит.
            {"Spec_rule{Fav;g(a))s(b)}", "Spec_rule{Fav;g@@a)@@s@@b)}", "canonical double close"},
        };
        for (const NormCase &test : cases) {
            const GS::UniString source = GS::UniString (test.source);
            const GS::UniString result = Spec::NormalizeRuleDescription (source);
            const GS::UniString label = GS::UniString ("Spec normalize ") + test.label;
            DBtest (result, GS::UniString (test.expected), label);
            // Исходная строка не должна меняться - от этого зависит AddRule,
            // который переиспользует definition.description на каждом вызове.
            DBtest (source, GS::UniString (test.source), label + " source intact");
        }
        // Нормализованная строка обязана быть принята парсером без потерь:
        // сквозной путь "нормализация -> разбор" вместо ручной передачи строки.
        {
            const GS::UniString raw = "Spec_rule{Fav;\r\ng (a;b;f;c))s (x;y)\n}";
            GS::UniString normalized = Spec::NormalizeRuleDescription (raw);
            const Spec::SpecRule rule = Spec::GetRuleFromDescription (normalized);
            DBtest (rule.parseValid, true, "Spec normalize end to end valid");
            DBtest (rule.favorite_name, GS::UniString ("Fav"), "Spec normalize end to end favorite");
            DBtest (rule.groups.GetSize (), 1, "Spec normalize end to end group count");
            DBtest (rule.out_paramrawname.GetSize (), 1, "Spec normalize end to end output count");
            if (rule.groups.GetSize () == 1) {
                const Spec::GroupSpec &group = rule.groups[0];
                DBtest (group.unic_paramrawname.GetSize () == 1 && group.unic_paramrawname[0] == "{@gdl:a}",
                        "Spec normalize end to end unique");
                DBtest (group.out_paramrawname.GetSize () == 1 && group.out_paramrawname[0] == "{@gdl:b}",
                        "Spec normalize end to end read");
                DBtest (group.flag_paramrawname, GS::UniString ("{@gdl:f}"), "Spec normalize end to end flag");
            }
        }
        // Пустая и безгрузовая строки не должны падать.
        DBtest (Spec::NormalizeRuleDescription (GS::UniString ("")).IsEmpty (), true, "Spec normalize empty");
        DBtest (
            Spec::NormalizeRuleDescription (GS::UniString ("   ")), GS::UniString (" "), "Spec normalize only spaces");
        // Пробелы ВНУТРИ скобок нормализация не трогает - они уходят в имя
        // параметра, и это историческое поведение, а не дефект.
        DBtest (Spec::NormalizeRuleDescription (GS::UniString ("Spec_rule{Fav;g ( a )}")),
                GS::UniString ("Spec_rule{Fav;g@@ a )}"),
                "Spec normalize inner spaces kept");
        DBprnt ("SpecRegression normalize", "end");
    }

    void TestSpecParser () {
        DBprnt ("SpecRegression parser", "start");
        const char *prefixes[] = {"Spec_rule", "Spec_rule_v2", "Spec_rule_v3", "Spec_rule_km", "Spec_rule_kzh"};
        for (Int32 i = 0; i < 5; ++i) {
            GS::UniString description = GS::UniString (prefixes[i]) + "{Fav;g@@u;p;f;q@@s@@x;y)}";
            const Spec::SpecRule rule = Spec::GetRuleFromDescription (description);
            const GS::UniString label = GS::UniString ("Spec parser ") + prefixes[i];
            DBtest (rule.parseValid, label + " valid");
            DBtest (rule.favorite_name, GS::UniString ("Fav"), label + " favorite");
            DBtest (rule.delete_old, i == 1 || i == 2, label + " delete old");
            DBtest (rule.stop_on_error, i < 2, label + " stop");
            DBtest (rule.only_visible, i != 2, label + " visible");
            DBtest (rule.isKM, i == 3, label + " KM");
            DBtest (rule.isKZH, i == 4, label + " KZH");
            DBtest (rule.groups.GetSize (), 1, label + " group count");
            DBtest (rule.out_paramrawname.GetSize (), 1, label + " output count");
            DBtest (rule.out_sum_paramrawname.GetSize (), 1, label + " sum count");
            // R4.3: парсер больше НЕ мутирует вход - обрезки идут на локальной
            // копии. Раньше здесь проверялось, что строка «съедена» парсером;
            // теперь контракт обратный - вызывающий сохраняет свою строку.
            DBtest (description, GS::UniString (prefixes[i]) + "{Fav;g@@u;p;f;q@@s@@x;y)}", label + " input intact");
            if (rule.groups.GetSize () == 1) {
                const Spec::GroupSpec &group = rule.groups[0];
                DBtest (group.unic_paramrawname.GetSize () == 1 && group.unic_paramrawname[0] == "{@gdl:u}",
                        label + " unique");
                DBtest (group.out_paramrawname.GetSize () == 1 && group.out_paramrawname[0] == "{@gdl:p}",
                        label + " read");
                DBtest (group.sum_paramrawname.GetSize () == 1 && group.sum_paramrawname[0] == "{@gdl:q}",
                        label + " sum source");
                DBtest (group.flag_paramrawname, GS::UniString ("{@gdl:f}"), label + " flag");
                DBtest (group.is_Valid && !group.fromMaterial && !group.fromLibData && group.n_layer == 0,
                        label + " ordinary group");
            }
            DBtest (rule.elements.IsEmpty () && rule.exsist_elements.IsEmpty (), label + " no elements");
            // Ключ словаря строится из той же строки ПОСЛЕ разбора - раньше это
            // было невозможно, потому что парсер оставлял строку в обрезанном виде.
            const GS::UniString keyAfter = description.GetSubstring (CHARBRACESTART, CHARBRACEEND, 0);
            DBtest (keyAfter.Contains (GS::UniString ("s@@x;y)")), label + " key reusable after parse");
        }

        struct ParserCase {
            const char *body;
            bool valid;
            UInt32 groups;
            const char *label;
        };

        const ParserCase cases[] = {{"g@@u;p;f;q", false, 0, "no summary"},
                                    {"g@@u;p;f;q@@s@@x", false, 0, "short summary"},
                                    {"g@@u;p;f;q@@s@@x;y;z", false, 0, "long summary"},
                                    {"g@@u@@s@@x;y", false, 0, "short group"},
                                    {"g@@u;p1,p2;f;q@@s@@x;y", false, 0, "output mismatch"},
                                    {"g@@u;p;f;q1,q2@@s@@x;y", false, 0, "sum mismatch"},
                                    {"g@@u;p;f;q@@s@@%;y", false, 0, "empty output"},
                                    {"g@@u;p;f;q@@s@@x;%", false, 0, "empty sum output"},
                                    {"g@@u;p;f@@s@@x;y1,y2", true, 1, "implicit counts"},
                                    {"g@@u;p;f;q@@s@@x;y1,y2", true, 1, "partial implicit count"},
                                    {"g@@u;p,-;f;q@@s@@x;y", true, 1, "dash read skipped"},
                                    {"g@@u;p;f;q;ignored@@s@@x;y", true, 1, "extra group field"},
                                    {"g@@u1;p1;f1;q1@@g@@u2;p2;f2;q2@@s@@x;y", true, 2, "two groups"},
                                    {"g@@u;p1,p2;f;q@@g@@u2;p2;f2;q2@@s@@x;y", true, 1, "invalid group dropped"},
                                    {"u;p;f;q@@s@@x;y", true, 1, "group marker optional"},
                                    {"g@@u;p;f;q@@s@@x;y@@s@@ignored", true, 1, "extra summary ignored"},
                                    {"g@@u;p;f;q@@s@@x[3];y[2]", true, 1, "output array suffix removed"},
                                    {"g@@u;p[2];f;q@@s@@x;y", true, 2, "array two rows"},
                                    {"g@@u[3];p[2];f[4];q[5]@@s@@x;y", true, 2, "minimum array rows"},
                                    {"g@@u;p[bad];f;q@@s@@x;y", true, 10, "array bad default"},
                                    {"g@@u;p[0];f;q@@s@@x;y", true, 10, "array zero default"},
                                    {"g@@u;p[-1];f;q@@s@@x;y", true, 10, "array negative default"},
                                    {"g@@u;p[51];f;q@@s@@x;y", true, 10, "array too large default"},
                                    {"g@@u;p[1];f;q@@s@@x;y", true, 1, "array lower bound"},
                                    {"g@@u;p[50];f;q@@s@@x;y", true, 50, "array upper bound"},
                                    {"g@@Material_all@u;p;f;q@@s@@x;y", true, 50, "material groups"},
                                    {"g@@libdata@u;p;f;q@@s@@x;y", true, 100, "libdata groups"}};
        for (const ParserCase &test : cases) {
            GS::UniString description = GS::UniString ("Spec_rule{Fav;") + test.body + "}";
            const Spec::SpecRule rule = Spec::GetRuleFromDescription (description);
            const GS::UniString label = GS::UniString ("Spec parser ") + test.label;
            DBtest (rule.parseValid, test.valid, label + " valid");
            DBtest (rule.groups.GetSize (), test.groups, label + " groups");
            if (test.groups == 50 && GS::UniString (test.label) == "material groups") {
                for (UInt32 i = 0; i < rule.groups.GetSize (); ++i)
                    DBtest (rule.groups[i].fromMaterial && !rule.groups[i].fromLibData &&
                                rule.groups[i].n_layer == static_cast<Int32> (i),
                            label + GS::UniString::Printf (" layer %u", i));
            }
            if (test.groups == 100) {
                for (UInt32 i = 0; i < rule.groups.GetSize (); ++i)
                    DBtest (rule.groups[i].fromLibData && !rule.groups[i].fromMaterial &&
                                rule.groups[i].n_layer == static_cast<Int32> (i),
                            label + GS::UniString::Printf (" layer %u", i));
            }
        }
        const char *uniqueAliases[] = {"-", "\"-\"", "\"\""};
        for (const char *alias : uniqueAliases) {
            GS::UniString description = GS::UniString ("Spec_rule{Fav;g@@") + alias + ";p;f;q@@s@@x;y}";
            const Spec::SpecRule rule = Spec::GetRuleFromDescription (description);
            DBtest (rule.parseValid && rule.groups.GetSize () == 1, "Spec parser unique alias valid");
            if (rule.groups.GetSize () == 1)
                DBtest (rule.groups[0].unic_paramrawname.GetSize () == 1 &&
                            rule.groups[0].unic_paramrawname[0] == "{@gdl:p}",
                        "Spec parser unique copies output");
        }
        {
            GS::UniString description =
                "Spec_rule{\"  Избранное  \";g@@ID;GDL:Pa;Property:Flag;IFC:Qty@@s@@GDL:X;Property:Total}";
            const Spec::SpecRule rule = Spec::GetRuleFromDescription (description);
            DBtest (rule.parseValid, "Spec parser typed names valid");
            DBtest (rule.favorite_name, GS::UniString ("Избранное"), "Spec parser quoted unicode favorite");
            if (rule.groups.GetSize () == 1) {
                const Spec::GroupSpec &group = rule.groups[0];
                DBtest (group.unic_paramrawname.GetSize () == 1 && group.unic_paramrawname[0] == "{@id:id}",
                        "Spec parser ID prefix");
                DBtest (group.out_paramrawname.GetSize () == 1 && group.out_paramrawname[0] == "{@gdl:pa}",
                        "Spec parser GDL prefix");
                DBtest (group.flag_paramrawname, GS::UniString ("{@property:flag}"), "Spec parser property prefix");
                DBtest (group.sum_paramrawname.GetSize () == 1 && group.sum_paramrawname[0] == "{@ifc:qty}",
                        "Spec parser IFC prefix");
            }
            DBtest (rule.out_paramrawname.GetSize () == 1 && rule.out_paramrawname[0] == "{@gdl:x}",
                    "Spec parser output prefix");
            DBtest (rule.out_sum_paramrawname.GetSize () == 1 && rule.out_sum_paramrawname[0] == "{@property:total}",
                    "Spec parser output sum prefix");
        }
        {
            GS::UniString description = "Spec_rule{Fav;g@@u[3];p[2];f[4];q[5]@@s@@x;y}";
            const Spec::SpecRule rule = Spec::GetRuleFromDescription (description);
            for (UInt32 i = 0; i < rule.groups.GetSize (); ++i) {
                const Spec::GroupSpec &group = rule.groups[i];
                const GS::UniString suffix = GS::UniString::Printf ("@arr_%u_%u_1_1_1}", i + 1, i + 1);
                DBtest (group.unic_paramrawname.GetSize () == 1 && group.unic_paramrawname[0] == "{@gdl:u" + suffix,
                        "Spec parser array unique substitution");
                DBtest (group.out_paramrawname.GetSize () == 1 && group.out_paramrawname[0] == "{@gdl:p" + suffix,
                        "Spec parser array output substitution");
                DBtest (group.flag_paramrawname, "{@gdl:f" + suffix, "Spec parser array flag substitution");
                DBtest (group.sum_paramrawname.GetSize () == 1 && group.sum_paramrawname[0] == "{@gdl:q" + suffix,
                        "Spec parser array sum substitution");
            }
        }
        {
            GS::UniString description = "Spec_rule{Fav;g@@u;p;f1,f2@@s@@x;y1,y2}";
            const Spec::SpecRule rule = Spec::GetRuleFromDescription (description);
            if (rule.groups.GetSize () == 1) {
                DBtest (rule.groups[0].flag_paramrawname, GS::UniString ("{@gdl:f2}"), "Spec parser last flag");
                DBtest (rule.groups[0].sum_paramrawname.GetSize () == 2 && rule.groups[0].sum_paramrawname[0] == "1" &&
                            rule.groups[0].sum_paramrawname[1] == "1",
                        "Spec parser literal counts");
            }
        }
        DBprnt ("SpecRegression parser", "end");
    }

    void TestSpecAddRule () {
        DBprnt ("SpecRegression add rule", "start");
        SpecFixture f;
        Spec::SpecRuleDict rules;
        API_PropertyDefinition definition = {};
        // Sync_name обходит кэш групп свойств: тест не зависит от открытой модели.
        definition.groupGuid = f.extra;
        definition.guid = f.old;
        definition.name = "Sync_name_SpecFixture";
        definition.description = "Spec_rule_v2{Fav;g(u;p;f;q)s(x;y)}";
        const GS::UniString original = definition.description;
        const GS::UniString key = "Fav;g@@u;p;f;q@@s@@x;y)";
        Spec::AddRule (definition, APINULLGuid, rules);
        DBtest (rules.GetSize (), 1, "Spec AddRule adds default");
        DBtest (definition.description, original, "Spec AddRule input unchanged");
        const Spec::SpecRule *rule = rules.GetPtr (key);
        DBtest (rule != nullptr, "Spec AddRule normalized key");
        if (rule == nullptr)
            return;
        DBtest (rule->parseValid && rule->delete_old, "Spec AddRule v2 valid");
        DBtest (rule->elements.IsEmpty (), "Spec AddRule null GUID not appended");
        DBtest (rule->rule_name, definition.name, "Spec AddRule name");
        DBtest (rule->subguid_paramrawname, definition.name, "Spec AddRule link");
        DBtest (rule->subguid_rulevalue, definition.name, "Spec AddRule rule value");
        DBtest (rule->subguid_rulename.IsEmpty (), "Spec AddRule subguid rule name default");
        DBtest (rule->rule_definitions.guid == definition.guid, "Spec AddRule definition GUID");
        Spec::AddRule (definition, f.first, rules);
        Spec::AddRule (definition, f.second, rules);
        Spec::AddRule (definition, f.first, rules);
        Spec::AddRule (definition, APINULLGuid, rules);
        rule = rules.GetPtr (key);
        DBtest (rules.GetSize (), 1, "Spec AddRule reuses key");
        DBtest (rule != nullptr && rule->elements.GetSize () == 3, "Spec AddRule preserves repeated GUID");
        if (rule != nullptr && rule->elements.GetSize () == 3)
            DBtest (rule->elements[0] == f.first && rule->elements[1] == f.second && rule->elements[2] == f.first,
                    "Spec AddRule source order");
        definition.name = "Sync_name_Other";
        definition.guid = f.extra;
        definition.description = "Spec_rule_v3{Fav;g(u;p;f;q)s(x;y)}";
        Spec::AddRule (definition, f.extra, rules);
        rule = rules.GetPtr (key);
        DBtest (rules.GetSize (), 1, "Spec AddRule variant shares key");
        DBtest (rule != nullptr && rule->stop_on_error && rule->only_visible, "Spec AddRule first variant wins");
        DBtest (rule != nullptr && rule->rule_definitions.guid == f.old && rule->rule_name == "Sync_name_SpecFixture",
                "Spec AddRule first metadata wins");
        definition.description = "Spec_rule { Fav ;\n\t g (u;p;f;q) s (x;y) }";
        Spec::AddRule (definition, APINULLGuid, rules);
        DBtest (rules.GetSize (), 1, "Spec AddRule whitespace normalized");
        definition.description = "Spec_rule{Bad;g(u;p;f;q)s(x)}";
        Spec::AddRule (definition, f.first, rules);
        Spec::AddRule (definition, f.second, rules);
        DBtest (rules.GetSize (), 2, "Spec AddRule caches invalid once");
        const Spec::SpecRule *invalid = rules.GetPtr ("Bad;g@@u;p;f;q@@s@@x)");
        DBtest (invalid != nullptr && !invalid->parseValid && invalid->elements.IsEmpty (),
                "Spec AddRule invalid has no sources");
        const char *groups[] = {"gm", "gl"};
        for (Int32 i = 0; i < 2; ++i) {
            Spec::SpecRuleDict expanded;
            definition.description = GS::UniString ("Spec_rule{Fav;") + groups[i] + "(u;p;f;q)s(x;y)}";
            Spec::AddRule (definition, APINULLGuid, expanded);
            const GS::UniString expandedKey =
                GS::UniString ("Fav;g@@") + (i == 0 ? "Material_all@" : "libdata@") + "u;p;f;q@@s@@x;y)";
            const Spec::SpecRule *parsed = expanded.GetPtr (expandedKey);
            DBtest (parsed != nullptr && parsed->parseValid, "Spec AddRule expanded valid");
            if (parsed != nullptr)
                DBtest (parsed->groups.GetSize (), i == 0 ? 50 : 100, "Spec AddRule expanded groups");
        }
        DBprnt ("SpecRegression add rule", "end");
    }

    // R4.4: причина отказа в разборе. Парсер пишет её в тех же точках, где
    // сбрасывает parseValid, поэтому набор значений и число точек обязаны
    // совпадать: 7 значений отказа + None.
    // Ожидания рассчитаны воспроизведением ПОРЯДКА проверок парсера, а не
    // подгонкой под вывод. Ключевой момент - каскад в конце GetRuleFromDescription:
    // три финальные проверки не прерываются, поэтому при нескольких нарушениях
    // видна ПОСЛЕДНЯЯ. Отсюда два неочевидных исхода:
    //   - пустые обе схемы дают EmptySumSchema, а не EmptyOutputSchema;
    //   - пустая выходная схема роняет и группы (0 != 1), поэтому признак
    //     принятия групп здесь всегда false - и это не ошибка набора.
    void TestSpecParseError () {
        DBprnt ("SpecRegression parse reason", "start");

        struct ErrorCase {
            const char *body; // часть описания после "Spec_rule{Fav;"
            bool valid;
            Spec::ParseError error;
            const char *label;
        };

        const ErrorCase cases[] = {
            {"g@@u;p;f;q@@s@@x;y)", true, Spec::ParseError::None, "valid rule reason None"},
            // Маркер есть, но тело группы без точки с запятой.
            {"g@@u@@s@@x;y)", false, Spec::ParseError::GroupNotSplit, "group not split"},
            // Выходная схема из одной части (фильтр пустых не помогает: часть не пуста).
            {"g@@u;p;f;q@@s@@x)", false, Spec::ParseError::OutputPartCount, "one output part"},
            // Выходная схема из трёх частей.
            {"g@@u;p;f;q@@s@@x;y;z)", false, Spec::ParseError::OutputPartCount, "three output parts"},
            // Нет части s@@ вовсе.
            {"g@@u;p;f;q)", false, Spec::ParseError::NoSummary, "no summary part"},
            // s@@ есть, но вторая часть пуста и отфильтровывается - остаётся одна.
            {"g@@u;p;f;q@@s@@)", false, Spec::ParseError::NoSummary, "summary filtered to one part"},
            // Схема корректна, группа не совпала с ней по числу параметров:
            // группа отброшена, схема непустая, значит видна NoGroupsAccepted.
            {"g@@u;p1,p2;f;q@@s@@x;y)", false, Spec::ParseError::NoGroupsAccepted, "group dropped by size"},
            // Выходная схема пуста (имя "%" срезается), суммы непусты. Группы
            // тоже отброшены (0 != 1), но последней срабатывает EmptyOutputSchema.
            {"g@@u;p;f;q@@s@@%;y)", false, Spec::ParseError::EmptyOutputSchema, "empty output schema"},
            // Суммы пусты, выход непуст, группа без поля количеств принята.
            {"g@@u;p;f@@s@@x;%)", false, Spec::ParseError::EmptySumSchema, "empty sum schema"},
        };

        for (const ErrorCase &test : cases) {
            GS::UniString description = GS::UniString ("Spec_rule{Fav;") + test.body + "}";
            const Spec::SpecRule rule = Spec::GetRuleFromDescription (description);
            const GS::UniString label = GS::UniString ("Spec parse reason ") + test.label;
            DBtest (rule.parseValid, test.valid, label + " parse flag");
            // Причина обязана называть именно ту точку, которая отвергла описание.
            DBtest (rule.parseError == test.error, label + " reason");
            // parseValid и parseError не могут расходиться: успех = None.
            DBtest (rule.parseValid == (rule.parseError == Spec::ParseError::None), label + " reason matches flag");
        }

        // НЕТ маркера g@@ - и разбор НЕ отказывает: StringSplt основан на
        // UniString::Split, который возвращает минимум одну часть, поэтому
        // условие "нет ни одной группы" не наступает НИКОГДА. Весь хвост после
        // имени избранного становится одной обычной группой, и если её размеры
        // совпали со схемой, правило принимается.
        // Это зафиксировано как контракт: значимость ParseError::NoGroupMarker
        // недостижима через GetRuleFromDescription, значение оставлено как
        // защита на случай, если разбивка станет строже.
        {
            GS::UniString description = "Spec_rule{Fav;u;p;f;q@@s@@x;y}";
            const Spec::SpecRule rule = Spec::GetRuleFromDescription (description);
            DBtest (rule.parseValid, true, "Spec parse reason absent marker still parsed");
            DBtest (rule.parseError == Spec::ParseError::None, "Spec parse reason absent marker None");
            DBtest (rule.groups.GetSize () == 1, "Spec parse reason absent marker one group");
            // Реально видна NoGroupsAccepted - единственный отказ по группам.
            GS::UniString mismatched = "Spec_rule{Fav;u;p1,p2;f;q@@s@@x;y}";
            const Spec::SpecRule dropped = Spec::GetRuleFromDescription (mismatched);
            DBtest (!dropped.parseValid && dropped.parseError == Spec::ParseError::NoGroupsAccepted,
                    "Spec parse reason NoGroupsAccepted reachable");
        }

        // Текст причины не пустой и разный для каждого значения: иначе сообщение
        // в AddRule станет бесполезным ("нет причины" на всё).
        {
            const Spec::ParseError all[] = {Spec::ParseError::None,
                                            Spec::ParseError::NoGroupMarker,
                                            Spec::ParseError::GroupNotSplit,
                                            Spec::ParseError::OutputPartCount,
                                            Spec::ParseError::NoSummary,
                                            Spec::ParseError::NoGroupsAccepted,
                                            Spec::ParseError::EmptyOutputSchema,
                                            Spec::ParseError::EmptySumSchema};
            const UInt32 n = sizeof (all) / sizeof (all[0]);
            for (UInt32 i = 0; i < n; ++i) {
                const GS::UniString text = Spec::ParseErrorText (all[i]);
                DBtest (!text.IsEmpty (), "Spec parse reason text non-empty");
                for (UInt32 j = i + 1; j < n; ++j) {
                    DBtest (text != Spec::ParseErrorText (all[j]),
                            GS::UniString::Printf ("Spec parse reason text %u differs from %u", i, j));
                }
            }
        }

        // Причина не теряется на пути AddRule -> словарь: невалидное правило
        // сохраняет её, валидное остаётся с None.
        {
            SpecFixture f;
            Spec::SpecRuleDict rules;
            API_PropertyDefinition definition = {};
            definition.groupGuid = f.extra;
            definition.guid = f.old;
            definition.name = "Sync_name_SpecFixture";
            definition.description = "Spec_rule{Bad;g(u;p;f;q)s(x)}";
            Spec::AddRule (definition, APINULLGuid, rules);
            const Spec::SpecRule *stored = rules.GetPtr ("Bad;g@@u;p;f;q@@s@@x)");
            DBtest (stored != nullptr, "Spec parse reason AddRule keeps invalid rule");
            if (stored != nullptr) {
                DBtest (!stored->parseValid, "Spec parse reason AddRule invalid flag true");
                DBtest (stored->parseError == Spec::ParseError::OutputPartCount,
                        "Spec parse reason AddRule keeps reason");
            }
            definition.description = "Spec_rule{Ok;g(u;p;f;q)s(x;y)}";
            Spec::AddRule (definition, APINULLGuid, rules);
            const Spec::SpecRule *good = rules.GetPtr ("Ok;g@@u;p;f;q@@s@@x;y)");
            DBtest (good != nullptr && good->parseValid && good->parseError == Spec::ParseError::None,
                    "Spec parse reason AddRule valid keeps None");
        }

        DBprnt ("SpecRegression parse reason", "end");
    }

    void TestSpecSizes () {
        DBprnt ("SpecRegression sizes", "start");
        API_Element element = {};
        API_ElementMemo memo = {};
        double dx = 17;
        double dy = 19;
        DBtest (!Spec::GetSizePlaceElement (element, memo, dx, dy), "Spec size null memo horizontal");
        DBtest (dx == 17 && dy == 19, "Spec size null memo unchanged");
        // Только плоские числовые параметры, вложенных handles нет.
        memo.params =
            reinterpret_cast<API_AddParType **> (BMAllocateHandle (5 * sizeof (API_AddParType), ALLOCATE_CLEAR, 0));
        DBtest (memo.params != nullptr, "Spec size fixture allocation");
        if (memo.params == nullptr)
            return;
        auto set = [&] (Int32 index, const char *name, double value) {
            auto &target = (*memo.params)[index].name;
            USize i = 0;
            for (; name[i] != '\0' && i + 1 < sizeof (target); ++i)
                target[i] = name[i];
            target[i] = '\0';
            (*memo.params)[index].value.real = value;
        };
        set (0, "A", 2);
        set (1, "B", 3);
        DBtest (!Spec::GetSizePlaceElement (element, memo, dx, dy), "Spec size AB horizontal");
        DBtest (dx == 2 && dy == 3, "Spec size AB dimensions");
        set (2, "somestuff_spec_hrow", 0.5);
        DBtest (Spec::GetSizePlaceElement (element, memo, dx, dy), "Spec size height vertical");
        DBtest (dx == 0 && dy == 0.5, "Spec size height dimensions");
        set (3, "somestuff_spec_bcol", 1.5);
        for (Int32 type = 0; type < 5; ++type) {
            set (4, "show_type", type);
            const bool vertical = Spec::GetSizePlaceElement (element, memo, dx, dy);
            const GS::UniString label = GS::UniString::Printf ("Spec size show type %d", type);
            DBtest (vertical, type != 2 && type != 3, label + " direction");
            DBtest (dx, type == 2 || type == 3 ? 1.5 : 0, label + " dx");
            DBtest (dy, 0.5, label + " dy");
        }
        set (2, "other-height", 7);
        set (3, "other-width", 8);
        set (4, "show_type", 1);
        DBtest (Spec::GetSizePlaceElement (element, memo, dx, dy), "Spec size type one missing dimensions");
        DBtest (dx == 0 && dy == 0, "Spec size missing dimensions default zero");
        set (4, "other-type", 0);
        set (1, "b", 3);
        dx = 17;
        dy = 19;
        DBtest (!Spec::GetSizePlaceElement (element, memo, dx, dy), "Spec size partial AB");
        DBtest (dx == 2 && dy == 19, "Spec size case sensitive B unchanged");
        BMKillHandle (reinterpret_cast<GSHandle *> (&memo.params));
        DBprnt ("SpecRegression sizes", "end");
    }

    void TestSpecRegression () {
        DBprnt ("SpecRegression", "start");
        TestSpecGetParamValue ();
        TestSpecValueReader ();
        TestSpecValueEdges ();
        TestSpecReadPlan ();
        TestSpecRuleDependencies ();
        TestSpecFavoriteResolution ();
        TestSpecGrouping ();
        TestSpecReconcile ();
        TestSpecMergeAndKey ();
        TestSpecRuleDedup ();
        TestSpecOutSlots ();
        TestSpecSlotBindings ();
        TestSpecExpandGroup ();
        TestSpecGroups ();
        TestSpecOutputSchema ();
        TestSpecPolicy ();
        TestSpecNormalize ();
        TestSpecParser ();
        TestSpecAddRule ();
        TestSpecParseError ();
        TestSpecSizes ();
        DBprnt ("SpecRegression", "end");
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

        // #202: числовые аргументы правила File: принимаются только как полное число.
        // Раньше std::stoi молча отбрасывал хвост, поэтому "2junk" читался как 2,
        // описание свойства выглядело рабочим, а данные брались не из того столбца.
        // Имена файла и ячеек берём в кавычки: GetSubstring ищет первую пару скобок,
        // поэтому вложенные {...} обрезают правило.
        const GS::UniString ruleFileOk = "Sync_from{File:lookup;\"data.txt\",2,\"col_end\"}";
        param = ParamValue ();
        syncdirection = SYNC_NO;
        DBtest (
            SyncString (elementType, ruleFileOk, syncdirection, param, ignorevals, stringformat, true, false, false),
            "SyncString File full number -> true");
        DBtest (param.fromFile, "SyncString File full number -> fromFile");
        DBtest (param.composite_pen == 2, "SyncString File full number -> composite_pen 2");

        param = ParamValue ();
        syncdirection = SYNC_NO;
        DBtest (!SyncString (elementType,
                             "Sync_from{File:lookup;\"data.txt\",2junk,\"col_end\"}",
                             syncdirection,
                             param,
                             ignorevals,
                             stringformat,
                             true,
                             false,
                             false),
                "SyncString File junk after number -> false");

        // Все пять числовых позиций: col_out, конец столбцов, начало столбцов,
        // конец строк, начало строк
        const GS::UniString ruleFileAll = "Sync_from{File:lookup;\"data.txt\",2,\"ce\",3,\"cs\",4,\"re\",5,\"rs\",6}";
        param = ParamValue ();
        syncdirection = SYNC_NO;
        DBtest (
            SyncString (elementType, ruleFileAll, syncdirection, param, ignorevals, stringformat, true, false, false),
            "SyncString File all five numbers -> true");
        DBtest (param.composite_pen == 2, "SyncString File all five numbers -> composite_pen 2");
        DBtest (param.val.array_column_end == 3, "SyncString File all five numbers -> array_column_end 3");
        DBtest (param.val.array_column_start == 4, "SyncString File all five numbers -> array_column_start 4");
        DBtest (param.val.array_row_end == 5, "SyncString File all five numbers -> array_row_end 5");
        DBtest (param.val.array_row_start == 6, "SyncString File all five numbers -> array_row_start 6");

        // Мусор после числа в каждой из пяти позиций - правило должно отбраковываться
        static const char *junkRules[] = {
            "Sync_from{File:lookup;\"data.txt\",2junk,\"ce\",3,\"cs\",4,\"re\",5,\"rs\",6}",
            "Sync_from{File:lookup;\"data.txt\",2,\"ce\",3junk,\"cs\",4,\"re\",5,\"rs\",6}",
            "Sync_from{File:lookup;\"data.txt\",2,\"ce\",3,\"cs\",4junk,\"re\",5,\"rs\",6}",
            "Sync_from{File:lookup;\"data.txt\",2,\"ce\",3,\"cs\",4,\"re\",5junk,\"rs\",6}",
            "Sync_from{File:lookup;\"data.txt\",2,\"ce\",3,\"cs\",4,\"re\",5,\"rs\",6junk}"};
        for (int junk = 0; junk < 5; junk++) {
            param = ParamValue ();
            syncdirection = SYNC_NO;
            DBtest (
                !SyncString (
                    elementType, junkRules[junk], syncdirection, param, ignorevals, stringformat, true, false, false),
                GS::UniString::Printf ("SyncString File junk in number #%d -> false", junk + 1));
        }

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

        // Тесты 3-6: RENUM/RENUMFLAG/Sum/Spec не разбираются SyncString - команда
        // не начинается с SYNCPART, поэтому проверять надо не наличие подстроки в
        // литерале (это тавтология: сверяется константа сама с собой), а исход
        // ParsePropertyDescriptionToRules. hasSyncRules обязан быть false, а
        // hasOtherCommands - true: команда опознана, но правил синхронизации нет.
        {
            GS::UniString desc = "Renum_flag{Property:RenumRule; NULL}";
            const ParsePropertyResult result = ParsePropertyDescriptionToRules (desc);
            DBtest (!result.hasSyncRules, "ParseDesc Renum_flag -> no sync rules");
            DBtest (result.hasOtherCommands, "ParseDesc Renum_flag -> has other commands");
        }
        {
            GS::UniString desc = "Renum{Property:Criteria; Property:Delimetr}";
            const ParsePropertyResult result = ParsePropertyDescriptionToRules (desc);
            DBtest (!result.hasSyncRules, "ParseDesc Renum -> no sync rules");
            DBtest (result.hasOtherCommands, "ParseDesc Renum -> has other commands");
            if (!result.otherCommands.IsEmpty ())
                DBtest (result.otherCommands[0].commandType == "Renum", "ParseDesc Renum -> commandType Renum");
        }
        {
            GS::UniString desc = "Sum{Property:SumProp1; Property:SumProp2; max}";
            const ParsePropertyResult result = ParsePropertyDescriptionToRules (desc);
            DBtest (!result.hasSyncRules, "ParseDesc Sum -> no sync rules");
            DBtest (result.hasOtherCommands, "ParseDesc Sum -> has other commands");
            if (!result.otherCommands.IsEmpty ())
                DBtest (result.otherCommands[0].commandType == "Sum", "ParseDesc Sum -> commandType Sum");
        }
        {
            GS::UniString desc = "Spec_rule{g(U, P, F, Q)}{s(Pn, Qn)}";
            const ParsePropertyResult result = ParsePropertyDescriptionToRules (desc);
            DBtest (!result.hasSyncRules, "ParseDesc Spec_rule -> no sync rules");
            DBtest (result.hasOtherCommands, "ParseDesc Spec_rule -> has other commands");
            if (!result.otherCommands.IsEmpty ())
                DBtest (result.otherCommands[0].commandType == "Spec_rule",
                        "ParseDesc Spec_rule -> commandType Spec_rule");
        }

        // Тест 7: Комбинированное описание (Sync + Renum_flag). Разбор отдаёт
        // И правило синхронизации, И прочую команду: RENUMFLAG не начинается с
        // SYNCPART, но остаётся в otherCommands. Проверяется разбор, а не
        // наличие подстроки в литерале описания.
        {
            GS::UniString desc = "Sync_from{Property:Source}Renum_flag{Property:RenumRule}";
            GS::Array<GS::UniString> parts;
            GS::Array<GS::UniString> scratch;
            UInt32 n = StringSpltFilter (desc, SYNCPART, parts, BRACESTART, &scratch);
            DBtest (n >= 1, "ParseDesc Combined -> at least 1 sync part");
            const ParsePropertyResult result = ParsePropertyDescriptionToRules (desc);
            DBtest (result.hasSyncRules, "ParseDesc Combined -> has sync rules");
            DBtest (result.hasOtherCommands, "ParseDesc Combined -> has other commands");
            if (!result.otherCommands.IsEmpty ())
                DBtest (result.otherCommands[0].commandType == "Renum_flag",
                        "ParseDesc Combined -> commandType Renum_flag");
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
                    GS::UniString (" wallCode=") + GS::ValueToUniString (wallListError) + GS::UniString (" slabCode=") +
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
                        GS::UniString ("RuleFlagProj definitions: code=") + GS::ValueToUniString ((Int32)err) +
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
