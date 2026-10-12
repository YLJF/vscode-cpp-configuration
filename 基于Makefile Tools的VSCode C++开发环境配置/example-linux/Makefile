# Each configuration has its own outputs and dependency files.
CONFIG ?= gcc-debug
VALID_CONFIGS := gcc-debug gcc-release clang-debug clang-release
ifneq ($(words $(CONFIG)),1)
$(error CONFIG must be a single configuration name)
endif
ifeq ($(filter $(CONFIG),$(VALID_CONFIGS)),)
$(error Unsupported CONFIG "$(CONFIG)". Choose one of: $(VALID_CONFIGS))
endif

TOOLCHAIN := $(word 1,$(subst -, ,$(CONFIG)))
MODE := $(word 2,$(subst -, ,$(CONFIG)))
ifeq ($(TOOLCHAIN),clang)
CXX_NAME := clang++
else
CXX_NAME := g++
endif

override BUILD_DIR := build/$(CONFIG)
OBJ_DIR := $(BUILD_DIR)/obj
BIN_DIR := $(BUILD_DIR)/bin

ifeq ($(OS),Windows_NT)
SHELL := cmd.exe
.SHELLFLAGS := /D /C
EXE_EXT := .exe
CXX := $(subst \,/,$(shell powershell.exe -NoProfile -Command "(Get-Command $(CXX_NAME) -CommandType Application | Select-Object -First 1).Source"))
else
SHELL := /bin/sh
EXE_EXT :=
CXX := $(shell command -v $(CXX_NAME))
endif

ifeq ($(strip $(CXX)),)
$(error Compiler $(CXX_NAME) was not found on PATH)
endif

CPPFLAGS += -Iinclude
CXXFLAGS += -std=c++20 -Wall -Wextra -Wpedantic
ifeq ($(MODE),debug)
CPPFLAGS += -DDEMO_BUILD_DEBUG=1
CXXFLAGS += -O0 -g
else
CPPFLAGS += -DNDEBUG -DDEMO_BUILD_DEBUG=0
CXXFLAGS += -O2
endif

APP := $(BIN_DIR)/example_app$(EXE_EXT)
CALC := $(BIN_DIR)/demo_calc$(EXE_EXT)
OBJECTS := $(OBJ_DIR)/main.o $(OBJ_DIR)/calc.o $(OBJ_DIR)/math.o
DEPS := $(OBJECTS:.o=.d)

.DEFAULT_GOAL := all
.PHONY: all example_app demo_calc clean
all: example_app demo_calc
example_app: $(APP)
demo_calc: $(CALC)

$(APP): $(OBJ_DIR)/main.o $(OBJ_DIR)/math.o Makefile | $(BIN_DIR)
	"$(CXX)" $(LDFLAGS) $(filter %.o,$^) $(LDLIBS) -o "$@"

$(CALC): $(OBJ_DIR)/calc.o $(OBJ_DIR)/math.o Makefile | $(BIN_DIR)
	"$(CXX)" $(LDFLAGS) $(filter %.o,$^) $(LDLIBS) -o "$@"

$(OBJ_DIR)/%.o: src/%.cpp Makefile | $(OBJ_DIR)
	"$(CXX)" $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c "$<" -o "$@"

$(OBJ_DIR) $(BIN_DIR):
ifeq ($(OS),Windows_NT)
	if not exist "$(subst /,\,$@)" mkdir "$(subst /,\,$@)"
else
	mkdir -p "$@"
endif

# Clean only the active, validated configuration.
clean:
ifeq ($(OS),Windows_NT)
	if exist "$(subst /,\,$(BUILD_DIR))" rmdir /S /Q "$(subst /,\,$(BUILD_DIR))"
else
	rm -rf "$(BUILD_DIR)"
endif

-include $(DEPS)
