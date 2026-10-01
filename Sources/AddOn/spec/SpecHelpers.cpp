//------------ kuvbur 2022 ------------
#include "SpecHelpers.hpp"

// Шаг сетки размещения и дамп значений элемента. Обе функции обращаются только
// к уже загруженному memo, поэтому вызываются и из PlaceElements, и из наборов
// тестов без модели.

namespace Spec {

    // --------------------------------------------------------------------
    // Определение размеров элемента для размещения по сетке.
    // Назначение: читает GDL-параметры элемента и определяет шаг сетки при размещении.
    // Параметры:
    //   elementt - элемент (не используется, но нужен для совместимости с сигнатурой)
    //   memot - memo-структура элемента (содержит GDL-параметры)
    //   dx - [OUT] шаг по горизонтали
    //   dy - [OUT] шаг по вертикали
    // Алгоритм:
    //   1. Ищет параметры "somestuff_spec_hrow" (высота строки) и "somestuff_spec_bcol" (ширина колонки)
    //   2. Ищет параметр "show_type" (тип отображения)
    //   3. Если show_type=1 - размещение сверху вниз (dx=0, dy=somestuff_spec_hrow)
    //   4. Если show_type=2 или 3 - размещение по сетке (dx=somestuff_spec_bcol, dy=somestuff_spec_hrow)
    //   5. Если нет show_type - пытается использовать параметры "A" и "B" как размеры
    // Возвращает: true, если элементы размещаются сверху вниз (show_type=1 или есть somestuff_spec_hrow)
    // Примечание: при show_type = 0 (параметр есть, но значение не 1/2/3) и отсутствии
    // somestuff_spec_hrow размеры берутся из "A"/"B" - то есть ветка show_type
    // проверяется на 1 и на 2/3, но не выходит при другом значении.
    // --------------------------------------------------------------------
    bool GetSizePlaceElement (const API_Element &elementt, const API_ElementMemo &memot, double &dx, double &dy) {
        bool flag_find_dx = false;
        bool flag_find_dy = false;
        bool flag_find_type = false;
        double somestuff_spec_hrow = 0;
        double somestuff_spec_bcol = 0;
        Int32 show_type = 0;
        if (memot.params == nullptr)
            return false;
        const GSSize nParams = BMGetHandleSize ((GSHandle)memot.params) / sizeof (API_AddParType);
        for (GSIndex ii = 0; ii < nParams; ++ii) {
            API_AddParType &actParam = (*memot.params)[ii];
            GS::UniString name = GS::UniString (actParam.name);
            if (name.IsEqual ("somestuff_spec_hrow")) {
                somestuff_spec_hrow = actParam.value.real;
                flag_find_dx = true;
            }
            if (name.IsEqual ("somestuff_spec_bcol")) {
                somestuff_spec_bcol = actParam.value.real;
                flag_find_dy = true;
            }
            if (name.IsEqual ("show_type")) {
                show_type = DoubleToInt32 (actParam.value.real, "Spec", "параметр show_type");
                flag_find_type = true;
            }
            if (flag_find_dx && flag_find_dy && flag_find_type)
                break;
        }
        if (flag_find_type) {
            if (show_type == 1) {
                dx = 0;
                dy = somestuff_spec_hrow;
                return true;
            }
            if (show_type == 2 || show_type == 3) {
                dx = somestuff_spec_bcol;
                dy = somestuff_spec_hrow;
                return false;
            }
        }
        if (flag_find_dx) {
            dx = 0;
            dy = somestuff_spec_hrow;
            return true;
        }
        for (GSIndex ii = 0; ii < nParams; ++ii) {
            API_AddParType &actParam = (*memot.params)[ii];
            GS::UniString name = GS::UniString (actParam.name);
            if (name.IsEqual ("A") && !flag_find_dx) {
                dx = actParam.value.real;
                flag_find_dx = true;
            }
            if (name.IsEqual ("B") && !flag_find_dy) {
                dy = actParam.value.real;
                flag_find_dy = true;
            }
            if (flag_find_dx && flag_find_dy)
                return false;
        }
        return false;
    }

    // --------------------------------------------------------------------
    // Перевод значения параметра в строку для дампа.
    // Повторяет формат ParamHelpers::ToString, но без DBBREAK в ветке неизвестного
    // типа: дамп — диагностический вывод и не должен прерывать построение.
    // --------------------------------------------------------------------
    GS::UniString ParamValueToDumpString (const ParamValue &pvalue) {
        switch (pvalue.val.type) {
        case API_PropertyIntegerValueType:
            return FormatStringFunc::NumToString (pvalue.val.intValue, pvalue.val.formatstring);
        case API_PropertyRealValueType:
            return FormatStringFunc::NumToString (pvalue.val.doubleValue, pvalue.val.formatstring);
        case API_PropertyStringValueType:
            return pvalue.val.uniStringValue;
        case API_PropertyBooleanValueType:
            return GS::ValueToUniString (pvalue.val.boolValue);
        case API_PropertyGuidValueType:
            return APIGuidToString (pvalue.val.guidval);
        default:
            return EMPTYSTRING;
        }
    }

    // --------------------------------------------------------------------
    // Заполняет дамп элемента по словарю записываемых параметров.
    // GDL-параметры (rawname с префиксом {@gdl:}) в properties не попадают — они
    // пишутся в memo и в paramOut отсутствуют, их пишет FillDumpGDLParameter.
    // --------------------------------------------------------------------
    void FillDumpFromParamDict (const ParamDictValue &param, SpecElementDump &dump) {
        for (ParamDictValue::ConstPairIterator cIt = param.EnumeratePairs (); cIt != NULL; ++cIt) {
#ifdef ServerMainVers_2800
            const GS::UniString rawname = cIt->key;
            const ParamValue &pvalue = cIt->value;
#else
            const GS::UniString rawname = *cIt->key;
            const ParamValue &pvalue = *cIt->value;
#endif
            if (!pvalue.isValid || rawname.BeginsWith (GDLNAMEPREFIX))
                continue;
            dump.properties.Add (rawname, ParamValueToDumpString (pvalue));
        }
    }

    // --------------------------------------------------------------------
    // Записывает в дамп фактическое значение GDL-параметра - то, что кладётся
    // в API_AddParType перед ACAPI_Element_Create, а не то, что было в ParamValue:
    // приведение к типу параметра может изменить значение.
    // --------------------------------------------------------------------
    void FillDumpGDLParameter (const API_AddParType &actParam, SpecElementDump &dump) {
        GS::UniString rawname = GDLNAMEPREFIX + GS::UniString (actParam.name).ToLowerCase () + BRACEEND;
        GS::UniString value;
        switch (actParam.typeID) {
        case APIParT_CString:
        case APIParT_Title:
            value = GS::UniString (actParam.value.uStr);
            break;
        default:
            // Остальные типы хранятся в объединении как double, включая
            // целочисленные и логические (проверено по коду записи в memo).
            value = GS::UniString::Printf ("%g", actParam.value.real);
            break;
        }
        dump.gdlParameters.Add (rawname, value);
    }

} // namespace Spec