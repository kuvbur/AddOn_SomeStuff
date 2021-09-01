#include "APIEnvir.h"
#include <stdio.h>
#include "ACAPinc.h"
#include "AddOnMain.h"
#include "ResourceIds.hpp"
#include "DGModule.hpp"
#include "APICommon.h"
#include "UniString.hpp"
#include "Property_Test_Helpers.hpp"
#include "APIdefs_Properties.h"

#define	 Menu_SyncAll		1
#define	 Menu_SyncSelect		2
#define	 Menu_LeghtMorh		3

static const GSResID AddOnInfoID			= ID_ADDON_INFO;
	static const Int32 AddOnNameID			= 1;
	static const Int32 AddOnDescriptionID	= 2;


static const short AddOnMenuID				= ID_ADDON_MENU;
	static const Int32 SyncAll_CommandID		= 1;
	static const Int32 SyncSelect_CommandID		= 2;
	static const Int32 LeghtMorh_CommandID = 3;

static	bool	elementMonitorEnabled = false;
static	bool	allNewElements = false;
static	bool	addMorph = false;
static GS::Array<API_Guid>	GetSelectedElements(bool assertIfNoSel /* = true*/, bool onlyEditable /*= true*/)
{
	GSErrCode            err;
	API_SelectionInfo    selectionInfo;
	GS::Array<API_Neig>  selNeigs;
	err = ACAPI_Selection_Get(&selectionInfo, &selNeigs, onlyEditable);
	BMKillHandle((GSHandle*)&selectionInfo.marquee.coords);
	if (err == APIERR_NOSEL || selectionInfo.typeID == API_SelEmpty) {
		if (assertIfNoSel) {
			DGAlert(DG_ERROR, "Error", "Please select an element!", "", "Ok");
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

void SyncAll(void) {
	GS::Array<API_Guid> guidArray;
	ACAPI_Element_GetElemList(API_ObjectID, &guidArray, APIFilt_IsEditable);
	for (UInt32 i = 0; i < guidArray.GetSize(); i++) {
		SyncData(guidArray[i]);
		ACAPI_Element_AttachObserver(guidArray[i]);
	}
}

void CallOnSelectedElem(void (*function)(const API_Guid&), bool assertIfNoSel /* = true*/, bool onlyEditable /* = true*/)
{
	GS::Array<API_Guid> guidArray = GetSelectedElements(assertIfNoSel, onlyEditable);

	if (!guidArray.IsEmpty()) {
		for (UInt32 i = 0; i < guidArray.GetSize(); i++) {
			function(guidArray[i]);
		}
	}
	else if (!assertIfNoSel) {
		function(APINULLGuid);
	}
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

// -----------------------------------------------------------------------------
// ElementEventHandlerProc
//
// -----------------------------------------------------------------------------
GSErrCode __ACENV_CALL	ElementEventHandlerProc(const API_NotifyElementType* elemType)
{
	GSErrCode		err = NoError;
	bool	sync_prop = false;

	if (elemType->notifID != APINotifyElement_BeginEvents && elemType->notifID != APINotifyElement_EndEvents
		&& (elemType->elemHead.typeID == API_ObjectID || elemType->elemHead.typeID == API_WindowID || elemType->elemHead.typeID == API_DoorID || (addMorph && elemType->elemHead.typeID == API_MorphID))) {
		API_Element			parentElement;
		API_ElementMemo		parentElementMemo;
		API_ElementUserData	parentUserData;

		BNZeroMemory(&parentElement, sizeof(API_Element));
		BNZeroMemory(&parentElementMemo, sizeof(API_ElementMemo));
		BNZeroMemory(&parentUserData, sizeof(API_ElementUserData));
		ACAPI_Notify_GetParentElement(&parentElement, &parentElementMemo, 0, &parentUserData);
		BMKillHandle(&parentUserData.dataHdl);

		switch (elemType->notifID) {
		case APINotifyElement_New:
			if (!allNewElements)
				break;
			sync_prop = true;
			err = ACAPI_Element_AttachObserver(elemType->elemHead.guid);
			if (err == APIERR_LINKEXIST)
				err = NoError;
			break;
		case APINotifyElement_Copy:
			if (!allNewElements)
				break;
			sync_prop = true;
			if (parentElement.header.guid != APINULLGuid) {
				err = ACAPI_Element_AttachObserver(elemType->elemHead.guid);
				if (err == APIERR_LINKEXIST)
					err = NoError;
			}
			break;
		case APINotifyElement_Change:
			sync_prop = true;

		case APINotifyElement_Edit:
			sync_prop = true;
			break;

		case APINotifyElement_Undo_Modified:
			sync_prop = true;
			break;

		case APINotifyElement_Redo_Created:
			sync_prop = true;
			break;

		case APINotifyElement_Redo_Modified:
			sync_prop = true;
			break;

		case APINotifyElement_Redo_Deleted:
			sync_prop = true;
			break;

		case APINotifyElement_PropertyValueChange:
			sync_prop = true;
			break;

		case APINotifyElement_ClassificationChange:
			sync_prop = true;
			break;

		default:
			break;
		}
		if (sync_prop) SyncData(elemType->elemHead.guid);
		ACAPI_DisposeElemMemoHdls(&parentElementMemo);
	}
	return err;
}	// ElementEventHandlerProc
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

//void WriteParam() 
//{
//	API_GetParamsType theParams = {};
//	API_ParamOwnerType paramOwner = {};
//	GS::uchar_t uStrBuffer[1024];
//
//	paramOwner.typeID = API_ObjectID;
//	paramOwner.libInd = libPartIndex;
//
//	GSErrCode err = ACAPI_Goodies(APIAny_OpenParametersID, &paramOwner);
//	err |= ACAPI_Goodies(APIAny_GetActParametersID, &theParams);
//	if (err == NoError) {
//		API_ChangeParamType changeParam = {};
//
//		CHTruncate(paramName, changeParam.name, API_NameLen);
//		GS::ucsncpy(uStrBuffer, newStrValue.ToUStr().Get(), BUFFER_SIZE);
//		changeParam.uStrValue = uStrBuffer;
//
//		err = ACAPI_Goodies(APIAny_ChangeAParameterID, &changeParam);
//		if (err == NoError) {
//			API_ElementMemo memo = {};
//			memo.params = theParams.params;
//			API_Element	mask;
//			ACAPI_ELEMENT_MASK_CLEAR(mask);
//			API_Element	elem = {};
//			elem.header.guid = elemGuid;
//			ACAPI_Element_Get(&elem);
//			err = ACAPI_Element_Change(&elem, &mask, &memo, APIMemoMask_AddPars, true);
//		}
//	}
//	ACAPI_DisposeAddParHdl(&theParams.params);
//	ACAPI_Goodies(APIAny_CloseParametersID);
//}

//bool SyncMorph(const API_Guid& elemGuid, API_Property& property){
//	bool flag_sync = false;
//	return flag_sync;
//}

bool SyncParamAndProp(const API_Guid &elemGuid, API_Property &property)
{
	bool flag_sync = false;
	GSErrCode		err = NoError;
	bool param2prop = false;
	GS::UniString property_definition = property.definition.description.ToUStr();
	property_definition.ReplaceAll(" ", "");
	if (property_definition.Contains("Sync_from")) param2prop = true;
	GS::UniString paramName = property_definition.GetSubstring('(', ')', 0);
	API_AddParType nthParameter;
	if (!GetLibParam(elemGuid, paramName, nthParameter))
	{
		return flag_sync;
	}
	GS::UniString param_string = "";
	GS::Int32 param_int = 0;
	bool param_bool = false;
	double param_real = 0;
	if (nthParameter.typeID == APIParT_CString) {
		param_string = nthParameter.value.uStr;
		param_bool = (param_string.GetLength() > 0);
		} else {
		param_real = round(nthParameter.value.real * 100) / 100;
		if (nthParameter.value.real - param_real > 0.001) param_real += 0.01;
		param_int = (GS::Int32)param_real;
		if (param_int / 1 < param_real) param_int += 1;
		}
	if (param_real > 0) param_bool = true;
	if (param_string.GetLength() == 0) {
		switch (nthParameter.typeID) {
		case APIParT_Integer:
			param_string = GS::UniString::Printf("%.0f", param_int);
			break;
		case APIParT_Boolean:
			if (param_bool){
				param_string = "ИСТИНА";
			} else { 
				param_string = "ЛОЖЬ"; 
			}
			break;
		case APIParT_Length:
			param_string = GS::UniString::Printf("%.0f", param_real*1000);
			break;
		case APIParT_Angle:
			param_string = GS::UniString::Printf("%.1f", param_real);
			break;
		case APIParT_RealNum:
			param_string = GS::UniString::Printf("%.3f", param_real);
			break;
		default:
			return flag_sync;
			break;
		}
	}
	property.isDefault = false;
	switch (property.definition.valueType) {
	case API_PropertyIntegerValueType:
		property.value.singleVariant.variant.intValue = param_int;
		break;
	case API_PropertyRealValueType:
		property.value.singleVariant.variant.doubleValue = param_real;
		break;
	case API_PropertyBooleanValueType:
		property.value.singleVariant.variant.boolValue = param_bool;
		break;
	case API_PropertyStringValueType:
		property.value.singleVariant.variant.uniStringValue = param_string;
		break;
	default:
		return flag_sync;
		break;
	}
	err = ACAPI_Element_SetProperty(elemGuid, property);
	if (!err) flag_sync=true;
	return flag_sync;
}

bool GetLibParam(const API_Guid& elemGuid, const GS::UniString &paramName, API_AddParType &nthParameter)
{
	GSErrCode		err = NoError;
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
		API_ElementMemo  memo;
		if (err == NoError && element.header.hasMemo)
		{
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
					}
				}
			}
			ACAPI_DisposeElemMemoHdls(&memo);
		}
	}
	return flag_find;
}

/*----------------------------------------------------------------------------**
** Lists all the visible properties (definition name and value) of an element **
**----------------------------------------------------------------------------*/

static void SyncData(const API_Guid& elemGuid)
{
	GSErrCode		err = NoError;
	API_Elem_Head elementHead;
	BNZeroMemory(&elementHead, sizeof(API_Elem_Head));
	elementHead.guid = elemGuid;
	err = ACAPI_Element_GetHeader(&elementHead);
	if (err) {
		return;
	}
	API_PropertyDefinitionFilter filter = API_PropertyDefinitionFilter_UserDefined;
	GS::Array<API_PropertyDefinition> definitions;
	err = ACAPI_Element_GetPropertyDefinitions(elemGuid, filter, definitions);
	if (err) {
		return;
	}
	GS::Array<API_Guid> propertyDefinitionList;
	for (UInt32 i = 0; i < definitions.GetSize(); i++) {
		if (ACAPI_Element_IsPropertyDefinitionVisible(elemGuid, definitions[i].guid)) {
			propertyDefinitionList.Push(definitions[i].guid);
		}
	}
	GS::Array<API_Property> properties;
	err = ACAPI_Element_GetPropertyValuesByGuid(elemGuid, propertyDefinitionList, properties);
	if (err) {
		return;
	}
	GS::Array<API_Property> properties_to_param;
	GS::Array<API_Property> param_to_properties;
	GS::UniString string;
	bool flag_sync = false;
	for (UInt32 i = 0; i < properties.GetSize(); i++) {
		if (properties[i].definition.description.Contains("Sync_flag")) {
			if (properties[i].isDefault)
			{
				flag_sync = properties[i].definition.defaultValue.basicValue.singleVariant.variant.boolValue;
			}
			else
			{
				flag_sync = properties[i].value.singleVariant.variant.boolValue;
			}
		}
	}
	if (flag_sync) {
		if (elementHead.typeID == API_MorphID)
		{
			//for (UInt32 i = 0; i < properties.GetSize(); i++) {
			//	if (properties[i].definition.description.Contains("Sync_") && properties[i].definition.description.Contains("(") && properties[i].definition.description.Contains(")")) {
			//		SyncMorph(elemGuid, properties[i]);
			//	}
			//}
		}
		else {
			for (UInt32 i = 0; i < properties.GetSize(); i++) {
				if (properties[i].definition.description.Contains("Sync_") && properties[i].definition.description.Contains("(") && properties[i].definition.description.Contains(")")) {
					SyncParamAndProp(elemGuid, properties[i]);
				}
			}
		}
	}
}


// ============================================================================
// Do_ElementMonitor
//
//	observe all newly created elements
// ============================================================================
void	Do_ElementMonitor(bool switchOn)
{
	if (switchOn) {
		ACAPI_Notify_CatchNewElement(nullptr, ElementEventHandlerProc);			// for all elements
		ACAPI_Notify_InstallElementObserver(ElementEventHandlerProc);			// observe all newly created elements
		allNewElements = true;
	}
	else {
		ACAPI_Notify_CatchNewElement(nullptr, nullptr);
		ACAPI_Notify_InstallElementObserver(nullptr);
		allNewElements = false;
	}

	return;
}	// Do_ElementMonitor

static GSErrCode MenuCommandHandler (const API_MenuParams *menuParams)
{
	bool t_elementMonitorEnabled = false;
	switch (menuParams->menuItemRef.menuResID) {
		case AddOnMenuID:
			switch (menuParams->menuItemRef.itemIndex) {
				case SyncAll_CommandID:
					elementMonitorEnabled = !elementMonitorEnabled;
					Do_ElementMonitor(elementMonitorEnabled);
					InvertMenuItemMark(32500, Menu_SyncAll);
					if (elementMonitorEnabled) SyncAll();
					break;
				case SyncSelect_CommandID:
					t_elementMonitorEnabled = elementMonitorEnabled;
					if (elementMonitorEnabled) {
						elementMonitorEnabled = false;
						Do_ElementMonitor(elementMonitorEnabled);
						InvertMenuItemMark(32500, Menu_SyncAll);
					}
					CallOnSelectedElem(SyncData);
					if (t_elementMonitorEnabled) {
						elementMonitorEnabled = true;
						Do_ElementMonitor(elementMonitorEnabled);
						InvertMenuItemMark(32500, Menu_SyncAll);
					}
					break;
				case LeghtMorh_CommandID:
					addMorph = !addMorph;
					InvertMenuItemMark(32500, Menu_LeghtMorh);
					break;
					

			}
			break;
	}
	return NoError;
}


API_AddonType __ACDLL_CALL CheckEnvironment (API_EnvirParams* envir)
{
	RSGetIndString (&envir->addOnInfo.name, AddOnInfoID, AddOnNameID, ACAPI_GetOwnResModule ());
	RSGetIndString (&envir->addOnInfo.description, AddOnInfoID, AddOnDescriptionID, ACAPI_GetOwnResModule ());

	return APIAddon_Normal;
}

GSErrCode __ACDLL_CALL RegisterInterface (void)
{
	return ACAPI_Register_Menu (AddOnMenuID, 0, MenuCode_Tools, MenuFlag_Default);
}

GSErrCode __ACENV_CALL Initialize (void)
{
	return ACAPI_Install_MenuHandler (AddOnMenuID, MenuCommandHandler);
}

GSErrCode __ACENV_CALL FreeData (void)
{
	return NoError;
}
