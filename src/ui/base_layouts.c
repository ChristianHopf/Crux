#include <string.h>
#include "ui/base_layouts.h"

Clay_TextElementConfig version_text_text_config = { .fontId = 0, .fontSize = 24, .textColor = {255, 255, 255, 255}};

struct Layout layout_version_text = {
  .type = LAYOUT_OVERLAY,
  .layout_function = ui_version_text
};

Clay_RenderCommandArray ui_version_text(void *arg){
  char *version_text = (char *)arg;

  Clay_BeginLayout();

  CLAY({ .id = CLAY_ID("VersionText"),
    .layout = {
      .sizing = {
        .width = CLAY_SIZING_GROW(),
        .height = CLAY_SIZING_GROW()
      },
    }
  }){
    Clay_String version_text_string = {
      .isStaticallyAllocated = false,
      .chars = version_text,
      .length = strlen(version_text)
    };
    CLAY_TEXT(version_text_string, &version_text_text_config);
    CLAY({
        .layout = {
          .sizing = {
            .width = CLAY_SIZING_GROW()
          }
        }
    }){}
  }

  return Clay_EndLayout();
}

void ui_fps_counter(void *arg){

}

void ui_fps_counter_update(void *arg){

}
