//------------ kuvbur 2026 ------------
#include "SpecExecutor.hpp"

// Создание элементов спецификации, группировка и GDL-скрипты — отдельная
// единица компиляции, потому что этап исполнения должен быть читаем без
// разбора расчёта: у него нет состояния между вызовами, всё приходит
// аргументами. Порядок вызовов ACAPI, границы транзакций и точки счётчиков
// заданы контрактом в SpecExecutor.hpp.

namespace Spec {

    GSErrCode PlaceElements (GS::Array<ElementDict> &elementstocreate,
                             ParamDictValue &paramToWrite,
                             ParamDictElement &paramOut,
                             Point2D &startpos,
                             SpecRunResult *runResult) {
        GSErrCode err = NoError;
        // Дамп собирается только по запросу (includeParameters в JSON-команде).
        const bool collectDetails = runResult != nullptr && runResult->includeDetails;
        API_Coord pos = {startpos.x, startpos.y};
        GS::Array<API_Elem_Head> elemsheader = {};
        double dx = 0;
        double dy = 0;
        API_StoryInfo storyInfo = {};
#ifdef ServerMainVers_2700
        err = ACAPI_ProjectSetting_GetStorySettings (&storyInfo);
#else
        err = ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo, nullptr);
#endif
        short act_st = 0;
        bool find_stor = false;
        if (err == NoError) {
            act_st = storyInfo.actStory;
            find_stor = true;
            BMKillHandle ((GSHandle *)&storyInfo.data);
        }
        {
            int n_elem = 0;
            API_Element element = {};
            GS::Array<API_Guid> group;
            for (auto &groups : elementstocreate) {
                if (group.IsEmpty ()) {
                    group.SetCapacity (groups.GetSize ());
                } else {
                    group.Clear ();
                }
                for (auto &cIt : groups) {
#ifdef ServerMainVers_2800
                    Element el = cIt.value;
#else
                    Element el = *cIt.value;
#endif
                    BNZeroMemory (&element, sizeof (API_Element));
                    API_ElementMemo memo = {};
                    err = GetElementForPlace (el.favorite_name, element, memo);
                    if (runResult != nullptr) {
                        // Попытка создания засчитывается ДО вызова: отказ
                        // GetElementForPlace — это отказ создания, а не
                        // «элемента не было в избранном».
                        runResult->create.attempted += 1;
                    }
                    if (err != NoError) {
                        if (runResult != nullptr) {
                            runResult->create.failed += 1;
                            runResult->hasPrimaryError = true;
                            runResult->hasUnconfirmedCreate = true;
                        }
                        ACAPI_DisposeElemMemoHdls (&memo);
                        msg_rep ("Spec", "ACAPI_Element_GetDefaults", err, APINULLGuid);
                        continue;
                    }
                    if (find_stor) {
                        element.header.floorInd = act_st;
                    }
                    // Снимает скрытие и блокировку слоя элемента перед созданием
                    // Проверяет floorInd & 0x8000 и layer.head.flags & 1, при необходимости вызывает
                    // ACAPI_Attribute_Set
                    UnhideUnlockElementLayer (element.header);
                    bool flag_find_row = GetSizePlaceElement (element, memo, dx, dy);
                    // Запись параметров
                    ParamDictValue param = {};
                    BuildRowParamToWrite (el, paramToWrite, param);
                    const GSSize nParams = (memo.params == nullptr)
                                               ? 0
                                               : BMGetHandleSize ((GSHandle)memo.params) / sizeof (API_AddParType);
                    // Дамп элемента: собирается до создания, чтобы в него попали
                    // значения ровно те, что уходят в memo (GDL-параметры после
                    // записи удаляются из param и в paramOut их уже нет).
                    // Всё под флагом: при выключенном дампе (обычный запуск Spec)
                    // не копируются даже favorite_name и sourceElements.
                    SpecElementDump dump = {};
                    if (collectDetails) {
                        dump.guid = element.header.guid;
                        dump.favorite_name = el.favorite_name;
                        dump.sourceElements = el.elements;
                        FillDumpFromParamDict (param, dump);
                    }
                    for (GSIndex ii = 0; ii < nParams; ++ii) {
                        API_AddParType &actParam = (*memo.params)[ii];
                        GS::UniString name = GS::UniString (actParam.name);
                        GS::UniString rawname;
                        bool flag_find = false;
                        if (actParam.typeMod == API_ParSimple) {
                            rawname = GDLNAMEPREFIX + name.ToLowerCase () + BRACEEND;
                            flag_find = param.ContainsKey (rawname);
                        }
                        if (actParam.typeMod == API_ParSimple && flag_find) {
                            ParamValueData paramfrom = param.Get (rawname).val;
                            switch (actParam.typeID) {
                            case APIParT_ColRGB:
                            case APIParT_Intens:
                            case APIParT_Length:
                            case APIParT_RealNum:
                            case APIParT_Angle:
                                actParam.value.real = paramfrom.doubleValue;
                                break;
                            case APIParT_Boolean:
                                actParam.value.real = paramfrom.doubleValue;
                                break;
                            case APIParT_Integer:
                            case APIParT_PenCol:
                            case APIParT_LineTyp:
                            case APIParT_Mater:
                            case APIParT_FillPat:
                            case APIParT_BuildingMaterial:
                            case APIParT_Profile:
                                actParam.value.real = paramfrom.intValue;
                                break;
                            case APIParT_CString:
                            case APIParT_Title:
                                GS::ucscpy (actParam.value.uStr,
                                            paramfrom.uniStringValue
                                                .ToUStr (0,
                                                         GS::Min (paramfrom.uniStringValue.GetLength (),
                                                                  (USize)API_UAddParStrLen))
                                                .Get ());
                                break;
                            default:
#ifdef ServerMainVers_2300
                            case APIParT_Dictionary:
#endif
                                break;
                            }
                            param.Delete (rawname);
                            if (collectDetails)
                                FillDumpGDLParameter (actParam, dump);
                        }
                    }
                    element.object.pos = pos;
                    err = ACAPI_Element_Create (&element, &memo);
                    if (runResult != nullptr) {
                        if (err == NoError)
                            runResult->create.succeeded += 1;
                        else {
                            runResult->create.failed += 1;
                            runResult->hasPrimaryError = true;
                            runResult->hasUnconfirmedCreate = true;
                        }
                    }
                    if (err == NoError) {
                        elemsheader.Push (element.header);
                        n_elem += 1;
                        if (flag_find_row) {
                            pos.y += dy;
                        } else {
                            if (n_elem % 10 == 0) {
                                pos.x = startpos.x;
                                pos.y += dy;
                            } else {
                                pos.x += dx;
                            }
                        }
                        // GUID элемента известен только после успешного создания
                        if (collectDetails) {
                            dump.guid = element.header.guid;
                            runResult->created.Push (dump);
                        }
                        paramOut.Add (element.header.guid, param);
                        group.Push (element.header.guid);
                    } else {
                        msg_rep ("Spec", "ACAPI_Element_Create", err, APINULLGuid);
                    }
                    ACAPI_DisposeElemMemoHdls (&memo);
                }
                pos.y += 2 * dy;
                if (group.GetSize () > 1) {
                    API_Guid groupGuid = APINULLGuid;
                    if (runResult != nullptr)
                        runResult->grouping.attempted += 1;
#ifdef ServerMainVers_2700
                    err = ACAPI_Grouping_CreateGroup (group, &groupGuid);
                    if (err != NoError)
                        err = ACAPI_Grouping_Tool (group, APITool_Group, nullptr);
#else
                    err = ACAPI_ElementGroup_Create (group, &groupGuid);
    #ifdef ServerMainVers_2300
                    if (err != NoError)
                        err = ACAPI_Element_Tool (group, APITool_Group, nullptr);
    #endif
#endif
                    if (err != NoError) {
                        if (runResult != nullptr) {
                            runResult->grouping.failed += 1;
                            // Группировка идёт ПОСЛЕ создания элементов, поэтому её
                            // отказ — ошибка восстановления, а не первичная.
                            runResult->hasRecoveryError = true;
                        }
                        msg_rep ("Spec", "ACAPI_ElementGroup_Create", err, APINULLGuid);
                    } else if (runResult != nullptr) {
                        runResult->grouping.succeeded += 1;
                    }
                }
            }
        }
        // GDL-скрипты выполняются ПОСЛЕ общей транзакции операции (не внутри неё),
        // поэтому отказ скрипта не откатывает уже созданный элемент.
        for (UInt32 i = 0; i < elemsheader.GetSize (); i++) {
            if (runResult != nullptr)
                runResult->gdl.attempted += 1;
#ifdef ServerMainVers_2700
            err = ACAPI_LibraryManagement_RunGDLParScript (&elemsheader[i], 0);
#else
            err = ACAPI_Goodies (APIAny_RunGDLParScriptID, &elemsheader[i], 0);
#endif
            if (runResult != nullptr) {
                if (err == NoError)
                    runResult->gdl.succeeded += 1;
                else {
                    runResult->gdl.failed += 1;
                    // Элемент уже создан, поэтому отказ скрипта - ошибка
                    // восстановления, а не первичная.
                    runResult->hasRecoveryError = true;
                }
            }
            if (err != NoError)
                msg_rep ("Spec", "APIAny_RunGDLParScriptID", err, APINULLGuid);
        }
        // Возвращает NoError, если первичный этап (создание) не дал отказа.
        // Раньше здесь всегда возвращалось NoError, и отказ создания был виден
        // только через пустой paramOut; при частичном отказе вызывающий его не
        // видел вовсе. Точное распределение отказов - в runResult->create.
        // Ошибки восстановления (группировка, GDL) в возврат НЕ входят: они
        // уже случились после того, как элемент создан, и откатом не являются.
        if (runResult != nullptr && runResult->hasPrimaryError)
            return APIERR_GENERAL;
        return NoError;
    }

    // Запись свойств и удаление устаревших строк.
    //
    // Транзакции внутри функции нет намеренно: вызывающий выполняет создание,
    // запись и удаление в одной общей транзакции, а вложенные
    // ACAPI_CallUndoableCommand запрещены (APIERR_NOTMINE). Функция обязана
    // вызываться изнутри открытой undo-транзакции.
    //
    // Результат удаления возвращается, а не пишется во внешний накопитель
    // ошибок - это позволяет отличить отказ удаления от успеха. Вызывающий
    // присваивает его своему накопителю.
    GSErrCode WriteSpecProperties (const GS::Array<API_Guid> &elementsDelete,
                                   ParamDictElement &paramOut,
                                   SpecRunResult *runResult) {
        GSErrCode err = NoError;
        if (!elementsDelete.IsEmpty ()) {
            if (runResult != nullptr) {
                runResult->deleteOld.attempted = elementsDelete.GetSize ();
            }
            err = ACAPI_Element_Delete (elementsDelete);
            if (runResult != nullptr) {
                // Удаление одним вызовом: подтверждено либо всё, либо ничего.
                // Проверка каждого удалённого элемента здесь невозможна и не
                // нужна - об этом читатель узнает по create/delete счётчикам.
                if (err == NoError)
                    runResult->deleteOld.succeeded = elementsDelete.GetSize ();
                else {
                    runResult->deleteOld.failed = elementsDelete.GetSize ();
                    // Удаление идёт ПОСЛЕ создания: элементы уже созданы,
                    // поэтому отказ удаления - ошибка восстановления.
                    runResult->hasRecoveryError = true;
                }
            }
            msg_rep ("Spec",
                     GS::UniString::Printf ("Removed %d obsolete spec elements", elementsDelete.GetSize ()),
                     err,
                     APINULLGuid);
        }
        ParamHelpers::ElementsWrite (paramOut);
        return err;
    }
} // namespace Spec
