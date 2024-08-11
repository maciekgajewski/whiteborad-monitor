add_executable(monitor 
    main.cc
)

target_link_libraries(monitor PRIVATE monitor_lib)
target_link_libraries(monitor PRIVATE fmt::fmt)

