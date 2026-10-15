#pragma once
#if !defined(ROOMBOOK_SURFACE_HPP)
    #define ROOMBOOK_SURFACE_HPP
    #include "ACAPinc.h"

    #ifdef ServerMainVers_2400
        #ifdef ServerMainVers_2700
            #define ACAPI_Element_Get3DInfo ACAPI_ModelAccess_Get3DInfo
            #define ACAPI_3D_GetComponent ACAPI_ModelAccess_GetComponent
        #endif

namespace Roombook {

    enum class RoomSurfaceStatus { NotRead, Extracted, Empty, ReadError, UnsupportedVersion };

    struct RoomSurfaceVertex {
        API_Coord3D local = {};
        API_Coord3D world = {};
        Int32 index = 0;
        Int32 signedEdgeIndex = 0;
    };

    struct RoomSurfaceContour {
        bool hole = false;
        GS::Array<RoomSurfaceVertex> vertices;
    };

    // Native3D хранит SDK-индексы; PreparedAnalytic — собственные конечные грани без native UMAT.
    enum class RoomSurfaceOrigin { Native3D, PreparedAnalytic };

    struct RoomSurface {
        RoomSurfaceOrigin origin = RoomSurfaceOrigin::Native3D;
        Int32 analyticFaceId = 0;
        Int32 preparedWallIndex = -1;
        Int32 bodyIndex = 0;
        Int32 polygonIndex = 0;
        API_AttributeIndex modelMaterialIndex = {};
        Int32 flags = 0;
        Int32 signedNormalIndex = 0;
        API_Coord3D normalLocal = {};
        API_Coord3D normalWorld = {};
        double planeOffset = 0;
        GS::Array<RoomSurfaceContour> contours;
    };

    struct RoomSurfaceBody {
        Int32 index = 0;
        Int32 polygonCount = 0;
        Int32 vertexCount = 0;
        API_Tranmat matrix = {};
        API_Guid parentGuid = APINULLGuid;
        API_ElemTypeID parentType = API_ZombieElemID;
        GS::Array<RoomSurface> surfaces;
    };

    struct RoomSurfaceReadError {
        GS::UniString stage;
        GSErrCode code = NoError;
        Int32 body = 0;
        Int32 polygon = 0;
        Int32 index = 0;
    };

    // Индексы тела/полигона нужны только для диагностики текущего снимка, не для постоянных связей.
    struct RoomSurfaceSnapshot {
        API_Guid guid = APINULLGuid;
        RoomSurfaceStatus status = RoomSurfaceStatus::NotRead;
        Int32 firstBody = 0;
        Int32 lastBody = 0;
        GS::Array<RoomSurfaceBody> bodies;
        GS::Array<Int32> deletedBodies;
        GS::Array<RoomSurfaceReadError> errors;

        bool IsComplete () const {
            return status == RoomSurfaceStatus::Extracted || status == RoomSurfaceStatus::Empty;
        }
    };

    // Преобразует собственные контуры и ориентированную нормаль; вырожденную матрицу не принимает.
    bool TransformRoomSurface (const API_Tranmat &matrix, RoomSurface &surface);

    struct RoomSurfaceReadCounters {
        UInt64 analyticWallSources = 0;
        UInt64 analyticSlabSources = 0;
        UInt64 analyticFaces = 0;
        UInt64 nativeFallbackSources = 0;
        UInt64 wallSnapshotReads = 0;
        UInt64 slabSnapshotReads = 0;
        UInt64 snapshotReads = 0;
        UInt64 snapshotHits = 0;
        UInt64 boundsReads = 0;
        UInt64 boundsHits = 0;
        UInt64 componentReads = 0;
        UInt64 componentHits = 0;
        UInt64 elementReads = 0;
        UInt64 elementHits = 0;
        UInt64 classificationReads = 0;
        UInt64 classificationHits = 0;
        UInt64 relationReads = 0;
        UInt64 relationHits = 0;
        UInt64 connectedReads = 0;
        UInt64 connectedHits = 0;
    };

    struct RoomSurfaceZBounds {
        bool complete = false;
        double bottom = 0;
        double top = 0;
    };

    // Оригинальный элемент: кэшируются и успешное чтение, и ошибка SDK.
    struct ElementRead {
        API_Element element = {};
        GSErrCode error = NoError;
    };

    // Сырые классификационные пары с кодом ошибки SDK; семантику ошибки выбирает потребитель.
    struct ClassificationRead {
        GS::Array<GS::Pair<API_Guid, API_Guid>> items;
        GSErrCode error = NoError;
    };

    // Value-снимок API_RoomRelation: перечисления и части не владеют хендлами, niches не читаем.
    struct RoomRelationValues {
        GS::HashTable<API_ElemTypeID, GS::Array<API_Guid>> elementsGroupedByType;
        GS::Array<API_WallPart> wallPart;
        GS::Array<API_BeamPart> beamPart;
    };

    struct ConnectedElementsRead {
        API_ElemTypeID type = API_ZombieElemID;
        API_ElemFilterFlags filter = APIFilt_None;
        API_ElemVariationID variation = APIVarId_Generic;
        API_Guid renovationFilterGuid = APINULLGuid;
        GS::Array<API_Guid> items;
        GSErrCode error = NoError;
    };

    // Только один этап чтения: после записи модели этот кэш больше не используется.
    struct RoomSurfaceCache {
        GS::HashTable<API_Guid, RoomSurfaceSnapshot> snapshots;
        GS::HashTable<API_Guid, RoomSurfaceZBounds> bounds;
        // Кэши read-only стадии: действительны до первой записи модели.
        GS::HashTable<API_Guid, ElementRead> elements;
        GS::HashTable<API_Guid, ClassificationRead> classifications;
        GS::HashTable<API_Guid, RoomRelationValues> relations;
        GS::HashTable<API_Guid, GS::Array<ConnectedElementsRead>> connectedElements;
        RoomSurfaceReadCounters counters;
    };

    RoomSurfaceSnapshot ReadRoomSurfaces (const API_Elem_Head &head, RoomSurfaceCache *cache = nullptr);
    RoomSurfaceZBounds ReadRoomSurfaceZBounds (const API_Elem_Head &head, RoomSurfaceCache *cache = nullptr);
    ElementRead ReadElement (const API_Guid &guid, RoomSurfaceCache *cache = nullptr);
    ClassificationRead ReadClassificationItems (const API_Guid &guid, RoomSurfaceCache *cache = nullptr);
    GSErrCode ReadZoneRelations (const API_Guid &zoneGuid, RoomSurfaceCache *cache, RoomRelationValues &values);

    // Callback позволяет regression проверить настоящий storage-путь без вызовов SDK.
    template <class Read>
    ConnectedElementsRead ReadConnectedElementsCached (const API_Guid &guid,
                                                       API_ElemTypeID type,
                                                       API_ElemFilterFlags filter,
                                                       API_ElemVariationID variation,
                                                       const API_Guid &renovationFilterGuid,
                                                       RoomSurfaceCache *cache,
                                                       Read &&read) {
        if (cache != nullptr) {
            if (const auto *entries = cache->connectedElements.GetPtr (guid)) {
                for (const auto &entry : *entries) {
                    if (entry.type == type && entry.filter == filter && entry.variation == variation &&
                        entry.renovationFilterGuid == renovationFilterGuid) {
                        ++cache->counters.connectedHits;
                        return entry;
                    }
                }
            }
            ++cache->counters.connectedReads;
        }
        ConnectedElementsRead result;
        result.type = type;
        result.filter = filter;
        result.variation = variation;
        result.renovationFilterGuid = renovationFilterGuid;
        result.error = read (result.items);
        if (cache != nullptr) {
            if (!cache->connectedElements.ContainsKey (guid))
                cache->connectedElements.Add (guid, GS::Array<ConnectedElementsRead> ());
            cache->connectedElements.Get (guid).Push (result);
        }
        return result;
    }

    ConnectedElementsRead ReadConnectedElements (const API_Guid &guid,
                                                 API_ElemTypeID type,
                                                 API_ElemFilterFlags filter,
                                                 RoomSurfaceCache *cache = nullptr,
                                                 API_ElemVariationID variation = APIVarId_Generic,
                                                 const API_Guid &renovationFilterGuid = APINULLGuid);

} // namespace Roombook
    #endif
#endif