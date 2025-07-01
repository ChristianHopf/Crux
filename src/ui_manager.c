#include "ui_manager.h"

// Sample code from https://github.com/nicbarker/clay/blob/main/README.md, Quick Start section
// - ui_manager_init
// - ui_prepare_frame
// - MeasureText

void ui_manager_init(){
  // Init Clay
  uint64_t totalMemorySize = Clay_MinMemorySize();
  Clay_Arena arena = Clay_CreateArenaWithCapacityAndMemory(totalMemorySize, malloc(totalMemorySize));
  Clay_Initialize(arena, (Clay_Dimensions) { screenWidth, screenHeight }, (Clay_ErrorHandler) { HandleClayErrors })
}

// Need to get GLFW window data somehow
void ui_prepare_frame(){
  // Clay_SetLayoutDimensions((Clay_Dimensions) { screenWidth, screenHeight });
  // // Optional: Update internal pointer position for handling mouseover / click / touch events - needed for scrolling & debug tools
  // Clay_SetPointerState((Clay_Vector2) { mousePositionX, mousePositionY }, isMouseDown);
  // // Optional: Update internal pointer position for handling mouseover / click / touch events - needed for scrolling and debug tools
  // Clay_UpdateScrollContainers(true, (Clay_Vector2) { mouseWheelX, mouseWheelY }, deltaTime);
}

void ui_draw_clay_layout(){

}

Clay_RenderCommandArray compute_clay_layout_menu(struct Menu *menu){
  Clay_BeginLayout();

  // Build layout from menu...

  return Clay_EndLayout();
}

static inline Clay_Dimensions MeasureText(Clay_StringSlice text, Clay_TextElementConfig *config, uintptr_t userData) {
    // Clay_TextElementConfig contains members such as fontId, fontSize, letterSpacing etc
    // Note: Clay_String->chars is not guaranteed to be null terminated
    return (Clay_Dimensions) {
            .width = text.length * config->fontSize, // <- this will only work for monospace fonts, see the renderers/ directory for more advanced text measurement
            .height = config->fontSize
    };
}
