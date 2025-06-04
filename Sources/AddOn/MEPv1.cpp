//------------ kuvbur 2022 ------------
#if defined(AC_27) || defined(AC_28)
#include	"ACAPinc.h"
#include	"APIEnvir.h"
#include	"MEPv1.hpp"
using namespace ACAPI::MEP;

API_Guid GetRigidSegmentClassIDFromRoutingElemClassID (const API_Guid& routingElemClassID)
{
    #if defined (AC_28)
    if (routingElemClassID == ACAPI::MEP::VentilationRoutingElementID)
        #else
    if (routingElemClassID == ACAPI::MEP::VentilationRoutingID)
        #endif
        return ACAPI::MEP::VentilationRigidSegmentID;
    #if defined (AC_28)
    if (routingElemClassID == ACAPI::MEP::PipingRoutingElementID)
        #else
    if (routingElemClassID == ACAPI::MEP::PipingRoutingID)
        #endif
        return ACAPI::MEP::PipingRigidSegmentID;
    #if defined (AC_28)
    if (routingElemClassID == ACAPI::MEP::CableCarrierRoutingElementID)
        #else
    if (routingElemClassID == ACAPI::MEP::CableCarrierRoutingID)
        #endif
        return ACAPI::MEP::CableCarrierRigidSegmentID;

    return APINULLGuid;
}

API_Guid GetBendClassIDFromRoutingElemClassID (const API_Guid& routingElemClassID)
{
    #if defined (AC_28)
    if (routingElemClassID == ACAPI::MEP::VentilationRoutingElementID)
        #else
    if (routingElemClassID == ACAPI::MEP::VentilationRoutingID)
        #endif
        return ACAPI::MEP::VentilationBendID;

    #if defined (AC_28)
    if (routingElemClassID == ACAPI::MEP::PipingRoutingElementID)
        #else
    if (routingElemClassID == ACAPI::MEP::PipingRoutingID)
        #endif
        return ACAPI::MEP::PipingBendID;

    #if defined (AC_28)
    if (routingElemClassID == ACAPI::MEP::CableCarrierRoutingElementID)
        #else
    if (routingElemClassID == ACAPI::MEP::CableCarrierRoutingID)
        #endif
        return ACAPI::MEP::CableCarrierBendID;

    return APINULLGuid;
}

API_Guid GetTransitionClassIDFromRoutingElemClassID (const API_Guid& routingElemClassID)
{
    #if defined (AC_28)
    if (routingElemClassID == ACAPI::MEP::VentilationRoutingElementID)
        #else
    if (routingElemClassID == ACAPI::MEP::VentilationRoutingID)
        #endif
        return ACAPI::MEP::VentilationTransitionID;

    #if defined (AC_28)
    if (routingElemClassID == ACAPI::MEP::PipingRoutingElementID)
        #else
    if (routingElemClassID == ACAPI::MEP::PipingRoutingID)
        #endif
        return ACAPI::MEP::PipingTransitionID;

    #if defined (AC_28)
    if (routingElemClassID == ACAPI::MEP::CableCarrierRoutingElementID)
        #else
    if (routingElemClassID == ACAPI::MEP::CableCarrierRoutingID)
        #endif
        return ACAPI::MEP::CableCarrierTransitionID;

    return APINULLGuid;
}

namespace MEPv1
{

bool ReadMEP (const API_Elem_Head& elem_head, ParamDictValue& paramByType)
{
    #if !defined (AC_28)
    return false;
    #else
    ACAPI::Result<RoutingElement> routingElement = RoutingElement::Get (Adapter::UniqueID (elem_head.guid));
    if (routingElement.IsErr ()) {
        ACAPI_WriteReport (routingElement.UnwrapErr ().text.c_str (), false);
        return false;
    }
    std::vector<ACAPI::MEP::UniqueID> routingNodeIds = routingElement->GetRoutingNodeIds ();
    std::vector<ACAPI::MEP::UniqueID> routingSegmentIds = routingElement->GetRoutingSegmentIds ();
    bool flag_find = false;
    return flag_find;
    #endif
}


void GetSubElementOfRouting (const API_Guid& elemGuid, GS::Array<API_Guid>& subelemGuid)
{
    ACAPI::Result<RoutingElement> routingElement = RoutingElement::Get (Adapter::UniqueID (elemGuid));
    if (routingElement.IsErr ()) {
        ACAPI_WriteReport (routingElement.UnwrapErr ().text.c_str (), false);
        return;
    }
    std::vector<ACAPI::MEP::UniqueID> routingNodeIds = routingElement->GetRoutingNodeIds ();
    std::vector<ACAPI::MEP::UniqueID> routingSegmentIds = routingElement->GetRoutingSegmentIds ();

    API_ElemType mepElemType;
    mepElemType.typeID = API_ExternalElemID;
    mepElemType.variationID = APIVarId_Generic;
    if (!routingNodeIds.empty ()) {
        for (UInt32 inx_segment = 0; inx_segment < routingNodeIds.size (); ++inx_segment) {
            ACAPI::Result<RoutingNode> routingNode = RoutingNode::Get (routingNodeIds[inx_segment]);
            if (routingNode.IsErr ()) {
                ACAPI_WriteReport (routingNode.UnwrapErr ().text.c_str (), false);
                return;
            }
            API_Guid rguid = GSGuid2APIGuid (routingNodeIds[inx_segment].GetGuid ());
            subelemGuid.Push (rguid);
            std::vector<ACAPI::MEP::UniqueID> bendsIds = routingNode->GetBendIds ();
            std::vector<ACAPI::MEP::UniqueID> transitionsIds = routingNode->GetTransitionIds ();
            if (!bendsIds.empty ()) {
                for (UInt32 inx_rigid = 0; inx_rigid < bendsIds.size (); ++inx_rigid) {
                    API_Guid rguid = GSGuid2APIGuid (bendsIds[inx_rigid].GetGuid ());
                    subelemGuid.Push (rguid);
                }
            }
            if (!transitionsIds.empty ()) {
                for (UInt32 inx_rigid = 0; inx_rigid < transitionsIds.size (); ++inx_rigid) {
                    API_Guid rguid = GSGuid2APIGuid (transitionsIds[inx_rigid].GetGuid ());
                    subelemGuid.Push (rguid);
                }
            }
        }
    }
    if (!routingSegmentIds.empty ()) {
        for (UInt32 inx_segment = 0; inx_segment < routingSegmentIds.size (); ++inx_segment) {
            ACAPI::Result<RoutingSegment> routingSegment = RoutingSegment::Get (routingSegmentIds[inx_segment]);
            if (routingSegment.IsErr ()) {
                ACAPI_WriteReport (routingSegment.UnwrapErr ().text.c_str (), false);
                return;
            }
            API_Guid rguid = GSGuid2APIGuid (routingSegmentIds[inx_segment].GetGuid ());
            subelemGuid.Push (rguid);
            std::vector<ACAPI::MEP::UniqueID> rigidSegmentIds = routingSegment->GetRigidSegmentIds ();
            if (!rigidSegmentIds.empty ()) {
                for (UInt32 inx_rigid = 0; inx_rigid < rigidSegmentIds.size (); ++inx_rigid) {
                    API_Guid rguid = GSGuid2APIGuid (rigidSegmentIds[inx_rigid].GetGuid ());
                    subelemGuid.Push (rguid);
                }
            }
        }
    }
}
void GetPreferenceTable (const API_Elem_Head& elem_head)
{
    if (IsRoutingElement (elem_head.type.classID)) {
        return;
    }
    if (IsBranch (elem_head.type.classID)) {
        return;
    }
    if (IsAccessory (elem_head.type.classID)) {
        return;
    }
    if (IsEquipment (elem_head.type.classID)) {
        return;
    }
    if (IsTerminal (elem_head.type.classID)) {
        return;
    }
    if (IsFitting (elem_head.type.classID)) {
        return;
    }
    if (IsFlexibleSegment (elem_head.type.classID)) {
        return;
    }
    if (IsTransition (elem_head.type.classID)) {
        return;
    }
    if (IsBend (elem_head.type.classID)) {
        return;
    }
    if (IsRigidSegment (elem_head.type.classID)) {
        return;
    }
}

void GetSubElement (const API_Guid& elemGuid, GS::Array<API_Guid>& subelemGuid)
{
    GSErrCode		err = NoError;

    API_Elem_Head elem_head = {};
    BNZeroMemory (&elem_head, sizeof (API_Elem_Head));
    elem_head.guid = elemGuid;
    err = ACAPI_Element_GetHeader (&elem_head);
    if (elem_head.type.typeID != API_ExternalElemID) return;
    GS::UniString txttype = "";
    if (IsRoutingElement (elem_head.type.classID)) {
        GetSubElementOfRouting (elemGuid, subelemGuid);
        return;
    }
    if (IsBranch (elem_head.type.classID)) {
        return;
    }
    if (IsAccessory (elem_head.type.classID)) {
        return;
    }
    if (IsEquipment (elem_head.type.classID)) {
        return;
    }
    if (IsTerminal (elem_head.type.classID)) {
        return;
    }
    if (IsFitting (elem_head.type.classID)) {
        return;
    }
    if (IsFlexibleSegment (elem_head.type.classID)) {
        return;
    }
    if (IsTransition (elem_head.type.classID)) {
        return;
    }
    if (IsBend (elem_head.type.classID)) {
        ACAPI::Result<Bend> bendElement = Bend::Get (Adapter::UniqueID (elemGuid));
        if (bendElement.IsErr ()) return;
        ACAPI::MEP::UniqueID nodeId = bendElement->GetRoutingNodeId ();
        ACAPI::Result<RoutingNode> nodeElement = RoutingNode::Get (nodeId);
        if (nodeElement.IsErr ()) return;
        ACAPI::MEP::UniqueID routingId = nodeElement->GetRoutingElementId ();
        GetSubElementOfRouting (GSGuid2APIGuid (routingId.GetGuid ()), subelemGuid);
        return;
    }
    if (IsRigidSegment (elem_head.type.classID)) {
        ACAPI::Result<RigidSegment> rigidElement = RigidSegment::Get (Adapter::UniqueID (elemGuid));
        if (rigidElement.IsErr ()) return;
        ACAPI::MEP::UniqueID segmentId = rigidElement->GetRoutingSegmentId ();
        ACAPI::Result<RoutingSegment> segmentElement = RoutingSegment::Get (segmentId);
        if (segmentElement.IsErr ()) return;
        ACAPI::MEP::UniqueID routingId = segmentElement->GetRoutingElementId ();
        GetSubElementOfRouting (GSGuid2APIGuid (routingId.GetGuid ()), subelemGuid);
        return;
    }
}
}
#endif
