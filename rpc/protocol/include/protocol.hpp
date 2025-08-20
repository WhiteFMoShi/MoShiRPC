#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "cJSON/cJSON.h"

namespace moshi {
const uint32_t PROTOCOL_VERSION_1 = 0x6A5D4C7E;
constexpr int HEADER_LENGTH = 2 * sizeof(uint32_t);

constexpr uint32_t MAX_ALLOWED_LENGTH = 1024 * 1024; // 1MB

class Protocol {
    friend Protocol Deserialize(std::vector<char>, int& p);
    friend void print_network_stream(const Protocol& ptl);
    friend void print_protocol(const Protocol& ptl);
public:
    /**
     * @brief 从一个 cJSON 项构造一个 Protocol 对象
     * 
     * @param item 指向一个cJSON对象的指针，该对象包含要通过网络发送的数据
     *             调用者保留cJSON树的所有权；此构造函数将把数据序列化到内部缓冲区（data_）中
     * @note 此构造函数不会对JSON内容进行深拷贝，而是直接使用std::shared_ptr接管item，因此无需再使用cJSON_Delete手动释放
     * @warning 输入的item必须是一个有效的cJSON对象（非空且格式正确）
     */
    Protocol(cJSON* item);

    /**
     * @brief 解析网络字节流，将其中的数据转换为cJSON对象，并封装到Protocol结构体中
     *
     * @param str 从网络接收到的原始消息，格式为：头部（魔数+长度）+JSON文本
     * @param p 一个指针，用于指示解析到的位置。
     */
    Protocol(const std::vector<char>& str, int& p);

    Protocol& operator=(Protocol&&) = default;

    /**
     * @brief 将Protocol类型的数据序列化为字节流，存储在vector<char>中，便于发向网络
     *
     * @return std::vector<char>
     * @throw 抛出std::runtime_error当cJSON_Print()返回NULL时
     */
    std::vector<char> Serialize() const; 

    /**
     * @brief 重设data_json
     *
     * @param item 
     */
    void reset_json(cJSON* item);
private:
    uint32_t magic_; // Protocol version
    std::shared_ptr<cJSON> data_json_; // json data
};

void print_network_stream(const Protocol& ptl);
void print_protocol(const Protocol& ptl);

/**
 * @brief 将网络字节流反序列化为Protocol
 * 
 * @param network_string 接收的网络字节流
 * @param p 用于标记str中，数据的处理位置
 * @return Protocol 
 */
Protocol Deserialize(const std::vector<char>& network_string, int& p);

} // namespace moshi