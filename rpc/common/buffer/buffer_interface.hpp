#pragma once

#include <cstddef>
#include <sys/types.h>

namespace moshi {

class BufferInterface {
public:
    virtual ~BufferInterface() = default;

    virtual ssize_t write(const void* buf, const size_t size) = 0;
    virtual ssize_t read(void* buf, const size_t size) = 0;

    virtual const char* peek() const = 0;
    virtual void consume(const size_t size) = 0;

    virtual size_t writable_bytes() const = 0;
    virtual size_t readable_bytes() const = 0;
    virtual bool empty() const = 0;
    virtual size_t size() const = 0;
};

} // namespace moshi