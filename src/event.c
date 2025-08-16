#include "event.h"

static struct GameEventQueue game_event_queue;
static bool game_event_queue_initialized;

static EventType event_types[ENTITY_TYPE_COUNT][ENTITY_TYPE_COUNT] = {
  //              WORLD             ITEM                      PLAYER
  /* WORLD */   {EVENT_COLLISION,   EVENT_COLLISION,          EVENT_COLLISION},
  /* ITEM */    {EVENT_COLLISION,   EVENT_COLLISION,          EVENT_PLAYER_ITEM_PICKUP},
  /* PLAYER */  {EVENT_COLLISION,   EVENT_PLAYER_ITEM_PICKUP, EVENT_COLLISION}
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
        printf("Processing collision event\n");
        break;
      }
      case EVENT_PLAYER_ITEM_PICKUP: {
        printf("Processing player item pickup event\n");
        char player_entity_uuid_str[37];
        uuid_unparse_lower(game_event.data.item_pickup.player_entity_id, player_entity_uuid_str);
        printf("Player id: %s\n", player_entity_uuid_str);
        printf("Item id: %d\n", game_event.data.item_pickup.item_id);
        char uuid_str[37];
        uuid_unparse_lower(game_event.data.item_pickup.item_entity_id, uuid_str);
        printf("Item entity id: %s\n", uuid_str);

        struct PlayerComponent *player = scene_get_player_by_entity_id(game_event_queue.scene, game_event.data.item_pickup.player_entity_id);

        // Toy hardcoded version for now. Refactor to a more ECS style structure,
        // then figure out creating items decoupled from the engine itself
        if (player_add_item(player, game_event.data.item_pickup.item_id, game_event.data.item_pickup.item_count)){
          // Remove the item's entity from the scene graph.
          scene_remove_entity(game_event_queue.scene, game_event.data.item_pickup.item_entity_id);
          // Could also use some kind of "persistent" bool in the future if I want
          // items that don't disappear when a player picks them up.
          printf("Successfully added %d item to the player's inventory\n", game_event.data.item_pickup.item_count);
        }
        break;
      }
      default: {
        printf("Default\n");
        break;
      }
    }
    // game_event_print(&game_event);
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
      printf("DEFAULT\n");
      break;
    }
  }
}
