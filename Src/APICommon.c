// *****************************************************************************
// Helper functions for Add-On development
// API Development Kit 24; Mac/Win
//
// Namespaces:		Contact person:
//		-None-
//
// [SG compatible] - Yes
// *****************************************************************************

#include "APIEnvir.h"
#define	_APICOMMON_TRANSL_


// ---------------------------------- Includes ---------------------------------

#include	<stdio.h>
#include	<stdarg.h>
#include	<math.h>

#include	"GSSystem.h"

#include	"ACAPinc.h"
#include	"APICommon.h"


#define USE_DEBUG_WINDOW	1

// ---------------------------------- Types ------------------------------------


// ---------------------------------- Variables --------------------------------


// ---------------------------------- Prototypes -------------------------------


// =============================================================================
//
//	Messages
//
// =============================================================================


// -----------------------------------------------------------------------------
// Write formatted info into the report window
// -----------------------------------------------------------------------------

void CCALL	WriteReport (const char* format, ...)
{
	char		buffer [512];
	va_list		argList;

	va_start (argList, format);
#if defined (macintosh)
	vsnprintf (buffer, sizeof (buffer), format, argList);
#else
	vsnprintf_s (buffer, sizeof (buffer), _TRUNCATE, format, argList);
#endif

#if USE_DEBUG_WINDOW
	GS::UniString bufferPercent (buffer);
	bufferPercent.ReplaceAll ("%", "%%");
	DBPrintf (bufferPercent.ToCStr ().Get ());
	DBPrintf ("\n");
#else
	ACAPI_WriteReport (buffer, false);
#endif

	return;
}		// WriteReport


// -----------------------------------------------------------------------------
// Write formatted info into the report window
// Give an alert also (with the same content)
// -----------------------------------------------------------------------------

void CCALL	WriteReport_Alert (const char* format, ...)
{
	va_list		argList;

	va_start (argList, format);
	ACAPI_WriteReport (format, true, argList);
	va_end (argList);

	return;
}		// WriteReport_Alert


// -----------------------------------------------------------------------------
// Write an error into the report window and display an alert
// -----------------------------------------------------------------------------

void CCALL	WriteReport_Err (const char* info, GSErrCode err)
{
	ACAPI_WriteReport ("%s: %s", true, info, ErrID_To_Name (err));
}		// WriteReport_Err


// -----------------------------------------------------------------------------
// Write an end report
// -----------------------------------------------------------------------------

void CCALL	WriteReport_End (GSErrCode err)
{
#if USE_DEBUG_WINDOW
	DBPrintf ("\n");
	if (err == NoError) {
		DBPrintf ("OK\n");
	} else {
		DBPrintf ("Error: %s\n", ErrID_To_Name (err));
	}
#else
	if (err == NoError)
		ACAPI_WriteReport ("OK", false);
	else
		ACAPI_WriteReport ("Error: %s", false, ErrID_To_Name (err));
#endif

	return;
}		// WriteReport_End


// -----------------------------------------------------------------------------
// Write an error into the DebugMonitor and give beep in DEBUVERS
// -----------------------------------------------------------------------------

void 	ErrorBeep (const char* info, GSErrCode err)
{
	DBPrintf ("%s: %s", info, ErrID_To_Name (err));
	GSSysBeep ();
}		// ErrorBeep



// =============================================================================
//
//	Conversions
//
// =============================================================================
#ifdef __APPLE__
#pragma mark -
#endif


// -----------------------------------------------------------------------------
// Convert the NeigID to element type
// -----------------------------------------------------------------------------

API_ElemTypeID	Neig_To_ElemID (API_NeigID neigID)
{
	API_ElemTypeID	typeID;
	GSErrCode		err;

	err = ACAPI_Goodies (APIAny_NeigIDToElemTypeID, &neigID, &typeID);
	if (err != NoError)
		typeID = API_ZombieElemID;

	return typeID;
}		// Neig_To_ElemID


// -----------------------------------------------------------------------------
// Convert the element header to a neig
// -----------------------------------------------------------------------------

bool	ElemHead_To_Neig (API_Neig				*neig,
						  const API_Elem_Head	*elemHead)
{
	BNZeroMemory (neig, sizeof (API_Neig));
	API_Elem_Head* elemHeadNonConst = const_cast<API_Elem_Head*>(elemHead);
	neig->guid = elemHead->guid;
	if (elemHeadNonConst->typeID == API_ZombieElemID && neig->guid != APINULLGuid) {
		BNZeroMemory (elemHeadNonConst, sizeof (API_Elem_Head));
		elemHeadNonConst->guid = neig->guid;
		ACAPI_Element_GetHeader (elemHeadNonConst);
	}

	switch (elemHeadNonConst->typeID) {
		case API_WallID:					neig->neigID = APINeig_Wall;				neig->inIndex = 1;	break;
		case API_ColumnID:					neig->neigID = APINeig_Colu;				neig->inIndex = 0;	break;
		case API_BeamID:					neig->neigID = APINeig_Beam;				neig->inIndex = 1;	break;
		case API_WindowID:					neig->neigID = APINeig_WindHole;			neig->inIndex = 0;	break;
		case API_DoorID:					neig->neigID = APINeig_DoorHole;			neig->inIndex = 0;	break;
		case API_ObjectID:					neig->neigID = APINeig_Symb;				neig->inIndex = 1;	break;
		case API_LampID:					neig->neigID = APINeig_Light;				neig->inIndex = 1;	break;
		case API_SlabID:					neig->neigID = APINeig_Ceil;				neig->inIndex = 1;	break;
		case API_RoofID:					neig->neigID = APINeig_Roof;				neig->inIndex = 1;	break;
		case API_MeshID:					neig->neigID = APINeig_Mesh;				neig->inIndex = 1;	break;

		case API_DimensionID:				neig->neigID = APINeig_DimOn;				neig->inIndex = 1;	break;
		case API_RadialDimensionID:			neig->neigID = APINeig_RadDim;				neig->inIndex = 1;	break;
		case API_LevelDimensionID:			neig->neigID = APINeig_LevDim;				neig->inIndex = 1;	break;
		case API_AngleDimensionID:			neig->neigID = APINeig_AngDimOn;			neig->inIndex = 1;	break;

		case API_TextID:					neig->neigID = APINeig_Word;				neig->inIndex = 1;	break;
		case API_LabelID:					neig->neigID = APINeig_Label;				neig->inIndex = 1;	break;
		case API_ZoneID:					neig->neigID = APINeig_Room;				neig->inIndex = 1;	break;

		case API_HatchID:					neig->neigID = APINeig_Hatch;				neig->inIndex = 1;	break;
		case API_LineID:					neig->neigID = APINeig_Line;				neig->inIndex = 1;	break;
		case API_PolyLineID:				neig->neigID = APINeig_PolyLine;			neig->inIndex = 1;	break;
		case API_ArcID:						neig->neigID = APINeig_Arc;					neig->inIndex = 1;	break;
		case API_CircleID:					neig->neigID = APINeig_Circ;				neig->inIndex = 1;	break;
		case API_SplineID:					neig->neigID = APINeig_Spline;				neig->inIndex = 1;	break;
		case API_HotspotID:					neig->neigID = APINeig_Hot;					neig->inIndex = 1;	break;

		case API_CutPlaneID:				neig->neigID = APINeig_CutPlane;			neig->inIndex = 1;	break;
		case API_ElevationID:				neig->neigID = APINeig_Elevation;			neig->inIndex = 1;	break;
		case API_InteriorElevationID:		neig->neigID = APINeig_InteriorElevation;	neig->inIndex = 1;	break;
		case API_CameraID:					neig->neigID = APINeig_Camera;				neig->inIndex = 1;	break;
		case API_CamSetID:					return false;

		case API_PictureID:					neig->neigID = APINeig_PictObj;				neig->inIndex = 1;	break;
		case API_DetailID:					neig->neigID = APINeig_Detail;				neig->inIndex = 1;	break;
		case API_WorksheetID:				neig->neigID = APINeig_Worksheet;			neig->inIndex = 1;	break;

		case API_SectElemID:				neig->neigID = APINeig_VirtSy;				neig->inIndex = 1;	break;
		case API_DrawingID:					neig->neigID = APINeig_DrawingCenter;		neig->inIndex = 1;	break;

		case API_CurtainWallID:				neig->neigID = APINeig_CurtainWall;			neig->inIndex = 1;	break;
		case API_CurtainWallSegmentID:		neig->neigID = APINeig_CWSegment;			neig->inIndex = 1;	break;
		case API_CurtainWallFrameID:		neig->neigID = APINeig_CWFrame;				neig->inIndex = 1;	break;
		case API_CurtainWallPanelID:		neig->neigID = APINeig_CWPanel;				neig->inIndex = 1;	break;
		case API_CurtainWallJunctionID:		neig->neigID = APINeig_CWJunction;			neig->inIndex = 1;	break;
		case API_CurtainWallAccessoryID:	neig->neigID = APINeig_CWAccessory;			neig->inIndex = 1;	break;
		case API_ShellID:					neig->neigID = APINeig_Shell;				neig->inIndex = 1;	break;
		case API_SkylightID:				neig->neigID = APINeig_SkylightHole;		neig->inIndex = 0;	break;
		case API_MorphID:					neig->neigID = APINeig_Morph;				neig->inIndex = 1;	break;
		case API_ChangeMarkerID:			neig->neigID = APINeig_ChangeMarker;		neig->inIndex = 1;	break;

		case API_GroupID:
		case API_HotlinkID:
		default:
				return false;
	}

	return true;
}		// ElemHead_To_Neig


// -----------------------------------------------------------------------------
// Return a descriptive name for an error code
// -----------------------------------------------------------------------------

const char*		ErrID_To_Name (GSErrCode err)
{
	const char	*str;

	switch (err) {
		case APIERR_GENERAL:			str = "APIERR_GENERAL";			break;
		case APIERR_MEMFULL:			str = "APIERR_MEMFULL";			break;
		case APIERR_CANCEL:				str = "APIERR_CANCEL";			break;

		case APIERR_BADID:				str = "APIERR_BADID";			break;
		case APIERR_BADINDEX:			str = "APIERR_BADINDEX";		break;
		case APIERR_BADNAME:			str = "APIERR_BADNAME";			break;
		case APIERR_BADPARS:			str = "APIERR_BADPARS";			break;
		case APIERR_BADPOLY:			str = "APIERR_BADPOLY";			break;
		case APIERR_BADDATABASE:		str = "APIERR_BADDATABASE";		break;
		case APIERR_BADWINDOW:			str = "APIERR_BADWINDOW";		break;
		case APIERR_BADKEYCODE:			str = "APIERR_BADKEYCODE";		break;
		case APIERR_BADPLATFORMSIGN:	str = "APIERR_BADPLATFORMSIGN";	break;
		case APIERR_BADPLANE:			str = "APIERR_BADPLANE";		break;
		case APIERR_BADUSERID:			str = "APIERR_BADUSERID";		break;
		case APIERR_BADVALUE:			str = "APIERR_BADVALUE";		break;
		case APIERR_BADELEMENTTYPE:		str = "APIERR_BADELEMENTTYPE";	break;
		case APIERR_IRREGULARPOLY:		str = "APIERR_IRREGULARPOLY";	break;

		case APIERR_NO3D:				str = "APIERR_NO3D";			break;
		case APIERR_NOMORE:				str = "APIERR_NOMORE";			break;
		case APIERR_NOPLAN:				str = "APIERR_NOPLAN";			break;
		case APIERR_NOLIB:				str = "APIERR_NOLIB";			break;
		case APIERR_NOLIBSECT:			str = "APIERR_NOLIBSECT";		break;
		case APIERR_NOSEL:				str = "APIERR_NOSEL";			break;
		case APIERR_NOTEDITABLE:		str = "APIERR_NOTEDITABLE";		break;
		case APIERR_NOTSUBTYPEOF:		str = "APIERR_NOTSUBTYPEOF";	break;
		case APIERR_NOTEQUALMAIN:		str = "APIERR_NOTEQUALMAIN";	break;
		case APIERR_NOTEQUALREVISION:	str = "APIERR_NOTEQUALREVISION";	break;
		case APIERR_NOTEAMWORKPROJECT:	str = "APIERR_NOTEAMWORKPROJECT";	break;

		case APIERR_NOUSERDATA:			str = "APIERR_NOUSERDATA";		break;
		case APIERR_MOREUSER:			str = "APIERR_MOREUSER";		break;
		case APIERR_LINKEXIST:			str = "APIERR_LINKEXIST";		break;
		case APIERR_LINKNOTEXIST:		str = "APIERR_LINKNOTEXIST";	break;
		case APIERR_WINDEXIST:			str = "APIERR_WINDEXIST";		break;
		case APIERR_WINDNOTEXIST:		str = "APIERR_WINDNOTEXIST";	break;
		case APIERR_UNDOEMPTY:			str = "APIERR_UNDOEMPTY";		break;
		case APIERR_REFERENCEEXIST:		str = "APIERR_REFERENCEEXIST";	break;
		case APIERR_NAMEALREADYUSED:	str = "APIERR_NAMEALREADYUSED";	break;

		case APIERR_ATTREXIST:					str = "APIERR_ATTREXIST";					break;
		case APIERR_DELETED:					str = "APIERR_DELETED";						break;
		case APIERR_LOCKEDLAY:					str = "APIERR_LOCKEDLAY";					break;
		case APIERR_HIDDENLAY:					str = "APIERR_HIDDENLAY";					break;
		case APIERR_INVALFLOOR:					str = "APIERR_INVALFLOOR";					break;
		case APIERR_NOTMINE:					str = "APIERR_NOTMINE";						break;
		case APIERR_NOACCESSRIGHT:				str = "APIERR_NOACCESSRIGHT";				break;
		case APIERR_BADPROPERTY:			str = "APIERR_BADPROPERTY";			break;
		case APIERR_BADCLASSIFICATION:	str = "APIERR_BADCLASSIFICATION";	break;

		case APIERR_MODULNOTINSTALLED:			str = "APIERR_MODULNOTINSTALLED";			break;
		case APIERR_MODULCMDMINE:				str = "APIERR_MODULCMDMINE";				break;
		case APIERR_MODULCMDNOTSUPPORTED:		str = "APIERR_MODULCMDNOTSUPPORTED";		break;
		case APIERR_MODULCMDVERSNOTSUPPORTED:	str = "APIERR_MODULCMDVERSNOTSUPPORTED";	break;
		case APIERR_NOMODULEDATA:				str = "APIERR_NOMODULEDATA";				break;

		case APIERR_PAROVERLAP:			str = "APIERR_PAROVERLAP";		break;
		case APIERR_PARMISSING:			str = "APIERR_PARMISSING";		break;
		case APIERR_PAROVERFLOW:		str = "APIERR_PAROVERFLOW";		break;
		case APIERR_PARIMPLICIT:		str = "APIERR_PARIMPLICIT";		break;

		case APIERR_RUNOVERLAP:			str = "APIERR_RUNOVERLAP";		break;
		case APIERR_RUNMISSING:			str = "APIERR_RUNMISSING";		break;
		case APIERR_RUNOVERFLOW:		str = "APIERR_RUNOVERFLOW";		break;
		case APIERR_RUNIMPLICIT:		str = "APIERR_RUNIMPLICIT";		break;
		case APIERR_RUNPROTECTED:		str = "APIERR_RUNPROTECTED";	break;

		case APIERR_EOLOVERLAP:			str = "APIERR_EOLOVERLAP";		break;

		case APIERR_TABOVERLAP:			str = "APIERR_TABOVERLAP";		break;

		case APIERR_NOTINIT:			str = "APIERR_NOTINIT";			break;
		case APIERR_NESTING:			str = "APIERR_NESTING";			break;
		case APIERR_NOTSUPPORTED:		str = "APIERR_NOTSUPPORTED";	break;
		case APIERR_REFUSEDCMD:			str = "APIERR_REFUSEDCMD";		break;
		case APIERR_REFUSEDPAR:			str = "APIERR_REFUSEDPAR";		break;
		case APIERR_READONLY:			str = "APIERR_READONLY";		break;
		case APIERR_SERVICEFAILED:		str = "APIERR_SERVICEFAILED";	break;
		case APIERR_COMMANDFAILED:		str = "APIERR_COMMANDFAILED";	break;
		case APIERR_NEEDSUNDOSCOPE:		str = "APIERR_NEEDSUNDOSCOPE";	break;

		case APIERR_MISSINGCODE:		str = "APIERR_MISSINGCODE";		break;
		case APIERR_MISSINGDEF:			str = "APIERR_MISSINGDEF";		break;
		default:						str = "???";					break;
	}

	return str;
}		// ErrID_To_Name


// -----------------------------------------------------------------------------
// Return a descriptive name for a library part type
// -----------------------------------------------------------------------------

const char*		LibID_To_Name (API_LibTypeID typeID)
{
	static char libNames[][32] = {
		"Zombie",

		"Spec",

		"Window",
		"Door",
		"Object",
		"Lamp",
		"Room",

		"Property",

		"PlanSign",
		"Label",

		"Macro",
		"Pict",
		"List Scheme",
		"Skylight"
	};

	if (typeID < API_ZombieLibID || typeID > APILib_SkylightID)
		return "???";

	return libNames[typeID];

}		// LibID_To_Name


// -----------------------------------------------------------------------------
// Return a descriptive name for an attribute type
// -----------------------------------------------------------------------------

const char*		AttrID_To_Name (API_AttrTypeID typeID)
{
	static char attrNames[][32] = {
		"Zombie",

		"Pen",
		"Layer",
		"Linetype",
		"Fill type",
		"Composite Wall",
		"Surface",
		"City",
		"Layer Combination",
		"Zone Category",
		"Font",
		"Profile",
		"Pen table",
		"Dimension style",
		"Model View options",
		"MEP System",
		"Operation Profile",
		"Building Material",
	};

	if (typeID < API_ZombieAttrID || typeID > API_LastAttributeID)
		return "???";

	return attrNames[typeID];
}		// AttrID_To_Name


// -----------------------------------------------------------------------------
// Return a descriptive name for an element type
// -----------------------------------------------------------------------------

const GS::UniString		ElemID_To_Name (API_ElemTypeID typeID)
{
	GS::UniString	elemNameStr;

	ACAPI_Goodies (APIAny_GetElemTypeNameID, (void*) typeID, &elemNameStr);

	return elemNameStr;
}		// ElemID_To_Name



// =============================================================================
//
//	Interface support
//
// =============================================================================
#ifdef __APPLE__
#pragma mark -
#endif

// -----------------------------------------------------------------------------
// Convert an API_Coord3D to an API_Coord
// -----------------------------------------------------------------------------

inline API_Coord ToCoord (const API_Coord3D& inCoo)
{
	API_Coord coo = { inCoo.x, inCoo.y };
	return coo;
}

// -----------------------------------------------------------------------------
// Ask the user to click a point
// -----------------------------------------------------------------------------

bool	ClickAPoint (const char		*prompt,
					 API_Coord		*c)
{
	API_GetPointType	pointInfo = {};
	GSErrCode			err;

	CHTruncate (prompt, pointInfo.prompt, sizeof (pointInfo.prompt));
	pointInfo.changeFilter = false;
	pointInfo.changePlane  = false;
	err = ACAPI_Interface (APIIo_GetPointID, &pointInfo, nullptr);
	if (err != NoError) {
		if (err != APIERR_CANCEL)
			ACAPI_WriteReport ("Error in APIIo_GetPointID: %s", true, ErrID_To_Name (err));
		return false;
	}

	c->x = pointInfo.pos.x;
	c->y = pointInfo.pos.y;

	return true;
}		// ClickAPoint


// -----------------------------------------------------------------------------
// Input an arc
// -----------------------------------------------------------------------------

bool	GetAnArc (const char*	prompt,
				  API_Coord*	origin,
				  API_Coord*	startPos,
				  API_Coord*	endPos,
				  bool*			isArcNegative /*= nullptr*/)
{
	API_GetPointType	pointInfo = {};
	API_GetLineType		lineInfo = {};
	API_GetArcType		arcInfo = {};
	GSErrCode			err;

	CHTruncate (prompt, pointInfo.prompt, sizeof (pointInfo.prompt));
	err = ACAPI_Interface (APIIo_GetPointID, &pointInfo, nullptr);

	if (err == NoError) {
		CHTruncate (prompt, lineInfo.prompt, sizeof (lineInfo.prompt));
		lineInfo.startCoord = pointInfo.pos;						// line starts with the clicked point
		lineInfo.disableDefaultFeedback = false;					// draw the default thick rubber line

		err = ACAPI_Interface (APIIo_GetLineID, &lineInfo, nullptr);
	}

	if (err == NoError) {
		CHTruncate (prompt, arcInfo.prompt, sizeof (arcInfo.prompt));
		arcInfo.origo = lineInfo.startCoord;						// set arc origo
		arcInfo.startCoord = lineInfo.pos;							// arc starts with the second clicked point
		arcInfo.startCoordGiven = true;
		arcInfo.disableDefaultFeedback = false;						// draw the default thick rubber line
		err = ACAPI_Interface (APIIo_GetArcID, &arcInfo, nullptr);
	}

	if (err == NoError) {
		if (origin != nullptr)
			*origin = ToCoord (arcInfo.origo);
		if (startPos != nullptr)
			*startPos = ToCoord (arcInfo.startCoord);
		if (endPos != nullptr)
			*endPos = ToCoord (arcInfo.pos);
		if (isArcNegative != nullptr)
			*isArcNegative = arcInfo.negArc ? true : false;
	}

	return (err == NoError);
}		// GetAnArc


// -----------------------------------------------------------------------------
// Ask the user to click an element
// 'needTypeID' specifies the requested element type
//	- API_ZombieElemID: all types pass
//	- API_XXXID: 		only this type will pass
// Return:
//	true:	the user clicked the correct element
//	false:	the input is canceled or wrong type of element was clicked
// -----------------------------------------------------------------------------

bool	ClickAnElem (const char			*prompt,
					 API_ElemTypeID		needTypeID,
					 API_Neig			*neig /*= nullptr*/,
					 API_ElemTypeID		*typeID /*= nullptr*/,
					 API_Guid			*guid /*= nullptr*/,
					 API_Coord3D		*c /*= nullptr*/,
					 bool				ignorePartialSelection /*= true*/)
{
	API_GetPointType	pointInfo = {};
	API_ElemTypeID		clickedID;
	GSErrCode			err;

	CHTruncate (prompt, pointInfo.prompt, sizeof (pointInfo.prompt));
	pointInfo.changeFilter = false;
	pointInfo.changePlane  = false;
	err = ACAPI_Interface (APIIo_GetPointID, &pointInfo, nullptr);
	if (err != NoError) {
		if (err != APIERR_CANCEL)
			ACAPI_WriteReport ("Error in APIIo_GetPointID: %s", true, ErrID_To_Name (err));
		return false;
	}

	if (pointInfo.neig.neigID == APINeig_None) {		// try to find polygonal element clicked inside the polygon area
		API_Elem_Head elemHead;
		BNZeroMemory (&elemHead, sizeof (API_Elem_Head));
		API_ElemSearchPars	pars;
		BNZeroMemory (&pars, sizeof (API_ElemSearchPars));
		pars.typeID = needTypeID;
		pars.loc.x = pointInfo.pos.x;
		pars.loc.y = pointInfo.pos.y;
		pars.z = 1.00E6;
		pars.filterBits = APIFilt_OnVisLayer | APIFilt_OnActFloor;
		err = ACAPI_Goodies (APIAny_SearchElementByCoordID, &pars, &elemHead.guid);
		if (err == NoError) {
			elemHead.typeID = pars.typeID;
			ElemHead_To_Neig (&pointInfo.neig, &elemHead);
		}
	}

	if (pointInfo.neig.elemPartType != APINeigElemPart_None && ignorePartialSelection) {
		pointInfo.neig.elemPartType = APINeigElemPart_None;
		pointInfo.neig.elemPartIndex = 0;
	}

	clickedID = Neig_To_ElemID (pointInfo.neig.neigID);

	if (neig != nullptr)
		*neig = pointInfo.neig;
	if (typeID != nullptr)
		*typeID = clickedID;
	if (guid != nullptr)
		*guid = pointInfo.neig.guid;
	if (c != nullptr)
		*c = pointInfo.pos;

	if (clickedID == API_ZombieElemID)
		return false;

	bool good = (needTypeID == API_ZombieElemID || needTypeID == clickedID);

	if (!good && clickedID == API_SectElemID) {
		API_Element element;
		BNZeroMemory (&element, sizeof (API_Element));
		element.header.guid = pointInfo.neig.guid;
		if (ACAPI_Element_Get (&element) == NoError)
			good = (needTypeID == element.sectElem.parentID);
	}

	return good;
}		// ClickAnElem


// -----------------------------------------------------------------------------
// Ask the user to click several elements of the requested type
//	return the neigs
// -----------------------------------------------------------------------------

GS::Array<API_Neig>	ClickElements_Neig (const char		*prompt,
										API_ElemTypeID	needTypeID)
{
	API_Neig			theNeig;
	GS::Array<API_Neig> neigs;

	while (true) {
		if (ClickAnElem (prompt, needTypeID, &theNeig)) {
			if (theNeig.neigID == APINeig_None)
				break;
			neigs.Push (theNeig);
		} else
			break;
	}

	return neigs;
}		// ClickElements_Neig


// -----------------------------------------------------------------------------
// Ask the user to click several elements of the requested type
//	return element guids
// -----------------------------------------------------------------------------

GS::Array<API_Guid>	ClickElements_Guid (const char		*prompt,
										API_ElemTypeID	needTypeID)
{
	GS::Array<API_Guid> elemGuids;
	GS::Array<API_Neig> neigs = ClickElements_Neig (prompt, needTypeID);
	for (const API_Neig& neig : neigs)
		elemGuids.Push (neig.guid);

	return elemGuids;
}		// ClickElements_Guid


// -----------------------------------------------------------------------------
// Return the state of a checked menu item
// -----------------------------------------------------------------------------

bool		GetMenuItemMark (short menuResID, short itemIndex)
{
	API_MenuItemRef		itemRef;
	GSFlags				itemFlags;

	BNZeroMemory (&itemRef, sizeof (API_MenuItemRef));
	itemRef.menuResID = menuResID;
	itemRef.itemIndex = itemIndex;
	itemFlags = 0;

	ACAPI_Interface (APIIo_GetMenuItemFlagsID, &itemRef, &itemFlags);

	return (bool) ((itemFlags & API_MenuItemChecked) != 0);
}		// GetMenuItemMark


// -----------------------------------------------------------------------------
// Toggle a checked menu item
// -----------------------------------------------------------------------------

bool		InvertMenuItemMark (short menuResID, short itemIndex)
{
	API_MenuItemRef		itemRef;
	GSFlags				itemFlags;

	BNZeroMemory (&itemRef, sizeof (API_MenuItemRef));
	itemRef.menuResID = menuResID;
	itemRef.itemIndex = itemIndex;
	itemFlags = 0;

	ACAPI_Interface (APIIo_GetMenuItemFlagsID, &itemRef, &itemFlags);

	if ((itemFlags & API_MenuItemChecked) == 0)
		itemFlags |= API_MenuItemChecked;
	else
		itemFlags &= ~API_MenuItemChecked;

	ACAPI_Interface (APIIo_SetMenuItemFlagsID, &itemRef, &itemFlags);

	return (bool) ((itemFlags & API_MenuItemChecked) != 0);
}		// InvertMenuItemMark


// -----------------------------------------------------------------------------
// Disable/Enable a menu item
// -----------------------------------------------------------------------------

void		DisableEnableMenuItem (short menuResID, short itemIndex, bool disable)
{
	API_MenuItemRef		itemRef;
	GSFlags				itemFlags;

	BNZeroMemory (&itemRef, sizeof (API_MenuItemRef));
	itemRef.menuResID = menuResID;
	itemRef.itemIndex = itemIndex;
	itemFlags = 0;

	ACAPI_Interface (APIIo_GetMenuItemFlagsID, &itemRef, &itemFlags);

	if (disable)
		itemFlags |= API_MenuItemDisabled;
	else
		itemFlags &= ~API_MenuItemDisabled;

	ACAPI_Interface (APIIo_SetMenuItemFlagsID, &itemRef, &itemFlags);

	return;
}		// DisEnableMenuItem


// =============================================================================
//
//	Geometry support
//
// =============================================================================
#ifdef __APPLE__
#pragma mark -
#endif

// -----------------------------------------------------------------------------
// Tell whether an arc starts from the given node in the polygon
//	Return:
//		-1		no arc starts from the given node
//		(long)	the index into the polygon arcs array
// -----------------------------------------------------------------------------

Int32		FindArc (const API_PolyArc *parcs, Int32 nArcs, Int32 node)
{
	Int32		i;

	if (parcs == nullptr)
		return (-1);
	for (i = 0; i < nArcs; i++)
		if (parcs [i].begIndex == node)
			return (i);
	return (-1);
}		// FindArc


// -----------------------------------------------------------------------------
// Return the origin of the given arc
// -----------------------------------------------------------------------------

bool		ArcGetOrigo (const API_Coord	*begC,
						 const API_Coord	*endC,
						 double				angle,
						 API_Coord			*origo)
{
	double	xm, ym, m;

	if (fabs (angle) < EPS) {
		origo->x = 0.0;
		origo->y = 0.0;
		return false;
	}
	xm = begC->x + endC->x;
	ym = begC->y + endC->y;
	if (fabs (fabs (angle) - PI) < EPS) {
		origo->x = xm / 2;
		origo->y = ym / 2;
	} else {
		m = 1.0 / tan (angle / 2.0);
		origo->x = (xm - m * (endC->y - begC->y)) / 2;
		origo->y = (ym + m * (endC->x - begC->x)) / 2;
	}
	return true;
}		// ArcGetOrigo


// -----------------------------------------------------------------------------
// Return the angle of the line between the two 2D points
// -----------------------------------------------------------------------------

double		ComputeFiPtr (const API_Coord *c1, const API_Coord *c2, bool enableNegativeAngle /*= false*/)
{
	double		fi;
	double		dx,dy;

	dx = c2->x - c1->x;
	dy = c2->y - c1->y;
	if (fabs (dx) < EPS && fabs (dy) < EPS)
		fi = 0.0;
	else {
		fi = atan2 (dy, dx);
		if (fi < 0.0 && !enableNegativeAngle)
			fi = fi + 2.0 * PI;
	}
	return (fi);
}		// ComputeFiPtr


// -----------------------------------------------------------------------------
// Return the distance of the two 2D points
// -----------------------------------------------------------------------------

double		DistCPtr (const API_Coord *c1, const API_Coord *c2)
{
	double		dx, dy;

	dx = c1->x - c2->x;
	dy = c1->y - c2->y;

	return (sqrt (dx * dx + dy * dy));
}		// DistCPtr

