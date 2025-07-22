#include <cglm/cam.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#include "tinycthread/tinycthread.h"
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
#include "audio_manager.h"
#include "ui_manager.h"
#include "menu/menu.h"
#include "game_state.h"
#include "window_manager.h"

typedef struct {
  // Window
  GLFWwindow *window;
  int screen_width;
  int screen_height;
  // Mouse
  bool mouse_down;
  // Scene
  struct Scene *active_scene;
  // Timing
  float delta_time;
  float last_frame;
} Engine;

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void window_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

// Screen settings
const unsigned int SCREEN_WIDTH = 1920;
const unsigned int SCREEN_HEIGHT = 1080;

// Mouse
bool firstMouse = true;
float lastX = 960.0f;
float lastY = 540.0f;

// Lighting
vec3 lightPos = {1.2f, 0.5f, 2.0f};

static int last_space_state = GLFW_RELEASE;
void processInput(GLFWwindow *window){
  Engine *engine = (Engine *)glfwGetWindowUserPointer(window);
  struct Camera *camera = engine->active_scene->player.camera;

  // Camera movement
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
    player_process_keyboard_input(&engine->active_scene->player, CAMERA_FORWARD, engine->delta_time);
    // camera_process_keyboard_input(camera, CAMERA_FORWARD, engine->delta_time);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
    player_process_keyboard_input(&engine->active_scene->player, CAMERA_BACKWARD, engine->delta_time);
    // camera_process_keyboard_input(camera, CAMERA_BACKWARD, engine->delta_time);
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
    player_process_keyboard_input(&engine->active_scene->player, CAMERA_LEFT, engine->delta_time);
    // camera_process_keyboard_input(camera, CAMERA_LEFT, engine->delta_time);
  }
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
    player_process_keyboard_input(&engine->active_scene->player, CAMERA_RIGHT, engine->delta_time);
    // camera_process_keyboard_input(camera, CAMERA_RIGHT, engine->delta_time);
	}
	// if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS){
	//    camera_process_keyboard_input(camera, CAMERA_DOWN, engine->delta_time);
	//  }

  // Only process these inputs a single time per press
  int space_state = glfwGetKey(window, GLFW_KEY_SPACE);
	if (space_state == GLFW_PRESS && last_space_state == GLFW_RELEASE){
    player_jump(&engine->active_scene->player);
  }
  last_space_state = space_state;
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height){
  Engine *engine = (Engine *)glfwGetWindowUserPointer(window);
	glViewport(0, 0, width, height);

  int window_width, window_height;
  glfwGetWindowSize(engine->window, &window_width, &window_height);
}

void window_size_callback(GLFWwindow *window, int width, int height){
  Engine *engine = (Engine *)glfwGetWindowUserPointer(window);
  engine->screen_width = width;
  engine->screen_height = height;
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos){
  Engine *engine = (Engine *)glfwGetWindowUserPointer(window);

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
  if (!game_state_is_paused()){
    camera_process_mouse_input(camera, xoffset, yoffset);
  }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods){
  Engine *engine = (Engine *)glfwGetWindowUserPointer(window);
  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS){
    engine->mouse_down = true;
  }
  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE){
    engine->mouse_down = false;
  }
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset){
  Engine *engine = (Engine *)glfwGetWindowUserPointer(window);
  struct Camera *camera = engine->active_scene->player.camera;
  if (!game_state_is_paused()){
    camera_process_scroll_input(camera, yoffset);
  }
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods){
  Engine *engine = (Engine *)glfwGetWindowUserPointer(window);
  // Pause
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS){
    if (!game_state_is_paused()){
      game_pause();
	    glfwSetInputMode(engine->window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
      firstMouse = true;
    } else {
      game_unpause();
	    glfwSetInputMode(engine->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
  }
}

Engine *engine_create(){
  // Allocate Engine struct
  Engine *engine = (Engine *)calloc(1, sizeof(Engine));
  if (!engine){
    fprintf(stderr, "Error: failed to allocate Engine\n");
    return NULL;
  }

	// Init GLFW
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  // glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

  // Create window and register callbacks
	GLFWwindow *window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Crux", NULL, NULL);
	if (window == NULL){
		fprintf(stderr, "Failed to create GLFW window\n");
    free(engine);
		glfwTerminate();
		return NULL;
	}
  engine->window = window;
  engine->screen_width = SCREEN_WIDTH;
  engine->screen_height = SCREEN_HEIGHT;
  glfwMakeContextCurrent(engine->window);
	glfwSetFramebufferSizeCallback(engine->window, framebuffer_size_callback);
  glfwSetWindowSizeCallback(engine->window, window_size_callback);
	glfwSetCursorPosCallback(engine->window, mouse_callback);
  glfwSetMouseButtonCallback(engine->window, mouse_button_callback);
	glfwSetScrollCallback(engine->window, scroll_callback);
  glfwSetKeyCallback(engine->window, key_callback);
  glfwSetWindowUserPointer(engine->window, engine);

  // Window manager for exposing window functions
  window_manager_init(window);

	// Capture mouse
	glfwSetInputMode(engine->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Init GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
		fprintf(stderr, "Failed to initialize GLAD\n");
    free(engine);
		return NULL;
	}

  // Flip textures across y-axis
  stbi_set_flip_vertically_on_load(true);

	// Configure global OpenGL state
	glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  //glBlendFunc(GL_ONE, GL_ONE); // additive blending
  //glDepthFunc(GL_ALWAYS);
  //glEnable(GL_STENCIL_TEST);
  //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  // Initialize AudioManager
  audio_manager_init();

  // Initialize MenuManager
  menu_manager_init();

  // UI manager
  ui_manager_init(SCREEN_WIDTH, SCREEN_HEIGHT);

  // Initialize game state
  game_state_init();

  // Add game state observers
  struct GameStateObserver *audio_game_state_observer = audio_game_state_observer_create();
  if (!audio_game_state_observer){
    fprintf(stderr, "Error: failed to get audio_game_state_observer in engine_create\n");
    return NULL;
  }
  attach_observer(audio_game_state_observer);
  struct GameStateObserver *ui_game_state_observer = ui_game_state_observer_create();
  if (!ui_game_state_observer){
    fprintf(stderr, "Error: failed to get ui_game_state_observer in engine_create\n");
    return NULL;
  }
  attach_observer(ui_game_state_observer);

  // Load scene
  engine->active_scene = scene_init("scenes/wizard.json");
  if (!engine->active_scene){
    fprintf(stderr, "Error: failed to create scene\n");
    free(engine);
    glfwTerminate();
    return NULL;
  }

  // Timing
  engine->delta_time = 0.0f;
  engine->last_frame = 0.0f;

  return engine;
}

int main(){
  Engine *engine = engine_create();
  if (!engine){
    fprintf(stderr, "Error: failed to create Engine\n");
    glfwTerminate();
    return -1;
  }

	// Render loop
	while (!glfwWindowShouldClose(engine->window)){
		// Per-frame timing logic
		float currentFrame = (float)(glfwGetTime());
    
		engine->delta_time = currentFrame - engine->last_frame;
		engine->last_frame = currentFrame;
		// printf("FPS: %f\n", 1.0 / engine->delta_time);
    
		// Handle input
		processInput(engine->window);

    if (game_state_is_paused()){

      // Update Clay layout dimensions and pointer state
      ui_update_frame(engine->screen_width, engine->screen_height);

      double xpos, ypos;
      glfwGetCursorPos(engine->window, &xpos, &ypos);
      ui_update_mouse(xpos, ypos, engine->mouse_down);

      // Render UI
      ui_render_frame();
      glfwSwapBuffers(engine->window);
    }
    else{
      // float update_start_time = glfwGetTime();
      scene_update(engine->active_scene, engine->delta_time);
      // float update_end_time = glfwGetTime();
      // printf("scene update took %.2f ms\n", (update_end_time - update_start_time) * 1000.0);

      // Render scene
      // float render_start_time = glfwGetTime();
      scene_render(engine->active_scene);

      // Update Clay layout dimensions
      ui_update_frame(engine->screen_width, engine->screen_height);

      // Render UI
      ui_render_frame();

      glfwSwapBuffers(engine->window);
      // float render_end_time = glfwGetTime();
      // printf("Render took %.2f ms\n", (render_end_time - render_start_time) * 1000.0);
    }

		// Check and call events
		glfwPollEvents();

    // Check if the game should quit
    if (game_state_should_quit()){
      glfwSetWindowShouldClose(engine->window, 1);
    }
  }

  // OpenAL
  struct AudioManager *audio_manager = audio_manager_get_global();
  if (audio_manager->audio_stream){
    audio_stream_destroy(audio_manager->audio_stream);
  }
  alcDestroyContext(audio_manager->context);
  alcCloseDevice(audio_manager->device);
  free(audio_manager);

  glfwDestroyWindow(engine->window);
	glfwTerminate();
  free(engine);
	return 0;
}
