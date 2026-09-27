//------------ kuvbur 2026 ------------
#include "api_headers/ResourceIds.hpp"

#include "TablesNavigator.hpp"

#include "CommonFunction.hpp"
#include "Constants.hpp"
#include "DGModule.hpp"
#include "INavigatorCallbackInterface.hpp"
#include "TableRenderer.hpp"

Int32 isEng ();

namespace TablesNavigator {
    namespace {

        constexpr bool kEnableNavigatorRegistration = true;
        const API_Guid kNavigatorRootGuid = APIGuidFromString ("4CDA3758-ECF8-4B03-BC1C-5C6F444E4F31");
        const GS::UniString kNavigatorRootDisplayId;
        const GS::UniString kScheduleDisplayIdPrefix = "SS";

        GS::UniString GetNavigatorRootDisplayName () {
            return RSGetIndString (ID_ADDON_STRINGS + isEng (), SomeStuffSchedulesNameID, ACAPI_GetOwnResModule ());
        }

        GS::UniString GetDefaultScheduleName () {
            return RSGetIndString (
                ID_ADDON_STRINGS + isEng (), SomeStuffScheduleDefaultNameID, ACAPI_GetOwnResModule ());
        }

        class ScheduleSettingsDialog final : public DG::ModalDialog,
                                             public DG::PanelObserver,
                                             public DG::ButtonItemObserver {
          public:
            ScheduleSettingsDialog (short dialogId, const GS::UniString &displayId, const GS::UniString &displayName);

            GS::UniString GetDisplayId () const;
            GS::UniString GetDisplayName () const;

          private:
            enum { OkButtonId = 1, CancelButtonId = 2, DisplayIdEditId = 4, DisplayNameEditId = 6 };

            DG::Button okButton;
            DG::Button cancelButton;
            DG::TextEdit displayIdEdit;
            DG::TextEdit displayNameEdit;

            void PanelCloseRequested (const DG::PanelCloseRequestEvent &ev, bool *accepted) override;
            void ButtonClicked (const DG::ButtonClickEvent &ev) override;
        };

        class NavigatorCallback final : public INavigatorCallbackInterface {
          public:
            GSErrCode OpenView (const API_Guid &viewPointID, bool newWindow) const override;
            GSErrCode OpenSettings (const API_Guid &viewPointID) const override;
            GSErrCode ExecuteMergePostProcess () const override;
            GSErrCode CreateIDFStore (const API_Guid &viewPointID,
                                      double scale,
                                      double &clipBoxWidth,
                                      double &clipBoxHeight,
                                      GSPtr &idfStore,
                                      API_Box &boundingBox,
                                      double &paddingX,
                                      double &paddingY,
                                      GS::Array<API_Guid> &elems) const override;
            GSErrCode GetElemsForDrawingCheck (const API_Guid &viewPointID, GS::Array<API_Guid> &elems) const override;
            GSErrCode NewItem (const API_Guid &viewPointID) const override;
            GSErrCode DeleteItem (const GS::Array<API_Guid> &viewPointIDList) const override;
            GSErrCode RenameItem (const API_Guid &viewPointID) const override;
            DG::Icon GetIcon (Int32 iconId,
                              IconTWMode iconTWMode,
                              IconLinkness linkness,
                              IconSize size,
                              IconContext context) const override;
        };

        NavigatorCallback &GetNavigatorCallback () {
            // INavigatorCallbackInterface должен жить всё время использования Archicad.
            // Статический объект даёт нужное время жизни без ручного владения и будет
            // безопасной точкой для будущего APINavigator_RegisterCallbackInterfaceID.
            static NavigatorCallback callback;
            return callback;
        }

        ScheduleSettingsDialog::ScheduleSettingsDialog (short dialogId,
                                                        const GS::UniString &displayId,
                                                        const GS::UniString &displayName)
            : DG::ModalDialog (ACAPI_GetOwnResModule (), dialogId, InvalidResModule),
              okButton (GetReference (), OkButtonId), cancelButton (GetReference (), CancelButtonId),
              displayIdEdit (GetReference (), DisplayIdEditId), displayNameEdit (GetReference (), DisplayNameEditId) {
            Attach (*this);
            okButton.Attach (*this);
            cancelButton.Attach (*this);

            displayIdEdit.SetText (displayId);
            displayNameEdit.SetText (displayName);
        }

        GS::UniString ScheduleSettingsDialog::GetDisplayId () const { return displayIdEdit.GetText (); }

        GS::UniString ScheduleSettingsDialog::GetDisplayName () const { return displayNameEdit.GetText (); }

        void ScheduleSettingsDialog::PanelCloseRequested (const DG::PanelCloseRequestEvent &ev, bool *accepted) {
            if (ev.IsAccepted () && displayNameEdit.GetText ().IsEmpty ())
                *accepted = false;
        }

        void ScheduleSettingsDialog::ButtonClicked (const DG::ButtonClickEvent &ev) {
            if (ev.GetSource () == &okButton) {
                PostCloseRequest (DG::ModalDialog::Accept);
            } else if (ev.GetSource () == &cancelButton) {
                PostCloseRequest (DG::ModalDialog::Cancel);
            }
        }

        GSErrCode GetViewPointData (const API_Guid &viewPointID, API_NavigatorAddOnViewPointData &vpData) {
            vpData.guid = viewPointID;
#ifdef ServerMainVers_2700
            return ACAPI_Navigator_GetNavigatorVPItem (reinterpret_cast<API_NavigatorAddOnViewPointData *> (&vpData));
#else
            return ACAPI_Navigator (APINavigator_GetNavigatorVPItemID, &vpData);
#endif
        }

        GSErrCode GetNewItemParentGuid (const API_Guid &selectedViewPointID, API_Guid &parentGuid) {
            API_NavigatorAddOnViewPointData selectedData;
            GSErrCode err = GetViewPointData (selectedViewPointID, selectedData);
            if (err != NoError)
                return err;

            parentGuid = (selectedData.itemType == API_NavigatorAddOnViewPointNodeID) ? selectedData.parentGuid
                                                                                      : selectedViewPointID;
            return NoError;
        }

        GSErrCode GetNavigatorChildren (const API_Guid &parentGuid, GS::Array<API_Guid> &children) {
            API_Guid mutableParentGuid = parentGuid;
#ifdef ServerMainVers_2700
            return ACAPI_Navigator_GetNavigatorVPItemChildren (reinterpret_cast<const API_Guid *> (&mutableParentGuid),
                                                               reinterpret_cast<GS::Array<API_Guid> *> (&children));
#else
            return ACAPI_Navigator (APINavigator_GetNavigatorVPItemChildrenID, &mutableParentGuid, &children);
#endif
        }

        GSErrCode BuildUniqueScheduleDefaults (const API_Guid &parentGuid,
                                               GS::UniString &displayId,
                                               GS::UniString &displayName) {
            GS::Array<API_Guid> children;
            GSErrCode err = GetNavigatorChildren (parentGuid, children);
            if (err != NoError)
                return err;

            const GS::UniString baseName = GetDefaultScheduleName ();

            for (UInt32 index = 1; index < 100000; ++index) {
                const GS::UniString candidateId = kScheduleDisplayIdPrefix + GS::UniString::Printf ("%03u", index);
                const GS::UniString candidateName =
                    (index == 1) ? baseName : baseName + " " + GS::UniString::Printf ("%u", index);

                bool found = false;
                for (const API_Guid &childGuid : children) {
                    API_NavigatorAddOnViewPointData childData;
                    err = GetViewPointData (childGuid, childData);
                    if (err != NoError)
                        return err;

                    if (childData.displayId == candidateId || childData.displayName == candidateName) {
                        found = true;
                        break;
                    }
                }

                if (!found) {
                    displayId = candidateId;
                    displayName = candidateName;
                    return NoError;
                }
            }

            return APIERR_GENERAL;
        }

        // ---------------------------------------------------------------------
        // Окно ведомости (MyDraw).
        // Механизм повторяет пример DevKit Navigator_Test: сброс текущей БД окна ->
        // создание 2D-элементов -> rebuild; отдельный валидатор определяет,
        // нужно ли перерисовать содержимое при активации окна.
        // ---------------------------------------------------------------------

        class ApiWindowGuard {
          public:
#ifdef ServerMainVers_2700
            ApiWindowGuard () { ACAPI_Window_ResetCurrentDatabase (); }
#else
            ApiWindowGuard () { ACAPI_Database (APIDb_ResetCurrentDatabaseID); }
#endif

            ~ApiWindowGuard () {
#ifdef ServerMainVers_2700
                if (ACAPI_View_SetZoom (nullptr, nullptr) == NoError)
#else
                if (ACAPI_Database (APIDb_SetZoomID) == NoError)
#endif
#ifdef ServerMainVers_2700
                    ACAPI_Database_RebuildCurrentDatabase ();
#else
                    ACAPI_Database (APIDb_RebuildCurrentDatabaseID);
#endif
            }
        };

        GSErrCode RenderScheduleIntoWindow () {
            // Layout считаем ДО сброса БД окна: шрифт и прочие дефолты текста берутся из
            // ACAPI_Element_GetDefaults текущей базы, а сразу после
            // APIDb_ResetCurrentDatabaseID это уже пустое окно — тогда кириллица
            // выводится чужими глифами (наблюдалось 2026-09-18).
            TableRenderer::TableRenderer renderer;
            renderer.SetPrototypeContent ();

            GSErrCode err = renderer.ComputeLayout ();
            if (err != NoError)
                return err;

            const API_Coord origin = {0.0, 0.0};
            {
                ApiWindowGuard windowGuard; // ResetCurrentDatabase + SetZoom/Rebuild
                err = renderer.Draw (origin);
            }

            return err;
        }

        void RegenerateScheduleContent (const API_Guid &viewPointID) {
            // Сброс текущей БД разрушителен, если текущее окно — не наше окно ведомости
            // (например план). Поэтому сначала проверяем текущее окно: тот же приём, что
            // в Navigator_Test (RegenerateContentIfAppropriate).
            API_WindowInfo windowInfo;
#ifdef ServerMainVers_2700
            const GSErrCode windowErr =
                ACAPI_Window_GetCurrentWindow (reinterpret_cast<API_WindowInfo *> (&windowInfo));
#else
            const GSErrCode windowErr = ACAPI_Database (APIDb_GetCurrentWindowID, &windowInfo, nullptr);
#endif
            if (windowErr != NoError || windowInfo.typeID != APIWind_MyDrawID ||
                windowInfo.databaseUnId.elemSetId != viewPointID) {
#ifdef TESTING
                DBprnt ("TablesNavigator::RegenerateScheduleContent", "current window is not the schedule window");
#endif
                return;
            }

            // Guard (сброс БД окна) находится внутри RenderScheduleIntoWindow:
            // layout обязан считаться ДО сброса, иначе дефолты текста берутся из пустого окна.
            const GSErrCode err = RenderScheduleIntoWindow ();
#ifdef TESTING
            DBprnt (static_cast<double> (err), "TablesNavigator schedule window render err");
#endif
        }

        GSErrCode GetWindowValidatorInfo (const API_Guid &userRefId, API_WindowValidatorInfo &validatorInfo) {
            validatorInfo.guid = userRefId;
            validatorInfo.elemList.Clear ();
            validatorInfo.checkSumList.Clear ();

            API_NavigatorAddOnViewPointData viewPointData;
            const GSErrCode err = GetViewPointData (userRefId, viewPointData);
            if (err != NoError)
                return err;

            // Отпечаток данных узла. Список зависимых элементов появится, когда строки
            // ведомости начнут вычисляться по модели; сейчас содержимое фиксировано.
            MD5::Generator md5Generator;
            if (viewPointData.data != nullptr)
                md5Generator.Update (*viewPointData.data, static_cast<unsigned int> (BMhGetSize (viewPointData.data)));

            MD5::FingerPrint fingerPrint;
            md5Generator.Finish (fingerPrint);
            validatorInfo.checkSumList.Push (fingerPrint);

            return NoError;
        }

        GSErrCode __ACENV_CALL ScheduleWindowHandlerProc (const API_Guid &userRefId, API_NotifyWindowEventID notifID) {
            switch (notifID) {
            case APINotifyWindow_Activate: {
                bool contentChanged = true;
                API_WindowValidatorInfo validatorInfo;
                if (GetWindowValidatorInfo (userRefId, validatorInfo) == NoError &&
#ifdef ServerMainVers_2700
                    ACAPI_Database_CheckWindowValidator (
                        reinterpret_cast<const API_WindowValidatorInfo *> (&validatorInfo),
                        reinterpret_cast<bool *> (&contentChanged)) == NoError &&
#else
                    ACAPI_Database (APIDb_CheckWindowValidatorID, &validatorInfo, &contentChanged) == NoError &&
#endif
                    contentChanged) {
#ifdef ServerMainVers_2700
                    ACAPI_Database_RebuildWindowValidator (
                        reinterpret_cast<const API_WindowValidatorInfo *> (&validatorInfo));
#else
                    ACAPI_Database (APIDb_RebuildWindowValidatorID, &validatorInfo);
#endif
                    RegenerateScheduleContent (userRefId);
                }
                break;
            }
            case APINotifyWindow_Rebuild:
                RegenerateScheduleContent (userRefId);
                break;
            case APINotifyWindow_Close: {
                API_Guid mutableGuid = userRefId;
#ifdef ServerMainVers_2700
                ACAPI_Window_DestroyWindowValidator (reinterpret_cast<const API_Guid *> (&mutableGuid));
#else
                ACAPI_Database (APIDb_DestroyWindowValidatorID, &mutableGuid);
#endif
                break;
            }
            default:
                break;
            }

            return NoError;
        }

        GSErrCode OpenScheduleWindow (const API_Guid &viewPointID, bool newWindow) {
            API_NavigatorAddOnViewPointData vpData;
            GSErrCode err = GetViewPointData (viewPointID, vpData);
            if (err != NoError)
                return err;

            const GS::UniString title = vpData.displayId + " " + vpData.displayName;

            GS::Array<API_Guid> ownWindows;
            API_WindowTypeID windowType = APIWind_MyDrawID;
#ifdef ServerMainVers_2700
            err = ACAPI_Window_GetOwnWindows (reinterpret_cast<API_WindowTypeID *> (&windowType),
                                              reinterpret_cast<GS::Array<API_Guid> *> (&ownWindows));
#else
            err = ACAPI_Database (APIDb_GetOwnWindowsID, &windowType, &ownWindows);
#endif
            if (err != NoError)
                return err;

            const bool windowExists = ownWindows.Contains (viewPointID);
            if (windowExists || (!newWindow && !ownWindows.IsEmpty ())) {
                API_WindowInfo windowInfo = {};
                windowInfo.typeID = APIWind_MyDrawID;
                windowInfo.databaseUnId.elemSetId = viewPointID;
                GS::UTruncate (title.ToUStr ().Get (), windowInfo.title, sizeof (windowInfo.title));

                if (!windowExists) {
                    // Окно MyDraw у аддона одно: переиспользуем открытое под другой узел
                    // (тот же приём, что в примере Navigator_Test).
                    API_Guid windowGuid = ownWindows[0];
#ifdef ServerMainVers_2700
                    err = ACAPI_Window_SetWindowId (
                        reinterpret_cast<const API_Guid *> (&windowGuid),
                        reinterpret_cast<const API_Guid *> (const_cast<API_Guid *> (&viewPointID)));
#else
                    err = ACAPI_Database (APIDb_SetWindowIdID, &windowGuid, const_cast<API_Guid *> (&viewPointID));
#endif
                    if (err != NoError)
                        return err;
                }

#ifdef ServerMainVers_2700
                return ACAPI_Window_ChangeWindow (reinterpret_cast<const API_WindowInfo *> (&windowInfo));
#else
                return ACAPI_Automate (APIDo_ChangeWindowID, &windowInfo);
#endif
            }

            API_NewWindowPars windowPars = {};
            windowPars.typeID = APIWind_MyDrawID;
            windowPars.userRefId = viewPointID;
            GS::UTruncate (title.ToUStr ().Get (), windowPars.wTitle, sizeof (windowPars.wTitle));

#ifdef ServerMainVers_2700
            err = ACAPI_Window_NewWindow (
                reinterpret_cast<API_NewWindowPars *> (&windowPars),
                reinterpret_cast<APICustomWindowHandlerProc *> (reinterpret_cast<void *> (ScheduleWindowHandlerProc)));
#else
            err = ACAPI_Database (APIDb_NewWindowID, &windowPars, reinterpret_cast<void *> (ScheduleWindowHandlerProc));
#endif
            if (err != NoError)
                return err;

            API_WindowValidatorInfo validatorInfo;
            if (GetWindowValidatorInfo (viewPointID, validatorInfo) == NoError)
#ifdef ServerMainVers_2700
                ACAPI_Database_BuildWindowValidator (
                    reinterpret_cast<const API_WindowValidatorInfo *> (&validatorInfo));
#else
                ACAPI_Database (APIDb_BuildWindowValidatorID, &validatorInfo);
#endif

            return NoError;
        }

        GSErrCode __ACENV_CALL
        MergeViewPointData (const GS::Array<API_NavigatorAddOnViewPointData> &sourceVPDataArray) {
            (void)sourceVPDataArray;

            // Реальная merge-политика должна разрешать коллизии имён/internalId и переназначение GUID.
            // Пока регистрация выключена, handler существует только как проверенная точка расширения.
            return NoError;
        }

        GSErrCode __ACENV_CALL
        SaveOldFormatViewPointData (API_FTypeID planFileType,
                                    const GS::Array<API_NavigatorAddOnViewPointData> &currentFormatVPDataArray,
                                    GS::Array<API_NavigatorAddOnViewPointData> &oldFormatVPDataArray) {
            (void)planFileType;
            oldFormatVPDataArray = currentFormatVPDataArray;
            return NoError;
        }

        GSErrCode __ACENV_CALL
        ConvertNewFormatViewPointData (API_FTypeID planFileType,
                                       const GS::Array<API_NavigatorAddOnViewPointData> &oldFormatVPDataArray,
                                       GS::Array<API_NavigatorAddOnViewPointData> &currentFormatVPDataArray) {
            (void)planFileType;
            currentFormatVPDataArray = oldFormatVPDataArray;
            return NoError;
        }

    } // namespace

    bool IsNavigatorRegistrationEnabled () { return kEnableNavigatorRegistration; }

    GSErrCode RegisterInterface () {
        if (!kEnableNavigatorRegistration)
            return NoError;

#ifdef ServerMainVers_2700
        return ACAPI_AddOnIntegration_RegisterNavigatorAddOnViewPointDataHandler ();
#else
        return ACAPI_Register_NavigatorAddOnViewPointDataHandler ();
#endif
    }

    GSErrCode Initialize () {
        if (!kEnableNavigatorRegistration)
            return NoError;

#ifdef ServerMainVers_2700
        GSErrCode err = ACAPI_AddOnIntegration_InstallNavigatorAddOnViewPointDataMergeHandler (MergeViewPointData);
#else
        GSErrCode err = ACAPI_Install_NavigatorAddOnViewPointDataMergeHandler (MergeViewPointData);
#endif
        if (err != NoError)
            return err;

#ifdef ServerMainVers_2700
        err =
            ACAPI_AddOnIntegration_InstallNavigatorAddOnViewPointDataSaveOldFormatHandler (SaveOldFormatViewPointData);
#else
        err = ACAPI_Install_NavigatorAddOnViewPointDataSaveOldFormatHandler (SaveOldFormatViewPointData);
#endif
        if (err != NoError)
            return err;

#ifdef ServerMainVers_2700
        err = ACAPI_AddOnIntegration_InstallNavigatorAddOnViewPointDataConvertNewFormatHandler (
            ConvertNewFormatViewPointData);
#else
        err = ACAPI_Install_NavigatorAddOnViewPointDataConvertNewFormatHandler (ConvertNewFormatViewPointData);
#endif
        if (err != NoError)
            return err;

#ifdef ServerMainVers_2700
        return ACAPI_Navigator_RegisterCallbackInterface (
            reinterpret_cast<INavigatorCallbackInterface *> (&GetNavigatorCallback ()));
#else
        return ACAPI_Navigator (APINavigator_RegisterCallbackInterfaceID, &GetNavigatorCallback ());
#endif
    }

    namespace {

        GSErrCode EnsureNavigatorRootImpl () {
            if (!kEnableNavigatorRegistration)
                return NoError;

            GS::Array<API_Guid> roots;
#ifdef ServerMainVers_2700
            GSErrCode err = ACAPI_Navigator_GetNavigatorVPRootGroups (reinterpret_cast<GS::Array<API_Guid> *> (&roots));
#else
            GSErrCode err = ACAPI_Navigator (APINavigator_GetNavigatorVPRootGroupsID, &roots);
#endif
            if (err != NoError)
                return err;

            for (const API_Guid &rootGuid : roots) {
                API_NavigatorAddOnViewPointData rootData;
                rootData.guid = rootGuid;
#ifdef ServerMainVers_2700
                err = ACAPI_Navigator_GetNavigatorVPItem (
                    reinterpret_cast<API_NavigatorAddOnViewPointData *> (&rootData));
#else
                err = ACAPI_Navigator (APINavigator_GetNavigatorVPItemID, &rootData);
#endif
                if (err != NoError)
                    return err;

                const GS::UniString rootDisplayName = GetNavigatorRootDisplayName ();
                if (rootData.guid == kNavigatorRootGuid || rootData.displayName == rootDisplayName) {
                    if (rootData.displayName != rootDisplayName || rootData.displayId != kNavigatorRootDisplayId) {
                        rootData.displayName = rootDisplayName;
                        rootData.displayId = kNavigatorRootDisplayId;
                        rootData.iconId = static_cast<Int32> (API_NavigatorAddOnViewPointRootID);
#ifdef ServerMainVers_2700
                        err = ACAPI_Navigator_ChangeNavigatorVPItem (
                            reinterpret_cast<API_NavigatorAddOnViewPointData *> (&rootData));
#else
                        err = ACAPI_Navigator (APINavigator_ChangeNavigatorVPItemID, &rootData);
#endif
#ifdef TESTING
                        if (err == NoError)
                            DBprnt ("TablesNavigator::EnsureNavigatorRoot", "updated schedules root");
#endif
                    } else {
#ifdef TESTING
                        DBprnt ("TablesNavigator::EnsureNavigatorRoot", "found schedules root");
#endif
                    }
                    return err;
                }
            }

            API_NavigatorAddOnViewPointData rootData;
            rootData.itemType = API_NavigatorAddOnViewPointRootID;
            rootData.iconId = static_cast<Int32> (API_NavigatorAddOnViewPointRootID);
            rootData.guid = kNavigatorRootGuid;
            rootData.displayId = kNavigatorRootDisplayId;
            rootData.displayName = GetNavigatorRootDisplayName ();

#ifdef ServerMainVers_2700
            err =
                ACAPI_Navigator_CreateNavigatorVPItem (reinterpret_cast<API_NavigatorAddOnViewPointData *> (&rootData));
#else
            err = ACAPI_Navigator (APINavigator_CreateNavigatorVPItemID, &rootData);
#endif
#ifdef TESTING
            if (err == NoError)
                DBprnt ("TablesNavigator::EnsureNavigatorRoot", "created schedules root");
#endif
            return err;
        }

    } // namespace

    GSErrCode EnsureNavigatorRoot () {
        const GSErrCode err = EnsureNavigatorRootImpl ();

#ifdef TESTING
        // Runtime-проверка TableRenderer (§7 ТЗ, issue #177): один раз за сессию.
        // Точка выбрана здесь, потому что измерение текста и создание drawing data
        // требуют открытого проекта (AllInputFinished наступает после открытия).
        static bool tableRendererSelfTestDone = false;
        if (!tableRendererSelfTestDone) {
            tableRendererSelfTestDone = true;
            TableRenderer::TableRenderer::RunSelfTest ();
        }
#endif

        return err;
    }

    GSErrCode NavigatorCallback::OpenView (const API_Guid &viewPointID, bool newWindow) const {
        // Порядок как в Navigator_Test (NavigatorCallbackInterface::OpenView):
        // сначала открыть/переключить окно MyDraw, затем сразу создать содержимое.
        // Обработчик окна (ScheduleWindowHandlerProc) получает Rebuild/Activate уже
        // позже — на него одного полагаться нельзя, окно останется пустым.
        const GSErrCode err = OpenScheduleWindow (viewPointID, newWindow);
        if (err != NoError)
            return err;

        // Содержимое рисуем только после успешного открытия окна: иначе сброс текущей
        // БД мог бы затронуть чужое окно (см. проверку внутри RegenerateScheduleContent).
        RegenerateScheduleContent (viewPointID);
        return NoError;
    }

    GSErrCode NavigatorCallback::OpenSettings (const API_Guid &viewPointID) const {
        API_NavigatorAddOnViewPointData vpData;
        GSErrCode err = GetViewPointData (viewPointID, vpData);
        if (err != NoError)
            return err;

        if (vpData.itemType == API_NavigatorAddOnViewPointRootID)
            return APIERR_REFUSEDCMD;

        ScheduleSettingsDialog settingsDialog (ID_ADDON_TABLE_EDIT_DLG, vpData.displayId, vpData.displayName);
        if (!settingsDialog.Invoke ())
            return NoError;

        vpData.displayId = settingsDialog.GetDisplayId ();
        vpData.displayName = settingsDialog.GetDisplayName ();
#ifdef ServerMainVers_2700
        return ACAPI_Navigator_ChangeNavigatorVPItem (reinterpret_cast<API_NavigatorAddOnViewPointData *> (&vpData));
#else
        return ACAPI_Navigator (APINavigator_ChangeNavigatorVPItemID, &vpData);
#endif
    }

    GSErrCode NavigatorCallback::ExecuteMergePostProcess () const {
        // Merge должен решать коллизии internalId/displayName и возможное
        // переназначение GUID. Возвращаем NoError только потому, что реальная
        // регистрация выключена; при включении здесь нужна полноценная политика.
        return NoError;
    }

    GSErrCode NavigatorCallback::CreateIDFStore (const API_Guid &viewPointID,
                                                 double scale,
                                                 double &clipBoxWidth,
                                                 double &clipBoxHeight,
                                                 GSPtr &idfStore,
                                                 API_Box &boundingBox,
                                                 double &paddingX,
                                                 double &paddingY,
                                                 GS::Array<API_Guid> &elems) const {
        (void)viewPointID; // содержимое-прототип пока не зависит от узла

        idfStore = nullptr;
        boundingBox = {};
        paddingX = 0.0;
        paddingY = 0.0;
        elems.Clear ();

        TableRenderer::TableRenderer renderer;
        renderer.SetPrototypeContent ();
        renderer.SetDrawingScale (scale);

        GSErrCode err = renderer.ComputeLayout ();
        if (err != NoError)
            return err;

        // Габарит отдаём до открытия сессии: он нужен вызывающей стороне, даже если
        // генерация 2D не удастся.
        clipBoxWidth = renderer.GetTotalWidth ();
        clipBoxHeight = renderer.GetTotalHeight ();

        // Сессию открывает и закрывает только этот метод (вложенность запрещена —
        // APIERR_NESTING). Ошибку генерации нельзя затирать ошибкой cleanup: сначала
        // закрываем и освобождаем store, затем возвращаем ошибку рисования.
        double drawingScale = scale;
#ifdef ServerMainVers_2700
        err = ACAPI_Drawing_StartDrawingData (reinterpret_cast<double *> (&drawingScale), nullptr);
#else
        err = ACAPI_Database (APIDb_StartDrawingDataID, &drawingScale);
#endif
        if (err != NoError)
            return err;

        const API_Coord origin = {0.0, 0.0};
        const GSErrCode drawErr = renderer.Draw (origin);

        GSPtr idfMemory = nullptr;
        API_Box tableBox = {};
#ifdef ServerMainVers_2700
        const GSErrCode stopErr = ACAPI_Drawing_StopDrawingData (reinterpret_cast<GSPtr *> (&idfMemory),
                                                                 reinterpret_cast<API_Box *> (&tableBox));
#else
        const GSErrCode stopErr = ACAPI_Database (APIDb_StopDrawingDataID, &idfMemory, &tableBox);
#endif

        if (stopErr != NoError) {
            if (idfMemory != nullptr)
                BMKillPtr (&idfMemory);
            return stopErr;
        }

        if (drawErr != NoError) {
            if (idfMemory != nullptr)
                BMKillPtr (&idfMemory);
            return drawErr;
        }

        idfStore = idfMemory;
        boundingBox = tableBox;
        return NoError;
    }

    GSErrCode NavigatorCallback::GetElemsForDrawingCheck (const API_Guid &viewPointID,
                                                          GS::Array<API_Guid> &elems) const {
        (void)viewPointID;

        // Здесь нельзя ничего рисовать и вообще менять БД: callback вызывается только
        // для проверки свежести. Ведомость генерируется, а не копирует элементы плана,
        // поэтому список зависимых элементов пока пуст (появится, когда строки начнут
        // вычисляться по модели).
        elems.Clear ();
        return NoError;
    }

    GSErrCode NavigatorCallback::NewItem (const API_Guid &viewPointID) const {
        API_Guid parentGuid = APINULLGuid;
        GSErrCode err = GetNewItemParentGuid (viewPointID, parentGuid);
        if (err != NoError)
            return err;

        GS::UniString displayId;
        GS::UniString displayName;
        err = BuildUniqueScheduleDefaults (parentGuid, displayId, displayName);
        if (err != NoError)
            return err;

        API_NavigatorAddOnViewPointData newData;
        newData.parentGuid = parentGuid;
        newData.itemType = API_NavigatorAddOnViewPointNodeID;
        newData.iconId = static_cast<Int32> (API_NavigatorAddOnViewPointNodeID);
        newData.displayId = displayId;
        newData.displayName = displayName;
        newData.viewSettingsFlags =
#ifdef ServerMainVers_2700
            // AC27 исправил опечатку в именах констант: Navgator → Navigator.
            API_NavigatorViewSettingsScaleAttributeID | API_NavigatorViewSettingsPenTableAttributeID |
            API_NavigatorViewSettingsLayerAttributeID;
#else
            API_NavgatorViewSettingsScaleAttributeID | API_NavgatorViewSettingsPenTableAttributeID |
            API_NavgatorViewSettingsLayerAttributeID;
#endif

// Создание leaf node в AC25 всегда выдаёт новый GUID; будущий код обязан
// сохранять возвращённый GUID, а не пытаться заранее назначить его сам.
#ifdef ServerMainVers_2700
        err = ACAPI_Navigator_CreateNavigatorVPItem (reinterpret_cast<API_NavigatorAddOnViewPointData *> (&newData));
#else
        err = ACAPI_Navigator (APINavigator_CreateNavigatorVPItemID, &newData);
#endif
        if (err != NoError)
            return err;

#ifdef TESTING
        DBprnt ("TablesNavigator::NewItem", "created schedule node");
#endif
        return OpenSettings (newData.guid);
    }

    GSErrCode NavigatorCallback::DeleteItem (const GS::Array<API_Guid> &viewPointIDList) const {
        for (const API_Guid &viewPointID : viewPointIDList) {
            API_NavigatorAddOnViewPointData vpData;
            GSErrCode err = GetViewPointData (viewPointID, vpData);
            if (err != NoError)
                return err;

            if (vpData.itemType == API_NavigatorAddOnViewPointRootID)
                continue;

#ifdef ServerMainVers_2700
            err = ACAPI_Navigator_DeleteNavigatorVPItem (reinterpret_cast<const API_Guid *> (&vpData.guid));
#else
            err = ACAPI_Navigator (APINavigator_DeleteNavigatorVPItemID, &vpData.guid);
#endif
            if (err != NoError)
                return err;
        }

        return NoError;
    }

    GSErrCode NavigatorCallback::RenameItem (const API_Guid &viewPointID) const { return OpenSettings (viewPointID); }

    DG::Icon NavigatorCallback::GetIcon (
        Int32 iconId, IconTWMode iconTWMode, IconLinkness linkness, IconSize size, IconContext context) const {
        (void)iconId;
        (void)iconTWMode;
        (void)linkness;
        (void)size;
        (void)context;

        // Иконки добавляются вместе с ресурсами Navigator. Пока отдаём пустую иконку,
        // чтобы каркас не требовал правок Sources/AddOnResources.
        return DG::Icon ();
    }

} // namespace TablesNavigator
