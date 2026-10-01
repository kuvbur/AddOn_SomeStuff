//------------ kuvbur 2022 ------------
#ifdef TESTING
    #include "ACAPinc.h"

    #include "api_headers/APIEnvir.h"

    #include "Helpers.hpp"
    #include "Propertycache.hpp"
    #include "spec/Spec.hpp"
    #include "spec/SpecPlanning.hpp"
    #include "Sync.hpp"
    #include "tests/TestFunc.hpp"
    #include "tests/TestKit.hpp"

namespace TestFunc {

    namespace {
        // Синтетические GUID используются только как ключи словарей, не как элементы модели.
        // Числовой ParamValue для проверок суммирования (R6.3). valid=false
        // даёт невалидный слот — на нём держится правило «isValid с обеих
        // сторон». Локальной функцией быть не может: определение в теле
        // функции запрещено, поэтому хелпер живёт здесь.
        ParamValue Num (Int32 v, bool valid = true) {
            ParamValue p = {};
            ParamHelpers::ConvertIntToParamValue (p, EMPTYSTRING, v);
            p.isValid = valid;
            return p;
        }

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

    void TestSpecGetParamValue () {
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
    }

    void TestSpecGrouping () {
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
    }

    void TestSpecReconcile () {
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
    }

    // R4.5: дедупликация AddRule по ключу = текст ВНУТРИ фигурных скобок.
    // Префикс правила ("Spec_rule", "_v2", "_v3", "_km") в ключ НЕ входит,
    // поэтому описания с одинаковым телом и разной политикой схлопываются, и
    // выигрывает ТО, ЧТО ПРИШЛО ПЕРВЫМ. Закрепляется как контракт: описания
    // в существующих моделях завязаны на такое поведение.
    void TestSpecRuleDedup () {
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
    }

    void TestSpecReadPlan () {
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
    }

    // R5.1: сбор зависимостей правила - что нужно прочитать у источников и что
    // потом записать. Сбор отделён от разворачивания имён в словарь элемента,
    // поэтому проверяется сам перечень (без обращения к модели), а сличение в
    // общие словари запуска остаётся отдельным контрактом адаптера.
    void TestSpecRuleDependencies () {
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
    }

    // R5.2: разрешение избранного, служебных полей и старых объектов. Четыре
    // вынесенные операции проверяются раздельно; MatchDestinationProperties,
    // SelectExistingElements и AddExistingReadRequests чисты, а
    // ResolveFavoriteLinks упирается в ambient-кэш свойств, поэтому для него
    // закреплён именно ПРОМАХ чтения: неудачное чтение не кэшируется и не
    // подменяет поля правила (R5.2 запрещает новую политику кэша без F).
    void TestSpecFavoriteResolution () {
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
    }

    // R5.5: граница выделения чтения. Оценочный набор — он не добавляет нового
    // поведения, а проверяет, что R5.1-R5.4 не изменили наблюдаемое поведение.
    //
    // Покрытие сценариев плана по состоянию на этот шаг:
    //   S10 материалы (один слой, многослойная, конец слоя) - TestSpecGetParamValue
    //       и TestSpecValueEdges;
    //   S11 отрицательный индекс / за концом / пустое-числовое-текстовое - там же;
    //   S12 listdata (нет данных, позиция, конец списка, невычисленная формула) -
    //       TestSpecGetParamValue;
    //   S13 GDL-массивы на ЧТЕНИИ - не покрыт: разбор @arr живёт в Helpers
    //       (ConvertStringToParamValue, Helpers.cpp:6220) и читает через ACAPI.
    //       R5 запрещает трогать Helpers, поэтому здесь закрепляется только
    //       спец-семантика СОБСТВЕННО чтения (первый ряд - ошибка строки);
    //   S14 формулы - частично (значение результата покрыто, отсутствие утечки
    //       значений между элементами - нет);
    //   S15 флаг и отсутствия полей - TestSpecGrouping;
    //   S16 два правила на одном избранном - см. блок S16 ниже.
    //
    // Главный критерий шага: ИСХОДНЫЕ СЛОВАРИ ДО/ПОСЛЕ ЧТЕНИЯ совпадают.
    // Чтение не обязано быть безопасно идемпотентным по значению (формулы
    // пересчитываются), но не обязано менять СЛОВАРИ - это проверяется здесь.
    // R6.1: расчётная часть вынесена в PlanRuleRows. Набор проверяет НОВУЮ
    // границу: расчёт считается БЕЗ сверки существующих строк и без модели —
    // это и есть выход блока R6 («вычисленные строки можно проверить без
    // создания объектов»).
    //
    // Сверка сравнивается с расчётом: delete_old = true у PlanRuleRows не
    // меняет ничего (сверка живёт в GetElementsForRule), а delete_old = false
    // у GetElementsForRule означает «не идти в сверку вообще».
    // R6.2: вклад источника (RuleContribution) проверяется БЕЗ словаря
    // элементов и без запуска GetElementsForRule — ровно то разделение, которое
    // шаг и вводил. Вклад описывает, что вносит ОДИН источник; раскладка в
    // агрегат — это R6.3 и здесь не проверяется.
    void TestSpecContribution () {
        // --- полный вклад: все слоты прочитаны ---
        {
            SpecFixture f;
            f.Source (f.first, "A", "Alpha", 7);
            const Spec::GroupSlotBinding binding = Spec::PrepareSlotBindings (f.rule)[0];
            FormatString fstr = FormatStringFunc::ParseFormatString (".2m");
            ParamDict none1 = {};
            ParamDict none2 = {};
            const Spec::SpecValueReader reader (f.context);
            const Spec::RuleContribution c =
                Spec::BuildContribution (f.first, 0, f.rule.groups[0], binding, reader, none1, none2);
            DBtest (c.isIncluded, true, "R6.2 full contribution included");
            DBtest (c.status, Spec::ContributionStatus::Partial, "R6.2 phase 1 stops before outputs");
            DBtest (c.source == f.first, true, "R6.2 contribution carries source");
            DBtest (c.groupIndex, 0u, "R6.2 contribution carries group index");
            DBtest (c.hasKey, true, "R6.2 key assembled");
            DBtest (c.key, GS::UniString ("@A"), "R6.2 key value");
            DBtest (c.outSumParam.GetSize (), 1, "R6.2 sum slot read in phase 1");
            DBtest (c.outSumParam[0].val.intValue, 7, "R6.2 sum slot value");
            DBtest (c.outParam.GetSize (), 0, "R6.2 outputs not read in phase 1");
            DBtest (c.outputsRead, false, "R6.2 phase 2 flag unset");
            DBtest (c.missingUnic.IsEmpty (), true, "R6.2 nothing missing");
            DBtest (c.missingSum.IsEmpty (), true, "R6.2 nothing missing in sum");
            DBtest (c.missingOut.IsEmpty (), true, "R6.2 nothing missing in out");
            // Фаза 2: выходные слоты появляются только здесь.
            Spec::RuleContribution full = c;
            Spec::ReadContributionOutputs (f.first, f.rule.groups[0], binding, reader, fstr, full);
            DBtest (full.outputsRead, true, "R6.2 phase 2 flag set");
            DBtest (full.outParam.GetSize (), 1, "R6.2 out slot read in phase 2");
            DBtest (full.outParam[0].val.uniStringValue, GS::UniString ("Alpha"), "R6.2 out slot value");
            DBtest (full.keyOut, GS::UniString ("@Alpha"), "R6.2 out key value");
            DBtest (full.isComplete, true, "R6.2 complete when all slots read");
            DBtest (Spec::ClassifyContribution (full, 1, 1),
                    Spec::ContributionStatus::Complete,
                    "R6.2 classified complete");
        }

        // --- сумма-константа "1": слот есть, чтения не было ---
        {
            SpecFixture f;
            f.Source (f.first, "A", "Alpha", 5);
            f.rule.groups[0].sum_paramrawname[0] = "1";
            f.rule.out_sum_paramrawname[0] = "1";
            const Spec::GroupSlotBinding binding = Spec::PrepareSlotBindings (f.rule)[0];
            DBtest (binding.sumSlots[0].isSumLiteral, true, "R6.2 literal slot recognised");
            FormatString fstr = FormatStringFunc::ParseFormatString (".2m");
            ParamDict none1 = {};
            ParamDict none2 = {};
            const Spec::SpecValueReader reader (f.context);
            Spec::RuleContribution c =
                Spec::BuildContribution (f.first, 0, f.rule.groups[0], binding, reader, none1, none2);
            DBtest (c.outSumParam.GetSize (), 1, "R6.2 literal produces a slot");
            DBtest (c.outSumParam[0].val.intValue, 1, "R6.2 literal value is one");
            DBtest (c.missingSum.IsEmpty (), true, "R6.2 literal is not a missing field");
        }

        // --- выключенный флаг: вклад создан, но НЕ включён ---
        {
            SpecFixture f;
            f.Source (f.first, "A", "Alpha", 3);
            f.rule.groups[0].flag_paramrawname = f.flag;
            ParamValue off = {};
            // rawName задаётся ДО конвертера: Convert* заполняет его только
            // если он пуст, поэтому так остаётся ключ f.flag, под которым поле
            // и лежит в словаре. Иначе Read искал бы несуществующий {@gdl:...}.
            off.rawName = f.flag;
            ParamHelpers::ConvertBoolToParamValue (off, f.flag, false);
            f.context.read.Get (f.first).Put (f.flag, off);
            const Spec::GroupSlotBinding binding = Spec::PrepareSlotBindings (f.rule)[0];
            ParamDict none1 = {};
            ParamDict none2 = {};
            const Spec::SpecValueReader reader (f.context);
            const Spec::RuleContribution c =
                Spec::BuildContribution (f.first, 0, f.rule.groups[0], binding, reader, none1, none2);
            DBtest (c.isIncluded, false, "R6.2 disabled flag excludes contribution");
            DBtest (c.status, Spec::ContributionStatus::Excluded, "R6.2 disabled flag status");
            DBtest (c.outSumParam.GetSize (), 0, "R6.2 excluded reads no slots");
            DBtest (
                Spec::ClassifyContribution (c, 1, 1), Spec::ContributionStatus::Excluded, "R6.2 excluded classified");
        }

        // --- непрочитанный уникальный параметр: вклад исключён, поле помечено ---
        {
            SpecFixture f;
            f.Source (f.first, "A", "Alpha", 3);
            f.context.read.Get (f.first).Delete (f.key);
            const Spec::GroupSlotBinding binding = Spec::PrepareSlotBindings (f.rule)[0];
            ParamDict none1 = {};
            ParamDict none2 = {};
            const Spec::SpecValueReader reader (f.context);
            const Spec::RuleContribution c =
                Spec::BuildContribution (f.first, 0, f.rule.groups[0], binding, reader, none1, none2);
            DBtest (c.status, Spec::ContributionStatus::Excluded, "R6.2 missing unic excludes");
            DBtest (c.missingUnic.GetSize (), 1, "R6.2 missing unic recorded");
            DBtest (c.missingUnic[0].rawname, f.key, "R6.2 missing unic name");
            DBtest (c.missingUnic[0].isError, true, "R6.2 missing unic is an error");
            DBtest (c.hasReadError, true, "R6.2 read error flagged");
            // Ключ склеивается ДАЖЕ при отказе — пустое значение плюс ATSIGN.
            DBtest (c.key, GS::UniString ("@"), "R6.2 key still gets sign on failure");
            DBtest (c.hasKey, false, "R6.2 key marked unusable");
        }

        // --- fromMaterial снимает ошибку у непрочитанного поля ---
        {
            SpecFixture f;
            f.Source (f.first, "A", "Alpha", 3);
            f.rule.groups[0].fromMaterial = true;
            f.context.read.Get (f.first).Delete (f.key);
            const Spec::GroupSlotBinding binding = Spec::PrepareSlotBindings (f.rule)[0];
            ParamDict none1 = {};
            ParamDict none2 = {};
            const Spec::SpecValueReader reader (f.context);
            const Spec::RuleContribution c =
                Spec::BuildContribution (f.first, 0, f.rule.groups[0], binding, reader, none1, none2);
            DBtest (c.missingUnic.GetSize (), 1, "R6.2 fromMaterial still records field");
            DBtest (c.missingUnic[0].isError, false, "R6.2 fromMaterial clears error flag");
            DBtest (c.hasReadError, false, "R6.2 fromMaterial clears read error");
        }

        // --- неполный выход: вклад частичный, isError хранится ПО ПОЛЮ ---
        {
            SpecFixture f;
            f.Source (f.first, "A", "Alpha", 3);
            f.context.read.Get (f.first).Delete (f.text);
            const Spec::GroupSlotBinding binding = Spec::PrepareSlotBindings (f.rule)[0];
            FormatString fstr = FormatStringFunc::ParseFormatString (".2m");
            ParamDict none1 = {};
            ParamDict none2 = {};
            const Spec::SpecValueReader reader (f.context);
            Spec::RuleContribution c =
                Spec::BuildContribution (f.first, 0, f.rule.groups[0], binding, reader, none1, none2);
            Spec::ReadContributionOutputs (f.first, f.rule.groups[0], binding, reader, fstr, c);
            DBtest (c.outParam.GetSize (), 0, "R6.2 missing out yields no slot");
            DBtest (c.missingOut.GetSize (), 1, "R6.2 missing out recorded");
            DBtest (c.missingOut[0].rawname, f.text, "R6.2 missing out name");
            DBtest (c.missingOut[0].isError, true, "R6.2 missing out is an error");
            DBtest (c.isComplete, false, "R6.2 partial contribution incomplete");
            DBtest (Spec::ClassifyContribution (c, 1, 1), Spec::ContributionStatus::Partial, "R6.2 partial classified");
            DBtest (c.keyOut, GS::UniString (EMPTYSTRING), "R6.2 empty out key on failure");
        }

        // --- схема шире фактического числа слотов: тоже частичный ---
        {
            SpecFixture f;
            f.Source (f.first, "A", "Alpha", 3);
            Spec::RuleContribution c = {};
            c.isIncluded = true;
            c.outputsRead = true;
            c.outSumParam.Push (ParamValue ());
            c.outParam.Push (ParamValue ());
            DBtest (
                Spec::ClassifyContribution (c, 2, 2), Spec::ContributionStatus::Partial, "R6.2 short slots vs schema");
            DBtest (
                Spec::ClassifyContribution (c, 1, 1), Spec::ContributionStatus::Complete, "R6.2 exact slots complete");
        }

        // --- два вклада одного ключа строятся независимо друг от друга ---
        // Основа для R6.3: раскладка в агрегат не должна зависеть от того,
        // в каком порядке вклады дошли до словаря.
        {
            SpecFixture f;
            f.Source (f.first, "A", "Alpha", 2);
            f.Source (f.second, "A", "Alpha", 3);
            const Spec::GroupSlotBinding binding = Spec::PrepareSlotBindings (f.rule)[0];
            ParamDict none1 = {};
            ParamDict none2 = {};
            const Spec::SpecValueReader reader (f.context);
            const Spec::RuleContribution c1 =
                Spec::BuildContribution (f.first, 0, f.rule.groups[0], binding, reader, none1, none2);
            const Spec::RuleContribution c2 =
                Spec::BuildContribution (f.second, 0, f.rule.groups[0], binding, reader, none1, none2);
            DBtest (c1.key, c2.key, "R6.2 same key from both sources");
            DBtest (c1.source == f.first, true, "R6.2 first contribution source");
            DBtest (c2.source == f.second, true, "R6.2 second contribution source");
            DBtest (c1.outSumParam[0].val.intValue, 2, "R6.2 first contribution sum");
            DBtest (c2.outSumParam[0].val.intValue, 3, "R6.2 second contribution sum");
        }
    }

    // R6.3: раскладка вклада в строку. Главное здесь — НЕ «улучшить»
    // суммирование неполных массивов, а зафиксировать его как поведение.
    // nsumm = MIN(длин), слот складывается только при isValid с обеих сторон,
    // слоты сверх nsumm не трогаются. План прямо запрещает заменять это
    // строгим конструктором: найденное расхождение оформляется F.
    void TestSpecRowLayout () {
        // --- суммирование равных массивов ---
        {
            Spec::Element row = {};
            row.out_sum_param.Push (Num (2));
            Spec::RuleContribution c = {};
            c.outSumParam.Push (Num (3));
            Spec::SumContributionIntoRow (row, c);
            DBtest (row.out_sum_param.GetSize (), 1, "R6.3 equal sizes keep size");
            DBtest (row.out_sum_param[0].val.intValue, 5, "R6.3 equal sizes summed");
        }

        // --- НЕПОЛНЫЕ массивы: вклад короче строки ---
        // Поведение сохранено: складываются только первые 2 из 3, третий слот
        // строки остаётся как был.
        {
            Spec::Element row = {};
            row.out_sum_param.Push (Num (1));
            row.out_sum_param.Push (Num (10));
            row.out_sum_param.Push (Num (100));
            Spec::RuleContribution c = {};
            c.outSumParam.Push (Num (2));
            c.outSumParam.Push (Num (20));
            Spec::SumContributionIntoRow (row, c);
            DBtest (row.out_sum_param.GetSize (), 3, "R6.3 short contribution does not resize row");
            DBtest (row.out_sum_param[0].val.intValue, 3, "R6.3 short contribution sums first slot");
            DBtest (row.out_sum_param[1].val.intValue, 30, "R6.3 short contribution sums second slot");
            DBtest (row.out_sum_param[2].val.intValue, 100, "R6.3 slot beyond nsumm untouched");
        }

        // --- НЕПОЛНЫЕ массивы: строка короче вклада ---
        {
            Spec::Element row = {};
            row.out_sum_param.Push (Num (7));
            Spec::RuleContribution c = {};
            c.outSumParam.Push (Num (1));
            c.outSumParam.Push (Num (2));
            c.outSumParam.Push (Num (4));
            Spec::SumContributionIntoRow (row, c);
            DBtest (row.out_sum_param.GetSize (), 1, "R6.3 longer contribution does not grow row");
            DBtest (row.out_sum_param[0].val.intValue, 8, "R6.3 longer contribution sums overlap only");
        }

        // --- isValid требуется с ОБЕИХ сторон ---
        {
            Spec::Element row = {};
            row.out_sum_param.Push (Num (5));
            row.out_sum_param.Push (Num (50));
            Spec::RuleContribution c = {};
            c.outSumParam.Push (Num (5, false)); // невалидный: слот остаётся как есть
            c.outSumParam.Push (Num (1));
            Spec::SumContributionIntoRow (row, c);
            DBtest (row.out_sum_param[0].val.intValue, 5, "R6.3 invalid side leaves slot untouched");
            DBtest (row.out_sum_param[1].val.intValue, 51, "R6.3 valid pair still summed");
        }
        {
            Spec::Element row = {};
            row.out_sum_param.Push (Num (5, false)); // невалидная строка
            Spec::RuleContribution c = {};
            c.outSumParam.Push (Num (5));
            Spec::SumContributionIntoRow (row, c);
            DBtest (row.out_sum_param[0].val.intValue, 5, "R6.3 invalid row slot untouched");
        }

        // --- пустые массивы: ничего не происходит, размер не меняется ---
        {
            Spec::Element row = {};
            row.out_sum_param.Push (Num (9));
            Spec::RuleContribution c = {};
            Spec::SumContributionIntoRow (row, c);
            DBtest (row.out_sum_param.GetSize (), 1, "R6.3 empty contribution keeps row size");
            DBtest (row.out_sum_param[0].val.intValue, 9, "R6.3 empty contribution keeps value");

            Spec::Element emptyRow = {};
            Spec::RuleContribution withValue = {};
            withValue.outSumParam.Push (Num (3));
            Spec::SumContributionIntoRow (emptyRow, withValue);
            DBtest (emptyRow.out_sum_param.GetSize (), 0, "R6.3 empty row stays empty");
        }

        // --- Created: строка собирается из вклада и копирует признаки правила ---
        {
            SpecFixture f;
            f.Source (f.first, "A", "Alpha", 6);
            const Spec::GroupSlotBinding binding = Spec::PrepareSlotBindings (f.rule)[0];
            ParamDict none1 = {};
            ParamDict none2 = {};
            const Spec::SpecValueReader reader (f.context);
            Spec::RuleContribution c =
                Spec::BuildContribution (f.first, 0, f.rule.groups[0], binding, reader, none1, none2);
            FormatString fstr = FormatStringFunc::ParseFormatString (".2m");
            Spec::ReadContributionOutputs (f.first, f.rule.groups[0], binding, reader, fstr, c);

            Spec::ElementDict rows = {};
            GS::HashTable<GS::UniString, GS::UniString> outParam = {};
            const Spec::RowAddition r1 = Spec::AddContributionToRow (rows, c, f.rule, 1, 1, outParam);
            DBtest (r1, Spec::RowAddition::Created, "R6.3 first contribution creates row");
            DBtest (rows.GetSize (), 1, "R6.3 one row created");
            const Spec::Element *row = rows.GetPtr ("@A");
            DBrequire (row != nullptr, "R6.3 created row found");
            DBtest (row->out_param.GetSize (), 1, "R6.3 created row has out slot");
            DBtest (row->out_sum_param.GetSize (), 1, "R6.3 created row has sum slot");
            DBtest (row->out_sum_param[0].val.intValue, 6, "R6.3 created row sum value");
            DBtest (row->elements.GetSize (), 1, "R6.3 created row has one source");
            DBtest (row->elements[0] == f.first, true, "R6.3 created row source is the contributor");
            DBtest (row->favorite_name, f.rule.favorite_name, "R6.3 created row carries favorite");
            DBtest (
                row->out_paramrawname.GetSize (), f.rule.out_paramrawname.GetSize (), "R6.3 created row out schema");
            DBtest (outParam.GetSize (), 1, "R6.3 outParam recorded on creation");
        }

        // --- Merged: источник дописан, сумма сложена, признаки НЕ пересчитываются ---
        {
            SpecFixture f;
            f.Source (f.first, "A", "Alpha", 2);
            f.Source (f.second, "A", "Alpha", 3);
            const Spec::GroupSlotBinding binding = Spec::PrepareSlotBindings (f.rule)[0];
            ParamDict none1 = {};
            ParamDict none2 = {};
            const Spec::SpecValueReader reader (f.context);
            FormatString fstr = FormatStringFunc::ParseFormatString (".2m");

            Spec::RuleContribution c1 =
                Spec::BuildContribution (f.first, 0, f.rule.groups[0], binding, reader, none1, none2);
            Spec::ReadContributionOutputs (f.first, f.rule.groups[0], binding, reader, fstr, c1);
            Spec::RuleContribution c2 =
                Spec::BuildContribution (f.second, 0, f.rule.groups[0], binding, reader, none1, none2);
            // Второй вклад — фазой 1: выходные слоты не читаются, как в цикле.
            Spec::ReadContributionOutputs (f.second, f.rule.groups[0], binding, reader, fstr, c2);

            Spec::ElementDict rows = {};
            GS::HashTable<GS::UniString, GS::UniString> outParam = {};
            DBtest (Spec::AddContributionToRow (rows, c1, f.rule, 1, 1, outParam),
                    Spec::RowAddition::Created,
                    "R6.3 first creates");
            DBtest (Spec::AddContributionToRow (rows, c2, f.rule, 1, 1, outParam),
                    Spec::RowAddition::Merged,
                    "R6.3 second merges");
            DBtest (rows.GetSize (), 1, "R6.3 merge keeps one row");
            const Spec::Element *row = rows.GetPtr ("@A");
            DBrequire (row != nullptr, "R6.3 merged row found");
            DBtest (row->out_sum_param[0].val.intValue, 5, "R6.3 merge summed both");
            DBtest (row->elements.GetSize (), 2, "R6.3 merge collected both sources");
            DBtest (row->elements[1] == f.second, true, "R6.3 merge appended second source");
            DBtest (outParam.GetSize (), 1, "R6.3 merge does not duplicate outParam");
        }

        // --- SchemaMismatch: строка НЕ добавлена, но ключ в outParam УЖЕ записан ---
        // Порядок операций сохранён: запись в outParam идёт ДО проверки схемы.
        {
            SpecFixture f;
            f.Source (f.first, "A", "Alpha", 6);
            const Spec::GroupSlotBinding binding = Spec::PrepareSlotBindings (f.rule)[0];
            ParamDict none1 = {};
            ParamDict none2 = {};
            const Spec::SpecValueReader reader (f.context);
            FormatString fstr = FormatStringFunc::ParseFormatString (".2m");
            Spec::RuleContribution c =
                Spec::BuildContribution (f.first, 0, f.rule.groups[0], binding, reader, none1, none2);
            Spec::ReadContributionOutputs (f.first, f.rule.groups[0], binding, reader, fstr, c);
            // Схема объявлена шире фактически прочитанного: 2 вместо 1.
            Spec::ElementDict rows = {};
            GS::HashTable<GS::UniString, GS::UniString> outParam = {};
            const Spec::RowAddition r = Spec::AddContributionToRow (rows, c, f.rule, 2, 2, outParam);
            DBtest (r, Spec::RowAddition::SchemaMismatch, "R6.3 schema mismatch reported");
            DBtest (rows.GetSize (), 0, "R6.3 schema mismatch adds no row");
            DBtest (outParam.GetSize (), 1, "R6.3 outParam written before schema check");
        }

        // --- раскладка эквивалентна полному пути: та же строка ---
        // Главная проверка выноса: агрегат, собранный раскладкой, совпадает с
        // тем, что строит GetElementsForRule.
        {
            SpecFixture layout;
            layout.Source (layout.first, "A", "Alpha", 2);
            layout.Source (layout.second, "A", "Alpha", 3);
            layout.Source (layout.extra, "B", "Beta", 7);
            const Spec::GroupSlotBinding binding = Spec::PrepareSlotBindings (layout.rule)[0];
            ParamDict none1 = {};
            ParamDict none2 = {};
            const Spec::SpecValueReader reader (layout.context);
            FormatString fstr = FormatStringFunc::ParseFormatString (".2m");
            Spec::ElementDict rows = {};
            GS::HashTable<GS::UniString, GS::UniString> outParam = {};
            for (const API_Guid &guid : layout.rule.elements) {
                Spec::RuleContribution c =
                    Spec::BuildContribution (guid, 0, layout.rule.groups[0], binding, reader, none1, none2);
                if (c.status == Spec::ContributionStatus::Excluded)
                    continue;
                if (!rows.ContainsKey (c.key))
                    Spec::ReadContributionOutputs (guid, layout.rule.groups[0], binding, reader, fstr, c);
                Spec::AddContributionToRow (rows, c, layout.rule, 1, 1, outParam);
            }

            SpecFixture full;
            full.Source (full.first, "A", "Alpha", 2);
            full.Source (full.second, "A", "Alpha", 3);
            full.Source (full.extra, "B", "Beta", 7);
            full.Run ();

            DBtest (rows.GetSize (), full.created.GetSize (), "R6.3 layout and full path agree on rows");
            // Ключ выхода строится из значений выходных слотов, поэтому у строк
            // с РАЗНЫМИ значениями выхода он разный: "@Alpha" и "@Beta". Одна
            // запись на каждый ключ строки, не на строку.
            DBtest (outParam.GetSize (), 2, "R6.3 layout outParam has one entry per row");
            for (auto it = rows.Begin (); it != rows.End (); ++it) {
    #ifdef ServerMainVers_2800
                const GS::UniString &key = it->key;
    #else
                const GS::UniString key = *it->key;
    #endif
                const Spec::Element *a = rows.GetPtr (key);
                const Spec::Element *b = full.created.GetPtr (key);
                DBrequire (a != nullptr && b != nullptr, "R6.3 both sides have the row");
                DBtest (a->out_sum_param.GetSize (), b->out_sum_param.GetSize (), "R6.3 agree on sum slot count");
                DBtest (a->out_param.GetSize (), b->out_param.GetSize (), "R6.3 agree on out slot count");
                DBtest (a->elements.GetSize (), b->elements.GetSize (), "R6.3 agree on source count");
                DBtest (a->favorite_name, b->favorite_name, "R6.3 agree on favorite");
                for (UInt32 j = 0; j < a->out_sum_param.GetSize () && j < b->out_sum_param.GetSize (); j++)
                    DBtest (
                        a->out_sum_param[j].val.intValue, b->out_sum_param[j].val.intValue, "R6.3 agree on sum value");
            }
            const Spec::Element *mergedA = rows.GetPtr ("@A");
            DBrequire (mergedA != nullptr, "R6.3 merged row A present");
            DBtest (mergedA->out_sum_param[0].val.intValue, 5, "R6.3 layout summed 2+3");
            const Spec::Element *rowB = rows.GetPtr ("@B");
            DBrequire (rowB != nullptr, "R6.3 row B present");
            DBtest (rowB->out_sum_param[0].val.intValue, 7, "R6.3 row B own sum");
        }
    }

    void TestSpecPlanning () {
        // --- расчёт единственного источника ---
        {
            SpecFixture f;
            f.Source (f.first, "A", "Alpha", 2);
            Spec::ElementDict planned;
            UnicGuid errors;
            GS::HashTable<GS::UniString, GS::UniString> outParam;
            const Int32 n = Spec::PlanRuleRows (f.rule, f.context, planned, errors, false, outParam);
            DBtest (n, 1, "R6.1 one source one row");
            DBtest (planned.GetSize (), 1, "R6.1 one row planned");
            DBtest (errors.IsEmpty (), true, "R6.1 no errors");
            // Строка рассчитана: слоты заполнены по схеме, источник и признаки
            // правила перенесены.
            const Spec::Element *row = planned.GetPtr ("@A");
            DBrequire (row != nullptr, "R6.1 row found by key");
            DBtest (row->out_param.GetSize (), 1, "R6.1 out slot filled");
            DBtest (row->out_sum_param.GetSize (), 1, "R6.1 sum slot filled");
            DBtest (row->out_param[0].val.uniStringValue, GS::UniString ("Alpha"), "R6.1 out value");
            DBtest (row->out_sum_param[0].val.intValue, 2, "R6.1 sum value");
            DBtest (row->elements.GetSize (), 1, "R6.1 source multiplicity");
            DBtest (row->out_paramrawname.GetSize (), f.rule.out_paramrawname.GetSize (), "R6.1 out schema carried");
            DBtest (row->favorite_name, f.rule.favorite_name, "R6.1 favorite carried");
            // out_param: ключ - склеенные выходящие значения, значение - ключ
            // строки. Он пережил вынос и остаётся тем же словарём.
            DBtest (outParam.GetSize (), 1, "R6.1 outParam one entry");
            DBtest (outParam.ContainsKey (EMPTYSTRING) || outParam.GetSize () == 1, true, "R6.1 outParam keyed");
        }

        // --- расчёт НЕ трогает существующие строки: delete_old не влияет ---
        {
            SpecFixture f;
            f.Source (f.first, "A", "Alpha", 2);
            f.Existing (f.old, "Alpha", 2);
            Spec::ElementDict planned;
            UnicGuid errors;
            GS::HashTable<GS::UniString, GS::UniString> outParam;
            f.rule.delete_old = true;
            Spec::PlanRuleRows (f.rule, f.context, planned, errors, false, outParam);
            // Сверки не было: ни удалений, ни модификаций — эти контейнеры
            // не входят в расчётную часть вовсе.
            DBtest (planned.GetSize (), 1, "R6.1 delete_old does not add rows");
            DBtest (f.rule.exsist_elements.GetSize (), 1, "R6.1 existing list untouched by planning");
        }

        // --- суммирование одинаковых ключей в расчётной части ---
        {
            SpecFixture f;
            f.Source (f.first, "A", "Alpha", 2);
            f.Source (f.second, "A", "Alpha", 3);
            Spec::ElementDict planned;
            UnicGuid errors;
            GS::HashTable<GS::UniString, GS::UniString> outParam;
            const Int32 n = Spec::PlanRuleRows (f.rule, f.context, planned, errors, false, outParam);
            // Одна строка, сумма сложена, источников два.
            DBtest (n, 1, "R6.1 merged sources one row");
            DBtest (planned.GetSize (), 1, "R6.1 merged row single");
            const Spec::Element *row = planned.GetPtr ("@A");
            DBrequire (row != nullptr, "R6.1 merged row found");
            DBtest (row->out_sum_param[0].val.intValue, 5, "R6.1 merged sum");
            DBtest (row->elements.GetSize (), 2, "R6.1 merged sources");
        }

        // --- расчёт и полный путь дают одну и ту же строку ---
        // Это главная проверка шага: вынос не изменил ни ключ, ни значения.
        {
            SpecFixture plannedOnly;
            plannedOnly.Source (plannedOnly.first, "A", "Alpha", 2);
            Spec::ElementDict planned;
            UnicGuid planErrors;
            GS::HashTable<GS::UniString, GS::UniString> outParam;
            const Int32 planCount =
                Spec::PlanRuleRows (plannedOnly.rule, plannedOnly.context, planned, planErrors, false, outParam);

            SpecFixture full;
            full.Source (full.first, "A", "Alpha", 2);
            const Int32 fullCount = full.Run ();

            DBtest (planCount, fullCount, "R6.1 planning and full path agree on count");
            DBtest (planned.GetSize (), full.created.GetSize (), "R6.1 planning and full path agree on rows");
            DBtest (planned.ContainsKey ("@A"), full.created.ContainsKey ("@A"), "R6.1 agree on key");
            const Spec::Element *a = planned.GetPtr ("@A");
            const Spec::Element *b = full.created.GetPtr ("@A");
            DBrequire (a != nullptr && b != nullptr, "R6.1 both rows available");
            DBtest (a->out_param.GetSize (), b->out_param.GetSize (), "R6.1 agree on out slot count");
            DBtest (a->out_sum_param.GetSize (), b->out_sum_param.GetSize (), "R6.1 agree on sum slot count");
            DBtest (a->out_param[0].val.uniStringValue, b->out_param[0].val.uniStringValue, "R6.1 agree on out value");
            DBtest (a->out_sum_param[0].val.intValue, b->out_sum_param[0].val.intValue, "R6.1 agree on sum value");
        }

        // --- отказ правила обнуляет расчёт, но НЕ трогает входные словари ---
        {
            SpecFixture f;
            f.Source (f.first, "A", "Alpha", 2);
            f.rule.stop_on_error = true;
            f.rule.groups[0].unic_paramrawname[0] = f.key;
            f.context.read.Get (f.first).Delete (f.key);
            Spec::ElementDict planned;
            UnicGuid errors;
            GS::HashTable<GS::UniString, GS::UniString> outParam;
            const Int32 n = Spec::PlanRuleRows (f.rule, f.context, planned, errors, false, outParam);
            DBtest (n, 0, "R6.1 rejected rule counts zero");
            DBtest (planned.IsEmpty (), true, "R6.1 rejected rule clears rows");
            DBtest (errors.ContainsKey (f.first), true, "R6.1 rejected rule marks element");
        }

        // --- видимость: skip происходит в расчётной части ---
        {
            SpecFixture f;
            f.Source (f.first, "A", "Alpha", 2);
            f.rule.only_visible = true; // элемент синтетический, реальной видимости нет
            Spec::ElementDict planned;
            UnicGuid errors;
            GS::HashTable<GS::UniString, GS::UniString> outParam;
            const Int32 n = Spec::PlanRuleRows (f.rule, f.context, planned, errors, false, outParam);
            DBtest (n, 0, "R6.1 invisible source skipped in planning");
            DBtest (planned.IsEmpty (), true, "R6.1 invisible source yields no rows");
        }
    }

    void TestSpecReadBoundary () {
        // --- Критерий шага: словари до/после чтения ---
        {
            SpecFixture f;
            f.Source (f.first, "A", "Alpha", 2);
            f.Text (f.second, f.text, "Beta");
            f.Text (f.first, f.outText, "Old");
            f.Number (f.first, f.outQuantity, 5);
            // Снимок ДО любых чтений.
            const UInt32 readSizeBefore = f.context.read.GetSize ();
            const UInt32 compositeSizeBefore = f.context.composite.GetSize ();
            const UInt32 listDataSizeBefore = f.context.listData.GetSize ();
            const GS::UniString sourceBefore = f.context.read.Get (f.first).Get (f.text).val.uniStringValue;
            const Int32 quantityBefore = f.context.read.Get (f.first).Get (f.quantity).val.intValue;

            // Чтение обычного поля, материала, отсутствующего поля и за концом слоя.
            const Spec::SpecValueReader reader (f.context);
            ParamValue value;
            DBtest (reader.Read (f.first, f.text, value, 0), "boundary ordinary read");
            DBtest (reader.Read (f.first, f.quantity, value, 0), "boundary number read");
            DBtest (reader.Read (f.first, f.outText, value, 0), "boundary out field read");
            DBtest (reader.Read (f.first, f.key, value, 0), "boundary missing key refuses");
            DBtest (reader.Read (f.first, f.text, value, 100), "boundary past end reads empty");

            // СЛОВАРИ обязаны совпасть со снимком.
            DBtest (f.context.read.GetSize (), readSizeBefore, "boundary read size unchanged");
            DBtest (f.context.composite.GetSize (), compositeSizeBefore, "boundary composite size unchanged");
            DBtest (f.context.listData.GetSize (), listDataSizeBefore, "boundary listdata size unchanged");
            DBtest (f.context.read.Get (f.first).Get (f.text).val.uniStringValue,
                    sourceBefore,
                    "boundary source text intact");
            DBtest (f.context.read.Get (f.first).Get (f.quantity).val.intValue,
                    quantityBefore,
                    "boundary source quantity intact");
            DBtest (f.context.read.Get (f.first).Get (f.outText).val.uniStringValue,
                    GS::UniString ("Old"),
                    "boundary out field not consumed");
            // Соседний элемент не тронут.
            DBtest (f.context.read.Get (f.second).Get (f.text).val.uniStringValue,
                    GS::UniString ("Beta"),
                    "boundary neighbour element intact");
        }

        // --- S14: формулы и отсутствие утечки значений между элементами ---
        // Контракт зафиксирован ЧТЕНИЕМ КОДА, а не предположением:
        //   - обычная формула (без маркеров %elem./%mat./... и без
        //     {@listdata:}) на этапе Read НЕ вычисляется - возвращается
        //     прочитанное значение из словаря как есть (hasLibData == false ->
        //     ветка GetParamValueForElements);
        //   - вычисляется только list-data формула, и делается это в ЛОКАЛЬНЫЙ
        //     словарь paramDict, который создаётся внутри Read и наружу не
        //     отдаётся. Значит результат одного элемента не может попасть в
        //     чтение другого - это и есть «нет утечки».
        // Первая часть: обычная формула отдаёт исходное значение.
        {
            SpecFixture f;
            const GS::UniString plain = FORMULANAMEPREFIX + "{@property:spec-text}<>";
            ParamValue pv;
            pv.isValid = true;
            pv.val.hasFormula = true;
            pv.val.type = API_PropertyRealValueType;
            pv.val.uniStringValue = "10";
            pv.val.doubleValue = 10;
            // Элемент надо завести: read.Get на отсутствующем ключе бросает.
            f.Text (f.first, f.text, "src");
            f.context.read.Get (f.first).Put (plain, pv);
            ParamValue got;
            DBtest (Spec::SpecValueReader (f.context).Read (f.first, plain, got, 0), "S14 plain formula read");
            DBtest (got.val.doubleValue, 10.0, "S14 plain formula not evaluated at read");
            DBtest (got.val.hasFormula, true, "S14 plain formula flag preserved");
        }
        // Вторая часть: два элемента с РАЗНЫМИ list-data формулами дают разные
        // результаты в любом порядке чтения - локальный словарь изолирован.
        {
            SpecFixture f;
            const GS::UniString libName = FORMULANAMEPREFIX + "{@listdata:elem.naen}<>";

            ListData::LibElement libA;
            ListData::Subpos subA;
            ListData::Arm a1;
            a1.naen = "Rebar-A";
            subA.arm.Add ("10@test", a1);
            libA.subpos.Add ("test", subA);
            libA.keys.Push (GS::Pair<GS::UniString, GS::UniString> ("test", "10@test"));
            f.context.listData.Add (f.first, libA);

            ListData::LibElement libB;
            ListData::Subpos subB;
            ListData::Arm a2;
            a2.naen = "Rebar-B";
            subB.arm.Add ("10@test", a2);
            libB.subpos.Add ("test", subB);
            libB.keys.Push (GS::Pair<GS::UniString, GS::UniString> ("test", "10@test"));
            f.context.listData.Add (f.second, libB);

            ParamValue fa;
            fa.isValid = true;
            fa.val.hasFormula = true;
            fa.val.type = API_PropertyStringValueType;
            // В uniStringValue лежит ВЫРАЖЕНИЕ, а не имя с префиксом
            // {@formula: (так устроен рабочий тест TestSpecGetParamValue).
            fa.val.uniStringValue = "{@listdata:elem.naen}<>";
            f.Text (f.first, f.text, "srcA");
            f.Text (f.second, f.text, "srcB");
            f.context.read.Get (f.first).Put (libName, fa);
            ParamValue fb;
            fb.isValid = true;
            fb.val.hasFormula = true;
            fb.val.type = API_PropertyStringValueType;
            fb.val.uniStringValue = "{@listdata:elem.naen}<>";
            f.context.read.Get (f.second).Put (libName, fb);

            const Spec::SpecValueReader reader (f.context);
            ParamValue first, second;
            DBtest (reader.Read (f.first, libName, first, 0), "S14 element A formula read");
            DBtest (reader.Read (f.second, libName, second, 0), "S14 element B formula read");
            // Обратный порядок - результаты те же, утечки нет.
            ParamValue secondFirst, firstSecond;
            DBtest (reader.Read (f.second, libName, secondFirst, 0), "S14 reverse B read");
            DBtest (reader.Read (f.first, libName, firstSecond, 0), "S14 reverse A read");
            DBtest (first.val.uniStringValue, GS::UniString ("Rebar-A"), "S14 element A value");
            DBtest (second.val.uniStringValue, GS::UniString ("Rebar-B"), "S14 element B value");
            DBtest (firstSecond.val.uniStringValue, first.val.uniStringValue, "S14 A stable across order");
            DBtest (secondFirst.val.uniStringValue, second.val.uniStringValue, "S14 B stable across order");
        }

        // --- S16: два правила на одном избранном ---
        // Соседнее правило не должно пострадать от разрешения избранного и от
        // сверки выходной схемы: признаки готовности независимы.
        {
            const GS::UniString OutName ("{@property:spec-out}"), SumName ("{@property:spec-sum}");
            GS::HashTable<GS::UniString, GS::UniString> favorite;
            favorite.Add (OutName, EMPTYSTRING);
            favorite.Add (SumName, EMPTYSTRING);
            ParamDict errors;

            Spec::SpecRule good;
            good.out_paramrawname.Push (OutName);
            good.out_sum_paramrawname.Push (SumName);
            Spec::SpecRule bad;
            bad.out_paramrawname.Push (OutName);
            bad.out_sum_paramrawname.Push ("{@property:spec-missing}");

            // Первое правило проверяется ДО второго: его признак не должен сброситься.
            DBtest (Spec::MatchDestinationProperties (good, favorite, errors), true, "S16 first rule ready");
            DBtest (Spec::MatchDestinationProperties (bad, favorite, errors), false, "S16 second rule not ready");
            DBtest (good.destinationReady, true, "S16 first rule flag survived neighbour");
            DBtest (bad.destinationReady, false, "S16 second rule flag cleared");
            DBtest (good.out_paramrawname.GetSize (), 1, "S16 first rule schema intact");
            DBtest (good.out_sum_paramrawname.GetSize (), 1, "S16 first rule sum schema intact");
            // Обратный порядок: неполное правило не должно влиять на полное.
            ParamDict errors2;
            Spec::MatchDestinationProperties (bad, favorite, errors2);
            DBtest (Spec::MatchDestinationProperties (good, favorite, errors2), true, "S16 good ready after bad");
            DBtest (good.destinationReady, true, "S16 good flag kept after bad");
            // Отсутствие избранного целиком: оба правила не готовы, ошибка одна.
            ParamDict errors3;
            GS::HashTable<GS::UniString, GS::UniString> emptyFavorite;
            DBtest (
                Spec::MatchDestinationProperties (good, emptyFavorite, errors3), false, "S16 no favorite not ready");
            DBtest (Spec::MatchDestinationProperties (bad, emptyFavorite, errors3),
                    false,
                    "S16 no favorite both not ready");
            DBtest (errors3.GetSize (), 3, "S16 three missing names recorded");
        }

        // --- S13: семантика первого ряда GDL-массива на чтении ---
        // Разбор имени @arr - territory Helpers (ConvertStringToParamValue,
        // Helpers.cpp:6220), он читает через ACAPI и R5 его не трогает. Здесь
        // закрепляется только то, что видит вычислитель.
        //
        // Предусловие найдено ЧТЕНИЕМ КОДА, и оно неочевидно: признак
        // fromGDLArray попадает в pvalue только если Read дошёл до материальной
        // ветки. Обычный отказ (нет элемента/ключа, isValid == false) pvalue не
        // заполняет, и тогда fromGDLArray == false, а is_error = !fromMaterial
        // даёт обычную ошибку. Маршрут с fromGDLArray воспроизводится так:
        // значение валидно И fromMaterial, но состава конструкции нет.
        {
            SpecFixture f;
            f.Source (f.first, "A", "Alpha", 2);
            f.rule.groups[0].unic_paramrawname[0] = f.text;
            f.rule.stop_on_error = true;

            // Валидное значение с признаком материала и без состава: Read
            // возвращает false, но pvalue сохраняет признаки источника.
            ParamValue firstRow = {};
            firstRow.isValid = true;
            firstRow.fromMaterial = true;
            firstRow.fromGDLArray = true;
            firstRow.val.array_row_start = 1;
            f.context.read.Get (f.first).Put (f.text, firstRow);
            f.Shape (f.Run (), 0, 0, 0, 0, "S13 first row stops");
            DBtest (f.errors.ContainsKey (f.first), true, "S13 first row marks element error");

            // Второй ряд: тот же маршрут, но array_row_start == 2 — отказ не
            // является ошибкой элемента.
            ParamValue secondRow = firstRow;
            secondRow.val.array_row_start = 2;
            f.context.read.Get (f.first).Put (f.text, secondRow);
            f.Shape (f.Run (), 0, 0, 0, 0, "S13 second row skips silently");
            DBtest (f.errors.ContainsKey (f.first), false, "S13 second row not an error");

            // Контроль маршрута, найденный чтением кода: признаки материала
            // НЕЛЬЗЯ снять и ждать отказа. Без fromMaterial Read возвращает
            // успех по прочитанному значению, строка создаётся, а ветка
            // is_error = !fromMaterial не оценивается вовсе. Проверка закрепляет
            // именно это, чтобы отличие от маршрута GDL было явным.
            ParamValue plain = firstRow;
            plain.fromMaterial = false;
            f.context.read.Get (f.first).Put (f.text, plain);
            f.Shape (f.Run (), 1, 1, 0, 0, "S13 non material reads fine");
            DBtest (f.errors.ContainsKey (f.first), false, "S13 non material is not an error");
        }
    }

    void TestSpecValueEdges () {
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
    }

    void TestSpecExpandGroup () {
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
    }

    void TestSpecGroups () {
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
    }

    void TestSpecOutputSchema () {
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
    }

    void TestSpecPolicy () {
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
    }

    void TestSpecNormalize () {
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
    }

    void TestSpecParser () {
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
    }

    void TestSpecAddRule () {
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
    }

    void TestSpecSizes () {
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
    }

} // namespace TestFunc
#endif
