#pragma once

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

void attach_observer(struct GameState *game_state, struct GameStateObserver *observer);
void detach_observer(struct GameState *game_state, struct GameStateObserver *observer);
