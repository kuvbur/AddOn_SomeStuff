#include	"APIEnvir.h"
#include	"ACAPinc.h"
#include	"Dimensions.hpp"
#include	"Helpers.hpp"


GSErrCode DimAddGrid(void) {
	GSErrCode err = NoError;
//
//
//	API_Element element = {};
//	API_ElementMemo memo = {};
//
//	element.header.typeID = API_DimensionID;
//	err = ACAPI_Element_GetDefaults(&element, &memo);
//	if (err != NoError) {
//		return err;
//	}
//
//	element.dimension.dimAppear = APIApp_Normal;
//	element.dimension.textPos = APIPos_Above;
//	element.dimension.textWay = APIDir_Parallel;
//	element.dimension.defStaticDim = false;
//	element.dimension.usedIn3D = false;
//	element.dimension.horizontalText = false;
//	element.dimension.refC = refPoint;
//	element.dimension.direction = { 1, 0 };
//
//	element.dimension.nDimElem = 1;
//
//	//GS::UniString undoString = RSGetIndString(AddOnStringsID, UndoDimGridId, ACAPI_GetOwnResModule());
//	//ACAPI_CallUndoableCommand(undoString, [&]() -> GSErrCode {
//
//	//	API_Element     element;
//	//	API_ElementMemo memo;
//	//	GSErrCode       err;
//
//	//	BNZeroMemory(&element, sizeof(API_Element));
//	//	BNZeroMemory(&memo, sizeof(API_ElementMemo));
//
//	//	element.header.typeID = API_ObjectID;
//	//	element.header.variationID = APIVarId_SymbStair;
//	//	err = ACAPI_Element_GetDefaults(&element, &memo);
//	//	if (err == NoError) {
//	//		/* do what you want */
//	//	}
//
//	ACAPI_DisposeElemMemoHdls(&memo);
//
//
//	//	return err;
//	//	});
	return err;
}
