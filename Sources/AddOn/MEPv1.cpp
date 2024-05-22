//------------ kuvbur 2022 ------------
#ifdef AC_27
#include	"APIEnvir.h"
#include	"ACAPinc.h"
#include	"MEPv1.hpp"
using namespace ACAPI::MEP;

API_Guid GetRigidSegmentClassIDFromRoutingElemClassID(const API_Guid& routingElemClassID)
{
	if (routingElemClassID == ACAPI::MEP::VentilationRoutingID)
		return ACAPI::MEP::VentilationRigidSegmentID;

	if (routingElemClassID == ACAPI::MEP::PipingRoutingID)
		return ACAPI::MEP::PipingRigidSegmentID;

	if (routingElemClassID == ACAPI::MEP::CableCarrierRoutingID)
		return ACAPI::MEP::CableCarrierRigidSegmentID;

	return APINULLGuid;
}

API_Guid GetBendClassIDFromRoutingElemClassID(const API_Guid& routingElemClassID)
{
	if (routingElemClassID == ACAPI::MEP::VentilationRoutingID)
		return ACAPI::MEP::VentilationBendID;

	if (routingElemClassID == ACAPI::MEP::PipingRoutingID)
		return ACAPI::MEP::PipingBendID;

	if (routingElemClassID == ACAPI::MEP::CableCarrierRoutingID)
		return ACAPI::MEP::CableCarrierBendID;

	return APINULLGuid;
}

API_Guid GetTransitionClassIDFromRoutingElemClassID(const API_Guid& routingElemClassID)
{
	if (routingElemClassID == ACAPI::MEP::VentilationRoutingID)
		return ACAPI::MEP::VentilationTransitionID;

	if (routingElemClassID == ACAPI::MEP::PipingRoutingID)
		return ACAPI::MEP::PipingTransitionID;

	if (routingElemClassID == ACAPI::MEP::CableCarrierRoutingID)
		return ACAPI::MEP::CableCarrierTransitionID;

	return APINULLGuid;
}

namespace MEPv1 {
	void GetSubElementOfRouting(const API_Guid& elemGuid, GS::Array<API_Guid>& subelemGuid) {
		ACAPI::Result<RoutingElement> routingElement = RoutingElement::Get(Adapter::UniqueID(elemGuid));
		if (routingElement.IsErr()) {
			ACAPI_WriteReport(routingElement.UnwrapErr().text.c_str(), false);
			return;
		}
		std::vector<UniqueID> routingNodeIds = routingElement->GetRoutingNodeIds();
		std::vector<UniqueID> routingSegmentIds = routingElement->GetRoutingSegmentIds();

		API_ElemType mepElemType;
		mepElemType.typeID = API_ExternalElemID;
		mepElemType.variationID = APIVarId_Generic;

		//if (!routingNodeIds.empty()) {
		//	for (UInt32 inx_segment = 0; inx_segment < routingNodeIds.size(); ++inx_segment) {
		//		ACAPI::Result<RoutingSegment> routingSegment = RoutingSegment::Get(routingSegmentIds[inx_segment]);
		//		if (routingSegment.IsErr()) {
		//			ACAPI_WriteReport(routingSegment.UnwrapErr().text.c_str(), false);
		//			return;
		//		}
		//		API_Guid rguid = GSGuid2APIGuid(routingSegmentIds[inx_segment].GetGuid());
		//		subelemGuid.Push(rguid);
		//		std::vector<UniqueID> rigidSegmentIds = routingSegment->GetRigidSegmentIds();
		//		if (!rigidSegmentIds.empty()) {
		//			for (UInt32 inx_rigid = 0; inx_rigid < rigidSegmentIds.size(); ++inx_rigid) {
		//				API_Guid rguid = GSGuid2APIGuid(rigidSegmentIds[inx_rigid].GetGuid());
		//				subelemGuid.Push(rguid);
		//			}
		//		}
		//	}
		//}

		if (!routingSegmentIds.empty()) {
			for (UInt32 inx_segment = 0; inx_segment < routingSegmentIds.size(); ++inx_segment) {
				ACAPI::Result<RoutingSegment> routingSegment = RoutingSegment::Get(routingSegmentIds[inx_segment]);
				if (routingSegment.IsErr()) {
					ACAPI_WriteReport(routingSegment.UnwrapErr().text.c_str(), false);
					return;
				}
				API_Guid rguid = GSGuid2APIGuid(routingSegmentIds[inx_segment].GetGuid());
				subelemGuid.Push(rguid);
				std::vector<UniqueID> rigidSegmentIds = routingSegment->GetRigidSegmentIds();
				if (!rigidSegmentIds.empty()) {
					for (UInt32 inx_rigid = 0; inx_rigid < rigidSegmentIds.size(); ++inx_rigid) {
						API_Guid rguid = GSGuid2APIGuid(rigidSegmentIds[inx_rigid].GetGuid());
						subelemGuid.Push(rguid);
					}
				}
			}
		}
	}
	void GetRouting(const API_Guid& elemGuid) {
	}

	void GetSubElement(const API_Guid& elemGuid, GS::Array<API_Guid>& subelemGuid) {
		GSErrCode		err = NoError;

		API_Elem_Head elem_head;
		BNZeroMemory(&elem_head, sizeof(API_Elem_Head));
		elem_head.guid = elemGuid;
		err = ACAPI_Element_GetHeader(&elem_head);
		if (elem_head.type.typeID != API_ExternalElemID) return;
		GS::UniString txttype = "";
		if (IsRoutingElement(elem_head.type.classID)) {
			GetSubElementOfRouting(elemGuid, subelemGuid);
			return;
		}
		if (IsBranch(elem_head.type.classID)) {
			return;
		}
		if (IsAccessory(elem_head.type.classID)) {
			return;
		}
		if (IsEquipment(elem_head.type.classID)) {
			return;
		}
		if (IsTerminal(elem_head.type.classID)) {
			return;
		}
		if (IsFitting(elem_head.type.classID)) {
			return;
		}
		if (IsFlexibleSegment(elem_head.type.classID)) {
			return;
		}
		if (IsTransition(elem_head.type.classID)) {
			return;
		}
		if (IsBend(elem_head.type.classID)) {
			ACAPI::Result<Bend> bendElement = Bend::Get(Adapter::UniqueID(elemGuid));
			if (bendElement.IsErr()) return;
			UniqueID nodeId = bendElement->GetRoutingNodeId();
			ACAPI::Result<RoutingNode> nodeElement = RoutingNode::Get(nodeId);
			if (nodeElement.IsErr()) return;
			UniqueID routingId = nodeElement->GetRoutingElementId();
			GetSubElementOfRouting(GSGuid2APIGuid(routingId.GetGuid()), subelemGuid);
			return;
		}
		if (IsRigidSegment(elem_head.type.classID)) {
			ACAPI::Result<RigidSegment> rigidElement = RigidSegment::Get(Adapter::UniqueID(elemGuid));
			if (rigidElement.IsErr()) return;
			UniqueID segmentId = rigidElement->GetRoutingSegmentId();
			ACAPI::Result<RoutingSegment> segmentElement = RoutingSegment::Get(segmentId);
			if (segmentElement.IsErr()) return;
			UniqueID routingId = segmentElement->GetRoutingElementId();
			GetSubElementOfRouting(GSGuid2APIGuid(routingId.GetGuid()), subelemGuid);
			return;
		}
	}
}
#endif