//------------ kuvbur 2022 ------------
#pragma once
#if !defined(SPECPLANNING_HPP)
    #define SPECPLANNING_HPP

    #include "Spec.hpp"

// Вклад одного источника хранит прочитанные значения и ошибки; итоговая строка
// объединяет вклады с одинаковым ключом. Вклад живёт один проход обработки
// источника и не хранится в отдельном списке.
namespace Spec {
    // Поле, которое не удалось прочитать, вместе с признанием того, является ли
    // это ошибкой элемента. isError = false — отказ не считается ошибкой
    // (чтение из состава конструкции либо продолжение строки GDL-массива).
    struct MissingField {
        GS::UniString rawname = EMPTYSTRING;
        bool isError = false;
    };

    // Статус различает исключённый источник, пустой и неполный вклад.
    enum class ContributionStatus : unsigned char {
        Excluded, // Источник не вносит вклад: не прошёл флаг/уникальность
        Empty,    // Включён, но ни одного значения прочитать не удалось
        Partial,  // Включён, прочитана часть слотов (Push только при успехе)
        Complete  // Включён, прочитаны все ожидаемые слоты
    };

    // Один вклад источника в итоговую строку.
    struct RuleContribution {
        // --- источник (кто вносит) ---
        API_Guid source = APINULLGuid; // Элемент-источник
        UInt32 groupIndex = 0;         // Индекс группы в rule.groups

        // --- статус включения ---
        ContributionStatus status = ContributionStatus::Excluded;
        bool isIncluded = false; // Участвует ли вклад в агрегате (status != Excluded)

        // --- значения по слотам ---
        // Порядок элементов соответствует порядку binding.outSlots/sumSlots
        // группы, то есть порядку, который читает цикл. Длина может отличаться
        // от объявленной схемы: Push выполняется только при успешном чтении.
        GS::Array<ParamValue> outParam = {};    // Значения выходных слотов
        GS::Array<ParamValue> outSumParam = {}; // Значения суммируемых слотов

        // --- ошибки чтения, влияющие на агрегирование ---
        // Признак isError хранится НА КАЖДОЕ поле, а не общим флагом: отчёт
        // различает «отказ с fromMaterial» (ошибка элемента) и «отказ в строке
        // >1 массива GDL» (тихо), и поле-источник ошибки нужно знать, чтобы
        // пометить элемент и не повторить сообщение.
        GS::Array<MissingField> missingOut = {};  // Поля выхода, не прочитанные
        GS::Array<MissingField> missingSum = {};  // Поля суммы, не прочитанные
        GS::Array<MissingField> missingUnic = {}; // Уникальные параметры, не прочитанные
        bool hasReadError = false;                // Было ли неуспешное чтение с isError

        // --- ключ и производные величины ---
        GS::UniString key;        // Ключ строки: сцепка значений уникальных параметров
        GS::UniString keyOut;     // Склейка выходных значений (для out_param)
        bool hasKey = false;      // Ключ собран (hasUnic == true)
        bool outputsRead = false; // Выходные слоты прочитаны

        // Признаки полноты вычисляются из фактически прочитанных слотов.
        bool hasOutSlots = false;
        bool hasSumSlots = false;
        bool isComplete = false; // hasOutSlots && hasSumSlots

        // Константа 1 для суммируемого слота хранится в SlotBinding.
    };

    // Для каждого подходящего источника читаются флаг, уникальные параметры,
    // ключ и суммы. Выходные слоты читаются отдельно только у первого
    // представителя ключа: чтение их у остальных изменило бы состав ошибок.
    // Порядок чтения: флаг -> уникальные -> суммы -> выход.
    // reader принадлежит вызывающему; счётчики notFoundParame/notFoundUnic
    // относятся ко всему правилу, вклад их не изменяет.
    RuleContribution BuildContribution (const API_Guid &elemguid,
                                        UInt32 groupIndex,
                                        const GroupSpec &group,
                                        const GroupSlotBinding &binding,
                                        const SpecValueReader &reader,
                                        const ParamDict &notFoundParame,
                                        const ParamDict &notFoundUnic);

    // Читает выходные слоты и ключ выхода только когда ключа ещё
    // нет в словаре элементов.
    void ReadContributionOutputs (const API_Guid &elemguid,
                                  const GroupSpec &group,
                                  const GroupSlotBinding &binding,
                                  const SpecValueReader &reader,
                                  FormatString &fstr,
                                  RuleContribution &contribution);

    // Определяет статус по фактическим размерам вклада и схеме.
    ContributionStatus ClassifyContribution (const RuleContribution &contribution,
                                             UInt32 schemaOutSlots,
                                             UInt32 schemaSumSlots);

    // ---- Раскладка вклада в итоговую строку ----

    // Что произошло с ключом при добавлении вклада.
    enum class RowAddition {
        Created,       // Ключа не было: строка создана и добавлена
        Merged,        // Ключ был: вклад слит с существующей строкой
        SchemaMismatch // Первый представитель не наполнил схему: строка НЕ добавлена
    };

    // Суммирует вклад в существующую строку: только первые MIN(длин)
    // суммируемых слотов с isValid у обеих сторон. Остальные слоты не меняет.
    void SumContributionIntoRow (Element &row, const RuleContribution &contribution);

    // -----------------------------------------------------------------------------
    // Сверяет существующие объекты со строками в порядке rule.runState.exsist_elements.
    // Ключ выхода собирается из выходных полей. Удаление проверяется в порядке:
    // нет прочитанных полей -> нет ключа выхода -> строка уже израсходована.
    // Затем сравниваются выходные и суммарные поля; GUID-поле читается, но
    // не влияет на решение об обновлении. Изменённая строка
    // попадает в elementsMod. Сопоставленная строка удаляется из elements,
    // объект отмечается обработанным; необработанные объекты удаляются.
    // При полном совпадении строка также удаляется из elements.
    // -----------------------------------------------------------------------------
    void ReconcileExistingRows (const SpecRule &rule,
                                const SpecValueReader &reader,
                                const FormatString &fstr,
                                const GS::HashTable<GS::UniString, GS::UniString> &outParam,
                                ElementDict &elements,
                                ElementDict &elementsMod,
                                GS::Array<API_Guid> &elementsDelete,
                                SpecChangePlan *plan = nullptr);

    // При слиянии источник добавляется до суммирования. При создании связь
    // keyOut -> key записывается до проверки схемы (даже если строка отвергнута);
    // выходные значения первого источника используются без повторного чтения.
    RowAddition AddContributionToRow (ElementDict &rows,
                                      const RuleContribution &contribution,
                                      const SpecRule &rule,
                                      UInt32 schemaOutSlots,
                                      UInt32 schemaSumSlots,
                                      GS::HashTable<GS::UniString, GS::UniString> &outParam);
} // namespace Spec
#endif // SPECPLANNING_HPP
