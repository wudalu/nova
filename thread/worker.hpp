#pragma once

#include "base.hpp"
#include "concurrentqueue/concurrentqueue.h"
#include "packet_handler.hpp"
#include <string>
#include <iostream>

class WorkerThread : public BaseThread {
public:
    explicit WorkerThread(moodycamel::ConcurrentQueue<std::string>& queue)
        : request_queue_(queue) {}

    void start() override {
        thread_ = std::thread(&WorkerThread::worker_thread_func, this);
    }

private:
    moodycamel::ConcurrentQueue<std::string>& request_queue_;
    PacketHandler packet_handler_;

    void worker_thread_func() {
        while (!is_stopped()) {
            std::string request;
            if (request_queue_.try_dequeue(request)) {
                process_request(request);
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }
    }

    void process_request(const std::string& request) {
        packet_handler_.append_data(request);

        Packet packet;
        while (packet_handler_.get_next_packet(packet)) {
            try {
                handle_packet(packet);
            } catch (const std::exception& e) {
                std::cerr << "处理数据包错误: " << e.what() << std::endl;
            }
        }
    }

    void handle_packet(const Packet& packet) {
        std::cout << "收到数据包：" << std::endl
                  << "  包长度: " << packet.packet_len << std::endl
                  << "  头部长度: " << packet.header_len << std::endl
                  << "  头部: " << packet.header.dump() << std::endl
                  << "  负载: " << packet.payload.dump() << std::endl;
        // 这里可以添加更多的处理逻辑
    }
};