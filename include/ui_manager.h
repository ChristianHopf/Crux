#pragma once

#include "clay_opengl_renderer.h"
#include "menu.h"
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


// Planning to build a global Clay arena to which a stack of things like menus, popups, HUD elements, etc can be pushed
// for rendering. Should end up with a single function call in the main render loop to render the overlay, whatever it might be.
// Get the front of all of this built, then write an OpenGL rendering backend.
void ui_manager_init(float screen_width, float screen_height);
void ui_render_frame();
void ui_update_frame(float screen_width, float screen_height);
void ui_update_mouse(double xpos, double ypos, bool mouse_down);
void ui_draw_clay_layout(Clay_RenderCommandArray render_commands);

// LayoutFunctions
Clay_RenderCommandArray compute_clay_layout_overlay(void *arg);
Clay_RenderCommandArray compute_clay_layout_pause_menu(void *arg);

struct GameStateObserver *ui_game_state_observer_create();
void ui_game_state_changed(void *instance, struct GameState *game_state);
