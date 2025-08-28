#pragma once

#include <stdbool.h>

// Linked List (might put in its own file and use void*s or something later)
struct ListNode {
  struct GameStateObserver *observer;
  struct ListNode *next;
};

struct GameState {
  bool is_paused;
  bool should_quit;
  struct ListNode *observers;
};

// Game state
void game_state_init();
void game_pause();
void game_unpause();
void game_quit();
bool game_state_is_paused();
bool game_state_should_quit();
void game_state_update();

// Observers and linked list
void attach_observer(struct GameStateObserver *observer);
void detach_observer(struct GameStateObserver *observer);
void append_to_list(struct ListNode **linked_list, struct GameStateObserver *observer);
void remove_from_list(struct ListNode **linked_list, struct GameStateObserver *observer);
