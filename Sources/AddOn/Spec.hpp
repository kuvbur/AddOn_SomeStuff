//------------ kuvbur 2022 ------------
#pragma once
#if !defined (SPEC_HPP)
#define	SPEC_HPP
#ifdef AC_25
#include	"APICommon25.h"
#endif // AC_25
#ifdef AC_26
#include	"APICommon26.h"
#endif // AC_26
#ifdef AC_27
#include	"APICommon27.h"
#endif // AC_27
#ifdef AC_28
#include	"APICommon28.h"
#endif // AC_28
#include	"Helpers.hpp"

namespace Spec
{
typedef struct
{
    GS::Array<GS::UniString> unic_paramrawname; // Массив имён уникальных параметров
    GS::Array<GS::UniString> out_paramrawname; // Массив имён параметров для передачи новым элементам
    GS::Array<GS::UniString> sum_paramrawname; // Массив имён параметров, которые требуется просуммировать
    GS::UniString flag_paramrawname;           // Имя параметра-флага, определяющего - будет ли элемент учтён в спецификации
    bool is_Valid = true;
} GroupSpec;


typedef struct
{
    GS::Array<GroupSpec> groups;                   // Массив с группами подэлементов 
    GS::Array<GS::UniString> out_paramrawname;     // Массив имён параметров новых элементов
    GS::Array<GS::UniString> out_sum_paramrawname; // Массив имён параметров сумм новых элементов
    GS::UniString subguid_paramrawname = "";       // Имя свойства для записи GUID созданных элементов (в описании должно содержатся Sync_GUID+Имя правила)
    GS::UniString subguid_rulename = "";           // Имя свойства с правилом, на основании которого созданы элементы
    GS::UniString subguid_rulevalue = "";
    GS::Array<API_Guid> elements;                  // Элементы, которые обрабатываются правилом
    API_PropertyDefinition rule_definitions;       // Определение свойства с правилом для поиска элементов, в которых оно доступно
    GS::UniString favorite_name = "";              //Имя элемента в избранном
    bool is_Valid = true;
} SpecRule;

typedef struct
{
    GS::Array<ParamValue> out_param;
    GS::Array<ParamValue> out_sum_param;
    GS::Array<GS::UniString> out_paramrawname;
    GS::Array<GS::UniString> out_sum_paramrawname;
    GS::UniString subguid_paramrawname = "";
    GS::UniString subguid_rulename = "";
    GS::UniString subguid_rulevalue = "";
    GS::UniString favorite_name = "";              //Имя элемента в избранном
    GS::Array<API_Guid> elements;                  // Элементы, которые обрабатываются правилом
} Element;

typedef GS::HashTable<GS::UniString, Element> ElementDict; // Словарь элементов для создания, ключ - сцепка значений уникальных параметров

typedef GS::HashTable<GS::UniString, SpecRule> SpecRuleDict; // Словарь правил, ключ - имя правила

// --------------------------------------------------------------------
// Получение правил из свойств элемента по умолчанию
// --------------------------------------------------------------------
bool GetRuleFromDefaultElem (SpecRuleDict& rules);

GSErrCode SpecAll (const SyncSettings& syncSettings);

GSErrCode SpecArray (const SyncSettings& syncSettings, GS::Array<API_Guid>& guidArray, SpecRuleDict& rules);

// --------------------------------------------------------------------
// Проверяет значение свойства с правилом и формрует правила
// --------------------------------------------------------------------
GSErrCode GetRuleFromElement (const API_Guid& elemguid, SpecRuleDict& rules);

void AddRule (const API_PropertyDefinition& definition, const API_Guid& elemguid, SpecRuleDict& rules);

// --------------------------------------------------------------------
// Разбивает строку на части. Будем постепенно заменять на пустоту обработанные части 
// Критерий - значение, по которому будет сгруппированы элементы
// g(P1, P2, P3; F; Q1, Q2) - P1...P3 параметры, уникальные для вложенного элемента,
// 
//                            F - флаг включения группы 1/0. Если не найден - всегда 1
//                            Q1...Q2 параметры или значения количества, будут просуммированы. Если их нет - запишем 1 для суммы.
// s(Pn1, Pn2, Pn3; Qn1, Qn2) - Pn1...Pn3 параметры размещаемых объектов,
//                              Qn1...Qn2 параметры для записи количества
// Spec_rule {КРИТЕРИЙ ;g(P1, P2, P3; Q1, Q2) g(P4, P5, P6; Q3, Q4) s(Pn1, Pn2, Pn3; Qn1, Qn2)}
// --------------------------------------------------------------------
SpecRule GetRuleFromDescription (GS::UniString& description);

// --------------------------------------------------------------------
// Выбирает из параметров групп имена свойств для дальнейшего чтения
// --------------------------------------------------------------------
void GetParamToReadFromRule (const SpecRuleDict& rules, ParamDictValue& propertyParams, ParamDictElement& paramToRead, ParamDictValue& paramToWrite);

Int32 GetElementsForRule (const SpecRule& rule, const ParamDictElement& paramToRead, ElementDict& elements);

GSErrCode GetElementForPlace (const GS::UniString& favorite_name, API_Element& element, API_ElementMemo& memo);

// --------------------------------------------------------------------
// Получение размеров элемента для размещения по сетке
// Возвращает истину, если был найден параметр somestuff_spec_hrow - в этом случае элементы размещаются сверху вниз
// --------------------------------------------------------------------
bool GetSizePlaceElement (const API_Element& elementt, const API_ElementMemo& memot, double& dx, double& dy);

GSErrCode PlaceElements (GS::Array<ElementDict>& elementstocreate, ParamDictValue& paramToWrite, ParamDictElement& paramOut, Point2D& startpos);
}

#endif
