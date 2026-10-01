//------------ kuvbur 2022 ------------
#include "SpecPlanning.hpp"

// Чтение вклада и сверка строк спецификации.
namespace Spec {
    namespace {
        // fromGDLArray имеет приоритет над fromMaterial: отказ в первой строке
        // массива считается ошибкой источника, в последующих — нет.
        inline bool ReadIsError (const ParamValue &pvalue, bool fromMaterial) {
            bool is_error = !fromMaterial;
            if (pvalue.fromGDLArray)
                is_error = pvalue.val.array_row_start == 1;
            return is_error;
        }
    } // namespace

    // Формирует первую часть вклада одного источника: проверяет флаг группы,
    // собирает ключ строки и читает суммы. Не читает выходные поля и не решает,
    // создавать ли строку; непрочитанные поля возвращает во вкладе вызывающему.
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
        // Флаг читается до уникальных параметров; выключенный источник исключается.
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
        // Даже при неудачном чтении в ключ добавляется ATSIGN с пустым значением.
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

    // --------------------------------------------------------------------
    // Дополняет вклад выходными значениями и ключом выхода. Вызывающий делает
    // это только для первого источника с данным ключом строки; ошибки чтения
    // остаются во вкладе, а решение о сообщении принимает PlanRuleRows.
    // --------------------------------------------------------------------
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

    // Определяет статус вклада по включению источника, чтению выходных слотов
    // и фактическому числу прочитанных слотов. Не меняет вклад и не заменяет
    // проверку схемы перед добавлением строки.
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
        // Сравнение со схемой даёт статус вклада, но не отменяет проверку в
        // цикле: фактическое число слотов известно лишь после чтения.
        if (contribution.outParam.GetSize () == schemaOutSlots && contribution.outSumParam.GetSize () == schemaSumSlots)
            return ContributionStatus::Complete;
        return ContributionStatus::Partial;
    }

    // Сборка общей схемы слотов строки. Внутренняя функция модуля: наружу она не
    // нужна, потому что схема всегда строится сама при создании строки.
    namespace {
        // Схема первого вклада: выходные слоты, затем суммы. Имена берутся
        // из правила по позиции, при отсутствии имени используется EMPTYSTRING.
        void BuildOutputSlots (Element &row, const SpecRule &rule, const RuleContribution &contribution) {
            row.out_slots.Clear ();
            for (UInt32 i = 0; i < contribution.outParam.GetSize (); i++) {
                OutputSlot slot = {};
                slot.rawname = i < rule.out_paramrawname.GetSize () ? rule.out_paramrawname[i] : EMPTYSTRING;
                slot.value = contribution.outParam[i];
                slot.isSum = false;
                row.out_slots.Push (slot);
            }
            for (UInt32 i = 0; i < contribution.outSumParam.GetSize (); i++) {
                OutputSlot slot = {};
                slot.rawname = i < rule.out_sum_paramrawname.GetSize () ? rule.out_sum_paramrawname[i] : EMPTYSTRING;
                slot.value = contribution.outSumParam[i];
                slot.isSum = true;
                row.out_slots.Push (slot);
            }
        }
    } // namespace

    // --------------------------------------------------------------------
    // Добавляет суммы очередного источника в уже существующую строку.
    // При разных длинах складывает только общую начальную часть суммарных
    // слотов; невалидные пары и оставшиеся слоты не меняет.
    // --------------------------------------------------------------------
    void SumContributionIntoRow (Element &row, const RuleContribution &contribution) {
        // Суммируются первые общие суммарные слоты; при неполном вкладе
        // отсутствующие слоты не создаются и оставшиеся значения не меняются.
        UInt32 accumulated = 0;
        for (const OutputSlot &slot : row.out_slots)
            if (slot.isSum)
                ++accumulated;
        const UInt32 incoming = contribution.outSumParam.GetSize ();
        const UInt32 nsumm = accumulated < incoming ? accumulated : incoming;
        // Суммы идут после выходных слотов; смещение зависит от числа
        // накопленных сумм, а не от числа слотов очередного вклада.
        const UInt32 firstSumSlot = row.out_slots.GetSize () - accumulated;
        for (UInt32 j = 0; j < nsumm; j++) {
            OutputSlot &slot = row.out_slots[firstSumSlot + j];
            if (slot.value.isValid && contribution.outSumParam[j].isValid)
                slot.value.val = slot.value.val + contribution.outSumParam[j].val;
        }
    }

    // Добавляет вклад к строке с тем же ключом либо создаёт новую строку.
    // При создании проверяет схему; при несовпадении строка не попадает в rows,
    // но связь keyOut -> key уже могла попасть в outParam. Возвращает исход.
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
        // Схема строится до проверки соответствия; ключ уже записан в outParam.
        BuildOutputSlots (row, rule, contribution);
        if (!OutSlotsMatchSchema (row, schemaOutSlots, schemaSumSlots))
            return RowAddition::SchemaMismatch;
        // Значения первого источника берутся из уже собранной схемы.
        row.subguid_paramrawname = rule.runState.destinationParamGuidName;
        row.subguid_rulevalue = rule.subguid_rulevalue;
        row.subguid_rulename = rule.subguid_rulename;
        row.favorite_name = rule.favorite_name;
        row.elements.Push (contribution.source);
        rows.Add (contribution.key, row);
        return RowAddition::Created;
    }

    // --------------------------------------------------------------------
    // Сопоставляет рассчитанные строки с ранее размещёнными объектами правила.
    // Сопоставленные строки вынимает из elements; изменённые добавляет в
    // elementsMod, лишние GUID — в elementsDelete, остальные строки оставляет
    // для создания. plan, если передан, наблюдает те же решения, но не управляет ими.
    // --------------------------------------------------------------------
    void ReconcileExistingRows (const SpecRule &rule,
                                const SpecValueReader &reader,
                                const FormatString &fstr,
                                const GS::HashTable<GS::UniString, GS::UniString> &outParam,
                                ElementDict &elements,
                                ElementDict &elementsMod,
                                GS::Array<API_Guid> &elementsDelete,
                                SpecChangePlan *plan) {
        // План лишь отражает решения сверки; реальные изменения списков
        // выполняются независимо от того, передан ли plan.
        if (plan)
            plan->deleteOld = 1;
        // Отклонения в сверке сообщаются через msg_rep.
        auto report = [&rule] (const GS::UniString &text) { msg_rep ("Spec", text, NoError, APINULLGuid); };

        UnicGuid guids = {};
        for (const API_Guid &elemguid : rule.runState.exsist_elements) {
            GS::UniString key_out;
            bool hasunic = true;
            // Принадлежность субэлемента к группе определим по ключу - сцепке значений уникальных параметров
            for (const GS::UniString &rawname : rule.out_paramrawname) {
                ParamValue pvalue = {};
                if (!reader.Read (elemguid, rawname, pvalue, 0))
                    hasunic = false;
                key_out = key_out + ATSIGN + ParamHelpers::ToString (pvalue, fstr);
            }
            if (!hasunic) {
                report ("!hasunic " + key_out);
                elementsDelete.Push (elemguid);
                if (plan)
                    plan->removals.Push ({elemguid, SpecChangePlan::DeleteReason::NoUniqueFields});
                guids.Add (elemguid, false);
                continue;
            }
            if (!outParam.ContainsKey (key_out)) {
                report ("out_param.ContainsKey (key_out) " + key_out);
                elementsDelete.Push (elemguid);
                if (plan)
                    plan->removals.Push ({elemguid, SpecChangePlan::DeleteReason::NoNewRow});
                guids.Add (elemguid, false);
                continue;
            }
            GS::UniString key = outParam.Get (key_out);
            if (!elements.ContainsKey (key)) {
                report ("!elements.ContainsKey (key) " + key_out);
                elementsDelete.Push (elemguid);
                if (plan)
                    plan->removals.Push ({elemguid, SpecChangePlan::DeleteReason::RowAlreadyClaimed});
                guids.Add (elemguid, false);
                continue;
            }
            // Нашли в создаваемых элементах уже существующую комбинацию значений
            // Такой элемент можно модифицировать
            Element el = elements.Get (key);
            if (elementsMod.ContainsKey (key)) {
                report ("elementsMod.ContainsKey (key) " + key_out);
                elementsDelete.Push (elemguid);
                guids.Add (elemguid, false);
                continue;
            }
            bool flag_change = false;
            // Сравниваются значения и имена слотов самой строки: сначала
            // выходные, затем суммарные.
            for (const OutputSlot &slot : el.out_slots) {
                if (slot.isSum)
                    continue;
                const GS::UniString &rawname = slot.rawname;
                ParamValue pvalue = {};
                ParamValue elvalue = slot.value;
                if (!reader.Read (elemguid, rawname, pvalue, 0)) {
                    report ("Param not valid: " + rawname);
                    flag_change = true;
                }
                elvalue.val.formatstring = pvalue.val.formatstring;
                ParamHelpers::ConvertByFormatString (elvalue);
                if (elvalue != pvalue) {
                    GS::UniString old_s = "old ";
                    GS::UniString new_s;
                    if (pvalue.type != API_PropertyStringValueType) {
                        old_s += FormatStringFunc::NumToString (pvalue.val.doubleValue, pvalue.val.formatstring);
                        new_s += FormatStringFunc::NumToString (elvalue.val.doubleValue, pvalue.val.formatstring);
                    } else {
                        old_s += pvalue.val.uniStringValue;
                        new_s += elvalue.val.uniStringValue;
                    }
                    new_s += " new";
                    report ("Param diff: " + rawname + SPACESTRING + old_s + " <=> " + new_s);
                    flag_change = true;
                }
            }
            for (const OutputSlot &slot : el.out_slots) {
                if (!slot.isSum)
                    continue;
                const GS::UniString &rawname = slot.rawname;
                ParamValue pvalue = {};
                ParamValue elvalue = slot.value;
                if (!reader.Read (elemguid, rawname, pvalue, 0)) {
                    report ("Param not valid: " + rawname);
                    flag_change = true;
                }
                elvalue.val.formatstring = pvalue.val.formatstring;
                ParamHelpers::ConvertByFormatString (elvalue);
                if (elvalue != pvalue) {
                    GS::UniString old_s = "old ";
                    GS::UniString new_s;
                    if (pvalue.type != API_PropertyStringValueType) {
                        old_s += FormatStringFunc::NumToString (pvalue.val.doubleValue, pvalue.val.formatstring);
                        new_s += FormatStringFunc::NumToString (elvalue.val.doubleValue, pvalue.val.formatstring);
                    } else {
                        old_s += pvalue.val.uniStringValue;
                        new_s += elvalue.val.uniStringValue;
                    }
                    report ("Sum diff: " + rawname + SPACESTRING + old_s + " <=> " + new_s);
                    flag_change = true;
                }
            }

            // GUID-связь читается из разрешённого свойства правила.
            GS::UniString rawname = rule.runState.destinationParamGuidName;
            if (!rawname.IsEmpty ()) {
                ParamValue pvalue = {};
                if (!reader.Read (elemguid, rawname, pvalue, 0)) {
                    report ("Param not valid: " + rawname);
                    flag_change = true;
                }
                GS::UniString instring = APIGuidToString (el.elements[0]);
                for (UInt32 k = 1; k < el.elements.GetSize (); k++) {
                    instring = instring + SEMICOLON + APIGuid2GSGuid (el.elements[k]).ToUniString ();
                }
            }
            // Если нашли изменения - добавим в список модифицированных
            if (flag_change) {
                el.exs_guid = elemguid;
                elementsMod.Add (key, el);
                if (plan)
                    plan->update.Add (key, el);
            } else if (plan) {
                // Для неизменённой строки хранится только счётчик.
                plan->unchanged += 1;
            }
            // Удаляем из списка новых элементов и добавляем в словарь обработанных
            elements.Delete (key);
            guids.Add (elemguid, true);
        }
        // Удаляем все существующие устаревшие элементы
        for (const API_Guid &elemguid : rule.runState.exsist_elements) {
            if (!guids.ContainsKey (elemguid)) {
                elementsDelete.Push (elemguid);
                if (plan)
                    plan->removals.Push ({elemguid, SpecChangePlan::DeleteReason::Obsolete});
            }
        }
        // Оставшиеся в elements строки — те, что не сопоставлены ни одному
        // существующему объекту; их создание. Порядок — порядок словаря строк.
        if (plan) {
            for (auto &cIt : elements) {
#ifdef ServerMainVers_2800
                const GS::UniString &rowKey = cIt.key;
                const Element &row = cIt.value;
#else
                const GS::UniString &rowKey = *cIt.key;
                const Element &row = *cIt.value;
#endif
                if (elementsMod.ContainsKey (rowKey))
                    continue;
                if (row.elements.IsEmpty ())
                    continue;
                plan->create.Push (row.elements[0]);
            }
        }
    }

} // namespace Spec
