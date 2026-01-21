
/**
 * @file total_test.cpp
 * @author moshi (3511784747@qq.com)
 * @brief 测试代码，使用gtest框架
 * @version 0.1
 * @date 2025-12-28
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include <csignal>
#include <stdexcept>
#include <sys/socket.h>
#include <unistd.h>
#include <mutex>
#include <condition_variable>
#include <gtest/gtest.h>
#include <future>
#include <thread>

#include "tcpsocket.hpp"
#include "event_loop.hpp"

class EventLoopTest : public ::testing::Test {
protected:
    EventLoopTest() :locker(mtx) { }
    EventLoop* loop;
    
    std::mutex mtx;
    std::unique_lock<std::mutex> locker;
    std::condition_variable cv;
};

class SocketTest : public ::testing::Test {
protected:

};

void sendmsg(const std::string& msg) {
    TcpSocket client;
    client.connect("127.0.0.1", 8080);
    client.send(client.get_sockfd(), msg.c_str(), msg.size());
}

TEST_F(EventLoopTest, EventLoopStopTest) {
    std::thread([&] {
        loop = new EventLoop();

        ASSERT_EQ(loop->run(), true);
    }).detach();


    sleep(1);
    EXPECT_EQ(loop->stop(), true);

    delete loop;
}

TEST_F(EventLoopTest, BasicConnectAndReceive) {
    // 使用 promise 来同步服务器线程是否已准备好监听
    std::promise<bool> server_ready_promise;
    std::future<bool> server_ready = server_ready_promise.get_future();

    // 用于通知主线程测试已完成
    std::promise<void> test_complete_promise;
    std::future<void> test_complete = test_complete_promise.get_future();

    std::string received_message;

    // 将服务器逻辑放在一个需要等待的线程中，而不是 detach
    std::thread server_thread([this, &server_ready_promise, &test_complete_promise, &received_message]() {
        // 确保 EventLoop 对象在服务器线程内创建
        EventLoop loop;
        TcpSocket server;
        
        try {
            server.listen("127.0.0.1", 8080);
        } catch(std::runtime_error& err) {
            server_ready_promise.set_value(false); // 通知监听失败
        }

        // 1. 注册事件回调
        loop.add_event(server.get_sockfd(), EPOLLIN, [&](int fd, uint32_t events) {
            std::string client_ip;
            uint16_t client_port;
            int client_fd = server.accept(client_ip, client_port);

            // 为新连接的客户端socket注册读事件
            loop.add_event(client_fd, EPOLLIN, [&, client_fd](int fd, uint32_t events) {
                char buff[256];
                ssize_t len = server.recv(client_fd, buff, sizeof(buff) - 1);
                if (len > 0) {
                    buff[len] = '\0';
                    received_message = buff; // 保存接收到的消息
                    std::cout << "Server received: " << received_message << std::endl;

                    // 收到一条消息后，就通知主线程测试完成，并停止事件循环
                    test_complete_promise.set_value();
                    loop.stop(); // 在你的 EventLoop 实现中，这需要是线程安全的
                }
            });
        });

        // 2. 通知主线程：服务器已准备好，可以开始连接了
        server_ready_promise.set_value(true);

        // 3. 启动事件循环（这会阻塞，直到 loop.stop() 被调用）
        loop.run();
    });

    // 主线程：等待服务器线程准备好
    ASSERT_TRUE(server_ready.get()) << "Server failed to start listening";

    // 服务器已就绪，现在创建客户端并发送一条测试消息
    std::string test_msg = "Hello, EventLoop!";
    sendmsg(test_msg); // 你的发送消息函数

    // 等待服务器线程通知测试完成，设置一个超时避免测试永久挂起
    auto status = test_complete.wait_for(std::chrono::seconds(1));
    EXPECT_EQ(status, std::future_status::ready) << "Test timed out or message not received";

    // 验证收到的消息是否正确
    if (status == std::future_status::ready) {
        EXPECT_EQ(received_message, test_msg);
    }
    EXPECT_EQ(loop->stop(), true);

    // 等待服务器线程正常退出
    server_thread.join();
}

TEST_F(EventLoopTest, MultipleCallingRunAndStop) {
    std::thread([&] {
        loop = new EventLoop();
        EXPECT_TRUE(loop->run());
        EXPECT_TRUE(loop->run());
    }).detach();

    sleep(1);
    EXPECT_TRUE(loop->stop());
    EXPECT_TRUE(loop->stop());
}