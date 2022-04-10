#pragma once

#include <chrono>
#include <numbers>

#include "geometry/point2.hpp"
#include "geometry/point2f.hpp"
#include "geometry/point3f.hpp"

using namespace std::chrono_literals;
constexpr Point3F world_center{ 0., 0., 0. };
constexpr double world_radius = 250.0;
constexpr Point2 canvas_size = { 200, 100 };
constexpr Point2F screen_offset = { canvas_size.x / 2., canvas_size.y * 3. / 4. };
constexpr auto win_threshold = 5.0;
constexpr int row_height = 4;
constexpr auto step_frame_count = 4;
constexpr double angle_step = std::numbers::pi_v<double> / 80;
constexpr auto stop_move_animation_threshold = 250ms;
constexpr auto exit_threshold = 2s;
using time_point = std::chrono::time_point<std::chrono::steady_clock>;