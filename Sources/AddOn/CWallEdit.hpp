//------------ kuvbur 2022 ------------
#pragma once
#ifndef CWALL_HPP
#define	CWALL_HPP
#ifdef AC_25
#include	"APICommon25.h"
#endif // AC_25
#ifdef AC_26
#include	"APICommon26.h"
#endif // AC_26
#include	"DG.h"
#include	"Helpers.hpp"

Sector3D GetSector3DFromCoord(const API_Coord3D);

bool IntersectPolygon3DSector3D(const Geometry::Polygon3D& poly, const Sector3D& sector, Sector3D& rezult);

bool IsPoint3DInPolygon3D(const Geometry::Polygon3D& poly, const Point3D& point);

bool IntersectPolygon3D(const Geometry::Polygon3D& poly1, const Geometry::Polygon3D& poly2, Geometry::Polygon3D& rezult);

Geometry::Polygon3D CreatePolygon3D(GS::PagedArray<Point3D>& points);

void AddHoleToSelectedCWall(const SyncSettings& syncSettings);

void Do_ChangeCWallWithUndo(const GS::Array<API_Guid>& elemsGuid, const GS::Array<Geometry::Polygon3D>& elems);

void Do_ChangeCWall(const API_Guid& elemGuid, const GS::Array<Geometry::Polygon3D>& elems);

#endif