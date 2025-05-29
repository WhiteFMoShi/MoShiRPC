#include <sstream>
#include <stdexcept>

#include "log_format.hpp"

// #define LOGFORMAT_DEBUG

std::string LogFormat::makeLog(LogLevel level, const std::string& module, const std::string& time, const std::string& msg) {
    std::ostringstream oss;
    splicing_(oss, level, module, time, msg);

#ifdef LOGFORMAT_DEBUG
    std::cout << oss.str() << std::endl;
#endif

    return oss.str();
}

std::string LogFormat::makeLogln(LogLevel level, const std::string& module, const std::string& time, const std::string& msg) {
    std::ostringstream oss;
    splicing_(oss, level, module, time, msg);
    oss << '\n';

#ifdef LOGFORMAT_DEBUG
    std::cout << oss.str() << std::endl;
#endif

    return oss.str();
}

void LogFormat::splicing_(std::ostringstream& oss, LogLevel level, const std::string& module, const std::string& time, const std::string& msg) {
    oss << time << " ";
    switch (level) {
        case LogLevel::Debug:
            oss << "[Debug]";
            break;
        case LogLevel::Info:
            oss << "[Info]";
            break; 
        case LogLevel::Warning:
            oss << "[Warning]";
            break;
        case LogLevel::Error:
            oss << "[Error]";
            break;
        case LogLevel::Critical:
            oss << "[Critical]";
            break;
        default:
            throw std::runtime_error("No matching Log Level!!!");
    }
    oss << " [" << module << "] " << msg;
}
