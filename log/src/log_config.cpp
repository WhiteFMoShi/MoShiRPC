#include <stdexcept>
#include <fstream>

#include "cJSON/cJSON.h"
#include "log_config.hpp"

LogConfig& LogConfig::getInstance(const std::string& configPath) {
    static LogConfig instance(configPath);
    return instance;
}

LogConfig::LogConfig(const std::string& configPath) {
    workspacePath_ = std::filesystem::current_path();
    
    if (!configPath.empty()) {
        configPath_ = configPath;
    } else {
        configPath_ = workspacePath_ / Defaults::LOG_DIR / "log_config.json";
    }

    if (!loadConfig()) {
        initDefaultConfig();
        saveConfig();
    }
}

bool LogConfig::loadConfig() {
    if (!std::filesystem::exists(configPath_)) {
        return false;
    }

    // 读取JSON文件
    std::ifstream file(configPath_);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open config file: " + configPath_.string());
    }

    // 直接使用字节流的方式快速读取文件中的数据
    std::string jsonContent((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
    file.close();

    // 解析JSON
    cJSON* root = cJSON_Parse(jsonContent.c_str());
    if (!root) {
        throw std::runtime_error("Failed to parse JSON config");
    }

    // 读取配置项
    cJSON* item = nullptr;
    if ((item = cJSON_GetObjectItem(root, ConfigKeys::ASYNC_MODE))) {
        asyncMode_ = cJSON_IsTrue(item);
    }
    if ((item = cJSON_GetObjectItem(root, ConfigKeys::THREAD_POOL_SIZE))) {
        threadPoolSize_ = item->valueint;
    }
    if ((item = cJSON_GetObjectItem(root, ConfigKeys::LOG_DIR))) {
        logDirectory_ = item->valuestring;
    }
    if ((item = cJSON_GetObjectItem(root, ConfigKeys::TERMINAL_PRINT))) {
        printToTerminal_ = cJSON_IsTrue(item);
    }

    cJSON_Delete(root);
    return true;
}

void LogConfig::saveConfig() const {
    // 确保配置目录存在
    std::filesystem::create_directories(configPath_.parent_path());

    // 创建JSON对象
    cJSON* root = cJSON_CreateObject();
    cJSON_AddBoolToObject(root, ConfigKeys::ASYNC_MODE, asyncMode_);
    cJSON_AddNumberToObject(root, ConfigKeys::THREAD_POOL_SIZE, threadPoolSize_);
    cJSON_AddStringToObject(root, ConfigKeys::LOG_DIR, logDirectory_.string().c_str());
    cJSON_AddBoolToObject(root, ConfigKeys::TERMINAL_PRINT, printToTerminal_);

    // 转换为字符串并保存
    char* jsonStr = cJSON_Print(root);
    std::ofstream file(configPath_);
    if (!file.is_open()) {
        cJSON_Delete(root);
        free(jsonStr);
        throw std::runtime_error("Cannot write config file: " + configPath_.string());
    }

    file << jsonStr;
    file.close();

    cJSON_Delete(root);
    free(jsonStr);
}

void LogConfig::initDefaultConfig() {
    asyncMode_ = Defaults::ASYNC_MODE;
    threadPoolSize_ = Defaults::THREAD_POOL_SIZE;
    logDirectory_ = Defaults::LOG_DIR;
    printToTerminal_ = Defaults::TERMINAL_PRINT;
}

// Getter methods
bool LogConfig::isAsyncMode() const { return asyncMode_; }
int LogConfig::getThreadPoolSize() const { return threadPoolSize_; }
bool LogConfig::isPrintToTerminal() const { return printToTerminal_; }
std::filesystem::path LogConfig::getLogDirectory() const { return logDirectory_; }
std::filesystem::path LogConfig::getWorkspacePath() const { return workspacePath_; }


void LogConfig::setAsyncMode(bool async) {
    asyncMode_ = async;
}

void LogConfig::setThreadPoolSize(int size) {
    if (size < 1) {
        throw std::invalid_argument("Thread pool size must be at least 1");
    }
    threadPoolSize_ = size;
}

void LogConfig::setPrintToTerminal(bool print) {
    printToTerminal_ = print;
}

void LogConfig::setLogDirectory(const std::filesystem::path& dir) {
    logDirectory_ = dir;
}

void LogConfig::configure(bool async, int threadPoolSize, 
                         const std::filesystem::path& logDir, bool printToTerminal) {
    setAsyncMode(async);
    setThreadPoolSize(threadPoolSize);
    setLogDirectory(logDir);
    setPrintToTerminal(printToTerminal);
    saveConfig(); // 保存更改到配置文件
}