#pragma once

#include <thread>
#include <atomic>

// 基础线程类
class BaseThread {
protected:
    std::thread thread;
    std::atomic<bool> stop_flag{false};

public:
    virtual ~BaseThread() {
        stop();
        if (thread.joinable()) {
            thread.join();
        }
    }

    virtual void start() = 0;

    void stop() {
        stop_flag.store(true);
    }

    bool is_stopped() const {
        return stop_flag.load();
    }
};