#include "gamestate.hpp"

#include "draw_helpers.hpp"

#include <fmt/format.h>
#include <ftxui/dom/canvas.hpp>

#include "geometry/geom_funcs.hpp"
#include "geometry/plane.hpp"
#include "geometry/point2.hpp"
#include "objects/exit.hpp"
#include "objects/grass.hpp"
#include "objects/object.hpp"

#include <algorithm>
#include <chrono>
#include <random>

inline Point3F GameState::get_world_center_to_player() const { return (world_center - player_pos).normalized(); }
Point3F GameState::get_camera_x() const { return get_world_center_to_player().cross(player_normal); }
void GameState::process_before_frame()
{
  if (cur_time - last_move_time > stop_move_animation_threshold) { step_frame = 0; }
}

void GameState::check_win_condition() {
  if (!is_win && (player_pos - exit_object->position).length() < win_threshold) {
    is_win = true;
    win_time = cur_time;
  }
}

void GameState::post_process_movement() {
  player_dir = get_world_center_to_player().cross (player_normal);
  if (!facing_right) { player_dir = -player_dir; }
  player_pos = world_center + (player_pos - world_center).normalized() * world_radius;
  last_move_time = cur_time;
  check_win_condition();
}

void GameState::post_process_rotation(double step) {
  const auto other = player_normal.cross(get_world_center_to_player());
  player_normal = player_normal * cos(step) + other * sin(step);
  post_process_movement();
}

GameState::GameState(){ generate_world (); }

void GameState::on_arrow_up() { post_process_rotation(angle_step); }
void GameState::on_arrow_down() {
  post_process_rotation(-angle_step);
}

void GameState::inc_step_frame() {
  ++step_frame;
  step_frame %= step_frame_count;
}

void GameState::on_arrow_right() {
  if (facing_right)
  {
    player_pos += player_dir;
    inc_step_frame();
  } else {
    facing_right = true;
  }
  post_process_movement();
}

void GameState::on_arrow_left() {
  if (!facing_right)
  {
    player_pos += player_dir;
    inc_step_frame();
  } else {
    facing_right = false;
  }
  post_process_movement();
}

void GameState::draw(ftxui::Canvas &canvas)
{
  cur_time = std::chrono::steady_clock::now();
  process_before_frame();
  draw_background(canvas);
  draw_object1(canvas);
  draw_object2(canvas);
  draw_objects_background(canvas);
  draw_planet(canvas);
  draw_player(canvas);
  draw_objects_foreground(canvas);
  draw_overlay(canvas);
#if _DEBUG
  draw_debug(canvas);
#endif
  if (is_win) { draw_victory(canvas); }
}

bool GameState::is_done() const { return is_win &&cur_time - win_time > exit_threshold; }

Point2F GameState::get_projection_f(const Point3F &point) const
{
  const auto projection = geom::project_on(point, { get_camera_normal(), player_pos }) - player_pos;
  const auto p = Point2F {projection.dot(get_camera_x()), projection.dot(get_world_center_to_player())} + screen_offset;
  return p;
}

Point2 GameState::get_projection(const Point3F &point) const { return Point2(get_projection_f (point)); }

project_info_t GameState::get_projection_info(const Point3F &point) const
{
  return { get_projection_f(point), (point - player_pos).dot(get_camera_normal()) };
}

void GameState::draw_objects_foreground(ftxui::Canvas &canvas) const
{
  for (const auto &object : objects) {
    auto info = get_projection_info(object->position);
    Point2 p(info.projection);
    if (info.distance >= 0.0) { object->draw_foreground(canvas, p); }
  }
}

void GameState::draw_objects_background(ftxui::Canvas &canvas) const
{
  for (const auto &object : objects) {
    auto info = get_projection_info(object->position);
    Point2 p(info.projection);
    if (info.distance < 0.0) { object->draw_background(canvas, p); }
  }
}

void GameState::generate_grass() {
  constexpr int grass_cnt = 300;
  static std::uniform_int_distribution<> distr(0, 1);
  for (int i = 0; i < grass_cnt; ++i) {
    objects.emplace_back(std::make_unique<grass_t>(generate_random_coords_on_world(), distr(re)));
  }
}

void GameState::generate_trees() {
  constexpr int tree_cnt = 75;

  for (int i = 0; i < tree_cnt; ++i) {
    objects.emplace_back(std::make_unique<tree_t>(generate_random_coords_on_world()));
  }
}

void GameState::draw_background(ftxui::Canvas &canvas)
{
  // pseudo random generation
  constexpr int x_start = 3;
  constexpr int y_start = 5;
  constexpr int x_mult = 48271;
  constexpr int y_mult = 11141;
  constexpr int mod_num = 0x7fffffff;
  constexpr int star_cnt = 200;

  int x = x_start;
  int y = y_start;
  for (int i = 0; i < star_cnt; ++i) {
    x = (x * x_mult) % mod_num;
    y = (y * y_mult) % mod_num;
    canvas.DrawPoint(x % canvas.width(), y % canvas.height(), true, ftxui::Color::White);
  }
}

void GameState::generate_exit() {
  auto obj = std::make_unique<exit_t>(generate_random_coords_on_world());
  exit_object = obj.get();
  objects.emplace_back(std::move(obj));
}

void GameState::generate_world() {
  generate_grass();
  generate_trees();
  generate_exit();
}

#ifdef _DEBUG
void GameState::draw_debug([[maybe_unused]] ftxui::Canvas &canvas) const
{
  canvas.DrawText(0, 0, fmt::format("Distance to exit: {}", (player_pos - exit_object->position).length()));
}
#endif

void GameState::draw_victory(ftxui::Canvas &canvas)
{
  constexpr Point2 victory_pos{ 20, 20 };
  canvas.DrawText(victory_pos.x, victory_pos.y, "Victory!", ftxui::Color::White);
}
void GameState::draw_overlay([[maybe_unused]] ftxui::Canvas &canvas) {}

std::string GameState::get_legs_str_left() const {
  switch (step_frame) {
  case 0:
    return R"(/ \)";
  case 1:
    return R"( |\)";
  case 2:
    return R"( | )";
  case 3:
    return R"(/| )";
  default:
    return {};
  }
}

std::string GameState::get_legs_str() const {
  auto str = get_legs_str_left();
  if (facing_right) {
    std::reverse(str.begin (), str.end ());
    for (auto &c : str) {
      if (c == '\\')
      {
        c = '/';
      } else if (c == '/')
      {
        c = '\\';
      }
    }
  }
  return str;
}

void GameState::draw_player(ftxui::Canvas &canvas) const
{
  const auto pos = get_projection(player_pos);
  auto str = fmt::format("{}\n{}\n{}", facing_right ? R"( O>)" : R"(<O)", R"(/|\)", get_legs_str());
  draw_chars(canvas, pos, ftxui::Color::White, str);
}

void GameState::draw_planet(ftxui::Canvas &canvas) const
{
  const auto pos = get_projection(world_center);
  canvas.DrawPointCircleFilled(pos.x, pos.y, static_cast<int>(world_radius), ftxui::Color::RosyBrown);
}

void GameState::draw_object1(ftxui::Canvas &canvas) const
{
  const auto info = get_projection_info(object1_pos);
  if (info.distance >= 0) {
    const auto p = Point2 (info.projection);
    constexpr auto base_radius = 15.0;
    canvas.DrawPointCircle(p.x, p.y, static_cast<int>(base_radius * (1. / (1. + info.distance))), ftxui::Color::Green);
  }
}

void GameState::draw_object2(ftxui::Canvas &canvas) const
{
  const auto info = get_projection_info(object2_pos);
  if (info.distance >= 0) {
    const auto p = Point2 (info.projection);
    constexpr auto base_radius = 25.0;
    constexpr auto size_pow_scale = 1. / 3.;
    constexpr auto max_color = 255;
    constexpr auto dist_mult = 3;
    canvas.DrawPointCircle(p.x,
      p.y,
      static_cast<int>(base_radius * (1. / pow(1. + info.distance, size_pow_scale))),
      ftxui::Color(static_cast<uint8_t>(max_color - dist_mult * static_cast<int>(info.distance)), 0, 0));
  }
}

Point3F GameState::generate_random_coords_on_world()
{
  static auto theta_distr = std::uniform_real_distribution<>(0.0, std::numbers::pi);
  static auto phi_distr = std::uniform_real_distribution(0.0, 2 * std::numbers::pi);
  const double theta = theta_distr(re);
  const double phi = phi_distr(re);
  return Point3F{ cos(phi) * sin(theta), sin(phi) * sin(theta), cos(theta) } * world_radius;
}
