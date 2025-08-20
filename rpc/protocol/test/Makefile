include ../../../config.mk
INCLUDE_PATH_P := -I../include -I$(THIRD_PARTY_PATH) -I$(shell find $(PROJECT_ROOT) -name log)/$(INCLUDE)

SRC_FILES := $(shell find .. -name '*.cpp')

CXX := g++
CXXFLAGS_P := -std=c++17 -g $(INCLUDE_PATH_P)

TARGET_LIST := test_protocol
TARGET := $(addsuffix .out, $(TARGET_LIST))

all: $(TARGET)
	./$(TARGET)

$(TARGET): $(SRC_FILES)
	$(CXX) $(CXXFLAGS_P) -o $@ $^ $(THIRD_PARTY_PATH)/cJSON/libcjson.a

clean:
	rm -f $(TARGET)