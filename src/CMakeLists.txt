aux_source_directory(. EVEIO_SRC)

# shared library configuration
add_library(eveio SHARED ${EVEIO_SRC})

target_include_directories(
  eveio PUBLIC ${eveio_SOURCE_DIR}/include
)

target_link_libraries(
  eveio
  PUBLIC
  Threads::Threads
)

set_target_properties(
  eveio
  PROPERTIES 
  VERSION ${eveio_VERSION}
  SOVERSION ${eveio_VERSION_MAJOR}
  CLEAN_DIRECT_OUTPUT 1
)

# static library configuration
add_library(eveio_static STATIC ${EVEIO_SRC})

target_include_directories(
  eveio_static PUBLIC ${eveio_SOURCE_DIR}/include
)

target_link_libraries(
  eveio_static
  PUBLIC
  Threads::Threads
)

set_target_properties(
  eveio_static 
  PROPERTIES 
  OUTPUT_NAME "eveio"
  CLEAN_DIRECT_OUTPUT 1
)

# install configuration
install(
  TARGETS eveio eveio_static
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
)

install(
  DIRECTORY ${eveio_SOURCE_DIR}/include/eveio
  DESTINATION include
)
