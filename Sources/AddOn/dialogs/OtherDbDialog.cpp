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

        // Подсвечивает элементы цветом, не меняя выделение. Версионные обёртки
        // те же, что в Spec.cpp / BrowserPalette.cpp: с AC26 появились функции
        // без возвращаемого кода, с AC27 — под ACAPI_UserInput_*.
        void HighlightElements (const GS::Array<API_Guid> &guids) {
            GS::HashTable<API_Guid, API_RGBAColor> hlElems;
            const API_RGBAColor hlColor = {1.0, 0.65, 0.0, 1.0};
            for (const API_Guid &guid : guids)
                hlElems.Put (guid, hlColor);
#ifdef ServerMainVers_2700
            ACAPI_UserInput_ClearElementHighlight ();
            ACAPI_UserInput_SetElementHighlight (hlElems);
#else
    #ifdef ServerMainVers_2600
            ACAPI_Interface_ClearElementHighlight ();
            ACAPI_Interface_SetElementHighlight (hlElems);
    #else
            // Вызов без par1 снимает предыдущую подсветку
            ACAPI_Interface (APIIo_HighlightElementsID);
            ACAPI_Interface (APIIo_HighlightElementsID, &hlElems);
    #endif
#endif
        }

        // Переход в 3D-окно и показ элементов сразу из ВСЕХ баз/этажей списка.
        // Выбор строки на результат не влияет: кнопка показывает всю совокупность.
        //
        // Порядок вызовов — экспериментальный (#246): сначала переход в 3D
        // (APIDo_ShowAllIn3D, чтобы в кадр попала модель целиком — иначе 3D-окно
        // может открыться «пустым» и зум по элементам другой базы не найдёт цели),
        // затем выделение уже в 3D-окне, затем подсветка цветом и зум к элементам.
        // Раньше выделение шло ПЕРЕД переходом — работало как «выделить + подсветить
        // в плане», но элементы чужой базы в 3D не выделялись; ACAPI_Selection_Select
        // документирует APIERR_BADDATABASE, однако это не доказывает запрет на
        // выделение после перехода, поэтому порядок проверяется на живом ArchiCAD.
        // Подсветка нужна в любом случае: она, в отличие от выделения, работает
        // и для элементов другой базы (контракт — «in the 2D … and 3D window»).
        GSErrCode ShowAllOtherDbTargetsIn3D (const GS::Array<OtherDbTarget> &targets) {
            GS::Array<API_Guid> guids;
            for (const auto &target : targets) {
                for (const auto &guid : target.guids)
                    guids.PushNew (guid);
            }
            if (guids.IsEmpty ())
                return NoError;

            // Переход в 3D с показом всей модели: элементы из разных баз не могут
            // быть в кадре одновременно, пока в 3D-окне не включён весь проект.
            GSErrCode err = NoError;
#ifdef ServerMainVers_2700
            err = ACAPI_View_ShowAllIn3D ();
#else
            err = ACAPI_Automate (APIDo_ShowAllIn3DID);
#endif
            if (err != NoError)
                msg_rep ("SyncShowSubelement", "ShowAllIn3D", err, APINULLGuid);

            // Выделение уже в 3D-окне — не блокирует дальнейшие шаги: элементы
            // из чужой базы могут не выделиться, и это не повод прерывать показ.
            GS::Array<API_Neig> selNeigs;
            for (const auto &guid : guids)
                selNeigs.PushNew (guid);
            GSErrCode selErr = NoError;
#ifdef ServerMainVers_2700
            selErr = ACAPI_Selection_Select (selNeigs, true);
#else
            selErr = ACAPI_Element_Select (selNeigs, true);
#endif
            if (selErr != NoError)
                msg_rep ("SyncShowSubelement", "Selection in 3D", selErr, APINULLGuid);

            // Подсветка цветом — работает по любой базе, в том числе после перехода.
            HighlightElements (guids);

            // Камера — к самим элементам (контракт: «works both in the 2D and 3D
            // window»). Если элементы недоступны для 3D-окна, зум вернёт отказ и
            // кадр останется на том, что показал ShowAllIn3D.
#ifdef ServerMainVers_2700
            err = ACAPI_View_ZoomToElements (&guids);
#else
            err = ACAPI_Automate (APIDo_ZoomToElementsID, &guids);
#endif
            if (err != NoError)
                msg_rep ("SyncShowSubelement", "ZoomToElements in 3D", err, APINULLGuid);

#ifdef ServerMainVers_2700
            ACAPI_View_Redraw ();
#else
            ACAPI_Automate (APIDo_RedrawID);
#endif
            return err;
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
#ifdef ServerMainVers_2700
                err = ACAPI_ProjectSetting_ChangeStorySettings (reinterpret_cast<API_StoryCmdType *> (&storyCmd));
#else
                err = ACAPI_Environment (APIEnv_ChangeStorySettingsID, &storyCmd, nullptr);
#endif
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
            enum DialogResourceID { CloseButtonId = 1, ShowButtonId = 2, ListBoxId = 3, Show3DButtonId = 4 };

            enum ResultID { ShowInDatabaseResult = 1, ShowIn3DResult = 2 };

            explicit OtherDbDialog (const GS::Array<OtherDbTarget> &targets)
                : DG::ModalDialog (ACAPI_GetOwnResModule (), ID_ADDON_OTHER_DB_DLG, ACAPI_GetOwnResModule ()),
                  closeButton (GetReference (), CloseButtonId), showButton (GetReference (), ShowButtonId),
                  show3DButton (GetReference (), Show3DButtonId), listBox (GetReference (), ListBoxId),
                  targets (targets) {
                const Int32 iseng = ID_ADDON_STRINGS + isEng ();
                DGSetDialogTitle (ID_ADDON_OTHER_DB_DLG,
                                  RSGetIndString (iseng, SubElementHalfId, ACAPI_GetOwnResModule ()));
                DGSetItemText (ID_ADDON_OTHER_DB_DLG,
                               CloseButtonId,
                               RSGetIndString (iseng, OtherDbCloseId, ACAPI_GetOwnResModule ()));
                DGSetItemText (ID_ADDON_OTHER_DB_DLG,
                               ShowButtonId,
                               RSGetIndString (iseng, OtherDbShowId, ACAPI_GetOwnResModule ()));
                DGSetItemText (ID_ADDON_OTHER_DB_DLG,
                               Show3DButtonId,
                               RSGetIndString (iseng, OtherDbShow3DId, ACAPI_GetOwnResModule ()));

                closeButton.Attach (*this);
                showButton.Attach (*this);
                listBox.Attach (*this);
                show3DButton.Attach (*this);
                Attach (*this);
                InitListBox ();
            }

            ~OtherDbDialog () {
                closeButton.Detach (*this);
                showButton.Detach (*this);
                listBox.Detach (*this);
                show3DButton.Detach (*this);
                Detach (*this);
            }

            short GetSelectedTargetIndex () const { return selectedTargetIndex; }

            ResultID GetResult () const { return result; }

            void ButtonClicked (const DG::ButtonClickEvent &ev) override {
                if (ev.GetSource () == &closeButton) {
                    PostCloseRequest (Cancel);
                } else if (ev.GetSource () == &showButton) {
                    if (AcceptSelection ())
                        PostCloseRequest (Accept);
                } else if (ev.GetSource () == &show3DButton) {
                    if (AcceptSelection ()) {
                        result = ShowIn3DResult;
                        PostCloseRequest (Accept);
                    }
                }
            }

            void ListBoxDoubleClicked (const DG::ListBoxDoubleClickEvent &) override {
                if (AcceptSelection ())
                    PostCloseRequest (Accept);
            }

            void PanelResized (const DG::PanelResizeEvent &ev) override {
                const short dh = ev.GetHorizontalChange ();
                const short dv = ev.GetVerticalChange ();
                if (dh == 0 && dv == 0)
                    return;
                showButton.Move (dh, dv);
                show3DButton.Move (dh, dv);
                closeButton.Move (0, dv);
                listBox.Resize (dh, dv);
                SetListBoxColumns ();
            }

          private:
            DG::Button closeButton;
            DG::Button showButton;
            DG::Button show3DButton;
            DG::SingleSelListBox listBox;
            const GS::Array<OtherDbTarget> &targets;
            short selectedTargetIndex = -1;
            ResultID result = ShowInDatabaseResult;
            const short NameTab = 1;
            const short CountTab = 2;

            // Читает выбранную строку списка; false, если строка не выбрана.
            bool AcceptSelection () {
                selectedTargetIndex = listBox.GetSelectedItem () - 1;
                return selectedTargetIndex >= 0 && selectedTargetIndex < static_cast<short> (targets.GetSize ());
            }

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

        // Кнопка «Показать в 3Д» показывает все цели сразу, поэтому выбранная
        // строка ей не нужна — проверяем её только для перехода в базу/этаж.
        if (dialog.GetResult () == OtherDbDialog::ShowIn3DResult) {
            ShowAllOtherDbTargetsIn3D (targets);
            return;
        }

        const short selectedIndex = dialog.GetSelectedTargetIndex ();
        if (selectedIndex < 0 || selectedIndex >= static_cast<short> (targets.GetSize ()))
            return;

        SelectOtherDbTarget (targets[selectedIndex]);
    }

} // namespace SyncDialogs
