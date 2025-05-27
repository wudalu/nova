#pragma once

#include <vector>
#include <string>
#include <cstring>
#include <arpa/inet.h>  // 用于 htonl 和 ntohl
#include <nlohmann/json.hpp>
#include "packet.pb.h"  // 确保包含生成的Protobuf头文件
#include "proto/service.hpp"  // 包含服务接口

struct PacketHeader {
    uint32_t packet_len;
    uint32_t header_len;
};

class PacketHandler {
public:
    static constexpr uint32_t CURRENT_VERSION = 1;
    static constexpr size_t MAX_PACKET_SIZE = 1024 * 1024;  // 1MB

    PacketHandler(IPacketService& service) : service_(service) {}

    void processRequest(const std::string& request) {
        appendData(request);

        mypacket::Packet packet;
        while (getNextPacket(packet)) {
            try {
                handlePacket(packet);
            } catch (const std::exception& e) {
                std::cerr << "处理数据包错误: " << e.what() << std::endl;
            }
        }
    }

private:
    std::vector<char> buffer_;
    IPacketService& service_;  // 引用服务接口

    bool appendData(const std::string& data) {
        buffer_.insert(buffer_.end(), data.begin(), data.end());
        return true;
    }

    bool hasCompletePacket() const {
        return buffer_.size() >= sizeof(PacketHeader) &&
               buffer_.size() >= ntohl(*reinterpret_cast<const uint32_t*>(buffer_.data()));
    }

    bool getNextPacket(mypacket::Packet& packet) {
        if (!hasCompletePacket()) {
            return false;
        }

        PacketHeader header;
        std::memcpy(&header, buffer_.data(), sizeof(PacketHeader));
        header.packet_len = ntohl(header.packet_len);
        header.header_len = ntohl(header.header_len);

        if (header.packet_len > MAX_PACKET_SIZE) {
            // 包太大，可能是恶意数据
            buffer_.clear();
            return false;
        }

        if (buffer_.size() < header.packet_len) {
            return false;
        }

        // 解析 JSON header
        try {
            nlohmann::json json_header = nlohmann::json::parse(buffer_.begin() + sizeof(PacketHeader), 
                                                               buffer_.begin() + sizeof(PacketHeader) + header.header_len);
            packet.set_method(json_header["method"]);
        } catch (const nlohmann::json::parse_error& e) {
            // JSON 解析失败，丢弃这个包
            buffer_.erase(buffer_.begin(), buffer_.begin() + header.packet_len);
            return false;
        }

        // 获取原始 payload 数据
        packet.set_payload(std::string(buffer_.begin() + sizeof(PacketHeader) + header.header_len, 
                                       buffer_.begin() + header.packet_len));

        buffer_.erase(buffer_.begin(), buffer_.begin() + header.packet_len);
        return true;
    }

    // 当前将 method 封装在 header 里面是权宜之计，后续需要设法优化
    // 可能的优化方案包括：
    // 1. 使用更高效的序列化方式
    // 2. 将 method 信息放在更合适的位置
    // 3. 重新设计数据包结构以提高解析效率
    void handlePacket(const mypacket::Packet& packet) {
        mypacket::Request request;
        request.set_payload(packet.payload());

        // 根据解析出的 method 调用相应的服务方法
        if (packet.method() == "PROCESS_DATA") {
            Response response = service_.processData(request);
            // 处理 response
        } else if (packet.method() == "STREAM_DATA") {
            std::vector<mypacket::Request> requests = {request};
            std::vector<Response> responses;
            service_.streamData(requests, responses);
            // 处理 responses
        } else if (packet.method() == "VALIDATE_PACKET") {
            Response response = service_.validatePacket(request);
            // 处理 response
        } else {
            std::cerr << "Unknown method: " << packet.method() << std::endl;
        }
    }
};
