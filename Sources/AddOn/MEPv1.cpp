//------------ kuvbur 2022 ------------
#include	"APIEnvir.h"
#include	"ACAPinc.h"
#include	"MEPv1.hpp"

using namespace ACAPI::MEP;

namespace MEPv1 {
	void test_mep(const API_Guid& elemGuid) {
		GSErrCode		err = NoError;
		API_Elem_Head elem_head;
		BNZeroMemory(&elem_head, sizeof(API_Elem_Head));
		elem_head.guid = elemGuid;
		err = ACAPI_Element_GetHeader(&elem_head);
		if (err != NoError) {
			return;
		}
		if (elem_head.type.typeID != API_ExternalElemID) return;
		GS::UniString txttype = "";
		if (IsBranch(elem_head.type.classID)) {
			txttype = "IsBranch";
		}
		if (IsRoutingElement(elem_head.type.classID)) {
			UniqueID id = Adapter::UniqueID(elemGuid);
			ACAPI::Result<RoutingElement> routingElement = RoutingElement::Get(id);
			if (routingElement.IsErr()) {
				ACAPI_WriteReport(routingElement.UnwrapErr().text.c_str(), false);
				return;
			}
			std::vector<UniqueID> routingNodeIds = routingElement->GetRoutingNodeIds();
			std::vector<UniqueID> routingSegmentIds = routingElement->GetRoutingSegmentIds();
			txttype = "IsRoutingElement";
		}
		if (IsAccessory(elem_head.type.classID)) {
			txttype = "IsAccessory";
		}
		if (IsBend(elem_head.type.classID)) {
			txttype = "IsBend";
		}
		if (IsEquipment(elem_head.type.classID)) {
			txttype = "IsEquipment";
		}
		if (IsFitting(elem_head.type.classID)) {
			txttype = "IsFitting";
		}
		if (IsFlexibleSegment(elem_head.type.classID)) {
			txttype = "IsFlexibleSegment";
		}
		if (IsRigidSegment(elem_head.type.classID)) {
			txttype = "IsRigidSegment";
		}
		if (IsTransition(elem_head.type.classID)) {
			txttype = "IsTransition";
		}
		if (IsTerminal(elem_head.type.classID)) {
			txttype = "IsTerminal";
		}
		GS::UniString txttype1 = "";
	}
}