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
    if (!GetScheme (layout_note_guid)) {
        msg_rep ("SetRevision", "Check layout property - somestuff property not found", err, APINULLGuid, true);
        return;
    }
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
    GetAllChangesMarker (layout_note_guid);

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
    msg_rep ("SetRevision", "end", NoError, APINULLGuid);
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
        if (name.Contains ("somestuff_layout_type")) {
            layout_note_guid.Add ("somestuff_layout_type", guid);
        }
    }
    GS::HashTable<API_Guid, GS::UniString> customScheme;
#if defined(AC_27) || defined(AC_28)
    err = ACAPI_Revision_GetRVMChangeCustomScheme (&customScheme);
#else
    err = ACAPI_Database (APIDb_GetRVMChangeCustomSchemeID, &customScheme);
#endif
    for (auto rev : customScheme) {
#if defined(AC_28)
        GS::UniString name = rev.value;
        API_Guid guid = rev.key;
#else
        GS::UniString name = *rev.value;
        API_Guid guid = *rev.key;
#endif
        name = name.ToLowerCase ();
        if (name.Contains ("somestuff_code_change")) {
            layout_note_guid.Add ("somestuff_code_change", guid);
            break;
        }
    }
    return !layout_note_guid.IsEmpty ();
}

void GetAllChangesMarker (GS::HashTable< GS::UniString, API_Guid>& layout_note_guid)
{
    GSErrCode err = NoError;
    // Получаем выпуски
    NoteByChangeDict allchanges;
    LayoutRevisionDict layoutRVI;
    GS::Array<API_RVMDocumentRevision> api_revisions;
#if defined(AC_27) || defined(AC_28)
    err = ACAPI_Revision_GetRVMDocumentRevisions (&api_revisions);
#else
    err = ACAPI_Database (APIDb_GetRVMDocumentRevisionsID, &api_revisions);
#endif
    if (err != NoError) {
        msg_rep ("GetChangesMarker", "APIDb_GetRVMDocumentRevisionsID", err, APINULLGuid);
        return;
    }
    bool isteamwork = false;
    short ownner_userid = 0;
    err = IsTeamwork (isteamwork, ownner_userid);
    for (auto revision : api_revisions) {
        GS::Array<API_RVMChange> api_changes;
#if defined(AC_27) || defined(AC_28)
        err = ACAPI_Revision_GetRVMDocumentRevisionChanges (&revision.guid, &api_changes);
#else
        err = ACAPI_Database (APIDb_GetRVMDocumentRevisionChangesID, &revision.guid, &api_changes);
#endif
        if (err == NoError) {
            if (isteamwork) {
                short userId;
#if defined(AC_27) || defined(AC_28)
                err = ACAPI_Teamwork_GetTWOwner (&revision.layoutInfo.dbId, &userId);
#else
                err = ACAPI_Database (APIDb_GetTWOwnerID, &revision.layoutInfo.dbId, &userId);
#endif
                if (err == NoError) {
                    if (userId != ownner_userid) {
                        continue;
                    }
                } else {
                    continue;
                }
            }
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
                GS::Array<API_RVMChange> layoutchange;
#if defined(AC_27) || defined(AC_28)
                err = ACAPI_Revision_GetRVMLayoutCurrentRevisionChanges (&(dbInfo.databaseUnId), &layoutchange);
#else
                err = ACAPI_Database (APIDb_GetRVMLayoutCurrentRevisionChangesID, &(dbInfo.databaseUnId), &layoutchange);
#endif
                if (!layoutchange.IsEmpty ()) {
                    // Обрабатываем маркеры
                    GetChangesMarker (changes);
                    GetChangesLayout (layoutchange, changes, layout_note_guid);
                    CheckChanges (changes, revision.layoutInfo.subsetName, revision.layoutInfo.id);
                    ChangeMarkerTextOnLayout (changes);
                    ChangeLayoutProperty (changes, layout_note_guid, dbInfo.databaseUnId, revision.layoutInfo.id, layoutRVI, allchanges);
                }
            } else {
                msg_rep ("GetChangesMarker", "APIDb_ChangeCurrentDatabaseID", err, APINULLGuid);
            }
        } else {
            msg_rep ("GetChangesMarker", "APIDb_GetRVMDocumentRevisionChangesID", err, revision.guid);
        }
    }
    if (layoutRVI.IsEmpty () || allchanges.IsEmpty ()) return;
    for (auto ch : allchanges) {
#if defined(AC_28)
        NoteDict change = ch.value;
        GS::UniString id = ch.key;
#else
        NoteDict change = *ch.value;
        GS::UniString id = *ch.key;
#endif
        if (!layoutRVI.ContainsKey (id)) continue;
        API_DatabaseUnId databaseUnId = layoutRVI.Get (id);
        bool flag_write = false;
        API_LayoutInfo	layoutInfo;
#if defined(AC_27) || defined(AC_28)
        err = ACAPI_Navigator_GetLayoutSets (&layoutInfo, &(databaseUnId));
#else
        err = ACAPI_Environment (APIEnv_GetLayoutSetsID, &layoutInfo, &(databaseUnId));
#endif
        if (err != NoError) {
            msg_rep ("GetAllChangesMarker", "APIEnv_GetLayoutSetsID", err, APINULLGuid);
            continue;
        }
        UInt32 n_prop = layout_note_guid.GetSize ();
        // Очистка полей
        for (UInt32 i = 0; i < n_prop; i++) {
            GS::UniString prop_name = GS::UniString::Printf ("somestuff_qtyissue_%d", i);
            if (layout_note_guid.ContainsKey (prop_name)) {
                API_Guid prop_guid = layout_note_guid.Get (prop_name);
                GS::UniString str = "";
                if (layoutInfo.customData->ContainsKey (prop_guid)) {
                    flag_write = true;
                    layoutInfo.customData->Set (prop_guid, str);
                }
            }
            for (UInt32 j = 1; j < 6; j++) {
                GS::UniString prop_name = GS::UniString::Printf ("somestuff_izm_%d_column_%d", i, j);
                if (layout_note_guid.ContainsKey (prop_name)) {
                    API_Guid prop_guid = layout_note_guid.Get (prop_name);
                    GS::UniString str = "";
                    if (layoutInfo.customData->ContainsKey (prop_guid)) {
                        flag_write = true;
                        layoutInfo.customData->Set (prop_guid, str);
                    }
                }
            }
        }
        GS::Int32 n_row = 1;
        for (auto n : change) {
#if defined(AC_28)
            Notes note = n.value;
            GS::UniString note_txt = n.key;
#else
            Notes note = *n.value;
            GS::UniString note_txt = *n.key;
#endif
            if (note_txt.IsEmpty ()) continue;
            // Запись для таблицы регистрации
            GS::UniString prop_name = GS::UniString::Printf ("somestuff_izm_%d_column_1", n_row);
            if (layout_note_guid.ContainsKey (prop_name)) {
                API_Guid prop_guid = layout_note_guid.Get (prop_name);
                GS::UniString str = note.nizm;
                flag_write = true;
                if (layoutInfo.customData->ContainsKey (prop_guid)) {
                    layoutInfo.customData->Set (prop_guid, str);
                } else {
                    layoutInfo.customData->Put (prop_guid, str);
                }
            } else {
                msg_rep ("ChangeLayoutProperty err", "not found " + prop_name, err, APINULLGuid, false);
            }
            prop_name = GS::UniString::Printf ("somestuff_izm_%d_column_2", n_row);
            if (layout_note_guid.ContainsKey (prop_name)) {
                API_Guid prop_guid = layout_note_guid.Get (prop_name);
                GS::UniString str = "";
                // Правильная сортировка по алфавиту
                std::map<std::string, GS::UniString, doj::alphanum_less<std::string> > abc_changes = {};
                for (auto l : note.layoutId) {
#if defined(AC_28)
                    GS::Int32 change = l.value;
                    GS::UniString id = l.key;
#else
                    GS::Int32 change = *l.value;
                    GS::UniString id = *l.key;
#endif
                    std::string s = id.ToCStr (0, MaxUSize, GChCode).Get ();
                    abc_changes[s] = id;
                }
                for (std::map<std::string, GS::UniString, doj::alphanum_less<std::string> >::iterator k = abc_changes.begin (); k != abc_changes.end (); ++k) {
                    GS::UniString s = k->second;
                    if (str.IsEmpty ()) {
                        str = s;
                    } else {
                        str = str + ", " + s;
                    }
                }
                flag_write = true;
                if (layoutInfo.customData->ContainsKey (prop_guid)) {
                    layoutInfo.customData->Set (prop_guid, str);
                } else {
                    layoutInfo.customData->Put (prop_guid, str);
                }
            } else {
                msg_rep ("ChangeLayoutProperty err", "not found " + prop_name, err, APINULLGuid, false);
            }
            prop_name = GS::UniString::Printf ("somestuff_izm_%d_column_3", n_row);
            if (layout_note_guid.ContainsKey (prop_name) && !note_txt.IsEmpty ()) {
                API_Guid prop_guid = layout_note_guid.Get (prop_name);
                GS::UniString str = note_txt;
                flag_write = true;
                if (layoutInfo.customData->ContainsKey (prop_guid)) {
                    layoutInfo.customData->Set (prop_guid, str);
                } else {
                    layoutInfo.customData->Put (prop_guid, str);
                }
            } else {
                if (!note_txt.IsEmpty ()) msg_rep ("ChangeLayoutProperty err", "not found " + prop_name, err, APINULLGuid, false);
            }
            prop_name = GS::UniString::Printf ("somestuff_izm_%d_column_4", n_row);
            if (layout_note_guid.ContainsKey (prop_name) && note.code > 0) {
                API_Guid prop_guid = layout_note_guid.Get (prop_name);
                GS::UniString str = GS::UniString::Printf ("%d", note.code);
                flag_write = true;
                if (layoutInfo.customData->ContainsKey (prop_guid)) {
                    layoutInfo.customData->Set (prop_guid, str);
                } else {
                    layoutInfo.customData->Put (prop_guid, str);
                }
            } else {
                if (note.code > 0) msg_rep ("ChangeLayoutProperty err", "not found " + prop_name, err, APINULLGuid, false);
            }
            n_row += 1;
        }
        if (flag_write) {
            DBprnt ("GetAllChangesMarker::APIEnv_ChangeLayoutSetsID", "start");
#if defined(AC_27) || defined(AC_28)
            err = ACAPI_Navigator_ChangeLayoutSets (&layoutInfo, &(databaseUnId));
#else
            err = ACAPI_Environment (APIEnv_ChangeLayoutSetsID, &layoutInfo, &(databaseUnId));
#endif
            DBprnt ("GetAllChangesMarker::APIEnv_ChangeLayoutSetsID", "end");
            if (err != NoError) msg_rep ("GetAllChangesMarker", "APIEnv_ChangeLayoutSetsID", err, APINULLGuid);
        }
        delete layoutInfo.customData;
    }
}

bool ChangeLayoutProperty (ChangeMarkerDict& changes, GS::HashTable<GS::UniString, API_Guid>& layout_note_guid, API_DatabaseUnId& databaseUnId, GS::UniString& layoutId, LayoutRevisionDict& layoutRVI, NoteByChangeDict& allchanges)
{
    /// Запись в свойства макета
    bool flag_write = false;
    API_LayoutInfo	layoutInfo;
    BNZeroMemory (&layoutInfo, sizeof (API_LayoutInfo));
    GSErrCode err = NoError;
#if defined(AC_27) || defined(AC_28)
    err = ACAPI_Navigator_GetLayoutSets (&layoutInfo, &(databaseUnId));
#else
    err = ACAPI_Environment (APIEnv_GetLayoutSetsID, &layoutInfo, &(databaseUnId));
#endif
    if (err != NoError) {
        msg_rep ("ChangeLayoutProperty", "APIEnv_GetLayoutSetsID", err, APINULLGuid);
        return false;
    }
    if (layoutInfo.customData == nullptr) {
        msg_rep ("ChangeLayoutProperty", "layoutInfo.customData == nullptr", err, APINULLGuid);
        return false;
    }
    Int32 isEng_ = isEng ();
    GS::UniString RVIString = RSGetIndString (ID_ADDON_STRINGS + isEng_, RVI_StringID, ACAPI_GetOwnResModule ());
    GS::UniString izmString = RSGetIndString (ID_ADDON_STRINGS + isEng_, Izm_StringID, ACAPI_GetOwnResModule ());
    GS::UniString zamString = RSGetIndString (ID_ADDON_STRINGS + isEng_, Zam_StringID, ACAPI_GetOwnResModule ());
    GS::UniString novString = RSGetIndString (ID_ADDON_STRINGS + isEng_, Nov_StringID, ACAPI_GetOwnResModule ());
    GS::UniString annulString = RSGetIndString (ID_ADDON_STRINGS + isEng_, Annul_StringID, ACAPI_GetOwnResModule ());
    // Правильная сортировка по алфавиту
    std::map<std::string, GS::UniString, doj::alphanum_less<std::string> > abc_changes = {};
    for (auto ch : changes) {
#if defined(AC_28)
        Changes& change = ch.value;
        GS::UniString id = ch.key;
#else
        Changes& change = *ch.value;
        GS::UniString id = *ch.key;
#endif
        std::string s = change.changeId.ToCStr (0, MaxUSize, GChCode).Get ();
        abc_changes[s] = id;
    }
    UInt32 n_prop = layout_note_guid.GetSize ();
    // Очистка полей
    for (UInt32 i = 0; i < n_prop; i++) {
        GS::UniString prop_name = GS::UniString::Printf ("somestuff_qtyissue_%d", i);
        if (layout_note_guid.ContainsKey (prop_name)) {
            API_Guid prop_guid = layout_note_guid.Get (prop_name);
            GS::UniString str = "";
            if (layoutInfo.customData->ContainsKey (prop_guid)) {
                flag_write = true;
                layoutInfo.customData->Set (prop_guid, str);
            }
        }
        for (UInt32 j = 1; j < 6; j++) {
            GS::UniString prop_name = GS::UniString::Printf ("somestuff_izm_%d_column_%d", i, j);
            if (layout_note_guid.ContainsKey (prop_name)) {
                API_Guid prop_guid = layout_note_guid.Get (prop_name);
                GS::UniString str = "";
                if (layoutInfo.customData->ContainsKey (prop_guid)) {
                    flag_write = true;
                    layoutInfo.customData->Set (prop_guid, str);
                }
            }
        }
    }
    GS::UniString prop_name = "somestuff_note";
    if (layout_note_guid.ContainsKey (prop_name)) {
        API_Guid prop_guid = layout_note_guid.Get (prop_name);
        flag_write = true;
        if (layoutInfo.customData->ContainsKey (prop_guid)) layoutInfo.customData->Set (prop_guid, "");
    }
    UInt32 n_izm = 1;
    GS::UniString note = "";
    for (std::map<std::string, GS::UniString, doj::alphanum_less<std::string> >::iterator k = abc_changes.begin (); k != abc_changes.end (); ++k) {
        GS::UniString s = k->second;
        Changes change;
        if (changes.ContainsKey (s)) change = changes.Get (s);
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
            flag_write = true;
            if (layoutInfo.customData->ContainsKey (prop_guid)) {
                layoutInfo.customData->Set (prop_guid, str);
            } else {
                layoutInfo.customData->Put (prop_guid, str);
            }
        } else {
            msg_rep ("ChangeLayoutProperty err", "not found " + prop_name, err, APINULLGuid, false);
        }
        if (note.IsEmpty ()) note = "Изм. ";
        GS::UniString str = change.nizm;
        if (change.typeizm == TypeZam) str = str + " (" + zamString + ".)";
        if (change.typeizm == TypeNov) str = str + " (" + novString + ".)";
        if (change.typeizm == TypeAnnul) str = str + " (" + annulString + ".)";
        str = str + "; ";
        note = note + str;
        n_izm += 1;
        // Запись для таблицы регистрации
        prop_name = "somestuff_izm_" + change.nizm + "_column_1";
        if (layout_note_guid.ContainsKey (prop_name)) {
            API_Guid prop_guid = layout_note_guid.Get (prop_name);
            GS::UniString str = change.nizm;
            flag_write = true;
            if (layoutInfo.customData->ContainsKey (prop_guid)) {
                layoutInfo.customData->Set (prop_guid, str);
            } else {
                layoutInfo.customData->Put (prop_guid, str);
            }
        } else {
            msg_rep ("ChangeLayoutProperty err", "not found " + prop_name, err, APINULLGuid, false);
        }
        prop_name = "somestuff_izm_" + change.nizm + "_column_2";
        if (layout_note_guid.ContainsKey (prop_name)) {
            API_Guid prop_guid = layout_note_guid.Get (prop_name);
            GS::UniString str = layoutId;
            flag_write = true;
            if (layoutInfo.customData->ContainsKey (prop_guid)) {
                layoutInfo.customData->Set (prop_guid, str);
            } else {
                layoutInfo.customData->Put (prop_guid, str);
            }
        } else {
            msg_rep ("ChangeLayoutProperty err", "not found " + prop_name, err, APINULLGuid, false);
        }

        prop_name = "somestuff_izm_" + change.nizm + "_column_3";
        if (layout_note_guid.ContainsKey (prop_name) && !change.note.IsEmpty ()) {
            API_Guid prop_guid = layout_note_guid.Get (prop_name);
            GS::UniString str = change.note;
            str.ReplaceAll ("@", "");
            flag_write = true;
            if (layoutInfo.customData->ContainsKey (prop_guid)) {
                layoutInfo.customData->Set (prop_guid, str);
            } else {
                layoutInfo.customData->Put (prop_guid, str);
            }
        } else {
            if (!change.note.IsEmpty ()) msg_rep ("ChangeLayoutProperty err", "not found " + prop_name, err, APINULLGuid, false);
        }

        prop_name = "somestuff_izm_" + change.nizm + "_column_4";
        if (layout_note_guid.ContainsKey (prop_name) && change.code > 0) {
            API_Guid prop_guid = layout_note_guid.Get (prop_name);
            GS::UniString str = GS::UniString::Printf ("%d", change.code);
            flag_write = true;
            if (layoutInfo.customData->ContainsKey (prop_guid)) {
                layoutInfo.customData->Set (prop_guid, str);
            } else {
                layoutInfo.customData->Put (prop_guid, str);
            }
        } else {
            if (change.code > 0) msg_rep ("ChangeLayoutProperty err", "not found " + prop_name, err, APINULLGuid, false);
        }
        //Добавление листов с РВИ в словарь
        prop_name = "somestuff_layout_type";
        if (layout_note_guid.ContainsKey (prop_name)) {
            API_Guid prop_guid = layout_note_guid.Get (prop_name);
            if (layoutInfo.customData->ContainsKey (prop_guid)) {
                GS::UniString type = layoutInfo.customData->Get (prop_guid).ToLowerCase ();
                if (type.IsEqual (RVIString)) {
                    if (!layoutRVI.ContainsKey (change.changeId)) {
                        layoutRVI.Add (change.changeId, databaseUnId);
                    }
                }
            }
            if (!allchanges.ContainsKey (change.changeId)) {
                NoteDict notesd;
                allchanges.Add (change.changeId, notesd);
            }
            if (allchanges.ContainsKey (change.changeId)) {
                GS::Array<GS::UniString> partnote;
                UInt32 n = StringSplt (change.note, ";@", partnote);
                for (auto noteone : partnote) {
                    if (!allchanges.Get (change.changeId).ContainsKey (noteone)) {
                        Notes notesd;
                        notesd.code = change.code;
                        notesd.nizm = change.nizm;
                        allchanges.Get (change.changeId).Add (noteone, notesd);
                    }
                    if (allchanges.Get (change.changeId).ContainsKey (noteone)) {
                        if (!allchanges.Get (change.changeId).Get (noteone).layoutId.ContainsKey (layoutId)) {
                            allchanges.Get (change.changeId).Get (noteone).layoutId.Add (layoutId, change.typeizm);
                        }
                    }
                }
            }

        }
    }
    prop_name = "somestuff_note";
    if (layout_note_guid.ContainsKey (prop_name) && !note.IsEmpty ()) {
        note.Trim ();
        note.Trim (';');
        API_Guid prop_guid = layout_note_guid.Get (prop_name);
        flag_write = true;
        if (layoutInfo.customData->ContainsKey (prop_guid)) {
            layoutInfo.customData->Set (prop_guid, note);
        } else {
            layoutInfo.customData->Put (prop_guid, note);
        }
    } else {
        if (!note.IsEmpty ()) msg_rep ("ChangeLayoutProperty err", "not found " + prop_name, err, APINULLGuid, false);
    }
    if (flag_write) {
        DBprnt ("GetChangesMarker::APIEnv_ChangeLayoutSetsID", "start");
#if defined(AC_27) || defined(AC_28)
        err = ACAPI_Navigator_ChangeLayoutSets (&layoutInfo, &(databaseUnId));
#else
        err = ACAPI_Environment (APIEnv_ChangeLayoutSetsID, &layoutInfo, &(databaseUnId));
#endif
        DBprnt ("GetChangesMarker::APIEnv_ChangeLayoutSetsID", "end");
        if (err != NoError) msg_rep ("GetChangesMarker", "APIEnv_ChangeLayoutSetsID", err, APINULLGuid);
    }
    delete layoutInfo.customData;
    return true;
}

void CheckChanges (ChangeMarkerDict& changes, GS::UniString& subsetName, GS::UniString& layoutid)
{
    // Проверка данных в изменениях
    for (auto ch : changes) {
#if defined(AC_28)
        Changes& change = ch.value;
        GS::UniString id = ch.key;
#else
        Changes& change = *ch.value;
        GS::UniString id = *ch.key;
#endif
        GS::UniString note = "";
        // Проверим тиы изменений
        // Если есть облачко с типом Изм - то ставим Изм
        // Если есть облачка без проставленного изменения и у листа стоит Зам - ставим Изм
        GS::Int32 typeizm_marker = TypeNone;
        GS::Int32 typeizm_layout = TypeNone;
        GS::Int32 typeizm = TypeNone;
        GS::UniString fam_marker = "";
        GS::UniString fam_layout = "";
        bool hasmarker = false;
        bool hasmarkerIzm = false;
        for (UInt32 i = 0; i < change.arr.GetSize (); i++) {
            if (change.arr[i].markerguid == APINULLGuid) {
                if (change.arr[i].typeizm != TypeNone) typeizm_layout = change.arr[i].typeizm;
                if (change.arr[i].fam.GetLength () > 3) fam_layout = change.arr[i].fam;
            } else {
                hasmarker = true;
                if (change.arr[i].typeizm == TypeIzm) hasmarkerIzm = true;
                if (typeizm_marker != TypeNone && change.arr[i].typeizm != TypeNone && change.arr[i].typeizm != typeizm_marker) {
                    msg_rep ("GetChangesMarker", "Different type : " + change.arr[i].changeId + " sheet ID " + subsetName + "/" + layoutid, APIERR_GENERAL, APINULLGuid, true);
                }
                if (change.arr[i].fam.GetLength () > 3 && fam_marker.GetLength () > 3 && !fam_marker.IsEqual (change.arr[i].fam)) {
                    msg_rep ("GetChangesMarker", "Different surname : " + fam_marker + "<->" + change.arr[i].fam + " on " + change.arr[i].changeId + " sheet ID " + subsetName + "/" + layoutid, APIERR_GENERAL, APINULLGuid, true);
                }
                if (change.arr[i].fam.GetLength () > 3) fam_marker = change.arr[i].fam;
                if (change.arr[i].typeizm != TypeNone) typeizm_marker = change.arr[i].typeizm;
                if (ACAPI_Element_Filter (change.arr[i].markerguid, APIFilt_InMyWorkspace)) {
                    if (ACAPI_Element_Filter (change.arr[i].markerguid, APIFilt_HasAccessRight)) {
                        if (!ACAPI_Element_Filter (change.arr[i].markerguid, APIFilt_IsEditable)) {
                            msg_rep ("GetChangesMarker", "Marker not editable : " + change.arr[i].changeId + " sheet ID " + subsetName + "/" + layoutid, APIERR_GENERAL, APINULLGuid, true);
                        }
                    } else {
                        msg_rep ("GetChangesMarker", "Marker has not access right : " + change.arr[i].changeId + " sheet ID " + subsetName + "/" + layoutid, APIERR_GENERAL, APINULLGuid, true);
                    }
                } else {
                    msg_rep ("GetChangesMarker", "Marker not in on workspace : " + change.arr[i].changeId + " sheet ID " + subsetName + "/" + layoutid, APIERR_GENERAL, APINULLGuid, true);
                }
            }
        }
        if (fam_marker.GetLength () > 3) fam_layout = fam_marker;
        if (hasmarker && typeizm_marker != TypeNone) typeizm = typeizm_marker;
        if (!hasmarker) typeizm = typeizm_layout;
        if (hasmarkerIzm && hasmarker) typeizm = TypeIzm;
        if (typeizm == TypeNone && hasmarker) typeizm = TypeIzm;
        change.typeizm = typeizm;
        change.fam = fam_layout;
        for (UInt32 i = 0; i < change.arr.GetSize (); i++) {
            change.arr[i].typeizm = typeizm;
            if (change.changeId.IsEmpty ()) change.changeId = change.arr[i].changeId;
            if (change.nizm.IsEmpty ()) change.nizm = change.arr[i].nizm;
            if (!note.IsEmpty () && !change.arr[i].note.IsEmpty ()) note = note + " ;@ " + change.arr[i].note;
            if (note.IsEmpty () && !change.arr[i].note.IsEmpty ()) note = change.arr[i].note;
            if (change.code == 0 && change.arr[i].code > 0) change.code = change.arr[i].code;
        }
        change.note = StringUnic (note, ";@");
    }
}

void GetChangesLayout (GS::Array<API_RVMChange>& layoutchange, ChangeMarkerDict& changes, GS::HashTable<GS::UniString, API_Guid>& layout_note_guid)
{
    Int32 isEng_ = isEng ();
    GS::UniString izmString = RSGetIndString (ID_ADDON_STRINGS + isEng_, Izm_StringID, ACAPI_GetOwnResModule ());
    GS::UniString zamString = RSGetIndString (ID_ADDON_STRINGS + isEng_, Zam_StringID, ACAPI_GetOwnResModule ());
    GS::UniString novString = RSGetIndString (ID_ADDON_STRINGS + isEng_, Nov_StringID, ACAPI_GetOwnResModule ());
    GS::UniString annulString = RSGetIndString (ID_ADDON_STRINGS + isEng_, Annul_StringID, ACAPI_GetOwnResModule ());
    // Обрабатываем изменения, не привязанные к маркерам
    for (auto c : layoutchange) {
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
            if (layout_note_guid.ContainsKey ("somestuff_code_change")) {
                API_Guid guid = layout_note_guid.Get ("somestuff_code_change");
                if (c.customData.ContainsKey (guid)) {
                    GS::UniString code = c.customData.Get (guid);
                    ch.code = std::atoi (code.ToCStr ());
                }
            }
            ch.markerguid = APINULLGuid;
            ch.changeName = c.description;
            ch.changeName.Trim ();
            ch.nizm = nizm;
            GS::UniString key = ch.changeId;
            if (!changes.ContainsKey (key)) {
                Changes chs;
                changes.Add (key, chs);
            }
            changes[key].arr.Push (ch);
        }
    }
}

bool GetChangesMarker (ChangeMarkerDict& changes)
{
    Int32 isEng_ = isEng ();
    GS::UniString izmString = RSGetIndString (ID_ADDON_STRINGS + isEng_, Izm_StringID, ACAPI_GetOwnResModule ());
    GS::UniString zamString = RSGetIndString (ID_ADDON_STRINGS + isEng_, Zam_StringID, ACAPI_GetOwnResModule ());
    GS::UniString novString = RSGetIndString (ID_ADDON_STRINGS + isEng_, Nov_StringID, ACAPI_GetOwnResModule ());
    GS::UniString annulString = RSGetIndString (ID_ADDON_STRINGS + isEng_, Annul_StringID, ACAPI_GetOwnResModule ());
    GSErrCode err = NoError;
    API_ElemTypeID elementType = API_ChangeMarkerID;
    GS::Array<API_Guid>	guidArray;
    err = ACAPI_Element_GetElemList (elementType, &guidArray);
    if (err != NoError) {
        msg_rep ("GetChangesMarker", "ACAPI_Element_GetElemList", err, APINULLGuid);
        return false;
    }
    if (guidArray.IsEmpty ()) return false;
    GS::HashTable < GS::UniString, Changes> ChangeMarkerDict;
    for (UInt32 i = 0; i < guidArray.GetSize (); i++) {
        API_Element element = {};
        BNZeroMemory (&element, sizeof (API_Element));
        element.header.guid = guidArray[i];
        err = ACAPI_Element_Get (&element);
        if (err == NoError && element.header.hasMemo) {
            GS::UniString fam = "";
            GS::UniString note = ""; GS::UniString nuch = ""; GS::UniString nizm = ""; API_Coord startpoint; GS::Int32 typeizm = TypeNone; GS::Int32 code = 0;
            if (GetMarkerPos (guidArray[i], startpoint) && GetMarkerText (element.changeMarker.markerGuid, note, nuch, nizm, typeizm, fam, code)) {
                Change ch;
                GS::UniString changeId = element.changeMarker.changeId;
                changeId.Trim ();
                changeId.ReplaceAll ("  ", " ");
                GS::Array<GS::UniString> partstring;
                UInt32 n = StringSplt (changeId, " ", partstring);
                ch.changeId = changeId;
                if (n > 3) {
                    if (typeizm > TypeNone && typeizm <= TypeAnnul) ch.typeizm = typeizm;
                    ch.changeId = partstring[0] + " " + partstring[1] + " " + partstring[3];
                    nizm = partstring[3];
                    nizm.Trim ();
                }
                if (!nizm.IsEqual ("0")) {
                    ch.markerguid = element.changeMarker.markerGuid;
                    ch.changeName = element.changeMarker.changeName;
                    ch.startpoint = startpoint;
                    ch.nizm = nizm;
                    ch.nuch = nuch;
                    ch.note = note;
                    ch.fam = fam;
                    ch.changeId.Trim ();
                    ch.changeName.Trim ();
                    ch.code = code;
                    GS::UniString key = ch.changeId;
                    if (!changes.ContainsKey (key)) {
                        Changes chs;
                        changes.Add (key, chs);
                    }
                    changes[key].arr.Push (ch);
                }
            }
        }
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

bool GetMarkerText (API_Guid& markerguid, GS::UniString& note, GS::UniString& nuch, GS::UniString& nizm, GS::Int32& typeizm, GS::UniString& fam, GS::Int32& code)
{
    GSErrCode err = NoError;
    API_ElementMemo memo;
    BNZeroMemory (&memo, sizeof (API_ElementMemo));
    err = ACAPI_Element_GetMemo (markerguid, &memo, APIMemoMask_AddPars);
    bool find_nuch = false; bool find_izm = false; bool find_note = false; bool find_type = false; bool find_fam = false; bool find_code = false;
    if (err == NoError) {
        Int32	addParNum = BMGetHandleSize ((GSHandle) memo.params) / sizeof (API_AddParType);
        for (Int32 i = 0; i < addParNum; ++i) {
            API_AddParType& actualParam = (*memo.params)[i];
            GS::UniString name = actualParam.name;
            if (name.Contains ("somestuff_change_text")) {
                note = GS::UniString (actualParam.value.uStr);
                note.Trim ();
                find_note = true;
            }
            if (name.Contains ("somestuff_nuch_change")) {
                nuch = GS::UniString (actualParam.value.uStr);
                nuch.Trim ();
                find_nuch = true;
            }
            if (name.Contains ("somestuff_nizm_change")) {
                nizm = GS::UniString (actualParam.value.uStr);
                nizm.Trim ();
                find_izm = true;
            }
            if (name.Contains ("somestuff_type_change")) {
                typeizm = (int) actualParam.value.real;
                find_type = true;
            }
            if (name.Contains ("somestuff_code_change")) {
                code = (int) actualParam.value.real;
                find_code = true;
            }
            if (name.Contains ("somestuff_surname")) {
                fam = GS::UniString (actualParam.value.uStr);
                fam.Trim ();
                find_fam = true;
            }
            if (find_nuch && find_izm && find_note && find_type && find_fam && find_code) {
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
void ChangeMarkerTextOnLayout (ChangeMarkerDict& changes)
{
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
                if (change.arr[i].typeizm == TypeIzm && change.arr[i].markerguid != APINULLGuid) {
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
                //Ничего не изменилось, можно не записывать
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
