//------------ kuvbur 2022 ------------
#include "ACAPinc.h"

#include "api_headers/APIEnvir.h"

#include "spec/Spec.hpp"
// вклад источника (RuleContribution) — отдельный внутренний модуль.
#include "spec/SpecExecutor.hpp"
#include "spec/SpecHelpers.hpp"
#include "spec/SpecPlanning.hpp"
#include "Sync.hpp"
#ifdef TESTING
    #include "tests/TestFunc.hpp"
#endif
#include "dialogs/DG4rule.hpp"
#include "Propertycache.hpp"

namespace Spec {
    // --------------------------------------------------------------------
    // Получение правил из свойств элемента по умолчанию
    // Назначение: ищет правила спецификации в пользовательских свойствах объекта по умолчанию
    // Параметры:
    //   rules - [OUT] словарь правил (заполняется)
    //   homedatabaseInfo - информация о текущей базе данных (для фильтрации элементов)
    //   has_elementspec - [OUT] true, если найден хотя бы один элемент с включённым флагом
    // Алгоритм:
    //   1. Получает определения пользовательских свойств элемента по умолчанию (API_ObjectID)
    //   2. Ищет свойства, в описании которых есть "Spec_rule{...}"
    //   3. Для каждого правила находит элементы через классификацию (rule.rule_definitions.availability)
    //   4. Проверяет значение свойства-флага у каждого элемента (включён ли флаг)
    //   5. Если флаг включён - добавляет элемент в rule.runState.elements
    // Возвращает: true, если найдены элементы (даже если флаги выключены)
    // Примечание: для AC_22 всегда возвращает false
    // --------------------------------------------------------------------
    bool GetRuleFromDefaultElem (SpecRuleDict &rules,
                                 API_DatabaseInfo &homedatabaseInfo,
                                 bool &has_elementspec,
                                 bool showUserInterface) {
#ifndef ServerMainVers_2300
        return false;
#else
        GSErrCode error = NoError;
        GS::Array<API_PropertyDefinition> definitions = {};
    #ifdef ServerMainVers_2600
        error = ACAPI_Element_GetPropertyDefinitionsOfDefaultElem (
            API_ObjectID, API_PropertyDefinitionFilter_UserDefined, definitions);
    #else
        error = ACAPI_Element_GetPropertyDefinitionsOfDefaultElem (
            API_ObjectID, APIVarId_Generic, API_PropertyDefinitionFilter_UserDefined, definitions);
    #endif
        if (error != NoError) {
            msg_rep ("Spec", "ACAPI_Element_GetPropertyDefinitionsOfDefaultElem", error, APINULLGuid);
            return false;
        }
        for (UInt32 i = 0; i < definitions.GetSize (); i++) {
            GS::UniString description = definitions[i].description;
            if (!description.IsEmpty ()) {
                if (description.Contains ("Spec_rule") && description.Contains (BRACESTART) &&
                    description.Contains (BRACEEND)) {
                    AddRule (definitions[i], APINULLGuid, rules);
                }
            }
        }
        if (rules.IsEmpty ())
            return false;
        bool has_element = false;
        ParamDict error_name = {};
        for (GS::HashTable<GS::UniString, SpecRule>::PairIterator cIt = rules.EnumeratePairs (); cIt != NULL; ++cIt) {
    #ifdef ServerMainVers_2800
            SpecRule &rule = cIt->value;
    #else
            SpecRule &rule = *cIt->value;
    #endif
            if (!rule.parseValid)
                continue;
            for (const auto &classificationItemGuid : rule.rule_definitions.availability) {
                GS::Array<API_Guid> elemGuids = {};
                if (ACAPI_Element_GetElementsWithClassification (classificationItemGuid, elemGuids) != NoError)
                    continue;
                SpecFilter (elemGuids, homedatabaseInfo);
                for (const auto &elemGuid : elemGuids) {
                    API_Property propertyflag = {};
                    bool flagfindspec = false;
                    error = ACAPI_Element_GetPropertyValue (elemGuid, rule.rule_definitions.guid, propertyflag);
                    if (error != NoError) {
                        msg_rep ("Spec", "ACAPI_Element_GetPropertyValue", error, elemGuid);
                        continue;
                    }
    #ifndef ServerMainVers_2400
                    if (!propertyflag.isEvaluated) {
                        flagfindspec = true;
                    }
                    if (propertyflag.isDefault && !propertyflag.isEvaluated) {
                        flagfindspec = propertyflag.definition.defaultValue.basicValue.singleVariant.variant.boolValue;
                    } else {
                        flagfindspec = propertyflag.value.singleVariant.variant.boolValue;
                    }
    #else
                    if (propertyflag.status == API_Property_NotAvailable) {
                        flagfindspec = true;
                    }
                    if (propertyflag.isDefault && propertyflag.status == API_Property_NotEvaluated) {
                        flagfindspec = propertyflag.definition.defaultValue.basicValue.singleVariant.variant.boolValue;
                    } else {
                        flagfindspec = propertyflag.value.singleVariant.variant.boolValue;
                    }
    #endif
                    // Здесь важно различать три состояния: свойство выключено, недоступно или не оценено.
                    // Это влияет на то, будет ли элемент включён в спецификацию или отложен для сообщения пользователю.
                    if (flagfindspec) {
                        rule.runState.elements.PushNew (elemGuid);
                        has_elementspec = true;
                    } else {
                        if (!error_name.ContainsKey (propertyflag.definition.name))
                            error_name.Add (propertyflag.definition.name, true);
                    }
                    has_element = true;
                }
            }
        }
        if (has_element && !has_elementspec) {
            msg_rep ("Spec", "All elements is off", APIERR_GENERAL, APINULLGuid);
            if (showUserInterface) {
                const Int32 iseng = ID_ADDON_STRINGS + isEng ();
                GS::UniString SpecRuleNotFoundString = RSGetIndString (iseng, SpecFlagOff, ACAPI_GetOwnResModule ());
                if (!error_name.IsEmpty ()) {
                    for (auto &cIt : error_name) {
    #ifdef ServerMainVers_2800
                        GS::UniString s = cIt.key;
    #else
                        GS::UniString s = *cIt.key;
    #endif
                        SpecRuleNotFoundString.Append (LINEBRAKE);
                        SpecRuleNotFoundString.Append (s);
                    }
                }
                ACAPI_WriteReport (SpecRuleNotFoundString, true);
            }
        }
        return has_element;
#endif
    }

    GSErrCode SpecAll (const SyncSettings &syncSettings,
                       const GS::Array<GS::UniString> *ruleNames,
                       const Point2D *placementPoint,
                       SpecRunResult *runResult) {
        if (runResult != nullptr) {
            // includeDetails задаётся вызывающим - сброс не должен его стирать.
            const bool includeDetails = runResult->includeDetails;
            *runResult = {};
            runResult->includeDetails = includeDetails;
        }
        const bool showUserInterface = placementPoint == nullptr;
        GSErrCode err = NoError;
        API_DatabaseInfo homedatabaseInfo = {};
#ifdef ServerMainVers_2700
        err = ACAPI_Database_GetCurrentDatabase (&homedatabaseInfo);
#else
        err = ACAPI_Database (APIDb_GetCurrentDatabaseID, &homedatabaseInfo, nullptr);
#endif
        if (err != NoError) {
            msg_rep ("Spec", "APIDb_GetCurrentDatabaseID", err, APINULLGuid);
        }
        SpecRuleDict rules = {};
        bool hasrule = false;
        GS::Array<API_Guid> guidArray = GetSelectedElements (false, false, syncSettings, false, false, false);
        UnicGuid selected_elements = {};
        if (!guidArray.IsEmpty ()) {
            msg_rep ("Spec", "Create spec from selection", NoError, APINULLGuid);
            SpecFilter (guidArray, homedatabaseInfo);
            for (UInt32 i = 0; i < guidArray.GetSize (); i++) {
                if (!selected_elements.ContainsKey (guidArray[i]))
                    selected_elements.Add (guidArray[i], true);
            }
        }
        bool has_elementspec = false;
        if (guidArray.IsEmpty ())
            hasrule = GetRuleFromDefaultElem (rules, homedatabaseInfo, has_elementspec, showUserInterface);
        if (hasrule)
            msg_rep ("Spec", "Create spec from default element", NoError, APINULLGuid);
        if (guidArray.IsEmpty () && !hasrule) {
            err = ACAPI_Element_GetElemList (API_ZombieElemID,
                                             &guidArray,
                                             APIFilt_OnVisLayer | APIFilt_IsVisibleByRenovation |
                                                 APIFilt_IsInStructureDisplay);
            if (err != NoError)
                return err;
            if (!guidArray.IsEmpty ()) {
                SpecFilter (guidArray, homedatabaseInfo);
                msg_rep ("Spec", "Create spec from all visible element", NoError, APINULLGuid);
            }
        }
        // Если default element уже нашёл включённые элементы, они сохранены в rule.runState.elements
        // и должны быть обработаны SpecArray даже при пустом guidArray.
        if (guidArray.IsEmpty () && !has_elementspec)
            return NoError;
        err = SpecArray (syncSettings, guidArray, rules, selected_elements, ruleNames, placementPoint, runResult);
        return err;
    }

    // --------------------------------------------------------------------
    // Фильтрация одного элемента
    // Назначение: исключает элементы определённых типов и элементы из других баз данных
    // Отбрасывает элементы, которые не подходят для спецификации: размеры, тексты, линии и элементы
    // из другой базы данных. В случае неподходящего элемента GUID заменяется на APINULLGuid.
    // Параметры:
    //   elemguid - GUID элемента (изменяется на APINULLGuid, если элемент не подходит)
    //   homedatabaseInfo - информация о текущей базе данных (этаж, разрез и т.д.)
    // Алгоритм:
    //   1. Проверяет тип элемента - исключает размеры, тексты, линии, штриховки и т.д.
    //   2. Проверяет, находится ли элемент в той же базе данных, что и homedatabaseInfo
    //   3. Если элемент из другой базы данных - заменяет GUID на APINULLGuid
    // Возвращает: void (результат в elemguid)
    // --------------------------------------------------------------------
    void SpecFilter (API_Guid &elemguid, API_DatabaseInfo &homedatabaseInfo) {
        GSErrCode err = NoError;
        API_ElemTypeID elementType = GetElemTypeID (elemguid);

        if (elementType == API_ZombieElemID) {
            elemguid = APINULLGuid;
            return;
        }
        if (elementType == API_DimensionID) {
            elemguid = APINULLGuid;
            return;
        }
        if (elementType == API_RadialDimensionID) {
            elemguid = APINULLGuid;
            return;
        }
        if (elementType == API_LevelDimensionID) {
            elemguid = APINULLGuid;
            return;
        }
        if (elementType == API_AngleDimensionID) {
            elemguid = APINULLGuid;
            return;
        }
        if (elementType == API_TextID) {
            elemguid = APINULLGuid;
            return;
        }
        if (elementType == API_LabelID) {
            elemguid = APINULLGuid;
            return;
        }
        if (elementType == API_HatchID) {
            elemguid = APINULLGuid;
            return;
        }
        if (elementType == API_LineID) {
            elemguid = APINULLGuid;
            return;
        }
        if (elementType == API_PolyLineID) {
            elemguid = APINULLGuid;
            return;
        }
        if (elementType == API_ArcID) {
            elemguid = APINULLGuid;
            return;
        }
        if (elementType == API_CircleID) {
            elemguid = APINULLGuid;
            return;
        }
        if (elementType == API_SplineID) {
            elemguid = APINULLGuid;
            return;
        }
        if (elementType == API_HotspotID) {
            elemguid = APINULLGuid;
            return;
        }
        if (elementType == API_CutPlaneID) {
            elemguid = APINULLGuid;
            return;
        }
        if (elementType == API_CameraID) {
            elemguid = APINULLGuid;
            return;
        }
        if (elementType == API_CamSetID) {
            elemguid = APINULLGuid;
            return;
        }
        if (elementType == API_GroupID) {
            elemguid = APINULLGuid;
            return;
        }
        if (elementType == API_SectElemID) {
            elemguid = APINULLGuid;
            return;
        }
        if (elementType == API_DrawingID) {
            elemguid = APINULLGuid;
            return;
        };
        if (elementType == API_PictureID) {
            elemguid = APINULLGuid;
            return;
        }
        if (elementType == API_DetailID) {
            elemguid = APINULLGuid;
            return;
        }
        if (elementType == API_ElevationID) {
            elemguid = APINULLGuid;
            return;
        }
        if (elementType == API_InteriorElevationID) {
            elemguid = APINULLGuid;
            return;
        }
        if (elementType == API_WorksheetID) {
            elemguid = APINULLGuid;
            return;
        }
        if (elementType == API_ChangeMarkerID) {
            elemguid = APINULLGuid;
            return;
        }
        if (homedatabaseInfo.databaseUnId.elemSetId == APINULLGuid)
            return;
        API_DatabaseInfo elementdatabaseInfo = {};
#ifdef ServerMainVers_2700
        err = ACAPI_Database_GetContainingDatabase (&elemguid, &elementdatabaseInfo);
#else
        err = ACAPI_Database (APIDb_GetContainingDatabaseID, &elemguid, &elementdatabaseInfo);
#endif
        if (err == NoError) {
            if (elementdatabaseInfo.databaseUnId == homedatabaseInfo.databaseUnId) {
                return;
            } else {
                elemguid = APINULLGuid;
                GS::UniString name = elementdatabaseInfo.name;
                msg_rep ("SpecFilter", "Filter element in diff DB:", err, elemguid);
            }
        } else {
            msg_rep ("SpecFilter", "APIDb_GetCurrentDatabaseID", err, elemguid);
            return;
        }
    }

    // --------------------------------------------------------------------
    // Фильтрация массива элементов
    // Назначение: исключает элементы определённых типов и элементы из других баз данных
    // Параметры:
    //   guidArray - массив GUID элементов (изменяется - остаются только подходящие)
    //   homedatabaseInfo - информация о текущей базе данных
    // Алгоритм:
    //   1. Проверяет тип каждого элемента - исключает размеры, тексты, линии и т.д.
    //   2. Проверяет, находится ли элемент в той же базе данных
    //   3. Формирует новый массив только из подходящих элементов
    // Примечание: исходный массив заменяется на отфильтрованный
    // --------------------------------------------------------------------
    void SpecFilter (GS::Array<API_Guid> &guidArray, API_DatabaseInfo &homedatabaseInfo) {
        GSErrCode err = NoError;
        if (homedatabaseInfo.databaseUnId.elemSetId == APINULLGuid)
            return;
        API_DatabaseInfo elementdatabaseInfo = {};
        GS::Array<API_Guid> out = {};
        GS::UniString рname = GetDBName (homedatabaseInfo);
        for (UInt32 i = 0; i < guidArray.GetSize (); i++) {
            API_Guid elemguid = guidArray[i];
            API_ElemTypeID elementType = GetElemTypeID (elemguid);

            if (elementType == API_ZombieElemID)
                continue;
            if (elementType == API_DimensionID)
                continue;
            if (elementType == API_RadialDimensionID)
                continue;
            if (elementType == API_LevelDimensionID)
                continue;
            if (elementType == API_AngleDimensionID)
                continue;
            if (elementType == API_TextID)
                continue;
            if (elementType == API_LabelID)
                continue;
            if (elementType == API_HatchID)
                continue;
            if (elementType == API_LineID)
                continue;
            if (elementType == API_PolyLineID)
                continue;
            if (elementType == API_ArcID)
                continue;
            if (elementType == API_CircleID)
                continue;
            if (elementType == API_SplineID)
                continue;
            if (elementType == API_HotspotID)
                continue;
            if (elementType == API_CutPlaneID)
                continue;
            if (elementType == API_CameraID)
                continue;
            if (elementType == API_CamSetID)
                continue;
            if (elementType == API_GroupID)
                continue;
            if (elementType == API_SectElemID)
                continue;
            if (elementType == API_DrawingID)
                continue;
            if (elementType == API_PictureID)
                continue;
            if (elementType == API_DetailID)
                continue;
            if (elementType == API_ElevationID)
                continue;
            if (elementType == API_InteriorElevationID)
                continue;
            if (elementType == API_WorksheetID)
                continue;
            if (elementType == API_ChangeMarkerID)
                continue;
            BNZeroMemory (&elementdatabaseInfo, sizeof (API_DatabaseInfo));
#ifdef ServerMainVers_2700
            err = ACAPI_Database_GetContainingDatabase (&elemguid, &elementdatabaseInfo);
#else
            err = ACAPI_Database (APIDb_GetContainingDatabaseID, &elemguid, &elementdatabaseInfo);
#endif
            if (err == NoError) {
                if (elementdatabaseInfo.databaseUnId == homedatabaseInfo.databaseUnId) {
                    out.Push (elemguid);
                } else {
                    GS::UniString name = GetDBName (elementdatabaseInfo);
                    msg_rep ("SpecFilter", "Filter element in diff DB: " + рname + " <-> " + name, err, elemguid);
                }
            } else {
                msg_rep ("SpecFilter", "APIDb_GetCurrentDatabaseID", err, elemguid);
                continue;
            }
        }
        if (out.GetSize () != guidArray.GetSize ()) {
            guidArray.Clear ();
            guidArray = out;
        }
    }

    // --------------------------------------------------------------------
    // Диалог выбора правил спецификации
    // Назначение: показывает пользователю диалог с выбором правил для применения
    // Параметры:
    //   spec_rules - словарь правил (изменяется - пользователь может отключить некоторые правила)
    //   rule_from_one - флаг предупреждения (если true - показывает предупреждение)
    // Алгоритм:
    //   1. Формирует данные для диалога (RuleSelectData) из словаря правил
    //   2. Показывает диалог (RuleSelectDialog)
    //   3. Если пользователь нажал OK - обновляет выбор правил (rule.runState.selected)
    // Возвращает: true, если пользователь выбрал хотя бы одно правило и нажал OK
    // Примечание: если пользователь отменил диалог - возвращает false
    // --------------------------------------------------------------------
    bool SpecDG (SpecRuleDict &spec_rules, bool &rule_from_one) {
        RuleSelectData rules = {};
        for (GS::HashTable<GS::UniString, SpecRule>::PairIterator cIt = spec_rules.EnumeratePairs (); cIt != NULL;
             ++cIt) {
#ifdef ServerMainVers_2800
            const SpecRule &rule = cIt->value;
#else
            const SpecRule &rule = *cIt->value;
#endif
            if (!rule.parseValid || !rule.runState.selected)
                continue;
            if (rules.rules.ContainsKey (rule.rule_name))
                continue;
            if (rules.qty_elements.ContainsKey (rule.rule_name))
                continue;
            rules.rules.Add (rule.rule_name, true);
            rules.qty_elements.Add (rule.rule_name, GS::UniString::Printf ("%d", rule.runState.elements.GetSize ()));
        }
        rules.is_warn = rule_from_one;
        rules.titleResID = UndoSumId;
        RuleSelectDialog dialog (rules);
        if (!dialog.Invoke ())
            return false;
        bool has_true_state = false;
        for (GS::HashTable<GS::UniString, SpecRule>::PairIterator cIt = spec_rules.EnumeratePairs (); cIt != NULL;
             ++cIt) {
#ifdef ServerMainVers_2800
            SpecRule &rule = cIt->value;
#else
            SpecRule &rule = *cIt->value;
#endif
            if (!rule.parseValid || !rule.runState.selected)
                continue;
            if (!rules.rules.ContainsKey (rule.rule_name))
                continue;
            rule.runState.selected = rules.rules.Get (rule.rule_name);
            if (rule.runState.selected)
                has_true_state = true;
        }
        return has_true_state;
    }

    // --------------------------------------------------------------------
    // Основная функция создания спецификации из массива элементов
    // Назначение: обрабатывает элементы согласно правилам и создаёт/модифицирует элементы спецификации
    // Параметры:
    //   syncSettings - настройки синхронизации
    //   guidArray - массив GUID элементов для обработки
    //   rules - словарь правил спецификации (заполняется в процессе)
    //   selected_elements - выбранные пользователем элементы (для фильтрации существующих)
    // Алгоритм:
    //   1. Получает правила из свойств элементов (GetRuleFromElement)
    //   2. Формирует список параметров для чтения (GetParamToReadFromRule)
    //   3. Читает параметры избранного элемента (favorite_name)
    //   4. Показывает диалог выбора правил (SpecDG)
    //   5. Читает данные из всех элементов (ParamHelpers::ElementsRead)
    //   6. Формирует списки элементов для создания/модификации/удаления (GetElementsForRule)
    //   7. Размещает новые элементы (PlaceElements) - требует клика мышью
    //   8. Записывает свойства и удаляет устаревшие элементы (в undoable command)
    //   9. Синхронизирует созданные элементы (SyncArray)
    // Возвращает: код ошибки (NoError при успехе)
    // --------------------------------------------------------------------
    GSErrCode SpecArray (const SyncSettings &syncSettings,
                         GS::Array<API_Guid> &guidArray,
                         SpecRuleDict &rules,
                         const UnicGuid &selected_elements,
                         const GS::Array<GS::UniString> *ruleNames,
                         const Point2D *placementPoint,
                         SpecRunResult *runResult) {
        const bool showUserInterface = placementPoint == nullptr;
        if (runResult != nullptr) {
            // includeDetails задан вызывающим; сохраняем его при сбросе счётчиков.
            const bool includeDetails = runResult->includeDetails;
            *runResult = {};
            runResult->includeDetails = includeDetails;
        }
        clock_t start, finish;
        double duration;
        start = clock ();
        GS::UniString funcname = "SpecAll";
        GS::Int32 nPhase = 4;
        GS::UniString subtitle;
#ifdef ServerMainVers_2700
        Int32 maxval = 1;
        bool showPercent = true;
#else
        short i = 1;
#endif
        // набор прочитанных словарей один на весь запуск. До чтения
        // заполняется только поле read (запросы), composite/listData — выход
        // ParamHelpers::ElementsRead.
        SpecReadContext readContext = {};
        ParamDictValue paramToWrite = {};         // Словарь с параметрами для записи (с нулевым GUID)
        GS::Array<ElementDict> elements_new = {}; // Массив со словарём создаваемых элементов
        GS::Array<ElementDict> elements_mod = {}; // Массив со словарём модифицируемых элементов
        GS::Array<API_Guid> elements_delete = {}; // Массив удаляемых элементов
        ParamDictElement paramOut = {};           // Словарь свойств для записи в расставленные элементы
        GS::Array<API_Guid> guidArraysync = {};   // Список элементов, которые требуется синхронизировать (расставленные
                                                  // элементы)
        UnicGuid error_element = {};              // Элементы с ошибками
        ParamDict error_name = {};                // Список имён, не найденных у избранного
        GS::HashTable<GS::UniString, GS::HashTable<GS::UniString, GS::UniString>> paramdict_favorite =
            {}; // Словарь с именами параметров и описаниями свойств избранных элементов
        ProcessWindowGuard pwGuard (funcname, nPhase, showUserInterface);
        GSErrCode err = NoError;
        subtitle = GS::UniString::Printf ("Get rule from %d elements", guidArray.GetSize ());
        if (showUserInterface) {
#ifdef ServerMainVers_2700
            maxval = 2;
            ACAPI_ProcessWindow_SetNextProcessPhase (&subtitle, &maxval, &showPercent);
#else
            i = 2;
            ACAPI_Interface (APIIo_SetNextProcessPhaseID, &subtitle, &i);
#endif
        }
        const Int32 iseng = ID_ADDON_STRINGS + isEng ();
        if (!guidArray.IsEmpty ()) {
            bool flagfindspec = false;
            for (const auto &guid : guidArray) {
                err = GetRuleFromElement (guid, rules);
                if (err == NoError)
                    flagfindspec = true;
            }
            if (!flagfindspec) {
                msg_rep ("Spec", "Rules not found", APIERR_GENERAL, APINULLGuid);
                if (showUserInterface) {
                    GS::UniString SpecRuleNotFoundString =
                        RSGetIndString (iseng, SpecRuleNotFoundId, ACAPI_GetOwnResModule ());
                    ACAPI_WriteReport (SpecRuleNotFoundString, true);
                }
                if (runResult != nullptr)
                    runResult->prepareFailureStage = SpecPrepareStage::RulesNotFound;
                return APIERR_GENERAL;
            }
        }
        if (ruleNames != nullptr) {
            bool hasSelectedRule = false;
            for (GS::HashTable<GS::UniString, SpecRule>::PairIterator cIt = rules.EnumeratePairs (); cIt != NULL;
                 ++cIt) {
#ifdef ServerMainVers_2800
                SpecRule &rule = cIt->value;
#else
                SpecRule &rule = *cIt->value;
#endif
                if (!rule.parseValid || !rule.runState.destinationReady)
                    continue;
                rule.runState.selected = ruleNames->Contains (rule.rule_name);
                hasSelectedRule = hasSelectedRule || rule.runState.selected;
            }
            if (!hasSelectedRule) {
                msg_rep ("Spec", "Requested rules not found", APIERR_BADPARS, APINULLGuid);
                return APIERR_BADPARS;
            }
        }
        // Теперь пройдём по правилам и соберём все нужные параметры для чтения из исходных элементов.
        for (GS::HashTable<GS::UniString, SpecRule>::PairIterator cIt = rules.EnumeratePairs (); cIt != NULL; ++cIt) {
#ifdef ServerMainVers_2800
            SpecRule &rule = cIt->value;
#else
            SpecRule &rule = *cIt->value;
#endif
            // Читаем только параметры групп
            if (!rule.IsRunnableForRun ()) {
                continue;
            }
            GetParamToReadFromRule (rule, readContext.read, paramToWrite);
        }
        if (readContext.read.IsEmpty ()) {
            msg_rep ("Spec", "Parameters for read not found", APIERR_GENERAL, APINULLGuid);
            if (showUserInterface) {
                GS::UniString SpecRuleReadFoundString =
                    RSGetIndString (iseng, SpecRuleReadFoundId, ACAPI_GetOwnResModule ());
                ACAPI_WriteReport (SpecRuleReadFoundString, true);
            }
            if (runResult != nullptr)
                runResult->prepareFailureStage = SpecPrepareStage::ReadParamsNotFound;
            return APIERR_GENERAL;
        }
        if (paramToWrite.IsEmpty ()) {
            msg_rep ("Spec", "Parameters for write not found", APIERR_GENERAL, APINULLGuid);
            if (showUserInterface) {
                GS::UniString SpecWriteNotFoundString =
                    RSGetIndString (iseng, SpecWriteNotFoundId, ACAPI_GetOwnResModule ());
                ACAPI_WriteReport (SpecWriteNotFoundString, true);
            }
            if (runResult != nullptr)
                runResult->prepareFailureStage = SpecPrepareStage::WriteParamsNotFound;
            return APIERR_GENERAL;
        }
        subtitle = GS::UniString::Printf ("Reading parameters from %d elements", readContext.read.GetSize ());
        if (showUserInterface) {
#ifdef ServerMainVers_2700
            maxval = 2;
            ACAPI_ProcessWindow_SetNextProcessPhase (&subtitle, &maxval, &showPercent);
#else
            i = 2;
            ACAPI_Interface (APIIo_SetNextProcessPhaseID, &subtitle, &i);
#endif
        }
        // Читаем свойства избранного
        for (GS::HashTable<GS::UniString, SpecRule>::PairIterator cIt = rules.EnumeratePairs (); cIt != NULL; ++cIt) {
#ifdef ServerMainVers_2800
            SpecRule &rule = cIt->value;
#else
            SpecRule &rule = *cIt->value;
#endif
            if (!rule.parseValid || !rule.runState.selected)
                continue;
            GS::HashTable<GS::UniString, GS::UniString> *pRuleFavorite = paramdict_favorite.GetPtr (rule.favorite_name);
            if (pRuleFavorite == nullptr) {
                GS::HashTable<GS::UniString, GS::UniString> paramdict = {};
                err = GetElementForPlaceProperties (rule.favorite_name, paramdict);
                paramdict_favorite.Add (rule.favorite_name, paramdict);
                pRuleFavorite = paramdict_favorite.GetPtr (rule.favorite_name);
            }
            if (pRuleFavorite == nullptr)
                continue;
            // Сверка выходной схемы с избранным. Признак готовности ставится
            // функцией MatchDestinationProperties.
            if (!MatchDestinationProperties (rule, *pRuleFavorite, error_name))
                continue;
            // Поиск у избранного двух служебных свойств: носителя имени правила
            // и носителя GUID. Поиск проверяет оба свойства и
            // завершает обход после обнаружения обоих; значения читаются из кэша.
            const bool guidFound = ResolveFavoriteLinks (rule, *pRuleFavorite, paramToWrite);
            // Поиск существующих объектов
            if (!rule.delete_old)
                continue;
            if (rule.subguid_rulename.IsEmpty ())
                continue;
            if (!guidFound)
                continue;
            ParamValue subguid_pvalue;
            if (!ParamHelpers::GetParamValueFromCache (rule.subguid_rulename, subguid_pvalue)) {
#if defined(TESTING)
                DBprnt ("ERROR SpecArray - GetParamValueFromCache subguid_rulename", rule.subguid_rulename);
#endif
                continue;
            }
            GS::Array<API_Guid> exsist_elements =
                GetElementByPropertyDescription (subguid_pvalue.definition, rule.subguid_rulevalue.ToLowerCase ());
            SelectExistingElements (rule, exsist_elements, selected_elements);
            if (rule.runState.exsist_elements.IsEmpty ())
                continue;
            // Собираем список параметрв для чтения у существующих элементов
            AddExistingReadRequests (rule, rule.runState.exsist_elements, readContext.read);
        }
        // Если для размещаемого объекта не удалось найти нужные параметры, дальнейшая работа бессмысленна.
        if (!error_name.IsEmpty ()) {
            GS::UniString out = ":\n";
            for (auto &cIt : error_name) {
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
            msg_rep ("Spec", "Can't find parameters in place element: " + out, err, APINULLGuid);
            if (showUserInterface) {
                GS::UniString SpecEmptyListdString =
                    RSGetIndString (iseng, SpecParamPlaceNotFoundId, ACAPI_GetOwnResModule ());
                ACAPI_WriteReport (SpecEmptyListdString + out, true);
            }
            if (runResult != nullptr)
                runResult->prepareFailureStage = SpecPrepareStage::PlaceParamsNotFound;
            return APIERR_GENERAL;
        }
        // Перед формированием итоговых элементов читаются данные уже размещённых объектов, чтобы их можно было сравнить
        // с правилами.
        if (placementPoint == nullptr) {
            bool rule_from_one = false;
            if (!SpecDG (rules, rule_from_one)) {
                msg_rep ("ReNumSelected", "Execution interrupted by user", NoError, APINULLGuid);
                if (runResult != nullptr)
                    runResult->prepareFailureStage = SpecPrepareStage::CanceledByUser;
                return APIERR_CANCEL;
            }
        }
        // Пакетное чтение выполняется после SpecDG.
        ParamHelpers::ElementsRead (readContext.read, readContext.composite, readContext.listData, true, true);
        // Массив со словарями элементов для создания по правилам
        Int32 n_elements = 0; // Количество создаваемых элементов для отчёта
        bool has_v2 = false;
        for (GS::HashTable<GS::UniString, SpecRule>::PairIterator cIt = rules.EnumeratePairs (); cIt != NULL; ++cIt) {
#ifdef ServerMainVers_2800
            SpecRule &rule = cIt->value;
#else
            SpecRule &rule = *cIt->value;
#endif
            if (!rule.IsRunnableForRun ())
                continue;
            ElementDict elements_n = {}; // Словарь создаваемых элементов для правила
            ElementDict elements_m = {};
            n_elements += GetElementsForRule (
                rule, readContext, elements_n, elements_m, elements_delete, error_element, showUserInterface);
            if (!elements_n.IsEmpty ())
                elements_new.Push (elements_n);
            if (!elements_m.IsEmpty ())
                elements_mod.Push (elements_m);
            if (rule.delete_old)
                has_v2 = true;
        }
        if (runResult != nullptr) {
            for (const ElementDict &elements : elements_new)
                runResult->elementsToCreate += elements.GetSize ();
            for (const ElementDict &elements : elements_mod)
                runResult->elementsToModify += elements.GetSize ();
            runResult->elementsToDelete = elements_delete.GetSize ();
            if (runResult->includeDetails)
                runResult->deleted = elements_delete;
        }
        SuspendGroupsGuard suspGuard;
#ifdef ServerMainVers_2300
        if (!error_element.IsEmpty ()) {
            if (showUserInterface) {
                if (error_element.GetSize () < 20) {
    #ifdef ServerMainVers_2700
                    ACAPI_UserInput_ClearElementHighlight ();
    #else
        #ifdef ServerMainVers_2600
                    ACAPI_Interface_ClearElementHighlight ();
        #else
                    ACAPI_Interface (APIIo_HighlightElementsID);
        #endif
    #endif
                    GS::HashTable<API_Guid, API_RGBAColor> hlElems = {};
                    API_RGBAColor hlColor = {1, 0.0, 0.0, 1};
                    GS::Array<API_Neig> error_elements = {};
                    for (const auto &cIt : error_element) {
    #ifdef ServerMainVers_2800
                        API_Guid el = cIt.key;
    #else
                        API_Guid el = *cIt.key;
    #endif
                        hlElems.Add (el, hlColor);
                        error_elements.PushNew (el);
                    }
    #ifdef ServerMainVers_2700
                    ACAPI_UserInput_SetElementHighlight (hlElems);
    #else
        #ifdef ServerMainVers_2600
                    ACAPI_Interface_SetElementHighlight (hlElems);
        #else
                    ACAPI_Interface (APIIo_HighlightElementsID, &hlElems);
        #endif
    #endif
    #ifdef ServerMainVers_2700
                    err = ACAPI_Selection_Select (error_elements, true);
                    if (err == NoError)
                        ACAPI_View_ZoomToSelected ();
    #else
                    err = ACAPI_Element_Select (error_elements, true);
                    if (err == NoError)
                        ACAPI_Automate (APIDo_ZoomToSelectedID);
    #endif
                } else {
                    msg_rep ("Spec",
                             GS::UniString::Printf ("Too many element for highlight - %d", error_element.GetSize ()),
                             err,
                             APINULLGuid);
                }
            }
            if (runResult != nullptr)
                runResult->prepareFailureStage = SpecPrepareStage::TooManyErrorElements;
            return APIERR_GENERAL;
        }
#endif
        if (!elements_mod.IsEmpty ()) {
            if (showUserInterface) {
#ifdef ServerMainVers_2700
                ACAPI_UserInput_ClearElementHighlight ();
#else
    #ifdef ServerMainVers_2600
                ACAPI_Interface_ClearElementHighlight ();
    #else
                ACAPI_Interface (APIIo_HighlightElementsID);
    #endif
#endif
            }
            GS::HashTable<API_Guid, API_RGBAColor> hlElems = {};
            API_RGBAColor hlColor = {0.8, 0.0, 0.0, 0.5};
            for (const auto &eldict : elements_mod) {
                for (auto &cIt : eldict) {
#ifdef ServerMainVers_2800
                    Element el = cIt.value;
#else
                    Element el = *cIt.value;
#endif
                    hlElems.Add (el.exs_guid, hlColor);
                    ParamDictValue param = {};
                    BuildRowParamToWrite (el, paramToWrite, param);
                    paramOut.Add (el.exs_guid, param);
                    // Дамп изменяемого элемента: GUID уже известен (элемент создан
                    // предыдущим запуском), в отличие от создаваемого.
                    if (runResult != nullptr && runResult->includeDetails) {
                        SpecElementDump dump = {};
                        dump.guid = el.exs_guid;
                        dump.favorite_name = el.favorite_name;
                        dump.sourceElements = el.elements;
                        FillDumpFromParamDict (param, dump);
                        runResult->modified.Push (dump);
                    }
                }
            }
            if (showUserInterface) {
#ifdef ServerMainVers_2700
                ACAPI_UserInput_SetElementHighlight (hlElems);
#else
    #ifdef ServerMainVers_2600
                ACAPI_Interface_SetElementHighlight (hlElems);
    #else
                ACAPI_Interface (APIIo_HighlightElementsID, &hlElems);
    #endif
#endif
            }
        }
        subtitle = GS::UniString::Printf ("Create %d elements", n_elements);
        if (showUserInterface) {
#ifdef ServerMainVers_2700
            maxval = 3;
            ACAPI_ProcessWindow_SetNextProcessPhase (&subtitle, &maxval, &showPercent);
#else
            i = 3;
            ACAPI_Interface (APIIo_SetNextProcessPhaseID, &subtitle, &i);
#endif
        }
        // Какое-то действие требуется: создать, изменить или удалить.
        bool has_action = false;
        for (const ElementDict &ed : elements_new) {
            if (!ed.IsEmpty ())
                has_action = true;
        }
        for (const ElementDict &ed : elements_mod) {
            if (!ed.IsEmpty ())
                has_action = true;
        }
        // Совпадение с уже существующими объектами без изменений — это штатный no-op,
        // а не «правило ничего не дало».
        bool rule_produced_rows = false;
        for (GS::HashTable<GS::UniString, SpecRule>::PairIterator cIt = rules.EnumeratePairs (); cIt != NULL; ++cIt) {
#ifdef ServerMainVers_2800
            const SpecRule &r = cIt->value;
#else
            const SpecRule &r = *cIt->value;
#endif
            if (r.IsRunnableForRun ())
                rule_produced_rows = true;
        }
        if (!has_action && !rule_produced_rows) {
            msg_rep ("Spec", "Elements list empty", NoError, APINULLGuid);
            if (showUserInterface) {
                GS::UniString SpecEmptyListdString = RSGetIndString (iseng, SpecEmptyListdId, ACAPI_GetOwnResModule ());
                if (has_v2)
                    SpecEmptyListdString += LINEBRAKE + RSGetIndString (iseng, 67, ACAPI_GetOwnResModule ());
                ACAPI_WriteReport (SpecEmptyListdString, true);
            }
            if (runResult != nullptr)
                runResult->prepareFailureStage = SpecPrepareStage::EmptyElementsList;
            return APIERR_GENERAL;
        }

        Point2D startpos = {0, 0};
        finish = clock ();
        duration = (double)(finish - start) / CLOCKS_PER_SEC;
        if (!elements_new.IsEmpty ()) {
            if (placementPoint == nullptr) {
                if (!ClickAPoint ("Click the lower corner of the spec elements creation", &startpos))
                    return APIERR_CANCEL;
            } else {
                startpos = *placementPoint;
            }
            start = clock ();
            const UInt32 previousCount = paramOut.GetSize ();
            PlaceElements (elements_new, paramToWrite, paramOut, startpos, runResult);
            const UInt32 createdCount = paramOut.GetSize () - previousCount;
            if (runResult != nullptr)
                runResult->elementsToCreate = createdCount;
            if (createdCount == 0 && elements_mod.IsEmpty () && elements_delete.IsEmpty ()) {
                msg_rep ("Spec", "Elements not created", APIERR_GENERAL, APINULLGuid);
                if (runResult != nullptr)
                    runResult->prepareFailureStage = SpecPrepareStage::NothingCreated;
                return APIERR_GENERAL;
            }
        } else {
            start = clock ();
        }
        // Запись свойств и удаление устаревших строк - отдельный этап (#228 R8.4).
        // Результат удаления возвращается этапом и присваивается накопителю
        // ошибок функции запуска: так ошибка удаления доходит до вызывающего,
        // как и до выделения. Sync выполняется ниже и в транзакцию записи не входит.
        err = WriteSpecProperties (elements_delete, paramOut, runResult);
        if (has_v2 && showUserInterface) {
            GS::UniString msg;
            if (!elements_delete.IsEmpty ()) {
                msg += RSGetIndString (iseng, 70, ACAPI_GetOwnResModule ()) +
                       GS::UniString::Printf (" %d \n", elements_delete.GetSize ());
            }
            if (!elements_new.IsEmpty ()) {
                UInt32 n_elem = 0;
                for (UInt32 i = 0; i < elements_new.GetSize (); i++) {
                    n_elem += elements_new[i].GetSize ();
                }
                msg += RSGetIndString (iseng, 68, ACAPI_GetOwnResModule ()) + GS::UniString::Printf (" %d \n", n_elem);
            }
            if (!elements_mod.IsEmpty ()) {
                UInt32 n_elem = 0;
                for (UInt32 i = 0; i < elements_mod.GetSize (); i++) {
                    n_elem += elements_mod[i].GetSize ();
                }
                msg += RSGetIndString (iseng, 69, ACAPI_GetOwnResModule ()) + GS::UniString::Printf (" %d \n", n_elem);
            }
            if (msg.IsEmpty ())
                msg = RSGetIndString (iseng, 67, ACAPI_GetOwnResModule ());
            ACAPI_WriteReport (msg, true);
        }

        for (ParamDictElement::PairIterator cIt = paramOut.EnumeratePairs (); cIt != NULL; ++cIt) {
#ifdef ServerMainVers_2800
            API_Guid elemGuid = cIt->key;
#else
            API_Guid elemGuid = *cIt->key;
#endif
            guidArraysync.Push (elemGuid);
        }
        SyncArray (syncSettings, guidArraysync);
        finish = clock ();
        duration += (double)(finish - start) / CLOCKS_PER_SEC;
        GS::UniString time = GS::UniString::Printf (" %.3f s", duration);
        GS::UniString intString = GS::UniString::Printf ("Qty elements - %d ", guidArray.GetSize ()) +
                                  GS::UniString::Printf ("wrtite to - %d", n_elements) + time;
        msg_rep (funcname, intString, err, APINULLGuid);
        return err;
    }

    // --------------------------------------------------------------------
    // Получение правил из свойств конкретного элемента
    // Назначение: ищет в свойствах элемента правила спецификации и добавляет их в словарь
    // Параметры:
    //   elemguid - GUID элемента, из свойств которого извлекаются правила
    //   rules - словарь правил (заполняется)
    // Алгоритм:
    //   1. Получает определения пользовательских свойств элемента
    //   2. Ищет свойства, в описании которых есть "Spec_rule" и фигурные скобки
    //   3. Проверяет значение свойства (включён ли флаг спецификации)
    //   4. Если флаг включён - вызывает AddRule для добавления правила
    // Возвращает: код ошибки (NoError, если правила найдены)
    // Примечание: правило добавляется только если значение свойства истинно (флаг включён)
    // --------------------------------------------------------------------
    GSErrCode GetRuleFromElement (const API_Guid &elemguid, SpecRuleDict &rules) {
        GSErrCode err = NoError;
        GS::Array<API_PropertyDefinition> definitions = {};
        err = ACAPI_Element_GetPropertyDefinitions (elemguid, API_PropertyDefinitionFilter_UserDefined, definitions);
        if (err != NoError || definitions.IsEmpty ())
            return err;
        for (UInt32 i = 0; i < definitions.GetSize (); i++) {
            GS::UniString description = definitions[i].description;
            if (description.IsEmpty ())
                continue;
            if (!description.Contains ("Spec_rule"))
                continue;
            if (!description.Contains (BRACESTART))
                continue;
            if (!description.Contains (BRACEEND))
                continue;
            bool flagfindspec = false;
            // Проверяем - включено ли свойство

            API_Property propertyflag = {};
            if (ACAPI_Element_GetPropertyValue (elemguid, definitions[i].guid, propertyflag) == NoError) {
#ifndef ServerMainVers_2400
                if (!propertyflag.isEvaluated) {
                    flagfindspec = true;
                }
                if (propertyflag.isDefault && !propertyflag.isEvaluated) {
                    flagfindspec = propertyflag.definition.defaultValue.basicValue.singleVariant.variant.boolValue;
                } else {
                    flagfindspec = propertyflag.value.singleVariant.variant.boolValue;
                }
#else
                if (propertyflag.status == API_Property_NotAvailable) {
                    flagfindspec = true;
                }
                if (propertyflag.isDefault && propertyflag.status == API_Property_NotEvaluated) {
                    flagfindspec = propertyflag.definition.defaultValue.basicValue.singleVariant.variant.boolValue;
                } else {
                    flagfindspec = propertyflag.value.singleVariant.variant.boolValue;
                }
#endif
            }
            if (flagfindspec)
                AddRule (definitions[i], elemguid, rules);
        }
        return NoError;
    }

    // --------------------------------------------------------------------
    // Добавление правила в словарь правил
    // Назначение: парсит описание свойства и добавляет правило в словарь
    // Параметры:
    //   definition - определение свойства (содержит описание с правилом)
    //   elemguid - GUID элемента, к которому относится правило (может быть APINULLGuid для default element)
    //   rules - словарь правил (заполняется)
    // Алгоритм:
    //   1. Очищает описание от лишних символов (переводы строк, табуляция, двойные пробелы)
    //   2. Форматирует строку (заменяет "g(" на "g@@", "s(" на "s@@" и т.д. для упрощения парсинга)
    //   3. Извлекает ключ - текст в фигурных скобках {....}
    //   4. Если правило с таким ключом уже существует - добавляет элемент в существующее правило
    //   5. Иначе вызывает GetRuleFromDescription для парсинга и создания нового правила
    // Примечание: правило добавляется в словарь даже если разбор не удался (parseValid=false),
    //             чтобы избежать повторной обработки
    // --------------------------------------------------------------------
    GS::UniString NormalizeRuleDescription (const GS::UniString &source) {
        GS::UniString description = source;
        // Переводы строк и табуляции убираем: описание набирается в несколько строк.
        description.ReplaceAll (LINEBRAKE, EMPTYSTRING);
        description.ReplaceAll (LINEBRAKER, EMPTYSTRING);
        description.ReplaceAll (TABSTRING, EMPTYSTRING);
        // Схлопываем кратные пробелы (шесть проходов - действующее ограничение).
        description.ReplaceAll ("  ", SPACESTRING);
        description.ReplaceAll ("  ", SPACESTRING);
        description.ReplaceAll ("  ", SPACESTRING);
        description.ReplaceAll ("  ", SPACESTRING);
        description.ReplaceAll ("  ", SPACESTRING);
        description.ReplaceAll ("  ", SPACESTRING);
        // Знаки подтягиваем вплотную: " {", "{ ", " }", "} ", " ;", "; ".
        description.ReplaceAll (" {", BRACESTART);
        description.ReplaceAll ("{ ", BRACESTART);
        description.ReplaceAll (" }", BRACEEND);
        description.ReplaceAll ("} ", BRACEEND);
        description.ReplaceAll (" ;", SEMICOLON);
        description.ReplaceAll ("; ", SEMICOLON);
        // Убираем пробел перед открывающей скобкой у всех видов групп.
        description.ReplaceAll ("gm (", "gm(");
        description.ReplaceAll (" gm(", "gm(");
        description.ReplaceAll ("gl (", "gl(");
        description.ReplaceAll (" gl(", "gl(");
        description.ReplaceAll ("g (", "g(");
        description.ReplaceAll ("s (", "s(");
        description.ReplaceAll (" g(", "g(");
        description.ReplaceAll (" s(", "s(");
        // Переписываем вызовы групп во внутренние маркеры. Этот блок обязан идти
        // ПОСЛЕ ужимания пробелов (см. комментарий функции).
        description.ReplaceAll ("g(", "g@@");
        description.ReplaceAll ("gl(", "g@@libdata@");
        description.ReplaceAll ("s(", "s@@");
        description.ReplaceAll (")s", "@@s");
        description.ReplaceAll (")g", "@@g");
        description.ReplaceAll ("))", ")@@");
        description.ReplaceAll ("gm(", "g@@Material_all@");
        return description;
    }

    // --------------------------------------------------------------------
    // Разбирает описание свойства и добавляет правило в словарь.
    // Описание сперва нормализуется (см. NormalizeRuleDescription), затем обрезается
    // по первой закрывающей скобке и ищется уже в словаре: повторное описание
    // только дополняет список элементов и НЕ пересобирает правило заново.
    // Правило добавляется в словарь даже невалидным - чтобы не обработать его дважды.
    // --------------------------------------------------------------------
    void AddRule (const API_PropertyDefinition &definition, const API_Guid &elemguid, SpecRuleDict &rules) {
        GS::UniString description = NormalizeRuleDescription (definition.description);
        GS::Array<GS::UniString> partstring = {};
        if (StringSplt (description, BRACEEND, partstring, "pec_rule") > 0) {
            description = partstring[0] + BRACEEND;
        }
        GS::UniString key = description.GetSubstring (CHARBRACESTART, CHARBRACEEND, 0);
        if (rules.ContainsKey (key)) {
            if (rules.Get (key).parseValid && elemguid != APINULLGuid)
                rules.Get (key).runState.elements.Push (elemguid);
        } else {
            // Добавление группы и элемента
            SpecRule rule = GetRuleFromDescription (description);
            if (rule.parseValid) {
                GS::UniString fname;
                GetPropertyFullName (definition, fname);
                rule.subguid_paramrawname = fname;
                rule.subguid_rulevalue = fname;
                rule.rule_definitions = definition;
                rule.rule_name = fname;
                if (elemguid != APINULLGuid)
                    rule.runState.elements.Push (elemguid);
            }
            rules.Add (key, rule); // Добавляем в любом случае, чтоб потом дважды не обрабатывать
            if (rule.parseValid) {
                msg_rep ("Spec", "Find correct rule: " + definition.name, NoError, APINULLGuid);
            } else {
                // Исходное описание печатается ЗДЕСЬ, а не берётся из парсера:
                // парсер работает на нормализованной копии и наружу её не отдаёт
                // а правило может быть отвергнуто и в AddRule-канале.
                // Текст причины — константа ParseErrorText, описание не копируется
                // повторно, сообщение собирается только для невалидного правила.
                msg_rep ("Spec",
                         "Rule is not valid: " + definition.name + " (" + ParseErrorText (rule.parseError) +
                             "): " + definition.description,
                         APIERR_GENERAL,
                         APINULLGuid);
            }
        }
    }

    // --------------------------------------------------------------------
    // Зависимости правила: что читать у источников и что потом записывать
    // CollectRuleDependencies перечисляет зависимости, BuildReadParamDict
    // разворачивает имена в словарь элемента, GetParamToReadFromRule
    // объединяет запросы для всего запуска. Порядок и кратность чтения значимы.
    //
    // Сбор зависимостей не обращается к модели - это объявление данных, а не
    // их чтение. Объединение запросов делает вызывающий, потому что словари
    // Словари запросов на чтение и на запись у всего запуска общие.
    // --------------------------------------------------------------------
    RuleDependencies CollectRuleDependencies (const SpecRule &rule) {
        RuleDependencies dependencies = {};
        for (const GroupSpec &group : rule.groups) {
            // Для материалов и компонент читаем только 0 группу - остальные одинаковые
            if ((group.fromLibData || group.fromMaterial) && group.n_layer > 0) {
                continue;
            }
            // Флаг добавляется ДО проверки is_Valid: элементы невалидной группы
            // всё равно отбрасываются по значению флага, поэтому читать его надо.
            // Пустое имя флага попадает в набор, но BuildReadParamDict его
            // пропускает (AddValueToParamDictValue игнорирует пустое имя) -
            // поведение прежнее.
            if (!dependencies.read.ContainsKey (group.flag_paramrawname))
                dependencies.read.Add (group.flag_paramrawname, true);
            if (!group.is_Valid) {
                continue;
            }
            for (const GS::UniString &rawname : group.sum_paramrawname) {
                // Литерал "1" - счётчик источников, читать нечего.
                if (!dependencies.read.ContainsKey (rawname) && !rawname.IsEqual ("1"))
                    dependencies.read.Add (rawname, true);
            }
            for (const GS::UniString &rawname : group.unic_paramrawname) {
                if (!dependencies.read.ContainsKey (rawname))
                    dependencies.read.Add (rawname, true);
            }
            for (const GS::UniString &rawname : group.out_paramrawname) {
                if (!dependencies.read.ContainsKey (rawname))
                    dependencies.read.Add (rawname, true);
            }
        }
        for (const GS::UniString &rawname : rule.out_sum_paramrawname) {
            if (!dependencies.write.ContainsKey (rawname))
                dependencies.write.Add (rawname, true);
        }
        for (const GS::UniString &rawname : rule.out_paramrawname) {
            if (!dependencies.write.ContainsKey (rawname))
                dependencies.write.Add (rawname, true);
        }
        return dependencies;
    }

    // Разворачивает собранные имена в словарь параметров ОДНОГО элемента.
    // Обычные имена добавляются как есть; имена материалов и формул требуют
    // разбора выражения, служебных свойств слоя и самой формулы - поэтому
    // они здесь, а не в CollectRuleDependencies, который остаётся чистым
    // перечислением имён.
    void BuildReadParamDict (const ParamDict &readNames, ParamDictValue &paramDict) {
        for (const auto &cItt : readNames) {
#ifdef ServerMainVers_2800
            const GS::UniString rawname = cItt.key;
#else
            const GS::UniString rawname = *cItt.key;
#endif
            if (!rawname.Contains (MATERIALNAMEPREFIX) && !rawname.Contains (FORMULANAMEPREFIX)) {
                ParamHelpers::AddValueToParamDictValue (paramDict, rawname);
                continue;
            }
            // Добавление материалов
            GS::UniString t = rawname;
            if (rawname.Contains (MATERIALNAMEPREFIX)) {
                t.ReplaceAll ("{@material:layers_auto,all;", BRACESTART);
                t = t.GetSubstring (CHARBRACESTART, CHARBRACEEND, 0);
                ParamHelpers::ParseParamNameMaterial (t, paramDict);
                ParamHelpers::AddValueToParamDictValue (paramDict, MAT_SOME_STUFF_TH);
                ParamHelpers::AddValueToParamDictValue (paramDict, MAT_SOME_STUFF_UNITS);
                ParamHelpers::AddValueToParamDictValue (paramDict, MAT_SOME_STUFF_KZAP);
                for (UInt32 inx = 0; inx < 20; inx++) {
                    ParamHelpers::AddValueToParamDictValue (paramDict,
                                                            "@property:sync_name" + GS::UniString::Printf ("%d", inx));
                }
                ParamValue param = {};
                param.rawName = rawname;
                param.val.uniStringValue = t;
                param.composite_pen = -2;
                param.fromQuantity = true;
                param.fromMaterial = true;
                ParamHelpers::AddParamValue2ParamDict (APINULLGuid, param, paramDict);
            } else {
                if (rawname.Contains (FORMULANAMEPREFIX)) {
                    t.ReplaceAll (FORMULANAMEPREFIX, EMPTYSTRING);
                    ParamHelpers::ParseParamNameMaterial (t, paramDict);
                    ParamValue param = {};
                    param.rawName = rawname;
                    param.val.uniStringValue = t;
                    param.val.hasFormula = true;
                    ParamHelpers::AddParamValue2ParamDict (APINULLGuid, param, paramDict);
                }
            }
        }
    }

    // Адаптер: собирает зависимости правила и сливает их в общие
    // словари запуска. Объединение запросов между правилами остаётся здесь
    // (AddParamDictValue2ParamDictElement не трогает уже набранное), поэтому
    // кратность чтения компонентов и порядок добавления значимы.
    void GetParamToReadFromRule (SpecRule &rule, ParamDictElement &paramToRead, ParamDictValue &paramToWrite) {
        const RuleDependencies dependencies = CollectRuleDependencies (rule);
        ParamDictValue paramDict = {}; // Словарь параметров для чтения для одного элемента
        BuildReadParamDict (dependencies.read, paramDict);
        // Добавляем параметры для каждого элемента
        if (!paramDict.IsEmpty ()) {
            for (const API_Guid elemguid : rule.runState.elements) {
                ParamHelpers::AddParamDictValue2ParamDictElement (elemguid, paramDict, paramToRead);
            }
        }
        for (const auto &cItt : dependencies.write) {
#ifdef ServerMainVers_2800
            const GS::UniString rawname = cItt.key;
#else
            const GS::UniString rawname = *cItt.key;
#endif
            ParamHelpers::AddValueToParamDictValue (paramToWrite, rawname);
        }
    }

    // --------------------------------------------------------------------
    // Разрешение избранного, служебных полей и существующих объектов
    // выполняется до SpecDG. Неудачное чтение не кэшируется
    // (см. ResolveFavoriteLinks).
    // --------------------------------------------------------------------

    // --------------------------------------------------------------------
    // Сверяет выходную схему правила с набором свойств избранного: все имена
    // должны найтись. Отсутствующие имена копятся в error_name, признак
    // готовности правила снимается. Возвращает признак готовности.
    // --------------------------------------------------------------------
    bool MatchDestinationProperties (SpecRule &rule,
                                     const GS::HashTable<GS::UniString, GS::UniString> &favorite,
                                     ParamDict &error_name) {
        for (const auto &rawname : rule.out_paramrawname) {
            if (!favorite.ContainsKey (rawname)) {
                rule.runState.destinationReady = false;
                if (!error_name.ContainsKey (rawname))
                    error_name.Add (rawname, true);
            }
        }
        for (const auto &rawname : rule.out_sum_paramrawname) {
            if (!favorite.ContainsKey (rawname)) {
                rule.runState.destinationReady = false;
                if (!error_name.ContainsKey (rawname))
                    error_name.Add (rawname, true);
            }
        }
        return rule.runState.destinationReady;
    }

    // Ищет у избранного два служебных свойства и пишет их в правило:
    //   subguid_paramrawname (маркер из описания) -> destinationParamGuidName
    //       (свойство, найденное по маркеру и слову "sync_guid");
    //   описание со словом "spec_rule_name"      -> subguid_rulename.
    // Значения берутся из кэша свойств; при неудаче чтение НЕ кэшируется и
    // переход к следующему свойству выполняется без кэширования неудачи.
    // Возвращает признак, что носитель GUID найден.
    bool ResolveFavoriteLinks (SpecRule &rule,
                               const GS::HashTable<GS::UniString, GS::UniString> &favorite,
                               ParamDictValue &paramToWrite) {
        bool guidFound = false;
        // Поисковый маркер ОТДЕЛЕН от определения правила и изменяем по ходу
        // обхода: после успешного разрешения следующие свойства сопоставляются
        // уже по найденному имени, а не по исходному маркеру описания.
        // Прежде тот же эффект давало присваивание обратно в subguid_paramrawname;
        // теперь определение правила неизменяемо, а последовательность выбора
        // сохраняется (при двух подходящих свойствах выбирается первое).
        GS::UniString searchMarker = rule.subguid_paramrawname;
        for (const auto &cItt : favorite) {
#ifdef ServerMainVers_2800
            const GS::UniString rawname = cItt.key;
            const GS::UniString description = cItt.value;
#else
            const GS::UniString rawname = *cItt.key;
            const GS::UniString description = *cItt.value;
#endif
            // Ищем у избранного свойство для записи имя правила
            if (description.Contains ("spec_rule_name")) {
                if (!paramToWrite.ContainsKey (rawname)) {
                    ParamValue chpvalue;
                    if (!ParamHelpers::GetParamValueFromCache (rawname, chpvalue)) {
#if defined(TESTING)
                        DBprnt ("ERROR SpecArray - GetParamValueFromCache spec_rule_name", rawname);
#endif
                        continue;
                    }
                    paramToWrite.Add (rawname, chpvalue);
                }
                rule.subguid_rulename = rawname;
            }
            // Сопоставление идёт по поисковому маркеру: сначала это неизменяемый
            // маркер subguid_paramrawname из описания правила, а после
            // успешного разрешения - уже найденное имя свойства. Найденное имя
            // пишется в destinationParamGuidName, поэтому определение правила
            // не подменяется результатом и повторный проход по тому же правилу
            // ищет то же самое.
            if (!searchMarker.IsEmpty ()) {
                if (description.Contains (searchMarker.ToLowerCase ()) && description.Contains ("sync_guid")) {
                    if (!paramToWrite.ContainsKey (rawname)) {
                        ParamValue chpvalue;
                        if (!ParamHelpers::GetParamValueFromCache (rawname, chpvalue)) {
#if defined(TESTING)
                            DBprnt ("ERROR SpecArray - GetParamValueFromCache sync_guid", rawname);
#endif
                            continue;
                        }
                        paramToWrite.Add (rawname, chpvalue);
                    }
                    rule.runState.destinationParamGuidName = rawname;
                    searchMarker = rawname;
                    guidFound = true;
                }
            }
            if (guidFound && !rule.subguid_rulename.IsEmpty ())
                break;
        }
        return guidFound;
    }

    // Отбирает ранее созданные элементы правила. ПУСТОЙ selected_elements
    // означает «взять все найденные» — прежнее поведение, менять его нельзя:
    // это изменило бы объём удаляемых строк.
    void SelectExistingElements (SpecRule &rule, const GS::Array<API_Guid> &found, const UnicGuid &selected_elements) {
        if (selected_elements.IsEmpty ()) {
            rule.runState.exsist_elements = found;
            return;
        }
        // Не присваивание, а ДОПИСЫВАНИЕ: поле к
        // этому моменту не очищается. Присваивание изменило бы поведение в
        // случае непустого runState.exsist_elements на входе (сейчас безопасен только
        // тем, что словарь правил создаётся заново на каждый запуск).
        for (const API_Guid &exsist_element : found) {
            if (!selected_elements.ContainsKey (exsist_element))
                continue;
            rule.runState.exsist_elements.Push (exsist_element);
        }
    }

    // Запрашивает чтение выходных имён, сумм и носителя GUID у ранее созданных
    // элементов, чтобы их можно было сравнить с правилом. Имена идут в
    // НИЖНЕМ регистре (добавляет NameToRawName), поэтому сверять их с
    // out_paramrawname напрямую нельзя.
    void AddExistingReadRequests (const SpecRule &rule,
                                  const GS::Array<API_Guid> &elements,
                                  ParamDictElement &paramToRead) {
        ParamDictValue paramDict = {}; // Словарь параметров для чтения для одного элемента
        for (const GS::UniString &rawname : rule.out_sum_paramrawname) {
            if (!paramDict.ContainsKey (rawname))
                ParamHelpers::AddValueToParamDictValue (paramDict, rawname);
        }
        for (const GS::UniString &rawname : rule.out_paramrawname) {
            if (!paramDict.ContainsKey (rawname))
                ParamHelpers::AddValueToParamDictValue (paramDict, rawname);
        }
        ParamHelpers::AddValueToParamDictValue (paramDict, rule.runState.destinationParamGuidName);
        // Добавляем параметры для каждого элемента
        for (const API_Guid elemguid : elements) {
            ParamHelpers::AddParamDictValue2ParamDictElement (elemguid, paramDict, paramToRead);
        }
    }

    // --------------------------------------------------------------------
    // Получение значения параметра элемента
    // Назначение: читает значение свойства/GDL-параметра/материала/формулы для элемента
    // Параметры:
    //   elemguid - GUID элемента
    //   rawname - "сырое" имя параметра (с префиксами @property:, @material:, @formula:, @gdl:)
    //   context - набор прочитанных словарей (заполняется ElementsRead)
    //   pvalue - [OUT] полученное значение
    //   fromMaterial - флаг: читать из материалов слоев конструкции
    //   n_layer - номер слоя материала
    // Значения берутся только из context. Исключение — формулы:
    // EvalExpression обращается к PROPERTYCACHE (), поэтому под TESTING
    // чтение пополняет formulaCacheStats (calls, fullHits, fullClears).
    // Алгоритм:
    //   1. Если это libdata (@listdata:) - парсит формулу и вычисляет через ListData
    //   2. Если это формула (@formula:) - парсит и вычисляет через ReadFormula
    //   3. Если это материал (@material:) - читает из context.composite по n_layer
    //   4. Иначе - читает через GetParamValueForElements (обычное свойство/GDL)
    // Возвращает: true если значение успешно прочитано
    // Примечание: pvalue.isValid = true только при успешном чтении
    // --------------------------------------------------------------------
    bool SpecValueReader::Read (const API_Guid &elemguid,
                                const GS::UniString &rawname,
                                ParamValue &pvalue,
                                GS::Int32 n_layer) const {
        pvalue.isValid = false;
        if (hasLibData (rawname)) {
            if (n_layer < 0)
                return false;
            const ParamDictValue *p = context.read.GetPtr (elemguid);
            if (p == nullptr)
                return false;
            const ParamValue *formula = p->GetPtr (rawname);
            if (formula == nullptr)
                return false;
            ParamDictValue paramDict = {}; // Словарь параметров в формуле
            paramDict.Add (rawname, *formula);
            GS::UniString formula_expression = formula->val.uniStringValue;
            ParamHelpers::ParseParamName (formula_expression, paramDict);
            if (!ListData::AddLibdataToParamValueDict (
                    elemguid, n_layer, context.listData, formula_expression, paramDict)) {
                pvalue.val.type = API_PropertyStringValueType;
                pvalue.val.uniStringValue = EMPTYSTRING;
                pvalue.val.doubleValue = 0;
                pvalue.val.rawDoubleValue = 0;
                pvalue.val.intValue = 0;
                pvalue.val.boolValue = false;
                pvalue.val.canCalculate = false;
                pvalue.isValid = true;
                return true;
            }
            if (!ParamHelpers::ReadFormula (paramDict, true)) {
                pvalue.val.type = API_PropertyStringValueType;
                pvalue.val.uniStringValue = EMPTYSTRING;
                pvalue.val.doubleValue = 0;
                pvalue.val.rawDoubleValue = 0;
                pvalue.val.intValue = 0;
                pvalue.val.boolValue = false;
                pvalue.val.canCalculate = false;
                pvalue.isValid = true;
                return true;
            }
            const ParamValue *result = paramDict.GetPtr (rawname);
            if (result == nullptr)
                return false;
            pvalue = *result;
            return pvalue.isValid;
        }
        if (!ParamHelpers::GetParamValueForElements (elemguid, rawname, context.read, pvalue))
            return false;
        if (!pvalue.fromMaterial)
            return true;
        // Общее значение прочитано, но значение отдельного слоя ещё не получено.
        pvalue.isValid = false;
        if (n_layer < 0)
            return false;
        const ParamDictComposite *pc = context.composite.GetPtr (elemguid);
        if (pc == nullptr) {
#if defined(TESTING)
            DBprnt ("Spec err", "!context.composite.ContainsKey (elemguid)");
#endif
            return false;
        }
        const ParamComposite *pcelem = pc->GetPtr (rawname);
        if (pcelem == nullptr) {
#if defined(TESTING)
            DBprnt ("Spec err", "!context.composite.ContainsKey (rawname)");
#endif
            return false;
        }
        // Если параметр читался как материал из состава конструкции, берём значение из слоя.
        if (pcelem->composite.IsEmpty ()) {
#if defined(TESTING)
            DBprnt ("Spec err", "pcelem.composite.IsEmpty()");
#endif
            return false;
        }
        GS::Int32 max_layers = pcelem->composite.GetSize ();
        if (n_layer >= max_layers) {
            pvalue.val.type = API_PropertyStringValueType;
            pvalue.val.uniStringValue = EMPTYSTRING;
            pvalue.val.doubleValue = 0;
            pvalue.val.rawDoubleValue = 0;
            pvalue.val.intValue = 0;
            pvalue.val.boolValue = false;
            pvalue.val.canCalculate = false;
            pvalue.isValid = true;
            return true;
        }
        if (max_layers >= max_group_mat) {
            msg_rep ("Spec err",
                     GS::UniString::Printf ("Max layers over critical - %d", max_layers),
                     APIERR_GENERAL,
                     elemguid);
        }
        pvalue.val.type = API_PropertyStringValueType;
        double x = 0;
        const GS::UniString &val = pcelem->composite[n_layer].val;
        pvalue.val.canCalculate = UniStringToDouble (val, x);
        pvalue.val.uniStringValue = val;
        pvalue.val.doubleValue = x;
        pvalue.val.rawDoubleValue = x;
        pvalue.val.intValue = DoubleToInt32 (x, "Spec::GetParamValue", "intValue материала слоя " + val, elemguid);
        if (pvalue.val.canCalculate) {
            pvalue.val.boolValue = !is_equal (x, 0);
        } else {
            pvalue.val.boolValue = !pvalue.val.uniStringValue.IsEmpty ();
        }
        pvalue.isValid = true;
        return true;
    }

    bool OutSlotsMatchSchema (const Element &element, UInt32 outSlots, UInt32 sumSlots) {
        // Сверка по схеме слотов; условие
        // (ни один набор не пуст, числа совпадают).
        if (element.out_slots.IsEmpty ())
            return false;
        UInt32 nonSum = 0;
        UInt32 sum = 0;
        for (const OutputSlot &slot : element.out_slots) {
            if (slot.isSum)
                ++sum;
            else
                ++nonSum;
        }
        if (nonSum == 0 || sum == 0)
            return false;
        if (sum != sumSlots || nonSum != outSlots)
            return false;
        // Порядок обязателен, а не желателен: OutParamValue/OutSumValue и
        // SumContributionIntoRow адресуют слоты ПО ПОЗИЦИИ (первые
        // OutParamSlotCount () — выходные, остальные — суммы), пересчитывая
        // число выходных через флаг. Перемешанная схема прошла бы проверку
        // чисел, и тогда суммирование сложило бы ВЫХОДНОЙ слот вместо
        // суммарного — тихо и без диагностики. Производственный конструктор
        // BuildOutputSlots порядок соблюдает, но схема — публичные данные
        // элемента, и полагаться на единственного автора нельзя.
        for (UInt32 i = 1; i < element.out_slots.GetSize (); ++i) {
            if (!element.out_slots[i - 1].isSum && element.out_slots[i].isSum)
                continue;
            if (element.out_slots[i - 1].isSum && !element.out_slots[i].isSum)
                return false; // выходной слот ПОСЛЕ суммарного — порядок нарушен
        }
        return true;
    }

    // --------------------------------------------------------------------
    // Связывает поля групп с выходными слотами ОДИН раз до цикла по элементам.
    // Типы SlotBinding / GroupSlotBinding объявлены в Spec.hpp.
    // --------------------------------------------------------------------
    GS::Array<GroupSlotBinding> PrepareSlotBindings (const SpecRule &rule) {
        GS::Array<GroupSlotBinding> bindings = {};
        const UInt32 schemaOutSlots = rule.out_paramrawname.GetSize ();
        const UInt32 schemaSumSlots = rule.out_sum_paramrawname.GetSize ();
        for (const GroupSpec &group : rule.groups) {
            GroupSlotBinding binding = {};
            binding.schemaOutSlots = schemaOutSlots;
            binding.schemaSumSlots = schemaSumSlots;
            for (const GS::UniString &rawname : group.out_paramrawname) {
                SlotBinding slot = {};
                slot.rawname = &rawname;
                binding.outSlots.Push (slot);
            }
            for (const GS::UniString &rawname : group.sum_paramrawname) {
                SlotBinding slot = {};
                slot.rawname = &rawname;
                slot.isSumLiteral = rawname.IsEqual ("1");
                binding.sumSlots.Push (slot);
            }
            binding.sizesMatchSchema =
                binding.outSlots.GetSize () == schemaOutSlots && binding.sumSlots.GetSize () == schemaSumSlots;
            bindings.Push (binding);
        }
        return bindings;
    }

    // --------------------------------------------------------------------
    // Видимость источника на момент расчёта.
    // Проверка видимости выполняется в цикле по элементам: ACAPI_Element_Filter
    // вызывается с тремя флагами, по одному разу на элемент, до чтения любых
    // значений.
    // --------------------------------------------------------------------
    bool IsSourceVisible (const API_Guid &elemguid, bool onlyVisible) {
        if (!onlyVisible)
            return true;
        return ACAPI_Element_Filter (elemguid,
                                     APIFilt_OnVisLayer | APIFilt_IsVisibleByRenovation | APIFilt_IsInStructureDisplay);
    }

    // --------------------------------------------------------------------
    // Расчётная часть правила заполняет out_param, elements и счётчики до
    // сверки существующих строк. Здесь нет создания или удаления в модели.
    // --------------------------------------------------------------------
    Int32 PlanRuleRows (SpecRule &rule,
                        const SpecReadContext &context,
                        ElementDict &elements,
                        UnicGuid &error_element,
                        bool showUserInterface,
                        GS::HashTable<GS::UniString, GS::UniString> &out_param,
                        SpecChangePlan *plan) {
        ParamDict not_found_paramname = {};
        ParamDict not_found_unic = {};
        Int32 n_elements = 0;
        FormatString fstr = FormatStringFunc::ParseFormatString (".2m");
        // out_param передан вызывающим: ключ - уникальные значения,
        // значение - выходящие параметры. Словарь передаётся вызывающим кодом.
        // Число выходных слотов берётся из схемы правила ОДИН раз: оно не меняется
        // во время исполнения, а сверять его приходится для каждого элемента.
        const UInt32 out_slots = rule.out_paramrawname.GetSize ();
        const UInt32 sum_slots = rule.out_sum_paramrawname.GetSize ();
        // Привязка слотов группы к полям готовится ОДИН раз до цикла по элементам
        // Далее используются готовые указатели на имена, поэтому
        // ни имя поля, ни признак константной суммы не вычисляются заново для
        // каждого источника. Размер равен rule.groups.GetSize (), группы идут в
        // том же порядке, поэтому индексы сопоставимы.
        const GS::Array<GroupSlotBinding> slot_bindings = PrepareSlotBindings (rule);
        // Один reader на всё правило. Значения он только читает, а
        // число чтений задаёт вычислитель, поэтому изменить прочитанные данные
        // «по ходу» нельзя.
        const SpecValueReader reader (context);
        for (const API_Guid &elemguid : rule.runState.elements) {
            // Видимость проверяется до чтения любых значений, как и начала расчёта.
            if (!IsSourceVisible (elemguid, rule.only_visible))
                continue;
            for (UInt32 group_index = 0; group_index < rule.groups.GetSize (); group_index++) {
                const GroupSpec &group = rule.groups[group_index];
                const GroupSlotBinding &binding = slot_bindings[group_index];
                if (!group.is_Valid)
                    continue;
                // Вклад одного источника: фаза 1 — флаг, уникальные параметры, ключ и
                // суммируемые слоты; фаза 2 (выходные слоты) вызывается ниже,
                // только для первого представителя ключа. Число и порядок
                // чтений: флаг -> уникальные -> суммы -> выход.
                const RuleContribution contribution = BuildContribution (
                    elemguid, group_index, group, binding, reader, not_found_paramname, not_found_unic);
                GS::UniString key = contribution.key;
                // полнота чтения считается по САМОМУ вкладу, а не по
                // словарям not_found_*. Те хранят только поля, по которым выведено сообщение
                // (политика повторов, зависит от stop_on_error), поэтому при
                // stop_on_error = false показали бы «полное чтение» там, где
                // поля действительно не прочитаны. Ни одно условие ниже этого
                // не читает — счётчики только наблюдают.
                // contributionsPartial — ЧИСЛО ВКЛАДОВ, а не число причин.
                // Один и тот же вклад может быть одновременно неполным по
                // чтению и отброшен сверкой схемы; засчитывать его дважды
                // завышало бы счётчик и делало бы сравнение с contributionsTotal
                // бессмысленным. Поэтому решение принимается здесь, а на
                // SchemaMismatch инкремент не делается вовсе.
                bool partialThisContribution = false;
                // ВСЕ вклады, включая отклонённые: непрочитанное уникальное поле
                // делает чтение неполным; это отражается в плане. Здесь
                // учитывается даже для исключённых вкладов.
                if (plan) {
                    plan->contributionsTotal += 1;
                    plan->notFoundUnicCount += contribution.missingUnic.GetSize ();
                    plan->notFoundParamCount += contribution.missingSum.GetSize ();
                    // Неполное чтение видно по трём признакам вклада: непрочитанные
                    // поля либо пустой набор выходов/сумм. Флаг isComplete
                    // выставляется только в фазе 2, поэтому здесь (конец фазы 1)
                    // он ещё не задан — неполноту фазы 1 считаем по missingSum
                    // и hasSumSlots, а полноту выходов — после фазы 2, где
                    // missingOut уже заполнен.
                    // Неполнота РАСЧЁТА засчитывается только для вкладов, дошедших
                    // до фазы сумм. Вклад без уникального ключа возвращается
                    // до чтения сумм, и его hasSumSlots не выставлен — считать
                    // его «неполным расчётом» значило бы задваивать одну и ту же
                    // неполноту, уже учтённую в notFoundUnicCount. Здесь речь о
                    // схеме и суммах, там — о чтении: признаки разные.
                    if (contribution.status != ContributionStatus::Excluded &&
                        (!contribution.missingSum.IsEmpty () || !contribution.hasSumSlots)) {
                        partialThisContribution = true;
                        // Здесь и только здесь contributionsPartial растёт.
                        // Ветка SchemaMismatch ниже инкрементит лишь тогда, когда
                        // вклад ещё не помечен, чтобы один вклад не считался
                        // дважды.
                        plan->contributionsPartial += 1;
                    }
                }
                // Политика отказов задаётся здесь: вклад лишь
                // сообщает, ЧТО не прочитано, а решение (писать ли отчёт, вести
                // ли счётчик, останавливать ли правило) — здесь.
                if (contribution.status == ContributionStatus::Excluded) {
                    if (!contribution.missingUnic.IsEmpty ()) {
                        for (const Spec::MissingField &field : contribution.missingUnic) {
                            if (field.isError && rule.stop_on_error && !not_found_unic.ContainsKey (field.rawname)) {
                                if (!error_element.ContainsKey (elemguid))
                                    error_element.Add (elemguid, true);
                                msg_rep (
                                    "Spec", "Unic parameter not valid: " + field.rawname, APIERR_GENERAL, elemguid);
                                not_found_unic.Add (field.rawname, true);
                            }
                        }
                    }
                    continue;
                }
                // Отчёт по непрочитанным полям суммы формируется здесь при stop_on_error.
                for (const Spec::MissingField &field : contribution.missingSum) {
                    if (field.isError && rule.stop_on_error &&
                        !not_found_paramname.ContainsKey ("sum:" + field.rawname)) {
                        if (!error_element.ContainsKey (elemguid))
                            error_element.Add (elemguid, true);
                        msg_rep ("Spec", "Sum parameter not valid: " + field.rawname, APIERR_GENERAL, elemguid);
                        not_found_paramname.Add ("sum:" + field.rawname, false);
                    }
                }

                // выходные слоты читаются ТОЛЬКО для первого
                // представителя ключа. Решение «первый ли это ключ» принимает
                // раскладка ниже, поэтому фаза 2 вызывается лишь когда ключа
                // ещё нет в словаре строк.
                RuleContribution rowContribution = contribution;
                if (!elements.ContainsKey (key))
                    ReadContributionOutputs (elemguid, group, binding, reader, fstr, rowContribution);
                // Отчёт по непрочитанным полям выхода: только для первого
                // представителя ключа.
                if (plan)
                    plan->notFoundParamCount += rowContribution.missingOut.GetSize ();
                for (const Spec::MissingField &field : rowContribution.missingOut) {
                    if (!not_found_paramname.ContainsKey (field.rawname) && rule.stop_on_error && field.isError) {
                        if (!error_element.ContainsKey (elemguid))
                            error_element.Add (elemguid, true);
                        not_found_paramname.Add ("out:" + field.rawname, false);
                    }
                }

                // Раскладка вклада в строку суммирует только общие допустимые слоты.
                const RowAddition addition =
                    AddContributionToRow (elements, rowContribution, rule, out_slots, sum_slots, out_param);
                if (addition == RowAddition::Created) {
                    n_elements += 1;
                } else if (addition == RowAddition::SchemaMismatch) {
                    // contributionsPartial здесь НЕ инкрементится: вклад с
                    // несовпадением схемы уже помечен partialThisContribution,
                    // потому что несовпадение и есть следствие неполного чтения.
                    // schemaMismatchCount отдельно учитывает несовпадение со схемой.
                    if (plan) {
                        plan->schemaMismatchCount += 1;
                        if (!partialThisContribution)
                            plan->contributionsPartial += 1;
                    }
                    if (rule.stop_on_error) {
                        if (!error_element.ContainsKey (elemguid))
                            error_element.Add (elemguid, true);
                        n_elements = 0;
                    }
                }
            }
        }
        if (rule.stop_on_error) {
            const Int32 iseng = ID_ADDON_STRINGS + isEng ();
            if (!not_found_paramname.IsEmpty ()) {
                if (showUserInterface) {
                    GS::UniString SpecNotFoundParametersString =
                        RSGetIndString (iseng, SpecNotFoundParametersId, ACAPI_GetOwnResModule ());
                    ACAPI_WriteReport (SpecNotFoundParametersString, true);
                }
                GS::UniString out = "Not found param:";
                for (auto &cIt : not_found_paramname) {
#ifdef ServerMainVers_2800
                    GS::UniString s = cIt.key;
#else
                    GS::UniString s = *cIt.key;
#endif
                    out.Append (s);
                    out.Append ("; ");
                }
                out.ReplaceAll (PVALPREFIX, EMPTYSTRING);
                out.ReplaceAll (BRACEEND, EMPTYSTRING);
                out.ReplaceAll (":", " : ");
                out.ReplaceAll (STRINGPROC, EMPTYSTRING);
                out.ReplaceAll ("nosyncname", EMPTYSTRING);
                msg_rep ("Spec", out, NoError, APINULLGuid);
            }
            if (!not_found_unic.IsEmpty ()) {
                if (showUserInterface) {
                    GS::UniString SpecNotFoundParametersString =
                        RSGetIndString (iseng, SpecNotFoundParametersId, ACAPI_GetOwnResModule ());
                    ACAPI_WriteReport (SpecNotFoundParametersString, true);
                }
                GS::UniString out = "Not found unic:";
                for (auto &cIt : not_found_unic) {
#ifdef ServerMainVers_2800
                    GS::UniString s = cIt.key;
#else
                    GS::UniString s = *cIt.key;
#endif
                    out.Append (s);
                    out.Append ("; ");
                }
                out.ReplaceAll (PVALPREFIX, EMPTYSTRING);
                out.ReplaceAll (BRACEEND, EMPTYSTRING);
                out.ReplaceAll (":", " : ");
                out.ReplaceAll (STRINGPROC, EMPTYSTRING);
                out.ReplaceAll ("nosyncname", EMPTYSTRING);
                msg_rep ("Spec", out, NoError, APINULLGuid);
            }
            if (!not_found_paramname.IsEmpty () || !not_found_unic.IsEmpty ()) {
                n_elements = 0;
                elements.Clear ();
                return 0;
            }
        }
        // Возвращаем число рассчитанных строк
        // независимо от необходимости сверки существующих объектов.
        return n_elements;
    }

    // --------------------------------------------------------------------
    // Формирование элементов для создания/модификации на основе правила
    // Назначение: обрабатывает элементы согласно правилу и группирует их
    // Параметры:
    //   rule - правило спецификации (содержит группы, параметры для чтения/записи)
    //   context - набор прочитанных словарей
    //   elements - [OUT] словарь создаваемых элементов (ключ - уникальная комбинация)
    //   elements_mod - [OUT] словарь модифицируемых элементов (для delete_old)
    //   elements_delete - [OUT] массив удаляемых устаревших элементов
    //   error_element - [OUT] элементы с ошибками чтения параметров
    // Алгоритм:
    //   1. Для каждого элемента в rule.runState.elements:
    //      - проверяет видимость (если rule.only_visible)
    //      - для каждой группы (group) проверяет флаг (flag_paramrawname)
    //      - формирует ключ из уникальных параметров (unic_paramrawname)
    //      - читает параметры для записи (out_paramrawname) и суммы (sum_paramrawname)
    //      - если ключ уже есть в elements - суммирует количества
    //      - иначе создаёт новый элемент
    //   2. Если rule.delete_old - обрабатывает существующие элементы (exsist_elements)
    // Возвращает: количество элементов для создания/модификации
    // Примечание: при stop_on_error = true и ошибке чтения возвращает 0
    // --------------------------------------------------------------------
    Int32 GetElementsForRule (SpecRule &rule,
                              const SpecReadContext &context,
                              ElementDict &elements,
                              ElementDict &elements_mod,
                              GS::Array<API_Guid> &elements_delete,
                              UnicGuid &error_element,
                              bool showUserInterface,
                              SpecChangePlan *plan) {
        Int32 n_elements = 0;
        // out_param — словарь рассчитанных строк: он
        // строится в расчётной части и читается в сверке существующих строк.

        GS::HashTable<GS::UniString, GS::UniString> out_param = {};
        // Для сверки существующих строк используется тот же контекст чтения
        // и формат ".2m"; расчёт и сверка не изменяют контекст.
        const SpecValueReader reader (context);
        FormatString fstr = FormatStringFunc::ParseFormatString (".2m");
        // план получает полноту чтения/расчёта из расчётной части.
        // Передаётся и когда сверка не пойдёт (delete_old = false): неполное
        // чтение делает расчёт недостоверным независимо от того, удаляются ли
        // существующие строки. Здесь фиксируется полнота без решения о применении.
        n_elements = PlanRuleRows (rule, context, elements, error_element, showUserInterface, out_param, plan);
        if (!rule.delete_old)
            return n_elements;
        // План наблюдает решение сверки, но не становится вторым источником
        // истины — тот же вызов пишет и фактические списки. На рабочем пути
        // plan == nullptr, и тогда план НЕ создаётся: иначе каждое изменение
        // оплачивало бы второй копией строки (вставкой в plan->update) плюс
        // сбором removals/create и обходом остатков, то есть ровно тем
        // дублированием payload, которое запрещено.
        if (plan != nullptr)
            plan->deleteOld = 1;
        // elements_delete — общий накопительный массив запуска: он передаётся
        // каждому правилу и содержит удаления ПРЕДЫДУЩИХ правил тоже. Сверка
        // сопоставляет с планом только свой суффикс, иначе второе правило,
        // ничего не удалившее, дало бы ложное расхождение.
        const UIndex deleteOffset = elements_delete.GetSize ();
        ReconcileExistingRows (rule, reader, fstr, out_param, elements, elements_mod, elements_delete, plan);
        if (plan != nullptr && !plan->Matches (elements_mod, elements_delete, deleteOffset)) {
            // Страховка: план и фактические списки пишутся из одних точек, так
            // что расхождение указывает на нарушение согласованности списков.
            msg_rep ("Spec", "SpecChangePlan mismatch", NoError, APINULLGuid);
        }
        n_elements = 0;
        n_elements += elements_delete.GetSize ();
        n_elements += elements_mod.GetSize ();
        n_elements += elements.GetSize ();
        return n_elements;
    }

    // --------------------------------------------------------------------
    // Парсинг описания правила спецификации
    // Назначение: разбирает строку описания свойства и создаёт структуру SpecRule
    // Параметры:
    //   description - описание свойства (содержит правило в формате Spec_rule{...})
    // Алгоритм:
    //   1. Определяет тип правила (v2, v3, km, kzh) по ключевым словам в описании
    //   2. Извлекает критерий (имя избранного элемента) - текст до первой точки с запятой
    //   3. Разбивает оставшуюся часть на группы g() и итог s()
    //   4. Парсит каждую группу g():
    //      - уникальные параметры (U1,U2,U3) - part 0
    //      - параметры для чтения (P1,P2,P3) - part 1
    //      - флаг (F) - part 2
    //      - параметры для суммирования (Q1,Q2) - part 3
    //   5. Парсит группу s():
    //      - параметры для записи (Pn1,Pn2,Pn3) - part 0
    //      - параметры для записи количества (Qn1,Qn2) - part 1
    // Возвращает: структуру SpecRule (заполненную на основе описания)
    // Формат: Spec_rule{КРИТЕРИЙ ;g(U1,U2,U3; P1,P2,P3; F; Q1,Q2) s(Pn1,Pn2,Pn3; Qn1,Qn2)}
    // --------------------------------------------------------------------
    GS::UniString ParseErrorText (ParseError error) {
        switch (error) {
        case ParseError::NoGroupMarker:
            return "no group marker g@@ in description";
        case ParseError::GroupNotSplit:
            return "group body has no semicolon, cannot split into parts";
        case ParseError::OutputPartCount:
            return "output schema must have exactly two parts";
        case ParseError::NoSummary:
            return "no summary part s@@ or fewer than two parts";
        case ParseError::NoGroupsAccepted:
            return "no group was accepted by the output schema";
        case ParseError::EmptyOutputSchema:
            return "output schema is empty";
        case ParseError::EmptySumSchema:
            return "sum schema is empty";
        case ParseError::None:
        default:
            return "no parse error";
        }
    }

    // --------------------------------------------------------------------
    // Определяет ПОЛИТИКУ правила по имени префикса описания.
    // Имя проверяется в нижнем регистре и по МЕСТУ "pec_rule", а не по префиксу
    // целиком: сравнение с "pec_rule_km" описывает вхождение в любой части строки,
    // поэтому правило вида "Spec_rule_my_km_data" тоже получит политику KM.
    // Порядок ветвления значим: v2 проверяется перед v3, а v3 перед KM/KZH, и
    // первый совпавший вариант выигрывает. Политика KM/KZH затем перекрывает
    // значения v2/v3 (delete_old=false, stop_on_error=false, only_visible=true).
    // Всё остальное - разбор групп и полей - делает GetRuleFromDescription.
    // --------------------------------------------------------------------
    void ApplyRulePolicy (const GS::UniString &description, SpecRule &rule) {
        const GS::UniString ldescription = description.ToLowerCase ();
        if (ldescription.Contains ("pec_rule_v2")) {
            rule.delete_old = true;
        } else {
            if (ldescription.Contains ("pec_rule_v3")) {
                rule.delete_old = true;
                rule.stop_on_error = false;
                rule.only_visible = false;
            } else {
                if (ldescription.Contains ("pec_rule_km")) {
                    rule.isKM = true;
                } else {
                    if (ldescription.Contains ("pec_rule_kzh")) {
                        rule.isKZH = true;
                    }
                }
            }
        }
        if (rule.isKM) {
            rule.delete_old = false;
            rule.stop_on_error = false;
            rule.only_visible = true;
        }
        if (rule.isKZH) {
            rule.delete_old = false;
            rule.stop_on_error = false;
            rule.only_visible = true;
        }
    }

    // --------------------------------------------------------------------
    // Раскрытие группы в итоговые группы правила:
    //   min_row > 0 - параметры-массивы "[N]" разворачиваются в min_row отдельных
    //                 групп, каждой строке массива достаётся свой @arr_индекс;
    //   иначе      - группа сверяется с выходной схемой и размножается по слоям:
    //                 материалы - max_group_mat групп, listdata - max_group_lib,
    //                 обычная группа - одна.
    // Параметры:
    //   group   - [IN/OUT] разобранная группа; заполняется n_layer/is_Valid
    //   min_row - число строк массива, 0 если массивов нет
    //   rule    - [IN/OUT] правило; готовые группы добавляются в rule.groups
    // --------------------------------------------------------------------
    void ExpandGroup (GroupSpec &group, Int32 min_row, SpecRule &rule) {
        if (min_row > 0) {
            // создаём группы для параметров с массивами
            for (Int32 jj = 1; jj <= min_row; jj++) {
                GroupSpec group_add = {};
                for (GS::UniString rawName : group.out_paramrawname) {
                    if (rawName.Contains ("[") && rawName.Contains ("]")) {
                        GS::UniString n_row_txt = rawName.GetSubstring ('[', ']', 0);
                        rawName.ReplaceFirst ("[" + n_row_txt + "]", EMPTYSTRING);
                        rawName.ReplaceAll (BRACEEND,
                                            GS::UniString::Printf ("@arr_%d_%d_%d_%d_%d", jj, jj, 1, 1, ARRAY_UNIC) +
                                                BRACEEND);
                    }
                    group_add.out_paramrawname.Push (rawName);
                }
                for (GS::UniString rawName : group.unic_paramrawname) {
                    if (rawName.Contains ("[") && rawName.Contains ("]")) {
                        GS::UniString n_row_txt = rawName.GetSubstring ('[', ']', 0);
                        rawName.ReplaceFirst ("[" + n_row_txt + "]", EMPTYSTRING);
                        rawName.ReplaceAll (BRACEEND,
                                            GS::UniString::Printf ("@arr_%d_%d_%d_%d_%d", jj, jj, 1, 1, ARRAY_UNIC) +
                                                BRACEEND);
                    }
                    group_add.unic_paramrawname.Push (rawName);
                }
                GS::UniString rawName = group.flag_paramrawname;
                if (rawName.Contains ("[") && rawName.Contains ("]")) {
                    GS::UniString n_row_txt = rawName.GetSubstring ('[', ']', 0);
                    rawName.ReplaceFirst ("[" + n_row_txt + "]", EMPTYSTRING);
                    rawName.ReplaceAll (
                        BRACEEND, GS::UniString::Printf ("@arr_%d_%d_%d_%d_%d", jj, jj, 1, 1, ARRAY_UNIC) + BRACEEND);
                }
                group_add.flag_paramrawname = rawName;
                for (GS::UniString rawName : group.sum_paramrawname) {
                    if (rawName.Contains ("[") && rawName.Contains ("]")) {
                        GS::UniString n_row_txt = rawName.GetSubstring ('[', ']', 0);
                        rawName.ReplaceFirst ("[" + n_row_txt + "]", EMPTYSTRING);
                        rawName.ReplaceAll (BRACEEND,
                                            GS::UniString::Printf ("@arr_%d_%d_%d_%d_%d", jj, jj, 1, 1, ARRAY_UNIC) +
                                                BRACEEND);
                    }
                    group_add.sum_paramrawname.Push (rawName);
                }
                rule.groups.Push (group_add);
            }
        } else {
            if (group.out_paramrawname.GetSize () != rule.out_paramrawname.GetSize ()) {
                group.is_Valid = false;
                msg_rep ("Spec",
                         "group.out_paramrawname.GetSize () != rule.out_paramrawname.GetSize ()",
                         APIERR_BADINDEX,
                         APINULLGuid);
            }
            if (group.sum_paramrawname.GetSize () != rule.out_sum_paramrawname.GetSize ()) {
                group.is_Valid = false;
                msg_rep ("Spec",
                         "group.sum_paramrawname.GetSize () != rule.out_sum_paramrawname.GetSize ()",
                         APIERR_BADINDEX,
                         APINULLGuid);
            }
            if (group.is_Valid) {
                if (group.fromMaterial) {
                    // Создаём необходимое количество групп
                    for (UInt32 n_layer = 0; n_layer < max_group_mat; n_layer++) {
                        group.n_layer = n_layer;
                        rule.groups.PushNew (group);
                    }
                } else {
                    if (group.fromLibData) {
                        // Создаём необходимое количество групп
                        for (UInt32 n_layer = 0; n_layer < max_group_lib; n_layer++) {
                            group.n_layer = n_layer;
                            rule.groups.PushNew (group);
                        }
                    } else {
                        rule.groups.Push (group);
                    }
                }
            }
        }
    }

    // --------------------------------------------------------------------
    // Разбор группы g(): уникальные параметры, параметры для чтения, флаг,
    // параметры количеств + раскрытие параметров-массивов в отдельные группы
    // Параметры:
    //   readPart - часть описания ДО "s@@": имя избранного и группы g()
    //   scratch  - [IN/OUT] буфер StringSplt, состояние переносится между вызовами
    //   rule     - [IN/OUT] правило; группы добавляются в rule.groups
    // Возвращает: false, если групп не оказалось ни одной или разбор невозможен;
    //             в этом случае parseValid уже сброшен внутри
    // --------------------------------------------------------------------
    bool ParseGroups (const GS::UniString &readPart, GS::Array<GS::UniString> &scratch, SpecRule &rule) {
        // Разбивка на группы
        GS::Array<GS::UniString> rulestring_group = {}; // Массив с строками групп
        UInt32 nrule_group = StringSplt (readPart, "g@@", rulestring_group, false, &scratch);
        if (nrule_group < 1) {
            rule.parseValid = false;
            rule.parseError = ParseError::NoGroupMarker;
            return false;
        }

        for (GS::UniString &rulestring_one_group : rulestring_group) {
            GS::Array<GS::UniString> rulestring_read = {}; // Массив групп параметров (уникальные, для чтения, флаг, для
                                                           // суммы)
            GroupSpec group = {};
            if (rulestring_one_group.IsEmpty ())
                continue;
            // Проверка типа группы
            if (rulestring_one_group.Contains (ATSIGN)) {
                // Чтение материалов
                if (rulestring_one_group.Contains ("Material_all@")) {
                    rulestring_one_group.ReplaceAll ("Material_all@", EMPTYSTRING);
                    group.fromMaterial = true;
                } else {
                    // Чтение данных ведомостей
                    if (rulestring_one_group.Contains ("libdata@")) {
                        rulestring_one_group.ReplaceAll ("libdata@", EMPTYSTRING);
                        group.fromLibData = true;
                    }
                }
            }
            if (hasLibData (rulestring_one_group))
                group.fromLibData = true;
            // Разбивка группы на параметры
            UInt32 nrule_read = StringSplt (rulestring_one_group, SEMICOLON, rulestring_read, false, &scratch);
            if (nrule_read <= 1) {
                rule.parseValid = false;
                rule.parseError = ParseError::GroupNotSplit;
                return false;
            }
            // g(U1,U2,U3; P1,P2,P3; F1; Q1,Q2) =>
            // U1,U2,U3 - уникальные параметры, part = 0
            // P1,P2,P3 - параметры для чтения, part == 1
            // F1 - флаг, part == 2
            // Q1,Q2 - количество, part == 3
            Int32 min_row = 0;            // Количество строк, указанных для параметра-массива
            bool isUnicSameAsOut = false; // Совпадают ли уникальные параметры с параметрами для записи
            for (UInt32 part = 0; part < nrule_read; part++) {
                GS::Array<GS::UniString> rulestring_param = {}; // Массив параметров
                UInt32 nrule_param = StringSplt (rulestring_read[part], COMMA, rulestring_param, false, &scratch);
                if (nrule_param < 1)
                    continue;
                if (part == 0 && nrule_param == 1) {
                    GS::UniString &name = rulestring_param[0];
                    if (name.IsEmpty ()) {
                        isUnicSameAsOut = true;
                        continue;
                    }
                    if (name == "-") {
                        isUnicSameAsOut = true;
                        continue;
                    }
                    if (name == "\"-\"") {
                        isUnicSameAsOut = true;
                        continue;
                    }
                    if (name == "\"\"") {
                        isUnicSameAsOut = true;
                        continue;
                    }
                    if (name == " ") {
                        isUnicSameAsOut = true;
                        continue;
                    }
                }
                for (GS::UniString &name : rulestring_param) {
                    name.Trim ('@');
                    if (!group.fromMaterial && !group.fromLibData)
                        name.Trim (CHARPROC);
                    name.Trim (CHARBRACESTART);
                    name.Trim (CHARBRACEEND);
                    name.Trim ();
                    if (part == 1 && name.IsEqual ("-")) {
                        name = EMPTYSTRING;
                        continue;
                    }
                    if (name.IsEmpty ())
                        continue;
                    if (name.Contains ("[") && name.Contains ("]")) {
                        GS::UniString n_row_txt = name.GetSubstring ('[', ']', 0);
                        double doubleValue = 0;
                        Int32 n_row = 10;
                        if (UniStringToDouble (n_row_txt, doubleValue) && doubleValue >= 1 &&
                            doubleValue <= max_group_mat) {
                            n_row = DoubleToInt32 (doubleValue, "Spec", "номер строки " + n_row_txt);
                        }
                        if (min_row == 0)
                            min_row = n_row;
                        min_row = n_row < min_row ? n_row : min_row;
                    }
                    GS::UniString rawName;
                    if (name.Contains (STRINGPROC)) {
                        if (group.fromMaterial) {
                            rawName = MATERIALNAMEPREFIX + "layers_auto,all;" + name + BRACEEND;
                        } else {
                            GS::UniString name_old = name;
                            name.ReplaceAll ("%elem.", "%@listdata:elem.");
                            name.ReplaceAll ("%prokat.", "%@listdata:prokat.");
                            name.ReplaceAll ("%mat.", "%@listdata:mat.");
                            name.ReplaceAll ("%arm.", "%@listdata:arm.");
                            name.ReplaceAll ("%subpos.", "%@listdata:subpos.");
                            if (!name.Contains ("<"))
                                name = name + STRFORMULASTART + STRFORMULAEND;
                            ParamHelpers::ReplaceProcToBrace (name, false);
                            if (name.Contains (STRINGPROC)) {
                                name_old.ReplaceAll (STRINGPROC, SPACESTRING);
                                msg_rep (
                                    "Spec",
                                    "Check the spelling of the parameter names - one of the names is missing the proc "
                                    "sign " +
                                        name_old,
                                    NoError,
                                    APINULLGuid);
                            }
                            rawName = FORMULANAMEPREFIX + name;
                        }
                    } else {
                        FormatString formatstring;
                        rawName = ParamHelpers::NameToRawName (name, formatstring);
                    }
                    switch (part) {
                    case 0:
                        group.unic_paramrawname.Push (rawName);
                        break;
                    case 1:
                        group.out_paramrawname.Push (rawName);
                        break;
                    case 2:
                        group.flag_paramrawname = rawName;
                        break;
                    case 3:
                        group.sum_paramrawname.Push (rawName);
                        break;
                    default:
                        break;
                    }
                }
            }
            if (isUnicSameAsOut) {
                group.unic_paramrawname = group.out_paramrawname;
            }
            // Добавим значения для суммы
            if (group.sum_paramrawname.GetSize () < rule.out_sum_paramrawname.GetSize ()) {
                for (UInt32 i = group.sum_paramrawname.GetSize (); i < rule.out_sum_paramrawname.GetSize (); i++) {
                    group.sum_paramrawname.Push ("1");
                    msg_rep ("Spec", "Check if the number of quantity parameters matches", NoError, APINULLGuid);
                }
            }
            // ExpandGroup раскрывает поля группы в схему правила.
            ExpandGroup (group, min_row, rule);
        }
        return true;
    }

    // --------------------------------------------------------------------
    // Разбирает ВЫХОДНУЮ СХЕМУ правила - часть описания после "s@@".
    // Формат: s (Pn1, Pn2, Pn3; Qn1, Qn2), где
    //   часть 0 (до точки с запятой) - свойства элемента-результата;
    //   часть 1 (после)              - свойства для записи количеств.
    // Требование РОВНО двух частей: одна или три части - невалидное описание, и
    // парсер обязан отвергнуть всё правило (parseValid = false), а не молча
    // взять первые две.
    // Внутри части имена чистятся от служебных символов ("proc", "@", фигурные
    // скобки, пробелы), после чего суффикс "[N]" (номер строки массива) срезается
    // и отбрасывается: в выходной схеме он не несёт смысла, разворачивание массива
    // делает группа. Пустые имена пропускаются, поэтому ",," не даёт пустых слотов.
    // Возвращает false, если частей не две; тогда и только тогда сбрасывается
    // rule.parseValid, а out_* остаются пустыми. При успехе флаг НЕ трогается:
    // дальше парсер продолжает разбор групп и вправе сбросить его сам.
    // --------------------------------------------------------------------
    bool ParseOutputSchema (const GS::UniString &writePart, GS::Array<GS::UniString> &scratch, SpecRule &rule) {
        GS::Array<GS::UniString> rulestring_write = {}; // Свойства для записи из группы s()
        const UInt32 nrule_write = StringSplt (writePart, SEMICOLON, rulestring_write, true, &scratch);
        if (nrule_write != 2) {
            rule.parseValid = false;
            rule.parseError = ParseError::OutputPartCount;
            return false;
        }
        for (UInt32 part = 0; part < nrule_write; part++) {
            GS::Array<GS::UniString> rulestring_param = {};
            const UInt32 nrule_param = StringSplt (rulestring_write[part], COMMA, rulestring_param, true, &scratch);
            if (nrule_param > 0) {
                for (UInt32 i = 0; i < nrule_param; i++) {
                    FormatString formatstring;
                    GS::UniString name = rulestring_param[i];
                    name.Trim (CHARPROC);
                    name.Trim ('@');
                    name.Trim (CHARBRACESTART);
                    name.Trim (CHARBRACEEND);
                    name.Trim ();
                    if (!name.IsEmpty ()) {
                        if (name.Contains ("[") && name.Contains ("]")) {
                            GS::UniString n_row_txt = name.GetSubstring ('[', ']', 0);
                            name.ReplaceFirst ("[" + n_row_txt + "]", EMPTYSTRING);
                        }
                        FormatString formatstring;
                        GS::UniString rawName = ParamHelpers::NameToRawName (name, formatstring);
                        if (part == 0)
                            rule.out_paramrawname.Push (rawName);
                        if (part == 1)
                            rule.out_sum_paramrawname.Push (rawName);
                    }
                }
            }
        }
        return true;
    }

    // --------------------------------------------------------------------
    // Разбирает строку описания правила и превращает её в структуру SpecRule.
    // Распознаёт критерий, группы и поля записи.
    // --------------------------------------------------------------------
    SpecRule GetRuleFromDescription (const GS::UniString &normalizedDescription) {
        // Рабочая копия: парсер правит её на месте (обрезает по скобкам, снимает
        // закрывающую скобку), но вызывающая строка остаётся нетронутой.
        GS::UniString description = normalizedDescription;
        // Сначала извлекается критерий — имя избранного элемента или другой ключевой текст.
        SpecRule rule = {};
        GS::Array<GS::UniString> partstring = {};
        GS::Array<GS::UniString> local_scratch;
        ApplyRulePolicy (description, rule);
        if (StringSplt (description, BRACEEND, partstring, "pec_rule") > 0) {
            description = partstring[0] + BRACEEND;
        }
        GS::UniString criteria = description.GetSubstring (CHARBRACESTART, CHARBSEMICOLON, 0);
        description.ReplaceAll (BRACESTART + criteria + SEMICOLON, BRACESTART);
        description = description.GetSubstring (CHARBRACESTART, CHARBRACEEND, 0);
        description.Trim (')');
        description.Trim ();
        if (criteria.Contains ("\""))
            criteria = criteria.GetSubstring (CHARDQUT, CHARDQUT, 0);
        criteria.Trim ();
        rule.favorite_name = criteria;
        GS::Array<GS::UniString> paramss = {};
        // Разбивка на группы и итог
        GS::Array<GS::UniString> rulestring_summ = {}; // Массив из имени избранного, групп g() и s()
        if (StringSplt (description, "s@@", rulestring_summ, true, &local_scratch) < 2) {
            rule.parseValid = false;
            rule.parseError = ParseError::NoSummary;
            return rule;
        }
        if (!ParseOutputSchema (rulestring_summ[1], local_scratch, rule))
            return rule;
        // ParseGroups разбирает группы g() правила.
        if (!ParseGroups (rulestring_summ[0], local_scratch, rule))
            return rule;
        // Финальные проверки идут каскадом и не прерываются: пустое описание
        // нарушает все три условия сразу. Поэтому пишется ПОСЛЕДНЯЯ сработавшая
        // причина, а не первая. Порядок проверок менять нельзя, не меняя эту
        // договорённость.
        if (rule.groups.IsEmpty ()) {
            rule.parseValid = false;
            rule.parseError = ParseError::NoGroupsAccepted;
        }
        if (rule.out_paramrawname.IsEmpty ()) {
            rule.parseValid = false;
            rule.parseError = ParseError::EmptyOutputSchema;
        }
        if (rule.out_sum_paramrawname.IsEmpty ()) {
            rule.parseValid = false;
            rule.parseError = ParseError::EmptySumSchema;
        }
        return rule;
    }

    // --------------------------------------------------------------------
    // Получение свойств избранного элемента для размещения
    // Назначение: читает свойства и GDL-параметры элемента из избранного (favorite)
    // Параметры:
    //   favorite_name - имя элемента в избранном
    //   paramdict - [OUT] словарь: ключ = rawName (@property:... или @gdl:...), значение = описание свойства
    // Алгоритм:
    //   1. Пытается загрузить избранный элемент через ACAPI_Favorite_Get
    //   2. Читает пользовательские свойства (favorite.properties) и добавляет в словарь
    //   3. Читает GDL-параметры (favorite.memo.params) и добавляет в словарь
    //   4. Если избранное не найдено - читает настройки по умолчанию для объекта (ACAPI_Element_GetDefaults)
    //   5. Добавляет GDL-параметры объекта по умолчанию
    //   6. Добавляет определения пользовательских свойств элемента по умолчанию
    // Возвращает: код ошибки
    // Примечание: вызывается в SpecArray для проверки наличия необходимых параметров у избранного
    // --------------------------------------------------------------------
    // --------------------------------------------------------------------
    // Состояние свойства-флага правила по одному API_Property.
    // Назначение: разложить ответ SDK на отдельные состояния, не подменяя
    //   «недоступно» и «не вычислено» значением true.
    // Параметры:
    //   property - прочитанное свойство
    //   flag - [OUT] состояние флага (origin и sourceName остаются как заданы)
    // Особенности:
    //   isSingleValue проверяется по definition.collectionType, потому что у
    //   перечислений и списков поля singleVariant не существует вовсе.
    //   Значение по умолчанию берётся из definition ТОЛЬКО когда свойство не
    //   вычислено, поэтому isDefault безусловно перекрывать нельзя.
    // --------------------------------------------------------------------
    void EvaluateRuleFlag (const API_Property &property, RuleFlagCheck &flag) {
        flag.checked = false;
        flag.value = false;
        flag.isDefault = property.isDefault;
        flag.isSingleValue = property.definition.collectionType == API_PropertySingleCollectionType;
#ifndef ServerMainVers_2400
        // До AC24 вычисленность выражает isEvaluated; полем status эти версии не
        // обходятся вообще.
        flag.evaluated = property.isEvaluated;
#else
        flag.evaluated = property.status == API_Property_HasValue;
#endif
        if (!flag.isSingleValue) {
            // Значение не одиночное: булево поле смотреть не на что.
            flag.status = RuleFlagStatus::NotPresent;
            return;
        }
        if (property.definition.valueType != API_PropertyBooleanValueType) {
            flag.status = RuleFlagStatus::NotPresent;
            return;
        }
#ifndef ServerMainVers_2400
        if (!property.isEvaluated)
            flag.status = RuleFlagStatus::NotEvaluated;
        else
            flag.status = RuleFlagStatus::HasValue;
        const API_PropertyValue &value =
            property.isDefault && !property.isEvaluated ? property.definition.defaultValue.basicValue : property.value;
#else
        if (property.status == API_Property_NotAvailable) {
            flag.status = RuleFlagStatus::NotAvailable;
            return;
        }
        if (property.status == API_Property_NotEvaluated)
            flag.status = RuleFlagStatus::NotEvaluated;
        else
            flag.status = RuleFlagStatus::HasValue;
        if (property.value.variantStatus != API_VariantStatusNormal) {
            // Вариант не приведён к нормальному виду: значение брать нельзя,
            // даже если статус утверждает, что оно есть.
            flag.checked = false;
            return;
        }
        const API_PropertyValue &value = property.isDefault && property.status == API_Property_NotEvaluated
                                             ? property.definition.defaultValue.basicValue
                                             : property.value;
#endif
        // Откуда взялось значение: при isDefault && не вычислено оно подставлено
        // из определения, и это исходное значение правила, а не прочитанное.
        const bool fromDefinition = flag.status == RuleFlagStatus::NotEvaluated && property.isDefault;
        if (fromDefinition)
            flag.origin = RuleFlagOrigin::DefaultDefinition;
        flag.checked = true;
        flag.value = value.singleVariant.variant.boolValue;
    }

    // --------------------------------------------------------------------
    // Имена, которые правило требует прочитать, но которых нет в словаре
    //   чтения элемента.
    // Назначение: перечислить расхождения между тем, что правило просит, и тем,
    //   что реально прочитано (CollectRuleDependencies + BuildReadParamDict
    //   задают запрос, ElementsRead его выполняет).
    // Параметры:
    //   rule - разобранное правило
    //   elemguid - элемент, для которого выполнено чтение
    //   read - словарь прочитанного (SpecReadContext::read)
    //   missing - [OUT] имена без значения либо с невалидным значением
    // Возвращает: число расхождений
    // Примечание: признак «имя не найдено вовсе» и «значение помечено
    //   невалидным» — РАЗНЫЕ случаи, но оба означают «прочитать не удалось»,
    //   поэтому попадают в один список; порядок — порядок обхода
    //   зависимостей, а не алфавитный.
    // --------------------------------------------------------------------
    UInt32 CollectUnreadRuleNames (const SpecRule &rule,
                                   const API_Guid &elemguid,
                                   const ParamDictElement &read,
                                   GS::Array<GS::UniString> &missing) {
        missing.Clear ();
        if (elemguid == APINULLGuid)
            return 0;
        const ParamDictValue *values = read.GetPtr (elemguid);
        if (values == nullptr)
            return 0;
        const RuleDependencies dependencies = CollectRuleDependencies (rule);
        for (const auto &cItt : dependencies.read) {
#ifdef ServerMainVers_2800
            const GS::UniString &rawname = cItt.key;
#else
            const GS::UniString &rawname = *cItt.key;
#endif
            // Литерал-счётчик и пустое имя читать нечего: BuildReadParamDict их
            // пропускает, поэтому отсутствие в словаре не является расхождением.
            if (rawname.IsEmpty () || rawname.IsEqual ("1"))
                continue;
            // Служебные имена читаются как части составного значения: их в
            // словаре элемента нет по построению, их наполняет разбор
            // материала/формулы.
            if (rawname.Contains (MATERIALNAMEPREFIX) || rawname.Contains (FORMULANAMEPREFIX))
                continue;
            const ParamValue *value = values->GetPtr (rawname);
            if (value == nullptr || !value->isValid)
                missing.Push (rawname);
        }
        return missing.GetSize ();
    }

    // --------------------------------------------------------------------
    // Определение свойства-правила по GUID.
    // Отдельная функция, а не GetRuleFromElement: та ищет правила перебором
    // описаний элемента и попутно решает, включён ли флаг, а здесь нужен
    // ОДИН запрос по GUID без перебора и без влияния на словарь правил.
    // --------------------------------------------------------------------
    static bool GetRulePropertyDefinition (const API_Guid &propertyGuid, API_PropertyDefinition &definition) {
        definition = {};
        definition.guid = propertyGuid;
        return ACAPI_Property_GetPropertyDefinition (definition) == NoError;
    }

    // --------------------------------------------------------------------
    // Значение свойства-флага на элементе.
    // Отсутствие определения у элемента (ACAPI_Element_GetPropertyValue вернул
    // не NoError) — это NotPresent, а не «флаг выключен»: свойство может быть
    // просто не применимо к типу элемента.
    // --------------------------------------------------------------------
    static bool ReadElementRuleFlag (const API_Guid &elemguid,
                                     const API_Guid &propertyGuid,
                                     RuleFlagCheck &flag,
                                     RuleFlagOrigin origin) {
        API_Property property = {};
        flag = {};
        flag.origin = origin;
        flag.sourceName = APIGuidToString (elemguid);
        if (ACAPI_Element_GetPropertyValue (elemguid, propertyGuid, property) != NoError) {
            flag.status = RuleFlagStatus::NotPresent;
            return false;
        }
        EvaluateRuleFlag (property, flag);
        return true;
    }

    // --------------------------------------------------------------------
    // Проверка правила спецификации по GUID свойства-правила.
    // Назначение: read-only проверка корректности правила и полноты данных
    //   для его выполнения, без создания элементов спецификации.
    // Параметры:
    //   propertyGuid - GUID свойства, в описании которого живёт правило
    //   elemguid - элемент для проверки; APINULLGuid — элемент не задан
    //   result - [OUT] разобранное правило и результаты проверок
    // Возвращает: true, если определение свойства найдено в проекте
    // Алгоритм:
    //   1. Читается определение свойства по GUID (без перебора описаний).
    //   2. Описание нормализуется и разбирается теми же функциями, что и запуск
    //      (NormalizeRuleDescription -> GetRuleFromDescription).
    //   3. Назначение правила читается тем же GetElementForPlaceProperties, что
    //      использует запуск, но с наблюдателем источника: так валидатор видит
    //      и избранное, и случайный fallback на объект по умолчанию.
    //   4. При заданном элементе собираются те же зависимости, что и для
    //      запуска, и читаются существующим чтением элементов.
    // Возвращает правило в result.rule ДАЖЕ при неудачном разборе, чтобы
    //   вызывающий получил причину отказа, а не пустую структуру.
    // --------------------------------------------------------------------
    bool CheckRuleByPropertyGuid (const API_Guid &propertyGuid, const API_Guid &elemguid, RuleCheckResult &result) {
        result = {};
        API_PropertyDefinition definition = {};
        if (!GetRulePropertyDefinition (propertyGuid, definition)) {
            result.rule.parseValid = false;
            return false;
        }
        result.definitionFound = true;
        GS::UniString fullName = EMPTYSTRING;
        GetPropertyFullName (definition, fullName);
        result.propertyName = fullName;

        // Разбор описания: тот же путь, что и у AddRule, включая обрезку по
        // первой закрывающей скобке — иначе длинное описание с пояснением
        // разбиралось бы иначе, чем при запуске.
        GS::UniString description = NormalizeRuleDescription (definition.description);
        GS::Array<GS::UniString> partstring = {};
        if (StringSplt (description, BRACEEND, partstring, "pec_rule") > 0) {
            description = partstring[0] + BRACEEND;
        }
        SpecRule rule = GetRuleFromDescription (description);
        if (rule.parseValid) {
            // Те же поля, что ставит AddRule: без них разобранное правило не
            // описывает само себя.
            rule.rule_name = fullName;
            rule.subguid_paramrawname = fullName;
            rule.subguid_rulevalue = fullName;
            rule.rule_definitions = definition;
        }
        result.rule = rule;
        result.ruleParsed = rule.parseValid;
        result.parseError = rule.parseError;
        if (!rule.parseValid)
            return true; // Причина отказа уже в result.parseError.

        // Проверка назначения: избранное из правила (или объект по умолчанию).
        // Сверка выходной схемы идёт той же функцией, что и при запуске, но в
        // ОТДЕЛЬНУЮ копию правила: сброс destinationReady проверки не должен
        // влиять на то, что возвращается вызывающему.
        GS::HashTable<GS::UniString, GS::UniString> destination = {};
        PlaceSourceInfo source = {};
        // Наблюдатель передаётся всегда: без него валидатор не отличил бы
        // найденное избранное от чтения настроек объекта по умолчанию.
        const GSErrCode destErr = GetElementForPlaceProperties (rule.favorite_name, destination, &source);
        result.favoriteFound = source.favoriteFound;
        result.fromDefaultElem = source.fromDefaultElem || destErr != NoError;
        if (destErr == NoError) {
            SpecRule destinationRule = rule;
            ParamDict destinationErrors = {};
            MatchDestinationProperties (destinationRule, destination, destinationErrors);
            for (const auto &cIt : destinationErrors) {
#ifdef ServerMainVers_2800
                const GS::UniString &name = cIt.key;
#else
                const GS::UniString &name = *cIt.key;
#endif
                result.missingWrite.Push (name);
            }
            // Флаг правила у назначения: у избранного, если оно найдено, иначе у
            // объекта по умолчанию. Проверяется ВСЕГДА, в том числе когда
            // назначение признано негодным по выходной схеме: отсутствие
            // свойства-флага у избранного — самостоятельный дефект правила.
            RuleFlagCheck &flag = result.destinationFlag;
            flag = {};
            flag.sourceName = rule.favorite_name;
            for (const API_Property &property : source.properties) {
                if (property.definition.guid != propertyGuid)
                    continue;
                EvaluateRuleFlag (property, flag);
                flag.sourceName = rule.favorite_name;
                break;
            }
            if (flag.status == RuleFlagStatus::Unknown) {
                flag.status = RuleFlagStatus::NotPresent;
                flag.checked = false;
            }
            flag.origin = source.favoriteFound ? RuleFlagOrigin::FavoriteValue : RuleFlagOrigin::DefaultElemValue;
        } else {
            result.destinationFlag.status = RuleFlagStatus::Unknown;
            result.destinationFlag.origin = RuleFlagOrigin::NotChecked;
            result.destinationFlag.sourceName = rule.favorite_name;
        }

        // Проверка с элементом. Без элемента читается только проект: тогда
        // проверяется, что определения нужных свойств в проекте вообще есть.
        if (elemguid == APINULLGuid) {
            auto &cache = PROPERTYCACHE ();
            if (!cache.isPropertyDefinitionRead_full)
                cache.ReadPropertyDefinition ();
            if (!cache.isPropertyDefinition_OK)
                return true; // Кэш недоступен — это не дефект правила.
            for (const auto &cItt : CollectRuleDependencies (rule).read) {
#ifdef ServerMainVers_2800
                const GS::UniString &rawname = cItt.key;
#else
                const GS::UniString &rawname = *cItt.key;
#endif
                if (rawname.IsEmpty () || rawname.IsEqual ("1"))
                    continue;
                if (rawname.Contains (MATERIALNAMEPREFIX) || rawname.Contains (FORMULANAMEPREFIX))
                    continue;
                if (cache.property.GetPtr (rawname) == nullptr)
                    result.unresolvedInProject.Push (rawname);
            }
            for (const auto &cItt : CollectRuleDependencies (rule).write) {
#ifdef ServerMainVers_2800
                const GS::UniString &rawname = cItt.key;
#else
                const GS::UniString &rawname = *cItt.key;
#endif
                if (cache.property.GetPtr (rawname) == nullptr)
                    result.unresolvedInProject.Push (rawname);
            }
            return true;
        }

        result.checkedElement = true;
        ReadElementRuleFlag (elemguid, propertyGuid, result.elementFlag, RuleFlagOrigin::ElementValue);
        // Чтение элемента идёт существующим путём чтения аддона: собираются те
        // же имена, что и для запуска, и выполняется одно чтение на элемент.
        ParamDictElement paramToRead = {};
        ParamDictValue paramDict = {};
        BuildReadParamDict (CollectRuleDependencies (rule).read, paramDict);
        if (!paramDict.IsEmpty ()) {
            ParamHelpers::AddParamDictValue2ParamDictElement (elemguid, paramDict, paramToRead);
        }
        ParamDictCompositeElement paramComposite = {};
        ListData::LibElements listData = {};
        if (!paramToRead.IsEmpty ()) {
            ParamHelpers::ElementsRead (paramToRead, paramComposite, listData, true, true);
        }
        CollectUnreadRuleNames (rule, elemguid, paramToRead, result.missingRead);
        return true;
    }

    GSErrCode GetElementForPlaceProperties (const GS::UniString &favorite_name,
                                            GS::HashTable<GS::UniString, GS::UniString> &paramdict,
                                            PlaceSourceInfo *readInfo) {
        GSErrCode err = NoError;
        API_Element element = {};
        API_ElementMemo memo = {};
        // Источник чтения отслеживается только когда запрошен: прежние вызовы
        // передают nullptr и о нём ничего не знают.
        if (readInfo != nullptr) {
            readInfo->properties.Clear ();
            readInfo->name = favorite_name;
            readInfo->favoriteFound = false;
            readInfo->fromDefaultElem = false;
        }
#ifdef ServerMainVers_2300
        if (!favorite_name.IsEmpty ()) {
            API_Favorite favorite (favorite_name);
            favorite.memo.New ();
            favorite.properties.New ();
            BNZeroMemory (&favorite.memo.Get (), sizeof (API_ElementMemo));
            err = ACAPI_Favorite_Get (&favorite);
            if (err == NoError) {
                if (readInfo != nullptr) {
                    readInfo->favoriteFound = true;
                    if (favorite.properties.HasValue ())
                        readInfo->properties = favorite.properties.Get ();
                }
                if (favorite.properties.HasValue ()) {
                    for (const auto &property : favorite.properties.Get ()) {
                        GS::UniString fname;
                        GS::UniString rawName = PROPERTYNAMEPREFIX;
                        GetPropertyFullName (property.definition, fname);
                        rawName.Append (fname.ToLowerCase ());
                        rawName.Append (BRACEEND);
                        if (!paramdict.ContainsKey (rawName))
                            paramdict.Add (rawName, property.definition.description.ToLowerCase ());
                    }
                }
                if (favorite.memo.HasValue ()) {
                    if (favorite.memo.Get ().params != nullptr) {
                        const GSSize nParams =
                            BMGetHandleSize ((GSHandle)favorite.memo.Get ().params) / sizeof (API_AddParType);
                        for (GSIndex ii = 0; ii < nParams; ++ii) {
                            API_AddParType &actParam = (*favorite.memo.Get ().params)[ii];
                            GS::UniString fname = GS::UniString (actParam.name);
                            GS::UniString rawName = GDLNAMEPREFIX;
                            rawName.Append (fname.ToLowerCase ());
                            rawName.Append (BRACEEND);
                            if (!paramdict.ContainsKey (rawName))
                                paramdict.Add (rawName, EMPTYSTRING);
                        }
                    }
                }
                ACAPI_DisposeElemMemoHdls (&favorite.memo.Get ());
                return err;
            } else {
                msg_rep ("Spec",
                         "Can't find favorite with name: " + favorite_name + " . Contiune with default objec.",
                         err,
                         APINULLGuid);
            }
        }
#endif
        SetElemTypeID (element, API_ObjectID);
#ifndef ServerMainVers_2300
        element.header.variationID = APIVarId_Object;
#endif
        // Источник чтения — объект по умолчанию: избранного нет либо оно не
        // найдено. Это отдельный признак, а не следствие успеха чтения.
        if (readInfo != nullptr)
            readInfo->fromDefaultElem = true;
        msg_rep ("Spec", "Read the default settings of the object", err, APINULLGuid);
        err = ACAPI_Element_GetDefaults (&element, &memo);
        if (err != NoError) {
            msg_rep ("Spec", "ACAPI_Element_GetDefaults", err, APINULLGuid);
            ACAPI_DisposeElemMemoHdls (&memo);
            return err;
        }
        if (memo.params != nullptr) {
            const GSSize nParams = BMGetHandleSize ((GSHandle)memo.params) / sizeof (API_AddParType);
            for (GSIndex ii = 0; ii < nParams; ++ii) {
                API_AddParType &actParam = (*memo.params)[ii];
                GS::UniString fname = GS::UniString (actParam.name);
                GS::UniString rawName = GDLNAMEPREFIX;
                rawName.Append (fname.ToLowerCase ());
                rawName.Append (BRACEEND);
                if (!paramdict.ContainsKey (rawName))
                    paramdict.Add (rawName, EMPTYSTRING);
            }
        }
        ACAPI_DisposeElemMemoHdls (&memo);
        GS::Array<API_PropertyDefinition> definitions = {};
#ifdef ServerMainVers_2600
        err = ACAPI_Element_GetPropertyDefinitionsOfDefaultElem (
            element.header.type, API_PropertyDefinitionFilter_UserDefined, definitions);
#else
        err = ACAPI_Element_GetPropertyDefinitionsOfDefaultElem (
            element.header.typeID, element.header.variationID, API_PropertyDefinitionFilter_UserDefined, definitions);
#endif
        if (err != NoError) {
            msg_rep ("Spec", "ACAPI_Element_GetPropertyDefinitionsOfDefaultElem", err, APINULLGuid);
            return err;
        }
        for (const auto &definition : definitions) {
            GS::UniString fname;
            GS::UniString rawName = PROPERTYNAMEPREFIX;
            GetPropertyFullName (definition, fname);
            rawName.Append (fname.ToLowerCase ());
            rawName.Append (BRACEEND);
            if (!paramdict.ContainsKey (rawName))
                paramdict.Add (rawName, definition.description.ToLowerCase ());
        }
        return err;
    }

    // --------------------------------------------------------------------
    // Получение элемента из избранного для размещения
    // Назначение: загружает элемент из избранного (favorite) или создаёт элемент по умолчанию
    // Параметры:
    //   favorite_name - имя элемента в избранном (если пустое - создаётся элемент по умолчанию)
    //   element - [OUT] структура элемента (заполняется)
    //   memo - [OUT] memo-структура элемента (заполняется)
    // Алгоритм:
    //   1. Если favorite_name не пустое - пытается загрузить элемент из избранного (ACAPI_Favorite_Get)
    //   2. Если не удалось или имя пустое - создаёт элемент по умолчанию (ACAPI_Element_GetDefaults)
    // Возвращает: код ошибки (NoError при успехе)
    // Примечание: вызывается для каждого создаваемого элемента спецификации
    // --------------------------------------------------------------------
    GSErrCode GetElementForPlace (const GS::UniString &favorite_name, API_Element &element, API_ElementMemo &memo) {
        GSErrCode err = NoError;
#ifdef ServerMainVers_2300
        if (!favorite_name.IsEmpty ()) {
            API_Favorite favorite (favorite_name);
            favorite.memo.New ();
            BNZeroMemory (&favorite.memo.Get (), sizeof (API_ElementMemo));
            err = ACAPI_Favorite_Get (&favorite);
            if (err == NoError) {
                element = favorite.element;
                memo = *favorite.memo;
                return err;
            } else {
                ACAPI_DisposeElemMemoHdls (&favorite.memo.Get ());
            }
        }
#endif
        SetElemTypeID (element, API_ObjectID);
#ifndef ServerMainVers_2300
        element.header.variationID = APIVarId_Object;
#endif
        err = ACAPI_Element_GetDefaults (&element, &memo);
        if (err != NoError) {
            ACAPI_DisposeElemMemoHdls (&memo);
            msg_rep ("Spec", "ACAPI_Element_GetDefaults", err, APINULLGuid);
        }
        return err;
    }

    // --------------------------------------------------------------------
    // Размещение создаваемых элементов спецификации
    // Назначение: создаёт элементы на чертеже на основе словаря elementstocreate
    // Параметры:
    //   elementstocreate - массив словарей элементов для создания (по правилам)
    //   paramToWrite - параметры для записи (из избранного элемента)
    //   paramOut - [OUT] словарь свойств для записи в созданные элементы
    //   startpos -起始ная точка размещения (получается кликом мышью)
    // Алгоритм:
    //   1. Определяет текущий этаж
    //   2. Для каждого элемента в elementstocreate:
    //      - получает элемент из избранного (GetElementForPlace)
    //      - записывает параметры (GUID, имя правила, значения параметров)
    //      - записывает GDL-параметры в memo.params
    //      - размещает элемент (ACAPI_Element_Create)
    //      - обновляет позицию для следующего элемента (сетка)
    //      - группирует элементы, если в группе больше одного
    //   3. Запускает GDL-скрипты параметров (RunGDLParScript)
    // Возвращает: код ошибки (NoError при успехе)
} // namespace Spec
