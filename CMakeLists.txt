cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
project(
  SmartFactory
  VERSION 0.9.0
  LANGUAGES CXX)

include(GNUInstallDirs)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_INSTALL_CMAKEDIR ${CMAKE_INSTALL_LIBDIR}/cmake/${CMAKE_PROJECT_NAME})

# ==============================================================================
# fetch external content
include(FetchContent)
# ------------------------------------------------------------------------------
# cmake_tools
FetchContent_Declare(
  cmake_tools
  GIT_REPOSITORY https://github.com/rlalik/cmake_tools
  GIT_TAG master)

FetchContent_MakeAvailable(cmake_tools)
list(APPEND CMAKE_MODULE_PATH ${cmake_tools_SOURCE_DIR})
include(check_3rd_party_tool)
# ------------------------------------------------------------------------------
# cmake - scripts
check_3rd_party_tool(
  cmake-scripts 9999 https://github.com/StableCoder/cmake-scripts TAG
  774a07ece3945b4f72e3ddc11646ab7cc9a349e2 AUTO)

list(APPEND CMAKE_MODULE_PATH ${cmake-scripts_SOURCE_DIR})
# include(c++-standards)
include(code-coverage)
include(sanitizers)
# include(doxygen) include(formatting) include(tools)
# ==============================================================================

# find ROOT
list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})
find_package(ROOT REQUIRED COMPONENTS Gpad)
include(${ROOT_USE_FILE})

add_library(${CMAKE_PROJECT_NAME} SHARED src/SmartFactory.cxx)
add_library(RT::${CMAKE_PROJECT_NAME} ALIAS ${CMAKE_PROJECT_NAME})

target_include_directories(
  ${CMAKE_PROJECT_NAME}
  PUBLIC $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
  PRIVATE ${CMAKE_SOURCE_DIR}/inc)

if(ROOT_VERSION VERSION_LESS 6)
  root_generate_dictionary(G__${CMAKE_PROJECT_NAME}_cc SmartFactory.h LINKDEF
                           LinkDef.h)
  target_sources(${CMAKE_PROJECT_NAME} PRIVATE G__${CMAKE_PROJECT_NAME}_cc)
  target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC Gpad)
else()
  root_generate_dictionary(G__${CMAKE_PROJECT_NAME}_cc SmartFactory.h MODULE
                           ${CMAKE_PROJECT_NAME} LINKDEF LinkDef.h)
  target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC ROOT::Gpad)

endif()

set_target_properties(
  ${CMAKE_PROJECT_NAME}
  PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR}
             VERSION ${PROJECT_VERSION}
             PUBLIC_HEADER "inc/SmartFactory.h")

install(
  TARGETS ${CMAKE_PROJECT_NAME}
  EXPORT ${CMAKE_PROJECT_NAME}Targets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Export the package for use from the build-tree (this registers the build-tree
# with a global CMake-registry)
export(PACKAGE ${CMAKE_PROJECT_NAME})

install(
  EXPORT ${CMAKE_PROJECT_NAME}Targets
  FILE ${CMAKE_PROJECT_NAME}Targets.cmake
  NAMESPACE RT::
  DESTINATION ${CMAKE_INSTALL_CMAKEDIR}
  COMPONENT dev)

include(CMakePackageConfigHelpers)

configure_package_config_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}Config.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_CMAKEDIR}
  # PATH_VARS CMAKE_INSTALL_INCLUDEDIR CMAKE_INSTALL_LIBDIR CMAKE_INSTALL_BINDIR
)

write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}ConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion)

export(
  EXPORT ${CMAKE_PROJECT_NAME}Targets
  FILE ${CMAKE_CURRENT_BINARY_DIR}/cmake/${CMAKE_PROJECT_NAME}Targets.cmake
  NAMESPACE ${CMAKE_PROJECT_NAME}::)

install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}Config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}ConfigVersion.cmake
  DESTINATION ${CMAKE_INSTALL_CMAKEDIR})

add_subdirectory(examples)
enable_testing()
add_subdirectory(tests)

# messages
message(
  STATUS
    "<<< Configuration >>>
Project:        ${CMAKE_PROJECT_NAME}

Architecture:   ${CMAKE_LIBRARY_ARCHITECTURE}
Build type      ${CMAKE_BUILD_TYPE}
Install path    ${CMAKE_INSTALL_PREFIX}

Compiler:
C               ${CMAKE_C_COMPILER}
C++             ${CMAKE_CXX_COMPILER}

Linker:
Ld              ${CMAKE_LINKER}

Compiler flags:
C               ${CMAKE_C_FLAGS}
C++             ${CMAKE_CXX_FLAGS}

Linker flags:
Executable      ${CMAKE_EXE_LINKER_FLAGS}
Module          ${CMAKE_MODULE_LINKER_FLAGS}
Shared          ${CMAKE_SHARED_LINKER_FLAGS}")

foreach(p LIB BIN INCLUDE CMAKE)
  message(STATUS "CMAKE_INSTALL_${p}DIR: ${CMAKE_INSTALL_${p}DIR}")
endforeach()
