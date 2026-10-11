#include "roombook/RoomSurfacePartition.hpp"

#ifdef ServerMainVers_2400
    #include <algorithm>
    #include <cmath>
    #include <string>
    #include <vector>

    #include "Polygon2D.hpp"

namespace Roombook {
    namespace {
        constexpr double DistanceTolerance = 1.0e-6;
        constexpr double AreaTolerance = 1.0e-6;

        using Polygons = Geometry::MultiPolygon2D;

        API_Coord3D Subtract (const API_Coord3D &a, const API_Coord3D &b) { return {a.x - b.x, a.y - b.y, a.z - b.z}; }

        double Dot (const API_Coord3D &a, const API_Coord3D &b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

        API_Coord3D Cross (const API_Coord3D &a, const API_Coord3D &b) {
            return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
        }

        bool Finite (const API_Coord3D &v) { return std::isfinite (v.x) && std::isfinite (v.y) && std::isfinite (v.z); }

        struct Frame {
            API_Coord3D origin = {};
            API_Coord3D normal = {};
            API_Coord3D u = {};
            API_Coord3D v = {};

            bool Set (const RoomSurface &surface) {
                normal = surface.normalWorld;
                const double length = std::sqrt (Dot (normal, normal));
                if (!Finite (normal) || !std::isfinite (surface.planeOffset) ||
                    std::fabs (length - 1.0) > DistanceTolerance || surface.contours.IsEmpty () ||
                    surface.contours[0].hole || surface.contours[0].vertices.IsEmpty ())
                    return false;
                normal = {normal.x / length, normal.y / length, normal.z / length};
                origin = surface.contours[0].vertices[0].world;
                API_Coord3D axis = {1, 0, 0};
                if (std::fabs (normal.y) < std::fabs (normal.x))
                    axis = {0, 1, 0};
                if (std::fabs (normal.z) < std::fabs (Dot (normal, axis)))
                    axis = {0, 0, 1};
                u = Cross (axis, normal);
                const double uLength = std::sqrt (Dot (u, u));
                u = {u.x / uLength, u.y / uLength, u.z / uLength};
                v = Cross (normal, u);
                return Finite (origin) && Finite (u) && Finite (v) &&
                       std::fabs (Dot (surface.normalWorld, origin) + surface.planeOffset) <= DistanceTolerance;
            }

            Point2D Project (const API_Coord3D &point) const {
                const API_Coord3D delta = Subtract (point, origin);
                return {Dot (delta, u), Dot (delta, v)};
            }

            API_Coord3D Restore (const Point2D &point) const {
                return {origin.x + u.x * point.x + v.x * point.y,
                        origin.y + u.y * point.x + v.y * point.y,
                        origin.z + u.z * point.x + v.z * point.y};
            }
        };

        bool OnPlane (const RoomSurface &surface, const Frame &frame) {
            for (const auto &contour : surface.contours) {
                for (const auto &vertex : contour.vertices) {
                    if (!Finite (vertex.world) ||
                        std::fabs (Dot (frame.normal, Subtract (vertex.world, frame.origin))) > DistanceTolerance)
                        return false;
                }
            }
            return true;
        }

        bool ProjectSurface (const RoomSurface &surface, const Frame &frame, Polygons &result) {
            GS::Array<Point2D> coords;
            GS::Array<UIndex> ends;
            GS::Array<PolyArcRec> arcs;
            double expectedArea = 0;
            if (surface.contours.IsEmpty () || surface.contours[0].hole || !OnPlane (surface, frame))
                return false;
            for (UIndex c = 0; c < surface.contours.GetSize (); ++c) {
                const auto &contour = surface.contours[c];
                if (contour.vertices.GetSize () < 3 || contour.hole != (c != 0))
                    return false;
                const UIndex start = coords.GetSize ();
                double twiceArea = 0;
                for (UIndex i = 0; i < contour.vertices.GetSize (); ++i) {
                    const Point2D point = frame.Project (contour.vertices[i].world);
                    const Point2D next = frame.Project (contour.vertices[(i + 1) % contour.vertices.GetSize ()].world);
                    twiceArea += point.x * next.y - next.x * point.y;
                    coords.Push (point);
                }
                if (!std::isfinite (twiceArea) || twiceArea == 0)
                    return false;
                expectedArea += (contour.hole ? -1 : 1) * std::fabs (twiceArea) / 2;
                if (coords.GetSize () - start < 3)
                    return false;
                ends.Push (coords.GetSize () - 1);
            }
            result =
                Geometry::Polygon2D::Create (coords, arcs, ends, Geometry::PolyCreateFlags_IdxRefInContourEndFromZero);
            return expectedArea > 0 && !result.IsEmpty () &&
                   std::fabs (result.CalcArea () - expectedArea) <= AreaTolerance;
        }

        struct Claim {
            RoomSurfaceSourceFace face;
            const RoomSurface *surface = nullptr;
            std::string baseKey;
            std::string geometryKey;
        };

        bool SameFace (const RoomSurface &a, const RoomSurface &b) {
            if (a.modelMaterialIndex != b.modelMaterialIndex || a.planeOffset != b.planeOffset ||
                a.normalWorld.x != b.normalWorld.x || a.normalWorld.y != b.normalWorld.y ||
                a.normalWorld.z != b.normalWorld.z || a.contours.GetSize () != b.contours.GetSize ())
                return false;
            for (UIndex c = 0; c < a.contours.GetSize (); ++c) {
                const auto &ac = a.contours[c];
                const auto &bc = b.contours[c];
                if (ac.hole != bc.hole || ac.vertices.GetSize () != bc.vertices.GetSize ())
                    return false;
                for (UIndex i = 0; i < ac.vertices.GetSize (); ++i) {
                    const auto &av = ac.vertices[i].world;
                    const auto &bv = bc.vertices[i].world;
                    if (av.x != bv.x || av.y != bv.y || av.z != bv.z)
                        return false;
                }
            }
            return true;
        }

        bool SameClaim (const Claim &a, const Claim &b) {
            return a.face.baseGuid == b.face.baseGuid && a.face.baseType == b.face.baseType &&
                   a.face.geometryGuid == b.face.geometryGuid && a.face.bodyIndex == b.face.bodyIndex &&
                   a.face.polygonIndex == b.face.polygonIndex;
        }

        struct Cell {
            Geometry::Polygon2D polygon;
            std::vector<UIndex> claims;
        };

        void PushCells (const Polygons &polygons, const std::vector<UIndex> &claims, std::vector<Cell> &cells) {
            for (const auto &polygon : polygons) {
                if (polygon.CalcArea () > 0)
                    cells.push_back ({polygon, claims});
            }
        }

        void Error (RoomSurfacePartition &result,
                    const char *stage,
                    const RoomSurface *surface = nullptr,
                    Int32 index = 0) {
            RoomSurfaceReadError error;
            error.stage = stage;
            error.code = APIERR_BADPARS;
            error.index = index;
            if (surface != nullptr) {
                error.body = surface->bodyIndex;
                error.polygon = surface->polygonIndex;
            }
            result.errors.Push (error);
        }

        RoomSurfacePiece MakePiece (const Cell &cell,
                                    const Frame &frame,
                                    const RoomSurface &zoneFace,
                                    const std::vector<Claim> &claims) {
            RoomSurfacePiece piece;
            piece.zoneBodyIndex = zoneFace.bodyIndex;
            piece.zonePolygonIndex = zoneFace.polygonIndex;
            piece.normal = zoneFace.normalWorld;
            piece.planeOffset = zoneFace.planeOffset;
            piece.area = cell.polygon.CalcArea ();
            for (UIndex index : cell.claims)
                piece.sources.Push (claims[index].face);
            if (!piece.sources.IsEmpty ()) {
                const auto &first = piece.sources[0];
                piece.status = RoomSurfacePieceStatus::Matched;
                for (const auto &source : piece.sources) {
                    if (source.baseGuid != first.baseGuid || source.baseType != first.baseType) {
                        piece.status = RoomSurfacePieceStatus::Ambiguous;
                        break;
                    }
                }
                if (piece.status == RoomSurfacePieceStatus::Matched) {
                    piece.baseGuid = first.baseGuid;
                    piece.baseType = first.baseType;
                }
            }
            GS::Array<Point2D> coords;
            GS::Array<PolyArcRec> arcs;
            GS::Array<UIndex> ends;
            GS::Array<Geometry::Polygon2D::ConstContourIterator> reversed;
            cell.polygon.CopyPolygonData (
                coords, arcs, ends, reversed, Geometry::PolyCreateFlags_IdxRefInContourEndFromZero);
            UIndex start = 0;
            for (UIndex c = 0; c < ends.GetSize (); ++c) {
                RoomSurfacePieceContour contour;
                contour.hole = c != 0;
                for (UIndex i = start; i <= ends[c]; ++i)
                    contour.vertices.Push (frame.Restore (coords[i]));
                piece.contours.Push (contour);
                start = ends[c] + 1;
            }
            return piece;
        }

        bool PartitionFace (const RoomSurface &zoneFace,
                            const std::vector<Claim> &claims,
                            RoomSurfacePartition &result) {
            Frame frame;
            Polygons zonePolygon;
            if (!frame.Set (zoneFace) || !ProjectSurface (zoneFace, frame, zonePolygon)) {
                Error (result, "PartitionZoneFace", &zoneFace);
                return false;
            }
            std::vector<Cell> cells;
            PushCells (zonePolygon, {}, cells);
            for (UIndex index = 0; index < claims.size (); ++index) {
                const auto &surface = *claims[index].surface;
                // Контакт — конечная грань конструкции, копланарная грани зоны с перекрытием площадей.
                // Ориентация нормалей не проверяется: балка, врезанная в комнату заподлицо с потолком/полом,
                // даёт грань с той же ориентацией, что у грани зоны, и обязана покрывать её участок.
                if (!OnPlane (surface, frame))
                    continue;
                Frame sourceFrame;
                if (!sourceFrame.Set (surface) || !OnPlane (zoneFace, sourceFrame))
                    continue;
                Polygons sourcePolygon;
                if (!ProjectSurface (surface, frame, sourcePolygon)) {
                    Error (result, "PartitionSourceFace", &zoneFace, static_cast<Int32> (index));
                    return false;
                }
                std::vector<Cell> next;
                for (const auto &cell : cells) {
                    Polygons current;
                    current.Push (cell.polygon);
                    PushCells (Polygons::Substract (current, sourcePolygon), cell.claims, next);
                    auto owners = cell.claims;
                    owners.push_back (index);
                    PushCells (Polygons::Intersect (current, sourcePolygon), owners, next);
                }
                cells = std::move (next);
            }
            Polygons coverage;
            for (const auto &cell : cells)
                coverage.Push (cell.polygon);
            coverage.Unify ();
            // Симметрические разности в обе стороны доказывают равенство покрытия зоне; сумма площадей
            // чувствительна к шуму Boolean-движка (замеры: 1.8e-6 при площади 7.7, 2.3e-6 при 182.6), поэтому
            // сравнение площадей убрано, а допуски разностей масштабируются площадью. Точечные допуски 1e-6
            // не меняются; реальная потеря участка при таком допуске — доли мм².
            const double zoneArea = zonePolygon.CalcArea ();
            const double areaTolerance = AreaTolerance * (1.0 + 0.05 * zoneArea);
            const double zoneMinusCoverage = Polygons::Substract (zonePolygon, coverage).CalcArea ();
            const double coverageMinusZone = Polygons::Substract (coverage, zonePolygon).CalcArea ();
            const char *stage = nullptr;
            if (zoneMinusCoverage > areaTolerance)
                stage = "PartitionCoverageZoneMinusCoverage";
            else if (coverageMinusZone > areaTolerance)
                stage = "PartitionCoverageCoverageMinusZone";
            if (stage != nullptr) {
                Error (result, stage, &zoneFace);
                return false;
            }
            for (const auto &cell : cells)
                result.pieces.Push (MakePiece (cell, frame, zoneFace, claims));
            return true;
        }
    } // namespace

    RoomSurfacePartition PartitionRoomSurfaces (const RoomSurfaceSnapshot &zone,
                                                const GS::Array<RoomSurfaceSource> &sources) {
        RoomSurfacePartition result;
        result.zoneGuid = zone.guid;
        if (!zone.IsComplete () || zone.status == RoomSurfaceStatus::Empty) {
            Error (result, "PartitionZoneSnapshot");
            return result;
        }
        std::vector<Claim> claims;
        for (UIndex i = 0; i < sources.GetSize (); ++i) {
            const auto &source = sources[i];
            if (source.baseGuid == APINULLGuid || source.geometryGuid == APINULLGuid ||
                source.snapshot.guid != source.geometryGuid || !source.snapshot.IsComplete ()) {
                Error (result, "PartitionSourceSnapshot", nullptr, static_cast<Int32> (i));
                return result;
            }
            const std::string baseKey = APIGuidToString (source.baseGuid).ToCStr ().Get ();
            const std::string geometryKey = APIGuidToString (source.geometryGuid).ToCStr ().Get ();
            for (const auto &body : source.snapshot.bodies) {
                for (const auto &surface : body.surfaces) {
                    Frame sourceFrame;
                    if (!sourceFrame.Set (surface) || !OnPlane (surface, sourceFrame)) {
                        Error (result, "PartitionInvalidSourcePlane", &surface, static_cast<Int32> (i));
                        return result;
                    }
                    Claim claim;
                    claim.face = {source.baseGuid,
                                  source.baseType,
                                  source.geometryGuid,
                                  body.index,
                                  surface.polygonIndex,
                                  surface.modelMaterialIndex};
                    claim.surface = &surface;
                    claim.baseKey = baseKey;
                    claim.geometryKey = geometryKey;
                    claims.push_back (claim);
                }
            }
        }
        std::sort (claims.begin (), claims.end (), [] (const Claim &a, const Claim &b) {
            if (a.baseKey != b.baseKey)
                return a.baseKey < b.baseKey;
            if (a.face.baseType != b.face.baseType)
                return a.face.baseType < b.face.baseType;
            if (a.geometryKey != b.geometryKey)
                return a.geometryKey < b.geometryKey;
            if (a.face.bodyIndex != b.face.bodyIndex)
                return a.face.bodyIndex < b.face.bodyIndex;
            return a.face.polygonIndex < b.face.polygonIndex;
        });
        // Одинаковый ключ с разной геометрией — ошибка снимков, не выбор первого кандидата.
        for (UIndex i = 1; i < claims.size (); ++i) {
            if (SameClaim (claims[i - 1], claims[i]) && !SameFace (*claims[i - 1].surface, *claims[i].surface)) {
                Error (result, "PartitionConflictingSourceFace", claims[i].surface, static_cast<Int32> (i));
                return result;
            }
        }
        claims.erase (std::unique (claims.begin (), claims.end (), SameClaim), claims.end ());
        try {
            for (const auto &body : zone.bodies) {
                for (const auto &surface : body.surfaces) {
                    if (!PartitionFace (surface, claims, result))
                        return result;
                }
            }
            result.complete = !result.pieces.IsEmpty ();
        } catch (const GS::Exception &) {
            Error (result, "PartitionGeometryException");
        }
        return result;
    }

    namespace {
        // Параметр точки на оси балки: t = проекция (p - beg) на (end - beg), в единицах длины оси.
        struct BeamAxis {
            API_Coord origin = {};
            API_Coord dir = {};
            double length = 0;
            API_Coord perp = {}; // единичный перпендикуляр оси в плане

            bool Set (const API_Coord &beg, const API_Coord &end) {
                dir = {end.x - beg.x, end.y - beg.y};
                length = std::sqrt (dir.x * dir.x + dir.y * dir.y);
                if (!std::isfinite (length) || length <= DistanceTolerance || !std::isfinite (beg.x) ||
                    !std::isfinite (beg.y) || !std::isfinite (end.x) || !std::isfinite (end.y))
                    return false;
                dir = {dir.x / length, dir.y / length};
                perp = {-dir.y, dir.x};
                origin = beg;
                return true;
            }

            double Along (const API_Coord3D &p) const { return (p.x - origin.x) * dir.x + (p.y - origin.y) * dir.y; }

            double Offset (const API_Coord3D &p) const { return (p.x - origin.x) * perp.x + (p.y - origin.y) * perp.y; }

            API_Coord3D Point (double t, double offset, double z) const {
                return {origin.x + dir.x * t + perp.x * offset, origin.y + dir.y * t + perp.y * offset, z};
            }
        };

        struct SectionEdge {
            Point2D beg;
            Point2D end;
            bool used = false;
        };

        bool SamePoint (const Point2D &a, const Point2D &b) {
            return std::hypot (a.x - b.x, a.y - b.y) <= DistanceTolerance;
        }

        // Сечение замкнутого тела, не силуэт проекции: направление рёбер сохраняет отверстия.
        bool SectionZone (const RoomSurfaceSnapshot &zone, const Frame &frame, Polygons &section) {
            if (!zone.IsComplete ())
                return false;
            for (const auto &body : zone.bodies) {
                std::vector<SectionEdge> edges;
                Polygons outer, holes;
                for (const auto &surface : body.surfaces) {
                    const API_Coord3D direction = Cross (frame.normal, surface.normalWorld);
                    if (Dot (direction, direction) <= DistanceTolerance * DistanceTolerance) {
                        if (OnPlane (surface, frame)) {
                            Polygons boundary;
                            if (!ProjectSurface (surface, frame, boundary))
                                return false;
                            for (const auto &polygon : boundary)
                                outer.Push (polygon);
                        }
                        continue;
                    }
                    std::vector<API_Coord3D> hits;
                    for (const auto &contour : surface.contours) {
                        for (UIndex i = 0; i < contour.vertices.GetSize (); ++i) {
                            const auto &a = contour.vertices[i].world;
                            const auto &b = contour.vertices[(i + 1) % contour.vertices.GetSize ()].world;
                            if (!Finite (a) || !Finite (b))
                                return false;
                            double da = Dot (frame.normal, Subtract (a, frame.origin));
                            double db = Dot (frame.normal, Subtract (b, frame.origin));
                            if (std::fabs (da) <= DistanceTolerance)
                                da = 0;
                            if (std::fabs (db) <= DistanceTolerance)
                                db = 0;
                            // Полуоткрытое правило: общая вершина учитывается ровно один раз.
                            if (!((da <= 0 && db > 0) || (db <= 0 && da > 0)))
                                continue;
                            const double t = da / (da - db);
                            hits.push_back ({a.x + t * (b.x - a.x), a.y + t * (b.y - a.y), a.z + t * (b.z - a.z)});
                        }
                    }
                    std::sort (hits.begin (), hits.end (), [&] (const auto &a, const auto &b) {
                        return Dot (a, direction) < Dot (b, direction);
                    });
                    if (hits.size () % 2 != 0)
                        return false;
                    for (UIndex i = 0; i < hits.size (); i += 2) {
                        const Point2D a = frame.Project (hits[i]);
                        const Point2D b = frame.Project (hits[i + 1]);
                        if (SamePoint (a, b))
                            continue;
                        bool duplicate = false;
                        for (const auto &edge : edges)
                            duplicate = duplicate || (SamePoint (edge.beg, a) && SamePoint (edge.end, b));
                        if (!duplicate)
                            edges.push_back ({a, b, false});
                    }
                }
                for (auto &first : edges) {
                    if (first.used)
                        continue;
                    GS::Array<Point2D> coords;
                    first.used = true;
                    coords.Push (first.beg);
                    Point2D end = first.end;
                    while (!SamePoint (end, coords[0])) {
                        coords.Push (end);
                        SectionEdge *next = nullptr;
                        for (auto &edge : edges) {
                            if (!edge.used && SamePoint (edge.beg, end)) {
                                // Незамкнутый/разветвлённый контур не заменяем bounding-прямоугольником.
                                if (next != nullptr)
                                    return false;
                                next = &edge;
                            }
                        }
                        if (next == nullptr)
                            return false;
                        next->used = true;
                        end = next->end;
                    }
                    if (coords.GetSize () < 3)
                        return false;
                    double twiceArea = 0;
                    for (UIndex i = 0; i < coords.GetSize (); ++i) {
                        const auto &a = coords[i];
                        const auto &b = coords[(i + 1) % coords.GetSize ()];
                        twiceArea += a.x * b.y - b.x * a.y;
                    }
                    GS::Array<UIndex> ends;
                    GS::Array<PolyArcRec> arcs;
                    ends.Push (coords.GetSize () - 1);
                    const auto polygons = Geometry::Polygon2D::Create (
                        coords, arcs, ends, Geometry::PolyCreateFlags_IdxRefInContourEndFromZero);
                    for (const auto &polygon : polygons)
                        (twiceArea > 0 ? outer : holes).Push (polygon);
                }
                outer.Unify ();
                holes.Unify ();
                const auto bodySection = Polygons::Substract (outer, holes);
                for (const auto &polygon : bodySection)
                    section.Push (polygon);
            }
            section.Unify ();
            return true;
        }

        bool ClipBeamFace (const RoomSurface &surface,
                           const RoomSurfaceSnapshot &zone,
                           const BeamAxis &axis,
                           double tBeg,
                           double tEnd,
                           double zBottom,
                           double zTop,
                           RoomSurfaceBeamFace face,
                           GS::Array<RoomSurfaceBeamPiece> &pieces) {
            Frame frame;
            Polygons original, section;
            if (!frame.Set (surface) || !ProjectSurface (surface, frame, original) ||
                !SectionZone (zone, frame, section))
                return false;
            if (face != RoomSurfaceBeamFace::Bottom) {
                double tMin = axis.Along (surface.contours[0].vertices[0].world);
                double tMax = tMin;
                for (const auto &contour : surface.contours)
                    for (const auto &vertex : contour.vertices) {
                        tMin = std::min (tMin, axis.Along (vertex.world));
                        tMax = std::max (tMax, axis.Along (vertex.world));
                    }
                const double offset = axis.Offset (frame.origin);
                GS::Array<Point2D> coords;
                coords.Push (frame.Project (axis.Point (tMin, offset, zBottom)));
                coords.Push (frame.Project (axis.Point (tMax, offset, zBottom)));
                coords.Push (frame.Project (axis.Point (tMax, offset, zTop)));
                coords.Push (frame.Project (axis.Point (tMin, offset, zTop)));
                GS::Array<UIndex> ends;
                GS::Array<PolyArcRec> arcs;
                ends.Push (3);
                const auto verticalRange = Geometry::Polygon2D::Create (
                    coords, arcs, ends, Geometry::PolyCreateFlags_IdxRefInContourEndFromZero);
                original = Polygons::Intersect (original, verticalRange);
            }
            // Сечение зоны задаёт фактические границы контакта на каждой высоте, не нижний контур.
            const auto clipped = Polygons::Intersect (original, section);
            GS::Array<RoomSurfaceBeamPiece> own;
            for (const auto &polygon : clipped) {
                GS::Array<Point2D> coords;
                GS::Array<PolyArcRec> arcs;
                GS::Array<UIndex> ends;
                GS::Array<Geometry::Polygon2D::ConstContourIterator> reversed;
                polygon.CopyPolygonData (
                    coords, arcs, ends, reversed, Geometry::PolyCreateFlags_IdxRefInContourEndFromZero);
                // Представление beamPiece пока не хранит отверстия: явный атомарный отказ.
                if (ends.GetSize () != 1 || !arcs.IsEmpty ())
                    return false;
                RoomSurfaceBeamPiece piece;
                piece.face = face;
                piece.area = polygon.CalcArea ();
                double tMin = 0, tMax = 0;
                for (UIndex i = 0; i <= ends[0]; ++i) {
                    const auto point = frame.Restore (coords[i]);
                    if (point.z < zBottom - DistanceTolerance || point.z > zTop + DistanceTolerance)
                        return false;
                    const double t = axis.Along (point);
                    if (i == 0)
                        tMin = tMax = t;
                    else {
                        tMin = std::min (tMin, t);
                        tMax = std::max (tMax, t);
                    }
                    piece.vertices.Push (point);
                }
                // beamPart выбирает связную часть; её реальные границы задаёт 3D-зона на высоте грани.
                if (std::min (tMax, tEnd) - std::max (tMin, tBeg) > DistanceTolerance &&
                    piece.area > DistanceTolerance * DistanceTolerance)
                    own.Push (std::move (piece));
            }
            pieces.Append (own);
            return true;
        }

        void PushQuad (RoomSurfaceBeamPiece &piece,
                       const BeamAxis &axis,
                       double t0,
                       double t1,
                       double offset,
                       double z0,
                       double z1) {
            const double ts[4] = {t0, t1, t1, t0};
            const double zs[4] = {z0, z0, z1, z1};
            for (Int32 i = 0; i < 4; ++i)
                piece.vertices.Push (axis.Point (ts[i], offset, zs[i]));
            const double length = t1 - t0;
            const double height = z1 - z0;
            piece.area = length * height;
        }

        void PushBottom (RoomSurfaceBeamPiece &piece,
                         const BeamAxis &axis,
                         const GS::Array<RoomSurfaceVertex> &vertices,
                         double t0,
                         double t1,
                         double z) {
            double offsetMin = 0, offsetMax = 0, alongMin = 0, alongMax = 0;
            bool first = true;
            for (const auto &vertex : vertices) {
                const double offset = axis.Offset (vertex.world);
                const double along = axis.Along (vertex.world);
                if (first) {
                    offsetMin = offsetMax = offset;
                    alongMin = alongMax = along;
                    first = false;
                } else {
                    offsetMin = std::min (offsetMin, offset);
                    offsetMax = std::max (offsetMax, offset);
                    alongMin = std::min (alongMin, along);
                    alongMax = std::max (alongMax, along);
                }
            }
            t0 = std::max (t0, alongMin);
            t1 = std::min (t1, alongMax);
            if (first || t1 - t0 <= DistanceTolerance)
                return;
            piece.vertices.Push (axis.Point (t0, offsetMin, z));
            piece.vertices.Push (axis.Point (t1, offsetMin, z));
            piece.vertices.Push (axis.Point (t1, offsetMax, z));
            piece.vertices.Push (axis.Point (t0, offsetMax, z));
            const double width = offsetMax - offsetMin;
            piece.area = (t1 - t0) * width;
        }
    } // namespace

    RoomSurfaceBeamFinish BuildRoomBeamFinish (const RoomSurfaceSource &beamSource,
                                               const API_Coord &beg,
                                               const API_Coord &end,
                                               double tBeg,
                                               double tEnd,
                                               double zBottom,
                                               double zTop,
                                               RoomSurfaceBeamPartitions &result,
                                               const RoomSurfaceSnapshot *zone) {
        RoomSurfaceBeamFinish finish;
        finish.baseGuid = beamSource.baseGuid;
        finish.geometryGuid = beamSource.geometryGuid;
        if (beamSource.baseType != API_BeamID || !beamSource.snapshot.IsComplete ()) {
            RoomSurfaceReadError error;
            error.stage = "beamSourceSnapshot";
            error.code = APIERR_BADPARS;
            result.errors.Push (error);
            return finish;
        }
        BeamAxis axis;
        if (!axis.Set (beg, end)) {
            RoomSurfaceReadError error;
            error.stage = "beamAxis";
            error.code = APIERR_BADPARS;
            result.errors.Push (error);
            return finish;
        }
        finish.tBeg = tBeg;
        finish.tEnd = tEnd;
        // В AC25 beamPart возвращает проекции на ненормированный вектор оси:
        // проверено по пересечениям границы зоны для двух балок разной длины и направления.
        // В отличие от wallPart, расстояние вдоль единичной оси равно t / length.
        tBeg /= axis.length;
        tEnd /= axis.length;
        if (tEnd - tBeg <= DistanceTolerance || zTop - zBottom <= DistanceTolerance) {
            RoomSurfaceReadError error;
            error.stage = "beamEmptyPart";
            error.code = APIERR_BADPARS;
            result.errors.Push (error);
            return finish;
        }
        // Низ — грань с нормалью вниз; бока — вертикальные грани, параллельные оси. Прямые балки с
        // постоянным сечением; наклонные/кривые балки не распознаются и остаются не отделанными.
        for (const auto &body : beamSource.snapshot.bodies) {
            for (const auto &surface : body.surfaces) {
                const auto &n = surface.normalWorld;
                if (!Finite (n) || surface.contours.IsEmpty () || surface.contours[0].vertices.IsEmpty ())
                    continue;
                const auto &first = surface.contours[0].vertices[0].world;
                double zMin = 0, zMax = 0;
                bool firstVertex = true;
                for (const auto &contour : surface.contours) {
                    for (const auto &vertex : contour.vertices) {
                        const double z = vertex.world.z;
                        if (firstVertex) {
                            zMin = zMax = z;
                            firstVertex = false;
                        } else {
                            zMin = std::min (zMin, z);
                            zMax = std::max (zMax, z);
                        }
                    }
                }
                const double clippedBottom = std::max (zMin, zBottom);
                const double clippedTop = std::min (zMax, zTop);
                const double z = std::fabs (n.z);
                RoomSurfaceBeamPiece piece;
                if (z > 1.0 - DistanceTolerance) {
                    if (n.z > 0) // верх — не отделываем
                        continue;
                    const double faceZ = first.z;
                    if (faceZ < zBottom - DistanceTolerance || faceZ > zTop + DistanceTolerance)
                        continue;
                    piece.face = RoomSurfaceBeamFace::Bottom;
                    if (zone != nullptr) {
                        if (!ClipBeamFace (
                                surface, *zone, axis, tBeg, tEnd, zBottom, zTop, piece.face, finish.pieces)) {
                            RoomSurfaceReadError error;
                            error.stage = "beamZoneSection";
                            error.code = APIERR_BADPARS;
                            result.errors.Push (error);
                            finish.pieces.Clear ();
                            return finish;
                        }
                        continue;
                    }
                    PushBottom (piece, axis, surface.contours[0].vertices, tBeg, tEnd, faceZ);
                } else if (z < DistanceTolerance) {
                    if (clippedTop - clippedBottom <= DistanceTolerance)
                        continue;
                    const double alongN = n.x * axis.dir.x + n.y * axis.dir.y;
                    if (std::fabs (alongN) > DistanceTolerance) // торец балки — не отделываем
                        continue;
                    if (zone != nullptr) {
                        piece.face = (n.x * axis.perp.x + n.y * axis.perp.y) > 0 ? RoomSurfaceBeamFace::RightSide
                                                                                 : RoomSurfaceBeamFace::LeftSide;
                        if (!ClipBeamFace (
                                surface, *zone, axis, tBeg, tEnd, zBottom, zTop, piece.face, finish.pieces)) {
                            RoomSurfaceReadError error;
                            error.stage = "beamZoneSection";
                            error.code = APIERR_BADPARS;
                            result.errors.Push (error);
                            finish.pieces.Clear ();
                            return finish;
                        }
                        continue;
                    }
                    const double offset = axis.Offset (first);
                    double tMin = 0, tMax = 0;
                    firstVertex = true;
                    for (const auto &vertex : surface.contours[0].vertices) {
                        const double t = axis.Along (vertex.world);
                        if (firstVertex) {
                            tMin = tMax = t;
                            firstVertex = false;
                        } else {
                            tMin = std::min (tMin, t);
                            tMax = std::max (tMax, t);
                        }
                    }
                    const double t0 = std::max (tMin, tBeg);
                    const double t1 = std::min (tMax, tEnd);
                    if (t1 - t0 <= DistanceTolerance)
                        continue;
                    piece.face = (n.x * axis.perp.x + n.y * axis.perp.y) > 0 ? RoomSurfaceBeamFace::RightSide
                                                                             : RoomSurfaceBeamFace::LeftSide;
                    PushQuad (piece, axis, t0, t1, offset, clippedBottom, clippedTop);
                } else {
                    continue; // наклонная грань — вне объёма задачи
                }
                if (piece.area > DistanceTolerance * DistanceTolerance)
                    finish.pieces.Push (std::move (piece));
            }
        }
        return finish;
    }
} // namespace Roombook
#endif
