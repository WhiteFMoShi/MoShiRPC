include ../../config.mk

SRC_FILES := $(shell find ../src -name '*.cpp')

all: $(SRC_FILES)
	$(CXX) $(CXXFLAGS) -o test.out $^ test.cpp -L$(THIRD_PARTY_PATH)/cJSON -lcjson
	./test.out

.PYONE: clean

clean:
	rm -rf conf Logfile test.out