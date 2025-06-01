#!/bin/bash
gcc -o executables/newmodel1 main.c src/shader.c src/camera.c src/newmodel.c src/scene.c src/utils.c src/stb_image.c src/glad.c -L/usr/lib -Iinclude -lglfw -lGL -lcglm -lm -ldl -lassimp
