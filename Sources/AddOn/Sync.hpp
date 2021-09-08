
#if !defined (SYNC_HPP)
#define	SYNC_HPP
#include "APICommon.h"
#include "DG.h"

bool CheckElementType(const API_ElemTypeID& elementType);
void SyncSelected(void);
void SyncAndMonAll(void);
bool SyncByType(const API_ElemTypeID& elementType);
GSErrCode SyncParamAndProp(const API_Guid& elemGuid, const GS::UniString& paramName, const int& syncdirection, API_Property& property);
bool SyncState(const API_Guid& elemGuid, const GS::Array<API_PropertyDefinition> definitions);
void SyncData(const API_Guid& elemGuid);
bool SyncString(GS::UniString& description_string, GS::UniString& paramName, int& synctype, int& syncdirection);
GSErrCode SyncParamAndProp(const API_Guid& elemGuid, const GS::UniString& paramName, const int& syncdirection, API_Property& property);
GSErrCode SyncPropAndProp(const API_Guid& elemGuid, API_Property& property, const GS::UniString& paramName);
#endif