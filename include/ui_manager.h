#pragma once

#include "clay_opengl_renderer.h"
#include "menu.h"
#include "text.h"

// Forward declarations
typedef struct Clay_RenderCommandArray Clay_RenderCommandArray;
typedef struct Clay_Dimensions Clay_Dimensions;
typedef struct Clay_StringSlice Clay_StringSlice;
typedef struct Clay_TextElementConfig Clay_TextElementConfig;

// Planning to build a global Clay arena to which a stack of things like menus, popups, HUD elements, etc can be pushed
// for rendering. Should end up with a single function call in the main render loop to render the overlay, whatever it might be.
// Get the front of all of this built, then write an OpenGL rendering backend.
void ui_manager_init();
void ui_render_frame();
void ui_draw_clay_layout(Clay_RenderCommandArray render_commands);
Clay_RenderCommandArray compute_clay_layout_menu(struct Menu *menu);
