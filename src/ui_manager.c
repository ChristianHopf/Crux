#define CLAY_IMPLEMENTATION
#include "clay.h"
#include "ui_manager.h"
#include "clay_opengl_renderer.h"
#include "text.h"

static Clay_Arena arena;
Clay_TextElementConfig pause_menu_text_config = {.fontId = 0, .fontSize = 24, .textColor = {255, 255, 255, 255}};

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
      line_text_width += current_character.advance >> 6;
    }
    else{
      line_text_width += current_character.size[0] + current_character.bearing[0];
    }
  }
  max_text_width = fmax(max_text_width, line_text_width);
  max_line_char_count = CLAY__MAX(max_line_char_count, line_char_count);

  // Width is scaled max text width + letter spacing multiplied by the number of chars
  text_size.width = max_text_width * scale + (line_char_count * config->letterSpacing);
  text_size.height = text_height;

  return text_size;

  // return (Clay_Dimensions) {
  //         .width = text.length * config->fontSize, // <- this will only work for monospace fonts, see the renderers/ directory for more advanced text measurement
  //         .height = config->fontSize
  // };
}

void ui_manager_init(float screen_width, float screen_height){
  // Init Clay
  uint64_t totalMemorySize = Clay_MinMemorySize();
  arena = Clay_CreateArenaWithCapacityAndMemory(totalMemorySize, malloc(totalMemorySize));
  Clay_Initialize(arena, (Clay_Dimensions) { screen_width, screen_height }, (Clay_ErrorHandler) { HandleClayErrors });
  // Load fonts and set MeasureText function
  // (passing fonts matters later for non-monospace text)
  fonts[0] = load_font_face("resources/fonts/HackNerdFontMono-Regular.ttf", 24);
  Clay_SetMeasureTextFunction(MeasureText, fonts);

  // Init renderer
  clay_opengl_renderer_init(screen_width, screen_height);
}

void ui_update_frame(float screen_width, float screen_height, double xpos, double ypos, bool mouse_down){
  Clay_SetLayoutDimensions((Clay_Dimensions) { screen_width, screen_height });
  Clay_SetPointerState((Clay_Vector2) { xpos, ypos }, mouse_down);
  clay_opengl_renderer_update_dimensions(screen_width, screen_height);
}

// Need to get GLFW window data somehow
void ui_render_frame(){
  // Optional: Update internal pointer position for handling mouseover / click / touch events - needed for scrolling & debug tools
  // Optional: Update internal pointer position for handling mouseover / click / touch events - needed for scrolling and debug tools
  // Clay_UpdateScrollContainers(true, (Clay_Vector2) { mouseWheelX, mouseWheelY }, deltaTime);

  // Call ui_draw_call_layout for each set of render commands
  // Probably don't have to worry about clearing color and depth bits?

  struct Menu *pause_menu = menu_manager_get_pause_menu();
  if (!pause_menu){
    fprintf(stderr, "Error: failed to get pause menu in ui_render_frame\n");
    return;
  }
  Clay_RenderCommandArray render_commands = compute_clay_layout_menu(pause_menu);
  clay_opengl_render(render_commands, fonts);
  // ui_draw_clay_layout(render_commands);
}

// void ui_draw_clay_layout(Clay_RenderCommandArray render_commands){
//   clay_opengl_render(render_commands);
// }

void pause_menu_button(Clay_String text){
  CLAY({
    .layout = {
      .padding = {16, 16, 8, 8}
    },
    // .backgroundColor = {0.0f, 0.549f, 1.0f, 1.0f}
  }){
    CLAY_TEXT(text, &pause_menu_text_config);
  }
}

Clay_RenderCommandArray compute_clay_layout_menu(struct Menu *menu){
  Clay_BeginLayout();

  // Just get a rectangle up for now
  CLAY({ .id = CLAY_ID("MenuContainer"),
    .layout = {
      .layoutDirection = CLAY_TOP_TO_BOTTOM,
      .sizing = {
        .width = CLAY_SIZING_GROW(),
        .height = CLAY_SIZING_GROW(),
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
          .height = CLAY_SIZING_FIXED(60),
        },
        // .padding = { 64, 64, 64, 64 },
        .childAlignment = {
          .x = CLAY_ALIGN_X_CENTER,
          .y = CLAY_ALIGN_Y_CENTER,
        }
        // .childGap = 32
      },
      .backgroundColor = {0.0f, 0.3745f, 0.6294f, 1.0f}
    }) {
      CLAY_TEXT(CLAY_STRING("PAUSED"), &pause_menu_text_config);
    }
    CLAY({ .id = CLAY_ID("MenuNav"),
      .layout = {
        .layoutDirection = CLAY_TOP_TO_BOTTOM,
        .sizing = {
          .width = CLAY_SIZING_FIXED(250),
          .height = CLAY_SIZING_GROW(),
        },
        .padding = {0, 0, 4, 4 },
        .childGap = 16,
      },
      .backgroundColor = {0.0f, 0.3745f, 0.6294f, 1.0f}
    }) {
      pause_menu_button(CLAY_STRING("RESUME"));
      pause_menu_button(CLAY_STRING("EXIT"));
    }
  }
  // Build layout from menu, maybe hardcode the translation first
  return Clay_EndLayout();
}
