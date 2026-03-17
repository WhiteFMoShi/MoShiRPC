#include "channel.hpp"
#include <utility>
#include <sys/epoll.h>

using moshi::Channel;

void Channel::handle_event(uint32_t revent) const {
    if(revent & focus_events_ & EPOLLERR) { // 错误拥有最高的优先级
        handle_error_();
        return;
    }
    if(revent & focus_events_ & EPOLLIN) {
        if(epollin_cb_ != nullptr)
            epollin_cb_();
    }
    if(revent & focus_events_ & EPOLLOUT) {
        if(epollout_cb_ != nullptr)
            epollout_cb_();
    }
}

void Channel::handle_error_() const {
    if(focus_events_ & EPOLLERR) { // 设置了错误回调才可执行
        if(epollerr_cb_ != nullptr)
            epollerr_cb_();
    }
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
    focus_events_ |= EPOLLERR;
}