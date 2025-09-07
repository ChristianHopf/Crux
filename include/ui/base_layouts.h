#pragma once

#include "ui_manager.h"

extern struct Layout layout_version_text;
extern struct Layout layout_fps_counter;
extern struct Layout layout_main_menu;


// Text layouts
Clay_RenderCommandArray ui_base_version_text(void *arg);
void ui_base_version_text_update(float delta_time, void *user_data);
Clay_RenderCommandArray ui_base_fps_counter(void *arg);
void ui_base_fps_counter_update(float delta_time, void *user_data);

// Menu layouts
Clay_RenderCommandArray ui_base_pause_menu(void *arg);
void ui_base_pause_menu_update(float delta_time, void *user_data);
Clay_RenderCommandArray ui_base_main_menu(void *arg);
void ui_base_main_menu_update(float delta_time, void *user_data);
