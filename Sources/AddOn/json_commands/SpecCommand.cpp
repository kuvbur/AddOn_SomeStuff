#include <algorithm>
#include <chrono>
#include <vector>

#include "ACAPinc.h"

#include "json_commands/SpecCommand.hpp"

#include "dialogs/SyncSettings.hpp"
#include "spec/Spec.hpp"

#if defined(ServerMainVers_2500)

// -----------------------------------------------------------------------------
// Создаёт JSON-команду запуска построения спецификации.
// -----------------------------------------------------------------------------
SpecCommand::SpecCommand () : CommandBase (CommonSchema::NotUsed) {}

// -----------------------------------------------------------------------------
// Возвращает имя команды запуска построения спецификации.
// -----------------------------------------------------------------------------
GS::String SpecCommand::GetName () const { return "Spec"; }

// -----------------------------------------------------------------------------
// Возвращает схему параметров non-interactive запуска спецификации.
// -----------------------------------------------------------------------------
GS::Optional<GS::UniString> SpecCommand::GetInputParametersSchema () const {
    return R"({
        "type": "object",
        "properties": {
            "ruleNames": {
                "type": "array",
                "items": { "type": "string" },
                "minItems": 1
            },
            "includeParameters": {
                "description": "Include per-element property and GDL parameter values in the response",
                "type": "boolean"
            },
            "placementPoint": {
                "type": "object",
                "properties": {
                    "x": { "type": "number" },
                    "y": { "type": "number" }
                },
                "additionalProperties": false,
                "required": ["x", "y"]
            }
        },
        "additionalProperties": false,
        "required": ["placementPoint"]
    })";
}

// -----------------------------------------------------------------------------
// Указывает, что команда построения спецификации возвращает свободный ответ.
// -----------------------------------------------------------------------------
GS::Optional<GS::UniString> SpecCommand::GetResponseSchema () const { return GS::NoValue; }

// -----------------------------------------------------------------------------
// Список GUID в объект ответа, чтобы не тянуть вручную API в SpecCommand.cpp.
// Повторяющиеся поля добавляются через AddList: ObjectState::Add требует
// уникального имени поля, поэтому список нельзя наполнять вызовами Add.
// -----------------------------------------------------------------------------
static GS::ObjectState GuidListToObjectState (const GS::Array<API_Guid> &guids) {
    GS::ObjectState list;
    const auto addGuid = list.AddList<GS::ObjectState> ("element");
    for (UIndex i = 0; i < guids.GetSize (); ++i) {
        GS::ObjectState guid;
        guid.Add ("guid", APIGuidToString (guids[i]));
        addGuid (guid);
    }
    return list;
}

// -----------------------------------------------------------------------------
// Счётчики одного этапа в объекте ответа. Поля добавляются всегда, в том числе
// нулевые, чтобы форма ответа не зависела от данных: иначе сравнение прогонов
// отличало бы «этап не выполнялся» от «этап не выведен».
// -----------------------------------------------------------------------------
static GS::ObjectState StageCountersToObjectState (const Spec::SpecStageCounters &stage) {
    GS::ObjectState object;
    object.Add ("attempted", static_cast<GS::Int32> (stage.attempted));
    object.Add ("succeeded", static_cast<GS::Int32> (stage.succeeded));
    object.Add ("failed", static_cast<GS::Int32> (stage.failed));
    return object;
}

// -----------------------------------------------------------------------------
// Идентификатор этапа подготовки в объекте ответа. Выводится строкой, а не
// числом: коды GSErrCode у разных причин отказа одинаковы (все точки отказа
// подготовки возвращают APIERR_GENERAL), и по одному коду этап не различить.
// -----------------------------------------------------------------------------
static const char *PrepareStageName (Spec::SpecPrepareStage stage) {
    switch (stage) {
    case Spec::SpecPrepareStage::None:
        return "none";
    case Spec::SpecPrepareStage::RulesNotFound:
        return "rulesNotFound";
    case Spec::SpecPrepareStage::ReadParamsNotFound:
        return "readParamsNotFound";
    case Spec::SpecPrepareStage::WriteParamsNotFound:
        return "writeParamsNotFound";
    case Spec::SpecPrepareStage::PlaceParamsNotFound:
        return "placeParamsNotFound";
    case Spec::SpecPrepareStage::TooManyErrorElements:
        return "tooManyErrorElements";
    case Spec::SpecPrepareStage::EmptyElementsList:
        return "emptyElementsList";
    case Spec::SpecPrepareStage::NothingCreated:
        return "nothingCreated";
    case Spec::SpecPrepareStage::CanceledByUser:
        return "canceledByUser";
    case Spec::SpecPrepareStage::ExistingElementsLocked:
        return "existingElementsLocked";
    }
    return "unknown";
}

// -----------------------------------------------------------------------------
// Фактические результаты этапов запуска. Именно эти поля, а не
// elementsToCreate/Modify/Delete, показывают, чем закончился каждый этап:
// счётчик успеха подтверждает возврат вызова ACAPI, а не наличие элемента в
// модели. Признак hasUnconfirmedCreate выведен в отдельное поле, потому что
// состояние «счётчик утверждает успех, подтверждения нет» не выводится из
// счётчиков.
// -----------------------------------------------------------------------------
static void AddStageCounters (GS::ObjectState &response, const Spec::SpecRunResult &runResult) {
    response.Add ("create", StageCountersToObjectState (runResult.create));
    response.Add ("grouping", StageCountersToObjectState (runResult.grouping));
    response.Add ("gdl", StageCountersToObjectState (runResult.gdl));
    response.Add ("deleteOld", StageCountersToObjectState (runResult.deleteOld));
    response.Add ("hasPrimaryError", runResult.hasPrimaryError);
    response.Add ("hasRecoveryError", runResult.hasRecoveryError);
    response.Add ("hasUnconfirmedCreate", runResult.hasUnconfirmedCreate);
    // Отказ подготовки не отражается ни в одном счётчике выше - он происходит
    // до обращения к модели, поэтому все счётчики остаются нулевыми. Без этого
    // поля отчёт неотличим от «создавать было нечего».
    response.Add ("prepareFailureStage", PrepareStageName (runResult.prepareFailureStage));
}

// -----------------------------------------------------------------------------
// Сериализует значения в список ответа под именем fieldName.
// Имена выводятся отсортированными: словарь - хеш-таблица, порядок обхода
// нестабилен между запусками, а результат используется для сравнения прогонов.
// -----------------------------------------------------------------------------
static void AddDumpedValues (GS::ObjectState &target,
                             const char *fieldName,
                             const GS::HashTable<GS::UniString, GS::UniString> &values) {
    // GS::Array не имеет Sort ни в AC25, ни в AC29 - сортируем через std::vector,
    // как уже сделано для элементов в Roombook.cpp.
    std::vector<GS::UniString> names;
    for (GS::HashTable<GS::UniString, GS::UniString>::ConstPairIterator cIt = values.EnumeratePairs (); cIt != NULL;
         ++cIt) {
    #ifdef ServerMainVers_2800
        names.push_back (cIt->key);
    #else
        names.push_back (*cIt->key);
    #endif
    }
    std::sort (names.begin (), names.end (), [] (const GS::UniString &a, const GS::UniString &b) { return a < b; });
    // Поле добавляется даже при пустом словаре - тогда в JSON будет [] ,
    // а не исчезающее свойство, и форма ответа не зависит от данных.
    const auto addParameter = target.AddList<GS::ObjectState> (fieldName);
    for (const GS::UniString &name : names) {
        const GS::UniString *value = values.GetPtr (name);
        if (value == nullptr)
            continue;
        GS::ObjectState entry;
        entry.Add ("name", name);
        entry.Add ("value", *value);
        addParameter (entry);
    }
}

// -----------------------------------------------------------------------------
// Добавляет в ответ массив дампов элементов под именем fieldName.
// -----------------------------------------------------------------------------
static void AddElementDumps (GS::ObjectState &response,
                             const char *fieldName,
                             const GS::Array<Spec::SpecElementDump> &dumps) {
    GS::ObjectState list;
    const auto addElement = list.AddList<GS::ObjectState> ("element");
    for (const Spec::SpecElementDump &dump : dumps) {
        GS::ObjectState element;
        element.Add ("guid", APIGuidToString (dump.guid));
        element.Add ("favoriteName", dump.favorite_name);
        const auto addSource = element.AddList<GS::ObjectState> ("sourceElement");
        for (const API_Guid &sourceGuid : dump.sourceElements) {
            GS::ObjectState source;
            source.Add ("guid", APIGuidToString (sourceGuid));
            addSource (source);
        }
        AddDumpedValues (element, "property", dump.properties);
        AddDumpedValues (element, "gdlParameter", dump.gdlParameters);
        addElement (element);
    }
    response.Add (fieldName, list);
}

// -----------------------------------------------------------------------------
// Загружает настройки, запускает non-interactive построение спецификации и возвращает его результат.
// -----------------------------------------------------------------------------
GS::ObjectState SpecCommand::Execute (const GS::ObjectState &parameters,
                                      GS::ProcessControl & /*processControl*/) const {
    const GS::ObjectState *placementPointObject = parameters.Get ("placementPoint");
    if (placementPointObject == nullptr)
        return CreateErrorResponse (APIERR_BADPARS, "placementPoint is missing");

    Point2D placementPoint = {};
    if (!placementPointObject->Get ("x", placementPoint.x) || !placementPointObject->Get ("y", placementPoint.y))
        return CreateErrorResponse (APIERR_BADPARS, "placementPoint.x and placementPoint.y are required");

    GS::Array<GS::UniString> ruleNames;
    const bool hasRuleNames = parameters.Get ("ruleNames", ruleNames);
    // По умолчанию выключено: обычный запуск Spec не должен платить за сбор дампа.
    bool includeParameters = false;
    parameters.Get ("includeParameters", includeParameters);
    const auto start = std::chrono::steady_clock::now ();
    SyncSettings syncSettings;
    LoadSyncSettingsFromPreferences (syncSettings);
    Spec::SpecRunResult runResult;
    runResult.includeDetails = includeParameters;
    const GSErrCode err =
        Spec::SpecAll (syncSettings, hasRuleNames ? &ruleNames : nullptr, &placementPoint, &runResult);
    const auto finish = std::chrono::steady_clock::now ();

    const double elapsedSeconds = std::chrono::duration<double> (finish - start).count ();
    GS::ObjectState response;
    response.Add ("status", err == NoError ? "completed" : "failed");
    response.Add ("resultCode", static_cast<GS::Int32> (err));
    response.Add ("elementsToCreate", static_cast<GS::Int32> (runResult.elementsToCreate));
    response.Add ("elementsToModify", static_cast<GS::Int32> (runResult.elementsToModify));
    response.Add ("elementsToDelete", static_cast<GS::Int32> (runResult.elementsToDelete));
    response.Add ("elapsedSeconds", elapsedSeconds);
    response.Add ("includeParameters", includeParameters);
    AddStageCounters (response, runResult);
    if (includeParameters) {
        AddElementDumps (response, "created", runResult.created);
        AddElementDumps (response, "modified", runResult.modified);
        response.Add ("deleted", GuidListToObjectState (runResult.deleted));
    }
    return response;
}

#endif // defined (ServerMainVers_2500)