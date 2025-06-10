#!/bin/bash

OUT=executables/collision2
SRC_DIR=src
PHYSICS_DIR=$SRC_DIR/physics

SRC_FILES="main.c $SRC_DIR/*.c $PHYSICS_DIR/*.c"

CC=gcc
CFLAGS="-Iinclude $(pkg-config --cflags freetype2)"
LDFLAGS="-L/usr/lib $(pkg-config --libs freetype2) -lglfw -lGL -lcglm -lm -ldl -lassimp"

$CC -o $OUT $SRC_FILES $CFLAGS $LDFLAGS

# gcc -o executables/aabb1 main.c src/shader.c src/camera.c src/model.c src/material.c src/scene.c src/player.c src/skybox.c src/text.c src/physics/aabb.c src/utils.c src/stb_image.c src/glad.c -L/usr/lib -Iinclude $(pkg-config --cflags freetype2) $(pkg-config --libs freetype2) -lglfw -lGL -lcglm -lm -ldl -lassimp
