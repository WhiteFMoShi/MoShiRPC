#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>

#include "protocol.hpp"
#include "cJSON/cJSON.h"

// using namespace std;
namespace fs = std::filesystem;  // 命名空间别名简化

std::string ReadJsonFileToString(const fs::path& file_path) {
    // 检查文件是否存在
    if (!fs::exists(file_path)) {
        throw std::runtime_error("File not found: " + file_path.string());
    }

    // 打开文件并读取内容
    std::ifstream file(file_path, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + file_path.string());
    }

    // 获取文件大小
    const auto file_size = fs::file_size(file_path);
    std::string content(file_size, '\0');

    // 读取全部内容
    file.read(content.data(), file_size);
    return content;
}

int main() {
    // 1. 读取 JSON 文件内容
    const std::string json_str = ReadJsonFileToString("test.json");
    
    // 2. 解析为 cJSON 对象
    cJSON* json_data = cJSON_Parse(json_str.c_str());
    if (!json_data) {
        throw std::runtime_error("Failed to parse JSON: " + std::string(cJSON_GetErrorPtr()));
    }

    moshi::Protocol ptl(json_data);

    moshi::print_network_stream(ptl);

    // MoShi::Deserialize(ptl.Serialize());

}