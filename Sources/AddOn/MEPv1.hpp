//------------ kuvbur 2022 ------------
#pragma once
#ifndef MEPV1_HPP
#define MEPV1_HPP
#if defined(AC_27) || defined(AC_28) || defined(AC_29)
#include "CommonFunction.hpp"
#include "Definitions.hpp"
#include "Helpers.hpp"
// ACAPI

#include "ACAPI/Result.hpp"

// MEPAPI
#include "ACAPI/MEPAdapter.hpp"
#include "ACAPI/MEPElement.hpp"
#include "ACAPI/MEPModifiableElement.hpp"
#include "ACAPI/MEPPort.hpp"
#include "ACAPI/MEPVentilationPort.hpp"
#include "ACAPI/MEPPipingPort.hpp"
#include "ACAPI/MEPRoutingElement.hpp"
#include "ACAPI/MEPRoutingElementDefault.hpp"
#include "ACAPI/MEPRoutingSegment.hpp"
#include "ACAPI/MEPRoutingSegmentDefault.hpp"
#include "ACAPI/MEPRigidSegment.hpp"
#include "ACAPI/MEPRigidSegmentDefault.hpp"
#include "ACAPI/MEPRoutingNode.hpp"
#include "ACAPI/MEPRoutingNodeDefault.hpp"
#include "ACAPI/MEPBend.hpp"
#include "ACAPI/MEPBendDefault.hpp"
#include "ACAPI/MEPTransition.hpp"
#include "ACAPI/MEPTransitionDefault.hpp"
#include "ACAPI/MEPBranch.hpp"
#include "ACAPI/MEPTerminal.hpp"
#include "ACAPI/MEPFitting.hpp"
#include "ACAPI/MEPFlexibleSegment.hpp"
#include "ACAPI/MEPAccessoryDefault.hpp"
#include "ACAPI/MEPEquipmentDefault.hpp"
#include "ACAPI/MEPUniqueID.hpp"
#include "ACAPI/MEPPreferenceTableContainerBase.hpp"
#include "GSUnID.hpp"

#if defined(AC_27)
#include "ACAPI/MEPDuctPreferenceTableContainer.hpp"
#include "ACAPI/MEPPipePreferenceTableContainer.hpp"
#include "ACAPI/MEPCableCarrierPreferenceTableContainer.hpp"
#include "ACAPI/MEPPreferenceTableBase.hpp"
#endif
#if defined(AC_28) || defined(AC_29)
#include "ACAPI/MEPPreferenceTableContainerBase.hpp"
#include "ACAPI/MEPDuctSegmentPreferenceTableContainer.hpp"
#include "ACAPI/MEPPipeSegmentPreferenceTableContainer.hpp"
#include "ACAPI/MEPCableCarrierSegmentPreferenceTableContainer.hpp"
#include "ACAPI/MEPPipeSegmentPreferenceTable.hpp"
#include "ACAPI/MEPDuctCircularSegmentPreferenceTable.hpp"
#include "ACAPI/MEPDuctRectangularSegmentPreferenceTable.hpp"
#include "ACAPI/MEPCableCarrierSegmentPreferenceTable.hpp"
#include "ACAPI/MEPPipeElbowPreferenceTableContainer.hpp"
#include "ACAPI/MEPPipeElbowPreferenceTable.hpp"
#include "ACAPI/MEPDuctReferenceSet.hpp"
#include "ACAPI/MEPPipeReferenceSet.hpp"
#include "ACAPI/MEPDuctElbowPreferenceTableContainer.hpp"
#include "ACAPI/MEPDuctElbowPreferenceTable.hpp"
#include "ACAPI/MEPPipeBranchPreferenceTableContainer.hpp"
#include "ACAPI/MEPPipeBranchPreferenceTable.hpp"
#include "ACAPI/MEPDuctBranchPreferenceTableContainer.hpp"
#include "ACAPI/MEPDuctBranchPreferenceTable.hpp"
#if defined(MEPAPI_VERSION)
#include "ACAPI/MEPPipeTransitionPreferenceTableContainer.hpp"
#include "ACAPI/MEPPipeTransitionPreferenceTable.hpp"
#include "ACAPI/MEPDuctTransitionPreferenceTableContainer.hpp"
#include "ACAPI/MEPDuctTransitionPreferenceTable.hpp"
#endif
#include <ACAPI/MEPEnums.hpp>
#endif

namespace MEPv1
{
void GetSubElementOfRouting (const API_Guid& elemGuid, GS::Array<API_Guid>& subelemGuid);
void GetSubElement (const API_Guid& elemGuid, GS::Array<API_Guid>& subelemGuid);

bool ReadMEP (const API_Elem_Head& elem_head, ParamDictValue& paramByType);
#if defined (AC_28)
bool GetMEPData (const API_Elem_Head& elem_head, ParamDictValue& paramByType);
bool ReadTransitionData (const API_Guid& guid, bool& flag, ParamDictValue& paramByType, ACAPI::MEP::ConnectorShape& shape, ACAPI::MEP::UniqueID& transtableID, double& bdiametr, double& ediametr);
bool ReadRoutingElementData (const ACAPI::MEP::UniqueID& elementID, bool& flag, ParamDictValue& paramByType, ACAPI::MEP::UniqueID& branchtableID);

bool ReadBendData (const API_Guid& guid, bool& flag, ParamDictValue& paramByType, ACAPI::MEP::ConnectorShape& shape, ACAPI::MEP::UniqueID& bendtableID, double& diametr, double& radius);
bool ReadRigidSegmentData (const API_Guid& guid, bool& flag, ParamDictValue& paramByType, ACAPI::MEP::UniqueID& segmentId);
bool ReadRoutingSegmentData (const ACAPI::MEP::UniqueID& segmentId, bool& flag, ParamDictValue& paramByType, ACAPI::MEP::ConnectorShape& shape, ACAPI::MEP::UniqueID& tableID, uint32_t& refid);

bool ReadDuctSegmentPreferenceTable (bool& flag, ParamDictValue& paramByType, ACAPI::MEP::ConnectorShape& shape, ACAPI::MEP::UniqueID& tableID, uint32_t& refid);
bool ReadDuctBendPreferenceTable (bool& flag, ParamDictValue& paramByType, ACAPI::MEP::ConnectorShape& shape, ACAPI::MEP::UniqueID& tableID, double& diametr, double& radius);


bool ReadPipeSegmentPreferenceTable (bool& flag, ParamDictValue& paramByType, ACAPI::MEP::ConnectorShape& shape, ACAPI::MEP::UniqueID& tableID, uint32_t& refid);
bool ReadPipeBendPreferenceTable (bool& flag, ParamDictValue& paramByType, ACAPI::MEP::ConnectorShape& shape, ACAPI::MEP::UniqueID& tableID, double& diametr, double& radius);
bool ReadTransitionPreferenceTable (bool& flag, ParamDictValue& paramByType, ACAPI::MEP::ConnectorShape& shape, ACAPI::MEP::UniqueID& tableID, double& bdiametr, double& ediametr);
#endif


}
#endif
#endif
