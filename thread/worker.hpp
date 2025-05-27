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
        packet_handler_.processRequest(request);
    }
};
