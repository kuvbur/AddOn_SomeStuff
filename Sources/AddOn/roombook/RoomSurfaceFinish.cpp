#include "ACAPinc.h"

#include "roombook/RoomSurfaceFinish.hpp"

#include "roombook/FloorCoreGeometry.hpp"

#ifdef ServerMainVers_2400
    #include <algorithm>
    #include <cmath>
    #include <limits>
    #include <vector>

namespace Roombook {
    USize CleanCollinearPolygonVertices (Geometry::Polygon2D &polygon) {
        if (polygon.IsEmpty ())
            return 0;
        const USize before = polygon.GetVertexNum ();
        Geometry::Polygon2D::RemoveSameDirEdgesWithSameEdgeUserData (polygon);
        return before - polygon.GetVertexNum ();
    }

    bool ExtendFloorContoursToCore (OtdRoom &room) {
        using namespace FloorCoreGeometry;
        GS::Array<OtdSlab> result;
        bool complete = true;
        for (const auto &floor : room.otdslab) {
            if (!floor.isValid || floor.type != Floor || floor.poly.IsEmpty ()) {
                result.Push (floor);
                continue;
            }
            std::vector<Contact> contacts;
            std::vector<API_Guid> owners;
            for (const auto &wall : room.otdwall) {
                if (!wall.isValid || wall.base_guid == APINULLGuid || !wall.surface_geometry ||
                    wall.type == Reveal_Main || wall.type == Reveal_Up || wall.type == Reveal_Down ||
                    floor.zBottom < wall.zBottom - Tolerance || floor.zBottom >= wall.zBottom + wall.height - Tolerance)
                    continue;
                auto owner = std::find (owners.begin (), owners.end (), wall.base_guid);
                if (owner == owners.end ()) {
                    owners.push_back (wall.base_guid);
                    owner = owners.end () - 1;
                }
                const std::size_t ownerIndex = static_cast<std::size_t> (owner - owners.begin ());
                const double dz = floor.zBottom - wall.zBottom;
                const Point begin = {wall.begC.x + dz * wall.rise_x, wall.begC.y + dz * wall.rise_y};
                const Point end = {wall.endC.x + dz * wall.rise_x, wall.endC.y + dz * wall.rise_y};
                const double length = Length (end - begin);
                if (length <= Tolerance)
                    continue;
                const Point dir = (end - begin) * (1 / length);
                // Не расширяем под открытым проёмом на отметке пола.
                std::vector<std::pair<double, double>> ranges = {{0, length}};
                for (const auto &opening : wall.openings) {
                    if (opening.zBottom > floor.zBottom + Tolerance ||
                        opening.zBottom + opening.height <= floor.zBottom + Tolerance)
                        continue;
                    const double a = std::max (0.0, opening.objLoc - opening.width / 2);
                    const double b = std::min (length, opening.objLoc + opening.width / 2);
                    if (b <= a)
                        continue;
                    std::vector<std::pair<double, double>> next;
                    for (const auto &range : ranges) {
                        if (b <= range.first || a >= range.second) {
                            next.push_back (range);
                            continue;
                        }
                        if (a > range.first)
                            next.push_back ({range.first, a});
                        if (b < range.second)
                            next.push_back ({b, range.second});
                    }
                    ranges = std::move (next);
                }
                const double offset = wall.base_type == API_WallID && !wall.slab_step && wall.floor_core_offset_valid
                                          ? wall.floor_core_offset
                                          : 0;
                for (const auto &range : ranges)
                    contacts.push_back ({begin + dir * range.first, begin + dir * range.second, offset, ownerIndex});
            }
            Geometry::MultiPolygon2D polygons;
            polygons.Push (floor.poly);
            bool changed = false;
            for (auto contour = floor.poly.BeginContour (); contour != floor.poly.EndContour (); ++contour) {
                std::vector<Edge> edges;
                for (auto edge = contour->BeginEdge (); edge != contour->EndEdge (); ++edge) {
                    Point2D begin, end;
                    double angle = 0;
                    floor.poly.GetSector (edge, begin, end, angle);
                    edges.push_back ({{begin.x, begin.y}, {end.x, end.y}, std::fabs (angle) <= Tolerance});
                }
                for (const auto &patch : Extensions (edges, contacts)) {
                    GS::Array<Point2D> coords;
                    for (const auto &point : patch)
                        coords.Push ({point.x, point.y});
                    const auto strips = Geometry::Polygon2D::Create (coords, 0);
                    for (const auto &strip : strips)
                        polygons.Push (strip);
                    changed = changed || !strips.IsEmpty ();
                }
            }
            if (!changed) {
                result.Push (floor);
                continue;
            }
            polygons.Unify ();
            // Расширение не может удалить старое покрытие или разделить один owner на острова.
            Geometry::MultiPolygon2D original;
            original.Push (floor.poly);
            if (polygons.GetSize () != 1 || polygons[0].GetContourNum () != floor.poly.GetContourNum () ||
                Geometry::MultiPolygon2D::Substract (original, polygons).CalcArea () > Tolerance) {
                result.Push (floor);
                complete = false;
                continue;
            }
            OtdSlab extended = floor; // Все links/материальные правила/избранное/отметка сохраняются.
            extended.poly = polygons[0];
            result.Push (std::move (extended));
        }
        for (auto &slab : result)
            if (slab.isValid && (slab.type == Floor || slab.type == Ceil))
                CleanCollinearPolygonVertices (slab.poly);
        room.otdslab = std::move (result);
        return complete;
    }

    namespace {
        constexpr double DistanceTolerance = 1.e-6;
        constexpr double AreaTolerance = 1.e-6;

        API_Coord3D Subtract (const API_Coord3D &a, const API_Coord3D &b) { return {a.x - b.x, a.y - b.y, a.z - b.z}; }

        double Dot (const API_Coord3D &a, const API_Coord3D &b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

        API_Coord3D Cross (const API_Coord3D &a, const API_Coord3D &b) {
            return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
        }

        double Length (const API_Coord3D &a) { return std::hypot (std::hypot (a.x, a.y), a.z); }

        bool Finite (const API_Coord3D &a) { return std::isfinite (a.x) && std::isfinite (a.y) && std::isfinite (a.z); }

        bool Near (const API_Coord3D &a, const API_Coord3D &b) { return Length (Subtract (a, b)) <= DistanceTolerance; }

        bool ValidPiece (const RoomSurfacePiece &piece) {
            if (!Finite (piece.normal) || std::fabs (Length (piece.normal) - 1) > DistanceTolerance ||
                !std::isfinite (piece.planeOffset) || !std::isfinite (piece.area) || piece.area <= AreaTolerance ||
                piece.contours.IsEmpty ())
                return false;
            for (const auto &contour : piece.contours) {
                if (contour.vertices.GetSize () < 3)
                    return false;
                for (const auto &p : contour.vertices)
                    if (!Finite (p) || std::fabs (Dot (piece.normal, p) + piece.planeOffset) > DistanceTolerance)
                        return false;
            }
            return true;
        }

        // Удаляем только точки между соседями, не разворот контура.
        std::vector<API_Coord3D> Corners (const RoomSurfacePieceContour &contour) {
            std::vector<API_Coord3D> points;
            for (const auto &p : contour.vertices)
                if (points.empty () || !Near (points.back (), p))
                    points.push_back (p);
            if (points.size () > 1 && Near (points.front (), points.back ()))
                points.pop_back ();
            bool changed = true;
            while (changed && points.size () > 3) {
                changed = false;
                for (size_t i = 0; i < points.size (); ++i) {
                    const auto a = Subtract (points[i], points[(i + points.size () - 1) % points.size ()]);
                    const auto b = Subtract (points[(i + 1) % points.size ()], points[i]);
                    if (Dot (a, b) >= 0 && Length (Cross (a, b)) <= DistanceTolerance * (Length (a) + Length (b))) {
                        points.erase (points.begin () + i);
                        changed = true;
                        break;
                    }
                }
            }
            return points;
        }
    } // namespace

    bool BuildSurfaceWall (const RoomSurfacePiece &piece, short floorInd, OtdWall &wall) {
        if (!ValidPiece (piece) || piece.contours.GetSize () != 1 || piece.contours[0].hole)
            return false;
        const double horizontal = std::hypot (piece.normal.x, piece.normal.y);
        if (horizontal <= DistanceTolerance)
            return false;
        auto points = Corners (piece.contours[0]);
        if (points.size () != 4)
            return false;
        // Штатной стене нужен параллелограмм; экстремумы произвольной грани не заменяют её контур.
        size_t bottom = 4;
        for (size_t i = 0; i < 4; ++i) {
            if (std::fabs (points[i].z - points[(i + 1) % 4].z) <= DistanceTolerance &&
                points[(i + 2) % 4].z > points[i].z + min_dim && points[(i + 3) % 4].z > points[i].z + min_dim) {
                bottom = i;
                break;
            }
        }
        if (bottom == 4)
            return false;
        auto a = points[bottom], b = points[(bottom + 1) % 4];
        const auto topB = points[(bottom + 2) % 4], topA = points[(bottom + 3) % 4];
        const double height = topA.z - a.z;
        if (std::fabs (topB.z - topA.z) > DistanceTolerance || height < min_dim)
            return false;
        const double riseX = -piece.normal.z * piece.normal.x / (horizontal * horizontal);
        const double riseY = -piece.normal.z * piece.normal.y / (horizontal * horizontal);
        const API_Coord3D rise = {riseX * height, riseY * height, height};
        if (!Near (Subtract (topA, a), rise) || !Near (Subtract (topB, b), rise))
            return false;
        const double length = std::hypot (b.x - a.x, b.y - a.y);
        const double area = length * height / horizontal;
        // Площадь наследует погрешность точек вдоль периметра; допуск по точкам не меняем.
        const double areaError = std::max (AreaTolerance, 2 * (length + height / horizontal) * DistanceTolerance);
        if (!std::isfinite (area) || length < min_dim || std::fabs (area - piece.area) > areaError)
            return false;
        // Inside при flipped=false строит тело слева от линии: слева должна быть зона.
        // Значит, правый бок (dy,-dx) сонаправлен наружной нормали зоны.
        if ((b.y - a.y) * piece.normal.x - (b.x - a.x) * piece.normal.y < 0)
            std::swap (a, b);
        const double rightX = (b.y - a.y) / length, rightY = -(b.x - a.x) / length;
        if (std::fabs (rightX - piece.normal.x / horizontal) > DistanceTolerance ||
            std::fabs (rightY - piece.normal.y / horizontal) > DistanceTolerance)
            return false;
        OtdWall result;
        const Sector edge = {{a.x, a.y}, {b.x, b.y}};
        if (!OtdWall_Add_One (piece.baseGuid, edge, false, height, a.z, floorInd, 0, result))
            return false;
        result.length = length;
        result.base_guid = piece.baseGuid;
        result.base_type = piece.baseType;
        result.type = piece.baseType == API_ColumnID ? Column : Wall_Main;
        // В AC25 положительный котангенс slantAlpha наклоняет влево от линии.
        // Отрицательная проекция подъёма на правый бок сохраняет заданную контактную плоскость.
        result.slant_angle = std::atan2 (1.0, -(riseX * rightX + riseY * rightY));
        result.surface_geometry = true;
        result.rise_x = riseX;
        result.rise_y = riseY;
        wall = result;
        return true;
    }

    // Участок задаёт конечные границы только на своём roomEdge; внутренние разрывы не заполняем.
    GS::Array<OtdWall> ClampSurfaceWallToParts (const OtdWall &wall,
                                                const API_Coord &wallBeg,
                                                const API_Coord &wallEnd,
                                                const GS::Array<API_WallPart> &parts,
                                                const GS::Array<Sector> &roomEdges,
                                                double roomBottom,
                                                const GS::Array<OtdWall> &contacts) {
        GS::Array<OtdWall> result;
        const double dx = wallEnd.x - wallBeg.x, dy = wallEnd.y - wallBeg.y;
        const double length = std::hypot (dx, dy);
        const double ex = wall.endC.x - wall.begC.x, ey = wall.endC.y - wall.begC.y;
        const double span = std::hypot (ex, ey);
        if (!std::isfinite (length) || !std::isfinite (span) || !std::isfinite (roomBottom) ||
            !std::isfinite (wall.zBottom) || !std::isfinite (wall.height) || !std::isfinite (wall.rise_x) ||
            !std::isfinite (wall.rise_y) || length < min_dim || span < min_dim || wall.height < min_dim)
            return result;
        const double ux = dx / length, uy = dy / length, vx = ex / span, vy = ey / span;
        if (std::fabs (ux * vy - uy * vx) > DistanceTolerance)
            return result;
        const auto along = [&] (double x, double y) { return (x - wall.begC.x) * vx + (y - wall.begC.y) * vy; };
        const auto onLine = [&] (double x, double y) {
            return std::fabs ((x - wall.begC.x) * vy - (y - wall.begC.y) * vx) <= DistanceTolerance;
        };

        std::vector<std::pair<double, double>> intervals;
        bool hasParts = false;
        for (const auto &part : parts) {
            if (part.guid != wall.base_guid)
                continue;
            hasParts = true;
            if (part.roomEdge <= 0 || static_cast<USize> (part.roomEdge) > roomEdges.GetSize () ||
                !std::isfinite (part.tBeg) || !std::isfinite (part.tEnd))
                return {};
            const auto &edge = roomEdges[part.roomEdge - 1];
            // Ребро зоны переносим с отметки пола на низ наклонного контакта.
            const double dz = wall.zBottom - roomBottom;
            const double ax = edge.c1.x + wall.rise_x * dz, ay = edge.c1.y + wall.rise_y * dz;
            const double bx = edge.c2.x + wall.rise_x * dz, by = edge.c2.y + wall.rise_y * dz;
            if (!onLine (ax, ay) || !onLine (bx, by))
                continue;
            // Sentinel-границы обрабатываем так же, как OtdWall_Create_FromWall.
            const double tBeg =
                std::fabs (part.tBeg) <= DistanceTolerance || std::fabs (part.tBeg - length) <= DistanceTolerance
                    ? 0
                    : part.tBeg;
            const double tEnd =
                std::fabs (part.tEnd) <= DistanceTolerance || std::fabs (part.tEnd - length) <= DistanceTolerance
                    ? length
                    : part.tEnd;
            const double a = along (wallBeg.x + ux * tBeg, wallBeg.y + uy * tBeg);
            const double b = along (wallBeg.x + ux * tEnd, wallBeg.y + uy * tEnd);
            const double begin = std::max (std::min (along (ax, ay), along (bx, by)), std::min (a, b));
            const double end = std::min (std::max (along (ax, ay), along (bx, by)), std::max (a, b));
            if (end - begin > DistanceTolerance)
                intervals.push_back ({begin, end});
        }
        if (!hasParts) {
            result.Push (wall);
            return result;
        }
        std::sort (intervals.begin (), intervals.end ());
        std::vector<std::pair<double, double>> merged;
        for (const auto &interval : intervals) {
            if (!merged.empty () && interval.first <= merged.back ().second + DistanceTolerance)
                merged.back ().second = std::max (merged.back ().second, interval.second);
            else
                merged.push_back (interval);
        }
        for (const auto &interval : merged) {
            double firstContact = 0, lastContact = span;
            for (const auto &other : contacts) {
                if (!other.surface_geometry || other.base_guid != wall.base_guid || other.base_type != API_WallID ||
                    other.slab_step ||
                    std::min (wall.zBottom + wall.height, other.zBottom + other.height) -
                            std::max (wall.zBottom, other.zBottom) <=
                        DistanceTolerance ||
                    std::fabs (wall.rise_x - other.rise_x) > DistanceTolerance ||
                    std::fabs (wall.rise_y - other.rise_y) > DistanceTolerance)
                    continue;
                const double dz = wall.zBottom - other.zBottom;
                const double ax = other.begC.x + other.rise_x * dz, ay = other.begC.y + other.rise_y * dz;
                const double bx = other.endC.x + other.rise_x * dz, by = other.endC.y + other.rise_y * dz;
                if (!onLine (ax, ay) || !onLine (bx, by) || (bx - ax) * vx + (by - ay) * vy <= DistanceTolerance)
                    continue;
                if (std::min (std::max (along (ax, ay), along (bx, by)), interval.second) -
                        std::max (std::min (along (ax, ay), along (bx, by)), interval.first) <=
                    DistanceTolerance)
                    continue;
                firstContact = std::min (firstContact, along (ax, ay));
                lastContact = std::max (lastContact, along (bx, by));
            }
            if (std::min (span, interval.second) - std::max (0.0, interval.first) <= DistanceTolerance)
                continue;
            const double begin = firstContact >= -DistanceTolerance ? interval.first : std::max (0.0, interval.first);
            const double end =
                lastContact <= span + DistanceTolerance ? interval.second : std::min (span, interval.second);
            if (end - begin < min_dim)
                return {};
            OtdWall trimmed = wall;
            trimmed.begC = {wall.begC.x + vx * begin, wall.begC.y + vy * begin};
            trimmed.endC = {wall.begC.x + vx * end, wall.begC.y + vy * end};
            trimmed.length = end - begin;
            trimmed.openings.Clear ();
            for (const auto &op : wall.openings) {
                const double a = op.objLoc - op.width / 2, b = op.objLoc + op.width / 2;
                // Краевой вырез не закрываем при удлинении оболочки.
                const double left = std::max (a <= DistanceTolerance ? std::min (a, begin) : a, begin);
                const double right = std::min (b >= span - DistanceTolerance ? std::max (b, end) : b, end);
                if (right - left <= DistanceTolerance)
                    continue;
                if (right - left < min_dim)
                    return {};
                OtdOpening clipped = op;
                clipped.width = right - left;
                clipped.objLoc = (left + right) / 2 - begin;
                trimmed.openings.Push (std::move (clipped));
            }
            result.Push (std::move (trimmed));
        }
        return result;
    }

    bool BuildSurfaceWalls (const RoomSurfacePiece &piece, short floorInd, GS::Array<OtdWall> &walls) {
        OtdWall single;
        if (BuildSurfaceWall (piece, floorInd, single)) {
            walls.Push (std::move (single));
            return true;
        }
        if (!ValidPiece (piece) || piece.contours[0].hole)
            return false;
        const double horizontal = std::hypot (piece.normal.x, piece.normal.y);
        if (horizontal <= DistanceTolerance)
            return false;
        const API_Coord3D along = {-piece.normal.y / horizontal, piece.normal.x / horizontal, 0};
        const double riseX = -piece.normal.z * piece.normal.x / (horizontal * horizontal);
        const double riseY = -piece.normal.z * piece.normal.y / (horizontal * horizontal);
        const auto origin = piece.contours[0].vertices[0];
        std::vector<std::vector<API_Coord>> contours;
        std::vector<double> levels;
        double perimeter = 0;
        double leftBound = 0, rightBound = 0;
        for (const auto &contour : piece.contours) {
            if (&contour != &piece.contours[0] && !contour.hole)
                return false;
            std::vector<API_Coord> points;
            for (const auto &p : Corners (contour)) {
                const double x = Dot (Subtract (p, origin), along);
                points.push_back ({x, p.z});
                levels.push_back (p.z);
                leftBound = std::min (leftBound, x);
                rightBound = std::max (rightBound, x);
            }
            for (size_t i = 0; i < points.size (); ++i) {
                const auto &a = points[i], &b = points[(i + 1) % points.size ()];
                // Прямоугольный проём не заменяет диагональный край контакта.
                if (std::fabs (a.x - b.x) > DistanceTolerance && std::fabs (a.y - b.y) > DistanceTolerance)
                    return false;
                perimeter += std::hypot (a.x - b.x, (a.y - b.y) / horizontal);
            }
            contours.push_back (std::move (points));
        }
        std::sort (levels.begin (), levels.end ());
        levels.erase (std::unique (levels.begin (),
                                   levels.end (),
                                   [] (double a, double b) { return std::fabs (a - b) <= DistanceTolerance; }),
                      levels.end ());
        if (levels.size () < 2)
            return false;
        const auto point = [&] (double x, double z) -> API_Coord3D {
            return {
                origin.x + along.x * x + riseX * (z - origin.z), origin.y + along.y * x + riseY * (z - origin.z), z};
        };
        RoomSurfacePiece envelope = piece;
        envelope.contours.Clear ();
        RoomSurfacePieceContour outline;
        outline.vertices = {point (leftBound, levels.front ()),
                            point (rightBound, levels.front ()),
                            point (rightBound, levels.back ()),
                            point (leftBound, levels.back ())};
        envelope.contours.Push (std::move (outline));
        envelope.area = (rightBound - leftBound) * (levels.back () - levels.front ()) / horizontal;
        if (!BuildSurfaceWall (envelope, floorInd, single))
            return false;

        struct Cutout {
            double left, right, bottom, top;
        };

        std::vector<Cutout> cutouts;
        double area = 0;
        for (size_t band = 1; band < levels.size (); ++band) {
            const double lower = levels[band - 1], upper = levels[band];
            const double middle = (lower + upper) / 2;
            std::vector<double> crossings;
            for (const auto &contour : contours) {
                for (size_t i = 0; i < contour.size (); ++i) {
                    const auto &a = contour[i], &b = contour[(i + 1) % contour.size ()];
                    if ((a.y > middle) != (b.y > middle))
                        crossings.push_back (a.x + (b.x - a.x) * (middle - a.y) / (b.y - a.y));
                }
            }
            std::sort (crossings.begin (), crossings.end ());
            if (crossings.size () % 2 != 0)
                return false;
            // Разлагаем только пустоту внутри оболочки; материал остаётся одной стеной.
            double previous = leftBound;
            for (size_t i = 0; i <= crossings.size (); i += 2) {
                const double next = i < crossings.size () ? crossings[i] : rightBound;
                if (next - previous > DistanceTolerance) {
                    bool merged = false;
                    for (auto &cutout : cutouts) {
                        if (std::fabs (cutout.top - lower) <= DistanceTolerance &&
                            std::fabs (cutout.left - previous) <= DistanceTolerance &&
                            std::fabs (cutout.right - next) <= DistanceTolerance) {
                            cutout.top = upper;
                            merged = true;
                            break;
                        }
                    }
                    if (!merged)
                        cutouts.push_back ({previous, next, lower, upper});
                }
                if (i < crossings.size ()) {
                    area += (crossings[i + 1] - crossings[i]) * (upper - lower) / horizontal;
                    previous = crossings[i + 1];
                }
            }
        }
        if (std::fabs (area - piece.area) > std::max (AreaTolerance, 2 * perimeter * DistanceTolerance))
            return false;
        const double wallLength = rightBound - leftBound;
        const bool reversed = (single.endC.x - single.begC.x) * along.x + (single.endC.y - single.begC.y) * along.y < 0;
        for (const auto &cutout : cutouts) {
            // Не допускам молчаливого заполнения выреза из-за порога штатного writer.
            if (cutout.right - cutout.left <= min_dim || cutout.top - cutout.bottom <= min_dim)
                return false;
            OtdOpening opening;
            opening.width = cutout.right - cutout.left;
            opening.height = cutout.top - cutout.bottom;
            opening.objLoc = (cutout.left + cutout.right) / 2 - leftBound;
            opening.lower = cutout.bottom - single.zBottom;
            Opening_Add_One (opening, reversed, single.zBottom, 0, wallLength, wallLength, single);
            single.openings.GetLast ().lower = opening.lower;
            single.openings.GetLast ().surface_geometry = true;
        }
        if (std::fabs (OtdWall_GetArea (single) - piece.area) >
            std::max (AreaTolerance, 2 * perimeter * DistanceTolerance))
            return false;
        walls.Push (std::move (single));
        return true;
    }

    bool LinkSurfaceOpenings (OtdWall &wall, const GS::Array<OtdWall> &openingWalls, bool analyticOverlap) {
        if (!wall.surface_geometry || wall.openings.IsEmpty ())
            return true;
        const double length = std::hypot (wall.endC.x - wall.begC.x, wall.endC.y - wall.begC.y);
        if (length <= min_dim)
            return false;
        const double ux = (wall.endC.x - wall.begC.x) / length;
        const double uy = (wall.endC.y - wall.begC.y) / length;

        struct SourceOpening {
            double left, right, bottom, top;
            OtdOpening opening;
        };

        std::vector<SourceOpening> sources;
        for (const auto &sourceWall : openingWalls) {
            if (sourceWall.base_guid != wall.base_guid)
                continue;
            const double dx = sourceWall.endC.x - sourceWall.begC.x;
            const double dy = sourceWall.endC.y - sourceWall.begC.y;
            const double sourceLength = std::hypot (dx, dy);
            if (sourceLength <= min_dim || std::fabs (dx * uy - dy * ux) > DistanceTolerance * sourceLength)
                continue;
            for (const auto &opening : sourceWall.openings) {
                if (opening.base_guid == APINULLGuid)
                    continue;
                const double x = sourceWall.begC.x + dx * opening.objLoc / sourceLength;
                const double y = sourceWall.begC.y + dy * opening.objLoc / sourceLength;
                const double center = (x - wall.begC.x) * ux + (y - wall.begC.y) * uy;
                sources.push_back ({center - opening.width / 2,
                                    center + opening.width / 2,
                                    opening.zBottom,
                                    opening.zBottom + opening.height,
                                    opening});
            }
        }
        GS::Array<OtdOpening> linked;
        for (const auto &cutout : wall.openings) {
            const double left = cutout.objLoc - cutout.width / 2, right = cutout.objLoc + cutout.width / 2;
            const double bottom = cutout.zBottom, top = cutout.zBottom + cutout.height;
            std::vector<double> xs = {left, right}, zs = {bottom, top};
            for (const auto &source : sources) {
                if (std::min (right, source.right) - std::max (left, source.left) <= DistanceTolerance ||
                    std::min (top, source.top) - std::max (bottom, source.bottom) <= DistanceTolerance)
                    continue;
                xs.push_back (std::max (left, source.left));
                xs.push_back (std::min (right, source.right));
                zs.push_back (std::max (bottom, source.bottom));
                zs.push_back (std::min (top, source.top));
            }
            const auto normalize = [] (std::vector<double> &values) {
                std::sort (values.begin (), values.end ());
                values.erase (std::unique (values.begin (),
                                           values.end (),
                                           [] (double a, double b) { return std::fabs (a - b) <= DistanceTolerance; }),
                              values.end ());
            };
            normalize (xs);
            normalize (zs);
            for (size_t z = 1; z < zs.size (); ++z) {
                for (size_t x = 1; x < xs.size (); ++x) {
                    if (xs[x] - xs[x - 1] <= min_dim || zs[z] - zs[z - 1] <= min_dim)
                        return false;
                    const double centerX = (xs[x] + xs[x - 1]) / 2, centerZ = (zs[z] + zs[z - 1]) / 2;
                    const SourceOpening *owner = nullptr;
                    GS::Array<API_Guid> sourceGuids;
                    for (const auto &source : sources) {
                        if (centerX < source.left - DistanceTolerance || centerX > source.right + DistanceTolerance ||
                            centerZ < source.bottom - DistanceTolerance || centerZ > source.top + DistanceTolerance)
                            continue;
                        if (owner != nullptr && owner->opening.base_guid != source.opening.base_guid &&
                            !analyticOverlap)
                            return false;
                        if (!sourceGuids.Contains (source.opening.base_guid))
                            sourceGuids.Push (source.opening.base_guid);
                        owner = &source;
                    }
                    OtdOpening opening = cutout;
                    opening.objLoc = centerX;
                    opening.width = xs[x] - xs[x - 1];
                    opening.zBottom = zs[z - 1];
                    opening.height = zs[z] - zs[z - 1];
                    opening.lower = opening.zBottom - wall.zBottom;
                    opening.source_opening_guids = sourceGuids;
                    opening.base_guid = sourceGuids.GetSize () == 1 ? sourceGuids[0] : APINULLGuid;
                    // Только привязка к реальному проёму; откосы создаются отдельным legacy-путём.
                    opening.has_reveal = false;
                    opening.surface_geometry = true;
                    linked.Push (std::move (opening));
                }
            }
        }
        double originalArea = 0, linkedArea = 0;
        for (const auto &opening : wall.openings)
            originalArea += opening.width * opening.height;
        for (const auto &opening : linked)
            linkedArea += opening.width * opening.height;
        if (std::fabs (originalArea - linkedArea) >
            std::max (AreaTolerance, 2 * (length + wall.height) * DistanceTolerance))
            return false;
        wall.openings = std::move (linked);
        return true;
    }

    bool GroupSurfaceWallPieces (const GS::Array<RoomSurfacePiece> &pieces,
                                 short floorInd,
                                 GS::Array<RoomSurfacePiece> &grouped) {
        using Polygons = Geometry::MultiPolygon2D;
        (void)floorInd;
        const auto sameFace = [] (const RoomSurfaceSourceFace &a, const RoomSurfaceSourceFace &b) {
            return a.baseGuid == b.baseGuid && a.baseType == b.baseType && a.geometryGuid == b.geometryGuid &&
                   a.bodyIndex == b.bodyIndex && a.polygonIndex == b.polygonIndex &&
                   a.modelMaterialIndex == b.modelMaterialIndex && a.origin == b.origin &&
                   a.analyticFaceId == b.analyticFaceId && a.preparedWallIndex == b.preparedWallIndex;
        };
        // Сравниваем полный набор граней взаимно однозначно: одинакового GUID основания недостаточно.
        const auto sameSource = [&] (const RoomSurfacePiece &a, const RoomSurfacePiece &b) {
            if (a.baseGuid != b.baseGuid || a.baseType != b.baseType || a.zoneBodyIndex != b.zoneBodyIndex ||
                a.sources.GetSize () != b.sources.GetSize ())
                return false;
            std::vector<bool> matched (b.sources.GetSize (), false);
            for (const auto &source : a.sources) {
                bool found = false;
                for (UIndex i = 0; i < b.sources.GetSize (); ++i) {
                    if (!matched[i] && sameFace (source, b.sources[i])) {
                        matched[i] = true;
                        found = true;
                        break;
                    }
                }
                if (!found)
                    return false;
            }
            return true;
        };
        // Объединяем до проверки представимости: диагональный кусок не должен исчезать
        // из общего контура и оставлять случайную прямоугольную часть той же поверхности.
        std::vector<const RoomSurfacePiece *> candidates;
        GS::Array<RoomSurfacePiece> results;
        for (const auto &piece : pieces) {
            if (piece.status == RoomSurfacePieceStatus::Matched && piece.baseGuid != APINULLGuid &&
                piece.baseType == API_WallID && !piece.sources.IsEmpty () && ValidPiece (piece) &&
                std::hypot (piece.normal.x, piece.normal.y) > DistanceTolerance)
                candidates.push_back (&piece);
            else
                results.Push (piece);
        }
        // Опорная плоскость не зависит от порядка входа; кластер не растёт цепочкой близости.
        std::sort (candidates.begin (), candidates.end (), [] (const auto *a, const auto *b) {
            if (a->normal.x != b->normal.x)
                return a->normal.x < b->normal.x;
            if (a->normal.y != b->normal.y)
                return a->normal.y < b->normal.y;
            if (a->normal.z != b->normal.z)
                return a->normal.z < b->normal.z;
            if (a->planeOffset != b->planeOffset)
                return a->planeOffset < b->planeOffset;
            return a->area < b->area;
        });
        std::vector<bool> used (candidates.size (), false);
        for (size_t first = 0; first < candidates.size (); ++first) {
            if (used[first])
                continue;
            const auto &reference = *candidates[first];
            const double normalLength = Length (reference.normal);
            const API_Coord3D normal = {reference.normal.x / normalLength,
                                        reference.normal.y / normalLength,
                                        reference.normal.z / normalLength};
            const double offset = reference.planeOffset / normalLength;
            const double horizontal = std::hypot (normal.x, normal.y);
            const API_Coord3D along = {-normal.y / horizontal, normal.x / horizontal, 0};
            // Начало на отметке ноль вычисляем из плоскости, а не из первой вершины контура.
            // Вторая координата — абсолютная Z; площадь в этих координатах делится на horizontal.
            const API_Coord3D origin = {
                -offset * normal.x / (horizontal * horizontal), -offset * normal.y / (horizontal * horizontal), 0};
            const API_Coord3D rise = {
                -normal.z * normal.x / (horizontal * horizontal), -normal.z * normal.y / (horizontal * horizontal), 1};
            const auto restore = [&] (const Point2D &p) -> API_Coord3D {
                return {origin.x + along.x * p.x + rise.x * p.y, origin.y + along.y * p.x + rise.y * p.y, p.y};
            };
            const auto project = [&] (const API_Coord3D &p) -> Point2D {
                return {Dot (Subtract (p, origin), along), p.z};
            };
            std::vector<size_t> members;
            for (size_t index = first; index < candidates.size (); ++index) {
                if (used[index] || !sameSource (reference, *candidates[index]) ||
                    Length (Subtract (normal, candidates[index]->normal)) > DistanceTolerance)
                    continue;
                bool coplanar = true;
                for (const auto &contour : candidates[index]->contours)
                    for (const auto &point : contour.vertices)
                        coplanar = coplanar && std::fabs (Dot (normal, point) + offset) <= DistanceTolerance &&
                                   Near (restore (project (point)), point);
                if (coplanar)
                    members.push_back (index);
            }
            if (members.empty ())
                return false;
            if (members.size () == 1) {
                used[first] = true;
                results.Push (reference);
                continue;
            }
            Polygons input;
            double perimeter = 0;
            for (size_t index : members) {
                GS::Array<Point2D> coords;
                GS::Array<PolyArcRec> arcs;
                GS::Array<UIndex> ends;
                for (const auto &contour : candidates[index]->contours) {
                    const auto corners = Corners (contour);
                    for (size_t vertex = 0; vertex < corners.size (); ++vertex) {
                        coords.Push (project (corners[vertex]));
                        perimeter += Length (Subtract (corners[vertex], corners[(vertex + 1) % corners.size ()]));
                    }
                    ends.Push (coords.GetSize () - 1);
                }
                const auto polygons = Geometry::Polygon2D::Create (
                    coords, arcs, ends, Geometry::PolyCreateFlags_IdxRefInContourEndFromZero);
                if (polygons.IsEmpty () || std::fabs (polygons.CalcArea () / horizontal - candidates[index]->area) >
                                               std::max (AreaTolerance, 2 * perimeter * DistanceTolerance))
                    return false;
                for (const auto &polygon : polygons)
                    input.Push (polygon);
            }
            // Union убирает внутренние швы и дубли перекрытия до общего разложения по уровням.
            // Чётность применяется внутри одного контура, но не к перекрывающимся исходным кускам.
            Polygons united = input;
            united.Unify ();
            const double areaError = std::max (AreaTolerance, 2 * perimeter * DistanceTolerance);
            // Обе разности проверяют реальное покрытие, не сумму площадей пересекающихся частей.
            if (united.IsEmpty () || Polygons::Substract (input, united).CalcArea () / horizontal > areaError ||
                Polygons::Substract (united, input).CalcArea () / horizontal > areaError)
                return false;
            GS::Array<RoomSurfacePiece> merged;
            for (const auto &polygon : united) {
                RoomSurfacePiece piece = reference;
                piece.normal = normal;
                piece.planeOffset = offset;
                piece.area = polygon.CalcArea () / horizontal;
                piece.contours.Clear ();
                GS::Array<Point2D> coords;
                GS::Array<PolyArcRec> arcs;
                GS::Array<UIndex> ends;
                GS::Array<Geometry::Polygon2D::ConstContourIterator> reversed;
                polygon.CopyPolygonData (
                    coords, arcs, ends, reversed, Geometry::PolyCreateFlags_IdxRefInContourEndFromZero);
                UIndex start = 0;
                for (UIndex contourIndex = 0; contourIndex < ends.GetSize (); ++contourIndex) {
                    RoomSurfacePieceContour contour;
                    contour.hole = contourIndex != 0;
                    for (UIndex vertex = start; vertex <= ends[contourIndex]; ++vertex)
                        contour.vertices.Push (restore (coords[vertex]));
                    piece.contours.Push (std::move (contour));
                    start = ends[contourIndex] + 1;
                }
                merged.Push (std::move (piece));
            }
            results.Append (merged);
            for (size_t index : members)
                used[index] = true;
        }
        // До этой точки grouped не меняется: ошибочное объединение не оставляет частичного результата.
        grouped.Append (results);
        return true;
    }

    bool DeduplicateAcceptedSlabContacts (OtdRoom &room) {
        using Polygons = Geometry::MultiPolygon2D;
        // Условия включения совпадают с Draw_Elements; Floor/Ceil-боковины идут стеновым writer.
        const auto enabled = [&] (const OtdWall &w) {
            if (!room.isValid || !room.create_all_elements || !w.isValid ||
                !(room.create_wall_elements || room.create_column_elements || room.create_reveal_elements))
                return false;
            if ((w.type == Wall_Main || w.type == Wall_Up || w.type == Wall_Down) && !room.create_wall_elements)
                return false;
            if (w.type == Column && !room.create_column_elements)
                return false;
            if ((w.type == Reveal_Main || w.type == Reveal_Up || w.type == Reveal_Down) && !room.create_reveal_elements)
                return false;
            return w.surface_geometry && w.draw_type == API_WallID && (!w.slab_step || w.slab_step_writer_eligible);
        };
        GS::Array<OtdWall> output;
        bool complete = true;
        for (const auto &wall : room.otdwall) {
            if (wall.base_type != API_SlabID || wall.slab_step || !enabled (wall)) {
                output.Push (wall);
                continue;
            }
            const double length = std::hypot (wall.endC.x - wall.begC.x, wall.endC.y - wall.begC.y);
            if (!std::isfinite (length) || length <= min_dim || !std::isfinite (wall.height) ||
                wall.height <= min_dim || !std::isfinite (wall.zBottom) || !std::isfinite (wall.rise_x) ||
                !std::isfinite (wall.rise_y)) {
                output.Push (wall);
                complete = false;
                continue;
            }
            const double ux = (wall.endC.x - wall.begC.x) / length, uy = (wall.endC.y - wall.begC.y) / length;
            API_Coord3D normal = {uy, -ux, -(uy * wall.rise_x - ux * wall.rise_y)};
            const double norm = Length (normal);
            normal = {normal.x / norm, normal.y / norm, normal.z / norm};
            const auto point = [&] (double x, double z) -> API_Coord3D {
                return {wall.begC.x + ux * x + wall.rise_x * (z - wall.zBottom),
                        wall.begC.y + uy * x + wall.rise_y * (z - wall.zBottom),
                        z};
            };
            const auto rectangle = [] (double left, double right, double bottom, double top) {
                GS::Array<Point2D> coords = {{left, bottom}, {right, bottom}, {right, top}, {left, top}};
                GS::Array<PolyArcRec> arcs;
                GS::Array<UIndex> ends = {3};
                return Geometry::Polygon2D::Create (
                    coords, arcs, ends, Geometry::PolyCreateFlags_IdxRefInContourEndFromZero);
            };
            const auto coverage = [&] (const OtdWall &w, Polygons &polys) {
                const double l = std::hypot (w.endC.x - w.begC.x, w.endC.y - w.begC.y);
                if (!std::isfinite (l) || l <= min_dim || !std::isfinite (w.height) || w.height <= min_dim ||
                    !std::isfinite (w.zBottom) || !std::isfinite (w.rise_x) || !std::isfinite (w.rise_y))
                    return false;
                const double dx = (w.endC.x - w.begC.x) / l, dy = (w.endC.y - w.begC.y) / l;
                if (std::fabs (dx * uy - dy * ux) > DistanceTolerance ||
                    std::fabs (w.rise_x - wall.rise_x) > DistanceTolerance ||
                    std::fabs (w.rise_y - wall.rise_y) > DistanceTolerance)
                    return false;
                const double x = (w.begC.x - wall.begC.x) * ux + (w.begC.y - wall.begC.y) * uy;
                const auto restored = point (x, w.zBottom);
                if (!Near (restored, {w.begC.x, w.begC.y, w.zBottom}))
                    return false;
                const double direction = dx * ux + dy * uy;
                polys = rectangle (
                    std::min (x, x + direction * l), std::max (x, x + direction * l), w.zBottom, w.zBottom + w.height);
                for (const auto &opening : w.openings) {
                    if (!std::isfinite (opening.objLoc) || !std::isfinite (opening.width) ||
                        !std::isfinite (opening.zBottom) || !std::isfinite (opening.height) ||
                        opening.width <= min_dim || opening.height <= min_dim)
                        return false;
                    const double center = x + direction * opening.objLoc;
                    polys = Polygons::Substract (polys,
                                                 rectangle (center - opening.width / 2,
                                                            center + opening.width / 2,
                                                            opening.zBottom,
                                                            opening.zBottom + opening.height));
                }
                return true;
            };
            Polygons original;
            if (!coverage (wall, original)) {
                output.Push (wall);
                complete = false;
                continue;
            }
            Polygons remainder = original;
            for (const auto &step : room.otdwall) {
                if (!step.slab_step || step.base_guid != wall.base_guid || step.base_guid == APINULLGuid ||
                    step.base_type != API_SlabID || !enabled (step))
                    continue;
                Polygons accepted;
                if (coverage (step, accepted))
                    remainder = Polygons::Substract (remainder, accepted);
            }
            if (Polygons::Substract (original, remainder).IsEmpty ()) {
                output.Push (wall);
                continue;
            }
            GS::Array<OtdWall> fragments;
            bool representable = true;
            // Реальная ownership проёма не теряется при смене разложения: такой случай оставляем явно неоднозначным.
            for (const auto &opening : wall.openings)
                if (opening.base_guid != APINULLGuid)
                    representable = false;
            Polygons rebuilt;
            for (const auto &polygon : remainder) {
                RoomSurfacePiece piece;
                piece.baseGuid = wall.base_guid;
                piece.baseType = wall.base_type;
                piece.normal = normal;
                piece.planeOffset = -Dot (normal, point (0, wall.zBottom));
                piece.area = polygon.CalcArea () * norm;
                GS::Array<Point2D> coords;
                GS::Array<PolyArcRec> arcs;
                GS::Array<UIndex> ends;
                GS::Array<Geometry::Polygon2D::ConstContourIterator> reversed;
                polygon.CopyPolygonData (
                    coords, arcs, ends, reversed, Geometry::PolyCreateFlags_IdxRefInContourEndFromZero);
                if (!arcs.IsEmpty ()) {
                    representable = false;
                    break;
                }
                UIndex first = 0;
                for (UIndex i = 0; i < ends.GetSize (); ++i) {
                    RoomSurfacePieceContour contour;
                    contour.hole = i != 0;
                    for (UIndex j = first; j <= ends[i]; ++j)
                        contour.vertices.Push (point (coords[j].x, coords[j].y));
                    piece.contours.Push (contour);
                    first = ends[i] + 1;
                }
                GS::Array<OtdWall> adapted;
                if (!BuildSurfaceWalls (piece, wall.floorInd, adapted)) {
                    representable = false;
                    break;
                }
                for (const auto &geometry : adapted) {
                    // Копируем принятые правила, меняя только конечное покрытие; правила повторно не вызываются.
                    OtdWall fragment = wall;
                    fragment.begC = geometry.begC;
                    fragment.endC = geometry.endC;
                    fragment.zBottom = geometry.zBottom;
                    fragment.height = geometry.height;
                    fragment.length = geometry.length;
                    fragment.openings = geometry.openings;
                    Polygons checked;
                    if (!coverage (fragment, checked)) {
                        representable = false;
                        break;
                    }
                    for (const auto &part : checked)
                        rebuilt.Push (part);
                    fragments.Push (fragment);
                }
            }
            // Проверяем обе направленные разности конечных контуров, а не только сумму площадей.
            representable = representable &&
                            Polygons::Substract (remainder, rebuilt).CalcArea () * norm <= AreaTolerance &&
                            Polygons::Substract (rebuilt, remainder).CalcArea () * norm <= AreaTolerance;
            if (representable)
                output.Append (fragments);
            else {
                output.Push (wall);
                complete = false;
            }
        }
        room.otdwall = std::move (output);
        return complete;
    }

    bool BuildSurfaceSlabs (const RoomSurfacePiece &piece, short floorInd, GS::Array<OtdSlab> &slabs) {
        if (!ValidPiece (piece) || std::hypot (piece.normal.x, piece.normal.y) > DistanceTolerance ||
            std::fabs (std::fabs (piece.normal.z) - 1) > DistanceTolerance || piece.contours[0].hole)
            return false;
        const double elevation = piece.contours[0].vertices[0].z;
        GS::Array<Point2D> coords;
        GS::Array<UIndex> ends;
        GS::Array<PolyArcRec> arcs;
        double expectedArea = 0;
        bool firstContour = true;
        for (const auto &contour : piece.contours) {
            // Перегрузка Create с концами контуров принимает внешний контур и отверстия.
            if (contour.hole == firstContour)
                return false;
            firstContour = false;
            // Сохраняем все XY-вершины; регуляризацию контура выполняет Polygon2D.
            std::vector<API_Coord3D> points;
            for (const auto &point : contour.vertices)
                points.push_back (point);
            if (points.size () > 1 && points.front ().x == points.back ().x && points.front ().y == points.back ().y &&
                points.front ().z == points.back ().z)
                points.pop_back ();
            if (points.size () < 3)
                return false;
            double twiceArea = 0;
            const auto origin = points.front ();
            for (size_t i = 0; i < points.size (); ++i) {
                const auto &p = points[i], &next = points[(i + 1) % points.size ()];
                if (std::fabs (p.z - elevation) > DistanceTolerance)
                    return false;
                twiceArea += (p.x - origin.x) * (next.y - origin.y) - (next.x - origin.x) * (p.y - origin.y);
                coords.Push ({p.x, p.y});
            }
            if (!std::isfinite (twiceArea) || std::fabs (twiceArea) <= 2 * AreaTolerance)
                return false;
            expectedArea += (contour.hole ? -1 : 1) * std::fabs (twiceArea) / 2;
            ends.Push (coords.GetSize () - 1);
        }
        if (expectedArea <= AreaTolerance || std::fabs (expectedArea - piece.area) > AreaTolerance)
            return false;
        const Geometry::MultiPolygon2D polygons =
            Geometry::Polygon2D::Create (coords, arcs, ends, Geometry::PolyCreateFlags_IdxRefInContourEndFromZero);
        if (polygons.IsEmpty () || std::fabs (polygons.CalcArea () - expectedArea) > AreaTolerance)
            return false;
        GS::Array<OtdSlab> results;
        for (const auto &polygon : polygons) {
            OtdSlab slab;
            slab.poly = polygon;
            slab.zBottom = elevation;
            slab.floorInd = floorInd;
            slab.base_guid = piece.baseGuid;
            slab.base_type = piece.baseType;
            slab.type = piece.normal.z < 0 ? Floor : Ceil;
            slab.surface_geometry = true;
            results.Push (slab);
        }
        slabs.Append (results);
        return true;
    }

    bool BuildProjectedSlabFinishes (const RoomSurfaceSnapshot &zone,
                                     const GS::Array<RoomSurfaceSource> &sources,
                                     const GS::Array<API_Guid> &roles,
                                     TypeOtd type,
                                     short floorInd,
                                     GS::Array<OtdSlab> &slabs,
                                     double &rejectedArea,
                                     const OtdSlab *nominal,
                                     bool onlyOnSlab,
                                     GS::Array<OtdWall> *walls,
                                     const GS::Array<OtdSlab> *prepared) {
        if (!zone.IsComplete () || (type != Floor && type != Ceil))
            return false;
        const double normal = type == Floor ? -1 : 1;
        const auto horizontal = [] (const RoomSurface &face) {
            if (face.contours.IsEmpty () || face.contours[0].vertices.IsEmpty () ||
                std::hypot (face.normalWorld.x, face.normalWorld.y) > DistanceTolerance ||
                std::fabs (std::fabs (face.normalWorld.z) - 1) > DistanceTolerance)
                return false;
            const double z = face.contours[0].vertices[0].world.z;
            for (const auto &contour : face.contours)
                for (const auto &point : contour.vertices)
                    if (!Finite (point.world) || std::fabs (point.world.z - z) > DistanceTolerance)
                        return false;
            return true;
        };
        // Объединение проекций убирает дубли разных тел/ступеней зоны, сохраняя отверстия.
        Geometry::MultiPolygon2D footprint;
        for (const auto &body : zone.bodies)
            for (const auto &face : body.surfaces) {
                if (!horizontal (face) || face.normalWorld.z * normal < 0)
                    continue;
                RoomSurfacePiece piece;
                piece.status = RoomSurfacePieceStatus::Matched;
                piece.baseGuid = zone.guid;
                piece.baseType = API_ZoneID;
                piece.normal = {0, 0, normal};
                for (const auto &contour : face.contours) {
                    if (contour.vertices.GetSize () < 3)
                        return false;
                    const auto origin = contour.vertices[0].world;
                    RoomSurfacePieceContour projected;
                    projected.hole = contour.hole;
                    double twiceArea = 0;
                    for (UIndex i = 0; i < contour.vertices.GetSize (); ++i) {
                        const auto &a = contour.vertices[i].world;
                        const auto &b = contour.vertices[(i + 1) % contour.vertices.GetSize ()].world;
                        projected.vertices.Push ({a.x, a.y, 0});
                        twiceArea += (a.x - origin.x) * (b.y - origin.y) - (b.x - origin.x) * (a.y - origin.y);
                    }
                    piece.area += (contour.hole ? -1 : 1) * std::fabs (twiceArea) / 2;
                    piece.contours.Push (projected);
                }
                GS::Array<OtdSlab> projected;
                if (!BuildSurfaceSlabs (piece, floorInd, projected))
                    return false;
                for (const auto &slab : projected)
                    footprint.Push (slab.poly);
            }
        if (footprint.IsEmpty ())
            return false;
        footprint.Unify ();

        struct Candidate {
            OtdSlab slab;
            double bottom, top;
        };

        std::vector<Candidate> candidates;
        GS::Array<API_Guid> visited;
        for (const auto &guid : roles) {
            bool duplicate = false;
            for (const auto &seen : visited)
                duplicate = duplicate || seen == guid;
            if (duplicate)
                continue;
            visited.Push (guid);
            bool found = false;
            for (const auto &source : sources) {
                if (source.baseGuid != guid || source.baseType != API_SlabID)
                    continue;
                found = true;
                if (!source.snapshot.IsComplete ())
                    return false;
                if (source.origin == RoomSurfaceOrigin::PreparedAnalytic && prepared != nullptr) {
                    bool hasBase = false;
                    for (const auto &base : *prepared) {
                        if (base.base_guid != guid)
                            continue;
                        hasBase = true;
                        OtdSlab slab = base;
                        slab.type = type;
                        slab.floorInd = floorInd;
                        slab.surface_geometry = true;
                        const double bottom = base.zBottom, top = bottom + base.height;
                        slab.zBottom = type == Floor ? top : bottom;
                        candidates.push_back ({slab, bottom, top});
                    }
                    if (!hasBase)
                        return false;
                    continue;
                }
                for (const auto &body : source.snapshot.bodies) {
                    double bottom = std::numeric_limits<double>::infinity ();
                    double top = -bottom;
                    for (const auto &face : body.surfaces)
                        for (const auto &contour : face.contours)
                            for (const auto &vertex : contour.vertices) {
                                if (!Finite (vertex.world))
                                    return false;
                                bottom = std::min (bottom, vertex.world.z);
                                top = std::max (top, vertex.world.z);
                            }
                    for (const auto &face : body.surfaces) {
                        if (!horizontal (face) || face.normalWorld.z * normal > 0)
                            continue;
                        RoomSurfacePiece piece;
                        piece.status = RoomSurfacePieceStatus::Matched;
                        piece.baseGuid = guid;
                        piece.baseType = API_SlabID;
                        piece.normal = {0, 0, normal};
                        const double elevation = face.contours[0].vertices[0].world.z;
                        piece.planeOffset = -normal * elevation;
                        for (const auto &contour : face.contours) {
                            if (contour.vertices.GetSize () < 3)
                                return false;
                            RoomSurfacePieceContour output;
                            output.hole = contour.hole;
                            const auto origin = contour.vertices[0].world;
                            double twiceArea = 0;
                            for (UIndex i = 0; i < contour.vertices.GetSize (); ++i) {
                                const auto a = contour.vertices[i].world;
                                const auto b = contour.vertices[(i + 1) % contour.vertices.GetSize ()].world;
                                output.vertices.Push (a);
                                twiceArea += (a.x - origin.x) * (b.y - origin.y) - (b.x - origin.x) * (a.y - origin.y);
                            }
                            piece.area += (contour.hole ? -1 : 1) * std::fabs (twiceArea) / 2;
                            piece.contours.Push (output);
                        }
                        GS::Array<OtdSlab> selected;
                        if (!BuildSurfaceSlabs (piece, floorInd, selected))
                            return false;
                        for (const auto &slab : selected)
                            candidates.push_back ({slab, bottom, top});
                    }
                }
            }
            if (!found)
                return false;
        }
        // Потолок зоны — такой же кандидат: ближние перекрытия обрабатываются раньше,
        // а остаток покрывается отметкой height_ceil прежним алгоритмом остатка.
        const bool zoneCeilingCandidate = type == Ceil && nominal != nullptr && !onlyOnSlab;
        if (zoneCeilingCandidate) {
            Candidate zoneCeiling;
            zoneCeiling.slab = *nominal;
            zoneCeiling.slab.type = type;
            zoneCeiling.slab.floorInd = floorInd;
            zoneCeiling.slab.surface_geometry = true;
            zoneCeiling.slab.zBottom = nominal->zBottom;
            // Перепад до номинальной отметки не строим: собственных границ у кандидата нет.
            zoneCeiling.bottom = std::numeric_limits<double>::infinity ();
            zoneCeiling.top = -zoneCeiling.bottom;
            candidates.push_back (zoneCeiling);
        }
        // Изнутри зоны: верхние грани пола по убыванию, нижние грани потолка по возрастанию.
        std::sort (candidates.begin (), candidates.end (), [type] (const Candidate &a, const Candidate &b) {
            return type == Floor ? a.slab.zBottom > b.slab.zBottom : a.slab.zBottom < b.slab.zBottom;
        });
        Geometry::MultiPolygon2D remainder = footprint;
        GS::Array<OtdSlab> results;
        GS::Array<OtdWall> steps;
        double rejected = 0;
        for (UIndex i = 0; i < candidates.size (); ++i) {
            const auto &candidate = candidates[i];
            Geometry::MultiPolygon2D intersections;
            for (const auto &remaining : remainder)
                for (const auto &part : remaining.Intersect (candidate.slab.poly))
                    intersections.Push (part);
            if (intersections.IsEmpty ())
                continue;
            // Явное ограничение legacy: сохраняем только наибольшую несвязную часть пересечения/остатка.
            Geometry::Polygon2D processed = intersections.PopLargest ();
            if (processed.CalcArea () < 0.001)
                continue;
            Geometry::MultiPolygon2D accepted;
            accepted.Push (processed);
            // std::sort не задавал владельца при равных отметках; не выбираем его по GUID/порядку.
            for (UIndex j = 0; j < candidates.size (); ++j) {
                if (j == i || candidates[j].slab.base_guid == candidate.slab.base_guid ||
                    std::fabs (candidates[j].slab.zBottom - candidate.slab.zBottom) > DistanceTolerance)
                    continue;
                Geometry::MultiPolygon2D next;
                for (const auto &part : accepted) {
                    for (const auto &ambiguous : part.Intersect (candidates[j].slab.poly))
                        rejected += ambiguous.CalcArea ();
                    for (const auto &unique : part.Substract (candidates[j].slab.poly))
                        next.Push (unique);
                }
                accepted = next;
            }
            if (!accepted.IsEmpty ()) {
                OtdSlab slab = candidate.slab;
                slab.poly = accepted.PopLargest ();
                if (nominal != nullptr)
                    slab.material = nominal->material;
                results.Push (slab);
                Geometry::Polygon2D sidePolygon = slab.poly;
                sidePolygon.RemoveHoles ();
                if (nominal != nullptr && walls != nullptr && nominal->zBottom >= candidate.bottom &&
                    nominal->zBottom <= candidate.top) {
                    const double height =
                        type == Floor ? slab.zBottom - nominal->zBottom : nominal->zBottom - slab.zBottom;
                    if (height > 0.005) {
                        GS::Array<Point2D> coords;
                        GS::Array<PolyArcRec> arcs;
                        GS::Array<UIndex> ends;
                        GS::Array<Geometry::Polygon2D::ConstContourIterator> reversed;
                        const auto segmentedSides = sidePolygon.SegmentUp (ArcDivParams (0.00001));
                        if (segmentedSides.GetSize () != 1)
                            return false;
                        segmentedSides[0].CopyPolygonData (
                            coords, arcs, ends, reversed, Geometry::PolyCreateFlags_IdxRefInContourEndFromZero);
                        if (!arcs.IsEmpty () || ends.IsEmpty ())
                            return false;
                        for (UIndex k = 0; k <= ends[0]; ++k) {
                            const auto beg = coords[k];
                            const auto end = coords[k == ends[0] ? 0 : k + 1];
                            // Концы внутренней хорды могут лежать на границе; вычитаем только
                            // совпадающие конечные участки ребра, а не всё ребро по его концам.
                            GS::Array<Sector> internalEdges = {Sector (beg, end)};
                            for (const auto &remaining : remainder) {
                                GS::Array<Sector> nextEdges;
                                for (const auto &edge : internalEdges)
                                    nextEdges.Append (remaining.ElemPolyDiffBorderOnly (edge));
                                internalEdges = nextEdges;
                            }
                            for (const auto &edge : internalEdges) {
                                if (std::hypot (edge.c1.x - edge.c2.x, edge.c1.y - edge.c2.y) <= DistanceTolerance)
                                    continue;
                                OtdWall wall;
                                wall.begC = {edge.c2.x, edge.c2.y};
                                wall.endC = {edge.c1.x, edge.c1.y};
                                wall.zBottom = type == Floor ? nominal->zBottom : slab.zBottom;
                                wall.height = height;
                                wall.base_guid = slab.base_guid;
                                wall.base_type = API_SlabID;
                                wall.type = type;
                                wall.floorInd = floorInd;
                                wall.material = nominal->material;
                                wall.surface_geometry = true;
                                wall.slab_step = true;
                                steps.Push (wall);
                            }
                        }
                    }
                }
            }
            // Прежнее исключение: отверстия обработанного покрытия не становятся нижней/резервной отделкой.
            processed.RemoveHoles ();
            Geometry::MultiPolygon2D next;
            for (const auto &remaining : remainder)
                for (const auto &part : remaining.Substract (processed))
                    next.Push (part);
            remainder.Clear ();
            if (!next.IsEmpty ())
                remainder.Push (next.PopLargest ());
        }
        // Номинальный потолок уже был кандидатом: второй остаток на той же отметке не нужен.
        if (!onlyOnSlab && nominal != nullptr && !zoneCeilingCandidate && remainder.CalcArea () > 0.0001) {
            OtdSlab residual = *nominal;
            residual.poly = remainder.PopLargest ();
            residual.floorInd = floorInd;
            residual.type = type;
            residual.surface_geometry = true;
            results.Push (residual);
        }
        if (walls != nullptr)
            walls->Append (steps);
        slabs.Append (results);
        rejectedArea += rejected;
        return true;
    }
} // namespace Roombook
#endif
