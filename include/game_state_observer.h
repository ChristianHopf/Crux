#pragma once

#include "game_state.h"

// Forward declaration for circular include
typedef struct GameState;

typedef void (*GameStateNotification)(void *instance, GameState *game_state);

struct GameStateObserver {
  void *instance;
  GameStateNotification notification;
};
