#pragma once

#include "clay.h"
#include "stdio.h"

// Button click handlers
void handle_button_click(Clay_ElementId elementId, Clay_PointerData pointerData, intptr_t userData);

// Layout components
void pause_menu_button(struct Button *button);

// Layout functions
Clay_RenderCommandArray compute_clay_layout_overlay(void *arg);
Clay_RenderCommandArray compute_clay_layout_pause_menu(void *arg);

struct Menu *pause_menu_create();
struct Menu *main_menu_create();

void action_load_scene_bouncehouse(void *arg);
void action_load_scene_items(void *arg);
void action_load_scene_scenegraph(void *arg);
void action_start(void *arg);
void action_resume(void *arg);
void action_exit(void *arg);
void action_quit(void *arg);
