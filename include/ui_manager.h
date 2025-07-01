#define CLAY_IMPLEMENTATION
#include "clay.h"
#include "menu.h"

// Planning to build a global Clay arena to which a stack of things like menus, popups, HUD elements, etc can be pushed
// for rendering. Should end up with a single function call in the main render loop to render the overlay, whatever it might be.
// Get the front of all of this built, then write an OpenGL rendering backend.
void ui_manager_init();
void ui_manager_prepare_frame();
void ui_draw_clay_layout();
Clay_RenderCommandArray compute_clay_layout_menu(struct Menu *menu);

// Helpers
static inline Clay_Dimensions MeasureText(Clay_StringSlice text, Clay_TextElementConfig *config, uintptr_t userData);
