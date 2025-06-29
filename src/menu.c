#include "menu.h"

static struct MenuManager *menu_manager;

void menu_manager_init(){
  menu_manager = (struct MenuManager *)calloc(1, sizeof(struct MenuManager));

  // Set menu stack to empty (depth -1), create menus
  menu_manager->current_depth = -1;
  menu_manager->pause_menu = pause_menu_create();
}

void menu_manager_destroy(){
  // Free menus
  // Free menu manager
}

void menu_render(){
  // Get current menu
  struct Menu *menu = menu_manager->menu_stack[menu_manager->current_depth];
  // Render background color
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Render title
  text_render(menu->title, 960.0f, 1000.0f, 1.0f, (vec3){1.0f, 1.0f, 1.0f});

  // Render buttons
  for(int i = 0; i < menu->num_buttons; i++){
    text_render(menu->buttons[i].text, menu->buttons[i].x, menu->buttons[i].y, 1.0f, (vec3){1.0f, 1.0f, 1.0f});
  }
}

struct Menu *pause_menu_create(){
  struct Menu *pause_menu = (struct Menu *)malloc(sizeof(struct Menu));
  pause_menu->title = "PAUSED";
  pause_menu->num_buttons = 2;
  pause_menu->buttons = (struct Button *)malloc(pause_menu->num_buttons * sizeof(struct Button));
  pause_menu->buttons[0].text = "RESUME";
  pause_menu->buttons[0].type = BUTTON_ACTION;
  pause_menu->buttons[0].data.action = button_print_text;
  pause_menu->buttons[0].x = 960.0f;
  pause_menu->buttons[0].y = 540.0f;
  pause_menu->buttons[1].text = "EXIT";
  pause_menu->buttons[1].type = BUTTON_ACTION;
  pause_menu->buttons[1].data.action = button_print_text;
  pause_menu->buttons[1].x = 960.0f;
  pause_menu->buttons[1].y = 500.0f;
  pause_menu->parent = NULL;

  return pause_menu;
}

struct Menu *menu_manager_get_pause_menu(){
  return menu_manager->pause_menu;
}

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

void button_print_text(void *arg){
  struct Button *button = (struct Button *)arg;

  if (button){
    printf("Clicked button with text: %s\n", button->text);
  }
}
