cmake_minimum_required (VERSION 3.22)

project(ftxui-plotting
  LANGUAGES CXX
  VERSION 1.0.0
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(ftxui REQUIRED)

add_executable(example src/main.cpp src/plot.cpp)
target_include_directories(example PRIVATE src)

target_link_libraries(example
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
)
