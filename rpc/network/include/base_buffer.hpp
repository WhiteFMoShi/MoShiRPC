#pragma once

#include <cctype>

namespace moshi {

class BaseBuffer {
public:
    virtual ~BaseBuffer() = 0;

    virtual int read(int fd) = 0;
    virtual int write(int fd) = 0;

    virtual bool empty() const = 0;
};
} // namespace mosh