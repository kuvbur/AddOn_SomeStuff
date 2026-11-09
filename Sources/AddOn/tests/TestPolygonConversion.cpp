#ifdef TESTING
    #include <cmath>

    #include "ACAPinc.h"

    #include "api_headers/APIEnvir.h"

    #include "CommonFunction.hpp"
    #include "Polygon2D.hpp"
    #include "Polygon2DData.h"
    #include "tests/TestFunc.hpp"
    #include "tests/TestKit.hpp"

namespace TestFunc {
    namespace {
        struct ConversionArc {
            UInt32 beg;
            UInt32 end;
            double angle;
        };

        // Независимые API-индексы: замыкающая вершина каждого контура дублируется.
        void CheckConversionFixture (
            const char *name, const ConversionArc *expected, USize arcCount, bool hole, API_ElementMemo &memo) {
            const GS::Array<Point2D> points =
                hole ? GS::Array<Point2D>{{0, 0}, {10, 0}, {10, 10}, {0, 10}, {3, 3}, {3, 5}, {5, 5}, {5, 3}}
                     : GS::Array<Point2D>{{0, 0}, {10, 0}, {10, 10}, {0, 10}};
            GS::Array<PolyArcRec> inputArcs;
            for (UIndex i = 0; i < arcCount; ++i) {
                PolyArcRec arc = {};
                // Вход Create не содержит дублированных замыкающих вершин.
                arc.begIndex = expected[i].beg > 5 ? expected[i].beg - 2 : expected[i].beg - 1;
                arc.endIndex =
                    expected[i].end == 5
                        ? 0
                        : (expected[i].end == 10 ? 4
                                                 : (expected[i].end > 5 ? expected[i].end - 2 : expected[i].end - 1));
                arc.arcAngle = expected[i].angle;
                inputArcs.Push (arc);
            }
            const auto polygons = Geometry::Polygon2D::Create (points,
                                                               inputArcs,
                                                               hole ? GS::Array<UIndex>{3, 7} : GS::Array<UIndex>{3},
                                                               Geometry::PolyCreateFlags_IdxRefInContourEndFromZero |
                                                                   Geometry::PolyCreateFlags_IdxRefInArcFromZero);
            DBtest (polygons.GetSize () == 1, GS::UniString::Printf ("%s fixture regular", name));
            if (polygons.GetSize () != 1)
                return;
            const Geometry::Polygon2D &source = polygons[0];
            const double originalArea = source.CalcArea ();
            Geometry::Polygon2DData before;
            Geometry::InitPolygon2DData (&before);
            Geometry::ConvertPolygon2DToPolygon2DData (before, source);
            API_Polygon api = {};
            const GSErrCode err = ConvertPolygon2DToAPIPolygon (source, api, memo);
            DBtest (err == NoError, GS::UniString::Printf ("%s conversion succeeds", name));
            const Int32 count = hole ? 10 : 5;
            DBtest (api.nCoords == count && api.nSubPolys == (hole ? 2 : 1) && api.nArcs == arcCount,
                    GS::UniString::Printf ("%s counts", name));
            const bool allocated = memo.coords != nullptr && memo.pends != nullptr && memo.vertexIDs != nullptr &&
                                   (arcCount == 0 || memo.parcs != nullptr);
            DBtest (allocated, GS::UniString::Printf ("%s handles", name));
            bool safe = err == NoError && allocated && api.nCoords == count && api.nArcs == arcCount;
            if (safe) {
                const API_Coord literal[] = {
                    {0, 0}, {0, 0}, {10, 0}, {10, 10}, {0, 10}, {0, 0}, {3, 3}, {3, 5}, {5, 5}, {5, 3}, {3, 3}};
                bool coords = true;
                bool ids = (*memo.vertexIDs)[0] == count;
                for (Int32 i = 1; i <= count; ++i) {
                    coords = coords && std::abs ((*memo.coords)[i].x - literal[i].x) < 1e-10 &&
                             std::abs ((*memo.coords)[i].y - literal[i].y) < 1e-10;
                    ids = ids && (*memo.vertexIDs)[i] == static_cast<UInt32> (i);
                }
                DBtest (coords, GS::UniString::Printf ("%s literal coords including closing nodes", name));
                DBtest (ids, GS::UniString::Printf ("%s vertex IDs", name));
                const bool ends = (*memo.pends)[0] == 0 && (*memo.pends)[1] == 5 && (!hole || (*memo.pends)[2] == 10);
                DBtest (ends, GS::UniString::Printf ("%s literal pends", name));
                safe = safe && coords && ends;
                for (UIndex i = 0; i < arcCount; ++i) {
                    const API_PolyArc &arc = (*memo.parcs)[i];
                    const bool matches = arc.begIndex == expected[i].beg && arc.endIndex == expected[i].end &&
                                         std::abs (arc.arcAngle - expected[i].angle) < 1e-10;
                    DBtest (matches, GS::UniString::Printf ("%s literal arc %u", name, static_cast<unsigned> (i)));
                    safe = safe && matches;
                }
            }
            // Повреждённые записи нельзя передавать геометрическому ядру даже в RED.
            DBtest (safe, GS::UniString::Printf ("%s roundtrip preflight", name));
            if (safe) {
                Geometry::Polygon2D roundtrip;
                const GSErrCode back = ConstructPolygon2DFromElementMemo (memo, roundtrip);
                DBtest (back == NoError, GS::UniString::Printf ("%s actual reverse converter", name));
                if (back == NoError) {
                    DBtest (std::abs (roundtrip.CalcArea () - originalArea) < 1e-8,
                            GS::UniString::Printf ("%s roundtrip area", name));
                    Geometry::Polygon2DData data;
                    Geometry::InitPolygon2DData (&data);
                    Geometry::ConvertPolygon2DToPolygon2DData (data, roundtrip);
                    bool arcs = data.nArcs == arcCount;
                    for (UIndex i = 0; arcs && i < arcCount; ++i)
                        arcs = (*data.arcs)[i + 1].begIndex == expected[i].beg &&
                               (*data.arcs)[i + 1].endIndex == expected[i].end &&
                               std::abs ((*data.arcs)[i + 1].arcAngle - expected[i].angle) < 1e-10;
                    DBtest (arcs, GS::UniString::Printf ("%s roundtrip literal arcs", name));
                    Geometry::FreePolygon2DData (&data);
                }
            }
            Geometry::Polygon2DData after;
            Geometry::InitPolygon2DData (&after);
            Geometry::ConvertPolygon2DToPolygon2DData (after, source);
            bool unchanged = before.nVertices == after.nVertices && before.nArcs == after.nArcs &&
                             before.nContours == after.nContours &&
                             std::abs (source.CalcArea () - originalArea) < 1e-10;
            for (UIndex i = 1; unchanged && i <= before.nVertices; ++i)
                unchanged = (*before.vertices)[i].x == (*after.vertices)[i].x &&
                            (*before.vertices)[i].y == (*after.vertices)[i].y;
            for (UIndex i = 0; unchanged && i <= before.nContours; ++i)
                unchanged = (*before.contourEnds)[i] == (*after.contourEnds)[i];
            for (UIndex i = 1; unchanged && i <= before.nArcs; ++i)
                unchanged = (*before.arcs)[i].begIndex == (*after.arcs)[i].begIndex &&
                            (*before.arcs)[i].endIndex == (*after.arcs)[i].endIndex &&
                            (*before.arcs)[i].arcAngle == (*after.arcs)[i].arcAngle;
            DBtest (unchanged, GS::UniString::Printf ("%s original source unchanged", name));
            Geometry::FreePolygon2DData (&before);
            Geometry::FreePolygon2DData (&after);
        }
    } // namespace

    void TestPolygonConversion () {
        const ConversionArc one[] = {{1, 2, .4}};
        const ConversionArc multiple[] = {{1, 2, .4}, {2, 3, -.3}};
        const ConversionArc closing[] = {{4, 5, .25}};
        const ConversionArc hole[] = {{1, 2, .4}, {6, 7, -.3}, {9, 10, .2}};
        API_ElementMemo memo = {};
        // Один memo повторно используется после дуг, отверстия и прямого контура.
        for (unsigned repeat = 0; repeat < 2; ++repeat) {
            CheckConversionFixture ("straight", nullptr, 0, false, memo);
            CheckConversionFixture ("one positive", one, 1, false, memo);
            CheckConversionFixture ("multiple signed", multiple, 2, false, memo);
            CheckConversionFixture ("closing edge", closing, 1, false, memo);
            CheckConversionFixture ("hole arcs", hole, 3, true, memo);
        }
        BMhKill (reinterpret_cast<GSHandle *> (&memo.coords));
        BMhKill (reinterpret_cast<GSHandle *> (&memo.pends));
        BMhKill (reinterpret_cast<GSHandle *> (&memo.vertexIDs));
        BMhKill (reinterpret_cast<GSHandle *> (&memo.parcs));
    }
} // namespace TestFunc
#endif
