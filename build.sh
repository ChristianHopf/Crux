#!/bin/bash
gcc -o executables/models1 main.c src/mesh.c src/shader.c src/camera.c src/model.c src/scene.c src/utils.c src/stb_image.c src/glad.c src/cJSON.c -L/usr/lib -Iinclude -lglfw -lGL -lcglm -lm -ldl -lassimp
