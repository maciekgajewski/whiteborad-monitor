add_library(monitor_lib

    monitor.cc include/monitor_lib/monitor.hh
)

target_include_directories(monitor_lib INTERFACE include)
target_include_directories(monitor_lib PRIVATE include/monitor_lib)

target_link_libraries(monitor_lib fmt)