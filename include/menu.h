#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>
#include "text.h"

#define MAX_MENUS 32

typedef enum {
  BUTTON_ACTION, // call some function
  BUTTON_MENU_FORWARD, // navigate forward/back through menu stack
  BUTTON_MENU_BACK,
  BUTTON_QUIT // modify game state, set a should_exit bool
} ButtonType;

struct Button {
  char *text;
  ButtonType type;
  union {
    // ACTION (some function)
    void (*action)(void);
    // FORWARD/BACK
    struct Menu *menu;
  } data;
};

struct Menu {
  char *title
  struct Button *buttons;
  int num_buttons;
  struct Menu *parent;
};

struct MenuStack {
  struct Menu *menus[MAX_MENUS];
  int current_depth;
};


// Menu
void menu_render(struct Menu *menu);
void pause_menu_render();

// Button (maybe make menu_stack a static variable in menu.c like the audio manager)
void button_activate(struct MenuStack *menu_stack, struct Button *button);

// Stack
bool menu_stack_push(struct MenuStack *menu_stack, struct Menu *menu);
bool menu_stack_pop(struct MenuStack *menu_stack);
