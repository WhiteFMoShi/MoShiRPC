/**
 * @file test_socket.cpp
 * @author moshi (3511784747@qq.com)
 * @brief 此测试的内容已废弃，所有的测试部分已经被集成到total_test中
 * @version 0.1
 * @date 2025-12-28
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include <iostream>
#include <ostream>
#include <string>
#include <thread>
#include <unistd.h>

#include "network/tcpsocket.hpp"

using namespace moshi;

void server_thread() {
    TcpSocket server;
    server.Listen("127.0.0.1", 8080);

    std::cout << "监听中\n";

    std::string client_ip;
    uint16_t client_port;
    int client_fd = server.Accept(client_ip, client_port);

    char buf[1024];
    ssize_t len;
    while((len = server.Recv(client_fd, buf, sizeof(buf))) > 0) {
    std::cout << "Server received: " << std::string(buf, len);
    }
    std::endl(std::cout);

    server.Close(client_fd);
}

int main() {
    std::thread t(server_thread);
    t.detach();

    TcpSocket client;
    if (client.Connect("127.0.0.1", 8080) < 0)
        return 0;

    std::cout << "信息已发送\n";
    std::string msg = "Hello moshiRPC";
    client.Send(client.GetSockfd(), msg.c_str(), msg.size());
    sleep(1 / 10);

    return 0;
}