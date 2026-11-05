#pragma once
#include <limits>

#include "roombook/FloorCoreGeometry.hpp"

namespace TestFunc {
    // Same production kernel and assertions run both in Archicad and standalone.
    template <class Check> void FloorCoreChecks (const Check &check) {
        using namespace Roombook::FloorCoreGeometry;

        struct Layer {
            double fillThick;
            int structype;
        };

        const auto close = [] (double a, double b) { return std::fabs (a - b) < 1.e-9; };
        double offset = 0;
        std::vector<Layer> layers = {{.02, 32}, {.03, 0}, {.25, 2}, {.04, 32}};
        check (Offset (layers, layers.size (), false, 2, 32, offset) && close (offset, .05),
               "268 sum before core excludes core");
        check (Offset (layers, layers.size (), true, 2, 32, offset) && close (offset, .04),
               "268 flipped reads opposite side");
        layers = {{.25, 2}, {.04, 32}};
        check (Offset (layers, layers.size (), false, 2, 32, offset) && offset == 0, "268 core at surface valid zero");
        layers = {{.03, 0}, {.02, 32}, {.06, 32}};
        check (Offset (layers, layers.size (), false, 2, 32, offset) && close (offset, .05),
               "268 first finish far side fallback");
        check (Offset (layers, layers.size (), true, 2, 32, offset) && close (offset, .06),
               "268 flipped finish fallback");
        layers = {{.2, 0}};
        check (!Offset (layers, layers.size (), false, 2, 32, offset), "268 no layer undefined");
        layers = {{-.02, 32}, {.2, 2}};
        check (!Offset (layers, layers.size (), false, 2, 32, offset), "268 negative thickness rejected");
        layers = {{std::numeric_limits<double>::quiet_NaN (), 0}, {.2, 2}};
        check (!Offset (layers, layers.size (), false, 2, 32, offset), "268 nonfinite thickness rejected");
        check (!Offset (layers, 0, false, 2, 32, offset), "268 empty composition undefined");
        const std::vector<Edge> edge = {{{0, 0}, {4, 0}, true}};
        auto patches = Extensions (edge, {{{1, 0}, {3, 0}, .1, 1}});
        check (patches.size () == 1 && close (patches[0][0].x, 1) && close (patches[0][1].x, 3) &&
                   close (patches[0][2].y, -.1),
               "268 finite middle-only extension");
        patches = Extensions (edge, {{{0, 0}, {2, 0}, .1, 1}, {{2, 0}, {4, 0}, .2, 2}});
        check (patches.size () == 2 && close (patches[0][2].y, -.1) && close (patches[1][2].y, -.2),
               "268 different partial depths split");
        patches = Extensions (edge, {{{0, 0}, {3, 0}, .1, 1}, {{2, 0}, {4, 0}, .1, 2}});
        check (patches.size () == 2 && close (patches[0][1].x, 2) && close (patches[1][0].x, 3),
               "268 ambiguous owners keep overlap untouched");
        patches = Extensions (edge, {{{0, 0}, {4, 0}, .1, 1}, {{0, 0}, {4, 0}, .1, 1}});
        check (patches.size () == 1, "268 same-owner duplicate accepted once");
        check (Extensions (edge, {{{0, 0}, {4, 0}, .1, 1}, {{0, 0}, {4, 0}, .2, 1}}).empty (),
               "268 conflicting same-owner depth unchanged");
        check (Extensions (edge, {{{0, 0}, {4, 0}, .1, 1}, {{0, 0}, {4, 0}, 0, 2}}).empty (),
               "268 unknown competing owner blocks extension");
        check (Extensions (edge, {{{4, 0}, {0, 0}, .1, 1}}).empty (), "268 opposite wall direction excluded");
        check (Extensions (edge, {{{0, .01}, {4, .01}, .1, 1}}).empty (), "268 nearby parallel not contact");
        check (Extensions (edge, {{{4, 0}, {5, 0}, .1, 1}}).empty (), "268 endpoint-only contact ignored");
        check (Extensions (edge, {}).empty (), "268 other boundary unchanged");
        check (Extensions ({{{0, 0}, {4, 0}, false}}, {{{0, 0}, {4, 0}, .1, 1}}).empty (),
               "268 arc never replaced by chord");
        const std::vector<Edge> corner = {{{0, 0}, {4, 0}, true}, {{4, 0}, {4, 3}, true}};
        patches = Extensions (corner, {{{0, 0}, {4, 0}, .1, 1}, {{4, 0}, {4, 3}, .2, 2}});
        check (patches.size () == 3 && close (patches[2][2].x, 4.2) && close (patches[2][2].y, -.1),
               "268 unequal corner miter closes gap");

        struct CornerGapCase {
            double gap;
            bool closes;
            const char *label;
        };

        const CornerGapCase cornerGaps[] = {{0, true, "floor corner exact endpoints"},
                                            {1.e-12, true, "floor corner picometre endpoint gap"},
                                            {1.e-9, true, "floor corner nanometre endpoint gap"},
                                            {5.e-7, true, "floor corner sub-tolerance endpoint gap"},
                                            {2.e-6, false, "floor corner real endpoint gap retained"}};
        const auto closesCorner = [&] (const std::vector<Patch> &parts) {
            return parts.size () == 3 && parts.back ().size () == 4 && close (parts.back ()[2].x, 4.03) &&
                   close (parts.back ()[2].y, -.02);
        };
        for (const auto &test : cornerGaps) {
            check (closesCorner (Extensions (
                       corner, {{{0, 0}, {4 - test.gap, 0}, .02, 1}, {{4, test.gap}, {4, 3}, .03, 2}})) == test.closes,
                   test.label);
            check (closesCorner (Extensions (
                       corner, {{{0, 0}, {4 - test.gap, 0}, .02, 1}, {{4, 0}, {4, 3}, .03, 2}})) == test.closes,
                   test.label);
            check (closesCorner (Extensions (corner, {{{0, 0}, {4, 0}, .02, 1}, {{4, test.gap}, {4, 3}, .03, 2}})) ==
                       test.closes,
                   test.label);
        }
        check (
            !closesCorner (Extensions (corner, {{{4 - Tolerance / 2, 0}, {4, 0}, .02, 1}, {{4, 0}, {4, 3}, .03, 2}})),
            "floor corner tiny contact is not expanded into accepted support");
        check (!closesCorner (
                   Extensions (corner, {{{0, 0}, {4, 0}, .02, 1}, {{4, 0}, {4, 3}, .03, 2}, {{3.9, 0}, {4, 0}, 0, 3}})),
               "floor corner ambiguous terminal support stays unextended");
        const auto rotate = [] (Roombook::FloorCoreGeometry::Point point) {
            return Roombook::FloorCoreGeometry::Point{100 + .6 * point.x - .8 * point.y,
                                                      -50 + .8 * point.x + .6 * point.y};
        };
        patches = Extensions (
            {{rotate ({0, 0}), rotate ({4, 0}), true}, {rotate ({4, 0}), rotate ({4, 3}), true}},
            {{rotate ({0, 0}), rotate ({4 - 5.e-7, 0}), .02, 1}, {rotate ({4, 5.e-7}), rotate ({4, 3}), .03, 2}});
        const auto expectedCorner = rotate ({4.03, -.02});
        check (patches.size () == 3 && close (patches.back ()[2].x, expectedCorner.x) &&
                   close (patches.back ()[2].y, expectedCorner.y),
               "floor corner endpoint tolerance is invariant under rotation and translation");
        patches = Extensions ({{{1, 1}, {1, 3}, true}}, {{{1, 1}, {1, 3}, .1, 1}});
        check (patches.size () == 1 && close (patches[0][2].x, 1.1), "268 clockwise hole extends into wall");
        check (Extensions ({{{0, -.1}, {4, -.1}, true}}, {{{0, 0}, {4, 0}, .1, 1}}).empty (),
               "268 repeated shifted edge not extended again");
    }
} // namespace TestFunc
