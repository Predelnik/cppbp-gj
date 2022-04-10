#pragma once

#include "object.hpp"

#include <random>

class grass_t : public object_t
{
public:
  grass_t(const Point3F &pos, int grass_type);
  void draw_background([[maybe_unused]] ftxui::Canvas &canvas, [[maybe_unused]] const Point2 &point) override;
  void draw_foreground(ftxui::Canvas &canvas, const Point2 &point) override;

private:
  std::string str;
};


