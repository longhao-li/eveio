aux_source_directory(. EVEIO_SRC)

add_library(eveio STATIC ${EVEIO_SRC})

target_link_libraries(
  eveio
  spdlog::spdlog
  fmt::fmt
  ${CMAKE_THREAD_LIBS_INIT}
)
