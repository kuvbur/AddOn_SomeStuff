//------------ kuvbur 2022 ------------
#include "APIEnvir.h"
#include "CommonFunction.hpp"
#include "qrcodegen.hpp"
#include <APIdefs_Environment.h>
#include <bitset>
#include <cmath>
#include <limits>
#include <math.h>
static const Int32 VersionId = 49;

GS::UniString GetDBName (API_DatabaseInfo& databaseInfo)
{
    GS::UniString рname = "";
    switch (databaseInfo.typeID) {
        case (APIWind_FloorPlanID):
            рname = "FloorPlan";
            break;
        case (APIWind_SectionID):
            рname = "Section";
            break;											// section window type
        case (APIWind_DetailID):
            рname = "Detail";
            break;											// detail window type
        case (APIWind_3DModelID):
            рname = "3DModel";
            break;											// 3D model window type
        case (APIWind_LayoutID):
            рname = "Layout";
            break;											// layout window type
        case (APIWind_DrawingID):
            рname = "Drawing";
            break;											// drawing's database type
        case (APIWind_MyTextID):
            рname = "MyText";
            break;											// custom text window type
        case (APIWind_MyDrawID):
            рname = "MyDraw";
            break;											// custom draw window type
        case (APIWind_MasterLayoutID):
            рname = "MasterLayout";
            break;										// master layout window type
        case (APIWind_ElevationID):
            рname = "Elevation";
            break;										// elevation window type
        case (APIWind_InteriorElevationID):
            рname = "InteriorElevation";
            break;								// interior elevation window type
        case (APIWind_WorksheetID):
            рname = "Worksheet";
            break;										// worksheet window type
        case (APIWind_DocumentFrom3DID):
            рname = "DocumentFrom3D";
            break;									// 3D Document window type
        default:
            break;
    }
    рname = рname + " ";
    рname = рname + databaseInfo.title;
    return рname;
}

Stories GetStories ()
{
    Stories stories;
    API_StoryInfo storyInfo = {};
    #if defined AC_27 || defined AC_28
    GSErrCode err = ACAPI_ProjectSetting_GetStorySettings (&storyInfo);
    #else
    GSErrCode err = ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo, nullptr);
    #endif
    if (err == NoError) {
        const short numberOfStories = storyInfo.lastStory - storyInfo.firstStory + 1;
        for (short i = 0; i < numberOfStories; ++i) {
            stories.PushNew ((*storyInfo.data)[i].index, (*storyInfo.data)[i].level);
        }
        BMKillHandle ((GSHandle*) &storyInfo.data);
    }
    return stories;
}

GS::Pair<short, double> GetFloorIndexAndOffset (const double zPos, const Stories& stories)
{
    if (stories.IsEmpty ()) {
        return { 0, zPos };
    }

    const Story* storyPtr = &stories[0];
    for (const auto& story : stories) {
        if (story.level > zPos) {
            break;
        }
        storyPtr = &story;
    }
    return { storyPtr->index, zPos - storyPtr->level };
}

double GetOffsetFromStory (const double zPos, const short floorInd, const Stories& stories)
{
    if (stories.IsEmpty ()) {
        return zPos;
    }
    const Story* storyPtr = &stories[0];
    for (const auto& story : stories) {
        if (story.index == floorInd) {
            return zPos - story.level;
        }
        storyPtr = &story;
    }
    return zPos;
}

double GetzPos (const double bottomOffset, const short floorInd, const Stories& stories)
{
    if (stories.IsEmpty ()) {
        return 0;
    }
    const Story* storyPtr = &stories[0];
    for (const auto& story : stories) {
        if (story.index == floorInd) {
            return story.level + bottomOffset;
        }
        storyPtr = &story;
    }
    return 0;
}

GS::UniString TextToQRCode (GS::UniString& text, int error_lvl)
{
    GS::UniString qr_txt = "";
    if (text.IsEmpty ()) return qr_txt;
    qrcodegen::QrCode::Ecc lvl = qrcodegen::QrCode::Ecc::HIGH;
    switch (error_lvl) {
        case 1:
            lvl = qrcodegen::QrCode::Ecc::LOW;
            break;
        case 2:
            lvl = qrcodegen::QrCode::Ecc::MEDIUM;
            break;
        case 3:
            lvl = qrcodegen::QrCode::Ecc::QUARTILE;
            break;
        case 4:
            lvl = qrcodegen::QrCode::Ecc::HIGH;
            break;
        default:
            break;
    }
    const qrcodegen::QrCode qr = qrcodegen::QrCode::encodeText (text.ToCStr ().Get (), lvl);
    qr_txt = GS::UniString::Printf ("%d;", qr.getSize ());
    for (int x = 0; x < qr.getSize (); x++) {
        for (int y = 0; y < qr.getSize (); y += 4) {
            std::bitset<4> b1 (0);
            b1.set (0, qr.getModule (x, y));
            b1.set (1, qr.getModule (x, y + 1));
            b1.set (2, qr.getModule (x, y + 2));
            b1.set (3, qr.getModule (x, y + 3));
            int d = b1.to_ulong ();
            if (d < 10) {
                qr_txt = qr_txt + GS::UniString::Printf ("%d", d);
            } else {
                if (d == 10) qr_txt.Append ("A");
                if (d == 11) qr_txt.Append ("B");
                if (d == 12) qr_txt.Append ("C");
                if (d == 13) qr_txt.Append ("D");
                if (d == 14) qr_txt.Append ("E");
                if (d == 15) qr_txt.Append ("F");
            }
        }
        qr_txt.Append ("=");
    }
    return qr_txt;
}

GS::UniString TextToQRCode (GS::UniString& text)
{
    GS::UniString qr_txt = "";
    if (text.IsEmpty ()) return qr_txt;
    try {
        qr_txt = TextToQRCode (text, 4);
    } catch (std::length_error) {
        try {
            qr_txt = TextToQRCode (text, 3);
        } catch (std::length_error) {
            try {
                qr_txt = TextToQRCode (text, 2);
            } catch (std::length_error) {
                try {
                    qr_txt = TextToQRCode (text, 1);
                } catch (std::length_error) {
                    qr_txt = "ERROR: data Too long";
                }
            }
        }
    }
    return qr_txt;
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


void DBprnt (GS::UniString msg, GS::UniString reportString)
{
    #if defined(TESTING)
    if (msg.Contains ("err") || msg.Contains ("ERROR") || reportString.Contains ("err") || reportString.Contains ("ERROR")) {
        #if defined(AC_22)
        DBPrintf ("== ERROR == ");
        #else
        DBPrint ("== ERROR == ");
        #endif
    }
    #if defined(AC_22)
    DBPrintf ("== SMSTF == ");
    #else
    DBPrint ("== SMSTF == ");
    #endif
    std::string var_str = msg.ToCStr (0, MaxUSize, GChCode).Get ();
    #if defined(AC_22)
    DBPrintf (var_str.c_str ());
    #else
    DBPrint (var_str.c_str ());
    #endif
    if (!reportString.IsEmpty ()) {
        std::string reportString_str = reportString.ToCStr (0, MaxUSize, GChCode).Get ();
        #if defined(AC_22)
        DBPrintf (" : ");
        DBPrintf (reportString_str.c_str ());
        #else
        DBPrint (" : ");
        DBPrint (reportString_str.c_str ());
        #endif
    }
    #if defined(AC_22)
    DBPrintf ("\n");
    #else
    DBPrint ("\n");
    #endif
    #else
    UNUSED_VARIABLE (msg);
    UNUSED_VARIABLE (reportString);
    #endif
}

void DBtest (bool usl, GS::UniString reportString, bool asserton)
{
    #if defined(TESTING)
    if (usl) {
        DBprnt (reportString, "ok");
    } else {
        DBprnt ("=== ERROR IN TEST ===", reportString);
    }
    if (asserton) assert (usl);
    #else
    UNUSED_VARIABLE (usl);
    UNUSED_VARIABLE (asserton);
    UNUSED_VARIABLE (reportString);
    #endif
}

void DBtest (GS::UniString a, GS::UniString b, GS::UniString reportString, bool asserton)
{
    #if defined(TESTING)
    GS::UniString out = a + " = " + b;
    if (a.IsEqual (b)) {
        reportString = "test " + reportString + " ok";
        DBprnt (out, reportString);
    } else {
        out = "=== ERROR IN TEST === " + out;
        DBprnt (out, reportString);
    }
    if (asserton) assert (a.IsEqual (b));
    #else
    UNUSED_VARIABLE (a);
    UNUSED_VARIABLE (b);
    UNUSED_VARIABLE (asserton);
    UNUSED_VARIABLE (reportString);
    #endif
}

void DBtest (double a, double b, GS::UniString reportString, bool asserton)
{
    #if defined(TESTING)
    GS::UniString out = GS::UniString::Printf ("%d = %d", a, b);
    if (is_equal (a, b)) {
        reportString = "test " + reportString + " ok";
        DBprnt (out, reportString);
    } else {
        out = "=== ERROR IN TEST === " + out;
        DBprnt (out, reportString);
    }
    if (asserton) assert (is_equal (a, b));
    #else
    UNUSED_VARIABLE (a);
    UNUSED_VARIABLE (b);
    UNUSED_VARIABLE (asserton);
    UNUSED_VARIABLE (reportString);
    #endif
}


// -----------------------------------------------------------------------------
// Проверка языка Архикада. Для INT возвращает 1000
// -----------------------------------------------------------------------------
Int32 isEng ()
{
    #ifdef EXTNDVERSION
    return 0;
    #endif
    GSErrCode err = NoError;
    API_ServerApplicationInfo AppInfo;
    #if defined(AC_27) || defined(AC_28)
    err = ACAPI_AddOnIdentification_Application (&AppInfo);
    #else
    err = ACAPI_Environment (APIEnv_ApplicationID, &AppInfo);
    #endif // AC_27
    if (err != NoError) return 0;
    if (!AppInfo.language.IsEqual ("RUS")) return 1000;
    return 0;
}

// -----------------------------------------------------------------------------
// Вывод сообщения в отчёт
// -----------------------------------------------------------------------------
void msg_rep (const GS::UniString& modulename, const GS::UniString& reportString, const GSErrCode& err, const API_Guid& elemGuid, bool show)
{
    GS::UniString error_type = "";
    if (err != NoError) {
        switch (err) {
            case APIERR_GENERAL:
                error_type = "APIERR_GENERAL - General error code";
                break;
            case APIERR_MEMFULL:
                error_type = "APIERR_MEMFULL Insufficient memory.";
                break;
            case APIERR_CANCEL:
                error_type = "APIERR_CANCEL The operation has been canceled by the user, in case of a long process.";
                break;
            case APIERR_BADID:
                error_type = "APIERR_BADID The passed identifier is not a valid one, or valid, but not proper for the given operation.";
                break;
            case APIERR_BADINDEX:
                error_type = "APIERR_BADINDEX The passed index is out of range.";
                break;
            case APIERR_BADNAME:
                error_type = "APIERR_BADNAME The passed name is not proper or not found in the existing list.";
                break;
            case APIERR_BADPARS:
                error_type = "APIERR_BADPARS The passed parameters are inconsistent.";
                break;
            case APIERR_BADPOLY:
                error_type = "APIERR_BADPOLY The passed polygon cannot be interpreted.";
                break;
            case APIERR_BADDATABASE:
                error_type = "APIERR_BADDATABASE The command cannot be executed on the current database.";
                break;
            case APIERR_BADWINDOW:
                error_type = "APIERR_BADWINDOW The command cannot be executed while the current window is active.";
                break;
            case APIERR_BADKEYCODE:
                error_type = "APIERR_BADKEYCODE The key code cannot be found in the listing database.";
                break;
            case APIERR_BADPLATFORMSIGN:
                error_type = "APIERR_BADPLATFORMSIGN The passed platform sign is not valid";
                break;
            case APIERR_BADPLANE:
                error_type = "APIERR_BADPLANE The plane equation is incorrect.";
                break;
            case APIERR_BADUSERID:
                error_type = "APIERR_BADUSERID The passed user ID(TeamWork client) is not valid.";
                break;
            case APIERR_BADVALUE:
                error_type = "APIERR_BADVALUE The passed autotext value is not valid";
                break;
            case APIERR_BADELEMENTTYPE:
                error_type = "APIERR_BADELEMENTTYPE The function cannot be applied to the passed element type";
                break;
            case APIERR_IRREGULARPOLY:
                error_type = "APIERR_IRREGULARPOLY The passed polygon or polyline is irregular.See API_RegularizedPoly.";
                break;
            case APIERR_BADEXPRESSION:
                error_type = "The passed expression string is syntactically incorrect.";
                break;
            case -2130313100:
                error_type = "The passed guid is invalid or valid, but not proper for the given operation..";
                break;
            case APIERR_NO3D:
                error_type = "There is no 3D information assigned to the passed element.";
                break;
            case APIERR_NOMORE:
                error_type = "No more database items can be returned.";
                break;
            case APIERR_NOPLAN:
                error_type = "There is no open project.The operation cannot be executed without an open project.";
                break;
            case APIERR_NOLIB:
                error_type = "No library was loaded.The operation cannot be executed without a loaded library.Can be returned by ACAPI_LibPart_Create.";
                break;
            case APIERR_NOLIBSECT:
                error_type = "The requested LibPart section is not found.";
                break;
            case APIERR_NOSEL:
                error_type = "No selection.The operation cannot be executed without any element selected.";
                break;
            case APIERR_NOTEDITABLE:
                error_type = "The referenced element is not editable.";
                break;
            case APIERR_NOTSUBTYPEOF:
                error_type = "The specified first library part unique ID does not refer to a subtype of the second unique ID.See APIAny_CheckLibPartSubtypeOfID.";
                break;
            case APIERR_NOTEQUALMAIN:
                error_type = "The main GUID parts of the specified two library part unique IDs are not equal.See APIAny_CompareLibPartUnIdsID.";
                break;
            case APIERR_NOTEQUALREVISION:
                error_type = "The main GUID parts of the specified two library part unique IDs are equal but their revision IDs differ.See APIAny_CompareLibPartUnIdsID.";
                break;
            case APIERR_NOTEAMWORKPROJECT:
                error_type = "There is no open project, or not in Teamwork mode.";
                break;
            case APIERR_NOUSERDATA:
                error_type = "Attempt to get user data assigned to an element, but there isn’t any.";
                break;
            case APIERR_MOREUSER:
                error_type = "The user data cannot be assigned to the element, since there is no free storage block avaliable.";
                break;
            case APIERR_LINKEXIST:
                error_type = "The link already exists.";
                break;
            case APIERR_LINKNOTEXIST:
                error_type = "The link doesn’t exist.";
                break;
            case APIERR_WINDEXIST:
                error_type = "The window to be opened already exists.";
                break;
            case APIERR_WINDNOTEXIST:
                error_type = "The referenced window does not exist.";
                break;
            case APIERR_UNDOEMPTY:
                error_type = "No undoable entry has got into the opened undo operation.";
                break;
            case APIERR_REFERENCEEXIST:
                error_type = "The reference already exists.";
                break;
            case APIERR_NAMEALREADYUSED:
                error_type = "The resource must have a unique name but the specified one is already taken.";
                break;
            case APIERR_ATTREXIST:
                error_type = "The attribute already exists.";
                break;
            case APIERR_DELETED:
                error_type = "Reference to a deleted, purged or non - existent database item.";
                break;
            case APIERR_LOCKEDLAY:
                error_type = "The referenced layer is locked.";
                break;
            case APIERR_HIDDENLAY:
                error_type = "The referenced layer is hidden.";
                break;
            case APIERR_INVALFLOOR:
                error_type = "The passed floor index is out of range.";
                break;
            case APIERR_NOTMINE:
                error_type = "The database item is not in the user’s workspace.";
                break;
            case APIERR_NOACCESSRIGHT:
                error_type = "Can’t access / create / modify / delete an item in a teamwork server.";
                break;
                #if defined(AC_22) || defined(AC_23)
            case APIERR_BADPROPERTYFORELEM:
                error_type = "The property for the passed element or attribute is not available.";
                break;
            case APIERR_BADCLASSIFICATIONFORELEM:
                error_type = "Can’t set the classification for the passed element or attribute.";
                break;
                #else
            case APIERR_BADPROPERTY:
                error_type = "The property for the passed element or attribute is not available.";
                break;
            case APIERR_BADCLASSIFICATION:
                error_type = "Can’t set the classification for the passed element or attribute.";
                break;
                #endif // AC_22 or AC_23
            case APIERR_MODULNOTINSTALLED:
                error_type = "The referenced add - on is not installed.For more details see the Communication Manager.";
                break;
            case APIERR_MODULCMDMINE:
                error_type = "The target add - on is the caller add - on.For more details see the Communication Manager.";
                break;
            case APIERR_MODULCMDNOTSUPPORTED:
                error_type = "The referenced command is not supported by the target add - on.For more details see the Communication Manager.";
                break;
            case APIERR_MODULCMDVERSNOTSUPPORTED:
                error_type = "The requested command version is newer than the version of the command that the target add - on can support.For more details see the Communication Manager.";
                break;
            case APIERR_NOMODULEDATA:
                error_type = "No custom data section is saved into the project file identified by the add - on’s unique ID.See ACAPI_ModulData_Get and ACAPI_ModulData_GetInfo.";
                break;
            case APIERR_PAROVERLAP:
                error_type = "Two or more paragraphs are overlapped.The end offset of one is greater than the beginner offset of the next one.";
                break;
            case APIERR_PARMISSING:
                error_type = "Number of paragraphs – the size of paragraphs handle – is zero.";
                break;
            case APIERR_PAROVERFLOW:
                error_type = "Paragraph end offset is run over the content length.";
                break;
            case APIERR_PARIMPLICIT:
                error_type = "The content string contains line end character(CR) at invalid position(inside the paragraph range).";
                break;
            case APIERR_RUNOVERLAP:
                error_type = "Two or more runs are overlapped.The end offset of one is greater than the beginner offset of the next one.";
                break;
            case APIERR_RUNMISSING:
                error_type = "Number of runs – the size of run pointer – is zero.";
                break;
            case APIERR_RUNOVERFLOW:
                error_type = "Run end offset is run over the content length.";
                break;
            case APIERR_RUNIMPLICIT:
                error_type = "The beginner offset of one is greater than the end offset of the previous one.";
                break;
            case APIERR_RUNPROTECTED:
                error_type = "Attempted to overwrite a protected text run(not used yet).";
                break;
            case APIERR_EOLOVERLAP:
                error_type = "The EOL array is not a monotonous ascendant sequence.";
                break;
            case APIERR_TABOVERLAP:
                error_type = "The tabulator array is not a monotonous ascendant sequence.";
                break;
            case APIERR_NOTINIT:
                error_type = "The command needs initialization by an other API call.";
                break;
            case APIERR_NESTING:
                error_type = "The API function is not reentrant.Nesting occurred.";
                break;
            case APIERR_NOTSUPPORTED:
                error_type = "The command is not supported by the server application.It is not environment dependent.The server application cannot execute the command generally.";
                break;
            case APIERR_REFUSEDCMD:
                error_type = "The passed identifier is not subject to the operation.";
                break;
            case APIERR_REFUSEDPAR:
                error_type = "The command cannot be executed with the passed parameters.";
                break;
            case APIERR_READONLY:
                error_type = "The specified location is read - only.Can be returned by ACAPI_LibPart_Create.";
                break;
            case APIERR_SERVICEFAILED:
                error_type = "The invoked Teamwork service has failed.";
                break;
            case APIERR_COMMANDFAILED:
                error_type = "The invoked undoable command threw an exception.Can be returned by ACAPI_CallUndoableCommand.";
                break;
            case APIERR_NEEDSUNDOSCOPE:
                error_type = "The called command should be encapsulated in a ACAPI_CallUndoableCommand scope.";
                break;
            case APIERR_MISSINGCODE:
                error_type = "The function is not implemented yet.";
                break;
            case APIERR_MISSINGDEF:
                error_type = "The originating library part file is missing.The document name is still filled.";
                break;
            default:
                break;
        }
    }
    if (elemGuid != APINULLGuid) {
        error_type = "GUID element: " + APIGuid2GSGuid (elemGuid).ToUniString () + " " + error_type;
        API_Elem_Head elem_head = {}; BNZeroMemory (&elem_head, sizeof (API_Elem_Head));
        elem_head.guid = elemGuid;
        if (ACAPI_Element_GetHeader (&elem_head) == NoError) {
            GS::UniString elemName;

            #if defined(AC_27) || defined(AC_28)
            if (ACAPI_Element_GetElemTypeName (elem_head.type, elemName) == NoError) {
                #else
            #ifdef AC_26
            if (ACAPI_Goodies_GetElemTypeName (elem_head.type, elemName) == NoError) {
                #else
            if (ACAPI_Goodies (APIAny_GetElemTypeNameID, (void*) elem_head.typeID, &elemName) == NoError) {
                #endif
                #endif
                error_type = error_type + " type:" + elemName;
            }
            API_Attribute layer;
            BNZeroMemory (&layer, sizeof (API_Attribute));
            layer.header.typeID = API_LayerID;
            layer.header.index = elem_head.layer;
            if (ACAPI_Attribute_Get (&layer) == NoError) error_type = error_type + " layer:" + layer.header.name;
        }
    }
    GS::UniString msg = modulename + ": " + reportString;
    if (err != NoError) msg = "!! ERROR !!" + msg;
    if (!show) msg = msg + " " + error_type;
    GS::UniString version = RSGetIndString (ID_ADDON_STRINGS, VersionId, ACAPI_GetOwnResModule ());
    msg = version + msg + "\n";
    ACAPI_WriteReport (msg, false);
    if (show) ACAPI_WriteReport (msg, show);
    if (err != NoError) {
        msg = "== SMSTF ERROR ==" + msg;
    }
    #if defined(TESTING)
    DBprnt (msg);
    #endif
}


// --------------------------------------------------------------------
// Отмечает заданный пункт активным/неактивным
// --------------------------------------------------------------------
void	MenuItemCheckAC (short itemInd, bool checked)
{
    API_MenuItemRef itemRef;
    GSFlags         itemFlags;

    BNZeroMemory (&itemRef, sizeof (API_MenuItemRef));
    itemRef.menuResID = ID_ADDON_MENU;
    itemRef.itemIndex = itemInd;

    itemFlags = 0;
    #if defined(AC_27) || defined(AC_28)
    ACAPI_MenuItem_GetMenuItemFlags (&itemRef, &itemFlags);
    #else
    ACAPI_Interface (APIIo_GetMenuItemFlagsID, &itemRef, &itemFlags);
    #endif
    if (checked)
        itemFlags |= API_MenuItemChecked;
    else
        itemFlags &= ~API_MenuItemChecked;
    #if defined(AC_27) || defined(AC_28)
    ACAPI_MenuItem_SetMenuItemFlags (&itemRef, &itemFlags);
    #else
    ACAPI_Interface (APIIo_SetMenuItemFlagsID, &itemRef, &itemFlags);
    #endif
    return;
}

// -----------------------------------------------------------------------------
// Получить массив Guid выбранных элементов
// Версия без чтения настроек
// -----------------------------------------------------------------------------
GS::Array<API_Guid>	GetSelectedElements2 (bool assertIfNoSel /* = true*/, bool onlyEditable /*= true*/)
{
    GSErrCode            err;
    API_SelectionInfo    selectionInfo;
    GS::UniString errorString = "Empty";
    #ifdef AC_22
    API_Neig** selNeigs;
    #else
    GS::Array<API_Neig>  selNeigs;
    #endif
    err = ACAPI_Selection_Get (&selectionInfo, &selNeigs, onlyEditable);
    BMKillHandle ((GSHandle*) &selectionInfo.marquee.coords);
    if (err == APIERR_NOSEL || selectionInfo.typeID == API_SelEmpty) {
        if (assertIfNoSel) {
            DGAlert (DG_ERROR, "Error", errorString, "", "Ok");
        }
    }
    if (err != NoError) {
        #ifdef AC_22
        BMKillHandle ((GSHandle*) &selNeigs);
        #endif // AC_22
        return GS::Array<API_Guid> ();
    }
    GS::Array<API_Guid> guidArray;
    #ifdef AC_22
    USize nSel = BMGetHandleSize ((GSHandle) selNeigs) / sizeof (API_Neig);
    for (USize i = 0; i < nSel; i++) {
        guidArray.Push ((*selNeigs)[i].guid);
    }
    BMKillHandle ((GSHandle*) &selNeigs);
    #else
    for (const API_Neig& neig : selNeigs) {

        // Получаем список связанных элементов
        guidArray.Push (neig.guid);
    }
    #endif // AC_22
    return guidArray;
}

// -----------------------------------------------------------------------------
// Вызов функции для выбранных элементов
//	(функция должна принимать в качетве аргумента API_Guid
// Версия без чтения настроек
// -----------------------------------------------------------------------------
void CallOnSelectedElem2 (void (*function)(const API_Guid&), bool assertIfNoSel /* = true*/, bool onlyEditable /* = true*/, GS::UniString & funcname)
{
    GS::Array<API_Guid> guidArray = GetSelectedElements2 (assertIfNoSel, onlyEditable);
    if (!guidArray.IsEmpty ()) {
        long time_start = clock ();
        GS::UniString subtitle ("working...");
        GS::Int32 nPhase = 1;
        #if defined(AC_27) || defined(AC_28)
        bool showPercent = true;
        Int32 maxval = guidArray.GetSize ();
        ACAPI_ProcessWindow_InitProcessWindow (&funcname, &nPhase);
        #else
        ACAPI_Interface (APIIo_InitProcessWindowID, &funcname, &nPhase);
        #endif
        for (UInt32 i = 0; i < guidArray.GetSize (); i++) {
            #if defined(AC_27) || defined(AC_28)
            if (i % 10 == 0) ACAPI_ProcessWindow_SetNextProcessPhase (&subtitle, &maxval, &showPercent);
            #else
            if (i % 10 == 0) ACAPI_Interface (APIIo_SetNextProcessPhaseID, &subtitle, &i);
            #endif
            function (guidArray[i]);
            #if defined(AC_27) || defined(AC_28)
            if (ACAPI_ProcessWindow_IsProcessCanceled ()) return;
            #else
            if (ACAPI_Interface (APIIo_IsProcessCanceledID, nullptr, nullptr)) return;
            #endif
        }
        long time_end = clock ();
        GS::UniString time = GS::UniString::Printf (" %d ms", (time_end - time_start) / 1000);
        GS::UniString intString = GS::UniString::Printf (" %d qty", guidArray.GetSize ());
        msg_rep (funcname + " Selected", intString + time, NoError, APINULLGuid);
        #if defined(AC_27) || defined(AC_28)
        ACAPI_ProcessWindow_CloseProcessWindow ();
        #else
        ACAPI_Interface (APIIo_CloseProcessWindowID, nullptr, nullptr);
        #endif
    } else if (!assertIfNoSel) {
        function (APINULLGuid);
    }
}

// -----------------------------------------------------------------------------
// Получение типа объекта по его API_Guid
// -----------------------------------------------------------------------------
GSErrCode GetTypeByGUID (const API_Guid & elemGuid, API_ElemTypeID & elementType)
{
    GSErrCode		err = NoError;
    API_Elem_Head elem_head = {};
    BNZeroMemory (&elem_head, sizeof (API_Elem_Head));
    elem_head.guid = elemGuid;
    err = ACAPI_Element_GetHeader (&elem_head);
    if (err != NoError) {
        msg_rep ("GetTypeByGUID", "", err, elemGuid);
        return err;
    }
    elementType = GetElemTypeID (elem_head);
    return err;
}

#if defined AC_26 || defined AC_27 || defined AC_28
// -----------------------------------------------------------------------------
// Получение названия типа элемента
// -----------------------------------------------------------------------------
bool GetElementTypeString (API_ElemType elemType, char* elemStr)
{
    GS::UniString	ustr;
    GSErrCode	err = NoError;
    #if defined(AC_27) || defined(AC_28)
    err = ACAPI_Element_GetElemTypeName (elemType, ustr);
    #else
    err = ACAPI_Goodies_GetElemTypeName (elemType, ustr);
    #endif
    if (err == NoError) {
        CHTruncate (ustr.ToCStr (), elemStr, ELEMSTR_LEN - 1);
        return true;
    }
    return false;
}
#else
// -----------------------------------------------------------------------------
// Получение названия типа элемента
// -----------------------------------------------------------------------------
bool GetElementTypeString (API_ElemTypeID typeID, char* elemStr)
{
    GS::UniString	ustr;
    GSErrCode	err = ACAPI_Goodies (APIAny_GetElemTypeNameID, (void*) typeID, &ustr);
    if (err == NoError) {
        CHTruncate (ustr.ToCStr (), elemStr, ELEMSTR_LEN - 1);
        return true;
    }
    return false;
}
#endif // !AC_26

// -----------------------------------------------------------------------------
// Получить полное имя свойства (включая имя группы)
// -----------------------------------------------------------------------------
GSErrCode GetPropertyFullName (const API_PropertyDefinition & definision, GS::UniString & name)
{
    if (definision.groupGuid == APINULLGuid) return APIERR_BADID;
    GSErrCode error = NoError;

    if (definision.name.Contains ("ync_name")) {
        name = definision.name;
    } else {
        #if defined(AC_28)
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
        group.guid = definision.groupGuid;
        error = ACAPI_Property_GetPropertyGroup (group);
        if (error == NoError) {
            name = group.name;
            name.Append ("/");
            name.Append (definision.name);
        } else {
            msg_rep ("GetPropertyFullName", "ACAPI_Property_GetPropertyGroup " + definision.name, error, APINULLGuid);
        }
    }
    return error;
}

// --------------------------------------------------------------------
// Проверка наличия дробной части, возвращает ЛОЖЬ если дробная часть есть
// --------------------------------------------------------------------
bool check_accuracy (double val, double tolerance)
{
    if (std::isinf (val) || std::isnan (val)) return true;
    val = std::fabs (val * 1000.0);
    if (val < std::numeric_limits<double>::epsilon ()) return true;
    double reciprocal = std::round ((1 / tolerance)); // Коэффицент домножения для заданной точности
    double val_round = std::round (val * reciprocal) / reciprocal; // Приведённое к заданной точности значение
    if (val_round < std::numeric_limits<double>::epsilon () && val>tolerance) {
        return false;
    }
    double val_correct1 = std::fabs (val_round - std::round (val_round));
    double val_correct2 = std::fabs (val_round - std::floor (val_round));
    bool bval1 = val_correct1 < tolerance;
    bool bval2 = val_correct2 < tolerance;
    return bval1 && bval2;
}

// --------------------------------------------------------------------
// Сравнение double c учётом точности
// --------------------------------------------------------------------
bool is_equal (double x, double y)
{
    return std::fabs (x - y) < std::numeric_limits<double>::epsilon ();
}

// --------------------------------------------------------------------
// Содержит ли значения элементиз списка игнорируемых
// --------------------------------------------------------------------
bool CheckIgnoreVal (const std::string & ignoreval, const GS::UniString & val)
{
    GS::UniString unignoreval = GS::UniString (ignoreval.c_str (), GChCode);
    return CheckIgnoreVal (unignoreval, val);
}

bool CheckIgnoreVal (const GS::UniString & ignoreval, const GS::UniString & val)
{
    if (ignoreval.IsEmpty ()) return false;
    if ((ignoreval.ToLowerCase () == "empty" || ignoreval.ToLowerCase () == u8"пусто") && val.GetLength () < 1) {
        return true;
    }
    if (val == ignoreval) {
        return true;
    }
    return false;
}

// --------------------------------------------------------------------
// Содержит ли значения элементиз списка игнорируемых
// --------------------------------------------------------------------
bool CheckIgnoreVal (const GS::Array<GS::UniString>&ignorevals, const GS::UniString & val)
{
    if (ignorevals.GetSize () > 0) {
        for (UInt32 i = 0; i < ignorevals.GetSize (); i++) {
            if (CheckIgnoreVal (ignorevals[i], val)) return true;
        }
    }
    return false;
}

// --------------------------------------------------------------------
// Перевод строки в число
// --------------------------------------------------------------------
bool UniStringToDouble (const GS::UniString & var, double& x)
{
    if (var.IsEmpty ()) return false;
    GS::UniString var_clear = var;
    var_clear.Trim ();
    var_clear.ReplaceAll (",", ".");
    GSCharCode chcode = GetCharCode (var);
    std::string var_str = var_clear.ToCStr (0, MaxUSize, chcode).Get ();
    int n = sscanf (var_str.c_str (), "%lf", &x);
    if (n <= 0) {
        var_clear.ReplaceAll (".", ",");
        var_str = var_clear.ToCStr (0, MaxUSize, chcode).Get ();
        n = sscanf (var_str.c_str (), "%lf", &x);
    }
    return n > 0;
}

// --------------------------------------------------------------------
// Округлить целое n вверх до ближайшего целого числа, кратного k
// --------------------------------------------------------------------
Int32 ceil_mod (Int32 n, Int32 k)
{
    if (!k) return 0;
    if (!n) return 0;
    Int32 tmp = abs (n % k);
    if (tmp) n += (n > -1 ? (abs (k) - tmp) : (tmp));
    return n;
}

// --------------------------------------------------------------------
// Округлить целое n до ближайшего целого числа, кратного k
// --------------------------------------------------------------------
Int32 ceil_mod_classic (Int32 n, Int32 k)
{
    if (!k) return 0;
    if (!n) return 0;
    double n_real = n / 1.0;
    double k_real = k / 1.0;
    double param_real = round (n_real / k_real) * k;
    return (GS::Int32) param_real;
}
// --------------------------------------------------------------------
// Перевод метров, заданных типом double в мм Int32
// --------------------------------------------------------------------
Int32 DoubleM2IntMM (const double& value)
{
    double param_real = round (value * 1000.0) / 1000.0;
    if (value - param_real > 0.001) param_real += 0.001;
    param_real = param_real * 1000.0;
    Int32 param_int = ceil_mod ((GS::Int32) param_real, 1);
    return param_int;
}

// -----------------------------------------------------------------------------
// Замена \n на перенос строки
// -----------------------------------------------------------------------------
void ReplaceCR (GS::UniString & val, bool clear)
{
    GS::UniString p = "\\n";
    if (val.Contains (p)) {
        if (!clear) {
            for (UInt32 i = 0; i < val.Count (p); i++) {
                UIndex inx = val.FindFirst (p);
                val.ReplaceFirst (p, "");
                val.SetChar (inx, CharCR);
            }
        } else {
            val.ReplaceAll (p, "");
        }
    }
}

// -----------------------------------------------------------------------------
// Дополнение строки заданным количеством пробелов или табуляций
// -----------------------------------------------------------------------------
void GetNumSymbSpase (GS::UniString & outstring, GS::UniChar symb, char charrepl)
{

    //Ищем указание длины строки
    Int32 stringlen = 0;
    GS::UniString part = "";
    if (outstring.Contains (symb)) {
        part = outstring.GetSubstring (symb, ' ', 0);
        if (!part.IsEmpty () && part.GetLength () < 4)
            stringlen = std::atoi (part.ToCStr ());
        if (stringlen > 0) part = symb + part;
    }
    if (stringlen > 0) {
        Int32 modlen = outstring.GetLength () - part.GetLength () - 1;
        Int32 addspace = stringlen - modlen;
        if (modlen > stringlen) {
            addspace = modlen % stringlen;
        }
        outstring.ReplaceAll (part + ' ', GS::UniString::Printf ("%s", std::string (addspace, charrepl).c_str ()));
    }
}

// -----------------------------------------------------------------------------
// Замена символов \\TAB и др. на юникод
// -----------------------------------------------------------------------------
void ReplaceSymbSpase (GS::UniString & outstring)
{
    GetNumSymbSpase (outstring, '~', ' ');
    GetNumSymbSpase (outstring, '@', CharTAB);
    outstring.ReplaceAll ("\\TAB", u8"\u0009");
    outstring.ReplaceAll ("\\CRLF", u8"\u000D\u000A");
    outstring.ReplaceAll ("\\CR", u8"\u000D");
    outstring.ReplaceAll ("\\LF", u8"\u000A");
    outstring.ReplaceAll ("\\PS", u8"\u2029");
    outstring.ReplaceAll ("\\LS", u8"\u2028");
    outstring.ReplaceAll ("\\NEL", u8"\u0085");
    outstring.ReplaceAll ("\\NL", u8"\u2424");
}


short GetFontIndex (GS::UniString & fontname)
{
    GSErrCode err = NoError;
    short inx = 0;
    #if defined(AC_27) || defined(AC_28)
    API_FontType font; BNZeroMemory (&font, sizeof (API_FontType));
    font.head.index = 0;
    font.head.uniStringNamePtr = &fontname;
    err = ACAPI_Font_SearchFont (font);
    inx = font.head.index;
    #else
    API_Attribute attrib; BNZeroMemory (&attrib, sizeof (API_Attribute));
    attrib.header.typeID = API_FontID;
    attrib.header.index = 0;
    attrib.header.uniStringNamePtr = &fontname;
    err = ACAPI_Attribute_Search (&attrib.header);
    inx = attrib.header.index;
    #endif
    return inx;
}

double GetTextWidth (short& font, double& fontsize, GS::UniString & var)
{
    GSErrCode err = NoError;
    double width = 0.0;
    API_TextLinePars tlp; BNZeroMemory (&tlp, sizeof (API_TextLinePars));
    tlp.drvScaleCorr = false;
    tlp.index = 0;
    tlp.wantsLongestIndex = false;
    tlp.lineUniStr = &var;
    tlp.wFace = APIFace_Plain;
    tlp.wFont = font;
    tlp.wSize = fontsize;
    tlp.wSlant = PI / 2.0;
    #if defined(AC_27) || defined(AC_28)
    err = ACAPI_Element_GetTextLineLength (&tlp, &width);
    #else
    err = ACAPI_Goodies (APIAny_GetTextLineLengthID, &tlp, &width);
    #endif
    if (err != NoError || width < 0.001) {
        #if defined(TESTING)
        DBprnt ("GetTextWidth err", "zero width text = ~" + var + "~");
        #endif 
    }
    return width;
}

GS::Array<GS::UniString> DelimTextLine (short& font, double& fontsize, double& width, GS::UniString & var, GS::UniString & no_breake_space, GS::UniString & narow_space)
{
    GS::Array<GS::UniString> str;
    if (var.IsEmpty ()) return str;
    double width_space = 0;
    double width_in = GetTextWidth (font, fontsize, var);
    if (fabs (width_in - width) < 0.01) {
        str.PushNew (var);
        return str;
    }
    var.ReplaceAll (" ", no_breake_space);
    width_space = GetTextWidth (font, fontsize, narow_space);
    if (width_in < width) {
        Int32 addspace = (Int32) ((width - width_in) / width_space);
        if (fabs (addspace * width_space - width + width_in) > 0.01) addspace -= 1;
        GS::UniString addspace_txt = var;
        if (addspace > 0) {
            for (Int32 j = 0; j < addspace; j++) {
                addspace_txt.Append (narow_space);
            }
        }
        str.Push (addspace_txt);
        return str;
    }
    GS::Array<GS::UniString> parts;
    UInt32 npart = StringSplt (var, no_breake_space, parts);
    if (npart == 1) npart = StringSplt (var, ",", parts);
    if (npart == 1) npart = StringSplt (var, ".", parts);
    if (npart == 1) npart = StringSplt (var, ")", parts);
    if (npart == 1) npart = StringSplt (var, ":", parts);
    if (npart == 1) npart = StringSplt (var, "-", parts);
    if (npart == 1) {
        str.PushNew (var);
        return str;
    };
    GS::UniString out = "";
    GS::UniString old = "";
    double width_old = 0; Int32 addspace = 0;
    for (UInt32 i = 0; i < npart; i++) {
        if (out.IsEmpty ()) {
            out = parts.Get (i);
        } else {
            out.Append (no_breake_space);
            out.Append (parts.Get (i));
        }
        width_in = GetTextWidth (font, fontsize, out);
        if (width_in > width && !old.IsEmpty ()) {
            addspace = (Int32) ((width - width_old) / width_space);
            if (fabs (addspace * width_space - width + width_old) > 0.01) addspace -= 1;
            if (addspace > 0) {
                for (Int32 j = 0; j < addspace; j++) {
                    old.Append (narow_space);
                }
            }
            str.Push (old);
            out = parts.Get (i);
        }
        if (i == npart - 1) {
            width_in = GetTextWidth (font, fontsize, out);
            addspace = (Int32) ((width - width_in) / width_space);
            if (fabs (addspace * width_space - width + width_in) > 0.01) addspace -= 1;
            if (addspace > 0) {
                for (Int32 j = 0; j < addspace; j++) {
                    out.Append (narow_space);
                }
            }
            str.Push (out);
        }
        old = out;
        width_old = width_in;
    }
    return str;
}

// -----------------------------------------------------------------------------
// Проверка статуса и получение ID пользователя Teamwork
// -----------------------------------------------------------------------------
GSErrCode IsTeamwork (bool& isteamwork, short& userid)
{
    isteamwork = false;
    userid = 0;
    API_ProjectInfo projectInfo = {};
    GSErrCode err = NoError;
    #if defined(AC_27) || defined(AC_28)
    err = ACAPI_ProjectOperation_Project (&projectInfo);
    #else
    err = ACAPI_Environment (APIEnv_ProjectID, &projectInfo);
    #endif
    if (err == NoError) {
        isteamwork = projectInfo.teamwork;
        userid = projectInfo.userId;
    }
    return err;
}

// -----------------------------------------------------------------------------
// Вычисление выражений, заключённых в < >
// Что не может вычислить - заменит на пустоту
// -----------------------------------------------------------------------------
bool EvalExpression (GS::UniString & unistring_expression)
{
    if (unistring_expression.IsEmpty ()) return false;
    if (!unistring_expression.Contains (char_formula_start) && !unistring_expression.Contains (char_formula_end)) return false;
    GS::UniString part = "";
    GS::UniString part_clean = "";
    GS::UniString stringformat = "";
    GS::UniString rezult_txt = "";
    FormatString fstring;
    bool flag_change = true;
    // Определение правильного разделителя для расчётов
    GS::UniString delim = ".";
    GS::UniString baddelim = ",";
    GS::UniString delim_test = GS::UniString::Printf ("%.3f", 3.1456);
    if (delim_test.Contains (baddelim)) {
        #if defined(TESTING)
        DBprnt ("EvalExpression", "delimetr change");
        #endif
        baddelim = ".";
        delim = ",";
    }
    GS::UniString string_to_find = "";
    GSCharCode chcode = GetCharCode (unistring_expression);
    while (unistring_expression.Contains (char_formula_start) && unistring_expression.Contains (char_formula_end) && flag_change) {
        GS::UniString expression_old = unistring_expression;
        part = unistring_expression.GetSubstring (char_formula_start, char_formula_end, 0);
        // Ищем строку-формат
        stringformat.Clear ();
        fstring = FormatStringFunc::GetFormatStringFromFormula (unistring_expression, part, stringformat);
        typedef double T;
        typedef exprtk::expression<T> expression_t;
        typedef exprtk::parser<T> parser_t;
        part_clean = part;
        if (part_clean.Contains (baddelim)) part_clean.ReplaceAll (baddelim, delim);
        std::string expression_string (part_clean.ToCStr (0, MaxUSize, chcode).Get ());
        expression_t expression;
        parser_t parser;
        parser.compile (expression_string, expression);
        const T result = expression.value ();
        rezult_txt.Clear ();
        if (!std::isnan (result)) rezult_txt = FormatStringFunc::NumToString (result, fstring);
        #if defined(TESTING)
        if (std::isnan (result)) {
            DBprnt ("err Formula is nan", part_clean);
        }
        #endif
        string_to_find.Clear ();
        string_to_find.Append (char_formula_start);
        string_to_find.Append (part);
        string_to_find.Append (char_formula_end);
        string_to_find.Append (stringformat);
        unistring_expression.ReplaceAll (string_to_find, rezult_txt);
        if (expression_old.IsEqual (unistring_expression)) flag_change = false;
    }
    return (!unistring_expression.IsEmpty ());
}

// -----------------------------------------------------------------------------
// Toggle a checked menu item
// -----------------------------------------------------------------------------
bool MenuInvertItemMark (short menuResID, short itemIndex)
{
    API_MenuItemRef		itemRef;
    GSFlags				itemFlags;
    BNZeroMemory (&itemRef, sizeof (API_MenuItemRef));
    itemRef.menuResID = menuResID;
    itemRef.itemIndex = itemIndex;
    itemFlags = 0;
    #if defined(AC_27) || defined(AC_28)
    ACAPI_MenuItem_GetMenuItemFlags (&itemRef, &itemFlags);
    #else
    ACAPI_Interface (APIIo_GetMenuItemFlagsID, &itemRef, &itemFlags);
    #endif
    if ((itemFlags & API_MenuItemChecked) == 0)
        itemFlags |= API_MenuItemChecked;
    else
        itemFlags &= ~API_MenuItemChecked;
    #if defined(AC_27) || defined(AC_28)
    ACAPI_MenuItem_SetMenuItemFlags (&itemRef, &itemFlags);
    #else
    ACAPI_Interface (APIIo_SetMenuItemFlagsID, &itemRef, &itemFlags);
    #endif
    return (bool) ((itemFlags & API_MenuItemChecked) != 0);
}

// -----------------------------------------------------------------------------
// Возвращает уникальные вхождения текста
// -----------------------------------------------------------------------------
GS::UniString StringUnic (const GS::UniString & instring, const GS::UniString & delim)
{
    if (!instring.Contains (delim)) return instring;
    GS::Array<GS::UniString> partstring;
    GS::UniString outsting = "";
    UInt32 n = StringSpltUnic (instring, delim, partstring);
    for (UInt32 i = 0; i < n; i++) {
        outsting.Append (partstring[i]);
        if (i < n - 1) outsting.Append (delim);
    }
    return outsting;
}

// -----------------------------------------------------------------------------
// Возвращает уникальные вхождения текста
// -----------------------------------------------------------------------------
UInt32 StringSpltUnic (const GS::UniString & instring, const GS::UniString & delim, GS::Array<GS::UniString>&partstring)
{
    if (!instring.Contains (delim)) {
        partstring.Push (instring);
        return 1;
    }
    bool findecode = true;
    GSCharCode chcode = GetCharCode (instring, findecode);
    GSCharCode chcode_ = chcode;
    GS::Array<GS::UniString> tpartstring;
    UInt32 n = StringSplt (instring, delim, tpartstring);
    std::map<std::string, GSCharCode, doj::alphanum_less<std::string> > unic = {};
    for (UInt32 i = 0; i < n; i++) {
        if (!findecode) chcode_ = GetCharCode (tpartstring[i]);
        std::string s = tpartstring[i].ToCStr (0, MaxUSize, chcode_).Get ();
        unic[s] = chcode_;
    }
    UInt32 nout = 0;
    for (std::map<std::string, GSCharCode, doj::alphanum_less<std::string> >::iterator k = unic.begin (); k != unic.end (); ++k) {
        std::string s = k->first;
        chcode_ = k->second;
        GS::UniString unis = GS::UniString (s.c_str (), chcode_);
        partstring.Push (unis);
        nout = nout + 1;
    }
    return nout;
}

GSCharCode GetCharCode (const GS::UniString & instring)
{
    bool findecode = true;
    return GetCharCode (instring, findecode);
}

GSCharCode GetCharCode (const GS::UniString & instring, bool& findecode)
{
    findecode = true;
    #ifdef EXTNDVERSION
    return CC_Cyrillic;
    #endif
    if (ProbeCharCode (instring, CC_Cyrillic)) return CC_Cyrillic;
    if (ProbeCharCode (instring, CC_Korean)) return CC_Korean;
    if (ProbeCharCode (instring, CC_WestEuropean)) return CC_WestEuropean;
    if (ProbeCharCode (instring, CC_EastEuropean)) return CC_EastEuropean;
    if (ProbeCharCode (instring, CC_Greek)) return CC_Greek;
    if (ProbeCharCode (instring, CC_Turkish)) return CC_Turkish;
    if (ProbeCharCode (instring, CC_Hebrew)) return CC_Hebrew;
    if (ProbeCharCode (instring, CC_Arabic)) return CC_Arabic;
    if (ProbeCharCode (instring, CC_Thai)) return CC_Thai;
    if (ProbeCharCode (instring, CC_Japanese)) return CC_Japanese;
    if (ProbeCharCode (instring, CC_TradChinese)) return CC_TradChinese;
    if (ProbeCharCode (instring, CC_SimpChinese)) return CC_SimpChinese;
    if (ProbeCharCode (instring, CC_Symbol)) return CC_Symbol;
    findecode = false;
    return CC_Cyrillic;
}

bool ProbeCharCode (const GS::UniString & instring, GSCharCode chcode)
{
    std::string s = instring.ToCStr (0, MaxUSize, chcode).Get ();
    GS::UniString unis = GS::UniString (s.c_str (), chcode);
    bool b = unis.IsEqual (instring);
    return b;
}


// -----------------------------------------------------------------------------
// Делит строку по разделителю, возвращает кол-во частей
// -----------------------------------------------------------------------------
UInt32 StringSplt (const GS::UniString & instring, const GS::UniString & delim, GS::Array<GS::UniString>&partstring)
{
    if (!instring.Contains (delim)) {
        partstring.Push (instring);
        return 1;
    }
    GS::Array<GS::UniString> parts = {};
    GS::UniString tinstring = instring;
    UInt32 npart = instring.Split (delim, &parts);
    UInt32 n = 0;
    GS::UniString part = "";
    for (UInt32 i = 0; i < npart; i++) {
        part = parts.Get (i);
        if (!part.IsEmpty ()) {
            part.Trim ('\r');
            part.Trim ('\n');
            part.Trim ();
            if (!part.IsEmpty ()) {
                partstring.Push (part);
                n += 1;
            }
        }
    }
    return n;
}

// -----------------------------------------------------------------------------
// Делит строку по разделителю, возвращает кол-во частей
// Записывает в массив только части, содержащие строку filter
// -----------------------------------------------------------------------------
UInt32 StringSplt (const GS::UniString & instring, const GS::UniString & delim, GS::Array<GS::UniString>&partstring, const GS::UniString & filter)
{
    if (!instring.Contains (delim) || !instring.Contains (filter)) {
        partstring.Push (instring);
        return 1;
    }
    GS::Array<GS::UniString> parts = {};
    UInt32 n = 0;
    UInt32 npart = StringSplt (instring, delim, parts);
    for (UInt32 i = 0; i < npart; i++) {
        if (parts[i].Contains (filter)) {
            partstring.Push (parts.Get (i));
            n += 1;
        }
    }
    return n;
}


// -----------------------------------------------------------------------------
// Возвращает elemType и elemGuid для корректного чтение параметров элементов навесной стены
// -----------------------------------------------------------------------------
void GetGDLParametersHead (const API_Element & element, const API_Elem_Head & elem_head, API_ElemTypeID & elemType, API_Guid & elemGuid)
{
    API_ElemTypeID elemtype_ = GetElemTypeID (elem_head);
    switch (elemtype_) {
        case API_CurtainWallPanelID:
            elemGuid = element.cwPanel.symbolID;
            elemType = API_ObjectID;
            break;
        case API_CurtainWallJunctionID:
            elemGuid = element.cwJunction.symbolID;
            elemType = API_ObjectID;
            break;
        case API_CurtainWallAccessoryID:
            elemGuid = element.cwAccessory.symbolID;
            elemType = API_ObjectID;
            break;
        default:
            UNUSED_VARIABLE (element);
            elemGuid = elem_head.guid;
            elemType = elemtype_;
            break;
    }
    return;
}

// -----------------------------------------------------------------------------
// Возвращает список параметров API_AddParType из memo
// -----------------------------------------------------------------------------
GSErrCode GetGDLParametersFromMemo (const API_Guid & elemGuid, API_AddParType * *&params)
{
    API_ElementMemo	memo = {};
    BNZeroMemory (&memo, sizeof (API_ElementMemo));
    GSErrCode err = ACAPI_Element_GetMemo (elemGuid, &memo, APIMemoMask_AddPars);
    params = memo.params;
    if (err != NoError) {
        msg_rep ("GetGDLParametersFromMemo", "ACAPI_Element_GetMemo", err, elemGuid);
        return err;
    }
    if (memo.params == nullptr) {
        msg_rep ("GetGDLParametersFromMemo", "ACAPI_Element_GetMemo", err, elemGuid);
    }
    return err;
}

// -----------------------------------------------------------------------------
// Возвращает список параметров API_AddParType
// -----------------------------------------------------------------------------
GSErrCode GetGDLParameters (const API_ElemTypeID & elemType, const API_Guid & elemGuid, API_AddParType * *&params)
{
    return GetGDLParametersFromMemo (elemGuid, params);
    /*   GSErrCode	err = NoError;
       API_ParamOwnerType	apiOwner = {};
       API_GetParamsType	apiParams = {};
       BNZeroMemory (&apiOwner, sizeof (API_ParamOwnerType));
       BNZeroMemory (&apiParams, sizeof (API_GetParamsType));

       if (elemType == API_RailingToprailID
          || elemType == API_RailingHandrailID
          || elemType == API_RailingRailID
          || elemType == API_RailingPostID
          || elemType == API_RailingInnerPostID
          || elemType == API_RailingBalusterID
          || elemType == API_RailingPanelID
          || elemType == API_RailingNodeID
          || elemType == API_RailingToprailEndID
          || elemType == API_RailingHandrailEndID
          || elemType == API_RailingRailEndID
          || elemType == API_RailingToprailConnectionID
          || elemType == API_RailingHandrailConnectionID
          || elemType == API_RailingRailConnectionID
          || elemType == API_RailingEndFinishID) {
           return GetGDLParametersFromMemo (elemGuid, params);
       }

       #if defined(AC_27) || defined(AC_28)
       if (elemType == API_ExternalElemID) {
           return GetGDLParametersFromMemo (elemGuid, params);
       }
       #endif
       apiOwner.guid = elemGuid;
       #if defined AC_26 || defined AC_27 || defined AC_28
       apiOwner.type.typeID = elemType;
       #else
       apiOwner.typeID = elemType;
       #endif
       #if defined(AC_27) || defined(AC_28)
       err = ACAPI_LibraryPart_OpenParameters (&apiOwner);
       #else
       err = ACAPI_Goodies (APIAny_OpenParametersID, &apiOwner, nullptr);
       #endif
       if (err != NoError) {
           msg_rep ("GetGDLParameters", "APIAny_OpenParametersID. Check library for missing library parts", err, elemGuid);
           return GetGDLParametersFromMemo (elemGuid, params);
       }
       #if defined(AC_27) || defined(AC_28)
       err = ACAPI_LibraryPart_GetActParameters (&apiParams);
       #else
       err = ACAPI_Goodies (APIAny_GetActParametersID, &apiParams);
       #endif
       if (err != NoError) {
           msg_rep ("GetGDLParameters", "APIAny_GetActParametersID", err, elemGuid);
           #if defined(AC_27) || defined(AC_28)
           err = ACAPI_LibraryPart_CloseParameters ();
           #else
           err = ACAPI_Goodies (APIAny_CloseParametersID);
           #endif
           if (err != NoError) msg_rep ("GetGDLParameters", "APIAny_CloseParametersID", err, elemGuid);
           return err;
       }
       params = apiParams.params;
       #if defined(AC_27) || defined(AC_28)
       err = ACAPI_LibraryPart_CloseParameters ();
       #else
       err = ACAPI_Goodies (APIAny_CloseParametersID);
       #endif
       if (err != NoError) msg_rep ("GetGDLParameters", "APIAny_CloseParametersID", err, elemGuid);
       return err;*/
}


// --------------------------------------------------------------------
// Получение списка GUID панелей, рам и аксессуаров навесной стены
// --------------------------------------------------------------------
GSErrCode GetRElementsForCWall (const API_Guid & cwGuid, GS::Array<API_Guid>&elementsSymbolGuids)
{
    API_Element element = {}; BNZeroMemory (&element, sizeof (API_Element));
    element.header.guid = cwGuid;
    GSErrCode err = ACAPI_Element_Get (&element);
    if (err != NoError || !element.header.hasMemo) {
        return err;
    }
    API_ElementMemo	memo = {}; BNZeroMemory (&memo, sizeof (API_ElementMemo));
    UInt64 mask = APIMemoMask_CWallFrames | APIMemoMask_CWallPanels | APIMemoMask_CWallJunctions | APIMemoMask_CWallAccessories;
    err = ACAPI_Element_GetMemo (cwGuid, &memo, mask);
    if (err != NoError) {
        ACAPI_DisposeElemMemoHdls (&memo);
        return err;
    }
    bool isDegenerate = false;
    const GSSize nPanels = BMGetPtrSize (reinterpret_cast<GSPtr>(memo.cWallPanels)) / sizeof (API_CWPanelType);
    if (nPanels > 0) {
        for (Int32 idx = 0; idx < nPanels; ++idx) {
            #if defined(AC_27) || defined(AC_28)
            err = ACAPI_CurtainWall_IsCWPanelDegenerate (&memo.cWallPanels[idx].head.guid, &isDegenerate);
            #else
            err = ACAPI_Database (APIDb_IsCWPanelDegenerateID, (void*) (&memo.cWallPanels[idx].head.guid), &isDegenerate);
            #endif
            if (err == NoError && !isDegenerate && memo.cWallPanels[idx].hasSymbol && !memo.cWallPanels[idx].hidden) {
                elementsSymbolGuids.Push (std::move (memo.cWallPanels[idx].head.guid));
            }
        }
    }
    const GSSize nWallFrames = BMGetPtrSize (reinterpret_cast<GSPtr>(memo.cWallFrames)) / sizeof (API_CWFrameType);
    if (nWallFrames > 0) {
        for (Int32 idx = 0; idx < nWallFrames; ++idx) {
            if (memo.cWallFrames[idx].hasSymbol && !memo.cWallFrames[idx].deleteFlag && memo.cWallFrames[idx].objectType != APICWFrObjectType_Invisible) {
                elementsSymbolGuids.Push (std::move (memo.cWallFrames[idx].head.guid));
            }
        }
    }
    const GSSize nWallJunctions = BMGetPtrSize (reinterpret_cast<GSPtr> (memo.cWallJunctions)) / sizeof (API_CWJunctionType);
    if (nWallJunctions > 0) {
        for (Int32 idx = 0; idx < nWallJunctions; ++idx) {
            if (memo.cWallJunctions[idx].hasSymbol) {
                elementsSymbolGuids.Push (std::move (memo.cWallJunctions[idx].head.guid));
            }
        }
    }
    const GSSize nWallAccessories = BMGetPtrSize (reinterpret_cast<GSPtr> (memo.cWallAccessories)) / sizeof (API_CWAccessoryType);
    if (nWallAccessories > 0) {
        for (Int32 idx = 0; idx < nWallAccessories; ++idx) {
            if (memo.cWallAccessories[idx].hasSymbol) {
                elementsSymbolGuids.Push (std::move (memo.cWallAccessories[idx].head.guid));
            }
        }
    }
    ACAPI_DisposeElemMemoHdls (&memo);
    return err;
}

// --------------------------------------------------------------------
// Получение списка GUID элементов ограждения
// --------------------------------------------------------------------
GSErrCode GetRElementsForRailing (const API_Guid & elemGuid, GS::Array<API_Guid>&elementsGuids)
{
    API_Element element = {}; BNZeroMemory (&element, sizeof (API_Element));
    element.header.guid = elemGuid;
    GSErrCode err = ACAPI_Element_Get (&element);
    if (err != NoError) {
        return err;
    }
    API_ElementMemo	memo = {}; BNZeroMemory (&memo, sizeof (API_ElementMemo));
    UInt64 mask = APIMemoMask_RailingNode | APIMemoMask_RailingSegment | APIMemoMask_RailingPost | APIMemoMask_RailingInnerPost | APIMemoMask_RailingRail | APIMemoMask_RailingHandrail | APIMemoMask_RailingToprail | APIMemoMask_RailingPanel | APIMemoMask_RailingBaluster | APIMemoMask_RailingPattern | APIMemoMask_RailingBalusterSet | APIMemoMask_RailingRailEnd | APIMemoMask_RailingHandrailEnd | APIMemoMask_RailingToprailEnd | APIMemoMask_RailingRailConnection | APIMemoMask_RailingHandrailConnection | APIMemoMask_RailingToprailConnection;
    err = ACAPI_Element_GetMemo (elemGuid, &memo, mask);
    if (err != NoError) {
        ACAPI_DisposeElemMemoHdls (&memo);
        return err;
    }
    GSSize n = 0;
    n = BMGetPtrSize (reinterpret_cast<GSPtr>(memo.railingRailConnections)) / sizeof (API_RailingRailConnectionType);
    if (n > 0) {
        for (Int32 idx = 0; idx < n; ++idx) {
            elementsGuids.Push (std::move (memo.railingRailConnections[idx].head.guid));
        }
    }
    n = BMGetPtrSize (reinterpret_cast<GSPtr> (memo.railingHandrailConnections)) / sizeof (API_RailingRailConnectionType);
    if (n > 0) {
        for (Int32 idx = 0; idx < n; ++idx) {
            elementsGuids.Push (std::move (memo.railingHandrailConnections[idx].head.guid));
        }
    }
    n = BMGetPtrSize (reinterpret_cast<GSPtr> (memo.railingToprailConnections)) / sizeof (API_RailingRailConnectionType);
    if (n > 0) {
        for (Int32 idx = 0; idx < n; ++idx) {
            elementsGuids.Push (std::move (memo.railingToprailConnections[idx].head.guid));
        }
    }
    n = BMGetPtrSize (reinterpret_cast<GSPtr> (memo.railingRailEnds)) / sizeof (API_RailingRailEndType);
    if (n > 0) {
        for (Int32 idx = 0; idx < n; ++idx) {
            elementsGuids.Push (std::move (memo.railingRailEnds[idx].head.guid));
        }
    }
    n = BMGetPtrSize (reinterpret_cast<GSPtr> (memo.railingHandrailEnds)) / sizeof (API_RailingRailEndType);
    if (n > 0) {
        for (Int32 idx = 0; idx < n; ++idx) {
            elementsGuids.Push (std::move (memo.railingHandrailEnds[idx].head.guid));
        }
    }
    n = BMGetPtrSize (reinterpret_cast<GSPtr> (memo.railingToprailEnds)) / sizeof (API_RailingRailEndType);
    if (n > 0) {
        for (Int32 idx = 0; idx < n; ++idx) {
            elementsGuids.Push (std::move (memo.railingToprailEnds[idx].head.guid));
        }
    }
    n = BMGetPtrSize (reinterpret_cast<GSPtr> (memo.railingPosts)) / sizeof (API_RailingPostType);
    if (n > 0) {
        for (Int32 idx = 0; idx < n; ++idx) {
            elementsGuids.Push (std::move (memo.railingPosts[idx].head.guid));
        }
    }
    n = BMGetPtrSize (reinterpret_cast<GSPtr> (memo.railingRails)) / sizeof (API_RailingRailType);
    if (n > 0) {
        for (Int32 idx = 0; idx < n; ++idx) {
            if (memo.railingRails[idx].visible) elementsGuids.Push (std::move (memo.railingRails[idx].head.guid));
        }
    }
    n = BMGetPtrSize (reinterpret_cast<GSPtr> (memo.railingToprails)) / sizeof (API_RailingToprailType);
    if (n > 0) {
        for (Int32 idx = 0; idx < n; ++idx) {
            if (memo.railingToprails[idx].visible) elementsGuids.Push (std::move (memo.railingToprails[idx].head.guid));
        }
    }
    n = BMGetPtrSize (reinterpret_cast<GSPtr> (memo.railingHandrails)) / sizeof (API_RailingHandrailType);
    if (n > 0) {
        for (Int32 idx = 0; idx < n; ++idx) {
            if (memo.railingHandrails[idx].visible) elementsGuids.Push (std::move (memo.railingHandrails[idx].head.guid));
        }
    }
    n = BMGetPtrSize (reinterpret_cast<GSPtr> (memo.railingInnerPosts)) / sizeof (API_RailingInnerPostType);
    if (n > 0) {
        for (Int32 idx = 0; idx < n; ++idx) {
            elementsGuids.Push (std::move (memo.railingInnerPosts[idx].head.guid));
        }
    }
    n = BMGetPtrSize (reinterpret_cast<GSPtr> (memo.railingBalusters)) / sizeof (API_RailingBalusterType);
    if (n > 0) {
        for (Int32 idx = 0; idx < n; ++idx) {
            elementsGuids.Push (std::move (memo.railingBalusters[idx].head.guid));
        }
    }
    n = BMGetPtrSize (reinterpret_cast<GSPtr> (memo.railingPanels)) / sizeof (API_RailingPanelType);
    if (n > 0) {
        for (Int32 idx = 0; idx < n; ++idx) {
            if (memo.railingPanels[idx].visible) elementsGuids.Push (std::move (memo.railingPanels[idx].head.guid));
        }
    }
    ACAPI_DisposeElemMemoHdls (&memo);
    return err;
}

// --------------------------------------------------------------------
// Возвращает координаты заданной точки после трансформации матрицей
// --------------------------------------------------------------------
API_Coord3D GetWordCoord3DTM (const API_Coord3D vtx, const API_Tranmat & tm)
{
    API_Coord3D	trCoord = {};	// world coordinates 
    trCoord.x = tm.tmx[0] * vtx.x + tm.tmx[1] * vtx.y + tm.tmx[2] * vtx.z + tm.tmx[3];
    trCoord.y = tm.tmx[4] * vtx.x + tm.tmx[5] * vtx.y + tm.tmx[6] * vtx.z + tm.tmx[7];
    trCoord.z = tm.tmx[8] * vtx.x + tm.tmx[9] * vtx.y + tm.tmx[10] * vtx.z + tm.tmx[11];
    return trCoord;
}


Point2D GetWordPoint2DTM (const Point2D vtx, const API_Tranmat & tm)
{
    API_Coord3D c = GetWordCoord3DTM ({ vtx.x, vtx.y, 0 }, tm);
    return { c.x, c.y };
}

// -----------------------------------------------------------------------------
// Ask the user to click a point
// -----------------------------------------------------------------------------

bool	ClickAPoint (const char* prompt, Point2D * c)
{
    API_GetPointType	pointInfo = {};
    GSErrCode			err;
    CHTruncate (prompt, pointInfo.prompt, sizeof (pointInfo.prompt));
    pointInfo.changeFilter = false;
    pointInfo.changePlane = false;
    #if defined(AC_27) || defined(AC_28)
    err = ACAPI_UserInput_GetPoint (&pointInfo);
    #else
    err = ACAPI_Interface (APIIo_GetPointID, &pointInfo, nullptr);
    #endif
    if (err != NoError) {
        return false;
    }
    c->x = pointInfo.pos.x;
    c->y = pointInfo.pos.y;
    return true;
}		// ClickAPoint

#if defined(AC_27) || defined(AC_28) || defined(AC_26)
// -----------------------------------------------------------------------------
// Convert the NeigID to element type
// -----------------------------------------------------------------------------
API_ElemType Neig_To_ElemID (API_NeigID neigID)
{
    API_ElemType	type;
    GSErrCode		err;
    #if defined (AC_26)
    err = ACAPI_Goodies_NeigIDToElemType (neigID, type);
    #else
    err = ACAPI_Element_NeigIDToElemType (neigID, type);
    #endif
    if (err != NoError)
        type = API_ZombieElemID;

    return type;
}
#else
// -----------------------------------------------------------------------------
// Convert the NeigID to element type
// -----------------------------------------------------------------------------
API_ElemTypeID Neig_To_ElemID (API_NeigID neigID)
{
    API_ElemTypeID	typeID;
    GSErrCode		err;
    err = ACAPI_Goodies (APIAny_NeigIDToElemTypeID, &neigID, &typeID);
    if (err != NoError) typeID = API_ZombieElemID;
    return typeID;
}
#endif

// -----------------------------------------------------------------------------
// Convert the element header to a neig
// -----------------------------------------------------------------------------
bool	ElemHead_To_Neig (API_Neig * neig,
                          const API_Elem_Head * elemHead)
{
    API_ElemTypeID typeID = API_ZombieElemID;
    #if defined(AC_27) || defined(AC_28) || defined(AC_26)
    *neig = {};
    neig->guid = elemHead->guid;
    API_ElemType type = elemHead->type;
    if (type == API_ZombieElemID && neig->guid != APINULLGuid) {
        API_Elem_Head elemHeadCopy = {};
        elemHeadCopy.guid = elemHead->guid;
        ACAPI_Element_GetHeader (&elemHeadCopy);
        typeID = elemHeadCopy.type.typeID;
    }
    #else
    BNZeroMemory (neig, sizeof (API_Neig));
    API_Elem_Head* elemHeadNonConst = const_cast<API_Elem_Head*>(elemHead);
    neig->guid = elemHead->guid;
    if (elemHeadNonConst->typeID == API_ZombieElemID && neig->guid != APINULLGuid) {
        BNZeroMemory (elemHeadNonConst, sizeof (API_Elem_Head));
        elemHeadNonConst->guid = neig->guid;
        ACAPI_Element_GetHeader (elemHeadNonConst);
        typeID = elemHeadNonConst->typeID;
    }
    #endif
    switch (typeID) {
        case API_WallID:					neig->neigID = APINeig_Wall;				neig->inIndex = 1;	break;
        case API_ColumnID:					neig->neigID = APINeig_Colu;				neig->inIndex = 0;	break;
        case API_BeamID:					neig->neigID = APINeig_Beam;				neig->inIndex = 1;	break;
        case API_WindowID:					neig->neigID = APINeig_WindHole;			neig->inIndex = 0;	break;
        case API_DoorID:					neig->neigID = APINeig_DoorHole;			neig->inIndex = 0;	break;
        case API_ObjectID:					neig->neigID = APINeig_Symb;				neig->inIndex = 1;	break;
        case API_LampID:					neig->neigID = APINeig_Light;				neig->inIndex = 1;	break;
        case API_SlabID:					neig->neigID = APINeig_Ceil;				neig->inIndex = 1;	break;
        case API_RoofID:					neig->neigID = APINeig_Roof;				neig->inIndex = 1;	break;
        case API_MeshID:					neig->neigID = APINeig_Mesh;				neig->inIndex = 1;	break;

        case API_DimensionID:				neig->neigID = APINeig_DimOn;				neig->inIndex = 1;	break;
        case API_RadialDimensionID:			neig->neigID = APINeig_RadDim;				neig->inIndex = 1;	break;
        case API_LevelDimensionID:			neig->neigID = APINeig_LevDim;				neig->inIndex = 1;	break;
        case API_AngleDimensionID:			neig->neigID = APINeig_AngDimOn;			neig->inIndex = 1;	break;

        case API_TextID:					neig->neigID = APINeig_Word;				neig->inIndex = 1;	break;
        case API_LabelID:					neig->neigID = APINeig_Label;				neig->inIndex = 1;	break;
        case API_ZoneID:					neig->neigID = APINeig_Room;				neig->inIndex = 1;	break;

        case API_HatchID:					neig->neigID = APINeig_Hatch;				neig->inIndex = 1;	break;
        case API_LineID:					neig->neigID = APINeig_Line;				neig->inIndex = 1;	break;
        case API_PolyLineID:				neig->neigID = APINeig_PolyLine;			neig->inIndex = 1;	break;
        case API_ArcID:						neig->neigID = APINeig_Arc;					neig->inIndex = 1;	break;
        case API_CircleID:					neig->neigID = APINeig_Circ;				neig->inIndex = 1;	break;
        case API_SplineID:					neig->neigID = APINeig_Spline;				neig->inIndex = 1;	break;
        case API_HotspotID:					neig->neigID = APINeig_Hot;					neig->inIndex = 1;	break;

        case API_CutPlaneID:				neig->neigID = APINeig_CutPlane;			neig->inIndex = 1;	break;
        case API_ElevationID:				neig->neigID = APINeig_Elevation;			neig->inIndex = 1;	break;
        case API_InteriorElevationID:		neig->neigID = APINeig_InteriorElevation;	neig->inIndex = 1;	break;
        case API_CameraID:					neig->neigID = APINeig_Camera;				neig->inIndex = 1;	break;
        case API_CamSetID:					return false;

        case API_PictureID:					neig->neigID = APINeig_PictObj;				neig->inIndex = 1;	break;
        case API_DetailID:					neig->neigID = APINeig_Detail;				neig->inIndex = 1;	break;
        case API_WorksheetID:				neig->neigID = APINeig_Worksheet;			neig->inIndex = 1;	break;

        case API_SectElemID:				neig->neigID = APINeig_VirtSy;				neig->inIndex = 1;	break;
        case API_DrawingID:					neig->neigID = APINeig_DrawingCenter;		neig->inIndex = 1;	break;

        case API_CurtainWallID:				neig->neigID = APINeig_CurtainWall;			neig->inIndex = 1;	break;
        case API_CurtainWallSegmentID:		neig->neigID = APINeig_CWSegment;			neig->inIndex = 1;	break;
        case API_CurtainWallFrameID:		neig->neigID = APINeig_CWFrame;				neig->inIndex = 1;	break;
        case API_CurtainWallPanelID:		neig->neigID = APINeig_CWPanel;				neig->inIndex = 1;	break;
        case API_CurtainWallJunctionID:		neig->neigID = APINeig_CWJunction;			neig->inIndex = 1;	break;
        case API_CurtainWallAccessoryID:	neig->neigID = APINeig_CWAccessory;			neig->inIndex = 1;	break;
        case API_ShellID:					neig->neigID = APINeig_Shell;				neig->inIndex = 1;	break;
        case API_SkylightID:				neig->neigID = APINeig_SkylightHole;		neig->inIndex = 0;	break;
        case API_MorphID:					neig->neigID = APINeig_Morph;				neig->inIndex = 1;	break;
        case API_ChangeMarkerID:			neig->neigID = APINeig_ChangeMarker;		neig->inIndex = 1;	break;

        case API_GroupID:
        case API_HotlinkID:
        default:
            return false;
    }

    return true;
}		// ElemHead_To_Neig

// -----------------------------------------------------------------------------
// Ask the user to click an element
// 'needTypeID' specifies the requested element type
//	- API_ZombieElemID: all types pass
//	- API_XXXID: 		only this type will pass
// Return:
//	true:	the user clicked the correct element
//	false:	the input is canceled or wrong type of element was clicked
// -----------------------------------------------------------------------------
#if defined(AC_27) || defined(AC_28) || defined(AC_26)
bool	ClickAnElem (const char* prompt,
                     const API_ElemType & needType,
                     API_Neig * neig /*= nullptr*/,
                     API_ElemType * type /*= nullptr*/,
                     API_Guid * guid /*= nullptr*/,
                     API_Coord3D * c /*= nullptr*/,
                     bool					ignorePartialSelection /*= true*/)
{
    API_GetPointType	pointInfo = {};
    API_ElemType		clickedType;
    GSErrCode			err;

    CHTruncate (prompt, pointInfo.prompt, sizeof (pointInfo.prompt));
    pointInfo.changeFilter = false;
    pointInfo.changePlane = false;
    #if defined(AC_27) || defined(AC_28)
    err = ACAPI_UserInput_GetPoint (&pointInfo);
    #else
    err = ACAPI_Interface (APIIo_GetPointID, &pointInfo, nullptr);
    #endif
    if (err != NoError) {
        return false;
    }

    if (pointInfo.neig.neigID == APINeig_None) {		// try to find polygonal element clicked inside the polygon area
        API_Elem_Head		elemHead = {};
        API_ElemSearchPars	pars = {};
        pars.type = needType;
        pars.loc.x = pointInfo.pos.x;
        pars.loc.y = pointInfo.pos.y;
        pars.z = 1.00E6;
        pars.filterBits = APIFilt_OnVisLayer | APIFilt_OnActFloor;
        #if defined(AC_27) || defined(AC_28)
        err = ACAPI_Element_SearchElementByCoord (&pars, &elemHead.guid);
        #else
        err = ACAPI_Goodies (APIAny_SearchElementByCoordID, &pars, &elemHead.guid);
        #endif
        if (err == NoError) {
            elemHead.type = pars.type;
            ElemHead_To_Neig (&pointInfo.neig, &elemHead);
        }
    }

    if (pointInfo.neig.elemPartType != APINeigElemPart_None && ignorePartialSelection) {
        pointInfo.neig.elemPartType = APINeigElemPart_None;
        pointInfo.neig.elemPartIndex = 0;
    }

    clickedType = Neig_To_ElemID (pointInfo.neig.neigID);

    if (neig != nullptr)
        *neig = pointInfo.neig;
    if (type != nullptr)
        *type = clickedType;
    if (guid != nullptr)
        *guid = pointInfo.neig.guid;
    if (c != nullptr)
        *c = pointInfo.pos;

    if (clickedType == API_ZombieElemID)
        return false;

    bool good = (needType == API_ZombieElemID || needType == clickedType);

    if (!good && clickedType == API_SectElemID) {
        API_Element element = {};
        element.header.guid = pointInfo.neig.guid;
        if (ACAPI_Element_Get (&element) == NoError)
            good = (needType == element.sectElem.parentType);
    }

    return good;
}		// ClickAnElem
#else
bool	ClickAnElem (const char* prompt,
                     API_ElemTypeID		needTypeID,
                     API_Neig * neig /*= nullptr*/,
                     API_ElemTypeID * typeID /*= nullptr*/,
                     API_Guid * guid /*= nullptr*/,
                     API_Coord3D * c /*= nullptr*/,
                     bool				ignorePartialSelection /*= true*/)
{
    API_GetPointType	pointInfo = {};
    API_ElemTypeID		clickedID;
    GSErrCode			err;

    CHTruncate (prompt, pointInfo.prompt, sizeof (pointInfo.prompt));
    pointInfo.changeFilter = false;
    pointInfo.changePlane = false;
    err = ACAPI_Interface (APIIo_GetPointID, &pointInfo, nullptr);
    if (err != NoError) {
        if (err != APIERR_CANCEL)
            return false;
    }

    if (pointInfo.neig.neigID == APINeig_None) {		// try to find polygonal element clicked inside the polygon area
        API_Elem_Head elemHead = {};
        BNZeroMemory (&elemHead, sizeof (API_Elem_Head));
        API_ElemSearchPars	pars;
        BNZeroMemory (&pars, sizeof (API_ElemSearchPars));
        pars.typeID = needTypeID;
        pars.loc.x = pointInfo.pos.x;
        pars.loc.y = pointInfo.pos.y;
        //pars.z = 1.00E6;
        //pars.filterBits = APIFilt_OnVisLayer | APIFilt_OnActFloor;
        //err = ACAPI_Goodies (APIAny_SearchElementByCoordID, &pars, &elemHead.guid);
        if (err == NoError) {
            elemHead.typeID = pars.typeID;
            ElemHead_To_Neig (&pointInfo.neig, &elemHead);
        }
    }

    if (pointInfo.neig.elemPartType != APINeigElemPart_None && ignorePartialSelection) {
        pointInfo.neig.elemPartType = APINeigElemPart_None;
        pointInfo.neig.elemPartIndex = 0;
    }

    clickedID = Neig_To_ElemID (pointInfo.neig.neigID);

    if (neig != nullptr)
        *neig = pointInfo.neig;
    if (typeID != nullptr)
        *typeID = clickedID;
    if (guid != nullptr)
        *guid = pointInfo.neig.guid;
    if (c != nullptr)
        *c = pointInfo.pos;

    if (clickedID == API_ZombieElemID)
        return false;

    bool good = (needTypeID == API_ZombieElemID || needTypeID == clickedID);

    if (!good && clickedID == API_SectElemID) {
        API_Element element;
        BNZeroMemory (&element, sizeof (API_Element));
        element.header.guid = pointInfo.neig.guid;
        if (ACAPI_Element_Get (&element) == NoError)
            good = (needTypeID == element.sectElem.parentID);
    }

    return good;
}		// ClickAnElem
#endif

namespace FormatStringFunc
{
FormatString GetFormatStringFromFormula (const GS::UniString& formula, const  GS::UniString& part, GS::UniString& stringformat)
{
    FormatString f = ParseFormatString (".3m");
    if (!formula.Contains ('.')) return f;
    GS::UniString texpression = formula;
    GS::UniString texpression_ = formula;
    FormatStringFunc::ReplaceMeters (texpression);
    if (!texpression.Contains ('m')) return f;
    GS::UniString tpart = part;
    texpression.ReplaceAll ("{", "");
    texpression.ReplaceAll ("}", "");
    texpression_.ReplaceAll ("{", "");
    texpression_.ReplaceAll ("}", "");
    tpart.ReplaceAll ("{", "");
    tpart.ReplaceAll ("}", "");
    UInt32 n_start = texpression.FindFirst (tpart) + tpart.GetLength (); // Индекс начала поиска строки-формата
    GS::UniString stringformat_ = texpression.GetSubstring (char_formula_end, 'm', n_start) + 'm'; // Предположительно, строка-формат
    if (stringformat_.IsEmpty ()) stringformat_ = texpression.GetSubstring ('"', 'm', n_start) + 'm';
    if (stringformat_.Contains ('.') && !stringformat_.Contains (' ')) {
        // Проверим, не обрезали ли лишнюю m
        n_start = texpression.FindFirst (stringformat_) - 1;
        UInt32 n_end = n_start + stringformat_.GetLength ();
        if (n_end + 1 < texpression.GetLength ()) {
            GS::UniString endm = texpression.ToLowerCase ().GetSubstring (n_end + 1, 1);
            if (endm.IsEqual ("m") || endm.IsEqual ("p") || endm.IsEqual ("r") || endm.IsEqual ("f")) {
                n_end = n_end + 1;
            }
        }
        stringformat = texpression_.GetSubstring (n_start + 1, n_end - n_start);
        #ifdef TESTING
        DBtest (!stringformat.Contains ('"'), "GetFormatStringFromFormula : stringformat.Contains('\"') " + stringformat, false);
        DBtest (!stringformat.Contains (char_formula_end), "GetFormatStringFromFormula : stringformat.Contains(char_formula_end) " + stringformat, false);
        DBtest (!stringformat.Contains ('%'), "GetFormatStringFromFormula : stringformat.Contains('%') " + stringformat, false);
        DBtest (!stringformat.Contains ('}'), "GetFormatStringFromFormula : stringformat.Contains('}') " + stringformat, false);
        #endif
        stringformat.Trim ('"');
        stringformat.Trim (char_formula_end);
        stringformat.Trim ('%');
        stringformat.Trim ('}');
        stringformat.Trim ();
        f = FormatStringFunc::ParseFormatString (stringformat);
    }
    return f;
}

// -----------------------------------------------------------------------------
// Обработка количества нулей и единиц измерения в имени свойства
// Удаляет из имени paramName найденные единицы измерения
// Возвращает строку для скармливания функции NumToStig
// -----------------------------------------------------------------------------
GS::UniString GetFormatString (GS::UniString& paramName)
{
    GS::UniString formatstring = "";
    Int32 iseng = isEng ();
    if (!paramName.Contains (".")) return formatstring;
    GS::UniString meterString = RSGetIndString (ID_ADDON_STRINGS + iseng, MeterStringID, ACAPI_GetOwnResModule ());
    if (!paramName.Contains ('m') && !paramName.Contains (meterString)) return formatstring;
    GS::Array<GS::UniString> partstring;
    UInt32 n = StringSplt (paramName, ".", partstring);
    if (n > 1) {
        formatstring = partstring[n - 1];
        if (formatstring.Contains ('m') || formatstring.Contains (meterString)) {
            if (formatstring.Contains (CharENTER)) {
                UIndex attribinx = formatstring.FindLast (CharENTER);
                formatstring = formatstring.GetSubstring (0, attribinx);
            }
            paramName.ReplaceAll ('.' + formatstring, "");
            ReplaceMeters (formatstring, iseng);
        } else {
            // Если .м найдена не в последнем блоке - то это не строка-формат
            formatstring = "";
        }
    }
    return formatstring;
}

void ReplaceMeters (GS::UniString& formatstring)
{
    Int32 iseng = isEng ();
    ReplaceMeters (formatstring, iseng);
}

void ReplaceMeters (GS::UniString& formatstring, Int32& iseng)
{
    GS::UniString meterString = RSGetIndString (ID_ADDON_STRINGS + iseng, MeterStringID, ACAPI_GetOwnResModule ());
    formatstring.ReplaceAll (meterString, "m");
    meterString = RSGetIndString (ID_ADDON_STRINGS + iseng, DMeterStringID, ACAPI_GetOwnResModule ());
    formatstring.ReplaceAll (meterString, "d");
    meterString = RSGetIndString (ID_ADDON_STRINGS + iseng, CMeterStringID, ACAPI_GetOwnResModule ());
    formatstring.ReplaceAll (meterString, "c");
}


// -----------------------------------------------------------------------------
// Возвращает словарь строк-форматов для типов данных согласно настройкам Рабочей среды проекта
// -----------------------------------------------------------------------------
FormatStringDict GetFotmatStringForMeasureType ()
{
    FormatStringDict fdict = {};
    // Получаем данные об округлении и типе расчёта
    API_CalcUnitPrefs unitPrefs1;
    #if defined(AC_27) || defined(AC_28)
    ACAPI_ProjectSetting_GetPreferences (&unitPrefs1, APIPrefs_CalcUnitsID);
    #else
    ACAPI_Environment (APIEnv_GetPreferencesID, &unitPrefs1, (void*) APIPrefs_CalcUnitsID);
    #endif
    API_WorkingUnitPrefs unitPrefs;
    #if defined(AC_27) || defined(AC_28)
    ACAPI_ProjectSetting_GetPreferences (&unitPrefs, APIPrefs_WorkingUnitsID);
    #else
    ACAPI_Environment (APIEnv_GetPreferencesID, &unitPrefs, (void*) APIPrefs_WorkingUnitsID);
    #endif
    FormatString fstring = {};
    fstring.needRound = unitPrefs1.useDisplayedValues;

    fstring.n_zero = 2; fstring.stringformat = "2";
    fdict.Add (API_PropertyUndefinedMeasureType, fstring);

    fstring.n_zero = 2; fstring.stringformat = "2";
    fdict.Add (API_PropertyDefaultMeasureType, fstring);

    fstring.n_zero = unitPrefs.areaDecimals; fstring.stringformat = GS::UniString::Printf ("0%d", unitPrefs.areaDecimals);
    fdict.Add (API_PropertyAreaMeasureType, fstring);

    fstring.n_zero = unitPrefs.lenDecimals; fstring.stringformat = GS::UniString::Printf ("0%dmm", unitPrefs.lenDecimals);
    fdict.Add (API_PropertyLengthMeasureType, fstring);

    fstring.n_zero = unitPrefs.volumeDecimals; fstring.stringformat = GS::UniString::Printf ("0%d", unitPrefs.volumeDecimals);
    fdict.Add (API_PropertyVolumeMeasureType, fstring);

    fstring.n_zero = unitPrefs.angleDecimals; fstring.stringformat = GS::UniString::Printf ("0%d", unitPrefs.angleDecimals);
    fdict.Add (API_PropertyAngleMeasureType, fstring);
    return fdict;
}

// -----------------------------------------------------------------------------
// Извлекает из строки информацио о единицах измерении и округлении
// -----------------------------------------------------------------------------
FormatString ParseFormatString (const GS::UniString& stringformat)
{
    int n_zero = 3;
    Int32 krat = 0; // Крутность округления
    double koeff = 1; //Коэфф. увеличения
    bool trim_zero = true; //Требуется образать нули после запятой
    bool needround = false; //Требуется округлить численное значение для вычислений
    bool forceRaw = false; // Использовать неокруглённое значение для записи
    GS::UniString delimetr = ","; // Разделитель дробной части
    FormatString format;
    format.stringformat = stringformat;
    format.isEmpty = true;
    if (!stringformat.IsEmpty ()) {
        GS::UniString outstringformat = stringformat;
        if (stringformat.Contains (".")) {
            outstringformat.ReplaceAll (".", "");
            format.stringformat.ReplaceAll (".", "");
        }
        ReplaceMeters (outstringformat);
        if (outstringformat.Contains ("mm")) {
            n_zero = 0;
            koeff = 1000;
            outstringformat.ReplaceAll ("mm", "");
        }
        if (outstringformat.Contains ("cm")) {
            n_zero = 1;
            koeff = 100;
            outstringformat.ReplaceAll ("cm", "");
        }
        if (outstringformat.Contains ("dm")) {
            n_zero = 2;
            koeff = 10;
            outstringformat.ReplaceAll ("dm", "");
        }
        if (outstringformat.Contains ("gm")) {
            koeff = 1 / 100;
            outstringformat.ReplaceAll ("gm", "");
        }
        if (outstringformat.Contains ("km")) {
            koeff = 1 / 1000;
            outstringformat.ReplaceAll ("km", "");
        }
        if (outstringformat.Contains ("m")) {
            koeff = 1;
            n_zero = 3;
            outstringformat.ReplaceAll ("m", "");
        }
        if (outstringformat.Contains ("p")) {
            delimetr = ".";
            outstringformat.ReplaceAll ("p", "");
        }
        if (outstringformat.Contains ("r")) {
            needround = true;
            outstringformat.ReplaceAll ("r", "");
        }
        if (outstringformat.Contains ("f")) {
            forceRaw = true;
            outstringformat.ReplaceAll ("f", "");
        }
        // Принудительный вывод заданного кол-ва нулей после запятой
        if (outstringformat.Contains ("0")) {
            outstringformat.ReplaceAll ("0", "");
            outstringformat.Trim ();
            if (!outstringformat.IsEmpty ()) trim_zero = false;
        }
        if (!outstringformat.IsEmpty ()) {
            n_zero = std::atoi (outstringformat.ToCStr ());
        }
        format.isEmpty = false;
        format.isRead = true;
    }
    format.forceRaw = forceRaw;
    format.needRound = needround;
    format.delimetr = delimetr;
    format.n_zero = n_zero;
    format.krat = krat;
    format.koeff = koeff;
    format.trim_zero = trim_zero;
    return format;
}


// -----------------------------------------------------------------------------
// Переводит число в строку согласно настройкам строки-формата
// -----------------------------------------------------------------------------
// TODO Придумать более изящную обработку округления
GS::UniString NumToString (const double& var, const FormatString& stringformat)
{
    if (fabs (var) < 0.00000001) return "0";
    GS::UniString out = "";
    Int32 n_zero = stringformat.n_zero;
    Int32 krat = stringformat.krat;
    double koeff = stringformat.koeff;
    bool trim_zero = stringformat.trim_zero;
    GS::UniString delimetr = stringformat.delimetr;
    double outvar = var * koeff;
    outvar = round (outvar * pow (10, n_zero)) / pow (10, n_zero);
    if (krat > 0) outvar = ceil_mod ((GS::Int32) var, krat);
    out = GS::UniString::Printf ("%f", outvar);
    out.ReplaceAll (".", delimetr);
    out.ReplaceAll (",", delimetr);
    out.TrimRight ('0');
    if (trim_zero) {
        out.TrimRight (delimetr.GetChar (0));
    } else {
        Int32 addzero = n_zero - (out.GetLength () - out.FindFirst (delimetr.GetChar (0)) - 1);
        if (addzero > 0) {
            for (Int32 i = 0; i < addzero; i++) {
                out.Append ("0");
            }
        }
    }
    return out;
}
}

GSErrCode ConstructPolygon2DFromElementMemo (const API_ElementMemo & memo, Geometry::Polygon2D & poly)
{
    GSErrCode err = NoError;
    Geometry::Polygon2DData polygon2DData;
    Geometry::InitPolygon2DData (&polygon2DData);
    static_assert (sizeof (API_Coord) == sizeof (Coord), "sizeof (API_Coord) != sizeof (Coord)");
    static_assert (sizeof (API_PolyArc) == sizeof (PolyArcRec), "sizeof (API_PolyArc) != sizeof (PolyArcRec)");

    polygon2DData.nVertices = BMGetHandleSize (reinterpret_cast<GSHandle> (memo.coords)) / sizeof (Coord) - 1;
    polygon2DData.vertices = reinterpret_cast<Coord**> (BMAllocateHandle ((polygon2DData.nVertices + 1) * sizeof (Coord), ALLOCATE_CLEAR, 0));
    if (polygon2DData.vertices != nullptr)
        BNCopyMemory (*polygon2DData.vertices, *memo.coords, (polygon2DData.nVertices + 1) * sizeof (Coord));
    else
        err = APIERR_MEMFULL;
    if (err == NoError && memo.parcs != nullptr) {
        polygon2DData.nArcs = BMGetHandleSize (reinterpret_cast<GSHandle> (memo.parcs)) / sizeof (PolyArcRec);
        if (polygon2DData.nArcs > 0) {
            polygon2DData.arcs = reinterpret_cast<PolyArcRec**> (BMAllocateHandle ((polygon2DData.nArcs + 1) * sizeof (PolyArcRec), ALLOCATE_CLEAR, 0));
            if (polygon2DData.arcs != nullptr)
                BNCopyMemory (*polygon2DData.arcs + 1, *memo.parcs, polygon2DData.nArcs * sizeof (PolyArcRec));
            else
                err = APIERR_MEMFULL;
        }
    }
    if (err == NoError) {
        polygon2DData.nContours = BMGetHandleSize (reinterpret_cast<GSHandle> (memo.pends)) / sizeof (Int32) - 1;
        polygon2DData.contourEnds = reinterpret_cast<UIndex**> (BMAllocateHandle ((polygon2DData.nContours + 1) * sizeof (UIndex), ALLOCATE_CLEAR, 0));
        if (polygon2DData.contourEnds != nullptr)
            BNCopyMemory (*polygon2DData.contourEnds, *memo.pends, (polygon2DData.nContours + 1) * sizeof (UIndex));
        else
            err = APIERR_MEMFULL;
    }
    if (err == NoError) {
        Geometry::GetPolygon2DDataBoundBox (polygon2DData, &polygon2DData.boundBox);
        polygon2DData.status.isBoundBoxValid = true;
        Geometry::MultiPolygon2D multi;
        Geometry::ConvertPolygon2DDataToPolygon2D (multi, polygon2DData);
        poly = multi.PopLargest ();
    }
    Geometry::FreePolygon2DData (&polygon2DData);
    return err;
}

GSErrCode ConvertPolygon2DToAPIPolygon (const Geometry::Polygon2D & polygon, API_Polygon & poly, API_ElementMemo & memo)
{
    GSErrCode err = NoError;
    Geometry::Polygon2DData polygon2DData;
    Geometry::InitPolygon2DData (&polygon2DData);
    Geometry::ConvertPolygon2DToPolygon2DData (polygon2DData, polygon);

    poly.nCoords = polygon2DData.nVertices;
    poly.nSubPolys = polygon2DData.nContours;
    poly.nArcs = polygon2DData.nArcs;

    memo.coords = reinterpret_cast<API_Coord**>	(BMAllocateHandle ((poly.nCoords + 1) * sizeof (API_Coord), ALLOCATE_CLEAR, 0));
    memo.pends = reinterpret_cast<Int32**>		(BMAllocateHandle ((poly.nSubPolys + 1) * sizeof (Int32), ALLOCATE_CLEAR, 0));
    if (memo.coords != nullptr && memo.pends != nullptr && polygon2DData.vertices != nullptr) {
        static_assert (sizeof (API_Coord) == sizeof (Coord), "sizeof (API_Coord) != sizeof (Coord)");
        BNCopyMemory (*memo.coords, *polygon2DData.vertices, (poly.nCoords + 1) * sizeof (API_Coord));
        BNCopyMemory (*memo.pends, *polygon2DData.contourEnds, (poly.nSubPolys + 1) * sizeof (Int32));
    } else {
        err = APIERR_MEMFULL;
    }
    if (err == NoError && polygon2DData.arcs != nullptr) {
        memo.parcs = reinterpret_cast<API_PolyArc**> (BMAllocateHandle (poly.nArcs * sizeof (API_PolyArc), ALLOCATE_CLEAR, 0));
        if (memo.parcs != nullptr) {
            static_assert (sizeof (API_PolyArc) == sizeof (PolyArcRec), "sizeof (API_PolyArc) != sizeof (PolyArcRec)");
            BNCopyMemory (*memo.parcs, *polygon2DData.arcs, poly.nArcs * sizeof (API_PolyArc));
        } else {
            err = APIERR_MEMFULL;
        }
    }
    Geometry::FreePolygon2DData (&polygon2DData);
    return err;
}

void UnhideUnlockElementLayer (const API_Guid & elemGuid)
{
    GSErrCode err = NoError;
    if (ACAPI_Element_Filter (elemGuid, APIFilt_OnVisLayer)) return;
    API_Elem_Head elem_head = {};
    BNZeroMemory (&elem_head, sizeof (API_Elem_Head));
    elem_head.guid = elemGuid;
    err = ACAPI_Element_GetHeader (&elem_head);
    if (err != NoError) {
        msg_rep ("UnhideUnlockElementLayer", "", err, elemGuid);
        return;
    }
    UnhideUnlockElementLayer (elem_head);
}

void UnhideUnlockElementLayer (const API_Elem_Head & elem_head)
{
    if (ACAPI_Element_Filter (elem_head.guid, APIFilt_OnVisLayer)) return;
    UnhideUnlockElementLayer (elem_head.layer);
}

void UnhideUnlockElementLayer (const API_AttributeIndex & layer)
{
    API_Attribute attrib;
    GSErrCode err;
    BNZeroMemory (&attrib, sizeof (API_Attribute));
    attrib.header.typeID = API_LayerID;
    attrib.header.index = layer;
    err = ACAPI_Attribute_Get (&attrib);
    if (err != NoError) {
        msg_rep ("UnhideUnlockElementLayer", "ACAPI_Attribute_Get", err, APINULLGuid);
        return;
    }
    bool flag_write = false;
    if (attrib.header.flags & APILay_Hidden) {
        attrib.header.flags ^= APILay_Hidden;
        flag_write = true;
    }
    if (attrib.header.flags & APILay_Locked) {
        attrib.header.flags ^= APILay_Locked;
        flag_write = true;
    }
    if (flag_write) {
        err = ACAPI_Attribute_Modify (&attrib, NULL);
        if (err != NoError) msg_rep ("UnhideUnlockElementLayer", attrib.header.name, err, APINULLGuid);
    }
}

bool API_AttributeIndexFindByName (GS::UniString name, const API_AttrTypeID & type, API_AttributeIndex & attribinx)
{
    BNZeroMemory (&attribinx, sizeof (API_AttributeIndex));
    if (type == API_ZombieAttrID) return false;
    double inx = 0;
    if (UniStringToDouble (name, inx)) {
        #if defined(AC_27) || defined(AC_28)
        attribinx = ACAPI_CreateAttributeIndex ((Int32) inx);
        #else
        attribinx = (Int32) inx;
        #endif
        return true;
    } else {
        GSErrCode err = NoError;
        API_Attribute attrib;
        BNZeroMemory (&attrib, sizeof (API_Attribute));
        attrib.header.uniStringNamePtr = &name;
        attrib.header.typeID = type;
        attrib.header.guid = APINULLGuid;
        err = ACAPI_Attribute_Search (&attrib.header);
        attrib.header.uniStringNamePtr = nullptr;
        if (err != NoError) {
            msg_rep ("API_AttributeIndexFindByName", "ACAPI_Attribute_Search - " + name, err, APINULLGuid);
            return false;
        }
        attribinx = attrib.header.index;
        return true;
    }
}

GSErrCode Favorite_GetNum (const API_ElemTypeID & type, short* count, GS::Array< API_FavoriteFolderHierarchy >*folders, GS::Array< GS::UniString >*names)
{
    #if defined AC_22
    return APIERR_GENERAL;
    #else
    #if defined AC_26 || defined AC_27 || defined AC_28
    API_ElemType type_;
    type_.typeID = type;
    return ACAPI_Favorite_GetNum (type, count, folders, names);
    #else
    return ACAPI_Favorite_GetNum (type, APIVarId_Generic, count, folders, names);
    #endif
    #endif
}

API_ElemTypeID GetElemTypeID (const API_Guid & guid)
{
    API_ElemTypeID eltype = API_ZombieElemID;
    API_Elem_Head elementHead = {}; BNZeroMemory (&elementHead, sizeof (API_Elem_Head));
    elementHead.guid = guid;
    GSErrCode err = NoError;
    err = ACAPI_Element_GetHeader (&elementHead);
    if (err == NoError) eltype = GetElemTypeID (elementHead);
    return eltype;
}

API_ElemTypeID GetElemTypeID (const API_Elem_Head & elementhead)
{
    API_ElemTypeID eltype;
    #if defined AC_26 || defined AC_27 || defined AC_28
    eltype = elementhead.type.typeID;
    #else
    eltype = elementhead.typeID;
    #endif
    return eltype;
}

API_ElemTypeID GetElemTypeID (const API_Element & element)
{
    API_ElemTypeID eltype;
    #if defined AC_26 || defined AC_27 || defined AC_28
    eltype = element.header.type.typeID;
    #else
    eltype = element.header.typeID;
    #endif
    return eltype;
}

void SetElemTypeID (API_Element & element, const API_ElemTypeID eltype)
{
    #if defined AC_26 || defined AC_27 || defined AC_28
    element.header.type.typeID = eltype;
    #else
    element.header.typeID = eltype;
    #endif
}

void SetElemTypeID (API_Elem_Head & elementhead, const API_ElemTypeID eltype)
{
    #if defined AC_26 || defined AC_27 || defined AC_28
    elementhead.type.typeID = eltype;
    #else
    elementhead.typeID = eltype;
    #endif
}

GS::Array<API_Guid> GetElementByPropertyDescription (API_PropertyDefinition & definition, const GS::UniString value)
{
    GSErrCode error = NoError;
    GS::Array<API_Guid> elements = {};
    for (const auto& classificationItemGuid : definition.availability) {
        GS::Array<API_Guid> elemGuids = {};
        error = ACAPI_Element_GetElementsWithClassification (classificationItemGuid, elemGuids);
        if (error != NoError) {
            msg_rep ("GetElementByPropertyDescription", "ACAPI_Element_GetElementsWithClassification", error, classificationItemGuid);
            continue;
        }
        for (const auto& elemGuid : elemGuids) {
            API_Property propertyflag = {};
            error = ACAPI_Element_GetPropertyValue (elemGuid, definition.guid, propertyflag);
            if (error != NoError) {
                msg_rep ("GetElementByPropertyDescription", "ACAPI_Element_GetPropertyValue", error, elemGuid);
                continue;
            }
            #if defined(AC_22) || defined(AC_23)
            if (!propertyflag.isEvaluated) continue;
            if (propertyflag.isDefault) continue;
            if (propertyflag.value.singleVariant.variant.uniStringValue.IsEmpty ()) continue;
            if (propertyflag.value.singleVariant.variant.uniStringValue.ToLowerCase () == value.ToLowerCase ())                     elements.Push (elemGuid);
            #else
            if (propertyflag.status != API_Property_HasValue) continue;
            if (propertyflag.isDefault) continue;
            if (propertyflag.value.singleVariant.variant.uniStringValue.IsEmpty ()) continue;
            if (propertyflag.value.singleVariant.variant.uniStringValue.ToLowerCase ().IsEqual (value)) elements.Push (elemGuid);
            #endif
        }
    }
    return elements;
}

namespace GDLHelpers
{
bool ParamToMemo (API_ElementMemo& memo, ParamDict& param)
{
    const GSSize nParams = BMGetHandleSize ((GSHandle) memo.params) / sizeof (API_AddParType);
    for (GSIndex ii = 0; ii < nParams; ++ii) {
        API_AddParType& actParam = (*memo.params)[ii];
        GS::UniString rawname = "{@gdl:" + GS::UniString (actParam.name).ToLowerCase () + "}";
        if (!param.ContainsKey (rawname)) continue;
        Param pp = param.Get (rawname);
        if (actParam.typeMod == API_ParSimple) {
            switch (actParam.typeID) {
                case APIParT_LineTyp:
                case APIParT_Profile:
                case APIParT_BuildingMaterial:
                case APIParT_FillPat:
                case APIParT_Mater:
                case APIParT_Boolean:
                case APIParT_Integer:
                case APIParT_PenCol:
                case APIParT_Length:
                case APIParT_RealNum:
                case APIParT_ColRGB:
                case APIParT_Intens:
                case APIParT_Angle:
                    actParam.value.real = pp.num;
                    break;
                case APIParT_CString:
                    GS::ucscpy (actParam.value.uStr, pp.str.ToUStr (0, GS::Min (pp.str.GetLength (), (USize) API_UAddParStrLen)).Get ());
                    break;
                default:
                    break;
            }
        } else {
            double** newArrHdl = nullptr;
            double** origArrHdl = nullptr;
            switch (actParam.typeID) {
                case APIParT_LineTyp:
                case APIParT_Profile:
                case APIParT_BuildingMaterial:
                case APIParT_FillPat:
                case APIParT_Mater:
                case APIParT_Boolean:
                case APIParT_Integer:
                case APIParT_PenCol:
                case APIParT_Length:
                case APIParT_RealNum:
                case APIParT_ColRGB:
                case APIParT_Intens:
                case APIParT_Angle:
                    if (actParam.dim1 != pp.dim1 || actParam.dim2 != pp.dim2) {
                        actParam.dim1 = pp.dim1;
                        actParam.dim2 = pp.dim2;
                    }
                    origArrHdl = (double**) actParam.value.array;
                    newArrHdl = (double**) BMAllocateHandle (actParam.dim1 * actParam.dim2 * sizeof (double), ALLOCATE_CLEAR, 0);
                    for (Int32 k = 0; k < actParam.dim1; k++)
                        for (Int32 j = 0; j < actParam.dim2; j++)
                            (*newArrHdl)[k * actParam.dim2 + j] = pp.arr_num[k * actParam.dim2 + j];
                    BMKillHandle ((GSHandle*) &origArrHdl);
                    actParam.value.array = (GSHandle) newArrHdl;
                    break;
                case APIParT_CString:
                    break;
                default:
                    break;
            }
        }
        param.Delete (rawname);
        if (param.IsEmpty ()) {
            return true;
        }
    }
    return param.IsEmpty ();
}
}

