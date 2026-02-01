#pragma once

#include <functional>
#include <sys/epoll.h>
#include <sys/types.h>

// class EventLoop;

class Channel {
public:
    void set_fd(int fd) { fd_ = fd; }
    int  get_fd() const { return fd_; }

    void set_epoll_events(epoll_event events) { epoll_events_ = events; }
    epoll_event* get_epoll_events() { return &epoll_events_; }

    using EpollinCallback  = std::function<void()>;
    using EpolloutCallback = std::function<void()>;
    using EpollerrCallback = std::function<void()>;

    void set_epollin_callback (EpollinCallback cb);
    void set_epollout_callback(EpolloutCallback cb);
    void set_epollerr_callback(EpollerrCallback cb);

    void remove_epollin_callback()  { epoll_events_.events &= ~EPOLLIN; }
    void remove_epollout_callback() { epoll_events_.events &= ~EPOLLOUT; }
    void remove_epollerr_callback() { epoll_events_.events &= ~EPOLLERR; }

    EpollinCallback  get_readable_callback() const { return epollin_cb_; }
    EpolloutCallback get_writable_callback() const { return epollout_cb_; }
    EpollerrCallback get_error_callback()     const { return epollerr_cb_; }
    
    void handle_event() const;
    void handle_error() const;
private:
    EpollinCallback  epollin_cb_;
    EpolloutCallback epollout_cb_;
    EpollerrCallback epollerr_cb_;

    epoll_event epoll_events_;
    int fd_;
};