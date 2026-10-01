//------------ kuvbur 2022 ------------
#pragma once
#if !defined(SPECHELPERS_HPP)
    #define SPECHELPERS_HPP

    #include "Spec.hpp"

// Вспомогательные функции модуля спецификаций: шаг сетки размещения и
// построение дампа значений элемента. Не обращаются к модели, кроме чтения
// GDL-параметров из уже загруженного memo, поэтому вызываются и из
// PlaceElements, и из наборов тестов без модели.
namespace Spec {
    // Читает GDL-параметры элемента и определяет шаг сетки при размещении.
    // dx/dy заполняются только в ветках, которые признак размещения определён;
    // в остальных вызывающий должен иметь собственные значения (возвращается
    // false, а параметры остаются нетронутыми).
    bool GetSizePlaceElement (const API_Element &elementt, const API_ElementMemo &memot, double &dx, double &dy);

    // Строковое представление значения параметра для дампа. Формат совпадает
    // с ParamHelpers::ToString, но ветка неизвестного типа не прерывает
    // построение: дамп — диагностический вывод.
    GS::UniString ParamValueToDumpString (const ParamValue &pvalue);

    // Заполняет дамп элемента по словарю записываемых параметров.
    // GDL-параметры (rawname с префиксом {@gdl:}) в properties не попадают —
    // они пишутся в memo и в paramOut отсутствуют, их пишет FillDumpGDLParameter.
    void FillDumpFromParamDict (const ParamDictValue &param, SpecElementDump &dump);

    // Записывает в дамп фактическое значение GDL-параметра - то, что кладётся
    // в API_AddParType перед ACAPI_Element_Create, а не то, что было в ParamValue:
    // приведение к типу параметра может изменить значение.
    void FillDumpGDLParameter (const API_AddParType &actParam, SpecElementDump &dump);
} // namespace Spec

#endif