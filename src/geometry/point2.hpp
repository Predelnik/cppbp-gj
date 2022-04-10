#pragma once

#include "geometry/point2f.hpp"

class Point2
{
public:
  constexpr Point2(int x_arg, int y_arg) : x(x_arg), y(y_arg) {}
  explicit constexpr Point2(const Point2F &p) : x(static_cast<int>(p.x)), y(static_cast<int>(p.y)) {}
  Point2 operator-(const Point2 &other) const { return { x - other.x, y - other.y}; }
  Point2 operator+(const Point2 &other) const { return { x + other.x, y + other.y }; }

public:
  int x;
  int y;
};
