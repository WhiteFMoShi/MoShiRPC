#include <sstream>
#include <stdexcept>

#include "logFormat.h"

const std::string LogFormat::make_log(Level level, std::string module, const std::string& msg) {
    std::ostringstream oss;
    oss << st_.now() << " ";
    switch (level) {
        case Level::Debug:
            oss << "[Debug]";
            break;
        case Level::Info:
            oss << "[Info]";
            break; 
        case Level::Warning:
            oss << "[Warning]";
            break;
        case Level::Error:
            oss << "[Error]";
            break;
        case Level::Critical:
            oss << "[Critical]";
            break;
        default:
            throw std::runtime_error("No matching Log Level!!!\n");
    }
    oss << " [" << module << "] " << msg << std::endl;
    return oss.str();
}
