#pragma once

#include "ui_manager.h"

extern struct Layout layout_version_text;

Clay_RenderCommandArray ui_version_text(void *arg);
void ui_fps_counter(void *arg);
void ui_fps_counter_update(void *arg);
