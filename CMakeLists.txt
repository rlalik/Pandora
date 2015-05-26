cmake_minimum_required(VERSION 2.6)
project(SmartFactory)

# set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/Modules" ${CMAKE_MODULE_PATH})
# find_package(ROOT)

list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})
find_package(ROOT REQUIRED COMPONENTS Core Cling RIO Net Hist Graf Graf3d Gpad Tree Rint Postscript Matrix Physics MathCore Thread)
include(${ROOT_USE_FILE})

# The version number.
set (library_VERSION_MAJOR 0)
set (library_VERSION_MINOR 1)
set (library_VERSION_PATCH 1)

set (VERSION ${library_VERSION_MAJOR}.${library_VERSION_MINOR}.${library_VERSION_PATCH})

set(PRONAME SmartFactory)
##### find programs
include_directories(${ROOT_INCLUDE_DIR})
link_directories(${root_LIBRARIES_DIR})

set(library_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/SmartFactory.cpp) # ${CMAKE_CURRENT_SOURCE_DIR}/
set(library_HDRS ${CMAKE_CURRENT_SOURCE_DIR}/SmartFactory.h) #

set(library_DICT G__${PRONAME})
set(library_LINKDEF ${CMAKE_CURRENT_SOURCE_DIR}/LinkDef.h)

# ROOT_GENERATE_DICTIONARY(
# 	"${library_HDRS}" "${library_LINKDEF}"
# 	"${library_DICT}" "${INCLUE_DIRECTORIES}"
# )

ROOT_GENERATE_DICTIONARY(
	"${library_DICT}" "${library_HDRS}" LINKDEF "${library_LINKDEF}" "${INCLUE_DIRECTORIES}"
)

add_library(${PRONAME} SHARED ${library_SRCS} ${library_DICT})

string(REGEX REPLACE "/" "." ROOT_VERSION_DOT ${ROOT_VERSION})

target_link_libraries(${PRONAME} ${ROOT_LIBRARIES} ASImage)
set_target_properties(${PRONAME} PROPERTIES
	SOVERSION ${ROOT_VERSION_DOT}
	PUBLIC_HEADER SmartFactory.h)

#set_target_properties (tsetup PROPERTIES LINK_FLAGS ${root_LDFLAGS})

add_executable(example_write examples/example_write.cpp)
target_link_libraries(example_write SmartFactory ${ROOT_LIBRARIES})

add_executable(example_read examples/example_read.cpp)
target_link_libraries(example_read SmartFactory ${ROOT_LIBRARIES})

install(TARGETS ${PRONAME}
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	PUBLIC_HEADER DESTINATION include)

install(FILES
		${CMAKE_CURRENT_BINARY_DIR}/${library_DICT}_rdict.pcm
	DESTINATION lib)

# messages
MESSAGE(STATUS "<<< Configuration >>>
Project:	${CMAKE_PROJECT_NAME}

Build type	${CMAKE_BUILD_TYPE}
Install path	${CMAKE_INSTALL_PREFIX}

Compiler:
C		${CMAKE_C_COMPILER}
C++		${CMAKE_CXX_COMPILER}

Linker:
Ld		${CMAKE_LINKER}

Compiler flags:
C		${CMAKE_C_FLAGS}
C++		${CMAKE_CXX_FLAGS}

Linker flags:
Executable      ${CMAKE_EXE_LINKER_FLAGS}
Module          ${CMAKE_MODULE_LINKER_FLAGS}
Shared          ${CMAKE_SHARED_LINKER_FLAGS}\n")
