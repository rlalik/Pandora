set(INSTALL_GTEST OFF)
find_or_fetch_package(GTest https://github.com/google/googletest.git
                      VERSION 1.12.0  GIT_TAG release-1.12.0 AUTO)

# configure_file(tests_config.h.in tests_config.h)

add_executable(gtests basic_tests.cpp copy_move_tests.cpp)

if(TARGET GTest::gtest_main)
  # new cmake provided
  set(GTEST_TRG GTest::gtest_main)
else()
  # old cmake and pc files
  set(GTEST_TRG gtest gtest_main)
endif()

target_link_libraries(gtests PRIVATE ${GTEST_TRG} pandora::pandora)

if(ENABLE_ADVANCE_TOOLS)
  target_code_coverage(gtests ALL)
endif()

gtest_discover_tests(gtests)
