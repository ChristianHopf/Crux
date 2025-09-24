#pragma once

// #include <glad/glad.h>
// #include <GLFW/glfw3.h>
// #include <cglm/cglm.h>

#define MAX_MENU_DEPTH 32

typedef enum {
  BUTTON_ACTION, // call some function
  BUTTON_MENU_FORWARD, // navigate forward/back through menu stack
  BUTTON_MENU_BACK,
} ButtonType;

struct Button {
  char *text;
  ButtonType type;
  union {
    // ACTION (some function)
    void (*action)(void *arg);
    // FORWARD/BACK
    struct Menu *menu;
  } data;
  float x;
  float y;
};

struct Menu {
  char *title;
  struct Button *buttons;
  int num_buttons;
  struct Layout *layout;
  struct Menu *parent;
};

struct MenuManager {
  struct Menu *menu_stack[MAX_MENU_DEPTH];
  int current_depth;

  struct Menu *pause_menu;
  struct Menu *main_menu;
};


// MenuManager and Menus
void menu_manager_init();
void menu_manager_destroy();
void menu_render();
struct Menu *pause_menu_create();
struct Menu *menu_manager_get_pause_menu();
struct Menu *main_menu_create();
struct Menu *menu_manager_get_main_menu();

// Stack
// void menu_stack_init();
bool menu_stack_push(struct Menu *menu);
bool menu_stack_pop();
bool menu_stack_is_full();
bool menu_stack_is_empty();

// Button
void menu_button_activate(struct Button *button);
void button_print_text(void *arg);

// Actions
void action_start(void *arg);
void action_resume(void *arg);
void action_exit(void *arg);
void action_quit(void *arg);
