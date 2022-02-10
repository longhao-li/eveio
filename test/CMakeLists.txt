enable_testing()

find_package(GTest)
if(NOT GTest_FOUND)
  message(STATUS "GTest not found, downloading from GitHub...")
  # Download from GitHub
  include(FetchContent)
  FetchContent_Declare(
    googletest
    GIT_REPOSITORY "https://github.com/google/googletest.git"
    GIT_TAG "main"
  )
  FetchContent_MakeAvailable(googletest)
endif()

# Unit test
aux_source_directory(UnitTest EVEIO_UNIT_TEST_SRC)
add_executable(eveioUnitTest ${EVEIO_UNIT_TEST_SRC})

target_include_directories(
  eveioUnitTest
  PRIVATE
  ${GTEST_INCLUDE_DIRS}
)

target_link_libraries(
  eveioUnitTest
  eveio
  GTest::gtest
  GTest::gtest_main
  Threads::Threads
)

add_test(
  NAME eveioUnitTest
  COMMAND eveioUnitTest
)

# TcpEchoServer
add_executable(
  eveioTcpEchoServer
  TcpEchoServer.cpp
)

target_link_libraries(
  eveioTcpEchoServer
  eveio
  Threads::Threads
)

# PingPong
add_executable(
  eveioPingPong
  PingPong.cpp
)

target_link_libraries(
  eveioPingPong
  eveio
  Threads::Threads
)
