#!/bin/bash
gcc -o executables/embtex2 main.c src/shader.c src/camera.c src/model.c src/scene.c src/utils.c src/stb_image.c src/glad.c -L/usr/lib -Iinclude -lglfw -lGL -lcglm -lm -ldl -lassimp
