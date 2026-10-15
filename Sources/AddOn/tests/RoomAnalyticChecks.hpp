#pragma once
#include <limits>

#include "roombook/RoomAnalyticGeometry.hpp"

namespace Roombook {
    // Общие проверки выполняются SDK-набором и независимым executable над тем же production-helper.
    template <class Check> void CheckRoomAnalyticGeometry (Check check) {
        using namespace AnalyticGeometry;
        const auto almostEqual = [] (double a, double b) { return std::abs (a - b) < 1e-10; };
        Point ob, oe, ib, ie;
        check (WallSides ({0, 0}, {4, 0}, 0.3, 0.1, false, ob, oe, ib, ie) && almostEqual (ob.y, 0.1) &&
                   almostEqual (ib.y, -0.2) && almostEqual (oe.x, 4),
               "wall reference offset");
        check (WallSides ({4, 0}, {0, 0}, 0.3, 0.1, false, ob, oe, ib, ie) && almostEqual (ob.y, -0.1) &&
                   almostEqual (ib.y, 0.2),
               "reversed wall sides");
        check (WallSides ({0, 0}, {4, 0}, 0.3, 0.1, true, ob, oe, ib, ie) && almostEqual (ob.y, -0.1) &&
                   almostEqual (ib.y, 0.2),
               "flipped wall sides");
        check (!WallSides ({0, 0}, {0, 0}, 0.3, 0, false, ob, oe, ib, ie), "zero wall rejected");
        std::vector<Rect> result;
        const Rect base{0, 0, 4, 3};
        const auto area = [&] () {
            double sum = 0;
            for (const auto &part : result)
                sum += (part.right - part.left) * (part.top - part.bottom);
            return sum;
        };
        check (SubtractOpenings (base, {}, result) && almostEqual (area (), 12), "uncut wall");
        check (SubtractOpenings (base, {{1, 1, 3, 2}}, result) && almostEqual (area (), 10), "interior opening hole");
        check (SubtractOpenings (base, {{1, 0, 3, 2}}, result) && almostEqual (area (), 8), "boundary opening notch");
        check (SubtractOpenings (base, {{1, 0, 3, 2}, {2, 1, 4, 3}}, result) && almostEqual (area (), 5),
               "overlapping openings union");
        check (SubtractOpenings (base, {{-1, -1, 5, 4}}, result) && result.empty (), "full cut leaves no wall");
        check (SubtractOpenings (base, {{4, 0, 5, 3}}, result) && almostEqual (area (), 12), "touching opening no cut");
        check (!SubtractOpenings (base, {{1, 1, 1, 2}}, result) && result.empty (), "invalid opening fails closed");
        check (SlabRole (-.5545, 0, 0) == SlabCandidateRole::Floor,
               "floor slab crossing nominal floor survives wall bottom minus .1");
        check (SlabRole (-.9, -.5545, 0) == SlabCandidateRole::Floor, "below floor support retained");
        check (SlabRole (3.3, 3.6, 0) == SlabCandidateRole::Ceiling, "nearest ceiling below nominal zone top retained");
        check (SlabRole (6.9, 7.2, 0) == SlabCandidateRole::Ceiling, "ceiling above nominal zone top retained");
        check (SlabRole (0, .3, 0) == SlabCandidateRole::Floor, "floor wins boundary equality");
        check (SlabRole (1, 0, 0) == SlabCandidateRole::None, "invalid slab interval rejected");
        check (SlabRole (0, 1, std::numeric_limits<double>::infinity ()) == SlabCandidateRole::None,
               "nonfinite floor rejected");
        double bottom, top;
        check (SlabLevels (10, 0.2, 0.3, bottom, top) && almostEqual (bottom, 9.9) && almostEqual (top, 10.2),
               "slab core reference levels");
        check (SlabLevels (-3, 0, 0.3, bottom, top) && almostEqual (bottom, -3.3) && almostEqual (top, -3),
               "slab top reference");
        check (!SlabLevels (0, 0, -1, bottom, top), "invalid slab thickness");
        check (!SlabLevels (std::numeric_limits<double>::infinity (), 0, 1, bottom, top), "nonfinite slab");
        check (OrdinaryWall (false, true, true, true, false, false), "ordinary wall gate");
        check (!OrdinaryWall (true, true, true, true, false, false), "profile native fallback gate");
        check (!OrdinaryWall (false, false, true, true, false, false), "curved native fallback gate");
        check (!OrdinaryWall (false, true, false, true, false, false), "slanted native fallback gate");
        check (OrdinaryWall (false, true, true, true, false, true), "linked wall uses resolved element height");
    }
} // namespace Roombook
