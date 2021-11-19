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
#include	<cmath>
#include	<limits>

bool is_equal(double x, double y) {
	return std::fabs(x - y) < std::numeric_limits<double>::epsilon();
}

// --------------------------------------------------------------------
// Содержит ли значения элементиз списка игнорируемых
// --------------------------------------------------------------------
bool CheckIgnoreVal(const std::string& ignoreval, const GS::UniString& val) {
	GS::UniString unignoreval = GS::UniString(ignoreval.c_str());
	return CheckIgnoreVal(unignoreval, val);
}
bool CheckIgnoreVal(const GS::UniString& ignoreval, const GS::UniString& val) {
	if (ignoreval.IsEmpty()) return false;
	if ((ignoreval.ToLowerCase() == "empty" || ignoreval.ToLowerCase() == "пусто") && val.GetLength() < 1) {
		return true;
	}
	if (val == ignoreval) {
		return true;
	}
	return false;
}

// --------------------------------------------------------------------
// Содержит ли значения элементиз списка игнорируемых
// --------------------------------------------------------------------
bool CheckIgnoreVal(const GS::Array<GS::UniString>& ignorevals, const GS::UniString& val) {
	if (ignorevals.GetSize() > 0) {
		for (UInt32 i = 0; i < ignorevals.GetSize(); i++) {
			if (CheckIgnoreVal(ignorevals[i], val)) return true;
		}
	}
	return false;
}

// --------------------------------------------------------------------
// Перевод метров, заданных типом double в мм Int32
// --------------------------------------------------------------------
Int32 DoubleM2IntMM(const double& value) {
	double param_real = round(value * 1000) / 1000;
	if (value - param_real > 0.001) param_real += 0.001;
	param_real = param_real * 1000;
	Int32 param_int = (GS::Int32)param_real;
	if (param_int / 1 < param_real) param_int += 1;
	return param_int;
}

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
GSErrCode	AttachObserver(const API_Guid& objectId, const SyncSettings& syncSettings)
{
	GSErrCode		err = NoError;
	if (IsElementEditable(objectId, syncSettings)) {

#ifdef AC_22
		API_Elem_Head elemHead;
		elemHead.guid = objectId;
		err = ACAPI_Element_AttachObserver(&elemHead, 0);
#else
		err = ACAPI_Element_AttachObserver(objectId);
#endif
}
	return err;
}

// --------------------------------------------------------------------
// Проверяет - попадает ли тип элемента в под настройки синхронизации
// --------------------------------------------------------------------
bool SyncCheckElementType(const API_ElemTypeID& elementType, const SyncSettings&  syncSettings) {
	bool flag_type = false;
	if ((elementType == API_WallID || elementType == API_ColumnID || elementType == API_BeamID || elementType == API_SlabID ||
		elementType == API_RoofID || elementType == API_MeshID || elementType == API_ZoneID ||
		elementType == API_CurtainWallSegmentID || elementType == API_CurtainWallFrameID ||
		elementType == API_CurtainWallJunctionID || elementType == API_CurtainWallAccessoryID || elementType == API_ShellID ||
		elementType == API_MorphID || elementType == API_StairID || elementType == API_RiserID ||
		elementType == API_TreadID || elementType == API_StairStructureID ||
		elementType == API_RailingID || elementType == API_RailingToprailID || elementType == API_RailingHandrailID ||
		elementType == API_RailingRailID || elementType == API_RailingPostID || elementType == API_RailingInnerPostID ||
		elementType == API_RailingBalusterID || elementType == API_RailingPanelID || elementType == API_RailingSegmentID ||
		elementType == API_RailingNodeID || elementType == API_RailingBalusterSetID || elementType == API_RailingPatternID ||
		elementType == API_RailingToprailEndID || elementType == API_RailingHandrailEndID ||
		elementType == API_RailingRailEndID ||
		elementType == API_RailingToprailConnectionID ||
		elementType == API_RailingHandrailConnectionID ||
		elementType == API_RailingRailConnectionID ||
		elementType == API_RailingEndFinishID ||
		elementType == API_BeamSegmentID ||
		elementType == API_ColumnSegmentID ||
		elementType == API_OpeningID) && syncSettings.wallS) flag_type = true;
	if ((elementType == API_ObjectID ||
		elementType == API_ZoneID ||
		elementType == API_LampID ||
		elementType == API_CurtainWallID ||
		elementType == API_CurtainWallPanelID) && syncSettings.objS) flag_type = true;
	if ((elementType == API_WindowID || elementType == API_DoorID || elementType == API_SkylightID) && syncSettings.widoS) flag_type = true;
	return flag_type;
}

// -----------------------------------------------------------------------------
// Проверяет возможность редактирования объекта (не находится в модуле, разблокирован, зарезервирован)
// -----------------------------------------------------------------------------
bool IsElementEditable(const API_Guid& objectId, const SyncSettings&  syncSettings) {
	// Проверяем - зарезервирован ли объект
	if (objectId == APINULLGuid) return false;
#ifdef PK_1
	DeleteElementUserData(objectId);
#endif
	if (!ACAPI_Element_Filter(objectId, APIFilt_IsEditable)) return false;
	if (!ACAPI_Element_Filter(objectId, APIFilt_HasAccessRight)) return false;
	if (!ACAPI_Element_Filter(objectId, APIFilt_InMyWorkspace)) return false;
	// Проверяем - на находится ли объект в модуле
	API_Elem_Head	tElemHead;
	BNZeroMemory(&tElemHead, sizeof(API_Elem_Head));
	tElemHead.guid = objectId;
	if (ACAPI_Element_GetHeader(&tElemHead) != NoError) return false;
	if (!SyncCheckElementType(tElemHead.typeID, syncSettings)) return false;
	if (tElemHead.hotlinkGuid != APINULLGuid) return false;
	return true;
}


// -----------------------------------------------------------------------------
// Обновление отмеченных в меню пунктов
// -----------------------------------------------------------------------------
void MenuSetState(SyncSettings& syncSettings) {
	MenuItemCheckAC(Menu_MonAll, syncSettings.syncMon);
	MenuItemCheckAC(Menu_wallS, syncSettings.wallS);
	MenuItemCheckAC(Menu_widoS, syncSettings.widoS);
	MenuItemCheckAC(Menu_objS, syncSettings.objS);
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
		if (neig.neigID == APINeig_CurtainWall) {
			GS::Array<API_Guid> panelGuid;
			err = GetCWPanelsForCWall(neig.guid, panelGuid);
			if (err == NoError) {
				for (UInt32 i = 0; i < panelGuid.GetSize(); ++i) {
					guidArray.Push(panelGuid[i]);
				}
			}
		}
		guidArray.Push(neig.guid);
	}
	return guidArray;
#endif // AC_22
}


void CallOnSelectedElemSettings(void (*function)(const API_Guid&, const SyncSettings&), bool assertIfNoSel /* = true*/, bool onlyEditable /* = true*/, const SyncSettings& syncSettings)
{
	GS::UniString	title("Sync Selected");
	Int32 nLib = 0;
	ACAPI_Interface(APIIo_InitProcessWindowID, &title, &nLib);
	GS::Array<API_Guid> guidArray = GetSelectedElements(assertIfNoSel, onlyEditable);
	if (!guidArray.IsEmpty()) {
		//TODO Возможно, предварительный сбор имён свойств в хэш-таблицу ускорит работу
		// Надо только прокинуть их в функцию и добававить функцию поиска по ним
		//GetAllPropertyName(propname);
		GS::UniString intString = GS::UniString::Printf(" %d", guidArray.GetSize());
		msg_rep("Sync Selected", intString, NoError, APINULLGuid);
		for (UInt32 i = 0; i < guidArray.GetSize(); i++) {
			ACAPI_Interface(APIIo_SetProcessValueID, &i, nullptr);
			function(guidArray[i], syncSettings);
			if (ACAPI_Interface(APIIo_IsProcessCanceledID, nullptr, nullptr)) {
				return;
			}
		}
	}
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
		//TODO Возможно, предварительный сбор имён свойств в хэш-таблицу ускорит работу
		// Надо только прокинуть их в функцию и добававить функцию поиска по ним
		//GetAllPropertyName(propname);
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
GSErrCode WriteProp(const API_Guid& elemGuid, API_Property& property, GS::UniString& param_string) {
	GS::Int32 param_int = 0;
	bool param_bool = false;
	double param_real = 0;
	GSErrCode err = WriteProp(elemGuid, property, param_string,param_int, param_bool, param_real);
	return err;
}

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
		if (property.definition.valueType == API_PropertyStringValueType) {
			GS::UniString val = PropertyTestHelpers::ToString(propertyfrom);
			if (property.value.singleVariant.variant.uniStringValue != val) {
				property.value.singleVariant.variant.uniStringValue = val;
			}
			else {
				write = false;
			}
		}
		else {
			msg_rep("WriteProp2Prop", "Diff type " + propertyfrom.definition.name + "<->" + property.definition.name, NoError, elemGuid);
			err = APIERR_MISSINGCODE;
			write = false;
		}
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
	GS::UniString tinstring = instring;
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

// --------------------------------------------------------------------
// Получение списка GUID панелей навесной стены
// --------------------------------------------------------------------
GSErrCode GetCWPanelsForCWall(const API_Guid& cwGuid, GS::Array<API_Guid>& panelSymbolGuids)
{
	GSErrCode	err = NoError;
	API_ElementMemo	memo = {};
	err = ACAPI_Element_GetMemo(cwGuid, &memo, APIMemoMask_CWallPanels);
	if (err != NoError) {
		ACAPI_DisposeElemMemoHdls(&memo);
		return err;
	}
	bool isDegenerate = false;
	const GSSize nPanels = BMGetPtrSize(reinterpret_cast<GSPtr>(memo.cWallPanels)) / sizeof(API_CWPanelType);
	for (Int32 idx = 0; idx < nPanels; ++idx) {
		ACAPI_Database(APIDb_IsCWPanelDegenerateID, (void*)(&memo.cWallPanels[idx].head.guid), &isDegenerate);
		if (!isDegenerate && memo.cWallPanels[idx].hasSymbol) {
			panelSymbolGuids.Push(std::move(memo.cWallPanels[idx].head.guid));
		}
	}
	ACAPI_DisposeElemMemoHdls(&memo);
	return err;
}


// -----------------------------------------------------------------------------
// Получение определения свойства по имени свойства
// Формат имени ГРУППА/ИМЯ_СВОЙСТВА
// -----------------------------------------------------------------------------
GSErrCode GetPropertyDefinitionByName(const GS::UniString& propertyname, API_PropertyDefinition& definition) {
	GSErrCode err = GetPropertyDefinitionByName(APINULLGuid, propertyname, definition);
	return err;
}

GSErrCode GetPropertyDefinitionByName(const API_Guid& elemGuid, const GS::UniString& tpropertyname, API_PropertyDefinition& definition) {
	GSErrCode err = NoError;
	GS::UniString propertyname = tpropertyname;
	propertyname.ReplaceAll("\\/", "@@");
	if (propertyname.Contains("/")) {
		GS::Array<API_PropertyGroup> groups;
		GS::Array<GS::UniString> partstring;
		StringSplt(propertyname, "/", partstring);
		GS::UniString gname = partstring[0].ToLowerCase();
		GS::UniString pname = partstring[1].ToLowerCase();
		gname.ReplaceAll("@@", "/");
		pname.ReplaceAll("@@", "/");
		err = ACAPI_Property_GetPropertyGroups(groups);
		if (err != NoError) msg_rep("GetPropertyByName", "ACAPI_Property_GetPropertyGroups " + propertyname, err, elemGuid);
		if (err == NoError) {
			for (UInt32 i = 0; i < groups.GetSize(); i++) {
				if (groups[i].groupType == API_PropertyStaticBuiltInGroupType || groups[i].groupType == API_PropertyCustomGroupType){
					if (groups[i].name.ToLowerCase() == gname) {
						GS::Array<API_PropertyDefinition> definitions;
						err = ACAPI_Property_GetPropertyDefinitions(groups[i].guid, definitions);
						if (err != NoError) msg_rep("GetPropertyByName", "ACAPI_Property_GetPropertyDefinitions " + propertyname, err, elemGuid);
						if (err == NoError) {
							for (UInt32 j = 0; j < definitions.GetSize(); j++) {
								if (definitions[j].name.ToLowerCase() == pname) {
									definition = definitions[j];
									return err;
								}
							}
						}
					}
				}
			}
		}
	}
	GS::Array<API_PropertyDefinition> definitions;
	err = ACAPI_Property_GetPropertyDefinitions(APINULLGuid, definitions);
	if (err != NoError) msg_rep("GetPropertyByName", "ACAPI_Property_GetPropertyDefinitions All" + propertyname, err, elemGuid);
	if (err == NoError) {
		for (UInt32 j = 0; j < definitions.GetSize(); j++) {
			if (definitions[j].name.ToLowerCase() == propertyname.ToLowerCase()) {
				definition = definitions[j];
				return err;
			}
		}
	}
	return APIERR_MISSINGCODE;
}

GSErrCode GetPropertyFullName(const API_PropertyDefinition& definision, GS::UniString& name)
{
	if (definision.groupGuid == APINULLGuid) return APIERR_BADID;
	API_PropertyGroup group;
	group.guid = definision.groupGuid;
	GSErrCode error = ACAPI_Property_GetPropertyGroup(group);
	if (error == NoError)
		name = group.name + "/" + definision.name;
	return error;
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

GSErrCode GetGDLParameters(const API_Guid& elemGuid, const API_ElemTypeID& elemType, API_AddParType**& params)
{
	GSErrCode	err = NoError;
	API_ParamOwnerType	apiOwner = {};
	API_GetParamsType	apiParams = {};
	apiOwner.guid = elemGuid;
	apiOwner.typeID = elemType;
	err = ACAPI_Goodies(APIAny_OpenParametersID, &apiOwner);
	if (err != NoError) return err;
	err = ACAPI_Goodies(APIAny_GetActParametersID, &apiParams);
	if (err != NoError) return err;
	params = apiParams.params;
	err = ACAPI_Goodies(APIAny_CloseParametersID);
	return err;
}

GSErrCode GetGDLParametersHead(const API_Elem_Head elem_head, API_ElemTypeID& elemType, API_Guid& elemGuid) {
	GSErrCode		err = NoError;
	API_Element element = {};
	switch (elem_head.typeID) {
	case API_CurtainWallPanelID:
		element.header = elem_head;
		err = ACAPI_Element_Get(&element);
		if (err == NoError) {
			elemGuid = element.cwPanel.symbolID;
			elemType = API_ObjectID;
		}
		break;
	default:
		elemGuid = elem_head.guid;
		elemType = elem_head.typeID;
		break;
	}
	return err;
}

// -----------------------------------------------------------------------------
// Возвращает индекс параметра по его имени
// -----------------------------------------------------------------------------
bool FindGDLParametersByName(const GS::UniString& paramName, API_AddParType**& params, Int32& inx) {
	Int32	addParNum = BMGetHandleSize((GSHandle)params) / sizeof(API_AddParType);
	for (Int32 ii = 0; ii < addParNum; ++ii) {
		API_AddParType& actualParam = (*params)[ii];
		if (paramName.IsEqual(actualParam.name, GS::UniString::CaseInsensitive)) {
			inx = ii;
			return true;
		}
	}
	return false;
}

// -----------------------------------------------------------------------------
// Возвращает индекс параметра по его описанию
// -----------------------------------------------------------------------------
bool FindGDLParametersByDescription(const GS::UniString& paramName, const API_Elem_Head elem_head, Int32& inx) {
	API_LibPart libpart;
	API_Element element = {};
	element.header = elem_head;
	GSErrCode err = ACAPI_Element_Get(&element);
	if (err != NoError) {
		msg_rep("FindGDLParametersByDescription", "ACAPI_Element_Get", err, elem_head.guid);
		return false;
	}
	BNZeroMemory(&libpart, sizeof(libpart));
	libpart.index = element.object.libInd;
	err = ACAPI_LibPart_Get(&libpart);
	if (err != NoError) {
		msg_rep("FindGDLParametersByDescription", "ACAPI_LibPart_Get", err, elem_head.guid);
		return false;
	}
	double aParam = 0.0;
	double bParam = 0.0;
	Int32 paramNum = 0;
	API_AddParType** addPars = NULL;
	err = ACAPI_LibPart_GetParams(libpart.index, &aParam, &bParam, &paramNum, &addPars);
	if (err != NoError) {
		msg_rep("FindGDLParametersByDescription", "ACAPI_LibPart_GetParams " + paramName, err, elem_head.guid);
		ACAPI_DisposeAddParHdl(&addPars);
		return false;
	}
	for (Int32 i = 0; i < paramNum; i++) {
		GS::UniString tparamName = "";
		tparamName = (*addPars)[i].uDescname;
		tparamName.ReplaceAll(" ", "");
		if (paramName.IsEqual(tparamName, GS::UniString::CaseInsensitive)) {
			inx = i;
			ACAPI_DisposeAddParHdl(&addPars);
			return true;
		}
	}
	ACAPI_DisposeAddParHdl(&addPars);
	return false;
}

bool FindGDLParameters(const GS::UniString& paramName, const API_Elem_Head& elem_head, API_AddParType& nthParameter) {
	// Определяемся - как ищем. По имени или описанию?
	GS::UniString findstr = paramName.ToLowerCase();
	bool find_by_description = false;
	if (findstr.Contains("description:")) {
		if (elem_head.typeID == API_CurtainWallPanelID) return false; // По описанию в панелях не ищет.
		find_by_description = true;
		findstr.ReplaceAll("description:", "");
		findstr.ReplaceAll(" ", "");
	}
	bool flag_find = false;
	Int32 inx = 0;
	// Поищем по описанию. Запрашивать список актуальных параметров пока не будем - вдруг по описанию ничего не найдём
	if (find_by_description) {
		flag_find = FindGDLParametersByDescription(findstr, elem_head, inx);
		if (!flag_find) return false; // Ничего не нашли
	}
	API_AddParType** addPars = NULL;
	GSErrCode	err = GetGDLParameters(elem_head, addPars);
	if (err != NoError) return false; // Параметров нет. Почему - хз.
	if (!find_by_description) {
		flag_find = FindGDLParametersByName(findstr, addPars, inx);
		if (!flag_find) return false; // Ничего не нашли
	}
	nthParameter = (*addPars)[inx];
	ACAPI_DisposeAddParHdl(&addPars);
	return true;
}

GSErrCode GetGDLParameters(const API_Elem_Head elem_head, API_AddParType**& params) {
	API_ElemTypeID	elemType;
	API_Guid		elemGuid;
	GSErrCode	err = GetGDLParametersHead(elem_head, elemType, elemGuid);
	if (err == NoError) {
		err = GetGDLParameters(elemGuid, elemType, params);
	}
	return err;
}

bool FindLibCoords(const GS::UniString& paramName, const API_Elem_Head& elem_head, API_AddParType& nthParameter) {
	API_Element element = {};
	element.header = elem_head;
	GSErrCode err = ACAPI_Element_Get(&element);
	if (err != NoError) return false;
	double x = 0; double y = 0; double z = 0;
	switch (elem_head.typeID) {
	case API_CurtainWallPanelID:
		x = element.cwPanel.centroid.x;
		y = element.cwPanel.centroid.y;
		z = element.cwPanel.centroid.z;
		break;
	case API_ObjectID:
		x = element.object.pos.x;
		y = element.object.pos.y;
		z = element.object.level;
		break;
	case API_ZoneID:
		x = element.zone.pos.x;
		y = element.zone.pos.y;
		z = 0;
		break;
	default:
		return false;
	}
	if (paramName.Contains("symb_pos_x")) nthParameter.value.real = x;
	if (paramName.Contains("symb_pos_y")) nthParameter.value.real = y;
	if (paramName.Contains("symb_pos_z")) nthParameter.value.real = z;
	nthParameter.typeID = APIParT_RealNum;
	return true;
}

// -----------------------------------------------------------------------------
// Получить значение параметра с конвертацией типа данных
// TODO добавить обработку массивов
// -----------------------------------------------------------------------------
bool GetLibParam(const API_Guid& elemGuid, const GS::UniString& paramName, GS::UniString& param_string, GS::Int32& param_int, bool& param_bool, double& param_real) {
	GSErrCode		err = NoError;
	bool			flag_find = false;
	API_AddParType	nthParameter = {};
	API_Elem_Head	elem_head = {};
	elem_head.guid = elemGuid;
	err = ACAPI_Element_GetHeader(&elem_head);
	if (err == NoError) {
		BNZeroMemory(&nthParameter, sizeof(API_AddParType));
		if (paramName.Contains("symb_pos_")) {
			flag_find = FindLibCoords(paramName, elem_head, nthParameter); //Ищём координаты
		} else {
			flag_find = FindGDLParameters(paramName, elem_head, nthParameter); //Ищём пареметры
		}
		if (!flag_find) return false;
	}
	else {
		msg_rep(" GetLibParam", "ACAPI_Element_GetHeader", err, elem_head.guid);
		return false;
	}
	// Что-то нашли. Определяем тип и вычисляем текстовое, целочисленное и дробное значение.
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
	// Если параметр не строковое - определяем текстовое значение конвертацией
	if (param_string.GetLength() == 0) {
		API_AttrTypeID attrType = API_ZombieAttrID;
		short attrInx = (short)param_int;
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
		// Для реквезитов в текст выведем имена
		case APIParT_Profile:
			attrType = API_ProfileID;
			break;
		case APIParT_BuildingMaterial:
			attrType = API_BuildingMaterialID;
			break;
		case APIParT_FillPat:
			attrType = API_FilltypeID;
			break;
		case APIParT_Mater:
			attrType = API_MaterialID;
			break;
		default:
			flag_find = false;
			break;
		}
		if (attrType != API_ZombieAttrID) {
			API_Attribute	attrib = {};
			attrib.header.typeID = attrType;
			attrib.header.index = attrInx;
			err = ACAPI_Attribute_Get(&attrib);
			if (err == NoError) {
				param_string = GS::UniString::Printf("%s", attrib.header.name);
			}
			else {
				flag_find = false;
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


// TODO Придумать более изящную обработку округления
GS::UniString PropertyTestHelpers::NumToString(const double& var, const GS::UniString stringformat) {
	if (abs(var) < 0.00000001) return "0";
	GS::UniString out = "";
	double outvar = var;
	Int32 n_zero = 3;
	GS::UniString outstringformat = stringformat;
	bool trim_zero = true;
	if (!stringformat.IsEmpty()) {
		if (stringformat.Contains("mm")) {
			outvar = var * 1000;
			outstringformat.ReplaceAll("mm", "");
		}
		if (stringformat.Contains("cm")) {
			outvar = var * 100;
			outstringformat.ReplaceAll("cm", "");
		}
		if (stringformat.Contains("dm")) {
			outvar = var * 10;
			outstringformat.ReplaceAll("dm", "");
		}
		if (stringformat.Contains("gm")) {
			outvar = var / 100;
			outstringformat.ReplaceAll("gm", "");
		}
		if (stringformat.Contains("km")) {
			outvar = var / 1000;
			outstringformat.ReplaceAll("km", "");
		}
		if (outstringformat.Contains("m")) {
			outvar = var;
			outstringformat.ReplaceAll("m", "");
		}
		// Принудительный вывод заданного кол-ва нулей после запятой
		if (outstringformat.Contains("0")) {
			outstringformat.ReplaceAll("0", "");
			outstringformat.Trim();
			if (!outstringformat.IsEmpty()) trim_zero = false;
		}
		if (outstringformat.IsEmpty()) { 
			n_zero = 0;
		}
		else {
			n_zero = std::atoi(outstringformat.ToCStr());
		}
	}
	outvar = round(outvar * pow(10, n_zero)) / pow(10, n_zero);
	out = GS::UniString::Printf("%f", outvar);
	out.ReplaceAll(".", ",");
	out.TrimRight('0');
	if (trim_zero) {
		out.TrimRight(',');
	}
	else {
		Int32 addzero = n_zero- (out.GetLength() - out.FindFirst(',') - 1);
		if (addzero>0) out = out + GS::UniString::Printf("%*s", addzero, "0");
	}
	return out;
}

GS::UniString PropertyTestHelpers::ToString(const API_Variant& variant, const GS::UniString stringformat) {
	switch (variant.type) {
	case API_PropertyIntegerValueType: return  NumToString(variant.intValue, stringformat);
	case API_PropertyRealValueType: return NumToString(variant.doubleValue, stringformat);
	case API_PropertyStringValueType: return variant.uniStringValue;
	case API_PropertyBooleanValueType: return GS::ValueToUniString(variant.boolValue);
	case API_PropertyGuidValueType: return APIGuid2GSGuid(variant.guidValue).ToUniString();
	case API_PropertyUndefinedValueType: return "Undefined Value";
	default: DBBREAK(); return "Invalid Value";
	}
}

GS::UniString PropertyTestHelpers::ToString (const API_Variant& variant) {
	return PropertyTestHelpers::ToString(variant, "");
}

GS::UniString PropertyTestHelpers::ToString(const API_Property& property) {
	return PropertyTestHelpers::ToString(property, "");
}

GS::UniString PropertyTestHelpers::ToString (const API_Property& property, const GS::UniString stringformat) {
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
			string += ToString (value->singleVariant.variant, stringformat);
		} break;
		case API_PropertyListCollectionType: {
			for (UInt32 i = 0; i < value->listVariant.variants.GetSize (); i++) {
				string += ToString (value->listVariant.variants[i], stringformat);
				if (i != value->listVariant.variants.GetSize () - 1) {
					string += "; ";
				}
			}
		} break;
		case API_PropertySingleChoiceEnumerationCollectionType: {
#ifdef AC_25
			string += ToString(value->singleVariant.variant, stringformat);
#else // AC_25
			string += ToString(value->singleEnumVariant.displayVariant, stringformat);
#endif
		} break;
		case API_PropertyMultipleChoiceEnumerationCollectionType: {
#ifdef AC_25
			for (UInt32 i = 0; i < value->listVariant.variants.GetSize(); i++) {
				string += ToString(value->listVariant.variants[i], stringformat);
				if (i != value->listVariant.variants.GetSize() - 1) {
					string += "; ";
				}
			}
#else // AC_25
			for (UInt32 i = 0; i < value->multipleEnumVariant.variants.GetSize(); i++) {
				string += ToString(value->multipleEnumVariant.variants[i].displayVariant, stringformat);
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

void DeleteElementUserData(const API_Guid& elemguid) {
	API_Elem_Head	tElemHead = {};
	tElemHead.guid = elemguid;
	API_ElementUserData userData = {};
	GSErrCode err = ACAPI_Element_GetUserData(&tElemHead, &userData);
	if (err == NoError && userData.dataHdl != nullptr && userData.platformSign == GS::Act_Platform_Sign)
		err = ACAPI_Element_DeleteUserData(&tElemHead);
	BMKillHandle(&userData.dataHdl);
	GS::Array<API_Guid> setGuids;
	err = ACAPI_ElementSet_Identify(elemguid, &setGuids);
	if (err == NoError) {
		USize nSet = setGuids.GetSize();
		for (UIndex i = 0; i < nSet; i++) {
			err = ACAPI_ElementSet_Delete(setGuids[i]);
			if (err != NoError) {
				DBPRINTF("Delete Element Set error: %d\n", err);
			}
		}
	}
}

void DeleteElementsUserData()
{
	GSErrCode err = NoError;
	Int32       version;
	GSSize      nBytes;

	err = ACAPI_GetPreferences(&version, &nBytes, nullptr);
	if (version == CURR_ADDON_VERS && nBytes>0) {
		err = ACAPI_SetPreferences(CURR_ADDON_VERS, 0, nullptr);
	}
	err = ACAPI_GetPreferences_Platform(&version, &nBytes, NULL, NULL);
	if (version == CURR_ADDON_VERS && nBytes > 0) {
		err = ACAPI_SetPreferences(CURR_ADDON_VERS, 0, nullptr);
	}
	GS::Array<API_Guid> elemList;
	ACAPI_Element_GetElemList(API_ZombieElemID, &elemList);
	USize ng = elemList.GetSize();
	if (err == NoError) {
		ACAPI_CallUndoableCommand("Delete Element Set",
			[&]() -> GSErrCode {
				for (UIndex ii = 0; ii < ng; ii++) {
					DeleteElementUserData(elemList[ii]);
				}
				return NoError;
			});
	}
}
