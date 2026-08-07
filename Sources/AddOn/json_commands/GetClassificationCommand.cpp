// ---------------------------------------------------------------
// GetClassificationCommand - получение дерева классификации для выделения
// ---------------------------------------------------------------
#include "json_commands/GetClassificationCommand.hpp"

#include "ClassificationFunction.hpp"
#include "CommonFunction.hpp"
#include "Propertycache.hpp"

GS::ObjectState GetClassificationCommand::Execute (const GS::ObjectState &parameters,
                                                   GS::ProcessControl & /*processControl*/) const {
    // Получаем GUID-ы выделенных элементов
    GS::Array<API_Guid> selectedElements = GetSelectedElements2 (false, true);

    GS::ObjectState response;

    if (selectedElements.IsEmpty ()) {
        response.Add ("common", true);
        response.Add ("commonPath", GS::Array<GS::UniString> ());
        response.Add ("differing", GS::Array<GS::ObjectState> ());
        response.Add ("options", GS::Array<GS::UniString> ());
        response.Add ("status", "ok");
        return response;
    }

    // Убедимся, что классификации загружены в кэш
    if (!ClassificationFunc::ReadSystemDict ()) {
        return CreateErrorResponse (APIERR_GENERAL, "Failed to load classification systems");
    }

    auto &cache = PROPERTYCACHE ();

    // Собираем классификацию для каждого выделенного элемента
    GS::HashTable<GS::Pair<API_Guid, API_Guid>, Int32> classificationCounts; // systemGuid+itemGuid -> count
    GS::HashTable<GS::Pair<API_Guid, API_Guid>, GS::UniString>
        classificationDisplayNames; // systemGuid+itemGuid -> display name

    for (const API_Guid &elemGuid : selectedElements) {
        GS::Array<GS::Pair<API_Guid, API_Guid>> systemItemPairs;
        GSErrCode err = ACAPI_Element_GetClassificationItems (elemGuid, systemItemPairs);
        if (err != NoError) {
            continue; // Элемент без классификации или ошибка
        }

        for (const auto &pair : systemItemPairs) {
            const GS::Pair<API_Guid, API_Guid> key = pair;
            const Int32 *currentCountPtr = classificationCounts.GetPtr (key);
            Int32 currentCount = currentCountPtr ? *currentCountPtr : 0;
            classificationCounts.Put (key, currentCount + 1);

            // Получаем отображаемое имя для этого класса
            if (!classificationDisplayNames.ContainsKey (key)) {
                auto *systemDict = cache.reversesystemdict.GetPtr (pair.first);
                if (systemDict != nullptr) {
                    auto *classNamePtr = systemDict->GetPtr (pair.second);
                    if (classNamePtr != nullptr) {
                        GS::UniString displayName;
                        auto *dictPtr = cache.systemdict.GetPtr (*systemDict->GetPtr (APINULLGuid));
                        if (dictPtr != nullptr) {
                            ClassificationFunc::GetFullName (dictPtr->Get (*classNamePtr).item, *dictPtr, displayName);
                            classificationDisplayNames.Put (key, displayName);
                        }
                    }
                }
            }
        }
    }

    // Определяем общий путь классификации (все элементы имеют одинаковые классы)
    bool isCommon = true;
    GS::Array<GS::UniString> commonPath;
    GS::Array<GS::ObjectState> differing;

    if (classificationCounts.IsEmpty ()) {
        // Ни у одного элемента нет классификации
        isCommon = true;
        commonPath = GS::Array<GS::UniString> ();
    } else {
        // Проверяем, есть ли классы, которые есть у всех элементов
        GS::Array<GS::Pair<API_Guid, API_Guid>> allKeys;
        classificationCounts.EnumerateKeys ([&allKeys] (const GS::Pair<API_Guid, API_Guid> &key) -> bool {
            allKeys.Push (key);
            return true;
        });

        for (const auto &key : allKeys) {
            Int32 count = classificationCounts.Get (key);
            if (count == selectedElements.GetSize ()) {
                // Этот класс есть у всех элементов
                GS::UniString displayName;
                if (classificationDisplayNames.GetPtr (key)) {
                    displayName = *classificationDisplayNames.GetPtr (key);
                }
                commonPath.Push (displayName);
            } else {
                // Класс не у всех элементов
                isCommon = false;
                GS::ObjectState diffObj;
                GS::UniString displayName;
                if (classificationDisplayNames.GetPtr (key)) {
                    displayName = *classificationDisplayNames.GetPtr (key);
                }
                diffObj.Add ("classification", displayName);
                diffObj.Add ("count", count);
                diffObj.Add ("total", selectedElements.GetSize ());
                differing.Push (diffObj);
            }
        }
    }

    // Собираем список всех доступных классификаций (опции для выбора)
    GS::Array<GS::UniString> options;
    auto &systemdict = cache.systemdict;
    for (const auto &sysPair : systemdict) {
        const ClassificationFunc::ClassificationDict *classDict = sysPair.value;
        for (const auto &classPair : *classDict) {
            const ClassificationFunc::ClassificationValues *cv = classPair.value;
            GS::UniString fullName;
            ClassificationFunc::GetFullName (cv->item, *classDict, fullName);
            options.Push (fullName);
        }
    }

    response.Add ("common", isCommon);
    response.Add ("commonPath", commonPath);
    response.Add ("differing", differing);
    response.Add ("options", options);
    response.Add ("status", "ok");

    return response;
}