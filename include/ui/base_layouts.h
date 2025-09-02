#pragma once

#include "ui_manager.h"

extern struct Layout layout_version_text;
extern struct Layout layout_fps_counter;

Clay_RenderCommandArray ui_version_text(void *arg);
Clay_RenderCommandArray ui_fps_counter(void *arg);
void ui_fps_counter_update(float delta_time, void *arg);
