#pragma once

#include <cmath>

class Point3F
{
public:
  double x;
  double y;
  double z;

public:
  Point3F operator-(const Point3F &other) const
  {
    return { x - other.x, y - other.y, z - other.z };
  }
  Point3F operator+(const Point3F &other) const
  {
    return { x + other.x, y + other.y, z + other.z };
  }
  Point3F &operator+=(const Point3F &other)
  {
    *this = *this + other;
    return *this;
  }

  [[nodiscard]] double length() const
  {
    return std::sqrt(sq_length());
  }

  [[nodiscard]] double sq_length() const
  {
    return x * x + y * y + z * z;
  }

  [[nodiscard]] Point3F normalized() const
  {
    return *this / length();
  }
  Point3F operator-() const
  {
    return {-x, -y, -z};
  }
  Point3F operator/(double v) const
  {
    return{x/v, y/v, z/v};
  }
  Point3F operator*(double v) const { return { x * v, y * v, z * v }; }

  [[nodiscard]] Point3F cross(const Point3F &other) const
  {
    return {y *other.z - z *other.y, other.x * z - other.z * x, x * other.y - y * other.x};
  }

  [[nodiscard]] double dot(const Point3F &other) const
  {
    return x *other.x + y *other.y + z * other.z;
  }
};
