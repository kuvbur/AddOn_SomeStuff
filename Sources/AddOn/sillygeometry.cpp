//------------ kuvbur 2022 ------------
#include	"APIEnvir.h"
#include	"ACAPinc.h"
#include	"sillygeometry.hpp"

Sector3D GetSector3DFromCoord(const Coord& coord1, const Coord& coord2) {
	Point3D point1 = GetPoint3DFromCoord(coord1);
	Point3D point2 = GetPoint3DFromCoord(coord2);
	Sector3D sector(point1, point2);
	return sector;
}

Sector3D GetSector3DFromCoord(const API_Coord3D& coord1, const API_Coord3D& coord2) {
	Point3D point1 = GetPoint3DFromCoord(coord1);
	Point3D point2 = GetPoint3DFromCoord(coord2);
	Sector3D sector(point1, point2);
	return sector;
}

Sector3D GetSector3DFromCoord(double x1, double y1, double x2, double y2) {
	Point3D point1 = GetPoint3DFromCoord(x1, y1);
	Point3D point2 = GetPoint3DFromCoord(x2, y2);
	Sector3D sector(point1, point2);
	return sector;
}

Point3D GetPoint3DFromCoord(const Coord& coord) {
	double x = coord.GetX();
	double y = coord.GetY();
	double z = 0;
	Point3D point(x, y, z);
	return point;
}

Point3D GetPoint3DFromCoord(const API_Coord3D& coord) {
	double x = coord.x;
	double y = coord.y;
	double z = coord.z;
	Point3D point(x, y, z);
	return point;
}

Point3D GetPoint3DFromCoord(double x, double y) {
	double z = 0;
	Point3D point(x, y, z);
	return point;
}

double GetAngXYSector3D(const Sector3D& sector) {
	double angz = 0;
	Point3D p1 = sector.BegPoint();
	Point3D p2 = sector.EndPoint();
	double dx = p2.x - p1.x;
	double dy = p2.y - p1.y;
	if (dx > 0 && dy >= 0) {
		angz = atan(dy / dx);
	}
	if (dx > 0 && dy < 0) {
		angz = atan(dy / dx) + 2 * PI;
	}
	if (dx < 0) {
		angz = atan(dy / dx) + PI;
	}
	if (dx == 0 && dy > 0) {
		angz = PI / 2;
	}
	if (dx == 0 && dy < 0) {
		angz = 3 * PI / 2;
	}
	if (dx == 0 && dy < 0) {
		angz = 0;
	}
	return angz;
}
void TransformPoint3D(const Sector3D& sector, Point3D& point) {
	Point3D basepoint = sector.BegPoint();
	double ang = GetAngXYSector3D(sector);
	double x = point.x;
	double y = point.y;
	double z = point.z;
	x = x * cos(ang) - y * sin(ang);
	y = x * sin(ang) + y * cos(ang);
	x = x + basepoint.x;
	y = y + basepoint.y;
	z = z + basepoint.z;
	point.Set(x, y, z);
}

bool IntersectPolygon3DSector3D(const Geometry::Polygon3D& poly, const Sector3D& sector, Sector3D& rezult) {
	Int32 nSector = poly.GetCoord3DCount();
	bool ip1 = IsPoint3DInPolygon3D(poly, sector.c1);
	bool ip2 = IsPoint3DInPolygon3D(poly, sector.c2);
	if (ip1 && ip2) { //Отрезок внутри многоугольника
		rezult = sector;
		return true;
	}
	if (!ip1 && !ip2) return false; //Отрезок вне многоугольника
	for (Int32 i = 0; i < nSector; i++) {
		Sector3D s;
		Point3D ccc;
		poly.GetSector3D(i, s);
		if (XLines3D(&s, &sector, &ccc)) {
			if (ip1) rezult.c1 = sector.c1;
			if (ip2) rezult.c1 = sector.c2;
			rezult.c2 = ccc;
			return true;
		}
	}
	return false;
}

bool IsPoint3DInPolygon3D(const Geometry::Polygon3D& poly, const Point3D& point) {
	int incount = 0; // Счётчик пересечений

	// Формуируем луч с началом в точке и направлением на середину полигона
	Box3D bx = poly.GetBounds();
	double x = 0; if (!(bx.xMax == point.x && bx.xMin == point.x)) x = ((bx.xMax - bx.xMin) / 2) - point.x;
	double y = 0; if (!(bx.yMax == point.y && bx.yMin == point.y)) y = ((bx.yMax - bx.yMin) / 2) - point.y;
	double z = 0; if (!(bx.zMax == point.z && bx.zMin == point.z)) z = ((bx.zMax - bx.zMin) / 2) - point.z;
	UnitVector_3D direction = Geometry::UnitVector_3D::Create(x, y, z);
	Geometry::Ray3D r(point, direction);
	for (Int32 i = 0; i < poly.GetCoord3DCount(); i++) {
		Sector3D s;
		poly.GetSector3D(i, s);
		if (point == s.c1 || point == s.c2) return true; // Точка совпадает с углами
		if (s.ContainsPoint(point)) return true; // Точка лежит на ребре
		auto intersect = Intersect(r, s);
		if (intersect.Is<Point3D>()) {
			incount = incount + 1;
		}
		else {
			if (intersect.Is<Sector3D>()) {
				Sector3D pp = intersect.Get<Sector3D>();
				incount = incount + 1;
			}
		}
	}
	if (incount == 0) {
		return false;
	} // Нет пересечений
	return incount % 2 != 0;
}

bool IntersectPolygon3D(const Geometry::Polygon3D& poly1, const Geometry::Polygon3D& poly2, Geometry::Polygon3D& rezult) {

	//Box3DInBox3D
	GS::PagedArray<Point3D> projectedpoints;
	Geometry::Plane plane1 = poly1.GetPlane();
	GS::PagedArray<Point3D> points;

	// Последовательно проецируем отрезки из poly2 на плоскость poly1
	// Сразу проверяем на пересечение границ
	for (Int32 i = 0; i < poly2.GetCoord3DCount(); i++) {
		Sector3D s2;
		poly2.GetSector3D(i, s2);

		// Проецируем отрезон на полигон
		Point3D p1 = plane1.ProjectToPlane(s2.c1);
		Point3D p2 = plane1.ProjectToPlane(s2.c2);
		Sector3D s(p1, p2);
		Sector3D side;
		if (IntersectPolygon3DSector3D(poly1, s, side)) {
			if (!points.Contains(side.c1)) points.Push(side.c1);
			if (!points.Contains(side.c2)) points.Push(side.c2);
		}
	}
	if (points.GetSize() > 1) {
		rezult = CreatePolygon3D(points);
		return true;
	}
	return false;
}

Geometry::Polygon3D CreatePolygon3D(GS::PagedArray<Point3D>& points) {
	Geometry::Polygon3D poly;
	Geometry::Plane plane;
	Geometry::CreatePlane(points, plane);
	poly.SetPointArray(points);
	poly.setPlane(plane);
	return poly;
}