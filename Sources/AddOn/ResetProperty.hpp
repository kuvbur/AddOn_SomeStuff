//------------ kuvbur 2022 ------------
#if !defined (RESET_HPP)
#define	RESET_HPP
#ifdef AC_25
#include	"APICommon25.h"
#endif // AC_25
#ifdef AC_26
#include	"APICommon26.h"
#endif // AC_26
#ifdef AC_27
#include	"APICommon27.h"
#endif // AC_27
#include	"DG.h"
#include	"SyncSettings.hpp"

typedef GS::HashTable<API_Guid, bool> DoneElemGuid;

//--------------------------------------------------------------------------------------------------------------------------
// Сброс свойств
//--------------------------------------------------------------------------------------------------------------------------
bool ResetProperty(ParamDictValue& propertyParams);

//--------------------------------------------------------------------------------------------------------------------------
// Сброс свойств во всех БД файла и настройках по умолчанию
//--------------------------------------------------------------------------------------------------------------------------
UInt32 ResetPropertyElement2Defult(const GS::Array<API_PropertyDefinition>& definitions_to_reset);

UInt32 ResetElementsInDB(const API_DatabaseID commandID, const GS::Array<API_PropertyDefinition>& definitions_to_reset, API_AttributeIndex layerCombIndex, DoneElemGuid& doneelemguid);

GSErrCode ResetOneElemen(const API_Guid elemGuid, const GS::Array<API_PropertyDefinition>& definitions_to_reset);

UInt32 ResetElementsDefault(const GS::Array<API_PropertyDefinition>& definitions_to_reset);

GSErrCode ResetOneElemenDefault(API_ElemTypeID typeId, const GS::Array<API_PropertyDefinition>& definitions_to_reset, int variationID);

#endif