#include "menu/menu.h"
#include "menu/menu_presets.h"
#include "ui_manager.h"
#include "game_state.h"
#include "text.h"
#include "engine.h"

static struct MenuManager *menu_manager;

void menu_manager_init(){
  menu_manager = (struct MenuManager *)calloc(1, sizeof(struct MenuManager));

  // Set menu stack to empty (depth -1), create menus
  menu_manager->current_depth = -1;
  menu_manager->pause_menu = pause_menu_create();
  menu_manager->main_menu = main_menu_create();
  // menu_manager->scene_select_menu = scene_select_menu_create();
}

void menu_manager_destroy(){
  // Free menus
  // Free menu manager
}

void menu_render(){
  // Get current menu
  struct Menu *menu = menu_manager->menu_stack[menu_manager->current_depth];
  // Render background color
  glClearColor(0.0f, 0.2745f, 0.5294f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Render title
  text_render(menu->title, 920.0f, 1000.0f, 1.0f, (vec3){1.0f, 1.0f, 1.0f});

  // Render buttons
  for(int i = 0; i < menu->num_buttons; i++){
    text_render(menu->buttons[i].text, menu->buttons[i].x, menu->buttons[i].y, 1.0f, (vec3){1.0f, 1.0f, 1.0f});
  }
}

// struct Menu *pause_menu_create(){
//   struct Menu *pause_menu = (struct Menu *)calloc(1, sizeof(struct Menu));
//   pause_menu->title = "PAUSED";
//   pause_menu->num_buttons = 2;
//   pause_menu->buttons = (struct Button *)calloc(pause_menu->num_buttons, sizeof(struct Button));
//   pause_menu->buttons[0].text = "RESUME";
//   pause_menu->buttons[0].type = BUTTON_ACTION;
//   pause_menu->buttons[0].data.action = action_resume;
//   pause_menu->buttons[0].x = 920.0f;
//   pause_menu->buttons[0].y = 540.0f;
//   pause_menu->buttons[1].text = "EXIT";
//   pause_menu->buttons[1].type = BUTTON_ACTION;
//   pause_menu->buttons[1].data.action = action_exit;
//   pause_menu->buttons[1].x = 920.0f;
//   pause_menu->buttons[1].y = 480.0f;
//   pause_menu->parent = NULL;
//
//   return pause_menu;
// }

struct Menu *menu_manager_get_pause_menu(){
  return menu_manager->pause_menu;
}

// struct Menu *main_menu_create(){
//   struct Menu *main_menu = (struct Menu *)calloc(1, sizeof(struct Menu));
//   main_menu->title = "MAIN";
//   main_menu->num_buttons = 2;
//   main_menu->buttons = (struct Button *)calloc(main_menu->num_buttons, sizeof(struct Button));
//   main_menu->buttons[0].text = "START";
//   main_menu->buttons[0].type = BUTTON_ACTION;
//   main_menu->buttons[0].data.action = action_start;
//   main_menu->buttons[0].x = 920.0f;
//   main_menu->buttons[0].y = 540.0f;
//   main_menu->buttons[1].text = "QUIT";
//   main_menu->buttons[1].type = BUTTON_ACTION;
//   main_menu->buttons[1].data.action = action_quit;
//   main_menu->buttons[1].x = 920.0f;
//   main_menu->buttons[1].y = 480.0f;
//   main_menu->parent = NULL;
//
//   return main_menu;
// }

struct Menu *menu_manager_get_main_menu(){
  return menu_manager->main_menu;
}

// struct Menu *menu_manager_get_scene_select_menu(){
//   return menu_manager->scene_select_menu;
// }

bool menu_stack_push(struct Menu *menu){
  // Check if stack is already full
  if (menu_stack_is_full()){
    fprintf(stderr, "Error: failed to push to menu stack: menu stack is full\n");
    return false;
  }

  menu_manager->menu_stack[++menu_manager->current_depth] = menu;
  return true;
}

bool menu_stack_pop(){
  // Check if stack is already empty
  if (menu_stack_is_empty()){
    fprintf(stderr, "Error: failed to pop from menu stack: menu stack is empty\n");
    return false;
  }

  menu_manager->current_depth--;
  return true;
}

bool menu_stack_is_full(){
  return menu_manager->current_depth >= MAX_MENU_DEPTH - 1;
}

bool menu_stack_is_empty(){
  return menu_manager->current_depth == -1;
}

// void action_start(void *arg){
//   engine_start_game();
// }
//
// void action_resume(void *arg){
//   // Unpause
//   // printf("Resume action\n");
//   game_state_unpause();
// }
//
// void action_exit(void *arg){
//   engine_exit_game();
// }
//
// void action_quit(void *arg){
//   // printf("Quit action\n");
//   game_state_quit();
// }

void menu_button_activate(struct Button *button){
  if (!button){
    fprintf(stderr, "Error: invalid button in menu_button_activate\n");
    return;
  }
  
  struct UIManager *ui_manager = engine_get_ui_manager();
  if (!ui_manager){
    fprintf(stderr, "Error: failed to get ui manager in menu_button_activate\n");
    return;
  }

  switch(button->type){
    case BUTTON_ACTION: {
      if (button->data.action){
        button->data.action(NULL);
      }
      break;
    }
    case BUTTON_MENU_FORWARD: {
      if (button->data.menu){
        // Swap active menu with a new menu
        ui_layout_stack_pop(ui_manager);
        ui_layout_stack_push(ui_manager, button->data.menu->layout);
      }
      break;
    }
    case BUTTON_MENU_BACK: {
      ui_layout_stack_pop(ui_manager);
      break;
    }
    default: {
      fprintf(stderr, "Error: unknown button type in menu_button_activate\n");
      break;
    }
  }
}

void button_print_text(void *arg){
  struct Button *button = (struct Button *)arg;

  if (button){
    printf("Clicked button with text: %s\n", button->text);
  }
}
