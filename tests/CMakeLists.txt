aux_source_directory(./ SRC_LIST)

add_executable(test test.cc)
add_dependencies(test sylar)
target_link_libraries(test sylar yaml-cpp)

add_executable(test_config test_config.cc)
add_dependencies(test_config sylar)
target_link_libraries(test_config sylar yaml-cpp)

add_executable(my_test my_test.cc)