//------------ kuvbur 2022 ------------
#include	"ACAPinc.h"
#include	"APIEnvir.h"
#include	"Revision.hpp"
#include	<stdlib.h> /* atoi */
#include	<time.h>

typedef struct
{
    GS::Array<API_Guid> guids;
} Revision;

typedef std::map<std::string, Revision, doj::alphanum_less<std::string>> RevisionDict;

//bool GetAllChanges (void)
//{
//    GS::Array<API_RVMChange> changes;
//    GSErrCode err = ACAPI_Database (APIDb_GetRVMChangesID, &changes);
//    if (err != NoError) {
//        return false;
//    }
//    if (changes.IsEmpty ()) {
//        return false;
//    }
//    char	buffer[256];
//    for (auto& change : changes.AsConst ()) {
//        sprintf (buffer, "ID: %s, Description: %s", change.id.ToCStr ().Get (), change.description.ToCStr ().Get ());
//    }
//    return false;
//}


static void		Do_GetChangeCustomScheme (void)
{
    GS::Array<API_RVMChange> changes;
    GSErrCode err = ACAPI_Database (APIDb_GetRVMChangesID, &changes);
    if (err != NoError) {
        return;
    }

    if (changes.IsEmpty ()) {
        return;
    }

    GS::HashTable<API_Guid, GS::UniString> changeCustomSchemes;
    err = ACAPI_Database (APIDb_GetRVMChangeCustomSchemeID, &changeCustomSchemes);
    if (err != NoError) {

        return;
    }

    if (changeCustomSchemes.IsEmpty ()) {

        return;
    }

    GS::UniString buffer;
    for (auto& change : changes.AsConst ()) {
        buffer = GS::UniString::Printf ("ID: %T, ", change.id.ToPrintf ());
        bool firstLoop = true;
        for (auto& cIt : changeCustomSchemes) {
            const API_Guid& guid = *cIt.key;
            const GS::UniString& customFieldName = *cIt.value;
            GS::UniString			customFieldValue;

            if (!firstLoop)
                buffer.Append (", ");

            change.customData.Get (guid, &customFieldValue);
            buffer.Append (customFieldName + ": " + customFieldValue);

            firstLoop = false;
        }
    }
}

void ChangeMarkerText (API_Guid& markerguid, GS::UniString& text)
{
    GSErrCode err = NoError;
    API_Element markerElement;
    BNZeroMemory (&markerElement, sizeof (API_Element));
    markerElement.header.guid = markerguid;

    err = ACAPI_Element_Get (&markerElement);
    if (err != NoError) {
        return;
    }

    API_ElementMemo memo;
    BNZeroMemory (&memo, sizeof (API_ElementMemo));
    err = ACAPI_Element_GetMemo (markerElement.header.guid, &memo, APIMemoMask_AddPars);
    if (err != NoError) {
        return;
    }

    API_Element markerMask;
    ACAPI_ELEMENT_MASK_CLEAR (markerMask);

    GS::ucscpy ((*memo.params)[2].value.uStr, GS::UniString ("Arial Black Central European").ToUStr ());

    err = ACAPI_Element_Change (&markerElement, &markerMask, &memo, APIMemoMask_AddPars, true);
    if (err != NoError) {
        return;
    }

    ACAPI_DisposeElemMemoHdls (&memo);
}




bool GetMarkerPos (API_Guid& markerguid, API_Coord& startpoint)
{
    GSErrCode err = NoError;
    API_ElementMemo memo;
    BNZeroMemory (&memo, sizeof (API_ElementMemo));
    err = ACAPI_Element_GetMemo (markerguid, &memo, APIMemoMask_Polygon);
    if (err == NoError) {
        int begInd = (*memo.pends)[0] + 1;
        startpoint = (*memo.coords)[begInd];
        ACAPI_DisposeElemMemoHdls (&memo);
        return true;
    } else {
        msg_rep ("GetChangesMarker", "ACAPI_Element_GetMemo_1", err, markerguid);
    }
    ACAPI_DisposeElemMemoHdls (&memo);
    return false;
}

bool GetMarkerText (API_Guid& markerguid, GS::UniString& text, GS::UniString& number, GS::UniString& inx)
{
    bool find_nuch = false; bool find_izm = false; bool find_text = false;
    GSErrCode err = NoError;
    API_ElementMemo memo;
    BNZeroMemory (&memo, sizeof (API_ElementMemo));
    err = ACAPI_Element_GetMemo (markerguid, &memo, APIMemoMask_AddPars);
    if (err == NoError) {
        Int32	addParNum = BMGetHandleSize ((GSHandle) memo.params) / sizeof (API_AddParType);
        for (Int32 i = 0; i < addParNum; ++i) {
            API_AddParType& actualParam = (*memo.params)[i];
            GS::UniString name = actualParam.name;
            if (name.Contains ("somestuff_change_text")) {
                text = actualParam.value.uStr;
                find_text = true;
            }
            if (name.Contains ("somestuff_number_change")) {
                number = actualParam.value.uStr;
                find_nuch = true;
            }
            if (name.Contains ("somestuff_inx_change")) {
                inx = actualParam.value.uStr;
                find_izm = true;
            }
            if (find_nuch && find_izm && find_text) {
                ACAPI_DisposeElemMemoHdls (&memo);
                return true;
            }
        }
    } else {
        msg_rep ("GetChangesMarker", "ACAPI_Element_GetMemo_2", err, markerguid);
    }
    ACAPI_DisposeElemMemoHdls (&memo);
    return find_nuch && find_izm && find_text;
}


bool GetChangesMarker (GS::Array<Change>& changes)
{
    GSErrCode err = NoError;
    API_ElemTypeID elementType = API_ChangeMarkerID;
    GS::Array<API_Guid>	guidArray;
    err = ACAPI_Element_GetElemList (elementType, &guidArray, APIFilt_IsEditable | APIFilt_HasAccessRight | APIFilt_InMyWorkspace);
    if (err != NoError) {
        msg_rep ("GetChangesMarker", "ACAPI_Element_GetElemList", err, APINULLGuid);
        return false;
    }
    if (!guidArray.IsEmpty ()) {
        for (UInt32 i = 0; i < guidArray.GetSize (); i++) {
            API_Element element = {};
            BNZeroMemory (&element, sizeof (API_Element));
            element.header.guid = guidArray[i];
            err = ACAPI_Element_Get (&element);
            if (err == NoError && element.header.hasMemo) {
                GS::UniString text = ""; GS::UniString number = ""; GS::UniString inx = ""; API_Coord startpoint;
                if (GetMarkerPos (guidArray[i], startpoint) && GetMarkerText (element.changeMarker.markerGuid, text, number, inx)) {
                    Change ch;
                    ch.markerguid = element.changeMarker.markerGuid;
                    ch.changeId = element.changeMarker.changeId;
                    ch.changeName = element.changeMarker.changeName;
                    ch.startpoint = startpoint;
                    ch.number = number;
                    ch.inx = inx;
                    ch.text = text;
                    changes.Push (ch);
                }
            }
        }
    }
    return false;
}


bool GetAllChangesMarker (void)
{
    GSErrCode err = NoError;
    GS::Array<API_DatabaseUnId> dbases;
    err = ACAPI_Database (APIDb_GetLayoutDatabasesID, nullptr, &dbases);
    if (err != NoError) return false;
    GS::Array<Change> changes;
    for (const auto& dbUnId : dbases) {
        API_DatabaseInfo dbInfo = {};
        dbInfo.typeID = APIWind_LayoutID;
        dbInfo.databaseUnId = dbUnId;
        err = ACAPI_Database (APIDb_ChangeCurrentDatabaseID, &dbInfo, nullptr);
        if (err != NoError) {
            msg_rep ("GetChangesMarker", "APIDb_ChangeCurrentDatabaseID", err, APINULLGuid);
        } else {
            GetChangesMarker (changes);
        }
        API_LayoutInfo	layoutInfo;
        BNZeroMemory (&layoutInfo, sizeof (API_LayoutInfo));
        if (ACAPI_Environment (APIEnv_GetLayoutSetsID, &layoutInfo, &dbInfo.databaseUnId) == NoError) {
            if (layoutInfo.customData != nullptr) {
                for (auto it = layoutInfo.customData->EnumeratePairs (); it != nullptr; ++it) {
                    *it->value += " - Modified via API";
                }
                ACAPI_Environment (APIEnv_ChangeLayoutSetsID, &layoutInfo, &dbInfo.databaseUnId);
            }
            if (layoutInfo.customData != nullptr) delete layoutInfo.customData;

        }
    }
    return false;
}


void SetRevision (void)
{
    GSErrCode err = NoError;
    //bool haschanges = GetAllChanges ();


    API_DatabaseInfo databasestart;
    API_WindowInfo windowstart;
    GS::IntPtr	store = 1;
    err = ACAPI_Database (APIDb_StoreViewSettingsID, (void*) store);
    if (err != NoError) {
        store = -1;
        err = NoError;
    }
    BNZeroMemory (&databasestart, sizeof (API_DatabaseInfo));
    err = ACAPI_Database (APIDb_GetCurrentDatabaseID, &databasestart, nullptr);
    if (err != NoError) {
        msg_rep ("SetRevision", "APIDb_GetCurrentDatabaseID", err, APINULLGuid);
        return;
    }
    BNZeroMemory (&windowstart, sizeof (API_WindowInfo));
    err = ACAPI_Database (APIDb_GetCurrentWindowID, &windowstart, nullptr);
    if (err != NoError) {
        msg_rep ("SetRevision", "APIDb_GetCurrentWindowID", err, APINULLGuid);
        return;
    }
    Do_GetChangeCustomScheme ();
    bool haschanges = GetAllChangesMarker ();

    // Возвращение на исходную БД и окно
    err = ACAPI_Database (APIDb_ChangeCurrentDatabaseID, &databasestart, nullptr);
    if (err != NoError) {
        msg_rep ("SetRevision", "APIDb_ChangeCurrentDatabaseID", err, APINULLGuid);
        return;
    }
    err = ACAPI_Automate (APIDo_ChangeWindowID, &windowstart, nullptr);
    if (err != NoError) {
        msg_rep ("SetRevision", "APIDo_ChangeWindowID", err, APINULLGuid);
        return;
    }
    if (store == 1) {
        store = 0;
        ACAPI_Database (APIDb_StoreViewSettingsID, (void*) store);
    }
    //API_DatabaseInfo dbInfo;
    //BNZeroMemory (&dbInfo, sizeof (API_DatabaseInfo));
    //err = ACAPI_Database (APIDb_GetCurrentDatabaseID, &dbInfo);
    //if (err != NoError) return;
    //if (dbInfo.typeID != APIWind_LayoutID) return;







    //API_ElemTypeID elementType = API_ChangeMarkerID;
    //GS::Array<API_Guid>	guidArray;
    //err = ACAPI_Element_GetElemList (elementType, &guidArray, APIFilt_IsEditable | APIFilt_HasAccessRight | APIFilt_InMyWorkspace | APIFilt_OnVisLayer);
    //if (guidArray.IsEmpty ()) return;
    //API_Element element = {};
    //for (UInt32 i = 0; i < guidArray.GetSize (); i++) {
    //    element.header.guid = guidArray[i];
    //    err = ACAPI_Element_Get (&element);

    //    if (err != NoError) {
    //    }
    //}

    //GS::Array<API_RVMChange> changes;
    //err = ACAPI_Database(APIDb_GetRVMLayoutCurrentRevisionChangesID, &(dbInfo.databaseUnId), &changes);
    //if (err != NoError) {
    //	ACAPI_WriteReport("Do_GetLayoutCurrentRevisionChanges: error occured!", false);
    //	return;
    //}

    //char buffer[256];

    //if (changes.IsEmpty()) {
    //	sprintf(buffer, "There are no changes in current revision of {%s} layout!", APIGuid2GSGuid(dbInfo.databaseUnId.elemSetId).ToUniString().ToCStr().Get());
    //	ACAPI_WriteReport(buffer, false);
    //	return;
    //}

    //sprintf(buffer, "# Changes in current revision of {%s} layout:", APIGuid2GSGuid(dbInfo.databaseUnId.elemSetId).ToUniString().ToCStr().Get());
    //ACAPI_WriteReport(buffer, false);

    //for (auto& change : changes.AsConst()) {
    //	sprintf(buffer, "ID: %s, Description: %s", change.id.ToCStr().Get(), change.description.ToCStr().Get());
    //	ACAPI_WriteReport(buffer, false);
    //}
}
