add_executable(monitor 
    monitor.cc
    run.hh run.cc
)

target_link_libraries(monitor fmt)
