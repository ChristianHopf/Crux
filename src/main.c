#include <cglm/cam.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "tinycthread/tinycthread.h"
#include <stdio.h>
#include <stdbool.h>
#include "stb_image/stb_image.h"
#include <cglm/cglm.h>
#include "camera.h"
#include "shader.h"
#include "scene.h"
#include "player.h"
#include "audio_manager.h"
#include "ui_manager.h"
#include "menu/menu.h"
#include "menu/menu_presets.h"
#include "ui/base_layouts.h"
#include "game_state.h"
#include "window_manager.h"
#include "event.h"
#include "event/callbacks.h"
#include <uuid/uuid.h>
#include "engine.h"

typedef struct {
  GLFWwindow *window;
  int screen_width;
  int screen_height;
  bool mouse_down;
  // struct Scene *active_scene;
  struct SceneManager scene_manager;
  struct AudioManager audio_manager;
  struct UIManager ui_manager;
  struct GameEventQueue game_event_queue;
  float delta_time;
  float last_frame;
} Engine;

static Engine *engine = NULL;

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
  struct Scene *scene = engine->scene_manager.active_scene;

  if (!game_state_is_paused()){
    // Camera movement
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
      player_process_keyboard_input(scene, scene->local_player_entity_id, CAMERA_FORWARD, engine->delta_time);
      // player_process_keyboard_input(&engine->active_scene->player, CAMERA_FORWARD, engine->delta_time);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
      player_process_keyboard_input(scene, scene->local_player_entity_id, CAMERA_BACKWARD, engine->delta_time);
      // player_process_keyboard_input(&engine->active_scene->player, CAMERA_BACKWARD, engine->delta_time);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
      player_process_keyboard_input(scene, scene->local_player_entity_id, CAMERA_LEFT, engine->delta_time);
      // player_process_keyboard_input(&engine->active_scene->player, CAMERA_LEFT, engine->delta_time);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
      player_process_keyboard_input(scene, scene->local_player_entity_id, CAMERA_RIGHT, engine->delta_time);
      // player_process_keyboard_input(&engine->active_scene->player, CAMERA_RIGHT, engine->delta_time);
    }

    // Only process these inputs a single time per press
    int space_state = glfwGetKey(window, GLFW_KEY_SPACE);
    if (space_state == GLFW_PRESS && last_space_state == GLFW_RELEASE){
      player_jump(scene, scene->local_player_entity_id);
    }
    last_space_state = space_state;
  }
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
  struct Scene *scene = engine->scene_manager.active_scene;

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
  if (game_state_is_playing()){
    player_process_mouse_input(scene, scene->local_player_entity_id, xoffset, yoffset);
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
  struct Scene *scene = engine->scene_manager.active_scene;

  struct CameraComponent *camera = scene_get_camera_by_entity_id(scene, scene->local_player_entity_id);
  if (!game_state_is_paused()){
    camera_process_scroll_input(camera, yoffset);
  }
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods){
  Engine *engine = (Engine *)glfwGetWindowUserPointer(window);
  // Pause
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS){
    if (game_state_is_main_menu()) return;

    if (!game_state_is_paused()){
      game_state_pause();
	    glfwSetInputMode(engine->window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
      firstMouse = true;
    } else {
      game_state_unpause();
	    glfwSetInputMode(engine->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
  }
}

void engine_init(){
  // Allocate Engine struct
  engine = (Engine *)calloc(1, sizeof(Engine));
  if (!engine){
    fprintf(stderr, "Error: failed to allocate Engine\n");
    return;
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
		return;
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

  window_release_cursor();

	// Init GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
		fprintf(stderr, "Failed to initialize GLAD\n");
    free(engine);
		return;
	}

  // Flip textures across y-axis
  stbi_set_flip_vertically_on_load(true);

	// Configure global OpenGL state
	glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // Initialize AudioManager
  if (!audio_manager_init(&engine->audio_manager)){
    fprintf(stderr, "Error: failed to initialize AudioManager in engine_init\n");
    free(engine);
    return;
  }

  // Initialize MenuManager
  menu_manager_init();

  // Initialize SceneManager
  if (!scene_manager_init(&engine->scene_manager)){
    fprintf(stderr, "Error: failed to initialize SceneManager in engine_init\n");
    free(engine);
    return;
  }

  // UI manager
  if (!ui_manager_init(&engine->ui_manager, SCREEN_WIDTH, SCREEN_HEIGHT)){
    fprintf(stderr, "Error: failed to initialize UIManager in engine_init\n");
    free(engine);
    return;
  }

  ui_layout_stack_push(&engine->ui_manager, &layout_version_text);
  char **fps_text = calloc(1, sizeof(char *));
  layout_fps_counter.user_data = fps_text;
  ui_layout_stack_push(&engine->ui_manager, &layout_fps_counter);

  // Main menu layout
  struct Menu *main_menu = menu_manager_get_main_menu();
  layout_main_menu.user_data = main_menu;
  ui_layout_stack_push(&engine->ui_manager, &layout_main_menu);

  // Initialize game state
  game_state_init();

  // Add game state observers
  struct GameStateObserver *audio_game_state_observer = audio_game_state_observer_create(&engine->audio_manager);
  if (!audio_game_state_observer){
    fprintf(stderr, "Error: failed to get audio_game_state_observer in engine_create\n");
    return;
  }
  attach_observer(audio_game_state_observer);
  struct GameStateObserver *ui_game_state_observer = ui_game_state_observer_create(&engine->ui_manager);
  if (!ui_game_state_observer){
    fprintf(stderr, "Error: failed to get ui_game_state_observer in engine_create\n");
    return;
  }
  attach_observer(ui_game_state_observer);



  // Timing
  engine->delta_time = 0.0f;
  engine->last_frame = 0.0f;
}

struct SceneManager *engine_get_scene_manager(){
  return &engine->scene_manager;
}

struct AudioManager *engine_get_audio_manager(){
  return &engine->audio_manager;
}

struct UIManager *engine_get_ui_manager(){
  return &engine->ui_manager;
}

void engine_start_game(){
  if (!engine){
    fprintf(stderr, "Error: engine is null in engine_start_game\n");
    return;
  }

  if (!engine->scene_manager.active_scene){
    fprintf(stderr, "Error: no active scene in engine_start_game\n");
    return;
  }

  // Init GameState, GameEventQueue
  game_state_set_mode(GAME_STATE_PLAYING);
  game_event_queue_init(engine->scene_manager.active_scene);

  // Register event listeners
  event_listener_register(EVENT_PLAYER_ITEM_PICKUP, event_listener_on_item_pickup_add_to_inventory, engine->scene_manager.active_scene);
  event_listener_register(EVENT_PLAYER_ITEM_PICKUP, event_listener_on_item_pickup_sound, &engine->audio_manager);
  event_listener_register(EVENT_PLAYER_ITEM_PICKUP, event_listener_on_item_pickup_remove_entity, engine->scene_manager.active_scene);

  // Pop main menu layout
  ui_layout_stack_pop(&engine->ui_manager);

  // Capture cursor
  window_capture_cursor();

  stbi_set_flip_vertically_on_load(true);
}

void engine_exit_game(){
  if (!engine || !engine->scene_manager.active_scene){
    fprintf(stderr, "Error: failed to start game in start_game, engine or scene_manager is null\n");
    return;
  }

  // Unload scene
  scene_manager_unload_scene(&engine->scene_manager);
  game_event_queue_destroy();
  game_state_exit();

  // Pop pause menu, push main menu
  struct Menu *main_menu = menu_manager_get_main_menu();
  layout_main_menu.user_data = main_menu;
  // ui_layout_stack_pop(&engine->ui_manager);
  ui_layout_stack_push(&engine->ui_manager, &layout_main_menu);

  // Release cursor
  window_release_cursor();
}

void engine_free(){
  if (!engine) return;

  glfwDestroyWindow(engine->window);

  scene_manager_destroy(&engine->scene_manager);
  audio_manager_destroy(&engine->audio_manager);
  ui_manager_destroy(&engine->ui_manager);

  free(engine->game_event_queue.events);
  free(engine);
}

int main(){
  engine_init();
  if (!engine){
    fprintf(stderr, "Error: failed to create Engine\n");
    glfwTerminate();
    return -1;
  }

  // start_game();

	// Render loop
	while (!glfwWindowShouldClose(engine->window)){
		// Per-frame timing logic
		float currentFrame = (float)(glfwGetTime());
    
		engine->delta_time = currentFrame - engine->last_frame;
		engine->last_frame = currentFrame;
		// printf("FPS: %f\n", 1.0 / engine->delta_time);
    
		// Handle input
		processInput(engine->window);

    // if (game_state_is_paused()){

    // Update Clay layout dimensions and pointer state
    ui_update_frame(&engine->ui_manager, engine->screen_width, engine->screen_height, engine->delta_time);

    GameStateMode mode = game_state_get_mode();
    if (mode != GAME_STATE_PLAYING){
      double xpos, ypos;
      glfwGetCursorPos(engine->window, &xpos, &ypos);
      ui_update_mouse(xpos, ypos, engine->mouse_down);
    }

    struct Scene *active_scene = engine->scene_manager.active_scene;
    if (active_scene){
      if (mode == GAME_STATE_PLAYING){
        scene_update(active_scene, engine->delta_time);
      }
      scene_render(active_scene);
    }
    // Render UI
    ui_render_frame(&engine->ui_manager);

    glfwSwapBuffers(engine->window);
		glfwPollEvents();

    // Check if the game should quit
    if (game_state_should_quit()){
      glfwSetWindowShouldClose(engine->window, 1);
    }
  }

  // Teardown
  engine_free();
  glfwTerminate();

	return 0;
}
