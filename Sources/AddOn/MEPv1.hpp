//------------ kuvbur 2022 ------------
#pragma once
#ifndef MEPV1_HPP
    #define MEPV1_HPP
    #if defined(AC_27) || defined(AC_28) || defined(AC_29)
        #include "ACAPI/MEPAdapter.hpp"
        #include "ACAPI/MEPElement.hpp"
        #include "ACAPI/MEPModifiableElement.hpp"
        #include "ACAPI/MEPPipingPort.hpp"
        #include "ACAPI/MEPPort.hpp"
        #include "ACAPI/MEPRigidSegment.hpp"
        #include "ACAPI/MEPRigidSegmentDefault.hpp"
        #include "ACAPI/MEPRoutingElement.hpp"
        #include "ACAPI/MEPRoutingElementDefault.hpp"
        #include "ACAPI/MEPRoutingNode.hpp"
        #include "ACAPI/MEPRoutingNodeDefault.hpp"
        #include "ACAPI/MEPRoutingSegment.hpp"
        #include "ACAPI/MEPRoutingSegmentDefault.hpp"
        #include "ACAPI/MEPVentilationPort.hpp"
        #include "ACAPI/Result.hpp"
        #include "CommonFunction.hpp"
        #include "Definitions.hpp"
        #include "Helpers.hpp"
        #include "Propertycache.hpp"
        #if defined(AC_29)
            #include "ACAPI/MEPElbow.hpp"
            #include "ACAPI/MEPElbowDefault.hpp"
            #include "ACAPI/MEPPhysicalSystem.hpp"
            #include "ACAPI/MEPSystemGroup.hpp"
        #else
            #include "ACAPI/MEPBend.hpp"
            #include "ACAPI/MEPBendDefault.hpp"
        #endif
        #include "ACAPI/MEPAccessoryDefault.hpp"
        #include "ACAPI/MEPBranch.hpp"
        #include "ACAPI/MEPEquipmentDefault.hpp"
        #include "ACAPI/MEPFitting.hpp"
        #include "ACAPI/MEPFlexibleSegment.hpp"
        #include "ACAPI/MEPPreferenceTableContainerBase.hpp"
        #include "ACAPI/MEPTerminal.hpp"
        #include "ACAPI/MEPTransition.hpp"
        #include "ACAPI/MEPTransitionDefault.hpp"
        #include "ACAPI/MEPUniqueID.hpp"
        #include "GSUnID.hpp"

        #if defined(AC_27)
            #include "ACAPI/MEPCableCarrierPreferenceTableContainer.hpp"
            #include "ACAPI/MEPDuctPreferenceTableContainer.hpp"
            #include "ACAPI/MEPPipePreferenceTableContainer.hpp"
            #include "ACAPI/MEPPreferenceTableBase.hpp"
        #endif
        #if defined(AC_28) || defined(AC_29)
            #include "ACAPI/MEPCableCarrierSegmentPreferenceTable.hpp"
            #include "ACAPI/MEPCableCarrierSegmentPreferenceTableContainer.hpp"
            #include "ACAPI/MEPDuctBranchPreferenceTable.hpp"
            #include "ACAPI/MEPDuctBranchPreferenceTableContainer.hpp"
            #include "ACAPI/MEPDuctCircularSegmentPreferenceTable.hpp"
            #include "ACAPI/MEPDuctElbowPreferenceTable.hpp"
            #include "ACAPI/MEPDuctElbowPreferenceTableContainer.hpp"
            #include "ACAPI/MEPDuctRectangularSegmentPreferenceTable.hpp"
            #include "ACAPI/MEPDuctReferenceSet.hpp"
            #include "ACAPI/MEPDuctSegmentPreferenceTableContainer.hpp"
            #include "ACAPI/MEPPipeBranchPreferenceTable.hpp"
            #include "ACAPI/MEPPipeBranchPreferenceTableContainer.hpp"
            #include "ACAPI/MEPPipeElbowPreferenceTable.hpp"
            #include "ACAPI/MEPPipeElbowPreferenceTableContainer.hpp"
            #include "ACAPI/MEPPipeReferenceSet.hpp"
            #include "ACAPI/MEPPipeSegmentPreferenceTable.hpp"
            #include "ACAPI/MEPPipeSegmentPreferenceTableContainer.hpp"
            #include "ACAPI/MEPPreferenceTableContainerBase.hpp"
            #if defined(MEPAPI_VERSION)
                #include "ACAPI/MEPDuctTransitionPreferenceTable.hpp"
                #include "ACAPI/MEPDuctTransitionPreferenceTableContainer.hpp"
                #include "ACAPI/MEPPipeTransitionPreferenceTable.hpp"
                #include "ACAPI/MEPPipeTransitionPreferenceTableContainer.hpp"
            #endif
            #include <ACAPI/MEPEnums.hpp>
        #endif

namespace MEPv1 {

    static const GS::UniString rawnamephysicalsystemname = "{@mep:physical system name}";
    static const GS::UniString rawnamephysicalsystemgroupname = "{@mep:physical system group name}";
    static const GS::UniString rawnamereferencesetname = "{@mep:reference set name}";
    static const GS::UniString rawnametest = "{@mep:test}";
    static const GS::UniString rawnameinsulationthickness = "{@mep:insulation thickness}";
    static const GS::UniString rawnamelength = "{@mep:length}";
    static const GS::UniString rawnamedescription = "{@mep:description}";
    static const GS::UniString rawnamesystem = "{@mep:system}";
    static const GS::UniString rawnameroutinglength = "{@mep:routing length}";
    static const GS::UniString rawnamebendfactorradius = "{@mep:bend factor radius}";
    static const GS::UniString rawnamebendradius = "{@mep:bend radius}";
    static const GS::UniString rawnamewidth = "{@mep:width}";
    static const GS::UniString rawnameheight = "{@mep:height}";
    static const GS::UniString rawnamewallthickness = "{@mep:wall thickness}";
    static const GS::UniString rawnameshape = "{@mep:shape}";
    static const GS::UniString rawnameelementsetname = "{@mep:element set name}";
    static const GS::UniString rawnamediametr = "{@mep:diametr}";

    void GetSubElementOfRouting (const API_Guid &elemGuid, GS::Array<API_Guid> &subelemGuid);
    void GetSubElement (const API_Guid &elemGuid, GS::Array<API_Guid> &subelemGuid);

    bool ReadMEP (const API_Elem_Head &elem_head, ParamDictValue &paramByType);

    void ClearRoutingSubelemCache ();
        #if defined(AC_28) || defined(AC_29)
    bool GetMEPData (const API_Elem_Head &elem_head, ParamDictValue &paramByType);
    bool ReadTransitionData (const API_Guid &guid,
                             bool &flag,
                             ParamDictValue &paramByType,
                             ACAPI::MEP::ConnectorShape &shape,
                             ACAPI::MEP::UniqueID &transtableID,
                             double &bdiametr,
                             double &ediametr);
    bool ReadRoutingElementData (const ACAPI::MEP::UniqueID &elementID,
                                 bool &flag,
                                 ParamDictValue &paramByType,
                                 ACAPI::MEP::UniqueID &branchtableID);

    bool ReadBendData (const API_Guid &guid,
                       bool &flag,
                       ParamDictValue &paramByType,
                       ACAPI::MEP::ConnectorShape &shape,
                       ACAPI::MEP::UniqueID &bendtableID,
                       double &diametr,
                       double &radius);
    bool ReadRigidSegmentData (const API_Guid &guid,
                               bool &flag,
                               ParamDictValue &paramByType,
                               ACAPI::MEP::UniqueID &segmentId);
    bool ReadRoutingSegmentData (const ACAPI::MEP::UniqueID &segmentId,
                                 bool &flag,
                                 ParamDictValue &paramByType,
                                 ACAPI::MEP::ConnectorShape &shape,
                                 ACAPI::MEP::UniqueID &tableID,
                                 uint32_t &refid);

    bool ReadDuctSegmentPreferenceTable (bool &flag,
                                         ParamDictValue &paramByType,
                                         ACAPI::MEP::ConnectorShape &shape,
                                         ACAPI::MEP::UniqueID &tableID,
                                         uint32_t &refid);
    bool ReadDuctBendPreferenceTable (bool &flag,
                                      ParamDictValue &paramByType,
                                      ACAPI::MEP::ConnectorShape &shape,
                                      ACAPI::MEP::UniqueID &tableID,
                                      double &diametr,
                                      double &radius);

    bool ReadPipeSegmentPreferenceTable (bool &flag,
                                         ParamDictValue &paramByType,
                                         ACAPI::MEP::ConnectorShape &shape,
                                         ACAPI::MEP::UniqueID &tableID,
                                         uint32_t &refid);
    bool ReadPipeBendPreferenceTable (bool &flag,
                                      ParamDictValue &paramByType,
                                      ACAPI::MEP::ConnectorShape &shape,
                                      ACAPI::MEP::UniqueID &tableID,
                                      double &diametr,
                                      double &radius);
    bool ReadTransitionPreferenceTable (bool &flag,
                                        ParamDictValue &paramByType,
                                        ACAPI::MEP::ConnectorShape &shape,
                                        ACAPI::MEP::UniqueID &tableID,
                                        double &bdiametr,
                                        double &ediametr);
        #endif

} // namespace MEPv1
    #endif
#endif
