#include <cglm/cam.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <stdio.h>
#include <stdbool.h>
#include "stb_image/stb_image.h"
#include <cglm/cglm.h>
#include "camera.h"
#include "shader.h"
#include "model.h"
#include "scene.h"
#include "player.h"
#include "text.h"

typedef struct {
  GLFWwindow *window;
  Scene *active_scene;
  // Timing
  float deltaTime;
  float lastFrame;
} Engine;

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

// Screen settings
const unsigned int SCREEN_WIDTH = 1024;
const unsigned int SCREEN_HEIGHT = 768;

// Mouse
bool firstMouse = true;
float lastX = 400.0f;
float lastY = 300.0f;

// Lighting
vec3 lightPos = {1.2f, 0.5f, 2.0f};

static int last_space_state = GLFW_RELEASE;
void processInput(GLFWwindow *window){
	if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
		glfwSetWindowShouldClose(window, 1);
	}
  Engine *engine = (Engine *)glfwGetWindowUserPointer(window);
  struct Camera *camera = engine->active_scene->player.camera;

  // Don't process input (other than the Escape key) if the game is paused
  if (engine->active_scene->paused){
    return;
  }

  // Camera movement
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
    camera_process_keyboard_input(camera, CAMERA_FORWARD, engine->deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
    camera_process_keyboard_input(camera, CAMERA_BACKWARD, engine->deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
    camera_process_keyboard_input(camera, CAMERA_LEFT, engine->deltaTime);
  }
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
    camera_process_keyboard_input(camera, CAMERA_RIGHT, engine->deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS){
    camera_process_keyboard_input(camera, CAMERA_DOWN, engine->deltaTime);
  }

  // Only process these inputs a single time per press
  int space_state = glfwGetKey(window, GLFW_KEY_SPACE);
	if (space_state == GLFW_PRESS && last_space_state == GLFW_RELEASE){
    player_jump(&engine->active_scene->player);
    //camera_process_keyboard_input(camera, CAMERA_UP, engine->deltaTime);
  }
  last_space_state = space_state;
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height){
	glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos){
  Engine *engine = (Engine *)glfwGetWindowUserPointer(window);

  // Don't process input (other than the Escape key) if the game is paused
  if (engine->active_scene->paused){
    // A better way to handle this: on pause, set firstMouse to true.
    // Would have to move it from a main.c global var
    //firstMouse = true;
    return;
  }

  struct Camera *camera = engine->active_scene->player.camera;

	if (firstMouse){
		lastX = (float)xpos;
		lastY = (float)ypos;
		firstMouse = false;
	}
	// Get offset from last frame, update last frame's mouse position
	float xoffset = (float)xpos - lastX;
	float yoffset = lastY - (float)ypos;
	lastX = (float)xpos;
	lastY = (float)ypos;

  // Update camera
  camera_process_mouse_input(camera, xoffset, yoffset);
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset){
  Engine *engine = (Engine *)glfwGetWindowUserPointer(window);
  struct Camera *camera = engine->active_scene->player.camera;
  camera_process_scroll_input(camera, yoffset);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods){
  Scene *active_scene = ((Engine *)glfwGetWindowUserPointer(window))->active_scene;

  // Pause
  if (key == GLFW_KEY_P && action == GLFW_PRESS){
    scene_pause(active_scene);
  }
}

Engine *engine_create(){
  Engine *engine = (Engine *)malloc(sizeof(Engine));
  if (!engine){
    printf("Error: failed to allocate Engine\n");
    return NULL;
  }

	// Init GLFW
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // Create window
	GLFWwindow *window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL){
		printf("Failed to create GLFW window\n");
    free(engine);
		glfwTerminate();
		return NULL;
	}
  engine->window = window;
  glfwMakeContextCurrent(engine->window);
	glfwSetFramebufferSizeCallback(engine->window, framebuffer_size_callback);
	glfwSetCursorPosCallback(engine->window, mouse_callback);
	glfwSetScrollCallback(engine->window, scroll_callback);
  glfwSetKeyCallback(engine->window, key_callback);
  glfwSetWindowUserPointer(engine->window, engine);

	// Capture mouse
	glfwSetInputMode(engine->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Init GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
		printf("Failed to initialize GLAD\n");
    free(engine);
		return NULL;
	}

  // Flip textures across y-axis
  stbi_set_flip_vertically_on_load(true);

	// Configure global OpenGL state
	glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  // glEnable(GL_FRAMEBUFFER_SRGB);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  //glBlendFunc(GL_ONE, GL_ONE); // additive blending

  //glDepthFunc(GL_ALWAYS);
  //glEnable(GL_STENCIL_TEST);
  //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  engine->active_scene = scene_create();
  if (!engine->active_scene){
    printf("Error: failed to create scene\n");
    free(engine);
    glfwTerminate();
    return NULL;
  }

  // Timing
  engine->deltaTime = 0.0f;
  engine->lastFrame = 0.0f;

  return engine;
}

int main(){
  Engine *engine = engine_create();
  if (!engine){
    printf("Error: failed to create Engine\n");
    return -1;
  }

  load_font_face();

	// Render loop
	while (!glfwWindowShouldClose(engine->window)){
		// Per-frame timing logic
		float currentFrame = (float)(glfwGetTime());
		engine->deltaTime = currentFrame - engine->lastFrame;
		engine->lastFrame = currentFrame;

		printf("FPS: %f\n", 1.0 / engine->deltaTime);

		// Handle input, update, render
		processInput(engine->window);
		scene_update(engine->active_scene, engine->deltaTime);
    scene_render(engine->active_scene);

		// Check and call events, swap buffers
		glfwSwapBuffers(engine->window);
		glfwPollEvents();
  }

	glfwTerminate();
	return 0;
}
