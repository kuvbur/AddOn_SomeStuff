//------------ kuvbur 2022 ------------
#pragma once
#ifdef TESTING
    #if !defined(TEST_HPP)
        #define TEST_HPP
        #ifdef AC_25
            #include "api_headers/APICommon25.h"
        #endif // AC_25
        #ifdef AC_26
            #include "api_headers/APICommon26.h"
        #endif // AC_26
        #ifdef AC_27
            #include "api_headers/APICommon27.h"
        #endif // AC_26

// Вспомогательные функции для локального тестирования и отладки add-on.
namespace TestFunc {
    // Запускает набор локальных проверок основных helpers.
    void Test ();

    // Проверяет вычисление длины текстовой строки в нестандартных случаях.
    void TestGetTextLineLength (GS::UniString &var);

    // Проверяет арифметические и логические операции внутренних функций.
    void TestCalc ();

    // Проверяет работу формульного парсинга и вычисления.
    void TestFormula ();

    // Проверяет форматирование строк по правилам add-on.
    void TestFormatString ();

    // Проверяет форматирование строк на основе формул.
    void TestFormatStringFormula ();

    // Проверяет преобразование значений в ParamValue.
    void TestConvertToParamValue ();

    // Проверяет преобразование атрибутов в ParamValue.
    void TestConvertAttributeToParamValue ();

    // Проверяет преобразование свойств в ParamValue.
    void TestConvertPropertyToParamValue ();

    // Проверяет преобразование определений свойств в ParamValue.
    void TestConvertPropertyDefinitionToParamValue ();

    // Проверяет определение источника параметра по его raw-name.
    void TestSetParamValueSourseByName ();

    // Проверяет извлечение raw-name из описания свойства.
    void TestSetrawNameFromProperty ();

    // Проверяет правила игнорирования отдельных значений.
    void TestCheckIgnoreVal ();

    // Проверяет чтение свойств элемента.
    void TestReadProperty ();

    // Проверяет добавление новых свойств в словарь.
    void TestAddProperty ();

    // Проверяет преобразование вспомогательных структур в строку.
    void TestPropertyHelpersToString ();

    // Проверяет функцию Name2Rawname - преобразование имени в rawname.
    void TestName2Rawname ();

    // Проверяет функцию Name2Rawname с уже обёрнутыми скобками (временное решение до исправления бага).
    void TestName2RawnameWithBrackets ();

    // Проверяет функцию SyncString - парсинг строки правила синхронизации.
    void TestSyncString ();

    // Проверяет реальные правила синхронизации из BuildingInformation.xml.
    void TestSyncStringRealRules ();

    // Проверяет константы префиксов для парсинга.
    void TestParsePrefixes ();

    // Проверяет парсинг описания свойства с командами Sync, Renum, Sum, Spec.
    void TestParsePropertyDescription ();

    // Проверяет независимый вызов ParseSyncString (Этап 2 TDD).
    void TestParseSyncStringIndependent ();

    // Выводит все встроенные свойства в отладочный журнал.
    void DumpAllBuiltInProperties ();

    // Сбрасывает свойства синхронизации для массива элементов.
    void ResetSyncPropertyArray (GS::Array<API_Guid> guidArray);

    // Сбрасывает свойства синхронизации для одного элемента.
    void ResetSyncPropertyOne (const API_Guid &elemGuid);

    // Сбрасывает свойства синхронизации для одного элемента с заданным набором свойств.
    void ResetSyncPropertyOne (const API_Guid &elemGuid, GS::Array<API_Property> &propertywrite);
} // namespace TestFunc

    #endif
#endif
