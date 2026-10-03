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

    // Проверяет SyncAddSubelement — развёртывание правил from_sub/to_sub на подэлементы.
    // Включает тест на известный дефект развёртки to_sub и регрессии

    // Регрессии логики нумерации: RenumPos (конструкторы, Add, FormatToMax,

    // Регрессии ParsePropertyDescriptionToRules для to_sub/from_sub/GUID:
    // targetType/targetName/hasSub/hasGUID/guidSourceProperty. Фиксирует контракт,

    // Диагностика моста BrowserPalette: воспроизводит путь GetPropertiesList на
    // реальных элементах проекта и печатает длины описаний из двух источников

    // Выводит все встроенные свойства в отладочный журнал.
    void DumpAllBuiltInProperties ();

    // Сбрасывает свойства синхронизации для массива элементов.
    void ResetSyncPropertyArray (GS::Array<API_Guid> guidArray);

    // Сбрасывает свойства синхронизации для одного элемента.
    void ResetSyncPropertyOne (const API_Guid &elemGuid);

    // Сбрасывает свойства синхронизации для одного элемента с заданным набором свойств.
    void ResetSyncPropertyOne (const API_Guid &elemGuid, GS::Array<API_Property> &propertywrite);

    // ---------------------------------------------------------------------
    // Наборы. Определения разнесены по файлам, раскладка совпадает с группами
    // реестра в TestFunc.cpp: spec -> TestSpec.cpp, sync -> TestSync.cpp,
    // param -> TestParam.cpp, format -> TestFormat.cpp, renum -> TestRenum.cpp,
    // core -> TestCore.cpp, общие хелперы -> TestUtil.cpp.
    // Список наборов держится в реестре, а не статическими инициализаторами:
    // иначе при разбиении файла регистратор выкидывается линковкой вместе со
    // своим набором, и тот молча исчезает из прогона.

    // --- spec: разбор правил, чтение значений, выходные слоты, расчёт, вклад, раскладка, схема, равносильность (27)
    // ---
    void TestSpecGetParamValue ();
    void TestSpecGrouping ();
    void TestSpecReconcile ();
    void TestSpecMergeAndKey ();
    void TestSpecRuleDedup ();
    void TestSpecReadPlan ();
    void TestSpecRuleDependencies ();
    void TestSpecFavoriteResolution ();
    void TestSpecValueReader ();
    void TestSpecReadBoundary ();
    void TestSpecPlanning ();
    void TestSpecContribution ();
    void TestSpecRowLayout ();
    void TestSpecRowSlots ();
    void TestSpecReconcileFixtures ();
    void TestSpecChangePlan ();
    void TestSpecReconcileScaling ();
    void TestSpecBuildRowParam ();
    void TestSpecResponseContract ();
    void TestSpecRunCounters ();
    void TestSpecRunReport ();
    void TestSpecEngineEquivalence ();
    void TestSpecValueEdges ();
    void TestSpecOutSlots ();
    void TestSpecSlotOrder ();
    void TestSpecRunStateBoundary ();
    void TestSpecSelectionPolicy ();
    void TestSpecPlanCompleteness ();
    void TestSpecSlotBindings ();
    void TestSpecExpandGroup ();
    void TestSpecGroups ();
    void TestSpecOutputSchema ();
    void TestSpecPolicy ();
    void TestSpecNormalize ();
    void TestSpecParser ();
    void TestSpecAddRule ();
    void TestSpecParseError ();
    void TestSpecRuleCheck ();
    void TestSpecSizes ();

    // --- sync: правила синхронизации и разбор описаний (10) ---
    void TestName2Rawname ();
    void TestName2RawnameWithBrackets ();
    void TestSyncString ();
    void TestSyncStringRealRules ();
    void TestParsePrefixes ();
    void TestParsePropertyDescription ();
    void TestParseSyncStringIndependent ();
    void TestParsePropertyDescriptionToRules ();
    void TestSyncAddSubelement ();
    void TestDescToRulesSubGuid ();

    // --- param: значения, свойства, словари (12) ---
    void TestConvertToParamValue ();
    void TestConvertAttributeToParamValue ();
    void TestConvertPropertyToParamValue ();
    void TestConvertPropertyDefinitionToParamValue ();
    void TestSetParamValueSourseByName ();
    void TestSetrawNameFromProperty ();
    void TestCheckIgnoreVal ();
    void TestReadProperty ();
    void TestAddProperty ();
    void TestPropertyHelpersToString ();
    void TestGetPropertyRuleFlag ();
    void TestPropertyRuleFlagOnProjectElements ();

    // --- format: вычисления и форматирование (3) ---
    void TestCalc ();
    void TestFormula ();
    void TestFormatString ();

    // --- renum: логика нумерации (2) ---
    void TestRenumPosLogic ();
    void TestRenumFormulaParse ();
    void TestRenumRunReport ();

    // --- core: разное (2) ---
    void TestStringSplt ();
    void TestBuildOtdByParent ();
} // namespace TestFunc

    #endif
#endif
