#pragma once

#include "thread/reader.hpp"
#include "thread/worker.hpp"
#include "include/macros.h"
#include "concurrentqueue/concurrentqueue.h"

class NetworkProcessor {
public:
    NetworkProcessor(const std::string& address = DEFAULT_ADDRESS, 
                     unsigned short port = DEFAULT_PORT);
    ~NetworkProcessor() = default;

private:
    std::vector<std::unique_ptr<KcpReader>> reader_threads_;
    std::vector<std::unique_ptr<WorkerThread>> worker_threads_;
    moodycamel::ConcurrentQueue<std::string> request_queue_;
};

NetworkProcessor::NetworkProcessor(const std::string& address, unsigned short port) {
    for (int i = 0; i < READER_THREAD_COUNT; ++i) {
        reader_threads_.emplace_back(std::make_unique<KcpReader>(
            address.c_str(), port, request_queue_));
    }

    for (int i = 0; i < WORKER_THREAD_COUNT; ++i) {
        worker_threads_.emplace_back(std::make_unique<WorkerThread>(request_queue_));
    }

    for (auto& thread : reader_threads_) {
        thread->start();
    }
    for (auto& thread : worker_threads_) {
        thread->start();
    }
}