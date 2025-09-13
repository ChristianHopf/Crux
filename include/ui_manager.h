#pragma once

#include "clay.h"
#include <stdbool.h>

// Forward declarations
typedef struct Clay_RenderCommandArray Clay_RenderCommandArray;
typedef struct Clay_Dimensions Clay_Dimensions;
typedef struct Clay_StringSlice Clay_StringSlice;
typedef struct Clay_TextElementConfig Clay_TextElementConfig;

#define MAX_LAYOUTS 16

typedef enum {
  LAYOUT_OVERLAY,
  LAYOUT_MENU
} LayoutType;

typedef Clay_RenderCommandArray (*LayoutFunction)(void *arg);
typedef void (*LayoutUpdateFunction)(float delta_time, void *user_data);

struct Layout {
  LayoutType type;
  LayoutFunction layout_function;
  void *user_data;
  LayoutUpdateFunction layout_update_function;
};

struct LayoutStack {
  struct Layout layouts[MAX_LAYOUTS];
  unsigned int size;
  unsigned int capacity;
};

struct UIManager {
   Clay_Arena clay_arena;
   struct LayoutStack layout_stack;
   struct Font *fonts[16];
   int num_fonts;

   bool paused;
};

void ui_handle_button_click(Clay_ElementId elementId, Clay_PointerData pointerData, intptr_t userData);

// Documentation later
bool ui_manager_init(struct UIManager *ui_manager, float screen_width, float screen_height);
void ui_load_font(struct UIManager *ui_manager, char *path, int size);
void ui_render_frame(struct UIManager *ui_manager);
void ui_update_frame(struct UIManager *ui_manager, float screen_width, float screen_height, float delta_time);
void ui_update_mouse(double xpos, double ypos, bool mouse_down);

// Layout stack
void ui_layout_stack_push(struct UIManager *ui_manager, struct Layout *layout);
void ui_layout_stack_pop(struct UIManager *ui_manager);
void ui_layout_stack_clear(struct UIManager *ui_manager);
bool ui_layout_stack_is_full(struct UIManager *ui_manager);
bool ui_layout_stack_is_empty(struct UIManager *ui_manager);

// GameState observation
struct GameStateObserver *ui_game_state_observer_create();
void ui_game_state_changed(void *instance, struct GameState *game_state);
void ui_pause(struct UIManager *ui_manager);
void ui_unpause(struct UIManager *ui_manager);
