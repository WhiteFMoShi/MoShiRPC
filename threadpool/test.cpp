#include <iostream>

class MyClass {
private:
    int value;
public:
    MyClass(int val = 0) : value(val) {}

    // 返回引用类型
    MyClass operator=(const MyClass& other) {
        if (this != &other) {
            this->value = other.value;
        }
        return *this;
    }

    int getValue() const {
        return value;
    }
};

int main() {
    MyClass a(1), b(2), c(3);
    a = b = c;
    std::cout << a.getValue() << std::endl;
    return 0;
}