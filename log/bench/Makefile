include ../../config.mk

EXECUTABLE = bench

SRC_FILES := bench.cpp

OBJ_FILES = $(SRC_FILES:.cpp=.o)

all: $(EXECUTABLE)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(EXECUTABLE): $(OBJ_FILES)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^ -L.. -llog

run: $(EXECUTABLE)
	./$(EXECUTABLE)

clean:
	rm -rf $(OBJ_FILES) $(EXECUTABLE) Logfile conf
