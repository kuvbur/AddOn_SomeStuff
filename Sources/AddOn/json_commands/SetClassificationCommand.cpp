// ---------------------------------------------------------------
// SetClassificationCommand - назначение классификации выделенным элементам
// ---------------------------------------------------------------
#include "json_commands/SetClassificationCommand.hpp"

#include "ClassificationFunction.hpp"
#include "CommonFunction.hpp"
#include "Propertycache.hpp"

GS::ObjectState SetClassificationCommand::Execute (const GS::ObjectState &parameters,
                                                   GS::ProcessControl & /*processControl*/) const {
    // Получаем classificationValue (полное имя классификации или GUID)
    GS::UniString classificationValue;
    if (!parameters.Get ("classificationValue", classificationValue) || classificationValue.IsEmpty ()) {
        return CreateErrorResponse (APIERR_BADPARS, "Missing or empty classificationValue parameter");
    }

    // Получаем GUID-ы выделенных элементов
    GS::Array<API_Guid> selectedElements = GetSelectedElements2 (false, true);

    if (selectedElements.IsEmpty ()) {
        return CreateErrorResponse (APIERR_BADPARS, "No elements selected");
    }

    // Убедимся, что классификации загружены в кэш
    if (!ClassificationFunc::ReadSystemDict ()) {
        return CreateErrorResponse (APIERR_GENERAL, "Failed to load classification systems");
    }

    auto &cache = PROPERTYCACHE ();

    // Ищем класс по полному имени во всех системах
    API_Guid targetSystemGuid = APINULLGuid;
    API_Guid targetItemGuid = APINULLGuid;
    bool found = false;

    for (const auto &sysPair : cache.systemdict) {
        const ClassificationFunc::ClassificationDict *classDict = sysPair.value;
        for (const auto &classPair : *classDict) {
            const ClassificationFunc::ClassificationValues *cv = classPair.value;
            GS::UniString fullName;
            ClassificationFunc::GetFullName (cv->item, *classDict, fullName);
            if (fullName == classificationValue) {
                targetSystemGuid = cv->system.guid;
                targetItemGuid = cv->item.guid;
                found = true;
                break;
            }
        }
        if (found)
            break;
    }

    if (!found) {
        // Попробуем найти по GUID, если передан в формате "systemGuid:itemGuid"
        GS::Array<GS::UniString> parts;
        classificationValue.Split (":", &parts);
        if (parts.GetSize () == 2) {
            targetSystemGuid = APIGuidFromString (parts[0].ToCStr ().Get ());
            targetItemGuid = APIGuidFromString (parts[1].ToCStr ().Get ());
            if (targetSystemGuid != APINULLGuid && targetItemGuid != APINULLGuid) {
                found = true;
            }
        }
    }

    if (!found) {
        return CreateErrorResponse (
            APIERR_BADPARS, GS::UniString ("Classification not found: ") + classificationValue.ToCStr ().Get ());
    }

    // Назначаем классификацию каждому выделенному элементу
    Int32 successCount = 0;
    Int32 errorCount = 0;

    for (const API_Guid &elemGuid : selectedElements) {
        // Сначала проверяем, есть ли уже эта классификация у элемента
        GS::Array<GS::Pair<API_Guid, API_Guid>> systemItemPairs;
        GSErrCode err = ACAPI_Element_GetClassificationItems (elemGuid, systemItemPairs);
        if (err != NoError) {
            // Если ошибка при чтении, пробуем просто добавить
        }

        bool alreadyHas = false;
        for (const auto &pair : systemItemPairs) {
            if (pair.first == targetSystemGuid && pair.second == targetItemGuid) {
                alreadyHas = true;
                break;
            }
        }

        if (alreadyHas) {
            successCount++;
            continue;
        }

        // Добавляем классификацию
        err = ACAPI_Element_AddClassificationItem (elemGuid, targetItemGuid);
        if (err == NoError) {
            successCount++;
        } else {
            errorCount++;
            msg_rep ("SetClassificationCommand", "ACAPI_Element_AddClassificationItem", err, elemGuid);
        }
    }

    GS::ObjectState response;
    response.Add ("success", errorCount == 0);
    response.Add ("successCount", successCount);
    response.Add ("errorCount", errorCount);
    response.Add ("status", errorCount == 0 ? "ok" : "partial");

    return response;
}