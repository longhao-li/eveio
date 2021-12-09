enable_testing()

aux_source_directory(unittest EVEIO_UNITTEST_SRC)
add_executable(eveio_unittest ${EVEIO_UNITTEST_SRC})
target_link_libraries(
  eveio_unittest
  eveio
  ${GTEST_BOTH_LIBRARIES}
)
add_test(NAME eveio_unittest COMMAND eveio_unittest)

add_executable(eveio_TcpEchoServer TcpEchoServer.cpp)
target_link_libraries(eveio_TcpEchoServer eveio)

add_executable(eveio_UdpEchoServer UdpEchoServer.cpp)
target_link_libraries(eveio_UdpEchoServer eveio)

add_executable(eveio_echo PingPong.cpp)
target_link_libraries(eveio_echo eveio)
