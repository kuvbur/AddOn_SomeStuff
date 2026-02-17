//------------ kuvbur 2026 ------------
#pragma once
#if !defined (PROPERTYCACHE_HPP)
#define	PROPERTYCACHE_HPP
#include    "Helpers.hpp"
struct PropertyCache
{
    ParamDictValue propertyParams;
    ClassificationFunc::SystemDict systemdict;

    bool isPropertyDefinitionRead_OK;
    bool isPropertyDefinitionRead;

    bool isInfoRead_OK;
    bool isInfoRead;

    bool isClassificationRead_OK;
    bool isClassificationRead;
    PropertyCache ()
    {
        propertyParams.Clear ();
        systemdict.Clear ();
        isPropertyDefinitionRead_OK = false;
        isPropertyDefinitionRead = false;
        isInfoRead_OK = false;
        isInfoRead = false;
        isClassificationRead_OK = false;
        isClassificationRead = false;
    }

    void ReadPropertyDefinition ()
    {
        isPropertyDefinitionRead_OK = ParamHelpers::AllPropertyDefinitionToParamDict (propertyParams);
        isPropertyDefinitionRead = true;
    }

    void ReadGetGeoLocation ()
    {
        ParamHelpers::GetGeoLocationToParamDict (propertyParams);
        isPropertyDefinitionRead = true;
        if (propertyParams.ContainsKey ("flag:has_geolocation")) isPropertyDefinitionRead_OK = true;
    }

    void ReadPropertyDefinition ()
    {
        ParamHelpers::AllPropertyDefinitionToParamDict (propertyParams);
        isPropertyDefinitionRead = true;
        if (propertyParams.ContainsKey ("flag:has_locorigin")) isPropertyDefinitionRead_OK = true;
    }

    void ReadPropertyDefinition ()
    {
        ParamHelpers::AllPropertyDefinitionToParamDict (propertyParams);
        isPropertyDefinitionRead = true;
        if (propertyParams.ContainsKey ("flag:has_survtm")) isPropertyDefinitionRead_OK = true;
    }

    void ReadPropertyDefinition ()
    {
        ParamHelpers::AllPropertyDefinitionToParamDict (propertyParams);
        isPropertyDefinitionRead = true;
        if (propertyParams.ContainsKey ("flag:has_surv")) isPropertyDefinitionRead_OK = true;
    }


    void ReadPropertyDefinition ()
    {
        ParamHelpers::AllPropertyDefinitionToParamDict (propertyParams);
        isPropertyDefinitionRead = true;
        if (propertyParams.ContainsKey ("flag:has_attrib")) isPropertyDefinitionRead_OK = true;
    }

    void ReadPropertyDefinition ()
    {
        ParamHelpers::AllPropertyDefinitionToParamDict (propertyParams);
        isPropertyDefinitionRead = true;
        if (propertyParams.ContainsKey ("flag:has_info")) isPropertyDefinitionRead_OK = true;
    }

    void ReadClassification ()
    {
        isClassificationRead = true;
        if (ClassificationFunc::GetAllClassification (systemdict) == NoError) isClassificationRead_OK = true;
    }

    //
    //ParamHelpers::GetAllInfoToParamDict (propertyParams);
    //ParamHelpers::GetAllGlobToParamDict (propertyParams);
};

PropertyCache& GetInstance ();

extern PropertyCache& (*PROPERTYCACHE)();

namespace ParamHelpers
{

bool GetGeoLocationToParamDict (ParamDictValue& propertyParams);

bool GetSurveyPointTransformationToParamDict (ParamDictValue& propertyParams);

// --------------------------------------------------------------------
// Получение списка глобальных переменных о местоположении проекта, солнца
// --------------------------------------------------------------------
bool GetAllGlobToParamDict (ParamDictValue& propertyParams);


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

}
#endif
