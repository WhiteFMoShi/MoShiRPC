#pragma once

#include <exception>

namespace moshi {

struct Error : public std::exception {
public:
    Error() = delete;
    Error(int err_code, const char* file_name, const char* func_name, const char* err_msg);
    const char* what() const noexcept override;
private:
    int err_code_;
    const char* file_name_;
    const char* func_name_;
    const char* err_msg_;
    const char* str_;
};

} // namespace moshi