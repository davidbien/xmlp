# CMakeList.txt : CMake project for xmlp, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

include ( bienutil/bienutil.cmake )

project ("xmlp")

# Add source to this project's executable.
add_executable (xmlp "xmlpgen.cpp")

#setup internal submodule include directories.
target_include_directories(jsonpp_cmake PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/bienutil>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/dgraph>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lexang>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/xmlpinc>
)
