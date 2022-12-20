//------------ kuvbur 2022 ------------
#include	<math.h>
#include	<cmath>
#include	<limits>
#include	"APIEnvir.h"
#include	"ACAPinc.h"
#include	"Helpers.hpp"
#include	"Model3D/model.h"
#include	"Model3D/MeshBody.hpp"

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
	double param_real = round(value * 1000.0) / 1000.0;
	if (value - param_real > 0.001) param_real += 0.001;
	param_real = param_real * 1000.0;
	Int32 param_int = ceil_mod((GS::Int32)param_real, 1);
	return param_int;
}

// --------------------------------------------------------------------
// Округлить целое n вверх до ближайшего целого числа, кратного k
// --------------------------------------------------------------------
Int32 ceil_mod(Int32 n, Int32 k) {
	if (!k) return 0;
	Int32 tmp = abs(n % k);
	if (tmp) n += (n > -1 ? (abs(k) - tmp) : (tmp));
	return n;
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
	if (IsElementEditable(objectId, syncSettings, true)) {
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
bool CheckElementType(const API_ElemTypeID& elementType, const SyncSettings& syncSettings) {
	if (elementType == API_DimensionID)
		return true;
	if (syncSettings.wallS &&
		(elementType == API_WallID || elementType == API_ColumnID || elementType == API_BeamID || elementType == API_SlabID ||
			elementType == API_RoofID || elementType == API_MeshID || elementType == API_ShellID ||
			elementType == API_MorphID ||
			elementType == API_BeamSegmentID ||
			elementType == API_ColumnSegmentID))
		return true;
	if (syncSettings.objS &&
		(elementType == API_StairID || elementType == API_RiserID ||
			elementType == API_TreadID || elementType == API_StairStructureID ||
			elementType == API_ObjectID ||
			elementType == API_ZoneID ||
			elementType == API_LampID))
		return true;
	if (syncSettings.objS &&
		(elementType == API_RailingID || elementType == API_RailingToprailID || elementType == API_RailingHandrailID ||
			elementType == API_RailingRailID || elementType == API_RailingPostID || elementType == API_RailingInnerPostID ||
			elementType == API_RailingBalusterID || elementType == API_RailingPanelID || elementType == API_RailingSegmentID ||
			elementType == API_RailingNodeID || elementType == API_RailingBalusterSetID || elementType == API_RailingPatternID ||
			elementType == API_RailingToprailEndID || elementType == API_RailingHandrailEndID ||
			elementType == API_RailingRailEndID ||
			elementType == API_RailingToprailConnectionID ||
			elementType == API_RailingHandrailConnectionID ||
			elementType == API_RailingRailConnectionID ||
			elementType == API_RailingEndFinishID))
		return true;
	if (syncSettings.cwallS &&
		(elementType == API_CurtainWallSegmentID ||
			elementType == API_CurtainWallFrameID ||
			elementType == API_CurtainWallJunctionID ||
			elementType == API_CurtainWallAccessoryID ||
			elementType == API_CurtainWallID ||
			elementType == API_CurtainWallPanelID))
		return true;
	if (syncSettings.widoS &&
		(elementType == API_WindowID ||
			elementType == API_DoorID ||
			elementType == API_SkylightID ||
			elementType == API_OpeningID))
		return true;
	return false;
}

// -----------------------------------------------------------------------------
// Проверяет возможность редактирования объекта (не находится в модуле, разблокирован, зарезервирован)
// -----------------------------------------------------------------------------
bool IsElementEditable(const API_Guid& objectId, const SyncSettings& syncSettings, const bool needCheckElementType) {

	// Проверяем - зарезервирован ли объект
	if (objectId == APINULLGuid) return false;
	if (!ACAPI_Element_Filter(objectId, APIFilt_InMyWorkspace)) return false;
	if (!ACAPI_Element_Filter(objectId, APIFilt_HasAccessRight)) return false;
	if (!ACAPI_Element_Filter(objectId, APIFilt_IsEditable)) return false;

	// Проверяем - на находится ли объект в модуле
	API_Elem_Head	tElemHead;
	BNZeroMemory(&tElemHead, sizeof(API_Elem_Head));
	tElemHead.guid = objectId;
	if (ACAPI_Element_GetHeader(&tElemHead) != NoError) return false;
	if (tElemHead.hotlinkGuid != APINULLGuid) return false;
	API_ElemTypeID eltype;
#ifdef ServerMainVers_2600
	eltype = tElemHead.type.typeID;
#else
	eltype = tElemHead.typeID;
#endif
	if (needCheckElementType && !CheckElementType(eltype, syncSettings)) return false;
	return true;
}

// -----------------------------------------------------------------------------
// Резервируем, разблокируем, вообщем - делаем элемент редактируемым
// Единственное, что может нас остановить - объект находится в модуле.
// -----------------------------------------------------------------------------
bool ReserveElement(const API_Guid& objectId, GSErrCode& err) {

	// Проверяем - на находится ли объект в модуле
	API_Elem_Head	tElemHead;
	BNZeroMemory(&tElemHead, sizeof(API_Elem_Head));
	tElemHead.guid = objectId;
	if (ACAPI_Element_GetHeader(&tElemHead) != NoError) return false;
	if (tElemHead.hotlinkGuid != APINULLGuid) return false; // С объектами в модуле сделать ничего не получится

	// Проверяем - зарезервирован ли объект и резервируем, если надо
	if (ACAPI_TeamworkControl_HasConnection() && !ACAPI_Element_Filter(objectId, APIFilt_InMyWorkspace)) {
#ifdef ServerMainVers_2400
		GS::PagedArray<API_Guid>	elements;
#else
		GS::Array<API_Guid>	elements;
#endif // ServerMainVers_2400

		GS::HashTable<API_Guid, short>  conflicts;
		elements.Push(objectId);
		ACAPI_TeamworkControl_ReserveElements(elements, &conflicts);
		if (!conflicts.IsEmpty()) return false; // Не получилось зарезервировать
	}
	if (ACAPI_Element_Filter(objectId, APIFilt_HasAccessRight)) {
		if (ACAPI_Element_Filter(objectId, APIFilt_IsEditable)) {
			if (ACAPI_Element_Filter(objectId, APIFilt_InMyWorkspace)) {
				return true;; // Зарезервировали
			}
		}
	};
	return false; // Не получилось зарезервировать
}

// -----------------------------------------------------------------------------
// Обновление отмеченных в меню пунктов
// -----------------------------------------------------------------------------
void MenuSetState(SyncSettings& syncSettings) {
	MenuItemCheckAC(Menu_MonAll, syncSettings.syncMon);
	MenuItemCheckAC(Menu_wallS, syncSettings.wallS);
	MenuItemCheckAC(Menu_widoS, syncSettings.widoS);
	MenuItemCheckAC(Menu_objS, syncSettings.objS);
	MenuItemCheckAC(Menu_cwallS, syncSettings.cwallS);
}

void msg_rep(const GS::UniString& modulename, const GS::UniString& reportString, const GSErrCode& err, const API_Guid& elemGuid) {
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
#if defined(AC_22) || defined(ServerMainVers_2300)
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
#endif // AC_22 or ServerMainVers_2300
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
		API_Elem_Head	elem_head = {};
		elem_head.guid = elemGuid;
		if (ACAPI_Element_GetHeader(&elem_head) == NoError) {
			GS::UniString elemName;
#ifdef ServerMainVers_2600
			if (ACAPI_Goodies_GetElemTypeName(elem_head.type, elemName) == NoError)
#else
			if (ACAPI_Goodies(APIAny_GetElemTypeNameID, (void*)elem_head.typeID, &elemName) == NoError)
#endif
				error_type = error_type + " type:" + elemName;
			API_Attribute layer;
			BNZeroMemory(&layer, sizeof(API_Attribute));
			layer.header.typeID = API_LayerID;
			layer.header.index = elem_head.layer;
			if (ACAPI_Attribute_Get(&layer) == NoError) error_type = error_type + " layer:" + layer.header.name;
		}
	}
	GS::UniString msg = modulename + ": " + reportString + " " + error_type;
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
		if (neig.neigID == APINeig_CurtainWall || neig.neigID == APINeig_CurtainWallOn) {
			GS::Array<API_Guid> panelGuid;
			err = GetCWElementsForCWall(neig.guid, panelGuid);
			if (err == NoError) {
				for (UInt32 i = 0; i < panelGuid.GetSize(); ++i) {
					guidArray.Push(panelGuid.Get(i));
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
#ifdef ServerMainVers_2600
	elementType = elementHead.type.typeID;
#else
	elementType = elementHead.typeID;
#endif
	return err;
}

#ifndef ServerMainVers_2600
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
#else
bool	GetElementTypeString(API_ElemType elemType, char* elemStr)
{
	GS::UniString	ustr;
	GSErrCode	err = ACAPI_Goodies_GetElemTypeName(elemType, ustr);
	if (err == NoError) {
		CHTruncate(ustr.ToCStr(), elemStr, ELEMSTR_LEN - 1);
		return true;
	}
	return false;
}
#endif // !ServerMainVers_2600

// -----------------------------------------------------------------------------
// Запись в свойство строки
// -----------------------------------------------------------------------------
GSErrCode WriteProp(const API_Guid& elemGuid, API_Property& property, GS::UniString& param_string) {
	GS::Int32 param_int = 0;
	bool param_bool = false;
	double param_real = 0;
	GSErrCode err = WriteProp(elemGuid, property, param_string, param_int, param_bool, param_real);
	return err;
}

// -----------------------------------------------------------------------------
// Запись в свойство ParamValue
// -----------------------------------------------------------------------------
GSErrCode WriteProp(const API_Guid& elemGuid, API_Property& property, ParamValue& pvalue) {
	GS::UniString param_string = pvalue.uniStringValue;
	GS::Int32 param_int = pvalue.intValue;
	bool param_bool = pvalue.boolValue;
	double param_real = pvalue.doubleValue;
	GSErrCode err = WriteProp(elemGuid, property, param_string, param_int, param_bool, param_real);
	return err;
}

// -----------------------------------------------------------------------------
// Синхронизация ParamValue и API_Property
// Возвращает true и подготовленное для записи свойство в случае отличий
// TODO Переписать всё под запись ParamValue
// -----------------------------------------------------------------------------
bool IsEqualPropParamValue(const ParamValue& pvalue, API_Property& property) {
	bool flag_rec = false;
	switch (property.definition.valueType) {
	case API_PropertyIntegerValueType:
		if (property.value.singleVariant.variant.intValue != pvalue.intValue) {
			property.value.singleVariant.variant.intValue = pvalue.intValue;
			flag_rec = true;
		}
		break;
	case API_PropertyRealValueType:
		if (!is_equal(property.value.singleVariant.variant.doubleValue, pvalue.doubleValue)) {
			property.value.singleVariant.variant.doubleValue = pvalue.doubleValue;
			flag_rec = true;
		}
		break;
	case API_PropertyBooleanValueType:
		if (property.value.singleVariant.variant.boolValue != pvalue.boolValue) {
			property.value.singleVariant.variant.boolValue = pvalue.boolValue;
			flag_rec = true;
		}
		break;
	case API_PropertyStringValueType:
		if (property.value.singleVariant.variant.uniStringValue != pvalue.uniStringValue) {
			property.value.singleVariant.variant.uniStringValue = pvalue.uniStringValue;
			flag_rec = true;
		}
		break;
	default:
		break;
	}
	if (flag_rec && property.value.singleVariant.variant.type == API_PropertyGuidValueType && property.definition.collectionType == API_PropertySingleChoiceEnumerationCollectionType) {
		API_Guid guidValue = APINULLGuid;
		API_SingleEnumerationVariant possible_value;

		// Для свойств с набором параметров необходимо задавать не само значение, а его GUID
		for (UInt32 i = 0; i < property.definition.possibleEnumValues.GetSize(); i++) {
			switch (property.definition.valueType) {
			case API_PropertyIntegerValueType:
				if (property.value.singleVariant.variant.intValue == property.definition.possibleEnumValues[i].displayVariant.intValue) {
					guidValue = property.definition.possibleEnumValues[i].keyVariant.guidValue;
				}
				break;
			case API_PropertyRealValueType:
				if (!is_equal(property.value.singleVariant.variant.doubleValue, property.definition.possibleEnumValues[i].displayVariant.doubleValue)) {
					guidValue = property.definition.possibleEnumValues[i].keyVariant.guidValue;
				}
				break;
			case API_PropertyBooleanValueType:
				if (property.value.singleVariant.variant.boolValue == property.definition.possibleEnumValues[i].displayVariant.boolValue) {
					guidValue = property.definition.possibleEnumValues[i].keyVariant.guidValue;
				}
				break;
			case API_PropertyStringValueType:
				if (property.value.singleVariant.variant.uniStringValue == property.definition.possibleEnumValues[i].displayVariant.uniStringValue) {
					guidValue = property.definition.possibleEnumValues[i].keyVariant.guidValue;
				}
				break;
			default:
				break;
			}
			if (guidValue != APINULLGuid) {
				property.value.singleVariant.variant.guidValue = guidValue;
				break;
			}
		}
		if (guidValue == APINULLGuid) flag_rec = false;
	}
	if (flag_rec) property.isDefault = false;
	return flag_rec;
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
		if (!is_equal(property.value.singleVariant.variant.doubleValue, param_real)) {
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

	if (flag_rec && property.value.singleVariant.variant.type == API_PropertyGuidValueType && property.definition.collectionType == API_PropertySingleChoiceEnumerationCollectionType) {
		API_Guid guidValue = APINULLGuid;
		API_SingleEnumerationVariant possible_value;

		// Для свойств с набором параметров необходимо задавать не само значение, а его GUID
		for (UInt32 i = 0; i < property.definition.possibleEnumValues.GetSize(); i++) {
			possible_value = property.definition.possibleEnumValues[i];
			switch (property.definition.valueType) {
			case API_PropertyIntegerValueType:
				if (property.value.singleVariant.variant.intValue == possible_value.displayVariant.intValue) {
					guidValue = possible_value.keyVariant.guidValue;
				}
				break;
			case API_PropertyRealValueType:
				if (!is_equal(property.value.singleVariant.variant.doubleValue, possible_value.displayVariant.doubleValue)) {
					guidValue = possible_value.keyVariant.guidValue;
				}
				break;
			case API_PropertyBooleanValueType:
				if (property.value.singleVariant.variant.boolValue == possible_value.displayVariant.boolValue) {
					guidValue = possible_value.keyVariant.guidValue;
				}
				break;
			case API_PropertyStringValueType:
				if (property.value.singleVariant.variant.uniStringValue == possible_value.displayVariant.uniStringValue) {
					guidValue = possible_value.keyVariant.guidValue;
				}
				break;
			default:
				break;
			}
			if (guidValue != APINULLGuid) {
				property.value.singleVariant.variant.guidValue = guidValue;
				break;
			}
		}
		if (guidValue == APINULLGuid) {
			flag_rec = false;
			msg_rep("WriteProp", "Failed to match values", err, elemGuid);
		}
	}
	if (flag_rec) {
		property.isDefault = false;
		if (property.value.variantStatus != API_VariantStatusNormal) property.value.variantStatus = API_VariantStatusNormal;
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
GSErrCode WriteProp2Prop(const API_Guid& elemGuid, const API_Property& property_from, API_Property& property)
{
	GSErrCode	err = NoError;
	bool write = true;

	// Есть ли вычисленное/доступное значение?
#if defined(AC_22) || defined(ServerMainVers_2300)
	bool isnoteval = (!property_from.isEvaluated);
#else
	bool isnoteval = (property_from.status != API_Property_HasValue);
#endif

	// Свойство недоступно или содержит невычисленное значение
	if (isnoteval && write) {
		err = APIERR_MISSINGCODE;
		write = false;
	}

	// Совпадают ли типы?
	if (property_from.definition.valueType != property.definition.valueType && write) {
		if (property.definition.valueType == API_PropertyStringValueType) {
			GS::UniString val;
#if defined(ServerMainVers_2500) || defined(ServerMainVers_2600)
			err = ACAPI_Property_GetPropertyValueString(property_from, &val);
#else
			val = PropertyTestHelpers::ToString(property_from);
#endif
			if (property.value.singleVariant.variant.uniStringValue != val) {
				property.value.singleVariant.variant.uniStringValue = val;
			}
			else {
				write = false;
			}
		}
		else {
			msg_rep("WriteProp2Prop", "Diff type " + property_from.definition.name + "<->" + property.definition.name, NoError, elemGuid);
			err = APIERR_MISSINGCODE;
			write = false;
		}
	}

	// Если нужно записать список текста в текст
	if (property.definition.collectionType != property_from.definition.collectionType && property.definition.valueType == API_PropertyStringValueType && property.definition.collectionType == API_PropertySingleCollectionType && write) {
		GS::UniString val;
#if defined(ServerMainVers_2500) || defined(ServerMainVers_2600)
		err = ACAPI_Property_GetPropertyValueString(property_from, &val);
#else
		val = PropertyTestHelpers::ToString(property_from);
#endif
		if (property.value.singleVariant.variant.uniStringValue != val) {
			property.value.singleVariant.variant.uniStringValue = val;
		}
		else {
			write = false; //Значения одинаковые
		}
	}

	// Обычное копирование
	if (property.definition.collectionType == property_from.definition.collectionType && property.definition.collectionType == API_PropertySingleCollectionType && write) {
		if (property.value.singleVariant != property_from.value.singleVariant) {
			property.value.singleVariant = property_from.value.singleVariant;
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
// Запись значения свойства в параметры объекта
// Пока записывает только GLOB_ID
// -----------------------------------------------------------------------------
GSErrCode WriteProp2Param(const API_Guid& elemGuid, GS::UniString paramName, API_Property& property) {
	GSErrCode		err = NoError;
	if (paramName.ToLowerCase() == "id") {
		GS::UniString val = PropertyTestHelpers::ToString(property);
		err = ACAPI_Database(APIDb_ChangeElementInfoStringID, (void*)&elemGuid, (void*)&val);
		if (err != NoError) msg_rep("WriteProp2Param - ID", "ACAPI_Database(APIDb_ChangeElementInfoStringID", err, elemGuid);
		return err;
	}
	return err;
}

// -----------------------------------------------------------------------------
// Делит строку по разделителю, возвращает кол-во частей
// -----------------------------------------------------------------------------
UInt32 StringSplt(const GS::UniString& instring, const GS::UniString& delim, GS::Array<GS::UniString>& partstring) {
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
			partstring.Push(parts.Get(i));
			n += 1;
		}
	}
	return n;
}

// --------------------------------------------------------------------
// Получение списка GUID панелей, рам и аксессуаров навесной стены
// --------------------------------------------------------------------
GSErrCode GetCWElementsForCWall(const API_Guid& cwGuid, GS::Array<API_Guid>& elementsSymbolGuids)
{
	API_Element      element = {};
	element.header.guid = cwGuid;
	GSErrCode err = ACAPI_Element_Get(&element);
	if (err != NoError || !element.header.hasMemo) {
		return err;
	}
	API_ElementMemo	memo = {};
	UInt64 mask = APIMemoMask_CWallFrames | APIMemoMask_CWallPanels | APIMemoMask_CWallJunctions | APIMemoMask_CWallAccessories;
	err = ACAPI_Element_GetMemo(cwGuid, &memo, mask);
	if (err != NoError) {
		ACAPI_DisposeElemMemoHdls(&memo);
		return err;
	}
	bool isDegenerate = false;
	const GSSize nPanels = BMGetPtrSize(reinterpret_cast<GSPtr>(memo.cWallPanels)) / sizeof(API_CWPanelType);
	if (nPanels > 0) {
		for (Int32 idx = 0; idx < nPanels; ++idx) {
			err = ACAPI_Database(APIDb_IsCWPanelDegenerateID, (void*)(&memo.cWallPanels[idx].head.guid), &isDegenerate);
			if (err == NoError && !isDegenerate && memo.cWallPanels[idx].hasSymbol && !memo.cWallPanels[idx].hidden) {
				elementsSymbolGuids.Push(std::move(memo.cWallPanels[idx].head.guid));
			}
		}
	}
	const GSSize nWallFrames = BMGetPtrSize(reinterpret_cast<GSPtr>(memo.cWallFrames)) / sizeof(API_CWFrameType);
	if (nWallFrames > 0) {
		for (Int32 idx = 0; idx < nWallFrames; ++idx) {
			if (memo.cWallFrames[idx].hasSymbol && !memo.cWallFrames[idx].deleteFlag && memo.cWallFrames[idx].objectType != APICWFrObjectType_Invisible) {
				elementsSymbolGuids.Push(std::move(memo.cWallFrames[idx].head.guid));
			}
		}
	}
	const GSSize nWallJunctions = BMGetPtrSize(reinterpret_cast<GSPtr>(memo.cWallJunctions)) / sizeof(API_CWJunctionType);
	if (nWallJunctions > 0) {
		for (Int32 idx = 0; idx < nWallJunctions; ++idx) {
			if (memo.cWallJunctions[idx].hasSymbol) {
				elementsSymbolGuids.Push(std::move(memo.cWallJunctions[idx].head.guid));
			}
		}
	}
	const GSSize nWallAccessories = BMGetPtrSize(reinterpret_cast<GSPtr>(memo.cWallAccessories)) / sizeof(API_CWAccessoryType);
	if (nWallAccessories > 0) {
		for (Int32 idx = 0; idx < nWallAccessories; ++idx) {
			if (memo.cWallAccessories[idx].hasSymbol) {
				elementsSymbolGuids.Push(std::move(memo.cWallAccessories[idx].head.guid));
			}
		}
	}
	ACAPI_DisposeElemMemoHdls(&memo);
	return err;
}

// --------------------------------------------------------------------
// Получение списка GUID элементов ограждения
// --------------------------------------------------------------------
GSErrCode GetRElementsForRailing(const API_Guid& elemGuid, GS::Array<API_Guid>& elementsGuids)
{
	API_Element      element = {};
	element.header.guid = elemGuid;
	GSErrCode err = ACAPI_Element_Get(&element);
	if (err != NoError || !element.header.hasMemo) {
		return err;
	}
	API_ElementMemo	memo = {};
	UInt64 mask = APIMemoMask_RailingPost | APIMemoMask_RailingInnerPost | APIMemoMask_RailingRail | APIMemoMask_RailingHandrail | APIMemoMask_RailingToprail | APIMemoMask_RailingPanel | APIMemoMask_RailingBaluster | APIMemoMask_RailingPattern | APIMemoMask_RailingBalusterSet | APIMemoMask_RailingRailEnd | APIMemoMask_RailingRailConnection;
	err = ACAPI_Element_GetMemo(elemGuid, &memo, mask);
	if (err != NoError) {
		ACAPI_DisposeElemMemoHdls(&memo);
		return err;
	}
	GSSize n = 0;
	n = BMGetPtrSize(reinterpret_cast<GSPtr>(memo.railingPosts)) / sizeof(API_RailingPostType);
	if (n > 0) {
		for (Int32 idx = 0; idx < n; ++idx) {
			elementsGuids.Push(std::move(memo.railingPosts[idx].head.guid));
		}
	}
	n = BMGetPtrSize(reinterpret_cast<GSPtr>(memo.railingRailEnds)) / sizeof(API_RailingRailEndType);
	if (n > 0) {
		for (Int32 idx = 0; idx < n; ++idx) {
			elementsGuids.Push(std::move(memo.railingRailEnds[idx].head.guid));
		}
	}
	n = BMGetPtrSize(reinterpret_cast<GSPtr>(memo.railingRails)) / sizeof(API_RailingRailType);
	if (n > 0) {
		for (Int32 idx = 0; idx < n; ++idx) {
			if (memo.railingRails[idx].visible) elementsGuids.Push(std::move(memo.railingRails[idx].head.guid));
		}
	}
	n = BMGetPtrSize(reinterpret_cast<GSPtr>(memo.railingToprails)) / sizeof(API_RailingToprailType);
	if (n > 0) {
		for (Int32 idx = 0; idx < n; ++idx) {
			if (memo.railingToprails[idx].visible) elementsGuids.Push(std::move(memo.railingToprails[idx].head.guid));
		}
	}
	n = BMGetPtrSize(reinterpret_cast<GSPtr>(memo.railingHandrails)) / sizeof(API_RailingHandrailType);
	if (n > 0) {
		for (Int32 idx = 0; idx < n; ++idx) {
			if (memo.railingHandrails[idx].visible) elementsGuids.Push(std::move(memo.railingHandrails[idx].head.guid));
		}
	}
	n = BMGetPtrSize(reinterpret_cast<GSPtr>(memo.railingInnerPosts)) / sizeof(API_RailingInnerPostType);
	if (n > 0) {
		for (Int32 idx = 0; idx < n; ++idx) {
			elementsGuids.Push(std::move(memo.railingInnerPosts[idx].head.guid));
		}
	}
	n = BMGetPtrSize(reinterpret_cast<GSPtr>(memo.railingBalusters)) / sizeof(API_RailingBalusterType);
	if (n > 0) {
		for (Int32 idx = 0; idx < n; ++idx) {
			elementsGuids.Push(std::move(memo.railingBalusters[idx].head.guid));
		}
	}
	n = BMGetPtrSize(reinterpret_cast<GSPtr>(memo.railingPanels)) / sizeof(API_RailingPanelType);
	if (n > 0) {
		for (Int32 idx = 0; idx < n; ++idx) {
			if (memo.railingPanels[idx].visible) elementsGuids.Push(std::move(memo.railingPanels[idx].head.guid));
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

// -----------------------------------------------------------------------------
// Получение значения IFC свойства по имени свойства
// -----------------------------------------------------------------------------
GSErrCode GetIFCPropertyByName(const API_Guid& elemGuid, const GS::UniString& tpropertyname, API_IFCProperty& property) {
	GS::Array<API_IFCProperty>     properties;
	GSErrCode err = NoError;
	err = ACAPI_Element_GetIFCProperties(elemGuid, false, &properties);
	if (err != NoError) msg_rep("GetIFCPropertyByName", "ACAPI_Element_GetIFCProperties " + tpropertyname, err, elemGuid);
	if (err == NoError) {
		GS::UniString propertyname = tpropertyname;
		propertyname.ReplaceAll("\\/", "@@");
		bool flag_find_gname = false;
		bool flag_find_pname = false;
		GS::UniString pname = "";
		GS::UniString gname = "";
		if (propertyname.Contains("/")) {
			GS::Array<GS::UniString> partstring;
			StringSplt(propertyname, "/", partstring);
			gname = partstring[0].ToLowerCase();
			pname = partstring[1].ToLowerCase();
			gname.ReplaceAll("@@", "/");
			pname.ReplaceAll("@@", "/");

			//Сначала ищем по группе/имени
			for (unsigned int i = 0; i < properties.GetSize(); i++)
			{
				API_IFCProperty prop = properties.Get(i);
				if (prop.head.propertySetName.ToLowerCase() == gname && prop.head.propertyName.ToLowerCase() == pname) {
					property = prop;
					return err;
				}
			}
		}
		else {
			propertyname.ReplaceAll("@@", "/");
			pname = propertyname.ToLowerCase();
			flag_find_pname = true;
		}

		//Если не нашли по группе - поищем по имени в других группах (если это имя попадалось, flag_find_pname).
		//Если имя группы задано не было - просто поищем по имени
		if (flag_find_pname) {
			for (unsigned int i = 0; i < properties.GetSize(); i++)
			{
				API_IFCProperty prop = properties.Get(i);
				if (prop.head.propertyName.ToLowerCase() == pname && flag_find_pname) {
					property = prop;
					return err;
				}
			}
		}
	}
	return APIERR_MISSINGCODE;
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
				if (groups[i].groupType == API_PropertyStaticBuiltInGroupType || groups[i].groupType == API_PropertyCustomGroupType) {
					if (groups[i].name.ToLowerCase() == gname) {
						GS::Array<API_PropertyDefinition> definitions;
						err = ACAPI_Property_GetPropertyDefinitions(groups[i].guid, definitions);
						if (err != NoError) msg_rep("GetPropertyByName", "ACAPI_Property_GetPropertyDefinitions " + propertyname, err, elemGuid);
						if (err == NoError) {
							for (UInt32 j = 0; j < definitions.GetSize(); j++) {
								if (definitions[j].name.ToLowerCase() == pname) {
									definition = definitions.Get(j);
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
				definition = definitions.Get(j);
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
				visibleProperties.Push(definitions.Get(i));
			}
		}
	}
	return error;
}

// -----------------------------------------------------------------------------
// Получение размеров Морфа
// Формирует два словаря - ParamDict& paramDict с именами параметров и ParamDictValue& pdictvalue со значениями
// -----------------------------------------------------------------------------
GSErrCode GetMorphParam(const API_Guid& elemGuid, ParamDictValue& pdictvalue) {
	API_Element      element = {};
	element.header.guid = elemGuid;
	GSErrCode err = ACAPI_Element_Get(&element);
	if (err == NoError && element.header.hasMemo) {
		API_ElementMemo  memo;
		BNZeroMemory(&memo, sizeof(API_ElementMemo));
		err = ACAPI_Element_GetMemo(element.header.guid, &memo);
		if (err != NoError || memo.morphBody == nullptr)
			return APIERR_GENERAL;
		if (err == NoError) {
			double L = 0;
			double Lx = 0;
			double Ly = 0;
			double Lz = 0;
			double Max_x = 0;
			double Max_y = 0;
			double Max_z = 0;
			double Min_x = 0;
			double Min_y = 0;
			double Min_z = 0;
			double A = 0;
			double B = 0;
			double ZZYZX = 0;
			if (memo.morphBody->IsWireBody() && !memo.morphBody->IsSolidBody()) {
				Int32 edgeCnt = memo.morphBody->GetEdgeCount();
				for (Int32 iEdge = 0; iEdge < edgeCnt; iEdge++) {
					const EDGE& edge = memo.morphBody->GetConstEdge(iEdge);
					const VERT& vtx1 = memo.morphBody->GetConstVertex(edge.vert1);
					const VERT& vtx2 = memo.morphBody->GetConstVertex(edge.vert2);
					double x1 = vtx1.x;
					double x2 = vtx2.x;
					double y1 = vtx1.y;
					double y2 = vtx2.y;
					double z1 = vtx1.z;
					double z2 = vtx2.z;
					double dx = pow(x2 - x1, 2);
					double dy = pow(y2 - y1, 2);
					double dz = pow(z2 - z1, 2);
					double dt = sqrt(dx + dy + dz);
					double dl = DoubleM2IntMM(sqrt(dx + dy + dz)) / 1000.0;
					double dlx = DoubleM2IntMM(sqrt(dy + dx)) / 1000.0;
					double dly = DoubleM2IntMM(sqrt(dx + dz)) / 1000.0;
					double dlz = DoubleM2IntMM(sqrt(dx + dy)) / 1000.0;
					L = L + dl;
					Lx = Lx + dlx;
					Ly = Ly + dly;
					Lz = Lz + dlz;
					Max_x = max(Max_x, x1);
					Max_x = max(Max_x, x2);
					Max_y = max(Max_y, y1);
					Max_y = max(Max_y, y2);
					Max_z = max(Max_z, z1);
					Max_z = max(Max_z, z2);
					Min_x = min(Min_x, x1);
					Min_x = min(Min_x, x2);
					Min_y = min(Min_y, y1);
					Min_y = min(Min_y, y2);
					Min_z = min(Min_z, z1);
					Min_z = min(Min_z, z2);
				}
				Max_x = DoubleM2IntMM(Max_x) / 1000.0;
				Max_y = DoubleM2IntMM(Max_y) / 1000.0;
				Max_z = DoubleM2IntMM(Max_z) / 1000.0;
				Min_x = DoubleM2IntMM(Min_x) / 1000.0;
				Min_y = DoubleM2IntMM(Min_y) / 1000.0;
				Min_z = DoubleM2IntMM(Min_z) / 1000.0;
				A = Max_x - Min_x;
				B = Max_y - Min_y;
				ZZYZX = Max_z - Min_z;
				AddParam2Dict("Morph:L", L, pdictvalue);
				AddParam2Dict("Morph:Lx", Lx, pdictvalue);
				AddParam2Dict("Morph:Ly", Ly, pdictvalue);
				AddParam2Dict("Morph:Lz", Lz, pdictvalue);
				AddParam2Dict("Morph:Max_x", Max_x, pdictvalue);
				AddParam2Dict("Morph:Max_y", Max_y, pdictvalue);
				AddParam2Dict("Morph:Max_z", Max_z, pdictvalue);
				AddParam2Dict("Morph:Min_x", Min_x, pdictvalue);
				AddParam2Dict("Morph:Min_y", Min_y, pdictvalue);
				AddParam2Dict("Morph:Min_z", Min_z, pdictvalue);
				AddParam2Dict("Morph:A", A, pdictvalue);
				AddParam2Dict("Morph:B", B, pdictvalue);
				AddParam2Dict("Morph:ZZYZX", ZZYZX, pdictvalue);
			}
		}
		ACAPI_DisposeElemMemoHdls(&memo);
	}
	return err;
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
// Возвращает elemType и elemGuid для корректного чтение параметров элементов навесной стены
// -----------------------------------------------------------------------------
GSErrCode GetGDLParametersHead(const API_Elem_Head& elem_head, API_ElemTypeID& elemType, API_Guid& elemGuid) {
	GSErrCode		err = NoError;
	API_Element element = {};
#ifdef ServerMainVers_2600
	switch (elem_head.type.typeID) {
#else
	switch (elem_head.typeID) {
#endif
	case API_CurtainWallPanelID:
		element.header = elem_head;
		err = ACAPI_Element_Get(&element);
		if (err == NoError) {
			elemGuid = element.cwPanel.symbolID;
			elemType = API_ObjectID;
		}
		break;
	case API_RailingBalusterID:
		element.header = elem_head;
		err = ACAPI_Element_Get(&element);
		if (err == NoError) {
			elemGuid = element.railingBaluster.symbID;
			elemType = API_ObjectID;
		}
		break;
	case API_RailingHandrailID:
		element.header = elem_head;
		err = ACAPI_Element_Get(&element);
		if (err == NoError) {
			elemGuid = element.railingHandrail.symbID;
			elemType = API_ObjectID;
		}
		break;
	default:
		elemGuid = elem_head.guid;
#ifdef ServerMainVers_2600
		elemType = elem_head.type.typeID;
#else
		elemType = elem_head.typeID;
#endif
		break;
	}
	return err;
}

// -----------------------------------------------------------------------------
// Возвращает список параметров API_AddParType
// -----------------------------------------------------------------------------
GSErrCode GetGDLParameters(const API_Guid & elemGuid, const API_ElemTypeID & elemType, API_AddParType * *&params)
{
	GSErrCode	err = NoError;
	API_ParamOwnerType	apiOwner = {};
	API_GetParamsType	apiParams = {};
	apiOwner.guid = elemGuid;
#ifdef ServerMainVers_2600
	apiOwner.type.typeID = elemType;
#else
	apiOwner.typeID = elemType;
#endif
	err = ACAPI_Goodies(APIAny_OpenParametersID, &apiOwner, nullptr);
	if (err != NoError) return err;
	err = ACAPI_Goodies(APIAny_GetActParametersID, &apiParams);
	if (err != NoError) {
		err = ACAPI_Goodies(APIAny_CloseParametersID);
		return err;
	}
	params = apiParams.params;
	err = ACAPI_Goodies(APIAny_CloseParametersID);
	return err;
}

// -----------------------------------------------------------------------------
// Прослойка для подмены API_Elem_Head в случае, если необходимо получить параметр элемента навесной стены
// -----------------------------------------------------------------------------
GSErrCode GetGDLParameters(const API_Elem_Head elem_head, API_AddParType * *&params) {
	API_ElemTypeID	elemType;
	API_Guid		elemGuid;
	GSErrCode	err = GetGDLParametersHead(elem_head, elemType, elemGuid);
	if (err == NoError) {
		err = GetGDLParameters(elemGuid, elemType, params);
	}
	return err;
}

// -----------------------------------------------------------------------------
// Возвращает индекс параметра по его имени
// -----------------------------------------------------------------------------
bool FindGDLParametersByName(const GS::UniString & paramName, API_AddParType * *&params, Int32 & inx) {
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
bool FindGDLParametersByDescription(const GS::UniString & paramName, const API_Elem_Head elem_head, Int32 & inx) {
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

// -----------------------------------------------------------------------------
// Поиск параметра GDL объекта по описанию или имени, с учётом типа элемента
// Level 3
// Возвращает сам параметр в формате API_AddParType
// -----------------------------------------------------------------------------
bool FindGDLParameters(const GS::UniString & paramName, const API_Elem_Head & elem_head, API_AddParType & nthParameter) {
	API_ElemTypeID eltype;
#ifdef ServerMainVers_2600
	eltype = elem_head.type.typeID;
#else
	eltype = elem_head.typeID;
#endif

	// Ещё раз проверим тип элемента
	if (eltype != API_ObjectID &&
		eltype != API_WindowID &&
		eltype != API_DoorID &&
		eltype != API_ZoneID &&
		eltype != API_LampID &&
		eltype != API_CurtainWallPanelID &&
		eltype != API_CurtainWallFrameID &&
		eltype != API_CurtainWallJunctionID &&
		eltype != API_CurtainWallAccessoryID &&
		eltype != API_RailingToprailID &&
		eltype != API_RailingHandrailID &&
		eltype != API_RailingRailID &&
		eltype != API_RailingPostID &&
		eltype != API_RailingInnerPostID &&
		eltype != API_RailingBalusterID &&
		eltype != API_RailingPanelID &&
		eltype != API_RailingToprailEndID &&
		eltype != API_RailingHandrailEndID &&
		eltype != API_RailingRailEndID &&
		eltype != API_RailingToprailConnectionID &&
		eltype != API_RailingHandrailConnectionID &&
		eltype != API_RailingRailConnectionID
		) {
		return false;
	}

	// Определяемся - как ищем. По имени или описанию?
	GS::UniString findstr = paramName.ToLowerCase();
	bool find_by_description = false;
	if (findstr.Contains("description:")) {
		if (eltype == API_CurtainWallPanelID) return false; // По описанию в панелях не ищет.
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

// -----------------------------------------------------------------------------
// Получение координат объекта
// Level 3
// symb_pos_x , symb_pos_y, symb_pos_z
// Для панелей навесной стены возвращает центр панели
// Для колонны или объекта - центр колонны и отм. низа
// Для зоны - центр зоны (без отметки, symb_pos_z = 0)
// -----------------------------------------------------------------------------
bool FindLibCoords(const GS::UniString & paramName, const API_Elem_Head & elem_head, API_AddParType & nthParameter) {
	API_Element element = {};
	element.header = elem_head;
	GSErrCode err = ACAPI_Element_Get(&element);
	if (err != NoError) return false;
	double x = 0; double y = 0; double z = 0;
	API_ElemTypeID eltype;
#ifdef ServerMainVers_2600
	eltype = elem_head.type.typeID;
#else
	eltype = elem_head.typeID;
#endif
	switch (eltype) {
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
	case API_ColumnID:
		x = element.column.origoPos.x;
		y = element.column.origoPos.y;
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
// Получить значение параметра с конвертацией типа данных OLD
// Level 1
// -----------------------------------------------------------------------------
bool GetLibParam(const API_Guid & elemGuid, const GS::UniString & paramName, GS::UniString & param_string, GS::Int32 & param_int, bool& param_bool, double& param_real) {
	ParamValue pvalue;
	pvalue.name = paramName;
	if (GetLibParam(elemGuid, paramName, pvalue)) {
		param_string = pvalue.uniStringValue;
		param_int = pvalue.intValue;
		param_bool = pvalue.boolValue;
		param_real = pvalue.doubleValue;
		return true;
	}
	return false;
}

// -----------------------------------------------------------------------------
// Получить значение свойства или параметра в ParamValue
// Level 1
// -----------------------------------------------------------------------------
bool GetParam(const API_Guid & elemGuid, const GS::UniString & paramName, ParamValue & pvalue) {
	GS::UniString& paramName_t = paramName.ToLowerCase();
	paramName_t.ReplaceAll("{", "");
	paramName_t.ReplaceAll("}", "");
	GS::UniString formatstring = GetFormatString(paramName_t);
	API_VariantType type = GetTypeString(paramName_t);
	if (paramName_t.Contains("property:")) {
		paramName_t.ReplaceAll("property:", "");
		bool flag_find = GetPropertyParam(elemGuid, paramName_t, pvalue);
		return flag_find;
	}
	if (paramName_t.Contains("Property:")) {
		paramName_t.ReplaceAll("Property:", "");
		bool flag_find = GetPropertyParam(elemGuid, paramName_t, pvalue);
		return flag_find;
	}
	if (paramName_t.Contains("Material:")) {
		paramName_t.ReplaceAll("Material:", "");
	}
	if (paramName_t.Contains("Info:")) {
		paramName_t.ReplaceAll("Info:", "");
	}
	if (paramName_t.Contains("IFC:")) {
		paramName_t.ReplaceAll("IFC:", "");
	}
	if (paramName_t.Contains("Morph:")) {
		paramName_t.ReplaceAll("Morph:", "");
	}
	bool flag_find = GetLibParam(elemGuid, paramName_t, pvalue);
	return flag_find;
}

// -----------------------------------------------------------------------------
// Получить значение параметра по его имени или описанию в ParamValue
// Level 2
// В зависимости от того, что ищем (координаты или параметры) и типа элемента (объект или элемент навенсной стены/ограждения)
// -----------------------------------------------------------------------------
bool GetLibParam(const API_Guid & elemGuid, const GS::UniString & paramName, ParamValue & pvalue) {
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
		}
		else {
			flag_find = FindGDLParameters(paramName, elem_head, nthParameter); //Ищём пареметры
		}
		if (!flag_find) return false;
	}
	else {
		msg_rep(" GetLibParam", "ACAPI_Element_GetHeader", err, elem_head.guid);
		return false;
	}
	flag_find = ConvParamValue(pvalue, nthParameter);
	return flag_find;
}

// -----------------------------------------------------------------------------
// Получить значение свойства по его имени в ParamValue
// -----------------------------------------------------------------------------
bool GetPropertyParam(const API_Guid & elemGuid, const GS::UniString & paramName, ParamValue & pvalue) {
	API_PropertyDefinition definitions = {};
	GSErrCode err = GetPropertyDefinitionByName(elemGuid, paramName, definitions);
	if (err != NoError) {
		msg_rep("GetParamByName", "GetPropertyDefinitionByName " + paramName, err, elemGuid);
		return false;
	}
	API_Property  property = {};
	err = ACAPI_Element_GetPropertyValue(elemGuid, definitions.guid, property);
	if (err != NoError) {
		msg_rep("GetParamByName", "ACAPI_Element_GetPropertyValue " + definitions.name, err, elemGuid);
		return false;
	}
	return ConvParamValue(pvalue, property);
}

// -----------------------------------------------------------------------------
// Конвертация параметров библиотечного элемента в ParamValue
// -----------------------------------------------------------------------------
bool ConvParamValue(ParamValue & pvalue, const API_AddParType & nthParameter) {
	GS::UniString param_string = "";
	GS::Int32 param_int = 0;
	double param_real = 0.0;
	bool param_bool = false;
	pvalue.canCalculate = false;

	// Определяем тип и вычисляем текстовое, целочисленное и дробное значение.
	if (nthParameter.typeID == APIParT_CString) {
		param_string = nthParameter.value.uStr;
		param_bool = (param_string.GetLength() > 0);
		if (param_bool) {
			param_int = 1;
			param_real = 1.0;
		}
	}
	else {
		param_real = round(nthParameter.value.real * 1000) / 1000;
		if (nthParameter.value.real - param_real > 0.001) param_real += 0.001;
		param_int = (GS::Int32)param_real;
		if (param_int / 1 < param_real) param_int += 1;
	}
	if (abs(param_real) > std::numeric_limits<double>::epsilon()) param_bool = true;

	// Если параметр не строковое - определяем текстовое значение конвертацией
	if (param_string.GetLength() == 0) {
		API_AttrTypeID attrType = API_ZombieAttrID;
		short attrInx = (short)param_int;
		switch (nthParameter.typeID) {
		case APIParT_Integer:
			param_string = GS::UniString::Printf("%d", param_int);
			pvalue.type = API_PropertyIntegerValueType;
			break;
		case APIParT_Boolean:
			if (param_bool) {
				param_string = RSGetIndString(AddOnStringsID, TrueId, ACAPI_GetOwnResModule());
				param_int = 1;
				param_real = 1.0;
			}
			else {
				param_string = RSGetIndString(AddOnStringsID, FalseId, ACAPI_GetOwnResModule());
				param_int = 0;
				param_real = 0.0;
			}
			pvalue.type = API_PropertyBooleanValueType;
			break;
		case APIParT_Length:
			param_string = GS::UniString::Printf("%.0f", param_real * 1000);
			pvalue.type = API_PropertyRealValueType;
			break;
		case APIParT_Angle:
			param_string = GS::UniString::Printf("%.1f", param_real);
			pvalue.type = API_PropertyRealValueType;
			break;
		case APIParT_RealNum:
			param_string = GS::UniString::Printf("%.3f", param_real);
			pvalue.type = API_PropertyRealValueType;
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
			return false;
			break;
		}
		if (attrType != API_ZombieAttrID) {
			API_Attribute	attrib = {};
			attrib.header.typeID = attrType;
			attrib.header.index = attrInx;
			if (ACAPI_Attribute_Get(&attrib) == NoError) {
				param_string = GS::UniString::Printf("%s", attrib.header.name);
				pvalue.type = API_PropertyStringValueType;
				if (param_bool) {
					param_int = 1;
					param_real = 1.0;
				}
			}
			else {
				return false;
			}
		}
	}
	pvalue.name = "{" + GS::UniString(nthParameter.name) + "}";
	pvalue.boolValue = param_bool;
	pvalue.doubleValue = param_real;
	pvalue.intValue = param_int;
	pvalue.uniStringValue = param_string;
	pvalue.canCalculate = true;
	return true;
}

// -----------------------------------------------------------------------------
// Конвертация свойства в ParamValue
// -----------------------------------------------------------------------------
bool ConvParamValue(ParamValue & pvalue, const API_Property & property) {
	GS::UniString fname;
	GetPropertyFullName(property.definition, fname);
	pvalue.name = "{Property:" + fname + "}";
	pvalue.uniStringValue = PropertyTestHelpers::ToString(property);
	pvalue.boolValue = false;
	pvalue.intValue = 0;
	pvalue.doubleValue = 0.0;
	pvalue.canCalculate = false;
	switch (property.value.singleVariant.variant.type) {
	case API_PropertyIntegerValueType:
		pvalue.intValue = property.value.singleVariant.variant.intValue;
		pvalue.doubleValue = property.value.singleVariant.variant.intValue * 1.0;
		if (pvalue.intValue > 0) pvalue.boolValue = true;
		pvalue.type = API_PropertyIntegerValueType;
		break;
	case API_PropertyRealValueType:
		pvalue.doubleValue = round(property.value.singleVariant.variant.doubleValue * 1000) / 1000;
		if (property.value.singleVariant.variant.doubleValue - pvalue.doubleValue > 0.001) pvalue.doubleValue += 0.001;
		pvalue.intValue = (GS::Int32)pvalue.doubleValue;
		if (pvalue.intValue / 1.0 < pvalue.doubleValue) pvalue.intValue += 1;
		if (abs(pvalue.doubleValue) > std::numeric_limits<double>::epsilon()) pvalue.boolValue = true;
		pvalue.type = API_PropertyRealValueType;
		break;
	case API_PropertyBooleanValueType:
		pvalue.boolValue = property.value.singleVariant.variant.boolValue;
		if (pvalue.boolValue) {
			pvalue.intValue = 1;
			pvalue.doubleValue = 1.0;
		}
		pvalue.type = API_PropertyBooleanValueType;
		break;
	case API_PropertyStringValueType:
	case API_PropertyGuidValueType:
		pvalue.type = API_PropertyStringValueType;
		pvalue.boolValue = (pvalue.uniStringValue.GetLength() > 0);
		if (pvalue.boolValue) {
			pvalue.intValue = 1;
			pvalue.doubleValue = 1.0;
		}
		break;
	case API_PropertyUndefinedValueType:
		return false;
		break;
	default:
		return false;
		break;
	}
	pvalue.canCalculate = true;
	return true;
}

// -----------------------------------------------------------------------------
// Конвертация целого числа в ParamValue
// -----------------------------------------------------------------------------
bool ConvParamValue(ParamValue & pvalue, const GS::UniString & paramName, const Int32 intValue) {
	pvalue.name = paramName;
	pvalue.type = API_PropertyIntegerValueType;
	pvalue.canCalculate = true;
	pvalue.intValue = intValue;
	pvalue.doubleValue = intValue * 1.0;
	if (pvalue.intValue > 0) pvalue.boolValue = true;
	pvalue.uniStringValue = GS::UniString::Printf("%d", intValue);
	return true;
}

// -----------------------------------------------------------------------------
// Конвертация double в ParamValue
// -----------------------------------------------------------------------------
bool ConvParamValue(ParamValue & pvalue, const GS::UniString & paramName, const double doubleValue) {
	pvalue.name = paramName;
	pvalue.type = API_PropertyRealValueType;
	pvalue.canCalculate = true;
	pvalue.intValue = (GS::Int32)doubleValue;
	pvalue.doubleValue = doubleValue;
	pvalue.boolValue = false;
	if (abs(pvalue.doubleValue) > std::numeric_limits<double>::epsilon()) pvalue.boolValue = true;
	pvalue.uniStringValue = GS::UniString::Printf("%.3f", doubleValue);
	return true;
}

// -----------------------------------------------------------------------------------------------------
// Добавление в словари ParamDict и ParamDictValue параметра с именем paramName и значением doubleValue
// -----------------------------------------------------------------------------------------------------
void AddParam2Dict(const GS::UniString & paramName, const double doubleValue, ParamDictValue & pdictvalue) {
	if (!pdictvalue.ContainsKey(paramName)) {
		ParamValue pvalue;
		ConvParamValue(pvalue, paramName, doubleValue);
		pdictvalue.Add(paramName, pvalue);
	}
}

// -----------------------------------------------------------------------------
// Обработка количества нулей и единиц измерения в имени свойства
// Удаляет из имени paramName найденные единицы измерения
// Возвращает строку для скармливания функции NumToStig
// -----------------------------------------------------------------------------
GS::UniString GetFormatString(GS::UniString & paramName) {
	GS::UniString formatstring = "";
	if (!paramName.Contains(".")) return formatstring;
	GS::Array<GS::UniString> partstring;
	UInt32 n = StringSplt(paramName, ".", partstring);
	if (StringSplt(paramName, ".", partstring) > 1) {
		if (partstring[n - 1].Contains("m") || partstring[n - 1].Contains(u8"м")) {
			formatstring = partstring[n - 1];
			paramName.ReplaceAll("." + formatstring, "");
			formatstring.ReplaceAll(u8"м", "m");
			formatstring.ReplaceAll(u8"д", "d");
			formatstring.ReplaceAll(u8"с", "c");
		}
	}
	return formatstring;
}

// -----------------------------------------------------------------------------
// Обработка типа данных в имени
// $ - вернуть строку
// # - вернуть целое число
// По умолчанию - double
// Удаляет из имени paramName найденные указания на тип данных
// -----------------------------------------------------------------------------
API_VariantType GetTypeString(GS::UniString & paramName) {
	API_VariantType type = API_PropertyRealValueType;
	if (paramName.Contains("$")) {
		paramName.ReplaceAll("$", "");
		type = API_PropertyStringValueType;
	}
	if (paramName.Contains("#")) {
		paramName.ReplaceAll("#", "");
		type = API_PropertyRealValueType;
	}
	return type;
}

// -----------------------------------------------------------------------------
// Извлекает из строки все имена свойств или параметров, заключенные в знаки %
// -----------------------------------------------------------------------------
bool GetParamNameDict(const GS::UniString & expression, ParamDict & paramDict) {
	GS::UniString outstring = expression;
	if (!outstring.Contains('{')) return (paramDict.GetSize() > 0);
	GS::UniString part = "";
	bool hasmorph = false;
	for (UInt32 i = 0; i < expression.Count('{'); i++) {
		part = outstring.GetSubstring('{', '}', 0);
		if (!paramDict.ContainsKey(part)) {
			paramDict.Add(part, true);
		}
		if (part.Contains("Morph:")) hasmorph = true;
		outstring.ReplaceAll("{" + part + "}", "");
	}
	if (hasmorph) {
		paramDict.Add("hasmorph", true);
		paramDict.Add("Morph:L", true);
	}
	return (paramDict.GetSize() > 0);
}

// -----------------------------------------------------------------------------
// Получение значений параметров из словаря для заданного элемента
// Имена в формате %ИМЯ_GDL_ПАРАМЕТРА% или %Property:ИМЯ_СВОЙСТВА%
// Также можно указывать тип параметра ($ - вывести значение строкой, # - использовать целое)
// Для перевода в другие единицы использовать %ИМЯ_ПАРАМЕТРА.ЕД_ИЗМ%
// -----------------------------------------------------------------------------
bool GetParamValueDict(const API_Guid & elemGuid, const ParamDict & paramDict, ParamDictValue & pdictvalue) {
	if (elemGuid == APINULLGuid) return false;
	if (paramDict.GetSize() == 0) return false;
	UInt32 n = 0;
	for (GS::HashTable<GS::UniString, bool>::ConstPairIterator cIt = paramDict.EnumeratePairs(); cIt != NULL; ++cIt) {
		const GS::UniString& paramName = *cIt->key;
		ParamValue pvalue;
		if (GetParam(elemGuid, paramName, pvalue)) {
			if (pvalue.canCalculate) {
				pdictvalue.Add(paramName, pvalue);
				n++;
			}
		}
	}
	return (n > 0);
}

// -----------------------------------------------------------------------------
// Замена имен параметров на значения в выражении
// Значения передаются словарём, вычисление значений см. GetParamValueDict
// -----------------------------------------------------------------------------
bool ReplaceParamInExpression(const ParamDictValue & pdictvalue, GS::UniString & expression) {
	if (pdictvalue.GetSize() == 0) return false;
	if (expression.IsEmpty()) return false;
	UInt32 n = 0;
	for (GS::HashTable<GS::UniString, ParamValue>::ConstPairIterator cIt = pdictvalue.EnumeratePairs(); cIt != NULL; ++cIt) {
		const ParamValue& pvalue = *cIt->value;
		if (pvalue.canCalculate) {
			expression.ReplaceAll(pvalue.name, pvalue.uniStringValue);
			n++;
		}
	}
	return (n > 0);
}

// -----------------------------------------------------------------------------
// Вычисление выражений, заключённых в < >
// Что не может вычислить - заменит на пустоту
// -----------------------------------------------------------------------------
bool EvalExpression(GS::UniString & unistring_expression) {
	if (unistring_expression.IsEmpty()) return false;
	if (!unistring_expression.Contains('<')) return false;
	GS::UniString part = "";
	GS::UniString texpression = unistring_expression;
	for (UInt32 i = 0; i < texpression.Count('<'); i++) {
		typedef double T;
		part = unistring_expression.GetSubstring('<', '>', 0);
		typedef exprtk::expression<T>   expression_t;
		typedef exprtk::parser<T>       parser_t;
		std::string expression_string(part.ToCStr(0, MaxUSize, CC_Cyrillic).Get());
		expression_t expression;
		parser_t parser;
		parser.compile(expression_string, expression);
		const T result = expression.value();
		GS::UniString rezult_txt = GS::UniString::Printf("%.3g", result);
		unistring_expression.ReplaceAll("<" + part + ">", rezult_txt);
	}
	return (!unistring_expression.IsEmpty());
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
		Int32 addzero = n_zero - (out.GetLength() - out.FindFirst(',') - 1);
		if (addzero > 0) out = out + GS::UniString::Printf("%*s", addzero, "0");
	}
	return out;
}

GS::UniString PropertyTestHelpers::ToString(const API_Variant & variant, const GS::UniString stringformat) {
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

GS::UniString PropertyTestHelpers::ToString(const API_Variant & variant) {
	return PropertyTestHelpers::ToString(variant, "");
}

GS::UniString PropertyTestHelpers::ToString(const API_Property & property) {
	return PropertyTestHelpers::ToString(property, "");
}

GS::UniString PropertyTestHelpers::ToString(const API_Property & property, const GS::UniString stringformat) {
	GS::UniString string;
	const API_PropertyValue* value;
#if defined(AC_22) || defined(ServerMainVers_2300)
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
	}
	else {
		value = &property.value;
	}
#endif
	switch (property.definition.collectionType) {
	case API_PropertySingleCollectionType: {
		string += ToString(value->singleVariant.variant, stringformat);
	} break;
	case API_PropertyListCollectionType: {
		for (UInt32 i = 0; i < value->listVariant.variants.GetSize(); i++) {
			string += ToString(value->listVariant.variants[i], stringformat);
			if (i != value->listVariant.variants.GetSize() - 1) {
				string += "; ";
			}
		}
	} break;
	case API_PropertySingleChoiceEnumerationCollectionType: {
#if defined(ServerMainVers_2500) || defined(ServerMainVers_2600)
		string += ToString(value->singleVariant.variant, stringformat);
#else // ServerMainVers_2500
		string += ToString(value->singleEnumVariant.displayVariant, stringformat);
#endif
	} break;
	case API_PropertyMultipleChoiceEnumerationCollectionType: {
#if defined(ServerMainVers_2500) || defined(ServerMainVers_2600)
		for (UInt32 i = 0; i < value->listVariant.variants.GetSize(); i++) {
			string += ToString(value->listVariant.variants[i], stringformat);
			if (i != value->listVariant.variants.GetSize() - 1) {
				string += "; ";
			}
			}
#else // ServerMainVers_2500
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

bool operator== (const API_Variant & lhs, const API_Variant & rhs)
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

bool operator== (const API_SingleVariant & lhs, const API_SingleVariant & rhs)
{
	return lhs.variant == rhs.variant;
}

bool operator== (const API_ListVariant & lhs, const API_ListVariant & rhs)
{
	return lhs.variants == rhs.variants;
}

bool operator== (const API_SingleEnumerationVariant & lhs, const API_SingleEnumerationVariant & rhs)
{
	return lhs.keyVariant == rhs.keyVariant && lhs.displayVariant == rhs.displayVariant;
}

#if !defined(ServerMainVers_2500) && !defined(ServerMainVers_2600)
bool operator== (const API_MultipleEnumerationVariant & lhs, const API_MultipleEnumerationVariant & rhs)
{
	return lhs.variants == rhs.variants;
}
#endif

bool Equals(const API_PropertyDefaultValue & lhs, const API_PropertyDefaultValue & rhs, API_PropertyCollectionType collType)
{
	if (lhs.hasExpression != rhs.hasExpression) {
		return false;
	}

	if (lhs.hasExpression) {
		return lhs.propertyExpressions == rhs.propertyExpressions;
}
	else {
		return Equals(lhs.basicValue, rhs.basicValue, collType);
	}
	}

bool Equals(const API_PropertyValue & lhs, const API_PropertyValue & rhs, API_PropertyCollectionType collType)
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
#if defined(ServerMainVers_2500) || defined(ServerMainVers_2600)
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
		DBBREAK();
		return false;
	}
}

bool operator== (const API_PropertyGroup & lhs, const API_PropertyGroup & rhs)
{
	return lhs.guid == rhs.guid &&
		lhs.name == rhs.name;
	}

bool operator== (const API_PropertyDefinition & lhs, const API_PropertyDefinition & rhs)
{
	return lhs.guid == rhs.guid &&
		lhs.groupGuid == rhs.groupGuid &&
		lhs.name == rhs.name &&
		lhs.description == rhs.description &&
		lhs.collectionType == rhs.collectionType &&
		lhs.valueType == rhs.valueType &&
		lhs.measureType == rhs.measureType &&
		Equals(lhs.defaultValue, rhs.defaultValue, lhs.collectionType) &&
		lhs.availability == rhs.availability &&
		lhs.possibleEnumValues == rhs.possibleEnumValues;
}

bool operator== (const API_Property & lhs, const API_Property & rhs)
{
	if (lhs.definition != rhs.definition || lhs.isDefault != rhs.isDefault) {
		return false;
	}
	if (!lhs.isDefault) {
		return Equals(lhs.value, rhs.value, lhs.definition.collectionType);
	}
	else {
		return true;
	}
}

// -----------------------------------------------------------------------------
// Удаление данных аддона из элемента
// -----------------------------------------------------------------------------
void DeleteElementUserData(const API_Guid & elemguid) {
	API_Elem_Head	tElemHead = {};
	tElemHead.guid = elemguid;
	API_ElementUserData userData = {};
	GSErrCode err = ACAPI_Element_GetUserData(&tElemHead, &userData);
	if (err == NoError && userData.dataHdl != nullptr) {
		err = ACAPI_Element_DeleteUserData(&tElemHead);
		msg_rep("Del user data", " ", NoError, APINULLGuid);
	}
	BMKillHandle(&userData.dataHdl);
	GS::Array<API_Guid> setGuids;
	err = ACAPI_ElementSet_Identify(elemguid, &setGuids);
	if (err == NoError) {
		USize nSet = setGuids.GetSize();
		if (nSet > 0) {
			for (UIndex i = 0; i < nSet; i++) {
				err = ACAPI_ElementSet_Delete(setGuids[i]);
				if (err != NoError) {
					DBPRINTF("Delete Element Set error: %d\n", err);
				}
			}
			GS::UniString intString = GS::UniString::Printf(" %d", nSet);
			msg_rep("Del set", intString, NoError, APINULLGuid);
		}
	}
}

// -----------------------------------------------------------------------------
// Удаление данных аддона из всех элементов
// -----------------------------------------------------------------------------
void DeleteElementsUserData()
{
	GSErrCode err = NoError;
	GS::Array<API_Guid> addonelemList;
	err = ACAPI_AddOnObject_GetObjectList(&addonelemList);
	USize ngl = addonelemList.GetSize();
	if (ngl > 0) {
		for (UIndex ii = 0; ii < ngl; ii++) {
			err = ACAPI_AddOnObject_DeleteObject(addonelemList[ii]);
		}
		GS::UniString intString = GS::UniString::Printf(" %d", ngl);
		msg_rep("Del addon obj", intString, NoError, APINULLGuid);
	}
	GS::Array<API_Guid> elemList;
	ACAPI_Element_GetElemList(API_ZombieElemID, &elemList, APIFilt_IsEditable | APIFilt_HasAccessRight);
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

void UnhideUnlockAllLayer(void)
{
	API_Attribute		attrib;
	API_AttributeIndex	count, i;
	GSErrCode			err;
	err = ACAPI_Attribute_GetNum(API_LayerID, &count);
	if (err != NoError) msg_rep("UnhideUnlockAllLayer", "ACAPI_Attribute_GetNum", err, APINULLGuid);
	if (err == NoError) {
		for (i = 2; i <= count; i++) {
			BNZeroMemory(&attrib, sizeof(API_Attribute));
			attrib.header.typeID = API_LayerID;
			attrib.header.index = i;
			err = ACAPI_Attribute_Get(&attrib);
			if (err != NoError) msg_rep("UnhideUnlockAllLayer", "ACAPI_Attribute_Get", err, APINULLGuid);
			if (err == NoError) {
				bool flag_write = false;
				if (attrib.header.flags & APILay_Hidden) {
					attrib.layer.head.flags |= !APILay_Hidden;
					flag_write = true;
				}
				if (attrib.header.flags & APILay_Locked) {
					attrib.layer.head.flags |= !APILay_Locked;
					flag_write = true;
				}
				if (flag_write) {
					err = ACAPI_Attribute_Modify(&attrib, NULL);
					if (err != NoError) msg_rep("UnhideUnlockAllLayer", attrib.header.name, err, APINULLGuid);
				}
			}
		}
	}
	return;
}