#pragma once
#include <algorithm>
#include <cmath>
#include <vector>

namespace Roombook {
    namespace AnalyticGeometry {
        struct Point {
            double x, y;
        };

        // Внешняя грань отстоит от оси на SDK offsetFromOutside; flipped зеркалит всё сечение.
        inline bool WallSides (Point beg,
                               Point end,
                               double thickness,
                               double outsideOffset,
                               bool flipped,
                               Point &outsideBeg,
                               Point &outsideEnd,
                               Point &insideBeg,
                               Point &insideEnd) {
            const double dx = end.x - beg.x, dy = end.y - beg.y;
            const double length = std::hypot (dx, dy);
            if (!std::isfinite (length) || length <= 0 || !std::isfinite (thickness) || thickness <= 0 ||
                !std::isfinite (outsideOffset))
                return false;
            const double sign = flipped ? -1 : 1;
            const Point left{-dy / length * sign, dx / length * sign};
            outsideBeg = {beg.x + left.x * outsideOffset, beg.y + left.y * outsideOffset};
            outsideEnd = {end.x + left.x * outsideOffset, end.y + left.y * outsideOffset};
            insideBeg = {outsideBeg.x - left.x * thickness, outsideBeg.y - left.y * thickness};
            insideEnd = {outsideEnd.x - left.x * thickness, outsideEnd.y - left.y * thickness};
            return true;
        }

        inline bool OrdinaryWall (
            bool profile, bool straight, bool vertical, bool rectangular, bool logWall, bool topLinked) {
            // Привязка к этажу не меняет прямоугольное сечение; высоту из API_Element проверяет builder.
            (void)topLinked;
            return !profile && straight && vertical && rectangular && !logWall;
        }

        struct Rect {
            double left, bottom, right, top;
        };

        inline bool Valid (const Rect &r) {
            return std::isfinite (r.left) && std::isfinite (r.bottom) && std::isfinite (r.right) &&
                   std::isfinite (r.top) && r.left < r.right && r.bottom < r.top;
        }

        // Последовательная разность сохраняет объединение вырезов, включая перекрывающиеся и граничные проёмы.
        inline bool SubtractOpenings (const Rect &base, const std::vector<Rect> &openings, std::vector<Rect> &result) {
            result.clear ();
            if (!Valid (base))
                return false;
            result.push_back (base);
            for (const auto &opening : openings) {
                if (!Valid (opening)) {
                    result.clear ();
                    return false;
                }
                std::vector<Rect> next;
                for (const auto &r : result) {
                    const Rect cut{std::max (r.left, opening.left),
                                   std::max (r.bottom, opening.bottom),
                                   std::min (r.right, opening.right),
                                   std::min (r.top, opening.top)};
                    if (!Valid (cut)) {
                        next.push_back (r);
                        continue;
                    }
                    for (const auto &part : {Rect{r.left, r.bottom, cut.left, r.top},
                                             Rect{cut.right, r.bottom, r.right, r.top},
                                             Rect{cut.left, r.bottom, cut.right, cut.bottom},
                                             Rect{cut.left, cut.top, cut.right, r.top}})
                        if (Valid (part))
                            next.push_back (part);
                }
                result = std::move (next);
            }
            return true;
        }

        enum class SlabCandidateRole { None, Floor, Ceiling };

        // Пол включает опоры ниже и пересекающие отметку; потолок — любой низ строго выше пола.
        inline SlabCandidateRole SlabRole (double bottom, double top, double floor) {
            if (!std::isfinite (bottom) || !std::isfinite (top) || !std::isfinite (floor) || bottom >= top)
                return SlabCandidateRole::None;
            return bottom <= floor ? SlabCandidateRole::Floor : SlabCandidateRole::Ceiling;
        }

        inline bool SlabLevels (double reference, double offsetFromTop, double thickness, double &bottom, double &top) {
            top = reference + offsetFromTop;
            bottom = top - thickness;
            return std::isfinite (reference) && std::isfinite (offsetFromTop) && std::isfinite (thickness) &&
                   thickness > 0 && std::isfinite (bottom) && std::isfinite (top) && bottom < top;
        }
    } // namespace AnalyticGeometry
} // namespace Roombook
