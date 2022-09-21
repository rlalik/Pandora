add_executable(example_write example_write.cpp)
target_link_libraries(example_write RT::Pandora)

add_executable(example_read example_read.cpp)
target_link_libraries(example_read RT::Pandora)

add_executable(example_multi_read example_multi_read.cpp)
target_link_libraries(example_multi_read RT::Pandora)
