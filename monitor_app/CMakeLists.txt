add_executable(monitor 
    main.cc
)

target_link_libraries(monitor monitor_lib)
target_link_libraries(monitor fmt)

