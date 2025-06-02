//------------ kuvbur 2022 ------------
#pragma once
#ifndef MEPV1_HPP
#define MEPV1_HPP
#if defined(AC_27) || defined(AC_28)
#include "CommonFunction.hpp"
#include "Definitions.hpp"
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

namespace MEPv1 {
    bool ReadMEP (const API_Elem_Head& elem_head, ParamDictValue& paramByType);
    void GetSubElementOfRouting (const API_Guid& elemGuid, GS::Array<API_Guid>& subelemGuid);

    void GetSubElement (const API_Guid& elemGuid, GS::Array<API_Guid>& subelemGuid);

    void GetPreferenceTable (const API_Elem_Head& elem_head);
}
#endif
#endif
