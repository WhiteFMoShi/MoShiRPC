#include "log.hpp"

using namespace MoShi;

int main() {
    Log& logger = Log::getInstance();

    logger.addLog(LogLevel::Info, "test", "Info level");
    logger.addLog(LogLevel::Debug, "test", "Debug level");
    logger.addLog(LogLevel::Error, "test", "Error level");
    logger.addLog(LogLevel::Warning, "test", "warning level");
    logger.addLog(LogLevel::Critical, "test", "Critical level");

}