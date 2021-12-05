enable_testing()

aux_source_directory(unittest EVEIO_UNITTEST_SRC)
add_executable(eveio_unittest ${EVEIO_UNITTEST_SRC})
target_link_libraries(
  eveio_unittest
  eveio
  ${GTEST_MAIN_LIBRARIES}
)
add_test(NAME eveio_unittest COMMAND eveio_unittest)
