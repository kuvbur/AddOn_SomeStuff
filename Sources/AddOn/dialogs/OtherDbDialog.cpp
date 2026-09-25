//------------ kuvbur 2022 ------------
#include "ACAPinc.h"

#include "api_headers/APIEnvir.h"
#include "api_headers/ResourceIds.hpp"

#include "dialogs/OtherDbDialog.hpp"

#include "CommonFunction.hpp"
#include "DGModule.hpp"
#include "Propertycache.hpp"

namespace SyncDialogs {
    namespace {
        bool IsSameOtherDbTarget (const OtherDbTarget &target,
                                  const API_DatabaseInfo &dbInfo,
                                  bool hasStory,
                                  short storyIndex) {
            return target.dbInfo.databaseUnId == dbInfo.databaseUnId && target.hasStory == hasStory &&
                   (!hasStory || target.storyIndex == storyIndex);
        }

        void AddOtherDbTargetImpl (GS::Array<OtherDbTarget> &targets,
                                   const API_DatabaseInfo &dbInfo,
                                   bool hasStory,
                                   short storyIndex,
                                   const GS::UniString &name,
                                   const API_Guid &guid) {
            for (auto &target : targets) {
                if (IsSameOtherDbTarget (target, dbInfo, hasStory, storyIndex)) {
                    target.guids.PushNew (guid);
                    return;
                }
            }

            OtherDbTarget target;
            target.dbInfo = dbInfo;
            target.hasStory = hasStory;
            target.storyIndex = storyIndex;
            target.name = name;
            target.guids.Push (guid);
            targets.Push (target);
        }

        GS::UniString GetOtherStoryNameImpl (short storyIndex) {
            API_StoryInfo storyInfo = {};
#ifdef ServerMainVers_2700
            GSErrCode err = ACAPI_ProjectSetting_GetStorySettings (&storyInfo);
#else
            GSErrCode err = ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo, nullptr);
#endif
            if (err != NoError || storyInfo.data == nullptr)
                return GS::UniString::Printf ("Story %d", storyIndex);

            GS::UniString name = GS::UniString::Printf ("Story %d", storyIndex);
            const API_StoryType *stories = reinterpret_cast<const API_StoryType *> (*storyInfo.data);
            for (short i = 0; i < storyInfo.lastStory - storyInfo.firstStory + 1; ++i) {
                if (stories[i].index == storyIndex) {
                    name = GS::UniString (stories[i].uName);
                    break;
                }
            }
            BMKillHandle (reinterpret_cast<GSHandle *> (&storyInfo.data));
            return name;
        }

        GSErrCode SelectOtherDbTarget (const OtherDbTarget &target) {
            API_DatabaseInfo dbInfo = target.dbInfo;
#ifdef ServerMainVers_2700
            GSErrCode err = ACAPI_Database_ChangeCurrentDatabase (&dbInfo);
#else
            GSErrCode err = ACAPI_Database (APIDb_ChangeCurrentDatabaseID, &dbInfo, nullptr);
#endif
            if (err != NoError) {
                msg_rep ("SyncShowSubelement", "APIDb_ChangeCurrentDatabaseID", err, APINULLGuid);
                return err;
            }

            if (target.hasStory) {
                API_StoryCmdType storyCmd = {};
                storyCmd.action = APIStory_GoTo;
                storyCmd.index = target.storyIndex;
                err = ACAPI_Environment (APIEnv_ChangeStorySettingsID, &storyCmd, nullptr);
                if (err != NoError) {
                    msg_rep ("SyncShowSubelement", "APIEnv_ChangeStorySettingsID", err, APINULLGuid);
                    return err;
                }
            }

            GS::Array<API_Neig> selNeigs;
            for (const auto &guid : target.guids)
                selNeigs.PushNew (guid);

#ifdef ServerMainVers_2700
            err = ACAPI_Selection_Select (selNeigs, true);
            if (err == NoError)
                ACAPI_View_ZoomToSelected ();
#else
            err = ACAPI_Element_Select (selNeigs, true);
            if (err == NoError)
                ACAPI_Automate (APIDo_ZoomToSelectedID);
#endif
            if (err != NoError)
                msg_rep ("SyncShowSubelement", "ACAPI_Selection_Select", err, APINULLGuid);
            return err;
        }

        class OtherDbDialog final : public DG::ModalDialog,
                                    public DG::PanelObserver,
                                    public DG::ButtonItemObserver,
                                    public DG::ListBoxObserver {
          public:
            enum DialogResourceID { CloseButtonId = 1, ShowButtonId = 2, ListBoxId = 3 };

            explicit OtherDbDialog (const GS::Array<OtherDbTarget> &targets)
                : DG::ModalDialog (ACAPI_GetOwnResModule (), ID_ADDON_OTHER_DB_DLG, ACAPI_GetOwnResModule ()),
                  closeButton (GetReference (), CloseButtonId), showButton (GetReference (), ShowButtonId),
                  listBox (GetReference (), ListBoxId), targets (targets) {
                const Int32 iseng = ID_ADDON_STRINGS + isEng ();
                DGSetDialogTitle (ID_ADDON_OTHER_DB_DLG,
                                  RSGetIndString (iseng, SubElementHalfId, ACAPI_GetOwnResModule ()));
                DGSetItemText (ID_ADDON_OTHER_DB_DLG,
                               CloseButtonId,
                               RSGetIndString (iseng, OtherDbCloseId, ACAPI_GetOwnResModule ()));
                DGSetItemText (ID_ADDON_OTHER_DB_DLG,
                               ShowButtonId,
                               RSGetIndString (iseng, OtherDbShowId, ACAPI_GetOwnResModule ()));

                closeButton.Attach (*this);
                showButton.Attach (*this);
                listBox.Attach (*this);
                Attach (*this);
                InitListBox ();
            }

            ~OtherDbDialog () {
                closeButton.Detach (*this);
                showButton.Detach (*this);
                listBox.Detach (*this);
                Detach (*this);
            }

            short GetSelectedTargetIndex () const { return selectedTargetIndex; }

            void ButtonClicked (const DG::ButtonClickEvent &ev) override {
                if (ev.GetSource () == &closeButton) {
                    PostCloseRequest (Cancel);
                } else if (ev.GetSource () == &showButton) {
                    selectedTargetIndex = listBox.GetSelectedItem () - 1;
                    if (selectedTargetIndex >= 0 && selectedTargetIndex < static_cast<short> (targets.GetSize ()))
                        PostCloseRequest (Accept);
                }
            }

            void ListBoxDoubleClicked (const DG::ListBoxDoubleClickEvent &) override {
                selectedTargetIndex = listBox.GetSelectedItem () - 1;
                if (selectedTargetIndex >= 0 && selectedTargetIndex < static_cast<short> (targets.GetSize ()))
                    PostCloseRequest (Accept);
            }

            void PanelResized (const DG::PanelResizeEvent &ev) override {
                const short dh = ev.GetHorizontalChange ();
                const short dv = ev.GetVerticalChange ();
                if (dh == 0 && dv == 0)
                    return;
                showButton.Move (dh, dv);
                closeButton.Move (0, dv);
                listBox.Resize (dh, dv);
                SetListBoxColumns ();
            }

          private:
            DG::Button closeButton;
            DG::Button showButton;
            DG::SingleSelListBox listBox;
            const GS::Array<OtherDbTarget> &targets;
            short selectedTargetIndex = -1;
            const short NameTab = 1;
            const short CountTab = 2;

            void SetListBoxColumns () {
                const short countWidth = 95;
                const short nameWidth = listBox.GetItemWidth () - countWidth;
                listBox.SetHeaderItemSize (NameTab, nameWidth);
                listBox.SetHeaderItemSize (CountTab, countWidth);
                listBox.SetTabFieldProperties (
                    NameTab, 0, nameWidth, DG::ListBox::Left, DG::ListBox::NoTruncate, false, true);
                listBox.SetTabFieldProperties (CountTab,
                                               nameWidth,
                                               nameWidth + countWidth,
                                               DG::ListBox::Center,
                                               DG::ListBox::NoTruncate,
                                               false,
                                               true);
            }

            void InitListBox () {
                const Int32 iseng = ID_ADDON_STRINGS + isEng ();
                listBox.SetTabFieldCount (2);
                listBox.SetHeaderItemCount (2);
                listBox.SetHeaderSynchronState (true);
                listBox.SetHeaderPushableButtons (false);
                listBox.SetHeaderItemText (NameTab,
                                           RSGetIndString (iseng, OtherDbDatabaseId, ACAPI_GetOwnResModule ()));
                listBox.SetHeaderItemText (CountTab,
                                           RSGetIndString (iseng, OtherDbElementsId, ACAPI_GetOwnResModule ()));
                listBox.SetHeaderItemSizeableFlag (NameTab, true);
                listBox.SetHeaderItemSizeableFlag (CountTab, false);
                SetListBoxColumns ();

                for (const auto &target : targets) {
                    listBox.AppendItem ();
                    listBox.SetTabItemText (DG::ListBox::BottomItem, NameTab, target.name);
                    listBox.SetTabItemText (DG::ListBox::BottomItem,
                                            CountTab,
                                            GS::UniString::Printf ("%u", (UInt32)target.guids.GetSize ()));
                }
                if (!targets.IsEmpty ())
                    listBox.SelectItem (1);
            }
        };

    } // namespace

    void AddOtherDbTarget (GS::Array<OtherDbTarget> &targets,
                           const API_DatabaseInfo &dbInfo,
                           bool hasStory,
                           short storyIndex,
                           const GS::UniString &name,
                           const API_Guid &guid) {
        AddOtherDbTargetImpl (targets, dbInfo, hasStory, storyIndex, name, guid);
    }

    GS::UniString GetOtherStoryName (short storyIndex) { return GetOtherStoryNameImpl (storyIndex); }

    void ShowOtherDbDialog (const GS::Array<OtherDbTarget> &targets) {
        if (targets.IsEmpty ())
            return;

        OtherDbDialog dialog (targets);
        if (!dialog.Invoke ())
            return;

        const short selectedIndex = dialog.GetSelectedTargetIndex ();
        if (selectedIndex < 0 || selectedIndex >= static_cast<short> (targets.GetSize ()))
            return;

        SelectOtherDbTarget (targets[selectedIndex]);
    }

} // namespace SyncDialogs
