cmake_minimum_required(VERSION 3.0.0)
project(project1)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5Widgets REQUIRED)
find_package(Eigen3 REQUIRED)
qt5_wrap_ui(UI_HEADERS mainwindow.ui)

include_directories(
  ${EIGEN3_INCLUDE_DIRS}
)

add_library(project1 project1.hpp)

add_executable(project1_main_cli project1_main_cli.cc)
target_link_libraries(project1_main_cli project1)

add_executable(project1_main_gui project1_main_gui.cc mainwindow.cc ${UI_HEADERS})
target_link_libraries(project1_main_gui Qt5::Widgets Qt5::Gui project1)
