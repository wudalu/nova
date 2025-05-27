#!/bin/bash

# 设置变量
PROTOBUF_VERSION="3.20.3"  # 选择一个稳定版本
INSTALL_DIR="$(pwd)/../3rd/protobuf"

# 创建安装目录
mkdir -p $INSTALL_DIR

# 下载 Protocol Buffers
curl -OL "https://github.com/protocolbuffers/protobuf/releases/download/v${PROTOBUF_VERSION}/protobuf-cpp-${PROTOBUF_VERSION}.tar.gz"

# 解压
tar -xzf "protobuf-cpp-${PROTOBUF_VERSION}.tar.gz"

# 编译和安装
cd "protobuf-${PROTOBUF_VERSION}"
./configure --prefix=$INSTALL_DIR
make -j$(nproc)
make install

# 清理
cd ..
rm -rf "protobuf-${PROTOBUF_VERSION}" "protobuf-cpp-${PROTOBUF_VERSION}.tar.gz"

echo "Protocol Buffers ${PROTOBUF_VERSION} has been installed to ${INSTALL_DIR}"