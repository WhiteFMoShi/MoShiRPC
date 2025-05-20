#pragma once

#include <string>

class LogFileManager {
public:
    LogFileManager();

    void createLogFile();
private:
    std::string log_dir_;
};