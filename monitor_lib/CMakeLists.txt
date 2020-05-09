add_library(monitor_lib

    monitor.cc monitor.hh
)

target_include_directories(monitor_lib INTERFACE ..)

target_link_libraries(monitor_lib fmt)