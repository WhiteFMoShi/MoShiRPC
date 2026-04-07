#include "channel.hpp"
#include <unistd.h>
#include <utility>
#include <sys/epoll.h>

using moshi::Channel;

int Channel::handle_event(uint32_t revent) const {
    if(revent & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) { // 错误拥有最高的优先级
        if(epollerr_cb_ != nullptr)
            epollerr_cb_();
        return -1;
    }
    if(revent & focus_events_ & EPOLLIN) {
        if(epollin_cb_ != nullptr)
            epollin_cb_();
    }
    if(revent & focus_events_ & EPOLLOUT) {
        if(epollout_cb_ != nullptr)
            epollout_cb_();
    }
    return 1;
}

void Channel::set_epollout_callback(EpolloutCallback cb) {
    epollout_cb_ = std::move(cb);
    focus_events_ |= EPOLLOUT;
}

void Channel::set_epollin_callback(EpollinCallback cb) {
    epollin_cb_ = std::move(cb);
    focus_events_ |= EPOLLIN;
}

void Channel::set_epollerr_callback(EpollerrCallback cb) {
    epollerr_cb_ = std::move(cb);
}