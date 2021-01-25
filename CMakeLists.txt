# CMakeList.txt : CMake project for xmlpgen, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("xmlpgen")

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
    OUTPUT  _xmlplex.h
    COMMAND xmlpgen
    DEPENDS xmlpgen
)

add_executable (lexer lexer.cpp ${CMAKE_CURRENT_BINARY_DIR}/_xmlplex.h )
#setup internal submodule include directories.
target_include_directories(lexer PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/bienutil>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/dgraph>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lexang>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/xmlpinc>
  ${CMAKE_CURRENT_BINARY_DIR}
)
target_compile_definitions( lexer
  PUBLIC __NDEBUG_THROW
)

add_executable (xmltestsame xmltestsame.cpp ${CMAKE_CURRENT_BINARY_DIR}/_xmlplex.h )
#setup internal submodule include directories.
target_include_directories(xmltestsame PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/bienutil>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/dgraph>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lexang>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/xmlpinc>
  ${CMAKE_CURRENT_BINARY_DIR}
)
target_compile_definitions( xmltestsame
  PUBLIC __NDEBUG_THROW
)
