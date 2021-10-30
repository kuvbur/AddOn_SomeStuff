
#if !defined (SUMM_HPP)
#define	SUMM_HPP
#include "APICommon.h"
#include "DG.h"

typedef struct {
	API_Guid		guid;
	std::string		criteria;
	std::string		val;
} SumElement;

typedef struct {
	API_PropertyDefinition		value;
	API_PropertyDefinition		criteria;
	std::string					delimetr;
	GS::Array <SumElement>		elemts;
} SumRule;

typedef GS::HashTable<API_Guid, SumRule> SumRules;

GSErrCode SumSelected(void);

GSErrCode Sum_GetElement(const API_Guid& elemGuid, SumRules& rules);


#endif