# echo server
add_executable(eveio_echo echo.cpp)
target_include_directories(
    eveio_echo PUBLIC ${eveio_SOURCE_DIR}/include
)

target_link_libraries(
    eveio_echo
    PUBLIC
    eveio
    Threads::Threads
)

# Tcp Server
add_executable(eveio_tcp_server tcp_server.cpp)
target_include_directories(
    eveio_tcp_server PUBLIC ${eveio_SOURCE_DIR}/include
)

target_link_libraries(
    eveio_tcp_server
    PUBLIC
    eveio
    Threads::Threads
)
