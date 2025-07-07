#define CLAY_IMPLEMENTATION
#include "clay.h"
#include "ui_manager.h"
#include "clay_opengl_renderer.h"
#include "text.h"

static Clay_Arena arena;
Clay_TextElementConfig pause_button_text_config = {.fontId = 0, .fontSize = 24, .textColor = {255, 255, 255, 255}};

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

static inline Clay_Dimensions MeasureText(Clay_StringSlice text, Clay_TextElementConfig *config, void *userData) {
    // Clay_TextElementConfig contains members such as fontId, fontSize, letterSpacing etc
    // Note: Clay_String->chars is not guaranteed to be null terminated
    return (Clay_Dimensions) {
            .width = text.length * config->fontSize, // <- this will only work for monospace fonts, see the renderers/ directory for more advanced text measurement
            .height = config->fontSize
    };
}

void ui_manager_init(){
  // Init Clay
  uint64_t totalMemorySize = Clay_MinMemorySize();
  arena = Clay_CreateArenaWithCapacityAndMemory(totalMemorySize, malloc(totalMemorySize));
  Clay_Initialize(arena, (Clay_Dimensions) { 1920, 1080 }, (Clay_ErrorHandler) { HandleClayErrors });

  // Load fonts and set MeasureText function
  // (passing fonts matters later for non-monospace text)
  fonts[0] = load_font_face("resources/fonts/HackNerdFontMono-Regular.ttf", 24);
  Clay_SetMeasureTextFunction(MeasureText, fonts);

  // Init renderer
  clay_opengl_renderer_init();
}

// Need to get GLFW window data somehow
void ui_render_frame(){
  Clay_SetLayoutDimensions((Clay_Dimensions) { 1920.0f, 1080.0f });
  // Optional: Update internal pointer position for handling mouseover / click / touch events - needed for scrolling & debug tools
  // Clay_SetPointerState((Clay_Vector2) { mousePositionX, mousePositionY }, isMouseDown);
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

Clay_RenderCommandArray compute_clay_layout_menu(struct Menu *menu){
  Clay_BeginLayout();

  // Just get a rectangle up for now
  CLAY({ .id = CLAY_ID("MenuContainer"),
    .layout = {
      // .layoutDirection = CLAY_TOP_TO_BOTTOM,
      .sizing = {
        .width = CLAY_SIZING_GROW(0),
        .height = CLAY_SIZING_GROW(0),
        // .padding = { 32, 32 }
      }
      // .childGap = 32
    },
    .backgroundColor = {0.0f, 0.2745f, 0.5294f, 1.0f},
    }) {
       CLAY_TEXT(CLAY_STRING("PAUSED"), &pause_button_text_config);
    }
  
  // Build the rest of the pause menu later
  // {
  //   CLAY(
  //     CLAY_ID("MenuTitle"),
  //     CLAY_TEXT()
  //   ) {}
  //   CLAY(
  //     CLAY_ID("MenuNav"),
  //     CLAY_LAYOUT({
  //       .sizing = {
  //         .width = CLAY_SIZING_FIXED(250),
  //         .height = CLAY_SIZING_GROW()
  //       }
  //       .childGap = 32,
  //       .childAlignment = {
  //         .x = CLAY_ALIGN_X_CENTER
  //       }
  //     })
  //   ) {
  //     CLAY(
  //       CLAY_LAYOUT({.padding = {32, 16}}),
  //       CLAY_RECTANGLE({
  //         .color = {0, 140, 255}
  //       })
  //     ) {
  //       CLAY_TEXT(CLAY_STRING("RESUME"), CLAY_TEXT_CONFIG({
  //         .fontId = 0,
  //         .fontSize = 24,
  //         .textColor = {255, 255, 255, 255}
  //       }));
  //     }
  //   }
  // }
  // Build layout from menu, maybe hardcode the translation first
  return Clay_EndLayout();
}
