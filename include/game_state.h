#pragma once

#include <stdbool.h>

// Linked List (might put in its own file and use void*s or something later)
struct ListNode {
  struct GameStateObserver *observer;
  struct ListNode *next;
};

typedef enum {
  GAME_STATE_MAIN_MENU,
  GAME_STATE_PLAYING,
  GAME_STATE_PAUSED
} GameStateMode;

struct GameState {
  GameStateMode mode;
  bool is_paused;
  bool should_quit;
  struct ListNode *observers;
};

// Game state
void game_state_init();
GameStateMode game_state_get_mode();
void game_state_set_mode(GameStateMode mode);
void game_start();
void game_state_pause();
void game_state_unpause();
void game_state_quit();
bool game_state_is_paused();
bool game_state_is_playing();
bool game_state_should_quit();
void game_state_update();

// Observers and linked list
void attach_observer(struct GameStateObserver *observer);
void detach_observer(struct GameStateObserver *observer);
void append_to_list(struct ListNode **linked_list, struct GameStateObserver *observer);
void remove_from_list(struct ListNode **linked_list, struct GameStateObserver *observer);
