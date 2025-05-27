cmake_minimum_required(VERSION 3.14)

# 设置工程名称
project(NOVA)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 设置 Protocol Buffers 路径
set(PROTOBUF_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/3rd/protobuf)
set(Protobuf_INCLUDE_DIRS ${PROTOBUF_ROOT}/include)
set(Protobuf_LIBRARIES ${PROTOBUF_ROOT}/lib/libprotobuf.a)
set(Protobuf_PROTOC_EXECUTABLE ${PROTOBUF_ROOT}/bin/protoc)

# 查找Python库
find_package(pybind11 REQUIRED)

# 生成 Protocol Buffers 代码
set(PROTO_FILES packet.proto)
set(GENERATED_PROTO_FILES "")

foreach(PROTO_FILE ${PROTO_FILES})
    get_filename_component(PROTO_NAME ${PROTO_FILE} NAME_WE)
    list(APPEND GENERATED_PROTO_FILES 
        ${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.pb.cc
        ${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.pb.h
    )
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.pb.cc
               ${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.pb.h
        COMMAND ${Protobuf_PROTOC_EXECUTABLE}
        ARGS --cpp_out=${CMAKE_CURRENT_BINARY_DIR}
             -I${CMAKE_CURRENT_SOURCE_DIR}
             ${CMAKE_CURRENT_SOURCE_DIR}/${PROTO_FILE}
        DEPENDS ${PROTO_FILE}
        COMMENT "Generating ${PROTO_NAME} protocol buffer files"
    )
endforeach()

# 添加源文件
set(SOURCES
    main.cc
    script/python.cpp  # 添加 Python 嵌入代码
    ${PROJECT_SOURCE_DIR}/3rd/kcp/ikcp.c  # 添加 KCP 源文件
    ${GENERATED_PROTO_FILES}
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
    ${Protobuf_INCLUDE_DIRS}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/proto
)

# 链接库
target_link_libraries(NOVA PRIVATE 
    pybind11::embed
    ${Protobuf_LIBRARIES}
)

# 调试输出
message(STATUS "LLVM_DIR: ${LLVM_DIR}")
message(STATUS "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
message(STATUS "CMAKE_EXE_LINKER_FLAGS: ${CMAKE_EXE_LINKER_FLAGS}")
message(STATUS "Protobuf_INCLUDE_DIRS: ${Protobuf_INCLUDE_DIRS}")
message(STATUS "Protobuf_LIBRARIES: ${Protobuf_LIBRARIES}")
message(STATUS "Protobuf_PROTOC_EXECUTABLE: ${Protobuf_PROTOC_EXECUTABLE}")

