//------------ kuvbur 2026 ------------
#include	"ACAPinc.h"
#include    "Helpers.hpp"
#include	"Propertycache.hpp"

PropertyCache& GetCache ()
{
    static PropertyCache instance;
    return instance;
}

PropertyCache& (*PROPERTYCACHE)() = GetCache;

namespace ParamHelpers
{

void SetParamValueFromCache (const GS::UniString& rawname, ParamValue& pvalue)
{
    ParamValue paramFrom;
    if (!GetParamValueFromCache (rawname, paramFrom)) {
        return;
    }
    paramFrom.fromGuid = pvalue.fromGuid;
    paramFrom.toQRCode = pvalue.toQRCode;
    pvalue = paramFrom;
}

bool GetParamValueFromCache (const GS::UniString& rawname, ParamValue& pvalue)
{
    short inx = ParamHelpers::GetTypeInxByRawnamePrefix (rawname);
    if (!(inx == PROPERTYTYPEINX || inx == GLOBTYPEINX || inx == ATTRIBTYPEINX || inx == INFOTYPEINX)) {
        return false;
    }
    if (!isCacheContainsParamValue (inx, rawname)) {
        return false;
    }
    switch (inx) {
        case PROPERTYTYPEINX:
            pvalue = PROPERTYCACHE ().property.Get (rawname);
            return true;
            break;
        case GLOBTYPEINX:
            pvalue = PROPERTYCACHE ().glob.Get (rawname);
            return true;
            break;
        case ATTRIBTYPEINX:
            pvalue = PROPERTYCACHE ().attrib.Get (rawname);
            return true;
            break;
        case INFOTYPEINX:
            pvalue = PROPERTYCACHE ().info.Get (rawname);
            return true;
            break;
        default:
            return false;
            break;
    }
    return false;
}

bool isPropertyDefinitionRead ()
{
    if (!PROPERTYCACHE ().isPropertyDefinitionRead_full) {
        PROPERTYCACHE ().ReadPropertyDefinition ();
    }
    return PROPERTYCACHE ().isPropertyDefinition_OK;
}

bool isAttributeRead ()
{
    if (!PROPERTYCACHE ().isAttributeRead) {
        PROPERTYCACHE ().ReadAttribute ();
    }
    return PROPERTYCACHE ().isAttribute_OK;
}

bool isCacheContainsGroup (const API_Guid& guid)
{
    if (PROPERTYCACHE ().isGroupPropertyRead && PROPERTYCACHE ().isGroupProperty_OK) {
        if (PROPERTYCACHE ().propertygroups.ContainsKey (guid)) return true;
    }
    if (!PROPERTYCACHE ().isGroupPropertyRead_full) {
        PROPERTYCACHE ().AddGroupProperty (guid);
    }
    return PROPERTYCACHE ().propertygroups.ContainsKey (guid);
}

bool GetGroupFromCache (const API_Guid& guid, API_PropertyGroup& group)
{
    if (!isCacheContainsGroup (guid)) {
        #if defined(TESTING)
        DBprnt ("ERROR isCacheContainsGroup (guid) ");
        #endif
        return false;
    }
    group = PROPERTYCACHE ().propertygroups.Get (guid);
    return true;
}

bool isCacheContainsParamValue (const GS::UniString& rawname)
{
    short inx = ParamHelpers::GetTypeInxByRawnamePrefix (rawname);
    if (!(inx == PROPERTYTYPEINX || inx == GLOBTYPEINX || inx == ATTRIBTYPEINX || inx == INFOTYPEINX)) {
        return false;
    }
    return isCacheContainsParamValue (inx, rawname);
    return false;
}

bool isCacheContainsParamValue (const short& inx, const GS::UniString& rawname)
{
    switch (inx) {
        case PROPERTYTYPEINX:
            // Проверяем с кэша, если в кэше есть, то не читаем из API, если нет, то читаем и добавляем в кэш
            if (PROPERTYCACHE ().isPropertyDefinitionRead && PROPERTYCACHE ().isPropertyDefinition_OK) {
                if (PROPERTYCACHE ().property.ContainsKey (rawname)) return true;
            }
            // Если кэш не был прочитан, то читаем его и проверяем наличие ключа
            if (!PROPERTYCACHE ().isPropertyDefinitionRead_full) {
                PROPERTYCACHE ().ReadPropertyDefinition ();
            }
            if (!PROPERTYCACHE ().isPropertyDefinition_OK) {
                #if defined(TESTING)
                DBprnt ("ERROR isPropertyDefinition_OK " + rawname);
                #endif
                return false;
            }
            return PROPERTYCACHE ().property.ContainsKey (rawname);
            break;
        case GLOBTYPEINX:
            if (!PROPERTYCACHE ().isGetGeoLocationRead) PROPERTYCACHE ().ReadGetGeoLocation ();
            if (!PROPERTYCACHE ().isGetGeoLocation_OK) {
                #if defined(TESTING)
                DBprnt ("ERROR isGetGeoLocation_OK " + rawname);
                #endif
            }
            if (PROPERTYCACHE ().glob.ContainsKey (rawname)) return true;
            if (!PROPERTYCACHE ().isSurveyPointTransformationRead) PROPERTYCACHE ().ReadSurveyPointTransformation ();
            if (!PROPERTYCACHE ().isSurveyPointTransformation_OK) {
                #if defined(TESTING)
                DBprnt ("ERROR isSurveyPointTransformation_OK " + rawname);
                #endif
            }
            if (PROPERTYCACHE ().glob.ContainsKey (rawname)) return true;
            if (!PROPERTYCACHE ().isPlaceSetsRead) PROPERTYCACHE ().ReadPlaceSets ();
            if (!PROPERTYCACHE ().isPlaceSets_OK) {
                #if defined(TESTING)
                DBprnt ("ERROR isPlaceSets_OK " + rawname);
                #endif
            }
            if (PROPERTYCACHE ().glob.ContainsKey (rawname)) return true;
            if (!PROPERTYCACHE ().isLocOriginRead) PROPERTYCACHE ().ReadLocOrigin ();
            if (!PROPERTYCACHE ().isLocOrigin_OK) {
                #if defined(TESTING)
                DBprnt ("ERROR isLocOrigin_OK " + rawname);
                #endif
            }
            return PROPERTYCACHE ().glob.ContainsKey (rawname);
            break;
        case ATTRIBTYPEINX:
            if (!PROPERTYCACHE ().isAttributeRead) PROPERTYCACHE ().ReadAttribute ();
            if (!PROPERTYCACHE ().isAttribute_OK) {
                #if defined(TESTING)
                DBprnt ("ERROR isAttribute_OK " + rawname);
                #endif
                return false;
            }
            return PROPERTYCACHE ().attrib.ContainsKey (rawname);
            break;
        case INFOTYPEINX:
            if (!PROPERTYCACHE ().isInfoRead) PROPERTYCACHE ().ReadInfo ();
            if (!PROPERTYCACHE ().isInfo_OK) {
                #if defined(TESTING)
                DBprnt ("ERROR isInfo_OK " + rawname);
                #endif
                return false;
            }
            return PROPERTYCACHE ().info.ContainsKey (rawname);
            break;
        default:
            return false;
            break;
    }
    return false;
}

// --------------------------------------------------------------------
// Получение списка глобальных переменных о местоположении проекта, солнца
// --------------------------------------------------------------------
bool GetGeoLocationToParamDict (ParamDictValue& propertyParams)
{
    #if defined(AC_22) || defined(AC_23) || defined(AC_24)
    return false;
    #else
    #if defined(TESTING)
    DBprnt ("   GetGeoLocationToParamDict start");
    #endif
    GS::UniString name = "";
    GS::UniString rawName = "";
    GS::UniString prefix = GLOBNAMEPREFIX;
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
    DBprnt ("   GetGeoLocationToParamDict end");
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
    DBprnt ("   GetSurveyPointTransformationToParamDict start");
    #endif
    GS::UniString name = "";
    GS::UniString rawName = "";
    GS::UniString prefix = GLOBNAMEPREFIX;
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
    DBprnt ("   GetSurveyPointTransformationToParamDict end");
    #endif
    return true;
    #endif
}

// --------------------------------------------------------------------
// Получение списка глобальных переменных о местоположении проекта, солнца
// --------------------------------------------------------------------
bool GetPlaceSetsToParamDict (ParamDictValue& propertyParams)
{
    #if defined(TESTING)
    DBprnt ("   GetAllGlobToParamDict start");
    #endif
    GS::UniString name = "";
    GS::UniString rawName = "";
    GS::UniString prefix = GLOBNAMEPREFIX;
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
    DBprnt ("   GetAllGlobToParamDict end");
    #endif
    return true;
}

// --------------------------------------------------------------------
// Заполнение информации о локальном начале координат
// --------------------------------------------------------------------
bool GetLocOriginToParamDict (ParamDictValue& propertyParams)
{
    #if defined(TESTING)
    DBprnt ("   GetLocOriginToParamDict start");
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
        return false;
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
    GS::UniString prefix = GLOBNAMEPREFIX;
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
    DBprnt ("   GetLocOriginToParamDict end");
    #endif
    return true;
}

// --------------------------------------------------------------------
// Заполнение информации о проекте
// --------------------------------------------------------------------
bool GetAllInfoToParamDict (ParamDictValue& propertyParams)
{
    #if defined(TESTING)
    DBprnt ("   GetAllInfoToParamDict start");
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
    GS::UniString prefix = INFONAMEPREFIX;
    GS::UniString suffix = "}";
    for (UInt32 i = 0; i < autotexts.GetSize (); i++) {
        if (autotexts[i][0].Contains ("Addon_Dimens") && !autotexts[i][2].IsEmpty ()) {
            rawName = "{@info:addon_dimension_autotext}";
        } else {
            rawName = prefix;
            rawName.Append (autotexts[i][0].ToLowerCase ());
            rawName.Append (suffix);
        }
        if (!propertyParams.ContainsKey (rawName)) {
            ParamValue pvalue;
            pvalue.name = autotexts[i][1];
            pvalue.rawName = rawName;
            pvalue.fromInfo = true;
            ParamHelpers::ConvertStringToParamValue (pvalue, rawName, autotexts[i][2]);
            propertyParams.Add (rawName, pvalue);
        }
    }
    #if defined(TESTING)
    DBprnt ("   GetAllInfoToParamDict end");
    #endif
    return true;
}

// --------------------------------------------------------------------
// Получение списка аттрибутов (имён слоёв, материалов)
// --------------------------------------------------------------------
bool GetAllAttributeToParamDict (ParamDictValue& propertyParams)
{
    #if defined(TESTING)
    DBprnt ("   GetAllAttributeToParamDict start");
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
            ParamValue pvalue = {};
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
    DBprnt ("   GetAllAttributeToParamDict end");
    #endif
    return true;
}


// --------------------------------------------------------------------
// Получить все доступные свойства в формарте ParamDictValue
// --------------------------------------------------------------------
bool GetAllPropertyDefinitionToParamDict (ParamDictValue& propertyParams)
{
    #if defined(TESTING)
    DBprnt ("   GetAllPropertyDefinitionToParamDict start");
    #endif
    if (!PROPERTYCACHE ().isGroupPropertyRead_full) PROPERTYCACHE ().ReadGroupProperty ();
    if (!PROPERTYCACHE ().isGroupPropertyRead_full) return false;
    GSErrCode err = NoError;
    // Созданим словарь с определением всех свойств
    for (const auto& cIt : PROPERTYCACHE ().propertygroups) {
        #if defined(AC_28) || defined(AC_29)
        const API_PropertyGroup& group = cIt.value;
        #else
        const API_PropertyGroup& group = *cIt.value;
        #endif
        bool filter = true;
        #if defined(AC_28) || defined(AC_29)
        GS::UniString strguid = APIGuidToString (groups[i].guid);
        filter = (strguid.IsEqual ("3CF63E55-AA52-4AB4-B1C3-0920B2F352BF") || strguid.IsEqual ("6EE946D2-E840-4909-8EF1-F016AE905C52") || strguid.IsEqual ("BF31D3E0-A2B1-4543-A3DA-C1191D059FD8"));
        //TODO Дописать Guid группы "GeneralElemProperties"
        #else
        filter = (group.name.Contains ("Material") || group.name.IsEqual ("GeneralElemProperties"));
        #endif
        if (group.groupType == API_PropertyCustomGroupType || (group.groupType == API_PropertyStaticBuiltInGroupType && filter)) {
            GS::Array<API_PropertyDefinition> definitions = {};
            err = ACAPI_Property_GetPropertyDefinitions (group.guid, definitions);
            if (err != NoError) msg_rep ("GetPropertyByName", "ACAPI_Property_GetPropertyDefinitions", err, APINULLGuid);
            if (err == NoError) GetArrayPropertyDefinitionToParamDict (propertyParams, definitions);
        }
    }
    #if defined(TESTING)
    DBprnt ("   GetAllPropertyDefinitionToParamDict end");
    #endif
    return true;
}

// --------------------------------------------------------------------
// Перевод GS::Array<API_PropertyDefinition> в ParamDictValue
// --------------------------------------------------------------------
bool GetArrayPropertyDefinitionToParamDict (ParamDictValue& propertyParams, GS::Array<API_PropertyDefinition>& definitions)
{
    if (definitions.IsEmpty ()) return false;
    GS::UniString name = "";
    GS::UniString rawName = "";
    GS::UniString prefix = PROPERTYNAMEPREFIX;
    API_PropertyGroup group;
    bool flag_add = false;
    for (auto& definision : definitions) {
        if (!ParamHelpers::GetGroupFromCache (definision.groupGuid, group)) continue;
        if (definision.description.Contains ("Sync_name")) {
            for (UInt32 inx = 0; inx < 20; inx++) {
                GS::UniString strinx = GS::UniString::Printf ("%d", inx);
                rawName = "{@property:sync_name";
                rawName.Append (strinx);
                rawName.Append ("}");
                name = "sync_name";
                name.Append (strinx);
                if (!propertyParams.ContainsKey (rawName)) break;
            }
            definision.name = name;
            ParamValue pvalue = {};
            pvalue.rawName = rawName;
            pvalue.name = group.name;
            pvalue.name.Append ("/");
            pvalue.name.Append (definision.name);
            ParamHelpers::ConvertToParamValue (pvalue, definision);
            propertyParams.Add (pvalue.rawName, pvalue);
            flag_add = true;
        } else {
            #if defined(AC_28) || defined(AC_29)
            name = GetPropertyNameByGUID (definision.guid);
            if (name.IsEmpty ()) {
                name = groups[i].name;
                name.Append ("/");
                name.Append (definision.name);
            }
            #else
            name = group.name;
            name.Append ("/");
            name.Append (definision.name);
            #endif
            rawName = prefix;
            rawName.Append (name.ToLowerCase ());
            rawName.Append ("}");
            if (!propertyParams.ContainsKey (rawName)) {
                ParamValue pvalue;
                pvalue.rawName = rawName;
                pvalue.name = name;
                ParamHelpers::ConvertToParamValue (pvalue, definision);
                propertyParams.Add (pvalue.rawName, pvalue);
                flag_add = true;
            } else {
                if (propertyParams.Get (rawName).definition.guid != definision.guid) {
                    ParamValue pvalue = propertyParams.Get (rawName);
                    FormatString fstring = pvalue.val.formatstring;
                    if (!pvalue.fromPropertyDefinition && !pvalue.fromAttribDefinition) {
                        pvalue.rawName = rawName;
                        pvalue.name = name;
                        ParamHelpers::ConvertToParamValue (pvalue, definision);
                        if (!fstring.isEmpty) {
                            pvalue.val.formatstring = fstring;
                        }
                        propertyParams.Get (pvalue.rawName) = pvalue;
                        flag_add = true;
                    }
                }
            }
        }
    }
    return flag_add;
}
}

// -----------------------------------------------------------------------------
// Проверка языка Архикада. Для INT возвращает 1000
// -----------------------------------------------------------------------------
Int32 isEng ()
{
    #ifdef EXTNDVERSION
    return 0;
    #endif
    if (!PROPERTYCACHE ().isEng_OK) PROPERTYCACHE ().ReadisEng ();
    return PROPERTYCACHE ().isEng;
}

GS::UniString GetPropertyNameByGUID (const API_Guid& guid)
{
    if (guid == APINULLGuid) return "";
    GS::UniString strguid = APIGuidToString (guid);
    if (strguid.IsEqual ("2E906CCE-9A42-4E49-AE45-193D0D709CC4")) return "BuildingMaterialProperties/Building Material CutFill";
    if (strguid.IsEqual ("FAF74D9D-3CD4-4A03-9840-A39DB757DB1C")) return "BuildingMaterialProperties/Building Material Density";
    if (strguid.IsEqual ("68947382-7220-449A-AE47-F6F8CB47DE49")) return "BuildingMaterialProperties/Building Material Description";
    if (strguid.IsEqual ("902756A0-71D1-402B-B639-640BA5837A95")) return "BuildingMaterialProperties/Building Material ID";
    if (strguid.IsEqual ("A01BCC22-D1FC-4CD8-AD34-95BBE73BDD5E")) return "BuildingMaterialProperties/Building Material Manufacturer";
    if (strguid.IsEqual ("294C063C-98D8-42B5-B2C1-C27DE7CAB756")) return "BuildingMaterialProperties/Building Material Thermal Conductivity";
    if (strguid.IsEqual ("F99C8A52-810A-4D01-A33A-AB5FDBA43A20")) return "BuildingMaterialProperties/Building Material Heat Capacity";
    if (strguid.IsEqual ("A01BCC22-D1FC-4CD8-AD34-95BBE73BDD5E")) return "BuildingMaterialProperties/Building Material Manufacturer";
    if (strguid.IsEqual ("A936C5CB-5126-4135-BD87-D2A46AEF5A07")) return "BuildingMaterialProperties/Building Material Name";
    return "";
}

// -----------------------------------------------------------------------------
// Получить полное имя свойства (включая имя группы)
// -----------------------------------------------------------------------------
GSErrCode GetPropertyFullName (const API_PropertyDefinition& definision, GS::UniString& name)
{
    if (definision.groupGuid == APINULLGuid) return APIERR_BADID;
    GSErrCode error = NoError;
    if (definision.name.Contains ("ync_name")) {
        name = definision.name;
    } else {
        #if defined(AC_28) || defined(AC_29)
        name = GetPropertyNameByGUID (definision.guid);
        if (!name.IsEmpty ()) {
            if (definision.name.Contains (CharENTER)) {
                UInt32 n = definision.name.FindFirst (CharENTER);
                UInt32 l = definision.name.GetLength ();
                GS::UniString attribsiffix = definision.name.GetSuffix (l - n);
                name = name + attribsiffix;
            }
            return NoError;
        }
        #endif
        API_PropertyGroup group;
        if (ParamHelpers::GetGroupFromCache (definision.groupGuid, group)) {
            name = group.name;
            name.Append ("/");
            name.Append (definision.name);
        } else {
            msg_rep ("GetPropertyFullName", "ACAPI_Property_GetPropertyGroup " + definision.name, error, APINULLGuid);
        }
    }
    return error;
}

// -----------------------------------------------------------------------------
//	Формат записи: ПЕРО_РАЗМЕРА - КРАТНОСТЬ_ММ, ПЕРО_ТЕКСТА_ИЗМЕНЁННОЕ, ФЛАГ_ИЗМЕНЕНИЯ_СОДЕРЖИМОГО, "ФОРМУЛА", либо
//					"Слой" - КРАТНОСТЬ_ММ, ПЕРО_ТЕКСТА_ИЗМЕНЁННОЕ, ФЛАГ_ИЗМЕНЕНИЯ_СОДЕРЖИМОГО, "ФОРМУЛА"
// -----------------------------------------------------------------------------
bool DimReadPref (DimRules& dimrules, const GS::UniString& autotext)
{
    bool hasexpression = false; // Нужно ли нам читать список свойств
    if (autotext.Contains (";")) {
        GS::Array<GS::UniString> partstring = {};
        StringSplt (autotext, ";", partstring);
        for (UInt32 k = 0; k < partstring.GetSize (); k++) {
            DimRule dimrule = {};
            if (DimParsePref (partstring[k], dimrule, hasexpression)) {
                GS::UniString kstr;
                if (dimrule.layer.IsEmpty ()) {
                    kstr = GS::UniString::Printf ("%d", dimrule.pen_original);
                } else {
                    kstr = dimrule.layer;
                }
                dimrules.Add (kstr, dimrule);
            }
        }
    } else {
        DimRule dimrule = {};
        if (DimParsePref (autotext, dimrule, hasexpression)) {
            GS::UniString kstr = "";
            if (dimrule.layer.IsEmpty ()) {
                kstr = GS::UniString::Printf ("%d", dimrule.pen_original);
            } else {
                kstr = dimrule.layer;
            }
            dimrules.Add (kstr, dimrule);
        }
    }
    return !dimrules.IsEmpty ();
}

// -----------------------------------------------------------------------------
// Обработка текста правила
// -----------------------------------------------------------------------------
bool DimParsePref (const GS::UniString& rawrule, DimRule& dimrule, bool& hasexpression)
{
    if (rawrule.IsEmpty ()) return false;
    if (!rawrule.Contains ("-")) return false;
    bool flag_find = false;
    GS::Array<GS::UniString> partstring_1 = {};
    if (StringSplt (rawrule, "-", partstring_1) == 2) {
        //Проверяем - что указано в правиле: слой или номер пера
        // Слой указываем в кавычках, в regexp формате
        if (partstring_1[0].Contains ('"')) {
            GS::UniString layer = partstring_1[0];
            layer.ReplaceAll ('"', ' ');
            layer.Trim ();
            dimrule.layer = layer;
        } else {
            dimrule.pen_original = std::atoi (partstring_1[0].ToCStr ());
        }
        if (partstring_1[1].Contains ("DeleteWall")) {
            dimrule.flag_change = true;
            dimrule.flag_deletewall = true;
            dimrule.pen_rounded = dimrule.pen_original;
            flag_find = true;
        }
        if (partstring_1[1].Contains ("ResetText")) {
            dimrule.flag_change = true;
            dimrule.flag_reset = true;
            dimrule.pen_rounded = dimrule.pen_original;
            flag_find = true;
        }
        if (partstring_1[1].Contains ("CheckCustom")) {
            dimrule.flag_change = false;
            dimrule.flag_custom = true;
            flag_find = true;
        }
        if (partstring_1[1].Contains ("lassic")) {
            dimrule.classic_round_mode = true;
        }
        if (!partstring_1[1].Contains (",")) return flag_find;
        GS::Array<GS::UniString> partstring_2 = {};
        if (StringSplt (partstring_1[1], ",", partstring_2) > 1) {
            if (!partstring_2[0].IsEmpty ()) {
                dimrule.round_value = std::atoi (partstring_2[0].ToCStr ());
                flag_find = true;
            }
            if (!partstring_2[1].IsEmpty ()) {
                dimrule.pen_rounded = std::atoi (partstring_2[1].ToCStr ());
                flag_find = true;
            }
            if (partstring_2.GetSize () < 3) return flag_find;
            for (UInt32 k = 2; k < partstring_2.GetSize (); k++) {
                if (partstring_2[k].IsEmpty ()) continue;
                if (partstring_2[k].Contains ("{") && partstring_2[k].Contains ("}")) {
                    ParamDictValue paramDict = {};
                    GS::UniString expression = partstring_2[k];
                    expression.ReplaceAll ("<MeasuredValue>", "{MeasuredValue}");
                    ParamHelpers::ParseParamName (expression, paramDict);
                    dimrule.paramDict = paramDict;
                    dimrule.expression = expression;
                    if (!hasexpression) hasexpression = !paramDict.IsEmpty ();
                } else {
                    if (k == 2) {
                        dimrule.flag_change = (std::atoi (partstring_2[k].ToCStr ()) > 0);
                        flag_find = true;
                    }
                }
            }
        }
    }
    return flag_find;
}
