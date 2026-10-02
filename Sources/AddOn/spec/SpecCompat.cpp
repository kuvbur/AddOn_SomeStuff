//------------ kuvbur 2022 ------------
// Определения имён контракта и проверок его формы. См. SpecCompat.hpp.
#include "ACAPinc.h"

#include "spec/SpecCompat.hpp"

namespace SpecCompat {

    const char *const ResponseFieldNames::status = "status";
    const char *const ResponseFieldNames::resultCode = "resultCode";
    const char *const ResponseFieldNames::elementsToCreate = "elementsToCreate";
    const char *const ResponseFieldNames::elementsToModify = "elementsToModify";
    const char *const ResponseFieldNames::elementsToDelete = "elementsToDelete";
    const char *const ResponseFieldNames::elapsedSeconds = "elapsedSeconds";
    const char *const ResponseFieldNames::includeParameters = "includeParameters";
    const char *const ResponseFieldNames::hasPrimaryError = "hasPrimaryError";
    const char *const ResponseFieldNames::hasRecoveryError = "hasRecoveryError";
    const char *const ResponseFieldNames::hasUnconfirmedCreate = "hasUnconfirmedCreate";
    const char *const ResponseFieldNames::prepareFailureStage = "prepareFailureStage";
    const char *const ResponseFieldNames::created = "created";
    const char *const ResponseFieldNames::modified = "modified";
    const char *const ResponseFieldNames::deleted = "deleted";
    const char *const ResponseFieldNames::rules = "rules";
    const char *const ResponseFieldNames::messages = "messages";

    const char *const NestedFieldNames::attempted = "attempted";
    const char *const NestedFieldNames::succeeded = "succeeded";
    const char *const NestedFieldNames::failed = "failed";
    const char *const NestedFieldNames::element = "element";
    const char *const NestedFieldNames::guid = "guid";
    const char *const NestedFieldNames::rule = "rule";
    const char *const NestedFieldNames::message = "message";

    const char *const RuleFieldNames::name = "name";
    const char *const RuleFieldNames::created = "created";
    const char *const RuleFieldNames::modified = "modified";
    const char *const RuleFieldNames::deleted = "deleted";

    const char *const MessageFieldNames::ruleName = "ruleName";
    const char *const MessageFieldNames::text = "text";

    const char *const ElementFieldNames::guid = "guid";
    const char *const ElementFieldNames::property = "property";
    const char *const ElementFieldNames::sourceElement = "sourceElement";
    const char *const ElementFieldNames::gdlParameter = "gdlParameter";

    const char *const PropertyFieldNames::name = "name";
    const char *const PropertyFieldNames::value = "value";

    const char *const InputFieldNames::placementPoint = "placementPoint";
    const char *const InputFieldNames::ruleNames = "ruleNames";
    const char *const InputFieldNames::includeParameters = "includeParameters";
    const char *const InputFieldNames::x = "x";
    const char *const InputFieldNames::y = "y";

    const char *const ErrorFieldNames::errorCode = "errorCode";
    const char *const ErrorFieldNames::errorMessage = "errorMessage";

    const char *const StageCounterNames[4] = {"create", "grouping", "gdl", "deleteOld"};

    // ------------------------------------------------------------------------
    // Статус ответа. Единственное место, где GSErrCode переводится в строку:
    // иначе появление нового кода молча дало бы "completed".
    // ------------------------------------------------------------------------
    const char *StatusText (GSErrCode err) { return err == NoError ? "completed" : "failed"; }

    namespace {
        // Имя поля годно, если оно непустое и не содержит разделителей схемы
        // ответа. Имя с точкой или двоеточием ломает не форму ответа, а разбор
        // дампа на стенде: Tools/spec_baseline.py режет свойства по '/'.
        bool IsUsableName (const char *name) {
            if (name == nullptr)
                return false;
            const GS::UniString text (name);
            if (text.IsEmpty ())
                return false;
            return !text.Contains ("/") && !text.Contains (":");
        }

        // Группа имён пригодна, если все имена годны и попарно различны.
        // Совпадение внутри группы недопустимо: ObjectState::Add отверг бы
        // второе добавление, и поле молча пропало бы из ответа.
        int CountGroupFailures (const char *const *names, UIndex count) {
            int failures = 0;
            for (UIndex i = 0; i < count; ++i) {
                if (!IsUsableName (names[i])) {
                    ++failures;
                    continue;
                }
                const GS::UniString current (names[i]);
                for (UIndex j = i + 1; j < count; ++j) {
                    if (names[j] != nullptr && GS::UniString (names[j]) == current) {
                        ++failures;
                        break;
                    }
                }
            }
            return failures;
        }
    } // namespace

    // ------------------------------------------------------------------------
    // Форма ответа и обязательность входных параметров.
    //
    // Проверяются ИМЕНА как таковые, а не их совпадение с литералами из этого
    // же модуля: такое сравнение всегда зелёное и не поймало бы ничего.
    // Ловятся здесь четыре реальных дефекта формы:
    //   - пустое или nullptr-имя - поле потеряло бы форму ответа;
    //   - разделитель в имени - сломался бы разбор дампа на стенде;
    //   - совпадение имён внутри группы - второе Add отвергли бы, поле пропало;
    //   - имя этапа, совпадающее с полем верхнего уровня, - ObjectState::Add
    //     столкнул бы вложенный объект со строкой.
    //
    // Значения полей не проверяются: они зависят от модели, которой в наборе
    // нет, и проверяются прогонами на стенде.
    // ------------------------------------------------------------------------
    int VerifyResponseFields () {
        static const char *const responseNames[] = {ResponseFieldNames::status,
                                                    ResponseFieldNames::resultCode,
                                                    ResponseFieldNames::elementsToCreate,
                                                    ResponseFieldNames::elementsToModify,
                                                    ResponseFieldNames::elementsToDelete,
                                                    ResponseFieldNames::elapsedSeconds,
                                                    ResponseFieldNames::includeParameters,
                                                    ResponseFieldNames::hasPrimaryError,
                                                    ResponseFieldNames::hasRecoveryError,
                                                    ResponseFieldNames::hasUnconfirmedCreate,
                                                    ResponseFieldNames::prepareFailureStage,
                                                    ResponseFieldNames::created,
                                                    ResponseFieldNames::modified,
                                                    ResponseFieldNames::deleted,
                                                    ResponseFieldNames::rules,
                                                    ResponseFieldNames::messages};
        static const char *const inputNames[] = {InputFieldNames::placementPoint,
                                                 InputFieldNames::ruleNames,
                                                 InputFieldNames::includeParameters,
                                                 InputFieldNames::x,
                                                 InputFieldNames::y};
        static const char *const elementNames[] = {ElementFieldNames::guid,
                                                   ElementFieldNames::property,
                                                   ElementFieldNames::sourceElement,
                                                   ElementFieldNames::gdlParameter};
        static const char *const propertyNames[] = {PropertyFieldNames::name, PropertyFieldNames::value};
        static const char *const nestedNames[] = {NestedFieldNames::attempted,
                                                  NestedFieldNames::succeeded,
                                                  NestedFieldNames::failed,
                                                  NestedFieldNames::element,
                                                  NestedFieldNames::rule,
                                                  NestedFieldNames::message};

        int failures = 0;
        failures += CountGroupFailures (responseNames, 16);
        failures += CountGroupFailures (inputNames, 5);
        failures += CountGroupFailures (elementNames, 4);
        failures += CountGroupFailures (propertyNames, 2);
        failures += CountGroupFailures (nestedNames, 6);
        // Группы новых полей проверяются отдельно: иначе добавление полей в
        // responseNames расширило бы верхнюю группу, а имена вложенных объектов
        // остались бы молча непроверенными.
        static const char *const ruleNames[] = {
            RuleFieldNames::name, RuleFieldNames::created, RuleFieldNames::modified, RuleFieldNames::deleted};
        static const char *const messageNames[] = {MessageFieldNames::ruleName, MessageFieldNames::text};
        failures += CountGroupFailures (ruleNames, 4);
        failures += CountGroupFailures (messageNames, 2);

        // Имена счётчиков этапов - отдельная группа: они добавляются в ответ
        // как вложенные объекты и не должны совпадать между собой.
        failures += CountGroupFailures (StageCounterNames, 4);

        // Имя этапа не должно совпадать с полем верхнего уровня ответа.
        for (UIndex i = 0; i < 4; ++i) {
            if (StageCounterNames[i] == nullptr)
                continue;
            const GS::UniString stage (StageCounterNames[i]);
            for (UIndex j = 0; j < 16; ++j) {
                if (responseNames[j] != nullptr && GS::UniString (responseNames[j]) == stage) {
                    ++failures;
                    break;
                }
            }
        }

        // Статус - единственный перевод кода в строку; проверяются обе ветви,
        // потому что ошибка в любой из них сделала бы ответ неоднозначным.
        if (GS::UniString (StatusText (NoError)) != GS::UniString ("completed"))
            ++failures;
        if (GS::UniString (StatusText (APIERR_GENERAL)) == GS::UniString ("completed"))
            ++failures;
        if (GS::UniString (StatusText (APIERR_CANCEL)) == GS::UniString ("completed"))
            ++failures;

        // Ответ об ошибке разбора входа собирается тем же ObjectState, поэтому
        // имена его полей обязаны быть такими же годными.
        if (!IsUsableName (ErrorFieldNames::errorCode))
            ++failures;
        if (!IsUsableName (ErrorFieldNames::errorMessage))
            ++failures;

        return failures;
    }

} // namespace SpecCompat
