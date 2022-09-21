check_3rd_party_tool(googletest 1.12.0 https://github.com/google/googletest.git
                     TAG release-1.12.0 AUTO)
list(APPEND CMAKE_MODULE_PATH ${cmake-scripts_SOURCE_DIR})

find_package(GTest)

# configure_file(tests_config.h.in tests_config.h)

add_executable(gtests basic_tests.cpp copy_move_tests.cpp)

if(TARGET GTest::gtest_main)
  # new cmake provided
  set(GTEST_TRG GTest::gtest_main)
else()
  # old cmake and pc files
  set(GTEST_TRG gtest gtest_main)
endif()

target_link_libraries(gtests PRIVATE ${GTEST_TRG} RT::Pandora)
target_code_coverage(gtests ALL)

gtest_discover_tests(gtests)
