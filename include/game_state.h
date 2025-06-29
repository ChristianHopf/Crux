#pragma once

#include <stdio.h>
#include <stdlib.h>
#include "game_state_observer.h"

// Linked List (might put in its own file and use void*s or something later)
struct ListNode {
  struct GameStateObserver *observer;
  struct ListNode *next;
};

struct GameState {
  bool is_paused;
  struct ListNode *observers;
};

// Game state
struct GameState game_state_init();
void game_pause(struct GameState *game_state);
void game_unpause(struct GameState *game_state);
void game_state_update(struct GameState *game_state);

// Observers and linked list
void attach_observer(struct GameState *game_state, struct GameStateObserver *observer);
void detach_observer(struct GameState *game_state, struct GameStateObserver *observer);
void append_to_list(struct ListNode **linked_list, struct GameStateObserver *observer);
void remove_from_list(struct ListNode **linked_list, struct GameStateObserver *observer);
