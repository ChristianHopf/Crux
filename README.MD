# Crux
3D game engine written in C, using OpenGL for 3D graphics.

# Installation

## Arch Linux
To build an executable:
1. Clone the repo.
2. Run the below command to install dependencies:
```
sudo pacman -Syu --needed base-devel pkgconf glm freetype2 glfw openal assimp libsndfile
```
3. cd into the project directory and run the ```make``` command.
4. Run the created executable in the ```executables``` directory.

## Ubuntu
To build an executable:
1. Clone the repo.
2. Run the below command to install dependencies:
```
sudo apt-get update && apt-get install -y build-essential pkg-config libcglm-dev libfreetype6-dev libglfw3-dev libopenal-dev libassimp-dev libsndfile1-dev
```
3. cd into the project directory and run the ```make``` command.
4. Run the created executable in the ```executables``` directory.
