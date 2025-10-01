#include "event/callbacks.h"
#include "engine.h"
#include "scene.h"

void event_listener_on_item_pickup_sound(struct GameEvent *game_event, void *user_data){
  struct AudioManager *audio_manager = (struct AudioManager *)user_data;
  struct SceneManager *scene_manager = engine_get_scene_manager();
  struct AudioComponent *item_audio_component = scene_get_audio_component_by_entity_id(scene_manager->active_scene, game_event->data.item_pickup.item_entity_id);

  if (item_audio_component) audio_component_play(audio_manager, item_audio_component);
}
