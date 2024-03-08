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
#ifdef AC_27
#include	"APICommon27.h"
#endif // AC_27
#include	"Helpers.hpp"
#include	"Model3D/model.h"
#include	"Model3D/MeshBody.hpp"
#include	"VectorImageIterator.hpp"
#include	"ProfileVectorImageOperations.hpp"
#include	"ProfileAdditionalInfo.hpp"

#if defined AC_26 || defined AC_27

API_ElemType	NeigToElemID(API_NeigID neigID)
{
	API_ElemType	type;
	GSErrCode		err;
#ifdef AC_27
	err = ACAPI_Element_NeigIDToElemType(neigID, type);
#else
	err = ACAPI_Goodies_NeigIDToElemType(neigID, type);
#endif
	if (err != NoError)
		type = API_ZombieElemID;

	return type;
}		// Neig_To_ElemID

bool	ElemHeadToNeig(API_Neig* neig,
	const API_Elem_Head* elemHead)
{
	*neig = {};
	neig->guid = elemHead->guid;

	API_ElemType type = elemHead->type;
	if (type == API_ZombieElemID && neig->guid != APINULLGuid) {
		API_Elem_Head elemHeadCopy = {};
		elemHeadCopy.guid = elemHead->guid;
		ACAPI_Element_GetHeader(&elemHeadCopy);
		type = elemHeadCopy.type;
	}

	switch (type.typeID) {
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

	case API_StairID:						neig->neigID = APINeig_Stair;			neig->inIndex = 1;	break;
	case API_RiserID:						neig->neigID = APINeig_Riser;			neig->inIndex = 1;	break;
	case API_TreadID:						neig->neigID = APINeig_Tread;			neig->inIndex = 1;	break;
	case API_StairStructureID:				neig->neigID = APINeig_StairStructure;	neig->inIndex = 1;	break;

	case API_RailingID:						neig->neigID = APINeig_Railing;						neig->inIndex = 1;	break;
	case API_RailingToprailID:				neig->neigID = APINeig_RailingToprail;				neig->inIndex = 1;	break;
	case API_RailingHandrailID:				neig->neigID = APINeig_RailingHandrail;				neig->inIndex = 1;	break;
	case API_RailingRailID:					neig->neigID = APINeig_RailingRail;					neig->inIndex = 1;	break;
	case API_RailingPostID:					neig->neigID = APINeig_RailingPost;					neig->inIndex = 1;	break;
	case API_RailingInnerPostID:			neig->neigID = APINeig_RailingInnerPost;			neig->inIndex = 1;	break;
	case API_RailingBalusterID:				neig->neigID = APINeig_RailingBaluster;				neig->inIndex = 1;	break;
	case API_RailingPanelID:				neig->neigID = APINeig_RailingPanel;				neig->inIndex = 1;	break;
	case API_RailingSegmentID:				return false;
	case API_RailingNodeID:					return false;
	case API_RailingBalusterSetID:			return false;
	case API_RailingPatternID:				return false;
	case API_RailingToprailEndID:			neig->neigID = APINeig_RailingToprailEnd;			neig->inIndex = 1;	break;
	case API_RailingHandrailEndID:			neig->neigID = APINeig_RailingHandrailEnd;			neig->inIndex = 1;	break;
	case API_RailingRailEndID:				neig->neigID = APINeig_RailingRailEnd;				neig->inIndex = 1;	break;
	case API_RailingToprailConnectionID:	neig->neigID = APINeig_RailingToprailConnection;	neig->inIndex = 1;	break;
	case API_RailingHandrailConnectionID:	neig->neigID = APINeig_RailingHandrailConnection;	neig->inIndex = 1;	break;
	case API_RailingRailConnectionID:		neig->neigID = APINeig_RailingRailConnection;		neig->inIndex = 1;	break;
	case API_RailingEndFinishID:			neig->neigID = APINeig_RailingEndFinish;			neig->inIndex = 1;	break;

	case API_BeamSegmentID:					neig->neigID = APINeig_BeamSegment;					neig->inIndex = 1;	break;
	case API_ColumnSegmentID:				neig->neigID = APINeig_ColumnSegment;				neig->inIndex = 1;	break;
	case API_OpeningID:						return false;

	case API_GroupID:
	case API_HotlinkID:
	default:
		return false;
	}

	static_assert (API_LastElemType == API_OpeningID, "Do not forget to update ElemHead_To_Neig function after new element type was introduced!");

	return true;
}		// ElemHead_To_Neig

bool	GetAnElem(const char* prompt,
	const API_ElemType& needType,
	API_Neig* neig /*= nullptr*/,
	API_ElemType* type /*= nullptr*/,
	API_Guid* guid /*= nullptr*/,
	API_Coord3D* c /*= nullptr*/,
	bool					ignorePartialSelection /*= true*/)
{
	API_GetPointType	pointInfo = {};
	API_ElemType		clickedType;
	GSErrCode			err;

	CHTruncate(prompt, pointInfo.prompt, sizeof(pointInfo.prompt));
	pointInfo.changeFilter = false;
	pointInfo.changePlane = false;
#ifdef AC_27
	err = ACAPI_UserInput_GetPoint(&pointInfo, nullptr);
#else
	err = ACAPI_Interface(APIIo_GetPointID, &pointInfo, nullptr);
#endif
	if (err != NoError) {
		return false;
	}

	if (pointInfo.neig.neigID == APINeig_None) {		// try to find polygonal element clicked inside the polygon area
		API_Elem_Head		elemHead = {};
		API_ElemSearchPars	pars = {};
		pars.type = needType;
		pars.loc.x = pointInfo.pos.x;
		pars.loc.y = pointInfo.pos.y;
		pars.z = 1.00E6;
		pars.filterBits = APIFilt_OnVisLayer | APIFilt_OnActFloor;
#ifdef AC_27
		err = ACAPI_Element_SearchElementByCoord(&pars, &elemHead.guid);
#else
		err = ACAPI_Goodies(APIAny_SearchElementByCoordID, &pars, &elemHead.guid);
#endif
		if (err == NoError) {
			elemHead.type = pars.type;
			ElemHeadToNeig(&pointInfo.neig, &elemHead);
		}
	}

	if (pointInfo.neig.elemPartType != APINeigElemPart_None && ignorePartialSelection) {
		pointInfo.neig.elemPartType = APINeigElemPart_None;
		pointInfo.neig.elemPartIndex = 0;
	}

	clickedType = NeigToElemID(pointInfo.neig.neigID);

	if (neig != nullptr)
		*neig = pointInfo.neig;
	if (type != nullptr)
		*type = clickedType;
	if (guid != nullptr)
		*guid = pointInfo.neig.guid;
	if (c != nullptr)
		*c = pointInfo.pos;

	if (clickedType == API_ZombieElemID)
		return false;

	bool good = (needType == API_ZombieElemID || needType == clickedType);

	if (!good && clickedType == API_SectElemID) {
		API_Element element = {};
		element.header.guid = pointInfo.neig.guid;
		if (ACAPI_Element_Get(&element) == NoError)
			good = (needType == element.sectElem.parentType);
	}

	return good;
}		// ClickAnElem
#endif

#ifdef AC_25

// -----------------------------------------------------------------------------
// Convert the element header to a neig
// -----------------------------------------------------------------------------

bool	ElemHeadToNeig(API_Neig* neig,
	const API_Elem_Head* elemHead)
{
	BNZeroMemory(neig, sizeof(API_Neig));
	API_Elem_Head* elemHeadNonConst = const_cast<API_Elem_Head*>(elemHead);
	neig->guid = elemHead->guid;
	if (elemHeadNonConst->typeID == API_ZombieElemID && neig->guid != APINULLGuid) {
		BNZeroMemory(elemHeadNonConst, sizeof(API_Elem_Head));
		elemHeadNonConst->guid = neig->guid;
		ACAPI_Element_GetHeader(elemHeadNonConst);
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
// Convert the NeigID to element type
// -----------------------------------------------------------------------------

API_ElemTypeID	NeigToElemID(API_NeigID neigID)
{
	API_ElemTypeID	typeID;
	GSErrCode		err;

	err = ACAPI_Goodies(APIAny_NeigIDToElemTypeID, &neigID, &typeID);
	if (err != NoError)
		typeID = API_ZombieElemID;

	return typeID;
}		// Neig_To_ElemID
bool	GetAnElem(const char* prompt,
	API_ElemTypeID		needTypeID,
	API_Neig* neig /*= nullptr*/,
	API_ElemTypeID* typeID /*= nullptr*/,
	API_Guid* guid /*= nullptr*/,
	API_Coord3D* c /*= nullptr*/,
	bool				ignorePartialSelection /*= true*/)
{
	API_GetPointType	pointInfo = {};
	API_ElemTypeID		clickedID;
	GSErrCode			err;

	CHTruncate(prompt, pointInfo.prompt, sizeof(pointInfo.prompt));
	pointInfo.changeFilter = false;
	pointInfo.changePlane = false;
	err = ACAPI_Interface(APIIo_GetPointID, &pointInfo, nullptr);
	if (err != NoError) {
		if (err != APIERR_CANCEL)
			msg_rep("GetAnElem", "APIIo_GetPointID", err, APINULLGuid);
		return false;
	}

	if (pointInfo.neig.neigID == APINeig_None) {		// try to find polygonal element clicked inside the polygon area
		API_Elem_Head elemHead;
		BNZeroMemory(&elemHead, sizeof(API_Elem_Head));
		API_ElemSearchPars	pars;
		BNZeroMemory(&pars, sizeof(API_ElemSearchPars));
		pars.typeID = needTypeID;
		pars.loc.x = pointInfo.pos.x;
		pars.loc.y = pointInfo.pos.y;
		pars.z = 1.00E6;
		pars.filterBits = APIFilt_OnVisLayer | APIFilt_OnActFloor;
		err = ACAPI_Goodies(APIAny_SearchElementByCoordID, &pars, &elemHead.guid);
		if (err == NoError) {
			elemHead.typeID = pars.typeID;
			ElemHeadToNeig(&pointInfo.neig, &elemHead);
		}
	}

	if (pointInfo.neig.elemPartType != APINeigElemPart_None && ignorePartialSelection) {
		pointInfo.neig.elemPartType = APINeigElemPart_None;
		pointInfo.neig.elemPartIndex = 0;
	}

	clickedID = NeigToElemID(pointInfo.neig.neigID);

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
		BNZeroMemory(&element, sizeof(API_Element));
		element.header.guid = pointInfo.neig.guid;
		if (ACAPI_Element_Get(&element) == NoError)
			good = (needTypeID == element.sectElem.parentID);
	}

	return good;
}		// ClickAnElem
#endif

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
	GS::UniString unignoreval = GS::UniString(ignoreval.c_str(), GChCode);
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

bool UniStringToDouble(const GS::UniString& var, double& x) {
	if (var.IsEmpty()) return false;
	GS::UniString var_clear = var;
	var_clear.Trim();
	var_clear.ReplaceAll(",", ".");
	std::string var_str = var_clear.ToCStr(0, MaxUSize, GChCode).Get();
	int n = sscanf(var_str.c_str(), "%lf", &x);
	return n > 0;
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
	GetNumSymbSpase(outstring, '~', ' ');
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
	GSErrCode err = NoError;
#ifdef AC_27
	err = ACAPI_ProjectOperation_Project(&projectInfo);
#else
	err = ACAPI_Environment(APIEnv_ProjectID, &projectInfo);
#endif
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
	if (IsElementEditable(objectId, syncSettings, false)) {
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
	if (elementType == API_GroupID)
		return false;
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
#if defined AC_26 || defined AC_27
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

#ifdef AC_27
	if (ACAPI_Teamwork_HasConnection() && !ACAPI_Element_Filter(objectId, APIFilt_InMyWorkspace)) {
#else
	if (ACAPI_TeamworkControl_HasConnection() && !ACAPI_Element_Filter(objectId, APIFilt_InMyWorkspace)) {
#endif
#if defined(AC_24) || defined(AC_23)
		GS::PagedArray<API_Guid>	elements;
#else
		GS::Array<API_Guid>	elements;
#endif // AC_24

		GS::HashTable<API_Guid, short>  conflicts;
		elements.Push(objectId);
#ifdef AC_27
		ACAPI_Teamwork_ReserveElements(elements, &conflicts, true);
#else
		ACAPI_TeamworkControl_ReserveElements(elements, &conflicts);
#endif
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

void msg_rep(const GS::UniString & modulename, const GS::UniString & reportString, const GSErrCode & err, const API_Guid & elemGuid) {
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

#ifdef AC_27
			if (ACAPI_Element_GetElemTypeName(elem_head.type, elemName) == NoError) {
#else
#ifdef AC_26
			if (ACAPI_Goodies_GetElemTypeName(elem_head.type, elemName) == NoError) {
#else
			if (ACAPI_Goodies(APIAny_GetElemTypeNameID, (void*)elem_head.typeID, &elemName) == NoError) {
#endif
#endif
				error_type = error_type + " type:" + elemName;
			}
			API_Attribute layer;
			BNZeroMemory(&layer, sizeof(API_Attribute));
			layer.header.typeID = API_LayerID;
			layer.header.index = elem_head.layer;
			if (ACAPI_Attribute_Get(&layer) == NoError) error_type = error_type + " layer:" + layer.header.name;
		}
	}
	GS::UniString msg = modulename + ": " + reportString + " " + error_type + "\n";
	ACAPI_WriteReport(msg, false);
	if (err != NoError) {
		msg = "== SMSTF ERR ==";
	}
	DBPrintf(msg.ToCStr());
}

void	MenuItemCheckAC(short itemInd, bool checked) {
	API_MenuItemRef itemRef;
	GSFlags         itemFlags;

	BNZeroMemory(&itemRef, sizeof(API_MenuItemRef));
	itemRef.menuResID = 32500;
	itemRef.itemIndex = itemInd;

	itemFlags = 0;
#ifdef AC_27
	ACAPI_MenuItem_GetMenuItemFlags(&itemRef, &itemFlags);
#else
	ACAPI_Interface(APIIo_GetMenuItemFlagsID, &itemRef, &itemFlags);
#endif
	if (checked)
		itemFlags |= API_MenuItemChecked;
	else
		itemFlags &= ~API_MenuItemChecked;

#ifdef AC_27
	ACAPI_MenuItem_SetMenuItemFlags(&itemRef, &itemFlags);
#else
	ACAPI_Interface(APIIo_SetMenuItemFlagsID, &itemRef, &itemFlags);
#endif
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
GS::Array<API_Guid>	GetSelectedElements(bool assertIfNoSel /* = true*/, bool onlyEditable /*= true*/, SyncSettings & syncSettings, bool addSubelement) {
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
			GSErrCode err = NoError;
#if defined AC_26 || defined AC_27
			API_ElemType elemType26;
#ifdef AC_27
			err = ACAPI_Element_NeigIDToElemType(neigID, elemType26);
#else
			err = ACAPI_Goodies_NeigIDToElemType(neigID, elemType26);
#endif
			elementType = elemType26.typeID;
#else
			err = ACAPI_Goodies(APIAny_NeigIDToElemTypeID, &neigID, &elementType);
#endif // AC_26
			if (err == NoError) GetRelationsElement(neig.guid, elementType, syncSettings, guidArray);
		}
	}
	return guidArray;
#endif // AC_22
}

void CallOnSelectedElemSettings(void (*function)(const API_Guid&, const SyncSettings&), bool assertIfNoSel /* = true*/, bool onlyEditable /* = true*/, const SyncSettings & syncSettings, GS::UniString & funcname, bool addSubelement) {
	GS::Array<API_Guid> guidArray = GetSelectedElements(assertIfNoSel, onlyEditable, addSubelement);
	if (!guidArray.IsEmpty()) {
		GS::UniString subtitle("working...");
		GS::Int32 nPhase = 1;
#ifdef AC_27
		bool showPercent = true;
		Int32 maxval = guidArray.GetSize();
		ACAPI_ProcessWindow_InitProcessWindow(&funcname, &nPhase);
#else
		ACAPI_Interface(APIIo_InitProcessWindowID, &funcname, &nPhase);
#endif
		long time_start = clock();
		for (UInt32 i = 0; i < guidArray.GetSize(); i++) {
			function(guidArray[i], syncSettings);
#ifdef AC_27
			if (i % 10 == 0) ACAPI_ProcessWindow_SetNextProcessPhase(&subtitle, &maxval, &showPercent);
#else
			if (i % 10 == 0) ACAPI_Interface(APIIo_SetNextProcessPhaseID, &subtitle, &i);
#endif
#ifdef AC_27
			if (ACAPI_ProcessWindow_IsProcessCanceled()) return;
#else
			if (ACAPI_Interface(APIIo_IsProcessCanceledID, nullptr, nullptr)) return;
#endif
		}
		long time_end = clock();
		GS::UniString time = GS::UniString::Printf(" %d s", (time_end - time_start) / 1000);
		GS::UniString intString = GS::UniString::Printf(" %d qty", guidArray.GetSize());
		msg_rep(funcname + " Selected", intString + time, NoError, APINULLGuid);
#ifdef AC_27
		ACAPI_ProcessWindow_CloseProcessWindow();
#else
		ACAPI_Interface(APIIo_CloseProcessWindowID, nullptr, nullptr);
#endif
	}
}

// -----------------------------------------------------------------------------
// Вызов функции для выбранных элементов
//	(функция должна принимать в качетве аргумента API_Guid
// -----------------------------------------------------------------------------
void CallOnSelectedElem(void (*function)(const API_Guid&), bool assertIfNoSel /* = true*/, bool onlyEditable /* = true*/, GS::UniString & funcname, bool addSubelement) {
	GS::Array<API_Guid> guidArray = GetSelectedElements(assertIfNoSel, onlyEditable, addSubelement);
	if (!guidArray.IsEmpty()) {
		long time_start = clock();
		GS::UniString subtitle("working...");
		GS::Int32 nPhase = 1;
#ifdef AC_27
		bool showPercent = true;
		Int32 maxval = guidArray.GetSize();
		ACAPI_ProcessWindow_InitProcessWindow(&funcname, &nPhase);
#else
		ACAPI_Interface(APIIo_InitProcessWindowID, &funcname, &nPhase);
#endif
		for (UInt32 i = 0; i < guidArray.GetSize(); i++) {
#ifdef AC_27
			if (i % 10 == 0) ACAPI_ProcessWindow_SetNextProcessPhase(&subtitle, &maxval, &showPercent);
#else
			if (i % 10 == 0) ACAPI_Interface(APIIo_SetNextProcessPhaseID, &subtitle, &i);
#endif
			function(guidArray[i]);
#ifdef AC_27
			if (ACAPI_ProcessWindow_IsProcessCanceled()) return;
#else
			if (ACAPI_Interface(APIIo_IsProcessCanceledID, nullptr, nullptr)) return;
#endif
		}
		long time_end = clock();
		GS::UniString time = GS::UniString::Printf(" %d ms", (time_end - time_start) / 1000);
		GS::UniString intString = GS::UniString::Printf(" %d qty", guidArray.GetSize());
		msg_rep(funcname + " Selected", intString + time, NoError, APINULLGuid);
#ifdef AC_27
		ACAPI_ProcessWindow_CloseProcessWindow();
#else
		ACAPI_Interface(APIIo_CloseProcessWindowID, nullptr, nullptr);
#endif
	}
	else if (!assertIfNoSel) {
		function(APINULLGuid);
	}
}

// -----------------------------------------------------------------------------
// Получение типа объекта по его API_Guid
// -----------------------------------------------------------------------------
GSErrCode GetTypeByGUID(const API_Guid & elemGuid, API_ElemTypeID & elementType) {
	GSErrCode		err = NoError;
	API_Elem_Head elem_head;
	BNZeroMemory(&elem_head, sizeof(API_Elem_Head));
	elem_head.guid = elemGuid;
	err = ACAPI_Element_GetHeader(&elem_head);
	if (err != NoError) {
		msg_rep("GetTypeByGUID", "", err, elemGuid);
		return err;
	}
#if defined AC_26 || defined AC_27
	elementType = elem_head.type.typeID;
#else
	elementType = elem_head.typeID;
#endif
	return err;
}

#if defined AC_26 || defined AC_27
bool	GetElementTypeString(API_ElemType elemType, char* elemStr) {
	GS::UniString	ustr;
	GSErrCode	err = NoError;
#ifdef AC_27
	err = ACAPI_Element_GetElemTypeName(elemType, ustr);
#else
	err = ACAPI_Goodies_GetElemTypeName(elemType, ustr);
#endif
	if (err == NoError) {
		CHTruncate(ustr.ToCStr(), elemStr, ELEMSTR_LEN - 1);
		return true;
	}
	return false;
}
#else
bool	GetElementTypeString(API_ElemTypeID typeID, char* elemStr) {
	GS::UniString	ustr;
	GSErrCode	err = ACAPI_Goodies(APIAny_GetElemTypeNameID, (void*)typeID, &ustr);
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
void GetRelationsElement(const API_Guid & elemGuid, const SyncSettings & syncSettings, GS::Array<API_Guid>&subelemGuid) {
	API_ElemTypeID elementType;
	if (GetTypeByGUID(elemGuid, elementType) != NoError) return;
	GetRelationsElement(elemGuid, elementType, syncSettings, subelemGuid);
}

// --------------------------------------------------------------------
// Поиск связанных элементов
// --------------------------------------------------------------------
void GetRelationsElement(const API_Guid & elemGuid, const  API_ElemTypeID & elementType, const SyncSettings & syncSettings, GS::Array<API_Guid>&subelemGuid) {
	GSErrCode	err = NoError;
	API_RoomRelation	relData;
	GS::Array<API_ElemTypeID> typeinzone;

	API_HierarchicalOwnerType hierarchicalOwnerType = API_ParentHierarchicalOwner;
	API_HierarchicalElemType hierarchicalElemType = API_SingleElem;
	API_HierarchicalElemType hierarchicalElemType_root = API_SingleElem;
	API_Guid ownerElemApiGuid = APINULLGuid;
	API_Guid ownerElemApiGuid_root = APINULLGuid;
	API_Guid elemGuid_t = elemGuid;

#ifdef AC_27
	ACAPI_HierarchicalEditing_GetHierarchicalElementOwner(&elemGuid_t, &hierarchicalOwnerType, &hierarchicalElemType, &ownerElemApiGuid);
#else
	ACAPI_Goodies(APIAny_GetHierarchicalElementOwnerID, &elemGuid_t, &hierarchicalOwnerType, &hierarchicalElemType, &ownerElemApiGuid);
#endif
	hierarchicalOwnerType = API_RootHierarchicalOwner;
#ifdef AC_27
	ACAPI_HierarchicalEditing_GetHierarchicalElementOwner(&elemGuid_t, &hierarchicalOwnerType, &hierarchicalElemType, &ownerElemApiGuid_root);
#else
	ACAPI_Goodies(APIAny_GetHierarchicalElementOwnerID, &elemGuid_t, &hierarchicalOwnerType, &hierarchicalElemType, &ownerElemApiGuid_root);
#endif
	switch (elementType) {
	case API_WallID:
		if (syncSettings.widoS) {
			GS::Array<API_Guid> windows;
#ifdef AC_27
			err = ACAPI_Grouping_GetConnectedElements(elemGuid, API_WindowID, &windows, APIFilt_None, APINULLGuid);
#else
			err = ACAPI_Element_GetConnectedElements(elemGuid, API_WindowID, &windows);
#endif
			if (!windows.IsEmpty()) {
				for (UInt32 i = 0; i < windows.GetSize(); i++) {
					subelemGuid.Push(windows[i]);
				}
			}
			GS::Array<API_Guid> doors;
#ifdef AC_27
			err = ACAPI_Grouping_GetConnectedElements(elemGuid, API_DoorID, &doors, APIFilt_None, APINULLGuid);
#else
			err = ACAPI_Element_GetConnectedElements(elemGuid, API_DoorID, &doors);
#endif
			if (!doors.IsEmpty()) {
				for (UInt32 i = 0; i < doors.GetSize(); i++) {
					subelemGuid.Push(doors[i]);
				}
			}
			break;
		}
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
			if (ownerElemApiGuid != APINULLGuid && hierarchicalElemType == API_ChildElemInMultipleElem) subelemGuid.Push(ownerElemApiGuid);
			if (ownerElemApiGuid_root != ownerElemApiGuid && ownerElemApiGuid_root != APINULLGuid && hierarchicalElemType_root == API_ChildElemInMultipleElem) subelemGuid.Push(ownerElemApiGuid_root);
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
			if (ownerElemApiGuid != APINULLGuid && hierarchicalElemType == API_ChildElemInMultipleElem) subelemGuid.Push(ownerElemApiGuid);
			if (ownerElemApiGuid_root != ownerElemApiGuid && ownerElemApiGuid_root != APINULLGuid && hierarchicalElemType_root == API_ChildElemInMultipleElem) subelemGuid.Push(ownerElemApiGuid_root);
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
			if (ownerElemApiGuid != APINULLGuid && hierarchicalElemType == API_ChildElemInMultipleElem) subelemGuid.Push(ownerElemApiGuid);
			if (ownerElemApiGuid_root != ownerElemApiGuid && ownerElemApiGuid_root != APINULLGuid && hierarchicalElemType_root == API_ChildElemInMultipleElem) subelemGuid.Push(ownerElemApiGuid_root);
		}
		break;
	case API_ZoneID:
		if (syncSettings.objS) {
			err = ACAPI_Element_GetRelations(elemGuid, API_ZombieElemID, &relData);
			if (err == NoError) {
#ifdef AC_23
				for (Int32 i = 0; i < relData.nObject - 1; i++) {
					API_Guid elGuid = *(relData.objects)[i];
					subelemGuid.Push(elGuid);
				}
				if (syncSettings.widoS) {
					for (Int32 i = 0; i < relData.nWindow - 1; i++) {
						API_Guid elGuid = *(relData.windows)[i];
						subelemGuid.Push(elGuid);
					}
					for (Int32 i = 0; i < relData.nDoor - 1; i++) {
						API_Guid elGuid = *(relData.doors)[i];
						subelemGuid.Push(elGuid);
					}
					for (Int32 i = 0; i < relData.nSkylight - 1; i++) {
						API_Guid elGuid = *(relData.skylights)[i];
						subelemGuid.Push(elGuid);
					}
				}
				if (syncSettings.wallS) {
					for (Int32 i = 0; i < relData.nColumn - 1; i++) {
						API_Guid elGuid = *(relData.columns)[i];
						subelemGuid.Push(elGuid);
					}
					for (Int32 i = 0; i < relData.nWallPart - 1; i++) {
						API_Guid elGuid = relData.wallPart[i]->guid;
						subelemGuid.Push(elGuid);
					}
					for (Int32 i = 0; i < relData.nBeamPart - 1; i++) {
						API_Guid elGuid = relData.beamPart[i]->guid;
						subelemGuid.Push(elGuid);
					}
					for (Int32 i = 0; i < relData.nMorph - 1; i++) {
						API_Guid elGuid = *(relData.morphs)[i];
						subelemGuid.Push(elGuid);
					}
				}
#else
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
					typeinzone.Push(API_SkylightID);
				}
				if (syncSettings.cwallS) typeinzone.Push(API_CurtainWallID);
				for (const API_ElemTypeID& typeelem : typeinzone) {
					if (relData.elementsGroupedByType.ContainsKey(typeelem)) {
						for (const API_Guid& elGuid : relData.elementsGroupedByType[typeelem]) {
							subelemGuid.Push(elGuid);
						}
					}
				}
#endif
			}
		}
		break;
	default:
		break;
	}
	ACAPI_DisposeRoomRelationHdls(&relData);
}

// -----------------------------------------------------------------------------
// Возвращает уникальные вхождения текста
// -----------------------------------------------------------------------------
GS::UniString StringUnic(const GS::UniString & instring, const GS::UniString & delim) {
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
UInt32 StringSpltUnic(const GS::UniString & instring, const GS::UniString & delim, GS::Array<GS::UniString>&partstring) {
	if (!instring.Contains(delim)) {
		partstring.Push(instring);
		return 1;
	}
	GS::Array<GS::UniString> tpartstring;
	UInt32 n = StringSplt(instring, delim, tpartstring);
	std::map<std::string, int, doj::alphanum_less<std::string> > unic = {};
	for (UInt32 i = 0; i < n; i++) {
		std::string s = tpartstring[i].ToCStr(0, MaxUSize, GChCode).Get();
		unic[s];
	}
	UInt32 nout = 0;
	for (std::map<std::string, int, doj::alphanum_less<std::string> >::iterator k = unic.begin(); k != unic.end(); ++k) {
		std::string s = k->first;
		GS::UniString unis = GS::UniString(s.c_str(), GChCode);
		partstring.Push(unis);
		nout = nout + 1;
	}
	return nout;
}

// -----------------------------------------------------------------------------
// Делит строку по разделителю, возвращает кол-во частей
// -----------------------------------------------------------------------------
UInt32 StringSplt(const GS::UniString & instring, const GS::UniString & delim, GS::Array<GS::UniString>&partstring) {
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
UInt32 StringSplt(const GS::UniString & instring, const GS::UniString & delim, GS::Array<GS::UniString>&partstring, const GS::UniString & filter) {
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
GSErrCode GetRElementsForCWall(const API_Guid & cwGuid, GS::Array<API_Guid>&elementsSymbolGuids) {
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
#ifdef AC_27
			err = ACAPI_CurtainWall_IsCWPanelDegenerate(&memo.cWallPanels[idx].head.guid, &isDegenerate);
#else
			err = ACAPI_Database(APIDb_IsCWPanelDegenerateID, (void*)(&memo.cWallPanels[idx].head.guid), &isDegenerate);
#endif
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
GSErrCode GetRElementsForRailing(const API_Guid & elemGuid, GS::Array<API_Guid>&elementsGuids) {
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
bool ParamHelpers::ReadMorphParam(const API_Element & element, ParamDictValue & pdictvalue) {
	if (!element.header.hasMemo) return NoError;
	DBPrintf("== SMSTF ==      ReadMorphParam\n");
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
		ParamHelpers::AddValueToParamDictValue(pdictvaluemorph, element.header.guid, "morph:", "l", L);
		ParamHelpers::AddValueToParamDictValue(pdictvaluemorph, element.header.guid, "morph:", "lx", Lx);
		ParamHelpers::AddValueToParamDictValue(pdictvaluemorph, element.header.guid, "morph:", "ly", Ly);
		ParamHelpers::AddValueToParamDictValue(pdictvaluemorph, element.header.guid, "morph:", "lz", Lz);
		ParamHelpers::AddValueToParamDictValue(pdictvaluemorph, element.header.guid, "morph:", "max_x", Max_x);
		ParamHelpers::AddValueToParamDictValue(pdictvaluemorph, element.header.guid, "morph:", "min_x", Min_x);
		ParamHelpers::AddValueToParamDictValue(pdictvaluemorph, element.header.guid, "morph:", "max_y", Max_y);
		ParamHelpers::AddValueToParamDictValue(pdictvaluemorph, element.header.guid, "morph:", "min_y", Min_y);
		ParamHelpers::AddValueToParamDictValue(pdictvaluemorph, element.header.guid, "morph:", "max_z", Max_z);
		ParamHelpers::AddValueToParamDictValue(pdictvaluemorph, element.header.guid, "morph:", "min_z", Min_z);
		ParamHelpers::AddValueToParamDictValue(pdictvaluemorph, element.header.guid, "morph:", "a", A);
		ParamHelpers::AddValueToParamDictValue(pdictvaluemorph, element.header.guid, "morph:", "b", B);
		ParamHelpers::AddValueToParamDictValue(pdictvaluemorph, element.header.guid, "morph:", "zzyzx", ZZYZX);
		ParamHelpers::CompareParamDictValue(pdictvaluemorph, pdictvalue);
		ACAPI_DisposeElemMemoHdls(&memo);
		return true;
	}
	else {
		ACAPI_DisposeElemMemoHdls(&memo);
		return false;
	}
}

// -----------------------------------------------------------------------------
// Назначает флаги источника чтения по rawName параметра
// -----------------------------------------------------------------------------
void ParamHelpers::SetParamValueSourseByName(ParamValue & pvalue) {
	if (pvalue.rawName.Contains("{@coord:")) pvalue.fromCoord = true;
	if (pvalue.rawName.Contains("{@gdl")) pvalue.fromGDLparam = true;
	if (pvalue.rawName.Contains("{@description:")) {
		pvalue.fromGDLparam = true;
		pvalue.fromGDLdescription = true;
	}
	if (pvalue.rawName.Contains("{@property")) pvalue.fromProperty = true;
	if (pvalue.rawName.Contains("{@material")) pvalue.fromMaterial = true;
	if (pvalue.rawName.Contains("{@info")) pvalue.fromInfo = true;
	if (pvalue.rawName.Contains("{@ifc")) pvalue.fromIFCProperty = true;
	if (pvalue.rawName.Contains("{@morph")) pvalue.fromMorph = true;
	if (pvalue.rawName.Contains("{@id")) pvalue.fromID = true;
}

// -----------------------------------------------------------------------------
// Добавление пустого значения в словарь ParamDictValue
// -----------------------------------------------------------------------------
GS::UniString ParamHelpers::AddValueToParamDictValue(ParamDictValue & params, const GS::UniString & name) {
	if (name.IsEmpty()) return "";
	GS::UniString rawname_prefix = "";
	GS::UniString name_ = name.ToLowerCase();

	// Ищём строку с указанием формата вывода (метры/миллиметры)
	GS::UniString stringformat = GetFormatString(name_);
	name_ = GetPropertyENGName(name_).ToLowerCase();

	// Проверяем - есть ли указатель на тип параметра (GDL, Property, IFC)
	if (name_.Contains(":")) {
		GS::Array<GS::UniString> partstring;
		UInt32 n = StringSplt(name_, ":", partstring);
		if (n > 1) {
			rawname_prefix = partstring[0] + ":";
			name_ = partstring[1].ToLowerCase();
		}
	}
	if (rawname_prefix.IsEmpty()) rawname_prefix = "@gdl:";
	if (!rawname_prefix.Contains("@"))
	{
		rawname_prefix = "@" + rawname_prefix;
	}

	// Ищём строку с указанием формата вывода (метры/миллиметры)
	GS::UniString rawName = "{" + rawname_prefix + name_ + "}";
	if (!params.ContainsKey(rawName)) {
		ParamValue pvalue;
		pvalue.rawName = rawName;
		pvalue.name = name_;
		if (!stringformat.IsEmpty()) { // Если строка не пустая - посмотрим количество знаков после запятой
			Int32 n_zero = 3;
			Int32 krat = 0;
			double koeff = 1;
			bool trim_zero = true;
			PropertyHelpers::ParseFormatString(stringformat, n_zero, krat, koeff, trim_zero);
			UNUSED_VARIABLE(krat); UNUSED_VARIABLE(koeff); UNUSED_VARIABLE(trim_zero);
			pvalue.val.stringformat = stringformat;
			pvalue.val.n_zero = n_zero;
		}
		params.Add(rawName, pvalue);
		return rawName;
	}

	// TODO Провыерить - зачем сделано возвращение пустого значения при наличии имени в словаре?
	return rawName;
}

bool ParamHelpers::needAdd(ParamDictValue & params, GS::UniString & rawName) {
	bool addNew = false;
	if (rawName.Contains(CharENTER)) {
		UInt32 n = rawName.FindFirst(CharENTER);
		GS::UniString rawName_ = rawName.GetSubstring(0, n) + "}";
		addNew = params.ContainsKey(rawName_);
	}
	return addNew;
}

// --------------------------------------------------------------------
// Запись параметра ParamValue в словарь ParamDict, если его там прежде не было
// --------------------------------------------------------------------
void ParamHelpers::AddParamValue2ParamDict(const API_Guid & elemGuid, ParamValue & param, ParamDictValue & paramToRead) {
	GS::UniString rawName = param.rawName;
	if (!paramToRead.ContainsKey(rawName)) {
		if (param.fromGuid == APINULLGuid) param.fromGuid = elemGuid;
		paramToRead.Add(rawName, param);
	}
}

// --------------------------------------------------------------------
// Запись параметра ParamValue в словарь элементов ParamDictElement, если его там прежде не было
// --------------------------------------------------------------------
void ParamHelpers::AddParamValue2ParamDictElement(const ParamValue & param, ParamDictElement & paramToRead) {
	ParamHelpers::AddParamValue2ParamDictElement(param.fromGuid, param, paramToRead);
}

// --------------------------------------------------------------------
// Сопоставляет параметры
// --------------------------------------------------------------------
bool ParamHelpers::CompareParamValue(ParamValue & paramFrom, ParamValue & paramTo, GS::UniString stringformat) {
	if (!paramFrom.isValid) return false;
	if (paramTo.isValid || paramTo.fromProperty || paramTo.fromPropertyDefinition) {
		if (stringformat.IsEmpty()) stringformat = paramTo.val.stringformat;
		if (stringformat.IsEmpty()) stringformat = paramFrom.val.stringformat;

		// Приводим к единому виду перед проверкой
		if (!stringformat.IsEmpty()) {
			paramTo.val.stringformat = stringformat;
			paramFrom.val.stringformat = stringformat;
			ParamHelpers::ConvertByFormatString(paramTo);
			ParamHelpers::ConvertByFormatString(paramFrom);
		}

		//Сопоставляем и записываем, если значения отличаются
		if (paramFrom != paramTo) {
			paramTo.val = paramFrom.val; // Записываем только значения
			paramTo.isValid = true;
			return true;
		}
	}
	return false;
}

// --------------------------------------------------------------------
// Запись параметра ParamValue в словарь элементов ParamDictElement, если его там прежде не было
// --------------------------------------------------------------------
void ParamHelpers::AddParamValue2ParamDictElement(const API_Guid & elemGuid, const ParamValue & param, ParamDictElement & paramToRead) {
	GS::UniString rawName = param.rawName;
	if (paramToRead.ContainsKey(elemGuid)) {
		if (!paramToRead.Get(elemGuid).ContainsKey(rawName)) {
			paramToRead.Get(elemGuid).Add(rawName, param);
		}
		else {
			paramToRead.Get(elemGuid).Set(rawName, param);
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
void ParamHelpers::AddParamDictValue2ParamDictElement(const API_Guid & elemGuid, ParamDictValue & param, ParamDictElement & paramToRead) {
	if (paramToRead.ContainsKey(elemGuid)) {
		for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = param.EnumeratePairs(); cIt != NULL; ++cIt) {
			GS::UniString rawName = *cIt->key;
			if (!paramToRead.Get(elemGuid).ContainsKey(rawName)) {
				ParamValue& param = *cIt->value;
				if (param.fromGuid == APINULLGuid) param.fromGuid = elemGuid;
				paramToRead.Get(elemGuid).Add(rawName, param);
			}
		}
	}
	else {
		paramToRead.Add(elemGuid, param);
	}
}

// -----------------------------------------------------------------------------
// Добавление массива свойств в словарь
// -----------------------------------------------------------------------------
bool ParamHelpers::AddProperty(ParamDictValue & params, GS::Array<API_Property>&properties) {
	UInt32 nparams = params.GetSize();
	if (nparams < 1) return false;
	bool flag_find = false;
	for (UInt32 i = 0; i < properties.GetSize(); i++) {
		ParamValue pvalue;
		API_Property property = properties.Get(i);
		if (ParamHelpers::ConvertToParamValue(pvalue, property)) {
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
void ParamHelpers::AddValueToParamDictValue(ParamDictValue & params, const API_Guid & elemGuid, const GS::UniString & rawName_prefix, const GS::UniString & name, const double& val) {
	ParamValue pvalue;
	pvalue.rawName = "{@" + rawName_prefix + name.ToLowerCase() + "}";
	pvalue.name = name.ToLowerCase();
	ParamHelpers::ConvertToParamValue(pvalue, "", val);
	params.Add(pvalue.rawName, pvalue);
}

// -----------------------------------------------------------------------------
// Добавление значения в словарь ParamDictValue
// -----------------------------------------------------------------------------
void ParamHelpers::AddValueToParamDictValue(ParamDictValue & params, const API_Guid & elemGuid, const GS::UniString & rawName_prefix, const GS::UniString & name, const GS::UniString & val) {
	ParamValue pvalue;
	pvalue.rawName = "{@" + rawName_prefix + name.ToLowerCase() + "}";
	pvalue.name = name.ToLowerCase();
	ParamHelpers::ConvertToParamValue(pvalue, "", val);
	params.Add(pvalue.rawName, pvalue);
}

// -----------------------------------------------------------------------------
// Возвращает elemType и elemGuid для корректного чтение параметров элементов навесной стены
// -----------------------------------------------------------------------------
void GetGDLParametersHead(const API_Element & element, const API_Elem_Head & elem_head, API_ElemTypeID & elemType, API_Guid & elemGuid) {
#if defined AC_26 || defined AC_27
	switch (elem_head.type.typeID) {
#else
	switch (elem_head.typeID) {
#endif // AC_26
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
#if defined AC_26 || defined AC_27
		elemType = elem_head.type.typeID;
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
#if defined AC_26 || defined AC_27
	apiOwner.type.typeID = elemType;
#else
	apiOwner.typeID = elemType;
#endif
#ifdef AC_27
	err = ACAPI_LibraryPart_OpenParameters(&apiOwner);
#else
	err = ACAPI_Goodies(APIAny_OpenParametersID, &apiOwner, nullptr);
#endif
	if (err != NoError) {
		msg_rep("GetGDLParameters", "APIAny_OpenParametersID", err, elemGuid);
		return err;
	}
#ifdef AC_27
	err = ACAPI_LibraryPart_GetActParameters(&apiParams);
#else
	err = ACAPI_Goodies(APIAny_GetActParametersID, &apiParams);
#endif
	if (err != NoError) {
		msg_rep("GetGDLParameters", "APIAny_GetActParametersID", err, elemGuid);
#ifdef AC_27
		err = ACAPI_LibraryPart_CloseParameters();
#else
		err = ACAPI_Goodies(APIAny_CloseParametersID);
#endif
		if (err != NoError) msg_rep("GetGDLParameters", "APIAny_CloseParametersID", err, elemGuid);
		return err;
	}
	params = apiParams.params;
#ifdef AC_27
	err = ACAPI_LibraryPart_CloseParameters();
#else
	err = ACAPI_Goodies(APIAny_CloseParametersID);
#endif
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
	DBPrintf("== SMSTF ==      ReadElemCoords\n");
	double x = 0; double y = 0; double z = 0; double angz = 0;
	double sx = 0; double sy = 0;
	double dx = 0; double dy = 0;
	double ex = 0; double ey = 0;
	double tolerance_coord = 0.01;
	bool hasSymbpos = false; bool hasLine = false;
	bool isFliped = false;

	GS::UniString globnorthkey = "{@glob:glob_north_dir}";
	API_ElemTypeID eltype;
#if defined AC_26 || defined AC_27
	eltype = element.header.type.typeID;
#else
	eltype = element.header.typeID;
#endif
	API_Element owner;

	// Если нужно определить направление окон или дверей - запрашиваем родительский элемент
	if (params.ContainsKey(globnorthkey) && (eltype == API_WindowID || eltype == API_DoorID)) {
		BNZeroMemory(&owner, sizeof(API_Element));
		if (eltype == API_WindowID) owner.header.guid = element.window.owner;
		if (eltype == API_DoorID) owner.header.guid = element.door.owner;
		if (ACAPI_Element_Get(&owner) != NoError) return false;
#if defined AC_26 || defined AC_27
		if (owner.header.type.typeID == API_WallID)
#else
		if (owner.header.typeID == API_WallID)
#endif
		{
			sx = owner.wall.begC.x;
			sy = owner.wall.begC.y;
			ex = owner.wall.endC.x;
			ey = owner.wall.endC.y;
			isFliped = owner.wall.flipped;
			hasLine = true;
		}
	}
	else {
		UNUSED_VARIABLE(owner);
	}
	switch (eltype) {
	case API_WindowID:
		x = element.window.objLoc;
		y = 0;
		z = 0;
		hasSymbpos = true;
		break;
	case API_DoorID:
		x = element.door.objLoc;
		y = 0;
		z = 0;
		hasSymbpos = true;
		break;
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
		isFliped = element.wall.flipped;
		hasLine = true;
		break;
	case API_BeamID:
		sx = element.beam.begC.x;
		sy = element.beam.begC.y;
		ex = element.beam.endC.x;
		ey = element.beam.endC.y;
		isFliped = element.beam.isFlipped;
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
		ParamHelpers::AddValueToParamDictValue(pdictvaluecoord, element.header.guid, "coord:", "symb_pos_x", x);
		ParamHelpers::AddValueToParamDictValue(pdictvaluecoord, element.header.guid, "coord:", "symb_pos_y", y);
		ParamHelpers::AddValueToParamDictValue(pdictvaluecoord, element.header.guid, "coord:", "symb_pos_z", z);
		double symb_pos_x_correct = abs(abs(x * 1000.0) - floor(abs(x * 1000.0)));
		double symb_pos_y_correct = abs(abs(y * 1000.0) - floor(abs(y * 1000.0)));
		double symb_pos_correct = (symb_pos_x_correct < tolerance_coord&& symb_pos_y_correct < tolerance_coord);
		ParamHelpers::AddValueToParamDictValue(pdictvaluecoord, element.header.guid, "coord:", "symb_pos_x_correct", symb_pos_x_correct < tolerance_coord);
		ParamHelpers::AddValueToParamDictValue(pdictvaluecoord, element.header.guid, "coord:", "symb_pos_y_correct", symb_pos_y_correct < tolerance_coord);
		ParamHelpers::AddValueToParamDictValue(pdictvaluecoord, element.header.guid, "coord:", "symb_pos_correct", symb_pos_correct);
	}
	double tolerance_ang = 0.0000001;
	if (hasLine) {
		if (isFliped) {
			dx = ex - sx;
			dy = ey - sy;
		}
		else {
			dx = sx - ex;
			dy = sy - ey;
		}
		if (is_equal(dx, 0.0) && is_equal(dy, 0.0)) {
			angz = 0.0;
		}
		else {
			angz = atan2(dy, dx) + PI;
		}
		double k = 100000.0;
		sx = round(sx * k) / k;
		sy = round(sy * k) / k;
		ex = round(ex * k) / k;
		ey = round(ey * k) / k;
		ParamHelpers::AddValueToParamDictValue(pdictvaluecoord, element.header.guid, "coord:", "symb_pos_x", sx);
		ParamHelpers::AddValueToParamDictValue(pdictvaluecoord, element.header.guid, "coord:", "symb_pos_y", sy);
		ParamHelpers::AddValueToParamDictValue(pdictvaluecoord, element.header.guid, "coord:", "symb_pos_sx", sx);
		ParamHelpers::AddValueToParamDictValue(pdictvaluecoord, element.header.guid, "coord:", "symb_pos_sy", sy);
		ParamHelpers::AddValueToParamDictValue(pdictvaluecoord, element.header.guid, "coord:", "symb_pos_ex", ex);
		ParamHelpers::AddValueToParamDictValue(pdictvaluecoord, element.header.guid, "coord:", "symb_pos_ey", ey);
		double symb_pos_sx_correct = abs(abs(sx * 1000.0) - floor(abs(sx * 1000.0)));
		double symb_pos_sy_correct = abs(abs(sy * 1000.0) - floor(abs(sy * 1000.0)));
		double symb_pos_ex_correct = abs(abs(ex * 1000.0) - floor(abs(ex * 1000.0)));
		double symb_pos_ey_correct = abs(abs(ey * 1000.0) - floor(abs(ey * 1000.0)));
		double symb_pos_correct = (symb_pos_sx_correct < tolerance_coord&& symb_pos_sy_correct < tolerance_coord&& symb_pos_ex_correct < tolerance_coord&& symb_pos_ey_correct < tolerance_coord);
		ParamHelpers::AddValueToParamDictValue(pdictvaluecoord, element.header.guid, "coord:", "symb_pos_x_correct", symb_pos_sx_correct < tolerance_coord);
		ParamHelpers::AddValueToParamDictValue(pdictvaluecoord, element.header.guid, "coord:", "symb_pos_y_correct", symb_pos_sy_correct < tolerance_coord);
		ParamHelpers::AddValueToParamDictValue(pdictvaluecoord, element.header.guid, "coord:", "symb_pos_sx_correct", symb_pos_sx_correct < tolerance_coord);
		ParamHelpers::AddValueToParamDictValue(pdictvaluecoord, element.header.guid, "coord:", "symb_pos_sy_correct", symb_pos_sy_correct < tolerance_coord);
		ParamHelpers::AddValueToParamDictValue(pdictvaluecoord, element.header.guid, "coord:", "symb_pos_ex_correct", symb_pos_ex_correct < tolerance_coord);
		ParamHelpers::AddValueToParamDictValue(pdictvaluecoord, element.header.guid, "coord:", "symb_pos_ey_correct", symb_pos_ey_correct < tolerance_coord);
		ParamHelpers::AddValueToParamDictValue(pdictvaluecoord, element.header.guid, "coord:", "symb_pos_correct", symb_pos_correct);
		double l = sqr(dx * dx + dy * dy);
		double l_correct = abs(abs(l * 200.0) - floor(abs(l * 200.0)));
		ParamHelpers::AddValueToParamDictValue(pdictvaluecoord, element.header.guid, "coord:", "l_correct", l_correct);
	}
	double k = 100000.0;
	if (abs(angz) > 0.0000001) {
		angz = fmod(round((angz * 180.0 / PI) * k) / k, 360.0);
	}
	else {
		angz = 0.0;
	}
	if (params.ContainsKey(globnorthkey)) {
		double north = params.Get(globnorthkey).val.doubleValue;
		double angznorth = fmod(angz - north + 90.0, 360.0);
		if (abs(angznorth) < 0.0000001) {
			angznorth = 0.0;
		}
		else {
			if (angznorth < 0.0) angznorth = 360.0 + angznorth;
			angznorth = round(angznorth * k) / k;
		}
		ParamHelpers::AddValueToParamDictValue(pdictvaluecoord, element.header.guid, "coord:", "north_dir", angznorth);
		GS::UniString angznorthtxt = "";
		double n = 0.0;		//"С"
		double nw = 45.0;	//"СЗ"
		double w = 90.0;	//"З"
		double sw = 135.0;	//"ЮЗ"
		double s = 180.0;	//"Ю"
		double se = 225.0;	//"ЮВ"
		double e = 270.0;	//"В";
		double ne = 315.0;	//"СB";
		double nn = 360.0;

		//if (angznorth > nn - 22.5 || angznorth < n + 22.5) angznorthtxt = "N";
		//if (angznorth > ne - 22.5 && angznorth < ne + 22.5) angznorthtxt = "NE";
		//if (angznorth > e - 22.5 && angznorth < e + 22.5) angznorthtxt = "E";
		//if (angznorth > se - 22.5 && angznorth < se + 22.5) angznorthtxt = "SE";
		//if (angznorth > s - 22.5 && angznorth < s + 22.5) angznorthtxt = "S";
		//if (angznorth > sw - 22.5 && angznorth < sw + 22.5) angznorthtxt = "SW";
		//if (angznorth > w - 22.5 && angznorth < w + 22.5) angznorthtxt = "W";
		//if (angznorth > nw - 22.5 && angznorth < nw + 22.5) angznorthtxt = "NW";

		if (angznorth > nn - 22.5 || angznorth < n + 22.5) angznorthtxt = RSGetIndString(AddOnStringsID, N_StringID, ACAPI_GetOwnResModule());
		if (angznorth > ne - 22.5 && angznorth < ne + 22.5) angznorthtxt = RSGetIndString(AddOnStringsID, NE_StringID, ACAPI_GetOwnResModule());
		if (angznorth > e - 22.5 && angznorth < e + 22.5) angznorthtxt = RSGetIndString(AddOnStringsID, E_StringID, ACAPI_GetOwnResModule());
		if (angznorth > se - 22.5 && angznorth < se + 22.5) angznorthtxt = RSGetIndString(AddOnStringsID, SE_StringID, ACAPI_GetOwnResModule());
		if (angznorth > s - 22.5 && angznorth < s + 22.5) angznorthtxt = RSGetIndString(AddOnStringsID, S_StringID, ACAPI_GetOwnResModule());
		if (angznorth > sw - 22.5 && angznorth < sw + 22.5) angznorthtxt = RSGetIndString(AddOnStringsID, SW_StringID, ACAPI_GetOwnResModule());
		if (angznorth > w - 22.5 && angznorth < w + 22.5) angznorthtxt = RSGetIndString(AddOnStringsID, W_StringID, ACAPI_GetOwnResModule());
		if (angznorth > nw - 22.5 && angznorth < nw + 22.5) angznorthtxt = RSGetIndString(AddOnStringsID, NW_StringID, ACAPI_GetOwnResModule());
		if (is_equal(angznorth, n + 22.5)) angznorthtxt = RSGetIndString(AddOnStringsID, N_StringID, ACAPI_GetOwnResModule());
		if (is_equal(angznorth, ne + 22.5)) angznorthtxt = RSGetIndString(AddOnStringsID, NE_StringID, ACAPI_GetOwnResModule());
		if (is_equal(angznorth, e + 22.5)) angznorthtxt = RSGetIndString(AddOnStringsID, E_StringID, ACAPI_GetOwnResModule());
		if (is_equal(angznorth, se + 22.5)) angznorthtxt = RSGetIndString(AddOnStringsID, SE_StringID, ACAPI_GetOwnResModule());
		if (is_equal(angznorth, s + 22.5)) angznorthtxt = RSGetIndString(AddOnStringsID, S_StringID, ACAPI_GetOwnResModule());
		if (is_equal(angznorth, sw + 22.5)) angznorthtxt = RSGetIndString(AddOnStringsID, SW_StringID, ACAPI_GetOwnResModule());
		if (is_equal(angznorth, w + 22.5)) angznorthtxt = RSGetIndString(AddOnStringsID, W_StringID, ACAPI_GetOwnResModule());
		if (is_equal(angznorth, nn - 22.5)) angznorthtxt = RSGetIndString(AddOnStringsID, NW_StringID, ACAPI_GetOwnResModule());
		ParamHelpers::AddValueToParamDictValue(pdictvaluecoord, element.header.guid, "coord:", "north_dir_str", angznorthtxt);
	}
	double symb_rotangle_fraction = abs(abs(angz) - floor(abs(angz))) * 10000;
	ParamHelpers::AddValueToParamDictValue(pdictvaluecoord, element.header.guid, "coord:", "symb_rotangle", angz);
	ParamHelpers::AddValueToParamDictValue(pdictvaluecoord, element.header.guid, "coord:", "symb_rotangle_fraction", symb_rotangle_fraction);
	ParamHelpers::AddValueToParamDictValue(pdictvaluecoord, element.header.guid, "coord:", "symb_rotangle_correct", symb_rotangle_fraction < 0.1);
	ParamHelpers::AddValueToParamDictValue(pdictvaluecoord, element.header.guid, "coord:", "symb_rotangle_mod5", fmod(angz, 5.0));
	ParamHelpers::AddValueToParamDictValue(pdictvaluecoord, element.header.guid, "coord:", "symb_rotangle_mod10", fmod(angz, 10.0));
	ParamHelpers::AddValueToParamDictValue(pdictvaluecoord, element.header.guid, "coord:", "symb_rotangle_mod45", fmod(angz, 45.0));
	ParamHelpers::AddValueToParamDictValue(pdictvaluecoord, element.header.guid, "coord:", "symb_rotangle_mod90", fmod(angz, 90.0));
	ParamHelpers::AddValueToParamDictValue(pdictvaluecoord, element.header.guid, "coord:", "symb_rotangle_mod180", fmod(angz, 180.0));
	ParamHelpers::CompareParamDictValue(pdictvaluecoord, params);
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
	GS::UniString meterString = RSGetIndString(AddOnStringsID, MeterStringID, ACAPI_GetOwnResModule());
	if (n > 1) {
		formatstring = partstring[n - 1];
		if (partstring[n - 1].Contains('m') || partstring[n - 1].Contains(meterString)) {
			formatstring = partstring[n - 1];
			paramName.ReplaceAll('.' + formatstring, "");
			formatstring.ReplaceAll(meterString, "m");
			meterString = RSGetIndString(AddOnStringsID, DMeterStringID, ACAPI_GetOwnResModule());
			formatstring.ReplaceAll(meterString, "d");
			meterString = RSGetIndString(AddOnStringsID, CMeterStringID, ACAPI_GetOwnResModule());
			formatstring.ReplaceAll(meterString, "c");
		}
	}
	return formatstring;
}

FormatStringDict GetFotmatStringForMeasureType() {
	FormatStringDict fdict = {};

	// Получаем данные об округлении и типе расчёта
	API_CalcUnitPrefs unitPrefs1;
#ifdef AC_27
	ACAPI_ProjectSetting_GetPreferences(&unitPrefs1, APIPrefs_CalcUnitsID);
#else
	ACAPI_Environment(APIEnv_GetPreferencesID, &unitPrefs1, (void*)APIPrefs_CalcUnitsID);
#endif
	API_WorkingUnitPrefs unitPrefs;

#ifdef AC_27
	ACAPI_ProjectSetting_GetPreferences(&unitPrefs, APIPrefs_WorkingUnitsID);
#else
	ACAPI_Environment(APIEnv_GetPreferencesID, &unitPrefs, (void*)APIPrefs_WorkingUnitsID);
#endif
	FormatString fstring = {};
	fstring.needRound = unitPrefs1.useDisplayedValues;

	fstring.n_zero = 2; fstring.stringformat = "2";
	fdict.Add(API_PropertyUndefinedMeasureType, fstring);

	fstring.n_zero = 2; fstring.stringformat = "2";
	fdict.Add(API_PropertyDefaultMeasureType, fstring);

	fstring.n_zero = unitPrefs.areaDecimals; fstring.stringformat = GS::UniString::Printf("0%d", unitPrefs.areaDecimals);
	fdict.Add(API_PropertyAreaMeasureType, fstring);

	fstring.n_zero = unitPrefs.lenDecimals; fstring.stringformat = GS::UniString::Printf("0%dmm", unitPrefs.lenDecimals);
	fdict.Add(API_PropertyLengthMeasureType, fstring);

	fstring.n_zero = unitPrefs.volumeDecimals; fstring.stringformat = GS::UniString::Printf("0%d", unitPrefs.volumeDecimals);
	fdict.Add(API_PropertyVolumeMeasureType, fstring);

	fstring.n_zero = unitPrefs.angleDecimals; fstring.stringformat = GS::UniString::Printf("0%d", unitPrefs.angleDecimals);
	fdict.Add(API_PropertyAngleMeasureType, fstring);
	return fdict;
}

// -----------------------------------------------------------------------------
// Получение имени внутренних свойств по русскому имени
// -----------------------------------------------------------------------------
GS::UniString GetPropertyENGName(GS::UniString & name) {
	if (!name.Contains("@property:")) return name;
	if (name.IsEqual("@property:id")) return "@property:BuildingMaterialProperties/Building Material ID";
	if (name.IsEqual("@property:n")) return "@material:n";
	if (name.IsEqual("@property:layer_thickness")) return "@material:layer thickness";
	if (name.IsEqual("@property:bmat_inx")) return "@material:bmat_inx";
	GS::UniString nameproperty = "";
	nameproperty = "@property:" + RSGetIndString(AddOnStringsID, BuildingMaterialNameID, ACAPI_GetOwnResModule());
	if (name.IsEqual(nameproperty)) return "@property:BuildingMaterialProperties/Building Material Name";

	nameproperty = "@property:" + RSGetIndString(AddOnStringsID, BuildingMaterialDescriptionID, ACAPI_GetOwnResModule());
	if (name.IsEqual(nameproperty)) return "@property:BuildingMaterialProperties/Building Material Description";

	nameproperty = "@property:" + RSGetIndString(AddOnStringsID, BuildingMaterialDensityID, ACAPI_GetOwnResModule());
	if (name.IsEqual(nameproperty)) return "@property:BuildingMaterialProperties/Building Material Density";

	nameproperty = "@property:" + RSGetIndString(AddOnStringsID, BuildingMaterialManufacturerID, ACAPI_GetOwnResModule());
	if (name.IsEqual(nameproperty)) return "@property:BuildingMaterialProperties/Building Material Manufacturer";

	nameproperty = "@property:" + RSGetIndString(AddOnStringsID, BuildingMaterialCutFillID, ACAPI_GetOwnResModule());
	if (name.IsEqual(nameproperty)) return "@property:BuildingMaterialProperties/Building Material CutFill";

	nameproperty = "@property:" + RSGetIndString(AddOnStringsID, ThicknessID, ACAPI_GetOwnResModule());
	if (name.IsEqual(nameproperty)) return "@material:layer thickness";
	return name;
}

// -----------------------------------------------------------------------------
// Извлекает из строки все имена свойств или параметров, заключенные в знаки %
// -----------------------------------------------------------------------------
bool ParamHelpers::ParseParamNameMaterial(GS::UniString & expression, ParamDictValue & paramDict) {
	GS::UniString part = "";
	while (expression.Count('%') > 1) {
		part = expression.GetSubstring('%', '%', 0);
		if (!part.IsEmpty()) expression.ReplaceAll('%' + part + '%', "{@property:" + part.ToLowerCase() + '}');
	}
	return ParamHelpers::ParseParamName(expression, paramDict);
}

// -----------------------------------------------------------------------------
// Извлекает из строки все имена свойств или параметров, заключенные в знаки { }
// -----------------------------------------------------------------------------
bool ParamHelpers::ParseParamName(GS::UniString & expression, ParamDictValue & paramDict) {
	GS::UniString tempstring = expression;
	if (!tempstring.Contains('{')) return false;
	GS::UniString part = "";
	while (tempstring.Contains('{') && tempstring.Contains('}')) {
		part = tempstring.GetSubstring('{', '}', 0);

		// TODO Переписать всю эту хреноту - отделить парсинг от добавления в словарь
		GS::UniString part_ = ParamHelpers::AddValueToParamDictValue(paramDict, part);
		expression.ReplaceAll('{' + part + '}', part_);
		tempstring.ReplaceAll('{' + part + '}', "");
	}
	return true;
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
			if (pvalue.isValid) {
				val = ParamHelpers::ToString(pvalue);
				flag_find = true;
			}
		}
		else {
			if (pdictvalue.ContainsKey(partc)) {
				ParamValue pvalue = pdictvalue.Get(partc);
				if (pvalue.isValid) {
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
		std::string expression_string(part.ToCStr(0, MaxUSize, GChCode).Get());
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
	paramTypesList.Push("{@property:");
	paramTypesList.Push("{@coord:");
	paramTypesList.Push("{@gdl:");
	paramTypesList.Push("{@info:");
	paramTypesList.Push("{@ifc:");
	paramTypesList.Push("{@morph:");
	paramTypesList.Push("{@material:");
	paramTypesList.Push("{@glob:");
	paramTypesList.Push("{@id:");
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
#ifdef AC_27
	ACAPI_MenuItem_GetMenuItemFlags(&itemRef, &itemFlags);
#else
	ACAPI_Interface(APIIo_GetMenuItemFlagsID, &itemRef, &itemFlags);
#endif
	if ((itemFlags & API_MenuItemChecked) == 0)
		itemFlags |= API_MenuItemChecked;
	else
		itemFlags &= ~API_MenuItemChecked;

#ifdef AC_27
	ACAPI_MenuItem_SetMenuItemFlags(&itemRef, &itemFlags);
#else
	ACAPI_Interface(APIIo_SetMenuItemFlagsID, &itemRef, &itemFlags);
#endif
	return (bool)((itemFlags & API_MenuItemChecked) != 0);
}

void PropertyHelpers::ParseFormatString(const GS::UniString & stringformat, Int32 & n_zero, Int32 & krat, double& koeff, bool& trim_zero) {
	n_zero = 3;
	krat = 0;
	koeff = 1;
	trim_zero = true;
	GS::UniString outstringformat = stringformat;
	if (!stringformat.IsEmpty()) {
		if (stringformat.Contains("mm")) {
			n_zero = 0;
			koeff = 1000;
			outstringformat.ReplaceAll("mm", "");
		}
		if (stringformat.Contains("cm")) {
			n_zero = 1;
			koeff = 100;
			outstringformat.ReplaceAll("cm", "");
		}
		if (stringformat.Contains("dm")) {
			n_zero = 2;
			koeff = 10;
			outstringformat.ReplaceAll("dm", "");
		}
		if (stringformat.Contains("gm")) {
			koeff = 1 / 100;
			outstringformat.ReplaceAll("gm", "");
		}
		if (stringformat.Contains("km")) {
			koeff = 1 / 1000;
			outstringformat.ReplaceAll("km", "");
		}
		if (outstringformat.Contains("m")) {
			n_zero = 3;
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
}

// TODO Придумать более изящную обработку округления
GS::UniString PropertyHelpers::NumToString(const double& var, const GS::UniString & stringformat) {
	if (abs(var) < 0.00000001) return "0";
	GS::UniString out = "";
	Int32 n_zero = 3;
	Int32 krat = 0;
	double koeff = 1;
	bool trim_zero = true;
	PropertyHelpers::ParseFormatString(stringformat, n_zero, krat, koeff, trim_zero);
	double outvar = var * koeff;
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

GS::UniString PropertyHelpers::ToString(const API_Variant & variant, const GS::UniString & stringformat) {
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

GS::UniString PropertyHelpers::ToString(const API_Property & property, const GS::UniString & stringformat) {
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
#if defined(AC_25) || defined(AC_26) || defined(AC_27)
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
#if defined(AC_25) || defined(AC_26) || defined(AC_27)
		GS::Array<API_SingleEnumerationVariant> possibleEnumValues = property.definition.possibleEnumValues;
		UInt32 qty_finded_values = value->listVariant.variants.GetSize();
		for (UInt32 i = 0; i < possibleEnumValues.GetSize(); i++) {
			API_Guid guidValue = possibleEnumValues[i].keyVariant.guidValue;
			for (UInt32 j = 0; j < value->listVariant.variants.GetSize(); j++) {
				if (value->listVariant.variants[j].guidValue == guidValue) {
					string += ToString(possibleEnumValues[i].displayVariant, stringformat);
					qty_finded_values = qty_finded_values - 1;
					if (qty_finded_values != 0) string += "; ";
					break;
				}
			}
			if (qty_finded_values == 0) break;
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
	switch (rhs.val.type) {
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

#if !defined(AC_25) && !defined(AC_26) && !defined(AC_27)
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
#if defined(AC_25) || defined(AC_26) || defined(AC_27)
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
#ifdef AC_27
		err = ACAPI_UserData_DeleteUserData(&tElemHead);
#else
		err = ACAPI_Element_DeleteUserData(&tElemHead);
#endif
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
	GSErrCode			err;
#ifdef AC_27
	UInt32 count, i;
	err = ACAPI_Attribute_GetNum(API_LayerID, count);
#else
	API_AttributeIndex count, i;
	err = ACAPI_Attribute_GetNum(API_LayerID, &count);
#endif
	if (err != NoError) msg_rep("UnhideUnlockAllLayer", "ACAPI_Attribute_GetNum", err, APINULLGuid);
	if (err == NoError) {
		for (i = 2; i <= count; i++) {
			BNZeroMemory(&attrib, sizeof(API_Attribute));
			attrib.header.typeID = API_LayerID;
#ifdef AC_27
			attrib.header.index = ACAPI_CreateAttributeIndex(i);
#else
			attrib.header.index = i;
#endif
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
bool ParamHelpers::ConvertToProperty(ParamValue & pvalue) {
	if (!pvalue.isValid) return false;
	if (!pvalue.fromPropertyDefinition) return false;
	API_Property property = pvalue.property;
	if (ParamHelpers::ConvertToProperty(pvalue, property)) {
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
bool ParamHelpers::ConvertToProperty(const ParamValue & pvalue, API_Property & property) {
	if (!property.definition.canValueBeEditable) {
		DBPrintf("== SMSTF == ParamHelpers::ConvertToProperty !property.definition.canValueBeEditable\n");
		return false;
	}
	if (!pvalue.isValid) {
		DBPrintf("== SMSTF == ParamHelpers::ConvertToProperty !pvalue.isValid\n");
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

		// Конвертация угла из радиан в градусы
		if (property.definition.measureType == API_PropertyAngleMeasureType) {
			if (!is_equal(pvalue.val.doubleValue * 180 / PI, property.value.singleVariant.variant.doubleValue)) {
				property.value.singleVariant.variant.doubleValue = pvalue.val.doubleValue * PI / 180;
				flag_rec = true;
			}
		}
		else {
			if (!is_equal(property.value.singleVariant.variant.doubleValue, pvalue.val.doubleValue)) {
				property.value.singleVariant.variant.doubleValue = pvalue.val.doubleValue;
				flag_rec = true;
			}
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
bool GetElemState(const API_Guid & elemGuid, const GS::Array<API_PropertyDefinition>&definitions, GS::UniString property_flag_name) {
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
	DBPrintf("== SMSTF == ElementsWrite start\n");
	for (GS::HashTable<API_Guid, ParamDictValue>::PairIterator cIt = paramToWrite.EnumeratePairs(); cIt != NULL; ++cIt) {
		ParamDictValue& params = *cIt->value;
		API_Guid elemGuid = *cIt->key;
		if (!params.IsEmpty()) {
			ParamHelpers::Write(elemGuid, params);
		}
	}
	DBPrintf("== SMSTF == ElementsWrite end\n");
}

// --------------------------------------------------------------------
// Запись ParamDictValue в один элемент
// --------------------------------------------------------------------
void ParamHelpers::Write(const API_Guid & elemGuid, ParamDictValue & params) {
	if (params.IsEmpty()) return;
	if (elemGuid == APINULLGuid) return;

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
			if (paramType.IsEqual("{@property:")) {
				ParamHelpers::WritePropertyValues(elemGuid, paramByType);
			}
			if (paramType.IsEqual("{@gdl:")) {
				ParamHelpers::WriteGDLValues(elemGuid, paramByType);
			}
			if (paramType.IsEqual("{@id:")) {
				ParamHelpers::WriteIDValues(elemGuid, paramByType);
			}
		}
	}
}

// --------------------------------------------------------------------
// Запись ParamDictValue в автотекст
// --------------------------------------------------------------------
void ParamHelpers::InfoWrite(ParamDictElement & paramToWrite) {
	if (paramToWrite.IsEmpty()) return;
	DBPrintf("== SMSTF == InfoWrite start\n");
	ParamDictValue paramsinfo;
	for (GS::HashTable<API_Guid, ParamDictValue>::PairIterator cIt = paramToWrite.EnumeratePairs(); cIt != NULL; ++cIt) {
		ParamDictValue& params = *cIt->value;
		if (!params.IsEmpty()) {
			for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = params.EnumeratePairs(); cIt != NULL; ++cIt) {
				ParamValue& param = *cIt->value;
				if (param.fromInfo && !paramsinfo.ContainsKey(param.rawName)) paramsinfo.Add(param.rawName, param);
			}
		}
	}
	if (paramsinfo.IsEmpty()) return;
	for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = paramsinfo.EnumeratePairs(); cIt != NULL; ++cIt) {
		ParamValue& param = *cIt->value;
		GS::UniString dbKey = param.name;
		GS::UniString value = ParamHelpers::ToString(param, param.val.stringformat);
		GSErrCode err = NoError;
#ifdef AC_27
		err = ACAPI_AutoText_SetAnAutoText(&dbKey, &value);
#else
		err = ACAPI_Goodies(APIAny_SetAnAutoTextID, &dbKey, &value);
#endif
		if (err != NoError) msg_rep("InfoWrite", "APIAny_SetAnAutoTextID", err, APINULLGuid);
	}
	msg_rep("InfoWrite", "write", NoError, APINULLGuid);
}

// --------------------------------------------------------------------
// Запись ParamDictValue в ID
// --------------------------------------------------------------------
void ParamHelpers::WriteIDValues(const API_Guid & elemGuid, ParamDictValue & params) {
	if (params.IsEmpty()) return;
	if (elemGuid == APINULLGuid) return;
	if (!params.ContainsKey("{@id:id}")) return;
	DBPrintf("== SMSTF ==      WriteIDValues\n");
	GS::UniString val = ParamHelpers::ToString(params.Get("{@id:id}"));
	GSErrCode err = NoError;
#ifdef AC_27
	err = ACAPI_Element_ChangeElementInfoString(&elemGuid, &val);
#else
	err = ACAPI_Database(APIDb_ChangeElementInfoStringID, (void*)&elemGuid, (void*)&val);
#endif
	if (err != NoError) msg_rep("WriteGDLValues - ID", "ACAPI_Database(APIDb_ChangeElementInfoStringID", err, elemGuid);
}

// --------------------------------------------------------------------
// Запись ParamDictValue в GDL параметры
// --------------------------------------------------------------------
void ParamHelpers::WriteGDLValues(const API_Guid & elemGuid, ParamDictValue & params) {
	if (params.IsEmpty()) return;
	if (elemGuid == APINULLGuid) return;
	DBPrintf("== SMSTF ==      WriteGDLValues\n");
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
#if defined AC_26 || defined AC_27
	eltype = elem_head.type.typeID;
#else
	eltype = elem_head.typeID;
#endif
	GetGDLParametersHead(element, elem_head, elemType, elemGuidt);
	BNZeroMemory(&apiOwner, sizeof(API_ParamOwnerType));
	BNZeroMemory(&apiParams, sizeof(API_GetParamsType));
	apiOwner.guid = elemGuidt;
#if defined AC_26 || defined AC_27
	apiOwner.type.typeID = elemType;
#else
	apiOwner.typeID = elemType;
#endif
#ifdef AC_27
	err = ACAPI_LibraryPart_OpenParameters(&apiOwner);
#else
	err = ACAPI_Goodies(APIAny_OpenParametersID, &apiOwner, nullptr);
#endif
	if (err != NoError) {
		msg_rep("ParamHelpers::WriteGDLValues", "APIAny_OpenParametersID", err, elem_head.guid);
		return;
	}
#ifdef AC_27
	err = ACAPI_LibraryPart_GetActParameters(&apiParams);
#else
	err = ACAPI_Goodies(APIAny_GetActParametersID, &apiParams);
#endif
	if (err != NoError) {
		msg_rep("ParamHelpers::WriteGDLValues", "APIAny_GetActParametersID", err, elem_head.guid);
#ifdef AC_27
		err = ACAPI_LibraryPart_CloseParameters();
#else
		err = ACAPI_Goodies(APIAny_CloseParametersID);
#endif
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
		GS::UniString rawname = "{@gdl:" + name.ToLowerCase() + "}";
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
#ifdef AC_27
			err = ACAPI_LibraryPart_ChangeAParameter(&chgParam);
#else
			err = ACAPI_Goodies(APIAny_ChangeAParameterID, &chgParam, nullptr);
#endif
			if (err != NoError) {
				msg_rep("ParamHelpers::WriteGDLValues", "APIAny_ChangeAParameterID", err, elem_head.guid);
				return;
			}
		}
	}
#ifdef AC_27
	err = ACAPI_LibraryPart_GetActParameters(&apiParams);
#else
	err = ACAPI_Goodies(APIAny_GetActParametersID, &apiParams);
#endif
	if (err != NoError) {
		msg_rep("ParamHelpers::WriteGDLValues", "APIAny_GetActParametersID", err, elem_head.guid);
		return;
	}
#ifdef AC_27
	err = ACAPI_LibraryPart_CloseParameters();
#else
	err = ACAPI_Goodies(APIAny_CloseParametersID);
#endif
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
	DBPrintf("== SMSTF ==      WritePropertyValues\n");
	GS::Array<API_Property> properties;
	for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = params.EnumeratePairs(); cIt != NULL; ++cIt) {
		ParamValue& param = *cIt->value;
		if (param.isValid && param.fromPropertyDefinition) {
			API_Property property = param.property;

			// TODO выяснить - что быстрее - пакетная запись или запись по-отдельности
			//if (ParamHelpers::ConvertToProperty(param, property)) properties.Push(property);
			if (ParamHelpers::ConvertToProperty(param, property)) {
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
				if (param.fromProperty && !param.fromPropertyDefinition && !param.fromAttribDefinition) {
					return true;
				}
			}
		}
	}
	return false;
}

bool ParamHelpers::hasUnreadInfo(ParamDictElement & paramToRead, ParamDictValue & propertyParams) {
	for (GS::HashTable<API_Guid, ParamDictValue>::PairIterator cIt = paramToRead.EnumeratePairs(); cIt != NULL; ++cIt) {
		ParamDictValue& params = *cIt->value;
		if (!params.IsEmpty()) {
			for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = params.EnumeratePairs(); cIt != NULL; ++cIt) {
				ParamValue& param = *cIt->value;
				if (param.fromInfo) {

					// Проверим - есть ли уже считанный параметр
					if (propertyParams.ContainsKey(param.rawName)) {
						if (!propertyParams.Get(param.rawName).isValid) {
							return true; // Прочитан криво, перечитываем
						}
					}
					else {
						return true; // В списке общих параметров не найден, перечитаем
					}
				}
			}
		}
	}
	return false;
}

bool ParamHelpers::hasGlob(ParamDictValue & propertyParams) {
	if (propertyParams.IsEmpty()) return false;
	if (!propertyParams.ContainsKey("has_Glob")) return false;
	return true;
}

bool ParamHelpers::hasInfo(ParamDictValue & propertyParams) {
	if (propertyParams.IsEmpty()) return false;
	if (!propertyParams.ContainsKey("has_Info")) return false;
	return true;
}

bool ParamHelpers::hasProperyDefinitoin(ParamDictValue & propertyParams) {
	if (propertyParams.IsEmpty()) return false;
	if (!propertyParams.ContainsKey("has_ProperyDefinitoin")) return false;
	return true;
}

bool ParamHelpers::hasUnreadGlob(ParamDictElement & paramToRead, ParamDictValue & propertyParams) {
	for (GS::HashTable<API_Guid, ParamDictValue>::PairIterator cIt = paramToRead.EnumeratePairs(); cIt != NULL; ++cIt) {
		ParamDictValue& params = *cIt->value;
		if (!params.IsEmpty()) {
			for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = params.EnumeratePairs(); cIt != NULL; ++cIt) {
				ParamValue& param = *cIt->value;
				if (param.fromGlob) {

					// Проверим - есть ли уже считанный параметр
					if (propertyParams.ContainsKey(param.rawName)) {
						if (!propertyParams.Get(param.rawName).isValid) {
							return true; // Прочитан криво, перечитываем
						}
					}
					else {
						return true; // В списке общих параметров не найден, перечитаем
					}
				}
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
	DBPrintf("== SMSTF == ElementsRead start\n");
	if (ParamHelpers::hasUnreadInfo(paramToRead, propertyParams) && !ParamHelpers::hasInfo(propertyParams)) ParamHelpers::GetAllInfoToParamDict(propertyParams);
	if (ParamHelpers::hasUnreadGlob(paramToRead, propertyParams) && !ParamHelpers::hasGlob(propertyParams)) ParamHelpers::GetAllGlobToParamDict(propertyParams);
	if (ParamHelpers::hasUnreadProperyDefinitoin(paramToRead) && !ParamHelpers::hasProperyDefinitoin(propertyParams)) ParamHelpers::AllPropertyDefinitionToParamDict(propertyParams);

	// Выбираем по-элементно параметры для чтения
	for (GS::HashTable<API_Guid, ParamDictValue>::PairIterator cIt = paramToRead.EnumeratePairs(); cIt != NULL; ++cIt) {
		ParamDictValue& params = *cIt->value;
		API_Guid elemGuid = *cIt->key;
		if (!params.IsEmpty()) {
			if (!propertyParams.IsEmpty()) ParamHelpers::CompareParamDictValue(propertyParams, params); // Сопоставляем свойства
			ParamHelpers::Read(elemGuid, params, propertyParams);
		}
	}
	DBPrintf("== SMSTF == ElementsRead end\n");
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
#if defined AC_26 || defined AC_27
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
		AllPropertyDefinitionToParamDict(params, elemGuid);
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
		GS::UniString paramType = paramTypesList.Get(i);
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
			if (paramType.IsEqual("{@property:")) {
				needCompare = ParamHelpers::ReadPropertyValues(elemGuid, paramByType);
			}
			if (paramType.IsEqual("{@coord:")) {

				// Для определения угла к северу нам потребуется значение направления на север.
				// Оно должно быть в Info, проверим и добавим, если оно есть
				GS::UniString globnorthkey = "{@glob:glob_north_dir}";
				if (propertyParams.ContainsKey(globnorthkey)) {
					paramByType.Add(globnorthkey, propertyParams.Get(globnorthkey));
				}
				needCompare = ParamHelpers::ReadElemCoords(element, paramByType);
			}
			if (paramType.IsEqual("{@gdl:")) {
				needCompare = ParamHelpers::ReadGDLValues(element, elem_head, paramByType);
			}
			if (paramType.IsEqual("{@ifc:")) {
				needCompare = ParamHelpers::ReadIFCValues(elemGuid, paramByType);
			}
			if (paramType.IsEqual("{@morph:")) {
				needCompare = ParamHelpers::ReadMorphParam(element, paramByType);
			}
			if (paramType.IsEqual("{@id:")) {
				needCompare = ParamHelpers::ReadIDValues(elem_head, paramByType);
			}
			if (paramType.IsEqual("{@material:")) {
				if (ParamHelpers::ReadMaterial(element, params, propertyParams)) DBPrintf("== SMSTF ==          CompareParamDictValue\n");
				needCompare = false;
			}
			if (needCompare) {
				DBPrintf("== SMSTF ==          CompareParamDictValue\n");
				ParamHelpers::CompareParamDictValue(paramByType, params);
			}
			else {
				if (!paramType.IsEqual("{@material:")) {
					DBPrintf("== SMSTF ERR ==          Not found\n");
				}
			}
		}
	}
	DBPrintf("== SMSTF ==      ConvertByFormatString\n");
	for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = params.EnumeratePairs(); cIt != NULL; ++cIt) {
		ParamValue& param = *cIt->value;
		if (param.isValid && param.val.canCalculate) {
			ParamHelpers::ConvertByFormatString(param);
		}
	}
}

void ParamHelpers::GetAllInfoToParamDict(ParamDictValue & propertyParams) {
	GS::Array<GS::ArrayFB<GS::UniString, 3> >	autotexts;
	API_AutotextType	type = APIAutoText_Custom;
	DBPrintf("== SMSTF == GetAllInfoToParamDict start\n");
	GSErrCode err = NoError;
#ifdef AC_27
	err = ACAPI_AutoText_GetAutoTexts(&autotexts, type);
#else
	err = ACAPI_Goodies(APIAny_GetAutoTextsID, &autotexts, (void*)(GS::IntPtr)type);
#endif
	if (err != NoError) {
		msg_rep("GetAllInfoToParamDict", "APIAny_GetAutoTextsID", err, APINULLGuid);
		return;
	}
	for (UInt32 i = 0; i < autotexts.GetSize(); i++) {
		GS::UniString name = autotexts[i][0];
		GS::UniString rawName = "{@info:" + name.ToLowerCase() + "}";
		if (!propertyParams.ContainsKey(rawName)) {
			ParamValue pvalue;
			pvalue.name = autotexts[i][1];
			pvalue.rawName = rawName;
			pvalue.fromInfo = true;

			ParamHelpers::ConvertToParamValue(pvalue, rawName, autotexts[i][2]);
			propertyParams.Add(rawName, pvalue);
		}
	}
	ParamHelpers::AddValueToParamDictValue(propertyParams, "has_Info");
	DBPrintf("== SMSTF == GetAllInfoToParamDict end\n");
}

void ParamHelpers::GetAllGlobToParamDict(ParamDictValue & propertyParams) {
	GS::UniString name = "";
	GS::UniString rawName = "";
	ParamValue pvalue;
	API_PlaceInfo placeInfo = {};
	DBPrintf("== SMSTF == GetAllGlobToParamDict start\n");
	GSErrCode err = NoError;
#ifdef AC_27
	err = ACAPI_GeoLocation_GetPlaceSets(&placeInfo);
#else
	err = ACAPI_Environment(APIEnv_GetPlaceSetsID, &placeInfo, nullptr);
#endif
	if (err != NoError) {
		msg_rep("GetAllGlobToParamDict", "APIEnv_GetPlaceSetsID", err, APINULLGuid);
		return;
	}
	name = "GLOB_NORTH_DIR"; rawName = "{@glob:" + name.ToLowerCase() + "}";
	pvalue.name = name; pvalue.rawName = rawName;
	ParamHelpers::ConvertToParamValue(pvalue, rawName, round((placeInfo.north * 180 / PI) * 1000.0) / 1000.0);
	propertyParams.Add(rawName, pvalue);
	name = "GLOB_PROJECT_LONGITUDE"; rawName = "{@glob:" + name.ToLowerCase() + "}";
	pvalue.name = name; pvalue.rawName = rawName;
	ParamHelpers::ConvertToParamValue(pvalue, rawName, placeInfo.longitude);
	propertyParams.Add(rawName, pvalue);
	name = "GLOB_PROJECT_LATITUDE"; rawName = "{@glob:" + name.ToLowerCase() + "}";
	pvalue.name = name; pvalue.rawName = rawName;
	ParamHelpers::ConvertToParamValue(pvalue, rawName, placeInfo.latitude);
	propertyParams.Add(rawName, pvalue);
	name = "GLOB_PROJECT_ALTITUDE"; rawName = "{@glob:" + name.ToLowerCase() + "}";
	pvalue.name = name; pvalue.rawName = rawName;
	ParamHelpers::ConvertToParamValue(pvalue, rawName, placeInfo.altitude);
	propertyParams.Add(rawName, pvalue);
	name = "GLOB_SUN_AZIMUTH"; rawName = "{@glob:" + name.ToLowerCase() + "}";
	pvalue.name = name; pvalue.rawName = rawName;
	ParamHelpers::ConvertToParamValue(pvalue, rawName, round((placeInfo.sunAngXY * 180 / PI) * 1000.0) / 1000.0);
	propertyParams.Add(rawName, pvalue);
	name = "GLOB_SUN_ALTITUDE"; rawName = "{@glob:" + name.ToLowerCase() + "}";
	pvalue.name = name; pvalue.rawName = rawName;
	ParamHelpers::ConvertToParamValue(pvalue, rawName, round((placeInfo.sunAngZ * 180 / PI) * 1000.0) / 1000.0);
	propertyParams.Add(rawName, pvalue);
	ParamHelpers::AddValueToParamDictValue(propertyParams, "has_Glob");
	DBPrintf("== SMSTF == GetAllGlobToParamDict end\n");
}

// --------------------------------------------------------------------
// Заполнение свойств для элемента
// --------------------------------------------------------------------
void ParamHelpers::AllPropertyDefinitionToParamDict(ParamDictValue & propertyParams, const API_Guid & elemGuid) {
	DBPrintf("== SMSTF == AllPropertyDefinitionToParamDict GUID start\n");
	if (elemGuid == APINULLGuid) {
		if (!ParamHelpers::hasProperyDefinitoin(propertyParams)) ParamHelpers::AllPropertyDefinitionToParamDict(propertyParams);
	}
	else {
		GS::Array<API_PropertyDefinition> definitions;
		GSErrCode err = ACAPI_Element_GetPropertyDefinitions(elemGuid, API_PropertyDefinitionFilter_All, definitions);
		if (err != NoError) {
			msg_rep("GetAllPropertyDefinitionToParamDict", "ACAPI_Element_GetPropertyDefinitions", err, elemGuid);
			return;
		}
		if (definitions.IsEmpty()) return;
		ParamHelpers::AllPropertyDefinitionToParamDict(propertyParams, definitions);
	}
	DBPrintf("== SMSTF == AllPropertyDefinitionToParamDict GUID end\n");
}

// --------------------------------------------------------------------
// Перевод GS::Array<API_PropertyDefinition> в ParamDictValue
// --------------------------------------------------------------------
void ParamHelpers::AllPropertyDefinitionToParamDict(ParamDictValue & propertyParams, GS::Array<API_PropertyDefinition>&definitions) {
	if (definitions.IsEmpty()) return;
	DBPrintf("== SMSTF == GetAllPropertyDefinitionToParamDict definition start\n");
	UInt32 nparams = propertyParams.GetSize();
	bool needAddNew = (nparams == 0);
	for (UInt32 j = 0; j < definitions.GetSize(); j++) {
		ParamValue pvalue;
		ParamHelpers::ConvertToParamValue(pvalue, definitions[j]);
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
					DBPrintf("== SMSTF == GetAllPropertyDefinitionToParamDict definition return\n");
					return;
				}
			}
		}
	}
	DBPrintf("== SMSTF == GetAllPropertyDefinitionToParamDict definition end\n");
}

// --------------------------------------------------------------------
// Получение массива описаний свойств с указанием GUID родительского объекта
// --------------------------------------------------------------------
bool ParamHelpers::SubGuid_GetDefinition(const GS::Array<API_PropertyDefinition>&definitions, GS::Array<API_PropertyDefinition>&definitionsout) {
	if (definitions.IsEmpty()) return false;
	GS::HashTable<GS::UniString, API_PropertyDefinition> GuidDefinition;
	bool flag_find = false;
	for (UInt32 i = 0; i < definitions.GetSize(); i++) {
		if (!definitions[i].description.IsEmpty()) {
			if (definitions[i].description.Contains("Sync_GUID"))
			{
				definitionsout.Push(definitions[i]);
				flag_find = true;
			}
		}
	}
	return flag_find;
}

// --------------------------------------------------------------------
// Получение словаря значений свойств с указанием GUID родительского объекта
// --------------------------------------------------------------------
bool ParamHelpers::SubGuid_GetParamValue(const API_Guid & elemGuid, ParamDictValue & propertyParams, const GS::Array<API_PropertyDefinition>&definitions) {
	if (definitions.IsEmpty()) return false;
	GS::Array<API_PropertyDefinition> subdefinitions;
	if (!SubGuid_GetDefinition(definitions, subdefinitions)) return false;
	ParamDictValue subproperty;
	ParamHelpers::AllPropertyDefinitionToParamDict(subproperty, subdefinitions);
	ParamHelpers::Read(elemGuid, subproperty, propertyParams);
	if (propertyParams.IsEmpty()) {
		propertyParams = subproperty;
	}
	else {
		ParamHelpers::CompareParamDictValue(subproperty, propertyParams, true);
	}
	return true;
}

// --------------------------------------------------------------------
// Получить все доступные свойства в формарте ParamDictValue
// --------------------------------------------------------------------
void ParamHelpers::AllPropertyDefinitionToParamDict(ParamDictValue & propertyParams) {
	GS::Array<API_PropertyGroup> groups;
	DBPrintf("== SMSTF == AllPropertyDefinitionToParamDict start\n");
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

					// TODO Когда в проекте есть два и более свойств с описанием Sync_name возникает ошибка
					if (definitions[j].description.Contains("Sync_name")) {
						for (UInt32 inx = 0; inx < 20; inx++) {
							rawName = "{@property:sync_name" + GS::UniString::Printf("%d", inx) + "}";
							name = "sync_name" + GS::UniString::Printf("%d", inx);
							if (!propertyParams.ContainsKey(rawName)) break;
						}
						definitions[j].name = name;
						ParamValue pvalue;
						pvalue.rawName = rawName;
						pvalue.name = groups[i].name + "/" + definitions[j].name;
						ParamHelpers::ConvertToParamValue(pvalue, definitions[j]);
						propertyParams.Add(pvalue.rawName, pvalue);
					}
					else {
						name = groups[i].name + "/" + definitions[j].name;
						rawName = "{@property:" + name.ToLowerCase() + "}";
						bool changeExs = propertyParams.ContainsKey(rawName);
						if (needAddNew && !changeExs) {
							ParamValue pvalue;
							pvalue.rawName = rawName;
							pvalue.name = groups[i].name + "/" + definitions[j].name;
							ParamHelpers::ConvertToParamValue(pvalue, definitions[j]);
							propertyParams.Add(pvalue.rawName, pvalue);
						}
						else {
							ParamValue pvalue = propertyParams.Get(rawName);
							pvalue.rawName = rawName;
							pvalue.name = groups[i].name + "/" + definitions[j].name;
							ParamHelpers::ConvertToParamValue(pvalue, definitions[j]);
							propertyParams.Get(pvalue.rawName) = pvalue;
							nparams--;
							if (nparams == 0) {
								DBPrintf("== SMSTF == AllPropertyDefinitionToParamDict return\n");
								return;
							}
						}
					}
				}
			}
		}
	}
	ParamHelpers::AddValueToParamDictValue(propertyParams, "has_ProperyDefinitoin");
	DBPrintf("== SMSTF == AllPropertyDefinitionToParamDict end\n");
}

// --------------------------------------------------------------------
// Сопоставление двух словарей ParamDictElement
// --------------------------------------------------------------------
void ParamHelpers::CompareParamDictElement(ParamDictElement & paramsFrom, ParamDictElement & paramsTo) {
	if (paramsFrom.IsEmpty() || paramsTo.IsEmpty()) return;
	for (auto& cIt : paramsTo) {
		ParamDictValue& paramTo = *cIt.value;
		API_Guid elemGuid = *cIt.key;
		if (paramsFrom.ContainsKey(elemGuid)) {
			ParamHelpers::CompareParamDictValue(paramsFrom.Get(elemGuid), paramTo, false);
		}
	}
}

// --------------------------------------------------------------------
// Сопоставление двух словарей ParamDictValue
// --------------------------------------------------------------------
void ParamHelpers::CompareParamDictValue(ParamDictValue & paramsFrom, ParamDictValue & paramsTo) {
	ParamHelpers::CompareParamDictValue(paramsFrom, paramsTo, false);
}

// --------------------------------------------------------------------
// Сопоставление двух словарей ParamDictValue
// --------------------------------------------------------------------
void ParamHelpers::CompareParamDictValue(ParamDictValue & paramsFrom, ParamDictValue & paramsTo, bool addInNotEx) {
	if (paramsFrom.IsEmpty() || paramsTo.IsEmpty()) return;
	for (auto& cIt : paramsFrom) {
		GS::UniString k = *cIt.key;
		if (paramsTo.ContainsKey(k)) {
			ParamValue paramFrom = paramsFrom.Get(k);
			paramFrom.fromGuid = paramsTo.Get(paramFrom.rawName).fromGuid; // Чтоб GUID не перезаписался
			paramsTo.Set(paramFrom.rawName, paramFrom);
		}
		else {
			if (addInNotEx) {
				ParamValue paramFrom = paramsFrom.Get(k);
				paramsTo.Set(paramFrom.rawName, paramFrom);
			}
		}
	}
	return;
}

// --------------------------------------------------------------------
// Чтение значений свойств в ParamDictValue
// --------------------------------------------------------------------
bool ParamHelpers::ReadPropertyValues(const API_Guid & elemGuid, ParamDictValue & params) {
	if (params.IsEmpty()) return false;
	DBPrintf("== SMSTF ==      ReadPropertyValues\n");

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
	DBPrintf("== SMSTF == ReadPropertyValues -- no property\n");
	return false;
}

// -----------------------------------------------------------------------------
// Получение значения IFC свойств
// -----------------------------------------------------------------------------
bool ParamHelpers::ReadIFCValues(const API_Guid & elemGuid, ParamDictValue & params) {
	if (params.IsEmpty()) return false;
	DBPrintf("== SMSTF ==      ReadIFCValues\n");
	GS::Array<API_IFCProperty> properties;
	GSErrCode err = ACAPI_Element_GetIFCProperties(elemGuid, false, &properties);
	if (err != NoError) {
		msg_rep("ParamDictGetIFCValues", "ACAPI_Element_GetIFCProperties", err, elemGuid);
		return false;
	}
	bool flag_find = false;
	UInt32 nparams = params.GetSize();
	for (UInt32 i = 0; i < properties.GetSize(); i++) {
		API_IFCProperty prop = properties.Get(i);
		GS::UniString fname = properties[i].head.propertySetName + "/" + properties[i].head.propertyName;
		GS::UniString rawName = "{@ifc:" + fname.ToLowerCase() + "}";
		if (params.ContainsKey(rawName)) {
			ParamValue pvalue;
			API_IFCProperty property = properties.Get(i);
			if (ParamHelpers::ConvertToParamValue(pvalue, property)) {
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
			rawName = "{@ifc:" + fname.ToLowerCase() + "}";
			if (params.ContainsKey(rawName)) {
				ParamValue pvalue;
				API_IFCProperty property = properties.Get(i);
				if (ParamHelpers::ConvertToParamValue(pvalue, property)) {
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

bool ParamHelpers::ReadIDValues(const API_Elem_Head & elem_head, ParamDictValue & params) {
	if (params.IsEmpty()) return false;
	if (!params.ContainsKey("{@id:id}")) return false;
	DBPrintf("== SMSTF ==      ReadIDValues\n");
	ParamValue param = params.Get("{@id:id}");
	GS::UniString infoString;
	API_Guid elguid = elem_head.guid;
	GSErrCode err = NoError;
#ifdef AC_27
	err = ACAPI_Element_GetElementInfoString(&elguid, &infoString);
#else
	err = ACAPI_Database(APIDb_GetElementInfoStringID, &elguid, &infoString);
#endif
	if (err != NoError) {
		msg_rep("ReadIDValues - ID", "ACAPI_Database(APIDb_GetElementInfoStringID", err, elguid);
		return false;
	}
	else {
		param.isValid = true;
		param.val.type = API_PropertyStringValueType;
		param.type = API_PropertyStringValueType;
		param.val.boolValue = !infoString.IsEmpty();
		if (UniStringToDouble(infoString, param.val.doubleValue)) {
			param.val.intValue = (GS::Int32)param.val.doubleValue;
			param.val.canCalculate = true;
		}
		else {
			param.val.intValue = !infoString.IsEmpty();
			param.val.doubleValue = param.val.intValue * 1.0;
		}
		param.val.uniStringValue = infoString;
		params.Set(param.rawName, param);
		return true;
	}
}

// -----------------------------------------------------------------------------
// Получить значение GDL параметра по его имени или описанию в ParamValue
// -----------------------------------------------------------------------------
bool ParamHelpers::ReadGDLValues(const API_Element & element, const API_Elem_Head & elem_head, ParamDictValue & params) {
	if (params.IsEmpty()) return false;
	DBPrintf("== SMSTF ==      ReadGDLValues\n");
	API_ElemTypeID eltype;
#if defined AC_26 || defined AC_27
	eltype = elem_head.type.typeID;
#else
	eltype = elem_head.typeID;
#endif
	ParamDictValue paramBydescription;
	ParamDictValue paramByName;
	GS::HashTable<GS::UniString, GS::Array<GS::UniString>> paramnamearray;

	// Если диапазоны массивов хранятся в параметра х - прочитаем сначала их
	ParamDictValue paramdiap;
	for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = params.EnumeratePairs(); cIt != NULL; ++cIt) {
		ParamValue& param = *cIt->value;
		if (param.needPreRead) {
			if (!param.rawName_row_start.IsEmpty()) {
				ParamValue arr_row_start;
				arr_row_start.fromGDLparam = true;
				arr_row_start.rawName = param.rawName_row_start;
				paramdiap.Add(arr_row_start.rawName, arr_row_start);
			}
			if (!param.rawName_row_end.IsEmpty()) {
				ParamValue arr_row_end;
				arr_row_end.fromGDLparam = true;
				arr_row_end.rawName = param.rawName_row_end;
				paramdiap.Add(arr_row_end.rawName, arr_row_end);
			}
			if (!param.rawName_col_start.IsEmpty()) {
				ParamValue arr_col_start;
				arr_col_start.fromGDLparam = true;
				arr_col_start.rawName = param.rawName_col_start;
				paramdiap.Add(arr_col_start.rawName, arr_col_start);
			}
			if (!param.rawName_col_end.IsEmpty()) {
				ParamValue arr_col_end;
				arr_col_end.fromGDLparam = true;
				arr_col_end.rawName = param.rawName_col_end;
				paramdiap.Add(arr_col_end.rawName, arr_col_end);
			}
		}
	}
	if (!paramdiap.IsEmpty()) {
		if (ParamHelpers::GDLParamByName(element, elem_head, paramdiap, paramnamearray)) {
			for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = params.EnumeratePairs(); cIt != NULL; ++cIt) {
				ParamValue& param = *cIt->value;
				if (param.needPreRead) {
					if (!param.rawName_row_start.IsEmpty()) {
						if (paramdiap.ContainsKey(param.rawName_row_start)) params.Get(param.rawName).val.array_row_start = paramdiap.Get(param.rawName_row_start).val.intValue;
					}
					if (!param.rawName_row_end.IsEmpty()) {
						if (paramdiap.ContainsKey(param.rawName_row_end)) params.Get(param.rawName).val.array_row_end = paramdiap.Get(param.rawName_row_end).val.intValue;
					}
					if (!param.rawName_col_start.IsEmpty()) {
						if (paramdiap.ContainsKey(param.rawName_col_start)) params.Get(param.rawName).val.array_column_start = paramdiap.Get(param.rawName_col_start).val.intValue;
					}
					if (!param.rawName_col_end.IsEmpty()) {
						if (paramdiap.ContainsKey(param.rawName_col_end)) params.Get(param.rawName).val.array_column_end = paramdiap.Get(param.rawName_col_end).val.intValue;
					}
				}
			}
		}
	}

	// Разбиваем по типам поиска - по описанию/по имени
	for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = params.EnumeratePairs(); cIt != NULL; ++cIt) {
		ParamValue& param = *cIt->value;
		GS::UniString rawName = param.rawName;
		if (param.fromGDLArray) {
			GS::Array<GS::UniString> tparams;
			UInt32 nparam = StringSplt(rawName, "@arr", tparams);
			rawName = tparams[0] + "}";
			if (!paramnamearray.ContainsKey(rawName)) {
				GS::Array<GS::UniString> t;
				paramnamearray.Add(rawName, t);
			}
			paramnamearray.Get(rawName).Push(param.rawName);
			if (param.fromGDLdescription && eltype == API_ObjectID) {
				paramBydescription.Add(rawName, param);
			}
			else {
				if (param.fromGDLparam) paramByName.Add(rawName, param);
			}
		}
		if (param.fromGDLdescription && eltype == API_ObjectID) {
			paramBydescription.Add(param.rawName, param);
		}
		else {
			if (param.fromGDLparam) paramByName.Add(param.rawName, param);
		}
	}
	if (paramBydescription.IsEmpty() && paramByName.IsEmpty()) return false;

	// Поиск по описанию
	bool flag_find_desc = false;
	bool flag_find_name = false;
	if (!paramBydescription.IsEmpty()) {
		flag_find_desc = ParamHelpers::GDLParamByDescription(element, paramBydescription, paramByName, paramnamearray);
	}
	if (!paramByName.IsEmpty()) flag_find_name = ParamHelpers::GDLParamByName(element, elem_head, paramByName, paramnamearray);
	if (flag_find_desc && flag_find_name) {
		for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = paramBydescription.EnumeratePairs(); cIt != NULL; ++cIt) {
			ParamValue& param_by_desc = *cIt->value;
			GS::UniString rawname = param_by_desc.name;
			if (paramByName.ContainsKey(rawname)) {
				ParamValue param_by_name = paramByName.Get(rawname);
				GS::UniString desc_name = param_by_desc.val.uniStringValue;
				GS::UniString desc_rawname = param_by_desc.rawName;
				param_by_name.name = desc_name;
				param_by_name.rawName = desc_rawname;
				paramByName.Add(*cIt->key, param_by_name);
			}
		}
	}
	if (flag_find_name) ParamHelpers::CompareParamDictValue(paramByName, params);
	return (flag_find_name);
}

// -----------------------------------------------------------------------------
// Поиск по описанию GDL параметра
// Данный способ не работает с элементами навесных стен
// -----------------------------------------------------------------------------
bool ParamHelpers::GDLParamByDescription(const API_Element & element, ParamDictValue & params, ParamDictValue & find_params, GS::HashTable<GS::UniString, GS::Array<GS::UniString>>&paramnamearray) {
	API_LibPart libpart;
	BNZeroMemory(&libpart, sizeof(libpart));
	libpart.index = element.object.libInd;
	GSErrCode err = NoError;
#ifdef AC_27
	err = ACAPI_LibraryPart_Get(&libpart);
#else
	err = ACAPI_LibPart_Get(&libpart);
#endif
	if (err != NoError) {
		msg_rep("FindGDLParametersByDescription", "ACAPI_LibPart_Get", err, element.header.guid);
		return false;
	}
	double aParam = 0.0;
	double bParam = 0.0;
	Int32 addParNum = 0;
	API_AddParType** addPars = NULL;
#ifdef AC_27
	err = ACAPI_LibraryPart_GetParams(libpart.index, &aParam, &bParam, &addParNum, &addPars);
#else
	err = ACAPI_LibPart_GetParams(libpart.index, &aParam, &bParam, &addParNum, &addPars);
#endif
	if (err != NoError) {
		ACAPI_DisposeAddParHdl(&addPars);
		msg_rep("FindGDLParametersByDescription", "ACAPI_LibPart_GetParams", err, element.header.guid);
		return false;
	}
	bool flagFind = false;
	Int32 nfind = params.GetSize();

	// Ищем описание параметров
	for (Int32 i = 0; i < addParNum; ++i) {
		API_AddParType& actualParam = (*addPars)[i];
		GS::UniString desc_name = actualParam.uDescname;
		GS::UniString desc_rawname = "{@gdl:" + desc_name.ToLowerCase() + "}";
		if (params.ContainsKey(desc_rawname)) {

			// Получаем имя параметра
			GS::UniString name = actualParam.name;
			GS::UniString rawname = "{@gdl:" + name.ToLowerCase() + "}";

			// Если в словаре для чтения по имени параметра такого параметра нет - добавим
			if (!find_params.ContainsKey(rawname)) {
				ParamValue pvalue = params.Get(desc_rawname);
				pvalue.rawName = rawname;
				find_params.Add(rawname, pvalue);
			}

			// Описание на время сохраним в val.uniStringValue
			params.Get(desc_rawname).val.uniStringValue = params.Get(desc_rawname).name;

			// rawname с именем параметра для дальнейшего сопоставления
			params.Get(desc_rawname).name = rawname;
			flagFind = true;
			nfind--;
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
bool ParamHelpers::GDLParamByName(const API_Element & element, const API_Elem_Head & elem_head, ParamDictValue & params, GS::HashTable<GS::UniString, GS::Array<GS::UniString>>&paramnamearray) {
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
		GS::UniString rawname = "{@gdl:" + name.ToLowerCase() + "}";
		if (params.ContainsKey(rawname)) {

			// Проверим - нет ли подходящих параметров-массивов?
			if (paramnamearray.ContainsKey(rawname)) {
				GS::Array<GS::UniString> paramarray = paramnamearray.Get(rawname);
				for (UInt32 j = 0; j < paramarray.GetSize(); ++j) {
					nfind--;
					ParamValue pvalue = params.Get(paramarray[j]);
					ParamHelpers::ConvertToParamValue(pvalue, actualParam);
					if (pvalue.isValid) {
						params.Set(pvalue.rawName, pvalue);
						flagFind = true;
					}
				}
				ParamValue pvalue = params.Get(rawname);
				ParamHelpers::ConvertToParamValue(pvalue, actualParam);
				if (pvalue.isValid) {
					params.Set(rawname, pvalue);
					flagFind = true;
				}
				nfind--;
			}
			else {
				nfind--;
				ParamValue pvalue = params.Get(rawname);
				ParamHelpers::ConvertToParamValue(pvalue, actualParam);
				if (pvalue.isValid) {
					params.Set(rawname, pvalue);
					flagFind = true;
				}
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
	DBPrintf("== SMSTF ==      ReadMaterial\n");

	// Получим состав элемента, добавив в словарь требуемые параметры
	ParamDictValue paramsAdd;
	if (!ParamHelpers::Components(element, params, paramsAdd)) return false;

	ParamDictValue paramlayers;
	for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = params.EnumeratePairs(); cIt != NULL; ++cIt) {
		ParamValue& param = *cIt->value;
		if (param.fromMaterial) {
			if (param.rawName.Contains("{@material:layers")) {
				paramlayers.Add(param.rawName, param);
			}
		}
	}
	if (paramlayers.IsEmpty()) return true;

	// В свойствах могли быть ссылки на другие свойста. Проверим, распарсим
	if (!paramsAdd.IsEmpty()) {
		ParamHelpers::CompareParamDictValue(propertyParams, paramsAdd);
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

				// Если для материала было указано уникальное наименование - заменим его
				GS::UniString attribsuffix = CharENTER + GS::UniString::Printf("%d", constrinx) + "}";
				for (UInt32 inx = 0; inx < 20; inx++) {
					GS::UniString syncname = "{@property:sync_name" + GS::UniString::Printf("%d", inx) + attribsuffix;
					if (params.ContainsKey(syncname)) {
						if (params.Get(syncname).isValid && !params.Get(syncname).property.isDefault) {
							templatestring = params.Get(syncname).val.uniStringValue;
							break;
						}
					}
				}

				// Если нужно заполнить толщину
				GS::UniString layer_thickness = "{@material:layer thickness}";
				if (params.ContainsKey(layer_thickness)) {
					double fillThick = param_composite.composite[i].fillThick;
					GS::UniString formatsting = params.Get(layer_thickness).val.stringformat;
					if (formatsting.IsEmpty()) {
						formatsting = "1mm";
						params.Get(layer_thickness).val.stringformat = formatsting;
						params.Get(layer_thickness).val.n_zero = 1;
					}
					GS::UniString fillThickstring = PropertyHelpers::NumToString(fillThick, formatsting);
					templatestring.ReplaceAll(layer_thickness, fillThickstring);
				}
				templatestring.ReplaceAll("{@material:n}", GS::UniString::Printf("%d", i + 1));
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

void ParamHelpers::Array2ParamValue(GS::Array<ParamValueData>&pvalue, ParamValueData & pvalrezult) {
	if (pvalue.IsEmpty()) return;
	GS::UniString delim = ";";
	int array_format_out = pvalrezult.array_format_out;
	GS::UniString param_string = "";
	double param_real = 0;
	bool param_bool = false;
	if (array_format_out == ARRAY_MIN) param_bool = true;
	GS::Int32 param_int = 0;
	bool canCalculate = false;
	std::string p = "";

	if (array_format_out == ARRAY_MAX || array_format_out == ARRAY_MIN) {
		param_real = pvalue.Get(0).doubleValue;
		param_int = pvalue.Get(0).intValue;
		param_bool = pvalue.Get(0).boolValue;
		param_string = pvalue.Get(0).uniStringValue;
	}

	for (UInt32 i = 0; i < pvalue.GetSize(); i++) {
		ParamValueData pval = pvalue.Get(i);
		if (pval.canCalculate) {
			canCalculate = true;
			if (array_format_out == ARRAY_SUM || array_format_out == ARRAY_UNIC) {
				param_real = param_real + pval.doubleValue;
				param_int = param_int + pval.intValue;
				param_bool = param_bool + pval.boolValue;
			}
			if (array_format_out == ARRAY_MAX) {
				param_real = fmax(param_real, pval.doubleValue);
				if (pval.intValue > param_int) param_int = pval.intValue;
				if (pval.boolValue) param_bool = true;
			}
			if (array_format_out == ARRAY_MIN) {
				param_real = fmin(param_real, pval.doubleValue);
				if (pval.intValue < param_int) param_int = pval.intValue;
				if (!pval.boolValue) param_bool = false;
			}
		}
		if (array_format_out == ARRAY_SUM || array_format_out == ARRAY_UNIC) {
			if (param_string.IsEmpty()) {
				param_string = pval.uniStringValue;
			}
			else {
				param_string = param_string + delim + pval.uniStringValue;
			}
		}
		if (array_format_out == ARRAY_MAX) {
			std::string s = pval.uniStringValue.ToCStr(0, MaxUSize, GChCode).Get();
			p = param_string.ToCStr(0, MaxUSize, GChCode).Get();
			if (doj::alphanum_comp(s, p) > 0) param_string = GS::UniString(s.c_str(), GChCode);
		}
		if (array_format_out == ARRAY_MIN) {
			std::string s = pval.uniStringValue.ToCStr(0, MaxUSize, GChCode).Get();
			if (doj::alphanum_comp(s, p) < 0) param_string = GS::UniString(s.c_str(), GChCode);
		}
	}
	pvalrezult = pvalue.Get(0);
	if (array_format_out == ARRAY_UNIC) {
		pvalrezult.uniStringValue = StringUnic(param_string, delim);
	}
	else {
		pvalrezult.uniStringValue = param_string;
	}
	pvalrezult.boolValue = param_bool;
	pvalrezult.doubleValue = param_real;
	pvalrezult.intValue = param_int;
	pvalrezult.canCalculate = canCalculate;
}

// -----------------------------------------------------------------------------
// Конвертация одиночного параметра библиотечного элемента (тип API_ParSimple) в ParamValue
// -----------------------------------------------------------------------------
bool ParamHelpers::ConvertToParamValue(ParamValueData & pvalue, const API_AddParID & typeIDr, const GS::UniString & pstring, const double& preal) {

	// TODO добавить округления на основе настроек проекта
	GS::UniString param_string = pstring;
	double param_real = preal;
	bool param_bool = false;
	GS::Int32 param_int = 0;
	pvalue.canCalculate = false;
	if (typeIDr == APIParT_CString) {
		pvalue.type = API_PropertyStringValueType;
		param_bool = (!param_string.IsEmpty());

		if (UniStringToDouble(param_string, param_real)) {
			param_real = round(param_real * 100000) / 100000;
			param_int = (GS::Int32)param_real;
			if (param_int / 1 < param_real) param_int += 1;
			pvalue.canCalculate = true;
		}
		else {
			if (param_bool) {
				param_int = 1;
				param_real = 1.0;
			}
		}
	}
	else {
		param_real = round(param_real * 100000) / 100000;
		if (preal - param_real > 0.00001) param_real += 0.00001;
		param_int = (GS::Int32)param_real;
		if (param_int / 1 < param_real) param_int += 1;
	}
	if (abs(param_real) > std::numeric_limits<double>::epsilon()) param_bool = true;

	// Если параметр не строковое - определяем текстовое значение конвертацией
	if (typeIDr != APIParT_CString) {
		API_AttrTypeID attrType = API_ZombieAttrID;
#ifdef AC_27
		API_AttributeIndex attrInx = ACAPI_CreateAttributeIndex(param_int);
#else
		short attrInx = (short)param_int;
#endif
		switch (typeIDr) {
		case APIParT_Integer:
			param_string = GS::UniString::Printf("%d", param_int);
			pvalue.type = API_PropertyIntegerValueType;
			pvalue.canCalculate = true;
			pvalue.n_zero = 0;
			pvalue.stringformat = "0m";
			break;
		case APIParT_Boolean:
			pvalue.n_zero = 0;
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
			pvalue.stringformat = "0m";
			pvalue.n_zero = 0;
			pvalue.canCalculate = true;
			pvalue.type = API_PropertyBooleanValueType;
			break;
		case APIParT_Length:
			param_string = GS::UniString::Printf("%.0f", param_real * 1000);
			pvalue.canCalculate = true;
			pvalue.type = API_PropertyRealValueType;
			pvalue.stringformat = "1mm";
			break;
		case APIParT_Angle:
			param_string = GS::UniString::Printf("%.1f", param_real);
			pvalue.canCalculate = true;
			pvalue.type = API_PropertyRealValueType;
			pvalue.stringformat = "2m";
			break;
		case APIParT_RealNum:
			param_string = GS::UniString::Printf("%.3f", param_real);
			pvalue.canCalculate = true;
			pvalue.type = API_PropertyRealValueType;
			pvalue.stringformat = "3m";
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
#ifdef AC_27
				param_bool = (attrInx.ToInt32_Deprecated() != 0);
				param_int = attrInx.ToInt32_Deprecated();
#else
				param_bool = (attrInx != 0);
				param_int = attrInx;
#endif
				param_real = param_int / 1.0;
				pvalue.n_zero = 0;
			}
			else {
				return false;
			}
		}
	}
	pvalue.boolValue = param_bool;
	pvalue.doubleValue = param_real;
	pvalue.intValue = param_int;
	pvalue.uniStringValue = param_string;
	return true;
}

// -----------------------------------------------------------------------------
// Конвертация параметра-массива библиотечного элемента (тип API_ParArray) в ParamValue
// -----------------------------------------------------------------------------
bool ParamHelpers::ConvertToParamValue(ParamValueData & pvalue, const API_AddParID & typeIDr, const GS::Array<GS::UniString> &pstring, const GS::Array<double> &preal, const GS::Int32 & dim1, const GS::Int32 & dim2) {

	// TODO Добавить обработку игнорируемых значений
	GS::Array<ParamValueData> pvalues;
	GS::UniString param_string = "";
	double param_real = 0.0;
	GS::Int32 inx_row = 1;
	GS::Int32 inx_col = 0;
	int array_row_start = pvalue.array_row_start;
	int array_row_end = pvalue.array_row_end;
	int array_column_start = pvalue.array_column_start;
	int array_column_end = pvalue.array_column_end;
	if (array_row_end < 1) array_row_end = dim1;
	if (array_column_end < 1) array_column_end = dim2;

	UInt32 n = 0;
	if (typeIDr == APIParT_CString) {
		if (pstring.IsEmpty()) return false;
		n = pstring.GetSize();
	}
	else {
		if (preal.IsEmpty()) return false;
		n = preal.GetSize();
	}
	for (UInt32 i = 0; i < n; i++) {
		inx_col += 1;
		if (inx_col > dim2) {
			inx_col = 1;
			inx_row += 1;
		}
		if (inx_col <= array_column_end && inx_col >= array_column_start && inx_row >= array_row_start && inx_row <= array_row_end) {
			ParamValueData pval = {};
			if (typeIDr == APIParT_CString) {
				param_string = pstring.Get(i);
			}
			else {
				param_real = preal.Get(i);
			}
			if (ParamHelpers::ConvertToParamValue(pval, typeIDr, param_string, param_real)) {
				pvalues.Push(pval);
			}
		}
	}
	if (pvalues.IsEmpty()) {
		DBPrintf("== SMSTF == Empty array\n");
		return false;
	}
	else {
		ParamHelpers::Array2ParamValue(pvalues, pvalue);
		return true;
	}
}

// -----------------------------------------------------------------------------
// Конвертация параметра библиотечного элемента в ParamValue
// -----------------------------------------------------------------------------
bool ParamHelpers::ConvertToParamValue(ParamValue & pvalue, const API_AddParType & nthParameter) {
	GS::UniString param_string = "";
	double param_real = 0.0;
	API_AddParID typeIDr = nthParameter.typeID;
	ParamValueData pval = pvalue.val;
	if (nthParameter.typeMod == API_ParArray) {
		size_t ind = 0;
		GS::Array<GS::UniString> arr_param_string;
		GS::Array<double> arr_param_real;
		if (nthParameter.typeID != APIParT_CString) {
			for (Int32 i = 1; i <= nthParameter.dim1; i++) {
				for (Int32 j = 1; j <= nthParameter.dim2; j++) {
					param_real = (Int32)((double*)*nthParameter.value.array)[ind];
					arr_param_real.Push(param_real);
					ind++;
				}
			}
		}
		else {
			char* valueStr;
			UInt32 num_chars = BMGetHandleSize((GSConstHandle)nthParameter.value.array) / sizeof(GS::uchar_t);
			for (UInt32 i = 0; i < num_chars; i++) {
				valueStr = *nthParameter.value.array + ind;
				if (*valueStr == '\0') {
					arr_param_string.Push(param_string);
					param_string = "";
					ind += 2;
				}
				else {
					param_string.Append(valueStr);
					ind += strlen(valueStr) + 1;
				}
			}
		}
		if (!ParamHelpers::ConvertToParamValue(pval, typeIDr, arr_param_string, arr_param_real, nthParameter.dim1, nthParameter.dim2)) return false;
	}
	if (nthParameter.typeMod == API_ParSimple) {
		param_string = nthParameter.value.uStr;
		param_real = nthParameter.value.real;
		if (!ParamHelpers::ConvertToParamValue(pval, typeIDr, param_string, param_real)) return false;
	}
	if (pvalue.rawName.IsEmpty()) pvalue.rawName = "{@gdl:" + GS::UniString(nthParameter.name).ToLowerCase() + "}";
	if (pvalue.name.IsEmpty()) pvalue.name = nthParameter.name;
	pvalue.val = pval;
	pvalue.type = pval.type;
	pvalue.fromGDLparam = true;
	pvalue.isValid = true;
	return true;
}

// -----------------------------------------------------------------------------
// Конвертация свойства в ParamValue
// -----------------------------------------------------------------------------
bool ParamHelpers::ConvertToParamValue(ParamValue & pvalue, const API_Property & property) {
	if (pvalue.rawName.IsEmpty() || pvalue.name.IsEmpty()) {
		GS::UniString fname;
		GetPropertyFullName(property.definition, fname);
		if (pvalue.rawName.IsEmpty()) pvalue.rawName = "{@property:" + fname.ToLowerCase() + "}";
		if (pvalue.name.IsEmpty()) pvalue.name = fname;
	}
#if defined(AC_22) || defined(AC_23)
	pvalue.isValid = property.isEvaluated;
#else
	pvalue.isValid = (property.status == API_Property_HasValue);
#endif
	if (!pvalue.isValid) {
		return false;
	}
	pvalue.val.boolValue = false;
	pvalue.val.intValue = 0;
	pvalue.val.doubleValue = 0.0;
	pvalue.val.uniStringValue = PropertyHelpers::ToString(property);
	std::string var = pvalue.val.uniStringValue.ToCStr(0, MaxUSize, GChCode).Get();
	FormatStringDict formatstringdict;
	switch (property.definition.valueType) {
	case API_PropertyIntegerValueType:
		pvalue.val.intValue = property.value.singleVariant.variant.intValue;
		pvalue.val.doubleValue = property.value.singleVariant.variant.intValue * 1.0;
		if (pvalue.val.intValue > 0) pvalue.val.boolValue = true;
		pvalue.val.type = API_PropertyIntegerValueType;
		pvalue.val.canCalculate = true;
		pvalue.val.n_zero = 0;
		break;
	case API_PropertyRealValueType:
		pvalue.val.doubleValue = round(property.value.singleVariant.variant.doubleValue * 1000) / 1000;
		if (property.value.singleVariant.variant.doubleValue - pvalue.val.doubleValue > 0.001) pvalue.val.doubleValue += 0.001;

		// Конвертация угла из радиан в градусы
		if (property.definition.measureType == API_PropertyAngleMeasureType) {
			pvalue.val.doubleValue = round((pvalue.val.doubleValue * 180 * 1000 / PI) / 1000);
		}
		pvalue.val.intValue = (GS::Int32)pvalue.val.doubleValue;
		if (pvalue.val.intValue / 1 < pvalue.val.doubleValue) pvalue.val.intValue += 1;
		if (abs(pvalue.val.doubleValue) > std::numeric_limits<double>::epsilon()) pvalue.val.boolValue = true;
		pvalue.val.type = API_PropertyRealValueType;
		formatstringdict = GetFotmatStringForMeasureType();
		if (formatstringdict.ContainsKey(property.definition.measureType)) {
			if (formatstringdict.Get(property.definition.measureType).needRound && property.definition.measureType != API_PropertyLengthMeasureType) {
				double l = pow(10, formatstringdict.Get(property.definition.measureType).n_zero);
				pvalue.val.doubleValue = round(pvalue.val.doubleValue * pow(10, formatstringdict.Get(property.definition.measureType).n_zero)) / pow(10, formatstringdict.Get(property.definition.measureType).n_zero);
				pvalue.val.intValue = (GS::Int32)pvalue.val.doubleValue;
				if (abs(pvalue.val.doubleValue) > std::numeric_limits<double>::epsilon()) pvalue.val.boolValue = true;
			}
			if (pvalue.val.stringformat.IsEmpty()) {
				pvalue.val.stringformat = formatstringdict.Get(property.definition.measureType).stringformat;
				pvalue.val.n_zero = formatstringdict.Get(property.definition.measureType).n_zero;
			}
			pvalue.val.uniStringValue = ParamHelpers::ToString(pvalue);
		}
		pvalue.val.canCalculate = true;
		break;
	case API_PropertyBooleanValueType:
		pvalue.val.boolValue = property.value.singleVariant.variant.boolValue;
		if (pvalue.val.boolValue) {
			pvalue.val.intValue = 1;
			pvalue.val.doubleValue = 1.0;
		}
		pvalue.val.type = API_PropertyBooleanValueType;
		pvalue.val.canCalculate = true;
		pvalue.val.n_zero = 0;
		break;
	case API_PropertyStringValueType:
	case API_PropertyGuidValueType:
		pvalue.val.type = API_PropertyStringValueType;
		pvalue.val.boolValue = !pvalue.val.uniStringValue.IsEmpty();
		if (UniStringToDouble(pvalue.val.uniStringValue, pvalue.val.doubleValue)) {
			pvalue.val.intValue = (GS::Int32)pvalue.val.doubleValue;
			if (pvalue.val.intValue / 1 < pvalue.val.doubleValue) pvalue.val.intValue += 1;
			pvalue.val.canCalculate = true;
		}
		else {
			if (pvalue.val.boolValue) {
				pvalue.val.intValue = 1;
				pvalue.val.doubleValue = 1.0;
			}
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
	return true;
	}

// -----------------------------------------------------------------------------
// Конвертация определения свойства в ParamValue
// -----------------------------------------------------------------------------
bool ParamHelpers::ConvertToParamValue(ParamValue & pvalue, const API_PropertyDefinition & definition) {
	if (pvalue.rawName.IsEmpty() || pvalue.name.IsEmpty()) {
		GS::UniString fname;
		GetPropertyFullName(definition, fname);
		if (pvalue.rawName.IsEmpty()) pvalue.rawName = "{@property:" + fname.ToLowerCase() + "}";
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
	if (definition.description.Contains("ync_name")) {
		if (!pvalue.rawName.Contains("{@property:sync_name")) {
			pvalue.rawName = "{@property:sync_name0}";
			pvalue.name = "Sync_name0";
		}
		pvalue.fromAttribDefinition = true;
	}
	pvalue.fromProperty = true;
	pvalue.fromPropertyDefinition = !pvalue.fromAttribDefinition;
	pvalue.definition = definition;
	return true;
}

// -----------------------------------------------------------------------------
// Конвертация строки в ParamValue
// -----------------------------------------------------------------------------
bool ParamHelpers::ConvertToParamValue(ParamValue & pvalue, const GS::UniString & paramName, const GS::UniString & strvalue) {
	if (pvalue.name.IsEmpty()) pvalue.name = paramName;
	if (pvalue.rawName.IsEmpty()) pvalue.rawName = "{@gdl:" + paramName.ToLowerCase() + "}";
	pvalue.val.uniStringValue = strvalue;
	pvalue.val.boolValue = !pvalue.val.uniStringValue.IsEmpty();
	if (UniStringToDouble(pvalue.val.uniStringValue, pvalue.val.doubleValue)) {
		pvalue.val.intValue = (GS::Int32)pvalue.val.doubleValue;
		if (pvalue.val.intValue / 1 < pvalue.val.doubleValue) pvalue.val.intValue += 1;
		pvalue.val.canCalculate = true;
	}
	else {
		if (pvalue.val.boolValue) {
			pvalue.val.intValue = 1;
			pvalue.val.doubleValue = 1.0;
		}
	}
	pvalue.val.type = API_PropertyStringValueType;
	pvalue.isValid = true;

	return true;
}

// -----------------------------------------------------------------------------
// Конвертация целого числа в ParamValue
// -----------------------------------------------------------------------------
bool ParamHelpers::ConvertToParamValue(ParamValue & pvalue, const GS::UniString & paramName, const Int32 intValue) {
	if (pvalue.name.IsEmpty()) pvalue.name = paramName;
	if (pvalue.rawName.IsEmpty()) pvalue.rawName = "{@gdl:" + paramName.ToLowerCase() + "}";
	pvalue.val.type = API_PropertyIntegerValueType;
	pvalue.val.canCalculate = true;
	pvalue.val.intValue = intValue;
	pvalue.val.doubleValue = intValue * 1.0;
	if (pvalue.val.intValue > 0) pvalue.val.boolValue = true;
	pvalue.val.uniStringValue = GS::UniString::Printf("%d", intValue);
	pvalue.isValid = true;
	pvalue.val.n_zero = 0;
	return true;
}

// -----------------------------------------------------------------------------
// Конвертация double в ParamValue
// -----------------------------------------------------------------------------
bool ParamHelpers::ConvertToParamValue(ParamValue & pvalue, const GS::UniString & paramName, const double doubleValue) {
	if (pvalue.name.IsEmpty()) pvalue.name = paramName;
	if (pvalue.rawName.IsEmpty()) pvalue.rawName = "{@gdl:" + paramName.ToLowerCase() + "}";
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
bool ParamHelpers::ConvertToParamValue(ParamValue & pvalue, const API_IFCProperty & property) {
	if (pvalue.rawName.IsEmpty() || pvalue.name.IsEmpty()) {
		GS::UniString fname = property.head.propertySetName + "/" + property.head.propertyName;
		if (pvalue.rawName.IsEmpty()) pvalue.rawName = "{@ifc:" + fname.ToLowerCase() + "}";
		if (pvalue.name.IsEmpty()) pvalue.name = fname;
	}
	pvalue.isValid = true;
	if (property.head.propertyType == API_IFCPropertySingleValueType) {
		switch (property.singleValue.nominalValue.value.primitiveType) {
		case API_IFCPropertyAnyValueStringType:
			pvalue.val.type = API_PropertyStringValueType;
			pvalue.val.uniStringValue = property.singleValue.nominalValue.value.stringValue;
			pvalue.val.boolValue = !pvalue.val.uniStringValue.IsEmpty();
			if (UniStringToDouble(pvalue.val.uniStringValue, pvalue.val.doubleValue)) {
				pvalue.val.intValue = (GS::Int32)pvalue.val.doubleValue;
				if (pvalue.val.intValue / 1 < pvalue.val.doubleValue) pvalue.val.intValue += 1;
				pvalue.val.canCalculate = true;
			}
			else {
				if (pvalue.val.boolValue) {
					pvalue.val.intValue = 1;
					pvalue.val.doubleValue = 1.0;
				}
			}
			break;
		case API_IFCPropertyAnyValueRealType:
			pvalue.val.canCalculate = true;
			pvalue.val.type = API_PropertyRealValueType;
			pvalue.val.doubleValue = round(property.singleValue.nominalValue.value.doubleValue * 1000) / 1000;
			if (property.singleValue.nominalValue.value.doubleValue - pvalue.val.doubleValue > 0.001) pvalue.val.doubleValue += 0.001;
			pvalue.val.intValue = (GS::Int32)pvalue.val.doubleValue;
			if (pvalue.val.intValue / 1 < pvalue.val.doubleValue) pvalue.val.intValue += 1;
			if (abs(pvalue.val.doubleValue) > std::numeric_limits<double>::epsilon()) pvalue.val.boolValue = true;
			pvalue.val.uniStringValue = GS::UniString::Printf("%.3f", pvalue.val.doubleValue);
			break;
		case API_IFCPropertyAnyValueIntegerType:
			pvalue.val.canCalculate = true;
			pvalue.val.type = API_PropertyIntegerValueType;
			pvalue.val.intValue = (GS::Int32)property.singleValue.nominalValue.value.intValue;
			pvalue.val.doubleValue = pvalue.val.intValue * 1.0;
			if (pvalue.val.intValue > 0) pvalue.val.boolValue = true;
			pvalue.val.uniStringValue = GS::UniString::Printf("%d", pvalue.val.intValue);
			pvalue.val.n_zero = 0;
			break;
		case API_IFCPropertyAnyValueBooleanType:
			pvalue.val.canCalculate = true;
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
			pvalue.val.n_zero = 0;
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
void ParamHelpers::ConvertByFormatString(ParamValue & pvalue) {
	if (pvalue.val.type == API_PropertyRealValueType || pvalue.val.type == API_PropertyIntegerValueType) {
		Int32 n_zero = 3;
		Int32 krat = 0;
		double koeff = 1;
		bool trim_zero = true;
		PropertyHelpers::ParseFormatString(pvalue.val.stringformat, n_zero, krat, koeff, trim_zero);
		UNUSED_VARIABLE(krat); UNUSED_VARIABLE(trim_zero);
		pvalue.val.uniStringValue = ParamHelpers::ToString(pvalue);
		if (koeff != 1) n_zero = n_zero + (GS::Int32)log10(koeff);
		pvalue.val.doubleValue = round(pvalue.val.doubleValue * pow(10, n_zero)) / pow(10, n_zero);
		pvalue.val.intValue = (GS::Int32)pvalue.val.doubleValue;
		if (abs(pvalue.val.doubleValue) > std::numeric_limits<double>::epsilon()) pvalue.val.boolValue = true;
	}
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
bool ParamHelpers::ComponentsBasicStructure(const API_AttributeIndex & constrinx, const double& fillThick, const API_AttributeIndex & constrinx_ven, const double& fillThick_ven, ParamDictValue & params, ParamDictValue & paramlayers, ParamDictValue & paramsAdd) {
	DBPrintf("== SMSTF ==          ComponentsBasicStructure\n");
	ParamValue param_composite = {};
	if (fillThick_ven > 0.0001) {
		ParamValueComposite layer = {};
		layer.inx = constrinx_ven;
		layer.fillThick = fillThick_ven;
		param_composite.composite.Push(layer);
		ParamHelpers::GetAttributeValues(constrinx_ven, params, paramsAdd);
	}
	ParamValueComposite layer = {};
	layer.inx = constrinx;
	layer.fillThick = fillThick;
	param_composite.composite.Push(layer);
	ParamHelpers::GetAttributeValues(constrinx, params, paramsAdd);
	for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = paramlayers.EnumeratePairs(); cIt != NULL; ++cIt) {
		paramlayers.Get(*cIt->key).composite = param_composite.composite;
	}
	ParamHelpers::CompareParamDictValue(paramlayers, params);
	return true;
}

// --------------------------------------------------------------------
// Получение данных из многослойной конструкции
// --------------------------------------------------------------------
bool ParamHelpers::ComponentsCompositeStructure(const API_Guid & elemguid, API_AttributeIndex & constrinx, ParamDictValue & params, ParamDictValue & paramlayers, ParamDictValue & paramsAdd, GS::HashTable<API_AttributeIndex, bool>&existsmaterial) {
	DBPrintf("== SMSTF ==          ComponentsCompositeStructure\n");
	API_Attribute attrib;
	BNZeroMemory(&attrib, sizeof(API_Attribute));

	API_AttributeDef defs;
	BNZeroMemory(&defs, sizeof(API_AttributeDef));

	attrib.header.index = constrinx;
	attrib.header.typeID = API_CompWallID;
	GSErrCode err = ACAPI_Attribute_Get(&attrib);
	if (err != NoError) {
		msg_rep("materialString::ComponentsCompositeStructure", " ACAPI_Attribute_Get", err, elemguid);
		return false;
	}
	err = ACAPI_Attribute_GetDef(attrib.header.typeID, attrib.header.index, &defs);
	if (err != NoError) {
		msg_rep("materialString::ComponentsCompositeStructure", " ACAPI_Attribute_GetDef", err, elemguid);
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
	ParamHelpers::CompareParamDictValue(paramlayers, params);
	ACAPI_DisposeAttrDefsHdls(&defs);
	return true;
}

// --------------------------------------------------------------------
// Получение данных из сложного профиля
// --------------------------------------------------------------------
#ifndef AC_23
bool ParamHelpers::ComponentsProfileStructure(ProfileVectorImage & profileDescription, ParamDictValue & params, ParamDictValue & paramlayers, ParamDictValue & paramsAdd, GS::HashTable<API_AttributeIndex, bool>&existsmaterial) {
	DBPrintf("== SMSTF ==          ComponentsProfileStructure\n");
	ConstProfileVectorImageIterator profileDescriptionIt(profileDescription);
	GS::HashTable<short, OrientedSegments> lines; // Для хранения точки начала сечения и линии сечения
	GS::HashTable<short, GS::Array<Sector>> segment; // Для хранения отрезков линий сечения и последующего объединения
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
				GS::Array<Sector> segments;
				lines.Add(pen, s);
				segment.Add(pen, segments);
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
				Point2D s = { pSyArc->origC };
				lines.Get(pen).start = s;
			}
		}
		break;
		case SyLine: // Поиск линий-сечений
		{
			const Sy_LinType* pSyPolyLine = static_cast <const Sy_LinType*> (profileDescriptionIt);
			short pen = pSyPolyLine->GetExtendedPen().GetIndex();
			if (segment.ContainsKey(pen)) {
				Sector line = { pSyPolyLine->begC, pSyPolyLine->endC };
				segment.Get(pen).Push(line);
				profilehasLine = true;
			}
		}
		break;
		}
		++profileDescriptionIt;
	}

	// Если линии сечения не найдены - создадим парочку - вертикальную и горизонтальную
	if (!profilehasLine) {
		Point2D p1 = { -1000, 0 };
		Point2D p2 = { 1000, 0 };
		Sector cut1 = { p1, p2 };
		OrientedSegments d;
		d.start = p2;
		d.cut_start = p1;
		d.cut_direction = Geometry::SectorVector(cut1);
		if (lines.ContainsKey(20)) {
			lines.Set(20, d);
		}
		else {
			lines.Add(20, d);
		}
		Point2D p3 = { 0, -1000 };
		Point2D p4 = { 0, 1000 };
		Sector cut2 = { p3, p4 };
		OrientedSegments d2;
		d2.start = p3;
		d2.cut_start = p4;
		d2.cut_direction = Geometry::SectorVector(cut2);
		if (lines.ContainsKey(6)) {
			lines.Set(6, d2);
		}
		else {
			lines.Add(6, d2);
		}
		ParamValue p;
		param_composite.Add(20, p);
		param_composite.Add(6, p);
	}
	else {

		// Проходим по сегментам, соединяем их в одну линию
		for (GS::HashTable<short, GS::Array<Sector>>::PairIterator cIt = segment.EnumeratePairs(); cIt != NULL; ++cIt) {
			GS::Array<Sector>& segment = *cIt->value;
			Point2D pstart = lines.Get(*cIt->key).start;
			Sector cutline;
			double max_r = 0; double min_r = 300000;
			for (UInt32 j = 0; j < segment.GetSize(); j++) {
				double r = Geometry::Dist(pstart, segment[j].c1);
				if (r > max_r) {
					cutline.c1 = segment[j].c1;
					max_r = r;
				}
				if (r < min_r) {
					cutline.c2 = segment[j].c1;
					min_r = r;
				}
				r = Geometry::Dist(pstart, segment[j].c2);
				if (r > max_r) {
					cutline.c1 = segment[j].c2;
					max_r = r;
				}
				if (r < min_r) {
					cutline.c2 = segment[j].c2;
					min_r = r;
				}
			}
			lines.Get(*cIt->key).cut_start = cutline.c2;
			lines.Get(*cIt->key).cut_direction = Geometry::SectorVector(cutline);
		}
	}
	bool hasData = false;
	ConstProfileVectorImageIterator profileDescriptionIt1(profileDescription);
	while (!profileDescriptionIt1.IsEOI()) {
		switch (profileDescriptionIt1->item_Typ) {
		case SyHatch:
		{
			const HatchObject& syHatch = profileDescriptionIt1;
			Geometry::MultiPolygon2D result;

			// Получаем полигон штриховки
			if (syHatch.ToPolygon2D(result, HatchObject::VertexAndEdgeData::Omit) == NoError) {

				// Проходим по полигонам
				for (UInt32 i = 0; i < result.GetSize(); i++) {

					// Находим пересечения каждого полигона с линиями
					for (GS::HashTable<short, OrientedSegments>::PairIterator cIt = lines.EnumeratePairs(); cIt != NULL; ++cIt) {
						OrientedSegments& l = *cIt->value;
						GS::Array<Sector> resSectors;
						bool h = result[i].Intersect(l.cut_start, l.cut_direction, &resSectors);
						if (!resSectors.IsEmpty()) {
							for (UInt32 k = 0; k < resSectors.GetSize(); k++) {
								double fillThickL = resSectors[k].GetLength();
								double rfromstart = Geometry::Dist(l.start, resSectors[k].GetMidPoint()); // Расстояние до окружности(начала порядка слоёв)
#ifdef AC_27
								API_AttributeIndex	constrinxL = ACAPI_CreateAttributeIndex((GS::Int32)syHatch.GetBuildMatIdx().ToGSAttributeIndex());
#else
								API_AttributeIndex	constrinxL = (API_AttributeIndex)syHatch.GetBuildMatIdx();
#endif
								ParamValueComposite layer = {};
								layer.inx = constrinxL;
								layer.fillThick = fillThickL;
								layer.rfromstart = rfromstart;
								param_composite.Get(*cIt->key).composite.Push(layer);
								if (!existsmaterial.ContainsKey(constrinxL)) {
									ParamHelpers::GetAttributeValues(constrinxL, params, paramsAdd);
									existsmaterial.Add(constrinxL, true);
								}
								hasData = true;
							}
						}
					}
				}
			}
			else {
				DBPrintf("== SMSTF ERR == syHatch.ToPolygon2D ====================\n");
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
		ParamHelpers::CompareParamDictValue(paramlayers, params);
	}
	return hasData;
}
#endif

// --------------------------------------------------------------------
// Вытаскивает всё, что может, из информации о составе элемента
// --------------------------------------------------------------------
bool ParamHelpers::Components(const API_Element & element, ParamDictValue & params, ParamDictValue & paramsAdd) {
	DBPrintf("== SMSTF ==          Components\n");
	API_ModelElemStructureType	structtype = {};
	API_AttributeIndex			constrinx = {};
	double						fillThick = 0;

	// Отделка колонн
	API_AttributeIndex			constrinx_ven = {};
	double						fillThick_ven = 0;
	API_Elem_Head elemhead = element.header;
	GS::HashTable<API_AttributeIndex, bool> existsmaterial; // Словарь с уже прочитанными материалами

	// Получаем данные о составе конструкции. Т.к. для разных типов элементов
	// информация храница в разных местах - запишем всё в одни переменные
	API_ElemTypeID eltype;
#if defined AC_26 || defined AC_27
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
					constrinx_ven = memo.columnSegments[0].venBuildingMaterial;
					fillThick_ven = memo.columnSegments[0].venThick;
				}
				if (structtype == API_ProfileStructure) constrinx = memo.columnSegments[0].assemblySegmentData.profileAttr;
			}
			else {
				msg_rep("materialString::Components", "ACAPI_Element_GetMemo - ColumnSegment", err, element.header.guid);
				return false;
			}
		}
		else {
			msg_rep("materialString::Components", "Multisegment column not supported", NoError, element.header.guid);
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
				msg_rep("materialString::Components", "ACAPI_Element_GetMemo - BeamSegment", err, element.header.guid);
				return false;
			}
		}
		else {
			msg_rep("materialString::Components", "Multisegment beam not supported", NoError, element.header.guid);
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
			if (param.rawName.Contains("{@material:layers")) {
				paramlayers.Add(param.rawName, param);
			}
		}
	}

	// Если ничего нет - слои нам всё равно нужны
	if (paramlayers.IsEmpty()) {
		ParamValue param_composite = {};
		param_composite.fromGuid = element.header.guid;
		param_composite.isValid = true;
		paramlayers.Add("{@material:layers,20}", param_composite);
	}

	bool hasData = false;

	// Получим индексы строительных материалов и толщины
	if (structtype == API_BasicStructure) {
		hasData = ParamHelpers::ComponentsBasicStructure(constrinx, fillThick, constrinx_ven, fillThick_ven, params, paramlayers, paramsAdd);
	}
	if (structtype == API_CompositeStructure) hasData = ParamHelpers::ComponentsCompositeStructure(elemhead.guid, constrinx, params, paramlayers, paramsAdd, existsmaterial);
#ifndef AC_23
	if (structtype == API_ProfileStructure) {
		API_ElementMemo	memo = {};
		UInt64 mask = APIMemoMask_StretchedProfile;
		GSErrCode err = ACAPI_Element_GetMemo(elemhead.guid, &memo, mask);
		if (err != NoError) {
			ACAPI_DisposeElemMemoHdls(&memo);
			return false;
		}
		ProfileVectorImage profileDescription = *memo.stretchedProfile;
		hasData = ParamHelpers::ComponentsProfileStructure(profileDescription, params, paramlayers, paramsAdd, existsmaterial);
		ACAPI_DisposeElemMemoHdls(&memo);
	}
#endif
	return hasData;
}

// --------------------------------------------------------------------
// Заполнение данных для одного слоя
// --------------------------------------------------------------------
bool ParamHelpers::GetAttributeValues(const API_AttributeIndex & constrinx, ParamDictValue & params, ParamDictValue & paramsAdd) {
	API_Attribute	attrib = {};
	GS::UniString name = "";
	BNZeroMemory(&attrib, sizeof(API_Attribute));
	attrib.header.typeID = API_BuildingMaterialID;
	attrib.header.index = constrinx;
	attrib.header.uniStringNamePtr = &name;
	GS::UniString attribsuffix = GS::UniString::Printf("%d", constrinx);
	GSErrCode error = ACAPI_Attribute_Get(&attrib);
	if (error != NoError) {
		msg_rep("materialString::GetAttributeValues", "ACAPI_Attribute_Get", error, APINULLGuid);
		return false;
	};

	if (params.ContainsKey("{@property:bmat}")) {
		ParamValue pvalue_bmat;
		pvalue_bmat.rawName = "{@material:bmat_inx}";
		pvalue_bmat.rawName.ReplaceAll("}", CharENTER + attribsuffix + "}");
		pvalue_bmat.name = "bmat_inx";
		pvalue_bmat.name = pvalue_bmat.name + CharENTER + attribsuffix;
#ifdef AC_27
		ParamHelpers::ConvertToParamValue(pvalue_bmat, pvalue_bmat.name, constrinx.ToInt32_Deprecated());
#else
		ParamHelpers::ConvertToParamValue(pvalue_bmat, pvalue_bmat.name, (Int32)constrinx);
#endif
		pvalue_bmat.fromMaterial = true;
		ParamHelpers::AddParamValue2ParamDict(params.Get("{@property:bmat}").fromGuid, pvalue_bmat, params);
	}

	// Определения и свойста для элементов
	bool flag_find = false;
	GS::Array<API_PropertyDefinition> propertyDefinitions;
	for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = params.EnumeratePairs(); cIt != NULL; ++cIt) {
		ParamValue& param = *cIt->value;
		if (param.fromAttribDefinition) {
			bool flag_add = true;

			// Если в списке есть штриховка или покрытие - получим их имена.
			if (param.rawName.Contains("buildingmaterialproperties/building material cutfill")) {
				GS::UniString namet = "";
				API_Attribute	attribt = {};
				BNZeroMemory(&attribt, sizeof(API_Attribute));
				attribt.header.typeID = API_FilltypeID;
				attribt.header.index = attrib.buildingMaterial.cutFill;
				attribt.header.uniStringNamePtr = &namet;
				error = ACAPI_Attribute_Get(&attribt);
				ParamValue pvalue;
				GS::UniString rawName = param.rawName;
				rawName.ReplaceAll("}", CharENTER + attribsuffix + "}");
				pvalue.name = param.name + CharENTER + attribsuffix;
				pvalue.rawName = rawName;
				ParamHelpers::ConvertToParamValue(pvalue, pvalue.name, namet);
				pvalue.fromMaterial = true;
				ParamHelpers::AddParamValue2ParamDict(param.fromGuid, pvalue, params);
				flag_add = false;
				flag_find = true;
			}
			if (flag_add) {
				API_PropertyDefinition definition = param.definition;
				if (!definition.name.Contains(CharENTER)) propertyDefinitions.Push(definition);
			}
		}
	}
	if (!propertyDefinitions.IsEmpty()) {
		GS::Array<API_Property> properties;
		error = ACAPI_Attribute_GetPropertyValues(attrib.header, propertyDefinitions, properties);
		if (error != NoError) {
			msg_rep("materialString::GetAttributeValues", "ACAPI_Attribute_GetPropertyValues", error, APINULLGuid);
			return flag_find;
		};
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
		return (ParamHelpers::AddProperty(params, properties) || flag_find);
	}
	return flag_find;
	}