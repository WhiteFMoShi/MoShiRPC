include config.mk

construct:
	cd SDK && . ./construction.sh

build: construct
	-mkdir -p $(BIN_PATH) $(LIBS_PATH)
	cp SDK/cJSON/libcjson.a lib/libcjson.a # 库拷贝

.PHONY: clean
clean:
	@cd SDK && $(MAKE) clean