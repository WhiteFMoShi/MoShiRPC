#pragma once

#include <functional>
#include <mutex>
#include <condition_variable>
#include <chrono>

class AdvancedConditionalTimer {
public:
    void start_ms(const std::chrono::milliseconds& ms, std::function<void()> callback);
    void start_s(const std::chrono::seconds& s, std::function<void()> callback);
};