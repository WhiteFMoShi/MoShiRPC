#pragma once

#include <string>
#include <filesystem>

class LogConfig {
public:
    // 获取配置单例
    static LogConfig& getInstance(const std::string& configPath = "");
    
    // 配置访问接口
    bool isAsyncMode() const;
    int getThreadPoolSize() const;
    bool isPrintToTerminal() const;
    std::filesystem::path getLogDirectory() const;
    std::filesystem::path getWorkspacePath() const;

    // 禁用拷贝和移动
    LogConfig(const LogConfig&) = delete;
    LogConfig& operator=(const LogConfig&) = delete;
    LogConfig(LogConfig&&) = delete;
    LogConfig& operator=(LogConfig&&) = delete;

public:
    void setAsyncMode(bool async);
    void setThreadPoolSize(int size);
    void setPrintToTerminal(bool print);
    void setLogDirectory(const std::filesystem::path& dir);

    // 添加一个方法用于一次性设置所有配置
    void configure(bool async, int threadPoolSize, 
                  const std::filesystem::path& logDir, bool printToTerminal);

private:
    // 配置项键名定义
    struct ConfigKeys {
        static constexpr const char* ASYNC_MODE = "asyncMode";
        static constexpr const char* THREAD_POOL_SIZE = "threadPoolSize";
        static constexpr const char* LOG_DIR = "logDirectory";
        static constexpr const char* TERMINAL_PRINT = "printToTerminal";
    };

    // 默认配置值
    struct Defaults {
        static constexpr bool ASYNC_MODE = false;
        static constexpr int THREAD_POOL_SIZE = 1;
        static constexpr const char* LOG_DIR = "logs";
        static constexpr bool TERMINAL_PRINT = true;
    };

    LogConfig(const std::string& configPath = "");
    ~LogConfig() = default;

    /**
     * @brief 加载配置文件
     * 
     * @return true 
     * @return false 
     */
    bool loadConfig();
    /**
     * @brief 保存当前配置到文件
     */
    void saveConfig() const;
    /**
     * @brief 初始化默认配置
     */
    void initDefaultConfig();
    
    std::filesystem::path resolveConfigPath() const;

private:
    std::filesystem::path configPath_;
    std::filesystem::path workspacePath_;
    
    // 配置数据
    bool asyncMode_{Defaults::ASYNC_MODE};
    int threadPoolSize_{Defaults::THREAD_POOL_SIZE};
    std::filesystem::path logDirectory_{Defaults::LOG_DIR};
    bool printToTerminal_{Defaults::TERMINAL_PRINT};
};