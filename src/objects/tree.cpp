#include "tree.hpp"

#include "draw_helpers.hpp"
#include "ftxui/screen/color.hpp"
#include "geometry/point2.hpp"

tree_t::tree_t(const Point3F &position): object_t(position) {}
void tree_t::do_draw(ftxui::Canvas &canvas, Point2 point, ftxui::Color color) {
  draw_chars(canvas,
    point,
    color,
    R"(
 /\
/||\
/||\
 ||)");
}


void tree_t::draw_foreground(ftxui::Canvas &canvas, const Point2 &point) {
  const auto fg_color = ftxui::Color(48, 150, 0);
  do_draw(canvas, point, fg_color);
}

void tree_t::draw_background(ftxui::Canvas &canvas, const Point2 &point)
{
  const auto bg_color = ftxui::Color(32, 90, 0);
  do_draw(canvas, point, bg_color);
}
