#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/select.h>
#include <random>
#include "threadPool2.h"


bool isPortOpen(const std::string& host, int port, int timeoutSeconds = 1) {
    struct addrinfo hints, *result, *rp;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;        // AF_UNSPEC表示不关心具体使用哪个协议，可以是IPv4或IPv6
    hints.ai_socktype = SOCK_STREAM;    // TCP

    // 获取地址信息
    int status = getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints, &result);
    if (status != 0) {
        std::cerr << "getaddrinfo error: " << gai_strerror(status) << std::endl;
        return false;
    }
    
    bool isOpen = false;
    // 遍历result链表，尝试连接每一个地址
    for (rp = result; rp != nullptr; rp = rp->ai_next) {
        int sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sockfd == -1) {
            continue;
        }
        // 尝试连接
        if (connect(sockfd, rp->ai_addr, rp->ai_addrlen) == 0) {
            isOpen = true;
            close(sockfd);
            break;
        }

        close(sockfd);
    }

    freeaddrinfo(result);

    return isOpen;
}

int main() {
    std::string ip = "45.33.32.156";

    // 创建线程池
    std::cout << std::thread::hardware_concurrency() << std::endl;
    auto pool = ThreadPool::get_instance();
    pool->init();

    // 程序运行时间
    auto start = std::chrono::steady_clock::now();

    // // 存储所有任务的 future 对象
    std::vector<std::future<bool>> futures;

    // // 提交所有任务
    for (int i = 21; i < 1024; i++) {
        auto future_ = pool->submitTask(isPortOpen, ip, i, 1);
        futures.push_back(std::move(future_));
    }


    // 处理所有任务的结果
    for (int i = 21; i < 1024; i++) {
        bool result = futures[i - 21].get();
        if (result) {
            std::cout << ip << ":" << i << " 端口打开了!!!!!!!!!!!!!\n";
        } else {
            // std::cout << ip << ":" << i << " 关闭\n";
        }
    }
    
    // 单线程
    // for (int i = 21; i < 1024; i++) {
    //     bool result = isPortOpen(ip2, i, 1);
    //     if (result) {
    //         std::cout << ip2 << ":" << i << " 端口打开了!!!!!!!!!!!!!\n";
    //     } else {
    //         std::cout << ip2 << ":" << i << " 关闭\n";
    //     }
    // }


    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> diff = end - start;

    std::cout << "程序运行时间：" << diff.count() << " 秒" << std::endl;
    return 0;
}