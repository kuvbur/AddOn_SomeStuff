#pragma once
#if !defined (SUMM_HPP)
#define	SUMM_HPP
//#ifdef ServerMainVers_2500
//#include	"APICommon25.h"
//#endif // AC_25
//#ifdef ServerMainVers_2600
//#include	"APICommon26.h"
//#endif // AC_26
#include	"DG.h"

static const short TextSum = 1;
static const short NumSum = 2;

typedef struct {
	API_Guid		guid = APINULLGuid;
	std::string		criteria = "";
	std::string		string_value = "";
	double			num_value = 0;
} SumElement;

typedef struct {
	GS::UniString position = "";
	GS::UniString value = "";
	GS::UniString criteria = "";
	std::string					delimetr = "";
	std::string					ignore_val = "";
	short						sum_type = 0;
	GS::Array <SumElement>		elemts;
} SumRule;

typedef GS::HashTable<API_Guid, SumRule> SumRules;

GSErrCode SumSelected(void);

bool Sum_GetElement(const API_Guid& elemGuid, ParamDictValue& propertyParams, ParamDictElement& paramToReadelem, SumRules& rules);

bool Sum_Rule(const API_Guid& elemGuid, const API_PropertyDefinition& definition, ParamDictValue& propertyParams, SumRule& paramtype);

GSErrCode Sum_OneRule(const SumRule& rule);

#endif