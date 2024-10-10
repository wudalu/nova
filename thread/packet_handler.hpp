#pragma once

#include <vector>
#include <string>
#include <cstring>
#include "nlohmann/json.hpp"

struct Packet {
    int32_t packet_len;
    int32_t header_len;
    nlohmann::json header;
    nlohmann::json payload;
};

class PacketHandler {
public:
    bool append_data(const std::string& data) {
        buffer_.insert(buffer_.end(), data.begin(), data.end());
        return true;
    }

    bool has_complete_packet() const {
        return buffer_.size() >= 8 && buffer_.size() >= *reinterpret_cast<const int32_t*>(buffer_.data());
    }

    bool get_next_packet(Packet& packet) {
        if (!has_complete_packet()) {
            return false;
        }

        std::memcpy(&packet.packet_len, buffer_.data(), sizeof(int32_t));
        std::memcpy(&packet.header_len, buffer_.data() + sizeof(int32_t), sizeof(int32_t));

        size_t offset = 2 * sizeof(int32_t);
        std::string header_str(buffer_.data() + offset, buffer_.data() + offset + packet.header_len);
        packet.header = nlohmann::json::parse(header_str);

        offset += packet.header_len;
        std::string payload_str(buffer_.data() + offset, buffer_.data() + packet.packet_len);
        packet.payload = nlohmann::json::parse(payload_str);

        buffer_.erase(buffer_.begin(), buffer_.begin() + packet.packet_len);

        return true;
    }

    static std::string pack_data(const Packet& packet) {
        std::string header_str = packet.header.dump();
        std::string payload_str = packet.payload.dump();

        int32_t packet_len = 2 * sizeof(int32_t) + header_str.length() + payload_str.length();
        int32_t header_len = header_str.length();

        std::string result;
        result.resize(packet_len);

        char* ptr = result.data();
        std::memcpy(ptr, &packet_len, sizeof(int32_t));
        ptr += sizeof(int32_t);
        std::memcpy(ptr, &header_len, sizeof(int32_t));
        ptr += sizeof(int32_t);
        std::memcpy(ptr, header_str.data(), header_str.length());
        ptr += header_str.length();
        std::memcpy(ptr, payload_str.data(), payload_str.length());

        return result;
    }

private:
    std::vector<char> buffer_;
};
