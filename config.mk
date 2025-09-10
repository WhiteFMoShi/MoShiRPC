################### 全局控制 ###################
PROJECT_ROOT = $(abspath $(dir $(lastword $(MAKEFILE_LIST))))

BUILD_PATH = $(PROJECT_ROOT)/build
BIN_PATH := $(BUILD_PATH)/bin
LIBS_PATH := $(BUILD_PATH)/lib
THIRD_PARTY_PATH := $(PROJECT_ROOT)/third_party
INCLUDE := include
SRC := src

INCLUDE_PATH := -I$(SRC) -I$(INCLUDE) -I$(THIRD_PARTY_PATH) \
				-I$(shell find $(PROJECT_ROOT) -name log)/$(INCLUDE) \
				-I$(shell find $(PROJECT_ROOT) -name config_loader)/$(INCLUDE)
OJBS_PATH := $(BUILD_PATH)/obj

CXX := g++
CXXFLAGS := -Wall -g -std=c++17 $(INCLUDE_PATH) -static
