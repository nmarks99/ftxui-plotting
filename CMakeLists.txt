cmake_minimum_required (VERSION 3.22)

project(ftxui-plot)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)

include(cmake/ftxui.cmake)

add_library(ftxui-plot STATIC
    include/ftxui-plot/plot.hpp
)
target_link_libraries(ftxui-plot PUBLIC ftxui::component)

target_include_directories(ftxui-plot PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

if (PROJECT_IS_TOP_LEVEL)
    add_subdirectory(example)
endif()
