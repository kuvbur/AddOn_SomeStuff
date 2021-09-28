
#if !defined (SYNC_HPP)
#define	SYNC_HPP
#include "APICommon.h"
#include "DG.h"

typedef struct {
	GS::UniString paramName;
	GS::UniString ignoreval;
	int synctype;
	int syncdirection;
} SyncRule;

bool CheckElementType(const API_ElemTypeID& elementType);
void SyncSelected(void);
void SyncAndMonAll(void);
bool SyncByType(const API_ElemTypeID& elementType);
GSErrCode SyncParamAndProp(const API_Guid& elemGuid, const  SyncRule& syncRule, API_Property& property);
bool SyncState(const API_Guid& elemGuid, const GS::Array<API_PropertyDefinition> definitions);
void SyncData(const API_Guid& elemGuid);
bool SyncString(GS::UniString& description_string, SyncRule& syncRule);
GSErrCode SyncParamAndProp(const API_Guid& elemGuid, const  SyncRule& syncRule, API_Property& property);
GSErrCode SyncPropAndProp(const API_Guid& elemGuid, API_Property& property, const  SyncRule& syncRule);
void SyncReservation(short type, const API_Guid objectId);
#endif