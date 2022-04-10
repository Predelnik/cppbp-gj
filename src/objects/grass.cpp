#include "grass.hpp"

#include "ftxui/dom/canvas.hpp"
#include "ftxui/screen/color.hpp"
#include "geometry/point2.hpp"
#include "geometry/point3f.hpp"

grass_t::grass_t(const Point3F &pos, int grass_type) : object_t(pos)
{
  switch (grass_type) {
  case 0:
    str = R"(///)";
    break;
  case 1:
    str = R"(\\\)";
    break;
  default:
    break;
  }
}

void grass_t::draw_background([[maybe_unused]] ftxui::Canvas &canvas, [[maybe_unused]] const Point2 &point) {}
void grass_t::draw_foreground(ftxui::Canvas &canvas, const Point2 &point)
{
  canvas.DrawText(point.x, point.y, str, ftxui::Color::Green);
}
