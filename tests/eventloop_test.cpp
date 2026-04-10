#include <atomic>
#include <algorithm>
#include <chrono>
#include <cerrno>
#include <csignal>
#include <cstdio>
#include <future>
#include <memory>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <sys/socket.h>
#include <unistd.h>

#include <gtest/gtest.h>

#include "network/event_loop.hpp"

using moshi::Channel;
using moshi::EventLoop;

namespace {

class UniqueFd {
public:
    UniqueFd() = default;
    explicit UniqueFd(int fd) : fd_(fd) {}
    ~UniqueFd() { reset(); }

    UniqueFd(const UniqueFd&) = delete;
    UniqueFd& operator=(const UniqueFd&) = delete;

    UniqueFd(UniqueFd&& other) noexcept : fd_(other.fd_) { other.fd_ = -1; }
    UniqueFd& operator=(UniqueFd&& other) noexcept {
        if (this == &other) {
            return *this;
        }
        reset();
        fd_ = other.fd_;
        other.fd_ = -1;
        return *this;
    }

    int get() const { return fd_; }

    void reset(int new_fd = -1) {
        if (fd_ >= 0) {
            ::close(fd_);
        }
        fd_ = new_fd;
    }

private:
    int fd_{-1};
};

void set_nonblocking(int fd) {
    const int flags = ::fcntl(fd, F_GETFL, 0);
    ASSERT_GE(flags, 0);
    ASSERT_EQ(::fcntl(fd, F_SETFL, flags | O_NONBLOCK), 0);
}

void drain_fd(int fd) {
    char buf[4096];
    while (true) {
        const ssize_t n = ::read(fd, buf, sizeof(buf));
        if (n > 0) {
            continue;
        }
        if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            break;
        }
        break;
    }
}

ssize_t read_some(int fd, char* buf, size_t len) {
    while (true) {
        const ssize_t n = ::read(fd, buf, len);
        if (n < 0 && errno == EINTR) {
            continue;
        }
        return n;
    }
}

void write_all(int fd, const void* data, size_t len) {
    const char* p = static_cast<const char*>(data);
    size_t left = len;
    while (left > 0) {
        const ssize_t n = ::write(fd, p, left);
        if (n > 0) {
            p += n;
            left -= static_cast<size_t>(n);
            continue;
        }
        if (n < 0 && errno == EINTR) {
            continue;
        }
        // 对测试来说，失败直接断言
        ASSERT_GT(n, 0);
    }
}

void write_byte(int fd) {
    const char b = 'x';
    write_all(fd, &b, 1);
}

void try_write_byte(int fd) {
    const char b = 'x';
    while (true) {
        const ssize_t n = ::write(fd, &b, 1);
        if (n == 1) {
            return;
        }
        if (n < 0 && errno == EINTR) {
            continue;
        }
        return;
    }
}

bool wait_until(std::function<bool()> pred, std::chrono::milliseconds timeout) {
    const auto deadline = std::chrono::steady_clock::now() + timeout;
    while (std::chrono::steady_clock::now() < deadline) {
        if (pred()) {
            return true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    return pred();
}

std::string read_until_exact(int fd, size_t bytes, std::chrono::milliseconds timeout) {
    std::string out;
    out.resize(bytes);
    size_t off = 0;

    const auto deadline = std::chrono::steady_clock::now() + timeout;
    while (off < bytes && std::chrono::steady_clock::now() < deadline) {
        const ssize_t n = read_some(fd, out.data() + off, bytes - off);
        if (n > 0) {
            off += static_cast<size_t>(n);
            continue;
        }
        if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }
        break;
    }

    out.resize(off);
    return out;
}

void sigusr1_handler(int) {}

bool install_sigusr1_handler_once() {
    static std::once_flag once;
    static std::atomic<int> installed{0}; // 1 ok, -1 failed
    std::call_once(once, [] {
        struct sigaction sa {};
        sa.sa_handler = sigusr1_handler;
        ::sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        installed.store((::sigaction(SIGUSR1, &sa, nullptr) == 0) ? 1 : -1);
    });
    return installed.load() == 1;
}

} // namespace

TEST(EventLoopReliability, StopBeforeStartAndRestartWorks) {
    std::promise<std::shared_ptr<EventLoop>> loop_promise;
    auto loop_future = loop_promise.get_future();

    std::promise<void> go_promise;
    std::shared_future<void> go_future = go_promise.get_future().share();

    std::promise<bool> first_start_promise;
    auto first_start_future = first_start_promise.get_future();

    std::promise<bool> second_start_promise;
    auto second_start_future = second_start_promise.get_future();

    std::thread loop_thread([&] {
        auto loop = std::make_shared<EventLoop>();
        loop_promise.set_value(loop);

        go_future.wait();

        first_start_promise.set_value(loop->Start());
        second_start_promise.set_value(loop->Start());
    });

    std::shared_ptr<EventLoop> loop_ptr = loop_future.get();

    // stop() called before start(): should not prevent later starts.
    EXPECT_TRUE(loop_ptr->Stop());

    go_promise.set_value();

    const bool first_running = wait_until([&] { return loop_ptr->GetRunStatus(); }, std::chrono::milliseconds(200));
    if (!first_running) {
        loop_ptr->Stop();
    }
    EXPECT_TRUE(first_running);
    EXPECT_TRUE(loop_ptr->Stop());
    const auto first_status = first_start_future.wait_for(std::chrono::milliseconds(500));
    if (first_status != std::future_status::ready) {
        loop_ptr->Stop();
    }
    EXPECT_EQ(first_status, std::future_status::ready);
    if (first_status == std::future_status::ready) {
        EXPECT_TRUE(first_start_future.get());
    }

    const bool second_running = wait_until([&] { return loop_ptr->GetRunStatus(); }, std::chrono::milliseconds(200));
    if (!second_running) {
        loop_ptr->Stop();
    }
    EXPECT_TRUE(second_running);
    EXPECT_TRUE(loop_ptr->Stop());
    const auto second_status = second_start_future.wait_for(std::chrono::milliseconds(500));
    if (second_status != std::future_status::ready) {
        loop_ptr->Stop();
    }
    EXPECT_EQ(second_status, std::future_status::ready);
    if (second_status == std::future_status::ready) {
        EXPECT_TRUE(second_start_future.get());
    }

    loop_thread.join();
}

TEST(EventLoopReliability, StopWakesUpEpollWait) {
    std::promise<EventLoop*> loop_promise;
    auto loop_future = loop_promise.get_future();

    std::promise<bool> start_ret_promise;
    auto start_ret_future = start_ret_promise.get_future();

    std::thread t([&] {
        EventLoop loop;
        loop_promise.set_value(&loop);
        start_ret_promise.set_value(loop.Start());
    });

    EventLoop* loop_ptr = loop_future.get();
    const bool running = wait_until([&] { return loop_ptr->GetRunStatus(); }, std::chrono::milliseconds(200));
    if (!running) {
        loop_ptr->Stop();
    }
    EXPECT_TRUE(running);

    EXPECT_TRUE(loop_ptr->Stop());
    EXPECT_TRUE(start_ret_future.get());
    t.join();
}

TEST(EventLoopReliability, ConcurrentStopStormDoesNotHang) {
    std::promise<std::shared_ptr<EventLoop>> loop_promise;
    auto loop_future = loop_promise.get_future();

    std::promise<bool> start_ret_promise;
    auto start_ret_future = start_ret_promise.get_future();

    std::thread loop_thread([&] {
        auto loop = std::make_shared<EventLoop>();
        loop_promise.set_value(loop);
        start_ret_promise.set_value(loop->Start());
    });

    std::shared_ptr<EventLoop> loop_ptr = loop_future.get();
    const bool running = wait_until([&] { return loop_ptr->GetRunStatus(); }, std::chrono::milliseconds(200));
    if (!running) {
        loop_ptr->Stop();
    }
    EXPECT_TRUE(running);

    constexpr int kThreads = 8;
    constexpr int kLoops = 2000;
    std::vector<std::thread> stoppers;
    stoppers.reserve(kThreads);
    for (int i = 0; i < kThreads; ++i) {
        stoppers.emplace_back([&] {
            for (int j = 0; j < kLoops; ++j) {
                loop_ptr->Stop();
            }
        });
    }
    for (auto& th : stoppers) {
        th.join();
    }

    EXPECT_TRUE(start_ret_future.get());
    loop_thread.join();
}

TEST(EventLoopReliability, PipeReadableTriggersCallback) {
    int fds[2]{};
    ASSERT_EQ(::pipe(fds), 0);
    UniqueFd read_fd(fds[0]);
    UniqueFd write_fd(fds[1]);
    set_nonblocking(read_fd.get());

    int ctl_fds[2]{};
    ASSERT_EQ(::pipe(ctl_fds), 0);
    UniqueFd ctl_read(ctl_fds[0]);
    UniqueFd ctl_write(ctl_fds[1]);
    set_nonblocking(ctl_read.get());

    std::promise<void> ready_promise;
    auto ready_future = ready_promise.get_future();

    std::promise<void> got_promise;
    auto got_future = got_promise.get_future();

    std::promise<bool> add_ok_promise;
    auto add_ok_future = add_ok_promise.get_future();

    std::promise<bool> start_ok_promise;
    auto start_ok_future = start_ok_promise.get_future();

    std::atomic<bool> got{false};

    std::thread t([&] {
        EventLoop loop;

        auto ch = std::make_shared<Channel>();
        ch->SetEpollinCallback([&] {
            drain_fd(read_fd.get());
            if (got.exchange(true)) {
                return;
            }
            got_promise.set_value();
            loop.Stop();
        });

        auto ctl_ch = std::make_shared<Channel>();
        ctl_ch->SetEpollinCallback([&] {
            drain_fd(ctl_read.get());
            loop.Stop();
        });

        const bool add_ok = loop.AddFd(read_fd.get(), ch) && loop.AddFd(ctl_read.get(), ctl_ch);
        add_ok_promise.set_value(add_ok);
        ready_promise.set_value();

        if (!add_ok) {
            start_ok_promise.set_value(false);
            return;
        }
        start_ok_promise.set_value(loop.Start());
    });

    ready_future.get();
    const bool add_ok = add_ok_future.get();
    EXPECT_TRUE(add_ok);
    if (!add_ok) {
        t.join();
        return;
    }
    write_byte(write_fd.get());

    const auto got_status = got_future.wait_for(std::chrono::milliseconds(200));
    if (got_status != std::future_status::ready &&
        start_ok_future.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready) {
        try_write_byte(ctl_write.get());
    }
    EXPECT_EQ(got_status, std::future_status::ready);
    EXPECT_TRUE(start_ok_future.get());
    t.join();
}

TEST(EventLoopReliability, HighVolumePipeEvents) {
    int fds[2]{};
    ASSERT_EQ(::pipe(fds), 0);
    UniqueFd read_fd(fds[0]);
    UniqueFd write_fd(fds[1]);
    set_nonblocking(read_fd.get());
    set_nonblocking(write_fd.get());

    int ctl_fds[2]{};
    ASSERT_EQ(::pipe(ctl_fds), 0);
    UniqueFd ctl_read(ctl_fds[0]);
    UniqueFd ctl_write(ctl_fds[1]);
    set_nonblocking(ctl_read.get());

    constexpr size_t kBytes = 256 * 1024;
    std::atomic<size_t> bytes_read{0};
    std::atomic<bool> done{false};

    std::promise<void> ready_promise;
    auto ready_future = ready_promise.get_future();

    std::promise<void> done_promise;
    auto done_future = done_promise.get_future();

    std::promise<bool> add_ok_promise;
    auto add_ok_future = add_ok_promise.get_future();

    std::promise<bool> start_ok_promise;
    auto start_ok_future = start_ok_promise.get_future();

    std::thread loop_thread([&] {
        EventLoop loop;

        auto ch = std::make_shared<Channel>();
        ch->SetEpollinCallback([&] {
            char buf[4096];
            while (true) {
                const ssize_t n = read_some(read_fd.get(), buf, sizeof(buf));
                if (n > 0) {
                    bytes_read.fetch_add(static_cast<size_t>(n));
                    continue;
                }
                if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                    break;
                }
                break;
            }

            if (bytes_read.load() >= kBytes) {
                if (done.exchange(true)) {
                    return;
                }
                done_promise.set_value();
                loop.Stop();
            }
        });

        auto ctl_ch = std::make_shared<Channel>();
        ctl_ch->SetEpollinCallback([&] {
            drain_fd(ctl_read.get());
            loop.Stop();
        });

        const bool add_ok = loop.AddFd(read_fd.get(), ch) && loop.AddFd(ctl_read.get(), ctl_ch);
        add_ok_promise.set_value(add_ok);
        ready_promise.set_value();

        if (!add_ok) {
            start_ok_promise.set_value(false);
            return;
        }
        start_ok_promise.set_value(loop.Start());
    });

    ready_future.get();
    const bool add_ok = add_ok_future.get();
    EXPECT_TRUE(add_ok);
    if (!add_ok) {
        loop_thread.join();
        return;
    }

    std::atomic<bool> stop_writer{false};
    std::atomic<bool> writer_ok{true};

    std::thread writer([&] {
        std::string payload;
        payload.resize(4096, 'a');
        size_t left = kBytes;
        while (left > 0) {
            if (stop_writer.load()) {
                break;
            }
            const size_t chunk = std::min(left, payload.size());
            const ssize_t n = ::write(write_fd.get(), payload.data(), chunk);
            if (n > 0) {
                left -= static_cast<size_t>(n);
                continue;
            }
            if (n < 0 && errno == EINTR) {
                continue;
            }
            if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                continue;
            }
            writer_ok.store(false);
            break;
        }
    });

    const auto done_status = done_future.wait_for(std::chrono::seconds(2));
    if (done_status != std::future_status::ready &&
        start_ok_future.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready) {
        try_write_byte(ctl_write.get());
    }
    EXPECT_EQ(done_status, std::future_status::ready);

    if (done_status != std::future_status::ready) {
        stop_writer.store(true);
    }
    writer.join();
    EXPECT_TRUE(writer_ok.load());
    EXPECT_TRUE(start_ok_future.get());
    loop_thread.join();

    EXPECT_GE(bytes_read.load(), kBytes);
}

TEST(EventLoopReliability, RemoveFdDisablesFurtherCallbacks) {
    int data_fds[2]{};
    ASSERT_EQ(::pipe(data_fds), 0);
    UniqueFd data_read(data_fds[0]);
    UniqueFd data_write(data_fds[1]);
    set_nonblocking(data_read.get());

    int ctl_fds[2]{};
    ASSERT_EQ(::pipe(ctl_fds), 0);
    UniqueFd ctl_read(ctl_fds[0]);
    UniqueFd ctl_write(ctl_fds[1]);
    set_nonblocking(ctl_read.get());

    std::atomic<int> calls{0};
    std::atomic<bool> removed{false};

    std::promise<void> ready_promise;
    auto ready_future = ready_promise.get_future();

    std::promise<void> removed_promise;
    auto removed_future = removed_promise.get_future();

    std::promise<bool> add_ok_promise;
    auto add_ok_future = add_ok_promise.get_future();

    std::promise<bool> start_ok_promise;
    auto start_ok_future = start_ok_promise.get_future();

    std::atomic<bool> remove_ok{false};

    std::thread t([&] {
        EventLoop loop;

        auto data_ch = std::make_shared<Channel>();
        data_ch->SetEpollinCallback([&] {
            drain_fd(data_read.get());
            const int c = ++calls;
            if (c == 1) {
                remove_ok.store(loop.RemoveFd(data_read.get()));
                bool expected = false;
                if (removed.compare_exchange_strong(expected, true)) {
                    removed_promise.set_value();
                }
            }
        });

        auto ctl_ch = std::make_shared<Channel>();
        ctl_ch->SetEpollinCallback([&] {
            drain_fd(ctl_read.get());
            loop.Stop();
        });

        const bool add_ok = loop.AddFd(data_read.get(), data_ch) && loop.AddFd(ctl_read.get(), ctl_ch);
        add_ok_promise.set_value(add_ok);
        ready_promise.set_value();

        if (!add_ok) {
            start_ok_promise.set_value(false);
            return;
        }
        start_ok_promise.set_value(loop.Start());
    });

    ready_future.get();
    const bool add_ok = add_ok_future.get();
    EXPECT_TRUE(add_ok);
    if (!add_ok) {
        EXPECT_TRUE(start_ok_future.get());
        t.join();
        return;
    }

    write_byte(data_write.get());
    const auto removed_status = removed_future.wait_for(std::chrono::milliseconds(200));
    if (removed_status != std::future_status::ready &&
        start_ok_future.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready) {
        try_write_byte(ctl_write.get());
    }
    EXPECT_EQ(removed_status, std::future_status::ready);
    if (removed_status != std::future_status::ready) {
        EXPECT_TRUE(start_ok_future.get());
        t.join();
        return;
    }

    // 这次写入不应再触发 data_ch
    write_byte(data_write.get());

    // 触发一次 control 事件，保证 loop 继续跑了一轮再退出
    write_byte(ctl_write.get());

    EXPECT_TRUE(start_ok_future.get());
    t.join();
    EXPECT_EQ(calls.load(), 1);
    EXPECT_TRUE(remove_ok.load());
}

TEST(EventLoopReliability, ResetChannelSwapsCallback) {
    int data_fds[2]{};
    ASSERT_EQ(::pipe(data_fds), 0);
    UniqueFd data_read(data_fds[0]);
    UniqueFd data_write(data_fds[1]);
    set_nonblocking(data_read.get());

    int ctl_fds[2]{};
    ASSERT_EQ(::pipe(ctl_fds), 0);
    UniqueFd ctl_read(ctl_fds[0]);
    UniqueFd ctl_write(ctl_fds[1]);
    set_nonblocking(ctl_read.get());

    std::atomic<int> cb1_calls{0};
    std::atomic<int> cb2_calls{0};
    std::atomic<bool> switched{false};

    std::promise<void> ready_promise;
    auto ready_future = ready_promise.get_future();

    std::promise<void> switched_promise;
    auto switched_future = switched_promise.get_future();

    std::promise<bool> add_ok_promise;
    auto add_ok_future = add_ok_promise.get_future();

    std::promise<bool> start_ok_promise;
    auto start_ok_future = start_ok_promise.get_future();

    std::atomic<bool> reset_ok{false};

    std::thread t([&] {
        EventLoop loop;

        auto ch1 = std::make_shared<Channel>();
        ch1->SetEpollinCallback([&] {
            drain_fd(data_read.get());
            ++cb1_calls;

            bool expected = false;
            if (!switched.compare_exchange_strong(expected, true)) {
                return;
            }

            auto ch2 = std::make_shared<Channel>();
                ch2->SetEpollinCallback([&] {
                    drain_fd(data_read.get());
                    ++cb2_calls;
                });

            reset_ok.store(loop.ResetChannel(data_read.get(), ch2));
            switched_promise.set_value();
        });

        auto ctl_ch = std::make_shared<Channel>();
        ctl_ch->SetEpollinCallback([&] {
            drain_fd(ctl_read.get());
            loop.Stop();
        });

        const bool add_ok = loop.AddFd(data_read.get(), ch1) && loop.AddFd(ctl_read.get(), ctl_ch);
        add_ok_promise.set_value(add_ok);
        ready_promise.set_value();

        if (!add_ok) {
            start_ok_promise.set_value(false);
            return;
        }
        start_ok_promise.set_value(loop.Start());
    });

    ready_future.get();
    const bool add_ok = add_ok_future.get();
    EXPECT_TRUE(add_ok);
    if (!add_ok) {
        EXPECT_TRUE(start_ok_future.get());
        t.join();
        return;
    }

    write_byte(data_write.get());
    const auto switched_status = switched_future.wait_for(std::chrono::milliseconds(200));
    if (switched_status != std::future_status::ready &&
        start_ok_future.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready) {
        try_write_byte(ctl_write.get());
    }
    EXPECT_EQ(switched_status, std::future_status::ready);
    if (switched_status != std::future_status::ready) {
        EXPECT_TRUE(start_ok_future.get());
        t.join();
        return;
    }

    write_byte(data_write.get());
    write_byte(ctl_write.get());

    EXPECT_TRUE(start_ok_future.get());
    t.join();
    EXPECT_EQ(cb1_calls.load(), 1);
    EXPECT_EQ(cb2_calls.load(), 1);
    EXPECT_TRUE(reset_ok.load());
}

TEST(EventLoopReliability, SocketpairEcho) {
    int fds[2]{};
    ASSERT_EQ(::socketpair(AF_UNIX, SOCK_STREAM, 0, fds), 0);
    UniqueFd server_fd(fds[0]);
    UniqueFd client_fd(fds[1]);
    set_nonblocking(server_fd.get());
    set_nonblocking(client_fd.get());

    const std::string msg = "hello echo";

    std::promise<void> ready_promise;
    auto ready_future = ready_promise.get_future();

    std::promise<void> echoed_promise;
    auto echoed_future = echoed_promise.get_future();

    int ctl_fds[2]{};
    ASSERT_EQ(::pipe(ctl_fds), 0);
    UniqueFd ctl_read(ctl_fds[0]);
    UniqueFd ctl_write(ctl_fds[1]);
    set_nonblocking(ctl_read.get());

    std::promise<bool> add_ok_promise;
    auto add_ok_future = add_ok_promise.get_future();

    std::promise<bool> start_ok_promise;
    auto start_ok_future = start_ok_promise.get_future();

    std::atomic<bool> echoed_once{false};
    std::atomic<bool> echo_write_ok{true};

    std::thread loop_thread([&] {
        EventLoop loop;

        auto ch = std::make_shared<Channel>();
        ch->SetEpollinCallback([&] {
            char buf[128];
            const ssize_t n = read_some(server_fd.get(), buf, sizeof(buf));
            if (n > 0) {
                ssize_t off = 0;
                while (off < n) {
                    const ssize_t wn = ::write(server_fd.get(), buf + off, static_cast<size_t>(n - off));
                    if (wn > 0) {
                        off += wn;
                        continue;
                    }
                    if (wn < 0 && errno == EINTR) {
                        continue;
                    }
                    echo_write_ok.store(false);
                    loop.Stop();
                    return;
                }
                if (echoed_once.exchange(true)) {
                    return;
                }
                echoed_promise.set_value();
                loop.Stop();
                return;
            }
            if (n == 0) {
                loop.Stop();
                return;
            }
            if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                return;
            }
            loop.Stop();
        });

        auto ctl_ch = std::make_shared<Channel>();
        ctl_ch->SetEpollinCallback([&] {
            drain_fd(ctl_read.get());
            loop.Stop();
        });

        const bool add_ok = loop.AddFd(server_fd.get(), ch) && loop.AddFd(ctl_read.get(), ctl_ch);
        add_ok_promise.set_value(add_ok);
        ready_promise.set_value();

        if (!add_ok) {
            start_ok_promise.set_value(false);
            return;
        }
        start_ok_promise.set_value(loop.Start());
    });

    ready_future.get();
    const bool add_ok = add_ok_future.get();
    EXPECT_TRUE(add_ok);
    if (!add_ok) {
        EXPECT_TRUE(start_ok_future.get());
        loop_thread.join();
        return;
    }

    // 让 RDHUP 和 EPOLLIN 有机会同时到来（取决于内核/类型）
    write_all(client_fd.get(), msg.data(), msg.size());
    ::shutdown(client_fd.get(), SHUT_WR);

    const auto echoed_status = echoed_future.wait_for(std::chrono::milliseconds(500));
    if (echoed_status != std::future_status::ready &&
        start_ok_future.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready) {
        try_write_byte(ctl_write.get());
    }
    EXPECT_EQ(echoed_status, std::future_status::ready);

    if (echoed_status == std::future_status::ready) {
        const std::string echoed = read_until_exact(client_fd.get(), msg.size(), std::chrono::milliseconds(500));
        EXPECT_EQ(echoed, msg);
    }

    EXPECT_TRUE(start_ok_future.get());
    loop_thread.join();
    EXPECT_TRUE(echo_write_ok.load());
}

TEST(EventLoopReliability, AddFdRequiresFocusedEvents) {
    int fds[2]{};
    ASSERT_EQ(::pipe(fds), 0);
    UniqueFd read_fd(fds[0]);
    UniqueFd write_fd(fds[1]);
    (void)write_fd;

    EventLoop loop;
    auto ch = std::make_shared<Channel>();
    EXPECT_FALSE(loop.AddFd(read_fd.get(), ch));
}

TEST(EventLoopReliability, StopIsIdempotentWhenNotRunning) {
    EventLoop loop;
    EXPECT_FALSE(loop.GetRunStatus());
    EXPECT_TRUE(loop.Stop());
    EXPECT_FALSE(loop.GetRunStatus());
}

TEST(EventLoopReliability, AddFdNullChannelRejected) {
    int fds[2]{};
    ASSERT_EQ(::pipe(fds), 0);
    UniqueFd read_fd(fds[0]);
    UniqueFd write_fd(fds[1]);
    (void)write_fd;

    EventLoop loop;
    EXPECT_FALSE(loop.AddFd(read_fd.get(), nullptr));
}

TEST(EventLoopReliability, AddResetRemoveMustBeCreatorThread) {
    int fds[2]{};
    ASSERT_EQ(::pipe(fds), 0);
    UniqueFd read_fd(fds[0]);
    UniqueFd write_fd(fds[1]);
    (void)write_fd;

    EventLoop loop;
    auto ch = std::make_shared<Channel>();
    ch->SetEpollinCallback([] {});

    std::promise<bool> add_promise;
    std::promise<bool> reset_promise;
    std::promise<bool> remove_promise;

    auto add_future = add_promise.get_future();
    auto reset_future = reset_promise.get_future();
    auto remove_future = remove_promise.get_future();

    std::thread t([&] {
        add_promise.set_value(loop.AddFd(read_fd.get(), ch));
        reset_promise.set_value(loop.ResetChannel(read_fd.get(), ch));
        remove_promise.set_value(loop.RemoveFd(read_fd.get()));
    });
    t.join();

    EXPECT_FALSE(add_future.get());
    EXPECT_FALSE(reset_future.get());
    EXPECT_FALSE(remove_future.get());
}

TEST(EventLoopReliability, RemoveUnknownFdRejected) {
    int fds[2]{};
    ASSERT_EQ(::pipe(fds), 0);
    UniqueFd read_fd(fds[0]);
    UniqueFd write_fd(fds[1]);
    (void)write_fd;

    EventLoop loop;
    EXPECT_FALSE(loop.RemoveFd(read_fd.get()));
}

TEST(EventLoopReliability, ResetUnknownFdRejected) {
    int fds[2]{};
    ASSERT_EQ(::pipe(fds), 0);
    UniqueFd read_fd(fds[0]);
    UniqueFd write_fd(fds[1]);
    (void)write_fd;

    EventLoop loop;
    auto ch = std::make_shared<Channel>();
    ch->SetEpollinCallback([] {});
    EXPECT_FALSE(loop.ResetChannel(read_fd.get(), ch));
}

TEST(EventLoopReliability, ResetChannelRequiresFocusedEvents) {
    int fds[2]{};
    ASSERT_EQ(::pipe(fds), 0);
    UniqueFd read_fd(fds[0]);
    UniqueFd write_fd(fds[1]);
    (void)write_fd;

    EventLoop loop;
    auto ch = std::make_shared<Channel>(); // focus_events_ == 0
    EXPECT_FALSE(loop.ResetChannel(read_fd.get(), ch));
}

TEST(EventLoopReliability, AddSameFdTwiceKeepsOriginalCallback) {
    int fds[2]{};
    ASSERT_EQ(::pipe(fds), 0);
    UniqueFd read_fd(fds[0]);
    UniqueFd write_fd(fds[1]);
    set_nonblocking(read_fd.get());

    EventLoop loop;

    std::atomic<int> cb1_calls{0};
    std::atomic<int> cb2_calls{0};

    auto ch1 = std::make_shared<Channel>();
    ch1->SetEpollinCallback([&] {
        drain_fd(read_fd.get());
        ++cb1_calls;
        loop.Stop();
    });

    auto ch2 = std::make_shared<Channel>();
    ch2->SetEpollinCallback([&] { ++cb2_calls; });

    ASSERT_TRUE(loop.AddFd(read_fd.get(), ch1));
    EXPECT_FALSE(loop.AddFd(read_fd.get(), ch2));

    std::thread writer([&] {
        const char b = 'x';
        (void)::write(write_fd.get(), &b, 1);
    });

    EXPECT_TRUE(loop.Start());
    writer.join();

    EXPECT_EQ(cb1_calls.load(), 1);
    EXPECT_EQ(cb2_calls.load(), 0);
}

TEST(EventLoopReliability, ErrorEventsPreferErrorCallback) {
    int fds[2]{};
    ASSERT_EQ(::socketpair(AF_UNIX, SOCK_STREAM, 0, fds), 0);
    UniqueFd server_fd(fds[0]);
    UniqueFd client_fd(fds[1]);
    set_nonblocking(server_fd.get());

    EventLoop loop;

    std::atomic<int> read_calls{0};
    std::atomic<int> err_calls{0};

    auto ch = std::make_shared<Channel>();
    ch->SetEpollinCallback([&] {
        ++read_calls;
        loop.Stop();
    });
    ch->SetEpollerrCallback([&] {
        ++err_calls;
        loop.Stop();
    });

    ASSERT_TRUE(loop.AddFd(server_fd.get(), ch));

    // 让对端先关闭，server_fd 上应出现 HUP/RDHUP
    client_fd.reset();

    EXPECT_TRUE(loop.Start());

    EXPECT_EQ(err_calls.load(), 1);
    EXPECT_EQ(read_calls.load(), 0);
}

TEST(EventLoopReliability, EpollWaitEintrIsIgnored) {
    ASSERT_TRUE(install_sigusr1_handler_once());

    int fds[2]{};
    ASSERT_EQ(::pipe(fds), 0);
    UniqueFd read_fd(fds[0]);
    UniqueFd write_fd(fds[1]);
    set_nonblocking(read_fd.get());

    int ctl_fds[2]{};
    ASSERT_EQ(::pipe(ctl_fds), 0);
    UniqueFd ctl_read(ctl_fds[0]);
    UniqueFd ctl_write(ctl_fds[1]);
    set_nonblocking(ctl_read.get());

    std::promise<void> ready_promise;
    auto ready_future = ready_promise.get_future();

    std::promise<void> got_event_promise;
    auto got_event_future = got_event_promise.get_future();

    std::promise<bool> start_ret_promise;
    auto start_ret_future = start_ret_promise.get_future();

    std::promise<bool> add_ok_promise;
    auto add_ok_future = add_ok_promise.get_future();

    std::atomic<bool> got_once{false};

    std::thread loop_thread([&] {
        EventLoop loop;
        auto ch = std::make_shared<Channel>();
        ch->SetEpollinCallback([&] {
            drain_fd(read_fd.get());
            if (!got_once.exchange(true)) {
                got_event_promise.set_value();
            }
            loop.Stop();
        });

        auto ctl_ch = std::make_shared<Channel>();
        ctl_ch->SetEpollinCallback([&] {
            drain_fd(ctl_read.get());
            loop.Stop();
        });

        const bool add_ok = loop.AddFd(read_fd.get(), ch) && loop.AddFd(ctl_read.get(), ctl_ch);
        add_ok_promise.set_value(add_ok);
        ready_promise.set_value();
        if (!add_ok) {
            start_ret_promise.set_value(false);
            return;
        }

        start_ret_promise.set_value(loop.Start());
    });

    ready_future.get();
    const bool add_ok = add_ok_future.get();
    EXPECT_TRUE(add_ok);
    if (!add_ok) {
        EXPECT_FALSE(start_ret_future.get());
        loop_thread.join();
        return;
    }

    // 打断一次 epoll_wait（应被忽略，loop 继续运行）
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    const int kill_rc = ::pthread_kill(loop_thread.native_handle(), SIGUSR1);
    if (kill_rc != 0 && start_ret_future.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready) {
        try_write_byte(ctl_write.get());
    }
    EXPECT_EQ(kill_rc, 0);

    write_byte(write_fd.get());

    const auto got_status = got_event_future.wait_for(std::chrono::milliseconds(500));
    if (got_status != std::future_status::ready &&
        start_ret_future.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready) {
        try_write_byte(ctl_write.get());
    }
    EXPECT_EQ(got_status, std::future_status::ready);
    EXPECT_TRUE(start_ret_future.get());
    loop_thread.join();
}

TEST(EventLoopReliability, RapidStartStopCycles) {
    constexpr int kCycles = 50;

    for (int i = 0; i < kCycles; ++i) {
        int ctl_fds[2]{};
        ASSERT_EQ(::pipe(ctl_fds), 0);
        UniqueFd ctl_read(ctl_fds[0]);
        UniqueFd ctl_write(ctl_fds[1]);
        set_nonblocking(ctl_read.get());

        std::promise<EventLoop*> loop_promise;
        auto loop_future = loop_promise.get_future();

        std::promise<void> ready_promise;
        auto ready_future = ready_promise.get_future();

        std::promise<bool> start_ret_promise;
        auto start_ret_future = start_ret_promise.get_future();

        std::thread t([&] {
            EventLoop loop;
            loop_promise.set_value(&loop);

            auto ctl_ch = std::make_shared<Channel>();
            ctl_ch->SetEpollinCallback([&] {
                drain_fd(ctl_read.get());
                loop.Stop();
            });

            const bool add_ok = loop.AddFd(ctl_read.get(), ctl_ch);
            ready_promise.set_value();
            if (!add_ok) {
                start_ret_promise.set_value(false);
                return;
            }
            start_ret_promise.set_value(loop.Start());
        });

        EventLoop* loop_ptr = loop_future.get();
        ready_future.get();

        // 让 loop 跑起来，然后验证 stop() 能打断 epoll_wait 并返回
        (void)wait_until([&] { return loop_ptr->GetRunStatus(); }, std::chrono::milliseconds(200));
        loop_ptr->Stop();

        const auto st = start_ret_future.wait_for(std::chrono::milliseconds(200));
        if (st != std::future_status::ready) {
            // stop() 没打断住的话，用控制 fd 强制退出，避免卡死
            try_write_byte(ctl_write.get());
        }

        EXPECT_EQ(st, std::future_status::ready) << "cycle=" << i;
        if (st == std::future_status::ready) {
            EXPECT_TRUE(start_ret_future.get()) << "cycle=" << i;
        }
        t.join();
    }
}

TEST(EventLoopReliability, ManyFdsAllCallbacksTriggered) {
    constexpr int kPipes = 64;

    std::vector<UniqueFd> read_fds;
    std::vector<UniqueFd> write_fds;
    read_fds.reserve(kPipes);
    write_fds.reserve(kPipes);

    for (int i = 0; i < kPipes; ++i) {
        int fds[2]{};
        ASSERT_EQ(::pipe(fds), 0);
        read_fds.emplace_back(fds[0]);
        write_fds.emplace_back(fds[1]);
        set_nonblocking(read_fds.back().get());
    }

    int ctl_fds[2]{};
    ASSERT_EQ(::pipe(ctl_fds), 0);
    UniqueFd ctl_read(ctl_fds[0]);
    UniqueFd ctl_write(ctl_fds[1]);
    set_nonblocking(ctl_read.get());

    std::atomic<int> hits{0};
    std::atomic<bool> done{false};

    std::promise<void> ready_promise;
    auto ready_future = ready_promise.get_future();

    std::promise<void> all_promise;
    auto all_future = all_promise.get_future();

    std::promise<bool> add_ok_promise;
    auto add_ok_future = add_ok_promise.get_future();

    std::promise<bool> start_ok_promise;
    auto start_ok_future = start_ok_promise.get_future();

    std::thread loop_thread([&] {
        EventLoop loop;

        auto ctl_ch = std::make_shared<Channel>();
        ctl_ch->SetEpollinCallback([&] {
            drain_fd(ctl_read.get());
            loop.Stop();
        });

        bool add_ok = loop.AddFd(ctl_read.get(), ctl_ch);

        for (int i = 0; i < kPipes; ++i) {
            auto ch = std::make_shared<Channel>();
            ch->SetEpollinCallback([&, i] {
                drain_fd(read_fds[static_cast<size_t>(i)].get());
                const int v = hits.fetch_add(1) + 1;
                if (v == kPipes) {
                    bool expected = false;
                    if (done.compare_exchange_strong(expected, true)) {
                        all_promise.set_value();
                    }
                    loop.Stop();
                }
            });
            add_ok = add_ok && loop.AddFd(read_fds[static_cast<size_t>(i)].get(), ch);
        }

        add_ok_promise.set_value(add_ok);
        ready_promise.set_value();

        if (!add_ok) {
            start_ok_promise.set_value(false);
            return;
        }

        start_ok_promise.set_value(loop.Start());
    });

    ready_future.get();
    const bool add_ok = add_ok_future.get();
    EXPECT_TRUE(add_ok);
    if (!add_ok) {
        EXPECT_TRUE(start_ok_future.get());
        loop_thread.join();
        return;
    }

    for (int i = 0; i < kPipes; ++i) {
        write_byte(write_fds[static_cast<size_t>(i)].get());
    }

    const auto all_status = all_future.wait_for(std::chrono::milliseconds(500));
    if (all_status != std::future_status::ready &&
        start_ok_future.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready) {
        try_write_byte(ctl_write.get());
    }
    EXPECT_EQ(all_status, std::future_status::ready);

    EXPECT_TRUE(start_ok_future.get());
    loop_thread.join();

    EXPECT_EQ(hits.load(), kPipes);
}

TEST(EventLoopReliability, EpollOutTriggersCallback) {
    int fds[2]{};
    ASSERT_EQ(::socketpair(AF_UNIX, SOCK_STREAM, 0, fds), 0);
    UniqueFd server_fd(fds[0]);
    UniqueFd client_fd(fds[1]);
    (void)client_fd;

    set_nonblocking(server_fd.get());

    int ctl_fds[2]{};
    ASSERT_EQ(::pipe(ctl_fds), 0);
    UniqueFd ctl_read(ctl_fds[0]);
    UniqueFd ctl_write(ctl_fds[1]);
    set_nonblocking(ctl_read.get());

    EventLoop loop;

    std::promise<void> out_promise;
    auto out_future = out_promise.get_future();

    std::atomic<bool> fired{false};
    std::atomic<int> out_calls{0};

    auto ch = std::make_shared<Channel>();
    ch->SetEpolloutCallback([&] {
        ++out_calls;
        if (!fired.exchange(true)) {
            out_promise.set_value();
        }
        loop.Stop();
    });

    auto ctl_ch = std::make_shared<Channel>();
    ctl_ch->SetEpollinCallback([&] {
        drain_fd(ctl_read.get());
        loop.Stop();
    });

    ASSERT_TRUE(loop.AddFd(server_fd.get(), ch));
    ASSERT_TRUE(loop.AddFd(ctl_read.get(), ctl_ch));

    std::thread watchdog([&] {
        if (out_future.wait_for(std::chrono::milliseconds(200)) != std::future_status::ready) {
            try_write_byte(ctl_write.get());
        }
    });

    EXPECT_TRUE(loop.Start());
    watchdog.join();

    EXPECT_EQ(out_calls.load(), 1);
}

TEST(EventLoopReliability, ResetChannelTogglesFromInToOut) {
    int fds[2]{};
    ASSERT_EQ(::socketpair(AF_UNIX, SOCK_STREAM, 0, fds), 0);
    UniqueFd server_fd(fds[0]);
    UniqueFd client_fd(fds[1]);
    set_nonblocking(server_fd.get());
    set_nonblocking(client_fd.get());

    int ctl_fds[2]{};
    ASSERT_EQ(::pipe(ctl_fds), 0);
    UniqueFd ctl_read(ctl_fds[0]);
    UniqueFd ctl_write(ctl_fds[1]);
    set_nonblocking(ctl_read.get());

    EventLoop loop;

    std::promise<void> in_promise;
    auto in_future = in_promise.get_future();

    std::promise<void> out_promise;
    auto out_future = out_promise.get_future();

    std::atomic<bool> in_once{false};
    std::atomic<bool> out_once{false};
    std::atomic<bool> reset_ok{false};

    auto ch = std::make_shared<Channel>();
    ch->SetEpollinCallback([&] {
        char buf[64];
        (void)read_some(server_fd.get(), buf, sizeof(buf));

        if (!in_once.exchange(true)) {
            in_promise.set_value();
        }

        ch->RemoveEpollinEvent();
        ch->SetEpolloutCallback([&] {
            if (!out_once.exchange(true)) {
                out_promise.set_value();
            }
            loop.Stop();
        });
        reset_ok.store(loop.ResetChannel(server_fd.get(), ch));
    });

    auto ctl_ch = std::make_shared<Channel>();
    ctl_ch->SetEpollinCallback([&] {
        drain_fd(ctl_read.get());
        loop.Stop();
    });

    ASSERT_TRUE(loop.AddFd(server_fd.get(), ch));
    ASSERT_TRUE(loop.AddFd(ctl_read.get(), ctl_ch));

    std::thread writer([&] {
        const char msg[] = "x";
        (void)::write(client_fd.get(), msg, sizeof(msg));
    });

    std::thread watchdog([&] {
        if (out_future.wait_for(std::chrono::milliseconds(300)) != std::future_status::ready) {
            try_write_byte(ctl_write.get());
        }
    });

    EXPECT_TRUE(loop.Start());
    writer.join();
    watchdog.join();

    EXPECT_EQ(in_future.wait_for(std::chrono::milliseconds(0)), std::future_status::ready);
    EXPECT_EQ(out_future.wait_for(std::chrono::milliseconds(0)), std::future_status::ready);
    EXPECT_TRUE(reset_ok.load());
}

TEST(EventLoopReliability, RemoveThenAddSameFdWorks) {
    int fds[2]{};
    ASSERT_EQ(::pipe(fds), 0);
    UniqueFd read_fd(fds[0]);
    UniqueFd write_fd(fds[1]);
    set_nonblocking(read_fd.get());

    int ctl_fds[2]{};
    ASSERT_EQ(::pipe(ctl_fds), 0);
    UniqueFd ctl_read(ctl_fds[0]);
    UniqueFd ctl_write(ctl_fds[1]);
    set_nonblocking(ctl_read.get());

    EventLoop loop;

    std::promise<void> phase1_promise;
    auto phase1_future = phase1_promise.get_future();

    std::atomic<int> cb1_calls{0};
    std::atomic<int> cb2_calls{0};
    std::atomic<bool> remove_ok{false};
    std::atomic<bool> add_ok{false};
    std::atomic<bool> phase1_once{false};

    auto ch1 = std::make_shared<Channel>();
    ch1->SetEpollinCallback([&] {
        drain_fd(read_fd.get());
        ++cb1_calls;

        if (phase1_once.exchange(true)) {
            return;
        }

        remove_ok.store(loop.RemoveFd(read_fd.get()));

        auto ch2 = std::make_shared<Channel>();
        ch2->SetEpollinCallback([&] {
            drain_fd(read_fd.get());
            ++cb2_calls;
            loop.Stop();
        });

        add_ok.store(loop.AddFd(read_fd.get(), ch2));
        phase1_promise.set_value();
    });

    auto ctl_ch = std::make_shared<Channel>();
    ctl_ch->SetEpollinCallback([&] {
        drain_fd(ctl_read.get());
        loop.Stop();
    });

    ASSERT_TRUE(loop.AddFd(read_fd.get(), ch1));
    ASSERT_TRUE(loop.AddFd(ctl_read.get(), ctl_ch));

    std::thread writer([&] {
        try_write_byte(write_fd.get());
        (void)phase1_future.wait_for(std::chrono::milliseconds(500));
        try_write_byte(write_fd.get());
    });

    std::thread watchdog([&] {
        if (phase1_future.wait_for(std::chrono::milliseconds(500)) != std::future_status::ready) {
            try_write_byte(ctl_write.get());
            return;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        if (cb2_calls.load() == 0) {
            try_write_byte(ctl_write.get());
        }
    });

    EXPECT_TRUE(loop.Start());
    writer.join();
    watchdog.join();

    EXPECT_EQ(cb1_calls.load(), 1);
    EXPECT_EQ(cb2_calls.load(), 1);
    EXPECT_TRUE(remove_ok.load());
    EXPECT_TRUE(add_ok.load());
}

TEST(EventLoopReliability, MultiWriterStressManyFds) {
    constexpr int kPipes = 64;
    constexpr size_t kTargetBytes = 512 * 1024;

    std::vector<UniqueFd> read_fds;
    std::vector<UniqueFd> write_fds;
    read_fds.reserve(kPipes);
    write_fds.reserve(kPipes);

    for (int i = 0; i < kPipes; ++i) {
        int fds[2]{};
        ASSERT_EQ(::pipe(fds), 0);
        read_fds.emplace_back(fds[0]);
        write_fds.emplace_back(fds[1]);
        set_nonblocking(read_fds.back().get());
        set_nonblocking(write_fds.back().get());
    }

    int ctl_fds[2]{};
    ASSERT_EQ(::pipe(ctl_fds), 0);
    UniqueFd ctl_read(ctl_fds[0]);
    UniqueFd ctl_write(ctl_fds[1]);
    set_nonblocking(ctl_read.get());

    EventLoop loop;

    std::atomic<size_t> bytes_read{0};
    std::atomic<bool> done{false};

    auto ctl_ch = std::make_shared<Channel>();
    ctl_ch->SetEpollinCallback([&] {
        drain_fd(ctl_read.get());
        loop.Stop();
    });
    ASSERT_TRUE(loop.AddFd(ctl_read.get(), ctl_ch));

    for (int i = 0; i < kPipes; ++i) {
        auto ch = std::make_shared<Channel>();
        ch->SetEpollinCallback([&, i] {
            char buf[4096];
            while (true) {
                const ssize_t n = read_some(read_fds[static_cast<size_t>(i)].get(), buf, sizeof(buf));
                if (n > 0) {
                    bytes_read.fetch_add(static_cast<size_t>(n));
                    continue;
                }
                if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                    break;
                }
                break;
            }

            if (bytes_read.load() >= kTargetBytes) {
                bool expected = false;
                if (done.compare_exchange_strong(expected, true)) {
                    loop.Stop();
                }
            }
        });
        ASSERT_TRUE(loop.AddFd(read_fds[static_cast<size_t>(i)].get(), ch));
    }

    constexpr int kWriters = 4;
    std::vector<std::thread> writers;
    writers.reserve(kWriters);

    for (int t = 0; t < kWriters; ++t) {
        writers.emplace_back([&, t] {
            std::string payload;
            payload.resize(4096, static_cast<char>('a' + t));

            size_t idx = static_cast<size_t>(t);
            while (!done.load()) {
                idx = (idx + 13) % static_cast<size_t>(kPipes);
                const int fd = write_fds[idx].get();
                const ssize_t n = ::write(fd, payload.data(), payload.size());
                if (n > 0) {
                    continue;
                }
                if (n < 0 && errno == EINTR) {
                    continue;
                }
                if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    continue;
                }
                break;
            }
        });
    }

    std::thread watchdog([&] {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        if (!done.load()) {
            try_write_byte(ctl_write.get());
        }
    });

    EXPECT_TRUE(loop.Start());
    done.store(true);

    for (auto& th : writers) {
        th.join();
    }
    watchdog.join();

    EXPECT_GE(bytes_read.load(), kTargetBytes);
}

TEST(EventLoopReliability, StartMustBeCreatorThread) {
    EventLoop loop;

    std::promise<bool> ret_promise;
    auto ret_future = ret_promise.get_future();

    std::thread t([&] { ret_promise.set_value(loop.Start()); });
    t.join();

    EXPECT_FALSE(ret_future.get());
}
