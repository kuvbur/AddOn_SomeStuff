//------------ kuvbur 2026 ------------
#pragma once
#if !defined (PROPERTYCACHE_HPP)
#define	PROPERTYCACHE_HPP
#include    "Helpers.hpp"

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
        DBprnt ("!!! PropertyCache clear");
        #endif
        property.Clear ();
        info.Clear ();
        attrib.Clear ();
        glob.Clear ();
        systemdict.Clear ();
        propertygroups.Clear ();

        isGetGeoLocation_OK = false;
        isGetGeoLocationRead = false;

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
    }

    void ReadGetGeoLocation ()
    {
        #if defined(TESTING)
        DBprnt ("!!! ReadGetGeoLocation");
        #endif
        isGetGeoLocationRead = true;
        isGetGeoLocation_OK = ParamHelpers::GetGeoLocationToParamDict (glob);
        if (glob.IsEmpty ()) isGetGeoLocation_OK = false;
    }

    void ReadSurveyPointTransformation ()
    {
        #if defined(TESTING)
        DBprnt ("!!! ReadSurveyPointTransformation");
        #endif
        isSurveyPointTransformationRead = true;
        isSurveyPointTransformation_OK = ParamHelpers::GetSurveyPointTransformationToParamDict (glob);
        if (glob.IsEmpty ()) isSurveyPointTransformation_OK = false;
    }

    void ReadPlaceSets ()
    {
        isPlaceSetsRead = true;
        isPlaceSets_OK = ParamHelpers::GetPlaceSetsToParamDict (glob);
        if (glob.IsEmpty ()) isPlaceSets_OK = false;
    }

    void ReadLocOrigin ()
    {
        isLocOriginRead = true;
        isLocOrigin_OK = ParamHelpers::GetLocOriginToParamDict (glob);
        if (glob.IsEmpty ()) isLocOrigin_OK = false;
    }

    void ReadInfo ()
    {
        info.Clear ();
        isInfoRead = true;
        isInfo_OK = ParamHelpers::GetAllInfoToParamDict (info);
        if (info.IsEmpty ()) isInfo_OK = false;
    }

    void ReadAttribute ()
    {
        attrib.Clear ();
        isAttributeRead = true;
        isAttribute_OK = ParamHelpers::GetAllAttributeToParamDict (attrib);
        if (attrib.IsEmpty ()) isAttribute_OK = false;
    }

    void ReadPropertyDefinition ()
    {
        #if defined(TESTING)
        DBprnt ("!!! ReadPropertyDefinition");
        #endif
        property.Clear ();
        isPropertyDefinitionRead = true;
        isPropertyDefinitionRead_full = true;
        isPropertyDefinition_OK = ParamHelpers::GetAllPropertyDefinitionToParamDict (property);
        if (property.IsEmpty ()) isPropertyDefinition_OK = false;
    }

    void ReadClassification ()
    {
        systemdict.Clear ();
        isClassificationRead = true;
        if (ClassificationFunc::GetAllClassification (systemdict) == NoError) isClassification_OK = true;
        if (systemdict.IsEmpty ()) isClassification_OK = false;
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
