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

struct SumRule
{
    GS::UniString position = "";
    GS::UniString value = "";
    GS::UniString criteria = "";
    std::string delimetr = "; ";
    std::string ignore_val = "";
    short sum_type = 0;
    GS::Array <API_Guid> elemts;
};

typedef GS::HashTable<API_Guid, SumRule> SumRules;

GSErrCode SumSelected (SyncSettings& syncSettings);

bool GetSumValuesOfElements (GS::Array<API_Guid>& guidArray, ParamDictElement& paramToWriteelem);

// ----------------------------------------------------------------------------
// Функция распределяет элемент в таблицу с правилами нумерации
// ----------------------------------------------------------------------------
bool Sum_GetElement (const GS::Array<API_Guid>& guidArray, const GS::HashTable<API_Guid, API_PropertyDefinition>& rule_definitions, ParamDictElement& paramToRead, SumRules& rules);

bool Sum_Rule (const API_PropertyDefinition& definition, SumRule& paramtype);

void Sum_OneRule (const SumRule& rule, ParamDictElement& paramToReadelem, ParamDictElement& paramToWriteelem);

#endif
