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
#include	"DG.h"
#include	"Helpers.hpp"
static const short TextSum = 1;
static const short NumSum = 2;

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

GSErrCode SumSelected(SyncSettings& syncSettings);

bool GetSumValuesOfElements(const GS::Array<API_Guid> guidArray, ParamDictElement& paramToWriteelem);

bool Sum_GetElement(const API_Guid& elemGuid, ParamDictValue& propertyParams, ParamDictValue & paramToRead, SumRules& rules);

bool Sum_Rule(const API_Guid& elemGuid, const API_PropertyDefinition& definition, ParamDictValue& propertyParams, SumRule& paramtype);

void Sum_OneRule(const SumRule& rule, ParamDictElement& paramToReadelem, ParamDictElement& paramToWriteelem);

#endif