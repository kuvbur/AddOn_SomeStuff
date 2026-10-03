//------------ kuvbur 2022 ------------
#include "ACAPinc.h"
#ifdef ServerMainVers_2300
    #include <API_Guid.hpp>
    #include <APIdefs_Elements.h>
    #include <APIdefs_Environment.h>
    #include <APIdefs_ErrorCodes.h>
    #include <APIdefs_Interface.h>
    #include <APIdefs_Properties.h>
    #include <Array.hpp>
    #include <CH.hpp>
    #include <ctime>
    #include <Definitions.hpp>
    #include <HashTable.hpp>
    #include <map>
    #include <RS.hpp>
    #include <string>
    #include <UniString.hpp>
    #include <unordered_map>

    #include "api_headers/ResourceIds.hpp"

    #include "CommonFunction.hpp"
    #include "dialogs/DG4rule.hpp"
    #include "dialogs/SyncSettings.hpp"
    #include "Helpers.hpp"
    #include "Propertycache.hpp"
    #include "ReNum.hpp"
    #include "Sync.hpp"

// -----------------------------------------------------------------------------------------------------------------------
// 1. Получаем список объектов, в свойствах которых ищем
//		Флаг включения нумерации в формате
//					Renum_flag{*имя свойства с правилом*}
//					Renum_flag{*имя свойства с правилом*; NULL}
//					Renum_flag{*имя свойства с правилом*; ALLNULL}
//					Renum_flag{*имя свойства с правилом*; SPACE}
//					Renum_flag{*имя свойства с правилом*; ALLSPACE}
//					Тип данных свойства-флага: Набор параметров с вариантами "Включить",
//"Исключить", "Не менять" 		Правило нумерации в одном из форматов
// Renum{*имя свойства-критерия*} 					Renum{*имя свойства-критерия*; *имя
// свойства-разбивки*}
// 2. Записываем для каждого элемента в структуру свойства, участвующие в правиле
// 3. Откидываем все с вфключенным флагом
// 4. По количеству уникальных имён свойств-правил, взятых из Renum_flag{*имя свойства с правилом*}, разбиваем элементы
// -----------------------------------------------------------------------------------------------------------------------
// Основная функция перенумерации выбранных элементов
// Алгоритм:
// 1. Получаем выбранные элементы
// 2. Ищем у них свойства с флагом Renum_flag{...} (указывают на правило нумерации)
// 3. Собираем данные из свойств (позиция, флаг, критерий, разбивка)
// 4. Показываем диалог выбора правил (RenumDG)
// 5. Читаем значения свойств
// 6. Распределяем позиции согласно правилам
// 7. Записываем новые позиции
// Все сообщения копятся в runResult и показываются ОДНИМ окном в конце
// (ShowRenumResult) - прежде каждая точка отказа открыла своё всплывающее окно.
GSErrCode ReNumSelected (SyncSettings &syncSettings) {
    GS::UniString funcname ("Numbering");
    GS::Int32 nPhase = 1;
    GSErrCode err = NoError;
    ProcessWindowGuard pwGuard (funcname, nPhase);
    clock_t start, finish;
    double duration;
    start = clock ();
    const Int32 iseng = ID_ADDON_STRINGS + isEng ();
    GS::UniString undoString = RSGetIndString (iseng, UndoReNumId, ACAPI_GetOwnResModule ());

    // Получаем выбранные элементы (только видимые, только из модели)
    GS::Array<API_Guid> guidArray = GetSelectedElements (true, false, syncSettings, false, false, false, true);
    if (guidArray.IsEmpty ())
        return NoError;
    // Если выбран только один элемент - правило берётся из него одного
    bool rule_from_one = (guidArray.GetSize () == 1);
    // Таблица определений свойств-правил (заполняется в GetRuleFromSelected)
    GS::HashTable<API_Guid, API_PropertyDefinition> rule_definitions = {};
    // Накопитель результата запуска. Живёт до конца функции: окно
    // показывается после записи, чтобы в него попали и ошибки подготовки, и
    // итог по правилам.
    RenumRunResult runResult = {};
    // Ищем свойства с флагом Renum_flag среди выбранных элементов
    if (!GetRuleFromSelected (guidArray, rule_definitions, RENUMFLAG, false)) {
        msg_rep ("ReNumSelected",
                 "No Num rule found.\nCheck that the description of the user property contains Renum_flag",
                 NoError,
                 APINULLGuid);
        // Всплывающего окна здесь нет: сообщение копится и показывается в
        // общем окне результата вместе со всем остальным запуском.
        runResult.AddGeneralMessage (RSGetIndString (iseng, RenumNoRuleId, ACAPI_GetOwnResModule ()));
        ShowRenumResult (&runResult);
        return NoError;
    }
    // Словарь для записи новых значений (guid элемента -> имя свойства -> новое значение)
    ParamDictElement paramToWriteelem = {};
    // Основная обработка: сбор данных, диалог, чтение, распределение позиций
    if (!GetRenumElements (guidArray, paramToWriteelem, rule_definitions, rule_from_one, &runResult)) {
        msg_rep ("ReNumSelected", "No data to write", NoError, APINULLGuid);
        ShowRenumResult (&runResult);
        return NoError;
    }
    UInt32 qtywrite = paramToWriteelem.GetSize ();
    GS::UniString subtitle = GS::UniString::Printf ("Writing data to %d elements", qtywrite);
    Int32 i = 2; // APIIo_SetNextProcessPhaseID ожидает Int32* maxval (см. DevKit-25)
    #ifdef ServerMainVers_2700
    bool showPercent = false;
    Int32 maxval = 2;
    ACAPI_ProcessWindow_SetNextProcessPhase (&subtitle, &maxval, &showPercent);
    #else
    ACAPI_Interface (APIIo_SetNextProcessPhaseID, &subtitle, &i);
    #endif
    SuspendGroupsGuard suspGuard; // отключаем режим группировки на время записи.
    #if defined(TESTING)
    DBprnt ("Write start");
    #endif
    err = ACAPI_CallUndoableCommand (undoString, [&] () -> GSErrCode {
        ParamHelpers::ElementsWrite (paramToWriteelem);
        return NoError;
    });
    if (err != NoError) {
        if (err == APIERR_REFUSEDCMD) {
            ParamHelpers::ElementsWrite (paramToWriteelem);
            msg_rep ("ReNumSelected", "Undo is disabled", err, APINULLGuid);
        } else {
            msg_rep ("ReNumSelected", "ACAPI_CallUndoableCommand", err, APINULLGuid);
            runResult.AddGeneralMessage (RSGetIndString (iseng, RenumUndoFailedId, ACAPI_GetOwnResModule ()));
            ShowRenumResult (&runResult);
            return err;
        }
    }
    #if defined(TESTING)
    DBprnt ("Write end");
    #endif
    SyncArray (syncSettings, guidArray);
    finish = clock ();
    duration = (double)(finish - start) / CLOCKS_PER_SEC;
    // Итог по всему запуску кладётся в накопитель ДО показа окна: число
    // записанных позиций принадлежит всем правилам сразу, и вне окна ему
    // показываться негде.
    runResult.elementsToWrite = qtywrite;
    GS::UniString time = GS::UniString::Printf (" %.3f s", duration);
    msg_rep ("ReNumSelected", GS::UniString::Printf ("Time spent%s", time), NoError, APINULLGuid);
    // Окно результата - единственное место, где пользователь видит итог по
    // правилам. Показывается и на успешном запуске: список счётчиков и есть
    // результат, а не только ошибка.
    ShowRenumResult (&runResult);
    return NoError;
}

// -----------------------------------------------------------------------------------------------------------------------
// Окно результата запуска перенумерации
//
// Показывается ОДИН раз, в конце запуска. Раньше каждая точка отказа
// открывала своё всплывающее окно (ACAPI_WriteReport (…, true)), поэтому при
// нескольких проблемах пользователь получал их по одной и терял предыдущие из
// виду. Теперь сообщения копятся в RenumRunResult, а окно показывает их все
// разом: строка правила, две колонки чисел (элементов / изменено позиций) и
// текст ошибки под своим правилом.
//
// Окно только показывает: флажков нет, клик по строке ничего не переключает.
// -----------------------------------------------------------------------------------------------------------------------
void ShowRenumResult (RenumRunResult *runResult) {
    // Без накопителя показывать нечего: все точки отказа пишут сообщения
    // именно в него.
    if (runResult == nullptr)
        return;
    const Int32 iseng = ID_ADDON_STRINGS + isEng ();
    // Окно показывается, если есть что показать: либо строки правил, либо
    // сообщения. Пустой отчёт не должен занимать экран диалогом.
    if (runResult->ruleNames.IsEmpty () && runResult->messages.IsEmpty ())
        return;

    RuleSelectData data = {};
    data.isReadOnly = true;
    data.titleResID = UndoReNumId;
    // Пять колонок по умолчанию (60 px) не оставляют подписям места: окно
    // 500 px, на имя правила уходит остаток, и «Игнорировано» обрезается.
    data.valueColumnWidth = 70;
    // Тире в строке «[ИМЯ] — текст»: символ U+2014. Задан константой, а не
    // литералом в вызове, потому что иначе он неотличим от машинописного
    // дефиса при просмотре кода.
    const GS::UniString DASHES ("\xE2\x80\x94");
    // Подписи колонок берутся из ресурса, а не пишутся литералами: они
    // показываются пользователю и должны переводиться.
    data.columnTitles.Push (RSGetIndString (iseng, RenumElementsId, ACAPI_GetOwnResModule ()));
    data.columnTitles.Push (RSGetIndString (iseng, RenumWrittenId, ACAPI_GetOwnResModule ()));
    data.columnTitles.Push (RSGetIndString (iseng, RenumIgnoredId, ACAPI_GetOwnResModule ()));
    data.columnTitles.Push (RSGetIndString (iseng, RenumSkippedId, ACAPI_GetOwnResModule ()));
    data.columnTitles.Push (RSGetIndString (iseng, RenumErrorsId, ACAPI_GetOwnResModule ()));

    GS::UniString footer;
    for (UIndex i = 0; i < runResult->ruleNames.GetSize (); ++i) {
        const GS::UniString &name = runResult->ruleNames[i];
        data.rules.Add (name, true);
        GS::Array<GS::UniString> values = {};
        const RenumRuleStats &stats = runResult->ruleStats[i];
        values.Push (GS::UniString::Printf ("%d", (int)stats.elements));
        values.Push (GS::UniString::Printf ("%d", (int)stats.written));
        values.Push (GS::UniString::Printf ("%d", (int)stats.ignored));
        values.Push (GS::UniString::Printf ("%d", (int)stats.skipped));
        values.Push (GS::UniString::Printf ("%d", (int)stats.errors));
        data.valuesPerRule.Add (name, values);
        // Ошибка правила показывается под строкой в виде «[ИМЯ] — текст».
        if (runResult->HasRuleError (i)) {
            data.color.Add (name, Gfx::Color::Red);
            for (const RenumMessage &message : runResult->messages) {
                if (message.ruleName != name)
                    continue;
                if (!footer.IsEmpty ())
                    footer.Append (LINEBRAKE);
                // Строки склеиваются конкатенацией, а не Printf с ToCStr:
                // CStr - некопируемый класс, и передача его в функцию с
                // переменным числом аргументов не компилируется (C2280/C4839 в
                // UniString.hpp). Шаблон же принимает GS::UniString по значению.
                footer.Append ("[" + name + "] " + DASHES + " " + message.text);
            }
        }
    }
    // Ошибки без правила идут вниз без привязки: правила, к которому их можно
    // было бы отнести, у них нет.
    for (const RenumMessage &message : runResult->messages) {
        if (!message.ruleName.IsEmpty ())
            continue;
        if (!footer.IsEmpty ())
            footer.Append (LINEBRAKE);
        footer.Append (message.text);
    }
    // Итог по запуску идёт под списком сообщений: он принадлежит всем
    // правилам сразу, поэтому привязать его к строке нельзя.
    if (runResult->elementsToWrite > 0) {
        if (!footer.IsEmpty ())
            footer.Append (LINEBRAKE);
        footer.Append (RSGetIndString (iseng, RenumWrittenTotalId, ACAPI_GetOwnResModule ()) +
                       GS::UniString::Printf ("%d", (int)runResult->elementsToWrite));
    }
    data.footerText = footer;
    data.is_warn = !runResult->messages.IsEmpty ();
    data.footerIsWarn = !runResult->messages.IsEmpty ();
    RuleSelectDialog dialog (data);
    dialog.Invoke ();
}

// Диалог выбора правил нумерации пользователем
// Показывает список найденных правил с количеством элементов для каждого
// Возвращает true, если пользователь подтвердил выбор и есть активные правила
bool RenumDG (Rules &renum_rules, bool &rule_from_one) {
    #if defined(TESTING)
    DBprnt ("Show DG start");
    #endif
    RuleSelectData rules = {};
    for (GS::HashTable<API_Guid, RenumRule>::PairIterator cIt = renum_rules.EnumeratePairs (); cIt != NULL; ++cIt) {
    #ifdef ServerMainVers_2800
        const RenumRule &rule = cIt->value;
    #else
        const RenumRule &rule = *cIt->value;
    #endif
        if (!rule.state)
            continue;
        if (rules.rules.ContainsKey (rule.rule_name))
            continue;
        if (rules.qty_elements.ContainsKey (rule.rule_name))
            continue;
        rules.rules.Add (rule.rule_name, true);
        rules.qty_elements.Add (rule.rule_name, GS::UniString::Printf ("%d", rule.elemts.GetSize ()));
    }
    rules.is_warn = rule_from_one;
    rules.titleResID = UndoReNumId;
    RuleSelectDialog dialog (rules);
    if (!dialog.Invoke ())
        return false;
    bool has_true_state = false;
    for (GS::HashTable<API_Guid, RenumRule>::PairIterator cIt = renum_rules.EnumeratePairs (); cIt != NULL; ++cIt) {
    #ifdef ServerMainVers_2800
        RenumRule &rule = cIt->value;
    #else
        RenumRule &rule = *cIt->value;
    #endif
        if (!rule.state)
            continue;
        if (const auto *r = rules.rules.GetPtr (rule.rule_name)) {
            rule.state = *r;
            if (rule.state)
                has_true_state = true;
        }
    }
    #if defined(TESTING)
    DBprnt ("Show DG end");
    #endif
    return has_true_state;
}

// Основная функция сбора данных и распределения позиций
// Алгоритм:
// 1. Для каждого выбранного элемента ищем свойства с правилами нумерации
// 2. Собираем значения свойств (позиция, флаг, критерий, разбивка) в paramToReadelem
// 3. Показываем диалог выбора правил (RenumDG)
// 4. Читаем значения свойств через ElementsRead
// 5. Для каждого правила вызываем ReNumOneRule для распределения позиций
// 6. Формируем paramToWriteelem с новыми позициями и наполняем runResult
// Возвращает: true, если есть данные для записи
// Все сообщения пишутся в runResult, а не открывают окна: одно окно в конце
// запуска показывает их все вместе с итогом по правилам.
bool GetRenumElements (GS::Array<API_Guid> &guidArray,
                       ParamDictElement &paramToWriteelem,
                       GS::HashTable<API_Guid, API_PropertyDefinition> &rule_definitions,
                       bool &rule_from_one,
                       RenumRunResult *runResult) {
    #if defined(TESTING)
    DBprnt ("GetRenumElements start");
    #endif
    // Получаем список правил суммирования
    Rules rules = {};
    ParamDictElement paramToReadelem = {};
    bool hasRule = !rule_definitions.IsEmpty ();
    const Int32 iseng = ID_ADDON_STRINGS + isEng ();
    GS::UniString subtitle = GS::UniString::Printf ("Reading data from %d elements", guidArray.GetSize ());
    // Отсутствующие свойства копятся по правилам-владельцам: свойство
    // отсутствует у конкретного правила, и общий список имён не показал бы,
    // какое из правил негодно.
    RenumMissingProps missing_props = {};
    #ifndef ServerMainVers_2700
    // Счётчик фазы для APIIo_SetNextProcessPhaseID. Начиная с AC27 фаза
    // задаётся парой (maxval, showPercent), и номер больше не нужен.
    int n_elem = 0;
    #endif
    #if defined(TESTING)
    DBprnt ("find rule");
    #endif
    GS::Array<API_PropertyDefinition> definitions = {};
    for (const auto &guid : guidArray) {
        definitions.Clear ();
        GSErrCode err =
            ACAPI_Element_GetPropertyDefinitions (guid, API_PropertyDefinitionFilter_UserDefined, definitions);
        if (err != NoError) {
            msg_rep ("GetRenumElements", "ACAPI_Element_GetPropertyDefinitions", err, APINULLGuid);
            continue;
        }
        if (definitions.IsEmpty ()) {
            continue;
        }
        bool hasDef = false;
        if (hasRule) {
            for (UInt32 i = definitions.GetSize (); i-- > 0;) {
                if (definitions[i].description.IsEmpty ())
                    continue;
                if (definitions[i].description.Contains (RENUMFLAG)) {
                    if (!rule_definitions.ContainsKey (definitions[i].guid)) {
                        definitions.Delete (i);
                    } else {
                        hasDef = true;
                    }
                }
            }
        } else {
            hasDef = ReNumHasFlag (definitions);
        }
    #ifdef ServerMainVers_2700
        bool showPercent = true;
        Int32 maxval = guidArray.GetSize ();
        ACAPI_ProcessWindow_SetNextProcessPhase (&subtitle, &maxval, &showPercent);
    #else
        n_elem += 1;
        ACAPI_Interface (APIIo_SetNextProcessPhaseID, &subtitle, &n_elem);
    #endif
    #ifdef ServerMainVers_2700
        if (ACAPI_ProcessWindow_IsProcessCanceled ())
            return false;
    #else
        if (ACAPI_Interface (APIIo_IsProcessCanceledID, nullptr, nullptr))
            return false;
    #endif
        if (!hasDef)
            continue;
        ReNum_GetElement (guid, paramToReadelem, rules, missing_props, definitions);
    }

    // Отсутствующие свойства отмечают конкретные правила, а не элементы:
    // сообщение привязывается к строке правила, и остальные правила при этом
    // дорабатывают. Прежний код на этом месте останавливал весь запуск, из-за
    // чего одно негодное правило обнуляло нумерацию по всем остальным.
    if (!missing_props.IsEmpty ()) {
        GS::UniString out = EMPTYSTRING;
        // До AC28 CurrentPair хранила указатели (const Key* key; Value* value),
        // поэтому пару приходилось разыменовывать. С AC28 пара хранит ссылки
        // (const Key& key; Value& value) и лишняя разыменовка не нужна. Само
        // обращение к паре в обеих версиях - через operator->, как во всех
        // обходах словарей проекта.
        for (RenumMissingProps::PairIterator cIt = missing_props.EnumeratePairs (); cIt != NULL; ++cIt) {
    #ifdef ServerMainVers_2800
            const GS::UniString &rname = cIt->key;
            const GS::Array<GS::UniString> &rawnames = cIt->value;
    #else
            const GS::UniString &rname = *cIt->key;
            const GS::Array<GS::UniString> &rawnames = *cIt->value;
    #endif
            // Пустое имя правила возможно только при отсутствии описания у
            // свойства: показать его нечем, и строка уходит вниз без привязки.
            GS::UniString list = EMPTYSTRING;
            for (const GS::UniString &rawname : rawnames) {
                if (!list.IsEmpty ())
                    list.Append (LINEBRAKE);
                list.Append (RenumReadableName (rawname));
            }
            if (rname.IsEmpty ()) {
                if (!out.IsEmpty ())
                    out.Append (LINEBRAKE);
                out.Append (list);
            } else {
                if (runResult != nullptr)
                    runResult->AddRuleMessage (rname, list);
            }
        }
        msg_rep ("ReNumSelected", "Can't find property, check name: " + out, APIERR_GENERAL, APINULLGuid);
        if (runResult != nullptr)
            runResult->AddGeneralMessage (RSGetIndString (iseng, RenumMissingPropId, ACAPI_GetOwnResModule ()) +
                                          (out.IsEmpty () ? EMPTYSTRING : LINEBRAKE + out));
    }
    if (!paramToReadelem.IsEmpty ())
        msg_rep ("ReNumSelected",
                 GS::UniString::Printf ("Find elements - %d ", paramToReadelem.GetSize ()),
                 NoError,
                 APINULLGuid);
    if (!rules.IsEmpty ())
        msg_rep ("ReNumSelected", GS::UniString::Printf ("Find rules - %d ", rules.GetSize ()), NoError, APINULLGuid);
    // Отвергнутые правила регистрируются ДАВ проверки на пустой результат: они
    // найдены и негодны, и пользователь должен видеть их в окне результата
    // вместе с причиной. Без этого «правило отработало вхолостую» выглядело бы
    // как «правила не было».
    for (GS::HashTable<API_Guid, RenumRule>::PairIterator cIt = rules.EnumeratePairs (); cIt != NULL; ++cIt) {
    #ifdef ServerMainVers_2800
        const RenumRule &rule = cIt->value;
    #else
        const RenumRule &rule = *cIt->value;
    #endif
        if (rule.state)
            continue;
        const UIndex index = (runResult != nullptr) ? runResult->EnsureRuleStats (rule.rule_name) : 0;
        if (runResult != nullptr)
            runResult->ruleStats[index].elements += rule.elemts.GetSize ();
    }
    if (paramToReadelem.IsEmpty () || rules.IsEmpty ()) {
        if (paramToReadelem.IsEmpty ())
            msg_rep ("ReNumSelected", "Parameters for read not found", NoError, APINULLGuid);
        if (rules.IsEmpty ())
            msg_rep ("ReNumSelected", "Rules not found", NoError, APINULLGuid);
        if (runResult != nullptr)
            runResult->AddGeneralMessage (RSGetIndString (iseng, RenumRuleErrorId, ACAPI_GetOwnResModule ()));
        return false;
    }
    if (!RenumDG (rules, rule_from_one)) {
        msg_rep ("ReNumSelected", "Execution interrupted by user", NoError, APINULLGuid);
        return false;
    }
    ParamHelpers::ElementsRead (paramToReadelem); // Читаем значения
    // Теперь выясняем - какой режим нумерации у элементов и распределяем позиции
    // по правилам. Каждое правило даёт строку результата: сколько элементов под
    // ним и сколько позиций в них изменилось.
    for (GS::HashTable<API_Guid, RenumRule>::PairIterator cIt = rules.EnumeratePairs (); cIt != NULL; ++cIt) {
    #ifdef ServerMainVers_2800
        RenumRule &rule = cIt->value;
    #else
        RenumRule &rule = *cIt->value;
    #endif
        if (!rule.state)
            continue;
        // Повторная регистрация того же имени (два свойства с одинаковым
        // описанием правила) возвращает прежний индекс, поэтому числа
        // накапливаются: строка показывает объединённый итог по правилу.
        const UIndex index = (runResult != nullptr) ? runResult->EnsureRuleStats (rule.rule_name) : 0;
        if (runResult != nullptr)
            runResult->ruleStats[index].elements += rule.elemts.GetSize ();
        bool has_error = false;
        if (!rule.elemts.IsEmpty ())
            ReNumOneRule (rule, paramToReadelem, paramToWriteelem, has_error);
        if (runResult != nullptr) {
            runResult->ruleStats[index].written += rule.n_write;
            runResult->ruleStats[index].ignored += rule.n_ignore;
            runResult->ruleStats[index].skipped += rule.n_skip;
            runResult->ruleStats[index].errors += rule.n_error;
        }
        if (has_error && runResult != nullptr) {
            // Счётчик ошибок, а не пропусков: элементы, помеченные флагом
            // «пропустить», - законное решение правила, ошибкой оно не является.
            GS::UniString text = RSGetIndString (iseng, RenumPartialErrorId, ACAPI_GetOwnResModule ()) +
                                 GS::UniString::Printf ("%d", rule.n_error);
            runResult->AddRuleMessage (rule.rule_name, text);
        }
    }
    if (paramToWriteelem.IsEmpty ()) {
        msg_rep ("ReNumSelected", "No position changes required", NoError, APINULLGuid);
        if (runResult != nullptr)
            runResult->AddGeneralMessage (RSGetIndString (iseng, RenumNoChangeId, ACAPI_GetOwnResModule ()));
        return false;
    }
    #if defined(TESTING)
    DBprnt ("GetRenumElements end");
    #endif
    return !paramToWriteelem.IsEmpty ();
}

// -----------------------------------------------------------------------------------------------------------------------
// Вырезает шаблон формулы из части правила Renum{...}
// Кавычки - детектор формулы, берётся первая пара (как в ParseSyncString).
// Подстрока берётся ДО ToLowerCase правила - литерал шаблона должен сохранить
// исходный регистр, тогда как имя свойства между процентами к нижнему регистру
// приводит уже ReplaceProcToBrace.
// Возвращает true, если часть кавычена и между кавычками непустой шаблон
// -----------------------------------------------------------------------------------------------------------------------
bool GetFormulaTemplate (const GS::UniString &rulepart, GS::UniString &templateOut) {
    templateOut.Clear ();
    if (!rulepart.Contains (CHARDQUT))
        return false;
    UIndex firstQuote = rulepart.FindFirst (CHARDQUT);
    UIndex secondQuote = rulepart.FindFirst (CHARDQUT, firstQuote + 1);
    if (secondQuote == MaxUIndex) // непарные кавычки - трактуем как обычное свойство
        return false;
    if (secondQuote == firstQuote + 1)
        return false; // пустые кавычки - не формула
    templateOut = rulepart.GetSubstring (firstQuote + 1, secondQuote - firstQuote - 1);
    templateOut.Trim ();
    return !templateOut.IsEmpty ();
}

// -----------------------------------------------------------------------------------------------------------------------
// Имя параметра-формулы по роли части правила
// Критерий и разбивка получают разные имена, иначе формулы с одинаковым
// шаблоном получили бы один ключ в словаре параметров элемента.
// -----------------------------------------------------------------------------------------------------------------------
GS::UniString RenumFormulaName (RenumPart part) {
    return part == RenumPart::Criteria ? GS::UniString ("renum_criteria") : GS::UniString ("renum_delimetr");
}

// -----------------------------------------------------------------------------------------------------------------------
// Имя параметра-формулы для критерия/разбивки правила
// Уникально в пределах правила: префикс + имя + ';' + шаблон (как в Sync.cpp).
// Шаблон входит в имя, поэтому две разные формулы одного правила не склеиваются
// и не совпадают с формулой такого же шаблона в другом правиле.
// -----------------------------------------------------------------------------------------------------------------------
GS::UniString GetFormulaRawName (const GS::UniString &paramName, const GS::UniString &templateFormula) {
    return FORMULANAMEPREFIX + paramName + SEMICOLON + templateFormula + BRACEEND;
}

// -----------------------------------------------------------------------------------------------------------------------
// Добавляет в paramToRead значение-формулу по шаблону правила
// Работает существующий путь чтения: ParamValue с hasFormula уходит в
// ParamHelpers::Read -> ReadFormula -> ReplaceParamInExpression -> EvalExpression,
// поэтому ElementsSeparation получает уже вычисленную строку.
// Зависимости %имя% разбираются как свойства (fromMaterial по умолчанию),
// как в BuildReadParamDict (Spec.cpp) - критерий нумерации задаётся именем
// свойства Archicad, а не параметром библиотечного элемента.
// -----------------------------------------------------------------------------------------------------------------------
void AddFormulaToRead (const API_Guid &elemGuid,
                       const GS::UniString &paramName,
                       const GS::UniString &templateFormula,
                       ParamDictElement &paramToRead) {
    ParamDictValue paramDict = {}; // Зависимости шаблона для одного элемента
    GS::UniString templatestring = templateFormula;
    // %имя% -> {@property:имя} и добавление самих зависимостей в словарь
    ParamHelpers::ParseParamNameMaterial (templatestring, paramDict);
    ParamValue param = {};
    param.rawName = GetFormulaRawName (paramName, templateFormula);
    param.name = paramName;
    param.typeinx = FORMULATYPEINX;
    param.val.hasFormula = true;
    // Шаблон НЕ оборачивается в <...>: EvalExpression вычисляет каждую пару
    // <...> на месте, поэтому внешняя обёртка сломала бы внутреннюю формулу
    // (парсер ищет первую '>' и обрезал бы по ней). Так же поступает и Sync для
    // кавыченной формулы (Sync.cpp:1774) — обёртка там только для <...>-формы.
    param.val.uniStringValue = templatestring;
    ParamHelpers::AddParamValue2ParamDict (elemGuid, param, paramDict);
    // Переносим зависимости шаблона в словарь элемента (как BuildReadParamDict в Spec)
    ParamHelpers::AddParamDictValue2ParamDictElement (elemGuid, paramDict, paramToRead);
}

// -----------------------------------------------------------------------------------------------------------------------
// Имя свойства в читаемом виде для сообщения пользователю.
// Из «{@property:этаж}» делает «этаж», из «{@gdl:тип}» - «тип».
//
// Служебные префиксы пользователю ничего не говорят. Отбрасывается вся
// известная приставка целиком, вместе с двоеточием: замена по одному слову
// «property» оставляла бы «:этаж», а замена всех двоеточий портила бы имена
// формул, у которых двоеточие внутри («{@formula:renum_criteria;…}»). Поэтому
// сначала отрезается префикс, а двоеточие убирается только там, где осталось
// ровно одно - оставшееся имя.
// -----------------------------------------------------------------------------------------------------------------------
GS::UniString RenumReadableName (const GS::UniString &rawname) {
    GS::UniString out = rawname;
    // Порядок важен: префиксы проверяются от длинного к короткему, иначе
    // «{@property:…}» не совпал бы ни с одной приставкой целиком.
    const GS::Array<GS::UniString> knownPrefixes = {
        PROPERTYNAMEPREFIX, MATERIALNAMEPREFIX, FORMULANAMEPREFIX, GDLNAMEPREFIX};
    for (const GS::UniString &prefix : knownPrefixes) {
        if (!out.BeginsWith (prefix))
            continue;
        UIndex bodyStart = prefix.GetLength ();
        UIndex bodyEnd = out.GetLength ();
        if (bodyEnd > bodyStart && out[bodyEnd - 1] == CHARBRACEEND)
            bodyEnd -= 1;
        return out.GetSubstring (bodyStart, bodyEnd - bodyStart);
    }
    // Префикс неизвестен: оставляем как есть, только снимаем обрамляющие скобки.
    if (out.BeginsWith (PVALPREFIX))
        out = out.GetSubstring (PVALPREFIX.GetLength (), out.GetLength () - PVALPREFIX.GetLength ());
    if (out.GetLength () > 0 && out[out.GetLength () - 1] == CHARBRACEEND)
        out = out.GetSubstring (0, out.GetLength () - 1);
    return out;
}

// -----------------------------------------------------------------------------------------------------------------------
// Регистрирует отсутствующее свойство в словаре по правилам-владельцам.
// Повторное добавление того же имени под тем же правилом игнорируется: одно
// свойство может отсутствовать у многих элементов, а список пользователю нужен
// один раз.
// -----------------------------------------------------------------------------------------------------------------------
void AddMissingProp (RenumMissingProps &missing_props,
                     const GS::UniString &rule_name,
                     const GS::UniString &rawname,
                     const GS::UniString &formula,
                     const ParamDictValue &propertyParams) {
    // Формульная часть не является свойством: имени у неё нет, и пустое имя
    // отчиталось бы как отсутствующее свойство (#249).
    if (!formula.IsEmpty () || rawname.IsEmpty ())
        return;
    if (propertyParams.ContainsKey (rawname))
        return;
    GS::Array<GS::UniString> *list = missing_props.GetPtr (rule_name);
    if (list == nullptr) {
        missing_props.Add (rule_name, GS::Array<GS::UniString> ());
        list = missing_props.GetPtr (rule_name);
    }
    for (const GS::UniString &item : *list)
        if (item == rawname)
            return;
    list->Push (rawname);
}

// -----------------------------------------------------------------------------------------------------------------------
// Имя правила для показа в окне результата: текст между фигурными скобками
// описания свойства-флага, без "Renum_flag{" и без хвостовых "}".
//
// Имя нужно ДО разбора правила на годность: сообщение о негодном правиле
// привязывается к строке окна, а строка без вычисленного имени осталась бы
// пустой и потеряла бы связь с исходным описанием.
// -----------------------------------------------------------------------------------------------------------------------
GS::UniString RenumRuleDisplayName (const GS::UniString &description) {
    GS::Array<GS::UniString> partstring;
    GS::UniString name = description;
    if (StringSplt (name, BRACEEND, partstring, "enum_flag") > 0)
        name = partstring[0] + BRACEEND;
    name = name.GetSubstring (CHARBRACESTART, CHARBRACEEND, 0);
    name.ReplaceAll ("Property:", SPACESTRING);
    name = CHARBSEMICOLON + name + CHARBSEMICOLON;
    name = name.GetSubstring (CHARBSEMICOLON, CHARBSEMICOLON, 0);
    name.Trim ();
    return name;
}

// -----------------------------------------------------------------------------------------------------------------------
// Функция распределяет элемент в таблицу с правилами нумерации
// Для каждого свойства элемента с флагом Renum_flag{...}:
// 1. Парсит описание флага, извлекает имя свойства-позиции и настройки нулей
// 2. Находит связанное свойство-правило (с описанием Renum{...})
// 3. Из правила извлекает имя свойства-критерия и свойства-разбивки
// 4. Создаёт или обновляет RenumRule в таблице rules
// 5. Добавляет guid элемента в rule.elemts
// 6. Подготавливает чтение значений свойств (позиция, флаг, критерий, разбивка) в paramToRead
// Возвращает true, если найдено хотя бы одно правило нумерации
// -----------------------------------------------------------------------------------------------------------------------
bool ReNum_GetElement (const API_Guid &elemGuid,
                       ParamDictElement &paramToRead,
                       Rules &rules,
                       RenumMissingProps &missing_props,
                       const GS::Array<API_PropertyDefinition> &definitions) {
    bool hasRenum = false;
    if (!ParamHelpers::isPropertyDefinitionRead ())
        return false;
    ParamDictValue &propertyParams = PROPERTYCACHE ().property;
    GS::Array<GS::UniString> partstring_;
    GS::Array<GS::UniString> partstring;
    GS::Array<GS::UniString> local_scratch;
    ParamValue pvalue_position;
    ParamValue pvalue_flag;
    ParamValue pvalue_criteria;
    ParamValue pvalue_delimetr;
    for (const API_PropertyDefinition &definition : definitions) {
        if (!definition.description.Contains (RENUMFLAG))
            continue;
        if (!definition.description.Contains (BRACESTART)) {
            msg_rep ("ReNumSelected",
                     definition.name + " Renum_flag: check the opening bracket, there should be {",
                     APIERR_GENERAL,
                     APINULLGuid);
            continue;
        }
        if (!definition.description.Contains (BRACEEND)) {
            msg_rep ("ReNumSelected",
                     definition.name + " Renum_flag: check the closing bracket, there should be }",
                     APIERR_GENERAL,
                     APINULLGuid);
            continue;
        }
        RenumRule *rulecritetiaPtr = rules.GetPtr (definition.guid);
        if (rulecritetiaPtr == nullptr) {
            RenumRule rulecritetia = {};
            // Имя правила считается сразу, до разбора на годность: по нему
            // привязываются сообщения об отсутствующих свойствах, в том числе
            // когда правило признано негодным и до строки присваивания не
            // дошло дело.
            const GS::UniString rule_name = RenumRuleDisplayName (definition.description);
            // Разбираем - что записано в свойстве с флагом
            // В нём должно быть имя свойства и, возможно, флаг добавления нулей
            GS::UniString paramName = definition.description.ToLowerCase ();
            partstring.Clear ();
            if (StringSplt (paramName, BRACEEND, partstring, "enum_flag", &local_scratch) > 0) {
                paramName = partstring[0] + BRACEEND;
            }
            paramName = paramName.GetSubstring (CHARBRACESTART, CHARBRACEEND, 0);
            paramName.ReplaceAll (SLASHEKR, SLASH);
            GS::UniString rawNameposition = PVALPREFIX;
            if (paramName.Contains (SEMICOLON)) { // Есть указание на нули
                partstring.Clear ();
                int nparam = StringSplt (paramName, SEMICOLON, partstring, true, &local_scratch);
                rawNameposition = rawNameposition + partstring[0] + BRACEEND;
                if (nparam > 1) {
                    if (partstring[1].Contains ("null"))
                        rulecritetia.nulltype = ADDZEROS;
                    if (partstring[1].Contains ("allnull"))
                        rulecritetia.nulltype = ADDMAXZEROS;
                    if (partstring[1].Contains ("space"))
                        rulecritetia.nulltype = ADDSPACE;
                    if (partstring[1].Contains ("allspace"))
                        rulecritetia.nulltype = ADDMAXSPACE;
                    // Добавление нужного количества знаков
                    if (rulecritetia.nulltype != NOZEROS && partstring[1].Contains ("_")) {
                        if (StringSplt (partstring[1], "_", partstring_, true, &local_scratch) > 0) {
                            double nullcount = 0;
                            if (UniStringToDouble (partstring_[0], nullcount))
                                rulecritetia.nullcount = (int)nullcount;
                        }
                    }
                }
            } else {
                rawNameposition = rawNameposition + paramName + BRACEEND;
            }
            if (!rawNameposition.Contains (PROPERTYSTRING))
                rawNameposition.ReplaceAll (BRACESTART, GDLNAMEPREFIX);
            // Проверяем - есть ли у объекта такое свойство-правило
            if (const auto *p = propertyParams.GetPtr (rawNameposition)) {
                // В описании правила может быть указано имя свойства-критерия и, возможно, имя свойства-разбивки
                GS::UniString ruleparamName = p->definition.description;
                ruleparamName.ReplaceAll (SLASHEKR, SLASH);
                if (ruleparamName.Contains (RENUM) && ruleparamName.Contains (BRACESTART) &&
                    ruleparamName.Contains (BRACEEND)) {
                    partstring.Clear ();
                    if (StringSplt (ruleparamName, BRACEEND, partstring, "enum") > 0) {
                        ruleparamName = partstring[0] + BRACEEND;
                    }
                    // Шаблоны формул вырезаются ДО ToLowerCase: литерал шаблона
                    // должен сохранить исходный регистр, а к нижнему регистру
                    // приводится только имя свойства (ReNum.cpp:502).
                    GS::UniString ruleBody = ruleparamName.GetSubstring (CHARBRACESTART, CHARBRACEEND, 0);
                    GS::Array<GS::UniString> ruleParts = {};
                    int nRulePart = StringSplt (ruleBody, SEMICOLON, ruleParts, true, &local_scratch);
                    GS::UniString criteria_formula;
                    GS::UniString delimetr_formula;
                    // Кавычки - детектор формулы; некавыченная часть остаётся свойством
                    if (nRulePart > 0)
                        GetFormulaTemplate (ruleParts[0], criteria_formula);
                    if (nRulePart > 1)
                        GetFormulaTemplate (ruleParts[1], delimetr_formula);
                    ruleparamName = ruleBody.ToLowerCase ();
                    GS::UniString rawNamecriteria = PVALPREFIX;
                    GS::UniString rawNamedelimetr;
                    if (ruleparamName.Contains (SEMICOLON)) { // Есть указание на нули
                        partstring.Clear ();
                        int nparam = StringSplt (ruleparamName, SEMICOLON, partstring, true, &local_scratch);
                        // Формульная часть не является свойством - имя для неё не строим
                        if (criteria_formula.IsEmpty ())
                            rawNamecriteria = rawNamecriteria + partstring[0] + BRACEEND;
                        if (nparam > 1 && delimetr_formula.IsEmpty ())
                            rawNamedelimetr = PVALPREFIX + rawNamedelimetr + partstring[1] + BRACEEND;
                    } else {
                        if (criteria_formula.IsEmpty ())
                            rawNamecriteria = rawNamecriteria + ruleparamName + BRACEEND;
                    }
                    // Формульная часть не является свойством - для неё имя не строим
                    // (иначе ReplaceAll ниже склеил бы "{@" в несуществующий ключ)
                    if (criteria_formula.IsEmpty ()) {
                        if (!rawNamecriteria.Contains (PROPERTYSTRING))
                            rawNamecriteria.ReplaceAll (BRACESTART, GDLNAMEPREFIX);
                    }
                    if (!rawNamedelimetr.IsEmpty () && !rawNamedelimetr.Contains (PROPERTYSTRING))
                        rawNamedelimetr.ReplaceAll (BRACESTART, GDLNAMEPREFIX);
                    // Если такие свойства есть - записываем правило
                    // Для формульной части проверки свойства не делаем: за неё
                    // отвечают зависимости шаблона, они собираются при чтении.
                    bool has_criteria = !criteria_formula.IsEmpty () || propertyParams.ContainsKey (rawNamecriteria);
                    bool has_delimetr = !delimetr_formula.IsEmpty () || rawNamedelimetr.IsEmpty () ||
                                        propertyParams.ContainsKey (rawNamedelimetr);
                    if (has_criteria && has_delimetr) {
                        rulecritetia.state = true;
                        rulecritetia.oldalgoritm = false;
                        rulecritetia.position = rawNameposition;
                        GS::UniString fname;
                        GS::UniString rawName = PROPERTYNAMEPREFIX;
                        GetPropertyFullName (definition, fname);
                        rawName.Append (fname.ToLowerCase ());
                        rawName.Append (BRACEEND);
                        rulecritetia.flag = rawName;
                        // Формульная часть не имеет имени свойства - вместо него
                        // правило ссылается на параметр-формулу, который будет
                        // прочитан как обычный параметр элемента.
                        rulecritetia.criteria =
                            criteria_formula.IsEmpty ()
                                ? rawNamecriteria
                                : GetFormulaRawName (RenumFormulaName (RenumPart::Criteria), criteria_formula);
                        rulecritetia.delimetr =
                            delimetr_formula.IsEmpty ()
                                ? rawNamedelimetr
                                : GetFormulaRawName (RenumFormulaName (RenumPart::Delimetr), delimetr_formula);
                        rulecritetia.criteria_formula = criteria_formula;
                        rulecritetia.delimetr_formula = delimetr_formula;
                        rulecritetia.guid = definition.guid;
                        rulecritetia.rule_name = rule_name;
                    } else {
                        // Формульные части в missing_props не попадают: имени
                        // свойства у них нет, а пустое имя отчиталось бы как
                        // отсутствующее свойство.
                        AddMissingProp (missing_props, rule_name, rawNamecriteria, criteria_formula, propertyParams);
                        AddMissingProp (missing_props, rule_name, rawNamedelimetr, delimetr_formula, propertyParams);
                    }
                } else {
                    if (ruleparamName.Contains (RENUM)) {
                        GS::UniString msg =
                            ", in property description " + propertyParams.Get (rawNameposition).definition.name;
                        if (!ruleparamName.Contains (BRACESTART))
                            msg_rep ("ReNumSelected",
                                     "Renum: check the closing bracket, there should be }" + msg,
                                     APIERR_GENERAL,
                                     APINULLGuid);
                        if (!ruleparamName.Contains (BRACEEND))
                            msg_rep ("ReNumSelected",
                                     "Renum: check the opening bracket, there should be {" + msg,
                                     APIERR_GENERAL,
                                     APINULLGuid);
                    } else {
                        msg_rep ("ReNumSelected",
                                 "Renum_flag must refer to a property with the description Renum, check property "
                                 "description " +
                                     definition.name,
                                 APIERR_GENERAL,
                                 APINULLGuid);
                    }
                }
            } else {
                // Свойство-позиция не найдено: правило негодно целиком, и его
                // имя уже разобрано (rule_name) - сообщение привязывается к нему.
                AddMissingProp (missing_props, rule_name, rawNameposition, EMPTYSTRING, propertyParams);
            }
            rules.Add (definition.guid, std::move (rulecritetia));
            rulecritetiaPtr = rules.GetPtr (definition.guid);
        }
        if (rulecritetiaPtr != nullptr) {

            if (rulecritetiaPtr->guid != APINULLGuid)
                hasRenum = true;
            rulecritetiaPtr->elemts.Push (elemGuid);

            pvalue_position.Clear ();
            pvalue_flag.Clear ();
            pvalue_criteria.Clear ();
            pvalue_delimetr.Clear ();

            bool has_position = false;
            bool has_flag = false;
            bool has_criteria = false;
            bool has_delimetr = false;
            // Формульные критерий/разбивка не читаются из кэша свойств: им
            // соответствует параметр-формула, который собирается ниже.
            if (!rulecritetiaPtr->position.IsEmpty ())
                has_position = ParamHelpers::GetParamValueFromCache (rulecritetiaPtr->position, pvalue_position);
            if (!rulecritetiaPtr->flag.IsEmpty ())
                has_flag = ParamHelpers::GetParamValueFromCache (rulecritetiaPtr->flag, pvalue_flag);
            if (!rulecritetiaPtr->criteria.IsEmpty ())
                has_criteria = ParamHelpers::GetParamValueFromCache (rulecritetiaPtr->criteria, pvalue_criteria);
            if (!rulecritetiaPtr->delimetr.IsEmpty ())
                has_delimetr = ParamHelpers::GetParamValueFromCache (rulecritetiaPtr->delimetr, pvalue_delimetr);
            pvalue_position.fromGuid = elemGuid;
            pvalue_flag.fromGuid = elemGuid;
            pvalue_criteria.fromGuid = elemGuid;
            pvalue_delimetr.fromGuid = elemGuid;
            if (has_position)
                ParamHelpers::AddParamValue2ParamDictElement (elemGuid, pvalue_position, paramToRead);
            if (has_flag)
                ParamHelpers::AddParamValue2ParamDictElement (elemGuid, pvalue_flag, paramToRead);
            if (has_criteria)
                ParamHelpers::AddParamValue2ParamDictElement (elemGuid, pvalue_criteria, paramToRead);
            if (has_delimetr)
                ParamHelpers::AddParamValue2ParamDictElement (elemGuid, pvalue_delimetr, paramToRead);
            // Критерий-формула и разбивка-формула: имя параметра известно уже на
            // этапе разбора правила, само значение вычислит ReadFormula.
            if (!rulecritetiaPtr->criteria_formula.IsEmpty ())
                AddFormulaToRead (
                    elemGuid, RenumFormulaName (RenumPart::Criteria), rulecritetiaPtr->criteria_formula, paramToRead);
            if (!rulecritetiaPtr->delimetr_formula.IsEmpty ())
                AddFormulaToRead (
                    elemGuid, RenumFormulaName (RenumPart::Delimetr), rulecritetiaPtr->delimetr_formula, paramToRead);
        }
    }
    return hasRenum;
}

// Возвращает самое часто встречающееся значение позиции из массива
// Используется для подбора элементов с одинаковым критерием, но разной позицией
// Алгоритм: подсчитывает частоту каждой позиции, возвращает позицию с максимальным счётчиком
// Если массив пустой - возвращает дефолтный RenumPos()
RenumPos GetMostFrequentPos (const GS::Array<RenumPos> &eleminpos) {
    if (eleminpos.IsEmpty ()) {
        return RenumPos ();
    }
    std::map<std::string, int> npos; // Словарь для подсчёта
    UInt32 inx = 0;
    int maxcont = 0;
    for (UInt32 j = 0; j < eleminpos.GetSize (); j++) {
        std::string pos = eleminpos[j].strpos;
        if (npos.count (pos) != 0) {
            int countpos = npos[pos] + 1;
            npos[pos] = countpos;
            if (countpos > maxcont) {
                maxcont = countpos;
                inx = j;
            }
        } else {
            npos[pos] = 1;
        }
    }
    RenumPos out = eleminpos.Get (inx);
    return out;
}

// Генерирует новую уникальную позицию для заданного критерия и разбивки
// Алгоритм: начинает с позиции 1, инкрементирует пока позиция не станет уникальной в unicpos[delimetr]
// Записывает соответствие позиции -> критерий в unicpos и критерий -> позиция в unicriteria
// Возвращает сгенерированную позицию
RenumPos GetPos (DRenumPosDict &unicpos,
                 DStringDict &unicriteria,
                 const std::string &delimetr,
                 const std::string &criteria) {
    RenumPos pos (1);
    while (unicpos[delimetr].count (pos.strpos) != 0) {
        pos.Add (1);
    }
    unicpos[delimetr][pos.strpos] = criteria;
    unicriteria[delimetr][criteria] = pos;
    return pos;
}

// Основная функция распределения позиций для одного правила
// Алгоритм:
// 1. Вызывает ElementsSeparation для разделения элементов по разбивке, типу нумерации и критерию
// 2. Стадия 1: определяет часто встречающиеся позиции для игнорируемых (RENUM_IGNORE) и добавляемых (RENUM_ADD)
// элементов
// 3. Стадия 2: расставляет позиции для добавляемых и новых элементов, используя словари unicriteria/unicpos
// 4. Стадия 3: записывает итоговые позиции в paramToWriteelem с учётом форматирования нулей/пробелов
// Параметр has_error устанавливается в true при ошибках обработки
void ReNumOneRule (RenumRule &rule,
                   ParamDictElement &paramToReadelem,
                   ParamDictElement &paramToWriteelem,
                   bool &has_error) {

    // Рассортируем элементы по разделителю, типу нумерации и критерию.
    // delimetrList: [разбивка][тип_нумерации][критерий] -> массив позиций элементов
    Delimetr delimetrList;
    #if defined(TESTING)
    DBprnt ("    ReNumOneRule start");
    #endif
    if (!ElementsSeparation (rule, paramToReadelem, delimetrList, has_error))
        return;

    // Словари для отслеживания занятых позиций:
    // unicpos: [разбивка][позиция] -> критерий (какой критерий занял эту позицию)
    // unicriteria: [разбивка][критерий] -> позиция (какую позицию получил этот критерий)
    // Это нужно, чтобы избежать конфликтов позиций между элементами с одинаковым критерием
    DRenumPosDict unicpos;
    DStringDict unicriteria;
    // Словарь максимальных позиций для каждой разбивки (для форматирования нулей)
    std::map<std::string, RenumPos> maxpos;
    RenumPos maxposall; // Глобальный максимум по всем разбивкам
    #if defined(TESTING)
    DBprnt ("        stage 1");
    #endif
    // СТАДИЯ 1: Определяем часто встречающиеся позиции для игнорируемых и добавляемых элементов
    // Игнорируемые (RENUM_IGNORE) - их позиции нельзя занимать, они задают "якоря"
    // Добавляемые (RENUM_ADD) - если у них нет подходящей позиции среди игнорируемых, создаём новую
    if (!rule.oldalgoritm) {
        for (Delimetr::iterator i = delimetrList.begin (); i != delimetrList.end (); ++i) {
            TypeValues &tv = i->second;
            std::string delimetr = i->first;
            RenumPos maxposdelim = maxpos[delimetr];
            if (tv.count (RENUM_IGNORE) != 0) {
                for (Values::iterator k = tv[RENUM_IGNORE].begin (); k != tv[RENUM_IGNORE].end (); ++k) {
                    GS::Array<RenumPos> eleminpos = k->second.elements;
                    std::string criteria = k->first;
                    RenumPos pos = GetMostFrequentPos (eleminpos);
                    unicriteria[delimetr][criteria] = pos;
                    unicpos[delimetr][pos.strpos] = criteria;

                    // Игнорируемые позиции нельзя занимать. Добавим их в словарь
                    for (UInt32 j = 0; j < eleminpos.GetSize (); j++) {
                        if (pos.strpos != eleminpos[j].strpos)
                            unicpos[delimetr][eleminpos[j].strpos] = criteria;
                    }
                    maxposdelim.SetToMax (pos);
                }
            }
            if (tv.count (RENUM_ADD) != 0) {
                for (Values::iterator k = tv[RENUM_ADD].begin (); k != tv[RENUM_ADD].end (); ++k) {
                    GS::Array<RenumPos> eleminpos = k->second.elements;
                    std::string criteria = k->first;

                    // Отбираем элементы, не встретившиеся прежде в игнорируемых
                    if (unicriteria[delimetr].count (criteria) == 0) { // Если такой критерий уже есть в словаре -
                                                                       // значит для него есть подходящая позиция
                        RenumPos pos = GetMostFrequentPos (eleminpos);
                        unicriteria[delimetr][criteria] = pos;
                    }
                }
            }
            maxposall.SetToMax (maxposdelim);
            maxpos[delimetr].SetToMax (maxposdelim);
        }
    }

    // Предолагаемое поведение:
    // Игнорируемые позиции (RENUM_IGNORE) - не меняют значения.
    //		Остальные элементы, при совпадении критерия, могут принимать значения
    // подходящих игнорируемых. Позиции игнорируемых могут совпадать.
    // Добавочные позиции (RENUM_ADD) - меняют значения в случаях:
    //		Сначала проверяем по критерию ищем подходящую позацию среди
    // игнорируемых. В качестве подходящей для назначения выбирается самая
    // часто встречающаяся позиция.
    // Новые позиции (RENUM_NORMAL)
    //		Сначала идёт поиск по подходящим позициям предыдущих типов. Если не
    // нашли - ищем по-порядку свободную позицию.
    // Если критерий элемента совпадает с подходящим критерием игнорируемого - будет применена позиция игнорируемого

    // СТАДИЯ 2: Распределяем позиции для добавляемых (RENUM_ADD) и новых (RENUM_NORMAL) элементов
    // Логика: если для критерия уже есть позиция в unicriteria - используем её, иначе создаём новую через GetPos
    // Теперь последовательно идём по словарю c разделителями, вытаскиваем оттуда guid и нумеруем
    #if defined(TESTING)
    DBprnt ("        stage 2");
    #endif
    for (Delimetr::iterator i = delimetrList.begin (); i != delimetrList.end (); ++i) {
        TypeValues &tv = i->second;
        std::string delimetr = i->first;
        RenumPos maxposdelim;

        // Получаем позиции добавляемых элементов
        if (tv.count (RENUM_ADD) != 0 && !rule.oldalgoritm) {
            for (Values::iterator k = tv[RENUM_ADD].begin (); k != tv[RENUM_ADD].end (); ++k) {
                std::string criteria = k->first;
                // Расставляем позиции для элементов, для которых есть подходящая по критериям позиция
                if (unicriteria[delimetr].count (criteria) != 0) {
                    // Критерий уже встречался среди игнорируемых - используем ту же позицию
                    delimetrList[delimetr][RENUM_ADD][criteria].mostFrequentPos = unicriteria[delimetr][criteria];
                } else {
                    // Критерий новый - генерируем уникальную позицию
                    RenumPos pos = GetPos (unicpos, unicriteria, delimetr, criteria);
                    delimetrList[delimetr][RENUM_ADD][criteria].mostFrequentPos = pos;
                    maxposdelim.SetToMax (pos);
                }
            }
        }

        // Получаем позиции новых элементов
        if (tv.count (RENUM_NORMAL) != 0) {
            for (Values::iterator k = tv[RENUM_NORMAL].begin (); k != tv[RENUM_NORMAL].end (); ++k) {
                std::string criteria = k->first;
                // Расставляем позиции для элементов, для которых есть подходящая по критериям позиция
                if (unicriteria[delimetr].count (criteria) != 0) {
                    delimetrList[delimetr][RENUM_NORMAL][criteria].mostFrequentPos = unicriteria[delimetr][criteria];
                } else {
                    RenumPos pos = GetPos (unicpos, unicriteria, delimetr, criteria);
                    delimetrList[delimetr][RENUM_NORMAL][criteria].mostFrequentPos = pos;
                    maxposdelim.SetToMax (pos);
                }
            }
        }
        maxposall.SetToMax (maxposdelim);
        maxpos[delimetr] = maxposdelim;
    }
    #if defined(TESTING)
    DBprnt ("        stage 3");
    #endif
    // СТАДИЯ 3: Записываем итоговые позиции в paramToWriteelem
    // Форматируем позиции с учётом настроек нулей/пробелов (nulltype, nullcount)
    // Сравниваем новую позицию с текущей - если изменилась, добавляем в список на запись
    // Финишная прямая. Берём позиции из словаря и расставляем значения.
    GS::UniString rawname_position = rule.position;
    RenumPos maxposdelim;
    if (rule.nulltype == ADDMAXZEROS || rule.nulltype == ADDMAXSPACE)
        maxposdelim = maxposall;

    for (auto &i : delimetrList) {
        TypeValues &tv = i.second;
        // Обрабатываем только добавляемые (RENUM_ADD) и новые (RENUM_NORMAL) элементы
        for (int renumTypeInt = RENUM_ADD; renumTypeInt <= RENUM_NORMAL; renumTypeInt++) {
            RenumMode renumType = static_cast<RenumMode> (renumTypeInt);
            if (tv.count (renumType) == 0)
                continue;
            // Для ADDZEROS/ADDSPACE берём максимум позиций в текущей разбивке
            if (rule.nulltype == ADDZEROS || rule.nulltype == ADDSPACE) {
                maxposdelim = maxpos[i.first];
            }
            for (const auto &k : tv[renumType]) {
                const std::string &criteria = k.first;
                const GS::Array<RenumPos> &eleminpos = k.second.elements;
                RenumPos pos = k.second.mostFrequentPos;
                pos.FormatToMax (maxposdelim, rule.nulltype, rule.nullcount);
                ParamValue posvalue = pos.ToParamValue (rawname_position);
                for (const auto &elem : eleminpos) {
                    const ParamDictValue *p = paramToReadelem.GetPtr (elem.guid);
                    if (p == nullptr) {
                        continue;
                    }
                    const ParamValue *parampositionptr = p->GetPtr (rawname_position);
                    if (parampositionptr == nullptr) {
                        continue;
                    }
                    ParamValue paramposition = *parampositionptr;
                    paramposition.isValid = true;
                    posvalue.val.type = paramposition.val.type;
                    if (paramposition != posvalue) {
                        rule.n_write += 1;
                        paramposition.val = posvalue.val;
                        ParamHelpers::AddParamValue2ParamDictElement (elem.guid, paramposition, paramToWriteelem);
                    }
                }
            }
        }
    }
    #if defined(TESTING)
    DBprnt ("    ReNumOneRule end");
    #endif
    return;
}

// Разделяет элементы правила по разбивке, типу нумерации и критерию
// Алгоритм:
// 1. Для каждого элемента в rule.elemts определяет режим нумерации через ReNumGetFlag
// 2. Получает значение разбивки (delimetr) и критерия (criteria) из свойств элемента
// 3. Группирует элементы в delimetrList[delimetr][state][criteria].elements
// Типы нумерации (state): RENUM_SKIP, RENUM_IGNORE, RENUM_ADD, RENUM_NORMAL
// Возвращает true, если есть элементы для обработки (не RENUM_SKIP)
bool ElementsSeparation (RenumRule &rule,
                         const ParamDictElement &paramToReadelem,
                         Delimetr &delimetrList,
                         bool &has_error) {
    if (!rule.state)
        return false;
    #if defined(TESTING)
    DBprnt ("    ElementsSeparation start");
    #endif
    bool flag = false;
    // Собираем значения свойств из criteria. Нам нужны только уникальные значения.
    for (const auto &guid : rule.elemts) {
        const ParamDictValue *params = paramToReadelem.GetPtr (guid);

        if (params == nullptr)
            continue;

        // Получаем указатели на нужные свойства элемента
        // rule.flag - свойство-флаг (определяет режим нумерации)
        // rule.position - свойство с текущей позицией
        // rule.criteria - свойство-критерий (по которому группируем элементы)
        // rule.delimetr - свойство-разбивка (опциональная группировка)
        const ParamValue *paramflag = params->GetPtr (rule.flag);
        const ParamValue *paramposition = params->GetPtr (rule.position);
        // Сразу проверим режим нумерации элемента
        RenumMode state = RENUM_SKIP;
        RenumPos pos;
        if (paramflag != nullptr && paramposition != nullptr) {
            const ParamValue &flag = *paramflag;
            const ParamValue &position = *paramposition;
            if (!flag.isValid) {
                msg_rep (
                    "ReNumSelected", "Skip element with not valid value in flag: " + rule.flag, APIERR_GENERAL, guid);
                has_error = true;
                rule.n_error += 1;
                state = RENUM_SKIP;
            } else {
                state = ReNumGetFlag (flag, position);
            }
            if (state != RENUM_SKIP) {
                if (!position.isValid) {
                    msg_rep ("ReNumSelected",
                             "Skip element with not valid position: " + rule.position,
                             APIERR_GENERAL,
                             guid);
                    has_error = true;
                    rule.n_error += 1;
                    state = RENUM_SKIP;
                } else {
                    if (state != RENUM_SKIP)
                        pos = RenumPos (position);
                }
            }
        }

        // Получаем разделитель (delimetr), если он задан в правиле
        std::string delimetr;
        const ParamValue *paramdelimetr = params->GetPtr (rule.delimetr);
        if (paramdelimetr != nullptr) {
            if (paramdelimetr->isValid) {
                GSCharCode chcode = GetCharCode (paramdelimetr->val.uniStringValue);
                delimetr = paramdelimetr->val.uniStringValue.ToCStr (0, MaxUSize, chcode).Get ();
            } else {
                has_error = has_error || (state != RENUM_SKIP);
                if (has_error) {
                    msg_rep ("ReNumSelected",
                             "Skip element with not valid value in delimetr: " + rule.delimetr,
                             APIERR_GENERAL,
                             guid);
                    rule.n_error += 1;
                }
                state = RENUM_SKIP;
            }
        }

        // Получаем критерий (criteria), если он задан в правиле
        std::string criteria;
        const ParamValue *paramcriteria = params->GetPtr (rule.criteria);
        if (paramcriteria != nullptr) {
            if (paramcriteria->isValid) {
                if (paramcriteria->val.uniStringValue.IsEmpty ()) {
                    has_error = has_error || (state != RENUM_SKIP);
                    if (has_error) {
                        msg_rep ("ReNumSelected",
                                 "Skip element with empty value in criteria: " + rule.criteria,
                                 APIERR_GENERAL,
                                 guid);
                        rule.n_error += 1;
                    }
                    state = RENUM_SKIP;
                } else {
                    GSCharCode chcode = GetCharCode (paramcriteria->val.uniStringValue);
                    criteria = paramcriteria->val.uniStringValue.ToCStr (0, MaxUSize, chcode).Get ();
                }
            } else {
                msg_rep ("ReNumSelected",
                         "Skip element with not valid value in criteria: " + rule.criteria,
                         APIERR_GENERAL,
                         guid);
                state = RENUM_SKIP;
                has_error = true;
                rule.n_error += 1;
            }
        }
        if (state != RENUM_SKIP) {
            if (state == RENUM_IGNORE)
                rule.n_ignore += 1;
            if (delimetrList.count (delimetr) == 0)
                delimetrList[delimetr] = {};
            if (delimetrList[delimetr].count (state) == 0)
                delimetrList[delimetr][state] = {};
            if (delimetrList[delimetr][state].count (criteria) == 0)
                delimetrList[delimetr][state][criteria] = {};
            delimetrList[delimetr][state][criteria].elements.Push (pos);
            flag = true;
        } else {
            rule.n_skip += 1;
        }
    }
    #if defined(TESTING)
    DBprnt ("    ElementsSeparation end");
    #endif
    return flag;
}

//------------------------------------------------------------------------------------------------------------
// Проверяет - есть ли хоть одно описание флага Renum_flag в массиве определений свойств
// Используется для быстрой проверки, есть ли у элемента свойства с правилами нумерации
// Возвращает true, если найдено хотя бы одно свойство с Renum_flag в описании
//------------------------------------------------------------------------------------------------------------
bool ReNumHasFlag (const GS::Array<API_PropertyDefinition> definitions) {
    if (definitions.IsEmpty ())
        return false;
    for (const auto &definition : definitions) {
        if (definition.description.IsEmpty ())
            continue;
        if (definition.description.Contains (RENUMFLAG)) {
            return true;
        }
    }
    return false;
}

// -----------------------------------------------------------------------------------------------------------------------
// Функция возвращает режим нумерации элемента на основе значений свойств флага и позиции
// Режимы:
//   RENUM_SKIP (-1)    - исключить элемент из обработки
//   RENUM_IGNORE (0)   - не менять позицию, но учитывать элемент при группировке
//   RENUM_ADD (1)      - не менять позицию, если нет пропусков в нумерации
//   RENUM_NORMAL (2)   - обычная нумерация/перенумерация
// Логика для булевых свойств: true -> RENUM_NORMAL (если редактируемо), false -> RENUM_SKIP
// Логика для строковых свойств:
//   содержит "skip" -> RENUM_SKIP
//   содержит "ignore" -> RENUM_IGNORE
//   позиция не равна 0 -> RENUM_ADD
//   иначе -> RENUM_NORMAL
// -----------------------------------------------------------------------------------------------------------------------
RenumMode ReNumGetFlag (const ParamValue &paramflag, const ParamValue &paramposition) {
    if (!paramflag.isValid)
        return RENUM_SKIP;
    SyncSettings syncSettings;
    bool isEditable = IsElementEditable (paramflag.fromGuid, syncSettings, false);
    if (paramflag.type == API_PropertyBooleanValueType) {
        if (paramflag.val.boolValue) {
            if (isEditable) {
                return RENUM_NORMAL;
            } else {
                msg_rep ("ReNumSelected",
                         "Ignore not editable element for rule: " + paramflag.name,
                         NoError,
                         paramflag.fromGuid);
                return RENUM_IGNORE;
            }
        } else {
            return RENUM_SKIP;
        }
    }
    if (paramflag.type == API_PropertyStringValueType) {
        GS::UniString flag = paramflag.val.uniStringValue.ToLowerCase ();
        const Int32 iseng = ID_ADDON_STRINGS + isEng ();
        // Исключаемые позиции
        GS::UniString txtypenum = RSGetIndString (iseng, RenumSkipID, ACAPI_GetOwnResModule ());
        if (flag.Contains (txtypenum) || flag.Contains ("skip"))
            return RENUM_SKIP;

        // У нередактируемых элементов нет возможности поменять позицию - просто учтём её
        if (!isEditable) {
            msg_rep ("ReNumSelected",
                     "Ignore not editable element for rule: " + paramflag.name,
                     NoError,
                     paramflag.fromGuid);
            return RENUM_IGNORE;
        }
        // Неизменные позиции
        txtypenum = RSGetIndString (iseng, RenumIgnoreID, ACAPI_GetOwnResModule ());
        if (flag.Contains (txtypenum) || flag.Contains ("ignore"))
            return RENUM_IGNORE;

        // Пустые позиции (если строка пустая - значение ноль.)
        if (paramposition.val.intValue != 0)
            return RENUM_ADD;

        // Все прочие
        return RENUM_NORMAL;
    }
    return RENUM_SKIP;
}
#endif
