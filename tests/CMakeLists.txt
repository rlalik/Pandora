set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR})
find_package(cppunit)

file(GLOB UnitTests_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*_tests.cpp" )

add_executable(UnitTester test_runner.cpp ${UnitTests_SRCS})
target_link_libraries(UnitTester SmartFactory ${CPPUNIT_LIBRARIES})

foreach(test ${UnitTests_SRCS})
    file(READ ${test} test_file)
    string(REGEX MATCH "CPPUNIT_TEST_SUITE\(.([A-Za-z0-9 ]+).\)" _ ${test_file})
    string(STRIP ${CMAKE_MATCH_2} suite_name)
    get_filename_component(TestName ${test} NAME_WE)
    add_test(NAME ${suite_name} COMMAND UnitTester ${suite_name})
endforeach(test)
