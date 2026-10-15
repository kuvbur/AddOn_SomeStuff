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
    // Удаляет избыточные вершины сонаправленных рёбер с одинаковыми edge user data.
    // SDK сохраняет покрытие, отверстия и геометрию дуг; возвращает число удалённых вершин.
    USize CleanCollinearPolygonVertices (Geometry::Polygon2D &polygon);
    // Доводит до ядра только пол; затем очищает лишние вершины пола и потолка.
    // Стены — read-only источник конечных контактов.
    bool ExtendFloorContoursToCore (OtdRoom &room);
    // Добавляет одну стену в плоскости контакта; прямоугольные вырезы и отверстия задаёт проёмами.
    // Диагональные рёбра в координатах ось/отметка отвергаются без частичного добавления.
    bool BuildSurfaceWalls (const RoomSurfacePiece &piece, short floorInd, GS::Array<OtdWall> &walls);
    // Учитывает roomEdge, объединяет интервалы и доводит только крайние внешние концы.
    // contacts — неизменённый набор контактов комнаты, необходимый для сохранения разрывов.
    // Без частей основания сохраняет стену; невалидная/непредставимая часть даёт пустой результат.
    GS::Array<OtdWall> ClampSurfaceWallToParts (const OtdWall &wall,
                                                const API_Coord &wallBeg,
                                                const API_Coord &wallEnd,
                                                const GS::Array<API_WallPart> &parts,
                                                const GS::Array<Sector> &roomEdges,
                                                double roomBottom,
                                                const GS::Array<OtdWall> &contacts);
    // Сопоставляет вырезы с окнами/дверями того же основания, не меняя покрытие стены.
    // Native-неоднозначность отвергается; analyticOverlap сохраняет множественные IDs без ложного owner.
    bool LinkSurfaceOpenings (OtdWall &wall, const GS::Array<OtdWall> &openingWalls, bool analyticOverlap = false);
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
    // Для потолка номинальная отметка зоны участвует в выборе наравне с перекрытиями.
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
                                     GS::Array<OtdWall> *walls = nullptr,
                                     const GS::Array<OtdSlab> *prepared = nullptr);
} // namespace Roombook
#endif
