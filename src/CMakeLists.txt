aux_source_directory(. EVEIO_SRC)

add_library(eveio STATIC ${EVEIO_SRC})

target_link_libraries(
  eveio
  PUBLIC
  spdlog::spdlog
  fmt::fmt
  mimalloc-static
  ${CMAKE_THREAD_LIBS_INIT}
)
