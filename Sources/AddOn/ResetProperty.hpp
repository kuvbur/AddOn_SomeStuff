
#if !defined (RESET_HPP)
#define	RESET_HPP

#include	"APICommon.h"
#include	"DG.h"
#include	"SyncSettings.hpp"

typedef GS::HashTable<API_Guid, bool> DoneElemGuid;

bool ResetAllProperty();

UInt32 ResetPropertyElement2Defult(const GS::Array<API_PropertyDefinition>& definitions_to_reset);

UInt32 ResetElementsInDB(const API_DatabaseID commandID, const GS::Array<API_PropertyDefinition>& definitions_to_reset, API_AttributeIndex layerCombIndex, DoneElemGuid& doneelemguid);

GSErrCode ResetOneElemen(const API_Guid elemGuid, const GS::Array<API_PropertyDefinition>& definitions_to_reset);

UInt32 ResetElementsDefault(const GS::Array<API_PropertyDefinition>& definitions_to_reset);

GSErrCode ResetOneElemenDefault(API_ElemTypeID typeId, const GS::Array<API_PropertyDefinition>& definitions_to_reset, int variationID);

#endif