project(whiteboard-monitor)
cmake_minimum_required(VERSION 3.23)

find_package(Boost REQUIRED COMPONENTS filesystem)

add_subdirectory(external)

add_compile_options(-Wall)
set(CMAKE_CXX_STANDARD 17)
add_subdirectory(monitor_lib)
add_subdirectory(monitor_app)
