#pragma once
#include "roombook/Roombook.hpp"
#include "roombook/RoomSurfacePartition.hpp"

#ifdef ServerMainVers_2400
namespace Roombook {
    // Геометрические адаптеры без SDK-чтения, назначения материала и создания.
    // При отказе выход не меняется; zBottom — абсолютная отметка контакта.
    // Один параллелограмм без отверстий; линия и уклон задают контактную плоскость, не габарит грани.
    bool BuildSurfaceWall (const RoomSurfacePiece &piece, short floorInd, OtdWall &wall);
    // Добавляет прямоугольные полосы в плоскости контакта; вырезы и отверстия остаются пустыми.
    // Диагональные рёбра в координатах ось/отметка отвергаются без частичного добавления.
    bool BuildSurfaceWalls (const RoomSurfacePiece &piece, short floorInd, GS::Array<OtdWall> &walls);
    // Объединяет представимые стеновые контакты с одинаковым полным набором исходных граней.
    // Основание, геометрия, тело, полигон, материал снимка и направление нормали должны совпадать.
    // Чужие/непредставимые участки сохраняет; при успехе добавляет результат к grouped.
    // При ошибке объединения выход не меняется. Материалы и исходный partition не изменяет.
    bool GroupSurfaceWallPieces (const GS::Array<RoomSurfacePiece> &pieces,
                                 short floorInd,
                                 GS::Array<RoomSurfacePiece> &grouped);
    // Горизонтальные контакты с отверстиями; тип пола/потолка определяется нормалью зоны.
    bool BuildSurfaceSlabs (const RoomSurfacePiece &piece, short floorInd, GS::Array<OtdSlab> &slabs);
} // namespace Roombook
#endif
