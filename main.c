#include <cglm/mat4.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdbool.h>
#include "stb_image/stb_image.h"
#include <cglm/cglm.h>
#include "camera.h"
#include "shader.h"
#include "model.h"
#include "scene.h"

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

// Screen settings
const unsigned int SCREEN_WIDTH = 800;
const unsigned int SCREEN_HEIGHT = 600;

// Mouse
bool firstMouse = true;
float lastX = 400.0f;
float lastY = 300.0f;

// Lighting
vec3 lightPos = {1.2f, 0.5f, 2.0f};

void processInput(GLFWwindow *window){
	if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
		glfwSetWindowShouldClose(window, 1);
	}
  Engine *engine = (Engine *)glfwGetWindowUserPointer(window);
  Camera *camera = engine->active_scene->camera;

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
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS){
    camera_process_keyboard_input(camera, CAMERA_UP, engine->deltaTime);
  }
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height){
	glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos){
  Engine *engine = (Engine *)glfwGetWindowUserPointer(window);
  Camera *camera = engine->active_scene->camera;

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
  Camera *camera = engine->active_scene->camera;
  camera_process_scroll_input(camera, yoffset);
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

	// Render loop
	while (!glfwWindowShouldClose(engine->window)){
		// Per-frame timing logic
		float currentFrame = (float)(glfwGetTime());
		engine->deltaTime = currentFrame - engine->lastFrame;
		engine->lastFrame = currentFrame;

    printf("FPS: %f\n", 1.0 / engine->deltaTime);

		// Handle input
		processInput(engine->window);

    scene_update(engine->active_scene, engine->deltaTime);

    // Render scene
    scene_render(engine->active_scene);

		// Check and call events, swap buffers
		glfwSwapBuffers(engine->window);
		glfwPollEvents();
  }

	glfwTerminate();
	return 0;
}
