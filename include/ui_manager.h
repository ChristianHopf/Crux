#pragma once

#include "clay_opengl_renderer.h"
#include "menu/menu.h"
#include "menu/menu_presets.h"
#include "text.h"
#include "game_state.h"

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

struct Layout {
  LayoutType type;
  LayoutFunction layout_function;
};

struct LayoutQueue {
  struct Layout layouts[MAX_LAYOUTS];
  int num_layouts;
};

struct UIManager {
   Clay_Arena clay_arena;
   struct LayoutQueue layout_queue;
   struct Font fonts[16];

   bool paused;
};

// Documentation later
void ui_manager_init(float screen_width, float screen_height);
void ui_render_frame();
void ui_update_frame(float screen_width, float screen_height);
void ui_update_mouse(double xpos, double ypos, bool mouse_down);
void ui_draw_clay_layout(Clay_RenderCommandArray render_commands);

// GameState observation
struct GameStateObserver *ui_game_state_observer_create();
void ui_game_state_changed(void *instance, struct GameState *game_state);
void ui_pause();
void ui_unpause();
