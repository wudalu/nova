cmake_minimum_required(VERSION 3.14)

# 设置工程名称
project(NOVA)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 查找Python库
find_package(pybind11 REQUIRED)

# 添加源文件
set(SOURCES
    main.cc
    script/python.cpp  # 添加 Python 嵌入代码
    ${PROJECT_SOURCE_DIR}/3rd/kcp/ikcp.c  # 添加 KCP 源文件
    # 如果有其他源文件，请在此添加
)

# 生成可执行文件
add_executable(NOVA ${SOURCES})

# 添加包含目录
target_include_directories(NOVA PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/thread
    ${PROJECT_SOURCE_DIR}/network
    ${PROJECT_SOURCE_DIR}/3rd/kcp
    ${PROJECT_SOURCE_DIR}/3rd
    ${PROJECT_SOURCE_DIR}/3rd/pybind11  # 添加本地 pybind11 头文件目录
    ${Python3_INCLUDE_DIRS}  # 添加 Python 头文件目录
)

# 链接库
target_link_libraries(NOVA PRIVATE pybind11::embed)

# 调试输出
message(STATUS "LLVM_DIR: ${LLVM_DIR}")
message(STATUS "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
message(STATUS "CMAKE_EXE_LINKER_FLAGS: ${CMAKE_EXE_LINKER_FLAGS}")

