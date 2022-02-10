cmake_minimum_required(VERSION 3.11)

project(eveio VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()

option(EVEIO_BUILD_TEST "Build tests" ON)

if(MSVC)
  # TODO: To be written
else()
  set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wold-style-cast -Wshadow")
  if(CMAKE_BUILD_BITS EQUAL 32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
  endif()
  if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-null-dereference")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-sign-conversion")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-local-typedef")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wthread-safety")
  endif()
endif()

# Fetch external packages
find_package(Threads REQUIRED)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

add_subdirectory(src)

if(EVEIO_BUILD_TEST)
  add_subdirectory(test)
endif()

message("eveio version: ${eveio_VERSION}")
message("cxx compiler: ${CMAKE_CXX_COMPILER}")
message("cxx flags: ${CMAKE_CXX_FLAGS}")
message("install prefix: ${CMAKE_INSTALL_PREFIX}")
