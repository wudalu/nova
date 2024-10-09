#pragma once

#include "base.hpp"
#include "concurrentqueue/concurrentqueue.h"
#include <string>
#include <iostream>

// 工作线程类
class WorkerThread : public BaseThread {
private:
    moodycamel::ConcurrentQueue<std::string>& request_queue;

    void worker_thread_func() {
        while (!is_stopped()) {  // 使用 is_stopped() 替代 stop_flag
            std::string request;
            if (request_queue.try_dequeue(request)) {
                // 处理请求（目前只是打印）
                std::cout << "处理请求: " << request << std::endl;
            } else {
                // 队列为空，短暂休眠避免CPU占用过高
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }
    }

public:
    WorkerThread(moodycamel::ConcurrentQueue<std::string>& queue)
        : request_queue(queue) {}

    void start() override {
        thread = std::thread(&WorkerThread::worker_thread_func, this);
    }
};