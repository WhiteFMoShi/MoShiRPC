#pragma once

#include <cstdint>
#include <functional>
#include <sys/epoll.h>
#include <sys/types.h>

// class EventLoop;

namespace moshi{

/**
* @brief 专注于进行事件的解耦
* 
*/
class Channel {
public:
    using EpollinCallback  = std::function<void()>;
    using EpolloutCallback = std::function<void()>;
    using EpollerrCallback = std::function<void()>;

public:
    Channel(EpollerrCallback err_cb = nullptr) :
        epollin_cb_(nullptr), epollout_cb_(nullptr), epollerr_cb_(err_cb), 
        fd_(-1), focus_events_(0) {}

    void set_fd(int fd) { fd_ = fd; }
    int  get_fd() const { return fd_; }

    void set_focus_events(uint32_t fc_events) { focus_events_ = fc_events; }
    uint32_t get_focus_events() const { return focus_events_; }

    void set_epollin_callback (EpollinCallback cb);
    void set_epollout_callback(EpolloutCallback cb);
    void set_epollerr_callback(EpollerrCallback cb);

    void remove_epollin_event()  { focus_events_ &= ~EPOLLIN; }
    void remove_epollout_event() { focus_events_ &= ~EPOLLOUT; }

    /**
     * @brief 将ERR对应的callback置空
     * 
     */
    void clear_error_callback() { epollerr_cb_ = nullptr; }

    /**
     * @brief 事件处理
     * 
     * @param revent epoll_wait所监听到的fd上触发的事件
     * @return int 1 表示正确处理revent
                    -1 表示出现了ERR或是HUP/RDHUP，但是Channel不会关闭连接
     */
    int handle_event(uint32_t revent) const;


    EpollinCallback  get_readable_callback() const { return epollin_cb_; }
    EpolloutCallback get_writable_callback() const { return epollout_cb_; }
    EpollerrCallback get_error_callback()    const { return epollerr_cb_; }

private:
    // 事件对应的回调函数
    EpollinCallback  epollin_cb_;
    EpolloutCallback epollout_cb_;
    EpollerrCallback epollerr_cb_;

    // epoll_event focus_events_; // 关注的事件handle
    int fd_;
    uint32_t focus_events_;
};

} // namespace moshi