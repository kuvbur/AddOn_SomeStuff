// *****************************************************************************
// File:			Property_Test_Helper.cpp
// Description:		Property_Test add-on helper macros and functions
// Project:			APITools/Property_Test
// Namespace:		-
// Contact person:	CSAT
// *****************************************************************************
#include	"APIEnvir.h"
#include	"ACAPinc.h"
#include	"Helpers.hpp"

// -----------------------------------------------------------------------------
// Проверка статуса и получение ID пользователя Teamwork
// -----------------------------------------------------------------------------
GSErrCode IsTeamwork(bool& isteamwork, short& userid) {
	isteamwork = false;
	API_ProjectInfo projectInfo = {};
	GSErrCode err = ACAPI_Environment(APIEnv_ProjectID, &projectInfo);
	if (err == NoError) {
		isteamwork = projectInfo.teamwork;
		userid = projectInfo.userId;
	}
	return err;
}

// -----------------------------------------------------------------------------
// Добавление отслеживания (для разных версий)
// -----------------------------------------------------------------------------
GSErrCode	AttachObserver(const API_Guid& objectId)
{
	GSErrCode		err = NoError;
#ifdef AC_22
	API_Elem_Head elemHead;
	elemHead.guid = objectId;
	err = ACAPI_Element_AttachObserver(&elemHead, 0);
#else
	err = ACAPI_Element_AttachObserver(objectId);
#endif
	return err;
}

// -----------------------------------------------------------------------------
// Обновление отмеченных в меню пунктов
// -----------------------------------------------------------------------------
void MenuSetState(void) {
	SyncPrefs prefsData;
	SyncSettingsGet(prefsData);
	MenuItemCheckAC(Menu_MonAll, prefsData.syncMon);
	MenuItemCheckAC(Menu_wallS, prefsData.wallS);
	MenuItemCheckAC(Menu_widoS, prefsData.widoS);
	MenuItemCheckAC(Menu_objS, prefsData.objS);
	MenuItemCheckAC(Menu_Log, prefsData.logMon);
}

void msg_rep(const GS::UniString& modulename, const GS::UniString &reportString, const GSErrCode &err, const API_Guid& elemGuid) {
	GS::UniString error_type = "";
	if (err != NoError) {
		switch (err)
		{
		case APIERR_GENERAL:
			error_type = "APIERR_GENERAL - General error code";
			break;
		case APIERR_MEMFULL:
			error_type = "APIERR_MEMFULL Insufficient memory.";
			break;
		case APIERR_CANCEL:
			error_type = "APIERR_CANCEL The operation has been canceled by the user, in case of a long process.";
			break;
		case APIERR_BADID:
			error_type = "APIERR_BADID The passed identifier is not a valid one, or valid, but not proper for the given operation.";
			break;
		case APIERR_BADINDEX:
			error_type = "APIERR_BADINDEX The passed index is out of range.";
			break;
		case APIERR_BADNAME:
			error_type = "APIERR_BADNAME The passed name is not proper or not found in the existing list.";
			break;
		case APIERR_BADPARS:
			error_type = "APIERR_BADPARS The passed parameters are inconsistent.";
			break;
		case APIERR_BADPOLY:
			error_type = "APIERR_BADPOLY The passed polygon cannot be interpreted.";
			break;
		case APIERR_BADDATABASE:
			error_type = "APIERR_BADDATABASE The command cannot be executed on the current database.";
			break;
		case APIERR_BADWINDOW:
			error_type = "APIERR_BADWINDOW The command cannot be executed while the current window is active.";
			break;
		case APIERR_BADKEYCODE:
			error_type = "APIERR_BADKEYCODE The key code cannot be found in the listing database.";
			break;
		case APIERR_BADPLATFORMSIGN:
			error_type = "APIERR_BADPLATFORMSIGN The passed platform sign is not valid";
			break;
		case APIERR_BADPLANE:
			error_type = "APIERR_BADPLANE The plane equation is incorrect.";
			break;
		case APIERR_BADUSERID:
			error_type = "APIERR_BADUSERID The passed user ID(TeamWork client) is not valid.";
			break;
		case APIERR_BADVALUE:
			error_type = "APIERR_BADVALUE The passed autotext value is not valid";
			break;
		case APIERR_BADELEMENTTYPE:
			error_type = "APIERR_BADELEMENTTYPE The function cannot be applied to the passed element type";
			break;
		case APIERR_IRREGULARPOLY:
			error_type = "APIERR_IRREGULARPOLY The passed polygon or polyline is irregular.See API_RegularizedPoly.";
			break;
		case APIERR_BADEXPRESSION:
			error_type = "The passed expression string is syntactically incorrect.";
			break;
		case -2130313100:
			error_type = "The passed guid is invalid or valid, but not proper for the given operation..";
			break;
		case APIERR_NO3D:
			error_type = "There is no 3D information assigned to the passed element.";
			break;
		case APIERR_NOMORE:
			error_type = "No more database items can be returned.";
			break;
		case APIERR_NOPLAN:
			error_type = "There is no open project.The operation cannot be executed without an open project.";
			break;
		case APIERR_NOLIB:
			error_type = "No library was loaded.The operation cannot be executed without a loaded library.Can be returned by ACAPI_LibPart_Create.";
			break;
		case APIERR_NOLIBSECT:
			error_type = "The requested LibPart section is not found.";
			break;
		case APIERR_NOSEL:
			error_type = "No selection.The operation cannot be executed without any element selected.";
			break;
		case APIERR_NOTEDITABLE:
			error_type = "The referenced element is not editable.";
			break;
		case APIERR_NOTSUBTYPEOF:
			error_type = "The specified first library part unique ID does not refer to a subtype of the second unique ID.See APIAny_CheckLibPartSubtypeOfID.";
			break;
		case APIERR_NOTEQUALMAIN:
			error_type = "The main GUID parts of the specified two library part unique IDs are not equal.See APIAny_CompareLibPartUnIdsID.";
			break;
		case APIERR_NOTEQUALREVISION:
			error_type = "The main GUID parts of the specified two library part unique IDs are equal but their revision IDs differ.See APIAny_CompareLibPartUnIdsID.";
			break;
		case APIERR_NOTEAMWORKPROJECT:
			error_type = "There is no open project, or not in Teamwork mode.";
			break;
		case APIERR_NOUSERDATA:
			error_type = "Attempt to get user data assigned to an element, but there isn’t any.";
			break;
		case APIERR_MOREUSER:
			error_type = "The user data cannot be assigned to the element, since there is no free storage block avaliable.";
			break;
		case APIERR_LINKEXIST:
			error_type = "The link already exists.";
			break;
		case APIERR_LINKNOTEXIST:
			error_type = "The link doesn’t exist.";
			break;
		case APIERR_WINDEXIST:
			error_type = "The window to be opened already exists.";
			break;
		case APIERR_WINDNOTEXIST:
			error_type = "The referenced window does not exist.";
			break;
		case APIERR_UNDOEMPTY:
			error_type = "No undoable entry has got into the opened undo operation.";
			break;
		case APIERR_REFERENCEEXIST:
			error_type = "The reference already exists.";
			break;
		case APIERR_NAMEALREADYUSED:
			error_type = "The resource must have a unique name but the specified one is already taken.";
			break;
		case APIERR_ATTREXIST:
			error_type = "The attribute already exists.";
			break;
		case APIERR_DELETED:
			error_type = "Reference to a deleted, purged or non - existent database item.";
			break;
		case APIERR_LOCKEDLAY:
			error_type = "The referenced layer is locked.";
			break;
		case APIERR_HIDDENLAY:
			error_type = "The referenced layer is hidden.";
			break;
		case APIERR_INVALFLOOR:
			error_type = "The passed floor index is out of range.";
			break;
		case APIERR_NOTMINE:
			error_type = "The database item is not in the user’s workspace.";
			break;
		case APIERR_NOACCESSRIGHT:
			error_type = "Can’t access / create / modify / delete an item in a teamwork server.";
			break;
#if defined(AC_22) || defined(AC_23)
		case APIERR_BADPROPERTYFORELEM:
			error_type = "The property for the passed element or attribute is not available.";
			break;
		case APIERR_BADCLASSIFICATIONFORELEM:
			error_type = "Can’t set the classification for the passed element or attribute.";
			break;
#else
		case APIERR_BADPROPERTY:
			error_type = "The property for the passed element or attribute is not available.";
			break;
		case APIERR_BADCLASSIFICATION:
			error_type = "Can’t set the classification for the passed element or attribute.";
			break;
#endif // AC_22 or AC_23
		case APIERR_MODULNOTINSTALLED:
			error_type = "The referenced add - on is not installed.For more details see the Communication Manager.";
			break;
		case APIERR_MODULCMDMINE:
			error_type = "The target add - on is the caller add - on.For more details see the Communication Manager.";
			break;
		case APIERR_MODULCMDNOTSUPPORTED:
			error_type = "The referenced command is not supported by the target add - on.For more details see the Communication Manager.";
			break;
		case APIERR_MODULCMDVERSNOTSUPPORTED:
			error_type = "The requested command version is newer than the version of the command that the target add - on can support.For more details see the Communication Manager.";
			break;
		case APIERR_NOMODULEDATA:
			error_type = "No custom data section is saved into the project file identified by the add - on’s unique ID.See ACAPI_ModulData_Get and ACAPI_ModulData_GetInfo.";
			break;
		case APIERR_PAROVERLAP:
			error_type = "Two or more paragraphs are overlapped.The end offset of one is greater than the beginner offset of the next one.";
			break;
		case APIERR_PARMISSING:
			error_type = "Number of paragraphs – the size of paragraphs handle – is zero.";
			break;
		case APIERR_PAROVERFLOW:
			error_type = "Paragraph end offset is run over the content length.";
			break;
		case APIERR_PARIMPLICIT:
			error_type = "The content string contains line end character(CR) at invalid position(inside the paragraph range).";
			break;
		case APIERR_RUNOVERLAP:
			error_type = "Two or more runs are overlapped.The end offset of one is greater than the beginner offset of the next one.";
			break;
		case APIERR_RUNMISSING:
			error_type = "Number of runs – the size of run pointer – is zero.";
			break;
		case APIERR_RUNOVERFLOW:
			error_type = "Run end offset is run over the content length.";
			break;
		case APIERR_RUNIMPLICIT:
			error_type = "The beginner offset of one is greater than the end offset of the previous one.";
			break;
		case APIERR_RUNPROTECTED:
			error_type = "Attempted to overwrite a protected text run(not used yet).";
			break;
		case APIERR_EOLOVERLAP:
			error_type = "The EOL array is not a monotonous ascendant sequence.";
			break;
		case APIERR_TABOVERLAP:
			error_type = "The tabulator array is not a monotonous ascendant sequence.";
			break;
		case APIERR_NOTINIT:
			error_type = "The command needs initialization by an other API call.";
			break;
		case APIERR_NESTING:
			error_type = "The API function is not reentrant.Nesting occurred.";
			break;
		case APIERR_NOTSUPPORTED:
			error_type = "The command is not supported by the server application.It is not environment dependent.The server application cannot execute the command generally.";
			break;
		case APIERR_REFUSEDCMD:
			error_type = "The passed identifier is not subject to the operation.";
			break;
		case APIERR_REFUSEDPAR:
			error_type = "The command cannot be executed with the passed parameters.";
			break;
		case APIERR_READONLY:
			error_type = "The specified location is read - only.Can be returned by ACAPI_LibPart_Create.";
			break;
		case APIERR_SERVICEFAILED:
			error_type = "The invoked Teamwork service has failed.";
			break;
		case APIERR_COMMANDFAILED:
			error_type = "The invoked undoable command threw an exception.Can be returned by ACAPI_CallUndoableCommand.";
			break;
		case APIERR_NEEDSUNDOSCOPE:
			error_type = "The called command should be encapsulated in a ACAPI_CallUndoableCommand scope.";
			break;
		case APIERR_MISSINGCODE:
			error_type = "The function is not implemented yet.";
			break;
		case APIERR_MISSINGDEF:
			error_type = "The originating library part file is missing.The document name is still filled.";
			break;
		default:
			break;
		}
	}
	if (elemGuid != APINULLGuid) {
		error_type = "GUID: " + APIGuid2GSGuid(elemGuid).ToUniString() + " " + error_type;
	}
	GS::UniString msg = modulename + ": " +  reportString + " " + error_type;
	ACAPI_WriteReport(msg, false);
}

GSErrCode SyncSettingsDefult(SyncPrefs& prefsData) {
	BNZeroMemory(&prefsData, sizeof(SyncPrefs));
	prefsData.version = CURR_ADDON_VERS;
	prefsData.syncAll = false;
	prefsData.syncMon = false;
	prefsData.wallS = true;
	prefsData.widoS = true;
	prefsData.objS = true;
	prefsData.logMon = true;
	GSErrCode err = ACAPI_SetPreferences(CURR_ADDON_VERS, sizeof(SyncPrefs), (GSPtr) &prefsData);
	return err;
}

void SyncSettingsGet(SyncPrefs& prefsData)
{
	GSErrCode err = NoError;
	Int32			version;
	GSSize			nBytes;
	unsigned short	platformSign = GS::Act_Platform_Sign;
	err = ACAPI_GetPreferences_Platform(&version, &nBytes, NULL, NULL);
	if (version == CURR_ADDON_VERS) {
		err = ACAPI_GetPreferences_Platform(&version, &nBytes, (GSPtr)&prefsData, &platformSign);
		if (platformSign != GS::Act_Platform_Sign) {
				GS::PlatformSign	inplatform = (GS::PlatformSign)platformSign;
				IVLong(inplatform, &prefsData.version);
				IVBool(inplatform, &prefsData.syncAll);
				IVBool(inplatform, &prefsData.syncMon);
				IVBool(inplatform, &prefsData.wallS);
				IVBool(inplatform, &prefsData.widoS);
				IVBool(inplatform, &prefsData.objS);
				IVBool(inplatform, &prefsData.logMon);
			}
	}
	else {
		err = SyncSettingsDefult(prefsData);
	}
#ifdef PK_1
	prefsData.syncMon = true;
	prefsData.logMon = true;
#endif // PK_1

}

void	MenuItemCheckAC(short itemInd, bool checked)
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
GS::Array<API_Guid>	GetSelectedElements(bool assertIfNoSel /* = true*/, bool onlyEditable /*= true*/)
{
	GSErrCode            err;
	API_SelectionInfo    selectionInfo;
	GS::UniString errorString = RSGetIndString(AddOnStringsID, ErrorSelectID, ACAPI_GetOwnResModule());
#ifdef AC_22
	API_Neig** selNeigs;
#else
	GS::Array<API_Neig>  selNeigs;
#endif 
	err = ACAPI_Selection_Get(&selectionInfo, &selNeigs, onlyEditable);
	BMKillHandle((GSHandle*)&selectionInfo.marquee.coords);
	if (err == APIERR_NOSEL || selectionInfo.typeID == API_SelEmpty) {
		if (assertIfNoSel) {
			DGAlert(DG_ERROR, "Error", errorString, "", "Ok");
		}
	}
	if (err != NoError) {
#ifdef AC_22
		BMKillHandle((GSHandle*)&selNeigs);
#endif // AC_22
		return GS::Array<API_Guid>();
	}
	GS::Array<API_Guid> guidArray;
#ifdef AC_22
	USize nSel = BMGetHandleSize((GSHandle)selNeigs) / sizeof(API_Neig);
	for (USize i = 0; i < nSel; i++) {
		guidArray.Push((*selNeigs)[i].guid);
	}
	BMKillHandle((GSHandle*)&selNeigs);
#else
	for (const API_Neig& neig : selNeigs) {
		guidArray.Push(neig.guid);
	}
	return guidArray;
#endif // AC_22
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
		GS::UniString intString = GS::UniString::Printf(" %d", guidArray.GetSize());
		msg_rep("Sync Selected", intString, NoError, APINULLGuid);
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
	if (err != NoError) {
		msg_rep("GetTypeByGUID", "", err, elemGuid);
		return err;
	}
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

// -----------------------------------------------------------------------------
// Запись в свойство строки/целочисленного/дробного/булевого значения
//  в зависимости от типа данных свойства
// -----------------------------------------------------------------------------
GSErrCode WriteProp(const API_Guid& elemGuid, API_Property& property, GS::UniString& param_string, GS::Int32& param_int, bool& param_bool, double& param_real) {
	GSErrCode		err = NoError;
	bool flag_rec = false;
	switch (property.definition.valueType) {
	case API_PropertyIntegerValueType:
		if (property.value.singleVariant.variant.intValue != param_int) {
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
		if (err != NoError) msg_rep("WriteProp", "ACAPI_Element_SetProperty", err, elemGuid);
	}
	return err;
}

// -----------------------------------------------------------------------------
// Запись значения параметра библиотечного элемента в свойство
// -----------------------------------------------------------------------------
GSErrCode WriteParam2Prop(const API_Guid& elemGuid, const GS::UniString& paramName, API_Property& property) {
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
// Запись значения свойства в другое свойство
// -----------------------------------------------------------------------------
GSErrCode WriteProp2Prop(const API_Guid& elemGuid, const API_Property& propertyfrom, API_Property& property)
{
	GSErrCode	err = NoError;
	bool write = true;
	// Есть ли вычисленное/доступное значение?
#if defined(AC_22) || defined(AC_23)
	bool isnoteval = (!propertyfrom.isEvaluated);
#else
	bool isnoteval = (propertyfrom.status != API_Property_HasValue);
#endif
	// Свойство недоступно или содержит невычисленное значение
	if (isnoteval && write) {
		err = APIERR_MISSINGCODE;
		write = false;
	}
	// Совпадают ли типы?
	if (propertyfrom.definition.valueType != property.definition.valueType && write) {
		msg_rep("WriteProp2Prop", "Diff type " + propertyfrom.definition.name + "<->" + property.definition.name, NoError, elemGuid);
		err = APIERR_MISSINGCODE;
		write = false;
	}
	// Если нужно записать список текста в текст
	if (property.definition.collectionType != propertyfrom.definition.collectionType && property.definition.valueType == API_PropertyStringValueType && property.definition.collectionType == API_PropertySingleCollectionType && write) {
		GS::UniString val = PropertyTestHelpers::ToString(propertyfrom);
		if (property.value.singleVariant.variant.uniStringValue != val) {
			property.value.singleVariant.variant.uniStringValue = val;
		}
		else {
			write = false; //Значения одинаковые
		}
	}
	// Обычное копирование
	if (property.definition.collectionType == propertyfrom.definition.collectionType && property.definition.collectionType == API_PropertySingleCollectionType && write) {
		if (property.value.singleVariant != propertyfrom.value.singleVariant) {
			property.value.singleVariant = propertyfrom.value.singleVariant;
		}
		else {
			write = false; //Значения одинаковые
		}
	}
	if (write) {
		property.isDefault = false;
		err = ACAPI_Element_SetProperty(elemGuid, property);
		if (err != NoError) msg_rep("WriteProp2Prop", "ACAPI_Element_SetProperty", err, elemGuid);
	}
	return err;
}

// -----------------------------------------------------------------------------
// Делит строку по разделителю, возвращает кол-во частей
// -----------------------------------------------------------------------------
UInt32 StringSplt(const GS::UniString& instring, const GS::UniString& delim, GS::Array<GS::UniString> &partstring) {
	GS::Array<GS::UniString> parts;
	UInt32 npart = instring.Split(delim, &parts);
	UInt32 n = 0;
	for (UInt32 i = 0; i < npart; i++) {
		GS::UniString part = parts[i];
		if (!part.IsEmpty()) {
			part.Trim('\r');
			part.Trim('\n');
			part.Trim();
			if (!part.IsEmpty()) {
				partstring.Push(part);
				n += 1;
			}
		}
	}
	return n;
}

// -----------------------------------------------------------------------------
// Делит строку по разделителю, возвращает кол-во частей
// Записывает в массив только части, содержащие строку filter
// -----------------------------------------------------------------------------
UInt32 StringSplt(const GS::UniString& instring, const GS::UniString& delim, GS::Array<GS::UniString>& partstring, const GS::UniString& filter) {
	GS::Array<GS::UniString> parts;
	UInt32 n = 0;
	UInt32 npart = StringSplt(instring, delim, parts);
	for (UInt32 i = 0; i < npart; i++) {
		if (parts[i].Contains(filter)) {
			partstring.Push(parts[i]);
			n += 1;
		}
	}
	return n;
}

// -----------------------------------------------------------------------------
// Получение определения свойства по имени свойства
// Формат имени ГРУППА/ИМЯ_СВОЙСТВА
// -----------------------------------------------------------------------------
GSErrCode GetPropertyDefinitionByName(const API_Guid& elemGuid, const GS::UniString& propertyname, API_PropertyDefinition& definition) {
	GS::UniString fullnames;
	GS::Array<API_PropertyGroup> groups;
	GS::Array<GS::UniString> partstring;
	StringSplt(propertyname, "/", partstring);
	GSErrCode err = ACAPI_Property_GetPropertyGroups(groups);
	if (err != NoError) msg_rep("GetPropertyByName", "ACAPI_Property_GetPropertyGroups " + propertyname, err, elemGuid);
	if (err == NoError) {
		for (UInt32 i = 0; i < groups.GetSize(); i++) {
			if (groups[i].groupType == API_PropertyCustomGroupType && groups[i].name.ToLowerCase() == partstring[0].ToLowerCase())
			{
				GS::Array<API_PropertyDefinition> definitions;
				err = ACAPI_Property_GetPropertyDefinitions(groups[i].guid, definitions);
				if (err != NoError) msg_rep("GetPropertyByName", "ACAPI_Property_GetPropertyDefinitions " + propertyname, err, elemGuid);
				if (err == NoError) {
					for (UInt32 j = 0; j < definitions.GetSize(); j++) {
						if (definitions[j].name.ToLowerCase() == partstring[1].ToLowerCase()) {
							definition = definitions[j];
							return err;
						}
					}
				}
			}
		}
	}
	return APIERR_MISSINGCODE;
}

GSErrCode GetVisiblePropertyDefinitions(const API_Guid& elemGuid, GS::Array<API_PropertyDefinition>& visibleProperties)
{
	GS::Array<API_PropertyDefinition> definitions;
	GSErrCode error = ACAPI_Element_GetPropertyDefinitions(elemGuid, API_PropertyDefinitionFilter_UserDefined, definitions);
	if (error == NoError) {
		for (UInt32 i = 0; i < definitions.GetSize(); ++i) {
			if (ACAPI_Element_IsPropertyDefinitionVisible(elemGuid, definitions[i].guid)) {
				visibleProperties.Push(definitions[i]);
			}
		}
	}
	return error;
}

// -----------------------------------------------------------------------------
// Поиск свойства по имени, включая имя группы
// Формат имени ГРУППА/ИМЯ_СВОЙСТВА
// -----------------------------------------------------------------------------
GSErrCode GetPropertyByName(const API_Guid& elemGuid, const GS::UniString& propertyname, API_Property& property) {
	API_PropertyDefinition definitions;
	GSErrCode err = GetPropertyDefinitionByName(elemGuid, propertyname, definitions);
	if (err == NoError) {
		err = ACAPI_Element_GetPropertyValue(elemGuid, definitions.guid, property);
		if (err != NoError) msg_rep("GetPropertyByName", "ACAPI_Element_GetPropertyValue " + definitions.name, err, elemGuid);
	}
	return err;
}

// -----------------------------------------------------------------------------
// Получить значение параметра с конвертацией типа данных
// TODO добавить обработку массивов
// -----------------------------------------------------------------------------
bool GetLibParam(const API_Guid& elemGuid, const GS::UniString& paramName, GS::UniString& param_string, GS::Int32& param_int, bool& param_bool, double& param_real) {
	GSErrCode		err = NoError;
	bool flag_find = false;
	API_AddParType nthParameter;
	BNZeroMemory(&nthParameter, sizeof(API_AddParType));
	API_Element element = {};
	element.header.guid = elemGuid;
	err = ACAPI_Element_Get(&element);
	if (err != NoError) msg_rep("GetLibParam", "ACAPI_Element_GetHeader " + paramName, err, elemGuid);
	if (err == NoError) {
		if (element.header.hasMemo) {
			API_ElementMemo  memo;
			BNZeroMemory(&memo, sizeof(API_ElementMemo));
			err = ACAPI_Element_GetMemo(element.header.guid, &memo, APIMemoMask_AddPars);
			if (err != NoError) msg_rep("GetLibParam", "ACAPI_Element_GetMemo " + paramName, err, elemGuid);
			if (err == NoError){
				UInt32 totalParams = BMGetHandleSize((GSConstHandle)memo.params) / sizeof(API_AddParType);  // number of parameters = handlesize / size of single handle
				for (UInt32 i = 0; i < totalParams; i++) {
					if (paramName.IsEqual((*memo.params)[i].name, GS::UniString::CaseInsensitive)) {
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
					param_string = RSGetIndString(AddOnStringsID, TrueId, ACAPI_GetOwnResModule());
				}
				else {
					param_string = RSGetIndString(AddOnStringsID, FalseId, ACAPI_GetOwnResModule());
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
bool		MenuInvertItemMark(short menuResID, short itemIndex) {
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


GS::UniString PropertyTestHelpers::ToString (const API_Variant& variant) {
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

GS::UniString PropertyTestHelpers::ToString (const API_Property& property) {
	GS::UniString string;
	const API_PropertyValue* value;
#if defined(AC_22) || defined(AC_23)
	if (!property.isEvaluated) {
		return string;
	}
	if (property.isDefault && !property.isEvaluated) {
		value = &property.definition.defaultValue.basicValue;
	}
	else {
		value = &property.value;
	}
#else
	if (property.status == API_Property_NotAvailable) {
		return string;
	}
	if (property.isDefault && property.status == API_Property_NotEvaluated) {
		value = &property.definition.defaultValue.basicValue;
	} else {
		value = &property.value;
	}
#endif
	switch (property.definition.collectionType) {
		case API_PropertySingleCollectionType: {
			string += ToString (value->singleVariant.variant);
		} break;
		case API_PropertyListCollectionType: {
			for (UInt32 i = 0; i < value->listVariant.variants.GetSize (); i++) {
				string += ToString (value->listVariant.variants[i]);
				if (i != value->listVariant.variants.GetSize () - 1) {
					string += "; ";
				}
			}
		} break;
		case API_PropertySingleChoiceEnumerationCollectionType: {
#ifdef AC_25
			string += ToString(value->singleVariant.variant);
#else // AC_25
			string += ToString(value->singleEnumVariant.displayVariant);
#endif
		} break;
		case API_PropertyMultipleChoiceEnumerationCollectionType: {
#ifdef AC_25
			for (UInt32 i = 0; i < value->listVariant.variants.GetSize(); i++) {
				string += ToString(value->listVariant.variants[i]);
				if (i != value->listVariant.variants.GetSize() - 1) {
					string += "; ";
				}
			}
#else // AC_25
			for (UInt32 i = 0; i < value->multipleEnumVariant.variants.GetSize(); i++) {
				string += ToString(value->multipleEnumVariant.variants[i].displayVariant);
				if (i != value->multipleEnumVariant.variants.GetSize() - 1) {
					string += "; ";
				}
			}
#endif
		} break;
		default: {
			break;
		}
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

#ifndef AC_25
bool operator== (const API_MultipleEnumerationVariant& lhs, const API_MultipleEnumerationVariant& rhs)
{
	return lhs.variants == rhs.variants;
}
#endif

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
#ifdef AC_25
		case API_PropertySingleChoiceEnumerationCollectionType:
			return lhs.singleVariant == rhs.singleVariant;
		case API_PropertyMultipleChoiceEnumerationCollectionType:
			return lhs.listVariant == rhs.listVariant;
#else
		case API_PropertySingleChoiceEnumerationCollectionType:
			return lhs.singleEnumVariant == rhs.singleEnumVariant;
		case API_PropertyMultipleChoiceEnumerationCollectionType:
			return lhs.multipleEnumVariant == rhs.multipleEnumVariant;
#endif
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
