DIR := /home/xfy/module/log
SRC := bench.cpp \
	$(DIR)/logConfig/logConfig.cpp \
	$(DIR)/logEntry/logFormat/logFormat.cpp \
	$(DIR)/logFileManager/logFileManager.cpp \
	$(DIR)/utils/timeStamp.cpp \
	$(DIR)/logEntry/logEntry.cpp \
	$(DIR)/log.cpp \
	$(DIR)/utils/logQueue.cpp

FLAG := -std=c++17 -Wall -g
TARGET := bench

bench:
	g++ -o $(TARGET) $(FLAG) $(SRC)
	./$(TARGET)

clean:
	rm -rf Log conf