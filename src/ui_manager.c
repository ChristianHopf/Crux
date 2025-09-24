#define CLAY_IMPLEMENTATION
#include "clay.h"
#include "game_state_observer.h"
#include "game_state.h"
#include "ui_manager.h"
#include "menu/menu.h"
#include "menu/menu_presets.h"
#include "text.h"
#include "clay_opengl_renderer.h"
#include "ui/base_layouts.h"

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

void ui_handle_button_click(Clay_ElementId elementId, Clay_PointerData pointerData, intptr_t userData){
  if (pointerData.state == CLAY_POINTER_DATA_RELEASED_THIS_FRAME){
    // Call the button's action
    struct Button *button = (struct Button *)userData;
    menu_button_activate(button);
    // void (*action)(void *arg) = (void (*)(void *))userData;
    // action(NULL);
  }
}

bool ui_manager_init(struct UIManager *ui_manager, float screen_width, float screen_height){
  // Init Clay
  uint64_t totalMemorySize = Clay_MinMemorySize();
  ui_manager->clay_arena = Clay_CreateArenaWithCapacityAndMemory(totalMemorySize, malloc(totalMemorySize));
  Clay_Initialize(ui_manager->clay_arena, (Clay_Dimensions) { screen_width, screen_height }, (Clay_ErrorHandler) { HandleClayErrors });

  // Load fonts and set MeasureText function
  // ui_manager->fonts[0] = load_font_face("resources/fonts/HackNerdFontMono-Regular.ttf", 24);
  // ui_manager->fonts[1] = load_font_face("resources/fonts/HackNerdFontMono-Bold.ttf", 48);
  // ui_manager->fonts[2] = load_font_face("resources/fonts/HackNerdFontMono-Regular.ttf", 48);
  ui_load_font(ui_manager, "resources/fonts/HackNerdFontMono-Regular.ttf", 24);
  ui_load_font(ui_manager, "resources/fonts/HackNerdFontMono-Bold.ttf", 48);
  ui_load_font(ui_manager, "resources/fonts/HackNerdFontMono-Regular.ttf", 48);
  Clay_SetMeasureTextFunction(MeasureText, ui_manager->fonts);

  ui_manager->paused = false;
  ui_manager->num_fonts = 0;

  ui_manager->layout_stack.size = 0;
  ui_manager->layout_stack.capacity = MAX_LAYOUTS;

  // Load layouts (figure out a better way than hardcoding things here)
  // struct Layout layout_version_text = {
  //   .type = LAYOUT_OVERLAY,
  //   .layout_function = compute_clay_layout_overlay
  // };
  // ui_manager.layout_stack.layouts[ui_manager.layout_stack.size++] = layout_version_text;

  // Init renderer
  if (!clay_opengl_renderer_init(screen_width, screen_height)){
    return false;
  }

  return true;
}

void ui_manager_destroy(struct UIManager *ui_manager){
  for (unsigned int i = 0; i < ui_manager->num_fonts; i++){
    free(ui_manager->fonts[i]);
  }
}

void ui_load_font(struct UIManager *ui_manager, char *path, int size){
  ui_manager->fonts[ui_manager->num_fonts] = load_font_face(path, size);
  if (ui_manager->fonts[ui_manager->num_fonts]){
    ui_manager->num_fonts++;
  }
}

void ui_update_frame(struct UIManager *ui_manager, float screen_width, float screen_height, float delta_time){
  Clay_SetLayoutDimensions((Clay_Dimensions) { screen_width, screen_height });
  clay_opengl_renderer_update_dimensions(screen_width, screen_height);
  
  // Call each active layout's update function
  for (unsigned int i = 0; i < ui_manager->layout_stack.size; i++){
    struct Layout *layout = &ui_manager->layout_stack.layouts[i];
    LayoutUpdateFunction update_function = layout->layout_update_function;
    if (update_function){
      update_function(delta_time, layout->user_data);
    }
  }
}

void ui_update_mouse(double xpos, double ypos, bool mouse_down){
  Clay_SetPointerState((Clay_Vector2) { xpos, ypos }, mouse_down);
  // Clay_UpdateScrollContainers(true, (Clay_Vector2) { mouseWheelX, mouseWheelY }, deltaTime);
}

void ui_render_frame(struct UIManager *ui_manager){
  // Render each of this frame's layouts
  for(unsigned int i = 0; i < ui_manager->layout_stack.size; i++){
    struct Layout current_layout = ui_manager->layout_stack.layouts[i];
    void *arg = current_layout.user_data;

    // Get arg for layout_function based on layout type
    // switch(current_layout.type){
    //   case LAYOUT_OVERLAY: {
    //     arg = current_layout.user_data;
    //     break;
    //   }
    //   case LAYOUT_MENU: {
    //     // Get current menu (just getting pause menu for now)
    //     struct Menu *pause_menu = menu_manager_get_pause_menu();
    //     if (!pause_menu){
    //       fprintf(stderr, "Error: failed to get pause menu in ui_render_frame\n");
    //       return;
    //     }
    //     arg = (void *)pause_menu;
    //     break;
    //   }
    // }
    LayoutFunction layout_function = ui_manager->layout_stack.layouts[i].layout_function;

    // Compute and render this layout
    if (!layout_function){
      fprintf(stderr, "Error: failed to get layout function for layout %d in ui_render_frame\n", i);
      continue;
    }
    Clay_RenderCommandArray render_commands = layout_function(arg);
    clay_opengl_render(render_commands, ui_manager->fonts);
  }
}

void ui_layout_stack_push(struct UIManager *ui_manager, struct Layout *layout){
  if (ui_layout_stack_is_full(ui_manager)) return;

  ui_manager->layout_stack.layouts[ui_manager->layout_stack.size++] = *layout;
}

void ui_layout_stack_pop(struct UIManager *ui_manager){
  if (ui_layout_stack_is_empty(ui_manager)) return;

  ui_manager->layout_stack.size--;
}

// void ui_layout_stack(){
//   ui_manager.layout_stack.size = 0;
// }

bool ui_layout_stack_is_full(struct UIManager *ui_manager){
  return ui_manager->layout_stack.size >= ui_manager->layout_stack.capacity;
}

bool ui_layout_stack_is_empty(struct UIManager *ui_manager){
  return ui_manager->layout_stack.size == 0;
}

struct GameStateObserver *ui_game_state_observer_create(struct UIManager *ui_manager){
  struct GameStateObserver *ui_game_state_observer = (struct GameStateObserver *)malloc(sizeof(struct GameStateObserver));
  if (!ui_game_state_observer){
    fprintf(stderr, "Error: failed to allocate ui_game_state_observer in ui_game_state_observer_create\n");
    return NULL;
  }

  ui_game_state_observer->instance = ui_manager;
  ui_game_state_observer->notification = ui_game_state_changed;

  return ui_game_state_observer;
}

void ui_game_state_changed(void *instance, struct GameState *game_state){
  // Check instance
  struct UIManager *ui_manager = (struct UIManager *)instance;

  // Handle game state (pause)
  if (game_state->is_paused){
    ui_pause(ui_manager);
  }
  else{
    ui_unpause(ui_manager);
  }
}

void ui_pause(struct UIManager *ui_manager){
  if (!ui_manager->paused){
    // pause UI
    ui_manager->paused = true;
    struct Menu *pause_menu = menu_manager_get_pause_menu();
    if (!pause_menu){
      fprintf(stderr, "Error: failed to get pause menu in ui_render_frame\n");
      return;
    }
    struct Layout layout_pause_menu = {
      .type = LAYOUT_MENU,
      // Could maybe decouple this from my preset by giving ui_manager a struct Layout pause_menu,
      // and you have to set your own pause menu
      .layout_function = ui_base_pause_menu,
      .user_data = pause_menu,
      .layout_update_function = ui_base_pause_menu_update
    };
    ui_manager->layout_stack.layouts[ui_manager->layout_stack.size++] = layout_pause_menu;
  }
}

void ui_unpause(struct UIManager *ui_manager){
  if (ui_manager->paused){
    // unpause UI
    ui_manager->paused = false;
    ui_manager->layout_stack.size--;
  }
}
