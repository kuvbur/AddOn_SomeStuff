//------------ kuvbur 2022 ------------
// SpecCompat — явная фиксация публичного контракта запуска Spec (R9.3).
//
// Зачем модуль: контракт non-interactive запуска (обязательность
// placementPoint, необязательность ruleNames, имена и набор полей ответа)
// размазан по SpecCommand::Execute, поэтому любая правка оркестратора
// (R9.1/R9.2) меняла бы его молча, и заметить это можно было бы только
// сравнением прогонов на модели. Здесь контракт собран в одном месте,
// им пользуется сам порт, и его можно проверить набором без модели.
//
// Чего модуль НЕ делает: не преобразует формат и не меняет значения. Он
// объявляет, что считается контрактом; сборка ответа остаётся в
// SpecCommand, но обязана брать имена полей отсюда. Отступление от
// контракта — намеренное изменение, а не accident: его фиксируют здесь.
#pragma once

#include "ACAPinc.h"

#include "spec/Spec.hpp"

namespace SpecCompat {

    // Имена полей объекта ответа. Формы ответа сравниваются между прогонами
    // (Tools/spec_baseline.py), поэтому переименование поля ломает стенд, а
    // добавление поля меняет форму сравнения: оба случая обязаны проходить
    // через эту структуру, а не редактироваться в SpecCommand.
    struct ResponseFieldNames {
        static const char *const status;
        static const char *const resultCode;
        static const char *const elementsToCreate;
        static const char *const elementsToModify;
        static const char *const elementsToDelete;
        static const char *const elapsedSeconds;
        static const char *const includeParameters;
        static const char *const hasPrimaryError;
        static const char *const hasRecoveryError;
        static const char *const hasUnconfirmedCreate;
        static const char *const prepareFailureStage;
        static const char *const created;
        static const char *const modified;
        static const char *const deleted;
    };

    // Имена полей вложенных объектов счётчиков этапа и списков элементов.
    struct NestedFieldNames {
        static const char *const attempted;
        static const char *const succeeded;
        static const char *const failed;
        static const char *const element;
        static const char *const guid;
    };

    // Имена полей объекта одного элемента в дампе.
    struct ElementFieldNames {
        static const char *const guid;
        static const char *const property;
        static const char *const sourceElement;
        static const char *const gdlParameter;
    };

    // Имена полей одного свойства в дампе элемента.
    struct PropertyFieldNames {
        static const char *const name;
        static const char *const value;
    };

    // Имена параметров входа. placementPoint обязателен: без него запуск
    // неинтерактивный и порт не может запросить точку размещения у пользователя.
    struct InputFieldNames {
        static const char *const placementPoint;
        static const char *const ruleNames;
        static const char *const includeParameters;
        static const char *const x;
        static const char *const y;
    };

    // Имена полей объекта ошибки разбора входа.
    struct ErrorFieldNames {
        static const char *const errorCode;
        static const char *const errorMessage;
    };

    // Этапы, счётчики которых входят в контракт ответа. Порядок объявления
    // повторяет порядок полей в ответе: его проверяет набор.
    extern const char *const StageCounterNames[4];

    // Проверки формы контракта, не требующие модели.
    //
    // Отдельная функция, а не только данные в структурах: нарушение контракта
    // — это прежде всего расхождение формы ответа, и проверять его должны
    // инварианты (годность и уникальность имён, обе ветви статуса), а не
    // сравнение с литералами из того же модуля: такое сравнение всегда
    // зелёное и не поймало бы ничего. Возвращает число нарушений.
    int VerifyResponseFields ();

    // Приводит код запуска к строке статуса ответа. Обратная к этой функции
    // связи нет намеренно: статус — единственное, что читатель ответа
    // обязан понимать без знания кодов GSErrCode.
    const char *StatusText (GSErrCode err);

} // namespace SpecCompat
