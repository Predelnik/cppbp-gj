#include <numbers>
#include <random>
#include <random>

#include <docopt/docopt.h>
#include <ftxui/component/captured_mouse.hpp>// for ftxui
#include <ftxui/component/component.hpp>// for Slider
#include <ftxui/component/screen_interactive.hpp>// for ScreenInteractive
#include <spdlog/spdlog.h>

#include "geometry/geom_funcs.hpp"
#include "geometry/plane.hpp"
#include "geometry/point2.hpp"
#include "geometry/point2f.hpp"
#include "geometry/point3f.hpp"

// This file will be generated automatically when you run the CMake
// configuration step. It creates a namespace called `cppbp-gj`. You can modify
// the source template at `configured_files/config.hpp.in`.
#include <internal_use_only/config.hpp>

class exit_t;
using namespace std::chrono_literals;

constexpr Point3F world_center{ 0., 0., 0. };
constexpr double world_radius = 250.0;
constexpr Point2 canvas_size = { 200, 100 };
constexpr Point2F screen_offset = { canvas_size.x / 2., canvas_size.y * 3. / 4. };
constexpr auto win_threshold = 5.0;
constexpr int row_height = 4;
constexpr auto step_frame_count = 4;
constexpr auto compass_len = 15.0;

bool facing_right = true;
int step_frame = 0;
Point3F player_pos = { world_center.x, world_center.y, world_center.z + world_radius};
Point3F object1_pos = { world_center.x + world_radius / 10, world_center.y, world_center.z + world_radius};
Point3F object2_pos = { world_center.x - world_radius / 5, world_center.y, world_center.z + world_radius };
double angle = 0.0;
constexpr double angle_step = std::numbers::pi_v<double> / 80;
Point3F player_dir = { 1.0, 0.0, 0.0 };
Point3F player_normal = { 0.0, 1.0, 0.0 };
Point3F get_world_center_to_player() { return (world_center - player_pos).normalized(); }
Point3F  get_camera_x() { return get_world_center_to_player().cross (player_normal); }
Point3F get_camera_normal() { return player_normal; }
std::chrono::time_point<std::chrono::steady_clock> last_move_time;
std::chrono::time_point<std::chrono::steady_clock> cur_time;
std::chrono::time_point<std::chrono::steady_clock> win_time;
auto stop_move_animation_threshold = 250ms;
auto exit_threshold = 2s;
exit_t *exit_object;
bool is_win = false;

class object_t
{
public:
  object_t(Point3F position) : position(position) {}
  object_t(const object_t &) = delete;
  object_t &operator=(const object_t &) = delete;
  object_t(object_t &&) = delete;
  object_t &operator=(object_t &&) = delete;
  virtual ~object_t() = default;

  virtual void draw_foreground(ftxui::Canvas &canvas, const Point2 &point) = 0;
  virtual void draw_background(ftxui::Canvas &canvas, const Point2 &point) = 0;

public:
  Point3F position{};
};

std::random_device rd;
std::default_random_engine re(rd());

class grass_t : public object_t
{
public:
  grass_t(Point3F pos) : object_t(pos)
  {
    static std::uniform_int_distribution<> distr(0, 1);
    switch (distr(re)) {
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

  void draw_background([[maybe_unused]] ftxui::Canvas &canvas, [[maybe_unused]] const Point2 &point) override
  {
  }

  void draw_foreground(ftxui::Canvas &canvas, const Point2 &point) override
  {
    canvas.DrawText(point.x, point.y, str, ftxui::Color::Green);
  }

private:
  std::string str;
};

void draw_chars(ftxui::Canvas &canvas, Point2 point, const ftxui::Color &color, std::string_view str)
{
  point.y = point.y / row_height * row_height; // to align y-coordinate with rows for text
  const auto line_cnt = std::ranges::count(str,'\n');
  point.y -= static_cast<int> (line_cnt) * row_height;
  size_t line_start = 0, line_end;
  do
  {
    line_end = str.find('\n', line_start);
    canvas.DrawText(point.x, point.y, std::string (str.substr(line_start, line_end - line_start)), color);
    line_start = line_end + 1;
    point.y += row_height;
  } while (line_end != std::string_view::npos);
}

class exit_t : public object_t
{
public:
  explicit exit_t(const Point3F &position) : object_t(position) {}
  void do_draw(ftxui::Canvas &canvas, Point2 point, ftxui::Color color)
  {
    draw_chars(canvas, point, color, 
R"(/-\
| |
| |
\-/)");
  }

  void draw_foreground(ftxui::Canvas &canvas, const Point2 &point) override { do_draw(canvas, point, ftxui::Color (0, 0, 255));
  }

  void draw_background(ftxui::Canvas &canvas, const Point2 &point) override
  {
    do_draw(canvas, point, ftxui::Color(0, 0, 192));
  }
};

class tree_t : public object_t
{
public:
  explicit tree_t(const Point3F &position) : object_t(position) {}

  static void do_draw(ftxui::Canvas &canvas, Point2 point, ftxui::Color color)
  {
    draw_chars(canvas,
      point,
      color,
      R"(
 /\
/||\
/||\
 ||)");
  }

  void draw_foreground(ftxui::Canvas &canvas, const Point2 &point) override
  {
    do_draw(canvas, point, ftxui::Color(48, 150, 0));
  }

  void draw_background(ftxui::Canvas &canvas, const Point2 &point) override
  {
    do_draw(canvas, point, ftxui::Color(32, 90, 0));
  }
};

void check_win_condition()
{
  if (!is_win && (player_pos - exit_object->position).length() < win_threshold) {
    is_win = true;
    win_time = cur_time;
  }
}

void post_process_movement()
{
  player_dir = get_world_center_to_player().cross (player_normal);
  if (!facing_right)
    player_dir = -player_dir;
  player_pos = world_center + (player_pos - world_center).normalized() * world_radius;
  last_move_time = cur_time;
  check_win_condition();
}

void post_process_rotation(double step)
{
  const auto other = player_normal.cross(get_world_center_to_player());
  player_normal = player_normal * cos(step) + other * sin(step);
  post_process_movement();
}

void on_arrow_up()
{ post_process_rotation(angle_step); }

void on_arrow_down()
{
  post_process_rotation(-angle_step);
}

void inc_step_frame()
{
  ++step_frame;
  step_frame %= step_frame_count;
}


void on_arrow_right()
{
  if (facing_right)
  {
    player_pos += player_dir;
    inc_step_frame();
  }
  else
    facing_right = true;
  post_process_movement();
}
void on_arrow_left()
{
  if (!facing_right)
  {
    player_pos += player_dir;
    inc_step_frame();
  }
  else
    facing_right = false;
  post_process_movement();
}

Point2F get_projection(const Point3F &point)
{
  const auto projection = geom::project_on(point, { get_camera_normal(), player_pos }) - player_pos;
  const auto p = Point2F {projection.dot(get_camera_x()), projection.dot(get_world_center_to_player())} + screen_offset;
  return p;
}

class project_info_t
{
public:
  Point2F projection;
  double distance; // if negative then object is in front
};

project_info_t get_projection_info(const Point3F &point)
{
  return {get_projection (point), (point - player_pos).dot(get_camera_normal()) };
}

std::vector<std::unique_ptr<object_t>> objects;

void draw_objects_foreground(ftxui::Canvas & canvas) {
  for (const auto &object : objects) {
    auto info = get_projection_info(object->position);
    Point2 p(info.projection);
    if (info.distance >= 0.0) {
      object->draw_foreground(canvas, p);
    }
  }
}

void draw_objects_background(ftxui::Canvas &canvas)
{
  for (const auto &object : objects) {
    auto info = get_projection_info(object->position);
    Point2 p(info.projection);
    if (info.distance < 0.0) { object->draw_background(canvas, p); }
  }
}

Point3F generate_random_coords_on_world()
{
  static const auto theta_distr = std::uniform_real_distribution<>(0.0, std::numbers::pi);
  static const auto phi_distr = std::uniform_real_distribution(0.0, 2 * std::numbers::pi);
  const double theta = theta_distr(re);
  const double phi = phi_distr(re);
  return Point3F{ cos(phi) * sin(theta), sin(phi) * sin(theta), cos(theta) } * world_radius;
}

void generate_grass()
{
  constexpr int grass_cnt = 300;

  for (int i = 0; i < grass_cnt; ++i) {
    objects.emplace_back(std::make_unique<grass_t>(generate_random_coords_on_world()));
  }
}

void generate_trees()
{
  constexpr int tree_cnt = 75;

  for (int i = 0; i < tree_cnt; ++i) {
    objects.emplace_back(std::make_unique<tree_t>(generate_random_coords_on_world()));
  }
}

void draw_background(ftxui::Canvas &canvas)
{
  int x = 3;
  int y = 5;
  for (int i = 0; i < 200; ++i) {
    x = (x * 48271) % 0x7fffffff;
    y = (y * 11141) % 0x7fffffff;
    canvas.DrawPoint(x % canvas.width(), y % canvas.height(), true, ftxui::Color::White);
  }
}

void generate_exit()
{
  auto obj = std::make_unique<exit_t>(generate_random_coords_on_world());
  exit_object = obj.get();
  objects.emplace_back(std::move (obj));
}

void generate_world()
{
  generate_grass();
  generate_trees();
  generate_exit();
}

std::string get_legs_str_left()
{
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

std::string get_legs_str()
{
  auto str = get_legs_str_left();
  if (facing_right) {
    std::ranges::reverse(str);
    for (auto &c : str) {
      if (c == '\\') c = '/';
      else if (c == '/') c = '\\';
    }
  }
  return str;
}

void draw_player(ftxui::Canvas &canvas)
{
  if (cur_time - last_move_time > stop_move_animation_threshold) step_frame = 0;
  const auto pos = get_projection(player_pos);
  const Point2 p(pos);
  canvas.DrawText(p.x - 1, p.y - 9, facing_right ? R"( O>)" : R"(<O)", ftxui::Color::White);
  canvas.DrawText(p.x - 1, p.y - 6, R"(/|\)", ftxui::Color::White);
  const auto str = get_legs_str();

  canvas.DrawText(p.x - 1, p.y - 3, str, ftxui::Color::White);
}

void draw_planet(ftxui::Canvas &canvas)
{
  const auto pos = get_projection(world_center);
  const Point2 p(pos);
  canvas.DrawPointCircleFilled(p.x, p.y, static_cast<int>(world_radius), ftxui::Color::RosyBrown);
}

void draw_object1(ftxui::Canvas &canvas)
{
  const auto info = get_projection_info(object1_pos);
  if (info.distance >= 0) {
    const Point2 p(info.projection);
    canvas.DrawPointCircle(p.x, p.y, static_cast<int>(15 * (1. / (1. + info.distance))), ftxui::Color::Green);
  }
}

void draw_object2(ftxui::Canvas &canvas)
{
  const auto info = get_projection_info(object2_pos);
  if (info.distance >= 0) {
    const Point2 p(info.projection);
    canvas.DrawPointCircle(p.x,
      p.y,
      static_cast<int>(25 * (1. / pow(1. + info.distance, 0.3333))),
      ftxui::Color(static_cast<uint8_t>(255 - 3 * static_cast<int>(info.distance)), 0, 0));
  }
}

void draw_debug(ftxui::Canvas &canvas)
{
#if _DEBUG
  canvas.DrawText(0, 0, fmt::format("Distance to exit: {}", (player_pos - exit_object->position).length()));
#endif
}

void draw_victory(ftxui::Canvas &canvas) { canvas.DrawText(20, 20, "Victory!", ftxui::Color::White); }

void draw_overlay(ftxui::Canvas &) {}

void game_iteration_canvas()
{
  auto screen = ftxui::ScreenInteractive::TerminalOutput();
  auto make_layout = [&] {
    cur_time = std::chrono::steady_clock::now();
    auto canvas = ftxui::Canvas(canvas_size.x, canvas_size.y);
    draw_background(canvas);
    draw_object1(canvas);
    draw_object2(canvas);
    draw_objects_background(canvas);
    draw_planet(canvas);
    draw_player(canvas);
    draw_objects_foreground(canvas);
    draw_overlay(canvas);
    draw_debug(canvas);
    if (is_win) draw_victory(canvas);
    return ftxui::canvas(std::move(canvas));
  };

  const auto renderer = ftxui::Renderer(make_layout);

  std::atomic<bool> refresh_ui_continue = true;

  // This thread exists to make sure that the event queue has an event to
  // process at approximately a rate of 30 FPS
  constexpr auto refresh_time = 1.0s / 30.0;
  std::thread refresh_ui([&] {
    while (refresh_ui_continue) {
      std::this_thread::sleep_for(refresh_time);// NOLINT magic numbers
      screen.PostEvent(ftxui::Event::Custom);
    }
  });

  const auto component = ftxui::CatchEvent(renderer, [&](const ftxui::Event &event) {
    if (is_win && cur_time - win_time > exit_threshold) {
      screen.ExitLoopClosure()();
      return true;
     }
    if (event == ftxui::Event::ArrowRight) {
      on_arrow_right();
      return true;
    }
    else if (event == ftxui::Event::ArrowLeft) {
      on_arrow_left();
      return true;
     }
    else if (event == ftxui::Event::ArrowUp) {
      on_arrow_up();
      return true;
    }
    else if (event == ftxui::Event::ArrowDown) {
       on_arrow_down();
       return true;
     }
    return false;
  });
  screen.Loop(component);

  refresh_ui_continue = false;
  refresh_ui.join();
}

int main(int argc, const char **argv)
{
  try {
    static constexpr auto USAGE =
      R"(intro

    Usage:
          intro
          intro (-h | --help)
          intro --version
 Options:
          -h --help     Show this screen.
          --version     Show version.
)";

    std::map<std::string, docopt::value> args = docopt::docopt(USAGE,
      { std::next(argv), std::next(argv, argc) },
      true,// show help if requested
      fmt::format("{} {}",
        cppbp_gj::cmake::project_name,
        cppbp_gj::cmake::project_version));// version string, acquired
                                            // from config.hpp via CMake
    generate_world();
    game_iteration_canvas();

    //    consequence_game();
  } catch (const std::exception &e) {
    fmt::print("Unhandled exception in main: {}", e.what());
  }
}
