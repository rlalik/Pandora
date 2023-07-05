cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
project(
  Pandora
  VERSION 0.9.4
  LANGUAGES CXX)

include(GNUInstallDirs)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

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
include(find_or_fetch_package)
# ------------------------------------------------------------------------------
# cmake - scripts

# conditional modules
option(ENABLE_ADVANCE_TOOLS "Enables advanced tools" OFF)
if(ENABLE_ADVANCE_TOOLS)
  FetchContent_Declare(
    cmake-scripts
    GIT_REPOSITORY https://github.com/StableCoder/cmake-scripts
    GIT_TAG main)
  FetchContent_GetProperties(cmake-scripts)
  if(NOT cmake-scripts_POPULATED)
    FetchContent_Populate(cmake-scripts)
    list(APPEND CMAKE_MODULE_PATH ${cmake-scripts_SOURCE_DIR})
    message(STATUS "${cmake-scripts_SOURCE_DIR}")
  endif()
  include(sanitizers)
  include(code-coverage)
  include(tools)
  include_what_you_use(-Xiwyu;--error_always)
endif()

# find ROOT
list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})
find_package(ROOT REQUIRED COMPONENTS Gpad)

# ==============================================================================
# general
# if(NOT DEFINED CMAKE_CXX_VISIBILITY_PRESET AND NOT DEFINED
#                                                CMAKE_VISIBILITY_INLINES_HIDDEN)
#   set(CMAKE_CXX_VISIBILITY_PRESET hidden)
#   set(CMAKE_VISIBILITY_INLINES_HIDDEN YES)
# endif()
# 
# if(DEFINED Pandora_SHARED_LIBS)
#   set(BUILD_SHARED_LIBS "${Pandora_SHARED_LIBS}")
# endif()

# ==============================================================================
# build project

include(shared_or_static)
shared_or_static(Pandora)
add_library(pandora ${Pandora_LIBRARY_TYPE} source/pandora.cpp)
add_library(pandora::pandora ALIAS pandora)

set_target_properties(
  pandora
  PROPERTIES VERSION ${PROJECT_VERSION}
             SOVERSION ${PROJECT_VERSION_MAJOR}
             PUBLIC_HEADER "include/pandora.hpp")

target_include_directories(
  pandora
  PUBLIC $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
         $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
         $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/inc>
  PRIVATE ${CMAKE_SOURCE_DIR}/inc
)

if(ROOT_VERSION VERSION_LESS 6)
  include(Root5to6Comp)
  include_directories(${CMAKE_BINARY_DIR}/include)
  root_generate_dictionary(G__Pandora_cc pandora.hpp LINKDEF LinkDef.h)
  target_sources(Pandora PRIVATE G__Pandora_cc)
else()
  root_generate_dictionary(G__Pandora_cc pandora.hpp MODULE pandora LINKDEF
                           LinkDef.h)
endif()

target_link_libraries(pandora PUBLIC ROOT::Gpad)

# target export

include(GenerateExportHeader)
generate_export_header(pandora EXPORT_FILE_NAME include/Pandora/export.h)
target_compile_definitions(
  pandora PUBLIC "$<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:PANDORA_STATIC_DEFINE>")
target_include_directories(
  pandora PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>")

# installing

if(NOT DEFINED Pandora_INSTALL_CMAKEDIR)
  set(Pandora_INSTALL_CMAKEDIR
      "${CMAKE_INSTALL_LIBDIR}/cmake/Pandora"
      CACHE STRING "Path to Pandora CMake files")
endif()

install(
  TARGETS pandora
  EXPORT Pandora_Targets
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
          COMPONENT Libraries
          NAMELINK_COMPONENT Development
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  INCLUDES
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} COMPONENT Development)

install(
  DIRECTORY ${Pandora_BINARY_DIR}/include/
  TYPE INCLUDE
  COMPONENT Development)

# Export the package for use from the build-tree (this registers the build-tree
# with a global CMake-registry)
export(PACKAGE Pandora)

if(BUILD_SHARED_LIBS)
  set(type shared)
else()
  set(type static)
endif()

install(
  EXPORT Pandora_Targets
  FILE Pandora-${type}-targets.cmake
  NAMESPACE pandora::
  DESTINATION ${Pandora_INSTALL_CMAKEDIR}
  COMPONENT dev)

include(CMakePackageConfigHelpers)

configure_package_config_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/PandoraConfig.cmake
  INSTALL_DESTINATION ${Pandora_INSTALL_CMAKEDIR})

write_basic_package_version_file(PandoraConfigVersion.cmake
                                 COMPATIBILITY SameMajorVersion)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/PandoraConfig.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/PandoraConfigVersion.cmake
        DESTINATION ${Pandora_INSTALL_CMAKEDIR})

add_subdirectory(examples)

# tests
enable_testing()
add_subdirectory(tests)

# messages
message(
  STATUS
    "<<< Configuration >>>
Project:        Pandora

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
