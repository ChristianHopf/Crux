#include "menu/menu_presets.h"

Clay_TextElementConfig pause_menu_title_text_config = {.fontId = 1, .fontSize = 48, .textColor = {255, 255, 255, 255}};
Clay_TextElementConfig pause_menu_button_text_config = {.fontId = 2, .fontSize = 48, .lineHeight = 48, .textAlignment = CLAY_TEXT_ALIGN_CENTER, .textColor = {255, 255, 255, 255}};
Clay_TextElementConfig overlay_version_text_config = { .fontId = 0, .fontSize = 24, .textColor = {255, 255, 255, 255}};

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
    .backgroundColor = Clay_Hovered() ? (Clay_Color){0.0f, 0.549f, 1.0f, 0.8f} : (Clay_Color){0.0f, 0.3745f, 0.6294f, 0.0f}
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
    Clay_String oiiai_count_string = {
       .isStaticallyAllocated = false,
       .chars = "OIIAI count: 8",
       .length = 14
    };
    CLAY_TEXT(version_text_string, &overlay_version_text_config);
    CLAY({
        .layout = {
          .sizing = {
            .width = CLAY_SIZING_GROW()
          }
        }
    }){}
    CLAY_TEXT(oiiai_count_string, &overlay_version_text_config);
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
      CLAY_TEXT(CLAY_STRING("MENU: PAUSE"), &pause_menu_title_text_config);
    }
    CLAY({ .id = CLAY_ID("MenuNav"),
      .layout = {
        .layoutDirection = CLAY_TOP_TO_BOTTOM,
        .sizing = {
          .width = CLAY_SIZING_GROW(),
          .height = CLAY_SIZING_GROW()
        },
        .childAlignment = {
          .x = CLAY_ALIGN_X_CENTER,
          .y = CLAY_ALIGN_Y_CENTER
        },
        // Would be cool if there was a better way to ignore MenuHeader's height
        // in centering this on the screen. But this is fine for the preset
        .padding = { 8, 8, 8, 68 },
        .childGap = 16
      },
      // .backgroundColor = {0.0f, 0.3745f, 0.6294f, 1.0f}
    }) {
      pause_menu_button(menu->buttons[0]);
      // CLAY({
      //   .layout = {
      //     .sizing = {
      //       .height = CLAY_SIZING_GROW()
      //     }
      //   }
      // }){}
      pause_menu_button(menu->buttons[1]);
    }
  }

  return Clay_EndLayout();
}
