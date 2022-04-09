#include "geom_funcs.hpp"

#include "plane.hpp"
#include "point3f.hpp"

namespace geom
{
Point3F project_on(const Point3F &point, const Plane &plane)
{
  // inspired by https://stackoverflow.com/a/9605695/1269661
  const auto v = point - plane.origin;
  const auto dist= v.dot(plane.normal);
  return point - plane.normal * dist;
}
}
