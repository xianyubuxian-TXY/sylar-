aux_source_directory(./ SRC_LIST)

add_executable(test ${SRC_LIST})
add_dependencies(test sylar)

target_link_libraries(test sylar)