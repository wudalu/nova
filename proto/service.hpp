#pragma once

#include "packet.pb.h"

class IPacketService {
public:
    virtual ~IPacketService() = default;

    virtual Response processData(const mypacket::Request& request) = 0;
    virtual void streamData(const std::vector<mypacket::Request>& requests, std::vector<Response>& responses) = 0;
    virtual Response validatePacket(const mypacket::Request& request) = 0;
};

class PacketServiceImpl : public IPacketService {
public:
    Response processData(const mypacket::Request& request) override {
        // 实现处理数据的逻辑
        Response response;
        // 设置 response 的数据
        return response;
    }

    void streamData(const std::vector<mypacket::Request>& requests, std::vector<Response>& responses) override {
        // 实现处理流数据的逻辑
        for (const auto& request : requests) {
            Response response;
            // 设置 response 的数据
            responses.push_back(response);
        }
    }

    Response validatePacket(const mypacket::Request& request) override {
        // 实现验证数据包的逻辑
        Response response;
        // 设置 response 的数据
        return response;
    }
};
