//------------ kuvbur 2022 ------------
#ifdef TESTING
    #include "ACAPinc.h"

    #include "api_headers/APIEnvir.h"

    #include "dialogs/CommandHelpers.hpp"
    #include "Helpers.hpp"
    #include "Propertycache.hpp"
    #include "tests/TestFunc.hpp"
    #include "tests/TestKit.hpp"

namespace TestFunc {

    // -----------------------------------------------------------------------------
    // Простые тесты функций конвертации базовых типов в ParamValue (Helpers.hpp/cpp)
    // -----------------------------------------------------------------------------
    void TestConvertToParamValue () {
        ParamValue pvalue;

        // ---- ConvertIntToParamValue ----
        pvalue = ParamValue ();
        DBtest (ParamHelpers::ConvertIntToParamValue (pvalue, "TestInt", 42), "ConvertIntToParamValue : return");
        DBtest (pvalue.name, GS::UniString ("TestInt"), "ConvertIntToParamValue : name");
        DBtest (!pvalue.rawName.IsEmpty (), "ConvertIntToParamValue : rawName не пуст");
        DBtest (pvalue.val.type == API_PropertyIntegerValueType, "ConvertIntToParamValue : type");
        DBtest (pvalue.val.intValue, (Int32)42, "ConvertIntToParamValue : intValue");
        DBtest (is_equal (pvalue.val.doubleValue, 42.0), "ConvertIntToParamValue : doubleValue");
        DBtest (pvalue.val.boolValue, "ConvertIntToParamValue : boolValue (42 > 0)");
        DBtest (pvalue.val.uniStringValue, GS::UniString ("42"), "ConvertIntToParamValue : uniStringValue");
        DBtest (pvalue.isValid, "ConvertIntToParamValue : isValid");

        pvalue = ParamValue ();
        ParamHelpers::ConvertIntToParamValue (pvalue, "TestIntNeg", -5);
        DBtest (!pvalue.val.boolValue, "ConvertIntToParamValue : boolValue (-5 не > 0)");
        DBtest (pvalue.val.intValue, (Int32)(-5), "ConvertIntToParamValue : intValue (-5)");

        pvalue = ParamValue ();
        ParamHelpers::ConvertIntToParamValue (pvalue, "TestIntZero", 0);
        DBtest (!pvalue.val.boolValue, "ConvertIntToParamValue : boolValue (0 не > 0)");

        // ---- ConvertDoubleToParamValue ----
        pvalue = ParamValue ();
        DBtest (ParamHelpers::ConvertDoubleToParamValue (pvalue, "TestDouble", 3.14),
                "ConvertDoubleToParamValue : return");
        DBtest (pvalue.val.type == API_PropertyRealValueType, "ConvertDoubleToParamValue : type");
        DBtest (pvalue.val.intValue, (Int32)3, "ConvertDoubleToParamValue : intValue (усечение 3.14 -> 3)");
        DBtest (is_equal (pvalue.val.doubleValue, 3.14), "ConvertDoubleToParamValue : doubleValue");
        DBtest (pvalue.val.boolValue, "ConvertDoubleToParamValue : boolValue (!= 0)");
        DBtest (
            pvalue.val.uniStringValue, GS::UniString ("3.140"), "ConvertDoubleToParamValue : uniStringValue (%.3f)");
        DBtest (pvalue.isValid, "ConvertDoubleToParamValue : isValid");

        pvalue = ParamValue ();
        ParamHelpers::ConvertDoubleToParamValue (pvalue, "TestDoubleZero", 0.0);
        DBtest (!pvalue.val.boolValue, "ConvertDoubleToParamValue : boolValue (== 0)");

        pvalue = ParamValue ();
        ParamHelpers::ConvertDoubleToParamValue (pvalue, "TestDoubleNeg", -2.5);
        DBtest (pvalue.val.intValue, (Int32)(-2), "ConvertDoubleToParamValue : intValue (усечение -2.5 -> -2)");
        DBtest (pvalue.val.boolValue, "ConvertDoubleToParamValue : boolValue (-2.5 != 0)");

        // ---- ConvertBoolToParamValue ----
        pvalue = ParamValue ();
        DBtest (ParamHelpers::ConvertBoolToParamValue (pvalue, "TestBoolTrue", true),
                "ConvertBoolToParamValue : return");
        DBtest (pvalue.val.type == API_PropertyBooleanValueType, "ConvertBoolToParamValue : type");
        DBtest (pvalue.val.boolValue, "ConvertBoolToParamValue : boolValue (true)");
        DBtest (pvalue.val.intValue, (Int32)1, "ConvertBoolToParamValue : intValue (true -> 1)");
        DBtest (is_equal (pvalue.val.doubleValue, 1.0), "ConvertBoolToParamValue : doubleValue (true -> 1.0)");
        DBtest (!pvalue.val.uniStringValue.IsEmpty (), "ConvertBoolToParamValue : uniStringValue не пуст (true)");

        pvalue = ParamValue ();
        ParamHelpers::ConvertBoolToParamValue (pvalue, "TestBoolFalse", false);
        DBtest (!pvalue.val.boolValue, "ConvertBoolToParamValue : boolValue (false)");
        DBtest (pvalue.val.intValue, (Int32)0, "ConvertBoolToParamValue : intValue (false -> 0)");
        DBtest (is_equal (pvalue.val.doubleValue, 0.0), "ConvertBoolToParamValue : doubleValue (false -> 0.0)");

        // ---- ConvertStringToParamValue ----
        pvalue = ParamValue ();
        DBtest (ParamHelpers::ConvertStringToParamValue (pvalue, "TestStrNum", "123.5"),
                "ConvertStringToParamValue : return");
        DBtest (pvalue.val.type == API_PropertyStringValueType, "ConvertStringToParamValue : type");
        DBtest (pvalue.val.canCalculate, "ConvertStringToParamValue : canCalculate (\"123.5\" - число)");
        DBtest (is_equal (pvalue.val.doubleValue, 123.5), "ConvertStringToParamValue : doubleValue (123.5)");
        DBtest (
            pvalue.val.intValue, (Int32)124, "ConvertStringToParamValue : intValue (округление вверх 123.5 -> 124)");
        DBtest (pvalue.val.boolValue, "ConvertStringToParamValue : boolValue (непустая строка)");

        pvalue = ParamValue ();
        ParamHelpers::ConvertStringToParamValue (pvalue, "TestStrInt", "10");
        DBtest (pvalue.val.canCalculate, "ConvertStringToParamValue : canCalculate (\"10\" - число)");
        DBtest (pvalue.val.intValue, (Int32)10, "ConvertStringToParamValue : intValue (\"10\" -> 10, без округления)");

        pvalue = ParamValue ();
        ParamHelpers::ConvertStringToParamValue (pvalue, "TestStrText", "abc");
        DBtest (!pvalue.val.canCalculate, "ConvertStringToParamValue : canCalculate (\"abc\" - не число)");
        DBtest (pvalue.val.boolValue, "ConvertStringToParamValue : boolValue (\"abc\" непустая)");
        DBtest (pvalue.val.intValue, (Int32)1, "ConvertStringToParamValue : intValue (\"abc\" -> 1)");
        DBtest (is_equal (pvalue.val.doubleValue, 1.0), "ConvertStringToParamValue : doubleValue (\"abc\" -> 1.0)");

        pvalue = ParamValue ();
        ParamHelpers::ConvertStringToParamValue (pvalue, "TestStrEmpty", "");
        DBtest (!pvalue.val.boolValue, "ConvertStringToParamValue : boolValue (пустая строка)");
        DBtest (!pvalue.val.canCalculate, "ConvertStringToParamValue : canCalculate (пустая строка)");

        return;
    }

    // -----------------------------------------------------------------------------
    // Тест ConvertAttributeToParamValue на граничных значениях API_Attribute
    // -----------------------------------------------------------------------------
    void TestConvertAttributeToParamValue () {
        ParamValue pvalue;
        API_Attribute attrib;

        // ---- Граница: индекс атрибута = 0 (несуществующий/неинициализированный индекс) ----
        pvalue = ParamValue ();
        attrib = {};
        attrib.header.typeID = API_LayerID;
    #ifdef ServerMainVers_2700
        attrib.header.index = ACAPI_CreateAttributeIndex (0);
    #else
        attrib.header.index = 0;
    #endif
        ParamHelpers::ConvertAttributeToParamValue (pvalue, "TestAttrZero", attrib);
        DBtest (pvalue.val.intValue, (Int32)0, "ConvertAttributeToParamValue : intValue (index == 0)");
        DBtest (!pvalue.val.boolValue, "ConvertAttributeToParamValue : boolValue (index == 0, не > 0)");
        DBtest (is_equal (pvalue.val.doubleValue, 0.0), "ConvertAttributeToParamValue : doubleValue (index == 0)");
        DBtest (pvalue.val.type == API_PropertyStringValueType, "ConvertAttributeToParamValue : type");
        DBtest (pvalue.isValid, "ConvertAttributeToParamValue : isValid");
        DBtest (pvalue.fromAttribElement, "ConvertAttributeToParamValue : fromAttribElement");
        DBtest (pvalue.val.uniStringValue.IsEmpty (),
                "ConvertAttributeToParamValue : uniStringValue пуст (имя не задано)");

        // ---- Граница: минимально возможный положительный индекс (1) ----
        pvalue = ParamValue ();
        attrib = {};
        attrib.header.typeID = API_LayerID;
    #ifdef ServerMainVers_2700
        attrib.header.index = ACAPI_CreateAttributeIndex (1);
    #else
        attrib.header.index = 1;
    #endif
        ParamHelpers::ConvertAttributeToParamValue (pvalue, "TestAttrOne", attrib);
        DBtest (pvalue.val.intValue, (Int32)1, "ConvertAttributeToParamValue : intValue (index == 1)");
        DBtest (pvalue.val.boolValue, "ConvertAttributeToParamValue : boolValue (index == 1 > 0)");
        DBtest (is_equal (pvalue.val.doubleValue, 1.0), "ConvertAttributeToParamValue : doubleValue (index == 1)");

        // ---- Граница: максимальный short-индекс (граница типа для старых версий ACAPI) ----
        pvalue = ParamValue ();
        attrib = {};
        attrib.header.typeID = API_LayerID;
    #ifdef ServerMainVers_2700
        attrib.header.index = ACAPI_CreateAttributeIndex (std::numeric_limits<short>::max ());
    #else
        attrib.header.index = std::numeric_limits<short>::max ();
    #endif
        ParamHelpers::ConvertAttributeToParamValue (pvalue, "TestAttrMax", attrib);
        DBtest (pvalue.val.intValue,
                (Int32)std::numeric_limits<short>::max (),
                "ConvertAttributeToParamValue : intValue (index == SHRT_MAX)");
        DBtest (pvalue.val.boolValue, "ConvertAttributeToParamValue : boolValue (SHRT_MAX > 0)");
        DBtest (is_equal (pvalue.val.doubleValue, (double)std::numeric_limits<short>::max ()),
                "ConvertAttributeToParamValue : doubleValue (index == SHRT_MAX)");

        // ---- Граница: rawName уже задан вызывающей стороной -> не должен перезаписываться ----
        pvalue = ParamValue ();
        pvalue.rawName = "{@attrib:custom_rawname}";
        attrib = {};
        attrib.header.typeID = API_LayerID;
    #ifdef ServerMainVers_2700
        attrib.header.index = ACAPI_CreateAttributeIndex (5);
    #else
        attrib.header.index = 5;
    #endif
        ParamHelpers::ConvertAttributeToParamValue (pvalue, "TestAttrCustomRaw", attrib);
        DBtest (pvalue.rawName,
                GS::UniString ("{@attrib:custom_rawname}"),
                "ConvertAttributeToParamValue : rawName не перезаписывается, если не пуст");

        // Примечание: конкретное имя атрибута (attr.header.name) в данном тесте не заполняется -
        // это отдельное низкоуровневое поле фиксированного размера в API_AttributeHeader,
        // корректно заполняемое реальным ACAPI_Attribute_Get. Пустое имя - тоже граничный случай,
        // корректно обрабатываемый функцией (см. проверку uniStringValue.IsEmpty () выше).
        return;
    }

    // -----------------------------------------------------------------------------
    // Тест ConvertToParamValue (API_Property) на граничных значениях
    // -----------------------------------------------------------------------------
    void TestConvertPropertyToParamValue () {
        ParamValue pvalue;
        API_Property property;

        // ---- Integer: граничные значения INT32_MIN / 0 / INT32_MAX ----
        pvalue = ParamValue ();
        pvalue.rawName = "{@property:testint_min}";
        pvalue.name = "TestIntMin";
        property = {};
        property.isDefault = false;
    #ifndef ServerMainVers_2400
        property.isEvaluated = true;
    #else
        property.status = API_Property_HasValue;
    #endif
        property.definition.collectionType = API_PropertySingleCollectionType;
        property.definition.valueType = API_PropertyIntegerValueType;
        property.value.singleVariant.variant.type = API_PropertyIntegerValueType;
        property.value.singleVariant.variant.intValue = std::numeric_limits<Int32>::min ();
        DBtest (ParamHelpers::ConvertToParamValue (pvalue, property),
                "ConvertToParamValue(Property) : return (Integer INT32_MIN)");
        DBtest (pvalue.val.intValue,
                std::numeric_limits<Int32>::min (),
                "ConvertToParamValue(Property) : intValue (INT32_MIN)");
        DBtest (!pvalue.val.boolValue, "ConvertToParamValue(Property) : boolValue (INT32_MIN не > 0)");
        DBtest (pvalue.val.type == API_PropertyIntegerValueType, "ConvertToParamValue(Property) : type (Integer)");
        DBtest (pvalue.isValid, "ConvertToParamValue(Property) : isValid (HasValue)");

        pvalue = ParamValue ();
        pvalue.rawName = "{@property:testint_zero}";
        pvalue.name = "TestIntZero";
        property.value.singleVariant.variant.intValue = 0;
        ParamHelpers::ConvertToParamValue (pvalue, property);
        DBtest (!pvalue.val.boolValue, "ConvertToParamValue(Property) : boolValue (0 не > 0)");

        pvalue = ParamValue ();
        pvalue.rawName = "{@property:testint_max}";
        pvalue.name = "TestIntMax";
        property.value.singleVariant.variant.intValue = std::numeric_limits<Int32>::max ();
        ParamHelpers::ConvertToParamValue (pvalue, property);
        DBtest (pvalue.val.intValue,
                std::numeric_limits<Int32>::max (),
                "ConvertToParamValue(Property) : intValue (INT32_MAX)");
        DBtest (pvalue.val.boolValue, "ConvertToParamValue(Property) : boolValue (INT32_MAX > 0)");

        // ---- Real: 0.0 (граница bool == false), максимум double, отрицательное значение ----
        pvalue = ParamValue ();
        pvalue.rawName = "{@property:testreal_zero}";
        pvalue.name = "TestRealZero";
        property = {};
    #ifndef ServerMainVers_2400
        property.isEvaluated = true;
    #else
        property.status = API_Property_HasValue;
    #endif
        property.definition.collectionType = API_PropertySingleCollectionType;
        property.definition.valueType = API_PropertyRealValueType;
        property.definition.measureType = API_PropertyUndefinedMeasureType;
        property.value.singleVariant.variant.type = API_PropertyRealValueType;
        property.value.singleVariant.variant.doubleValue = 0.0;
        ParamHelpers::ConvertToParamValue (pvalue, property);
        DBtest (!pvalue.val.boolValue, "ConvertToParamValue(Property) : boolValue (0.0 == 0)");
        DBtest (pvalue.val.type == API_PropertyRealValueType, "ConvertToParamValue(Property) : type (Real)");

        pvalue = ParamValue ();
        pvalue.rawName = "{@property:testreal_neg}";
        pvalue.name = "TestRealNeg";
        property.value.singleVariant.variant.doubleValue = -123456.789;
        ParamHelpers::ConvertToParamValue (pvalue, property);
        DBtest (pvalue.val.boolValue, "ConvertToParamValue(Property) : boolValue (отрицательное != 0)");
        DBtest (is_equal (pvalue.val.doubleValue, -123456.789),
                "ConvertToParamValue(Property) : doubleValue (отрицательное)");

        pvalue = ParamValue ();
        pvalue.rawName = "{@property:testreal_max}";
        pvalue.name = "TestRealMax";
        property.value.singleVariant.variant.doubleValue = std::numeric_limits<double>::max ();
        ParamHelpers::ConvertToParamValue (pvalue, property);
        DBtest (pvalue.val.boolValue, "ConvertToParamValue(Property) : boolValue (DBL_MAX != 0)");

        // ---- Boolean: true / false ----
        pvalue = ParamValue ();
        pvalue.rawName = "{@property:testbool_true}";
        pvalue.name = "TestBoolTrue";
        property = {};
    #ifndef ServerMainVers_2400
        property.isEvaluated = true;
    #else
        property.status = API_Property_HasValue;
    #endif
        property.definition.collectionType = API_PropertySingleCollectionType;
        property.definition.valueType = API_PropertyBooleanValueType;
        property.value.singleVariant.variant.type = API_PropertyBooleanValueType;
        property.value.singleVariant.variant.boolValue = true;
        ParamHelpers::ConvertToParamValue (pvalue, property);
        DBtest (pvalue.val.boolValue, "ConvertToParamValue(Property) : boolValue (true)");
        DBtest (pvalue.val.intValue, (Int32)1, "ConvertToParamValue(Property) : intValue (true -> 1)");
        DBtest (pvalue.val.type == API_PropertyBooleanValueType, "ConvertToParamValue(Property) : type (Boolean)");

        pvalue = ParamValue ();
        pvalue.rawName = "{@property:testbool_false}";
        pvalue.name = "TestBoolFalse";
        property.value.singleVariant.variant.boolValue = false;
        ParamHelpers::ConvertToParamValue (pvalue, property);
        DBtest (!pvalue.val.boolValue, "ConvertToParamValue(Property) : boolValue (false)");

        // ---- String: пустая строка / длинная юникод-строка ----
        pvalue = ParamValue ();
        pvalue.rawName = "{@property:teststr_empty}";
        pvalue.name = "TestStrEmpty";
        property = {};
    #ifndef ServerMainVers_2400
        property.isEvaluated = true;
    #else
        property.status = API_Property_HasValue;
    #endif
        property.definition.collectionType = API_PropertySingleCollectionType;
        property.definition.valueType = API_PropertyStringValueType;
        property.value.singleVariant.variant.type = API_PropertyStringValueType;
        property.value.singleVariant.variant.uniStringValue = "";
        ParamHelpers::ConvertToParamValue (pvalue, property);
        DBtest (!pvalue.val.boolValue, "ConvertToParamValue(Property) : boolValue (пустая строка)");
        DBtest (pvalue.val.type == API_PropertyStringValueType, "ConvertToParamValue(Property) : type (String)");

        pvalue = ParamValue ();
        pvalue.rawName = "{@property:teststr_long}";
        pvalue.name = "TestStrLong";
        GS::UniString longString = "";
        for (UInt32 i = 0; i < 200; i++)
            longString.Append ("Ё");
        property.value.singleVariant.variant.uniStringValue = longString;
        ParamHelpers::ConvertToParamValue (pvalue, property);
        DBtest (pvalue.val.boolValue, "ConvertToParamValue(Property) : boolValue (длинная строка непустая)");
        DBtest (pvalue.val.uniStringValue.GetLength () == 200,
                "ConvertToParamValue(Property) : uniStringValue длина сохранена");

        // ---- Undefined: функция должна вернуть false и isValid == false ----
        pvalue = ParamValue ();
        pvalue.rawName = "{@property:testundef}";
        pvalue.name = "TestUndefined";
        property = {};
    #ifndef ServerMainVers_2400
        property.isEvaluated = true;
    #else
        property.status = API_Property_HasValue;
    #endif
        property.definition.collectionType = API_PropertySingleCollectionType;
        property.definition.valueType = API_PropertyUndefinedValueType;
        DBtest (!ParamHelpers::ConvertToParamValue (pvalue, property),
                "ConvertToParamValue(Property) : return (Undefined -> false)");
        DBtest (!pvalue.isValid, "ConvertToParamValue(Property) : isValid (Undefined -> false)");

        return;
    }

    // -----------------------------------------------------------------------------
    // Тест ConvertToParamValue (API_PropertyDefinition) на граничных значениях
    // -----------------------------------------------------------------------------
    void TestConvertPropertyDefinitionToParamValue () {
        ParamValue pvalue;
        API_PropertyDefinition definition;

        // ---- Обычное определение без специальных маркеров в description ----
        pvalue = ParamValue ();
        pvalue.rawName = "{@property:testdef_plain}";
        pvalue.name = "TestDefPlain";
        definition = {};
        definition.guid = APINULLGuid;
        definition.description = "";
        definition.valueType = API_PropertyRealValueType;
        DBtest (ParamHelpers::ConvertToParamValue (pvalue, definition), "ConvertToParamValue(Definition) : return");
        DBtest (pvalue.val.type == API_PropertyRealValueType, "ConvertToParamValue(Definition) : val.type");
        DBtest (pvalue.type == API_PropertyRealValueType, "ConvertToParamValue(Definition) : type");
        DBtest (pvalue.fromProperty, "ConvertToParamValue(Definition) : fromProperty");
        DBtest (pvalue.fromPropertyDefinition,
                "ConvertToParamValue(Definition) : fromPropertyDefinition (нет спецмаркеров)");

        // ---- Граница: description содержит SYNCNAME -> особый rawName ----
        pvalue = ParamValue ();
        pvalue.rawName = "{@property:anything}";
        pvalue.name = "TestDefSyncName";
        definition = {};
        definition.guid = APINULLGuid;
        definition.description = SYNCNAME;
        definition.valueType = API_PropertyStringValueType;
        ParamHelpers::ConvertToParamValue (pvalue, definition);
        DBtest (pvalue.rawName,
                GS::UniString ("{@property:sync_name0}"),
                "ConvertToParamValue(Definition) : rawName (sync_name)");
        DBtest (pvalue.fromAttribDefinition, "ConvertToParamValue(Definition) : fromAttribDefinition (sync_name)");

        // ---- Граница: rawName содержит "buildingmaterial" -> fromAttribDefinition, без изменения rawName ----
        pvalue = ParamValue ();
        pvalue.rawName = "{@property:buildingmaterialproperties/density}";
        pvalue.name = "TestDefBuildingMaterial";
        definition = {};
        definition.guid = APINULLGuid;
        definition.description = "";
        definition.valueType = API_PropertyRealValueType;
        ParamHelpers::ConvertToParamValue (pvalue, definition);
        DBtest (pvalue.fromAttribDefinition,
                "ConvertToParamValue(Definition) : fromAttribDefinition (buildingmaterial in rawName)");
        DBtest (!pvalue.fromPropertyDefinition,
                "ConvertToParamValue(Definition) : fromPropertyDefinition == false (attrib имеет приоритет)");

        // ---- Граница: пустой guid и пустое valueType (Undefined) - функция всё равно возвращает true ----
        pvalue = ParamValue ();
        pvalue.rawName = "{@property:testdef_undef}";
        pvalue.name = "TestDefUndefined";
        definition = {};
        definition.guid = APINULLGuid;
        definition.description = "";
        definition.valueType = API_PropertyUndefinedValueType;
        DBtest (ParamHelpers::ConvertToParamValue (pvalue, definition),
                "ConvertToParamValue(Definition) : return (Undefined valueType всё равно true)");
        DBtest (pvalue.val.type == API_PropertyUndefinedValueType,
                "ConvertToParamValue(Definition) : val.type (Undefined)");
        return;
    }

    // -----------------------------------------------------------------------------
    // Тест SetParamValueSourseByName - назначение флагов источника по префиксу rawName
    // -----------------------------------------------------------------------------
    void TestSetParamValueSourseByName () {
        ParamValue pvalue;

        // ---- PROPERTYNAMEPREFIX ----
        pvalue = ParamValue ();
        pvalue.rawName = PROPERTYNAMEPREFIX + GS::UniString ("testprop") + BRACEEND;
        ParamHelpers::SetParamValueSourseByName (pvalue);
        DBtest (pvalue.fromPropertyDefinition,
                "SetParamValueSourseByName : fromPropertyDefinition (PROPERTYNAMEPREFIX)");
        DBtest (pvalue.typeinx, (short)PROPERTYTYPEINX, "SetParamValueSourseByName : typeinx (PROPERTYNAMEPREFIX)");
        DBtest (!pvalue.fromAttribDefinition,
                "SetParamValueSourseByName : fromAttribDefinition == false (обычное свойство)");

        // ---- Граница: PROPERTYNAMEPREFIX + "buildingmaterialproperties/" -> дополнительно fromAttribDefinition ----
        pvalue = ParamValue ();
        pvalue.rawName = PROPERTYNAMEPREFIX + GS::UniString ("buildingmaterialproperties/density") + BRACEEND;
        ParamHelpers::SetParamValueSourseByName (pvalue);
        DBtest (pvalue.fromPropertyDefinition, "SetParamValueSourseByName : fromPropertyDefinition (buildingmaterial)");
        DBtest (pvalue.fromAttribDefinition,
                "SetParamValueSourseByName : fromAttribDefinition (buildingmaterialproperties/)");

        // ---- GDLNAMEPREFIX ----
        pvalue = ParamValue ();
        pvalue.rawName = GDLNAMEPREFIX + GS::UniString ("testgdl") + BRACEEND;
        ParamHelpers::SetParamValueSourseByName (pvalue);
        DBtest (pvalue.fromGDLparam, "SetParamValueSourseByName : fromGDLparam (GDLNAMEPREFIX)");
        DBtest (!pvalue.fromGDLdescription, "SetParamValueSourseByName : fromGDLdescription == false (обычный GDL)");
        DBtest (pvalue.typeinx, (short)GDLTYPEINX, "SetParamValueSourseByName : typeinx (GDLNAMEPREFIX)");

        // ---- GDLDESCNAMEPREFIX ----
        pvalue = ParamValue ();
        pvalue.rawName = GDLDESCNAMEPREFIX + GS::UniString ("testdesc") + BRACEEND;
        ParamHelpers::SetParamValueSourseByName (pvalue);
        DBtest (pvalue.fromGDLparam, "SetParamValueSourseByName : fromGDLparam (GDLDESCNAMEPREFIX)");
        DBtest (pvalue.fromGDLdescription, "SetParamValueSourseByName : fromGDLdescription (GDLDESCNAMEPREFIX)");
        DBtest (pvalue.typeinx, (short)GDLDESCTYPEINX, "SetParamValueSourseByName : typeinx (GDLDESCNAMEPREFIX)");

        // ---- COORDNAMEPREFIX ----
        pvalue = ParamValue ();
        pvalue.rawName = COORDNAMEPREFIX + GS::UniString ("testcoord") + BRACEEND;
        ParamHelpers::SetParamValueSourseByName (pvalue);
        DBtest (pvalue.fromCoord, "SetParamValueSourseByName : fromCoord (COORDNAMEPREFIX)");

        // ---- GLOBNAMEPREFIX ----
        pvalue = ParamValue ();
        pvalue.rawName = GLOBNAMEPREFIX + GS::UniString ("testglob") + BRACEEND;
        ParamHelpers::SetParamValueSourseByName (pvalue);
        DBtest (pvalue.fromGlob, "SetParamValueSourseByName : fromGlob (GLOBNAMEPREFIX)");

        // ---- INFONAMEPREFIX ----
        pvalue = ParamValue ();
        pvalue.rawName = INFONAMEPREFIX + GS::UniString ("testinfo") + BRACEEND;
        ParamHelpers::SetParamValueSourseByName (pvalue);
        DBtest (pvalue.fromInfo, "SetParamValueSourseByName : fromInfo (INFONAMEPREFIX)");

        // ---- MEPNAMEPREFIX ----
        pvalue = ParamValue ();
        pvalue.rawName = MEPNAMEPREFIX + GS::UniString ("testmep") + BRACEEND;
        ParamHelpers::SetParamValueSourseByName (pvalue);
        DBtest (pvalue.fromMEP, "SetParamValueSourseByName : fromMEP (MEPNAMEPREFIX)");

        // ---- FORMULANAMEPREFIX (устанавливает val.hasFormula, а не from*-флаг) ----
        pvalue = ParamValue ();
        pvalue.rawName = FORMULANAMEPREFIX + GS::UniString ("testformula") + BRACEEND;
        ParamHelpers::SetParamValueSourseByName (pvalue);
        DBtest (pvalue.val.hasFormula, "SetParamValueSourseByName : val.hasFormula (FORMULANAMEPREFIX)");

        // ---- IDNAMEPREFIX ----
        pvalue = ParamValue ();
        pvalue.rawName = IDNAMEPREFIX + GS::UniString ("testid") + BRACEEND;
        ParamHelpers::SetParamValueSourseByName (pvalue);
        DBtest (pvalue.fromID, "SetParamValueSourseByName : fromID (IDNAMEPREFIX)");

        // ---- IFCNAMEPREFIX ----
        pvalue = ParamValue ();
        pvalue.rawName = IFCNAMEPREFIX + GS::UniString ("testifc") + BRACEEND;
        ParamHelpers::SetParamValueSourseByName (pvalue);
        DBtest (pvalue.fromIFCProperty, "SetParamValueSourseByName : fromIFCProperty (IFCNAMEPREFIX)");

        // ---- MORPHNAMEPREFIX ----
        pvalue = ParamValue ();
        pvalue.rawName = MORPHNAMEPREFIX + GS::UniString ("testmorph") + BRACEEND;
        ParamHelpers::SetParamValueSourseByName (pvalue);
        DBtest (pvalue.fromMorph, "SetParamValueSourseByName : fromMorph (MORPHNAMEPREFIX)");

        // ---- ATTRIBNAMEPREFIX ----
        pvalue = ParamValue ();
        pvalue.rawName = ATTRIBNAMEPREFIX + GS::UniString ("testattrib") + BRACEEND;
        ParamHelpers::SetParamValueSourseByName (pvalue);
        DBtest (pvalue.fromAttribElement, "SetParamValueSourseByName : fromAttribElement (ATTRIBNAMEPREFIX)");

        // ---- LISTDATANAMEPREFIX ----
        pvalue = ParamValue ();
        pvalue.rawName = LISTDATANAMEPREFIX + GS::UniString ("testlistdata") + BRACEEND;
        ParamHelpers::SetParamValueSourseByName (pvalue);
        DBtest (pvalue.fromListData, "SetParamValueSourseByName : fromListData (LISTDATANAMEPREFIX)");

        // ---- MATERIALNAMEPREFIX ----
        pvalue = ParamValue ();
        pvalue.rawName = MATERIALNAMEPREFIX + GS::UniString ("testmaterial") + BRACEEND;
        ParamHelpers::SetParamValueSourseByName (pvalue);
        DBtest (pvalue.fromMaterial, "SetParamValueSourseByName : fromMaterial (MATERIALNAMEPREFIX)");

        // ---- CLASSNAMEPREFIX ----
        pvalue = ParamValue ();
        pvalue.rawName = CLASSNAMEPREFIX + GS::UniString ("testclass") + BRACEEND;
        ParamHelpers::SetParamValueSourseByName (pvalue);
        DBtest (pvalue.fromClassification, "SetParamValueSourseByName : fromClassification (CLASSNAMEPREFIX)");

        // ---- ELEMENTNAMEPREFIX ----
        pvalue = ParamValue ();
        pvalue.rawName = ELEMENTNAMEPREFIX + GS::UniString ("testelement") + BRACEEND;
        ParamHelpers::SetParamValueSourseByName (pvalue);
        DBtest (pvalue.fromElement, "SetParamValueSourseByName : fromElement (ELEMENTNAMEPREFIX)");

        // ---- FILENAMEPREFIX ----
        pvalue = ParamValue ();
        pvalue.rawName = FILENAMEPREFIX + GS::UniString ("testfile") + BRACEEND;
        ParamHelpers::SetParamValueSourseByName (pvalue);
        DBtest (pvalue.fromFile, "SetParamValueSourseByName : fromFile (FILENAMEPREFIX)");

        // ---- Граница: неизвестный префикс -> typeinx == 0, ни один флаг не установлен ----
        pvalue = ParamValue ();
        pvalue.rawName = "{@unknownprefix:test}";
        ParamHelpers::SetParamValueSourseByName (pvalue);
        DBtest (pvalue.typeinx, (short)0, "SetParamValueSourseByName : typeinx (неизвестный префикс)");
        DBtest (!pvalue.fromProperty && !pvalue.fromGDLparam && !pvalue.fromCoord && !pvalue.fromElement,
                "SetParamValueSourseByName : флаги не установлены (неизвестный префикс)");

        // ---- Граница: ранний выход, если уже установлен любой from*-флаг (например, fromProperty) ----
        pvalue = ParamValue ();
        pvalue.rawName = GDLNAMEPREFIX + GS::UniString ("shouldnotchange") + BRACEEND;
        pvalue.fromProperty = true; // Уже определён источник
        ParamHelpers::SetParamValueSourseByName (pvalue);
        DBtest (!pvalue.fromGDLparam,
                "SetParamValueSourseByName : ранний выход - fromGDLparam не выставляется, если fromProperty уже true");
        DBtest (pvalue.typeinx, (short)0, "SetParamValueSourseByName : ранний выход - typeinx не пересчитывается");

        // ---- Граница: GDL rawName с индексами массива (@arr_row_start_row_end_col_start_col_end) ----
        pvalue = ParamValue ();
        pvalue.rawName = GDLNAMEPREFIX + GS::UniString ("myarrparam@arr_3_5_7_9") + BRACEEND;
        ParamHelpers::SetParamValueSourseByName (pvalue);
        DBtest (pvalue.fromGDLparam, "SetParamValueSourseByName : fromGDLparam (GDL с @arr_)");
        DBtest (pvalue.val.array_row_start, 3, "SetParamValueSourseByName : val.array_row_start (@arr_3_5_7_9)");
        DBtest (pvalue.val.array_row_end, 5, "SetParamValueSourseByName : val.array_row_end (@arr_3_5_7_9)");
        DBtest (pvalue.val.array_column_start, 7, "SetParamValueSourseByName : val.array_column_start (@arr_3_5_7_9)");
        DBtest (pvalue.val.array_column_end, 9, "SetParamValueSourseByName : val.array_column_end (@arr_3_5_7_9)");

        return;
    }

    // -----------------------------------------------------------------------------
    // Тест SetrawNameFromProperty - переопределение rawName/name по описанию свойства
    // -----------------------------------------------------------------------------
    void TestSetrawNameFromProperty () {
        ParamValue pvalue;
        API_Property property;

        // ---- Обычное свойство, rawName/name уже заданы вызывающей стороной, описание без спецмаркеров ----
        // (rawName/name предзаданы, чтобы не зависеть от внешней GetPropertyFullName)
        pvalue = ParamValue ();
        pvalue.rawName = "{@property:testplain}";
        pvalue.name = "TestPlain";
        property = {};
        property.definition.description = "";
        ParamHelpers::SetrawNameFromProperty (pvalue, property);
        DBtest (pvalue.rawName,
                GS::UniString ("{@property:testplain}"),
                "SetrawNameFromProperty : rawName не меняется (обычное свойство)");
        DBtest (
            pvalue.name, GS::UniString ("TestPlain"), "SetrawNameFromProperty : name не меняется (обычное свойство)");

        // ---- Граница: rawName без CharENTER (fromAttrib == false) -> спецветки some_stuff_* не срабатывают,
        //      даже если описание их содержит ----
        pvalue = ParamValue ();
        pvalue.rawName = "{@property:notfromattrib}";
        pvalue.name = "NotFromAttrib";
        property = {};
        property.definition.description = "some_stuff_th";
        ParamHelpers::SetrawNameFromProperty (pvalue, property);
        DBtest (pvalue.rawName,
                GS::UniString ("{@property:notfromattrib}"),
                "SetrawNameFromProperty : rawName не меняется без CharENTER (fromAttrib == false)");
        DBtest (pvalue.name,
                GS::UniString ("NotFromAttrib"),
                "SetrawNameFromProperty : name не меняется без CharENTER (fromAttrib == false)");

        // ---- Граница: rawName содержит CharENTER (fromAttrib == true) + описание "some_stuff_th" ----
        pvalue = ParamValue ();
        pvalue.rawName = GS::UniString ("{@property:some_stuff_th") + CharENTER + GS::UniString ("7") + BRACEEND;
        pvalue.name = "Original";
        property = {};
        property.definition.description = "Some_Stuff_TH"; // Проверка регистронезависимости (ToLowerCase внутри)
        ParamHelpers::SetrawNameFromProperty (pvalue, property);
        DBtest (pvalue.rawName.BeginsWith ("{@property:buildingmaterialproperties/some_stuff_th"),
                "SetrawNameFromProperty : rawName переписан на some_stuff_th");
        DBtest (pvalue.rawName.Contains ("7"), "SetrawNameFromProperty : сохранён индекс атрибута (some_stuff_th)");
        DBtest (pvalue.name, GS::UniString ("some_stuff_th"), "SetrawNameFromProperty : name == some_stuff_th");
        DBtest (!pvalue.val.formatstring.isEmpty, "SetrawNameFromProperty : formatstring задан для some_stuff_th");

        // ---- Граница: fromAttrib == true + описание "some_stuff_units" ----
        pvalue = ParamValue ();
        pvalue.rawName = GS::UniString ("{@property:some_stuff_units") + CharENTER + GS::UniString ("2") + BRACEEND;
        pvalue.name = "Original";
        property = {};
        property.definition.description = "some_stuff_units";
        ParamHelpers::SetrawNameFromProperty (pvalue, property);
        DBtest (pvalue.rawName.BeginsWith ("{@property:buildingmaterialproperties/some_stuff_units"),
                "SetrawNameFromProperty : rawName переписан на some_stuff_units");
        DBtest (pvalue.rawName.Contains ("2"), "SetrawNameFromProperty : сохранён индекс атрибута (some_stuff_units)");
        DBtest (pvalue.name, GS::UniString ("some_stuff_units"), "SetrawNameFromProperty : name == some_stuff_units");

        // ---- Граница: fromAttrib == true + описание "some_stuff_kzap" ----
        pvalue = ParamValue ();
        pvalue.rawName = GS::UniString ("{@property:some_stuff_kzap") + CharENTER + GS::UniString ("1") + BRACEEND;
        pvalue.name = "Original";
        property = {};
        property.definition.description = "some_stuff_kzap";
        ParamHelpers::SetrawNameFromProperty (pvalue, property);
        DBtest (pvalue.rawName.BeginsWith ("{@property:buildingmaterialproperties/some_stuff_kzap"),
                "SetrawNameFromProperty : rawName переписан на some_stuff_kzap");
        DBtest (pvalue.name, GS::UniString ("some_stuff_kzap"), "SetrawNameFromProperty : name == some_stuff_kzap");

        // Примечание: ветка description.Contains (SYNCCORRECTFLAG) не покрыта тестом - значение
        // константы SYNCCORRECTFLAG не определено в Helpers.hpp/cpp (внешний заголовок), поэтому
        // корректную тестовую строку для срабатывания этой ветки составить нельзя.
        return;
    }

    // -----------------------------------------------------------------------------
    // Тест CheckIgnoreVal на граничных значениях
    // -----------------------------------------------------------------------------
    void TestCheckIgnoreVal () {
        ParamValue param;
        SkipValues ignorevals;

        // ---- Граница: пустая строка + skip_empty == true -> игнорировать ----
        param = ParamValue ();
        param.val.type = API_PropertyStringValueType;
        param.val.uniStringValue = "";
        ignorevals = {};
        ignorevals.skip_empty = true;
        DBtest (ParamHelpers::CheckIgnoreVal (ignorevals, param), "CheckIgnoreVal : пустая строка + skip_empty");

        // ---- Граница: строка из пробелов + skip_empty == true (без skip_trim_empty) -> НЕ игнорировать ----
        param = ParamValue ();
        param.val.type = API_PropertyStringValueType;
        param.val.uniStringValue = "   ";
        ignorevals = {};
        ignorevals.skip_empty = true;
        DBtest (!ParamHelpers::CheckIgnoreVal (ignorevals, param),
                "CheckIgnoreVal : строка из пробелов + только skip_empty -> не игнорируется");

        // ---- Граница: строка из пробелов + skip_trim_empty == true -> игнорировать ----
        param = ParamValue ();
        param.val.type = API_PropertyStringValueType;
        param.val.uniStringValue = "   ";
        ignorevals = {};
        ignorevals.skip_trim_empty = true;
        DBtest (ParamHelpers::CheckIgnoreVal (ignorevals, param),
                "CheckIgnoreVal : строка из пробелов + skip_trim_empty");

        // ---- Граница: непустая строка + skip_empty/skip_trim_empty == true -> НЕ игнорировать ----
        param = ParamValue ();
        param.val.type = API_PropertyStringValueType;
        param.val.uniStringValue = "непустое значение";
        ignorevals = {};
        ignorevals.skip_empty = true;
        ignorevals.skip_trim_empty = true;
        DBtest (!ParamHelpers::CheckIgnoreVal (ignorevals, param),
                "CheckIgnoreVal : непустая строка -> не игнорируется");

        // ---- Граница: числовой (не строковый, не булевый) тип, doubleValue == 0 + skip_empty -> игнорировать ----
        param = ParamValue ();
        param.val.type = API_PropertyRealValueType;
        param.val.doubleValue = 0.0;
        ignorevals = {};
        ignorevals.skip_empty = true;
        DBtest (ParamHelpers::CheckIgnoreVal (ignorevals, param), "CheckIgnoreVal : Real == 0.0 + skip_empty");

        // ---- Граница: числовой тип, doubleValue != 0 + skip_empty -> НЕ игнорировать ----
        param = ParamValue ();
        param.val.type = API_PropertyRealValueType;
        param.val.doubleValue = 1.0;
        ignorevals = {};
        ignorevals.skip_empty = true;
        DBtest (!ParamHelpers::CheckIgnoreVal (ignorevals, param),
                "CheckIgnoreVal : Real == 1.0 + skip_empty -> не игнорируется");

        // ---- Граница: булевый тип НЕ подчиняется skip_empty/skip_trim_empty, даже если doubleValue == 0 ----
        param = ParamValue ();
        param.val.type = API_PropertyBooleanValueType;
        param.val.doubleValue = 0.0;
        param.val.boolValue = false;
        ignorevals = {};
        ignorevals.skip_empty = true;
        ignorevals.skip_trim_empty = true;
        DBtest (!ParamHelpers::CheckIgnoreVal (ignorevals, param),
                "CheckIgnoreVal : Boolean игнорирует skip_empty/skip_trim_empty");

        // ---- Граница: пустой список ignorevals.ignorevals -> false, даже без skip-флагов ----
        param = ParamValue ();
        param.val.type = API_PropertyStringValueType;
        param.val.uniStringValue = "любое значение";
        ignorevals = {};
        DBtest (!ParamHelpers::CheckIgnoreVal (ignorevals, param),
                "CheckIgnoreVal : пустой ignorevals и все флаги false -> false");

        // ---- Граница: точное совпадение со значением из списка ignorevals ----
        param = ParamValue ();
        param.val.type = API_PropertyStringValueType;
        param.val.uniStringValue = "ignoreme";
        ignorevals = {};
        ignorevals.ignorevals.Push ("ignoreme");
        DBtest (ParamHelpers::CheckIgnoreVal (ignorevals, param), "CheckIgnoreVal : точное совпадение со списком");

        // ---- Граница: список с шаблоном "*суффикс" - совпадение по окончанию строки ----
        param = ParamValue ();
        param.val.type = API_PropertyStringValueType;
        param.val.uniStringValue = "значение_суффикс";
        ignorevals = {};
        ignorevals.ignorevals.Push ("*суффикс");
        DBtest (ParamHelpers::CheckIgnoreVal (ignorevals, param), "CheckIgnoreVal : шаблон *суффикс (EndsWith)");

        // ---- Граница: список с шаблоном "префикс*" - совпадение по началу строки ----
        param = ParamValue ();
        param.val.type = API_PropertyStringValueType;
        param.val.uniStringValue = "префикс_значение";
        ignorevals = {};
        ignorevals.ignorevals.Push ("префикс*");
        DBtest (ParamHelpers::CheckIgnoreVal (ignorevals, param), "CheckIgnoreVal : шаблон префикс* (BeginsWith)");

        // ---- Граница: список задан, но ничего не совпадает -> false ----
        param = ParamValue ();
        param.val.type = API_PropertyStringValueType;
        param.val.uniStringValue = "совершенно другое значение";
        ignorevals = {};
        ignorevals.ignorevals.Push ("ignoreme");
        ignorevals.ignorevals.Push ("*суффикс");
        DBtest (!ParamHelpers::CheckIgnoreVal (ignorevals, param),
                "CheckIgnoreVal : список задан, но нет совпадений -> false");

        // ---- Граница: сравнение со списком идёт по обрезанной (trim) строке ----
        param = ParamValue ();
        param.val.type = API_PropertyStringValueType;
        param.val.uniStringValue = "  ignoreme  ";
        ignorevals = {};
        ignorevals.ignorevals.Push ("ignoreme");
        DBtest (ParamHelpers::CheckIgnoreVal (ignorevals, param), "CheckIgnoreVal : сравнение по trim-строке");

        return;
    }

    // -----------------------------------------------------------------------------
    // Тест ReadProperty - чтение значений свойств для элемента в ParamDictValue
    // -----------------------------------------------------------------------------
    // Примечание: ReadProperty обращается к живому ACAPI_Element_GetPropertyValues,
    // поэтому полноценно проверить "успешный" путь (реальные значения свойств)
    // без реального элемента на плане нельзя. Ниже проверяются детерминированные
    // граничные случаи: обе защитные проверки в начале функции, и вызов с
    // заведомо невалидным APINULLGuid, для которого ACAPI_Element_GetPropertyValues
    // гарантированно не найдёт элемент и вернёт ошибку.
    void TestReadProperty () {
        ParamDictValue params;
        GS::Array<API_PropertyDefinition> propertyDefinitions;

        // ---- Граница: params пуст -> немедленный false, независимо от propertyDefinitions ----
        params = {};
        propertyDefinitions = {};
        DBtest (!ParamHelpers::ReadProperty (APINULLGuid, params, propertyDefinitions),
                "ReadProperty : params пуст -> false");

        API_PropertyDefinition dummyDefinition = {};
        dummyDefinition.guid = APINULLGuid;
        propertyDefinitions.Push (dummyDefinition);
        DBtest (!ParamHelpers::ReadProperty (APINULLGuid, params, propertyDefinitions),
                "ReadProperty : params пуст + propertyDefinitions не пуст -> всё равно false");

        // ---- Граница: params не пуст, но propertyDefinitions пуст -> false ----
        params = {};
        ParamValue pvalue = {};
        pvalue.rawName = PROPERTYNAMEPREFIX + GS::UniString ("testreadproperty") + BRACEEND;
        pvalue.name = "TestReadProperty";
        params.Put (pvalue.rawName, pvalue);
        propertyDefinitions = {};
        DBtest (!ParamHelpers::ReadProperty (APINULLGuid, params, propertyDefinitions),
                "ReadProperty : propertyDefinitions пуст -> false");

        // ---- Граница: params и propertyDefinitions не пусты, но elemGuid == APINULLGuid ----
        // ACAPI_Element_GetPropertyValues не найдёт элемент по несуществующему guid и вернёт ошибку,
        // поэтому ReadProperty должен вернуть false, а словарь params - остаться без изменений.
        params = {};
        params.Put (pvalue.rawName, pvalue);
        propertyDefinitions = {};
        propertyDefinitions.Push (dummyDefinition);
        ParamValue pvalueBefore = *params.GetPtr (pvalue.rawName);
        DBtest (!ParamHelpers::ReadProperty (APINULLGuid, params, propertyDefinitions),
                "ReadProperty : APINULLGuid -> ACAPI-ошибка -> false");
        // Ключ только что задан через Put, но полагаться на это нельзя: без
        // проверки разыменование nullptr уронило бы ArchiCAD целиком.
        DBrequire (params.GetPtr (pvalue.rawName) != nullptr, "ReadProperty : запись по ключу не исчезла");
        DBtest (params.GetPtr (pvalue.rawName)->isValid,
                pvalueBefore.isValid,
                "ReadProperty : значение в словаре не изменилось после ошибки ACAPI");

        return;
    }

    // -----------------------------------------------------------------------------
    // Тест AddProperty - добавление/слияние массива API_Property в словарь ParamDictValue
    // -----------------------------------------------------------------------------
    // Примечание: rawName для "новых" свойств строится через SetrawNameFromProperty,
    // которая для пустого pvalue (как в AddProperty) использует внешнюю GetPropertyFullName -
    // её точный результат нам не известен. Чтобы не зависеть от этого, для сценария
    // "совпадение по имени" ключ в params формируется той же функцией SetrawNameFromProperty,
    // а не догадкой о содержимом rawName - так тест остаётся корректным независимо от
    // конкретной реализации GetPropertyFullName.
    void TestAddProperty () {
        ParamDictValue params;
        GS::Array<API_Property> properties;

        // ---- Строим свойство типа Integer, которого нет в словаре ----
        API_PropertyDefinition definition = {};
        definition.guid = APINULLGuid;
        definition.name = "TestAddPropertyInt";
        definition.description = "";
        definition.collectionType = API_PropertySingleCollectionType;
        definition.valueType = API_PropertyIntegerValueType;

        API_Property property = {};
        property.definition = definition;
        property.isDefault = false;
    #ifndef ServerMainVers_2400
        property.isEvaluated = true;
    #else
        property.status = API_Property_HasValue;
    #endif
        property.value.singleVariant.variant.type = API_PropertyIntegerValueType;
        property.value.singleVariant.variant.intValue = 55;

        // ---- Граница: свойства нет в params и needAdd == false -> пропускается, возвращает false ----
        params = {};
        properties = {};
        properties.Push (property);
        DBtest (!ParamHelpers::AddProperty (params, properties, APINULLGuid),
                "AddProperty : свойство отсутствует в словаре -> false");
        DBtest (params.GetSize () == 0, "AddProperty : словарь остаётся пустым, если совпадений нет");

        // ---- Вычисляем ожидаемый rawName той же функцией, что использует AddProperty внутри ----
        ParamValue probe = {};
        ParamHelpers::SetrawNameFromProperty (probe, property);
        GS::UniString expectedRawName = probe.rawName;

        // ---- Граница: свойство есть в params (совпадение по rawName) -> значение обновляется, возвращает true ----
        params = {};
        ParamValue placeholder = {};
        placeholder.rawName = expectedRawName;
        placeholder.name = probe.name;
        params.Put (expectedRawName, placeholder);
        properties = {};
        properties.Push (property);
        DBtest (ParamHelpers::AddProperty (params, properties, APINULLGuid),
                "AddProperty : совпадение по rawName -> true");
        if (const ParamValue *stored = params.GetPtr (expectedRawName)) {
            DBtest (
                stored->val.intValue, (Int32)55, "AddProperty : значение свойства записано в словарь (intValue == 55)");
            DBtest (stored->val.type == API_PropertyIntegerValueType, "AddProperty : тип значения (Integer)");
            DBtest (stored->isValid, "AddProperty : isValid == true после успешной конвертации");
            DBtest (stored->fromGuid == APINULLGuid, "AddProperty : fromGuid == APINULLGuid (elemguid == APINULLGuid)");
        } else {
            DBtest (false, "AddProperty : значение должно быть найдено в словаре после Put");
        }

        // ---- Граница: свойство есть в params, но ConvertToParamValue не может его сконвертировать
        //      (valueType == Undefined) -> запись в словаре не меняется, AddProperty возвращает false ----
        API_PropertyDefinition undefDefinition = {};
        undefDefinition.guid = APINULLGuid;
        undefDefinition.name = "TestAddPropertyUndefined";
        undefDefinition.description = "";
        undefDefinition.collectionType = API_PropertySingleCollectionType;
        undefDefinition.valueType = API_PropertyUndefinedValueType;

        API_Property undefProperty = {};
        undefProperty.definition = undefDefinition;
    #ifndef ServerMainVers_2400
        undefProperty.isEvaluated = true;
    #else
        undefProperty.status = API_Property_HasValue;
    #endif

        ParamValue undefProbe = {};
        ParamHelpers::SetrawNameFromProperty (undefProbe, undefProperty);
        GS::UniString undefRawName = undefProbe.rawName;

        params = {};
        ParamValue undefPlaceholder = {};
        undefPlaceholder.rawName = undefRawName;
        undefPlaceholder.name = undefProbe.name;
        undefPlaceholder.isValid = false; // Значение ещё не заполнено - как это обычно бывает до первого чтения
        params.Put (undefRawName, undefPlaceholder);
        properties = {};
        properties.Push (undefProperty);
        DBtest (!ParamHelpers::AddProperty (params, properties, APINULLGuid),
                "AddProperty : Undefined valueType -> ConvertToParamValue не проходит -> false");
        if (const ParamValue *storedUndef = params.GetPtr (undefRawName)) {
            DBtest (!storedUndef->isValid, "AddProperty : запись в словаре не изменилась (осталась isValid == false)");
        }

        // ---- Граница: несколько свойств в одном вызове - одно совпадает, другое нет.
        //      Итоговый результат должен быть true (найдено хотя бы одно совпадение) ----
        params = {};
        params.Put (expectedRawName, placeholder);
        properties = {};
        properties.Push (property); // совпадёт
        API_Property noMatchProperty = {};
        API_PropertyDefinition noMatchDefinition = {};
        noMatchDefinition.guid = APINULLGuid;
        noMatchDefinition.name = "SomeCompletelyDifferentPropertyNameNoMatch";
        noMatchDefinition.description = "";
        noMatchDefinition.collectionType = API_PropertySingleCollectionType;
        noMatchDefinition.valueType = API_PropertyIntegerValueType;
        noMatchProperty.definition = noMatchDefinition;
    #ifndef ServerMainVers_2400
        noMatchProperty.isEvaluated = true;
    #else
        noMatchProperty.status = API_Property_HasValue;
    #endif
        noMatchProperty.value.singleVariant.variant.type = API_PropertyIntegerValueType;
        noMatchProperty.value.singleVariant.variant.intValue = 1;
        properties.Push (noMatchProperty); // не совпадёт (отдельное описание -> другой rawName)
        DBtest (ParamHelpers::AddProperty (params, properties, APINULLGuid),
                "AddProperty : несколько свойств, есть хотя бы одно совпадение -> true");
        DBtest (params.GetSize () == 1,
                "AddProperty : несовпавшее свойство не добавляется в словарь (needAdd == false)");

        return;
    }

    // -----------------------------------------------------------------------------
    // Тест PropertyHelpers::ToString (API_Property) - проверка строкового представления свойств
    // -----------------------------------------------------------------------------
    void TestPropertyHelpersToString () {
        API_Property property;
        FormatString fstring;

        // ---- Integer: базовое значение ----
        property = {};
    #ifndef ServerMainVers_2400
        property.isEvaluated = true;
    #else
        property.status = API_Property_HasValue;
    #endif
        property.definition.collectionType = API_PropertySingleCollectionType;
        property.definition.valueType = API_PropertyIntegerValueType;
        property.value.singleVariant.variant.type = API_PropertyIntegerValueType;
        property.value.singleVariant.variant.intValue = 42;
        GS::UniString intResult = PropertyHelpers::ToString (property);
        DBtest (intResult, GS::UniString ("42"), "ToString(Property) : Integer 42");

        // ---- Real: базовое значение ----
        property = {};
    #ifndef ServerMainVers_2400
        property.isEvaluated = true;
    #else
        property.status = API_Property_HasValue;
    #endif
        property.definition.collectionType = API_PropertySingleCollectionType;
        property.definition.valueType = API_PropertyRealValueType;
        property.definition.measureType = API_PropertyUndefinedMeasureType;
        property.value.singleVariant.variant.type = API_PropertyRealValueType;
        property.value.singleVariant.variant.doubleValue = 3.14159;
        GS::UniString realResult = PropertyHelpers::ToString (property);
        // ToString для Real использует форматирование с точностью, проверяем что это число
        DBtest (!realResult.IsEmpty (), "ToString(Property) : Real не пустая строка");
        DBtest (realResult.Contains ("3.14") || realResult.Contains ("3,14"),
                "ToString(Property) : Real содержит значение");

        // ---- Boolean: true ----
        property = {};
    #ifndef ServerMainVers_2400
        property.isEvaluated = true;
    #else
        property.status = API_Property_HasValue;
    #endif
        property.definition.collectionType = API_PropertySingleCollectionType;
        property.definition.valueType = API_PropertyBooleanValueType;
        property.value.singleVariant.variant.type = API_PropertyBooleanValueType;
        property.value.singleVariant.variant.boolValue = true;
        GS::UniString boolTrueResult = PropertyHelpers::ToString (property);
        DBtest (!boolTrueResult.IsEmpty (), "ToString(Property) : Boolean true не пустая");

        // ---- Boolean: false ----
        property.value.singleVariant.variant.boolValue = false;
        GS::UniString boolFalseResult = PropertyHelpers::ToString (property);
        DBtest (!boolFalseResult.IsEmpty (), "ToString(Property) : Boolean false не пустая");

        // ---- String: обычное значение ----
        property = {};
    #ifndef ServerMainVers_2400
        property.isEvaluated = true;
    #else
        property.status = API_Property_HasValue;
    #endif
        property.definition.collectionType = API_PropertySingleCollectionType;
        property.definition.valueType = API_PropertyStringValueType;
        property.value.singleVariant.variant.type = API_PropertyStringValueType;
        property.value.singleVariant.variant.uniStringValue = "TestString";
        GS::UniString strResult = PropertyHelpers::ToString (property);
        DBtest (strResult, GS::UniString ("TestString"), "ToString(Property) : String значение");

        // ---- String: пустая строка ----
        property.value.singleVariant.variant.uniStringValue = "";
        GS::UniString emptyStrResult = PropertyHelpers::ToString (property);
        DBtest (emptyStrResult.IsEmpty (), "ToString(Property) : пустая строка -> пустой результат");

        // ---- List: список Integer ----
        property = {};
    #ifndef ServerMainVers_2400
        property.isEvaluated = true;
    #else
        property.status = API_Property_HasValue;
    #endif
        property.definition.collectionType = API_PropertyListCollectionType;
        property.definition.valueType = API_PropertyIntegerValueType;
        API_Variant v1 = {}, v2 = {}, v3 = {};
        v1.type = API_PropertyIntegerValueType;
        v1.intValue = 1;
        v2.type = API_PropertyIntegerValueType;
        v2.intValue = 2;
        v3.type = API_PropertyIntegerValueType;
        v3.intValue = 3;
        property.value.listVariant.variants.Push (v1);
        property.value.listVariant.variants.Push (v2);
        property.value.listVariant.variants.Push (v3);
        GS::UniString listResult = PropertyHelpers::ToString (property);
        DBtest (listResult.Contains ("1"), "ToString(Property) : List содержит 1");
        DBtest (listResult.Contains ("2"), "ToString(Property) : List содержит 2");
        DBtest (listResult.Contains ("3"), "ToString(Property) : List содержит 3");
        DBtest (listResult.Contains (";"), "ToString(Property) : List содержит разделитель");

        // ---- List: список String ----
        property = {};
    #ifndef ServerMainVers_2400
        property.isEvaluated = true;
    #else
        property.status = API_Property_HasValue;
    #endif
        property.definition.collectionType = API_PropertyListCollectionType;
        property.definition.valueType = API_PropertyStringValueType;
        API_Variant sv1 = {}, sv2 = {};
        sv1.type = API_PropertyStringValueType;
        sv1.uniStringValue = "apple";
        sv2.type = API_PropertyStringValueType;
        sv2.uniStringValue = "banana";
        property.value.listVariant.variants.Push (sv1);
        property.value.listVariant.variants.Push (sv2);
        GS::UniString strListResult = PropertyHelpers::ToString (property);
        DBtest (strListResult.Contains ("apple"), "ToString(Property) : String List содержит apple");
        DBtest (strListResult.Contains ("banana"), "ToString(Property) : String List содержит banana");
        DBtest (strListResult.Contains (";"), "ToString(Property) : String List содержит разделитель");

        // ---- NotAvailable / NotEvaluated: должна вернуть пустую строку ----
        property = {};
    #ifndef ServerMainVers_2400
        property.isEvaluated = false;
    #else
        property.status = API_Property_NotAvailable;
    #endif
        property.definition.collectionType = API_PropertySingleCollectionType;
        property.definition.valueType = API_PropertyIntegerValueType;
        GS::UniString notAvailResult = PropertyHelpers::ToString (property);
        DBtest (notAvailResult.IsEmpty (), "ToString(Property) : NotAvailable -> пустая строка");

        // ---- Default value (isDefault + NotEvaluated) ----
        property = {};
    #ifndef ServerMainVers_2400
        property.isEvaluated = true;
    #else
        property.status = API_Property_NotEvaluated;
        property.isDefault = true;
    #endif
        property.definition.collectionType = API_PropertySingleCollectionType;
        property.definition.valueType = API_PropertyIntegerValueType;
        property.definition.defaultValue.basicValue.singleVariant.variant.type = API_PropertyIntegerValueType;
        property.definition.defaultValue.basicValue.singleVariant.variant.intValue = 999;
        GS::UniString defaultResult = PropertyHelpers::ToString (property);
        DBtest (defaultResult, GS::UniString ("999"), "ToString(Property) : Default value (999)");

        // ---- Default value: Real ----
        property.definition.defaultValue.basicValue.singleVariant.variant.type = API_PropertyRealValueType;
        property.definition.defaultValue.basicValue.singleVariant.variant.doubleValue = 2.5;
        property.definition.valueType = API_PropertyRealValueType;
        GS::UniString defaultRealResult = PropertyHelpers::ToString (property);
        DBtest (!defaultRealResult.IsEmpty (), "ToString(Property) : Default Real не пустая");

        return;
    }

    // -----------------------------------------------------------------------------
    // Тест GetPropertyRuleFlag — кэш признака правила SomeStuff в описании свойства.
    // Проверяет: первичный расчёт, попадание в кэш (второй вызов без перечитывания),
    // инвалидацию по изменению описания и отсутствие ложных правил у обычных свойств.
    // -----------------------------------------------------------------------------
    void TestGetPropertyRuleFlag () {
        API_PropertyDefinition definition = {};
        definition.guid = APINULLGuid;

        // ---- Описание с Sync-правилом: признак true ----
        definition.description = "Sync_from{Property:TestSource}";
        DBtest (GetPropertyRuleFlag (definition), "RuleFlag Sync_from -> true");

        // ---- Второй вызов с тем же описанием: кэш, результат тот же ----
        DBtest (GetPropertyRuleFlag (definition), "RuleFlag Sync_from cached -> true");

        // ---- Инвалидация: описание изменилось -> признак пересчитан (#159: Renum тоже правило) ----
        definition.description = "Renum_flag{Property:RenumRule; NULL}";
        DBtest (GetPropertyRuleFlag (definition), "RuleFlag changed to Renum_flag -> true");

        // ---- Обычное свойство без команд в описании ----
        definition.description = "Just a plain description";
        DBtest (!GetPropertyRuleFlag (definition), "RuleFlag plain description -> false");

        // ---- Пустое описание ----
        definition.description = "";
        DBtest (!GetPropertyRuleFlag (definition), "RuleFlag empty description -> false");

        // Spec хранится в otherCommands; Renum/Sum не должны давать ложный признак.
        definition.description = "Spec_rule{Property:TestSpec}";
        DBtest (GetPropertyRuleFlag (definition), "RuleFlag Spec_rule -> true");
        DBtest (GetPropertyRuleFlag (definition), "RuleFlag Spec_rule cached -> true");
        definition.description = "Spec_rule_v2{Property:TestSpec}";
        DBtest (GetPropertyRuleFlag (definition), "RuleFlag Spec_rule_v2 -> true");
        definition.description = "spec_rule_v3{Property:TestSpec}";
        DBtest (GetPropertyRuleFlag (definition), "RuleFlag spec_rule_v3 -> true");
        definition.description = "Spec_rule{Property:TestSpec";
        DBtest (!GetPropertyRuleFlag (definition), "RuleFlag incomplete Spec -> false");
        definition.description = "Sum{Property:TestSum}";
        DBtest (GetPropertyRuleFlag (definition), "RuleFlag Sum only -> true");
        definition.description = "";
        DBtest (!GetPropertyRuleFlag (definition), "RuleFlag cleared after Spec -> false");

        // Проверяем сохранение результата без повторного чтения определений из Archicad.
        DBprnt ("TEST", "RuleCache content checks start");
        definition.description = "Sync_from{Property:TestSource}";
        const ParsePropertyResult expected = ParsePropertyDescriptionToRules (definition.description);
        GetPropertyRuleFlag (definition);
        auto &flags = PROPERTYCACHE ().propertyRuleFlags;
        PropertyRuleFlag *entry = flags.GetPtr (definition.guid);
        DBtest (entry != nullptr && entry->parsed.hasSyncRules && entry->parsed.syncRules.GetSize () == 1 &&
                    expected.syncRules.GetSize () == 1 &&
                    entry->parsed.syncRules[0].fullCommand == expected.syncRules[0].fullCommand &&
                    entry->parsed.syncRules[0].sourceName == expected.syncRules[0].sourceName &&
                    entry->parsed.syncRules[0].isValid == expected.syncRules[0].isValid,
                "RuleCache retains Sync command");
        // Маркер только в тестовой записи: повторный разбор затёр бы его.
        if (entry != nullptr)
            entry->parsed.remainingText = "cache reuse sentinel";
        GetPropertyRuleFlag (definition);
        entry = flags.GetPtr (definition.guid);
        DBtest (entry != nullptr && entry->parsed.remainingText == "cache reuse sentinel",
                "RuleCache reuses unchanged description");
        definition.description = "Spec_rule_v2{Property:TestSpec}";
        GetPropertyRuleFlag (definition);
        entry = flags.GetPtr (definition.guid);
        DBtest (entry != nullptr && entry->parsed.syncRules.IsEmpty () && entry->parsed.hasOtherCommands &&
                    entry->parsed.otherCommands.GetSize () == 1 &&
                    entry->parsed.otherCommands[0].commandType == "Spec_rule" &&
                    entry->parsed.remainingText != "cache reuse sentinel",
                "RuleCache replaces Sync result with Spec");
        definition.description = "";
        GetPropertyRuleFlag (definition);
        entry = flags.GetPtr (definition.guid);
        DBtest (entry != nullptr && entry->parsed.syncRules.IsEmpty () && entry->parsed.otherCommands.IsEmpty () &&
                    !entry->parsed.hasSyncRules && !entry->parsed.hasOtherCommands,
                "RuleCache clears parsed commands for empty description");
        flags.Delete (definition.guid);
        return;
    }

    // -----------------------------------------------------------------------------
    // Диагностика #184/#185: почему мост отдаёт hasRule=false для всех свойств.
    // Повторяет путь BrowserPalette::GetPropertiesList на реальных элементах проекта
    // (ACAPI_Element_GetPropertyDefinitions -> ACAPI_Element_GetPropertyValues) и
    // печатает длины описаний из двух источников определения:
    //   prop.definition — то, что использовал мост;
    //   definitions[i]  — тот же источник, что в рабочем пути Sync.cpp:690/1099.
    // Без этого нельзя отличить «описание не приходит от API» от «в описании нет правил».
    // Только DBprnt: это измерение, а не проверка — провалов теста оно не создаёт.
    // -----------------------------------------------------------------------------
    void TestPropertyRuleFlagOnProjectElements () {
        GS::Array<API_Guid> elements;
        GSErrCode err = ACAPI_Element_GetElemList (API_WallID, &elements);
        const Int32 wallListError = (Int32)err;
        if (err != NoError || elements.IsEmpty ()) {
            err = ACAPI_Element_GetElemList (API_SlabID, &elements);
        }
        DBprnt ("TEST",
                GS::UniString ("RuleFlagProj list: elements=") + GS::ValueToUniString ((Int32)elements.GetSize ()) +
                    GS::UniString (" wallCode=") + GS::ValueToUniString (wallListError) + GS::UniString (" slabCode=") +
                    GS::ValueToUniString ((Int32)err));
        if (err != NoError || elements.IsEmpty ())
            return;

        // #184/#185: добавляем один объект — правила координат/углов живут на объектах.
        GS::Array<API_Guid> objectElements;
        if (ACAPI_Element_GetElemList (API_ObjectID, &objectElements) == NoError && !objectElements.IsEmpty ())
            elements.Push (objectElements[0]);
        const USize elementLimit = GS::Min (elements.GetSize (), (USize)3);
        USize descriptionsDumped = 0; // общий лимит печати описаний на все элементы
        for (USize elementIndex = 0; elementIndex < elementLimit; ++elementIndex) {
            const API_Guid elemGuid = elements[elementIndex];
            GS::Array<API_PropertyDefinition> definitions;
            err =
                ACAPI_Element_GetPropertyDefinitions (elemGuid, API_PropertyDefinitionFilter_UserDefined, definitions);
            if (err != NoError || definitions.IsEmpty ()) {
                DBprnt ("TEST",
                        GS::UniString ("RuleFlagProj definitions: code=") + GS::ValueToUniString ((Int32)err) +
                            GS::UniString (" count=") + GS::ValueToUniString ((Int32)definitions.GetSize ()));
                continue;
            }

            USize definitionsWithDescription = 0;
            for (const API_PropertyDefinition &definition : definitions) {
                if (!definition.description.IsEmpty ())
                    ++definitionsWithDescription;
            }

            GS::Array<API_Property> properties;
            err = ACAPI_Element_GetPropertyValues (elemGuid, definitions, properties);

            USize propertiesWithDescription = 0;
            USize rulesFromPropertyDefinition = 0;
            USize rulesFromArrayDefinition = 0;
            for (const API_Property &prop : properties) {
                if (!prop.definition.description.IsEmpty ())
                    ++propertiesWithDescription;

                const bool ruleFromPropertyDefinition = GetPropertyRuleFlag (prop.definition);
                if (ruleFromPropertyDefinition)
                    ++rulesFromPropertyDefinition;

                bool ruleFromArrayDefinition = false;
                USize arrayDescriptionLength = 0;
                for (const API_PropertyDefinition &definition : definitions) {
                    if (definition.guid != prop.definition.guid)
                        continue;
                    arrayDescriptionLength = definition.description.GetLength ();
                    ruleFromArrayDefinition = !definition.description.IsEmpty () && GetPropertyRuleFlag (definition);
                    break;
                }
                if (ruleFromArrayDefinition)
                    ++rulesFromArrayDefinition;

                // Печатаем сами описания первого элемента: длины у обоих источников
                // совпадают, поэтому отличить «описание без правила» от «правило другого
                // формата» можно только по тексту (#184/#185).
                if (descriptionsDumped < 24 && !prop.definition.description.IsEmpty ()) {
                    ++descriptionsDumped;
                    const USize descriptionLimit = 160;
                    const GS::UniString descriptionText =
                        prop.definition.description.GetLength () > descriptionLimit
                            ? prop.definition.description.GetSubstring (0, descriptionLimit) + GS::UniString ("...")
                            : prop.definition.description;
                    DBprnt ("TEST",
                            GS::UniString ("RuleFlagProj desc ") + prop.definition.name + GS::UniString (" len=") +
                                GS::ValueToUniString ((Int32)prop.definition.description.GetLength ()) +
                                GS::UniString (" dArray=") + GS::ValueToUniString ((Int32)arrayDescriptionLength) +
                                GS::UniString (" rule=") + GS::ValueToUniString (ruleFromPropertyDefinition) +
                                GS::UniString (" [") + descriptionText + GS::UniString ("]"));
                }
            }

            DBprnt ("TEST",
                    GS::UniString ("RuleFlagProj elem=") + APIGuidToString (elemGuid) + GS::UniString (" defs=") +
                        GS::ValueToUniString ((Int32)definitions.GetSize ()) + GS::UniString (" defsWithDesc=") +
                        GS::ValueToUniString ((Int32)definitionsWithDescription) + GS::UniString (" props=") +
                        GS::ValueToUniString ((Int32)properties.GetSize ()) + GS::UniString (" propsWithDesc=") +
                        GS::ValueToUniString ((Int32)propertiesWithDescription) + GS::UniString (" rulesFromPropDef=") +
                        GS::ValueToUniString ((Int32)rulesFromPropertyDefinition) +
                        GS::UniString (" rulesFromArrayDef=") + GS::ValueToUniString ((Int32)rulesFromArrayDefinition));
        }
        return;
    }

} // namespace TestFunc
#endif
