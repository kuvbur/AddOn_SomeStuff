//------------ kuvbur 2022 ------------
#include	<math.h>
#include	<cmath>
#include	<limits>
#include	"APIEnvir.h"
#include	"ACAPinc.h"
#ifdef AC_25
#include	"APICommon25.h"
#endif // AC_25
#ifdef AC_26
#include	"APICommon26.h"
#endif // AC_26
#include	"Helpers.hpp"
#include	"Model3D/model.h"
#include	"Model3D/MeshBody.hpp"
#include	"Dimensions.hpp"
#include	"VectorImageIterator.hpp"
#include	"ProfileVectorImageOperations.hpp"
#include	"ProfileAdditionalInfo.hpp"

// --------------------------------------------------------------------
// Сравнение double c учётом точности
// --------------------------------------------------------------------
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
	if ((ignoreval.ToLowerCase() == "empty" || ignoreval.ToLowerCase() == u8"пусто") && val.GetLength() < 1) {
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
// Замена \n на перенос строки
// -----------------------------------------------------------------------------
void ReplaceCR(GS::UniString& val, bool clear) {
	GS::UniString p = "\\n";
	if (val.Contains(p)) {
		if (!clear) {
			for (UInt32 i = 0; i < val.Count(p); i++) {
				UIndex inx = val.FindFirst(p);
				val.ReplaceFirst(p, "");
				val.SetChar(inx, CharCR);
			}
		}
		else {
			val.ReplaceAll(p, "");
		}
	}
}

void GetNumSymbSpase(GS::UniString& outstring, GS::UniChar symb, char charrepl) {

	//Ищем указание длины строки
	Int32 stringlen = 0;
	GS::UniString part = "";
	if (outstring.Contains(symb)) {
		part = outstring.GetSubstring(symb, ' ', 0);
		if (!part.IsEmpty() && part.GetLength() < 4)
			stringlen = std::atoi(part.ToCStr());
		if (stringlen > 0) part = symb + part;
	}
	if (stringlen > 0) {
		Int32 modlen = outstring.GetLength() - part.GetLength() - 1;
		Int32 addspace = stringlen - modlen;
		if (modlen > stringlen) {
			addspace = modlen % stringlen;
		}
		outstring.ReplaceAll(part + ' ', GS::UniString::Printf("%s", std::string(addspace, charrepl).c_str()));
	}
}

void ReplaceSymbSpase(GS::UniString& outstring) {
	GetNumSymbSpase(outstring, '~', u8' ');
	GetNumSymbSpase(outstring, '@', CharTAB);
	outstring.ReplaceAll("\\TAB", u8"\u0009");
	outstring.ReplaceAll("\\CRLF", u8"\u000D\u000A");
	outstring.ReplaceAll("\\CR", u8"\u000D");
	outstring.ReplaceAll("\\LF", u8"\u000A");
	outstring.ReplaceAll("\\PS", u8"\u2029");
	outstring.ReplaceAll("\\LS", u8"\u2028");
	outstring.ReplaceAll("\\NEL", u8"\u0085");
	outstring.ReplaceAll("\\NL", u8"\u2424");
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
GSErrCode	AttachObserver(const API_Guid& objectId, const SyncSettings& syncSettings) {
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
#ifdef AC_26
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
	(void)err;

	// Проверяем - на находится ли объект в модуле
	API_Elem_Head	tElemHead;
	BNZeroMemory(&tElemHead, sizeof(API_Elem_Head));
	tElemHead.guid = objectId;
	if (ACAPI_Element_GetHeader(&tElemHead) != NoError) return false;
	if (tElemHead.hotlinkGuid != APINULLGuid) return false; // С объектами в модуле сделать ничего не получится

	// Проверяем - зарезервирован ли объект и резервируем, если надо
	if (ACAPI_TeamworkControl_HasConnection() && !ACAPI_Element_Filter(objectId, APIFilt_InMyWorkspace)) {
#ifdef AC_24
		GS::PagedArray<API_Guid>	elements;
#else
		GS::Array<API_Guid>	elements;
#endif // AC_24

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

void msg_rep(const GS::UniString& modulename, const GS::UniString& reportString, const GSErrCode& err, const API_Guid& elemGuid) {
	GS::UniString error_type = "";
	if (err != NoError) {
		switch (err) {
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
		API_Elem_Head	elem_head = {};
		elem_head.guid = elemGuid;
		if (ACAPI_Element_GetHeader(&elem_head) == NoError) {
			GS::UniString elemName;
#ifdef AC_26
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

void	MenuItemCheckAC(short itemInd, bool checked) {
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

GS::Array<API_Guid>	GetSelectedElements(bool assertIfNoSel /* = true*/, bool onlyEditable /*= true*/, bool addSubelement /*= true*/) {
	SyncSettings syncSettings(false, false, true, true, true, true, false);
	LoadSyncSettingsFromPreferences(syncSettings);
	return GetSelectedElements(assertIfNoSel, onlyEditable, syncSettings, addSubelement);
}

// -----------------------------------------------------------------------------
// Получить массив Guid выбранных элементов
// -----------------------------------------------------------------------------
GS::Array<API_Guid>	GetSelectedElements(bool assertIfNoSel /* = true*/, bool onlyEditable /*= true*/, SyncSettings& syncSettings, bool addSubelement) {
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

		// Получаем список связанных элементов
		guidArray.Push(neig.guid);
		if (addSubelement) {
			API_ElemTypeID elementType;
			API_NeigID neigID = neig.neigID;
			GSErrCode err = ACAPI_Goodies(APIAny_NeigIDToElemTypeID, &neigID, &elementType);
			if (err == NoError) GetRelationsElement(neig.guid, elementType, syncSettings, guidArray);
		}
	}
	return guidArray;
#endif // AC_22
}

void CallOnSelectedElemSettings(void (*function)(const API_Guid&, const SyncSettings&), bool assertIfNoSel /* = true*/, bool onlyEditable /* = true*/, const SyncSettings& syncSettings, GS::UniString& funcname, bool addSubelement) {
	GS::Array<API_Guid> guidArray = GetSelectedElements(assertIfNoSel, onlyEditable, addSubelement);
	if (!guidArray.IsEmpty()) {
		GS::UniString subtitle("working...");
		short nPhase = 1;
		ACAPI_Interface(APIIo_InitProcessWindowID, &funcname, &nPhase);
		long time_start = clock();
		for (UInt32 i = 0; i < guidArray.GetSize(); i++) {
			function(guidArray[i], syncSettings);
			ACAPI_Interface(APIIo_SetNextProcessPhaseID, &subtitle, &i);
			if (ACAPI_Interface(APIIo_IsProcessCanceledID, nullptr, nullptr)) {
				return;
			}
		}
		long time_end = clock();
		GS::UniString time = GS::UniString::Printf(" %d s", (time_end - time_start) / 1000);
		GS::UniString intString = GS::UniString::Printf(" %d qty", guidArray.GetSize());
		msg_rep(funcname + " Selected", intString + time, NoError, APINULLGuid);
		ACAPI_Interface(APIIo_CloseProcessWindowID, nullptr, nullptr);
	}
}

void CallOnSelectedElemSettings(void (*function)(const API_Guid&, const SyncSettings&, ParamDictValue& propertyParams, ParamDictElement& paramToWrite), bool assertIfNoSel /* = true*/, bool onlyEditable /* = true*/, const SyncSettings& syncSettings, GS::UniString& funcname, bool addSubelement) {
	GS::Array<API_Guid> guidArray = GetSelectedElements(assertIfNoSel, onlyEditable, addSubelement);
	if (guidArray.IsEmpty()) return;
	ParamDictValue propertyParams = {};
	ParamHelpers::GetAllPropertyDefinitionToParamDict(propertyParams);
	ParamDictElement paramToWrite;
	GS::UniString subtitle("working...");
	short nPhase = 1;
	ACAPI_Interface(APIIo_InitProcessWindowID, &funcname, &nPhase);
	long time_start = clock();

	DimRules dimrules = {};
	GSErrCode err = DimReadPref(dimrules);
	bool needdimround = (!dimrules.IsEmpty() && err == NoError);

	for (UInt32 i = 0; i < guidArray.GetSize(); i++) {
		function(guidArray[i], syncSettings, propertyParams, paramToWrite);
		if (needdimround) err = DimAutoRound(guidArray[i], dimrules, propertyParams);
		ACAPI_Interface(APIIo_SetNextProcessPhaseID, &subtitle, &i);
		if (ACAPI_Interface(APIIo_IsProcessCanceledID, nullptr, nullptr)) {
			return;
		}
	}
	if (!paramToWrite.IsEmpty()) {
		ParamHelpers::ElementsWrite(paramToWrite);
	}
	else {
		msg_rep(funcname, "No data to write", NoError, APINULLGuid);
	}
	long time_end = clock();
	GS::UniString time = GS::UniString::Printf(" %d s", (time_end - time_start) / 1000);
	GS::UniString intString = GS::UniString::Printf(" %d qty", guidArray.GetSize());
	msg_rep(funcname + " Selected", intString + time, NoError, APINULLGuid);
	ACAPI_Interface(APIIo_CloseProcessWindowID, nullptr, nullptr);
}

// -----------------------------------------------------------------------------
// Вызов функции для выбранных элементов
//	(функция должна принимать в качетве аргумента API_Guid
// -----------------------------------------------------------------------------
void CallOnSelectedElem(void (*function)(const API_Guid&), bool assertIfNoSel /* = true*/, bool onlyEditable /* = true*/, GS::UniString& funcname, bool addSubelement) {
	GS::Array<API_Guid> guidArray = GetSelectedElements(assertIfNoSel, onlyEditable, addSubelement);
	if (!guidArray.IsEmpty()) {
		long time_start = clock();
		GS::UniString subtitle("working...");
		short nPhase = 1;
		ACAPI_Interface(APIIo_InitProcessWindowID, &funcname, &nPhase);
		for (UInt32 i = 0; i < guidArray.GetSize(); i++) {
			ACAPI_Interface(APIIo_SetNextProcessPhaseID, &subtitle, &i);
			function(guidArray[i]);
			if (ACAPI_Interface(APIIo_IsProcessCanceledID, nullptr, nullptr)) {
				return;
			}
		}
		long time_end = clock();
		GS::UniString time = GS::UniString::Printf(" %d ms", (time_end - time_start) / 1000);
		GS::UniString intString = GS::UniString::Printf(" %d qty", guidArray.GetSize());
		msg_rep(funcname + " Selected", intString + time, NoError, APINULLGuid);
		ACAPI_Interface(APIIo_CloseProcessWindowID, nullptr, nullptr);
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
#ifdef AC_26
	elementType = elementHead.type.typeID;
#else
	elementType = elementHead.typeID;
#endif
	return err;
}

#ifndef AC_26
bool	GetElementTypeString(API_ElemTypeID typeID, char* elemStr) {
	GS::UniString	ustr;
	GSErrCode	err = ACAPI_Goodies(APIAny_GetElemTypeNameID, (void*)typeID, &ustr);
	if (err == NoError) {
		CHTruncate(ustr.ToCStr(), elemStr, ELEMSTR_LEN - 1);
		return true;
	}
	return false;
}
#else
bool	GetElementTypeString(API_ElemType elemType, char* elemStr) {
	GS::UniString	ustr;
	GSErrCode	err = ACAPI_Goodies_GetElemTypeName(elemType, ustr);
	if (err == NoError) {
		CHTruncate(ustr.ToCStr(), elemStr, ELEMSTR_LEN - 1);
		return true;
	}
	return false;
}
#endif // !AC_26

// --------------------------------------------------------------------
// Поиск связанных элементов
// --------------------------------------------------------------------
void GetRelationsElement(const API_Guid& elemGuid, const SyncSettings& syncSettings, GS::Array<API_Guid>& subelemGuid) {
	API_ElemTypeID elementType;
	if (GetTypeByGUID(elemGuid, elementType) != NoError) return;
	GetRelationsElement(elemGuid, elementType, syncSettings, subelemGuid);
}

// --------------------------------------------------------------------
// Поиск связанных элементов
// --------------------------------------------------------------------
void GetRelationsElement(const API_Guid& elemGuid, const  API_ElemTypeID& elementType, const SyncSettings& syncSettings, GS::Array<API_Guid>& subelemGuid) {
	GSErrCode	err = NoError;
	API_RoomRelation	relData;
	GS::Array<API_ElemTypeID> typeinzone;
	switch (elementType) {
	case API_RailingID:
		if (syncSettings.cwallS) {
			err = GetRElementsForRailing(elemGuid, subelemGuid);
		}
		break;
	case API_CurtainWallID:
		if (syncSettings.cwallS) {
			err = GetRElementsForCWall(elemGuid, subelemGuid);
		}
		break;
	case API_CurtainWallSegmentID:
	case API_CurtainWallFrameID:
	case API_CurtainWallJunctionID:
	case API_CurtainWallAccessoryID:
	case API_CurtainWallPanelID:
		if (syncSettings.cwallS) {
			API_CWPanelRelation crelData;
			err = ACAPI_Element_GetRelations(elemGuid, API_ZoneID, &crelData);
			if (err != NoError) {
				if (crelData.fromRoom != APINULLGuid) subelemGuid.Push(crelData.fromRoom);
				if (crelData.toRoom != APINULLGuid) subelemGuid.Push(crelData.toRoom);
			}
		}
		break;
	case API_DoorID:
		if (syncSettings.widoS) {
			API_DoorRelation drelData;
			err = ACAPI_Element_GetRelations(elemGuid, API_ZoneID, &drelData);
			if (err != NoError) {
				if (drelData.fromRoom != APINULLGuid) subelemGuid.Push(drelData.fromRoom);
				if (drelData.toRoom != APINULLGuid) subelemGuid.Push(drelData.toRoom);
			}
		}
		break;
	case API_WindowID:
		if (syncSettings.widoS) {
			API_WindowRelation wrelData;
			err = ACAPI_Element_GetRelations(elemGuid, API_ZoneID, &wrelData);
			if (err != NoError) {
				if (wrelData.fromRoom != APINULLGuid) subelemGuid.Push(wrelData.fromRoom);
				if (wrelData.toRoom != APINULLGuid) subelemGuid.Push(wrelData.toRoom);
			}
		}
		break;
	case API_ZoneID:
		if (syncSettings.objS) {
			err = ACAPI_Element_GetRelations(elemGuid, API_ZombieElemID, &relData);
			if (err != NoError) {
				typeinzone.Push(API_ObjectID);
				typeinzone.Push(API_LampID);
				typeinzone.Push(API_StairID);
				typeinzone.Push(API_RiserID);
				typeinzone.Push(API_TreadID);
				typeinzone.Push(API_StairStructureID);
				if (syncSettings.wallS) {
					typeinzone.Push(API_WallID);
					typeinzone.Push(API_SlabID);
					typeinzone.Push(API_ColumnID);
					typeinzone.Push(API_BeamID);
					typeinzone.Push(API_RoofID);
					typeinzone.Push(API_ShellID);
					typeinzone.Push(API_MorphID);
				}
				if (syncSettings.widoS) {
					typeinzone.Push(API_WindowID);
					typeinzone.Push(API_DoorID);
				}
				if (syncSettings.cwallS) {

					//typeinzone.Push(API_RailingID);
					//typeinzone.Push(API_RailingToprailID);
					//typeinzone.Push(API_RailingHandrailID);
					//typeinzone.Push(API_RailingRailID);
					//typeinzone.Push(API_RailingPostID);
					//typeinzone.Push(API_RailingInnerPostID);
					//typeinzone.Push(API_RailingBalusterID);
					//typeinzone.Push(API_RailingPanelID);
					//typeinzone.Push(API_RailingSegmentID);
					//typeinzone.Push(API_RailingToprailEndID);
					//typeinzone.Push(API_RailingHandrailEndID);
					//typeinzone.Push(API_RailingRailEndID);
					//typeinzone.Push(API_RailingToprailConnectionID);
					//typeinzone.Push(API_RailingHandrailConnectionID);
					//typeinzone.Push(API_RailingRailConnectionID);
					//typeinzone.Push(API_RailingNodeID);
					typeinzone.Push(API_CurtainWallID);
					typeinzone.Push(API_CurtainWallFrameID);
					typeinzone.Push(API_CurtainWallPanelID);
					typeinzone.Push(API_CurtainWallJunctionID);
					typeinzone.Push(API_CurtainWallAccessoryID);
					typeinzone.Push(API_CurtainWallSegmentID);
				}
				typeinzone.Push(API_SkylightID);
				for (const API_ElemTypeID& typeelem : typeinzone) {
					if (relData.elementsGroupedByType.ContainsKey(typeelem)) {
						for (const API_Guid& elGuid : relData.elementsGroupedByType[typeelem]) {
							subelemGuid.Push(elGuid);
						}
					}
				}
			}
		}
		break;
	default:
		break;
	}
	ACAPI_DisposeRoomRelationHdls(&relData);
}

// -----------------------------------------------------------------------------
// Запись значения свойства в параметры объекта
// Пока записывает только GLOB_ID
// -----------------------------------------------------------------------------
GSErrCode WriteProp2Param(const API_Guid& elemGuid, GS::UniString paramName, API_Property& property) {
	GSErrCode		err = NoError;
	if (paramName.ToLowerCase() == "id") {
		GS::UniString val = PropertyHelpers::ToString(property);
		err = ACAPI_Database(APIDb_ChangeElementInfoStringID, (void*)&elemGuid, (void*)&val);
		if (err != NoError) msg_rep("WriteProp2Param - ID", "ACAPI_Database(APIDb_ChangeElementInfoStringID", err, elemGuid);
		return err;
	}
	return err;
}

// -----------------------------------------------------------------------------
// Возвращает уникальные вхождения текста
// -----------------------------------------------------------------------------
GS::UniString StringUnic(const GS::UniString& instring, const GS::UniString& delim) {
	if (!instring.Contains(delim)) return instring;
	GS::Array<GS::UniString> partstring;
	GS::UniString outsting = "";
	UInt32 n = StringSpltUnic(instring, delim, partstring);
	for (UInt32 i = 0; i < n; i++) {
		outsting = outsting + partstring[i];
		if (i < n - 1) outsting = outsting + delim;
	}
	return outsting;
}

// -----------------------------------------------------------------------------
// Возвращает уникальные вхождения текста
// -----------------------------------------------------------------------------
UInt32 StringSpltUnic(const GS::UniString& instring, const GS::UniString& delim, GS::Array<GS::UniString>& partstring) {
	if (!instring.Contains(delim)) {
		partstring.Push(instring);
		return 1;
	}
	GS::Array<GS::UniString> tpartstring;
	UInt32 n = StringSplt(instring, delim, tpartstring);
	std::map<std::string, int, doj::alphanum_less<std::string> > unic = {};
	for (UInt32 i = 0; i < n; i++) {
		std::string s = tpartstring[i].ToCStr(0, MaxUSize, CC_Cyrillic).Get();
		unic[s];
	}
	UInt32 nout = 0;
	for (std::map<std::string, int, doj::alphanum_less<std::string> >::iterator k = unic.begin(); k != unic.end(); ++k) {
		std::string s = k->first;
		GS::UniString unis = GS::UniString(s.c_str());
		partstring.Push(unis);
		nout = nout + 1;
	}
	return nout;
}

// -----------------------------------------------------------------------------
// Делит строку по разделителю, возвращает кол-во частей
// -----------------------------------------------------------------------------
UInt32 StringSplt(const GS::UniString& instring, const GS::UniString& delim, GS::Array<GS::UniString>& partstring) {
	if (!instring.Contains(delim)) {
		partstring.Push(instring);
		return 1;
	}
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
	if (!instring.Contains(delim) || !instring.Contains(filter)) {
		partstring.Push(instring);
		return 1;
	}
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
GSErrCode GetRElementsForCWall(const API_Guid& cwGuid, GS::Array<API_Guid>& elementsSymbolGuids) {
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
GSErrCode GetRElementsForRailing(const API_Guid& elemGuid, GS::Array<API_Guid>& elementsGuids) {
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
// Получение размеров Морфа
// Формирует словарь ParamDictValue& pdictvalue со значениями
// -----------------------------------------------------------------------------
bool ParamHelpers::ReadMorphParam(const API_Element& element, ParamDictValue& pdictvalue) {
	if (!element.header.hasMemo) return NoError;
	API_ElementMemo  memo;
	BNZeroMemory(&memo, sizeof(API_ElementMemo));
	GSErrCode err = ACAPI_Element_GetMemo(element.header.guid, &memo);
	if (err != NoError || memo.morphBody == nullptr) {
		ACAPI_DisposeElemMemoHdls(&memo);
		return false;
	}
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
			double dl = DoubleM2IntMM(sqrt(dx + dy + dz)) / 1000.0;
			double dlx = DoubleM2IntMM(sqrt(dy + dx)) / 1000.0;
			double dly = DoubleM2IntMM(sqrt(dx + dz)) / 1000.0;
			double dlz = DoubleM2IntMM(sqrt(dx + dy)) / 1000.0;
			L = L + dl;
			Lx = Lx + dlx;
			Ly = Ly + dly;
			Lz = Lz + dlz;
			Max_x = fmax(Max_x, x1);
			Max_x = fmax(Max_x, x2);
			Max_y = fmax(Max_y, y1);
			Max_y = fmax(Max_y, y2);
			Max_z = fmax(Max_z, z1);
			Max_z = fmax(Max_z, z2);
			Min_x = fmin(Min_x, x1);
			Min_x = fmin(Min_x, x2);
			Min_y = fmin(Min_y, y1);
			Min_y = fmin(Min_y, y2);
			Min_z = fmin(Min_z, z1);
			Min_z = fmin(Min_z, z2);
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
		ParamDictValue pdictvaluemorph;
		ParamHelpers::AddVal(pdictvaluemorph, element.header.guid, "morph:", "l", L);
		ParamHelpers::AddVal(pdictvaluemorph, element.header.guid, "morph:", "lx", Lx);
		ParamHelpers::AddVal(pdictvaluemorph, element.header.guid, "morph:", "ly", Ly);
		ParamHelpers::AddVal(pdictvaluemorph, element.header.guid, "morph:", "lz", Lz);
		ParamHelpers::AddVal(pdictvaluemorph, element.header.guid, "morph:", "max_x", Max_x);
		ParamHelpers::AddVal(pdictvaluemorph, element.header.guid, "morph:", "min_x", Min_x);
		ParamHelpers::AddVal(pdictvaluemorph, element.header.guid, "morph:", "max_y", Max_y);
		ParamHelpers::AddVal(pdictvaluemorph, element.header.guid, "morph:", "min_y", Min_y);
		ParamHelpers::AddVal(pdictvaluemorph, element.header.guid, "morph:", "max_z", Max_z);
		ParamHelpers::AddVal(pdictvaluemorph, element.header.guid, "morph:", "min_z", Min_z);
		ParamHelpers::AddVal(pdictvaluemorph, element.header.guid, "morph:", "a", A);
		ParamHelpers::AddVal(pdictvaluemorph, element.header.guid, "morph:", "b", B);
		ParamHelpers::AddVal(pdictvaluemorph, element.header.guid, "morph:", "zzyzx", ZZYZX);
		ParamHelpers::Compare(pdictvaluemorph, pdictvalue);
		ACAPI_DisposeElemMemoHdls(&memo);
		return true;
	}
	else {
		ACAPI_DisposeElemMemoHdls(&memo);
		return false;
	}
}

// -----------------------------------------------------------------------------
// Добавление пустого значения в словарь ParamDictValue
// -----------------------------------------------------------------------------
GS::UniString ParamHelpers::AddVal(ParamDictValue& params, const GS::UniString& name) {
	if (name.IsEmpty()) return "";
	GS::UniString rawname_prefix = "";
	GS::UniString name_ = name.ToLowerCase();

	// Ищём строку с указанием формата вывода (метры/миллиметры)
	GS::UniString stringformat = GetFormatString(name_);

	name_ = GetPropertyENGName(name_).ToLowerCase();

	// Проверяем - есть лу указатель на тип параметра (GDL, Property, IFC)
	if (name_.Contains(":")) {
		GS::Array<GS::UniString> partstring;
		UInt32 n = StringSplt(name_, ":", partstring);
		if (n > 1) {
			rawname_prefix = partstring[0] + ":";
			name_ = partstring[1].ToLowerCase();
		}
	}
	if (rawname_prefix.IsEmpty()) rawname_prefix = "gdl:";

	// Ищём строку с указанием формата вывода (метры/миллиметры)
	GS::UniString rawName = "{" + rawname_prefix + name_ + "}";
	if (!params.ContainsKey(rawName)) {
		ParamValue pvalue;
		pvalue.rawName = rawName;
		pvalue.name = name_;
		if (rawName.Contains("coord")) pvalue.fromCoord = true;
		if (rawName.Contains("gdl")) pvalue.fromGDLparam = true;
		if (rawName.Contains("escription:") || rawName.Contains("esc:")) {
			pvalue.fromGDLparam = true;
			pvalue.fromGDLdescription = true;
		}
		if (rawName.Contains("property")) pvalue.fromProperty = true;
		if (rawName.Contains("material")) pvalue.fromMaterial = true;
		if (rawName.Contains("info")) pvalue.fromInfo = true;
		if (rawName.Contains("ifc")) pvalue.fromIFCProperty = true;
		if (rawName.Contains("morph")) pvalue.fromMorph = true;
		pvalue.val.stringformat = stringformat;
		params.Add(rawName, pvalue);
		return rawName;
	}
	return "";
}

bool ParamHelpers::needAdd(ParamDictValue& params, GS::UniString& rawName) {
	bool addNew = false;
	if (rawName.Contains(CharENTER)) {
		UInt32 n = rawName.FindFirst(CharENTER);
		GS::UniString rawName_ = rawName.GetSubstring(0, n) + "}";
		addNew = params.ContainsKey(rawName_);
	}
	return addNew;
}

// --------------------------------------------------------------------
// Запись параметра ParamValue в словарь элементов ParamDictElement, если его там прежде не было
// --------------------------------------------------------------------
void ParamHelpers::AddParamValue2ParamDictElement(const ParamValue& param, ParamDictElement& paramToRead) {
	ParamHelpers::AddParamValue2ParamDictElement(param.fromGuid, param, paramToRead);
}

// --------------------------------------------------------------------
// Запись параметра ParamValue в словарь элементов ParamDictElement, если его там прежде не было
// --------------------------------------------------------------------
void ParamHelpers::AddParamValue2ParamDictElement(const API_Guid& elemGuid, const ParamValue& param, ParamDictElement& paramToRead) {
	GS::UniString rawName = param.rawName;
	if (paramToRead.ContainsKey(elemGuid)) {
		if (!paramToRead.Get(elemGuid).ContainsKey(rawName)) {
			paramToRead.Get(elemGuid).Add(rawName, param);
		}
	}
	else {
		ParamDictValue params;
		params.Add(rawName, param);
		paramToRead.Add(elemGuid, params);
	}
}

// --------------------------------------------------------------------
// Запись словаря ParamDictValue в словарь элементов ParamDictElement
// --------------------------------------------------------------------
void ParamHelpers::AddParamDictValue2ParamDictElement(ParamDictValue& params, const API_Guid& elemGuid, ParamDictElement& paramToRead) {
	if (paramToRead.ContainsKey(elemGuid)) {
		for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = params.EnumeratePairs(); cIt != NULL; ++cIt) {
			GS::UniString rawName = *cIt->key;
			if (!paramToRead.Get(elemGuid).ContainsKey(rawName)) {
				ParamValue& param = *cIt->value;
				if (param.fromGuid == APINULLGuid) param.fromGuid = elemGuid;
				paramToRead.Get(elemGuid).Add(rawName, param);
			}
		}
	}
	else {
		paramToRead.Add(elemGuid, params);
	}
}

// -----------------------------------------------------------------------------
// Добавление массива свойств в словарь
// -----------------------------------------------------------------------------
bool ParamHelpers::AddProperty(ParamDictValue& params, GS::Array<API_Property>& properties) {
	UInt32 nparams = params.GetSize();
	if (nparams < 1) return false;
	bool flag_find = false;
	for (UInt32 i = 0; i < properties.GetSize(); i++) {
		ParamValue pvalue;
		API_Property property = properties.Get(i);
		if (ParamHelpers::ConvValue(pvalue, property)) {
			GS::UniString rawName = pvalue.rawName;
			if (params.ContainsKey(rawName)) {
				params.Get(rawName) = pvalue;
				nparams--;
				flag_find = true;
				if (nparams == 0) {
					return flag_find;
				}
			}
			else {
				if (ParamHelpers::needAdd(params, rawName)) {
					params.Add(rawName, pvalue);
					flag_find = true;
				}
			}
		}
	}
	return flag_find;
}

// -----------------------------------------------------------------------------
// Добавление значения в словарь ParamDictValue
// -----------------------------------------------------------------------------
void ParamHelpers::AddVal(ParamDictValue& params, const API_Guid& elemGuid, const GS::UniString& rawName_prefix, const GS::UniString& name, const double& val) {
	ParamValue pvalue;
	pvalue.rawName = "{" + rawName_prefix + name.ToLowerCase() + "}";
	pvalue.name = name.ToLowerCase();
	ParamHelpers::ConvValue(pvalue, "", val);
	params.Add(pvalue.rawName, pvalue);
}

// -----------------------------------------------------------------------------
// Возвращает elemType и elemGuid для корректного чтение параметров элементов навесной стены
// -----------------------------------------------------------------------------
void GetGDLParametersHead(const API_Element& element, const API_Elem_Head& elem_head, API_ElemTypeID& elemType, API_Guid& elemGuid) {
#ifdef AC_26
	switch (elem_head.type.typeID) {
#else
	switch (elem_head.typeID) {
#endif
	case API_CurtainWallPanelID:
		elemGuid = element.cwPanel.symbolID;
		elemType = API_ObjectID;
		break;
	case API_RailingBalusterID:
		elemGuid = element.railingBaluster.symbID;
		elemType = API_ObjectID;
		break;
	case API_RailingHandrailID:
		elemGuid = element.railingHandrail.symbID;
		elemType = API_ObjectID;
		break;
	default:
		UNUSED_VARIABLE(element);
		elemGuid = elem_head.guid;
#ifdef AC_26
		elemType = elem_head.typeID;
#else
		elemType = elem_head.typeID;
#endif
		break;
	}
	return;
}

// -----------------------------------------------------------------------------
// Возвращает список параметров API_AddParType
// -----------------------------------------------------------------------------
GSErrCode GetGDLParameters(const API_ElemTypeID & elemType, const API_Guid & elemGuid, API_AddParType * *&params) {
	GSErrCode	err = NoError;
	API_ParamOwnerType	apiOwner = {};
	API_GetParamsType	apiParams = {};
	BNZeroMemory(&apiOwner, sizeof(API_ParamOwnerType));
	BNZeroMemory(&apiParams, sizeof(API_GetParamsType));
	apiOwner.guid = elemGuid;
#ifdef AC_26
	apiOwner.type.typeID = elemType;
#else
	apiOwner.typeID = elemType;
#endif
	err = ACAPI_Goodies(APIAny_OpenParametersID, &apiOwner, nullptr);
	if (err != NoError) {
		msg_rep("GetGDLParameters", "APIAny_OpenParametersID", err, elemGuid);
		return err;
	}
	err = ACAPI_Goodies(APIAny_GetActParametersID, &apiParams);
	if (err != NoError) {
		msg_rep("GetGDLParameters", "APIAny_GetActParametersID", err, elemGuid);
		err = ACAPI_Goodies(APIAny_CloseParametersID);
		if (err != NoError) msg_rep("GetGDLParameters", "APIAny_CloseParametersID", err, elemGuid);
		return err;
	}
	params = apiParams.params;
	err = ACAPI_Goodies(APIAny_CloseParametersID);
	if (err != NoError) msg_rep("GetGDLParameters", "APIAny_CloseParametersID", err, elemGuid);
	return err;
}

// -----------------------------------------------------------------------------
// Получение координат объекта
// symb_pos_x , symb_pos_y, symb_pos_z
// Для панелей навесной стены возвращает центр панели
// Для колонны или объекта - центр колонны и отм. низа
// Для зоны - центр зоны (без отметки, symb_pos_z = 0)
// -----------------------------------------------------------------------------
bool ParamHelpers::ReadElemCoords(const API_Element & element, ParamDictValue & params) {
	double x = 0; double y = 0; double z = 0; double angz = 0;
	double sx = 0; double sy = 0;
	double dx = 0; double dy = 0;
	double ex = 0; double ey = 0;
	double tolerance_coord = 0.1;
	bool hasSymbpos = false; bool hasLine = false;
	API_ElemTypeID eltype;
#ifdef AC_26
	eltype = element.header.type.typeID;
#else
	eltype = element.header.typeID;
#endif
	switch (eltype) {
	case API_CurtainWallPanelID:
		x = element.cwPanel.centroid.x;
		y = element.cwPanel.centroid.y;
		z = element.cwPanel.centroid.z;
		hasSymbpos = true;
		break;
	case API_ObjectID:
		x = element.object.pos.x;
		y = element.object.pos.y;
		z = element.object.level;
		angz = element.object.angle;
		hasSymbpos = true;
		break;
	case API_ZoneID:
		x = element.zone.pos.x;
		y = element.zone.pos.y;
		hasSymbpos = true;
		break;
	case API_ColumnID:
		x = element.column.origoPos.x;
		y = element.column.origoPos.y;
		angz = element.column.slantAngle;
		hasSymbpos = true;
		break;
	case API_WallID:
		sx = element.wall.begC.x;
		sy = element.wall.begC.y;
		ex = element.wall.endC.x;
		ey = element.wall.endC.y;
		hasLine = true;
		break;
	case API_BeamID:
		sx = element.beam.begC.x;
		sy = element.beam.begC.y;
		ex = element.beam.endC.x;
		ey = element.beam.endC.y;
		hasLine = true;
		break;
	default:
		return false;
	}
	ParamDictValue pdictvaluecoord;
	if (hasSymbpos) {
		double k = 10000.0;
		x = round(x * k) / k;
		y = round(y * k) / k;
		z = round(y * k) / k;
		ParamHelpers::AddVal(pdictvaluecoord, element.header.guid, "coord:", "symb_pos_x", x);
		ParamHelpers::AddVal(pdictvaluecoord, element.header.guid, "coord:", "symb_pos_y", y);
		ParamHelpers::AddVal(pdictvaluecoord, element.header.guid, "coord:", "symb_pos_z", z);
		double symb_pos_x_correct = abs(abs(x * k) - floor(abs(x * k)));
		double symb_pos_y_correct = abs(abs(y * k) - floor(abs(y * k)));
		double symb_pos_correct = (symb_pos_x_correct < tolerance_coord&& symb_pos_y_correct < tolerance_coord);
		ParamHelpers::AddVal(pdictvaluecoord, element.header.guid, "coord:", "symb_pos_x_correct", symb_pos_x_correct < tolerance_coord);
		ParamHelpers::AddVal(pdictvaluecoord, element.header.guid, "coord:", "symb_pos_y_correct", symb_pos_y_correct < tolerance_coord);
		ParamHelpers::AddVal(pdictvaluecoord, element.header.guid, "coord:", "symb_pos_correct", symb_pos_correct);
	}
	if (hasLine) {
		dx = ex - sx;
		dy = ey - sy;
		if (dx > 0 && dy >= 0) {
			angz = atan(dy / dx);
		}
		if (dx > 0 && dy < 0) {
			angz = atan(dy / dx) + 2 * PI;
		}
		if (dx < 0) {
			angz = atan(dy / dx) + PI;
		}
		if (dx == 0 && dy > 0) {
			angz = PI / 2;
		}
		if (dx == 0 && dy < 0) {
			angz = 3 * PI / 2;
		}
		if (dx == 0 && dy < 0) {
			angz = 0;
		}
		double k = 100000.0;
		sx = round(sx * k) / k;
		sy = round(sy * k) / k;
		ex = round(ex * k) / k;
		ey = round(ey * k) / k;
		ParamHelpers::AddVal(pdictvaluecoord, element.header.guid, "coord:", "symb_pos_sx", sx);
		ParamHelpers::AddVal(pdictvaluecoord, element.header.guid, "coord:", "symb_pos_sy", sy);
		ParamHelpers::AddVal(pdictvaluecoord, element.header.guid, "coord:", "symb_pos_ex", ex);
		ParamHelpers::AddVal(pdictvaluecoord, element.header.guid, "coord:", "symb_pos_ey", ey);
		double symb_pos_sx_correct = abs(abs(sx * k) - floor(abs(sx * k)));
		double symb_pos_sy_correct = abs(abs(sy * k) - floor(abs(sy * k)));
		double symb_pos_ex_correct = abs(abs(ex * k) - floor(abs(ex * k)));
		double symb_pos_ey_correct = abs(abs(ey * k) - floor(abs(ey * k)));
		double symb_pos_correct = (symb_pos_sx_correct < tolerance_coord&& symb_pos_sy_correct < tolerance_coord&& symb_pos_ex_correct < tolerance_coord&& symb_pos_ey_correct < tolerance_coord);
		ParamHelpers::AddVal(pdictvaluecoord, element.header.guid, "coord:", "symb_pos_sx_correct", symb_pos_sx_correct < tolerance_coord);
		ParamHelpers::AddVal(pdictvaluecoord, element.header.guid, "coord:", "symb_pos_sy_correct", symb_pos_sy_correct < tolerance_coord);
		ParamHelpers::AddVal(pdictvaluecoord, element.header.guid, "coord:", "symb_pos_ex_correct", symb_pos_ex_correct < tolerance_coord);
		ParamHelpers::AddVal(pdictvaluecoord, element.header.guid, "coord:", "symb_pos_ey_correct", symb_pos_ey_correct < tolerance_coord);
		ParamHelpers::AddVal(pdictvaluecoord, element.header.guid, "coord:", "symb_pos_correct", symb_pos_correct);
	}
	double k = 100.0;
	if (abs(angz) > 0.0001) {
		angz = round((angz * 180 / PI) * k) / k;
	}
	else {
		angz = 0.0;
	}
	double symb_rotangle_fraction = abs(abs(angz * k) - floor(abs(angz * k))) * 10.0;
	ParamHelpers::AddVal(pdictvaluecoord, element.header.guid, "coord:", "symb_rotangle", angz);
	ParamHelpers::AddVal(pdictvaluecoord, element.header.guid, "coord:", "symb_rotangle_fraction", symb_rotangle_fraction);
	ParamHelpers::AddVal(pdictvaluecoord, element.header.guid, "coord:", "symb_rotangle_correct", symb_rotangle_fraction < 1.0);
	ParamHelpers::AddVal(pdictvaluecoord, element.header.guid, "coord:", "symb_rotangle_mod5", fmod(angz, 5.0));
	ParamHelpers::AddVal(pdictvaluecoord, element.header.guid, "coord:", "symb_rotangle_mod10", fmod(angz, 10.0));
	ParamHelpers::AddVal(pdictvaluecoord, element.header.guid, "coord:", "symb_rotangle_mod45", fmod(angz, 45.0));
	ParamHelpers::AddVal(pdictvaluecoord, element.header.guid, "coord:", "symb_rotangle_mod90", fmod(angz, 90.0));
	ParamHelpers::AddVal(pdictvaluecoord, element.header.guid, "coord:", "symb_rotangle_mod180", fmod(angz, 180.0));
	ParamHelpers::Compare(pdictvaluecoord, params);
	return true;
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
// Получение имени внутренних свойств по русскому имени
// -----------------------------------------------------------------------------
GS::UniString GetPropertyENGName(GS::UniString & name) {
	if (name.IsEqual("property:id")) return "property:BuildingMaterialProperties/Building Material ID";
	if (name.IsEqual("property:n")) return "material:n";
	GS::UniString nameproperty = "";
	nameproperty = "property:" + RSGetIndString(AddOnStringsID, BuildingMaterialNameID, ACAPI_GetOwnResModule());
	if (name.IsEqual(nameproperty)) return "property:BuildingMaterialProperties/Building Material Name";

	nameproperty = "property:" + RSGetIndString(AddOnStringsID, BuildingMaterialDescriptionID, ACAPI_GetOwnResModule());
	if (name.IsEqual(nameproperty)) return "property:BuildingMaterialProperties/Building Material Description";

	nameproperty = "property:" + RSGetIndString(AddOnStringsID, BuildingMaterialDensityID, ACAPI_GetOwnResModule());
	if (name.IsEqual(nameproperty)) return "property:BuildingMaterialProperties/Building Material Density";

	nameproperty = "property:" + RSGetIndString(AddOnStringsID, BuildingMaterialManufacturerID, ACAPI_GetOwnResModule());
	if (name.IsEqual(nameproperty)) return "property:BuildingMaterialProperties/Building Material Manufacturer";

	nameproperty = "property:" + RSGetIndString(AddOnStringsID, ThicknessID, ACAPI_GetOwnResModule());
	if (name.IsEqual(nameproperty)) return "material:layer thickness";

	return name;
}

// -----------------------------------------------------------------------------
// Извлекает из строки все имена свойств или параметров, заключенные в знаки %
// -----------------------------------------------------------------------------
bool ParamHelpers::ParseParamNameMaterial(GS::UniString & expression, ParamDictValue & paramDict) {
	GS::UniString part = "";
	while (expression.Count('%') > 1) {
		part = expression.GetSubstring('%', '%', 0);
		if (!part.IsEmpty()) expression.ReplaceAll('%' + part + '%', "{property:" + part.ToLowerCase() + '}');
	}
	return ParamHelpers::ParseParamName(expression, paramDict);
}

// -----------------------------------------------------------------------------
// Извлекает из строки все имена свойств или параметров, заключенные в знаки { }
// -----------------------------------------------------------------------------
bool ParamHelpers::ParseParamName(GS::UniString & expression, ParamDictValue & paramDict) {
	GS::UniString tempstring = expression;
	if (!tempstring.Contains('{')) return (!paramDict.IsEmpty());
	GS::UniString part = "";
	while (tempstring.Contains('{') && tempstring.Contains('}')) {
		part = tempstring.GetSubstring('{', '}', 0);
		GS::UniString part_ = ParamHelpers::AddVal(paramDict, part);
		expression.ReplaceAll('{' + part + '}', part_);
		tempstring.ReplaceAll('{' + part + '}', "");
	}
	return (!paramDict.IsEmpty());
}

// -----------------------------------------------------------------------------
// Замена имен параметров на значения в выражении
// Значения передаются словарём, вычисление значений см. GetParamValueDict
// -----------------------------------------------------------------------------
bool ParamHelpers::ReplaceParamInExpression(const ParamDictValue & pdictvalue, GS::UniString & expression) {
	if (pdictvalue.IsEmpty()) return false;
	if (expression.IsEmpty()) return false;
	if (!expression.Contains('{')) return true;
	bool flag_find = false;
	GS::UniString attribsuffix = "";
	if (expression.Contains(CharENTER)) attribsuffix = CharENTER + expression.GetSubstring(CharENTER, '}', 0) + '}';
	GS::UniString part = "";
	GS::UniString partc = "";
	GS::UniString parts = "";
	GS::UniString val = "";
	while (expression.Contains('{') && expression.Contains('}')) {
		part = expression.GetSubstring('{', '}', 0);
		partc = '{' + part + '}';
		parts = '{' + part + attribsuffix;
		val = "";
		if (pdictvalue.ContainsKey(parts)) {
			ParamValue pvalue = pdictvalue.Get(parts);
			if (pvalue.isValid && pvalue.val.canCalculate) {
				val = ParamHelpers::ToString(pvalue);
				flag_find = true;
			}
		}
		else {
			if (pdictvalue.ContainsKey(partc)) {
				ParamValue pvalue = pdictvalue.Get(partc);
				if (pvalue.isValid && pvalue.val.canCalculate) {
					val = ParamHelpers::ToString(pvalue);
					flag_find = true;
				}
			}
		}
		expression.ReplaceAll('{' + part + '}', val);
	}
	return flag_find;
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
	while (unistring_expression.Contains('<') && unistring_expression.Contains('>')) {
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
// Список возможных префиксов полных имён параметров
// -----------------------------------------------------------------------------
void ParamHelpers::GetParamTypeList(GS::Array<GS::UniString>&paramTypesList) {
	if (!paramTypesList.IsEmpty()) paramTypesList.Clear();
	paramTypesList.Push("property");
	paramTypesList.Push("coord");
	paramTypesList.Push("gdl");
	paramTypesList.Push("info");
	paramTypesList.Push("ifc");
	paramTypesList.Push("morph");
	paramTypesList.Push("material");
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
GS::UniString PropertyHelpers::NumToString(const double& var, const GS::UniString stringformat) {
	if (abs(var) < 0.00000001) return "0";
	GS::UniString out = "";
	double outvar = var;
	Int32 n_zero = 3;
	Int32 krat = 0;
	GS::UniString outstringformat = stringformat;
	bool trim_zero = true;
	if (!stringformat.IsEmpty()) {
		if (stringformat.Contains("mm")) {
			n_zero = 0;
			outvar = var * 1000;
			outstringformat.ReplaceAll("mm", "");
		}
		if (stringformat.Contains("cm")) {
			n_zero = 1;
			outvar = var * 100;
			outstringformat.ReplaceAll("cm", "");
		}
		if (stringformat.Contains("dm")) {
			n_zero = 2;
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
			n_zero = 3;
			outvar = var;
			outstringformat.ReplaceAll("m", "");
		}

		// Принудительный вывод заданного кол-ва нулей после запятой
		if (outstringformat.Contains("0")) {
			outstringformat.ReplaceAll("0", "");
			outstringformat.Trim();
			if (!outstringformat.IsEmpty()) trim_zero = false;
		}
		if (!outstringformat.IsEmpty()) {

			// Кратность округления
			if (outstringformat.Contains("/")) {
				GS::Array<GS::UniString> params;
				UInt32 nparam = StringSplt(outstringformat, "/", params);
				if (params.GetSize() > 0) n_zero = std::atoi(params[0].ToCStr());
				if (params.GetSize() > 1) krat = std::atoi(params[0].ToCStr());
			}
			else {
				n_zero = std::atoi(outstringformat.ToCStr());
			}
		}
	}
	outvar = round(outvar * pow(10, n_zero)) / pow(10, n_zero);
	if (krat > 0) outvar = ceil_mod((GS::Int32)var, krat);
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

GS::UniString PropertyHelpers::ToString(const API_Variant & variant, const GS::UniString stringformat) {
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

GS::UniString PropertyHelpers::ToString(const API_Variant & variant) {
	return PropertyHelpers::ToString(variant, "");
}

GS::UniString PropertyHelpers::ToString(const API_Property & property) {
	return PropertyHelpers::ToString(property, "");
}

GS::UniString PropertyHelpers::ToString(const API_Property & property, const GS::UniString stringformat) {
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
	}
	else {
		value = &property.value;
	}
#endif
	switch (property.definition.collectionType) {
	case API_PropertySingleCollectionType:
	{
		string += ToString(value->singleVariant.variant, stringformat);
	} break;
	case API_PropertyListCollectionType:
	{
		for (UInt32 i = 0; i < value->listVariant.variants.GetSize(); i++) {
			string += ToString(value->listVariant.variants[i], stringformat);
			if (i != value->listVariant.variants.GetSize() - 1) {
				string += "; ";
			}
		}
	} break;
	case API_PropertySingleChoiceEnumerationCollectionType:
	{
#if defined(AC_25) || defined(AC_26)
		API_Guid guidValue = value->singleVariant.variant.guidValue;
		GS::Array<API_SingleEnumerationVariant> possibleEnumValues = property.definition.possibleEnumValues;
		for (UInt32 i = 0; i < possibleEnumValues.GetSize(); i++) {
			if (possibleEnumValues[i].keyVariant.guidValue == guidValue) {
				string += ToString(possibleEnumValues[i].displayVariant, stringformat);
				break;
			}
		}
#else // AC_25
		string += ToString(value->singleEnumVariant.displayVariant, stringformat);
#endif
	} break;
	case API_PropertyMultipleChoiceEnumerationCollectionType:
	{
#if defined(AC_25) || defined(AC_26)
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
	default:
	{
		break;
	}
	}
	return string;
}

bool operator== (const ParamValue & lhs, const ParamValue & rhs) {
	switch (lhs.val.type) {
	case API_PropertyIntegerValueType:
		return lhs.val.intValue == rhs.val.intValue;
	case API_PropertyRealValueType:
		return is_equal(lhs.val.doubleValue, rhs.val.doubleValue);
	case API_PropertyStringValueType:
		return lhs.val.uniStringValue == rhs.val.uniStringValue;
	case API_PropertyBooleanValueType:
		return lhs.val.boolValue == rhs.val.boolValue;
	default:
		return false;
	}
}

bool operator== (const API_Variant & lhs, const API_Variant & rhs) {
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

bool operator== (const API_SingleVariant & lhs, const API_SingleVariant & rhs) {
	return lhs.variant == rhs.variant;
}

bool operator== (const API_ListVariant & lhs, const API_ListVariant & rhs) {
	return lhs.variants == rhs.variants;
}

bool operator== (const API_SingleEnumerationVariant & lhs, const API_SingleEnumerationVariant & rhs) {
	return lhs.keyVariant == rhs.keyVariant && lhs.displayVariant == rhs.displayVariant;
}

#if !defined(AC_25) && !defined(AC_26)
bool operator== (const API_MultipleEnumerationVariant & lhs, const API_MultipleEnumerationVariant & rhs) {
	return lhs.variants == rhs.variants;
}
#endif

bool Equals(const API_PropertyDefaultValue & lhs, const API_PropertyDefaultValue & rhs, API_PropertyCollectionType collType) {
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

bool Equals(const API_PropertyValue & lhs, const API_PropertyValue & rhs, API_PropertyCollectionType collType) {
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
#if defined(AC_25) || defined(AC_26)
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

bool operator== (const API_PropertyGroup & lhs, const API_PropertyGroup & rhs) {
	return lhs.guid == rhs.guid &&
		lhs.name == rhs.name;
}

bool operator== (const API_PropertyDefinition & lhs, const API_PropertyDefinition & rhs) {
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

bool operator== (const API_Property & lhs, const API_Property & rhs) {
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
void DeleteElementsUserData() {
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

void UnhideUnlockAllLayer(void) {
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

// -----------------------------------------------------------------------------
// Конвертация значений ParamValue в свойства, находящиеся в нём
// Возвращает true если значения отличались
// -----------------------------------------------------------------------------
bool ParamHelpers::ToProperty(ParamValue & pvalue) {
	if (!pvalue.isValid) return false;
	if (!pvalue.fromPropertyDefinition) return false;
	API_Property property = pvalue.property;
	if (ParamHelpers::ToProperty(pvalue, property)) {
		pvalue.property = property;
		return true;
	}
	else {
		return false;
	}
}

// -----------------------------------------------------------------------------
// Синхронизация ParamValue и API_Property
// Возвращает true и подготовленное для записи свойство в случае отличий
// -----------------------------------------------------------------------------
bool ParamHelpers::ToProperty(const ParamValue & pvalue, API_Property & property) {
	if (property.definition.canValueBeEditable == false) {
		return false;
	}
	if (pvalue.isValid == false) {
		return false;
	}
	bool flag_rec = false;
	GS::UniString val = "";

	// TODO добавить обработку isDefault
//#if defined(AC_22) || defined(AC_23)
//	if (!property.isEvaluated) {
//		return string;
//	}
//	if (property.isDefault && !property.isEvaluated) {
//		value = &property.definition.defaultValue.basicValue;
//	}
//	else {
//		value = &property.value;
//	}
//#else
//	if (property.status == API_Property_NotAvailable) {
//		return string;
//	}
//	if (property.isDefault && property.status == API_Property_NotEvaluated) {
//		value = &property.definition.defaultValue.basicValue;
//	}
//	else {
//		value = &property.value;
//	}
//#endif
	switch (property.definition.valueType) {
	case API_PropertyIntegerValueType:
		if (property.value.singleVariant.variant.intValue != pvalue.val.intValue) {
			property.value.singleVariant.variant.intValue = pvalue.val.intValue;
			flag_rec = true;
		}
		break;
	case API_PropertyRealValueType:
		if (!is_equal(property.value.singleVariant.variant.doubleValue, pvalue.val.doubleValue)) {
			property.value.singleVariant.variant.doubleValue = pvalue.val.doubleValue;
			flag_rec = true;
		}
		break;
	case API_PropertyBooleanValueType:
		if (property.value.singleVariant.variant.boolValue != pvalue.val.boolValue) {
			property.value.singleVariant.variant.boolValue = pvalue.val.boolValue;
			flag_rec = true;
		}
		break;
	case API_PropertyStringValueType:
		val = ParamHelpers::ToString(pvalue);
		ReplaceCR(val, true);
		if (property.value.singleVariant.variant.uniStringValue != val) {
			property.value.singleVariant.variant.uniStringValue = val;
			flag_rec = true;
		}
		break;
	default:
		break;
	}
	if (flag_rec && property.value.singleVariant.variant.type == API_PropertyGuidValueType && property.definition.collectionType == API_PropertySingleChoiceEnumerationCollectionType) {
		API_Guid guidValue = APINULLGuid;
		GS::Array<API_SingleEnumerationVariant> possibleEnumValues = property.definition.possibleEnumValues;

		// Для свойств с набором параметров необходимо задавать не само значение, а его GUID
		for (UInt32 i = 0; i < possibleEnumValues.GetSize(); i++) {
			switch (property.definition.valueType) {
			case API_PropertyIntegerValueType:
				if (property.value.singleVariant.variant.intValue == possibleEnumValues[i].displayVariant.intValue) {
					guidValue = possibleEnumValues[i].keyVariant.guidValue;
				}
				break;
			case API_PropertyRealValueType:
				if (!is_equal(property.value.singleVariant.variant.doubleValue, possibleEnumValues[i].displayVariant.doubleValue)) {
					guidValue = possibleEnumValues[i].keyVariant.guidValue;
				}
				break;
			case API_PropertyBooleanValueType:
				if (property.value.singleVariant.variant.boolValue == possibleEnumValues[i].displayVariant.boolValue) {
					guidValue = possibleEnumValues[i].keyVariant.guidValue;
				}
				break;
			case API_PropertyStringValueType:
				if (property.value.singleVariant.variant.uniStringValue == possibleEnumValues[i].displayVariant.uniStringValue) {
					guidValue = possibleEnumValues[i].keyVariant.guidValue;
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
	if (flag_rec) {
		property.isDefault = false;
		if (property.value.variantStatus != API_VariantStatusNormal) property.value.variantStatus = API_VariantStatusNormal;
	}
	return flag_rec;
}

//--------------------------------------------------------------------------------------------------------------------------
//Ищет свойство property_flag_name в описании и по значению определяет - нужно ли обрабатывать элемент
//--------------------------------------------------------------------------------------------------------------------------
bool GetElemState(const API_Guid & elemGuid, const GS::Array<API_PropertyDefinition> definitions, GS::UniString property_flag_name) {
	if (definitions.IsEmpty()) return false;
	GSErrCode	err = NoError;
	for (UInt32 i = 0; i < definitions.GetSize(); i++) {
		if (!definitions[i].description.IsEmpty()) {
			if (definitions[i].description.Contains(property_flag_name)) {
				API_Property propertyflag = {};
				err = ACAPI_Element_GetPropertyValue(elemGuid, definitions[i].guid, propertyflag);
				if (err == NoError) {
					if (propertyflag.isDefault) {
						return propertyflag.definition.defaultValue.basicValue.singleVariant.variant.boolValue;
					}
					else {
						return propertyflag.value.singleVariant.variant.boolValue;
					}
				}
				else {
					return false;
				}
			}
		}
	}
	return false;
}

// --------------------------------------------------------------------
// Запись словаря параметров для множества элементов
// --------------------------------------------------------------------
void ParamHelpers::ElementsWrite(ParamDictElement & paramToWrite) {
	if (paramToWrite.IsEmpty()) return;
	for (GS::HashTable<API_Guid, ParamDictValue>::PairIterator cIt = paramToWrite.EnumeratePairs(); cIt != NULL; ++cIt) {
		ParamDictValue& params = *cIt->value;
		API_Guid elemGuid = *cIt->key;
		if (!params.IsEmpty()) {
			ParamHelpers::Write(elemGuid, params);
		}
	}
}

// --------------------------------------------------------------------
// Запись ParamDictValue в один элемент
// --------------------------------------------------------------------
void ParamHelpers::Write(const API_Guid & elemGuid, ParamDictValue & params) {
	if (params.IsEmpty()) return;

	// Получаем список возможных префиксов
	ParamDictValue paramByType;
	GS::Array<GS::UniString> paramTypesList;
	GetParamTypeList(paramTypesList);

	// Для каждого типа - свой способ получения данных. Поэтому разбиваем по типам и обрабатываем по-отдельности
	for (UInt32 i = 0; i < paramTypesList.GetSize(); i++) {
		GS::UniString paramType = paramTypesList[i];
		ParamDictValue paramByType;
		for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = params.EnumeratePairs(); cIt != NULL; ++cIt) {
			ParamValue& param = *cIt->value;
			if (param.rawName.Contains(paramType)) paramByType.Add(param.rawName, param);
		}
		if (!paramByType.IsEmpty()) {

			// Проходим поиском, специфичным для каждого типа
			// TODO переписать без использования сравнения с текстом!
			if (paramType.IsEqual("property")) {
				ParamHelpers::WritePropertyValues(elemGuid, paramByType);
			}
			if (paramType.IsEqual("gdl")) {
				ParamHelpers::WriteGDLValues(elemGuid, paramByType);
			}

			//if (paramType.IsEqual("ifc")) {}
		}
	}
}

// --------------------------------------------------------------------
// Запись ParamDictValue в GDL параметры и ID
// --------------------------------------------------------------------
void ParamHelpers::WriteGDLValues(const API_Guid & elemGuid, ParamDictValue & params) {
	if (params.IsEmpty()) return;
	if (elemGuid == APINULLGuid) return;

	// Поиск и запись ID
	for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = params.EnumeratePairs(); cIt != NULL; ++cIt) {
		ParamValue& param = *cIt->value;
		if (param.rawName.IsEqual("{gdl:id}")) {
			GS::UniString val = ParamHelpers::ToString(param);
			GSErrCode err = ACAPI_Database(APIDb_ChangeElementInfoStringID, (void*)&elemGuid, (void*)&val);
			if (err != NoError) msg_rep("WriteGDLValues - ID", "ACAPI_Database(APIDb_ChangeElementInfoStringID", err, elemGuid);
			params.Delete(param.rawName);
		}
	}
	if (params.IsEmpty()) return;
	API_Elem_Head elem_head = {};
	API_Element element = {};
	API_ElemTypeID	elemType;
	API_Guid		elemGuidt;
	API_ParamOwnerType	apiOwner = {};
	API_GetParamsType	apiParams = {};
	API_ElemTypeID eltype;
	API_ChangeParamType	chgParam;
	elem_head.guid = elemGuid;
	GSErrCode err = ACAPI_Element_GetHeader(&elem_head);
	if (err != NoError) {
		msg_rep("ParamHelpers::WriteGDLValues", "ACAPI_Element_GetHeader", err, elem_head.guid);
		return;
	}
	element.header.guid = elemGuid;
	err = ACAPI_Element_Get(&element);
	if (err != NoError) {
		msg_rep("ParamHelpers::WriteGDLValues", "ACAPI_Element_Get", err, elem_head.guid);
		return;
	}
#ifdef AC_26
	eltype = elem_head.type.typeID;
#else
	eltype = elem_head.typeID;
#endif
	GetGDLParametersHead(element, elem_head, elemType, elemGuidt);
	BNZeroMemory(&apiOwner, sizeof(API_ParamOwnerType));
	BNZeroMemory(&apiParams, sizeof(API_GetParamsType));
	apiOwner.guid = elemGuidt;
#ifdef AC_26
	apiOwner.type.typeID = elemType;
#else
	apiOwner.typeID = elemType;
#endif
	err = ACAPI_Goodies(APIAny_OpenParametersID, &apiOwner, nullptr);
	if (err != NoError) {
		msg_rep("ParamHelpers::WriteGDLValues", "APIAny_OpenParametersID", err, elem_head.guid);
		return;
	}
	err = ACAPI_Goodies(APIAny_GetActParametersID, &apiParams);
	if (err != NoError) {
		msg_rep("ParamHelpers::WriteGDLValues", "APIAny_GetActParametersID", err, elem_head.guid);
		err = ACAPI_Goodies(APIAny_CloseParametersID);
		if (err != NoError) {
			msg_rep("ParamHelpers::WriteGDLValues", "APIAny_CloseParametersID", err, elem_head.guid);
			return;
		}
	}

	// TODO Оптимизировать, разнести по функциям
	bool flagFind = false;
	Int32	addParNum = BMGetHandleSize((GSHandle)apiParams.params) / sizeof(API_AddParType);
	Int32 nfind = params.GetSize();
	for (Int32 i = 0; i < addParNum; ++i) {
		API_AddParType& actualParam = (*apiParams.params)[i];
		GS::UniString name = actualParam.name;
		GS::UniString rawname = "{gdl:" + name.ToLowerCase() + "}";
		if (params.ContainsKey(rawname)) {
			ParamValueData paramfrom = params.Get(rawname).val;
			BNZeroMemory(&chgParam, sizeof(API_ChangeParamType));
			chgParam.index = actualParam.index;
			CHTruncate(actualParam.name, chgParam.name, API_NameLen);
			if (actualParam.typeID == APIParT_CString) {
				GS::uchar_t uStrBuffer[256];
				GS::ucsncpy(uStrBuffer, paramfrom.uniStringValue.ToUStr().Get(), 256);
				chgParam.uStrValue = uStrBuffer;
			}
			if (actualParam.typeID == APIParT_Integer) {
				chgParam.realValue = paramfrom.intValue;
			}
			if (actualParam.typeID == APIParT_Length) {
				chgParam.realValue = paramfrom.doubleValue;
			}
			if (actualParam.typeID == APIParT_Angle) {
				chgParam.realValue = paramfrom.doubleValue;
			}
			if (actualParam.typeID == APIParT_RealNum) {
				chgParam.realValue = paramfrom.doubleValue;
			}
			if (actualParam.typeID == APIParT_Boolean) {
				chgParam.realValue = paramfrom.boolValue;
			}
			err = ACAPI_Goodies(APIAny_ChangeAParameterID, &chgParam, nullptr);
			if (err != NoError) {
				msg_rep("ParamHelpers::WriteGDLValues", "APIAny_ChangeAParameterID", err, elem_head.guid);
				return;
			}
		}
	}
	err = ACAPI_Goodies(APIAny_GetActParametersID, &apiParams, nullptr);
	if (err != NoError) {
		msg_rep("ParamHelpers::WriteGDLValues", "APIAny_GetActParametersID", err, elem_head.guid);
		return;
	}
	err = ACAPI_Goodies(APIAny_CloseParametersID, nullptr, nullptr);
	if (err != NoError) {
		msg_rep("ParamHelpers::WriteGDLValues", "APIAny_CloseParametersID", err, elem_head.guid);
		return;
	}
	API_ElementMemo	elemMemo = {};
	elemMemo.params = apiParams.params;
	err = ACAPI_Element_ChangeMemo(elemGuidt, APIMemoMask_AddPars, &elemMemo);
	if (err != NoError) msg_rep("ParamHelpers::WriteGDLValues", "ACAPI_Element_ChangeMemo", err, elem_head.guid);
	ACAPI_DisposeAddParHdl(&apiParams.params);
}

// --------------------------------------------------------------------
// Запись ParamDictValue в свойства
// --------------------------------------------------------------------
void ParamHelpers::WritePropertyValues(const API_Guid & elemGuid, ParamDictValue & params) {
	if (params.IsEmpty()) return;
	if (elemGuid == APINULLGuid) return;
	GS::Array<API_Property> properties;
	for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = params.EnumeratePairs(); cIt != NULL; ++cIt) {
		ParamValue& param = *cIt->value;
		if (param.isValid && param.fromPropertyDefinition) {
			API_Property property = param.property;

			// TODO выяснить - что быстрее - пакетная запись или запись по-отдельности
			//if (ParamHelpers::ToProperty(param, property)) properties.Push(property);
			if (ParamHelpers::ToProperty(param, property)) {
				GSErrCode error = ACAPI_Element_SetProperty(elemGuid, property);
				if (error != NoError) msg_rep("WritePropertyValues", "ACAPI_Element_SetProperty", error, elemGuid);
			}
		}
	}

	//if (properties.IsEmpty()) return;
	//GSErrCode error = ACAPI_Element_SetProperties(elemGuid, properties);
}

bool ParamHelpers::hasUnreadProperyDefinitoin(ParamDictElement & paramToRead) {
	for (GS::HashTable<API_Guid, ParamDictValue>::PairIterator cIt = paramToRead.EnumeratePairs(); cIt != NULL; ++cIt) {
		ParamDictValue& params = *cIt->value;
		if (!params.IsEmpty()) {
			for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = params.EnumeratePairs(); cIt != NULL; ++cIt) {
				ParamValue& param = *cIt->value;
				if (param.fromProperty && !param.fromPropertyDefinition && !param.fromAttribDefinition) return true;
			}
		}
	}
	return false;
}

// --------------------------------------------------------------------
// Заполнение словаря параметров для множества элементов
// --------------------------------------------------------------------
void ParamHelpers::ElementsRead(ParamDictElement & paramToRead, ParamDictValue & propertyParams) {
	if (paramToRead.IsEmpty()) return;

	// Словарь со всеми возможными определениями свойств
	if (propertyParams.IsEmpty()) {
		if (ParamHelpers::hasUnreadProperyDefinitoin(paramToRead)) ParamHelpers::GetAllPropertyDefinitionToParamDict(propertyParams);
	}

	// Выбираем по-элементно параметры для чтения
	for (GS::HashTable<API_Guid, ParamDictValue>::PairIterator cIt = paramToRead.EnumeratePairs(); cIt != NULL; ++cIt) {
		ParamDictValue& params = *cIt->value;
		API_Guid elemGuid = *cIt->key;
		if (!params.IsEmpty()) {
			if (!propertyParams.IsEmpty()) ParamHelpers::Compare(propertyParams, params); // Сопоставляем свойства
			ParamHelpers::Read(elemGuid, params, propertyParams);
		}
	}
	return;
}

// --------------------------------------------------------------------
// Заполнение словаря с параметрами
// --------------------------------------------------------------------
void ParamHelpers::Read(const API_Guid & elemGuid, ParamDictValue & params, ParamDictValue & propertyParams) {
	if (params.IsEmpty()) return;
	API_Elem_Head elem_head = {};
	elem_head.guid = elemGuid;
	GSErrCode err = ACAPI_Element_GetHeader(&elem_head);
	if (err != NoError) {
		msg_rep("ParamDictRead", "ACAPI_Element_GetHeader", err, elem_head.guid);
		return;
	}
	API_ElemTypeID eltype;
#ifdef AC_26
	eltype = elem_head.type.typeID;
#else
	eltype = elem_head.typeID;
#endif

	// Для некоторых типов элементов есть общая информация, которая может потребоваться
	// Пройдём по параметрам и посмотрим - что нам нужно заранее прочитать
	bool needGetElement = false;
	bool needGetAllDefinitions = false;
	for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = params.EnumeratePairs(); cIt != NULL; ++cIt) {
		ParamValue& param = *cIt->value;
		if (param.fromGuid == APINULLGuid) param.fromGuid = elemGuid;
		if (param.fromGuid == elemGuid && param.fromGuid != APINULLGuid) {

			// Когда нужно получить весь элемент
			if (param.fromGDLdescription || param.fromCoord || param.fromMorph || param.fromMaterial || param.fromAttribDefinition) {
				needGetElement = true;
			}
			if (eltype == API_CurtainWallPanelID || eltype == API_RailingBalusterID || eltype == API_RailingHandrailID) {
				needGetElement = true;
			}
			if (param.fromProperty && !param.fromPropertyDefinition && !param.fromAttribDefinition) needGetAllDefinitions = true; // Нужно проверить соответсвие описаний имени свойства
		}
	}
	if (needGetAllDefinitions) {
		GetAllPropertyDefinitionToParamDict(params, elemGuid);
	} // Проверим - для всех ли свойств подобраны определения

	API_Element element = {};
	if (needGetElement) {
		element.header.guid = elemGuid;
		err = ACAPI_Element_Get(&element);
		if (err != NoError) {
			msg_rep("ParamDictRead", "ACAPI_Element_Get", err, elem_head.guid);
			return;
		}
	}
	else {
		UNUSED_VARIABLE(element);
	}

	// Получаем список возможных префиксов
	GS::Array<GS::UniString> paramTypesList;
	GetParamTypeList(paramTypesList);

	// Для каждого типа - свой способ получения данных. Поэтому разбиваем по типам и обрабатываем по-отдельности
	for (UInt32 i = 0; i < paramTypesList.GetSize(); i++) {
		GS::UniString paramType = paramTypesList[i];
		ParamDictValue paramByType;
		for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = params.EnumeratePairs(); cIt != NULL; ++cIt) {
			ParamValue& param = *cIt->value;
			if (param.fromGuid == elemGuid) {
				if (param.rawName.Contains(paramType)) paramByType.Add(param.rawName, param);
			}
		}
		if (!paramByType.IsEmpty()) {
			bool needCompare = false;

			// Проходим поиском, специфичным для каждого типа
			// TODO переписать без использования сравнения с текстом!
			if (paramType.IsEqual("property")) {
				needCompare = ParamHelpers::ReadPropertyValues(elemGuid, paramByType);
			}
			if (paramType.IsEqual("coord")) {
				needCompare = ParamHelpers::ReadElemCoords(element, paramByType);
			}
			if (paramType.IsEqual("gdl")) {
				needCompare = ParamHelpers::ReadGDLValues(element, elem_head, paramByType);
			}
			if (paramType.IsEqual("material") && !paramByType.IsEmpty()) {
				ParamHelpers::ReadMaterial(element, params, propertyParams);
			}
			if (paramType.IsEqual("ifc")) {
				needCompare = ParamHelpers::ReadIFCValues(elemGuid, paramByType);
			}
			if (paramType.IsEqual("morph")) {
				needCompare = ParamHelpers::ReadMorphParam(element, paramByType);
			}
			if (needCompare) {
				ParamHelpers::Compare(paramByType, params);
			}
		}
	}
}

// -----------------------------------------------------------------------------
// Получение определения свойства по имени свойства
// Формат имени ГРУППА/ИМЯ_СВОЙСТВА
// -----------------------------------------------------------------------------
GSErrCode GetPropertyDefinitionByName(const GS::UniString & propertyname, API_PropertyDefinition & definition) {
	GSErrCode err = GetPropertyDefinitionByName(APINULLGuid, propertyname, definition);
	return err;
}

// -----------------------------------------------------------------------------
// Получение определения свойства по имени свойства
// Формат имени ГРУППА/ИМЯ_СВОЙСТВА
// -----------------------------------------------------------------------------
GSErrCode GetPropertyDefinitionByName(const API_Guid & elemGuid, const GS::UniString & tpropertyname, API_PropertyDefinition & definition) {
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
				if ((groups[i].groupType == API_PropertyStaticBuiltInGroupType) || groups[i].groupType == API_PropertyCustomGroupType) {
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

// --------------------------------------------------------------------
// Заполнение свойств для элемента
// --------------------------------------------------------------------
void ParamHelpers::GetAllPropertyDefinitionToParamDict(ParamDictValue & propertyParams, const API_Guid & elemGuid) {
	if (elemGuid == APINULLGuid) {
		ParamHelpers::GetAllPropertyDefinitionToParamDict(propertyParams);
	}
	else {
		GS::Array<API_PropertyDefinition> definitions;
		GSErrCode err = ACAPI_Element_GetPropertyDefinitions(elemGuid, API_PropertyDefinitionFilter_All, definitions);
		if (err != NoError) {
			msg_rep("GetAllPropertyDefinitionToParamDict", "ACAPI_Element_GetPropertyDefinitions", err, elemGuid);
			return;
		}
		if (definitions.IsEmpty()) return;
		ParamHelpers::GetAllPropertyDefinitionToParamDict(propertyParams, definitions);
	}
}

// --------------------------------------------------------------------
// Перевод GS::Array<API_PropertyDefinition> в ParamDictValue
// --------------------------------------------------------------------
void ParamHelpers::GetAllPropertyDefinitionToParamDict(ParamDictValue & propertyParams, GS::Array<API_PropertyDefinition>&definitions) {
	if (definitions.IsEmpty()) return;
	UInt32 nparams = propertyParams.GetSize();
	bool needAddNew = (nparams == 0);
	for (UInt32 j = 0; j < definitions.GetSize(); j++) {
		ParamValue pvalue;
		ParamHelpers::ConvValue(pvalue, definitions[j]);
		bool changeExs = propertyParams.ContainsKey(pvalue.rawName);
		if (needAddNew && !changeExs) {
			propertyParams.Add(pvalue.rawName, pvalue);
		}
		else {
			if (changeExs) {
				pvalue.fromGuid = propertyParams.Get(pvalue.rawName).fromGuid;
				propertyParams.Get(pvalue.rawName) = pvalue;
				nparams--;
				if (nparams == 0) {
					return;
				}
			}
		}
	}
}

// --------------------------------------------------------------------
// Получить все доступные свойства в формарте ParamDictValue
// --------------------------------------------------------------------
void ParamHelpers::GetAllPropertyDefinitionToParamDict(ParamDictValue & propertyParams) {
	GS::Array<API_PropertyGroup> groups;
	GSErrCode err = ACAPI_Property_GetPropertyGroups(groups);
	if (err != NoError) {
		msg_rep("GetAllPropertyDefinitionToParamDict", "ACAPI_Property_GetPropertyGroups", err, APINULLGuid);
		return;
	}
	UInt32 nparams = propertyParams.GetSize();
	bool needAddNew = (nparams == 0);

	// Созданим словарь с определением всех свойств
	for (UInt32 i = 0; i < groups.GetSize(); i++) {
		if (groups[i].groupType == API_PropertyCustomGroupType || (groups[i].groupType == API_PropertyStaticBuiltInGroupType && groups[i].name.Contains("Material"))) {
			GS::Array<API_PropertyDefinition> definitions;
			err = ACAPI_Property_GetPropertyDefinitions(groups[i].guid, definitions);
			if (err != NoError) msg_rep("GetPropertyByName", "ACAPI_Property_GetPropertyDefinitions", err, APINULLGuid);
			if (err == NoError) {
				for (UInt32 j = 0; j < definitions.GetSize(); j++) {
					GS::UniString name = "";
					GS::UniString rawName = "";
					if (definitions[j].description.Contains("Sync_name")) {
						rawName = "{property:sync_name}";
						name = "Sync_name";
						definitions[j].name = name;
					}
					else {
						name = groups[i].name + "/" + definitions[j].name;
						rawName = "{property:" + name.ToLowerCase() + "}";
					}
					bool changeExs = propertyParams.ContainsKey(rawName);
					if (needAddNew && !changeExs) {
						ParamValue pvalue;
						pvalue.rawName = rawName;
						pvalue.name = groups[i].name + "/" + definitions[j].name;
						ParamHelpers::ConvValue(pvalue, definitions[j]);
						propertyParams.Add(pvalue.rawName, pvalue);
					}
					else {
						ParamValue pvalue = propertyParams.Get(rawName);
						pvalue.rawName = rawName;
						pvalue.name = groups[i].name + "/" + definitions[j].name;
						ParamHelpers::ConvValue(pvalue, definitions[j]);
						propertyParams.Get(pvalue.rawName) = pvalue;
						nparams--;
						if (nparams == 0) {
							return;
						}
					}
				}
			}
		}
	}
}

// --------------------------------------------------------------------
// Сопоставление двух словарей ParamDictValue
// --------------------------------------------------------------------
void ParamHelpers::Compare(const ParamDictValue & paramsFrom, ParamDictValue & paramsTo) {
	if (paramsFrom.IsEmpty() || paramsTo.IsEmpty()) return;
	if (paramsTo.IsEmpty()) return;
	UInt32 nparams = paramsTo.GetSize();
	for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = paramsTo.EnumeratePairs(); cIt != NULL; ++cIt) {
		GS::UniString k = *cIt->key;
		if (paramsFrom.ContainsKey(k)) {
			nparams--;
			ParamValue paramFrom = paramsFrom.Get(k);
			paramFrom.fromGuid = paramsTo.Get(k).fromGuid; // Чтоб GUID не перезаписался
			paramsTo.Set(k, paramFrom);
			if (nparams == 0) return; // Если все ключи найдены - выходим
		}
	}
}

// --------------------------------------------------------------------
// Чтение значений свойств в ParamDictValue
// --------------------------------------------------------------------
bool ParamHelpers::ReadPropertyValues(const API_Guid & elemGuid, ParamDictValue & params) {
	if (params.IsEmpty()) return false;

	// Определения и свойста для элементов
	GS::Array<API_PropertyDefinition> propertyDefinitions;
	for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = params.EnumeratePairs(); cIt != NULL; ++cIt) {
		ParamValue& param = *cIt->value;
		if (param.fromPropertyDefinition) {
			API_PropertyDefinition definition = param.definition;
			propertyDefinitions.Push(definition);
		}
	}
	if (!propertyDefinitions.IsEmpty()) {
		GS::Array<API_Property> properties;
		GSErrCode error = ACAPI_Element_GetPropertyValues(elemGuid, propertyDefinitions, properties);
		if (error != NoError) {
			msg_rep("ParamDictGetPropertyValues", "ACAPI_Element_GetPropertyValues", error, elemGuid);
			return false;
		}
		return (ParamHelpers::AddProperty(params, properties));
	}
	return false;
}

// -----------------------------------------------------------------------------
// Получение значения IFC свойств
// -----------------------------------------------------------------------------
bool ParamHelpers::ReadIFCValues(const API_Guid & elemGuid, ParamDictValue & params) {
	UInt32 nparams = params.GetSize();
	if (nparams == 0) return false;
	GS::Array<API_IFCProperty> properties;
	GSErrCode err = ACAPI_Element_GetIFCProperties(elemGuid, false, &properties);
	if (err != NoError) {
		msg_rep("ParamDictGetIFCValues", "ACAPI_Element_GetIFCProperties", err, elemGuid);
		return false;
	}
	bool flag_find = false;
	for (UInt32 i = 0; i < properties.GetSize(); i++) {
		API_IFCProperty prop = properties.Get(i);
		GS::UniString fname = properties[i].head.propertySetName + "/" + properties[i].head.propertyName;
		GS::UniString rawName = "{ifc:" + fname.ToLowerCase() + "}";
		if (params.ContainsKey(rawName)) {
			ParamValue pvalue;
			API_IFCProperty property = properties.Get(i);
			if (ParamHelpers::ConvValue(pvalue, property)) {
				params.Get(rawName) = pvalue;
				flag_find = true;
			}
			nparams--;
			if (nparams == 0) {
				return flag_find;
			}
		}
		else {
			fname = properties[i].head.propertyName;
			rawName = "{ifc:" + fname.ToLowerCase() + "}";
			if (params.ContainsKey(rawName)) {
				ParamValue pvalue;
				API_IFCProperty property = properties.Get(i);
				if (ParamHelpers::ConvValue(pvalue, property)) {
					params.Get(rawName) = pvalue;
					flag_find = true;
				}
				nparams--;
				if (nparams == 0) {
					return flag_find;
				}
			}
		}
	}
	return flag_find;
}

// -----------------------------------------------------------------------------
// Получить значение GDL параметра по его имени или описанию в ParamValue
// -----------------------------------------------------------------------------
bool ParamHelpers::ReadGDLValues(const API_Element & element, const API_Elem_Head & elem_head, ParamDictValue & params) {
	if (params.IsEmpty()) return false;
	API_ElemTypeID eltype;
#ifdef AC_26
	eltype = elem_head.type.typeID;
#else
	eltype = elem_head.typeID;
#endif

	// Разбиваем по типам поиска - по описанию/по имени
	ParamDictValue paramBydescription;
	ParamDictValue paramByName;
	bool flag_find_ID = false;
	for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = params.EnumeratePairs(); cIt != NULL; ++cIt) {
		ParamValue& param = *cIt->value;
		if (param.rawName.IsEqual("{gdl:id}")) {
			GS::UniString infoString;
			API_Guid elguid = elem_head.guid;
			GSErrCode err = ACAPI_Database(APIDb_GetElementInfoStringID, &elguid, &infoString);
			if (err == NoError) {

				// TODO Добавить перевод ID в число
				param.isValid = true;
				param.val.type = API_PropertyStringValueType;
				param.type = API_PropertyStringValueType;
				param.val.canCalculate = true;
				param.val.boolValue = !infoString.IsEmpty();
				std::string var = infoString.ToCStr(0, MaxUSize, CC_Cyrillic).Get();
				param.val.doubleValue = atof(var.c_str());
				param.val.intValue = atoi(var.c_str());
				if (param.val.intValue == 0 && param.val.boolValue) {
					param.val.intValue = !infoString.IsEmpty();
					param.val.doubleValue = !infoString.IsEmpty() * 1.0;
				}
				param.val.uniStringValue = infoString;
				params.Set(param.rawName, param);
				flag_find_ID = true;
			}
		}
		else {
			if (param.fromGDLdescription && eltype == API_ObjectID) {
				paramBydescription.Add(param.rawName, param);
			}
			else {
				if (param.fromGDLparam) paramByName.Add(param.rawName, param);
			}
		}
	}
	if (paramBydescription.IsEmpty() && paramByName.IsEmpty() && !flag_find_ID) return false;

	// Поиск по описанию
	bool flag_find_desc = false;
	if (!paramBydescription.IsEmpty()) {
		flag_find_desc = ParamHelpers::GDLParamByDescription(element, paramBydescription);
		if (flag_find_desc) ParamHelpers::Compare(paramBydescription, params);
	}

	// Поиск по описанию
	bool flag_find_name = false;
	if (!paramByName.IsEmpty()) {
		flag_find_name = ParamHelpers::GDLParamByName(element, elem_head, paramByName);
		if (flag_find_name) ParamHelpers::Compare(paramByName, params);
	}
	return (flag_find_desc || flag_find_name || flag_find_ID);
}

// -----------------------------------------------------------------------------
// Поиск по описанию GDL параметра
// Данный способ не работает с элементами навесных стен
// -----------------------------------------------------------------------------
bool ParamHelpers::GDLParamByDescription(const API_Element & element, ParamDictValue & params) {
	API_LibPart libpart;
	BNZeroMemory(&libpart, sizeof(libpart));
	libpart.index = element.object.libInd;
	GSErrCode err = ACAPI_LibPart_Get(&libpart);
	if (err != NoError) {
		msg_rep("FindGDLParametersByDescription", "ACAPI_LibPart_Get", err, element.header.guid);
		return false;
	}
	double aParam = 0.0;
	double bParam = 0.0;
	Int32 addParNum = 0;
	API_AddParType** addPars = NULL;
	err = ACAPI_LibPart_GetParams(libpart.index, &aParam, &bParam, &addParNum, &addPars);
	if (err != NoError) {
		ACAPI_DisposeAddParHdl(&addPars);
		msg_rep("FindGDLParametersByDescription", "ACAPI_LibPart_GetParams", err, element.header.guid);
		return false;
	}
	bool flagFind = false;
	Int32 nfind = params.GetSize();
	for (Int32 i = 0; i < addParNum; ++i) {
		API_AddParType& actualParam = (*addPars)[i];
		GS::UniString name = actualParam.uDescname;
		GS::UniString rawname = "{gdl:" + name.ToLowerCase() + "}";
		if (params.ContainsKey(rawname)) {
			nfind--;
			ParamValue pvalue = params.Get(rawname);
			ParamHelpers::ConvValue(pvalue, actualParam);
			if (pvalue.isValid) {
				params.Set(rawname, pvalue);
				flagFind = true;
			}
			if (nfind == 0) {
				ACAPI_DisposeAddParHdl(&addPars);
				return flagFind;
			}
		}
	}
	ACAPI_DisposeAddParHdl(&addPars);
	return flagFind;
}

// -----------------------------------------------------------------------------
// Поиск по имени GDL параметра
// -----------------------------------------------------------------------------
bool ParamHelpers::GDLParamByName(const API_Element & element, const API_Elem_Head & elem_head, ParamDictValue & params) {
	API_ElemTypeID	elemType;
	API_Guid		elemGuid;
	GetGDLParametersHead(element, elem_head, elemType, elemGuid);
	API_AddParType** addPars = NULL;
	GSErrCode err = GetGDLParameters(elemType, elemGuid, addPars);
	if (err != NoError) {
		ACAPI_DisposeAddParHdl(&addPars);
		return false;
	}
	bool flagFind = false;
	Int32	addParNum = BMGetHandleSize((GSHandle)addPars) / sizeof(API_AddParType);
	Int32 nfind = params.GetSize();
	for (Int32 i = 0; i < addParNum; ++i) {
		API_AddParType& actualParam = (*addPars)[i];
		GS::UniString name = actualParam.name;
		GS::UniString rawname = "{gdl:" + name.ToLowerCase() + "}";
		if (params.ContainsKey(rawname)) {
			nfind--;
			ParamValue pvalue = params.Get(rawname);
			ParamHelpers::ConvValue(pvalue, actualParam);
			if (pvalue.isValid) {
				params.Set(rawname, pvalue);
				flagFind = true;
			}
			if (nfind == 0) {
				ACAPI_DisposeAddParHdl(&addPars);
				return flagFind;
			}
		}
	}
	ACAPI_DisposeAddParHdl(&addPars);
	return flagFind;
}

// -----------------------------------------------------------------------------
// Получение информации о материалах и составе конструкции
// -----------------------------------------------------------------------------
bool ParamHelpers::ReadMaterial(const API_Element & element, ParamDictValue & params, ParamDictValue & propertyParams) {

	// Получим состав элемента, добавив в словарь требуемые параметры
	ParamDictValue paramsAdd;
	if (!ParamHelpers::GetComponents(element, params, paramsAdd)) return false;

	ParamDictValue paramlayers;
	for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = params.EnumeratePairs(); cIt != NULL; ++cIt) {
		ParamValue& param = *cIt->value;
		if (param.fromMaterial) {
			if (param.rawName.Contains("{material:layers")) {
				paramlayers.Add(param.rawName, param);
			}
		}
	}
	if (paramlayers.IsEmpty()) return true;

	// В свойствах могли быть ссылки на другие свойста. Проверим, распарсим
	if (!paramsAdd.IsEmpty()) {
		ParamHelpers::Compare(propertyParams, paramsAdd);
		GS::HashTable<API_AttributeIndex, bool> existsmaterial; // Словарь с уже прочитанными материалами
		for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = paramlayers.EnumeratePairs(); cIt != NULL; ++cIt) {
			ParamValue& param_composite = *cIt->value;
			ParamDictValue paramsAdd_1;
			if (param_composite.val.uniStringValue.Contains("{")) {
				Int32 nlayers = param_composite.composite.GetSize();
				bool flag = false;
				for (Int32 i = 0; i < nlayers; ++i) {
					API_AttributeIndex constrinx = param_composite.composite[i].inx;
					if (!existsmaterial.ContainsKey(constrinx)) {
						if (ParamHelpers::GetAttributeValues(constrinx, paramsAdd, paramsAdd_1)) {
							flag = true;
							existsmaterial.Add(constrinx, true);
						}
					}
				}
				if (flag) {
					for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = paramsAdd.EnumeratePairs(); cIt != NULL; ++cIt) {
						params.Add(*cIt->key, *cIt->value);
					}
				}
			}
		}
	}
	bool flag_add = false;

	// Если есть строка-шаблон - заполним её
	for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = paramlayers.EnumeratePairs(); cIt != NULL; ++cIt) {
		bool flag = false;
		ParamValue& param_composite = *cIt->value;
		GS::UniString rawName = *cIt->key;
		GS::UniString outstring = "";
		if (param_composite.val.uniStringValue.Contains("{")) {
			Int32 nlayers = param_composite.composite.GetSize();
			for (Int32 i = 0; i < nlayers; ++i) {
				GS::UniString templatestring = param_composite.val.uniStringValue;
				API_AttributeIndex constrinx = param_composite.composite[i].inx;

				// Если нужно заполнить толщину
				if (params.ContainsKey("{material:layer thickness}")) {
					double fillThick = param_composite.composite[i].fillThick;
					GS::UniString formatsting = params.Get("{material:layer thickness}").val.stringformat;
					if (formatsting.IsEmpty()) {
						formatsting = "1mm";
						params.Get("{material:layer thickness}").val.stringformat = formatsting;
					}
					GS::UniString fillThickstring = PropertyHelpers::NumToString(fillThick, formatsting);
					templatestring.ReplaceAll("{material:layer thickness}", fillThickstring);
				}
				templatestring.ReplaceAll("{material:n}", GS::UniString::Printf("%d", i + 1));

				// Если для материала было указано уникальное наименование - заменим его
				GS::UniString attribsuffix = CharENTER + GS::UniString::Printf("%d", constrinx) + "}";
				if (params.ContainsKey("{property:sync_name" + attribsuffix)) {
					if (params.Get("{property:sync_name" + attribsuffix).isValid) templatestring.ReplaceAll("property:buildingmaterialproperties/building material name", "property:sync_name");
				}
				templatestring.ReplaceAll("}", attribsuffix);
				if (ParamHelpers::ReplaceParamInExpression(params, templatestring)) {
					flag = true;
					flag_add = true;
					ReplaceSymbSpase(templatestring);
					outstring = outstring + templatestring;
				}
			}
		}
		if (flag) {
			params.Get(rawName).val.uniStringValue = outstring;
			params.Get(rawName).val.canCalculate = true;
			params.Get(rawName).isValid = true;
			params.Get(rawName).val.type = API_PropertyStringValueType;
		}
	}
	return flag_add;
}

GSErrCode GetPropertyFullName(const API_PropertyDefinition & definision, GS::UniString & name) {
	if (definision.groupGuid == APINULLGuid) return APIERR_BADID;
	GSErrCode error = NoError;
	if (definision.name.Contains("ync_name")) {
		name = definision.name;
	}
	else {
		API_PropertyGroup group;
		group.guid = definision.groupGuid;
		error = ACAPI_Property_GetPropertyGroup(group);
		if (error == NoError) {
			name = group.name + "/" + definision.name;
		}
		else {
			msg_rep("GetPropertyFullName", "ACAPI_Property_GetPropertyGroup " + definision.name, error, APINULLGuid);
		}
	}
	return error;
}

// -----------------------------------------------------------------------------
// Конвертация параметров библиотечного элемента в ParamValue
// -----------------------------------------------------------------------------
bool ParamHelpers::ConvValue(ParamValue & pvalue, const API_AddParType & nthParameter) {
	GS::UniString param_string = "";
	GS::Int32 param_int = 0;
	double param_real = 0.0;
	bool param_bool = false;
	pvalue.val.canCalculate = false;

	// Определяем тип и вычисляем текстовое, целочисленное и дробное значение.
	if (nthParameter.typeID == APIParT_CString) {
		pvalue.val.type = API_PropertyStringValueType;
		param_string = nthParameter.value.uStr;
		param_bool = (!param_string.IsEmpty());
		std::string var = param_string.ToCStr(0, MaxUSize, CC_Cyrillic).Get();
		param_real = atof(var.c_str());
		param_int = atoi(var.c_str());
		if (param_bool && param_int == 0) {
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
	if (param_string.IsEmpty()) {
		API_AttrTypeID attrType = API_ZombieAttrID;
		short attrInx = (short)param_int;
		switch (nthParameter.typeID) {
		case APIParT_Integer:
			param_string = GS::UniString::Printf("%d", param_int);
			pvalue.val.type = API_PropertyIntegerValueType;
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
			pvalue.val.type = API_PropertyBooleanValueType;
			break;
		case APIParT_Length:
			param_string = GS::UniString::Printf("%.0f", param_real * 1000);
			pvalue.val.type = API_PropertyRealValueType;
			break;
		case APIParT_Angle:
			param_string = GS::UniString::Printf("%.1f", param_real);
			pvalue.val.type = API_PropertyRealValueType;
			break;
		case APIParT_RealNum:
			param_string = GS::UniString::Printf("%.3f", param_real);
			pvalue.val.type = API_PropertyRealValueType;
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
				pvalue.val.type = API_PropertyStringValueType;
				param_bool = (attrInx != 0);
				param_int = attrInx;
				param_real = attrInx / 1.0;
			}
			else {
				return false;
			}
		}
	}
	if (pvalue.rawName.IsEmpty()) pvalue.rawName = "{gdl:" + GS::UniString(nthParameter.name).ToLowerCase() + "}";
	if (pvalue.name.IsEmpty()) pvalue.name = nthParameter.name;
	pvalue.type = pvalue.val.type;
	pvalue.fromGDLparam = true;
	pvalue.val.boolValue = param_bool;
	pvalue.val.doubleValue = param_real;
	pvalue.val.intValue = param_int;
	pvalue.val.uniStringValue = param_string;
	pvalue.val.canCalculate = true;
	pvalue.isValid = true;
	return true;
}

// -----------------------------------------------------------------------------
// Конвертация свойства в ParamValue
// -----------------------------------------------------------------------------
bool ParamHelpers::ConvValue(ParamValue & pvalue, const API_Property & property) {
	if (pvalue.rawName.IsEmpty() || pvalue.name.IsEmpty()) {
		GS::UniString fname;
		GetPropertyFullName(property.definition, fname);
		if (pvalue.rawName.IsEmpty()) pvalue.rawName = "{property:" + fname.ToLowerCase() + "}";
		if (pvalue.name.IsEmpty()) pvalue.name = fname;
	}
#if defined(AC_22) || defined(AC_23)
	pvalue.isValid = property_from.isEvaluated;
#else
	pvalue.isValid = (property.status == API_Property_HasValue);
#endif
	if (!pvalue.isValid) {
		return false;
	}
	pvalue.val.boolValue = false;
	pvalue.val.intValue = 0;
	pvalue.val.doubleValue = 0.0;
	pvalue.val.canCalculate = false;
	pvalue.val.uniStringValue = PropertyHelpers::ToString(property);
	std::string var = pvalue.val.uniStringValue.ToCStr(0, MaxUSize, CC_Cyrillic).Get();
	switch (property.value.singleVariant.variant.type) {
	case API_PropertyIntegerValueType:
		pvalue.val.intValue = property.value.singleVariant.variant.intValue;
		pvalue.val.doubleValue = property.value.singleVariant.variant.intValue * 1.0;
		if (pvalue.val.intValue > 0) pvalue.val.boolValue = true;
		pvalue.val.type = API_PropertyIntegerValueType;
		break;
	case API_PropertyRealValueType:
		pvalue.val.doubleValue = round(property.value.singleVariant.variant.doubleValue * 1000) / 1000;
		if (property.value.singleVariant.variant.doubleValue - pvalue.val.doubleValue > 0.001) pvalue.val.doubleValue += 0.001;
		pvalue.val.intValue = (GS::Int32)pvalue.val.doubleValue;
		if (pvalue.val.intValue / 1 < pvalue.val.doubleValue) pvalue.val.intValue += 1;
		if (abs(pvalue.val.doubleValue) > std::numeric_limits<double>::epsilon()) pvalue.val.boolValue = true;
		pvalue.val.type = API_PropertyRealValueType;
		break;
	case API_PropertyBooleanValueType:
		pvalue.val.boolValue = property.value.singleVariant.variant.boolValue;
		if (pvalue.val.boolValue) {
			pvalue.val.intValue = 1;
			pvalue.val.doubleValue = 1.0;
		}
		pvalue.val.type = API_PropertyBooleanValueType;
		break;
	case API_PropertyStringValueType:
	case API_PropertyGuidValueType:
		pvalue.val.type = API_PropertyStringValueType;
		pvalue.val.boolValue = !pvalue.val.uniStringValue.IsEmpty();
		pvalue.val.doubleValue = atof(var.c_str());
		pvalue.val.intValue = atoi(var.c_str());
		if (pvalue.val.boolValue && pvalue.val.intValue == 0) {
			pvalue.val.intValue = 1;
			pvalue.val.doubleValue = 1.0;
		}
		break;
	case API_PropertyUndefinedValueType:
		return false;
		break;
	default:
		return false;
		break;
	}
	if (pvalue.rawName.Contains("buildingmaterial")) {
		pvalue.fromAttribDefinition = true;
	}
	if (!pvalue.fromAttribDefinition) {
		if (pvalue.rawName.Contains("component")) pvalue.fromAttribDefinition = true;
	}
	pvalue.type = pvalue.val.type;
	pvalue.fromProperty = true;
	pvalue.fromPropertyDefinition = !pvalue.fromAttribDefinition;
	pvalue.definition = property.definition;
	pvalue.property = property;
	pvalue.val.canCalculate = true;
	return true;
}

// -----------------------------------------------------------------------------
// Конвертация определения свойства в ParamValue
// -----------------------------------------------------------------------------
bool ParamHelpers::ConvValue(ParamValue & pvalue, const API_PropertyDefinition & definition) {
	if (pvalue.rawName.IsEmpty() || pvalue.name.IsEmpty()) {
		GS::UniString fname;
		GetPropertyFullName(definition, fname);
		if (pvalue.rawName.IsEmpty()) pvalue.rawName = "{property:" + fname.ToLowerCase() + "}";
		if (pvalue.name.IsEmpty()) pvalue.name = fname;
	}
	if (pvalue.rawName.Contains("buildingmaterial")) {
		pvalue.fromAttribDefinition = true;
	}
	if (!pvalue.fromAttribDefinition) {
		if (pvalue.rawName.Contains("component")) {
			pvalue.fromAttribDefinition = true;
		}
	}
	if (definition.description.Contains("Sync_name")) {
		pvalue.rawName = "{property:sync_name}";
		pvalue.name = "Sync_name";
		pvalue.fromAttribDefinition = true;
	}
	pvalue.fromProperty = true;
	pvalue.fromPropertyDefinition = !pvalue.fromAttribDefinition;
	pvalue.definition = definition;
	return true;
}

// -----------------------------------------------------------------------------
// Конвертация целого числа в ParamValue
// -----------------------------------------------------------------------------
bool ParamHelpers::ConvValue(ParamValue & pvalue, const GS::UniString & paramName, const Int32 intValue) {
	if (pvalue.name.IsEmpty()) pvalue.name = paramName;
	if (pvalue.rawName.IsEmpty()) pvalue.rawName = "{gdl:" + paramName.ToLowerCase() + "}";
	pvalue.val.type = API_PropertyIntegerValueType;
	pvalue.val.canCalculate = true;
	pvalue.val.intValue = intValue;
	pvalue.val.doubleValue = intValue * 1.0;
	if (pvalue.val.intValue > 0) pvalue.val.boolValue = true;
	pvalue.val.uniStringValue = GS::UniString::Printf("%d", intValue);
	pvalue.isValid = true;
	return true;
}

// -----------------------------------------------------------------------------
// Конвертация double в ParamValue
// -----------------------------------------------------------------------------
bool ParamHelpers::ConvValue(ParamValue & pvalue, const GS::UniString & paramName, const double doubleValue) {
	if (pvalue.name.IsEmpty()) pvalue.name = paramName;
	if (pvalue.rawName.IsEmpty()) pvalue.rawName = "{gdl:" + paramName.ToLowerCase() + "}";
	pvalue.val.type = API_PropertyRealValueType;
	pvalue.val.canCalculate = true;
	pvalue.val.intValue = (GS::Int32)doubleValue;
	pvalue.val.doubleValue = doubleValue;
	pvalue.val.boolValue = false;
	if (abs(pvalue.val.doubleValue) > std::numeric_limits<double>::epsilon()) pvalue.val.boolValue = true;
	pvalue.val.uniStringValue = GS::UniString::Printf("%.3f", doubleValue);
	pvalue.isValid = true;
	return true;
}

// -----------------------------------------------------------------------------
// Конвертация API_IFCProperty в ParamValue
// -----------------------------------------------------------------------------
bool ParamHelpers::ConvValue(ParamValue & pvalue, const API_IFCProperty & property) {
	if (pvalue.rawName.IsEmpty() || pvalue.name.IsEmpty()) {
		GS::UniString fname = property.head.propertySetName + "/" + property.head.propertyName;
		if (pvalue.rawName.IsEmpty()) pvalue.rawName = "{ifc:" + fname.ToLowerCase() + "}";
		if (pvalue.name.IsEmpty()) pvalue.name = fname;
	}
	pvalue.val.canCalculate = true;
	pvalue.isValid = true;
	if (property.head.propertyType == API_IFCPropertySingleValueType) {
		switch (property.singleValue.nominalValue.value.primitiveType) {
		case API_IFCPropertyAnyValueStringType:
			pvalue.val.type = API_PropertyStringValueType;
			pvalue.val.uniStringValue = property.singleValue.nominalValue.value.stringValue;
			pvalue.val.boolValue = !pvalue.val.uniStringValue.IsEmpty();
			if (pvalue.val.boolValue) {
				pvalue.val.intValue = 1;
				pvalue.val.doubleValue = 1.0;
			}
			break;
		case API_IFCPropertyAnyValueRealType:
			pvalue.val.type = API_PropertyRealValueType;
			pvalue.val.doubleValue = round(property.singleValue.nominalValue.value.doubleValue * 1000) / 1000;
			if (property.singleValue.nominalValue.value.doubleValue - pvalue.val.doubleValue > 0.001) pvalue.val.doubleValue += 0.001;
			pvalue.val.intValue = (GS::Int32)pvalue.val.doubleValue;
			if (pvalue.val.intValue / 1 < pvalue.val.doubleValue) pvalue.val.intValue += 1;
			if (abs(pvalue.val.doubleValue) > std::numeric_limits<double>::epsilon()) pvalue.val.boolValue = true;
			pvalue.val.uniStringValue = GS::UniString::Printf("%.3f", pvalue.val.doubleValue);
			break;
		case API_IFCPropertyAnyValueIntegerType:
			pvalue.val.type = API_PropertyIntegerValueType;
			pvalue.val.intValue = (GS::Int32)property.singleValue.nominalValue.value.intValue;
			pvalue.val.doubleValue = pvalue.val.intValue * 1.0;
			if (pvalue.val.intValue > 0) pvalue.val.boolValue = true;
			pvalue.val.uniStringValue = GS::UniString::Printf("%d", pvalue.val.intValue);
			break;
		case API_IFCPropertyAnyValueBooleanType:
			pvalue.val.type = API_PropertyBooleanValueType;
			pvalue.val.boolValue = property.singleValue.nominalValue.value.boolValue;
			if (pvalue.val.boolValue) {
				pvalue.val.uniStringValue = RSGetIndString(AddOnStringsID, TrueId, ACAPI_GetOwnResModule());
				pvalue.val.intValue = 1;
				pvalue.val.doubleValue = 1.0;
			}
			else {
				pvalue.val.uniStringValue = RSGetIndString(AddOnStringsID, FalseId, ACAPI_GetOwnResModule());
				pvalue.val.intValue = 0;
				pvalue.val.doubleValue = 0.0;
			}
			break;
		case API_IFCPropertyAnyValueLogicalType:
			pvalue.val.type = API_PropertyBooleanValueType;
			if (property.singleValue.nominalValue.value.intValue == 0) pvalue.val.boolValue = false;
			if (property.singleValue.nominalValue.value.intValue == 1) pvalue.isValid = false;
			if (property.singleValue.nominalValue.value.intValue == 2) pvalue.val.boolValue = true;
			if (pvalue.val.boolValue) {
				pvalue.val.uniStringValue = RSGetIndString(AddOnStringsID, TrueId, ACAPI_GetOwnResModule());
				pvalue.val.intValue = 1;
				pvalue.val.doubleValue = 1.0;
			}
			else {
				pvalue.val.uniStringValue = RSGetIndString(AddOnStringsID, FalseId, ACAPI_GetOwnResModule());
				pvalue.val.intValue = 0;
				pvalue.val.doubleValue = 0.0;
			}
			break;
		default:
			pvalue.val.canCalculate = false;
			pvalue.isValid = false;
			break;
		}
	}
	return pvalue.isValid;
}

GS::UniString ParamHelpers::ToString(const ParamValue & pvalue) {
	GS::UniString stringformat = pvalue.val.stringformat;
	switch (pvalue.val.type) {
	case API_PropertyIntegerValueType: return  PropertyHelpers::NumToString(pvalue.val.intValue, stringformat);
	case API_PropertyRealValueType: return PropertyHelpers::NumToString(pvalue.val.doubleValue, stringformat);
	case API_PropertyStringValueType: return pvalue.val.uniStringValue;
	case API_PropertyBooleanValueType: return GS::ValueToUniString(pvalue.val.boolValue);
	default: DBBREAK(); return "Invalid Value";
	}
}

GS::UniString ParamHelpers::ToString(const ParamValue & pvalue, const GS::UniString stringformat) {
	switch (pvalue.val.type) {
	case API_PropertyIntegerValueType: return  PropertyHelpers::NumToString(pvalue.val.intValue, stringformat);
	case API_PropertyRealValueType: return PropertyHelpers::NumToString(pvalue.val.doubleValue, stringformat);
	case API_PropertyStringValueType: return pvalue.val.uniStringValue;
	case API_PropertyBooleanValueType: return GS::ValueToUniString(pvalue.val.boolValue);
	default: DBBREAK(); return "Invalid Value";
	}
}

// --------------------------------------------------------------------
// Получение данных из однородной конструкции
// --------------------------------------------------------------------
bool ParamHelpers::GetComponentsBasicStructure(const API_AttributeIndex & constrinx, const double& fillThick, ParamDictValue & params, ParamDictValue & paramlayers, ParamDictValue & paramsAdd) {
	ParamValue param_composite = {};
	ParamValueComposite layer = {};
	layer.inx = constrinx;
	layer.fillThick = fillThick;
	param_composite.composite.Push(layer);
	ParamHelpers::GetAttributeValues(constrinx, params, paramsAdd);
	for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = paramlayers.EnumeratePairs(); cIt != NULL; ++cIt) {
		paramlayers.Get(*cIt->key).composite = param_composite.composite;
	}
	ParamHelpers::Compare(paramlayers, params);
	return true;
}

// --------------------------------------------------------------------
// Получение данных из многослойной конструкции
// --------------------------------------------------------------------
bool ParamHelpers::GetComponentsCompositeStructure(const API_Guid & elemguid, API_AttributeIndex & constrinx, ParamDictValue & params, ParamDictValue & paramlayers, ParamDictValue & paramsAdd, GS::HashTable<API_AttributeIndex, bool>&existsmaterial) {
	API_Attribute attrib;
	BNZeroMemory(&attrib, sizeof(API_Attribute));

	API_AttributeDef defs;
	BNZeroMemory(&defs, sizeof(API_AttributeDef));

	attrib.header.index = constrinx;
	attrib.header.typeID = API_CompWallID;
	GSErrCode err = ACAPI_Attribute_Get(&attrib);
	if (err != NoError) {
		msg_rep("materialString::GetComponentsCompositeStructure", " ACAPI_Attribute_Get", err, elemguid);
		return false;
	}
	err = ACAPI_Attribute_GetDef(attrib.header.typeID, attrib.header.index, &defs);
	if (err != NoError) {
		msg_rep("materialString::GetComponentsCompositeStructure", " ACAPI_Attribute_GetDef", err, elemguid);
		ACAPI_DisposeAttrDefsHdls(&defs);
		return false;
	}
	ParamValue param_composite = {};
	for (short i = 0; i < attrib.compWall.nComps; i++) {
		API_AttributeIndex	constrinxL = (*defs.cwall_compItems)[i].buildingMaterial;
		double	fillThickL = (*defs.cwall_compItems)[i].fillThick;
		ParamValueComposite layer = {};
		layer.inx = constrinxL;
		layer.fillThick = fillThickL;
		param_composite.composite.Push(layer);
		if (!existsmaterial.ContainsKey(constrinxL)) {
			ParamHelpers::GetAttributeValues(constrinxL, params, paramsAdd);
			existsmaterial.Add(constrinxL, true);
		}
	}
	for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = paramlayers.EnumeratePairs(); cIt != NULL; ++cIt) {
		paramlayers.Get(*cIt->key).composite = param_composite.composite;
	}
	ParamHelpers::Compare(paramlayers, params);
	ACAPI_DisposeAttrDefsHdls(&defs);
	return true;
}

// --------------------------------------------------------------------
// Получение данных из сложного профиля
// --------------------------------------------------------------------
bool ParamHelpers::GetComponentsProfileStructure(ProfileVectorImage & profileDescription, ParamDictValue & params, ParamDictValue & paramlayers, ParamDictValue & paramsAdd, GS::HashTable<API_AttributeIndex, bool>&existsmaterial) {
	ConstProfileVectorImageIterator profileDescriptionIt(profileDescription);
	GS::HashTable<short, OrientedSegments> lines;
	GS::HashTable<short, ParamValue> param_composite;

	// Получаем список перьев в параметрах

	for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = paramlayers.EnumeratePairs(); cIt != NULL; ++cIt) {
		GS::UniString rawName = *cIt->key;
		rawName.ReplaceAll(" ", "");
		if (rawName.Contains(",")) {
			GS::Array<GS::UniString> partstring;
			UInt32 n = StringSplt(rawName, ",", partstring);
			if (n > 0) {
				short pen = std::atoi(partstring[1].ToCStr());
				OrientedSegments s;
				lines.Add(pen, s);
				ParamValue p;
				param_composite.Add(pen, p);
				paramlayers.Get(*cIt->key).val.intValue = pen;
			}
		}
	}
	bool hasLine = !lines.IsEmpty();
	bool profilehasLine = false;

	// Ищем полилинию с нужным цветом
	while (!profileDescriptionIt.IsEOI()) {
		switch (profileDescriptionIt->item_Typ) {
		case SyArc: // Указателем начала линии служит окружность с тем же пером
		{
			const Sy_ArcType* pSyArc = static_cast <const Sy_ArcType*> (profileDescriptionIt);
			short pen = pSyArc->GetExtendedPen().GetIndex();
			if (lines.ContainsKey(pen)) {
				Point3D s = GetPoint3DFromCoord(pSyArc->origC);
				lines.Get(pen).start = s;
			}
		}
		break;
		case SyLine: // Поиск линий-сечений
		{
			const Sy_LinType* pSyPolyLine = static_cast <const Sy_LinType*> (profileDescriptionIt);
			short pen = pSyPolyLine->GetExtendedPen().GetIndex();
			if (lines.ContainsKey(pen)) {
				Sector3D line = GetSector3DFromCoord(pSyPolyLine->begC, pSyPolyLine->endC);
				lines.Get(pen).segments.Push(line);
				profilehasLine = true;
			}
		}
		break;
		}
		++profileDescriptionIt;
	}

	// Если линии сечения не найдены - создадим парочку - вертикальную и горизонтальную
	if (!profilehasLine) {
		Sector3D line = GetSector3DFromCoord(-1000, 0, 1000, 0);
		OrientedSegments d;
		d.segments.Push(line);
		lines.Add(20, d);
		Sector3D line2 = GetSector3DFromCoord(0, 1000, 0, -1000);
		OrientedSegments d2;
		d2.segments.Push(line2);
		lines.Add(6, d2);
		ParamValue p;
		param_composite.Add(20, p);
		param_composite.Add(6, p);
	}
	bool hasData = false;
	ConstProfileVectorImageIterator profileDescriptionIt1(profileDescription);
	while (!profileDescriptionIt1.IsEOI()) {
		switch (profileDescriptionIt1->item_Typ) {
		case SyHatch:
			const HatchObject& syHatch = profileDescriptionIt1;

			GS::Array<Sector3D> cont;
			GS::Array<Point2D> coords = syHatch.GetCoords();
			Point3D p1 = GetPoint3DFromCoord(coords[0]);
			Point3D p2;
			for (UInt32 i = 1; i < coords.GetSize(); i++) {
				Point3D p2 = GetPoint3DFromCoord(coords[i]);
				Sector3D line(p1, p2);
				cont.Push(line);
				p1 = p2;
			}

			// Проходим по всем линиям - сечениям
			for (GS::HashTable<short, OrientedSegments>::PairIterator cIt = lines.EnumeratePairs(); cIt != NULL; ++cIt) {
				OrientedSegments& l = *cIt->value;
				GS::Array<Point3D> points;
				for (UInt32 i = 0; i < l.segments.GetSize(); i++) {
					Point3D res;
					for (UInt32 j = 1; j < cont.GetSize(); j++) {
						if (XLines3D(&l.segments[i], &cont[j], &res)) points.Push(res);
					}
				}
				if (points.GetSize() == 2) {
					Sector3D sfillThickL(points[0], points[1]);
					Sector3D srfromstart(l.start, sfillThickL.GetMidPoint());
					double fillThickL = sfillThickL.GetLength();
					double rfromstart = srfromstart.GetLength();
					API_AttributeIndex	constrinxL = (API_AttributeIndex)syHatch.GetBuildMatIdx();
					ParamValueComposite layer = {};
					layer.inx = constrinxL;
					layer.fillThick = fillThickL;
					layer.rfromstart = rfromstart;
					param_composite.Get(*cIt->key).composite.Push(layer);
					if (!existsmaterial.ContainsKey(constrinxL)) {
						ParamHelpers::GetAttributeValues(constrinxL, params, paramsAdd);
						existsmaterial.Add(constrinxL, true);
					};
					hasData = true;
				}
			}
			break;
		}
		++profileDescriptionIt1;
	}
	if (hasData) {
		for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = paramlayers.EnumeratePairs(); cIt != NULL; ++cIt) {
			short pen = paramlayers.Get(*cIt->key).val.intValue;
			if (param_composite.ContainsKey(pen)) {

				// Теперь нам надо отсортировать слои по параметру rfromstart
				std::map<double, ParamValueComposite> comps;
				GS::Array<ParamValueComposite> param = param_composite.Get(pen).composite;
				for (UInt32 i = 0; i < param.GetSize(); i++) {
					ParamValueComposite comp = param.Get(i);
					comps[comp.rfromstart] = comp;
				}
				GS::Array<ParamValueComposite> paramout;
				for (std::map<double, ParamValueComposite>::iterator k = comps.begin(); k != comps.end(); ++k) {
					paramout.Push(k->second);
				}
				paramlayers.Get(*cIt->key).composite = paramout;
			}
		}
		ParamHelpers::Compare(paramlayers, params);
	}
	return hasData;
}

// --------------------------------------------------------------------
// Вытаскивает всё, что может, из информации о составе элемента
// --------------------------------------------------------------------
bool ParamHelpers::GetComponents(const API_Element & element, ParamDictValue & params, ParamDictValue & paramsAdd) {
	API_ModelElemStructureType	structtype = {};
	API_AttributeIndex			constrinx = {};
	double						fillThick = 0;
	API_Elem_Head elemhead = element.header;
	GS::HashTable<API_AttributeIndex, bool> existsmaterial; // Словарь с уже прочитанными материалами

	// Получаем данные о составе конструкции. Т.к. для разных типов элементов
	// информация храница в разных местах - запишем всё в одни переменные
	API_ElemTypeID eltype;
#ifdef AC_26
	eltype = element.header.type.typeID;
#else
	eltype = element.header.typeID;
#endif
	API_ElementMemo	memo = {};
	switch (eltype) {
	case API_ColumnID:

		// TODO Добавить поддержку многосегментных колонн
		if (element.column.nSegments == 1) {
			BNZeroMemory(&memo, sizeof(API_ElementMemo));
			GSErrCode err = ACAPI_Element_GetMemo(element.header.guid, &memo, APIMemoMask_ColumnSegment);
			if (err == NoError && memo.columnSegments != nullptr) {
				elemhead = memo.columnSegments[0].head;
				structtype = memo.columnSegments[0].assemblySegmentData.modelElemStructureType;
				if (structtype == API_BasicStructure) {
					constrinx = memo.columnSegments[0].assemblySegmentData.buildingMaterial;
					fillThick = memo.columnSegments[0].assemblySegmentData.nominalHeight;
				}
				if (structtype == API_ProfileStructure) constrinx = memo.columnSegments[0].assemblySegmentData.profileAttr;
			}
			else {
				msg_rep("materialString::GetComponents", "ACAPI_Element_GetMemo - ColumnSegment", err, element.header.guid);
				return false;
			}
		}
		else {
			msg_rep("materialString::GetComponents", "Multisegment column not supported", NoError, element.header.guid);
			return false;
		}
		break;
	case API_BeamID:

		// TODO Добавить поддержку многосегментных балок
		if (element.beam.nSegments == 1) {
			BNZeroMemory(&memo, sizeof(API_ElementMemo));
			GSErrCode err = ACAPI_Element_GetMemo(element.header.guid, &memo, APIMemoMask_BeamSegment);
			if (err == NoError && memo.beamSegments != nullptr) {
				elemhead = memo.beamSegments[0].head;
				structtype = memo.beamSegments[0].assemblySegmentData.modelElemStructureType;
				if (structtype == API_BasicStructure) {
					constrinx = memo.beamSegments[0].assemblySegmentData.buildingMaterial;
					fillThick = memo.beamSegments[0].assemblySegmentData.nominalHeight;
				}
				if (structtype == API_ProfileStructure) constrinx = memo.beamSegments[0].assemblySegmentData.profileAttr;
			}
			else {
				msg_rep("materialString::GetComponents", "ACAPI_Element_GetMemo - BeamSegment", err, element.header.guid);
				return false;
			}
		}
		else {
			msg_rep("materialString::GetComponents", "Multisegment beam not supported", NoError, element.header.guid);
			return false;
		}
		break;
	case API_WallID:
		structtype = element.wall.modelElemStructureType;
		if (structtype == API_CompositeStructure) constrinx = element.wall.composite;
		if (structtype == API_BasicStructure) constrinx = element.wall.buildingMaterial;
		if (structtype == API_ProfileStructure) constrinx = element.wall.profileAttr;
		fillThick = element.wall.thickness;
		break;
	case API_SlabID:
		structtype = element.slab.modelElemStructureType;
		if (structtype == API_CompositeStructure) constrinx = element.slab.composite;
		if (structtype == API_BasicStructure) constrinx = element.slab.buildingMaterial;
		fillThick = element.slab.thickness;
		break;
	case API_RoofID:
		structtype = element.roof.shellBase.modelElemStructureType;
		if (structtype == API_CompositeStructure) constrinx = element.roof.shellBase.composite;
		if (structtype == API_BasicStructure) constrinx = element.roof.shellBase.buildingMaterial;
		fillThick = element.roof.shellBase.thickness;
		break;
	case API_ShellID:
		structtype = element.shell.shellBase.modelElemStructureType;
		if (structtype == API_CompositeStructure) constrinx = element.shell.shellBase.composite;
		if (structtype == API_BasicStructure) constrinx = element.shell.shellBase.buildingMaterial;
		fillThick = element.shell.shellBase.thickness;
		break;
	default:
		return false;
		break;
	}
	ACAPI_DisposeElemMemoHdls(&memo);

	// Типов вывода слоёв может быть насколько - для сложных профилей, для учёта несущих/ненесущих слоёв
	// Получим словарь исключительно с определениями состава
	ParamDictValue paramlayers;
	for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = params.EnumeratePairs(); cIt != NULL; ++cIt) {
		ParamValue& param = *cIt->value;
		if (param.fromMaterial) {
			if (param.rawName.Contains("{material:layers")) {
				paramlayers.Add(param.rawName, param);
			}
		}
	}

	// Если ничего нет - слои нам всё равно нужны
	if (paramlayers.IsEmpty()) {
		ParamValue param_composite = {};
		param_composite.fromGuid = element.header.guid;
		param_composite.isValid = true;
		param_composite.val.canCalculate = true;
		paramlayers.Add("{material:layers,20}", param_composite);
	}

	bool hasData = false;

	// Получим индексы строительных материалов и толщины
	if (structtype == API_BasicStructure) hasData = ParamHelpers::GetComponentsBasicStructure(constrinx, fillThick, params, paramlayers, paramsAdd);
	if (structtype == API_CompositeStructure) hasData = ParamHelpers::GetComponentsCompositeStructure(elemhead.guid, constrinx, params, paramlayers, paramsAdd, existsmaterial);
	if (structtype == API_ProfileStructure) {
		API_ElementMemo	memo = {};
		UInt64 mask = APIMemoMask_StretchedProfile;
		GSErrCode err = ACAPI_Element_GetMemo(elemhead.guid, &memo, mask);
		if (err != NoError) {
			ACAPI_DisposeElemMemoHdls(&memo);
			return false;
		}
		ProfileVectorImage profileDescription = *memo.stretchedProfile;
		hasData = ParamHelpers::GetComponentsProfileStructure(profileDescription, params, paramlayers, paramsAdd, existsmaterial);
		ACAPI_DisposeElemMemoHdls(&memo);
	}
	return hasData;
}

// --------------------------------------------------------------------
// Заполнение данных для одного слоя
// --------------------------------------------------------------------
bool ParamHelpers::GetAttributeValues(const API_AttributeIndex & constrinx, ParamDictValue & params, ParamDictValue & paramsAdd) {
	API_Attribute	attrib = {};
	BNZeroMemory(&attrib, sizeof(API_Attribute));
	attrib.header.typeID = API_BuildingMaterialID;
	attrib.header.index = constrinx;
	GS::UniString attribsuffix = GS::UniString::Printf("%d", constrinx);
	GSErrCode error = ACAPI_Attribute_Get(&attrib);
	if (error != NoError) {
		msg_rep("materialString::GetAttributeValues", "ACAPI_Attribute_Get", error, APINULLGuid);
		return false;
	};

	// Определения и свойста для элементов
	GS::Array<API_PropertyDefinition> propertyDefinitions;
	for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = params.EnumeratePairs(); cIt != NULL; ++cIt) {
		ParamValue& param = *cIt->value;
		if (param.fromAttribDefinition) {
			API_PropertyDefinition definition = param.definition;
			if (!definition.name.Contains(CharENTER)) propertyDefinitions.Push(definition);
		}
	}
	if (!propertyDefinitions.IsEmpty()) {
		GS::Array<API_Property> properties;
		error = ACAPI_Attribute_GetPropertyValues(attrib.header, propertyDefinitions, properties);
		if (error != NoError) {
			msg_rep("materialString::GetAttributeValues", "ACAPI_Attribute_GetPropertyValues", error, APINULLGuid);
			return false;
		};
		if (error != NoError) {
			msg_rep("ParamDictGetPropertyValues", "ACAPI_Attribute_GetPropertyValues", error, APINULLGuid);
			return false;
		}
		for (UInt32 i = 0; i < properties.GetSize(); i++) {
			properties[i].definition.name = properties[i].definition.name + CharENTER + attribsuffix;
			GS::UniString val = PropertyHelpers::ToString(properties[i]);
			if (val.Count("%") > 1 || (val.Contains("{") && val.Contains("}"))) {
				if (ParamHelpers::ParseParamNameMaterial(val, paramsAdd)) {
					properties[i].value.singleVariant.variant.uniStringValue = val;
					properties[i].isDefault = false;
				}
			}
		}
		return (ParamHelpers::AddProperty(params, properties));
	}
	return false;
}