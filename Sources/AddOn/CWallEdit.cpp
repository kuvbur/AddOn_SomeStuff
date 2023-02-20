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
		if (true) {
			DGAlert(DG_ERROR, "Error", errorString, "", "Ok");
		}
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
	GS::Array<API_Box3D> elemsCoord = {};
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
			if (err != NoError) elemsCoord.Push(coord);
		}
	}
	if (!cWallguidArray.IsEmpty() && !elemsCoord.IsEmpty()) Do_ChangeCWallWithUndo(cWallguidArray, elemsCoord);
}

void Do_ChangeCWallWithUndo(const GS::Array<API_Guid>& elemsGuid, const GS::Array<API_Box3D>& elemsCoord) {
	GS::UniString undoString = RSGetIndString(AddOnStringsID, UndoSyncId, ACAPI_GetOwnResModule());
	ACAPI_CallUndoableCommand(undoString, [&]() -> GSErrCode {
		for (UInt32 i = 0; i < elemsGuid.GetSize(); i++) {
			Do_ChangeCWall(elemsGuid.Get(i), elemsCoord);
		}
		return NoError;
							  });
}

void Do_ChangeCWall(const API_Guid& elemGuid, const  GS::Array<API_Box3D>& elemsCoord) {
	API_Element element = {};
	API_ElementMemo	memo = {};
	BNZeroMemory(&element, sizeof(API_Element));
	BNZeroMemory(&memo, sizeof(API_ElementMemo));
	element.header.guid = elemGuid;
	GSErrCode err = ACAPI_Element_Get(&element);
	if (err != NoError) return;
	err = ACAPI_Element_GetMemo(element.header.guid, &memo, APIMemoMask_CWallFrames);
	if (err != NoError) {
		ACAPI_DisposeElemMemoHdls(&memo);
		return;
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

	//ACAPI_ELEMENT_MASK_SETFULL(mask);
	//err = ACAPI_Element_Change(&element, &mask, &memo, APIMemoMask_CWallFrames, true);
	ACAPI_DisposeElemMemoHdls(&memo);
	if (err != NoError) {
		return;
	}
}