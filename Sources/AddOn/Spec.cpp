//------------ kuvbur 2022 ------------
#include "ACAPinc.h"
#include "APIEnvir.h"
#include "Spec.hpp"
#include "Sync.hpp"
#ifdef TESTING
#include "TestFunc.hpp"
#endif
#include "Propertycache.hpp"
namespace Spec
{
// --------------------------------------------------------------------
// Получение правил из свойств элемента по умолчанию
// --------------------------------------------------------------------
bool GetRuleFromDefaultElem (SpecRuleDict& rules, API_DatabaseInfo& homedatabaseInfo, bool& has_elementspec)
{
    #if defined(AC_22)
    return false;
    #else
    GSErrCode error = NoError;
    GS::Array<API_PropertyDefinition> definitions = {};
    #if defined(AC_27) || defined(AC_28) || defined(AC_29) || defined(AC_26)
    error = ACAPI_Element_GetPropertyDefinitionsOfDefaultElem (API_ObjectID, API_PropertyDefinitionFilter_UserDefined, definitions);
    #else
    error = ACAPI_Element_GetPropertyDefinitionsOfDefaultElem (API_ObjectID, APIVarId_Generic, API_PropertyDefinitionFilter_UserDefined, definitions);
    #endif
    if (error != NoError) {
        msg_rep ("Spec", "ACAPI_Element_GetPropertyDefinitionsOfDefaultElem", error, APINULLGuid);
        return false;
    }
    for (UInt32 i = 0; i < definitions.GetSize (); i++) {
        GS::UniString description = definitions[i].description;
        if (!description.IsEmpty ()) {
            if (description.Contains ("Spec_rule") && description.Contains ("{") && description.Contains ("}")) {
                AddRule (definitions[i], APINULLGuid, rules);
            }
        }
    }
    if (rules.IsEmpty ()) return false;
    bool has_element = false;
    ParamDict error_name = {};
    for (GS::HashTable<GS::UniString, SpecRule>::PairIterator cIt = rules.EnumeratePairs (); cIt != NULL; ++cIt) {
        #if defined(AC_28) || defined(AC_29)
        SpecRule& rule = cIt->value;
        #else
        SpecRule& rule = *cIt->value;
        #endif
        if (!rule.is_Valid) continue;
        for (const auto& classificationItemGuid : rule.rule_definitions.availability) {
            GS::Array<API_Guid> elemGuids = {};
            if (ACAPI_Element_GetElementsWithClassification (classificationItemGuid, elemGuids) != NoError) continue;
            SpecFilter (elemGuids, homedatabaseInfo);
            for (const auto& elemGuid : elemGuids) {
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
                if (flagfindspec) {
                    rule.elements.PushNew (elemGuid);
                    has_elementspec = true;
                } else {
                    if (!error_name.ContainsKey (propertyflag.definition.name)) error_name.Add (propertyflag.definition.name, true);
                }
                has_element = true;
            }
        }
    }
    if (has_element && !has_elementspec) {
        msg_rep ("Spec", "All elements is off", APIERR_GENERAL, APINULLGuid);
        GS::UniString SpecRuleNotFoundString = RSGetIndString (ID_ADDON_STRINGS + isEng (), SpecFlagOff, ACAPI_GetOwnResModule ());
        if (!error_name.IsEmpty ()) {
            for (auto& cIt : error_name) {
                #if defined(AC_28) || defined(AC_29)
                GS::UniString s = cIt.key;
                #else
                GS::UniString s = *cIt.key;
                #endif
                SpecRuleNotFoundString.Append ("\n");
                SpecRuleNotFoundString.Append (s);
            }
        }
        ACAPI_WriteReport (SpecRuleNotFoundString, true);
    }
    return has_element;
    #endif
}

GSErrCode SpecAll (const SyncSettings& syncSettings)
{
    GSErrCode err = NoError;
    API_DatabaseInfo homedatabaseInfo = {};
    BNZeroMemory (&homedatabaseInfo, sizeof (API_DatabaseInfo));
    #if defined(AC_27) || defined(AC_28) || defined(AC_29)
    err = ACAPI_Database_GetCurrentDatabase (&homedatabaseInfo);
    #else
    err = ACAPI_Database (APIDb_GetCurrentDatabaseID, &homedatabaseInfo, nullptr);
    #endif
    if (err != NoError) {
        msg_rep ("Spec", "APIDb_GetCurrentDatabaseID", err, APINULLGuid);
    }
    SpecRuleDict rules = {}; bool hasrule = false;
    GS::Array<API_Guid> guidArray = GetSelectedElements (false, false, syncSettings, false, false, false);
    UnicGuid selected_elements = {};
    if (!guidArray.IsEmpty ()) {
        msg_rep ("Spec", "Create spec from selection", NoError, APINULLGuid);
        SpecFilter (guidArray, homedatabaseInfo);
        for (UInt32 i = 0; i < guidArray.GetSize (); i++) {
            if (!selected_elements.ContainsKey (guidArray[i])) selected_elements.Add (guidArray[i], true);
        }
    }
    bool has_elementspec = false;
    if (guidArray.IsEmpty ()) hasrule = GetRuleFromDefaultElem (rules, homedatabaseInfo, has_elementspec);
    if (hasrule) msg_rep ("Spec", "Create spec from default element", NoError, APINULLGuid);
    if (guidArray.IsEmpty () && !hasrule) {
        err = ACAPI_Element_GetElemList (API_ZombieElemID, &guidArray, APIFilt_OnVisLayer | APIFilt_IsVisibleByRenovation | APIFilt_IsInStructureDisplay);
        if (err != NoError) return err;
        if (!guidArray.IsEmpty ()) {
            SpecFilter (guidArray, homedatabaseInfo);
            msg_rep ("Spec", "Create spec from all visible element", NoError, APINULLGuid);
        }
    }
    if (guidArray.IsEmpty () && (!hasrule || !has_elementspec)) return NoError;
    err = SpecArray (syncSettings, guidArray, rules, selected_elements);
    return err;
}

void SpecFilter (API_Guid& elemguid, API_DatabaseInfo& homedatabaseInfo)
{
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
    if (homedatabaseInfo.databaseUnId.elemSetId == APINULLGuid) return;
    API_DatabaseInfo elementdatabaseInfo;
    BNZeroMemory (&elementdatabaseInfo, sizeof (API_DatabaseInfo));
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

void SpecFilter (GS::Array<API_Guid>& guidArray, API_DatabaseInfo& homedatabaseInfo)
{
    GSErrCode err = NoError;
    if (homedatabaseInfo.databaseUnId.elemSetId == APINULLGuid) return;
    API_DatabaseInfo elementdatabaseInfo = {};
    GS::Array<API_Guid> out = {};
    GS::UniString рname = GetDBName (homedatabaseInfo);
    for (UInt32 i = 0; i < guidArray.GetSize (); i++) {
        API_Guid elemguid = guidArray[i];
        API_ElemTypeID elementType = GetElemTypeID (elemguid);
        if (elementType == API_ZombieElemID) continue;
        if (elementType == API_DimensionID) continue;
        if (elementType == API_RadialDimensionID) continue;
        if (elementType == API_LevelDimensionID) continue;
        if (elementType == API_AngleDimensionID) continue;
        if (elementType == API_TextID) continue;
        if (elementType == API_LabelID) continue;
        if (elementType == API_HatchID) continue;
        if (elementType == API_LineID) continue;
        if (elementType == API_PolyLineID) continue;
        if (elementType == API_ArcID) continue;
        if (elementType == API_CircleID) continue;
        if (elementType == API_SplineID) continue;
        if (elementType == API_HotspotID) continue;
        if (elementType == API_CutPlaneID) continue;
        if (elementType == API_CameraID) continue;
        if (elementType == API_CamSetID) continue;
        if (elementType == API_GroupID) continue;
        if (elementType == API_SectElemID) continue;
        if (elementType == API_DrawingID) continue;
        if (elementType == API_PictureID) continue;
        if (elementType == API_DetailID) continue;
        if (elementType == API_ElevationID) continue;
        if (elementType == API_InteriorElevationID) continue;
        if (elementType == API_WorksheetID) continue;
        if (elementType == API_ChangeMarkerID) continue;
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

GSErrCode SpecArray (const SyncSettings& syncSettings, GS::Array<API_Guid>& guidArray, SpecRuleDict& rules, const UnicGuid& selected_elements)
{
    clock_t start, finish;
    double  duration;
    start = clock ();
    GS::UniString funcname = "SpecAll";
    GS::Int32 nPhase = 4;
    GS::UniString subtitle = ""; Int32 maxval = 1; short i = 1;
    ParamDictElement paramToRead = {}; // Словарь с параметрами для чтения
    ParamDictCompositeElement paramCompositeToRead = {}; // Прочитанные составы конструкции
    bool needReturnComposite = true;
    ParamDictValue paramToWrite = {}; // Словарь с параметрами для записи (с нулевым GUID)
    GS::Array<ElementDict> elements_new = {}; // Массив со словарём создаваемых элементов
    GS::Array<ElementDict> elements_mod = {}; // Массив со словарём модифицируемых элементов
    GS::Array<API_Guid> elements_delete = {}; // Массив удаляемых элементов
    ParamDictElement paramOut = {}; // Словарь свойств для записи в расставленные элементы
    GS::Array<API_Guid> guidArraysync = {}; // Список элементов, которые требуется синхронизировать (расставленные элементы)
    UnicGuid error_element = {}; //Элементы с ошибками
    ParamDict error_name = {}; // Список имён, не найденных у избранного
    GS::HashTable<GS::UniString, GS::HashTable<GS::UniString, GS::UniString>> paramdict_favorite = {}; // Словарь с именами параметров и описаниями свойств избранных элементов
    #if defined(AC_27) || defined(AC_28) || defined(AC_29)
    bool showPercent = true;
    ACAPI_ProcessWindow_InitProcessWindow (&funcname, &nPhase);
    #else
    ACAPI_Interface (APIIo_InitProcessWindowID, &funcname, &nPhase);
    #endif
    GSErrCode err = NoError;
    subtitle = GS::UniString::Printf ("Get rule from %d elements", guidArray.GetSize ());
    #if defined(AC_27) || defined(AC_28) || defined(AC_29)
    maxval = 2; ACAPI_ProcessWindow_SetNextProcessPhase (&subtitle, &maxval, &showPercent);
    #else
    i = 2; ACAPI_Interface (APIIo_SetNextProcessPhaseID, &subtitle, &i);
    #endif
    int dummymode = IsDummyModeOn ();
    if (!guidArray.IsEmpty ()) {
        bool flagfindspec = false;
        for (UInt32 i = 0; i < guidArray.GetSize (); i++) {
            err = GetRuleFromElement (guidArray[i], rules);
            if (err == NoError) flagfindspec = true;
        }
        if (!flagfindspec) {
            msg_rep ("Spec", "Rules not found", APIERR_GENERAL, APINULLGuid);
            GS::UniString SpecRuleNotFoundString = RSGetIndString (ID_ADDON_STRINGS + isEng (), SpecRuleNotFoundId, ACAPI_GetOwnResModule ());
            ACAPI_WriteReport (SpecRuleNotFoundString, true);
            #if defined(AC_27) || defined(AC_28) || defined(AC_29)
            ACAPI_ProcessWindow_CloseProcessWindow ();
            #else
            ACAPI_Interface (APIIo_CloseProcessWindowID, nullptr, nullptr);
            #endif
            return APIERR_GENERAL;
        }
    }
    // Теперь пройдём по прочитанным правилам и сформируем список параметров для чтения
    GetParamToReadFromRule (rules, paramToRead, paramToWrite);
    if (paramToRead.IsEmpty ()) {
        msg_rep ("Spec", "Parameters for read not found", APIERR_GENERAL, APINULLGuid);
        GS::UniString SpecRuleReadFoundString = RSGetIndString (ID_ADDON_STRINGS + isEng (), SpecRuleReadFoundId, ACAPI_GetOwnResModule ());
        ACAPI_WriteReport (SpecRuleReadFoundString, true);
        #if defined(AC_27) || defined(AC_28) || defined(AC_29)
        ACAPI_ProcessWindow_CloseProcessWindow ();
        #else
        ACAPI_Interface (APIIo_CloseProcessWindowID, nullptr, nullptr);
        #endif
        return APIERR_GENERAL;
    }
    if (paramToWrite.IsEmpty ()) {
        msg_rep ("Spec", "Parameters for write not found", APIERR_GENERAL, APINULLGuid);
        GS::UniString SpecWriteNotFoundString = RSGetIndString (ID_ADDON_STRINGS + isEng (), SpecWriteNotFoundId, ACAPI_GetOwnResModule ());
        ACAPI_WriteReport (SpecWriteNotFoundString, true);
        #if defined(AC_27) || defined(AC_28) || defined(AC_29)
        ACAPI_ProcessWindow_CloseProcessWindow ();
        #else
        ACAPI_Interface (APIIo_CloseProcessWindowID, nullptr, nullptr);
        #endif
        return APIERR_GENERAL;
    }
    subtitle = GS::UniString::Printf ("Reading parameters from %d elements", paramToRead.GetSize ());
    #if defined(AC_27) || defined(AC_28) || defined(AC_29)
    maxval = 2; ACAPI_ProcessWindow_SetNextProcessPhase (&subtitle, &maxval, &showPercent);
    #else
    i = 2; ACAPI_Interface (APIIo_SetNextProcessPhaseID, &subtitle, &i);
    #endif
    // Читаем свойства избранного
    for (GS::HashTable<GS::UniString, SpecRule>::PairIterator cIt = rules.EnumeratePairs (); cIt != NULL; ++cIt) {
        #if defined(AC_28) || defined(AC_29)
        SpecRule& rule = cIt->value;
        #else
        SpecRule& rule = *cIt->value;
        #endif
        if (!rule.is_Valid) continue;
        if (!paramdict_favorite.ContainsKey (rule.favorite_name)) {
            GS::HashTable<GS::UniString, GS::UniString> paramdict = {};
            err = GetElementForPlaceProperties (rule.favorite_name, paramdict);
            paramdict_favorite.Add (rule.favorite_name, paramdict);
        }
        if (!paramdict_favorite.ContainsKey (rule.favorite_name)) continue;
        for (const auto& rawname : rule.out_paramrawname) {
            if (!paramdict_favorite.Get (rule.favorite_name).ContainsKey (rawname)) {
                rule.is_Valid = false;
                if (!error_name.ContainsKey (rawname)) error_name.Add (rawname, true);
            }
        }
        for (const auto& rawname : rule.out_sum_paramrawname) {
            if (!paramdict_favorite.Get (rule.favorite_name).ContainsKey (rawname)) {
                rule.is_Valid = false;
                if (!error_name.ContainsKey (rawname)) error_name.Add (rawname, true);
            }
        }
        if (!rule.is_Valid) continue;
        bool flag_find = false;
        for (auto& cItt : paramdict_favorite.Get (rule.favorite_name)) {
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
            if (flag_find && !rule.subguid_rulename.IsEmpty ()) break;
        }
        // Поиск существующих объектов
        if (!rule.delete_old) continue;
        if (rule.subguid_rulename.IsEmpty ()) continue;
        if (!flag_find) continue;
        ParamValue subguid_pvalue;
        if (!ParamHelpers::GetParamValueFromCache (rule.subguid_rulename, subguid_pvalue)) {
            #if defined(TESTING)
            DBprnt ("ERROR SpecArray - GetParamValueFromCache subguid_rulename", rule.subguid_rulename);
            #endif
            continue;
        }
        GS::Array<API_Guid> exsist_elements = GetElementByPropertyDescription (subguid_pvalue.definition, rule.subguid_rulevalue.ToLowerCase ());
        if (!selected_elements.IsEmpty ()) {
            for (UInt32 i = 0; i < exsist_elements.GetSize (); i++) {
                if (selected_elements.ContainsKey (exsist_elements[i])) rule.exsist_elements.Push (exsist_elements[i]);
            }
        } else {
            rule.exsist_elements = exsist_elements;
        }
        if (rule.exsist_elements.IsEmpty ()) continue;
        // Собираем список параметрв для чтения у существующих элементов
        ParamDictValue paramDict = {}; // Словарь параметров для чтения для одного элемента
        for (const GS::UniString& rawname : rule.out_sum_paramrawname) {
            if (!paramDict.ContainsKey (rawname)) ParamHelpers::AddValueToParamDictValue (paramDict, rawname);
        }
        for (const GS::UniString& rawname : rule.out_paramrawname) {
            if (!paramDict.ContainsKey (rawname)) ParamHelpers::AddValueToParamDictValue (paramDict, rawname);
        }
        ParamHelpers::AddValueToParamDictValue (paramDict, rule.subguid_paramrawname);
        // Добавляем параметры для каждого элемента
        for (const API_Guid elemguid : rule.exsist_elements) {
            ParamHelpers::AddParamDictValue2ParamDictElement (elemguid, paramDict, paramToRead);
        }
    }
    // Если есть ненайденные параметры - останавливаем работу
    if (!error_name.IsEmpty ()) {
        GS::UniString out = ":\n";
        for (auto& cIt : error_name) {
            #if defined(AC_28) || defined(AC_29)
            GS::UniString s = cIt.key;
            #else
            GS::UniString s = *cIt.key;
            #endif
            s.ReplaceAll ("{@", "");
            s.ReplaceAll ("}", "");
            s.ReplaceAll (":", " : ");
            out.Append (s); out.Append ("\n");
        }
        msg_rep ("Spec", "Can't find parameters in place element: " + out, err, APINULLGuid);
        GS::UniString SpecEmptyListdString = RSGetIndString (ID_ADDON_STRINGS + isEng (), SpecParamPlaceNotFoundId, ACAPI_GetOwnResModule ());
        ACAPI_WriteReport (SpecEmptyListdString + out, true);
        #if defined(AC_27) || defined(AC_28) || defined(AC_29)
        ACAPI_ProcessWindow_CloseProcessWindow ();
        #else
        ACAPI_Interface (APIIo_CloseProcessWindowID, nullptr, nullptr);
        #endif
        return APIERR_GENERAL;
    }
    // Читаем данные из размещённых элементов
    ParamHelpers::ElementsRead (paramToRead, paramCompositeToRead, needReturnComposite);
    //Массив со словарями элементов для создания по правилам
    Int32 n_elements = 0; // Количество создаваемых элементов для отчёта
    bool has_v2 = false;
    for (GS::HashTable<GS::UniString, SpecRule>::PairIterator cIt = rules.EnumeratePairs (); cIt != NULL; ++cIt) {
        #if defined(AC_28) || defined(AC_29)
        SpecRule& rule = cIt->value;
        #else
        SpecRule& rule = *cIt->value;
        #endif
        if (!rule.is_Valid) continue;
        ElementDict elements_n = {}; // Словарь создаваемых элементов для правила
        ElementDict elements_m = {};
        n_elements += GetElementsForRule (rule, paramToRead, paramCompositeToRead, elements_n, elements_m, elements_delete, error_element);
        if (!elements_n.IsEmpty ()) elements_new.Push (elements_n);
        if (!elements_m.IsEmpty ()) elements_mod.Push (elements_m);
        if (rule.delete_old) has_v2 = true;
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
            API_RGBAColor hlColor = { 1, 0.0, 0.0, 1 };
            GS::Array<API_Neig> error_elements = {};
            for (const auto& cIt : error_element) {
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
            if (err == NoError) ACAPI_View_ZoomToSelected ();
            #else
            err = ACAPI_Element_Select (error_elements, true);
            if (err == NoError) ACAPI_Automate (APIDo_ZoomToSelectedID);
            #endif
        } else {
            msg_rep ("Spec", GS::UniString::Printf ("Too many element for highlight - %d", error_element.GetSize ()), err, APINULLGuid);
        }
        #if defined(AC_27) || defined(AC_28) || defined(AC_29)
        ACAPI_ProcessWindow_CloseProcessWindow ();
        #else
        ACAPI_Interface (APIIo_CloseProcessWindowID, nullptr, nullptr);
        #endif
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
        API_RGBAColor hlColor = { 0.8, 0.0, 0.0, 0.5 };
        for (const auto& eldict : elements_mod) {
            for (auto& cIt : eldict) {
                #if defined(AC_28) || defined(AC_29)
                Element el = cIt.value;
                #else
                Element el = *cIt.value;
                #endif
                hlElems.Add (el.exs_guid, hlColor);
                ParamDictValue param = {};
                if (!el.subguid_paramrawname.IsEmpty ()) {
                    if (paramToWrite.ContainsKey (el.subguid_paramrawname) && !param.ContainsKey (el.subguid_paramrawname)) {
                        ParamValue paramTo = paramToWrite.Get (el.subguid_paramrawname);
                        GS::UniString instring = APIGuidToString (el.elements[0]);
                        for (UInt32 k = 1; k < el.elements.GetSize (); k++) {
                            instring = instring + ";" + APIGuid2GSGuid (el.elements[k]).ToUniString ();
                        }
                        paramTo.val.uniStringValue = StringUnic (instring, ";");
                        paramTo.isValid = true;
                        paramTo.val.type = API_PropertyStringValueType;
                        param.Add (el.subguid_paramrawname, paramTo);
                    }
                }
                if (!el.subguid_rulename.IsEmpty () && !el.subguid_rulevalue.IsEmpty ()) {
                    if (paramToWrite.ContainsKey (el.subguid_rulename) && !param.ContainsKey (el.subguid_rulename)) {
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
                            if (paramTo.definition.valueType == API_PropertyStringValueType) paramTo.val.type = API_PropertyStringValueType;
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
    maxval = 3; ACAPI_ProcessWindow_SetNextProcessPhase (&subtitle, &maxval, &showPercent);
    #else
    i = 3; ACAPI_Interface (APIIo_SetNextProcessPhaseID, &subtitle, &i);
    #endif
    if (elements_new.IsEmpty () && elements_mod.IsEmpty () && elements_delete.IsEmpty ()) {
        msg_rep ("Spec", "Elements list empty", NoError, APINULLGuid);
        GS::UniString SpecEmptyListdString = RSGetIndString (ID_ADDON_STRINGS + isEng (), SpecEmptyListdId, ACAPI_GetOwnResModule ());
        if (has_v2) SpecEmptyListdString += "\n" + RSGetIndString (ID_ADDON_STRINGS + isEng (), 67, ACAPI_GetOwnResModule ());
        ACAPI_WriteReport (SpecEmptyListdString, true);
        #if defined(AC_27) || defined(AC_28) || defined(AC_29)
        ACAPI_ProcessWindow_CloseProcessWindow ();
        #else
        ACAPI_Interface (APIIo_CloseProcessWindowID, nullptr, nullptr);
        #endif
        return APIERR_GENERAL;
    }
    Point2D startpos = { 0, 0 };
    finish = clock ();
    duration = (double) (finish - start) / CLOCKS_PER_SEC;
    if (!elements_new.IsEmpty ()) {
        if (!ClickAPoint ("Click the lower corner of the spec elements creation", &startpos)) return APIERR_CANCEL;
        start = clock ();
        PlaceElements (elements_new, paramToWrite, paramOut, startpos);
    } else {
        start = clock ();
    }
    ACAPI_CallUndoableCommand ("Writing properties to created spec elements", [&]() -> GSErrCode {
        bool suspGrp = false;
        #ifndef AC_22
        #if defined(AC_27) || defined(AC_28) || defined(AC_29)
        err = ACAPI_View_IsSuspendGroupOn (&suspGrp);
        if (!suspGrp) ACAPI_Grouping_Tool (elements_delete, APITool_SuspendGroups, nullptr);
        #else
        err = ACAPI_Environment (APIEnv_IsSuspendGroupOnID, &suspGrp);
        if (!suspGrp) ACAPI_Element_Tool (elements_delete, APITool_SuspendGroups, nullptr);
        #endif
        if (!elements_delete.IsEmpty ()) {
            err = ACAPI_Element_Delete (elements_delete);
            msg_rep ("Spec", GS::UniString::Printf ("Removed %d obsolete spec elements", elements_delete.GetSize ()), err, APINULLGuid);
        }
        #endif // !AC_22
        ParamHelpers::ElementsWrite (paramOut);
        return NoError;
    });

    #if defined(AC_27) || defined(AC_28) || defined(AC_29)
    ACAPI_ProcessWindow_CloseProcessWindow ();
    #else
    ACAPI_Interface (APIIo_CloseProcessWindowID, nullptr, nullptr);
    #endif
    if (has_v2) {
        GS::UniString msg = "";
        if (!elements_delete.IsEmpty ()) {
            msg += RSGetIndString (ID_ADDON_STRINGS + isEng (), 70, ACAPI_GetOwnResModule ()) + GS::UniString::Printf (" %d \n", elements_delete.GetSize ());
        }
        if (!elements_new.IsEmpty ()) {
            UInt32 n_elem = 0;
            for (UInt32 i = 0; i < elements_new.GetSize (); i++) {
                n_elem += elements_new[i].GetSize ();
            }
            msg += RSGetIndString (ID_ADDON_STRINGS + isEng (), 68, ACAPI_GetOwnResModule ()) + GS::UniString::Printf (" %d \n", n_elem);
        }
        if (!elements_mod.IsEmpty ()) {
            UInt32 n_elem = 0;
            for (UInt32 i = 0; i < elements_mod.GetSize (); i++) {
                n_elem += elements_mod[i].GetSize ();
            }
            msg += RSGetIndString (ID_ADDON_STRINGS + isEng (), 69, ACAPI_GetOwnResModule ()) + GS::UniString::Printf (" %d \n", n_elem);
        }
        if (msg.IsEmpty ()) msg = RSGetIndString (ID_ADDON_STRINGS + isEng (), 67, ACAPI_GetOwnResModule ());
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
    duration += (double) (finish - start) / CLOCKS_PER_SEC;
    GS::UniString time = GS::UniString::Printf (" %.3f s", duration);
    GS::UniString intString = GS::UniString::Printf ("Qty elements - %d ", guidArray.GetSize ()) + GS::UniString::Printf ("wrtite to - %d", n_elements) + time;
    msg_rep (funcname, intString, err, APINULLGuid);
    return err;
}

// --------------------------------------------------------------------
// Проверяет значение свойства с правилом и формрует правила
// --------------------------------------------------------------------
GSErrCode GetRuleFromElement (const API_Guid& elemguid, SpecRuleDict& rules)
{
    GSErrCode err = NoError;
    GS::Array<API_PropertyDefinition> definitions = {};
    err = ACAPI_Element_GetPropertyDefinitions (elemguid, API_PropertyDefinitionFilter_UserDefined, definitions);
    if (err != NoError || definitions.IsEmpty ()) return err;
    for (UInt32 i = 0; i < definitions.GetSize (); i++) {
        GS::UniString description = definitions[i].description;
        if (description.IsEmpty ()) continue;
        if (!description.Contains ("Spec_rule")) continue;
        if (!description.Contains ("{")) continue;
        if (!description.Contains ("}")) continue;
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
        if (flagfindspec) AddRule (definitions[i], elemguid, rules);
    }
    return NoError;
}

void AddRule (const API_PropertyDefinition& definition, const API_Guid& elemguid, SpecRuleDict& rules)
{
    // Чистим описание
    GS::UniString description = definition.description;
    description.ReplaceAll ("\n", "");
    description.ReplaceAll ("\r", "");
    description.ReplaceAll ("\t", "");
    description.ReplaceAll ("  ", " ");
    description.ReplaceAll ("  ", " ");
    description.ReplaceAll ("  ", " ");
    description.ReplaceAll ("  ", " ");
    description.ReplaceAll ("  ", " ");
    description.ReplaceAll ("  ", " ");
    description.ReplaceAll (" {", "{");
    description.ReplaceAll ("{ ", "{");
    description.ReplaceAll (" }", "}");
    description.ReplaceAll ("} ", "}");
    description.ReplaceAll (" ;", ";");
    description.ReplaceAll ("; ", ";");
    description.ReplaceAll ("gm (", "gm(");
    description.ReplaceAll (" gm(", "gm(");
    description.ReplaceAll ("g (", "g(");
    description.ReplaceAll ("s (", "s(");
    description.ReplaceAll (" g(", "g(");
    description.ReplaceAll (" s(", "s(");
    description.ReplaceAll ("g(", "g@@");
    description.ReplaceAll ("s(", "s@@");
    description.ReplaceAll (")s", "@@s");
    description.ReplaceAll (")g", "@@g");
    description.ReplaceAll ("))", ")@@");
    description.ReplaceAll ("gm(", "g@@Material_all@");
    GS::Array<GS::UniString> partstring = {};
    if (StringSplt (description, "}", partstring, "pec_rule") > 0) {
        description = partstring[0] + "}";
    }
    GS::UniString key = description.GetSubstring ('{', '}', 0);
    if (rules.ContainsKey (key)) {
        if (rules.Get (key).is_Valid && elemguid != APINULLGuid) rules.Get (key).elements.Push (elemguid);
    } else {
        // Добавление группы и элемента
        SpecRule rule = GetRuleFromDescription (description);
        if (rule.is_Valid) {
            GS::UniString fname;
            GetPropertyFullName (definition, fname);
            rule.subguid_paramrawname = fname;
            rule.subguid_rulevalue = fname;
            rule.rule_definitions = definition;
            if (elemguid != APINULLGuid) rule.elements.Push (elemguid);
        }
        rules.Add (key, rule); //Добавляем в любом случае, чтоб потом дважды не обрабатывать
        if (rule.is_Valid) {
            msg_rep ("Spec", "Find correct rule: " + definition.name, NoError, APINULLGuid);
        } else {
            msg_rep ("Spec", "Rule is not valid: " + definition.name, APIERR_GENERAL, APINULLGuid);
        }
    }
}

// --------------------------------------------------------------------
// Выбирает из параметров групп имена свойств для дальнейшего чтения
// --------------------------------------------------------------------
void GetParamToReadFromRule (SpecRuleDict& rules, ParamDictElement& paramToRead, ParamDictValue& paramToWrite)
{
    for (GS::HashTable<GS::UniString, SpecRule>::PairIterator cIt = rules.EnumeratePairs (); cIt != NULL; ++cIt) {
        #if defined(AC_28) || defined(AC_29)
        SpecRule& rule = cIt->value;
        #else
        SpecRule& rule = *cIt->value;
        #endif
        // Читаем только параметры групп
        if (!rule.is_Valid) {
            continue;
        }
        ParamDict params = {}; // Словарь с уникальными параметрами читаемых элементов
        for (const GroupSpec& group : rule.groups) {
            GS::UniString rawname = group.flag_paramrawname;
            if (!params.ContainsKey (rawname)) params.Add (rawname, true);
            if (!group.is_Valid) {
                continue;
            }
            for (const GS::UniString& rawname : group.sum_paramrawname) {
                if (!params.ContainsKey (rawname) && !rawname.IsEqual ("1")) params.Add (rawname, true);
            }
            for (const GS::UniString& rawname : group.unic_paramrawname) {
                if (!params.ContainsKey (rawname)) params.Add (rawname, true);
            }
            for (const GS::UniString& rawname : group.out_paramrawname) {
                if (!params.ContainsKey (rawname)) params.Add (rawname, true);
            }
        }
        ParamDictValue paramDict = {}; // Словарь параметров для чтения для одного элемента
        for (auto& cItt : params) {
            #if defined(AC_28) || defined(AC_29)
            GS::UniString rawname = cItt.key;
            #else
            GS::UniString rawname = *cItt.key;
            #endif
            if (rawname.Contains ("{@material:layers_auto,all;")) {
                GS::UniString t = rawname;
                t.ReplaceAll ("{@material:layers_auto,all;", "{");
                t = t.GetSubstring ('{', '}', 0);
                ParamHelpers::ParseParamNameMaterial (t, paramDict);
                ParamHelpers::AddValueToParamDictValue (paramDict, "@property:buildingmaterialproperties/some_stuff_th");
                ParamHelpers::AddValueToParamDictValue (paramDict, "@property:buildingmaterialproperties/some_stuff_units");
                ParamHelpers::AddValueToParamDictValue (paramDict, "@property:buildingmaterialproperties/some_stuff_kzap");
                for (UInt32 inx = 0; inx < 20; inx++) {
                    ParamHelpers::AddValueToParamDictValue (paramDict, "@property:sync_name" + GS::UniString::Printf ("%d", inx));
                }
                ParamValue param = {};
                param.rawName = rawname;
                param.val.uniStringValue = t;
                param.composite_pen = -2;
                param.fromQuantity = true;
                param.fromMaterial = true;
                ParamHelpers::AddParamValue2ParamDict (APINULLGuid, param, paramDict);
            } else {
                ParamHelpers::AddValueToParamDictValue (paramDict, rawname);
            }
        }
        // Добавляем параметры для каждого элемента
        for (const API_Guid elemguid : rule.elements) {
            ParamHelpers::AddParamDictValue2ParamDictElement (elemguid, paramDict, paramToRead);
        }
        // Добавляем параметры для записи
        ParamDict paramswrite = {}; // Словарь с уникальными параметрами записываемых элементов
        for (const GS::UniString& rawname : rule.out_sum_paramrawname) {
            if (!paramswrite.ContainsKey (rawname)) paramswrite.Add (rawname, true);
        }
        for (const GS::UniString& rawname : rule.out_paramrawname) {
            if (!paramswrite.ContainsKey (rawname)) paramswrite.Add (rawname, true);
        }
        // Добавляем параметры для каждого элемента
        for (auto& cItt : paramswrite) {
            #if defined(AC_28) || defined(AC_29)
            GS::UniString rawname = cItt.key;
            #else
            GS::UniString rawname = *cItt.key;
            #endif
            ParamHelpers::AddValueToParamDictValue (paramToWrite, rawname);
        }
    }
}

bool GetParamValue (const API_Guid& elemguid, const GS::UniString& rawname, const ParamDictElement& paramToRead, ParamValue& pvalue, bool fromMaterial, GS::Int32 n_layer, const  ParamDictCompositeElement& paramCompositeToRead)
{
    if (!ParamHelpers::GetParamValueForElements (elemguid, rawname, paramToRead, pvalue)) return false;
    if (!pvalue.fromMaterial) return true;
    if (!paramCompositeToRead.ContainsKey (elemguid)) return false;
    if (!paramCompositeToRead.Get (elemguid).ContainsKey (rawname)) return false;
    ParamComposite pcelem = paramCompositeToRead.Get (elemguid).Get (rawname);
    GS::Int32 max_layers = pcelem.composite.GetSize ();
    if (n_layer >= max_layers) {
        pvalue.val.uniStringValue = "";
        pvalue.val.doubleValue = 0;
        pvalue.val.rawDoubleValue = 0;
        pvalue.val.intValue = 0;
        pvalue.val.boolValue = false;
        return true;
    }
    double x = 0;
    pvalue.val.uniStringValue = pcelem.composite[n_layer].val;
    pvalue.val.canCalculate = UniStringToDouble (pcelem.templatestring, x);
    pvalue.val.doubleValue = x;
    pvalue.val.rawDoubleValue = x;
    pvalue.val.intValue = (GS::Int32) x;
    if (pvalue.val.canCalculate) {
        pvalue.val.boolValue = !is_equal (x, 0);
    } else {
        pvalue.val.boolValue = !pvalue.val.uniStringValue.IsEmpty ();
    }
    return true;
}

Int32 GetElementsForRule (SpecRule& rule, const ParamDictElement& paramToRead, const ParamDictCompositeElement& paramCompositeToRead, ElementDict& elements, ElementDict& elements_mod, GS::Array<API_Guid>& elements_delete, UnicGuid& error_element)
{
    ParamDict not_found_paramname = {};
    ParamDict not_found_unic = {};
    Int32 n_elements = 0;
    FormatString fstr = FormatStringFunc::ParseFormatString (".2m");
    GS::HashTable<GS::UniString, GS::UniString> out_param = {}; // Ключ - уникальные значения, значение - выходящие параметры
    for (const API_Guid& elemguid : rule.elements) {
        for (const GroupSpec& group : rule.groups) {
            Element element = {};
            GS::UniString key = "";
            if (!group.is_Valid) continue;
            if (rule.only_visible) {
                if (!ACAPI_Element_Filter (elemguid, APIFilt_OnVisLayer)) continue;
                if (!ACAPI_Element_Filter (elemguid, APIFilt_IsVisibleByRenovation)) continue;
                if (!ACAPI_Element_Filter (elemguid, APIFilt_IsInStructureDisplay)) continue;
            }
            // Проверяем значение флага, если он не найден - всё равно добавляем
            bool flag = true;
            if (!group.flag_paramrawname.IsEmpty ()) {
                ParamValue pvalue = {};
                if (GetParamValue (elemguid, group.flag_paramrawname, paramToRead, pvalue, group.fromMaterial, group.n_layer, paramCompositeToRead)) {
                    flag = pvalue.val.boolValue;
                }
            }
            if (!flag) {
                continue;
            }
            bool hasunic = true;
            // Принадлежность субэлемента к группе определим по ключу - сцепке значений уникальных параметров
            for (const GS::UniString& rawname : group.unic_paramrawname) {
                ParamValue pvalue = {};
                if (!GetParamValue (elemguid, rawname, paramToRead, pvalue, group.fromMaterial, group.n_layer, paramCompositeToRead)) {
                    hasunic = false;
                    bool is_error = !group.fromMaterial;
                    if (pvalue.fromGDLArray) is_error = pvalue.val.array_row_start == 1;
                    if (is_error && rule.stop_on_error && !not_found_unic.ContainsKey (rawname)) {
                        if (!error_element.ContainsKey (elemguid)) error_element.Add (elemguid, true);
                        msg_rep ("Spec", "Unic parameter not valid: " + rawname, APIERR_GENERAL, elemguid);
                        not_found_unic.Add (rawname, true);
                    }
                }
                GS::UniString val = pvalue.val.uniStringValue;
                val.ReplaceAll ("  ", " ");
                val.Trim ();
                key = key + "@" + val;
            }
            if (!hasunic) {
                continue;
            }
            for (const GS::UniString& rawname : group.sum_paramrawname) {
                ParamValue pvalue = {};
                if (rawname.IsEqual ("1")) {
                    ParamHelpers::ConvertIntToParamValue (pvalue, rawname, 1);
                    element.out_sum_param.Push (pvalue);
                } else {
                    if (GetParamValue (elemguid, rawname, paramToRead, pvalue, group.fromMaterial, group.n_layer, paramCompositeToRead)) {
                        element.out_sum_param.Push (pvalue);
                    } else {
                        bool is_error = !group.fromMaterial;
                        if (pvalue.fromGDLArray) is_error = pvalue.val.array_row_start == 1;
                        if (is_error && rule.stop_on_error && !not_found_paramname.ContainsKey ("sum:" + rawname)) {
                            if (!error_element.ContainsKey (elemguid)) error_element.Add (elemguid, true);
                            msg_rep ("Spec", "Sum parameter not valid: " + rawname, APIERR_GENERAL, elemguid);
                            not_found_paramname.Add ("sum:" + rawname, false);
                        }
                    }
                }
            }
            if (elements.ContainsKey (key)) {
                Element& exsists_element = elements.Get (key);
                exsists_element.elements.Push (elemguid);
                UInt32 nsumm = exsists_element.out_sum_param.GetSize ();
                if (nsumm != element.out_sum_param.GetSize ()) {
                    nsumm = nsumm < element.out_sum_param.GetSize () ? nsumm : element.out_sum_param.GetSize ();
                }
                for (UInt32 j = 0; j < nsumm; j++) {
                    if (exsists_element.out_sum_param[j].isValid && element.out_sum_param[j].isValid)
                        exsists_element.out_sum_param[j].val = exsists_element.out_sum_param[j].val + element.out_sum_param[j].val;
                }
            } else {
                GS::UniString key_out = "";
                for (const GS::UniString& rawname : group.out_paramrawname) {
                    ParamValue pvalue = {};
                    if (GetParamValue (elemguid, rawname, paramToRead, pvalue, group.fromMaterial, group.n_layer, paramCompositeToRead)) {
                        element.out_param.Push (pvalue);
                        key_out = key_out + "@" + ParamHelpers::ToString (pvalue, fstr);
                    } else {
                        bool is_error = !group.fromMaterial;
                        if (pvalue.fromGDLArray) is_error = pvalue.val.array_row_start == 1;
                        if (!not_found_paramname.ContainsKey (rawname) && rule.stop_on_error && is_error) {
                            if (!error_element.ContainsKey (elemguid)) error_element.Add (elemguid, true);
                            not_found_paramname.Add ("out:" + rawname, false);
                        }
                    }
                }
                if (!out_param.ContainsKey (key_out)) out_param.Add (key_out, key);
                if (!element.out_sum_param.IsEmpty () && !element.out_param.IsEmpty () &&
                    element.out_sum_param.GetSize () == rule.out_sum_paramrawname.GetSize () &&
                    element.out_param.GetSize () == rule.out_paramrawname.GetSize ()
                    ) {
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
                        if (!error_element.ContainsKey (elemguid)) error_element.Add (elemguid, true);
                        n_elements = 0;
                    }
                }
            }
        }
    }
    if (rule.stop_on_error) {
        if (!not_found_paramname.IsEmpty ()) {
            GS::UniString SpecNotFoundParametersString = RSGetIndString (ID_ADDON_STRINGS + isEng (), SpecNotFoundParametersId, ACAPI_GetOwnResModule ());
            ACAPI_WriteReport (SpecNotFoundParametersString, true);
            GS::UniString out = "Not found param:";
            for (auto& cIt : not_found_paramname) {
                #if defined(AC_28) || defined(AC_29)
                GS::UniString s = cIt.key;
                #else
                GS::UniString s = *cIt.key;
                #endif
                out.Append (s); out.Append ("; ");
            }
            out.ReplaceAll ("{@", "");
            out.ReplaceAll ("}", "");
            out.ReplaceAll (":", " : ");
            out.ReplaceAll ("%", "");
            out.ReplaceAll ("nosyncname", "");
            msg_rep ("Spec", out, NoError, APINULLGuid);
        }
        if (!not_found_unic.IsEmpty ()) {
            GS::UniString SpecNotFoundParametersString = RSGetIndString (ID_ADDON_STRINGS + isEng (), SpecNotFoundParametersId, ACAPI_GetOwnResModule ());
            ACAPI_WriteReport (SpecNotFoundParametersString, true);
            GS::UniString out = "Not found unic:";
            for (auto& cIt : not_found_unic) {
                #if defined(AC_28) || defined(AC_29)
                GS::UniString s = cIt.key;
                #else
                GS::UniString s = *cIt.key;
                #endif
                out.Append (s); out.Append ("; ");
            }
            out.ReplaceAll ("{@", "");
            out.ReplaceAll ("}", "");
            out.ReplaceAll (":", " : ");
            out.ReplaceAll ("%", "");
            out.ReplaceAll ("nosyncname", "");
            msg_rep ("Spec", out, NoError, APINULLGuid);
        }
        if (!not_found_paramname.IsEmpty () || !not_found_unic.IsEmpty ()) {
            n_elements = 0;
            elements.Clear ();
            return 0;
        }
    }
    if (!rule.delete_old) return n_elements;
    UnicGuid guids = {};
    for (const API_Guid& elemguid : rule.exsist_elements) {
        GS::UniString key_out = "";
        bool hasunic = true;
        // Принадлежность субэлемента к группе определим по ключу - сцепке значений уникальных параметров
        for (const GS::UniString& rawname : rule.out_paramrawname) {
            ParamValue pvalue = {};
            if (!GetParamValue (elemguid, rawname, paramToRead, pvalue, false, 0, paramCompositeToRead)) hasunic = false;
            key_out = key_out + "@" + ParamHelpers::ToString (pvalue, fstr);
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
            if (!GetParamValue (elemguid, rawname, paramToRead, pvalue, false, 0, paramCompositeToRead)) {
                msg_rep ("Spec", "Param not valid: " + rawname, NoError, APINULLGuid);
                flag_change = true;
            }
            elvalue.val.formatstring = pvalue.val.formatstring;
            ParamHelpers::ConvertByFormatString (elvalue);
            if (elvalue != pvalue) {
                GS::UniString old_s = "old "; GS::UniString new_s = "";
                if (pvalue.type != API_PropertyStringValueType) {
                    old_s += FormatStringFunc::NumToString (pvalue.val.doubleValue, pvalue.val.formatstring);
                    new_s += FormatStringFunc::NumToString (elvalue.val.doubleValue, pvalue.val.formatstring);
                } else {
                    old_s += pvalue.val.uniStringValue;
                    new_s += elvalue.val.uniStringValue;
                }
                new_s += " new";
                msg_rep ("Spec", "Param diff: " + rawname + " " + old_s + " <=> " + new_s, NoError, APINULLGuid);
                flag_change = true;
            }
        }
        for (UInt32 i = 0; i < rule.out_sum_paramrawname.GetSize (); i++) {
            GS::UniString rawname = rule.out_sum_paramrawname[i];
            ParamValue pvalue = {};
            ParamValue elvalue = el.out_sum_param[i];
            if (!GetParamValue (elemguid, rawname, paramToRead, pvalue, false, 0, paramCompositeToRead)) {
                msg_rep ("Spec", "Param not valid: " + rawname, NoError, APINULLGuid);
                flag_change = true;
            }
            elvalue.val.formatstring = pvalue.val.formatstring;
            ParamHelpers::ConvertByFormatString (elvalue);
            if (elvalue != pvalue) {
                GS::UniString old_s = "old "; GS::UniString new_s = "";
                if (pvalue.type != API_PropertyStringValueType) {
                    old_s += FormatStringFunc::NumToString (pvalue.val.doubleValue, pvalue.val.formatstring);
                    new_s += FormatStringFunc::NumToString (elvalue.val.doubleValue, pvalue.val.formatstring);
                } else {
                    old_s += pvalue.val.uniStringValue;
                    new_s += elvalue.val.uniStringValue;
                }
                msg_rep ("Spec", "Sum diff: " + rawname + " " + old_s + " <=> " + new_s, NoError, APINULLGuid);
                flag_change = true;
            }
        }

        GS::UniString rawname = rule.subguid_paramrawname;
        if (!rawname.IsEmpty ()) {
            ParamValue pvalue = {};
            if (!GetParamValue (elemguid, rawname, paramToRead, pvalue, false, 0, paramCompositeToRead)) {
                msg_rep ("Spec", "Param not valid: " + rawname, NoError, APINULLGuid);
                flag_change = true;
            }
            GS::UniString instring = APIGuidToString (el.elements[0]);
            for (UInt32 k = 1; k < el.elements.GetSize (); k++) {
                instring = instring + ";" + APIGuid2GSGuid (el.elements[k]).ToUniString ();
            }
            instring = StringUnic (instring, ";");
            if (!instring.IsEqual (pvalue.val.uniStringValue)) {
                msg_rep ("Spec", "SubGUID diff: " + rawname + " " + instring + " <=> " + pvalue.val.uniStringValue, NoError, APINULLGuid);
                flag_change = true;
            }
        }
        //Если нашли изменения - добавим в список модифицированных
        if (flag_change) {
            el.exs_guid = elemguid;
            elements_mod.Add (key, el);
        }
        // Удаляем из списка новых элементов и добавляем в словарь обработанных
        elements.Delete (key);
        guids.Add (elemguid, true);
    }
    // Удаляем все существующие устаревшие элементы
    for (const API_Guid& elemguid : rule.exsist_elements) {
        if (!guids.ContainsKey (elemguid)) elements_delete.Push (elemguid);
    }
    n_elements = 0;
    n_elements += elements_delete.GetSize ();
    n_elements += elements_mod.GetSize ();
    n_elements += elements.GetSize ();
    return n_elements;
}

// --------------------------------------------------------------------
// Разбивает строку на части. Будем постепенно заменять на пустоту обработанные части
// Критерий - значение, по которому будет сгруппированы элементы
// g(P1, P2, P3; F; Q1, Q2) - P1...P3 параметры, уникальные для вложенного элемента,
//
//                            F - флаг включения группы 1/0. Если не найден - всегда 1
//                            Q1...Q2 параметры или значения количества, будут просуммированы. Если их нет - запишем 1 для суммы.
// s(Pn1, Pn2, Pn3; Qn1, Qn2) - Pn1...Pn3 параметры размещаемых объектов,
//                              Qn1...Qn2 параметры для записи количества
// Spec_rule {КРИТЕРИЙ ;g(P1, P2, P3; Q1, Q2) g(P4, P5, P6; Q3, Q4) s(Pn1, Pn2, Pn3; Qn1, Qn2)}
// --------------------------------------------------------------------
SpecRule GetRuleFromDescription (GS::UniString& description)
{
    // Получаем критерий
    SpecRule rule = {};
    GS::Array<GS::UniString> partstring = {};
    if (description.Contains ("pec_rule_v2")) {
        rule.delete_old = true;
    }
    if (description.Contains ("pec_rule_v3")) {
        rule.delete_old = true;
        rule.stop_on_error = false;
        rule.only_visible = false;
    }
    if (StringSplt (description, "}", partstring, "pec_rule") > 0) {
        description = partstring[0] + "}";
    }
    GS::UniString criteria = description.GetSubstring ('{', ';', 0);
    description = description.GetSubstring ('{', '}', 0);
    description.ReplaceAll (criteria + ";", "");
    description.Trim (')');
    description.Trim ();
    if (criteria.Contains ("\"")) criteria = criteria.GetSubstring ('"', '"', 0);
    criteria.Trim ();
    rule.favorite_name = criteria;
    GS::Array<GS::UniString> paramss = {};
    // Разбивка на группы и итог
    GS::Array<GS::UniString> rulestring_summ = {};
    if (StringSplt (description, "s@@", rulestring_summ) < 2) {
        rule.is_Valid = false;
        return rule;
    }
    // Параметры для записи
    // До точки с запятой - уникальные параметры , после - параметры для суммы
    GS::Array<GS::UniString> rulestring_write = {};
    UInt32 nrule_write = StringSplt (rulestring_summ[1], ";", rulestring_write);
    if (nrule_write != 2) {
        rule.is_Valid = false;
        return rule;
    }
    for (UInt32 part = 0; part < nrule_write; part++) {
        GS::Array<GS::UniString> rulestring_param = {};
        UInt32 nrule_param = StringSplt (rulestring_write[part], ",", rulestring_param);
        if (nrule_param > 0) {
            for (UInt32 i = 0; i < nrule_param; i++) {
                FormatString formatstring;
                GS::UniString name = rulestring_param[i];
                name.Trim ('%');
                name.Trim ('@');
                name.Trim ('{');
                name.Trim ('}');
                name.Trim ();
                if (!name.IsEmpty ()) {
                    if (name.Contains ("[") && name.Contains ("]")) {
                        GS::UniString n_row_txt = name.GetSubstring ('[', ']', 0);
                        name.ReplaceFirst ("[" + n_row_txt + "]", "");
                    }
                    FormatString formatstring;
                    GS::UniString rawName = ParamHelpers::NameToRawName (name, formatstring);
                    if (part == 0) rule.out_paramrawname.Push (rawName);
                    if (part == 1) rule.out_sum_paramrawname.Push (rawName);
                }
            }
        }
    }
    // Параметры для чтения
    // До точки с запятой - уникальные параметры , после - параметры для суммы
    GS::Array<GS::UniString> rulestring_group = {};
    UInt32 nrule_group = StringSplt (rulestring_summ[0], "g@@", rulestring_group);
    if (nrule_group < 1) {
        rule.is_Valid = false;
        return rule;
    }
    for (UInt32 igroup = 0; igroup < nrule_group; igroup++) {
        GS::Array<GS::UniString> rulestring_read = {};
        bool fromMaterial = false;
        if (rulestring_group[igroup].Contains ("Material_all@")) {
            rulestring_group[igroup].ReplaceAll ("Material_all@", "");
            fromMaterial = true;
        }
        UInt32 nrule_read = StringSplt (rulestring_group[igroup], ";", rulestring_read);
        if (nrule_read <= 1) {
            rule.is_Valid = false;
            return rule;
        }
        GroupSpec group = {};
        group.fromMaterial = fromMaterial;
        Int32 min_row = 0;
        for (UInt32 part = 0; part < nrule_read; part++) {
            GS::Array<GS::UniString> rulestring_param = {};
            UInt32 nrule_param = StringSplt (rulestring_read[part], ",", rulestring_param);
            if (nrule_param > 0) {
                for (UInt32 i = 0; i < nrule_param; i++) {
                    GS::UniString name = rulestring_param[i];
                    name.Trim ('@');
                    if (!fromMaterial) name.Trim ('%');
                    name.Trim ('{');
                    name.Trim ('}');
                    name.Trim ();
                    if (part == 1 && name.IsEqual ("-")) {
                        name = "";
                    } else {
                        if (!name.IsEmpty ()) {
                            if (name.Contains ("[") && name.Contains ("]")) {
                                GS::UniString n_row_txt = name.GetSubstring ('[', ']', 0);
                                double doubleValue = 0;
                                Int32 n_row = 10;
                                if (UniStringToDouble (n_row_txt, doubleValue)) {
                                    n_row = (GS::Int32) doubleValue;
                                }
                                if (min_row == 0) min_row = n_row;
                                min_row = n_row < min_row ? n_row : min_row;
                            }
                            GS::UniString rawName = "";
                            if (fromMaterial && name.Contains ("%")) {
                                rawName = "{@material:layers_auto,all;" + name + "}";
                            } else {
                                FormatString formatstring;
                                rawName = ParamHelpers::NameToRawName (name, formatstring);
                            }
                            if (part == 0) group.unic_paramrawname.Push (rawName);
                            if (part == 1) group.out_paramrawname.Push (rawName);
                            if (part == 2) group.flag_paramrawname = rawName;
                            if (part == 3) group.sum_paramrawname.Push (rawName);
                        }
                    }
                }
            }
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
                        rawName.ReplaceFirst ("[" + n_row_txt + "]", "");
                        rawName.ReplaceAll ("}", GS::UniString::Printf ("@arr_%d_%d_%d_%d_%d", jj, jj, 1, 1, ARRAY_UNIC) + "}");
                    }
                    group_add.out_paramrawname.Push (rawName);
                }
                for (GS::UniString rawName : group.unic_paramrawname) {
                    if (rawName.Contains ("[") && rawName.Contains ("]")) {
                        GS::UniString n_row_txt = rawName.GetSubstring ('[', ']', 0);
                        rawName.ReplaceFirst ("[" + n_row_txt + "]", "");
                        rawName.ReplaceAll ("}", GS::UniString::Printf ("@arr_%d_%d_%d_%d_%d", jj, jj, 1, 1, ARRAY_UNIC) + "}");
                    }
                    group_add.unic_paramrawname.Push (rawName);
                }
                GS::UniString rawName = group.flag_paramrawname;
                if (rawName.Contains ("[") && rawName.Contains ("]")) {
                    GS::UniString n_row_txt = rawName.GetSubstring ('[', ']', 0);
                    rawName.ReplaceFirst ("[" + n_row_txt + "]", "");
                    rawName.ReplaceAll ("}", GS::UniString::Printf ("@arr_%d_%d_%d_%d_%d", jj, jj, 1, 1, ARRAY_UNIC) + "}");
                }
                group_add.flag_paramrawname = rawName;
                for (GS::UniString rawName : group.sum_paramrawname) {
                    if (rawName.Contains ("[") && rawName.Contains ("]")) {
                        GS::UniString n_row_txt = rawName.GetSubstring ('[', ']', 0);
                        rawName.ReplaceFirst ("[" + n_row_txt + "]", "");
                        rawName.ReplaceAll ("}", GS::UniString::Printf ("@arr_%d_%d_%d_%d_%d", jj, jj, 1, 1, ARRAY_UNIC) + "}");
                    }
                    group_add.sum_paramrawname.Push (rawName);
                }
                rule.groups.Push (group_add);
            }
        } else {
            if (group.out_paramrawname.GetSize () != rule.out_paramrawname.GetSize ()) {
                group.is_Valid = false;
                msg_rep ("Spec", "group.out_paramrawname.GetSize () != rule.out_paramrawname.GetSize ()", APIERR_BADINDEX, APINULLGuid);
            }
            if (group.sum_paramrawname.GetSize () != rule.out_sum_paramrawname.GetSize ()) {
                group.is_Valid = false;
                msg_rep ("Spec", "group.sum_paramrawname.GetSize () != rule.out_sum_paramrawname.GetSize ()", APIERR_BADINDEX, APINULLGuid);
            }
            if (group.is_Valid) {
                if (group.fromMaterial) {
                    // Для материалов определим макимальное количество слоёв
                    for (UInt32 n_layer = 0; n_layer < 30; n_layer++) {
                        group.n_layer = n_layer;
                        rule.groups.PushNew (group);
                    }
                } else {
                    rule.groups.Push (group);
                }
            }
        }
    }
    if (rule.groups.IsEmpty ()) rule.is_Valid = false;
    if (rule.out_paramrawname.IsEmpty ()) rule.is_Valid = false;
    if (rule.out_sum_paramrawname.IsEmpty ()) rule.is_Valid = false;
    return rule;
}


GSErrCode GetElementForPlaceProperties (const GS::UniString& favorite_name, GS::HashTable<GS::UniString, GS::UniString>& paramdict)
{
    GSErrCode err = NoError;
    API_Element element = {}; BNZeroMemory (&element, sizeof (API_Element));
    API_ElementMemo memo = {}; BNZeroMemory (&memo, sizeof (API_ElementMemo));
    #ifndef AC_22
    if (!favorite_name.IsEmpty ()) {
        API_Favorite favorite (favorite_name);
        favorite.memo.New ();
        favorite.properties.New ();
        BNZeroMemory (&favorite.memo.Get (), sizeof (API_ElementMemo));
        err = ACAPI_Favorite_Get (&favorite);
        if (err == NoError) {
            if (favorite.properties.HasValue ()) {
                for (const auto& property : favorite.properties.Get ()) {
                    GS::UniString fname = ""; GS::UniString rawName = PROPERTYNAMEPREFIX;
                    GetPropertyFullName (property.definition, fname);
                    rawName.Append (fname.ToLowerCase ());
                    rawName.Append ("}");
                    if (!paramdict.ContainsKey (rawName)) paramdict.Add (rawName, property.definition.description.ToLowerCase ());
                }
            }
            if (favorite.memo.HasValue ()) {
                if (favorite.memo.Get ().params != nullptr) {
                    const GSSize nParams = BMGetHandleSize ((GSHandle) favorite.memo.Get ().params) / sizeof (API_AddParType);
                    for (GSIndex ii = 0; ii < nParams; ++ii) {
                        API_AddParType& actParam = (*favorite.memo.Get ().params)[ii];
                        GS::UniString fname = GS::UniString (actParam.name);
                        GS::UniString rawName = GDLNAMEPREFIX;
                        rawName.Append (fname.ToLowerCase ());
                        rawName.Append ("}");
                        if (!paramdict.ContainsKey (rawName)) paramdict.Add (rawName, "");
                    }
                }
            }
            ACAPI_DisposeElemMemoHdls (&favorite.memo.Get ());
            return err;
        } else {
            msg_rep ("Spec", "Can't find favorite with name: " + favorite_name + " . Contiune with default objec.", err, APINULLGuid);
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
        const GSSize nParams = BMGetHandleSize ((GSHandle) memo.params) / sizeof (API_AddParType);
        for (GSIndex ii = 0; ii < nParams; ++ii) {
            API_AddParType& actParam = (*memo.params)[ii];
            GS::UniString fname = GS::UniString (actParam.name);
            GS::UniString rawName = GDLNAMEPREFIX;
            rawName.Append (fname.ToLowerCase ());
            rawName.Append ("}");
            if (!paramdict.ContainsKey (rawName)) paramdict.Add (rawName, "");
        }
    }
    ACAPI_DisposeElemMemoHdls (&memo);
    GS::Array<API_PropertyDefinition> definitions = {};
    #if defined(AC_27) || defined(AC_28) || defined(AC_29) || defined(AC_26)
    err = ACAPI_Element_GetPropertyDefinitionsOfDefaultElem (element.header.type, API_PropertyDefinitionFilter_UserDefined, definitions);
    #else
    err = ACAPI_Element_GetPropertyDefinitionsOfDefaultElem (element.header.typeID, element.header.variationID, API_PropertyDefinitionFilter_UserDefined, definitions);
    #endif
    if (err != NoError) {
        msg_rep ("Spec", "ACAPI_Element_GetPropertyDefinitionsOfDefaultElem", err, APINULLGuid);
        return err;
    }
    for (const auto& definition : definitions) {
        GS::UniString fname = ""; GS::UniString rawName = PROPERTYNAMEPREFIX;
        GetPropertyFullName (definition, fname);
        rawName.Append (fname.ToLowerCase ());
        rawName.Append ("}");
        if (!paramdict.ContainsKey (rawName)) paramdict.Add (rawName, definition.description.ToLowerCase ());
    }
    return err;
}

GSErrCode GetElementForPlace (const GS::UniString& favorite_name, API_Element& element, API_ElementMemo& memo)
{
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
// Получение размеров элемента для размещения по сетке
// Возвращает истину, если был найден параметр somestuff_spec_hrow - в этом случае элементы размещаются сверху вниз
// --------------------------------------------------------------------
bool GetSizePlaceElement (const API_Element& elementt, const API_ElementMemo& memot, double& dx, double& dy)
{
    bool flag_find_dx = false; bool flag_find_dy = false; bool flag_find_type = false;
    double somestuff_spec_hrow = 0;
    double somestuff_spec_bcol = 0;
    Int32 show_type = 0;
    const GSSize nParams = BMGetHandleSize ((GSHandle) memot.params) / sizeof (API_AddParType);
    for (GSIndex ii = 0; ii < nParams; ++ii) {
        API_AddParType& actParam = (*memot.params)[ii];
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
            show_type = (Int32) actParam.value.real;
            flag_find_type = true;
        }
        if (flag_find_dx && flag_find_dy && flag_find_type) break;
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
        API_AddParType& actParam = (*memot.params)[ii];
        GS::UniString name = GS::UniString (actParam.name);
        if (name.IsEqual ("A") && !flag_find_dx) {
            dx = actParam.value.real;
            flag_find_dx = true;
        }
        if (name.IsEqual ("B") && !flag_find_dy) {
            dy = actParam.value.real;
            flag_find_dy = true;
        }
        if (flag_find_dx && flag_find_dy) return false;
    }
    return false;
}

GSErrCode PlaceElements (GS::Array<ElementDict>& elementstocreate, ParamDictValue& paramToWrite, ParamDictElement& paramOut, Point2D& startpos)
{
    GSErrCode err = NoError;
    API_Coord pos = { startpos.x, startpos.y };
    GS::Array <API_Elem_Head> elemsheader = {};
    double dx = 0; double dy = 0;
    API_StoryInfo storyInfo = {};
    #if defined(AC_27) || defined(AC_28) || defined(AC_29)
    err = ACAPI_ProjectSetting_GetStorySettings (&storyInfo);
    #else
    err = ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo, nullptr);
    #endif
    short act_st = 0; bool find_stor = false;
    if (err == NoError) {
        act_st = storyInfo.actStory;
        find_stor = true;
        BMKillHandle ((GSHandle*) &storyInfo.data);
    }
    ACAPI_CallUndoableCommand ("Create Spec element", [&]() -> GSErrCode {
        int n_elem = 0;
        for (UInt32 i = 0; i < elementstocreate.GetSize (); i++) {
            GS::Array<API_Guid> group = {};
            for (auto& cIt : elementstocreate[i]) {
                #if defined(AC_28) || defined(AC_29)
                Element el = cIt.value;
                #else
                Element el = *cIt.value;
                #endif
                API_Element element = {}; BNZeroMemory (&element, sizeof (API_Element));
                API_ElementMemo memo = {}; BNZeroMemory (&memo, sizeof (API_ElementMemo));
                err = GetElementForPlace (el.favorite_name, element, memo);
                if (err != NoError) {
                    ACAPI_DisposeElemMemoHdls (&memo);
                    msg_rep ("Spec", "ACAPI_Element_GetDefaults", err, APINULLGuid);
                    continue;
                }
                if (find_stor) {
                    element.header.floorInd = act_st;
                }
                UnhideUnlockElementLayer (element.header);
                bool flag_find_row = GetSizePlaceElement (element, memo, dx, dy);
                // Запись параметров
                ParamDictValue param = {};
                if (!el.subguid_paramrawname.IsEmpty ()) {
                    if (paramToWrite.ContainsKey (el.subguid_paramrawname) && !param.ContainsKey (el.subguid_paramrawname)) {
                        ParamValue paramTo = paramToWrite.Get (el.subguid_paramrawname);
                        GS::UniString instring = APIGuidToString (el.elements[0]);
                        for (UInt32 k = 1; k < el.elements.GetSize (); k++) {
                            instring = instring + ";" + APIGuid2GSGuid (el.elements[k]).ToUniString ();
                        }
                        paramTo.val.uniStringValue = StringUnic (instring, ";");
                        paramTo.isValid = true;
                        paramTo.val.type = API_PropertyStringValueType;
                        param.Add (el.subguid_paramrawname, paramTo);
                    }
                }
                if (!el.subguid_rulename.IsEmpty () && !el.subguid_rulevalue.IsEmpty ()) {
                    if (paramToWrite.ContainsKey (el.subguid_rulename) && !param.ContainsKey (el.subguid_rulename)) {
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
                            if (paramTo.definition.valueType == API_PropertyStringValueType) paramTo.val.type = API_PropertyStringValueType;
                        }
                        paramTo.isValid = true;
                        param.Add (rawname, paramTo);
                    }
                }
                const GSSize nParams = BMGetHandleSize ((GSHandle) memo.params) / sizeof (API_AddParType);
                for (GSIndex ii = 0; ii < nParams; ++ii) {
                    API_AddParType& actParam = (*memo.params)[ii];
                    GS::UniString name = GS::UniString (actParam.name);
                    GS::UniString rawname = "";
                    bool flag_find = false;
                    if (actParam.typeMod == API_ParSimple) {
                        rawname = GDLNAMEPREFIX + name.ToLowerCase () + "}";
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
                                GS::ucscpy (actParam.value.uStr, paramfrom.uniStringValue.ToUStr (0, GS::Min (paramfrom.uniStringValue.GetLength (), (USize) API_UAddParStrLen)).Get ());
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
                if (err != NoError) err = ACAPI_Grouping_Tool (group, APITool_Group, nullptr);
                #else
                err = ACAPI_ElementGroup_Create (group, &groupGuid);
                #ifndef AC_22
                if (err != NoError) err = ACAPI_Element_Tool (group, APITool_Group, nullptr);
                #endif
                #endif
                if (err != NoError) msg_rep ("Spec", "ACAPI_ElementGroup_Create", err, APINULLGuid);
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
        if (err != NoError) msg_rep ("Spec", "APIAny_RunGDLParScriptID", err, APINULLGuid);
    }
    return NoError;
}

}
