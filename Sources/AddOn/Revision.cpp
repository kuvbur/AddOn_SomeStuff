//------------ kuvbur 2022 ------------
#ifdef PK_1
#include	"ACAPinc.h"
#include	"APIEnvir.h"
#include	"Revision.hpp"
#include	<stdlib.h> /* atoi */
#include	<time.h>

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

void ChangeMarkerText (API_Guid& markerguid, GS::UniString& number, GS::UniString& inx)
{
    GSErrCode err = NoError;
    API_Element markerElement;
    BNZeroMemory (&markerElement, sizeof (API_Element));
    markerElement.header.guid = markerguid;

    err = ACAPI_Element_Get (&markerElement);
    if (err != NoError) {
        msg_rep ("ChangeMarkerText", "ACAPI_Element_Get", err, markerguid);
        return;
    }
    API_Element markerMask;
    ACAPI_ELEMENT_MASK_CLEAR (markerMask);

    API_ElementMemo memo;
    BNZeroMemory (&memo, sizeof (API_ElementMemo));
    err = ACAPI_Element_GetMemo (markerguid, &memo, APIMemoMask_AddPars);
    if (err == NoError) {
        bool find_nuch = false; bool find_izm = false;
        Int32	addParNum = BMGetHandleSize ((GSHandle) memo.params) / sizeof (API_AddParType);
        for (Int32 i = 0; i < addParNum; ++i) {
            API_AddParType& actualParam = (*memo.params)[i];
            GS::UniString name = actualParam.name;
            if (name.Contains ("somestuff_number_change")) {
                GS::ucscpy ((*memo.params)[i].value.uStr, number.ToUStr ());
                find_nuch = true;
            }
            if (name.Contains ("somestuff_inx_change")) {
                GS::ucscpy ((*memo.params)[i].value.uStr, inx.ToUStr ());
                find_izm = true;
            }
            if (find_nuch && find_izm) {
                err = ACAPI_Element_Change (&markerElement, &markerMask, &memo, APIMemoMask_AddPars, true);
                if (err != NoError) {
                    msg_rep ("ChangeMarkerText", "ACAPI_Element_Change", err, markerguid);
                }
                break;
            }
        }
    } else {
        msg_rep ("ChangeMarkerText", "ACAPI_Element_GetMemo", err, markerguid);
    }
    ACAPI_DisposeElemMemoHdls (&memo);
    return;
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

bool GetMarkerText (API_Guid& markerguid, GS::UniString& text, GS::UniString& number, GS::UniString& inx, GS::Int32& typeizm)
{
    GSErrCode err = NoError;
    API_ElementMemo memo;
    BNZeroMemory (&memo, sizeof (API_ElementMemo));
    err = ACAPI_Element_GetMemo (markerguid, &memo, APIMemoMask_AddPars);
    bool find_nuch = false; bool find_izm = false; bool find_text = false; bool find_type = false;
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
            if (name.Contains ("somestuff_type_change")) {
                typeizm = (int) actualParam.value.real;
                find_type = true;
            }
            if (find_nuch && find_izm && find_text && find_type) {
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

bool GetChangesMarker (ChangeMarkerDict& changes)
{
    GS::UniString izmString = RSGetIndString (ID_ADDON_STRINGS + isEng (), Izm_StringID, ACAPI_GetOwnResModule ());
    GS::UniString zamString = RSGetIndString (ID_ADDON_STRINGS + isEng (), Zam_StringID, ACAPI_GetOwnResModule ());
    GS::UniString novString = RSGetIndString (ID_ADDON_STRINGS + isEng (), Nov_StringID, ACAPI_GetOwnResModule ());
    GS::UniString annulString = RSGetIndString (ID_ADDON_STRINGS + isEng (), Annul_StringID, ACAPI_GetOwnResModule ());
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
                GS::UniString text = ""; GS::UniString number = ""; GS::UniString inx = ""; API_Coord startpoint; GS::Int32 typeizm;
                if (GetMarkerPos (guidArray[i], startpoint) && GetMarkerText (element.changeMarker.markerGuid, text, number, inx, typeizm)) {
                    Change ch;
                    ch.changeId = element.changeMarker.changeId;
                    if (typeizm > TypeNone && typeizm <= TypeAnnul) {
                        ch.typeizm = typeizm;
                    } else {
                        if (ch.changeId.Contains (izmString)) ch.typeizm = TypeIzm;
                        if (ch.changeId.Contains (zamString)) ch.typeizm = TypeZam;
                        if (ch.changeId.Contains (novString)) ch.typeizm = TypeNov;
                        if (ch.changeId.Contains (annulString)) ch.typeizm = TypeAnnul;
                    }
                    if (ch.typeizm != TypeNone) {
                        ch.markerguid = element.changeMarker.markerGuid;
                        ch.changeName = element.changeMarker.changeName;
                        ch.startpoint = startpoint;
                        ch.number = number;
                        ch.inx = inx;
                        ch.text = text;
                        ch.changeId.ReplaceAll (izmString + ".", "");
                        ch.changeId.ReplaceAll (zamString + ".", "");
                        ch.changeId.ReplaceAll (novString + ".", "");
                        ch.changeId.ReplaceAll (annulString + ".", "");
                        ch.changeId.ReplaceAll (izmString, "");
                        ch.changeId.ReplaceAll (zamString, "");
                        ch.changeId.ReplaceAll (novString, "");
                        ch.changeId.ReplaceAll (annulString, "");
                        ch.changeId.ReplaceAll ("  ", " ");
                        ch.changeId.Trim ();
                        ch.changeName.Trim ();
                        ch.number.Trim ();
                        ch.inx.Trim ();
                        ch.text.Trim ();
                        std::string key = ch.changeId.ToCStr (0, MaxUSize, GChCode).Get ();
                        if (changes.count (key) == 0) changes[key] = {};
                        changes[key].arr.Push (ch);
                    }
                }
            }
        }
    }

    if (!changes.empty ()) {
        GS::UniString undoString = RSGetIndString (ID_ADDON_STRINGS + isEng (), UndoReNumId, ACAPI_GetOwnResModule ());
        ACAPI_CallUndoableCommand (undoString, [&]() -> GSErrCode {
            for (ChangeMarkerDict::iterator ch = changes.begin (); ch != changes.end (); ++ch) {
                int number_n = 0;
                Changes& change = ch->second;
                for (UInt32 i = 0; i < change.arr.GetSize (); i++) {
                    if (change.arr[i].typeizm == TypeIzm) {
                        number_n += 1;
                        change.arr[i].number = GS::UniString::Printf ("%d", number_n);
                        ChangeMarkerText (change.arr[i].markerguid, change.arr[i].number, change.arr[i].changeName);
                        change.nuch = number_n;
                    }
                }
            }
            return NoError;
        });
    }
    return !changes.empty ();
}

void GetScheme ()
{
    GSErrCode err = NoError;
    /// Свойства выпуска
    GS::HashTable<API_Guid, GS::UniString> vipcheme;
    err = ACAPI_Database (APIDb_GetRVMIssueCustomSchemeID, &vipcheme);
    /// Свойства изменения
    GS::HashTable<API_Guid, GS::UniString> izmScheme;
    err = ACAPI_Database (APIDb_GetRVMChangeCustomSchemeID, &izmScheme);
}


bool GetAllChangesMarker (void)
{
    GSErrCode err = NoError;
    ChangeMarkerByListDict allchanges;
    // Получаем выпуски
    GS::Array<API_RVMDocumentRevision> api_revisions;
    err = ACAPI_Database (APIDb_GetRVMDocumentRevisionsID, &api_revisions);
    if (err != NoError) {
        msg_rep ("GetChangesMarker", "APIDb_GetRVMDocumentRevisionsID", err, APINULLGuid);
        return false;
    }
    GS::UniString izmString = RSGetIndString (ID_ADDON_STRINGS + isEng (), Izm_StringID, ACAPI_GetOwnResModule ());
    GS::UniString zamString = RSGetIndString (ID_ADDON_STRINGS + isEng (), Zam_StringID, ACAPI_GetOwnResModule ());
    GS::UniString novString = RSGetIndString (ID_ADDON_STRINGS + isEng (), Nov_StringID, ACAPI_GetOwnResModule ());
    GS::UniString annulString = RSGetIndString (ID_ADDON_STRINGS + isEng (), Annul_StringID, ACAPI_GetOwnResModule ());
    for (auto revision : api_revisions) {
        GS::Array<API_RVMChange> api_changes;
        err = ACAPI_Database (APIDb_GetRVMDocumentRevisionChangesID, &revision.guid, &api_changes);
        if (err == NoError) {
            ChangeMarkerDict changes;
            API_DatabaseInfo dbInfo = {};
            dbInfo.typeID = APIWind_LayoutID;
            dbInfo.databaseUnId = revision.layoutInfo.dbId;
            err = ACAPI_Database (APIDb_ChangeCurrentDatabaseID, &dbInfo, nullptr);
            if (err == NoError) {
                GS::Array<API_RVMChange> change;
                err = ACAPI_Database (APIDb_GetRVMLayoutCurrentRevisionChangesID, &(dbInfo.databaseUnId), &change);
                if (change.GetSize () > 1) {
                    // Обрабатываем маркеры
                    GetChangesMarker (changes);
                    // Обрабатываем изменения, не привязанные к маркерам
                    for (auto c : change) {
                        Change ch;
                        if (c.id.Contains (izmString)) ch.typeizm = TypeIzm;
                        if (c.id.Contains (zamString)) ch.typeizm = TypeZam;
                        if (c.id.Contains (novString)) ch.typeizm = TypeNov;
                        if (c.id.Contains (annulString)) ch.typeizm = TypeAnnul;
                        if (ch.typeizm != TypeNone) {
                            ch.markerguid = APINULLGuid;
                            ch.changeId = c.id;
                            ch.changeName = c.description;
                            ch.changeId.ReplaceAll (izmString + ".", "");
                            ch.changeId.ReplaceAll (zamString + ".", "");
                            ch.changeId.ReplaceAll (novString + ".", "");
                            ch.changeId.ReplaceAll (annulString + ".", "");
                            ch.changeId.ReplaceAll (izmString, "");
                            ch.changeId.ReplaceAll (zamString, "");
                            ch.changeId.ReplaceAll (novString, "");
                            ch.changeId.ReplaceAll (annulString, "");
                            ch.changeId.ReplaceAll ("  ", " ");
                            ch.changeId.Trim ();
                            ch.changeName.Trim ();
                            std::string key = ch.changeId.ToCStr (0, MaxUSize, GChCode).Get ();
                            if (changes.count (key) == 0) changes[key] = {};
                            changes[key].arr.Push (ch);
                        }
                    }
                    GS::UniString key = revision.layoutInfo.id + revision.layoutInfo.name;
                    std::string key_ = key.ToCStr (0, MaxUSize, GChCode).Get ();
                    if (allchanges.count (key_) == 0) allchanges[key_] = changes;
                }
            } else {
                msg_rep ("GetChangesMarker", "APIDb_ChangeCurrentDatabaseID", err, APINULLGuid);
            }
        } else {
            msg_rep ("GetChangesMarker", "APIDb_GetRVMDocumentRevisionChangesID", err, revision.guid);
        }
    }
    return !allchanges.empty ();
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
#endif
