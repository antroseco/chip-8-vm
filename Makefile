# Target executables
DEBUG_EXEC := chip8_vm
TEST_EXEC := run_tests
FUZZ_EXEC := fuzz

# Header file directories
INC_DIRS := include lib
INC_FLAGS := $(addprefix -I, $(INC_DIRS))

# Compiler options
CXX := clang++-10
COMMONFLAGS := -flto -g -O1
CXXFLAGS := $(INC_FLAGS) $(COMMONFLAGS) -MMD -MP -std=c++17 -Wall -Wextra -pedantic
LDFLAGS := $(COMMONFLAGS) -lncurses
FUZZFLAGS := -fsanitize=fuzzer,address,undefined -DFUZZING

# Project directories
BUILD_ROOT := build
BUILD_DIR := build/debug
FUZZ_DIR := build/fuzzing
SRC_DIR := src
TEST_DIR := test

# Source files
SRCS := $(shell find $(SRC_DIR) -name *.cpp)
TEST_SRCS := $(shell find $(TEST_DIR) -name *.cpp)

# Object files (TEST_OBJS excludes the normal main function)
OBJS := $(SRCS:%.cpp=$(BUILD_DIR)/%.o)
TEST_OBJS := $(TEST_SRCS:%.cpp=$(BUILD_DIR)/%.o)
TEST_OBJS += $(filter-out $(BUILD_DIR)/$(SRC_DIR)/main.o, $(OBJS))
FUZZ_OBJS := $(SRCS:%.cpp=$(FUZZ_DIR)/%.o)

# Make dependencies
DEPS := $(OBJS:.o=.d) + $(TEST_OBJS:.o=.d)

# Target executable
$(BUILD_ROOT)/$(DEBUG_EXEC): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

# C++ source files (normal)
$(BUILD_DIR)/%.o: %.cpp
	@$(MKDIR_P) $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# C++ source files (for fuzzing)
$(FUZZ_DIR)/%.o: %.cpp
	@$(MKDIR_P) $(dir $@)
	$(CXX) $(CXXFLAGS) $(FUZZFLAGS) -c $< -o $@


# Catch2 executable
test: $(TEST_OBJS)
	$(CXX) $(TEST_OBJS) -o $(BUILD_ROOT)/$(TEST_EXEC) $(LDFLAGS)
	@./$(BUILD_ROOT)/$(TEST_EXEC)


# libFuzzer executable
fuzz: $(FUZZ_OBJS)
	$(CXX) $(FUZZ_OBJS) -o $(BUILD_ROOT)/$(FUZZ_EXEC) $(FUZZFLAGS) $(LDFLAGS)

.PHONY: clean

clean:
	@$(RM) -r $(BUILD_ROOT)

-include $(DEPS)

MKDIR_P ?= mkdir -p
