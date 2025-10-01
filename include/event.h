#pragma once

#include <stdint.h>
#include <uuid/uuid.h>
#include "time.h"
#include "types.h"

#define MAX_EVENT_TYPES 64
#define MAX_LISTENERS_PER_TYPE 32

typedef enum {
  EVENT_COLLISION = 0,
  EVENT_PLAYER_ITEM_PICKUP,
} EventType;

struct GameEvent {
  EventType type;
  struct timespec timestamp;
  union {
    struct {
      uuid_t entity_A_id;
      uuid_t entity_B_id;
    } collision;
    struct {
      uuid_t player_entity_id;
      int item_id;
      int item_count;
      uuid_t item_entity_id;
    } item_pickup;
    void *custom;
  } data;
};

typedef void (*EventCallback)(struct GameEvent *game_event, void *user_data);

struct EventListener {
  EventCallback callback;
  void *user_data;
};

struct EventRegistry {
  struct EventListener listeners[MAX_EVENT_TYPES][MAX_LISTENERS_PER_TYPE];
  int listener_counts[MAX_EVENT_TYPES];
};

struct GameEventQueue {
  struct GameEvent *events;
  int capacity;
  int front;
  int back;
  int size;
  struct Scene *scene;
  struct EventRegistry event_registry;
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

// Listeners
void event_listener_register(EventType type, EventCallback callback, void *user_data);
void event_listener_unregister(EventType type, EventCallback callback);
