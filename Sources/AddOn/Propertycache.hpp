//------------ kuvbur 2026 ------------
#pragma once
#if !defined (PROPERTYCACHE_HPP)
#define	PROPERTYCACHE_HPP
#include    "Helpers.hpp"


// -----------------------------------------------------------------------------
// Чтение настроек из информации о проекте
//	Имя свойства: "Addon_Dimenstions"
//	Формат записи: ПЕРО_РАЗМЕРА - КРАТНОСТЬ_ММ, ПЕРО_ТЕКСТА_ИЗМЕНЁННОЕ, ФЛАГ_ИЗМЕНЕНИЯ_СОДЕРЖИМОГО, "ФОРМУЛА", либо
//					"Слой" - КРАТНОСТЬ_ММ, ПЕРО_ТЕКСТА_ИЗМЕНЁННОЕ, ФЛАГ_ИЗМЕНЕНИЯ_СОДЕРЖИМОГО, "ФОРМУЛА"
// -----------------------------------------------------------------------------
bool DimReadPref (DimRules& dimrules, const GS::UniString& autotext);

// -----------------------------------------------------------------------------
// Обработка текста правила
// -----------------------------------------------------------------------------
bool DimParsePref (const GS::UniString& rawrule, DimRule& dimrule, bool& hasexpression);

namespace ParamHelpers
{
void SetParamValueFromCache (const GS::UniString& rawname, ParamValue& pvalue);
bool GetParamValueFromCache (const GS::UniString& rawname, ParamValue& pvalue);

bool isPropertyDefinitionRead ();

bool isAttributeRead ();

bool isCacheContainsGroup (const API_Guid& guid);

bool GetGroupFromCache (const API_Guid& guid, API_PropertyGroup& group);

bool isCacheContainsParamValue (const GS::UniString& rawname);

bool isCacheContainsParamValue (const short& inx, const GS::UniString& rawname);

bool GetGeoLocationToParamDict (ParamDictValue& propertyParams);

bool GetSurveyPointTransformationToParamDict (ParamDictValue& propertyParams);

// --------------------------------------------------------------------
// Получение списка глобальных переменных о местоположении проекта, солнца
// --------------------------------------------------------------------
bool GetPlaceSetsToParamDict (ParamDictValue& propertyParams);

// --------------------------------------------------------------------
// Заполнение информации о локальном начале координат
// --------------------------------------------------------------------
bool GetLocOriginToParamDict (ParamDictValue& propertyParams);

// --------------------------------------------------------------------
// Заполнение информации о проекте
// --------------------------------------------------------------------
bool GetAllInfoToParamDict (ParamDictValue& propertyParams);

// --------------------------------------------------------------------
// Получение списка аттрибутов (имён слоёв, материалов)
// --------------------------------------------------------------------
bool GetAllAttributeToParamDict (ParamDictValue& propertyParams);

// --------------------------------------------------------------------
// Получить все доступные свойства в формарте ParamDictValue
// --------------------------------------------------------------------
bool GetAllPropertyDefinitionToParamDict (ParamDictValue& propertyParams);

bool GetArrayPropertyDefinitionToParamDict (ParamDictValue& propertyParams, GS::Array<API_PropertyDefinition>& definitions);

}

struct PropertyCache
{
    ParamDictValue property;
    ParamDictValue info;
    ParamDictValue attrib;
    ParamDictValue glob;
    ClassificationFunc::SystemDict systemdict;
    GS::HashTable <API_Guid, API_PropertyGroup> propertygroups;
    DimRules dimrules; // Правила для размеров, прочитанные из информации о проекте
    bool hasDimAutotext;
    bool isGetGeoLocation_OK; // Успешно прочитан
    bool isGetGeoLocationRead;    // Был запрошен

    bool isSurveyPointTransformation_OK; // Успешно прочитан
    bool isSurveyPointTransformationRead;    // Был запрошен

    bool isPlaceSets_OK; // Успешно прочитан
    bool isPlaceSetsRead;    // Был запрошен

    bool isLocOrigin_OK; // Успешно прочитан
    bool isLocOriginRead;    // Был запрошен

    bool isInfo_OK; // Успешно прочитан
    bool isInfoRead;    // Был запрошен

    bool isAttribute_OK; // Успешно прочитан
    bool isAttributeRead;    // Был запрошен

    bool isPropertyDefinition_OK; // Успешно прочитан
    bool isPropertyDefinitionRead;    // Был запрошен
    bool isPropertyDefinitionRead_full;    // Был прочитан полностью

    bool isClassification_OK; // Успешно прочитан
    bool isClassificationRead;    // Был запрошен


    bool isGroupProperty_OK; // Успешно прочитан
    bool isGroupPropertyRead;    // Был запрошен
    bool isGroupPropertyRead_full;    // Был прочитан полностью

    PropertyCache ()
    {
        #if defined(TESTING)
        DBprnt ("=PropertyCache= clear");
        #endif
        property.Clear ();
        info.Clear ();
        attrib.Clear ();
        glob.Clear ();
        systemdict.Clear ();
        propertygroups.Clear ();
        dimrules.Clear ();
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
    }

    void Update ()
    {
        #if defined(TESTING)
        DBprnt ("=PropertyCache= Update start");
        #endif
        glob.Clear ();
        ReadGetGeoLocation ();
        ReadSurveyPointTransformation ();
        ReadPlaceSets ();
        ReadLocOrigin ();
        ReadClassification ();
        ReadGroupProperty ();
        ReadPropertyDefinition ();
        ReadAttribute ();
        ReadInfo ();
        #if defined(TESTING)
        DBprnt ("=PropertyCache= Update end");
        #endif
    }

    void ReadGetGeoLocation ()
    {
        #if defined(TESTING)
        DBprnt ("=PropertyCache= ReadGetGeoLocation");
        #endif
        isGetGeoLocationRead = true;
        isGetGeoLocation_OK = ParamHelpers::GetGeoLocationToParamDict (glob);
        if (glob.IsEmpty ()) isGetGeoLocation_OK = false;
        #if defined(TESTING)
        if (!isGetGeoLocation_OK) DBprnt ("=PropertyCache= ReadGetGeoLocation ERROR");
        #endif
    }

    void ReadSurveyPointTransformation ()
    {
        #if defined(TESTING)
        DBprnt ("=PropertyCache= ReadSurveyPointTransformation");
        #endif
        isSurveyPointTransformationRead = true;
        isSurveyPointTransformation_OK = ParamHelpers::GetSurveyPointTransformationToParamDict (glob);
        if (glob.IsEmpty ()) isSurveyPointTransformation_OK = false;
        #if defined(TESTING)
        if (!isSurveyPointTransformation_OK) DBprnt ("=PropertyCache= ReadSurveyPointTransformation ERROR");
        #endif
    }

    void ReadPlaceSets ()
    {
        #if defined(TESTING)
        DBprnt ("=PropertyCache= ReadPlaceSets");
        #endif
        isPlaceSetsRead = true;
        isPlaceSets_OK = ParamHelpers::GetPlaceSetsToParamDict (glob);
        if (glob.IsEmpty ()) isPlaceSets_OK = false;
        #if defined(TESTING)
        if (!isPlaceSets_OK) DBprnt ("=PropertyCache= ReadPlaceSets ERROR");
        #endif
    }

    void ReadLocOrigin ()
    {
        #if defined(TESTING)
        DBprnt ("=PropertyCache= ReadLocOrigin");
        #endif
        isLocOriginRead = true;
        isLocOrigin_OK = ParamHelpers::GetLocOriginToParamDict (glob);
        if (glob.IsEmpty ()) isLocOrigin_OK = false;
        #if defined(TESTING)
        if (!isLocOrigin_OK) DBprnt ("=PropertyCache= ReadLocOrigin ERROR");
        #endif
    }

    void ReadInfo ()
    {
        #if defined(TESTING)
        DBprnt ("=PropertyCache= ReadInfo");
        #endif
        info.Clear ();
        isInfoRead = true;
        isInfo_OK = ParamHelpers::GetAllInfoToParamDict (info);
        if (info.IsEmpty ()) isInfo_OK = false;
        hasDimAutotext = false;
        if (isInfo_OK) {
            dimrules.Clear ();
            if (info.ContainsKey ("addon_dimension_autotext")) {
                const GS::UniString autotext = "";
                hasDimAutotext = DimReadPref (dimrules, autotext);
                #if defined(TESTING)
                DBprnt ("=PropertyCache= ReadInfo find dim rule " + autotext);
                #endif
            }
        }
        #if defined(TESTING)
        if (!isInfo_OK) DBprnt ("=PropertyCache= ReadInfo ERROR");
        #endif
    }

    void ReadAttribute ()
    {
        #if defined(TESTING)
        DBprnt ("=PropertyCache= ReadAttribute");
        #endif
        attrib.Clear ();
        isAttributeRead = true;
        isAttribute_OK = ParamHelpers::GetAllAttributeToParamDict (attrib);
        if (attrib.IsEmpty ())  isAttribute_OK = false;
        #if defined(TESTING)
        if (!isAttribute_OK) DBprnt ("=PropertyCache= ReadAttribute ERROR");
        #endif
    }

    void ReadPropertyDefinition ()
    {
        #if defined(TESTING)
        DBprnt ("=PropertyCache= ReadPropertyDefinition");
        #endif
        property.Clear ();
        isPropertyDefinitionRead = true;
        isPropertyDefinitionRead_full = true;
        isPropertyDefinition_OK = ParamHelpers::GetAllPropertyDefinitionToParamDict (property);
        if (property.IsEmpty ()) isPropertyDefinition_OK = false;
        #if defined(TESTING)
        if (!isPropertyDefinition_OK) DBprnt ("=PropertyCache= ReadPropertyDefinition ERROR");
        #endif
    }

    void ReadClassification ()
    {
        #if defined(TESTING)
        DBprnt ("=PropertyCache= ReadClassification");
        #endif
        systemdict.Clear ();
        isClassificationRead = true;
        if (ClassificationFunc::GetAllClassification (systemdict) == NoError) isClassification_OK = true;
        if (systemdict.IsEmpty ()) isClassification_OK = false;
        #if defined(TESTING)
        if (!isClassification_OK) DBprnt ("=PropertyCache= ReadClassification ERROR");
        #endif
    }

    void AddPropertyDefinition (GS::Array<API_PropertyDefinition>& definitions)
    {
        isPropertyDefinitionRead = true;
        ParamHelpers::GetArrayPropertyDefinitionToParamDict (property, definitions);
        if (property.IsEmpty ()) isPropertyDefinition_OK = false;
    }

    void AddGroupProperty (const API_Guid& guid)
    {
        isGroupPropertyRead = true;
        API_PropertyGroup group;
        group.guid = guid;
        GSErrCode err = ACAPI_Property_GetPropertyGroup (group);
        if (err != NoError) {
            msg_rep ("AddGroupProperty", "ACAPI_Property_GetPropertyGroups", err, APINULLGuid);
            return;
        }
        if (!propertygroups.ContainsKey (group.guid)) {
            propertygroups.Add (group.guid, group);
        }
        isGroupProperty_OK = true;
    }

    void ReadGroupProperty ()
    {
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
        for (const auto& group : groups) {
            if (!propertygroups.ContainsKey (group.guid)) {
                propertygroups.Add (group.guid, group);
                isGroupProperty_OK = true;
            }
        }
        if (propertygroups.IsEmpty ()) isGroupProperty_OK = false;
        #if defined(TESTING)
        if (!isGroupProperty_OK) DBprnt ("=PropertyCache= ReadGroupProperty ERROR");
        #endif
    }
};

GS::UniString GetPropertyNameByGUID (const API_Guid& guid);

// -----------------------------------------------------------------------------
// Получить полное имя свойства (включая имя группы)
// -----------------------------------------------------------------------------
GSErrCode GetPropertyFullName (const API_PropertyDefinition& definision, GS::UniString& name);

PropertyCache& GetCache ();

extern PropertyCache& (*PROPERTYCACHE)();

#endif
