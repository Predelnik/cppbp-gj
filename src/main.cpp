#include <numbers>
#include <random>
#include <random>

#include <docopt/docopt.h>
#include <ftxui/component/captured_mouse.hpp>// for ftxui
#include <ftxui/component/component.hpp>// for Slider
#include <ftxui/component/screen_interactive.hpp>// for ScreenInteractive
#include <spdlog/spdlog.h>

// This file will be generated automatically when you run the CMake
// configuration step. It creates a namespace called `cppbp-gj`. You can modify
// the source template at `configured_files/config.hpp.in`.
#include <internal_use_only/config.hpp>

using namespace std::chrono_literals;

struct Color
{
  std::uint8_t R{};
  std::uint8_t G{};
  std::uint8_t B{};
};

// A simple way of representing a bitmap on screen using only characters
struct Bitmap : ftxui::Node
{
  Bitmap(std::size_t width, std::size_t height)// NOLINT same typed parameters adjacent to each other
    : width_(width), height_(height)
  {}

  Color &at(std::size_t x, std::size_t y) { return pixels.at(width_ * y + x); }

  void ComputeRequirement() override
  {
    requirement_ = ftxui::Requirement{
      .min_x = static_cast<int>(width_), .min_y = static_cast<int>(height_ / 2), .selected_box{ 0, 0, 0, 0 }
    };
  }

  void Render(ftxui::Screen &screen) override
  {
    for (std::size_t x = 0; x < width_; ++x) {
      for (std::size_t y = 0; y < height_ / 2; ++y) {
        auto &p = screen.PixelAt(box_.x_min + static_cast<int>(x), box_.y_min + static_cast<int>(y));
        p.character = "▄";
        const auto &top_color = at(x, y * 2);
        const auto &bottom_color = at(x, y * 2 + 1);
        p.background_color = ftxui::Color{ top_color.R, top_color.G, top_color.B };
        p.foreground_color = ftxui::Color{ bottom_color.R, bottom_color.G, bottom_color.B };
      }
    }
  }

  [[nodiscard]] auto width() const noexcept { return width_; }

  [[nodiscard]] auto height() const noexcept { return height_; }

  [[nodiscard]] auto &data() noexcept { return pixels; }

private:
  std::size_t width_;
  std::size_t height_;

  std::vector<Color> pixels = std::vector<Color>(width_ * height_, Color{});
};

class Point3
{
public:
  int x;
  int y;
  int z;
};

class Point2F
{
public:
  Point2F operator+(const Point2F &other) const { return { x + other.x, y + other.y}; }
  Point2F operator-(const Point2F &other) const { return { x - other.x, y - other.y }; }
  [[nodiscard]] double length() const { return sqrt(sq_length()); }
  [[nodiscard]] double sq_length() const { return x * x + y * y; }

public:
  double x;
  double y;
};

class Point2
{
public:
  constexpr Point2(int x, int y) : x(x), y(y) {}
  constexpr Point2(const Point2F &p) : x(static_cast<int>(p.x)), y(static_cast<int>(p.y)) {}

public:
  int x;
  int y;
};

class Point3F
{
public:
  double x;
  double y;
  double z;

public:
  Point3F operator-(const Point3F &other) const
  {
    return { x - other.x, y - other.y, z - other.z };
  }
  Point3F operator+(const Point3F &other) const
  {
    return { x + other.x, y + other.y, z + other.z };
  }
  Point3F &operator+=(const Point3F &other)
  {
    *this = *this + other;
    return *this;
  }

  [[nodiscard]] double length() const
  {
    return sqrt(sq_length());
  }

  [[nodiscard]] double sq_length() const
  {
    return x * x + y * y + z * z;
  }

  [[nodiscard]] Point3F normalized() const
  {
    return *this / length();
  }
  Point3F operator-() const
  {
    return {-x, -y, -z};
  }
  Point3F operator/(double v) const
  {
    return{x/v, y/v, z/v};
  }
  Point3F operator*(double v) const { return { x * v, y * v, z * v }; }

  [[nodiscard]] Point3F cross(const Point3F &other) const
  {
    return {y *other.z - z *other.y, other.x * z - other.z * x, x * other.y - y * other.x};
  }

  [[nodiscard]] double dot(const Point3F &other) const
  {
    return x *other.x + y *other.y + z * other.z;
  }
};

class Plane
{
public:
  Point3F normal;
  Point3F origin;
};

Point3F project_on(const Point3F &point, const Plane &plane)
{
  // inspired by https://stackoverflow.com/a/9605695/1269661
  const auto v = point - plane.origin;
  const auto dist= v.dot(plane.normal);
  return point - plane.normal * dist;
}



constexpr Point3F world_center{ 0., 0., 0. };
constexpr double world_radius = 400.0;
constexpr Point2 canvas_size = { 200, 100 };
constexpr Point2F screen_offset = { canvas_size.x / 2., canvas_size.y * 3. / 4. };

bool facing_right = true;
int step_frame = 0;
Point3F player_pos = { world_center.x, world_center.y, world_center.z + world_radius};
Point3F object1_pos = { world_center.x + world_radius / 10, world_center.y, world_center.z + world_radius};
Point3F object2_pos = { world_center.x - world_radius / 5, world_center.y, world_center.z + world_radius };
double angle = 0.0;
constexpr double angle_step = std::numbers::pi_v<double> * 0.025;
Point3F player_dir = { 1.0, 0.0, 0.0 };
Point3F player_normal = { 0.0, 1.0, 0.0 };
Point3F get_world_center_to_player() { return (world_center - player_pos).normalized(); }
Point3F  get_camera_x() { return get_world_center_to_player().cross (player_normal); }
Point3F get_camera_normal() { return player_normal; }
std::chrono::time_point<std::chrono::steady_clock> last_move_time;
std::chrono::time_point<std::chrono::steady_clock> cur_time;
auto stop_move_animation_threshold = 250ms;


void post_process_movement()
{
  player_dir = get_world_center_to_player().cross (player_normal);
  if (!facing_right)
    player_dir = -player_dir;
  player_pos = world_center + (player_pos - world_center).normalized() * world_radius;
  last_move_time = cur_time;
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

void on_arrow_right()
{
  if (facing_right)
    player_pos += player_dir;
  else
    facing_right = true;
  post_process_movement();
}
void on_arrow_left()
{
  if (!facing_right)
    player_pos += player_dir;
  else
    facing_right = false;
  post_process_movement();
}

Point2F get_projection(const Point3F &point)
{
  const auto projection = project_on(point, { get_camera_normal(), player_pos }) - player_pos;
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

class object_t
{
public:
  object_t(Point3F position) : position(position) {}
  object_t(const object_t &) = delete;
  object_t &operator=(const object_t &) = delete;
  object_t(object_t &&) = delete;
  object_t &operator=(object_t &&) = delete;
  virtual ~object_t() = default;

  virtual void draw(ftxui::Canvas &canvas, const Point2 &point) = 0;

public:
  Point3F position;
};

std::random_device rd;
std::default_random_engine re(rd());

class grass_t : public object_t
{
public:
  grass_t(Point3F pos) : object_t(pos)
  {
    static std::uniform_int_distribution<> distr(0,1);
    switch (distr (re)) {
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

  void draw(ftxui::Canvas &canvas, const Point2 &point) override { canvas.DrawText(point.x, point.y, str, ftxui::Color::Green);
  }
private:
  std::string str;
};

std::vector<std::unique_ptr<object_t>> objects;

void draw_objects(ftxui::Canvas & canvas) {
  for (const auto &object : objects) {
    auto info = get_projection_info(object->position);
    if (info.distance < 0.0) {
      Point2 p(info.projection);
      object->draw(canvas, p);
    }
  }
}

Point3F generate_random_coords_on_world()
{
  static const auto distr = std::uniform_real_distribution<>(-1.0, 1.0);
  static const auto c_sign = std::uniform_int_distribution(0, 1);
  const double a = distr(re);
  const double b = distr(re);
  const double c = sqrt(1 - a * a - b * b) * (c_sign(re) * 2 - 1);
  return Point3F{ a, b, c } * world_radius ;
}

void generate_grass()
{
  constexpr int grass_cnt = 300;

  for (int i = 0; i < grass_cnt; ++i) {
    objects.emplace_back(std::make_unique<grass_t>(generate_random_coords_on_world()));
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

void generate_world()
{ generate_grass(); }

constexpr auto step_frame_count = 4;
void inc_step_frame()
{
  ++step_frame;
  step_frame %= step_frame_count;
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

void game_iteration_canvas()
{
  auto screen = ftxui::ScreenInteractive::TerminalOutput();
  auto make_layout = [&] {
    cur_time = std::chrono::steady_clock::now();
    auto canvas = ftxui::Canvas(canvas_size.x, canvas_size.y);

    draw_background(canvas);
    draw_object1(canvas);
    draw_object2(canvas);
    draw_planet(canvas);
    draw_player(canvas);
    draw_objects(canvas);
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
    if (event == ftxui::Event::ArrowRight) {
      on_arrow_right();
      inc_step_frame();
      return true;
    }
    else if (event == ftxui::Event::ArrowLeft) {
      on_arrow_left();
      inc_step_frame();
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
