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

add_executable (xmlpgen "xmlpgen.cpp")

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
)

if (0)
add_executable (xmlgtest xmlgtest.cpp ${CMAKE_CURRENT_BINARY_DIR}/_xmlplex.h )
target_include_directories(xmlgtest PUBLIC
  ${CMAKE_CURRENT_BINARY_DIR}
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/bienutil>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/dgraph>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lexang>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/xmlpinc>
)
target_link_libraries(xmlgtest gtest gtest_main)

gtest_discover_tests(xmlgtest
                    EXTRA_ARGS  --someArg someValue
                    TEST_SUFFIX .withArgs
                    TEST_LIST   withArgsTests
)
endif (0)
