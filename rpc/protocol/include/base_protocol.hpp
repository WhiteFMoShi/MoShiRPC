#pragma once

#include <cstdint>
#include <memory>
#include <vector>

namespace moshi {
    
    class BaseProtocol {
    public:
        virtual int serialize(const void* data, const int len, void* dest, const int dest_len) = 0;
        virtual int deserialize(const void* data, const int len, void* dest, const int dest_len) = 0;
        
        virtual ~BaseProtocol() = default;
    };
}