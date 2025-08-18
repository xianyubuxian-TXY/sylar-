cmake_minimum_required(VERSION 3.10)
project(sylar)

#设置编译选项
set(CMAKE_VERBOSE_MAKEFILE ON)#CMake 在生成的 Makefile 中启用详细模式,打印出每条编译命令
#设置 C++ 编译器的参数（CXX = C++）
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O0 -g -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function")

#设置头文件搜索路径
include_directories(${PROJECT_SOURCE_DIR}/sylar)
#设置库文件搜索路径
link_directories(${PROJECT_SOURCE_DIR}/lib)


#项目可执行文件输出的路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
#项目库文件输出的路径
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)


add_subdirectory(sylar)
add_subdirectory(tests)