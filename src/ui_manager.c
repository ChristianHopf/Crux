#define CLAY_IMPLEMENTATION
#include "clay.h"
#include "game_state_observer.h"
#include "game_state.h"
#include "ui_manager.h"
#include "menu/menu.h"
#include "menu/menu_presets.h"
#include "text.h"
#include "clay_opengl_renderer.h"

static struct UIManager ui_manager;

// Sample code from https://github.com/nicbarker/clay/blob/main/README.md, Quick Start section
// - ui_manager_init
// - ui_prepare_frame
// - MeasureText

void HandleClayErrors(Clay_ErrorData errorData) {
    // See the Clay_ErrorData struct for more information
    printf("%s", errorData.errorText.chars);
    switch(errorData.errorType) {
      default:
        break;
    }
}

// Adapted from Raylib_MeasureText (clay/renderers/raylib/clay_renderer/raylib.c)
static inline Clay_Dimensions MeasureText(Clay_StringSlice text, Clay_TextElementConfig *config, void *userData) {
  Clay_Dimensions text_size = {0};
  float max_text_width = 0.0f;
  float line_text_width = 0.0f;
  int max_line_char_count = 0;
  int line_char_count = 0;

  float text_height = config->fontSize;
  struct Font **fonts = (struct Font **)userData;
  struct Font *font_to_use = fonts[config->fontId];

  float scale = config->fontSize / (float)font_to_use->base_size;

  for(int i = 0; i < text.length; i++, line_char_count++){
    // Newline -> set maxes, reset line text width and char count
    if (text.chars[i] == '\n'){
      max_text_width = fmax(max_text_width, line_text_width);
      max_line_char_count = CLAY__MAX(max_line_char_count, line_char_count);
      line_text_width = 0;
      line_char_count = 0;
      continue;
    }

    int index = text.chars[i] - 32;
    struct Character current_character = font_to_use->characters[index];
    if (current_character.advance != 0){
      line_text_width += (current_character.advance >> 6) * scale;
    }
    else{
      line_text_width += (current_character.size[0] + current_character.bearing[0]) * scale;
    }
  }
  max_text_width = fmax(max_text_width, line_text_width);
  max_line_char_count = CLAY__MAX(max_line_char_count, line_char_count);

  // Width is scaled max text width + letter spacing multiplied by the number of chars
// text_size.width = max_text_width + (max_line_char_count > 1 ? (max_line_char_count - 1) * config->letterSpacing * scale : 0);
  text_size.width = max_text_width * scale + (max_line_char_count * config->letterSpacing);
  text_size.height = text_height;

  return text_size;
}

void ui_manager_init(float screen_width, float screen_height){
  // Init Clay
  uint64_t totalMemorySize = Clay_MinMemorySize();
  ui_manager.clay_arena = Clay_CreateArenaWithCapacityAndMemory(totalMemorySize, malloc(totalMemorySize));
  Clay_Initialize(ui_manager.clay_arena, (Clay_Dimensions) { screen_width, screen_height }, (Clay_ErrorHandler) { HandleClayErrors });

  // Load fonts and set MeasureText function
  ui_manager.fonts[0] = load_font_face("resources/fonts/HackNerdFontMono-Regular.ttf", 24);
  ui_manager.fonts[1] = load_font_face("resources/fonts/HackNerdFontMono-Bold.ttf", 48);
  ui_manager.fonts[2] = load_font_face("resources/fonts/HackNerdFontMono-Regular.ttf", 48);
  Clay_SetMeasureTextFunction(MeasureText, ui_manager.fonts);

  ui_manager.paused = false;
  ui_manager.num_fonts = 0;

  // Load layouts (figure out a better way than hardcoding things here)
  struct Layout layout_version_text = {
    .type = LAYOUT_OVERLAY,
    .layout_function = compute_clay_layout_overlay
  };
  ui_manager.layout_stack.layouts[ui_manager.layout_stack.num_layouts++] = layout_version_text;

  // Init renderer
  clay_opengl_renderer_init(screen_width, screen_height);
}

void ui_load_font(char *path, int size){
  ui_manager.fonts[ui_manager.num_fonts] = load_font_face(path, size);
  if (ui_manager.fonts[ui_manager.num_fonts]){
    ui_manager.num_fonts++;
  }
}

void ui_update_frame(float screen_width, float screen_height){
  Clay_SetLayoutDimensions((Clay_Dimensions) { screen_width, screen_height });
  clay_opengl_renderer_update_dimensions(screen_width, screen_height);
}

void ui_update_mouse(double xpos, double ypos, bool mouse_down){
  Clay_SetPointerState((Clay_Vector2) { xpos, ypos }, mouse_down);
  // Clay_UpdateScrollContainers(true, (Clay_Vector2) { mouseWheelX, mouseWheelY }, deltaTime);
}

void ui_render_frame(){
  // Render each of this frame's layouts
  for(int i = 0; i < ui_manager.layout_stack.num_layouts; i++){
    struct Layout current_layout = ui_manager.layout_stack.layouts[i];
    void *arg = NULL;

    // Get arg for layout_function based on layout type
    switch(current_layout.type){
      case LAYOUT_OVERLAY: {
        arg = (void *)"Crux Engine 0.3";
        break;
      }
      case LAYOUT_MENU: {
        // Get current menu (just getting pause menu for now)
        struct Menu *pause_menu = menu_manager_get_pause_menu();
        if (!pause_menu){
          fprintf(stderr, "Error: failed to get pause menu in ui_render_frame\n");
          return;
        }
        arg = (void *)pause_menu;
        break;
      }
    }
    LayoutFunction layout_function = ui_manager.layout_stack.layouts[i].layout_function;

    // Compute and render this layout
    if (!layout_function){
      fprintf(stderr, "Error: failed to get layout function for layout %d in ui_render_frame\n", i);
      continue;
    }
    Clay_RenderCommandArray render_commands = layout_function(arg);
    clay_opengl_render(render_commands, ui_manager.fonts);
  }
}

struct GameStateObserver *ui_game_state_observer_create(){
  struct GameStateObserver *ui_game_state_observer = (struct GameStateObserver *)malloc(sizeof(struct GameStateObserver));
  if (!ui_game_state_observer){
    fprintf(stderr, "Error: failed to allocate ui_game_state_observer in ui_game_state_observer_create\n");
    return NULL;
  }

  ui_game_state_observer->instance = NULL;
  ui_game_state_observer->notification = ui_game_state_changed;

  return ui_game_state_observer;
}

void ui_game_state_changed(void *instance, struct GameState *game_state){
  // Check instance

  // Handle game state (pause)
  if (game_state->is_paused){
    ui_pause();
  }
  else{
    ui_unpause();
  }
}

void ui_pause(){
  if (!ui_manager.paused){
    // pause UI
    ui_manager.paused = true;
    struct Layout layout_pause_menu = {
      .type = LAYOUT_MENU,
      // Could maybe decouple this from my preset by giving ui_manager a struct Layout pause_menu,
      // and you have to set your own pause menu
      .layout_function = compute_clay_layout_pause_menu
    };
    ui_manager.layout_stack.layouts[ui_manager.layout_stack.num_layouts++] = layout_pause_menu;
  }
}

void ui_unpause(){
  if (ui_manager.paused){
    // unpause UI
    ui_manager.paused = false;
    ui_manager.layout_stack.num_layouts--;
  }
}
