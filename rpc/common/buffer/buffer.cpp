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
}

int ChainedBuffer::write(const void* data, const int len) {
    int offset = 0;
    BufferNode* temp_node = nullptr;
    BufferNode* mark = w_point_;

    // 还有空白缓冲区
    while(w_point_->next != nullptr) {
        w_point_ = w_point_->next;
        if(w_point_->is_used()) {
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
    if(r_point_->is_special()) { // 跳过哨兵
        r_point_ = r_point_->next;
    }

    int offset = 0;
    BufferNode* mark = r_point_;
    while(offset < dest_len) {
        if(r_point_->is_used()) {
            int len = std::min(r_point_->used_size - r_point_->read_offset, dest_len - offset);
            std::memcpy(static_cast<char*>(dest) + offset, r_point_->data + r_point_->read_offset, len);
            offset += len;
            r_point_->read_offset += len; // 本次可能没读完
        }

        if(r_point_->TAIL_FLAG == true && r_point_->read_offset == r_point_->used_size)
            break;
    }

    // 这个节点被读完了
    if(r_point_->TAIL_FLAG == true && r_point_->read_offset == r_point_->used_size) {
        while(mark != r_point_) {
            mark = mark->next;
            mark->reset_node();
            recycle_node_(mark);
        }
    }
    
    return offset;
}

void ChainedBuffer::clear() {
    while(head_node_ != nullptr) {
        del_node_(head_node_);
        head_node_ = head_node_->next;
    }
}

int ChainedBuffer::insert_(BufferNode* ds, BufferNode* rs) {
    if(size_ >= MAX_NODE_COUNT) {
        return -1; // 出错了
    }

    ds->next = rs;
    rs->prev = ds;
    if(rs->is_used()) {
        size_++;
    }
    capacity_++;

    return false;
}

int ChainedBuffer::remove_(BufferNode* ds) {
    return del_node_(ds);
}

BufferNode* ChainedBuffer::create_node_() {
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