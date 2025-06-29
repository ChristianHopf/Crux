#include "game_state.h"

struct GameState game_state_init(){
  // Return a GameState with default values
  struct GameState game_state = {
    .is_paused = false,
    .observers = NULL
  };

  return game_state;
}

// GameState modifiers
void game_pause(struct GameState *game_state){
  game_state->is_paused = true;
  game_state_update(game_state);
}

void game_unpause(struct GameState *game_state){
  game_state->is_paused = false;
  game_state_update(game_state);
}

// GameState notification sender
void game_state_update(struct GameState *game_state){
  // Iterate through linked list of observers and call their GameStateNotification functions
  struct ListNode *observer_node = game_state->observers;
  while (observer_node != NULL){
    struct GameStateObserver *current_observer = observer_node->observer;
    GameStateNotification notification_function = current_observer->notification;
    notification_function(current_observer->instance, game_state);
    observer_node = observer_node->next;
  }
}

void attach_observer(struct GameState *game_state, struct GameStateObserver *observer){
  // Append observer to this GameState's linked list
  append_to_list(&game_state->observers, observer);
}

void detach_observer(struct GameState *game_state, struct GameStateObserver *observer){
  // Remove observer to this GameState's linked list
  remove_from_list(&game_state->observers, observer);
}

void append_to_list(struct ListNode **linked_list, struct GameStateObserver *observer){
  // Allocate a new list node
  struct ListNode *new_node = (struct ListNode *)malloc(sizeof(struct ListNode));
  new_node->observer = observer;
  new_node->next = NULL;

  // If root node is NULL, we're adding the first node
  if (*linked_list == NULL){
    *linked_list = new_node;
  }

  // Traverse linked list until we find the end (the node whose next is NULL)
  struct ListNode *current_node = *linked_list;
  while (current_node->next != NULL){
    current_node = current_node->next;
  }
  
  // Found the end: append current_node
  current_node->next = new_node;
}

void remove_from_list(struct ListNode **linked_list, struct GameStateObserver *observer){
  // Check root node
  struct ListNode *current_node = *linked_list;
  if (current_node->observer == observer){
    // If this is the only node, set the linked list to NULL
    if (current_node->next == NULL){
      *linked_list = NULL;
      return;
    }
    // Otherwise, set linked list to point to its next node
    *linked_list = current_node->next;
  }

  // Traverse linked list
  while(current_node->next != NULL){
    
    // If the next node is the one we want to delete:
    // - get a pointer to its next node
    // - free the node we want to delete
    // - point this node to that node's next node
    if (current_node->next->observer == observer){
      struct ListNode *next = current_node->next->next;
      free(current_node->next);
      current_node->next = next;
      return;
    }
  }

  // Observer not in list
  fprintf(stderr, "Error: failed to remove observer from list: not in list\n");
}
