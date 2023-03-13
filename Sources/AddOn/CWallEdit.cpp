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
	GS::Array<SegmentBox> elems = {};
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
				SegmentBox sg;

				// Каждая сторона будет плоскостью. Всего плоскостей 4. Затем мы будлем искать линию пересечения это плоскости с плоскостью навесной стены.
				Geometry::Plane out_p;
				GS::PagedArray<Point3D> points;
				Point3D p1; p1.Set(coord.xMin, coord.yMin, coord.zMin); points.Push(p1);
				Point3D p2; p2.Set(coord.xMin, coord.yMax, coord.zMin); points.Push(p2);
				Point3D p3; p3.Set(coord.xMin, coord.yMax, coord.zMax); points.Push(p3);
				Point3D p4; p4.Set(coord.xMin, coord.yMin, coord.zMax); points.Push(p4);
				if (Geometry::CreatePlane(points, out_p)) sg.edges.Push(out_p);
				if (!sg.edges.IsEmpty()) elems.Push(sg);
			}
		}
	}
	if (!cWallguidArray.IsEmpty() && !elems.IsEmpty()) Do_ChangeCWallWithUndo(cWallguidArray, elems);
}

void Do_ChangeCWallWithUndo(const GS::Array<API_Guid>& elemsGuid, const GS::Array<SegmentBox>& elems) {
	GS::UniString undoString = RSGetIndString(AddOnStringsID, UndoSyncId, ACAPI_GetOwnResModule());
	ACAPI_CallUndoableCommand(undoString, [&]() -> GSErrCode {
		for (UInt32 i = 0; i < elemsGuid.GetSize(); i++) {
			Do_ChangeCWall(elemsGuid.Get(i), elems);
		}
		return NoError;
							  });
}

void Do_ChangeCWall(const API_Guid& elemGuid, const  GS::Array<SegmentBox>& elems) {
	API_Element element = {};
	API_ElementMemo	memo = {};
	BNZeroMemory(&element, sizeof(API_Element));
	BNZeroMemory(&memo, sizeof(API_ElementMemo));
	element.header.guid = elemGuid;
	GSErrCode err = ACAPI_Element_Get(&element);
	if (err != NoError) return;
	err = ACAPI_Element_GetMemo(element.header.guid, &memo, APIMemoMask_CWallSegments | APIMemoMask_CWallFrames);
	if (err != NoError) {
		ACAPI_DisposeElemMemoHdls(&memo);
		return;
	}
	API_ElementMemo	memo_poly = {};
	for (UInt32 i = 0; i < element.curtainWall.nSegments; ++i) {
		const API_CWSegmentType& segment = memo.cWallSegments[i];
		BNZeroMemory(&memo_poly, sizeof(API_ElementMemo));
		err = ACAPI_Element_GetMemo(segment.head.guid, &memo_poly, APIMemoMask_CWSegContour);
		if (err != NoError) {
			ACAPI_DisposeElemMemoHdls(&memo_poly);
			return;
		}

		//Geometry::Plane cwplane;
		//GS::PagedArray<Point3D> points;
		//Point3D p1; p1.Set(coord.xMin, coord.yMin, coord.zMin); points.Push(p1);
		//Point3D p2; p2.Set(coord.xMin, coord.yMax, coord.zMin); points.Push(p2);
		//Point3D p3; p3.Set(coord.xMin, coord.yMax, coord.zMax); points.Push(p3);
		//Point3D p4; p4.Set(coord.xMin, coord.yMin, coord.zMax); points.Push(p4);
		//if (Geometry::CreatePlane(points, out_p)) sg.edges.Push(out_p);

		//for (UInt32 j = 0; j < segment.contourNum; ++j) {
			//const API_CWContourData& countur = memo_poly.cWSegContour[j];
			//for (Int32 k = 0; k < countur.polygon.nCoords; ++k) {
			//	double x = countur.coords[k]->x;
			//	double y = countur.coords[k]->y;
			//	int hh = 1;
			//}
		//}

		// Надо помнить, что система координат полигона навесной стены повёрнута.
		int hh = 1;
	}
	ACAPI_DisposeElemMemoHdls(&memo_poly);

	//ACAPI_ELEMENT_MASK_SETFULL(mask);
	//err = ACAPI_Element_Change(&element, &mask, &memo, APIMemoMask_CWallFrames, true);
	ACAPI_DisposeElemMemoHdls(&memo);
	if (err != NoError) {
		return;
	}
}