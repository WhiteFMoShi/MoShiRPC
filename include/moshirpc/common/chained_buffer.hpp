#pragma once

#include <cstddef>
#include <cstdint>
#include <sys/types.h>

namespace moshi {
/**
 * @brief 缓冲区节点,进行数据的实际存储
 * 
 */
struct BufferNode {
    explicit BufferNode(std::size_t node_size);
    ~BufferNode();

    /**
     * @brief 删除缓冲区节点中的数据存储区，释放内存
     * 
     */
    void ReleaseNode();

    /**
     * @brief 重置缓冲区节点的状态，将其标记为未使用状态
     * 
     */
    void ResetNode();

    bool IsUsed() const { return used_size > 0; }

    ssize_t WriteableSize() const;
    ssize_t ReadableSize() const;

public:
    BufferNode* next;
    BufferNode* prev;

    // 读写控制
    std::size_t capacity;  // data 缓冲区容量（byte）
    std::size_t used_size;   // 实际写入的大小（byte）
    std::size_t read_offset; // 读取偏移量（byte）
    char* data; // buffer
};

/**
 * @brief 链式缓冲区,仅仅存储数据,不进行其它的任何操作,仅仅实现最基本的缓冲区操作
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
    ChainedBuffer(std::size_t node_size, std::size_t max_node_count);
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
    int Write(const void* data, const int len);

    /**
     * @brief 尝试直接获取一个完整的报文，将数据存入ds中
     * 
     * @param dest 
     * @return int 读取的字节数, 读取失败返回-1
     */
    int Read(void* dest, const std::size_t dest_len);

    bool Empty() const;

    /**
     * @brief 清空缓冲区中的所有数据结点
     * @details 清除数据结点,但是不会清除head_node_(哨兵结点)
     * 
     */
    void Clear();

    /**
     * @brief 该函数比较特别，返回的是链表中已使用的节点的个数，而不是其中的数据大小
     * 
     * @details 该函数的原意应该是要获得可读的数据字节长度，但是由于设计问题，实现极为困难
     * @return int 可读的节点个数
     */
    int Size() const;

    // Backward-compat for existing tests / old naming.
    int get_node_count() const { return Size(); }

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

    std::size_t size_;     // 已使用节点个数（不含哨兵）
    std::size_t capacity_; // 当前已分配的数据节点个数（不含哨兵）
    const std::size_t SINGLE_NODE_SIZE_OF_BYTE;
    const std::size_t MAX_NODE_COUNT;
};

} // namespace moshi
