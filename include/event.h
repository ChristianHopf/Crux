#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <uuid/uuid.h>
#include "scene.h"
#include "time.h"
// #include "player.h"
#include "types.h"

typedef enum {
  EVENT_COLLISION = 0,
  EVENT_PLAYER_ITEM_PICKUP
} EventType;

struct GameEvent {
  EventType type;
  struct timespec timestamp;
  union {
    struct {
      int entity_A;
      int entity_B;
    } collision;
    struct {
      uuid_t player_entity_id;
      int item_id;
      int item_count;
      uuid_t item_entity_id;
    } item_pickup;
  } data;
};

struct GameEventQueue {
  struct GameEvent *events;
  int capacity;
  int front;
  int back;
  int size;
  struct Scene *scene;
};


// Setup/teardown
void game_event_queue_init(struct Scene *scene);
void game_event_queue_destroy();

// Enqueue/dequeue
void game_event_queue_enqueue(struct GameEvent game_event);
bool game_event_queue_dequeue(struct GameEvent *game_event);
void game_event_queue_clear();
bool game_event_queue_is_full();
bool game_event_queue_is_empty();
void game_event_queue_process();
EventType get_event_type(EntityType type_A, EntityType type_B);
void game_event_print(struct GameEvent *game_event);
