add_executable(test_push_back push_back.cc)

add_executable(test_simple_allocation simple_allocation.cc)
target_link_libraries(test_simple_allocation fmt::fmt)