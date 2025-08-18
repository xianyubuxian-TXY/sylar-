aux_source_directory(. LOG_LIST)

# 生成动态库sylar
add_library(sylar SHARED ${LOG_LIST})

