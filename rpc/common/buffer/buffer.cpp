#include "buffer.hpp"
#include <cstring>

ChainedBuffer::ChainedBuffer(uint node_size) : r_point_(nullptr), w_point_(nullptr),
                                head_node_(nullptr), tail_node_(nullptr),
                                size_(0), SINGLE_NODE_SIZE_OF_BYTE(node_size)
{
}

ChainedBuffer::~ChainedBuffer() {
    clear();
}

int ChainedBuffer::write(const void* data, int len) {
    int offset = 0;
    BufferNode* temp_node = nullptr;
    BufferNode* mark = tail_node_;
    while((temp_node = create_node_()) != nullptr) {
        if(len > SINGLE_NODE_SIZE_OF_BYTE) { // 一个结点放不下
            set_node_data_(temp_node, static_cast<const char*>(data) + offset, SINGLE_NODE_SIZE_OF_BYTE);
            set_node_flag_(temp_node, offset == 0, false, true);
            offset += SINGLE_NODE_SIZE_OF_BYTE;
        }
        else { // 能放下
            set_node_data_(temp_node, static_cast<const char*>(data) + offset, len - offset);
            offset += len - offset;
            set_node_flag_(temp_node, offset == 0, true, false);
        }
        // 链接结点
        insert_(tail_node_, temp_node);
    }
    // 存在分配失败的情况，需要回滚(不实际删除结点!)
    if(temp_node == nullptr) {
        while(temp_node != mark) {
            temp_node->clear_node();
        }
    }
    return offset;
}

void ChainedBuffer::insert_(BufferNode* ds, BufferNode* rs) {
    ds->next = rs;
    rs->prev = ds;
    if(rs->is_used()) {
        size_++;
    }
    capacity_++;
}

BufferNode* ChainedBuffer::create_node_() {
    BufferNode* node = new BufferNode(SINGLE_NODE_SIZE_OF_BYTE);
    capacity_++;
    return node;
}

void ChainedBuffer::del_node_(BufferNode* node) {
    if(node == nullptr) {
        return;
    }
    if(node->prev != nullptr) {
        node->prev->next = node->next;
    }
    if(node->next != nullptr) {
        node->next->prev = node->prev;
    }
    capacity_--;
    delete node;
}

void ChainedBuffer::set_node_flag_(BufferNode* node, bool HEAD_FLAG, bool TAIL_FLAG, bool MORE_FLAG) {
    node->HEAD_FLAG = HEAD_FLAG;
    node->TAIL_FLAG = TAIL_FLAG;
    node->MORE_FLAG = MORE_FLAG;
}

bool ChainedBuffer::set_node_data_(BufferNode* node, const void* data, int len) {
    if(len <= sizeof(node->data)) {
        memcpy(node->data, data, len);
        node->used_size = len;
        return true;
    }
    return false;
}