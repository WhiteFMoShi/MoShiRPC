#pragma once

#include <sys/types.h>

namespace moshi {
/**
 * @brief 缓冲区节点,进行数据的实际存储
 * 
 */
struct BufferNode {
    BufferNode(uint node_size);
    ~BufferNode();

    /**
     * @brief 删除缓冲区节点中的数据存储区，释放内存
     * 
     */
    void release_data_memory();

    /**
     * @brief 重置缓冲区节点的状态，将其标记为未使用状态
     * 
     */
    void reset_node();

    bool is_used() const { return used_size > 0; }

    bool is_special() const { return SPECIAL_FLAG; } // 看是不是哨兵
public:
    BufferNode* next;
    BufferNode* prev;

    bool SPECIAL_FLAG; // 哨兵位(标识head_node_)
    bool MORE_FLAG; // More_Node(false/true)
    bool HEAD_FLAG; // HEAD_NODE
    bool TAIL_FLAG; // TAIL_NODE

    // 读写控制
    uint used_size; // 实际使用的大小
    uint read_offset; // 读取偏移量(断点续读)
    char* data; // buffer
};

/**
 * @brief 链式缓冲区,仅仅存储数据,不进行其它的任何操作
 * @details 当前的设计,在缓冲区的开始会有一个“哨兵位”的存在
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
     * @param dest 
     * @return int 读取的字节数, 读取失败返回-1
     */
    int read(void* dest, const uint dest_len);

    bool empty() const { return size_ == 0; }

    /**
     * @brief 清空缓冲区中的所有数据结点
     * @details 清除数据结点,但是不会清楚head_node_(哨兵结点)
     * 
     */
    void clear();

    uint get_node_count() const { return size_; }

private:
    /**
    * @brief 将ds插在rs之后
    * 
    * @param dest
    * @param resource
    * @return int 成功返回0, 失败返回-1
    */
    int insert_(BufferNode* dest, BufferNode* resource);

    /**
     * @brief 删除ds结点
     * 
     * @param dest 
     * @return int 成功返回0, 失败返回-1
     */
    int remove_(BufferNode* dest);

    /* node操作 */
    BufferNode* create_node_(); // 创建node
    int del_node_(BufferNode*); // 删除node

    void set_node_flag_(BufferNode* node, bool HEAD_FLAG, bool TAIL_FLAG, bool MORE_FLAG);
    bool set_node_data_(BufferNode* node, const void* data, int len);
    
    /**
     * @brief 将一个可回收的结点移动至缓冲区的末尾
     * 
     * @param node 
     */
    bool recycle_node_(BufferNode* node);
private:
    BufferNode* r_point_;
    BufferNode* w_point_;

    BufferNode* head_node_;
    BufferNode* tail_node_;

    uint size_;
    uint capacity_;
    const uint SINGLE_NODE_SIZE_OF_BYTE;
    const uint MAX_NODE_COUNT;
};

} // namespace moshi