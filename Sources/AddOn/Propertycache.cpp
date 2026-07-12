//------------ kuvbur 2026 ------------
#include "ACAPinc.h"
#include "CommonFunction.hpp"
#include "File.hpp"
#include "Helpers.hpp"
#include "Propertycache.hpp"

PropertyCache &GetCache () {
    static PropertyCache instance;
    return instance;
}

PropertyCache &(*PROPERTYCACHE) () = GetCache;

namespace ParamHelpers {

    bool ReadLibraryFile (const GS::UniString &fileName, GS::Array<GS::Array<GS::UniString>> &data) {
        std::string lineStr;
        GS::UniString s;
        API_LibPart settingsText = {};
        GS::UniString locPath = "";
        GSErrCode err = NoError;
        // Поиск файла
        GS::ucscpy (settingsText.file_UName,
                    fileName.ToUStr (0, GS::Min (fileName.GetLength (), (USize)API_UniLongNameLen)).Get ());
#if defined(AC_27) || defined(AC_28) || defined(AC_29)
        err = ACAPI_LibraryPart_Search (&settingsText, false, false);
#else
        err = ACAPI_LibPart_Search (&settingsText, false, false);
#endif
        if (err != NoError || settingsText.location == nullptr) {
            msg_rep ("ReadLibraryFile", "Cant find file " + fileName, err, APINULLGuid);
            return false;
        }
        // Открытие файла
        settingsText.location->ToPath (&locPath);
        delete settingsText.location;
        IO::Location fileLoc (locPath);
        IO::File file (fileLoc);
        UInt64 fSize = 0;
        file.GetDataLength (&fSize);
        char *buff = new char[fSize];
        if (buff == nullptr) {
            msg_rep ("ReadLibraryFile", "buff == nullptr" + fileName, err, APINULLGuid);
            return false;
        }
        err = file.Open (IO::File::ReadMode);
        if (err != NoError) {
            msg_rep ("ReadLibraryFile", "Cant read file " + fileName, err, APINULLGuid);
            delete[] buff;
            return false;
        }
        file.ReadBin (buff, (USize)fSize);
        file.Close ();
        std::istringstream stream (std::string (buff, fSize));
        GS::UniString separatorString = "";
        USize n_col = 0;
        while (std::getline (stream, lineStr)) {
            GS::Array<GS::UniString> lines;
            GSCharCode chcode = GetCharCode (lineStr);
            s = GS::UniString (lineStr.c_str (), chcode);
            if (separatorString.IsEmpty ()) {
                if (s.Count ("\t") > s.Count (";")) {
                    separatorString = "\t";
                } else {
                    separatorString = ";";
                }
            }
            s.Split (separatorString, GS::UniString::KeepEmptyParts, &lines);
            if (lines.IsEmpty ())
                continue;
            if (n_col < 2) {
                n_col = lines.GetSize ();
            }
            if (n_col != lines.GetSize ()) {
                msg_rep ("ReadLibraryFile", "Different n col " + fileName, err, APINULLGuid);
            } else {
                data.Push (lines);
            }
        }
        delete[] buff;
        return !data.IsEmpty ();
    }

    void SetParamValueFromCache (const GS::UniString &rawname, ParamValue &pvalue) {
        ParamValue paramFrom;
        if (!GetParamValueFromCache (rawname, paramFrom))
            return;
        paramFrom.fromGuid = pvalue.fromGuid;
        paramFrom.toQRCode = pvalue.toQRCode;
        pvalue = paramFrom;
    }

    bool isPropertyDefinitionRead () {
        auto &cache = PROPERTYCACHE ();
        if (!cache.isPropertyDefinitionRead_full)
            cache.ReadPropertyDefinition ();
        return cache.isPropertyDefinition_OK;
    }

    bool isAttributeRead () {
        auto &cache = PROPERTYCACHE ();
        if (!cache.isAttributeRead)
            cache.ReadAttribute ();
        return cache.isAttribute_OK;
    }

#if defined(AC_29)
    bool isMEPRead () {
        auto &cache = PROPERTYCACHE ();
        if (!cache.isMEPRead_full)
            cache.ReadMEP ();
        return cache.isMEP_OK;
    }
#endif

    GS::UniString GetLayerFromCache (const API_AttributeIndex &layerinx) {
        auto &cache = PROPERTYCACHE ();
        if (cache.isAttributeRead && cache.isAttribute_OK) {
#if defined(AC_27) || defined(AC_28) || defined(AC_29)
            GS::UniString rawName = "layer_inx_" + GS::UniString::Printf ("%d", layerinx.ToInt32_Deprecated ());
#else
            GS::UniString rawName = "layer_inx_" + GS::UniString::Printf ("%d", layerinx);
#endif
            if (const auto *ptr = cache.attrib.GetPtr (rawName)) {
                if (ptr->isValid) {
                    return ptr->val.uniStringValue;
                }
            }
        }
        API_Attribute layer = {};
        layer.header.typeID = API_LayerID;
        layer.header.index = layerinx;
        GSErrCode err = ACAPI_Attribute_Get (&layer);
        if (err != NoError) {
            msg_rep ("GetLayerFromCache", "ACAPI_Attribute_Get", err, APINULLGuid);
            return GS::UniString{};
        }
        return GS::UniString (layer.header.name);
    }

    bool GetGroupFromCache (const API_Guid &guid, API_PropertyGroup &group) {
        auto &cache = PROPERTYCACHE ();
        if (cache.isGroupPropertyRead && cache.isGroupProperty_OK) {
            if (const auto *ptr = cache.propertygroups.GetPtr (guid)) {
                group = *ptr;
                return true;
            }
        }
        if (!cache.isGroupPropertyRead_full && cache.isGroupProperty_OK) {
            if (const auto *ptr = cache.propertygroups.GetPtr (guid)) {
                group = *ptr;
                return true;
            }
            cache.isGroupPropertyRead = true;
            API_PropertyGroup group = {};
            group.guid = guid;
            GSErrCode err = ACAPI_Property_GetPropertyGroup (group);
            if (err != NoError) {
                msg_rep ("GetGroupFromCache", "ACAPI_Property_GetPropertyGroups", err, APINULLGuid);
                return false;
            }
            cache.isGroupProperty_OK = true;
        }
        if (const auto *ptr = cache.propertygroups.GetPtr (guid)) {
            group = *ptr;
            return true;
        }

#if defined(TESTING)
        DBprnt ("ERROR GetGroupFromCache (guid) ");
#endif
        return false;
    }

    GS::UniString GetGDLRawName (const GS::UniString &name) {
        auto &cache = PROPERTYCACHE ();
        if (const auto *ptr = cache.gdlparamname.GetPtr (name)) {
            return *ptr;
        }
        GS::UniString lowerName = name;
        lowerName.SetToLowerCase ();
        GS::UniString rawname = GDLNAMEPREFIX + lowerName + BRACEEND;
        cache.gdlparamname.Add (name, rawname);
        return rawname;
    }

    bool GetParamValueFromCache (const GS::UniString &rawname, ParamValue &pvalue) {
        short inx = ParamHelpers::GetTypeInxByRawnamePrefix (rawname);
        if (inx != PROPERTYTYPEINX && inx != GLOBTYPEINX && inx != ATTRIBTYPEINX && inx != INFOTYPEINX) {
            return false;
        }
        auto &cache = PROPERTYCACHE ();
        switch (inx) {
        case PROPERTYTYPEINX:
            if (!cache.isPropertyDefinitionRead_full)
                cache.ReadPropertyDefinition ();
            if (!cache.isPropertyDefinition_OK)
                return false;
            if (const auto *ptr = cache.property.GetPtr (rawname)) {
                pvalue = *ptr;
                return true;
            }
            return false;

        case GLOBTYPEINX:
            if (const auto *ptr = cache.glob.GetPtr (rawname)) {
                pvalue = *ptr;
                return true;
            }

            if (!cache.isGetGeoLocationRead)
                cache.ReadGetGeoLocation ();
            if (const auto *ptr = cache.glob.GetPtr (rawname)) {
                pvalue = *ptr;
                return true;
            }

            if (!cache.isSurveyPointTransformationRead)
                cache.ReadSurveyPointTransformation ();
            if (const auto *ptr = cache.glob.GetPtr (rawname)) {
                pvalue = *ptr;
                return true;
            }

            if (!cache.isPlaceSetsRead)
                cache.ReadPlaceSets ();
            if (const auto *ptr = cache.glob.GetPtr (rawname)) {
                pvalue = *ptr;
                return true;
            }

            if (!cache.isLocOriginRead)
                cache.ReadLocOrigin ();
            if (const auto *ptr = cache.glob.GetPtr (rawname)) {
                pvalue = *ptr;
                return true;
            }

            return false;

        case ATTRIBTYPEINX:
            if (!cache.isAttributeRead)
                cache.ReadAttribute ();
            if (!cache.isAttribute_OK)
                return false;

            if (const auto *ptr = cache.attrib.GetPtr (rawname)) {
                pvalue = *ptr;
                return true;
            }
            return false;

        case INFOTYPEINX:
            if (!cache.isInfoRead)
                cache.ReadInfo ();
            if (!cache.isInfo_OK)
                return false;

            if (const auto *ptr = cache.info.GetPtr (rawname)) {
                pvalue = *ptr;
                return true;
            }
            return false;

        default:
            return false;
        }
    }

    bool isCacheContainsParamValue (const GS::UniString &rawname) {
        short inx = ParamHelpers::GetTypeInxByRawnamePrefix (rawname);
        if (inx != PROPERTYTYPEINX && inx != GLOBTYPEINX && inx != ATTRIBTYPEINX && inx != INFOTYPEINX) {
            return false;
        }
        auto &cache = PROPERTYCACHE ();
        switch (inx) {
        case PROPERTYTYPEINX:
            if (!cache.isPropertyDefinitionRead_full)
                cache.ReadPropertyDefinition ();
            if (!cache.isPropertyDefinition_OK)
                return false;
            return cache.property.ContainsKey (rawname);

        case GLOBTYPEINX:
            if (cache.glob.ContainsKey (rawname))
                return true;

            if (!cache.isGetGeoLocationRead)
                cache.ReadGetGeoLocation ();
            if (!cache.isGetGeoLocation_OK)
                return false;
            if (cache.glob.ContainsKey (rawname))
                return true;

            if (!cache.isSurveyPointTransformationRead)
                cache.ReadSurveyPointTransformation ();
            if (!cache.isSurveyPointTransformation_OK)
                return false;
            if (cache.glob.ContainsKey (rawname))
                return true;

            if (!cache.isPlaceSetsRead)
                cache.ReadPlaceSets ();
            if (!cache.isPlaceSets_OK)
                return false;
            if (cache.glob.ContainsKey (rawname))
                return true;

            if (!cache.isLocOriginRead)
                cache.ReadLocOrigin ();
            if (!cache.isLocOrigin_OK)
                return false;
            return cache.glob.ContainsKey (rawname);

        case ATTRIBTYPEINX:
            if (!cache.isAttributeRead)
                cache.ReadAttribute ();
            if (!cache.isAttribute_OK)
                return false;
            return cache.attrib.ContainsKey (rawname);

        case INFOTYPEINX:
            if (!cache.isInfoRead)
                cache.ReadInfo ();
            if (!cache.isInfo_OK)
                return false;
            return cache.info.ContainsKey (rawname);

        default:
            return false;
        }
    }

#if defined(AC_29)
    bool GetMEPSystemGroup (MEPDicts &mepdict) {
        const ACAPI::Result<std::vector<ACAPI::MEP::UniqueID>> systemGroupIDs{ACAPI::MEP::GetSystemGroupIDs ()};
        if (systemGroupIDs.IsErr ())
            return false;

        for (const ACAPI::MEP::UniqueID &systemGroupID : systemGroupIDs.Unwrap ()) {
            const ACAPI::Result<ACAPI::MEP::SystemGroup> systemGroup{ACAPI::MEP::SystemGroup::Get (systemGroupID)};
            if (systemGroup.IsErr ())
                continue;
            const std::vector<ACAPI::MEP::UniqueID> systemIDs = systemGroup.Unwrap ().GetSystemIDs ();
            GS::Guid systemGroupguid = systemGroupID.GetGuid ();
            for (const ACAPI::MEP::UniqueID &systemID : systemIDs) {
                GS::Guid systemguid = systemID.GetGuid ();
                if (MEPDict *pPtr = mepdict.GetPtr (systemguid)) {
                    if (!pPtr->ContainsKey (systemGroupguid)) {
                        pPtr->Add (systemGroupguid, true);
                    }
                } else {
                    MEPDict newDict;
                    newDict.Add (systemGroupguid, true);
                    mepdict.Add (systemguid, std::move (newDict));
                }
            }
        }
        return !mepdict.IsEmpty ();
    }
#endif
    // --------------------------------------------------------------------
    // Получение списка глобальных переменных о местоположении проекта, солнца
    // --------------------------------------------------------------------
    bool GetGeoLocationToParamDict (ParamDictValue &propertyParams) {
#if defined(AC_22) || defined(AC_23) || defined(AC_24)
        return false;
#else
    #if defined(TESTING)
        DBprnt ("   GetGeoLocationToParamDict start");
    #endif
        GS::UniString name = "";
        GS::UniString rawName = "";
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
        name = "surveyPointPosition_x";
        rawName = GLOBNAMEPREFIX;
        rawName.Append (name.ToLowerCase ());
        rawName.Append (BRACEEND);
        pvalue.name = name;
        pvalue.rawName = rawName;
        ParamHelpers::ConvertDoubleToParamValue (pvalue, rawName, apiGeoLocation.surveyPointPosition.x);
        propertyParams.Add (rawName, pvalue);
        name = "surveyPointPosition_y";
        rawName = GLOBNAMEPREFIX;
        rawName.Append (name.ToLowerCase ());
        rawName.Append (BRACEEND);
        pvalue.name = name;
        pvalue.rawName = rawName;
        ParamHelpers::ConvertDoubleToParamValue (pvalue, rawName, apiGeoLocation.surveyPointPosition.y);
        propertyParams.Add (rawName, pvalue);
        name = "surveyPointPosition_z";
        rawName = GLOBNAMEPREFIX;
        rawName.Append (name.ToLowerCase ());
        rawName.Append (BRACEEND);
        pvalue.name = name;
        pvalue.rawName = rawName;
        ParamHelpers::ConvertDoubleToParamValue (pvalue, rawName, apiGeoLocation.surveyPointPosition.z);
        propertyParams.Add (rawName, pvalue);
        name = "eastings";
        rawName = GLOBNAMEPREFIX;
        rawName.Append (name.ToLowerCase ());
        rawName.Append (BRACEEND);
        pvalue.name = name;
        pvalue.rawName = rawName;
        ParamHelpers::ConvertDoubleToParamValue (pvalue, rawName, apiGeoLocation.geoReferenceData.eastings);
        propertyParams.Add (rawName, pvalue);
        name = "northings";
        rawName = GLOBNAMEPREFIX;
        rawName.Append (name.ToLowerCase ());
        rawName.Append (BRACEEND);
        pvalue.name = name;
        pvalue.rawName = rawName;
        ParamHelpers::ConvertDoubleToParamValue (pvalue, rawName, apiGeoLocation.geoReferenceData.northings);
        propertyParams.Add (rawName, pvalue);
        name = "orthogonalHeight";
        rawName = GLOBNAMEPREFIX;
        rawName.Append (name.ToLowerCase ());
        rawName.Append (BRACEEND);
        pvalue.name = name;
        pvalue.rawName = rawName;
        ParamHelpers::ConvertDoubleToParamValue (pvalue, rawName, apiGeoLocation.geoReferenceData.orthogonalHeight);
        propertyParams.Add (rawName, pvalue);
        name = "xAxisAbscissa";
        rawName = GLOBNAMEPREFIX;
        rawName.Append (name.ToLowerCase ());
        rawName.Append (BRACEEND);
        pvalue.name = name;
        pvalue.rawName = rawName;
        ParamHelpers::ConvertDoubleToParamValue (pvalue, rawName, apiGeoLocation.geoReferenceData.xAxisAbscissa);
        propertyParams.Add (rawName, pvalue);
        name = "xAxisOrdinate";
        rawName = GLOBNAMEPREFIX;
        rawName.Append (name.ToLowerCase ());
        rawName.Append (BRACEEND);
        pvalue.name = name;
        pvalue.rawName = rawName;
        ParamHelpers::ConvertDoubleToParamValue (pvalue, rawName, apiGeoLocation.geoReferenceData.xAxisOrdinate);
        propertyParams.Add (rawName, pvalue);
        name = "scale";
        rawName = GLOBNAMEPREFIX;
        rawName.Append (name.ToLowerCase ());
        rawName.Append (BRACEEND);
        pvalue.name = name;
        pvalue.rawName = rawName;
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
    bool GetPlaceSetsToParamDict (ParamDictValue &propertyParams) {
#if defined(TESTING)
        DBprnt ("   GetAllGlobToParamDict start");
#endif
        GS::UniString name = "";
        GS::UniString rawName = "";
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
        name = "GLOB_NORTH_DIR";
        rawName = GLOBNAMEPREFIX;
        rawName.Append (name.ToLowerCase ());
        rawName.Append (BRACEEND);
        pvalue.name = name;
        pvalue.rawName = rawName;
        ParamHelpers::ConvertDoubleToParamValue (
            pvalue, rawName, round ((placeInfo.north * 180 / PI) * 1000.0) / 1000.0);
        propertyParams.Add (rawName, pvalue);
        name = "GLOB_PROJECT_LONGITUDE";
        rawName = GLOBNAMEPREFIX;
        rawName.Append (name.ToLowerCase ());
        rawName.Append (BRACEEND);
        pvalue.name = name;
        pvalue.rawName = rawName;
        ParamHelpers::ConvertDoubleToParamValue (pvalue, rawName, placeInfo.longitude);
        propertyParams.Add (rawName, pvalue);
        name = "GLOB_PROJECT_LATITUDE";
        rawName = GLOBNAMEPREFIX;
        rawName.Append (name.ToLowerCase ());
        rawName.Append (BRACEEND);
        pvalue.name = name;
        pvalue.rawName = rawName;
        ParamHelpers::ConvertDoubleToParamValue (pvalue, rawName, placeInfo.latitude);
        propertyParams.Add (rawName, pvalue);
        name = "GLOB_PROJECT_ALTITUDE";
        rawName = GLOBNAMEPREFIX;
        rawName.Append (name.ToLowerCase ());
        rawName.Append (BRACEEND);
        pvalue.name = name;
        pvalue.rawName = rawName;
        ParamHelpers::ConvertDoubleToParamValue (pvalue, rawName, placeInfo.altitude);
        propertyParams.Add (rawName, pvalue);
        name = "GLOB_SUN_AZIMUTH";
        rawName = GLOBNAMEPREFIX;
        rawName.Append (name.ToLowerCase ());
        rawName.Append (BRACEEND);
        pvalue.name = name;
        pvalue.rawName = rawName;
        ParamHelpers::ConvertDoubleToParamValue (
            pvalue, rawName, round ((placeInfo.sunAngXY * 180 / PI) * 1000.0) / 1000.0);
        propertyParams.Add (rawName, pvalue);
        name = "GLOB_SUN_ALTITUDE";
        rawName = GLOBNAMEPREFIX;
        rawName.Append (name.ToLowerCase ());
        rawName.Append (BRACEEND);
        pvalue.name = name;
        pvalue.rawName = rawName;
        ParamHelpers::ConvertDoubleToParamValue (
            pvalue, rawName, round ((placeInfo.sunAngZ * 180 / PI) * 1000.0) / 1000.0);
        propertyParams.Add (rawName, pvalue);
#if defined(TESTING)
        DBprnt ("   GetAllGlobToParamDict end");
#endif
        return true;
    }

    // --------------------------------------------------------------------
    // Заполнение информации о локальном начале координат
    // --------------------------------------------------------------------
    bool GetLocOriginToParamDict (ParamDictValue &propertyParams) {
#if defined(TESTING)
        DBprnt ("   GetLocOriginToParamDict start");
#endif
        // Пользовательское начало
        API_Coord3D locOrigin = {};
        API_Coord offset = {};
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
        ParamValue pvalue = {};
        pvalue.val.formatstring = FormatStringFunc::ParseFormatString (DEFULTLEGHTFSTRING);
        pvalue.fromCoord = true;

        pvalue.name = "locOrigin_x";
        pvalue.rawName = GLOBNAMEPREFIX;
        pvalue.rawName.Append (pvalue.name.ToLowerCase ());
        pvalue.rawName.Append (BRACEEND);
        ParamHelpers::ConvertDoubleToParamValue (pvalue, EMPTYSTRING, locOrigin.x + offset.x);
        propertyParams.Add (pvalue.rawName, pvalue);

        pvalue.name = "locOrigin_y";
        pvalue.rawName = GLOBNAMEPREFIX;
        pvalue.rawName.Append (pvalue.name.ToLowerCase ());
        pvalue.rawName.Append (BRACEEND);
        ParamHelpers::ConvertDoubleToParamValue (pvalue, EMPTYSTRING, locOrigin.y + offset.y);
        propertyParams.Add (pvalue.rawName, pvalue);

        pvalue.name = "locOrigin_z";
        pvalue.rawName = GLOBNAMEPREFIX;
        pvalue.rawName.Append (pvalue.name.ToLowerCase ());
        pvalue.rawName.Append (BRACEEND);
        ParamHelpers::ConvertDoubleToParamValue (pvalue, EMPTYSTRING, locOrigin.z);
        propertyParams.Add (pvalue.rawName, pvalue);

        pvalue.name = "offsetOrigin_x";
        pvalue.rawName = GLOBNAMEPREFIX;
        pvalue.rawName.Append (pvalue.name.ToLowerCase ());
        pvalue.rawName.Append (BRACEEND);
        ParamHelpers::ConvertDoubleToParamValue (pvalue, EMPTYSTRING, offset.x);
        propertyParams.Add (pvalue.rawName, pvalue);

        pvalue.name = "offsetOrigin_y";
        pvalue.rawName = GLOBNAMEPREFIX;
        pvalue.rawName.Append (pvalue.name.ToLowerCase ());
        pvalue.rawName.Append (BRACEEND);
        ParamHelpers::ConvertDoubleToParamValue (pvalue, EMPTYSTRING, offset.y);
        propertyParams.Add (pvalue.rawName, pvalue);
#if defined(TESTING)
        DBprnt ("   GetLocOriginToParamDict end");
#endif
        return true;
    }

    // --------------------------------------------------------------------
    // Заполнение информации о проекте
    // --------------------------------------------------------------------
    bool GetAllInfoToParamDict (ParamDictValue &propertyParams) {
#if defined(TESTING)
        DBprnt ("   GetAllInfoToParamDict start");
#endif
        GS::Array<GS::ArrayFB<GS::UniString, 3>> autotexts = {};
        API_AutotextType type = APIAutoText_Custom;
        GSErrCode err = NoError;
#if defined(AC_27) || defined(AC_28) || defined(AC_29)
        err = ACAPI_AutoText_GetAutoTexts (&autotexts, type);
#else
        err = ACAPI_Goodies (APIAny_GetAutoTextsID, &autotexts, (void *)(GS::IntPtr)type);
#endif
        if (err != NoError) {
            msg_rep ("GetAllInfoToParamDict", "APIAny_GetAutoTextsID", err, APINULLGuid);
            return false;
        }
        GS::UniString rawName = "";
        for (UInt32 i = 0; i < autotexts.GetSize (); i++) {
            if (autotexts[i][0].Contains ("Addon_Dimens") && !autotexts[i][2].IsEmpty ()) {
                rawName = "{@info:addon_dimension_autotext}";
            } else {
                rawName = INFONAMEPREFIX;
                rawName.Append (autotexts[i][0].ToLowerCase ());
                rawName.Append (BRACEEND);
            }
            if (!propertyParams.ContainsKey (rawName)) {
                ParamValue pvalue;
                pvalue.name = autotexts[i][1];
                pvalue.rawName = rawName;
                pvalue.fromInfo = true;
                ParamHelpers::ConvertStringToParamValue (pvalue, rawName, autotexts[i][2]);
                propertyParams.Add (rawName, std::move (pvalue));
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
    bool GetAllAttributeToParamDict (ParamDictValue &propertyParams) {
#if defined(TESTING)
        DBprnt ("   GetAllAttributeToParamDict start");
#endif
        API_Attribute attrib = {};
        GSErrCode err = NoError;
        GS::Array<API_Attribute> attributes = {};
        GS::UniString rawName;
        GS::UniString attribname;
        err = ACAPI_Attribute_GetAttributesByType (API_LayerID, attributes);
        for (API_Attribute &attrib : attributes) {
            attribname.Clear ();
            attrib.header.uniStringNamePtr = &attribname;
            err = ACAPI_Attribute_Get (&attrib);
            if (err == NoError) {
                ParamValue pvalue = {};
                rawName = "layer_name_" + attribname;
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
                propertyParams.Add (pvalue.rawName, std::move (pvalue));
            } else {
                if (err == APIERR_DELETED)
                    err = NoError;
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
    bool GetAllPropertyDefinitionToParamDict (ParamDictValue &propertyParams) {
#if defined(TESTING)
        DBprnt ("   GetAllPropertyDefinitionToParamDict start");
#endif
        auto &cache = PROPERTYCACHE ();
        if (!cache.isGroupPropertyRead_full)
            cache.ReadGroupProperty ();
        if (!cache.isGroupPropertyRead_full)
            return false;
        GSErrCode err = NoError;
        // Созданим словарь с определением всех свойств
        GS::Array<API_PropertyDefinition> definitions = {};
        for (const auto &cIt : cache.propertygroups) {
#if defined(AC_28) || defined(AC_29)
            const API_PropertyGroup &group = cIt.value;
#else
            const API_PropertyGroup &group = *cIt.value;
#endif
            bool filter = true;
#if defined(AC_28) || defined(AC_29)
            GS::UniString strguid = APIGuidToString (group.guid);
            filter = (strguid.IsEqual ("3CF63E55-AA52-4AB4-B1C3-0920B2F352BF") ||
                      strguid.IsEqual ("6EE946D2-E840-4909-8EF1-F016AE905C52") ||
                      strguid.IsEqual ("BF31D3E0-A2B1-4543-A3DA-C1191D059FD8"));
// TODO Дописать Guid группы "GeneralElemProperties"
#else
            filter = (group.name.Contains ("Material") || group.name.IsEqual ("GeneralElemProperties"));
#endif
            if (group.groupType == API_PropertyCustomGroupType ||
                (group.groupType == API_PropertyStaticBuiltInGroupType && filter)) {
                definitions.Clear ();
                err = ACAPI_Property_GetPropertyDefinitions (group.guid, definitions);
                if (err != NoError)
                    msg_rep ("GetPropertyByName", "ACAPI_Property_GetPropertyDefinitions", err, APINULLGuid);
                if (err == NoError)
                    GetArrayPropertyDefinitionToParamDict (propertyParams, definitions);
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
    bool GetArrayPropertyDefinitionToParamDict (ParamDictValue &propertyParams,
                                                GS::Array<API_PropertyDefinition> &definitions) {
        if (definitions.IsEmpty ())
            return false;
        GS::UniString name = "";
        GS::UniString rawName = "";
        API_PropertyGroup group;
        bool flag_add = false;
        for (auto &definision : definitions) {
            if (!ParamHelpers::GetGroupFromCache (definision.groupGuid, group))
                continue;
            if (definision.description.Contains ("Sync_name")) {
                for (UInt32 inx = 0; inx < 20; inx++) {
                    GS::UniString strinx = GS::UniString::Printf ("%d", inx);
                    rawName = "{@property:sync_name";
                    rawName.Append (strinx);
                    rawName.Append (BRACEEND);
                    name = SYNCNAME;
                    name.Append (strinx);
                    if (!propertyParams.ContainsKey (rawName))
                        break;
                }
                definision.name = name;
                ParamValue pvalue = {};
                pvalue.rawName = rawName;
                pvalue.name = group.name;
                pvalue.name.Append (SLASH);
                pvalue.name.Append (definision.name);
                ParamHelpers::ConvertToParamValue (pvalue, definision);
                propertyParams.Add (pvalue.rawName, std::move (pvalue));
                flag_add = true;
                continue;
            }
#if defined(AC_28) || defined(AC_29)
            name = GetPropertyNameByGUID (definision.guid);
            if (name.IsEmpty ()) {
                name = group.name;
                name.Append (SLASH);
                name.Append (definision.name);
            }
#else
            name = group.name;
            name.Append (SLASH);
            name.Append (definision.name);
#endif
            rawName = PROPERTYNAMEPREFIX;
            rawName.Append (name.ToLowerCase ());
            rawName.Append (BRACEEND);
            if (ParamValue *pvaluePtr = propertyParams.GetPtr (rawName)) {
                if (pvaluePtr->definition.guid != definision.guid) {
                    if (!pvaluePtr->fromPropertyDefinition && !pvaluePtr->fromAttribDefinition) {
                        FormatString fstring = pvaluePtr->val.formatstring;
                        pvaluePtr->rawName = rawName;
                        pvaluePtr->name = name;
                        ParamHelpers::ConvertToParamValue (*pvaluePtr, definision);
                        if (!fstring.isEmpty)
                            pvaluePtr->val.formatstring = fstring;
                        flag_add = true;
                    }
                }
            } else {
                ParamValue pvalue;
                pvalue.rawName = rawName;
                pvalue.name = name;
                ParamHelpers::ConvertToParamValue (pvalue, definision);
                rawName = pvalue.rawName; // при конвертации могло измениться
                propertyParams.Add (rawName, std::move (pvalue));
                flag_add = true;
            }
        }
        return flag_add;
    }
} // namespace ParamHelpers

// -----------------------------------------------------------------------------
// Проверка языка Архикада. Для INT возвращает 1000
// -----------------------------------------------------------------------------
Int32 isEng () {
    auto &cache = PROPERTYCACHE ();
    if (!cache.isEng_OK)
        cache.ReadisEng ();
    return cache.isEng;
}

void AddUnreadGDLParams (const Int32 &libinx, const GS::UniString &rawname) {
    if (libinx < 1)
        return;
    auto &cache = PROPERTYCACHE ();
    bool isParamAdded = false;
    if (ParamDict *pPtr = cache.unreadedgdlparams.GetPtr (libinx)) {
        if (!pPtr->ContainsKey (rawname)) {
            pPtr->Add (rawname, true);
            isParamAdded = true;
        }
    } else {
        ParamDict p;
        p.Add (rawname, true);
        cache.unreadedgdlparams.Add (libinx, std::move (p));
        isParamAdded = true;
    }
#if defined(TESTING)
    if (isParamAdded) {
        DBprnt ("        Add skip GDL param", rawname);
    }
#endif
}

GS::UniString CountUnreadGDLParams () {
    GS::UniString out = "";
    Int32 clib = 0;
    Int32 cparam = 0;
    auto &cache = PROPERTYCACHE ();
    if (cache.unreadedgdlparams.IsEmpty ())
        return out;
    for (GS::HashTable<Int32, ParamDict>::PairIterator cIt = cache.unreadedgdlparams.EnumeratePairs (); cIt != NULL;
         ++cIt) {
#if defined(AC_28) || defined(AC_29)
        ParamDict &param = cIt->value;
#else
        ParamDict &param = *cIt->value;
#endif
        if (param.IsEmpty ())
            continue;
        clib += 1;
        cparam += param.GetSize ();
    }
    out = GS::UniString::Printf (" Skip %d params in %d linpart ", cparam, clib);
    return out;
}

bool IsUnreadGDLParams (const Int32 &libinx, const GS::UniString &rawname) {
    if (libinx < 1)
        return false;
    const auto &cache = PROPERTYCACHE ();
    if (const ParamDict *pPtr = cache.unreadedgdlparams.GetPtr (libinx)) {
        return pPtr->ContainsKey (rawname);
    }
    return false;
}

GS::UniString GetPropertyNameByGUID (const API_Guid &guid) {
    if (guid == APINULLGuid)
        return EMPTYSTRING;
    GS::UniString strguid = APIGuidToString (guid);
    if (strguid.IsEqual ("2E906CCE-9A42-4E49-AE45-193D0D709CC4"))
        return "BuildingMaterialProperties/Building Material CutFill";
    if (strguid.IsEqual ("FAF74D9D-3CD4-4A03-9840-A39DB757DB1C"))
        return "BuildingMaterialProperties/Building Material Density";
    if (strguid.IsEqual ("68947382-7220-449A-AE47-F6F8CB47DE49"))
        return "BuildingMaterialProperties/Building Material Description";
    if (strguid.IsEqual ("902756A0-71D1-402B-B639-640BA5837A95"))
        return "BuildingMaterialProperties/Building Material ID";
    if (strguid.IsEqual ("A01BCC22-D1FC-4CD8-AD34-95BBE73BDD5E"))
        return "BuildingMaterialProperties/Building Material Manufacturer";
    if (strguid.IsEqual ("294C063C-98D8-42B5-B2C1-C27DE7CAB756"))
        return "BuildingMaterialProperties/Building Material Thermal Conductivity";
    if (strguid.IsEqual ("F99C8A52-810A-4D01-A33A-AB5FDBA43A20"))
        return "BuildingMaterialProperties/Building Material Heat Capacity";
    if (strguid.IsEqual ("A01BCC22-D1FC-4CD8-AD34-95BBE73BDD5E"))
        return "BuildingMaterialProperties/Building Material Manufacturer";
    if (strguid.IsEqual ("A936C5CB-5126-4135-BD87-D2A46AEF5A07"))
        return "BuildingMaterialProperties/Building Material Name";
    return EMPTYSTRING;
}

// -----------------------------------------------------------------------------
// Получить полное имя свойства (включая имя группы)
// -----------------------------------------------------------------------------
GSErrCode GetPropertyFullName (const API_PropertyDefinition &definision, GS::UniString &name) {
    if (definision.groupGuid == APINULLGuid)
        return APIERR_BADID;
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
        API_PropertyGroup group = {};
        if (ParamHelpers::GetGroupFromCache (definision.groupGuid, group)) {
            name = group.name;
            name.Append (SLASH);
            name.Append (definision.name);
        } else {
            msg_rep ("GetPropertyFullName", "ACAPI_Property_GetPropertyGroup " + definision.name, error, APINULLGuid);
        }
    }
    return error;
}

// -----------------------------------------------------------------------------
//	Формат записи: ПЕРО_РАЗМЕРА - КРАТНОСТЬ_ММ, ПЕРО_ТЕКСТА_ИЗМЕНЁННОЕ, ФЛАГ_ИЗМЕНЕНИЯ_СОДЕРЖИМОГО, "ФОРМУЛА", либо
//					"Слой" - КРАТНОСТЬ_ММ, ПЕРО_ТЕКСТА_ИЗМЕНЁННОЕ, ФЛАГ_ИЗМЕНЕНИЯ_СОДЕРЖИМОГО,
//"ФОРМУЛА"
// -----------------------------------------------------------------------------
bool DimReadPref (DimRules &dimrules, const GS::UniString &autotext, bool &hasLayerNameInDimRules) {
    bool hasexpression = false;             // Нужно ли нам читать список свойств
    bool hasLayerNameInOneDimRules = false; // Нужно ли нам читать список свойств
    if (autotext.Contains (SEMICOLON)) {
        GS::Array<GS::UniString> partstring = {};
        StringSplt (autotext, SEMICOLON, partstring, true);
        for (const auto &part : partstring) {
            DimRule dimrule = {};
            if (DimParsePref (part, dimrule, hasexpression, hasLayerNameInOneDimRules)) {
                if (hasLayerNameInOneDimRules)
                    hasLayerNameInDimRules = true;
                GS::UniString kstr;
                if (dimrule.layer.IsEmpty ()) {
                    kstr = GS::UniString::Printf ("%d", dimrule.pen_original);
                } else {
                    kstr = dimrule.layer;
                }
                dimrules.Add (std::move (kstr), std::move (dimrule));
            }
        }
    } else {
        DimRule dimrule = {};
        if (DimParsePref (autotext, dimrule, hasexpression, hasLayerNameInOneDimRules)) {
            if (hasLayerNameInOneDimRules)
                hasLayerNameInDimRules = true;
            GS::UniString kstr = "";
            if (dimrule.layer.IsEmpty ()) {
                kstr = GS::UniString::Printf ("%d", dimrule.pen_original);
            } else {
                kstr = dimrule.layer;
            }
            dimrule.kstr = kstr;
            dimrules.Add (std::move (kstr), std::move (dimrule));
        }
    }
    return !dimrules.IsEmpty ();
}

// -----------------------------------------------------------------------------
// Обработка текста правила
// -----------------------------------------------------------------------------
bool DimParsePref (const GS::UniString &rawrule, DimRule &dimrule, bool &hasexpression, bool &hasLayerNameInDimRules) {
    if (rawrule.IsEmpty ())
        return false;
    if (!rawrule.Contains ("-"))
        return false;
    bool flag_find = false;
    GS::Array<GS::UniString> partstring_1 = {};
    if (StringSplt (rawrule, "-", partstring_1, false) == 2) {
        // Проверяем - что указано в правиле: слой или номер пера
        //  Слой указываем в кавычках, в regexp формате
        if (partstring_1[0].Contains (CHARDQUT)) {
            dimrule.layer = partstring_1[0];
            dimrule.layer.ReplaceAll (CHARDQUT, ' ');
            dimrule.layer.Trim ();
            hasLayerNameInDimRules = true;
        } else {
            try {
                dimrule.pen_original = std::stoi (partstring_1[0].ToCStr ().Get ());
            } catch (const std::exception &) {
                dimrule.pen_original = 0;
                return false;
            }
        }
        partstring_1[1].SetToLowerCase ();
        if (partstring_1[1].Contains ("deletewall")) {
            dimrule.flag_change = true;
            dimrule.flag_deletewall = true;
            dimrule.pen_rounded = dimrule.pen_original;
            partstring_1[1].ReplaceAll ("deletewall", EMPTYSTRING);
            flag_find = true;
        }
        if (partstring_1[1].Contains ("resettext")) {
            dimrule.flag_change = true;
            dimrule.flag_reset = true;
            dimrule.pen_rounded = dimrule.pen_original;
            partstring_1[1].ReplaceAll ("resettext", EMPTYSTRING);
            flag_find = true;
        }
        if (partstring_1[1].Contains ("checkcustom")) {
            dimrule.flag_change = false;
            dimrule.flag_custom = true;
            partstring_1[1].ReplaceAll ("checkcustom", EMPTYSTRING);
            flag_find = true;
        }
        if (partstring_1[1].Contains ("classic")) {
            dimrule.classic_round_mode = true;
            partstring_1[1].ReplaceAll ("classicround", EMPTYSTRING);
            partstring_1[1].ReplaceAll ("classic", EMPTYSTRING);
        }
        if (!partstring_1[1].Contains (COMMA))
            return flag_find;
        GS::Array<GS::UniString> partstring_2 = {};
        if (StringSplt (partstring_1[1], COMMA, partstring_2, true) > 1) {
            if (!partstring_2[0].IsEmpty ()) {
                flag_find = true;
                try {
                    dimrule.round_value = std::stoi (partstring_2[0].ToCStr ().Get ());
                } catch (const std::exception &) {
                    dimrule.round_value = 0;
                    flag_find = false;
                }
            }
            if (!partstring_2[1].IsEmpty ()) {
                flag_find = true;
                try {
                    dimrule.pen_rounded = std::stoi (partstring_2[1].ToCStr ().Get ());
                } catch (const std::exception &) {
                    dimrule.pen_rounded = 0;
                    flag_find = false;
                }
            }
            if (partstring_2.GetSize () < 3)
                return flag_find;
            for (UInt32 k = 2; k < partstring_2.GetSize (); k++) {
                if (partstring_2[k].IsEmpty ())
                    continue;
                if (partstring_2[k].Contains (BRACESTART) && partstring_2[k].Contains (BRACEEND)) {
                    ParamDictValue paramDict = {};
                    GS::UniString expression = partstring_2[k];
                    expression.ReplaceAll ("<MeasuredValue>", "{MeasuredValue}");
                    ParamHelpers::ParseParamName (expression, paramDict);
                    dimrule.paramDict = std::move (paramDict);
                    dimrule.expression = std::move (expression);
                    if (!hasexpression)
                        hasexpression = !paramDict.IsEmpty ();
                } else {
                    if (k == 2) {
                        flag_find = true;
                        short flag_change;
                        try {
                            flag_change = std::stoi (partstring_2[k].ToCStr ().Get ());
                        } catch (const std::exception &) {
                            flag_change = 0;
                            flag_find = false;
                        }
                        dimrule.flag_change = (flag_change > 0);
                    }
                }
            }
        }
    }
    return flag_find;
}
