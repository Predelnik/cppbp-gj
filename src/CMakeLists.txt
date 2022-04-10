find_package(fmt CONFIG)
find_package(spdlog CONFIG)
find_package(docopt CONFIG)

# Generic test that uses conan libs
add_executable(demo
	main.cpp
	gamestate.cpp
	gamestate.hpp
	constants.hpp
	draw_helpers.hpp
	draw_helpers.cpp
	geometry/geom_funcs.cpp
	geometry/geom_funcs.hpp
	geometry/point3f.hpp
	geometry/point2f.hpp
	geometry/point3.hpp
	geometry/point2.hpp
	geometry/plane.hpp
	objects/object.cpp
	objects/object.hpp
	objects/tree.cpp
	objects/tree.hpp
	objects/grass.cpp
	objects/grass.hpp
	objects/exit.cpp
	objects/exit.hpp
)
target_link_libraries(
  demo
  PRIVATE project_options
          project_warnings
          docopt::docopt
          fmt::fmt
          spdlog::spdlog)

target_link_system_libraries(
  demo
  PRIVATE
  ftxui::screen
  ftxui::dom
  ftxui::component)

target_include_directories(demo PRIVATE "${CMAKE_BINARY_DIR}/configured_files/include")
target_include_directories(demo PRIVATE "${CMAKE_SOURCE_DIR}/src")