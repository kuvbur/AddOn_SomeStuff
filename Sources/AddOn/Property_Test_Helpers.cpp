// *****************************************************************************
// File:			Property_Test_Helper.cpp
// Description:		Property_Test add-on helper macros and functions
// Project:			APITools/Property_Test
// Namespace:		-
// Contact person:	CSAT
// *****************************************************************************
#include	"APIEnvir.h"
#include	"ACAPinc.h"
#include "Property_Test_Helpers.hpp"

// -----------------------------------------------------------------------------
// Парсит описание свойства
// Результат
//	имя параметра (свойства)
//	тип синхронизации (читаем из параметра GDL - 1, из свойства - 2)
//	направление синхронизации для работы с GDL (читаем из параметра - 1, записываем в параметр - 2)
// -----------------------------------------------------------------------------
bool SyncString(GS::UniString& description_string, GS::UniString& paramName, int& synctype, int& syncdirection) {
	bool flag_sync = false;
	if (description_string.IsEmpty()) {
		return flag_sync;
	}
	if (description_string.Contains("Sync_") && description_string.Contains("{") && description_string.Contains("}")) {
		flag_sync = true;
		synctype = 1;
		syncdirection = 2;
		if (description_string.Contains("Sync_from")) syncdirection = 1;
		if (description_string.Contains("Property:")) {
			synctype = 2;
			description_string.ReplaceAll("Property:", "");
		}
		paramName = description_string.GetSubstring('{', '}', 0);
	}
	return flag_sync;
}

GSErrCode DefultSyncSettings(SyncPrefs& prefsData) {
	BNZeroMemory(&prefsData, sizeof(SyncPrefs));
	prefsData.version = CURR_ADDON_VERS;
	prefsData.syncNew = false;
	prefsData.syncMon = false;
	prefsData.wallS = true;
	prefsData.widoS = true;
	prefsData.objS = true;
	GSErrCode err = ACAPI_SetPreferences(CURR_ADDON_VERS, sizeof(SyncPrefs), (GSPtr) &prefsData);
	return err;
}

void GetSyncSettings(SyncPrefs& prefsData)
{
	Int32			version;
	GSSize			nBytes;
	GSErrCode err = NoError;
	unsigned short	platformSign = GS::Act_Platform_Sign;

	err = ACAPI_GetPreferences_Platform(&version, &nBytes, NULL, NULL);
	if (version == CURR_ADDON_VERS) {
		err = ACAPI_GetPreferences_Platform(&version, &nBytes, (GSPtr)&prefsData, &platformSign);
		if (platformSign != GS::Act_Platform_Sign) {
				GS::PlatformSign	inplatform = (GS::PlatformSign)platformSign;
				IVLong(inplatform, &prefsData.version);
				IVBool(inplatform, &prefsData.syncNew);
				IVBool(inplatform, &prefsData.syncMon);
				IVBool(inplatform, &prefsData.wallS);
				IVBool(inplatform, &prefsData.widoS);
				IVBool(inplatform, &prefsData.objS);
			}
	}
	else {
		err = DefultSyncSettings(prefsData);
	}
}

void	CheckACMenuItem(short itemInd, bool checked)
{
	API_MenuItemRef itemRef;
	GSFlags         itemFlags;

	BNZeroMemory(&itemRef, sizeof(API_MenuItemRef));
	itemRef.menuResID = 32500;
	itemRef.itemIndex = itemInd;

	itemFlags = 0;
	ACAPI_Interface(APIIo_GetMenuItemFlagsID, &itemRef, &itemFlags);
	if (checked)
		itemFlags |= API_MenuItemChecked;
	else
		itemFlags &= ~API_MenuItemChecked;
	ACAPI_Interface(APIIo_SetMenuItemFlagsID, &itemRef, &itemFlags);
	return;
}

// -----------------------------------------------------------------------------
// Получить массив Guid выбранных элементов
// -----------------------------------------------------------------------------
static GS::Array<API_Guid>	GetSelectedElements(bool assertIfNoSel /* = true*/, bool onlyEditable /*= true*/)
{
	GSErrCode            err;
	API_SelectionInfo    selectionInfo;
	GS::Array<API_Neig>  selNeigs;
	err = ACAPI_Selection_Get(&selectionInfo, &selNeigs, onlyEditable);
	BMKillHandle((GSHandle*)&selectionInfo.marquee.coords);
	if (err == APIERR_NOSEL || selectionInfo.typeID == API_SelEmpty) {
		if (assertIfNoSel) {
			DGAlert(DG_ERROR, "Error", "Сначала выберите элементы!", "", "Ok");
		}
	}
	if (err != NoError) {
		return GS::Array<API_Guid>();
	}
	GS::Array<API_Guid> guidArray;
	for (const API_Neig& neig : selNeigs) {
		guidArray.Push(neig.guid);
	}
	return guidArray;
}


// -----------------------------------------------------------------------------
// Вызов функции для выбранных элементов
//	(функция должна принимать в качетве аргумента API_Guid
// -----------------------------------------------------------------------------
void CallOnSelectedElem(void (*function)(const API_Guid&), bool assertIfNoSel /* = true*/, bool onlyEditable /* = true*/)
{
	GS::UniString	title("Sync Selected");
	Int32 nLib = 0;
	ACAPI_Interface(APIIo_InitProcessWindowID, &title, &nLib);
	GS::Array<API_Guid> guidArray = GetSelectedElements(assertIfNoSel, onlyEditable);
	if (!guidArray.IsEmpty()) {
		for (UInt32 i = 0; i < guidArray.GetSize(); i++) {
			ACAPI_Interface(APIIo_SetProcessValueID, &i, nullptr);
			function(guidArray[i]);
			if (ACAPI_Interface(APIIo_IsProcessCanceledID, nullptr, nullptr)) {
				return;
			}
		}
	}
	else if (!assertIfNoSel) {
		function(APINULLGuid);
	}
}

// -----------------------------------------------------------------------------
// Получение типа объекта по его API_Guid
// -----------------------------------------------------------------------------
GSErrCode GetTypeByGUID(const API_Guid& elemGuid, API_ElemTypeID& elementType) {
	GSErrCode		err = NoError;
	API_Elem_Head elementHead;
	BNZeroMemory(&elementHead, sizeof(API_Elem_Head));
	elementHead.guid = elemGuid;
	err = ACAPI_Element_GetHeader(&elementHead);
	elementType = elementHead.typeID;
	return err;
}

bool	GetElementTypeString(API_ElemTypeID typeID, char* elemStr)
{
	GS::UniString	ustr;
	GSErrCode	err = ACAPI_Goodies(APIAny_GetElemTypeNameID, (void*)typeID, &ustr);
	if (err == NoError) {
		CHTruncate(ustr.ToCStr(), elemStr, ELEMSTR_LEN - 1);
		return true;
	}
	return false;
}

/*----------------------------------------------------------------------------**
	Lists all the visible properties (definition name and value) of an element
**----------------------------------------------------------------------------*/
GSErrCode GetPropertyByGuid(const API_Guid& elemGuid, GS::Array<API_Property>& properties) {
	GSErrCode		err = NoError;
	GS::Array<API_PropertyDefinition> definitions;
	err = ACAPI_Element_GetPropertyDefinitions(elemGuid, API_PropertyDefinitionFilter_UserDefined, definitions);
	if (!err) {
		GS::Array<API_Guid> propertyDefinitionList;
		for (UInt32 i = 0; i < definitions.GetSize(); i++) {
			if (ACAPI_Element_IsPropertyDefinitionVisible(elemGuid, definitions[i].guid)) {
				propertyDefinitionList.Push(definitions[i].guid);
			}
		}
		err = ACAPI_Element_GetPropertyValuesByGuid(elemGuid, propertyDefinitionList, properties);
	}
	return err;
}

// -----------------------------------------------------------------------------
// Получить значение параметра с конвертацие типа данных
// -----------------------------------------------------------------------------
bool GetLibParam(const API_Guid& elemGuid, const GS::UniString& paramName, GS::UniString& param_string, GS::Int32& param_int, bool& param_bool, double& param_real)
{
	API_AddParType nthParameter;
	BNZeroMemory(&nthParameter, sizeof(API_AddParType));
	GSErrCode		err = NoError;
	API_ElementMemo  memo;
	API_Element element = {};
	element.header.guid = elemGuid;
	err = ACAPI_Element_Get(&element);
	Int32 lib_idx = element.object.libInd;
	API_LibPart libPart;
	BNZeroMemory(&libPart, sizeof(API_LibPart));
	libPart.index = lib_idx;
	err = ACAPI_LibPart_Get(&libPart);
	bool flag_find = false;
	if (!err)
	{
		if (err == NoError && element.header.hasMemo)
		{
			BNZeroMemory(&memo, sizeof(API_ElementMemo));
			err = ACAPI_Element_GetMemo(element.header.guid, &memo, APIMemoMask_AddPars);
			if (err == NoError)
			{
				UInt32 totalParams = BMGetHandleSize((GSConstHandle)memo.params) / sizeof(API_AddParType);  // number of parameters = handlesize / size of single handle
				for (UInt32 i = 0; i < totalParams; i++)
				{
					if ((*memo.params)[i].name == paramName)
					{
						nthParameter = (*memo.params)[i];
						flag_find = true;
						break;
					}
				}
			}
			ACAPI_DisposeElemMemoHdls(&memo);
		}
	}
	if (flag_find) {
		if (nthParameter.typeID == APIParT_CString) {
			param_string = nthParameter.value.uStr;
			param_bool = (param_string.GetLength() > 0);
		}
		else {
			param_real = round(nthParameter.value.real * 1000) / 1000;
			if (nthParameter.value.real - param_real > 0.001) param_real += 0.001;
			param_int = (GS::Int32)param_real;
			if (param_int / 1 < param_real) param_int += 1;
		}
		if (param_real > 0) param_bool = true;
		if (param_string.GetLength() == 0) {
			switch (nthParameter.typeID) {
			case APIParT_Integer:
				param_string = GS::UniString::Printf("%d", param_int);
				break;
			case APIParT_Boolean:
				if (param_bool) {
					param_string = "ИСТИНА";
				}
				else {
					param_string = "ЛОЖЬ";
				}
				break;
			case APIParT_Length:
				param_string = GS::UniString::Printf("%.0f", param_real * 1000);
				break;
			case APIParT_Angle:
				param_string = GS::UniString::Printf("%.1f", param_real);
				break;
			case APIParT_RealNum:
				param_string = GS::UniString::Printf("%.3f", param_real);
				break;
			default:
				flag_find = false;
				break;
			}
		}
	}
	return flag_find;
}

// -----------------------------------------------------------------------------
// Toggle a checked menu item
// -----------------------------------------------------------------------------
bool		InvertMenuItemMark(short menuResID, short itemIndex)
{
	API_MenuItemRef		itemRef;
	GSFlags				itemFlags;

	BNZeroMemory(&itemRef, sizeof(API_MenuItemRef));
	itemRef.menuResID = menuResID;
	itemRef.itemIndex = itemIndex;
	itemFlags = 0;

	ACAPI_Interface(APIIo_GetMenuItemFlagsID, &itemRef, &itemFlags);

	if ((itemFlags & API_MenuItemChecked) == 0)
		itemFlags |= API_MenuItemChecked;
	else
		itemFlags &= ~API_MenuItemChecked;

	ACAPI_Interface(APIIo_SetMenuItemFlagsID, &itemRef, &itemFlags);

	return (bool)((itemFlags & API_MenuItemChecked) != 0);
}


GS::UniString PropertyTestHelpers::ToString (const API_Variant& variant)
{
	switch (variant.type) {
		case API_PropertyIntegerValueType: return GS::ValueToUniString (variant.intValue);
		case API_PropertyRealValueType: return GS::ValueToUniString (variant.doubleValue);
		case API_PropertyStringValueType: return variant.uniStringValue;
		case API_PropertyBooleanValueType: return GS::ValueToUniString (variant.boolValue);
		case API_PropertyGuidValueType: return APIGuid2GSGuid (variant.guidValue).ToUniString ();
		case API_PropertyUndefinedValueType: return "Undefined Value";
		default: DBBREAK(); return "Invalid Value";
	}
}

GS::UniString PropertyTestHelpers::ToString (const API_Property& property)
{
	GS::UniString string;
	string += property.definition.name;
	string += ": ";
	const API_PropertyValue* value;
	if (property.status == API_Property_NotAvailable) {
		string += "(not available value)";
		return string;
	}

	if (property.isDefault && property.status == API_Property_NotEvaluated) {
		value = &property.definition.defaultValue.basicValue;
	} else {
		value = &property.value;
	}
	switch (property.definition.collectionType) {
		case API_PropertySingleCollectionType: {
			string += ToString (value->singleVariant.variant);
		} break;
		case API_PropertyListCollectionType: {
			string += '[';
			for (UInt32 i = 0; i < value->listVariant.variants.GetSize (); i++) {
				string += ToString (value->listVariant.variants[i]);
				if (i != value->listVariant.variants.GetSize () - 1) {
					string += ", ";
				}
			}
			string += ']';
		} break;
		case API_PropertySingleChoiceEnumerationCollectionType: {
			string += ToString (value->singleEnumVariant.displayVariant);
		} break;
		case API_PropertyMultipleChoiceEnumerationCollectionType: {
			string += '[';
			for (UInt32 i = 0; i < value->multipleEnumVariant.variants.GetSize (); i++) {
				string += ToString (value->multipleEnumVariant.variants[i].displayVariant);
				if (i != value->multipleEnumVariant.variants.GetSize () - 1) {
					string += ", ";
				}
			}
			string += ']';
		} break;
		default: {
			DBBREAK();
			string = "Invalid value";
		}
	}

	if (property.isDefault) {
		string += " (default value)";
	} else {
		string += " (custom value)";
	}

	return string;
}


bool operator== (const API_Variant& lhs, const API_Variant& rhs)
{
	if (lhs.type != rhs.type) {
		return false;
	}

	switch (lhs.type) {
		case API_PropertyIntegerValueType:
			return lhs.intValue == rhs.intValue;
		case API_PropertyRealValueType:
			return lhs.doubleValue == rhs.doubleValue;
		case API_PropertyStringValueType:
			return lhs.uniStringValue == rhs.uniStringValue;
		case API_PropertyBooleanValueType:
			return lhs.boolValue == rhs.boolValue;
		case API_PropertyGuidValueType:
			return lhs.guidValue == rhs.guidValue;
		default:
			return false;
	}
}


bool operator== (const API_SingleVariant& lhs, const API_SingleVariant& rhs)
{
	return lhs.variant == rhs.variant;
}


bool operator== (const API_ListVariant& lhs, const API_ListVariant& rhs)
{
	return lhs.variants == rhs.variants;
}


bool operator== (const API_SingleEnumerationVariant& lhs, const API_SingleEnumerationVariant& rhs)
{
	return lhs.keyVariant == rhs.keyVariant && lhs.displayVariant == rhs.displayVariant;
}


bool operator== (const API_MultipleEnumerationVariant& lhs, const API_MultipleEnumerationVariant& rhs)
{
	return lhs.variants == rhs.variants;
}


bool Equals (const API_PropertyDefaultValue& lhs, const API_PropertyDefaultValue& rhs, API_PropertyCollectionType collType)
{
	if (lhs.hasExpression != rhs.hasExpression) {
		return false;
	}

	if (lhs.hasExpression) {
		   return lhs.propertyExpressions == rhs.propertyExpressions;
	} else {
		   return Equals (lhs.basicValue, rhs.basicValue, collType);
	}
}


bool Equals (const API_PropertyValue& lhs, const API_PropertyValue& rhs, API_PropertyCollectionType collType)
{
	if (lhs.variantStatus != rhs.variantStatus) {
		return false;
	}

	if (lhs.variantStatus != API_VariantStatusNormal) {
		return true;
	}

	switch (collType) {
		case API_PropertySingleCollectionType:
			return lhs.singleVariant == rhs.singleVariant;
		case API_PropertyListCollectionType:
			return lhs.listVariant == rhs.listVariant;
		case API_PropertySingleChoiceEnumerationCollectionType:
			return lhs.singleEnumVariant == rhs.singleEnumVariant;
		case API_PropertyMultipleChoiceEnumerationCollectionType:
			return lhs.multipleEnumVariant == rhs.multipleEnumVariant;
		default:
			DBBREAK ();
			return false;
	}
}


bool operator== (const API_PropertyGroup& lhs, const API_PropertyGroup& rhs)
{
	return lhs.guid == rhs.guid &&
		   lhs.name == rhs.name;
}


bool operator== (const API_PropertyDefinition& lhs, const API_PropertyDefinition& rhs)
{
	return lhs.guid == rhs.guid &&
		   lhs.groupGuid == rhs.groupGuid &&
		   lhs.name == rhs.name &&
		   lhs.description == rhs.description &&
		   lhs.collectionType == rhs.collectionType &&
		   lhs.valueType == rhs.valueType &&
		   lhs.measureType == rhs.measureType &&
		   Equals (lhs.defaultValue, rhs.defaultValue, lhs.collectionType) &&
		   lhs.availability == rhs.availability &&
		   lhs.possibleEnumValues == rhs.possibleEnumValues;
}


bool operator== (const API_Property& lhs, const API_Property& rhs)
{
	if (lhs.definition != rhs.definition || lhs.isDefault != rhs.isDefault) {
		return false;
	}
	if (!lhs.isDefault) {
		return Equals (lhs.value, rhs.value, lhs.definition.collectionType);
	} else {
		return true;
	}
}
