#pragma once

#include "clay.h"
#include "stdio.h"
#include "menu.h"

// Button click handlers
void handle_button_click(Clay_ElementId elementId, Clay_PointerData pointerData, intptr_t userData);

// Layout components
void pause_menu_button(struct Button button);

// Layout functions
Clay_RenderCommandArray compute_clay_layout_overlay(void *arg);
Clay_RenderCommandArray compute_clay_layout_pause_menu(void *arg);
