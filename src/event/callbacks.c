#include <uuid/uuid.h>
#include "event.h"
#include "event/callbacks.h"
#include "engine.h"
#include "scene.h"
#include "audio_manager.h"
#include "inventory.h"

bool event_listener_on_item_pickup_add_to_inventory(struct GameEvent *game_event, void *user_data){
  struct Scene *active_scene = (struct Scene *)user_data;
  struct PlayerComponent *player = scene_get_player_by_entity_id(active_scene, game_event->data.item_pickup.player_entity_id);
  struct InventoryComponent *inventory_component = scene_get_inventory_by_entity_id(active_scene, game_event->data.item_pickup.player_entity_id);

  // Add to inventory
  if (inventory_add_item(inventory_component, &active_scene->item_registry, game_event->data.item_pickup.item_id, game_event->data.item_pickup.item_count)){
    return false;
  }
  return true;
}

bool event_listener_on_item_pickup_sound(struct GameEvent *game_event, void *user_data){
  printf("Calling event_listener_on_item_pickup_sound\n");
  struct AudioManager *audio_manager = (struct AudioManager *)user_data;
  struct SceneManager *scene_manager = engine_get_scene_manager();
  struct AudioComponent *item_audio_component = scene_get_audio_component_by_entity_id(scene_manager->active_scene, game_event->data.item_pickup.item_entity_id);

  char uuid_str[37];
  uuid_unparse(game_event->data.item_pickup.item_entity_id, uuid_str);
  printf("Item entity id %s\n", uuid_str);

  if (item_audio_component){
    audio_component_play(audio_manager, item_audio_component, 1);
    return false;
  }
  else{
    printf("No item audio component\n");
    return true;
  }
}

bool event_listener_on_item_pickup_remove_entity(struct GameEvent *game_event, void *user_data){
  struct Scene *active_scene = (struct Scene *)user_data;
  scene_remove_entity(active_scene, game_event->data.item_pickup.item_entity_id);
  return true;
}
