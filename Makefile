include config.mk

MOUDLES = cJSON log threadpool

all: build

########### submodules compile ############
compile_SDK:
	cd SDK && . ./construction.sh

compile_log:
	cd log && $(MAKE)

compile_threadpool:
	cd thread_pool && $(MAKE)

########### global construction ############
build: compile_SDK compile_log compile_threadpool
	mkdir -p $(BIN_PATH) $(LIBS_PATH)
	cp $(SDK_PATH)/cJSON/libcjson.a $(LIBS_PATH)/libcjson.a
	cp log/liblog.a $(LIBS_PATH)/liblog.a
	cp thread_pool/libthreadpool.a $(LIBS_PATH)/libthreadpool.a

.PHONY: clean build
clean:
	rm -rf build
	cd log && $(MAKE) clean
	cd thread_pool && $(MAKE) clean
