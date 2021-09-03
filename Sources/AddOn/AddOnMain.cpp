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

// -----------------------------------------------------------------------------
// Запускает обработку всех объектов, окон, дверей
// -----------------------------------------------------------------------------
void SyncAll(void) {
	GS::Array<API_Guid> guidArray;
	ACAPI_Element_GetElemList(API_ObjectID, &guidArray, APIFilt_IsEditable);
	for (UInt32 i = 0; i < guidArray.GetSize(); i++) {
		SyncData(guidArray[i]);
		ACAPI_Element_AttachObserver(guidArray[i]);
	}

	ACAPI_Element_GetElemList(API_WindowID, &guidArray, APIFilt_IsEditable);
	for (UInt32 i = 0; i < guidArray.GetSize(); i++) {
		SyncData(guidArray[i]);
		ACAPI_Element_AttachObserver(guidArray[i]);
	}

	ACAPI_Element_GetElemList(API_DoorID, &guidArray, APIFilt_IsEditable);
	for (UInt32 i = 0; i < guidArray.GetSize(); i++) {
		SyncData(guidArray[i]);
		ACAPI_Element_AttachObserver(guidArray[i]);
	}
}

// -----------------------------------------------------------------------------
// Срабатывает при изменении элемента
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

bool SyncMorph(const API_Elem_Head &elementHead){
	GSErrCode        err;
	API_ElemInfo3D info3D;
	API_Component3D m_component;
	char    msgStr[1024];
	BNZeroMemory(&info3D, sizeof(API_ElemInfo3D));
	err = ACAPI_Element_Get3DInfo(elementHead, &info3D);
	BNZeroMemory(&m_component, sizeof(API_Component3D));
	m_component.header.typeID = API_BodyID;
	m_component.header.index = info3D.fbody;
	err = ACAPI_3D_GetComponent(&m_component);
	long firstEdge = m_component.pgon.fpedg;
	long lastEdge = m_component.pgon.lpedg;
	// Walk through the edges
		m_component.header.typeID = API_PedgID;
		m_component.header.index = lastEdge;
		ACAPI_3D_GetComponent(&m_component);
		long edgeInd = m_component.pedg.pedg;
		m_component.header.typeID = API_EdgeID;
		m_component.header.index = abs(edgeInd);
		ACAPI_3D_GetComponent(&m_component);
		long vert1 = m_component.edge.vert1;
		m_component.header.typeID = API_VertID;
		m_component.header.index = vert1;
		ACAPI_3D_GetComponent(&m_component);
		sprintf(msgStr, " vertex 1 index = %d coord (%Lf, %Lf, %Lf)",
			m_component.header.index,
			m_component.vert.x, m_component.vert.y, m_component.vert.z);
		ACAPI_WriteReport(msgStr, false);

		m_component.header.typeID = API_PedgID;
		m_component.header.index = firstEdge;
		ACAPI_3D_GetComponent(&m_component);
		edgeInd = m_component.pedg.pedg;
		m_component.header.typeID = API_EdgeID;
		m_component.header.index = abs(edgeInd);
		ACAPI_3D_GetComponent(&m_component);
		vert1 = m_component.edge.vert1;
		m_component.header.typeID = API_VertID;
		m_component.header.index = vert1;
		ACAPI_3D_GetComponent(&m_component);
		sprintf(msgStr, " vertex 1 index = %d coord (%Lf, %Lf, %Lf)",
			m_component.header.index,
			m_component.vert.x, m_component.vert.y, m_component.vert.z);
		ACAPI_WriteReport(msgStr, false);

	bool flag_sync = false;
	return flag_sync;
}

// -----------------------------------------------------------------------------
// Запись в свойство строки/целочисленного/дробного/булевого значения
//  в зависимости от типа данных свойства
// -----------------------------------------------------------------------------
GSErrCode WriteProp(const API_Guid& elemGuid, API_Property& property, GS::UniString& param_string, GS::Int32& param_int, bool& param_bool, double& param_real) {
	GSErrCode		err = NoError;
	bool flag_rec = false;
	switch (property.definition.valueType) {
	case API_PropertyIntegerValueType:
		if (property.value.singleVariant.variant.intValue != param_int){
			property.value.singleVariant.variant.intValue = param_int;
			flag_rec = true;
		}
		break;
	case API_PropertyRealValueType:
		if (property.value.singleVariant.variant.doubleValue != param_real) {
			property.value.singleVariant.variant.doubleValue = param_real;
			flag_rec = true;
		}
		break;
	case API_PropertyBooleanValueType:
		if (property.value.singleVariant.variant.boolValue != param_bool) {
			property.value.singleVariant.variant.boolValue = param_bool;
			flag_rec = true;
		}
		break;
	case API_PropertyStringValueType:
		if (property.value.singleVariant.variant.uniStringValue != param_string) {
			property.value.singleVariant.variant.uniStringValue = param_string;
			flag_rec = true;
		}
		break;
	default:
		break;
	}
	if (flag_rec) {
		property.isDefault = false;
		err = ACAPI_Element_SetProperty(elemGuid, property);
	}
	return err;
}

// -----------------------------------------------------------------------------
// Запись значения параметра библиотечного элемента в свойство
// -----------------------------------------------------------------------------
GSErrCode WriteParam2Prop(const API_Guid& elemGuid, API_Property& property, const GS::UniString& paramName) {
	GSErrCode		err = NoError;
	GS::UniString param_string = "";
	GS::Int32 param_int = 0;
	bool param_bool = false;
	double param_real = 0;
	if (GetLibParam(elemGuid, paramName, param_string, param_int, param_bool, param_real))
	{
		err = WriteProp(elemGuid, property, param_string, param_int, param_bool, param_real);
	}
	else {
		err = APIERR_MISSINGCODE;
	}
	return err;
}

// -----------------------------------------------------------------------------
// Поиск свойства по имени, включая имя группы
// Формат имяни ГРУППА/ИМЯ_СВОЙСТВА
// -----------------------------------------------------------------------------
GSErrCode GetPropertyByName(const API_Guid& elemGuid, API_Property& property, const GS::UniString& propertyname) {
	GS::UniString fullnames;
	GS::Array<API_PropertyGroup> groups;
	GSErrCode error = ACAPI_Property_GetPropertyGroups(groups);
	if (error == NoError) {
		for (UInt32 i = 0; i < groups.GetSize(); i++) {
			if (groups[i].groupType==API_PropertyCustomGroupType)
			{
				GS::Array<API_PropertyDefinition> definitions;
				error = ACAPI_Property_GetPropertyDefinitions(groups[i].guid, definitions);
				if (error == NoError) {
					for (UInt32 j = 0; j < definitions.GetSize(); j++) {
						fullnames = groups[i].name + "/" + definitions[j].name;
						if (propertyname == fullnames) {
							GS::Array<API_PropertyDefinition> definitionsList;
							GS::Array<API_Property> propertyList;
							definitionsList.Push(definitions[j]);
							error = ACAPI_Element_GetPropertyValues(elemGuid, definitionsList, propertyList);
							if (error==NoError) property = propertyList[0];
							break;
						}
					}
				}
			}
		}
	}
	return error;
}

// -----------------------------------------------------------------------------
// Запись значения свойства в другое свойство
// -----------------------------------------------------------------------------
GSErrCode SyncPropAndProp(const API_Guid& elemGuid, API_Property& property, const GS::UniString& paramName)
{
	GSErrCode		err = NoError;
	API_Property propertyfrom;
	err = GetPropertyByName(elemGuid, propertyfrom, paramName);
	if (err == NoError) {
		if (propertyfrom.status == API_Property_HasValue) {
			if (property.value.singleVariant != propertyfrom.value.singleVariant) {
				property.isDefault = false;
				err = ACAPI_Element_SetProperty(elemGuid, property);
			}
		}
	}
	return err;
}

// -----------------------------------------------------------------------------
// Синхронизация значений свойства и параметра
// -----------------------------------------------------------------------------
GSErrCode SyncParamAndProp(const API_Guid &elemGuid, API_Property &property, const GS::UniString &paramName,  int &syncdirection)
{
	GSErrCode		err = NoError;
	if (syncdirection==1){
		err = WriteParam2Prop(elemGuid, property, paramName);
	}
	return err;
}

// --------------------------------------------------------------------------------------------------------------------------
// Ищет свойство со значение "Sync_flag" в описании и по значению определяет - нужно ли синхронизировать параметры элемента
// --------------------------------------------------------------------------------------------------------------------------
bool SyncState(const API_Guid& elemGuid) {
	bool flag_sync = false;
	GSErrCode err = NoError;
	GS::Array<API_Property> properties;
	err = GetPropertyByGuid(elemGuid, properties);
	if (!err) {
		for (UInt32 i = 0; i < properties.GetSize(); i++) {
			if (properties[i].definition.description.Contains("Sync_flag")) {
				if (properties[i].isDefault)
				{
					flag_sync = properties[i].definition.defaultValue.basicValue.singleVariant.variant.boolValue;
					break;
				}
				else
				{
					flag_sync = properties[i].value.singleVariant.variant.boolValue;
					break;
				}
			}
		}
	}
	return flag_sync;
}

// --------------------------------------------------------------------
// Синхронизация данныъ элемента согласно указаниям в описании свойств
// --------------------------------------------------------------------
static void SyncData(const API_Guid& elemGuid) {
	if (SyncState(elemGuid)) {
		GSErrCode		err = NoError;
		GS::Array<API_Property> properties;
		err = GetPropertyByGuid(elemGuid, properties);
		if (!err) {
			API_Elem_Head elementHead;
			BNZeroMemory(&elementHead, sizeof(API_Elem_Head));
			elementHead.guid = elemGuid;
			err = ACAPI_Element_GetHeader(&elementHead);
			GS::UniString description_string = "";
			GS::UniString paramName = "";
			int synctype = 0;
			int syncdirection = 0;
			if (!err) {
				for (UInt32 i = 0; i < properties.GetSize(); i++) {
					description_string = properties[i].definition.description.ToUStr();
					if (SyncString(description_string, paramName, synctype, syncdirection)) {
						switch (synctype) {
						case 1:
							SyncParamAndProp(elemGuid, properties[i], paramName, syncdirection);
							break;
						case 2:
							SyncPropAndProp(elemGuid, properties[i], paramName);
							break;
						default:
							break;
						}

					}
				}
			}
		}
	}
}


// ============================================================================
// Do_ElementMonitor
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
					}
					CallOnSelectedElem(SyncData);
					if (t_elementMonitorEnabled) {
						elementMonitorEnabled = true;
						Do_ElementMonitor(elementMonitorEnabled);
					}
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
