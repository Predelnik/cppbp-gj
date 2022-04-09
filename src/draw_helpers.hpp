#pragma once
#include <string_view>

class Point2;

namespace ftxui {
struct Canvas;
class Color;
}

void draw_chars(ftxui::Canvas &canvas, Point2 point, const ftxui::Color &color, std::string_view str);
