#include	"APIEnvir.h"
#include	"ACAPinc.h"
#include	"Helpers.hpp"
#include	"ResetProperty.hpp"

//--------------------------------------------------------------------------------------------------------------------------
// Ищет свойство со значение "Sync_reset"
// Если оно есть - значение свойства нужно сборосить к значению по умолчанию
//--------------------------------------------------------------------------------------------------------------------------
bool ResetAllProperty() {
	GSErrCode	err = NoError;
	UInt32 flag_reset = 0;
	bool skip_sinc = false;
	GS::Array<API_PropertyDefinition> definitions;
	GS::Array<API_PropertyDefinition> definitions_to_reset;
	err = ACAPI_Property_GetPropertyDefinitions(APINULLGuid, definitions);
	if (err != NoError) msg_rep("ResetAllProperty", "ACAPI_Property_GetPropertyDefinitions All", err, APINULLGuid);
	if (err == NoError) {
		GS::Array<API_Guid>	guidArray;
		for (UInt32 j = 0; j < definitions.GetSize(); j++) {
			if (definitions[j].description.Contains("Sync_reset")) definitions_to_reset.Push(definitions.Get(j));
		}
	}
	if (definitions_to_reset.GetSize()>0) {
		flag_reset = flag_reset + ResetPropertyElement2Defult(definitions_to_reset);
		bool skip_sinc = true;
	}
	return skip_sinc;
}

UInt32 ResetPropertyElement2Defult(const GS::Array<API_PropertyDefinition>& definitions_to_reset) {
	DoneElemGuid doneelemguid;
	UInt32 flag_reset = 0;
	GSErrCode	err = NoError;
	API_DatabaseID commandID = APIDb_GetCurrentDatabaseID;
	API_AttributeIndex layerCombIndex;
	err = ACAPI_Environment(APIEnv_GetCurrLayerCombID,&layerCombIndex);
	flag_reset = flag_reset + ResetElementsDefault(definitions_to_reset);
	flag_reset = flag_reset + ResetElementsInDB(APIDb_GetCurrentDatabaseID, definitions_to_reset, layerCombIndex, doneelemguid);
	flag_reset = flag_reset + ResetElementsInDB(APIDb_GetElevationDatabasesID, definitions_to_reset, layerCombIndex, doneelemguid);
	flag_reset = flag_reset + ResetElementsInDB(APIDb_GetDetailDatabasesID, definitions_to_reset, layerCombIndex, doneelemguid);
	flag_reset = flag_reset + ResetElementsInDB(APIDb_GetWorksheetDatabasesID, definitions_to_reset, layerCombIndex, doneelemguid);
	flag_reset = flag_reset + ResetElementsInDB(APIDb_GetDocumentFrom3DDatabasesID, definitions_to_reset, layerCombIndex, doneelemguid);
	flag_reset = flag_reset + ResetElementsInDB(APIDb_GetLayoutDatabasesID, definitions_to_reset, layerCombIndex, doneelemguid);
	flag_reset = flag_reset + ResetElementsInDB(APIDb_GetMasterLayoutDatabasesID, definitions_to_reset, layerCombIndex, doneelemguid);
	flag_reset = flag_reset + ResetElementsInDB(APIDb_GetSectionDatabasesID, definitions_to_reset, layerCombIndex, doneelemguid);
	flag_reset = flag_reset + ResetElementsInDB(APIDb_GetElevationDatabasesID, definitions_to_reset, layerCombIndex, doneelemguid);
	flag_reset = flag_reset + ResetElementsInDB(APIDb_GetInteriorElevationDatabasesID, definitions_to_reset, layerCombIndex, doneelemguid);
	err = ACAPI_Database(APIDb_ChangeCurrentDatabaseID, &commandID);
	err = ACAPI_Environment(APIEnv_ChangeCurrLayerCombID, &layerCombIndex);
	return flag_reset;
}

UInt32 ResetElementsInDB(const API_DatabaseID commandID, const GS::Array<API_PropertyDefinition>& definitions_to_reset, API_AttributeIndex layerCombIndex, DoneElemGuid& doneelemguid) {
	UInt32 flag_reset = 0;
	GSErrCode	err = NoError;
	if (commandID == APIDb_GetCurrentDatabaseID) {
		GS::Array<API_Guid>	guidArray;
		err = ACAPI_Element_GetElemList(API_ZombieElemID, &guidArray);
		err = ACAPI_Element_Tool(guidArray, APITool_Unlock, nullptr);
		if (err != NoError) msg_rep("ResetElementsInDB", "ACAPI_Element_GetElemList_1", err, APINULLGuid);
		if (err == NoError) {
			for (UInt32 i = 0; i < guidArray.GetSize(); i++) {
				if (!doneelemguid.ContainsKey(guidArray.Get(i))) {
					err = ResetOneElemen(guidArray.Get(i), definitions_to_reset);
					if (err == NoError) {
						flag_reset++;
						doneelemguid.Add(guidArray.Get(i), true);
					}
				}
			}
		}
		return flag_reset;
	}
	GS::Array<API_DatabaseUnId>	dbases;
	err = ACAPI_Database(commandID, nullptr, &dbases);
	if (err == NoError) {
		for (const auto& dbUnId : dbases) {
			API_DatabaseInfo dbPars = {};
			dbPars.databaseUnId = dbUnId;
			err = ACAPI_Database(APIDb_GetDatabaseInfoID, &dbPars);
			if (err != NoError) msg_rep("ResetElementsInDB", "APIDb_GetDatabaseInfoID", err, APINULLGuid);
			if (err == NoError) {
				err = ACAPI_Database(APIDb_ChangeCurrentDatabaseID, &dbPars);
				if (err != NoError) msg_rep("ResetElementsInDB", "APIDb_ChangeCurrentDatabaseID", err, APINULLGuid);
				if (err == NoError) {
					err = ACAPI_Environment(APIEnv_ChangeCurrLayerCombID,&layerCombIndex);
					GS::Array<API_Guid>	guidArray;
					err = ACAPI_Element_GetElemList(API_ZombieElemID, &guidArray);
					err = ACAPI_Element_Tool(guidArray, APITool_Unlock, nullptr);
					if (err != NoError) msg_rep("ResetElementsInDB", "ACAPI_Element_GetElemList_2", err, APINULLGuid);
					if (err == NoError) {
						for (UInt32 i = 0; i < guidArray.GetSize(); i++) {
							if (!doneelemguid.ContainsKey(guidArray.Get(i))) {
								err = ResetOneElemen(guidArray.Get(i), definitions_to_reset);
								if (err == NoError) {
									flag_reset++;
									doneelemguid.Add(guidArray.Get(i), true);
								}
							}
						}
					}
				}
			}
		}
	}
	return flag_reset;
}

GSErrCode ResetOneElemen(const API_Guid elemGuid, const GS::Array<API_PropertyDefinition>& definitions_to_reset) {
	GSErrCode	err = NoError;
	GS::Array<API_Property>  properties;
	GS::Array<API_Property>  properties_to_reset;
	err = ACAPI_Element_GetPropertyValues(elemGuid, definitions_to_reset, properties);
	if (err != NoError) msg_rep("ResetOneElemen", "ACAPI_Element_GetPropertyValues", err, elemGuid);
	if (err == NoError) {
		for (UInt32 i = 0; i < properties.GetSize(); i++) {
			if (!properties[i].isDefault && properties[i].status == API_Property_HasValue) {
				properties[i].isDefault = true;
				properties_to_reset.Push(properties.Get(i));
			}
		}
		if (properties_to_reset.GetSize() > 0) {
			err = ACAPI_Element_SetProperties(elemGuid, properties_to_reset);
			if (err != NoError) msg_rep("ResetOneElemen", "ACAPI_Element_SetProperties", err, elemGuid);
		}
		else {
			err = APIERR_MISSINGCODE;
		}
	}
	return err;
}

UInt32 ResetElementsDefault(const GS::Array<API_PropertyDefinition>& definitions_to_reset) {
	UInt32 flag_reset = 0;
	GSErrCode	err = NoError;
	flag_reset = flag_reset + (ResetOneElemenDefault(API_ObjectID, definitions_to_reset, 1146245920) == NoError);
	flag_reset = flag_reset + (ResetOneElemenDefault(API_ObjectID, definitions_to_reset, 1145194016) == NoError);
	flag_reset = flag_reset + (ResetOneElemenDefault(API_ObjectID, definitions_to_reset, 1145459744) == NoError);
	flag_reset = flag_reset + (ResetOneElemenDefault(API_ObjectID, definitions_to_reset, 1146374688) == NoError);
	flag_reset = flag_reset + (ResetOneElemenDefault(API_ObjectID, definitions_to_reset, 1146373664) == NoError);
	flag_reset = flag_reset + (ResetOneElemenDefault(API_ObjectID, definitions_to_reset, 1178935328) == NoError);
	flag_reset = flag_reset + (ResetOneElemenDefault(API_ObjectID, definitions_to_reset, 1145654304) == NoError);
	flag_reset = flag_reset + (ResetOneElemenDefault(API_ObjectID, definitions_to_reset, 1347572512) == NoError);
	flag_reset = flag_reset + (ResetOneElemenDefault(API_ObjectID, definitions_to_reset, 1346520608) == NoError);
	flag_reset = flag_reset + (ResetOneElemenDefault(API_ObjectID, definitions_to_reset, 1346786336) == NoError);
	flag_reset = flag_reset + (ResetOneElemenDefault(API_ObjectID, definitions_to_reset, 1347897888) == NoError);
	flag_reset = flag_reset + (ResetOneElemenDefault(API_ObjectID, definitions_to_reset, 1347702602) == NoError);
	flag_reset = flag_reset + (ResetOneElemenDefault(API_ObjectID, definitions_to_reset, 1346980896) == NoError);
	flag_reset = flag_reset + (ResetOneElemenDefault(API_ObjectID, definitions_to_reset, 1346782752) == NoError);
	flag_reset = flag_reset + (ResetOneElemenDefault(API_ObjectID, definitions_to_reset, 1129468704) == NoError);
	flag_reset = flag_reset + (ResetOneElemenDefault(API_ObjectID, definitions_to_reset, 1128416800) == NoError);
	flag_reset = flag_reset + (ResetOneElemenDefault(API_ObjectID, definitions_to_reset, 1129597472) == NoError);
	flag_reset = flag_reset + (ResetOneElemenDefault(API_ObjectID, definitions_to_reset, 1129596448) == NoError);
	flag_reset = flag_reset + (ResetOneElemenDefault(API_ObjectID, definitions_to_reset, 1128678944) == NoError);
	flag_reset = flag_reset + (ResetOneElemenDefault(API_ObjectID, definitions_to_reset, 1178869792) == NoError);
	flag_reset = flag_reset + (ResetOneElemenDefault(API_ObjectID, definitions_to_reset, 1196574028) == NoError);
	flag_reset = flag_reset + (ResetOneElemenDefault(API_ObjectID, definitions_to_reset, 1146571296) == NoError);
	flag_reset = flag_reset + (ResetOneElemenDefault(API_ObjectID, definitions_to_reset, 0) == NoError);
	flag_reset = flag_reset + (ResetOneElemenDefault(API_WallID, definitions_to_reset, 0) == NoError);
	flag_reset = flag_reset + (ResetOneElemenDefault(API_ColumnID, definitions_to_reset, 0) == NoError);
	flag_reset = flag_reset + (ResetOneElemenDefault(API_BeamID, definitions_to_reset, 0) == NoError);
	flag_reset = flag_reset + (ResetOneElemenDefault(API_WindowID, definitions_to_reset, 0) == NoError);
	flag_reset = flag_reset + (ResetOneElemenDefault(API_DoorID, definitions_to_reset, 0) == NoError);
	flag_reset = flag_reset + (ResetOneElemenDefault(API_ObjectID, definitions_to_reset, 0) == NoError);
	flag_reset = flag_reset + (ResetOneElemenDefault(API_LampID, definitions_to_reset, 0) == NoError);
	flag_reset = flag_reset + (ResetOneElemenDefault(API_SlabID, definitions_to_reset, 0) == NoError);
	flag_reset = flag_reset + (ResetOneElemenDefault(API_RoofID, definitions_to_reset, 0) == NoError);
	flag_reset = flag_reset + (ResetOneElemenDefault(API_MeshID, definitions_to_reset, 0) == NoError);
	flag_reset = flag_reset + (ResetOneElemenDefault(API_ZoneID, definitions_to_reset, 0) == NoError);
	flag_reset = flag_reset + (ResetOneElemenDefault(API_CurtainWallID, definitions_to_reset, 0) == NoError);
	flag_reset = flag_reset + (ResetOneElemenDefault(API_CurtainWallSegmentID, definitions_to_reset, 0) == NoError);
	flag_reset = flag_reset + (ResetOneElemenDefault(API_CurtainWallFrameID, definitions_to_reset, 0) == NoError);
	flag_reset = flag_reset + (ResetOneElemenDefault(API_CurtainWallPanelID, definitions_to_reset, 0) == NoError);
	flag_reset = flag_reset + (ResetOneElemenDefault(API_CurtainWallJunctionID, definitions_to_reset, 0) == NoError);
	flag_reset = flag_reset + (ResetOneElemenDefault(API_CurtainWallAccessoryID, definitions_to_reset, 0) == NoError);
	flag_reset = flag_reset + (ResetOneElemenDefault(API_ShellID, definitions_to_reset, 0) == NoError);
	flag_reset = flag_reset + (ResetOneElemenDefault(API_SkylightID, definitions_to_reset, 0) == NoError);
	flag_reset = flag_reset + (ResetOneElemenDefault(API_MorphID, definitions_to_reset, 0) == NoError);
	flag_reset = flag_reset + (ResetOneElemenDefault(API_StairID, definitions_to_reset, 0) == NoError);
	flag_reset = flag_reset + (ResetOneElemenDefault(API_RiserID, definitions_to_reset, 0) == NoError);
	flag_reset = flag_reset + (ResetOneElemenDefault(API_TreadID, definitions_to_reset, 0) == NoError);
	flag_reset = flag_reset + (ResetOneElemenDefault(API_StairStructureID, definitions_to_reset, 0) == NoError);
	flag_reset = flag_reset + (ResetOneElemenDefault(API_RailingID, definitions_to_reset, 0) == NoError);
	flag_reset = flag_reset + (ResetOneElemenDefault(API_RailingToprailID, definitions_to_reset, 0) == NoError);
	flag_reset = flag_reset + (ResetOneElemenDefault(API_RailingHandrailID, definitions_to_reset, 0) == NoError);
	flag_reset = flag_reset + (ResetOneElemenDefault(API_RailingRailID, definitions_to_reset, 0) == NoError);
	flag_reset = flag_reset + (ResetOneElemenDefault(API_RailingPostID, definitions_to_reset, 0) == NoError);
	flag_reset = flag_reset + (ResetOneElemenDefault(API_RailingInnerPostID, definitions_to_reset, 0) == NoError);
	flag_reset = flag_reset + (ResetOneElemenDefault(API_RailingBalusterID, definitions_to_reset, 0) == NoError);
	flag_reset = flag_reset + (ResetOneElemenDefault(API_RailingPanelID, definitions_to_reset, 0) == NoError);
	flag_reset = flag_reset + (ResetOneElemenDefault(API_RailingSegmentID, definitions_to_reset, 0) == NoError);
	flag_reset = flag_reset + (ResetOneElemenDefault(API_RailingNodeID, definitions_to_reset, 0) == NoError);
	flag_reset = flag_reset + (ResetOneElemenDefault(API_RailingBalusterSetID, definitions_to_reset, 0) == NoError);
	flag_reset = flag_reset + (ResetOneElemenDefault(API_RailingPatternID, definitions_to_reset, 0) == NoError);
	flag_reset = flag_reset + (ResetOneElemenDefault(API_RailingToprailEndID, definitions_to_reset, 0) == NoError);
	flag_reset = flag_reset + (ResetOneElemenDefault(API_RailingHandrailEndID, definitions_to_reset, 0) == NoError);
	flag_reset = flag_reset + (ResetOneElemenDefault(API_RailingRailEndID, definitions_to_reset, 0) == NoError);
	flag_reset = flag_reset + (ResetOneElemenDefault(API_RailingToprailConnectionID, definitions_to_reset, 0) == NoError);
	flag_reset = flag_reset + (ResetOneElemenDefault(API_RailingHandrailConnectionID, definitions_to_reset, 0) == NoError);
	flag_reset = flag_reset + (ResetOneElemenDefault(API_RailingRailConnectionID, definitions_to_reset, 0) == NoError);
	flag_reset = flag_reset + (ResetOneElemenDefault(API_RailingEndFinishID, definitions_to_reset, 0) == NoError);
	flag_reset = flag_reset + (ResetOneElemenDefault(API_BeamSegmentID, definitions_to_reset, 0) == NoError);
	flag_reset = flag_reset + (ResetOneElemenDefault(API_ColumnSegmentID, definitions_to_reset, 0) == NoError);
	return flag_reset;
}


GSErrCode ResetOneElemenDefault(API_ElemTypeID typeId, const GS::Array<API_PropertyDefinition>& definitions_to_reset, int variationID)
{
	GSErrCode	err = NoError;
	GS::Array<API_Property>  properties;
	GS::Array<API_Property>  properties_to_reset;
	err = ACAPI_Element_GetPropertyValuesOfDefaultElem(typeId, static_cast<API_ElemVariationID>(variationID), definitions_to_reset, properties);
	if (err != NoError) msg_rep("ResetOneElemenDefault", "ACAPI_Element_GetPropertyValuesOfDefaultElem", err, APINULLGuid);
	if (err == NoError) {
		for (UInt32 i = 0; i < properties.GetSize(); i++) {
			if (!properties[i].isDefault && properties[i].status == API_Property_HasValue) {
				properties[i].isDefault = true;
				properties_to_reset.Push(properties.Get(i));
			}
		}
		if (properties_to_reset.GetSize() > 0) {
			err = ACAPI_Element_SetPropertiesOfDefaultElem(typeId, static_cast<API_ElemVariationID>(variationID), properties_to_reset);
			if (err != NoError) msg_rep("ResetOneElemenDefault", "ACAPI_Element_SetPropertiesOfDefaultElem", err, APINULLGuid);
		}
		else {
			err = APIERR_MISSINGCODE;
		}
	}
	return err;
}