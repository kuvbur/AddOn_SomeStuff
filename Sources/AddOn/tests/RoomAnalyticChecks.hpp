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
        const Rect bottomWall{0, 5, 10, 8};
        std::vector<std::pair<double, double>> bottomSegments;
        const auto bottomMatches = [&] (const std::vector<Rect> &openings,
                                        const std::vector<std::pair<double, double>> &expected) {
            if (!CutWallBottom (bottomWall, openings, 1e-6, bottomSegments) ||
                bottomSegments.size () != expected.size ())
                return false;
            for (std::size_t i = 0; i < expected.size (); ++i)
                if (!almostEqual (bottomSegments[i].first, expected[i].first) ||
                    !almostEqual (bottomSegments[i].second, expected[i].second))
                    return false;
            return true;
        };
        check (bottomMatches ({}, {{0, 10}}), "skirting uncut bottom");
        check (bottomMatches ({{1, 5, 3, 7}}, {{0, 1}, {3, 10}}), "skirting door cuts bottom");
        check (bottomMatches ({{1, 6, 3, 7}}, {{0, 10}}), "skirting window above bottom retained");
        check (bottomMatches ({{1, 3, 3, 5}}, {{0, 10}}), "skirting opening ending at bottom retained");
        check (bottomMatches ({{1, 4, 3, 6}}, {{0, 1}, {3, 10}}), "skirting opening crossing bottom cuts");
        check (bottomMatches ({{4, 5, 7, 7}, {1, 5, 5, 7}}, {{0, 1}, {7, 10}}),
               "skirting overlapping unsorted openings union");
        check (bottomMatches ({{1, 5, 8, 7}, {2, 5, 3, 7}}, {{0, 1}, {8, 10}}), "skirting nested openings union");
        check (bottomMatches ({{1, 5, 3, 7}, {3, 5, 5, 7}}, {{0, 1}, {5, 10}}),
               "skirting touching openings leave no duplicate segment");
        check (bottomMatches ({{1, 5, 2, 7}, {5, 5, 6, 7}}, {{0, 1}, {2, 5}, {6, 10}}),
               "skirting separated openings preserve intermediate segment");
        check (bottomMatches ({{-2, 5, 3, 7}}, {{3, 10}}), "skirting opening clipped at wall beginning");
        check (bottomMatches ({{7, 5, 12, 7}}, {{0, 7}}), "skirting opening clipped at wall end");
        check (bottomMatches ({{-1, 5, 11, 7}}, {}), "skirting full opening leaves empty bottom");
        check (bottomMatches ({{11, 5, 12, 7}, {-2, 5, -1, 7}}, {{0, 10}}), "skirting outside openings do not cut");
        check (bottomMatches ({{1, 5 + 5e-7, 3, 7}}, {{0, 1}, {3, 10}}),
               "skirting opening bottom uses existing distance tolerance");
        check (bottomMatches ({{1, 5 + 2e-6, 3, 7}}, {{0, 10}}), "skirting real opening gap above bottom retained");
        check (!CutWallBottom (bottomWall, {{1, 5, 1, 7}}, 1e-6, bottomSegments) && bottomSegments.empty (),
               "skirting invalid opening fails without partial bottom");
        check (!CutWallBottom ({0, 5, 0, 8}, {}, 1e-6, bottomSegments) && bottomSegments.empty (),
               "skirting zero wall rejected");
        check (
            !CutWallBottom (bottomWall, {{1, 5, std::numeric_limits<double>::infinity (), 7}}, 1e-6, bottomSegments) &&
                bottomSegments.empty (),
            "skirting nonfinite opening rejected");
        check (bottomMatches ({}, {{0, 10}}), "skirting repeat replaces previous bottom segments");
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
