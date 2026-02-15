#include "buffer.hpp"
#include <cstring>

ChainedBuffer::ChainedBuffer(uint node_size) : r_point_(nullptr), w_point_(nullptr),
                                head_node_(nullptr), tail_node_(nullptr),
                                node_count_(0), SINGLE_NODE_SIZE_OF_KB(node_size)
{
}

ChainedBuffer::~ChainedBuffer() {
    clear();
}

int ChainedBuffer::write(const void* data, int len) {
    // int offset = 0;
    // while(BufferNode* temp_node = allocate_node_()) {
    //     if(len > temp_node->NODE_SIZE) { // 一个结点放不下
    //         set_node_data_(temp_node, static_cast<const char*>(data) + offset, temp_node->NODE_SIZE);
    //         set_node_flag_(temp_node, offset == 0, false, true);
    //         offset += temp_node->NODE_SIZE;
    //     }
    //     else { // 能放下
    //         set_node_data_(temp_node, static_cast<const char*>(data) + offset, len - offset);
    //         offset += len - offset;
    //         set_node_flag_(temp_node, offset == 0, true, false);
    //     }
    // }
    // return offset;
}

void ChainedBuffer::set_node_data_(BufferNode* node, const void* data, int len) {
    memcpy(node->data, data, len);
    node->used_size = len;
}

void ChainedBuffer::set_node_flag_(BufferNode* node, bool HEAD_FLAG, bool TAIL_FLAG, bool MORE_FLAG) {
    node->HEAD_FLAG = HEAD_FLAG;
    node->TAIL_FLAG = TAIL_FLAG;
    node->MORE_FLAG = MORE_FLAG;
}

