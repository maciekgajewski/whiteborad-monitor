add_library(monitor_lib

    monitor.cc monitor.hh
    mem_maps.cc mem_maps.hh
    process_debug_info.cc process_debug_info.hh
    file_debug_info.cc file_debug_info.hh
    word.hh
    registers.cc registers.hh
)

target_include_directories(monitor_lib INTERFACE ..)

target_link_libraries(monitor_lib PRIVATE fmt::fmt)
target_link_libraries(monitor_lib PRIVATE Boost::filesystem)

# TODO supply these nicely as well
target_include_directories(monitor_lib SYSTEM PRIVATE /usr/include/libelfin)
target_link_libraries(monitor_lib PRIVATE elf++)
target_link_libraries(monitor_lib PRIVATE dwarf++)
