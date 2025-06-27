#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "cJSON/cJSON.h"

const uint32_t PROTOCOL_VERSION_1 = 0x6A5D4C7E;

namespace MoShi {

class Protocol {
    friend Protocol Deserialize(std::string network_string);
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
    * @param msg The raw message received from the network, expected to be in the format:
    *               - Header (magic number + length) + JSON payload (if text mode is enabled).
    *               - Or a pure JSON string (if the protocol is configured for text-only mode).
    */
    Protocol(std::string msg);

    /**
     * @brief Uses RAII to automatically call cJSON_Delete() and release the memory of data_json_.
     */
    ~Protocol();

    /**
     * @brief Serialize Protocol object to Network text string.
     * @return std::vector<char> 
     * @throw runtime_error When cJSON_Print() return NULL
     */
    std::vector<char> Serialize() const; 
    std::string Print() const;
    /**
     * @brief Replace item to data_json_, and delete old data_json_.
     * @param item 
     */
    void reset_json(cJSON* item);
private:
    uint32_t magic_; // Protocol identifier
    std::string data_string_; // Json to string
    cJSON* data_json_; // resource data
};

Protocol Deserialize(std::string network_string);

} // namespace MoShi