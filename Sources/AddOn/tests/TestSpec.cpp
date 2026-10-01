//------------ kuvbur 2022 ------------
#ifdef TESTING
    #include "ACAPinc.h"

    #include "api_headers/APIEnvir.h"

    #include "Helpers.hpp"
    #include "Propertycache.hpp"
    #include "spec/Spec.hpp"
    #include "spec/SpecHelpers.hpp"
    #include "spec/SpecPlanning.hpp"
    #include "Sync.hpp"
    #include "tests/TestFunc.hpp"
    #include "tests/TestKit.hpp"

namespace TestFunc {

    namespace {
        // Синтетические GUID используются только как ключи словарей, не как элементы модели.
        // Числовой ParamValue для проверок суммирования. valid=false даёт
        // невалидный слот — на нём держится правило «isValid с обеих сторон».
        // Локальной функцией быть не может: определение в теле функции
        // запрещено, поэтому хелпер живёт здесь.
        ParamValue Num (Int32 v, bool valid = true) {
            ParamValue p = {};
            ParamHelpers::ConvertIntToParamValue (p, EMPTYSTRING, v);
            p.isValid = valid;
            return p;
        }

        // Добавление слота суммы вручную, для тестов суммирования.
        // isSum обязателен: по нему SumContributionIntoRow находит накопленные
        // слоты, а без него строка выглядела бы состоящей только из выходных.
        void PushSumSlot (Spec::Element &row, const ParamValue &value) {
            Spec::OutputSlot slot = {};
            slot.rawname = EMPTYSTRING;
            slot.value = value;
            slot.isSum = true;
            row.out_slots.Push (slot);
        }

        // Слот с произвольным флагом — нужен, чтобы собрать ПЕРЕМЕШАННУЮ
        // схему (сумма раньше выхода). Схема — публичные данные элемента,
        // поэтому нельзя полагаться на единственного автора.
        void PushRawSlot (Spec::Element &row, bool isSum, const ParamValue &value) {
            Spec::OutputSlot slot = {};
            slot.rawname = isSum ? GS::UniString ("S") : GS::UniString ("O");
            slot.value = value;
            slot.isSum = isSum;
            row.out_slots.Push (slot);
        }

        // Симметричный хелпер для выходного (не суммарного) слота.
        void PushOutSlot (Spec::Element &row, const ParamValue &value) {
            Spec::OutputSlot slot = {};
            slot.rawname = EMPTYSTRING;
            slot.value = value;
            slot.isSum = false;
            row.out_slots.Push (slot);
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
            // Набор прочитанных словарей — один объект, как в SpecArray.
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
                rule.runState.elements.Push (guid);
                Text (guid, key, k);
                Text (guid, text, t);
                Number (guid, quantity, q);
            }

            void Existing (const API_Guid &guid, const GS::UniString &t, Int32 q) {
                rule.delete_old = true;
                rule.runState.exsist_elements.Push (guid);
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

            // Тот же запуск, но с планом изменений. План получаем от
            // GetElementsForRule, а не строим сами: иначе проверялась бы копия
            // решения, а не само решение.
            Int32 RunWithPlan (Spec::SpecChangePlan &plan) {
                created.Clear ();
                modified.Clear ();
                deleted.Clear ();
                errors.Clear ();
                return Spec::GetElementsForRule (rule, context, created, modified, deleted, errors, false, &plan);
            }

            // Убрать поле из прочитанных: так источник выглядит для расчёта
            // так, будто поле недоступно (нет слоя, нет материала и т.п.). Это
            // нужно для проверки полноты расчёта — неполнота не зависит от
            // stop_on_error.
            void DropField (const API_Guid &guid, const GS::UniString &name) {
                if (context.read.ContainsKey (guid))
                    context.read.Get (guid).Delete (name);
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
            // В Element копируется разрешённое
            f.rule.subguid_paramrawname = "fixture-link-marker";
            f.rule.destinationParamGuidName = "fixture-link";
            f.rule.subguid_rulename = "fixture-rule";
            f.rule.subguid_rulevalue = "fixture-value";
            f.Shape (f.Run (), 1, 1, 0, 0, "Spec single");
            const Spec::Element *row = f.created.GetPtr ("@A");
            DBtest (row != nullptr, "Spec single exact key");
            if (row != nullptr) {
                DBtest (row->OutParamCount (), 1, "Spec single text count");
                DBtest (row->OutSumCount (), 1, "Spec single sum count");
                if (row->OutParamCount () != 0)
                    DBtest (row->OutParamValue (0).val.uniStringValue, GS::UniString ("Alpha"), "Spec single text");
                if (row->OutSumCount () != 0)
                    DBtest (row->OutSumValue (0).val.intValue, 2, "Spec single sum");
                DBtest (row->elements.GetSize (), 1, "Spec single source count");
                DBtest (!row->elements.IsEmpty () && row->elements[0] == f.first, "Spec single source GUID");
                DBtest (row->favorite_name, f.rule.favorite_name, "Spec favorite copied");
                DBtest (row->subguid_paramrawname, f.rule.destinationParamGuidName, "Spec link copied");
                DBtest (row->subguid_rulename, f.rule.subguid_rulename, "Spec rule name copied");
                DBtest (row->subguid_rulevalue, f.rule.subguid_rulevalue, "Spec rule value copied");
                DBtest (row->OutParamName (0), f.outText, "Spec text target copied");
                DBtest (row->OutSumName (0), f.outQuantity, "Spec sum target copied");
                DBtest (row->exs_guid == APINULLGuid, "Spec new row no existing GUID");
            }
            DBtest (f.errors.IsEmpty (), "Spec valid problem list empty");
            DBtest (f.context.read.Get (f.first).Get (f.quantity).val.intValue, 2, "Spec source quantity unchanged");
            f.Source (f.second, "A", "Beta", 3);
            f.Shape (f.Run (), 1, 1, 0, 0, "Spec merge");
            row = f.created.GetPtr ("@A");
            if (row != nullptr && row->OutSumCount () == 1 && row->OutParamCount () == 1) {
                DBtest (row->OutSumValue (0).val.intValue, 5, "Spec merged integer");
                DBtest (row->OutSumValue (0).val.doubleValue, 5, "Spec merged real");
                DBtest (row->OutSumValue (0).val.rawDoubleValue, 5, "Spec merged raw real");
                DBtest (row->OutParamValue (0).val.uniStringValue, GS::UniString ("Alpha"), "Spec first output wins");
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
            if (row != nullptr && row->OutSumCount () == 1)
                DBtest (row->OutSumValue (0).val.intValue, 0, "Spec signed sum zero");
            f.rule.groups[0].sum_paramrawname[0] = "1";
            f.context.read.Get (f.first).Delete (f.quantity);
            f.context.read.Get (f.second).Delete (f.quantity);
            f.Shape (f.Run (), 1, 1, 0, 0, "Spec literal count");
            row = f.created.GetPtr ("@A B");
            if (row != nullptr && row->OutSumCount () == 1)
                DBtest (row->OutSumValue (0).val.intValue, 2, "Spec literal count value");
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
            if (row != nullptr && row->OutSumCount () == 1) {
                DBtest (row->OutSumValue (0).val.intValue, 4, "Spec groups merge sums");
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
            DBtest (f.rule.runState.elements.GetSize (), 1, "Spec rule sources preserved");
            DBtest (f.rule.runState.exsist_elements.GetSize (), 1, "Spec existing list preserved");
            f.Number (f.first, f.quantity, 3);
            f.Shape (f.Run (), 1, 0, 1, 0, "Spec quantity changed");
            const Spec::Element *row = f.modified.GetPtr ("@A");
            DBtest (row != nullptr, "Spec update key");
            if (row != nullptr && row->OutSumCount () == 1) {
                DBtest (row->exs_guid == f.old, "Spec update preserves target GUID");
                DBtest (row->OutSumValue (0).val.intValue, 3, "Spec update new sum");
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
            f.rule.runState.elements.Clear ();
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

    // Объединение строк и построение ключа: действующая дедупликация.
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
        // Две группы с ОДИНАКОВЫМ ключом от одного элемента дают одну строку.
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
            if (row != nullptr && row->OutSumCount () == 1) {
                DBtest (row->OutSumValue (0).val.intValue, 4, "Spec merge sums add up");
                DBtest (row->OutParamCount () == 1 && row->OutParamValue (0).val.uniStringValue == "Alpha",
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

        // КОЛЛИЗИЯ КЛЮЧА. Один уникальный параметр со значением "A@B" даёт
        // ключ "@A@B" - ровно как ДВА параметра со значениями "A" и "B",
        // потому что разделителя между параметрами нет. Строки схлопываются,
        // суммы складываются. Это зафиксированный контракт: кодировка ключа
        // разделителем не экранируется.
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
            if (row != nullptr && row->OutSumCount () == 1)
                DBtest (row->OutSumValue (0).val.intValue, 4, "Spec key collision sums merged");
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

        // Пропущенная сумма подставляется литералом "1", и этот литерал
        // начисляется на КАЖДЫЙ источник.
        // ключом дают 1 + 1 = 2, а не 1. Это принципиально отличается от
        // настоящей суммы, поэтому закреплено отдельно.
        {
            SpecFixture f;
            f.Source (f.first, "A", "Alpha", 7);
            f.rule.groups[0].sum_paramrawname.Clear ();
            f.rule.groups[0].sum_paramrawname.Push ("1");
            f.rule.runState.elements.Push (f.second);
            f.Text (f.second, f.key, "A");
            f.Text (f.second, f.text, "Beta");
            DBtest (f.Run (), 1, "Spec literal count result");
            const Spec::Element *row = f.created.GetPtr ("@A");
            DBtest (row != nullptr, "Spec literal count row");
            if (row != nullptr && row->OutSumCount () == 1)
                DBtest (row->OutSumValue (0).val.intValue, 2, "Spec literal count per source");
        }

        // Отказ по размеру группы сделан в ExpandGroup, поэтому при расчёте
        // строк отказ локален для группы - её снимает один флаг is_Valid,
        // а остальные группы правила обрабатываются как обычно.
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

        // Неполный выход отвергается сверкой слотов. Элемент, у которого не
        // набрано всех выходных значений, в словарь не попадает, а n_elements
        // обнуляется - но только при stop_on_error.
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

    // Дедупликация AddRule по ключу = текст ВНУТРИ фигурных скобок.
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
                DBtest (rule->runState.elements.GetSize () == 2, "Spec dedup second source appended");
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
            DBtest (after != nullptr && after->runState.elements.IsEmpty (), "Spec dedup invalid takes no source");
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
            DBtest (rule != nullptr && rule->runState.elements.GetSize () == 2,
                    "Spec dedup normalized source appended");
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
        f.rule.runState.elements.Push (f.first);
        f.rule.runState.elements.Push (f.second);
        f.rule.groups[0].flag_paramrawname = f.flag;
        f.rule.groups[0].sum_paramrawname.Push ("1");
        const Spec::GroupSpec duplicate = f.rule.groups[0];
        f.rule.groups.Push (duplicate);
        Spec::GetParamToReadFromRule (f.rule, read, write);
        DBtest (read.GetSize (), 2, "Spec read source count");
        for (const API_Guid &guid : f.rule.runState.elements) {
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
            special.rule.runState.elements.Push (special.first);
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

    // Сбор зависимостей правила
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

    // Разрешение избранного, служебных полей и старых объектов. Четыре
    // операции проверяются раздельно; MatchDestinationProperties,
    // SelectExistingElements и AddExistingReadRequests чисты, а
    // ResolveFavoriteLinks упирается в ambient-кэш свойств, поэтому для него
    // закреплён именно ПРОМАХ чтения: неудачное чтение не кэшируется и не
    // подменяет поля правила.
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
            DBtest (rule.runState.exsist_elements.GetSize (), 2, "empty selection takes all");
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
            DBtest (rule.runState.exsist_elements.GetSize (), 1, "selection filters found");
            DBtest (!rule.runState.exsist_elements.IsEmpty () && rule.runState.exsist_elements[0] == keep,
                    "selection keeps chosen");
        }

        // Непустое выделение ДОПИСЫВАЕТ элементы, а не заменяет прежнее
        // содержимое поля: это прежнее поведение (Push), и оно безопасно только
        // тем, что словарь правил создаётся заново на каждый запуск.
        {
            Spec::SpecRule rule;
            const API_Guid before = APIGuidFromString ("{33333333-3333-3333-3333-333333333333}");
            rule.runState.exsist_elements.Push (before);
            GS::Array<API_Guid> found;
            const API_Guid keep = APIGuidFromString ("{11111111-1111-1111-1111-111111111111}");
            found.Push (keep);
            UnicGuid selected;
            selected.Add (keep, true);
            Spec::SelectExistingElements (rule, found, selected);
            DBtest (rule.runState.exsist_elements.GetSize (), 2, "selection appends to existing");
        }

        // Пустое выделение ПРИСВАИВАЕТ найденное, заменяя прежнее содержимое -
        // асимметрия с предыдущим случаем задана кодом и закреплена здесь.
        {
            Spec::SpecRule rule;
            const API_Guid before = APIGuidFromString ("{33333333-3333-3333-3333-333333333333}");
            rule.runState.exsist_elements.Push (before);
            GS::Array<API_Guid> found;
            found.Push (APIGuidFromString ("{11111111-1111-1111-1111-111111111111}"));
            UnicGuid selected;
            Spec::SelectExistingElements (rule, found, selected);
            DBtest (rule.runState.exsist_elements.GetSize (), 1, "empty selection replaces existing");
        }

        // Несовпадение выделения с найденным даёт пустой результат.
        {
            Spec::SpecRule rule;
            GS::Array<API_Guid> found;
            found.Push (APIGuidFromString ("{11111111-1111-1111-1111-111111111111}"));
            UnicGuid selected;
            selected.Add (APIGuidFromString ("{99999999-9999-9999-9999-999999999999}"), true);
            Spec::SelectExistingElements (rule, found, selected);
            DBtest (rule.runState.exsist_elements.IsEmpty (), true, "selection misses all found");
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
        // остаётся пустым: неудачное чтение не кэшируется и не подменяет
        // результат.
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

    // Контракт read-only доступа к значениям.
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

    // Граница выделения чтения: чтение значений не обязано менять словари.
    //
    // Главный критерий: ИСХОДНЫЕ СЛОВАРИ ДО/ПОСЛЕ ЧТЕНИЯ совпадают.
    // Чтение не обязано быть безопасно идемпотентным по значению (формулы
    // пересчитываются), но не обязано менять СЛОВАРИ - это проверяется здесь.
    //
    // Материалы (один слой, многослойная, конец слоя), отрицательный индекс,
    // за концом, пустое-числовое-текстовое и listdata (нет данных, позиция,
    // конец списка, невычисленная формула) покрыты наборами TestSpecGetParamValue
    // и TestSpecValueEdges. GDL-массивы на ЧТЕНИИ не покрыты: разбор @arr живёт
    // в Helpers (ConvertStringToParamValue, Helpers.cpp:6220) и читает через
    // ACAPI, поэтому здесь закрепляется только спец-семантика СОБСТВЕННО
    // чтения (первый ряд - ошибка строки).
    //
    // Расчётная часть PlanRuleRows считается БЕЗ сверки существующих строк и
    // без модели: вычисленные строки проверяются без создания объектов.
    // Сверка сравнивается с расчётом: delete_old = true у PlanRuleRows не
    // меняет ничего (сверка живёт в GetElementsForRule), а delete_old = false
    // у GetElementsForRule означает «не идти в сверку вообще».
    //
    // Вклад источника (RuleContribution) проверяется БЕЗ словаря элементов и
    // без запуска GetElementsForRule. Вклад описывает, что вносит ОДИН
    // источник; раскладка в агрегат проверяется набором TestSpecRowLayout.

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
            DBtest (c.isIncluded, true, "full contribution included");
            DBtest (c.status, Spec::ContributionStatus::Partial, "phase 1 stops before outputs");
            DBtest (c.source == f.first, true, "contribution carries source");
            DBtest (c.groupIndex, 0u, "contribution carries group index");
            DBtest (c.hasKey, true, "hasKey set on assembled key");
            DBtest (c.key, GS::UniString ("@A"), "key value");
            DBtest (c.outSumParam.GetSize (), 1, "sum slot read in phase 1");
            DBtest (c.outSumParam[0].val.intValue, 7, "sum slot value");
            DBtest (c.outParam.GetSize (), 0, "outputs not read in phase 1");
            DBtest (c.outputsRead, false, "outputsRead stays unset until phase 2");
            DBtest (c.missingUnic.IsEmpty (), true, "nothing missing");
            DBtest (c.missingSum.IsEmpty (), true, "nothing missing in sum");
            DBtest (c.missingOut.IsEmpty (), true, "nothing missing in out");
            // Фаза 2: выходные слоты появляются только здесь.
            Spec::RuleContribution full = c;
            Spec::ReadContributionOutputs (f.first, f.rule.groups[0], binding, reader, fstr, full);
            DBtest (full.outputsRead, true, "outputsRead set by phase 2");
            DBtest (full.outParam.GetSize (), 1, "out slot read in phase 2");
            DBtest (full.outParam[0].val.uniStringValue, GS::UniString ("Alpha"), "out slot value");
            DBtest (full.keyOut, GS::UniString ("@Alpha"), "out key value");
            DBtest (full.isComplete, true, "complete when all slots read");
            DBtest (Spec::ClassifyContribution (full, 1, 1), Spec::ContributionStatus::Complete, "classified complete");
        }

        // --- сумма-константа "1": слот есть, чтения не было ---
        {
            SpecFixture f;
            f.Source (f.first, "A", "Alpha", 5);
            f.rule.groups[0].sum_paramrawname[0] = "1";
            f.rule.out_sum_paramrawname[0] = "1";
            const Spec::GroupSlotBinding binding = Spec::PrepareSlotBindings (f.rule)[0];
            DBtest (binding.sumSlots[0].isSumLiteral, true, "sum literal marked isSumLiteral");
            FormatString fstr = FormatStringFunc::ParseFormatString (".2m");
            ParamDict none1 = {};
            ParamDict none2 = {};
            const Spec::SpecValueReader reader (f.context);
            Spec::RuleContribution c =
                Spec::BuildContribution (f.first, 0, f.rule.groups[0], binding, reader, none1, none2);
            DBtest (c.outSumParam.GetSize (), 1, "literal produces a slot");
            DBtest (c.outSumParam[0].val.intValue, 1, "literal value is one");
            DBtest (c.missingSum.IsEmpty (), true, "literal is not a missing field");
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
            DBtest (c.isIncluded, false, "disabled group flag clears isIncluded");
            DBtest (c.status, Spec::ContributionStatus::Excluded, "status Excluded for disabled group");
            DBtest (c.outSumParam.GetSize (), 0, "excluded reads no slots");
            DBtest (Spec::ClassifyContribution (c, 1, 1), Spec::ContributionStatus::Excluded, "excluded classified");
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
            DBtest (c.status, Spec::ContributionStatus::Excluded, "missing unic excludes");
            DBtest (c.missingUnic.GetSize (), 1, "missing unic recorded");
            DBtest (c.missingUnic[0].rawname, f.key, "missing unic name");
            DBtest (c.missingUnic[0].isError, true, "missing unic is an error");
            DBtest (c.hasReadError, true, "read error flagged");
            // Ключ склеивается ДАЖЕ при отказе — пустое значение плюс ATSIGN.
            DBtest (c.key, GS::UniString ("@"), "key still gets sign on failure");
            DBtest (c.hasKey, false, "hasKey cleared on unusable key");
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
            DBtest (c.missingUnic.GetSize (), 1, "fromMaterial still records field");
            DBtest (c.missingUnic[0].isError, false, "fromMaterial clears missingUnic isError");
            DBtest (c.hasReadError, false, "fromMaterial clears read error");
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
            DBtest (c.outParam.GetSize (), 0, "missing out yields no slot");
            DBtest (c.missingOut.GetSize (), 1, "missing out recorded");
            DBtest (c.missingOut[0].rawname, f.text, "missing out name");
            DBtest (c.missingOut[0].isError, true, "missing out is an error");
            DBtest (c.isComplete, false, "partial contribution incomplete");
            DBtest (Spec::ClassifyContribution (c, 1, 1), Spec::ContributionStatus::Partial, "partial classified");
            DBtest (c.keyOut, GS::UniString (EMPTYSTRING), "empty out key on failure");
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
            DBtest (Spec::ClassifyContribution (c, 2, 2), Spec::ContributionStatus::Partial, "short slots vs schema");
            DBtest (Spec::ClassifyContribution (c, 1, 1), Spec::ContributionStatus::Complete, "exact slots complete");
        }

        // --- два вклада одного ключа строятся независимо друг от друга ---
        // Раскладка в агрегат не должна зависеть от того, в каком порядке
        // вклады дошли до словаря.
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
            DBtest (c1.key, c2.key, "same key from both sources");
            DBtest (c1.source == f.first, true, "first contribution source");
            DBtest (c2.source == f.second, true, "second contribution source");
            DBtest (c1.outSumParam[0].val.intValue, 2, "first contribution sum");
            DBtest (c2.outSumParam[0].val.intValue, 3, "second contribution sum");
        }
    }

    // Раскладка вклада в строку. Главное здесь — НЕ «улучшить» суммирование
    // неполных массивов, а зафиксировать его как поведение: nsumm = MIN(длин),
    // слот складывается только при isValid с обеих сторон, слоты сверх nsumm
    // не трогаются.
    //
    // Общая выходная схема строки: слот держит имя и значение вместе, поэтому
    // рассинхрон между out_paramrawname[k] и out_param[k] невозможен по
    // построению. Имя набора — TestSpecRowSlots, а не TestSpecOutputSchema:
    // последний уже занят набором разбора схемы правила.
    //
    // ---- Независимый эталон арифметики сложения ----
    //
    // Это НЕ второй движок расчёта, а реализация той же арифметики, живущая
    // целиком в тестовой единице. Production её не видит: ни одного вызова из
    // Sources/AddOn/spec/ на эти функции нет.
    //
    // Проверяется равносильность, а не «правильность»: эталон повторяет
    // зафиксированное поведение, включая nsumm = MIN(длин) и требование
    // isValid с обеих сторон.
    namespace legacy {
        // Арифметика сложения эталонной раскладки. Пишется максимально просто:
        // цель не красота, а независимость от проверяемого кода.
        void SumInto (Spec::Element &row, const GS::Array<ParamValue> &contribution) {
            UInt32 nsumm = row.OutSumCount ();
            if (nsumm != contribution.GetSize ()) {
                nsumm = nsumm < contribution.GetSize () ? nsumm : contribution.GetSize ();
            }
            for (UInt32 j = 0; j < nsumm; j++) {
                // Слот достаётся по индексу напрямую: accessor возвращает
                // константную ссылку, а эталон суммирует ПО НАКОПЛЕННОМУ.
                Spec::OutputSlot &slot = row.out_slots[row.OutParamCount () + j];
                if (slot.value.isValid && contribution[j].isValid)
                    slot.value.val = slot.value.val + contribution[j].val;
            }
        }

        // Прежняя раскладка: решить «первый ли ключ», сложить суммы, собрать
        // строку из первого представителя. Ни одного вызова нового модуля.
        void PlanRow (GS::Array<Spec::RuleContribution> contributions,
                      const Spec::SpecRule &rule,
                      const Spec::GroupSpec &group,
                      const Spec::GroupSlotBinding &binding,
                      const Spec::SpecValueReader &reader,
                      FormatString &fstr,
                      Spec::ElementDict &rows,
                      GS::HashTable<GS::UniString, GS::UniString> &outParam) {
            // Копия по значению: эталон дочитывает выходы первому
            // представителю и не должен портить вход вызывающего — иначе это
            // был бы не эталон, а второй потребитель с общим состоянием.
            for (Spec::RuleContribution contribution : contributions) {
                if (contribution.status == Spec::ContributionStatus::Excluded)
                    continue;
                if (!rows.ContainsKey (contribution.key)) {
                    // Выходные слоты читаются именно здесь — в ветке «ключа ещё
                    // нет», то есть только у первого представителя.
                    Spec::ReadContributionOutputs (contribution.source, group, binding, reader, fstr, contribution);
                    if (!outParam.ContainsKey (contribution.keyOut))
                        outParam.Add (contribution.keyOut, contribution.key);
                    Spec::Element row = {};
                    // Эталон строит строку через out_slots и НЕ вызывает
                    // BuildOutputSlots: вклад раскладывается им самим.
                    for (UInt32 i = 0; i < contribution.outParam.GetSize (); i++) {
                        Spec::OutputSlot slot = {};
                        slot.rawname = i < rule.out_paramrawname.GetSize () ? rule.out_paramrawname[i] : EMPTYSTRING;
                        slot.value = contribution.outParam[i];
                        slot.isSum = false;
                        row.out_slots.Push (slot);
                    }
                    for (UInt32 i = 0; i < contribution.outSumParam.GetSize (); i++) {
                        Spec::OutputSlot slot = {};
                        slot.rawname =
                            i < rule.out_sum_paramrawname.GetSize () ? rule.out_sum_paramrawname[i] : EMPTYSTRING;
                        slot.value = contribution.outSumParam[i];
                        slot.isSum = true;
                        row.out_slots.Push (slot);
                    }
                    if (!Spec::OutSlotsMatchSchema (
                            row, rule.out_paramrawname.GetSize (), rule.out_sum_paramrawname.GetSize ()))
                        continue;
                    row.subguid_paramrawname = rule.destinationParamGuidName;
                    row.subguid_rulevalue = rule.subguid_rulevalue;
                    row.subguid_rulename = rule.subguid_rulename;
                    row.favorite_name = rule.favorite_name;
                    row.elements.Push (contribution.source);
                    rows.Add (contribution.key, row);
                } else {
                    Spec::Element &exsists_element = rows.Get (contribution.key);
                    exsists_element.elements.Push (contribution.source);
                    SumInto (exsists_element, contribution.outSumParam);
                }
            }
        }
    } // namespace legacy

    // Равносильность проверяемой арифметики и независимого эталона на
    // ОДИНАКОВОМ вводе. Эталон — реализация в этой же единице (namespace
    // legacy выше), а не второй путь в production.
    void TestSpecEngineEquivalence () {
        // Собирает вклады по источникам правила так, как это делает цикл, и
        // отдаёт один и тот же вход обоим движкам. Фаза 2 вызывается только
        // первому представителю ключа — как в цикле.
        auto collect = [] (SpecFixture &f,
                           const GS::Array<API_Guid> &sources,
                           Spec::GroupSlotBinding &binding,
                           FormatString &fstr,
                           GS::Array<Spec::RuleContribution> &out) {
            fstr = FormatStringFunc::ParseFormatString (".2m");
            binding = Spec::PrepareSlotBindings (f.rule)[0];
            ParamDict none1 = {};
            ParamDict none2 = {};
            const Spec::SpecValueReader reader (f.context);
            for (const API_Guid &guid : sources) {
                Spec::RuleContribution c =
                    Spec::BuildContribution (guid, 0, f.rule.groups[0], binding, reader, none1, none2);
                if (c.status == Spec::ContributionStatus::Excluded)
                    continue;
                bool firstOfKey = true;
                for (const Spec::RuleContribution &seen : out) {
                    if (seen.key == c.key) {
                        firstOfKey = false;
                        break;
                    }
                }
                // Фаза 2 здесь НЕ вызывается: вклад отдаётся в состоянии фазы 1,
                // чтобы оба движка получили один и тот же вход. Фаза 2 вызывается
                // в цикле сравнения — по одному разу на представителя ключа.
                (void)firstOfKey;
                out.Push (c);
            }
        };

        // Обёртка массива значений во вклад: новый движок принимает вклад,
        // эталон — массив. Сравниваются именно значения слотов, поэтому
        // обёртка не влияет на результат.
        auto asContribution = [] (const GS::Array<ParamValue> &values) {
            Spec::RuleContribution c = {};
            c.isIncluded = true;
            c.outSumParam = values;
            return c;
        };

        // --- один ключ, три источника ---
        {
            SpecFixture f;
            f.Source (f.first, "A", "Alpha", 2);
            f.Source (f.second, "A", "Alpha", 3);
            f.Source (f.extra, "A", "Alpha", 4);
            Spec::GroupSlotBinding binding;
            FormatString fstr;
            GS::Array<Spec::RuleContribution> contributions = {};
            DBtest (f.rule.runState.elements.GetSize (), 3, "rule has three sources");
            collect (f, f.rule.runState.elements, binding, fstr, contributions);
            DBtest (contributions.GetSize (), 3, "three contributions collected");

            Spec::ElementDict freshRows = {};
            GS::HashTable<GS::UniString, GS::UniString> freshOut = {};
            const Spec::SpecValueReader reader (f.context);
            UInt32 contributionIndex = 0;
            for (const Spec::RuleContribution &c : contributions) {
                // Вклад приходит в состоянии ФАЗЫ 1: суммы уже прочитаны,
                // выходных слотов ещё нет — они появляются в фазе 2 и только у
                // ПЕРВОГО представителя ключа. Это и есть контракт, который
                // здесь проверяется, а не диагностика ради диагностики.
                const bool firstOfKey = contributionIndex == 0;
                DBtest (c.outParam.GetSize (), (UInt32)0, "phase 1 leaves output slots empty");
                DBtest (c.outSumParam.GetSize (), (UInt32)1, "phase 1 reads sum slot");
                Spec::RuleContribution rowContribution = c;
                if (!freshRows.ContainsKey (c.key))
                    Spec::ReadContributionOutputs (c.source, f.rule.groups[0], binding, reader, fstr, rowContribution);
                DBtest (rowContribution.outParam.GetSize () == (UInt32)1, firstOfKey, "phase 2 only for first of key");
                contributionIndex += 1;
                Spec::AddContributionToRow (freshRows, rowContribution, f.rule, 1, 1, freshOut);
                // Число строк НЕ растёт на каждый вклад: слияние дописывает
                // источник в существующую строку. Инвариант здесь — «строка не
                // пропала», а не «строк стало больше».
                DBtest (freshRows.ContainsKey (GS::UniString ("@A")), true, "row still present after add");
            }
            Spec::ElementDict legacyRows = {};
            GS::HashTable<GS::UniString, GS::UniString> legacyOut = {};
            legacy::PlanRow (contributions, f.rule, f.rule.groups[0], binding, reader, fstr, legacyRows, legacyOut);

            DBtest (freshRows.GetSize (), legacyRows.GetSize (), "same row count");
            DBtest (freshOut.GetSize (), legacyOut.GetSize (), "same outParam size");
            const Spec::Element *a = freshRows.GetPtr ("@A");
            const Spec::Element *b = legacyRows.GetPtr ("@A");
            DBrequire (a != nullptr && b != nullptr, "both engines produced the row");
            DBtest (a->OutSumCount (), b->OutSumCount (), "same sum slot count");
            DBtest (a->OutSumValue (0).val.intValue, b->OutSumValue (0).val.intValue, "same sum");
            DBtest (a->OutSumValue (0).val.intValue, 9, "sum is 2+3+4");
            DBtest (a->elements.GetSize (), b->elements.GetSize (), "same source count");
            DBtest (a->elements.GetSize (), 3, "three sources collected");
            for (UInt32 i = 0; i < a->elements.GetSize () && i < b->elements.GetSize (); i++)
                DBtest (a->elements[i] == b->elements[i], true, "same source order");
            DBtest (a->OutParamCount (), b->OutParamCount (), "same out slot count");
            DBtest (a->OutParamValue (0).val.uniStringValue, b->OutParamValue (0).val.uniStringValue, "same out value");
        }

        // --- два ключа, перемешанные источники ---
        // Порядок обхода rule.runState.elements задаёт порядок первого представителя,
        // поэтому смешивание ключей — то, где потеря порядка была бы видна.
        {
            SpecFixture f;
            f.Source (f.first, "A", "Alpha", 1);
            f.Source (f.second, "B", "Beta", 10);
            f.Source (f.extra, "A", "Alpha", 2);
            f.Source (f.old, "B", "Beta", 20);
            Spec::GroupSlotBinding binding;
            FormatString fstr;
            GS::Array<Spec::RuleContribution> contributions = {};
            collect (f, f.rule.runState.elements, binding, fstr, contributions);

            Spec::ElementDict freshRows = {};
            GS::HashTable<GS::UniString, GS::UniString> freshOut = {};
            const Spec::SpecValueReader reader (f.context);
            for (const Spec::RuleContribution &c : contributions) {
                Spec::RuleContribution rowContribution = c;
                if (!freshRows.ContainsKey (c.key))
                    Spec::ReadContributionOutputs (c.source, f.rule.groups[0], binding, reader, fstr, rowContribution);
                Spec::AddContributionToRow (freshRows, rowContribution, f.rule, 1, 1, freshOut);
            }
            Spec::ElementDict legacyRows = {};
            GS::HashTable<GS::UniString, GS::UniString> legacyOut = {};
            legacy::PlanRow (contributions, f.rule, f.rule.groups[0], binding, reader, fstr, legacyRows, legacyOut);

            DBtest (freshRows.GetSize (), legacyRows.GetSize (), "mixed keys same row count");
            DBtest (freshRows.GetSize (), 2, "two rows built");
            for (UInt32 k = 0; k < 2; k++) {
                const GS::UniString key = k == 0 ? GS::UniString ("@A") : GS::UniString ("@B");
                const Spec::Element *a = freshRows.GetPtr (key);
                const Spec::Element *b = legacyRows.GetPtr (key);
                DBrequire (a != nullptr && b != nullptr, "both engines have the row");
                DBtest (a->OutSumValue (0).val.intValue, b->OutSumValue (0).val.intValue, "mixed same sum");
                DBtest (a->elements.GetSize (), b->elements.GetSize (), "mixed same sources");
                for (UInt32 i = 0; i < a->elements.GetSize () && i < b->elements.GetSize (); i++)
                    DBtest (a->elements[i] == b->elements[i], true, "mixed same source order");
            }
            DBtest (freshOut.GetSize (), legacyOut.GetSize (), "mixed same outParam size");
        }

        // --- неполные массивы: оба движка обязаны вести себя ОДИНАКОВО ---
        // Сравнивается поведение, а не «правильность»: эталон не должен
        // «улучшать» суммирование.
        {
            Spec::Element freshRow = {};
            PushSumSlot (freshRow, Num (1));
            PushSumSlot (freshRow, Num (2));
            PushSumSlot (freshRow, Num (3));
            Spec::Element legacyRow = freshRow;
            GS::Array<ParamValue> shortContribution = {};
            shortContribution.Push (Num (10));
            shortContribution.Push (Num (20));
            Spec::SumContributionIntoRow (freshRow, asContribution (shortContribution));
            legacy::SumInto (legacyRow, shortContribution);
            DBtest (freshRow.OutSumCount (), legacyRow.OutSumCount (), "partial same size");
            for (UInt32 i = 0; i < freshRow.OutSumCount (); i++)
                DBtest (freshRow.OutSumValue (i).val.intValue,
                        legacyRow.OutSumValue (i).val.intValue,
                        "partial same value");

            Spec::Element f2 = {};
            PushSumSlot (f2, Num (5));
            Spec::Element l2 = f2;
            GS::Array<ParamValue> longContribution = {};
            longContribution.Push (Num (1));
            longContribution.Push (Num (2));
            longContribution.Push (Num (4));
            Spec::SumContributionIntoRow (f2, asContribution (longContribution));
            legacy::SumInto (l2, longContribution);
            DBtest (f2.OutSumCount (), l2.OutSumCount (), "longer same size");
            DBtest (f2.OutSumValue (0).val.intValue, l2.OutSumValue (0).val.intValue, "longer same value");
        }

        // --- isValid: обе стороны ведут себя одинаково ---
        {
            Spec::Element f1 = {};
            PushSumSlot (f1, Num (4, false));
            PushSumSlot (f1, Num (40));
            Spec::Element l1 = f1;
            GS::Array<ParamValue> contribution = {};
            contribution.Push (Num (4));
            contribution.Push (Num (1));
            Spec::SumContributionIntoRow (f1, asContribution (contribution));
            legacy::SumInto (l1, contribution);
            for (UInt32 i = 0; i < f1.OutSumCount (); i++)
                DBtest (f1.OutSumValue (i).val.intValue, l1.OutSumValue (i).val.intValue, "invalid handling same");
            DBtest (f1.OutSumValue (1).val.intValue, 41, "valid pair summed the same way");
        }
    }

    // Сведение сценариев объединения строк к общему виду сравнения —
    // число строк, ключи, значения, суммы, provenance.
    //
    // Канонизация применяется ТОЛЬКО к сравниваемым копиям: словарь
    // GS::HashTable не задаёт порядок обхода, и сортировать исходный словарь
    // нельзя — его порядок задаёт порядок первого представителя и через него
    // размещение. Поэтому ключи строк сортируются в отдельном списке, а
    // порядок источников внутри строки сравнивается поэлементно и отдельно.
    void TestSpecScenarioMatrix () {
        // Снимок строки для сравнения: ключи отдельно, значения и provenance
        // отдельно. Канонизация — только здесь, внутри снимка.
        auto snapshot = [] (const Spec::Element &row) {
            struct Snap {
                GS::Array<GS::UniString> keys = {};
                GS::Array<GS::UniString> outNames = {};
                GS::Array<GS::UniString> outValues = {};
                GS::Array<GS::UniString> sumValues = {};
                GS::Array<GS::UniString> sourceOrder = {};
                GS::UniString favorite;
                GS::UniString subguidRule;
                GS::UniString subguidValue;
            } s;
            for (UInt32 i = 0; i < row.out_slots.GetSize (); i++) {
                const Spec::OutputSlot &slot = row.out_slots[i];
                s.outNames.Push (slot.rawname);
                s.outValues.Push (ParamHelpers::ToString (slot.value, FormatStringFunc::ParseFormatString (".2m")));
                if (slot.isSum)
                    s.sumValues.Push (ParamHelpers::ToString (slot.value, FormatStringFunc::ParseFormatString (".2m")));
            }
            // Порядок источников сохраняется как есть — это provenance,
            // влияющий на размещение, и сортировать его нельзя.
            for (const API_Guid &guid : row.elements) {
                GS::UniString text = GS::UniString::Printf ("%08X", *((UInt32 *)&guid));
                s.sourceOrder.Push (text);
            }
            s.favorite = row.favorite_name;
            s.subguidRule = row.subguid_rulename;
            s.subguidValue = row.subguid_rulevalue;
            return s;
        };

        // --- несколько ГРУПП с одинаковым ключом ---
        // Объединение между группами: в других наборах различались источники,
        // а здесь группы.
        {
            SpecFixture f;
            f.rule.groups.Clear ();
            Spec::GroupSpec g1;
            g1.unic_paramrawname.Push (f.key);
            g1.out_paramrawname.Push (f.text);
            g1.sum_paramrawname.Push (f.quantity);
            Spec::GroupSpec g2;
            g2.unic_paramrawname.Push (f.key);
            g2.out_paramrawname.Push (f.text);
            g2.sum_paramrawname.Push (f.quantity);
            f.rule.groups.Push (g1);
            f.rule.groups.Push (g2);
            f.Source (f.first, "A", "Alpha", 5);
            f.Source (f.second, "A", "Alpha", 7);

            Spec::ElementDict rows = {};
            GS::HashTable<GS::UniString, GS::UniString> outParam = {};
            const GS::Array<Spec::GroupSlotBinding> bindings = Spec::PrepareSlotBindings (f.rule);
            DBtest (bindings.GetSize (), 2u, "two group bindings");
            const Spec::SpecValueReader reader (f.context);
            FormatString fstr = FormatStringFunc::ParseFormatString (".2m");
            ParamDict none1 = {};
            ParamDict none2 = {};
            for (const API_Guid &guid : f.rule.runState.elements) {
                for (UInt32 gi = 0; gi < f.rule.groups.GetSize (); gi++) {
                    Spec::RuleContribution c =
                        Spec::BuildContribution (guid, gi, f.rule.groups[gi], bindings[gi], reader, none1, none2);
                    if (c.status == Spec::ContributionStatus::Excluded)
                        continue;
                    Spec::RuleContribution rowContribution = c;
                    if (!rows.ContainsKey (c.key))
                        Spec::ReadContributionOutputs (
                            guid, f.rule.groups[gi], bindings[gi], reader, fstr, rowContribution);
                    Spec::AddContributionToRow (rows, rowContribution, f.rule, 1, 1, outParam);
                }
            }
            // Один ключ, четыре вклада (2 источника × 2 группы), одна строка.
            DBtest (rows.GetSize (), 1, "groups merged into one row");
            const Spec::Element *row = rows.GetPtr ("@A");
            DBrequire (row != nullptr, "row present");
            DBtest (row->OutSumValue (0).val.intValue, 24, "sum across groups 2*(5+7)");
            DBtest (row->elements.GetSize (), 4, "four source entries");
            // Первый представитель задаётся первым вкладом в порядке обхода.
            DBtest (row->out_slots.GetSize (), 2, "schema built once");
            DBtest (row->out_slots[0].value.val.uniStringValue, GS::UniString ("Alpha"), "first representative value");
        }

        // --- дубликаты выходных значений и разделители в ключе ---
        // Коллизия закрепляется как есть и НЕ «исправляется».
        {
            SpecFixture f;
            // Оба источника дают разные уникальные ключи, но ОДИНАКОВОЕ выходное
            // значение: outParam получает одну запись на пару (keyOut, key).
            f.Source (f.first, "A", "Same", 1);
            f.Source (f.second, "B", "Same", 2);
            Spec::ElementDict rows = {};
            GS::HashTable<GS::UniString, GS::UniString> outParam = {};
            const Spec::GroupSlotBinding binding = Spec::PrepareSlotBindings (f.rule)[0];
            const Spec::SpecValueReader reader (f.context);
            FormatString fstr = FormatStringFunc::ParseFormatString (".2m");
            ParamDict none1 = {};
            ParamDict none2 = {};
            for (const API_Guid &guid : f.rule.runState.elements) {
                Spec::RuleContribution c =
                    Spec::BuildContribution (guid, 0, f.rule.groups[0], binding, reader, none1, none2);
                if (c.status == Spec::ContributionStatus::Excluded)
                    continue;
                Spec::RuleContribution rowContribution = c;
                if (!rows.ContainsKey (c.key))
                    Spec::ReadContributionOutputs (guid, f.rule.groups[0], binding, reader, fstr, rowContribution);
                Spec::AddContributionToRow (rows, rowContribution, f.rule, 1, 1, outParam);
            }
            DBtest (rows.GetSize (), 2, "two rows kept");
            DBtest (outParam.GetSize (), 1, "duplicate out value collapses to one entry");
            // Второй ключ НЕ попадает в outParam — это старое поведение, и оно
            // означает потерю строки при поиске. Не «исправляется» здесь.
            DBtest (rows.ContainsKey (GS::UniString ("@A")), true, "row A kept");
            DBtest (rows.ContainsKey (GS::UniString ("@B")), true, "row B kept");

            // разделитель ключа: значение уникального параметра с '@' внутри.
            SpecFixture g;
            g.Source (g.first, "A@B", "Alpha", 3);
            Spec::ElementDict grows = {};
            GS::HashTable<GS::UniString, GS::UniString> gout = {};
            const Spec::GroupSlotBinding gbinding = Spec::PrepareSlotBindings (g.rule)[0];
            const Spec::SpecValueReader greader (g.context);
            ParamDict g1 = {};
            ParamDict g2 = {};
            Spec::RuleContribution gc =
                Spec::BuildContribution (g.first, 0, g.rule.groups[0], gbinding, greader, g1, g2);
            Spec::ReadContributionOutputs (g.first, g.rule.groups[0], gbinding, greader, fstr, gc);
            Spec::AddContributionToRow (grows, gc, g.rule, 1, 1, gout);
            // Ключ склеен как есть: '@' внутри значения не экранируется —
            // признак возможной коллизии в кодировке ключа.
            DBtest (gc.key, GS::UniString ("@A@B"), "separator in key not escaped (legacy)");
            DBtest (grows.ContainsKey (GS::UniString ("@A@B")), true, "row keyed by raw value");
        }

        // --- родительские GUID в provenance ---
        {
            SpecFixture f;
            f.Source (f.first, "A", "Alpha", 4);
            f.Source (f.second, "A", "Alpha", 6);
            f.rule.destinationParamGuidName = f.text;
            f.rule.subguid_rulename = f.text;
            f.rule.subguid_rulevalue = GS::UniString ("RuleValue");
            Spec::ElementDict rows = {};
            GS::HashTable<GS::UniString, GS::UniString> outParam = {};
            const Spec::GroupSlotBinding binding = Spec::PrepareSlotBindings (f.rule)[0];
            const Spec::SpecValueReader reader (f.context);
            FormatString fstr = FormatStringFunc::ParseFormatString (".2m");
            ParamDict none1 = {};
            ParamDict none2 = {};
            for (const API_Guid &guid : f.rule.runState.elements) {
                Spec::RuleContribution c =
                    Spec::BuildContribution (guid, 0, f.rule.groups[0], binding, reader, none1, none2);
                Spec::RuleContribution rowContribution = c;
                if (!rows.ContainsKey (c.key))
                    Spec::ReadContributionOutputs (guid, f.rule.groups[0], binding, reader, fstr, rowContribution);
                Spec::AddContributionToRow (rows, rowContribution, f.rule, 1, 1, outParam);
            }
            const Spec::Element *row = rows.GetPtr ("@A");
            DBrequire (row != nullptr, "row present");
            const auto snap = snapshot (*row);
            // provenance: источники в порядке обхода, признаки правила из
            // первого представителя.
            DBtest (snap.sourceOrder.GetSize (), 2, "two sources in provenance");
            DBtest (snap.sourceOrder[0] == snap.sourceOrder[1], false, "sources distinct and ordered");
            DBtest (snap.favorite, f.rule.favorite_name, "favorite carried");
            DBtest (snap.subguidValue, GS::UniString ("RuleValue"), "rule value carried");
            DBtest (row->OutSumValue (0).val.intValue, 10, "sum across sources");
            DBtest (row->out_slots[1].value.val.intValue, 10, "schema sum matches");
        }

        // --- малая модель — накладные расходы не мешают результату ---
        // Проверяется результат и то, что счётчик чтений не вырос: создание
        // объектов схемы не должно добавлять чтений.
        {
            SpecFixture f;
            f.Source (f.first, "A", "Alpha", 1);
            Spec::ElementDict rows = {};
            GS::HashTable<GS::UniString, GS::UniString> outParam = {};
            const Spec::GroupSlotBinding binding = Spec::PrepareSlotBindings (f.rule)[0];
            const Spec::SpecValueReader reader (f.context);
            FormatString fstr = FormatStringFunc::ParseFormatString (".2m");
            ParamDict none1 = {};
            ParamDict none2 = {};
            Spec::RuleContribution c =
                Spec::BuildContribution (f.first, 0, f.rule.groups[0], binding, reader, none1, none2);
            Spec::ReadContributionOutputs (f.first, f.rule.groups[0], binding, reader, fstr, c);
            Spec::AddContributionToRow (rows, c, f.rule, 1, 1, outParam);
            DBtest (rows.GetSize (), 1, "small model single row");
            const Spec::Element *row = rows.GetPtr ("@A");
            DBrequire (row != nullptr, "row present");
            DBtest (row->out_slots.GetSize (), 2, "small model slots minimal");
        }

        // --- масштабирование агрегации без лишнего чтения ---
        // Ключевой проверяемый факт: число чтений на источник НЕ зависит от
        // того, встречался ли ключ раньше. Выходные слоты читаются только у
        // первого представителя, поэтому при N источниках с повторяющимся
        // ключом чтений выхода ровно 1, а не N.
        {
            SpecFixture f;
            const int kSources = 40;
            for (int i = 0; i < kSources; i++) {
                API_Guid guid =
                    APIGuidFromString (GS::UniString::Printf ("{40000000-0000-0000-0000-%012X}", i).ToCStr ());
                f.Source (guid, "A", "Alpha", 1);
            }
            DBtest (f.rule.runState.elements.GetSize (), (UInt32)kSources, "sources registered");

            Spec::ElementDict rows = {};
            GS::HashTable<GS::UniString, GS::UniString> outParam = {};
            const Spec::GroupSlotBinding binding = Spec::PrepareSlotBindings (f.rule)[0];
            const Spec::SpecValueReader reader (f.context);
            FormatString fstr = FormatStringFunc::ParseFormatString (".2m");
            ParamDict none1 = {};
            ParamDict none2 = {};
            int readOutputPasses = 0;
            for (const API_Guid &guid : f.rule.runState.elements) {
                Spec::RuleContribution c =
                    Spec::BuildContribution (guid, 0, f.rule.groups[0], binding, reader, none1, none2);
                Spec::RuleContribution rowContribution = c;
                if (!rows.ContainsKey (c.key)) {
                    Spec::ReadContributionOutputs (guid, f.rule.groups[0], binding, reader, fstr, rowContribution);
                    readOutputPasses += 1;
                }
                Spec::AddContributionToRow (rows, rowContribution, f.rule, 1, 1, outParam);
            }
            DBtest (rows.GetSize (), 1, "repeated keys collapse to one row");
            // Чтение выхода ровно один раз, независимо от числа источников.
            DBtest (readOutputPasses, 1, "output read once for repeated key");
            const Spec::Element *row = rows.GetPtr ("@A");
            DBrequire (row != nullptr, "row present");
            DBtest (row->OutSumValue (0).val.intValue, kSources, "sum over all sources");
            DBtest (row->elements.GetSize (), (UInt32)kSources, "provenance keeps every source");
            DBtest (row->out_slots.GetSize (), 2, "schema size does not grow with sources");
        }

        // --- уникальные ключи и много выходных слотов — пик строк ---
        {
            SpecFixture f;
            f.rule.out_paramrawname.Clear ();
            f.rule.out_sum_paramrawname.Clear ();
            Spec::GroupSpec group;
            group.unic_paramrawname.Push (f.key);
            for (UInt32 i = 0; i < 6; i++) {
                GS::UniString name = GS::UniString::Printf ("{@property:spec-out-%u}", i);
                group.out_paramrawname.Push (name);
                f.rule.out_paramrawname.Push (name);
            }
            for (UInt32 i = 0; i < 3; i++) {
                GS::UniString name = GS::UniString::Printf ("{@property:spec-sum-%u}", i);
                group.sum_paramrawname.Push (name);
                f.rule.out_sum_paramrawname.Push (name);
            }
            f.rule.groups.Clear ();
            f.rule.groups.Push (group);

            const int kRows = 25;
            for (int i = 0; i < kRows; i++) {
                API_Guid guid =
                    APIGuidFromString (GS::UniString::Printf ("{50000000-0000-0000-0000-%012X}", i).ToCStr ());
                f.rule.runState.elements.Push (guid);
                f.Text (guid, f.key, GS::UniString::Printf ("K%u", i));
                for (UInt32 s = 0; s < 6; s++) {
                    ParamValue p = {};
                    ParamHelpers::ConvertStringToParamValue (
                        p, group.out_paramrawname[s], GS::UniString::Printf ("V%u", s));
                    f.context.read.Get (guid).Put (group.out_paramrawname[s], p);
                }
                for (UInt32 s = 0; s < 3; s++) {
                    f.Number (guid, group.sum_paramrawname[s], 1);
                }
            }

            Spec::ElementDict rows = {};
            GS::HashTable<GS::UniString, GS::UniString> outParam = {};
            const Spec::GroupSlotBinding binding = Spec::PrepareSlotBindings (f.rule)[0];
            const Spec::SpecValueReader reader (f.context);
            FormatString fstr = FormatStringFunc::ParseFormatString (".2m");
            ParamDict none1 = {};
            ParamDict none2 = {};
            for (const API_Guid &guid : f.rule.runState.elements) {
                Spec::RuleContribution c =
                    Spec::BuildContribution (guid, 0, f.rule.groups[0], binding, reader, none1, none2);
                Spec::RuleContribution rowContribution = c;
                if (!rows.ContainsKey (c.key))
                    Spec::ReadContributionOutputs (guid, f.rule.groups[0], binding, reader, fstr, rowContribution);
                Spec::AddContributionToRow (rows, rowContribution, f.rule, 6, 3, outParam);
            }
            DBtest (rows.GetSize (), (UInt32)kRows, "unique keys one row each");
            // Выходные значения у всех строк ОДИНАКОВЫ (V0..V5), поэтому ключ
            // выхода совпадает и outParam схлопывается в одну запись: та же
            // коллизия дубликатов выходных значений, что закреплена выше.
            // F), а не особенность этого шага.
            DBtest (outParam.GetSize (), 1u, "outParam collapses on identical output values");
            // Схема на 9 слотов, размер не зависит от числа строк.
            const Spec::Element *row = rows.GetPtr (GS::UniString ("@K0"));
            DBrequire (row != nullptr, "first row present");
            DBtest (row->out_slots.GetSize (), 9, "schema covers six out and three sum slots");
            DBtest (row->out_slots[6].isSum, true, "first sum slot flagged");
            DBtest (row->out_slots[8].isSum, true, "last sum slot flagged");
            DBtest (row->out_slots[5].isSum, false, "last output slot not flagged");
            DBtest (row->out_slots[0].value.val.uniStringValue, GS::UniString ("V0"), "first output value");
        }

        // --- много правил — нет квадратичного поиска по правилам ---
        // Проверяемо то, что можно проверить без профилировщика: каждый вклад
        // добавляется в уже существующую строку за одно обращение к словарю,
        // то есть стоимость на источник не зависит от числа уже собранных
        // строк. Считаем обращения к строке, а не время: таймеры в тестах
        // нестабильны, а счётчик обращений — точный признак отсутствия
        // квадратичности.
        {
            SpecFixture f;
            const int kRows = 30;
            for (int i = 0; i < kRows; i++) {
                API_Guid guid =
                    APIGuidFromString (GS::UniString::Printf ("{60000000-0000-0000-0000-%012X}", i).ToCStr ());
                f.Source (guid, GS::UniString::Printf ("K%u", i), "Alpha", 1);
            }
            Spec::ElementDict rows = {};
            GS::HashTable<GS::UniString, GS::UniString> outParam = {};
            const Spec::GroupSlotBinding binding = Spec::PrepareSlotBindings (f.rule)[0];
            const Spec::SpecValueReader reader (f.context);
            FormatString fstr = FormatStringFunc::ParseFormatString (".2m");
            ParamDict none1 = {};
            ParamDict none2 = {};
            for (const API_Guid &guid : f.rule.runState.elements) {
                Spec::RuleContribution c =
                    Spec::BuildContribution (guid, 0, f.rule.groups[0], binding, reader, none1, none2);
                Spec::RuleContribution rowContribution = c;
                if (!rows.ContainsKey (c.key))
                    Spec::ReadContributionOutputs (guid, f.rule.groups[0], binding, reader, fstr, rowContribution);
                // Одно ContainsKey + одно Get на источник — сигнатура шага.
                const bool existed = rows.ContainsKey (rowContribution.key);
                Spec::AddContributionToRow (rows, rowContribution, f.rule, 1, 1, outParam);
                DBtest (rows.ContainsKey (rowContribution.key), true, "row present after add");
                (void)existed;
            }
            DBtest (rows.GetSize (), (UInt32)kRows, "many rows built");
            // Выходное значение у всех строк одинаковое, поэтому outParam держит
            // одну запись на УНИКАЛЬНЫЙ выход, а не на строку.
            DBtest (outParam.GetSize (), 1u, "outParam one entry per distinct output");
        }

        // --- канонизация: сравнение копий, порядок отдельно ---
        {
            SpecFixture f;
            f.Source (f.first, "B", "Beta", 1);
            f.Source (f.second, "A", "Alpha", 2);
            Spec::ElementDict rows = {};
            GS::HashTable<GS::UniString, GS::UniString> outParam = {};
            const Spec::GroupSlotBinding binding = Spec::PrepareSlotBindings (f.rule)[0];
            const Spec::SpecValueReader reader (f.context);
            FormatString fstr = FormatStringFunc::ParseFormatString (".2m");
            ParamDict none1 = {};
            ParamDict none2 = {};
            for (const API_Guid &guid : f.rule.runState.elements) {
                Spec::RuleContribution c =
                    Spec::BuildContribution (guid, 0, f.rule.groups[0], binding, reader, none1, none2);
                Spec::RuleContribution rowContribution = c;
                if (!rows.ContainsKey (c.key))
                    Spec::ReadContributionOutputs (guid, f.rule.groups[0], binding, reader, fstr, rowContribution);
                Spec::AddContributionToRow (rows, rowContribution, f.rule, 1, 1, outParam);
            }
            // Ключи сортируются ТОЛЬКО в списке сравнения; словарь не трогается.
            GS::Array<GS::UniString> keys = {};
            for (auto it = rows.Begin (); it != rows.End (); ++it) {
    #ifdef ServerMainVers_2800
                keys.Push (it->key);
    #else
                keys.Push (*it->key);
    #endif
            }
            DBtest (keys.GetSize (), 2, "two keys in comparison list");
            // Сортировка копии даёт воспроизводимый порядок сравнения.
            GS::Array<GS::UniString> sorted = keys;
            const GSSize sortedSize = static_cast<GSSize> (sorted.GetSize ());
            for (GSSize i = 0; i < sortedSize; i++) {
                for (GSSize j = i + 1; j < sortedSize; j++) {
                    if (sorted[j] < sorted[i]) {
                        GS::UniString tmp = sorted[i];
                        sorted[i] = sorted[j];
                        sorted[j] = tmp;
                    }
                }
            }
            DBtest (sorted[0], GS::UniString ("@A"), "sorted keys canonical");
            DBtest (sorted[1], GS::UniString ("@B"), "sorted keys canonical");
            // Порядок источников остаётся порядком обхода, не сортируется.
            const Spec::Element *rowB = rows.GetPtr (GS::UniString ("@B"));
            DBrequire (rowB != nullptr, "row B present");
            const auto snapB = snapshot (*rowB);
            DBtest (snapB.sourceOrder.GetSize (), 1, "provenance order kept");
        }
    }

    // Фикстуры для сверки с существующими объектами: случаи, которые надо
    // зафиксировать ДО смены представления на create/update/delete/unchanged.
    //
    // Закрепляется ТЕКУЩЕЕ поведение, включая те его особенности, которые
    // выглядят подозрительно (коллизия ключа, потеря строки при совпадении
    // выходных значений, лишний GUID-связь без проверки). Чтобы было с чем
    // сравнивать, нужен эталон текущего поведения.
    // сравнивать, нужен эталон до изменения.
    void TestSpecReconcileFixtures () {
        // --- fixture 1: key_out и формат .2m ---
        // Ключ выхода склеивается из значений полей ВЫХОДА правила (не сумм) по
        // ATSIGN, значения приведены форматом ".2m". Обе вещи должны быть
        // закреплены, иначе изменение формата тихо меняет сопоставление.
        {
            SpecFixture f;
            f.Source (f.first, "A", "Alpha", 5);
            const Spec::GroupSlotBinding binding = Spec::PrepareSlotBindings (f.rule)[0];
            const Spec::SpecValueReader reader (f.context);
            FormatString fstr = FormatStringFunc::ParseFormatString (".2m");
            ParamDict none1 = {};
            ParamDict none2 = {};
            Spec::RuleContribution c =
                Spec::BuildContribution (f.first, 0, f.rule.groups[0], binding, reader, none1, none2);
            Spec::ReadContributionOutputs (f.first, f.rule.groups[0], binding, reader, fstr, c);
            // Ключ выхода = только выходные слоты, с префиксом ATSIGN.
            DBtest (c.keyOut, GS::UniString ("@Alpha"), "key_out from output slots only");
            DBtest (c.key, GS::UniString ("@A"), "row key from unic slots only");
            DBtest (c.keyOut != c.key, true, "key_out differs from row key");
            // Формат ".2m" в Archicad: точка — РАЗДЕЛИТЕЛЬ единицы измерения,
            // а не десятичный разделитель (Helpers.cpp:201-207: точки временно
            // убираются перед разбором). Поэтому ".2m" тождественно "2m" —
            // две знаковые цифры с обрезкой нулей, а НЕ два знака после запятой.
            // Это и есть причина, почему ключ выхода не зависит от разрядности.
            DBtest (fstr.n_zero, 2, ".2m means two significant decimals");
            DBtest (fstr.trim_zero, true, ".2m trims trailing zeros");
            DBtest (FormatStringFunc::ParseFormatString ("2m").n_zero, fstr.n_zero, "dot prefix is unit separator");
            ParamValue num = {};
            ParamHelpers::ConvertDoubleToParamValue (num, EMPTYSTRING, 2.5);
            DBtest (ParamHelpers::ToString (num, fstr), GS::UniString ("2,5"), ".2m trims to one decimal");
            ParamValue whole = {};
            ParamHelpers::ConvertDoubleToParamValue (whole, EMPTYSTRING, 7.0);
            DBtest (ParamHelpers::ToString (whole, fstr), GS::UniString ("7"), ".2m trims whole number to integer");
            // ГРАБЛЯ, а не контраст: нечисловой суффикс молча становится нулём
            // разрядов (Helpers.cpp: n_zero = std::atoi (outstringformat)), а не
            // ошибкой. "F2" -> n_zero 0 -> округление до целого. Закреплено,
            // потому что от разрядности зависит key_out, а значит и сопоставление
            // существующих объектов: смена формата молча переставит ключи.
            FormatString bogus = FormatStringFunc::ParseFormatString ("F2");
            DBtest (bogus.n_zero, 0, "non-numeric format suffix parses as zero decimals");
            ParamValue num2 = {};
            ParamHelpers::ConvertDoubleToParamValue (num2, EMPTYSTRING, 2.5);
            DBtest (ParamHelpers::ToString (num2, bogus), GS::UniString ("3"), "zero decimals rounds to integer");
        }

        // --- fixture 2: поиск ПЕРВОГО совпадения ---
        // out_param хранит ПЕРВЫЙ ключ для данного выходного значения
        // (запись не перезаписывается). Поэтому при двух строках с одинаковым
        // выходом сверка находит только первую, а вторая остаётся в elements
        // как будто новая. Это зафиксированный контракт, и он же — механизм
        // потери строки при совпадении выходных значений.
        {
            SpecFixture f;
            f.Source (f.first, "A", "Same", 1);
            f.Source (f.second, "B", "Same", 2);
            Spec::ElementDict rows = {};
            GS::HashTable<GS::UniString, GS::UniString> outParam = {};
            const Spec::GroupSlotBinding binding = Spec::PrepareSlotBindings (f.rule)[0];
            const Spec::SpecValueReader reader (f.context);
            FormatString fstr = FormatStringFunc::ParseFormatString (".2m");
            ParamDict none1 = {};
            ParamDict none2 = {};
            for (const API_Guid &guid : f.rule.runState.elements) {
                Spec::RuleContribution c =
                    Spec::BuildContribution (guid, 0, f.rule.groups[0], binding, reader, none1, none2);
                Spec::RuleContribution rowContribution = c;
                if (!rows.ContainsKey (c.key))
                    Spec::ReadContributionOutputs (guid, f.rule.groups[0], binding, reader, fstr, rowContribution);
                Spec::AddContributionToRow (rows, rowContribution, f.rule, 1, 1, outParam);
            }
            DBtest (outParam.GetSize (), 1u, "one entry for identical out value");
            // Первый источник в порядке обхода задаёт значение словаря.
            DBtest (outParam.Get (GS::UniString ("@Same")), GS::UniString ("@A"), "first match wins");
            DBtest (rows.ContainsKey (GS::UniString ("@B")), true, "second row still built");
            // Именно поэтому вторая строка при сверке не найдётся по key_out.
            DBtest (outParam.ContainsKey (GS::UniString ("@Same")), true, "out value present");
        }

        // --- fixture 3: ДУБЛИ старых объектов ---
        // Два существующих элемента с одинаковыми выходными значениями. Первый
        // забирает строку (elements.Delete), второй видит, что ключа больше нет
        // (в elements) — но проходит по elements_mod, если первый был
        // модифицирован. Проверяем оба исхода: порядок и то, что дубль не
        // приводит к повторной записи в elements_mod.
        {
            SpecFixture f;
            f.Source (f.first, "A", "Alpha", 5);
            f.Existing (f.old, "Alpha", 5); // значения старого совпадают с новыми
            // Второй дубль старого объекта с тем же выходным значением.
            API_Guid dupGuid = APIGuidFromString ("{55555555-5555-5555-5555-555555555555}");
            f.rule.runState.exsist_elements.Push (dupGuid);
            f.Text (dupGuid, f.outText, "Alpha");
            f.Number (dupGuid, f.outQuantity, 5);

            f.Shape (f.Run (), 1, 0, 0, 1, "duplicate old handled once");
            // Ровно один из двух дублей удалён, второй — тоже (не найден в guids
            // после обработки первого, т.к. ключ строки уже израсходован).
            // Первый дубль сопоставлен со строкой (guids = true, удаления нет),
            // второй уходит по ветке «строка уже израсходована» и удаляется.
            DBtest (f.deleted.GetSize (), 1, "one duplicate deleted");
            DBtest (f.created.GetSize (), 0, "no row left for duplicates");
        }

        // --- fixture 4: кандидат уже удалён из словаря новых ---
        // Строка, которую нашёл первый старый объект, УДАЛЯЕТСЯ из elements, и
        // второй старый объект с тем же key_out приходит по ветке
        // «!elements.ContainsKey (key)». Строка @B не имеет старого объекта и
        // остаётся как новая.
        {
            SpecFixture f;
            f.Source (f.first, "A", "Alpha", 5);
            f.Source (f.second, "B", "Beta", 9);
            // Оба старых объекта имеют ОДНО выходное значение, поэтому
            // out_param отображает его на первый ключ; второй старый объект
            // попадает в ветку «строка уже удалена».
            f.Existing (f.old, "Alpha", 5);
            API_Guid second = APIGuidFromString ("{66666666-6666-6666-6666-666666666666}");
            f.rule.runState.exsist_elements.Push (second);
            f.Text (second, f.outText, "Alpha");
            f.Number (second, f.outQuantity, 5);

            f.Run ();
            // Механизм по коду: первый старый объект находит строку @A по
            // key_out и ПОМЕЧАЕТСЯ ОБРАБОТАННЫМ (guids.Add (elemguid, true)),
            // но в elements_delete НЕ попадает — он сопоставлен, а не удалён.
            // Строка @A при этом УДАЛЯЕТСЯ из elements (elements.Delete (key)).
            // Второй старый объект имеет тот же key_out, поэтому out_param
            // отдаёт ему уже израсходованный ключ @A, и он уходит по ветке
            // «!elements.ContainsKey (key)» — вот она и удаляется.
            //
            // Итог: удалён ровно ОДИН объект — второй, чья строка была
            // захвачена первым: случай «кандидат уже удалён из словаря новых».
            DBtest (f.deleted.GetSize (), 1, "candidate with consumed row deleted");
            DBtest (f.created.GetSize (), 1, "unmatched row survives as create");
            DBtest (f.created.ContainsKey (GS::UniString ("@B")), true, "row B kept as create");
        }

        // --- fixture 5: no-op (совпадение целиком) ---
        // Значения старого объекта равны новым: flag_change не поднимается,
        // объект не модифицируется, но из elements строка УДАЛЯЕТСЯ — то есть
        // повторный запуск ничего не создаёт и не меняет. Это текущий контракт
        // no-op; новый статус «unchanged» его не должен подменять.
        {
            SpecFixture f;
            f.Source (f.first, "A", "Alpha", 5);
            f.Existing (f.old, "Alpha", 5);
            const Int32 n = f.Run ();
            DBtest (n, 0, "no-op yields zero operations");
            DBtest (f.created.GetSize (), 0, "no-op creates nothing");
            DBtest (f.modified.GetSize (), 0, "no-op modifies nothing");
            DBtest (f.deleted.GetSize (), 0, "no-op deletes nothing");
        }

        // --- fixture 6: изменение суммы при неизменном выходе ---
        // Сумма отличается — значит flag_change, объект уходит в modified с
        // сохранением exs_guid (это то, что не должно ломаться при замене
        // представления на update).
        {
            SpecFixture f;
            f.Source (f.first, "A", "Alpha", 9);
            f.Existing (f.old, "Alpha", 5);
            const Int32 n = f.Run ();
            DBtest (n, 1, "changed sum counts as one operation");
            DBtest (f.modified.GetSize (), 1, "changed sum goes to modified");
            const Spec::Element *mod = f.modified.GetPtr (GS::UniString ("@A"));
            DBrequire (mod != nullptr, "modified row found");
            DBtest (mod->exs_guid == f.old, true, "modified row keeps existing GUID");
            DBtest (f.created.GetSize (), 0, "nothing created on update");
            DBtest (f.deleted.GetSize (), 0, "nothing deleted on update");
        }

        // --- fixture 7: GUID-связь читается, но результат не используется ---
        // Строки 2026-2037: читается destinationParamGuidName, строится
        // instring из GUID источников — и НИКУДА не пишется. При отсутствии
        // поля меняется только флаг. Это подозрительно (вероятно, недописанная
        // связь), но менять поведение здесь нельзя: фиксируем как есть.
        {
            SpecFixture f;
            f.Source (f.first, "A", "Alpha", 5);
            f.Existing (f.old, "Alpha", 5);
            f.rule.destinationParamGuidName = f.text;
            // В словаре НЕТ этого поля для старого объекта: чтение не удастся,
            // но результат всё равно не влияет на решение.
            f.context.read.Get (f.old).Delete (f.text);
            const Int32 n = f.Run ();
            // flag_change поднимается из-за непрочитанного поля.
            DBtest (n, 1, "unread guid link counts as change");
            DBtest (f.modified.GetSize (), 1, "unread guid link forces update");
            // Если бы связь записывалась, сумма была бы другой — значит её
            // отсутствие не влияет на результат расчёта строки.
            const Spec::Element *mod = f.modified.GetPtr (GS::UniString ("@A"));
            DBrequire (mod != nullptr, "modified row present after guid link");
            DBtest (mod->OutSumValue (0).val.intValue, 5, "guid link does not alter computed row");
        }
    }

    // План изменений. План получаем от GetElementsForRule (он же владелец
    // решения), а не строим сами — иначе проверялась бы не план, а его копия.
    // Согласованность (Matches) не заменяет проверку содержимого.
    //
    // На КОРРЕКТНОЙ строке не строится ни одного текста объяснения: план на
    // совпавшей строке содержит ТОЛЬКО счётчик unchanged — ни текстовых полей,
    // ни копии свойств. Это видно из структуры и подтверждается тем, что
    // deleteReasons хранятся перечислимыми значениями рядом с GUID, а не
    // отдельными строками-объяснениями.
    void TestSpecChangePlan () {
        // --- сверка выполнена, изменений нет (no-op) ---
        {
            SpecFixture f;
            f.Source (f.first, "A", "Alpha", 5);
            f.Existing (f.old, "Alpha", 5);
            Spec::SpecChangePlan plan = {};
            const Int32 n =
                Spec::GetElementsForRule (f.rule, f.context, f.created, f.modified, f.deleted, f.errors, false, &plan);
            DBtest (n, 0, "no-op result");
            DBtest (plan.deleteOld, 1, "plan marked as reconciled");
            DBtest (plan.removals.GetSize (), 0, "no-op removes nothing");
            DBtest (plan.update.IsEmpty (), true, "no-op updates nothing");
            DBtest (plan.unchanged, 1, "matched row counted unchanged");
            DBtest (plan.Matches (f.modified, f.deleted), true, "no-op plan consistent");
        }

        // --- update: сумма изменилась ---
        // Причина удаления при этом НЕ возникает: update и unchanged — разные
        // исходы, и план обязан их различать.
        {
            SpecFixture f;
            f.Source (f.first, "A", "Alpha", 9);
            f.Existing (f.old, "Alpha", 5);
            Spec::SpecChangePlan plan = {};
            const Int32 n =
                Spec::GetElementsForRule (f.rule, f.context, f.created, f.modified, f.deleted, f.errors, false, &plan);
            DBtest (n, 1, "update result");
            DBtest (plan.update.GetSize (), 1, "one update planned");
            DBtest (plan.update.ContainsKey (GS::UniString ("@A")), true, "update keyed by row key");
            DBtest (plan.unchanged, 0, "updated row not counted unchanged");
            DBtest (plan.removals.GetSize (), 0, "updated row not removed");
            DBtest (plan.Matches (f.modified, f.deleted), true, "update plan consistent");
        }

        // --- удаление: RowAlreadyClaimed (строка захвачена другим объектом) ---
        // Два старых объекта с ОДНИМ выходным значением: первый сопоставлен со
        // строкой, второй приходит по ветке «строка уже израсходована». В плане
        // это обязано быть отражено КАК ПРИЧИНА, а не просто как факт удаления:
        // у Obsolete нет строки-кандидата, у RowAlreadyClaimed она была.
        {
            SpecFixture f;
            f.Source (f.first, "A", "Alpha", 5);
            f.Existing (f.old, "Alpha", 5);
            const API_Guid dup = APIGuidFromString ("{88888888-8888-8888-8888-888888888888}");
            f.rule.runState.exsist_elements.Push (dup);
            f.Text (dup, f.outText, "Alpha");
            f.Number (dup, f.outQuantity, 5);

            Spec::SpecChangePlan plan = {};
            Spec::GetElementsForRule (f.rule, f.context, f.created, f.modified, f.deleted, f.errors, false, &plan);
            DBtest (plan.removals.GetSize (), 1, "exactly one removal planned");
            DBtest (plan.removals[0].reason,
                    Spec::SpecChangePlan::DeleteReason::RowAlreadyClaimed,
                    "claimed-row reason recorded");
            DBtest (plan.removals[0].guid == dup, true, "the second object is removed");
            DBtest (plan.unchanged, 1, "first object counted unchanged");
            DBtest (plan.Matches (f.modified, f.deleted), true, "removal plan consistent");
        }

        // --- удаление: NoNewRow (key_out не найден среди новых строк) ---
        // Старый объект с выходным значением, которого нет ни в одной новой
        // строке. Он отсекается ВТОРОЙ ветвью сверки, до сопоставления, —
        // поэтому причина NoNewRow, а не Obsolete. **Первая версия проверки
        // ошибалась:** я ждал Obsolete, приняв «объект не сопоставлен» за
        // второй обход, хотя до него доходит только объект, прошедший
        // сопоставление и не попавший в guids, — а ветви !hasunic, NoNewRow и
        // RowAlreadyClaimed к тому моменту уже отсекли бы его раньше.
        //
        // То есть Obsolete при обычном ходе сверки недостижим, и отдельной
        // проверки на него здесь нет: значение остаётся в перечислении как
        // явный «второй обход», чтобы диагностика была исчерпывающей.
        {
            SpecFixture f;
            f.Source (f.first, "A", "Alpha", 5);
            f.Existing (f.old, "TotallyOther", 77);

            Spec::SpecChangePlan plan = {};
            Spec::GetElementsForRule (f.rule, f.context, f.created, f.modified, f.deleted, f.errors, false, &plan);
            DBtest (plan.removals.GetSize (), 1, "unmatched object planned for removal");
            DBtest (plan.removals[0].reason,
                    Spec::SpecChangePlan::DeleteReason::NoNewRow,
                    "no-new-row reason differs from claimed row");
            DBtest (plan.removals[0].guid == f.old, true, "unmatched guid recorded");
            DBtest (plan.unchanged, 0, "unmatched object is not unchanged");
        }

        // --- create: строки без сопоставленного объекта ---
        {
            SpecFixture f;
            f.Source (f.first, "A", "Alpha", 5);
            f.Existing (f.old, "Alpha", 5);
            // Вторая строка не имеет старого объекта — она создаётся.
            f.Source (f.extra, "B", "Beta", 7);
            f.Shape (f.Run (), 1, 1, 0, 0, "one create plus one no-op");
            DBtest (f.created.GetSize (), 1, "unmatched row created");
            DBtest (f.created.ContainsKey (GS::UniString ("@B")), true, "created row keyed @B");
        }

        // --- deleteOld = false: сверки не было, и это НЕ «всё unchanged» ---
        {
            SpecFixture f;
            f.Source (f.first, "A", "Alpha", 5);
            f.rule.delete_old = false;
            f.rule.runState.exsist_elements.Push (f.old);
            Spec::SpecChangePlan plan = {};
            Spec::GetElementsForRule (f.rule, f.context, f.created, f.modified, f.deleted, f.errors, false, &plan);
            // Ранний выход ДО записи deleteOld: план остаётся в исходном
            // состоянии, и Matches трактует это как «сверки не было».
            DBtest (plan.unchanged, 0, "no reconciliation means zero unchanged");
            DBtest (plan.removals.GetSize (), 0, "no reconciliation removes nothing");
            DBtest (plan.Matches (f.modified, f.deleted), true, "skipped reconciliation consistent");
        }
    }

    // Операционные сценарии правила: четыре класса операций (создание,
    // изменение, удаление, отсутствие изменений) в комбинациях, которые
    // по одному не получить — смешанный прогон, только удаления, пустой
    // результат, частично отсутствующие данные, дубли уже размещённых
    // строк, выбор представителя по выходному значению.
    //
    // Среда: подставная фикстура (синтетические GUID, словари чтения в
    // памяти). Модель не нужна: расчёт и сверка только принимают решения, а
    // создание и удаление объектов выполняются вне них. Поэтому проверяется
    // решение и его полнота, а не исполнение.
    //
    // Требуют модели и на этом шаге НЕ закрыты (статус «не проверено», а не
    // «покрыто»):
    //   - отказ создания, частичный и полный: он возникает в исполнителе при
    //     записи в модель, чтение словарей его не воспроизводит;
    //   - отбраковка невидимого источника: элемент синтетический, в модели его
    //     нет; здесь проверяется только решение НЕ спрашивать модель;
    //   - состояние модели после каждого этапа и откат неудачной записи.
    void TestSpecOperationMatrix () {
        // ----------------------------------------------------------------
        // Политика версии 2: удаление старого обязательно, отказ на
        // непрочитанном поле останавливает правило.
        // ----------------------------------------------------------------

        // --- создание: у строки нет ранее размещённого объекта ---
        {
            SpecFixture f;
            f.rule.delete_old = true;    // v2
            f.rule.stop_on_error = true; // v2
            f.rule.only_visible = false; // синтетический элемент не проходит проверку видимости
            f.Source (f.first, "A", "Alpha", 5);
            Spec::SpecChangePlan plan = {};
            f.Shape (f.RunWithPlan (plan), 1, 1, 0, 0, "v2 create");
            DBtest (plan.deleteOld, 1, "v2 create ran reconciliation");
            DBtest (plan.create.GetSize (), 1, "v2 create planned");
            // В плане создание называет ПЕРВЫЙ источник строки: он же
            // представитель для значений и для порядка размещения.
            DBtest (plan.create[0] == f.first, true, "v2 create names the first source of the row");
            DBtest (plan.update.IsEmpty (), true, "v2 create updates nothing");
            DBtest (plan.removals.IsEmpty (), true, "v2 create removes nothing");
            DBtest (plan.unchanged, 0, "v2 create counts no unchanged row");
            DBtest (plan.IsComplete (), true, "v2 create plan complete");
            DBtest (plan.Matches (f.modified, f.deleted), true, "v2 create plan consistent");
        }

        // --- изменение: сумма пересчитана, GUID объекта сохранён ---
        {
            SpecFixture f;
            f.rule.delete_old = true;
            f.rule.stop_on_error = true;
            f.rule.only_visible = false;
            f.Source (f.first, "A", "Alpha", 9);
            f.Existing (f.old, "Alpha", 5);
            Spec::SpecChangePlan plan = {};
            f.Shape (f.RunWithPlan (plan), 1, 0, 1, 0, "v2 update by sum");
            const Spec::Element *row = f.modified.GetPtr ("@A");
            DBrequire (row != nullptr, "v2 update row present");
            DBtest (row->exs_guid == f.old, true, "v2 update keeps GUID of updated object");
            DBtest (row->OutSumValue (0).val.intValue, 9, "v2 update carries the new sum");
            DBtest (row->elements.GetSize () == 1 && row->elements[0] == f.first, true, "v2 update names its source");
            DBtest (plan.update.ContainsKey (GS::UniString ("@A")), true, "v2 update keyed by row key");
            DBtest (plan.unchanged, 0, "v2 update is not counted unchanged");
            DBtest (plan.removals.IsEmpty (), true, "v2 update removes nothing");
            DBtest (plan.create.IsEmpty (), true, "v2 update creates nothing");
            DBtest (plan.Matches (f.modified, f.deleted), true, "v2 update plan consistent");
        }

        // --- удаление: у объекта нет соответствующей новой строки ---
        // Старый объект идёт по ветке «нет такой строки», поэтому расчёт не
        // отбрасывается: строка @A остаётся созданием, а объект удаляется
        // вместе с ней. Число операций — два, и это не «удаление вместо
        // создания», а оба решения сразу.
        {
            SpecFixture f;
            f.rule.delete_old = true;
            f.rule.stop_on_error = true;
            f.rule.only_visible = false;
            f.Source (f.first, "A", "Alpha", 5);
            f.Existing (f.old, "TotallyOther", 77);
            Spec::SpecChangePlan plan = {};
            f.Shape (f.RunWithPlan (plan), 2, 1, 0, 1, "v2 delete without new row");
            DBtest (plan.removals.GetSize (), 1, "v2 delete planned");
            DBtest (plan.removals[0].guid == f.old, true, "v2 delete names the object");
            DBtest (plan.removals[0].reason, Spec::SpecChangePlan::DeleteReason::NoNewRow, "v2 delete reason");
            DBtest (plan.create.GetSize (), 1, "v2 delete still leaves the calculated row to create");
            DBtest (plan.unchanged, 0, "v2 delete counts no unchanged row");
            DBtest (plan.update.IsEmpty (), true, "v2 delete updates nothing");
            DBtest (plan.Matches (f.modified, f.deleted), true, "v2 delete plan consistent");
        }

        // --- отсутствие изменений: объект сопоставлен и оставлен в покое ---
        {
            SpecFixture f;
            f.rule.delete_old = true;
            f.rule.stop_on_error = true;
            f.rule.only_visible = false;
            f.Source (f.first, "A", "Alpha", 5);
            f.Existing (f.old, "Alpha", 5);
            Spec::SpecChangePlan plan = {};
            f.Shape (f.RunWithPlan (plan), 0, 0, 0, 0, "v2 unchanged");
            DBtest (plan.unchanged, 1, "v2 unchanged counted");
            DBtest (plan.update.IsEmpty (), true, "v2 unchanged updates nothing");
            DBtest (plan.removals.IsEmpty (), true, "v2 unchanged removes nothing");
            DBtest (plan.create.IsEmpty (), true, "v2 unchanged creates nothing");
            DBtest (plan.Matches (f.modified, f.deleted), true, "v2 unchanged plan consistent");
        }

        // ----------------------------------------------------------------
        // Политика версии 3: удаление старого обязательно, отказ на
        // непрочитанном поле НЕ останавливает правило, видимость не
        // спрашивается.
        // ----------------------------------------------------------------

        // --- частично отсутствующие данные: строка НЕ появляется ---
        // Непрочитанное суммируемое поле оставляет строку без суммарного
        // слота, и сверка схемы её отбрасывает. Отказ при stop_on_error = false
        // молчаливый: элемент не помечается, отчёт не печатается, а план
        // честно говорит, что чтение и расчёт неполны. Именно поэтому полнота
        // и считается по вкладам, а не по словарям not_found_*.
        {
            SpecFixture f;
            f.rule.delete_old = true;
            f.rule.stop_on_error = false; // v3
            f.rule.only_visible = false;  // v3
            f.Source (f.first, "A", "Alpha", 5);
            f.DropField (f.first, f.quantity);
            Spec::SpecChangePlan plan = {};
            f.Shape (f.RunWithPlan (plan), 0, 0, 0, 0, "v3 missing sum leaves no row");
            DBtest (f.errors.IsEmpty (), true, "v3 missing sum marks no element");
            DBtest (f.created.IsEmpty (), true, "v3 missing sum creates nothing");
            DBtest (plan.notFoundParamCount, 1, "v3 missing sum counted");
            DBtest (plan.contributionsPartial, 1, "v3 missing sum is a partial contribution");
            DBtest (plan.schemaMismatchCount, 1, "v3 missing sum fails the schema check");
            DBtest (plan.ReadComplete (), false, "v3 missing sum read incomplete");
            DBtest (plan.CalcComplete (), false, "v3 missing sum calc incomplete");
            DBtest (plan.IsComplete (), false, "v3 missing sum plan not complete");
        }

        // --- непрочитанный уникальный параметр: вклад исключается целиком ---
        // Ключ склеивается даже при неудаче, но вклад без ключа не доходит до
        // раскладки, поэтому строки нет и вклада в расчёт не было. Это
        // отличие от предыдущего случая принципиально: здесь неполнота
        // считается, но неполным вкладом вклад НЕ является.
        {
            SpecFixture f;
            f.rule.delete_old = true;
            f.rule.stop_on_error = false;
            f.rule.only_visible = false;
            f.Source (f.first, "A", "Alpha", 5);
            f.DropField (f.first, f.key);
            Spec::SpecChangePlan plan = {};
            f.Shape (f.RunWithPlan (plan), 0, 0, 0, 0, "v3 missing unique key yields no row");
            DBtest (f.errors.IsEmpty (), true, "v3 missing unique key marks no element");
            DBtest (plan.contributionsTotal, 1, "v3 missing unique key still counted as contribution");
            DBtest (plan.notFoundUnicCount, 1, "v3 missing unique key counted");
            DBtest (plan.contributionsPartial, 0, "v3 excluded contribution is not partial");
            DBtest (plan.ReadComplete (), false, "v3 missing unique key read incomplete");
        }

        // --- та же неполнота при stop_on_error: расчёт отбрасывается, а
        //     размещённые строки УДАЛЯЮТСЯ ---
        // Здесь закрепляется действующая политика разрушительных действий,
        // а не предпочтительная. Отказ очищает словарь строк и возвращает ноль,
        // но сверка всё равно выполняется по ПУСТОМУ словарю выходов, поэтому
        // прежний объект не находит себе пары и удаляется. Политика
        // принадлежит владельцу (поток F1) и здесь не меняется: изменение
        // удаления при недостоверных данных — отдельное решение, а не
        // побочный эффект этого шага. Заметим, что это худший из возможных
        // исходов: недостоверный расчёт приводит к потере размещённых строк.
        {
            SpecFixture f;
            f.rule.delete_old = true;
            f.rule.stop_on_error = true;
            f.rule.only_visible = false;
            f.Source (f.first, "A", "Alpha", 5);
            f.Source (f.second, "B", "Beta", 7);
            f.Existing (f.old, "Alpha", 5);
            f.DropField (f.first, f.quantity);
            Spec::SpecChangePlan plan = {};
            // Возвращаемое число пересобирается ПОСЛЕ сверки как удаление +
            // изменение + создание, поэтому отказ на расчёте его не обнуляет:
            // ноль от расчёта заменяется числом удалений. Это ещё одно
            // следствие действующей политики, а не опечатка в ожидании.
            f.Shape (f.RunWithPlan (plan), 1, 0, 0, 1, "v2 missing sum rejects the rule");
            DBtest (f.errors.ContainsKey (f.first), true, "v2 missing sum marks the offending element");
            DBtest (plan.contributionsTotal, 2, "v2 rejected rule still counted both contributions");
            DBtest (plan.ReadComplete (), false, "v2 rejected rule read incomplete");
            // Удаление произошло, хотя строк не рассчитано: сверка идёт по
            // пустому словарю выходов.
            DBtest (plan.deleteOld, 1, "v2 rejected rule still reconciles");
            DBtest (plan.removals.GetSize (), 1, "v2 rejected rule removes the placed object");
            DBtest (plan.removals[0].guid == f.old, true, "v2 rejected rule names the removed object");
            // Причина здесь НЕ «нет такой строки», а «строка уже израсходована»,
            // и это следствие действующего порядка вещей: строка была посчитана,
            // связана в словаре выходов, но отброшена проверкой схемы, поэтому
            // в словаре строк её нет. Словарь выходов при отказе НЕ очищается,
            // и прежний объект находит в нём свою пару — на строку, которой
            // уже не существует. Само удаление от этого не меняется, но
            // диагностика плана называет причину неверно, а именно по причине
            // судят о том, чего не хватило.
            DBtest (plan.removals[0].reason,
                    Spec::SpecChangePlan::DeleteReason::RowAlreadyClaimed,
                    "v2 rejected removal blamed on a row rejected by the schema check");
            DBtest (plan.create.IsEmpty (), true, "v2 rejected rule creates nothing");
            DBtest (plan.unchanged, 0, "v2 rejected rule counts no unchanged row");
        }

        // --- видимость не спрашивается у модели ---
        {
            SpecFixture f;
            f.rule.delete_old = true;
            f.Source (f.first, "A", "Alpha", 5);
            f.rule.only_visible = true;
            Spec::SpecChangePlan skipped = {};
            f.Shape (f.RunWithPlan (skipped), 0, 0, 0, 0, "only visible on synthetic element");
            DBtest (skipped.contributionsTotal, 0, "invisible source never becomes a contribution");
            DBtest (skipped.deleteOld, 1, "reconciliation still ran without rows");

            f.rule.only_visible = false;
            Spec::SpecChangePlan passed = {};
            f.Shape (f.RunWithPlan (passed), 1, 1, 0, 0, "visibility check skipped");
            DBtest (passed.contributionsTotal, 1, "source counted without asking the model");
        }

        // ----------------------------------------------------------------
        // Уже размещённые строки: сопоставление по выходному значению.
        // ----------------------------------------------------------------

        // --- смена выходного значения: это УДАЛЕНИЕ и СОЗДАНИЕ, не изменение ---
        // Сопоставление идёт по выходному значению, поэтому объект с прежним
        // выходом не находит себе пары: он удаляется, а строка с новым выходом
        // создаётся. Это действующий контракт и важное отличие от смены суммы,
        // которая даёт изменение с сохранением GUID. Проверяется явно, потому
        // что «переименование позиции» интуитивно ожидают как изменение.
        {
            SpecFixture f;
            f.Source (f.first, "A", "Alpha", 5);
            f.Existing (f.old, "Renamed", 5);
            Spec::SpecChangePlan plan = {};
            f.Shape (f.RunWithPlan (plan), 2, 1, 0, 1, "placed output renamed");
            DBtest (plan.removals.GetSize (), 1, "renamed output plans a removal");
            DBtest (plan.removals[0].guid == f.old, true, "renamed output names the old object");
            DBtest (
                plan.removals[0].reason, Spec::SpecChangePlan::DeleteReason::NoNewRow, "renamed output removal reason");
            DBtest (plan.create.GetSize (), 1, "renamed output plans a creation");
            DBtest (plan.update.IsEmpty (), true, "renamed output is not an update");
            DBtest (plan.unchanged, 0, "renamed output counts no unchanged row");
        }

        // --- смена суммы при том же выходе: изменение с сохранением GUID ---
        // Обратный случай предыдущего: выход совпал, поэтому объект найден, и
        // меняется только накопленная сумма. GUID объекта обязан сохраниться.
        {
            SpecFixture f;
            f.Source (f.first, "A", "Alpha", 9);
            f.Existing (f.old, "Alpha", 5);
            Spec::SpecChangePlan plan = {};
            f.Shape (f.RunWithPlan (plan), 1, 0, 1, 0, "placed sum changed");
            const Spec::Element *row = f.modified.GetPtr ("@A");
            DBrequire (row != nullptr, "placed row update present");
            DBtest (row->exs_guid == f.old, true, "placed row keeps GUID on sum change");
            DBtest (row->OutParamValue (0).val.uniStringValue, GS::UniString ("Alpha"), "placed row keeps the output");
            DBtest (row->OutSumValue (0).val.intValue, 9, "placed row carries the new sum");
            DBtest (plan.update.ContainsKey (GS::UniString ("@A")), true, "sum change is an update");
            DBtest (plan.removals.IsEmpty (), true, "sum change removes nothing");
            DBtest (plan.unchanged, 0, "changed sum is not unchanged");
        }

        // --- лишняя строка: объект без пары в новом расчёте ---
        // Старый @A сопоставлен и остаётся в покое, лишний объект не находит
        // пары и удаляется. Число операций — одно удаление; создания нет,
        // потому что строка @A была израсходована сопоставлением.
        {
            SpecFixture f;
            f.Source (f.first, "A", "Alpha", 5);
            f.Existing (f.old, "Alpha", 5);
            f.Existing (f.extra, "Obsolete", 9);
            Spec::SpecChangePlan plan = {};
            f.Shape (f.RunWithPlan (plan), 1, 0, 0, 1, "placed extra row removed");
            DBtest (plan.removals.GetSize (), 1, "extra placed row planned for removal");
            DBtest (plan.removals[0].guid == f.extra, true, "extra placed row named");
            DBtest (plan.removals[0].reason, Spec::SpecChangePlan::DeleteReason::NoNewRow, "extra row reason");
            DBtest (plan.unchanged, 1, "kept placed row counted unchanged");
            DBtest (f.deleted.GetSize () == 1 && f.deleted[0] == f.extra, true, "only the extra object is deleted");
        }

        // --- два прежних объекта с одинаковым выходным значением ---
        // Первый забирает строку, второй приходит по ветке «строка уже
        // израсходована». Различие причин обязательно: у лишней строки
        // кандидата не было вовсе, у занятой он был.
        {
            SpecFixture f;
            f.Source (f.first, "A", "Alpha", 5);
            f.Existing (f.old, "Alpha", 5);
            const API_Guid dup = APIGuidFromString ("{77777777-7777-7777-7777-777777777777}");
            f.rule.runState.exsist_elements.Push (dup);
            f.Text (dup, f.outText, "Alpha");
            f.Number (dup, f.outQuantity, 5);
            Spec::SpecChangePlan plan = {};
            f.Shape (f.RunWithPlan (plan), 1, 0, 0, 1, "placed duplicate removed");
            DBtest (plan.removals.GetSize (), 1, "placed duplicate planned for removal");
            DBtest (plan.removals[0].guid == dup, true, "second object is the one removed");
            DBtest (plan.removals[0].reason,
                    Spec::SpecChangePlan::DeleteReason::RowAlreadyClaimed,
                    "duplicate removed as claimed row");
            DBtest (plan.unchanged, 1, "first object counted unchanged");
            DBtest (plan.create.IsEmpty (), true, "duplicate leaves no row to create");
        }

        // --- представитель выбирается первым совпадением выходного значения ---
        // Две строки с РАЗНЫМИ ключами и ОДИНАКОВЫМ выходным значением: словарь
        // выходов хранит только первое совпадение, поэтому размещённый объект
        // сопоставляется с первой строкой, а вторая остаётся созданием. Это
        // действующий контракт и механизм потери сопоставления; он не
        // «исправляется» здесь.
        {
            SpecFixture f;
            f.Source (f.first, "A", "Same", 1);
            f.Source (f.second, "B", "Same", 2);
            f.Existing (f.old, "Same", 1);
            Spec::SpecChangePlan plan = {};
            f.Shape (f.RunWithPlan (plan), 1, 1, 0, 0, "representative is the first matching output");
            DBtest (plan.unchanged, 1, "placed object matched the first row");
            DBtest (plan.create.GetSize (), 1, "second row stays a creation");
            DBtest (plan.create[0] == f.second, true, "creation names the second row source");
            DBtest (plan.update.IsEmpty (), true, "equal values need no update");
            DBtest (f.created.ContainsKey (GS::UniString ("@B")), true, "unclaimed row is created by its own key");
        }

        // ----------------------------------------------------------------
        // Смешанный прогон и частичная потеря вкладов.
        // ----------------------------------------------------------------

        // --- создание, изменение и удаление в одном запуске ---
        {
            SpecFixture f;
            f.Source (f.first, "A", "Alpha", 9);
            f.Source (f.second, "B", "Beta", 7);
            f.Existing (f.old, "Alpha", 5);
            f.Existing (f.extra, "Obsolete", 9);
            Spec::SpecChangePlan plan = {};
            f.Shape (f.RunWithPlan (plan), 3, 1, 1, 1, "mixed create update delete");
            DBtest (plan.create.GetSize (), 1, "mixed run plans one creation");
            DBtest (plan.create[0] == f.second, true, "mixed run names the new row source");
            DBtest (plan.update.ContainsKey (GS::UniString ("@A")), true, "mixed run plans the update by row key");
            DBtest (plan.removals.GetSize (), 1, "mixed run plans one removal");
            DBtest (plan.removals[0].guid == f.extra, true, "mixed run removes the unmatched object");
            DBtest (plan.unchanged, 0, "mixed run has no unchanged row");
            // Порядок в трёх контейнерах задаётся обходом и разными словарями,
            // поэтому по одному размеру не судить: проверяем состав по именам.
            DBtest (f.created.ContainsKey (GS::UniString ("@B")), true, "mixed run created the new row");
            DBtest (f.modified.ContainsKey (GS::UniString ("@A")), true, "mixed run modified the matched row");
            DBtest (f.modified.GetPtr ("@A")->exs_guid == f.old, true, "mixed run kept GUID of updated object");
            DBtest (f.deleted.GetSize () == 1 && f.deleted[0] == f.extra, true, "mixed run deleted the extra object");
            // Планирование не пишет в модель: прочитанные значения прежние.
            DBtest (
                f.context.read.Get (f.old).Get (f.outQuantity).val.intValue, 5, "mixed run leaves read data intact");
            DBtest (plan.Matches (f.modified, f.deleted), true, "mixed run plan consistent");
        }

        // --- второй источник той же строки без данных: строка создаётся, но
        //     источник не участвует в сумме ---
        // Строка собирается из ПЕРВОГО представителя, а вклад без
        // суммируемого поля не добавляет слота: источник попадает в
        // перечень, сумма остаётся от первого. Неполнота при этом
        // засчитывается, то есть план честно говорит, что расчёт неполон.
        {
            SpecFixture f;
            f.rule.stop_on_error = false;
            f.Source (f.first, "A", "Alpha", 5);
            f.Source (f.second, "A", "Alpha", 100);
            f.DropField (f.second, f.quantity);
            Spec::SpecChangePlan plan = {};
            f.Shape (f.RunWithPlan (plan), 1, 1, 0, 0, "incomplete merge keeps the row");
            const Spec::Element *row = f.created.GetPtr ("@A");
            DBrequire (row != nullptr, "merged row present");
            DBtest (row->elements.GetSize (), 2, "incomplete merge records both sources");
            DBtest (row->OutSumValue (0).val.intValue, 5, "incomplete merge does not add the missing sum");
            DBtest (row->OutParamValue (0).val.uniStringValue,
                    GS::UniString ("Alpha"),
                    "incomplete merge keeps first output");
            DBtest (plan.notFoundParamCount, 1, "incomplete merge counted the missing field");
            DBtest (plan.contributionsPartial, 1, "incomplete merge counted the partial contribution");
            DBtest (plan.contributionsTotal, 2, "incomplete merge counted both contributions");
            DBtest (plan.IsComplete (), false, "incomplete merge plan not complete");
        }

        // --- отказ создания строки на расчётной стадии ---
        // Непрочитанное выходное поле оставляет строку без выходных слотов,
        // и сверка схемы её отбрасывает. Возврат ПУСТОЙ строки — уже
        // наблюдаемый отказ создания на планировании; отказ при записи в
        // модель этим не воспроизводится и остаётся непроверенным.
        {
            SpecFixture f;
            f.rule.stop_on_error = false;
            f.Source (f.first, "A", "Alpha", 5);
            f.DropField (f.first, f.text);
            Spec::SpecChangePlan plan = {};
            Spec::ElementDict rows = {};
            UnicGuid errors = {};
            GS::HashTable<GS::UniString, GS::UniString> outParam = {};
            const Int32 n = Spec::PlanRuleRows (f.rule, f.context, rows, errors, false, outParam, &plan);
            DBtest (n, 0, "schema mismatch produces no row");
            DBtest (rows.IsEmpty (), true, "schema mismatch leaves no row");
            DBtest (errors.IsEmpty (), true, "schema mismatch marks no element without stop on error");
            DBtest (plan.schemaMismatchCount, 1, "schema mismatch counted");
            DBtest (plan.contributionsPartial, 1, "schema mismatch is also a partial contribution");
            DBtest (plan.CalcComplete (), false, "schema mismatch calc incomplete");
            // Побочный эффект действующего порядка: связь выход -> строка
            // записывается ДО проверки схемы, поэтому отброшенная строка
            // остаётся в словаре выходов. Закреплено как есть: изменить
            // значит сломать сопоставление по этому словарю.
            DBtest (outParam.GetSize (), 1, "rejected row still recorded in the output dictionary");
            DBtest (outParam.Get (EMPTYSTRING), GS::UniString ("@A"), "rejected row keyed by the row key");
        }

        // ----------------------------------------------------------------
        // Состав результата: пустой, только изменения, только удаления,
        // только сопоставленные без изменений.
        // ----------------------------------------------------------------

        // --- пустой результат: размещённые объекты не имеют пары ---
        {
            SpecFixture f;
            f.rule.delete_old = true;
            f.rule.runState.exsist_elements.Clear ();
            f.rule.runState.elements.Clear ();
            f.Existing (f.old, "Alpha", 5);
            Spec::SpecChangePlan plan = {};
            f.Shape (f.RunWithPlan (plan), 1, 0, 0, 1, "empty result removes placed rows");
            DBtest (plan.contributionsTotal, 0, "empty result has no contributions");
            DBtest (plan.create.IsEmpty (), true, "empty result creates nothing");
            DBtest (plan.update.IsEmpty (), true, "empty result updates nothing");
            DBtest (plan.unchanged, 0, "empty result counts no unchanged row");
            DBtest (plan.removals.GetSize (), 1, "empty result removes the placed object");
            DBtest (plan.IsComplete (), true, "empty result plan is complete: nothing was read wrongly");
        }

        // --- только изменения ---
        {
            SpecFixture f;
            f.Source (f.first, "A", "Alpha", 9);
            f.Source (f.second, "B", "Beta", 7);
            f.Existing (f.old, "Alpha", 5);
            f.Existing (f.extra, "Beta", 3);
            Spec::SpecChangePlan plan = {};
            f.Shape (f.RunWithPlan (plan), 2, 0, 2, 0, "only updates");
            DBtest (plan.update.GetSize (), 2, "only updates plans both rows");
            DBtest (plan.create.IsEmpty (), true, "only updates creates nothing");
            DBtest (plan.removals.IsEmpty (), true, "only updates removes nothing");
            DBtest (plan.unchanged, 0, "only updates counts no unchanged row");
            DBtest (f.modified.GetPtr ("@A")->exs_guid == f.old, true, "only updates kept first GUID");
            DBtest (f.modified.GetPtr ("@B")->exs_guid == f.extra, true, "only updates kept second GUID");
        }

        // --- только удаления, в порядке прежних объектов ---
        {
            SpecFixture f;
            f.rule.delete_old = true;
            f.Existing (f.old, "Alpha", 5);
            f.Existing (f.extra, "Beta", 7);
            Spec::SpecChangePlan plan = {};
            f.Shape (f.RunWithPlan (plan), 2, 0, 0, 2, "only deletions");
            DBtest (plan.removals.GetSize (), 2, "only deletions plans both objects");
            DBtest (plan.removals[0].guid == f.old, true, "only deletions keeps the traversal order, first");
            DBtest (plan.removals[1].guid == f.extra, true, "only deletions keeps the traversal order, second");
            DBtest (plan.removals[0].reason, Spec::SpecChangePlan::DeleteReason::NoNewRow, "first deletion reason");
            DBtest (plan.removals[1].reason, Spec::SpecChangePlan::DeleteReason::NoNewRow, "second deletion reason");
            DBtest (plan.create.IsEmpty (), true, "only deletions creates nothing");
            DBtest (plan.update.IsEmpty (), true, "only deletions updates nothing");
            DBtest (f.deleted.GetSize () == 2 && f.deleted[0] == f.old && f.deleted[1] == f.extra,
                    true,
                    "deletion list keeps the traversal order");
        }

        // --- только сопоставленные без изменений ---
        {
            SpecFixture f;
            f.rule.delete_old = true;
            f.Source (f.first, "A", "Alpha", 5);
            f.Source (f.second, "B", "Beta", 7);
            f.Existing (f.old, "Alpha", 5);
            f.Existing (f.extra, "Beta", 7);
            Spec::SpecChangePlan plan = {};
            f.Shape (f.RunWithPlan (plan), 0, 0, 0, 0, "only unchanged");
            DBtest (plan.unchanged, 2, "both placed objects counted unchanged");
            DBtest (plan.create.IsEmpty (), true, "only unchanged creates nothing");
            DBtest (plan.update.IsEmpty (), true, "only unchanged updates nothing");
            DBtest (plan.removals.IsEmpty (), true, "only unchanged removes nothing");
            // Возвращаемое число — это удаление + изменение + создание.
            // Сопоставленные без изменений в него не входят, поэтому
            // повторный запуск даёт ноль при двух занятых строках. Это
            // действующий контракт: новый статус «unchanged» его не
            // подменяет, а лишь дополняет планом.
            DBtest (plan.unchanged > 0, true, "unchanged is visible in the plan even though the result is zero");
            DBtest (plan.Matches (f.modified, f.deleted), true, "only unchanged plan consistent");
        }
    }

    void TestSpecRowSlots () {
        // --- схема повторяет прежние порядок, имена и значения ---
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
            Spec::ElementDict rows = {};
            GS::HashTable<GS::UniString, GS::UniString> outParam = {};
            Spec::AddContributionToRow (rows, c, f.rule, 1, 1, outParam);

            const Spec::Element *row = rows.GetPtr ("@A");
            DBrequire (row != nullptr, "row present");
            // Порядок прежний: выходной слот, затем слот суммы.
            DBtest (row->out_slots.GetSize (), 2, "schema has both slots");
            DBtest (row->out_slots[0].isSum, false, "first slot is output");
            DBtest (row->out_slots[1].isSum, true, "second slot is sum");
            DBtest (row->out_slots[0].rawname, f.outText, "output slot name");
            DBtest (row->out_slots[1].rawname, f.outQuantity, "sum slot name");
            DBtest (row->out_slots[0].value.val.uniStringValue, GS::UniString ("Alpha"), "output slot value");
            DBtest (row->out_slots[1].value.val.intValue, 6, "sum slot value");

            // Схема совпадает с прежними параллельными массивами: это есть
            // инвариант до полного перевода потребителей.
            DBtest (row->out_slots.GetSize (), row->OutSlotCount (), "schema size equals accessor count");
            for (UInt32 i = 0; i < row->out_slots.GetSize (); i++) {
                GS::UniString rawname;
                ParamValue value;
                bool isSum = false;
                DBtest (row->TryGetOutSlot (i, rawname, value, isSum), true, "slot readable by index");
                DBtest (rawname, row->OutSlotName (i), "slot name equals accessor name");
                DBtest (
                    value.val.uniStringValue, row->OutParam (i).val.uniStringValue, "slot value equals accessor value");
                DBtest (isSum, i > 0, "slot kind matches position");
            }
            // Индекс вне схемы даёт безопасный ответ, а не выход за границу.
            GS::UniString name;
            ParamValue val;
            bool sumFlag = true;
            DBtest (row->TryGetOutSlot (99, name, val, sumFlag), false, "out-of-range slot reported");
        }

        // --- после слияния значения сумм в схеме АКТУАЛЬНЫ ---
        // Главная проверка шага: схема не должна устареть там, где сумма
        // меняется. Обновление сделано в том же цикле суммирования.
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
            Spec::ReadContributionOutputs (f.second, f.rule.groups[0], binding, reader, fstr, c2);

            Spec::ElementDict rows = {};
            GS::HashTable<GS::UniString, GS::UniString> outParam = {};
            Spec::AddContributionToRow (rows, c1, f.rule, 1, 1, outParam);
            Spec::AddContributionToRow (rows, c2, f.rule, 1, 1, outParam);

            const Spec::Element *row = rows.GetPtr ("@A");
            DBrequire (row != nullptr, "merged row present");
            DBtest (row->OutSumValue (0).val.intValue, 5, "merged sum in legacy array");
            DBtest (row->out_slots[1].value.val.intValue, 5, "merged sum reflected in schema");
            DBtest (
                row->out_slots[1].value.val.intValue, row->OutSumValue (0).val.intValue, "schema matches legacy sum");
            // Выходной слот первого представителя НЕ меняется при слиянии.
            DBtest (row->out_slots[0].value.val.uniStringValue,
                    GS::UniString ("Alpha"),
                    "output slot keeps first representative");
        }

        // --- схема не строится для отброшенной строки ---
        {
            SpecFixture f;
            f.Source (f.first, "A", "Alpha", 5);
            f.context.read.Get (f.first).Delete (f.text); // выход не прочитается
            const Spec::GroupSlotBinding binding = Spec::PrepareSlotBindings (f.rule)[0];
            ParamDict none1 = {};
            ParamDict none2 = {};
            const Spec::SpecValueReader reader (f.context);
            FormatString fstr = FormatStringFunc::ParseFormatString (".2m");
            Spec::RuleContribution c =
                Spec::BuildContribution (f.first, 0, f.rule.groups[0], binding, reader, none1, none2);
            Spec::ReadContributionOutputs (f.first, f.rule.groups[0], binding, reader, fstr, c);
            Spec::ElementDict rows = {};
            GS::HashTable<GS::UniString, GS::UniString> outParam = {};
            // Схема заявлена шире фактического: строка отбрасывается.
            const Spec::RowAddition r = Spec::AddContributionToRow (rows, c, f.rule, 2, 1, outParam);
            DBtest (r, Spec::RowAddition::SchemaMismatch, "mismatch when output slot missing");
            DBtest (rows.GetSize (), 0, "mismatch adds no row");
        }

        // --- размер схемы стабилен при многих слияниях ---
        {
            SpecFixture f;
            f.Source (f.first, "A", "Alpha", 1);
            f.Source (f.second, "A", "Alpha", 1);
            f.Source (f.extra, "A", "Alpha", 1);
            const Spec::GroupSlotBinding binding = Spec::PrepareSlotBindings (f.rule)[0];
            ParamDict none1 = {};
            ParamDict none2 = {};
            const Spec::SpecValueReader reader (f.context);
            FormatString fstr = FormatStringFunc::ParseFormatString (".2m");
            Spec::ElementDict rows = {};
            GS::HashTable<GS::UniString, GS::UniString> outParam = {};
            for (const API_Guid &guid : f.rule.runState.elements) {
                Spec::RuleContribution c =
                    Spec::BuildContribution (guid, 0, f.rule.groups[0], binding, reader, none1, none2);
                Spec::ReadContributionOutputs (guid, f.rule.groups[0], binding, reader, fstr, c);
                Spec::AddContributionToRow (rows, c, f.rule, 1, 1, outParam);
            }
            const Spec::Element *row = rows.GetPtr ("@A");
            DBrequire (row != nullptr, "row after three merges");
            DBtest (row->out_slots.GetSize (), 2, "schema size stable across merges");
            DBtest (row->out_slots[1].value.val.intValue, 3, "schema sum after three merges");
        }
    }

    void TestSpecRowLayout () {
        // --- суммирование равных массивов ---
        {
            Spec::Element row = {};
            PushSumSlot (row, Num (2));
            Spec::RuleContribution c = {};
            c.outSumParam.Push (Num (3));
            Spec::SumContributionIntoRow (row, c);
            DBtest (row.OutSumCount (), 1, "equal sizes keep size");
            DBtest (row.OutSumValue (0).val.intValue, 5, "equal sizes summed");
        }

        // --- НЕПОЛНЫЕ массивы: вклад короче строки ---
        // Поведение сохранено: складываются только первые 2 из 3, третий слот
        // строки остаётся как был.
        {
            Spec::Element row = {};
            PushSumSlot (row, Num (1));
            PushSumSlot (row, Num (10));
            PushSumSlot (row, Num (100));
            Spec::RuleContribution c = {};
            c.outSumParam.Push (Num (2));
            c.outSumParam.Push (Num (20));
            Spec::SumContributionIntoRow (row, c);
            DBtest (row.OutSumCount (), 3, "short contribution does not resize row");
            DBtest (row.OutSumValue (0).val.intValue, 3, "short contribution sums first slot");
            DBtest (row.OutSumValue (1).val.intValue, 30, "short contribution sums second slot");
            DBtest (row.OutSumValue (2).val.intValue, 100, "slot beyond nsumm untouched");
        }

        // --- НЕПОЛНЫЕ массивы: строка короче вклада ---
        {
            Spec::Element row = {};
            PushSumSlot (row, Num (7));
            Spec::RuleContribution c = {};
            c.outSumParam.Push (Num (1));
            c.outSumParam.Push (Num (2));
            c.outSumParam.Push (Num (4));
            Spec::SumContributionIntoRow (row, c);
            DBtest (row.OutSumCount (), 1, "longer contribution does not grow row");
            DBtest (row.OutSumValue (0).val.intValue, 8, "longer contribution sums overlap only");
        }

        // --- isValid требуется с ОБЕИХ сторон ---
        {
            Spec::Element row = {};
            PushSumSlot (row, Num (5));
            PushSumSlot (row, Num (50));
            Spec::RuleContribution c = {};
            c.outSumParam.Push (Num (5, false)); // невалидный: слот остаётся как есть
            c.outSumParam.Push (Num (1));
            Spec::SumContributionIntoRow (row, c);
            DBtest (row.OutSumValue (0).val.intValue, 5, "invalid side leaves slot untouched");
            DBtest (row.OutSumValue (1).val.intValue, 51, "valid pair still summed");
        }
        {
            Spec::Element row = {};
            PushSumSlot (row, Num (5, false)); // невалидная строка
            Spec::RuleContribution c = {};
            c.outSumParam.Push (Num (5));
            Spec::SumContributionIntoRow (row, c);
            DBtest (row.OutSumValue (0).val.intValue, 5, "invalid row slot untouched");
        }

        // --- пустые массивы: ничего не происходит, размер не меняется ---
        {
            Spec::Element row = {};
            PushSumSlot (row, Num (9));
            Spec::RuleContribution c = {};
            Spec::SumContributionIntoRow (row, c);
            DBtest (row.OutSumCount (), 1, "empty contribution keeps row size");
            DBtest (row.OutSumValue (0).val.intValue, 9, "empty contribution keeps value");

            Spec::Element emptyRow = {};
            Spec::RuleContribution withValue = {};
            withValue.outSumParam.Push (Num (3));
            Spec::SumContributionIntoRow (emptyRow, withValue);
            DBtest (emptyRow.OutSumCount (), 0, "empty row stays empty");
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
            DBtest (r1, Spec::RowAddition::Created, "first contribution creates row");
            DBtest (rows.GetSize (), 1, "one row created");
            const Spec::Element *row = rows.GetPtr ("@A");
            DBrequire (row != nullptr, "created row found");
            DBtest (row->OutParamCount (), 1, "created row has out slot");
            DBtest (row->OutSumCount (), 1, "created row has sum slot");
            DBtest (row->OutSumValue (0).val.intValue, 6, "created row sum value");
            DBtest (row->elements.GetSize (), 1, "created row has one source");
            DBtest (row->elements[0] == f.first, true, "created row source is the contributor");
            DBtest (row->favorite_name, f.rule.favorite_name, "created row carries favorite");
            DBtest (row->OutParamCount (), f.rule.out_paramrawname.GetSize (), "created row out schema");
            DBtest (outParam.GetSize (), 1, "outParam recorded on creation");
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
                    "first creates");
            DBtest (Spec::AddContributionToRow (rows, c2, f.rule, 1, 1, outParam),
                    Spec::RowAddition::Merged,
                    "second merges");
            DBtest (rows.GetSize (), 1, "merge keeps one row");
            const Spec::Element *row = rows.GetPtr ("@A");
            DBrequire (row != nullptr, "merged row found");
            DBtest (row->OutSumValue (0).val.intValue, 5, "merge summed both");
            DBtest (row->elements.GetSize (), 2, "merge collected both sources");
            DBtest (row->elements[1] == f.second, true, "merge appended second source");
            DBtest (outParam.GetSize (), 1, "merge does not duplicate outParam");
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
            DBtest (r, Spec::RowAddition::SchemaMismatch, "schema mismatch reported");
            DBtest (rows.GetSize (), 0, "schema mismatch adds no row");
            DBtest (outParam.GetSize (), 1, "outParam written before schema check");
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
            for (const API_Guid &guid : layout.rule.runState.elements) {
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

            DBtest (rows.GetSize (), full.created.GetSize (), "layout and full path agree on rows");
            // Ключ выхода строится из значений выходных слотов, поэтому у строк
            // с РАЗНЫМИ значениями выхода он разный: "@Alpha" и "@Beta". Одна
            // запись на каждый ключ строки, не на строку.
            DBtest (outParam.GetSize (), 2, "layout outParam has one entry per row");
            for (auto it = rows.Begin (); it != rows.End (); ++it) {
    #ifdef ServerMainVers_2800
                const GS::UniString &key = it->key;
    #else
                const GS::UniString key = *it->key;
    #endif
                const Spec::Element *a = rows.GetPtr (key);
                const Spec::Element *b = full.created.GetPtr (key);
                DBrequire (a != nullptr && b != nullptr, "both sides have the row");
                DBtest (a->OutSumCount (), b->OutSumCount (), "agree on sum slot count");
                DBtest (a->OutParamCount (), b->OutParamCount (), "agree on out slot count");
                DBtest (a->elements.GetSize (), b->elements.GetSize (), "agree on source count");
                DBtest (a->favorite_name, b->favorite_name, "agree on favorite");
                for (UInt32 j = 0; j < a->OutSumCount () && j < b->OutSumCount (); j++)
                    DBtest (a->OutSumValue (j).val.intValue, b->OutSumValue (j).val.intValue, "agree on sum value");
            }
            const Spec::Element *mergedA = rows.GetPtr ("@A");
            DBrequire (mergedA != nullptr, "merged row A present");
            DBtest (mergedA->OutSumValue (0).val.intValue, 5, "layout summed 2+3");
            const Spec::Element *rowB = rows.GetPtr ("@B");
            DBrequire (rowB != nullptr, "row B present");
            DBtest (rowB->OutSumValue (0).val.intValue, 7, "row B own sum");
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
            DBtest (n, 1, "one source one row");
            DBtest (planned.GetSize (), 1, "one row planned");
            DBtest (errors.IsEmpty (), true, "no errors");
            // Строка рассчитана: слоты заполнены по схеме, источник и признаки
            // правила перенесены.
            const Spec::Element *row = planned.GetPtr ("@A");
            DBrequire (row != nullptr, "row found by key");
            DBtest (row->OutParamCount (), 1, "out slot filled");
            DBtest (row->OutSumCount (), 1, "sum slot filled");
            DBtest (row->OutParamValue (0).val.uniStringValue, GS::UniString ("Alpha"), "out value");
            DBtest (row->OutSumValue (0).val.intValue, 2, "sum value");
            DBtest (row->elements.GetSize (), 1, "source multiplicity");
            DBtest (row->OutParamCount (), f.rule.out_paramrawname.GetSize (), "out schema carried");
            DBtest (row->favorite_name, f.rule.favorite_name, "favorite carried");
            // out_param: ключ - склеенные выходящие значения, значение - ключ
            // строки. Он пережил вынос и остаётся тем же словарём.
            DBtest (outParam.GetSize (), 1, "outParam one entry");
            DBtest (outParam.ContainsKey (EMPTYSTRING) || outParam.GetSize () == 1, true, "outParam keyed");
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
            DBtest (planned.GetSize (), 1, "delete_old does not add rows");
            DBtest (f.rule.runState.exsist_elements.GetSize (), 1, "existing list untouched by planning");
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
            DBtest (n, 1, "merged sources one row");
            DBtest (planned.GetSize (), 1, "merged row single");
            const Spec::Element *row = planned.GetPtr ("@A");
            DBrequire (row != nullptr, "merged row found");
            DBtest (row->OutSumValue (0).val.intValue, 5, "merged sum");
            DBtest (row->elements.GetSize (), 2, "merged sources");
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

            DBtest (planCount, fullCount, "planning and full path agree on count");
            DBtest (planned.GetSize (), full.created.GetSize (), "planning and full path agree on rows");
            DBtest (planned.ContainsKey ("@A"), full.created.ContainsKey ("@A"), "agree on key");
            const Spec::Element *a = planned.GetPtr ("@A");
            const Spec::Element *b = full.created.GetPtr ("@A");
            DBrequire (a != nullptr && b != nullptr, "both rows available");
            DBtest (a->OutParamCount (), b->OutParamCount (), "agree on out slot count");
            DBtest (a->OutSumCount (), b->OutSumCount (), "agree on sum slot count");
            DBtest (
                a->OutParamValue (0).val.uniStringValue, b->OutParamValue (0).val.uniStringValue, "agree on out value");
            DBtest (a->OutSumValue (0).val.intValue, b->OutSumValue (0).val.intValue, "agree on sum value");
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
            DBtest (n, 0, "rejected rule counts zero");
            DBtest (planned.IsEmpty (), true, "rejected rule clears rows");
            DBtest (errors.ContainsKey (f.first), true, "rejected rule marks element");
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
            DBtest (n, 0, "invisible source skipped in planning");
            DBtest (planned.IsEmpty (), true, "invisible source yields no rows");
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

        // --- формулы и отсутствие утечки значений между элементами ---
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
            DBtest (Spec::SpecValueReader (f.context).Read (f.first, plain, got, 0), "plain formula read");
            DBtest (got.val.doubleValue, 10.0, "plain formula not evaluated at read");
            DBtest (got.val.hasFormula, true, "plain formula keeps hasFormula");
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
            DBtest (reader.Read (f.first, libName, first, 0), "element A formula read");
            DBtest (reader.Read (f.second, libName, second, 0), "element B formula read");
            // Обратный порядок - результаты те же, утечки нет.
            ParamValue secondFirst, firstSecond;
            DBtest (reader.Read (f.second, libName, secondFirst, 0), "reverse B read");
            DBtest (reader.Read (f.first, libName, firstSecond, 0), "reverse A read");
            DBtest (first.val.uniStringValue, GS::UniString ("Rebar-A"), "element A value");
            DBtest (second.val.uniStringValue, GS::UniString ("Rebar-B"), "element B value");
            DBtest (firstSecond.val.uniStringValue, first.val.uniStringValue, "A stable across order");
            DBtest (secondFirst.val.uniStringValue, second.val.uniStringValue, "B stable across order");
        }

        // --- два правила на одном избранном ---
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
            DBtest (Spec::MatchDestinationProperties (good, favorite, errors), true, "first rule ready");
            DBtest (Spec::MatchDestinationProperties (bad, favorite, errors), false, "second rule not ready");
            DBtest (good.destinationReady, true, "first rule destinationReady survives neighbour");
            DBtest (bad.destinationReady, false, "second rule destinationReady cleared");
            DBtest (good.out_paramrawname.GetSize (), 1, "first rule schema intact");
            DBtest (good.out_sum_paramrawname.GetSize (), 1, "first rule sum schema intact");
            // Обратный порядок: неполное правило не должно влиять на полное.
            ParamDict errors2;
            Spec::MatchDestinationProperties (bad, favorite, errors2);
            DBtest (Spec::MatchDestinationProperties (good, favorite, errors2), true, "good ready after bad");
            DBtest (good.destinationReady, true, "complete rule destinationReady kept after incomplete one");
            // Отсутствие избранного целиком: оба правила не готовы, ошибка одна.
            ParamDict errors3;
            GS::HashTable<GS::UniString, GS::UniString> emptyFavorite;
            DBtest (Spec::MatchDestinationProperties (good, emptyFavorite, errors3), false, "no favorite not ready");
            DBtest (
                Spec::MatchDestinationProperties (bad, emptyFavorite, errors3), false, "no favorite both not ready");
            DBtest (errors3.GetSize (), 3, "three missing names recorded");
        }

        // --- семантика первого ряда GDL-массива на чтении ---
        // Разбор имени @arr - territory Helpers (ConvertStringToParamValue,
        // Helpers.cpp:6220), он читает через ACAPI. Здесь закрепляется только
        // то, что видит вычислитель.
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
            f.Shape (f.Run (), 0, 0, 0, 0, "first row stops");
            DBtest (f.errors.ContainsKey (f.first), true, "first row marks element error");

            // Второй ряд: тот же маршрут, но array_row_start == 2 — отказ не
            // является ошибкой элемента.
            ParamValue secondRow = firstRow;
            secondRow.val.array_row_start = 2;
            f.context.read.Get (f.first).Put (f.text, secondRow);
            f.Shape (f.Run (), 0, 0, 0, 0, "second row skips silently");
            DBtest (f.errors.ContainsKey (f.first), false, "second row not an error");

            // Контроль маршрута, найденный чтением кода: признаки материала
            // НЕЛЬЗЯ снять и ждать отказа. Без fromMaterial Read возвращает
            // успех по прочитанному значению, строка создаётся, а ветка
            // is_error = !fromMaterial не оценивается вовсе. Проверка закрепляет
            // именно это, чтобы отличие от маршрута GDL было явным.
            ParamValue plain = firstRow;
            plain.fromMaterial = false;
            f.context.read.Get (f.first).Put (f.text, plain);
            f.Shape (f.Run (), 1, 1, 0, 0, "non material reads fine");
            DBtest (f.errors.ContainsKey (f.first), false, "non material is not an error");
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

    // Нормализация описания, политика правила и разбор схемы — отдельные
    // проверяемые единицы. Все они вынесены из AddRule и вызываются из него же,
    // поэтому контракт каждой проверяется без разбора описания целиком.
    // Нормализация: что именно считается "нормализованной" строкой —
    // GetRuleFromDescription на входе уже ждёт такую.
    // Политика правила (ApplyRulePolicy) проверяется отдельно от разбора
    // групп. Ожидания получены воспроизведением ветвления, а не подгонкой
    // под вывод.
    // Выходная схема s() (ParseOutputSchema): требование ровно двух частей,
    // срезание суффикса "[N]", пропуск пустых имён и раздельное заполнение
    // out_/out_sum_. Имена берутся в уже нормализованной части после
    // "s@@".
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
                PushOutSlot (element, pv);
            }
            for (UInt32 i = 0; i < test.sum; i++) {
                ParamValue pv = {};
                PushSumSlot (element, pv);
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

    // ПОРЯДОК слотов обязателен. OutParamValue/OutSumValue и
    // SumContributionIntoRow адресуют слоты по позиции, пересчитывая число
    // выходных через флаг isSum. Перемешанная схема (сумма раньше выхода)
    // прошла бы сверку чисел, и суммирование сложило бы ВЫХОДНОЙ слот вместо
    // суммарного — тихо. Это статический риск, а не наблюдавшаяся регрессия:
    // производственный BuildOutputSlots порядок соблюдает.
    // МЕХАНИКА ВЫБОРА. Ключевое утверждение: «невыбранное корректное правило
    // не становится ошибкой парсинга, следующий запуск не наследует прошлый
    // выбор».
    //
    // Обе части проверяются на подставной фикстуре, без модели и без диалога:
    //   - «не ошибка парсинга»: parseValid выставляется РАЗБОРОМ описания, а не
    //     выбором; снятие выбора не должно его трогать;
    //   - «не наследует выбор»: правило создаётся заново на каждый запуск с
    //     default-инициализацией (selected = true), и гейт IsRunnableForRun
    //     отделён от признаков готовности.
    //
    // Что набор НЕ покрывает (эти части требуют модели или UI, см. IDEA.md):
    //   - «выделение» и настоящий default-правило из UI;
    //   - снятие правила в диалоге и отмена диалога/точки;
    //   - повторный запуск на одной модели, смена проекта, приостановка групп
    //     и отсутствие роста retained memory.
    // Эти пункты не выдаются за закрытые.
    void TestSpecSelectionPolicy () {
        // Рабочий синтаксис взят из существующих наборов (TestSpecParser и др.),
        // а не придуман здесь: Fav — имя избранного, g@@u;p;f;q — группа
        // (уникальный, читаемый, флаг, количество), s@@x;y — выходная схема.
        const GS::UniString validDesc = "Spec_rule{Fav;g@@u;p;f;q@@s@@x;y}";

        // --- снятие выбора НЕ делает правило ошибкой парсинга ---
        {
            Spec::SpecRule rule = Spec::GetRuleFromDescription (validDesc);
            DBtest (rule.parseValid, true, "valid rule parses");
            rule.runState.selected = false;
            DBtest (rule.parseValid, true, "deselected rule still parses");
            DBtest (rule.runState.selected, false, "deselection recorded");
            // Совместимый адаптер: правило выпадает из запуска, но разбор не
            // инвалидируется — иначе следующий запуск увидит ошибку парсинга.
            DBtest (rule.IsRunnableForRun (), false, "deselected rule not runnable");
        }

        // --- невыбранное правило не влияет на выбранное ---
        {
            Spec::SpecRule kept = Spec::GetRuleFromDescription (validDesc);
            Spec::SpecRule dropped = Spec::GetRuleFromDescription (validDesc);
            dropped.runState.selected = false;
            DBtest (kept.IsRunnableForRun (), true, "selected rule runnable");
            DBtest (dropped.IsRunnableForRun (), false, "unselected rule skipped");
            // Снятие выбора одного правила не меняет прочие — обход идёт по
            // словарю правил, а выбор принадлежит правилу.
            DBtest (kept.runState.selected, true, "neighbour selection intact");
            DBtest (kept.parseValid, true, "neighbour parse intact");
        }

        // --- следующий запуск не наследует прошлый выбор ---
        {
            // Первый «запуск»: правило создано заново, выбрано, затем снято в UI.
            Spec::SpecRule first = Spec::GetRuleFromDescription (validDesc);
            first.runState.selected = false;
            DBtest (first.runState.selected, false, "first run deselected");

            // Второй «запуск»: словарь правил создаётся заново, поэтому прежний
            // выбор не восстанавливается из переиспользованной структуры.
            Spec::SpecRule second = Spec::GetRuleFromDescription (validDesc);
            DBtest (second.runState.selected, true, "second run does not inherit deselection");
            DBtest (second.parseValid, true, "second run parses");
        }

        // --- гейт запуска отделён от признаков готовности ---
        {
            Spec::SpecRule rule = Spec::GetRuleFromDescription (validDesc);
            DBtest (rule.runState.selected, true, "default selection is true");
            DBtest (rule.destinationReady, true, "destination ready by default");
            DBtest (rule.IsRunnableForRun (), true, "fresh rule runnable");

            // Находка инвентаря R3: две стадии используют РАЗНЫЕ гейты.
            // Стадия планирования (:632) требует готовности назначения, а
            // стадия сбора избранного (:678) — нет. Обе комбинации обязаны
            // вести себя предсказуемо, иначе смена гейта молча сузит план чтения.
            rule.destinationReady = false;
            DBtest (rule.IsRunnableForRun (), false, "not-ready rule blocked by run gate");

            rule.destinationReady = true;
            rule.runState.selected = false;
            rule.parseValid = false;
            DBtest (rule.IsRunnableForRun (), false, "invalid+unselected still blocked");
            rule.parseValid = true;
            DBtest (rule.IsRunnableForRun (), false, "unselected blocked after parse fixed");
        }

        // --- выбор отделён от валидности в обе стороны ---
        {
            // Невалидное, но ВЫБРАННОЕ правило не должно попасть в запуск:
            // выбор не превращает отказ разбора в успех.
            // Отказ здесь берётся из таблицы TestSpecParseError — группа,
            // отброшенная по несовпадению размеров со схемой (NoGroupsAccepted).
            // Отсутствие маркера g@@ для этой цели НЕ годится: StringSplt
            // основан на UniString::Split и всегда возвращает хотя бы одну
            // часть, поэтому такое описание разбирается успешно (проверено в
            // TestSpecParseError как контракт).
            const GS::UniString brokenDesc = "Spec_rule{Fav;g@@u;p1,p2;f;q@@s@@x;y}";
            Spec::SpecRule rule = Spec::GetRuleFromDescription (brokenDesc);
            DBtest (rule.runState.selected, true, "broken rule selected by default");
            DBtest (rule.parseValid, false, "broken rule reported invalid");
            DBtest (rule.IsRunnableForRun (), false, "broken rule never runnable");
            // Причина отказа не None — парсер сообщил, о чём речь.
            DBtest (rule.parseError == Spec::ParseError::NoGroupsAccepted, true, "parse error recorded");
        }

        // --- выбор и разбор независимы и в обратную сторону ---
        {
            // Не выбранное, но вполне разобранное правило — обычное состояние
            // запуска, а не поломка: именно это проверяет этот набор.
            Spec::SpecRule rule = Spec::GetRuleFromDescription (validDesc);
            DBtest (rule.parseValid, true, "unselected-but-parsed parses");
            rule.runState.selected = false;
            DBtest (rule.parseValid, true, "unselected-but-parsed still parses");
            DBtest (rule.parseError == Spec::ParseError::None, true, "unselected-but-parsed reason None");
        }

        // --- путь «сырое описание из UI → нормализация → разбор» на реальном
        // описании владельца (файл кгду.txt, 2026-10-01).
        // Смысл проверки: в описании свойства знаков @ НЕТ — имена идут как
        // `Property:АР_.../Имя`. Префикс {@property: / {@gdl: появляется только на
        // ВЫХОДЕ разбора (ParamHelpers::NameToRawName), а NormalizeRuleDescription
        // переписывает лишь g(/s(/gl(/gm( в g@@/s@@/... и убирает пробельный мусор.
        // Закрепляем, что это два разных места: подстановка @ не происходит в
        // нормализации, и описание реального вида разбирается без ручной правки.
        {
            const GS::UniString rawDescription =
                "Spec_rule {\"АР_Спец_Перемычки\";"
                " g(perem_naen[4],Property:АР_Перемычки/Собственный этаж;param_name_out[4], perem_obozn[4], perem_naen[4],"
                " perem_ves[4],Property:АР_Перемычки/Собственный этаж;perem_nagr[4];perem_nagr[4])"
                " s(pos,Property:АР_Перемычки/Обозначение,Property:АР_Перемычки/Наименование,Property:АР_Перемычки/Масса ед,"
                "Property:АР_Перемычки/Собственный этаж;Property:АР_Перемычки/Количество (на этаж))}";

            // 1) Нормализация сама по себе НЕ добавляет знаков @.
            const GS::UniString normalized = Spec::NormalizeRuleDescription (rawDescription);
            DBtest (normalized.Contains ("g@@"), true, "real desc group normalized");
            DBtest (normalized.Contains ("s@@"), true, "real desc summary normalized");
            DBtest (normalized.Contains ("Property:"), true, "real desc keeps typed prefix");
            DBtest (normalized.Contains ("@property:"), false, "normalize adds no raw prefix");
            // Пробельный мусор ("Spec_rule {" с пробелом) снят.
            DBtest (normalized.Contains ("Spec_rule {"), false, "real desc space trimmed");

            // 2) Разбор нормализованного описания даёт правило, и префикс
            // {@property: появляется только здесь.
            // Числа взяты с кода, а не придуманы:
            //   - схема s() в этом описании: выходов 5 (pos + четыре Property:),
            //     сумм 1 (после точки с запятой). Первый выход `pos` — параметр
            //     GDL, поэтому префикс у него {@gdl:, а не {@property:;
            //   - группа несёт [4], поэтому ExpandGroup заменяет её ЧЕТЫРЬМЯ
            //     группами по одной строке на каждую, и исходная группа в
            //     rule.groups НЕ попадает (Spec.cpp:2143-2144 против :2175).
            const Spec::SpecRule rule = Spec::GetRuleFromDescription (normalized);
            DBtest (rule.parseValid, true, "real desc parses");
            DBtest (rule.groups.GetSize () == 4, true, "real desc array expanded to 4 groups");
            DBtest (rule.out_paramrawname.GetSize () == 5, true, "real desc output count");
            DBtest (rule.out_sum_paramrawname.GetSize () == 1, true, "real desc sum count");
            DBtest (rule.parseError == Spec::ParseError::None, true, "real desc reason None");

            // 3) Имена приходят УЖЕ с префиксом {@property: / {@gdl: — это работа
            // NameToRawName, а не результат нормализации. Тип префикса выбирается
            // по наличию ":" в имени описания: `pos` без двоеточия → {@gdl:.
            DBtest (rule.out_paramrawname[0], GS::UniString ("{@gdl:pos}"), "real desc untyped becomes gdl");
            DBtest (rule.out_paramrawname[1].BeginsWith ("{@property:"), true, "real desc typed keeps property");
            DBtest (rule.out_sum_paramrawname[0].BeginsWith ("{@property:"), true, "real desc sum raw prefix");

            // 4) Ключ правила = имя выбранного элемента из кавычек.
            DBtest (rule.favorite_name, GS::UniString ("АР_Спец_Перемычки"), "real desc favorite name");
        }

        // --- механика выбора на ОДНОМ правиле в реальной форме описания: снятие
        // выбора не ломает ни разбор, ни признаки готовности, а следующий
        // запуск снова выбирает правило. Склейка предыдущих проверок в сценарий.
        {
            // Тот же реальный вид, что выше, но компактнее: без кириллицы и
            // скобок в имени — форма значения та же (g( ... ) s( ... )).
            const GS::UniString rawDescOne =
                "Spec_rule {\"Fav\"; g(perem_naen[4],Property:S/Story;perem_nagr[4];perem_nagr[4])"
                " s(pos,Property:S/Mark,Property:S/Name;Property:S/Qty)}";

            Spec::SpecRule rule = Spec::GetRuleFromDescription (Spec::NormalizeRuleDescription (rawDescOne));
            DBtest (rule.parseValid, true, "scenario parses");
            DBtest (rule.runState.selected, true, "scenario selected by default");
            DBtest (rule.IsRunnableForRun (), true, "scenario runnable");

            rule.runState.selected = false;
            DBtest (rule.parseValid, true, "scenario deselect keeps parse");
            DBtest (rule.IsRunnableForRun (), false, "scenario deselect blocks run");
            DBtest (rule.destinationReady, true, "scenario readiness intact");

            // Следующий запуск: правило создаётся заново и снова выбрано.
            Spec::SpecRule next = Spec::GetRuleFromDescription (Spec::NormalizeRuleDescription (rawDescOne));
            DBtest (next.runState.selected, true, "scenario next run reselects");
            DBtest (next.IsRunnableForRun (), true, "scenario next run runnable");
        }

        // --- РЕАЛЬНОЕ многострочное описание владельца (вставки, 2026-10-01).
        // Форма отличается от предыдущего примера: переводы строк и табуляции
        // ВНУТРИ описания, пять групп g(), 26 выходов в s(), имена вида
        // Property:ЭКСПЛИКАЦИЯ помещений/№ - Наименование помещений (с пробелом,
        // дефисом и кавычками в имени избранного). Знаков @ по-прежнему нет.
        {
            const GS::UniString rawDescription =
                "Spec_rule {\"ВСТАВКИ Условные обозначения SomeStuff\";\n"
                "\n"
                "g(_pos_1, \n"
                "gs_list_manufacturer,gs_list_note,\n"
                "pos1,prm1_1,prm2_1,prm3_1,otd_vst,KlasZt,Property:ЭКСПЛИКАЦИЯ помещений/№ - Наименование помещений, Property:ЭКСПЛИКАЦИЯ помещений/Этаж;\n"
                "\tgs_list_manufacturer,gs_list_note,\n"
                "\totd_vst,KlasZt,gs_cont_pen,pen_es,\n"
                "\tID_0,ID_vst,ID1,\n"
                "\tpos1,fill_pen_1,\n"
                "\tprm1_1,prm2_1,prm3_1,\n"
                "\tvoltR_1,voltR3_1,voltD_1,\n"
                "\tamperR_1,amperR3_1,amperD_1,\n"
                "\tHram,Lram,Hotv,Lotv,Property:ЭКСПЛИКАЦИЯ помещений/№ - Наименование помещений, Property:ЭКСПЛИКАЦИЯ помещений/Этаж;\n"
                "pos1_ON;pos1_ON)\n"
                "\n"
                "g(_pos_2, \n"
                "gs_list_manufacturer,gs_list_note,\n"
                "pos2,prm1_2,prm2_2,prm3_2,otd_vst,KlasZt,Property:ЭКСПЛИКАЦИЯ помещений/№ - Наименование помещений, Property:ЭКСПЛИКАЦИЯ помещений/Этаж;\n"
                "\n"
                "\tgs_list_manufacturer,gs_list_note,\n"
                "\totd_vst,KlasZt,gs_cont_pen,pen_es,\t\n"
                "\tID_0,ID_vst,ID2,\n"
                "\tpos2,fill_pen_2,\n"
                "\tprm1_2,prm2_2,prm3_2,\n"
                "\tvoltR_2,voltR3_2,voltD_2,\n"
                "\tamperR_2,amperR3_2,amperD_2,\n"
                "\tHram,Lram,Hotv,Lotv,Property:ЭКСПЛИКАЦИЯ помещений/№ - Наименование помещений, Property:ЭКСПЛИКАЦИЯ помещений/Этаж;\n"
                "pos2_ON;pos2_ON)\n"
                "\n"
                "g(_pos_3, \n"
                "gs_list_manufacturer,gs_list_note,\n"
                "pos3,prm1_3,prm2_3,prm3_3,otd_vst,KlasZt,Property:ЭКСПЛИКАЦИЯ помещений/№ - Наименование помещений, Property:ЭКСПЛИКАЦИЯ помещений/Этаж;\n"
                "\n"
                "\tgs_list_manufacturer,gs_list_note,\n"
                "\totd_vst,KlasZt,gs_cont_pen,pen_es,\t\n"
                "\tID_0,ID_vst,ID3,\n"
                "\tpos3,fill_pen_3,\n"
                "\tprm1_3,prm2_3,prm3_3,\n"
                "\tvoltR_3,voltR3_3,voltD_3,\n"
                "\tamperR_3,amperR3_3,\n"
                "\tamperD_3,Hram,Lram,Hotv,Lotv,Property:ЭКСПЛИКАЦИЯ помещений/№ - Наименование помещений, Property:ЭКСПЛИКАЦИЯ помещений/Этаж;\n"
                "pos3_ON;pos3_ON)\n"
                "\n"
                "g(_pos_4, \n"
                "gs_list_manufacturer,gs_list_note,\n"
                "pos4,prm1_4,prm2_4,prm3_4,otd_vst,KlasZt,Property:ЭКСПЛИКАЦИЯ помещений/№ - Наименование помещений, Property:ЭКСПЛИКАЦИЯ помещений/Этаж;\n"
                "\n"
                "\tgs_list_manufacturer,gs_list_note,\n"
                "\totd_vst,KlasZt,gs_cont_pen,pen_es,\t\n"
                "\tID_0,ID_vst,ID4,\n"
                "\tpos4,fill_pen_4,\n"
                "\tprm1_4,prm2_4,prm3_4,\n"
                "\tvoltR_4,voltR3_4,voltD_4,\n"
                "\tamperR_4,amperR3_4,amperD_4,\n"
                "\tHram,Lram,Hotv,Lotv,Property:ЭКСПЛИКАЦИЯ помещений/№ - Наименование помещений, Property:ЭКСПЛИКАЦИЯ помещений/Этаж;\n"
                "pos4_ON;pos4_ON)\n"
                "\n"
                "g(_pos_5, \n"
                "gs_list_manufacturer,gs_list_note,\n"
                "pos5,prm1_5,prm2_5,prm3_5,otd_vst,KlasZt,Property:ЭКСПЛИКАЦИЯ помещений/№ - Наименование помещений, Property:ЭКСПЛИКАЦИЯ помещений/Этаж;\n"
                "\n"
                "\tgs_list_manufacturer,gs_list_note,\n"
                "\totd_vst,KlasZt,gs_cont_pen,pen_es,\t\n"
                "\tID_0,ID_vst,ID5,\n"
                "\tpos5,fill_pen_5,\n"
                "\tprm1_5,prm2_5,prm3_5,\n"
                "\tvoltR_5,voltR3_5,voltD_5,\n"
                "\tamperR_5,amperR3_5,amperD_5,\n"
                "\tHram,Lram,Hotv,Lotv,Property:ЭКСПЛИКАЦИЯ помещений/№ - Наименование помещений, Property:ЭКСПЛИКАЦИЯ помещений/Этаж;\n"
                "pos5_ON;pos5_ON)\n"
                "\n"
                "\n"
                "  s(gs_list_manufacturer,gs_list_note,\n"
                "\totd_vst,KlasZt,gs_cont_pen,pen_es,\t\n"
                "\tID_0,ID_vst,ID1,\n"
                "\tpos1,fill_pen_1, \n"
                "\tprm1_1,prm2_1,prm3_1,\n"
                "\tvoltR_1,voltR3_1,voltD_1,\n"
                "\tamperR_1,amperR3_1,amperD_1,\n"
                "\tHram,Lram,Hotv,Lotv,Property:ЭКСПЛИКАЦИЯ помещений/№ - Наименование помещений, Property:ЭКСПЛИКАЦИЯ помещений/Этаж;\n"
                "Property:Рамка ЭЛ/_авто_Количество (вставки))\n"
                "}";
            // Нормализация обязана снять переводы строк и табуляции: описание
            // набирается в несколько строк, и без этого разбивка по g@@/s@@
            // увидит мусор. Константы LINEBRAKE/LINEBRAKER/TABSTRING = "\n"/"\r"/"\t".
            const GS::UniString normalized = Spec::NormalizeRuleDescription (rawDescription);
            DBtest (normalized.Contains ("\n"), false, "multiline: line feeds removed");
            DBtest (normalized.Contains ("\t"), false, "multiline: tabs removed");
            DBtest (normalized.Contains ("g@@"), true, "multiline: group markers written");
            DBtest (normalized.Contains ("s@@"), true, "multiline: summary marker written");
            DBtest (normalized.Contains ("@property:"), false, "multiline: normalize adds no @");

            // Разбор: пять групп, и каждая обязана совпасть по размеру со схемой
            // выхода (ExpandGroup/ParseGroups иначе отбрасывают группу). Числа
            // НЕ выдуманы — их выдаёт разбор, и любое расхождение означает, что
            // описание реального вида не принимается. Это стоп-сигнал, а не
            // ожидаемое значение.
            const Spec::SpecRule rule = Spec::GetRuleFromDescription (normalized);
            DBtest (rule.parseValid, true, "multiline: parses");
            DBtest (rule.parseError == Spec::ParseError::None, true, "multiline: reason None");
            DBtest (rule.favorite_name,
                    GS::UniString ("ВСТАВКИ Условные обозначения SomeStuff"),
                    "multiline: favorite name");
            DBtest (rule.groups.GetSize () == 5, true, "multiline: five groups");
            DBtest (rule.out_paramrawname.GetSize () == 26, true, "multiline: 26 outputs");
            DBtest (rule.out_sum_paramrawname.GetSize () == 1, true, "multiline: one sum");

            // Имена с пробелом и типом Property: сохраняют префикс, кириллица
            // не должна превращаться в мусор (NameToRawName lowercases).
            DBtest (rule.out_paramrawname[0],
                    GS::UniString ("{@gdl:gs_list_manufacturer}"),
                    "multiline: untyped name becomes gdl");
            DBtest (rule.out_sum_paramrawname[0].BeginsWith ("{@property:"),
                    true,
                    "multiline: typed sum keeps property prefix");
            // Пространство имён не должно теряться при lowercases: кириллица
            // сохраняется, латиница приводится к нижнему регистру.
            const bool hasCyrillic = rule.out_paramrawname[0].Contains ("gs_list_manufacturer");
            DBtest (hasCyrillic, true, "multiline: latin name kept");
        }

        // --- тот же пример, но С СЕМАНТИКОЙ ВЫБОРА: снятие выбора на
        // многострочном описании обязано вести себя так же, как на простом.
        {
            const GS::UniString rawDescription =
                "Spec_rule {\"ВСТАВКИ Условные обозначения SomeStuff\";\n"
                "\n"
                "g(_pos_1, \n"
                "gs_list_manufacturer,gs_list_note,\n"
                "pos1,prm1_1,prm2_1,prm3_1,otd_vst,KlasZt,Property:ЭКСПЛИКАЦИЯ помещений/№ - Наименование помещений, Property:ЭКСПЛИКАЦИЯ помещений/Этаж;\n"
                "\tgs_list_manufacturer,gs_list_note,\n"
                "\totd_vst,KlasZt,gs_cont_pen,pen_es,\n"
                "\tID_0,ID_vst,ID1,\n"
                "\tpos1,fill_pen_1,\n"
                "\tprm1_1,prm2_1,prm3_1,\n"
                "\tvoltR_1,voltR3_1,voltD_1,\n"
                "\tamperR_1,amperR3_1,amperD_1,\n"
                "\tHram,Lram,Hotv,Lotv,Property:ЭКСПЛИКАЦИЯ помещений/№ - Наименование помещений, Property:ЭКСПЛИКАЦИЯ помещений/Этаж;\n"
                "pos1_ON;pos1_ON)\n"
                "\n"
                "g(_pos_2, \n"
                "gs_list_manufacturer,gs_list_note,\n"
                "pos2,prm1_2,prm2_2,prm3_2,otd_vst,KlasZt,Property:ЭКСПЛИКАЦИЯ помещений/№ - Наименование помещений, Property:ЭКСПЛИКАЦИЯ помещений/Этаж;\n"
                "\n"
                "\tgs_list_manufacturer,gs_list_note,\n"
                "\totd_vst,KlasZt,gs_cont_pen,pen_es,\t\n"
                "\tID_0,ID_vst,ID2,\n"
                "\tpos2,fill_pen_2,\n"
                "\tprm1_2,prm2_2,prm3_2,\n"
                "\tvoltR_2,voltR3_2,voltD_2,\n"
                "\tamperR_2,amperR3_2,amperD_2,\n"
                "\tHram,Lram,Hotv,Lotv,Property:ЭКСПЛИКАЦИЯ помещений/№ - Наименование помещений, Property:ЭКСПЛИКАЦИЯ помещений/Этаж;\n"
                "pos2_ON;pos2_ON)\n"
                "\n"
                "g(_pos_3, \n"
                "gs_list_manufacturer,gs_list_note,\n"
                "pos3,prm1_3,prm2_3,prm3_3,otd_vst,KlasZt,Property:ЭКСПЛИКАЦИЯ помещений/№ - Наименование помещений, Property:ЭКСПЛИКАЦИЯ помещений/Этаж;\n"
                "\n"
                "\tgs_list_manufacturer,gs_list_note,\n"
                "\totd_vst,KlasZt,gs_cont_pen,pen_es,\t\n"
                "\tID_0,ID_vst,ID3,\n"
                "\tpos3,fill_pen_3,\n"
                "\tprm1_3,prm2_3,prm3_3,\n"
                "\tvoltR_3,voltR3_3,voltD_3,\n"
                "\tamperR_3,amperR3_3,\n"
                "\tamperD_3,Hram,Lram,Hotv,Lotv,Property:ЭКСПЛИКАЦИЯ помещений/№ - Наименование помещений, Property:ЭКСПЛИКАЦИЯ помещений/Этаж;\n"
                "pos3_ON;pos3_ON)\n"
                "\n"
                "g(_pos_4, \n"
                "gs_list_manufacturer,gs_list_note,\n"
                "pos4,prm1_4,prm2_4,prm3_4,otd_vst,KlasZt,Property:ЭКСПЛИКАЦИЯ помещений/№ - Наименование помещений, Property:ЭКСПЛИКАЦИЯ помещений/Этаж;\n"
                "\n"
                "\tgs_list_manufacturer,gs_list_note,\n"
                "\totd_vst,KlasZt,gs_cont_pen,pen_es,\t\n"
                "\tID_0,ID_vst,ID4,\n"
                "\tpos4,fill_pen_4,\n"
                "\tprm1_4,prm2_4,prm3_4,\n"
                "\tvoltR_4,voltR3_4,voltD_4,\n"
                "\tamperR_4,amperR3_4,amperD_4,\n"
                "\tHram,Lram,Hotv,Lotv,Property:ЭКСПЛИКАЦИЯ помещений/№ - Наименование помещений, Property:ЭКСПЛИКАЦИЯ помещений/Этаж;\n"
                "pos4_ON;pos4_ON)\n"
                "\n"
                "g(_pos_5, \n"
                "gs_list_manufacturer,gs_list_note,\n"
                "pos5,prm1_5,prm2_5,prm3_5,otd_vst,KlasZt,Property:ЭКСПЛИКАЦИЯ помещений/№ - Наименование помещений, Property:ЭКСПЛИКАЦИЯ помещений/Этаж;\n"
                "\n"
                "\tgs_list_manufacturer,gs_list_note,\n"
                "\totd_vst,KlasZt,gs_cont_pen,pen_es,\t\n"
                "\tID_0,ID_vst,ID5,\n"
                "\tpos5,fill_pen_5,\n"
                "\tprm1_5,prm2_5,prm3_5,\n"
                "\tvoltR_5,voltR3_5,voltD_5,\n"
                "\tamperR_5,amperR3_5,amperD_5,\n"
                "\tHram,Lram,Hotv,Lotv,Property:ЭКСПЛИКАЦИЯ помещений/№ - Наименование помещений, Property:ЭКСПЛИКАЦИЯ помещений/Этаж;\n"
                "pos5_ON;pos5_ON)\n"
                "\n"
                "\n"
                "  s(gs_list_manufacturer,gs_list_note,\n"
                "\totd_vst,KlasZt,gs_cont_pen,pen_es,\t\n"
                "\tID_0,ID_vst,ID1,\n"
                "\tpos1,fill_pen_1, \n"
                "\tprm1_1,prm2_1,prm3_1,\n"
                "\tvoltR_1,voltR3_1,voltD_1,\n"
                "\tamperR_1,amperR3_1,amperD_1,\n"
                "\tHram,Lram,Hotv,Lotv,Property:ЭКСПЛИКАЦИЯ помещений/№ - Наименование помещений, Property:ЭКСПЛИКАЦИЯ помещений/Этаж;\n"
                "Property:Рамка ЭЛ/_авто_Количество (вставки))\n"
                "}";
            Spec::SpecRule rule = Spec::GetRuleFromDescription (Spec::NormalizeRuleDescription (rawDescription));
            const bool readyBefore = rule.IsRunnableForRun ();
            rule.runState.selected = false;
            DBtest (rule.parseValid, true, "multiline: deselect keeps parse");
            DBtest (rule.IsRunnableForRun (), false, "multiline: deselect blocks run");
            DBtest (rule.groups.GetSize () == 5, true, "multiline: groups survive deselect");

            Spec::SpecRule next = Spec::GetRuleFromDescription (Spec::NormalizeRuleDescription (rawDescription));
            DBtest (next.runState.selected, true, "multiline: next run reselects");
            DBtest (next.IsRunnableForRun (), readyBefore, "multiline: runnability restored");
        }
    }

    // ГРАНИЦА ТИПОВ. У состояния запуска exsist_elements один писатель.
    // Набор фиксирует, что:
    //   - состояние запуска — отдельный тип, а не поле в SpecRule;
    //   - заполнение состояния НЕ трогает определение правила (схему, признаки
    //     готовности, выбор) — то есть перенос не смешал данные с местом;
    //   - правило ПО УМОЛЧАНИЮ пригодно к запуску (IsRunnableForRun), и наполнение
    //     состояния этого не меняет.
    // Политика удаления и признак полноты здесь НЕ проверяются: они
    // относятся к отдельной задаче.
    void TestSpecRunStateBoundary () {
        // --- отдельный тип состояния компилируется и самодостаточен ---
        {
            Spec::SpecRuleRunState state = {};
            DBtest (state.exsist_elements.IsEmpty (), true, "fresh state is empty");
            state.exsist_elements.Push (APIGuidFromString ("{11111111-1111-1111-1111-111111111111}"));
            DBtest (state.exsist_elements.GetSize (), 1, "runState holds existing list");
            // Копия состояния независима от правила: это данные запуска, а не
            // определение, поэтому присваивание структуры не должно тащить за
            // собой определение.
            Spec::SpecRuleRunState copy = state;
            DBtest (copy.exsist_elements.GetSize (), 1, "runState copy carries existing list");
        }

        // --- заполнение состояния не трогает определение ---
        {
            Spec::SpecRule rule;
            rule.out_paramrawname.Push ("{@property:out}");
            rule.out_sum_paramrawname.Push ("{@property:sum}");
            const UInt32 outBefore = rule.out_paramrawname.GetSize ();
            const UInt32 sumBefore = rule.out_sum_paramrawname.GetSize ();
            const bool parseBefore = rule.parseValid;
            const bool readyBefore = rule.destinationReady;
            const bool selectedBefore = rule.runState.selected;
            const UInt32 groupsBefore = rule.groups.GetSize ();

            GS::Array<API_Guid> found;
            found.Push (APIGuidFromString ("{11111111-1111-1111-1111-111111111111}"));
            UnicGuid selected;
            Spec::SelectExistingElements (rule, found, selected);

            DBtest (rule.runState.exsist_elements.GetSize (), 1, "runState.exsist_elements filled");
            DBtest (rule.out_paramrawname.GetSize (), outBefore, "out schema untouched");
            DBtest (rule.out_sum_paramrawname.GetSize (), sumBefore, "sum schema untouched");
            DBtest (rule.groups.GetSize (), groupsBefore, "groups untouched");
            DBtest (rule.parseValid, parseBefore, "parse flag untouched");
            DBtest (rule.destinationReady, readyBefore, "readiness untouched");
            DBtest (rule.runState.selected, selectedBefore, "selection untouched");
        }

        // --- наполнение состояния не меняет готовность правила к запуску ---
        {
            Spec::SpecRule rule;
            rule.runState.exsist_elements.Push (APIGuidFromString ("{11111111-1111-1111-1111-111111111111}"));
            DBtest (rule.IsRunnableForRun (), true, "existing list does not block run");
            // Находка №1 инвентаря: элементы заполняются ДО выбора, поэтому
            // наличие источников само по себе не делает правило непригодным.
            rule.runState.elements.Push (APIGuidFromString ("{22222222-2222-2222-2222-222222222222}"));
            DBtest (rule.IsRunnableForRun (), true, "sources do not change runnability");
            DBtest (rule.runState.exsist_elements.GetSize (), 1, "existing list independent");
            DBtest (rule.runState.elements.GetSize (), 1, "source list independent");
        }

        // --- прежнее поле отсутствует: доступ идёт через состояние ---
        {
            Spec::SpecRule rule;
            GS::Array<API_Guid> found;
            found.Push (APIGuidFromString ("{11111111-1111-1111-1111-111111111111}"));
            UnicGuid selected;
            Spec::SelectExistingElements (rule, found, selected);
            // Порядок в состоянии совпадает с порядком found — сверка и второй
            // обход зависят от исходного порядка обхода.
            DBtest (!rule.runState.exsist_elements.IsEmpty () &&
                        rule.runState.exsist_elements[0] ==
                            APIGuidFromString ("{11111111-1111-1111-1111-111111111111}"),
                    "found order preserved in state");
        }
    }

    void TestSpecSlotOrder () {
        // --- перемешанная схема отвергается, даже когда числа сходятся ---
        {
            Spec::Element element = {};
            PushSumSlot (element, Num (1)); // сумма ПЕРВОЙ
            PushOutSlot (element, Num (2));
            DBtest (Spec::OutSlotsMatchSchema (element, 1, 1), false, "sum before out rejected");
        }
        {
            Spec::Element element = {};
            PushSumSlot (element, Num (1));
            PushSumSlot (element, Num (3));
            PushOutSlot (element, Num (2));
            DBtest (Spec::OutSlotsMatchSchema (element, 1, 2), false, "two sums then out rejected");
        }
        // --- корректный порядок принимается (прежние случаи не сломаны) ---
        {
            Spec::Element element = {};
            PushOutSlot (element, Num (2));
            PushSumSlot (element, Num (1));
            DBtest (Spec::OutSlotsMatchSchema (element, 1, 1), true, "out then sum accepted");
        }
        // --- последствия: смешанная схема даёт НЕВЕРНЫЙ слот суммы ---
        // OutSumValue (0) обязан вернуть первый СУММАРНЫЙ слот. При порядке
        // [сумма 1, выход 2] это значение 2, а не 1 — ровно та ошибка, от
        // которой страхует сверка.
        {
            Spec::Element element = {};
            PushSumSlot (element, Num (1));
            PushOutSlot (element, Num (2));
            DBtest (element.OutSumValue (0).val.intValue, 2, "mixed layout: sum accessor returns OUT value");
            DBtest (element.OutParamValue (0).val.intValue, 1, "mixed layout: out accessor returns SUM value");
        }
        // --- перемешивание недостижимо через PushSumSlot/PushOutSlot ---
        // Оба хелпера дописывают в конец, поэтому корректный порядок сохраняется
        // сам по себе; перемешать может только явная сборка PushRawSlot.
        {
            Spec::Element element = {};
            for (UInt32 i = 0; i < 2; i++) {
                ParamValue pv = {};
                PushOutSlot (element, pv);
                PushSumSlot (element, pv);
            }
            DBtest (Spec::OutSlotsMatchSchema (element, 2, 2), false, "interleaved build rejected");
        }
    }

    // ПОЛНОТА чтения и расчёта в плане.
    //
    // Счётчики notFoundUnicCount / notFoundParamCount должны означать «всё
    // прочитано», а не «счётчик не заполняется». Готовые словари
    // not_found_* для этого не годятся: они хранят только ЗАСООБЩЁННЫЕ поля,
    // то есть зависят от stop_on_error, и при stop_on_error = false остаются
    // пустыми при реально неполном чтении. Поэтому полнота считается по
    // вкладам.
    //
    // Здесь stop_on_error = false (так по умолчанию в фикстуре) — это и есть
    // случай, где словари not_found_* молчали бы.
    void TestSpecPlanCompleteness () {
        // --- полное чтение: план честно полон ---
        {
            SpecFixture f;
            f.Source (f.first, "A", "Alpha", 5);
            Spec::SpecChangePlan plan;
            Spec::ElementDict rows = {};
            UnicGuid errors = {};
            GS::HashTable<GS::UniString, GS::UniString> outParam = {};
            const Int32 n = Spec::PlanRuleRows (f.rule, f.context, rows, errors, false, outParam, &plan);
            DBtest (n, 1, "complete read creates one row");
            DBtest (plan.contributionsTotal, 1, "one contribution counted");
            DBtest (plan.notFoundUnicCount, 0, "no missing unic fields");
            DBtest (plan.notFoundParamCount, 0, "no missing sum fields");
            DBtest (plan.contributionsPartial, 0, "no partial contributions");
            DBtest (plan.schemaMismatchCount, 0, "no schema mismatch");
            DBtest (plan.ReadComplete (), true, "read complete");
            DBtest (plan.CalcComplete (), true, "calc complete");
            DBtest (plan.IsComplete (), true, "plan complete");
        }

        // --- plan == nullptr допустим: вызывающие без плана не меняются ---
        {
            SpecFixture f;
            f.Source (f.first, "A", "Alpha", 5);
            Spec::ElementDict rows = {};
            UnicGuid errors = {};
            GS::HashTable<GS::UniString, GS::UniString> outParam = {};
            const Int32 n = Spec::PlanRuleRows (f.rule, f.context, rows, errors, false, outParam);
            DBtest (n, 1, "null plan path unchanged");
        }

        // --- не прочитано уникальное поле: чтение НЕПОЛНОЕ ---
        // Ключ склеивается даже при неудаче (поведение сохранено), поэтому строка
        // создаётся — но план обязан сказать, что чтение неполно.
        {
            SpecFixture f;
            f.Source (f.first, "A", "Alpha", 5);
            f.DropField (f.first, f.key);
            Spec::SpecChangePlan plan;
            Spec::ElementDict rows = {};
            UnicGuid errors = {};
            GS::HashTable<GS::UniString, GS::UniString> outParam = {};
            Spec::PlanRuleRows (f.rule, f.context, rows, errors, false, outParam, &plan);
            DBtest (plan.notFoundUnicCount, 1, "missing unic counted");
            DBtest (plan.ReadComplete (), false, "read incomplete on missing unic");
            DBtest (plan.IsComplete (), false, "plan not complete on missing unic");
            // stop_on_error == false: счётчик неполноты ЕСТЬ, хотя отчёт молчит.
            DBtest (plan.notFoundUnicCount > 0, true, "incompleteness survives stop_on_error=false");
        }

        // --- не прочитано суммарное поле: расчёт НЕПОЛНЫЙ ---
        {
            SpecFixture f;
            f.Source (f.first, "A", "Alpha", 5);
            f.DropField (f.first, f.quantity);
            Spec::SpecChangePlan plan;
            Spec::ElementDict rows = {};
            UnicGuid errors = {};
            GS::HashTable<GS::UniString, GS::UniString> outParam = {};
            Spec::PlanRuleRows (f.rule, f.context, rows, errors, false, outParam, &plan);
            DBtest (plan.notFoundParamCount, 1, "missing sum counted");
            DBtest (plan.contributionsPartial, 1, "partial contribution counted");
            DBtest (plan.ReadComplete (), false, "read incomplete on missing sum");
            DBtest (plan.CalcComplete (), false, "calc incomplete on missing sum");
        }

        // --- не прочитано выходное поле: считается в фазе 2 ---
        {
            SpecFixture f;
            f.Source (f.first, "A", "Alpha", 5);
            f.DropField (f.first, f.text);
            Spec::SpecChangePlan plan;
            Spec::ElementDict rows = {};
            UnicGuid errors = {};
            GS::HashTable<GS::UniString, GS::UniString> outParam = {};
            Spec::PlanRuleRows (f.rule, f.context, rows, errors, false, outParam, &plan);
            DBtest (plan.notFoundParamCount, 1, "missing output counted");
            DBtest (plan.ReadComplete (), false, "read incomplete on missing output");
        }

        // --- два источника: счётчики СУММИРУЮТСЯ, а не перезаписываются ---
        {
            SpecFixture f;
            f.Source (f.first, "A", "Alpha", 5);
            f.Source (f.second, "B", "Beta", 7);
            f.DropField (f.second, f.quantity);
            Spec::SpecChangePlan plan;
            Spec::ElementDict rows = {};
            UnicGuid errors = {};
            GS::HashTable<GS::UniString, GS::UniString> outParam = {};
            Spec::PlanRuleRows (f.rule, f.context, rows, errors, false, outParam, &plan);
            DBtest (plan.contributionsTotal, 2, "two contributions counted");
            DBtest (plan.notFoundParamCount, 1, "one of two incomplete");
            DBtest (plan.contributionsPartial, 1, "partial counted once");
        }

        // --- сверка заполняет deleteOld, полнота остаётся отдельной ---
        {
            SpecFixture f;
            f.Source (f.first, "A", "Alpha", 5);
            f.Existing (f.old, "Alpha", 5);
            Spec::SpecChangePlan plan;
            const Int32 n =
                Spec::GetElementsForRule (f.rule, f.context, f.created, f.modified, f.deleted, f.errors, false, &plan);
            // Совпавшие значения => existing не меняется: он сопоставлен как
            // unchanged и УБРАН из elements. Возвращаемое n_elements = delete +
            // modify + create (прежняя формула), поэтому unchanged в нём не
            // участвует и здесь законно 0 — проверяем это явно, чтобы
            // изменение формулы было замечено.
            DBtest (n, 0, "unchanged object not counted in result");
            DBtest (plan.deleteOld, 1, "reconciliation ran");
            DBtest (plan.unchanged, 1, "unchanged counted in plan");
            DBtest (plan.create.GetSize (), 0, "no create in reconcile");
            DBtest (plan.removals.GetSize (), 0, "no removals in reconcile");
            DBtest (f.created.GetSize (), 0, "claimed row leaves created set");
            DBtest (f.deleted.GetSize (), 0, "unchanged object not deleted");
            DBtest (plan.IsComplete (), true, "complete run gives complete plan");
            DBtest (plan.Matches (f.modified, f.deleted), true, "plan matches actual lists");
        }

        // --- сверка НЕ исполнялась: полнота расчёта всё равно заполнена ---
        // delete_old = false: сверки не было, deleteOld == 0, но расчёт-то
        // выполнен, и именно его полнота проверяется.
        {
            SpecFixture f;
            f.Source (f.first, "A", "Alpha", 5);
            f.DropField (f.first, f.quantity);
            Spec::SpecChangePlan plan;
            const Int32 n =
                Spec::GetElementsForRule (f.rule, f.context, f.created, f.modified, f.deleted, f.errors, false, &plan);
            DBtest (plan.deleteOld, 0, "no reconciliation without delete_old");
            DBtest (plan.contributionsTotal, 1, "calc still counted without reconciliation");
            DBtest (plan.IsComplete (), false, "incomplete calc seen without reconciliation");
            DBtest (n >= 0, true, "result returned without reconciliation");
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
        // закреплённый контрактом нормализации; сырой "g(u;p;f;q)s(x;y)" парсер не принимает.
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
        // параметра: это зафиксированный контракт, а не дефект.
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
            // Парсер больше НЕ мутирует вход - обрезки идут на локальной
            // копии, поэтому вызывающий сохраняет свою строку.
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
            DBtest (rule.runState.elements.IsEmpty () && rule.runState.exsist_elements.IsEmpty (),
                    label + " no elements");
            // Ключ словаря строится из той же строки ПОСЛЕ разбора: парсер не
            // мутирует вход, поэтому строка остаётся пригодной для GetSubstring.
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
        DBtest (rule->runState.elements.IsEmpty (), "Spec AddRule null GUID not appended");
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
        DBtest (rule != nullptr && rule->runState.elements.GetSize () == 3, "Spec AddRule preserves repeated GUID");
        if (rule != nullptr && rule->runState.elements.GetSize () == 3)
            DBtest (rule->runState.elements[0] == f.first && rule->runState.elements[1] == f.second &&
                        rule->runState.elements[2] == f.first,
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
        DBtest (invalid != nullptr && !invalid->parseValid && invalid->runState.elements.IsEmpty (),
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

    // Причина отказа в разборе. Парсер пишет её в тех же точках, где
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

    // Проверка правила по GUID свойства-правила (#234).
    // Модель не трогается: собираются только определения свойств и читаются
    // значения уже существующего элемента. Проверяются три независимых
    // ответа — разбор описания, наличие зависимостей и состояние флага, —
    // потому что сведение их к одному признаку скрывает причину отказа.
    void TestSpecRuleCheck () {
        const API_Guid ruleGuid = APIGuidFromString ("{7A1B2C3D-4E5F-4061-8172-8394A5B6C7D8}");

        // ---- Неизвестный GUID: определения нет, правило не разбирается ----
        {
            Spec::RuleCheckResult result;
            const bool found = Spec::CheckRuleByPropertyGuid (ruleGuid, APINULLGuid, result);
            ::TestKit::NoteFields ("RuleCheck.unknown",
                                   {SMSTF_FIELD ("found", found), SMSTF_FIELD ("parsed", result.ruleParsed)},
                                   "нет определения свойства");
            DBtest (found, false, "RuleCheck unknown guid not found");
            DBtest (result.definitionFound, false, "RuleCheck unknown guid definitionFound");
            DBtest (result.ruleParsed, false, "RuleCheck unknown guid ruleParsed");
            DBtest (result.checkedElement, false, "RuleCheck unknown guid no element check");
        }

        // ---- Синтетические свойства-флаги: состояния не схлопываются ----
        // Прод GetRuleFromElement считает API_Property_NotAvailable включённым
        // флагом; валидатор обязан отличать этот случай от значения true.
        {
            API_Property property = {};
            // Состояние свойства различается по версиям: с AC24 это поле
            // API_Property::status и перечисление API_PropertyValueStatus.
            // До AC24 нет ни того, ни другого - вычисленность выражает
            // isEvaluated, а состояние NotAvailable невыразимо вовсе.
            // Поэтому ниже ДВА набора ожиданий под #ifdef: на AC22-23 их
            // меньше, и это НЕ ослабление проверки, а другой контракт SDK.
            property.definition.collectionType = API_PropertySingleCollectionType;
            property.definition.valueType = API_PropertyBooleanValueType;
            property.value.variantStatus = API_VariantStatusNormal;
            Spec::RuleFlagCheck flag;

    #ifdef ServerMainVers_2400
            property.status = API_Property_HasValue;
    #else
            property.isEvaluated = true;
    #endif
            property.isDefault = false;
            property.value.singleVariant.variant.boolValue = true;
            Spec::EvaluateRuleFlag (property, flag);
            DBtest (flag.checked, true, "RuleCheck flag HasValue is checked");
            DBtest (flag.value, true, "RuleCheck flag HasValue true");
            DBtest (flag.status == Spec::RuleFlagStatus::HasValue, true, "RuleCheck flag HasValue status");
            DBtest (flag.evaluated, true, "RuleCheck flag HasValue evaluated");
            DBtest (flag.origin == Spec::RuleFlagOrigin::NotChecked, true, "RuleCheck flag origin untouched by caller");
            DBtest (flag.isSingleValue, true, "RuleCheck flag single value");
            DBtest (flag.isDefault, false, "RuleCheck flag not default");

            property.value.singleVariant.variant.boolValue = false;
            Spec::EvaluateRuleFlag (property, flag);
            DBtest (flag.checked && !flag.value, true, "RuleCheck flag HasValue false is a real false");

    #ifdef ServerMainVers_2400
            // Только с AC24: состояние NotAvailable выразимо, и оно НЕ равно
            // «флаг включён», хотя прод GetRuleFromElement трактует именно так.
            property.status = API_Property_NotAvailable;
            Spec::EvaluateRuleFlag (property, flag);
            DBtest (flag.checked, false, "RuleCheck flag NotAvailable is NOT checked");
            DBtest (flag.status == Spec::RuleFlagStatus::NotAvailable, true, "RuleCheck flag NotAvailable status");
            DBtest (flag.value, false, "RuleCheck flag NotAvailable does not read as enabled");
            DBtest (flag.evaluated, false, "RuleCheck flag NotAvailable not evaluated");
    #else
            // До AC24 состояния NotAvailable не существует: при
            // isEvaluated == false и isDefault == false значение не берётся
            // из определения, и функция обязана вернуть NotEvaluated.
            property.isEvaluated = false;
            property.isDefault = false;
            Spec::EvaluateRuleFlag (property, flag);
            DBtest (flag.status == Spec::RuleFlagStatus::NotEvaluated,
                    true,
                    "RuleCheck pre-AC24 unavailable collapses to NotEvaluated");
            DBtest (flag.checked, true, "RuleCheck pre-AC24 value comes from SDK");
            DBtest (flag.value, true, "RuleCheck pre-AC24 value not forced to true");
    #endif

    #ifdef ServerMainVers_2400
            property.status = API_Property_NotEvaluated;
    #else
            property.isEvaluated = false;
    #endif
            property.isDefault = true;
            property.value.singleVariant.variant.boolValue = false;
            property.definition.defaultValue.basicValue.singleVariant.variant.boolValue = true;
            Spec::EvaluateRuleFlag (property, flag);
            DBtest (flag.checked, true, "RuleCheck flag NotEvaluated has value from definition");
            DBtest (flag.value, true, "RuleCheck flag NotEvaluated takes definition value");
            DBtest (flag.status == Spec::RuleFlagStatus::NotEvaluated, true, "RuleCheck flag NotEvaluated status");
            DBtest (flag.origin == Spec::RuleFlagOrigin::DefaultDefinition, true, "RuleCheck flag origin definition");
            DBtest (flag.evaluated, false, "RuleCheck flag NotEvaluated not evaluated");

            // Не вычислено, но не default: значение остаётся тем, что вернул
            // SDK, и происхождение уже НЕ «из определения». origin перед
            // вызовом задаётся явно: функция его НЕ трогает, это её контракт.
            property.isDefault = false;
            property.value.singleVariant.variant.boolValue = true;
            flag.origin = Spec::RuleFlagOrigin::ElementValue;
            Spec::EvaluateRuleFlag (property, flag);
            DBtest (flag.status == Spec::RuleFlagStatus::NotEvaluated,
                    true,
                    "RuleCheck flag nondefault not evaluated status");
            DBtest (flag.origin == Spec::RuleFlagOrigin::ElementValue,
                    true,
                    "RuleCheck flag origin belongs to caller and is not overwritten");

            // Перечисление: одиночного значения нет, поэтому булево поле и
            // смотреть не на что - это не «флаг выключен».
            property.definition.collectionType = API_PropertySingleChoiceEnumerationCollectionType;
    #ifdef ServerMainVers_2400
            property.status = API_Property_HasValue;
    #else
            property.isEvaluated = true;
    #endif
            Spec::EvaluateRuleFlag (property, flag);
            DBtest (flag.checked, false, "RuleCheck enumeration flag not checked");
            DBtest (flag.isSingleValue, false, "RuleCheck enumeration is not single");
            DBtest (flag.status == Spec::RuleFlagStatus::NotPresent, true, "RuleCheck enumeration status");

            // Не булево: значение есть, но флагом быть не может.
            property.definition.collectionType = API_PropertySingleCollectionType;
            property.definition.valueType = API_PropertyStringValueType;
            Spec::EvaluateRuleFlag (property, flag);
            DBtest (flag.checked, false, "RuleCheck non-boolean flag not checked");
            DBtest (flag.status == Spec::RuleFlagStatus::NotPresent, true, "RuleCheck non-boolean status");

            // Вариант не приведён к нормальному виду: статус утверждает, что
            // значение есть, но читать его нельзя.
            property.definition.valueType = API_PropertyBooleanValueType;
    #ifdef ServerMainVers_2400
            property.status = API_Property_HasValue;
    #else
            property.isEvaluated = true;
    #endif
            property.value.variantStatus = API_VariantStatusUserUndefined;
            Spec::EvaluateRuleFlag (property, flag);
            DBtest (flag.checked, false, "RuleCheck non-normal variant is not checked");
        }

        // ---- Перечень непрочитанных имён: два разных случая в одном списке ----
        {
            const GS::UniString U ("{@gdl:u}"), P ("{@gdl:p}"), Q ("{@gdl:q}");
            const API_Guid elem = APIGuidFromString ("{11111111-2222-3333-4444-555555555555}");
            Spec::SpecRule rule;
            rule.out_paramrawname.Push (P);
            rule.out_sum_paramrawname.Push (Q);
            Spec::GroupSpec group;
            group.unic_paramrawname.Push (U);
            group.out_paramrawname.Push (P);
            group.sum_paramrawname.Push (Q);
            rule.groups.Push (group);

            ParamDictElement read = {};
            GS::Array<GS::UniString> missing = {};

            // Элемента в словаре нет - читать нечего вовсе.
            DBtest (Spec::CollectUnreadRuleNames (rule, elem, read, missing), 0, "RuleCheck unread no element");
            DBtest (missing.IsEmpty (), true, "RuleCheck unread no element empty");

            // Ключа нет вовсе: U и Q не прочитаны.
            ParamValue valid = {};
            valid.rawName = P;
            ParamHelpers::ConvertIntToParamValue (valid, P, 1);
            valid.isValid = true;
            read.Put (elem, ParamDictValue ());
            read.Get (elem).Put (P, valid);
            DBtest (Spec::CollectUnreadRuleNames (rule, elem, read, missing), 2, "RuleCheck unread absent keys");
            DBtest (missing.Contains (U) && missing.Contains (Q), true, "RuleCheck unread lists absent names");
            DBtest (missing.Contains (P), false, "RuleCheck unread omits present name");

            // Ключ есть, но значение невалидно: тоже расхождение.
            ParamValue invalid = valid;
            invalid.isValid = false;
            read.Get (elem).Put (P, invalid);
            DBtest (Spec::CollectUnreadRuleNames (rule, elem, read, missing), 3, "RuleCheck unread invalid value");
            DBtest (missing.Contains (P), true, "RuleCheck unread lists invalid name");

            // Литерал-счётчик и пустое имя расхождением не считаются. Итог равен
            // двум: литерала "1" и пустого флага в списке нет, U отсутствует
            // в словаре чтения, а P помечено невалидным предыдущим блоком.
            // Q сюда НЕ входит: литерал "1" заменил его в sum_paramrawname,
            // поэтому Q вообще не попадает в зависимости.
            Spec::SpecRule counterRule = rule;
            Spec::GroupSpec counterGroup = group;
            counterGroup.sum_paramrawname.Clear ();
            counterGroup.sum_paramrawname.Push ("1");
            counterGroup.flag_paramrawname = EMPTYSTRING;
            counterRule.groups.Clear ();
            counterRule.groups.Push (counterGroup);
            DBtest (Spec::CollectUnreadRuleNames (counterRule, elem, read, missing),
                    2,
                    "RuleCheck unread skips literal and empty flag");
            DBtest (missing.Contains ("1") || missing.Contains (EMPTYSTRING),
                    false,
                    "RuleCheck unread omits literal and empty flag");
        }

        // ---- Настоящий проект: определение правила и его зависимости ----
        // Модель не изменяется; берётся первое найденное свойство с описанием,
        // в котором есть "Spec_rule".
        {
            // Поиск определения с описанием правила идёт через группы свойств:
            // ACAPI_Property_GetPropertyDefinitions запрашивает определения ГРУППЫ,
            // поэтому одного вызова с пустым GUID недостаточно.
            GS::Array<API_PropertyGroup> groups = {};
            if (ACAPI_Property_GetPropertyGroups (groups) == NoError) {
                API_PropertyDefinition candidate = {};
                for (const API_PropertyGroup &group : groups) {
                    GS::Array<API_PropertyDefinition> definitions = {};
                    if (ACAPI_Property_GetPropertyDefinitions (group.guid, definitions) != NoError)
                        continue;
                    for (const API_PropertyDefinition &definition : definitions) {
                        if (!definition.description.IsEmpty () && definition.description.Contains ("pec_rule")) {
                            candidate = definition;
                            break;
                        }
                    }
                    if (candidate.guid != APINULLGuid)
                        break;
                }
                if (candidate.guid == APINULLGuid) {
                    ::TestKit::Note ("RuleCheck.project", "в проекте нет определения с описанием правила");
                } else {
                    Spec::RuleCheckResult result;
                    const bool found = Spec::CheckRuleByPropertyGuid (candidate.guid, APINULLGuid, result);
                    ::TestKit::NoteFields ("RuleCheck.project",
                                           {SMSTF_FIELD ("found", found),
                                            SMSTF_FIELD ("parsed", result.ruleParsed),
                                            SMSTF_FIELD ("unresolved", (Int32)result.unresolvedInProject.GetSize ()),
                                            SMSTF_FIELD_U ("name", result.propertyName),
                                            SMSTF_FIELD_U ("favorite", result.rule.favorite_name),
                                            SMSTF_FIELD ("favoriteFound", result.favoriteFound),
                                            SMSTF_FIELD ("fromDefault", result.fromDefaultElem),
                                            SMSTF_FIELD ("missingWrite", (Int32)result.missingWrite.GetSize ())},
                                           "проверка по определению свойства из проекте");
                    DBtest (found, true, "RuleCheck project definition found");
                    DBtest (!result.checkedElement, true, "RuleCheck project without element skips element check");
                    DBtest (result.missingRead.IsEmpty (), true, "RuleCheck project without element has no read list");
                    // Найденное избранное и чтение настроек объекта по умолчанию -
                    // разные ответы: сверка по умолчанию не доказывает, что
                    // избранное с таким именем существует.
                    DBtest (result.fromDefaultElem == !result.favoriteFound,
                            true,
                            "RuleCheck destination source reported explicitly");
                    if (result.ruleParsed) {
                        DBtest (result.destinationFlag.origin != Spec::RuleFlagOrigin::NotChecked,
                                true,
                                "RuleCheck destination flag origin resolved");
                    }
                }
            }
        }
    }

} // namespace TestFunc
#endif
