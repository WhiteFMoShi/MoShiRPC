#pragma once

#include <string>
#include <sys/types.h>
#include <vector>

/**
 * @brief 缓冲区节点,进行数据的实际存储
 * 
 */
struct BufferNode {
    BufferNode(uint node_size) : next(nullptr), prev(nullptr), 
                                MORE_FLAG(false), HEAD_FLAG(false), TAIL_FLAG(false), 
                                used_size(0), read_offset(0), write_offset(0), 
                                data(new char[node_size]) {}
    ~BufferNode() {
        if(data != nullptr) {
            delete[] data;
            data = nullptr;
        }
    }

    /**
     * @brief 删除缓冲区节点中的数据存储区，释放内存
     * 
     */
    void delete_node_data() {
        if(data != nullptr) {
            delete[] data;
            data = nullptr;
        }
    }

    /**
     * @brief 清空缓冲区节点的数据，不删除节点
     * 
     */
    void clear_node() {
        used_size = 0;
        read_offset = 0;
        write_offset = 0;

        MORE_FLAG = false;
        HEAD_FLAG = false;
        TAIL_FLAG = false;
    }

    bool is_used() const { return used_size > 0; }

public:
    BufferNode* next;
    BufferNode* prev;

    bool MORE_FLAG; // More_Node(false/true)
    bool HEAD_FLAG; // HEAD_NODE
    bool TAIL_FLAG; // TAIL_NODE

    // 读写控制
    uint used_size; // 实际使用的大小
    uint read_offset;
    uint write_offset;

    char* data; // buffer
};

/**
 * @brief 链式缓冲区,仅仅存储数据,不进行其它的任何操作
 * 
 */
class ChainedBuffer {
public:
    /**
    * @brief Construct a new Chained Buffer object
    * 
    * @param node_size 单个缓冲区节点的大小，单位为BYTE
    * @param max_node_count 最大缓冲区节点数量
    */
    ChainedBuffer(uint node_size, uint max_node_count);
    ~ChainedBuffer();

    ChainedBuffer(ChainedBuffer&&) = delete;
    ChainedBuffer& operator=(const ChainedBuffer&) = delete;
    ChainedBuffer(const ChainedBuffer&) = delete;

    /**
     * @brief 向缓冲区中写入长len的数据
     * 
     * @param data 
     * @param len 
     * @return int 写入的字节数, 写入失败返回-1
     */
    int write(const void* data, const int len);

    /**
     * @brief 尝试直接获取一个完整的报文，将数据存入ds中
     * 
     * @param ds 
     * @return int 读取的字节数, 读取失败返回-1
     */
    int read(void* ds, const int len);

    bool empty() const { return size_ == 0; }
    void clear();

    uint get_node_count() const { return size_; }

private:
    /**
    * @brief 将ds插在rs之后
    * 
    * @param ds
    * @param rs 
    */
    void insert_(BufferNode* ds, BufferNode* rs);

    /**
     * @brief 删除ds结点
     * 
     * @param ds 
     */
    void remove_(BufferNode* ds);

    /* node操作 */
    BufferNode* create_node_(); // 创建node
    void del_node_(BufferNode*); // 删除node
    void set_node_flag_(BufferNode* node, bool HEAD_FLAG, bool TAIL_FLAG, bool MORE_FLAG);
    bool set_node_data_(BufferNode* node, const void* data, int len);

private:
    BufferNode* r_point_;
    BufferNode* w_point_;

    BufferNode* head_node_;
    BufferNode* tail_node_;

    uint size_;
    uint capacity_;
    uint SINGLE_NODE_SIZE_OF_BYTE;
    uint MAX_NODE_COUNT;
};
