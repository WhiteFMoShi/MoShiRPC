include ../../config.mk

TARGETNAME := net
STATIC := a

SRC_FILES := $(shell find src -name *.cpp)
OBJ_FILES := $(SRC_FILES:.cpp=.o)

all: static

static: $(TARGETNAME).$(STATIC)

$(TARGETNAME).$(STATIC): $(OBJ_FILES)
	$(AR) rcs $@ $^

.PHONY:	clean static
clean:
	rm -f $(OBJ_FILES) *.a