#define CLAY_IMPLEMENTATION
#include "clay.h"
#include "ui_manager.h"
#include "clay_opengl_renderer.h"
#include "text.h"

static Clay_Arena arena;
static struct LayoutQueue layout_queue = {0};

Clay_TextElementConfig pause_menu_title_text_config = {.fontId = 0, .fontSize = 24, .textColor = {255, 255, 255, 255}};
Clay_TextElementConfig pause_menu_button_text_config = {.fontId = 1, .fontSize = 48, .lineHeight = 48, .textAlignment = CLAY_TEXT_ALIGN_CENTER, .textColor = {255, 255, 255, 255}};
Clay_TextElementConfig overlay_version_text_config = { .fontId = 0, .fontSize = 24, .textColor = {255, 255, 255, 255}};

static struct Font fonts[16];

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
  // Clay_TextElementConfig contains members such as fontId, fontSize, letterSpacing etc
  // Note: Clay_String->chars is not guaranteed to be null terminated

  Clay_Dimensions text_size = {0};
  float max_text_width = 0.0f;
  float line_text_width = 0.0f;
  int max_line_char_count = 0;
  int line_char_count = 0;

  float text_height = config->fontSize;
  struct Font *fonts = (struct Font *)userData;
  struct Font font_to_use = fonts[config->fontId];

  float scale = config->fontSize / (float)font_to_use.base_size;

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
    struct Character current_character = font_to_use.characters[index];
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
  arena = Clay_CreateArenaWithCapacityAndMemory(totalMemorySize, malloc(totalMemorySize));
  Clay_Initialize(arena, (Clay_Dimensions) { screen_width, screen_height }, (Clay_ErrorHandler) { HandleClayErrors });
  // Load fonts and set MeasureText function
  // (passing fonts matters later for non-monospace text)
  fonts[0] = load_font_face("resources/fonts/HackNerdFontMono-Regular.ttf", 24);
  fonts[1] = load_font_face("resources/fonts/HackNerdFontMono-Regular.ttf", 48);
  // fonts[1] = load_font_face("resources/fonts/OpenSans-Regular.ttf", 48);
  Clay_SetMeasureTextFunction(MeasureText, fonts);

  // Load layouts (figure out a better way than hardcoding things here)
  struct Layout layout_version_text = {
    .type = LAYOUT_OVERLAY,
    .layout_function = compute_clay_layout_overlay
  };
  layout_queue.layouts[layout_queue.num_layouts++] = layout_version_text;

  // Init renderer
  clay_opengl_renderer_init(screen_width, screen_height);
}

void ui_update_frame(float screen_width, float screen_height){
  Clay_SetLayoutDimensions((Clay_Dimensions) { screen_width, screen_height });
  clay_opengl_renderer_update_dimensions(screen_width, screen_height);
}

void ui_update_mouse(double xpos, double ypos, bool mouse_down){
  Clay_SetPointerState((Clay_Vector2) { xpos, ypos }, mouse_down);
  // Clay_UpdateScrollContainers(true, (Clay_Vector2) { mouseWheelX, mouseWheelY }, deltaTime);
}

// Need to get GLFW window data somehow
void ui_render_frame(){
  // TODO: Use a queue of some new struct for UI elements.
  // Other places in code can push and pop from the stack.
  // compute_clay_layout would accept the stack and generate render commands
  // by traversing the queue.
  // clay_opengl_render wouldn't change, it only needs one array of render commands.

  // Render each of this frame's layouts
  printf("Time to render %d layouts\n", layout_queue.num_layouts);
  for(int i = 0; i < layout_queue.num_layouts; i++){
    struct Layout current_layout = layout_queue.layouts[i];
    void *arg = NULL;

    switch(current_layout.type){
      case LAYOUT_OVERLAY: {
        arg = (void *)"Crux Engine 0.3";
        break;
      }
      case LAYOUT_MENU: {
        // Get current menu
        struct Menu *pause_menu = menu_manager_get_pause_menu();
        if (!pause_menu){
          fprintf(stderr, "Error: failed to get pause menu in ui_render_frame\n");
          return;
        }
        arg = (void *)pause_menu;
        break;
      }
    }
    LayoutFunction layout_function = layout_queue.layouts[i].layout_function;
    if (!layout_function){
      fprintf(stderr, "Error: failed to get layout function for layout %d in ui_render_frame\n", i);
      continue;
    }
    Clay_RenderCommandArray render_commands = layout_function(arg);
    clay_opengl_render(render_commands, fonts);
  }
}

void handle_button_click(Clay_ElementId elementId, Clay_PointerData pointerData, intptr_t userData){
  printf("Hovering\n");
  if (pointerData.state == CLAY_POINTER_DATA_RELEASED_THIS_FRAME){
    printf("Button clicked!\n");

    // Call the button's action
    void (*action)(void *arg) = (void (*)(void *))userData;
    action(NULL);
  }
}

void pause_menu_button(struct Button button){
  CLAY({
    .layout = {
      .padding = {16, 16, 0, 12}
    },
    .backgroundColor = Clay_Hovered() ? (Clay_Color){0.0f, 0.549f, 1.0f, 0.8f} : (Clay_Color){0.0f, 0.3745f, 0.6294f, 1.0f}
  }){
    Clay_OnHover(handle_button_click, (intptr_t)button.data.action);
    Clay_String button_string = {
      .isStaticallyAllocated = false,
      .chars = button.text,
      .length = strlen(button.text)
    };
    CLAY_TEXT(button_string, &pause_menu_button_text_config);
  }
}

Clay_RenderCommandArray compute_clay_layout_overlay(void *arg){
  char *version_text = (char *)arg;

  Clay_BeginLayout();

  CLAY({ .id = CLAY_ID("VersionText")}){
    Clay_String version_text_string = {
      .isStaticallyAllocated = false,
      .chars = version_text,
      .length = strlen(version_text)
    };
    CLAY_TEXT(CLAY_STRING("Crux Engine 0.3"), &overlay_version_text_config);
  }

  return Clay_EndLayout();
}

Clay_RenderCommandArray compute_clay_layout_pause_menu(void *arg){
  struct Menu *menu = (struct Menu *)arg;

  Clay_BeginLayout();

  CLAY({ .id = CLAY_ID("MenuContainer"),
    .layout = {
      .layoutDirection = CLAY_TOP_TO_BOTTOM,
      .sizing = {
        .width = CLAY_SIZING_GROW(),
        .height = CLAY_SIZING_GROW()
      },
       .padding = {0, 0, 0, 32 },
       .childAlignment = {
        .x = CLAY_ALIGN_X_CENTER
       },
       .childGap = 32
    },
    .backgroundColor = {0.0f, 0.2745f, 0.5294f, 1.0f}
    }) {
    CLAY({ .id = CLAY_ID("MenuHeader"),
      .layout = {
        .layoutDirection = CLAY_TOP_TO_BOTTOM,
        .sizing = {
          .width = CLAY_SIZING_GROW(),
          .height = CLAY_SIZING_FIXED(60)
        },
        .childAlignment = {
          .x = CLAY_ALIGN_X_CENTER,
          .y = CLAY_ALIGN_Y_CENTER
        }
      }
    }) {
      CLAY_TEXT(CLAY_STRING("PAUSED"), &pause_menu_title_text_config);
    }
    CLAY({ .id = CLAY_ID("MenuNav"),
      .layout = {
        .layoutDirection = CLAY_TOP_TO_BOTTOM,
        .sizing = {
          .width = CLAY_SIZING_FIXED(400),
          .height = CLAY_SIZING_GROW()
        },
        .childAlignment = {
          .x = CLAY_ALIGN_X_CENTER,
        },
        .padding = { 8, 8, 8, 8 },
        .childGap = 16
      },
      .backgroundColor = {0.0f, 0.3745f, 0.6294f, 1.0f}
    }) {
      pause_menu_button(menu->buttons[0]);
      CLAY({
        .layout = {
          .sizing = {
            .height = CLAY_SIZING_GROW()
          }
        }
      }){}
      pause_menu_button(menu->buttons[1]);
    }
  }

  return Clay_EndLayout();
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
    // Maybe make another function for this to be less tightly coupled w my own menu
    // For now, just assume you can push or pop once
    printf("ui_game_state_changed: paused!\n");
    struct Layout layout_pause_menu = {
      .type = LAYOUT_MENU,
      .layout_function = compute_clay_layout_pause_menu
    };
    layout_queue.layouts[layout_queue.num_layouts++] = layout_pause_menu;
  }
  else{
    // pop pause menu
    printf("ui_game_state_changed: unpaused!\n");
    // layout_queue.layouts[layout_queue.num_layouts--] = NULL;
    layout_queue.num_layouts--;
  }
}
