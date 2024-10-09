#include <iostream>
#include "network/network.hpp"

int main() {
    try {
        NetworkProcessor processor;
        std::cout << "网络处理器已启动，按Enter键退出..." << std::endl;
        std::cin.get();
    } catch (const std::exception& e) {
        std::cerr << "异常: " << e.what() << std::endl;
    }

    return 0;
}
