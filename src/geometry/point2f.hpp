#pragma once

class Point2F
{
public:
  Point2F operator+(const Point2F &other) const { return { x + other.x, y + other.y}; }
  Point2F operator-(const Point2F &other) const { return { x - other.x, y - other.y }; }
  [[nodiscard]] double length() const { return sqrt(sq_length()); }
  [[nodiscard]] double sq_length() const { return x * x + y * y; }

public:
  double x;
  double y;
};
