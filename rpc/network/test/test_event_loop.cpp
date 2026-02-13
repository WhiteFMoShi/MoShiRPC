/**
 * @file test_event_loop.cpp
 * @author moshi (3511784747@qq.com)
 * @brief 此测试的内容已废弃，所有的测试部分已经被集成到total_test中
 * @version 0.1
 * @date 2025-12-28
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include <cstring>
#include <iostream>
#include <thread>

#include <gtest/gtest.h>
#include <unistd.h>

#include "event_loop.hpp"
#include "tcpsocket.hpp"

int main() {
    EventLoop* loop;

    std::thread t([&loop](){
        loop = new EventLoop();
        TcpSocket server;
        server.listen("127.0.0.1", 8080);

        char buff[256];

        loop->add_channel(server.get_sockfd(), EPOLLIN, [&](int fd, uint32_t events) {
            std::string client_ip;
            uint16_t client_port;
            int client_fd = server.accept(client_ip, client_port);
            std::cout << "New connection from " << client_ip << ":" << client_port << std::endl;
            server.recv(client_fd, buff, 256);

            printf("%s\n", buff);
        });

        loop->start();
    });

    sleep(5);
    std::cout << "stop called" << std::endl;
    loop->stop();
    t.join();
    return 0;
}