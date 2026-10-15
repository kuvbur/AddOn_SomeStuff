#include "ACAPinc.h"

#include "roombook/RoomSurfaces.hpp"

#include "Helpers.hpp"

#ifdef ServerMainVers_2400
    #include <cmath>
    #include <limits>
    #include <map>
    #include <utility>

namespace Roombook {

    bool TransformRoomSurface (const API_Tranmat &matrix, RoomSurface &surface) {
        if (surface.contours.IsEmpty () || surface.contours[0].hole || surface.contours[0].vertices.GetSize () < 3)
            return false;
        for (double value : matrix.tmx) {
            if (!std::isfinite (value))
                return false;
        }
        const double *m = matrix.tmx;
        const double c00 = m[5] * m[10] - m[6] * m[9];
        const double c01 = m[6] * m[8] - m[4] * m[10];
        const double c02 = m[4] * m[9] - m[5] * m[8];
        const double determinant = m[0] * c00 + m[1] * c01 + m[2] * c02;
        if (!std::isfinite (determinant) || determinant == 0)
            return false;
        const API_Coord3D &n = surface.normalLocal;
        // Нормаль — обратная транспонированная матрица, не преобразование точки/обычного вектора.
        API_Coord3D normal = {(c00 * n.x + c01 * n.y + c02 * n.z) / determinant,
                              ((m[2] * m[9] - m[1] * m[10]) * n.x + (m[0] * m[10] - m[2] * m[8]) * n.y +
                               (m[1] * m[8] - m[0] * m[9]) * n.z) /
                                  determinant,
                              ((m[1] * m[6] - m[2] * m[5]) * n.x + (m[2] * m[4] - m[0] * m[6]) * n.y +
                               (m[0] * m[5] - m[1] * m[4]) * n.z) /
                                  determinant};
        const double length = std::hypot (std::hypot (normal.x, normal.y), normal.z);
        if (!std::isfinite (length) || length == 0)
            return false;
        normal.x /= length;
        normal.y /= length;
        normal.z /= length;
        for (RoomSurfaceContour &contour : surface.contours) {
            if (contour.vertices.GetSize () < 3)
                return false;
            for (RoomSurfaceVertex &vertex : contour.vertices) {
                const API_Coord3D &point = vertex.local;
                vertex.world = {m[0] * point.x + m[1] * point.y + m[2] * point.z + m[3],
                                m[4] * point.x + m[5] * point.y + m[6] * point.z + m[7],
                                m[8] * point.x + m[9] * point.y + m[10] * point.z + m[11]};
                if (!std::isfinite (vertex.world.x) || !std::isfinite (vertex.world.y) ||
                    !std::isfinite (vertex.world.z))
                    return false;
            }
        }
        surface.normalWorld = normal;
        const API_Coord3D &point = surface.contours[0].vertices[0].world;
        surface.planeOffset = -(normal.x * point.x + normal.y * point.y + normal.z * point.z);
        return std::isfinite (surface.planeOffset);
    }

    namespace {

        void AddSurfaceError (
            RoomSurfaceSnapshot &snapshot, const char *stage, GSErrCode code, Int32 body, Int32 polygon, Int32 index) {
            RoomSurfaceReadError error;
            error.stage = stage;
            error.code = code;
            error.body = body;
            error.polygon = polygon;
            error.index = index;
            snapshot.errors.Push (error);
        }

        struct ComponentEntry {
            API_Component3D component = {};
            GSErrCode error = NoError;
        };

        struct BodyComponentCache {
            std::map<std::pair<API_3DTypeID, Int32>, ComponentEntry> entries;
            RoomSurfaceReadCounters *counters = nullptr;
        };

        bool ReadSurfaceComponent (BodyComponentCache &cache,
                                   API_3DTypeID type,
                                   Int32 index,
                                   Int32 count,
                                   API_Component3D &component,
                                   RoomSurfaceSnapshot &snapshot,
                                   Int32 body,
                                   Int32 polygon,
                                   const char *stage) {
            component = {};
            component.header.typeID = type;
            component.header.index = index;
            GSErrCode err = APIERR_BADINDEX;
            if (index >= 1 && index <= count) {
                const auto key = std::make_pair (type, index);
                const auto found = cache.entries.find (key);
                if (found != cache.entries.end ()) {
                    component = found->second.component;
                    err = found->second.error;
                    if (cache.counters != nullptr)
                        ++cache.counters->componentHits;
                } else {
                    err = ACAPI_3D_GetComponent (&component);
                    if (cache.counters != nullptr)
                        ++cache.counters->componentReads;
                    cache.entries.emplace (key, ComponentEntry{component, err});
                }
            }
            // Ошибка повторяется с исходным контекстом каждого полигона, даже при попадании в кэш.
            if (err != NoError)
                AddSurfaceError (snapshot, stage, err, body, polygon, index);
            return err == NoError;
        }

        bool ReadSurfacePolygon (BodyComponentCache &cache,
                                 const API_BodyType &body,
                                 Int32 bodyIndex,
                                 Int32 polygonIndex,
                                 RoomSurface &out,
                                 RoomSurfaceSnapshot &snapshot) {
            API_Component3D component = {};
            if (!ReadSurfaceComponent (cache,
                                       API_PgonID,
                                       polygonIndex,
                                       body.nPgon,
                                       component,
                                       snapshot,
                                       bodyIndex,
                                       polygonIndex,
                                       "polygon"))
                return false;
            const API_PgonType polygon = component.pgon;
            out.bodyIndex = bodyIndex;
            out.polygonIndex = polygonIndex;
            out.modelMaterialIndex = polygon.iumat;
            out.flags = polygon.status;
            out.signedNormalIndex = polygon.ivect;
            if (polygon.ivect == std::numeric_limits<Int32>::min ()) {
                AddSurfaceError (snapshot, "normal", APIERR_BADINDEX, bodyIndex, polygonIndex, polygon.ivect);
                return false;
            }
            if (!ReadSurfaceComponent (cache,
                                       API_VectID,
                                       std::abs (polygon.ivect),
                                       body.nVect,
                                       component,
                                       snapshot,
                                       bodyIndex,
                                       polygonIndex,
                                       "normal"))
                return false;
            const double sign = polygon.ivect < 0 ? -1.0 : 1.0;
            out.normalLocal = {component.vect.x * sign, component.vect.y * sign, component.vect.z * sign};
            if (polygon.fpedg < 1 || polygon.lpedg > body.nPedg || polygon.fpedg > polygon.lpedg) {
                AddSurfaceError (snapshot, "pedgRange", APIERR_BADINDEX, bodyIndex, polygonIndex, polygon.fpedg);
                return false;
            }
            RoomSurfaceContour contour;
            Int32 firstVertex = 0;
            Int32 previousEnd = 0;
            const auto finishContour = [&] () -> bool {
                if (contour.vertices.GetSize () < 3 || firstVertex != previousEnd) {
                    AddSurfaceError (snapshot, "contourClosure", APIERR_BADPARS, bodyIndex, polygonIndex, previousEnd);
                    return false;
                }
                contour.hole = !out.contours.IsEmpty ();
                out.contours.Push (contour);
                contour.vertices.Clear ();
                firstVertex = previousEnd = 0;
                return true;
            };
            // Индексы примитивов локальны выбранному телу; нулевой PEDG разделяет контуры отверстий.
            for (Int32 pe = polygon.fpedg; pe <= polygon.lpedg; ++pe) {
                if (!ReadSurfaceComponent (
                        cache, API_PedgID, pe, body.nPedg, component, snapshot, bodyIndex, polygonIndex, "pedg"))
                    return false;
                const Int32 signedEdge = component.pedg.pedg;
                if (signedEdge == 0) {
                    if (!finishContour ())
                        return false;
                    continue;
                }
                if (signedEdge == std::numeric_limits<Int32>::min ()) {
                    AddSurfaceError (snapshot, "edge", APIERR_BADINDEX, bodyIndex, polygonIndex, signedEdge);
                    return false;
                }
                if (!ReadSurfaceComponent (cache,
                                           API_EdgeID,
                                           std::abs (signedEdge),
                                           body.nEdge,
                                           component,
                                           snapshot,
                                           bodyIndex,
                                           polygonIndex,
                                           "edge"))
                    return false;
                const Int32 start = signedEdge > 0 ? component.edge.vert1 : component.edge.vert2;
                const Int32 end = signedEdge > 0 ? component.edge.vert2 : component.edge.vert1;
                if (end < 1 || end > body.nVert || (!contour.vertices.IsEmpty () && start != previousEnd)) {
                    AddSurfaceError (snapshot, "edgeContinuity", APIERR_BADINDEX, bodyIndex, polygonIndex, start);
                    return false;
                }
                if (contour.vertices.IsEmpty ())
                    firstVertex = start;
                previousEnd = end;
                if (!ReadSurfaceComponent (
                        cache, API_VertID, start, body.nVert, component, snapshot, bodyIndex, polygonIndex, "vertex"))
                    return false;
                RoomSurfaceVertex vertex;
                vertex.local = {component.vert.x, component.vert.y, component.vert.z};
                vertex.index = start;
                vertex.signedEdgeIndex = signedEdge;
                contour.vertices.Push (vertex);
            }
            if (!finishContour ())
                return false;
            if (!TransformRoomSurface (body.tranmat, out)) {
                AddSurfaceError (snapshot, "transform", APIERR_BADPARS, bodyIndex, polygonIndex, 0);
                return false;
            }
            return true;
        }
    } // namespace

    RoomSurfaceSnapshot ReadRoomSurfaces (const API_Elem_Head &head, RoomSurfaceCache *cache) {
        if (cache != nullptr) {
            if (const auto *existing = cache->snapshots.GetPtr (head.guid)) {
                ++cache->counters.snapshotHits;
                return *existing;
            }
            ++cache->counters.snapshotReads;
    #ifdef ServerMainVers_2600
            const auto type = head.type.typeID;
    #else
            const auto type = head.typeID;
    #endif
            if (type == API_WallID)
                ++cache->counters.wallSnapshotReads;
            if (type == API_SlabID)
                ++cache->counters.slabSnapshotReads;
        }
        RoomSurfaceSnapshot snapshot;
        snapshot.guid = head.guid;
        API_ElemInfo3D info = {};
        const GSErrCode err = ACAPI_Element_Get3DInfo (head, &info);
        if (err != NoError) {
            AddSurfaceError (snapshot, "get3DInfo", err, 0, 0, 0);
        } else {
            snapshot.firstBody = info.fbody;
            snapshot.lastBody = info.lbody;
            for (Int32 bi = info.fbody; bi > 0 && bi <= info.lbody; ++bi) {
                API_Component3D component = {};
                component.header.typeID = API_BodyID;
                component.header.index = bi;
                const GSErrCode bodyErr = ACAPI_3D_GetComponent (&component);
                if (cache != nullptr)
                    ++cache->counters.componentReads;
                if (bodyErr == APIERR_DELETED) {
                    snapshot.deletedBodies.Push (bi);
                    continue;
                }
                if (bodyErr != NoError) {
                    AddSurfaceError (snapshot, "body", bodyErr, bi, 0, bi);
                    continue;
                }
                const API_BodyType body = component.body;
                RoomSurfaceBody bodyOut;
                bodyOut.index = bi;
                bodyOut.polygonCount = body.nPgon;
                bodyOut.vertexCount = body.nVert;
                bodyOut.matrix = body.tranmat;
                bodyOut.parentGuid = body.parent.guid;
                bodyOut.parentType = body.parent.typeID;
                BodyComponentCache components;
                components.counters = cache != nullptr ? &cache->counters : nullptr;
                for (Int32 pi = 1; pi <= body.nPgon; ++pi) {
                    RoomSurface surface;
                    if (ReadSurfacePolygon (components, body, bi, pi, surface, snapshot))
                        bodyOut.surfaces.Push (std::move (surface));
                }
                snapshot.bodies.Push (std::move (bodyOut));
            }
        }
        snapshot.status = !snapshot.errors.IsEmpty ()  ? RoomSurfaceStatus::ReadError
                          : snapshot.bodies.IsEmpty () ? RoomSurfaceStatus::Empty
                                                       : RoomSurfaceStatus::Extracted;
        if (cache != nullptr)
            cache->snapshots.Add (head.guid, snapshot);
        return snapshot;
    }

    RoomSurfaceZBounds ReadRoomSurfaceZBounds (const API_Elem_Head &head, RoomSurfaceCache *cache) {
        RoomSurfaceZBounds bounds;
        double bottom = std::numeric_limits<double>::infinity ();
        double top = -bottom;
        bool valid = true;
        if (cache != nullptr) {
            if (const auto *snapshot = cache->snapshots.GetPtr (head.guid)) {
                ++cache->counters.boundsHits;
                valid = snapshot->IsComplete ();
                for (const auto &body : snapshot->bodies)
                    for (const auto &face : body.surfaces)
                        for (const auto &contour : face.contours)
                            for (const auto &vertex : contour.vertices) {
                                bottom = std::min (bottom, vertex.world.z);
                                top = std::max (top, vertex.world.z);
                            }
                bounds = {valid && std::isfinite (bottom) && std::isfinite (top), bottom, top};
                return bounds;
            }
            if (const auto *existing = cache->bounds.GetPtr (head.guid)) {
                ++cache->counters.boundsHits;
                return *existing;
            }
            ++cache->counters.boundsReads;
        }
        // SDK25 не определяет систему координат bbox; безопасно читаем только вершины, без полигонов.
        API_ElemInfo3D info = {};
        valid = ACAPI_Element_Get3DInfo (head, &info) == NoError;
        for (Int32 bi = info.fbody; valid && bi > 0 && bi <= info.lbody; ++bi) {
            API_Component3D component = {};
            component.header.typeID = API_BodyID;
            component.header.index = bi;
            const GSErrCode err = ACAPI_3D_GetComponent (&component);
            if (cache != nullptr)
                ++cache->counters.componentReads;
            if (err == APIERR_DELETED)
                continue;
            if (err != NoError) {
                valid = false;
                break;
            }
            const API_BodyType body = component.body;
            const double *m = body.tranmat.tmx;
            for (double value : body.tranmat.tmx)
                valid = valid && std::isfinite (value);
            for (Int32 vi = 1; valid && vi <= body.nVert; ++vi) {
                component = {};
                component.header.typeID = API_VertID;
                component.header.index = vi;
                valid = ACAPI_3D_GetComponent (&component) == NoError;
                if (cache != nullptr)
                    ++cache->counters.componentReads;
                if (!valid)
                    break;
                const double z = m[8] * component.vert.x + m[9] * component.vert.y + m[10] * component.vert.z + m[11];
                valid = std::isfinite (z);
                bottom = std::min (bottom, z);
                top = std::max (top, z);
            }
        }
        bounds = {valid && std::isfinite (bottom) && std::isfinite (top), bottom, top};
        if (cache != nullptr)
            cache->bounds.Add (head.guid, bounds);
        return bounds;
    }

    ElementRead ReadElement (const API_Guid &guid, RoomSurfaceCache *cache) {
        if (cache != nullptr) {
            if (const auto *found = cache->elements.GetPtr (guid)) {
                ++cache->counters.elementHits;
                return *found;
            }
            ++cache->counters.elementReads;
        }
        ElementRead read;
        read.element.header.guid = guid;
        read.error = ACAPI_Element_Get (&read.element);
        if (cache != nullptr)
            cache->elements.Add (guid, read);
        return read;
    }

    ClassificationRead ReadClassificationItems (const API_Guid &guid, RoomSurfaceCache *cache) {
        if (cache != nullptr) {
            if (const auto *found = cache->classifications.GetPtr (guid)) {
                ++cache->counters.classificationHits;
                return *found;
            }
            ++cache->counters.classificationReads;
        }
        ClassificationRead read;
        read.error = ACAPI_Element_GetClassificationItems (guid, read.items);
        if (cache != nullptr)
            cache->classifications.Add (guid, read);
        return read;
    }

    GSErrCode ReadZoneRelations (const API_Guid &zoneGuid, RoomSurfaceCache *cache, RoomRelationValues &values) {
        if (cache != nullptr) {
            if (const auto *found = cache->relations.GetPtr (zoneGuid)) {
                ++cache->counters.relationHits;
                values = *found;
                return NoError;
            }
            ++cache->counters.relationReads;
        }
        API_RoomRelation relations = {};
        const GSErrCode error = ACAPI_Element_GetRelations (zoneGuid, API_ZombieElemID, &relations);
        if (error != NoError) {
            ACAPI_DisposeRoomRelationHdls (&relations);
            return error;
        }
        // Только value-поля: владеющие хендлы API_RoomRelation живут в niches, которые не читаем.
        values.elementsGroupedByType = relations.elementsGroupedByType;
        values.wallPart = relations.wallPart;
        values.beamPart = relations.beamPart;
        ACAPI_DisposeRoomRelationHdls (&relations);
        if (cache != nullptr)
            cache->relations.Add (zoneGuid, values);
        return NoError;
    }

    ConnectedElementsRead ReadConnectedElements (const API_Guid &guid,
                                                 API_ElemTypeID type,
                                                 API_ElemFilterFlags filter,
                                                 RoomSurfaceCache *cache,
                                                 API_ElemVariationID variation,
                                                 const API_Guid &renovationFilterGuid) {
        return ReadConnectedElementsCached (
            guid, type, filter, variation, renovationFilterGuid, cache, [&] (GS::Array<API_Guid> &items) {
                return ACAPI_Element_GetConnectedElements (guid, type, &items, filter, variation, renovationFilterGuid);
            });
    }

} // namespace Roombook
#endif
