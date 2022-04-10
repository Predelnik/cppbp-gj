#pragma once

#include "geometry/point3f.hpp"

class Point2;

namespace ftxui {
struct Canvas;
}

class object_t
{
public:
  explicit object_t(const Point3F &pos);
  object_t(const object_t &) = delete;
  object_t &operator=(const object_t &) = delete;
  object_t(object_t &&) = delete;
  object_t &operator=(object_t &&) = delete;
  virtual ~object_t() = default;

  virtual void draw_foreground(ftxui::Canvas &canvas, const Point2 &point) = 0;
  virtual void draw_background(ftxui::Canvas &canvas, const Point2 &point) = 0;

public:
  Point3F position{};
};
