#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <stdexcept>
#include <vector>
#include <iostream>
#include <cstring>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "protocol.hpp"
#include "cJSON/cJSON.h"

moshi::Protocol::Protocol(cJSON* item) : magic_(PROTOCOL_VERSION_1) {
    data_json_ = std::shared_ptr<cJSON>(item, cJSON_Delete);
}

/*
    现在使用p来进行字符串划分，能在一定程度上解决粘包问题
*/
moshi::Protocol::Protocol(const std::vector<char>& str, int& p) {
    uint32_t magic_net, length_net;
    memcpy(&magic_net, str.data(), sizeof(magic_net));
    memcpy(&length_net, str.data() + sizeof(magic_net), sizeof(length_net));

    magic_ = ntohl(magic_net);
    uint32_t data_length = htonl(length_net);

    if(data_length > MAX_ALLOWED_LENGTH) {
        throw std::runtime_error("Prortocol::Protocol(const std::vector<char>&): Invalid data length!!!)");
    }

    p += HEADER_LENGTH + data_length;

    if(magic_ != PROTOCOL_VERSION_1) 
        throw std::runtime_error("Protocol::Protocol(const std::vector<char>&): Invalid magic number!");

    cJSON* data_parsed = cJSON_ParseWithLength(str.data() + HEADER_LENGTH, data_length);
    if(data_parsed == nullptr) {
        throw std::runtime_error("Protocol::Protocol(const std::vector<char>&): Failure parse data!");
    }
    data_json_ = std::shared_ptr<cJSON>(data_parsed,
                                        cJSON_Delete); // 指定删除器
}

std::vector<char> moshi::Protocol::Serialize() const {
    class FreeDeleter {
    public:
        void operator()(char* ptr) const {
            free(ptr);
        }
    };

    uint32_t magic_net = htonl(magic_);
    // data_net可以实现一个包装器进行优化，就不用手动free了
    char* data_net = cJSON_Print(data_json_.get());
    // char[]和char*还是不同的，在delete的时候，前者释放整个字符串，后者只释放指针
    std::unique_ptr<char[], FreeDeleter> data_guard(data_net);
    if(data_net == nullptr) {
        throw std::runtime_error("protocol::Serialize(): Failure parse to Json!");
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

void moshi::Protocol::reset_json(cJSON* item) {
    if(item == nullptr) {
        throw std::invalid_argument("Protocol::reset_json(cJSON*): The argument is nullptr!");
    }
    data_json_.reset(item, cJSON_Delete); // 安全释放旧指针，接管新指针
}

void moshi::print_network_stream(const Protocol& ptl) {
    // 输出的应该是Json字符流加上前面8字节的二进制数
    int i = 0;
    auto str = ptl.Serialize();
    for(auto& temp : str) {
        if(i < 8) {
            // 前8个字节可能会对应ASCII中的特殊字符，打印不出来，只能使用16进制
            printf("%02x ", temp);
            i++;
        }
        else
            printf("%c", temp);
    }
    std::cout << std::endl;
}

void moshi::print_protocol(const Protocol& ptl) {
    printf("%02x ", ptl.magic_);
    char* str = cJSON_Print(ptl.data_json_.get());
    printf("\n Json: %s", str);
    free(str);
}

// 这里存在一个拷贝问题，不知道是否会影响，若是编译器优化了return不会将对象释放，就不会有问题
// 后续应该换成完美转发来处理吧？
moshi::Protocol moshi::Deserialize(const std::vector<char>& network_string, int& p) {
    return Protocol(network_string, p);
}