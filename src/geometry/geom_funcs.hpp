#pragma once

class Plane;
class Point3F;

namespace geom {
Point3F project_on(const Point3F &point, const Plane &plane);
}
