#pragma once

#include <string>
#include <sys/types.h>
#include <vector>

struct BufferNode {
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
 * @brief 链式缓冲区，用于存储和管理TCP报文
 * 
 */
class ChainedBuffer {
public:
    /**
    * @brief Construct a new Chained Buffer object
    * 
    * @param node_size 单个
    */
    ChainedBuffer(uint node_size);
    ~ChainedBuffer();

    ChainedBuffer(ChainedBuffer&&) = delete;
    ChainedBuffer& operator=(const ChainedBuffer&) = delete;
    ChainedBuffer(const ChainedBuffer&) = delete;

    /**
     * @brief 向缓冲区中写入长len的数据
     * 
     * @param data 
     * @param len 
     * @return int 写入的字节数
     */
    int write(const void* data, const int len);

    /**
     * @brief 尝试直接获取一个完整的报文，将数据存入ds中
     * 
     * @param ds 
     * @return int 读取的字节数
     */
    int read(void* ds, const int len);

    /**
     * @brief 尝试从p开始，解析一个完整报文
     * 
     * @param p 
     * @return bool true：p开始的报文完整； false：p开始的报文不完整
     */
    bool try_to_parse_from(const BufferNode* p) const;

    bool empty() const;
    void clear();

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
    BufferNode* allocate_node_(); // 单纯的创建node
    void del_node_(BufferNode*); // 单纯的清理node
    void set_node_flag_(BufferNode* node, bool HEAD_FLAG, bool TAIL_FLAG, bool MORE_FLAG);
    void set_node_data_(BufferNode* node, const void* data, int len);

private:
    BufferNode* r_point_;
    BufferNode* w_point_;

    BufferNode* head_node_;
    BufferNode* tail_node_;

    uint node_count_;
    uint SINGLE_NODE_SIZE_OF_KB;
};
