#pragma once

#include <thread>
#include <atomic>

// 基础线程类
class BaseThread {
public:
    BaseThread() : stop_flag_(false) {}
    virtual ~BaseThread() = default;

    virtual void start() = 0;
    
    void stop() {
        stop_flag_ = true;
    }

    bool is_stopped() const {
        return stop_flag_;
    }

protected:
    std::thread thread_;
    std::atomic<bool> stop_flag_;
};