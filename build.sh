#!/bin/bash

MAIN_OUT=executables/collision2
SRC_DIR=src
SRC_FILES="main.c $(find $SRC_DIR -type f -name "*.c")"

TEST_OUT=executables/testing
TEST_DIR=tests
TEST_SRC_FILES="$(find $TESTS_DIR -type f -name "test_*.c")"

CC=gcc
CFLAGS="-Iinclude $(pkg-config --cflags freetype2) -Ithird_party/unity"
LDFLAGS="-L/usr/lib $(pkg-config --libs freetype2) -lglfw -lGL -lcglm -lm -ldl -lassimp"

# Compile main binary
$CC -o $OUT $SRC_FILES $CFLAGS $LDFLAGS

# Compile test binary
$CC -o $TEST_OUT $TEST_SRC_FILES $CFLAGS $LDFLAGS
