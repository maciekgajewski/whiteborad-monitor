cmake_minimum_required(VERSION 3.23)
project(whiteboard-monitor)

find_package(Boost CONFIG REQUIRED COMPONENTS filesystem)
find_package(fmt CONFIG REQUIRED)

add_compile_options(-Wall)
set(CMAKE_CXX_STANDARD 20)

add_subdirectory(monitor_lib)
add_subdirectory(monitor_app)
add_subdirectory(test_programs)
