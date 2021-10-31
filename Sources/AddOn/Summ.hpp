#pragma once
#if !defined (SUMM_HPP)
#define	SUMM_HPP
#include	"APICommon.h"
#include	"DG.h"

typedef struct {
	API_Guid		guid;
	std::string		criteria;
	std::string		value;
} SumElement;

typedef struct {
	API_Guid					position;
	API_PropertyDefinition		value;
	API_PropertyDefinition		criteria;
	std::string					delimetr;
	std::string					ignore_val;
	GS::Array <SumElement>		elemts;
} SumRule;

typedef GS::HashTable<API_Guid, SumRule> SumRules;

GSErrCode SumSelected(void);

GSErrCode Sum_GetElement(const API_Guid& elemGuid, SumRules& rules);

bool Sum_Rule(const API_Guid& elemGuid, const API_PropertyDefinition& definition, SumRule& paramtype);

GSErrCode Sum_OneRule(const SumRule& rule);


#endif