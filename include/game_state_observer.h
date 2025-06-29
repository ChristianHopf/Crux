#pragma once

// Forward declaration for circular dependency
typedef struct GameState GameState;

typedef void (*GameStateNotification)(void *instance, GameState *game_state);

struct GameStateObserver {
  void *instance;
  GameStateNotification notification;
};
