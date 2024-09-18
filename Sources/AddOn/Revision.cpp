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

void ChangeMarkerText (API_Guid& markerguid, GS::UniString& nuch, GS::UniString& nizm)
{
    GSErrCode err = NoError;
    API_Element markerElement;
    BNZeroMemory (&markerElement, sizeof (API_Element));
    markerElement.header.guid = markerguid;
    if (markerguid == APINULLGuid) return;
    if (!ACAPI_Element_Filter (markerguid, APIFilt_HasAccessRight)) {
        msg_rep ("ChangeMarkerText", "Marker not HasAccessRight", NoError, markerguid);
        return;
    }
    if (!ACAPI_Element_Filter (markerguid, APIFilt_IsEditable)) {
        msg_rep ("ChangeMarkerText", "Marker not Editable", NoError, markerguid);
        return;
    }
#if defined(AC_27) || defined(AC_28)
    if (ACAPI_Teamwork_HasConnection () && !ACAPI_Element_Filter (objectId, APIFilt_InMyWorkspace)) {
#else
    if (ACAPI_TeamworkControl_HasConnection () && !ACAPI_Element_Filter (markerguid, APIFilt_InMyWorkspace)) {
#endif
        msg_rep ("ChangeMarkerText", "Marker not reserved", NoError, markerguid);
        return;
    }
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
        bool find_nuch = false; bool find_izm = false; bool flag_write = false;
        Int32	addParNum = BMGetHandleSize ((GSHandle) memo.params) / sizeof (API_AddParType);
        for (Int32 i = 0; i < addParNum; ++i) {
            API_AddParType& actualParam = (*memo.params)[i];
            GS::UniString name = actualParam.name;
            if (name.Contains ("somestuff_nuch_change")) {
                GS::UniString t = (*memo.params)[i].value.uStr;
                if (!t.IsEqual (nuch)) {
                    flag_write = true;
                    GS::ucscpy ((*memo.params)[i].value.uStr, nuch.ToUStr ());
                }
                find_nuch = true;
            }
            if (name.Contains ("somestuff_nizm_change")) {
                GS::UniString t = (*memo.params)[i].value.uStr;
                if (!t.IsEqual (nizm)) {
                    flag_write = true;
                    GS::ucscpy ((*memo.params)[i].value.uStr, nizm.ToUStr ());
                }
                find_izm = true;
            }
            if (find_nuch && find_izm && flag_write) {
                err = ACAPI_Element_Change (&markerElement, &markerMask, &memo, APIMemoMask_AddPars, true);
                if (err != NoError) {
                    msg_rep ("ChangeMarkerText", "ACAPI_Element_Change", err, markerguid);
                }
                break;
            }
            if (find_nuch && find_izm && !flag_write) {
                //Ничего не изминилось, можно не записывать
                break;
            }
        }
    } else {
        msg_rep ("ChangeMarkerText", "ACAPI_Element_GetMemo", err, markerguid);
    }
    ACAPI_DisposeElemMemoHdls (&memo);
    return;
}

bool GetMarkerPos (API_Guid & markerguid, API_Coord & startpoint)
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

bool GetMarkerText (API_Guid & markerguid, GS::UniString & note, GS::UniString & nuch, GS::UniString & nizm, GS::Int32 & typeizm, GS::UniString & fam)
{
    GSErrCode err = NoError;
    API_ElementMemo memo;
    BNZeroMemory (&memo, sizeof (API_ElementMemo));
    err = ACAPI_Element_GetMemo (markerguid, &memo, APIMemoMask_AddPars);
    bool find_nuch = false; bool find_izm = false; bool find_note = false; bool find_type = false; bool find_fam = false;
    if (err == NoError) {
        Int32	addParNum = BMGetHandleSize ((GSHandle) memo.params) / sizeof (API_AddParType);
        for (Int32 i = 0; i < addParNum; ++i) {
            API_AddParType& actualParam = (*memo.params)[i];
            GS::UniString name = actualParam.name;
            if (name.Contains ("somestuff_change_text")) {
                note = actualParam.value.uStr;
                note.Trim ();
                find_note = true;
            }
            if (name.Contains ("somestuff_nuch_change")) {
                nuch = actualParam.value.uStr;
                nuch.Trim ();
                find_nuch = true;
            }
            if (name.Contains ("somestuff_nizm_change")) {
                nizm = actualParam.value.uStr;
                nizm.Trim ();
                find_izm = true;
            }
            if (name.Contains ("somestuff_type_change")) {
                typeizm = (int) actualParam.value.real;
                find_type = true;
            }
            if (name.Contains ("somestuff_surname")) {
                fam = actualParam.value.uStr;
                fam.Trim ();
                find_fam = true;
            }

            if (find_nuch && find_izm && find_note && find_type && find_fam) {
                ACAPI_DisposeElemMemoHdls (&memo);
                return true;
            }
        }
    } else {
        msg_rep ("GetChangesMarker", "ACAPI_Element_GetMemo_2", err, markerguid);
    }
    ACAPI_DisposeElemMemoHdls (&memo);
    return find_nuch && find_izm && find_note;
}

bool GetChangesMarker (ChangeMarkerDict & changes)
{
    GS::UniString izmString = RSGetIndString (ID_ADDON_STRINGS + isEng (), Izm_StringID, ACAPI_GetOwnResModule ());
    GS::UniString zamString = RSGetIndString (ID_ADDON_STRINGS + isEng (), Zam_StringID, ACAPI_GetOwnResModule ());
    GS::UniString novString = RSGetIndString (ID_ADDON_STRINGS + isEng (), Nov_StringID, ACAPI_GetOwnResModule ());
    GS::UniString annulString = RSGetIndString (ID_ADDON_STRINGS + isEng (), Annul_StringID, ACAPI_GetOwnResModule ());
    GSErrCode err = NoError;
    API_ElemTypeID elementType = API_ChangeMarkerID;
    GS::Array<API_Guid>	guidArray;
    err = ACAPI_Element_GetElemList (elementType, &guidArray);
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
                GS::UniString fam = "";
                GS::UniString note = ""; GS::UniString nuch = ""; GS::UniString nizm = ""; API_Coord startpoint; GS::Int32 typeizm = TypeNone;
                if (GetMarkerPos (guidArray[i], startpoint) && GetMarkerText (element.changeMarker.markerGuid, note, nuch, nizm, typeizm, fam)) {
                    Change ch;
                    GS::UniString changeId = element.changeMarker.changeId;
                    changeId.Trim ();
                    changeId.ReplaceAll ("  ", " ");
                    GS::Array<GS::UniString> partstring;
                    UInt32 n = StringSplt (changeId, " ", partstring);
                    ch.changeId = changeId;
                    if (n > 3) {
                        if (typeizm > TypeNone && typeizm <= TypeAnnul) {
                            ch.typeizm = typeizm;
                        } else {
                            if (ch.typeizm == TypeNone && partstring[2].Contains (izmString)) ch.typeizm = TypeIzm;
                            if (ch.typeizm == TypeNone && partstring[2].Contains (zamString)) ch.typeizm = TypeZam;
                            if (ch.typeizm == TypeNone && partstring[2].Contains (novString)) ch.typeizm = TypeNov;
                            if (ch.typeizm == TypeNone && partstring[2].Contains (annulString)) ch.typeizm = TypeAnnul;
                        }
                        ch.changeId = partstring[0] + " " + partstring[1] + " " + partstring[3];
                        nizm = partstring[3];
                        nizm.Trim ();
                    }
                    if (ch.typeizm != TypeNone && !nizm.IsEqual ("0")) {
                        ch.markerguid = element.changeMarker.markerGuid;
                        ch.changeName = element.changeMarker.changeName;
                        ch.startpoint = startpoint;
                        ch.nizm = nizm;
                        ch.nuch = nuch;
                        ch.note = note;
                        ch.fam = fam;
                        ch.changeId.Trim ();
                        ch.changeName.Trim ();
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
                        change.arr[i].nuch = GS::UniString::Printf ("%d", number_n);
                        ChangeMarkerText (change.arr[i].markerguid, change.arr[i].nuch, change.arr[i].nizm);
                        change.nuch = number_n;
                    }
                }
            }
            return NoError;
        });
    }
    return !changes.empty ();
}

bool GetScheme (GS::HashTable< GS::UniString, API_Guid>&layout_note_guid)
{
    GSErrCode err = NoError;
    API_LayoutBook layoutScheme;
    err = ACAPI_Database (APIDb_GetLayoutBookID, &layoutScheme);
    if (err != NoError) return false;
    for (auto layout : layoutScheme.customScheme) {
        GS::UniString name = *layout.value;
        name = name.ToLowerCase ();
        API_Guid guid = *layout.key;
        if (name.Contains ("somestuff_")) {
            layout_note_guid.Add (name, guid);
        }
    }
    return !layout_note_guid.IsEmpty ();
}

bool GetAllChangesMarker (GS::HashTable< GS::UniString, API_Guid>&layout_note_guid)
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
                        GS::UniString nizm = "";
                        GS::UniString changeId = c.id;
                        changeId.Trim ();
                        changeId.ReplaceAll ("  ", " ");
                        GS::Array<GS::UniString> partstring;
                        UInt32 n = StringSplt (changeId, " ", partstring);
                        ch.changeId = changeId;
                        if (n > 3) {
                            if (ch.typeizm == TypeNone && partstring[2].Contains (izmString)) ch.typeizm = TypeIzm;
                            if (ch.typeizm == TypeNone && partstring[2].Contains (zamString)) ch.typeizm = TypeZam;
                            if (ch.typeizm == TypeNone && partstring[2].Contains (novString)) ch.typeizm = TypeNov;
                            if (ch.typeizm == TypeNone && partstring[2].Contains (annulString)) ch.typeizm = TypeAnnul;
                            ch.changeId = partstring[0] + " " + partstring[1] + " " + partstring[3];
                            nizm = partstring[3];
                            nizm.Trim ();
                        }
                        if (ch.typeizm != TypeNone && !nizm.IsEqual ("0")) {
                            ch.markerguid = APINULLGuid;
                            ch.changeName = c.description;
                            ch.changeName.Trim ();
                            ch.nizm = nizm;
                            std::string key = ch.changeId.ToCStr (0, MaxUSize, GChCode).Get ();
                            if (changes.count (key) == 0) changes[key] = {};
                            changes[key].arr.Push (ch);
                        }
                    }
                    for (ChangeMarkerDict::iterator ch = changes.begin (); ch != changes.end (); ++ch) {
                        Changes& change = ch->second;
                        for (UInt32 i = 0; i < change.arr.GetSize (); i++) {
                            if (change.arr[i].fam.GetLength () > 3 && change.fam.GetLength () > 3 && !change.fam.IsEqual (change.arr[i].fam)) {
                                msg_rep ("GetChangesMarker", "Different surname", err, APINULLGuid);
                            }
                            if (change.fam.IsEmpty () && change.arr[i].fam.GetLength () > 3) change.fam = change.arr[i].fam;
                            if (change.typeizm != TypeNone && change.arr[i].typeizm != TypeNone && change.arr[i].typeizm != change.typeizm) {
                                msg_rep ("GetChangesMarker", "Different type", err, APINULLGuid);
                            }
                            if (change.typeizm == TypeNone && change.arr[i].typeizm != TypeNone) change.typeizm = change.arr[i].typeizm;
                            if (change.changeId.IsEmpty ()) change.changeId = change.arr[i].changeId;
                        }
                    }
                    GS::UniString key = revision.layoutInfo.id + revision.layoutInfo.name;
                    std::string key_ = key.ToCStr (0, MaxUSize, GChCode).Get ();
                    if (allchanges.count (key_) == 0) allchanges[key_] = changes;
                    /// Запись в свойства макета
                    bool flag_write = false;
                    API_LayoutInfo	layoutInfo;			// temporary here
                    BNZeroMemory (&layoutInfo, sizeof (API_LayoutInfo));
                    err = ACAPI_Environment (APIEnv_GetLayoutSetsID, &layoutInfo, &(dbInfo.databaseUnId));
                    if (err == NoError) {
                        if (layoutInfo.customData != nullptr) {
                            UInt32 n_izm = 1;
                            UInt32 n_prop = layout_note_guid.GetSize ();
                            for (ChangeMarkerDict::iterator ch = changes.begin (); ch != changes.end (); ++ch) {
                                GS::UniString prop_name = GS::UniString::Printf ("somestuff_qtyissue_%d", n_izm);
                                if (layout_note_guid.ContainsKey (prop_name)) {
                                    API_Guid prop_guid = layout_note_guid.Get (prop_name);
                                    Changes change = ch->second;
                                    GS::UniString str = change.changeId;
                                    str = str + "@" + GS::UniString::Printf ("%d", change.nuch);
                                    if (change.typeizm == TypeIzm) str = str + "@" + izmString;
                                    if (change.typeizm == TypeZam) str = str + "@" + zamString;
                                    if (change.typeizm == TypeNov) str = str + "@" + novString;
                                    if (change.typeizm == TypeAnnul) str = str + "@" + annulString;
                                    if (!change.fam.IsEmpty ()) str = str + "@" + change.fam;
                                    layoutInfo.customData->Set (prop_guid, str);
                                } else {
                                    DBprnt ("GetChangesMarker err", "not found " + prop_name);
                                }
                                n_izm += 1;
                            }
                            if (n_izm < n_prop) {
                                for (UInt32 i = n_izm; i < n_prop; i++) {
                                    GS::UniString prop_name = GS::UniString::Printf ("somestuff_qtyissue_%d", i);
                                    if (layout_note_guid.ContainsKey (prop_name)) {
                                        API_Guid prop_guid = layout_note_guid.Get (prop_name);
                                        GS::UniString str = "";
                                        layoutInfo.customData->Set (prop_guid, str);
                                    } else {
                                        DBprnt ("GetChangesMarker err", "not found " + prop_name);
                                    }
                                }
                            }
                            err = ACAPI_Environment (APIEnv_ChangeLayoutSetsID, &layoutInfo, &(dbInfo.databaseUnId));
                            if (err != NoError) msg_rep ("GetChangesMarker", "APIEnv_ChangeLayoutSetsID", err, APINULLGuid);
                            delete layoutInfo.customData;
                        }
                    } else {
                        msg_rep ("GetChangesMarker", "APIEnv_GetLayoutSetsID", err, APINULLGuid);
                    }
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
    GS::HashTable<GS::UniString, API_Guid> layout_note_guid;
    if (!GetScheme (layout_note_guid)) return;

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
    bool haschanges = GetAllChangesMarker (layout_note_guid);

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
}
#endif
