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

typedef struct {
  GLFWwindow *window;
  Scene *active_scene;
  // Timing
  float deltaTime;
  float lastFrame;
} Engine;

typedef struct {
  unsigned int ID;
  Model *model;
  Shader *shader;
} Entity;

typedef struct {
  Entity *entities;
  int num_entities;
  int max_entities;
  Camera *camera;
} Scene;

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

// Move to a scene.c when done
//
Scene *scene_create(){
  // Allocate scene
  Scene *scene = (Scene *)malloc(sizeof(Scene));
  if (!scene){
    printf("Error: failed to allocate scene\n");
    return NULL;
  }
  scene->entity_count = 1;
  scene->max_entities = 1;
  scene->entities = (Entity *)malloc(scene->max_entities * sizeof(Entity));
  if (!scene->entities){
    printf("Error: failed to allocate scene entities\n");
    free(scene);
    return NULL;
  }

  // Camera
  scene->camera = camera_create((vec3){0.0f, 0.0f, 3.0f}, (vec3){0.0f, 1.0f, 0.0f}, -90.0f, 0.0f, 45.0f, 0.1f, 2.5f);
  if (!scene->camera){
    printf("Error: failed to create camera\n");
    free(scene->entities);
    free(scene);
    return NULL;
  }

  // Load our backpack model
  // Later make this use some kind of loading function
  Entity *backpack = (Entity *)malloc(sizeof(Entity));
  if (!backpack){
    printf("Error: failed to allocate backpack entity\n");
    free(scene->entities);
    free(scene);
    return NULL;
  }
  backpack->ID = 1;
  Model *model = model_create("resources/objects/backpack/backpack.obj");
  if (!model){
    printf("Error: failed to create Model\n");
  }
  entity->model = model;

  // Shader program
	Shader shader = shader_create("shaders/shader.vs", "shaders/shader.fs");
	if (!shader.ID){
		printf("Error: failed to create shader program\n");
		glfwTerminate();
		return NULL;
	}
  entity->shader = shader;

  return scene;
}
void scene_render(Scene *scene){
  // Render (clear color and depth buffer bits)
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Get view and projection matrices
  mat4 view;
  mat4 projection;
  camera_get_view_matrix(scene->camera, view);
  glm_perspective(glm_rad(scene->camera->fov), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f, projection);

  // For each entity in the scene
  for(int i = 0; i < scene->num_entities; i++){
    // Bind its shader
    Entity *entity = &scene->entities[i];
    shader_use(entity->shader->ID);

    // Get its model matrix
    mat4 model;
    glm_mat4_identity(model);
    glm_translate(model, (vec3){0.0f, 0.0f, 0.0f});
    glm_scale(model, (vec3){1.0f, 1.0f, 1.0f});
    shader_set_mat4(entity->shader, "model", model);

    // Set its model, view, and projection matrix uniforms
    shader_set_mat4(entity->shader, "model", model);
    shader_set_mat4(entity->shader, "view", view);
    shader_set_mat4(entity->shader, "projection", projection);

    // Draw model
    model_draw(entity->model, entity->shader);
  }
}

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
  Camera *camera = engine->camera;

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
		glfwTerminate();
		return NULL;
	}
  engine->window = window;
  glfwSetWindowUserPointer(engine->window, engine);

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

	glfwMakeContextCurrent(engine->window);
	glfwSetFramebufferSizeCallback(engine->window, framebuffer_size_callback);
	glfwSetCursorPosCallback(engine->window, mouse_callback);
	glfwSetScrollCallback(engine->window, scroll_callback);

	// Capture mouse
	glfwSetInputMode(engine->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Init GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
		printf("Failed to initialize GLAD\n");
		return -1;
	}

  // Flip textures across y-axis
  stbi_set_flip_vertically_on_load(true);

	// Configure global OpenGL state
	glEnable(GL_DEPTH_TEST);


	// Shader program
	Shader shader = shader_create("shaders/shader.vs", "shaders/shader.fs");
	if (!shader.ID){
		printf("Error: failed to create shader program\n");
		glfwTerminate();
		return -1;
	}

	// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// Render loop
	while (!glfwWindowShouldClose(engine->window)){
		// Per-frame timing logic
		float currentFrame = (float)(glfwGetTime());
		engine->deltaTime = currentFrame - engine->lastFrame;
		engine->lastFrame = currentFrame;

		// Handle input
		processInput(engine->window);

		// Render (clear and replace with background of specified color)
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use the shader program
		shader_use(&shader);

		// Projection matrix for scroll zoom feature
		mat4 projection;
		glm_perspective(glm_rad(engine->camera->fov), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f, projection);
		shader_set_mat4(&shader, "projection", projection);
		
		// Camera/view transformation
		mat4 view;
    camera_get_view_matrix(engine->camera, view);
		shader_set_mat4(&shader, "view", view);

    // Render loaded model
    mat4 model_matrix;
    glm_mat4_identity(model_matrix);
    glm_translate(model_matrix, (vec3){0.0f, 0.0f, 0.0f});
    glm_scale(model_matrix, (vec3){1.0f, 1.0f, 1.0f});
    shader_set_mat4(&shader, "model", model_matrix);
    model_draw(model, &shader);

		// Check and call events, swap buffers
		glfwSwapBuffers(engine->window);
		glfwPollEvents();
  }

	glfwTerminate();
	return 0;
}
