#pragma once
#if !defined(ROOMBOOK_SURFACE_PART_HPP)
    #define ROOMBOOK_SURFACE_PART_HPP
    #include "roombook/RoomSurfaces.hpp"

    #ifdef ServerMainVers_2400
namespace Roombook {

    struct RoomSurfaceSource {
        API_Guid baseGuid = APINULLGuid;
        API_ElemTypeID baseType = API_ZombieElemID;
        API_Guid geometryGuid = APINULLGuid;
        RoomSurfaceSnapshot snapshot;
        // Параметрическая геометрия сознательно игнорирует SEO/подрезки (решение владельца).
        RoomSurfaceOrigin origin = RoomSurfaceOrigin::Native3D;
        // Диагностическая причина маршрута источника; записывается, не читается.
        GS::UniString routeReason;
    };

    struct RoomSurfaceSourceFace {
        API_Guid baseGuid = APINULLGuid;
        API_ElemTypeID baseType = API_ZombieElemID;
        API_Guid geometryGuid = APINULLGuid;
        Int32 bodyIndex = 0;
        Int32 polygonIndex = 0;
        // Материал модельной грани; сам по себе не устанавливает слой многослойного основания.
        API_AttributeIndex modelMaterialIndex = {};
        RoomSurfaceOrigin origin = RoomSurfaceOrigin::Native3D;
        Int32 analyticFaceId = 0;
        Int32 preparedWallIndex = -1;
    };

    enum class RoomSurfacePieceStatus { Matched, Unmatched, Ambiguous };

    struct RoomSurfacePieceContour {
        bool hole = false;
        GS::Array<API_Coord3D> vertices;
    };

    struct RoomSurfacePiece {
        Int32 zoneBodyIndex = 0;
        Int32 zonePolygonIndex = 0;
        RoomSurfacePieceStatus status = RoomSurfacePieceStatus::Unmatched;
        API_Guid baseGuid = APINULLGuid;
        API_ElemTypeID baseType = API_ZombieElemID;
        API_Coord3D normal = {};
        double planeOffset = 0;
        double area = 0;
        GS::Array<RoomSurfacePieceContour> contours;
        // Ссылки на все реальные грани участка; покрытие не доказывает компонент/стройматериал.
        GS::Array<RoomSurfaceSourceFace> sources;
    };

    struct RoomSurfacePartition {
        API_Guid zoneGuid = APINULLGuid;
        bool complete = false;
        GS::Array<RoomSurfacePiece> pieces;
        GS::Array<RoomSurfaceReadError> errors;
    };

    // Не читает SDK-базу и не создаёт элементы; входы — собственные проверяемые снимки.
    // Несколько разных оснований дают Ambiguous без выбора первого; незаявленные площади — Unmatched.
    // complete подтверждает разбиение снимков, но не разрешимость всех оснований или writer-геометрии.
    RoomSurfacePartition PartitionRoomSurfaces (const RoomSurfaceSnapshot &zone,
                                                const GS::Array<RoomSurfaceSource> &sources);

    enum class RoomSurfaceBeamFace { Bottom, LeftSide, RightSide };

    struct RoomSurfaceBeamPiece {
        RoomSurfaceBeamFace face = RoomSurfaceBeamFace::Bottom;
        double area = 0;
        GS::Array<API_Coord3D> vertices;
    };

    struct RoomSurfaceBeamFinish {
        API_Guid baseGuid = APINULLGuid;
        API_Guid geometryGuid = APINULLGuid;
        double tBeg = 0;
        double tEnd = 0;
        GS::Array<RoomSurfaceBeamPiece> pieces;
    };

    struct RoomSurfaceBeamPartitions {
        bool complete = false;
        GS::Array<RoomSurfaceBeamFinish> beams;
        GS::Array<RoomSurfaceReadError> errors;
    };

    // beamPart выбирает балку/связную часть; при zone != nullptr низ и бока пересекаются с 3D-зоной.
    // tBeg/tEnd — исходные проекции SDK AC25, не параметры [0,1]; переводятся в расстояния по оси.
    // Обрезка по вертикали комнаты [zBottom, zTop]. Прямые балки с постоянным сечением.
    // Без zone применяется прежняя обрезка по осевому интервалу; с zone учитываются высотные ступени.
    // Ошибочное сечение или отверстие в конечном участке даёт явную ошибку без частичных pieces.
    RoomSurfaceBeamFinish BuildRoomBeamFinish (const RoomSurfaceSource &beamSource,
                                               const API_Coord &beg,
                                               const API_Coord &end,
                                               double tBeg,
                                               double tEnd,
                                               double zBottom,
                                               double zTop,
                                               RoomSurfaceBeamPartitions &result,
                                               const RoomSurfaceSnapshot *zone = nullptr);

} // namespace Roombook
    #endif
#endif