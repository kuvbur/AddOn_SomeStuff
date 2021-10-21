
#if !defined (SYNC_HPP)
#define	SYNC_HPP
#include "APICommon.h"
#include "DG.h"

typedef struct {
	GS::UniString paramName;
	GS::Array<GS::UniString> ignorevals;
	int synctype;
	int syncdirection;
} SyncRule;

bool CheckElementType(const API_ElemTypeID& elementType);
void SyncSelected(void);
void SyncAndMonAll(void);
bool SyncByType(const API_ElemTypeID& elementType);
GSErrCode SyncParamAndProp(const API_Guid& elemGuid, const  SyncRule& syncRule, API_Property& property);
bool SyncState(const API_Guid& elemGuid, const GS::Array<API_PropertyDefinition> definitions);
GSErrCode SyncRelationsToWindow(const API_Guid& elemGuid, bool& issuncwall);
GSErrCode SyncRelationsToDoor(const API_Guid& elemGuid, bool& issuncwall);
void SyncRelationsElement(const API_Guid& elemGuid);
void SyncData(const API_Guid& elemGuid);
GSErrCode SyncOneProperty(const API_Guid& elemGuid, const API_ElemTypeID elementType, API_PropertyDefinition definition);
bool SyncOneRule(const API_Guid& elemGuid, const API_ElemTypeID elementType, API_Property property, SyncRule syncRule);

bool SyncCheckIgnoreVal(const SyncRule& syncRule, const GS::UniString& val);
bool SyncCheckIgnoreVal(const SyncRule& syncRule, const API_Property& property);

bool SyncString(GS::UniString& description_string, GS::Array <SyncRule>& syncRule);
GSErrCode SyncParamAndProp(const API_Guid& elemGuid, const  SyncRule& syncRule, API_Property& property);
GSErrCode SyncPropAndProp(const API_Guid& elemGuid, const SyncRule & syncRule, API_Property& property);
void SyncReservation(short type, const API_Guid objectId);
#endif