//------------ kuvbur 2026 ------------
#include	"ACAPinc.h"
#include    "Helpers.hpp"
#include	"Propertycache.hpp"

PropertyCache& GetInstance ()
{
    static PropertyCache instance;
    return instance;
}

PropertyCache& (*PROPERTYCACHE)() = GetInstance;

namespace ParamHelpers
{
// --------------------------------------------------------------------
// Получение списка глобальных переменных о местоположении проекта, солнца
// --------------------------------------------------------------------
bool GetGeoLocationToParamDict (ParamDictValue& propertyParams)
{
    #if defined(AC_22) || defined(AC_23) || defined(AC_24)
    return false;
    #else
    #if defined(TESTING)
    DBprnt ("GetGeoLocationToParamDict start");
    #endif
    GS::UniString name = "";
    GS::UniString rawName = "";
    GS::UniString prefix = "{@glob:";
    GS::UniString suffix = "}";
    ParamValue pvalue = {};
    GSErrCode err = NoError;
    API_GeoLocation apiGeoLocation = {};
    #if defined(AC_27) || defined(AC_28) || defined(AC_29)
    err = ACAPI_GeoLocation_GetGeoLocation (&apiGeoLocation);
    #else
    err = ACAPI_Environment (APIEnv_GetGeoLocationID, &apiGeoLocation);
    #endif
    if (err != NoError) {
        msg_rep ("GetGeoLocationToParamDict", "APIEnv_GetPlaceSetsID", err, APINULLGuid);
        return false;
    }
    name = "surveyPointPosition_x"; rawName = prefix; rawName.Append (name.ToLowerCase ()); rawName.Append (suffix);
    pvalue.name = name; pvalue.rawName = rawName;
    ParamHelpers::ConvertDoubleToParamValue (pvalue, rawName, apiGeoLocation.surveyPointPosition.x);
    propertyParams.Add (rawName, pvalue);
    name = "surveyPointPosition_y"; rawName = prefix; rawName.Append (name.ToLowerCase ()); rawName.Append (suffix);
    pvalue.name = name; pvalue.rawName = rawName;
    ParamHelpers::ConvertDoubleToParamValue (pvalue, rawName, apiGeoLocation.surveyPointPosition.y);
    propertyParams.Add (rawName, pvalue);
    name = "surveyPointPosition_z"; rawName = prefix; rawName.Append (name.ToLowerCase ()); rawName.Append (suffix);
    pvalue.name = name; pvalue.rawName = rawName;
    ParamHelpers::ConvertDoubleToParamValue (pvalue, rawName, apiGeoLocation.surveyPointPosition.z);
    propertyParams.Add (rawName, pvalue);
    name = "eastings"; rawName = prefix; rawName.Append (name.ToLowerCase ()); rawName.Append (suffix);
    pvalue.name = name; pvalue.rawName = rawName;
    ParamHelpers::ConvertDoubleToParamValue (pvalue, rawName, apiGeoLocation.geoReferenceData.eastings);
    propertyParams.Add (rawName, pvalue);
    name = "northings"; rawName = prefix; rawName.Append (name.ToLowerCase ()); rawName.Append (suffix);
    pvalue.name = name; pvalue.rawName = rawName;
    ParamHelpers::ConvertDoubleToParamValue (pvalue, rawName, apiGeoLocation.geoReferenceData.northings);
    propertyParams.Add (rawName, pvalue);
    name = "orthogonalHeight"; rawName = prefix; rawName.Append (name.ToLowerCase ()); rawName.Append (suffix);
    pvalue.name = name; pvalue.rawName = rawName;
    ParamHelpers::ConvertDoubleToParamValue (pvalue, rawName, apiGeoLocation.geoReferenceData.orthogonalHeight);
    propertyParams.Add (rawName, pvalue);
    name = "xAxisAbscissa"; rawName = prefix; rawName.Append (name.ToLowerCase ()); rawName.Append (suffix);
    pvalue.name = name; pvalue.rawName = rawName;
    ParamHelpers::ConvertDoubleToParamValue (pvalue, rawName, apiGeoLocation.geoReferenceData.xAxisAbscissa);
    propertyParams.Add (rawName, pvalue);
    name = "xAxisOrdinate"; rawName = prefix; rawName.Append (name.ToLowerCase ()); rawName.Append (suffix);
    pvalue.name = name; pvalue.rawName = rawName;
    ParamHelpers::ConvertDoubleToParamValue (pvalue, rawName, apiGeoLocation.geoReferenceData.xAxisOrdinate);
    propertyParams.Add (rawName, pvalue);
    name = "scale"; rawName = prefix; rawName.Append (name.ToLowerCase ()); rawName.Append (suffix);
    pvalue.name = name; pvalue.rawName = rawName;
    ParamHelpers::ConvertDoubleToParamValue (pvalue, rawName, apiGeoLocation.geoReferenceData.scale);
    #if defined(TESTING)
    DBprnt ("  GetGeoLocationToParamDict end");
    #endif
    return true;
    #endif
}
// --------------------------------------------------------------------
// Получение списка глобальных переменных о местоположении проекта, солнца
// --------------------------------------------------------------------
bool GetSurveyPointTransformationToParamDict (ParamDictValue& propertyParams)
{
    #if defined(AC_22) || defined(AC_23) || defined(AC_24)
    return false;
    #else
    #if defined(TESTING)
    DBprnt ("GetSurveyPointTransformationToParamDict start");
    #endif
    GS::UniString name = "";
    GS::UniString rawName = "";
    GS::UniString prefix = "{@glob:";
    GS::UniString suffix = "}";
    ParamValue pvalue = {};
    GSErrCode err = NoError;
    API_Tranmat tm = {};
    #if defined(AC_27) || defined(AC_28) || defined(AC_29)
    err = ACAPI_SurveyPoint_GetSurveyPointTransformation (&tm);
    #else
    err = ACAPI_Environment (APIEnv_GetSurveyPointTransformationID, &tm);
    #endif
    if (err != NoError) {
        msg_rep ("GetSurveyPointTransformationToParamDict", "APIEnv_GetSurveyPointTransformationID", err, APINULLGuid);
        return false;
    }
    for (UInt32 i = 0; i < 13; i++) {
        name = "tmx" + GS::UniString::Printf ("%d", i); rawName = prefix; rawName.Append (name.ToLowerCase ()); rawName.Append (suffix);
        pvalue.name = name; pvalue.rawName = rawName;
        ParamHelpers::ConvertDoubleToParamValue (pvalue, rawName, tm.tmx[i]);
        propertyParams.Add (rawName, pvalue);
    }
    #if defined(TESTING)
    DBprnt ("  GetSurveyPointTransformationToParamDict end");
    #endif
    return true;
    #endif
}

// --------------------------------------------------------------------
// Получение списка глобальных переменных о местоположении проекта, солнца
// --------------------------------------------------------------------
bool GetAllGlobToParamDict (ParamDictValue& propertyParams)
{
    #if defined(TESTING)
    DBprnt ("GetAllGlobToParamDict start");
    #endif
    GS::UniString name = "";
    GS::UniString rawName = "";
    GS::UniString prefix = "{@glob:";
    GS::UniString suffix = "}";
    ParamValue pvalue = {};
    API_PlaceInfo placeInfo = {};
    GSErrCode err = NoError;
    #if defined(AC_27) || defined(AC_28) || defined(AC_29)
    err = ACAPI_GeoLocation_GetPlaceSets (&placeInfo);
    #else
    err = ACAPI_Environment (APIEnv_GetPlaceSetsID, &placeInfo, nullptr);
    #endif
    if (err != NoError) {
        msg_rep ("GetAllGlobToParamDict", "APIEnv_GetPlaceSetsID", err, APINULLGuid);
        return false;
    }
    name = "GLOB_NORTH_DIR"; rawName = prefix; rawName.Append (name.ToLowerCase ()); rawName.Append (suffix);
    pvalue.name = name; pvalue.rawName = rawName;
    ParamHelpers::ConvertDoubleToParamValue (pvalue, rawName, round ((placeInfo.north * 180 / PI) * 1000.0) / 1000.0);
    propertyParams.Add (rawName, pvalue);
    name = "GLOB_PROJECT_LONGITUDE"; rawName = prefix; rawName.Append (name.ToLowerCase ()); rawName.Append (suffix);
    pvalue.name = name; pvalue.rawName = rawName;
    ParamHelpers::ConvertDoubleToParamValue (pvalue, rawName, placeInfo.longitude);
    propertyParams.Add (rawName, pvalue);
    name = "GLOB_PROJECT_LATITUDE"; rawName = prefix; rawName.Append (name.ToLowerCase ()); rawName.Append (suffix);
    pvalue.name = name; pvalue.rawName = rawName;
    ParamHelpers::ConvertDoubleToParamValue (pvalue, rawName, placeInfo.latitude);
    propertyParams.Add (rawName, pvalue);
    name = "GLOB_PROJECT_ALTITUDE"; rawName = prefix; rawName.Append (name.ToLowerCase ()); rawName.Append (suffix);
    pvalue.name = name; pvalue.rawName = rawName;
    ParamHelpers::ConvertDoubleToParamValue (pvalue, rawName, placeInfo.altitude);
    propertyParams.Add (rawName, pvalue);
    name = "GLOB_SUN_AZIMUTH"; rawName = prefix; rawName.Append (name.ToLowerCase ()); rawName.Append (suffix);
    pvalue.name = name; pvalue.rawName = rawName;
    ParamHelpers::ConvertDoubleToParamValue (pvalue, rawName, round ((placeInfo.sunAngXY * 180 / PI) * 1000.0) / 1000.0);
    propertyParams.Add (rawName, pvalue);
    name = "GLOB_SUN_ALTITUDE"; rawName = prefix; rawName.Append (name.ToLowerCase ()); rawName.Append (suffix);
    pvalue.name = name; pvalue.rawName = rawName;
    ParamHelpers::ConvertDoubleToParamValue (pvalue, rawName, round ((placeInfo.sunAngZ * 180 / PI) * 1000.0) / 1000.0);
    propertyParams.Add (rawName, pvalue);
    #if defined(TESTING)
    DBprnt ("  GetAllGlobToParamDict end");
    #endif
    return true;
}


// --------------------------------------------------------------------
// Заполнение информации о локальном начале координат
// --------------------------------------------------------------------
bool GetLocOriginToParamDict (ParamDictValue& propertyParams)
{
    #if defined(TESTING)
    DBprnt ("GetLocOriginToParamDict start");
    #endif
    //Пользовательское начало
    API_Coord3D locOrigin;
    API_Coord offset;
    GSErrCode err = NoError;
    #if defined(AC_27) || defined(AC_28) || defined(AC_29)
    err = ACAPI_Database_GetLocOrigo (&locOrigin);
    #else
    err = ACAPI_Database (APIDb_GetLocOrigoID, &locOrigin);
    #endif
    if (err != NoError) {
        msg_rep ("GetLocOriginToParamDict", "APIDb_GetLocOrigoID", err, APINULLGuid);
        return;
    }
    #if defined(AC_27) || defined(AC_28) || defined(AC_29)
    err = ACAPI_ProjectSetting_GetOffset (&offset);
    #else
    err = ACAPI_Database (APIDb_GetOffsetID, &offset);
    #endif
    if (err != NoError) {
        msg_rep ("GetLocOriginToParamDict", "APIDb_GetOffsetID", err, APINULLGuid);
        return false;
    }
    GS::UniString prefix = "{@coord:";
    GS::UniString suffix = "}";
    ParamValue pvalue = {};
    pvalue.val.formatstring = FormatStringFunc::ParseFormatString ("1mm");
    pvalue.fromCoord = true;

    pvalue.name = "locOrigin_x";
    pvalue.rawName = prefix; pvalue.rawName.Append (pvalue.name.ToLowerCase ()); pvalue.rawName.Append (suffix);
    ParamHelpers::ConvertDoubleToParamValue (pvalue, "", locOrigin.x + offset.x);
    propertyParams.Add (pvalue.rawName, pvalue);

    pvalue.name = "locOrigin_y";
    pvalue.rawName = prefix; pvalue.rawName.Append (pvalue.name.ToLowerCase ()); pvalue.rawName.Append (suffix);
    ParamHelpers::ConvertDoubleToParamValue (pvalue, "", locOrigin.y + offset.y);
    propertyParams.Add (pvalue.rawName, pvalue);

    pvalue.name = "locOrigin_z";
    pvalue.rawName = prefix; pvalue.rawName.Append (pvalue.name.ToLowerCase ()); pvalue.rawName.Append (suffix);
    ParamHelpers::ConvertDoubleToParamValue (pvalue, "", locOrigin.z);
    propertyParams.Add (pvalue.rawName, pvalue);

    pvalue.name = "offsetOrigin_x";
    pvalue.rawName = prefix; pvalue.rawName.Append (pvalue.name.ToLowerCase ()); pvalue.rawName.Append (suffix);
    ParamHelpers::ConvertDoubleToParamValue (pvalue, "", offset.x);
    propertyParams.Add (pvalue.rawName, pvalue);

    pvalue.name = "offsetOrigin_y";
    pvalue.rawName = prefix; pvalue.rawName.Append (pvalue.name.ToLowerCase ()); pvalue.rawName.Append (suffix);
    ParamHelpers::ConvertDoubleToParamValue (pvalue, "", offset.y);
    propertyParams.Add (pvalue.rawName, pvalue);
    #if defined(TESTING)
    DBprnt ("  GetLocOriginToParamDict end");
    #endif
    return true;
}

// --------------------------------------------------------------------
// Заполнение информации о проекте
// --------------------------------------------------------------------
bool GetAllInfoToParamDict (ParamDictValue& propertyParams)
{
    #if defined(TESTING)
    DBprnt ("GetAllInfoToParamDict start");
    #endif
    GS::Array<GS::ArrayFB<GS::UniString, 3> > autotexts = {};
    API_AutotextType type = APIAutoText_Custom;
    GSErrCode err = NoError;
    #if defined(AC_27) || defined(AC_28) || defined(AC_29)
    err = ACAPI_AutoText_GetAutoTexts (&autotexts, type);
    #else
    err = ACAPI_Goodies (APIAny_GetAutoTextsID, &autotexts, (void*) (GS::IntPtr) type);
    #endif
    if (err != NoError) {
        msg_rep ("GetAllInfoToParamDict", "APIAny_GetAutoTextsID", err, APINULLGuid);
        return false;
    }
    GS::UniString rawName = "";
    GS::UniString prefix = "{@info:";
    GS::UniString suffix = "}";
    for (UInt32 i = 0; i < autotexts.GetSize (); i++) {
        rawName = prefix;
        rawName.Append (autotexts[i][0].ToLowerCase ());
        rawName.Append (suffix);
        if (!propertyParams.ContainsKey (rawName)) {
            ParamValue pvalue;
            pvalue.name = autotexts[i][1];
            pvalue.rawName = rawName;
            pvalue.fromInfo = true;
            ParamHelpers::ConvertStringToParamValue (pvalue, rawName, autotexts[i][2]);
            propertyParams.Add (rawName, pvalue);
        }
    }
    ParamHelpers::AddValueToParamDictValue (propertyParams, "flag:has_info");
    #if defined(TESTING)
    DBprnt ("  GetAllInfoToParamDict end");
    #endif
    return true;
}

// --------------------------------------------------------------------
// Получение списка аттрибутов (имён слоёв, материалов)
// --------------------------------------------------------------------
bool GetAllAttributeToParamDict (ParamDictValue& propertyParams)
{
    #if defined(TESTING)
    DBprnt ("GetAllAttributeToParamDict start");
    #endif
    API_Attribute attrib = {};
    GSErrCode err = NoError;
    GS::Array<API_Attribute> attributes = {};
    err = ACAPI_Attribute_GetAttributesByType (API_LayerID, attributes);
    for (API_Attribute& attrib : attributes) {
        GS::UniString attribname = "";
        attrib.header.uniStringNamePtr = &attribname;
        err = ACAPI_Attribute_Get (&attrib);
        if (err == NoError) {
            ParamValue pvalue;
            GS::UniString rawName = "layer_name_" + attribname;
            ParamHelpers::ConvertAttributeToParamValue (pvalue, rawName, attrib);
            propertyParams.Add (pvalue.rawName, pvalue);
            pvalue.name = "";
            pvalue.rawName = "";
            #if defined(AC_27) || defined(AC_28) || defined(AC_29)
            rawName = "layer_inx_" + GS::UniString::Printf ("%d", attrib.header.index.ToInt32_Deprecated ());
            #else
            rawName = "layer_inx_" + GS::UniString::Printf ("%d", attrib.header.index);
            #endif
            ParamHelpers::ConvertAttributeToParamValue (pvalue, rawName, attrib);
            propertyParams.Add (pvalue.rawName, pvalue);
        } else {
            if (err == APIERR_DELETED) err = NoError;
            if (err != NoError) {
                msg_rep ("GetAllAttributeToParamDict", "ACAPI_Attribute_Get", err, APINULLGuid);
                return false;
            }
        }
    }
    #if defined(TESTING)
    DBprnt ("  GetAllAttributeToParamDict end");
    #endif
    return true;
}


// --------------------------------------------------------------------
// Получить все доступные свойства в формарте ParamDictValue
// --------------------------------------------------------------------
void ParamHelpers::AllPropertyDefinitionToParamDict (ParamDictValue& propertyParams)
{
    GS::Array<API_PropertyGroup> groups = {};
    #if defined(TESTING)
    DBprnt ("AllPropertyDefinitionToParamDict start");
    #endif
    if (ParamHelpers::hasProperyDefinition (propertyParams)) {
        #if defined(TESTING)
        DBprnt ("  AllPropertyDefinitionToParamDict READ BEFORE end");
        #endif
        return;
    }
    GSErrCode err = ACAPI_Property_GetPropertyGroups (groups);
    if (err != NoError) {
        msg_rep ("GetAllPropertyDefinitionToParamDict", "ACAPI_Property_GetPropertyGroups", err, APINULLGuid);
        return;
    }
    UInt32 nparams = propertyParams.GetSize ();
    GS::UniString name = "";
    GS::UniString rawName = "";
    GS::UniString prefix = "{@property:";
    // Созданим словарь с определением всех свойств
    for (UInt32 i = 0; i < groups.GetSize (); i++) {
        bool filter = true;
        #if defined(AC_28) || defined(AC_29)
        GS::UniString strguid = APIGuidToString (groups[i].guid);
        filter = (strguid.IsEqual ("3CF63E55-AA52-4AB4-B1C3-0920B2F352BF") || strguid.IsEqual ("6EE946D2-E840-4909-8EF1-F016AE905C52") || strguid.IsEqual ("BF31D3E0-A2B1-4543-A3DA-C1191D059FD8"));
        //TODO Дописать Guid группы "GeneralElemProperties"
        #else
        filter = (groups[i].name.Contains ("Material") || groups[i].name.IsEqual ("GeneralElemProperties"));
        #endif
        if (groups[i].groupType == API_PropertyCustomGroupType || (groups[i].groupType == API_PropertyStaticBuiltInGroupType && filter)) {
            GS::Array<API_PropertyDefinition> definitions = {};
            err = ACAPI_Property_GetPropertyDefinitions (groups[i].guid, definitions);
            if (err != NoError) msg_rep ("GetPropertyByName", "ACAPI_Property_GetPropertyDefinitions", err, APINULLGuid);
            if (err == NoError) {
                for (UInt32 j = 0; j < definitions.GetSize (); j++) {
                    // TODO Когда в проекте есть два и более свойств с описанием Sync_name возникает ошибка
                    if (definitions[j].description.Contains ("Sync_name")) {
                        for (UInt32 inx = 0; inx < 20; inx++) {
                            GS::UniString strinx = GS::UniString::Printf ("%d", inx);
                            rawName = "{@property:sync_name";
                            rawName.Append (strinx);
                            rawName.Append ("}");
                            name = "sync_name";
                            name.Append (strinx);
                            if (!propertyParams.ContainsKey (rawName)) break;
                        }
                        definitions[j].name = name;
                        ParamValue pvalue = {};
                        pvalue.rawName = rawName;
                        pvalue.name = groups[i].name;
                        pvalue.name.Append ("/");
                        pvalue.name.Append (definitions[j].name);
                        ParamHelpers::ConvertToParamValue (pvalue, definitions[j]);
                        propertyParams.Add (pvalue.rawName, pvalue);
                    } else {
                        #if defined(AC_28) || defined(AC_29)
                        name = GetPropertyNameByGUID (definitions[j].guid);
                        if (name.IsEmpty ()) {
                            name = groups[i].name;
                            name.Append ("/");
                            name.Append (definitions[j].name);
                        }
                        #else
                        name = groups[i].name;
                        name.Append ("/");
                        name.Append (definitions[j].name);
                        #endif
                        rawName = prefix;
                        rawName.Append (name.ToLowerCase ());
                        rawName.Append ("}");
                        if (!propertyParams.ContainsKey (rawName)) {
                            ParamValue pvalue;
                            pvalue.rawName = rawName;
                            pvalue.name = name;
                            ParamHelpers::ConvertToParamValue (pvalue, definitions[j]);
                            propertyParams.Add (pvalue.rawName, pvalue);
                        } else {
                            ParamValue pvalue = propertyParams.Get (rawName);
                            FormatString fstring = pvalue.val.formatstring;
                            if (!pvalue.fromPropertyDefinition && !pvalue.fromAttribDefinition) {
                                pvalue.rawName = rawName;
                                pvalue.name = name;
                                ParamHelpers::ConvertToParamValue (pvalue, definitions[j]);
                                if (!fstring.isEmpty) {
                                    pvalue.val.formatstring = fstring;
                                }
                                propertyParams.Get (pvalue.rawName) = pvalue;
                                nparams--;
                                if (nparams == 0) {
                                    #if defined(TESTING)
                                    DBprnt ("    AllPropertyDefinitionToParamDict return");
                                    #endif
                                    return true;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    #if defined(TESTING)
    DBprnt ("  AllPropertyDefinitionToParamDict end");
    #endif
    return true;
}


}

