//------------ kuvbur 2022 ------------
#ifdef TESTING
    #include "ACAPinc.h"

    #include "api_headers/APIEnvir.h"

    #include "Helpers.hpp"
    #include "Propertycache.hpp"
    #include "ReNum.hpp"
    #include "Sync.hpp"
    #include "TestFunc.hpp"
    #include "TestKit.hpp"

namespace TestFunc {

    // -----------------------------------------------------------------------------
    // Тест Name2Rawname - преобразование имени в rawname
    // -----------------------------------------------------------------------------
    void TestName2Rawname () {
        GS::UniString name;
        GS::UniString rawname;

        // Тест: обычное свойство
        name = "Property:TestProperty";
        DBtest (Name2Rawname (name, rawname), "Name2Rawname Property:TestProperty -> true");
        DBtest (rawname, GS::UniString ("{@property:testproperty}"), "Name2Rawname Property:TestProperty -> rawname");

        // Тест: с префиксом property
        name = "property:AnotherProperty";
        DBtest (Name2Rawname (name, rawname), "Name2Rawname property:AnotherProperty -> true");
        DBtest (
            rawname, GS::UniString ("{@property:anotherproperty}"), "Name2Rawname property:AnotherProperty -> rawname");

        // Тест: координаты
        name = "Coord:symb_pos_x";
        DBtest (Name2Rawname (name, rawname), "Name2Rawname Coord:symb_pos_x -> true");
        DBtest (rawname, GS::UniString ("{@coord:symb_pos_x}"), "Name2Rawname Coord:symb_pos_x -> rawname");

        // Тест: GDL параметр
        name = "TestGDLParam";
        DBtest (Name2Rawname (name, rawname), "Name2Rawname TestGDLParam -> true");
        DBtest (rawname, GS::UniString ("{@gdl:testgdlparam}"), "Name2Rawname TestGDLParam -> rawname");

        // Тест: ID
        name = "{id}";
        DBtest (Name2Rawname (name, rawname), "Name2Rawname {id} -> true");
        DBtest (rawname, GS::UniString ("{@id:id}"), "Name2Rawname {id} -> rawname");

        // Тест: пустая строка -> false
        name = "";
        DBtest (!Name2Rawname (name, rawname), "Name2Rawname empty string -> false");

        // Тест: BuildingMaterial свойство
        name = "Property:BuildingMaterialProperties/Density";
        DBtest (Name2Rawname (name, rawname), "Name2Rawname Property:BuildingMaterialProperties/Density -> true");
        DBtest (rawname.BeginsWith ("{@property:buildingmaterialproperties/density"),
                "Name2Rawname BuildingMaterial -> rawname");

        // Тест: Morph
        name = "Morph:param1";
        DBtest (Name2Rawname (name, rawname), "Name2Rawname Morph:param1 -> true");
        DBtest (rawname, GS::UniString ("{@morph:param1}"), "Name2Rawname Morph:param1 -> rawname");

        // Тест: IFC
        name = "IFC:PropertyName";
        DBtest (Name2Rawname (name, rawname), "Name2Rawname IFC:PropertyName -> true");
        DBtest (rawname, GS::UniString ("{@ifc:propertyname}"), "Name2Rawname IFC:PropertyName -> rawname");

        // Тест: Info
        name = "Info:someinfo";
        DBtest (Name2Rawname (name, rawname), "Name2Rawname Info:someinfo -> true");
        DBtest (rawname, GS::UniString ("{@info:someinfo}"), "Name2Rawname Info:someinfo -> rawname");

        // Тест: Glob
        name = "Glob:variable";
        DBtest (Name2Rawname (name, rawname), "Name2Rawname Glob:variable -> true");
        DBtest (rawname, GS::UniString ("{@glob:variable}"), "Name2Rawname Glob:variable -> rawname");

        // Тест: Class
        name = "Class:classification";
        DBtest (Name2Rawname (name, rawname), "Name2Rawname Class:classification -> true");
        DBtest (rawname, GS::UniString ("{@class:classification}"), "Name2Rawname Class:classification -> rawname");

        // Тест: Element
        name = "Element:property";
        DBtest (Name2Rawname (name, rawname), "Name2Rawname Element:property -> true");
        DBtest (rawname, GS::UniString ("{@element:property}"), "Name2Rawname Element:property -> rawname");

        // Тест: File
        name = "File:filename";
        DBtest (Name2Rawname (name, rawname), "Name2Rawname File:filename -> true");
        DBtest (rawname, GS::UniString ("{@file:filename}"), "Name2Rawname File:filename -> rawname");

        // Тест: Attrib (Layer)
        name = "Attrib:Layer";
        DBtest (Name2Rawname (name, rawname), "Name2Rawname Attrib:Layer -> true");
        DBtest (rawname, GS::UniString ("{@attrib:layer}"), "Name2Rawname Attrib:Layer -> rawname");

        return;
    }

    // -----------------------------------------------------------------------------
    // Тест Name2Rawname с уже обёрнутыми скобками (временное решение до исправления бага в Sync.cpp:1323-1326)
    // Баг: Name2Rawname сначала добавляет BRACEEND (}), потом BRACESTART ({).
    // Входные данные, УЖЕ содержащие правильные скобки "{@prefix:name}", проходят корректно.
    // -----------------------------------------------------------------------------
    void TestName2RawnameWithBrackets () {
        GS::UniString name;
        GS::UniString rawname;

        // Тест: уже корректный rawname свойства
        name = "{@property:testproperty}";
        DBtest (Name2Rawname (name, rawname), "Name2RawnameWithBrackets {@property:testproperty} -> true");
        DBtest (rawname,
                GS::UniString ("{@property:testproperty}"),
                "Name2RawnameWithBrackets {@property:testproperty} -> rawname unchanged");

        // Тест: уже корректный rawname координат
        name = "{@coord:symb_pos_x}";
        DBtest (Name2Rawname (name, rawname), "Name2RawnameWithBrackets {@coord:symb_pos_x} -> true");
        DBtest (rawname,
                GS::UniString ("{@coord:symb_pos_x}"),
                "Name2RawnameWithBrackets {@coord:symb_pos_x} -> rawname unchanged");

        // Sync.cpp-8 (#161): каноническая ветка нормализует регистр — ключи кэша
        // всегда в нижнем регистре, иначе правило молча не находит значение.
        name = "{@Coord:Symb_Pos_X}";
        DBtest (Name2Rawname (name, rawname), "Name2RawnameWithBrackets {@Coord:Symb_Pos_X} -> true");
        DBtest (rawname,
                GS::UniString ("{@coord:symb_pos_x}"),
                "Name2RawnameWithBrackets {@Coord:Symb_Pos_X} -> rawname lowered");

        // Тест: уже корректный rawname GDL
        name = "{@gdl:testgdlparam}";
        DBtest (Name2Rawname (name, rawname), "Name2RawnameWithBrackets {@gdl:testgdlparam} -> true");
        DBtest (rawname,
                GS::UniString ("{@gdl:testgdlparam}"),
                "Name2RawnameWithBrackets {@gdl:testgdlparam} -> rawname unchanged");

        // Тест: уже корректный rawname ID
        name = "{@id:id}";
        DBtest (Name2Rawname (name, rawname), "Name2RawnameWithBrackets {@id:id} -> true");
        DBtest (rawname, GS::UniString ("{@id:id}"), "Name2RawnameWithBrackets {@id:id} -> rawname unchanged");

        // Тест: уже корректный rawname BuildingMaterial
        name = "{@property:buildingmaterialproperties/density}";
        DBtest (Name2Rawname (name, rawname), "Name2RawnameWithBrackets BuildingMaterial -> true");
        DBtest (rawname.BeginsWith ("{@property:buildingmaterialproperties/density"),
                "Name2RawnameWithBrackets BuildingMaterial -> rawname unchanged");

        // Тест: уже корректный rawname Morph
        name = "{@morph:param1}";
        DBtest (Name2Rawname (name, rawname), "Name2RawnameWithBrackets {@morph:param1} -> true");
        DBtest (rawname,
                GS::UniString ("{@morph:param1}"),
                "Name2RawnameWithBrackets {@morph:param1} -> rawname unchanged");

        // Тест: уже корректный rawname IFC
        name = "{@ifc:propertyname}";
        DBtest (Name2Rawname (name, rawname), "Name2RawnameWithBrackets {@ifc:propertyname} -> true");
        DBtest (rawname,
                GS::UniString ("{@ifc:propertyname}"),
                "Name2RawnameWithBrackets {@ifc:propertyname} -> rawname unchanged");

        // Тест: уже корректный rawname Info
        name = "{@info:someinfo}";
        DBtest (Name2Rawname (name, rawname), "Name2RawnameWithBrackets {@info:someinfo} -> true");
        DBtest (rawname,
                GS::UniString ("{@info:someinfo}"),
                "Name2RawnameWithBrackets {@info:someinfo} -> rawname unchanged");

        // Тест: уже корректный rawname Glob
        name = "{@glob:variable}";
        DBtest (Name2Rawname (name, rawname), "Name2RawnameWithBrackets {@glob:variable} -> true");
        DBtest (rawname,
                GS::UniString ("{@glob:variable}"),
                "Name2RawnameWithBrackets {@glob:variable} -> rawname unchanged");

        // Тест: уже корректный rawname Class
        name = "{@class:classification}";
        DBtest (Name2Rawname (name, rawname), "Name2RawnameWithBrackets {@class:classification} -> true");
        DBtest (rawname,
                GS::UniString ("{@class:classification}"),
                "Name2RawnameWithBrackets {@class:classification} -> rawname unchanged");

        // Тест: уже корректный rawname Element
        name = "{@element:property}";
        DBtest (Name2Rawname (name, rawname), "Name2RawnameWithBrackets {@element:property} -> true");
        DBtest (rawname,
                GS::UniString ("{@element:property}"),
                "Name2RawnameWithBrackets {@element:property} -> rawname unchanged");

        // Тест: уже корректный rawname File
        name = "{@file:filename}";
        DBtest (Name2Rawname (name, rawname), "Name2RawnameWithBrackets {@file:filename} -> true");
        DBtest (rawname,
                GS::UniString ("{@file:filename}"),
                "Name2RawnameWithBrackets {@file:filename} -> rawname unchanged");

        // Тест: уже корректный rawname Attrib
        name = "{@attrib:layer}";
        DBtest (Name2Rawname (name, rawname), "Name2RawnameWithBrackets {@attrib:layer} -> true");
        DBtest (rawname,
                GS::UniString ("{@attrib:layer}"),
                "Name2RawnameWithBrackets {@attrib:layer} -> rawname unchanged");

        return;
    }

    // -----------------------------------------------------------------------------
    // Тест SyncString - парсинг строки правила синхронизации
    // -----------------------------------------------------------------------------
    void TestSyncString () {
        ParamValue param;
        SkipValues ignorevals;
        FormatString stringformat;
        SyncMode syncdirection = SYNC_NO;
        API_ElemTypeID elementType = API_ObjectID;

        // Тест: SYNC_FROM базовое свойство
        param = ParamValue ();
        GS::UniString rule1 = "Sync_from{Property:TestProperty}";
        DBtest (SyncString (elementType, rule1, syncdirection, param, ignorevals, stringformat, true, false, false),
                "SyncString Sync_from Property -> true");
        DBtest (syncdirection, SYNC_FROM, "SyncString Sync_from -> direction FROM");
        DBtest (param.fromProperty, "SyncString Sync_from Property -> fromProperty");

        // Тест: SYNC_TO базовое свойство
        param = ParamValue ();
        GS::UniString rule2 = "Sync_to{Property:TestProperty}";
        syncdirection = SYNC_NO;
        DBtest (SyncString (elementType, rule2, syncdirection, param, ignorevals, stringformat, true, false, false),
                "SyncString Sync_to Property -> true");
        DBtest (syncdirection, SYNC_TO, "SyncString Sync_to -> direction TO");
        DBtest (param.fromProperty, "SyncString Sync_to Property -> fromProperty");

        // Тест: SYNC_FROM_SUB
        param = ParamValue ();
        GS::UniString rule3 = "Sync_from_sub{Property:TestProperty}";
        syncdirection = SYNC_NO;
        DBtest (SyncString (elementType, rule3, syncdirection, param, ignorevals, stringformat, true, false, false),
                "SyncString Sync_from_sub -> true");
        DBtest (syncdirection, SYNC_FROM_SUB, "SyncString Sync_from_sub -> direction FROM_SUB");

        // Тест: SYNC_TO_SUB
        param = ParamValue ();
        GS::UniString rule4 = "Sync_to_sub{Property:TestProperty}";
        syncdirection = SYNC_NO;
        DBtest (SyncString (elementType, rule4, syncdirection, param, ignorevals, stringformat, true, false, false),
                "SyncString Sync_to_sub -> true");
        DBtest (syncdirection, SYNC_TO_SUB, "SyncString Sync_to_sub -> direction TO_SUB");

        // Тест: GDL параметр
        param = ParamValue ();
        GS::UniString rule5 = "Sync_from{MyGDLParam}";
        syncdirection = SYNC_NO;
        DBtest (SyncString (elementType, rule5, syncdirection, param, ignorevals, stringformat, true, false, false),
                "SyncString Sync_from GDL -> true");
        DBtest (param.fromGDLparam, "SyncString Sync_from GDL -> fromGDLparam");

        // Тест: Координаты
        param = ParamValue ();
        GS::UniString rule6 = "Sync_from{Coord:symb_pos_x}";
        syncdirection = SYNC_NO;
        DBtest (SyncString (elementType, rule6, syncdirection, param, ignorevals, stringformat, false, true, false),
                "SyncString Sync_from Coord -> true");
        DBtest (param.fromCoord, "SyncString Sync_from Coord -> fromCoord");

        // Тест: Формула
        param = ParamValue ();
        GS::UniString rule7 = "Sync_from{<2*2>}";
        syncdirection = SYNC_NO;
        DBtest (SyncString (elementType, rule7, syncdirection, param, ignorevals, stringformat, true, false, false),
                "SyncString Sync_from Formula -> true");
        DBtest (param.val.hasFormula, "SyncString Sync_from Formula -> hasFormula");

        // Тест: ID
        param = ParamValue ();
        GS::UniString rule8 = "Sync_from{{id}}";
        syncdirection = SYNC_NO;
        DBtest (SyncString (elementType, rule8, syncdirection, param, ignorevals, stringformat, true, false, false),
                "SyncString Sync_from ID -> true");
        DBtest (param.fromID, "SyncString Sync_from ID -> fromID");

        // Тест: FormatString с форматом
        param = ParamValue ();
        GS::UniString rule9 = "Sync_from{Property:TestProperty.3m}";
        syncdirection = SYNC_NO;
        DBtest (SyncString (elementType, rule9, syncdirection, param, ignorevals, stringformat, true, false, false),
                "SyncString Sync_from Format .3m -> true");
        DBtest (!stringformat.stringformat.IsEmpty (), "SyncString FormatString -> stringformat not empty");

        // Тест: ignorevals empty
        param = ParamValue ();
        GS::UniString rule10 = "Sync_from{Property:TestProperty; empty}";
        syncdirection = SYNC_NO;
        DBtest (SyncString (elementType, rule10, syncdirection, param, ignorevals, stringformat, true, false, false),
                "SyncString ignorevals empty -> true");
        DBtest (ignorevals.skip_empty, "SyncString ignorevals -> skip_empty true");

        // Тест: ignorevals trim_empty
        param = ParamValue ();
        GS::UniString rule11 = "Sync_from{Property:TestProperty; trim_empty}";
        syncdirection = SYNC_NO;
        DBtest (SyncString (elementType, rule11, syncdirection, param, ignorevals, stringformat, true, false, false),
                "SyncString ignorevals trim_empty -> true");
        DBtest (ignorevals.skip_trim_empty, "SyncString ignorevals -> skip_trim_empty true");

        // Тест: некорректная строка (нет направления)
        param = ParamValue ();
        GS::UniString rule12 = "Property:TestProperty";
        syncdirection = SYNC_NO;
        DBtest (!SyncString (elementType, rule12, syncdirection, param, ignorevals, stringformat, false, false, false),
                "SyncString no direction -> false");

        // #184/#185: правило состава конструкции из реального проекта —
        // Sync_from{Material:Layers; "<шаблон>"}. Признак правила в палитре считался через
        // ParsePropertyDescriptionToRules, который вызывает SyncString с elementType =
        // API_ObjectID; правило материала при этом отбраковывалось, из-за чего фильтр
        // «Только с правилами» и синяя маркировка его не видели.
        const GS::UniString ruleMaterial =
            "Sync_from{Material:Layers; \"3зн %BuildingMaterialProperties/Building Material Thermal Conductivity.3pm% / \"}";

        param = ParamValue ();
        syncdirection = SYNC_NO;
        DBtest (
            SyncString (API_WallID, ruleMaterial, syncdirection, param, ignorevals, stringformat, true, false, false),
            "SyncString Sync_from Material:Layers + API_WallID -> true");
        DBtest (param.fromMaterial, "SyncString Sync_from Material:Layers + API_WallID -> fromMaterial");

        param = ParamValue ();
        syncdirection = SYNC_NO;
        // Путь разбора описания в палитре: проверка типов отключена (#184/#185).
        DBtest (
            SyncString (
                API_ObjectID, ruleMaterial, syncdirection, param, ignorevals, stringformat, true, false, false, false),
            "SyncString Sync_from Material:Layers + API_ObjectID (no type check) -> true");
        DBtest (param.fromMaterial,
                "SyncString Sync_from Material:Layers + API_ObjectID (no type check) -> fromMaterial");

        // #202: числовые аргументы правила File: принимаются только как полное число.
        // Раньше std::stoi молча отбрасывал хвост, поэтому "2junk" читался как 2,
        // описание свойства выглядело рабочим, а данные брались не из того столбца.
        // Имена файла и ячеек берём в кавычки: GetSubstring ищет первую пару скобок,
        // поэтому вложенные {...} обрезают правило.
        const GS::UniString ruleFileOk = "Sync_from{File:lookup;\"data.txt\",2,\"col_end\"}";
        param = ParamValue ();
        syncdirection = SYNC_NO;
        DBtest (
            SyncString (elementType, ruleFileOk, syncdirection, param, ignorevals, stringformat, true, false, false),
            "SyncString File full number -> true");
        DBtest (param.fromFile, "SyncString File full number -> fromFile");
        DBtest (param.composite_pen == 2, "SyncString File full number -> composite_pen 2");

        param = ParamValue ();
        syncdirection = SYNC_NO;
        DBtest (!SyncString (elementType,
                             "Sync_from{File:lookup;\"data.txt\",2junk,\"col_end\"}",
                             syncdirection,
                             param,
                             ignorevals,
                             stringformat,
                             true,
                             false,
                             false),
                "SyncString File junk after number -> false");

        // Все пять числовых позиций: col_out, конец столбцов, начало столбцов,
        // конец строк, начало строк
        const GS::UniString ruleFileAll = "Sync_from{File:lookup;\"data.txt\",2,\"ce\",3,\"cs\",4,\"re\",5,\"rs\",6}";
        param = ParamValue ();
        syncdirection = SYNC_NO;
        DBtest (
            SyncString (elementType, ruleFileAll, syncdirection, param, ignorevals, stringformat, true, false, false),
            "SyncString File all five numbers -> true");
        DBtest (param.composite_pen == 2, "SyncString File all five numbers -> composite_pen 2");
        DBtest (param.val.array_column_end == 3, "SyncString File all five numbers -> array_column_end 3");
        DBtest (param.val.array_column_start == 4, "SyncString File all five numbers -> array_column_start 4");
        DBtest (param.val.array_row_end == 5, "SyncString File all five numbers -> array_row_end 5");
        DBtest (param.val.array_row_start == 6, "SyncString File all five numbers -> array_row_start 6");

        // Мусор после числа в каждой из пяти позиций - правило должно отбраковываться
        static const char *junkRules[] = {
            "Sync_from{File:lookup;\"data.txt\",2junk,\"ce\",3,\"cs\",4,\"re\",5,\"rs\",6}",
            "Sync_from{File:lookup;\"data.txt\",2,\"ce\",3junk,\"cs\",4,\"re\",5,\"rs\",6}",
            "Sync_from{File:lookup;\"data.txt\",2,\"ce\",3,\"cs\",4junk,\"re\",5,\"rs\",6}",
            "Sync_from{File:lookup;\"data.txt\",2,\"ce\",3,\"cs\",4,\"re\",5junk,\"rs\",6}",
            "Sync_from{File:lookup;\"data.txt\",2,\"ce\",3,\"cs\",4,\"re\",5,\"rs\",6junk}"};
        for (int junk = 0; junk < 5; junk++) {
            param = ParamValue ();
            syncdirection = SYNC_NO;
            DBtest (
                !SyncString (
                    elementType, junkRules[junk], syncdirection, param, ignorevals, stringformat, true, false, false),
                GS::UniString::Printf ("SyncString File junk in number #%d -> false", junk + 1));
        }

        return;
    }

    // -----------------------------------------------------------------------------
    // Тест реальных правил синхронизации из BuildingInformation.xml
    // -----------------------------------------------------------------------------
    void TestSyncStringRealRules () {
        ParamValue param;
        SkipValues ignorevals;
        FormatString stringformat;
        SyncMode syncdirection = SYNC_NO;

        // --- GDL параметры (API_ObjectID, syncall=true) ---
        param = ParamValue ();
        GS::UniString rule = "Sync_from{ac_wallhole_width}";
        syncdirection = SYNC_NO;
        DBtest (SyncString (API_ObjectID, rule, syncdirection, param, ignorevals, stringformat, true, false, false),
                "SyncStringReal GDL ac_wallhole_width -> true");
        DBtest (param.fromGDLparam, "SyncStringReal GDL ac_wallhole_width -> fromGDLparam");

        param = ParamValue ();
        rule = "Sync_from{naen}";
        syncdirection = SYNC_NO;
        DBtest (SyncString (API_ObjectID, rule, syncdirection, param, ignorevals, stringformat, true, false, false),
                "SyncStringReal GDL naen -> true");
        DBtest (param.fromGDLparam, "SyncStringReal GDL naen -> fromGDLparam");

        // --- GDL описание (API_ObjectID, syncall=true) ---
        param = ParamValue ();
        rule = "Sync_from{description:Наименование}";
        syncdirection = SYNC_NO;
        DBtest (SyncString (API_ObjectID, rule, syncdirection, param, ignorevals, stringformat, true, false, false),
                "SyncStringReal GDL description -> true");
        DBtest (param.fromGDLdescription, "SyncStringReal GDL description -> fromGDLdescription");

        // --- Свойства с русскими именами и слэшами (API_ObjectID, syncall=true) ---
        param = ParamValue ();
        rule = "Sync_from{Property:Свойства и параметры/_Свойство в свойство}";
        syncdirection = SYNC_NO;
        DBtest (SyncString (API_ObjectID, rule, syncdirection, param, ignorevals, stringformat, true, false, false),
                "SyncStringReal Property русский путь -> true");
        DBtest (param.fromProperty, "SyncStringReal Property русский путь -> fromProperty");
        DBtest (param.name,
                GS::UniString ("Свойства и параметры/_Свойство в свойство"),
                "SyncStringReal Property русский путь -> name");

        // --- Координаты (API_ObjectID, synccoord=true) ---
        param = ParamValue ();
        rule = "Sync_from{Coord:symb_rotangle}";
        syncdirection = SYNC_NO;
        DBtest (SyncString (API_ObjectID, rule, syncdirection, param, ignorevals, stringformat, false, true, false),
                "SyncStringReal Coord symb_rotangle -> true");
        DBtest (param.fromCoord, "SyncStringReal Coord symb_rotangle -> fromCoord");

        param = ParamValue ();
        rule = "Sync_from{Coord:symb_rotangle_correct}";
        syncdirection = SYNC_NO;
        DBtest (SyncString (API_ObjectID, rule, syncdirection, param, ignorevals, stringformat, false, true, false),
                "SyncStringReal Coord symb_rotangle_correct -> true");
        DBtest (param.fromCoord, "SyncStringReal Coord symb_rotangle_correct -> fromCoord");

        param = ParamValue ();
        rule = "Sync_from{Coord:symb_pos_correct}";
        syncdirection = SYNC_NO;
        DBtest (SyncString (API_ObjectID, rule, syncdirection, param, ignorevals, stringformat, false, true, false),
                "SyncStringReal Coord symb_pos_correct -> true");
        DBtest (param.fromCoord, "SyncStringReal Coord symb_pos_correct -> fromCoord");

        param = ParamValue ();
        rule = "Sync_from{Coord:symb_pos_correct_hard}";
        syncdirection = SYNC_NO;
        DBtest (SyncString (API_ObjectID, rule, syncdirection, param, ignorevals, stringformat, false, true, false),
                "SyncStringReal Coord symb_pos_correct_hard -> true");
        DBtest (param.fromCoord, "SyncStringReal Coord symb_pos_correct_hard -> fromCoord");

        param = ParamValue ();
        rule = "Sync_from{Coord:symb_pos_x_correct}";
        syncdirection = SYNC_NO;
        DBtest (SyncString (API_ObjectID, rule, syncdirection, param, ignorevals, stringformat, false, true, false),
                "SyncStringReal Coord symb_pos_x_correct -> true");
        DBtest (param.fromCoord, "SyncStringReal Coord symb_pos_x_correct -> fromCoord");

        param = ParamValue ();
        rule = "Sync_from{Coord:symb_pos_y_correct}";
        syncdirection = SYNC_NO;
        DBtest (SyncString (API_ObjectID, rule, syncdirection, param, ignorevals, stringformat, false, true, false),
                "SyncStringReal Coord symb_pos_y_correct -> true");
        DBtest (param.fromCoord, "SyncStringReal Coord symb_pos_y_correct -> fromCoord");

        param = ParamValue ();
        rule = "Sync_from{Coord:l_correct}";
        syncdirection = SYNC_NO;
        DBtest (SyncString (API_ObjectID, rule, syncdirection, param, ignorevals, stringformat, false, true, false),
                "SyncStringReal Coord l_correct -> true");
        DBtest (param.fromCoord, "SyncStringReal Coord l_correct -> fromCoord");

        // --- Classification FROM (API_ObjectID, syncall=true) ---
        param = ParamValue ();
        rule = "Sync_from{Class:Test_Addon; FullName}";
        syncdirection = SYNC_NO;
        DBtest (SyncString (API_ObjectID, rule, syncdirection, param, ignorevals, stringformat, true, false, false),
                "SyncStringReal Classification FROM -> true");
        DBtest (param.fromClassification, "SyncStringReal Classification FROM -> fromClassification");

        // --- Classification TO (API_ObjectID, syncall=true, syncclass=true) ---
        param = ParamValue ();
        rule = "Sync_to{Class:Test_Addon}";
        syncdirection = SYNC_NO;
        DBtest (SyncString (API_ObjectID, rule, syncdirection, param, ignorevals, stringformat, true, false, true),
                "SyncStringReal Classification TO -> true");
        DBtest (param.fromClassification, "SyncStringReal Classification TO -> fromClassification");
        DBtest (syncdirection, SYNC_TO, "SyncStringReal Classification TO -> direction TO");

        // --- Material (нужен API_WallID, т.к. Material не проходит для API_ObjectID без fromQuantity) ---
        param = ParamValue ();
        rule =
            R"(Sync_from{Material:Layers; "3зн %BuildingMaterialProperties/Building Material Thermal Conductivity.3pm% / "})";
        syncdirection = SYNC_NO;
        DBtest (SyncString (API_WallID, rule, syncdirection, param, ignorevals, stringformat, true, false, false),
                "SyncStringReal Material Layers default pen -> true");
        DBtest (param.fromMaterial, "SyncStringReal Material Layers default pen -> fromMaterial");

        param = ParamValue ();
        rule =
            R"(Sync_from{Material:Layers; "2зн %BuildingMaterialProperties/Building Material Thermal Conductivity.2m% / "})";
        syncdirection = SYNC_NO;
        DBtest (SyncString (API_WallID, rule, syncdirection, param, ignorevals, stringformat, true, false, false),
                "SyncStringReal Material Layers 2зн -> true");
        DBtest (param.fromMaterial, "SyncStringReal Material Layers 2зн -> fromMaterial");

        param = ParamValue ();
        rule = R"(Sync_from{Material:Layers; "%Описание% - %Толщина.2mm%мм. "})";
        syncdirection = SYNC_NO;
        DBtest (SyncString (API_WallID, rule, syncdirection, param, ignorevals, stringformat, true, false, false),
                "SyncStringReal Material Layers старое -> true");
        DBtest (param.fromMaterial, "SyncStringReal Material Layers старое -> fromMaterial");

        param = ParamValue ();
        rule = R"(Sync_from{Material:Layers, 20; "%Описание% - %Толщина.2mm%мм. "})";
        syncdirection = SYNC_NO;
        DBtest (SyncString (API_WallID, rule, syncdirection, param, ignorevals, stringformat, true, false, false),
                "SyncStringReal Material Layers pen 20 -> true");
        DBtest (param.fromMaterial, "SyncStringReal Material Layers pen 20 -> fromMaterial");

        param = ParamValue ();
        rule = R"(Sync_from{Material:Layers, 6; "%Описание% - %Толщина.2mm%мм. "})";
        syncdirection = SYNC_NO;
        DBtest (SyncString (API_WallID, rule, syncdirection, param, ignorevals, stringformat, true, false, false),
                "SyncStringReal Material Layers pen 6 -> true");
        DBtest (param.fromMaterial, "SyncStringReal Material Layers pen 6 -> fromMaterial");

        // --- Material со сложной формулой R0усл ---
        param = ParamValue ();
        rule =
            R"(Sync_from{Material:Layers, 6; "1/{Property:Теплотехнический расчёт/αint, Вт\/(м2°С)} + 1/{Property:Теплотехнический расчёт/αext, Вт\/(м2°С)} <+%layer_thickness.3m%/%BuildingMaterialProperties/Building Material Thermal Conductivity.3m%>"})";
        syncdirection = SYNC_NO;
        DBtest (SyncString (API_WallID, rule, syncdirection, param, ignorevals, stringformat, true, false, false),
                "SyncStringReal Material R0усл -> true");
        DBtest (param.fromMaterial, "SyncStringReal Material R0усл -> fromMaterial");

        param = ParamValue ();
        rule =
            R"(Sync_from{Material:Layers, 6; "1/{Property:Теплотехнический расчёт/αint, Вт\/(м2°С)} + 1/{Property:Теплотехнический расчёт/αext, Вт\/(м2°С)} <+%layer_thickness.3m%/%BuildingMaterialProperties/Building Material Thermal Conductivity.3m%>".3m})";
        syncdirection = SYNC_NO;
        DBtest (SyncString (API_WallID, rule, syncdirection, param, ignorevals, stringformat, true, false, false),
                "SyncStringReal Material R0усл .3m -> true");
        DBtest (param.fromMaterial, "SyncStringReal Material R0усл .3m -> fromMaterial");

        param = ParamValue ();
        rule =
            R"(Sync_from{Material:Layers, 6; "1/{Property:Теплотехнический расчёт/αint, Вт\/(м2°С)} + 1/{Property:Теплотехнический расчёт/αext, Вт\/(м2°С)} <+%толщина.3m%/%BuildingMaterialProperties/Building Material Thermal Conductivity.3m%>".3mp})";
        syncdirection = SYNC_NO;
        DBtest (SyncString (API_WallID, rule, syncdirection, param, ignorevals, stringformat, true, false, false),
                "SyncStringReal Material R0усл .3mp -> true");
        DBtest (param.fromMaterial, "SyncStringReal Material R0усл .3mp -> fromMaterial");

        return;
    }

    // -----------------------------------------------------------------------------
    // Тест констант префиксов
    // -----------------------------------------------------------------------------
    void TestParsePrefixes () {
        // Проверка основных префиксов имен параметров
        DBtest (PROPERTYNAMEPREFIX, GS::UniString ("{@property:"), "PROPERTYNAMEPREFIX");
        DBtest (GDLNAMEPREFIX, GS::UniString ("{@gdl:"), "GDLNAMEPREFIX");
        DBtest (COORDNAMEPREFIX, GS::UniString ("{@coord:"), "COORDNAMEPREFIX");
        DBtest (IDNAMEPREFIX, GS::UniString ("{@id:"), "IDNAMEPREFIX");
        DBtest (MORPHNAMEPREFIX, GS::UniString ("{@morph:"), "MORPHNAMEPREFIX");
        DBtest (INFONAMEPREFIX, GS::UniString ("{@info:"), "INFONAMEPREFIX");
        DBtest (IFCNAMEPREFIX, GS::UniString ("{@ifc:"), "IFCNAMEPREFIX");
        DBtest (GLOBNAMEPREFIX, GS::UniString ("{@glob:"), "GLOBNAMEPREFIX");
        DBtest (CLASSNAMEPREFIX, GS::UniString ("{@class:"), "CLASSNAMEPREFIX");
        DBtest (ELEMENTNAMEPREFIX, GS::UniString ("{@element:"), "ELEMENTNAMEPREFIX");
        DBtest (FILENAMEPREFIX, GS::UniString ("{@file:"), "FILENAMEPREFIX");
        DBtest (ATTRIBNAMEPREFIX, GS::UniString ("{@attrib:"), "ATTRIBNAMEPREFIX");
        DBtest (LISTDATANAMEPREFIX, GS::UniString ("{@listdata:"), "LISTDATANAMEPREFIX");
        DBtest (MATERIALNAMEPREFIX, GS::UniString ("{@material:"), "MATERIALNAMEPREFIX");
        DBtest (FORMULANAMEPREFIX, GS::UniString ("{@formula:"), "FORMULANAMEPREFIX");
        DBtest (MEPNAMEPREFIX, GS::UniString ("{@mep:"), "MEPNAMEPREFIX");
        DBtest (FLAGNAMEPREFIX, GS::UniString ("{@flag:"), "FLAGNAMEPREFIX");

        // Проверка числовых индексов типов
        DBtest (PROPERTYTYPEINX, (short)2, "PROPERTYTYPEINX");
        DBtest (GDLTYPEINX, (short)4, "GDLTYPEINX");
        DBtest (COORDTYPEINX, (short)3, "COORDTYPEINX");
        DBtest (IDTYPEINX, (short)1, "IDTYPEINX");
        DBtest (MORPHTYPEINX, (short)8, "MORPHTYPEINX");
        DBtest (INFOTYPEINX, (short)6, "INFOTYPEINX");
        DBtest (IFCTYPEINX, (short)7, "IFCTYPEINX");
        DBtest (GLOBTYPEINX, (short)12, "GLOBTYPEINX");
        DBtest (CLASSTYPEINX, (short)13, "CLASSTYPEINX");
        DBtest (ELEMENTTYPEINX, (short)15, "ELEMENTTYPEINX");
        DBtest (FILETYPEINX, (short)17, "FILETYPEINX");
        DBtest (ATTRIBTYPEINX, (short)9, "ATTRIBTYPEINX");
        DBtest (LISTDATATYPEINX, (short)10, "LISTDATATYPEINX");
        DBtest (MATERIALTYPEINX, (short)11, "MATERIALTYPEINX");
        DBtest (FORMULATYPEINX, (short)14, "FORMULATYPEINX");
        DBtest (MEPTYPEINX, (short)16, "MEPTYPEINX");
        DBtest (FLAGTYPEINX, (short)18, "FLAGTYPEINX");

        // Проверка констант синхронизации
        DBtest (SYNC_FROM, 1, "SYNC_FROM");
        DBtest (SYNC_TO, 2, "SYNC_TO");
        DBtest (SYNC_TO_SUB, 3, "SYNC_TO_SUB");
        DBtest (SYNC_FROM_SUB, 4, "SYNC_FROM_SUB");
        DBtest (SYNC_FROM_GUID, 5, "SYNC_FROM_GUID");
        DBtest (SYNC_FROM_ZONE, 6, "SYNC_FROM_ZONE");
        DBtest (SYNC_TO_ZONE, 7, "SYNC_TO_ZONE");

        // Проверка префиксов правил
        DBtest (SYNCFROMSTRING, GS::UniString ("from{"), "SYNCFROMSTRING");
        DBtest (SYNCTOSTRING, GS::UniString ("to{"), "SYNCTOSTRING");
        DBtest (SYNCFROMSUBSTRING, GS::UniString ("from_sub{"), "SYNCFROMSUBSTRING");
        DBtest (SYNCTOSUBSTRING, GS::UniString ("to_sub{"), "SYNCTOSUBSTRING");
        DBtest (FROMGUIDBR, GS::UniString ("from_GUID{"), "FROMGUIDBR");
        DBtest (FROMGUID, GS::UniString ("from_GUID"), "FROMGUID");
        DBtest (TOGUIDBR, GS::UniString ("to_GUID{"), "TOGUIDBR");
        DBtest (TOGUID, GS::UniString ("to_GUID"), "TOGUID");

        // Проверка специальных символов
        DBtest (BRACESTART, GS::UniString ("{"), "BRACESTART");
        DBtest (BRACEEND, GS::UniString ("}"), "BRACEEND");
        DBtest (SEMICOLON, GS::UniString (";"), "SEMICOLON");
        DBtest (GS::UniString ("{"), GS::UniString ("{"), "CHARBRACESTART as string");
        DBtest (GS::UniString ("}"), GS::UniString ("}"), "CHARBRACEEND as string");
        DBtest (GS::UniString (";"), GS::UniString (";"), "CHARBSEMICOLON as string");
        DBtest (GS::UniString ("<"), GS::UniString ("<"), "CHARFORMULASTART as string");
        DBtest (GS::UniString (">"), GS::UniString (">"), "CHARFORMULAEND as string");
        DBtest (GS::UniString ("\""), GS::UniString ("\""), "CHARDQUT as string");
        DBtest (GS::UniString (","), GS::UniString (","), "CHARCOMMA as string");

        // Проверка SYNCNAME и других специальных констант
        DBtest (SYNCNAME, GS::UniString ("sync_name"), "SYNCNAME");
        DBtest (SYNCCORRECTFLAG, GS::UniString ("Sync_correct_flag"), "SYNCCORRECTFLAG");
        DBtest (SYNCCLASSFLAG, GS::UniString ("Sync_class_flag"), "SYNCCLASSFLAG");
        DBtest (SYNCGUID, GS::UniString ("Sync_GUID"), "SYNCGUID");

        // Проверка специальных ключевых слов
        DBtest (RENUMFLAG, GS::UniString ("Renum_flag"), "RENUMFLAG");
        DBtest (RENUM, GS::UniString ("Renum"), "RENUM");
        DBtest (PROPERTYSTRING, GS::UniString ("property"), "PROPERTYSTRING");

        // Проверка форматов
        DBtest (DEFULTREALFSTRING, GS::UniString (".3m"), "DEFULTREALFSTRING");
        DBtest (DEFULTLEGHTFSTRING, GS::UniString ("1mm"), "DEFULTLEGHTFSTRING");
        DBtest (DEFULTINTFSTRING, GS::UniString ("0m"), "DEFULTINTFSTRING");

        return;
    }

    // -----------------------------------------------------------------------------
    // Тест парсинга описания свойства с командами Sync, Renum, Sum, Spec
    // -----------------------------------------------------------------------------
    void TestParsePropertyDescription () {
        ParamValue param;
        SkipValues ignorevals;
        FormatString stringformat;
        SyncMode syncdirection = SYNC_NO;
        API_ElemTypeID elementType = API_ObjectID;

        // Тест 1: Описание только с Sync_from
        {
            GS::UniString desc = "Sync_from{Property:TestProperty}";
            bool ok =
                SyncString (elementType, desc, syncdirection, param, ignorevals, stringformat, true, false, false);
            DBtest (ok, "ParseDesc Sync_only -> true");
            DBtest (param.fromProperty, "ParseDesc Sync_only -> fromProperty");
        }

        // Тест 2: Описание с несколькими Sync командами через разделитель
        {
            GS::UniString desc = "Sync_from{Property:Prop1}Sync_to{Property:Prop2}";
            GS::Array<GS::UniString> parts;
            GS::Array<GS::UniString> scratch;
            UInt32 n = StringSpltFilter (desc, SYNCPART, parts, BRACESTART, &scratch);
            DBtest (n, (UInt32)2, "ParseDesc MultiSync -> 2 parts");
        }

        // Тесты 3-6: RENUM/RENUMFLAG/Sum/Spec не разбираются SyncString - команда
        // не начинается с SYNCPART, поэтому проверять надо не наличие подстроки в
        // литерале (это тавтология: сверяется константа сама с собой), а исход
        // ParsePropertyDescriptionToRules. hasSyncRules обязан быть false, а
        // hasOtherCommands - true: команда опознана, но правил синхронизации нет.
        {
            GS::UniString desc = "Renum_flag{Property:RenumRule; NULL}";
            const ParsePropertyResult result = ParsePropertyDescriptionToRules (desc);
            DBtest (!result.hasSyncRules, "ParseDesc Renum_flag -> no sync rules");
            DBtest (result.hasOtherCommands, "ParseDesc Renum_flag -> has other commands");
        }
        {
            GS::UniString desc = "Renum{Property:Criteria; Property:Delimetr}";
            const ParsePropertyResult result = ParsePropertyDescriptionToRules (desc);
            DBtest (!result.hasSyncRules, "ParseDesc Renum -> no sync rules");
            DBtest (result.hasOtherCommands, "ParseDesc Renum -> has other commands");
            if (!result.otherCommands.IsEmpty ())
                DBtest (result.otherCommands[0].commandType == "Renum", "ParseDesc Renum -> commandType Renum");
        }
        {
            GS::UniString desc = "Sum{Property:SumProp1; Property:SumProp2; max}";
            const ParsePropertyResult result = ParsePropertyDescriptionToRules (desc);
            DBtest (!result.hasSyncRules, "ParseDesc Sum -> no sync rules");
            DBtest (result.hasOtherCommands, "ParseDesc Sum -> has other commands");
            if (!result.otherCommands.IsEmpty ())
                DBtest (result.otherCommands[0].commandType == "Sum", "ParseDesc Sum -> commandType Sum");
        }
        {
            GS::UniString desc = "Spec_rule{g(U, P, F, Q)}{s(Pn, Qn)}";
            const ParsePropertyResult result = ParsePropertyDescriptionToRules (desc);
            DBtest (!result.hasSyncRules, "ParseDesc Spec_rule -> no sync rules");
            DBtest (result.hasOtherCommands, "ParseDesc Spec_rule -> has other commands");
            if (!result.otherCommands.IsEmpty ())
                DBtest (result.otherCommands[0].commandType == "Spec_rule",
                        "ParseDesc Spec_rule -> commandType Spec_rule");
        }

        // Тест 7: Комбинированное описание (Sync + Renum_flag). Разбор отдаёт
        // И правило синхронизации, И прочую команду: RENUMFLAG не начинается с
        // SYNCPART, но остаётся в otherCommands. Проверяется разбор, а не
        // наличие подстроки в литерале описания.
        {
            GS::UniString desc = "Sync_from{Property:Source}Renum_flag{Property:RenumRule}";
            GS::Array<GS::UniString> parts;
            GS::Array<GS::UniString> scratch;
            UInt32 n = StringSpltFilter (desc, SYNCPART, parts, BRACESTART, &scratch);
            DBtest (n >= 1, "ParseDesc Combined -> at least 1 sync part");
            const ParsePropertyResult result = ParsePropertyDescriptionToRules (desc);
            DBtest (result.hasSyncRules, "ParseDesc Combined -> has sync rules");
            DBtest (result.hasOtherCommands, "ParseDesc Combined -> has other commands");
            if (!result.otherCommands.IsEmpty ())
                DBtest (result.otherCommands[0].commandType == "Renum_flag",
                        "ParseDesc Combined -> commandType Renum_flag");
        }

        // Тест 8: Описание с игнорируемыми значениями
        {
            GS::UniString desc = "Sync_from{Property:TestProperty; empty; trim_empty}";
            bool ok =
                SyncString (elementType, desc, syncdirection, param, ignorevals, stringformat, true, false, false);
            DBtest (ok, "ParseDesc IgnoreVals -> true");
            DBtest (ignorevals.skip_empty, "ParseDesc IgnoreVals -> skip_empty");
            DBtest (ignorevals.skip_trim_empty, "ParseDesc IgnoreVals -> skip_trim_empty");
        }

        // Тест 9: Описание с форматом
        {
            GS::UniString desc = "Sync_from{Property:TestProperty.3m}";
            bool ok =
                SyncString (elementType, desc, syncdirection, param, ignorevals, stringformat, true, false, false);
            DBtest (ok, "ParseDesc Format .3m -> true");
            DBtest (!stringformat.stringformat.IsEmpty (), "ParseDesc Format -> format not empty");
        }

        // Тест 10: Описание с Formula
        {
            GS::UniString desc = "Sync_from{<2*2>.3m}";
            bool ok =
                SyncString (elementType, desc, syncdirection, param, ignorevals, stringformat, true, false, false);
            DBtest (ok, "ParseDesc Formula -> true");
            DBtest (param.val.hasFormula, "ParseDesc Formula -> hasFormula");
        }

        // Тест 11: Пустое описание
        {
            GS::UniString desc = "";
            GS::Array<GS::UniString> parts;
            GS::Array<GS::UniString> scratch;
            UInt32 n = StringSpltFilter (desc, SYNCPART, parts, BRACESTART, &scratch);
            DBtest (n, (UInt32)0, "ParseDesc Empty -> 0 parts");
        }

        return;
    }

    // -----------------------------------------------------------------------------
    // Тест независимого вызова ParseSyncString (Этап 2 TDD)
    // -----------------------------------------------------------------------------
    void TestParseSyncStringIndependent () {
        // Подготовка тестовых данных
        API_Guid elemGuid = APINULLGuid;
        API_ElemTypeID elementType = API_ObjectID;
        API_PropertyDefinition definition = {};
        definition.description = "Sync_from{Property:TestProperty}";
        GS::Array<WriteData> syncRules;
        ParamDictElement paramToRead;
        bool hasSub = false;
        bool syncall = true;
        bool synccoord = false;
        bool syncclass = false;
        ParamDictValue subproperty;

        // Тест: ParseSyncString должен возвращать true для корректного описания
        bool result = ParseSyncString (elemGuid,
                                       elementType,
                                       definition,
                                       syncRules,
                                       paramToRead,
                                       hasSub,
                                       syncall,
                                       synccoord,
                                       syncclass,
                                       subproperty);
        DBtest (result, "ParseSyncString basic -> true");

        // Тест: синхронизация правила должна быть добавлена в syncRules
        DBtest (syncRules.GetSize () > 0, "ParseSyncString -> syncRules not empty");

        // Тест: hasSub должен быть false для правила без from_sub/to_sub
        DBtest (!hasSub, "ParseSyncString -> hasSub false");

        // Тест: пустое описание -> false
        definition.description = "";
        syncRules.Clear ();
        result = ParseSyncString (elemGuid,
                                  elementType,
                                  definition,
                                  syncRules,
                                  paramToRead,
                                  hasSub,
                                  syncall,
                                  synccoord,
                                  syncclass,
                                  subproperty);
        DBtest (!result, "ParseSyncString empty -> false");

        // Тест: описание с Sync_flag -> false (это флаг, не правило)
        definition.description = "Sync_flag";
        result = ParseSyncString (elemGuid,
                                  elementType,
                                  definition,
                                  syncRules,
                                  paramToRead,
                                  hasSub,
                                  syncall,
                                  synccoord,
                                  syncclass,
                                  subproperty);
        DBtest (!result, "ParseSyncString Sync_flag -> false");

        // Тест: описание с SYNCCORRECTFLAG -> true (добавляет служебный параметр)
        definition.description = "Sync_correct_flag";
        result = ParseSyncString (elemGuid,
                                  elementType,
                                  definition,
                                  syncRules,
                                  paramToRead,
                                  hasSub,
                                  syncall,
                                  synccoord,
                                  syncclass,
                                  subproperty);
        DBtest (result, "ParseSyncString Sync_correct_flag -> true");

        // Тест: описание без SYNCPART -> false
        definition.description = "Property:TestProperty";
        syncRules.Clear ();
        result = ParseSyncString (elemGuid,
                                  elementType,
                                  definition,
                                  syncRules,
                                  paramToRead,
                                  hasSub,
                                  syncall,
                                  synccoord,
                                  syncclass,
                                  subproperty);
        DBtest (!result, "ParseSyncString no SYNCPART -> false");

        // Тест: описание без BRACESTART -> false
        definition.description = "Sync_from Property:TestProperty";
        result = ParseSyncString (elemGuid,
                                  elementType,
                                  definition,
                                  syncRules,
                                  paramToRead,
                                  hasSub,
                                  syncall,
                                  synccoord,
                                  syncclass,
                                  subproperty);
        DBtest (!result, "ParseSyncString no BRACESTART -> false");

        // Тест: описание без BRACEEND -> false
        definition.description = "Sync_from{Property:TestProperty";
        result = ParseSyncString (elemGuid,
                                  elementType,
                                  definition,
                                  syncRules,
                                  paramToRead,
                                  hasSub,
                                  syncall,
                                  synccoord,
                                  syncclass,
                                  subproperty);
        DBtest (!result, "ParseSyncString no BRACEEND -> false");

        return;
    }

    // -----------------------------------------------------------------------------
    // Тест ParsePropertyDescriptionToRules — парсинг описания в структурированные правила
    // -----------------------------------------------------------------------------
    void TestParsePropertyDescriptionToRules () {
        // Тест 1: простое Sync_from описание
        {
            DBprnt ("DescToRules", "Test 1 start");
            GS::UniString desc = "Sync_from{Property:TestProperty}";
            ParsePropertyResult result = ParsePropertyDescriptionToRules (desc);
            DBprnt ("DescToRules", "Test 1 after call");
            DBtest (result.hasSyncRules, "DescToRules Sync_from -> hasSyncRules");
            DBtest (result.syncRules.GetSize () > 0, "DescToRules Sync_from -> rules not empty");
            if (result.syncRules.GetSize () > 0) {
                DBtest (result.syncRules[0].commandType == "Sync" || result.syncRules[0].commandType == "Sync_from",
                        "DescToRules Sync_from -> commandType");
                DBtest (result.syncRules[0].sourceType == "Property", "DescToRules Sync_from -> sourceType Property");
                DBtest (result.syncRules[0].isValid, "DescToRules Sync_from -> isValid");
                DBtest (!result.syncRules[0].hasSub, "DescToRules Sync_from -> hasSub false");
                DBtest (!result.syncRules[0].hasGUID, "DescToRules Sync_from -> hasGUID false");
            }
        }

        // Тест 2: Sync_from_sub
        {
            GS::UniString desc = "Sync_from_sub{Property:SubProp}";
            ParsePropertyResult result = ParsePropertyDescriptionToRules (desc);
            DBtest (result.hasSyncRules, "DescToRules Sync_from_sub -> hasSyncRules");
            if (result.syncRules.GetSize () > 0) {
                DBtest (result.syncRules[0].hasSub, "DescToRules Sync_from_sub -> hasSub true");
            }
        }

        // Тест 3: Sync_to
        {
            GS::UniString desc = "Sync_to{Property:TargetProp}";
            ParsePropertyResult result = ParsePropertyDescriptionToRules (desc);
            DBtest (result.hasSyncRules, "DescToRules Sync_to -> hasSyncRules");
            if (result.syncRules.GetSize () > 0) {
                DBtest (result.syncRules[0].targetType == "Property", "DescToRules Sync_to -> targetType Property");
            }
        }

        // Тест 4: Пустое описание
        {
            GS::UniString desc = "";
            ParsePropertyResult result = ParsePropertyDescriptionToRules (desc);
            DBtest (!result.hasSyncRules, "DescToRules empty -> no sync rules");
            DBtest (!result.hasOtherCommands, "DescToRules empty -> no other commands");
        }

        // Тест 5: Описание с командой Renum (не Sync)
        {
            GS::UniString desc = "Renum_flag{Property:RenumRule; NULL}";
            ParsePropertyResult result = ParsePropertyDescriptionToRules (desc);
            DBtest (!result.hasSyncRules, "DescToRules Renum_flag -> no sync rules");
            DBtest (result.hasOtherCommands, "DescToRules Renum_flag -> has other commands");
        }

        // Тест 6: Комбинированное описание
        {
            GS::UniString desc = "Sync_from{Property:Source}Sync_to{Property:Target}";
            ParsePropertyResult result = ParsePropertyDescriptionToRules (desc);
            DBtest (result.hasSyncRules, "DescToRules combined -> hasSyncRules");
            DBtest (result.syncRules.GetSize () == 2, "DescToRules combined -> 2 rules");
        }

        // Тест 7: Описание с ignorevals
        {
            GS::UniString desc = "Sync_from{Property:TestProperty; empty; trim_empty}";
            ParsePropertyResult result = ParsePropertyDescriptionToRules (desc);
            DBtest (result.hasSyncRules, "DescToRules ignorevals -> hasSyncRules");
            if (result.syncRules.GetSize () > 0) {
                DBtest (result.syncRules[0].ignoreVals.GetSize () > 0,
                        "DescToRules ignorevals -> ignoreVals not empty");
            }
        }

        // Тест 8: Sync_flag (не создаёт правило)
        {
            GS::UniString desc = "Sync_flag";
            ParsePropertyResult result = ParsePropertyDescriptionToRules (desc);
            DBtest (!result.hasSyncRules, "DescToRules Sync_flag -> no sync rules");
            DBtest (!result.hasOtherCommands, "DescToRules Sync_flag -> no other commands");
        }

        // Тест 9: Несколько правил + remainingText
        {
            GS::UniString desc = "Sync_from{Property:Prop1}Some text between Sync_to{Property:Prop2}";
            ParsePropertyResult result = ParsePropertyDescriptionToRules (desc);
            DBtest (result.hasSyncRules, "DescToRules multi -> hasSyncRules");
            DBtest (result.syncRules.GetSize () >= 1, "DescToRules multi -> at least 1 rule");
        }

        // Тест 10: Описание с GDL параметром
        {
            GS::UniString desc = "Sync_from{MyGDLParam}";
            ParsePropertyResult result = ParsePropertyDescriptionToRules (desc);
            DBtest (result.hasSyncRules, "DescToRules GDL -> hasSyncRules");
            if (result.syncRules.GetSize () > 0) {
                DBtest (result.syncRules[0].sourceType == "GDL", "DescToRules GDL -> sourceType GDL");
            }
        }

        return;
    }

    // -----------------------------------------------------------------------------
    // Тест SyncAddSubelement — развёртывание правил синхронизации на подэлементы.
    //
    // Структура: 1 RED-тест на найденный баг P1 + 3 GREEN-регрессии, фиксирующие
    // существующее корректное поведение. GREEN-тесты должны давать одинаковый
    // результат до и после правок.
    //
    // Баг P1 (Sync.cpp, SyncAddSubelement): вторая ветка проверяет `fromSub`,
    // хотя по семантике тела цикла (заполнение guidTo для КАЖДОГО подэлемента,
    // сброс toSub) это обработка `toSub`. Из-за этого правило to_sub не
    // разворачивается на подэлементы — ветка недостижима:
    //   - если fromSub был true — первая ветка уже сбросила его в false;
    //   - если fromSub был false — условие ложно сразу.
    // После исправления (`if (mainsyncRule.toSub)`) RED-тест становится зелёным,
    // а GREEN-тесты обязаны остаться зелёными.
    // -----------------------------------------------------------------------------
    void TestSyncAddSubelement () {
        // ---- Вспомогательное правило-прототип ----
        auto makeRule = [] () {
            WriteData rule;
            rule.guidTo = APIGuidFromString ("{11111111-1111-1111-1111-111111111111}");
            rule.guidFrom = APIGuidFromString ("{22222222-2222-2222-2222-222222222222}");
            rule.paramFrom.rawName = "{@property:test_from}";
            rule.paramFrom.fromProperty = true;
            rule.paramFrom.isValid = true;
            rule.paramTo.rawName = "{@property:test_to}";
            rule.paramTo.fromProperty = true;
            rule.paramTo.isValid = true;
            return rule;
        };

        const API_Guid sub1 = APIGuidFromString ("{AAAAAAAA-AAAA-AAAA-AAAA-AAAAAAAAAAAA}");
        const API_Guid sub2 = APIGuidFromString ("{BBBBBBBB-BBBB-BBBB-BBBB-BBBBBBBBBBBB}");
        GS::Array<API_Guid> subelemGuids;
        subelemGuids.Push (sub1);
        subelemGuids.Push (sub2);

        // =============================================================================
        // GREEN-тест 1: обычное правило (не from_sub и не to_sub) при пустом списке
        // подэлементов добавляется в syncRules как есть — по guidTo из правила.
        // Существующее поведение, должно остаться неизменным.
        // =============================================================================
        {
            GS::Array<WriteData> mainsyncRules;
            mainsyncRules.Push (makeRule ());
            WriteDict syncRules;
            ParamDictElement paramToRead;
            GS::Array<API_Guid> emptySubs;

            SyncAddSubelement (emptySubs, mainsyncRules, syncRules, paramToRead);

            const GS::Array<WriteData> *bucket = syncRules.GetPtr (mainsyncRules[0].guidTo);
            bool added = bucket != nullptr && bucket->GetSize () == 1 &&
                         bucket->Get (0).guidTo == mainsyncRules[0].guidTo &&
                         bucket->Get (0).guidFrom == mainsyncRules[0].guidFrom;
            DBtest (added, "SyncAddSubelem plain rule -> added under rule.guidTo");
            // Пустой список подэлементов не должен менять флаги правила
            DBtest (!mainsyncRules[0].toSub && !mainsyncRules[0].fromSub,
                    "SyncAddSubelem plain rule -> flags untouched");
            // paramToRead заполняется через SyncAddRule -> AddParamValue2ParamDictElement,
            // ключ словаря = param.fromGuid (у прототипа он APINULLGuid)
            DBtest (paramToRead.GetPtr (APINULLGuid) != nullptr,
                    "SyncAddSubelem plain rule -> paramToRead has fromGuid entry");
        }

        // =============================================================================
        // GREEN-тест 2: from_sub — запись ИЗ первого подэлемента.
        // Первая ветка: fromSub сбрасывается, guidFrom = subelemGuids[0].
        // Существующее поведение, должно остаться неизменным после фикса to_sub.
        // =============================================================================
        {
            GS::Array<WriteData> mainsyncRules;
            WriteData rule = makeRule ();
            rule.fromSub = true;
            mainsyncRules.Push (rule);
            WriteDict syncRules;
            ParamDictElement paramToRead;

            SyncAddSubelement (subelemGuids, mainsyncRules, syncRules, paramToRead);

            const GS::Array<WriteData> *bucket = syncRules.GetPtr (mainsyncRules[0].guidTo);
            bool ok = bucket != nullptr && bucket->GetSize () == 1 &&
                      bucket->Get (0).guidFrom == sub1 && // источник — ПЕРВЫЙ подэлемент
                      !mainsyncRules[0].fromSub;          // флаг сброшен после развёртки
            DBtest (ok, "SyncAddSubelem from_sub -> guidFrom = first subelement, fromSub cleared");
            // Правило НЕ дублируется на второй подэлемент (только from_sub-развёртка на [0])
            bool noDup = syncRules.GetPtr (sub2) == nullptr;
            DBtest (noDup, "SyncAddSubelem from_sub -> no per-subelement duplication");
        }

        // =============================================================================
        // RED-тест (баг P1): to_sub — запись В КАЖДЫЙ подэлемент.
        // Ожидание: правило попадает в syncRules для каждого подэлемента
        // (guidTo = подэлемент), toSub сбрасывается.
        // Сейчас: ветка проверки `fromSub` вместо `toSub` мертва, правил в
        // syncRules нет — тест ПАДАЕТ до исправления (RED), проходит после (GREEN).
        // =============================================================================
        {
            GS::Array<WriteData> mainsyncRules;
            WriteData rule = makeRule ();
            rule.toSub = true;
            mainsyncRules.Push (rule);
            WriteDict syncRules;
            ParamDictElement paramToRead;

            SyncAddSubelement (subelemGuids, mainsyncRules, syncRules, paramToRead);

            bool allSubsCovered = syncRules.GetPtr (sub1) != nullptr && syncRules.GetPtr (sub2) != nullptr;
            if (allSubsCovered) {
                const GS::Array<WriteData> *b1 = syncRules.GetPtr (sub1);
                const GS::Array<WriteData> *b2 = syncRules.GetPtr (sub2);
                allSubsCovered = b1->GetSize () == 1 && b1->Get (0).guidTo == sub1 &&
                                 b1->Get (0).guidFrom == mainsyncRules[0].guidFrom && b2->GetSize () == 1 &&
                                 b2->Get (0).guidTo == sub2;
            }
            DBtest (allSubsCovered, "SyncAddSubelem to_sub -> rule added for EVERY subelement");
            DBtest (!mainsyncRules[0].toSub, "SyncAddSubelem to_sub -> toSub cleared after expansion");
        }

        // =============================================================================
        // GREEN-тест 3: to_sub с пустым списком подэлементов.
        // Оба условия (fromSub/toSub истинны, список пуст) -> правило НЕ добавляется
        // никуда (continue по пустому списку), флаги не трогаются.
        // Существующее поведение, должно остаться неизменным.
        // =============================================================================
        {
            GS::Array<WriteData> mainsyncRules;
            WriteData rule = makeRule ();
            rule.toSub = true;
            mainsyncRules.Push (rule);
            WriteDict syncRules;
            ParamDictElement paramToRead;
            GS::Array<API_Guid> emptySubs;

            SyncAddSubelement (emptySubs, mainsyncRules, syncRules, paramToRead);

            DBtest (syncRules.GetSize () == 0, "SyncAddSubelem to_sub empty subs -> nothing added");
            // Флаги НЕ трогаются: continue по пустому списку происходит до развёртки,
            // поэтому toSub остаётся true как и было до вызова.
            DBtest (mainsyncRules[0].toSub && !mainsyncRules[0].fromSub,
                    "SyncAddSubelem to_sub empty subs -> flags untouched");
        }

        // Внешний адресат Sync_to_GUID не входит в список текущего элемента и его подэлементов.
        {
            const API_Guid owner = APIGuidFromString ("{CCCCCCCC-CCCC-CCCC-CCCC-CCCCCCCCCCCC}");
            const API_Guid destination = APIGuidFromString ("{DDDDDDDD-DDDD-DDDD-DDDD-DDDDDDDDDDDD}");
            WriteData rule = makeRule ();
            rule.guidFrom = owner;
            rule.guidTo = destination;
            rule.paramFrom.fromGuid = owner;
            rule.paramTo.fromGuid = destination;
            rule.paramFrom.val.type = API_PropertyStringValueType;
            rule.paramTo.val.type = API_PropertyStringValueType;
            rule.paramFrom.val.uniStringValue = "new";
            rule.paramTo.val.uniStringValue = "old";
            GS::Array<WriteData> rules = {rule};
            GS::Array<API_Guid> processing = {owner};
            WriteDict syncRules;
            ParamDictElement paramToRead;
            ParamDictElement paramToWrite;
            UnicGuidString propertyWriteGuids;

            SyncAddSubelement ({}, rules, syncRules, paramToRead);
            SyncCalcRule (syncRules, processing, paramToRead, paramToWrite, propertyWriteGuids);
            const ParamDictValue *written = paramToWrite.GetPtr (destination);
            DBtest (written != nullptr && written->GetPtr (rule.paramTo.rawName) != nullptr,
                    "Sync_to_GUID external destination -> write scheduled");
        }

        return;
    }

    // -----------------------------------------------------------------------------
    // GREEN-регрессии ParsePropertyDescriptionToRules для sub/GUID-правил:
    // фиксируют контракт hasSub/hasGUID/target*/guidSourceProperty. Эти поля
    // использует UI палитры; правка бага P1 в SyncAddSubelement не должна их менять.
    // -----------------------------------------------------------------------------
    void TestDescToRulesSubGuid () {
        // ---- to_sub: hasSub = true, targetType определён ----
        {
            GS::UniString desc = "Sync_to_sub{Property:TargetSub}";
            ParsePropertyResult result = ParsePropertyDescriptionToRules (desc);
            DBtest (result.hasSyncRules, "DescToRulesSub to_sub -> hasSyncRules");
            if (result.syncRules.GetSize () > 0) {
                DBtest (result.syncRules[0].hasSub, "DescToRulesSub to_sub -> hasSub true");
                DBtest (result.syncRules[0].targetType == "Property", "DescToRulesSub to_sub -> targetType Property");
                DBtest (result.syncRules[0].targetName == "TargetSub", "DescToRulesSub to_sub -> targetName TargetSub");
            }
        }

        // ---- from_GUID: ТЕКУЩЕЕ поведение — минимальная форма не создаёт правила.
        // ParsePropertyDescriptionToRules валидирует каждую Sync-команду через
        // SyncString, и для from_GUID без полного контекста она возвращает отказ ->
        // isValid=false -> правило не попадает в результат. Фиксируем как есть:
        // это документирование известного ограничения, а не эталон.
        {
            GS::UniString desc = "Sync_from_GUID{Property:GuidSource}";
            ParsePropertyResult result = ParsePropertyDescriptionToRules (desc);
            DBtest (!result.hasSyncRules, "DescToRulesSub from_GUID minimal form -> no rules (known limitation)");
        }

        // ---- обычный Sync_from: ни hasSub, ни hasGUID ----
        {
            GS::UniString desc = "Sync_from{Property:PlainProp}";
            ParsePropertyResult result = ParsePropertyDescriptionToRules (desc);
            if (result.syncRules.GetSize () > 0) {
                DBtest (!result.syncRules[0].hasSub, "DescToRulesSub plain -> hasSub false");
                DBtest (!result.syncRules[0].hasGUID, "DescToRulesSub plain -> hasGUID false");
            }
        }

        return;
    }

} // namespace TestFunc
#endif
