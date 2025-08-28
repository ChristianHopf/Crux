#include "physics/collider.h"

// Map colliding entity types to the appropriate resolution strategy
// Entities with ENTITY_GROUPING should never collide, because such Entities should
// never have PhysicsBodies.
// It has to be included in the table, though, since I'm using it for giving entities to nodes used for logically grouping other nodes.
static CollisionBehavior collision_behaviors[ENTITY_TYPE_COUNT][ENTITY_TYPE_COUNT] = {
  //            GROUPING                    WORLD                       ITEM                        PLAYER
  /* GROUPING */ {COLLISION_BEHAVIOR_TRIGGER, COLLISION_BEHAVIOR_TRIGGER, COLLISION_BEHAVIOR_TRIGGER, COLLISION_BEHAVIOR_TRIGGER},
  /* WORLD */ {COLLISION_BEHAVIOR_TRIGGER, COLLISION_BEHAVIOR_PHYSICS, COLLISION_BEHAVIOR_PHYSICS, COLLISION_BEHAVIOR_PHYSICS},
  /* ITEM */  {COLLISION_BEHAVIOR_TRIGGER, COLLISION_BEHAVIOR_PHYSICS, COLLISION_BEHAVIOR_PHYSICS, COLLISION_BEHAVIOR_TRIGGER},
  /* PLAYER */ {COLLISION_BEHAVIOR_TRIGGER, COLLISION_BEHAVIOR_PHYSICS, COLLISION_BEHAVIOR_TRIGGER, COLLISION_BEHAVIOR_PHYSICS},
};

CollisionBehavior get_collision_behavior(EntityType type_A, EntityType type_B){
  return collision_behaviors[type_A][type_B];
}
