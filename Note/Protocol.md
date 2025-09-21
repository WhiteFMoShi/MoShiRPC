## unique_ptr的高级使用
首先，要知道`unique_ptr`有两种初始化方式：
1. 直接接管现有指针，也就是本项目中出现的部分：
    ```cpp
    class FreeDeleter {
    public:
        void operator()(char* ptr) const {
            free(ptr);
        }
    };

    char* data_net = cJSON_Print(data_json_);
    // char[]和char*还是不同的，在delete的时候，前者释放整个字符串，后者只释放指针
    std::unique_ptr<char[], FreeDeleter> data_guard(data_net);
    ```
    但是在此种用法中，要确保`FreeDeleter`这个删除器和指针的创建方式保持一致，同时，此处的只能指针的模板中只能使用`char[]`，因为要保持语义一致，若是改为`char*`会报错。

2. 使用`make_unique`创建新对象：
    ```cpp
    std::unique_ptr<char[]> ptr = std::make_unique<char[]>(100);
    ```

`make_unique`是使用`new`进行内存分配的，在本项目中，由于cJSON库是使用`malloc`进行内存分配的，因此在进行接管之后，编写了自己的仿函数`FreeDeleter`进行内存管理。

### char*和char[]的区别
这点主要是在我创建`unique_ptr`的时候出现的，我一开始的写法是：
```cpp
std::unique_ptr<char*, FreeDeleter> data_guard(data_net);
```

我原本一直以为`char*`和`char[]`是一样的，但是实际上它们之间有着本质的区别：==`char*`只是一个指针类型，而`char[]`是数组类型==，同时，前者可以更改指向而后者不行（因为数组一般是指针常量）。

我之所以会有这个误区是因为：在很多时候，数组的头元素往往用`char*`进行指向，并且，在函数传参中，即使我声明为`char[]`也会退化为`char*`，失去其大小信息：
```cpp
void func(char[] arr) { // 错误写法，不支持在函数中直接传数组类型
    // 更改为char*
}
```
即使如此，在定义变量的时候它们之间还是有本质区别的。