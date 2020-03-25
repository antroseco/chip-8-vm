# Target executables
TARGET_EXEC := chip8_vm
TEST_EXEC := run_tests

# Header file directories
INC_DIRS := include lib
INC_FLAGS := $(addprefix -I, $(INC_DIRS))

# Compiler options
CXX := g++
CXXFLAGS := $(INC_FLAGS) -MMD -MP -std=c++17 -Wall -Wextra -pedantic -g -Og -pthread
LDFLAGS := -Wall -Wextra -lncurses -pthread

# Project directories
BUILD_DIR := build
SRC_DIR := src
TEST_DIR := test

# Source files
SRCS := $(shell find $(SRC_DIR) -name *.cpp)
TEST_SRCS := $(shell find $(TEST_DIR) -name *.cpp)

# Object files (TEST_OBJS excludes the normal main function)
OBJS := $(SRCS:%.cpp=$(BUILD_DIR)/%.o)
TEST_OBJS := $(TEST_SRCS:%.cpp=$(BUILD_DIR)/%.o)
TEST_OBJS += $(filter-out $(BUILD_DIR)/$(SRC_DIR)/main.o, $(OBJS))

# Make dependencies
DEPS := $(OBJS:.o=.d) + $(TEST_OBJS:.o=.d)

# Target executable
$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

# C++ source files
$(BUILD_DIR)/%.o: %.cpp
	@$(MKDIR_P) $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@


# Test executable
test: $(TEST_OBJS)
	$(CXX) $(TEST_OBJS) -o $(BUILD_DIR)/$(TEST_EXEC) $(LDFLAGS)
	@./$(BUILD_DIR)/$(TEST_EXEC)


.PHONY: clean

clean:
	@$(RM) -r $(BUILD_DIR)

-include $(DEPS)

MKDIR_P ?= mkdir -p
