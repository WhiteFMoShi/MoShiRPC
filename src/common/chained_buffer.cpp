#include <algorithm>
#include <cstring>

#include "common/chained_buffer.hpp"

using moshi::ChainedBuffer;
using moshi::BufferNode;

BufferNode::BufferNode(std::size_t node_size)
    : next(nullptr),
      prev(nullptr),
      capacity(node_size),
      used_size(0),
      read_offset(0),
      data(new char[node_size]) {}
BufferNode::~BufferNode() {
    if(data != nullptr) {
        delete[] data;
        data = nullptr;
    }
}

void BufferNode::ReleaseNode() {
    if(data != nullptr) {
        delete[] data;
        data = nullptr;
    }
}

void BufferNode::ResetNode() {
    used_size = 0;
    read_offset = 0;
}

ssize_t BufferNode::WriteableSize() const {
    if (capacity < used_size) {
        return 0;
    }
    return static_cast<ssize_t>(capacity - used_size);
}

ssize_t BufferNode::ReadableSize() const {
    if (used_size < read_offset) {
        return 0;
    }
    return static_cast<ssize_t>(used_size - read_offset);
}

ChainedBuffer::ChainedBuffer(std::size_t node_size, std::size_t max_node_count)
    : r_point_(nullptr),
      w_point_(nullptr),
      head_node_(nullptr),
      tail_node_(nullptr),
      size_(0),
      capacity_(0),
      SINGLE_NODE_SIZE_OF_BYTE(node_size),
      MAX_NODE_COUNT(max_node_count) {
    // 哨兵节点不计入 capacity_ / size_。
    head_node_ = new BufferNode(SINGLE_NODE_SIZE_OF_BYTE);
    head_node_->ResetNode();
    head_node_->next = nullptr;
    head_node_->prev = nullptr;

    tail_node_ = head_node_;
    r_point_ = head_node_;
    w_point_ = head_node_;
}

ChainedBuffer::~ChainedBuffer() {
    Clear();
    del_node_(head_node_);
}

int ChainedBuffer::Write(const void* data, const int len) {
    if (len < 0) {
        return -1;
    }
    if (len == 0) {
        return 0;
    }
    if (data == nullptr) {
        return -1;
    }

    const char* p = static_cast<const char*>(data);
    std::size_t left = static_cast<std::size_t>(len);
    std::size_t written = 0;

    while (left > 0) {
        // Ensure we have a writable node.
        BufferNode* node = w_point_;
        if (node == head_node_ || node == nullptr || node->used_size >= SINGLE_NODE_SIZE_OF_BYTE) {
            if (capacity_ >= MAX_NODE_COUNT) {
                return -1;
            }
            BufferNode* new_node = create_node_();
            if (new_node == nullptr) {
                return -1;
            }
            new_node->ResetNode();
            insert_(tail_node_, new_node);
            tail_node_ = new_node;
            w_point_ = new_node;
            if (r_point_ == head_node_) {
                r_point_ = new_node;
            }
            node = new_node;
        }

        const std::size_t writeable = SINGLE_NODE_SIZE_OF_BYTE - node->used_size;
        const std::size_t n = std::min(writeable, left);
        std::memcpy(node->data + node->used_size, p + written, n);
        node->used_size += n;

        written += n;
        left -= n;
    }

    return static_cast<int>(written);
}

int ChainedBuffer::Read(void* dest, const std::size_t dest_len) {
    if (dest == nullptr || dest_len == 0) {
        return -1;
    }
    if (Empty()) {
        return 0;
    }

    char* out = static_cast<char*>(dest);
    std::size_t need = dest_len;
    std::size_t read = 0;

    BufferNode* node = r_point_;
    while (need > 0 && node != nullptr && node != head_node_) {
        if (node->used_size < node->read_offset) {
            // Corrupted state; fail-fast.
            return -1;
        }
        const std::size_t readable = node->used_size - node->read_offset;
        if (readable == 0) {
            BufferNode* next = node->next;
            if (node == w_point_) {
                w_point_ = head_node_;
            }
            del_node_(node);
            node = next;
            r_point_ = node ? node : head_node_;
            if (node == nullptr) {
                tail_node_ = head_node_;
                r_point_ = head_node_;
                w_point_ = head_node_;
            }
            continue;
        }

        const std::size_t n = std::min(readable, need);
        std::memcpy(out + read, node->data + node->read_offset, n);
        node->read_offset += n;
        read += n;
        need -= n;

        if (node->read_offset == node->used_size) {
            BufferNode* next = node->next;
            if (node == w_point_) {
                w_point_ = head_node_;
            }
            del_node_(node);
            node = next;
            r_point_ = node ? node : head_node_;
            if (node == nullptr) {
                tail_node_ = head_node_;
                r_point_ = head_node_;
                w_point_ = head_node_;
            }
        }
    }

    return static_cast<int>(read);
}

bool ChainedBuffer::Empty() const {
    return size_ == 0;
}

void ChainedBuffer::Clear() {
    BufferNode* node = head_node_->next;
    while (node != nullptr) {
        BufferNode* next = node->next;
        del_node_(node);
        node = next;
    }

    head_node_->next = nullptr;
    tail_node_ = head_node_;
    r_point_ = head_node_;
    w_point_ = head_node_;
    size_ = 0;
    capacity_ = 0;
}

int ChainedBuffer::Size() const {
    return static_cast<int>(size_);
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

    // 这里的 size_ 表达“链表中数据节点个数”（不含哨兵）。
    size_++;

    return 0;
}

int ChainedBuffer::remove_(BufferNode* ds) {
    return del_node_(ds);
}

BufferNode* ChainedBuffer::create_node_() {
    // 允许创建 MAX_NODE_COUNT 个数据节点（不包括哨兵节点）
    if (capacity_ >= MAX_NODE_COUNT) {
        return nullptr;
    }
    auto* node = new BufferNode(SINGLE_NODE_SIZE_OF_BYTE);
    capacity_++;
    return node;
}

int ChainedBuffer::del_node_(BufferNode* node) {
    if(node == nullptr) {
        return -1;
    }

    if(node == head_node_) {
        // head node is handled by caller
        delete node;
        return 0;
    }

    if (size_ > 0) {
        size_--;
    }
    if(node->prev != nullptr) {
        node->prev->next = node->next;
    }
    if(node->next != nullptr) {
        node->next->prev = node->prev;
    }
    if (capacity_ > 0) {
        capacity_--;
    }
    delete node;

    return 0;
}

bool ChainedBuffer::recycle_node_(BufferNode* node) {
    if(node == nullptr) {
        return false;
    }

    node->ResetNode();
    if(node->prev != nullptr) {
        node->prev->next = node->next;
    }
    if(node->next != nullptr) {
        node->next->prev = node->prev;
    }

    return insert_(tail_node_, node);
}
