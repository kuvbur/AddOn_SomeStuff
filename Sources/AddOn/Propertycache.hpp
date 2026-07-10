//------------ kuvbur 2026 ------------
#pragma once
#if !defined(PROPERTYCACHE_HPP)
    #define PROPERTYCACHE_HPP
    #include "Helpers.hpp"

    #if defined(AC_29)
        #include "ACAPI/MEPAdapter.hpp"
        #include "ACAPI/MEPPhysicalSystem.hpp"
        #include "ACAPI/MEPSystemGroup.hpp"
        #include "ACAPI/MEPUniqueID.hpp"
        #include "ACAPI/Result.hpp"
        #include <ACAPI/MEPEnums.hpp>
    #endif

typedef GS::HashTable<GS::Guid, bool> MEPDict;
typedef GS::HashTable<GS::Guid, MEPDict> MEPDicts;

// -----------------------------------------------------------------------------
// Чтение настроек из информации о проекте
//	Имя свойства: "Addon_Dimenstions"
//	Формат записи: ПЕРО_РАЗМЕРА - КРАТНОСТЬ_ММ, ПЕРО_ТЕКСТА_ИЗМЕНЁННОЕ, ФЛАГ_ИЗМЕНЕНИЯ_СОДЕРЖИМОГО, "ФОРМУЛА", либо
//					"Слой" - КРАТНОСТЬ_ММ, ПЕРО_ТЕКСТА_ИЗМЕНЁННОЕ, ФЛАГ_ИЗМЕНЕНИЯ_СОДЕРЖИМОГО, "ФОРМУЛА"
// -----------------------------------------------------------------------------
bool DimReadPref (DimRules &dimrules, const GS::UniString &autotext, bool &hasLayerNameInDimRules);

// -----------------------------------------------------------------------------
// Обработка текста правила
// -----------------------------------------------------------------------------
bool DimParsePref (const GS::UniString &rawrule, DimRule &dimrule, bool &hasexpression, bool &hasLayerNameInDimRules);

namespace ParamHelpers {

    GS::UniString GetLayerFromCache (const API_AttributeIndex &layerinx);

    bool ReadLibraryFile (const GS::UniString &fileName, GS::Array<GS::Array<GS::UniString>> &data);

    void SetParamValueFromCache (const GS::UniString &rawname, ParamValue &pvalue);

    bool GetParamValueFromCache (const GS::UniString &rawname, ParamValue &pvalue);

    bool isCacheContainsParamValue (const GS::UniString &rawname);

    bool isPropertyDefinitionRead ();

    bool isAttributeRead ();

    bool GetGroupFromCache (const API_Guid &guid, API_PropertyGroup &group);

    GS::UniString GetGDLRawName (const GS::UniString &name);

    #if defined(AC_29)
    bool isMEPRead ();

    bool GetMEPSystemGroup (MEPDicts &mepdict);
    #endif

    bool GetGeoLocationToParamDict (ParamDictValue &propertyParams);

    FormatStringDict GetFormatStringForMeasureType ();

    // --------------------------------------------------------------------
    // Получение списка глобальных переменных о местоположении проекта, солнца
    // --------------------------------------------------------------------
    bool GetPlaceSetsToParamDict (ParamDictValue &propertyParams);

    // --------------------------------------------------------------------
    // Заполнение информации о локальном начале координат
    // --------------------------------------------------------------------
    bool GetLocOriginToParamDict (ParamDictValue &propertyParams);

    // --------------------------------------------------------------------
    // Заполнение информации о проекте
    // --------------------------------------------------------------------
    bool GetAllInfoToParamDict (ParamDictValue &propertyParams);

    // --------------------------------------------------------------------
    // Получение списка аттрибутов (имён слоёв, материалов)
    // --------------------------------------------------------------------
    bool GetAllAttributeToParamDict (ParamDictValue &propertyParams);

    // --------------------------------------------------------------------
    // Получить все доступные свойства в формарте ParamDictValue
    // --------------------------------------------------------------------
    bool GetAllPropertyDefinitionToParamDict (ParamDictValue &propertyParams);

    bool GetArrayPropertyDefinitionToParamDict (ParamDictValue &propertyParams,
                                                GS::Array<API_PropertyDefinition> &definitions);

} // namespace ParamHelpers

// Структура для хранения чистых данных одного слоя в кэше
struct CachedLayer {
    API_AttributeIndex buildingMaterial = {};
    double fillThick = 0;
    short flagBits = 0;
};

struct PropertyCache {
    ParamDictValue property;
    ParamDictValue info;
    ParamDictValue attrib;
    ParamDictValue glob;
    ParamDict file;                                                             // Прочитанные файлы
    GS::HashTable<GS::UniString, GS::Array<GS::Array<GS::UniString>>> filedata; // Данные в файлах
    ClassificationFunc::SystemDict systemdict;
    UnicGuidByGuidString reversesystemdict;
    GS::HashTable<API_Guid, API_PropertyGroup> propertygroups;
    DimRules dimrules; // Правила для размеров, прочитанные из информации о проекте
    bool hasLayerNameInDimRules;
    GS::HashTable<GS::UniString, FormatString> parsedformatstring;
    GS::HashTable<GS::UniString, GS::UniString> gdlparamname;
    FormatStringDict formatstringformeasuretype;
    GS::UniString meterString = "m";
    GS::UniString santimeterString = "cm";
    GS::UniString decimeterString = "dm";
    GS::HashTable<Int32, GS::Array<CachedLayer>> compositeCache;
    GS::UniString buildingMaterialNameString;
    GS::UniString buildingMaterialDescriptionString;
    GS::UniString buildingMaterialDensityString;
    GS::UniString buildingMaterialManufacturerString;
    GS::UniString buildingMaterialCutFillString;
    GS::UniString thicknessString;
    GS::UniString dontspecstr_1;
    GS::UniString dontspecstr_2;
    GS::UniString areastr_1;
    GS::UniString areastr_2;
    GS::UniString volumestr_1;
    GS::UniString volumestr_2;
    GS::UniString lengthstr_1;
    GS::UniString areastr_3;
    GS::UniString areastr_4;
    GS::UniString volumestr_3;
    GS::UniString volumestr_4;
    GS::UniString lengthstr_3;

    Int32 isEng;
    bool isEng_OK;

    GS::HashTable<Int32, ParamDict> unreadedgdlparams;

    API_Tranmat surv_point_tm;

    bool hasDimAutotext;
    bool isGetGeoLocation_OK;  // Успешно прочитан
    bool isGetGeoLocationRead; // Был запрошен

    bool isSurveyPointTransformation_OK;  // Успешно прочитан
    bool isSurveyPointTransformationRead; // Был запрошен

    bool isPlaceSets_OK;  // Успешно прочитан
    bool isPlaceSetsRead; // Был запрошен

    bool isLocOrigin_OK;  // Успешно прочитан
    bool isLocOriginRead; // Был запрошен

    bool isInfo_OK;  // Успешно прочитан
    bool isInfoRead; // Был запрошен

    bool isAttribute_OK;  // Успешно прочитан
    bool isAttributeRead; // Был запрошен

    bool isPropertyDefinition_OK;       // Успешно прочитан
    bool isPropertyDefinitionRead;      // Был запрошен
    bool isPropertyDefinitionRead_full; // Был прочитан полностью

    bool isClassification_OK;  // Успешно прочитан
    bool isClassificationRead; // Был запрошен

    bool isGroupProperty_OK;       // Успешно прочитан
    bool isGroupPropertyRead;      // Был запрошен
    bool isGroupPropertyRead_full; // Был прочитан полностью

    bool isFormatStringFormeasureTypeRead;
    bool isFormatStringFormeasureType_OK;

    #if defined(AC_29)
    MEPDicts mepdict;
    bool isMEP_OK;       // Успешно прочитан
    bool isMEPRead_full; // Был запрошен
    #endif
    PropertyCache () {
    #if defined(TESTING)
        DBprnt ("=PropertyCache= clear");
    #endif
        property.Clear ();
        info.Clear ();
        attrib.Clear ();
        glob.Clear ();
        systemdict.Clear ();
        reversesystemdict.Clear ();
        propertygroups.Clear ();
        dimrules.Clear ();
        unreadedgdlparams.Clear ();
        file.Clear ();
        filedata.Clear ();
        formatstringformeasuretype.Clear ();
        compositeCache.Clear ();
        surv_point_tm = {};
        isEng = 0;
        isEng_OK = false;
        isGetGeoLocation_OK = false;
        isGetGeoLocationRead = false;
        hasDimAutotext = false;
        isSurveyPointTransformation_OK = false;
        isSurveyPointTransformationRead = false;

        isPlaceSets_OK = false;
        isPlaceSetsRead = false;

        isLocOrigin_OK = false;
        isLocOriginRead = false;

        isInfo_OK = false;
        isInfoRead = false;

        isPropertyDefinition_OK = false;
        isPropertyDefinitionRead = false;
        isPropertyDefinitionRead_full = false;

        isAttribute_OK = false;
        isAttributeRead = false;

        isClassification_OK = false;
        isClassificationRead = false;

        isGroupProperty_OK = false;
        isGroupPropertyRead = false;
        isGroupPropertyRead_full = false;

        hasLayerNameInDimRules = false;

    #if defined(AC_29)
        mepdict.Clear ();
        isMEP_OK = false;
        isMEPRead_full = false;
    #endif
    }
    #if defined(AC_29)
    void ReadMEP () {
        isMEPRead_full = true;
        isMEP_OK = ParamHelpers::GetMEPSystemGroup (mepdict);
    }
    #endif

    void ReadisEng () {
        GSErrCode err = NoError;
        API_ServerApplicationInfo AppInfo = {};
    #if defined(AC_27) || defined(AC_28) || defined(AC_29)
        err = ACAPI_AddOnIdentification_Application (&AppInfo);
    #else
        err = ACAPI_Environment (APIEnv_ApplicationID, &AppInfo);
    #endif // AC_27
        if (err != NoError) {
            msg_rep ("PropertyCache", "APIEnv_ApplicationID", err, APINULLGuid);
            return;
        }
        isEng = 0;
        isEng_OK = true;
        msg_rep ("PropertyCache AppInfo.language is", AppInfo.language, err, APINULLGuid);
        if (!AppInfo.language.IsEqual ("RUS"))
            isEng = 1000;
        const Int32 iseng_ = ID_ADDON_STRINGS + isEng;
        meterString = RSGetIndString (iseng_, MeterStringID, ACAPI_GetOwnResModule ());
        santimeterString = RSGetIndString (iseng_, CMeterStringID, ACAPI_GetOwnResModule ());
        decimeterString = RSGetIndString (iseng_, DMeterStringID, ACAPI_GetOwnResModule ());
        const GS::UniString PROP_PREFIX = "@property:";
        buildingMaterialNameString =
            PROP_PREFIX + RSGetIndString (iseng_, BuildingMaterialNameID, ACAPI_GetOwnResModule ());
        buildingMaterialDescriptionString =
            PROP_PREFIX + RSGetIndString (iseng_, BuildingMaterialDescriptionID, ACAPI_GetOwnResModule ());
        buildingMaterialDensityString =
            PROP_PREFIX + RSGetIndString (iseng_, BuildingMaterialDensityID, ACAPI_GetOwnResModule ());
        buildingMaterialManufacturerString =
            PROP_PREFIX + RSGetIndString (iseng_, BuildingMaterialManufacturerID, ACAPI_GetOwnResModule ());
        buildingMaterialCutFillString =
            PROP_PREFIX + RSGetIndString (iseng_, BuildingMaterialCutFillID, ACAPI_GetOwnResModule ());
        thicknessString = PROP_PREFIX + RSGetIndString (iseng_, ThicknessID, ACAPI_GetOwnResModule ());

        areastr_1 = RSGetIndString (iseng_, 53, ACAPI_GetOwnResModule ());
        areastr_2 = RSGetIndString (iseng_, 54, ACAPI_GetOwnResModule ());
        volumestr_1 = RSGetIndString (iseng_, 55, ACAPI_GetOwnResModule ());
        volumestr_2 = RSGetIndString (iseng_, 56, ACAPI_GetOwnResModule ());
        lengthstr_1 = RSGetIndString (iseng_, 57, ACAPI_GetOwnResModule ());
        dontspecstr_1 = RSGetIndString (iseng_, 58, ACAPI_GetOwnResModule ());
        areastr_3 = RSGetIndString (iseng_, 59, ACAPI_GetOwnResModule ());
        areastr_4 = RSGetIndString (iseng_, 60, ACAPI_GetOwnResModule ());
        volumestr_3 = RSGetIndString (iseng_, 61, ACAPI_GetOwnResModule ());
        volumestr_4 = RSGetIndString (iseng_, 62, ACAPI_GetOwnResModule ());
        lengthstr_3 = RSGetIndString (iseng_, 63, ACAPI_GetOwnResModule ());
        dontspecstr_2 = RSGetIndString (iseng_, 64, ACAPI_GetOwnResModule ());
    }

    void Update () {
        clock_t start, finish;
        double duration;
        start = clock ();
    #if defined(TESTING)
        DBprnt ("=PropertyCache= Update start");
    #endif
        glob.Clear ();
        unreadedgdlparams.Clear ();
        ReadGetGeoLocation ();
        ReadSurveyPointTransformation ();
        ReadPlaceSets ();
        ReadLocOrigin ();
        ReadClassification ();
        ReadGroupProperty ();
        ReadPropertyDefinition ();
        ReadAttribute ();
        ReadInfo ();
        ReadFileFromDefinition ();
    #if defined(AC_29)
        ReadMEP ();
    #endif
    #if defined(TESTING)
        DBprnt ("=PropertyCache= Update end");
    #endif
        finish = clock ();
        duration = (double)(finish - start) / CLOCKS_PER_SEC;
        GS::UniString time = GS::UniString::Printf ("Property Cache updated in %.3f s", duration);
        msg_rep ("=PropertyCache=", time, NoError, APINULLGuid);
    }

    void ReadFormatStringForMeasureType () {
    #if defined(TESTING)
        DBprnt ("=PropertyCache= ReadFormatStringForMeasureType");
    #endif
        isFormatStringFormeasureTypeRead = true;
        // Получаем данные об округлении и типе расчёта
        API_CalcUnitPrefs unitPrefs1 = {};
    #if defined(AC_27) || defined(AC_28) || defined(AC_29)
        GSErrCode err = ACAPI_ProjectSetting_GetPreferences (&unitPrefs1, APIPrefs_CalcUnitsID);
    #else
        GSErrCode err = ACAPI_Environment (APIEnv_GetPreferencesID, &unitPrefs1, (void *)APIPrefs_CalcUnitsID);
    #endif
        if (err != NoError) {
            msg_rep ("PropertyCache", "APIEnv_GetPreferencesID_APIPrefs_CalcUnitsID", err, APINULLGuid);
            return;
        }
        API_WorkingUnitPrefs unitPrefs = {};
    #if defined(AC_27) || defined(AC_28) || defined(AC_29)
        err = ACAPI_ProjectSetting_GetPreferences (&unitPrefs, APIPrefs_WorkingUnitsID);
    #else
        err = ACAPI_Environment (APIEnv_GetPreferencesID, &unitPrefs, (void *)APIPrefs_WorkingUnitsID);
    #endif
        if (err != NoError) {
            msg_rep ("PropertyCache", "APIEnv_GetPreferencesID_APIPrefs_WorkingUnitsID", err, APINULLGuid);
            return;
        }
        FormatString fstring = {};
        fstring.needRound = unitPrefs1.useDisplayedValues;

        fstring.n_zero = 2;
        fstring.stringformat = "2";
        formatstringformeasuretype.Add (API_PropertyUndefinedMeasureType, fstring);

        fstring.n_zero = 2;
        fstring.stringformat = "2";
        formatstringformeasuretype.Add (API_PropertyDefaultMeasureType, fstring);

        fstring.n_zero = unitPrefs.areaDecimals;
        fstring.stringformat = GS::UniString::Printf ("0%d", unitPrefs.areaDecimals);
        formatstringformeasuretype.Add (API_PropertyAreaMeasureType, fstring);

        fstring.n_zero = unitPrefs.lenDecimals;
        fstring.stringformat = GS::UniString::Printf ("0%dmm", unitPrefs.lenDecimals);
        formatstringformeasuretype.Add (API_PropertyLengthMeasureType, fstring);

        fstring.n_zero = unitPrefs.volumeDecimals;
        fstring.stringformat = GS::UniString::Printf ("0%d", unitPrefs.volumeDecimals);
        formatstringformeasuretype.Add (API_PropertyVolumeMeasureType, fstring);

        fstring.n_zero = unitPrefs.angleDecimals;
        fstring.stringformat = GS::UniString::Printf ("0%d", unitPrefs.angleDecimals);
        formatstringformeasuretype.Add (API_PropertyAngleMeasureType, fstring);
        isFormatStringFormeasureType_OK = true;
    }

    void ReadGetGeoLocation () {
    #if defined(TESTING)
        DBprnt ("=PropertyCache= ReadGetGeoLocation");
    #endif
        isGetGeoLocationRead = true;
        isGetGeoLocation_OK = ParamHelpers::GetGeoLocationToParamDict (glob);
        if (glob.IsEmpty ())
            isGetGeoLocation_OK = false;
    #if defined(TESTING)
        if (!isGetGeoLocation_OK)
            DBprnt ("=PropertyCache= ReadGetGeoLocation ERROR");
    #endif
    }

    bool AddFile (GS::UniString &fileName) {
        if (file.ContainsKey (fileName))
            return file.Get (fileName);
        GS::Array<GS::Array<GS::UniString>> data = {};
        bool flag = ParamHelpers::ReadLibraryFile (fileName, data);
        file.Add (fileName, flag);
        if (!flag) {
    #if defined(TESTING)
            DBprnt ("=PropertyCache= AddFile read file ERROR " + fileName);
    #endif
            return false;
        }
        if (filedata.ContainsKey (fileName)) {
            filedata.Set (fileName, data);
        } else {
            filedata.Add (fileName, data);
        }
    #if defined(TESTING)
        DBprnt ("=PropertyCache= AddFile file " + fileName);
    #endif
        return true;
    }

    void ReadSurveyPointTransformation () {
    #if defined(AC_22) || defined(AC_23) || defined(AC_24)
        isSurveyPointTransformationRead = true;
        isSurveyPointTransformation_OK = false;
        return;
    #else
        #if defined(TESTING)
        DBprnt ("=PropertyCache= ReadSurveyPointTransformation");
        #endif
        isSurveyPointTransformationRead = true;
        GSErrCode err = NoError;
        #if defined(AC_27) || defined(AC_28) || defined(AC_29)
        err = ACAPI_SurveyPoint_GetSurveyPointTransformation (&surv_point_tm);
        #else
        err = ACAPI_Environment (APIEnv_GetSurveyPointTransformationID, &surv_point_tm);
        #endif
        if (err != NoError) {
            msg_rep (
                "GetSurveyPointTransformationToParamDict", "APIEnv_GetSurveyPointTransformationID", err, APINULLGuid);
            isSurveyPointTransformation_OK = false;
        } else {
            isSurveyPointTransformation_OK = true;
        }
        #if defined(TESTING)
        if (!isSurveyPointTransformation_OK)
            DBprnt ("=PropertyCache= ReadSurveyPointTransformation ERROR");
        #endif
    #endif
    }

    void ReadPlaceSets () {
    #if defined(TESTING)
        DBprnt ("=PropertyCache= ReadPlaceSets");
    #endif
        isPlaceSetsRead = true;
        isPlaceSets_OK = ParamHelpers::GetPlaceSetsToParamDict (glob);
        if (glob.IsEmpty ())
            isPlaceSets_OK = false;
    #if defined(TESTING)
        if (!isPlaceSets_OK)
            DBprnt ("=PropertyCache= ReadPlaceSets ERROR");
    #endif
    }

    void ReadLocOrigin () {
    #if defined(TESTING)
        DBprnt ("=PropertyCache= ReadLocOrigin");
    #endif
        isLocOriginRead = true;
        isLocOrigin_OK = ParamHelpers::GetLocOriginToParamDict (glob);
        if (glob.IsEmpty ())
            isLocOrigin_OK = false;
    #if defined(TESTING)
        if (!isLocOrigin_OK)
            DBprnt ("=PropertyCache= ReadLocOrigin ERROR");
    #endif
    }

    void ReadInfo () {
    #if defined(TESTING)
        DBprnt ("=PropertyCache= ReadInfo");
    #endif
        GS::UniString autotextkey = "{@info:addon_dimension_autotext}";
        info.Clear ();
        isInfoRead = true;
        isInfo_OK = ParamHelpers::GetAllInfoToParamDict (info);
        if (info.IsEmpty ())
            isInfo_OK = false;
        hasDimAutotext = false;
        hasLayerNameInDimRules = false;
        if (isInfo_OK) {
            dimrules.Clear ();
            if (info.ContainsKey (autotextkey)) {
                GS::UniString autotext = info.Get (autotextkey).val.uniStringValue;
                hasDimAutotext = DimReadPref (dimrules, autotext, hasLayerNameInDimRules);
                if (hasDimAutotext) {
                    msg_rep ("=PropertyCache= find dim rule ", autotext, NoError, APINULLGuid);
    #if defined(TESTING)
                    DBprnt ("=PropertyCache= ReadInfo find dim rule " + autotext);
    #endif
                }
            }
        }
    #if defined(TESTING)
        if (!isInfo_OK)
            DBprnt ("=PropertyCache= ReadInfo ERROR");
    #endif
    }

    void ReadAttribute () {
    #if defined(TESTING)
        DBprnt ("=PropertyCache= ReadAttribute");
    #endif
        attrib.Clear ();
        isAttributeRead = true;
        isAttribute_OK = ParamHelpers::GetAllAttributeToParamDict (attrib);
        if (attrib.IsEmpty ())
            isAttribute_OK = false;
    #if defined(TESTING)
        if (!isAttribute_OK)
            DBprnt ("=PropertyCache= ReadAttribute ERROR");
    #endif
    }

    void ReadPropertyDefinition () {
    #if defined(TESTING)
        DBprnt ("=PropertyCache= ReadPropertyDefinition");
    #endif
        property.Clear ();
        isPropertyDefinitionRead = true;
        isPropertyDefinitionRead_full = true;
        isPropertyDefinition_OK = ParamHelpers::GetAllPropertyDefinitionToParamDict (property);
        if (property.IsEmpty ())
            isPropertyDefinition_OK = false;
    #if defined(TESTING)
        if (!isPropertyDefinition_OK)
            DBprnt ("=PropertyCache= ReadPropertyDefinition ERROR");
    #endif
    }

    void ReadFileFromDefinition () {
    #if defined(TESTING)
        DBprnt ("=PropertyCache= ReadFileFromDefinition");
    #endif
        if (!isPropertyDefinition_OK)
            return;
        GS::Array<GS::UniString> loopScratch;
        for (const auto &cIt : property) {
    #if defined(AC_28) || defined(AC_29)
            const ParamValue &param = cIt.value;
    #else
            const ParamValue &param = *cIt.value;
    #endif
            GS::UniString fname = param.definition.description;
            if (fname.IsEmpty ())
                continue;
            if (!fname.Contains (CHARDQUT))
                continue;
            if (!fname.Contains ("Sync_from"))
                continue;
            if (!fname.Contains ("File:"))
                continue;
            GS::Array<GS::UniString> params = {};
            if (StringSplt (fname, COMMA, params, "\"", &loopScratch) > 0) {
                fname = fname.GetSubstring (CHARDQUT, CHARDQUT, 0);
                if (file.ContainsKey (fname))
                    file.Delete (fname);
                if (!fname.Contains (".txt") && !fname.Contains (".csv"))
                    fname.Append (".txt");
                AddFile (fname);
            }
        }
    }

    void ReadClassification () {
    #if defined(TESTING)
        DBprnt ("=PropertyCache= ReadClassification");
    #endif
        systemdict.Clear ();
        reversesystemdict.Clear ();
        isClassificationRead = true;
        if (ClassificationFunc::GetAllClassification (systemdict) == NoError)
            isClassification_OK = true;
        if (systemdict.IsEmpty ())
            isClassification_OK = false;
        if (isClassification_OK) {
            for (ClassificationFunc::SystemDict::PairIterator cIt = systemdict.EnumeratePairs (); cIt != NULL; ++cIt) {
    #if defined(AC_28) || defined(AC_29)
                ClassificationFunc::ClassificationDict &cd = cIt->value;
                GS::UniString systemname = cIt->key;
    #else
                ClassificationFunc::ClassificationDict &cd = *cIt->value;
                GS::UniString systemname = *cIt->key;
    #endif
                if (!cd.ContainsKey ("@system@"))
                    continue;
                API_Guid &systemguid = cd.Get ("@system@").system.guid;
                if (!reversesystemdict.ContainsKey (systemguid)) {
                    UnicGuidString d = {};
                    d.Put (APINULLGuid, systemname);
                    reversesystemdict.Add (systemguid, d);
                } else {
                    continue;
                }
                for (ClassificationFunc::ClassificationDict::PairIterator cItt = cd.EnumeratePairs (); cItt != NULL;
                     ++cItt) {
    #if defined(AC_28) || defined(AC_29)
                    ClassificationFunc::ClassificationValues &cl = cItt->value;
                    GS::UniString classname = cItt->key;
    #else
                    ClassificationFunc::ClassificationValues &cl = *cItt->value;
                    GS::UniString classname = *cItt->key;
    #endif
                    API_Guid &classguid = cl.item.guid;
                    UnicGuidString &d = reversesystemdict.Get (systemguid);
                    d.Put (classguid, classname);
                }
            }
        }
    #if defined(TESTING)
        if (!isClassification_OK)
            DBprnt ("=PropertyCache= ReadClassification ERROR");
    #endif
    }

    void AddPropertyDefinition (GS::Array<API_PropertyDefinition> &definitions) {
        isPropertyDefinitionRead = true;
        ParamHelpers::GetArrayPropertyDefinitionToParamDict (property, definitions);
        if (property.IsEmpty ())
            isPropertyDefinition_OK = false;
    }

    void ReadGroupProperty () {
    #if defined(TESTING)
        DBprnt ("=PropertyCache= ReadGroupProperty");
    #endif
        propertygroups.Clear ();
        isGroupPropertyRead = true;
        isGroupPropertyRead_full = true;
        GS::Array<API_PropertyGroup> groups = {};
        GSErrCode err = ACAPI_Property_GetPropertyGroups (groups);
        if (err != NoError) {
            msg_rep ("ReadGroupProperty", "ACAPI_Property_GetPropertyGroups", err, APINULLGuid);
            return;
        }
        for (const auto &group : groups) {
            propertygroups.Put (group.guid, group);
            isGroupProperty_OK = true;
        }
        if (propertygroups.IsEmpty ())
            isGroupProperty_OK = false;
    #if defined(TESTING)
        if (!isGroupProperty_OK)
            DBprnt ("=PropertyCache= ReadGroupProperty ERROR");
    #endif
    }
};

// -----------------------------------------------------------------------------
// Проверка языка Архикада. Для INT возвращает 1000
// -----------------------------------------------------------------------------
Int32 isEng ();

void AddUnreadGDLParams (const Int32 &libinx, const GS::UniString &rawname);

GS::UniString CountUnreadGDLParams ();

bool IsUnreadGDLParams (const Int32 &libinx, const GS::UniString &rawname);

GS::UniString GetPropertyNameByGUID (const API_Guid &guid);

// -----------------------------------------------------------------------------
// Получить полное имя свойства (включая имя группы)
// -----------------------------------------------------------------------------
GSErrCode GetPropertyFullName (const API_PropertyDefinition &definision, GS::UniString &name);

PropertyCache &GetCache ();

extern PropertyCache &(*PROPERTYCACHE) ();

#endif
