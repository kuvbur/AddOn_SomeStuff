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
				coords.Push(element.line.endC);
			}
		}
		ParamDictValue params;

	}

	bool WriteGDLValues(API_Guid elemGuid, ParamDictValue& params) {
		if(params.IsEmpty()) return false;
		if(elemGuid == APINULLGuid) return false;
		API_Elem_Head elem_head = {};
		elem_head.guid = elemGuid;
		if(ACAPI_Element_GetHeader(&elem_head) != NoError) return false;

		API_ParamOwnerType	apiOwner = {};
		API_GetParamsType	apiParams = {};
		apiOwner.guid = elemGuid;
		apiOwner.typeID = elem_head.typeID;
		if(ACAPI_Goodies(APIAny_OpenParametersID, &apiOwner, nullptr) != NoError) return false;
		if(ACAPI_Goodies(APIAny_GetActParametersID, &apiParams) != NoError) {
			ACAPI_Goodies(APIAny_CloseParametersID);
			return false;
		}

		API_ChangeParamType	chgParam;

		bool flagFind = false;
		Int32	addParNum = BMGetHandleSize((GSHandle) apiParams.params) / sizeof(API_AddParType);
		Int32 nfind = params.GetSize();
		for(Int32 i = 0; i < addParNum; ++i) {
			API_AddParType& actualParam = (*apiParams.params)[i];
			GS::UniString name = actualParam.name;
			GS::UniString rawname = "{@gdl:" + name.ToLowerCase() + "}";
			if(params.ContainsKey(rawname)) {
				ParamValueData paramfrom = params.Get(rawname).val;
				BNZeroMemory(&chgParam, sizeof(API_ChangeParamType));
				chgParam.index = actualParam.index;
				CHTruncate(actualParam.name, chgParam.name, API_NameLen);
				if(actualParam.typeID == APIParT_CString) {
					GS::uchar_t uStrBuffer[256];
					GS::ucsncpy(uStrBuffer, paramfrom.uniStringValue.ToUStr().Get(), 256);
					chgParam.uStrValue = uStrBuffer;
				}
				if(actualParam.typeID == APIParT_Integer) {
					chgParam.realValue = paramfrom.intValue;
				}
				if(actualParam.typeID == APIParT_Length) {
					chgParam.realValue = paramfrom.doubleValue;
				}
				if(actualParam.typeID == APIParT_Angle) {
					chgParam.realValue = paramfrom.doubleValue;
				}
				if(actualParam.typeID == APIParT_RealNum) {
					chgParam.realValue = paramfrom.doubleValue;
				}
				if(actualParam.typeID == APIParT_Boolean) {
					chgParam.realValue = paramfrom.boolValue;
				}
				if(ACAPI_Goodies(APIAny_ChangeAParameterID, &chgParam, nullptr) != NoError) return false;
			}
		}

		if(ACAPI_Goodies(APIAny_GetActParametersID, &apiParams) != NoError) return false;
		if(ACAPI_Goodies(APIAny_CloseParametersID) != NoError) return false;

		API_ElementMemo	elemMemo = {};
		elemMemo.params = apiParams.params;
		GSErrCode err = ACAPI_Element_ChangeMemo(elemGuid, APIMemoMask_AddPars, &elemMemo);
		ACAPI_DisposeAddParHdl(&apiParams.params);
		return (err == NoError);
	}
}