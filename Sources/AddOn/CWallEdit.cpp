//------------ kuvbur 2022 ------------
#include	"APIEnvir.h"
#include	"ACAPinc.h"
#include	"CWallEdit.hpp"

void AddHoleToSelectedCWall(const SyncSettings& syncSettings) {
	GSErrCode            err;
	API_SelectionInfo    selectionInfo;
	GS::UniString errorString = RSGetIndString(AddOnStringsID, ErrorSelectID, ACAPI_GetOwnResModule());
#ifdef AC_22
	API_Neig** selNeigs;
#else
	GS::Array<API_Neig>  selNeigs;
#endif
	err = ACAPI_Selection_Get(&selectionInfo, &selNeigs, true);
	BMKillHandle((GSHandle*)&selectionInfo.marquee.coords);
	if (err == APIERR_NOSEL || selectionInfo.typeID == API_SelEmpty) {
		DGAlert(DG_ERROR, "Error", errorString, "", "Ok");
		return;
	}
	if (err != NoError) {
#ifdef AC_22
		BMKillHandle((GSHandle*)&selNeigs);
#endif // AC_22
		return;
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
#endif // AC_22
	if (guidArray.IsEmpty()) return;
	GS::Array<API_Guid> cWallguidArray;
	GS::Array<Geometry::Polygon3D> elems = {};
	for (UInt32 i = 0; i < guidArray.GetSize(); i++) {
		API_Elem_Head elementHead;
		BNZeroMemory(&elementHead, sizeof(API_Elem_Head));
		elementHead.guid = guidArray[i];
		GSErrCode err = ACAPI_Element_GetHeader(&elementHead);
		bool needGetElem = false;
		if (err != NoError) return;
		switch (elementHead.typeID)
		{
		case API_CurtainWallID:
			cWallguidArray.Push(guidArray[i]);
			break;
		case API_WallID:
			needGetElem = true;
			break;
		case API_SlabID:
			needGetElem = true;
			break;
		default:
			break;
		}
		if (needGetElem) {
			API_Box3D coord;
			err = ACAPI_Database(APIDb_CalcBoundsID, &elementHead, &coord);
			if (err == NoError) {
				GS::PagedArray<Point3D> points;
				Point3D p1; p1.Set(coord.xMin, coord.yMin, coord.zMin); points.Push(p1);
				Point3D p2; p2.Set(coord.xMax, coord.yMax, coord.zMin); points.Push(p2);
				Point3D p3; p3.Set(coord.xMax, coord.yMax, coord.zMax); points.Push(p3);
				Point3D p4; p4.Set(coord.xMin, coord.yMin, coord.zMax); points.Push(p4);
				Geometry::Polygon3D polytyn = CreatePolygon3D(points);
				elems.Push(polytyn);
			}
		}
	}
	if (!cWallguidArray.IsEmpty() && !elems.IsEmpty()) Do_ChangeCWallWithUndo(cWallguidArray, elems);
}

void Do_ChangeCWallWithUndo(const GS::Array<API_Guid>& elemsGuid, const GS::Array<Geometry::Polygon3D>& elems) {
	GS::UniString undoString = RSGetIndString(AddOnStringsID, UndoSyncId, ACAPI_GetOwnResModule());
	ACAPI_CallUndoableCommand(undoString, [&]() -> GSErrCode {
		for (UInt32 i = 0; i < elemsGuid.GetSize(); i++) {
			Do_ChangeCWall(elemsGuid.Get(i), elems);
		}
		return NoError;
							  });
}

void Do_ChangeCWall(const API_Guid& elemGuid, const  GS::Array<Geometry::Polygon3D>& elems) {
	API_Element element = {};
	API_ElementMemo	memo = {};
	BNZeroMemory(&element, sizeof(API_Element));
	BNZeroMemory(&memo, sizeof(API_ElementMemo));
	element.header.guid = elemGuid;
	GSErrCode err = ACAPI_Element_Get(&element);
	if (err != NoError) return;
	err = ACAPI_Element_GetMemo(element.header.guid, &memo);
	if (err != NoError) {
		ACAPI_DisposeElemMemoHdls(&memo);
		return;
	}
	API_ElementMemo	memo_poly = {};
	for (UInt32 i = 0; i < element.curtainWall.nSegments; ++i) {
		API_CWSegmentType& segment = memo.cWallSegments[i];
		Sector3D baseLine = GetSector3DFromCoord(segment.begC, segment.endC);
		BNZeroMemory(&memo_poly, sizeof(API_ElementMemo));
		err = ACAPI_Element_GetMemo(segment.head.guid, &memo_poly, APIMemoMask_CWSegContour);
		if (err != NoError || !memo.coords) {
			ACAPI_DisposeElemMemoHdls(&memo_poly);
			return;
		}
		GS::PagedArray<Point3D> points;
		for (UInt32 j = 0; j < segment.contourNum; ++j) {
			const API_CWContourData& countur = memo_poly.cWSegContour[j];
			for (Int32 k = 0; k < countur.polygon.nCoords; ++k) {
				double x = (*countur.coords)[k].x;
				double y = (*countur.coords)[k].y;
				Point3D p; p.Set(x, 0, y);
				TransformPoint3D(baseLine, p);
				if (!points.Contains(p)) points.Push(p);
			}
		}
		Geometry::Polygon3D polywall = CreatePolygon3D(points);
		Geometry::Polygon3D rezult;
		bool hasIntersect = IntersectPolygon3D(polywall, elems[0], rezult);
		if (hasIntersect) {
			const Int32 nCustomFrames = rezult.GetCoord3DCount();
			const UInt32 nFrameClasses = BMpGetSize(reinterpret_cast<GSPtr> (memo.cWallFrameDefaults)) / sizeof(API_CWFrameType);
			memo.cWallFrames = reinterpret_cast<API_CWFrameType*> (BMpAll(sizeof(API_CWFrameType) * nCustomFrames));
			for (Int32 ii = 0; ii < nCustomFrames; ++ii) {
				Sector3D s;
				rezult.GetSector3D(ii, s);
				memo.cWallFrames[ii] = memo.cWallFrameDefaults[nFrameClasses - 1];
				memo.cWallFrames[ii].begC.x = 0.2;//s.c1.x;
				memo.cWallFrames[ii].begC.y = 0.3; //s.c1.z;
				memo.cWallFrames[ii].endC.x = 0.5;//s.c2.x;
				memo.cWallFrames[ii].endC.y = 0.6;// s.c2.z;
				memo.cWallFrames[ii].segmentID = 0;
				memo.cWallFrames[ii].cellID = 0;
			}
		}
	}
	ACAPI_DisposeElemMemoHdls(&memo_poly);

	{
		const UInt32 nFrameClasses = BMpGetSize(reinterpret_cast<GSPtr> (memo.cWallFrameDefaults)) / sizeof(API_CWFrameType);
		DBASSERT(nFrameClasses > 0);
		const UInt32 nCustomFrames = 8 * element.curtainWall.nSegments;
		memo.cWallFrames = reinterpret_cast<API_CWFrameType*> (BMpAll(sizeof(API_CWFrameType) * nCustomFrames));
		for (UInt32 i = 0; i < nCustomFrames; ++i) {
			memo.cWallFrames[i] = memo.cWallFrameDefaults[nFrameClasses - 1];
			memo.cWallFrames[i].segmentID = i / 8;
			memo.cWallFrames[i].cellID = 0;
		}
		for (UInt32 i = 0; i < element.curtainWall.nSegments; ++i) {
			memo.cWallFrames[0 + i * 8].endRel.x = memo.cWallFrames[1 + i * 8].begRel.x = 0.2;
			memo.cWallFrames[0 + i * 8].endRel.y = memo.cWallFrames[1 + i * 8].begRel.y = 0.5;
			memo.cWallFrames[1 + i * 8].endRel.x = memo.cWallFrames[2 + i * 8].begRel.x = 0.4;
			memo.cWallFrames[1 + i * 8].endRel.y = memo.cWallFrames[2 + i * 8].begRel.y = 0.6;
			memo.cWallFrames[2 + i * 8].endRel.x = memo.cWallFrames[3 + i * 8].begRel.x = 0.5;
			memo.cWallFrames[2 + i * 8].endRel.y = memo.cWallFrames[3 + i * 8].begRel.y = 0.8;
			memo.cWallFrames[3 + i * 8].endRel.x = memo.cWallFrames[4 + i * 8].begRel.x = 0.6;
			memo.cWallFrames[3 + i * 8].endRel.y = memo.cWallFrames[4 + i * 8].begRel.y = 0.6;
			memo.cWallFrames[4 + i * 8].endRel.x = memo.cWallFrames[5 + i * 8].begRel.x = 0.8;
			memo.cWallFrames[4 + i * 8].endRel.y = memo.cWallFrames[5 + i * 8].begRel.y = 0.5;
			memo.cWallFrames[5 + i * 8].endRel.x = memo.cWallFrames[6 + i * 8].begRel.x = 0.6;
			memo.cWallFrames[5 + i * 8].endRel.y = memo.cWallFrames[6 + i * 8].begRel.y = 0.4;
			memo.cWallFrames[6 + i * 8].endRel.x = memo.cWallFrames[7 + i * 8].begRel.x = 0.5;
			memo.cWallFrames[6 + i * 8].endRel.y = memo.cWallFrames[7 + i * 8].begRel.y = 0.2;
			memo.cWallFrames[7 + i * 8].endRel.x = memo.cWallFrames[0 + i * 8].begRel.x = 0.4;
			memo.cWallFrames[7 + i * 8].endRel.y = memo.cWallFrames[0 + i * 8].begRel.y = 0.4;
		}
	}

	API_Element mask = {};
	ACAPI_ELEMENT_MASK_SETFULL(mask);
	err = ACAPI_Element_Change(&element, &mask, &memo, APIMemoMask_CWallFrames, true);
	ACAPI_DisposeElemMemoHdls(&memo);
	if (err != NoError) {
		return;
	}
}