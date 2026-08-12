cmake_minimum_required(VERSION 3.14)
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project(mister-lister)
find_package(ftxui REQUIRED)
add_executable(mister-lister
  main.cpp
  csv.cpp
  modify_table.cpp
  utilities.cpp)
target_link_libraries(mister-lister PRIVATE ftxui::component ftxui::dom ftxui::screen)

