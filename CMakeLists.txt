# CMakeList.txt : CMake project for xmlpgen, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)
include(FetchContent)
include(GoogleTest)

project ("xmlpgen")

set(GTEST_VERSION 1.10.0 CACHE STRING "Google test version")
FetchContent_Declare(googletest
GIT_REPOSITORY https://github.com/google/googletest.git
GIT_TAG release-${GTEST_VERSION})
FetchContent_MakeAvailable(googletest)
include_directories("${gtest_SOURCE_DIR}/include")

#if(NOT googletest_POPULATED)
#  FetchContent_Populate(googletest)
#  add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})
#endif()

enable_testing()

include ( bienutil/bienutil.cmake )

# create an output directory where we will create all of our unittest files based on the template unittest files that are in the source tree.
add_custom_target(make-build-unittest-dir ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory "unittests")

add_executable (xmlpgen "xmlpgen.cpp" "xmlpgen.h" "xmlpgen_utf32.cpp" "xmlpgen_utf16.cpp" "xmlpgen_utf8.cpp" )

target_include_directories(xmlpgen PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/bienutil>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/dgraph>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lexang>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/xmlpinc>
)
target_compile_definitions( xmlpgen
  PUBLIC __NDEBUG_THROW
)

add_custom_command(
    OUTPUT  _xmlplex_utf32.h _xmlplex_utf16.h _xmlplex_utf8.h
    COMMAND xmlpgen
    DEPENDS xmlpgen
)

add_executable (lexer lexer.cpp ${CMAKE_CURRENT_BINARY_DIR}/_xmlplex_utf32.h ${CMAKE_CURRENT_BINARY_DIR}/_xmlplex_utf16.h ${CMAKE_CURRENT_BINARY_DIR}/_xmlplex_utf8.h )
#setup internal submodule include directories.
target_include_directories(lexer PUBLIC
  ${CMAKE_CURRENT_BINARY_DIR}
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/bienutil>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/dgraph>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lexang>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/xmlpinc>
)
target_compile_definitions( lexer
  PUBLIC __NDEBUG_THROW
)

add_executable (xmlpgtest_cursorcopy xmlpgtest_cursorcopy.cpp ${CMAKE_CURRENT_BINARY_DIR}/_xmlplex_utf32.h ${CMAKE_CURRENT_BINARY_DIR}/_xmlplex_utf16.h ${CMAKE_CURRENT_BINARY_DIR}/_xmlplex_utf8.h )
target_include_directories(xmlpgtest_cursorcopy PUBLIC
  ${CMAKE_CURRENT_BINARY_DIR}
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/bienutil>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/dgraph>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lexang>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/xmlpinc>
)
target_link_libraries(xmlpgtest_cursorcopy gtest gtest_main)
target_compile_definitions( xmlpgtest_cursorcopy
  PUBLIC __NDEBUG_THROW
  PUBLIC TRACESENABLED=0
  # PUBLIC ASSERTSENABLED=1
)

# We may not notice new unittests until CMakeLists.txt is rebuilt - that's ok.
file(GLOB XMLUNITTESTS LIST_DIRECTORIES true unittests/*.xml )
foreach( XMLTEST ${XMLUNITTESTS} )
get_filename_component( XMLTEST_NAME ${XMLTEST} NAME_WE )
message("Unit test: ${XMLTEST_NAME}")    
gtest_discover_tests(xmlpgtest_cursorcopy
                    EXTRA_ARGS  ${XMLTEST}
                    TEST_SUFFIX .${XMLTEST_NAME}
)
endforeach()

if(1)
add_executable (xmlgtest xmlgtest.cpp ${CMAKE_CURRENT_BINARY_DIR}/_xmlplex_utf32.h ${CMAKE_CURRENT_BINARY_DIR}/_xmlplex_utf16.h ${CMAKE_CURRENT_BINARY_DIR}/_xmlplex_utf8.h )
target_include_directories(xmlgtest PUBLIC
  ${CMAKE_CURRENT_BINARY_DIR}
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/bienutil>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/dgraph>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lexang>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/xmlpinc>
)
target_link_libraries(xmlgtest gtest gtest_main)
target_compile_definitions( xmlgtest
  PUBLIC __NDEBUG_THROW
  PUBLIC TRACESENABLED=0
  # PUBLIC ASSERTSENABLED=1
)

# We may not notice new unittests until CMakeLists.txt is rebuilt - that's ok.
file(GLOB XMLUNITTESTS LIST_DIRECTORIES true unittests/*.xml )
foreach( XMLTEST ${XMLUNITTESTS} )
get_filename_component( XMLTEST_NAME ${XMLTEST} NAME_WE )
message("Unit test: ${XMLTEST_NAME}")    
gtest_discover_tests(xmlgtest
                    EXTRA_ARGS  ${XMLTEST}
                    TEST_SUFFIX .${XMLTEST_NAME}
)
endforeach()
endif(1)

add_executable (xmltestsame xmltestsame.cpp ${CMAKE_CURRENT_BINARY_DIR}/_xmlplex_utf32.h ${CMAKE_CURRENT_BINARY_DIR}/_xmlplex_utf16.h ${CMAKE_CURRENT_BINARY_DIR}/_xmlplex_utf8.h )
#setup internal submodule include directories.
target_include_directories(xmltestsame PUBLIC
  ${CMAKE_CURRENT_BINARY_DIR}
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/bienutil>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/dgraph>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lexang>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/xmlpinc>
)
target_compile_definitions( xmltestsame
  PUBLIC __NDEBUG_THROW
  # PUBLIC TRACESENABLED=0
  # PUBLIC ASSERTSENABLED=0
)
