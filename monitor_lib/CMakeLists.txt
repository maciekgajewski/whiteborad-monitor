add_library(monitor_lib

    monitor.cc monitor.hh
    mem_maps.cc mem_maps.hh
)

target_include_directories(monitor_lib INTERFACE ..)

target_link_libraries(monitor_lib fmt)

target_include_directories(monitor_lib SYSTEM PRIVATE /usr/include/libelfin)
target_link_libraries(monitor_lib elf++)
target_link_libraries(monitor_lib dwarf++)
target_link_libraries(monitor_lib Boost::filesystem)