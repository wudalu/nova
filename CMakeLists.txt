cmake_minimum_required(VERSION 3.10)

# 设置工程名称
project(NET_FRAMEWORK)

# LLVM 设置保持不变

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


# 添加源文件
set(SOURCES
    main.cc
    ${PROJECT_SOURCE_DIR}/3rd/kcp/ikcp.c  # 添加 KCP 源文件
    # 如果有其他源文件，请在此添加
)

# 生成可执行文件
add_executable(NET_FRAMEWORK ${SOURCES})

# 添加包含目录
target_include_directories(NET_FRAMEWORK PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/thread
    ${PROJECT_SOURCE_DIR}/network
    ${PROJECT_SOURCE_DIR}/3rd/kcp
    ${PROJECT_SOURCE_DIR}/3rd
)


# 调试输出
message(STATUS "LLVM_DIR: ${LLVM_DIR}")
message(STATUS "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
message(STATUS "CMAKE_EXE_LINKER_FLAGS: ${CMAKE_EXE_LINKER_FLAGS}")

