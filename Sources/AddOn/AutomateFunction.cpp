//------------ kuvbur 2022 ------------
#include	"APIEnvir.h"
#include	"ACAPinc.h"
#include	"Helpers.hpp"
#include	"AutomateFunction.hpp"


namespace AutoFunc {
	void KM_ListUpdate(){
		GS::Array<API_Guid> guidArray = GetSelectedElements(true, true, false);
		GS::Array<API_Guid> elements;
		GS::Array<API_Guid> lines;
		for(UInt32 i = 0; i < guidArray.GetSize(); i++) {
			API_Elem_Head elem_head;
			elem_head.guid = guidArray[i];
			if(ACAPI_Element_GetHeader(&elem_head) == NoError) {
				API_ElemTypeID elementType = elem_head.typeID;
				if (elementType == API_ObjectID) elements.Push(guidArray[i]);
				if (elementType == API_PolyLineID || elementType == API_LineID) lines.Push(guidArray[i]);
			}
		}
		if (elements.IsEmpty() || lines.IsEmpty()) return;
		GS::Array<API_Coord> coords;
		for(UInt32 i = 0; i < lines.GetSize(); i++) {
			API_Element element = {};
			BNZeroMemory(&element, sizeof(API_Element));
			element.header.guid = lines[i];
			if(ACAPI_Element_Get(&element) == NoError) {
				coords.Push(element.line.begC);
			}
		}
		ACAPI_CallUndoableCommand("undoString", [&] () -> GSErrCode {
			GSErrCode err = WriteGDLValues(elements.Get(0), coords);

			return err;
			});
	}

	GSErrCode WriteGDLValues(API_Guid elemGuid, GS::Array<API_Coord>& coords) {
		if(coords.IsEmpty()) return APIERR_GENERAL;
		if(elemGuid == APINULLGuid) return APIERR_GENERAL;
		GSErrCode err = NoError;
		API_Elem_Head elem_head = {};
		API_Element element = {};
		API_Element mask = {};
		elem_head.guid = elemGuid;
		err = ACAPI_Element_GetHeader(&elem_head); if(err != NoError) return err;
		element.header.guid = elemGuid;
		err = ACAPI_Element_Get(&element); if(err != NoError) return err;
		API_ParamOwnerType	apiOwner = {};
		API_GetParamsType	apiParams = {};
		BNZeroMemory(&apiOwner, sizeof(API_ParamOwnerType));
		BNZeroMemory(&apiParams, sizeof(API_GetParamsType));
		apiOwner.guid = elemGuid;
		apiOwner.typeID = elem_head.typeID;
		err = ACAPI_Goodies(APIAny_OpenParametersID, &apiOwner, nullptr);
		if(err != NoError) {
			ACAPI_Goodies(APIAny_CloseParametersID, nullptr, nullptr);
			return err;
		}
		err = ACAPI_Goodies(APIAny_GetActParametersID, &apiParams);
		if(err != NoError) {
			ACAPI_Goodies(APIAny_CloseParametersID, nullptr, nullptr);
			return err;
		}
		Int32 n_t = coords.GetSize();
		Int32 inx_kontur = 0;
		API_ChangeParamType	chgParam;
		Int32 addParNum = BMGetHandleSize((GSHandle) apiParams.params) / sizeof(API_AddParType);
		for(Int32 i = 0; i < addParNum; ++i) {
			GS::UniString name = (*apiParams.params)[i].name;
			if(name.IsEqual("kontur") && (*apiParams.params)[i].typeMod == API_ParArray) {
				inx_kontur = i;
				BNZeroMemory(&chgParam, sizeof(API_ChangeParamType));
				chgParam.index = (*apiParams.params)[i].index;
				CHTruncate((*apiParams.params)[i].name, chgParam.name, API_NameLen);
				chgParam.realValue = 0;
				chgParam.ind1 = (*apiParams.params)[i].dim1;
				chgParam.ind2 = (*apiParams.params)[i].dim2;
				err = ACAPI_Goodies(APIAny_ChangeAParameterID, &chgParam, nullptr); if(err != NoError) return err;
			}
			if(name.IsEqual("n_t")) {
				BNZeroMemory(&chgParam, sizeof(API_ChangeParamType));
				chgParam.index = (*apiParams.params)[i].index;
				CHTruncate((*apiParams.params)[i].name, chgParam.name, API_NameLen);
				chgParam.realValue = n_t;
				err = ACAPI_Goodies(APIAny_ChangeAParameterID, &chgParam, nullptr); if(err != NoError) return err;
			}
		}
		err = ACAPI_Goodies(APIAny_GetActParametersID, &apiParams); if(err != NoError) return err;
		err = ACAPI_Goodies(APIAny_CloseParametersID, nullptr, nullptr); if(err != NoError) return err;

		Int32 inDim1 = (*apiParams.params)[inx_kontur].dim1;
		Int32 inDim2 = (*apiParams.params)[inx_kontur].dim2;
		size_t ind = 0;
		double** newArrHdl = (double**)(*apiParams.params)[inx_kontur].value.array;
		for(Int32 i = 0; i < inDim1; i++) {
			if (i < n_t){
				(*newArrHdl)[ind] = coords.Get(i).x;
				ind++;
				(*newArrHdl)[ind] = coords.Get(i).y;
				ind++;
			} else{
				(*newArrHdl)[ind] = 0;
				ind++;
				(*newArrHdl)[ind] = 0;
				ind++;
			}
		}
		API_ElementMemo	elemMemo = {};
		BNZeroMemory(&elemMemo, sizeof(elemMemo));
		(*apiParams.params)[inx_kontur].value.array = (GSHandle) newArrHdl;
		elemMemo.params = apiParams.params;
		ACAPI_ELEMENT_MASK_CLEAR(mask);
		err = ACAPI_Element_Change(&element, &mask, &elemMemo, APIMemoMask_AddPars, true);
		ACAPI_DisposeAddParHdl(&apiParams.params);
		return err;
	}
}