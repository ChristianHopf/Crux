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
#include "menu.h"
#include "game_state.h"

typedef struct {
  GLFWwindow *window;
  struct Scene *active_scene;
  struct GameState game_state;
  // Timing
  float delta_time;
  float last_frame;
  // Multithreading
  mtx_t scene_mutex;
  cnd_t render_signal;
  cnd_t render_done_signal;
  bool render_ready;
} Engine;

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
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
    camera_process_keyboard_input(camera, CAMERA_FORWARD, engine->delta_time);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
    camera_process_keyboard_input(camera, CAMERA_BACKWARD, engine->delta_time);
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
    camera_process_keyboard_input(camera, CAMERA_LEFT, engine->delta_time);
  }
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
    camera_process_keyboard_input(camera, CAMERA_RIGHT, engine->delta_time);
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS){
    camera_process_keyboard_input(camera, CAMERA_DOWN, engine->delta_time);
  }

  // Only process these inputs a single time per press
  int space_state = glfwGetKey(window, GLFW_KEY_SPACE);
	if (space_state == GLFW_PRESS && last_space_state == GLFW_RELEASE){
    player_jump(&engine->active_scene->player);
  }
  last_space_state = space_state;
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height){
	glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos){
  Engine *engine = (Engine *)glfwGetWindowUserPointer(window);

  // Don't process input (other than the Escape key) if the game is paused
  // if (engine->active_scene->paused){
  //   // A better way to handle this: on pause, set firstMouse to true.
  //   // Would have to move it from a main.c global var
  //   //firstMouse = true;
  //   return;
  // }

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

  printf("Last X: %f, last Y: %f\n", lastX, lastY);
  printf("xoffset: %f, yoffset: %f\n", xoffset, yoffset);

  // Update camera
  if (!engine->game_state.is_paused){
    camera_process_mouse_input(camera, xoffset, yoffset);
  }
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset){
  Engine *engine = (Engine *)glfwGetWindowUserPointer(window);
  struct Camera *camera = engine->active_scene->player.camera;
  camera_process_scroll_input(camera, yoffset);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods){
  Engine *engine = (Engine *)glfwGetWindowUserPointer(window);
  // Pause
  if (key == GLFW_KEY_P && action == GLFW_PRESS){
    if (!engine->game_state.is_paused){
      game_pause(&engine->game_state);
      game_state_update(&engine->game_state);
	    glfwSetInputMode(engine->window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    } else {
      game_unpause(&engine->game_state);
      game_state_update(&engine->game_state);
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

  // Create window and register callbacks
	GLFWwindow *window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Crux", NULL, NULL);
	if (window == NULL){
		fprintf(stderr, "Failed to create GLFW window\n");
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

  // Initialize game state
  engine->game_state = game_state_init();

  // Add game state observers
  struct GameStateObserver *audio_game_state_observer = audio_game_state_observer_create();
  if (!audio_game_state_observer){
    fprintf(stderr, "Error: failed to get audio_game_state_observer in engine_create\n");
    return NULL;
  }
  attach_observer(&engine->game_state, audio_game_state_observer);

  // Initialize MenuManager
  menu_manager_init();

  // Load scene
  engine->active_scene = scene_init("scenes/bouncehouse.json");
  if (!engine->active_scene){
    fprintf(stderr, "Error: failed to create scene\n");
    free(engine);
    glfwTerminate();
    return NULL;
  }

  // Timing
  engine->delta_time = 0.0f;
  engine->last_frame = 0.0f;

  // Scene mutex
  mtx_init(&engine->scene_mutex, mtx_plain);
  cnd_init(&engine->render_signal);
  cnd_init(&engine->render_done_signal);
  engine->render_ready = false;

  return engine;
}

// int render_thread(void *arg){
//   Engine *engine = (Engine *)arg;
//   glfwMakeContextCurrent(engine->window);
//   while (!glfwWindowShouldClose(engine->window)){
//     // Lock mutex
//     mtx_lock(&engine->scene_mutex);
//
//     // Wait for render signal
//     while (!engine->render_ready){
//       cnd_wait(&engine->render_signal, &engine->scene_mutex);
//     }
//
//     // Render scene
//     scene_render(engine->active_scene);
//     glfwSwapBuffers(engine->window);
//     printf("RENDER THREAD STILL WORKING!\n");
//
//     // Set render_ready back to false, signal rendering is done, unlock mutex
//     engine->render_ready = false;
//     cnd_signal(&engine->render_done_signal);
//     mtx_unlock(&engine->scene_mutex);
//   }
//
//   glfwMakeContextCurrent(NULL);
//   return 0;
// }

int main(){
  Engine *engine = engine_create();
  if (!engine){
    fprintf(stderr, "Error: failed to create Engine\n");
    glfwTerminate();
    return -1;
  }

  // Load font
  load_font_face();

  // thrd_t render_thrd;
  // thrd_create(&render_thrd, render_thread, engine);

	// Render loop
	while (!glfwWindowShouldClose(engine->window)){
		// Per-frame timing logic
		float currentFrame = (float)(glfwGetTime());
    
		engine->delta_time = currentFrame - engine->last_frame;
		engine->last_frame = currentFrame;
		// printf("FPS: %f\n", 1.0 / engine->delta_time);

		// Handle input
		processInput(engine->window);

    if (engine->game_state.is_paused){
      // Render pause menu
      // printf("paused!\n");
      glfwMakeContextCurrent(engine->window);
      menu_render();
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
      glfwSwapBuffers(engine->window);
      
      // float render_end_time = glfwGetTime();
      // printf("Render took %.2f ms\n", (render_end_time - render_start_time) * 1000.0);
    }

		// Check and call events
		glfwPollEvents();
  }

  // thrd_join(render_thrd, NULL);
  mtx_destroy(&engine->scene_mutex);
  cnd_destroy(&engine->render_signal);
  cnd_destroy(&engine->render_done_signal);

  // OpenAL
  // alcMakeContextCurrent(NULL);
  struct AudioManager *audio_manager = audio_manager_get_global();
  audio_stream_destroy(audio_manager->audio_stream);
  alcDestroyContext(audio_manager->context);
  alcCloseDevice(audio_manager->device);
  free(audio_manager);

  glfwDestroyWindow(engine->window);
	glfwTerminate();
  free(engine);
	return 0;
}
