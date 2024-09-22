//------------ kuvbur 2022 ------------
#include	"ACAPinc.h"
#include	"APIEnvir.h"
#include	"Revision.hpp"
#include	<stdlib.h> /* atoi */
#include	<time.h>
namespace Revision
{
void SetRevision (void)
{
    GSErrCode err = NoError;
    GS::HashTable<GS::UniString, API_Guid> layout_note_guid;
    if (!GetScheme (layout_note_guid)) return;

    API_DatabaseInfo databasestart;
    API_WindowInfo windowstart;
    GS::IntPtr	store = 1;
#if defined(AC_27) || defined(AC_28)
    err = ACAPI_View_StoreViewSettings (store);
#else
    err = ACAPI_Database (APIDb_StoreViewSettingsID, (void*) store);
#endif
    if (err != NoError) {
        store = -1;
        err = NoError;
    }
    BNZeroMemory (&databasestart, sizeof (API_DatabaseInfo));
#if defined(AC_27) || defined(AC_28)
    err = ACAPI_Database_GetCurrentDatabase (&databasestart);
#else
    err = ACAPI_Database (APIDb_GetCurrentDatabaseID, &databasestart, nullptr);
#endif
    if (err != NoError) {
        msg_rep ("SetRevision", "APIDb_GetCurrentDatabaseID", err, APINULLGuid);
        return;
    }
    BNZeroMemory (&windowstart, sizeof (API_WindowInfo));
#if defined(AC_27) || defined(AC_28)
    err = ACAPI_Window_GetCurrentWindow (&windowstart);
#else
    err = ACAPI_Database (APIDb_GetCurrentWindowID, &windowstart, nullptr);
#endif
    if (err != NoError) {
        msg_rep ("SetRevision", "APIDb_GetCurrentWindowID", err, APINULLGuid);
        return;
    }
    bool haschanges = GetAllChangesMarker (layout_note_guid);

    // Возвращение на исходную БД и окно
#if defined(AC_27) || defined(AC_28)
    err = ACAPI_Database_ChangeCurrentDatabase (&databasestart);
#else
    err = ACAPI_Database (APIDb_ChangeCurrentDatabaseID, &databasestart, nullptr);
#endif
    if (err != NoError) {
        msg_rep ("SetRevision", "APIDb_ChangeCurrentDatabaseID", err, APINULLGuid);
        return;
    }
#if defined(AC_27) || defined(AC_28)
    err = ACAPI_Window_ChangeWindow (&windowstart);
#else
    err = ACAPI_Automate (APIDo_ChangeWindowID, &windowstart, nullptr);
#endif
    if (err != NoError) {
        msg_rep ("SetRevision", "APIDo_ChangeWindowID", err, APINULLGuid);
        return;
    }
    if (store == 1) {
        store = 0;
#if defined(AC_27) || defined(AC_28)
        ACAPI_View_StoreViewSettings (store);
#else
        ACAPI_Database (APIDb_StoreViewSettingsID, (void*) store);
#endif
    }
}

bool GetScheme (GS::HashTable< GS::UniString, API_Guid>& layout_note_guid)
{
    GSErrCode err = NoError;
    API_LayoutBook layoutScheme;
#if defined(AC_27) || defined(AC_28)
    err = ACAPI_Navigator_GetLayoutBook (&layoutScheme);
#else
    err = ACAPI_Database (APIDb_GetLayoutBookID, &layoutScheme);
#endif
    if (err != NoError) {
        msg_rep ("GetScheme", "APIDb_GetLayoutBookID", err, APINULLGuid);
        return false;
    }
    for (auto layout : layoutScheme.customScheme) {
#if defined(AC_28)
        GS::UniString name = layout.value;
        API_Guid guid = layout.key;
#else
        GS::UniString name = *layout.value;
        API_Guid guid = *layout.key;
#endif
        name = name.ToLowerCase ();
        if (name.Contains ("somestuff_")) {
            layout_note_guid.Add (name, guid);
        }
    }
    return !layout_note_guid.IsEmpty ();
}

bool GetAllChangesMarker (GS::HashTable< GS::UniString, API_Guid>& layout_note_guid)
{
    GSErrCode err = NoError;
    //ChangeMarkerByListDict allchanges;
    // Получаем выпуски
    GS::Array<API_RVMDocumentRevision> api_revisions;
#if defined(AC_27) || defined(AC_28)
    err = ACAPI_Revision_GetRVMDocumentRevisions (&api_revisions);
#else
    err = ACAPI_Database (APIDb_GetRVMDocumentRevisionsID, &api_revisions);
#endif
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
#if defined(AC_27) || defined(AC_28)
        err = ACAPI_Revision_GetRVMDocumentRevisionChanges (&revision.guid, &api_changes);
#else
        err = ACAPI_Database (APIDb_GetRVMDocumentRevisionChangesID, &revision.guid, &api_changes);
#endif
        if (err == NoError) {
            ChangeMarkerDict changes;
            API_DatabaseInfo dbInfo = {};
            dbInfo.typeID = APIWind_LayoutID;
            dbInfo.databaseUnId = revision.layoutInfo.dbId;
#if defined(AC_27) || defined(AC_28)
            err = ACAPI_Database_ChangeCurrentDatabase (&dbInfo);
#else
            err = ACAPI_Database (APIDb_ChangeCurrentDatabaseID, &dbInfo, nullptr);
#endif
            if (err == NoError) {
                GS::Array<API_RVMChange> change;
#if defined(AC_27) || defined(AC_28)
                err = ACAPI_Revision_GetRVMLayoutCurrentRevisionChanges (&(dbInfo.databaseUnId), &change);
#else
                err = ACAPI_Database (APIDb_GetRVMLayoutCurrentRevisionChangesID, &(dbInfo.databaseUnId), &change);
#endif
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
                            GS::UniString key = ch.changeId;
                            if (!changes.ContainsKey (key)) {
                                Changes chs;
                                changes.Add (key, chs);
                            }
                            if (changes[key].typeizm != TypeNone) ch.typeizm = changes[key].typeizm;
                            if (changes[key].typeizm == TypeNone) changes[key].typeizm = ch.typeizm;
                            changes[key].arr.Push (ch);
                        }
                    }
                    for (auto ch : changes) {
#if defined(AC_28)
                        Changes change = ch.value;
#else
                        Changes change = *ch.value;
#endif
                        for (UInt32 i = 0; i < change.arr.GetSize (); i++) {
                            if (change.arr[i].fam.GetLength () > 3 && change.fam.GetLength () > 3 && !change.fam.IsEqual (change.arr[i].fam)) {
                                msg_rep ("GetChangesMarker", "Different surname " + change.fam + "<->" + change.arr[i].fam + " on " + change.changeId + " sheet ID " + revision.layoutInfo.subsetName + "/" + revision.layoutInfo.id, err, APINULLGuid, true);
                            }
                            if (change.fam.IsEmpty () && change.arr[i].fam.GetLength () > 3) change.fam = change.arr[i].fam;
                            if (change.typeizm != TypeNone && change.arr[i].typeizm != TypeNone && change.arr[i].typeizm != change.typeizm) {
                                msg_rep ("GetChangesMarker", "Different type on " + change.changeId + " sheet ID " + revision.layoutInfo.subsetName + "/" + revision.layoutInfo.id, err, APINULLGuid, true);
                            }
                            if (change.typeizm == TypeNone && change.arr[i].typeizm != TypeNone) change.typeizm = change.arr[i].typeizm;
                            if (change.changeId.IsEmpty ()) change.changeId = change.arr[i].changeId;
                            if (change.nizm.IsEmpty ()) change.nizm = change.arr[i].nizm;
                        }
                    }
                    //GS::UniString key = revision.layoutInfo.id + revision.layoutInfo.name;
                    //std::string key_ = key.ToCStr (0, MaxUSize, GChCode).Get ();
                    //if (!allchanges.ContainsKey(key_)) {
                    //    ChangeMarkerDict chs;
                    //    allchanges.Add (key_, chs);
                    //}
                }
                /// Запись в свойства макета
                bool flag_write = false;
                API_LayoutInfo	layoutInfo;			// temporary here
                BNZeroMemory (&layoutInfo, sizeof (API_LayoutInfo));
#if defined(AC_27) || defined(AC_28)
                err = ACAPI_Navigator_GetLayoutSets (&layoutInfo, &(dbInfo.databaseUnId));
#else
                err = ACAPI_Environment (APIEnv_GetLayoutSetsID, &layoutInfo, &(dbInfo.databaseUnId));
#endif
                if (err == NoError) {
                    if (layoutInfo.customData != nullptr) {
                        bool flag_write = false;
                        UInt32 n_izm = 1;
                        UInt32 n_prop = layout_note_guid.GetSize ();
                        GS::UniString note = "";
                        for (auto ch : changes) {
#if defined(AC_28)
                            Changes change = ch.value;
#else
                            Changes change = *ch.value;
#endif
                            GS::UniString prop_name = GS::UniString::Printf ("somestuff_qtyissue_%d", n_izm);
                            if (layout_note_guid.ContainsKey (prop_name)) {
                                API_Guid prop_guid = layout_note_guid.Get (prop_name);
                                GS::UniString str = change.changeId;
                                str = str + "@" + GS::UniString::Printf ("%d", change.nuch);
                                if (change.typeizm == TypeIzm) str = str + "@" + izmString;
                                if (change.typeizm == TypeZam) str = str + "@" + zamString;
                                if (change.typeizm == TypeNov) str = str + "@" + novString;
                                if (change.typeizm == TypeAnnul) str = str + "@" + annulString;
                                if (!change.fam.IsEmpty ()) str = str + "@" + change.fam;
                                if (layoutInfo.customData->ContainsKey (prop_guid)) {
                                    flag_write = true;
                                    layoutInfo.customData->Set (prop_guid, str);
                                } else {
                                    msg_rep ("GetChangesMarker err", "not found " + prop_name, err, APINULLGuid, false);
                                }
                            } else {
                                msg_rep ("GetChangesMarker err", "not found " + prop_name, err, APINULLGuid, false);
                            }
                            if (note.IsEmpty ()) note = "Изм. ";
                            GS::UniString str = change.nizm;
                            if (change.typeizm == TypeZam) str = str + " (" + zamString + ".)";
                            if (change.typeizm == TypeNov) str = str + " (" + novString + ".)";
                            if (change.typeizm == TypeAnnul) str = str + " (" + annulString + ".)";
                            str = str + "; ";
                            note = note + str;
                            n_izm += 1;
                        }
                        GS::UniString prop_name = "somestuff_note";
                        if (layout_note_guid.ContainsKey (prop_name)) {
                            note.Trim ();
                            note.Trim (';');
                            API_Guid prop_guid = layout_note_guid.Get (prop_name);
                            if (layoutInfo.customData->ContainsKey (prop_guid)) {
                                flag_write = true;
                                layoutInfo.customData->Set (prop_guid, note);
                            } else {
                                msg_rep ("GetChangesMarker err", "not found " + prop_name, err, APINULLGuid, false);
                            }
                        } else {
                            msg_rep ("GetChangesMarker err", "not found " + prop_name, err, APINULLGuid, false);
                        }
                        if (n_izm < n_prop) {
                            for (UInt32 i = n_izm; i < n_prop; i++) {
                                GS::UniString prop_name = GS::UniString::Printf ("somestuff_qtyissue_%d", i);
                                if (layout_note_guid.ContainsKey (prop_name)) {
                                    API_Guid prop_guid = layout_note_guid.Get (prop_name);
                                    GS::UniString str = "";
                                    if (layoutInfo.customData->ContainsKey (prop_guid)) {
                                        flag_write = true;
                                        layoutInfo.customData->Set (prop_guid, str);
                                    } else {
                                        msg_rep ("GetChangesMarker err", "not found " + prop_name, err, APINULLGuid, false);
                                    }
                                } else {
                                    msg_rep ("GetChangesMarker err", "not found " + prop_name, err, APINULLGuid, false);
                                }
                            }
                        }
                        if (flag_write) {
                            DBprnt ("GetChangesMarker::APIEnv_ChangeLayoutSetsID", "start");
#if defined(AC_27) || defined(AC_28)
                            err = ACAPI_Navigator_ChangeLayoutSets (&layoutInfo, &(dbInfo.databaseUnId));
#else
                            err = ACAPI_Environment (APIEnv_ChangeLayoutSetsID, &layoutInfo, &(dbInfo.databaseUnId));
#endif
                            DBprnt ("GetChangesMarker::APIEnv_ChangeLayoutSetsID", "end");
                            if (err != NoError) msg_rep ("GetChangesMarker", "APIEnv_ChangeLayoutSetsID", err, APINULLGuid);
                        }
                        delete layoutInfo.customData;
                    }
                } else {
                    msg_rep ("GetChangesMarker", "APIEnv_GetLayoutSetsID", err, APINULLGuid);
                }
            } else {
                msg_rep ("GetChangesMarker", "APIDb_ChangeCurrentDatabaseID", err, APINULLGuid);
            }
        } else {
            msg_rep ("GetChangesMarker", "APIDb_GetRVMDocumentRevisionChangesID", err, revision.guid);
        }
    }
    return true;
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
                        GS::UniString key = ch.changeId;
                        if (!changes.ContainsKey (key)) {
                            Changes chs;
                            changes.Add (key, chs);
                        }
                        if (changes[key].typeizm == TypeNone) changes[key].typeizm = ch.typeizm;
                        changes[key].arr.Push (ch);
                    }
                }
            }
        }
    }
    if (!changes.IsEmpty ()) {
        GS::UniString undoString = RSGetIndString (ID_ADDON_STRINGS + isEng (), UndoReNumId, ACAPI_GetOwnResModule ());
        ACAPI_CallUndoableCommand (undoString, [&]() -> GSErrCode {
            for (auto ch : changes) {
#if defined(AC_28)
                Changes& change = ch.value;
#else
                Changes& change = *ch.value;
#endif
                int number_n = 0;
                for (UInt32 i = 0; i < change.arr.GetSize (); i++) {
                    if (change.arr[i].typeizm == TypeIzm) {
                        number_n += 1;
                        change.arr[i].nuch = GS::UniString::Printf ("%d", number_n);
                        change.nuch = number_n;
                    } else {
                        change.arr[i].nuch = "";
                    }
                    ChangeMarkerText (change.arr[i].markerguid, change.arr[i].nuch, change.arr[i].nizm);
                }
            }
            return NoError;
        });
    }
    return !changes.IsEmpty ();
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

bool GetMarkerText (API_Guid& markerguid, GS::UniString& note, GS::UniString& nuch, GS::UniString& nizm, GS::Int32& typeizm, GS::UniString& fam)
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
    if (ACAPI_Teamwork_HasConnection () && !ACAPI_Element_Filter (markerguid, APIFilt_InMyWorkspace)) {
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
                DBprnt ("ACAPI_Element_Change", "start");
                err = ACAPI_Element_Change (&markerElement, &markerMask, &memo, APIMemoMask_AddPars, true);
                if (err != NoError) {
                    msg_rep ("ChangeMarkerText", "ACAPI_Element_Change", err, markerguid);
                }
                DBprnt ("ACAPI_Element_Change", "end");
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
}
