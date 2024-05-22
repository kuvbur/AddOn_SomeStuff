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
				if (elementType == API_PolyLineID || API_LineID) lines.Push(guidArray[i]);
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
		int hh=1;
	}
}