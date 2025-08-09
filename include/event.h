#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef enum {
  EVENT_COLLISION,
  EVENT_PICKUP
} EventType;

struct GameEvent {
  EventType type;
  uint32_t timestamp;
  union {
    struct {
      int entity_A;
      int entity_B;
    } collision;
    struct {
      int player_id;
      int item_id;
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
void game_event_queue_enqueue(struct GameEvent *game_event);
struct GameEvent *game_event_queue_dequeue();
