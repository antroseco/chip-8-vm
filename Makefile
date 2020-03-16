TARGET_EXEC := chip8_vm
TEST_EXEC := run_tests

CXX := g++
CXXFLAGS := $(INC_FLAGS) -MMD -MP -std=c++17 -Wall -Wextra -pedantic -g
LDFLAGS := -Wall -Wextra -lncurses

BUILD_DIR := build
SRC_DIR := src
TEST_DIR := test

SRCS := $(shell find $(SRC_DIR) -name *.cpp)
TEST_SRCS := $(shell find $(TEST_DIR) -name *.cpp)
OBJS := $(SRCS:%.cpp=$(BUILD_DIR)/%.o)
TEST_OBJS := $(TEST_SRCS:%.cpp=$(BUILD_DIR)/%.o)
TEST_OBJS += $(filter-out $(BUILD_DIR)/$(SRC_DIR)/main.o, $(OBJS))
DEPS := $(OBJS:.o=.d)

INC_DIRS := $(shell find $(SRC_DIR) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

# Target executable
$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

# C++ source files
$(BUILD_DIR)/%.o: %.cpp
	@$(MKDIR_P) $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@


test: $(TEST_OBJS)
	$(CXX) $(TEST_OBJS) -o $(BUILD_DIR)/$(TEST_EXEC) $(LDFLAGS)
	@./$(BUILD_DIR)/$(TEST_EXEC)


.PHONY: clean

clean:
	$(RM) -r $(BUILD_DIR)

-include $(DEPS)

MKDIR_P ?= mkdir -p
