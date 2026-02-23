#include "error.hpp"

moshi::Error::Error(int err_code, const char* file_name, const char* func_name, const char* err_msg)
    : err_code_(err_code), file_name_(file_name), func_name_(func_name), err_msg_(err_msg) {}

const char* moshi::Error::what() const noexcept {
    
    return str_;
};

