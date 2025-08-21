#pragma once

#include <any>
#include <functional>
#include <map>
#include <string>
#include <typeindex>
#include <vector>

using AnyFunction = std::function<std::any(const std::vector<std::any>&)>;

struct FunctionMeta {
    std::string func_name;
    std::type_index return_type;
    std::vector<std::type_index> arguments_type;

    AnyFunction callable;
};

class ServerRegister {
public:
    ServerRegister() : counter_(0) {}

    /**
     * @brief 将函数注册为RPC服务器的可调用服务  
     * 
     * @param sign eg：func(int, std::string)->void
     * @param callable sign对应的可调用对象
     * @return true 注册成功
     * @return false 注册失败（已经注册了相同签名的服务）
     */
    bool register_service(FunctionMeta funcmeta);

    /**
     * @brief 获取服务签名列表
     * 
     * @return std::vector<std::string> 
     */
    std::vector<std::string> get_services_list() const;

    /**
     * @brief 
     * 
     * @return true 
     * @return false 
     */
    bool delete_service(const std::string&);

private:
    bool sign_check_(const std::string&) const;
    std::string generated_sign_(std::string func_name, AnyFunction) const;
    
private:
    std::map<std::string, AnyFunction> services_;
    int counter_; // 已添加的服务数量
};