//------------ kuvbur 2026 ------------
#pragma once
#if !defined (PROPERTYCACHE_HPP)
#define	PROPERTYCACHE_HPP
#include    "Helpers.hpp"

namespace ParamHelpers
{
bool GetParamValueFromCache (const GS::UniString& rawname, ParamValue& pvalue);

bool isPropertyDefinitionRead ();

bool isAttributeRead ();

bool isCacheContainsParamValue (const GS::UniString& rawname);

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

}

struct PropertyCache
{
    ParamDictValue property;
    ParamDictValue info;
    ParamDictValue attrib;
    ParamDictValue glob;
    ClassificationFunc::SystemDict systemdict;

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

    bool isClassification_OK; // Успешно прочитан
    bool isClassificationRead;    // Был запрошен

    PropertyCache ()
    {
        property.Clear ();
        info.Clear ();
        attrib.Clear ();
        glob.Clear ();
        systemdict.Clear ();

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

        isAttribute_OK = false;
        isAttributeRead = false;

        isClassification_OK = false;
        isClassificationRead = false;
    }

    void ReadGetGeoLocation ()
    {
        isGetGeoLocationRead = true;
        isGetGeoLocation_OK = ParamHelpers::GetGeoLocationToParamDict (glob);
        if (glob.IsEmpty ()) isGetGeoLocation_OK = false;
    }

    void ReadSurveyPointTransformation ()
    {
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
        isInfoRead = true;
        isInfo_OK = ParamHelpers::GetAllInfoToParamDict (info);
        if (info.IsEmpty ()) isInfo_OK = false;
    }

    void ReadAttribute ()
    {
        isAttributeRead = true;
        isAttribute_OK = ParamHelpers::GetAllAttributeToParamDict (attrib);
        if (attrib.IsEmpty ()) isAttribute_OK = false;
    }

    void ReadPropertyDefinition ()
    {
        isPropertyDefinitionRead = true;
        isPropertyDefinition_OK = ParamHelpers::GetAllPropertyDefinitionToParamDict (property);
        if (property.IsEmpty ()) isPropertyDefinition_OK = false;
    }

    void ReadClassification ()
    {
        isClassificationRead = true;
        if (ClassificationFunc::GetAllClassification (systemdict) == NoError) isClassification_OK = true;
        if (systemdict.IsEmpty ()) isClassification_OK = false;
    }
};

PropertyCache& GetCache ();

extern PropertyCache& (*PROPERTYCACHE)();

#endif
