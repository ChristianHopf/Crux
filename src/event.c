#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "event.h"
#include "scene.h"
#include "player.h"
#include "inventory.h"
#include "audio_manager.h"
#include "engine.h"

static struct GameEventQueue game_event_queue;
static bool game_event_queue_initialized;

static EventType event_types[ENTITY_TYPE_COUNT][ENTITY_TYPE_COUNT] = {
  //              GROUPING        WORLD             ITEM                      PLAYER
  /* GROUPING */{EVENT_COLLISION, EVENT_COLLISION,  EVENT_COLLISION,          EVENT_COLLISION},
  /* WORLD */   {EVENT_COLLISION, EVENT_COLLISION,  EVENT_COLLISION,          EVENT_COLLISION},
  /* ITEM */    {EVENT_COLLISION, EVENT_COLLISION,  EVENT_COLLISION,          EVENT_PLAYER_ITEM_PICKUP},
  /* PLAYER */  {EVENT_COLLISION, EVENT_COLLISION,  EVENT_PLAYER_ITEM_PICKUP, EVENT_COLLISION}
};


void game_event_queue_init(struct Scene *scene){
  if (game_event_queue_initialized) return;

  game_event_queue.capacity = 1024;
  game_event_queue.events = (struct GameEvent *)calloc(game_event_queue.capacity, sizeof(struct GameEvent));
  if (!game_event_queue.events){
    fprintf(stderr, "Error: failed to allocate GameEvents in game_event_queue_init\n");
    return;
  }
  game_event_queue.front = 0;
  game_event_queue.back = 0;
  game_event_queue.size = 0;

  game_event_queue.scene = scene;

  game_event_queue_initialized = true;
}

void game_event_queue_destroy(){
  if (!game_event_queue_initialized) return;
  
  // Not needed unless I refactor events to be an array of pointers
  // to dynamically allocated GameEvents, but passing them by value should be fine.
  // Free each GameEvent in the queue
  // int i = 0;
  // while (i < game_event_queue.size){
  //   int index = (game_event_queue.front + i) % game_event_queue.capacity;
  //   struct GameEvent *game_event = game_event_queue.events[index];
  //   if (game_event) free(game_event);
  //
  //   i++;
  // }

  // Free queue
  free(game_event_queue.events);

  game_event_queue_initialized = false;
}

void game_event_queue_enqueue(struct GameEvent game_event){
  if (game_event_queue.size >= game_event_queue.capacity){
    fprintf(stderr, "Error: failed to add GameEvent to queue: queue is full\n");
    return;
  }

  // Add GameEvent to the back of the queue
  game_event_queue.events[game_event_queue.back] = game_event;
  game_event_queue.back = (game_event_queue.back + 1) % game_event_queue.capacity;
  game_event_queue.size++;
}

bool game_event_queue_dequeue(struct GameEvent *game_event){
  if (game_event_queue.size == 0){
    // fprintf(stderr, "Error: failed to dequeue GameEvent from queue: queue is empty\n");
    return false;
  }

  // Assign current front event to given event
  *game_event = game_event_queue.events[game_event_queue.front];

  // Move front back, decrement size
  game_event_queue.front = (game_event_queue.front + 1) % game_event_queue.capacity;
  game_event_queue.size--;

  return true;
}

void game_event_queue_clear(){
  free(game_event_queue.events);
  game_event_queue.events = (struct GameEvent *)calloc(game_event_queue.capacity, sizeof(struct GameEvent));
}

bool game_event_queue_is_full(){
  return game_event_queue.size >= game_event_queue.capacity;
}

bool game_event_queue_is_empty(){
  return game_event_queue.size == 0;
}

void game_event_queue_process(){
  struct GameEvent game_event;
  while (game_event_queue_dequeue(&game_event)){
    switch (game_event.type){
      case EVENT_COLLISION: {
        // Get colliding entities' AudioComponents
        struct AudioComponent *audio_component_A = scene_get_audio_component_by_entity_id(game_event_queue.scene, game_event.data.collision.entity_A_id);
        struct AudioComponent *audio_component_B = scene_get_audio_component_by_entity_id(game_event_queue.scene, game_event.data.collision.entity_B_id);

        struct AudioManager *audio_manager = engine_get_audio_manager();

        if (audio_component_A) audio_component_play(audio_manager, audio_component_A);
        if (audio_component_B) audio_component_play(audio_manager, audio_component_B);
        break;
      }
      case EVENT_PLAYER_ITEM_PICKUP: {
        struct PlayerComponent *player = scene_get_player_by_entity_id(game_event_queue.scene, game_event.data.item_pickup.player_entity_id);
        struct InventoryComponent *inventory_component = scene_get_inventory_by_entity_id(game_event_queue.scene, game_event.data.item_pickup.player_entity_id);

        if (inventory_add_item(inventory_component, &game_event_queue.scene->item_registry, game_event.data.item_pickup.item_id, game_event.data.item_pickup.item_count)){
          scene_remove_entity(game_event_queue.scene, game_event.data.item_pickup.item_entity_id);
          inventory_print(&game_event_queue.scene->item_registry, inventory_component);
        }
        else{
          // printf("Failed to add %d item(s) to the player's inventory\n", game_event.data.item_pickup.item_count);
        }
        break;
      }
      default: {
        fprintf(stderr, "Error: unknown event type in game_event_queue_process\n");
        break;
      }
    }
  }
}

EventType get_event_type(EntityType type_A, EntityType type_B){
  return event_types[type_A][type_B];
}

void game_event_print(struct GameEvent *game_event){
  switch(game_event->type){
    case EVENT_COLLISION: {
      printf("Event type: EVENT_COLLISION\n");
      printf("Timestamp seconds: %ld, timestamp nanoseconds: %ld\n\n", game_event->timestamp.tv_sec, game_event->timestamp.tv_nsec);
      break;
    }
    case EVENT_PLAYER_ITEM_PICKUP: {
      printf("Event type: EVENT_PLAYER_ITEM_PICKUP\n");
      printf("Timestamp seconds: %ld, timestamp nanoseconds: %ld\n\n", game_event->timestamp.tv_sec, game_event->timestamp.tv_nsec);
      break;
    }
    default: {
      printf("Unknown event type in game_event_print\n");
      break;
    }
  }
}
