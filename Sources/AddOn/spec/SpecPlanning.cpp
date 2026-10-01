//------------ kuvbur 2022 ------------
#include "SpecPlanning.hpp"

// Реализация вклада (R6.2). Тела циклов перенесены из PlanRuleRows дословно:
// порядок чтения (флаг -> уникальные параметры -> суммы -> выход), условия
// is_error, формат сообщений и ключей не менялись. Перенесено ТОЛЬКО описание
// вклада; решение о том, как вклад ложится в агрегат, осталось в PlanRuleRows
// (это R6.3).

namespace Spec {
    namespace {
        // Признак «чтение не удалось и это ошибка». ПОРЯДОК ИМЕЕТ ЗНАЧЕНИЕ и
        // совпадает с исходным циклом: признак fromMaterial даёт отказ по
        // умолчанию, но fromGDLArray ПЕРЕКРЫВАЕТ его — строка 1 массива GDL
        // является ошибкой, а строки со второй и далее проходят молча
        // (R5.5, S13). Обратный порядок здесь ломает S13: при fromMaterial и
        // array_row_start > 1 отказ стал бы ошибкой элемента.
        inline bool ReadIsError (const ParamValue &pvalue, bool fromMaterial) {
            bool is_error = !fromMaterial;
            if (pvalue.fromGDLArray)
                is_error = pvalue.val.array_row_start == 1;
            return is_error;
        }
    } // namespace

    RuleContribution BuildContribution (const API_Guid &elemguid,
                                        UInt32 groupIndex,
                                        const GroupSpec &group,
                                        const GroupSlotBinding &binding,
                                        const SpecValueReader &reader,
                                        const ParamDict &notFoundParame,
                                        const ParamDict &notFoundUnic) {
        RuleContribution contribution = {};
        contribution.source = elemguid;
        contribution.groupIndex = groupIndex;

        // --- флаг группы ---
        // Прежде это был блок прямо в цикле. Флаг читается ДО проверки
        // уникальных параметров, и выключенный источник не создаёт вклада
        // вовсе — поэтому default статуса (Excluded) здесь верен.
        bool flag = true;
        if (!group.flag_paramrawname.IsEmpty ()) {
            ParamValue pvalue = {};
            if (reader.Read (elemguid, group.flag_paramrawname, pvalue, group.n_layer)) {
                flag = pvalue.val.boolValue;
            }
        }
        if (!flag) {
            contribution.status = ContributionStatus::Excluded;
            return contribution;
        }

        // --- уникальные параметры и ключ строки ---
        // Ключ склеивается ДАЖЕ при неудачном чтении: val пустой, но
        // ATSIGN добавляется, как и прежде. Это поведение сохранено
        // намеренно — от него зависит состав отвергнутых ключей.
        bool hasunic = true;
        GS::UniString key;
        for (const GS::UniString &rawname : group.unic_paramrawname) {
            ParamValue pvalue = {};
            if (!reader.Read (elemguid, rawname, pvalue, group.n_layer)) {
                hasunic = false;
                MissingField field = {};
                field.rawname = rawname;
                field.isError = ReadIsError (pvalue, group.fromMaterial);
                contribution.missingUnic.Push (field);
                if (field.isError)
                    contribution.hasReadError = true;
                // Счётчик notFoundUnic — на всё правило; вклад только читает
                // его, чтобы вызывающая не повторяла сообщение для того же поля.
                (void)notFoundUnic;
            }
            GS::UniString val = pvalue.val.uniStringValue;
            val.ReplaceAll ("  ", SPACESTRING);
            val.Trim ();
            key = key + ATSIGN + val;
        }
        contribution.key = key;
        contribution.hasKey = hasunic;
        if (!hasunic) {
            contribution.status = ContributionStatus::Excluded;
            return contribution;
        }
        contribution.isIncluded = true;

        // --- суммируемые слоты ---
        for (const SlotBinding &slot : binding.sumSlots) {
            const GS::UniString &rawname = *slot.rawname;
            ParamValue pvalue = {};
            if (slot.isSumLiteral) {
                ParamHelpers::ConvertIntToParamValue (pvalue, rawname, 1);
                contribution.outSumParam.Push (pvalue);
            } else {
                if (reader.Read (elemguid, rawname, pvalue, group.n_layer)) {
                    contribution.outSumParam.Push (pvalue);
                } else {
                    MissingField field = {};
                    field.rawname = rawname;
                    field.isError = ReadIsError (pvalue, group.fromMaterial);
                    contribution.missingSum.Push (field);
                    if (field.isError)
                        contribution.hasReadError = true;
                    if (notFoundParame.ContainsKey ("sum:" + rawname))
                        (void)0; // счётчик уже содержит это поле: сообщение не повторяется
                }
            }
        }

        // Выходные слоты здесь НЕ читаются: для этого нужно знать, первый ли
        // это представитель ключа, а словаря элементов у вклада нет. См.
        // ReadContributionOutputs.
        contribution.hasSumSlots = !contribution.outSumParam.IsEmpty ();
        contribution.status = ContributionStatus::Partial;
        return contribution;
    }

    void ReadContributionOutputs (const API_Guid &elemguid,
                                  const GroupSpec &group,
                                  const GroupSlotBinding &binding,
                                  const SpecValueReader &reader,
                                  FormatString &fstr,
                                  RuleContribution &contribution) {
        GS::UniString keyOut;
        for (const SlotBinding &slot : binding.outSlots) {
            const GS::UniString &rawname = *slot.rawname;
            ParamValue pvalue = {};
            if (reader.Read (elemguid, rawname, pvalue, group.n_layer)) {
                contribution.outParam.Push (pvalue);
                keyOut = keyOut + ATSIGN + ParamHelpers::ToString (pvalue, fstr);
            } else {
                MissingField field = {};
                field.rawname = rawname;
                field.isError = ReadIsError (pvalue, group.fromMaterial);
                contribution.missingOut.Push (field);
                if (field.isError)
                    contribution.hasReadError = true;
            }
        }
        contribution.keyOut = keyOut;
        contribution.outputsRead = true;
        contribution.hasOutSlots = !contribution.outParam.IsEmpty ();
        contribution.isComplete = contribution.hasOutSlots && contribution.hasSumSlots;
    }

    ContributionStatus ClassifyContribution (const RuleContribution &contribution,
                                             UInt32 schemaOutSlots,
                                             UInt32 schemaSumSlots) {
        if (!contribution.isIncluded)
            return ContributionStatus::Excluded;
        if (!contribution.outputsRead)
            return contribution.hasSumSlots ? ContributionStatus::Partial : ContributionStatus::Empty;
        // Ни одного прочитанного значения — вклад пустой, а не частичный:
        // различать это важно, потому что в агрегат пустой вклад не
        // добавляет ни источника, ни нулевого слота.
        if (contribution.outParam.IsEmpty () && contribution.outSumParam.IsEmpty ())
            return ContributionStatus::Empty;
        // Сравнение со схемой — предупреждение, а НЕ замена проверки в цикле:
        // фактическое число слотов известно лишь после чтения.
        if (contribution.outParam.GetSize () == schemaOutSlots && contribution.outSumParam.GetSize () == schemaSumSlots)
            return ContributionStatus::Complete;
        return ContributionStatus::Partial;
    }

    void SumContributionIntoRow (Element &row, const RuleContribution &contribution) {
        // Перенос дословно из цикла PlanRuleRows (R6.3). Именно эта арифметика
        // и её поведение на неполных массивах закреплены тестом, а не
        // «улучшены».
        UInt32 nsumm = row.out_sum_param.GetSize ();
        if (nsumm != contribution.outSumParam.GetSize ()) {
            nsumm = nsumm < contribution.outSumParam.GetSize () ? nsumm : contribution.outSumParam.GetSize ();
        }
        for (UInt32 j = 0; j < nsumm; j++) {
            if (row.out_sum_param[j].isValid && contribution.outSumParam[j].isValid)
                row.out_sum_param[j].val = row.out_sum_param[j].val + contribution.outSumParam[j].val;
        }
    }

    RowAddition AddContributionToRow (ElementDict &rows,
                                      const RuleContribution &contribution,
                                      const SpecRule &rule,
                                      UInt32 schemaOutSlots,
                                      UInt32 schemaSumSlots,
                                      GS::HashTable<GS::UniString, GS::UniString> &outParam) {
        if (rows.ContainsKey (contribution.key)) {
            Element &exsists_element = rows.Get (contribution.key);
            // Порядок 2: источник дописывается ДО суммирования.
            exsists_element.elements.Push (contribution.source);
            SumContributionIntoRow (exsists_element, contribution);
            return RowAddition::Merged;
        }

        // Порядок 1: запись в outParam делается ДО проверки схемы, поэтому
        // ключ попадает в словарь даже для строки, которая будет отброшена.
        if (!outParam.ContainsKey (contribution.keyOut))
            outParam.Add (contribution.keyOut, contribution.key);

        Element row = {};
        row.out_param = contribution.outParam;
        row.out_sum_param = contribution.outSumParam;
        if (!OutSlotsMatchSchema (row, schemaOutSlots, schemaSumSlots))
            return RowAddition::SchemaMismatch;

        // Признаки правила копируются как есть — это данные первого
        // представителя, они не вычисляются заново для последующих вкладов.
        row.out_sum_paramrawname = rule.out_sum_paramrawname;
        row.out_paramrawname = rule.out_paramrawname;
        row.subguid_paramrawname = rule.destinationParamGuidName;
        row.subguid_rulevalue = rule.subguid_rulevalue;
        row.subguid_rulename = rule.subguid_rulename;
        row.favorite_name = rule.favorite_name;
        row.elements.Push (contribution.source);
        rows.Add (contribution.key, row);
        return RowAddition::Created;
    }
} // namespace Spec
