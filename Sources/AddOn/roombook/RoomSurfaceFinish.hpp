#pragma once
#include "roombook/Roombook.hpp"
#include "roombook/RoomSurfacePartition.hpp"

#ifdef ServerMainVers_2400
namespace Roombook {
    // Геометрические адаптеры без SDK-чтения, назначения материала и создания.
    // При отказе выход не меняется; zBottom — абсолютная отметка контакта.
    // Один параллелограмм без отверстий; линия и уклон задают контактную плоскость, не габарит грани.
    bool BuildSurfaceWall (const RoomSurfacePiece &piece, short floorInd, OtdWall &wall);
    // После правил вычитает только принятое конечное покрытие боковин перепадов.
    // При непредставимом остатке сохраняет исходный контакт и возвращает false.
    bool DeduplicateAcceptedSlabContacts (OtdRoom &room);
    // Добавляет одну стену в плоскости контакта; прямоугольные вырезы и отверстия задаёт проёмами.
    // Диагональные рёбра в координатах ось/отметка отвергаются без частичного добавления.
    bool BuildSurfaceWalls (const RoomSurfacePiece &piece, short floorInd, GS::Array<OtdWall> &walls);
    // Сопоставляет вырезы с окнами/дверями того же основания, не меняя покрытие стены.
    // При неоднозначности или непредставимой подрезке исходные проёмы не меняются.
    bool LinkSurfaceOpenings (OtdWall &wall, const GS::Array<OtdWall> &openingWalls);
    // Объединяет стеновые контакты с одинаковым полным набором исходных граней до проверки writer.
    // Основание, геометрия, тело, полигон, материал снимка и направление нормали должны совпадать.
    // Чужие/непредставимые участки сохраняет; при успехе добавляет результат к grouped.
    // При ошибке объединения выход не меняется. Материалы и исходный partition не изменяет.
    bool GroupSurfaceWallPieces (const GS::Array<RoomSurfacePiece> &pieces,
                                 short floorInd,
                                 GS::Array<RoomSurfacePiece> &grouped);
    // Горизонтальные контакты с отверстиями; тип пола/потолка определяется нормалью зоны.
    bool BuildSurfaceSlabs (const RoomSurfacePiece &piece, short floorInd, GS::Array<OtdSlab> &slabs);
    // XY-покрытие комнаты пересекается с реальными гранями оснований; Z остаётся исходным.
    // Прежние приоритет по убыванию, наибольшие части, исключение отверстий и опциональные остаток/перепады.
    // Владелец при равных отметках неоднозначен; фактическая Z грани не округляется.
    bool BuildProjectedSlabFinishes (const RoomSurfaceSnapshot &zone,
                                     const GS::Array<RoomSurfaceSource> &sources,
                                     const GS::Array<API_Guid> &roles,
                                     TypeOtd type,
                                     short floorInd,
                                     GS::Array<OtdSlab> &slabs,
                                     double &rejectedArea,
                                     const OtdSlab *nominal = nullptr,
                                     bool onlyOnSlab = true,
                                     GS::Array<OtdWall> *walls = nullptr);
} // namespace Roombook
#endif
