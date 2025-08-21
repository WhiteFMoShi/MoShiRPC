include ../../config.mk

PROTO_DIR := proto
PROTO_SRC := $(shell find $(PROTO_DIR) -name "*.proto")
GENERATE_DIR := generated

SRC_FILES := $(shell find src -name *.cpp)

OBJ_FILES := $(SRC_FILES:.cpp=.o)

default: generate_code

generate_code:
	mkdir -p $(GENERATE_DIR) && \
		protoc --cpp_out=$(GENERATE_DIR) --proto_path=$(PROTO_DIR) $(PROTO_SRC)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ $(LDFLAGS)

clean:
	rm -rf $(GENERATE_DIR)