cmake_minimum_required(VERSION 3.11)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

project(eveio VERSION 2.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wshadow -pedantic -Wreturn-type")
    if(CMAKE_CXX_COMPILER_ID STREQUAL Clang OR 
        CMAKE_CXX_COMPILER_ID STREQUAL AppleClang OR 
        CMAKE_CXX_COMPILER_ID STREQUAL GNU)
      set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=undefined,thread")
    endif()
endif()

find_package(Threads REQUIRED)

add_subdirectory(src)
add_subdirectory(example)
