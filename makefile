# Compiler and flags
CC = gcc
CFLAGS = -Iinclude -Iinclude/physics $(shell pkg-config --cflags freetype2) -Ithird_party/unity -Wall -Wextra -g
LDFLAGS = -L/usr/lib $(shell pkg-config --libs freetype2) -lglfw -lGL -lcglm -lm -ldl -lassimp

# Directories
SRC_DIR = src
TEST_DIR = test
OUT_DIR = executables
UNITY_DIR = third_party/unity
OBJ_DIR = build

# Source files
SRC_FILES = $(shell find $(SRC_DIR) -type f -name "*.c")
TEST_FILES = $(wildcard $(TEST_DIR)/**/*.c)
UNITY_SRC = $(UNITY_DIR)/unity.c

# Object files
SRC_OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_FILES))
TEST_OBJS = $(patsubst $(TEST_DIR)/%.c,$(OBJ_DIR)/test/%.o,$(TEST_FILES))
UNITY_OBJ = $(OBJ_DIR)/unity.o

# Output binaries
MAIN_OUT = $(OUT_DIR)/resolution1
TEST_OUT = $(OUT_DIR)/test_runner

# Default target
all: $(MAIN_OUT)

# Main engine build
$(MAIN_OUT): $(SRC_OBJS)
	@mkdir -p $(OUT_DIR)
	@echo "Linking main binary: $@"
	$(CC) -o $@ $^ $(LDFLAGS)

# Object files for source
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	@echo "Compiling $<"
	$(CC) $(CFLAGS) -c $< -o $@

# Test build
test: $(TEST_OUT)
	@echo "Running tests: ./$(TEST_OUT)"
	./$(TEST_OUT)

# Test runner build
$(TEST_OUT): $(TEST_OBJS) $(UNITY_OBJ) $(filter-out $(OBJ_DIR)/main.o,$(SRC_OBJS))
	@mkdir -p $(OUT_DIR)
	@echo "Linking test binary: $@"
	$(CC) -o $@ $^ $(LDFLAGS)

# Object files for tests
$(OBJ_DIR)/test/%.o: $(TEST_DIR)/%.c
	@mkdir -p $(dir $@)
	@echo "Compiling test $<"
	$(CC) $(CFLAGS) -c $< -o $@

# Unity object file
$(OBJ_DIR)/unity.o: $(UNITY_SRC)
	@mkdir -p $(OBJ_DIR)
	@echo "Compiling Unity $<"
	$(CC) $(CFLAGS) -c $< -o $@

# Clean
clean:
	rm -rf $(OBJ_DIR) $(OUT_DIR)

# Debug target to print variables
debug:
	@echo "SRC_FILES: $(SRC_FILES)"
	@echo "SRC_OBJS: $(SRC_OBJS)"
	@echo "TEST_FILES: $(TEST_FILES)"
	@echo "TEST_OBJS: $(TEST_OBJS)"

.PHONY: all test clean debug
