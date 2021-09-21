
#if !defined (RENUM_HPP)
#define	RENUM_HPP
#include "APICommon.h"
#include "DG.h"

typedef struct {
	API_Guid		guid;
	std::string	val;
	GS::UniString	criteria;
	UInt32	state;
} RenumElement;

typedef struct {
	bool						state;
	API_PropertyDefinition		position;
	API_PropertyDefinition		criteria;
	API_PropertyDefinition		sort;
	API_PropertyDefinition		delimetr;
	GS::Array <RenumElement>	elemts;
} RenumRule;

typedef struct {
	GS::Array <API_Guid>	guid;
} SortGUID;

typedef GS::HashTable<API_Guid, RenumRule> Rules;

GSErrCode ReNum_Selected(void);
bool ReNumRule(const API_Guid& elemGuid, const GS::UniString& description_string, RenumRule& paramtype);
UInt32 ReNumGetRule(const API_PropertyDefinition definitionflag, const API_Guid& elemGuid, API_PropertyDefinition& propertdefyrule);
GSErrCode ReNum_GetElement(const API_Guid& elemGuid, Rules& rules);
GSErrCode ReNum_OneRule(const RenumRule& rule);
GSErrCode ReNum_GetElement(const API_Guid& elemGuid, Rules& rules);

#endif