#pragma once
#include "constants.hpp"
#include "geometry/point2f.hpp"
#include "geometry/point3f.hpp"
#include "objects/tree.hpp"

#include <memory>
#include <random>
#include <string>
#include <vector>

class object_t;

namespace ftxui {
struct Canvas;
}

class exit_t;

class project_info_t
{
public:
  Point2F projection;
  double distance;// if negative then object is in front
};

class GameState
{
public:
  GameState();
  void on_arrow_up();
  void on_arrow_down();
  void on_arrow_right();
  void on_arrow_left();
  void draw(ftxui::Canvas &canvas);
  bool is_done() const;
  void generate_world();

private:
  [[nodiscard]] Point3F get_world_center_to_player() const;
  [[nodiscard]] Point3F get_camera_x() const;
  [[nodiscard]] Point3F get_camera_normal() const { return player_normal; }
  [[nodiscard]] Point2F get_projection(const Point3F &point) const;
  [[nodiscard]] std::string get_legs_str_left() const;
  [[nodiscard]] std::string get_legs_str() const;
  [[nodiscard]] Point3F generate_random_coords_on_world();
  [[nodiscard]] project_info_t get_projection_info(const Point3F &point) const;

  void process_before_frame();
  void check_win_condition();
  void post_process_movement();
  void post_process_rotation(double step);
  void inc_step_frame();
  void generate_grass();
  void generate_trees();
  void generate_exit();

  static void draw_background(ftxui::Canvas &canvas);
  static void draw_victory(ftxui::Canvas &canvas);
  static void draw_overlay(ftxui::Canvas &);

  void draw_objects_foreground(ftxui::Canvas &canvas) const;
  void draw_objects_background(ftxui::Canvas &canvas) const;
  void draw_player(ftxui::Canvas &canvas) const;
  void draw_planet(ftxui::Canvas &canvas) const;
  void draw_object1(ftxui::Canvas &canvas) const;
  void draw_object2(ftxui::Canvas &canvas) const;
  void draw_debug(ftxui::Canvas &canvas) const;

public:
  bool facing_right = true;
  int step_frame = 0;
  Point3F player_pos = { world_center.x, world_center.y, world_center.z + world_radius };
  Point3F object1_pos = { world_center.x + world_radius / 10, world_center.y, world_center.z + world_radius };
  Point3F object2_pos = { world_center.x - world_radius / 5, world_center.y, world_center.z + world_radius };
  double angle = 0.0;
  Point3F player_dir = { 1.0, 0.0, 0.0 };
  Point3F player_normal = { 0.0, 1.0, 0.0 };
  time_point last_move_time;
  time_point cur_time;
  time_point win_time;
  exit_t *exit_object {};
  bool is_win = false;
  std::vector<std::unique_ptr<object_t>> objects;
  std::default_random_engine re{std::random_device{}()};
};


