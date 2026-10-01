//------------ kuvbur 2022 ------------
#ifdef TESTING
    #include <cstdlib> // std::getenv для отбора наборов (SMSTF_TEST)

    #include "ACAPinc.h"

    #include "api_headers/APIEnvir.h"

    #include "tests/TestFunc.hpp"

    #include "tests/TestKit.hpp"

namespace TestFunc {

    // Результат тестов пишется TestKit в файл отчёта (TestKit::Run), который
    // переживает падение ArchiCAD; в панель «Отладка» строки дублируются
    // тем же каналом, что и раньше. Прод-DBtest/DBprnt из CommonFunction.hpp
    // здесь НЕ используются: они объявлены в общем заголовке и вызываются
    // также из Helpers.cpp/CommonFunction.cpp, где вывод остаётся прежним.
    // Тег функции-владельца (#230) формирует сам TestKit - см. TestKit::Loc.

    // Группы наборов: по ним работает отбор SMSTF_TEST (TestKit::Run).
    namespace Groups {
        static const char *Spec = "spec";
        static const char *Sync = "sync";
        static const char *Param = "param";
        static const char *Format = "format";
        static const char *Renum = "renum";
        static const char *Core = "core";
    } // namespace Groups

    void Test () {
        // Реестр наполняется явно (не статическими инициализаторами): набор
        // объявляется один раз здесь и вызывается через TestKit::Run, который
        // печатает BEGIN/END, считает провалы и пишет файл отчёта.
        // Отбор: переменная окружения SMSTF_TEST — пусто (всё), группа
        // ("spec", "sync", "param", "format", "renum", "core"), префикс с
        // звёздочкой ("TestSpec*") или список через запятую.
        TestKit::Register ("TestSpecGetParamValue", Groups::Spec, TestSpecGetParamValue);
        TestKit::Register ("TestSpecGrouping", Groups::Spec, TestSpecGrouping);
        TestKit::Register ("TestSpecReconcile", Groups::Spec, TestSpecReconcile);
        TestKit::Register ("TestSpecMergeAndKey", Groups::Spec, TestSpecMergeAndKey);
        TestKit::Register ("TestSpecRuleDedup", Groups::Spec, TestSpecRuleDedup);
        TestKit::Register ("TestSpecReadPlan", Groups::Spec, TestSpecReadPlan);
        TestKit::Register ("TestSpecRuleDependencies", Groups::Spec, TestSpecRuleDependencies);
        TestKit::Register ("TestSpecFavoriteResolution", Groups::Spec, TestSpecFavoriteResolution);
        TestKit::Register ("TestSpecValueReader", Groups::Spec, TestSpecValueReader);
        TestKit::Register ("TestSpecReadBoundary", Groups::Spec, TestSpecReadBoundary);
        TestKit::Register ("TestSpecPlanning", Groups::Spec, TestSpecPlanning);
        TestKit::Register ("TestSpecContribution", Groups::Spec, TestSpecContribution);
        TestKit::Register ("TestSpecRowLayout", Groups::Spec, TestSpecRowLayout);
        TestKit::Register ("TestSpecRowSlots", Groups::Spec, TestSpecRowSlots);
        TestKit::Register ("TestSpecEngineEquivalence", Groups::Spec, TestSpecEngineEquivalence);
        TestKit::Register ("TestSpecOutputSchema", Groups::Spec, TestSpecOutputSchema);
        TestKit::Register ("TestSpecValueEdges", Groups::Spec, TestSpecValueEdges);
        TestKit::Register ("TestSpecOutSlots", Groups::Spec, TestSpecOutSlots);
        TestKit::Register ("TestSpecSlotBindings", Groups::Spec, TestSpecSlotBindings);
        TestKit::Register ("TestSpecExpandGroup", Groups::Spec, TestSpecExpandGroup);
        TestKit::Register ("TestSpecGroups", Groups::Spec, TestSpecGroups);
        TestKit::Register ("TestSpecOutputSchema", Groups::Spec, TestSpecOutputSchema);
        TestKit::Register ("TestSpecPolicy", Groups::Spec, TestSpecPolicy);
        TestKit::Register ("TestSpecNormalize", Groups::Spec, TestSpecNormalize);
        TestKit::Register ("TestSpecParser", Groups::Spec, TestSpecParser);
        TestKit::Register ("TestSpecAddRule", Groups::Spec, TestSpecAddRule);
        TestKit::Register ("TestSpecParseError", Groups::Spec, TestSpecParseError);
        TestKit::Register ("TestSpecSizes", Groups::Spec, TestSpecSizes);

        TestKit::Register ("TestStringSplt", Groups::Core, TestStringSplt);
        TestKit::Register ("TestBuildOtdByParent", Groups::Core, TestBuildOtdByParent);

        TestKit::Register ("TestCalc", Groups::Format, TestCalc);
        TestKit::Register ("TestFormula", Groups::Format, TestFormula);
        TestKit::Register ("TestFormatString", Groups::Format, TestFormatString);

        TestKit::Register ("TestConvertToParamValue", Groups::Param, TestConvertToParamValue);
        TestKit::Register ("TestConvertAttributeToParamValue", Groups::Param, TestConvertAttributeToParamValue);
        TestKit::Register ("TestConvertPropertyToParamValue", Groups::Param, TestConvertPropertyToParamValue);
        TestKit::Register (
            "TestConvertPropertyDefinitionToParamValue", Groups::Param, TestConvertPropertyDefinitionToParamValue);
        TestKit::Register ("TestSetParamValueSourseByName", Groups::Param, TestSetParamValueSourseByName);
        TestKit::Register ("TestSetrawNameFromProperty", Groups::Param, TestSetrawNameFromProperty);
        TestKit::Register ("TestCheckIgnoreVal", Groups::Param, TestCheckIgnoreVal);
        TestKit::Register ("TestReadProperty", Groups::Param, TestReadProperty);
        TestKit::Register ("TestAddProperty", Groups::Param, TestAddProperty);
        TestKit::Register ("TestPropertyHelpersToString", Groups::Param, TestPropertyHelpersToString);
        TestKit::Register ("TestGetPropertyRuleFlag", Groups::Param, TestGetPropertyRuleFlag);
        TestKit::Register (
            "TestPropertyRuleFlagOnProjectElements", Groups::Param, TestPropertyRuleFlagOnProjectElements);

        TestKit::Register ("TestName2Rawname", Groups::Sync, TestName2Rawname);
        TestKit::Register ("TestName2RawnameWithBrackets", Groups::Sync, TestName2RawnameWithBrackets);
        TestKit::Register ("TestSyncString", Groups::Sync, TestSyncString);
        TestKit::Register ("TestSyncStringRealRules", Groups::Sync, TestSyncStringRealRules);
        TestKit::Register ("TestParsePrefixes", Groups::Sync, TestParsePrefixes);
        TestKit::Register ("TestParsePropertyDescription", Groups::Sync, TestParsePropertyDescription);
        TestKit::Register ("TestParseSyncStringIndependent", Groups::Sync, TestParseSyncStringIndependent);
        TestKit::Register ("TestParsePropertyDescriptionToRules", Groups::Sync, TestParsePropertyDescriptionToRules);
        TestKit::Register ("TestSyncAddSubelement", Groups::Sync, TestSyncAddSubelement);
        TestKit::Register ("TestDescToRulesSubGuid", Groups::Sync, TestDescToRulesSubGuid);

        TestKit::Register ("TestRenumPosLogic", Groups::Renum, TestRenumPosLogic);

        const char *filter = std::getenv ("SMSTF_TEST");
        const int failed = TestKit::Run (filter);
        if (failed > 0)
            TestKit::Info ("RESULT", GS::UniString::Printf ("%d failed assertions, see report", failed));
    }

} // namespace TestFunc
#endif
