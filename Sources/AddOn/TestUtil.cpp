//------------ kuvbur 2022 ------------
#ifdef TESTING
    #include "ACAPinc.h"

    #include "api_headers/APIEnvir.h"

    #include "dialogs/CommandHelpers.hpp"
    #include "Helpers.hpp"
    #include "Propertycache.hpp"
    #include "Sync.hpp"
    #include "TestFunc.hpp"
    #include "TestKit.hpp"

namespace TestFunc {

    void TestGetTextLineLength (GS::UniString &var) {
        GSErrCode err = NoError;
        GS::UniString fontname = "Arial";
        double fontsize = 2.5;
        short font_inx = 0;
        double width = 0.0;
        API_TextLinePars tlp = {};
    #ifdef ServerMainVers_2700
        API_FontType font;
        BNZeroMemory (&font, sizeof (API_FontType));
        font.head.index = 0;
        font.head.uniStringNamePtr = &fontname;
        err = ACAPI_Font_SearchFont (font);
        font_inx = font.head.index;
    #else
        API_Attribute attrib = {};
        attrib.header.typeID = API_FontID;
        attrib.header.index = 0;
        attrib.header.uniStringNamePtr = &fontname;
        err = ACAPI_Attribute_Search (&attrib.header);
        font_inx = attrib.header.index;
    #endif
        font_inx = 135;
        tlp.drvScaleCorr = false;
        tlp.index = 0;
        tlp.wantsLongestIndex = false;
        tlp.lineUniStr = &var;
        tlp.wFace = APIFace_Plain;
        tlp.wFont = font_inx;
        tlp.wSize = fontsize;
        tlp.wSlant = PI / 2.0;
    #ifdef ServerMainVers_2700
        err = ACAPI_Element_GetTextLineLength (&tlp, &width);
    #else
        err = ACAPI_Goodies (APIAny_GetTextLineLengthID, &tlp, &width);
    #endif
    #ifdef TESTING
        DBtest (width > 0.00001, "TestGetTextLineLength");
    #endif
    }

    void DumpAllBuiltInProperties () {
        // GS::Array<API_PropertyGroup> groups;
        // ACAPI_Property_GetPropertyGroups (groups);
        // for (const API_PropertyGroup& group : groups) {
        //     GS::Array<API_PropertyDefinition> definitions;
        //     ACAPI_Property_GetPropertyDefinitions (group.guid, definitions);
        //     GS::UniString report_ =
        //         "======" + group.name + "\t" +
        //         APIGuidToString (group.guid);
        //     ACAPI_WriteReport (report_, false);
        //     for (const API_PropertyDefinition& definition : definitions) {
        //         if (definition.definitionType != API_PropertyStaticBuiltInDefinitionType) {
        //             continue;
        //         }
        //         GS::UniString report =
        //             group.name + "\t" +
        //             definition.name + "\t" +
        //             APIGuidToString (definition.guid);
        //         ACAPI_WriteReport (report, false);
        //     }
        // }
    }

    void ResetSyncPropertyArray (GS::Array<API_Guid> guidArray) {
        if (guidArray.IsEmpty ())
            return;
        for (UInt32 j = 0; j < guidArray.GetSize (); j++) {
            ResetSyncPropertyOne (guidArray[j]);
        }
    #if defined(TESTING)
        DBprnt ("TEST", "ResetSyncPropertyArray");
    #endif
    }

    void ResetSyncPropertyOne (const API_Guid &elemGuid) {
        GSErrCode err = NoError;
        GS::Array<API_Property> propertywrite;
        ResetSyncPropertyOne (elemGuid, propertywrite);
        if (propertywrite.IsEmpty ())
            return;
        const Int32 iseng = ID_ADDON_STRINGS + isEng ();
        GS::UniString undoString = RSGetIndString (iseng, UndoSyncId, ACAPI_GetOwnResModule ());
        err = ACAPI_CallUndoableCommand (
            undoString, [&] () -> GSErrCode { return ACAPI_Element_SetProperties (elemGuid, propertywrite); });
        if (err != NoError)
            msg_rep ("ResetSyncProperty", "ACAPI_Element_SetProperties", err, elemGuid);
    }

    void ResetSyncPropertyOne (const API_Guid &elemGuid, GS::Array<API_Property> &propertywrite) {
        GSErrCode err = NoError;
        GS::Array<API_PropertyDefinition> definitions;
        err = ACAPI_Element_GetPropertyDefinitions (elemGuid, API_PropertyDefinitionFilter_UserDefined, definitions);
        if (err != NoError) {
            msg_rep ("ResetSyncProperty", "ACAPI_Element_GetPropertyDefinitions", err, elemGuid);
            return;
        }
        for (UInt32 i = 0; i < definitions.GetSize (); i++) {
            if (!definitions[i].description.IsEmpty ()) {
                if (definitions[i].description.Contains ("Sync_from")) {
                    API_Property property = {};
                    const GSErrCode errGet = ACAPI_Element_GetPropertyValue (elemGuid, definitions[i].guid, property);
                    if (errGet == NoError) {
                        if (!property.isDefault) {
                            property.isDefault = true;
                            propertywrite.Push (property);
                        }
                    } else {
                        msg_rep ("ResetSyncProperty", "ACAPI_Element_GetPropertyValue", errGet, elemGuid);
                    }
                }
            }
        }
    }

} // namespace TestFunc
#endif
