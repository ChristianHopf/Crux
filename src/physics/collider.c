#include "physics/collider.h"

// Map colliding entity types to the appropriate resolution strategy
static CollisionBehavior collision_behaviors[ENTITY_TYPE_COUNT][ENTITY_TYPE_COUNT] = {
  //            WORLD                       ITEM                        PLAYER
  /* WORLD */ {COLLISION_BEHAVIOR_PHYSICS, COLLISION_BEHAVIOR_TRIGGER, COLLISION_BEHAVIOR_PHYSICS},
  /* ITEM */  {COLLISION_BEHAVIOR_TRIGGER, COLLISION_BEHAVIOR_PHYSICS, COLLISION_BEHAVIOR_TRIGGER},
  /* PLAYER */ {COLLISION_BEHAVIOR_PHYSICS, COLLISION_BEHAVIOR_TRIGGER, COLLISION_BEHAVIOR_PHYSICS}
};

CollisionBehavior get_collision_behavior(EntityType type_A, EntityType type_B){
  return collision_behaviors[type_A][type_B];
}
