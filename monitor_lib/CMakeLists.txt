file(GLOB SOURCES *.cc)
file(GLOB HEADERS *.hh)

add_library(monitor_lib
    ${SOURCES} ${HEADERS}
)

target_include_directories(monitor_lib INTERFACE ..)

target_link_libraries(monitor_lib PUBLIC libdwarf::dwarf)

target_link_libraries(monitor_lib PRIVATE fmt::fmt)
target_link_libraries(monitor_lib PRIVATE Boost::filesystem)
target_link_libraries(monitor_lib PRIVATE Boost::scope_exit)
