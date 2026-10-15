#include "roombook/RoomSurfaceCandidates.hpp"

#ifdef ServerMainVers_2400
    #include "CommonFunction.hpp"
    #include "OnExit.hpp"

namespace Roombook {
    namespace {
        void CandidateError (
            RoomSurfaceCandidates &result, const API_Guid &guid, const char *stage, GSErrCode code, Int32 index = 0) {
            result.errors.Push ({guid, GS::UniString (stage), code, index});
        }

        bool IsFinish (const API_Guid &guid,
                       const GS::Array<API_Guid> &finishClassGuids,
                       RoomSurfaceCandidates &result,
                       bool &valid,
                       RoomSurfaceCache *cache) {
            const ClassificationRead read = ReadClassificationItems (guid, cache);
            valid = read.error == NoError;
            if (!valid) {
                CandidateError (result, guid, "candidateClassification", read.error);
                return false;
            }
            // Как Class_IsElementFinClass, но ошибка чтения не означает «не отделка».
            for (const auto &item : read.items) {
                if (finishClassGuids.Contains (item.second)) {
                    if (!result.excludedFinish.Contains (guid))
                        result.excludedFinish.Push (guid);
                    return true;
                }
            }
            return false;
        }

        void ReadCandidate (const API_Elem_Head &base,
                            const API_Elem_Head &geometry,
                            const GS::Array<API_Guid> &finishClassGuids,
                            RoomSurfaceCandidates &result,
                            RoomSurfaceCache *cache) {
            if (geometry.guid != base.guid) {
                bool valid = false;
                if (IsFinish (geometry.guid, finishClassGuids, result, valid, cache) || !valid)
                    return;
            }
            for (const auto &source : result.sources) {
                if (source.baseGuid == base.guid && source.geometryGuid == geometry.guid)
                    return;
            }
            RoomSurfaceSource source;
            source.baseGuid = base.guid;
            source.baseType = GetElemTypeID (base);
            source.geometryGuid = geometry.guid;
            source.snapshot = ReadRoomSurfaces (geometry, cache);
            if (!source.snapshot.IsComplete ())
                CandidateError (result, geometry.guid, "candidateSurfaceSnapshot", APIERR_GENERAL);
            result.sources.Push (std::move (source));
        }

        template <class Segment>
        void ReadSegments (const API_Elem_Head &base,
                           const Segment *segments,
                           UInt32 count,
                           API_ElemTypeID expectedType,
                           const GS::Array<API_Guid> &finishClassGuids,
                           RoomSurfaceCandidates &result,
                           RoomSurfaceCache *cache) {
            const GSSize bytes =
                segments != nullptr ? BMGetPtrSize (reinterpret_cast<GSPtr> (const_cast<Segment *> (segments))) : 0;
            if (count == 0 || bytes <= 0 || bytes % static_cast<GSSize> (sizeof (Segment)) != 0 ||
                static_cast<UInt64> (bytes) / sizeof (Segment) != count) {
                CandidateError (result, base.guid, "candidateSegmentCount", APIERR_BADPARS);
                return;
            }
            GS::Array<API_Guid> seen;
            for (UInt32 i = 0; i < count; ++i) {
                const auto &segment = segments[i];
                if (segment.head.guid == APINULLGuid || segment.owner != base.guid ||
                    GetElemTypeID (segment.head) != expectedType || seen.Contains (segment.head.guid)) {
                    CandidateError (
                        result, base.guid, "candidateSegmentHeader", APIERR_BADPARS, static_cast<Int32> (i));
                    continue;
                }
                seen.Push (segment.head.guid);
                ReadCandidate (base, segment.head, finishClassGuids, result, cache);
            }
        }

        void ReadBase (API_Guid guid,
                       API_ElemTypeID type,
                       const GS::Array<API_Guid> &finishClassGuids,
                       RoomSurfaceCandidates &result,
                       RoomSurfaceCache *cache) {
            const ElementRead read = ReadElement (guid, cache);
            if (read.error != NoError || GetElemTypeID (read.element.header) != type) {
                CandidateError (result, guid, "candidateElement", read.error != NoError ? read.error : APIERR_BADID);
                return;
            }
            bool valid = false;
            if (IsFinish (guid, finishClassGuids, result, valid, cache) || !valid)
                return;
            if (type == API_WallID || type == API_SlabID) {
                // Wall/Slab готовятся после проёмов и legacy-контактов; discovery не читает их 3D.
                RoomSurfaceSource source;
                source.baseGuid = guid;
                source.baseType = type;
                source.geometryGuid = guid;
                result.sources.Push (std::move (source));
                return;
            }
            API_ElementMemo memo = {};
            const GS::OnExit memoGuard ([&memo] () { ACAPI_DisposeElemMemoHdls (&memo); });

            const GSErrCode memoError = ACAPI_Element_GetMemo (
                guid, &memo, type == API_ColumnID ? APIMemoMask_ColumnSegment : APIMemoMask_BeamSegment);
            if (memoError != NoError) {
                CandidateError (result, guid, "candidateSegmentMemo", memoError);
                return;
            }
            if (type == API_ColumnID)
                ReadSegments (read.element.header,
                              memo.columnSegments,
                              read.element.column.nSegments,
                              API_ColumnSegmentID,
                              finishClassGuids,
                              result,
                              cache);
            else
                ReadSegments (read.element.header,
                              memo.beamSegments,
                              read.element.beam.nSegments,
                              API_BeamSegmentID,
                              finishClassGuids,
                              result,
                              cache);
        }
    } // namespace

    RoomSurfaceCandidates ReadRoomSurfaceCandidates (const API_Elem_Head &zone,
                                                     const GS::Array<API_Guid> &finishClassGuids,
                                                     RoomSurfaceCache *cache) {
        RoomSurfaceCandidates result;
        result.zoneGuid = zone.guid;
        if (zone.guid == APINULLGuid || GetElemTypeID (zone) != API_ZoneID) {
            CandidateError (result, zone.guid, "candidateZoneHeader", APIERR_BADID);
            return result;
        }
        RoomRelationValues relData;
        const GSErrCode error = ReadZoneRelations (zone.guid, cache, relData);
        if (error != NoError) {
            CandidateError (result, zone.guid, "candidateRoomRelations", error);
            return result;
        }
        for (const API_BeamPart &part : relData.beamPart)
            result.beamParts.Push (part);
        GS::Array<API_Guid> seen;
        for (API_ElemTypeID type : {API_WallID, API_ColumnID, API_SlabID, API_BeamID}) {
            GS::Array<API_Guid> guids;
            if (const auto *group = relData.elementsGroupedByType.GetPtr (type))
                guids = *group;
            if (type == API_BeamID) {
                for (const auto &part : relData.beamPart)
                    if (!guids.Contains (part.guid))
                        guids.Push (part.guid);
            }
            for (const API_Guid &guid : guids) {
                if (guid == APINULLGuid) {
                    CandidateError (result, zone.guid, "candidateNullGuid", APIERR_BADPARS);
                    continue;
                }
                if (seen.Contains (guid))
                    continue;
                seen.Push (guid);
                ReadBase (guid, type, finishClassGuids, result, cache);
            }
        }
        result.complete = result.errors.IsEmpty ();
        return result;
    }
} // namespace Roombook
#endif
