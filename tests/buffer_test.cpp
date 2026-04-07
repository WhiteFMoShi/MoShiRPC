#include <gtest/gtest.h>
#include <string>
#include <cstring>
#include <vector>

#include "buffer.hpp"

using namespace moshi;

class ChainedBufferTest : public ::testing::Test {
protected:
    void SetUp() override {
        buffer_ = new ChainedBuffer(1024, 10);
    }
    
    void TearDown() override {
        delete buffer_;
    }
    
    ChainedBuffer* buffer_;
};

// 测试基本写入和读取
TEST_F(ChainedBufferTest, BasicWriteAndRead) {
    const char* test_data = "Hello, World!";
    const int data_len = strlen(test_data);
    
    // 写入数据
    int write_result = buffer_->write(test_data, data_len);
    EXPECT_EQ(write_result, data_len);
    EXPECT_FALSE(buffer_->empty());
    
    // 读取数据
    char read_data[64] = {0};
    int read_result = buffer_->read(read_data, data_len);
    EXPECT_EQ(read_result, data_len);
    EXPECT_STREQ(test_data, read_data);
}

// 测试空缓冲区
TEST_F(ChainedBufferTest, EmptyBuffer) {
    EXPECT_TRUE(buffer_->empty());
    EXPECT_EQ(buffer_->get_node_count(), 0);
    
    char read_data[64] = {0};
    int read_result = buffer_->read(read_data, 64);
    EXPECT_EQ(read_result, 0);
}

// 测试大块数据写入和读取（跨越多个节点）
TEST_F(ChainedBufferTest, LargeDataWriteAndRead) {
    // 创建大于单个节点大小的数据
    std::string large_data(2048, 'A');  // 2KB数据，跨越2个节点
    
    // 写入数据
    int write_result = buffer_->write(large_data.c_str(), large_data.size());
    EXPECT_EQ(write_result, large_data.size());
    
    // 读取数据
    std::vector<char> read_data(large_data.size() + 1, 0);
    int read_result = buffer_->read(read_data.data(), large_data.size());
    EXPECT_EQ(read_result, large_data.size());
    EXPECT_EQ(large_data, std::string(read_data.data()));
}

// 测试多次写入和多次读取
TEST_F(ChainedBufferTest, MultipleWriteAndRead) {
    // 第一次写入
    const char* data1 = "First message";
    int len1 = strlen(data1);
    int write1 = buffer_->write(data1, len1);
    EXPECT_EQ(write1, len1);
    
    // 第二次写入
    const char* data2 = "Second message";
    int len2 = strlen(data2);
    int write2 = buffer_->write(data2, len2);
    EXPECT_EQ(write2, len2);
    
    // 读取第一次写入的数据
    char read1[64] = {0};
    int read_result1 = buffer_->read(read1, len1);
    EXPECT_EQ(read_result1, len1);
    EXPECT_STREQ(data1, read1);
    
    // 读取第二次写入的数据
    char read2[64] = {0};
    int read_result2 = buffer_->read(read2, len2);
    EXPECT_EQ(read_result2, len2);
    EXPECT_STREQ(data2, read2);
}

// 测试部分读取
TEST_F(ChainedBufferTest, PartialRead) {
    const char* test_data = "This is a test message for partial reading";
    const int data_len = strlen(test_data);
    
    // 写入数据
    int write_result = buffer_->write(test_data, data_len);
    EXPECT_EQ(write_result, data_len);
    
    // 部分读取
    const int partial_len = 10;
    char read_data[64] = {0};
    int read_result = buffer_->read(read_data, partial_len);
    EXPECT_EQ(read_result, partial_len);
    EXPECT_EQ(strncmp(test_data, read_data, partial_len), 0);
    
    // 读取剩余部分
    char remaining_data[64] = {0};
    int remaining_result = buffer_->read(remaining_data, data_len - partial_len);
    EXPECT_EQ(remaining_result, data_len - partial_len);
    EXPECT_EQ(strncmp(test_data + partial_len, remaining_data, data_len - partial_len), 0);
}

// 测试清空缓冲区
TEST_F(ChainedBufferTest, ClearBuffer) {
    const char* test_data = "Data to be cleared";
    const int data_len = strlen(test_data);
    
    // 写入数据
    int write_result = buffer_->write(test_data, data_len);
    EXPECT_EQ(write_result, data_len);
    EXPECT_FALSE(buffer_->empty());
    
    // 清空缓冲区
    buffer_->clear();
    EXPECT_TRUE(buffer_->empty());
    EXPECT_EQ(buffer_->get_node_count(), 0);
    
    // 尝试读取应该返回0
    char read_data[64] = {0};
    int read_result = buffer_->read(read_data, 64);
    EXPECT_EQ(read_result, 0);
}

// 测试边界条件：写入空数据
TEST_F(ChainedBufferTest, WriteEmptyData) {
    int write_result = buffer_->write(nullptr, 0);
    EXPECT_EQ(write_result, 0);
    EXPECT_TRUE(buffer_->empty());
}

// 测试边界条件：读取空数据
TEST_F(ChainedBufferTest, ReadEmptyData) {
    char read_data[64] = {0};
    int read_result = buffer_->read(read_data, 0);
    EXPECT_EQ(read_result, -1);  // 根据实现，应该返回-1表示错误
    
    read_result = buffer_->read(nullptr, 64);
    EXPECT_EQ(read_result, -1);  // 根据实现，应该返回-1表示错误
}

// 测试节点数量限制
TEST_F(ChainedBufferTest, NodeCountLimit) {
    // 创建一个缓冲区，限制最多2个节点
    ChainedBuffer small_buffer(512, 2);
    
    // 写入数据，应该成功
    const char* data1 = "First data block";
    int len1 = strlen(data1);
    int write1 = small_buffer.write(data1, len1);
    EXPECT_EQ(write1, len1);
    
    // 写入更多数据，但不超过第二个节点
    const char* data2 = "Second data block";
    int len2 = strlen(data2);
    int write2 = small_buffer.write(data2, len2);
    EXPECT_EQ(write2, len2);
    
    // 尝试写入超过节点限制的数据
    std::string large_data(1024, 'X');  // 需要第三个节点
    int write3 = small_buffer.write(large_data.c_str(), large_data.size());
    EXPECT_EQ(write3, -1);  // 应该失败
}

// 测试二进制数据
TEST_F(ChainedBufferTest, BinaryData) {
    // 创建包含0字节和其他二进制值的数据
    std::vector<unsigned char> binary_data;
    for (int i = 0; i < 256; ++i) {
        binary_data.push_back(static_cast<unsigned char>(i));
    }
    
    // 写入二进制数据
    int write_result = buffer_->write(binary_data.data(), binary_data.size());
    EXPECT_EQ(write_result, binary_data.size());
    
    // 读取二进制数据
    std::vector<unsigned char> read_data(binary_data.size(), 0);
    int read_result = buffer_->read(read_data.data(), binary_data.size());
    EXPECT_EQ(read_result, binary_data.size());
    
    // 验证数据
    EXPECT_EQ(binary_data, read_data);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}