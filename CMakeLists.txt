# CMakeList.txt : CMake project for xmlpgen, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)
include(FetchContent)
include(GoogleTest)

project ("xmlpgen")

#if(NOT googletest_POPULATED)
#  FetchContent_Populate(googletest)
#  add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})
#endif()

enable_testing()

include ( bienutil/bienutil.cmake )

# All test programs' JSON logs go in one directory, cleared at the start of each ctest run.
bienutil_ctest_log_dir( "${CMAKE_BINARY_DIR}/testlogs" )

set(GTEST_VERSION 1.14.0 CACHE STRING "Google test version")
FetchContent_Declare(googletest
GIT_REPOSITORY https://github.com/google/googletest.git
GIT_TAG v${GTEST_VERSION})
FetchContent_MakeAvailable(googletest)
include_directories("${gtest_SOURCE_DIR}/include")

# create an output directory where we will create all of our unittest files based on the template unittest files that are in the source tree.
add_custom_target(make-build-unittest-dir ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory "unittests")

# xmlpgen generates the lexical analyzers ( with lexang ) and the parser of the DTD ( with pgen ) - xmlpgen_dtd.cpp.
add_executable (xmlpgen "xmlpgen.cpp" "xmlpgen.h" "xmlpgen_utf32.cpp" "xmlpgen_utf16.cpp" "xmlpgen_utf8.cpp" "xmlpgen_dtd.cpp" )

target_include_directories(xmlpgen PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/bienutil>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/dgraph>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lexang>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/pgen>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/xmlpinc>
)
target_compile_definitions( xmlpgen
  PUBLIC __NDEBUG_THROW
)

add_custom_command(
    OUTPUT  _xmlplex_utf32.h _xmlplex_utf16.h _xmlplex_utf8.h _xmlpdtd.h
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

if(1)
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
if ( NOT ${XMLTEST} MATCHES ".+skip" )
get_filename_component( XMLTEST_NAME ${XMLTEST} NAME_WE )
message("Unit test: ${XMLTEST_NAME}")    
gtest_discover_tests(xmlpgtest_cursorcopy
                    EXTRA_ARGS  ${XMLTEST}
                    TEST_SUFFIX .${XMLTEST_NAME}
)
endif()
endforeach()
endif(1)

add_executable (xmlpgtest_doccopy xmlpgtest_doccopy.cpp ${CMAKE_CURRENT_BINARY_DIR}/_xmlplex_utf32.h ${CMAKE_CURRENT_BINARY_DIR}/_xmlplex_utf16.h ${CMAKE_CURRENT_BINARY_DIR}/_xmlplex_utf8.h )
target_include_directories(xmlpgtest_doccopy PUBLIC
  ${CMAKE_CURRENT_BINARY_DIR}
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/bienutil>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/dgraph>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lexang>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/xmlpinc>
)
target_link_libraries(xmlpgtest_doccopy gtest gtest_main)
target_compile_definitions( xmlpgtest_doccopy
  PUBLIC __NDEBUG_THROW
  PUBLIC TRACESENABLED=0
  # PUBLIC ASSERTSENABLED=1
)

# We may not notice new unittests until CMakeLists.txt is rebuilt - that's ok.
file(GLOB XMLUNITTESTS LIST_DIRECTORIES true unittests/*.xml )
foreach( XMLTEST ${XMLUNITTESTS} )
get_filename_component( XMLTEST_NAME ${XMLTEST} NAME_WE )
if ( NOT ${XMLTEST} MATCHES ".+skip" )
message("Unit test: ${XMLTEST_NAME}")    
gtest_discover_tests(xmlpgtest_doccopy
                    EXTRA_ARGS  ${XMLTEST}
                    TEST_SUFFIX .${XMLTEST_NAME}
)
endif()
endforeach()

# Malformed-input testing of the lexical analyzer's triggers - see the comments in xmlpgtest_malformed.cpp.
add_executable (xmlpgtest_malformed xmlpgtest_malformed.cpp ${CMAKE_CURRENT_BINARY_DIR}/_xmlplex_utf32.h ${CMAKE_CURRENT_BINARY_DIR}/_xmlplex_utf16.h ${CMAKE_CURRENT_BINARY_DIR}/_xmlplex_utf8.h )
target_include_directories(xmlpgtest_malformed PUBLIC
  ${CMAKE_CURRENT_BINARY_DIR}
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/bienutil>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/dgraph>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lexang>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/xmlpinc>
)
target_link_libraries(xmlpgtest_malformed gtest)
target_compile_definitions( xmlpgtest_malformed
  PUBLIC __NDEBUG_THROW
  PUBLIC TRACESENABLED=0
)
gtest_discover_tests(xmlpgtest_malformed)

# The DTD: the tokens of the DTD and the parser generated for it by xmlpgen - see the comments in xmlpgtest_dtd.cpp.
add_executable (xmlpgtest_dtd xmlpgtest_dtd.cpp ${CMAKE_CURRENT_BINARY_DIR}/_xmlplex_utf32.h ${CMAKE_CURRENT_BINARY_DIR}/_xmlplex_utf16.h ${CMAKE_CURRENT_BINARY_DIR}/_xmlplex_utf8.h ${CMAKE_CURRENT_BINARY_DIR}/_xmlpdtd.h )
target_include_directories(xmlpgtest_dtd PUBLIC
  ${CMAKE_CURRENT_BINARY_DIR}
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/bienutil>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/dgraph>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lexang>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/pgen>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/xmlpinc>
)
target_link_libraries(xmlpgtest_dtd gtest)
target_compile_definitions( xmlpgtest_dtd
  PUBLIC __NDEBUG_THROW
  PUBLIC TRACESENABLED=0
)
gtest_discover_tests(xmlpgtest_dtd)

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

# dgraph builds its own unit tests - they are run by ctest along with ours.
add_subdirectory(dgraph)

# lexang builds its own unit tests as well.
add_subdirectory(lexang)

# pgen ( the parser generator ) builds its own unit tests as well.
add_subdirectory(pgen)
