//------------ kuvbur 2022 ------------
#include	"APIEnvir.h"
#include	"ACAPinc.h"
#include	"Helpers.hpp"
#include	"ResetProperty.hpp"

//--------------------------------------------------------------------------------------------------------------------------
// Сброс свойств
//--------------------------------------------------------------------------------------------------------------------------
bool ResetProperty(ParamDictValue& propertyParams) {
	GS::Array<API_PropertyDefinition> definitions_to_reset;
	for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = propertyParams.EnumeratePairs(); cIt != NULL; ++cIt) {
		ParamValue& param = *cIt->value;
		API_PropertyDefinition definition = param.definition;
		if (definition.description.Contains("Sync_reset")) definitions_to_reset.Push(definition);
	}
	if (definitions_to_reset.IsEmpty()) return false;
	return (ResetPropertyElement2Defult(definitions_to_reset) > 0);
}

//--------------------------------------------------------------------------------------------------------------------------
// Сброс свойств во всех БД файла и настройках по умолчанию
//--------------------------------------------------------------------------------------------------------------------------
UInt32 ResetPropertyElement2Defult(const GS::Array<API_PropertyDefinition>& definitions_to_reset) {
	if (definitions_to_reset.GetSize() > 0) return 0;
	DoneElemGuid doneelemguid; // словарь, куда будут попадать обработанные элементы
	UInt32 flag_reset = 0;
	GSErrCode	err = NoError;
	API_DatabaseID commandID = APIDb_GetCurrentDatabaseID;
	API_AttributeIndex layerCombIndex;

	// Сейчас будем переключаться между БД
	// Запомним номер текущей БД и комбинацию слоёв для восстановления по окончанию работы
	err = ACAPI_Environment(APIEnv_GetCurrLayerCombID, &layerCombIndex);
	if (err != NoError) { msg_rep("ResetPropertyElement2Defult", "APIEnv_GetCurrLayerCombID", err, APINULLGuid); }
	if (err == NoError) {
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
		if (err != NoError) { msg_rep("ResetPropertyElement2Defult", "APIDb_ChangeCurrentDatabaseID", err, APINULLGuid); }
		if (err == NoError) { err = ACAPI_Environment(APIEnv_ChangeCurrLayerCombID, &layerCombIndex); }
		if (err != NoError) { msg_rep("ResetPropertyElement2Defult", "APIEnv_ChangeCurrLayerCombID", err, APINULLGuid); }
	}
	if (!doneelemguid.IsEmpty()) {
		GS::UniString intString = GS::UniString::Printf(" %d", doneelemguid.GetSize());
		msg_rep("Reset property done - ", intString, NoError, APINULLGuid);
	}
	else {
		msg_rep("Reset property done", "", NoError, APINULLGuid);
	}
	return flag_reset;
}

//--------------------------------------------------------------------------------------------------------------------------
// Сброс свойств в выбранной БД
//--------------------------------------------------------------------------------------------------------------------------
UInt32 ResetElementsInDB(const API_DatabaseID commandID, const GS::Array<API_PropertyDefinition>& definitions_to_reset, API_AttributeIndex layerCombIndex, DoneElemGuid& doneelemguid) {
	UInt32 flag_reset = 0;
	GSErrCode	err = NoError;

	// Если чистим элементы в текущей БД - переключаться не нужно
	if (commandID == APIDb_GetCurrentDatabaseID) {
		if (layerCombIndex != 0) err = ACAPI_Environment(APIEnv_ChangeCurrLayerCombID, &layerCombIndex); // Устанавливаем комбинацию слоёв
		GS::Array<API_Guid>	guidArray;
		err = ACAPI_Element_GetElemList(API_ZombieElemID, &guidArray);
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
	err = ACAPI_Database(commandID, nullptr, &dbases); // Получаем список БД
	if (err != NoError) msg_rep("ResetElementsInDB", "ACAPI_Database", err, APINULLGuid);
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
					if (layerCombIndex != 0) err = ACAPI_Environment(APIEnv_ChangeCurrLayerCombID, &layerCombIndex); // Устанавливаем комбинацию слоёв
					GS::Array<API_Guid>	guidArray;
					err = ACAPI_Element_GetElemList(API_ZombieElemID, &guidArray);
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
					else {
						msg_rep("ResetElementsInDB", "ACAPI_Element_GetElemList_2", err, APINULLGuid);
					}
				}
			}
		}
	}
	return flag_reset;
}

//--------------------------------------------------------------------------------------------------------------------------
// Сброс свойств элемента
//--------------------------------------------------------------------------------------------------------------------------
GSErrCode ResetOneElemen(const API_Guid elemGuid, const GS::Array<API_PropertyDefinition>& definitions_to_reset) {
	GSErrCode	err = NoError;
	GS::Array<API_Property>  properties;
	GS::Array<API_Property>  properties_to_reset;
	err = ACAPI_Element_GetPropertyValues(elemGuid, definitions_to_reset, properties);
	if (err != NoError) msg_rep("ResetOneElemen", "ACAPI_Element_GetPropertyValues", err, elemGuid);
	if (err == NoError) {
		for (UInt32 i = 0; i < properties.GetSize(); i++) {
			API_Property property = properties.Get(i);

			// Сбрасываем только специальные значения
			if (!property.isDefault) {
				property.isDefault = true;
				property.value.variantStatus = API_VariantStatusNormal;
				properties_to_reset.Push(property);
			}
		}
		if (!properties_to_reset.IsEmpty()) {
			err = ACAPI_Element_SetProperties(elemGuid, properties_to_reset);
			if (err != NoError) {

				// попробуем разблокировать и повторить
				if (ReserveElement(elemGuid, err)) err = ACAPI_Element_SetProperties(elemGuid, properties_to_reset);
			}

			// Если не получилось - выведем ошибку.
			if (err != NoError) msg_rep("ResetOneElemen", "ACAPI_Element_SetProperties", err, elemGuid);
		}
		else {
			err = APIERR_MISSINGCODE;
		}
	}
	return err;
}

//--------------------------------------------------------------------------------------------------------------------------
// Сброс свойств в настройках по умолчанию
//--------------------------------------------------------------------------------------------------------------------------
UInt32 ResetElementsDefault(const GS::Array<API_PropertyDefinition>& definitions_to_reset) {
	UInt32 flag_reset = 0;

	// Элементы МЕР
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

	// Стандартные элементы
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

//--------------------------------------------------------------------------------------------------------------------------
// Сброс свойств в настройках по умолчанию для одного инструмента
//--------------------------------------------------------------------------------------------------------------------------
GSErrCode ResetOneElemenDefault(API_ElemTypeID typeId, const GS::Array<API_PropertyDefinition>& definitions_to_reset, int variationID) {
	GSErrCode	err = NoError;
	GS::Array<API_Property>  properties;
	GS::Array<API_Property>  properties_to_reset;
#ifdef AC_26
	API_ElemType type;
	type.typeID = typeId;
	type.variationID = static_cast<API_ElemVariationID>(variationID);
	err = ACAPI_Element_GetPropertyValuesOfDefaultElem(type, definitions_to_reset, properties);
#else
	err = ACAPI_Element_GetPropertyValuesOfDefaultElem(typeId, static_cast<API_ElemVariationID>(variationID), definitions_to_reset, properties);
#endif // AC_26
	if (err != NoError) msg_rep("ResetOneElemenDefault", "ACAPI_Element_GetPropertyValuesOfDefaultElem", err, APINULLGuid);
	if (err == NoError) {
		for (UInt32 i = 0; i < properties.GetSize(); i++) {
#if defined(AC_23)
			if (!properties[i].isDefault && properties[i].isEvaluated) {
#else
			if (!properties[i].isDefault && properties[i].status == API_Property_HasValue) {
#endif // AC_26
				properties[i].isDefault = true;
				properties_to_reset.Push(properties.Get(i));
			}
		}
		if (properties_to_reset.GetSize() > 0) {
#ifdef AC_26
			err = ACAPI_Element_SetPropertiesOfDefaultElem(type, properties);
#else
			err = ACAPI_Element_SetPropertiesOfDefaultElem(typeId, static_cast<API_ElemVariationID>(variationID), properties_to_reset);
#endif // AC_26
			if (err != NoError) msg_rep("ResetOneElemenDefault", "ACAPI_Element_SetPropertiesOfDefaultElem", err, APINULLGuid);
		}
		else {
			err = APIERR_MISSINGCODE;
		}
	}
	return err;
}