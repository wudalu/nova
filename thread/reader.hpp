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
private:
    ikcpcb* kcp;
    int sock;
    struct sockaddr_in server_addr;
    moodycamel::ConcurrentQueue<std::string>& request_queue;

    // KCP回调函数
    static int udp_output(const char *buf, int len, ikcpcb *kcp, void *user) {
        KcpReader* reader = (KcpReader*)user;
        return sendto(reader->sock, buf, len, 0, 
                      (struct sockaddr*)&reader->server_addr, sizeof(reader->server_addr));
    }

    // 获取当前时间（毫秒）
    static IUINT32 iclock() {
        return static_cast<IUINT32>(std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());
    }

public:
    KcpReader(const char* server_ip, int server_port, moodycamel::ConcurrentQueue<std::string>& queue)
        : request_queue(queue) {
        // 创建UDP套接字
        sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (sock < 0) {
            throw std::runtime_error("无法创建套接字");
        }

        // 设置服务器���址
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(server_port);
        if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
            throw std::runtime_error("无效的IP地址");
        }

        // 初始化KCP
        kcp = ikcp_create(0x11223344, this);
        if (!kcp) {
            throw std::runtime_error("无法创建KCP对象");
        }

        ikcp_setoutput(kcp, udp_output);
        ikcp_nodelay(kcp, 1, 10, 2, 1);
        ikcp_wndsize(kcp, 128, 128);
    }

    ~KcpReader() override {
        if (kcp) {
            ikcp_release(kcp);
        }
        if (sock >= 0) {
            close(sock);
        }
    }

    void start() override {
        thread = std::thread(&KcpReader::run, this);
    }

private:
    void run() {
        const int buffer_size = 1024;
        std::vector<char> buffer(buffer_size);

        while (!stop_flag) {  // 使用 stop_flag 替代 is_stopped()
            // 接收UDP数据
            struct sockaddr_in from_addr;
            socklen_t from_len = sizeof(from_addr);
            int recv_len = recvfrom(sock, buffer.data(), buffer_size, 0, 
                                    (struct sockaddr*)&from_addr, &from_len);

            if (recv_len > 0) {
                // 输入数据到KCP
                ikcp_input(kcp, buffer.data(), recv_len);

                // 从KCP接收数据
                int kcp_recv_len;
                while ((kcp_recv_len = ikcp_recv(kcp, buffer.data(), buffer_size)) > 0) {
                    // 处理接收到的数据
                    std::string received_data(buffer.data(), kcp_recv_len);
                    request_queue.enqueue(std::move(received_data));
                }

                if (kcp_recv_len < 0 && kcp_recv_len != -1) {
                    // 处理错误情况，-1 表示没有数据可读，不是错误
                    std::cerr << "KCP接收错误: " << kcp_recv_len << std::endl;
                }
            }

            // 更新KCP
            ikcp_update(kcp, iclock());

            // 短暂休眠避免CPU占用���高
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
};