#pragma once

#include "object.hpp"

class Point3F;
class Point2;

namespace ftxui {
class Color;
}

class tree_t : public object_t
{
public:
  explicit tree_t(const Point3F &position);
  static void do_draw(ftxui::Canvas &canvas, Point2 point, ftxui::Color color);
  void draw_foreground(ftxui::Canvas &canvas, const Point2 &point) override;
  void draw_background(ftxui::Canvas &canvas, const Point2 &point) override;
};
