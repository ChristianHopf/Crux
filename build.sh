#!/bin/bash
gcc -o executables/matloadtex1 main.c src/shader.c src/camera.c src/model.c src/material.c src/scene.c src/player.c src/utils.c src/stb_image.c src/glad.c -L/usr/lib -Iinclude -lglfw -lGL -lcglm -lm -ldl -lassimp
