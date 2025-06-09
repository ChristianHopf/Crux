#!/bin/bash
gcc -o executables/aabb1 main.c src/shader.c src/camera.c src/model.c src/material.c src/scene.c src/player.c src/skybox.c src/text.c src/utils.c src/stb_image.c src/glad.c -L/usr/lib -Iinclude $(pkg-config --cflags freetype2) $(pkg-config --libs freetype2) -lglfw -lGL -lcglm -lm -ldl -lassimp
