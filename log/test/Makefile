include ../../config.mk

SRC_FILES := $(shell find ../src -name '*.cpp')

TIMER_SRC_FLIES := $(shell find ../src -name 'timer.cpp')

all: $(SRC_FILES)
	$(CXX) $(CXXFLAGS) -o test.out $^ test.cpp
	./test.out

test_timer:
	g++ -o test_timer.out test_timer.cpp $(TIMER_SRC_FLIES)
	./test_timer.out

.PYONE: clean

clean:
	rm -rf conf Logfile test.out