#include <numbers>
#include <random>
#include <random>

#include <docopt/docopt.h>
#include <ftxui/component/captured_mouse.hpp>// for ftxui
#include <ftxui/component/component.hpp>// for Slider
#include <ftxui/component/screen_interactive.hpp>// for ScreenInteractive
#include <spdlog/spdlog.h>

#include "gamestate.hpp"
#include "geometry/point2.hpp"

// This file will be generated automatically when you run the CMake
// configuration step. It creates a namespace called `cppbp-gj`. You can modify
// the source template at `configured_files/config.hpp.in`.
#include <internal_use_only/config.hpp>

class exit_t;

void game_iteration_canvas()
{
  auto screen = ftxui::ScreenInteractive::TerminalOutput();
  GameState state;
  auto make_layout = [&] {
    auto canvas = ftxui::Canvas(canvas_size.x, canvas_size.y);
    state.draw(canvas);
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
    if (state.is_done ()) {
      screen.ExitLoopClosure()();
      return true;
     }
    if (event == ftxui::Event::ArrowRight) {
      state.on_arrow_right();
      return true;
    }
    if (event == ftxui::Event::ArrowLeft) {
      state.on_arrow_left();
      return true;
     }
    if (event == ftxui::Event::ArrowUp) {
      state.on_arrow_up();
      return true;
    }
    if (event == ftxui::Event::ArrowDown) {
       state.on_arrow_down();
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
      R"(demo

    Usage:
          demo
          demo (-h | --help)
          demo --version
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
    game_iteration_canvas();

    //    consequence_game();
  } catch (const std::exception &e) {
    fmt::print("Unhandled exception in main: {}", e.what());
  }
}
