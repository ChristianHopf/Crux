#!/bin/bash
gcc -o executables/engine3 main.c src/mesh.c src/shader.c src/camera.c src/model.c src/stb_image.c src/glad.c -L/usr/lib -Iinclude -lglfw -lGL -lcglm -lm -ldl -lassimp
