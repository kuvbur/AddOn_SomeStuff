//------------ kuvbur 2022 ------------
#ifndef SILLYGEOMETRY_HPP
#define SILLYGEOMETRY_HPP
#include "ACAPinc.h"
#ifdef AC_25
#include	"APICommon25.h"
#endif // AC_25
#ifdef AC_26
#include	"APICommon26.h"
#endif // AC_26
#include	"basicgeometry.h"
#include	"Polygon3D.hpp"
#include	"Intersection3D.hpp"

Sector3D GetSector3DFromCoord(const Coord& coord1, const Coord& coord2);

Sector3D GetSector3DFromCoord(const API_Coord3D& coord1, const API_Coord3D& coord2);

Sector3D GetSector3DFromCoord(double x1, double y1, double x2, double y2);

Point3D GetPoint3DFromCoord(const Coord& coord);

Point3D GetPoint3DFromCoord(const API_Coord3D& coord);

Point3D GetPoint3DFromCoord(double x, double y);

double GetAngXYSector3D(const Sector3D& sector);

void TransformPoint3D(const Sector3D& sector, Point3D& point);

bool IntersectPolygon3DSector3D(const Geometry::Polygon3D& poly, const Sector3D& sector, Sector3D& rezult);

bool IsPoint3DInPolygon3D(const Geometry::Polygon3D& poly, const Point3D& point);

bool IntersectPolygon3D(const Geometry::Polygon3D& poly1, const Geometry::Polygon3D& poly2, Geometry::Polygon3D& rezult);

Geometry::Polygon3D CreatePolygon3D(GS::PagedArray<Point3D>& points);

#endif