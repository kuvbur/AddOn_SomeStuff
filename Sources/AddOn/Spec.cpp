//------------ kuvbur 2022 ------------
#include "api_headers/APIEnvir.h"

#include "ACAPinc.h"

#include "Spec.hpp"
#include "Sync.hpp"
#ifdef TESTING
    #include "TestFunc.hpp"
#endif
#include "DG4rule.hpp"
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
    //   5. Если флаг включён - добавляет элемент в rule.elements
    // Возвращает: true, если найдены элементы (даже если флаги выключены)
    // Примечание: для AC_22 всегда возвращает false
    // --------------------------------------------------------------------
    // -----------------------------------------------------------------------------
    // Ищет правила спецификации в свойствах элемента по умолчанию.
    // Это нужно, когда пользователь не выделил конкретные элементы и правила берутся из шаблона.
    // -----------------------------------------------------------------------------
    bool GetRuleFromDefaultElem (SpecRuleDict &rules, API_DatabaseInfo &homedatabaseInfo, bool &has_elementspec) {
#if defined(AC_22)
        return false;
#else
        GSErrCode error = NoError;
        GS::Array<API_PropertyDefinition> definitions = {};
    #if defined(AC_27) || defined(AC_28) || defined(AC_29) || defined(AC_26)
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
    #if defined(AC_28) || defined(AC_29)
            SpecRule &rule = cIt->value;
    #else
            SpecRule &rule = *cIt->value;
    #endif
            if (!rule.is_Valid)
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
    #if defined(AC_22) || defined(AC_23)
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
                        rule.elements.PushNew (elemGuid);
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
            const Int32 iseng = ID_ADDON_STRINGS + isEng ();
            GS::UniString SpecRuleNotFoundString = RSGetIndString (iseng, SpecFlagOff, ACAPI_GetOwnResModule ());
            if (!error_name.IsEmpty ()) {
                for (auto &cIt : error_name) {
    #if defined(AC_28) || defined(AC_29)
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
        return has_element;
#endif
    }

    GSErrCode SpecAll (const SyncSettings &syncSettings) {
        GSErrCode err = NoError;
        API_DatabaseInfo homedatabaseInfo = {};
#if defined(AC_27) || defined(AC_28) || defined(AC_29)
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
            hasrule = GetRuleFromDefaultElem (rules, homedatabaseInfo, has_elementspec);
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
        // Ранний выход допустим только если нет элементов для обработки.
        // Даже если правила получены из элемента по умолчанию, это не означает, что нужно завершаться,
        // потому что часть логики может быть взята из самих выбранных элементов.
        if (guidArray.IsEmpty ())
            return NoError;
        err = SpecArray (syncSettings, guidArray, rules, selected_elements);
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
#if defined(AC_27) || defined(AC_28) || defined(AC_29)
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
    // -----------------------------------------------------------------------------
    // Отбрасывает из массива все неподходящие элементы и оставляет только те, что реально можно обработать.
    // -----------------------------------------------------------------------------
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
#if defined(AC_27) || defined(AC_28) || defined(AC_29)
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
    //   3. Если пользователь нажал OK - обновляет состояние правил (rule.is_Valid)
    // Возвращает: true, если пользователь выбрал хотя бы одно правило и нажал OK
    // Примечание: если пользователь отменил диалог - возвращает false
    // --------------------------------------------------------------------
    // -----------------------------------------------------------------------------
    // Показывает пользователю диалог выбора активных правил спецификации.
    // Важно: здесь не меняется сама логика правил, только их активность для текущего запуска.
    // -----------------------------------------------------------------------------
    bool SpecDG (SpecRuleDict &spec_rules, bool &rule_from_one) {
        RuleSelectData rules = {};
        for (GS::HashTable<GS::UniString, SpecRule>::PairIterator cIt = spec_rules.EnumeratePairs (); cIt != NULL;
             ++cIt) {
#if defined(AC_28) || defined(AC_29)
            const SpecRule &rule = cIt->value;
#else
            const SpecRule &rule = *cIt->value;
#endif
            if (!rule.is_Valid)
                continue;
            if (rules.rules.ContainsKey (rule.rule_name))
                continue;
            if (rules.qty_elements.ContainsKey (rule.rule_name))
                continue;
            rules.rules.Add (rule.rule_name, true);
            rules.qty_elements.Add (rule.rule_name, GS::UniString::Printf ("%d", rule.elements.GetSize ()));
        }
        rules.is_warn = rule_from_one;
        rules.titleResID = UndoSumId;
        RuleSelectDialog dialog (rules);
        if (!dialog.Invoke ())
            return false;
        bool has_true_state = false;
        for (GS::HashTable<GS::UniString, SpecRule>::PairIterator cIt = spec_rules.EnumeratePairs (); cIt != NULL;
             ++cIt) {
#if defined(AC_28) || defined(AC_29)
            SpecRule &rule = cIt->value;
#else
            SpecRule &rule = *cIt->value;
#endif
            if (!rule.is_Valid)
                continue;
            if (!rules.rules.ContainsKey (rule.rule_name))
                continue;
            rule.is_Valid = rules.rules.Get (rule.rule_name);
            if (rule.is_Valid)
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
    // -----------------------------------------------------------------------------
    // Основная функция создания спецификации из набора элементов.
    // Здесь правила собираются, параметры читаются, элементы создаются/обновляются и затем записываются.
    // -----------------------------------------------------------------------------
    GSErrCode SpecArray (const SyncSettings &syncSettings,
                         GS::Array<API_Guid> &guidArray,
                         SpecRuleDict &rules,
                         const UnicGuid &selected_elements) {
        clock_t start, finish;
        double duration;
        start = clock ();
        GS::UniString funcname = "SpecAll";
        GS::Int32 nPhase = 4;
        GS::UniString subtitle = "";
        Int32 maxval = 1;
        short i = 1;
        ParamDictElement paramToRead = {};                   // Словарь с параметрами для чтения
        ParamDictCompositeElement paramCompositeToRead = {}; // Прочитанные составы конструкции
        ListData::LibElements paramListDataToRead = {};      // Прочитанные данные объектов
        ParamDictValue paramToWrite = {};                    // Словарь с параметрами для записи (с нулевым GUID)
        GS::Array<ElementDict> elements_new = {};            // Массив со словарём создаваемых элементов
        GS::Array<ElementDict> elements_mod = {};            // Массив со словарём модифицируемых элементов
        GS::Array<API_Guid> elements_delete = {};            // Массив удаляемых элементов
        ParamDictElement paramOut = {};                      // Словарь свойств для записи в расставленные элементы
        GS::Array<API_Guid> guidArraysync = {}; // Список элементов, которые требуется синхронизировать (расставленные
                                                // элементы)
        UnicGuid error_element = {};            // Элементы с ошибками
        ParamDict error_name = {};              // Список имён, не найденных у избранного
        GS::HashTable<GS::UniString, GS::HashTable<GS::UniString, GS::UniString>> paramdict_favorite =
            {}; // Словарь с именами параметров и описаниями свойств избранных элементов
#if defined(AC_27) || defined(AC_28) || defined(AC_29)
        bool showPercent = true;
#endif
        ProcessWindowGuard pwGuard (funcname, nPhase);
        GSErrCode err = NoError;
        subtitle = GS::UniString::Printf ("Get rule from %d elements", guidArray.GetSize ());
#if defined(AC_27) || defined(AC_28) || defined(AC_29)
        maxval = 2;
        ACAPI_ProcessWindow_SetNextProcessPhase (&subtitle, &maxval, &showPercent);
#else
        i = 2;
        ACAPI_Interface (APIIo_SetNextProcessPhaseID, &subtitle, &i);
#endif
        int dummymode = IsDummyModeOn ();
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
                GS::UniString SpecRuleNotFoundString =
                    RSGetIndString (iseng, SpecRuleNotFoundId, ACAPI_GetOwnResModule ());
                ACAPI_WriteReport (SpecRuleNotFoundString, true);
                return APIERR_GENERAL;
            }
        }
        // Теперь пройдём по правилам и соберём все нужные параметры для чтения из исходных элементов.
        for (GS::HashTable<GS::UniString, SpecRule>::PairIterator cIt = rules.EnumeratePairs (); cIt != NULL; ++cIt) {
#if defined(AC_28) || defined(AC_29)
            SpecRule &rule = cIt->value;
#else
            SpecRule &rule = *cIt->value;
#endif
            // Читаем только параметры групп
            if (!rule.is_Valid) {
                continue;
            }
            GetParamToReadFromRule (rule, paramToRead, paramToWrite);
        }
        if (paramToRead.IsEmpty ()) {
            msg_rep ("Spec", "Parameters for read not found", APIERR_GENERAL, APINULLGuid);
            GS::UniString SpecRuleReadFoundString =
                RSGetIndString (iseng, SpecRuleReadFoundId, ACAPI_GetOwnResModule ());
            ACAPI_WriteReport (SpecRuleReadFoundString, true);
            return APIERR_GENERAL;
        }
        if (paramToWrite.IsEmpty ()) {
            msg_rep ("Spec", "Parameters for write not found", APIERR_GENERAL, APINULLGuid);
            GS::UniString SpecWriteNotFoundString =
                RSGetIndString (iseng, SpecWriteNotFoundId, ACAPI_GetOwnResModule ());
            ACAPI_WriteReport (SpecWriteNotFoundString, true);
            return APIERR_GENERAL;
        }
        subtitle = GS::UniString::Printf ("Reading parameters from %d elements", paramToRead.GetSize ());
#if defined(AC_27) || defined(AC_28) || defined(AC_29)
        maxval = 2;
        ACAPI_ProcessWindow_SetNextProcessPhase (&subtitle, &maxval, &showPercent);
#else
        i = 2;
        ACAPI_Interface (APIIo_SetNextProcessPhaseID, &subtitle, &i);
#endif
        // Читаем свойства избранного
        for (GS::HashTable<GS::UniString, SpecRule>::PairIterator cIt = rules.EnumeratePairs (); cIt != NULL; ++cIt) {
#if defined(AC_28) || defined(AC_29)
            SpecRule &rule = cIt->value;
#else
            SpecRule &rule = *cIt->value;
#endif
            if (!rule.is_Valid)
                continue;
            if (!paramdict_favorite.ContainsKey (rule.favorite_name)) {
                GS::HashTable<GS::UniString, GS::UniString> paramdict = {};
                err = GetElementForPlaceProperties (rule.favorite_name, paramdict);
                paramdict_favorite.Add (rule.favorite_name, paramdict);
            }
            if (!paramdict_favorite.ContainsKey (rule.favorite_name))
                continue;
            for (const auto &rawname : rule.out_paramrawname) {
                if (!paramdict_favorite.Get (rule.favorite_name).ContainsKey (rawname)) {
                    rule.is_Valid = false;
                    if (!error_name.ContainsKey (rawname))
                        error_name.Add (rawname, true);
                }
            }
            for (const auto &rawname : rule.out_sum_paramrawname) {
                if (!paramdict_favorite.Get (rule.favorite_name).ContainsKey (rawname)) {
                    rule.is_Valid = false;
                    if (!error_name.ContainsKey (rawname))
                        error_name.Add (rawname, true);
                }
            }
            if (!rule.is_Valid)
                continue;
            bool flag_find = false;
            GS::HashTable<GS::UniString, GS::UniString> &rule_favorite_name =
                paramdict_favorite.Get (rule.favorite_name);
            for (const auto &cItt : rule_favorite_name) {
#if defined(AC_28) || defined(AC_29)
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
                // Ищем свойство, в которое нужно будет записать GUID
                if (!rule.subguid_paramrawname.IsEmpty ()) {
                    if (description.Contains (rule.subguid_paramrawname.ToLowerCase ()) &&
                        description.Contains ("sync_guid")) {
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
                        rule.subguid_paramrawname = rawname;
                        flag_find = true;
                    }
                }
                if (flag_find && !rule.subguid_rulename.IsEmpty ())
                    break;
            }
            // Поиск существующих объектов
            if (!rule.delete_old)
                continue;
            if (rule.subguid_rulename.IsEmpty ())
                continue;
            if (!flag_find)
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
            if (!selected_elements.IsEmpty ()) {
                for (const API_Guid &exsist_element : exsist_elements) {
                    if (!selected_elements.ContainsKey (exsist_elements[i]))
                        continue;
                    rule.exsist_elements.Push (exsist_elements[i]);
                }
            } else {
                rule.exsist_elements = exsist_elements;
            }
            if (rule.exsist_elements.IsEmpty ())
                continue;
            // Собираем список параметрв для чтения у существующих элементов
            ParamDictValue paramDict = {}; // Словарь параметров для чтения для одного элемента
            for (const GS::UniString &rawname : rule.out_sum_paramrawname) {
                if (!paramDict.ContainsKey (rawname))
                    ParamHelpers::AddValueToParamDictValue (paramDict, rawname);
            }
            for (const GS::UniString &rawname : rule.out_paramrawname) {
                if (!paramDict.ContainsKey (rawname))
                    ParamHelpers::AddValueToParamDictValue (paramDict, rawname);
            }
            ParamHelpers::AddValueToParamDictValue (paramDict, rule.subguid_paramrawname);
            // Добавляем параметры для каждого элемента
            for (const API_Guid elemguid : rule.exsist_elements) {
                ParamHelpers::AddParamDictValue2ParamDictElement (elemguid, paramDict, paramToRead);
            }
        }
        // Если для размещаемого объекта не удалось найти нужные параметры, дальнейшая работа бессмысленна.
        if (!error_name.IsEmpty ()) {
            GS::UniString out = ":\n";
            for (auto &cIt : error_name) {
#if defined(AC_28) || defined(AC_29)
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
            GS::UniString SpecEmptyListdString =
                RSGetIndString (iseng, SpecParamPlaceNotFoundId, ACAPI_GetOwnResModule ());
            ACAPI_WriteReport (SpecEmptyListdString + out, true);
            return APIERR_GENERAL;
        }
        // Перед формированием итоговых элементов читаются данные уже размещённых объектов, чтобы их можно было сравнить
        // с правилами.
        bool rule_from_one = false;
        if (!SpecDG (rules, rule_from_one)) {
            msg_rep ("ReNumSelected", "Execution interrupted by user", NoError, APINULLGuid);
            return false;
        }
        ParamHelpers::ElementsRead (paramToRead, paramCompositeToRead, paramListDataToRead, true, true);
        // Массив со словарями элементов для создания по правилам
        Int32 n_elements = 0; // Количество создаваемых элементов для отчёта
        bool has_v2 = false;
        for (GS::HashTable<GS::UniString, SpecRule>::PairIterator cIt = rules.EnumeratePairs (); cIt != NULL; ++cIt) {
#if defined(AC_28) || defined(AC_29)
            SpecRule &rule = cIt->value;
#else
            SpecRule &rule = *cIt->value;
#endif
            if (!rule.is_Valid)
                continue;
            ElementDict elements_n = {}; // Словарь создаваемых элементов для правила
            ElementDict elements_m = {};
            n_elements += GetElementsForRule (rule,
                                              paramToRead,
                                              paramCompositeToRead,
                                              paramListDataToRead,
                                              elements_n,
                                              elements_m,
                                              elements_delete,
                                              error_element);
            if (!elements_n.IsEmpty ())
                elements_new.Push (elements_n);
            if (!elements_m.IsEmpty ())
                elements_mod.Push (elements_m);
            if (rule.delete_old)
                has_v2 = true;
        }
#ifndef AC_22
        if (!error_element.IsEmpty ()) {
            if (error_element.GetSize () < 20) {
    #if defined(AC_27) || defined(AC_28) || defined(AC_29)
                ACAPI_UserInput_ClearElementHighlight ();
    #else
        #if defined(AC_26)
                ACAPI_Interface_ClearElementHighlight ();
        #else
                ACAPI_Interface (APIIo_HighlightElementsID);
        #endif
    #endif
                GS::HashTable<API_Guid, API_RGBAColor> hlElems = {};
                API_RGBAColor hlColor = {1, 0.0, 0.0, 1};
                GS::Array<API_Neig> error_elements = {};
                for (const auto &cIt : error_element) {
    #if defined(AC_28) || defined(AC_29)
                    API_Guid el = cIt.key;
    #else
                    API_Guid el = *cIt.key;
    #endif
                    hlElems.Add (el, hlColor);
                    error_elements.PushNew (el);
                }
    #if defined(AC_27) || defined(AC_28) || defined(AC_29)
                ACAPI_UserInput_SetElementHighlight (hlElems);
    #else
        #if defined(AC_26)
                ACAPI_Interface_SetElementHighlight (hlElems);
        #else
                ACAPI_Interface (APIIo_HighlightElementsID, &hlElems);
        #endif
    #endif
    #if defined(AC_27) || defined(AC_28) || defined(AC_29)
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
            return APIERR_GENERAL;
        }
#endif
        if (!elements_mod.IsEmpty ()) {
#if defined(AC_27) || defined(AC_28) || defined(AC_29)
            ACAPI_UserInput_ClearElementHighlight ();
#else
    #if defined(AC_26)
            ACAPI_Interface_ClearElementHighlight ();
    #else
            ACAPI_Interface (APIIo_HighlightElementsID);
    #endif
#endif
            GS::HashTable<API_Guid, API_RGBAColor> hlElems = {};
            API_RGBAColor hlColor = {0.8, 0.0, 0.0, 0.5};
            for (const auto &eldict : elements_mod) {
                for (auto &cIt : eldict) {
#if defined(AC_28) || defined(AC_29)
                    Element el = cIt.value;
#else
                    Element el = *cIt.value;
#endif
                    hlElems.Add (el.exs_guid, hlColor);
                    ParamDictValue param = {};
                    if (!el.subguid_paramrawname.IsEmpty ()) {
                        if (paramToWrite.ContainsKey (el.subguid_paramrawname) &&
                            !param.ContainsKey (el.subguid_paramrawname)) {
                            ParamValue paramTo = paramToWrite.Get (el.subguid_paramrawname);
                            GS::UniString instring = APIGuidToString (el.elements[0]);
                            for (UInt32 k = 1; k < el.elements.GetSize (); k++) {
                                instring = instring + SEMICOLON + APIGuid2GSGuid (el.elements[k]).ToUniString ();
                            }
                            paramTo.val.uniStringValue = StringUnic (instring, SEMICOLON);
                            paramTo.isValid = true;
                            paramTo.val.type = API_PropertyStringValueType;
                            param.Add (el.subguid_paramrawname, paramTo);
                        }
                    }
                    if (!el.subguid_rulename.IsEmpty () && !el.subguid_rulevalue.IsEmpty ()) {
                        if (paramToWrite.ContainsKey (el.subguid_rulename) &&
                            !param.ContainsKey (el.subguid_rulename)) {
                            ParamValue paramTo = paramToWrite.Get (el.subguid_rulename);
                            paramTo.val.uniStringValue = el.subguid_rulevalue;
                            paramTo.isValid = true;
                            paramTo.val.type = API_PropertyStringValueType;
                            param.Add (el.subguid_rulename, paramTo);
                        }
                    }
                    for (UInt32 k = 0; k < el.out_paramrawname.GetSize (); k++) {
                        GS::UniString rawname = el.out_paramrawname[k];
                        if (paramToWrite.ContainsKey (rawname) && !param.ContainsKey (rawname)) {
                            FormatString stringformat;
                            ParamValue paramFrom = el.out_param[k];
                            ParamValue paramTo = paramToWrite.Get (rawname);
                            paramTo.val = paramFrom.val;
                            paramTo.isValid = true;
                            param.Add (rawname, paramTo);
                        }
                    }
                    for (UInt32 k = 0; k < el.out_sum_paramrawname.GetSize (); k++) {
                        GS::UniString rawname = el.out_sum_paramrawname[k];
                        if (paramToWrite.ContainsKey (rawname) && !param.ContainsKey (rawname)) {
                            FormatString stringformat;
                            ParamValue paramFrom = el.out_sum_param[k];
                            ParamValue paramTo = paramToWrite.Get (rawname);
                            paramTo.val = paramFrom.val;
                            if (paramTo.fromPropertyDefinition) {
                                if (paramTo.definition.valueType == API_PropertyStringValueType)
                                    paramTo.val.type = API_PropertyStringValueType;
                            }
                            paramTo.isValid = true;
                            param.Add (rawname, paramTo);
                        }
                    }
                    paramOut.Add (el.exs_guid, param);
                }
            }
#if defined(AC_27) || defined(AC_28) || defined(AC_29)
            ACAPI_UserInput_SetElementHighlight (hlElems);
#else
    #if defined(AC_26)
            ACAPI_Interface_SetElementHighlight (hlElems);
    #else
            ACAPI_Interface (APIIo_HighlightElementsID, &hlElems);
    #endif
#endif
        }
        subtitle = GS::UniString::Printf ("Create %d elements", n_elements);
#if defined(AC_27) || defined(AC_28) || defined(AC_29)
        maxval = 3;
        ACAPI_ProcessWindow_SetNextProcessPhase (&subtitle, &maxval, &showPercent);
#else
        i = 3;
        ACAPI_Interface (APIIo_SetNextProcessPhaseID, &subtitle, &i);
#endif
        if (elements_new.IsEmpty () && elements_mod.IsEmpty () && elements_delete.IsEmpty ()) {
            msg_rep ("Spec", "Elements list empty", NoError, APINULLGuid);
            GS::UniString SpecEmptyListdString = RSGetIndString (iseng, SpecEmptyListdId, ACAPI_GetOwnResModule ());
            if (has_v2)
                SpecEmptyListdString += LINEBRAKE + RSGetIndString (iseng, 67, ACAPI_GetOwnResModule ());
            ACAPI_WriteReport (SpecEmptyListdString, true);
            return APIERR_GENERAL;
        }
        Point2D startpos = {0, 0};
        finish = clock ();
        duration = (double)(finish - start) / CLOCKS_PER_SEC;
        if (!elements_new.IsEmpty ()) {
            if (!ClickAPoint ("Click the lower corner of the spec elements creation", &startpos))
                return APIERR_CANCEL;
            start = clock ();
            PlaceElements (elements_new, paramToWrite, paramOut, startpos);
        } else {
            start = clock ();
        }
        ACAPI_CallUndoableCommand ("Writing properties to created spec elements", [&] () -> GSErrCode {
            bool suspGrp = false;
#ifndef AC_22
    #if defined(AC_27) || defined(AC_28) || defined(AC_29)
            err = ACAPI_View_IsSuspendGroupOn (&suspGrp);
            if (!suspGrp)
                ACAPI_Grouping_Tool (elements_delete, APITool_SuspendGroups, nullptr);
    #else
        err = ACAPI_Environment (APIEnv_IsSuspendGroupOnID, &suspGrp);
        if (!suspGrp) ACAPI_Element_Tool (elements_delete, APITool_SuspendGroups, nullptr);
    #endif
            if (!elements_delete.IsEmpty ()) {
                err = ACAPI_Element_Delete (elements_delete);
                msg_rep ("Spec",
                         GS::UniString::Printf ("Removed %d obsolete spec elements", elements_delete.GetSize ()),
                         err,
                         APINULLGuid);
            }
#endif // !AC_22
            ParamHelpers::ElementsWrite (paramOut);
            return NoError;
        });
        if (has_v2) {
            GS::UniString msg = "";
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
#if defined(AC_28) || defined(AC_29)
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
            // TODO Вынести это в отдельную функцию, убрать повторение в GetElemState
            API_Property propertyflag = {};
            if (ACAPI_Element_GetPropertyValue (elemguid, definitions[i].guid, propertyflag) == NoError) {
#if defined(AC_22) || defined(AC_23)
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
    // Примечание: правило добавляется в словарь даже если оно невалидно (is_Valid=false),
    //             чтобы избежать повторной обработки
    // --------------------------------------------------------------------
    // -----------------------------------------------------------------------------
    // Разбирает описание свойства и добавляет правило в словарь.
    // Для сложных строк форматирования здесь выполняется нормализация, чтобы парсер видел понятный текст.
    // -----------------------------------------------------------------------------
    void AddRule (const API_PropertyDefinition &definition, const API_Guid &elemguid, SpecRuleDict &rules) {
        // Чистим описание
        GS::UniString description = definition.description;
        // Нормализуем описание: убираем переводы строк, лишние пробелы и приводим формат
        // к виду, который проще разбить на группы и параметры.
        description.ReplaceAll (LINEBRAKE, EMPTYSTRING);
        description.ReplaceAll (LINEBRAKER, EMPTYSTRING);
        description.ReplaceAll (TABSTRING, EMPTYSTRING);
        description.ReplaceAll ("  ", SPACESTRING);
        description.ReplaceAll ("  ", SPACESTRING);
        description.ReplaceAll ("  ", SPACESTRING);
        description.ReplaceAll ("  ", SPACESTRING);
        description.ReplaceAll ("  ", SPACESTRING);
        description.ReplaceAll ("  ", SPACESTRING);
        description.ReplaceAll (" {", BRACESTART);
        description.ReplaceAll ("{ ", BRACESTART);
        description.ReplaceAll (" }", BRACEEND);
        description.ReplaceAll ("} ", BRACEEND);
        description.ReplaceAll (" ;", SEMICOLON);
        description.ReplaceAll ("; ", SEMICOLON);
        description.ReplaceAll ("gm (", "gm(");
        description.ReplaceAll (" gm(", "gm(");
        description.ReplaceAll ("gl (", "gl(");
        description.ReplaceAll (" gl(", "gl(");
        description.ReplaceAll ("g (", "g(");
        description.ReplaceAll ("s (", "s(");
        description.ReplaceAll (" g(", "g(");
        description.ReplaceAll (" s(", "s(");
        description.ReplaceAll ("g(", "g@@");
        description.ReplaceAll ("gl(", "g@@libdata@");
        description.ReplaceAll ("s(", "s@@");
        description.ReplaceAll (")s", "@@s");
        description.ReplaceAll (")g", "@@g");
        description.ReplaceAll ("))", ")@@");
        description.ReplaceAll ("gm(", "g@@Material_all@");
        GS::Array<GS::UniString> partstring = {};
        if (StringSplt (description, BRACEEND, partstring, "pec_rule") > 0) {
            description = partstring[0] + BRACEEND;
        }
        GS::UniString key = description.GetSubstring (CHARBRACESTART, CHARBRACEEND, 0);
        if (rules.ContainsKey (key)) {
            if (rules.Get (key).is_Valid && elemguid != APINULLGuid)
                rules.Get (key).elements.Push (elemguid);
        } else {
            // Добавление группы и элемента
            SpecRule rule = GetRuleFromDescription (description);
            if (rule.is_Valid) {
                GS::UniString fname = "";
                GetPropertyFullName (definition, fname);
                rule.subguid_paramrawname = fname;
                rule.subguid_rulevalue = fname;
                rule.rule_definitions = definition;
                rule.rule_name = fname;
                if (elemguid != APINULLGuid)
                    rule.elements.Push (elemguid);
            }
            rules.Add (key, rule); // Добавляем в любом случае, чтоб потом дважды не обрабатывать
            if (rule.is_Valid) {
                msg_rep ("Spec", "Find correct rule: " + definition.name, NoError, APINULLGuid);
            } else {
                msg_rep ("Spec", "Rule is not valid: " + definition.name, APIERR_GENERAL, APINULLGuid);
            }
        }
    }

    // --------------------------------------------------------------------
    // Формирование списка параметров для чтения на основе правила
    // Назначение: анализирует группы правила и добавляет имена свойств в списки для чтения/записи
    // Параметры:
    //   rule - правило спецификации (содержит группы с параметрами)
    //   paramToRead - [OUT] словарь параметров для чтения (заполняется)
    //   paramToWrite - [OUT] словарь параметров для записи (заполняется)
    // Алгоритм:
    //   1. Для каждой группы (group) в правиле:
    //      - добавляет флаг (flag_paramrawname)
    //      - добавляет параметры для суммирования (sum_paramrawname)
    //      - добавляет уникальные параметры (unic_paramrawname)
    //      - добавляет параметры для вывода (out_paramrawname)
    //   2. Обрабатывает специальные префиксы: MATERIALNAMEPREFIX (@material:) и FORMULANAMEPREFIX (@formula:)
    //   3. Для каждого элемента в rule.elements добавляет параметры в paramToRead
    //   4. Добавляет параметры для записи в paramToWrite
    // Примечание: параметры материалов и формул обрабатываются особым образом
    // --------------------------------------------------------------------
    // -----------------------------------------------------------------------------
    // На основе правила формирует список параметров, которые нужно прочитать из исходных элементов
    // и список параметров, которые потом будут записаны в новые элементы.
    // -----------------------------------------------------------------------------
    void GetParamToReadFromRule (SpecRule &rule, ParamDictElement &paramToRead, ParamDictValue &paramToWrite) {
        ParamDict params = {}; // Словарь с уникальными параметрами читаемых элементов
        for (const GroupSpec &group : rule.groups) {
            // Для материалов и компонент читаем только 0 группу - остальные одинаковые
            if ((group.fromLibData || group.fromMaterial) && group.n_layer > 0) {
                continue;
            }
            const GS::UniString &rawname = group.flag_paramrawname;
            if (!params.ContainsKey (rawname))
                params.Add (rawname, true);
            if (!group.is_Valid) {
                continue;
            }
            for (const GS::UniString &rawname : group.sum_paramrawname) {
                if (!params.ContainsKey (rawname) && !rawname.IsEqual ("1"))
                    params.Add (rawname, true);
            }
            for (const GS::UniString &rawname : group.unic_paramrawname) {
                if (!params.ContainsKey (rawname))
                    params.Add (rawname, true);
            }
            for (const GS::UniString &rawname : group.out_paramrawname) {
                if (!params.ContainsKey (rawname))
                    params.Add (rawname, true);
            }
        }
        ParamDictValue paramDict = {}; // Словарь параметров для чтения для одного элемента
        for (const auto &cItt : params) {
#if defined(AC_28) || defined(AC_29)
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
        // Добавляем параметры для каждого элемента
        if (!paramDict.IsEmpty ()) {
            for (const API_Guid elemguid : rule.elements) {
                ParamHelpers::AddParamDictValue2ParamDictElement (elemguid, paramDict, paramToRead);
            }
        }
        // Добавляем параметры для записи
        ParamDict paramswrite = {}; // Словарь с уникальными параметрами записываемых элементов
        for (const GS::UniString &rawname : rule.out_sum_paramrawname) {
            if (!paramswrite.ContainsKey (rawname))
                paramswrite.Add (rawname, true);
        }
        for (const GS::UniString &rawname : rule.out_paramrawname) {
            if (!paramswrite.ContainsKey (rawname))
                paramswrite.Add (rawname, true);
        }
        // Добавляем параметры для каждого элемента
        if (!paramswrite.IsEmpty ()) {
            for (const auto &cItt : paramswrite) {
#if defined(AC_28) || defined(AC_29)
                const GS::UniString rawname = cItt.key;
#else
                const GS::UniString rawname = *cItt.key;
#endif
                ParamHelpers::AddValueToParamDictValue (paramToWrite, rawname);
            }
        }
    }

    // --------------------------------------------------------------------
    // Получение значения параметра элемента
    // Назначение: читает значение свойства/GDL-параметра/материала/формулы для элемента
    // Параметры:
    //   elemguid - GUID элемента
    //   rawname - "сырое" имя параметра (с префиксами @property:, @material:, @formula:, @gdl:)
    //   paramToRead - словарь прочитанных параметров (заполняется ElementsRead)
    //   pvalue - [OUT] полученное значение
    //   fromMaterial - флаг: читать из материалов слоев конструкции
    //   n_layer - номер слоя материала
    //   paramCompositeToRead - прочитанные составы конструкции
    //   paramListDataToRead - прочитанные данные ведомостей
    // Алгоритм:
    //   1. Если это libdata (@listdata:) - парсит формулу и вычисляет через ListData
    //   2. Если это формула (@formula:) - парсит и вычисляет через ReadFormula
    //   3. Если это материал (@material:) - читает из paramCompositeToRead по n_layer
    //   4. Иначе - читает через GetParamValueForElements (обычное свойство/GDL)
    // Возвращает: true если значение успешно прочитано
    // Примечание: pvalue.isValid = true только при успешном чтении
    // --------------------------------------------------------------------
    // -----------------------------------------------------------------------------
    // Читает одно значение параметра для конкретного элемента.
    // Поддерживаются обычные свойства, формулы, материалы слоёв и данные из list-data.
    // -----------------------------------------------------------------------------
    bool GetParamValue (const API_Guid &elemguid,
                        const GS::UniString &rawname,
                        const ParamDictElement &paramToRead,
                        ParamValue &pvalue,
                        bool fromMaterial,
                        const GS::Int32 &n_layer,
                        const ParamDictCompositeElement &paramCompositeToRead,
                        const ListData::LibElements &paramListDataToRead) {
        if (hasLibData (rawname)) {
            if (!paramToRead.ContainsKey (elemguid))
                return false;
            const ParamDictValue &p = paramToRead.Get (elemguid);
            if (!p.ContainsKey (rawname))
                return false;
            ParamDictValue paramDict = {}; // Словарь параметров в формуле
            ParamValue formula = p.Get (rawname);
            paramDict.Add (rawname, formula);
            ParamHelpers::ParseParamName (formula.val.uniStringValue, paramDict);
            if (!ListData::AddLibdataToParamValueDict (
                    elemguid, n_layer, paramListDataToRead, formula.val.uniStringValue, paramDict)) {
                pvalue.val.type = API_PropertyStringValueType;
                pvalue.val.uniStringValue = "";
                pvalue.val.doubleValue = 0;
                pvalue.val.rawDoubleValue = 0;
                pvalue.val.intValue = 0;
                pvalue.val.boolValue = false;
                pvalue.isValid = true;
                return true;
            }
            if (!ParamHelpers::ReadFormula (paramDict, true)) {
                pvalue.val.type = API_PropertyStringValueType;
                pvalue.val.uniStringValue = "";
                pvalue.val.doubleValue = 0;
                pvalue.val.rawDoubleValue = 0;
                pvalue.val.intValue = 0;
                pvalue.val.boolValue = false;
                pvalue.isValid = true;
                return true;
            }
            pvalue = paramDict.Get (rawname);
            return pvalue.isValid;
        }
        if (!ParamHelpers::GetParamValueForElements (elemguid, rawname, paramToRead, pvalue))
            return false;
        if (!pvalue.fromMaterial)
            return true;
        if (!paramCompositeToRead.ContainsKey (elemguid)) {
#if defined(TESTING)
            DBprnt ("Spec err", "!paramCompositeToRead.ContainsKey (elemguid)");
#endif
            return false;
        }
        const ParamDictComposite &pc = paramCompositeToRead.Get (elemguid);
        if (!pc.ContainsKey (rawname)) {
#if defined(TESTING)
            DBprnt ("Spec err", "!paramCompositeToRead.ContainsKey (rawname)");
#endif
            return false;
        }
        // Если параметр читался как материал из состава конструкции, берём значение из слоя.
        const ParamComposite &pcelem = pc.Get (rawname);
        if (pcelem.composite.IsEmpty ()) {
#if defined(TESTING)
            DBprnt ("Spec err", "pcelem.composite.IsEmpty()");
#endif
            return false;
        }
        if (pcelem.isValid) {
#if defined(TESTING)
            DBprnt ("Spec err", "pcelem.isValid");
#endif
            return false;
        }
        GS::Int32 max_layers = pcelem.composite.GetSize ();
        if (n_layer >= max_layers) {
            pvalue.val.type = API_PropertyStringValueType;
            pvalue.val.uniStringValue = "";
            pvalue.val.doubleValue = 0;
            pvalue.val.rawDoubleValue = 0;
            pvalue.val.intValue = 0;
            pvalue.val.boolValue = false;
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
        const GS::UniString &val = pcelem.composite[n_layer].val;
        pvalue.val.canCalculate = UniStringToDouble (val, x);
        pvalue.val.uniStringValue = val;
        pvalue.val.doubleValue = x;
        pvalue.val.rawDoubleValue = x;
        pvalue.val.intValue = (GS::Int32)x;
        if (pvalue.val.canCalculate) {
            pvalue.val.boolValue = !is_equal (x, 0);
        } else {
            pvalue.val.boolValue = !pvalue.val.uniStringValue.IsEmpty ();
        }
        pvalue.isValid = true;
        return true;
    }

    // --------------------------------------------------------------------
    // Формирование элементов для создания/модификации на основе правила
    // Назначение: обрабатывает элементы согласно правилу и группирует их
    // Параметры:
    //   rule - правило спецификации (содержит группы, параметры для чтения/записи)
    //   paramToRead - прочитанные параметры элементов
    //   paramCompositeToRead - прочитанные составы конструкции (для материалов)
    //   paramListDataToRead - прочитанные данные ведомостей
    //   elements - [OUT] словарь создаваемых элементов (ключ - уникальная комбинация)
    //   elements_mod - [OUT] словарь модифицируемых элементов (для delete_old)
    //   elements_delete - [OUT] массив удаляемых устаревших элементов
    //   error_element - [OUT] элементы с ошибками чтения параметров
    // Алгоритм:
    //   1. Для каждого элемента в rule.elements:
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
    // -----------------------------------------------------------------------------
    // Формирует набор элементов для создания или обновления на основе одного правила.
    // Здесь важно не только собрать данные, но и правильно сгруппировать одинаковые элементы.
    // -----------------------------------------------------------------------------
    Int32 GetElementsForRule (SpecRule &rule,
                              const ParamDictElement &paramToRead,
                              const ParamDictCompositeElement &paramCompositeToRead,
                              const ListData::LibElements &paramListDataToRead,
                              ElementDict &elements,
                              ElementDict &elements_mod,
                              GS::Array<API_Guid> &elements_delete,
                              UnicGuid &error_element) {
        ParamDict not_found_paramname = {};
        ParamDict not_found_unic = {};
        Int32 n_elements = 0;
        FormatString fstr = FormatStringFunc::ParseFormatString (".2m");
        GS::HashTable<GS::UniString, GS::UniString> out_param = {}; // Ключ - уникальные значения, значение - выходящие
                                                                    // параметры
        for (const API_Guid &elemguid : rule.elements) {
            if (rule.only_visible) {
                if (!ACAPI_Element_Filter (
                        elemguid, APIFilt_OnVisLayer | APIFilt_IsVisibleByRenovation | APIFilt_IsInStructureDisplay))
                    continue;
            }
            for (const GroupSpec &group : rule.groups) {
                Element element = {};
                GS::UniString key = "";
                if (!group.is_Valid)
                    continue;
                // Проверяем значение флага, если он не найден - всё равно добавляем
                bool flag = true;
                if (!group.flag_paramrawname.IsEmpty ()) {
                    ParamValue pvalue = {};
                    if (GetParamValue (elemguid,
                                       group.flag_paramrawname,
                                       paramToRead,
                                       pvalue,
                                       group.fromMaterial,
                                       group.n_layer,
                                       paramCompositeToRead,
                                       paramListDataToRead)) {
                        flag = pvalue.val.boolValue;
                    }
                }
                if (!flag) {
                    continue;
                }
                bool hasunic = true;
                // Принадлежность субэлемента к группе определим по ключу - сцепке значений уникальных параметров
                for (const GS::UniString &rawname : group.unic_paramrawname) {
                    ParamValue pvalue = {};
                    if (!GetParamValue (elemguid,
                                        rawname,
                                        paramToRead,
                                        pvalue,
                                        group.fromMaterial,
                                        group.n_layer,
                                        paramCompositeToRead,
                                        paramListDataToRead)) {
                        hasunic = false;
                        bool is_error = !group.fromMaterial;
                        if (pvalue.fromGDLArray)
                            is_error = pvalue.val.array_row_start == 1;
                        if (is_error && rule.stop_on_error && !not_found_unic.ContainsKey (rawname)) {
                            if (!error_element.ContainsKey (elemguid))
                                error_element.Add (elemguid, true);
                            msg_rep ("Spec", "Unic parameter not valid: " + rawname, APIERR_GENERAL, elemguid);
                            not_found_unic.Add (rawname, true);
                        }
                    }
                    GS::UniString val = pvalue.val.uniStringValue;
                    val.ReplaceAll ("  ", SPACESTRING);
                    val.Trim ();
                    key = key + ATSIGN + val;
                }
                if (!hasunic) {
                    continue;
                }
                for (const GS::UniString &rawname : group.sum_paramrawname) {
                    ParamValue pvalue = {};
                    if (rawname.IsEqual ("1")) {
                        ParamHelpers::ConvertIntToParamValue (pvalue, rawname, 1);
                        element.out_sum_param.Push (pvalue);
                    } else {
                        if (GetParamValue (elemguid,
                                           rawname,
                                           paramToRead,
                                           pvalue,
                                           group.fromMaterial,
                                           group.n_layer,
                                           paramCompositeToRead,
                                           paramListDataToRead)) {
                            element.out_sum_param.Push (pvalue);
                        } else {
                            bool is_error = !group.fromMaterial;
                            if (pvalue.fromGDLArray)
                                is_error = pvalue.val.array_row_start == 1;
                            if (is_error && rule.stop_on_error && !not_found_paramname.ContainsKey ("sum:" + rawname)) {
                                if (!error_element.ContainsKey (elemguid))
                                    error_element.Add (elemguid, true);
                                msg_rep ("Spec", "Sum parameter not valid: " + rawname, APIERR_GENERAL, elemguid);
                                not_found_paramname.Add ("sum:" + rawname, false);
                            }
                        }
                    }
                }
                if (elements.ContainsKey (key)) {
                    Element &exsists_element = elements.Get (key);
                    exsists_element.elements.Push (elemguid);
                    UInt32 nsumm = exsists_element.out_sum_param.GetSize ();
                    if (nsumm != element.out_sum_param.GetSize ()) {
                        nsumm = nsumm < element.out_sum_param.GetSize () ? nsumm : element.out_sum_param.GetSize ();
                    }
                    for (UInt32 j = 0; j < nsumm; j++) {
                        if (exsists_element.out_sum_param[j].isValid && element.out_sum_param[j].isValid)
                            exsists_element.out_sum_param[j].val =
                                exsists_element.out_sum_param[j].val + element.out_sum_param[j].val;
                    }
                } else {
                    GS::UniString key_out = "";
                    for (const GS::UniString &rawname : group.out_paramrawname) {
                        ParamValue pvalue = {};
                        if (GetParamValue (elemguid,
                                           rawname,
                                           paramToRead,
                                           pvalue,
                                           group.fromMaterial,
                                           group.n_layer,
                                           paramCompositeToRead,
                                           paramListDataToRead)) {
                            element.out_param.Push (pvalue);
                            key_out = key_out + ATSIGN + ParamHelpers::ToString (pvalue, fstr);
                        } else {
                            bool is_error = !group.fromMaterial;
                            if (pvalue.fromGDLArray)
                                is_error = pvalue.val.array_row_start == 1;
                            if (!not_found_paramname.ContainsKey (rawname) && rule.stop_on_error && is_error) {
                                if (!error_element.ContainsKey (elemguid))
                                    error_element.Add (elemguid, true);
                                not_found_paramname.Add ("out:" + rawname, false);
                            }
                        }
                    }
                    if (!out_param.ContainsKey (key_out))
                        out_param.Add (key_out, key);
                    if (!element.out_sum_param.IsEmpty () && !element.out_param.IsEmpty () &&
                        element.out_sum_param.GetSize () == rule.out_sum_paramrawname.GetSize () &&
                        element.out_param.GetSize () == rule.out_paramrawname.GetSize ()) {
                        element.out_sum_paramrawname = rule.out_sum_paramrawname;
                        element.out_paramrawname = rule.out_paramrawname;
                        element.subguid_paramrawname = rule.subguid_paramrawname;
                        element.subguid_rulevalue = rule.subguid_rulevalue;
                        element.subguid_rulename = rule.subguid_rulename;
                        element.favorite_name = rule.favorite_name;
                        element.elements.Push (elemguid);
                        elements.Add (key, element);
                        n_elements += 1;
                    } else {
                        if (rule.stop_on_error) {
                            if (!error_element.ContainsKey (elemguid))
                                error_element.Add (elemguid, true);
                            n_elements = 0;
                        }
                    }
                }
            }
        }
        if (rule.stop_on_error) {
            const Int32 iseng = ID_ADDON_STRINGS + isEng ();
            if (!not_found_paramname.IsEmpty ()) {
                GS::UniString SpecNotFoundParametersString =
                    RSGetIndString (iseng, SpecNotFoundParametersId, ACAPI_GetOwnResModule ());
                ACAPI_WriteReport (SpecNotFoundParametersString, true);
                GS::UniString out = "Not found param:";
                for (auto &cIt : not_found_paramname) {
#if defined(AC_28) || defined(AC_29)
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
                GS::UniString SpecNotFoundParametersString =
                    RSGetIndString (iseng, SpecNotFoundParametersId, ACAPI_GetOwnResModule ());
                ACAPI_WriteReport (SpecNotFoundParametersString, true);
                GS::UniString out = "Not found unic:";
                for (auto &cIt : not_found_unic) {
#if defined(AC_28) || defined(AC_29)
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
        if (!rule.delete_old)
            return n_elements;
        UnicGuid guids = {};
        for (const API_Guid &elemguid : rule.exsist_elements) {
            GS::UniString key_out = "";
            bool hasunic = true;
            // Принадлежность субэлемента к группе определим по ключу - сцепке значений уникальных параметров
            for (const GS::UniString &rawname : rule.out_paramrawname) {
                ParamValue pvalue = {};
                if (!GetParamValue (
                        elemguid, rawname, paramToRead, pvalue, false, 0, paramCompositeToRead, paramListDataToRead))
                    hasunic = false;
                key_out = key_out + ATSIGN + ParamHelpers::ToString (pvalue, fstr);
            }
            if (!hasunic) {
                msg_rep ("Spec", "!hasunic " + key_out, NoError, APINULLGuid);
                elements_delete.Push (elemguid);
                guids.Add (elemguid, false);
                continue;
            }
            if (!out_param.ContainsKey (key_out)) {
                msg_rep ("Spec", "out_param.ContainsKey (key_out) " + key_out, NoError, APINULLGuid);
                elements_delete.Push (elemguid);
                guids.Add (elemguid, false);
                continue;
            }
            GS::UniString key = out_param.Get (key_out);
            if (!elements.ContainsKey (key)) {
                msg_rep ("Spec", "!elements.ContainsKey (key) " + key_out, NoError, APINULLGuid);
                elements_delete.Push (elemguid);
                guids.Add (elemguid, false);
                continue;
            }
            // Нашли в создаваемых элементах уже существующую комбинацию значений
            // Такой элемент можно модифицировать
            Element el = elements.Get (key);
            if (elements_mod.ContainsKey (key)) {
                msg_rep ("Spec", "elements_mod.ContainsKey (key) " + key_out, NoError, APINULLGuid);
                elements_delete.Push (elemguid);
                guids.Add (elemguid, false);
                continue;
            }
            bool flag_change = false;
            for (UInt32 i = 0; i < rule.out_paramrawname.GetSize (); i++) {
                GS::UniString rawname = rule.out_paramrawname[i];
                ParamValue pvalue = {};
                ParamValue elvalue = el.out_param[i];
                if (!GetParamValue (
                        elemguid, rawname, paramToRead, pvalue, false, 0, paramCompositeToRead, paramListDataToRead)) {
                    msg_rep ("Spec", "Param not valid: " + rawname, NoError, APINULLGuid);
                    flag_change = true;
                }
                elvalue.val.formatstring = pvalue.val.formatstring;
                ParamHelpers::ConvertByFormatString (elvalue);
                if (elvalue != pvalue) {
                    GS::UniString old_s = "old ";
                    GS::UniString new_s = "";
                    if (pvalue.type != API_PropertyStringValueType) {
                        old_s += FormatStringFunc::NumToString (pvalue.val.doubleValue, pvalue.val.formatstring);
                        new_s += FormatStringFunc::NumToString (elvalue.val.doubleValue, pvalue.val.formatstring);
                    } else {
                        old_s += pvalue.val.uniStringValue;
                        new_s += elvalue.val.uniStringValue;
                    }
                    new_s += " new";
                    msg_rep (
                        "Spec", "Param diff: " + rawname + SPACESTRING + old_s + " <=> " + new_s, NoError, APINULLGuid);
                    flag_change = true;
                }
            }
            for (UInt32 i = 0; i < rule.out_sum_paramrawname.GetSize (); i++) {
                GS::UniString rawname = rule.out_sum_paramrawname[i];
                ParamValue pvalue = {};
                ParamValue elvalue = el.out_sum_param[i];
                if (!GetParamValue (
                        elemguid, rawname, paramToRead, pvalue, false, 0, paramCompositeToRead, paramListDataToRead)) {
                    msg_rep ("Spec", "Param not valid: " + rawname, NoError, APINULLGuid);
                    flag_change = true;
                }
                elvalue.val.formatstring = pvalue.val.formatstring;
                ParamHelpers::ConvertByFormatString (elvalue);
                if (elvalue != pvalue) {
                    GS::UniString old_s = "old ";
                    GS::UniString new_s = "";
                    if (pvalue.type != API_PropertyStringValueType) {
                        old_s += FormatStringFunc::NumToString (pvalue.val.doubleValue, pvalue.val.formatstring);
                        new_s += FormatStringFunc::NumToString (elvalue.val.doubleValue, pvalue.val.formatstring);
                    } else {
                        old_s += pvalue.val.uniStringValue;
                        new_s += elvalue.val.uniStringValue;
                    }
                    msg_rep (
                        "Spec", "Sum diff: " + rawname + SPACESTRING + old_s + " <=> " + new_s, NoError, APINULLGuid);
                    flag_change = true;
                }
            }

            GS::UniString rawname = rule.subguid_paramrawname;
            if (!rawname.IsEmpty ()) {
                ParamValue pvalue = {};
                if (!GetParamValue (
                        elemguid, rawname, paramToRead, pvalue, false, 0, paramCompositeToRead, paramListDataToRead)) {
                    msg_rep ("Spec", "Param not valid: " + rawname, NoError, APINULLGuid);
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
                elements_mod.Add (key, el);
            }
            // Удаляем из списка новых элементов и добавляем в словарь обработанных
            elements.Delete (key);
            guids.Add (elemguid, true);
        }
        // Удаляем все существующие устаревшие элементы
        for (const API_Guid &elemguid : rule.exsist_elements) {
            if (!guids.ContainsKey (elemguid))
                elements_delete.Push (elemguid);
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
    // -----------------------------------------------------------------------------
    // Разбирает строку описания правила и превращает её в структуру SpecRule.
    // Это наиболее сложная часть модуля, потому что здесь нужно распознать критерий, группы и поля записи.
    // -----------------------------------------------------------------------------
    SpecRule GetRuleFromDescription (GS::UniString &description) {
        // Сначала извлекается критерий — имя избранного элемента или другой ключевой текст.
        SpecRule rule = {};
        GS::Array<GS::UniString> partstring = {};
        GS::UniString ldescription = description.ToLowerCase ();
        GS::Array<GS::UniString> local_scratch;
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
            rule.is_Valid = false;
            return rule;
        }
        // Параметры для записи
        // До точки с запятой - уникальные параметры , после - параметры для суммы
        GS::Array<GS::UniString> rulestring_write = {}; // Свойства для записи из группы s()
        UInt32 nrule_write = StringSplt (rulestring_summ[1], SEMICOLON, rulestring_write, true, &local_scratch);
        if (nrule_write != 2) {
            rule.is_Valid = false;
            return rule;
        }
        // Обработка группы s()
        // s (Pn1, Pn2, Pn3; Qn1, Qn2) =>
        // Pn1, Pn2, Pn3 - имена параметров для записи свойств для передачи (P1,P2,P3) part == 0
        // Qn1, Qn2 - имена параметров для записи количества Q1,Q2, part == 1
        for (UInt32 part = 0; part < nrule_write; part++) {
            GS::Array<GS::UniString> rulestring_param = {};
            UInt32 nrule_param = StringSplt (rulestring_write[part], COMMA, rulestring_param, true, &local_scratch);
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
        // Разбивка на группы
        GS::Array<GS::UniString> rulestring_group = {}; // Массив с строками групп
        UInt32 nrule_group = StringSplt (rulestring_summ[0], "g@@", rulestring_group, false, &local_scratch);
        if (nrule_group < 1) {
            rule.is_Valid = false;
            return rule;
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
            UInt32 nrule_read = StringSplt (rulestring_one_group, SEMICOLON, rulestring_read, false, &local_scratch);
            if (nrule_read <= 1) {
                rule.is_Valid = false;
                return rule;
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
                UInt32 nrule_param = StringSplt (rulestring_read[part], COMMA, rulestring_param, false, &local_scratch);
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
                        name = "";
                        continue;
                    }
                    if (name.IsEmpty ())
                        continue;
                    if (name.Contains ("[") && name.Contains ("]")) {
                        GS::UniString n_row_txt = name.GetSubstring ('[', ']', 0);
                        double doubleValue = 0;
                        Int32 n_row = 10;
                        if (UniStringToDouble (n_row_txt, doubleValue)) {
                            n_row = (GS::Int32)doubleValue;
                        }
                        if (min_row == 0)
                            min_row = n_row;
                        min_row = n_row < min_row ? n_row : min_row;
                    }
                    GS::UniString rawName = "";
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
            if (min_row > 0) {
                // создаём группы для параметров с массивами
                for (Int32 jj = 1; jj <= min_row; jj++) {
                    GroupSpec group_add = {};
                    for (GS::UniString rawName : group.out_paramrawname) {
                        if (rawName.Contains ("[") && rawName.Contains ("]")) {
                            GS::UniString n_row_txt = rawName.GetSubstring ('[', ']', 0);
                            rawName.ReplaceFirst ("[" + n_row_txt + "]", EMPTYSTRING);
                            rawName.ReplaceAll (
                                BRACEEND,
                                GS::UniString::Printf ("@arr_%d_%d_%d_%d_%d", jj, jj, 1, 1, ARRAY_UNIC) + BRACEEND);
                        }
                        group_add.out_paramrawname.Push (rawName);
                    }
                    for (GS::UniString rawName : group.unic_paramrawname) {
                        if (rawName.Contains ("[") && rawName.Contains ("]")) {
                            GS::UniString n_row_txt = rawName.GetSubstring ('[', ']', 0);
                            rawName.ReplaceFirst ("[" + n_row_txt + "]", EMPTYSTRING);
                            rawName.ReplaceAll (
                                BRACEEND,
                                GS::UniString::Printf ("@arr_%d_%d_%d_%d_%d", jj, jj, 1, 1, ARRAY_UNIC) + BRACEEND);
                        }
                        group_add.unic_paramrawname.Push (rawName);
                    }
                    GS::UniString rawName = group.flag_paramrawname;
                    if (rawName.Contains ("[") && rawName.Contains ("]")) {
                        GS::UniString n_row_txt = rawName.GetSubstring ('[', ']', 0);
                        rawName.ReplaceFirst ("[" + n_row_txt + "]", EMPTYSTRING);
                        rawName.ReplaceAll (BRACEEND,
                                            GS::UniString::Printf ("@arr_%d_%d_%d_%d_%d", jj, jj, 1, 1, ARRAY_UNIC) +
                                                BRACEEND);
                    }
                    group_add.flag_paramrawname = rawName;
                    for (GS::UniString rawName : group.sum_paramrawname) {
                        if (rawName.Contains ("[") && rawName.Contains ("]")) {
                            GS::UniString n_row_txt = rawName.GetSubstring ('[', ']', 0);
                            rawName.ReplaceFirst ("[" + n_row_txt + "]", EMPTYSTRING);
                            rawName.ReplaceAll (
                                BRACEEND,
                                GS::UniString::Printf ("@arr_%d_%d_%d_%d_%d", jj, jj, 1, 1, ARRAY_UNIC) + BRACEEND);
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
        if (rule.groups.IsEmpty ())
            rule.is_Valid = false;
        if (rule.out_paramrawname.IsEmpty ())
            rule.is_Valid = false;
        if (rule.out_sum_paramrawname.IsEmpty ())
            rule.is_Valid = false;
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
    GSErrCode GetElementForPlaceProperties (const GS::UniString &favorite_name,
                                            GS::HashTable<GS::UniString, GS::UniString> &paramdict) {
        GSErrCode err = NoError;
        API_Element element = {};
        API_ElementMemo memo = {};
#ifndef AC_22
        if (!favorite_name.IsEmpty ()) {
            API_Favorite favorite (favorite_name);
            favorite.memo.New ();
            favorite.properties.New ();
            BNZeroMemory (&favorite.memo.Get (), sizeof (API_ElementMemo));
            err = ACAPI_Favorite_Get (&favorite);
            if (err == NoError) {
                if (favorite.properties.HasValue ()) {
                    for (const auto &property : favorite.properties.Get ()) {
                        GS::UniString fname = "";
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
#ifdef AC_22
        element.header.variationID = APIVarId_Object;
#endif
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
#if defined(AC_27) || defined(AC_28) || defined(AC_29) || defined(AC_26)
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
            GS::UniString fname = "";
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
#ifndef AC_22
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
#ifdef AC_22
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
    // Определение размеров элемента для размещения по сетке
    // Назначение: читает GDL-параметры элемента и определяет шаг сетки при размещении
    // Параметры:
    //   elementt - элемент (не используется, но нужен для совместимости с сигнатурой)
    //   memot - memo-структура элемента (содержит GDL-параметры)
    //   dx - [OUT] шаг по горизонтали
    //   dy - [OUT] шаг по вертикали
    // Алгоритм:
    //   1. Ищет параметры "somestuff_spec_hrow" (высота строки) и "somestuff_spec_bcol" (ширина колонки)
    //   2. Ищет параметр "show_type" (тип отображения)
    //   3. Если show_type=1 - размещение сверху вниз (dx=0, dy=somestuff_spec_hrow)
    //   4. Если show_type=2 или 3 - размещение по сетке (dx=somestuff_spec_bcol, dy=somestuff_spec_hrow)
    //   5. Если нет show_type - пытается использовать параметры "A" и "B" как размеры
    // Возвращает: true, если элементы размещаются сверху вниз (show_type=1 или есть somestuff_spec_hrow)
    // --------------------------------------------------------------------
    bool GetSizePlaceElement (const API_Element &elementt, const API_ElementMemo &memot, double &dx, double &dy) {
        bool flag_find_dx = false;
        bool flag_find_dy = false;
        bool flag_find_type = false;
        double somestuff_spec_hrow = 0;
        double somestuff_spec_bcol = 0;
        Int32 show_type = 0;
        const GSSize nParams = BMGetHandleSize ((GSHandle)memot.params) / sizeof (API_AddParType);
        for (GSIndex ii = 0; ii < nParams; ++ii) {
            API_AddParType &actParam = (*memot.params)[ii];
            GS::UniString name = GS::UniString (actParam.name);
            if (name.IsEqual ("somestuff_spec_hrow")) {
                somestuff_spec_hrow = actParam.value.real;
                flag_find_dx = true;
            }
            if (name.IsEqual ("somestuff_spec_bcol")) {
                somestuff_spec_bcol = actParam.value.real;
                flag_find_dy = true;
            }
            if (name.IsEqual ("show_type")) {
                show_type = (Int32)actParam.value.real;
                flag_find_type = true;
            }
            if (flag_find_dx && flag_find_dy && flag_find_type)
                break;
        }
        if (flag_find_type) {
            if (show_type == 1) {
                dx = 0;
                dy = somestuff_spec_hrow;
                return true;
            }
            if (show_type == 2 || show_type == 3) {
                dx = somestuff_spec_bcol;
                dy = somestuff_spec_hrow;
                return false;
            }
        }
        if (flag_find_dx) {
            dx = 0;
            dy = somestuff_spec_hrow;
            return true;
        }
        for (GSIndex ii = 0; ii < nParams; ++ii) {
            API_AddParType &actParam = (*memot.params)[ii];
            GS::UniString name = GS::UniString (actParam.name);
            if (name.IsEqual ("A") && !flag_find_dx) {
                dx = actParam.value.real;
                flag_find_dx = true;
            }
            if (name.IsEqual ("B") && !flag_find_dy) {
                dy = actParam.value.real;
                flag_find_dy = true;
            }
            if (flag_find_dx && flag_find_dy)
                return false;
        }
        return false;
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
    // Примечание: выполняется внутри ACAPI_CallUndoableCommand
    // --------------------------------------------------------------------
    GSErrCode PlaceElements (GS::Array<ElementDict> &elementstocreate,
                             ParamDictValue &paramToWrite,
                             ParamDictElement &paramOut,
                             Point2D &startpos) {
        GSErrCode err = NoError;
        API_Coord pos = {startpos.x, startpos.y};
        GS::Array<API_Elem_Head> elemsheader = {};
        double dx = 0;
        double dy = 0;
        API_StoryInfo storyInfo = {};
#if defined(AC_27) || defined(AC_28) || defined(AC_29)
        err = ACAPI_ProjectSetting_GetStorySettings (&storyInfo);
#else
        err = ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo, nullptr);
#endif
        short act_st = 0;
        bool find_stor = false;
        if (err == NoError) {
            act_st = storyInfo.actStory;
            find_stor = true;
            BMKillHandle ((GSHandle *)&storyInfo.data);
        }
        ACAPI_CallUndoableCommand ("Create Spec element", [&] () -> GSErrCode {
            int n_elem = 0;
            for (UInt32 i = 0; i < elementstocreate.GetSize (); i++) {
                GS::Array<API_Guid> group = {};
                for (auto &cIt : elementstocreate[i]) {
#if defined(AC_28) || defined(AC_29)
                    Element el = cIt.value;
#else
                Element el = *cIt.value;
#endif
                    API_Element element = {};
                    API_ElementMemo memo = {};
                    err = GetElementForPlace (el.favorite_name, element, memo);
                    if (err != NoError) {
                        ACAPI_DisposeElemMemoHdls (&memo);
                        msg_rep ("Spec", "ACAPI_Element_GetDefaults", err, APINULLGuid);
                        continue;
                    }
                    if (find_stor) {
                        element.header.floorInd = act_st;
                    }
                    // Снимает скрытие и блокировку слоя элемента перед созданием
                    // Проверяет floorInd & 0x8000 и layer.head.flags & 1, при необходимости вызывает
                    // ACAPI_Attribute_Set
                    UnhideUnlockElementLayer (element.header);
                    bool flag_find_row = GetSizePlaceElement (element, memo, dx, dy);
                    // Запись параметров
                    ParamDictValue param = {};
                    if (!el.subguid_paramrawname.IsEmpty ()) {
                        if (paramToWrite.ContainsKey (el.subguid_paramrawname) &&
                            !param.ContainsKey (el.subguid_paramrawname)) {
                            ParamValue paramTo = paramToWrite.Get (el.subguid_paramrawname);
                            GS::UniString instring = APIGuidToString (el.elements[0]);
                            for (UInt32 k = 1; k < el.elements.GetSize (); k++) {
                                instring = instring + SEMICOLON + APIGuid2GSGuid (el.elements[k]).ToUniString ();
                            }
                            paramTo.val.uniStringValue = StringUnic (instring, SEMICOLON);
                            paramTo.isValid = true;
                            paramTo.val.type = API_PropertyStringValueType;
                            param.Add (el.subguid_paramrawname, paramTo);
                        }
                    }
                    if (!el.subguid_rulename.IsEmpty () && !el.subguid_rulevalue.IsEmpty ()) {
                        if (paramToWrite.ContainsKey (el.subguid_rulename) &&
                            !param.ContainsKey (el.subguid_rulename)) {
                            ParamValue paramTo = paramToWrite.Get (el.subguid_rulename);
                            paramTo.val.uniStringValue = el.subguid_rulevalue;
                            paramTo.isValid = true;
                            paramTo.val.type = API_PropertyStringValueType;
                            param.Add (el.subguid_rulename, paramTo);
                        }
                    }
                    // GDL параметры сразу запишем в memo
                    for (UInt32 k = 0; k < el.out_paramrawname.GetSize (); k++) {
                        GS::UniString rawname = el.out_paramrawname[k];
                        if (paramToWrite.ContainsKey (rawname) && !param.ContainsKey (rawname)) {
                            FormatString stringformat;
                            ParamValue paramFrom = el.out_param[k];
                            ParamValue paramTo = paramToWrite.Get (rawname);
                            paramTo.val = paramFrom.val;
                            paramTo.isValid = true;
                            param.Add (rawname, paramTo);
                        }
                    }
                    for (UInt32 k = 0; k < el.out_sum_paramrawname.GetSize (); k++) {
                        GS::UniString rawname = el.out_sum_paramrawname[k];
                        if (paramToWrite.ContainsKey (rawname) && !param.ContainsKey (rawname)) {
                            FormatString stringformat;
                            ParamValue paramFrom = el.out_sum_param[k];
                            ParamValue paramTo = paramToWrite.Get (rawname);
                            paramTo.val = paramFrom.val;
                            if (paramTo.fromPropertyDefinition) {
                                if (paramTo.definition.valueType == API_PropertyStringValueType)
                                    paramTo.val.type = API_PropertyStringValueType;
                            }
                            paramTo.isValid = true;
                            param.Add (rawname, paramTo);
                        }
                    }
                    const GSSize nParams = BMGetHandleSize ((GSHandle)memo.params) / sizeof (API_AddParType);
                    for (GSIndex ii = 0; ii < nParams; ++ii) {
                        API_AddParType &actParam = (*memo.params)[ii];
                        GS::UniString name = GS::UniString (actParam.name);
                        GS::UniString rawname = "";
                        bool flag_find = false;
                        if (actParam.typeMod == API_ParSimple) {
                            rawname = GDLNAMEPREFIX + name.ToLowerCase () + BRACEEND;
                            flag_find = param.ContainsKey (rawname);
                        }
                        if (actParam.typeMod == API_ParSimple && flag_find) {
                            ParamValueData paramfrom = param.Get (rawname).val;
                            switch (actParam.typeID) {
                            case APIParT_ColRGB:
                            case APIParT_Intens:
                            case APIParT_Length:
                            case APIParT_RealNum:
                            case APIParT_Angle:
                                actParam.value.real = paramfrom.doubleValue;
                                break;
                            case APIParT_Boolean:
                                actParam.value.real = paramfrom.doubleValue;
                                break;
                            case APIParT_Integer:
                            case APIParT_PenCol:
                            case APIParT_LineTyp:
                            case APIParT_Mater:
                            case APIParT_FillPat:
                            case APIParT_BuildingMaterial:
                            case APIParT_Profile:
                                actParam.value.real = paramfrom.intValue;
                                break;
                            case APIParT_CString:
                            case APIParT_Title:
                                GS::ucscpy (actParam.value.uStr,
                                            paramfrom.uniStringValue
                                                .ToUStr (0,
                                                         GS::Min (paramfrom.uniStringValue.GetLength (),
                                                                  (USize)API_UAddParStrLen))
                                                .Get ());
                                break;
                            default:
#ifndef AC_22
                            case APIParT_Dictionary:
#endif
                                break;
                            }
                            param.Delete (rawname);
                        }
                    }
                    element.object.pos = pos;
                    err = ACAPI_Element_Create (&element, &memo);
                    if (err == NoError) {
                        elemsheader.Push (element.header);
                        n_elem += 1;
                        if (flag_find_row) {
                            pos.y += dy;
                        } else {
                            if (n_elem % 10 == 0) {
                                pos.x = startpos.x;
                                pos.y += dy;
                            } else {
                                pos.x += dx;
                            }
                        }
                        paramOut.Add (element.header.guid, param);
                        group.Push (element.header.guid);
                    } else {
                        msg_rep ("Spec", "ACAPI_Element_Create", err, APINULLGuid);
                    }
                    ACAPI_DisposeElemMemoHdls (&memo);
                }
                pos.y += 2 * dy;
                if (group.GetSize () > 1) {
                    API_Guid groupGuid = APINULLGuid;
#if defined(AC_27) || defined(AC_28) || defined(AC_29)
                    err = ACAPI_Grouping_CreateGroup (group, &groupGuid);
                    if (err != NoError)
                        err = ACAPI_Grouping_Tool (group, APITool_Group, nullptr);
#else
                err = ACAPI_ElementGroup_Create (group, &groupGuid);
    #ifndef AC_22
                if (err != NoError) err = ACAPI_Element_Tool (group, APITool_Group, nullptr);
    #endif
#endif
                    if (err != NoError)
                        msg_rep ("Spec", "ACAPI_ElementGroup_Create", err, APINULLGuid);
                }
            }
            return NoError;
        });
        for (UInt32 i = 0; i < elemsheader.GetSize (); i++) {
#if defined(AC_27) || defined(AC_28) || defined(AC_29)
            err = ACAPI_LibraryManagement_RunGDLParScript (&elemsheader[i], 0);
#else
            err = ACAPI_Goodies (APIAny_RunGDLParScriptID, &elemsheader[i], 0);
#endif
            if (err != NoError)
                msg_rep ("Spec", "APIAny_RunGDLParScriptID", err, APINULLGuid);
        }
        return NoError;
    }

} // namespace Spec
