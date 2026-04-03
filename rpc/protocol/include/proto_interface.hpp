#pragma once

#include <string>

namespace moshi {
    /**
     * @brief Protocol接口
     * 
     */
    class ProtocInterface {
    public:
        virtual ~ProtocInterface() = default;

        virtual const std::string& serialize(const ProtocInterface&) = 0;
        virtual const std::string& serialize(ProtocInterface&&) = 0;
        virtual void deserialize(const std::string&) = 0;
        virtual void deserialize(std::string&&) = 0;

        
    };

}
