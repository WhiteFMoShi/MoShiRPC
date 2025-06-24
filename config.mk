################### 全局控制 ###################
PROJECT_ROOT = $(abspath $(dir $(lastword $(MAKEFILE_LIST))))
BUILD_PATH = $(PROJECT_ROOT)/build
BIN_PATH := $(BUILD_PATH)/bin
LIBS_PATH := $(BUILD_PATH)/lib
SDK_PATH := $(PROJECT_ROOT)/SDK
INCLUDE_PATH := -Isrc -Iinclude -I$(SDK_PATH)

CXX := g++
CXXFLAGS := -Wall -O3 $(INCLUDE_PATH) -g

export CXX CXXFLAGS BIN_PATH LIBS_PATH