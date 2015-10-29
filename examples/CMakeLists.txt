add_executable(example_write example_write.cpp)
target_link_libraries(example_write ${ROOT_LIBRARIES} SmartFactory)

add_executable(example_read example_read.cpp)
target_link_libraries(example_read ${ROOT_LIBRARIES} SmartFactory)

add_executable(example_multi_read example_multi_read.cpp)
target_link_libraries(example_multi_read ${ROOT_LIBRARIES} SmartFactory)