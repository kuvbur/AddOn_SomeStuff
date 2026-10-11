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

    struct RoomSurface {
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
    RoomSurfaceSnapshot ReadRoomSurfaces (const API_Elem_Head &head);

} // namespace Roombook
    #endif
#endif