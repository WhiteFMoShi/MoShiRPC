#include "channel.hpp"
#include "event_loop.hpp"

using moshi::Channel;

void Channel::handle_event() const {
    if(epoll_events_.events & EPOLLERR) { // 错误拥有最高的优先级
        handle_error();
        return;
    }
    if(epoll_events_.events & EPOLLIN) {
        epollin_cb_();
    }
    if(epoll_events_.events & EPOLLOUT) {
        epollout_cb_();
    }
}

void Channel::handle_error() const {
    if(epoll_events_.events & EPOLLERR) { // 设置了错误回调才可执行
        epollerr_cb_();
    }
}

void Channel::set_epollout_callback(EpolloutCallback cb) {
    epollout_cb_ = std::move(cb);
    epoll_events_.events |= EPOLLOUT;
}

void Channel::set_epollin_callback(EpollinCallback cb) {
    epollin_cb_ = std::move(cb);
    epoll_events_.events |= EPOLLIN;
}

void Channel::set_epollerr_callback(EpollerrCallback cb) {
    epollerr_cb_ = std::move(cb);
    epoll_events_.events |= EPOLLERR;
}