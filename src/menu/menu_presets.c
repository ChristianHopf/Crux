#include <string.h>
#include "menu.h"
#include "menu/menu_presets.h"
#include "ui/base_layouts.h"
#include "engine.h"
#include "scene.h"
#include "game_state.h"

Clay_TextElementConfig pause_menu_title_text_config = {.fontId = 1, .fontSize = 48, .textColor = {255, 255, 255, 255}};
Clay_TextElementConfig pause_menu_button_text_config = {.fontId = 2, .fontSize = 48, .lineHeight = 48, .textAlignment = CLAY_TEXT_ALIGN_CENTER, .textColor = {255, 255, 255, 255}};
Clay_TextElementConfig overlay_version_text_config = { .fontId = 0, .fontSize = 24, .textColor = {255, 255, 255, 255}};

void handle_button_click(Clay_ElementId elementId, Clay_PointerData pointerData, intptr_t userData){
  if (pointerData.state == CLAY_POINTER_DATA_RELEASED_THIS_FRAME){
    // Call the button's action
    void (*action)(void *arg) = (void (*)(void *))userData;
    action(NULL);
  }
}

void pause_menu_button(struct Button *button){
  CLAY({
    .layout = {
      .padding = {16, 16, 0, 12}
    },
    .backgroundColor = Clay_Hovered() ? (Clay_Color){0.0f, 0.549f, 1.0f, 0.8f} : (Clay_Color){0.0f, 0.3745f, 0.6294f, 0.0f}
  }){
    Clay_OnHover(handle_button_click, (intptr_t)button->data.action);
    Clay_String button_string = {
      .isStaticallyAllocated = false,
      .chars = button->text,
      .length = strlen(button->text)
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
    // Clay_String oiiai_count_string = {
    //    .isStaticallyAllocated = false,
    //    .chars = "OIIAI count: 8",
    //    .length = 14
    // };
    CLAY_TEXT(version_text_string, &overlay_version_text_config);
    CLAY({
        .layout = {
          .sizing = {
            .width = CLAY_SIZING_GROW()
          }
        }
    }){}
    // CLAY_TEXT(oiiai_count_string, &overlay_version_text_config);
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
      pause_menu_button(&menu->buttons[0]);
      // CLAY({
      //   .layout = {
      //     .sizing = {
      //       .height = CLAY_SIZING_GROW()
      //     }
      //   }
      // }){}
      pause_menu_button(&menu->buttons[1]);
    }
  }

  return Clay_EndLayout();
}

struct Menu *pause_menu_create(){
  struct Menu *pause_menu = (struct Menu *)calloc(1, sizeof(struct Menu));
  if (!pause_menu){
    fprintf(stderr, "Error: failed to allocate menu in pause_menu_create\n");
    return NULL;
  }

  pause_menu->title = "PAUSED";
  pause_menu->num_buttons = 2;
  pause_menu->buttons = (struct Button *)calloc(pause_menu->num_buttons, sizeof(struct Button));
  if (!pause_menu->buttons){
    fprintf(stderr, "Error: failed to allocate buttons in pause_menu_create\n");
    return NULL;
  }

  pause_menu->buttons[0].text = "RESUME";
  pause_menu->buttons[0].type = BUTTON_ACTION;
  pause_menu->buttons[0].data.action = action_resume;
  pause_menu->buttons[0].x = 920.0f;
  pause_menu->buttons[0].y = 540.0f;
  pause_menu->buttons[1].text = "EXIT";
  pause_menu->buttons[1].type = BUTTON_ACTION;
  pause_menu->buttons[1].data.action = action_exit;
  pause_menu->buttons[1].x = 920.0f;
  pause_menu->buttons[1].y = 480.0f;

  pause_menu->layout = &layout_pause_menu;

  pause_menu->parent = NULL;

  return pause_menu;
}

struct Menu *main_menu_create(){
  struct Menu *main_menu = (struct Menu *)calloc(1, sizeof(struct Menu));
  if (!main_menu){
    fprintf(stderr, "Error: failed to allocate menu in main_menu_create\n");
    return NULL;
  }

  main_menu->title = "MAIN";
  main_menu->num_buttons = 3;
  main_menu->buttons = (struct Button *)calloc(main_menu->num_buttons, sizeof(struct Button));
  if (!main_menu->buttons){
    fprintf(stderr, "Error: failed to allocate Buttons in main_menu_create\n");
    free(main_menu);
    return NULL;
  }

  main_menu->buttons[0].text = "START";
  main_menu->buttons[0].type = BUTTON_ACTION;
  main_menu->buttons[0].data.action = action_start;

  struct Menu *scene_select_menu = scene_select_menu_create();
  if (!scene_select_menu){
    fprintf(stderr, "Error: failed to create scene select menu in main_menu_create\n");
    free(main_menu->buttons);
    free(main_menu);
    return NULL;
  }
  main_menu->buttons[1].text = "SCENE SELECT";
  main_menu->buttons[1].type = BUTTON_MENU_FORWARD;
  main_menu->buttons[1].data.menu = scene_select_menu;

  main_menu->buttons[2].text = "QUIT";
  main_menu->buttons[2].type = BUTTON_ACTION;
  main_menu->buttons[2].data.action = action_quit;

  main_menu->layout = &layout_main_menu;

  main_menu->parent = NULL;

  return main_menu;
}

struct Menu *scene_select_menu_create(){
  struct Menu *scene_select_menu = (struct Menu *)calloc(1, sizeof(struct Menu));
  if (!scene_select_menu){
    fprintf(stderr, "Error: failed to allocate menu in scene_select_menu_create\n");
    return NULL;
  }

  scene_select_menu->title = "SCENE SELECT";
  scene_select_menu->num_buttons = 4;
  scene_select_menu->buttons = (struct Button *)calloc(scene_select_menu->num_buttons, sizeof(struct Button));
  if (!scene_select_menu->buttons){
    fprintf(stderr, "Error: failed to allocate buttons in scene_select_menu_create\n");
    free(scene_select_menu);
    return NULL;
  }

  scene_select_menu->buttons[0].text = "BOUNCEHOUSE";
  scene_select_menu->buttons[0].type = BUTTON_ACTION;
  scene_select_menu->buttons[0].data.action = action_load_scene_bouncehouse;

  scene_select_menu->buttons[1].text = "ITEMS";
  scene_select_menu->buttons[1].type = BUTTON_ACTION;
  scene_select_menu->buttons[1].data.action = action_load_scene_items;

  scene_select_menu->buttons[2].text = "SCENE GRAPH";
  scene_select_menu->buttons[2].type = BUTTON_ACTION;
  scene_select_menu->buttons[2].data.action = action_load_scene_scenegraph;

  scene_select_menu->buttons[3].text = "BACK";
  scene_select_menu->buttons[3].type = BUTTON_MENU_BACK;
  // scene_select_menu->buttons[4].data.action = action_quit;

  // Having a menu own a layout which references the menu doesn't seem
  // like the best solution to settle on.
  layout_scene_select_menu.user_data = scene_select_menu;
  scene_select_menu->layout = &layout_scene_select_menu;

  scene_select_menu->parent = NULL;

  return scene_select_menu;
}

void action_load_scene_bouncehouse(void *arg){
  struct SceneManager *scene_manager = engine_get_scene_manager();
  if (!scene_manager){
    fprintf(stderr, "Error: failed to get SceneManager in action_load_scene_bouncehouse\n");
    return;
  }

  scene_manager_load_scene(scene_manager, "scenes/bouncehouse.json");
  if (!scene_manager->active_scene){
    fprintf(stderr, "Error: failed to load scene in action_load_scene_bouncehouse\n");
    return;
  }
  engine_start_game();
}

void action_load_scene_items(void *arg){
  struct SceneManager *scene_manager = engine_get_scene_manager();
  if (!scene_manager){
    fprintf(stderr, "Error: failed to get SceneManager in action_load_scene_bouncehouse\n");
    return;
  }

  scene_manager_load_scene(scene_manager, "scenes/items.json");
  if (!scene_manager->active_scene){
    fprintf(stderr, "Error: failed to load scene in action_load_scene_items\n");
    return;
  }
  engine_start_game();
}

void action_load_scene_scenegraph(void *arg){
  struct SceneManager *scene_manager = engine_get_scene_manager();
  if (!scene_manager){
    fprintf(stderr, "Error: failed to get SceneManager in action_load_scene_bouncehouse\n");
    return;
  }

  scene_manager_load_scene(scene_manager, "scenes/scenegraph.json");
  if (!scene_manager->active_scene){
    fprintf(stderr, "Error: failed to load scene in action_load_scene_scenegraph\n");
    return;
  }
  engine_start_game();
}

void action_start(void *arg){
  engine_start_game();
}

void action_resume(void *arg){
  game_state_unpause();
}

void action_exit(void *arg){
  engine_exit_game();
}

void action_quit(void *arg){
  game_state_quit();
}
