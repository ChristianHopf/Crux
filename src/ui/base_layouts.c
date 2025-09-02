#include <stdio.h>
#include <string.h>
#include "ui/base_layouts.h"

Clay_TextElementConfig version_text_text_config = { .fontId = 0, .fontSize = 24, .textColor = {255, 255, 255, 255}};

struct Layout layout_version_text = {
  .type = LAYOUT_OVERLAY,
  .layout_function = ui_version_text,
  .user_data = "Crux Engine 0.3",
  .layout_update_function = NULL
};

struct Layout layout_fps_counter = {
  .type = LAYOUT_OVERLAY,
  .layout_function = ui_fps_counter,
  .user_data = NULL,
  .layout_update_function = ui_fps_counter_update
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

Clay_RenderCommandArray ui_fps_counter(void *arg){
  char *fps_text = *(char **)arg;

  Clay_BeginLayout();

  CLAY({ .id = CLAY_ID("FpsCounter"),
    .layout = {
      .sizing = {
        .width = CLAY_SIZING_GROW(),
        .height = CLAY_SIZING_GROW()
      },
       .padding = {0, 0, 30, 0}
    }
  }){
    Clay_String version_text_string = {
      .isStaticallyAllocated = false,
      .chars = fps_text,
      .length = strlen(fps_text)
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

void ui_fps_counter_update(float delta_time, void *user_data){
  float fps = (float)1 / delta_time;
  if (fps > 9999) fps = 9999;

  // If user_data was a char *, handling the memory might be tricky.
  // A char ** means user_data can always point to the same memory.
  // If that pointer already points to a char pointer, free the string first
  // before losing a reference to it.
  char **fps_text_ptr = (char **)user_data;
  if (*fps_text_ptr){
    free(*fps_text_ptr);
  }

  char fps_text[16];
  snprintf(fps_text, sizeof(fps_text), "FPS: %.0f", fps);

  *fps_text_ptr = strdup(fps_text);
  if (!*fps_text_ptr){
    *fps_text_ptr = "FPS: ERROR";
  }
}
