#include "exit.hpp"

#include "draw_helpers.hpp"
#include "ftxui/screen/color.hpp"
#include "geometry/point2.hpp"

void exit_t::do_draw(ftxui::Canvas &canvas, Point2 point, ftxui::Color color)
{
  draw_chars(canvas,
    point,
    color,
    R"(/-\
| |
| |
\-/)");
}

void exit_t::draw_foreground(ftxui::Canvas &canvas, const Point2 &point) { do_draw(canvas, point, ftxui::Color (0, 0, 255));
}

void exit_t::draw_background(ftxui::Canvas &canvas, const Point2 &point) {
  do_draw(canvas, point, ftxui::Color(0, 0, 192));
}
