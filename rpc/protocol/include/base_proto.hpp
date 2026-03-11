#pragma once

#include <string>

namespace moshi {
    
    class BaseProtocol {
    public:
        virtual void serialize(std::string str) = 0;
        virtual std::string deserialize() = 0;
    };

}
