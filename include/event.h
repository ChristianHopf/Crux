#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <uuid/uuid.h>
#include "time.h"
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
      int player_id;
      int item_id;
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
};


// Setup/teardown
void game_event_queue_init();
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
