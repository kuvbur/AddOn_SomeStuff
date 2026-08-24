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
    GS::Array<API_Guid> guidArray = GetSelectedElements (true, false, syncSettings, true, false, false);
    if (guidArray.IsEmpty ())
        return NoError;
    // Если выбран только один элемент - правило берётся из него одного
    bool rule_from_one = (guidArray.GetSize () == 1);
    // Таблица определений свойств-правил (заполняется в GetRuleFromSelected)
    GS::HashTable<API_Guid, API_PropertyDefinition> rule_definitions = {};
    // Ищем свойства с флагом Renum_flag среди выбранных элементов
    if (!GetRuleFromSelected (guidArray, rule_definitions, RENUMFLAG, false)) {
        msg_rep ("ReNumSelected",
                 "No Num rule found.\nCheck that the description of the user property contains Renum_flag",
                 NoError,
                 APINULLGuid,
                 true);
        return NoError;
    }
    // Словарь для записи новых значений (guid элемента -> имя свойства -> новое значение)
    ParamDictElement paramToWriteelem = {};
    // Основная обработка: сбор данных, диалог, чтение, распределение позиций
    if (!GetRenumElements (guidArray, paramToWriteelem, rule_definitions, rule_from_one)) {
        msg_rep ("ReNumSelected", "No data to write", NoError, APINULLGuid);
        return NoError;
    }
    UInt32 qtywrite = paramToWriteelem.GetSize ();
    GS::UniString subtitle = GS::UniString::Printf ("Writing data to %d elements", qtywrite);
    short i = 2;
    #ifdef ServerMainVers_2700
    bool showPercent = false;
    Int32 maxval = 2;
    ACAPI_ProcessWindow_SetNextProcessPhase (&subtitle, &maxval, &showPercent);
    #else
    ACAPI_Interface (APIIo_SetNextProcessPhaseID, &subtitle, &i);
    #endif

    #if defined(TESTING)
    DBprnt ("Write start");
    #endif

    #ifdef ServerMainVers_2300
    bool suspGrp = false;
        #ifdef ServerMainVers_2700
    err = ACAPI_View_IsSuspendGroupOn (&suspGrp);
    if (err != NoError) {
        msg_rep ("ReNumSelected", "ACAPI_View_IsSuspendGroupOn", err, APINULLGuid);
        err = NoError;
    }
    if (!suspGrp)
        err = ACAPI_Grouping_Tool (guidArray, APITool_SuspendGroups, nullptr);
        #else
    err = ACAPI_Environment (APIEnv_IsSuspendGroupOnID, &suspGrp);
    if (err != NoError) {
        msg_rep ("ReNumSelected", "APIEnv_IsSuspendGroupOnID", err, APINULLGuid);
        err = NoError;
    }
    if (!suspGrp)
        err = ACAPI_Element_Tool (guidArray, APITool_SuspendGroups, nullptr);
        #endif
    #endif
    if (err != NoError) {
        msg_rep ("ReNumSelected", "APITool_SuspendGroups", err, APINULLGuid);
        err = NoError;
    }
    // Запоминаем: режим Suspend Groups включили МЫ (он был выключен до записи),
    // чтобы после записи вернуть состояние пользователя.
    const bool weSuspendedGroups = !suspGrp;
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
            return err;
        }
    }
    #if defined(TESTING)
    DBprnt ("Write end");
    #endif
    // Восстанавливаем режим Suspend Groups: APITool_SuspendGroups — тумблер
    // On/Off (док. AC25 ACAPI_Element_Tool), поэтому перед выключением
    // перечитываем состояние на случай ручного переключения пользователем.
    if (weSuspendedGroups) {
        bool suspNow = true;
    #ifdef ServerMainVers_2700
        if (ACAPI_View_IsSuspendGroupOn (&suspNow) == NoError && suspNow)
            ACAPI_Grouping_Tool (guidArray, APITool_SuspendGroups, nullptr);
    #else
        if (ACAPI_Environment (APIEnv_IsSuspendGroupOnID, &suspNow, nullptr) == NoError && suspNow)
            ACAPI_Element_Tool (guidArray, APITool_SuspendGroups, nullptr);
    #endif
    }
    SyncArray (syncSettings, guidArray);
    finish = clock ();
    duration = (double)(finish - start) / CLOCKS_PER_SEC;
    msg_rep ("ReNumSelected", GS::UniString::Printf ("Time spent %.3f s", duration), NoError, APINULLGuid);
    return NoError;
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
// 6. Формируем paramToWriteelem с новыми позициями
// Возвращает true, если есть данные для записи
bool GetRenumElements (GS::Array<API_Guid> &guidArray,
                       ParamDictElement &paramToWriteelem,
                       GS::HashTable<API_Guid, API_PropertyDefinition> &rule_definitions,
                       bool &rule_from_one) {
    #if defined(TESTING)
    DBprnt ("GetRenumElements start");
    #endif
    // Получаем список правил суммирования
    Rules rules = {};
    ParamDictElement paramToReadelem = {};
    bool hasRule = !rule_definitions.IsEmpty ();
    const Int32 iseng = ID_ADDON_STRINGS + isEng ();
    GS::UniString subtitle = GS::UniString::Printf ("Reading data from %d elements", guidArray.GetSize ());
    ParamDict error_propertyname = {};
    int n_elem = 0;
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
        ReNum_GetElement (guid, paramToReadelem, rules, error_propertyname, definitions);
    }

    if (!error_propertyname.IsEmpty ()) {
        GS::UniString out = ":\n";
        for (auto &cIt : error_propertyname) {
    #ifdef ServerMainVers_2800
            GS::UniString s = cIt.key;
    #else
            GS::UniString s = *cIt.key;
    #endif
            s.ReplaceAll (PVALPREFIX, EMPTYSTRING);
            s.ReplaceAll (BRACEEND, EMPTYSTRING);
            s.ReplaceAll (":", " : ");
            out.Append (s);
            out.Append (LINEBRAKE);
        }
        msg_rep ("ReNumSelected", "Can't find property, check name: " + out, APIERR_GENERAL, APINULLGuid);
        GS::UniString SpecEmptyListdString = RSGetIndString (iseng, 71, ACAPI_GetOwnResModule ());
        ACAPI_WriteReport (SpecEmptyListdString + out, true);
        return false;
    }
    if (!paramToReadelem.IsEmpty ())
        msg_rep ("ReNumSelected",
                 GS::UniString::Printf ("Find elements - %d ", paramToReadelem.GetSize ()),
                 NoError,
                 APINULLGuid);
    if (!rules.IsEmpty ())
        msg_rep ("ReNumSelected", GS::UniString::Printf ("Find rules - %d ", rules.GetSize ()), NoError, APINULLGuid);
    if (paramToReadelem.IsEmpty () || rules.IsEmpty ()) {
        if (paramToReadelem.IsEmpty ())
            msg_rep ("ReNumSelected", "Parameters for read not found", NoError, APINULLGuid);
        if (rules.IsEmpty ())
            msg_rep ("ReNumSelected", "Rules not found", NoError, APINULLGuid);
        GS::UniString SpecEmptyListdString = RSGetIndString (iseng, 75, ACAPI_GetOwnResModule ());
        ACAPI_WriteReport (SpecEmptyListdString, true);
        return false;
    }
    if (!RenumDG (rules, rule_from_one)) {
        msg_rep ("ReNumSelected", "Execution interrupted by user", NoError, APINULLGuid);
        return false;
    }
    ParamHelpers::ElementsRead (paramToReadelem); // Читаем значения
    bool has_error_ones = false;
    GS::UniString error_rule_name = "";
    GS::UniString ok_rule_name = "";
    // Теперь выясняем - какой режим нумерации у элементов и распределяем позиции
    for (GS::HashTable<API_Guid, RenumRule>::PairIterator cIt = rules.EnumeratePairs (); cIt != NULL; ++cIt) {
    #ifdef ServerMainVers_2800
        RenumRule &rule = cIt->value;
    #else
        RenumRule &rule = *cIt->value;
    #endif
        bool has_error = false;
        if (!rule.elemts.IsEmpty ())
            ReNumOneRule (rule, paramToReadelem, paramToWriteelem, has_error);
        if (has_error) {
            has_error_ones = true;
            error_rule_name.Append (rule.position);
            error_rule_name.Append (";\n");
        } else {
            ok_rule_name.Append (rule.position);
            ok_rule_name.Append (";\n");
        }
    }
    if (!error_rule_name.IsEmpty ()) {
        error_rule_name.ReplaceAll (PVALPREFIX, EMPTYSTRING);
        error_rule_name.ReplaceAll (BRACEEND, EMPTYSTRING);
        error_rule_name.ReplaceAll (":", EMPTYSTRING);
        error_rule_name.ReplaceAll (PROPERTYSTRING, EMPTYSTRING);
        error_rule_name.Trim ();
        error_rule_name = LINEBRAKE + error_rule_name;
    }
    if (!ok_rule_name.IsEmpty ()) {
        ok_rule_name.ReplaceAll (PVALPREFIX, EMPTYSTRING);
        ok_rule_name.ReplaceAll (BRACEEND, EMPTYSTRING);
        ok_rule_name.ReplaceAll (":", EMPTYSTRING);
        ok_rule_name.ReplaceAll (PROPERTYSTRING, EMPTYSTRING);
        ok_rule_name.Trim ();
        ok_rule_name = LINEBRAKE + ok_rule_name;
    }
    if (has_error_ones) {
        GS::UniString SpecEmptyListdString = RSGetIndString (iseng, 72, ACAPI_GetOwnResModule ()) + error_rule_name;
        ACAPI_WriteReport (SpecEmptyListdString, true);
    }
    if (paramToWriteelem.IsEmpty ()) {
        msg_rep ("ReNumSelected", "No position changes required", NoError, APINULLGuid);
        GS::UniString SpecEmptyListdString =
            RSGetIndString (iseng, 73, ACAPI_GetOwnResModule ()) + error_rule_name + ok_rule_name;
        ACAPI_WriteReport (SpecEmptyListdString, true);
        return false;
    }
    GS::UniString msg = GS::UniString::Printf ("%d", paramToWriteelem.GetSize ()) + ok_rule_name;
    GS::UniString SpecEmptyListdString = RSGetIndString (iseng, 74, ACAPI_GetOwnResModule ());
    ACAPI_WriteReport (SpecEmptyListdString + msg, true);
    msg_rep ("ReNumSelected",
             GS::UniString::Printf ("Elements with new position  - %d ", paramToWriteelem.GetSize ()),
             NoError,
             APINULLGuid);
    #if defined(TESTING)
    DBprnt ("GetRenumElements end");
    #endif
    return !paramToWriteelem.IsEmpty ();
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
                       ParamDict &error_propertyname,
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
                    ruleparamName = ruleparamName.ToLowerCase ().GetSubstring (CHARBRACESTART, CHARBRACEEND, 0);
                    GS::UniString rawNamecriteria = PVALPREFIX;
                    GS::UniString rawNamedelimetr = EMPTYSTRING;
                    if (ruleparamName.Contains (SEMICOLON)) { // Есть указание на нули
                        partstring.Clear ();
                        int nparam = StringSplt (ruleparamName, SEMICOLON, partstring, true, &local_scratch);
                        rawNamecriteria = rawNamecriteria + partstring[0] + BRACEEND;
                        if (nparam > 1)
                            rawNamedelimetr = PVALPREFIX + rawNamedelimetr + partstring[1] + BRACEEND;
                    } else {
                        rawNamecriteria = rawNamecriteria + ruleparamName + BRACEEND;
                    }
                    if (!rawNamecriteria.Contains (PROPERTYSTRING))
                        rawNamecriteria.ReplaceAll (BRACESTART, GDLNAMEPREFIX);
                    if (!rawNamedelimetr.IsEmpty () && !rawNamedelimetr.Contains (PROPERTYSTRING))
                        rawNamedelimetr.ReplaceAll (BRACESTART, GDLNAMEPREFIX);
                    // Если такие свойства есть - записываем правило
                    if (propertyParams.ContainsKey (rawNamecriteria) &&
                        (propertyParams.ContainsKey (rawNamedelimetr) || rawNamedelimetr.IsEmpty ())) {
                        rulecritetia.state = true;
                        rulecritetia.oldalgoritm = false;
                        rulecritetia.position = rawNameposition;
                        GS::UniString fname = EMPTYSTRING;
                        GS::UniString rawName = PROPERTYNAMEPREFIX;
                        GetPropertyFullName (definition, fname);
                        rawName.Append (fname.ToLowerCase ());
                        rawName.Append (BRACEEND);
                        rulecritetia.flag = rawName;
                        rulecritetia.criteria = rawNamecriteria;
                        rulecritetia.delimetr = rawNamedelimetr;
                        rulecritetia.guid = definition.guid;
                        rulecritetia.rule_name = definition.description;
                        partstring.Clear ();
                        if (StringSplt (rulecritetia.rule_name, BRACEEND, partstring, "enum_flag") > 0) {
                            rulecritetia.rule_name = partstring[0] + BRACEEND;
                        }
                        rulecritetia.rule_name = rulecritetia.rule_name.GetSubstring (CHARBRACESTART, CHARBRACEEND, 0);
                        rulecritetia.rule_name.ReplaceAll ("Property:", SPACESTRING);
                        rulecritetia.rule_name = CHARBSEMICOLON + rulecritetia.rule_name + CHARBSEMICOLON;
                        rulecritetia.rule_name =
                            rulecritetia.rule_name.GetSubstring (CHARBSEMICOLON, CHARBSEMICOLON, 0);
                        rulecritetia.rule_name.Trim ();
                    } else {
                        if (!propertyParams.ContainsKey (rawNamecriteria) &&
                            !error_propertyname.ContainsKey (rawNamecriteria))
                            error_propertyname.Add (rawNamecriteria, false);
                        if (!rawNamedelimetr.IsEmpty () && !propertyParams.ContainsKey (rawNamedelimetr) &&
                            !error_propertyname.ContainsKey (rawNamedelimetr))
                            error_propertyname.Add (rawNamedelimetr, false);
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
                if (!error_propertyname.ContainsKey (rawNameposition))
                    error_propertyname.Add (rawNameposition, false);
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
                    state = RENUM_SKIP;
                } else {
                    if (state != RENUM_SKIP)
                        pos = RenumPos (position);
                }
            }
        }

        // Получаем разделитель (delimetr), если он задан в правиле
        std::string delimetr = "";
        const ParamValue *paramdelimetr = params->GetPtr (rule.delimetr);
        if (paramdelimetr != nullptr) {
            if (paramdelimetr->isValid) {
                GSCharCode chcode = GetCharCode (paramdelimetr->val.uniStringValue);
                delimetr = paramdelimetr->val.uniStringValue.ToCStr (0, MaxUSize, chcode).Get ();
            } else {
                has_error = has_error || (state != RENUM_SKIP);
                if (has_error)
                    msg_rep ("ReNumSelected",
                             "Skip element with not valid value in delimetr: " + rule.delimetr,
                             APIERR_GENERAL,
                             guid);
                state = RENUM_SKIP;
            }
        }

        // Получаем критерий (criteria), если он задан в правиле
        std::string criteria = "";
        const ParamValue *paramcriteria = params->GetPtr (rule.criteria);
        if (paramcriteria != nullptr) {
            if (paramcriteria->isValid) {
                if (paramcriteria->val.uniStringValue.IsEmpty ()) {
                    has_error = has_error || (state != RENUM_SKIP);
                    if (has_error)
                        msg_rep ("ReNumSelected",
                                 "Skip element with empty value in criteria: " + rule.criteria,
                                 APIERR_GENERAL,
                                 guid);
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
