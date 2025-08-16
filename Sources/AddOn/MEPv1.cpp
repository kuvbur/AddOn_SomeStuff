//------------ kuvbur 2022 ------------
#if defined(AC_27) || defined(AC_28) || defined(AC_29)
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
    return GetMEPData (elem_head, paramByType);
    #endif
}


void GetSubElementOfRouting (const API_Guid& elemGuid, GS::Array<API_Guid>& subelemGuid)
{
    ACAPI::Result<RoutingElement> routingElement = RoutingElement::Get (Adapter::UniqueID (elemGuid));
    if (routingElement.IsErr ()) {
        ACAPI_WriteReport (routingElement.UnwrapErr ().text.c_str (), false);
        return;
    }
    subelemGuid.Push (elemGuid);
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
#if defined (AC_28)
bool GetMEPData (const API_Elem_Head& elem_head, ParamDictValue& paramByType)
{
    bool flag = false;
    GS::UniString rawName = "";
    if (IsVentilation (elem_head.type.classID)) {
        // Общая таблица воздуховодов
        rawName = "{@mep:reference set name}";
        if (paramByType.ContainsKey (rawName)) {
            ACAPI::Result<DuctReferenceSet> ductReferenceSet = GetDuctReferenceSet ();
            if (ductReferenceSet.IsErr ()) {
                ACAPI_WriteReport (ductReferenceSet.UnwrapErr ().text.c_str (), false);
                return flag;
            }
            ParamValue& pval = paramByType.Get (rawName);
            ParamHelpers::ConvertStringToParamValue (pval, "", ductReferenceSet->GetName ());
            flag = true;
        }
        if (IsBranch (elem_head.type.classID)) {
            rawName = "{@mep:test}";
            if (paramByType.ContainsKey (rawName)) {
                ParamValue& pval = paramByType.Get (rawName);
                ParamHelpers::ConvertStringToParamValue (pval, "", "IsBranch");
                flag = true;
            }
            return flag;
        }
        if (IsAccessory (elem_head.type.classID)) {
            rawName = "{@mep:test}";
            if (paramByType.ContainsKey (rawName)) {
                ParamValue& pval = paramByType.Get (rawName);
                ParamHelpers::ConvertStringToParamValue (pval, "", "IsAccessory");
                flag = true;
            }
            return flag;
        }
        if (IsEquipment (elem_head.type.classID)) {
            rawName = "{@mep:test}";
            if (paramByType.ContainsKey (rawName)) {
                ParamValue& pval = paramByType.Get (rawName);
                ParamHelpers::ConvertStringToParamValue (pval, "", "IsEquipment");
                flag = true;
            }
            return flag;
        }
        if (IsTerminal (elem_head.type.classID)) {
            rawName = "{@mep:test}";
            if (paramByType.ContainsKey (rawName)) {
                ParamValue& pval = paramByType.Get (rawName);
                ParamHelpers::ConvertStringToParamValue (pval, "", "IsTerminal");
                flag = true;
            }
            return flag;
        }
        if (IsRoutingElement (elem_head.type.classID)) {
            ACAPI::MEP::UniqueID segmentId = Adapter::UniqueID (elem_head.guid);
            ACAPI::MEP::UniqueID tableID = Adapter::UniqueID (APINULLGuid);
            if (!ReadRoutingElementData (segmentId, flag, paramByType, tableID)) return flag;
            return flag;
        }
        if (IsFitting (elem_head.type.classID)) {
            rawName = "{@mep:test}";
            if (paramByType.ContainsKey (rawName)) {
                ParamValue& pval = paramByType.Get (rawName);
                ParamHelpers::ConvertStringToParamValue (pval, "", "IsFitting");
                flag = true;
            }
            return flag;
        }
        if (IsRigidSegment (elem_head.type.classID)) {
            ACAPI::MEP::UniqueID segmentId = Adapter::UniqueID (APINULLGuid);
            ACAPI::MEP::ConnectorShape shape = ConnectorShape::Rectangular;
            ACAPI::MEP::UniqueID tableID = Adapter::UniqueID (APINULLGuid);
            uint32_t refid = 0;
            if (!ReadRigidSegmentData (elem_head.guid, flag, paramByType, segmentId)) return flag;
            if (!ReadRoutingSegmentData (segmentId, flag, paramByType, shape, tableID, refid)) return flag;
            if (!ReadDuctSegmentPreferenceTable (flag, paramByType, shape, tableID, refid)) return flag;
            return flag;
        }
        if (IsBend (elem_head.type.classID)) {
            ACAPI::MEP::UniqueID segmentId = Adapter::UniqueID (APINULLGuid);
            ACAPI::MEP::ConnectorShape shape = ConnectorShape::Rectangular;
            ACAPI::MEP::UniqueID tableID = Adapter::UniqueID (APINULLGuid);
            double diametr = 0; double radius = 0;
            if (!ReadBendData (elem_head.guid, flag, paramByType, shape, tableID, diametr, radius)) return flag;
            if (!ReadDuctBendPreferenceTable (flag, paramByType, shape, tableID, diametr, radius)) return flag;
            return flag;
        }
        return flag;
    }
    if (IsPiping (elem_head.type.classID)) {
        rawName = "{@mep:reference set name}";
        if (paramByType.ContainsKey (rawName)) {
            ACAPI::Result<PipeReferenceSet> pipeReferenceSet = GetPipeReferenceSet ();
            if (pipeReferenceSet.IsErr ()) {
                ACAPI_WriteReport (pipeReferenceSet.UnwrapErr ().text.c_str (), false);
                return flag;
            }
            ParamValue& pval = paramByType.Get (rawName);
            ParamHelpers::ConvertStringToParamValue (pval, "", pipeReferenceSet->GetName ());
            flag = true;
        }
        if (IsRoutingElement (elem_head.type.classID)) {
            ACAPI::MEP::UniqueID segmentId = Adapter::UniqueID (elem_head.guid);
            ACAPI::MEP::UniqueID tableID = Adapter::UniqueID (APINULLGuid);
            if (!ReadRoutingElementData (segmentId, flag, paramByType, tableID)) return flag;
            return flag;
        }
        if (IsRigidSegment (elem_head.type.classID)) {
            ACAPI::MEP::UniqueID segmentId = Adapter::UniqueID (APINULLGuid);
            ACAPI::MEP::ConnectorShape shape = ConnectorShape::Rectangular;
            ACAPI::MEP::UniqueID tableID = Adapter::UniqueID (APINULLGuid);
            uint32_t refid = 0;
            if (!ReadRigidSegmentData (elem_head.guid, flag, paramByType, segmentId)) return flag;
            if (!ReadRoutingSegmentData (segmentId, flag, paramByType, shape, tableID, refid)) return flag;
            if (!ReadPipeSegmentPreferenceTable (flag, paramByType, shape, tableID, refid)) return flag;
            return flag;
        }
        if (IsBend (elem_head.type.classID)) {
            ACAPI::MEP::UniqueID segmentId = Adapter::UniqueID (APINULLGuid);
            ACAPI::MEP::ConnectorShape shape = ConnectorShape::Rectangular;
            ACAPI::MEP::UniqueID tableID = Adapter::UniqueID (APINULLGuid);
            double diametr = 0; double radius = 0;
            if (!ReadBendData (elem_head.guid, flag, paramByType, shape, tableID, diametr, radius)) return flag;
            if (!ReadPipeBendPreferenceTable (flag, paramByType, shape, tableID, diametr, radius)) return flag;
            return flag;
        }
        if (IsTransition (elem_head.type.classID)) {
            ACAPI::MEP::ConnectorShape shape = ConnectorShape::Rectangular;
            ACAPI::MEP::UniqueID tableID = Adapter::UniqueID (APINULLGuid);
            double bdiametr = 0; double ediametr = 0;
            if (!ReadTransitionData (elem_head.guid, flag, paramByType, shape, tableID, bdiametr, ediametr)) return flag;
            if (!ReadTransitionPreferenceTable (flag, paramByType, shape, tableID, bdiametr, ediametr)) return flag;
            return flag;
        }
        if (IsBranch (elem_head.type.classID)) {
            rawName = "{@mep:test}";
            if (paramByType.ContainsKey (rawName)) {
                ParamValue& pval = paramByType.Get (rawName);
                ParamHelpers::ConvertStringToParamValue (pval, "", "IsBranch");
                flag = true;
            }
            return flag;
        }
        return flag;
    }
    if (IsCableCarrier (elem_head.type.classID)) {
        return flag;
    }
    return flag;
}

bool ReadTransitionData (const API_Guid& guid, bool& flag, ParamDictValue& paramByType, ACAPI::MEP::ConnectorShape& shape, ACAPI::MEP::UniqueID& transtableID, double& bdiametr, double& ediametr)
{
    GS::UniString rawName = "";
    rawName = "{@mep:test}";
    if (paramByType.ContainsKey (rawName)) {
        ParamValue& pval = paramByType.Get (rawName);
        ParamHelpers::ConvertStringToParamValue (pval, "", "IsTransition");
        flag = true;
    }
    #ifdef MEPAPI_VERSION
    ACAPI::Result<v2::Transition> element = v2::Transition::Get (Adapter::UniqueID (guid));
    if (element.IsErr ()) {
        ACAPI_WriteReport (element.UnwrapErr ().text.c_str (), false);
        return false;
    }
    rawName = "{@mep:insulation thickness}";
    if (paramByType.ContainsKey (rawName)) {
        ParamValue& pval = paramByType.Get (rawName);
        std::optional<double> insulation = element->GetInsulationThickness ();
        if (insulation.has_value ()) {
            ParamHelpers::ConvertDoubleToParamValue (pval, "", insulation.value ());
        } else {
            ParamHelpers::ConvertDoubleToParamValue (pval, "", 0);
        }
        flag = true;
    }
    rawName = "{@mep:length}";
    if (paramByType.ContainsKey (rawName)) {
        ParamValue& pval = paramByType.Get (rawName);
        ParamHelpers::ConvertDoubleToParamValue (pval, "", element->GetLength ());
        flag = true;
    }
    ACAPI::MEP::UniqueID nodeId = element->GetRoutingNodeId ();
    ACAPI::Result<RoutingNode> nodeElement = RoutingNode::Get (nodeId);
    if (nodeElement.IsOk ()) {
        ReadRoutingElementData (nodeElement->GetRoutingElementId (), flag, paramByType, transtableID);
    }
    rawName = "{@mep:description}";
    if (paramByType.ContainsKey (rawName)) {
        ACAPI::Result<Port> port_1 = Port::Get (element->GetWiderPortID ());
        if (port_1.IsErr ()) {
            ACAPI_WriteReport (port_1.UnwrapErr ().text.c_str (), false);
            return false;
        }
        ediametr = port_1->GetHeight ();
        ACAPI::Result<Port> port_2 = Port::Get (element->GetNarrowerPortID ());
        if (port_2.IsErr ()) {
            ACAPI_WriteReport (port_2.UnwrapErr ().text.c_str (), false);
            return false;
        }
        bdiametr = port_2->GetHeight ();
        shape = port_2->GetShape ();
    }
    ACAPI::Result<ACAPI::MEP::UniqueID> tableID_ = element->GetPreferenceTable ();
    if (tableID_.IsOk ()) {
        transtableID = tableID_.Unwrap ();
    }
    #else
    return false;
    #endif // MEPAPI_VERSION
    return true;
}

bool ReadRoutingElementData (const ACAPI::MEP::UniqueID& elementID, bool& flag, ParamDictValue& paramByType, ACAPI::MEP::UniqueID& branchtableID)
{
    GS::UniString rawName = "";
    rawName = "{@mep:test}";
    if (paramByType.ContainsKey (rawName)) {
        ParamValue& pval = paramByType.Get (rawName);
        ParamHelpers::ConvertStringToParamValue (pval, "", "IsRoutingElement");
        flag = true;
    }
    ACAPI::Result<RoutingElement> element = RoutingElement::Get (elementID);
    if (element.IsErr ()) {
        ACAPI_WriteReport (element.UnwrapErr ().text.c_str (), false);
        return false;
    }
    rawName = "{@mep:system}";
    if (paramByType.ContainsKey (rawName)) {
        ParamValue& pval = paramByType.Get (rawName);
        if (element->GetMEPSystem () != APIInvalidAttributeIndex) {
            API_Attribute attribute = {};
            attribute.header.typeID = API_MEPSystemID;
            attribute.header.index = element->GetMEPSystem ();
            if (ACAPI_Attribute_Get (&attribute) == NoError) {
                ParamHelpers::ConvertStringToParamValue (pval, "", GS::UniString { attribute.header.name });
                flag = true;
            }
        }
    }
    rawName = "{@mep:routing length}";
    if (paramByType.ContainsKey (rawName)) {
        ParamValue& pval = paramByType.Get (rawName);
        double length = 0;
        std::vector<API_Coord3D> poly = element->GetPolyLine ();
        bool is_first = true;
        double x1 = 0; double x2 = 0;
        double y1 = 0; double y2 = 0;
        double z1 = 0; double z2 = 0;
        for (const auto& pt : poly) {
            if (is_first) {
                x1 = pt.x; y1 = pt.y; z1 = pt.z;
                is_first = false;
            } else {
                x2 = pt.x; y2 = pt.y; z2 = pt.z;
                double dx = pow (x2 - x1, 2);
                double dy = pow (y2 - y1, 2);
                double dz = pow (z2 - z1, 2);
                length += sqrt (dx + dy + dz);
                x1 = x2; y1 = y2; z1 = z2;
            }
        }
        ParamHelpers::ConvertDoubleToParamValue (pval, "", length);
        flag = true;
    }
    ACAPI::Result<ACAPI::MEP::UniqueID> tableID_ = element->GetBranchPreferenceTableId ();
    if (tableID_.IsOk ()) {
        branchtableID = tableID_.Unwrap ();
    }
    return true;
}

bool ReadBendData (const API_Guid& guid, bool& flag, ParamDictValue& paramByType, ACAPI::MEP::ConnectorShape& shape, ACAPI::MEP::UniqueID& bendtableID, double& diametr, double& radius)
{
    GS::UniString rawName = "";
    rawName = "{@mep:test}";
    if (paramByType.ContainsKey (rawName)) {
        ParamValue& pval = paramByType.Get (rawName);
        ParamHelpers::ConvertStringToParamValue (pval, "", "IsBend");
        flag = true;
    }
    ACAPI::Result<Bend> element = Bend::Get (Adapter::UniqueID (guid));
    if (element.IsErr ()) {
        ACAPI_WriteReport (element.UnwrapErr ().text.c_str (), false);
        return false;
    }
    diametr = element->GetWidth ();
    radius = element->GetRadius ();
    rawName = "{@mep:bend factor radius}";
    if (paramByType.ContainsKey (rawName)) {
        ParamValue& pval = paramByType.Get (rawName);
        ACAPI::Result<double> factorRadius = element->GetFactorRadius ();
        if (factorRadius.IsOk ()) {
            ParamHelpers::ConvertDoubleToParamValue (pval, "", factorRadius.Unwrap ());
            flag = true;
        }
    }
    rawName = "{@mep:bend radius}";
    if (paramByType.ContainsKey (rawName)) {
        ParamValue& pval = paramByType.Get (rawName);
        ParamHelpers::ConvertDoubleToParamValue (pval, "", radius);
        flag = true;
    }
    rawName = "{@mep:width}";
    if (paramByType.ContainsKey (rawName)) {
        ParamValue& pval = paramByType.Get (rawName);
        ParamHelpers::ConvertDoubleToParamValue (pval, "", diametr);
        flag = true;
    }
    rawName = "{@mep:height}";
    if (paramByType.ContainsKey (rawName)) {
        ParamValue& pval = paramByType.Get (rawName);
        ParamHelpers::ConvertDoubleToParamValue (pval, "", element->GetHeight ());
        flag = true;
    }
    rawName = "{@mep:insulation thickness}";
    if (paramByType.ContainsKey (rawName)) {
        ParamValue& pval = paramByType.Get (rawName);
        std::optional<double> insulation = element->GetInsulationThickness ();
        if (insulation.has_value ()) {
            ParamHelpers::ConvertDoubleToParamValue (pval, "", insulation.value ());
        } else {
            ParamHelpers::ConvertDoubleToParamValue (pval, "", 0);
        }
        flag = true;
    }
    ACAPI::MEP::UniqueID nodeId = element->GetRoutingNodeId ();
    ACAPI::Result<RoutingNode> nodeElement = RoutingNode::Get (nodeId);
    if (nodeElement.IsOk ()) {
        ReadRoutingElementData (nodeElement->GetRoutingElementId (), flag, paramByType, bendtableID);
    }
    shape = element->GetShape ();
    ACAPI::Result<ACAPI::MEP::UniqueID> tableID_ = element->GetPreferenceTable ();
    if (tableID_.IsOk ()) {
        bendtableID = tableID_.Unwrap ();
    }
    return true;
}

bool ReadRigidSegmentData (const API_Guid& guid, bool& flag, ParamDictValue& paramByType, ACAPI::MEP::UniqueID& segmentId)
{
    GS::UniString rawName = "";
    rawName = "{@mep:test}";
    if (paramByType.ContainsKey (rawName)) {
        ParamValue& pval = paramByType.Get (rawName);
        ParamHelpers::ConvertStringToParamValue (pval, "", "IsRigidSegment");
        flag = true;
    }
    ACAPI::Result<RigidSegment> element = RigidSegment::Get (Adapter::UniqueID (guid));
    if (element.IsErr ()) {
        ACAPI_WriteReport (element.UnwrapErr ().text.c_str (), false);
        return false;
    }
    segmentId = element->GetRoutingSegmentId ();
    rawName = "{@mep:length}";
    if (paramByType.ContainsKey (rawName)) {
        ParamValue& pval = paramByType.Get (rawName);
        ParamHelpers::ConvertDoubleToParamValue (pval, "", element->GetLength ());
        flag = true;
    }
    rawName = "{@mep:width}";
    if (paramByType.ContainsKey (rawName)) {
        ParamValue& pval = paramByType.Get (rawName);
        ParamHelpers::ConvertDoubleToParamValue (pval, "", element->GetWidth ());
        flag = true;
    }
    rawName = "{@mep:height}";
    if (paramByType.ContainsKey (rawName)) {
        ParamValue& pval = paramByType.Get (rawName);
        ParamHelpers::ConvertDoubleToParamValue (pval, "", element->GetHeight ());
        flag = true;
    }
    rawName = "{@mep:insulation thickness}";
    if (paramByType.ContainsKey (rawName)) {
        ParamValue& pval = paramByType.Get (rawName);
        std::optional<double> insulation = element->GetInsulationThickness ();
        if (insulation.has_value ()) {
            ParamHelpers::ConvertDoubleToParamValue (pval, "", insulation.value ());
        } else {
            ParamHelpers::ConvertDoubleToParamValue (pval, "", 0);
        }
        flag = true;
    }
    return true;
}

bool ReadRoutingSegmentData (const ACAPI::MEP::UniqueID& segmentId, bool& flag, ParamDictValue& paramByType, ACAPI::MEP::ConnectorShape& shape, ACAPI::MEP::UniqueID& tableID, uint32_t& refid)
{
    GS::UniString rawName = "";
    rawName = "{@mep:test}";
    if (paramByType.ContainsKey (rawName)) {
        ParamValue& pval = paramByType.Get (rawName);
        if (!pval.isValid) {
            ParamHelpers::ConvertStringToParamValue (pval, "", "IsRoutingSegment");
            flag = true;
        }
    }
    ACAPI::Result<RoutingSegment> segmentElement = RoutingSegment::Get (segmentId);
    if (segmentElement.IsErr ()) {
        ACAPI_WriteReport (segmentElement.UnwrapErr ().text.c_str (), false);
        return false;
    }
    ReadRoutingElementData (segmentElement->GetRoutingElementId (), flag, paramByType, tableID);
    tableID = segmentElement->GetPreferenceTableId ();
    shape = segmentElement->GetCrossSectionShape ();
    rawName = "{@mep:wall thickness}";
    if (paramByType.ContainsKey (rawName)) {
        ParamValue& pval = paramByType.Get (rawName);
        if (!pval.isValid) {
            ParamHelpers::ConvertDoubleToParamValue (pval, "", segmentElement->GetWallThickness ());
            flag = true;
        }
    }
    rawName = "{@mep:width}";
    if (paramByType.ContainsKey (rawName)) {
        ParamValue& pval = paramByType.Get (rawName);
        if (!pval.isValid) {
            ParamHelpers::ConvertDoubleToParamValue (pval, "", segmentElement->GetCrossSectionWidth ());
            flag = true;
        }
    }
    rawName = "{@mep:height}";
    if (paramByType.ContainsKey (rawName)) {
        ParamValue& pval = paramByType.Get (rawName);
        if (!pval.isValid) {
            ParamHelpers::ConvertDoubleToParamValue (pval, "", segmentElement->GetCrossSectionHeight ());
            flag = true;
        }
    }
    rawName = "{@mep:shape}";
    if (paramByType.ContainsKey (rawName)) {
        ParamValue& pval = paramByType.Get (rawName);
        switch (shape) {
            case ACAPI::MEP::ConnectorShape::Rectangular:
                ParamHelpers::ConvertIntToParamValue (pval, "", 1);
                pval.val.uniStringValue = "Rectangular";
                break;
            case ACAPI::MEP::ConnectorShape::Circular:
                ParamHelpers::ConvertIntToParamValue (pval, "", 2);
                pval.val.uniStringValue = "Circular";
                break;
            case ACAPI::MEP::ConnectorShape::Oval:
                ParamHelpers::ConvertIntToParamValue (pval, "", 3);
                pval.val.uniStringValue = "Oval";
                break;
            case ACAPI::MEP::ConnectorShape::UShape:
                ParamHelpers::ConvertIntToParamValue (pval, "", 4);
                pval.val.uniStringValue = "UShape";
                break;
            default:
                break;
        }
        flag = true;
    }
    if (shape == ConnectorShape::Circular) {
        ACAPI::Result<uint32_t> refid_ = segmentElement->GetCrossSectionReferenceId ();
        if (refid_.IsErr ()) {
            ACAPI_WriteReport (refid_.UnwrapErr ().text.c_str (), false);
            return false;
        }
        refid = refid_.Unwrap ();
    }
    return true;
}

bool ReadDuctSegmentPreferenceTable (bool& flag, ParamDictValue& paramByType, ACAPI::MEP::ConnectorShape& shape, ACAPI::MEP::UniqueID& tableID, uint32_t& refid)
{
    GS::UniString rawName = "";
    if (shape == ACAPI::MEP::ConnectorShape::Rectangular) {
        rawName = "{@mep:element set name}";
        if (paramByType.ContainsKey (rawName)) {
            ACAPI::Result<DuctRectangularSegmentPreferenceTable> table = DuctRectangularSegmentPreferenceTable::Get (tableID);
            if (table.IsErr ()) {
                ACAPI_WriteReport (table.UnwrapErr ().text.c_str (), false);
                return false;
            }
            ParamValue& pval = paramByType.Get (rawName);
            ParamHelpers::ConvertStringToParamValue (pval, "", table->GetName ());
            flag = true;
        }
    }
    if (shape == ACAPI::MEP::ConnectorShape::Circular) {
        ACAPI::Result<DuctCircularSegmentPreferenceTable> table = DuctCircularSegmentPreferenceTable::Get (tableID);
        if (table.IsErr ()) {
            ACAPI_WriteReport (table.UnwrapErr ().text.c_str (), false);
            return false;
        }
        rawName = "{@mep:element set name}";
        if (paramByType.ContainsKey (rawName)) {
            ParamValue& pval = paramByType.Get (rawName);
            ParamHelpers::ConvertStringToParamValue (pval, "", table->GetName ());
            flag = true;
        }
        if (table->IsRowValidByReferenceId (refid).IsOk ()) {
            rawName = "{@mep:description}";
            if (paramByType.ContainsKey (rawName)) {
                ParamValue& pval = paramByType.Get (rawName);
                ACAPI::Result<GS::UniString> val = table->GetDescriptionByReferenceId (refid);
                if (val.IsOk ()) {
                    ParamHelpers::ConvertStringToParamValue (pval, "", val.Unwrap ());
                    flag = true;
                } else {
                    ParamHelpers::ConvertStringToParamValue (pval, "", "");
                    flag = true;
                }
            }
            rawName = "{@mep:diametr}";
            if (paramByType.ContainsKey (rawName)) {
                ACAPI::Result<double> val = table->GetDiameterByReferenceId (refid);
                if (val.IsOk ()) {
                    ParamValue& pval = paramByType.Get (rawName);
                    ParamHelpers::ConvertDoubleToParamValue (pval, "", val.Unwrap ());
                    flag = true;
                }
            }
            rawName = "{@mep:wall thickness}";
            if (paramByType.ContainsKey (rawName)) {
                ParamValue& pval = paramByType.Get (rawName);
                if (!pval.isValid) {
                    ACAPI::Result<double> val = table->GetWallThicknessByReferenceId (refid);
                    if (val.IsOk ()) {
                        ParamHelpers::ConvertDoubleToParamValue (pval, "", val.Unwrap ());
                        flag = true;
                    }
                }
            }
        }
    }
    return true;
}

bool ReadPipeSegmentPreferenceTable (bool& flag, ParamDictValue& paramByType, ACAPI::MEP::ConnectorShape& shape, ACAPI::MEP::UniqueID& tableID, uint32_t& refid)
{
    GS::UniString rawName = "";
    if (shape != ACAPI::MEP::ConnectorShape::Circular) return false;
    ACAPI::Result<PipeSegmentPreferenceTable> table = PipeSegmentPreferenceTable::Get (tableID);
    if (table.IsErr ()) {
        ACAPI_WriteReport (table.UnwrapErr ().text.c_str (), false);
        return false;
    }
    rawName = "{@mep:element set name}";
    if (paramByType.ContainsKey (rawName)) {
        ParamValue& pval = paramByType.Get (rawName);
        ParamHelpers::ConvertStringToParamValue (pval, "", table->GetName ());
        flag = true;
    }
    if (table->IsRowValidByReferenceId (refid).IsOk ()) {
        rawName = "{@mep:description}";
        if (paramByType.ContainsKey (rawName)) {
            ParamValue& pval = paramByType.Get (rawName);
            ACAPI::Result<GS::UniString> val = table->GetDescriptionByReferenceId (refid);
            if (val.IsOk ()) {
                ParamHelpers::ConvertStringToParamValue (pval, "", val.Unwrap ());
                flag = true;
            } else {
                ParamHelpers::ConvertStringToParamValue (pval, "", "");
                flag = true;
            }
        }
        rawName = "{@mep:diametr}";
        if (paramByType.ContainsKey (rawName)) {
            ACAPI::Result<double> val = table->GetDiameterByReferenceId (refid);
            if (val.IsOk ()) {
                ParamValue& pval = paramByType.Get (rawName);
                ParamHelpers::ConvertDoubleToParamValue (pval, "", val.Unwrap ());
                flag = true;
            }
        }
        rawName = "{@mep:wall thickness}";
        if (paramByType.ContainsKey (rawName)) {
            ParamValue& pval = paramByType.Get (rawName);
            if (!pval.isValid) {
                ACAPI::Result<double> val = table->GetWallThicknessByReferenceId (refid);
                if (val.IsOk ()) {
                    ParamHelpers::ConvertDoubleToParamValue (pval, "", val.Unwrap ());
                    flag = true;
                }
            }
        }
    }
    return true;
}

bool ReadDuctBendPreferenceTable (bool& flag, ParamDictValue& paramByType, ACAPI::MEP::ConnectorShape& shape, ACAPI::MEP::UniqueID& tableID, double& diametr, double& radius)
{
    GS::UniString rawName = "";
    if (shape != ACAPI::MEP::ConnectorShape::Circular) return false;

    ACAPI::Result<DuctElbowPreferenceTable> table = DuctElbowPreferenceTable::Get (tableID);
    if (table.IsErr ()) {
        ACAPI_WriteReport (table.UnwrapErr ().text.c_str (), false);
        return false;
    }
    rawName = "{@mep:element set name}";
    if (paramByType.ContainsKey (rawName)) {
        ParamValue& pval = paramByType.Get (rawName);
        ParamHelpers::ConvertStringToParamValue (pval, "", table->GetName ());
        flag = true;
    }
    uint32_t refid = 0;
    bool flag_find = false;
    for (uint32_t i = 0; i < table->GetSize (); ++i) {
        ACAPI::Result<double> d = table->GetDiameter (i);
        if (d.IsErr ()) continue;
        if (!is_equal (d.Unwrap (), diametr)) continue;
        ACAPI::Result<double> r = table->GetRadius (i);
        if (r.IsErr ()) continue;
        if (!is_equal (r.Unwrap (), radius)) continue;
        ACAPI::Result<uint32_t> rf = table->GetReferenceId (i);
        if (rf.IsOk ()) {
            refid = rf.Unwrap ();
            flag_find = true;
        }
        break;
    }
    if (!flag_find) return false;
    ACAPI::Result<bool> b = table->IsRowValidByReferenceId (refid);
    if (b.IsErr ()) return false;
    if (!b.Unwrap ()) return false;
    rawName = "{@mep:description}";
    if (paramByType.ContainsKey (rawName)) {
        ParamValue& pval = paramByType.Get (rawName);
        ACAPI::Result<GS::UniString> val = table->GetDescriptionByReferenceId (refid);
        if (val.IsOk ()) {
            ParamHelpers::ConvertStringToParamValue (pval, "", val.Unwrap ());
            flag = true;
        } else {
            ParamHelpers::ConvertStringToParamValue (pval, "", "");
            flag = true;
        }
    }
    rawName = "{@mep:diametr}";
    if (paramByType.ContainsKey (rawName)) {
        ACAPI::Result<double> val = table->GetDiameterByReferenceId (refid);
        if (val.IsOk ()) {
            ParamValue& pval = paramByType.Get (rawName);
            ParamHelpers::ConvertDoubleToParamValue (pval, "", val.Unwrap ());
            flag = true;
        }
    }
    rawName = "{@mep:bend radius}";
    if (paramByType.ContainsKey (rawName)) {
        ParamValue& pval = paramByType.Get (rawName);
        if (!pval.isValid) {
            ACAPI::Result<double> val = table->GetRadiusByReferenceId (refid);
            if (val.IsOk ()) {
                ParamHelpers::ConvertDoubleToParamValue (pval, "", val.Unwrap ());
                flag = true;
            }
        }
    }
    return true;
}

bool ReadPipeBendPreferenceTable (bool& flag, ParamDictValue& paramByType, ACAPI::MEP::ConnectorShape& shape, ACAPI::MEP::UniqueID& tableID, double& diametr, double& radius)
{
    GS::UniString rawName = "";
    if (shape != ACAPI::MEP::ConnectorShape::Circular) return false;
    ACAPI::Result<PipeElbowPreferenceTable> table = PipeElbowPreferenceTable::Get (tableID);
    if (table.IsErr ()) {
        ACAPI_WriteReport (table.UnwrapErr ().text.c_str (), false);
        return false;
    }
    rawName = "{@mep:element set name}";
    if (paramByType.ContainsKey (rawName)) {
        ParamValue& pval = paramByType.Get (rawName);
        ParamHelpers::ConvertStringToParamValue (pval, "", table->GetName ());
        flag = true;
    }
    uint32_t refid = 0;
    bool flag_find = false;
    for (uint32_t i = 0; i < table->GetSize (); ++i) {
        ACAPI::Result<double> d = table->GetDiameter (i);
        if (d.IsErr ()) continue;
        if (!is_equal (d.Unwrap (), diametr)) continue;
        ACAPI::Result<double> r = table->GetRadius (i);
        if (r.IsErr ()) continue;
        if (!is_equal (r.Unwrap (), radius)) continue;
        ACAPI::Result<uint32_t> rf = table->GetReferenceId (i);
        if (rf.IsOk ()) {
            refid = rf.Unwrap ();
            flag_find = true;
        }
        break;
    }
    if (!flag_find) return false;
    ACAPI::Result<bool> b = table->IsRowValidByReferenceId (refid);
    if (b.IsErr ()) return false;
    if (!b.Unwrap ()) return false;
    rawName = "{@mep:description}";
    if (paramByType.ContainsKey (rawName)) {
        ParamValue& pval = paramByType.Get (rawName);
        ACAPI::Result<GS::UniString> val = table->GetDescriptionByReferenceId (refid);
        if (val.IsOk ()) {
            ParamHelpers::ConvertStringToParamValue (pval, "", val.Unwrap ());
            flag = true;
        } else {
            ParamHelpers::ConvertStringToParamValue (pval, "", "");
            flag = true;
        }
    }
    rawName = "{@mep:diametr}";
    if (paramByType.ContainsKey (rawName)) {
        ACAPI::Result<double> val = table->GetDiameterByReferenceId (refid);
        if (val.IsOk ()) {
            ParamValue& pval = paramByType.Get (rawName);
            ParamHelpers::ConvertDoubleToParamValue (pval, "", val.Unwrap ());
            flag = true;
        }
    }
    rawName = "{@mep:bend radius}";
    if (paramByType.ContainsKey (rawName)) {
        ParamValue& pval = paramByType.Get (rawName);
        if (!pval.isValid) {
            ACAPI::Result<double> val = table->GetRadiusByReferenceId (refid);
            if (val.IsOk ()) {
                ParamHelpers::ConvertDoubleToParamValue (pval, "", val.Unwrap ());
                flag = true;
            }
        }
    }
    return true;
}

bool ReadTransitionPreferenceTable (bool& flag, ParamDictValue& paramByType, ACAPI::MEP::ConnectorShape& shape, ACAPI::MEP::UniqueID& tableID, double& bdiametr, double& ediametr)
{
    #ifdef MEPAPI_VERSION
    GS::UniString rawName = "";
    ACAPI::Result<PipeTransitionPreferenceTable> table = PipeTransitionPreferenceTable::Get (tableID);
    if (table.IsErr ()) {
        ACAPI_WriteReport (table.UnwrapErr ().text.c_str (), false);
        return false;
    }
    rawName = "{@mep:element set name}";
    if (paramByType.ContainsKey (rawName)) {
        ParamValue& pval = paramByType.Get (rawName);
        ParamHelpers::ConvertStringToParamValue (pval, "", table->GetName ());
        flag = true;
    }
    rawName = "{@mep:description}";
    if (paramByType.ContainsKey (rawName)) {
        uint32_t begrefid = 0;
        uint32_t endrefid = 0;
        bool flag_find = false;
        uint32_t s = table->GetSize ();
        for (uint32_t i = 0; i < s; ++i) {
            ACAPI::Result<double> bd = table->GetBeginDiameter (i);
            if (bd.IsErr ()) continue;
            if (!is_equal (bd.Unwrap (), bdiametr) && !is_equal (bd.Unwrap (), ediametr)) continue;
            ACAPI::Result<double> ed = table->GetEndDiameter (i);
            if (ed.IsErr ()) continue;
            if (!is_equal (ed.Unwrap (), ediametr) && !is_equal (ed.Unwrap (), bdiametr)) continue;
            ACAPI::Result<uint32_t> br = table->GetBeginReferenceId (i);
            if (br.IsOk ()) {
                begrefid = br.Unwrap ();
                ACAPI::Result<uint32_t> er = table->GetEndReferenceId (i);
                if (er.IsOk ()) {
                    endrefid = er.Unwrap ();
                    flag_find = true;
                }
            }
            break;
        }
        if (!flag_find) return false;
        ACAPI::Result<bool> res = table->IsRowValidByReferenceIds (begrefid, endrefid);
        if (res.IsErr ()) {
            return false;
        }
        if (!res.Unwrap ()) return false;
        ParamValue& pval = paramByType.Get (rawName);
        ACAPI::Result<GS::UniString> val = table->GetDescriptionByReferenceIds (begrefid, endrefid);
        if (val.IsOk ()) {
            ParamHelpers::ConvertStringToParamValue (pval, "", val.Unwrap ());
            flag = true;
        } else {
            ParamHelpers::ConvertStringToParamValue (pval, "", "");
            flag = true;
        }
    }
    return true;
    #else
    return false;
    #endif
}

#endif
}
#endif
