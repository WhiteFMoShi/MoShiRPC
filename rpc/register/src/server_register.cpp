#include <string>
#include <vector>

#include "server_register.hpp"

bool ServerRegister::register_service(FunctionMeta funcmeta) {
}

std::vector<std::string> ServerRegister::get_services_list() const {
    std::vector<std::string> ret(counter_);
    int p = 0;
    for(auto& [sign, _] : services_) {
        ret[p++] = sign;
    }
    return ret;
}

bool ServerRegister::delete_service(const std::string& sign) {
    if(services_.count(sign)) {
        services_.erase(sign);
        return true;
    }
    return false;
}

bool ServerRegister::sign_check_(const std::string&) const {
    
}

std::string ServerRegister::generated_sign_(std::string func_name, AnyFunction callable) const {
    std::string sign = "";
    sign += func_name;
}
