#include "roombook/RoomSurfaceFinish.hpp"

#ifdef ServerMainVers_2400
    #include <algorithm>
    #include <cmath>
    #include <vector>

namespace Roombook {
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
        for (const auto &contour : piece.contours) {
            if (&contour != &piece.contours[0] && !contour.hole)
                return false;
            std::vector<API_Coord> points;
            for (const auto &p : Corners (contour)) {
                points.push_back ({Dot (Subtract (p, origin), along), p.z});
                levels.push_back (p.z);
            }
            for (size_t i = 0; i < points.size (); ++i) {
                const auto &a = points[i], &b = points[(i + 1) % points.size ()];
                // Разложение точно только для рёбер вдоль оси стены или её подъёма.
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
        GS::Array<OtdWall> results;
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
            // Чётность сохраняет вырезы и отверстия; габарит грани здесь не используется.
            for (size_t i = 0; i < crossings.size (); i += 2) {
                const double left = crossings[i], right = crossings[i + 1];
                const auto point = [&] (double s, double z) -> API_Coord3D {
                    return {origin.x + along.x * s + riseX * (z - origin.z),
                            origin.y + along.y * s + riseY * (z - origin.z),
                            z};
                };
                RoomSurfacePiece rectangle = piece;
                rectangle.contours.Clear ();
                RoomSurfacePieceContour contour;
                contour.vertices = {
                    point (left, lower), point (right, lower), point (right, upper), point (left, upper)};
                rectangle.contours.Push (std::move (contour));
                rectangle.area = (right - left) * (upper - lower) / horizontal;
                OtdWall wall;
                if (!BuildSurfaceWall (rectangle, floorInd, wall))
                    return false;
                area += rectangle.area;
                results.Push (std::move (wall));
            }
        }
        if (results.IsEmpty () ||
            std::fabs (area - piece.area) > std::max (AreaTolerance, 2 * perimeter * DistanceTolerance))
            return false;
        walls.Append (results);
        return true;
    }

    bool GroupSurfaceWallPieces (const GS::Array<RoomSurfacePiece> &pieces,
                                 short floorInd,
                                 GS::Array<RoomSurfacePiece> &grouped) {
        using Polygons = Geometry::MultiPolygon2D;
        const auto sameFace = [] (const RoomSurfaceSourceFace &a, const RoomSurfaceSourceFace &b) {
            return a.baseGuid == b.baseGuid && a.baseType == b.baseType && a.geometryGuid == b.geometryGuid &&
                   a.bodyIndex == b.bodyIndex && a.polygonIndex == b.polygonIndex &&
                   a.modelMaterialIndex == b.modelMaterialIndex;
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
        // Проверка адаптером не создаёт элементов; непредставимые куски остаются в диагностике.
        std::vector<const RoomSurfacePiece *> candidates;
        GS::Array<RoomSurfacePiece> results;
        for (const auto &piece : pieces) {
            GS::Array<OtdWall> trial;
            if (piece.status == RoomSurfacePieceStatus::Matched && piece.baseGuid != APINULLGuid &&
                piece.baseType == API_WallID && !piece.sources.IsEmpty () && BuildSurfaceWalls (piece, floorInd, trial))
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
                GS::Array<OtdWall> trial;
                if (!BuildSurfaceWalls (piece, floorInd, trial))
                    return false;
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
} // namespace Roombook
#endif
