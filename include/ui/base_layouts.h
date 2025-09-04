#pragma once

#include "ui_manager.h"

extern struct Layout layout_version_text;
extern struct Layout layout_fps_counter;

Clay_RenderCommandArray ui_base_version_text(void *arg);
void ui_base_version_text_update(float delta_time, void *user_data);
Clay_RenderCommandArray ui_base_fps_counter(void *arg);
void ui_base_fps_counter_update(float delta_time, void *user_data);
