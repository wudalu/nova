#pragma once

#include "thread/reader.hpp"
#include "thread/worker.hpp"
#include "include/macros.h"
#include "concurrentqueue/concurrentqueue.h"

// 网络处理器类
class NetworkProcessor {
private:
    std::vector<std::unique_ptr<KcpReader>> reader_threads;
    std::vector<std::unique_ptr<WorkerThread>> worker_threads;
    moodycamel::ConcurrentQueue<std::string> request_queue;

public:
    NetworkProcessor(const std::string& address = DEFAULT_ADDRESS, 
                     unsigned short port = DEFAULT_PORT) {
        // 创建读取线程
        for (int i = 0; i < READER_THREAD_COUNT; ++i) {
            reader_threads.emplace_back(std::make_unique<KcpReader>(
                address.c_str(), port, request_queue));
        }

        // 创建工作线程
        for (int i = 0; i < WORKER_THREAD_COUNT; ++i) {
            worker_threads.emplace_back(std::make_unique<WorkerThread>(request_queue));
        }

        // 启动所有线程
        for (auto& thread : reader_threads) {
            thread->start();
        }
        for (auto& thread : worker_threads) {
            thread->start();
        }
    }

    ~NetworkProcessor() {
        // 析构函数中的停止逻辑会自动调用
    }
};