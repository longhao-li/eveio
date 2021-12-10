cmake_minimum_required(VERSION 3.11)

project(eveio VERSION 0.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()

option(EVEIO_BUILD_TEST "Build tests" ON)

set(
  EVEIO_CXX_FLAGS
  -Wall
  -Werror
  -Wconversion
  -Wno-unused-parameter
  -Wold-style-cast
  -Woverloaded-virtual
  -Wpointer-arith
  -Wshadow
  -Wwrite-strings
)

if(CMAKE_BUILD_BITS EQUAL 32)
  list(APPEND EVEIO_CXX_FLAGS "-m32")
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  list(APPEND EVEIO_CXX_FLAGS "-Wno-null-dereference")
  list(APPEND EVEIO_CXX_FLAGS "-Wno-sign-conversion")
  list(APPEND EVEIO_CXX_FLAGS "-Wno-unused-local-typedef")
  list(APPEND EVEIO_CXX_FLAGS "-Wthread-safety")
endif()
string(REPLACE ";" " " CMAKE_CXX_FLAGS "${EVEIO_CXX_FLAGS}")

set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)
find_package(Threads REQUIRED)
find_package(mimalloc REQUIRED)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

add_subdirectory(src)

if(EVEIO_BUILD_TEST)
  find_package(GTest)
  add_subdirectory(test)
endif()

message("eveio version: ${eveio_VERSION}")
message("cxx compiler: ${CMAKE_CXX_COMPILER}")
message("cxx flags: ${CMAKE_CXX_FLAGS}")
message("build tests: ${EVEIO_BUILD_TEST}")
message("install prefix: ${CMAKE_INSTALL_PREFIX}")
