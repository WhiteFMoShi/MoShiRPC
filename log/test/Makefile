include ../../config.mk

SRC_FILES := $(shell find ../src -name '*.cpp')

all: $(SRC_FILES)
	$(CXX) $(CXXFLAGS) -o test.out $^ test.cpp
	./test.out

.PYONE: clean

clean:
	rm -rf conf Logfile test.out