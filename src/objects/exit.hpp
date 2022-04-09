#pragma once

#include "object.hpp"

namespace ftxui {
class Color;
}

class exit_t : public object_t
{
public:
  explicit exit_t(const Point3F &position) : object_t(position) {}
  void do_draw(ftxui::Canvas &canvas, Point2 point, ftxui::Color color);
  void draw_foreground(ftxui::Canvas &canvas, const Point2 &point) override;
  void draw_background(ftxui::Canvas &canvas, const Point2 &point) override;
};
