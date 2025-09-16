#pragma once

struct SceneManager *engine_get_scene_manager();
struct AudioManager *engine_get_audio_manager();
struct UIManager *engine_get_ui_manager();
void engine_start_game();
void engine_exit_game();
void engine_free();
