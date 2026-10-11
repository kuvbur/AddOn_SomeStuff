#pragma once
#if !defined(ROOMBOOK_SURFACE_CANDIDATES_HPP)
    #define ROOMBOOK_SURFACE_CANDIDATES_HPP
    #include "roombook/RoomSurfacePartition.hpp"

    #ifdef ServerMainVers_2400
namespace Roombook {

    struct RoomSurfaceCandidateError {
        API_Guid guid = APINULLGuid;
        GS::UniString stage;
        GSErrCode code = NoError;
        Int32 index = 0;
    };

    struct RoomSurfaceCandidates {
        API_Guid zoneGuid = APINULLGuid;
        bool complete = false;
        GS::Array<RoomSurfaceSource> sources;
        // Участки балок в зоне из API_RoomRelation.beamPart (копируются до disposal).
        GS::Array<API_BeamPart> beamParts;
        GS::Array<API_Guid> excludedFinish;
        GS::Array<RoomSurfaceCandidateError> errors;
    };

    // Набор SDK relations, не доказательство полноты произвольных касаний в 3D.
    RoomSurfaceCandidates ReadRoomSurfaceCandidates (const API_Elem_Head &zone,
                                                     const GS::Array<API_Guid> &finishClassGuids);

} // namespace Roombook
    #endif
#endif