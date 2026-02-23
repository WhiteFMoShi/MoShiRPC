#include <algorithm>
#include <cstring>

#include "buffer.hpp"

using moshi::ChainedBuffer;
using moshi::BufferNode;

BufferNode::BufferNode(uint node_size) : next(nullptr), prev(nullptr), 
                            MORE_FLAG(false), HEAD_FLAG(false), TAIL_FLAG(false), 
                            used_size(0), read_offset(0),
                            data(new char[node_size]) {}
BufferNode::~BufferNode() {
    if(data != nullptr) {
        delete[] data;
        data = nullptr;
    }
}

void BufferNode::release_data_memory() {
    if(data != nullptr) {
        delete[] data;
        data = nullptr;
    }
}

void BufferNode::reset_node() {
    used_size = 0;
    read_offset = 0;

    MORE_FLAG = false;
    HEAD_FLAG = false;
    TAIL_FLAG = false;
}

ChainedBuffer::ChainedBuffer(uint node_size, uint max_node_count) : r_point_(nullptr), w_point_(nullptr),
                                head_node_(nullptr), tail_node_(nullptr),
                                size_(0), capacity_(0), 
                                SINGLE_NODE_SIZE_OF_BYTE(node_size), MAX_NODE_COUNT(max_node_count)
{
    head_node_ = create_node_();
    tail_node_ = head_node_;
    set_node_data_(head_node_, nullptr, 0);
    set_node_flag_(head_node_, true, true, false);
    head_node_->SPECIAL_FLAG = true; // 哨兵位

    r_point_ = head_node_;
    w_point_ = head_node_;
}

ChainedBuffer::~ChainedBuffer() {
    clear();
    del_node_(head_node_);
}

int ChainedBuffer::write(const void* data, const int len) {
    int offset = 0;
    BufferNode* temp_node = nullptr;
    BufferNode* mark = w_point_;

    // 还有空白缓冲区
    while(w_point_->next != nullptr) {
        w_point_ = w_point_->next;
        if(w_point_->is_used() && w_point_ != head_node_) {
            w_point_ = mark;
            return -1; // 出错了
        }
        // 单个结点可以放得下
        if(len - offset <= SINGLE_NODE_SIZE_OF_BYTE) {
            set_node_data_(w_point_, static_cast<const char*>(data) + offset, len - offset);
            set_node_flag_(w_point_, offset == 0, true, false);
            offset += len - offset;
            break;
        }
        else { // 单个结点放不下
            set_node_data_(w_point_, static_cast<const char*>(data) + offset, SINGLE_NODE_SIZE_OF_BYTE);
            set_node_flag_(w_point_, offset == 0, false, true);
            offset += SINGLE_NODE_SIZE_OF_BYTE;
        }
    }

    // 空闲缓冲区用完了
    while(offset < len) {
        if((temp_node = create_node_()) == nullptr) {
            while(w_point_ != mark) { // 回退
                w_point_->reset_node();
                w_point_ = w_point_->prev;
            }
            return -1; // 出错了
        }
        set_node_data_(temp_node, static_cast<const char*>(data) + offset, std::min(len - offset, static_cast<int>(SINGLE_NODE_SIZE_OF_BYTE)));
        set_node_flag_(temp_node, offset == 0, offset + SINGLE_NODE_SIZE_OF_BYTE >= len, offset + SINGLE_NODE_SIZE_OF_BYTE < len);
        offset += std::min(len - offset, static_cast<int>(SINGLE_NODE_SIZE_OF_BYTE));

        if(insert_(tail_node_, temp_node) < 0) {
            del_node_(temp_node);
            w_point_ = tail_node_;
            while(w_point_ != mark) { // 回退本次写入的所有结点
                w_point_->reset_node();
                w_point_ = w_point_->prev;
            }
            return -1; // 出错了
        }
        tail_node_ = tail_node_->next;
        w_point_ = tail_node_;
    }
    return offset;
}

int ChainedBuffer::read(void* dest, const uint dest_len) {
    if(dest == nullptr || dest_len == 0) {
        return -1; // 出错了
    }
    if(r_point_ == nullptr) {
        // 如果 r_point_ 为空，尝试从头开始
        r_point_ = head_node_;
    }
    if(r_point_->is_special()) { // 跳过哨兵
        r_point_ = r_point_->next;
        if(r_point_ == nullptr) {
            return 0;
        }
    }

    int offset = 0;

    while(offset < dest_len) {
        if(r_point_ == nullptr) {
            r_point_ = head_node_; // Reset for next time
            break;
        }
        
        // 如果遇到未使用的节点（空闲节点），说明没有数据了
        if (!r_point_->is_used()) {
            break;
        }

        int len = std::min(r_point_->used_size - r_point_->read_offset, dest_len - offset);
        std::memcpy(static_cast<char*>(dest) + offset, r_point_->data + r_point_->read_offset, len);
        offset += len;
        r_point_->read_offset += len; // 本次可能没读完

        // 如果读到末尾的同时,末尾结点也被读完了,则跳出循环
        // 这里需要注意：如果读完了当前节点，需要判断是否是 TAIL
        if(r_point_->read_offset == r_point_->used_size) {
            BufferNode* next_node = r_point_->next;
            bool is_tail = r_point_->TAIL_FLAG;

            // 如果是 tail_node_，需要更新 tail_node_ 指针，防止 recycle 后指针失效或逻辑错误
            if (r_point_ == tail_node_) {
                tail_node_ = r_point_->prev;
            }

            recycle_node_(r_point_);
            r_point_ = next_node;

            if(is_tail)
                break;
        }
    }
    
    return offset;
}

void ChainedBuffer::clear() {
    if(head_node_ == nullptr) {
        return;
    }

    BufferNode* node = head_node_->next;
    while(node != nullptr) {
        if(node->is_special()) {
             node = node->next;
             continue;
        }
        
        BufferNode* temp = node->next;
        del_node_(node);
        node = temp;
    }
    // 重置指针
    r_point_ = head_node_;
    w_point_ = head_node_;
    tail_node_ = head_node_;
    head_node_->next = nullptr;
}

int ChainedBuffer::insert_(BufferNode* ds, BufferNode* rs) {
    // 插入逻辑：将 rs 插入到 ds 之后
    // 需要保留 ds->next (即空闲链表)
    
    rs->next = ds->next;
    if(rs->next != nullptr) {
        rs->next->prev = rs;
    }
    ds->next = rs;
    rs->prev = ds;

    if(rs->is_used()) {
        size_++;
    }

    return 0;
}

int ChainedBuffer::remove_(BufferNode* ds) {
    return del_node_(ds);
}

BufferNode* ChainedBuffer::create_node_() {
    // 允许创建 MAX_NODE_COUNT 个数据节点（不包括哨兵节点）
    if (capacity_ > MAX_NODE_COUNT) {
        return nullptr;
    }
    BufferNode* node = new BufferNode(SINGLE_NODE_SIZE_OF_BYTE);
    capacity_++;
    return node;
}

int ChainedBuffer::del_node_(BufferNode* node) {
    if(node == nullptr) {
        return -1;
    }

    if(node->is_used()) {
        size_--;
    }
    if(node->prev != nullptr) {
        node->prev->next = node->next;
    }
    if(node->next != nullptr) {
        node->next->prev = node->prev;
    }
    capacity_--;
    delete node;

    return 0;
}

void ChainedBuffer::set_node_flag_(BufferNode* node, bool HEAD_FLAG, bool TAIL_FLAG, bool MORE_FLAG) {
    node->HEAD_FLAG = HEAD_FLAG;
    node->TAIL_FLAG = TAIL_FLAG;
    node->MORE_FLAG = MORE_FLAG;
}

bool ChainedBuffer::set_node_data_(BufferNode* node, const void* data, int len) {
    if(data == nullptr) {
        return true;
    }

    if(len <= SINGLE_NODE_SIZE_OF_BYTE) {
        std::memcpy(node->data, data, len);
        node->used_size = len;
        return true;
    }
    return false;
}

bool ChainedBuffer::recycle_node_(BufferNode* node) {
    if(node == nullptr) {
        return false;
    }

    node->reset_node();
    if(node->prev != nullptr) {
        node->prev->next = node->next;
    }
    if(node->next != nullptr) {
        node->next->prev = node->prev;
    }

    return insert_(tail_node_, node);
}