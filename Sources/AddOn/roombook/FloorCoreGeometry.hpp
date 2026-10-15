#pragma once
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

// Pure finite-boundary geometry. No SDK calls or mutation of wall geometry.
namespace Roombook {
    namespace FloorCoreGeometry {
        constexpr double Tolerance = 1.e-6;

        struct Point {
            double x = 0, y = 0;
        };

        inline Point operator+ (Point a, Point b) { return {a.x + b.x, a.y + b.y}; }

        inline Point operator- (Point a, Point b) { return {a.x - b.x, a.y - b.y}; }

        inline Point operator* (Point a, double k) { return {a.x * k, a.y * k}; }

        inline double Dot (Point a, Point b) { return a.x * b.x + a.y * b.y; }

        inline double Cross (Point a, Point b) { return a.x * b.y - a.y * b.x; }

        inline double Length (Point a) { return std::hypot (a.x, a.y); }

        struct Contact {
            Point begin, end;
            double offset = 0;
            std::size_t owner = 0;
        };

        struct Edge {
            Point begin, end;
            bool straight = true;
        };

        struct Portion {
            Point begin, end, shift;
        };

        using Patch = std::vector<Point>;

        // Layer types have already been normalized by LayerStructype.
        template <class Layers, class Size>
        bool Offset (const Layers &layers, Size count, bool flipped, int core, int finish, double &offset) {
            offset = 0;
            double depth = 0, firstFinish = -1;
            for (Size i = 0; i < count; ++i) {
                const auto &layer = layers[flipped ? count - 1 - i : i];
                if (!std::isfinite (layer.fillThick) || layer.fillThick < 0)
                    return false;
                if (layer.structype == core) {
                    offset = depth;
                    return true;
                }
                depth += layer.fillThick;
                if (!std::isfinite (depth))
                    return false;
                if (layer.structype == finish && firstFinish < 0)
                    firstFinish = depth;
            }
            if (firstFinish < 0)
                return false;
            offset = firstFinish;
            return true;
        }

        // Input contour is oriented with room interior on the left. Arcs are retained
        // by the SDK union adapter, never replaced with their chord.
        inline std::vector<Patch> Extensions (const std::vector<Edge> &edges, const std::vector<Contact> &contacts) {
            std::vector<Portion> portions;
            std::vector<Patch> patches;
            for (const auto &edge : edges) {
                const Point delta = edge.end - edge.begin;
                const double length = Length (delta);
                if (!edge.straight || length <= Tolerance) {
                    portions.push_back ({edge.begin, edge.end, {}});
                    continue;
                }
                const Point dir = delta * (1 / length), right = {dir.y, -dir.x};

                struct Match {
                    double begin, end, offset;
                    std::size_t owner;
                };

                std::vector<Match> matches;
                std::vector<double> cuts = {0, length};
                for (const auto &contact : contacts) {
                    const Point cdelta = contact.end - contact.begin;
                    const double clen = Length (cdelta);
                    if (clen <= Tolerance || !std::isfinite (contact.offset) || contact.offset < 0 ||
                        Dot (dir, cdelta) <= 0 || std::fabs (Cross (dir, cdelta)) > Tolerance * clen ||
                        std::fabs (Cross (dir, contact.begin - edge.begin)) > Tolerance ||
                        std::fabs (Cross (dir, contact.end - edge.begin)) > Tolerance)
                        continue;
                    const double a = std::max (0.0, Dot (contact.begin - edge.begin, dir));
                    const double b = std::min (length, Dot (contact.end - edge.begin, dir));
                    if (b - a <= Tolerance)
                        continue;
                    matches.push_back ({a, b, contact.offset, contact.owner});
                    cuts.push_back (a);
                    cuts.push_back (b);
                }
                std::sort (cuts.begin (), cuts.end ());
                // Keep exact endpoints: tolerance-sized spans are not expanded.
                cuts.erase (std::unique (cuts.begin (), cuts.end ()), cuts.end ());
                for (std::size_t i = 1; i < cuts.size (); ++i) {
                    double offset = 0;
                    std::size_t owner = 0;
                    bool found = false, ambiguous = false;
                    const double middle = (cuts[i - 1] + cuts[i]) / 2;
                    for (const auto &match : matches) {
                        if (middle < match.begin || middle > match.end)
                            continue;
                        if (found && (owner != match.owner || std::fabs (offset - match.offset) > Tolerance))
                            ambiguous = true;
                        if (!found) {
                            found = true;
                            owner = match.owner;
                            offset = match.offset;
                        }
                    }
                    if (ambiguous || cuts[i] - cuts[i - 1] <= Tolerance)
                        offset = 0;
                    Portion part = {edge.begin + dir * cuts[i - 1], edge.begin + dir * cuts[i], right * offset};
                    portions.push_back (part);
                    if (offset > Tolerance)
                        patches.push_back ({part.begin, part.end, part.end + part.shift, part.begin + part.shift});
                }
            }
            // Convex corner miter between two actually accepted finite portions.
            // Concave corners are already covered by the overlapping strips.
            for (std::size_t i = 0; i < portions.size (); ++i) {
                const auto &a = portions[i], &b = portions[(i + 1) % portions.size ()];
                if (Length (a.end - b.begin) > Tolerance || Length (a.shift) <= Tolerance ||
                    Length (b.shift) <= Tolerance)
                    continue;
                const Point da = a.end - a.begin, db = b.end - b.begin;
                const double determinant = Cross (da, db);
                if (determinant <= Tolerance * Length (da) * Length (db))
                    continue;
                const Point pa = a.end + a.shift, pb = b.begin + b.shift;
                const Point intersection = pa + da * (Cross (pb - pa, db) / determinant);
                if (std::isfinite (intersection.x) && std::isfinite (intersection.y))
                    patches.push_back ({a.end, pa, intersection, pb});
            }
            return patches;
        }
    } // namespace FloorCoreGeometry
} // namespace Roombook
