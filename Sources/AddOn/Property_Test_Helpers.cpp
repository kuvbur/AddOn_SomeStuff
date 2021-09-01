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
