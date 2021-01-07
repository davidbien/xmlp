# CMakeList.txt : CMake project for xmlp, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("xmlp")

include ( bienutil/bienutil.cmake )

add_executable (xmlp "xmlpgen.cpp")

target_include_directories(xmlp PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/bienutil>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/dgraph>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lexang>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/xmlpinc>
)
target_compile_definitions( xmlp
  PUBLIC __NDEBUG_THROW
)
