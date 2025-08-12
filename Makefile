include config.mk

MOUDLES = cJSON log threadpool

all: build

########### submodules compile ############
third_party:
	cd third_party && . ./construction.sh

compile_log:
	cd log && $(MAKE)

compile_threadpool:
	cd thread_pool && $(MAKE)

########### global construction ############
build: third_party compile_log compile_threadpool
	mkdir -p $(BIN_PATH) $(LIBS_PATH) $(OJBS_PATH)
	cp $(THIRD_PARTY_PATH)/cJSON/libcjson.a $(LIBS_PATH)/libcjson.a
	cp log/liblog.a $(LIBS_PATH)/liblog.a
	cp thread_pool/libthreadpool.a $(LIBS_PATH)/libthreadpool.a

.PHONY: clean build
clean:
	rm -rf build
	cd log && $(MAKE) clean
	cd thread_pool && $(MAKE) clean
