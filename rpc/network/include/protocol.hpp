#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "cJSON/cJSON.h"

namespace MoShi {
const uint32_t PROTOCOL_VERSION_1 = 0x6A5D4C7E;
constexpr int HEADER_LENGTH = 2 * sizeof(uint32_t);

class Protocol {
    friend Protocol Deserialize(std::vector<char>, int& p);
    friend void print_network_stream(const Protocol& ptl);
    friend void print_protocol(const Protocol& ptl);
public:
    /**
    * @brief Construct a Protocol object from a cJSON item.
    * 
    * @param item Pointer to a cJSON object containing the data to be sent over the network.
    *                The caller retains ownership of the cJSON tree; this constructor will
    *                serialize the data into an internal buffer (data_).
    * @note This constructor will deep-copy the JSON content. For performance-critical scenarios,
    *       consider using Protocol(const std::string&) with pre-serialized JSON.
    * @warning The input `item` must be a valid cJSON object (non-null and properly formatted).
    */
    Protocol(cJSON* item);

    /**
    * @brief Construct a Protocol object by parsing a network text stream.
    * @param str The raw message received from the network, expected to be in the format:
    *               - Header (magic number + length) + JSON payload (if text mode is enabled).
    *               - Or a pure JSON string (if the protocol is configured for text-only mode).
    * @param p A pointer, notice the position where are parsed.
    */
    Protocol(const std::vector<char>& str, int& p);

    Protocol& operator=(Protocol&&) = default;

    /**
     * @brief Serialize Protocol object to Network text string.
     * @return std::vector<char> 
     * @throw runtime_error When cJSON_Print() return NULL
     */
    std::vector<char> Serialize() const; 

    /**
     * @brief Replace item to data_json_, and delete old data_json_.
     * @param item 
     */
    void reset_json(cJSON* item);
private:
    uint32_t magic_; // Protocol identifier
    // uint32_t length_; // Length of Json's string（目前这个字段留着好像没用）
    std::shared_ptr<cJSON> data_json_; // resource data
};

void print_network_stream(const Protocol& ptl);
void print_protocol(const Protocol& ptl);
Protocol Deserialize(const std::vector<char>& network_string, int& p);

} // namespace MoShi