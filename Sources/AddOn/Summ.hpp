//------------ kuvbur 2022 ------------
#pragma once
#if !defined (SUMM_HPP)
#define	SUMM_HPP
#ifdef AC_25
#include	"APICommon25.h"
#endif // AC_25
#ifdef AC_26
#include	"APICommon26.h"
#endif // AC_26
#if defined(AC_27)
#include	"APICommon27.h"
#endif // AC_27
#ifdef AC_28
#include	"APICommon28.h"
#endif // AC_28
#include	"DG.h"
#include	"Helpers.hpp"

static const short TextSum = 1;
static const short NumSum = 2;
static const short MinSum = 3;
static const short MaxSum = 4;

typedef struct {
    GS::UniString position = "";
    GS::UniString value = "";
    GS::UniString criteria = "";
    std::string delimetr = "; ";
    std::string ignore_val = "";
    short sum_type = 0;
    GS::Array <API_Guid> elemts;
} SumRule;

typedef GS::HashTable<API_Guid, SumRule> SumRules;

GSErrCode SumSelected (SyncSettings& syncSettings);

bool GetSumRuleFromSelected (const API_Guid& elemguid, GS::HashTable<API_Guid, API_PropertyDefinition>& definitions);

void GetSumElementForPropertyDefinition (const GS::HashTable<API_Guid, API_PropertyDefinition>& definitions, GS::Array<API_Guid>& guidArray);

bool GetSumValuesOfElements (const GS::Array<API_Guid> guidArray, ParamDictElement& paramToWriteelem, GS::HashTable<API_Guid, API_PropertyDefinition>& rule_definitions);

bool Sum_GetElement (const API_Guid& elemGuid, ParamDictValue& propertyParams, ParamDictValue& paramToRead, SumRules& rules, GS::HashTable<API_Guid, API_PropertyDefinition>& rule_definitions);

bool Sum_Rule (const API_Guid& elemGuid, const API_PropertyDefinition& definition, ParamDictValue& propertyParams, SumRule& paramtype);

void Sum_OneRule (const SumRule& rule, ParamDictElement& paramToReadelem, ParamDictElement& paramToWriteelem);

#endif
