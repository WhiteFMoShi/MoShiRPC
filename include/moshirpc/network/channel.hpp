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

    void SetFd(int fd) { fd_ = fd; }
    int  GetFd() const { return fd_; }

    void SetFocusEvents(uint32_t fc_events) { focus_events_ = fc_events; }
    uint32_t GetFocusEvents() const { return focus_events_; }

    void SetEpollinCallback (EpollinCallback cb);
    void SetEpolloutCallback(EpolloutCallback cb);
    void SetEpollerrCallback(EpollerrCallback cb);

    void RemoveEpollinEvent()  { focus_events_ &= ~EPOLLIN; }
    void RemoveEpolloutEvent() { focus_events_ &= ~EPOLLOUT; }

    /**
     * @brief 将ERR对应的callback置空
     * 
     */
    void ClearErrorCallback() { epollerr_cb_ = nullptr; }

    /**
     * @brief 事件处理
     * 
     * @param revent epoll_wait所监听到的fd上触发的事件
     * @return int 1 表示正确处理revent
                    -1 表示出现了ERR或是HUP/RDHUP，但是Channel不会关闭连接
     */
    int HandleEvent(uint32_t revent) const;


    EpollinCallback  GetReadableCallback() const { return epollin_cb_; }
    EpolloutCallback GetWritableCallback() const { return epollout_cb_; }
    EpollerrCallback GetErrorCallback()    const { return epollerr_cb_; }

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