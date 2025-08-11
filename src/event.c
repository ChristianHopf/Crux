#include "event.h"

static struct GameEventQueue game_event_queue;
static bool game_event_queue_initialized;


void game_event_queue_init(){
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

  for(int i = 0; i < game_event_queue.size; i++){
    printf("Event index %d", i);
    game_event_print(&game_event_queue.events[(game_event_queue.front + i) % game_event_queue.capacity]);
  }
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
      default: {
        printf("Default\n");
        break;
      }
    }
    printf("Timestamp seconds: %ld, timestamp nanoseconds: %ld\n", game_event.timestamp.tv_sec, game_event.timestamp.tv_nsec);
  }
}

void game_event_print(struct GameEvent *game_event){
  switch(game_event->type){
    case EVENT_COLLISION: {
      printf("COLLISION EVENT\n");
      break;
    }
    default: {
      printf("DEFAULT\n");
      break;
    }
  }
}
