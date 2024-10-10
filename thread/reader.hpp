#pragma once

#include "base.hpp"
#include "ikcp.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <chrono>
#include <vector>
#include <stdexcept>
#include "concurrentqueue/concurrentqueue.h"

class KcpReader : public BaseThread {
public:
    KcpReader(const char* server_ip, int server_port, moodycamel::ConcurrentQueue<std::string>& queue);
    ~KcpReader() override;

    void start() override;

private:
    ikcpcb* kcp_;
    int sock_;
    struct sockaddr_in server_addr_;
    moodycamel::ConcurrentQueue<std::string>& request_queue_;

    static int udp_output(const char *buf, int len, ikcpcb *kcp, void *user);
    static IUINT32 iclock();
    void run();
};

KcpReader::KcpReader(const char* server_ip, int server_port, moodycamel::ConcurrentQueue<std::string>& queue)
    : request_queue_(queue) {
    sock_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_ < 0) {
        throw std::runtime_error("无法创建套接字");
    }

    memset(&server_addr_, 0, sizeof(server_addr_));
    server_addr_.sin_family = AF_INET;
    server_addr_.sin_port = htons(server_port);
    if (inet_pton(AF_INET, server_ip, &server_addr_.sin_addr) <= 0) {
        throw std::runtime_error("无效的IP地址");
    }

    kcp_ = ikcp_create(0x11223344, this);
    if (!kcp_) {
        throw std::runtime_error("无法创建KCP对象");
    }

    ikcp_setoutput(kcp_, udp_output);
    ikcp_nodelay(kcp_, 1, 10, 2, 1);
    ikcp_wndsize(kcp_, 128, 128);
}

KcpReader::~KcpReader() {
    if (kcp_) {
        ikcp_release(kcp_);
    }
    if (sock_ >= 0) {
        close(sock_);
    }
}

void KcpReader::start() {
    thread_ = std::thread(&KcpReader::run, this);
}

int KcpReader::udp_output(const char *buf, int len, ikcpcb *kcp, void *user) {
    KcpReader* reader = static_cast<KcpReader*>(user);
    return sendto(reader->sock_, buf, len, 0, 
                  reinterpret_cast<struct sockaddr*>(&reader->server_addr_), sizeof(reader->server_addr_));
}

IUINT32 KcpReader::iclock() {
    return static_cast<IUINT32>(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count());
}

void KcpReader::run() {
    const int buffer_size = 1024;
    std::vector<char> buffer(buffer_size);

    while (!is_stopped()) {
        struct sockaddr_in from_addr;
        socklen_t from_len = sizeof(from_addr);
        int recv_len = recvfrom(sock_, buffer.data(), buffer_size, 0, 
                                reinterpret_cast<struct sockaddr*>(&from_addr), &from_len);

        if (recv_len > 0) {
            ikcp_input(kcp_, buffer.data(), recv_len);

            int kcp_recv_len;
            while ((kcp_recv_len = ikcp_recv(kcp_, buffer.data(), buffer_size)) > 0) {
                std::string received_data(buffer.data(), kcp_recv_len);
                request_queue_.enqueue(std::move(received_data));
            }

            if (kcp_recv_len < 0 && kcp_recv_len != -1) {
                std::cerr << "KCP接收错误: " << kcp_recv_len << std::endl;
            }
        }

        ikcp_update(kcp_, iclock());
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}