#include "constants.hpp"

#include "draw_helpers.hpp"
#include "ftxui/dom/canvas.hpp"

#include <algorithm>

void draw_chars(ftxui::Canvas &canvas, Point2 point, const ftxui::Color &color, std::string_view str)
{
  point.y = point.y / row_height * row_height;// to align y-coordinate with rows for text
  const auto line_cnt = std::count(str.begin(), str.end(), '\n');
  point.y -= static_cast<int>(line_cnt) * row_height;
  size_t line_start = 0;
  size_t line_end = 0;
  do {
    line_end = str.find('\n', line_start);
    canvas.DrawText(point.x, point.y, std::string(str.substr(line_start, line_end - line_start)), color);
    line_start = line_end + 1;
    point.y += row_height;
  } while (line_end != std::string_view::npos);
}
