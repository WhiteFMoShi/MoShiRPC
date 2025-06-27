#include <cstdint>
#include <cstdlib>
#include <memory>
#include <stdexcept>
#include <vector>
#include <string>
#include <cstring>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "protocol.hpp"
#include "cJSON/cJSON.h"

using namespace MoShi;

Protocol::Protocol(cJSON* item) {
    magic_ = PROTOCOL_VERSION_1;
    data_json_ = item;
}

Protocol::~Protocol() {
    cJSON_Delete(data_json_);
}

std::vector<char> Protocol::Serialize() const {
    class FreeDeleter {
    public:
        void operator()(char* ptr) const {
            free(ptr);
        }
    };

    uint32_t magic_net = htonl(magic_);
    // data_net可以实现一个包装器进行优化，就不用手动free了
    char* data_net = cJSON_Print(data_json_);
    // char[]和char*还是不同的，在delete的时候，前者释放整个字符串，后者只释放指针
    std::unique_ptr<char[], FreeDeleter> data_guard(data_net);
    if(data_net == nullptr) {
        throw std::runtime_error("protocol::Serialize Json parse error!");
    }
    uint32_t data_length = static_cast<uint32_t>(strlen(data_net));
    uint32_t data_length_net = htonl(data_length);

    std::vector<char> buffer(sizeof(magic_net) + sizeof(data_length_net) + data_length);
    char* ptr = buffer.data();
    memcpy(ptr, &magic_net, sizeof(magic_net));
    ptr += sizeof(magic_net);
    memcpy(ptr, &data_length_net, sizeof(data_length_net));
    ptr += sizeof(data_length_net);
    memcpy(ptr, data_net, data_length);
    
    return buffer;
}

void Protocol::reset_json(cJSON* item) {
    cJSON_Delete(data_json_);
    data_json_ = item;
}

Protocol Deserialize(std::string network_string) {
    char* ptr = network_string.data();
    uint len = network_string.size();
    uint32_t magic = static_cast<uint32_t>(*ptr);   
}